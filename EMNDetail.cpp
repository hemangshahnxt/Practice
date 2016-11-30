#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "EMNDetail.h"
#include "EMRUtils.h"
#include "EmrEditorDlg.h"
#include "EMN.h"
#include "EMRTopic.h"
#include "EmrItemAdvListDlg.h"
#include "EmrItemAdvTextDlg.h"
#include "EmrItemAdvImageDlg.h"
#include "EmrItemAdvSliderDlg.h"
#include "EmrItemAdvNarrativeDlg.h"
#include "EmrItemAdvTableDlg.h"
#include "EmrItemEntryDlg.h"
#include "EMRItemAdvPopupDlg.h"
#include "AuditTrail.h"
#include "Mirror.h"
#include "FileUtils.h"
#include "EMNLoader.h"
#include "EMNUnspawner.h"
#include "EMNSpawner.h"
#include "EMRDebugDlg.h"
#include "TodoUtils.h"
#include "PicContainerDlg.h"
#include "InvVisionWebUtils.h"
#include "InternationalUtils.h"
#include "foreach.h"
#include "ImageArray.h"
#include "MultiSelectDlg.h"
#include "EmrColors.h"
#include "WoundCareCalculator.h"
#include <NxSystemUtilitiesLib/dynamic_ptr.h>
#include <NxXMLUtils.h>
#include <NxDataUtilitiesLib/NxSafeArray.h>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_set.hpp>
#include <NxAlgorithm.h>
#include "EMRHotSpot.h"
#include "NxInkPictureText.h"
#include "DevicePluginUtils.h"
#include "EmrInfoCommonListCollection.h"  // (c.haag 2011-03-17) - PLID 42895
#include "EMRItemAdvMultiPopupDlg.h"
#include "EMR.h"
#include "PatientDocumentStorageWarningDlg.h"
#include "HistoryUtils.h"
#include "NxCache.h"
#include "GlobalStringUtils.h"
#include "NxAutoQuantum.h"
#include "EmrItemAdvImageState.h"	// (j.armen 2014-07-21 16:32) - PLID 62836

// (a.walling 2011-08-11 16:43) - PLID 45021 - TableRow.m_pID is now TableRow.m_ID, which is not allocated on the heap.

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.walling 2014-01-30 00:00) - PLID 60541 - #NewObjectsT now a table so can be referenced by other sprocs

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2012-10-31 17:17) - PLID 53550 - LastSavedDetail - m_pLastSavedDetail now m_oLastSavedDetail;
// simple members should still work fine as if it was a pointer (eg m_strLabelText, etc)


//DRT 1/23/2008 - PLID 28697
// (a.walling 2012-10-31 17:17) - PLID 53551 - I changed this just to use the vector parameter,
// rather than muck about changing the CDWordArray types to a Container type
// if the data was sorted (or we copy to a vector and sort), we could simply do:
//	boost::set_difference(curData, oldData, back_inserter(addedData));
//	boost::set_difference(oldData, curData, back_inserter(removedData));
static void ForSave_CalculateChangedHotSpots(const CDWordArray *parySelectedData, const std::vector<long> *paryPreviousIDs, CDWordArray *paryNewIDs, CDWordArray *paryRemovedIDs)
{
	//Given:
	//	arySelectedData -- Currently selected IDs
	//	aryPreviousIDs -- Previously selected IDs
	//We need to find the "difference" between the 2, filling:
	//	aryNewIDs -- All IDs that are in "selected" and not "previous"
	//	aryRemovedIDs -- All IDs that are in "previous" and not "selected"

	//This might be faster in a map, but your average use of hotspots is going to end up
	//	with only very few clicked (maybe 10 in an extreme case), so the overhead of creating
	//	maps and moving data might make it worse.

	//Iteration #1 -- Find all the new ids
	for(int i = 0; i < parySelectedData->GetSize(); i++) {
		long dwSelected = (long)parySelectedData->GetAt(i);

		//Now look for it in the prev array
		bool bFound = false;
		for(size_t j = 0; j < paryPreviousIDs->size() && !bFound; j++) {
			long dwPrev = paryPreviousIDs->at(j);

			if(dwSelected == dwPrev) {
				//Match
				bFound = true;
			}
		}

		//If we found it, do nothing -- this id hasn't changed
		//If we did not find it, add it to "new"
		if(!bFound) {
			paryNewIDs->Add(dwSelected);
		}
	}

	//Iteration #2 -- Find all the removed ids
	for(size_t i = 0; i < paryPreviousIDs->size(); i++) {
		long dwPrev = paryPreviousIDs->at(i);

		//Now look for it in the selected array
		bool bFound = false;
		for(int j = 0; j < parySelectedData->GetSize() && !bFound; j++) {
			long dwSelected = (long)parySelectedData->GetAt(j);

			if(dwPrev == dwSelected) {
				//Match
				bFound = true;
			}
		}

		//If we found it, do nothing -- this id hasn't changed
		//If we did not find it, add it to "removed"
		if(!bFound) {
			paryRemovedIDs->Add(dwPrev);
		}
	}
}


// (a.walling 2009-10-13 13:57) - PLID 36046 - Manager keeps track of all CEMNDetails
#ifdef WATCH_EMNDETAIL_REFCOUNTS
#pragma TODO("PLID 36046 - CEMNDetail reference counts will be tracked - should be disabled before finishing the item.")
CEMNDetailManager g_EMNDetailManager;
#endif

#ifdef LOG_ALL_EMNDETAIL_REFCOUNTS
#pragma TODO("PLID 36046 - CEMNDetail reference counts will be logged - should be disabled before finishing the item.")
#endif

// (a.walling 2010-03-12 09:03) - PLID 37640 - Moved CEMNDetail creation to cpp
#if defined(WATCH_EMNDETAIL_REFCOUNTS) || defined(LOG_ALL_EMNDETAIL_REFCOUNTS)
CEMNDetail* CEMNDetail::__CreateDetail(CEMRTopic* pParentTopic, char* szInitialRefDescription) {
	return new CEMNDetail(pParentTopic, szInitialRefDescription, TRUE);
};

CEMNDetail* CEMNDetail::__CreateDetail(CEMRTopic* pParentTopic, char* szInitialRefDescription, BOOL bOwnTopicIfNull) {
	return new CEMNDetail(pParentTopic, szInitialRefDescription, bOwnTopicIfNull);
};
#else
CEMNDetail* CEMNDetail::__CreateDetail(CEMRTopic* pParentTopic) {
	return new CEMNDetail(pParentTopic, TRUE);
};

CEMNDetail* CEMNDetail::__CreateDetail(CEMRTopic* pParentTopic, BOOL bOwnTopicIfNull) {
	return new CEMNDetail(pParentTopic, bOwnTopicIfNull);
};
#endif

// (j.jones 2010-03-08 11:52) - PLID 37594 - added a critical section
// for reference counting
CCriticalSection g_EMNDetailRefCount;

void CEMNDetail::Internal__AddRef() {
	// (a.walling 2010-06-16 19:07) - PLID 39204 - Simple change to fix these critical section locks.
	// Previously: CSingleLock(&g_EMNDetailRefCount, TRUE);
	// which acquires and releases the lock in that single statement. Need a variable to keep alive in the scope.
	CSingleLock lock(&g_EMNDetailRefCount, TRUE);

	m_nRefCnt++; 
};

void CEMNDetail::Internal__Release() {
	// (a.walling 2010-06-16 19:07) - PLID 39204 - Simple change to fix these critical section locks.
	// Previously: CSingleLock(&g_EMNDetailRefCount, TRUE);
	// which acquires and releases the lock in that single statement. Need a variable to keep alive in the scope.
	CSingleLock lock(&g_EMNDetailRefCount, TRUE);

	_ASSERTE(m_nRefCnt >= 0 && m_nRefCnt < 0xffff); // if this trips, either something is wrong (maybe already deleted?) OR this detail has a bunch of valid ref counts and this line should be changed

	m_nRefCnt--;

	// (j.jones 2010-03-08 10:04) - PLID 37594 - For SmartStamps,
	// the image & table each point to each other. If there is
	// only one reference remaining, see if it is a SmartStamp
	// link, and if so, disconnect it.
	// (z.manning 2011-01-20 10:43) - PLID 42338 - This now supports more than one smart stamp image per table
	CEMNDetailArray arySmartStampReferences;
	if(m_nRefCnt == 1) {
		if(m_arySmartStampImageDetails.GetCount() > 0) {
			//this is a table, disconnect its pointer from the linked image,
			//without releasing it just yet
			arySmartStampReferences.Copy(m_arySmartStampImageDetails);
			m_arySmartStampImageDetails.RemoveAll();
		}
		else if(m_pSmartStampTableDetail) {
			//this is an image, disconnect its pointer from the linked table,
			//without releasing it just yet
			arySmartStampReferences.Add(m_pSmartStampTableDetail);
			m_pSmartStampTableDetail = NULL;
		}
	}
	
	if(m_nRefCnt == 0) { 
		delete this;
	}
	else if(m_nRefCnt == 1 && arySmartStampReferences.GetCount() > 0) {
		//release our reference to our connected detail, which
		//should itself release & delete us when it is finished
		arySmartStampReferences.ReleaseAll(FormatString("CEMNDetail::Internal__Release() Releasing reference to SmartStamp detail (this = 0x%08x)", (long)this));
	}
};

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace MSINKAUTLib;

// (a.walling 2010-03-08 09:46) - PLID 37640 - moved non-CEMNDetail stuff to EMNDetailStructure.cpp/.h

/////////////////////////////////////////////////////////////////////////////////////////////////////

	// (a.walling 2009-10-23 10:51) - PLID 36046 - Track construction in initial reference count
	// (j.jones 2013-05-16 11:02) - PLID 56596 - m_CommonLists and m_aryImageHotSpots are now references
#if defined(WATCH_EMNDETAIL_REFCOUNTS) || defined(LOG_ALL_EMNDETAIL_REFCOUNTS)
CEMNDetail::CEMNDetail(CEMRTopic* pParentTopic, char* szInitialRefDescription, BOOL bOwnTopicIfNull)
#else
CEMNDetail::CEMNDetail(CEMRTopic* pParentTopic, BOOL bOwnTopicIfNull)
#endif
 : m_CommonLists(*(new CEmrInfoCommonListCollection())),
	m_aryImageHotSpots(*(new CEMRHotSpotArray())),
	m_aisState(new CEmrItemAdvImageState())
{	
	// (a.walling 2009-10-13 13:57) - PLID 36046 - Track this detail
#ifdef WATCH_EMNDETAIL_REFCOUNTS
	g_EMNDetailManager.RegisterDetail(this);
#endif

	
	// (a.walling 2009-10-23 10:51) - PLID 36046 - Track construction in initial reference count
#if defined(WATCH_EMNDETAIL_REFCOUNTS) || defined(LOG_ALL_EMNDETAIL_REFCOUNTS)
	m_nRefCnt = 0;
	__AddRef(szInitialRefDescription);
#else
	// (c.haag 2007-05-22 11:52) - PLID 26095 - Reset our reference count
	m_nRefCnt = 1;
#endif

	// (c.haag 2007-08-02 17:40) - Details are not owned by a CEMNLoader object
	// by default
	m_bOwnedByEMNLoader = FALSE;

	// (c.haag 2007-08-06 11:58) - PLID 26954 - Default the spawner object to NULL
	m_pEMNSpawner = NULL;

	m_pEmrItemAdvDlg = NULL;
	m_nEMRInfoID = -1;
	m_nEMRInfoMasterID = -1;
	m_EMRInfoType = eitInvalid;
	m_EMRInfoSubType = eistNone;
	m_bTableRowsAsFields = FALSE; // (c.haag 2008-10-16 11:25) - PLID 31709
	m_nEMRDetailID = -1;
	m_nEMRTemplateDetailID = -1;
	m_bSaveTableColumnWidths = FALSE;
	m_bNeedToSyncContentAndState = FALSE;
	m_clrHighlight = 0;
	m_bCreatedOnNewEMN = FALSE;
	m_nReviewState = 0; // (b.cardillo 2006-11-20 10:37) - PLID 22565 - Init to 0 (no record to have a review state).
	m_bContentHasBeenLoaded = FALSE;

	// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
	m_nChildEMRInfoMasterID = -1;
	m_bSmartStampsEnabled = FALSE;
	m_nChildEMRDetailID = -1;
	m_nChildEMRTemplateDetailID = -1;

	//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
	m_bHasGlassesOrderData = FALSE;
	m_golLens = golInvalid;
	//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
	m_bHasContactLensData = FALSE;

	// (r.gonet 08/03/2012) - PLID 51948 - Can this table detail be used with Wound Care Coding?
	m_bUseWithWoundCareCoding = FALSE;

	// (j.jones 2010-02-12 09:29) - PLID 37318 - initialize the smart stamp pointers
	m_pSmartStampTableDetail = NULL;

	m_bInkHasBeenAdded = FALSE;
	m_bInkHasBeenErased = FALSE;
	m_bImageTextHasBeenAdded = FALSE;
	m_bImageTextHasBeenRemoved = FALSE;
	// (r.gonet 05/02/2012) - PLID 49946
	m_bImageTextHasBeenModified = FALSE;
	// (a.walling 2011-05-25 17:57) - PLID 43847 - This is now no longer a property of the detail.
	//m_bSizeImageToDetail = FALSE;

	// (j.jones 2013-08-07 16:12) - PLID 42958 - If a signature is added by another user,
	// these fields store that fact, and their username, until the initial audit is saved.
	// These are not filled when loading an EMN, they're only for new details.
	m_bIsSignatureAddedByAnotherUser = false;
	m_strSignatureAddedByAnotherUsername = "";

	if(pParentTopic) {
		m_pParentTopic = pParentTopic;
		m_pOriginalParentTopic = m_pParentTopic;
		m_strLastSavedParentTopicName = m_pOriginalParentTopic->GetName();
		m_bOwnTopic = FALSE;
	}
	else {
		m_pParentTopic = NULL;
		m_pOriginalParentTopic = NULL;
		if (bOwnTopicIfNull) {
			m_bOwnTopic = TRUE;
		} else {
			m_bOwnTopic = FALSE;
		}
	}

	m_bNeedReposition = TRUE;
	// (c.haag 2004-05-28 15:18) PL12289 - By default we don't need to refresh any
	// subcontrols (checkboxes or edit boxes or radio buttons). We only need to do
	// that if someone changes something from the EMR add/items dialog or changes
	// item content.
	m_bNeedToLoadContent = TRUE;
	// (j.jones 2007-07-25 17:29) - PLID 26810 - set m_bIsForcedReload to FALSE
	// which indicates that our first load is the item creation, not a refresh
	m_bIsForcedReload = FALSE;
	m_bAllowEdit = FALSE;
	m_bNeedRefreshMergeButton = FALSE;
	m_rcDefaultClientArea.SetRect(0, 0, 0, 0);
	m_bReadOnly = FALSE;
	m_bVisible = TRUE;
	m_bModified = FALSE;
	m_bStayUnsavedPostLoad = FALSE;
	m_emsf = emsfText;
	m_bMergeNameConflicts = FALSE;
	//DRT 8/2/2007 - PLID 26919
	m_nSourceActionSourceID = -1;
	//DRT 8/14/2007 - PLID 27067 - Added m_nSourceActionSourceDataGroupID
	m_nSourceActionSourceDataGroupID = -1;
	m_nSourceActionSourceHotSpotGroupID = -1;
	m_nSourceActionSourceTableDropdownGroupID = -1; // (z.manning 2009-02-13 09:57) - PLID 33070
	m_pCopiedFromDetail = NULL;
	m_SourceActionDestType = eaoInvalid;
	m_nEMRSourceTemplateID = -1;
	m_bAddingListElements = FALSE;
	m_strLongForm = "";
	m_nDataFormat = 0;
	m_strDataSeparator = "";
	m_strDataSeparatorFinal = "";

	m_bAllowUnspawn = TRUE;
	// (m.hancock 2006-06-06 12:53) - PLID 20519 - Keep the detailID of the source when copying a detail
	// (j.jones 2009-04-10 09:23) - PLID 33956 - renamed from m_nSourceEMRDetailID to m_nCopiedFromEMRDetailID
	m_nCopiedFromEMRDetailID = -1;
	m_bIsActiveInfo = FALSE;
	m_bIsActiveInfoValid = FALSE;
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	// (c.haag 2007-04-04 13:46) - PLID 25495 - By default, we maintain our own merge fields independently
	//m_bUsingParentEMNMergeFieldMap = FALSE;

	//TES 4/5/2007 - PLID 25456 - Our PopupDlg, if it's currently popped up.
	m_pPopupDlg = NULL;	

	m_eHasInfoActions = ehiasUndetermined;

	// (j.jones 2007-10-02 17:25) - PLID 26810 - added the EMRInfoT.Name
	m_strLabelText = "";

	m_bLinkedDetailsCached = false;

	// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
	// because they are now only calculated in the API, and not in Practice code
	/*
	// (j.jones 2007-08-27 10:36) - PLID 27056 - added fields for E/M coding
	m_nEMCodeCategoryID = -1;
	// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_eEMCodeUseTableCategories
	m_eEMCodeUseTableCategories = emcutcNone;
	m_bUseEMCoding = FALSE;
	m_emctEMCodingType = emctUndefined;
	*/

	// (b.savon 2012-06-06 12:20) - PLID 49144 - Keep a flag
	m_bDeviceImport = FALSE;

	// (a.walling 2007-12-17 15:49) - PLID 28391
	m_bHTMLVisiblePrint = TRUE;
	m_bHTMLVisible = TRUE;

	// (c.haag 2008-06-05 13:25) - PLID 27240 - Added field that tells us whether the detail
	// was brought up to date
	m_bDetailWasBroughtUpToDate = FALSE;

	//TES 6/24/2008 - PLID 29416 - Added field that tells us if this detail has loaded the "official" information, for
	// system details (Allergies, Current Medications)
	m_bHasLoadedSystemInfo = FALSE;

	// (a.walling 2008-06-30 12:36) - PLID 29271 - Preview Pane flags
	m_nPreviewFlags = 0;

	// (z.manning 2008-10-07 15:50) - PLID 31561 - Lab ID
	m_varLabID.vt = VT_NULL;
	
	// (a.walling 2009-01-13 13:53) - PLID 32107 - Initialize the image background info
	m_varInfoBackgroundImageFilePath.vt = VT_EMPTY;
	m_varInfoBackgroundImageType.vt = VT_EMPTY;

	//TES 9/16/2009 - PLID 35529 - We're not windowless by default
	m_bWindowless = false;

	// (a.walling 2009-11-17 12:02) - PLID 36365
	m_bUpdatingNarrativeFields = false;

	m_bIsPoppedUp = FALSE;

	m_bLoadedDetailImageStampsFromVariant = FALSE;

	// (r.gonet 06/10/2011) - PLID 30359 - Initialize the state change flag to be that we are not currently changing the state
	m_bStateChanging = false;

	m_nItemAdvDlgScalingFactor = 100; // (z.manning 2011-09-23 14:45) - PLID 42648

	m_nInfoFlags = 0; // (z.manning 2011-11-15 17:05) - PLID 38130
	m_nRememberedDetailID = -1; // (z.manning 2011-11-16 12:36) - PLID 38130

	m_pAdvTableBaseOverride = NULL;

	// (a.wilson 2013-03-26 10:43) - PLID 55826 - default false for stamp exclusions.
	m_bStampExclusionsLoaded = false;

	m_strLastWarnedInvalidImageFilePath = "";
}

CEMNDetail::~CEMNDetail()
{
	ClearContent();

	// (c.haag 2008-07-22 15:47) - PLID 30779 - Clear problems here
	// (c.haag 2009-05-19 11:52) - PLID 34311 - Now clear problem links
	for(int i = 0; i < m_apEmrProblemLinks.GetSize(); i++) {
		delete m_apEmrProblemLinks[i];
	}
	m_apEmrProblemLinks.RemoveAll();

	m_aryImageHotSpots.Clear();

	m_arypImageStamps.Clear();
	m_arynDeletedImageStampIDs.RemoveAll();

	//TES 9/16/2009 - PLID 35529 - If we're windowless, then we don't own m_pEmrItemAdvDlg
	if (m_pEmrItemAdvDlg && !m_bWindowless) {
		if (m_pEmrItemAdvDlg->m_hWnd) {
			m_pEmrItemAdvDlg->DestroyWindow();
		}
		// (a.walling 2012-03-05 15:56) - PLID 46075 - Auto deletes when destroyed
		//delete m_pEmrItemAdvDlg;
		m_pEmrItemAdvDlg = NULL;
	}

	if(m_bOwnTopic) {
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

	// (j.jones 2010-03-02 11:33) - PLID 37594 - handle reference counts
	// (z.manning 2011-01-20 10:45) - PLID 42338 - Support multiple images per table
	if(m_arySmartStampImageDetails.GetCount() > 0) {		
		m_arySmartStampImageDetails.ReleaseAll(FormatString("~CEMNDetail m_arySmartStampImageDetails (this table = 0x%08x)", (long)this));
	}
	if(m_pSmartStampTableDetail) {
		m_pSmartStampTableDetail->__Release(FormatString("~CEMNDetail m_pSmartStampTableDetail (this image = 0x%08x)", (long)this));
		m_pSmartStampTableDetail = NULL;
	}

	// (c.haag 2007-03-29 17:00) - PLID 25423 - Now that merge fields are
	// allocated, we need to deallocate them all in this destructor
	// (c.haag 2007-04-03 17:47) - PLID 25488 - Traverse a map rather than an array
	// (c.haag 2007-05-10 13:07) - PLID 25958 - We now use a recordset, and the
	// data will self-delete when the recordset is released
	
	// (a.walling 2009-10-13 13:57) - PLID 36046 - Stop tracking this detail
#ifdef WATCH_EMNDETAIL_REFCOUNTS
	g_EMNDetailManager.UnregisterDetail(this);
#endif

	// (j.jones 2013-05-16 11:03) - PLID 56596 - Needs to destroy the m_CommonLists and m_aryImageHotSpots references.
	// They are never null. They have to have been filled in the constructor.
	delete &m_CommonLists;
	delete &m_aryImageHotSpots;

	EMRLOG("* CEMNDetail (0x%08x) deleted", this); // (c.haag 2010-05-19 9:04) - PLID 38759
}

#ifdef _DEBUG
void CEMNDetail::DebugReportMembers(CEmrDebugDlg* pDlg, BOOL bOnlyInclude8300Fields, BOOL bAllowAddresses)
{
	// (c.haag 2007-08-07 11:45) - PLID 26946 - This is used for debugging and developer testing with EMR
	// (c.haag 2007-09-17 11:34) - PLID 27401 - We now do output through the dialog
	// (c.haag 2007-09-17 13:07) - PLID 27408 - We now know whether to include address data
	pDlg->Output(FormatString("Report for detail ID %d (Template ID %d) - %s:\n", m_nEMRDetailID,
		m_nEMRTemplateDetailID, m_strLabelText));
	pDlg->Output(FormatString("Parent Topic = %s (ID=%d)\n", (m_pParentTopic) ? m_pParentTopic->GetName() : "NULL", (m_pParentTopic) ? m_pParentTopic->GetID() : -1));
	if(bAllowAddresses) {
		pDlg->Output(FormatString("this = 0x%08x\n", this));
	}
	pDlg->Output(FormatString("m_nEMRDetailID = %d\n", m_nEMRDetailID));
	pDlg->Output(FormatString("m_nEMRTemplateDetailID = %d\n", m_nEMRTemplateDetailID));
	pDlg->Output(FormatString("m_nEMRSourceTemplateID = %d\n", m_nEMRSourceTemplateID));
	pDlg->Output(FormatString("m_nCopiedFromEMRDetailID = %d\n", m_nCopiedFromEMRDetailID));
	pDlg->Output(FormatString("m_nSourceDetailID = %d\n", m_sai.nSourceDetailID));
	pDlg->Output(FormatString("m_nEMRInfoID = %d\n", m_nEMRInfoID));
	pDlg->Output(FormatString("m_nEMRInfoMasterID = %d\n", m_nEMRInfoMasterID));
	pDlg->Output(FormatString("m_bReadOnly = %d\n", m_bReadOnly));
	pDlg->Output(FormatString("m_bVisible = %d\n", m_bVisible));
	pDlg->Output(FormatString("m_bIsTemplateDetail = %d\n", m_bIsTemplateDetail));
	pDlg->Output(FormatString("m_bCreatedOnNewEMN = %d\n", m_bCreatedOnNewEMN));
	pDlg->Output(FormatString("m_bContentHasBeenLoaded = %d\n", m_bContentHasBeenLoaded));
	pDlg->Output(FormatString("m_nReviewState = %d\n", m_nReviewState));
	pDlg->Output(FormatString("m_bSaveTableColumnWidths = %d\n", m_bSaveTableColumnWidths));
	pDlg->Output(FormatString("m_strLabelText = %s\n", m_strLabelText));
	pDlg->Output(FormatString("m_strSentenceHTML = %s\n", m_strSentenceHTML));
	pDlg->Output(FormatString("m_strSentenceNonHTML = %s\n", m_strSentenceNonHTML));
	pDlg->Output(FormatString("m_strSeparator = %s\n", m_strSeparator));
	pDlg->Output(FormatString("m_strSeparatorFinal = %s\n", m_strSeparatorFinal));
	pDlg->Output(FormatString("m_varState.vt = %d\n", m_varState.vt));
	pDlg->Output(FormatString("m_strLastSavedParentTopicName = %s\n", m_strLastSavedParentTopicName));
	pDlg->Output(FormatString("m_bOwnTopic = %d\n", m_bOwnTopic));
	pDlg->Output(FormatString("m_bNeedToSyncContentAndState = %d\n", m_bNeedToSyncContentAndState));
	pDlg->Output(FormatString("m_bModified = %d\n", m_bModified));
	pDlg->Output(FormatString("m_bMergeNameConflicts = %d\n", m_bMergeNameConflicts));
	pDlg->Output(FormatString("m_bIsActiveInfo = %d\n", m_bIsActiveInfo));
	pDlg->Output(FormatString("m_bIsActiveInfoValid = %d\n", m_bIsActiveInfoValid));
	pDlg->Output(FormatString("m_strMergeFieldOverride = %s\n", m_strMergeFieldOverride));
	pDlg->Output(FormatString("m_strLongForm = %s\n", m_strLongForm));
	pDlg->Output(FormatString("m_nDataFormat = %d\n", m_nDataFormat));
	pDlg->Output(FormatString("m_strDataSeparator = %s\n", m_strDataSeparator));
	pDlg->Output(FormatString("m_strDataSeparatorFinal = %s\n", m_strDataSeparatorFinal));
	pDlg->Output(FormatString("m_bNeedToLoadContent = %d\n", m_bNeedToLoadContent));
	pDlg->Output(FormatString("m_bAllowUnspawn = %d\n", m_bAllowUnspawn));
	pDlg->Output(FormatString("m_varLabID = %s\n", m_varLabID.vt == VT_I4 ? AsString(m_varLabID) : "NULL"));

	// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
	if(bAllowAddresses) {
		// (z.manning 2011-01-20 10:49) - PLID 42338 - Support multiple images per table
		for(int nImageIndex = 0; nImageIndex < m_arySmartStampImageDetails.GetCount(); nImageIndex++) {
			pDlg->Output(FormatString("m_arySmartStampImageDetails[%d] = 0x%08x\n", nImageIndex, m_arySmartStampImageDetails.GetAt(nImageIndex)));
		}
		pDlg->Output(FormatString("m_pSmartStampTableDetail = 0x%08x\n", m_pSmartStampTableDetail));
	}
	pDlg->Output(FormatString("m_nChildEMRInfoMasterID = %li\n", m_nChildEMRInfoMasterID));
	pDlg->Output(FormatString("m_bSmartStampsEnabled = %li\n", m_bSmartStampsEnabled ? 1: 0));
	pDlg->Output(FormatString("m_nChildEMRDetailID = %li\n", m_nChildEMRDetailID));
	pDlg->Output(FormatString("m_nChildEMRTemplateDetailID = %li\n", m_nChildEMRTemplateDetailID));

	//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
	pDlg->Output(FormatString("m_bHasGlassesOrderData = %li\n", m_bHasGlassesOrderData ? 1 : 0));
	pDlg->Output(FormatString("m_golLens = %i\n", m_golLens));
	//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
	pDlg->Output(FormatString("m_bHasContactLensData = %li\n", m_bHasContactLensData ? 1 : 0));
	// (r.gonet 08/03/2012) - PLID 51948 - Can this detail be used with Wound Care Coding
	pDlg->Output(FormatString("m_bUseWithWoundCareCoding = %li\n", m_bUseWithWoundCareCoding ? 1 : 0));

	// (z.manning 2011-11-15 17:05) - PLID 38130
	pDlg->Output(FormatString("m_nInfoFlags = %li\n", m_nInfoFlags));
	pDlg->Output(FormatString("m_nRememberedDetailID = %li\n", m_nRememberedDetailID)); // (z.manning 2011-11-16 12:36) - PLID 38130

	// (j.jones 2008-07-18 11:06) - PLID 30779 - added problem class, and an array to track problems
	// (c.haag 2009-05-19 11:53) - PLID 34311 - Use new problem structure
	for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
		CEmrProblemLink* pLink = m_apEmrProblemLinks[i];
		CEmrProblem *pProblem = pLink->GetProblem();
		if(pProblem) {
			pDlg->Output(FormatString("Problem %li - m_nID = %li\n", i+1, pProblem->m_nID));
			pDlg->Output(FormatString("Problem %li - m_strDescription = %s\n", i+1, pProblem->m_strDescription));
			pDlg->Output(FormatString("Problem %li - m_nStatusID = %li\n", i+1, pProblem->m_nStatusID));
			pDlg->Output(FormatString("Problem %li - m_eprtTypeID = %li\n", i+1, (long)pLink->GetType()));
			pDlg->Output(FormatString("Problem %li - m_nEMRRegardingID = %li\n", i+1, pLink->GetRegardingID()));
			// (c.haag 2008-08-15 12:48) - Adding m_nEmrProblemActionID
			pDlg->Output(FormatString("Problem %li - m_nEmrProblemActionID = %li\n", i+1, pProblem->m_nEmrProblemActionID));
		}
	}

	if (!bOnlyInclude8300Fields) {
		// (j.jones 2007-07-25 17:29) - PLID 26810 - added m_bIsForcedReload
		pDlg->Output(FormatString("m_bIsForcedReload = %d\n", m_bIsForcedReload));
		// (j.jones 2007-07-18 12:16) - PLID 26730
		pDlg->Output(FormatString("m_eHasInfoActions = %d\n", m_eHasInfoActions));
		// (a.walling 2007-08-09 11:24) - PLID 26996
		// (c.haag 2007-09-17 13:06) - PLID 27408 - Suppress if we don't allow addresses
		if (bAllowAddresses) {
			pDlg->Output(FormatString("m_pEmrItemAdvDlg = 0x%08x\n", m_pEmrItemAdvDlg));
		}
		pDlg->Output(FormatString("m_rcDefaultClientArea = (%d, %d, %d, %d)\n", m_rcDefaultClientArea.top, m_rcDefaultClientArea.left, m_rcDefaultClientArea.bottom, m_rcDefaultClientArea.right));
		// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
		// because they are now only calculated in the API, and not in Practice code
		/*
		// (j.jones 2007-08-27 10:35) - PLID 27056 - added the E/M coding data
		pDlg->Output(FormatString("m_nEMCodeCategoryID = %li\n", m_nEMCodeCategoryID));
		// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_eEMCodeUseTableCategories
		pDlg->Output(FormatString("m_eEMCodeUseTableCategories = %li\n", (long)m_eEMCodeUseTableCategories));
		pDlg->Output(FormatString("m_bUseEMCoding = %li\n", m_bUseEMCoding ? 1 : 0));
		pDlg->Output(FormatString("m_emctEMCodingType = %li\n", (long)m_emctEMCodingType));
		*/
		//TES 3/17/2010 - PLID 37530
		pDlg->Output(FormatString("m_nSourceStampID = %d\n", m_sai.GetStampID()));
		pDlg->Output(FormatString("m_nSourceStampIndex = %d\n", m_sai.GetStampIndexInDetailByType()));
	}
	pDlg->Output("\n");
}
#endif

// (a.walling 2009-10-13 13:57) - PLID 36046 - Ability to keep reference count history and dump
#ifdef WATCH_EMNDETAIL_REFCOUNTS
void CEMNDetailManager::RegisterDetail(CEMNDetail* newDetail) {
	CSingleLock sl(&m_cs, TRUE);

	m_mapAllDetails.SetAt(newDetail, TRUE);
}

void CEMNDetailManager::UnregisterDetail(CEMNDetail* dyingDetail) {
	CSingleLock sl(&m_cs, TRUE);

	BOOL bFound = m_mapAllDetails.RemoveKey(dyingDetail);

	ASSERT(bFound);
}

DWORD CEMNDetailManager::GetCount() {
	CSingleLock sl(&m_cs, TRUE);

	return m_mapAllDetails.GetSize();
}

void CEMNDetailManager::DumpAll() {
	CSingleLock sl(&m_cs, TRUE);
	
	CString strOut;
	strOut.Format("*Total details: %li\n", m_mapAllDetails.GetCount());
	::OutputDebugString(strOut);
	
	POSITION pos = m_mapAllDetails.GetStartPosition();

	while (pos) {
		CEMNDetail* pDetail;
		BOOL bIgnored;

		m_mapAllDetails.GetNextAssoc(pos, pDetail, bIgnored);

		strOut.Format("*Existing detail 0x%08x (ID %li, '%s'):\n", pDetail, pDetail->GetID(), pDetail->GetLabelText());
		::OutputDebugString(strOut);

		pDetail->DumpRefCountHistory();
	}
}
#endif

// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
ADODB::_ConnectionPtr CEMNDetail::GetRemoteData()
{
	if (m_pParentTopic) {
		return m_pParentTopic->GetRemoteData();
	} else {
		return ::GetRemoteData();
	}
}

// (a.walling 2010-04-01 10:31) - PLID 38013 - Added functions to get the parent topic, EMN, EMR
CEMRTopic* CEMNDetail::GetParentTopic()
{
	return m_pParentTopic;
}

CEMRTopic* CEMNDetail::GetOriginalParentTopic()
{
	return m_pOriginalParentTopic;
}

CEMN* CEMNDetail::GetParentEMN()
{
	return m_pParentTopic ? m_pParentTopic->GetParentEMN() : NULL;
}

CEMR* CEMNDetail::GetParentEMR()
{
	return GetParentEMN() ? GetParentEMN()->GetParentEMR() : NULL;
}

// (a.walling 2010-04-01 10:31) - PLID 38013 - Added function to get the top-level parent topic
CEMRTopic* CEMNDetail::GetTopLevelParentTopic()
{
	return m_pParentTopic ? m_pParentTopic->GetTopLevelParentTopic() : NULL;
}


// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

// (j.jones 2007-07-25 10:13) - PLID 26803 - converted to just call LoadFromInfoOrMasterID
//DRT 8/2/2007 - PLID 26919 - Added nSourceActionSourceID
//DRT 8/14/2007 - PLID 27067 - Added nSourceActionSourceDataGroupID
//TES 10/8/2007 - PLID 27660 - Added bIsInitialLoad.
// (z.manning 2009-02-13 09:58) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
// (z.manning 2009-03-11 09:47) - PLID 33338 - Use the new source action info class
// (j.jones 2010-06-21 11:36) - PLID 37981 - added ability to pass in generic table content
// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
void CEMNDetail::LoadFromInfoID(long nLoadFromInfoID, BOOL bIsOnTemplate, BOOL bIsInitialLoad, SourceActionInfo &sai, OPTIONAL IN long nSourceActionSourceID /*= -1*/, OPTIONAL IN long nSourceActionSourceDataGroupID /*= -1*/, long nSourceActionSourceHotSpotGroupID /* = -1 */, long nSourceActionSourceTableDropdownGroupID /* = -1 */, DevicePluginUtils::TableContent *pGenericTableContent /*= NULL*/) {

	LoadFromInfoOrMasterID(nLoadFromInfoID, -1, bIsOnTemplate, bIsInitialLoad, sai, nSourceActionSourceID, NULL, nSourceActionSourceDataGroupID, nSourceActionSourceHotSpotGroupID, nSourceActionSourceTableDropdownGroupID, pGenericTableContent);
}

// (j.jones 2007-07-25 10:13) - PLID 26803 - converted to just call LoadFromInfoOrMasterID
//DRT 8/2/2007 - PLID 26919 - Added nSourceActionSourceID
// (c.haag 2007-08-06 15:26) - PLID 26992 - Added an optional CEMNSpawner parameter for query-free accesses to EMR info data
//DRT 8/14/2007 - PLID 27067 - Added nSourceActionSourceDataGroupID
//TES 10/8/2007 - PLID 27660 - Added bIsInitialLoad.
// (z.manning 2009-02-13 09:58) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
// (z.manning 2009-03-11 09:47) - PLID 33338 - Use the new source action info class
// (j.jones 2010-06-21 11:36) - PLID 37981 - added ability to pass in generic table content
// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
void CEMNDetail::LoadFromInfoMasterID(long nLoadFromInfoMasterID, BOOL bIsOnTemplate, BOOL bIsInitialLoad, SourceActionInfo &sai, OPTIONAL IN long nSourceActionSourceID /*= -1*/, CEMNSpawner* pEMNSpawner /*= NULL */, OPTIONAL IN long nSourceActionSourceDataGroupID /*= -1*/, long nSourceActionSourceHotSpotGroupID /* = -1 */, long nSourceActionSourceTableDropdownGroupID /* = -1 */, DevicePluginUtils::TableContent *pGenericTableContent /*= NULL*/)
{
	//TES 12/7/2006 - PLID 23724 - Load the Active info record.
	LoadFromInfoOrMasterID(-1, nLoadFromInfoMasterID, bIsOnTemplate, bIsInitialLoad, sai, nSourceActionSourceID, pEMNSpawner, nSourceActionSourceDataGroupID, nSourceActionSourceHotSpotGroupID, nSourceActionSourceTableDropdownGroupID, pGenericTableContent);
}

// (j.jones 2007-07-25 10:03) - PLID 26803 - created to handle taking in an InfoID or a MasterID, basically moving
// the contents of LoadFromInfoID into this function
//DRT 8/2/2007 - PLID 26919 - Added nSourceActionSourceID
// (c.haag 2007-08-06 15:26) - PLID 26992 - Added an optional CEMNSpawner parameter for query-free accesses to EMR info data
//DRT 8/14/2007 - PLID 27067 - Added nSourceActionSourceDataGroupID
//TES 10/8/2007 - PLID 27660 - Added bIsInitialLoad; this function calls ProcessEmrActions(), therefore it needs to know whether
// we are in the initial load or not.
// (z.manning 2009-02-13 10:00) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
// (z.manning 2009-03-11 09:48) - PLID 33338 - Use the new source action info class
// (j.jones 2010-06-21 11:36) - PLID 37981 - added ability to pass in generic table content
// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
void CEMNDetail::LoadFromInfoOrMasterID(long nLoadFromInfoID, long nLoadFromInfoMasterID, BOOL bIsOnTemplate, BOOL bIsInitialLoad, SourceActionInfo &sai, OPTIONAL IN long nSourceActionSourceID /*= -1*/, CEMNSpawner* pEMNSpawner /*= NULL */, OPTIONAL IN long nSourceActionSourceDataGroupID /*= -1*/, long nSourceActionSourceHotSpotGroupID /* = -1 */, long nSourceActionSourceTableDropdownGroupID /* = -1 */, DevicePluginUtils::TableContent *pGenericTableContent /*= NULL*/)
{
	//this should never be called without at least one of the IDs filled in
	ASSERT(nLoadFromInfoID != -1 || nLoadFromInfoMasterID != -1);

	if(nLoadFromInfoID != -1)
		m_nEMRInfoID = nLoadFromInfoID;
	//if -1, we'll fill it in later

	m_bIsTemplateDetail = bIsOnTemplate;
	if(m_bOwnTopic && !m_pParentTopic) {
		//TES 4/15/2010 - PLID 24692 - We know this is a new topic, so give it a new entry in the linked list.
		//TES 5/3/2010 - PLID 24692 - For "bottom-up" loading, we don't maintain position entries.
		m_pParentTopic = new CEMRTopic(m_bIsTemplateDetail, TRUE, NULL);
		m_pOriginalParentTopic = m_pParentTopic;
		if(m_pOriginalParentTopic) {
			m_strLastSavedParentTopicName = m_pOriginalParentTopic->GetName();
		}
	}

	// (j.jones 2007-07-25 10:16) - PLID 26803 - alter the where clause based on whether we have
	// an nInfoID or nInfoMasterID
	// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
	CSqlFragment strWhere;
	if(nLoadFromInfoID != -1)
		strWhere.Create("WHERE EmrInfoT.ID = {INT}", nLoadFromInfoID);
	else if(nLoadFromInfoMasterID != -1)
		strWhere.Create("INNER JOIN EmrInfoMasterT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID WHERE EmrInfoMasterT.ID = {INT}", nLoadFromInfoMasterID);
	else {
		//should be impossible
		ASSERT(FALSE);
	}

	//TES 12/7/2006 - PLID 23724 - We also need to load the EmrInfoMasterID.
	// (j.jones 2007-07-18 10:20) - PLID 26730 - load whether or not the info item has Info actions,
	// which it usually does not, such that we don't have to search for them later

	// (c.haag 2007-08-06 15:36) - PLID 26992 - Try to use the data in CEMNSpawner since we already loaded it first
	CEMNSpawner::CEMRInfoItem* pInfoItem = NULL;
	if (NULL != pEMNSpawner) {
		if (-1 != nLoadFromInfoID) {
			pInfoItem = pEMNSpawner->GetEmrInfoItem(nLoadFromInfoID);
		} else if (-1 != nLoadFromInfoMasterID) {
			pInfoItem = pEMNSpawner->GetEmrInfoItemByMasterID(nLoadFromInfoMasterID);
		}
	}
	
	// (j.jones 2007-08-01 14:12) - PLID 26905 - added more data to this query that we can pass into
	// LoadEMRDetailStateDefault() later on
	// (c.haag 2007-08-06 15:39) - PLID 26992 - We only use rsInfo in the event that we do not have a spawner
	// object available	
	_RecordsetPtr rsInfo;
	
	if (NULL != pInfoItem) {
		m_nEMRInfoID = pInfoItem->m_nID;
		m_nEMRInfoMasterID = pInfoItem->m_nMasterID;
		// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
		m_nChildEMRInfoMasterID = VarLong(pInfoItem->m_vChildEMRInfoMasterID, -1);
		m_bSmartStampsEnabled = VarBool(pInfoItem->m_vSmartStampsEnabled, FALSE);
		// (j.jones 2007-10-22 09:43) - PLID 27830 - don't assign directly to m_strLabelText, as SetLabelText properly handles narratives
		SetLabelText(VarString(pInfoItem->m_vName));
		m_EMRInfoType = (EmrInfoType)VarByte(pInfoItem->m_vDataType);
		m_EMRInfoSubType = (EmrInfoSubType)VarByte(pInfoItem->m_vDataSubType);
		m_bTableRowsAsFields = VarBool(pInfoItem->m_vTableRowsAsFields); // (c.haag 2008-10-16 11:23) - PLID 31709
		m_eHasInfoActions = VarLong(pInfoItem->m_vHasInfoActions, 0) == 1 ? ehiasHasInfoItems : ehiasHasNoInfoItems;
		m_nPreviewFlags = VarLong(pInfoItem->m_vPreviewFlags);
		m_nInfoFlags = VarLong(pInfoItem->m_vInfoFlags);

		//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
		m_bHasGlassesOrderData = VarBool(pInfoItem->m_vHasGlassesOrderData, FALSE);
		m_golLens = (GlassesOrderLens)VarLong(pInfoItem->m_vGlassesOrderLens, (long)golInvalid);
		//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
		m_bHasContactLensData = VarBool(pInfoItem->m_vHasContactLensData, FALSE);
		// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
		m_bUseWithWoundCareCoding = VarBool(pInfoItem->m_vUseWithWoundCareCoding, FALSE);

		// (a.walling 2009-01-13 13:54) - PLID 32107 - Load the info item's background info
		m_varInfoBackgroundImageFilePath = pInfoItem->m_vBackgroundImageFilePath;
		m_varInfoBackgroundImageType = pInfoItem->m_vBackgroundImageType;

		// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

	} else {
		//DRT 2/14/2008 - PLID 28698 - Added an optimization parameter for HotSpots.  However, this code can only ever be executed for
		//	loading details, which will never pull through the template's selection, so it's always 0 in this particular case.
		//TES 6/5/2008 - PLID 29416 - The system tables are hardcoded to display as "Remember"ing, but the data doesn't
		// have that flag set, so override the data in that case.
		// (j.jones 2008-09-22 14:53) - PLID 31408 - supported RememberForEMR, which is always disabled when allergy/current meds
		//TES 2/25/2010 - PLID 37535 - Added UseSmartStampsLongForm
		// (z.manning 2010-07-26 13:29) - PLID 39848 - Removed UseSmartStampsLongForm
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		//TES 3/17/2011 - PLID 41108 - Added HasGlassesOrderData and GlassesOrderLens
		// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
		//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
		rsInfo = CreateParamRecordset("SELECT EmrInfoT.ID, EmrInfoT.EmrInfoMasterID, EMRInfoT.Name, EmrInfoT.DataType, EmrInfoT.DataSubType, "
			"(CASE WHEN EmrInfoT.ID IN (SELECT SourceID FROM EMRActionsT WHERE Deleted = 0 AND SourceType = 3) THEN 1 ELSE 0 END) AS HasInfoActions, "
			"EmrInfoT.BackgroundImageFilePath AS Info_BackgroundImageFilePath, EmrInfoT.BackgroundImageType AS Info_BackgroundImageType, EmrInfoT.DefaultText AS Info_DefaultText, "
			// (j.jones 2010-06-21 12:22) - PLID 37981 - generic tables never remember their values
			"CASE WHEN EmrInfoT.DataSubType IN ({CONST}, {CONST}) THEN convert(bit,1) WHEN EmrInfoT.DataSubType = {CONST} THEN Convert(bit,0) "
			"	ELSE EmrInfoT.RememberForPatient END AS Info_RememberForPatient, "
			"CASE WHEN EmrInfoT.DataSubType IN ({CONST}, {CONST}, {CONST}) THEN convert(bit,0) "
			"	ELSE EmrInfoT.RememberForEMR END AS Info_RememberForEMR, "
			"EmrInfoT.SliderMin AS Info_SliderMin, EmrInfoT.SliderMax AS Info_SliderMax, EmrInfoT.SliderInc AS Info_SliderInc, "
			"CASE WHEN EmrInfoT.ID IN (SELECT EMRInfoDefaultsT.EMRInfoID FROM EMRInfoDefaultsT INNER JOIN EMRDataT ON EMRInfoDefaultsT.EMRDataID = EMRDataT.ID WHERE EMRDataT.Inactive = 0 AND EMRDataT.IsLabel = 0) THEN 1 ELSE 0 END AS Info_HasDefaultValue, "
			"NULL AS Template_HasDefaultValue, NULL AS DefaultText, NULL AS SliderValue, 0 AS HasSliderValue, 0 AS HasTemplateHotSpots, "
			// (a.walling 2008-06-30 13:38) - PLID 29271 - Preview Pane flags
			"EmrInfoT.PreviewFlags, "
			// (c.haag 2008-10-16 11:23) - PLID 31709 - TableRowsAsFields
			"EmrInfoT.TableRowsAsFields, "
			// (j.jones 2010-02-11 15:24) - PLID 37318 - load ChildEMRInfoMasterID and SmartStampsEnabled
			"EmrInfoT.ChildEMRInfoMasterID, EmrInfoT.SmartStampsEnabled, "
			"EmrInfoT.HasGlassesOrderData, EmrInfoT.GlassesOrderLens, EmrInfoT.InfoFlags, EmrInfoT.HasContactLensData, "
			// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
			"EmrInfoT.UseWithWoundCareCoding "
			"FROM EmrInfoT "
			"{SQL}",
			eistCurrentMedicationsTable, eistAllergiesTable, eistGenericTable,
			eistCurrentMedicationsTable, eistAllergiesTable, eistGenericTable,
			strWhere);
		//
		// (c.haag 2006-03-15 12:04) - PLID 19720 - Auto-create the built-in image if it's missing
		//
		if (EMR_BUILT_IN_INFO__IMAGE == nLoadFromInfoID && rsInfo->eof) {
			rsInfo->Close();
			rsInfo.Release();
			// (c.haag 2007-01-23 11:07) - PLID 24376 - supported EmrInfoT.DataSubType
			//DRT 2/14/2008 - PLID 28698 - Same hotspot addition as above.
			// (j.jones 2008-09-22 14:53) - PLID 31408 - supported RememberForEMR
			//TES 2/25/2010 - PLID 37535 - Added UseSmartStampsLongForm
			// (z.manning 2010-07-26 13:29) - PLID 39848 - Removed UseSmartStampsLongForm
			//TES 3/17/2011 - PLID 41108 - Added HasGlassesOrderData and GlassesOrderLens
			// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
			//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
			rsInfo = CreateParamRecordset("SELECT ID, EmrInfoMasterID, Name, DataType, DataSubType, "
				"0 AS HasInfoActions, BackgroundImageFilePath AS Info_BackgroundImageFilePath, "
				"BackgroundImageType AS Info_BackgroundImageType, DefaultText AS Info_DefaultText, "
				"RememberForPatient AS Info_RememberForPatient, "
				"RememberForEMR AS Info_RememberForEMR, "
				"SliderMin AS Info_SliderMin, SliderMax AS Info_SliderMax, SliderInc AS Info_SliderInc, "
				"0 AS Info_HasDefaultValue, NULL AS Template_HasDefaultValue, NULL AS DefaultText, NULL AS SliderValue, 0 AS HasSliderValue, 0 AS HasTemplateHotSpots, "
				// (a.walling 2008-06-30 13:38) - PLID 29271 - Preview Pane flags
				"EmrInfoT.PreviewFlags, "
				// (c.haag 2008-10-16 11:22) - PLID 31709 - TableRowsAsFields
				"EmrInfoT.TableRowsAsFields, "
				// (j.jones 2010-02-11 15:24) - PLID 37318 - load ChildEMRInfoMasterID and SmartStampsEnabled
				"EmrInfoT.ChildEMRInfoMasterID, EmrInfoT.SmartStampsEnabled, "
				"EmrInfoT.HasGlassesOrderData, EmrInfoT.GlassesOrderLens, EmrInfoT.InfoFlags, EmrInfoT.HasContactLensData, "
				// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
				"EmrInfoT.UseWithWoundCareCoding "
				"FROM EmrInfoT WHERE ID = {INT}", EMR_BUILT_IN_INFO__IMAGE);
			if (rsInfo->eof) {
				AfxThrowNxException("The image detail could not be created because the data used to create it is absent from your database.\n\nPlease contact NexTech Technical Support to resolve this issue.");
			}
		}
		// (c.haag 2008-06-16 10:54) - PLID 30319 - Support for the built-in text macro item
		else if (EMR_BUILT_IN_INFO__TEXT_MACRO == nLoadFromInfoID && rsInfo->eof) {
			rsInfo->Close();
			rsInfo.Release();
			// (j.jones 2008-09-22 14:53) - PLID 31408 - supported RememberForEMR
			//TES 2/25/2010 - PLID 37535 - Added UseSmartStampsLongForm
			// (z.manning 2010-07-26 13:30) - PLID 39848 - Removed UseSmartStampsLongForm
			//TES 3/17/2011 - PLID 41108 - Added HasGlassesOrderData and GlassesOrderLens
			// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
			//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
			rsInfo = CreateParamRecordset("SELECT ID, EmrInfoMasterID, Name, DataType, DataSubType, "
				"0 AS HasInfoActions, BackgroundImageFilePath AS Info_BackgroundImageFilePath, "
				"BackgroundImageType AS Info_BackgroundImageType, DefaultText AS Info_DefaultText, "
				"RememberForPatient AS Info_RememberForPatient, "
				"RememberForEMR AS Info_RememberForEMR, "
				"SliderMin AS Info_SliderMin, SliderMax AS Info_SliderMax, SliderInc AS Info_SliderInc, "
				"0 AS Info_HasDefaultValue, NULL AS Template_HasDefaultValue, NULL AS DefaultText, NULL AS SliderValue, 0 AS HasSliderValue, 0 AS HasTemplateHotSpots, "
				// (a.walling 2008-06-30 13:38) - PLID 29271 - Preview Pane flags
				"EmrInfoT.PreviewFlags, "
				// (c.haag 2008-10-16 11:21) - PLID 31709 - TableRowsAsFields
				"EmrInfoT.TableRowsAsFields, "
				// (j.jones 2010-02-11 15:24) - PLID 37318 - load ChildEMRInfoMasterID and SmartStampsEnabled
				"EmrInfoT.ChildEMRInfoMasterID, EmrInfoT.SmartStampsEnabled, "
				"EmrInfoT.HasGlassesOrderData, EmrInfoT.GlassesOrderLens, EmrInfoT.InfoFlags, EmrInfoT.HasContactLensData, "
				// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
				"EmrInfoT.UseWithWoundCareCoding "
				"FROM EmrInfoT WHERE ID = {INT}", EMR_BUILT_IN_INFO__TEXT_MACRO);
			if (rsInfo->eof) {
				AfxThrowNxException("The text detail could not be created because the data used to create it is absent from your database.\n\nPlease contact NexTech Technical Support to resolve this issue.");
			}
		}

		if(!rsInfo->eof) {

			// (j.jones 2007-07-25 10:18) - PLID 26803 - load the InfoID and InfoMasterID, even if we already have one or the other
			m_nEMRInfoID = AdoFldLong(rsInfo, "ID");
			m_nEMRInfoMasterID = AdoFldLong(rsInfo, "EmrInfoMasterID");

			// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
			m_nChildEMRInfoMasterID = AdoFldLong(rsInfo, "ChildEMRInfoMasterID", -1);
			m_bSmartStampsEnabled = AdoFldBool(rsInfo, "SmartStampsEnabled", FALSE);

			// (j.jones 2007-10-22 09:43) - PLID 27830 - don't assign directly to m_strLabelText, as SetLabelText properly handles narratives
			SetLabelText(AdoFldString(rsInfo, "Name"));
			m_EMRInfoType = (EmrInfoType)AdoFldByte(rsInfo, "DataType");
			m_EMRInfoSubType = (EmrInfoSubType)AdoFldByte(rsInfo, "DataSubType");	
			m_bTableRowsAsFields = AdoFldBool(rsInfo, "TableRowsAsFields"); // (c.haag 2008-10-16 11:23) - PLID 31709
						
			// (j.jones 2007-07-18 10:20) - PLID 26730 - load whether or not the info item has Info actions,
			// which it usually does not, such that we don't have to search for them later
			m_eHasInfoActions = AdoFldLong(rsInfo, "HasInfoActions", 0) == 1 ? ehiasHasInfoItems : ehiasHasNoInfoItems;

			// (a.walling 2008-06-30 13:39) - PLID 29271 - Preview Pane flags
			m_nPreviewFlags = AdoFldLong(rsInfo, "PreviewFlags", 0);
			
			// (a.walling 2009-01-13 13:54) - PLID 32107 - Load the info item's background info
			m_varInfoBackgroundImageFilePath = rsInfo->Fields->Item["Info_BackgroundImageFilePath"]->Value;
			m_varInfoBackgroundImageType = rsInfo->Fields->Item["Info_BackgroundImageType"]->Value;

			//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
			m_bHasGlassesOrderData = AdoFldBool(rsInfo, "HasGlassesOrderData", FALSE);
			m_golLens = (GlassesOrderLens)AdoFldLong(rsInfo, "GlassesOrderLens", (long)golInvalid);
			//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
			m_bHasContactLensData = AdoFldBool(rsInfo, "HasContactLensData", FALSE);
			// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
			m_bUseWithWoundCareCoding = AdoFldBool(rsInfo, "UseWithWoundCareCoding", FALSE);

			// (z.manning 2011-11-15 17:06) - PLID 38130
			m_nInfoFlags = AdoFldLong(rsInfo, "InfoFlags", 0);
		}
		else {
			//should be impossible now
			ASSERT(FALSE);
		}
	}
	
	ASSERT(m_pParentTopic); //This function should never be called for details that aren't attached to a topic.

	if(pGenericTableContent && IsGenericTable()) {
		//load from a generic table (this cannot be pre-loaded or spawned)		
		SetState(GenerateStateFromGenericTableContent(m_nEMRInfoID, pGenericTableContent));
	}
	else {
		// (c.haag 2007-08-03 13:16) - PLID 26929 - We also set the flag to true if this is a template detail
		BOOL bSyncWithCurrentMedications = IsCurrentMedicationsTable();
		if(bSyncWithCurrentMedications) {

			// (c.haag 2007-01-23 11:52) - PLID 24376 - If this is the system current medications table,
			// then its default state should reflect the patient's current medications
			// Developer note: GetActiveCurrentMedicationsInfoID() runs a small query. See if we can safely make
			// it more global some day.
			// (j.jones 2007-07-24 09:27) - PLID 26742 - that day is now - it's cached in CEMR
			long nActiveCurrentMedicationsInfoID = -2;
			//do memory checks
			if(m_pParentTopic) {
				if(m_pParentTopic->GetParentEMN()) {
					if(m_pParentTopic->GetParentEMN()->GetParentEMR()) {
						nActiveCurrentMedicationsInfoID = m_pParentTopic->GetParentEMN()->GetParentEMR()->GetCurrentMedicationsInfoID();
					}
				}
			}

			if(nActiveCurrentMedicationsInfoID == -2) {
				//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
				//but why don't we have an EMR?
				ASSERT(FALSE);
				nActiveCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID();
			}

			//now, compare, and update bSyncWithCurrentMedications
			bSyncWithCurrentMedications = nActiveCurrentMedicationsInfoID == m_nEMRInfoID;
		}

		if (bSyncWithCurrentMedications) {
			// (c.haag 2007-02-08 12:17) - PLID 24376 - If we are pulling an official Current Medications detail,
			// then we must calculate the state from the EMN itself. Similar code is run in these three places:
			//
			// When loading Current Medication info items into templates, we should not do any special calculating. It
			// should behave exactly as it did before.
			//
			// CEMNDetail::LoadFromInfoID()				- When adding a Current Medications detail to a topic manually
			// CEMNDetail::LoadFromTemplateDetailID()	- When adding a Current Medications detail from a template
			// CEMRTopic::PostLoadDetails()				- When loading a patient EMN from a template
			// LoadEmrTopic								- When loading detail states in a thread
			// CEMRTopic::AddDetail()					- The rest of the time (failsafe)
			//

			// (c.haag 2007-08-03 13:17) - PLID 26929 - If this is a template detail, assign this a blank state
			if (m_bIsTemplateDetail) {
				SetState(LoadEMRDetailStateBlank( eitTable ));
			} else {
				//TES 6/5/2008 - PLID 29416 - Rather than using the "official" current medications state (which never really
				// existed), just load the state like normal, then merge in the Medications tab data (which only affects one
				// column in the table).
				if (NULL != pInfoItem && NULL != pEMNSpawner) {
					// (j.jones 2008-09-22 15:26) - PLID 31408 - send in the EMR ID
					SetState(pEMNSpawner->LoadEMRDetailStateDefault(m_nEMRInfoID, m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), m_pParentTopic->GetParentEMN()->GetParentEMR()->GetID(), GetRemoteData(), m_nRememberedDetailID));
				} else {
					// (j.jones 2008-09-22 15:20) - PLID 31408 - send in the EMR ID
					SetState(LoadEMRDetailStateDefault(rsInfo, m_nEMRInfoID, m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), m_pParentTopic->GetParentEMN()->GetParentEMR()->GetID(), -1, GetRemoteData(), m_nRememberedDetailID));
				}
				m_pParentTopic->GetParentEMN()->ApplyOfficialCurrentMedications(this, m_nEMRInfoID);
			}
		} else {
			// (c.haag 2007-04-05 13:16) - PLID 25516 - If this is a system allergies table, then its default
			// state should reflect the patient's allergies.
			// (c.haag 2007-08-03 13:18) - PLID 26929 - We also set the flag to true if this is a template detail
			BOOL bSyncWithAllergies = IsAllergiesTable();

			if(bSyncWithAllergies) {

				// (j.jones 2007-07-24 09:27) - PLID 26742 - the Allergy Info ID is cached in CEMR
				long nActiveCurrentAllergiesInfoID = -2;
				//do memory checks
				if(m_pParentTopic) {
					if(m_pParentTopic->GetParentEMN()) {
						if(m_pParentTopic->GetParentEMN()->GetParentEMR()) {
							nActiveCurrentAllergiesInfoID = m_pParentTopic->GetParentEMN()->GetParentEMR()->GetCurrentAllergiesInfoID();
						}
					}
				}

				if(nActiveCurrentAllergiesInfoID == -2) {
					//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
					//but why don't we have an EMR?
					ASSERT(FALSE);
					nActiveCurrentAllergiesInfoID = GetActiveAllergiesInfoID();
				}

				//now, compare, and update bSyncWithCurrentMedications
				bSyncWithAllergies = nActiveCurrentAllergiesInfoID == m_nEMRInfoID;
			}
			
			if (bSyncWithAllergies) {
				// (c.haag 2007-08-03 13:19) - PLID 26929 - If this is a template detail, assign this a blank state
				if (m_bIsTemplateDetail) {
					SetState(LoadEMRDetailStateBlank( eitTable ));
				} else {
					//TES 6/5/2008 - PLID 29416 - Rather than using the "official" allergies state (which never really
					// existed), just load the state like normal, then merge in the Medications tab data (which only affects one
					// column in the table).
					if (NULL != pInfoItem && NULL != pEMNSpawner) {
						// (j.jones 2008-09-22 15:26) - PLID 31408 - send in the EMR ID
						SetState(pEMNSpawner->LoadEMRDetailStateDefault(m_nEMRInfoID, m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), m_pParentTopic->GetParentEMN()->GetParentEMR()->GetID(), GetRemoteData(), m_nRememberedDetailID));
					} else {
						// (j.jones 2008-09-22 15:26) - PLID 31408 - send in the EMR ID
						SetState(LoadEMRDetailStateDefault(rsInfo, m_nEMRInfoID, m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), m_pParentTopic->GetParentEMN()->GetParentEMR()->GetID(), -1, GetRemoteData(), m_nRememberedDetailID));
					}
					m_pParentTopic->GetParentEMN()->ApplyOfficialAllergies(this, m_nEMRInfoID);
				}
			} else {
				// (j.jones 2007-08-01 14:11) - PLID 26905 - pass in rsInfo to share the data we already know
				// (c.haag 2007-08-07 10:18) - PLID 26998 - Use the CEMN spawner version if possible
				if (NULL != pInfoItem && NULL != pEMNSpawner) {
					// (j.jones 2008-09-22 15:26) - PLID 31408 - send in the EMR ID
					SetState(pEMNSpawner->LoadEMRDetailStateDefault(m_nEMRInfoID, m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), m_pParentTopic->GetParentEMN()->GetParentEMR()->GetID(), GetRemoteData(), m_nRememberedDetailID));
				} else {
					// (j.jones 2008-09-22 15:26) - PLID 31408 - send in the EMR ID
					SetState(LoadEMRDetailStateDefault(rsInfo, m_nEMRInfoID, m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), m_pParentTopic->GetParentEMN()->GetParentEMR()->GetID(), -1, GetRemoteData(), m_nRememberedDetailID));
				}
			}
		}
	}

	if (NULL != rsInfo) {
		rsInfo->Close();
	}
	
	if(!m_bIsTemplateDetail) {
		ApplyEmrLinks();
	}

	// (j.jones 2010-02-17 15:32) - PLID 37318 - If we reloaded a SmartStamp Image that is linked to a table,
	// that link may now be invalid. If so, this function will fix it.
	EnsureLinkedSmartStampTableValid();

	//DRT 8/2/2007 - PLID 26919
	m_nSourceActionSourceID = nSourceActionSourceID;
	//DRT 8/14/2007 - PLID 27067 - Added nSourceActionSourceDataGroupID
	m_nSourceActionSourceDataGroupID = nSourceActionSourceDataGroupID;
	// (z.manning, 01/23/2008) - PLID 28690 - Added m_nSourceActionSourceHotSpotGroupID
	m_nSourceActionSourceHotSpotGroupID = nSourceActionSourceHotSpotGroupID;
	// (z.manning 2009-02-13 10:01) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
	m_nSourceActionSourceTableDropdownGroupID = nSourceActionSourceTableDropdownGroupID;

	// (j.jones 2007-01-10 14:54) - PLID 24027 - supported SourceDetailID
	// (z.manning 2009-03-11 10:06) - PLID 33338 - Use the new source action info class
	m_sai = sai;

	//We'll need to look up the source action's desttype
	m_SourceActionDestType = eaoInvalid;

	// (a.walling 2012-10-31 17:17) - PLID 53550 - LastSavedDetail - reset
	m_oLastSavedDetail = boost::none;

	if(m_bVisible) {
		
		//Process the actions for this detail.

		// (j.jones 2007-07-18 10:20) - PLID 26730 - check whether or not the info item has Info actions,
		// which it usually does not, such that we don't have to search for them
		if(m_eHasInfoActions != ehiasHasNoInfoItems) {
			//TES 10/8/2007 - PLID 27660 - Pass our bInitialLoad parameter through.
			m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEMRInfoActions(this, bIsInitialLoad);
		}

		// (j.jones 2010-02-12 14:23) - PLID 37318 - check whether this is part of a SmartStamp
		// set of details, and ensure links (or create the table) accordingly
		m_pParentTopic->GetParentEMN()->EnsureSmartStampLinks(this);

		// (j.jones 2010-06-21 14:47) - PLID 37981 - ensure the content is loaded if we are loading from a generic table
		if(pGenericTableContent) {
			LoadContent(TRUE, NULL, NULL, TRUE, pGenericTableContent);
		}

		CollectAndProcessActionsPostLoad(bIsInitialLoad);
	}

	SetUnsaved();
}

// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
void CEMNDetail::LoadFromDetailID(long nDetailID, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	try {
		_ConnectionPtr pCon;
		if(lpCon) pCon = lpCon;
		else pCon = GetRemoteData();

		// (c.haag 2007-02-22 15:26) - PLID 24889 - Most of the code run prior to this query has
		// been moved to LoadFromDetailRecordset

		//TES 12/7/2006 - PLID 23724 - Added the EmrInfoMasterID.
		// (j.jones 2007-01-10 14:55) - PLID 24027 - supported SourceDetailID
		// (c.haag 2007-01-23 11:07) - PLID 24376 - supported EmrInfoT.DataSubType
		// (c.haag 2007-02-22 15:27) - PLID 24889 - We now generate a query we can pass into 
		// LoadFromDetailRecordset so that it doesn't have to run a query for every detail to get data
		// (j.jones 2007-07-18 13:26) - PLID 26730 - load whether or not the info item has Info actions,
		// which it usually does not, such that we don't have to search for them later
		// (j.jones 2007-08-02 10:25) - PLID 26912 - added extra fields to be passed into LoadEMRDetailState
		//DRT 8/2/2007 - PLID 26919 - Added SourceActionSourceID
		//DRT 8/14/2007 - PLID 27067 - Added SourceActionSourceDataGroupID
		// (z.manning, 01/23/2008) - PLID 28690 - Added SourceActionSourceHotSpotGroupID
		// (c.haag 2008-06-17 16:36) - PLID 30319 - We now have to do a calculation to figure the name out 
		// (z.manning 2008-10-08 15:09) - PLID 31613 - Added LabID
		// (c.haag 2008-10-17 12:54) - PLID 31709 - TableRowsAsFields 
		// (z.manning 2009-02-13 10:01) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
		// (z.manning 2009-03-11 10:10) - PLID 33338 - Added SourceDataGroupID and SourceType
		// (j.jones 2010-02-11 15:24) - PLID 37318 - load ChildEMRInfoMasterID, SmartStampsEnabled, and ChildEMRDetailID
		// (z.manning 2010-02-25 11:08) - PLID 37532 - SourceDetailImageStampID
		//TES 2/25/2010 - PLID 37535 - UseSmartStampsLongForm
		// (z.manning 2010-07-26 13:33) - PLID 39848 - Removed UseSmartStampsLongForm
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		// (z.manning 2011-01-25 15:09) - PLID 42336 - Removed parent detail IDs
		//TES 3/17/2011 - PLID 41108 - Added HasGlassesOrderData and GlassesOrderLens
		// (z.manning 2011-10-05 16:59) - PLID 45842 - Added PrintData
		// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
		//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
		// (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore zoom and pan offsets.
		//"EMRDetailsT.ZoomLevel, EMRDetailsT.OffsetX, EMRDetailsT.OffsetY, "
		// (z.manning 2010-02-23 14:38) - PLID 37412 - Load whether or not detail has detail image stamps
		// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
		// (j.jones 2011-03-09 09:05) - PLID 42283 - added EMCodeUseTableCategories and Data_EMCodeCategoryID
		// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields, because they are now only calculated in the API, and not in Practice code
		//"EMRInfoMasterT.EMCodeCategoryID, EMRInfoMasterT.EMCodeUseTableCategories, EMRInfoMasterT.UseEMCoding AS Info_UseEMCoding, EMRInfoMasterT.EMCodingType, "
		// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
		// (a.walling 2008-06-30 13:34) - PLID 29271 - Preview Pane flags
		// (a.walling 2009-01-13 14:14) - PLID 32107 - Load the info item's background info
		// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
		// (j.jones 2010-09-22 09:04) - PLID 29039 - ensured we force a join only on data item actions
		// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
		// (j.armen 2014-08-06 14:42) - PLID 63026 - Added InkPenSize / InkPenColor
		_RecordsetPtr rsInfo = CreateParamRecordset(pCon, R"(
SELECT
	EmrDetailsT.ID AS DetailID,
	CASE WHEN EmrInfoT.ID = {CONST} THEN EMRDetailsT.MacroName ELSE EmrInfoT.Name END AS Name,
	EmrInfoT.DataType, EmrInfoT.DataSubType,
	EmrDetailsT.EmrInfoID, x, y, Width, Height, MergeOverride, EMRDetailsT.InkPenColor, EMRDetailsT.InkPenSizePercent,
	SourceTemplateID, SaveTableColumnWidths, SourceActionID, SourceDetailID, EmrInfoT.DataFormat, EmrInfoT.DisableTableBorder,
	EmrInfoT.EmrInfoMasterID, EMRTemplateDetailToDetailLinkT.EMRTemplateDetailID, EmrInfoT.TableRowsAsFields,
	EmrInfoT.ChildEMRInfoMasterID, EmrInfoT.SmartStampsEnabled, EMRDetailsT.ChildEMRDetailID, NULL AS ChildEMRTemplateDetailID,
	(CASE WHEN EmrInfoT.ID IN (SELECT SourceID FROM EMRActionsT WHERE Deleted = 0 AND SourceType = 3) THEN 1 ELSE 0 END) AS HasInfoActions,
	ReconstructedEMRDetailsT.ReviewStatus,
	EMRDetailsT.Text, EMRDetailsT.SliderValue, EMRDetailsT.InkData, EMRDetailsT.InkImagePathOverride, EMRDetailsT.InkImageTypeOverride, EMRDetailsT.ImageTextData, EMRDetailsT.PrintData,
	CASE WHEN EMRDetailsT.ID IN (SELECT EMRDetailID FROM EmrSelectT) THEN 1 ELSE 0 END AS Detail_HasListSelections, EMRActionsT.SourceID AS SourceActionSourceID,
	CASE WHEN EmrDetailsT.ID IN (SELECT EmrDetailID FROM EmrDetailImageStampsT WHERE EmrDetailImageStampsT.Deleted = 0) THEN CONVERT(bit, 1) ELSE CONVERT(bit, 0) END AS Detail_HasDetailImageStamps,
	EMRDataT.EMRDataGroupID AS SourceActionSourceDataGroupID,
	EmrSpotGroupID,
	EmrDetailsT.PreviewFlags, EmrDetailsT.LabID,
	EmrInfoT.BackgroundImageFilePath AS Info_BackgroundImageFilePath, EmrInfoT.BackgroundImageType AS Info_BackgroundImageType,
	DropdownGroupID AS SourceActionSourceTableDropdownGroupID, EmrDetailsT.SourceDataGroupID, EmrDetailsT.SourceDetailImageStampID, EmrActionsT.SourceType,
	EmrInfoT.HasGlassesOrderData, EmrInfoT.GlassesOrderLens, EmrInfoT.InfoFlags, EmrInfoT.HasContactLensData,
	EmrInfoT.UseWithWoundCareCoding
FROM EmrDetailsT
INNER JOIN EmrInfoT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID
LEFT JOIN EMRInfoMasterT ON EMRInfoT.EMRInfoMasterID = EMRInfoMasterT.ID
LEFT JOIN EMRTemplateDetailToDetailLinkT ON EmrDetailsT.ID = EMRTemplateDetailToDetailLinkT.EMRDetailID
LEFT JOIN ReconstructedEMRDetailsT ON ReconstructedEMRDetailsT.EMRDetailID = EmrDetailsT.ID
LEFT JOIN EMRActionsT ON EMRDetailsT.SourceActionID = EMRActionsT.ID
LEFT JOIN EMRDataT ON EMRActionsT.SourceID = EMRDataT.ID AND EmrActionsT.SourceType = {CONST}
LEFT JOIN EmrImageHotSpotsT ON EmrActionsT.SourceID = EmrImageHotSpotsT.ID AND EmrActionsT.SourceType = {CONST}
LEFT JOIN EmrTableDropdownInfoT ON EmrActionsT.SourceID = EmrTableDropdownInfoT.ID AND EmrActionsT.SourceType = {CONST}
WHERE EmrDetailsT.ID = {INT})", EMR_BUILT_IN_INFO__TEXT_MACRO, eaoEmrDataItem, eaoEmrImageHotSpot, eaoEmrTableDropDownItem, nDetailID);

		if(rsInfo->eof) {
			ThrowNxException("CEMNDetail::LoadFromDetailID called with invalid ID!");
		}

		// (c.haag 2007-02-22 15:35) - PLID 24889 - All of the code which used to be here now exists
		// in LoadFromDetailRecordset
		LoadFromDetailRecordset(rsInfo, TRUE, pCon);

	// (j.jones 2008-10-31 13:03) - PLID 31869 - Use thread-safe exception handling
	}NxCatchAllThread("Error in CEMNDetail::LoadFromDetailID");
}

// (c.haag 2007-02-27 08:59) - PLID 24949 - Added 'bLoadState' for when this function is called
// when populating the preload detail array
// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
void CEMNDetail::LoadFromDetailRecordset(ADODB::_RecordsetPtr& rsInfo, BOOL bCloseRecordsetWhenDone, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL */,
										 BOOL bLoadState /*= TRUE */)
{
	try {

		// (c.haag 2007-02-22 16:20) - This function loads in detail information given a recordset. The
		// required list of fields includes:
		//
		// DetailID, Name, DataType, DataSubType, EmrInfoID, x, y, Width, Height, MergeOverride,
		// SourceTemplateID, SaveTableColumnWidths, SourceActionID, SourceDetailID, DataFormat, DisableTableBorder,
		// EmrInfoMasterID, (EMRTemplateDetailToDetailLinkT).EMRTemplateDetailID
		//
		// Refer to LoadFromDetailID for example usage
		//
		_ConnectionPtr pCon;
		if(lpCon) pCon = lpCon;
		else pCon = GetRemoteData();

		if (NULL == rsInfo) {
			ThrowNxException("CEMNDetail::LoadFromDetailRecordset called without a recordset!");
		}
		if (rsInfo->eof) {
			ThrowNxException("CEMNDetail::LoadFromDetailRecordset called with an EOF recordset!");
		}

		const long nDetailID = AdoFldLong(rsInfo, "DetailID");

		m_nEMRDetailID = nDetailID;
		m_bIsTemplateDetail = false;
		if(m_bOwnTopic && !m_pParentTopic) {
			//TES 4/15/2010 - PLID 24692 - We know this is a new topic, so give it a new entry in the linked list.
			//TES 5/3/2010 - PLID 24692 - For "bottom-up" loading, we don't maintain position entries.
			m_pParentTopic = new CEMRTopic(m_bIsTemplateDetail, TRUE, NULL);
			m_pOriginalParentTopic = m_pParentTopic;
			if(m_pOriginalParentTopic) {
				m_strLastSavedParentTopicName = m_pOriginalParentTopic->GetName();
			}
		}

		// (j.jones 2007-10-22 09:43) - PLID 27830 - don't assign directly to m_strLabelText, as SetLabelText properly handles narratives
		SetLabelText(AdoFldString(rsInfo, "Name"));
		m_EMRInfoType = (EmrInfoType)AdoFldByte(rsInfo, "DataType");
		m_EMRInfoSubType = (EmrInfoSubType)AdoFldByte(rsInfo, "DataSubType");
		m_bTableRowsAsFields = AdoFldBool(rsInfo, "TableRowsAsFields"); // (c.haag 2008-10-16 11:24) - PLID 31709

		m_nEMRInfoID = AdoFldLong(rsInfo, "EmrInfoID");
		m_nEMRInfoMasterID = AdoFldLong(rsInfo, "EmrInfoMasterID");
		// (j.jones 2007-01-15 13:39) - PLID 24027 - added EMRTemplateDetailToDetailLinkT so we can track this ID
		m_nEMRTemplateDetailID = AdoFldLong(rsInfo, "EMRTemplateDetailID", -1);

		// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
		m_nChildEMRInfoMasterID = AdoFldLong(rsInfo, "ChildEMRInfoMasterID", -1);
		m_bSmartStampsEnabled = AdoFldBool(rsInfo, "SmartStampsEnabled", FALSE);
		m_nChildEMRDetailID = AdoFldLong(rsInfo, "ChildEMRDetailID", -1);

		// (j.jones 2007-07-18 13:25) - PLID 26730 - load whether or not the info item has Info actions,
		// which it usually does not, such that we don't have to search for them later
		m_eHasInfoActions = AdoFldLong(rsInfo, "HasInfoActions", 0) == 1 ? ehiasHasInfoItems : ehiasHasNoInfoItems;

		//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
		m_bHasGlassesOrderData = AdoFldBool(rsInfo, "HasGlassesOrderData", FALSE);
		m_golLens = (GlassesOrderLens)AdoFldLong(rsInfo, "GlassesOrderLens", (long)golInvalid);
		//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
		m_bHasContactLensData = AdoFldBool(rsInfo, "HasContactLensData", FALSE);
		// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
		m_bUseWithWoundCareCoding = AdoFldBool(rsInfo, "UseWithWoundCareCoding", FALSE);

		// (z.manning 2011-11-15 17:08) - PLID 38130
		m_nInfoFlags = AdoFldLong(rsInfo, "InfoFlags", 0);

		// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Load the review state for details (when they're really 
		// details), otherwise default to 0, no review state.
		if (m_nEMRDetailID != -1) {
			// See if this detail is in the special table.
			// (c.haag 2007-05-02 08:26) - PLID 25870 - We now load the ReviewStatus field in rsInfo
			m_nReviewState = AdoFldLong(rsInfo, "ReviewStatus", 0);
			/*
			_RecordsetPtr prsReviewState = CreateRecordset("SELECT ReviewStatus FROM ReconstructedEMRDetailsT WHERE EMRDetailID = %li", m_nEMRDetailID);
			if (!prsReviewState->eof) {
				// Pull the review state from the table
				m_nReviewState = AdoFldLong(prsReviewState->GetFields()->GetItem("ReviewStatus"));
			} else {
				// Not in the table, so no special "review state" for this detail.
				m_nReviewState = 0;
			}*/
		} else {
			// No detail, so no special "review state" naturally.
			m_nReviewState = 0;
		}
		
		//m.hancock - 4/9/2006 - PLID 20059 - Merging table items to Word as HTML has been broken.
		//Apparently when we merge now, we build a list of details but the data format is not loaded for each detail.
		//So, we're just loading the data format here.
		m_nDataFormat = AdoFldLong(rsInfo, "DataFormat",0);

		//m.hancock - 4/9/2006 - PLID 20016 - Load the flag that is used to determine if the table should have a disabled border when merged.
		BOOL bDisableTableBorder = AdoFldBool(rsInfo, "DisableTableBorder", FALSE);
		m_bDisableTableBorder = (bDisableTableBorder == FALSE) ? false : true;

		long x = AdoFldLong(rsInfo, "x", -1);
		if(x != -1) {
			long y = AdoFldLong(rsInfo, "y");
			if(x < 0 || y < 0) {
				//We've got bad data, don't try to display in an invalid location.
				x = -1;
				y = -1;
			}
			else {
				m_rcDefaultClientArea.SetRect(x, y, x+AdoFldLong(rsInfo, "Width"), y+AdoFldLong(rsInfo, "Height"));
			}
		}
		m_strMergeFieldOverride = AdoFldString(rsInfo, "MergeOverride", "");
		m_nEMRSourceTemplateID = AdoFldLong(rsInfo, "SourceTemplateID", -1);
		m_bSaveTableColumnWidths = AdoFldBool(rsInfo, "SaveTableColumnWidths", FALSE);
		m_sai.nSourceActionID = AdoFldLong(rsInfo, "SourceActionID", -1);
		// (z.manning 2009-03-11 10:15) - PLID 33338 - Added SourceDataGroupID and source type
		m_sai.SetDataGroupID(AdoFldLong(rsInfo, "SourceDataGroupID", -1));
		// (z.manning 2010-02-25 11:09) - PLID 37532 - SourceDetailImageStampID
		m_sai.SetDetailStampID(AdoFldLong(rsInfo, "SourceDetailImageStampID", -1));
		m_sai.eaoSourceType = (EmrActionObject)AdoFldLong(rsInfo, "SourceType", eaoInvalid);
		//DRT 8/2/2007 - PLID 26919
		m_nSourceActionSourceID = AdoFldLong(rsInfo, "SourceActionSourceID", -1);
		//DRT 8/14/2007 - PLID 27067
		m_nSourceActionSourceDataGroupID = AdoFldLong(rsInfo, "SourceActionSourceDataGroupID", -1);
		// (z.manning, 01/23/2008) - PLID 28690
		m_nSourceActionSourceHotSpotGroupID = AdoFldLong(rsInfo, "EmrSpotGroupID", -1);
		// (z.manning 2009-02-13 10:04) - PLID 33070 - SourceActionSourceTableDropdownGroupID
		m_nSourceActionSourceTableDropdownGroupID = AdoFldLong(rsInfo, "SourceActionSourceTableDropdownGroupID", -1);
		m_sai.nSourceDetailID = AdoFldLong(rsInfo, "SourceDetailID", -1);
		//We'll need to look up the source action's desttype
		m_SourceActionDestType = eaoInvalid;
		// (a.walling 2008-06-30 13:33) - PLID 29271 - Preview Pane flags
		m_nPreviewFlags = AdoFldLong(rsInfo, "PreviewFlags", 0);
		// (z.manning 2008-10-08 15:10) - PLID 31613 - Lab ID
		m_varLabID = AdoFldVar(rsInfo, "LabID");

		// (a.walling 2009-01-13 13:54) - PLID 32107 - Load the info item's background info
		m_varInfoBackgroundImageFilePath = AdoFldVar(rsInfo, "Info_BackgroundImageFilePath");
		m_varInfoBackgroundImageType = AdoFldVar(rsInfo, "Info_BackgroundImageType");

		// (j.armen 2014-07-23 16:26) - PLID 63026 - Load Ink Pen Color
		_variant_t vtInkColor = AdoFldVar(rsInfo, "InkPenColor");
		if (vtInkColor != g_cvarNull)
			m_nDefaultPenColor = VarLong(vtInkColor);

		// (j.armen 2014-07-23 16:26) - PLID 63026 - Load Ink Pen Size
		_variant_t vtInkSize = AdoFldVar(rsInfo, "InkPenSizePercent");
		if (vtInkSize != g_cvarNull)
			m_nDefaultPenSizePercent = VarByte(vtInkSize);

		// (c.haag 2007-02-27 09:00) - PLID 24949 - Set the state only if we have to. bLoadState
		// will be false when populating the preload detail array on the initial load
		if (bLoadState) {
			// (j.jones 2007-08-02 10:25) - PLID 26912 - pass in rsInfo to be reused
			// (z.manning 2011-02-24 17:00) - PLID 42579 - This now takes an array of detail IDs
			CArray<long,long> arynDetailIDs;
			arynDetailIDs.Add(nDetailID);
			SetState(LoadEMRDetailState(rsInfo, arynDetailIDs, m_EMRInfoType, lpCon));
		}

		// (c.haag 2007-02-22 15:37) - PLID 24889 - We're done with rsInfo. Close the recordset
		// if we are allowed to
		if (bCloseRecordsetWhenDone) {
			rsInfo->Close();
		}

		//Now, do we need to load our parent topic?
		if(m_bOwnTopic) {
			// (j.jones 2007-08-02 12:09) - PLID 26915 - pass in our connection
			m_pParentTopic->LoadFromDetail(this, pCon);
		}

		// (a.walling 2012-10-31 17:17) - PLID 53550 - LastSavedDetail - reset
		m_oLastSavedDetail = boost::none;

		// (j.jones 2010-02-17 15:32) - PLID 37318 - If we reloaded a SmartStamp Image that is linked to a table,
		// that link may now be invalid. If so, this function will fix it.
		EnsureLinkedSmartStampTableValid();

		if(m_bVisible) {
			//TES 5/22/2006 - We're loading an existing EMR detail, so let's not actually process the actions.
			//These were always ignored anyway, because CEMR would ignore actions while it was loading, but since the loading is
			//now asynchronous, CEMR doesn't know when to stop ignoring them any more.
			/*//Process the actions for this detail.
			if(m_eHasInfoActions != ehiasHasNoInfoItems) {
				m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEMRInfoActions(this);
			}
			CDWordArray arDataIds;
			GetSelectedValues(arDataIds);
			for(int i = 0; i < arDataIds.GetSize(); i++) {
				m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEMRDataActions(arDataIds[i], this);
			}*/
		}

		// (j.jones 2008-10-31 13:03) - PLID 31869 - Use thread-safe exception handling
	}NxCatchAllThread("Error in CEMNDetail::LoadFromDetailRecordset");
}

// (j.jones 2007-04-12 14:19) - PLID 25604 - added parameter for bIsInitialLoad
// (j.jones 2008-09-23 09:41) - PLID 31408 - added parameter for EMRGroupID
// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
void CEMNDetail::LoadFromTemplateDetailID(long nTemplateDetailID, BOOL bIsOnTemplate, BOOL bIsNewDetail, BOOL bIsInitialLoad, IN long nPatientID, IN long nEMRGroupID, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	CSqlFragment sqlDeclare(R"(
DECLARE @EMR_BUILT_IN_INFO__TEXT_MACRO INT SET @EMR_BUILT_IN_INFO__TEXT_MACRO = {CONST}
DECLARE @eistCurrentMedicationsTable INT SET @eistCurrentMedicationsTable = {CONST}
DECLARE @eistAllergiesTable INT SET @eistAllergiesTable = {CONST}
DECLARE @eistGenericTable INT SET @eistGenericTable = {CONST}
DECLARE @eaoEmrDataItem INT SET @eaoEmrDataItem = {CONST}
DECLARE @eaoEmrImageHotSpot INT SET @eaoEmrImageHotSpot = {CONST}
DECLARE @eaoEmrTableDropDownItem INT SET @eaoEmrTableDropDownItem = {CONST}
)",
		EMR_BUILT_IN_INFO__TEXT_MACRO,
		eistCurrentMedicationsTable,
		eistAllergiesTable,
		eistGenericTable,
		eaoEmrDataItem,
		eaoEmrImageHotSpot,
		eaoEmrTableDropDownItem);

	// (c.haag 2007-02-22 13:14) - PLID 24881 - Most of the code run prior to the following query
	// has been moved to LoadFromTemplateDetailRecordset. This function now serves as nothing more
	// than a gateway to LoadFromTemplateDetailRecordset.

	//TES 12/7/2006 - PLID 23724 - Added the EmrInfoMasterID.
	// (j.jones 2007-01-10 14:55) - PLID 24027 - supported SourceDetailID
	// (c.haag 2007-01-23 11:07) - PLID 24376 - supported EmrInfoT.DataSubType
	// (c.haag 2007-02-22 12:21) - PLID 24881 - supports LoadFromTemplateDetailRecordset
	// (j.jones 2007-07-18 10:20) - PLID 26730 - load whether or not the info item has Info actions,
	// which it usually does not, such that we don't have to search for them later
	// (j.jones 2007-08-01 14:12) - PLID 26905 - added more data to this query that we can pass into
	// LoadEMRDetailStateDefault() later on
	//DRT 8/2/2007 - PLID 26919 - Added SourceActionSourceID
	//DRT 8/14/2007 - PLID 27067 - Added SourceActionSourceDataGroupID
	// (z.manning, 01/23/2008) - PLID 286909 - Added SourceActionSourceHotSpotGroupID
	//DRT 2/14/2008 - PLID 28698 - Added an optimization to check if any hotspot actions exist.
	//TES 6/5/2008 - PLID 29416 - The system tables are hardcoded to display as "Remember"ing, but the data doesn't
	// have that flag set, so override the data in that case.
	// (c.haag 2008-06-17 16:36) - PLID 30319 - We now have to do a calculation to figure the name out 
	// (j.jones 2008-09-22 15:02) - PLID 31408 - supported RememberForEMR, which is always disabled when allergy/current meds
	// (z.manning 2009-02-13 10:06) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
	// (z.manning 2009-03-11 10:17) - PLID 33338 - Added SourceDataGroupID and source type
	//TES 2/25/2010 - PLID 37535 - UseSmartStampsLongForm
	//TES 3/17/2011 - PLID 41108 - Added HasGlassesOrderData and GlassesOrderLens
	// (z.manning 2011-10-06 15:13) - PLID 45842 - Print data
	// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
	//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
	// (j.jones 2010-06-21 12:22) - PLID 37981 - generic tables never remember their values
	// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
	// (j.jones 2011-03-09 09:05) - PLID 42283 - added EMCodeUseTableCategories and Data_EMCodeCategoryID
	// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields, because they are now only calculated in the API, and not in Practice code
	//"EMRInfoMasterT.EMCodeCategoryID, EMRInfoMasterT.EMCodeUseTableCategories, EMRInfoMasterT.UseEMCoding AS Info_UseEMCoding, EMRInfoMasterT.EMCodingType, "
	// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
	// (a.walling 2008-06-30 13:34) - PLID 29271 - Preview Pane flags
	// (c.haag 2008-10-16 11:31) - PLID 31709 - TableRowsAsFields
	// (j.jones 2010-02-11 15:24) - PLID 37318 - load ChildEMRInfoMasterID, SmartStampsEnabled, and ChildEMRTemplateDetailID
	// (z.manning 2011-01-25 15:09) - PLID 42336 - Removed parent detail IDs
	//TES 3/1/7/2010 - PLID 37530 - SourceStampID, SourceStampIndex
	// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
	// (j.jones 2010-09-22 09:04) - PLID 29039 - ensured we force a join only on data item actions
	// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
	// (j.armen 2014-07-23 17:05) - PLID 63026 - Cleanup, Handle loading the InkPenColor, InkPenSize
	_RecordsetPtr rsInfo = CreateParamRecordset(pCon, R"(
{SQL}

SELECT
	-- EmrTemplateDetailsT
	TD.ID AS TemplateDetailID, TD.TemplateID, TD.x, TD.y, TD.Width, TD.Height, TD.MergeOverride,
	TD.SaveTableColumnWidths, TD.SourceActionID, TD.SourceDetailID, TD.EmrInfoMasterID, TD.DefaultText,
	TD.SliderValue, 1 AS HasSliderValue, TD.PreviewFlags, TD.SourceDataGroupID, NULL AS SourceDetailImageStampID,
	TD.ChildEMRTemplateDetailID, NULL AS ChildEMRDetailID, TD.SourceStampID, TD.SourceStampIndex, TD.PrintData,
	TD.InkPenColor, TD.InkPenSizePercent,

	CASE WHEN TD.ID IN (SELECT EMRTemplateDetailID FROM EMRTemplateSelectT) THEN 1 ELSE 0 END AS Template_HasDefaultValue,
	CASE WHEN TD.ID IN (SELECT EMRDetailID FROM EMRHotSpotTemplateSelectT) THEN 1 ELSE 0 END AS HasTemplateHotSpots,
	
	-- EMRInfoT
	I.DataType, I.DataSubType, I.ID AS EmrInfoID, I.DefaultText AS Info_DefaultText,
	I.BackgroundImageFilePath AS Info_BackgroundImageFilePath, I.BackgroundImageType AS Info_BackgroundImageType,
	I.SliderMin AS Info_SliderMin, I.SliderMax AS Info_SliderMax, I.SliderInc AS Info_SliderInc, I.TableRowsAsFields,
	I.ChildEMRInfoMasterID, I.SmartStampsEnabled, I.HasGlassesOrderData, I.GlassesOrderLens, I.InfoFlags,
	I.HasContactLensData, I.UseWithWoundCareCoding,

	CASE WHEN I.ID = @EMR_BUILT_IN_INFO__TEXT_MACRO THEN TD.MacroName ELSE I.Name END AS Name,
	CASE WHEN I.ID IN (SELECT SourceID FROM EMRActionsT WHERE Deleted = 0 AND SourceType = 3) THEN 1 ELSE 0 END AS HasInfoActions,
	
	CASE
		WHEN I.DataSubType IN (@eistCurrentMedicationsTable, @eistAllergiesTable) THEN convert(bit,1)
		WHEN I.DataSubType = @eistGenericTable THEN Convert(bit,0)
		ELSE I.RememberForPatient
	END AS Info_RememberForPatient,

	CASE
		WHEN I.DataSubType IN (@eistCurrentMedicationsTable, @eistAllergiesTable, @eistGenericTable) THEN convert(bit,0)
		ELSE I.RememberForEMR
	END AS Info_RememberForEMR,

	CASE
		WHEN I.ID IN (
			SELECT ID.EMRInfoID
			FROM EMRInfoDefaultsT ID
			INNER JOIN EMRDataT D ON ID.EMRDataID = D.ID
			WHERE D.Inactive = 0 AND D.IsLabel = 0
		) THEN 1
		ELSE 0
	END AS Info_HasDefaultValue,

	-- EMRActionsT
	A.SourceID AS SourceActionSourceID, A.SourceType,

	-- EMRDataT
	D.EMRDataGroupID AS SourceActionSourceDataGroupID,

	-- EmrImageHotSpotsT
	HS.EmrSpotGroupID,

	-- EmrTableDropdownInfoT
	DI.DropdownGroupID AS SourceActionSourceTableDropdownGroupID

FROM EmrTemplateDetailsT TD
INNER JOIN EmrInfoMasterT IM ON TD.EmrInfoMasterID = IM.ID 
INNER JOIN EmrInfoT I ON IM.ActiveEmrInfoID = I.ID
LEFT JOIN EMRActionsT A ON TD.SourceActionID = A.ID
LEFT JOIN EMRDataT D ON A.SourceID = D.ID AND A.SourceType = @eaoEmrDataItem
LEFT JOIN EmrImageHotSpotsT HS ON A.SourceID = HS.ID AND A.SourceType = @eaoEmrImageHotSpot
LEFT JOIN EmrTableDropdownInfoT DI ON A.SourceID = DI.ID AND A.SourceType = @eaoEmrTableDropDownItem
WHERE TD.ID = {INT})", sqlDeclare, nTemplateDetailID);

	if(rsInfo->eof) {
		ThrowNxException("CEMNDetail::LoadFromTemplateDetailID called with invalid ID!");
	}

	//
	// (c.haag 2007-02-22 12:23) - PLID 24881 - All the code that used to be executed below here
	// now exists within LoadFromTemplateDetailRecordset()
	//
	// (j.jones 2008-09-23 09:42) - PLID 31408 - send the EMRGroupID
	LoadFromTemplateDetailRecordset(rsInfo, TRUE, bIsOnTemplate, bIsNewDetail, bIsInitialLoad, nPatientID, nEMRGroupID, lpCon);
}

// (c.haag 2007-02-27 08:59) - PLID 24949 - Added 'bLoadState' for when this function is called
// when populating the preload detail array on the initial load
// (j.jones 2007-04-12 14:19) - PLID 25604 - added parameter for bIsInitialLoad
// (j.jones 2007-07-24 17:05) - PLID 26742 - added parameter for pParentEmr
// (j.jones 2008-09-23 09:41) - PLID 31408 - added parameter for EMRGroupID
// (z.manning 2011-01-26 11:33) - PLID 42336 - Added optional parameter for the EMN loader
// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
void CEMNDetail::LoadFromTemplateDetailRecordset(_RecordsetPtr& rsInfo, BOOL bCloseRecordsetWhenDone, BOOL bIsOnTemplate, BOOL bIsNewDetail, BOOL bIsInitialLoad, IN long nPatientID, IN long nEMRGroupID, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/,
												 BOOL bLoadState /*= TRUE */, CEMR *pParentEmr /*= NULL*/, CEMNLoader *pEmnLoader /* = NULL */)
{
	// (c.haag 2007-02-22 12:18) - PLID 24881 - This function loads in detail data given a recordset.
	// Most of the code below was copied from the old version of LoadFromTemplateDetailID. The recordset
	// must have these fields:
	//
	// TemplateDetailID, Name, DataType, DataSubType, TemplateID, x, y, Width, Height, EmrInfoID, MergeOverride,
	// SaveTableColumnWidths, SourceActionID, SourceDetailID, EmrInfoMasterID
	//
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	if (NULL == rsInfo) {
		ThrowNxException("CEMNDetail::LoadFromTemplateDetailRecordset called without a recordset!");
	}
	if (rsInfo->eof) {
		ThrowNxException("CEMNDetail::LoadFromTemplateDetailRecordset called with an EOF recordset!");
	}

	const long nTemplateDetailID = AdoFldLong(rsInfo, "TemplateDetailID");

	if(!bIsOnTemplate || !bIsNewDetail)
		m_nEMRTemplateDetailID = nTemplateDetailID;
	m_bIsTemplateDetail = bIsOnTemplate;
	if(m_bOwnTopic && !m_pParentTopic) {
		//TES 4/15/2010 - PLID 24692 - We know this is a new topic, so give it a new entry in the linked list.
		//TES 5/3/2010 - PLID 24692 - For "bottom-up" loading, we don't maintain position entries.
		m_pParentTopic = new CEMRTopic(m_bIsTemplateDetail, TRUE, NULL);
		m_pOriginalParentTopic = m_pParentTopic;
		if(m_pOriginalParentTopic) {
			m_strLastSavedParentTopicName = m_pOriginalParentTopic->GetName();
		}
	}

	// (j.jones 2007-10-22 09:43) - PLID 27830 - don't assign directly to m_strLabelText, as SetLabelText properly handles narratives
	SetLabelText(AdoFldString(rsInfo, "Name"));
	m_EMRInfoType = (EmrInfoType)AdoFldByte(rsInfo, "DataType");
	m_EMRInfoSubType = (EmrInfoSubType)AdoFldByte(rsInfo, "DataSubType");
	m_bTableRowsAsFields = AdoFldBool(rsInfo, "TableRowsAsFields"); // (c.haag 2008-10-16 11:24) - PLID 31709

	m_nEMRInfoID = AdoFldLong(rsInfo, "EmrInfoID");
	m_nEMRInfoMasterID = AdoFldLong(rsInfo, "EmrInfoMasterID");
	// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
	m_nChildEMRInfoMasterID = AdoFldLong(rsInfo, "ChildEMRInfoMasterID", -1);
	m_bSmartStampsEnabled = AdoFldBool(rsInfo, "SmartStampsEnabled", FALSE);
	m_nChildEMRTemplateDetailID = AdoFldLong(rsInfo, "ChildEMRTemplateDetailID", -1);

	//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
	m_bHasGlassesOrderData = AdoFldBool(rsInfo, "HasGlassesOrderData", FALSE);
	m_golLens = (GlassesOrderLens)AdoFldLong(rsInfo, "GlassesOrderLens", (long)golInvalid);
	//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
	m_bHasContactLensData = AdoFldBool(rsInfo, "HasContactLensData", FALSE);
	// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
	m_bUseWithWoundCareCoding = AdoFldBool(rsInfo, "UseWithWoundCareCoding", FALSE);

	// (z.manning 2011-11-15 17:08) - PLID 38130
	m_nInfoFlags = AdoFldLong(rsInfo, "InfoFlags", 0);

	long x = AdoFldLong(rsInfo, "x", -1);
	if(x != -1) {
		long y = AdoFldLong(rsInfo, "y");
		if(x < 0 || y < 0) {
			//We've got bad data, don't try to display in an invalid location.
			x = -1;
			y = -1;
		}
		else {
			m_rcDefaultClientArea.SetRect(x, y, x+AdoFldLong(rsInfo, "Width"), y+AdoFldLong(rsInfo, "Height"));
		}
	}
	m_strMergeFieldOverride = AdoFldString(rsInfo, "MergeOverride", "");
	m_nEMRSourceTemplateID = AdoFldLong(rsInfo, "TemplateID");
	m_bSaveTableColumnWidths = AdoFldBool(rsInfo, "SaveTableColumnWidths", FALSE);
	m_sai.nSourceActionID = AdoFldLong(rsInfo, "SourceActionID", -1);
	m_sai.SetDataGroupID(AdoFldLong(rsInfo, "SourceDataGroupID", -1));
	m_sai.eaoSourceType = (EmrActionObject)AdoFldLong(rsInfo, "SourceType", eaoInvalid);
	//TES 3/17/2010 - PLID 37530
	m_sai.SetGlobalStampIDAndIndex(AdoFldLong(rsInfo, "SourceStampID", -1), AdoFldLong(rsInfo, "SourceStampIndex", -1));
	//DRT 8/2/2007 - PLID 26919
	m_nSourceActionSourceID = AdoFldLong(rsInfo, "SourceActionSourceID", -1);
	//DRT 8/14/2007 - PLID 27067 - Added nSourceActionSourceDataGroupID
	m_nSourceActionSourceDataGroupID = AdoFldLong(rsInfo, "SourceActionSourceDataGroupID", -1);
	// (z.manning, 01/23/2008) - PLID 28690
	m_nSourceActionSourceHotSpotGroupID = AdoFldLong(rsInfo, "EmrSpotGroupID", -1);
	// (z.manning 2009-02-13 10:09) - PLID 33070
	m_nSourceActionSourceTableDropdownGroupID = AdoFldLong(rsInfo, "SourceActionSourceTableDropdownGroupID", -1);

	// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
	// because they are now only calculated in the API, and not in Practice code
	/*
	// (j.jones 2007-08-27 10:35) - PLID 27056 - load the E/M coding data
	m_nEMCodeCategoryID = AdoFldLong(rsInfo, "EMCodeCategoryID", -1);
	// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_eEMCodeUseTableCategories
	m_eEMCodeUseTableCategories = (EMCodeUseTableCategories)AdoFldLong(rsInfo, "EMCodeUseTableCategories", (long)emcutcNone);
	m_bUseEMCoding = AdoFldBool(rsInfo, "Info_UseEMCoding", FALSE);
	m_emctEMCodingType = (EMCodingTypes)AdoFldLong(rsInfo, "EMCodingType", -1);
	*/

	// (j.jones 2007-07-18 12:16) - PLID 26730 - load whether or not the info item has Info actions,
	// which it usually does not, such that we don't have to search for them later
	m_eHasInfoActions = AdoFldLong(rsInfo, "HasInfoActions", 0) == 1 ? ehiasHasInfoItems : ehiasHasNoInfoItems;

	//do not set the SourceDetailID if not a template, otherwise we'd be using the template's detail ID!
	if(bIsOnTemplate)
		m_sai.nSourceDetailID = AdoFldLong(rsInfo, "SourceDetailID", -1);
	if(m_sai.nSourceActionID != -1 && bIsOnTemplate) {
		m_bVisible = FALSE;
	}
	// (z.manning 2010-05-06 12:01) - PLID 38527 - The code above sets template details as invisible if they
	// are spawned. However, when you spawn a smart stamp image the corresponding smart stamp table is not
	// technically spawned so we need to check and see if its parent is spawned and if so make it invisible as well.
	// (z.manning 2011-01-25 17:57) - PLID 42336 - Updated this to handle multiple images for one smart stamp table
	CEMN *pEmn = NULL;
	if(m_pParentTopic != NULL) {
		pEmn = m_pParentTopic->GetParentEMN();
	}
	if(bIsOnTemplate)
	{
		CEMNDetail *pChildDetail = NULL;
		if(m_nChildEMRTemplateDetailID != -1) {
			// (z.manning 2011-01-26 09:33) - PLID 42336 - This is a parent detail
			if(pEmn != NULL) {
				pChildDetail = pEmn->GetDetailByTemplateDetailID(m_nChildEMRTemplateDetailID);
			}
			else if(pEmnLoader != NULL) {
				pChildDetail = pEmnLoader->GetDetailByTemplateDetailID(m_nChildEMRTemplateDetailID);
			}
		}
		else {
			// (z.manning 2011-01-26 09:35) - PLID 42336 - This is not a parent detail so it's a potential child detail if it's a table.
			if(m_EMRInfoType == eitTable) {
				pChildDetail = this;
			}
		}

		if(pChildDetail != NULL)
		{
			CEMNDetailArray aryParentDetails;
			if(pChildDetail->m_nEMRTemplateDetailID != -1) {
				if(pEmn != NULL) {
					pEmn->GetParentDetailsByTemplateDetailID(pChildDetail->m_nEMRTemplateDetailID, &aryParentDetails);
				}
				else if(pEmnLoader != NULL) {
					pEmnLoader->GetParentDetailsByTemplateDetailID(pChildDetail->m_nEMRTemplateDetailID, &aryParentDetails);
				}
			}

			if(aryParentDetails.GetSize() > 0)
			{
				if(aryParentDetails.AtLeastOneVisibleDetail()) {
					pChildDetail->m_bVisible = TRUE;
				}
				else {
					pChildDetail->m_bVisible = FALSE;
				}
			}
		}
	}

	//We'll need to look up the source action's desttype
	m_SourceActionDestType = eaoInvalid;
	
	m_nPreviewFlags = AdoFldLong(rsInfo, "PreviewFlags", 0);
	
	// (j.armen 2014-07-23 14:57) - PLID 62837 - Load Ink Pen Color
	_variant_t vtInkColor = AdoFldVar(rsInfo, "InkPenColor");
	if (vtInkColor != g_cvarNull)
		m_nDefaultPenColor = VarLong(vtInkColor);

	// (j.armen 2014-07-23 14:57) - PLID 62837 - Load Ink Pen Size
	_variant_t vtInkSize = AdoFldVar(rsInfo, "InkPenSizePercent");
	if (vtInkSize != g_cvarNull)
		m_nDefaultPenSizePercent = VarByte(vtInkSize);
	
	// (a.walling 2009-01-13 13:54) - PLID 32107 - Load the info item's background info
	m_varInfoBackgroundImageFilePath = AdoFldVar(rsInfo, "Info_BackgroundImageFilePath");
	m_varInfoBackgroundImageType = AdoFldVar(rsInfo, "Info_BackgroundImageType");

	long nPatientIDToUse = -1;
	if(!bIsOnTemplate) {
		if(!m_pParentTopic) {
			nPatientIDToUse = nPatientID;
		}
		else {
			nPatientIDToUse = m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID();
		}
	}

	// (c.haag 2007-02-27 10:41) - PLID 24949 - Set the state only if we have to. bLoadState
	// will be false when populating the preload detail array. The ApplyEmrLinks() function is
	// included here too because it assumes the state has been loaded. ApplyEmrLinks() is now
	// called in CEMN::PostTopicLoad().
	if (bLoadState) {

		// (c.haag 2007-08-03 13:19) - PLID 26929 - We now set this to true even for template details
		BOOL bSyncWithCurrentMedications = IsCurrentMedicationsTable();
		if(bSyncWithCurrentMedications) {

			// (j.jones 2007-07-24 09:27) - PLID 26742 - the medications info ID is cached in CEMR
			long nActiveCurrentMedicationsInfoID = -2;
			//do memory checks
			if(m_pParentTopic) {
				if(m_pParentTopic->GetParentEMN()) {
					if(m_pParentTopic->GetParentEMN()->GetParentEMR()) {
						// (a.walling 2010-08-23 18:34) - PLID 40222 - Pass in the connection 
						nActiveCurrentMedicationsInfoID = m_pParentTopic->GetParentEMN()->GetParentEMR()->GetCurrentMedicationsInfoID(pCon);
					}
				}
			}

			if(nActiveCurrentMedicationsInfoID == -2 && pParentEmr) {
				//sometimes this can be called on a detail not yet linked to an EMR,
				//though the EMR pointer may be passed in
				// (a.walling 2010-08-23 18:34) - PLID 40222 - Pass in the connection 
				nActiveCurrentMedicationsInfoID = pParentEmr->GetCurrentMedicationsInfoID(pCon);				
			}

			if(nActiveCurrentMedicationsInfoID == -2) {
				//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
				//but why don't we have an EMR?
				ASSERT(FALSE);
				// (a.walling 2010-08-23 18:34) - PLID 40222 - Pass in the connection 
				nActiveCurrentMedicationsInfoID = GetActiveCurrentMedicationsInfoID(pCon);
			}

			//now, compare, and update bSyncWithCurrentMedications
			bSyncWithCurrentMedications = nActiveCurrentMedicationsInfoID == m_nEMRInfoID;
		}

		if (bSyncWithCurrentMedications) {
			// (c.haag 2007-02-08 12:17) - PLID 24376 - If we are pulling an official Current Medications detail,
			// then we must calculate the state from the EMN itself. Similar code is run in these three places:
			//
			// When loading Current Medication info items into templates, we should not do any special calculating. It
			// should behave exactly as it did before.
			//
			// CEMNDetail::LoadFromInfoID()				- When adding a Current Medications detail to a topic manually
			// CEMNDetail::LoadFromTemplateDetailID()	- When adding a Current Medications detail from a template
			// CEMRTopic::PostLoadDetails()				- When loading a patient EMN from a template
			// LoadEmrTopic								- When loading detail states in a thread
			// CEMRTopic::AddDetail()					- The rest of the time (failsafe)
			//
			if (m_pParentTopic) {
				// (c.haag 2007-08-03 13:21) - PLID 26929 - If this is a template detail, this should default
				// to a blank state
				if (m_bIsTemplateDetail) {
					SetState(LoadEMRDetailStateBlank( eitTable ));
				} else {
					//TES 6/5/2008 - PLID 29416 - Rather than using the "official" current medications state (which never really
					// existed), just load the state like normal, then merge in the Medications tab data (which only affects one
					// column in the table).
					// (j.jones 2008-09-22 15:26) - PLID 31408 - send in the EMR ID
					if(nEMRGroupID == -1 && pParentEmr != NULL) {
						nEMRGroupID = pParentEmr->GetID();
					}
					SetState(LoadEMRDetailStateDefault(rsInfo, m_nEMRInfoID, nPatientIDToUse, nEMRGroupID, nTemplateDetailID, lpCon, m_nRememberedDetailID));
					m_pParentTopic->GetParentEMN()->ApplyOfficialCurrentMedications(this, m_nEMRInfoID);
				}
			} else {
				// (c.haag 2007-02-08 12:10) - If there is no parent topic, empty the state so that
				// CEMRTopic::PostLoadDetails will do the load for us
				_variant_t vEmpty;
				vEmpty.vt = VT_EMPTY;
				SetState(vEmpty);
			}
		} else {
			// (c.haag 2007-04-05 13:18) - PLID 25516 - If we are pulling an official allergies detail,
			// then we must calculate the state from the EMN itself.

			// (c.haag 2007-08-03 13:19) - PLID 26929 - We now set this to true even for template details
			BOOL bSyncWithAllergies = IsAllergiesTable();

			if(bSyncWithAllergies) {

				// (j.jones 2007-07-24 09:27) - PLID 26742 - the Allergy Info ID is cached in CEMR
				long nActiveCurrentAllergiesInfoID = -2;
				//do memory checks
				if(m_pParentTopic) {
					if(m_pParentTopic->GetParentEMN()) {
						if(m_pParentTopic->GetParentEMN()->GetParentEMR()) {
							// (a.walling 2010-08-23 18:34) - PLID 40222 - Pass in the connection 
							nActiveCurrentAllergiesInfoID = m_pParentTopic->GetParentEMN()->GetParentEMR()->GetCurrentAllergiesInfoID(pCon);
						}
					}
				}

				if(nActiveCurrentAllergiesInfoID == -2 && pParentEmr) {
					//sometimes this can be called on a detail not yet linked to an EMR,
					//though the EMR pointer may be passed in
					// (a.walling 2010-08-23 18:34) - PLID 40222 - Pass in the connection 
					nActiveCurrentAllergiesInfoID = pParentEmr->GetCurrentAllergiesInfoID(pCon);				
				}

				if(nActiveCurrentAllergiesInfoID == -2) {
					//should only remain -2 if we have no EMR (-1 is bad data, but indicative that we did perform the check),
					//but why don't we have an EMR?
					ASSERT(FALSE);
					// (a.walling 2010-08-23 18:34) - PLID 40222 - Pass in the connection 
					nActiveCurrentAllergiesInfoID = GetActiveAllergiesInfoID(pCon);
				}

				//now, compare, and update bSyncWithCurrentMedications
				bSyncWithAllergies = nActiveCurrentAllergiesInfoID == m_nEMRInfoID;
			}

			if (bSyncWithAllergies) {
				if (m_pParentTopic) {
					// (c.haag 2007-08-03 13:21) - PLID 26929 - If this is a template detail, this should default
					// to a blank state
					if (m_bIsTemplateDetail) {
						SetState(LoadEMRDetailStateBlank( eitTable ));
					} else {
						//TES 6/5/2008 - PLID 29416 - Rather than using the "official" allergies state (which never really
						// existed), just load the state like normal, then merge in the Medications tab data (which only affects one
						// column in the table).
						// (j.jones 2008-09-22 15:26) - PLID 31408 - send in the EMR ID
						if(nEMRGroupID == -1 && pParentEmr != NULL) {
							nEMRGroupID = pParentEmr->GetID();
						}
						SetState(LoadEMRDetailStateDefault(rsInfo, m_nEMRInfoID, nPatientIDToUse, nEMRGroupID, nTemplateDetailID, lpCon, m_nRememberedDetailID));
						m_pParentTopic->GetParentEMN()->ApplyOfficialAllergies(this, m_nEMRInfoID);
					}
				} else {
					// (c.haag 2007-04-05 13:19) - If there is no parent topic, empty the state so that
					// CEMRTopic::PostLoadDetails will do the load for us
					_variant_t vEmpty;
					vEmpty.vt = VT_EMPTY;
					SetState(vEmpty);
				}
			} else {
				// (j.jones 2007-08-01 14:11) - PLID 26905 - pass in rsInfo to share the data we already know
				// (j.jones 2008-09-22 15:26) - PLID 31408 - send in the EMR ID
				if(nEMRGroupID == -1 && pParentEmr != NULL) {
					nEMRGroupID = pParentEmr->GetID();
				}
				SetState(LoadEMRDetailStateDefault(rsInfo, m_nEMRInfoID, nPatientIDToUse, nEMRGroupID, nTemplateDetailID, lpCon, m_nRememberedDetailID));
			}
		}
			
		if(!m_bIsTemplateDetail) {
			ApplyEmrLinks();
		}
	}

	// (c.haag 2007-02-22 12:25) - PLID 24881 - We're done with rsInfo. If we are allowed to close it, do it now
	if (bCloseRecordsetWhenDone) {
		rsInfo->Close();
	}

	// (j.jones 2010-02-17 15:32) - PLID 37318 - If we reloaded a SmartStamp Image that is linked to a table,
	// that link may now be invalid. If so, this function will fix it.
	// (z.manning 2011-03-02 09:38) - PLID 42549 - Moved this call to after we have already loaded the new detail's state.
	EnsureLinkedSmartStampTableValid();

	// (a.walling 2012-10-31 17:17) - PLID 53550 - LastSavedDetail - reset
	m_oLastSavedDetail = boost::none;

	if(m_bVisible) {
		if(m_pParentTopic) { //We only need to process actions if we're attached to a topic.
			ASSERT(bLoadState); // (c.haag 2007-02-27 11:08) - PLID 24949 - The state must have been loaded
			
			//Process the actions for this detail.

			// (j.jones 2007-07-18 12:15) - PLID 26730 - check whether or not the info item has Info actions,
			// which it usually does not, such that we don't have to search for them
			if(m_eHasInfoActions != ehiasHasNoInfoItems) {
				m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEMRInfoActions(this, bIsInitialLoad);
			}

			// (j.jones 2010-02-12 14:23) - PLID 37318 - check whether this is part of a SmartStamp
			// set of details, and ensure links (or create the table) accordingly
			m_pParentTopic->GetParentEMN()->EnsureSmartStampLinks(this);

			CollectAndProcessActionsPostLoad(bIsInitialLoad, NULL, lpCon);
		}
	}

	//If this is being done as part of the load, m_bUnsaved will be set back to FALSE once the loading is complete.
	SetUnsaved();
}

BOOL CEMNDetail::IsCurrentMedicationsTable() const
{
	// (c.haag 2007-01-23 11:08) - PLID 24376 - Returns TRUE if this detail is founded
	// on the internal Current Medications table item
	if (eistCurrentMedicationsTable == m_EMRInfoSubType)
		return TRUE;
	else
		return FALSE;
}

BOOL CEMNDetail::IsAllergiesTable() const
{
	// (c.haag 2007-04-02 15:41) - PLID 25465 - Returns TRUE if this detail is founded
	// on the internal Allergies table item
	if (eistAllergiesTable == m_EMRInfoSubType)
		return TRUE;
	else
		return FALSE;
}

// (j.jones 2010-06-21 11:43) - PLID 37981 - added IsGenericTable
BOOL CEMNDetail::IsGenericTable() const
{
	if (eistGenericTable == m_EMRInfoSubType)
		return TRUE;
	else
		return FALSE;
}

// (c.haag 2004-07-12 14:00) - PLID 13325 - We need to be able to override the
// merge field names of individual details, and this is where we can get the
// field name.
CString CEMNDetail::GetMergeFieldName(BOOL bConvertToHeaderName) const
{
	CString strName;
	if (m_strMergeFieldOverride.GetLength()) {
		strName = m_strMergeFieldOverride;
	}
	else {
		strName = m_strLabelText;
	}
	if (bConvertToHeaderName) {
		return ConvertToHeaderName("EMR", strName);
	}
	return strName;
}

CString CEMNDetail::GetMergeFieldOverride() const
{
	return m_strMergeFieldOverride;
}

void CEMNDetail::SetMergeFieldOverride(CString strMergeFieldOverride)
{
	if(m_strMergeFieldOverride != strMergeFieldOverride)
		SetUnsaved();

	m_strMergeFieldOverride = strMergeFieldOverride;
}

CEmrItemAdvDlg *CEMNDetail::CreateEmrItemAdvDlg()
{
	try {
		// Set the type for this CEmrItemAdvDlg
		switch (m_EMRInfoType) {
		case eitText:
			{
				CEmrItemAdvTextDlg *p = new CEmrItemAdvTextDlg(this);
				return p;
			}
			break;
		case eitSingleList:
			{
				CEmrItemAdvListDlg *p = new CEmrItemAdvListDlg(this);
				p->m_eeialtType = eialtListSingleSelect;
				return p;
			}
			break;
		case eitMultiList:
			{
				CEmrItemAdvListDlg *p = new CEmrItemAdvListDlg(this);
				p->m_eeialtType = eialtListMultiSelect;
				return p;
			}
			break;
		case eitImage:
			{
				CEmrItemAdvImageDlg *p = new CEmrItemAdvImageDlg(this);
				return p;
			}
			break;
		case eitSlider:
			{
				CEmrItemAdvSliderDlg *p = new CEmrItemAdvSliderDlg(this);
				return p;
			}
			break;
		case eitNarrative:
			{
				CEmrItemAdvNarrativeDlg *p = new CEmrItemAdvNarrativeDlg(this);
				return p;
			}
		case eitTable:
			{
				CEmrItemAdvTableDlg *p = new CEmrItemAdvTableDlg(this);
				return p;
			}
			break;
		default:
			// Unexpected type
			ASSERT(FALSE);
			return NULL;
			break;
		}
	} NxCatchAllThrow("Failed to create EmrItemAdvDlg");
}

void CEMNDetail::SetReadOnly(BOOL bReadOnly)
{
	// (z.manning 2008-11-03 10:07) - PLID 31890 - Signature details are ALWAYS read only
	if(IsSignatureDetail()) {
		m_bReadOnly = TRUE;
	}
	else {
		m_bReadOnly = bReadOnly;
	}

	if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
		//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
		// SetReadOnly() to ReflectReadOnlyStatus()
		m_pEmrItemAdvDlg->ReflectReadOnlyStatus(m_bReadOnly);
	}
}

void CEMNDetail::SetVisible(BOOL bVisible, BOOL bRedraw, BOOL bIsInitialLoad)
{
	if(bVisible && !m_bVisible) {
		//Process the actions for this detail. (If we're attached to a topic.
		if(m_pParentTopic) {

			// (j.jones 2007-07-18 13:21) - PLID 26730 - check whether or not the info item has Info actions,
			// which it usually does not, such that we don't have to search for them
			if(m_eHasInfoActions != ehiasHasNoInfoItems) {
				m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEMRInfoActions(this, bIsInitialLoad);
			}

			// (j.jones 2010-02-12 14:23) - PLID 37318 - check whether this is part of a SmartStamp
			// set of details, and ensure links (or create the table) accordingly
			m_pParentTopic->GetParentEMN()->EnsureSmartStampLinks(this);

			CollectAndProcessActionsPostLoad(bIsInitialLoad);

			m_pParentTopic->GetParentEMN()->UpdateMergeConflicts(GetMergeFieldName(TRUE));
		}
	}
	else if(!bVisible && m_bVisible) {
		if(m_pParentTopic) {
/*	DRT 8/3/2007 - PLID 26937 - This is no longer needed here.  Anywhere that wants to revoke any actions will
		use the CEMNUnspawner, which will recursively find all details to be unspawned.

			//Revoke the actions for this detail
			// (j.jones 2007-07-27 10:29) - PLID 26845 - check whether or not the info item has Info actions,
			// which it usually does not, such that we don't have to search for them
			if(m_eHasInfoActions != ehiasHasNoInfoItems) {				
				m_pParentTopic->GetParentEMN()->GetParentEMR()->RevokeEMRInfoActions(this);
			}

			CDWordArray arDataIds;
			GetSelectedValues(arDataIds);
			for(int i = 0; i < arDataIds.GetSize(); i++) {
				m_pParentTopic->GetParentEMN()->GetParentEMR()->RevokeEMRDataActions(arDataIds[i], this);
			}
*/
			m_pParentTopic->GetParentEMN()->UpdateMergeConflicts(GetMergeFieldName(TRUE), this);
		}
	}
	m_bVisible = bVisible;
	if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd()) && bRedraw) {
		m_pEmrItemAdvDlg->ShowWindow(bVisible?SW_SHOWNA:SW_HIDE);
	}

	// (j.jones 2010-03-19 17:06) - PLID 37397 - apply this visibility change to our SmartStamp sibling
	// (z.manning 2011-01-20 17:43) - PLID 42338 - Support multiple images per smart stamp table
	// (z.manning 2011-01-27 10:53) - PLID 42336 - Now that we can have mulitple images for one smart stamp table,
	// we should not be displaying an image just because the table is visible. However, if the table is being hidden
	// then all images should be as well.
	if(!bVisible) {
		m_arySmartStampImageDetails.SetVisible(bVisible, bRedraw, bIsInitialLoad);
	}
	if(GetSmartStampTableDetail() && GetSmartStampTableDetail()->GetVisible() != bVisible) {
		if(bVisible) {
			GetSmartStampTableDetail()->SetVisible(bVisible, bRedraw, bIsInitialLoad);
		}
		else {
			// (z.manning 2011-01-26 12:07) - PLID 42336 - Do not hide the smart stamp table if it has another
			// visible image.
			if(!GetSmartStampTableDetail()->GetSmartStampImageDetails()->AtLeastOneVisibleDetail()) {
				GetSmartStampTableDetail()->SetVisible(bVisible, bRedraw, bIsInitialLoad);
			}
		}
	}
}

BOOL CEMNDetail::GetVisible() const
{
	if(m_pParentTopic)
		return m_bVisible && m_pParentTopic->GetVisible();
	else
		return m_bVisible;
}

long CalcRegionArea(const CRgn &rgn)
{
	long nBytes = rgn.GetRegionData(NULL, 0);
	if (nBytes > 0) {
		RGNDATA *prgndata = (RGNDATA *)new BYTE[nBytes];
		if (rgn.GetRegionData(prgndata, nBytes)) {
			long nAns = 0;
			for (DWORD i=0; i<prgndata->rdh.nRgnSize; i += sizeof(RECT)) {
				RECT &rrc = *(RECT *)(prgndata->Buffer + i);
				nAns += ((rrc.right - rrc.left) * (rrc.bottom - rrc.top));
			}
			delete []prgndata;
			return nAns;
		} else {
			delete []prgndata;
			ASSERT(FALSE);
			return 0;
		}
	} else {
		return 0;
	}
}

long CalcCutOffArea(const CRect &rcRect, const CRgn &rgnWithinRegion)
{
	CRgn rgnRect;
	rgnRect.CreateRectRgnIndirect(rcRect);
	CRgn rgnCutOff;
	rgnCutOff.CreateRectRgn(0,0,1,1);
	rgnCutOff.CombineRgn((CRgn *)&rgnRect, (CRgn *)&rgnWithinRegion, RGN_DIFF);
	return CalcRegionArea(rgnCutOff);
}

long CalcCutOffArea(const CRect &rcRect, const CRect &rcWithinRect)
{
	CRgn rgnWithin;
	rgnWithin.CreateRectRgnIndirect(rcWithinRect);
	return CalcCutOffArea(rcRect, rgnWithin);
}

long CalcInternalCutOffArea(const CRect &rcRect, const CRgn &rgnWithinRegion, const CRect &rcBorder)
{
	return CalcCutOffArea(rcRect, rgnWithinRegion) - CalcCutOffArea(rcRect, rcBorder);
}

// A local utility function specifically designed to make emr details look decent within the emr tab window.
BOOL CEMNDetail::IsRectBetterFit(const CRect &rcCompare, const BOOL bCompareIsPreferredSize, const CRect &rcCompareBounding, const CRect &rcCompareTo, const BOOL bCompareToIsPreferredSize, const CRect &rcCompareBoundingTo, const CRgn &rgnToFitInto, const CRgn &rgnToFitIntoInverse, IN const CRect &rcArea)
{
	// We prefer the non-empty one over the empty one of course
	BOOL bEmpty1 = rcCompare.IsRectEmpty() || rcCompare.IsRectNull() || rcCompare.Width() == 0 || rcCompare.Height() == 0;
	BOOL bEmpty2 = rcCompareTo.IsRectEmpty() || rcCompareTo.IsRectNull() || rcCompareTo.Width() == 0 || rcCompareTo.Height() == 0;
	if (!bEmpty1 && bEmpty2) {
		// The first is not empty and the second is, which makes the first one better
		return TRUE;
	} else if (bEmpty1 && !bEmpty2) {
		// The first is empty and the second is not, which makes the second one better
		return FALSE;
	} else if (bEmpty1 && bEmpty2) {
		// Both empty
		ASSERT(FALSE);
		return FALSE;
	}

	// We prefer less internal cut-off to more cutoff
	long nAreaInternalCutOff1 = CalcInternalCutOffArea(rcCompare, rgnToFitInto, rcArea);
	long nAreaInternalCutOff2 = CalcInternalCutOffArea(rcCompareTo, rgnToFitInto, rcArea);
	if (nAreaInternalCutOff1 < nAreaInternalCutOff2) {
		return TRUE;
	} else if (nAreaInternalCutOff1 > nAreaInternalCutOff2) {
		return FALSE;
	}

	// We prefer less cut-off to more cutoff
	long nAreaCutOff1 = CalcCutOffArea(rcCompare, rgnToFitInto);
	long nAreaCutOff2 = CalcCutOffArea(rcCompareTo, rgnToFitInto);
	if (nAreaCutOff1 < nAreaCutOff2) {
		return TRUE;
	} else if (nAreaCutOff1 > nAreaCutOff2) {
		return FALSE;
	}

	// We prefer the top to not the top
	BOOL bIsTop1 = rcCompare.top == rcArea.top;
	BOOL bIsTop2 = rcCompareTo.top == rcArea.top;
	if (bIsTop1 && !bIsTop2) {
		return TRUE;
	} else if (!bIsTop1 && bIsTop2) {
		return FALSE;
	}

	// We prefer the "preferred size" over not the "preferred size"
	if (bCompareIsPreferredSize && !bCompareToIsPreferredSize) {
		// The first is the preferred size and the second is not so the first is better
		return TRUE;
	} else if (!bCompareIsPreferredSize && bCompareToIsPreferredSize) {
		// The first is not the preferred size and the second is so the second is better
		return FALSE;
	}

	// We prefer the bounding area to span the bottom
	BOOL bIsBotSpan1 = rcCompareBounding.bottom == rcArea.bottom && rcCompareBounding.left == rcArea.left && rcCompareBounding.right == rcArea.right;
	BOOL bIsBotSpan2 = rcCompareBoundingTo.bottom == rcArea.bottom && rcCompareBoundingTo.left == rcArea.left && rcCompareBoundingTo.right == rcArea.right;
	if (bIsBotSpan1 && !bIsBotSpan2) {
		return TRUE;
	} else if (!bIsBotSpan1 && bIsBotSpan2) {
		return FALSE;
	}

	// We prefer higher up to further down
	long nYPos1 = rcCompare.top;
	long nYPos2 = rcCompareTo.top;
	if (nYPos1 < nYPos2) {
		// The first is higher up so the first is better
		return TRUE;
	} else if (nYPos1 > nYPos2) {
		// The first is further down so the second is better
		return FALSE;
	}

	// We prefer further left to further right
	long nXPos1 = rcCompare.left;
	long nXPos2 = rcCompareTo.left;
	if (nXPos1 < nXPos2) {
		// The first is futher left so the first is better
		return TRUE;
	} else if (nXPos1 > nXPos2) {
		// The first is further right so the second is better
		return FALSE;
	}

	// We prefer bigger bounding to smaller bounding
	long nAreaBound1 = rcCompareBounding.Width() & rcCompareBounding.Height();
	long nAreaBound2 = rcCompareBoundingTo.Width() & rcCompareBoundingTo.Height();
	if (nAreaBound1 > nAreaBound2) {
		return TRUE;
	} else if (nAreaBound1 < nAreaBound2) {
		return FALSE;
	}

	// We prefer bigger to smaller
	long nArea1 = rcCompare.Width() & rcCompare.Height();
	long nArea2 = rcCompareTo.Width() & rcCompareTo.Height();
	if (nArea1 > nArea2) {
		return TRUE;
	} else if (nArea1 < nArea2) {
		return FALSE;
	}

	// We prefer closer to square over more rectangular
	long nSquareness1 = MulDiv(10000, min(rcCompare.Width(), rcCompare.Height()), max(rcCompare.Width(), rcCompare.Height()));
	long nSquareness2 = MulDiv(10000, min(rcCompareTo.Width(), rcCompareTo.Height()), max(rcCompareTo.Width(), rcCompareTo.Height()));
	if (nSquareness1 < nSquareness2) {
		// The first is more square than the second, so the first is better
		return TRUE;
	} else if (nSquareness1 > nSquareness2) {
		// The first is less square than the second, so the second is better
		return FALSE;
	}

	// If we made it here, the two rects are equivalent according to our criteria 
	// so it shouldn't matter what we return
	return FALSE;
}

void CEMNDetail::InvalidateLinkedDetailCacheIfExists(CEMNDetail* pDetail, BOOL bAssertIfFound)
{
	//
	// (c.haag 2007-10-30 17:30) - PLID 27914 - If pDetail exists in the linked
	// detail cache, then it will be invalidated.
	//
	if (m_bLinkedDetailsCached) {
		const long nLinkedDetails = m_arLinkedDetails.GetSize();
		for (long j=0; j < nLinkedDetails; j++) {
			if (pDetail == m_arLinkedDetails[j].pDetail) {  // Added brackets to keep j.jones happy
				if (bAssertIfFound) {
					ASSERT(FALSE);
				}
				// We found the detail. Invalidate the cache because that's what
				// we exist to do.
				InvalidateLinkedDetailCache();
			}
		}
	}
}

BOOL CompareDetailArrays(const CArray<CEMNDetail*,CEMNDetail*>& x,
						 const CArray<CEMNDetail*,CEMNDetail*>& y)
{
	//
	// (c.haag 2007-10-30 17:42) - PLID 27914 - Compares two detail arrays.
	// Only used in AssertLinkedDetailCacheUpToDate().
	//
	if (x.GetSize() != y.GetSize()) {
		return FALSE;
	}

	CArray<CEMNDetail*,CEMNDetail*> a1, a2;
	a1.Copy(x);
	a2.Copy(y);

	while (a1.GetSize() > 0) {
		CEMNDetail* p = a1[0];
		BOOL bFound = FALSE;
		for (int i=0; i < a2.GetSize() && !bFound; i++) {
			if (a2[i] == p) {
				a1.RemoveAt(0);
				a2.RemoveAt(i);
				bFound = TRUE;
			}
		}
		if (!bFound) {
			return FALSE;
		}
	}

	return (a2.GetSize() == 0) ? TRUE : FALSE;
}

void CEMNDetail::AssertLinkedDetailCacheUpToDate()
{
	//
	// (c.haag 2007-10-30 17:34) - PLID 27914 - Compares the linked detail cache with
	// a calculated list of linked details. Returns TRUE if they match, FALSE if they
	// do not.
	// (c.haag 2008-11-25 09:52) - PLID 31693 - Updated code to support new structure of
	// m_arDetails
	//
	if (m_bLinkedDetailsCached && m_pParentTopic) {
		CArray<CEMNDetail*,CEMNDetail*> apLinked;
		CArray<LinkedDetailStruct,LinkedDetailStruct&> apCurrentList;
		CArray<CEMNDetail*,CEMNDetail*> apCurLinkedDetails;
		for (int i=0; i < m_arLinkedDetails.GetSize(); i++) {
			apCurLinkedDetails.Add(m_arLinkedDetails[i].pDetail);
		}

		// (c.haag 2007-10-31 08:58) - Preserve what's in the cache now
		apCurrentList.Append(m_arLinkedDetails);

		m_bLinkedDetailsCached = false;
		GetLinkedDetails(apLinked, m_pParentTopic->GetParentEMN());
		m_bLinkedDetailsCached = true;

		if (!CompareDetailArrays(apCurLinkedDetails, apLinked)) {
			ASSERT(FALSE);
		}

		// (c.haag 2007-10-31 08:56) - Now restore what was in the cache earlier. This
		// needs to be a zero-sum function. Even if the cache is wrong, the goal is to
		// ASSERT that it's wrong, not to fix it.
		m_arLinkedDetails.RemoveAll();
		m_arLinkedDetails.Append(apCurrentList);
	}
}

/* for debugging the selection algorithm
#ifdef _DEBUG
void DrawRectangle(CDC *pdc, const CRect &rc, const COLORREF clr, long nWidth)
{
	CPen pn(PS_SOLID, nWidth, clr);
	CPen *ppnOldPen = pdc->SelectObject(&pn);
	pdc->MoveTo(rc.TopLeft());
	pdc->LineTo(rc.left, rc.bottom);
	pdc->LineTo(rc.BottomRight());
	pdc->LineTo(rc.right, rc.top);
	pdc->LineTo(rc.TopLeft());
	pdc->SelectObject(ppnOldPen);
}

const long gtcnTotal = 20;

COLORREF gtarynColor[gtcnTotal] = {
	RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255), RGB(255, 255, 0), RGB(255, 0, 255), RGB(0, 255, 255),
	RGB(192, 0, 0), RGB(0, 192, 0), RGB(0, 0, 192), RGB(192, 192, 0), RGB(192, 0, 192), RGB(0, 192, 192),
	RGB(90, 0, 0), RGB(0, 90, 0), RGB(0, 0, 90), RGB(90, 90, 0), RGB(90, 0, 90), RGB(0, 90, 90),
	RGB(255, 128, 128), RGB(128, 255, 128), 
};

long gtarynWidth[gtcnTotal] = {
	40, 38, 36, 34, 32, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2
};

long gtnCount = 0;
#endif
//*/


// Always sets rcBestOption to what it can come up with as the best fit, even if something is getting cut off.
// NOTE: The rcBestOption rect will be a client rect, not a full window rect.  Before using it to set the size 
// or position of a window, convert it to a full window rect using CalcWindowRect(prc, CWnd::adjustOutside).
// Returns TRUE if it fits with no cutting off
// Returns FALSE if the returned CRect has some parts outside of the given region
BOOL CEMNDetail::SearchRegionForBestFit(IN const CRgn &rgnToFitInto, OUT CRect &rcBestOption, IN const CRect &rcArea)
{
	// Calculate the inverse of the region
	CRgn rgnInverse;
	{
		CRect rcOuter;
		rgnToFitInto.GetRgnBox(&rcOuter);
		rcOuter.InflateRect(1, 1, 1, 1);
		CRgn rgnOuter;
		rgnOuter.CreateRectRgnIndirect(rcOuter);
		rgnInverse.CreateRectRgn(0,0,1,1);
		rgnInverse.CombineRgn(&rgnOuter, (CRgn *)&rgnToFitInto, RGN_XOR);
	}

	CRect rcBestSoFar;
	rcBestSoFar.SetRectEmpty();
	CRect rcBestBounding;
	rcBestBounding.SetRectEmpty();
	BOOL bBestWasPreferredSize = FALSE;

	long nBytes = rgnToFitInto.GetRegionData(NULL, 0);
	RGNDATA *prgndata = (RGNDATA *)new BYTE[nBytes];
	if (rgnToFitInto.GetRegionData(prgndata, nBytes)) {
		for (DWORD i=0; i<prgndata->rdh.nRgnSize; i += sizeof(RECT)) {
			RECT &rrc = *(RECT *)(prgndata->Buffer + i);
			CRect rcThisBounding(rrc);
			CalcMaxRectContainingRectInRgn(rgnToFitInto, rgnInverse, rrc, rcThisBounding);
			
			/* for debugging the selection algorithm
			#ifdef _DEBUG
			{
				CDC dc;
				dc.Attach(::GetDC(NULL));
				DrawRectangle(&dc, rcThisBounding + CPoint(1600, 0), gtarynColor[gtnCount%gtcnTotal], gtarynWidth[gtnCount%gtcnTotal]);
				gtnCount++;
				::ReleaseDC(NULL, dc.Detach());
			}
			#endif
			//*/

			CSize szIdeal(rcThisBounding.Width(), rcThisBounding.Height());
			BOOL bThisIsPreferredSize = m_pEmrItemAdvDlg->RepositionControls(szIdeal, TRUE);
			CalcClientFromWindow(m_pEmrItemAdvDlg, &szIdeal);
			CRect rcThisOption(rcThisBounding.TopLeft(), szIdeal);
			
			// If this option is the best so far, replace the best variables with these
			if (IsRectBetterFit(rcThisOption, bThisIsPreferredSize, rcThisBounding, rcBestSoFar, bBestWasPreferredSize, rcBestBounding, rgnToFitInto, rgnInverse, rcArea)) {
				rcBestSoFar = rcThisOption;
				rcBestBounding = rcThisBounding;
				bBestWasPreferredSize = bThisIsPreferredSize;
			}
		}

		// (j.jones 2005-10-20 10:45) - PLID 18004 - if we can't find a rect, generate the ideal size,
		// and add to the bottom of the screen
		if(rcBestSoFar.IsRectNull()) {
			CSize szIdeal(rcArea.Width(), rcArea.Height());
			BOOL bThisIsPreferredSize = m_pEmrItemAdvDlg->RepositionControls(szIdeal, TRUE);
			CalcClientFromWindow(m_pEmrItemAdvDlg, &szIdeal);
			CRect rcThisOption(CPoint(rcArea.left,rcArea.bottom + gc_nStaticBorderWidth), szIdeal);
			rcBestSoFar = rcThisOption;
		}
		delete []prgndata;
		
		BOOL bFitInRegion;
		if (!rcBestSoFar.IsRectNull()) {
			// See if it fits in our region
			bFitInRegion = rgnToFitInto.RectInRegion(rcBestSoFar) && !rgnInverse.RectInRegion(rcBestSoFar);
			if (!bFitInRegion) {
				/*
				// It doesn't, so we need to find the least-reduced rect that DOES fit in the region.
				// TODO: (b.cardillo 2004-04-08 10:12) We still need to do this if we want to be optimal, 
				// but since that's a complex lgorithm for very little benefit and we have extremely 
				// little time right now due to the impending show, we're taking the relatively 
				// inexpensive and very quick shortcut of just making sure the rect is within rcArea.
				if (rcBestSoFar.right > rcArea.right) {
					rcBestSoFar.right = rcArea.right;
				}
				if (rcBestSoFar.bottom > rcArea.bottom) {
					rcBestSoFar.bottom = rcArea.bottom;
				}
				*/
			}
		} else {
			// Couldn't find ANY part of the region that we could even put PART of the rect in
			bFitInRegion = FALSE;
			//rcBestSoFar.SetRect(rcArea.left, rcArea.top, rcArea.left, rcArea.top);
		}
		// Finally return
		rcBestOption = rcBestSoFar;
		return bFitInRegion;
	} else {
		delete []prgndata;
		ThrowNxException("SearchRegionForBestFit: Could not get region data!");
	}
}

// (c.haag 6-5-2004 21:33) PLID 12400 - This function will destroy the advanced item dialog
// and then re-create it. This is useful when the type of item has changed, and
// the old advanced dialog no longer applies.
void CEMNDetail::ReloadEMRItemAdvDlg()
{
	if (m_pEmrItemAdvDlg != NULL)
	{
		//TES 9/16/2009 - PLID 35529 - If we're windowless, then we don't own our EmrItemAdvDlg
		if(!m_bWindowless) {
			dynamic_ptr<CEmrTopicWnd> pParentTopicWnd;
			if (m_pEmrItemAdvDlg->m_hWnd) {
				pParentTopicWnd = m_pEmrItemAdvDlg->GetParent();
				m_pEmrItemAdvDlg->DestroyWindow();
			}
			delete m_pEmrItemAdvDlg;
			m_pEmrItemAdvDlg = NULL;
			if(pParentTopicWnd) EnsureEmrItemAdvDlg(pParentTopicWnd, TRUE);
		}
		else {
			//TES 9/16/2009 - PLID 35529 - This will just reassign the global m_pEmrItemAdvDlg to us
			EnsureEmrItemAdvDlg(NULL, TRUE);
		}
	}
}

// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicit CEmrTopicWnd parent
void CEMNDetail::EnsureEmrItemAdvDlg(IN CEmrTopicWnd* pParentWindow, BOOL bForceLoadFromAdmin /*= FALSE*/)
{
	try {

		if (m_pEmrItemAdvDlg == NULL) {

			// (a.walling 2012-02-22 14:53) - PLID 48320 - Ensure we have a parent, or use the hidden detail parent wnd if not
			CWnd* pUseParent = pParentWindow->GetSafeHwnd() ? pParentWindow : &GetHiddenDetailParentWnd();

			if(m_bWindowless) {
				//TES 9/16/2009 - PLID 35529 - If we're windowless, we just want to borrow the global narrative dialog.
				// We should NEVER be set to windowless unless a.) we weren't given a parent, and b.) we are a 
				// narrative (every other type should never have EnsureEmrItemAdvDlg called unless it's actually
				// being created as a window).
				if(pParentWindow != NULL) {
					AfxThrowNxException("Parent window passed to windowless EMNDetail");
				}
				if(m_EMRInfoType != eitNarrative) {
					AfxThrowNxException("EnsureEmrItemAdvDlg called for windowless non-narrative EMNDetail");
				}
				m_pEmrItemAdvDlg = GetGlobalEmrItemAdvNarrativeDlg(this);
			}
			else
			{
				// Fill the data values
				// (j.jones 2010-03-23 12:16) - PLID 37318 - added override to NOT call EnsureLinkedSmartStampTableValid() in LoadContent()
				// (z.manning 2012-07-16 16:24) - PLID 33710 - Moved the LoadContent call before the call to CreateEmrItemAdvDlg
				// because otherwise load content (specifically the formula evaluation code) will think we have a valid adv
				// dialog even though it's still in the process of being created.
				LoadContent(bForceLoadFromAdmin, NULL, NULL, FALSE);

				// Create it
				// (a.walling 2007-08-09 12:39) - PLID 26781 - This can fail if we run out of GDI objects
				m_pEmrItemAdvDlg = CreateEmrItemAdvDlg();
				if (m_pEmrItemAdvDlg) {
					//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
					// SetReadOnly() to ReflectReadOnlyStatus()
					m_pEmrItemAdvDlg->ReflectReadOnlyStatus(m_bReadOnly);
					//We're "ghostly" if we're on a template, and were spawned by something.
					m_pEmrItemAdvDlg->SetGhostly(m_bIsTemplateDetail && m_sai.nSourceActionID != -1);
					m_pEmrItemAdvDlg->SetCanEditContent(m_bAllowEdit);
					m_pEmrItemAdvDlg->SetIsTemplate(m_bIsTemplateDetail);
					ASSERT(m_pEmrItemAdvDlg);
					if (!m_pEmrItemAdvDlg) {
						ThrowNxException("CEMNDetail: Could not create emr item advanced dialog for info type %li!", m_EMRInfoType);
					}
					// Create the window
					CRect rcClientArea(m_rcDefaultClientArea);
					// Move and position it if we were given a rect
					// CreateWithClientArea will adjust it to window coordinates as appropriate
					if (!m_rcDefaultClientArea.IsRectNull()) {
						m_bNeedReposition = FALSE;
					} else {
						rcClientArea.SetRect(0, 0, 1, 1);
						m_bNeedReposition = TRUE;
					}

					// Change the styles appropriately depending on whether we're in edit mode or not
					//(e.lally 2012-04-16) PLID 49719 - Only use visible style if the detail is in fact visible
					DWORD dwIsVisible = GetVisible() ? WS_VISIBLE : 0;
					if (m_bAllowEdit) {
						m_pEmrItemAdvDlg->CreateWithClientArea(WS_EX_CLIENTEDGE, dwIsVisible | WS_THICKFRAME, rcClientArea, pUseParent);
					} else {
						m_pEmrItemAdvDlg->CreateWithClientArea(WS_EX_STATICEDGE, dwIsVisible, rcClientArea, pUseParent);
					}

					m_pEmrItemAdvDlg->SetMergeNameConflict(m_bMergeNameConflicts);
					// Make sure it shows the current value state
					BOOL bModified = m_bModified;

					if (eitTable == m_EMRInfoType) {
						// (c.haag 2007-08-20 14:27) - PLID 27126 - If we get here, it means we're about to call
						// ReflectCurrentState on the table, which will fill every datalist cell with data. This was
						// already done when the dialog was created and ReflectCurrentContent was called on it. To
						// avoid redundant datalist populating, we set a flag in the table to have it not populate
						// the datalist. When we're done, we set it back to what it was.
						CEmrItemAdvTableDlg* pTableDlg = (CEmrItemAdvTableDlg*)m_pEmrItemAdvDlg;
						CEmrItemAdvTableDlg::EReflectCurrentStateDLHint rcshintOld = pTableDlg->GetReflectCurrentStateDLHint();
						pTableDlg->SetReflectCurrentStateDLHint(CEmrItemAdvTableDlg::eRCS_NoDatalistUpdate); 
						pTableDlg->ReflectCurrentState();
						pTableDlg->SetReflectCurrentStateDLHint(rcshintOld);
					} 
					else if (eitNarrative == m_EMRInfoType) {
						// (a.walling 2009-12-08 14:13) - PLID 36225 - Ensure the narrative fields are up to date
						m_pEmrItemAdvDlg->ReflectCurrentState();
						UpdateNarrativeFields();
					} else {
						m_pEmrItemAdvDlg->ReflectCurrentState();
					}

					if (m_clrHighlight != 0 && (m_EMRInfoType == 2 || m_EMRInfoType == 3)) {
						((CEmrItemAdvListDlg *)(m_pEmrItemAdvDlg))->ChangeHilightColor(m_clrHighlight);
					}

					// (j.jones 2010-03-23 12:23) - PLID 37318 - ensure our linked table is still valid after LoadContent() was called
					EnsureLinkedSmartStampTableValid();

					//Calling ReflectCurrentState() may make us think that we're modified, but we're not.
					m_bModified = bModified;
					m_pEmrItemAdvDlg->m_bIsLoading = FALSE;
				}
			}
		}

		if(pParentWindow && m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
			if(m_pEmrItemAdvDlg->GetParent() != pParentWindow)
				m_pEmrItemAdvDlg->SetParent(pParentWindow);
		}
	} NxCatchAllThrow("Error in CEMNDetail::EnsureEmrItemAdvDlg");

}

// (z.manning 2012-06-22 16:28) - PLID 48138 - Added function to ensure topic wnd and adv dialog
void CEMNDetail::EnsureTopicWndAndItemAdvDlg()
{
	if(GetParentTopic() != NULL && GetParentEMN() != NULL && GetParentEMN()->GetInterface() != NULL && GetParentEMN()->GetInterface()->GetEmrFrameWnd() != NULL)
	{
		CEmrTopicView *pTopicView = GetParentEMN()->GetInterface()->GetEmrFrameWnd()->GetEmrTopicView(GetParentEMN());
		if(pTopicView != NULL)
		{
			CEmrTopicWndPtr pTopicWnd = pTopicView->EnsureTopic(GetParentTopic());
			if(pTopicWnd != NULL)
			{
				EnsureEmrItemAdvDlg(pTopicWnd.get());
				pTopicWnd->RepositionDetailsInTopicByInfoID(-1, FALSE, this);
			}
		}
	}
}

BOOL IsValidRegionType(int nRegionType)
{
	switch (nRegionType) {
	case SIMPLEREGION:
	case COMPLEXREGION:
		return TRUE;
	case NULLREGION:
	case RGN_ERROR:
		return FALSE;
	default:
		// Invalid parameter given
		ASSERT(FALSE);
		return FALSE;
	}
}

BOOL AddRectToRegion(IN OUT CRgn &rgn, IN const CRect &rcToAdd)
{
	CRgn rgnRectToAdd;
	if (rgnRectToAdd.CreateRectRgnIndirect(rcToAdd)) {
		CRgn rgnNew;
		if (rgnNew.CreateRectRgn(0,0,1,1)) {
			if (IsValidRegionType(rgnNew.CombineRgn(&rgn, &rgnRectToAdd, RGN_OR))) {
				rgn.DeleteObject();
				rgn.Attach(rgnNew.Detach());
				return TRUE;
			} else {
				// The combined region was invalid
				return FALSE;
			}
		} else {
			// Couldn't create the temp output region
			return FALSE;
		}
	} else {
		// Couldn't create the rectangular region to add
		return FALSE;
	}
}

// (a.walling 2012-06-22 14:01) - PLID 51150 - dead code
//CRect CEMNDetail::ShowItemDialog(IN CWnd* pParentWindow)

// (b.savon 2012-06-07 14:53) - PLID 49144 - Utility function for device imports
// This function will return TRUE if the area is clear to place our side by side image detail
// It will return FALSE if the area has any part of the detail within it.
BOOL CEMNDetail::IsSafeMove(CEmrTopicWnd* pTopicWnd, CRect rectDetail)
{
	//	Store the Candidates bounding coordinates to check for detail intersection
	long nCandidateTopLeftX = rectDetail.TopLeft().x;
	long nCandidateTopLeftY = rectDetail.TopLeft().y;
	long nCandidateBottomRightX = rectDetail.BottomRight().x;
	long nCandidateBottomRightY = rectDetail.BottomRight().y;
	long nCandidateTopRightX = nCandidateBottomRightX;
	long nCandidateTopRightY = nCandidateTopLeftY;
	long nCandidateBottomLeftX = nCandidateTopLeftX;
	long nCandidateBottomLeftY = nCandidateBottomRightY;

	// Cycle through all the details on our topic and check to make sure we don't have someone in the 
	// way to place our second image detail
	for(long i = 0; i < pTopicWnd->GetTopic()->GetEMNDetailCount(); i++) {
		CEMNDetail *pDetail = pTopicWnd->GetTopic()->GetDetailByIndex(i);

		// If the detail is positioned 
		if(!pDetail->m_rcDefaultClientArea.IsRectNull()) {
			//If the top left or bottom right point of the detail is within the rectDetail area, it's not safe.
			//
			//
			//				-----------------
			//				|		---		|
			//				|	   |   |	|
			//			-------		---		|
			//			|	  |  rectDetail	|
			//			|	  |				|
			//			-------				|
			//				|	  _____		|
			//				-----|-----|-----
			//					 |_____|	
			//
			//	
			CRect rectPositionedDetail = pDetail->GetClientArea();
			long nTopLeftX = rectPositionedDetail.TopLeft().x;
			long nTopLeftY = rectPositionedDetail.TopLeft().y;
			long nBottomRightX = rectPositionedDetail.BottomRight().x;
			long nBottomRightY = rectPositionedDetail.BottomRight().y;
			long nTopRightX = nBottomRightX;
			long nTopRightY = nTopLeftY;
			long nBottomLeftX = nTopLeftX;
			long nBottomLeftY = nBottomRightY;

			// Check if we have an item where we want to put the second [right] PDF (side by side)
			// If we do, tell the caller this isn't a safe place to be, and place it on the next vertical below the 
			// lowest detail.

			// If any corner of the current detail is within our candidate area, tell the caller were not safe.
			if( ( nTopLeftY <= rectDetail.bottom && nTopLeftY >= rectDetail.top && nTopLeftX >= rectDetail.left && nTopLeftX <= rectDetail.right) ||
				( nBottomRightY <= rectDetail.bottom && nBottomRightY >= rectDetail.top && nBottomRightX >= rectDetail.left && nBottomRightX <= rectDetail.right) ||
				( nTopRightY <= rectDetail.bottom && nTopRightY >= rectDetail.top && nTopRightX >= rectDetail.left && nTopRightX <= rectDetail.right) ||
				( nBottomLeftY <= rectDetail.bottom && nBottomLeftY >= rectDetail.top && nBottomLeftX >= rectDetail.left && nBottomLeftX <= rectDetail.right) ) {
				return FALSE;
			}

			// If any corner of the candidate detail is within our current detail area, tell the caller were not safe.
			if( ( nCandidateTopLeftY <= rectPositionedDetail.bottom && nCandidateTopLeftY >= rectPositionedDetail.top &&
				  nCandidateTopLeftX >= rectPositionedDetail.left && nCandidateTopLeftX <= rectPositionedDetail.right ) ||
				( nCandidateBottomRightY <= rectPositionedDetail.bottom && nCandidateBottomRightY >= rectPositionedDetail.top &&
				  nCandidateBottomRightX >= rectPositionedDetail.left && nCandidateBottomRightX <= rectPositionedDetail.right) ||
				( nCandidateTopRightY <= rectPositionedDetail.bottom && nCandidateTopRightY >= rectPositionedDetail.top &&
				  nCandidateTopRightX >= rectPositionedDetail.left && nCandidateTopRightX <= rectPositionedDetail.right ) ||
				( nCandidateBottomLeftY <= rectPositionedDetail.bottom && nCandidateBottomLeftY >= rectPositionedDetail.top &&
				  nCandidateBottomLeftX >= rectPositionedDetail.left && nCandidateBottomLeftX <= rectPositionedDetail.right) ){
				return FALSE;
			}

			// If our current detail is wider than the candidate and within our candidate area, tell the caller were not safe.
			if( ( ((rectPositionedDetail.top <= rectDetail.bottom && rectPositionedDetail.top >= rectDetail.top) || 
				   (rectPositionedDetail.bottom <= rectDetail.bottom && rectPositionedDetail.bottom >= rectDetail.top))&& 
				   rectPositionedDetail.left <= rectDetail.left && rectPositionedDetail.right >= rectDetail.right ) ){
				return FALSE;
			}

			// If our current detail is taller than the candidate and within our candidate area, tell the caller were not safe.
			if( ( ((rectPositionedDetail.left >= rectDetail.left && rectPositionedDetail.left <= rectDetail.right) ||
				   (rectPositionedDetail.right >= rectDetail.left && rectPositionedDetail.right <= rectDetail.right)) &&
				   rectPositionedDetail.top <= rectDetail.top && rectPositionedDetail.bottom >= rectDetail.bottom) ){
				return FALSE;
			}

		}
	}

	// We ran the gauntlet and are safe to place ourselves there.
	return TRUE;
}//END BOOL CEMNDetail::IsSafeMove(CEmrTopicWnd* pTopicWnd, CRect rectDetail)

// Shows this object's window in the best possible spot within the given region.
// Returns TRUE if the window fit well within the region.
// Returns FALSE if the window extended outside the region.
// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicit CEmrTopicWnd parent
BOOL CEMNDetail::ShowItemDialogInRgn(IN CEmrTopicWnd* pParentWindow, IN OUT CRgn &rgn, IN OUT CRect &rcArea, OPTIONAL OUT CRect *prc, 
									 const BOOL bForceRecalc, OPTIONAL IN LPCRECT prcOverrideRect /*= NULL*/, 
									 OPTIONAL IN const DeviceImportSettings &disSettings /* = DeviceImportSettings(gc_nStaticBorderWidth, gc_nStaticBorderWidth)*/ )
{
	try {
		// Make sure the window is created
		EnsureEmrItemAdvDlg(pParentWindow);

		if (!prcOverrideRect && (m_bNeedReposition || bForceRecalc)) {
			
			// Find the best spot for this window within the given region
			CRect rc;
			BOOL bFitWasAppealing = SearchRegionForBestFit(rgn, rc, rcArea);

			// See if we got a valid rect at all
			if (rc.IsRectNull()) {
				ASSERT(FALSE);
			}

			// (b.savon 2012-06-06 12:18) - PLID 49144 - Position device import image details to be
			// side by side.
			//If were importing from a device, let's position them side by side (2 std. PDF reports)
			if( IsDeviceImport() ){
				CEmrTopicWnd* pTopic = (CEmrTopicWnd*)pParentWindow;

				long nDetailWidth = rc.right - rc.left;
				long nDetailHeight = rc.bottom - rc.top;

				/* This is the first image  */
				/*			OR				*/
				/* Reset, we've gone too far.  This needs to be on a new vertical */
				/*			OR				*/
				/* We have another topic that was placed where we'd like to put our second image */

				//Get Candidate to check if any detail is within the area we'd like to place
				//the second image detail
				CRect rectDetail(disSettings.m_nLeft + gc_nStaticBorderWidth,				/*Left*/
								 disSettings.m_nDeviceTop,									/*Top*/
								 disSettings.m_nLeft + gc_nStaticBorderWidth + nDetailWidth,/*Right*/
								 disSettings.m_nDeviceTop + nDetailHeight					/*Bottom*/
								); 

				/* NOTE: 11 is roughly the border width when in Edit Mode*/
				if( disSettings.m_nLeft < 700 /* Smaller than a PDF*/ ||
					disSettings.m_nLeft > 1400 /*Roughly 2 PDF Reports Side By Side*/ ||
					!IsSafeMove(pTopic, rectDetail)){
					rc.left = gc_nStaticBorderWidth;
					rc.top = disSettings.m_nTop + gc_nStaticBorderWidth;
				}else{
					rc.left = disSettings.m_nLeft + gc_nStaticBorderWidth;	
					rc.top = disSettings.m_nDeviceTop;
				}
				
				rc.right = rc.left + nDetailWidth;
				rc.bottom = rc.top + nDetailHeight;
			}

			// If the decided-upon rect goes outside rcArea, we need to expand any sides of rcArea necessary to 
			// contain that rect.  And whatever sides we expand, we need to also add the same exact dimensions 
			// to the rgn region.
			if (rc.left < rcArea.left) {
				AddRectToRegion(rgn, CRect(rc.left, rcArea.top, rcArea.left, rcArea.bottom));
				rcArea.left = rc.left;
			}
			if (rc.top < rcArea.top) {
				AddRectToRegion(rgn, CRect(rcArea.left, rc.top, rcArea.right, rcArea.top));
				rcArea.top = rc.top;
			}
			if (rc.right > rcArea.right) {
				AddRectToRegion(rgn, CRect(rcArea.right, rcArea.top, rc.right, rcArea.bottom));
				rcArea.right = rc.right;
			}
			if (rc.bottom > rcArea.bottom) {
				AddRectToRegion(rgn, CRect(rcArea.left, rcArea.bottom, rcArea.right, rc.bottom));
				rcArea.bottom = rc.bottom;
			}

			// Assert the given rect is within rcArea
			ASSERT(rc.left >= rcArea.left);
			ASSERT(rc.top >= rcArea.top);
			ASSERT(rc.right <= rcArea.right);
			ASSERT(rc.bottom <= rcArea.bottom);
			// Assert non-negative area
			ASSERT(rc.right >= rc.left);
			ASSERT(rc.bottom >= rc.top);
			
			// Now we can convert it from a client-sized rect to a full window rect
			m_pEmrItemAdvDlg->CalcWindowRect(&rc, CWnd::adjustOutside);

			// Show the window in that rect
			CRect rcCurrent;
			m_pEmrItemAdvDlg->GetWindowRect(&rcCurrent);
			if (rcCurrent.Width() != rc.Width() || rcCurrent.Height() != rc.Height()) {
				// Move, size, and show it
				UINT nShow = m_bVisible?SWP_SHOWWINDOW:0;
				m_pEmrItemAdvDlg->SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), nShow|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
			} else {
				// Just move and show it (no size, because the size hasn't changed).  As I recall, 
				// this is to help reduce flicker or something in that vein.
				UINT nShow = m_bVisible?SWP_SHOWWINDOW:0;
				m_pEmrItemAdvDlg->SetWindowPos(NULL, rc.left, rc.top, 0, 0, nShow|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
			}
			m_bNeedReposition = FALSE;
			m_pEmrItemAdvDlg->Invalidate(FALSE);

			// Optionally return the rect that we decided on
			if (prc) {
				*prc = rc;
			}

			// (a.walling 2007-08-08 10:37) - PLID 27017 - Update the m_rcDefaultClientArea
			// (previous behaviour: was not updated until saved normally!)
			CRect rcActualPos;
			m_pEmrItemAdvDlg->GetClientRect(rcActualPos);
			m_pEmrItemAdvDlg->ClientToScreen(rcActualPos);
			if (m_pEmrItemAdvDlg && m_pEmrItemAdvDlg->GetParent() && IsWindow(m_pEmrItemAdvDlg->GetParent()->GetSafeHwnd())) {
				m_pEmrItemAdvDlg->GetParent()->SendMessage(NXM_CONVERT_RECT_FOR_DATA, (WPARAM)&rcActualPos);

				m_rcDefaultClientArea = rcActualPos;
			}

			
			// Return TRUE if the window fit in the region, FALSE if it extended outside the region
			return bFitWasAppealing;
		} else {

			CRect rc;
			if (prcOverrideRect) {
				rc = *prcOverrideRect;
			} else {
				m_pEmrItemAdvDlg->GetWindowRect(&rc);
				pParentWindow->ScreenToClient(&rc);
			}
			// If the decided-upon rect goes outside rcArea, we need to expand any sides of rcArea necessary to 
			// contain that rect.  And whatever sides we expand, we need to also add the same exact dimensions 
			// to the rgn region.
			/*if (rc.left < rcArea.left) {
				AddRectToRegion(rgn, CRect(rc.left, rcArea.top, rcArea.left, rcArea.bottom));
				rcArea.left = rc.left;
			}
			if (rc.top < rcArea.top) {
				AddRectToRegion(rgn, CRect(rcArea.left, rc.top, rcArea.right, rcArea.top));
				rcArea.top = rc.top;
			}*/
			if (rc.right > rcArea.right) {
				AddRectToRegion(rgn, CRect(rcArea.right, rcArea.top, rc.right, rcArea.bottom));
				rcArea.right = rc.right;
			}
			if (rc.bottom > rcArea.bottom) {
				AddRectToRegion(rgn, CRect(rcArea.left, rcArea.bottom, rcArea.right, rc.bottom));
				rcArea.bottom = rc.bottom;
			}

			UINT nShow = m_bVisible?SWP_SHOWWINDOW:0;
			m_pEmrItemAdvDlg->SetWindowPos(NULL, 0, 0, 0, 0, nShow|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOOWNERZORDER);

			// Optionally return the rect that we decided on
			if (prc) {
				*prc = rc;
			}

			return TRUE;
		}
	} NxCatchAllThrow("Error in CEMNDetail::ShowItemDialogInRgn");
}

//Call this if this detail needs to reload its content when it next gets a chance.
// (j.jones 2011-07-22 17:26) - PLID 43504 - added optional connection pointer
void CEMNDetail::SetNeedContentReload(OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pCon;
	if(lpCon) {
		pCon = lpCon;
	}
	else {
		pCon = GetRemoteData();
	}

	//(e.lally 2007-02-09) PLID 24436 - We should only allow this flag to be 
		//set to true when it is an unlocked EMN, otherwise the content loading
		//can give locked EMNs the appearance of being modified.
	if(!GetIsOnLockedAndSavedEMN(pCon)) {
		m_bNeedToLoadContent = TRUE;
		// (j.jones 2007-07-25 17:54) - PLID 26810 - set m_bIsForcedReload = TRUE,
		// to indicate that an external process is forcing a content reload
		m_bIsForcedReload = TRUE;
	}
}

// (j.jones 2011-07-22 17:26) - PLID 43504 - added optional connection pointer
void CEMNDetail::SetNeedContentReload(BOOL bNeedsReload, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pCon;
	if(lpCon) {
		pCon = lpCon;
	}
	else {
		pCon = GetRemoteData();
	}

	//(e.lally 2007-02-09) PLID 24436 - We should only allow this flag to be 
		//set to true when it is an unlocked EMN, otherwise the content loading
		//can give locked EMNs the appearance of being modified.
	if(!(GetIsOnLockedAndSavedEMN(pCon) && bNeedsReload)) {
		m_bNeedToLoadContent = bNeedsReload;

		// (j.jones 2007-07-25 17:54) - PLID 26810 - set m_bIsForcedReload = TRUE,
		// to indicate that an external process is forcing a content reload
		m_bIsForcedReload = TRUE;
	}
}

BOOL CEMNDetail::GetNeedContentReload() const
{
	return m_bNeedToLoadContent;
}

// (j.jones 2007-07-25 17:32) - PLID 26810 - added m_bIsForcedReload 
BOOL CEMNDetail::GetIsForcedReload() const
{
	return m_bIsForcedReload;
}

// (c.haag 2006-03-31 11:33) - PLID 19387 - This makes it so when LoadContent
// is executed, it will also call SyncContentAndState.
void CEMNDetail::SetNeedSyncContentAndState()
{
	m_bNeedToSyncContentAndState = TRUE;
}

// (z.manning 2011-02-23 09:29) - PLID 42549 - Added params for the loader and a connection object
void CEMNDetail::LoadSmartStampTableContent(CEMNLoader *pEmnLoader, ADODB::_Connection *lpCon)
{
	if(!IsSmartStampTable()) {
		return;
	}

	if(pEmnLoader != NULL){
		pEmnLoader->EnsureLinkedSmartStampImageStatesLoaded(this, lpCon);
	}
	
	// (z.manning 2011-01-20 11:14) - PLID 42338 - We may have multiple images now
	CEmrDetailImageStampArray aryAllDetailStamps;
	m_arySmartStampImageDetails.GetAllDetailImageStampsInOrder(&aryAllDetailStamps);
	for(int nDetailImageStampIndex = 0; nDetailImageStampIndex < aryAllDetailStamps.GetSize(); nDetailImageStampIndex++)
	{
		EmrDetailImageStamp *pDetailImageStamp = aryAllDetailStamps.GetAt(nDetailImageStampIndex);
		if(pDetailImageStamp->eRule == esstsrIncreaseQuantity) {
			// (z.manning 2010-02-22 14:25) - For quantity based smart stamp table spawning,
			// we only want to load one row despite the number of detail stamps.
			TableRow *ptr = GetRowByStampID(pDetailImageStamp->nStampID);
			// (z.manning 2011-01-27 15:37) - PLID 42335 - We could have multiple instances of the same stamp
			// spread across multiple different images now. Let's check the newly added flag to determine if
			// the given stamp is in fact used in table data.
			if(ptr == NULL)
			{
				if(!pDetailImageStamp->bUsedInTableData) {
					// (z.manning 2011-03-01 16:20) - PLID 42335 - It's possible that the global stamp corresponding
					// to this detail stamp was never used in table data so we need to check that and if that's the
					// case we'll just arbitrarily use this one as the main stamp since it doesn't matter other than
					// the fact we need to choose a detail stamp to associate with the table row.
					if(!m_arySmartStampImageDetails.IsGlobalStampIDEverUsedInData(pDetailImageStamp->nStampID)) {
						// (z.manning 2011-03-02 15:31) - PLID 42335 - It's also possible that a quantity based stamp
						// was saved to data but the image it was loaded on is not being loaded again with this smart
						// stamp table in which case we need to update the table's state with the new detail stamp ID.
						pDetailImageStamp->bUsedInTableData = TRUE;
						long nDetailStampID = pDetailImageStamp->nID;
						if(nDetailStampID == -1) {
							nDetailStampID = pDetailImageStamp->nLoadedFromID;
						}
						if(nDetailStampID != -1) {
							UpdateTableStateWithNewDetailStampIDByStampID(pDetailImageStamp->nStampID, nDetailStampID);
						}
					}
				}

				if(pDetailImageStamp->bUsedInTableData) {
					//TES 3/17/2010 - PLID 37530 - Need to specify the stamp index
					AddSmartStampRow(pDetailImageStamp, m_arySmartStampImageDetails.GetStampIndexInDetailByType(pDetailImageStamp));
				}
			}
			else {
				pDetailImageStamp->bUsedInTableData = FALSE;
			}
		}
		// (j.jones 2010-04-07 14:15) - PLID 38069 - since we now have an option to
		// not add stamps as table rows, this else clause needs to specifically look
		// for the AddNewRow rule
		else if(pDetailImageStamp->eRule == esstsrAddNewRow) {
			//TES 3/17/2010 - PLID 37530 - Need to specify the stamp index
			AddSmartStampRow(pDetailImageStamp, m_arySmartStampImageDetails.GetStampIndexInDetailByType(pDetailImageStamp));
		}
	}
}

// (a.walling 2014-08-18 10:09) - PLID 63029 - Relatively complex logic to get the loader item or spawner item, and hold the loader mutex
struct LoaderSpawnerInfo
{
	// (c.haag 2007-04-26 11:01) - PLID 25790 - Try to load from the CEMNLoader
	// object if possible. Doing so is much faster than pulling data here.
	CEMNLoader* pEMNLoader;
	CEMNLoader::CEMRInfoItem* pLoaderInfoItem;
	BOOL bUseEMNLoader;

	// (c.haag 2007-08-06 12:15) - PLID 26954 - TRUE if we are using the EMN
	// spawner
	CEMNSpawner::CEMRInfoItem* pSpawnerInfoItem;
	BOOL bUseEMNSpawner;

	// (c.haag 2007-07-03 10:31) - PLID 26523 - We now request exclusive access
	// to EMR info data from the CEMNLoader
	// (c.haag 2007-07-05 14:53) - PLID 26595 - As well as details
	CEMNLoaderMultiMutex* pEMNLoaderMutex;

	~LoaderSpawnerInfo()
	{
		// (c.haag 2007-07-09 15:57) - PLID 26523 - We're all done with the EMR info and detail data
		if (pEMNLoaderMutex) {
			pEMNLoaderMutex->Unlock();
			// (c.haag 2007-07-10 09:00) - PLID 26523 - Now make it so if the silent exception is tripped,
			// it won't try to unlock it a second time
			pEMNLoaderMutex = NULL;
		}
	}

	LoaderSpawnerInfo(CEMNDetail* pThis, CEMNLoader* pTryEMNLoader)
		: pEMNLoader(pTryEMNLoader)
		, pLoaderInfoItem(NULL)
		, pSpawnerInfoItem(NULL)
		, bUseEMNLoader(FALSE)
		, bUseEMNSpawner(FALSE)
		, pEMNLoaderMutex(NULL)
	{
		// (c.haag 2007-07-02 08:48) - PLID 26515 - First, check the input parameter
		// for a valid loader object.
		//DRT 1/18/2008 - PLID 28603 - I've added hotspots to images, so if this is an
		//	image, those need to be loaded.  But we don't care about the EMRDataT records
		//	then.  So my change here will be to only check the "is loaded" variable for
		//	the specific data types that need it.  This code is copy/pasted below 3 times.
		//NOTE:  This then potentially changes the loading.  For types that don't use DataT
		//	or HotSpotsT (this is narratives, text, sliders), previously if the DataT map
		//	were not loaded, they would NOT use the EMN Loader.  Now they will.  c.haag and
		//	I believe this is the correct approach and really should have been done from 
		//	the start.
		if (NULL != pEMNLoader) {
			pEMNLoaderMutex = pEMNLoader->GetInternalDetailsAndEMRInfoMultiMutex();
			pEMNLoaderMutex->Lock();
			if (NULL != (pLoaderInfoItem = pEMNLoader->GetEmrInfoItem(pThis->m_nEMRInfoID))) {
				if (pLoaderInfoItem->m_DataType == eitSingleList || pLoaderInfoItem->m_DataType == eitMultiList || pLoaderInfoItem->m_DataType == eitTable) {
					if (pLoaderInfoItem->m_bDataItemsPopulated) {
						bUseEMNLoader = TRUE;
					}
				}
				else if (pLoaderInfoItem->m_DataType == eitImage) {
					if (pLoaderInfoItem->m_bHotSpotItemsPopulated) {
						bUseEMNLoader = TRUE;
					}
				}
				else {
					bUseEMNLoader = TRUE;
				}
			}
			if (!bUseEMNLoader) {
				pEMNLoaderMutex->Unlock();
				pEMNLoaderMutex = NULL;
			}
		}

		// (c.haag 2007-07-02 08:48) - PLID 26515 - If the following conditional is true,
		// it means that we either didn't get a loader object from an input parameter, or
		// we got it but it was invalid. Look to the detail itself for a preloader object.
		if (!bUseEMNLoader && pThis->m_pParentTopic) {
			// (c.haag 2007-05-30 10:16) - PLID 26175 - Try to get the loader object from
			// the parent topic before we try the EMN.
			EMRTopicLoadInfo* pLoadInfo = pThis->m_pParentTopic->GetLoadInfo();
			if (NULL != pLoadInfo && NULL != pLoadInfo->m_pLoader) {
				pEMNLoader = pLoadInfo->m_pLoader;
				pEMNLoaderMutex = pEMNLoader->GetInternalDetailsAndEMRInfoMultiMutex();
				pEMNLoaderMutex->Lock();
				if (NULL != (pLoaderInfoItem = pEMNLoader->GetEmrInfoItem(pThis->m_nEMRInfoID))) {
					if (pLoaderInfoItem->m_DataType == eitSingleList || pLoaderInfoItem->m_DataType == eitMultiList || pLoaderInfoItem->m_DataType == eitTable) {
						if (pLoaderInfoItem->m_bDataItemsPopulated) {
							bUseEMNLoader = TRUE;
						}
					}
					else if (pLoaderInfoItem->m_DataType == eitImage) {
						if (pLoaderInfoItem->m_bHotSpotItemsPopulated) {
							bUseEMNLoader = TRUE;
						}
					}
					else {
						bUseEMNLoader = TRUE;
					}
				}
				if (!bUseEMNLoader) {
					pEMNLoaderMutex->Unlock();
					pEMNLoaderMutex = NULL;
				}
			}
			else {
				CEMN* pEMN = pThis->m_pParentTopic->GetParentEMN();
				if (pEMN) {
					pEMNLoader = pEMN->GetEMNLoader();
					if (pEMNLoader) {
						pEMNLoaderMutex = pEMNLoader->GetInternalDetailsAndEMRInfoMultiMutex();
						pEMNLoaderMutex->Lock();
						if (NULL != (pLoaderInfoItem = pEMNLoader->GetEmrInfoItem(pThis->m_nEMRInfoID))) {
							if (pLoaderInfoItem->m_DataType == eitSingleList || pLoaderInfoItem->m_DataType == eitMultiList || pLoaderInfoItem->m_DataType == eitTable) {
								if (pLoaderInfoItem->m_bDataItemsPopulated) {
									bUseEMNLoader = TRUE;
								}
							}
							else if (pLoaderInfoItem->m_DataType == eitImage) {
								if (pLoaderInfoItem->m_bHotSpotItemsPopulated) {
									bUseEMNLoader = TRUE;
								}
							}
							else {
								bUseEMNLoader = TRUE;
							}
						}
						if (!bUseEMNLoader) {
							pEMNLoaderMutex->Unlock();
							pEMNLoaderMutex = NULL;
						}
					}
				}
			}
		}

		// (c.haag 2007-08-06 12:14) - PLID 26954 - If we can't use the EMN loader, see if 
		// we can use the EMN spawner
		if (!bUseEMNLoader) {
			if (NULL != pThis->GetEMNSpawner()) {
				pSpawnerInfoItem = pThis->GetEMNSpawner()->GetEmrInfoItem(pThis->m_nEMRInfoID);
				if (NULL != pSpawnerInfoItem) {
					bUseEMNSpawner = TRUE;
				}
			}
		}
	}
};


// (a.walling 2014-08-18 10:09) - PLID 63029 - Returns true if LoadContent can load from the given loader (or calculated loader / spawner)
bool CEMNDetail::IsAvailableInLoaderSpawner(CEMNLoader* pEMNLoader)
{
	LoaderSpawnerInfo info(this, pEMNLoader);
	return info.bUseEMNLoader || info.bUseEMNSpawner;
}

// (c.haag 2007-07-02 12:18) - PLID 26515 - Added an optional CEMNLoader object so that we
// can take advantage of it even if this detail has no parent topic
// (c.haag 2007-07-20 13:16) - PLID 26651 - Added a connection pointer
// (j.jones 2010-03-23 12:16) - PLID 37318 - added override for whether to call EnsureLinkedSmartStampTableValid()
// (j.jones 2010-06-21 15:47) - PLID 37981 - added ability to pass in pDefaultGenericTableContent
// (j.jones 2011-04-28 14:39) - PLID 43122 - added nProviderIDForFloatingData as an optional field
void CEMNDetail::LoadContent(BOOL bForceLoadFromAdmin /*= FALSE*/, CEMNLoader* pEMNLoader /*= NULL */,
							 ADODB::_Connection *lpCon, BOOL bEnsureSmartStampTableLinked /*= TRUE*/,
							 DevicePluginUtils::TableContent *pDefaultGenericTableContent /*= NULL*/,
							 long nProviderIDForFloatingData /*= -1*/)
{
	// (c.haag 2006-03-31 10:00) - This function reloads detail "content". By
	// "content", we really mean the boundaries of the data. For example, with
	// a slider, the "state" tells us where the slider is pointing, but the
	// "content" tells us the boundaries and increments of the slider. With
	// a list, the "state" tells us what boxes are checked. The "content" tells
	// us how many rows there are, and the name and style of each row.
	//
	// m_varState does not change as a consequence of LoadContent.
	//
	// The state and content are always perfectly matched to what you see on the
	// screen as long as ReflectCurrentContent and ReflectCurrentState are called
	// properly.
	//
	// This function does two things. First, it clears all the content (imagine a
	// table detail with no rows or columns, or a slider with no boundaries). Second,
	// it will load the content. In loading, it checks to see if we have an invalid
	// detail ID, or, we are forceably loading content from template data. If so, we
	// pull data from template tables. Otherwise, we pull it from the detail itself.
	// Keep that in mind when you don't want your content clobbered -- this will 
	// actually load from data if it needs to be loaded.
	//
	// As a consequence of this function, again, the state will not change, and
	// neither will what you see on the screen.
	//
	// A second consequence is that after this function has been called, there's
	// a possibility that the state will become invalid. For example, If you have
	// a slider range [1,100] and your state says we're pointing to 80, and then
	// you change the slider so its new range is [1,50], then your state is invalid.
	// Likewise, if a table loses a column, anything in m_varState that refers to
	// that column is invalid. You should have code between LoadContent and SaveDetail
	// that will ensure m_varState cannot be written to data in an invalid way. One
	// way to deal with this is to call SyncContentAndState, which ensures the state
	// is within the bounds of the content.
	//

	if(!m_bNeedToLoadContent) {
		//We don't need to load anything.
		return;
	}

	// (c.haag 2007-07-20 13:17) - PLID 26651 - Get a smart pointer to the connection
	// object
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	//We're loading the content now, so we don't need to load it again.
	m_bNeedToLoadContent = FALSE;

	// (j.jones 2007-07-25 17:29) - PLID 26810 - set m_bIsForcedReload to FALSE
	// when m_bNeedToLoadContent is set to FALSE, but cache it for the duration
	// of this function
	BOOL bIsForcedReload = m_bIsForcedReload;
	m_bIsForcedReload = FALSE;

	//We set this up here
	EmrInfoType eit = m_EMRInfoType;

	ClearContent();

	// (a.walling 2011-08-11 15:49) - PLID 44987 - Keep track of table row IDs and their TableRow pointers
	typedef std::map<TableRowID, TableRow*> TableRowMap;
	std::map<TableRowID, TableRow*> mapTableRows;
		
	if (m_nEMRInfoID != -1)
	{
		LoaderSpawnerInfo loaderSpawnerInfo(this, pEMNLoader);

		// (c.haag 2007-04-26 11:01) - PLID 25790 - Try to load from the CEMNLoader
		// object if possible. Doing so is much faster than pulling data here.
		pEMNLoader = loaderSpawnerInfo.pEMNLoader;
		CEMNLoader::CEMRInfoItem*& pLoaderInfoItem = loaderSpawnerInfo.pLoaderInfoItem;
		BOOL& bUseEMNLoader = loaderSpawnerInfo.bUseEMNLoader;
		// (c.haag 2007-08-06 12:15) - PLID 26954 - TRUE if we are using the EMN
		// spawner
		CEMNSpawner::CEMRInfoItem*& pSpawnerInfoItem = loaderSpawnerInfo.pSpawnerInfoItem;
		BOOL& bUseEMNSpawner = loaderSpawnerInfo.bUseEMNSpawner;
		CEMNLoaderMultiMutex*& pEMNLoaderMutex = loaderSpawnerInfo.pEMNLoaderMutex;
		
		// (z.manning 2011-10-25 12:49) - PLID 39401 - If this is an image then we need to load stamp exclusions.
		CSqlFragment sqlStampExclusions;
		if(m_EMRInfoType == eitImage && m_nEMRInfoMasterID > 0)
		{
			sqlStampExclusions = CSqlFragment(
				"SELECT StampID \r\n"
				"FROM EmrInfoStampExclusionsT \r\n"
				"WHERE EmrInfoMasterID = {INT} \r\n"
				, m_nEMRInfoMasterID);
		}

		// (c.haag 2007-07-03 10:34) - PLID 26523 - At this point, if bUseEMNLoader is TRUE,
		// then we have a lock on the EMR info mutex of the loader. We won't release it until
		// it's done. If an unexpected error occurs, however, we are stuck unless we somehow
		// release pEMRInfoMutex along the way. We can do that using a silent try-catch.
		{
			//TES 9/23/2004 - Check whether we're loading an existing detail, or generating a new one
			// (c.haag 2006-04-03 10:38) - PLID 19387 - If we have the detail element information in memory,
			// load from the admin.

			//double-check we have the correct InfoID, it could have changed due to inactivation/copying
			// (c.haag 2007-08-17 13:26) - PLID 27103 - This query is obselete under the new EMR design
			// where if an EMR Info item is edited, then a new version of it is made. The details that used
			// the old version still use the old version. If we edited an item on the fly, the new EMR info
			// ID will have already been assigned to the detail by now
			/*if(m_nEMRDetailID != -1) {
				_RecordsetPtr rs = CreateRecordset("SELECT EMRInfoID FROM EMRDetailsT WHERE ID = %li", m_nEMRDetailID);
				if(!rs->eof) {
					m_nEMRInfoID = AdoFldLong(rs, "EMRInfoID", -1);
				}
				rs->Close();
			}*/

			// (j.jones 2007-07-26 08:27) - PLID 26810 - we don't need to try and reload the name
			// and item type if we're only just now creating the detail, only when during a 
			// Forced Reload
			if(m_bIsTemplateDetail && m_nEMRTemplateDetailID != -1 && bIsForcedReload) {
				// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
				_RecordsetPtr rs = CreateParamRecordset(pCon, "SELECT ActiveEmrInfoID AS EMRInfoID FROM EmrInfoMasterT INNER JOIN EMRTemplateDetailsT ON EmrInfoMasterT.ID = EmrTemplateDetailsT.EmrInfoMasterID WHERE EmrTemplateDetailsT.ID = {INT}", m_nEMRTemplateDetailID);
				if(!rs->eof) {
					m_nEMRInfoID = AdoFldLong(rs, "EMRInfoID", -1);
				}
				rs->Close();
			}

			_RecordsetPtr prsContent;
			if((m_nEMRDetailID == -1 && m_nEMRTemplateDetailID == -1)
				|| bForceLoadFromAdmin) {
				//Load from Admin (will not load inactive items)
				// (c.haag 2007-02-05 13:15) - PLID 24423 - Cleaned up the sorting and did special handling for Current Medications.
				// A word about the sorting: We use to have a giant IF-ELSE clause that contained the same query twice with different
				// sort orders. Now we have two sort columns; A and B, whose content is defined by EmrDataT data and the AutoAlphabetize
				// flags. Lets look at it while ignoring Current Medications:
				//
				// ORDER BY (CASE WHEN @AutoAlphabetize = 1 THEN -1 ELSE EmrDataT.SortOrder END),
				//			(CASE WHEN @AutoAlphabetize = 1 THEN EmrDataT.Data ELSE '' END)
				//
				// If auto-alphabetize is zero, the data look like:
				//		1	|	''
				//		2	|	''
				//		3	|	''
				//
				// Because we're sorting on both columns, we are guaranteed that we will get our records in order of EmrDataT.SortOrder
				//
				// If auto-alphabetize is one, the data look like:
				//		-1	|	Advil
				//		-1	|	Bayer
				//		-1	|	Zyrtec
				//
				// Because were sorting on both columns, we are guaranteed the records will be in name order
				//
				// Now, Current Medications does not allow sorting by column names, so when AutoAlphabetize is true, we have to inject this logic:
				//
				//			CASE WHEN DataSubType = 1 AND ListType <> 2 THEN <do the opposite of what we normally do> ELSE <do what we normally do>
				//
				// When we do this, we only affect Current Medication non-rows. The recordset as a whole will look strangely jumbled,
				// but keep in mind that the only thing that matters are that rows are alphabetized, and columns are alphabetized.
				//
				// (c.haag 2007-04-27 10:02) -  PLID 25790 - Don't query data if we have access to a CEMNLoader object
				// (c.haag 2007-08-06 14:38) - PLID 26977 - Don't query data if we have an EMN spanwer object
				if (!bUseEMNLoader && !bUseEMNSpawner) {

					// (j.jones 2011-04-28 14:39) - PLID 43122 - If the currently selected EMN provider has ProvidersT.FloatEMRData = 1,
					// load the data from EmrProviderFloatDataT. If we are sorting alphabetically, sort the "floated" items first, 
					// then the regular items. If not sorting alphabetically, sort the "floated" items in Count order DESC, and then
					// the regular items in their normal sort order.
					
					if(pEMNLoader != NULL && nProviderIDForFloatingData == -1) {
						nProviderIDForFloatingData = pEMNLoader->m_nProviderIDForFloatingData;
					}
					if(nProviderIDForFloatingData == -1) {
						nProviderIDForFloatingData = GetProviderIDForFloatingData();
					}

					//TES 2/9/2007 - PLID 24671 - Added EmrDataGroupID
					// (c.haag 2007-04-03 08:59) - PLID 25468 - Expanded the special sorting described above to special Allergy items
					// (j.jones 2007-07-18 13:26) - PLID 26730 - load whether or not the info item has Info actions,
					// which it usually does not, such that we don't have to search for them later
					//DRT 7/26/2007 - PLID 26830 - Parameterized to speed execution.
					// (j.jones 2007-10-02 17:19) - PLID 26810 - added EmrInfoT.Name
					// (z.manning, 05/23/2008) - PLID - 30155 - Added EmrDataT.Formula and DecimalPlaces
					// (c.haag 2008-10-16 12:52) - PLID Added TableRowsAsFields
					// (z.manning 2009-01-15 16:02) - PLID 32724 - Added InputMask
					// (z.manning 2010-02-16 15:12) - PLID 37230 - ListSubType
					// (c.haag 2010-02-24 16:10) - PLID 21301 - AutoAlphabetizeDropdown
					//TES 2/22/2010 - PLID 37463 - Added SmartStampsLongForm and UseSmartStampsLongForm
					// (z.manning 2010-07-26 13:34) - PLID 39848 - Removed SmartStampsLongForm and UseSmartStampsLongForm
					// (z.manning 2011-03-11) - PLID 42778 - Added HasDropdownElements
					//TES 3/17/2011 - PLID 41108 - Added HasGlassesOrderData and GlassesOrderLens
					//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
					// (z.manning 2011-03-21 11:10) - PLID 30608 - Added autofill type
					// (z.manning 2011-05-26 14:50) - PLID 43865 - Added data flags
					// (z.manning 2011-09-19 17:14) - PLID 41954 - Dropdown separators
					// (z.manning 2011-11-07 10:41) - PLID 46309 - SpawnedItemsSeparator
					// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
					//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
					// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
					prsContent = CreateParamRecordset(pCon,
						"SELECT Name, DataSubType, "
						"(CASE WHEN EmrInfoT.ID IN (SELECT SourceID FROM EMRActionsT WHERE Deleted = 0 AND SourceType = 3) THEN 1 ELSE 0 END) AS HasInfoActions "
						"FROM EmrInfoT WHERE ID = {INT};\r\n"
						"\r\n"
						// (z.manning 2011-10-25 11:17) - PLID 39401 - Load stamp exclusions if necessary
						"{SQL}"
						"\r\n"
						"SELECT EmrDataT.ID, EmrDataT.EmrDataGroupID, EmrDataT.Data, EmrDataT.IsLabel, EmrDataT.ListType, EmrDataT.IsGrouped, EmrDataT.Formula, DecimalPlaces, EmrInfoT.DataType, EmrInfoT.DataSubType, EmrDataT.DataFlags, "
						// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
						"EmrDataT.AutoNumberType, EmrDataT.AutoNumberPrefix, "
						"EmrInfoT.BackgroundImageFilePath, EmrInfoT.BackgroundImageType, EmrInfoT.SliderMin, EmrInfoT.SliderMax, "
						"EmrInfoT.SliderInc, NULL AS SliderValue, EmrInfoT.LongForm, EmrDataT.LongForm AS DataLongForm, "
						"EmrInfoT.DataFormat, EmrInfoT.DataSeparator, EmrInfoT.DataSeparatorFinal, "
						"EmrInfoT.DisableTableBorder, EmrInfoT.TableRowsAsFields, "
						// (j.jones 2010-02-11 15:24) - PLID 37318 - load ChildEMRInfoMasterID and SmartStampsEnabled
						// (z.manning 2011-01-25 15:09) - PLID 42336 - Removed parent detail IDs
						"EmrInfoT.ChildEMRInfoMasterID, EmrInfoT.SmartStampsEnabled, NULL AS ChildEMRDetailID, NULL AS ChildEMRTemplateDetailID, "
						"CASE WHEN EmrInfoT.DataType = 2 OR EmrInfoT.DataType = 3 THEN (SELECT Min(CASE WHEN DestType = 3 OR DestType = 9 THEN 1 ELSE 2 END) AS MinActionType FROM EMRActionsT WHERE SourceType = 4 AND SourceID = EmrDataT.ID AND Deleted = 0) ELSE NULL END AS ActionsType, "						
						// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
						// (j.jones 2011-03-09 09:05) - PLID 42283 - added EMCodeUseTableCategories and Data_EMCodeCategoryID
						// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields, because they are now only calculated in the API, and not in Practice code
						//"EMRInfoMasterT.EMCodeCategoryID, EMRInfoMasterT.EMCodeUseTableCategories, EMRInfoMasterT.UseEMCoding AS Info_UseEMCoding, EMRInfoMasterT.EMCodingType, "
						// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
						"EmrDataT.InputMask, EmrDataT.ListSubType, EmrDataT.AutoAlphabetizeDropdown, "
						//DRT 1/22/2008 - PLID 28603 - Load the content for hotspots
						//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for HotSpots
						// (z.manning 2011-07-25 12:59) - PLID 44649 - Added ImageHotSpotID
						"EMRImageHotSpotsT.ID AS HotSpotID, EMRImageHotSpotsT.Data AS HotSpotData, EMRImageHotSpotsT.AnatomicLocationID AS HotSpotLocationID, LabAnatomyT.Description AS HotSpotLocation, EMRImageHotSpotsT.AnatomicQualifierID AS HotSpotQualifierID, AnatomyQualifiersT.Name AS HotSpotQualifier, EMRImageHotSpotsT.AnatomySide AS HotSpotSide, EMRImageHotSpotsT.EmrSpotGroupID AS HotSpotGroupID, EMRImageHotSpotsT.ImageHotSpotID, "
						// (a.walling 2008-07-01 09:15) - PLID 29271 - Preview Flags
						// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floating
						"EMRInfoT.PreviewFlags, Convert(bit, CASE WHEN FloatedItemsQ.Count Is Not Null THEN 1 ELSE 0 END) AS IsFloated, "
						// (a.walling 2013-02-28 17:13) - PLID 55390 - Avoid massive index scans and spooling and hash merges by limiting to individual seeks
						// based on the ListType of the Data when trying to determine whether dropdowns exist for this. Previously joining on the ad-hoc 
						// select dumps (potentially millions) of rows to a temp worktable and hashmatches. This replaces that with multiple seeks, unfortunately 2 per text/dropdown
						// item, but still better than before.
						// (a.walling 2013-07-02 14:57) - PLID 57401 - Indexed view for EMRTableDropdownInfoT's total/inactive counts
						"CONVERT(bit, "
							"CASE WHEN EmrDataT.ListType = 3 THEN "
								"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID AND TotalCount > InactiveCount) THEN 1 ELSE 0 END "
							"WHEN EmrDataT.ListType = 4 THEN "
								"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
							"ELSE 0 "
							"END "
						") AS HasActiveDropdownElements, "
						"CONVERT(bit, "
							"CASE WHEN EmrDataT.ListType IN (3,4) THEN "
								"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
							"ELSE 0 "
							"END "
						") AS HasDropdownElements, "
						"EmrInfoT.HasGlassesOrderData, EmrInfoT.GlassesOrderLens, EmrInfoT.InfoFlags, \r\n"
						"EmrDataT.GlassesOrderDataType, EmrDataT.GlassesOrderDataID, EmrDataT.AutofillType, DropdownSeparator, DropdownSeparatorFinal, SpawnedItemsSeparator, ParentLabelID, \r\n"
						"EmrInfoT.HasContactLensData, \r\n"
						// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType and UseWithWoundCareCoding
						"EmrDataT.WoundCareDataType, EmrInfoT.UseWithWoundCareCoding \r\n"
						"FROM EMRInfoT "
						"LEFT JOIN (SELECT * FROM EMRDataT WHERE Inactive = 0) EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID "
						"LEFT JOIN EMRInfoMasterT ON EMRInfoT.EMRInfoMasterID = EMRInfoMasterT.ID "
						// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of and the join to emrdatagroupst
						//DRT 1/22/2008 - PLID 28603 - Load the content for hotspots
						//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for hotspots
						"LEFT JOIN EMRImageHotSpotsT ON EMRInfoT.ID = EMRImageHotSpotsT.EMRInfoID "
						"LEFT JOIN LabAnatomyT ON EMRImageHotSpotsT.AnatomicLocationID = LabAnatomyT.ID "
						"LEFT JOIN AnatomyQualifiersT ON EMRImageHotSpotsT.AnatomicQualifierID = AnatomyQualifiersT.ID "
						// (j.jones 2011-04-28 14:39) - PLID 43122 - load the floating data info
						// (a.walling 2013-03-21 11:24) - PLID 55810 - The EmrProviderFloatDataT join can be skipped entirely if the providerid being scanned for is set to -1
						"LEFT JOIN (SELECT EMRDataGroupID, Count "
						"	FROM EmrProviderFloatDataT "
						"	WHERE ProviderID = {INT} AND {INT} <> -1) AS FloatedItemsQ ON EMRDataT.EmrDataGroupID = FloatedItemsQ.EMRDataGroupID "
						"WHERE EMRInfoT.ID = {INT} "
						"ORDER BY (CASE WHEN FloatedItemsQ.Count Is Not Null AND EMRInfoT.DataType IN (2,3) "
						"	THEN (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 0 THEN FloatedItemsQ.Count ELSE 1 END) "
						"	ELSE 0 END) DESC, "
						"	(CASE WHEN FloatedItemsQ.Count Is Not Null AND EMRInfoT.DataType IN (2,3) THEN -1 ELSE (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN EmrDataT.SortOrder ELSE -1 END ELSE EmrDataT.SortOrder END) END), "
						"	(CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN '' ELSE EmrDataT.Data END ELSE '' END) ",
						m_nEMRInfoID, sqlStampExclusions, nProviderIDForFloatingData, nProviderIDForFloatingData, m_nEMRInfoID);
				}
			}
			else {
				if(m_nEMRDetailID == -1 && m_nEMRTemplateDetailID != -1) {
					//Load from the Template Detail.
					// (c.haag 2007-04-26 11:01) - PLID 25790 - Don't query data if we have access to a CEMNLoader object
					// (c.haag 2007-08-06 14:38) - PLID 26977 - Don't query data if we have an EMN spanwer object
					if (!bUseEMNLoader && !bUseEMNSpawner) {

						// (j.jones 2011-04-28 14:39) - PLID 43122 - If the currently selected EMN provider has ProvidersT.FloatEMRData = 1,
						// load the data from EmrProviderFloatDataT. If we are sorting alphabetically, sort the "floated" items first, 
						// then the regular items. If not sorting alphabetically, sort the "floated" items in Count order DESC, and then
						// the regular items in their normal sort order.
						
						if(pEMNLoader != NULL && nProviderIDForFloatingData == -1) {
							nProviderIDForFloatingData = pEMNLoader->m_nProviderIDForFloatingData;
						}
						if(nProviderIDForFloatingData == -1) {
							nProviderIDForFloatingData = GetProviderIDForFloatingData();
						}

						//TES 6/29/2006 - Added slight optimization here: if this isn't a template, we don't need to do the (fairly slow)
						//calculation of what actions each element has.
						// (j.jones 2006-08-07 14:21) - PLID 21814 - ensured that inactive elements are not loaded
						// on templates nor when creating an EMN from a template
						// (c.haag 2007-02-05 13:28) - PLID 24423 - Cleaned up the sorting and did special handling for Current Medications
						//TES 2/9/2007 - PLID 24671 - Added EmrDataGroupID
						// (c.haag 2007-04-03 08:59) - PLID 25468 - Expanded the special sorting described above to special Allergy items
						// (j.jones 2007-07-18 13:26) - PLID 26730 - load whether or not the info item has Info actions,
						// which it usually does not, such that we don't have to search for them later
						//DRT 7/26/2007 - PLID 26830 - Parameterized to speed execution.
						// (j.jones 2007-10-02 17:19) - PLID 26810 - added EmrInfoT.Name
						// (z.manning, 05/23/2008) - PLID 30155 - Added EmrDataT.Formula and DecimalPlaces
						// (c.haag 2008-06-16 11:57) - PLID 30319 - Changed name calculation for text macros
						// (c.haag 2008-10-16 12:53) - PLID 30719 - TableRowsAsFields
						// (z.manning 2009-01-15 16:05) - PLID 32724 - Added InputMask
						// (z.manning 2010-02-16 15:15) - PLID 37230 - ListSubType
						//TES 2/22/2010 - PLID 37463 - Added SmartStampsLongForm and UseSmartStampsLongform
						// (c.haag 2010-02-24 16:10) - PLID 21301 - AutoAlphabetizeDropdown
						// (z.manning 2010-07-26 14:30) - PLID 39848 - Removed SmartStampsLongForm and UseSmartStampsLongform						
						// (z.manning 2011-03-11) - PLID 42778 - Added HasDropdownElements
						//TES 3/17/2011 - PLID 41108 - Added HasGlassesOrderData and GlassesOrderLens
						//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
						// (z.manning 2011-03-21 11:16) - PLID 30608 - Added autofill type
						// (z.manning 2011-05-26 14:51) - PLID 43865 - Added data flags
						// (z.manning 2011-09-19 17:14) - PLID 41954 - Dropdown separators
						// (z.manning 2011-11-07 10:42) - PLID 46309 - SpawnedItemsSeparator
						// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
						//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
						// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
						prsContent = CreateParamRecordset(pCon,
							"SELECT CASE WHEN EmrInfoT.ID = " + AsString((long)EMR_BUILT_IN_INFO__TEXT_MACRO) + " THEN (SELECT MacroName FROM EmrTemplateDetailsT WHERE ID = {INT}) ELSE EmrInfoT.Name END AS Name, DataSubType, "
							"(CASE WHEN EmrInfoT.ID IN (SELECT SourceID FROM EMRActionsT WHERE Deleted = 0 AND SourceType = 3) THEN 1 ELSE 0 END) AS HasInfoActions "
							"FROM EmrInfoT WHERE ID = {INT};\r\n"
							"\r\n"
							// (z.manning 2011-10-25 11:17) - PLID 39401 - Load stamp exclusions if necessary
							"{SQL}"
							"\r\n"
							"SELECT EmrDataT.ID, EmrDataT.EmrDataGroupID, EmrDataT.Data, EmrDataT.IsLabel, EmrDataT.ListType, EmrDataT.IsGrouped, EmrDataT.Formula, EmrDataT.DecimalPlaces, EmrDataT.ListSubType, EmrInfoT.DataType, EmrInfoT.DataSubType, EmrDataT.AutoAlphabetizeDropDown, EmrDataT.DataFlags, "
							"EmrInfoT.BackgroundImageFilePath, EmrInfoT.BackgroundImageType, EmrInfoT.SliderMin, EmrInfoT.SliderMax, "
							"EmrInfoT.SliderInc,  EmrTemplateDetailsT.SliderValue, EmrInfoT.LongForm, EmrDataT.LongForm AS DataLongForm, "
							"EmrInfoT.DataFormat, EmrInfoT.DataSeparator, EmrInfoT.DataSeparatorFinal, "
							"EmrInfoT.DisableTableBorder, EmrInfoT.TableRowsAsFields, "
							// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
							"EmrDataT.AutoNumberType, EmrDataT.AutoNumberPrefix, "
							// (j.jones 2010-02-11 15:24) - PLID 37318 - load ChildEMRInfoMasterID, SmartStampsEnabled, and ChildEMRTemplateDetailID
							// (z.manning 2011-01-25 15:09) - PLID 42336 - Removed parent detail IDs
							"EmrInfoT.ChildEMRInfoMasterID, EmrInfoT.SmartStampsEnabled, EMRTemplateDetailsT.ChildEMRTemplateDetailID, NULL AS ChildEMRDetailID, "
							"CASE WHEN {INT} = 1 THEN CASE WHEN EmrInfoT.DataType = 2 OR EmrInfoT.DataType = 3 THEN (SELECT Min(CASE WHEN DestType = 3 OR DestType = 9 THEN 1 ELSE 2 END) AS MinActionType FROM EMRActionsT WHERE SourceType = 4 AND SourceID = EmrDataT.ID AND Deleted = 0) ELSE NULL END ELSE NULL END AS ActionsType, "
							// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
							// (j.jones 2011-03-09 09:05) - PLID 42283 - added EMCodeUseTableCategories and Data_EMCodeCategoryID
							// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields, because they are now only calculated in the API, and not in Practice code
							//"EMRInfoMasterT.EMCodeCategoryID, EMRInfoMasterT.EMCodeUseTableCategories, EMRInfoMasterT.UseEMCoding AS Info_UseEMCoding, EMRInfoMasterT.EMCodingType, "
							// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
							"EmrDataT.InputMask, "
							//DRT 1/22/2008 - PLID 28603 - Load the content for hotspots
							//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for hotspots
							// (z.manning 2011-07-25 13:00) - PLID 44649 - Added ImageHotSpotID
							"EMRImageHotSpotsT.ID AS HotSpotID, EMRImageHotSpotsT.Data AS HotSpotData, EMRImageHotSpotsT.AnatomicLocationID AS HotSpotLocationID, LabAnatomyT.Description AS HotSpotLocation, EMRImageHotSpotsT.AnatomicQualifierID AS HotSpotQualifierID, AnatomyQualifiersT.Name AS HotSpotQualifier, EMRImageHotSpotsT.AnatomySide AS HotSpotSide, EMRImageHotSpotsT.EmrSpotGroupID AS HotSpotGroupID, EMRImageHotSpotsT.ImageHotSpotID, "
							// (a.walling 2008-07-01 09:16) - PLID 29271 - Preview Flags
							// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floating
							"EmrTemplateDetailsT.PreviewFlags, Convert(bit, CASE WHEN FloatedItemsQ.Count Is Not Null THEN 1 ELSE 0 END) AS IsFloated, "
							// (a.walling 2013-02-28 17:13) - PLID 55390 - Avoid massive index scans and spooling and hash merges by limiting to individual seeks
							// based on the ListType of the Data when trying to determine whether dropdowns exist for this. Previously joining on the ad-hoc 
							// select dumps (potentially millions) of rows to a temp worktable and hashmatches. This replaces that with multiple seeks, unfortunately 2 per text/dropdown
							// item, but still better than before.
							// (a.walling 2013-07-02 14:57) - PLID 57401 - Indexed view for EMRTableDropdownInfoT's total/inactive counts
							"CONVERT(bit, "
								"CASE WHEN EmrDataT.ListType = 3 THEN "
									"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID AND TotalCount > InactiveCount) THEN 1 ELSE 0 END "
								"WHEN EmrDataT.ListType = 4 THEN "
									"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
								"ELSE 0 "
								"END "
							") AS HasActiveDropdownElements, "
							"CONVERT(bit, "
								"CASE WHEN EmrDataT.ListType IN (3,4) THEN "
									"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
								"ELSE 0 "
								"END "
							") AS HasDropdownElements, "
							"EmrInfoT.HasGlassesOrderData, EmrInfoT.GlassesOrderLens, EmrInfoT.InfoFlags, "
							"EmrDataT.GlassesOrderDataType, EmrDataT.GlassesOrderDataID, EmrDataT.AutofillType, DropdownSeparator, DropdownSeparatorFinal, SpawnedItemsSeparator, ParentLabelID, \r\n"
							"EmrInfoT.HasContactLensData, \r\n"
							// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType and UseWithWoundCareCoding
							"EmrDataT.WoundCareDataType, EmrInfoT.UseWithWoundCareCoding \r\n"
							"FROM EmrTemplateDetailsT "
							"INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID "
							"LEFT JOIN EMRDataT ON EmrInfoMasterT.ActiveEmrInfoID = EmrDataT.EmrInfoID "
							"LEFT JOIN EmrInfoT ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID "
							// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
							//DRT 1/22/2008 - PLID 28603 - Load the content for hotspots
							//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for hotspots
							"LEFT JOIN EMRImageHotSpotsT ON EMRInfoT.ID = EMRImageHotSpotsT.EMRInfoID "
							"LEFT JOIN LabAnatomyT ON EMRImageHotSpotsT.AnatomicLocationID = LabAnatomyT.ID "
							"LEFT JOIN AnatomyQualifiersT ON EMRImageHotSpotsT.AnatomicQualifierID = AnatomyQualifiersT.ID "
							// (j.jones 2011-04-28 14:39) - PLID 43122 - load the floating data info
							// (a.walling 2013-03-21 11:25) - PLID 55810 - The EmrProviderFloatDataT join can be skipped entirely if the providerid being scanned for is set to -1
							"LEFT JOIN (SELECT EMRDataGroupID, Count "
							"	FROM EmrProviderFloatDataT "
							"	WHERE ProviderID = {INT} AND {INT} <> -1) AS FloatedItemsQ ON EMRDataT.EmrDataGroupID = FloatedItemsQ.EMRDataGroupID "
							"WHERE EmrTemplateDetailsT.ID = {INT} AND (EMRDataT.Inactive Is Null OR EMRDataT.Inactive = 0) "
							"ORDER BY (CASE WHEN FloatedItemsQ.Count Is Not Null AND EMRInfoT.DataType IN (2,3) "
							"	THEN (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 0 THEN FloatedItemsQ.Count ELSE 1 END) "
							"	ELSE 0 END) DESC, "
							"	(CASE WHEN FloatedItemsQ.Count Is Not Null AND EMRInfoT.DataType IN (2,3) THEN -1 ELSE (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN EmrDataT.SortOrder ELSE -1 END ELSE EmrDataT.SortOrder END) END), "
							"	(CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN '' ELSE EmrDataT.Data END ELSE '' END) ",
							m_nEMRTemplateDetailID, m_nEMRInfoID, sqlStampExclusions, m_bIsTemplateDetail ? 1 : 0, nProviderIDForFloatingData, nProviderIDForFloatingData, m_nEMRTemplateDetailID);
					}
				}
				else {
					//Loading existing detail. (will still load inactive items)
					// (c.haag 2007-04-27 10:10) - PLID 25790 - Don't query data if we have access to a CEMNLoader object
					// (c.haag 2007-08-06 14:38) - PLID 26977 - Don't query data if we have an EMN spanwer object
					if (!bUseEMNLoader && !bUseEMNSpawner) {

						// (j.jones 2011-04-28 14:39) - PLID 43122 - If the currently selected EMN provider has ProvidersT.FloatEMRData = 1,
						// load the data from EmrProviderFloatDataT. If we are sorting alphabetically, sort the "floated" items first, 
						// then the regular items. If not sorting alphabetically, sort the "floated" items in Count order DESC, and then
						// the regular items in their normal sort order.
						
						if(pEMNLoader != NULL && nProviderIDForFloatingData == -1) {
							nProviderIDForFloatingData = pEMNLoader->m_nProviderIDForFloatingData;
						}
						if(nProviderIDForFloatingData == -1) {
							nProviderIDForFloatingData = GetProviderIDForFloatingData();
						}

						CSqlFragment sqlOrderBy, sqlJoinForOrderBy, sqlIsFloated;
						if(bIsForcedReload) {
							// (z.manning 2011-06-13 09:47) - PLID 43140 - We only want to load by the detail order if this
							// is the initial load of the detail. If they changed the info item at all then let's go ahead and
							// reaload the order from admin.
							sqlOrderBy = CSqlFragment(
								"ORDER BY (CASE WHEN FloatedItemsQ.Count Is Not Null AND EMRInfoT.DataType IN (2,3) \r\n"
								"		THEN (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 0 THEN FloatedItemsQ.Count ELSE 1 END) \r\n"
								"		ELSE 0 END) DESC, \r\n"
								"	(CASE WHEN FloatedItemsQ.Count Is Not Null AND EMRInfoT.DataType IN (2,3) THEN -1 ELSE (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN EmrDataT.SortOrder ELSE -1 END ELSE EmrDataT.SortOrder END) END), \r\n"
								"	(CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN '' ELSE EmrDataT.Data END ELSE '' END) \r\n"
								);

							sqlIsFloated = "Convert(bit, CASE WHEN FloatedItemsQ.Count Is Not Null THEN 1 ELSE 0 END)";
							// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
							// (a.walling 2013-03-21 11:24) - PLID 55810 - The EmrProviderFloatDataT join can be skipped entirely if the providerid being scanned for is set to -1
							sqlJoinForOrderBy = CSqlFragment("LEFT JOIN (SELECT EMRDataGroupID, Count FROM EmrProviderFloatDataT WHERE ProviderID = {INT} AND {INT} <> -1) AS FloatedItemsQ ON EMRDataT.EmrDataGroupID = FloatedItemsQ.EMRDataGroupID \r\n", nProviderIDForFloatingData, nProviderIDForFloatingData);
						}
						else {
							// (z.manning 2011-04-05 17:49) - PLID 43140 - When loading existing patient EMN details, the #1
							// sort priority is now the per-detail sorting that we save.
							sqlOrderBy = CSqlFragment(
								"ORDER BY EmrDetailListOrderT.OrderIndex, \r\n"
								"	(CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN EmrDataT.SortOrder ELSE -1 END ELSE EmrDataT.SortOrder END), \r\n"
								"	(CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN '' ELSE EmrDataT.Data END ELSE '' END) \r\n"
							);

							sqlIsFloated = "Convert(bit, Coalesce(EmrDetailListOrderT.IsFloated, 0))";
						}

						//TES 8/9/2006 - In the new element-less system, this filters out inactive items, unless they are selected.
						// (c.haag 2007-02-05 13:15) - PLID 24423 - Cleaned up the sorting and did special handling for Current Medications
						//TES 2/9/2007 - PLID 24671 - Added EmrDataGroupID
						// (c.haag 2007-04-03 08:59) - PLID 25468 - Expanded the special sorting described above to special Allergy items
						// (j.jones 2007-07-18 13:26) - PLID 26730 - load whether or not the info item has Info actions,
						// which it usually does not, such that we don't have to search for them later
						//DRT 7/26/2007 - PLID 26830 - Parameterized to speed execution.
						// (j.jones 2007-10-02 17:19) - PLID 26810 - added EmrInfoT.Name
						// (z.manning, 05/23/2008) - PLID 30155 - Added EmrDataT.Formula and DecimalPlaces
						// (c.haag 2008-06-16 11:58) - PLID 30319 - Changed name calculation for text macros
						// (c.haag 2008-10-16 12:43) - PLID 31709 - Added TableRowsAsFields
						// (z.manning 2009-01-15 16:06) - PLID 32724 - Added InputMask
						// (z.manning 2010-02-16 15:16) - PLID 37230 - ListSubType
						// (c.haag 2010-02-24 16:10) - PLID 21301 - AutoAlphabetizeDropdown
						//TES 2/22/2010 - PLID 37463 - Added SmartStampsLongForm and UseSmartStampsLongForm
						// (j.jones 2010-06-09 15:12) - PLID 37981 - supported generic tables, which use EMRDetailTableOverrideDataT when on EMN details
						// (z.manning 2010-07-26 14:30) - PLID 39848 - Removed SmartStampsLongForm and UseSmartStampsLongForm
						// (z.manning 2011-03-11) - PLID 42778 - Added HasDropdownElements
						//TES 3/17/2011 - PLID 41108 - Added HasGlassesOrderData and GlassesOrderLens
						//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
						// (z.manning 2011-03-21 11:16) - PLID 30608 - Added autofill type
						// (z.manning 2011-05-26 14:51) - PLID 43865 - Added DataFlags
						// (z.manning 2011-09-19 17:15) - PLID 41954 - Dropdown separators
						// (z.manning 2011-11-07 10:43) - PLID 46309 - Added SpawnedItemsSeparator
						// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
						//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
						// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
						prsContent = CreateParamRecordset(pCon, 
							"SET NOCOUNT ON \r\n"
							"DECLARE @nEmrDetailID int \r\n"
							"DECLARE @nEmrInfoID int \r\n"
							"DECLARE @nGenericTableDataSubType int \r\n"
							"SET @nEmrDetailID = {INT} \r\n"
							"SET @nEmrInfoID = {INT} \r\n"
							"SET @nGenericTableDataSubType = {CONST} \r\n"
							"SELECT CASE WHEN EmrInfoT.ID = " + AsString((long)EMR_BUILT_IN_INFO__TEXT_MACRO) + " THEN EmrDetailsT.MacroName ELSE EmrInfoT.Name END AS Name, DataSubType, "
							"(CASE WHEN EmrInfoT.ID IN (SELECT SourceID FROM EMRActionsT WHERE Deleted = 0 AND SourceType = 3) THEN 1 ELSE 0 END) AS HasInfoActions "
							"FROM EmrInfoT "
							"INNER JOIN EmrDetailsT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID "
							"WHERE EmrInfoT.ID = @nEmrInfoID AND EmrDetailsT.ID = @nEmrDetailID;\r\n"
							"\r\n"
							// (z.manning 2011-10-25 11:17) - PLID 39401 - Load stamp exclusions if necessary
							"{SQL}"
							"\r\n"
							"SELECT EmrDataT.ID, EmrDataT.EmrDataGroupID, "
							"CASE WHEN EMRInfoT.DataSubType = @nGenericTableDataSubType AND EMRDetailTableOverrideDataT.Name Is Not Null THEN EMRDetailTableOverrideDataT.Name ELSE EmrDataT.Data END AS Data, "
							"EmrDataT.IsLabel, EmrDataT.ListType, EmrDataT.IsGrouped, EmrDataT.Formula, EmrDataT.DecimalPlaces, EmrInfoT.DataType, EmrInfoT.DataSubType, EmrDataT.ListSubType, EmrDataT.AutoAlphabetizeDropDown, EmrDataT.DataFlags, "
							// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
							"EmrDataT.AutoNumberType, EmrDataT.AutoNumberPrefix, "
							"EmrInfoT.BackgroundImageFilePath, EmrInfoT.BackgroundImageType, EmrInfoT.SliderMin, EmrInfoT.SliderMax, "
							"EmrInfoT.SliderInc, EmrDetailsT.SliderValue, EmrInfoT.LongForm, EmrDataT.LongForm AS DataLongForm, "
							"EmrInfoT.DataFormat, EmrInfoT.DataSeparator, EmrInfoT.DataSeparatorFinal, "
							"EmrInfoT.DisableTableBorder, EmrInfoT.TableRowsAsFields, "
							// (j.jones 2010-02-11 15:24) - PLID 37318 - load ChildEMRInfoMasterID, SmartStampsEnabled, and ChildEMRDetailID
							// (z.manning 2011-01-25 15:09) - PLID 42336 - Removed parent detail IDs
							"EmrInfoT.ChildEMRInfoMasterID, EmrInfoT.SmartStampsEnabled, EMRDetailsT.ChildEMRDetailID, NULL AS ChildEMRTemplateDetailID, "
							"NULL AS ActionsType, " //not used in patient EMNs
							// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
							// (j.jones 2011-03-09 09:05) - PLID 42283 - added EMCodeUseTableCategories and Data_EMCodeCategoryID
							// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields, because they are now only calculated in the API, and not in Practice code
							//"EMRInfoMasterT.EMCodeCategoryID, EMRInfoMasterT.EMCodeUseTableCategories, EMRInfoMasterT.UseEMCoding AS Info_UseEMCoding, EMRInfoMasterT.EMCodingType, "
							// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
							"EmrDataT.InputMask, "
							//DRT 1/22/2008 - PLID 28603 - Load the content for hotspots
							//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for HotSpots
							// (z.manning 2011-07-25 13:00) - PLID 44649 - Added ImageHotSpotID
							"EMRImageHotSpotsT.ID AS HotSpotID, EMRImageHotSpotsT.Data AS HotSpotData, EMRImageHotSpotsT.AnatomicLocationID AS HotSpotLocationID, LabAnatomyT.Description AS HotSpotLocation, EMRImageHotSpotsT.AnatomicQualifierID AS HotSpotQualifierID, AnatomyQualifiersT.Name AS HotSpotQualifier, EMRImageHotSpotsT.AnatomySide AS HotSpotSide, EMRImageHotSpotsT.EmrSpotGroupID AS HotSpotGroupID, EMRImageHotSpotsT.ImageHotSpotID, "
							// (a.walling 2008-07-01 09:16) - PLID 292971 - Preview Flags							
							"EmrDetailsT.PreviewFlags, "
							// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floating
							"{SQL} AS IsFloated, "
							// (a.walling 2013-02-28 17:13) - PLID 55390 - Avoid massive index scans and spooling and hash merges by limiting to individual seeks
							// based on the ListType of the Data when trying to determine whether dropdowns exist for this. Previously joining on the ad-hoc 
							// select dumps (potentially millions) of rows to a temp worktable and hashmatches. This replaces that with multiple seeks, unfortunately 2 per text/dropdown
							// item, but still better than before.
							// (a.walling 2013-07-02 14:57) - PLID 57401 - Indexed view for EMRTableDropdownInfoT's total/inactive counts
							"CONVERT(bit, "
								"CASE WHEN EmrDataT.ListType = 3 THEN "
									"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID AND TotalCount > InactiveCount) THEN 1 ELSE 0 END "
								"WHEN EmrDataT.ListType = 4 THEN "
									"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
								"ELSE 0 "
								"END "
							") AS HasActiveDropdownElements, "
							"CONVERT(bit, "
								"CASE WHEN EmrDataT.ListType IN (3,4) THEN "
									"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
								"ELSE 0 "
								"END "
							") AS HasDropdownElements, "
							"EmrInfoT.HasGlassesOrderData, EmrInfoT.GlassesOrderLens, EmrInfoT.InfoFlags, \r\n"
							"EmrDataT.GlassesOrderDataType, EmrDataT.GlassesOrderDataID, EmrDataT.AutofillType, DropdownSeparator, DropdownSeparatorFinal, SpawnedItemsSeparator, ParentLabelID, \r\n"
							"EmrInfoT.HasContactLensData, \r\n"
							// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType and UseWithWoundCareCoding
							"EmrDataT.WoundCareDataType, EmrInfoT.UseWithWoundCareCoding \r\n"
							"FROM EmrDetailsT "
							"LEFT JOIN EMRDataT ON EmrDetailsT.EmrInfoID = EmrDataT.EmrInfoID "
							"LEFT JOIN EmrInfoT ON EmrDetailsT.EmrInfoID = EmrInfoT.ID "
							"LEFT JOIN EMRDetailTableOverrideDataT ON EMRDataT.ID = EMRDetailTableOverrideDataT.EMRDataID AND EMRDetailsT.ID = EMRDetailTableOverrideDataT.EMRDetailID "
							"LEFT JOIN EMRInfoMasterT ON EMRInfoT.EMRInfoMasterID = EMRInfoMasterT.ID "
							// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
							//DRT 1/22/2008 - PLID 28603 - Load the content for hotspots
							//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for hotspots
							"LEFT JOIN EMRImageHotSpotsT ON EMRInfoT.ID = EMRImageHotSpotsT.EMRInfoID "
							"LEFT JOIN LabAnatomyT ON EMRImageHotSpotsT.AnatomicLocationID = LabAnatomyT.ID "
							"LEFT JOIN AnatomyQualifiersT ON EMRImageHotSpotsT.AnatomicQualifierID = AnatomyQualifiersT.ID "
							"LEFT JOIN EmrSelectT ON EmrDetailsT.ID = EmrSelectT.EmrDetailID AND EmrDataT.ID = EmrSelectT.EmrDataID "
							"LEFT JOIN EmrDetailListOrderT ON EmrDetailsT.ID = EmrDetailListOrderT.EmrDetailID AND EmrDataT.EmrDataGroupID = EmrDetailListOrderT.EmrDataGroupID \r\n"
							"{SQL}"
							"WHERE EmrDetailsT.ID = @nEmrDetailID AND (EmrDataT.Inactive Is Null OR EmrDataT.Inactive = 0 OR EmrSelectT.ID Is Not Null) "
							"{SQL}"
							, m_nEMRDetailID, m_nEMRInfoID, eistGenericTable, sqlStampExclusions, sqlIsFloated, sqlJoinForOrderBy, sqlOrderBy);
					}
				}
			}
			BeginAddListElements();

			// (c.haag 2007-04-26 11:03) - PLID 25790 - Use the CEMNLoader object if available. Unless otherwise commented,
			// all code below is attributed to 25790.
			if (bUseEMNLoader) {
				CEMNLoader::CEmrDataItemArray* paDataItems = pLoaderInfoItem->m_paDataItems;
				m_strLongForm = pLoaderInfoItem->m_strLongForm;
				m_nDataFormat = pLoaderInfoItem->m_nDataFormat;
				m_strDataSeparator = pLoaderInfoItem->m_strDataSeparator;
				m_strDataSeparatorFinal = pLoaderInfoItem->m_strDataSeparatorFinal;
				m_bTableRowsAsFields = pLoaderInfoItem->m_bTableRowsAsFields; // (c.haag 2008-10-16 11:33) - PLID 31709

				// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
				m_nChildEMRInfoMasterID = pLoaderInfoItem->m_nChildEMRInfoMasterID;
				m_bSmartStampsEnabled = pLoaderInfoItem->m_bSmartStampsEnabled;

				// (z.manning 2011-01-25 15:24) - PLID 42336 - These are both deprecated as we now support multiple images
				// for one smart stamp table.
				//m_nParentEMRDetailID = pLoaderInfoItem->m_nParentEMRDetailID;
				//m_nParentEMRTemplateDetailID = pLoaderInfoItem->m_nParentEMRTemplateDetailID;

				//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
				m_bHasGlassesOrderData = pLoaderInfoItem->m_bHasGlassesOrderData;
				m_golLens = pLoaderInfoItem->m_golLens;
				//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
				m_bHasContactLensData = pLoaderInfoItem->m_bHasContactLensData;
				// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
				m_bUseWithWoundCareCoding = pLoaderInfoItem->m_bUseWithWoundCareCoding;

				// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
				m_nInfoFlags = pLoaderInfoItem->m_nInfoFlags;
				
				// (a.walling 2009-01-13 13:54) - PLID 32107 - Load the info item's background info
				m_varInfoBackgroundImageFilePath = _variant_t(pLoaderInfoItem->m_strBackgroundImageFilePath);
				m_varInfoBackgroundImageType = _variant_t((long)pLoaderInfoItem->m_BackgroundImageType, VT_I4);

				// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
				// because they are now only calculated in the API, and not in Practice code
				/*
				// (j.jones 2007-08-27 10:35) - PLID 27056 - load the E/M coding data
				m_nEMCodeCategoryID = pLoaderInfoItem->m_nEMCodeCategoryID;
				// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_eEMCodeUseTableCategories
				m_eEMCodeUseTableCategories = pLoaderInfoItem->m_eEMCodeUseTableCategories;
				m_bUseEMCoding = pLoaderInfoItem->m_bUseEMCoding;
				m_emctEMCodingType = pLoaderInfoItem->m_emctEMCodingType;
				*/

				// (j.jones 2007-07-18 13:25) - PLID 26730 - load whether or not the info item has Info actions,
				// which it usually does not, such that we don't have to search for them later
				m_eHasInfoActions = pLoaderInfoItem->m_eHasInfoActions;

				// (a.walling 2008-07-01 09:17) - PLID 29271 - Preview Flags
				// (a.walling 2008-07-08 12:37) - PLID 29271 - Don't overwrite the preview flags from the info item
				//m_nPreviewFlags = pLoaderInfoItem->m_nPreviewFlags;

				// (j.jones 2007-10-02 17:21) - PLID 26810 - load the EmrInfoT.Name
				// (j.jones 2007-10-22 09:43) - PLID 27830 - don't assign directly to m_strLabelText, as SetLabelText properly handles narratives
				// (c.haag 2008-06-17 18:38) - PLID 30319 - Don't do this for built-in macro details 
				if (EMR_BUILT_IN_INFO__TEXT_MACRO != m_nEMRInfoID) {
					SetLabelText(pLoaderInfoItem->m_strLabelText);
				}

				// (j.jones 2010-02-17 15:32) - PLID 37318 - If we reloaded a SmartStamp Image that is linked to a table,
				// that link may now be invalid. If so, this function will fix it.
				// (j.jones 2010-03-23 12:16) - PLID 37318 - added override for whether to call this or not
				if(bEnsureSmartStampTableLinked) {
					EnsureLinkedSmartStampTableValid();
				}

				// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

				// (c.haag 2006-06-30 08:31) - PLID 19977 - Load problem-related values
				// (c.haag 2006-08-08 09:24) - PLID 21835 - A detail can have only one problem, so we need to flag if it has been loaded already
				// (c.haag 2006-10-19 11:46) - PLID 21454 - Do not load a deleted problem
				if((!((m_nEMRDetailID == -1 && m_nEMRTemplateDetailID == -1) || bForceLoadFromAdmin) &&
					!(m_nEMRDetailID == -1 && m_nEMRTemplateDetailID != -1))) {

					// (c.haag 2007-04-27 10:40) - PLID 25790 - Get the problem values from the preloaded detail object
					CEMNLoader::CPreloadedDetail* pPreloadedDetail = pEMNLoader->GetPreloadedDetailByID(m_nEMRDetailID);
					if(pPreloadedDetail) {

						// (j.jones 2008-07-21 11:40) - PLID 30779 - need to load problems from the loader
						// (c.haag 2009-05-16 12:49) - PLID 34311 - Perform the load using the new EMR problem
						// linking structure

						CEMNLoaderMutex* pmtxProblems = pEMNLoader->GetEMRProblemsMutex();
						CHoldEMNLoaderMutex mhProblems(pmtxProblems);

						CEmrProblemLinkAry *pAry = pEMNLoader->GetEmrProblemLinks(eprtEmrItem, m_nEMRDetailID);
						if(pAry) {
							// (j.jones 2008-07-18 12:05) - PLID 30779 - problems are now stored in an array of objects
							for(int i = 0; i < pAry->GetSize(); i++) {
								CEmrProblemLink& pLoaderLink = pAry->GetAt(i);
								CEmrProblem* pLoaderProblem = pLoaderLink.GetProblem();

								//see if it is in our list
								BOOL bFound = FALSE;
								for(int i=0; i<m_apEmrProblemLinks.GetSize() && !bFound; i++) {
									CEmrProblemLink *pLink = m_apEmrProblemLinks.GetAt(i);
									if(pLink->GetID() == pAry->GetAt(i).GetID()) {
										//don't update if we've modified it	
										CEmrProblem* pProblem = pLink->GetProblem();
										// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
										// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
										if(!pProblem->m_bIsModified) {
											pProblem->m_strDescription = pLoaderProblem->m_strDescription;
											pProblem->m_dtEnteredDate = pLoaderProblem->m_dtEnteredDate;
											pProblem->m_dtModifiedDate = pLoaderProblem->m_dtModifiedDate;
											pProblem->m_dtOnsetDate = pLoaderProblem->m_dtOnsetDate;
											pProblem->m_nStatusID = pLoaderProblem->m_nStatusID;
											// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
											pProblem->m_nDiagICD9CodeID = pLoaderProblem->m_nDiagICD9CodeID;
											pProblem->m_nDiagICD10CodeID = pLoaderProblem->m_nDiagICD10CodeID;
											pProblem->m_nChronicityID = pLoaderProblem->m_nChronicityID;
											// (c.haag 2009-05-28 09:38) - PLID 34311 - These members now exist at the link level
											//pProblem->m_eprtTypeID = pLoaderProblem->m_eprtTypeID;
											//pProblem->m_nEMRRegardingID = pLoaderProblem->m_nEMRRegardingID;
											// (b.spivey, October 22, 2013) - PLID 58677 - added codeID
											pProblem->m_nCodeID = pLoaderProblem->m_nCodeID; 
											// (s.tullis 2015-02-23 15:44) - PLID 64723 
											pProblem->m_bDoNotShowOnCCDA = pLoaderProblem->m_bDoNotShowOnCCDA;
											// (r.gonet 2015-03-09 18:21) - PLID 65008 - Assign DoNotShowOnProblemPrompt.
											pProblem->m_bDoNotShowOnProblemPrompt = pLoaderProblem->m_bDoNotShowOnProblemPrompt;
										}

										bFound = TRUE;										
									}
								}

								if(bFound) {
									continue;
								}

								//if we're still here, it wasn't in our list, so add it

								// (c.haag 2009-05-19 17:28) - PLID 34311 - Create the problem link. We're safe to 
								// pass in the problem which belongs to the EMR loader because the EMR object in 
								// memory permanently owns that problem.
								CEmrProblemLink* pNewLink = new CEmrProblemLink(pLoaderLink);
								pNewLink->UpdatePointersWithDetail(this);
								m_apEmrProblemLinks.Add(pNewLink);

								// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
								if (NULL != m_pParentTopic && NULL != m_pParentTopic->GetParentEMN()) {
									CWnd *pWnd = m_pParentTopic->GetParentEMN()->GetInterface();
									if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
										pWnd->PostMessage(NXM_EMR_PROBLEM_CHANGED);
									}
								}
							}
						}
					}
					else {

						// (c.haag 2007-08-17 13:16) - PLID 25790 - If we get here, it either means the detail was added after
						// the initial load, or that it was simply not available in the EMN loader. We will need to get the
						// value from data.
						// (j.jones 2008-07-15 17:21) - PLID 30739 - supported EMRRegardingType and EMRRegardingID
						// (j.jones 2008-07-18 11:06) - PLID 30779 - for the new problem tracking,
						// load up all the problems on this detail in a recordset, but do NOT
						// reload a problem that has been modified	
						// (c.haag 2009-05-19 10:21) - PLID 34311 - We now pull EMR problem links
						if((!((m_nEMRDetailID == -1 && m_nEMRTemplateDetailID == -1) || bForceLoadFromAdmin) &&
							!(m_nEMRDetailID == -1 && m_nEMRTemplateDetailID != -1))) {

							// (z.manning 2009-01-16 10:16) - PLID 32757 - Fixed a syntax error in this query
							// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
							// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
							// (c.haag 2009-05-12 12:19) - PLID 34234 - Problem link table
							// (z.manning 2009-05-27 10:05) - PLID 34297 - Added patient ID
							// (b.spivey, October 22, 2013) - PLID 58677 - added codeID
							// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
							// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
							// (a.walling 2014-07-23 09:09) - PLID 63003 - Filter on EMRProblemsT.PatientID when possible
							// (s.tullis 2015-02-23 15:44) - PLID 64723 
							// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
							_RecordsetPtr prs = CreateParamRecordset(pCon, "SELECT EMRProblemsT.ID, Description, StatusID, "
								"EnteredDate, ModifiedDate, OnsetDate, EMRRegardingType, EMRRegardingID, EMRDataID, "
								"DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemActionID, EMRProblemLinkT.ID AS EMRProblemLinkID, "
								"EmrProblemsT.PatientID, EMRProblemsT.CodeID, EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
								"FROM EMRProblemsT "
								"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
								"WHERE PatientID = {INT} AND ((EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}) "
								"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT})) " 
								"AND Deleted = 0", GetPatientID(), eprtEmrItem, m_nEMRDetailID, eprtEmrDataItem, m_nEMRDetailID);

							while(!prs->eof) {
								long nEMRProblemLinkID = AdoFldLong(prs, "EMRProblemLinkID");
								EMRProblemRegardingTypes eprtTypeID = (EMRProblemRegardingTypes)AdoFldLong(prs, "EMRRegardingType");
								long nEMRRegardingID = AdoFldLong(prs, "EMRRegardingID", -1);
								long nEMRDataID = AdoFldLong(prs, "EMRDataID", -1);

								//see if it is in our list
								BOOL bFound = FALSE;
								for(int i=0; i<m_apEmrProblemLinks.GetSize() && !bFound; i++) {
									CEmrProblemLink *pLink = m_apEmrProblemLinks.GetAt(i);
									if(pLink->GetID() == nEMRProblemLinkID) {
										// if the problem has been modified at all, do not reload from data
										CEmrProblem* pProblem = pLink->GetProblem();
										if (!pProblem->m_bIsModified) {
											pProblem->ReloadFromData(prs->Fields);
										}
										bFound = TRUE;										
									}
								}

								if(bFound) {
									prs->MoveNext();
									continue;
								}

								//if we're still here, it wasn't in our list, so add it
								// (c.haag 2009-05-19 17:22) - PLID 34311 - Allocate a problem from the parent EMR. If there
								// is no parent EMR, that means we're loading for some purpose outside the PIC; like getting
								// a list of problems for a patient warning, for example. In those cases, the problems will be
								// self managed through reference counting, and not have any one owner. This is OK because 
								// in those cases, no saving ever takes place; so there doesn't need to be a central owner for
								// coordinating problems.

								// Get the problem for the problem link
								CEmrProblem* pProblem = NULL;
								if (NULL != m_pParentTopic && NULL != m_pParentTopic->GetParentEMN()) {
									CEMR* pEMR = m_pParentTopic->GetParentEMN()->GetParentEMR();
									if (NULL != pEMR) {
										CEmrProblem* pProblem = pEMR->AllocateEmrProblem(prs->Fields);
									}
								}
								if (NULL == pProblem) {
									pProblem = new CEmrProblem(prs->Fields);
								}
								// Create the problem link
								CEmrProblemLink* pNewLink = new CEmrProblemLink(pProblem, nEMRProblemLinkID, eprtTypeID, nEMRRegardingID, nEMRDataID);
								pNewLink->UpdatePointersWithDetail(this);
								m_apEmrProblemLinks.Add(pNewLink);
								// Release our local reference to the problem
								pProblem->Release();

								// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
								if (NULL != m_pParentTopic && NULL != m_pParentTopic->GetParentEMN()) {
									CWnd *pWnd = m_pParentTopic->GetParentEMN()->GetInterface();
									if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
										pWnd->PostMessage(NXM_EMR_PROBLEM_CHANGED);
									}
								}

								prs->MoveNext();
							}
							prs->Close();
						}
					}
				}

				m_bDisableTableBorder = (pLoaderInfoItem->m_bDisableTableBorder) ? true : false;

				EmrInfoType eitDatabase = pLoaderInfoItem->m_DataType;
				EmrInfoSubType eistDatabase = pLoaderInfoItem->m_DataSubType;

				// (c.haag 2007-07-09 16:27) - PLID 26523 - Load in the rest of the EMR info data
				double dSliderMin = pLoaderInfoItem->m_dSliderMin;
				double dSliderMax = pLoaderInfoItem->m_dSliderMax;
				double dSliderInc = pLoaderInfoItem->m_dSliderInc;
				eImageType BackgroundImageType = pLoaderInfoItem->m_BackgroundImageType;
				CString strBackgroundImageFilePath = pLoaderInfoItem->m_strBackgroundImageFilePath;

				//DRT 1/18/2008 - PLID 28603 - Pull the image hotspots out of the loader
				CArray<CEMNLoader::EmrImageHotSpotItem*, CEMNLoader::EmrImageHotSpotItem*> *pHotSpotItems = pLoaderInfoItem->m_paryHotSpotItems;
				if(pHotSpotItems != NULL) {
					for(int i = 0; i < pHotSpotItems->GetSize(); i++) {
						CEMNLoader::EmrImageHotSpotItem *item = pHotSpotItems->GetAt(i);

						//Create a new hotspot
						CEMRHotSpot *pSpot = new CEMRHotSpot;
						pSpot->SetID(item->m_nID);
						pSpot->SetGroupID(item->m_nEMRHotSpotGroupID);
						pSpot->LoadData(item->m_strData);
						//TES 2/11/2010 - PLID 37298 - Added Anatomic Location information to HotSpots
						pSpot->SetOriginalAnatomicLocation(item->m_nAnatomicLocationID, item->m_strAnatomicLocation);
						pSpot->SetOriginalAnatomicQualifier(item->m_nAnatomicQualifierID, item->m_strAnatomicQualifier);
						pSpot->SetOriginalSide(item->m_asSide);
						pSpot->Set3DHotSpotID(item->m_n3DHotSpotID);

						m_aryImageHotSpots.Add(pSpot);
					}
				}

				// (z.manning 2011-10-25 10:16) - PLID 39401 - Pull the stamp exclusion data
				if(pLoaderInfoItem->m_pStampExclusions != NULL) {
					m_StampExclusions = *pLoaderInfoItem->m_pStampExclusions;
					// (a.wilson 2013-03-26 11:00) - PLID 55826 - set loaded flag.
					m_bStampExclusionsLoaded = true;
				}

				pLoaderInfoItem = NULL;

				// (c.haag 2007-07-09 15:57) - PLID 26523 - We're all done with the EMR info and detail data
				if (pEMNLoaderMutex) {
					pEMNLoaderMutex->Unlock();
					// (c.haag 2007-07-10 09:00) - PLID 26523 - Now make it so if the silent exception is tripped,
					// it won't try to unlock it a second time
					pEMNLoaderMutex = NULL;
				}

				// (a.walling 2008-02-08 11:11) - PLID 28857 - We can still return a data set if the info type change did not
				// require a state reset; for example, going from a single to multi, or a multi to single with one selection.
				// (z.manning 2011-10-21 16:19) - PLID 44649 - Compare on sub type here too.
				if(!IsSameEmrInfoType(m_EMRInfoType, m_EMRInfoSubType, eitDatabase, eistDatabase))
				{
					// (j.jones 2007-07-25 17:54) - PLID 26810 - set m_bIsForcedReload = TRUE,
					// to indicate that an external process is forcing a content reload
					// (z.manning, 03/31/2008) - PLID 29474 - We used to only set the forced reload flag if
					// the type change required a state reset, but we need to do it for ALL type changes
					// otherwise other parts of the program (e.g. the part that reloads the AdvDlg) will
					// not know about the type change.
					m_bIsForcedReload = TRUE;

					if(InfoTypeChangeRequiresDetailStateReset(m_EMRInfoType, m_EMRInfoSubType, eitDatabase, eistDatabase, this)) {
						//TES 6/22/2006 - This happens sometimes during the process of re-creating a detail whose type has been changed;
						//we can't load the content until our member variable is in sync.
						// (j.jones 2011-07-22 17:26) - PLID 43504 - pass in our connection
						SetNeedContentReload(pCon);
						return;
					}
				}

				switch (eitDatabase) {
				case eitTable: // table
					if (NULL == paDataItems || paDataItems->GetSize() == 0) {
						//no rows, which means the item type had been changed and
						//we have no elements at all, so we must re-load from Admin

						// (c.haag 2007-02-05 13:36) - PLID 24423 - Cleaned up the sorting and did special handling for Current Medications
						// (c.haag 2007-04-03 08:59) - PLID 25468 - Expanded the special sorting described above to special Allergy items
						//DRT 7/26/2007 - PLID 26830 - Parameterized to speed execution.
						// (z.manning, 05/23/2008) - PLID 30155 - Added EmrDataT.Formula and DecimalPlaces
						// (z.manning 2009-01-15 15:23) - PLID 32724 - Added EmrDataT.InputMask
						// (z.manning 2010-02-16 14:33) - PLID 37230 - ListSubType
						//TES 2/22/2010 - PLID 37463 - Added SmartStampsLongForm and UseSmartStampsLongForm
						// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
						// (z.manning 2010-07-26 14:31) - PLID 39848 - Removed SmartStampsLongForm and UseSmartStampsLongForm
						// (z.manning 2011-03-11) - PLID 42778 - Added HasDropdownElements
						//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
						// (z.manning 2011-03-21 11:22) - PLID 30608 - Added autofill type
						// (z.manning 2011-05-26 14:52) - PLID 43865 - Added DataFlags
						// (z.manning 2011-09-19 17:15) - PLID 41954 - Dropdown separator
						// (z.manning 2011-11-07 10:43) - PLID 46309 - Added SpawnedItemsSeparator
						// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
						_CommandPtr pCmd = OpenParamQuery(pCon, 
							"SELECT EmrDataT.ID, EmrDataT.Data, EmrDataT.IsLabel, EmrDataT.ListType, EmrDataT.IsGrouped, "
							"EmrDataT.Formula, EmrDataT.DecimalPlaces, EmrInfoT.DataType, EmrInfoT.DataSubType, EmrDataT.InputMask, EmrDataT.AutoAlphabetizeDropDown, EmrDataT.DataFlags, "
							// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
							"EmrDataT.AutoNumberType, EmrDataT.AutoNumberPrefix, "
							"EmrInfoT.BackgroundImageFilePath, EmrInfoT.BackgroundImageType, EmrInfoT.SliderMin, EmrInfoT.SliderMax, "
							"EmrInfoT.SliderInc, NULL AS SliderValue, EmrInfoT.LongForm, EmrDataT.LongForm AS DataLongForm, "
							"EmrInfoT.DataFormat, EmrInfoT.DataSeparator, EmrInfoT.DataSeparatorFinal, "
							"EmrInfoT.DisableTableBorder, "
							"NULL AS ActionsType, EmrDataT.EmrDataGroupID, EmrDataT.ListSubType, "
							// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
							// (j.jones 2011-03-09 09:05) - PLID 42283 - added EMCodeUseTableCategories and Data_EMCodeCategoryID
							// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields, because they are now only calculated in the API, and not in Practice code
							//"EMRInfoMasterT.EMCodeCategoryID, EMRInfoMasterT.EMCodeUseTableCategories, EMRInfoMasterT.UseEMCoding AS Info_UseEMCoding, EMRInfoMasterT.EMCodingType, "
							// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
							// (a.walling 2013-02-28 17:13) - PLID 55390 - Avoid massive index scans and spooling and hash merges by limiting to individual seeks
							// based on the ListType of the Data when trying to determine whether dropdowns exist for this. Previously joining on the ad-hoc 
							// select dumps (potentially millions) of rows to a temp worktable and hashmatches. This replaces that with multiple seeks, unfortunately 2 per text/dropdown
							// item, but still better than before.
							// (a.walling 2013-07-02 14:57) - PLID 57401 - Indexed view for EMRTableDropdownInfoT's total/inactive counts
							"CONVERT(bit, "
								"CASE WHEN EmrDataT.ListType = 3 THEN "
									"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID AND TotalCount > InactiveCount) THEN 1 ELSE 0 END "
								"WHEN EmrDataT.ListType = 4 THEN "
									"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
								"ELSE 0 "
								"END "
							") AS HasActiveDropdownElements, "
							"CONVERT(bit, "
								"CASE WHEN EmrDataT.ListType IN (3,4) THEN "
									"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
								"ELSE 0 "
								"END "
							") AS HasDropdownElements, "
							"EmrDataT.GlassesOrderDataType, EmrDataT.GlassesOrderDataID, EmrDataT.AutofillType, DropdownSeparator, DropdownSeparatorFinal, SpawnedItemsSeparator, ParentLabelID, "
							// (j.jones 2011-04-28 14:39) - PLID 43122 - IsFloated is not used on tables
							"NULL AS IsFloated, "
							// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
							"EmrDataT.WoundCareDataType "
							"FROM EMRInfoT "
							"LEFT JOIN (SELECT * FROM EMRDataT WHERE Inactive = 0) EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID "
							"LEFT JOIN EMRInfoMasterT ON EMRInfoT.EMRInfoMasterID = EMRInfoMasterT.ID "
							// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
							"WHERE EMRInfoT.ID = ? "
							"ORDER BY (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN EmrDataT.SortOrder ELSE -1 END ELSE EmrDataT.SortOrder END), "
							"	(CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN '' ELSE EmrDataT.Data END ELSE '' END) "
							);
						AddParameterLong(pCmd, "InfoID", m_nEMRInfoID);
						_RecordsetPtr prs = CreateRecordset(pCmd);
						FieldsPtr pflds = prs->GetFields();
						FieldPtr fldID = pflds->GetItem("ID");
						FieldPtr fldData = pflds->GetItem("Data");
						FieldPtr fldLabel = pflds->GetItem("IsLabel");
						//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
						FieldPtr fldGlassesOrderDataType = pflds->GetItem("GlassesOrderDataType");
						FieldPtr fldGlassesOrderDataID = pflds->GetItem("GlassesOrderDataID");
						// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
						FieldPtr fldWoundCareDataType = pflds->GetItem("WoundCareDataType");

						long nGenericRowsNeeded = 0;
						long nGenericColumnsNeeded = 0;

						// (j.jones 2010-06-21 15:54) - PLID 37981 - if we are given default table content,
						// then we need to only load a set number of rows and columns
						if(pDefaultGenericTableContent) {
							nGenericRowsNeeded = pDefaultGenericTableContent->aryRows.GetSize();
							nGenericColumnsNeeded = pDefaultGenericTableContent->aryColumns.GetSize();
						}

						while (!prs->eof) {
							CString strData = AdoFldString(pflds->GetItem("Data"),"");
							long nListType = AdoFldLong(pflds, "ListType", -1);

							// (j.jones 2010-06-21 15:54) - PLID 37981 - if we are given default table content,
							// then we need to only load a set number of rows and columns
							if(pDefaultGenericTableContent) {
								if(nListType == 2 && nGenericRowsNeeded > 0) {
									//override the name
									DevicePluginUtils::RowElement *pRowOverride = pDefaultGenericTableContent->aryRows.GetAt(pDefaultGenericTableContent->aryRows.GetSize() - nGenericRowsNeeded);
									if(pRowOverride) {
										strData = pRowOverride->strName;
									}
									nGenericRowsNeeded--;
								}
								else if(nListType == 3 && nGenericColumnsNeeded > 0) {
									//override the name
									DevicePluginUtils::ColumnElement *pColumnOverride = pDefaultGenericTableContent->aryColumns.GetAt(pDefaultGenericTableContent->aryColumns.GetSize() - nGenericColumnsNeeded);
									if(pColumnOverride) {
										strData = pColumnOverride->strName;
									}
									nGenericColumnsNeeded--;
								}
								else {
									//we do not need to load this record
									prs->MoveNext();
									continue;
								}
							}

							BYTE nListSubType = AdoFldByte(pflds, "ListSubType", lstDefault);							
							BOOL bIsGrouped = AdoFldBool(pflds, "IsGrouped", FALSE);
							// (a.walling 2013-03-21 10:14) - PLID 55804 - No more EM coding for data items necessary
							CString strFormula = AdoFldString(pflds, "Formula", "");
							BYTE nDecimalPlaces = AdoFldByte(pflds, "DecimalPlaces", 2);
							CString strInputMask = AdoFldString(pflds, "InputMask", "");
							CString strDataLongForm = AdoFldString(pflds, "DataLongForm", "");
							long nGroupID = AdoFldLong(pflds, "EmrDataGroupID", -1);
							// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
							BOOL bAutoAlphabetizeDropdown = AdoFldBool(pflds, "AutoAlphabetizeDropDown", FALSE);
							// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
							EEmrTableAutoNumberType etantAutoNumberType = (EEmrTableAutoNumberType)AdoFldByte(pflds, "AutoNumberType", (BYTE)etantPerRow);
							CString strAutoNumberPrefix = AdoFldString(pflds, "AutoNumberPrefix", "");
							BOOL bHasDropdownElements = AdoFldBool(pflds, "HasDropdownElements", TRUE);
							BOOL bHasActiveDropdownElements = AdoFldBool(pflds, "HasActiveDropdownElements", TRUE);
							BYTE nAutofillType = AdoFldByte(pflds, "AutofillType", etatNone);
							long nFlags = AdoFldLong(pflds, "DataFlags", 0);
							CString strDropdownSeparator = AdoFldString(pflds, "DropdownSeparator", ", ");
							CString strDropdownSeparatorFinal = AdoFldString(pflds, "DropdownSeparatorFinal", ", ");
							CString strSpawnedItemsSeparator = AdoFldString(pflds, "SpawnedItemsSeparator", ", ");
							if(nListType == 2) { //Row
								// (z.manning 2010-02-19 15:57) - PLID 37412 - Smart stamp tables load their rows elsewhere
								// (j.jones 2010-02-25 10:22) - PLID 37538 - user-defined rows are allowed
								//TES 3/17/2010 - PLID 37530 - Need to specify the stamp ID and index
								//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
								//(e.lally 2011-12-08) PLID 46471 - Specify if this is for a current medications or allergies detail

								// (a.walling 2011-08-11 15:49) - PLID 44987 - Insert if new, update strName if existing
								TableRowID rowID(AdoFldLong(fldID),-1,NULL,-1,-1);

								std::pair<TableRowMap::iterator, bool> insertionPoint = mapTableRows.insert(TableRowMap::value_type(rowID, NULL));
								if (insertionPoint.second) {
									// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
									insertionPoint.first->second = AddRow(rowID, strData, strFormula, nDecimalPlaces, nGroupID, AdoFldBool(fldLabel), (GlassesOrderDataType)AdoFldLong(fldGlassesOrderDataType,(long)godtInvalid), AdoFldLong(fldGlassesOrderDataID,-1), nFlags, aroSequential, -1, ((IsCurrentMedicationsTable() || IsAllergiesTable()) ? TRUE : FALSE));
								} else {
									insertionPoint.first->second->strName = strData;
								}
							}
							else if(nListType >= 3) { //Column
								//(e.lally 2011-12-08) PLID 46471 - We assume the first column of a current medication or allergy table detail to be the official usage checkbox column
								BOOL bIsCurrentMedOrAllergyUsageCol = (m_arTableColumns.GetSize()==0 && (IsCurrentMedicationsTable() || IsAllergiesTable()));
								if(bIsCurrentMedOrAllergyUsageCol){
									//(e.lally 2011-12-08) PLID 46471 - This had better be true or our assumption about which column to use for allergies and current medications is wrong
									ASSERT(nListType == LIST_TYPE_CHECKBOX);
								}

								//DRT 7/10/2007 - PLID 24105 - No longer requires a width
								// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
								//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
								//(e.lally 2011-12-08) PLID 46471 - Added bIsCurrentMedOrAllergyUsageCol
								// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
								// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
								AddColumn(AdoFldLong(fldID), strData, nListType, bIsGrouped, strFormula, nDecimalPlaces, strInputMask, nListSubType, bAutoAlphabetizeDropdown, AdoFldBool(fldLabel), strDataLongForm, etantAutoNumberType, strAutoNumberPrefix, bHasDropdownElements, bHasActiveDropdownElements, (GlassesOrderDataType)AdoFldLong(fldGlassesOrderDataType,(long)godtInvalid), AdoFldLong(fldGlassesOrderDataID,-1), nAutofillType, nFlags, strDropdownSeparator, strDropdownSeparatorFinal, strSpawnedItemsSeparator, bIsCurrentMedOrAllergyUsageCol, (EWoundCareDataType)AdoFldLong(fldWoundCareDataType, (long)wcdtNone));
							}
							prs->MoveNext();
						}

					} 
					else {

						// (j.jones 2010-06-21 17:31) - PLID 37981 - if a generic table, the EMRInfo content is loaded,
						// but we still need to load specific generic field names
						CMap<int, int, CString, CString> mapDataIDsToName;
						if(IsGenericTable() && !m_bIsTemplateDetail && m_nEMRDetailID != -1) {
							_RecordsetPtr rsOverride = CreateParamRecordset(pCon, "SELECT EMRDataID, Name FROM EMRDetailTableOverrideDataT "
								"WHERE EMRDetailID = {INT}", m_nEMRDetailID);
							while(!rsOverride->eof) {

								long nEMRDataID = AdoFldLong(rsOverride, "EMRDataID");
								CString strDataName = AdoFldString(rsOverride, "Name", "");

								//map it
								mapDataIDsToName.SetAt(nEMRDataID, strDataName);

								rsOverride->MoveNext();
							}
							rsOverride->Close();
						}

						for (int i=0; i < paDataItems->GetSize(); i++) {
							CEMNLoader::EmrDataItem& item = paDataItems->GetAt(i);
							if (item.m_nListType == 2) { // Row

								// (j.jones 2010-06-21 17:31) - PLID 37981 - if a generic table, we won't blindly
								// add everything in the EMRInfoT setup (if for some reason the map is empty,
								// just load everything rather than nothing)
								CString strData = item.m_strData;
								if(IsGenericTable() && !m_bIsTemplateDetail && m_nEMRDetailID != -1
									&& mapDataIDsToName.GetSize() > 0) {
									if(!mapDataIDsToName.Lookup(item.m_nID, strData)) {
										//does not exist, skip this row
										continue;
									}
								}

								// (z.manning 2010-02-19 15:57) - PLID 37412 - Smart stamp tables load their rows elsewhere
								// (j.jones 2010-02-25 10:22) - PLID 37538 - user-defined rows are allowed
								//TES 3/17/2010 - PLID 37530 - Need to specify the stamp ID and index
								//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
								//(e.lally 2011-12-08) PLID 46471 - Specify if this is for a current medications or allergies detail

								// (a.walling 2011-08-11 15:49) - PLID 44987 - Insert if new, update strName if existing
								TableRowID rowID(item.m_nID,-1,NULL,-1,-1);
								BOOL bIsCurrentMedOrAllergy = ((IsCurrentMedicationsTable() || IsAllergiesTable()) ? TRUE : FALSE);
								std::pair<TableRowMap::iterator, bool> insertionPoint = mapTableRows.insert(TableRowMap::value_type(rowID, NULL));
								if (insertionPoint.second) {
									// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
									insertionPoint.first->second = AddRow(rowID, strData, item.m_strFormula, item.m_nDecimalPlaces, item.m_nEMRDataGroupID, item.m_bIsLabel, item.m_GlassesOrderDataType, item.m_nGlassesOrderDataID, item.m_nFlags, aroSequential, -1, bIsCurrentMedOrAllergy);
								} else {
									insertionPoint.first->second->strName = strData;
								}								
							} 
							else if (item.m_nListType >= 3) { // Column

								// (j.jones 2010-06-21 17:31) - PLID 37981 - if a generic table, we won't blindly
								// add everything in the EMRInfoT setup (if for some reason the map is empty,
								// just load everything rather than nothing)
								CString strData = item.m_strData;
								if(IsGenericTable() && !m_bIsTemplateDetail && m_nEMRDetailID != -1
									&& mapDataIDsToName.GetSize() > 0) {
									if(!mapDataIDsToName.Lookup(item.m_nID, strData)) {
										//does not exist, skip this row
										continue;
									}
								}

								//(e.lally 2011-12-08) PLID 46471 - We assume the first column of a current medication or allergy table detail to be the official usage checkbox column
								BOOL bIsCurrentMedOrAllergyUsageCol = (m_arTableColumns.GetSize()==0 && (IsCurrentMedicationsTable() || IsAllergiesTable()));
								if(bIsCurrentMedOrAllergyUsageCol){
									//(e.lally 2011-12-08) PLID 46471 - This had better be true or our assumption about which column to use for allergies and current medications is wrong
									ASSERT(item.m_nListType == LIST_TYPE_CHECKBOX);
								}

								//DRT 7/10/2007 - PLID 24105 - No longer requires a width
								// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
								//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
								// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
								// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
								AddColumn(item.m_nID, strData, item.m_nListType, item.m_bIsGrouped, item.m_strFormula, item.m_nDecimalPlaces, item.m_strInputMask, item.m_nListSubType, item.m_bAutoAlphabetizeDropdown, item.m_bIsLabel, item.m_strLongForm, item.m_etantAutoNumberType, item.m_strAutoNumberPrefix, item.m_bHasDropdownElements, item.m_bHasActiveDropdownElements, item.m_GlassesOrderDataType, item.m_nGlassesOrderDataID, item.m_eAutofillType, item.m_nFlags, item.m_strDropdownSeparator, item.m_strDropdownSeparatorFinal, item.m_strSpawnedItemsSeparator, bIsCurrentMedOrAllergyUsageCol, item.m_ewccWoundCareDataType);
							}
						}

						// (c.haag 2007-07-31 15:34) - By the time we get here, m_arTableRows and m_arTableColumns
						// should contain data in exactly the same order as if we had used that crazy "order by"
						// clause in the above parameter query (which is used when there is no EMN loader object).
					}
					break;

				case 6: //narrative
					// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
					//if(m_pParentTopic) m_pParentTopic->GetParentEMN()->LoadNarrativeMergeFields(this);
					break;
				case 5: //slider
					m_dSliderMin = dSliderMin;
					m_dSliderMax = dSliderMax;
					m_dSliderInc = dSliderInc;
					//correct bad data
					if(m_dSliderInc <= 0.0)
						m_dSliderInc = 1.0;
					break;
				case 4: //image
					//
					// (c.haag 2006-11-10 09:36) - PLID 23365 - Whenever we pull BackgroundImageType, we could
					// get a null value. If so, it means we pulled it from an EmrInfo item where the user must
					// choose the image to assign to the detail. In that event, we need to assign "blank default
					// values" to the image state. In our program, this means clearing the path and assigning the
					// image type of itDiagram.
					//
					// On a side note, I don't think this will ever be -1, but I'm changing the code for completeness
					//
					// (c.haag 2007-02-09 15:49) - PLID 23365 - The previous comment is wrong. The simple story
					// is that EmrInfoT.BackgroundImageType is NULL if there is no default image for the info
					// item. When the detail is added, it should have no image unless the detail itself is assigned
					// one in its InkImage*Override fields. The user will have to pick an image.
					//
					if (itUndefined == (m_eitBackgroundImageType = BackgroundImageType)) {
						m_strBackgroundImageFilePath.Empty();
					} else {
						m_strBackgroundImageFilePath = strBackgroundImageFilePath;
					}
					m_aisState->CreateFromSafeArrayVariant(m_varState);

					// (c.haag 2007-02-22 11:11) - PLID 24701 - If the following conditional is true,
					// it means that the EmrInfo item and the detail have differing background picture
					// paths. This can happen if an image detail is edited on the fly, and the info item's
					// background picture is changed.
					//
					// If this happens on a patient chart, this would be perfectly normal. Patient image
					// details should retain their state and background path because, as a general rule,
					// the detail state should try to retain as much data as possible if the info item
					// changes.
					//
					// Template details don't behave the same way. You cannot choose the image on a template
					// detail like you can on a patient chart. The template detail image background path must
					// always match that of the info item. If they do not match, we need to update the template
					// detail state with the new image path, and wipe out any ink in the process too.
					//
					// In general, we should strive to avoid changing m_varState here, but we must do it to
					// ensure the detail has the info item's image path, and to make sure that when the template
					// is saved, that we don't carry the old, wrong image path to data.
					//
					if (ComparePaths(m_strBackgroundImageFilePath, m_aisState->m_strImagePathOverride)) {
						// If this is a template, reload the state from data
						if (m_bIsTemplateDetail) {
							// (j.jones 2007-08-01 14:42) - PLID 26905 - we have no eligible active recordset to send
							m_varState = LoadEMRDetailStateDefault(NULL, m_nEMRInfoID, -1, -1, -1, lpCon, m_nRememberedDetailID); 
							m_aisState->CreateFromSafeArrayVariant(m_varState);
						}
					}
					break;

					case 2: //single-select list
					case 3: //multi-select list
						//TES 10/14/2004: If there are no items in the list, don't add anything.
						if(paDataItems != NULL)
						{
							CEMNLoader::CEmrDataItemArray arySortedDataItem;
							if(m_nEMRDetailID != -1 && pEMNLoader != NULL)
							{
								// (z.manning 2011-04-06 17:38) - PLID 43140 - This is an existing patient EMN detail so we
								// may have a per-detail sort order that we need to use instead of the default content order.
								CEMNLoader::CEmrDataGroupOrderArray* paryDetailOrder = pEMNLoader->GetOrderArrayByDetailID(m_nEMRDetailID);
								if(paryDetailOrder != NULL)
								{
									// (z.manning 2011-04-06 17:25) - PLID 43140 - We found a per-detail sort for this detail.
									// Note: aryDetailOrder must already be in the proper order.
									for(int nDetailOrderIndex = 0; nDetailOrderIndex < paryDetailOrder->GetSize(); nDetailOrderIndex++)
									{
										// (z.manning 2011-04-06 17:39) - PLID 43140 - Try to find the data item corresponding to
										// this entry in the detail order array and re-add all the data items in that order.
										CEMNLoader::EmrDataGroupOrder order = paryDetailOrder->GetAt(nDetailOrderIndex);
										CEMNLoader::EmrDataItem item;
										if(paDataItems->FindByDataGroupID(order.nDataGroupID, &item)) {
											// (j.jones 2011-04-29 10:53) - PLID 43122 - override the floated status
											// with the detail order object's value
											//item.m_bIsFloated = order.bIsFloated;
											// (a.walling 2013-03-18 09:32) - PLID 55723 - Use float count
											item.m_nFloatCount = order.bIsFloated ? 1 : -1;
											arySortedDataItem.Add(item);
										}
										else {
											// (z.manning 2011-04-06 17:43) - PLID 43140 - We didn't find a matching data item
											// for this detail order so no point in going any further as we can't safely use
											// the detail sort order.
											break;
										}
									}
								}
							}

							if(arySortedDataItem.GetSize() != paDataItems->GetSize()) {
								// (z.manning 2011-04-06 17:36) - PLID 43140 - If for whatever reason we couldn't match ALL
								// content data items to a correpsonding detail sort order, then let's just use the default
								// content order.
								arySortedDataItem.RemoveAll();
								arySortedDataItem.Append(*paDataItems);
							}

							for(int i = 0; i < arySortedDataItem.GetSize(); i++)
							{
								CEMNLoader::EmrDataItem& item = arySortedDataItem.GetAt(i);
								if (item.m_nListType == 1) { // (c.haag 2006-03-17 09:14) - PLID 19557 - If we converted from a table to a multi-select list, we need to ignore legacy table columns
									//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
									// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
									AddListElement(item.m_nID, item.m_nEMRDataGroupID, item.m_strData, item.m_bIsLabel, item.m_strLongForm, item.m_nActionsType, item.m_GlassesOrderDataType, item.m_nGlassesOrderDataID, item.m_nFloatCount != -1, item.m_strSpawnedItemsSeparator, item.m_nParentLabelID);
								}
							}
						}
						break;

				} // switch (eitDatabase)

			} // if (bUseEMNLoader) {
			// (c.haag 2007-08-06 11:48) - PLID 26954 - Special handling for when we have an
			// EMN spawner object
			else if (bUseEMNSpawner) {
				// (c.haag 2007-08-06 14:39) - PLID 26977 - We used to query data earlier on in LoadContent
				// and run a loop for all queried records to fill in the content. We no longer need to do this.
				CArray<CEMNSpawner::EmrDataItem,CEMNSpawner::EmrDataItem&>& arDataItems = pSpawnerInfoItem->m_arDataItems;
				m_strLongForm = VarString(pSpawnerInfoItem->m_vLongForm, "<Data>");
				m_nDataFormat = VarLong(pSpawnerInfoItem->m_vDataFormat, 0);
				m_strDataSeparator = VarString(pSpawnerInfoItem->m_vDataSeparator,", ");
				m_strDataSeparatorFinal = VarString(pSpawnerInfoItem->m_vDataSeparatorFinal," and ");
				m_bTableRowsAsFields = VarBool(pSpawnerInfoItem->m_vTableRowsAsFields); // (c.haag 2008-10-16 11:33) - PLID 31709

				// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
				m_nChildEMRInfoMasterID = VarLong(pSpawnerInfoItem->m_vChildEMRInfoMasterID, -1);
				m_bSmartStampsEnabled = VarBool(pSpawnerInfoItem->m_vSmartStampsEnabled, FALSE);

				//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
				m_bHasGlassesOrderData = VarBool(pSpawnerInfoItem->m_vHasGlassesOrderData, FALSE);
				m_golLens = (GlassesOrderLens)VarLong(pSpawnerInfoItem->m_vGlassesOrderLens, (long)golInvalid);
				//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
				m_bHasContactLensData = VarBool(pSpawnerInfoItem->m_vHasContactLensData, FALSE);
				// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
				m_bUseWithWoundCareCoding = VarBool(pSpawnerInfoItem->m_vUseWithWoundCareCoding, FALSE);

				// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
				m_nInfoFlags = VarLong(pSpawnerInfoItem->m_vInfoFlags, 0);

				// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
				// because they are now only calculated in the API, and not in Practice code
				/*
				// (j.jones 2007-08-27 10:35) - PLID 27056 - load the E/M coding data
				m_nEMCodeCategoryID = VarLong(pSpawnerInfoItem->m_vEMCodeCategoryID, -1);
				// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_eEMCodeUseTableCategories
				m_eEMCodeUseTableCategories = (EMCodeUseTableCategories)VarLong(pSpawnerInfoItem->m_vEMCodeUseTableCategories, (long)emcutcNone);
				m_bUseEMCoding = VarBool(pSpawnerInfoItem->m_vUseEMCoding, FALSE);
				m_emctEMCodingType = (EMCodingTypes)VarLong(pSpawnerInfoItem->m_vEMCodingType, (long)emctUndefined);
				*/

				// (j.jones 2007-07-18 15:04) - PLID 26730 - load whether or not the info item has Info actions,
				// which it usually does not, such that we don't have to search for them later
				m_eHasInfoActions = VarLong(pSpawnerInfoItem->m_vHasInfoActions, 0) == 1 ? ehiasHasInfoItems : ehiasHasNoInfoItems;

				// (a.walling 2009-01-13 13:54) - PLID 32107 - Load the info item's background info
				m_varInfoBackgroundImageFilePath = pSpawnerInfoItem->m_vBackgroundImageFilePath;
				m_varInfoBackgroundImageType = pSpawnerInfoItem->m_vBackgroundImageType;

				// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

				// (j.jones 2007-10-02 17:21) - PLID 26810 - load the EmrInfoT.Name
				// (j.jones 2007-10-22 09:43) - PLID 27830 - don't assign directly to m_strLabelText, as SetLabelText properly handles narratives
				SetLabelText(VarString(pSpawnerInfoItem->m_vName, ""));

				// (j.jones 2010-02-17 15:32) - PLID 37318 - If we reloaded a SmartStamp Image that is linked to a table,
				// that link may now be invalid. If so, this function will fix it.
				// (j.jones 2010-03-23 12:16) - PLID 37318 - added override for whether to call this or not
				if(bEnsureSmartStampTableLinked) {
					EnsureLinkedSmartStampTableValid();
				}

				// (a.walling 2008-07-01 09:17) - PLID 29271 - Preview Flags
				// (a.walling 2008-07-08 12:37) - PLID 29271 - Don't overwrite the preview flags from the info item
				//m_nPreviewFlags = VarLong(pSpawnerInfoItem->m_vPreviewFlags, 0);

				// (c.haag 2007-08-06 12:35) - PLID 26954 - If this detail was spawned, it can't have
				// problem-related information.

				//m.hancock - 4/9/2006 - PLID 20016 - EmrInfoT.DisableTableBorder is queried every time you bring up a tooltip
				BOOL bDisableTableBorder = VarBool(pSpawnerInfoItem->m_vDisableTableBorder, FALSE);
				m_bDisableTableBorder = (bDisableTableBorder == FALSE) ? false : true;

				EmrInfoType eitDatabase = (EmrInfoType)VarByte(pSpawnerInfoItem->m_vDataType);
				EmrInfoSubType eistDatabase = (EmrInfoSubType)VarByte(pSpawnerInfoItem->m_vDataSubType);
				// (a.walling 2008-02-08 11:11) - PLID 28857 - We can still return a data set if the info type change did not
				// require a state reset; for example, going from a single to multi, or a multi to single with one selection.
				// (z.manning 2011-10-21 16:19) - PLID 44649 - Compare on sub type here too.
				if(!IsSameEmrInfoType(m_EMRInfoType, m_EMRInfoSubType, eitDatabase, eistDatabase))
				{
					// (j.jones 2007-07-25 17:54) - PLID 26810 - set m_bIsForcedReload = TRUE,
					// to indicate that an external process is forcing a content reload
					// (z.manning, 03/31/2008) - PLID 29474 - We used to only set the forced reload flag if
					// the type change required a state reset, but we need to do it for ALL type changes
					// otherwise other parts of the program (e.g. the part that reloads the AdvDlg) will
					// not know about the type change.
					m_bIsForcedReload = TRUE;

					if(InfoTypeChangeRequiresDetailStateReset(m_EMRInfoType, m_EMRInfoSubType, eitDatabase, eistDatabase, this)) {
						//TES 6/22/2006 - This happens sometimes during the process of re-creating a detail whose type has been changed;
						//we can't load the content until our member variable is in sync.
						// (j.jones 2011-07-22 17:26) - PLID 43504 - pass in our connection
						SetNeedContentReload(pCon);
						return;
					}
				}

				switch(eitDatabase) {
				case eitTable: //table
					// (c.haag 2007-08-06 14:32) - PLID 26977 - Populate the table with data from the CEMNSpawner object
					if (arDataItems.GetSize() == 0) {
						//no rows, which means the item type had been changed and
						//we have no elements at all, so we must re-load from Admin
						// (z.manning, 05/23/2008) - PLID 30155 - Added Formula and DecimalPlaces
						// (z.manning 2009-01-15 15:25) - PLID 32724 - Added InputMask
						// (z.manning 2009-02-24 14:05) - PLID 33141 - Added group ID
						// (z.manning 2010-02-16 14:38) - PLID 37230 - ListSubType
						//TES 2/22/2010 - PLID 37463 - Added SmartStampsLongForm and UseSmartStampsLongForm
						// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
						// (z.manning 2010-07-26 14:31) - PLID 39848 - Removed SmartStampsLongForm and UseSmartStampsLongForm
						// (z.manning 2011-03-11) - PLID 42778 - Added HasDropdownElements
						//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
						// (z.manning 2011-03-21 11:23) - PLID 30608 - Added autofill type
						// (z.manning 2011-05-26 14:53) - PLID 43865 - Added DataFlags
						// (z.manning 2011-09-19 17:17) - PLID 41954 - Added dropdown separator
						// (z.manning 2011-11-07 10:46) - PLID 46309 - Added SpawnedItemsSeparator
						// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
						_CommandPtr pCmd = OpenParamQuery(pCon, 
							"SELECT EmrDataT.ID, EmrDataT.Data, EmrDataT.IsLabel, EmrDataT.ListType, EmrDataT.IsGrouped, EmrDataT.Formula, EmrDataT.DecimalPlaces, EmrDataT.AutoAlphabetizeDropDown, EmrInfoT.DataType, EmrInfoT.DataSubType, EmrDataT.DataFlags, "
							"EmrInfoT.BackgroundImageFilePath, EmrInfoT.BackgroundImageType, EmrInfoT.SliderMin, EmrInfoT.SliderMax, "
							"EmrInfoT.SliderInc, NULL AS SliderValue, EmrInfoT.LongForm, EmrDataT.LongForm AS DataLongForm, "
							"EmrInfoT.DataFormat, EmrInfoT.DataSeparator, EmrInfoT.DataSeparatorFinal, "
							"EmrInfoT.DisableTableBorder, "
							// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
							"EmrDataT.AutoNumberType, EmrDataT.AutoNumberPrefix, "
							"NULL AS ActionsType, EmrDataT.EmrDataGroupID, EmrDataT.ListSubType, "
							// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
							// (j.jones 2011-03-09 09:05) - PLID 42283 - added EMCodeUseTableCategories and Data_EMCodeCategoryID
							// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields, because they are now only calculated in the API, and not in Practice code
							//"EMRInfoMasterT.EMCodeCategoryID, EMRInfoMasterT.EMCodeUseTableCategories, EMRInfoMasterT.UseEMCoding AS Info_UseEMCoding, EMRInfoMasterT.EMCodingType, "
							// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
							"EmrDataT.InputMask, "
							// (a.walling 2013-02-28 17:13) - PLID 55390 - Avoid massive index scans and spooling and hash merges by limiting to individual seeks
							// based on the ListType of the Data when trying to determine whether dropdowns exist for this. Previously joining on the ad-hoc 
							// select dumps (potentially millions) of rows to a temp worktable and hashmatches. This replaces that with multiple seeks, unfortunately 2 per text/dropdown
							// item, but still better than before.
							// (a.walling 2013-07-02 14:57) - PLID 57401 - Indexed view for EMRTableDropdownInfoT's total/inactive counts
							"CONVERT(bit, "
								"CASE WHEN EmrDataT.ListType = 3 THEN "
									"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID AND TotalCount > InactiveCount) THEN 1 ELSE 0 END "
								"WHEN EmrDataT.ListType = 4 THEN "
									"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
								"ELSE 0 "
								"END "
							") AS HasActiveDropdownElements, "
							"CONVERT(bit, "
								"CASE WHEN EmrDataT.ListType IN (3,4) THEN "
									"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
								"ELSE 0 "
								"END "
							") AS HasDropdownElements, "
							"EmrDataT.GlassesOrderDataType, EmrDataT.GlassesOrderDataID, EmrDataT.AutofillType, DropdownSeparator, DropdownSeparatorFinal, SpawnedItemsSeparator, ParentLabelID, \r\n"
							// (j.jones 2011-04-28 14:39) - PLID 43122 - IsFloated is not used on tables
							"NULL AS IsFloated, "
							// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
							"EmrDataT.WoundCareDataType "
							"FROM EMRInfoT "
							"LEFT JOIN (SELECT * FROM EMRDataT WHERE Inactive = 0) EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID "
							"LEFT JOIN EMRInfoMasterT ON EMRInfoT.EMRInfoMasterID = EMRInfoMasterT.ID "
							// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
							"WHERE EMRInfoT.ID = ? "
							"ORDER BY (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN EmrDataT.SortOrder ELSE -1 END ELSE EmrDataT.SortOrder END), "
							"	(CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN '' ELSE EmrDataT.Data END ELSE '' END) "
							);
						AddParameterLong(pCmd, "InfoID", m_nEMRInfoID);
						_RecordsetPtr prs = CreateRecordset(pCmd);
						FieldsPtr pflds = prs->GetFields();
						FieldPtr fldID = pflds->GetItem("ID");
						FieldPtr fldData = pflds->GetItem("Data");
						FieldPtr fldLabel = pflds->GetItem("IsLabel");
						//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
						FieldPtr fldGlassesOrderDataType = pflds->GetItem("GlassesOrderDataType");
						FieldPtr fldGlassesOrderDataID = pflds->GetItem("GlassesOrderDataID");
						// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
						FieldPtr fldWoundCareDataType = pflds->GetItem("WoundCareDataType");

						long nGenericRowsNeeded = 0;
						long nGenericColumnsNeeded = 0;

						// (j.jones 2010-06-21 15:54) - PLID 37981 - if we are given default table content,
						// then we need to only load a set number of rows and columns
						// Note: this should be impossible, as you cannot spawn generic tables.
						if(pDefaultGenericTableContent) {
							nGenericRowsNeeded = pDefaultGenericTableContent->aryRows.GetSize();
							nGenericColumnsNeeded = pDefaultGenericTableContent->aryColumns.GetSize();
						}

						while (!prs->eof) {
							CString strData = AdoFldString(pflds->GetItem("Data"),"");
							long nListType = AdoFldLong(pflds, "ListType", -1);

							// (j.jones 2010-06-21 15:54) - PLID 37981 - if we are given default table content,
							// then we need to only load a set number of rows and columns
							// Note: this should be impossible, as you cannot spawn generic tables.
							if(pDefaultGenericTableContent) {
								if(nListType == 2 && nGenericRowsNeeded > 0) {
									//override the name
									DevicePluginUtils::RowElement *pRowOverride = pDefaultGenericTableContent->aryRows.GetAt(pDefaultGenericTableContent->aryRows.GetSize() - nGenericRowsNeeded);
									if(pRowOverride) {
										strData = pRowOverride->strName;
									}
									nGenericRowsNeeded--;
								}
								else if(nListType == 3 && nGenericColumnsNeeded > 0) {
									//override the name
									DevicePluginUtils::ColumnElement *pColumnOverride = pDefaultGenericTableContent->aryColumns.GetAt(pDefaultGenericTableContent->aryColumns.GetSize() - nGenericColumnsNeeded);
									if(pColumnOverride) {
										strData = pColumnOverride->strName;
									}
									nGenericColumnsNeeded--;
								}
								else {
									//we do not need to load this record
									prs->MoveNext();
									continue;
								}
							}

							// (z.manning 2010-02-16 14:38) - PLID 37230 - ListSubType
							BYTE nListSubType = AdoFldByte(pflds, "ListSubType", lstDefault);
							BOOL bIsGrouped = AdoFldBool(pflds, "IsGrouped", FALSE);
							// (a.walling 2013-03-21 10:14) - PLID 55804 - No more EM coding for data items necessary
							CString strFormula = AdoFldString(pflds, "Formula", "");
							BYTE nDecimalPlaces = AdoFldByte(pflds, "DecimalPlaces", 2);
							CString strInputMask = AdoFldString(pflds, "InputMask", "");
							long nGroupID = AdoFldLong(pflds, "EmrDataGroupID", -1);
							BOOL bAutoAlphabetizeDropdown = AdoFldBool(pflds, "AutoAlphabetizeDropDown", FALSE); // (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
							CString strDataLongForm = AdoFldString(pflds, "DataLongForm", "");
							// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
							EEmrTableAutoNumberType etantAutoNumberType = (EEmrTableAutoNumberType)AdoFldByte(pflds, "AutoNumberType", (BYTE)etantPerRow);
							CString strAutoNumberPrefix = AdoFldString(pflds, "AutoNumberPrefix", "");
							BOOL bHasDropdownElements = AdoFldBool(pflds, "HasDropdownElements", TRUE); // (z.manning 2011-03-11) - PLID 42778
							BOOL bHasActiveDropdownElements = AdoFldBool(pflds, "HasActiveDropdownElements", TRUE); // (z.manning 2011-03-11) - PLID 42778
							BYTE nAutofillType = AdoFldByte(pflds, "AutofillType", etatNone);
							long nFlags = AdoFldLong(pflds, "DataFlags", 0);
							CString strDropdownSeparator = AdoFldString(pflds, "DropdownSeparator", ", ");
							CString strDropdownSeparatorFinal = AdoFldString(pflds, "DropdownSeparatorFinal", ", ");
							CString strSpawnedItemsSeparator = AdoFldString(pflds, "SpawnedItemsSeparator", ", ");
							if(nListType == 2) { //Row
								// (z.manning 2010-02-19 15:57) - PLID 37412 - Smart stamp tables load their rows elsewhere
								// (j.jones 2010-02-25 10:22) - PLID 37538 - user-defined rows are allowed
								//TES 3/17/2010 - PLID 37530 - Need to specify the stamp ID and index
								//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
								//(e.lally 2011-12-08) PLID 46471 - Specify if this is for a current medications or allergies detail
								
								// (a.walling 2011-08-11 15:49) - PLID 44987 - Insert if new, update strName if existing
								TableRowID rowID(AdoFldLong(fldID),-1,NULL,-1,-1);

								std::pair<TableRowMap::iterator, bool> insertionPoint = mapTableRows.insert(TableRowMap::value_type(rowID, NULL));
								if (insertionPoint.second) {
									// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
									insertionPoint.first->second = AddRow(rowID, strData, strFormula, nDecimalPlaces, nGroupID, AdoFldBool(fldLabel), (GlassesOrderDataType)AdoFldLong(fldGlassesOrderDataType, (long)godtInvalid), AdoFldLong(fldGlassesOrderDataID, -1), nFlags, aroSequential, -1, ((IsCurrentMedicationsTable() || IsAllergiesTable()) ? TRUE : FALSE));
								} else {
									insertionPoint.first->second->strName = strData;
								}
							}
							else if(nListType >= 3) { //Column
								//(e.lally 2011-12-08) PLID 46471 - We assume the first column of a current medication or allergy table detail to be the official usage checkbox column
								BOOL bIsCurrentMedOrAllergyUsageCol = (m_arTableColumns.GetSize()==0 && (IsCurrentMedicationsTable() || IsAllergiesTable()));
								if(bIsCurrentMedOrAllergyUsageCol){
									//(e.lally 2011-12-08) PLID 46471 - This had better be true or our assumption about which column to use for allergies and current medications is wrong
									ASSERT(nListType == LIST_TYPE_CHECKBOX);
								}

								//DRT 7/10/2007 - PLID 24105 - No longer requires a width
								// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
								//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
								// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
								// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
								AddColumn(AdoFldLong(fldID), strData, nListType, bIsGrouped, strFormula, nDecimalPlaces, strInputMask, nListSubType, bAutoAlphabetizeDropdown, AdoFldBool(fldLabel), strDataLongForm, etantAutoNumberType, strAutoNumberPrefix, bHasDropdownElements, bHasActiveDropdownElements, (GlassesOrderDataType)AdoFldLong(fldGlassesOrderDataType, (long)godtInvalid), AdoFldLong(fldGlassesOrderDataID, -1), nAutofillType, nFlags, strDropdownSeparator, strDropdownSeparatorFinal, strSpawnedItemsSeparator, bIsCurrentMedOrAllergyUsageCol, (EWoundCareDataType)AdoFldLong(fldWoundCareDataType, (long)wcdtNone));
							}
							prs->MoveNext();
						}

					} else {

						// (j.jones 2010-06-21 17:31) - PLID 37981 - if a generic table, the EMRInfo content is loaded,
						// but we still need to load specific generic field names
						// Note: this should be impossible, as you cannot spawn generic tables.
						CMap<int, int, CString, CString> mapDataIDsToName;
						if(IsGenericTable() && !m_bIsTemplateDetail && m_nEMRDetailID != -1) {
							_RecordsetPtr rsOverride = CreateParamRecordset(pCon, "SELECT EMRDataID, Name FROM EMRDetailTableOverrideDataT "
								"WHERE EMRDetailID = {INT}", m_nEMRDetailID);
							while(!rsOverride->eof) {

								long nEMRDataID = AdoFldLong(rsOverride, "EMRDataID");
								CString strDataName = AdoFldString(rsOverride, "Name", "");

								//map it
								mapDataIDsToName.SetAt(nEMRDataID, strDataName);

								rsOverride->MoveNext();
							}
							rsOverride->Close();
						}

						for (int i=0; i < arDataItems.GetSize(); i++) {
							CEMNSpawner::EmrDataItem& item = arDataItems[i];
							if (item.m_nListType == 2) { // Row

								// (j.jones 2010-06-21 17:31) - PLID 37981 - if a generic table, we won't blindly
								// add everything in the EMRInfoT setup (if for some reason the map is empty,
								// just load everything rather than nothing)
								// Note: this should be impossible, as you cannot spawn generic tables.
								CString strData = item.m_strData;
								if(IsGenericTable() && !m_bIsTemplateDetail && m_nEMRDetailID != -1
									&& mapDataIDsToName.GetSize() > 0) {
									if(!mapDataIDsToName.Lookup(item.m_nID, strData)) {
										//does not exist, skip this row
										continue;
									}
								}

								// (z.manning 2010-02-19 15:57) - PLID 37412 - Smart stamp tables load their rows elsewhere
								// (j.jones 2010-02-25 10:22) - PLID 37538 - user-defined rows are allowed
								//TES 3/17/2010 - PLID 37530 - Need to specify the stamp ID and index
								//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
								//(e.lally 2011-12-08) PLID 46471 - Specify if this is for a current medications or allergies detail

								// (a.walling 2011-08-11 15:49) - PLID 44987 - Insert if new, update strName if existing
								TableRowID rowID(item.m_nID,-1,NULL,-1,-1);

								std::pair<TableRowMap::iterator, bool> insertionPoint = mapTableRows.insert(TableRowMap::value_type(rowID, NULL));
								if (insertionPoint.second) {
									// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
									insertionPoint.first->second = AddRow(rowID, strData, item.m_strFormula, item.m_nDecimalPlaces, item.m_nEMRDataGroupID, item.m_bIsLabel, item.m_GlassesOrderDataType, item.m_nGlassesOrderDataID, item.m_nFlags, aroSequential, -1, ((IsCurrentMedicationsTable() || IsAllergiesTable()) ? TRUE : FALSE));
								} else {
									insertionPoint.first->second->strName = strData;
								}
							} else if (item.m_nListType >= 3) { // Column

								// (j.jones 2010-06-21 17:31) - PLID 37981 - if a generic table, we won't blindly
								// add everything in the EMRInfoT setup (if for some reason the map is empty,
								// just load everything rather than nothing)
								// Note: this should be impossible, as you cannot spawn generic tables.
								CString strData = item.m_strData;
								if(IsGenericTable() && !m_bIsTemplateDetail && m_nEMRDetailID != -1
									&& mapDataIDsToName.GetSize() > 0) {
									if(!mapDataIDsToName.Lookup(item.m_nID, strData)) {
										//does not exist, skip this row
										continue;
									}
								}

								//(e.lally 2011-12-08) PLID 46471 - We assume the first column of a current medication or allergy table detail to be the official usage checkbox column
								BOOL bIsCurrentMedOrAllergyUsageCol = (m_arTableColumns.GetSize()==0 && (IsCurrentMedicationsTable() || IsAllergiesTable()));
								if(bIsCurrentMedOrAllergyUsageCol){
									//(e.lally 2011-12-08) PLID 46471 - This had better be true or our assumption about which column to use for allergies and current medications is wrong
									ASSERT(item.m_nListType == LIST_TYPE_CHECKBOX);
								}

								//DRT 7/10/2007 - PLID 24105 - No longer requires a width
								// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
								//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
								// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
								// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
								AddColumn(item.m_nID, strData, item.m_nListType, item.m_bIsGrouped, item.m_strFormula, item.m_nDecimalPlaces, item.m_strInputMask, item.m_nListSubType, item.m_bAutoAlphabetizeDropdown, item.m_bIsLabel, item.m_strLongForm, item.m_etantAutoNumberType, item.m_strAutoNumberPrefix, item.m_bHasDropdownElements, item.m_bHasActiveDropdownElements, item.m_GlassesOrderDataType, item.m_nGlassesOrderDataID, item.m_eAutofillType, item.m_nFlags, item.m_strDropdownSeparator, item.m_strDropdownSeparatorFinal, item.m_strSpawnedItemsSeparator, bIsCurrentMedOrAllergyUsageCol, item.m_ewccWoundCareDataType);
							}
						}
					}
					break;
				case 6: //narrative
					// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
					//if(m_pParentTopic) m_pParentTopic->GetParentEMN()->LoadNarrativeMergeFields(this);
					break;
				case 5: //slider
					m_dSliderMin = VarDouble(pSpawnerInfoItem->m_vSliderMin, 0.0);
					m_dSliderMax = VarDouble(pSpawnerInfoItem->m_vSliderMax, 10.0);
					m_dSliderInc = VarDouble(pSpawnerInfoItem->m_vSliderInc, 1.0);
					//correct bad data
					if(m_dSliderInc <= 0.0)
						m_dSliderInc = 1.0;
					break;
				case 4: //image
					{
					//
					// (c.haag 2006-11-10 09:36) - PLID 23365 - Whenever we pull BackgroundImageType, we could
					// get a null value. If so, it means we pulled it from an EmrInfo item where the user must
					// choose the image to assign to the detail. In that event, we need to assign "blank default
					// values" to the image state. In our program, this means clearing the path and assigning the
					// image type of itDiagram.
					//
					// On a side note, I don't think this will ever be -1, but I'm changing the code for completeness
					//
					// (c.haag 2007-02-09 15:49) - PLID 23365 - The previous comment is wrong. The simple story
					// is that EmrInfoT.BackgroundImageType is NULL if there is no default image for the info
					// item. When the detail is added, it should have no image unless the detail itself is assigned
					// one in its InkImage*Override fields. The user will have to pick an image.
					//
					if (itUndefined == (m_eitBackgroundImageType = (eImageType)VarLong(pSpawnerInfoItem->m_vBackgroundImageType, -1))) {
						m_strBackgroundImageFilePath.Empty();
					} else {
						m_strBackgroundImageFilePath = VarString(pSpawnerInfoItem->m_vBackgroundImageFilePath, "");
					}
					m_aisState->CreateFromSafeArrayVariant(m_varState);

					//DRT 2/14/2008 - PLID 28603 - Pull the image hotspots out of the spawner.
					CArray<CEMNSpawner::EmrImageHotSpotItem, CEMNSpawner::EmrImageHotSpotItem&>& aryHotSpotItems = pSpawnerInfoItem->m_aryHotSpotItems;

					for(int i = 0; i < aryHotSpotItems.GetSize(); i++) {
						CEMNSpawner::EmrImageHotSpotItem& item = aryHotSpotItems[i];
						//Create a new hotspot
						CEMRHotSpot *pSpot = new CEMRHotSpot;
						pSpot->SetID(item.m_nID);
						pSpot->SetGroupID(item.m_nEMRHotSpotGroupID);
						pSpot->LoadData(item.m_strData);
						//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for hotspots
						pSpot->SetOriginalAnatomicLocation(item.m_nAnatomicLocationID, item.m_strAnatomicLocation);
						pSpot->SetOriginalAnatomicQualifier(item.m_nAnatomicQualifierID, item.m_strAnatomicQualifier);
						pSpot->SetOriginalSide(item.m_asSide);
						pSpot->Set3DHotSpotID(item.m_n3DHotSpotID);

						m_aryImageHotSpots.Add(pSpot);
					}

					// (z.manning 2011-10-25 10:51) - PLID 39401 - Pull stamp exclusions
					m_StampExclusions = pSpawnerInfoItem->m_StampExclusions;
					// (a.wilson 2013-03-26 11:00) - PLID 55826 - set loaded flag.
					m_bStampExclusionsLoaded = true;

					// (c.haag 2007-02-22 11:11) - PLID 24701 - If the following conditional is true,
					// it means that the EmrInfo item and the detail have differing background picture
					// paths. This can happen if an image detail is edited on the fly, and the info item's
					// background picture is changed.
					//
					// If this happens on a patient chart, this would be perfectly normal. Patient image
					// details should retain their state and background path because, as a general rule,
					// the detail state should try to retain as much data as possible if the info item
					// changes.
					//
					// Template details don't behave the same way. You cannot choose the image on a template
					// detail like you can on a patient chart. The template detail image background path must
					// always match that of the info item. If they do not match, we need to update the template
					// detail state with the new image path, and wipe out any ink in the process too.
					//
					// In general, we should strive to avoid changing m_varState here, but we must do it to
					// ensure the detail has the info item's image path, and to make sure that when the template
					// is saved, that we don't carry the old, wrong image path to data.
					//
					if (ComparePaths(m_strBackgroundImageFilePath, m_aisState->m_strImagePathOverride)) {
						// If this is a template, reload the state from data
						if (m_bIsTemplateDetail) {
							// (j.jones 2007-08-01 14:42) - PLID 26905 - we have no eligible active recordset to send
							m_varState = LoadEMRDetailStateDefault(NULL, m_nEMRInfoID, -1, -1, -1, lpCon, m_nRememberedDetailID); 
							m_aisState->CreateFromSafeArrayVariant(m_varState);
						}
					}
					}
					break;
				case 2: //single-select list
				case 3: //multi-select list

					// (c.haag 2007-08-06 13:24) - PLID 26977 - Populate m_arListElements with data from the CEMNSpawner object
					for (int i=0; i < arDataItems.GetSize(); i++) {
						CEMNSpawner::EmrDataItem& item = arDataItems[i];
						if (item.m_nListType == 1) { // (c.haag 2006-03-17 09:14) - PLID 19557 - If we converted from a table to a multi-select list, we need to ignore legacy table columns
							//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
							// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
							AddListElement(item.m_nID, item.m_nEMRDataGroupID, item.m_strData, item.m_bIsLabel, item.m_strLongForm, item.m_nActionsType, item.m_GlassesOrderDataType, item.m_nGlassesOrderDataID, item.m_bIsFloated, item.m_strSpawnedItemsSeparator, item.m_nParentLabelID);
						}
					}
					break;
				} // switch(eitDatabase) {
			} // if (bUseEMNSpawner) {
			else {
				FieldsPtr pflds = prsContent->GetFields();

				// (j.jones 2007-07-18 15:04) - PLID 26730 - load whether or not the info item has Info actions,
				// which it usually does not, such that we don't have to search for them later
				m_eHasInfoActions = AdoFldLong(prsContent, "HasInfoActions", 0) == 1 ? ehiasHasInfoItems : ehiasHasNoInfoItems;

				// (j.jones 2007-10-02 17:21) - PLID 26810 - load the EmrInfoT.Name
				// (j.jones 2007-10-22 09:43) - PLID 27830 - don't assign directly to m_strLabelText, as SetLabelText properly handles narratives
				// (c.haag 2008-06-17 18:38) - PLID 30319 - Don't do this for built-in macro details 
				if (EMR_BUILT_IN_INFO__TEXT_MACRO != m_nEMRInfoID) {
					SetLabelText(AdoFldString(prsContent, "Name", ""));
				}

				prsContent = prsContent->NextRecordset(NULL);

				// (z.manning 2011-10-25 11:27) - PLID 39401 - We may or may not need to load stamp exclusions here.
				if(!sqlStampExclusions.IsEmpty())
				{
					m_StampExclusions.LoadFromRecordset(prsContent);
					// (a.wilson 2013-03-26 11:00) - PLID 55826 - set loaded flag.
					m_bStampExclusionsLoaded = true;
					prsContent = prsContent->NextRecordset(NULL);
				}

				pflds = prsContent->GetFields();
				FieldPtr fldID = pflds->GetItem("ID");
				FieldPtr fldDataGroupID = pflds->GetItem("EmrDataGroupID");
				FieldPtr fldData = pflds->GetItem("Data");
				FieldPtr fldLabel = pflds->GetItem("IsLabel");
				FieldPtr fldActionsType = pflds->GetItem("ActionsType");		
				//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
				FieldPtr fldGlassesOrderDataType = pflds->GetItem("GlassesOrderDataType");
				FieldPtr fldGlassesOrderDataID = pflds->GetItem("GlassesOrderDataID");
				// (j.jones 2011-04-28 14:39) - PLID 43122 - load the IsFloated field
				FieldPtr fldIsFloated = pflds->GetItem("IsFloated");
				FieldPtr fldSpawnedItemsSeparator = pflds->GetItem("SpawnedItemsSeparator");
				// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
				FieldPtr fldWoundCareDataType = pflds->GetItem("WoundCareDataType");
				// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
				FieldPtr fldParentLabelID = pflds->GetItem("ParentLabelID");

				// (c.haag 2006-06-30 08:31) - PLID 19977 - Load problem-related values
				// (c.haag 2006-08-08 09:24) - PLID 21835 - A detail can have only one problem, so we need to flag if it has been loaded already
				// (c.haag 2006-10-19 11:46) - PLID 21454 - Do not load a deleted problem
				
				// (c.haag 2007-08-17 13:16) - PLID 25790 - If we get here, it either means the detail was added after
				// the initial load, or that it was simply not available in the EMN loader. We will need to get the
				// value from data.
				// (j.jones 2008-07-15 17:21) - PLID 30739 - supported EMRRegardingType and EMRRegardingID
				// (j.jones 2008-07-18 11:06) - PLID 30779 - for the new problem tracking,
				// load up all the problems on this detail in a recordset, but do NOT
				// reload a problem that has been modified
				// (c.haag 2008-12-03 16:24) - PLID 32316 - Moved to outside the item detail recordset loop
				if((!((m_nEMRDetailID == -1 && m_nEMRTemplateDetailID == -1) || bForceLoadFromAdmin) &&
					!(m_nEMRDetailID == -1 && m_nEMRTemplateDetailID != -1)) &&
					m_nEMRDetailID > 0) {

						
					// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
					// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
					// (c.haag 2009-05-12 12:23) - PLID 34234 - Problem link table
					// (c.haag 2009-05-20 10:44) - PLID 34311 - Added EMR problem link ID
					// (z.manning 2009-05-27 10:07) - PLID 34297 - Added patient ID
					// (b.spivey, October 22, 2013) - PLID 58677 - added codeID
					// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
					// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
					// (a.walling 2014-07-23 09:09) - PLID 63003 - Filter on EMRProblemsT.PatientID when possible
					// (s.tullis 2015-02-23 15:44) - PLID 64723
					// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
					_RecordsetPtr prsProblems = CreateParamRecordset(pCon, "SELECT EMRProblemsT.ID, Description, StatusID, "
						"EnteredDate, ModifiedDate, OnsetDate, EMRRegardingType, EMRRegardingID, EMRDataID, "
						"DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemActionID, EmrProblemLinkT.ID AS EmrProblemLinkID, EmrProblemsT.CodeID, "
						"EmrProblemsT.PatientID, EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
						"FROM EMRProblemsT "
						"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
						"WHERE PatientID = {INT} AND ((EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}) "
						"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT})) "
						"AND Deleted = 0", GetPatientID(), eprtEmrItem, m_nEMRDetailID, eprtEmrDataItem, m_nEMRDetailID);
					while(!prsProblems->eof) {
						// (c.haag 2009-05-20 10:44) - PLID 34311 - Now load the data into memory as problem
						// links, not problems.
						long nEMRProblemLinkID = AdoFldLong(prsProblems, "EMRProblemLinkID");
						EMRProblemRegardingTypes eprtTypeID = (EMRProblemRegardingTypes)AdoFldLong(prsProblems, "EMRRegardingType");
						long nEMRRegardingID = AdoFldLong(prsProblems, "EMRRegardingID", -1);
						long nEMRDataID = AdoFldLong(prsProblems, "EMRDataID", -1);

						//see if it is in our list
						BOOL bFound = FALSE;
						for(int i=0; i<m_apEmrProblemLinks.GetSize() && !bFound; i++) {
							CEmrProblemLink *pLink = m_apEmrProblemLinks.GetAt(i);
							if (pLink->GetID() == nEMRProblemLinkID) {
								// if the problem has been modified at all, do not reload from data
								CEmrProblem* pProblem = pLink->GetProblem();
								if (!pProblem->m_bIsModified) {
									pProblem->ReloadFromData(prsProblems->Fields);
								}
								bFound = TRUE;										
							}
						}

						if(bFound) {
							prsProblems->MoveNext();
							continue;
						}

						//if we're still here, it wasn't in our list, so add it
						// (c.haag 2009-05-19 17:22) - PLID 34311 - Allocate a problem from the parent EMR. If there
						// is no parent EMR, that means we're loading for some purpose outside the PIC; like getting
						// a list of problems for a patient warning, for example. In those cases, the problems will be
						// self managed through reference counting, and not have any one owner. This is OK because 
						// in those cases, no saving ever takes place; so there doesn't need to be a central owner for
						// coordinating problems.

						// Get the problem for the problem link
						CEmrProblem* pProblem = NULL;
						if (NULL != m_pParentTopic && NULL != m_pParentTopic->GetParentEMN()) {
							CEMR* pEMR = m_pParentTopic->GetParentEMN()->GetParentEMR();
							if (NULL != pEMR) {
								pProblem = pEMR->AllocateEmrProblem(prsProblems->Fields);
							}
						}
						if (NULL == pProblem) {
							pProblem = new CEmrProblem(prsProblems->Fields);
						}
						// Create the problem link
						CEmrProblemLink* pNewLink = new CEmrProblemLink(pProblem, nEMRProblemLinkID, eprtTypeID, nEMRRegardingID, nEMRDataID);
						pNewLink->UpdatePointersWithDetail(this);
						m_apEmrProblemLinks.Add(pNewLink);
						// Release our local reference to the problem
						pProblem->Release();

						// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
						if (NULL != m_pParentTopic && NULL != m_pParentTopic->GetParentEMN()) {
							CWnd *pWnd = m_pParentTopic->GetParentEMN()->GetInterface();
							if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
								pWnd->PostMessage(NXM_EMR_PROBLEM_CHANGED);
							}
						}

						prsProblems->MoveNext();
					} // while(!prsProblems->eof) {
					prsProblems->Close();
				} // if((!((m_nEMRDetailID == -1 && m_nEMRTemplateDetailID == -1) || bForceLoadFromAdmin) &&
				//	!(m_nEMRDetailID == -1 && m_nEMRTemplateDetailID != -1))) {

				long nGenericRowsNeeded = 0;
				long nGenericColumnsNeeded = 0;

				// (j.jones 2010-06-21 15:54) - PLID 37981 - if we are given default table content,
				// then we need to only load a set number of rows and columns
				if(pDefaultGenericTableContent) {
					nGenericRowsNeeded = pDefaultGenericTableContent->aryRows.GetSize();
					nGenericColumnsNeeded = pDefaultGenericTableContent->aryColumns.GetSize();
				}

				while (!prsContent->eof) {
					CString strData = AdoFldString(pflds->GetItem("Data"),"");
					long nListType = AdoFldLong(pflds, "ListType", -1);

					// (j.jones 2010-06-21 15:54) - PLID 37981 - if we are given default table content,
					// then we need to only load a set number of rows and columns
					if(pDefaultGenericTableContent) {
						if(nListType == 2 && nGenericRowsNeeded > 0) {
							//override the name
							DevicePluginUtils::RowElement *pRowOverride = pDefaultGenericTableContent->aryRows.GetAt(pDefaultGenericTableContent->aryRows.GetSize() - nGenericRowsNeeded);
							if(pRowOverride) {
								strData = pRowOverride->strName;
							}
							nGenericRowsNeeded--;
						}
						else if(nListType == 3 && nGenericColumnsNeeded > 0) {
							//override the name
							DevicePluginUtils::ColumnElement *pColumnOverride = pDefaultGenericTableContent->aryColumns.GetAt(pDefaultGenericTableContent->aryColumns.GetSize() - nGenericColumnsNeeded);
							if(pColumnOverride) {
								strData = pColumnOverride->strName;
							}
							nGenericColumnsNeeded--;
						}
						else {
							//we do not need to load this record
							prsContent->MoveNext();
							continue;
						}
					}

					BYTE nListSubType = AdoFldByte(pflds, "ListSubType", lstDefault);
					BOOL bIsGrouped = AdoFldBool(pflds, "IsGrouped", FALSE);
					// (z.manning, 05/23/2008) - PLID 30155
					CString strFormula = AdoFldString(pflds, "Formula", "");
					BYTE nDecimalPlaces = AdoFldByte(pflds, "DecimalPlaces", 2);
					CString strInputMask = AdoFldString(pflds, "InputMask", "");
					// (z.manning 2009-02-24 14:06) - PLID 33141
					long nGroupID = AdoFldLong(pflds, "EmrDataGroupID", -1);
					// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
					BOOL bAutoAlphabetizeDropdown = AdoFldBool(pflds, "AutoAlphabetizeDropDown", FALSE);
					// (z.manning 2010-07-29 15:13) - PLID 36150
					CString strDataLongForm = AdoFldString(pflds, "DataLongForm", "");
					BOOL bHasDropdownElements = AdoFldBool(pflds, "HasDropdownElements", TRUE);
					BOOL bHasActiveDropdownElements = AdoFldBool(pflds, "HasActiveDropdownElements", TRUE);
					BYTE nAutofillType = AdoFldByte(pflds, "AutofillType", etatNone);
					long nFlags = AdoFldLong(pflds, "DataFlags", 0);
					CString strDropdownSeparator = AdoFldString(pflds, "DropdownSeparator", ", ");
					CString strDropdownSeparatorFinal = AdoFldString(pflds, "DropdownSeparatorFinal", ", ");
					CString strSpawnedItemsSeparator = AdoFldString(pflds, "SpawnedItemsSeparator", ", ");

					m_strLongForm = AdoFldString(prsContent, "LongForm","<Data>");
					m_nDataFormat = AdoFldLong(prsContent, "DataFormat",0);
					m_strDataSeparator = AdoFldString(prsContent, "DataSeparator",", ");
					m_strDataSeparatorFinal = AdoFldString(prsContent, "DataSeparatorFinal"," and ");
					m_bTableRowsAsFields = AdoFldBool(prsContent, "TableRowsAsFields"); // (c.haag 2008-10-16 11:33) - PLID 31709

					// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
					EEmrTableAutoNumberType etantAutoNumberType = (EEmrTableAutoNumberType)AdoFldByte(prsContent, "AutoNumberType", (BYTE)etantPerRow);
					CString strAutoNumberPrefix = AdoFldString(prsContent, "AutoNumberPrefix", "");

					// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
					m_nChildEMRInfoMasterID = AdoFldLong(pflds, "ChildEMRInfoMasterID", -1);
					m_bSmartStampsEnabled = AdoFldBool(pflds, "SmartStampsEnabled", FALSE);
					m_nChildEMRDetailID = AdoFldLong(pflds, "ChildEMRDetailID", -1);
					m_nChildEMRTemplateDetailID = AdoFldLong(pflds, "ChildEMRTemplateDetailID", -1);

					//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
					m_bHasGlassesOrderData = AdoFldBool(pflds, "HasGlassesOrderData", FALSE);
					m_golLens = (GlassesOrderLens)AdoFldLong(pflds, "GlassesOrderLens", (long)golInvalid);
					//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
					m_bHasContactLensData = AdoFldBool(pflds, "HasContactLensData", FALSE);
					// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
					m_bUseWithWoundCareCoding = AdoFldBool(pflds, "UseWithWoundCareCoding", FALSE);

					// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
					m_nInfoFlags = AdoFldLong(pflds, "InfoFlags", 0);

					// (a.walling 2009-01-13 14:01) - PLID 32107 - Load the info item's background info
					m_varInfoBackgroundImageFilePath = prsContent->Fields->Item["BackgroundImageFilePath"]->Value;
					m_varInfoBackgroundImageType = prsContent->Fields->Item["BackgroundImageType"]->Value;

					// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
					// because they are now only calculated in the API, and not in Practice code
					/*
					// (j.jones 2007-08-27 10:35) - PLID 27056 - load the E/M coding data
					m_nEMCodeCategoryID = AdoFldLong(prsContent, "EMCodeCategoryID", -1);
					// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_eEMCodeUseTableCategories
					m_eEMCodeUseTableCategories = (EMCodeUseTableCategories)AdoFldLong(prsContent, "EMCodeUseTableCategories", (long)emcutcNone);
					m_bUseEMCoding = AdoFldBool(prsContent, "Info_UseEMCoding", FALSE);
					m_emctEMCodingType = (EMCodingTypes)AdoFldLong(prsContent, "EMCodingType", (long)emctUndefined);
					*/

					// (a.walling 2008-07-01 09:18) - PLID 29271 - Preview Flags
					// (a.walling 2008-07-08 12:37) - PLID 29271 - Don't overwrite the preview flags from the info item
					//m_nPreviewFlags = AdoFldLong(prs, "PreviewFlags", 0);

					// (a.walling 2013-03-21 10:14) - PLID 55804 - No more EM coding for data items necessary

					//m.hancock - 4/9/2006 - PLID 20016 - EmrInfoT.DisableTableBorder is queried every time you bring up a tooltip
					BOOL bDisableTableBorder = AdoFldBool(prsContent, "DisableTableBorder", FALSE);
					m_bDisableTableBorder = (bDisableTableBorder == FALSE) ? false : true;

					// (j.jones 2010-02-17 15:32) - PLID 37318 - If we reloaded a SmartStamp Image that is linked to a table,
					// that link may now be invalid. If so, this function will fix it.
					// (j.jones 2010-03-23 12:16) - PLID 37318 - added override for whether to call this or not
					if(bEnsureSmartStampTableLinked) {
						EnsureLinkedSmartStampTableValid();
					}

					EmrInfoType eitDatabase = (EmrInfoType)AdoFldByte(pflds, "DataType");
					EmrInfoSubType eistDatabase = (EmrInfoSubType)AdoFldByte(pflds, "DataSubType");
					// (a.walling 2008-02-08 11:11) - PLID 28857 - We can still return a data set if the info type change did not
					// require a state reset; for example, going from a single to multi, or a multi to single with one selection.
					// (z.manning 2011-10-21 16:19) - PLID 44649 - Compare on sub type here too.
					if(!IsSameEmrInfoType(m_EMRInfoType, m_EMRInfoSubType, eitDatabase, eistDatabase))
					{
						// (j.jones 2007-07-25 17:54) - PLID 26810 - set m_bIsForcedReload = TRUE,
						// to indicate that an external process is forcing a content reload
						// (z.manning, 03/31/2008) - PLID 29474 - We used to only set the forced reload flag if
						// the type change required a state reset, but we need to do it for ALL type changes
						// otherwise other parts of the program (e.g. the part that reloads the AdvDlg) will
						// not know about the type change.
						m_bIsForcedReload = TRUE;

						if(InfoTypeChangeRequiresDetailStateReset(m_EMRInfoType, m_EMRInfoSubType, eitDatabase, eistDatabase, this)) {
							//TES 6/22/2006 - This happens sometimes during the process of re-creating a detail whose type has been changed;
							//we can't load the content until our member variable is in sync.
							// (j.jones 2011-07-22 17:26) - PLID 43504 - pass in our connection
							SetNeedContentReload(pCon);
							return;
						}
					}

					switch(AdoFldByte(pflds, "DataType")) {
					case eitTable: //table
						{
							if(fldID->GetValue().vt == VT_NULL) {
								//no rows, which means the item type had been changed and
								//we have no elements at all, so we must re-load from Admin
								// (c.haag 2007-02-05 13:36) - PLID 24423 - Cleaned up the sorting and did special handling for Current Medications
								// (c.haag 2007-04-03 08:59) - PLID 25468 - Expanded the special sorting described above to special Allergy items
								//DRT 7/26/2007 - PLID 26830 - Parameterized to speed execution.
								// (z.manning, 05/23/2008) - PLID 30155 - Added EmrDataT.Formula and DecimalPlaces
								// (z.manning 2009-01-15 15:28) - PLID 32724 - Added InputMask
								// (z.manning 2009-02-24 14:04) - PLID 33141 - Added group ID
								// (z.manning 2010-02-16 14:41) - PLID 37230 - ListSubType
								//TES 2/22/2010 - PLID 37463 - Added SmartStampsLongForm and UseSmartStampsLongForm
								// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
								// (j.jones 2010-06-09 15:12) - PLID 37981 - supported generic tables, which use EMRDetailTableOverrideDataT when on EMN details
								// (EMRDetailTableOverrideDataQ will be empty if m_nEMRDetailID = -1, which is fine)
								// (z.manning 2010-07-26 14:32) - PLID 39848 - Removed SmartStampsLongForm and UseSmartStampsLongForm
								// (z.manning 2011-03-11) - PLID 42778 - Added HasDropdownElements
								//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
								// (z.manning 2011-03-21 11:25) - PLID 30608 - Added autofill type
								// (z.manning 2011-05-26 14:55) - PLID 43865 - Added DataFlags
								// (z.manning 2011-09-19 17:19) - PLID 41954 - Dropdown separators
								// (z.manning 2011-11-07 10:47) - PLID 46309 - Added SpawnedItemsSeparator
								// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
								prsContent = CreateParamRecordset(pCon, 
									"SELECT EmrDataT.ID, "
									"CASE WHEN EMRInfoT.DataSubType = {INT} AND EMRDetailTableOverrideDataQ.Name Is Not Null THEN EMRDetailTableOverrideDataQ.Name ELSE EmrDataT.Data END AS Data, "
									"EmrDataT.IsLabel, EmrDataT.ListType, EmrDataT.IsGrouped, EmrDataT.Formula, EmrDataT.DecimalPlaces, EmrInfoT.DataType, EmrInfoT.DataSubType, EmrDataT.DataFlags, "
									"EmrInfoT.BackgroundImageFilePath, EmrInfoT.BackgroundImageType, EmrInfoT.SliderMin, EmrInfoT.SliderMax, EmrDataT.AutoAlphabetizeDropdown, "
									// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
									"EmrDataT.AutoNumberType, EmrDataT.AutoNumberPrefix, "
									"EmrInfoT.SliderInc, NULL AS SliderValue, EmrInfoT.LongForm, EmrDataT.LongForm AS DataLongForm, "
									"EmrInfoT.DataFormat, EmrInfoT.DataSeparator, EmrInfoT.DataSeparatorFinal, "
									"EmrInfoT.DisableTableBorder, "
									"NULL AS ActionsType, EmrDataT.EmrDataGroupID, EmrDataT.ListSubType, "
									// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
									// (j.jones 2011-03-09 09:05) - PLID 42283 - added EMCodeUseTableCategories and Data_EMCodeCategoryID
									// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields, because they are now only calculated in the API, and not in Practice code
									//"EMRInfoMasterT.EMCodeCategoryID, EMRInfoMasterT.EMCodeUseTableCategories, EMRInfoMasterT.UseEMCoding AS Info_UseEMCoding, EMRInfoMasterT.EMCodingType, "
									// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
									"EmrDataT.InputMask, "
									// (a.walling 2013-02-28 17:13) - PLID 55390 - Avoid massive index scans and spooling and hash merges by limiting to individual seeks
									// based on the ListType of the Data when trying to determine whether dropdowns exist for this. Previously joining on the ad-hoc 
									// select dumps (potentially millions) of rows to a temp worktable and hashmatches. This replaces that with multiple seeks, unfortunately 2 per text/dropdown
									// item, but still better than before.
									// (a.walling 2013-07-02 14:57) - PLID 57401 - Indexed view for EMRTableDropdownInfoT's total/inactive counts
									"CONVERT(bit, "
										"CASE WHEN EmrDataT.ListType = 3 THEN "
											"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID AND TotalCount > InactiveCount) THEN 1 ELSE 0 END "
										"WHEN EmrDataT.ListType = 4 THEN "
											"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
										"ELSE 0 "
										"END "
									") AS HasActiveDropdownElements, "
									"CONVERT(bit, "
										"CASE WHEN EmrDataT.ListType IN (3,4) THEN "
											"CASE WHEN EXISTS(SELECT EmrTableDropdownUsageV.EmrDataID FROM EmrTableDropdownUsageV WHERE EmrTableDropdownUsageV.EmrDataID = EmrDataT.ID) THEN 1 ELSE 0 END "
										"ELSE 0 "
										"END "
									") AS HasDropdownElements, "
									"EmrDataT.GlassesOrderDataType, EmrDataT.GlassesOrderDataID, EmrDataT.AutofillType, "
									// (j.jones 2011-04-28 14:39) - PLID 43122 - IsFloated is not used on tables
									"NULL AS IsFloated, DropdownSeparator, DropdownSeparatorFinal, SpawnedItemsSeparator, ParentLabelID, "
									// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
									"EmrDataT.WoundCareDataType "
									"FROM EMRInfoT "
									"LEFT JOIN (SELECT * FROM EMRDataT WHERE Inactive = 0) EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID "
									"LEFT JOIN (SELECT EMRDataID, Name FROM EMRDetailTableOverrideDataT WHERE EMRDetailID = {INT}) AS EMRDetailTableOverrideDataQ ON EMRDataT.ID = EMRDetailTableOverrideDataQ.EMRDataID "
									"LEFT JOIN EMRInfoMasterT ON EMRInfoT.EMRInfoMasterID = EMRInfoMasterT.ID "
									// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
									"WHERE EMRInfoT.ID = {INT} "
									"ORDER BY (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN EmrDataT.SortOrder ELSE -1 END ELSE EmrDataT.SortOrder END), "
									"	(CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND ListType <> 2 THEN '' ELSE EmrDataT.Data END ELSE '' END) ",
									eistGenericTable, m_nEMRDetailID, m_nEMRInfoID);

								pflds = prsContent->GetFields();
								fldID = pflds->GetItem("ID");
								fldData = pflds->GetItem("Data");
								fldLabel = pflds->GetItem("IsLabel");	
								//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
								fldGlassesOrderDataType = pflds->GetItem("GlassesOrderDataType");
								fldGlassesOrderDataID = pflds->GetItem("GlassesOrderDataID");
								// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
								fldWoundCareDataType = pflds->GetItem("WoundCareDataType");
								if(prsContent->eof)
									break;

								nListType = AdoFldLong(pflds, "ListType", -1);
								// (z.manning 2010-02-16 14:42) - PLID 37230 - ListSubType
								nListSubType = AdoFldByte(pflds, "ListSubType", lstDefault);
								bIsGrouped = AdoFldBool(pflds, "IsGrouped", FALSE);
								// (a.walling 2013-03-21 10:14) - PLID 55804 - No more EM coding for data items necessary
								strFormula = AdoFldString(pflds, "Formula", "");
								nDecimalPlaces = AdoFldByte(pflds, "DecimalPlaces", 2);
								strInputMask = AdoFldString(pflds, "InputMask", "");
								nGroupID = AdoFldLong(pflds, "EmrDataGroupID", -1);
								// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
								bAutoAlphabetizeDropdown = AdoFldBool(pflds, "AutoAlphabetizeDropDown", FALSE);
								// (z.manning 2010-07-29 15:12) - PLID 36150
								strDataLongForm = AdoFldString(pflds, "DataLongForm", "");
								// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
								etantAutoNumberType = (EEmrTableAutoNumberType)AdoFldByte(pflds, "AutoNumberType", (BYTE)etantPerRow);
								strAutoNumberPrefix = AdoFldString(pflds, "AutoNumberPrefix", "");
								bHasDropdownElements = AdoFldBool(pflds, "HasDropdownElements", TRUE);
								bHasActiveDropdownElements = AdoFldBool(pflds, "HasActiveDropdownElements", TRUE);
								nAutofillType = AdoFldByte(pflds, "AutofillType", etatNone);
								nFlags = AdoFldLong(pflds, "DataFlags", 0);
								strDropdownSeparator = AdoFldString(pflds, "DropdownSeparator", ", ");
								strDropdownSeparatorFinal = AdoFldString(pflds, "DropdownSeparatorFinal", ", ");
								strSpawnedItemsSeparator = AdoFldString(pflds, "SpawnedItemsSeparator", ", ");
							}

							if(nListType == 2) { //Row
								// (z.manning 2010-02-19 15:57) - PLID 37412 - Smart stamp tables load their rows elsewhere
								// (j.jones 2010-02-25 10:22) - PLID 37538 - user-defined rows are allowed
								//TES 3/17/2010 - PLID 37530 - Need to specify the stamp ID and index
								//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
								
								// (a.walling 2011-08-11 15:49) - PLID 44987 - Insert if new, update strName if existing
								TableRowID rowID(AdoFldLong(fldID),-1,NULL,-1,-1);

								std::pair<TableRowMap::iterator, bool> insertionPoint = mapTableRows.insert(TableRowMap::value_type(rowID, NULL));
								if (insertionPoint.second) {
									// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
									insertionPoint.first->second = AddRow(rowID, strData, strFormula, nDecimalPlaces, nGroupID, AdoFldBool(fldLabel), (GlassesOrderDataType)AdoFldLong(fldGlassesOrderDataType,(long)godtInvalid), AdoFldLong(fldGlassesOrderDataID,-1), nFlags, aroSequential, -1, ((IsCurrentMedicationsTable() || IsAllergiesTable()) ? TRUE : FALSE));
								} else {
									insertionPoint.first->second->strName = strData;
								}								
							}
							else if(nListType >= 3) { //Column
								//(e.lally 2011-12-08) PLID 46471 - We assume the first column of a current medication or allergy table detail to be the official usage checkbox column
								BOOL bIsCurrentMedOrAllergyUsageCol = (m_arTableColumns.GetSize()==0 && (IsCurrentMedicationsTable() || IsAllergiesTable()));
								if(bIsCurrentMedOrAllergyUsageCol){
									//(e.lally 2011-12-08) PLID 46471 - This had better be true or our assumption about which column to use for allergies and current medications is wrong
									ASSERT(nListType == LIST_TYPE_CHECKBOX);
								}

								//DRT 7/10/2007 - PLID 24105 - No longer requires a width
								// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
								//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
								// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
								// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
								AddColumn(AdoFldLong(fldID), strData, nListType, bIsGrouped, strFormula, nDecimalPlaces, strInputMask, nListSubType, bAutoAlphabetizeDropdown, AdoFldBool(fldLabel), strDataLongForm, etantAutoNumberType, strAutoNumberPrefix, bHasDropdownElements, bHasActiveDropdownElements, (GlassesOrderDataType)AdoFldLong(fldGlassesOrderDataType,(long)godtInvalid), AdoFldLong(fldGlassesOrderDataID,-1), nAutofillType, nFlags, strDropdownSeparator, strDropdownSeparatorFinal, strSpawnedItemsSeparator, bIsCurrentMedOrAllergyUsageCol, (EWoundCareDataType)AdoFldLong(fldWoundCareDataType, (long)wcdtNone));
							}
						}
						break;
					case 6: //narrative
						// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
						//if(m_pParentTopic) m_pParentTopic->GetParentEMN()->LoadNarrativeMergeFields(this);
						break;
					case 5: //slider
						m_dSliderMin = AdoFldDouble(pflds, "SliderMin", 0.0);
						m_dSliderMax = AdoFldDouble(pflds, "SliderMax", 10.0);
						m_dSliderInc = AdoFldDouble(pflds, "SliderInc", 1.0);
						//correct bad data
						if(m_dSliderInc <= 0.0)
							m_dSliderInc = 1.0;
						break;
					case 4: //image
						{
						//
						// (c.haag 2006-11-10 09:36) - PLID 23365 - Whenever we pull BackgroundImageType, we could
						// get a null value. If so, it means we pulled it from an EmrInfo item where the user must
						// choose the image to assign to the detail. In that event, we need to assign "blank default
						// values" to the image state. In our program, this means clearing the path and assigning the
						// image type of itDiagram.
						//
						// On a side note, I don't think this will ever be -1, but I'm changing the code for completeness
						//
						// (c.haag 2007-02-09 15:49) - PLID 23365 - The previous comment is wrong. The simple story
						// is that EmrInfoT.BackgroundImageType is NULL if there is no default image for the info
						// item. When the detail is added, it should have no image unless the detail itself is assigned
						// one in its InkImage*Override fields. The user will have to pick an image.
						//
						if (itUndefined == (m_eitBackgroundImageType = (eImageType)AdoFldLong(pflds, "BackgroundImageType", -1))) {
							m_strBackgroundImageFilePath.Empty();
						} else {
							m_strBackgroundImageFilePath = AdoFldString(pflds, "BackgroundImageFilePath", "");
						}
						m_aisState->CreateFromSafeArrayVariant(m_varState);

						//DRT 1/22/2008 - PLID 28603 - This section is for loading content when we are not using
						//	the loader object or the spawner object.  This happens in cases like manually adding
						//	details to an EMN.
						_variant_t varID = prsContent->Fields->Item["HotSpotID"]->Value;
						if(varID.vt != VT_NULL) {
							//Add this hotspot
							CEMRHotSpot *pSpot = new CEMRHotSpot;
							pSpot->SetID(VarLong(varID));
							pSpot->SetGroupID(AdoFldLong(prsContent, "HotSpotGroupID"));
							pSpot->LoadData(AdoFldString(prsContent, "HotSpotData", ""));
							//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for hotspots
							pSpot->SetOriginalAnatomicLocation(AdoFldLong(prsContent, "HotSpotLocationID", -1), AdoFldString(prsContent, "HotSpotLocation",""));
							pSpot->SetOriginalAnatomicQualifier(AdoFldLong(prsContent, "HotSpotQualifierID", -1), AdoFldString(prsContent, "HotSpotQualifier",""));
							pSpot->SetOriginalSide((AnatomySide)AdoFldLong(prsContent, "HotSpotSide"));
							pSpot->Set3DHotSpotID(AdoFldShort(prsContent, "ImageHotSpotID", -1)); // (z.manning 2011-07-25 13:01) - PLID 44649

							m_aryImageHotSpots.Add(pSpot);
						}

						// (c.haag 2007-02-22 11:11) - PLID 24701 - If the following conditional is true,
						// it means that the EmrInfo item and the detail have differing background picture
						// paths. This can happen if an image detail is edited on the fly, and the info item's
						// background picture is changed.
						//
						// If this happens on a patient chart, this would be perfectly normal. Patient image
						// details should retain their state and background path because, as a general rule,
						// the detail state should try to retain as much data as possible if the info item
						// changes.
						//
						// Template details don't behave the same way. You cannot choose the image on a template
						// detail like you can on a patient chart. The template detail image background path must
						// always match that of the info item. If they do not match, we need to update the template
						// detail state with the new image path, and wipe out any ink in the process too.
						//
						// In general, we should strive to avoid changing m_varState here, but we must do it to
						// ensure the detail has the info item's image path, and to make sure that when the template
						// is saved, that we don't carry the old, wrong image path to data.
						//
						if (ComparePaths(m_strBackgroundImageFilePath, m_aisState->m_strImagePathOverride)) {
							// If this is a template, reload the state from data
							if (m_bIsTemplateDetail) {
								// (j.jones 2007-08-01 14:42) - PLID 26905 - we have no eligible active recordset to send
								m_varState = LoadEMRDetailStateDefault(NULL, m_nEMRInfoID, -1, -1, -1, lpCon, m_nRememberedDetailID); 
								m_aisState->CreateFromSafeArrayVariant(m_varState);
							}
						}
						}
						break;
					case 2: //single-select list
					case 3: //multi-select list
						//TES 10/14/2004: If there are no items in the list, don't add anything.
						if(fldID->GetValue().vt != VT_NULL &&
							(nListType == 1) // (c.haag 2006-03-17 09:14) - PLID 19557 - If we converted from a table to a multi-select list, we need to ignore legacy table columns
							)
						{
							//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
							// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
							AddListElement(AdoFldLong(fldID), AdoFldLong(fldDataGroupID), AdoFldString(fldData), AdoFldBool(fldLabel, FALSE), AdoFldString(prsContent, "DataLongForm",""), AdoFldLong(fldActionsType, -1), (GlassesOrderDataType)AdoFldLong(fldGlassesOrderDataType,(long)godtInvalid), AdoFldLong(fldGlassesOrderDataID,-1), AdoFldBool(fldIsFloated, FALSE), AdoFldString(fldSpawnedItemsSeparator,", "), AdoFldLong(fldParentLabelID, -1));
						}
						break;
					}
					prsContent->MoveNext();
				}
				prsContent->Close();
			}

			// (c.haag 2011-05-31) - PLID 43875 - At this point in time, all the tables rows have been added except
			// for inactive current medication or allergy items. These need to be loaded here because they were omitted
			// from previous data sources. The process is:
			//
			// 1) Find the selected rows that do not exist in the current content
			// 2) Load the content of the inactive rows into the table memory in the proper ordering
			//
			if ((IsCurrentMedicationsTable() || IsAllergiesTable()) 
				&& !m_bIsTemplateDetail
				&& GetStateVarType() == VT_BSTR
				&& VarString(GetState()).GetLength() > 0)
			{
				// 1) Find the selected rows that do not exist in the current content. This is tricky because we're comparing
				// old state values with new content; we will need to match on EmrDataGroupID.
				CArray<int,int> anOldEmrDataIDs;
				{
					// First, Populate anEmrDataIDs with the old state values
					CString strState = VarString(GetState());
					CEmrTableStateIterator etsi(strState);
					long X,Y,nEmrDetailImageStampID,nEmrDetailImageStampPointer,nStampID;
					CString strData;
					while(etsi.ReadNextElement(X,Y,strData,nEmrDetailImageStampID,nEmrDetailImageStampPointer,nStampID)) 
					{
						anOldEmrDataIDs.Add(X);
					}
				}

				// 2) Load the content of the inactive rows into the table memory in the proper ordering. To do this, I copied
				// an earlier query from this same function to get all the pertinent fields and tables; but I changed the EmrDataT
				// filter to only include inactive Emr Data records that are up-to-date versions of the ones assigned to the state.
				// We also include EmrDataT.SortOrder for proper sorted insertion where necessary.
				_RecordsetPtr prs = CreateParamRecordset(pCon,
					"SELECT AutoAlphabetizeListData FROM EmrInfoT WHERE ID = {INT}\r\n "
					"SELECT EmrDataT.ID, EmrDataT.Data, EmrDataT.IsLabel, EmrDataT.Formula, EmrDataT.DecimalPlaces, EmrInfoT.DataType, EmrInfoT.DataSubType, EmrDataT.DataFlags, "
					"EmrDataT.SortOrder, EmrDataT.EmrDataGroupID, "
					// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
					"EmrDataT.GlassesOrderDataType, EmrDataT.GlassesOrderDataID, "
					// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
					"EmrDataT.WoundCareDataType "
					"FROM EMRInfoT "
					"LEFT JOIN EMRDataT ON EMRInfoT.ID = EMRDataT.EMRInfoID "
					// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
					"WHERE EMRDataT.ListType = 2 AND EMRDataT.ID IN ("
					"	SELECT NewData.ID "
					"	FROM EmrDataT \r\n"
					"	INNER JOIN EmrDataT NewData ON NewData.EmrDataGroupID = EmrDataT.EmrDataGroupID AND NewData.EmrInfoID = {INT} \r\n"
					"	WHERE EmrDataT.ID IN ({INTARRAY}) AND NewData.Inactive = 1 \r\n"
					") "
					,m_nEMRInfoID
					,m_nEMRInfoID
					,anOldEmrDataIDs);
				// Fetch the auto-alphabetize flag
				BOOL bAutoAlphabetizeListData = AdoFldBool(prs->Fields, "AutoAlphabetizeListData");
				// Now read the actual inactive EmrData records
				prs = prs->NextRecordset(NULL);
				if (!prs->eof)
				{
					// If we're not auto-alphabetizing data, we need to insert values by sort order. Here's the problem: Since any 
					// number of table rows can be inactive, we cannot assume anything about the sort orders of the active rows
					// loaded into memory. We're going to have to load all of them right here into anSortedDataIDs.
					//
					// When done, anSortedDataIDs will effectively be a "map" where the key is SortOrder-1 and value is the EmrDataID.
					//
					// We can use that "map" when inserting inactive rows.
					//
					typedef struct {
						long nDataID;
						long nSortOrder;
					} EmrSortListItem;
					CArray<EmrSortListItem,EmrSortListItem&> aSortList;
					if (!bAutoAlphabetizeListData) 
					{
						_RecordsetPtr prsSort = CreateParamRecordset(pCon, "SELECT ID, SortOrder FROM EmrDataT WHERE ListType = 2 AND EmrInfoID = {INT} ORDER BY SortOrder", m_nEMRInfoID);
						while (!prsSort->eof)
						{
							EmrSortListItem item;
							item.nDataID = AdoFldLong(prsSort->Fields, "ID");
							item.nSortOrder = AdoFldLong(prsSort->Fields, "SortOrder");
							aSortList.Add(item);
							prsSort->MoveNext();
						}
					} // if (!bAutoAlphabetizeListData) 

					FieldsPtr pflds = prs->Fields;
					while (!prs->eof)
					{
						FieldPtr fldID = pflds->GetItem("ID");
						FieldPtr fldLabel = pflds->GetItem("IsLabel");
						FieldPtr fldGlassesOrderDataType = pflds->GetItem("GlassesOrderDataType");
						FieldPtr fldGlassesOrderDataID = pflds->GetItem("GlassesOrderDataID");
						// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
						FieldPtr fldWoundCareDataType = pflds->GetItem("WoundCareDataType");
						CString strData = AdoFldString(pflds->GetItem("Data"),"");
						// (a.walling 2013-03-21 10:14) - PLID 55804 - No more EM coding for data items necessary
						CString strFormula = AdoFldString(pflds, "Formula", "");
						BYTE nDecimalPlaces = AdoFldByte(pflds, "DecimalPlaces", 2);
						long nGroupID = AdoFldLong(pflds, "EmrDataGroupID", -1);
						long nFlags = AdoFldLong(pflds, "DataFlags", 0);
						long nSortOrder = AdoFldLong(pflds, "SortOrder");
						long nEmrDataID = AdoFldLong(fldID);

						long nPlacement = -1;
						if (!bAutoAlphabetizeListData) {
							// If we're not auto-alphabetizing data, we need to insert the row at the proper place. We know
							// where this row goes in a list where all the rows are active. We need to find the ordinal in
							// m_aryTableRows such that it's the first row to come after the inactive one we're adding.
							int n;
							// Start by seeking to where the current inactive row is in aSortList
							for (n=0; n < aSortList.GetSize() && aSortList[n].nDataID != nEmrDataID; n++);
							if (n == aSortList.GetSize()) {
								ThrowNxException("Failed to find EmrDataID %d in aSortList", nEmrDataID);
							}
							// Now starting from the first row after n, find the first active row.
							for (++n; n < aSortList.GetSize() && -1 == nPlacement; n++) 
							{
								// (a.walling 2011-08-11 15:49) - PLID 44987 - Faster check for existence
								TableRowID tr(aSortList[n].nDataID,-1,NULL,-1,-1);
								TableRowMap::iterator it = mapTableRows.find(tr);

								if (it != mapTableRows.end()) {
									for(int j = 0; j < m_arTableRows.GetSize() && -1 == nPlacement; j++) {
										if(m_arTableRows[j] == it->second) {
											// Success, we found the first active row that comes after the inactive one we're adding.
											// Since InsertAt(j) bumps the existing element down one, we can just plug j into nPlacement
											// and use it in AddRow.
											nPlacement = j;
										}
									}									
								}
							}
						}

						// (a.walling 2011-08-11 15:49) - PLID 44987 - Insert if new, update strName if existing
						TableRowID rowID(AdoFldLong(fldID),-1,NULL,-1,-1);

						//(e.lally 2011-12-08) PLID 46471 - Specify if this is for a current medications or allergies detail

						std::pair<TableRowMap::iterator, bool> insertionPoint = mapTableRows.insert(TableRowMap::value_type(rowID, NULL));
						if (insertionPoint.second) {
							// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
							insertionPoint.first->second = AddRow(rowID, strData, strFormula, nDecimalPlaces, nGroupID, AdoFldBool(fldLabel), (GlassesOrderDataType)AdoFldLong(fldGlassesOrderDataType, (long)godtInvalid), AdoFldLong(fldGlassesOrderDataID, -1), nFlags, (bAutoAlphabetizeListData) ? aroAlphabetically : aroInsertAt, nPlacement, ((IsCurrentMedicationsTable() || IsAllergiesTable()) ? TRUE : FALSE));
						} else {
							insertionPoint.first->second->strName = strData;
						}						
						prs->MoveNext();
					} // while (!prs->eof)
				} // if (!prs->eof)
			}

			// (j.jones 2010-02-17 15:32) - PLID 37318 - if the image loaded before the table,
			// they may not have been linked yet, so see if this needs to be linked to an image
			if(m_EMRInfoType == eitTable && !IsSmartStampTable() && m_pParentTopic != NULL && m_pParentTopic->GetParentEMN() != NULL) {
				m_pParentTopic->GetParentEMN()->EnsureSmartStampLinks(this);
			}

			// (z.manning 2010-02-19 13:48) - PLID 37412 - Smart stamp tables do not load their row
			// content from EmrDataT, but instead from the detail stamps of its linked image.
			// (j.jones 2010-02-25 10:54) - PLID 37538 - Row content is now loaded from EMRDataT,
			// the SmartStamp rows need added afterwards
			if(IsSmartStampTable()) {
				// (z.manning 2011-02-14 09:53) - PLID 42446 - Moved this logic to its own function
				LoadSmartStampTableContent(pEMNLoader, lpCon);
			}

			EndAddListElements();

			// (c.haag 2011-03-18) - PLID 42895 - Load common list items. For optimizational purposes, only
			// do it if this is a current medication or allergy table
			if (IsCurrentMedicationsTable() || IsAllergiesTable())
			{
				CEmrInfoCommonListCollection* pCollection = NULL;
				if (NULL != pEMNLoader)
				{
					pCollection = pEMNLoader->GetEmrInfoCommonLists(m_nEMRInfoID);
				}
				if (NULL == pCollection) 
				{
					// No collection available. Load the lists from data.
					_ConnectionPtr pCon = lpCon;
					if (NULL == pCon) { pCon = GetRemoteData(); }
					m_CommonLists.Load(pCon, m_nEMRInfoID);
				}
				else {
					m_CommonLists = *pCollection;
				}
			}
		}
		// (c.haag 2007-07-03 10:38) - PLID 26523 - Ensure we release our lock on the
		// EMR info data of the CEMNLoader

	} // if (m_nEMRInfoID != -1) {

	//Our cached sentence format is now invalid.
	m_strSentenceHTML = m_strSentenceNonHTML = "";

	// (c.haag 2006-03-31 11:34) - PLID 19387 - If necessary, synchronize the
	// content with the state. Notice that we call this before m_pLastSavedDetail
	// is modified.
	if (m_bNeedToSyncContentAndState) {
		SyncContentAndState();
		m_bNeedToSyncContentAndState = FALSE;
	}

	RefreshSmartStampTableBuiltInData(TRUE);

	// (z.manning 2012-03-28 11:51) - PLID 33710 - If this is not the intial detail load then we need recalculate
	// any table formulas in case they changed in the editor.
	// (z.manning 2012-07-16 16:41) - PLID 33710 - Also do this for newly created details
	if(bIsForcedReload || IsNew()) {
		UpdateTableCalculatedFields();
	}

	// (j.jones 2006-08-28 13:50) - PLID 22237 - used the m_bContentHasBeenLoaded variable to determine:
	// - if the content had not previously been loaded, be sure to initialize m_pLastSavedDetail
	// - if the content has been loaded, then set the status as unsaved because it would imply that we modified the content
	// Both of these uses are to more firmly support Tom's PLID 21000 assumptions.

	//TES 6/12/2006 - PLID 21000 - Only set the detail if we are loading for the first time; if we already have m_pLastSavedDetail, it means
	//that it's already been loaded, so don't overwrite it (maybe we're just refreshing the content).
	if(!m_bContentHasBeenLoaded) {
	
		if(!m_oLastSavedDetail) {
			// (c.haag 2007-07-17 13:25) - PLID 26651 - The act of creating m_pLastSavedDetail
			// is now self-contained
			m_oLastSavedDetail = CreateLastSavedDetail();
		}		
	}
	
	if(m_bContentHasBeenLoaded){
		//TES 6/12/2006 - PLID 21000 - If we have reloaded our content, we probably have been modified.
		SetUnsaved();
		CWnd *pWnd = m_pParentTopic->GetParentEMN()->GetInterface();
		if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
			pWnd->PostMessage(NXM_TOPIC_MODIFIED_CHANGED, (WPARAM)m_pParentTopic);
		}
	}

	m_bContentHasBeenLoaded = TRUE;
}

void CEMNDetail::ReflectCurrentContent()
{
	// (c.haag 2006-03-31 09:49) - This function causes the form to
	// reflect our content. By "content", we are referring to the member
	// data of this detail that describes how the form should look. This
	// will never reference m_varState. If you want to reference m_varState,
	// then use ReflectCurrentState. This will instead reference
	// m_arListElements, m_dSliderMin, etc., and other non-m_varState variables
	// for complex forms like tables, sliders, lists, and narratives.
	//
	// Upon completion of the function, the form on the screen will reflect
	// that of the current "content" of this detail.
	//
	// To truly have a form reflect a detail's value, we need to call both
	// ReflectCurrentContent and ReflectCurrentState.
	//
	//TES 6/3/2008 - PLID 29098 - We need to pass this to the popped up dialog (if any) as well.
	if (m_pPopupDlg && IsWindow(m_pPopupDlg->GetSafeHwnd())) {
		m_pPopupDlg->ReflectCurrentContent();
	}
	if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
		m_pEmrItemAdvDlg->ReflectCurrentContent();
	}
}

void CEMNDetail::ReflectCurrentState()
{
	// (c.haag 2006-03-31 09:55) - This function causes the form to
	// reflect our state. By "state", we mean m_varState. While m_varState
	// does not contain data about the names of rows and columns in tables
	// and lists, or the boundaries of the slider, it does contain data
	// about the stored values within the confines of the content, like what
	// data each table cell points to, and where the slider points to, and
	// what text is in a textbox.
	//
	// To truly have a form reflect a detail's value, we need to call both
	// ReflectCurrentContent and ReflectCurrentState.
	//
	// (a.walling 2007-08-22 16:54) - PLID 27160 - This only happens for narratives now, but when
	// there are two copies of the narrative (one on the EMN, one popped up), ensure both are updated.
	//TES 6/3/2008 - PLID 29098 - Always do popups first, since they are what the user is currently interacting with.
	if (m_pPopupDlg && IsWindow(m_pPopupDlg->GetSafeHwnd())) {
		m_pPopupDlg->ReflectCurrentState();
	}
	if (m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
		m_pEmrItemAdvDlg->ReflectCurrentState();
	}
}

BOOL CEMNDetail::RequestStateChange(const _variant_t &varNewState)
{
	return RequestStateChange(varNewState, m_varState);
}

// (z.manning 2009-02-16 17:46) - PLID 33072 - Added an overload paramater for the old state
// since tables states have already been recreated by the time this is called
BOOL CEMNDetail::RequestStateChange(const _variant_t &varNewState, const _variant_t &varOldState)
{
	// (r.gonet 06/10/2011) - PLID 30359 - Some event handling needs to know we are changing the state in order to not clobber it.
	CCriticalFlag cfChangingState(m_bStateChanging);

	// (z.manning 2011-04-12 17:33) - PLID 43261 - This function very much depends on the content already having
	// been loaded so call LoadContent here. If content has already been loaded for this detail then this won't
	// do anything.
	LoadContent();

	CEmrItemValueChangedInfo eivci;
	eivci.varOldState = varOldState;
	eivci.varNewState = varNewState;
	eivci.bDeleted = FALSE; // (a.walling 2007-07-11 15:24) - PLID 26261 - We are not deleting the detail

	// (a.walling 2007-12-14 14:50) - PLID 28354
	CArray<NarrativeField> arOriginalNarrativeFields;
	CArray<CEMNDetail*, CEMNDetail*> arOriginalLinked;
	if (m_EMRInfoType == eitNarrative && m_pParentTopic && m_pParentTopic->GetParentEMN()) {
		CString strNxRichText = VarString(m_varState);
		strNxRichText.TrimRight();
		//TES 6/29/2012 - PLID 50854 - New function that handles HTML narratives
		GetNarrativeFieldArray(strNxRichText, arOriginalNarrativeFields);
		GetLinkedDetails(arOriginalLinked, m_pParentTopic->GetParentEMN(), NULL, NULL, TRUE);
	}

	// (c.haag 2007-05-23 09:49) - PLID 26108 - If this is a list detail, and a user is checking
	// a box, then new EMN details could be spawned. Should that be the case, the user may see
	// one or more details popped up individually depending on the pop-up style of the details.
	//
	// However, if we change the detail state while the initial load is still running, then the 
	// details will be spawned, but the popups will not appear. The reason is that the preloader
	// will spawn those details for us since it sees that the state has been set to something
	// favorable for spawning. Later on, ProcessEmrActions sees that the details were already
	// spawned; therefore, it won't try to pop the details up.
	//
	// To prevent this, we must force the preload to finish BEFORE we actually set the state.
	// That way, the preloader won't spawn the details; rather, ProcessEmrActions will later on.
	// In the process, the preloader will pop up details that have the pop-up flag set.
	//
	if (m_EMRInfoType == eitSingleList || m_EMRInfoType == eitMultiList) { // Must be a list detail
		if(m_pParentTopic) { // Must have a parent topic
			CEMN* pEMN = m_pParentTopic->GetParentEMN();
			if (NULL != pEMN) {
				pEMN->EnsureCompletelyLoaded();
			}
		}
	}

	//TES 8/8/2006 - PLID 21709 - Do this before spawning actions, that way if anything is trying to auto-advance topics, it will
	//have an accurate understanding of whether this detail is completed.
	//now set the detail
	SetState(varNewState);

	// (a.walling 2007-03-15 09:23) - PLID 25212 - We've set the state, let's ensure that the content matches.
	// This was causing issues when processing the actions below with the preference to automatically move to
	// the next topic and the preference to save as you leave a topic. The saving code would be called within
	// our ProcessEMRDataActions function, and although we have a correct state, our content (member variables)
	// are still out of date. So when GenerateSaveString looks for selections, it may not find them. Then, if
	// we are saved again, the selections havn't changed, so both ways the selections may not be written to data.
	SetUnsaved();

	SyncContentAndState();

	if(m_EMRInfoType == eitTable) {
		// (z.manning 2012-03-27 17:46) - PLID 33710 - Calculated table cells are now saved to data. So the only time we
		// calculate them now is when something in the table changed, which means this (RequestStateChange) should be
		// the only place we ever need to call this.
		UpdateTableCalculatedFields();
	}

	//Handle the spawning.
	// The only ones that can have actions right now are the single- and multi-sel lists, and the 
	// actions are based on the newly selected DataID.
	if(m_pParentTopic) {
		if (m_EMRInfoType == eitSingleList || m_EMRInfoType == eitMultiList) {
			// (c.haag 2007-09-10 16:03) - PLID 26108 - Revoke and process data actions where necessary. This code actually
			// existed back in May 2007, but I took it out. The reason for reintroducing it was because my "new" version
			// of this branch of code was supposed to be an extra optimization, but instead, it broke the spawn ordering.
			// Reverting back to this code also appears to not be a major speed hit; and if it were, it's something we
			// would have to address in the next scope.
			CArray<long, long> aryNewlySelDataID, aryNewlyUnselDataID;

			CalcChangedDataIDFromState(eivci.varOldState, eivci.varNewState, aryNewlySelDataID, aryNewlyUnselDataID);
			
			if(aryNewlyUnselDataID.GetSize() > 0) {
				//At this point, we've got some things that need to be unselected.  We want to generate a list of all the
				//	ListElement objects, and pass those in to our CEMNUnspawner utility.
				CArray<ListElement, ListElement&> aryListElements;
				for(int i = 0; i < aryNewlyUnselDataID.GetSize(); i++) {
					long nDataID = aryNewlyUnselDataID.GetAt(i);

					//Search all list elements for a match
					for(int nListIdx = 0; nListIdx < GetListElementCount(); nListIdx++) {
						ListElement le = GetListElement(nListIdx);
						if(nDataID == le.nID) {
							aryListElements.Add(le);
						}
					}
				}

				//Now we've got a list of all the list elements to be removed.
				CEMNUnspawner eu(m_pParentTopic->GetParentEMN());
				eu.RemoveActionsByDataElements(&aryListElements, this);
			}

			// (j.jones 2013-01-09 16:44) - PLID 54541 - we now support processing the actions for
			// all the selected data IDs at once
			if(aryNewlySelDataID.GetSize() > 0) {
				m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEMRDataActions(aryNewlySelDataID, this, FALSE);
			}
		}
		// (z.manning, 01/22/2008) - PLID 28690 - Images are now joining in the spawning fun.
		else if(m_EMRInfoType == eitImage)
		{
			// (z.manning 2011-01-28 11:27) - PLID 42335 - Moved the image logic to its own function
			HandleImageStateChange(eivci.varNewState, eivci.varOldState, TRUE);
		}
		// (z.manning 2009-02-16 14:34) - PLID 33072 - Table dropdown item based spawning
		else if(m_EMRInfoType == eitTable)
		{
			// (z.manning 2009-03-04 11:13) - PLID 33072 - First get strings for the old and new states.
			// Note: The table's new state was almost certainly set before this function was called
			// so it's necessary that the original state be passed into this function.
			CString strOldState;
			if(eivci.varOldState.vt != VT_EMPTY) {
				strOldState = VarString(eivci.varOldState, "");
			}
			CString strNewState;
			if(eivci.varNewState.vt != VT_EMPTY) {
				strNewState = VarString(eivci.varNewState, "");
			}

			BOOL bTableContentChanged = FALSE;

			// (z.manning 2009-03-04 11:15) - PLID 33072 - Iterate through the old table state and setup
			// a map of table elements to their pre state change data.
			CEmrTableStateIterator etsiOld(strOldState), etsiNew(strNewState);
			long nRow, nCol, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID;
			CString strData;
			CMap<__int64,__int64,CString,CString&> mapElementKeyToOldData;
			while(etsiOld.ReadNextElement(nRow, nCol, strData, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID))
			{
				TableColumn *ptc = GetColumnByID(nCol);
				if(ptc != NULL) {
					if(ptc->nType == LIST_TYPE_DROPDOWN) {
						//TES 3/17/2010 - PLID 37530 - Pass in -1 for the SourceStampID and SourceStampIndex
						TableRow *ptr = GetRowByID(&TableRowID(nRow, nEmrDetailImageStampID, nEmrDetailImageStampPointer, -1,-1));
						if(ptr != NULL) {
							mapElementKeyToOldData.SetAt(GetTableElementKey(ptr, ptc), strData);
						}
					}
				}
				else {
					// (z.manning 2009-04-14 13:27) - PLID 33072 - If we get here then the current detail
					// can't find a column for this ID. This had better mean that either the column has
					// been deleted or inactivated.
					#ifdef _DEBUG
						_RecordsetPtr prs = CreateParamRecordset(
							"SELECT Inactive FROM EmrDataT WHERE ID = {INT}", nCol);
						if(!prs->eof) {
							ASSERT(AdoFldBool(prs->GetFields(), "Inactive"));
						}
					#endif
				}
			}

			// (z.manning 2009-03-04 11:16) - PLID 33072 - Now iterate through the new state
			CArray<TableSpawnInfo,TableSpawnInfo&> aryNewlySelectedDropdowns, aryNewlyUnselectedDropdowns;
			while(etsiNew.ReadNextElement(nRow, nCol, strData, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID))
			{
				// (z.manning 2009-03-04 11:16) - PLID 33072 - For table dropdown spawning we need to keep
				// track of what row spawned the object since the exact same actions and source details
				// existing for each row of the table.
				//TES 3/17/2010 - PLID 37530 - Need to specify the stamp ID and index
				// (z.manning 2011-01-20 12:08) - PLID 42338 - Support multiple images per smart stamp table
				long nStampIndexInDetailByType = (nEmrDetailImageStampPointer != 0) ? m_arySmartStampImageDetails.GetStampIndexInDetailByType((EmrDetailImageStamp*)nEmrDetailImageStampPointer) : -1;
				TableRowID rowid(nRow,nEmrDetailImageStampID,nEmrDetailImageStampPointer, (nEmrDetailImageStampPointer?((EmrDetailImageStamp*)nEmrDetailImageStampPointer)->nStampID:-1), nStampIndexInDetailByType);
				TableRow *ptr = GetRowByID(&rowid);
				TableSpawnInfo tdsi;
				if(ptr != NULL) {
					tdsi.tr = *ptr;
				}
				else {
					ASSERT(FALSE);
				}

				TableColumn *ptc = GetColumnByID(nCol);
				if(ptc != NULL) {
					if(ptc->nType == LIST_TYPE_DROPDOWN) {
						// (z.manning 2009-03-04 11:19) - PLID 33072 - This cell is a dropdown so let's see
						// if it changed from the old state.
						CString strOldData;
						__int64 nElementKey = GetTableElementKey(ptr, ptc);
						CArray<long,long> arynTempNewlySelected, arynTempNewlyUnselected;
						if(mapElementKeyToOldData.Lookup(nElementKey, strOldData)) {
							// (z.manning 2009-03-04 11:23) - PLID 33072 - We found data for this cell in
							// the old state. Let's compare the currently selected dropdown element against
							// what was previously selected.
							strOldData.Replace(',', ';');
							strData.Replace(',', ';');
							CalcChangedDataIDFromState(_bstr_t(strOldData), _bstr_t(strData), arynTempNewlySelected, arynTempNewlyUnselected);
							// (z.manning 2009-03-04 11:26) - PLID 33072 - We have handled this cell in the old
							// state so get rid of it so that we know later on which ones were not handled.
							BOOL bRemoved = mapElementKeyToOldData.RemoveKey(nElementKey);
							ASSERT(bRemoved);
						}
						else {
							// (z.manning 2009-03-04 11:25) - PLID 33072 - We did NOT find this data in the old
							// state which means that any selected dropdown IDs were newly selected.
							ParseDelimitedStringToLongArray(strData, ',', arynTempNewlySelected);
						}

						// (z.manning 2009-03-04 11:25) - PLID 33072 - Now add any applicable newly selected
						// or unselected dropdown elements to our overall array as we continue to iterate
						// through the table state.
						for(int nSelIndex = 0; nSelIndex < arynTempNewlySelected.GetSize(); nSelIndex++) {
							tdsi.nDropdownID = arynTempNewlySelected.GetAt(nSelIndex);
							if(tdsi.nDropdownID > 0) {
								aryNewlySelectedDropdowns.Add(tdsi);
							}
						}
						for(int nUnselIndex = 0; nUnselIndex < arynTempNewlyUnselected.GetSize(); nUnselIndex++) {
							tdsi.nDropdownID = arynTempNewlyUnselected.GetAt(nUnselIndex);
							if(tdsi.nDropdownID > 0) {
								aryNewlyUnselectedDropdowns.Add(tdsi);
							}
						}
					}
				}
			}

			// (z.manning 2009-03-04 11:26) - PLID 33072 - Now we need to check and see if we have any
			// unhadled element from the old state that weren't in the new state. If so, we presume those
			// to be unselected and thus must unspawn any actions.
			// Note: At this time I do not think it's possible for this to be true because even if you
			// unselect a table element that was previously selected, data from that cell will remain
			// in the state with a dropdown ID of zero. But I still put it here in case that ever changes.
			POSITION pos = mapElementKeyToOldData.GetStartPosition();
			while(pos != NULL)
			{
				__int64 nElementKey;
				mapElementKeyToOldData.GetNextAssoc(pos, nElementKey, strData);
				if(!strData.IsEmpty()) {
					CArray<long,long> arynTempNewlyUnselected;
					ParseDelimitedStringToLongArray(strData, ',', arynTempNewlyUnselected);
					TableRow *ptr = GetRowFromTableElementKey(nElementKey);
					TableSpawnInfo tdsi;
					if(ptr != NULL) {
						tdsi.tr = *ptr;
					}

					for(int nUnselIndex = 0; nUnselIndex < arynTempNewlyUnselected.GetSize(); nUnselIndex++) {
						tdsi.nDropdownID = arynTempNewlyUnselected.GetAt(nUnselIndex);
						if(tdsi.nDropdownID > 0) {
							aryNewlyUnselectedDropdowns.Add(tdsi);
						}
					}
				}
			}

			// (z.manning 2009-02-23 15:51) - PLID 33138 - Unspawning for table dropdown actions
			// (z.manning 2009-03-16 17:31) - Honor the m_bIgnoreActions flag.
			if(aryNewlyUnselectedDropdowns.GetSize() > 0 && !m_pParentTopic->GetParentEMN()->GetParentEMR()->m_bIgnoreActions) {
				CEMNUnspawner eu(m_pParentTopic->GetParentEMN());
				eu.RemoveActionsByTableDropdownItems(&aryNewlyUnselectedDropdowns, this);
			}

			// (z.manning 2010-03-18 10:22) - PLID 37571 - Did we unspawn ourself?
			BOOL bCurrentDetailDeleted = FALSE;
			// (z.manning 2011-03-02 17:18) - PLID 42335 - This should not check if the detail is simply pending deletion.
			if(m_pParentTopic != NULL && m_pParentTopic->IsDetailDeleted(this, FALSE)) {
				bCurrentDetailDeleted = TRUE;
			}

			// (z.manning 2010-03-18 10:47) - PLID 37571 - Don't spawn new actions if we just unspawned ourself
			if(!bCurrentDetailDeleted) {
				// (z.manning 2009-03-04 11:30) - PLID 33072 - Handle spawning for any newly selected dropdown items.
				for(int nDropdownIndex = 0; nDropdownIndex < aryNewlySelectedDropdowns.GetSize(); nDropdownIndex++) {
					TableSpawnInfo tdsi = aryNewlySelectedDropdowns.GetAt(nDropdownIndex);
					m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEmrTableDropdownItemActions(tdsi, this, FALSE);
				}
			}

			// (z.manning 2010-03-04 08:12) - PLID 37225 - Check and see if we have any table rows that need removed
			for(int nRowIndex = m_arTableRows.GetSize() - 1; nRowIndex >= 0; nRowIndex--) {
				TableRow *ptr = m_arTableRows.GetAt(nRowIndex);
				if(ptr->m_bDeleteOnStateChange) {
					RemoveTableRow(ptr);
					bTableContentChanged = TRUE;
				}
			}
			
			// (z.manning 2011-02-23 14:34) - PLID 42335 - We now renumber the auto-number column every time
			// a row is added or removed.
			// (z.manning 2011-05-11 10:58) - PLID 43568 - Only refresh content if the table content actually changed.
			RefreshSmartStampTableBuiltInData(bTableContentChanged);

			//TES 3/11/2010 - PLID 37535 - We're about to update any narratives pointing to us, so we need to make sure our cached
			// data output is up to date.
			UpdateTableDataOutput();
		}
	}


	// (c.haag 2007-01-23 11:13) - PLID 24376 - If this detail is a system Current Medications
	// table, we need to retroactively update every Current Medications detail in the entire
	// EMN with the same info ID to have the same data. This is because the Current Medications
	// detail is one-per-patient.
	if (IsCurrentMedicationsTable()) {
		m_pParentTopic->GetParentEMN()->ProcessCurrentMedicationsChange(this);
	}
	// (c.haag 2007-04-05 13:20) - PLID 25516 - If this is a system Allergies table detail, we
	// need to retroactively update every Allergies detail in the entire EMN with the same info ID
	// to have the same data.
	if (IsAllergiesTable()) {
		m_pParentTopic->GetParentEMN()->ProcessAllergiesChange(this);
	}

	/*
	// (a.walling 2007-03-15 09:23) - PLID 25212 - We should SyncContentAndState before processing actions. See above 25209 comment for details.
	SetUnsaved();

	SyncContentAndState();
	*/

	//Update the narratives.
	//TES 9/13/2006 - PLID 22044 - I don't know why this was passing TRUE here (thus forcing all narratives to be created), but
	// it shouldn't have been, and leaving it as the default (FALSE), doesn't seem to cause any problems I can find.
	//TES 1/23/2008 - PLID 24157 - Renamed.
	if(m_pParentTopic) m_pParentTopic->GetParentEMN()->HandleDetailChange(this);

	//Notify our parent.
	if(m_pParentTopic) {
		m_pParentTopic->HandleDetailStateChange(this);
	}

	// And changed or not, reflect the current state on screen	
	// (z.manning 2008-06-24 14:35) - PLID 30155 - I moved this to before posting messages about the 
	// state change to our parent because this is where EMR table calculated fields get updated and 
	// I want to make sure that is done before we update the preview pane, etc.
	ReflectCurrentState();

	//Notify our interface.
	// (a.walling 2007-04-13 09:16) - PLID 25623 - We still want to send STATECHANGED messages even if our topicwnd
	// does not exist yet, so try further up the hierarchy to get an interface wnd.
	//(e.lally 2012-03-15) PLID 48931 - The preview pane topic is not able to process the message if its parent topic is a hidden parent wnd.
	//	We should be able to start by notifying the parent EMN. The only time we are aware of something getting past the first check is when a popped up detail is processing
	//	an on click event, though we are not entirely sure why it is going through all the state change functions prior to the OK button anyway.
	if (m_pParentTopic && m_pParentTopic->GetParentEMN() && m_pParentTopic->GetParentEMN()->GetInterface() && IsWindow(m_pParentTopic->GetParentEMN()->GetInterface()->GetSafeHwnd()) ) {
		// send to the EMN's interface window.
		m_pParentTopic->GetParentEMN()->GetInterface()->SendMessage(NXM_EMR_ITEM_STATECHANGED, (WPARAM)this, (LPARAM)&eivci);
	}
	//else {

	//	if(m_pParent && IsWindow(m_pParent->GetSafeHwnd())) {
	//		m_pParent->SendMessage(NXM_EMR_ITEM_STATECHANGED, (WPARAM)this, (LPARAM)&eivci);
	//	} else if (m_pParentTopic && m_pParentTopic->GetInterface() && IsWindow(m_pParentTopic->GetInterface()->GetSafeHwnd()) ) {
	//		// this message is just passed to EMRTopicWnd's parent anyway, so let's do that if we can get the window
	//		m_pParentTopic->GetInterface()->SendMessage(NXM_EMR_ITEM_STATECHANGED, (WPARAM)this, (LPARAM)&eivci);
	//	}
	//}

	// (j.jones 2006-10-13 09:31) - PLID 22461 - we warn the user beforehand now, so we don't need to tell
	// them again after it has changed
	//m_pParentTopic->GetParentEMN()->CheckWarnOfficeVisitChanged();

	// (a.walling 2007-12-14 14:51) - PLID 28354
	CArray<NarrativeField> arNewNarrativeFields;
	if (m_EMRInfoType == eitNarrative && m_pParentTopic != NULL) {
		CString strNxRichText = VarString(m_varState);
		strNxRichText.TrimRight();
		//TES 6/29/2012 - PLID 50854 - New function that handles HTML narratives
		GetNarrativeFieldArray(strNxRichText, arNewNarrativeFields);
		
		BOOL bFieldsChanged = FALSE;

		// now compare the original and new fields
		if (arOriginalNarrativeFields.GetSize() != arNewNarrativeFields.GetSize()) {
			// optimize this case
			bFieldsChanged = TRUE;
		} else {
			// (c.haag 2008-11-24 11:23) - PLID 32170 - Upon further investigation, the entire previous loop is incorrect
			// because it does not verify that everything in arNewNarrativeFields is also in arOriginalNarrativeFields. Also
			// assuming both arrays are stored sorted, why would we even have to traverse beyond the first element? However,
			// since assumptions can be wrong, we will still traverse the arrays.
			CArray<NarrativeField> a1,a2;
			a1.Copy(arOriginalNarrativeFields);
			a2.Copy(arNewNarrativeFields);
			bFieldsChanged = FALSE;

			for (int i=0; i < a1.GetSize() && !bFieldsChanged; i++) {
				NarrativeField nf1 = a1[i];
				BOOL bFound = FALSE;
				for (int j = 0; j < a2.GetSize() && !bFound; j++) {
					NarrativeField nf2 = a2[j];
					if (nf1.strField == nf2.strField) {
						a1.RemoveAt(i--);
						a2.RemoveAt(j);
						bFound = TRUE;
					}
				}
				if (!bFound) {
					bFieldsChanged = TRUE;
				}
			}

			// If either array has any elements left, we know at least one array has an element
			// that the other does not. So, set the changed flag if that's the case.
			if (a1.GetSize() > 0 || a2.GetSize() > 0) {
				bFieldsChanged = TRUE;
			}
			/*
			// well, the array is sorted, so keep going until we get a mismatch
			for (int i = 0; i < arOriginalNarrativeFields.GetSize(); i++) {
				NarrativeField nfOrig = arOriginalNarrativeFields[i];

				BOOL bFound = FALSE;
				for (int j = 0; j < arNewNarrativeFields.GetSize(); j++) {
					// (c.haag 2008-11-24 11:07) - PLID 32170 - Get nfNew from arNewNarrativeFields, not arOriginalNarrativeFields
					NarrativeField nfNew = arNewNarrativeFields[j];
					if (nfNew.strField == nfOrig.strField) {
						bFound = TRUE;
						break;
					}
				}

				if (!bFound) {
					bFieldsChanged = TRUE;
					break;
				}
			}*/
		}

		if (bFieldsChanged) {
			InvalidateLinkedDetailCache();
			//m_bLinkedDetailsCached = false;

			CArray<CEMNDetail*, CEMNDetail*> arLinked;
			GetLinkedDetails(arLinked, m_pParentTopic->GetParentEMN(), NULL, NULL, TRUE);

			// merge all the details into one array so we can update them.
			for (int o = 0; o < arOriginalLinked.GetSize(); o++) {
				CEMNDetail* pOrigDetail = arOriginalLinked[o];

				BOOL bFound = FALSE;
				for (int p = 0; p < arLinked.GetSize(); p++) {
					if (pOrigDetail == arLinked[p]) {
						bFound = TRUE;
						break;
					}
				}

				if (!bFound) {
					arLinked.Add(pOrigDetail);
				}
			}

			CMap<CEMRTopic*, CEMRTopic*, BOOL, BOOL> mapTopics;

			for (int i = 0; i < arLinked.GetSize(); i++) {
				CEMNDetail* pDetail = arLinked[i];
				
				if (pDetail->m_pParentTopic && pDetail->m_pParentTopic->GetParentEMN() && pDetail->m_pParentTopic->GetParentEMN()->GetInterface()) {
					CEmrTreeWnd* pTreeWnd = pDetail->m_pParentTopic->GetParentEMN()->GetInterface();

					pTreeWnd->UpdateDetailPreview(pDetail);

					mapTopics[pDetail->m_pParentTopic] = TRUE;
				}
			}

			// (a.walling 2008-02-05 12:00) - PLID 28391 - Need to refresh the topics' visibilities after updating the detail
			POSITION pos = mapTopics.GetStartPosition();
			while (pos) {
				BOOL bDummy = FALSE;
				CEMRTopic* pAffectedTopic = NULL;
				mapTopics.GetNextAssoc(pos, pAffectedTopic, bDummy );

				if (pAffectedTopic) {
					pAffectedTopic->RefreshHTMLVisibility();
				}
			}
		}
	} else {
		//TES 7/26/2007 - PLID 25091 - They may have deleted a narrative merge field or selected a different linked detail
		// in a table, so we can't trust our cached array of details that we're linked to any more.
		//m_bLinkedDetailsCached = false;		
		InvalidateLinkedDetailCache();
	}

	// And return the result
	//return eivci.bAcceptChange;
	return TRUE;
}

// (z.manning 2011-02-02 12:35) - PLID 42335
void CEMNDetail::HandleImageStateChange(const _variant_t &varNewState, const _variant_t &varOldState, BOOL bFullStateChange)
{
	HandleImageStateChange(varNewState, varOldState, bFullStateChange, g_cvarNull, NULL);
}

// (z.manning 2011-01-28 11:25) - PLID 42335 - Moved the image state change logic to its own function
void CEMNDetail::HandleImageStateChange(const _variant_t &varNewState, const _variant_t &varOldState, BOOL bFullStateChange, const _variant_t &varRememberedTableState, OUT CArray<TableRow*,TableRow*> *parypNewTableRows)
{
	CEmrItemAdvImageState aisOld, aisNew;
	aisOld.CreateFromSafeArrayVariant(varOldState);
	aisNew.CreateFromSafeArrayVariant(varNewState);

	// (z.manning 2010-02-16 18:01) - PLID 37230 - Get the old and new text states
	CNxInkPictureText niptOld, niptNew;
	niptOld.LoadFromVariant(aisOld.m_varTextData);
	niptNew.LoadFromVariant(aisNew.m_varTextData);

	// (z.manning 2010-02-16 18:02) - PLID 37230 - Now find any newly created or newly
	// deleted text stamps.
	CArray<TextString,TextString&> aryNewStamps, aryDeletedStamps;
	//TES 3/6/2012 - PLID 45127 - It is now possible for stamps to change.
	//TES 3/8/2012 - PLID 48733 - It is now possible for multiple stamps to change (if they're all the same type)
	// (r.gonet 05/11/2012) - PLID 49950 - Changed ChangedFrom and ChangedTo array to be a single array with objects that wrap both the to, the from stamp, and the type of change.
	CArray<CStampChangeInfo, CStampChangeInfo&> aryChangedStamps;
	niptOld.CalcStampDifferences(&niptNew, aryNewStamps, aryDeletedStamps, aryChangedStamps);

	// (z.manning 2011-10-24 11:04) - PLID 42061 - Since a change to a smart stamp image likely means a change to the 
	// table too, make sure that the table's dialog exists because that is necessary to load certain portions of
	// the table that may be needed (such as dropdown contents).
	if(m_pSmartStampTableDetail != NULL && m_pSmartStampTableDetail->GetParentTopic() != NULL) {
		m_pSmartStampTableDetail->EnsureEmrItemAdvDlg(m_pSmartStampTableDetail->GetParentTopic()->GetTopicWndRaw());
	}
	
	BOOL bCurrentDetailDeleted = FALSE;

	// (z.manning 2010-02-16 09:19) - PLID 37230 - We also need to support smart stamp spawning
	// (z.manning 2010-02-26 18:00) - PLID 37230 - Don't do this if we're popped up!
	if(m_bSmartStampsEnabled && !m_bIsPoppedUp)
	{
		if(m_pSmartStampTableDetail == NULL) {
			// (j.jones 2010-03-02 11:40) - PLID 37318 - First ASSERT, because this shouldn't
			// be possible, and we should find out how we got to this state.
			//Second, try to ensure a link right now. If the detail ID is -1, we should be creating a new table.
			ASSERT(FALSE);
			m_pParentTopic->GetParentEMN()->EnsureSmartStampLinks(this);
			if(m_pSmartStampTableDetail == NULL) {
				AfxThrowNxException("Linked table is null for smart stamp image ID = %li", m_nEMRDetailID);
			}
		}

		if(bFullStateChange)
		{
			//TES 3/16/2012 - PLID 45127 - This handling all never accounted for the possibility that multiple stamps could be changed at once,
			// if they did it while the image was popped up.  So, what we need to do is first find any entire stamp types that have been 
			// changed, then go through all of the individual stamps and all of the stamp types.
			//TES 3/16/2012 - PLID 45127 - We keep arrays for just the types, then separate arrays for the stamps of those types.
			CArray<long,long> arStampTypesChangedFrom, arStampTypesChangedTo;
			CArray<TextString,TextString&> arStampTypesFromStamps, arStampTypesToStamps;
			for(int nStamp = 0; nStamp < aryChangedStamps.GetSize(); nStamp++) {
				// (r.gonet 05/11/2012) - PLID 49950 - Get the stamp from the singular array instead of the to/from arrays
				TextString tsChangedFrom = aryChangedStamps[nStamp].m_tsFrom;
				//TES 3/16/2012 - PLID 45127 - First, see if we've already determined that we're changing this entire type.
				// (r.gonet 05/11/2012) - PLID 49950 - Changes to type have some preprocessing of the array.
				if(aryChangedStamps[nStamp].m_esctChangeType == CStampChangeInfo::esctType) {
					bool bFound = false;
					for(int nType = 0; nType < arStampTypesChangedFrom.GetSize() && !bFound; nType++) {
						if(arStampTypesChangedFrom[nType] == tsChangedFrom.nStampID) bFound = true;
					}
					if(bFound) {
						//TES 3/16/2012 - PLID 45127 - We have, so add it to our type list, and remove it from our individual list.
						arStampTypesFromStamps.Add(tsChangedFrom);
						arStampTypesToStamps.Add(aryChangedStamps[nStamp].m_tsTo);
						aryChangedStamps.RemoveAt(nStamp);
						nStamp--;
					}
					else {
						//TES 3/16/2012 - PLID 45127 - Now, check all the stamps in the array for this type.  If any of them were changed to 
						// a different type, then we'll need to process them all individually.
						// (r.gonet 05/11/2012) - PLID 49950 - Get the stamp from the singular array instead of the to/from arrays
						TextString tsChangedTo = aryChangedStamps[nStamp].m_tsTo;
						bool bMismatched = false;
						int nCount = 0;
						for(int i = 0; i < aryChangedStamps.GetSize() && !bMismatched; i++) {
							if(aryChangedStamps[i].m_tsFrom.nStampID == tsChangedFrom.nStampID) {
								nCount++;
								if(aryChangedStamps[i].m_tsTo.nStampID != tsChangedTo.nStampID) {
									bMismatched = true;
								}
							}
						}
						if(!bMismatched && nCount > 1) {
							//TES 3/16/2012 - PLID 45127 - Next check if the stamps all came from the same row.
							EMRImageStamp* pGlobalStampFrom = GetMainFrame()->GetEMRImageStampByID(tsChangedFrom.nStampID);
							if(pGlobalStampFrom->eSmartStampTableSpawnRule == esstsrIncreaseQuantity) {
								//TES 3/16/2012 - PLID 45127 - OK, everything's looking good so far.  Our last check is to see
								// if there are any stamps on the image that are this type but aren't getting changed.
								bool bMissingStampFound = false;
								for(int i = 0; i < m_arypImageStamps.GetSize() && !bMissingStampFound; i++) {
									EmrDetailImageStamp *pStamp = m_arypImageStamps[i];
									if(pStamp->nStampID == tsChangedFrom.nStampID) {
										bool bFound = false;
										// (r.gonet 05/11/2012) - PLID 49950 - Get the stamp from the singular array instead of the to/from arrays
										for(int j = 0; j < aryChangedStamps.GetSize() && !bFound; j++) {
											if(DoStampsMatch(pStamp, aryChangedStamps[j].m_tsFrom)) bFound = true;
										}
										if(!bFound) {
											bMissingStampFound = true;
										}
									}
								}
								if(!bMissingStampFound) {
									//TES 3/16/2012 - PLID 45127 - Ok, every stamp of this type on the image is getting changed to the same
									// new type.  So, we can process this type separately.  Add to our type list and remove from the individual list.
									arStampTypesChangedFrom.Add(tsChangedFrom.nStampID);
									arStampTypesFromStamps.Add(tsChangedFrom);
									arStampTypesChangedTo.Add(tsChangedTo.nStampID);
									arStampTypesToStamps.Add(tsChangedTo);
									// (r.gonet 05/11/2012) - PLID 49950 - Remove the stamp from the singular array instead of the to/from arrays
									aryChangedStamps.RemoveAt(nStamp);
									nStamp--;
								}
							}
						}
					}
				} else if(aryChangedStamps[nStamp].m_esctChangeType == CStampChangeInfo::esctPosition) {
					// (r.gonet 05/11/2012) - PLID 49950 - A simple change, we don't need to preprocess the change array
				} else {
					// (r.gonet 05/11/2012) - PLID 49950 - Either this is a esctNone change, which should not be in here
					//  or there is a new type change that we forgot to account for here.
					//  If you are a developer and get this, you need to add your new change type.
					//  Also there are some places below you need to change as well. Basically anything that compares the ChangeType.
					ASSERT(FALSE);
				}
			}

			//TES 3/6/2012 - PLID 45127 - Special handling for when stamps are changed
			//TES 3/8/2012 - PLID 48733 - Multiple stamps may have changed, keep the previous behavior if there's only one.
			//TES 3/16/2012 - PLID 45127 - Actually, the way we'll do this now is first go through all our "process individually" stamps,
			// then we'll go through the stamps we want to process by type.
			// (r.gonet 05/11/2012) - PLID 49950 - Get the stamps from the singular array instead of the to/from arrays
			for(int nStamp = 0; nStamp < aryChangedStamps.GetSize(); nStamp++) {
				TextString tsChangedFrom = aryChangedStamps[nStamp].m_tsFrom;
				TextString tsChangedTo = aryChangedStamps[nStamp].m_tsTo;
				
				BOOL bWillRemoveOld = FALSE;
				BOOL bWillAddNew = FALSE;

				EMRImageStamp* pGlobalStampFrom = NULL;
				EMRImageStamp* pGlobalStampTo = NULL;
				EmrDetailImageStamp *pChangeStamp = NULL;
				//TES 3/30/2012 - PLID 48733 - Check if we're changing from a free-text stamp (ID = -2)
				if(tsChangedFrom.nStampID != -2) {
					//TES 3/6/2012 - PLID 45127 - Gather some information about what changed.
					pGlobalStampFrom = GetMainFrame()->GetEMRImageStampByID(tsChangedFrom.nStampID);
					pGlobalStampTo = GetMainFrame()->GetEMRImageStampByID(tsChangedTo.nStampID);
					pChangeStamp = m_arypImageStamps.FindByTextString(tsChangedFrom);
					ASSERT(pGlobalStampFrom);
					ASSERT(pGlobalStampTo);
					ASSERT(pChangeStamp);

					// (r.gonet 05/11/2012) - PLID 49950 - If our change requires modifying the smartstamp table, figure out what we need to do to the table
					if(aryChangedStamps[nStamp].m_esctChangeType == CStampChangeInfo::esctType) {
						//TES 3/6/2012 - PLID 45127 - Now, figure out how it would act if we were just removing the old stamp and adding the new one.
						if(pGlobalStampFrom->eSmartStampTableSpawnRule == esstsrAddNewRow) {
							bWillRemoveOld = TRUE;
						}
						else if(pGlobalStampFrom->eSmartStampTableSpawnRule == esstsrIncreaseQuantity) {
							//TES 3/6/2012 - PLID 45127 -We'll remove the old row only if this was the last stamp in it.
							EmrDetailImageStamp *pNewDetailStampForRow = m_pSmartStampTableDetail->GetSmartStampImageDetails()->FindReplacementDetailStamp(pChangeStamp);
							bWillRemoveOld = (pNewDetailStampForRow == NULL);
						}
						
						if(pGlobalStampTo->eSmartStampTableSpawnRule == esstsrAddNewRow) {
							bWillAddNew = TRUE;
						}
						else if(pGlobalStampTo->eSmartStampTableSpawnRule == esstsrIncreaseQuantity) {
							//TES 3/6/2012 - PLID 45127 - We'll add a new row only if there isn't one already
							TableRow *pRow = m_pSmartStampTableDetail->GetRowByStampID(pGlobalStampTo->nID);
							bWillAddNew = (pRow == NULL);
						}
					} else {
						// (r.gonet 05/11/2012) - PLID 49950 - Our change won't affect the table's rows, so no rows need added or deleted
					}
				}

				// (r.gonet 05/11/2012) - PLID 49950 - If we need to alter the smartstamp table, figure out the spawning and unspawning here.
				if(aryChangedStamps[nStamp].m_esctChangeType == CStampChangeInfo::esctType) {
					if(bWillRemoveOld && bWillAddNew) {
						//TES 3/6/2012 - PLID 45127 - Instead of removing the old row and adding a new one, change it in place.
						//TES 3/6/2012 - PLID 45127 - Update the detail stamp with the new global info
						pChangeStamp->nStampID = tsChangedTo.nStampID;
						pChangeStamp->eRule = pGlobalStampTo->eSmartStampTableSpawnRule;

						//TES 3/6/2012 - PLID 45127 - Get the row we're changing
						long nStampIndex = GetStampIndexInDetailByType(pChangeStamp);
						TableRowID rowid(pChangeStamp, nStampIndex);
						TableRow *ptr = m_pSmartStampTableDetail->GetRowByID(&rowid);
						TableRow tr;
						if(ptr == NULL) {
							tr.m_ID = TableRowID(pChangeStamp, nStampIndex);
							ptr = &tr;
						}
						//TES 3/16/2012 - PLID 45127 - We need to unspawn this stamp's regular actions before messing with the row.
						CArray<TableSpawnInfo,TableSpawnInfo&> aryStampUnspawnInfo;
						aryStampUnspawnInfo.Add(TableSpawnInfo(-1, *ptr, tsChangedFrom.nStampID));
						CEMNUnspawner eu(m_pParentTopic->GetParentEMN());
						eu.RemoveActionsBySmartStamps(&aryStampUnspawnInfo, this);

						//TES 3/6/2012 - PLID 45127 - Point it to the new stamp
						// (c.haag 2012-10-26) - PLID 53440 - Use the setter function
						ptr->m_ID.SetImageStampID(tsChangedTo.nStampID);
						
						//TES 3/6/2012 - PLID 45127 - Now update the type and description column, all the rest will retain their previous values.
						TableColumn *ptcType = m_pSmartStampTableDetail->GetColumnByListSubType(lstSmartStampType);
						if(ptcType) {
							TableElement te;
							m_pSmartStampTableDetail->GetTableElement(&(ptr->m_ID), ptcType->nID, te);
							te.LoadValueFromString(tsChangedTo.strTypeName, GetParentEMN());
							m_pSmartStampTableDetail->SetTableElement(te);
						}

						TableColumn *ptcDescription = m_pSmartStampTableDetail->GetColumnByListSubType(lstSmartStampDescription);
						if(ptcDescription) {
							TableElement te;
							m_pSmartStampTableDetail->GetTableElement(&(ptr->m_ID), ptcDescription->nID, te);
							te.LoadValueFromString(pGlobalStampTo->strDescription, GetParentEMN());
							m_pSmartStampTableDetail->SetTableElement(te);
						}

						//TES 3/8/2012 - PLID 45127 - We also need to apply any default dropdown selections with the new stamp
						for(int nColIndex = 0; nColIndex < m_pSmartStampTableDetail->GetColumnCount(); nColIndex++)
						{
							TableColumn *ptc = m_pSmartStampTableDetail->GetColumnPtr(nColIndex);
							if(ptc->nType == LIST_TYPE_DROPDOWN)
							{
								// (z.manning 2011-10-12 16:12) - PLID 45728 - This is a dropdown column so see if it has any
								// stamp based default selections.
								CArray<long,long> *parynDefaultDropdownIDs = ptc->GetDefaultDropdownIDsForStampID(tsChangedTo.nStampID);
								if(parynDefaultDropdownIDs != NULL && !parynDefaultDropdownIDs->IsEmpty())
								{
									// (z.manning 2011-10-12 16:14) - PLID 45728 - We have default selections so let's set them
									// up in the smart stamp table.
									TableElement teDropdown;
									teDropdown.m_pRow = ptr;
									teDropdown.m_pColumn = ptc;
									for(int nDefaultDropdownIndex = 0; nDefaultDropdownIndex < parynDefaultDropdownIDs->GetCount(); nDefaultDropdownIndex++) {
										const long nDropdownID = parynDefaultDropdownIDs->GetAt(nDefaultDropdownIndex);
										teDropdown.m_anDropdownIDs.Add(nDropdownID);
									}
									m_pSmartStampTableDetail->SetTableElement(teDropdown, TRUE, FALSE);
								}
							}
						}


						//TES 3/6/2012 - PLID 45127 - Now update the state to reflect our changes.
						_variant_t varOldTableState = m_pSmartStampTableDetail->GetState();
						m_pSmartStampTableDetail->RecreateStateFromContent();
						m_pSmartStampTableDetail->RequestStateChange(m_pSmartStampTableDetail->GetState(), varOldTableState);
						//TES 3/16/2012 - PLID 45127 - Now process the changed stamp's actions
						m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEmrSmartStampImageActions(ptr, this, FALSE);
					}
					else {
						//TES 3/6/2012 - PLID 45127 - We can't update the row in place, so just go ahead and act like we did remove the old 
						// stamp and add the new one.
						aryDeletedStamps.Add(tsChangedFrom);
						//TES 3/8/2012 - PLID 48728 - If a new row is added, it should reflect which stamp it started out as.
						//TES 3/26/2012 - PLID 48728 - Find the initial type in the smart stamp table
						TableRow *pRow = m_pSmartStampTableDetail->GetRowByStampID(tsChangedFrom.nStampID);
						if(pRow) {
							TableColumn *ptcInitialType = m_pSmartStampTableDetail->GetColumnByListSubType(lstSmartStampInitialType);
							if(ptcInitialType) {
								TableElement te;
								m_pSmartStampTableDetail->GetTableElement(&(pRow->m_ID), ptcInitialType->nID, te);
								tsChangedTo.strInitialType = te.m_strValue;
							}
						}
						aryNewStamps.Add(tsChangedTo);					
					}
				} // (r.gonet 05/11/2012) - PLID 49950 - Now for simple changes
				else if(aryChangedStamps[nStamp].m_esctChangeType == CStampChangeInfo::esctPosition) {
					// The user has moved the stamp, we need to update the position in Practice's stamp array
					if(pChangeStamp != NULL) {
						pChangeStamp->x = tsChangedTo.x;
						pChangeStamp->y = tsChangedTo.y;
					}
				}
			}
			//TES 3/16/2012 - PLID 45127 - Now go through each of the entire types that was changed
			// (r.gonet 05/11/2012) - PLID 49950 - ...If there were any changed
			for(int nStampType = 0; nStampType < arStampTypesChangedFrom.GetSize(); nStampType++) {
				//TES 3/8/2012 - PLID 48733 - This should only have happened if the previous row was an Increase Quantity row.
				long nStampTypeFrom = arStampTypesChangedFrom[nStampType];
				long nStampTypeTo = arStampTypesChangedTo[nStampType];
				EMRImageStamp* pGlobalStampFrom = GetMainFrame()->GetEMRImageStampByID(nStampTypeFrom);
				EMRImageStamp* pGlobalStampTo = GetMainFrame()->GetEMRImageStampByID(nStampTypeTo);
				ASSERT(pGlobalStampFrom);
				ASSERT(pGlobalStampTo);
				ASSERT(pGlobalStampFrom->eSmartStampTableSpawnRule == esstsrIncreaseQuantity);
				TableRow *ptr = m_pSmartStampTableDetail->GetRowByStampID(nStampTypeFrom);
				TableRow *ptrTo = m_pSmartStampTableDetail->GetRowByStampID(nStampTypeTo);

				//TES 3/16/2012 - PLID 45127 - We need to unspawn this stamp's regular actions before messing with the row.
				//TES 3/30/2012 - PLID 48733 - Make sure we unspawn the actions for ALL the stamps of this type
				CArray<TableSpawnInfo,TableSpawnInfo&> aryStampUnspawnInfo;
				for(int nStampIndex = 0; nStampIndex < m_arypImageStamps.GetSize(); nStampIndex++) {
					EmrDetailImageStamp *pDetailStamp = m_arypImageStamps.GetAt(nStampIndex);
					if(pDetailStamp->nStampID == nStampTypeFrom) {
						TableRowID rowid(pDetailStamp, nStampIndex);
						TableRow *ptr = m_pSmartStampTableDetail->GetRowByID(&rowid);
						TableRow tr;
						if(ptr == NULL) {
							tr.m_ID = TableRowID(pDetailStamp, nStampIndex);
							ptr = &tr;
						}
						aryStampUnspawnInfo.Add(TableSpawnInfo(-1, *ptr, nStampTypeFrom));
					}
				}
				CEMNUnspawner eu(m_pParentTopic->GetParentEMN());
				eu.RemoveActionsBySmartStamps(&aryStampUnspawnInfo, this);

				//TES 3/28/2012 - PLID 45127 - Remember to check whether the destination type already has a row; if so, we can't edit in-place.
				if(pGlobalStampTo->eSmartStampTableSpawnRule == esstsrIncreaseQuantity && ptr != NULL && ptrTo == NULL) {
					//TES 3/8/2012 - PLID 48733 - OK, they've got the same rule, so we can just edit the row in place
					for(int nStamp = 0; nStamp < arStampTypesFromStamps.GetSize(); nStamp++) {
						EmrDetailImageStamp *pChangeStamp = m_arypImageStamps.FindByTextString(arStampTypesFromStamps[nStamp]);
						ASSERT(pChangeStamp);
						if(pChangeStamp->nStampID == nStampTypeFrom) {
							pChangeStamp->nStampID = nStampTypeTo;
							pChangeStamp->eRule = pGlobalStampTo->eSmartStampTableSpawnRule;
						}
					}
					//TES 3/8/2012 - PLID 48733 - Point it to the new stamp
					// (c.haag 2012-10-26) - PLID 53440 - Use the setter function
					ptr->m_ID.SetImageStampID(nStampTypeTo);
					
					//TES 3/8/2012 - PLID 48733 - Now update the type and description column, all the rest will retain their previous values.
					TableColumn *ptcType = m_pSmartStampTableDetail->GetColumnByListSubType(lstSmartStampType);
					if(ptcType) {
						TableElement te;
						m_pSmartStampTableDetail->GetTableElement(&(ptr->m_ID), ptcType->nID, te);
						te.LoadValueFromString(pGlobalStampTo->strTypeName, GetParentEMN());
						m_pSmartStampTableDetail->SetTableElement(te);
					}

					TableColumn *ptcDescription = m_pSmartStampTableDetail->GetColumnByListSubType(lstSmartStampDescription);
					if(ptcDescription) {
						TableElement te;
						m_pSmartStampTableDetail->GetTableElement(&(ptr->m_ID), ptcDescription->nID, te);
						te.LoadValueFromString(pGlobalStampTo->strDescription, GetParentEMN());
						m_pSmartStampTableDetail->SetTableElement(te);
					}

					//TES 3/8/2012 - PLID 48733 - Apply any default dropdown selections
					for(int nColIndex = 0; nColIndex < m_pSmartStampTableDetail->GetColumnCount(); nColIndex++)
					{
						TableColumn *ptc = m_pSmartStampTableDetail->GetColumnPtr(nColIndex);
						if(ptc->nType == LIST_TYPE_DROPDOWN)
						{
							// (z.manning 2011-10-12 16:12) - PLID 45728 - This is a dropdown column so see if it has any
							// stamp based default selections.
							CArray<long,long> *parynDefaultDropdownIDs = ptc->GetDefaultDropdownIDsForStampID(nStampTypeTo);
							if(parynDefaultDropdownIDs != NULL && !parynDefaultDropdownIDs->IsEmpty())
							{
								// (z.manning 2011-10-12 16:14) - PLID 45728 - We have default selections so let's set them
								// up in the smart stamp table.
								TableElement teDropdown;
								teDropdown.m_pRow = ptr;
								teDropdown.m_pColumn = ptc;
								for(int nDefaultDropdownIndex = 0; nDefaultDropdownIndex < parynDefaultDropdownIDs->GetCount(); nDefaultDropdownIndex++) {
									const long nDropdownID = parynDefaultDropdownIDs->GetAt(nDefaultDropdownIndex);
									teDropdown.m_anDropdownIDs.Add(nDropdownID);
								}
								m_pSmartStampTableDetail->SetTableElement(teDropdown, TRUE, FALSE);
							}
						}
					}

					//TES 3/8/2012 - PLID 48733 - Now update the state to reflect our changes.
					_variant_t varOldTableState = m_pSmartStampTableDetail->GetState();
					m_pSmartStampTableDetail->RecreateStateFromContent();
					m_pSmartStampTableDetail->RequestStateChange(m_pSmartStampTableDetail->GetState(), varOldTableState);
					//TES 3/16/2012 - PLID 45127 - Now process the changed stamp's actions
					//TES 4/2/2012 - PLID 48733 - Make sure and capture all stamps of this type.
					for(int nStampIndex = 0; nStampIndex < m_arypImageStamps.GetSize(); nStampIndex++) {
						EmrDetailImageStamp *pDetailStamp = m_arypImageStamps.GetAt(nStampIndex);
						if(pDetailStamp->nStampID == nStampTypeTo) {
							TableRowID rowid(pDetailStamp, nStampIndex);
							TableRow *ptrSpawn = m_pSmartStampTableDetail->GetRowByID(&rowid);
							TableRow tr;
							if(ptrSpawn == NULL) {
								tr.m_ID = TableRowID(pDetailStamp, nStampIndex);
								ptrSpawn = &tr;
							}
							m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEmrSmartStampImageActions(ptrSpawn, this, FALSE);
						}
					}
					
				}
				else {
					//TES 3/8/2012 - PLID 48733 - The new stamps are going to be new rows, so just unspawn/respawn
					//aryDeletedStamps.Append(arStampTypesFromStamps);
					for(int nStamp = 0; nStamp < arStampTypesToStamps.GetSize(); nStamp++) {
						TextString tsFrom = arStampTypesFromStamps[nStamp];
						TextString tsTo = arStampTypesToStamps[nStamp];
						aryDeletedStamps.Add(tsFrom);
						tsTo.strInitialType = arStampTypesFromStamps[0].strTypeName;
						aryNewStamps.Add(tsTo);
					}
					//aryNewStamps.Append(arStampTypesToStamps);
				}
			}
			CArray<TableSpawnInfo,TableSpawnInfo&> aryStampUnspawnInfo;
			//TES 3/15/2010 - PLID 37530 - Smart Stamp spawning is now enabled on templates.
			//if(!m_bIsTemplateDetail)
			//{
				// (z.manning 2010-03-02 16:28) - PLID 37571 - Unspawn regular smart stamp actions. We must do this
				// before we potentially remove rows from the smart stamp table.
				for(int nDelStampIndex = 0; nDelStampIndex < aryDeletedStamps.GetSize(); nDelStampIndex++) {
					TextString ts = aryDeletedStamps.GetAt(nDelStampIndex);
					EmrDetailImageStamp *pDetailStamp = m_arypImageStamps.FindByTextString(ts);
					if(pDetailStamp != NULL) {
						//TES 3/17/2010 - PLID 37530 - Need to specify the stamp index
						long nStampIndex = GetStampIndexInDetailByType(pDetailStamp);
						TableRowID rowid(pDetailStamp, nStampIndex);
						TableRow *ptr = m_pSmartStampTableDetail->GetRowByID(&rowid);
						TableRow tr;
						if(ptr == NULL) {
							tr.m_ID = TableRowID(pDetailStamp, nStampIndex);
							ptr = &tr;
						}
						aryStampUnspawnInfo.Add(TableSpawnInfo(-1, *ptr, ts.nStampID));
					}
				}
				if(aryStampUnspawnInfo.GetSize() > 0)
				{
					CEMNUnspawner eu(m_pParentTopic->GetParentEMN());
					eu.RemoveActionsBySmartStamps(&aryStampUnspawnInfo, this);
				}
			//}
		}

		// (z.manning 2010-03-18 10:22) - PLID 37571 - Did we unspawn ourself?
		// (z.manning 2011-03-02 17:18) - PLID 42335 - This should not check if the detail is simply pending deletion.
		if(m_pParentTopic != NULL && m_pParentTopic->IsDetailDeleted(this, FALSE)) {
			bCurrentDetailDeleted = TRUE;
		}

		//TES 3/15/2010 - PLID 37757 - We allow Smart Stamp tables to be filled on templates now.
		// (z.manning 2010-03-18 10:45) - PLID 37571 - Don't bother with any of this if we just unspawned ourself
		if(/*!m_bIsTemplateDetail &&*/ !bCurrentDetailDeleted && (aryDeletedStamps.GetSize() > 0 || aryNewStamps.GetSize() > 0))
		{
			BOOL bTableContentChanged = FALSE;

			// (z.manning 2010-02-16 17:59) - PLID 37230 - We have 3 built-in column types that get
			// populated in the linked table with the special smart stamp spawning.
			TableColumn *ptcType = m_pSmartStampTableDetail->GetColumnByListSubType(lstSmartStampType);
			//TES 3/8/2012 - PLID 48728 - Added Initial Type as a built-in column
			TableColumn *ptcInitialType = m_pSmartStampTableDetail->GetColumnByListSubType(lstSmartStampInitialType);
			TableColumn *ptcLocation = m_pSmartStampTableDetail->GetColumnByListSubType(lstSmartStampLocation);
			TableColumn *ptcDescription = m_pSmartStampTableDetail->GetColumnByListSubType(lstSmartStampDescription);
			// (j.jones 2010-08-11 10:59) - PLID 39496 - supported the auto-number column, which may not exist
			// on some older smartstamp tables
			TableColumn *ptcAutoNumber = m_pSmartStampTableDetail->GetColumnByListSubType(lstSmartStampAutoNumber);

			for(int nErasedStampIndex = 0; nErasedStampIndex < aryDeletedStamps.GetSize(); nErasedStampIndex++)
			{
				// (z.manning 2010-02-24 09:40) - PLID 37225 - We need to go through and remove the corresponding
				// table row for whatever stamps were erased.
				TextString ts = aryDeletedStamps.GetAt(nErasedStampIndex);
				// (z.manning 2010-02-24 12:03) - PLID 37225 - Find the detail image stamp equivilent of the current
				// text string.
				EmrDetailImageStamp *pDetailStamp = m_arypImageStamps.FindByTextString(ts);
				if(pDetailStamp != NULL)
				{
					// (j.jones 2010-04-07 12:13) - PLID 38069 - if the stamp hasn't already been used
					// and the rule is esstsrDoNotAddToTable, don't try to remove from the table
					if(pDetailStamp->eRule != esstsrDoNotAddToTable) {
						BOOL bDetailStampHasRow = TRUE;
						// (z.manning 2010-02-24 12:03) - PLID 37225 - Find the table row that corresponds to this
						// detail stamp.
						//TES 3/17/2010 - PLID 37530 - Need to specify the stamp index
						TableRowID rowid(pDetailStamp, GetStampIndexInDetailByType(pDetailStamp));
						TableRow *ptr = m_pSmartStampTableDetail->GetRowByID(&rowid);
						if(ptr == NULL) {
							// (z.manning 2010-02-24 12:04) - PLID 37225 - We did not find a row. The only time this should
							// ever happen is with quantity-based spawning as only one of potentially many details with the
							// same stamp actually points to the TableRow object.
							bDetailStampHasRow = FALSE;
							if(pDetailStamp->eRule == esstsrIncreaseQuantity) {
								ptr = m_pSmartStampTableDetail->GetRowByStampID(pDetailStamp->nStampID);
							}
							if(ptr == NULL) {
								// (z.manning 2010-02-24 12:05) - PLID 37225 - We still didn't find a row which should not
								// be possible so throw an exception.
								AfxThrowNxException("Could not find smart stamp table row for detail stamp ID = %li", pDetailStamp->nID);
							}
						}

						BOOL bRemoveRow = FALSE;
						if(pDetailStamp->eRule == esstsrIncreaseQuantity)
						{
							// (z.manning 2010-04-09 15:42) - PLID 37937 - I reworked this code a bit as before we
							// could assume that the quantity column definitely existed, but no longer can. So remove
							// the row based on whether or not we have any more detail stamps from the same global stamp.
							// (z.manning 2011-01-27 15:59) - PLID 42335 - We need to check any of the possibly multiple
							// images linked to the currently linked smart stamp table.
							//EmrDetailImageStamp *pNewDetailStampForRow = m_arypImageStamps.FindReplacementDetailStamp(pDetailStamp);
							EmrDetailImageStamp *pNewDetailStampForRow = m_pSmartStampTableDetail->GetSmartStampImageDetails()->FindReplacementDetailStamp(pDetailStamp);
							bRemoveRow = (pNewDetailStampForRow == NULL);
							if(bDetailStampHasRow) {
								// (z.manning 2010-02-24 12:07) - PLID 37225 - For quantity based spawning, only
								// one detail image stamp actually is stored in the row object and in data. If that
								// is the one that we removed then we need to find a replacement detail stamp to
								// tie to the TableRow.
								if(pNewDetailStampForRow != NULL) {
									// (z.manning 2011-02-16 17:51) - PLID 42235 - Flag this detail stamp as being used in the table
									pNewDetailStampForRow->bUsedInTableData = TRUE;
									UpdateDetailStampPointer(pDetailStamp, pNewDetailStampForRow);
									//TES 3/17/2010 - PLID 37530 - Need to specify the stamp index
									ptr->m_ID = TableRowID(pNewDetailStampForRow, GetStampIndexInDetailByType(pNewDetailStampForRow));
								}
							}

							// (z.manning 2010-03-10 09:31) - PLID 37225 - Also update the location field if
							// necessary
							if(!bRemoveRow)
							{
								TableElement teLocation;
								if(ptcLocation != NULL && m_pSmartStampTableDetail->GetTableElement(&ptr->m_ID, ptcLocation->nID, teLocation))
								{
									// (z.manning 2010-03-10 14:33) - PLID 37225 - Ok, we have the location table
									// cell so let's see if the current detail stamp is in a hot spot.
									// NOTE: It is entirely possible that the hot spot location data or even
									// the hot spots themseleves are not the same as they were when this stamp
									// was created. We don't do any special handling for that. We simply check
									// the current hot spot this stamp is in and if that location text matches
									// part of the location text within the cell already we remove it. If someone
									// manually edited this cell or if the location data has semicolons in it then
									// this may not work, but this is how we're doing it for this release as anything
									// more advanced would require a data structure that we have decided not to
									// handle now. We may revisit this in PLID 37552.
									CString strLocation = teLocation.GetValueAsString();
									CEMRHotSpot *pHotSpot = GetHotSpotFromDetailStamp(pDetailStamp);
									if(pHotSpot != NULL) {
										if(!m_arypImageStamps.DoesStampAlreadyExistInHotspot(pDetailStamp->nStampID, pHotSpot, pDetailStamp)) {
											// (z.manning 2010-03-10 15:07) - PLID 37225 - Ok, this was the last instance
											// of this stamp on the image, so attemp to remove hot spot location info from
											// the linked table.
											CString strHotSpotLocation = pHotSpot->GetFullAnatomicLocation();
											int nFind = strLocation.Find(strHotSpotLocation + "; ");
											if(nFind != -1) {
												strLocation.Delete(nFind, strHotSpotLocation.GetLength() + 2);
											}
											else if(strLocation.GetLength() >= strHotSpotLocation.GetLength() && strLocation.Right(strHotSpotLocation.GetLength()) == strHotSpotLocation) {
												strLocation.Delete(strLocation.GetLength() - strHotSpotLocation.GetLength(), strHotSpotLocation.GetLength());
											}

											teLocation.LoadValueFromString(strLocation, m_pParentTopic->GetParentEMN());
											m_pSmartStampTableDetail->SetTableElement(teLocation, TRUE, FALSE);
										}
									}
								}
							}
						}
						else {
							bRemoveRow = TRUE;
						}

						if(bRemoveRow) {
							// (z.manning 2010-02-24 12:09) - PLID 37225 - We need to remove a row from the table
							// so let's do so now.
							//bTableContentChanged = TRUE;
							// (z.manning 2010-03-03 17:32) - PLID 37225 - We can't remove the row here because the
							// table may need to unspawn things itself and need data from the row object in order to do so.
							//m_pSmartStampTableDetail->RemoveTableRow(ptr);
							ptr->m_bDeleteOnStateChange = TRUE;

							// (j.jones 2010-08-11 10:59) - PLID 39496 - supported the auto-number column, which may not exist
							// on some older smartstamp tables
							if(ptcAutoNumber != NULL) {
								//must decrement numbers of remaining rows

								//get the old number
								long nAutoNumberOld = ParseAutoNumberIndexFromCell(m_pSmartStampTableDetail, &ptr->m_ID, ptcAutoNumber->nID);

								BOOL bDecrement = FALSE;

								if(nAutoNumberOld > 0) {

									//how are the numbers calculated?
									if(ptcAutoNumber->m_etantAutoNumberType == etantPerStamp) {

										//the autonumber is per stamp, so multiple rows for the same
										//stamp would have the same number, and we won't change
										//other numbers unless we just removed the last row for this stamp

										long nOldStampID = pDetailStamp->nStampID;
										
										//see if any other row has this StampID
										BOOL bFound = FALSE;
										for(int iRowIndex=0; iRowIndex<m_pSmartStampTableDetail->GetRowCount() && !bFound; iRowIndex++) {
											TableRow *ptrNew = m_pSmartStampTableDetail->GetRowPtr(iRowIndex);
											if(ptrNew) {
												// (c.haag 2012-10-26) - PLID 53440 - Use the getter function
												if(ptrNew->m_ID.GetImageStampID() == nOldStampID
													&& ptrNew != ptr) {

													bFound = TRUE;
												}
											}
										}

										if(!bFound) {
											//if we didn't find the same stamp in another row,
											//decrement all rows with a higher number
											bDecrement = TRUE;
										}
									}
									else { //per row

										//the autonumber is just a row-index, we will always decrement higher rows
										bDecrement = TRUE;
									}
								}

								if(bDecrement) {										
									//for all remaining rows with an index higher than ours, decrement by 1
									for(int iRowIndex=0; iRowIndex<m_pSmartStampTableDetail->GetRowCount(); iRowIndex++) {
										TableRow *ptrCur = m_pSmartStampTableDetail->GetRowPtr(iRowIndex);
										if(ptrCur) {

											//get the current number
											long nAutoNumberCur = ParseAutoNumberIndexFromCell(m_pSmartStampTableDetail, &ptrCur->m_ID, ptcAutoNumber->nID);

											//is it higher than the value we're removing?
											if(nAutoNumberCur > nAutoNumberOld) {

												//reset the cell
												nAutoNumberCur--;

												CString strAutoNumberCellValue;
												//apply the prefix
												strAutoNumberCellValue.Format("%s %li", ptcAutoNumber->m_strAutoNumberPrefix, nAutoNumberCur);
												//the prefix could have been blank, trim spaces if they exist
												strAutoNumberCellValue.TrimLeft();

												TableElement teAutoNumberCur;
												m_pSmartStampTableDetail->GetTableElement(&ptrCur->m_ID, ptcAutoNumber->nID, teAutoNumberCur);

												teAutoNumberCur.LoadValueFromString(strAutoNumberCellValue, m_pSmartStampTableDetail->m_pParentTopic->GetParentEMN());
												m_pSmartStampTableDetail->SetTableElement(teAutoNumberCur, TRUE, FALSE);
											}
										}
									}
								}
							}
						}
					}

					if(pDetailStamp->nID != -1) {
						// (z.manning 2010-02-24 12:10) - PLID 37225 - Make sure we keep track of deleted detail
						// stamps so we can delete them from data as well.
						m_arynDeletedImageStampIDs.Add(pDetailStamp->nID);
					}
					// (z.manning 2010-02-24 12:10) - PLID 37225 - Finally, remove our reference to this detail.
					m_arypImageStamps.RemoveDetailStamp(pDetailStamp);
				}
			}

			// (z.manning 2010-02-16 18:03) - PLID 37230 - Now let's handle the special smart stamp
			// spawning for each newly created stamp.
			for(int nNewStampIndex = 0; nNewStampIndex < aryNewStamps.GetSize(); nNewStampIndex++)
			{
				TextString ts = aryNewStamps.GetAt(nNewStampIndex);
				EMRImageStamp* pStamp = GetMainFrame()->GetEMRImageStampByID(ts.nStampID);
				if(pStamp == NULL) {
					AfxThrowNxException("Could not find stamp with ID = %li", ts.nStampID);
				}
				EMRSmartStampTableSpawnRule eRule = pStamp->eSmartStampTableSpawnRule;

				// (z.manning 2010-02-22 14:35) - PLID 37230 - Check and see if we already have
				// a table row for this stamp.
				TableRow *ptr = m_pSmartStampTableDetail->GetRowByStampID(pStamp->nID);
				if(ptr != NULL) {
					// (z.manning 2010-02-22 14:43) - PLID 37230 - This stamp does exist in the table
					// so now find the associated detail stamp.
					// (c.haag 2012-10-26) - PLID 53440 - Use the getter function
					EmrDetailImageStamp *pExistingDetailStamp = ptr->m_ID.GetDetailImageStampObject();
					if(pExistingDetailStamp == NULL) {
						// (z.manning 2011-03-02 16:01) - PLID 42235 - Need to check ALL detail stamps across ALL
						// linked images here.
						CEmrDetailImageStampArray aryAllDetailStamps;
						m_pSmartStampTableDetail->GetSmartStampImageDetails()->GetAllDetailImageStampsInOrder(&aryAllDetailStamps);
						pExistingDetailStamp = aryAllDetailStamps.FindByTableRow(ptr);
					}
					if(pExistingDetailStamp != NULL) {
						// (z.manning 2010-02-22 14:45) - PLID 37230 - If we have an existing occurrance
						// of this stamp already, then pull the rule field from here to avoid issues
						// where it could change on the global stamp setup.
						eRule = pExistingDetailStamp->eRule;
					}
					if(eRule != esstsrIncreaseQuantity) {
						// (z.manning 2010-02-22 14:46) - PLID 37230 - If this is not quantity based
						// spawning then we need to make a new row.
						ptr = NULL;
					}
				}
				
				// (z.manning 2010-02-22 14:47) - PLID 37230 - Create the new detail image stamp in memory
				long nNextOrderIndex = m_pSmartStampTableDetail->m_arySmartStampImageDetails.GetNextOrderIndex();
				EmrDetailImageStamp *pDetailImageStamp = m_arypImageStamps.FindByTextString(ts);
				if(pDetailImageStamp == NULL) {
					// (z.manning 2011-09-08 15:41) - PLID 45335 - Added 3D fields
					pDetailImageStamp = AddNewDetailImageStamp(-1, &ts, nNextOrderIndex, eRule);
				}
				else {
					// (z.manning 2011-02-02 15:40) - PLID 42335 - If we got here it means this stamp was already in
					// our array. The only current way this should happen is if this call to request the state change
					// is happening because of remembered smart stamp data.
					if(bFullStateChange) {
						ASSERT(FALSE);
						AfxThrowNxException("Unexpectedly found a detail image stamp (DetailStampID = %li, StampLoadedFromID = %li, StampID = %li, DetailID = %li)"
							, pDetailImageStamp->nID, pDetailImageStamp->nLoadedFromID, pDetailImageStamp->nStampID, m_nEMRDetailID);
					}
				}

				// (j.jones 2010-04-07 12:13) - PLID 38069 - if the stamp hasn't already been used
				// and the rule is esstsrDoNotAddToTable, don't add to the table!
				if(eRule != esstsrDoNotAddToTable)
				{
					if(ptr == NULL)
					{
						// (z.manning 2010-02-22 14:47) - PLID 37230 - We need to add the new to the table
						//TES 3/17/2010 - PLID 37530 - Need to specify the stamp index
						ptr = m_pSmartStampTableDetail->GetRowPtr(m_pSmartStampTableDetail->AddSmartStampRow(pDetailImageStamp, GetStampIndexInDetailByType(pDetailImageStamp)));

						// (z.manning 2011-02-16 17:45) - PLID 42335 - If this stamp is being used in the table
						// then we need to flag it as being used in data (even if it actually isn't yet).
						pDetailImageStamp->bUsedInTableData = TRUE;

						if(parypNewTableRows != NULL) {
							parypNewTableRows->Add(ptr);
						}
						bTableContentChanged = TRUE;

						// (z.manning 2011-02-23 12:51) - PLID 42549 - Check and see if we were passed in a remembered
						// table state.
						if(varRememberedTableState.vt != VT_EMPTY && varRememberedTableState.vt != VT_NULL)
						{
							CString strRememberedTableState = VarString(varRememberedTableState);
							CEmrTableStateIterator etsi(strRememberedTableState);
							long X, Y, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID;
							CString strData;
							while(etsi.ReadNextElement(X,Y,strData,nEmrDetailImageStampID,nEmrDetailImageStampPointer,nStampID))
							{
								if(nEmrDetailImageStampID == pDetailImageStamp->nLoadedFromID)
								{
									TableColumn *ptc = m_pSmartStampTableDetail->GetColumnByID(Y);
									if(ptc != NULL)
									{
										TableElement te;
										te.m_pRow = ptr;
										te.m_pColumn = ptc;
										if(!strData.IsEmpty()) {
											te.LoadValueFromString(strData, m_pSmartStampTableDetail->GetParentEMN());
											m_pSmartStampTableDetail->SetTableElement(te, TRUE, FALSE);
										}
									}
								}
							}
						}
						else
						{
							BOOL bNewRowHasData = FALSE;

							// (z.manning 2010-02-16 18:04) - PLID 37230 - Fill out the type column
							if(ptcType != NULL) {
								TableElement teType;
								teType.m_pRow = ptr;
								teType.m_pColumn = ptcType;
								if(!pStamp->strTypeName.IsEmpty()) {
									teType.LoadValueFromString(pStamp->strTypeName, m_pSmartStampTableDetail->m_pParentTopic->GetParentEMN());
									m_pSmartStampTableDetail->SetTableElement(teType, TRUE, FALSE);
									bNewRowHasData = TRUE;
								}
							}
							//TES 3/8/2012 - PLID 48728 - Added Initial Type, which defaults to the same value as the Type column
							// (but doesn't get updated when the stamp is changed)
							if(ptcInitialType != NULL) {
								TableElement teInitialType;
								teInitialType.m_pRow = ptr;
								teInitialType.m_pColumn = ptcInitialType;
								if(!pStamp->strTypeName.IsEmpty()) {
									//TES 3/8/2012 - PLID 48728 - If we were given an override Initial Type to use, then use it.
									CString strInitialType = ts.strInitialType.IsEmpty() ? pStamp->strTypeName : ts.strInitialType;
									teInitialType.LoadValueFromString(strInitialType, m_pSmartStampTableDetail->m_pParentTopic->GetParentEMN());
									m_pSmartStampTableDetail->SetTableElement(teInitialType, TRUE, FALSE);
									bNewRowHasData = TRUE;
								}
							}
							// (z.manning 2010-02-16 18:04) - PLID 37230 - Fill out the description column
							if(ptcDescription != NULL) {
								TableElement teDescription;
								teDescription.m_pRow = ptr;
								teDescription.m_pColumn = ptcDescription;
								if(!pStamp->strDescription.IsEmpty()) {
									teDescription.LoadValueFromString(pStamp->strDescription, m_pSmartStampTableDetail->m_pParentTopic->GetParentEMN());
									m_pSmartStampTableDetail->SetTableElement(teDescription, TRUE, FALSE);
									bNewRowHasData = TRUE;
								}
							}
							// (j.jones 2010-08-11 10:59) - PLID 39496 - supported the auto-number column, which may not exist
							// on some older smartstamp tables
							if(ptcAutoNumber != NULL) {

								TableElement teAutoNumber;
								teAutoNumber.m_pRow = ptr;
								teAutoNumber.m_pColumn = ptcAutoNumber;

								CString strAutoNumberCellValue = GenerateAutoNumberContentForCell(ptr, ptcAutoNumber);

								if(!strAutoNumberCellValue.IsEmpty()) {
									teAutoNumber.LoadValueFromString(strAutoNumberCellValue, m_pSmartStampTableDetail->m_pParentTopic->GetParentEMN());
									m_pSmartStampTableDetail->SetTableElement(teAutoNumber, TRUE, FALSE);
									bNewRowHasData = TRUE;
								}
							}

							// (z.manning 2011-10-12 16:12) - PLID 45728 - Go through each column and see if we need to default anything.
							for(int nColIndex = 0; nColIndex < m_pSmartStampTableDetail->GetColumnCount(); nColIndex++)
							{
								TableColumn *ptc = m_pSmartStampTableDetail->GetColumnPtr(nColIndex);
								if(ptc->nType == LIST_TYPE_DROPDOWN)
								{
									// (z.manning 2011-10-12 16:12) - PLID 45728 - This is a dropdown column so see if it has any
									// stamp based default selections.
									CArray<long,long> *parynDefaultDropdownIDs = ptc->GetDefaultDropdownIDsForStampID(pStamp->nID);
									if(parynDefaultDropdownIDs != NULL && !parynDefaultDropdownIDs->IsEmpty())
									{
										// (z.manning 2011-10-12 16:14) - PLID 45728 - We have default selections so let's set them
										// up in the smart stamp table.
										TableElement teDropdown;
										teDropdown.m_pRow = ptr;
										teDropdown.m_pColumn = ptc;
										for(int nDefaultDropdownIndex = 0; nDefaultDropdownIndex < parynDefaultDropdownIDs->GetCount(); nDefaultDropdownIndex++) {
											const long nDropdownID = parynDefaultDropdownIDs->GetAt(nDefaultDropdownIndex);
											teDropdown.m_anDropdownIDs.Add(nDropdownID);
										}
										m_pSmartStampTableDetail->SetTableElement(teDropdown, TRUE, FALSE);
										bNewRowHasData = TRUE;
									}
								}
							}

							if(bNewRowHasData) {
								// (z.manning 2011-03-21 17:29) - PLID 30608 - This row has data so let's update any autofill columns
								// in this row.
								m_pSmartStampTableDetail->UpdateAutofillColumnsByRow(ptr, NULL);
							}
						}
					}

					// (z.manning 2010-02-16 18:04) - PLID 37230 - Did we click in a hot spot? If so then
					// pull the location of that hot spot for the location column.
					CEMRHotSpot *pHotSpot = GetHotSpotFromDetailStamp(pDetailImageStamp);
					if(pHotSpot != NULL && ptcLocation != NULL) {
						TableElement teLocation;
						CString strExistingLocation;
						if(m_pSmartStampTableDetail->GetTableElement(&ptr->m_ID, ptcLocation->nID, teLocation)) {
							strExistingLocation = teLocation.GetValueAsString();
						}
						else {
							teLocation.m_pRow = ptr;
							teLocation.m_pColumn = ptcLocation;
						}
						CString strAnatomicLocation = FormatAnatomicLocation(pHotSpot->GetAnatomicLocationName(), pHotSpot->GetAnatomicQualifierName(), pHotSpot->GetSide());
						if(strExistingLocation.Find(strAnatomicLocation) == -1) {
							if(!strExistingLocation.IsEmpty()) {
								strAnatomicLocation = strExistingLocation + "; " + strAnatomicLocation;
							}
							teLocation.LoadValueFromString(strAnatomicLocation, m_pSmartStampTableDetail->m_pParentTopic->GetParentEMN());
							m_pSmartStampTableDetail->SetTableElement(teLocation, TRUE, FALSE);
						}
					}
				}
			}

			// (z.manning 2010-02-16 18:05) - PLID 37230 - We likely added or removed a row from
			// the linked table so we need to update its appearence.
			_variant_t varOldTableState = m_pSmartStampTableDetail->GetState();
			if(bTableContentChanged) {
				m_pSmartStampTableDetail->ReflectCurrentContent();
			}
			m_pSmartStampTableDetail->RecreateStateFromContent();
			//TES 3/11/2010 - PLID 37535 - RequestStateChange() now handles calling UpdateTableDataOutput()
			//m_pSmartStampTableDetail->UpdateTableDataOutput();
			m_pSmartStampTableDetail->RequestStateChange(m_pSmartStampTableDetail->GetState(), varOldTableState);
		}
	}
	// (j.jones 2010-04-07 10:35) - PLID 37971 - if not a smart stamp image,
	// simply track the changes to the text stamps, and handle spawning/unspawning
	else if(!m_bSmartStampsEnabled && !m_bIsPoppedUp)
	{
		//TES 3/26/2012 - PLID 45127 - We need to take all the stamps that changed, and add them to the deleted and new lists.  Since
		// this isn't a smart stamp image, we don't need to do anything differently than we would if they had erased the old stamps and
		// added the new ones.
		// (r.gonet 05/11/2012) - PLID 49950 - We must go through the changed stamps and append only the changed stamps where the changes affect
		//  the spawning.
		for(int nChangeIndex = 0; nChangeIndex < aryChangedStamps.GetSize(); nChangeIndex++) {
			if(aryChangedStamps[nChangeIndex].m_esctChangeType == CStampChangeInfo::esctType) {
				TextString tsFrom = aryChangedStamps[nChangeIndex].m_tsFrom;
				TextString tsTo = aryChangedStamps[nChangeIndex].m_tsTo;
				aryDeletedStamps.Add(tsFrom);
				aryNewStamps.Add(tsTo);
			} else if(aryChangedStamps[nChangeIndex].m_esctChangeType == CStampChangeInfo::esctPosition) {
				// (r.gonet 06/20/2012) - PLID 49950 - However, we do need to update positions
			} else {
				ASSERT(FALSE);
			}
		}

		if(bFullStateChange)
		{
			// (r.gonet 06/20/2012) - PLID 49950 - Handle changed stamps
			for(int nStamp = 0; nStamp < aryChangedStamps.GetSize(); nStamp++) {
				TextString tsChangedFrom = aryChangedStamps[nStamp].m_tsFrom;
				TextString tsChangedTo = aryChangedStamps[nStamp].m_tsTo;
				
				EMRImageStamp* pGlobalStampFrom = NULL;
				EMRImageStamp* pGlobalStampTo = NULL;
				EmrDetailImageStamp *pChangeStamp = NULL;
				
				if(tsChangedFrom.nStampID != -2) {
					// Gather some information about what changed.
					pGlobalStampFrom = GetMainFrame()->GetEMRImageStampByID(tsChangedFrom.nStampID);
					pGlobalStampTo = GetMainFrame()->GetEMRImageStampByID(tsChangedTo.nStampID);
					pChangeStamp = m_arypImageStamps.FindByTextString(tsChangedFrom);
					ASSERT(pGlobalStampFrom);
					ASSERT(pGlobalStampTo);
					ASSERT(pChangeStamp);
				}

				if(aryChangedStamps[nStamp].m_esctChangeType == CStampChangeInfo::esctPosition) {
					// The user has moved the stamp, we need to update the position in Practice's stamp array
					if(pChangeStamp != NULL) {
						pChangeStamp->x = tsChangedTo.x;
						pChangeStamp->y = tsChangedTo.y;
					}
				}
			}

			// (j.jones 2010-04-07 11:26) - PLID 37971 - unspawn regular smart stamp actions
			CArray<TableSpawnInfo,TableSpawnInfo&> aryStampUnspawnInfo;
			for(int nDelStampIndex = 0; nDelStampIndex < aryDeletedStamps.GetSize(); nDelStampIndex++) {
				TextString ts = aryDeletedStamps.GetAt(nDelStampIndex);
				EmrDetailImageStamp *pDetailStamp = m_arypImageStamps.FindByTextString(ts);
				if(pDetailStamp != NULL) {
					//TES 3/17/2010 - PLID 37530 - Need to specify the stamp index
					long nStampIndex = GetStampIndexInDetailByType(pDetailStamp);
					TableRowID rowid(pDetailStamp, nStampIndex);
					TableRow *ptr = NULL;
					if(m_pSmartStampTableDetail) {
						ptr = m_pSmartStampTableDetail->GetRowByID(&rowid);
					}
					TableRow tr;
					if(ptr == NULL) {
						tr.m_ID = TableRowID(pDetailStamp, nStampIndex);
						ptr = &tr;
					}
					aryStampUnspawnInfo.Add(TableSpawnInfo(-1, *ptr, ts.nStampID));

					if(pDetailStamp->nID != -1) {
						//make sure we keep track of deleted detail
						// stamps so we can delete them from data as well.
						m_arynDeletedImageStampIDs.Add(pDetailStamp->nID);
					}
					
					//finally, remove our reference to this detail.
					m_arypImageStamps.RemoveDetailStamp(pDetailStamp);
				}
			}
			if(aryStampUnspawnInfo.GetSize() > 0)
			{
				CEMNUnspawner eu(m_pParentTopic->GetParentEMN());
				eu.RemoveActionsBySmartStamps(&aryStampUnspawnInfo, this);
			}
		}

		//now handle new stamps
		for(int nNewStampIndex = 0; nNewStampIndex < aryNewStamps.GetSize(); nNewStampIndex++) {
			TextString ts = aryNewStamps.GetAt(nNewStampIndex);
			EMRImageStamp* pStamp = GetMainFrame()->GetEMRImageStampByID(ts.nStampID);
			if(pStamp == NULL) {
				AfxThrowNxException("Could not find stamp with ID = %li", ts.nStampID);
			}
			EMRSmartStampTableSpawnRule eRule = pStamp->eSmartStampTableSpawnRule;
			
			//create the new detail image stamp in memory
			EmrDetailImageStamp *pDetailImageStamp = m_arypImageStamps.FindByTextString(ts);
			if(pDetailImageStamp == NULL) {
				// (z.manning 2011-09-08 15:41) - PLID 45335 - Added 3D fields
				pDetailImageStamp = AddNewDetailImageStamp(-1, &ts, m_arypImageStamps.GetNextOrderIndex(), eRule);
			}
			else {
				// (z.manning 2011-02-02 15:40) - PLID 42335 - If we got here it means this stamp was already in
				// our array. The only current way this should happen is if this call to request the state change
				// is happening because of remembered smart stamp data.
				if(bFullStateChange) {
					ASSERT(FALSE);
					AfxThrowNxException("Unexpectedly found a detail image stamp [2] (DetailStampID = %li, StampLoadedFromID = %li, StampID = %li, DetailID = %li)"
						, pDetailImageStamp->nID, pDetailImageStamp->nLoadedFromID, pDetailImageStamp->nStampID, m_nEMRDetailID);
				}
			}
		}
	}

	//TES 3/15/2010 - PLID 37530 - Smart Stamp spawning is now enabled on templates.
	//if(!m_bIsTemplateDetail)
	// (z.manning 2010-03-18 10:46) - PLID 37571 - Don't spawn any new actions if we just unspawned ourself
	if(!bCurrentDetailDeleted && bFullStateChange)
	{
		// (z.manning 2010-03-02 13:37) - PLID 37571 - Handle the processing of regular actions for smart stamps.
		for(int nTextStringIndex = 0; nTextStringIndex < aryNewStamps.GetSize(); nTextStringIndex++) {
			TextString ts = aryNewStamps.GetAt(nTextStringIndex);
			EmrDetailImageStamp *pDetailStamp = m_arypImageStamps.FindByTextString(ts);
			if(pDetailStamp != NULL) {
				BOOL bDetailStampHasRow = TRUE;
				// (z.manning 2010-02-24 12:03) - PLID 37225 - Find the table row that corresponds to this
				// detail stamp.
				//TES 3/17/2010 - PLID 37530 - Need to specify the stamp index
				long nStampIndex = GetStampIndexInDetailByType(pDetailStamp);
				TableRowID rowid(pDetailStamp, nStampIndex);
				TableRow *ptr = NULL;
				// (j.jones 2010-04-07 11:09) - PLID 37971 - we now support spawning
				// for non-smartstamp images
				if(m_pSmartStampTableDetail) {
					ptr = m_pSmartStampTableDetail->GetRowByID(&rowid);
				}
				TableRow tr;
				if(ptr == NULL) {
					tr.m_ID = TableRowID(pDetailStamp, nStampIndex);
					ptr = &tr;
				}
				m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEmrSmartStampImageActions(ptr, this, FALSE);
			}
		}
	}

	CArray<long,long> arynNewlySelectedHotSpotIDs, arynNewlyUnselectedHotSpotIDs;
	CalcChangedDataIDFromState(_bstr_t(aisOld.m_strSelectedHotSpotData), _bstr_t(aisNew.m_strSelectedHotSpotData), arynNewlySelectedHotSpotIDs, arynNewlyUnselectedHotSpotIDs);
	
	if(arynNewlyUnselectedHotSpotIDs.GetSize() > 0 && bFullStateChange) 
	{
		//At this point, we've got some things that need to be unselected.  We want to generate a list of all the
		//	ListElement objects, and pass those in to our CEMNUnspawner utility.
		CEMRHotSpotArray aryHotSpots;
		for(int i = 0; i < arynNewlyUnselectedHotSpotIDs.GetSize(); i++)
		{
			long nHotSpotID = arynNewlyUnselectedHotSpotIDs.GetAt(i);
			//Search all list elements for a match
			for(int nHotSpotIndex = 0; nHotSpotIndex < m_aryImageHotSpots.GetSize(); nHotSpotIndex++) {
				CEMRHotSpot *pSpot = m_aryImageHotSpots.GetAt(nHotSpotIndex);
				if(nHotSpotID == pSpot->GetID()) {
					aryHotSpots.Add(pSpot);
				}
			}
		}

		//Now we've got a list of all the list elements to be removed.
		CEMNUnspawner eu(m_pParentTopic->GetParentEMN());
		eu.RemoveActionsByImageHotSpots(&aryHotSpots, this);
	}

	if(bFullStateChange)
	{
		for(int nSelHotSpotIndex = 0; nSelHotSpotIndex < arynNewlySelectedHotSpotIDs.GetSize(); nSelHotSpotIndex++) {
			//TES 2/15/2010 - PLID 37375 - This now needs a CEMRHotSpot, not an ID, so look it up
			CEMRHotSpot *pSpot = GetHotSpotFromID(arynNewlySelectedHotSpotIDs[nSelHotSpotIndex]);
			if(pSpot == NULL) {
				ASSERT(FALSE);
				AfxThrowNxException("Could not find HotSpot corresponding to ID %li", arynNewlySelectedHotSpotIDs[nSelHotSpotIndex]);
			}
			m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEmrImageHotSpotActions(pSpot, this, FALSE);
		}
	}
}

BOOL CEMNDetail::RequestContentChange(int nChangeType, int nDataID /*= -1*/, CString strOldName /*= ""*/, int nNewPos /*= -1*/)
{
	SetUnsaved();

	if(m_pParentTopic && m_pParentTopic->GetParentEMN()->GetInterface()) {
		CEmrItemEditContentInfo eieci;
		eieci.pEMNDetail = this;
		eieci.nChangeType = nChangeType;
		
		//TES 2/22/2006 - These are never sent any more.
		/*switch (nChangeType) {
		case (ITEM_DELETE): {
			eieci.nDataID = nDataID;
		}
		break;
		case (ITEM_RENAME): {
			eieci.nDataID = nDataID;
			eieci.strOldName = strOldName;
		}
		break;
		case (ITEM_REORDER): {
			eieci.nDataID = nDataID;
			eieci.nNewIndex = nNewPos;
		}
		break;
		}*/
		m_pParentTopic->GetParentEMN()->GetInterface()->SendMessage(NXM_EMR_ITEM_EDITCONTENT,
			(WPARAM)this, (LPARAM)&eieci);
		return TRUE;
	}
	else {
		return FALSE;
	}

	return FALSE;
}

/*BOOL CEMNDetail::RequestItemDelete(int nDataID) 
{
	//ACW 5/20/04 I copied this code (with a few changes) from RequestContentChange()
	if (m_pwndEMRDlg->GetSafeHwnd()) {
		LRESULT lResult = m_pwndEMRDlg->SendMessage(NXM_EMR_ITEM_DELETE, 
			(WPARAM)nDataID, 0);
		// (b.cardillo 2004-04-19 14:57) - TODO: should we reflect only when lResult is a certain value (such as TRUE or IDOK)?
		ReflectCurrentState();
		return TRUE;
	} else {
		return FALSE;
	}
}


BOOL CEMNDetail::RequestItemRename(int nDataID, CString strOldName)
{
	//ACW 5/20/04 I copied this code (with a few changes) from RequestContentChange()	
	if (m_pwndEMRDlg->GetSafeHwnd()) {
		m_pwndEMRDlg->SendMessage(NXM_EMR_ITEM_RENAME, (WPARAM)nDataID, (LPARAM)strOldName.AllocSysString());
		return TRUE;
	} else {
		return FALSE;
	}
}*/

// (a.walling 2012-04-02 08:29) - PLID 49304 - Removed a lot of dead code regarding moving items

BOOL CEMNDetail::RequestRemove()
{
	// Unlike the other ::Request*() members this one actually does nothing, it just asks the 
	// owning window to remove us.  The reason is that for any removal process, there has to 
	// be a container we are being removed FROM.

	if (!GetParentTopic()) {
		ASSERT(FALSE);
		return FALSE;
	}
	
	// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
	if(GetParentTopic()->GetTopicWnd() && IsWindow(GetParentTopic()->GetTopicWnd()->GetSafeHwnd())) {
		// (z.manning 2008-11-03 10:28) - PLID 31890 - For signature details, user must enter password
		// before removing the detail.
		if(IsSignatureDetail()) {
			if(!CheckCurrentUserPassword()) {
				return FALSE;
			}
		}

		// Ask the data collector to remove us (NOTE: Do nothing more after sending this message 
		// because it's possible we have been deallocated by the message handler!)
		return GetParentTopic()->GetTopicWnd()->SendMessage(NXM_EMR_ITEM_REMOVE, (WPARAM)this, 0);
	} else {
		// In the absense of someone telling us NOT to, we DO NOT ALLOW the change, because then nobody 
		// can know what we're being removed FROM.
		return FALSE;
	}

	return FALSE;
}

void CEMNDetail::SetAllowEdit(BOOL bAllowEdit)
{
	//TES 12/13/2006 - PLID 23770 - Don't allow them to edit if we're locked.
	if(m_pParentTopic->GetParentEMN()->GetStatus() == 2) bAllowEdit = FALSE;
	// Only do anything if the state is changing
	if ( (m_bAllowEdit && !bAllowEdit) || (!m_bAllowEdit && bAllowEdit) ) {
		// Set the internal member variable that remembers what the state is
		m_bAllowEdit = bAllowEdit;
		// Now reflect the new state on screen
		if(IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
			m_pEmrItemAdvDlg->SetCanEditContent(bAllowEdit);
			// Get the client rect in screen coordinates
			CRect rc;
			m_pEmrItemAdvDlg->GetClientRect(&rc);
			m_pEmrItemAdvDlg->ClientToScreen(&rc);
			// Convert the screen coordinates to coords relative to the window's parent
			CWnd *pParent = m_pEmrItemAdvDlg->GetParent();
			if (IsWindow(pParent->GetSafeHwnd())) {
				pParent->ScreenToClient(&rc);
			}
			// Change the styles appropriately
			if (m_bAllowEdit) {
				m_pEmrItemAdvDlg->ModifyStyleEx(WS_EX_STATICEDGE, WS_EX_CLIENTEDGE);
				m_pEmrItemAdvDlg->ModifyStyle(0, WS_THICKFRAME);
			} else {
				m_pEmrItemAdvDlg->ModifyStyleEx(WS_EX_CLIENTEDGE, WS_EX_STATICEDGE);
				m_pEmrItemAdvDlg->ModifyStyle(WS_THICKFRAME, 0);
			}
			// Invalidate in case the move doesn't change anything
			m_pEmrItemAdvDlg->Invalidate(FALSE);
			// Since the border width has changed we have to move the window so the client area is in the same place.
			m_pEmrItemAdvDlg->CalcWindowRect(&rc, CWnd::adjustOutside);
			m_pEmrItemAdvDlg->MoveWindow(rc);
		}
	}
}

BOOL CEMNDetail::GetAllowEdit() const
{
	if(!m_bAllowEdit)
		return FALSE;

	//make sure we don't allow it on a locked EMN
	CEMN *pEMN = m_pParentTopic->GetParentEMN();
	return pEMN->GetStatus() != 2;
}

// (b.cardillo 2012-03-28 21:54) - PLID 42207 - (additional) Moved this logic out into its own function for use elsewhere (by this pl item)
//TES 7/7/2012 - PLID 50854 - This now takes the full narrative text, not just the header, and works for RTF or HTML
BOOL CEMNDetail::IsStateSet_Narrative(const CString &strNarrativeText)
{
	//TES 7/24/2006 - The narrative is incomplete if there are any empty fields on it, otherwise it's complete.
	CArray<NarrativeField> arFields;
	//TES 6/29/2012 - PLID 50854 - New function that handles HTML narratives
	// (j.armen 2012-12-03 11:48) - PLID 52752 - Try to determine if the merge fields are valid
	GetNarrativeFieldArray(strNarrativeText, arFields);

	for(int i = 0; i < arFields.GetSize(); i++) {
		//TES 8/1/2006 - If the field is empty, don't count it as needing to be filled in.
		NarrativeField nf = arFields[i];
		if(nf.nStart == -1) {
			//TES 7/6/2012 - PLID 50854 - Let's assume that if the field name is the same as the value, then the value is
			// actually empty and we're just displaying the field. Set bIsValid to distinguish this case from fields that
			// are placeholders for details not on the EMN.  I entered PLID 51428 to handle this better.
			// (j.armen 2012-12-03 15:02) - PLID 52752 - I have moved Tom's comment here, as this is the last location that
			//	may needs to handle determining if the field is valid
			if(nf.strValue == nf.strField) {
				nf.strValue = "";
				nf.bIsValid = true;
			}

			//TES 7/7/2012 - PLID 50854 - HTML narrative
			if(nf.bIsValid && nf.strValue.IsEmpty()) {
				return FALSE;
			}
		}
		else {
			if(arFields[i].nEnd != arFields[i].nStart && arFields[i].strValue.IsEmpty()) return FALSE;
		}
	}
	return TRUE;
}

// (a.walling 2009-01-13 14:57) - PLID 32107 - EmrUtils' IsDetailStateSet has been moved into this function
// (z.manning 2011-10-07 10:49) - PLID 45664 - Added bForOutput
BOOL CEMNDetail::IsStateSet(const _variant_t *pOverrideState /*= NULL*/, bool bForOutput /* = false */)
{
	//TES 7/24/2006 - The narrative used to try to access the interface here; that is no longer necessary.
	const _variant_t& varState(pOverrideState ? *pOverrideState : m_varState);

	// (a.walling 2008-03-24 10:25) - PLID 28811 - Return if state does not match item type
	if (!DataTypeMatchesState(m_EMRInfoType, varState.vt))
		return FALSE;

	switch (varState.vt) {
		case VT_EMPTY:
		case VT_NULL:
			// Definitely not set
			return FALSE;
			break;
		case VT_BSTR:
			{
				ASSERT(m_EMRInfoType == 1 || m_EMRInfoType == 2 || m_EMRInfoType == 3 || m_EMRInfoType == 6 || m_EMRInfoType == 7);
				CString str = VarString(varState);
				str.TrimRight();
				if(m_EMRInfoType == eitNarrative) {
					// (b.cardillo 2012-03-28 21:54) - PLID 42207 - (additional) Moved the code here to its own function
					//TES 7/7/2012 - PLID 50854 - This now takes the full narrative text, not just the header, and works for RTF or HTML
					return CEMNDetail::IsStateSet_Narrative(str);
				}
				else if(m_EMRInfoType == 7) {
					//table
					// (a.walling 2010-03-29 17:48) - PLID 27906 - Use the table state iterator to parse through table states
					if (!str.IsEmpty()) {
						CEmrTableStateIterator etsi(str);
						long X,Y,nEmrDetailImageStampID,nEmrDetailImageStampPointer,nStampID;
						CString strData;
						// (c.haag 2007-08-25 11:44) - PLID 27112 - Use the table state iterator class
						// (z.manning 2010-02-18 09:45) - PLID 37427 - Added EmrDetailImageStampID
						while(etsi.ReadNextElement(X,Y,strData,nEmrDetailImageStampID,nEmrDetailImageStampPointer,nStampID)) {
							// (z.manning, 03/26/2007) - PLID 25347 - We had already been checking for 0 here, but now also
							// check for -1 which can happen if someone selects the blank entry in a dropdown column.
							// (a.walling 04/12/2011) - PLID 43273 - We need to take into account the possibility of text columns
							// containing the values of 0 (and for completeness, -1)
							if(!strData.IsEmpty()) {
								if (strData == "-1" || strData == "0") {
									TableColumn* pCol = GetColumnByID(Y);
									if (pCol && pCol->nType == LIST_TYPE_TEXT) {
										return TRUE;
									}
								} else {
									return TRUE;
								}
							}
						}
					}
					//we found nothing, return as such
					str = "";
				}
				
				if (str.IsEmpty()) {
					// The variant was set but it was an empty string
					return FALSE;
				} else {
					// The variant was set to a non-empty string
					return TRUE;
				}
			}
			break;
		case VT_ARRAY|VT_UI1:
			{
				ASSERT(m_EMRInfoType == 4);
				try {
					CEmrItemAdvImageState ais;
					ais.CreateFromSafeArrayVariant(varState);
					
					// (a.walling 2009-01-13 14:57) - PLID 32107 - Wait! The image path 'override' is more of a misnomer. It will always
					// be the path that the image detail is currently using. We want to return FALSE if it matches the info item's
					// default paths and has no data.

					//if (!ais.m_strImagePathOverride.IsEmpty() || 
						// Something about the image state is SET

					if(bForOutput && Is3DImage())
					{
						// (z.manning 2011-10-07 10:56) - PLID 45664 - If this is a 3D image and they're checking this
						// for output purposes then return false if we don't have any snapshots as that means we have nothing
						// to output.
						CImageArray ary3DSnapshots;
						Get3DImageOutputData(&ary3DSnapshots);
						if(ary3DSnapshots.IsEmpty()) {
							return FALSE;
						}
					}

					if ( (ais.m_varInkData.vt != VT_NULL && ais.m_varInkData.vt != VT_EMPTY)  // has ink data
						|| (ais.m_varTextData.vt != VT_NULL && ais.m_varTextData.vt != VT_EMPTY) // has text data
						|| (!ais.m_strSelectedHotSpotData.IsEmpty()) // (a.walling 2009-01-13 15:47) - PLID 32107 - has hot spot selections
						) {
						return TRUE;
					} else {
						_ASSERTE(m_varInfoBackgroundImageFilePath.vt != VT_EMPTY);
						_ASSERTE(m_varInfoBackgroundImageType.vt != VT_EMPTY);

						if (m_varInfoBackgroundImageFilePath.vt == VT_EMPTY || m_varInfoBackgroundImageType.vt == VT_EMPTY) {
							LogDetail("Detail info item image paths not set!");
							return TRUE;
						}

						CString strInfoBackgroundFilePath = VarString(m_varInfoBackgroundImageFilePath, "");
						long nInfoBackgroundType = VarLong(m_varInfoBackgroundImageType, -1);

						// (d.thompson 2009-03-05 11:14) - PLID 32891 - Rule change for forced blank images.  If the default
						//	info item has an image, and we force it blank, that counts as completed (existing code handles this
						//	by the "did anything change" rule.  However, if the image has no default, we add an image, then 
						//	blank it, it should go back to the "clear" state.  We'll have to detect that specifically.
						if(nInfoBackgroundType == itUndefined && ais.m_eitImageTypeOverride == itForcedBlank) {
							return FALSE;
						}

						if (nInfoBackgroundType != ais.m_eitImageTypeOverride) {
							// the type has changed, so something must have been set.
							return TRUE;
						} else if (strInfoBackgroundFilePath.CompareNoCase(ais.m_strImagePathOverride) != 0) {
							// the file paths do not match; must have been set
							return TRUE;
						} else {
							// no text data, no ink data, and the background image has not been modified
							// from the info item's defaults
							return FALSE;
						}
					}
				} catch (...) {
					// This shouldn't be possible, but we really don't want exceptions to pass out to the caller 
					// and it's CONCEIVABLE that the varState that was given to us actually didn't come from a 
					// CEmrItemAdvImageState.
					ASSERT(FALSE);
					// We don't understand this variant array, so we should return TRUE (that it's set)
					return TRUE;
				}
			}
			break;
		case VT_R4:
			ASSERT(m_EMRInfoType == 5);
			return TRUE;
			break;
		default:
			// For now any other type we consider to be set
			return TRUE;
			break;
	}
}

BOOL CEMNDetail::IsModified() const
{
	return m_bModified;
}

// (c.haag 2008-11-12 11:37) - PLID 31693 - We now return whether the item was popped up, and
// the result of the popup.
BOOL CEMNDetail::Popup(CWnd* pOverrideParent /* = NULL */, INT_PTR* pnResult /* = NULL */)
{
	// (c.haag 2007-10-24 11:45) - PLID 27562 - Have additional error checking and
	// logging for pinning down problems rooted in one of many possible causes
	try {
		INT_PTR nResult;

		if(!m_pParentTopic) {
			//We can't pop up until we're attached to a topic.
			return FALSE;
		}

		// (j.jones 2007-07-17 10:10) - PLID 26702 - Added pSourceEMN as a parameter later used
		// in AddEMNFromTemplate, such that it can pull demographics from our current EMN,
		// instead of populating our dummy, placeholder EMN via recordsets. But we also
		// apply some EMN properties externally from this function, so I changed references
		// to m_pParentTopic->GetParentEMN() to now just use pSourceEMN
		CEMN *pSourceEMN = m_pParentTopic->GetParentEMN();

		if(!pSourceEMN) {
			//We can't pop up until we're attached to an EMN.
			return FALSE;
		}

		CWnd* pWnd = NULL;
		//TES 1/29/2008 - PLID 24157 - Use our override, if we were given one.
		if(pOverrideParent) {
			pWnd = pOverrideParent;
		}
		else {
			pWnd = pSourceEMN->GetParentEMR()->GetInterface();
		}
		if(pWnd->GetSafeHwnd() == NULL || !IsWindow(pWnd->GetSafeHwnd())) {
			//DRT 4/3/2006 - PLID 19961 - This was popping up in situations it shouldn't (such as the export), so we're
			//	going to check for the accuracy of the interface - if it exists, popups are OK, if it does not exist, then
			//	we must not be in an EMR.
			return FALSE;
		}

		// (a.walling 2007-06-21 17:18) - PLID 22097 - Help figure out if we are readonly or not.
		BOOL bLocked = pSourceEMN->GetStatus() == 2;
		// (a.walling 2007-11-28 11:20) - PLID 28044 - Check for expiration
		// (a.walling 2008-06-09 13:16) - PLID 22049 - Also include EMN writable status with bCanWrite
		BOOL bCanWrite = CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) && (g_pLicense->HasEMR(CLicense::cflrSilent) == 2) && pSourceEMN->IsWritable();

		BOOL bSpawnedDetail = (pSourceEMN->IsTemplate() && (m_pParentTopic->GetSourceActionID() != -1));

		if(m_EMRInfoType == eitNarrative) {
			// (a.walling 2007-06-21 17:20) - PLID 22097 - We can't be sure that the readonly state is correct unless the topicwnd exists.
			// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
			if (!m_pParentTopic->GetTopicWnd()) {
				SetReadOnly((bLocked || !bCanWrite || bSpawnedDetail) ? TRUE : FALSE);
			}
			//TES 4/6/2007 - PLID 25456 - Narratives can now be popped up, their case is actually much simpler.  Rather than
			// copying ourselves, passing in the copy, and then getting information out of that copy, we just pass in ourselves,
			// and then our code will handle whatever state changes, etc., need to happen.
			// (a.walling 2007-10-09 14:23) - PLID 27703 - Use the interfacewnd as the parent of the popup
			CEMRItemAdvPopupDlg dlg(pWnd);
			// (a.walling 2008-01-23 13:09) - PLID 14982 - Pass the RealDetail (which is also this)
			dlg.SetDetail(this, FALSE, this);
			m_pPopupDlg = &dlg;

			//TES 4/11/2007 - PLID 25456 - However, we DO need to tell our EMN that one of its details is popped up.
			// (a.walling 2007-08-22 14:51) - PLID 27160 - It is now possible that multiple items can be popped up.
			pSourceEMN->AddPoppedUpDetail(this);
			this->m_bIsPoppedUp = TRUE;
			nResult = dlg.DoModal();
			this->m_bIsPoppedUp = FALSE;
			m_pPopupDlg = NULL;
			//Now tell the EMN we're NOT popped up any more.
			// (a.walling 2007-08-22 14:52) - PLID 27160 - now restore the previously existing popped up detail.
			pSourceEMN->RemovePoppedUpDetail(this);
			// (c.haag 2008-11-12 11:46) - PLID 31693 - Preserve the modal result
			if (pnResult) { *pnResult = nResult; }
			return TRUE;
		}

		// (j.jones 2010-03-17 10:08) - PLID 37318 - track the smartstamp links
		// (z.manning 2011-01-20 12:10) - PLID 42338 - Support multiple images per smart stamp table
		CEMNDetailArray arySmartStampImageDetails;
		arySmartStampImageDetails.Copy(m_arySmartStampImageDetails);
		CEMNDetail *pSmartStampTableDetail = m_pSmartStampTableDetail;
		//add a reference to make sure the Popup can't cause it to be destroyed
		arySmartStampImageDetails.AddRefAll("CEMNDetail::Popup() - holding memory of old image pointer");
		if(pSmartStampTableDetail) {
			pSmartStampTableDetail->__AddRef("CEMNDetail::Popup() - holding memory of old table pointer");
		}

		// (a.walling 2007-10-09 14:23) - PLID 27703 - Use the interfacewnd as the parent of the popup
		CEMRItemAdvPopupDlg dlg(pWnd);
		// (j.jones 2008-09-22 12:07) - PLID 29040 - the details get instantiated poorly if not created
		// with the topic they belong on
		// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
		CEMNDetail *pCopy = CEMNDetail::CreateDetail(m_pParentTopic, "Popup copy");
		pCopy->CopyFrom(*this); // (a.walling 2012-10-31 17:17) - PLID 53550 - CEMNDetail::opreator= leads to very confusing semantics
		// (a.walling 2007-06-21 16:55) - PLID 22097 - Also set the readonly status
		// (since the detail's is not set until it is displayed)
		// (j.jones 2008-09-10 13:22) - PLID 31304 - recalculate always, not only
		// when it is FALSE
		//if (pCopy->GetReadOnly() == FALSE) {
			// calculate the readonly status
			pCopy->SetReadOnly((bLocked || !bCanWrite || bSpawnedDetail) ? TRUE : FALSE);
		//}

		//Make a placeholder EMR with the correct patient, and put this copy in it.
		CEMR emr;
		emr.CreateNew(pSourceEMN->GetParentEMR()->GetPatientID(), FALSE);	

		// (j.jones 2007-07-24 10:59) - PLID 26742 - set the current medication/allergy IDs,
		// so we don't have to query the database
		emr.SetCurrentMedicationsInfoID(pSourceEMN->GetParentEMR()->GetCurrentMedicationsInfoID());
		emr.SetCurrentAllergiesInfoID(pSourceEMN->GetParentEMR()->GetCurrentAllergiesInfoID());
		emr.SetCurrentGenericTableInfoID(pSourceEMN->GetParentEMR()->GetCurrentGenericTableInfoID());

		//TES 4/6/2007 - PLID 25456 - We pass in TRUE here to let the dialog know that (unlike for narratives), the detail
		// it has is a copied detail, so it can allow the user to Cancel, and also do things like permanently lock spawning on it.
		// (a.walling 2008-01-23 13:09) - PLID 14982 - Pass the RealDetail (this)
		// (z.manning 2010-03-18 09:07) - PLID 37318 - Because of linked details for smart stamps there were potential
		// pitfalls with EMNDetail reference counting that required special handling in order to propery ensure that
		// the linked details were being released appropriately. What had been happening in this function is that when
		// popping up a smart stamp table, the below call to AddDetail() would lead to swapping around the smart stamp pointer
		// which sometimes led to the special case where the table was unlinking itself from the smart stamp image because
		// the table was down to its last reference. That really should not have happened as the popup is using this detail
		// and should have a reference to it. The place where it adds that reference is the SetDetail function right here.
		// I simply moved this call to be BEFORE the call to AddDetail to resolve these issue.
		dlg.SetDetail(pCopy, TRUE, this);

		// (j.jones 2007-07-17 10:14) - PLID 26702 - uses pSourceEMN as a parameter now
		SourceActionInfo saiBlank; // (z.manning 2009-03-04 15:25) - PLID 33338
		// (c.haag 2009-04-06 09:48) - PLID 33859 - Last parameter of AddDetail should be false -- we don't need to try to 
		// sync this detail with the patient's official current medication information. We already have the content and state
		// copied from the source detail; it should not change.
		//TES 4/15/2010 - PLID 24692 - Pass in -1 as the topic ID.
		// (a.walling 2013-01-22 10:00) - PLID 54762 - Emr Appointment linking
		emr.AddEMNFromTemplate(-1, saiBlank, pSourceEMN, pSourceEMN ? pSourceEMN->GetAppointment().nID : -1)->AddTopic("", -1)->AddDetail(pCopy, FALSE, TRUE, FALSE);
		pCopy->m_pOriginalParentTopic = NULL;
		// (j.jones 2010-01-22 16:54) - PLID 36570 - set the EMN date on this placeholder EMN
		emr.GetEMN(0)->SetEMNDate(pSourceEMN->GetEMNDate());
		// (a.walling 2007-06-21 16:49) - PLID 22097 - Set the locked status on this placeholder emn
		// (j.jones 2011-07-05 11:36) - PLID 43603 - this now takes in a class
		// (a.walling 2012-04-06 12:13) - PLID 49496 - EMN Status - GetStatus now returns the object
		emr.GetEMN(0)->SetStatus(pSourceEMN->GetStatus());
		// (z.manning 2010-02-26 18:20) - PLID 37230 - Flag that the detail copy is a popped up detail
		pCopy->m_bIsPoppedUp = TRUE;

		//Copy dialog information.
		if(m_EMRInfoType == eitSingleList || m_EMRInfoType == eitMultiList) {
			// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
			EnsureEmrItemAdvDlg(GetParentTopic()->GetTopicWndRaw());
			dlg.m_clrHilightColor = ((CEmrItemAdvListDlg*)m_pEmrItemAdvDlg)->m_clrHilightColor;
			if(dlg.m_clrHilightColor == 0) {
				// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
				dlg.m_clrHilightColor = EmrColors::Topic::Complete();
			}
		}
		else if(m_EMRInfoType == eitImage) {
			if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
				// (j.armen 2014-07-23 11:19) - PLID 62836 - Methods for getting Pen Color / Size
				CEmrItemAdvImageDlg* pItemAdvImageDlg = dynamic_cast<CEmrItemAdvImageDlg*>(m_pEmrItemAdvDlg);
				if(pItemAdvImageDlg->m_pDetail->Is2DImage())
				{
					dlg.m_nCurPenColor = pItemAdvImageDlg->GetCurrentPenColor();
					dlg.m_fltCurPenSize = pItemAdvImageDlg->GetCurrentPenSize();
				}
				dlg.m_bIsEraserState = pItemAdvImageDlg->m_bIsEraserState;
			}
			else {
				//TES 5/3/2006 - PLID 20423 - We still want to initialize the pen color.
				dlg.m_nCurPenColor = GetRemotePropertyInt("DefaultEMRImagePenColor",RGB(255,0,0),0,GetCurrentUserName(),TRUE);
			}
		}
		else if(m_EMRInfoType == eitTable) {
			//We have to load this, since the copy doesn't have a valid EMN to copy it from.
			dlg.m_strLinkedItemList = pSourceEMN->GetTableItemList();

			//DRT 7/11/2007 - PLID 24105 - We must copy the column sizes to the popup window
		}

		// (c.haag 2006-08-09 12:37) - PLID 21877 - We now remember the original state of the detail, unless it's an image.
		_variant_t varOriginalState;
		//TES 5/15/2008 - PLID 27776 - We can now compare on images just like any other type, so always get the original state.
		//if (m_EMRInfoType != eitImage) {
			varOriginalState = dlg.GetDetail()->m_varState;
		//}
		

		// (c.haag 2007-10-12 15:58) - PLID 27599 - If this is an image detail, then it's possible
		// for other image details on the EMN, regardless of whether they are hidden, to steal
		// ink strokes from the pop-up image. Go through every detail in the entire EMN to look
		// for image details with valid window handles, and disable their ink inputs.
			// (c.haag 2008-06-04 13:18) - PLID 27777 - We only need to go through details in the parent topic
		const long nEMNTotalDetailCount = pSourceEMN->GetTotalDetailCount();
		long nDetail;
		if (eitImage == m_EMRInfoType) {
			const long nDetailCount = m_pParentTopic->GetEMNDetailCount();
			for (nDetail=0; nDetail < nDetailCount; nDetail++) {
				CEMNDetail *p = m_pParentTopic->GetDetailByIndex(nDetail);
				if (NULL != p && eitImage == p->m_EMRInfoType && NULL != p->m_pEmrItemAdvDlg &&
					IsWindow(p->m_pEmrItemAdvDlg->GetSafeHwnd())) 
				{
					((CEmrItemAdvImageDlg*)p->m_pEmrItemAdvDlg)->EnableInkInput(FALSE);
				}
			}
		}

		// (z.manning 2011-01-05 16:32) - PLID 42002 - When we added the copy of the current detail above it
		// likely resulted in the smart stamp image or table getting mislinked to that new copy. Because
		// it is possible for the current topic to be saved while this detail is popped up (such as when
		// you edit the info item), we need to ensure the proper links are restored before we pop up this detail.
		ReconfirmSmartStampLinks();

		nResult = dlg.DoModal();

		// (j.jones 2010-03-17 10:08) - PLID 37318 - reset the smartstamp links
		if(m_EMRInfoType == eitTable) {
			SetSmartStampImageDetails(arySmartStampImageDetails);
		}
		if(m_EMRInfoType == eitImage) {
			SetSmartStampTableDetail(pSmartStampTableDetail);
		}

		//de-reference to make sure the Popup can't cause it to be destroyed
		// (z.manning 2011-01-20 12:56) - PLID 42338 - Support multiple images per smart stamp table
		arySmartStampImageDetails.ReleaseAll("CEMNDetail::Popup() - releasing memory of old image pointer");
		if(pSmartStampTableDetail) {
			pSmartStampTableDetail->__Release("CEMNDetail::Popup() - releasing memory of old table pointer");
			pSmartStampTableDetail = NULL;
		}

		ReconfirmSmartStampLinks();

		// (c.haag 2007-10-12 16:00) - PLID 27599 - If this is an image detail, then it's possible
		// for other image details on the EMN, regardless of whether they are hidden, to steal
		// ink strokes from the pop-up image. Go through every detail in the entire EMN to look
		// for image details with valid window handles, and re-enable their ink inputs. This is
		// the only function in the program that calls EnableInkInput, so we don't need to check
		// the previous states because we know they're disabled. (Also, don't recalculate
		// nEMNTotalDetailCount; image pop-ups cannot spawn details in DoModal)
		// (c.haag 2008-06-04 13:18) - PLID 27777 - We only need to go through details in the parent topic
		if (eitImage == m_EMRInfoType) {
			const long nDetailCount = m_pParentTopic->GetEMNDetailCount();
			for (nDetail=0; nDetail < nDetailCount; nDetail++) {
				CEMNDetail *p = m_pParentTopic->GetDetailByIndex(nDetail);
				if (NULL != p && eitImage == p->m_EMRInfoType && NULL != p->m_pEmrItemAdvDlg &&
					IsWindow(p->m_pEmrItemAdvDlg->GetSafeHwnd())) 
				{
					((CEmrItemAdvImageDlg*)p->m_pEmrItemAdvDlg)->EnableInkInput(TRUE);
				}
			}
		}

		// (j.jones 2007-10-16 11:35) - PLID 27775 - don't attempt to process any changes if read only
		if (nResult != IDCANCEL && !m_bReadOnly) {
			//now process the changes
			if(m_EMRInfoType == eitImage) {
				//TES 10/12/2007 - PLID 23816 - The SizeImageToDetail property may have been changed while the image
				// was popped up, so pull it from the copied detail.
				// (a.walling 2011-05-25 17:57) - PLID 43847 - This is now no longer a property of the detail.
				//SetSizeImageToDetail(dlg.GetDetail()->GetSizeImageToDetail());
				if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
					// (j.armen 2014-07-23 11:19) - PLID 62836 - Methods for getting Pen Color / Size
					CEmrItemAdvImageDlg* pItemAdvImageDlg = dynamic_cast<CEmrItemAdvImageDlg*>(m_pEmrItemAdvDlg);
					if (pItemAdvImageDlg->m_pDetail->Is2DImage())
					{
						pItemAdvImageDlg->SetCurrentPenColor(dlg.m_nCurPenColor);
						pItemAdvImageDlg->SetCurrentPenSize(dlg.m_fltCurPenSize);
					}
					pItemAdvImageDlg->m_bIsEraserState = dlg.m_bIsEraserState;
					// (a.wetta 2007-04-10 10:07) - PLID 25532 - Be sure to update the custom stamps if they were changed in the pop-up
					pItemAdvImageDlg->SetCustomStamps(dlg.m_strCustomImageStamps);
				}
				else {
					// (a.wetta 2007-04-17 16:29) - PLID 25532 - Refresh the custom stamps in the other images in case they have changed
					// even if the EmrItemAdvDlg does not exist yet.  If it had existed, then this would have been done
					// through SetCustomStamps above.
					//SetPropertyMemo("EMR_Image_Custom_Stamps", dlg.m_strCustomImageStamps, 0);
					// (a.walling 2008-12-19 09:27) - PLID 29800 - Refresh custom stamps only
					// (j.jones 2010-02-10 14:59) - PLID 37224 - stamps are already in memory,
					// just refresh
					// (z.manning 2011-10-25 15:31) - PLID 39401 - There is no need to call this here. If stamps changed while
					// the image was popped up we already updated the entire EMR.
					//((CEmrTreeWnd*)pWnd)->RefreshContentByType(eitImage, FALSE, TRUE, TRUE);
				}

				// (j.armen 2014-07-23 10:37) - PLID 62836 - If either default pen size or color changed, reflect the change
				if (m_nDefaultPenSizePercent != dlg.GetDetail()->m_nDefaultPenSizePercent
					|| m_nDefaultPenColor != dlg.GetDetail()->m_nDefaultPenColor)
				{
					m_nDefaultPenSizePercent = dlg.GetDetail()->m_nDefaultPenSizePercent;
					m_nDefaultPenColor = dlg.GetDetail()->m_nDefaultPenColor;
					SetUnsaved();
				}

				//TES 5/15/2008 - PLID 27776 - Get the original and new CEmrItemAdvImageStates, and compare them.  If they're
				// identical, don't request a state change (which would incorrectly flag this detail as modified).
				CEmrItemAdvImageState eiaisOriginal;
				eiaisOriginal.CreateFromSafeArrayVariant(varOriginalState);
				CEmrItemAdvImageState eiaisNew;
				eiaisNew.CreateFromSafeArrayVariant(dlg.GetDetail()->m_varState);
				if( !(eiaisOriginal == eiaisNew) ) {
					// (a.walling 2011-06-08 11:54) - PLID 43847 - If the image changed in the popup, we may need to resize if SizeImageToDetail is turned off.
					bool bImageChanged = ( (eiaisOriginal.m_strImagePathOverride != eiaisNew.m_strImagePathOverride) || (eiaisOriginal.m_eitImageTypeOverride != eiaisNew.m_eitImageTypeOverride) );
					RequestStateChange(dlg.GetDetail()->m_varState);
					SetState(dlg.GetDetail()->m_varState);

					if (bImageChanged && !GetRemotePropertyInt("EMR_SizeImageToDetail", 1, 0, GetCurrentUserName(), true)) {
						if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
							((CEmrItemAdvImageDlg*)m_pEmrItemAdvDlg)->RestoreIdealSize(true);
						}
					}
				}
			} else {
				// (c.haag 2006-08-09 12:39) - PLID 21877 - If the original and new states match,
				// then do not request a state change.
				if (varOriginalState != dlg.GetDetail()->m_varState) {
					if(m_EMRInfoType == eitTable) {
						// (z.manning 2009-03-04 11:10) - PLID 33072 - Table states get recreated before
						// this is called, so we need to pass in the old state for spawning purposes.
						// (z.manning 2009-03-31 09:19) - PLID 33072 - I had originally used varOriginalState
						// as the old state here, but that won't work if they edited the item while it was
						// popped up because the row and column IDs (and thus the state) may have changed.
						// Instead, use the current state because the table state that gets recreated is that
						// of the copy of the detail we made for the popup dialog so in this case m_varState
						// is still the old table state which is what we want.
						RequestStateChange(dlg.GetDetail()->m_varState, m_varState);
					}
					else {
						RequestStateChange(dlg.GetDetail()->m_varState);
					}
					SetState(dlg.GetDetail()->m_varState);
				}
			}

			if(m_EMRInfoType == eitTable) {
				// (r.gonet 02/14/2013) - PLID 40017 - Since the popup's column widths can now be
				//  saved separately, we must copy them back so they can be saved later when we generate
				//  the save string.
				CTableColumnWidths *pPopupColumnWidths = dlg.GetDetail()->GetColumnWidths();

				// (r.gonet 02/14/2013) - PLID 40017 - Have the column widths changed?
				if(!this->GetColumnWidths()->Equals(pPopupColumnWidths)) {
					// (r.gonet 02/14/2013) - PLID 40017 - Copy back the widths since we now remember the popup table column widths separately
					this->GetColumnWidths()->ClearAll();
					this->GetColumnWidths()->CopyFrom(pPopupColumnWidths);
					this->SetSaveTableColumnWidths(dlg.GetDetail()->GetSaveTableColumnWidths());

					// (r.gonet 02/14/2013) - PLID 40017 - Notify the appropriate parties that the detail has changed and needs saved.
					if(this->m_pParentTopic->GetParentEMN()->GetStatus() != 2 && this->m_pParentTopic->GetParentEMN()->IsWritable()) {
						this->SetUnsaved();
						this->m_pParentTopic->SetUnsaved();
						CEMN *pEMN = GetParentEMN();
						if(pEMN) {
							CEmrTreeWnd *pTree = pEMN->GetInterface();

							// (r.gonet 02/14/2013) - PLID 40017 - find out if we can/should save
							if(pTree != NULL && !pEMN->IsLockedAndSaved() && !pEMN->IsLoading()) {
								pTree->SendMessage(NXM_EMR_ITEM_CHANGED, (WPARAM)this);	
							}
						}
					}
				}
			}
		} else {
			// (a.walling 2008-12-19 09:27) - PLID 29800 - Regardless, make sure the custom image stamps are up to date
			if(m_EMRInfoType == eitImage) {
				//SetPropertyMemo("EMR_Image_Custom_Stamps", dlg.m_strCustomImageStamps, 0);
				// (j.jones 2010-02-10 14:59) - PLID 37224 - stamps are already in memory,
				// just refresh
				// (z.manning 2011-10-25 15:31) - PLID 39401 - There is no need to call this here. If stamps changed while
				// the image was popped up we already updated the entire EMR.
				//((CEmrTreeWnd*)pWnd)->RefreshContentByType(eitImage, FALSE, TRUE, TRUE);
			}
		}
		
		if(nResult != IDCANCEL && m_EMRInfoType == eitImage) {
			// (r.gonet 02/14/2012) - 37682 - Always copy the filter back. It changes no data so it is safe to use on readonly and locked EMNs
			this->SetImageTextStringFilter(dlg.GetDetail()->GetImageTextStringFilter());
			this->ReflectImageTextStringFilter();
		}

		emr.GetEMN(0)->GetTopic(0)->DetachDetail(pCopy, TRUE); // (a.walling 2009-10-13 14:49) - PLID 36024 - We should release the topic's reference
		//delete pCopy;
		// (a.walling 2009-10-12 17:45) - PLID 36024
		pCopy->__Release("Popup release copy");

		// (c.haag 2008-11-12 11:46) - PLID 31693 - Preserve the modal result
		if (pnResult) { *pnResult = nResult; }

		// (j.jones 2012-10-03 16:27) - PLID 36220 - if a "blue" current meds. or allergies table,
		// check if the drug interaction preference is set to force a save
		if(m_EMRInfoType == eitTable && !m_bIsTemplateDetail && IsModified()) {
			CEmrItemAdvTableDlg *pAdvDlg = (CEmrItemAdvTableDlg*)m_pEmrItemAdvDlg;
			if(pAdvDlg != NULL && (pAdvDlg->GetIsActiveCurrentMedicationsTable() || pAdvDlg->GetIsActiveAllergiesTable())) {
				
				//this is a blue table, and it changed
				CEMN *pEMN = GetParentEMN();
				if(pEMN) {
					CEmrTreeWnd *pTree = pEMN->GetInterface();

					//find out if we can/should save
					if(pTree != NULL && !pEMN->IsLockedAndSaved()
						&& pTree->GetEMRDrugInteractionChecksPref() == edictSaveWarnWhenMedsChange) {

						//find out if this is the newest EMN
						COleDateTime dtNewestEMNDate = GetLatestEMNDateForPatient(pEMN->GetParentEMR()->GetPatientID());

						// We will not update patient allergies or current meds unless there is no EMN
						// on the patient's account with a newer date. If there was another EMN with
						// the same date, then that's fine. We're simply reducing this update ability
						// to only occur on the most recent EMNs.
						BOOL bCanSave = FALSE;
						if(dtNewestEMNDate.GetStatus() == COleDateTime::invalid) {
							//this means that we're saving the only EMN on the account
							bCanSave = TRUE;
						}
						else {
							COleDateTime dtEMNDate = pEMN->GetEMNDate();					
							//don't need to revert to date-only because it should already be that way and if it's the same date
							//and a greater time than the date-only dtNewestEMNDate, that's perfectly fine
							if(dtEMNDate >= dtNewestEMNDate) {
								bCanSave = TRUE;
							}
						}
						if(!pEMN->IsTemplate() && bCanSave && !pEMN->IsLoading()) {
							//We know now that the blue table changed, and we want changes to save.
							//It may or may not actually show drug interactions.
							// (j.jones 2012-11-13 10:10) - PLID 52869 - changed to be a posted message
							// (z.manning 2013-09-17 15:36) - PLID 58450 - New function for this
							pEMN->CheckSaveEMNForDrugInteractions(FALSE);
						}
					}
				}
			}
		}

		return TRUE;

	} NxCatchAllSilentCallThrow(
		ASSERT(FALSE);
		Log("An exception was thrown from CEMNDetail::Popup. The exception will quietly be passed to the calling function.");
	);
	this->m_bIsPoppedUp = FALSE;
	return FALSE;
}

//called by CEMRTopic to show the EMNDetail's EMRItemAdvDlg
void CEMNDetail::ShowDetailDlg() const
{
	ASSERT(m_pEmrItemAdvDlg);
	if(m_pEmrItemAdvDlg) {
		if(IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
			m_pEmrItemAdvDlg->ShowWindow(SW_SHOW);
		}
	}
}

//called by CEMRTopic to hide the EMNDetail's EMRItemAdvDlg
void CEMNDetail::HideDetailDlg() const
{
	if(m_pEmrItemAdvDlg) {
		if(IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
			m_pEmrItemAdvDlg->ShowWindow(SW_HIDE);
		}
	}
}

//DRT 2/24/2006 - PLID 19465 - This function is ONLY for use in generating the string to save.  This function should NOT
//	be changing any member variables under the assumption that the save succeeded.  All code that needs to be updated after
//	the save succeeds should be placed in PostSaveUpdate()
// (j.jones 2007-01-11 14:28) - PLID 24027 - tracked strPostSaveSql, for sql statements to occur after the main save
// (c.haag 2007-06-20 12:38) - PLID 26397 - We now store saved objects in a map for fast lookups
// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize
Nx::Quantum::Batch CEMNDetail::GenerateSaveString(long nEMNTopicID, long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN OUT CMapPtrToPtr& mapSavedObjects, CDWordArray &arynModifiedEMRDetailIDs, BOOL bIsTopLevelSave)
{
	Nx::Quantum::Batch strSaveString;

	// (b.cardillo 2004-05-25 13:26) - We used to try to skip anything that was empty or null.  Now 
	// we just assert that it's not empty (because empty means invalid, whereas null or emptystring 
	// or whatever else is now valid).
	ASSERT(m_varState.vt != VT_EMPTY);

	if(m_bModified) {

		// (c.haag 2013-04-26) - PLID 56450 - Ensure the HTML is XHTML compliant for HTML narratives. It's important to do it
		// right now before any saving or auditing; or anything else goes on that uses the detail state. (For the record, I don't
		// like changing the state GenerateSaveString, but it's necessary right now).
		if (eitNarrative == m_EMRInfoType && NULL != m_pEmrItemAdvDlg && !IsRtfNarrative()) {
			((CEmrItemAdvNarrativeDlg*)m_pEmrItemAdvDlg)->ValidateHtml();
		}

		if(m_bIsTemplateDetail) {
			//save a template detail record

			strSaveString += GenerateSaveString_Template(nEMNTopicID, nAuditTransactionID, strPostSaveSql, arystrErrors, mapSavedObjects);
		}
		else {
			//save a patient EMN detail record

			// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
			strSaveString += GenerateSaveString_PatientEMN(nEMNTopicID, nAuditTransactionID, strPostSaveSql, arystrErrors, mapSavedObjects, arynModifiedEMRDetailIDs);
		}
	}

	//We have now had our save string generated.
	mapSavedObjects[this] = this;

	return strSaveString;
}

//DRT 2/24/2006 - PLID 19465 - This is a notification function that lets us know a save has just taken place.  This function
//	should contain any code that updates members, etc based on the successful save, not GenerateSaveString()
// (a.walling 2007-10-18 16:40) - PLID 27664 - Added array to gather all topics affected in the PostSaveUpdate cascade.
void CEMNDetail::PostSaveUpdate(BOOL bTopLevelUpdate /*= FALSE*/, CArray<CEMRTopic*, CEMRTopic*> *parTopicsAffected /*= NULL*/)
{
	//if this is the top level update, we need to update our parent
	if(bTopLevelUpdate) {
		ASSERT(m_pParentTopic); //We shouldn't be saving until we're attached to a topic.
		// (a.walling 2007-10-18 16:40) - PLID 27664 - Pass the array to gather all affected topics
		m_pParentTopic->PostSaveUpdate(FALSE, TRUE, parTopicsAffected);
	}


	///////////////
	//Put all updating code here

	// (c.haag 2008-07-15 15:52) - PLID 17244 - If the label text had changed, it also means we've updated the notes of any corresponding
	// todo alarms in the batch coming from GenerateSaveString. Here, we send table checker messages so everyone else is made aware of the
	// todo alarm changes.
	if (m_nEMRDetailID > 0 && m_oLastSavedDetail && m_oLastSavedDetail->GetLabelText() != GetLabelText()) {
		_RecordsetPtr prs = CreateParamRecordset(FormatString("SELECT TaskID, PersonID, dbo.GetTodoAssignToIDString(ToDoList.TaskID) as AssignedIDs FROM TodoList WHERE RegardingType = %d AND RegardingID = {INT}",
			ttEMNDetail), m_nEMRDetailID);
		FieldsPtr f = prs->Fields;
		while (!prs->eof) {
			// Update the more info topic
			if (NULL != m_pParentTopic && NULL != m_pParentTopic->GetParentEMN()) {
				CWnd *pWnd = m_pParentTopic->GetParentEMN()->GetInterface();
				if (pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
					pWnd->PostMessage(NXM_EMN_TODO_REFRESH_LIST, (WPARAM)m_pParentTopic->GetParentEMN());
				}
			}
			// Update the Practice universe
			// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo
			CString strAssignedIDs = AdoFldString(f, "AssignedIDs", "");
			CArray < long, long> arrAssignedID;
			ParseDelimitedStringToLongArray(strAssignedIDs, " ", arrAssignedID);

			if (arrAssignedID.GetSize() == 1){
				CClient::RefreshTodoTable(AdoFldLong(f, "TaskID", -1), AdoFldLong(f, "PersonID", -1), arrAssignedID[0], TableCheckerDetailIndex::tddisChanged);
			}
			else{
				CClient::RefreshTodoTable(AdoFldLong(f, "TaskID", -1), AdoFldLong(f, "PersonID", -1), -1, TableCheckerDetailIndex::tddisChanged);
			}
			prs->MoveNext();
		}
	}

	// (j.jones 2008-07-21 09:07) - PLID 30779 - update problems	
	// (c.haag 2008-08-15 13:00) - PLID 30820 - Remove deleted problems from memory
	// (z.manning 2009-05-22 11:56) - PLID 34332 - Update problem links instead
	for(int nProblemLinkIndex = 0; nProblemLinkIndex < m_apEmrProblemLinks.GetSize(); nProblemLinkIndex++)
	{
		CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(nProblemLinkIndex);
		if(pProblemLink != NULL && (pProblemLink->GetIsDeleted() || (pProblemLink->GetProblem() != NULL && pProblemLink->GetProblem()->m_bIsDeleted))) {
			m_apEmrProblemLinks.RemoveAt(nProblemLinkIndex--);
			delete pProblemLink;
		}
	}

	// (j.jones 2006-02-21 09:08) - re-set the original parent topic to reflect the saved status
	m_pOriginalParentTopic = m_pParentTopic;
	if(m_pOriginalParentTopic) {
		m_strLastSavedParentTopicName = m_pOriginalParentTopic->GetName();
	}

	// (j.jones 2006-08-18 16:16) - revert the ink changed states
	m_bInkHasBeenAdded = FALSE;
	m_bInkHasBeenErased = FALSE;
	m_bImageTextHasBeenAdded = FALSE;
	m_bImageTextHasBeenRemoved = FALSE;
	// (r.gonet 05/02/2012) - PLID 49946
	m_bImageTextHasBeenModified = FALSE;

	// (j.jones 2013-08-07 16:12) - PLID 42958 - If a signature is added by another user,
	// these fields store that fact, and their username, until the initial audit is saved.
	// These are not filled when loading an EMN, they're only for new details.
	m_bIsSignatureAddedByAnotherUser = false;
	m_strSignatureAddedByAnotherUsername = "";

	// (z.manning 2010-02-24 11:23) - PLID 37225 - We just deleted these so go ahead and clear out this array.
	m_arynDeletedImageStampIDs.RemoveAll();

	// (j.jones 2006-02-21 12:05) - re-set the last saved detail state
	// (a.walling 2012-10-31 17:17) - PLID 53550 - LastSavedDetail - reset
	m_oLastSavedDetail = boost::none;


	// (c.haag 2006-12-27 15:02) - PLID 23158 - If this detail has a problem, we need
	// to check if it was deleted. If it was deleted, it should be hidden. To avoid
	// a horrible speed hit by blindly querying data, we will use the table checker way
	// of doing it in EmrItemAdvDlg.cpp
	// (j.jones 2008-07-21 08:23) - PLID 30779 - call HasProblems() instead
	if (m_pEmrItemAdvDlg && HasProblems()) {
		// (j.jones 2008-08-15 14:43) - PLID 30779 - pass in TRUE when this is a PostSaveUpdate
		if (m_pEmrItemAdvDlg->CheckForProblemDataChanges(TRUE)) {
			// If we get here, it means the problem did change. If the problem was
			// deleted, then CheckForProblemDataChanges would have set our problem ID
			// to -1.
			if (!HasProblems()) {
				// If we get here, the problem was surely deleted. Hide the button
				// by calling UpdateProblemStatusButtonAppearance, and warn the user.
				MsgBox("The problem for the detail \"%s\" on the topic \"%s\" for the EMN \"%s\" has been deleted. The problem button will now be removed from that detail.",
					GetMergeFieldName(FALSE), m_pParentTopic->GetName(), m_pParentTopic->GetParentEMN()->GetDescription());
				m_pEmrItemAdvDlg->UpdateProblemStatusButtonAppearance();
			}
		}
	}

	// (a.walling 2012-10-31 17:17) - PLID 53550 - LastSavedDetail - reset
	m_oLastSavedDetail = CreateLastSavedDetail();

	//End all updating code
	///////////////
}

// (c.haag 2007-06-20 12:38) - PLID 26397 - We now store saved objects in a map for fast lookups
// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize
Nx::Quantum::Batch CEMNDetail::GenerateSaveString_Template(long nEMNTopicID, long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN CMapPtrToPtr& mapSavedObjects)
{
	//save a template detail record

	//This does nothing if it's already loaded.
	LoadContent();
	Nx::Quantum::Batch strSaveString;

	ASSERT(m_pParentTopic); //We shouldn't be saving until we're attached to a topic.

	CString strTemplateID;
	long nTemplateID = m_pParentTopic->GetParentEMN()->GetID();
	if(nTemplateID != -1) {
		strTemplateID.Format("%li",nTemplateID);
	}
	else {
		AddStatementToSqlBatch(strSaveString, "SET @nEMRTemplateID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotEMN, (long)m_pParentTopic->GetParentEMN());
		strTemplateID = "@nEMRTemplateID";
	}

	CString strTopicID;
	if(nEMNTopicID != -1) {
		strTopicID.Format("%li",nEMNTopicID);
	}
	else {
		// (c.haag 2007-06-11 15:56) - PLID 26267 - This is no longer necessary now that we set nEMRTemplateTopicID
		// as soon as the new topic is generated.
		//AddStatementToSqlBatch(strSaveString, "SET @nEMRTemplateTopicID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotTopic, (long)m_pParentTopic);
		strTopicID = "@nEMRTemplateTopicID";
	}

	// (j.jones 2010-02-12 11:07) - PLID 37318 - supported ChildEMRTemplateDetailID
	CString strChildEMRTemplateDetailID = "NULL";
	if(m_pSmartStampTableDetail != NULL && m_pSmartStampTableDetail->m_nEMRTemplateDetailID != -1) {
		//we have a linked table detail and it has an ID, so simply save the ID
		strChildEMRTemplateDetailID.Format("%li", m_pSmartStampTableDetail->m_nEMRTemplateDetailID);
	}
	
	CString strID;

	if(m_nEMRTemplateDetailID == -1) {
		//insert a new record first

		// (c.haag 2007-06-14 10:43) - PLID 26277 - Get the new ID with scope_identity and store it in @nNewObjectID
		// (c.haag 2008-06-16 12:10) - PLID 30319 - If we're based off an EMR text macro, then we need to save our label text
		// to the MacroName field.
		// (j.jones 2010-02-12 11:07) - PLID 37318 - supported ChildEMRTemplateDetailID
		AddStatementToSqlBatch(strSaveString, "INSERT INTO EMRTemplateDetailsT (TemplateID, EMRInfoMasterID, MacroName, ChildEMRTemplateDetailID) "
			"VALUES (%s, %li, %s, %s)\r\n"
			"SET @nNewObjectID = SCOPE_IDENTITY()\r\n"
			"SET @nEMRTemplateDetailID = @nNewObjectID"
			, strTemplateID, m_nEMRInfoMasterID, 
			(EMR_BUILT_IN_INFO__TEXT_MACRO == m_nEMRInfoID) ? (CString("'") + _Q(GetLabelText()) + "'") : "NULL", strChildEMRTemplateDetailID
			);

		// (j.jones 2007-01-11 14:41) - PLID 24027 - if our source detail ID is -1, then
		// be sure to update SourceDetailID at the end of the save!
		if(m_sai.pSourceDetail && m_sai.nSourceDetailID == -1) {
			//find this detail's ID, find the source detail's ID, and update this detail's SourceDetailID
			AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRObjectIDToUpdate = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), -1)", esotDetail, (long)this);
			AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRSourceDetailID = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), NULL)", esotDetail, (long)m_sai.pSourceDetail);
			AddStatementToSqlBatch(strPostSaveSql, "UPDATE EMRTemplateDetailsT SET SourceDetailID = @nEMRSourceDetailID WHERE ID = @nEMRObjectIDToUpdate");
		}

		// (j.jones 2010-02-12 11:07) - PLID 37318 - supported ChildEMRTemplateDetailID, if the linked table is -1,
		// we will have to update later on
		if(m_pSmartStampTableDetail != NULL && m_pSmartStampTableDetail->m_nEMRTemplateDetailID == -1) {
			//we have a linked table detail and it has an ID, so simply save the ID
			AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRObjectIDToUpdate = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), -1)", esotDetail, (long)this);
			AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRChildEMRTemplateDetailID = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), NULL)", esotDetail, (long)m_pSmartStampTableDetail);
			AddStatementToSqlBatch(strPostSaveSql, "UPDATE EMRTemplateDetailsT SET ChildEMRTemplateDetailID = @nEMRChildEMRTemplateDetailID WHERE ID = @nEMRObjectIDToUpdate");
		}

		// (c.haag 2007-06-14 10:45) - PLID 26277 - This is depreciated now that we get the new template detail ID with scope_identity
		//AddStatementToSqlBatch(strSaveString, "SET @nNewObjectID = (SELECT COALESCE(MAX(ID), 0) FROM EMRTemplateDetailsT WITH(UPDLOCK, HOLDLOCK))");
		AddNewEMRObjectToSqlBatch(strSaveString, esotDetail, (long)this, mapSavedObjects);
		//AddStatementToSqlBatch(strSaveString, "SET @nEMRTemplateDetailID = @nNewObjectID");

		strID = "@nEMRTemplateDetailID";
	}
	else {
		strID.Format("%li",m_nEMRTemplateDetailID);
	}

	//now update the record with the shared data (data used by all detail types)

	// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize - Use variants and COALESCE to eliminate the need to conditionally include updates to coordinates
	_variant_t varPosX = g_cvarNull;
	_variant_t varPosY = g_cvarNull;
	_variant_t varPosWidth = g_cvarNull;
	_variant_t varPosHeight = g_cvarNull;

	bool bUpdateCoords = false;
	{
		// If the advdlg is visible, get its coordinates and save them in the recordset
		if(IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
			CRect rc;
			m_pEmrItemAdvDlg->GetClientRect(&rc);
			m_pEmrItemAdvDlg->ClientToScreen(&rc);
			m_pEmrItemAdvDlg->GetParent()->SendMessage(NXM_CONVERT_RECT_FOR_DATA, (WPARAM)&rc);
			
			varPosX = rc.left;
			varPosY = rc.top;
			varPosWidth = (long)rc.Width();
			varPosHeight = (long)rc.Height();

			bUpdateCoords = true;

		} else {
			//if we get here, it means the topic was never loaded, and the advdlg never created
			
			//if new, save the default area, if we were given one
			// (a.walling 2007-08-09 13:36) - PLID 26781 - The adv dlg may have been destroyed
			if((m_nEMRDetailID == -1 && !m_rcDefaultClientArea.IsRectNull()) || (m_oLastSavedDetail && (m_oLastSavedDetail->m_rcDefaultClientArea != m_rcDefaultClientArea)) ) {
				varPosX = (long)m_rcDefaultClientArea.left;
				varPosY = (long)m_rcDefaultClientArea.top;
				varPosWidth = (long)m_rcDefaultClientArea.Width();
				varPosHeight = (long)m_rcDefaultClientArea.Height();

				bUpdateCoords = true;
			}
		}
	}

	CString strMergeOverride = "NULL";
	if (m_strMergeFieldOverride.GetLength()) {
		strMergeOverride.Format("'%s'",_Q(m_strMergeFieldOverride));
	}

	CString strSourceActionID = "NULL";
	if(m_sai.nSourceActionID != -1) {
		strSourceActionID.Format("%li", m_sai.nSourceActionID);
	}

	// (j.jones 2007-01-10 14:57) - PLID 24027 - supported SourceDetailID
	CString strSourceDetailID = "NULL";
	if(m_sai.nSourceDetailID != -1) {
		strSourceDetailID.Format("%li", m_sai.nSourceDetailID);
	}

	// (z.manning 2009-03-11 10:40) - PLID 33338 - SourceDataGroupID
	CString strSourceDataGroupID = "NULL";
	if(m_sai.GetDataGroupID() != -1) {
		strSourceDataGroupID = AsString(m_sai.GetDataGroupID());
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

	// (z.manning 2011-10-05 16:40) - PLID 45742 - Handle 3D image print data
	// (j.armen 2014-07-22 08:58) - PLID 62836 - Handle 2D pen color and size
	// Default all values to null
	CString strInkPenColor = "NULL";
	CString strInkPenSize = "NULL";
	CString strPrintData = "CONVERT(VARBINARY(MAX), NULL)";

	if (Is2DImage())
	{
		// If this is a 2D image, and we have a pen color, save it
		if (m_nDefaultPenColor)
			strInkPenColor = AsStringForSql(*m_nDefaultPenColor);
		// If this is a 2D image, and we have a pen size, save it
		if (m_nDefaultPenSizePercent)
			strInkPenSize = AsStringForSql(*m_nDefaultPenSizePercent);
	}
	else if (Is3DImage())
	{
		CEmrItemAdvImageState ais(m_varState);
		if (ais.m_varPrintData.vt != VT_NULL && ais.m_varPrintData.vt != VT_EMPTY)
			strPrintData = CreateByteStringFromSafeArrayVariant(ais.m_varPrintData);
	}

	// (a.walling 2008-06-30 14:41) - PLID 29271 - Update Preview Pane flags
	//now we have our shared data, so update that now
	// (z.manning 2009-03-11 10:41) - PLID 33338 - SourceDataGroupID
	// (j.jones 2010-02-12 11:07) - PLID 37318 - supported ChildEMRTemplateDetailID
	//TES 3/17/2010 - PLID 37530 - SourceStampID, SourceStampIndex
	// (z.manning 2011-10-05 16:48) - PLID 45842 - Added print data
	// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize - Use variants and COALESCE to eliminate the need to conditionally include updates to coordinates
	// (j.armen 2014-07-22 09:03) - PLID 62836 - Always set InkPenColor, InkPenSize, PrintData, even if those values should be null
	AddStatementToSqlBatch(strSaveString, R"(
UPDATE EMRTemplateDetailsT SET
	EMRTemplateTopicID = %s,
	X = COALESCE(%s, X), Y = COALESCE(%s, Y),
	Width = COALESCE(%s, Width), Height = COALESCE(%s, Height),
	MergeOverride = %s,
	SourceActionID = %s,
	SourceDetailID = %s,
	PreviewFlags = %li,
	SourceDataGroupID = %s,
	ChildEMRTemplateDetailID = %s,
	SourceStampID = %s, SourceStampIndex = %s,
	InkPenColor = %s, InkPenSizePercent = %s,
	PrintData = %s
WHERE ID = %s)",
		strTopicID,
		AsStringForSql(varPosX), AsStringForSql(varPosY),
		AsStringForSql(varPosWidth), AsStringForSql(varPosHeight),
		strMergeOverride,
		strSourceActionID,
		strSourceDetailID,
		m_nPreviewFlags,
		strSourceDataGroupID,
		strChildEMRTemplateDetailID,
		strSourceStampID, strSourceStampIndex,
		strInkPenColor, strInkPenSize,
		strPrintData,
		strID);

	// (j.jones 2010-02-12 11:07) - PLID 37318 - supported ChildEMRTemplateDetailID, if the linked table is -1,
	// we will have to update later on
	if(m_pSmartStampTableDetail != NULL && m_pSmartStampTableDetail->m_nEMRTemplateDetailID == -1 && m_nEMRTemplateDetailID != -1) {
		//we have a linked table detail and it has an ID, so simply save the ID
		AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRChildEMRTemplateDetailID = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), NULL)", esotDetail, (long)m_pSmartStampTableDetail);
		AddStatementToSqlBatch(strPostSaveSql, "UPDATE EMRTemplateDetailsT SET ChildEMRTemplateDetailID = @nEMRChildEMRTemplateDetailID WHERE ID = %li", m_nEMRTemplateDetailID);
	}

	//while we're at it, audit the changes to shared data

	//for later auditing of creation on new EMNs, set the boolean now
	if(nTemplateID == -1 && m_nEMRTemplateDetailID == -1) {
		m_bCreatedOnNewEMN = TRUE;
	}

	//if the entire template is new, don't audit this at all
	if(nTemplateID != -1) {

		CString strOldDetailName, strOldDetailData, strNewDetailName, strNewDetailData;

		// (c.haag 2008-06-06 11:04) - PLID 27240 - If this item was brought up to date, so was the last saved detail.
		// So, pull the audit text from before it was brought up to date.
		if (m_bDetailWasBroughtUpToDate) {
			strOldDetailName = m_strPreUpdatedDetailName;
			strOldDetailData = m_strPreUpdatedDetailData;
		}
		else {
			if(m_oLastSavedDetail) {
				m_oLastSavedDetail->GetDetailNameAndDataForAudit(strOldDetailName,strOldDetailData);
			}
			else {
				ASSERT(m_nEMRTemplateDetailID == -1);
			}
		}

		GetDetailNameAndDataForAudit(strNewDetailName,strNewDetailData);

		if(m_nEMRTemplateDetailID == -1) {
			//detail added
			CString strNewValue;
			strNewValue.Format("Detail: %s, Data: %s",strNewDetailName,strNewDetailData);
			if(nAuditTransactionID == -1)
				nAuditTransactionID = BeginAuditTransaction();
			AuditEvent(-1, "",nAuditTransactionID,aeiEMNTemplateDetailAddedToExistingTemplate,nTemplateID,"",strNewValue,aepHigh,aetCreated);
		}
		else {
			//detail changed
			CString strOldValue, strNewValue;
			strOldValue.Format("Detail: %s, Data: %s",strOldDetailName,strOldDetailData);
			strNewValue.Format("Detail: %s, Data: %s",strNewDetailName,strNewDetailData);

			// (c.haag 2008-06-05 13:28) - PLID 27240 - Check to see whether the detail was brought up to date
			if (m_bDetailWasBroughtUpToDate) {
				CString strNewValue;
				strNewValue.Format("Detail: %s, Data: %s",strNewDetailName,strNewDetailData);
				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(-1, "",nAuditTransactionID,aeiEMNTemplateDetailBroughtUpToDate,nTemplateID,strOldValue,strNewValue,aepHigh,aetChanged);
			}

			// (j.jones 2006-08-21 14:11) - PLID 22092 - if only the merge override name
			// changed, audit separately and don't say the detail itself changed
			BOOL bMergeNameChanged = FALSE;
			if(m_oLastSavedDetail) {
				if(GetMergeFieldName(FALSE) != m_oLastSavedDetail->GetMergeFieldName()) {
					//the name is different, but is it the item name, or just the merge field name?
					if(m_strLabelText == m_oLastSavedDetail->m_strLabelText) {
						//if we get here, the name that merged has changed from before,
						//but the name of the detail has not changed, thus we know
						//the merge name changed in some way (this includes the addition
						//or removal of the merge name override)
						bMergeNameChanged = TRUE;
					}
				}
			}
			else {
				ASSERT(m_nEMRTemplateDetailID == -1);
			}

			if(bMergeNameChanged) {
				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(-1, "",nAuditTransactionID,aeiEMNTemplateDetailMergeName,nTemplateID,strOldValue,strNewValue,aepHigh,aetChanged);
			}

			//if the detail name or data changed, audit as such
			if(strOldDetailData != strNewDetailData || (m_oLastSavedDetail && (m_strLabelText != m_oLastSavedDetail->m_strLabelText))) {
				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(-1, "",nAuditTransactionID,aeiEMNTemplateDetailChanged,nTemplateID,strOldValue,strNewValue,aepHigh,aetChanged);
			}

			//see if we moved it between topics
			if(m_pOriginalParentTopic != m_pParentTopic) {
				CString strOldTopicName, strNewTopicName;
				if(!m_strLastSavedParentTopicName.IsEmpty())
					strOldTopicName = "Topic: " + m_strLastSavedParentTopicName;
				strNewTopicName = "Topic: " + m_pParentTopic->GetName();
				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(-1, "",nAuditTransactionID,aeiEMNTemplateTopicMoved,nTemplateID,strOldTopicName,strNewTopicName,aepMedium,aetChanged);
			}

			//see if we moved its location on a topic

			long nOldX = -1, nOldY = -1, nOldWidth = -1, nOldHeight = -1;
			if(m_oLastSavedDetail) {
				if(!m_oLastSavedDetail->m_rcDefaultClientArea.IsRectNull()) {
					nOldX = (long)m_oLastSavedDetail->m_rcDefaultClientArea.left;
					nOldY = (long)m_oLastSavedDetail->m_rcDefaultClientArea.top;
					nOldWidth = (long)m_oLastSavedDetail->m_rcDefaultClientArea.Width();
					nOldHeight = (long)m_oLastSavedDetail->m_rcDefaultClientArea.Height();
				}
			}
			else {
				ASSERT(m_nEMRTemplateDetailID == -1);
			}

			//next check to see if we moved its location on a topic
			//JMJ - for the time being, we will still audit this even if it moved topic, but we may consider not doing so at some point
			// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize - Use variants and COALESCE to eliminate the need to conditionally include updates to coordinates
			if(bUpdateCoords &&
				((nOldX != -1 && nOldX != (long)varPosX) || (nOldY != -1 && nOldY != (long)varPosY))) {
				//JMJ - I see no reason to give them coordinates, so just audit that it was moved
				CString strNewValue;
				strNewValue.Format("Detail: %s, Data: %s",strNewDetailName,strNewDetailData);
				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(-1, "",nAuditTransactionID,aeiEMNTemplateDetailPlacementMoved,nTemplateID,strNewValue,"Moved",aepLow,aetChanged);
			}
			
			//last, check to see if we resized the detail
			// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize - Use variants and COALESCE to eliminate the need to conditionally include updates to coordinates
			if(bUpdateCoords &&
				((nOldWidth != -1 && nOldWidth != (long)varPosWidth) || (nOldHeight != -1 && nOldHeight != (long)varPosHeight))) {
				//JMJ - I see no reason to give them coordinates, so just audit that it was resized
				CString strNewValue;
				strNewValue.Format("Detail: %s, Data: %s",strNewDetailName,strNewDetailData);
				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(-1, "",nAuditTransactionID,aeiEMNTemplateDetailResized,nTemplateID,strNewValue,"Resized",aepLow,aetChanged);
			}
		}
	}

	//now we need to save the changes to type-specific data

	switch (m_EMRInfoType) {
	case eitText:
		AddStatementToSqlBatch(strSaveString, "UPDATE EMRTemplateDetailsT SET DefaultText = '%s' WHERE ID = %s",
			_Q(VarString(m_varState, "")), strID);
		break;
	case eitSingleList:
	case eitMultiList:
		if (m_nEMRTemplateDetailID == -1) {
			//Make sure content is loaded (this will do nothing if it's already loaded).
			LoadContent();
			for(int nElement = 0; nElement < m_arListElements.GetSize(); nElement++) {
				ListElement le = m_arListElements[nElement];
				if(le.bIsSelected) {
					AddStatementToSqlBatch(strSaveString, "INSERT INTO EmrTemplateSelectT (EmrTemplateDetailID, EmrDataID) "
						"VALUES (%s, %li)", strID, le.nID);
				}
			}
		} else {
			//First, make sure we have the right list of EmrDetailElementsT.
			if(m_pEmrItemAdvDlg) {
				BOOL bElementsChanged = TRUE;
				if(m_oLastSavedDetail && m_oLastSavedDetail->m_listElements.size() == m_arListElements.GetSize()) {
					//TES 6/12/2006 - Now that we have a way of checking the original state of our list, don't do anything
					//if it hasn't changed.
					bElementsChanged = FALSE;
					for(int nElement = 0; nElement < m_arListElements.GetSize() && !bElementsChanged; nElement++) {
						// (a.walling 2012-10-31 17:17) - PLID 53551 - Compare with the LastSavedDetail SimpleListElement structure
						const Emr::SimpleListElement& leOld = m_oLastSavedDetail->m_listElements[nElement];
						ListElement leNew = m_arListElements[nElement];
						if(leOld.nID != leNew.nID || leOld.bIsSelected != leNew.bIsSelected) {
							bElementsChanged = TRUE;
						}
					}
				}
				
				if(bElementsChanged) {
					//TES 8/8/2006 - Clear out any previously selected details, and re-add them.
					AddStatementToSqlBatch(strSaveString, "DELETE FROM EmrTemplateSelectT WHERE EmrTemplateDetailID = %s", strID);

					for(int nElement = 0; nElement < m_arListElements.GetSize(); nElement++) {
						ListElement le = m_arListElements[nElement];
						if(le.bIsSelected) {
							AddStatementToSqlBatch(strSaveString, "INSERT INTO EmrTemplateSelectT (EmrTemplateDetailID, EmrDataID) "
								"VALUES (%s, %li)", strID, le.nID);
						}
					}
				}
			}
		}
		break;
	case eitImage:
		{
			//DRT 1/23/2008 - PLID 28697 - I don't get why this isn't merged with GenerateSaveString_State(), but oh well.
			//	Save the hotspot data which has been selected.
			//Fill an array with all the IDs that are selected
			CEmrItemAdvImageState ais;
			ais.CreateFromSafeArrayVariant(m_varState);
			CDWordArray arySelectedData;
			ParseDelimitedStringToDWordArray(ais.m_strSelectedHotSpotData, ";", arySelectedData);
			if (m_nEMRTemplateDetailID == -1) {
				//We are saving a new detail.  Don't need to worry about any old data.  
				//	Make sure content is loaded (this will do nothing if it's already loaded).
				LoadContent();

				//Then simply generate an SQL statement
				for(int i = 0; i < arySelectedData.GetSize(); i++) {
					AddStatementToSqlBatch(strSaveString, "INSERT INTO EMRHotSpotTemplateSelectT (EMRDetailID, EMRImageHotSpotID) "
								"VALUES (%s, %li)", strID, arySelectedData.GetAt(i));
				}
			}
			else {
				//In this case, we're updating an existing select list.  We need to figure out which elements have
				//	been newly added and which have been removed.

				//First, make sure we have the right list of EmrDetailElementsT.
				if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
					if(m_oLastSavedDetail) {
						//From here we have the old & current.  Figure out what differs
						CDWordArray aryNewIDs, aryRemovedIDs;
						ForSave_CalculateChangedHotSpots(&arySelectedData, &m_oLastSavedDetail->m_selectedHotSpotIDs, &aryNewIDs, &aryRemovedIDs);

						//At this point we know what changed, so write DELETE statements for the removed, and
						//	INSERT statements for the new.
						int i = 0;
						for(i = 0; i < aryNewIDs.GetSize(); i++) {
							AddStatementToSqlBatch(strSaveString, "INSERT INTO EMRHotSpotTemplateSelectT (EMRDetailID, EMRImageHotSpotID) "
										"VALUES (%s, %li)", strID, aryNewIDs.GetAt(i));
						}

						for(i = 0; i < aryRemovedIDs.GetSize(); i++) {
							AddStatementToSqlBatch(strSaveString, "DELETE FROM EMRHotSpotTemplateSelectT WHERE EMRDetailID = %s "
								"AND EMRImageHotSpotID = %li ", strID, aryRemovedIDs.GetAt(i));
						}
					}
					else {
						//TODO - Why would we not have a last saved detail?
						// (c.haag 2010-06-01 09:18) - PLID 38709 - While we still don't know how this could happen
						// and have never seen it in practice, we should not just assert here because unexpected behavior
						// in a save function can lead to data corruption.
						//ASSERT(FALSE);
						CString strEx;
						strEx.Format("CEMNDetail::GenerateSaveString_Template was called for template image detail ID %d, but m_oLastSavedDetail was NULL!", m_nEMRTemplateDetailID);
						ThrowNxException(strEx);
					}
				}
				else {
					// (c.haag 2010-05-27 16:59) - PLID 38709 - We should never get here because an image
					// detail cannot be modified without having a valid image dialog. Don't just fire an assertion,
					// we want the save to completely fail.
					//ASSERT(FALSE);
					CString strEx;
					strEx.Format("CEMNDetail::GenerateSaveString_Template was called for template image detail ID %d, but the detail does not have a valid image window!", m_nEMRTemplateDetailID);
					ThrowNxException(strEx);
				}
			}
		}
		break;
	case eitSlider:
		if(m_varState.vt == VT_NULL) {
			AddStatementToSqlBatch(strSaveString, "UPDATE EMRTemplateDetailsT SET SliderValue = NULL WHERE ID = %s",
				strID);
		}
		else {
			AddStatementToSqlBatch(strSaveString, "UPDATE EMRTemplateDetailsT SET SliderValue = %g WHERE ID = %s",
				VarDouble(m_varState), strID);
		}
		break;
	case eitNarrative:
		AddStatementToSqlBatch(strSaveString, "UPDATE EMRTemplateDetailsT SET DefaultText = '%s' WHERE ID = %s",
			_Q(VarString(m_varState, "")), strID);
		break;
	case eitTable: 
		{
			//Make sure content is loaded (this will do nothing if it's already loaded).
			LoadContent();
			SyncContentAndState();

			// (a.wetta 2005-11-04 10:53) - PLID 18168 - Update column width information
			if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd()))
				strSaveString += ((CEmrItemAdvTableDlg*)m_pEmrItemAdvDlg)->UpdateColumnInfoInData(GetSaveTableColumnWidths());

			BOOL bTableChanged = TRUE;
			BOOL bDataChanged = TRUE;

			// (a.walling 2012-10-31 17:17) - PLID 53552 - Compare with the simpler LastSavedDetail structures
			if (m_nEMRTemplateDetailID != -1 
				&& m_oLastSavedDetail 
				&& m_oLastSavedDetail->m_tableElements.size() == m_arTableElements.GetSize()
				&& m_oLastSavedDetail->m_tableRows.size() == m_arTableRows.GetSize()
				&& m_oLastSavedDetail->m_tableColumns.size() == m_arTableColumns.GetSize()
			) 
			{
				//TES 6/12/2006 - Now that we have a way of checking the original state of our list, don't do anything
				//if it hasn't changed.
				bTableChanged = FALSE;
				bDataChanged = FALSE;

				//TES 3/15/2010 - PLID 37757 - Make sure that Smart Stamp tables don't save their rows and columns.
				if(!IsSmartStampTable()) {
					// (a.walling 2012-10-31 17:17) - PLID 53552 - First check rows, then columns, then individual elements
					for(int nRow = 0; nRow < m_arTableRows.GetSize() && !bTableChanged; nRow++) {
						TableRow *ptrNew = GetRowPtr(nRow);
						const Emr::SimpleTableRow& oldRow = m_oLastSavedDetail->m_tableRows[nRow];
						if (
							(oldRow.nID != ptrNew->m_ID.nDataID) 
							||
							(oldRow.nEmrDetailImageStampID != ptrNew->m_ID.GetDetailImageStampID()) 
						)
						{
							bTableChanged = TRUE;
						}
					}
					for(int nCol = 0; nCol < m_arTableColumns.GetSize() && !bTableChanged; nCol++) {
						TableColumn tcNew = GetColumn(nCol);
						const Emr::SimpleTableColumn& oldCol = m_oLastSavedDetail->m_tableColumns[nCol];
						if (oldCol.nID != tcNew.nID) {
							bTableChanged = TRUE;
						}
					}
					for(int e = 0; e < m_arTableElements.GetSize() && !bTableChanged && !bDataChanged; e++) {
						TableElement elem = m_arTableElements[e];
						const Emr::SimpleTableElement& oldElem = m_oLastSavedDetail->m_tableElements[e];
						if (
							(elem.m_pRow->m_ID.nDataID != oldElem.row.nID)
							||
							(elem.m_pRow->m_ID.GetDetailImageStampID() != oldElem.row.nEmrDetailImageStampID)
							||
							(elem.m_pColumn->nID != oldElem.col.nID)
							||
							(elem.GetValueAsVariant() != oldElem.varValue)
						)
						{
							bDataChanged = TRUE;
						}
					}
				}
			}

			if(bTableChanged) bDataChanged = TRUE; //If the rows or columns changed, then the data must have changed.

			//now update EMRTemplateTableDefaultsT
			if(bDataChanged) {
				AddStatementToSqlBatch(strSaveString, "DELETE FROM EMRTemplateTableDefaultsT WHERE EMRTemplateDetailID = %s", strID);

				if(!IsSmartStampTable()) {
					for(int i = 0; i < m_arTableElements.GetSize(); i++) {
						TableElement te = m_arTableElements[i];
						// (z.manning 2008-06-26 15:00) - PLID 30155 - Do not save calculated field values in data.
						// (z.manning 2012-03-27 12:43) - PLID 33710 - Now we do save calculated cell values to data.
						//if(!te.IsCalculated())
						{
							// (a.walling 2009-03-12 13:56) - PLID 33486 - Do not save empty elements to data either
							if (!te.IsValueEmpty()) {
								AddStatementToSqlBatch(strSaveString, "SET @nNewObjectID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRTemplateTableDefaultsT WITH(UPDLOCK, HOLDLOCK))");
								
								//AddNewEMRObjectToSqlBatch is NOT necessary here because we aren't going to recall any IDs later
								AddStatementToSqlBatch(strSaveString, "INSERT INTO EMRTemplateTableDefaultsT (ID, EMRTemplateDetailID, EMRDataID_X, EMRDataID_Y, Data) "
									"VALUES (@nNewObjectID, %s, %li, %li, '%s')", strID, te.m_pRow->m_ID.nDataID, te.m_pColumn->nID, _Q(te.GetValueAsString()));
							}
						}
					}
				}
			}
		}
		break;

	default:
		// unknown
		ASSERT(FALSE);
		break;
	}

	// (j.jones 2006-08-29 08:57) - PLID 22250 - update the template's modified date
	if(IsModified() && nTemplateID != -1) {
		AddStatementToSqlBatch(strSaveString, "UPDATE EMRTemplateT SET ModifiedDate = GetDate(), ModifiedLogin = '%s' WHERE ID = %li", _Q(GetCurrentUserName()), nTemplateID);
	}

	return strSaveString;
}

// (c.haag 2007-06-20 12:38) - PLID 26397 - We now store saved objects in a map for fast lookups
// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize
Nx::Quantum::Batch CEMNDetail::GenerateSaveString_PatientEMN(long nEMNTopicID, long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN CMapPtrToPtr& mapSavedObjects, CDWordArray &arynModifiedEMRDetailIDs)
{
	//save a patient EMN detail record

	//This does nothing if it's already loaded.
	LoadContent();

	// (j.jones 2006-08-24 10:53) - PLID 22183 - find out if this detail is on a locked EMN
	BOOL bIsLockedAndSaved = GetIsOnLockedAndSavedEMN();

	Nx::Quantum::Batch strSaveString;
	
	ASSERT(m_pParentTopic); //We shouldn't be saving until we're attached to a topic.

	CString strEMNID;
	long nEMNID = m_pParentTopic->GetParentEMN()->GetID();
	if(nEMNID != -1) {
		strEMNID.Format("%li",nEMNID);
	}
	else {
		// (c.haag 2007-06-11 17:28) - PLID 26280 - This is no longer necessary. We assign nEMNID when the record is created in NewObjectsT.
		/*CString str;
		str.Format("SET @nEMNID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotEMN, (long)m_pParentTopic->GetParentEMN());
		AddStatementToSqlBatch(strSaveString, str);*/
		strEMNID = "@nEMNID";
	}

	CString strTopicID;
	if(nEMNTopicID != -1) {
		strTopicID.Format("%li",nEMNTopicID);
	}
	else {
		// (c.haag 2007-06-11 15:40) - PLID 26267 - This is no longer necessary now that we set nEMRTopicID
		// as soon as the new topic is generated.
		strTopicID = "@nEMRTopicID";
		/*CString str;
		str.Format("SET @nEMRTopicID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotTopic, (long)m_pParentTopic);
		AddStatementToSqlBatch(strSaveString, str);*/
	}
	
	CString strID;
	if(m_nEMRDetailID == -1) {
		// (c.haag 2007-06-14 12:49) - PLID 26338 - We no longer INSERT INTO here. We insert later on after we've figured out
		// all the columns we need to fill in; not just EMRID and EMRInfoID. All the code that did nothing but add statements
		// to the batch has been removed.
		//insert a new record first
		if(!bIsLockedAndSaved) {
			strID = "@nEMRDetailID";
		}
		else {
			//we tried to change this on a locked EMN!
			CString str;
			str.Format("Attempted to create a new detail '%s' on a locked EMN.", GetMergeFieldName(FALSE));
			arystrErrors.Add(str);
		}
	}
	else {
		strID.Format("%li",m_nEMRDetailID);
	}

	// (c.haag 2007-06-14 14:31) - PLID 26338 - We used to generate problem-related	queries for the batch here.
	// Along with the batches to create EMRDetailsT records, they have been moved further down to ensure that
	// new details are created before those batches are run.

	//now update the record with the shared data (data used by all detail types)

	long nOrderID = -1;
	long nTotalDetailCount = m_pParentTopic->GetParentEMN()->GetTotalDetailCount();
	for(long i=0;i<nTotalDetailCount && nOrderID == -1;i++) {
		if(m_pParentTopic->GetParentEMN()->GetDetail(i) == this)
			nOrderID = i+1;
	}

	// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize - Use variants and COALESCE to eliminate the need to conditionally include updates to coordinates
	_variant_t varPosX = g_cvarNull;
	_variant_t varPosY = g_cvarNull;
	_variant_t varPosWidth = g_cvarNull;
	_variant_t varPosHeight = g_cvarNull;

	bool bUpdateCoords = false;
	{
		// If the advdlg is visible, get its coordinates and save them in the recordset
		if(IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
			CRect rc;
			m_pEmrItemAdvDlg->GetClientRect(&rc);
			m_pEmrItemAdvDlg->ClientToScreen(&rc);
			m_pEmrItemAdvDlg->GetParent()->SendMessage(NXM_CONVERT_RECT_FOR_DATA, (WPARAM)&rc);
			
			varPosX = rc.left;
			varPosY = rc.top;
			varPosWidth = (long)rc.Width();
			varPosHeight = (long)rc.Height();

			bUpdateCoords = true;
		} else {
			//if we get here, it means the topic was never loaded, and the advdlg never created
			
			//if new, save the default area
			// (a.walling 2007-08-09 13:36) - PLID 26781 - The adv dlg may have been destroyed
			if( (m_nEMRDetailID == -1) || (m_oLastSavedDetail && (m_oLastSavedDetail->m_rcDefaultClientArea != m_rcDefaultClientArea)) ) {
				varPosX = (long)m_rcDefaultClientArea.left;
				varPosY = (long)m_rcDefaultClientArea.top;
				varPosWidth = (long)m_rcDefaultClientArea.Width();
				varPosHeight = (long)m_rcDefaultClientArea.Height();

				bUpdateCoords = true;
			}
		}
	}

	CString strMergeOverride = "NULL";
	if (m_strMergeFieldOverride.GetLength()) {
		strMergeOverride.Format("'%s'",_Q(m_strMergeFieldOverride));
	}

	CString strSourceTemplateID = "NULL";
	if (m_nEMRSourceTemplateID != -1) {
		strSourceTemplateID.Format("%li",(long)m_nEMRSourceTemplateID);
	}



	CString strSourceActionID = "NULL";
	if(m_sai.nSourceActionID != -1) {
		strSourceActionID.Format("%li", m_sai.nSourceActionID);
	}

	// (j.jones 2007-01-10 14:57) - PLID 24027 - supported SourceDetailID
	CString strSourceDetailID = "NULL";
	if(m_sai.nSourceDetailID != -1) {
		strSourceDetailID.Format("%li", m_sai.nSourceDetailID);
	}

	// (j.jones 2010-02-12 11:07) - PLID 37318 - supported ChildEMRDetailID
	CString strChildEMRDetailID = "NULL";
	if(m_pSmartStampTableDetail != NULL && m_pSmartStampTableDetail->m_nEMRDetailID != -1) {
		//we have a linked table detail and it has an ID, so simply save the ID
		strChildEMRDetailID.Format("%li", m_pSmartStampTableDetail->m_nEMRDetailID);
	}

	// (z.manning 2009-03-11 10:42) - PLID 33338 - SourceDataGroupID
	CString strSourceDataGroupID = "NULL";
	if(m_sai.GetDataGroupID() != -1) {
		strSourceDataGroupID = AsString(m_sai.GetDataGroupID());
	}
	// (z.manning 2010-02-26 16:36) - PLID 37540
	CString strSourceDetailImageStampID = "NULL";
	if(m_sai.GetDetailStampID() != -1) {
		strSourceDetailImageStampID = AsString(m_sai.GetDetailStampID());
	}

	// (z.manning 2008-10-08 15:12) - PLID 31613 - Lab ID
	CString strLabID = "NULL";
	if(m_varLabID.vt == VT_I4) {
		strLabID = AsString(m_varLabID);
	}

	if(!bIsLockedAndSaved)
	{
		// (c.haag 2007-06-14 12:50) - PLID 26338 - Insert everything at once if we're adding a new detail
		if(m_nEMRDetailID == -1)
		{
			// (c.haag 2007-06-13 12:22) - PLID 26316 - EMRDetailsT now has an identity column, so no need to fill out the ID
			// (c.haag 2008-06-16 12:10) - PLID 30319 - If we're based off an EMR text macro, then we need to save our label text
			// to the MacroName field.
			// (a.walling 2008-06-30 14:42) - PLID 29271 - Support Preview Pane flags
			// (z.manning 2008-10-08 15:14) - PLID 31613 - Added LabID
			// (z.manning 2009-03-11 10:43) - PLID 33338 - SourceDataGroupID
			// (j.jones 2010-02-12 11:07) - PLID 37318 - supported ChildEMRDetailID
			// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize - Use variants and COALESCE to eliminate the need to conditionally include updates to coordinates
			// (c.haag 2007-06-13 12:24) - PLID 26277 - Use the scope identity function to get the ID of the newly created record
			// (j.armen 2014-07-23 16:34) - PLID 63026 - Save the default pen color / size
			AddStatementToSqlBatch(strSaveString, R"(
INSERT INTO EMRDetailsT (
	EMRID, EMRInfoID, OrderID, EMRTopicID, MergeOverride, MacroName, SourceTemplateID, SourceActionID, 
	SourceDetailID, PreviewFlags, X, Y, Width, Height, LabID, SourceDataGroupID, ChildEMRDetailID, 
	SourceDetailImageStampID, InkPenColor, InkPenSizePercent)
VALUES (%s, %li, %li, %s, %s, %s, %s, %s, %s, %li, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s)

SET @nNewObjectID = SCOPE_IDENTITY()
SET @nEMRDetailID = @nNewObjectID
)"
				, strEMNID, m_nEMRInfoID, nOrderID, strTopicID, strMergeOverride, 
				(EMR_BUILT_IN_INFO__TEXT_MACRO == m_nEMRInfoID) ? (CString("'") + _Q(GetLabelText()) + "'") : "NULL",
				strSourceTemplateID, strSourceActionID, strSourceDetailID, m_nPreviewFlags, AsStringForSql(varPosX), 
				AsStringForSql(varPosY), AsStringForSql(varPosWidth), AsStringForSql(varPosHeight), strLabID, 
				strSourceDataGroupID, strChildEMRDetailID, strSourceDetailImageStampID,
				m_nDefaultPenColor ? AsStringForSql(*m_nDefaultPenColor) : "NULL",
				m_nDefaultPenSizePercent ? AsStringForSql(*m_nDefaultPenSizePercent) : "NULL");

			// (c.haag 2007-06-13 12:24) - PLID 26277 - This is no longer necessary now that we get these values after the insert
			//AddStatementToSqlBatch(strSaveString, "SET @nNewObjectID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM EMRDetailsT WITH(UPDLOCK, HOLDLOCK))");
			AddNewEMRObjectToSqlBatch(strSaveString, esotDetail, (long)this, mapSavedObjects);
			//AddStatementToSqlBatch(strSaveString, "SET @nEMRDetailID = @nNewObjectID");
			strID = "@nEMRDetailID";

			const int nTableRows = m_arTableRows.GetSize();
			const int nTableColumns = m_arTableColumns.GetSize();

			// (j.jones 2010-06-21 16:55) - PLID 37981 - if this is a new generic table, save its row & column names
			if(m_nEMRDetailID == -1 && IsGenericTable()) {
				AddStatementToSqlBatch(strSaveString, "SET @nEMRDetailID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotDetail, (long)this);
				for(int nRow = 0; nRow < nTableRows; nRow++) {
					TableRow *ptrRow = GetRowPtr(nRow);			
					AddStatementToSqlBatch(strSaveString, "INSERT INTO EMRDetailTableOverrideDataT (EMRDetailID, EMRDataID, Name) "
						"VALUES (@nEMRDetailID, %li, '%s')", ptrRow->m_ID.nDataID, _Q(ptrRow->strName));
				}
				for(int nColumn = 0; nColumn < nTableColumns; nColumn++) {
					TableColumn tcColumn = GetColumn(nColumn);
					AddStatementToSqlBatch(strSaveString, "INSERT INTO EMRDetailTableOverrideDataT (EMRDetailID, EMRDataID, Name) "
						"VALUES (@nEMRDetailID, %li, '%s')", tcColumn.nID, _Q(tcColumn.strName));
				}
			}

			// (j.jones 2007-01-15 13:39) - PLID 24027 - if generated from a template detail, save that detail ID
			// (c.haag 2008-09-09 10:25) - PLID 24670 - It may be the case that someone deleted the template detail
			// at some point in the past. If so, then we don't want to add a record to EMRTemplateDetailToDetailLinkT.
			// The detail will then be template-independent in data since there's nothing in the template to bind to.
			if(m_nEMRTemplateDetailID != -1) {
				AddStatementToSqlBatch(strSaveString, 
					"INSERT INTO EMRTemplateDetailToDetailLinkT (EMRDetailID, EMRTemplateDetailID) "
					"SELECT @nEMRDetailID, EMRTemplateDetailsT.ID "
					"FROM EMRTemplateDetailsT "
					"WHERE EMRTemplateDetailsT.ID = %li ", m_nEMRTemplateDetailID);
			}

			// (j.jones 2007-01-11 14:41) - PLID 24027 - if our source detail ID is -1, then
			// be sure to update SourceDetailID at the end of the save!
			if(m_sai.pSourceDetail && m_sai.nSourceDetailID == -1) {
				//find this detail's ID, find the source detail's ID, and update this detail's SourceDetailID
				AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRObjectIDToUpdate = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), -1)", esotDetail, (long)this);
				AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRSourceDetailID = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), NULL)", esotDetail, (long)m_sai.pSourceDetail);
				AddStatementToSqlBatch(strPostSaveSql, "UPDATE EMRDetailsT SET SourceDetailID = @nEMRSourceDetailID WHERE ID = @nEMRObjectIDToUpdate");
			}

			// (z.manning 2010-02-26 17:12) - PLID 37540 - Handle source detail stamp pointer
			if(m_sai.GetDetailStampPointer() != NULL && m_sai.GetDetailStampID() == -1) {
				AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRObjectIDToUpdate = COALESCE((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), -1)", esotDetail, (long)this);
				AddStatementToSqlBatch(strPostSaveSql, "SET @nSourceDetailImageStampID = COALESCE((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), NULL)", esotDetailImageStamp, (long)m_sai.GetDetailStampPointer());
				AddStatementToSqlBatch(strPostSaveSql, "UPDATE EMRDetailsT SET SourceDetailImageStampID = @nSourceDetailImageStampID WHERE ID = @nEMRObjectIDToUpdate");
			}

			// (j.jones 2010-02-12 11:07) - PLID 37318 - supported ChildEMRDetailID, if the linked table is -1,
			// we will have to update later on
			if(m_pSmartStampTableDetail != NULL && m_pSmartStampTableDetail->m_nEMRDetailID == -1) {
				//we have a linked table detail and it has an ID, so simply save the ID
				AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRObjectIDToUpdate = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), -1)", esotDetail, (long)this);
				AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRChildEMRDetailID = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), NULL)", esotDetail, (long)m_pSmartStampTableDetail);
				AddStatementToSqlBatch(strPostSaveSql, "UPDATE EMRDetailsT SET ChildEMRDetailID = @nEMRChildEMRDetailID WHERE ID = @nEMRObjectIDToUpdate");
			}

		}
		else
		{
			//now we have our shared data, so update that now
			// (a.walling 2008-06-30 14:43) - PLID 29271 - Update Preview Pane flags
			// (z.manning 2008-10-08 15:16) - PLID 31613 - LabID
			// (j.jones 2010-02-12 11:07) - PLID 37318 - supported ChildEMRDetailID
			// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize - Use variants and COALESCE to eliminate the need to conditionally include updates to coordinates
			AddStatementToSqlBatch(strSaveString, 
				"UPDATE EMRDetailsT SET OrderID = %li, EMRTopicID = %s, "				
				"	X = COALESCE(%s, X), Y = COALESCE(%s, Y), Width = COALESCE(%s, Width), Height = COALESCE(%s, Height), "
				"	MergeOverride = %s, "
				"	SourceTemplateID = %s, SourceActionID = %s, SourceDetailID = %s, ChildEMRDetailID = %s, "
				"	PreviewFlags = %li, LabID = %s "
				"WHERE ID = %s "
				, nOrderID, strTopicID
				, AsStringForSql(varPosX), AsStringForSql(varPosY), AsStringForSql(varPosWidth), AsStringForSql(varPosHeight) 
				, strMergeOverride, strSourceTemplateID
				, strSourceActionID, strSourceDetailID, strChildEMRDetailID, m_nPreviewFlags, strLabID
				, strID);

			// (c.haag 2008-07-15 15:52) - PLID 17244 - Update EMR detail todo alarm text because the label text may have changed. This may
			// have already been done in SetLabelText, but we want to have a catch-all for ensuring the todos are up to date with the detail
			// name.
			if (m_oLastSavedDetail && m_oLastSavedDetail->GetLabelText() != GetLabelText()) {
				// (a.walling 2010-11-08 12:12) - PLID 40965 - Flatten parameterized query
				// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize
				AddStatementToSqlBatch(strSaveString, TodoGetEmrDetailAlarmUpdateQ(-1, m_nEMRDetailID));
			}

			// (j.jones 2010-02-12 11:07) - PLID 37318 - supported ChildEMRDetailID, if the linked table is -1,
			// we will have to update later on
			if(m_pSmartStampTableDetail != NULL && m_pSmartStampTableDetail->m_nEMRDetailID == -1 && m_nEMRDetailID != -1) {
				//we have a linked table detail and it has an ID, so simply save the ID
				AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRChildEMRDetailID = Coalesce((SELECT ID FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li), NULL)", esotDetail, (long)m_pSmartStampTableDetail);
				AddStatementToSqlBatch(strPostSaveSql, "UPDATE EMRDetailsT SET ChildEMRDetailID = @nEMRChildEMRDetailID WHERE ID = %li", m_nEMRDetailID);
			}
		}
	}

	// (c.haag 2007-06-14 12:44) - PLID 26338 - Moved all problem code to here. This is the earliest place in
	// GenerateSaveString_PatientEMN 

	// (j.jones 2006-08-24 11:06) - PLID 22183 - For the locked-EMN-save-prevention, I am intentionally
	// ignoring problems, because while the interface doesn't allow modifications to problems on locked EMNs,
	// I suspect that may change in the future

	// (c.haag 2006-06-30 09:00) - PLID 19977 - Insert a new record for the detail problem if one exists
	long nPatientID = m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID();
	CString strPatientName = GetExistingPatientName(nPatientID);

	// (j.jones 2008-07-21 08:46) - PLID 30779 - save all our problems
	// (z.manning 2009-05-22 09:22) - PLID 34297 - Save problem links
	SaveProblemLinkArray(strSaveString, m_apEmrProblemLinks, strID, mapSavedObjects, nAuditTransactionID, nPatientID, strPatientName);

	//for later auditing of creation on new EMNs, set the boolean now
	if(nEMNID == -1 && m_nEMRDetailID == -1) {
		m_bCreatedOnNewEMN = TRUE;
	}

	//while we're at it, audit the changes to shared data

	//if the entire EMN is new, don't audit anything at all
	if(nEMNID != -1) {

		//in addition to auditing, we use this code to determine what was attempted to be
		//changed on a locked EMN, and warn accordingly

		CString strOldDetailName, strOldDetailData, strNewDetailName, strNewDetailData;		

		// (c.haag 2008-06-05 13:22) - PLID 27240 - If this item was brought up to date, so was the last saved detail.
		// So, pull the audit text from before it was brought up to date.
		if (m_bDetailWasBroughtUpToDate) {
			strOldDetailName = m_strPreUpdatedDetailName;
			strOldDetailData = m_strPreUpdatedDetailData;
		}
		else {
			if(m_oLastSavedDetail) {
				m_oLastSavedDetail->GetDetailNameAndDataForAudit(strOldDetailName,strOldDetailData);
			}
			else {
				ASSERT(m_nEMRDetailID == -1);
			}
		}

		GetDetailNameAndDataForAudit(strNewDetailName,strNewDetailData);

		if(m_nEMRDetailID == -1) {
			if(!bIsLockedAndSaved) {
				//detail added				
				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();

				CString strNewValue;
				strNewValue.Format("Detail: %s, Data: %s",strNewDetailName,strNewDetailData);
				
				// (j.jones 2009-06-01 15:03) - PLID 34431 - if this is a signature, use a unique audit for it
				if(IsSignatureDetail()) {					
					// (j.jones 2013-08-07 15:43) - PLID 42958 - added audits for when a user gets another user to sign their EMR
					if(m_bIsSignatureAddedByAnotherUser) {
						CString strUser;
						strUser.Format(", Signed By User: %s", m_strSignatureAddedByAnotherUsername);
						strNewValue += strUser;
						AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiEMNSignatureDetailAddedToExistingEMNByAnotherUser, nEMNID, "", strNewValue, aepHigh, aetCreated);
					}
					else {
						AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiEMNSignatureDetailAddedToExistingEMN, nEMNID,"", strNewValue, aepHigh, aetCreated);
					}
				}
				else {				
					//normal detail
					AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNDetailAddedToExistingEMN,nEMNID,"",strNewValue,aepHigh,aetCreated);
				}
			}
			//we already generated the "locked" message earlier
		}
		else {
			//detail changed
			CString strOldValue, strNewValue;
			strOldValue.Format("Detail: %s, Data: %s",strOldDetailName,strOldDetailData);
			strNewValue.Format("Detail: %s, Data: %s",strNewDetailName,strNewDetailData);

			// (c.haag 2008-06-05 13:28) - PLID 27240 - Check to see whether the detail was brought up to date
			if (m_bDetailWasBroughtUpToDate) {
				CString strNewValue;
				strNewValue.Format("Detail: %s, Data: %s",strNewDetailName,strNewDetailData);
				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();
				AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNDetailBroughtUpToDate,nEMNID,strOldValue,strNewValue,aepMedium,aetChanged);
			}

			// (j.jones 2006-08-21 14:11) - PLID 22092 - if only the merge override name
			// changed, audit separately and don't say the detail itself changed
			BOOL bMergeNameChanged = FALSE;
			if(GetMergeFieldName(FALSE) != m_oLastSavedDetail->GetMergeFieldName()) {
				//the name is different, but is it the item name, or just the merge field name?
				if(m_strLabelText == m_oLastSavedDetail->m_strLabelText) {
					//if we get here, the name that merged has changed from before,
					//but the name of the detail has not changed, thus we know
					//the merge name changed in some way (this includes the addition
					//or removal of the merge name override)
					bMergeNameChanged = TRUE;
				}
			}

			if(bMergeNameChanged) {

				if(!bIsLockedAndSaved) {
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNDetailMergeName,nEMNID,strOldValue,strNewValue,aepHigh,aetChanged);
				}
				else {
					//we tried to change this on a locked EMN!
					CString str;
					str.Format("Attempted to change the merge name from '%s' to '%s' on a detail on a locked EMN.", strOldDetailName, strNewDetailName);
					arystrErrors.Add(str);
				}
			}

			BOOL bImageInkChanged = FALSE;
			// (r.gonet 05/02/2012) - PLID 49946 - Added text modified
			if(m_EMRInfoType == eitImage && (m_bInkHasBeenAdded || m_bInkHasBeenErased || m_bImageTextHasBeenAdded || m_bImageTextHasBeenRemoved || m_bImageTextHasBeenModified)) {
				bImageInkChanged = TRUE;
			}

			//if the detail name or data changed, audit as such
			if(m_strLabelText != m_oLastSavedDetail->m_strLabelText) {

				if(!bIsLockedAndSaved) {
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNDetailChanged,nEMNID,strOldValue,strNewValue,aepHigh,aetChanged);
				}
				else {
					//we tried to change this on a locked EMN!
					CString str;
					str.Format("Attempted to change a detail's name from '%s' to '%s' on a locked EMN.", m_oLastSavedDetail->m_strLabelText, m_strLabelText);
					arystrErrors.Add(str);
				}
			}
			else if(strOldDetailData != strNewDetailData || bImageInkChanged) {

				// (j.jones 2006-08-18 16:03) - PLID 22058 - for images, we audit if the image changed
				// (which is the strNewDetailData) or if the ink changed
				if(bImageInkChanged) {
					// (r.gonet 05/02/2012) - PLID 49946 - Added text modified
					strNewValue.Format("Detail: %s, Data: %s%s%s%s%s%s",strNewDetailName,strNewDetailData,
						m_bInkHasBeenAdded ? " (Ink Was Added)" : "",
						m_bInkHasBeenErased ? " (Ink Was Erased)" : "",
						m_bImageTextHasBeenAdded ? " (Text Was Added)" : "",
						m_bImageTextHasBeenRemoved ? " (Text Was Removed)" : "",
						m_bImageTextHasBeenModified ? "(Text Was Modified)" : "");
				}

				if(!bIsLockedAndSaved) {
					//if the detail data changed, audit that, but don't audit if we audit changing the detail itself
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();					
					AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNDetailDataChanged,nEMNID,strOldValue,strNewValue,aepHigh,aetChanged);
				}
				else {
					//we tried to change this on a locked EMN!
					CString str;
					str.Format("Attempted to change a detail's data on a locked EMN. Old Value: '%s'; New Value: '%s'", strOldValue, strNewValue);
					arystrErrors.Add(str);
				}
			}

			//see if we moved it between topics
			if(m_pOriginalParentTopic != m_pParentTopic) {

				if(!bIsLockedAndSaved) {
					CString strOldTopicName, strNewTopicName;
					if(!m_strLastSavedParentTopicName.IsEmpty())
						strOldTopicName = "Topic: " + m_strLastSavedParentTopicName;
					strNewTopicName = "Topic: " + m_pParentTopic->GetName();
				
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNDetailTopicMoved,nEMNID,strOldTopicName,strNewTopicName,aepMedium,aetChanged);
				}
				else {
					//we tried to change this on a locked EMN!
					CString str;
					str.Format("Attempted to move detail '%s' from topic '%s' to topic '%s' on a locked EMN.", GetMergeFieldName(FALSE), m_strLastSavedParentTopicName, m_pParentTopic->GetName());
					arystrErrors.Add(str);
				}
			}

			//see if we moved its location on a topic

			long nOldX = -1, nOldY = -1, nOldWidth = -1, nOldHeight = -1;
			if(m_oLastSavedDetail) {
				if(!m_oLastSavedDetail->m_rcDefaultClientArea.IsRectNull()) {
					nOldX = (long)m_oLastSavedDetail->m_rcDefaultClientArea.left;
					nOldY = (long)m_oLastSavedDetail->m_rcDefaultClientArea.top;
					nOldWidth = (long)m_oLastSavedDetail->m_rcDefaultClientArea.Width();
					nOldHeight = (long)m_oLastSavedDetail->m_rcDefaultClientArea.Height();
				}
			}
			else {
				ASSERT(m_nEMRDetailID == -1);
			}

			//JMJ - for the time being, we will still audit this even if it moved topic, but we may consider not doing so at some point
			// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize - Use variants and COALESCE to eliminate the need to conditionally include updates to coordinates
			if(bUpdateCoords &&
				((nOldX != -1 && nOldX != (long)varPosX) || (nOldY != -1 && nOldY != (long)varPosY))) {

				if(!bIsLockedAndSaved) {
					//JMJ - I see no reason to give them coordinates, so just audit that it was moved
					CString strNewValue;
					strNewValue.Format("Detail: %s, Data: %s",strNewDetailName,strNewDetailData);
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNDetailPlacementMoved,nEMNID,strNewValue,"Moved",aepLow,aetChanged);
				}
				else {
					//we tried to change this on a locked EMN!
					CString str;
					str.Format("Attempted to move the placement of detail '%s' on a locked EMN.", GetMergeFieldName(FALSE));
					arystrErrors.Add(str);
				}
			}
				
			//last, check to see if we resized the detail
			// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize - Use variants and COALESCE to eliminate the need to conditionally include updates to coordinates
			if(bUpdateCoords &&
				((nOldWidth != -1 && nOldWidth != (long)varPosWidth) || (nOldHeight != -1 && nOldHeight != (long)varPosHeight))) {

				if(!bIsLockedAndSaved) {
					//JMJ - I see no reason to give them coordinates, so just audit that it was resized
					CString strNewValue;
					strNewValue.Format("Detail: %s, Data: %s",strNewDetailName,strNewDetailData);
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(nPatientID, strPatientName,nAuditTransactionID,aeiEMNDetailResized,nEMNID,strNewValue,"Resized",aepLow,aetChanged);
				}
				else {
					//we tried to change this on a locked EMN!
					CString str;
					str.Format("Attempted to resize detail '%s' on a locked EMN.", GetMergeFieldName(FALSE));
					arystrErrors.Add(str);
				}
			}
		}
	}

	//now we need to save the changes to type-specific data
	strSaveString += GenerateSaveString_State(strID, arystrErrors, strPostSaveSql, &mapSavedObjects);

	// (j.jones 2006-08-29 08:57) - PLID 22250 - update the EMN's modified date
	if(IsModified() && nEMNID != -1 && !bIsLockedAndSaved) {
		AddStatementToSqlBatch(strSaveString, "UPDATE EMRMasterT SET ModifiedDate = GetDate() WHERE ID = %li", nEMNID);
	}

	// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
	// We've just added ourselves to the save string, so if we weren't new, then we know we 
	// were "modified" by the save.
	if (m_nEMRDetailID != -1) {
		arynModifiedEMRDetailIDs.Add(m_nEMRDetailID);
	}

	return strSaveString;
}

// (z.manning 2010-02-18 14:53) - PLID 37404 - Added pmapSavedObjects parameter
// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize
Nx::Quantum::Batch CEMNDetail::GenerateSaveString_State(const CString& strID, CStringArray &arystrErrors, Nx::Quantum::Batch& strPostSaveSql, IN CMapPtrToPtr *pmapSavedObjects)
{
	// (c.haag 2007-01-03 10:07) - PLID 22849 - This was copied out of GenerateSaveString_PatientEMN
	Nx::Quantum::Batch strSaveString;

	// (c.haag 2007-02-01 15:47) - PLID 22849 - We now check if the detail is locked.
	if (!GetIsOnLockedAndSavedEMN()) {

		switch (m_EMRInfoType) {
		case eitText:
			{
				CString strState;
				if(IsLabDetail()) {
					// (z.manning 2008-10-30 09:43) - PLID 31613 - Update lab details by pulling the text from
					// LabsT. You may be wondering why we save the text at all instead of always just loading
					// from LabsT and the reason is to prevent lab details from changing on locked EMNs.
					//
					// IMPORTANT NOTE: The below SQL statement must be kept in sync with EMNLab::GetText().
					//TES 11/10/2009 - PLID 36260 - Replaced AnatomySide with AnatomyQualifierID
					//TES 12/8/2009 - PLID 36512 - Restored AnatomySide
					// (z.manning 2010-03-26 09:10) - PLID 37553 - Updated the way we format anatomic location
					AddStatementToSqlBatch(strSaveString,
						"SET @strLabDetailText = ( \r\n"
						"	SELECT CASE WHEN LabsT.Type = %i THEN \r\n"
						"		CASE WHEN LabAnatomicLocationQ.AnatomicLocation = '' THEN '' ELSE 'Anatomic Location:' + char(13) + char(10) END + \r\n"
						"		LabAnatomicLocationQ.AnatomicLocation \r\n"
						"	ELSE CASE WHEN ToBeOrdered = '' THEN '' ELSE 'To be ordered:' + char(13) + char(10) + ToBeOrdered END \r\n"
						"	END + \r\n"
						"	CASE WHEN COALESCE(convert(nvarchar(1000), ClinicalData), '') = '' THEN '' ELSE char(13) + char(10) + char(13) + char(10) + 'Comments:' + char(13) + char(10) + convert(nvarchar(4000), ClinicalData) END \r\n"
						"	FROM LabsT \r\n"
						"	LEFT JOIN LabAnatomicLocationQ ON LabsT.ID = LabAnatomicLocationQ.LabID \r\n"
						"	WHERE LabsT.ID = %li \r\n"
						") \r\n"
						, ltBiopsy, VarLong(GetLabID()));
					strState = "@strLabDetailText";
				}
				else {
					strState = "'" + _Q(VarString(m_varState, "")) + "'";
				}
				AddStatementToSqlBatch(strSaveString, "UPDATE EMRDetailsT SET Text = %s WHERE ID = %s",
					strState, strID);
			}
			break;
		case eitSingleList:
		case eitMultiList:
			{
				BOOL bInsertDetailSortOrders = FALSE;
				if (m_nEMRDetailID == -1)
				{
					//Make sure content is loaded (this will do nothing if it's already loaded).
					LoadContent();
					for(int nElement = 0; nElement < m_arListElements.GetSize(); nElement++) {
						ListElement le = m_arListElements[nElement];
						if(le.bIsSelected) {
							AddStatementToSqlBatch(strSaveString, "INSERT INTO EmrSelectT (EmrDetailID, EmrDataID) "
								"VALUES (%s, %li)", strID, le.nID);
						}
					}
					
					// (z.manning 2011-04-05 16:48) - PLID 43140 - This is a new detail so we need to save the sort orders
					bInsertDetailSortOrders = TRUE;
				}
				else
				{
					//First, make sure we have the right list of EmrDetailElementsT.
					//TES 2/27/2008 - PLID 29096 - Checking for the AdvDlg here appears to be an obsolete requirement,
					// since none of the code in this branch actually uses the AdvDlg.  Don added an exception to the
					// else branch, but we found at least one legit way for that branch to be reached, so we just
					// took out the conditional altogether, since it didn't serve any purpose.
					//if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
						BOOL bElementsChanged = TRUE;
						BOOL bSelectChanged = TRUE;
						BOOL bSortOrderChanged = TRUE;
						if(m_oLastSavedDetail && m_oLastSavedDetail->m_listElements.size() == m_arListElements.GetSize())
						{
							//TES 6/12/2006 - Now that we have a way of checking the original state of our list, don't do anything
							//if it hasn't changed.
							bElementsChanged = FALSE;
							bSelectChanged = FALSE;
							bSortOrderChanged = FALSE;
							for(int nElement = 0; nElement < m_arListElements.GetSize(); nElement++)
							{
								// (a.walling 2012-10-31 17:17) - PLID 53551 - Compare with the LastSavedDetail SimpleListElement structure
								const Emr::SimpleListElement& leOld = m_oLastSavedDetail->m_listElements[nElement];
								ListElement leNew = m_arListElements[nElement];
								if(leOld.nID != leNew.nID) {
									bElementsChanged = TRUE;
								}
								else if(leOld.bIsSelected != leNew.bIsSelected) {
									bSelectChanged = TRUE;
								}

								// (z.manning 2011-04-05 16:55) - PLID 43140 - The detail sort order saves by data group ID so see
								// if it changed.
								if(leOld.nDataGroupID != leNew.nDataGroupID) {
									bSortOrderChanged = TRUE;
								}

								if(bElementsChanged && bSortOrderChanged) {
									// (z.manning 2011-04-05 17:19) - PLID 43140 - We've found all the changes we were looking for,
									// so no point in looping any further.
									break;
								}
							}
						}

						if(bElementsChanged) {
							bSelectChanged = TRUE; //If the elements changed, we can't tell whether the selection changed, so recreated.
						}

						if(bSortOrderChanged) {
							// (z.manning 2011-04-05 16:49) - PLID 43140 - The list elements' sort orders changed so we need to re-save them.
							AddStatementToSqlBatch(strSaveString, "DELETE FROM EmrDetailListOrderT WHERE EmrDetailID = %s", strID);
							bInsertDetailSortOrders = TRUE;
						}

						if(bSelectChanged) {
							//TES 10/27/2004: They may have been reordered.  Delete the whole thing, and start over.
							AddStatementToSqlBatch(strSaveString, "DELETE FROM EmrSelectT WHERE EmrDetailID = %s", strID);
						}
						for(int nElement = 0; nElement < m_arListElements.GetSize(); nElement++) {
							ListElement le = m_arListElements[nElement];
							if(bSelectChanged && le.bIsSelected) {
								AddStatementToSqlBatch(strSaveString, "INSERT INTO EmrSelectT (EmrDetailID, EmrDataID) "
									"VALUES (%s, %li)",	strID, le.nID);
							}
						}
					/*}
					else {
						//DRT 2/14/2008 - PLID 28941 - I noticed this while setting up hotspots below.  If we don't have a valid ItemAdvDlg, this
						//	code was just blindly throwing away the save information and going on its merry way!  We can't just let data vanish for
						//	no reason.  I don't think this code should ever be hit, but just in case, we need to know about it.
						ThrowNxException("Could not access the interface for saving list data.");
					}*/
				}

				// (z.manning 2011-04-05 17:22) - PLID 43140 - If we need to insert the detail sort orders 
				if(bInsertDetailSortOrders)
				{
					for(int nListElementIndex = 0; nListElementIndex < m_arListElements.GetSize(); nListElementIndex++)
					{
						ListElement le = m_arListElements.GetAt(nListElementIndex);
						AddStatementToSqlBatch(strSaveString,
							// (j.jones 2011-04-29 10:46) - PLID 43122 - added IsFloated
							"INSERT INTO EmrDetailListOrderT (EmrDetailID, EmrDataGroupID, OrderIndex, IsFloated) VALUES (%s, %li, %li, %li) "
							, strID, le.nDataGroupID, nListElementIndex + 1, le.bIsFloated ? 1 : 0);
					}
				}
			}
			break;
		case eitImage:
			{
				CEmrItemAdvImageState ais;
				ais.CreateFromSafeArrayVariant(m_varState);
				_variant_t varInkData(ais.m_varInkData);
				if (varInkData.vt == VT_EMPTY) {
					varInkData.vt = VT_NULL;					
				}
				_variant_t varTextData(ais.m_varTextData);
				if (varTextData.vt == VT_EMPTY) {
					varTextData.vt = VT_NULL;
				}
				
				if(varInkData.vt == VT_NULL) {
					AddStatementToSqlBatch(strSaveString, "UPDATE EMRDetailsT SET InkData = NULL WHERE ID = %s", strID);
				}
				else {
					CString strImage = ais.CreateByteStringFromInkData();
					AddStatementToSqlBatch(strSaveString, "UPDATE EMRDetailsT SET InkData = %s WHERE ID = %s",
						strImage, strID);
				}
				if(varTextData.vt == VT_NULL) {
					AddStatementToSqlBatch(strSaveString, "UPDATE EMRDetailsT SET ImageTextData = NULL WHERE ID = %s", strID);
				}
				else {
					CString strImage = ais.CreateByteStringFromTextData();
					AddStatementToSqlBatch(strSaveString, "UPDATE EMRDetailsT SET ImageTextData = %s WHERE ID = %s",
						strImage, strID);
				}

				// (z.manning 2011-10-05 16:40) - PLID 45742 - Handle 3D image print data
				if(Is3DImage())
				{
					// (j.armen 2014-01-28 10:23) - PLID 60497 - Quantum saving - Handle NULL directly
					if(ais.m_varPrintData.vt != VT_NULL && ais.m_varPrintData.vt != VT_EMPTY) {
						CString strPrintData = CreateByteStringFromSafeArrayVariant(ais.m_varPrintData);
						AddStatementToSqlBatch(strSaveString, "UPDATE EmrDetailsT SET PrintData = %s WHERE ID = %s", strPrintData, strID);
					}
					else {
						AddStatementToSqlBatch(strSaveString, "UPDATE EmrDetailsT SET PrintData = NULL WHERE ID = %s", strID);
					}
				}

				/* (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore zoom and pan offsets.
				AddStatementToSqlBatch(strSaveString,
					"UPDATE EMRDetailsT "
					"SET ZoomLevel = %g, OffsetX = %li, OffsetY = %li "
					"WHERE ID = %s", 
					ais.m_dZoomLevel, ais.m_nOffsetX, ais.m_nOffsetY, strID);
				*/

				// (d.thompson 2009-03-05 09:45) - PLID 32891 - If we have the special "forced blank" type, then
				//	we do want to save a blank override path.
				if (!ais.m_strImagePathOverride.IsEmpty() || ais.m_eitImageTypeOverride == itForcedBlank) {
					AddStatementToSqlBatch(strSaveString, "UPDATE EMRDetailsT SET "
						"InkImagePathOverride = '%s', InkImageTypeOverride = %li WHERE ID = %s",
						_Q(ais.m_strImagePathOverride), (long)ais.m_eitImageTypeOverride, strID);
				}
				else {
					AddStatementToSqlBatch(strSaveString, "UPDATE EMRDetailsT SET "
						"InkImagePathOverride = NULL, InkImageTypeOverride = NULL WHERE ID = %s", strID);
				}

				//TES 1/28/2005 - PLID 15403 - Try to attach this to this PIC.
				long nEMRGroupID = m_pParentTopic->GetParentEMN()->GetParentEMR()->GetID();
				long nPatientID = m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID();
				if (nPatientID != -1) {
					if(ais.m_eitImageTypeOverride == itPatientDocument) {
						// (c.haag 2009-09-01 15:55) - PLID 34278 - Search the PathName field for the overide path;
						// but also do a search where it expects a prepended slash.
						_RecordsetPtr rsExistingHistory = CreateRecordset("SELECT MailID FROM MailSent "
							"WHERE PersonID = %li AND (PathName = '%s' OR ('\\' + PathName = '%s'))  AND PicID Is Null",
							nPatientID, _Q(ais.m_strImagePathOverride), _Q(ais.m_strImagePathOverride));
						if(!rsExistingHistory->eof) {
							if (nEMRGroupID != -1) {
								// This is an existing EMR. We can assign the PicID to ti.
								AddStatementToSqlBatch(strSaveString, "UPDATE MailSent SET PicID = %li WHERE MailID = %li", VarLong(GetTableField("PicT", "ID", "EmrGroupID", nEMRGroupID)), AdoFldLong(rsExistingHistory, "MailID"));
							}
							else if (NULL != m_pParentTopic) {
								// This must be a new EMR. Try to get the PIC ID from the topmost parent.
								CEmrTreeWnd* pTreeWnd = m_pParentTopic->GetParentEMN()->GetInterface();
								// (a.walling 2011-10-20 14:23) - PLID 46071 - Liberating window hierarchy dependencies among EMR interface components
								if (pTreeWnd && pTreeWnd->GetPicContainer()) {
									AddStatementToSqlBatch(strSaveString, "UPDATE MailSent SET PicID = %li WHERE MailID = %li", pTreeWnd->GetPicContainer()->GetCurrentPicID(), AdoFldLong(rsExistingHistory, "MailID"));
								}
							}
						}
					}
				}

				//DRT 1/23/2008 - PLID 28697 - We need to save the hotspot state
				//Fill an array with all the IDs that are selected
				CDWordArray arySelectedData;
				ParseDelimitedStringToDWordArray(ais.m_strSelectedHotSpotData, ";", arySelectedData);
				if (m_nEMRDetailID == -1) {
					//We are saving a new detail.  Don't need to worry about any old data.  
					//	Make sure content is loaded (this will do nothing if it's already loaded).
					LoadContent();

					//Then simply generate an SQL statement
					for(int i = 0; i < arySelectedData.GetSize(); i++) {
						AddStatementToSqlBatch(strSaveString, "INSERT INTO EMRHotSpotSelectT (EMRDetailID, EMRImageHotSpotID) "
									"VALUES (%s, %li)", strID, arySelectedData.GetAt(i));
					}
				}
				else {
					//In this case, we're updating an existing select list.  We need to figure out which elements have
					//	been newly added and which have been removed.

					//First, make sure we have the right list of EmrDetailElementsT.
					//TES 2/27/2008 - PLID 29096 - Checking for the AdvDlg here appears to be an obsolete requirement,
					// since none of the code in this branch actually uses the AdvDlg.  Don added an exception to the
					// else branch, but we found at least one legit way for that branch to be reached, so we just
					// took out the conditional altogether, since it didn't serve any purpose.
					//if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
						if(m_oLastSavedDetail) {
							//From here we have the old & current.  Figure out what differs
							CDWordArray aryNewIDs, aryRemovedIDs;
							ForSave_CalculateChangedHotSpots(&arySelectedData, &m_oLastSavedDetail->m_selectedHotSpotIDs, &aryNewIDs, &aryRemovedIDs);

							//At this point we know what changed, so write DELETE statements for the removed, and
							//	INSERT statements for the new.
							int i = 0;
							for(i = 0; i < aryNewIDs.GetSize(); i++) {
								AddStatementToSqlBatch(strSaveString, "INSERT INTO EMRHotSpotSelectT (EMRDetailID, EMRImageHotSpotID) "
											"VALUES (%s, %li)", strID, aryNewIDs.GetAt(i));
							}

							for(i = 0; i < aryRemovedIDs.GetSize(); i++) {
								AddStatementToSqlBatch(strSaveString, "DELETE FROM EMRHotSpotSelectT WHERE EMRDetailID = %s "
									"AND EMRImageHotSpotID = %li ", strID, aryRemovedIDs.GetAt(i));
							}
						}
						else {
							//If we do not have a last saved detail, there's no way we can accurately determine what changed.  I don't think
							//	this case should ever happen, but if we find some legitimate causes, we *could* use the functionality like
							//	the select lists, and just delete everything and re-insert.
							ThrowNxException("Could not determine previously saved information when attempting to save HotSpot data.");
						}
					/*}
					else {
						//I really don't know what this case means.  I copied most of the hotspot code from the select lists, and they check
						//	for this.  However, if you hit this case in the select lists, it just blindly doesn't save, and doesn't tell 
						//	anyone!  I'm at least throwing an error (and fixing the select list).
						ThrowNxException("Could not access the interface for saving HotSpot data.");
					}*/
				}

				// (c.haag 2013-08-29) - PLID 58379 - If there are image detail stamps, we need to make sure each one has
				// a matching ink picture text. To do so, start by gathering a list of all the stamp ID's used by the ink picture
				// text list; duplicates are allowed and needed for proper counting. We don't care about the coordinates; if one
				// fails, then the whole detail needs to be inspected.
				CArray<long> unusedTextStampIDs;
				if (!Is3DImage() && m_arypImageStamps.GetSize() > 0)
				{
					CNxInkPictureText inkText;
					inkText.LoadFromVariant(ais.m_varTextData);
					int nStrings = inkText.GetStringCount();
					for (int i=0; i < nStrings; i++)
					{
						unusedTextStampIDs.Add(inkText.GetStampIDByIndex(i));
					}
				}

				// Moved this so EMR image stamps will be marked deleted first in the save batch
				// (z.manning 2010-02-24 11:13) - PLID 37225 - Handle any deleted stamps
				if (m_arynDeletedImageStampIDs.GetSize() > 0)
				{
					CString strDetailStampIDs = ArrayAsString(m_arynDeletedImageStampIDs, false);
					AddStatementToSqlBatch(strSaveString,
						"UPDATE EmrDetailImageStampsT SET Deleted = 1 WHERE ID IN (%s) "
						, strDetailStampIDs);
				}

				// (z.manning 2010-02-18 13:48) - PLID 37404 - Save EmrDetailImageStampsT records
				for(int nDetailImageStampIndex = 0; nDetailImageStampIndex < m_arypImageStamps.GetSize(); nDetailImageStampIndex++)
				{
					EmrDetailImageStamp *pDetailImageStamp = m_arypImageStamps.GetAt(nDetailImageStampIndex);
					// (c.haag 2013-09-25) - PLID 58228 - Don't save invalid smart stamps. Assume they're all valid initially.
					BOOL bValidStamp = TRUE;

					if (!Is3DImage())
					{
						// (c.haag 2013-08-29) - PLID 58379 - Validation between EMR detail image stamps and ink picture text
						// objects happens here. Calculate which element in unusedTextStampIDs is the same as this image detail
						// stamp's StampID.
						int iTextStampIndex = -1;
						for (int i = 0; i < unusedTextStampIDs.GetSize() && -1 == iTextStampIndex; i++)
						{
							if (pDetailImageStamp->nStampID == unusedTextStampIDs[i])
							{
								iTextStampIndex = i;
							}
						}
						if (iTextStampIndex >= 0)
						{
							// If we get here, the image detail stamp has a matching ink picture text stamp ID. Remove it from
							// unusedTextStampIDs so it's not counted twice.
							unusedTextStampIDs.RemoveAt(iTextStampIndex);
						}
						else
						{
							// If we get here, the image detail stamp does not have a matching ink picture text. This is bad data
							// and we can't allow it to be saved.
							//ThrowNxException("Image detail stamp %li has no corresponding ink picture text!", pDetailImageStamp->nID);
							// (c.haag 2013-09-25) - PLID 58228 - Don't throw an exception any longer; but don't save it either
							bValidStamp = FALSE;

							// (z.manning 2016-06-10 13:30) - PLID-66502 - Let's at least log this though as ignoring a stamp
							// that was used somewhere (such as on a smart stamp table) would cause the save to fail.
							Log("Bad data detected-- Detail image stamp with not corresponding text stamp:\r\n"
								"Address: %li, ID: %li, LoadedFromID: %li, StampID: %li, OrderIndex: %li, Rule: %d, X: %li, Y: %li, UsedInTableData: %d"
								, (long)pDetailImageStamp
								, pDetailImageStamp->nID
								, pDetailImageStamp->nLoadedFromID
								, pDetailImageStamp->nStampID
								, pDetailImageStamp->nOrderIndex
								, pDetailImageStamp->eRule
								, pDetailImageStamp->x
								, pDetailImageStamp->y
								, pDetailImageStamp->bUsedInTableData
							);
						}
					}

					// (c.haag 2013-09-25) - PLID 58228 - Only save valid stamps
					if (bValidStamp)
					{
						if(pDetailImageStamp->nID == -1) {
							// (j.jones 2013-07-25 16:52) - PLID 57583 - fail if the positioning data is invalid
							if(pDetailImageStamp->HasInvalidPositioning()) {
								ASSERT(FALSE);	//for debugging
								ThrowNxException("A new image stamp for EMRImageStampsT.ID %li has invalid positioning data!", pDetailImageStamp->nStampID);
							}

							// (z.manning 2010-02-18 14:19) - PLID 37404 - This is a new detail image stamp, so insert it
							// (z.manning 2011-09-08 11:42) - PLID 45335 - Added 3D fields
							AddStatementToSqlBatch(strSaveString,
								"INSERT INTO EmrDetailImageStampsT (EmrDetailID, EmrImageStampID, OrderIndex, SmartStampTableSpawnRule, XPos, YPos, XPos3D, YPos3D, ZPos3D, XNormal, YNormal, ZNormal, HotSpot3D) \r\n"
								"VALUES (%s, %li, %li, %li, %s, %s, %s, %s, %s, %s, %s, %s, %s) "
								, strID, pDetailImageStamp->nStampID, pDetailImageStamp->nOrderIndex, pDetailImageStamp->eRule, AsStringForSql(pDetailImageStamp->GetXVar()), AsStringForSql(pDetailImageStamp->GetYVar())
								, AsStringForSql(pDetailImageStamp->var3DPosX), AsStringForSql(pDetailImageStamp->var3DPosY), AsStringForSql(pDetailImageStamp->var3DPosZ)
								, AsStringForSql(pDetailImageStamp->varNormalX), AsStringForSql(pDetailImageStamp->varNormalY), AsStringForSql(pDetailImageStamp->varNormalZ)
								, AsStringForSql(pDetailImageStamp->Get3DHotSpotIDVar())
								);
							if(pmapSavedObjects != NULL) {
								// (z.manning 2010-02-18 14:55) - PLID 37404 - We are keeping track of saved object so make sure
								// we include this detail image stamp here.
								AddStatementToSqlBatch(strSaveString, "SET @nNewObjectID = CONVERT(int, SCOPE_IDENTITY())");
								AddNewEMRObjectToSqlBatch(strSaveString, esotDetailImageStamp, (long)pDetailImageStamp, *pmapSavedObjects);
							}
						}
						else {
							// (z.manning 2010-02-18 14:19) - PLID 37404 - This is an existing detail image stamp, so let's
							// compare it against the same stamp in m_pLastSavedDetail and see if it changed.
							BOOL bNeedToUpdate = TRUE;
							if(m_oLastSavedDetail) {
								// (a.walling 2012-10-31 17:17) - PLID 53551 - We don't have FindByID in the vector<EmrDetailImageStamp>, just use find_if
								std::vector<EmrDetailImageStamp>::iterator it = boost::find_if(
									m_oLastSavedDetail->m_imageStamps, 
									bind(&EmrDetailImageStamp::nID, _1) == pDetailImageStamp->nID
								);
								if (it != m_oLastSavedDetail->m_imageStamps.end()) {
									if(*pDetailImageStamp == *it) {
										bNeedToUpdate = FALSE;
									}
								}
							}

							if(bNeedToUpdate) {

								// (j.jones 2013-07-25 16:52) - PLID 57583 - fail if the positioning data is invalid
								if(pDetailImageStamp->HasInvalidPositioning()) {
									ASSERT(FALSE);	//for debugging
									ThrowNxException("Existing EmrDetailImageStampsT.ID %li for EMRImageStampsT.ID %li has invalid positioning data!", pDetailImageStamp->nID, pDetailImageStamp->nStampID);
								}

								// (z.manning 2010-02-18 14:34) - PLID 37404 - This stamp changed to upate it in data.
								AddStatementToSqlBatch(strSaveString,
									"UPDATE EmrDetailImageStampsT SET EmrImageStampID = %li, OrderIndex = %li, SmartStampTableSpawnRule = %li, XPos = %s, YPos = %s \r\n"
									"	, XPos3D = %s, YPos3D = %s, ZPos3D = %s, XNormal = %s, YNormal = %s, ZNormal = %s, HotSpot3D = %s \r\n"
									"WHERE ID = %li "
									, pDetailImageStamp->nStampID, pDetailImageStamp->nOrderIndex, pDetailImageStamp->eRule, AsStringForSql(pDetailImageStamp->GetXVar()), AsStringForSql(pDetailImageStamp->GetYVar())
									, AsStringForSql(pDetailImageStamp->var3DPosX), AsStringForSql(pDetailImageStamp->var3DPosY), AsStringForSql(pDetailImageStamp->var3DPosZ)
									, AsStringForSql(pDetailImageStamp->varNormalX), AsStringForSql(pDetailImageStamp->varNormalY), AsStringForSql(pDetailImageStamp->varNormalZ)
									, AsStringForSql(pDetailImageStamp->Get3DHotSpotIDVar())
									, pDetailImageStamp->nID);
							}
						}
					} // if (bValidStamp)
				}
			}
			break;
		case eitSlider:
			if(m_varState.vt == VT_NULL) {
				AddStatementToSqlBatch(strSaveString, "UPDATE EMRDetailsT SET SliderValue = NULL WHERE ID = %s",
					strID);
			}
			else {
				AddStatementToSqlBatch(strSaveString, "UPDATE EMRDetailsT SET SliderValue = %g WHERE ID = %s",
					VarDouble(m_varState), strID);
			}
			break;
		case eitNarrative:
			AddStatementToSqlBatch(strSaveString, "UPDATE EMRDetailsT SET Text = '%s' WHERE ID = %s",
				_Q(VarString(m_varState, "")), strID);
			break;
		case eitTable: 
			{
				//Make sure content is loaded (this will do nothing if it's already loaded).
				LoadContent();
				SyncContentAndState();

				// (a.wetta 2005-11-04 10:53) - PLID 18168 - Update column width information
				if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
					strSaveString += ((CEmrItemAdvTableDlg*)m_pEmrItemAdvDlg)->UpdateColumnInfoInData(GetSaveTableColumnWidths());
				}
				else if (m_nEMRDetailID == -1 && GetSaveTableColumnWidths() && m_nEMRTemplateDetailID != -1) {
					// (a.wetta 2007-05-29 16:55) - PLID 26170 - If the table is on a new EMN created from a template and the table was
					// never viewed, then the table was not given a chance to load its saved table column widths from the template and the table's window
					// doesn't exists to pull those column widths from, so do it now by copying the template details widths
					// (c.haag 2008-10-22 13:54) - PLID 31709 - Added EmrDataID_X for flipped tables
					AddStatementToSqlBatch(strSaveString, "SET @nEMRDetailID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotDetail, (long)this);
					// (r.gonet 02/14/2013) - PLID 40017 - Need to also save the popup column width.
					AddStatementToSqlBatch(strSaveString, "INSERT INTO EMRTableColumnWidthsT (EMRDetailID, EMRDataID_Y, ColumnWidth, EmrDataID_X, PopupColumnWidth) SELECT @nEMRDetailID, EMRDataID_Y, ColumnWidth, EmrDataID_X, PopupColumnWidth "
									"FROM EMRTemplateTableColumnWidthsT WHERE EMRDetailID = %li ", m_nEMRTemplateDetailID);
					AddStatementToSqlBatch(strSaveString, "IF EXISTS (SELECT ID FROM EMRTemplateTableColumnWidthsT WHERE EMRDetailID = %li) "
									"UPDATE EMRDetailsT SET SaveTableColumnWidths = 1 WHERE ID = @nEMRDetailID", m_nEMRTemplateDetailID);
				}

				BOOL bTableChanged = TRUE;
				BOOL bDataChanged = TRUE;
				const int nTableRows = m_arTableRows.GetSize();
				const int nTableColumns = m_arTableColumns.GetSize();

				//TES 7/31/2006 - If we're new, then we've "changed", so don't bother going through all this.
				// (a.walling 2012-10-31 17:17) - PLID 53552 - Compare with the simpler LastSavedDetail structures
				if (m_nEMRDetailID != -1 
					&& m_oLastSavedDetail 
					&& m_oLastSavedDetail->m_tableElements.size() == m_arTableElements.GetSize()
					&& m_oLastSavedDetail->m_tableRows.size() == m_arTableRows.GetSize()
					&& m_oLastSavedDetail->m_tableColumns.size() == m_arTableColumns.GetSize()
				)
				{
					//TES 6/12/2006 - Now that we have a way of checking the original state of our list, don't do anything
					//if it hasn't changed.
					bTableChanged = FALSE;
					bDataChanged = FALSE;

					// (a.walling 2012-10-31 17:17) - PLID 53552 - First check rows, then columns, then individual elements

					for(int nRow = 0; nRow < m_arTableRows.GetSize() && !bTableChanged; nRow++) {
						TableRow *ptrNew = GetRowPtr(nRow);
						const Emr::SimpleTableRow& oldRow = m_oLastSavedDetail->m_tableRows[nRow];
						if (
							(oldRow.nID != ptrNew->m_ID.nDataID) 
							||
							(oldRow.nEmrDetailImageStampID != ptrNew->m_ID.GetDetailImageStampID()) 
						)
						{
							bTableChanged = TRUE;
						}
					}
					for(int nCol = 0; nCol < m_arTableColumns.GetSize() && !bTableChanged; nCol++) {
						TableColumn tcNew = GetColumn(nCol);
						const Emr::SimpleTableColumn& oldCol = m_oLastSavedDetail->m_tableColumns[nCol];
						if (oldCol.nID != tcNew.nID) {
							bTableChanged = TRUE;
						}
					}
					for(int e = 0; e < m_arTableElements.GetSize() && !bTableChanged && !bDataChanged; e++) {
						TableElement elem = m_arTableElements[e];
						const Emr::SimpleTableElement& oldElem = m_oLastSavedDetail->m_tableElements[e];
						if (
							(elem.m_pRow->m_ID.nDataID != oldElem.row.nID)
							||
							(elem.m_pRow->m_ID.GetDetailImageStampID() != oldElem.row.nEmrDetailImageStampID)
							||
							(elem.m_pColumn->nID != oldElem.col.nID)
							||
							(elem.GetValueAsVariant() != oldElem.varValue)
						)
						{
							bDataChanged = TRUE;
						}
					}
				}
				if(bTableChanged) bDataChanged = TRUE; //If the rows or columns changed, then the data must have changed.
				if(bDataChanged) {
					//TES 10/27/2004: They may have been reordered.  Delete the whole thing, and start over.
					AddStatementToSqlBatch(strSaveString, "DELETE FROM EmrSelectT WHERE EmrDetailID = %s", strID);
					AddStatementToSqlBatch(strSaveString, "DELETE FROM EMRDetailTableDataT WHERE EMRDetailID = %s", strID);
				}

				if(bDataChanged) {
						
					// (a.walling 2012-10-31 17:17) - PLID 53552 - Only needs populated if changed now
					// (c.haag 2007-08-20 09:45) - PLID 27118 - This object maps table positions (row,column) to table elements
					CMap<__int64, __int64, int, int> mapThis;

					PopulateTableElementMap(mapThis);

					// (c.haag 2007-08-20 09:43) - PLID 27118 - The legacy code assumed that m_arTableElements was fully
					// populated with all table elements, blank and otherwise. This is no longer the case. We will need to
					// go iterate for all rows for all columns, rather than through m_arTableElements.

					// (a.walling 2014-01-30 00:00) - PLID 60545 - Minimize number of lookups to #NewObjectsT for detail IDs when saving table state 
					bool bEMRDetailIDVariableUpToDate = false;

					for (int nRow = 0; nRow < nTableRows; nRow++) {
						TableRow *ptr = GetRowPtr(nRow);
						// (z.manning 2008-06-26 13:53) - PLID 30155 - Do NOT write calculated fields to data
						// (z.manning 2012-03-27 12:43) - PLID 33710 - Now we do save calculated cell values to data.
						//if(!ptr->IsCalculated())
						{
							for (int nColumn = 0; nColumn < nTableColumns; nColumn++) {
								TableColumn tc = GetColumn(nColumn);
								// (z.manning 2008-06-26 13:53) - PLID 30155 - Do NOT write calculated fields to data
								// (z.manning 2012-03-27 12:43) - PLID 33710 - Now we do save calculated cell values to data.
								//if(!tc.IsCalculated())
								{
									__int64 nKey = GetTableElementKey(ptr, tc.nID);
									int nTableElementIndex;

									if (mapThis.Lookup(nKey, nTableElementIndex)) {
										TableElement element = m_arTableElements[nTableElementIndex];
										// (a.walling 2009-03-09 10:20) - PLID 33409 - Do not write empty values to data
										if (!element.IsValueEmpty()) {
											// If we get here, we've found the element in the table
											// and it is not empty
											// (z.manning 2010-02-19 09:59) - PLID 37404 - We need to handle possible
											// null values for the row data ID.
											CString strDataID_X = "NULL";
											if(ptr->m_ID.nDataID != -1) {
												strDataID_X = AsString(ptr->m_ID.nDataID);
											}
											// (c.haag 2012-10-26) - PLID 53440 - We no longer directly expose the TableRowID
											// variables.
											CString strDetailImageStampID = "NULL";
											if(ptr->m_ID.GetDetailImageStampID() != -1) {												
												strDetailImageStampID = AsString(ptr->m_ID.GetDetailImageStampID());
											}
											else if(ptr->m_ID.GetDetailImageStampObject() != NULL) {
												CString str;
												str.Format("SET @nEmrDetailImageStampID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotDetailImageStamp, (long)ptr->m_ID.GetDetailImageStampObject());
												AddStatementToSqlBatch(strPostSaveSql, str);
												strDetailImageStampID = "@nEmrDetailImageStampID";
											}

											CString strDetailID;
											if(m_nEMRDetailID != -1) {
												strDetailID = AsString(m_nEMRDetailID);
											}
											else {
												if (!bEMRDetailIDVariableUpToDate) {
													AddStatementToSqlBatch(strPostSaveSql, "SET @nEMRDetailID = (SELECT COALESCE(MAX(ID), 0) FROM #NewObjectsT WHERE Type = %li AND ObjectPtr = %li)", esotDetail, (long)this);
													bEMRDetailIDVariableUpToDate = true;
												}
												strDetailID = "@nEMRDetailID";
											}

											// (z.manning 2010-03-02 12:53) - PLID 37404 - We now save this as part of the post save
											// part of the query because it may depend on both detail pointers and detail image stamp
											// pointers, which are themselves dependent on detail pointers. Nothing is dependent on
											// EMRDetailTableDataT so this should have no actualy affect on saving itself.
											AddStatementToSqlBatch(strPostSaveSql, "INSERT INTO EMRDetailTableDataT (EMRDetailID, EMRDataID_X, EMRDataID_Y, Data, EmrDetailImageStampID) "
												"VALUES (%s, %s, %li, '%s', %s)", strDetailID, strDataID_X, tc.nID, _Q(element.GetValueAsString()), strDetailImageStampID);
										}
									} else {
										// (a.walling 2009-03-09 10:20) - PLID 33409 - Do not write empty values to data
										// If we get here, this is either an empty string or an unfilled table element
									}
								}
							}
						}
					}
				}
			}
			break;
		default:
			// unknown
			ASSERT(FALSE);
			break;
		}
	} else {
		//we tried to change this on a locked EMN!
		CString str;
		str.Format("Attempted to alter the detail '%s' on a locked EMN.", GetMergeFieldName(FALSE));
		arystrErrors.Add(str);
		strSaveString = Nx::Quantum::Batch(); // Should be empty anyway, but just in case
	}

	return strSaveString;
}

void CEMNDetail::SetMergeNameConflict(BOOL bConflicts)
{
	m_bMergeNameConflicts = bConflicts;
	if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
		m_pEmrItemAdvDlg->SetMergeNameConflict(bConflicts);
	}
}

// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
// (a.walling 2014-01-30 00:00) - PLID 60544 - Quantize
Nx::Quantum::Batch CEMNDetail::GenerateDeleteString(long &nAuditTransactionID, CStringArray &arystrErrors, CDWordArray &arynAffectedDetailIDs)
{
	Nx::Quantum::Batch strDeleteString;
	//TES 9/14/2006 - There is one rare, but valid, case where we do this, which is cleaning up bad data in CEMRTopic::GenerateDeleteString().
	//ASSERT(m_pParentTopic); //We shouldn't be saving until we're attached to a topic.

	// (c.haag 2006-07-05 09:44) - PLID 19977 - We cannot allow the deletion of details that
	// have problem associated with them
	// (j.jones 2008-07-21 08:23) - PLID 30779 - call HasProblems() instead
	// (c.haag 2008-07-24 12:13) - PLID 30826 - New way of checking validity through problems
	// (c.haag 2008-08-05 11:33) - PLID 30799 - Do not fire an exception here any more. We allow users who do
	// not have permissions to explicitly delete problems to indirectly do so through unspawning.
	//if (!CanCurrentUserDeleteEmrProblems() && DoesDetailOrChildrenHaveSavedProblems()) {
	//	ThrowNxException(_T("CEMNDetail::GenerateDeleteString: Could not create the delete string because at least one detail is associated with a problem!"));
	//}
		
	if(m_bIsTemplateDetail) {
		//don't need to delete it if the item doesn't exist,
		//but this does need to be after the previous code so we can clean out the child lists
		if(m_nEMRTemplateDetailID == -1)
			return strDeleteString;

		// (j.jones 2007-01-12 11:09) - PLID 24027 - migrated to DeleteEMRTemplateDetail
		DeleteEMRTemplateDetail(m_nEMRTemplateDetailID, strDeleteString);

		/*
		AddStatementToSqlBatch(strDeleteString, "DELETE FROM EMRTemplateTableDefaultsT WHERE EmrTemplateDetailID = %li", m_nEMRTemplateDetailID);
		AddStatementToSqlBatch(strDeleteString, "DELETE FROM EMRTemplateTableColumnWidthsT WHERE EmrDetailID = %li", m_nEMRTemplateDetailID);
		AddStatementToSqlBatch(strDeleteString, "DELETE FROM EmrTemplateSelectT WHERE EmrTemplateDetailID = %li", m_nEMRTemplateDetailID);
		AddStatementToSqlBatch(strDeleteString, "DELETE FROM EmrTemplateDetailsT WHERE ID = %li", m_nEMRTemplateDetailID);
		*/

		//only audit if the EMN is not new
		if(m_pParentTopic) {
			if(m_pParentTopic->GetParentEMN()) {
				if(m_pParentTopic->GetParentEMN()->GetID() != -1) {
					//auditing (using transactions)
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(-1, "", nAuditTransactionID, aeiEMNTemplateDetailRemoved, m_pParentTopic->GetParentEMN()->GetID(), m_strLabelText, "<Deleted>", aepHigh, aetDeleted);
				}
			}
		}
	}
	else {
		//don't need to delete it if the item doesn't exist,
		//but this does need to be after the previous code so we can clean out the child lists
		if(m_nEMRDetailID == -1)
			return strDeleteString;

		// (j.jones 2006-08-24 10:53) - PLID 22183 - find out if this detail is on a locked EMN
		BOOL bIsLockedAndSaved = GetIsOnLockedAndSavedEMN();

		if(bIsLockedAndSaved) {
			//this is locked! add a warning and return
			CString str;
			str.Format("Attempted to delete detail '%s' from a locked EMN.", GetMergeFieldName(FALSE));
			arystrErrors.Add(str);
			return strDeleteString;
		}
		
		// (j.jones 2006-04-26 09:37) - PLID 20064 - we now simply mark the detail as being deleted
		//AddStatementToSqlBatch(strDeleteString, "DELETE FROM EMRTableColumnWidthsT WHERE EmrDetailID = %li", m_nEMRDetailID);
		//AddStatementToSqlBatch(strDeleteString, "DELETE FROM EmrDetailTableDataT WHERE EmrDetailID = %li", m_nEMRDetailID);
		//AddStatementToSqlBatch(strDeleteString, "DELETE FROM EmrSelectT WHERE DetailElementID IN (SELECT ID FROM EmrDetailElementsT WHERE EmrDetailID = %li)", m_nEMRDetailID);
		//AddStatementToSqlBatch(strDeleteString, "DELETE FROM EmrDetailElementsT WHERE EmrDetailID = %li", m_nEMRDetailID);
		//AddStatementToSqlBatch(strDeleteString, "DELETE FROM EmrDetailsT WHERE ID = %li", m_nEMRDetailID);
		AddStatementToSqlBatch(strDeleteString, "UPDATE EmrDetailsT SET Deleted = 1, DeleteDate = GetDate(), DeletedBy = '%s' WHERE ID = %li", _Q(GetCurrentUserName()), m_nEMRDetailID);
		// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
		// We existed before, and now we've been marked deleted, so store our ID in the array
		arynAffectedDetailIDs.Add(m_nEMRDetailID);
		// (c.haag 2006-10-19 13:03) - PLID 21454 - We must delete the problem as well because it has no meaning outside of a detail
		// (j.jones 2008-07-16 09:11) - PLID 30739 - supported EMRRegardingType and EMRRegardingID
		// (c.haag 2009-05-11 17:38) - PLID 28494 - Problem-regarding information now goes into its own table
		// (j.jones 2009-06-02 12:14) - PLID 34301 - only delete links right now, the parent should know to delete problems
		AddStatementToSqlBatch(strDeleteString, "DELETE FROM EMRProblemLinkT WHERE ((EMRRegardingType = %li AND EMRRegardingID = %li) "
			"OR (EMRRegardingType = %li AND EMRRegardingID = %li))", eprtEmrItem, m_nEMRDetailID, eprtEmrDataItem, m_nEMRDetailID);
		// (c.haag 2008-06-24 14:37) - PLID 17244 - Delete todo alarms
		// (c.haag 2008-07-10 15:33) - PLID 30674 - Include the EMR todo list table
		// (c.haag 2008-07-15 11:51) - PLID 30694 - Now the EMN is responsible for deleting detail todo alarms when the EMN is deleted. Furthermore, the only way a todo alarm can be
		// created for a specific detail is through spawning, and the act of unspawning when the detail was deleted would have already removed these alarms anyway. So, the following
		// commented code is now moot.
		//AddStatementToSqlBatch(strDeleteString, "DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT TaskID FROM TodoList WHERE RegardingType = %d AND RegardingID = %d)", (long)ttEMNDetail, m_nEMRDetailID);
		//AddStatementToSqlBatch(strDeleteString, "DELETE FROM EMRTodosT WHERE TaskID IN (SELECT TaskID FROM TodoList WHERE RegardingType = %d AND RegardingID = %d)", (long)ttEMNDetail, m_nEMRDetailID);
		//AddStatementToSqlBatch(strDeleteString, "DELETE FROM TodoList WHERE RegardingType = %d AND RegardingID = %d", (long)ttEMNDetail, m_nEMRDetailID);

		//only audit if the EMN is not new
		if(m_pParentTopic->GetParentEMN()->GetID() != -1) {
			//auditing (using transactions)
			if(nAuditTransactionID == -1)
				nAuditTransactionID = BeginAuditTransaction();
			
			// (j.jones 2009-06-01 15:03) - PLID 34431 - if this is a signature, use a unique audit for it
			if(IsSignatureDetail()) {	
				AuditEvent(m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), GetExistingPatientName(m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID()),
					nAuditTransactionID, aeiEMNSignatureDetailRemoved, m_pParentTopic->GetParentEMN()->GetID(), m_strLabelText, "<Deleted>", aepHigh, aetDeleted);
			}
			else {				
				//normal detail
				AuditEvent(m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), GetExistingPatientName(m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID()),
					nAuditTransactionID, aeiEMNDetailRemoved, m_pParentTopic->GetParentEMN()->GetID(), m_strLabelText, "<Deleted>", aepHigh, aetDeleted);
			}
		}
	}
	return strDeleteString;
}

// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
void CEMNDetail::LogEmrObjectData(int nIndent, BOOL bForceDeletedFlagTrue, BOOL bInTopicPendingDeletionDetailsAry)
{
	BOOL bDeleted = bForceDeletedFlagTrue;

	// Log this object
	::LogEmrObjectData(nIndent, m_nEMRDetailID, this, esotDetail, (m_nEMRDetailID == -1), m_bModified, bDeleted, m_strLabelText,
		"m_nEMRInfoID = %d  m_EMRInfoSubType = %d  m_nEMRSourceTemplateID = %d  m_nEMRTemplateDetailID = %d  m_bVisible = %d  bInTopicPendingDeletionDetailsAry = %d  sourceActionID = %d  sourceDetailID = %d  sourceDataGroupID = %d  sourceDetailImageStampID = %d"
		, m_nEMRInfoID
		, m_EMRInfoSubType
		, m_nEMRSourceTemplateID
		, m_nEMRTemplateDetailID
		, m_bVisible
		, bInTopicPendingDeletionDetailsAry
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

	// Log detail image stamps
	for (auto s : m_arypImageStamps)
	{
		s->LogEmrObjectData(nIndent + 1);
	}
	for (auto ID : m_arynDeletedImageStampIDs)
	{
		// We don't preserve deleted image stamps; just their ID's
		::LogEmrObjectData(nIndent + 1, ID, nullptr, esotDetailImageStamp, FALSE, FALSE, TRUE, "", "");
	}
}

void CEMNDetail::GetSelectedValues(OUT CDWordArray &arSelectedDataIDs) const
{
	//Only meaningful for list items.
	if(m_EMRInfoType == eitSingleList || m_EMRInfoType == eitMultiList) {
		// (c.haag 2007-07-16 08:32) - PLID 26651 - Use GetState() instead of m_varState
		// because GetState() throws a specific exception if we try to access an empty state,
		// which we should never do here
		// (a.walling 2008-03-24 10:25) - PLID 28811 - Return if state does not match item type
		if (!DataTypeMatchesState(m_EMRInfoType, GetState().vt))
			return;
		FillArrayFromSemiColonDelimitedIDList(arSelectedDataIDs, VarString(GetState(),""));
	}
}

// (z.manning 2011-11-16 16:10) - PLID 38130 - Function to call all of the below functions since we call them all in multiple places
void CEMNDetail::CollectAndProcessActionsPostLoad(BOOL bIsInitialLoad, CEMNLoader* pEMNLoader /* = NULL */, ADODB::_Connection *lpCon /* = NULL */)
{
	// (z.manning 2011-11-16 16:52) - PLID 38130 - If we're on a patient EMN and this item has the option set
	// to not spawn anything from remembered values and this item's state was remembered then skip any attempted
	// action processing. We are assuming that this function is only being called during the creation of a new detail
	// (whether it be from loading a new EMN, spawning a new item, etc.).  Normal spawning after the item has already
	// been loaded should still work properly in RequestStateChange regardless of the spawn/remember option.
	if(!m_bIsTemplateDetail && !ShouldSpawnFromRememberedValues() && WasStateRemembered()) {
		return;
	}

	if(!(m_EMRInfoType == eitSingleList || m_EMRInfoType == eitMultiList || m_EMRInfoType == eitTable || m_EMRInfoType == eitImage)) {
		// (z.manning 2011-11-16 16:53) - PLID 38130 - This item can't spawn anything anyway.
		return;
	}

	// (a.walling 2010-09-13 13:50) - PLID 40500 - Don't ensure any smart stamp links here. They will get ensured later on when the other side is loaded.
	LoadContent(FALSE, pEMNLoader, lpCon, FALSE);

	if(m_EMRInfoType == eitSingleList || m_EMRInfoType == eitMultiList) {
		CollectAndProcessEmrDataActions(bIsInitialLoad, pEMNLoader);
	}
	if(m_EMRInfoType == eitTable) {
		CollectAndProcessTableDropdownActions(bIsInitialLoad, pEMNLoader);
	}
	if(m_EMRInfoType == eitImage) {
		CollectAndProcessImageHotSpotActions(bIsInitialLoad, pEMNLoader);
		CollectAndProcessSmartStampImageActions(bIsInitialLoad, pEMNLoader);
	}
}

// (z.manning 2011-11-16 15:51) - PLID 38130 - Created this function to put some common code in
void CEMNDetail::CollectAndProcessEmrDataActions(BOOL bIsInitialLoad, CEMNLoader* pEMNLoader)
{
	CDWordArray arydwDataIds;
	GetSelectedValues(arydwDataIds);

	// (j.jones 2013-01-09 16:44) - PLID 54541 - we now support processing the actions for
	// all the selected data IDs at once, but it takes in an array of longs
	CArray<long, long> aryDataIDs;
	for(int i=0; i<arydwDataIds.GetSize(); i++) {
		aryDataIDs.Add((long)arydwDataIds.GetAt(i));
	}
	if(aryDataIDs.GetSize() > 0) {
		m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEMRDataActions(aryDataIDs, this, bIsInitialLoad, pEMNLoader);
	}
}

// (z.manning 2011-11-16 15:51) - PLID 38130 - Created this function to put some common code in
void CEMNDetail::CollectAndProcessImageHotSpotActions(BOOL bIsInitialLoad, CEMNLoader* pEMNLoader)
{
	CEMRHotSpotArray arynSelectedHotSpots;
	GetImageSelectedHotSpots(arynSelectedHotSpots);
	for(int nHotSpotIDIndex = 0; nHotSpotIDIndex < arynSelectedHotSpots.GetSize(); nHotSpotIDIndex++) {
		//TES 2/15/2010 - PLID 37375 - This now takes a CEMRHotSpot, not an ID.
		m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEmrImageHotSpotActions(arynSelectedHotSpots.GetAt(nHotSpotIDIndex), this, bIsInitialLoad, pEMNLoader);
	}
}

// (z.manning 2009-02-16 12:11) - PLID 33072
void CEMNDetail::CollectAndProcessTableDropdownActions(BOOL bIsInitialLoad, CEMNLoader* pEMNLoader)
{
	CArray<long,long> arynDropdownIDs;
	if(m_EMRInfoType == eitTable)
	{
		const int nElementCount = GetTableElementCount();
		for(int nElementIndex = 0; nElementIndex < nElementCount; nElementIndex++)
		{
			TableElement te;
			GetTableElementByIndex(nElementIndex, te);
			if(te.m_pColumn->nType == LIST_TYPE_DROPDOWN) {
				m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEmrTableDropdownItemActions(te.m_anDropdownIDs, this, te, bIsInitialLoad, pEMNLoader);
			}
		}
	}
}

// (z.manning 2010-03-02 14:32) - PLID 37571
void CEMNDetail::CollectAndProcessSmartStampImageActions(BOOL bIsInitialLoad, CEMNLoader* pEMNLoader)
{
	if(m_EMRInfoType == eitImage)
	{
		for(int nDetailStampIndex = 0; nDetailStampIndex < m_arypImageStamps.GetSize(); nDetailStampIndex++)
		{
			EmrDetailImageStamp *pDetailStamp = m_arypImageStamps.GetAt(nDetailStampIndex);
			// (z.manning 2011-11-16 09:12) - PLID 46496 - We used to check if the smart stamp table was non-null here
			// but we should not have been since we allow stamp spawning on non-smart stamp images.
			if(pDetailStamp != NULL) {
				// (j.jones 2010-04-07 11:09) - PLID 37971 - we now support spawning
				// for non-smartstamp images
				TableRow *ptr = NULL;
				if(m_pSmartStampTableDetail) {
					ptr = m_pSmartStampTableDetail->GetRowByDetailStamp(pDetailStamp);
				}
				TableRow tr;
				if(ptr == NULL) {
					//TES 3/17/2010 - PLID 37530 - Need to specify the stamp index
					tr.m_ID = TableRowID(pDetailStamp, GetStampIndexInDetailByType(pDetailStamp));
					ptr = &tr;
				}
				m_pParentTopic->GetParentEMN()->GetParentEMR()->ProcessEmrSmartStampImageActions(ptr, this, bIsInitialLoad, pEMNLoader);
			}
		}
	}
}

BOOL CEMNDetail::PropagateNewID(long nID, EmrSaveObjectType esotSaveType, long nObjectPtr, long &nAuditTransactionID)
{
	if(esotSaveType == esotDetail) {

		// (j.jones 2007-01-12 14:35) - PLID 24027 - see if our SourceDetail pointer is this detail,
		// if so, update our SourceDetailID accordingly
		
		if(m_sai.nSourceDetailID == -1 && m_sai.pSourceDetail != NULL
			&& m_sai.pSourceDetail == (CEMNDetail*)nObjectPtr) {

			//this is our source detail! update the ID
			// (c.haag 2007-06-20 11:06) - PLID 26388 - Fail if an ID already exists
			if (m_sai.nSourceDetailID != -1) {
				ASSERT(FALSE); ThrowNxException("Called CEMNDetail::PropagateNewID on a detail with an existing source ID! (Current = %d nID = %d)", m_sai.nSourceDetailID, nID);
			}
			m_sai.nSourceDetailID = nID;
		}

		//TES 7/11/2012 - I started on this functionality before deciding it wasn't necessary for the present scope, I entered PLID 51487
		// to finish this feature at some future time
		/*//TES 7/10/2012 - PLID 50855 - If this is a narrative, we need to see if we have this detail as a merge field, and if so, update
		// its nxdetailid attribute.
		if(m_EMRInfoType == eitNarrative && !IsRtfNarrative()) {
			CString strHtml = VarString(m_varState, "");
			CEMNDetail *pSavedDetail = (CEMNDetail*)nObjectPtr;
			CString strDetailName = pSavedDetail->GetLabelText();
			bool bTextChanged = false;
			//TES 7/10/2012 - PLID 50855 - Load this as an XML document.
			MSXML2::IXMLDOMDocument2Ptr pDoc(__uuidof(MSXML2::DOMDocument60));
			//TES 7/10/2012 - PLID 50855 - We want to use a function exposed in NexTech.COM that will clean up the HTML and make it as XML-friendly as
			// it can.
			NexTech_COM::IHtmlUtilsPtr pUtils;
			pUtils.CreateInstance("NexTech_COM.HtmlUtils");
			if(pUtils == NULL) {
				AfxThrowNxException("Invalid NexTech_Com pointer in CEMNDetail::GetHtml()");
			}
			BSTR bstrXHtml, bstrError;
			BOOL bSuccess = pUtils->GetAsXml(_bstr_t(""), _bstr_t(""), _bstr_t(strHtml), &bstrXHtml, &bstrError);
			if(!bSuccess) {
				//TES 7/10/2012 - PLID 50855 - It couldn't clean up the code, so pass the error along.
				AfxThrowNxException("Error parsing HTML narrative in HandleLink(): " + CString((LPCTSTR)_bstr_t(bstrError)));
			}
			CString strXml = "<?xml version=\"1.0\" encoding=\"UTF-16\" ?><validationblock>" + CString((LPCTSTR)_bstr_t(bstrXHtml)) + "</validationblock>";
			strXml.Replace("&nbsp;"," ");
			BSTR bstrXml = strXml.AllocSysString();
			if (VARIANT_FALSE != pDoc->loadXML( bstrXml ))
			{
				//TES 7/10/2012 - PLID 50855 - Now go through all the links
				MSXML2::IXMLDOMNodeListPtr pList = pDoc->selectNodes("//a");
				if(pList->length == 0) {
					pList = pDoc->selectNodes("//A");
				}
				for (long i=0; i < pList->length; i++)
				{
					MSXML2::IXMLDOMNodePtr pNode = pList->item[i];
					if(pNode) {
						CString strHref = (LPCTSTR)NxXMLUtils::SafeGetAttribute(pNode, "href");
						CString strFieldName = (LPCTSTR)NxXMLUtils::SafeGetAttribute(pNode, "nxmfname");
						if(strHref.GetLength() >= 10 && strHref.Left(10) == "nexguid://" && strFieldName == strDetailName) {
							//TES 7/10/2012 - PLID 50855 - OK, this is detail matches.  If its nxdetailid is -1, update it to the new id.
							CString strXml = (LPCTSTR)pNode->xml;
							CString strXmlNew = strXml;
							if(strXmlNew.Replace("nxdetailid=\"-1\"", "nxdetailid=\"" + AsString(nID) + "\"")) {
								if(!strHtml.Replace(strXml, strXmlNew)) {
									strXml.Replace("<a ", "<A ");
									strXml.Replace("</a>", "</A>");
									strHtml.Replace(strXml, strXmlNew);
								}
								bTextChanged = true;
							}
						}
					}
				}
			}
			if(bTextChanged) {
				//TES 7/10/2012 - PLID 50855 - Save the updated state
				RequestStateChange(_bstr_t(strHtml));
			}
		}*/
	}

	//test to see if the nObjectPtr and esotSaveType match this object,
	//if so, assign the ID and return TRUE, otherwise, return FALSE

	if(esotSaveType == esotDetail && (CEMNDetail*)nObjectPtr == this) {

		//we have a winner!
		if(m_bIsTemplateDetail) {

			//assign the ID
			// (c.haag 2007-06-19 17:38) - PLID 26388 - Fail if an ID already exists
			if (m_nEMRTemplateDetailID != -1) {
				ASSERT(FALSE); ThrowNxException("Called CEMNDetail::PropagateNewID on a detail with an existing template detail ID! (Current = %d nID = %d)", m_nEMRTemplateDetailID, nID);
			}
			m_nEMRTemplateDetailID = nID;

			// (j.jones 2010-03-02 08:59) - PLID 37318 - update the ID in the linked SmartStamp detail
			// (this actually fairly unnecessary since we have already linked their pointers)
			// (z.manning 2011-01-20 13:05) - PLID 42338 - Support multiple images per smart stamp table
			m_arySmartStampImageDetails.UpdateChildEMRTemplateDetailIDs(nID);

			//we've already copied to the last saved detail, so update its ID
			if(m_oLastSavedDetail) {
				// (c.haag 2007-06-19 17:38) - PLID 26388 - Fail if an ID already exists
				if (m_oLastSavedDetail->m_nEMRTemplateDetailID != -1) {
					ASSERT(FALSE); ThrowNxException("Called CEMNDetail::PropagateNewID on a detail with an existing last saved template detail ID! (Current = %d nID = %d)", m_oLastSavedDetail->m_nEMRTemplateDetailID, nID);
				}
				m_oLastSavedDetail->m_nEMRTemplateDetailID = nID;
			}

			//if this detail was created on a new template, we could not audit at the time
			//because we did not have the template ID. Now we do, so audit the creation.
			if(m_bCreatedOnNewEMN) {

				CString strNewDetailName, strNewDetailData;
				GetDetailNameAndDataForAudit(strNewDetailName,strNewDetailData);

				//detail added
				CString strNewValue;
				strNewValue.Format("Detail: %s, Data: %s",strNewDetailName,strNewDetailData);
				if(nAuditTransactionID == -1)
					nAuditTransactionID = BeginAuditTransaction();				
				AuditEvent(-1, "",nAuditTransactionID,aeiEMNTemplateDetailCreatedOnNewTemplate,m_pParentTopic->GetParentEMN()->GetID(),"",strNewValue,aepHigh,aetCreated);
			}
		}
		else {

			//assign the ID
			// (c.haag 2007-06-19 17:38) - PLID 26388 - Fail if an ID already exists
			if (m_nEMRDetailID != -1) {
				ASSERT(FALSE); ThrowNxException("Called CEMNDetail::PropagateNewID on a detail with an existing detail ID! (Current = %d nID = %d)", m_nEMRDetailID, nID);
			}
			m_nEMRDetailID = nID;

			// (j.jones 2010-03-02 08:59) - PLID 37318 - update the ID in the linked SmartStamp detail
			// (this actually fairly unnecessary since we have already linked their pointers)
			// (z.manning 2011-01-20 13:06) - PLID 42338 - Handle multiple images per smart stamp table
			m_arySmartStampImageDetails.UpdateChildEMRDetailIDs(nID);

			// (j.jones 2008-07-29 17:33) - PLID 30880 - if any problems are on this detail, update their regarding IDs
			// (z.manning 2009-05-22 14:51) - PLID 34332 - Handle problem links.
			for(int i = 0; i < m_apEmrProblemLinks.GetSize(); i++) {
				CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(i);
				if(pProblemLink != NULL && (pProblemLink->GetType() == eprtEmrItem || pProblemLink->GetType() == eprtEmrDataItem)) {
					pProblemLink->SetRegardingID(nID);
				}
			}

			//we've already copied to the last saved detail, so update its ID
			if(m_oLastSavedDetail) {
				// (c.haag 2007-06-19 17:38) - PLID 26388 - Fail if an ID already exists
				if (m_oLastSavedDetail->m_nEMRDetailID != -1) {
					ASSERT(FALSE); ThrowNxException("Called CEMNDetail::PropagateNewID on a detail with an existing last saved detail ID! (Current = %d nID = %d)", m_oLastSavedDetail->m_nEMRDetailID, nID);
				}
				m_oLastSavedDetail->m_nEMRDetailID = nID;
			}

			//if this detail was created on a new EMN, we could not audit at the time
			//because we did not have the EMN ID. Now we do, so audit the creation.
			if(m_bCreatedOnNewEMN) {

				CString strNewDetailName, strNewDetailData;
				GetDetailNameAndDataForAudit(strNewDetailName,strNewDetailData);

				if(!GetIsOnLockedAndSavedEMN()) {
					//detail added
					CString strNewValue;
					strNewValue.Format("Detail: %s, Data: %s",strNewDetailName,strNewDetailData);
					if(nAuditTransactionID == -1)
						nAuditTransactionID = BeginAuditTransaction();
					CString strPatientName = GetExistingPatientName(m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID());
					
					// (j.jones 2009-06-01 15:03) - PLID 34431 - if this is a signature, use a unique audit for it
					if(IsSignatureDetail()) {
						// (j.jones 2013-08-07 15:43) - PLID 42958 - added audits for when a user gets another user to sign their EMR
						if(m_bIsSignatureAddedByAnotherUser) {
							CString strUser;
							strUser.Format(", Signed By User: %s", m_strSignatureAddedByAnotherUsername);
							strNewValue += strUser;
							AuditEvent(m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), strPatientName, nAuditTransactionID, aeiEMNSignatureDetailCreatedOnNewEMNByAnotherUser, m_pParentTopic->GetParentEMN()->GetID(), "", strNewValue,aepHigh,aetCreated);
						}
						else {
							AuditEvent(m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), strPatientName, nAuditTransactionID, aeiEMNSignatureDetailCreatedOnNewEMN, m_pParentTopic->GetParentEMN()->GetID(), "", strNewValue, aepHigh, aetCreated);
						}
					}
					else {				
						//normal detail
						AuditEvent(m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), strPatientName,nAuditTransactionID,aeiEMNDetailCreatedOnNewEMN,m_pParentTopic->GetParentEMN()->GetID(),"",strNewValue,aepHigh,aetCreated);
					}
				}
				else {
					ASSERT(FALSE);
					//we should have already caught and warned about this event, and should never get here,
					//but if so, don't bother warning, just don't audit
				}
			}
		}
		return TRUE;
	}
	else if(esotSaveType == esotProblemLink)
	{
		// (z.manning 2009-05-22 15:14) - PLID 34332 - Need to handle problem links
		for(int nProblemLinkIndex = 0; nProblemLinkIndex < m_apEmrProblemLinks.GetSize(); nProblemLinkIndex++)
		{
			CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(nProblemLinkIndex);
			if(pProblemLink == (CEmrProblemLink*)nObjectPtr)
			{
				if(pProblemLink->GetID() > 0) {
					ThrowNxException("Called CEMNDetail::PropagateNewID on an existing problem link for a EMR detail! (Problem link ID: %li)", pProblemLink->GetID());
				}

				pProblemLink->SetID(nID);
				pProblemLink->Audit(aeiEMNProblemLinkCreated, nAuditTransactionID, GetExistingPatientName(m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID()));
				return TRUE;
			}
		}
	}
	// (z.manning 2010-02-18 14:58) - PLID 37404 - Handle ID propagation for detail image stamps
	else if(esotSaveType == esotDetailImageStamp)
	{
		// (z.manning 2011-01-03 17:05) - PLID 41974 - We must also check the source action info for this detail stamp.
		if(m_sai.GetDetailStampID() == -1 && m_sai.GetDetailStampPointer() != NULL
			&& (long)m_sai.GetDetailStampPointer() == nObjectPtr)
		{
			m_sai.SetDetailStampID(nID);
		}

		// (z.manning 2010-03-02 13:08) - PLID 37571 - First update any references to this pointer in our linked table
		UpdateDetailStampIDByPointer(nObjectPtr, nID);
		for(int nDetailStampIndex = 0; nDetailStampIndex < m_arypImageStamps.GetSize(); nDetailStampIndex++) {
			EmrDetailImageStamp *pDetailStamp = m_arypImageStamps.GetAt(nDetailStampIndex);
			if(pDetailStamp != NULL && (long)pDetailStamp == nObjectPtr) {

				// (c.haag 2012-10-26) - PLID 53440 - UpdateSourceDetailStampPointers should have already updated the detail image stamp ID.
				// So, lets only throw the exception if the ID's don't match.
				if(pDetailStamp->nID > 0 && pDetailStamp->nID != nID) {
					ThrowNxException("Called CEMNDetail::PropagateNewID on an existing detail image stamp for a EMR detail! (Detail image stamp ID: %li)", pDetailStamp->nID);
				}

				// (c.haag 2012-10-26) - PLID 53440 - But make sure we always set it as before just in case the previous comment is wrong
				// in some weird permutation I didn't consider
				pDetailStamp->nID = nID;

				// (a.walling 2012-10-31 17:17) - PLID 53552 - LastSavedDetail - LastSavedStamps no longer necessary


				return TRUE;
			}
		}
	}

	return FALSE;
}

void CEMNDetail::SetSaved(BOOL bIsPostLoad /*= FALSE*/)
{
	try {

		// (j.jones 2006-08-24 11:43) - PLID 22183 - if this is a post save, as opposed
		// to a post load, then don't mark as saved if it's on a locked EMN,
		// because it would not have been saved
		// (j.jones 2010-03-02 10:24) - PLID 37318 - if m_bStayUnsavedPostLoad is TRUE,
		// do not set this as saved when bIsPostLoad is true
		if(!m_bStayUnsavedPostLoad && (bIsPostLoad || !GetIsOnLockedAndSavedEMN())) {
			m_bModified = FALSE;
		}
		else if(!bIsPostLoad) {
			//regular save, we can now clear this flag
			m_bStayUnsavedPostLoad = FALSE;

			if(!GetIsOnLockedAndSavedEMN()) {
				m_bModified = FALSE;
			}
		}

	}NxCatchAll("Error in CEMNDetail::SetSaved");
}

// (j.jones 2010-03-02 10:19) - PLID 37318 - added parameter to force unsaved
// after the load completes, because loading completion will call SetSaved()
void CEMNDetail::SetUnsaved(BOOL bStayUnsavedPostLoad /*= FALSE*/)
{
	// (a.walling 2008-12-18 12:37) - PLID 27964 - ASSERT if this is called on a locked EMN
#ifdef _DEBUG
	if (m_pParentTopic && m_pParentTopic->GetParentEMN()) {
		_ASSERTE(m_pParentTopic->GetParentEMN()->GetStatus() != 2);
	}
#endif

	m_bModified = TRUE;
	if(bStayUnsavedPostLoad && !m_bStayUnsavedPostLoad) {
		m_bStayUnsavedPostLoad = TRUE;
	}
}

CString CEMNDetail::GetLabelText() const
{
	return m_strLabelText;
}

void CEMNDetail::SetLabelText(const CString &strLabelText)
{
	// (j.jones 2007-10-22 09:31) - PLID 27830 - altered the logic to be more narrative-friendly,
	// adding code that skips this function if the text didn't change, and adding code that
	// does not call UpdateNarrativesAndLinkedTables if m_strLabelText is empty at the start
	// of the function

	//don't bother processing anything if we aren't actually changing the text
	if(strLabelText != m_strLabelText) {

		const CString strOldLabel = m_strLabelText;

		BOOL bOriginallyEmpty = m_strLabelText.IsEmpty();

		//we have to call update narratives once to say we're removing the item
		if(m_pParentTopic && !bOriginallyEmpty) {
			//TES 1/23/2008 - PLID 24157 - Renamed.
			m_pParentTopic->GetParentEMN()->HandleDetailChange(this, TRUE);
		}

		//now assign the label
		m_strLabelText = strLabelText;

		//and again to re-add the new label

		// (j.jones 2007-10-22 10:07) - PLID 27830 - I made the design decision not to
		// update narratives yet if the label was originally blank, because if that's true
		// then we're likely still in a load, and the function adding the new detail should
		// be responsible for updating narratives. (ie. sentence format may not be set yet, etc.)
		if(m_pParentTopic && !bOriginallyEmpty) {
			//TES 1/23/2008 - PLID 24157 - Renamed.
			m_pParentTopic->GetParentEMN()->HandleDetailChange(this);

			// (c.haag 2008-06-09 14:33) - PLID 25298 - Because label text is the "primary key"
			// that links narrative fields with details, the label would normally just disappear
			// from the narrative. To prevent this, we go through all the narratives in the EMN,
			// search the rich text string for the old label text, and replace it with the new
			// label text. I referenced existing code to figure out how to do a clean traversal
			// through the rich text.
			//
			// Developer note: Behavior for when multiple details have the same label text is
			// undefined. Additionally, when we have a more precise way to bind narrative fields
			// with details, then this code should become obselete.
			//
			if(IsDetailLinkable(this)) {
				if (!strLabelText.IsEmpty()) { // Neither the old or new label must be empty
					CArray<CEMNDetail*,CEMNDetail*> arDetails;
					m_pParentTopic->GetParentEMN()->GenerateTotalEMNDetailArray(&arDetails);
					const int nDetails = arDetails.GetSize();
					int i;
					for (i=0; i < nDetails; i++) {
						CEMNDetail* pDetail = arDetails[i];
						if (eitNarrative == pDetail->m_EMRInfoType) {

							// If we get here, this is a narrative detail. Get the rich text and do the
							// find-and-replace. We begin by fetching the rich text, and then taking the
							// header out of it.
							CString strNxRichText = VarString(pDetail->GetState(), "");
							if(::IsRtfNarrative(strNxRichText)) {
								int nHeaderBegin = strNxRichText.Find(NXRTF_HEADER_BEGIN);
								ASSERT(nHeaderBegin == 0);
								int nHeaderEnd = strNxRichText.Find(NXRTF_HEADER_END);
								ASSERT(nHeaderEnd != -1);
								nHeaderBegin += CString(NXRTF_HEADER_BEGIN).GetLength();
								CString strHeader = strNxRichText.Mid(nHeaderBegin, nHeaderEnd - nHeaderBegin);
								nHeaderEnd += CString(NXRTF_HEADER_END).GetLength();
								strNxRichText.Delete(nHeaderBegin, strHeader.GetLength());
								
								// Now traverse the header for merge fields that match the old label, and replace
								// them with the new label.
								int nMergeField = -1;
								bool bFoundNext = true;
								bool bChanged = false;
								while(bFoundNext) {
									nMergeField = strHeader.Find("<MERGEFIELD=",nMergeField+1);
									if(nMergeField == -1) bFoundNext = false;
									else {
										int nMergeFieldBegin = nMergeField + 12;
										//The name of the field is in the first argument of this semicolon-delimited list.
										int nMergeFieldEnd = FindNextNonEscaped(strHeader, '>', nMergeField);
										CString strMergeField = strHeader.Mid(nMergeFieldBegin,nMergeFieldEnd-nMergeFieldBegin);
										int nFirstSemicolon = FindNextNonEscaped(strMergeField, ';', 0);
										strMergeField = DeEscape(strMergeField.Left(nFirstSemicolon));
										if (strMergeField == strOldLabel) {
											// If we get here, we found a match. We need to change the label
											strHeader.Delete(nMergeFieldBegin, strOldLabel.GetLength());
											strHeader.Insert(nMergeFieldBegin, strLabelText);
											bChanged = true;
										}
									}
								} // while(bFoundNext) {
								// If anything changed, then put the header back into the rich text, and
								// change the detail state.
								if (bChanged) {
									strNxRichText.Insert(nHeaderBegin, strHeader);
									pDetail->RequestStateChange((LPCTSTR)strNxRichText);
								}
							}
							else {
								//TES 7/10/2012 - PLID 49346 - If this is an HTML narrative, just go through and find any place where the old
								// name was set as the nxmfname attribute, and replace it.
								if(strNxRichText.Replace("nxmfname=\"" + strOldLabel + "\"", "nxmfname=\"" + strLabelText + "\"")) {
									//TES 7/10/2012 - PLID 49346 - If anything changed, update the detail.
									pDetail->RequestStateChange(_bstr_t(strNxRichText));
								}

							}

						} // if (eitNarrative == pDetail->m_EMRInfoType) {
					} // for (i=0; i < nDetails; i++) {
				} // if (!strLabelText.IsEmpty()) {
			}


			// (c.haag 2008-07-15 16:30) - PLID 17244 - It's possible that we got here because someone changed
			// the name of the EMR info item. If this item is linked with one or more todo alarms, we will update
			// the todo text now. This is because the detail name is calculated in part from EmrInfoT. Having changed
			// the name there is uncancelable. The More Info topic also needs the latest detail name so the client won't
			// get confused.
			if (m_nEMRDetailID > 0) {
				_RecordsetPtr prs = CreateParamRecordset(FormatString("SELECT TaskID,  dbo.GetTodoAssignToIDString(ToDoList.TaskID) as TodoAssigned ,PersonID FROM TodoList WHERE RegardingType = %d AND RegardingID = {INT}", (long)ttEMNDetail), m_nEMRDetailID);
				long nTaskID = -1;
				long nPersonID = -1;
				 CArray<long, long> arrAssignTo;
				if (!prs->eof) {
					nTaskID = AdoFldLong(prs, "TaskID",-1);
					nPersonID = AdoFldLong(prs, "PersonID", -1);
					
					CString  strAssigned = AdoFldString(prs, "TodoAssigned", "");
					
					ParseDelimitedStringToLongArray(strAssigned, " ", arrAssignTo);


				}
				prs->Close();
				if (nTaskID > -1) {					
					// (j.jones 2010-04-20 09:50) - PLID 30852 - converted to ExecuteSqlStd
					// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
					ExecuteParamSql("{SQL}", TodoGetEmrDetailAlarmUpdateQ(-1, m_nEMRDetailID));
					// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo
					if (arrAssignTo.GetSize() == 1){
						CClient::RefreshTodoTable(nTaskID, nPersonID, arrAssignTo[0], TableCheckerDetailIndex::tddisChanged);
					}
					else{
						CClient::RefreshTodoTable(nTaskID, nPersonID, -1, TableCheckerDetailIndex::tddisChanged);
					}
					

					// (c.haag 2008-07-09 09:58) - PLID 30607 - This detail might exist in the EMN more info todo list.
					// So, we will need to refresh the list. Keep in mind that this can cause the EMN more info todo list
					// to not look exactly the same as the follow-up todo list. I don't see a problem with this -- the more
					// info list needs to keep up-to-the-minute changes with the rest of the EMN. Otherwise, clients would
					// get confused.
					if (NULL != m_pParentTopic && NULL != m_pParentTopic->GetParentEMN()) {
						CWnd *pWnd = m_pParentTopic->GetParentEMN()->GetInterface();
						if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
							// Even though it's an existing todo, we use "Added" because all the operation really does is
							// ensure the todo is properly displayed.
							pWnd->PostMessage(NXM_EMN_TODO_ADDED, (WPARAM)m_pParentTopic->GetParentEMN(), nTaskID);
						}
					}
				}
			}

		} // if(m_pParentTopic && !bOriginallyEmpty) {
	} // if(strLabelText != m_strLabelText) {
}

long CEMNDetail::GetSpawnedGroupID()
{
	if(!m_pParentTopic) {
		return -1;
	}

	//This is only valid if we came from a template or an action
	if(m_nEMRSourceTemplateID == -1 && m_sai.nSourceActionID == -1) {
		return -1;
	}

	//TES 2/21/2007 - PLID 24864 - Read out the SourceActionID here, it's referenced again below and we don't want to 
	// call the function twice.
	long nParentSourceActionID = m_pParentTopic->GetSourceActionID();
	if(nParentSourceActionID == -1) {
		return -1;
	}

	if(m_SourceActionDestType == eaoInvalid) {
		//We need to look it up.
		//m_SourceActionDestType = (EmrActionObject)VarLong(GetTableField("EmrActionsT","DestType","ID",m_pParentTopic->GetSourceActionID()),eaoInvalid);

		// (c.haag 2006-03-03 16:27) - PLID 19566 - We now track the source action destination type in the topic
		m_SourceActionDestType = m_pParentTopic->GetSourceActionDestType();
	}
	if(m_SourceActionDestType == eaoMintItems) {
		return nParentSourceActionID;
	}
	else {
		return -1;
	}
}

//TES 3/18/2010 - PLID 37530 - Added, basically the same as GetSpawnedGroupID(), except instead of getting the action ID, it gets the index
// of the spawning stamp within its detail, as that is needed to calculate the "name" of the spawning item.
long CEMNDetail::GetSpawnedGroupStampIndex()
{
	if(!m_pParentTopic) {
		return -1;
	}

	//This is only valid if we came from a template or an action
	if(m_nEMRSourceTemplateID == -1 && m_sai.nSourceActionID == -1) {
		return -1;
	}

	//TES 3/18/2010 - PLID 37530 - Get the parent topic's spawning Stamp Index
	long nParentSourceStampIndex = m_pParentTopic->GetSourceActionInfo().GetStampIndexInDetailByType();
	if(nParentSourceStampIndex == -1) {
		return -1;
	}

	if(m_SourceActionDestType == eaoInvalid) {
		//We need to look it up.
		//m_SourceActionDestType = (EmrActionObject)VarLong(GetTableField("EmrActionsT","DestType","ID",m_pParentTopic->GetSourceActionID()),eaoInvalid);

		// (c.haag 2006-03-03 16:27) - PLID 19566 - We now track the source action destination type in the topic
		m_SourceActionDestType = m_pParentTopic->GetSourceActionDestType();
	}
	if(m_SourceActionDestType == eaoMintItems) {
		return nParentSourceStampIndex;
	}
	else {
		return -1;
	}
}

// (a.walling 2010-04-05 13:16) - PLID 38060 - Gets the SpawnedGroupTableRow for dropdowns in tables
SourceActionInfo CEMNDetail::GetSpawnedGroupSourceActionInfo()
{
	SourceActionInfo saiBlank;

	if(!m_pParentTopic) {
		return saiBlank;
	}

	//This is only valid if we came from a template or an action
	if(m_nEMRSourceTemplateID == -1 && m_sai.nSourceActionID == -1) {
		return saiBlank;
	}

	if(m_SourceActionDestType == eaoInvalid) {
		//We need to look it up.
		//m_SourceActionDestType = (EmrActionObject)VarLong(GetTableField("EmrActionsT","DestType","ID",m_pParentTopic->GetSourceActionID()),eaoInvalid);

		// (c.haag 2006-03-03 16:27) - PLID 19566 - We now track the source action destination type in the topic
		m_SourceActionDestType = m_pParentTopic->GetSourceActionDestType();
	}
	if(m_SourceActionDestType == eaoMintItems) {
		return m_pParentTopic->GetSourceActionInfo();
	}
	else {
		return saiBlank;
	}
}

// (c.haag 2006-03-06 11:45) - PLID 19574 - This returns the name of the "spawned group"
CString CEMNDetail::GetSpawnedGroupIDName()
{
	if(!m_pParentTopic) {
		return "";
	}

	if(m_pParentTopic->GetSourceActionID() == -1) {
		return "";
	}

	//This is only valid if we came from a template or an action
	if(m_nEMRSourceTemplateID == -1 && m_sai.nSourceActionID == -1) {
		return "";
	}

	if(m_SourceActionDestType == eaoInvalid) {
		//We need to look it up.
		//m_SourceActionDestType = (EmrActionObject)VarLong(GetTableField("EmrActionsT","DestType","ID",m_pParentTopic->GetSourceActionID()),eaoInvalid);

		// (c.haag 2006-03-03 16:27) - PLID 19566 - We now track the source action destination type in the topic
		m_SourceActionDestType = m_pParentTopic->GetSourceActionDestType();
	}
	if(m_SourceActionDestType == eaoMintItems) {
		return m_pParentTopic->GetSourceActionName();
	}
	else {
		return "";
	}
}

// (z.manning 2012-07-16 16:57) - PLID 33710
BOOL CEMNDetail::IsNew()
{
	if(m_bIsTemplateDetail) {
		return (m_nEMRTemplateDetailID == -1);
	}
	else {
		return (m_nEMRDetailID == -1);
	}
}

void CEMNDetail::SetNew()
{
	if(m_bIsTemplateDetail) {
		m_nEMRTemplateDetailID = -1;
		m_nChildEMRTemplateDetailID = -1;
	}
	else {
		m_nEMRDetailID = -1;
		m_nChildEMRDetailID = -1;
	}

	// (j.jones 2008-07-21 09:16) - PLID 30779 - set all problems as new
	// and reconfirm their pointers are correct with this object
	// (z.manning 2009-05-22 09:41) - PLID 34297 - We now have links to problems
	for(int i = 0; i < m_apEmrProblemLinks.GetSize(); i++)
	{
		CEmrProblemLink *pProblemLink = m_apEmrProblemLinks.GetAt(i);
		if(pProblemLink != NULL) {
			pProblemLink->SetID(-1);
			pProblemLink->SetRegardingID(-1);
			pProblemLink->UpdatePointersWithDetail(this);
		}
	}

	// (j.jones 2010-02-25 16:22) - PLID 37231 - ensure the loaded stamps are new
	BOOL bSmartStampTableChanged = FALSE;
	for(int i = 0; i < m_arypImageStamps.GetSize(); i++)
	{
		EmrDetailImageStamp *pDetailImageStamp = m_arypImageStamps.GetAt(i);
		if(pDetailImageStamp != NULL)
		{
			if(pDetailImageStamp->nID != -1)
			{
				// (z.manning 2011-03-04 18:26) - PLID 42682 - We need to update any source stamp IDs that
				// may have been referencing this ID before the detail was flagged as new.
				if(GetParentEMN() != NULL) {
					GetParentEMN()->UpdateSourceDetailStampIDs(pDetailImageStamp->nID, -1);
				}

				if(IsSmartStampImage()) {
					// (z.manning 2011-03-04 10:50) - PLID 42336 - If this is a smart stamp image then we
					// need to also need to potentially update the table row corresponding to this stamp as well.
					TableRow *ptr = m_pSmartStampTableDetail->GetRowByDetailStamp(pDetailImageStamp);
					if(ptr != NULL) {
						// (c.haag 2012-10-26) - PLID 53440 - Use SetNew on the TableRowID object
						ptr->m_ID.SetNew();
						bSmartStampTableChanged = TRUE;
					}
				}

				pDetailImageStamp->nID = -1;
				// (z.manning 2013-03-12 17:17) - PLID 55016 - Clear out the copied from detail stamp
				pDetailImageStamp->SetCopiedFromDetailStamp(NULL);
			}
		}
	}
	if(bSmartStampTableChanged) {
		// (z.manning 2011-03-04 10:53) - PLID 42336 - If we changed the smart stamp table then we need to refresh its state.
		m_pSmartStampTableDetail->RecreateStateFromContent();
	}

	// (j.jones 2010-02-25 16:22) - PLID 37231 - ensure stamps stored in the table data are new
	{
		BOOL bTableChanged = FALSE;
		// (z.manning 2010-02-26 10:57) - PLID 37412 - We now loop through rows instead of table elements
		for(int i = 0; i < m_arTableRows.GetSize(); i++) {
			TableRow *ptr = m_arTableRows.GetAt(i);
			if(ptr->m_ID.GetDetailImageStampID() != -1) {
				// (c.haag 2012-10-26) - PLID 53440 - Use SetNew on the TableRowID object
				ptr->m_ID.SetNew();
				bTableChanged = TRUE;
			}
		}
		if(bTableChanged) {
			RecreateStateFromContent();
		}
	}

	//TES 7/9/2012 - PLID 51359 - Reset any IDs from the narrative that may be out of date now.
	DereferenceNarrative();

	// (j.jones 2007-01-23 09:41) - PLID 24027 - empty out the "copied from detail" pointer
	m_pCopiedFromDetail = NULL;

	// (j.jones 2008-09-10 13:23) - PLID 31304 - Logic would suggest that all new details should
	// be non-read-only, and since we call SetUnsaved() in this function, the program is already
	// making this assumption. So we should be able to just call SetReadOnly(FALSE) and be done with it.
	// But for posterity, recalculate the status anyways and attempt to reset it, but only if we have
	// a parent EMN to check for.
	if(m_pParentTopic) {
		if(m_pParentTopic->GetParentEMN()) {
			BOOL bLocked = m_pParentTopic->GetParentEMN()->GetStatus() == 2;
			// (a.walling 2007-11-28 11:20) - PLID 28044 - Check for expiration
			// (a.walling 2008-06-09 13:16) - PLID 22049 - Also include EMN writable status with bCanWrite
			BOOL bCanWrite = CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) && (g_pLicense->HasEMR(CLicense::cflrSilent) == 2) && m_pParentTopic->GetParentEMN()->IsWritable();

			BOOL bSpawnedDetail = (m_pParentTopic->GetParentEMN()->IsTemplate() && (m_pParentTopic->GetSourceActionID() != -1));
				
			// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
			if (!m_pParentTopic->GetTopicWnd()) {
				SetReadOnly((bLocked || !bCanWrite || bSpawnedDetail) ? TRUE : FALSE);
			}
		}
	}

	SetUnsaved();
}

void CEMNDetail::RemoveListElement(long nID)
{
	for(int i = 0; i < m_arListElements.GetSize(); i++) {
		if(m_arListElements[i].nID == nID) m_arListElements.RemoveAt(i);
	}
	SyncContentAndState();
	ReflectCurrentContent();
	ReflectCurrentState();
}

// (z.manning 2011-11-04 17:53) - PLID 42765
ListElement* CEMNDetail::GetListElementByID(const long nListElementID)
{
	for(int nListElementIndex = 0; nListElementIndex < m_arListElements.GetCount(); nListElementIndex++)
	{
		ListElement *ple = &m_arListElements.GetAt(nListElementIndex);
		if(ple->nID == nListElementID) {
			return ple;
		}
	}

	return NULL;
}

void CEMNDetail::ClearContent()
{
	m_arListElements.RemoveAll();
	m_arTableElements.RemoveAll();
	//TES 3/18/2011 - PLID 41108 - Also clear out our cache of info about selected dropdown items.
	for(int i = 0; i < m_arypTableSelectedDropdowns.GetSize(); i++) {
		delete m_arypTableSelectedDropdowns[i];
	}
	m_arypTableSelectedDropdowns.RemoveAll();

	//TES 7/26/2007 - PLID 25091 - We just lost any details stored in m_arTableElements, so we can't trust our
	// cached array of linked details any more.
	m_bLinkedDetailsCached = false;

	for(i = 0; i < m_arTableRows.GetSize(); i++) {
		delete m_arTableRows[i];
	}
	m_arTableRows.RemoveAll();
	for(i = 0; i < m_arTableColumns.GetSize(); i++) {
		delete m_arTableColumns[i];
	}
	m_arTableColumns.RemoveAll();

	m_mapColumnKeysToQueries.RemoveAll();

	// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

	// (z.manning, 01/23/2008) - Remove all hot spots.
	m_aryImageHotSpots.Clear();

	// (j.jones 2012-10-31 13:47) - PLID 53450 - if a table, clear our cached dropdown information
	if(m_EMRInfoType == eitTable && m_pEmrItemAdvDlg != NULL) {
		((CEmrItemAdvTableDlg*)m_pEmrItemAdvDlg)->ClearDropdownSourceInfoMap();
	}

	// (j.jones 2008-07-18 11:06) - PLID 30779 - remove all problems
	// (c.haag 2008-07-22 15:45) - PLID 30779 - Don't remove all problems here;
	// do it in the destructor. We never did this in past releases; and if we did
	// do it now, then any problems spawned to a newly added Emr Item for that Emr
	// Item would be wiped out.
	//
	// This should give at least a little pause in thought because problems
	// are "sort-of" content. Problem data is indeed loaded from LoadContent (and
	// it's smart enough to not duplicate problems in memory)...however, it's only
	// loaded there because it's not state data. There's no pressing need to clean
	// up how problems are loaded, so we'll leave things be until that need arises.
	//
}

CArray<ListElement,ListElement&>& CEMNDetail::GetListElements()
{
	LoadContent();
	return m_arListElements;
}

int CEMNDetail::GetListElementCount()
{
	LoadContent();
	return m_arListElements.GetSize();
}

ListElement CEMNDetail::GetListElement(int nIndex)
{
	LoadContent();
	return m_arListElements.GetAt(nIndex);
}

CString CEMNDetail::GetListElementIDsCommaDelimited()
{
	LoadContent();
	CString strKeys;
	for(int i = 0; i < m_arListElements.GetSize(); i++) strKeys += FormatString("%li",m_arListElements.GetAt(i).nID)+",";
	strKeys.TrimRight(",");
	if(strKeys.IsEmpty()) strKeys = "-1";
	return strKeys;
}

// (a.walling 2012-10-31 17:17) - PLID 53550 - CEMNDetail::opreator= leads to very confusing semantics
//void CEMNDetail::operator =(CEMNDetail &edSource)
void CEMNDetail::CopyFrom(CEMNDetail &edSource)
{
	if(IsWindow(edSource.m_pEmrItemAdvDlg->GetSafeHwnd())) {
		edSource.m_pEmrItemAdvDlg->GetClientRect(&m_rcDefaultClientArea);
		edSource.m_pEmrItemAdvDlg->ClientToScreen(&m_rcDefaultClientArea);
		edSource.m_pEmrItemAdvDlg->GetParent()->SendMessage(NXM_CONVERT_RECT_FOR_DATA, (WPARAM)&m_rcDefaultClientArea);
	}
	else {
		m_rcDefaultClientArea = edSource.m_rcDefaultClientArea;
	}

	m_bReadOnly = edSource.m_bReadOnly;
	m_bVisible = edSource.m_bVisible;

	m_nEMRInfoID = edSource.m_nEMRInfoID;
	m_nEMRInfoMasterID = edSource.m_nEMRInfoMasterID;
	m_EMRInfoType = edSource.m_EMRInfoType;
	m_EMRInfoSubType = edSource.m_EMRInfoSubType;
	m_bTableRowsAsFields = edSource.m_bTableRowsAsFields; // (c.haag 2008-10-16 11:25) - PLID 31709
	m_strMergeFieldOverride = edSource.m_strMergeFieldOverride;
	m_nEMRDetailID = edSource.m_nEMRDetailID;
	// (m.hancock 2006-06-06 12:54) - PLID 20519 - Keep the detailID of the source when copying a detail
	// (j.jones 2009-04-10 09:23) - PLID 33956 - renamed from m_nSourceEMRDetailID to m_nCopiedFromEMRDetailID
	m_nCopiedFromEMRDetailID = edSource.m_nEMRDetailID;
	m_nEMRSourceTemplateID = edSource.m_nEMRSourceTemplateID;
	m_nEMRTemplateDetailID = edSource.m_nEMRTemplateDetailID;
	m_bIsTemplateDetail = edSource.m_bIsTemplateDetail;

	// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
	m_nChildEMRInfoMasterID = edSource.m_nChildEMRInfoMasterID;
	m_bSmartStampsEnabled = edSource.m_bSmartStampsEnabled;
	m_nChildEMRDetailID = edSource.m_nChildEMRDetailID;
	m_nChildEMRTemplateDetailID = edSource.m_nChildEMRTemplateDetailID;

	//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
	m_bHasGlassesOrderData = edSource.m_bHasGlassesOrderData;
	m_golLens = edSource.m_golLens;
	//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
	m_bHasContactLensData = edSource.m_bHasContactLensData;
	// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
	m_bUseWithWoundCareCoding = edSource.m_bUseWithWoundCareCoding;

	// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
	m_nInfoFlags = edSource.m_nInfoFlags;

	// (z.manning 2011-11-16 12:55) - PLID 38130
	m_nRememberedDetailID = edSource.m_nRememberedDetailID;

	// (z.manning 2010-03-30 10:04) - PLID 37985 - This was inadvertantly removed in 9500.
	m_bSaveTableColumnWidths = edSource.m_bSaveTableColumnWidths;

	// (j.jones 2010-02-12 09:29) - PLID 37318 - copy the smart stamp pointers
	// (j.jones 2010-03-02 11:32) - PLID 37594 - add reference counts
	// (z.manning 2011-01-20 13:07) - PLID 42338 - Support multiple images per smart stamp table
	SetSmartStampImageDetails(edSource.m_arySmartStampImageDetails);
	if(edSource.m_pSmartStampTableDetail) {
		edSource.m_pSmartStampTableDetail->__AddRef(FormatString("CEMNDetail::operator = copying table pointer (this image = 0x%08x)", (long)this));
	}
	m_pSmartStampTableDetail = edSource.m_pSmartStampTableDetail;

	m_strSentenceHTML = edSource.m_strSentenceHTML;
	m_strSentenceNonHTML = edSource.m_strSentenceNonHTML;
	m_emsf = edSource.m_emsf;
	m_strSeparator = edSource.m_strSeparator;
	m_strSeparatorFinal = edSource.m_strSeparatorFinal;

	m_varState = edSource.m_varState;
	// (r.gonet 02/14/2012) - Copy the image text filter
	m_pTextStringFilter = edSource.m_pTextStringFilter;

	m_pEmrItemAdvDlg = NULL;

	//Do we already have a parent topic?
	if(!m_pParentTopic) {
		//Nope, use the source's parent topic.
		m_pParentTopic = edSource.m_pParentTopic;		
	}
	if(!m_pOriginalParentTopic) {
		m_pOriginalParentTopic = edSource.m_pOriginalParentTopic;
	}

	if(m_pOriginalParentTopic) {
		m_strLastSavedParentTopicName = m_pOriginalParentTopic->GetName();
	}

	m_bAllowEdit = edSource.m_bAllowEdit;

	m_strLabelText = edSource.m_strLabelText;

	//DRT 8/2/2007 - PLID 26919
	m_nSourceActionSourceID = edSource.m_nSourceActionSourceID;
	//DRT 8/14/2007 - PLID 27067
	m_nSourceActionSourceDataGroupID = edSource.m_nSourceActionSourceDataGroupID;
	m_nSourceActionSourceHotSpotGroupID = edSource.m_nSourceActionSourceHotSpotGroupID;
	m_nSourceActionSourceTableDropdownGroupID = edSource.m_nSourceActionSourceTableDropdownGroupID; // (z.manning 2009-02-13 10:11) - PLID 33070
	m_SourceActionDestType = edSource.m_SourceActionDestType;
	// (j.jones 2007-01-10 15:06) - PLID 24027 - supported SourceDetailID
	// (z.manning 2009-03-11 10:45) - PLID 33338 - Use the new source action info class
	m_sai = edSource.m_sai;

	// (j.jones 2007-07-18 13:23) - PLID 26730 - tracks whether we have Info actions	
	m_eHasInfoActions = edSource.m_eHasInfoActions;

	// (j.jones 2007-01-23 09:41) - PLID 24027 - keep a temporary copy of edSource
	m_pCopiedFromDetail = &edSource;

	m_bMergeNameConflicts = edSource.m_bMergeNameConflicts;

	// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
	int i = 0;

	for(i = 0; i < edSource.m_arListElements.GetSize(); i++)
		m_arListElements.Add(edSource.m_arListElements[i]);
	
	for(i = 0; i < edSource.m_arTableColumns.GetSize(); i++) {
		TableColumn *pTc = new TableColumn;
		*pTc = *(edSource.m_arTableColumns[i]);
		m_arTableColumns.Add(pTc);
	}
	for(i = 0; i < edSource.m_arTableRows.GetSize(); i++) {
		TableRow *pTr = new TableRow;
		*pTr = *(edSource.m_arTableRows[i]);
		m_arTableRows.Add(pTr);
	}
	//DRT 7/11/2007 - PLID 24105 - We need to copy the saved column widths
	m_tcwWidths.CopyFrom(edSource.GetColumnWidths());
	//Note: We don't load the table elements, they will be loaded properly when we call SyncContentAndState().

	{
		long nKey;
		CSqlFragment sql;
		POSITION p = edSource.m_mapColumnKeysToQueries.GetStartPosition();
		while(p) {
			edSource.m_mapColumnKeysToQueries.GetNextAssoc(p, nKey, sql);
			m_mapColumnKeysToQueries.SetAt(nKey, sql);
		}
	}

	// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

	m_strLongForm = edSource.m_strLongForm;
	m_nDataFormat = edSource.m_nDataFormat;
	m_strDataSeparator = edSource.m_strDataSeparator;
	m_strDataSeparatorFinal = edSource.m_strDataSeparatorFinal;

	// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
	// because they are now only calculated in the API, and not in Practice code
	/*
	// (j.jones 2007-08-27 10:35) - PLID 27056 - load the E/M coding data
	m_nEMCodeCategoryID = edSource.m_nEMCodeCategoryID;
	// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_eEMCodeUseTableCategories
	m_eEMCodeUseTableCategories = edSource.m_eEMCodeUseTableCategories;
	m_bUseEMCoding = edSource.m_bUseEMCoding;
	m_emctEMCodingType = edSource.m_emctEMCodingType;
	*/

	m_dSliderMin = edSource.m_dSliderMin;
	m_dSliderMax = edSource.m_dSliderMax;
	m_dSliderInc = edSource.m_dSliderInc;

	m_strBackgroundImageFilePath = edSource.m_strBackgroundImageFilePath;
	m_eitBackgroundImageType = edSource.m_eitBackgroundImageType;

	m_bLoadedDetailImageStampsFromVariant = edSource.m_bLoadedDetailImageStampsFromVariant;

	m_bNeedToLoadContent = edSource.m_bNeedToLoadContent;

	m_bIsPoppedUp = edSource.m_bIsPoppedUp; // (z.manning 2010-02-26 18:12) - PLID 37230

	// (j.jones 2007-07-25 17:29) - PLID 26810 - added m_bIsForcedReload
	m_bIsForcedReload = edSource.m_bIsForcedReload;

	// (c.haag 2007-03-29 17:00) - PLID 25423 - Copying merge fields the dynamic allocation way
	// (c.haag 2007-04-03 17:47) - PLID 25488 - Traverse a map rather than an array
	// (c.haag 2007-05-10 13:07) - PLID 25958 - Traverse a recordset rather than a map
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	/*
	m_bUsingParentEMNMergeFieldMap = edSource.m_bUsingParentEMNMergeFieldMap;
	if (NULL != edSource.m_prsNarrativeFields) {
		EnsureNarrativeFieldRecordset();
		CopyNarrativeFieldSet(m_prsNarrativeFields, edSource.m_prsNarrativeFields);
	}
	*/

	// (j.jones 2008-07-18 12:05) - PLID 30779 - problems are now stored in an array of objects
	// (c.haag 2009-05-19 10:36) - PLID 34311 - Now we store problem links
	for(i = 0; i < edSource.m_apEmrProblemLinks.GetSize(); i++) {
		// (c.haag 2009-07-09 10:49) - PLID 34829 - Now that the parent EMR is responsible for
		// problem allocation, we must try to ensure that the new link is bound to a problem for
		// this EMR. Determine the EMR that owns this detail, and pass it into the EMR problem
		// link ctor so it can do this.
		CEMR* pOwningEMR = NULL;
		if (NULL != m_pParentTopic && NULL != m_pParentTopic->GetParentEMN()) {
			pOwningEMR = m_pParentTopic->GetParentEMN()->GetParentEMR();
		}
		CEmrProblemLink *pNewLink = new CEmrProblemLink(edSource.m_apEmrProblemLinks[i], pOwningEMR);
		pNewLink->UpdatePointersWithDetail(this);
		m_apEmrProblemLinks.Add(pNewLink);
	}

	// (z.manning, 02/21/2008) - Need to copy hot spots
	m_aryImageHotSpots.CopyFromArray(edSource.m_aryImageHotSpots);

	// (z.manning 2010-02-17 11:49) - PLID 37412
	m_arypImageStamps.RemoveAll();
	for(int nStampIndex = 0; nStampIndex < edSource.m_arypImageStamps.GetSize(); nStampIndex++) {
		EmrDetailImageStamp *pSource = edSource.m_arypImageStamps.GetAt(nStampIndex);
		// (z.manning 2010-03-01 14:09) - PLID 37412 - Make sure we add a reference to the detail
		// stamp pointer
		pSource->AddRef();
		m_arypImageStamps.Add(pSource);
	}

	/* (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore zoom and pan offsets.
	m_aisState.m_dZoomLevel = edSource.m_aisState.m_dZoomLevel;
	m_aisState.m_nOffsetX = edSource.m_aisState.m_nOffsetX;
	m_aisState.m_nOffsetY = edSource.m_aisState.m_nOffsetY;
	*/

	// (a.walling 2012-10-31 17:17) - PLID 53550 - LastSavedDetail - reset
	m_oLastSavedDetail = boost::none;

	// (z.manning, 03/10/2008) - PLID 29243 - Make sure we copy the review state
	m_nReviewState = edSource.m_nReviewState;

	//TES 3/28/2006 - Don't call SyncContentAndState() if the content hasn't been loaded yet.
	if(!m_bNeedToLoadContent) {
		SyncContentAndState();
	}

	m_bModified = edSource.m_bModified;

	// (a.walling 2007-12-17 15:49) - PLID 28391
	m_bHTMLVisiblePrint = edSource.m_bHTMLVisiblePrint;
	m_bHTMLVisible = edSource.m_bHTMLVisiblePrint;

	// (a.walling 2008-06-30 13:59) - PLID 29271 - Preview Pane flags
	m_nPreviewFlags = edSource.m_nPreviewFlags;

	// (z.manning 2008-10-08 15:06) - PLID 31613 - Lab ID
	m_varLabID = edSource.m_varLabID;

	
	// (a.walling 2009-01-13 13:53) - PLID 32107 - Initialize the image background info
	m_varInfoBackgroundImageFilePath = edSource.m_varInfoBackgroundImageFilePath;
	m_varInfoBackgroundImageType = edSource.m_varInfoBackgroundImageType;

	// (a.walling 2009-11-17 12:02) - PLID 36365 - Just in case
	m_bUpdatingNarrativeFields = edSource.m_bUpdatingNarrativeFields;

	// (c.haag 2011-03-23) - PLID 42895 - Copy common lists
	m_CommonLists = edSource.m_CommonLists;

	m_StampExclusions = edSource.m_StampExclusions; // (z.manning 2011-10-25 13:03) - PLID 39401
	m_bStampExclusionsLoaded = edSource.m_bStampExclusionsLoaded; // (a.wilson 2013-03-26 11:00) - PLID 55826 - set loaded flag.
	// (j.armen 2014-07-23 10:37) - PLID 62836
	m_nDefaultPenColor = edSource.m_nDefaultPenColor;
	m_nDefaultPenSizePercent = edSource.m_nDefaultPenSizePercent;
}

// (j.jones 2007-08-27 11:16) - PLID 27056 - added E/M coding data
// (z.manning, 05/23/2008) - PLID 30155 - Added Formula and DecimalPlaces
// (z.manning 2009-01-15 15:29) - PLID 32724 - Added InputMask
// (z.manning 2010-02-16 14:44) - PLID 37230 - ListSubType
// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
// (z.manning 2010-04-13 16:17) - PLID 38175 - IsLabel
// (z.manning 2010-07-29 15:04) - PLID 36150 - Added sentence format
// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
// (j.jones 2011-03-09 09:05) - PLID 42283 - added nEMCodeCategoryID
// (z.manning 2011-03-11) - PLID 42778 - Added bHadDropdownElements
//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
// (z.manning 2011-03-21 11:20) - PLID 30608 - Added autofill type
// (z.manning 2011-05-26 14:56) - PLID 43865 - Added flags
// (z.manning 2011-09-19 14:50) - PLID 41954 - Added dropdown separators
// (z.manning 2011-11-07 10:47) - PLID 46309 - Added SpawnedItemsSeparator
//(e.lally 2011-12-08) PLID 46471 - Added isCurrentMedOrAllergyUsageCol
// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType which tells the the CWoundCareCalculator 
//  that this column is special and should be used in calculation. wcdtNone is the default, non-special value.
// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
void CEMNDetail::AddColumn(long nID, const CString &strName, long nType, BOOL bIsGrouped, CString strFormula, BYTE nDecimalPlaces,
						   CString strInputMask, BYTE nSubType, BOOL bAutoAlphabetizeDropdown, BOOL bIsLabel, CString strLongForm,
						   EEmrTableAutoNumberType etantAutoNumberType, CString strAutoNumberPrefix, BOOL bHasDropdownElements, BOOL bHasActiveDropdownElements,
						   GlassesOrderDataType godtGlassesOrderDataType, long nGlassesOrderDataID, BYTE nAutofillType, const long nFlags,
						   LPCTSTR strDropdownSeparator, LPCTSTR strDropdownSeparatorFinal, LPCTSTR strSpawnedItemsSeparator, BOOL bIsCurrentMedOrAllergyUsageCol,
						   EWoundCareDataType ewccWoundCareDataType)
{
	for(int i = 0; i < m_arTableColumns.GetSize(); i++) {
		if(m_arTableColumns[i]->nID == nID) {
			//Found it!  Make sure it's up to date.
			m_arTableColumns[i]->strName = strName;
			m_arTableColumns[i]->nType = nType;
			m_arTableColumns[i]->nSubType = nSubType;
			m_arTableColumns[i]->bIsGrouped = bIsGrouped;
			return;
		}
	}

	//Need to add it.
	TableColumn *pTc = new TableColumn;
	pTc->nID = nID;
	pTc->strName = strName;
	pTc->nType = nType;
	pTc->nSubType = nSubType;
	pTc->bIsGrouped = bIsGrouped;
	// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
	pTc->m_strFormula = strFormula;
	pTc->m_nDecimalPlaces = nDecimalPlaces;
	pTc->m_strInputMask = strInputMask;
	pTc->m_bAutoAlphabetizeDropdown = bAutoAlphabetizeDropdown; // (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
	pTc->m_bIsLabel = bIsLabel; // (z.manning 2010-04-13 16:18) - PLID 38175
	pTc->m_strLongForm = strLongForm; // (z.manning 2010-07-29 15:05) - PLID 36150
	// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
	pTc->m_etantAutoNumberType = etantAutoNumberType;
	pTc->m_strAutoNumberPrefix = strAutoNumberPrefix;
	pTc->m_bHasDropdownElements = bHasDropdownElements;
	pTc->m_bHasActiveDropdownElements = bHasActiveDropdownElements;
	//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
	pTc->m_GlassesOrderDataType = godtGlassesOrderDataType;
	pTc->m_nGlassesOrderDataID = nGlassesOrderDataID;
	pTc->m_eAutofillType = (EmrTableAutofillType)nAutofillType; // (z.manning 2011-03-21 11:26) - PLID 30608
	pTc->m_nFlags = nFlags;
	pTc->m_strDropdownSeparator = strDropdownSeparator;
	pTc->m_strDropdownSeparatorFinal = strDropdownSeparatorFinal;
	pTc->m_strSpawnedItemsSeparator = strSpawnedItemsSeparator; // (z.manning 2011-11-07 10:48) - PLID 46309
	pTc->m_bIsCurrentMedOrAllergyUsageCol = bIsCurrentMedOrAllergyUsageCol;  //(e.lally 2011-12-08) PLID 46471
	pTc->m_ewccWoundCareDataType = ewccWoundCareDataType; // (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
	m_arTableColumns.Add(pTc);	
}

// (j.jones 2007-08-27 11:16) - PLID 27056 - added E/M coding data
// (z.manning 2008-06-06 10:12) - PLID 30155 - Added formula and decimal places
// (z.manning 2009-02-24 12:46) - PLID 33141 - Added group ID
// (z.manning 2010-02-18 10:47) - PLID 37427 - Row ID is now a struct
// (z.manning 2010-04-13 16:17) - PLID 38175 - IsLabel
// (j.jones 2011-03-09 09:05) - PLID 42283 - added nEMCodeCategoryID
//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
// (z.manning 2011-05-26 14:57) - PLID 43865 - Added flags
// (c.haag 2011-05-31) - PLID 43875 - We can now specify where the row goes. nPlacementIndex should be -1 unless placement is aroInsertAt, in which
// case the row will be inserted at nPlacementIndex.
// (a.walling 2011-08-11 15:49) - PLID 44987 - Returns a TableRow* now; also takes the table row ID as a const reference
//(e.lally 2011-12-08) PLID 46471 - Added bIsCurrentMedOrAllergy
// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
TableRow* CEMNDetail::AddRow(const TableRowID& rowid, const CString &strName, const CString strFormula, const BYTE nDecimalPlaces, const long nGroupID, BOOL bIsLabel,
					   GlassesOrderDataType godtGlassesOrderDataType, long nGlassesOrderDataID, const long nFlags, EAddRowPlacement placement, long nPlacementIndex, BOOL bIsCurrentMedOrAllergy)
{
	//for(int i = 0; i < m_arTableRows.GetSize(); i++) {
	//	if(*m_arTableRows[i]->m_pID == rowid) {
	//		//Found it!  Make sure the name is up to date.
	//		m_arTableRows[i]->strName = strName;
	//		return i;
	//	}
	//}

	//We didn't have it already.

	TableRow *pTr = new TableRow;
	pTr->m_ID = rowid;
	pTr->strName = strName;
	// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
	// (z.manning 2008-06-06 08:22) - PLID 30155 - Added formula and decimal places
	pTr->m_strFormula = strFormula;
	pTr->m_nDecimalPlaces = nDecimalPlaces;
	pTr->nGroupID = nGroupID;  // (z.manning 2009-02-24 14:43) - PLID 33141
	pTr->m_bIsLabel = bIsLabel; // (z.manning 2010-04-13 16:18) - PLID 38175
	//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
	pTr->m_GlassesOrderDataType = godtGlassesOrderDataType;
	pTr->m_nGlassesOrderDataID = nGlassesOrderDataID;
	pTr->m_nFlags = nFlags;

	//(e.lally 2011-12-08) PLID 46471 - Specify if this is for a current medications or allergies detail
	pTr->m_bIsCurrentMedOrAllergy = bIsCurrentMedOrAllergy;
	//(e.lally 2011-12-08) PLID 46471 - Default this to false always. It will be manually flagged later
	pTr->m_bIsCurrentMedOrAllergyChecked = FALSE;

	// (c.haag 2011-05-31) - PLID 43875 - If the placement is sequential, just tack it on at the end. This
	// is the default value, and most callers use it because the rows were already sorted before the AddRow
	// iterations.
	if (aroSequential == placement) 
	{
		m_arTableRows.Add(pTr);
	}
	// (c.haag 2011-05-31) - PLID 43875 - If the placement is defined by an index, add to the proper slot
	else if (aroInsertAt == placement)
	{
		m_arTableRows.InsertAt(nPlacementIndex, pTr);
	}
	// (c.haag 2011-05-31) - PLID 43875 - If the ordering is alphabetical, seek to the proper slot
	else if (aroAlphabetically == placement)
	{
		// First, figure out where to start looking. We accomplisht his with a binary "search"
		int min = 0, max = m_arTableRows.GetSize() - 1;
		while (min < max) {
			int c = (min + max) / 2;
			int cmp = strName.Compare(m_arTableRows[c]->strName);
			if (cmp < 0) {
				max = c - 1;
			} else if (cmp > 0) {
				min = c + 1;
			} else {
				min = max;
			}
		}
		// Now move the index until we find the first satisfactory place for the new row. This would work just as
		// well had we not done the binary search (just slower). Given an index I, the action is:
		//
		// 1. Move I forward until strName is a "greater" value than m_arTableRows[i]
		// 2. Move I backward until strName is a "lesser" value than m_arTableRows[i]
		// 3. Insert at I+1
		//
		// If it's hard to follow...try doing it on paper or running the loops through without doing the binary search.
		//
		int i = min;
		// 1. Move i forward until strName is a "greater" value than m_arTableRows[i]
		while (i < m_arTableRows.GetSize() && strName.CompareNoCase(m_arTableRows[i]->strName) > 0) { i++; }
		// (z.manning 2012-02-22 11:33) - PLID 48309 - We may have made it past the end in which case we
		// want to just add it to the end of the list.
		if(i >= m_arTableRows.GetSize()) {
			m_arTableRows.Add(pTr);
		}
		else {
			// 2. Move i backward until strName is a "lesser" value than m_arTableRows[i]
			while (i >= 0 && strName.CompareNoCase(m_arTableRows[i]->strName) < 0) { i--; }
			// 3. At this point, we know i points to the final element that comes before strName, so we can now add the new 
			// row at i+1. 
			m_arTableRows.InsertAt(i+1, pTr);
		}
	}
	else
	{
		ThrowNxException("CEMNDetail::AddRow was called with an invalid order parameter!");
	}
	
	return pTr;
}

// (z.manning 2010-02-16 14:19) - PLID 37230 - Function to add a new row from a smart stamp
//TES 3/17/2010 - PLID 37530 - Added an argument for the stamp's index by type (i.e., is this the first or second AK)
int CEMNDetail::AddSmartStampRow(EmrDetailImageStamp *pDetailImageStamp, long nStampIndexInDetailByType)
{
	TableRow *ptrNew = new TableRow;
	ptrNew->m_ID = TableRowID(pDetailImageStamp, nStampIndexInDetailByType);
	ptrNew->strName = "";

	int nReturn = -1;
	// (z.manning 2011-02-23 14:47) - PLID 42338 - Now that we support multiple images per smart stamp table
	// we (unfortunately) cannot guarantee that all images linked to a table will be loaded before the table
	// is so we must now factor in the order index when adding rows here.
	if(pDetailImageStamp->nOrderIndex != -1)
	{
		for(int nRowIndex = 0; nRowIndex < m_arTableRows.GetCount() && nReturn == -1; nRowIndex++)
		{
			TableRow *ptrTemp = m_arTableRows.GetAt(nRowIndex);
			// (c.haag 2012-10-26) - PLID 53440 - Use the getter function
			if(ptrTemp->m_ID.GetDetailImageStampObject() != NULL)
			{
				if(pDetailImageStamp->nOrderIndex <= ptrTemp->m_ID.GetDetailImageStampObject()->nOrderIndex)
				{
					if(pDetailImageStamp->nOrderIndex == ptrTemp->m_ID.GetDetailImageStampObject()->nOrderIndex) {
						// (z.manning 2011-02-23 14:56) - PLID 42338 - We can now have multiple images per smart
						// stamp table, but the same smart stamp table can be associated with different combinations
						// of images on different EMNs. Because of that fact, it's entirely possible we may
						// have duplicate order indices when remembering values from previous data. So if that's the case
						// here make sure we don't save duplicate indices to data for this detail.
						for(int nIncreaseOrderIndex = nRowIndex; nIncreaseOrderIndex < m_arTableRows.GetCount(); nIncreaseOrderIndex++) {
							TableRow *ptrIncreaseOrder = m_arTableRows.GetAt(nIncreaseOrderIndex);
							if(ptrIncreaseOrder->m_ID.GetDetailImageStampObject() != NULL) {
								ptrIncreaseOrder->m_ID.GetDetailImageStampObject()->nOrderIndex++;
							}
						}
					}

					m_arTableRows.InsertAt(nRowIndex, ptrNew);
					nReturn = nRowIndex;
				}
			}
		}
	}

	if(nReturn == -1) {
		nReturn = m_arTableRows.Add(ptrNew);
	}

	return nReturn;
}

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

void CEMNDetail::GetDetailNameAndDataForAudit(CString &strDetailName, CString &strDetailData)
{
	//return the DetailName and the data about to be saved in that detail

	//we already have the detail name
	strDetailName = GetMergeFieldName(FALSE);
	strDetailData = "";

	//now determine the detail data
	switch(m_EMRInfoType) {
		case eitText:
			strDetailData = VarString(m_varState, "");
			break;
		case eitSingleList:	{
			// (j.jones 2006-03-14 15:48) - 19699 - we search m_arListElements for the data name, 
			// instead of querying the database

			LoadContent();

			strDetailData = "";
			for(int i=0; i<m_arListElements.GetSize(); i++) {
				if(m_arListElements.GetAt(i).nID == atol(VarString(m_varState, ""))) {
					strDetailData = m_arListElements.GetAt(i).strName;
				}
			}
			if(strDetailData.IsEmpty())
				strDetailData = "{No Selection}";
			}
			break;
		case eitMultiList:	{
			// (j.jones 2006-03-14 15:48) - 19699 - we search m_arListElements for the data name, 
			// instead of querying the database

			LoadContent();

			CString strDataIDs = VarString(m_varState, "");
			strDataIDs.TrimLeft();
			strDataIDs.TrimRight();
			if(strDataIDs.IsEmpty()) {
				strDetailData = "{No Selection}";
			}
			else {
				strDetailData = "";
				strDataIDs.Replace(";",",");
				while(strDataIDs.GetLength() > 0) {

					long nComma = strDataIDs.Find(",");
					CString strIDToReference;
					if(nComma == -1) {
						strIDToReference = strDataIDs;
						strDataIDs = "";
					}
					else {
						strIDToReference = strDataIDs.Left(nComma);
						strIDToReference.TrimRight();
						strDataIDs = strDataIDs.Right(strDataIDs.GetLength() - nComma - 1);
						strDataIDs.TrimLeft();
						strDataIDs.TrimRight();
					}

					for(int i=0; i<m_arListElements.GetSize(); i++) {
						if(m_arListElements.GetAt(i).nID == atol(strIDToReference)) {
							if (!strDetailData.IsEmpty()) {
								strDetailData += ", ";
							}
							strDetailData += m_arListElements.GetAt(i).strName;
						}
					}
				}
				if(strDetailData.IsEmpty()) {
					strDetailData = "{No Selection}";
				}
			}
			}
			break;
		case eitImage:	{
				// (j.jones 2006-01-12 11:47) - we don't save any default information for images on templates
				if(m_bIsTemplateDetail)
					strDetailData = "";
				else {
					CEmrItemAdvImageState ais;
					ais.CreateFromSafeArrayVariant(m_varState);
					CString strImage = ais.m_strImagePathOverride;
					int pos = strImage.ReverseFind('\\');
					if(pos != -1)
						strDetailData = strImage.Right(strImage.GetLength() - pos - 1);
				}
			}
			break;
		case eitSlider:
			{
				if(m_varState.vt == VT_R4 || m_varState.vt == VT_R8) {
					strDetailData = AsString(m_varState);
				}
				else {
					strDetailData = "";
				}
			}
			break;
		case eitNarrative:
			{
				CString strDetailText = VarString(m_varState, "");
				//TES 7/11/2012 - PLID 51500 - Differentiate between RTF and HTML narratives
				if(IsRtfNarrative()) {
					strDetailData = ConvertTextFormat(ConvertTextFormat(strDetailText, tfNxRichText, tfRichTextForMerge), tfRichText, tfPlainText);
				}
				else {
					strDetailData = ConvertTextFormat(ConvertTextFormat(strDetailText, tfHTML, tfHTMLForMerge), tfHTML, tfPlainText);
				}
			}
			break;
		case eitTable:	{

			LoadContent();
			SyncContentAndState();

			for(int i = 0; i < m_arTableRows.GetSize(); i++) {
				for(int j = 0; j < m_arTableColumns.GetSize(); j++) {
					TableElement te;
					// (a.walling 2011-08-24 12:35) - PLID 45171 - We only need to look for existing table elements
					if(GetExistingTableElement(&m_arTableRows[i]->m_ID, m_arTableColumns[j]->nID, te)) {
						CStringArray saTemp;
						// (a.walling 2011-05-31 11:56) - PLID 42448
						CString strData = te.GetValueAsOutput(this, false, saTemp);
						ASSERT(!saTemp.GetSize()); //The function shouldn't add temp files unless it's doing html, which we told it not to.
						if(!strData.IsEmpty()) {
							strDetailData += FormatString("%s: %s: %s, ", m_arTableRows[i]->strName, m_arTableColumns[j]->strName, strData);
						}
					}
				}
			}

			strDetailData.TrimRight(", ");
			}
			break;
		default:
			break;
	}
}

//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
// (j.jones 2011-04-28 14:39) - PLID 43122 - added bIsFloated
// (z.manning 2011-11-07 17:02) - PLID 46309 - Added SpawnedItemsSeparator
// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
void CEMNDetail::AddListElement(long nID, long nDataGroupID, const CString &strName, BOOL bIsLabel, const CString &strLongForm, long nActionsType, GlassesOrderDataType godtGlassesOrderDataType, long nGlassesOrderDataID, const BOOL bIsFloated, LPCTSTR strSpawnedItemsSeparator, long nParentLabelID)
{
	for(int i = 0; i < m_arListElements.GetSize(); i++) {
		ListElement le = m_arListElements[i];
		if(le.nID == nID) {
			//Found it!  Make sure it's up-to-date.
			//TES 2/9/2007 - PLID 24671 - There's no way for the DataGroupID to change.
			// (c.haag 2007-08-01 18:12) - Same with the sort order
			ASSERT(le.nDataGroupID == nDataGroupID);
			le.strName = strName;
			le.bIsLabel = bIsLabel;
			le.strLongForm = strLongForm;
			le.nActionsType = nActionsType;
			//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
			le.godtGlassesOrderDataType = godtGlassesOrderDataType;
			le.nGlassesOrderDataID = nGlassesOrderDataID;
			// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floated,
			// which will require that we bold the text
			le.bIsFloated = bIsFloated;
			le.strSpawnedItemsSeparator = strSpawnedItemsSeparator;
			le.nParentLabelID = nParentLabelID;
			m_arListElements.SetAt(i,le);
			return;
		}
	}
	//We need to add it.
	ListElement le;
	le.nID = nID;
	le.nDataGroupID = nDataGroupID;
	le.strName = strName;
	le.bIsLabel = bIsLabel;
	le.strLongForm = strLongForm;
	le.nActionsType = nActionsType;
	//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
	le.godtGlassesOrderDataType = godtGlassesOrderDataType;
	le.nGlassesOrderDataID = nGlassesOrderDataID;
	// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floated,
	// which will require that we bold the text
	le.bIsFloated = bIsFloated;
	le.strSpawnedItemsSeparator = strSpawnedItemsSeparator;
	le.nParentLabelID = nParentLabelID;
	m_arListElements.Add(le);

	if(!m_bAddingListElements) {//If we're not in a batch add.
		SyncContentAndState();
		ReflectCurrentContent();
		ReflectCurrentState();
	}

}

void CEMNDetail::BeginAddListElements()
{
	m_bAddingListElements = TRUE;
}

void CEMNDetail::EndAddListElements()
{
	m_bAddingListElements = FALSE;
	SyncContentAndState();
	ReflectCurrentContent();
	ReflectCurrentState();
	if(m_pEmrItemAdvDlg && m_pEmrItemAdvDlg->GetSafeHwnd()) {
		CRect rcWindow;
		m_pEmrItemAdvDlg->GetWindowRect(&rcWindow);
		m_pEmrItemAdvDlg->RepositionControls(CSize(rcWindow.Width(), rcWindow.Height()), FALSE);
	}
}

// (z.manning 2010-02-19 11:17) - PLID 37427 - You must use the pointer to reference table rows now
/*TableRow CEMNDetail::GetRow(int nIndex) const
{
	return *(m_arTableRows.GetAt(nIndex));
}*/

TableRow* CEMNDetail::GetRowPtr(int nIndex) const
{
	// (c.haag 2007-08-20 12:49) - PLID 27121 - Returns a row pointer by index
	if (nIndex < 0 || nIndex >= m_arTableRows.GetSize()) {
		ASSERT(FALSE);
		return NULL;
	}
	return m_arTableRows.GetAt(nIndex);
}

// (z.manning 2010-02-18 10:54) - PLID 37427 - Row ID is now a struct
TableRow *CEMNDetail::GetRowByID(TableRowID *pRowID) const
{
	for(int i = 0; i < m_arTableRows.GetSize(); i++) {
		if(m_arTableRows[i]->m_ID == *pRowID) return m_arTableRows[i];
	}
	return NULL;
}

// (z.manning 2010-02-19 17:53) - PLID 37230 - Returns the row corresponding to the give stamp ID
TableRow *CEMNDetail::GetRowByStampID(const long nStampID)
{
	// (z.manning 2011-01-20 11:22) - PLID 42338 - Support multiple smart stamp images per table
	for(int nImageIndex = 0; nImageIndex < m_arySmartStampImageDetails.GetCount(); nImageIndex++)
	{
		CEMNDetail *pSmartStampImage = m_arySmartStampImageDetails.GetAt(nImageIndex);
		for(int nRowIndex = m_arTableRows.GetSize() - 1; nRowIndex >= 0; nRowIndex--) {
			TableRow *ptr = m_arTableRows.GetAt(nRowIndex);
			// (c.haag 2012-10-26) - PLID 53440 - Use the getter function
			//TES 11/6/2012 - PLID 53612 - If this pointer is about to get deleted, don't return it.
			if(ptr->m_ID.GetImageStampID() == nStampID && !ptr->m_bDeleteOnStateChange) {
				return ptr;
			}
		}
	}
	return NULL;
}

// (z.manning 2010-02-19 17:53) - PLID 37230 - Returns the row corresponding to the give stamp ID
TableRow *CEMNDetail::GetRowByDetailStamp(EmrDetailImageStamp *pDetailStamp)
{
	for(int nRowIndex = m_arTableRows.GetSize() - 1; nRowIndex >= 0; nRowIndex--)
	{
		TableRow *ptr = m_arTableRows.GetAt(nRowIndex);
		//TES 3/17/2010 - PLID 37530 - Need to specify the stamp index.
		if(TableRowID(pDetailStamp, m_arySmartStampImageDetails.GetStampIndexInDetailByType(pDetailStamp)) == ptr->m_ID) {
			return ptr;
		}
	}
	return NULL;
}

// (z.manning 2010-02-24 10:55) - PLID 37225 - Function to remove a row
void CEMNDetail::RemoveTableRow(TableRow *ptr)
{
	for(int nRowIndex = 0; nRowIndex < m_arTableRows.GetSize(); nRowIndex++)
	{
		TableRow *ptrCurrent = m_arTableRows.GetAt(nRowIndex);
		if(ptr == ptrCurrent)
		{
			// (z.manning 2010-02-24 10:58) - PLID 37225 - Make sure we remove all table elements for this row
			for(int nElementIndex = m_arTableElements.GetSize() - 1; nElementIndex >= 0; nElementIndex--) {
				TableElement te = m_arTableElements.GetAt(nElementIndex);
				if(te.m_pRow == ptr) {
					m_arTableElements.RemoveAt(nElementIndex);
					//TES 3/18/2011 - PLID 41108 - Also clear out our cache of info about selected dropdown items.
					delete m_arypTableSelectedDropdowns[nElementIndex];
					m_arypTableSelectedDropdowns.RemoveAt(nElementIndex);
				}
			}

			delete ptr;
			m_arTableRows.RemoveAt(nRowIndex);
			break;
		}
	}
}

int CEMNDetail::GetRowCount() const
{
	return m_arTableRows.GetSize();
}

TableColumn CEMNDetail::GetColumn(int nIndex) const
{
	return *(m_arTableColumns.GetAt(nIndex));
}

TableColumn* CEMNDetail::GetColumnPtr(int nIndex) const
{
	// (c.haag 2007-08-20 12:49) - PLID 27121 - Returns a column pointer by index
	if (nIndex < 0 || nIndex >= m_arTableColumns.GetSize()) {
		ASSERT(FALSE);
		return NULL;
	}
	return m_arTableColumns.GetAt(nIndex);
}

TableColumn* CEMNDetail::GetColumnByID(long nID) const
{
	for(int i = 0; i < m_arTableColumns.GetSize(); i++) {
		if(m_arTableColumns[i]->nID == nID) return m_arTableColumns[i];
	}
	return NULL;
}

// (z.manning 2010-02-16 14:52) - PLID 37230 - Gets a column based on subtype
TableColumn* CEMNDetail::GetColumnByListSubType(const EEmrDataElementListSubType eSubType)
{
	for(int i = 0; i < m_arTableColumns.GetSize(); i++) {
		TableColumn *ptc = m_arTableColumns.GetAt(i);
		if(ptc->nSubType == eSubType) {
			return ptc;
		}
	}
	return NULL;
}

int CEMNDetail::GetColumnCount() const
{
	return m_arTableColumns.GetSize();
}

// (c.haag 2007-08-20 09:24) - PLID 27118 - Made it const
// (z.manning 2010-02-18 10:42) - PLID 37427 - Row ID is now a struct
BOOL CEMNDetail::GetTableElement(TableRowID *pRowID, long nColumnID, OUT TableElement &te)
{
	for(int i = 0; i < m_arTableElements.GetSize(); i++) {
		TableElement& teTmp(m_arTableElements[i]);
		if(teTmp.m_pColumn->nID == nColumnID &&
			teTmp.m_pRow->m_ID == *pRowID)
		{
			te = teTmp;
			return TRUE;
		}
	}
	
	//If we get here, we didn't have one that matched already.
	TableColumn *pColumn = GetColumnByID(nColumnID);
	TableRow *pRow = GetRowByID(pRowID);
	if(!pColumn || !pRow) {
		return FALSE;
	}
	else {
		te.m_pColumn = pColumn;
		te.m_pRow = pRow;
		// (c.haag 2007-08-20 09:23) - PLID 27118 - If we didn't find one,
		// don't change m_arTableElements. Just return TRUE like the legacy
		// code did.
		//m_arTableElements.Add(te);
		return TRUE;
	}
}

// (a.walling 2011-08-24 12:35) - PLID 45171 - Unlike GetTableElement, this only gets existing elements
BOOL CEMNDetail::GetExistingTableElement(TableRowID *pRowID, long nColumnID, OUT TableElement &te)
{
	for(int i = 0; i < m_arTableElements.GetSize(); i++) {
		TableElement& teTmp(m_arTableElements[i]);
		if(teTmp.m_pColumn->nID == nColumnID &&
			teTmp.m_pRow->m_ID == *pRowID)
		{
			te = teTmp;
			return TRUE;
		}
	}
	
	return FALSE;
}

// (c.haag 2009-03-05 17:05) - PLID 33367 - Added first two optional parameters
// (z.manning 2011-05-31 14:50) - PLID 42131 - This now returns true if something actually changed.
BOOL CEMNDetail::SetTableElement(TableElement te, BOOL bSearchForExisting /* = TRUE */, BOOL bRecreateStateFromContent /* = TRUE */)
{
	//TES 7/26/2007 - PLID 25091 - This element's linked detail may have changed, so we can't trust our cached array 
	// any more.
	m_bLinkedDetailsCached = false;

	if (bSearchForExisting) {
		for(int i = 0; i < m_arTableElements.GetSize(); i++)
		{
			TableElement teExisting = m_arTableElements[i];
			if(teExisting.m_pColumn->nID == te.m_pColumn->nID && teExisting.m_pRow->m_ID == te.m_pRow->m_ID) {
				m_arTableElements.SetAt(i,te);
				//TES 3/18/2011 - PLID 41108 - We need to re-load this item's cache of selected dropdown data
				m_arypTableSelectedDropdowns.GetAt(i)->bAssociatedDataLoaded = false;
				if(te.GetValueAsString() == teExisting.GetValueAsString()) {
					return FALSE;
				}
				return TRUE;
			}
		}
	} else {
		// (c.haag 2009-03-05 17:06) - PLID 33367 - The caller must know that there is no existing element
	}

	m_arTableElements.Add(te);
	//TES 3/18/2011 - PLID 41108 - Add to our cache of selected dropdown data
	m_arypTableSelectedDropdowns.Add(new TableElementSelectedDropdownItems);

	if (bRecreateStateFromContent) {
		RecreateStateFromContent();
	} else {
		// (c.haag 2009-03-05 17:07) - PLID 33367 - The caller does not want us to do this; but that
		// also makes the caller responsible for doing it later
	}

	// (z.manning 2011-05-31 14:52) - PLID 42131 - Return true if something actually changed
	if(te.GetValueAsString().IsEmpty()) {
		return FALSE;
	}
	return TRUE;
}

int CEMNDetail::GetTableElementCount() const
{
	// (c.haag 2007-01-30 13:53) - PLID 24485 - Returns the count of m_arTableElements. This
	// should only be called in rare circumstances where we intend to do optimal traversals
	// of the table element list
	return m_arTableElements.GetSize(); 
}

BOOL CEMNDetail::GetTableElementByIndex(long nIndex, OUT TableElement &te) const
{
	// (c.haag 2007-01-30 13:53) - PLID 24485 - Returns a table element given an index. This
	// should only be called in rare circumstances where we intend to do optimal traversals
	// of the table element list
	if (nIndex < 0 || nIndex >= m_arTableElements.GetSize())
		return FALSE;
	te = m_arTableElements.GetAt(nIndex);
	return TRUE;
}

TableElement* CEMNDetail::GetTableElementPtrByIndex(long nIndex)
{
	// (c.haag 2007-08-20 11:29) - PLID 27121 - Returns an actual pointer to a table element
	if (nIndex < 0 || nIndex >= m_arTableElements.GetSize()) {
		ASSERT(FALSE); // This should never happen
		return NULL;
	}
	TableElement* pData = m_arTableElements.GetData();
	TableElement* pte = pData + nIndex;
	return pte;
}

// (z.manning 2010-02-16 15:34) - PLID 37230
TableElement* CEMNDetail::GetTableElementByRowColPtr(TableRow *ptr, TableColumn *ptc)
{
	if(ptr == NULL || ptc == NULL) {
		return NULL;
	}

	for(int i = 0; i < m_arTableElements.GetSize(); i++)
	{
		TableElement te = m_arTableElements.GetAt(i);
		if(te.m_pRow == ptr && te.m_pColumn == ptc) {
			return GetTableElementPtrByIndex(i);
		}
	}
	return NULL;
}

// (z.manning 2010-12-22 12:54) - PLID 41887
void CEMNDetail::GetTableElementsByRow(TableRow *ptr, OUT CArray<TableElement,TableElement&> &aryTableElements)
{
	for(int nElementIndex = 0; nElementIndex < m_arTableElements.GetSize(); nElementIndex++)
	{
		TableElement te = m_arTableElements.GetAt(nElementIndex);
		if(te.m_pRow == ptr) {
			aryTableElements.Add(te);
		}
	}
}

void CEMNDetail::SyncContentAndState()
{
	//TES 1/28/2008 - PLID 28673 - We need to track whether we did something that caused the content to need
	// to be re-displayed.  At the moment, the only such thing is the *s to restore multi-popups.  I intend to try
	// and find a better way of doing this, since the fact that this has never been done in this function before
	// implies that there ought to be a better place of accomplishing it.  But this will do for the AAD show.
	BOOL bContentChanged = FALSE;
	//First, go through the content variables, and make sure any state-related parameters are correct.
	switch(m_EMRInfoType) {
	case eitText:
	case eitNarrative:
	case eitSlider:
		//No state-related parameters.
		break;
	case eitSingleList:
	case eitMultiList:
		{
			//Update the selectedness.
			CDWordArray arDataIds;
			GetSelectedValues(arDataIds);
			for(int i = 0; i < m_arListElements.GetSize(); i++) {
				ListElement le = m_arListElements[i];
				bool bSelected = false;
				for(int j = 0; j < arDataIds.GetSize(); j++) if(arDataIds[j] == (DWORD)le.nID) bSelected = true;
				le.bIsSelected = bSelected;
				if(!bSelected) {
					//TES 1/28/2008 - PLID 28673 - If we've unchecked this box, and it originally had a multi-popup
					// that we've been storing to re-popup if requested, then we should no longer track that, since
					// all those details are now unspawned.  So, just set it to NULL (the EMN is responsible for freeing
					// the memory).
					DetailPopup *pDp = le.pDetailPopup;
					if(pDp) {
						le.pDetailPopup = NULL;
						//TES 1/28/2008 - PLID 28673 - We'll need to refresh the screen.
						bContentChanged = TRUE;
					}
				}
				m_arListElements.SetAt(i,le);
			}
		}
		break;
	case eitImage:
		{
		m_aisState->CreateFromSafeArrayVariant(m_varState);
			// (c.haag 2006-10-23 13:44) - PLID 23193 - We need to synchronize the image state with
			// the member variables that pertain to image content
			// (c.haag 2007-02-09 15:49) - PLID 23365 - If the state has no defined image type,
			// do NOT overwrite the existing content values
			if (itUndefined != m_aisState->m_eitImageTypeOverride) {
				m_eitBackgroundImageType = m_aisState->m_eitImageTypeOverride;
				m_strBackgroundImageFilePath = m_aisState->m_strImagePathOverride;
			}
			
			//DRT 1/24/2008 - PLID 28603 - Update the 'selected' hotspots from the state
			CDWordArray arySelected;
			ParseDelimitedStringToDWordArray(m_aisState->m_strSelectedHotSpotData, ";", arySelected);

			for(int i = 0; i < m_aryImageHotSpots.GetSize(); i++) {
				if(IsIDInArray(m_aryImageHotSpots[i]->GetID(), &arySelected)) {
					m_aryImageHotSpots[i]->SetSelected(true);
				}
				else {
					//If it's not selected in the state, we must make sure it's not selected on screen!
					m_aryImageHotSpots[i]->SetSelected(false);
				}
			}
		}
		break;
	case eitTable:
		{
			CString strState = AsString(m_varState);
			
			// (a.walling 2011-08-10 14:10) - PLID 44964 - Previously this attempted to 'create' the state by getting the table element
			// for every row/column. This has not been necessary for a long while since we stopped saving empty cells in table states.
			// The code was, in effect, pointless, yet it had exponential complexity and could lead to severe slowness for large tables.

			//Go through all the values, load the table element's value parameters.
			if(!strState.IsEmpty()) {
				// (c.haag 2007-01-30 12:42) - PLID 24485 - Some changes have been made in this
				// conditional for optimization reasons:
				//
				// - When traversing the state, we now use an index rather than shortening the string.
				// - We do not use GetTableElement, but we do use similar code in the same spirit
				// of the implementation. The difference is that we traverse maps rather than lists
				// to find the matching row and column objects.
				//

				// (c.haag 2007-01-30 12:44) - PLID 24485 - Populate our traversal maps. This
				// makes it possible for us to avoid calling GetTableElement, which is slow for
				// large tables
				//
				// (z.manning 2010-02-19 10:39) - PLID 37412 - Table rows no longer are required to 
				// be tied to EmrDataT, so I added a 2nd map for detail image stamp rows.
				//				

				CMap<long, long, TableRow*, TableRow*> mapDataRows;				
				CMap<long, long, TableRow*, TableRow*> mapDetailImageStampRows_ByID;				
				CMap<long, long, TableRow*, TableRow*> mapDetailImageStampPointerRows;
				CMap<long, long, TableColumn*, TableColumn*> mapCols;

				// (j.jones 2010-03-03 14:53) - PLID 37231 - When we load from a remembered value,
				// the table state is going to point to an EmrDetailImageStampID from the old EMN,
				// but the image stamp pointers in memory now have an ID of -1. For this reason,
				// the pointers retain nLoadedFromID so we can match up by the old ID.
				// Here, we need a second map for this.
				CMap<long, long, TableRow*, TableRow*> mapDetailImageStampRows_ByLoadedFromID;
				//if the above map is needed, that means our state needs fixed once the load is complete
				BOOL bNeedToRegenerateStateFromContent = FALSE;

				int n;

				for (n=0; n < m_arTableRows.GetSize(); n++) {
					if(m_arTableRows[n]->m_ID.nDataID != -1) {
						mapDataRows[ m_arTableRows[n]->m_ID.nDataID ] = m_arTableRows[n];
					}
					// (z.manning 2010-02-19 14:07) - PLID 37412 - Also populate the detail image stamp maps
					// (c.haag 2012-10-26) - PLID 53440 - Use the getter function
					long nEmrDetailImageStampID = m_arTableRows[n]->m_ID.GetDetailImageStampID();
					if(nEmrDetailImageStampID != -1) {
						mapDetailImageStampRows_ByID[ nEmrDetailImageStampID ] = m_arTableRows[n];
					}

					// (c.haag 2012-10-26) - PLID 53440 - Use the getter function
					EmrDetailImageStamp *pDetailImageStamp = m_arTableRows[n]->m_ID.GetDetailImageStampObject();
					if(pDetailImageStamp != NULL) {
						mapDetailImageStampPointerRows[ (long)pDetailImageStamp ] = m_arTableRows[n];

						// (j.jones 2010-03-03 14:53) - PLID 37231 - populate the LoadedFromID map as well
						mapDetailImageStampRows_ByLoadedFromID[ pDetailImageStamp->nLoadedFromID ] = m_arTableRows[n];
					}
				}
				for (n=0; n < m_arTableColumns.GetSize(); n++) {
					mapCols[ m_arTableColumns[n]->nID ] = m_arTableColumns[n];
				}

				// (c.haag 2007-05-31 16:52) - PLID 26202 - Build a map that takes a row-column combination
				// as the key, and table element index as the value
				// (c.haag 2007-07-12 09:28) - PLID 26579 - The row and column pointers in m_arTableElements
				// may be out of date by this point. Since we're rebuilding m_arTableElements with new state
				// information anyway, just clear it out.
				m_arTableElements.RemoveAll();
				//TES 3/18/2011 - PLID 41108 - Also clear out our cache of info about selected dropdown items.
				for(int i = 0; i < m_arypTableSelectedDropdowns.GetSize(); i++) {
					delete m_arypTableSelectedDropdowns[i];
				}
				m_arypTableSelectedDropdowns.RemoveAll();

				/*CMap<__int64, __int64, int, int> mapTable;
				const int nTableElements = m_arTableElements.GetSize();
				for(n = 0; n < nTableElements; n++) {
					const TableElement& teTmp =  m_arTableElements[n];
					__int64 nKey = (((__int64)teTmp.m_pRow->nID) << 32) + (__int64)teTmp.m_pColumn->nID;
					mapTable[nKey] = n;
				}*/


				// (c.haag 2007-01-30 12:57) - PLID 24485 - Traverse the state using
				// indexes rather than shortening the string as we go along
				// (c.haag 2007-08-18 10:54) - PLID 27112 - Use an iterator to do the work (which
				// internally uses indexes)
				CEmrTableStateIterator etsi(strState);
				long X,Y,nEmrDetailImageStampID,nEmrDetailImageStampPointer,nStampID;
				CString strData;
				// (z.manning 2010-02-18 09:34) - PLID 37427 - Added EmrDetailImageStampID
				while (etsi.ReadNextElement(X,Y,strData,nEmrDetailImageStampID,nEmrDetailImageStampPointer,nStampID)) {
					TableElement te;

					// (c.haag 2007-01-30 13:04) - PLID 24485 - We used to call GetTableElement
					// here. The following code behaves like GetTableElement, but will traverse 
					// maps rather than arrays to find the row and column objects.
					///////////// Begin optimized "GetTableElement" implementation /////////

					BOOL bFound = FALSE;
					int nTableElement = -1;

					// (c.haag 2007-05-31 16:52) - PLID 26202 - Instead of running a loop for all table
					// elements, just use the table element map to see if the element exists.
					// (c.haag 2007-07-12 09:29) - PLID 26579 - There is no longer existing data in
					// m_arTableElements before this loop runs. So, bFound will always be false.
					/*if (nTableElements > 0) {
						__int64 nKey = (((__int64)X) << 32) + (__int64)Y;
						if (mapTable.Lookup(nKey, nTableElement)) {
							te = m_arTableElements[nTableElement];
							bFound = TRUE;
						}
					}*/
					//if (!bFound) {
						//If we get here, we didn't have one that matched already.
						TableRow* pRow = NULL;
						TableColumn* pColumn = NULL;
						mapDataRows.Lookup(X, pRow);
						if(pRow == NULL) {
							// (z.manning 2010-02-19 14:08) - PLID 37412 - Also check the detail image stamp map.
							mapDetailImageStampRows_ByID.Lookup(nEmrDetailImageStampID, pRow);
						}
						if(pRow == NULL) {
							// (z.manning 2010-02-19 14:08) - PLID 37412 - Also check the detail image stamp pointer map.
							mapDetailImageStampPointerRows.Lookup(nEmrDetailImageStampPointer, pRow);
						}
						if(pRow == NULL) {
							// (j.jones 2010-03-03 14:53) - PLID 37231 - last chance, try to load from the LoadedFromID map
							mapDetailImageStampRows_ByLoadedFromID.Lookup(nEmrDetailImageStampID, pRow);
							if(pRow) {
								//found it, which means the state still is referencing an old ID,
								//and will need to be corrected
								bNeedToRegenerateStateFromContent = TRUE;
							}
						}
						if (pRow != NULL && mapCols.Lookup(Y, pColumn)) {
							te.m_pColumn = pColumn;
							te.m_pRow = pRow;
							nTableElement = m_arTableElements.GetSize();
							m_arTableElements.Add(te);
							//TES 3/18/2011 - PLID 41108 - Also add to our cache of info about selected dropdown items.
							m_arypTableSelectedDropdowns.Add(new TableElementSelectedDropdownItems);
							bFound = TRUE;
						}
					//}
					///////////// End optimized "GetTableElement" implementation /////////

					if (bFound) { // if(GetTableElement(X,Y, te)) {
						if(m_bOwnTopic || !m_pParentTopic) {
							//The parent won't have other details, so don't pass it in.
							te.LoadValueFromString(strData, NULL);
						}
						else {
							te.LoadValueFromString(strData, m_pParentTopic->GetParentEMN());
						}

						// (c.haag 2007-01-30 13:12) - PLID 24485 - Calling SetTableElement
						// is slow because it does a traversal to find the index of 
						// m_arTableElements that we want to store the value in. We already
						// know what the index is.

						//SetTableElement(te);
						m_arTableElements.SetAt(nTableElement, te);
						//TES 3/18/2011 - PLID 41108 - Clear out our cache of info about selected dropdown items.
						m_arypTableSelectedDropdowns.GetAt(nTableElement)->bAssociatedDataLoaded = false;
					}
				}

				// (j.jones 2010-03-03 15:11) - PLID 37231 - if we need to recreate the state, do it now
				if(bNeedToRegenerateStateFromContent) {
					RecreateStateFromContent();
				}
			}
		}
		break;
	default:
		ASSERT(FALSE);
		break;
	}

	//Now, correct anything in the state that may be invalid.
	switch(m_EMRInfoType) {
	case eitText:
	case eitNarrative:
	case eitImage:
		//No state-related parameters.
		break;
	case eitSlider:
		if(m_varState.vt == VT_R4) {
			if(VarDouble(m_varState) > m_dSliderMax)
				SetState(m_dSliderMax);
			else if(VarDouble(m_varState) < m_dSliderMin)
				SetState(m_dSliderMin);
		}
		break;

	case eitSingleList:
	case eitMultiList:
		{
			//Make sure all our selected items exist.
			CDWordArray arDataIds;
			GetSelectedValues(arDataIds);
			bool bSomeNotFound = false;
			// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
			int i = 0;
			for(i = 0; i < arDataIds.GetSize(); i++) {
				bool bMatched = false;
				for(int j =0; j < m_arListElements.GetSize() && !bMatched; j++) {
					if(m_arListElements[j].nID == (long)arDataIds[i])
						bMatched = true;
				}
				if(!bMatched) {
					bSomeNotFound = true;
					arDataIds.RemoveAt(i);
					i--;
				}
			}
			if(bSomeNotFound) {
				CString strNewState;
				for(i = 0; i < arDataIds.GetSize(); i++)
					strNewState += FormatString("%li; ",arDataIds[i]);
				SetState(_bstr_t(strNewState));
			}

		}
		break;
	case eitTable:

		//DRT 10/8/2007 - PLID 27657 - If the state is VT_EMPTY and we're on a current meds/allergies
		//	detail, this function should never be called.  Program execution must wait until PostLoadDetails()
		//	has been called, because these 2 details are "special" and require loading from patient data.
		if(m_varState.vt == VT_EMPTY && (IsCurrentMedicationsTable() || IsAllergiesTable())) {
			ASSERT(FALSE);
		}

		CString strState = AsString(m_varState);
		if(strState.IsEmpty()) {
			//Make a state that's blank for all our rows and columns.
			for(int i = 0; i < m_arTableRows.GetSize(); i++) {
				for(int j = 0; j < m_arTableColumns.GetSize(); j++) {
					// (c.haag 2007-08-18 11:00) - PLID 27111 - Use the utility function to build
					// the empty table state string for uniformity
					// (c.haag 2012-10-26) - PLID 53440 - Use the getter functions
					AppendTableStateWithFormattedElement(strState, m_arTableRows[i]->m_ID.nDataID, m_arTableColumns[j]->nID, "", m_arTableRows[i]->m_ID.GetDetailImageStampID(), (long)m_arTableRows[i]->m_ID.GetDetailImageStampObject(), m_arTableRows[i]->m_ID.GetImageStampID());
				}
			}
			SetState(_bstr_t(strState));
		}
		break;
	}

	if(bContentChanged) {
		//TES 1/28/2008 - PLID 28673 - Refresh the screen.  As per my comment at the top of this function, I intend
		// to find a better place in code to accomplish this, but I haven't found one so far, and I need to get this
		// working for the AAD show.
		ReflectCurrentContent();
		if(m_pEmrItemAdvDlg) {
			CRect rc;
			m_pEmrItemAdvDlg->GetWindowRect(&rc);
			CSize sz(rc.Width(), rc.Height());
			m_pEmrItemAdvDlg->RepositionControls(sz, FALSE);
		}
	}
}

CString CEMNDetail::GetUniqueIdentifierAgainstEMN() const
{
	// (c.haag 2007-06-13 17:54) - PLID 26296 - If this function is changed, please ensure that
	// choosing linked details in table popups still works correctly (refer to the PL item notes)
	CString strID;
	//If we have a valid detail ID, use that.
	if(!m_bIsTemplateDetail && m_nEMRDetailID != -1) {
		strID.Format("I:%li", m_nEMRDetailID);//"I" is for "ID"
	}
	else if(m_bIsTemplateDetail && m_nEMRTemplateDetailID != -1) {
		strID.Format("T:%li", m_nEMRTemplateDetailID);//"I" is for "TemplateDetailID"
	}
	else {
		strID.Format("P:%li", this);//"P" is for "pointer"
	}
	return strID;
}

void CEMNDetail::RecreateStateFromContent()
{
	switch(m_EMRInfoType) {
	case eitText:
	case eitNarrative:
	case eitImage:
	case eitSlider:
		//This only is needed for lists and sliders.
		break;
	case eitSingleList:
	case eitMultiList:
		{
			CString strNewState;
			for(int i = 0; i < m_arListElements.GetSize(); i++) {
				if(m_arListElements[i].bIsSelected) strNewState += FormatString("%li; ",m_arListElements[i].nID);
			}
			SetState(_bstr_t(strNewState));
		}
		break;
	case eitTable:
		{
			CString strNewState;

			// (c.haag 2007-08-20 10:02) - PLID 27118 - m_varState no longer contains non-empty elements,
			// so just use m_arTableElements to populate the state
			const int nTableElements = m_arTableElements.GetSize();
			for(int i = 0; i < nTableElements; i++) {
				TableElement te = m_arTableElements[i];
				// (z.manning 2010-03-03 17:36) - PLID 37225 - Ignore rows flagged for deletion
				// (c.haag 2012-10-26) - PLID 53440 - Use the getter functions
				if(!te.m_pRow->m_bDeleteOnStateChange) {
					AppendTableStateWithUnformattedElement(strNewState, te.m_pRow->m_ID.nDataID, te.m_pColumn->nID
						, te.GetValueAsString(), te.m_pRow->m_ID.GetDetailImageStampID(), (long)te.m_pRow->m_ID.GetDetailImageStampObject()
						, te.m_pRow->m_ID.GetImageStampID());
				}
			}
			SetState(_bstr_t(strNewState));

			/*
			// (c.haag 2007-01-30 13:35) - PLID 24485 - This function has been optimized. Prior to the
			// optimization, the logic was
			//
			// do for all rows
			//		do for all columns
			//			do for all table elements (::GetTableElement())
			//
			// Now it's
			//
			// do for all table elements
			// do for all rows
			//		do for all columns
			//
			// We accomplish this by taking GetTableElement out of the innermost loop, and using a map
			// to find an index to m_arTableElements instead
			//
			CMap<__int64, __int64, int, int> mapTable;
			const int nTableElements = m_arTableElements.GetSize();
			int i,j;
			for(i = 0; i < nTableElements; i++) {
				const TableElement& teTmp = m_arTableElements[i];
				__int64 nKey = (((__int64)teTmp.m_pRow->nID) << 32) + (__int64)teTmp.m_pColumn->nID;
				mapTable[nKey] = i;
			}

			CString strNewState;
			for(i = 0; i < m_arTableRows.GetSize(); i++) {
				for(j = 0; j < m_arTableColumns.GetSize(); j++) {
					long nRowID = m_arTableRows[i]->nID;
					long nColumnID = m_arTableColumns[j]->nID;

					TableElement te;

					// (c.haag 2007-01-30 13:40) - PLID 24485 - Do not use GetTableElement; instead use the map
					// to find the correct element
					//GetTableElement(nRowID, nColumnID, te);
					__int64 nKey = (((__int64)nRowID) << 32) + (__int64)nColumnID;
					int nTableIndex;
					if (mapTable.Lookup(nKey, nTableIndex)) {
						te = m_arTableElements[nTableIndex];
					} else {
						// (c.haag 2007-02-21 16:54) - PLID 24855 - If this cell has no element,
						// populate te with only a row and column pointer. For consistency with
						// ::GetTableElement, nothing else is explicitly filled in here. Unlike
						// ::GetTableElement, however, we don't add an empty element to the
						// detail. This does not cause any problems.
						te.m_pRow = m_arTableRows[i];
						te.m_pColumn = m_arTableColumns[j];
					}

					// (c.haag 2007-08-18 08:58) - PLID 27111 - Append the state with the next element
					AppendTableStateWithUnformattedElement(strNewState, nRowID, nColumnID, te.GetValueAsString());
				}
			}
			SetState(_bstr_t(strNewState));*/
		}
		break;
	}
}

void CEMNDetail::UpdateTable(CEMNDetail *pDetail, BOOL bRemovingDetail)
{
	if(m_pEmrItemAdvDlg  && ::IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
		ASSERT(m_EMRInfoType == eitTable);
		((CEmrItemAdvTableDlg*)m_pEmrItemAdvDlg)->UpdateLinkedItemList();
	}
	// (c.haag 2007-08-13 11:29) - PLID 27049 - Skip this code if pDetail is NULL
	if (NULL != pDetail) {
		// (a.walling 2007-10-17 11:49) - PLID 25549 - If an item is removed, there is no statechange
		// message for this table. Rather than try to find the table(s) and update it in the deleted
		// statechange message for the item that is being removed, we can just handle it here.
		// (c.haag 2008-06-03 12:18) - PLID 27778 - The old traversal method is slow. All we need to do
		// is a simple set of iterations through m_arTableElements. Furthermore, we want to track whether
		// we reset any table element linked detail values.
		BOOL bNeedToUpdateTablePreview = FALSE;
		BOOL bHasClearedLinkedDetail = FALSE;
		for(int i = 0; i < m_arTableElements.GetSize(); i++) {
			TableElement te = m_arTableElements[i];
			if (NULL != te.m_pColumn && LIST_TYPE_LINKED == te.m_pColumn->nType && te.m_pLinkedDetail == pDetail) {
				if(bRemovingDetail) {
					te.m_pLinkedDetail = NULL;
					te.m_strLinkedSentenceHtml = te.m_strLinkedSentenceNonHtml = "";
					
					bNeedToUpdateTablePreview = TRUE;

					// (c.haag 2008-06-03 12:20) - PLID 27778 - Remember that we set at least one m_pLinkedDetail to null
					bHasClearedLinkedDetail = TRUE;
				}
				else {
					te.ReloadSentence();
				}
				SetTableElement(te);
				//TES 3/18/2011 - PLID 41108 - SetTableElement() will update our cache of associated dropdowns
				
				m_strSentenceHTML = "";
				m_strSentenceNonHTML = "";
			}
		}
		// (c.haag 2008-06-03 12:20) - PLID 27778 - If one or more elements "lost" their linked detail,
		// we need to rebuild the state. Otherwise, the linked detail will effectively be left "dangling"
		// in the state, and will cause assertions and possibly unpredictable things to happen down the road.
		if (bHasClearedLinkedDetail) {
			RecreateStateFromContent();
		}
		/*for(int i = 0; i < m_arTableColumns.GetSize(); i++) {
			TableColumn *pTc = m_arTableColumns[i];
			if(pTc->nType == LIST_TYPE_LINKED) {
				for(int j = 0; j < m_arTableRows.GetSize(); j++) {
					TableElement te;
					GetTableElement(m_arTableRows[j]->nID, pTc->nID, te);
					if(te.m_pLinkedDetail == pDetail) {
						if(bRemovingDetail) {
							te.m_pLinkedDetail = NULL;
							te.m_strLinkedSentenceHtml = te.m_strLinkedSentenceNonHtml = "";
							
							bNeedToUpdateTablePreview = TRUE;
						}
						else {
							te.ReloadSentence();
						}
						SetTableElement(te);
						
						m_strSentenceHTML = "";
						m_strSentenceNonHTML = "";

					}
				}
			}
		}*/

		// (a.walling 2007-10-17 11:53) - PLID 25549 - If a linked detail column was removed, go ahead
		// and update the preview for this table (assuming we have an interfacewnd)
		if (bNeedToUpdateTablePreview && (m_pParentTopic != NULL) && (m_pParentTopic->IsTemplate() == FALSE) && (m_pParentTopic->GetParentEMN() != NULL) ) {
			CEmrTreeWnd* pTreeWnd = m_pParentTopic->GetParentEMN()->GetInterface();

			pTreeWnd->UpdateDetailPreview(this);
		}
	}
}

// (z.manning 2011-03-02 15:19) - PLID 42335
void CEMNDetail::UpdateTableStateWithNewDetailStampIDByStampID(const long nStampIDToLookFor, const long nNewDetailStampID)
{
	if(m_EMRInfoType != eitTable || m_varState.vt != VT_BSTR) {
		return;
	}

	CString strState = VarString(m_varState);
	CEmrTableStateIterator etsi(strState);
	long nRowID, nColID, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID;
	CString strData, strNewState;
	while(etsi.ReadNextElement(nRowID, nColID, strData, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID))
	{
		if(nStampID == nStampIDToLookFor) {
			nEmrDetailImageStampID = nNewDetailStampID;
		}
		
		// (z.manning 2011-06-08 17:46) - PLID 44029 - ReadNextElement returns the unformatted data so we need to call
		// AppendTableStateWithUnformattedElement here instead of AppendTableStateWithFormattedElement.
		::AppendTableStateWithUnformattedElement(strNewState, nRowID, nColID, strData, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID);
	}

	SetState(_bstr_t(strNewState));
}

// (z.manning 2009-08-04) - PLID 31910 - Will iterate through the table state and remove any elements
// where either the row or column ID matches nDataID
void CEMNDetail::ClearTableStateByDataID(const long nDataID)
{
	// (z.manning 2009-08-04) - PLID 31910 - Don't bother if this isn't a table with a state.
	if(m_EMRInfoType != eitTable || m_varState.vt != VT_BSTR) {
		return;
	}

	// (z.manning 2009-08-04) - PLID 31910 - Iterate through the table state and create a new
	// state that does not include any table elements whose row or column matches nDataID.
	CString strState = VarString(m_varState);
	CEmrTableStateIterator etsi(strState);
	long nRowID, nColID, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID;
	CString strData, strNewState;
	// (z.manning 2010-02-18 09:37) - LPID 37427 - Added EmrDetailImageStampID
	while(etsi.ReadNextElement(nRowID, nColID, strData, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID))
	{
		if(nRowID == nDataID || nColID == nDataID) {
			// (z.manning 2009-08-04) - PLID 31910 - The given data ID matches this table element, so
			// do not include it in the new state.
		}
		else {
			// (z.manning 2009-08-04) - PLID 31910 - This table element is fine, so make sure we keep it.
			// Also, use the function we have to do this.
			//strNewState += AsString(nRowID) + ';' + AsString(nColID) + ';' + strData + ';';
			// (z.manning 2011-06-08 17:46) - PLID 44029 - ReadNextElement returns the unformatted data so we need to call
			// AppendTableStateWithUnformattedElement here instead of AppendTableStateWithFormattedElement.
			::AppendTableStateWithUnformattedElement(strNewState, nRowID, nColID, strData, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID);
		}
	}

	SetState(_bstr_t(strNewState));
}

// (c.haag 2008-12-02 09:51) - PLID 32271 - Given an array of details, this function will take all the
// corresponding EMR list items that do not have their content loaded, and store all of their options
// in mapEmrDataT. I chose the uncanny function name to minimize the chance of a developer thinking
// this is used anywhere besides GetLinkedDetails. If this seems confusing or unclear, please see how the map
// is used in GetLinkedDetails.
// (c.haag 2008-12-04 16:07) - PLID 31693 - This used to take in a map of CStringArrays. Now it takes in
// a map of CEmrDataNameAndIDArys because we also need EMR Data ID's.
void CEMNDetail::GetEmrListOptionsForLinkedDetails(_Connection *lpCon, CEMN* pEMN, CArray<CEMNDetail*,CEMNDetail*>* paEMNDetails,
												   const CStringArray& saMergeFields,
												   CMap<long,long,CEmrDataNameAndIDAry*,CEmrDataNameAndIDAry*>& mapEmrDataT)
{
	const long nTotalDetailCount = (NULL != paEMNDetails) ? paEMNDetails->GetSize() : pEMN->GetTotalDetailCount();
	// The array of Emr Info ID's whose data elements will be queried
	CArray<long,long> anEmrInfoIDs;
	for (long n=0; n < nTotalDetailCount; n++) {
		CEMNDetail *pTmp = (NULL != paEMNDetails) ? paEMNDetails->GetAt(n) : pEMN->GetDetail(n);
		long nEMRInfoID = pTmp->m_nEMRInfoID;
		// (c.haag 2008-11-28 15:33) - PLID 32271 - Do not filter on spawned group ID
		if (pTmp->GetNeedContentReload() && (eitSingleList == pTmp->m_EMRInfoType || eitMultiList == pTmp->m_EMRInfoType)) {
			// If we get here, the detail corresponds to a list item, and its latest content is not in memory. So, it seems
			// like we would have to query data for it. The catch is that we could have hundreds of these. To narrow down the
			// query ever further, we need to see if this detail's label is contained in a merge field. (On a side note, I
			// think it should only matter if the detail label is at the beginning of the merge field; meaning the test
			// should be "0 == strLCaseField.Find(...)". However, I want to keep the test consistent with the legacy logic that
			// Adam last changed for PL 29562)
			CString strLCaseText = pTmp->GetLabelText();
			strLCaseText.MakeLower();
			// Go through all the merge fields seeing if this detail's label is contained within the field
			for(int iField = 0; iField < saMergeFields.GetSize(); iField++) {
				CString strLCaseField = saMergeFields[iField];
				strLCaseField.MakeLower();
				if (-1 != strLCaseField.Find( strLCaseText )) {
					// If we get here, the detail label exists somewhere in the field. So, add it to our list of EMR Info ID's
					// if it doesn't already exist.
					int idx;
					for (idx=0; idx < anEmrInfoIDs.GetSize(); idx++) {
						if (anEmrInfoIDs[idx] == nEMRInfoID) {
							break;
						}
					}
					if (idx == anEmrInfoIDs.GetSize()) {
						anEmrInfoIDs.Add(nEMRInfoID);
					}
					break;
				} else {
					// The detail label doesn't exist in this field. Keep looking through other fields.
				}
			} // for(int iField = 0; iField < saMergeFields.GetSize(); iField++) {
		} // if (pTmp->GetNeedContentReload() && (eitSingleList == pTmp->m_EMRInfoType || eitMultiList == pTmp->m_EMRInfoType)) {
	} // for (long n=0; n < nTotalDetailCount; n++) {

	// Now anEmrInfoIDs is filled with all the EMR Info ID's that are for list items and may exist in merge fields. Now pull
	// all the options for those items (being mindful of things like labels and inactive flags)
	if (anEmrInfoIDs.GetSize() > 0) {
		_ConnectionPtr pCon;
		if(lpCon) pCon = lpCon;
		else pCon = GetRemoteData();
		// (c.haag 2008-12-04 16:07) - PLID 31693 - Get EmrDataT.ID as well
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr prs = CreateParamRecordset(pCon,
			"SELECT EmrInfoID, ID, Data FROM EmrDataT "
			"WHERE EmrInfoID IN ({INTARRAY}) "
			"AND (EMRDataT.Inactive Is Null OR EMRDataT.Inactive = 0) "
			"AND EmrDataT.IsLabel = 0 ", anEmrInfoIDs);
		FieldsPtr f = prs->Fields;
		while (!prs->eof) {
			long nEmrInfoID = AdoFldLong(f, "EmrInfoID");
			long nDataID = AdoFldLong(f, "ID");
			CString strData = AdoFldString(f, "Data");
			CEmrDataNameAndIDAry* pastrData;
			if (!mapEmrDataT.Lookup(nEmrInfoID, pastrData)) {
				mapEmrDataT.SetAt(nEmrInfoID, (pastrData = new CEmrDataNameAndIDAry));
			}
			EmrDataNameAndID ednai;
			ednai.nDataID = nDataID;
			ednai.strData = strData;
			pastrData->Add(ednai);
			prs->MoveNext();
		}
	} // if (anEmrInfoIDs.GetSize() > 0) {
}


// (c.haag 2007-02-28 13:15) - PLID 24989 - We now support getting linked narrative details given a list of EMN details.
// Once place that uses this is the initial load of a new patient EMR being made from a template.
// (c.haag 2007-08-09 16:47) - PLID 27038 - We now require an EMN object.
// (c.haag 2007-08-14 17:44) - PLID 27073 - Optional parameter for connection objects
// (c.haag 2007-10-23 09:44) - PLID 27833 - Narrative fields are related to details by their label text.
// It's possible for more than one detail to have the same label text on an EMN. If bIncludeNarrativeDuplicates
// is FALSE, then this function will find the first qualifying detail (this is legacy behavior). If 
// bIncludeNarrativeDuplicates is TRUE, then we will pull all qualifying details. This is a workaround 
// for PLID 27833, and may be revisited in PLID 27840.
// (c.haag 2008-12-04 10:48) - PLID 31693 - This function now returns extended detail information.
// The original prototype now exists as a wrapper that only returns the detail pointers.
// (c.haag 2008-12-04 12:48) - PLID 31693 - Added bIncludeDetailDuplicates
void CEMNDetail::GetLinkedDetails(CArray<LinkedDetailStruct,LinkedDetailStruct&> &arDetails, CEMN* pEMN,
								  CArray<CEMNDetail*,CEMNDetail*>* paEMNDetails /*= NULL */,
								  _Connection *lpCon /*= NULL */,
								  BOOL bIncludeNarrativeDuplicates /*= FALSE */,
								  BOOL bIncludeDetailDuplicates /*= FALSE */)
{
	// (c.haag 2007-08-09 16:49) - PLID 27038 - If paEMNDetails is not NULL, pEMN will be ignored.
	// Otherwise, pEMN may be used.
	ASSERT( (NULL != pEMN && NULL == paEMNDetails) || (NULL == pEMN && NULL != paEMNDetails) );

	//TES 7/26/2007 - PLID 25091 - If we have cached what our linked details are, just return that.
	if(m_bLinkedDetailsCached) {
		int i;
		arDetails.RemoveAll();
		for(i = 0; i < m_arLinkedDetails.GetSize(); i++) {
			// (c.haag 2008-12-02 16:30) - PLID 32296 - Filter out duplicates if the caller wants them filtered out
			if (!m_arLinkedDetails[i].bIsNarrativeDuplicate || bIncludeNarrativeDuplicates) {
				arDetails.Add(m_arLinkedDetails[i]);
			} else {
				// If we get here, the linked detail is a duplicate, and the caller doesn't want duplicates
			}
		}

		// (c.haag 2008-11-18 11:41) - PLID 32061 - Ensure that every detail in the array exists once and only once by pointer.
		// (c.haag 2008-12-04 12:48) - PLID 31693 - Now an optional feature
		if (!bIncludeDetailDuplicates) {
			CArray<LinkedDetailStruct,LinkedDetailStruct&> aTmp;
			int iDetail, iTmp;
			for (iDetail=0; iDetail < arDetails.GetSize(); iDetail++) {
				CEMNDetail* p = arDetails[iDetail].pDetail;
				BOOL bFound = FALSE;
				for (iTmp=0; iTmp < aTmp.GetSize() && !bFound; iTmp++) {
					if (aTmp[iTmp].pDetail == p) {
						bFound = TRUE;
					}
				}
				if (!bFound) {
					aTmp.Add(arDetails[iDetail]);
				}
			}
			arDetails.Copy(aTmp);
		} // if (!bIncludeDetailDuplicates) {

		return;
	}

	try {
		// (c.haag 2008-12-04 12:43) - PLID 31693 - Clear the linked detail cache. Instead of filling
		// arDetails and copying them to m_arLinkedDetails, we simply fill m_arLinkedDetails as we go along.
		m_arLinkedDetails.RemoveAll();

		switch(m_EMRInfoType) {
		case eitTable:
			{
				if (NULL != paEMNDetails) {
					// (c.haag 2007-02-28 13:18) - PLID 24989 - This is unsupported
					ThrowNxException("Could not get linked details for table item!");
				}
				for(int i = 0; i < m_arTableElements.GetSize(); i++) {
					if(m_arTableElements[i].m_pLinkedDetail) {
						//Don't add duplicates.
						bool bMatched = false;
						for(int j = 0; j < m_arLinkedDetails.GetSize(); j++) {
							if(m_arTableElements[i].m_pLinkedDetail == m_arLinkedDetails[j].pDetail) {
								bMatched = true;
							}
						}
						if(!bMatched) {
							LinkedDetailStruct lds;
							lds.pDetail = m_arTableElements[i].m_pLinkedDetail;
							// (c.haag 2008-12-04 11:08) - PLID 31693 - These other fields don't apply here.
							// Just set them to zeros/invalids/nulls.
							lds.nStartPos = 0;
							lds.bIsNarrativeDuplicate = FALSE;
							lds.nDataID = -1;
							m_arLinkedDetails.Add(lds);
						}
					}
				}
			}
			break;
		case eitNarrative:
			{
				//TES 2/21/2007 - PLID 24864 - We used to pull the merge fields that were in use from our AdvDlg, but that was
				// unnecessary overhead, since we can parse all the information out of our state.
				// (a.walling 2008-02-07 12:41) - PLID 28856 - Need to handle the case where the state may be NULL (only when changing types)
				// (a.walling 2008-02-21 17:53) - PLID 28856 - Well, this was taken care of for NULLs, but apparently we are still having issues.
				// I'll use _ASSERTE so testers can catch it early and provide better debug info before an exception is thrown.
				_ASSERTE(m_varState.vt == VT_BSTR || m_varState.vt == VT_NULL); // if you get here, check what the state is, and what the m_pPendingEMRInfoType and m_EMRInfoType are.

				CString strNxRichText = VarString(m_varState, "");
				// (a.wetta 2007-03-06 13:16) - PLID 25008 - If this detail has a blank state, then there are no merge fields so we're done.
				if (strNxRichText == "")
					break;
				// (c.haag 2012-04-02) - PLID 49350 - Special handling for non-RTF narratives
				else if (!IsRtfNarrative()) 
				{
					if (NULL == m_pParentTopic || NULL == GetParentEMN() || GetParentEMN()->IsLoading())
						break;

					//TES 6/6/2012 - PLID 49350 - Call GetNarrativeFieldArray(), it works for both HTML and RTF
					CArray<NarrativeField> arNarrativeFields;
					GetNarrativeFieldArray(strNxRichText, arNarrativeFields);
					for(int i = 0; i < arNarrativeFields.GetSize(); i++)
					{
						NarrativeField nf = arNarrativeFields[i];
						CString strDummy;
						bool bDummy;
						// (z.manning 2013-05-21 17:01) - PLID 56810 - Skip over built in fields
						if(nf.nDetailID != -1 && nf.strField != NFIELD_SPAWNING_ITEM && nf.strField != NFIELD_EMN_SPAWNING_TEXT
							&& nf.strField != NFIELD_ITEM_SPAWNING_TEXT && nf.strField != NFIELD_ITEM_SPAWNING_TEXT_FURTHEST
							&& !pEMN->GetGenericMergeFieldValue(nf.strField, strDummy, bDummy))
						{
							LinkedDetailStruct s;
							s.nStartPos = nf.nStart;
							s.nDataID = -1;
							s.pDetail = GetParentEMN()->GetDetailByID(nf.nDetailID);
							if (NULL != s.pDetail) {
								m_arLinkedDetails.Add(s);
							}
						}
					}
					
					break;
				}

				//Separate into the header and the actual rtf.
				int nHeaderBegin = strNxRichText.Find(NXRTF_HEADER_BEGIN);
				ASSERT(nHeaderBegin == 0);
				int nHeaderEnd = strNxRichText.Find(NXRTF_HEADER_END);
				ASSERT(nHeaderEnd != -1);
				nHeaderBegin += CString(NXRTF_HEADER_BEGIN).GetLength();
				CString strHeader = strNxRichText.Mid(nHeaderBegin, nHeaderEnd - nHeaderBegin);
				nHeaderEnd += CString(NXRTF_HEADER_END).GetLength();
				
				CStringArray saMergeFields;
				// (c.haag 2008-11-25 09:34) - PLID 31693 - We now track the position of each element in saMergeFields
				CDWordArray anMergeFieldStartPos;
				// (c.haag 2008-11-26 17:27) - PLID 32268 - During our search, we may come across fields that exist as
				// list options. We used to query EmrDataT to get those options on a need-to-query basis, which could
				// result in ten or more queries run per call to GetLinkedDetails. Now we guarantee that only one query
				// is run by getting everything we could possibly need in one trip to the SQL server, and storing the
				// results in this map.
				// (c.haag 2008-12-04 16:14) - PLID 31693 - This map now holds both EmrDataT.Data and EmrDataT.ID
				CMap<long,long,CEmrDataNameAndIDAry*,CEmrDataNameAndIDAry*> mapEmrDataT; // Key = EmrInfoID, Value = Array of EmrDataT data's and ID's (one element per list option)
				BOOL bmapEmrDataTPopulated = FALSE;

				int nMergeField = -1;
				bool bFoundNext = true;
				while(bFoundNext) {
					nMergeField = strHeader.Find("<MERGEFIELD=",nMergeField+1);
					if(nMergeField == -1) bFoundNext = false;
					else {
						int nMergeFieldBegin = nMergeField + 12;
						//The name of the field is in the first argument of this semicolon-delimited list.
						int nMergeFieldEnd = FindNextNonEscaped(strHeader, '>', nMergeField);
						CString strMergeField = strHeader.Mid(nMergeFieldBegin,nMergeFieldEnd-nMergeFieldBegin);
						int nFirstSemicolon = FindNextNonEscaped(strMergeField, ';', 0);
						saMergeFields.Add(DeEscape(strMergeField.Left(nFirstSemicolon)));
						// (c.haag 2008-11-25 10:33) - PLID 31693 - Now pull the start position between the second
						// and third semicolons.
						int nSecondSemicolon = FindNextNonEscaped(strMergeField, ';', nFirstSemicolon+1);
						int nThirdSemicolon = FindNextNonEscaped(strMergeField, ';', nSecondSemicolon+1);
						int nNarrativeFieldPos = atol(strMergeField.Mid(nSecondSemicolon+1, nThirdSemicolon-nSecondSemicolon-1));
						anMergeFieldStartPos.Add(nNarrativeFieldPos);
					}
				}

				//OK, we have our list of merge fields, go through each of them and try to establish what detail they're
				// linked to.
				for(int i = 0; i < saMergeFields.GetSize(); i++) {
					CString strField = saMergeFields[i];
					const DWORD nMergeFieldStartPos = anMergeFieldStartPos[i];
					bool bIsDetail = true;
					//If this is one of the "standard" narrative fields that we add in, then it's not a detail, so we don't
					// need to bother trying to link it.
					if(strField == "Name" || strField == "Name (First)" || strField == "Name (Middle)" || 
						strField == "Name (Last)" || strField == "Male/Female" || strField == "male/female" ||
						strField == "He/She" || strField == "he/she" || strField == "Him/Her" || 
						strField == "him/her" || strField == "His/Her" || strField == "his/her" || strField == "Age" ||
						strField == "Date" || strField == "Provider") {
						bIsDetail = false;
					}
					if(bIsDetail) {
						//OK, we've got a valid field.  Find the detail this is associated with.
						//CEMNDetail *pDetail = NULL;
						// (c.haag 2008-12-04 16:15) - PLID 31693 - Now the qualifying arrays store LinkedDetailStruct objects, not CEMNDetail pointers
						// (c.haag 2007-10-23 10:04) - PLID 27833 - We now store "qualifying" details in an array
						// because there can be more than one
						CArray<LinkedDetailStruct,LinkedDetailStruct&> apQualifyingDetails;
						// (a.walling 2008-04-07 10:10) - PLID 29562 - This stores "qualifying" details based on a case-insensitive
						// comparison; this is only used if apQualifyingDetails is empty.
						CArray<LinkedDetailStruct,LinkedDetailStruct&> apQualifyingDetailsNoCase;
						// (c.haag 2008-11-28 15:29) - PLID 32271 - This was deprecated long ago
						//BOOL bContinueSearch = TRUE;

						// (c.haag 2008-11-26 08:50) - PLID 32196 - The logic below used to exist twice; once for when
						// paEMNDetails was NULL, and a second time where it was not. I merged the two branches together
						// and changed how we get pCheck.

						//First, try to find one with our spawned group id.
						// (c.haag 2007-08-09 16:55) - PLID 27038 - Safety check
						if (NULL == paEMNDetails && NULL == pEMN) {
							// If we get here, it means paEMNDetails and pEMN were NULL. The assertion at
							// the start of this function should also have been raised.
							ASSERT(FALSE);
							ThrowNxException("A valid CEMN object was required, but not passed in!");
						}
						// (c.haag 2007-02-28 13:17) - PLID 24989 - If we have an array of EMN details, reference
						// that array rather than the parent topic's EMN (we probably don't have a parent topic
						// if we get here anyway)
						const long nTotalDetailCount = (NULL != paEMNDetails) ? paEMNDetails->GetSize() : pEMN->GetTotalDetailCount();
						long nDetail;
						//TES 2/21/2007 - PLID 24864 - Make sure to break out of the loop once we've found a detail.

						// (a.walling 2009-03-20 13:25) - PLID 33604 - Optimization: some templates have one or more details that
						// contain ' - ' in their name, such as 'Lesion - Face' etc. Additionally is this is how we store the list
						// options in the rich text, in the form of '<detail name> - <list option>'. Previously, if the field
						// contained ' - ' we search for list options. However this requires a recordset to load the data. This was
						// happening almost hundreds of times on at least one client database. The optimization is to separate
						// the detail name into possible list option detail root names (in case of multiple ' - ' strings occurring).
						// This way, while we search through all the details, we can see if there are any details that have a name that
						// matches one of the possible list option roots. If none do, then we know for sure that the second loop would
						// never find any results, and can skip that loop (and the data loading) entirely. This cut a new chart creation
						// time down by 1 to 2 seconds.
						BOOL bFoundItemMatchingPossibleListOption = FALSE;

						// (a.walling 2009-03-20 13:29) - PLID 33604 - Split the field name into all components before each " - "
						// this way, 'Detail - Name - ListOption' would still work (it searches for 'Detail' and for 'Detail - Name')
						CStringArray saPossibleListOptionRoots;
						{
							long nIx = strField.Find(" - ");
							while (nIx != -1) {
								saPossibleListOptionRoots.Add(strField.Left(nIx));

								if (nIx + 3 < strField.GetLength()) {
									nIx = strField.Find(" - ", nIx + 3);
								} else {
									nIx = -1;
								}
							}
						}

						for (nDetail=0; nDetail < nTotalDetailCount /*&&*/ /*!pDetail*//*bContinueSearch*/; nDetail++) {
							CEMNDetail *pCheck = (NULL != paEMNDetails) ? paEMNDetails->GetAt(nDetail) : pEMN->GetDetail(nDetail);
							LinkedDetailStruct lds;
							lds.nStartPos = nMergeFieldStartPos;
							lds.bIsNarrativeDuplicate = 0; // This will be filled in later
							lds.pDetail = pCheck;
							lds.nDataID = -1;

							CString strLabelText = pCheck->GetLabelText();

							// (c.haag 2008-11-28 15:33) - PLID 32271 - Do not filter on spawned group ID
							if (strLabelText == strField) {
								// (c.haag 2007-10-23 10:07) - PLID 27833 - Use apQualifyingDetails and update the search flag
								//pDetail = pCheck;
								apQualifyingDetails.Add(lds);
								// (j.jones 2008-06-25 11:06) - PLID 27840 - add all duplicates, we will handle this later
								/*
								if (!bIncludeNarrativeDuplicates) {
									// If we get here, the caller just wants the first detail to match the field,
									// so stop looking
									bContinueSearch = FALSE;
								}
								*/
							} else if (strLabelText.CompareNoCase(strField) == 0) {
								// (a.walling 2008-04-07 10:12) - PLID 29562 - No case comparison is OK
								apQualifyingDetailsNoCase.Add(lds);
								// We won't stop the search here, and just use the NoCase selections if nothing else is found.
							}

							// (a.walling 2009-03-20 13:30) - PLID 33604 - If we have not found an item matching a possible
							// list opton root, then we should search for one. Of course if the array is empty nothing occurs.
							if (!bFoundItemMatchingPossibleListOption) {
								for (int f = 0; f < saPossibleListOptionRoots.GetSize(); f++) {
									if (strLabelText == saPossibleListOptionRoots[f]) {
										bFoundItemMatchingPossibleListOption = TRUE;
									} else if (strLabelText.CompareNoCase(saPossibleListOptionRoots[f]) == 0) {
										bFoundItemMatchingPossibleListOption = TRUE;
									}
								}
							}
						}
						// (c.haag 2007-08-14 17:39) - PLID 27073 - If we have no hits, try matching on
						// list details in the form "<list detail name> - <list option>" for all list options
						// (a.walling 2009-03-20 13:31) - PLID 33604 - This is only necessary if we know that we found a detail on the EMN
						// somewhere such that '<detailname> - <listoption>' can actually match something.
						if(/*!pDetail*//*bContinueSearch &&*/ bFoundItemMatchingPossibleListOption && saPossibleListOptionRoots.GetSize() > 0) {
							// (a.walling 2008-04-07 10:22) - PLID 29562
							CString strLCaseField = strField;
							strLCaseField.MakeLower();

							for (nDetail=0; nDetail < nTotalDetailCount /*&&*//*!pDetail*//*bContinueSearch*/; nDetail++) {
								CEMNDetail *pCheck = (NULL != paEMNDetails) ? paEMNDetails->GetAt(nDetail) : pEMN->GetDetail(nDetail);
								LinkedDetailStruct lds;
								lds.nStartPos = nMergeFieldStartPos;
								lds.bIsNarrativeDuplicate = 0; // This will be filled in later
								lds.pDetail = pCheck;
								lds.nDataID = -1; // This will be filled in later

								// (a.walling 2008-04-07 10:22) - PLID 29562
								CString strLCaseText = pCheck->GetLabelText();
								strLCaseText.MakeLower();

								// (a.walling 2009-03-20 13:35) - PLID 33604 - This would try to look only if the detail name
								// existed in the field. However, for 'Detail - Name' with field 'Detail - Name', then of course
								// it would exist. This is only necessary if there is a detail named 'Detail' so that it is possible
								// a list option in the form of 'Detail - ListOption' would exist. Of course this still runs
								// when unnecessary, mostly because there are lots of setups such as an 'Exam' detail that spawns 
								// an 'Exam - Xray' detail, etc. There is no real way around that until we optimize the way the
								// list option data is stored in the RTF header in the first place.
								// No code changes here, just a clarification.

								// (c.haag 2008-11-28 15:33) - PLID 32271 - Do not filter on spawned group ID
								if(	// and the label text exists within the field
									// (a.walling 2008-04-07 10:22) - PLID 29562 - Check without regards to case
									-1 != strLCaseField.Find( strLCaseText ) &&
									// and this is a list detail
									(eitSingleList == pCheck->m_EMRInfoType || eitMultiList == pCheck->m_EMRInfoType)) 
								{
									// If the list does not need a content reload, we will assume that the content
									// is up to date. So, just reference the list content
									if (!pCheck->GetNeedContentReload()) {
										const int nCount = pCheck->GetListElementCount();
										for(int nElement = 0; nElement < nCount /*&&*//*NULL == pDetail*//*bContinueSearch*/; nElement++) {
											if(strField == pCheck->GetLabelText() + " - " + pCheck->GetListElement(nElement).strName) {
												// (c.haag 2007-10-23 10:09) - PLID 27833 - Use apQualifyingDetails and update the search flag
												//pDetail = pCheck;
												lds.nDataID = pCheck->GetListElement(nElement).nID;
												apQualifyingDetails.Add(lds);
												// (j.jones 2008-06-25 11:06) - PLID 27840 - add all duplicates, we will handle this later
												/*
												if (!bIncludeNarrativeDuplicates) {
													// If we get here, the caller just wants the first detail to match the field,
													// so stop looking
													bContinueSearch = FALSE;
												}
												*/
											// (a.walling 2008-04-07 10:23) - PLID 29562 - Also compare case insensitive
											} else if (strField.CompareNoCase(pCheck->GetLabelText() + " - " + pCheck->GetListElement(nElement).strName) == 0) {
												lds.nDataID = pCheck->GetListElement(nElement).nID;
												apQualifyingDetailsNoCase.Add(lds);
												// We won't stop the search here, and just use the NoCase selections if nothing else is found.
											}
										}										
									} 
									// If the list is not loaded, we have no choice but to query data. The reason we cannot
									// use the EMN loader in the initial load is that it may not have populated the detail's
									// CEmrInfo object cache with list items yet. The reason we cannot call LoadContent is that
									// it does a lot of work that we don't need done at this juncture. Fortunately the query is
									// very compact, so it should not be a huge speed hit.
									else {

										// (c.haag 2008-11-26 17:29) - PLID 32268 - We used to run one query every time we came
										// here, but now we run one query to get every possible EmrDataT record we can get.
										if (!bmapEmrDataTPopulated) {
											// (c.haag 2008-12-02 10:44) - PLID 32271 - Moved into its own function
											GetEmrListOptionsForLinkedDetails(lpCon, pEMN, paEMNDetails, saMergeFields, mapEmrDataT);
											bmapEmrDataTPopulated = TRUE;
										} // if (!bmapEmrDataTPopulated) {

										// Now that we have all the EmrDataT records (EMR item list options) loaded in, we can go
										// ahead and search on those.
										CEmrDataNameAndIDAry* pastrData = NULL;
										if (mapEmrDataT.Lookup(pCheck->m_nEMRInfoID, pastrData)) {
											for (int n=0; n < pastrData->GetCount(); n++) {
												CString strData = pastrData->GetAt(n).strData;
												long nDataID = pastrData->GetAt(n).nDataID;
												if (strField == pCheck->GetLabelText() + " - " + strData) {
													// (c.haag 2007-10-23 10:09) - PLID 27833 - Use apQualifyingDetails and update the search flag
													lds.nDataID = nDataID;
													apQualifyingDetails.Add(lds);
													// (j.jones 2008-06-25 11:06) - PLID 27840 - add all duplicates, we will handle this later
												// (a.walling 2008-04-07 10:23) - PLID 29562 - Also compare case insensitive
												} else if (strField.CompareNoCase(pCheck->GetLabelText() + " - " + strData) == 0) {
													lds.nDataID = nDataID;
													apQualifyingDetailsNoCase.Add(lds);
													// We won't stop the search here, and just use the NoCase selections if nothing else is found.
												}

											} // for (int n=0; n < pastrData->GetCount(); n++) {
										} // if (mapEmrDataT.Lookup(pCheck->m_nEMRInfoID, pastrData)) {
									}
								}
							}
						} // if(/*!pDetail*//*bContinueSearch &&*/ -1 != strField.Find(" - ")) {

						/*if(pDetail) {
							if (pDetail->m_EMRInfoType != eitNarrative) {
								// (a.wetta 2007-03-06 16:07) - PLID 25008 - Don't add the detail if the linked detail is a narrative itself!
								arDetails.Add(pDetail);
							}
						}*/

						// (j.jones 2008-06-25 11:06) - PLID 27840 - sort apQualifyingDetails or apQualifyingDetailsNoCase,
						// this will order them by topic order, then by placement inside each topic
						if(apQualifyingDetails.GetSize() > 1) {
							SortDetailArray(apQualifyingDetails);
						}
						else if(apQualifyingDetailsNoCase.GetSize() > 1) {
							SortDetailArray(apQualifyingDetailsNoCase);
						}

						// (c.haag 2007-10-23 10:10) - PLID 27833 - Pull the results from apQualifyingDetails
						const long nQualifyingDetails = apQualifyingDetails.GetSize();
						/*
						if (!bIncludeNarrativeDuplicates) {
							// Safety check -- If we don't include duplicates, there should be none for the current field
							ASSERT(nQualifyingDetails <= 1); 
						}
						*/
						for (long n=0; n < nQualifyingDetails; n++) {
							LinkedDetailStruct lds = apQualifyingDetails[n];
							CEMNDetail* pDetail = lds.pDetail;
							if (pDetail->m_EMRInfoType != eitNarrative) {

								// (j.jones 2008-06-25 11:06) - PLID 27840 - if not allowing duplicates,
								// just break after the first item we find
								// (c.haag 2008-12-02 16:24) - PLID 32296 - Now we always store duplicates.
								// If we did not, then a non-duplicate cache may persist when a caller wants
								// duplicates at a later point in time. However, we need to know which details
								// are duplicates.
								//if(!bIncludeNarrativeDuplicates) {
								//	break;
								//}
								if (n > 0) {
									lds.bIsNarrativeDuplicate = TRUE;
								} else {
									lds.bIsNarrativeDuplicate = FALSE;
								}

								m_arLinkedDetails.Add(lds);
							}	
						}

						// (a.walling 2008-04-07 10:15) - PLID 29562 - Use the nocase results as a last resort
						if (nQualifyingDetails == 0) {
							for (long i=0; i < apQualifyingDetailsNoCase.GetSize(); i++) {
								LinkedDetailStruct lds = apQualifyingDetailsNoCase[i];
								CEMNDetail* pDetail = lds.pDetail;
								if (pDetail->m_EMRInfoType != eitNarrative) {

									// (c.haag 2008-12-02 16:24) - PLID 32296 - Now we always store duplicates.
									// If we did not, then a non-duplicate cache may persist when a caller wants
									// duplicates at a later point in time. However, we need to know which details
									// are duplicates.
									//if (!bIncludeNarrativeDuplicates) {
									//	break;
									//}
									if (i > 0) {
										lds.bIsNarrativeDuplicate = TRUE;
									} else {
										lds.bIsNarrativeDuplicate = FALSE;
									}

									m_arLinkedDetails.Add(lds);
								}	
							}
						}

					} // if(bIsDetail) {
				} // for(int i = 0; i < saMergeFields.GetSize(); i++) {

				// (c.haag 2008-11-26 17:46) - PLID 32268 - Cleanup
				POSITION pos = mapEmrDataT.GetStartPosition();
				while (pos) {
					CEmrDataNameAndIDAry* pastr;
					long nDummy = -1;
					mapEmrDataT.GetNextAssoc(pos, nDummy, pastr);
					delete pastr;
				}

			} // case eitNarrative:
			break;
		default:
			//No other types have linked details.
			break;
		}

		//TES 7/26/2007 - PLID 25091 - Remember which details are linked to us, so we don't have to repeat this tedious calculation.		
		// (c.haag 2008-12-04 12:41) - PLID 31693 - We used to build content in arDetails and copy it to m_arLinkedDetails. Now, we
		// build content in m_arLinkedDetails, and then pull from the cache at the end. The reason is that the caller may not want
		// *everything* in the cache. Some filtering has to be done. By calling GetLinkedDetails from itself (there will not be
		// infinite recursion), then we can maintain those filters in just one place in code.
		m_bLinkedDetailsCached = true;
		GetLinkedDetails(arDetails, pEMN, paEMNDetails, lpCon, bIncludeNarrativeDuplicates, bIncludeDetailDuplicates); // This will pull from cache

	// (c.haag 2007-03-19 13:19) - PLID 24663 - Use thread-safe exception handling
	}NxCatchAllThread("Error in CEMNDetail::GetLinkedDetails");
}


// (c.haag 2008-12-04 10:49) - PLID 31693 - This only returns detail pointers instead of extended detail information
void CEMNDetail::GetLinkedDetails(CArray<CEMNDetail*,CEMNDetail*> &arDetails, CEMN* pEMN,
								  CArray<CEMNDetail*,CEMNDetail*>* paEMNDetails /*= NULL */,
								  _Connection *lpCon /*= NULL */,
								  BOOL bIncludeNarrativeDuplicates /*= FALSE */,
								  BOOL bIncludeDetailDuplicates /*= FALSE */)
{
	CArray<LinkedDetailStruct,LinkedDetailStruct&> arDetailsWithInfo;
	GetLinkedDetails(arDetailsWithInfo, pEMN, paEMNDetails, lpCon, bIncludeNarrativeDuplicates, bIncludeDetailDuplicates);
	arDetails.RemoveAll();
	for (int i=0; i < arDetailsWithInfo.GetSize(); i++) {
		arDetails.Add(arDetailsWithInfo[i].pDetail);
	}
}

// (z.manning 2011-10-11 09:00) - PLID 42061 - Now returns a sql fragment
CSqlFragment CEMNDetail::GetColumnSql(long nColumnID) const
{
	CSqlFragment sqlCombo;
	m_mapColumnKeysToQueries.Lookup(nColumnID, sqlCombo);
	return sqlCombo;
}

// (z.manning 2011-10-11 09:00) - PLID 42061 - Now takes a sql fragment
void CEMNDetail::SetColumnSql(long nColumnID, CSqlFragment &sql)
{
	m_mapColumnKeysToQueries.SetAt(nColumnID, sql);
}

// (z.manning, 04/10/2007) - PLID 25560 - Moved this function here from CEmrItemAdvTableDlg so 
// that the popup dialog could do this as well.
void CEMNDetail::CalcComboSql()
{
	//calculate the combo sql for all columns
	for(int i = 0; i < GetColumnCount(); i++) 
	{
		CSqlFragment sqlCombo;
		TableColumn tc = GetColumn(i);

		// (z.manning 2009-03-19 12:56) - PLID 33576 - Renamed 'Active' to 'Visible' to be consisent
		// with other places.
		// (z.manning 2009-03-19 16:45) - PLID 33138 - Included DropdownGroupID here for use in 
		// unspawning.
		// (c.haag 2010-02-24 15:49) - PLID 21301 - The sorting can now either be by element, or by data
		// (z.manning 2011-03-11) - PLID 42618 - Text type columns can now have dropdown elements.
		// I ended up just removing the filter on EmrDataT.ListType as this is not the place to handle that.

		// (j.jones 2011-04-28 14:39) - PLID 43122 - If the currently selected EMN provider has ProvidersT.FloatEMRData = 1,
		// load the data from EmrProviderFloatTableDropdownT. If we are sorting alphabetically, sort the "floated" items
		// first, then the regular items. If not sorting alphabetically, sort the "floated" items in Count order DESC,
		// and then the regular items in their normal sort order.

		long nProviderIDForFloatingData = GetProviderIDForFloatingData();

		if(nProviderIDForFloatingData != -1)
		{
			//"float" most common selections first
			CSqlFragment sqlOrderBy;
			if(tc.m_bAutoAlphabetizeDropdown) {
				//sort "floated" items first, but alphabetically, remainder second, also alphabetically
				sqlOrderBy = CSqlFragment(
					"ORDER BY (CASE WHEN FloatedItemsQ.Count Is Not Null THEN 1 ELSE 0 END) DESC, "
					"EMRTableDropdownInfoT.Data ASC, EMRTableDropdownInfoT.ID");
			}
			else {
				//sort "floated" items first, by count DESC, remainder second, by sort order
				sqlOrderBy = CSqlFragment("ORDER BY Coalesce(FloatedItemsQ.Count, -1) DESC, "
					"EMRTableDropdownInfoT.SortOrder ASC, EMRTableDropdownInfoT.ID");
			}

			// (z.manning 2011-10-11 08:51) - PLID 42061 - Load dropdown stamp filters
			// (z.manning 2011-10-11 17:27) - PLID 45728 - Added IsDefault
			// (j.jones 2012-11-27 10:38) - PLID 53144 - default values have been moved to EMRTableDropdownStampDefaultsT
			// (j.jones 2013-03-04 13:06) - PLID 55380 - moved the stamp filters & defaults to their own next recordsets
			// (j.jones 2013-03-07 12:34) - PLID 55511 - rearranged the code so defaults are queried here,
			// stamp filters are queried later - and potentially not at all based on a new preference
			sqlCombo = CSqlFragment(
				"SELECT EMRTableDropdownInfoT.ID, EMRTableDropdownInfoT.Data, \r\n"
				"	CASE WHEN EMRTableDropdownInfoT.Inactive = 0 THEN 1 ELSE 0 END AS Visible, \r\n"
				"	EMRTableDropdownInfoT.DropdownGroupID, \r\n"
				"	Convert(bit, CASE WHEN FloatedItemsQ.Count Is Not Null THEN 1 ELSE 0 END) AS IsFloated \r\n"
				"FROM EMRTableDropdownInfoT \r\n"
				// (a.walling 2013-03-21 11:23) - PLID 55810 - The EmrProviderFloatDataT join can be skipped entirely if the providerid being scanned for is set to -1
				"LEFT JOIN ( \r\n"
				"	SELECT EMRTableDropdownGroupID AS DropdownGroupID, Count \r\n"
				"	FROM EmrProviderFloatTableDropdownT \r\n"
				"	WHERE ProviderID = {INT} AND {INT} <> -1) AS FloatedItemsQ ON EMRTableDropdownInfoT.DropdownGroupID = FloatedItemsQ.DropdownGroupID \r\n"
				"WHERE EMRTableDropdownInfoT.EMRDataID = {INT} \r\n"
				"{SQL} \r\n"
				"SELECT EMRTableDropdownStampDefaultsT.EMRTableDropdownInfoID, EMRTableDropdownStampDefaultsT.StampID \r\n"
				"FROM EMRTableDropdownStampDefaultsT \r\n"
				"INNER JOIN EMRTableDropdownInfoT ON EMRTableDropdownStampDefaultsT.EMRTableDropdownInfoID = EMRTableDropdownInfoT.ID \r\n"
				"WHERE EMRTableDropdownInfoT.EMRDataID = {INT}",
				nProviderIDForFloatingData, nProviderIDForFloatingData, tc.nID, sqlOrderBy,
				tc.nID);
		}
		else {
			//use the normal combo SQL, showing all items either sorted alphabetically or by sort order,
			//and force IsFloated to FALSE for all
			// (z.manning 2011-10-11 08:51) - PLID 42061 - Load dropdown stamp filters
			// (z.manning 2011-10-11 17:27) - PLID 45728 - Added IsDefault
			// (j.jones 2013-03-04 13:06) - PLID 55380 - moved the stamp filters & defaults to their own next recordsets
			// (j.jones 2013-03-07 12:34) - PLID 55511 - rearranged the code so defaults are queried here,
			// stamp filters are queried later - and potentially not at all based on a new preference
			CString strSortField = (tc.m_bAutoAlphabetizeDropdown) ? "EMRTableDropdownInfoT.Data" : "SortOrder";
			sqlCombo = CSqlFragment(
				"SELECT EMRTableDropdownInfoT.ID, EMRTableDropdownInfoT.Data, \r\n"
				"	CASE WHEN EMRTableDropdownInfoT.Inactive = 0 THEN 1 ELSE 0 END AS Visible, \r\n"
				"	EMRTableDropdownInfoT.DropdownGroupID, \r\n"
				"	Convert(bit, 0) AS IsFloated \r\n"
				"FROM EMRTableDropdownInfoT \r\n"
				"WHERE EMRTableDropdownInfoT.EMRDataID = {INT} \r\n"
				"ORDER BY {CONST_STRING} ASC, EMRTableDropdownInfoT.ID \r\n"
				"\r\n"
				// (z.manning 2011-10-26 15:34) - PLID 42061 - Do not set inactive dropdown items as defaults.
				"SELECT EMRTableDropdownStampDefaultsT.EMRTableDropdownInfoID, EMRTableDropdownStampDefaultsT.StampID \r\n"
				"FROM EMRTableDropdownStampDefaultsT \r\n"
				"INNER JOIN EMRTableDropdownInfoT ON EMRTableDropdownStampDefaultsT.EMRTableDropdownInfoID = EMRTableDropdownInfoT.ID \r\n"
				"WHERE EMRTableDropdownInfoT.EMRDataID = {INT} AND EMRTableDropdownInfoT.Inactive = 0",
				tc.nID, strSortField,
				tc.nID);
		}

		// (j.jones 2013-03-07 12:37) - PLID 55511 - Add the stamp filters only if filtering is not disabled.
		// We only need to do this once because it's the same query regardless of whether content is floated or not.
		if(GetRemotePropertyInt("DisableSmartStampTableDropdownStampFilters", 0, 0, "<None>", true) == 0) {
			sqlCombo = CSqlFragment("{SQL}"
				"\r\n"
				"SELECT EmrTableDropdownStampFilterT.EMRTableDropdownInfoID, EmrTableDropdownStampFilterT.StampID \r\n"
				"FROM EmrTableDropdownStampFilterT \r\n"
				"INNER JOIN EMRTableDropdownInfoT ON EmrTableDropdownStampFilterT.EMRTableDropdownInfoID = EMRTableDropdownInfoT.ID \r\n"
				"WHERE EMRTableDropdownInfoT.EMRDataID = {INT} ",
				sqlCombo, tc.nID);
		}

		SetColumnSql(tc.nID, sqlCombo);
	}
}

CString CEMNDetail::GetImagePath(bool bFullPath)
{
	if(bFullPath) {
		LoadContent();
//#pragma TODO("(t.schneider 5/24/2006) Could this be called before the detail is attached to a topic?")
		// Tom researched the answer, and it is "No"
		return m_aisState->CalcCurrentBackgroundImageFullPath(m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID(), m_strBackgroundImageFilePath, m_eitBackgroundImageType);
	}
	else {
		return m_strBackgroundImageFilePath;
	}
}

eImageType CEMNDetail::GetImageType() const
{
	return m_eitBackgroundImageType;
}

DWORD CEMNDetail::GetImageStateVersion()
{
	LoadContent();
	return m_aisState->m_dwVersion;
}

_variant_t CEMNDetail::GetInkData()
{
	LoadContent();
	return m_aisState->m_varInkData;
}

void CEMNDetail::SetInkData(const _variant_t &varData)
{
	m_aisState->m_varInkData = varData;
	RequestStateChange(m_aisState->AsSafeArrayVariant());
}

_variant_t CEMNDetail::GetImageTextData()
{
	LoadContent();
	return m_aisState->m_varTextData;
}

// (r.gonet 05/27/2011) - PLID 30359 - Added option not to change state
void CEMNDetail::SetImageTextData(const _variant_t &varData, bool bChangeState/*=true*/)
{
	VariantClear(&m_aisState->m_varTextData);
	m_aisState->m_varTextData = varData;
	// Only if the caller desires do we reload the state. It may be that the changed state is already reflected.
	if(bChangeState) {
		RequestStateChange(m_aisState->AsSafeArrayVariant());
	} else {
		// (r.gonet 05/27/2011) - PLID 30359 - At least set to unsaved
		SetUnsaved();
	}
}

// (r.gonet 02/14/2012) - PLID 37682 - Sets the text string filter that will filter out stamps.
//  Does not reflect the current filter to the image.
void CEMNDetail::SetImageTextStringFilter(CTextStringFilterPtr pTextStringFilter)
{
	m_pTextStringFilter = pTextStringFilter;
}

// (r.gonet 02/13/2012) - PLID 37682 - Gets the current filter on the image stamps.
CTextStringFilterPtr CEMNDetail::GetImageTextStringFilter()
{
	return m_pTextStringFilter;
}

// (r.gonet 02/13/2012) - PLID 37682 - Reflects the current filter onto the image so certain stamps are hidden.
void CEMNDetail::ReflectImageTextStringFilter()
{
	if(m_EMRInfoType == eitImage) {
		if(m_pEmrItemAdvDlg != NULL) {
			((CEmrItemAdvImageDlg *)m_pEmrItemAdvDlg)->SetImageTextStringFilter(m_pTextStringFilter);
		}

		// Update the preview so that it reflects our filter
		if(m_pParentTopic && m_pParentTopic->GetParentEMN()->GetInterface()) {
			m_pParentTopic->GetParentEMN()->GetInterface()->SendMessage(NXM_UPDATE_EMR_PREVIEW,
				(WPARAM)FALSE, (LPARAM)this);
		}
	}
}

// (r.gonet 02/13/2012) - PLID 37682 - Compare two CStrings for an ascending alphabetical order.
static int QSortCompareCStringsAsc(const void *a, const void *b)
{
	CString *pA = (CString*)a;
	CString *pB = (CString*)b;
	return (pA->Compare(*pB));
}

// (r.gonet 02/14/2012) - PLID 37682 - Opens the stamp filter editor, then sets and reflects the filter the user selects.
//  Returns TRUE if the image has been filtered. FALSE otherwise (ie the user cancelled or there were no stamps to filter on)
BOOL CEMNDetail::EditImageStampFilter(CWnd *pParent)
{
	// We can only filter when there are stamps
	CNxInkPictureText niptText;
	niptText.LoadFromVariant(m_aisState->m_varTextData);
	if(niptText.GetStringCount() == 0) {
		MessageBox(pParent->GetSafeHwnd(), "The image has no stamps to filter.", 0, MB_ICONERROR|MB_OK);
		return FALSE;
	}
	// (j.armen 2012-06-08 09:32) - PLID 49607 - Input for this multiselect dlg isn't cacheable, so let's set a manual cache name
	CMultiSelectDlg dlg(pParent, "EditImageStampFilter");

	// Get the in use stamps.
	CEmrItemAdvImageState ais;
	ais.CreateFromSafeArrayVariant(this->GetState());
	CNxInkPictureText stamps;
	stamps.LoadFromVariant(ais.m_varTextData);

	// Now find all the distinct stamp types in use on the image
	CStringArray saTypes;
	for(int i = 0; i < stamps.GetStringCount(); i++)
	{
		TextString ts = stamps.GetTextString(i);
		CString strTypeName = ts.strTypeName;
		if(strTypeName.IsEmpty()) {
			strTypeName = " < No Type >";
		}
		bool bAdd = true;
		for(int j = 0; j < saTypes.GetSize(); j++) {
			if(saTypes.GetAt(j) == strTypeName) {
				// We've already seen this type before and added it before, so don't add it again.
				bAdd = false;
				break;
			} else {
				// Not a match, it may be that the type exists further along or it may be
				//  that the type is new and must be added.
			}
		}
		if(bAdd) {
			// This type is not in our list, so add it.
			saTypes.Add(strTypeName);
		} else {
			// This type is already in our list. This is a unique list so don't add it again.
		}
	}
	// Sort the array alphabetically
	qsort(saTypes.GetData(), saTypes.GetSize(), sizeof(CString*), QSortCompareCStringsAsc);
	
	// Open the selection window so that the user can make their filtering decisions.
	CVariantArray arySelectedTypes;
	if(IDOK == dlg.OpenWithStringArray(saTypes, "Select the stamp types to be shown. Stamps with other types will be hidden. This filter is temporary and will not be saved when you close the EMN.")) {
		// Get every type the user selected.
		dlg.FillArrayWithNames(arySelectedTypes);

		// Modify the filter from these choices
		CTextStringFilterPtr pTextStringFilter = make_shared<CTextStringFilter>();

		this->SetImageTextStringFilter(pTextStringFilter);
		
		for(int i = 0; i < arySelectedTypes.GetSize(); i++){
			CString strType = VarString(arySelectedTypes.GetAt(i));
			if(strType == " < No Type >") {
				// For the no type stamps, we have a special display name, so translate it back to the type name, which is an empty string.
				strType = "";
			} else {
				// For any other type we have just used the type name as the display name
			}

			// Show this type.
			pTextStringFilter->AddType(strType);
		}
		this->ReflectImageTextStringFilter();

		return TRUE;
	} else {
		// User cancelled out of the dialog so cancel filtering.
		return FALSE;
	}
}

// (z.manning 2011-10-05 15:52) - PLID 45842
void CEMNDetail::SetImagePrintData(const _variant_t &varData)
{
	VariantClear(&m_aisState->m_varPrintData);
	m_aisState->m_varPrintData = varData;
	RequestStateChange(m_aisState->AsSafeArrayVariant());
}

// (j.armen 2014-07-22 09:03) - PLID 62836 - Handle state change for PenColorChanged.
// Only templates can modify the state
void CEMNDetail::OnCurrentPenColorChanged(long nPenColor)
{
	if (m_bIsTemplateDetail)
	{
		m_nDefaultPenColor = nPenColor;
		SetUnsaved();
	}
}

// (j.armen 2014-07-22 09:03) - PLID 62836 - Handle state change for PenSizeChanged.
// Only templates can modify the state
void CEMNDetail::OnCurrentPenSizeChanged(byte nPenSizePercent)
{
	if (m_bIsTemplateDetail)
	{
		m_nDefaultPenSizePercent = nPenSizePercent;
		SetUnsaved();
	}
}

// (z.manning 2011-10-05 17:16) - PLID 45842
_variant_t CEMNDetail::GetImagePrintData()
{
	return m_aisState->m_varPrintData;
}

//DRT 1/21/2008 - PLID 28603 - Retrieves a pointer to the member array of hotspots.  Ensures that 
//	content is loaded.
CEMRHotSpotArray* CEMNDetail::GetHotSpotArray()
{
	//This is content, not part of the state
	LoadContent();
	return &m_aryImageHotSpots;
}

void CEMNDetail::SetImage(const CString &strPath, enum eImageType nType)
{
	m_aisState->m_strImagePathOverride = strPath;
	m_aisState->m_eitImageTypeOverride = nType;
	RequestStateChange(m_aisState->AsSafeArrayVariant());
}

// (c.haag 2008-11-26 16:51) - PLID 32267 - We now support optional loader and connection pointers
void CEMNDetail::GetImageSelectedHotSpotIDs(CArray<long,long> &arynHotSpotIDs, CEMNLoader* pEMNLoader /*= NULL */,
							 ADODB::_Connection *lpCon /*= NULL */)
{
	// (c.haag 2008-11-26 16:52) - PLID 32267 - Pass in the EMN loader and connection pointers here
	LoadContent(FALSE, pEMNLoader, lpCon);

	arynHotSpotIDs.RemoveAll();
	CString strHotSpotIDs = m_aisState->m_strSelectedHotSpotData;
	while(!strHotSpotIDs.IsEmpty())
	{
		int nSemicolon = strHotSpotIDs.Find(';');
		if(nSemicolon == -1) {
			nSemicolon = strHotSpotIDs.GetLength();
		}
		
		arynHotSpotIDs.Add(AsLong(_bstr_t(strHotSpotIDs.Left(nSemicolon))));
		strHotSpotIDs.Delete(0, nSemicolon + 1);
	}
}

//TES 2/15/2010 - PLID 37375 - Added, similar to the above except it fills an array with CEMRHotSpots instead of IDs.
void CEMNDetail::GetImageSelectedHotSpots(CEMRHotSpotArray &arynHotSpots, CEMNLoader* pEMNLoader /*= NULL */,
							 ADODB::_Connection *lpCon /*= NULL */)
{
	// (c.haag 2008-11-26 16:52) - PLID 32267 - Pass in the EMN loader and connection pointers here
	LoadContent(FALSE, pEMNLoader, lpCon);

	arynHotSpots.RemoveAll();
	CString strHotSpotIDs = m_aisState->m_strSelectedHotSpotData;
	while(!strHotSpotIDs.IsEmpty())
	{
		int nSemicolon = strHotSpotIDs.Find(';');
		if(nSemicolon == -1) {
			nSemicolon = strHotSpotIDs.GetLength();
		}
		
		CEMRHotSpot *pSpot = GetHotSpotFromID(AsLong(_bstr_t(strHotSpotIDs.Left(nSemicolon))));
		if(pSpot == NULL) {
			ASSERT(FALSE);
			AfxThrowNxException("Could not find HotSpot corresponding to ID %li", AsLong(_bstr_t(strHotSpotIDs.Left(nSemicolon))));
		}
		arynHotSpots.Add(pSpot);
		strHotSpotIDs.Delete(0, nSemicolon + 1);
	}
}

//TES 2/15/2010 - PLID 37375 - For a given HotSpotID, looks up the corresponding CEMRHotSpot on this detail.
CEMRHotSpot* CEMNDetail::GetHotSpotFromID(long nID)
{
	for(int i = 0; i < m_aryImageHotSpots.GetSize(); i++) {
		if(m_aryImageHotSpots[i]->GetID() == nID) {
			return m_aryImageHotSpots[i];
		}
	}
	return NULL;
}

// (z.manning 2010-03-03 08:44) - PLID 37493 - Gets a hot spot for a given point
// Note: It's obviously possible that there is more than one hot spot for a given point so we just
// arbitrarily return the first one we come across.
// (z.manning 2011-09-09 11:20) - PLID 45335 - Renamed and reworked this function to also support 3D models.
CEMRHotSpot* CEMNDetail::GetHotSpotFromDetailStamp(EmrDetailImageStamp *pDetailStamp)
{
	if(Is3DImage())
	{
		CEMRHotSpotArray *paryHotSpots = GetHotSpotArray();
		CEMRHotSpot *pHotSpot = paryHotSpots->FindBy3DHotSpotID(pDetailStamp->n3DHotSpotID);
		return pHotSpot;
	}
	else
	{
		CPoint pt(pDetailStamp->x, pDetailStamp->y);
		CEMRHotSpotArray *paryHotSpots = GetHotSpotArray();
		for(int nHotSpotIndex = 0; nHotSpotIndex < paryHotSpots->GetSize(); nHotSpotIndex++) {
			if(paryHotSpots->GetAt(nHotSpotIndex)->PtInSpot(pt)) {
				return paryHotSpots->GetAt(nHotSpotIndex);
			}
		}
	}

	return NULL;
}

BOOL CEMNDetail::GetAllowUnspawn() const
{
	return m_bAllowUnspawn;
}

void CEMNDetail::SetAllowUnspawn(BOOL bAllowUnspawn)
{
	m_bAllowUnspawn = bAllowUnspawn;
}

void CEMNDetail::ChangeHilightColor(COLORREF clrHighlight)
{
	m_clrHighlight = clrHighlight;
	if (m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd()) && (m_EMRInfoType == 2 || m_EMRInfoType == 3)) {
		((CEmrItemAdvListDlg *)(m_pEmrItemAdvDlg))->ChangeHilightColor(clrHighlight);
	}
}

// (c.haag 2007-07-16 08:35) - PLID 26651 - Made this const
_variant_t CEMNDetail::GetState() const
{
	// (c.haag 2007-05-07 16:22) - PLID 25928 - This will fail if the state is empty
	if (VT_EMPTY == m_varState.vt) {
		ASSERT(FALSE);
		ThrowNxException("Called CEMNDetail::GetState() where the state was empty!");
	}
	return m_varState;
}

void CEMNDetail::SetState(_variant_t varNewState)
{
	if(m_EMRInfoType == eitImage) {
		// (z.manning 2010-02-23 13:07) - PLID 37412 - Image states are now even more complicated as we
		// now have data in the state for detail image stamps. As opposed to the rest of the state (which
		// we go well out of our way to inconviently force into one database field) the detail image stamps
		// are stored in their own table and we also must keep an array of pointers to the detail stamps
		// that we cannot be constantly recreating as the pointer values are needed for spawning. Thus,
		// the only time we ever want to load detail stamps from the state is if the state has data but
		// the array does not yet, which basically means the first time we load this state when loading
		// an existing EMN.
		if(!m_bLoadedDetailImageStampsFromVariant && m_arypImageStamps.GetSize() == 0) {
			CEmrItemAdvImageState aisNew;
			aisNew.CreateFromSafeArrayVariant(varNewState);
			if(aisNew.m_varDetailImageStamps.vt != VT_NULL && aisNew.m_varDetailImageStamps.vt != VT_EMPTY) {
				m_arypImageStamps.LoadFromSafeArrayVariant(aisNew.m_varDetailImageStamps);
				// (z.manning 2010-09-20 11:47) - PLID 40585 - We only ever want to load the detail stamps from
				// the variant state once, so since we just did that, set a flag so that we never attempt to 
				// load from it again.
				m_bLoadedDetailImageStampsFromVariant = TRUE;
			}
		}
	}

	m_varState = varNewState;	
}

// (j.jones 2006-08-01 10:30) - PLID 21704 - resets the stored widths in memory to -1
void CEMNDetail::ResetStoredColumnWidths()
{
	//DRT 7/11/2007 - PLID 24105 - Wipe out all our saved widths
	m_tcwWidths.ClearAll();
}

double CEMNDetail::GetSliderMin()
{
	LoadContent();
	return m_dSliderMin;
}

double CEMNDetail::GetSliderMax()
{
	LoadContent();
	return m_dSliderMax;
}

double CEMNDetail::GetSliderInc()
{
	LoadContent();
	return m_dSliderInc;
}

//if editing a locked item creates a new copy, and we have unsaved items
//using the old info item, make them use the new info item
void CEMNDetail::UpdateInfoID(long nOldEMRInfoID, long nNewEMRInfoID, OPTIONAL IN EMRInfoChangedIDMap* pChangedIDMap /*= NULL*/, long nForceUpdateForDetailID /*= -1*/)
{
	bool bNeedToFreeMap = false;
		
	//only update if we are on a template, or we are unsaved, or we are forcing an update for this detail
	if((m_bIsTemplateDetail || m_nEMRDetailID == -1 || m_nEMRDetailID == nForceUpdateForDetailID) && m_nEMRInfoID == nOldEMRInfoID) {

		// (j.jones 2010-11-11 16:56) - PLID 41457 - Mark this detail as unsaved. Kinda important.
		SetUnsaved(TRUE);

		// (c.haag 2008-06-05 13:24) - PLID 27240 - Remember the fact that this item was updated, and get audit information from it
		m_bDetailWasBroughtUpToDate = TRUE;
		GetDetailNameAndDataForAudit(m_strPreUpdatedDetailName,m_strPreUpdatedDetailData);

		m_nEMRInfoID = nNewEMRInfoID;

		if(!pChangedIDMap) {
			//TES 12/11/2006 - PLID 23790 - OK, we'll have to figure it out for ourselves from data.
			pChangedIDMap = new EMRInfoChangedIDMap;
			bNeedToFreeMap = true;
			
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			// (z.manning 2011-04-26 12:25) - PLID 37604 - Reworked this query a bit and also added old and new list type fields.
			// (c.haag 2011-10-28) - PLID 46167 - Optimized this query to get rid of the EmrDataT join on EmrDataT.
			_RecordsetPtr rsDataMap = CreateParamRecordset(

				"SET NOCOUNT ON \r\n"
				"DECLARE @t1 TABLE (EmrDataID INT NOT NULL PRIMARY KEY) \r\n"
				"INSERT INTO @t1 (EmrDataID) \r\n"
				"SELECT EmrDataID FROM EmrTableDropdownInfoT \r\n "
				"INNER JOIN EmrDataT E ON E.ID = EmrTableDropdownInfoT.EmrDataID \r\n "
				"AND E.ListType IN (3,4) \r\n" // (a.walling 2013-02-28 17:35) - PLID 55391 - This eliminates thousands of seeks!
				"WHERE E.EmrInfoID = {INT} GROUP BY EmrDataID \r\n "
				"\r\n "
				"DECLARE @OldData TABLE (ID INT NOT NULL, EmrDataGroupID INT NOT NULL PRIMARY KEY, ListType INT) \r\n"
				"DECLARE @NewData TABLE (ID INT NOT NULL, EmrDataGroupID INT NOT NULL PRIMARY KEY, ListType INT) \r\n"
				"\r\n "
				"INSERT INTO @NewData (ID, EmrDataGroupID, ListType) \r\n"
				"	SELECT ID, EmrDataGroupID, ListType FROM EmrDataT WHERE EmrInfoID = {INT} \r\n "
				"INSERT INTO @OldData (ID, EmrDataGroupID, ListType) \r\n"
				"	SELECT ID, EmrDataGroupID, ListType FROM EmrDataT WHERE EmrInfoID = {INT} \r\n "
				"\r\n "
				"SET NOCOUNT OFF \r\n"
				"\r\n"
				"SELECT EmrDataT.ID, NewData.ID AS NewDataID, EmrDataT.ListType, NewData.ListType AS NewListType, \r\n"
				"	CASE WHEN EmrDataT.ID IN (SELECT EmrDataID FROM @t1) THEN 1 ELSE 0 END AS HasDropdowns \r\n"
				"FROM @OldData EmrDataT \r\n"
				"LEFT JOIN @NewData NewData ON NewData.EmrDataGroupID = EmrDataT.EmrDataGroupID \r\n"

				/*
				"SELECT EmrDataT.ID, NewData.ID AS NewDataID, EmrDataT.ListType, NewData.ListType AS NewListType, \r\n"
				"	CASE WHEN EmrDataT.ID IN (SELECT EmrDataID FROM EmrTableDropdownInfoT) THEN 1 ELSE 0 END AS HasDropdowns \r\n"
				"FROM EmrDataT \r\n"
				"LEFT JOIN EmrDataT NewData ON NewData.EmrDataGroupID = EmrDataT.EmrDataGroupID AND NewData.EmrInfoID = {INT} \r\n"
				"WHERE EmrDataT.EmrInfoID = {INT} \r\n"*/
				, nOldEMRInfoID
				, nNewEMRInfoID, nOldEMRInfoID
				);

			while(!rsDataMap->eof) {
				EMRInfoChangedData *pData = new EMRInfoChangedData;
				pData->nOldDataID = AdoFldLong(rsDataMap, "ID");
				pData->nNewDataID = AdoFldLong(rsDataMap, "NewDataID", -1);
				pData->nOldListType = AdoFldLong(rsDataMap, "ListType", -1);
				pData->nNewListType = AdoFldLong(rsDataMap, "NewListType", -1);
				if(AdoFldLong(rsDataMap, "HasDropdowns")) {
					// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
					// (z.manning 2011-04-26 12:25) - PLID 37604 - Reworked this query a bit and also added old data field.
					_RecordsetPtr rsDropdownMap = CreateParamRecordset(
						"SELECT EmrTableDropdownInfoT.ID, NewDropdown.ID AS NewDropdownID, EmrTableDropdownInfoT.Data \r\n"
						"FROM EmrTableDropdownInfoT \r\n"
						"LEFT JOIN EmrTableDropdownInfoT NewDropdown ON NewDropdown.DropdownGroupID = EmrTableDropdownInfoT.DropdownGroupID \r\n"
						"	AND NewDropdown.EmrDataID = {INT} \r\n"
						"WHERE EmrTableDropdownInfoT.EmrDataID = {INT} \r\n"
						, pData->nNewDataID, pData->nOldDataID);
					while(!rsDropdownMap->eof) {
						EMRInfoChangedDropdown *pChangedDropdown = new EMRInfoChangedDropdown;
						pChangedDropdown->nOldDropdownID = AdoFldLong(rsDropdownMap, "ID");
						pChangedDropdown->nNewDropdownID = AdoFldLong(rsDropdownMap, "NewDropdownID", -1);
						pChangedDropdown->strOldData = AdoFldString(rsDropdownMap, "Data", "");
						pData->aryChangedDropdownIDs.Add(pChangedDropdown);
						rsDropdownMap->MoveNext();
					}
				}
				pChangedIDMap->aryChangedDataIDs.Add(pData);
				rsDataMap->MoveNext();
			}
		}

		//now the fun part - change over all our IDs to use the IDs in the EMRInfoChangedIDMap

		if(pChangedIDMap) {

			//EMRDataIDs

			for(int i=0; i<pChangedIDMap->aryChangedDataIDs.GetSize(); i++) {

				EMRInfoChangedData *pChangedData = pChangedIDMap->aryChangedDataIDs.GetAt(i);

				long nOldDataID = pChangedData->nOldDataID;
				long nNewDataID = pChangedData->nNewDataID;

				//update the data ID arrays

				// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
				long j = 0;

				BOOL bFound = FALSE;
				for (j=0; j<m_arListElements.GetSize() && !bFound; j++) {

					ListElement le = m_arListElements[j];
					if (le.nID == nOldDataID) {
						le.nID = nNewDataID;
						//DRT 10/5/2007 - PLID 27670 - I don't know how long this code has existed here, but
						//	it has never properly set the IDs back in the array!
						m_arListElements.SetAt(j, le);
						bFound = TRUE;
					}
				}
				bFound = FALSE;
				for (j=0; j<m_arTableElements.GetSize() && !bFound; j++) {

					TableElement te = m_arTableElements[j];
					if (te.m_pColumn->nID == nOldDataID) {
						te.m_pColumn->nID = nNewDataID;
						bFound = TRUE;
					}
					else if (te.m_pRow->m_ID.nDataID == nOldDataID) {
						te.m_pRow->m_ID.nDataID = nNewDataID;
						bFound = TRUE;
					}
				}
				
				//and now the table dropdowns

				/* JMJ I'm pretty sure this isn't necessary, LoadContent will handle it
				//Table Dropdown IDs

				for(int d=0; d<pChangedData->aryChangedDropdownIDs.GetSize(); d++) {

					EMRInfoChangedDropdown *pChangedDropdown = pChangedData->aryChangedDropdownIDs.GetAt(d);

					long nOldDropdownID = pChangedDropdown->nOldDropdownID;
					long nNewDropdownID = pChangedDropdown->nNewDropdownID;

					BOOL bFound = FALSE;
					for (long j=0; j<m_arTableElements.GetSize() && !bFound; j++) {

						TableElement te = m_arTableElements[j];
						if (te.m_pColumn->nID == nOldDataID) {
							te.m_pColumn->nID = nNewDataID;
							bFound = TRUE;
						}
					}
				}
				*/
			}

			// (z.manning, 02/07/2008) - PLID 28603 - Need to handle changed hot spot IDs as well
			for(int nChangedHotSpotIDIndex = 0; nChangedHotSpotIDIndex < pChangedIDMap->aryChangedHotSpotIDs.GetSize(); nChangedHotSpotIDIndex++)
			{
				EMRInfoChangedHotSpot *pChangedHotSpot = pChangedIDMap->aryChangedHotSpotIDs.GetAt(nChangedHotSpotIDIndex);
				long nOldHotSpotID = pChangedHotSpot->nOldHotSpotID;
				long nNewHotSpotID = pChangedHotSpot->nNewHotSpotID;

				BOOL bFound = FALSE;
				for(int nHotSpotIndex = 0; nHotSpotIndex < m_aryImageHotSpots.GetSize() && !bFound; nHotSpotIndex++)
				{
					CEMRHotSpot *pSpot = m_aryImageHotSpots[nHotSpotIndex];
					if(pSpot->GetID() == nOldHotSpotID) {
						pSpot->SetID(nNewHotSpotID);
						bFound = TRUE;
					}
				}
			}

			//now update the state
			SyncContentAndStateWithChangedInfo(nOldEMRInfoID, nNewEMRInfoID, pChangedIDMap);
		}

	} // if((m_bIsTemplateDetail || m_nEMRDetailID == -1 || m_nEMRDetailID == nForceUpdateForDetailID) && m_nEMRInfoID == nOldEMRInfoID) {

	//don't forget the "last saved detail"
	// (j.jones 2010-11-11 17:06) - PLID 41457 - No! Do not update last saved detail, are you crazy?
	// Personally I like my drastically altered EMN details to both save and audit, but maybe that's just me?

	// Well then.. Historic confirmation that PLID 53550 should have been done a long while ago


	if(bNeedToFreeMap) {
		for(int i = 0; i < pChangedIDMap->aryChangedDataIDs.GetSize(); i++) {
			EMRInfoChangedData *pChangedData = pChangedIDMap->aryChangedDataIDs.GetAt(i);
			for(int j = 0; j < pChangedData->aryChangedDropdownIDs.GetSize(); j++) {
				EMRInfoChangedDropdown *pChangedDropdown = pChangedData->aryChangedDropdownIDs.GetAt(j);
				delete pChangedDropdown;
			}
			delete pChangedData;
		}
		delete pChangedIDMap;
	}

}

BOOL CEMNDetail::TryRecoverMissingLinkedDetails()
{
	// (c.haag 2007-08-13 09:47) - PLID 27049 - This function will go through all linked
	// table entries that are missing linked details, and try to fill them in based on what
	// is currently in our EMN.

	// If this were true, we would not need to fill in m_pLinkedDetail
	if (m_bOwnTopic || !m_pParentTopic)
		return FALSE;

	const int nElements = m_arTableElements.GetSize();
	BOOL bChangedTableElement = FALSE;
	for (int i=0; i < nElements; i++) {
		TableElement te = m_arTableElements[i];
		if (!te.m_strMissingLinkedDetailID.IsEmpty() && NULL == te.m_pLinkedDetail)
		{
			// Try to load the linked detail again
			te.LoadValueFromString(te.m_strMissingLinkedDetailID, m_pParentTopic->GetParentEMN());
			if (NULL != te.m_pLinkedDetail) {
				// We found it. Clear the m_strMissingLinkedDetailID string, but don't do
				// anything else. We'll let the caller decide in what other ways this detail
				// should be modified.
				te.m_strMissingLinkedDetailID.Empty();
				bChangedTableElement = TRUE;
			}
		}
	}
	return bChangedTableElement;
}

// (j.jones 2006-08-18 08:32) - PLID 22078 - same concept as SyncContentAndState, but
	// re-maps the state to match a changed info record
// (c.haag 2011-10-28) - PLID 46170 - Added EMRInfoID parameters
void CEMNDetail::SyncContentAndStateWithChangedInfo(long nOldEMRInfoID, long nNewEMRInfoID, EMRInfoChangedIDMap* pChangedIDMap)
{
	//the loaded info IDs should be remapped before this function is called, all we need
	//to do is remap the state, i.e. the selected items

	//go through the content variables, re-map IDs, and recreate the state
	switch(m_EMRInfoType) {
	case eitText:
	case eitNarrative:
	case eitSlider:
		//No state-related parameters.
		break;
	case eitSingleList:
	case eitMultiList:
		{
			CDWordArray arDataIds;
			GetSelectedValues(arDataIds);

			// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
			int i = 0;

			//remap the selected IDs
			for(i = 0; i < arDataIds.GetSize(); i++) {

				BOOL bFound = FALSE;

				for(int j = 0; j < pChangedIDMap->aryChangedDataIDs.GetSize() && !bFound; j++) {

					EMRInfoChangedData *pChangedData = pChangedIDMap->aryChangedDataIDs.GetAt(j);

					long nOldDataID = pChangedData->nOldDataID;
					long nNewDataID = pChangedData->nNewDataID;

					if(arDataIds[i] == (DWORD)nOldDataID) {
						arDataIds[i] = nNewDataID;
						bFound = TRUE;
					}
				}
			}

			//Now update the selectedness.			
			for(i = 0; i < m_arListElements.GetSize(); i++) {
				ListElement le = m_arListElements[i];
				bool bSelected = false;
				for(int j = 0; j < arDataIds.GetSize(); j++) {
					if(arDataIds[j] == (DWORD)le.nID) {
						bSelected = true;
					}
				}
				le.bIsSelected = bSelected;
				m_arListElements.SetAt(i,le);
			}

			CString strNewState;
			for(i = 0; i < arDataIds.GetSize(); i++)
				strNewState += FormatString("%li; ",arDataIds[i]);
			SetState(_bstr_t(strNewState));
		}
		break;
	case eitImage:
		{
			m_aisState->CreateFromSafeArrayVariant(m_varState);
			// (c.haag 2006-11-01 09:18) - PLID 23193 - We need to synchronize the image state with
			// the member variables that pertain to image content
			// (c.haag 2007-02-09 15:49) - PLID 23365 - If the state has no defined image type,
			// do NOT overwrite the existing content values
			if (itUndefined != m_aisState->m_eitImageTypeOverride) {
				m_eitBackgroundImageType = m_aisState->m_eitImageTypeOverride;
				m_strBackgroundImageFilePath = m_aisState->m_strImagePathOverride;
			}

			// (z.manning, 02/07/2008) - PLID 28603 - Need to handle hot spot IDs may have changed.
			CArray<long,long> arynSelectedHotSpotIDs;
			GetImageSelectedHotSpotIDs(arynSelectedHotSpotIDs);
			for(int nSelectedHotSpotIndex = 0; nSelectedHotSpotIndex < arynSelectedHotSpotIDs.GetSize(); nSelectedHotSpotIndex++)
			{
				BOOL bFound = FALSE;
				for(int nChangeIndex = 0; nChangeIndex < pChangedIDMap->aryChangedHotSpotIDs.GetSize() && !bFound; nChangeIndex++)
				{
					EMRInfoChangedHotSpot *pChangedHotSpot = pChangedIDMap->aryChangedHotSpotIDs.GetAt(nChangeIndex);

					long nOldHotSpotID = pChangedHotSpot->nOldHotSpotID;
					long nNewHotSpotID = pChangedHotSpot->nNewHotSpotID;

					if(arynSelectedHotSpotIDs.GetAt(nSelectedHotSpotIndex) == nOldHotSpotID) {
						arynSelectedHotSpotIDs[nSelectedHotSpotIndex] = nNewHotSpotID;
						bFound = TRUE;
					}
				}
			}
			// (z.manning, 02/07/2008) - PLID 28603 - Now update the selectedness.
			for(int nHotSpotIndex = 0; nHotSpotIndex < m_aryImageHotSpots.GetSize(); nHotSpotIndex++)
			{
				CEMRHotSpot *pSpot = m_aryImageHotSpots[nHotSpotIndex];
				bool bSelected = false;
				for(int nSelectedHotSpotIDIndex = 0; nSelectedHotSpotIDIndex < arynSelectedHotSpotIDs.GetSize()  && !bSelected; nSelectedHotSpotIDIndex++) {
					if(pSpot->GetID() == arynSelectedHotSpotIDs.GetAt(nSelectedHotSpotIDIndex)) {
						bSelected = true;
					}
				}
				pSpot->SetSelected(bSelected);
			}
			// (z.manning, 02/07/2008) - PLID 28603 - And finally update the state.
			m_aisState->SetSelectedHotSpots(arynSelectedHotSpotIDs);
			SetState(m_aisState->AsSafeArrayVariant());
		}
		break;
	case eitTable:
		{
			//Go through all the values, load the table element's value parameters.
			CString strState = AsString(m_varState);
			CString strNewState = "";
			if(!strState.IsEmpty()) {

				// (c.haag 2008-11-26 11:54) - PLID 32041 - If a user changed a formula field into a non-formula
				// field, then we need to clear out any state element data that was tied in with that field. To
				// do this, we need to build a map of all the old data ID's that were formula fields, and a map of
				// all the new data ID's that are formula fields. Then as we go along traversing the state, we check
				// if the element belongs to a field that used to be a formula field, and no longer is; and if that's true,
				// then we clear out that element.
				CMap<long,long,BOOL,BOOL> mapOldFormulaDataIDs;
				CMap<long,long,BOOL,BOOL> mapNewFormulaDataIDs;
				if (pChangedIDMap->aryChangedDataIDs.GetSize() > 0)
				{
					// (c.haag 2011-10-28) - PLID 46170 - Old and busted
					/*CArray<long,long> anOldDataIDs;
					CArray<long,long> anNewDataIDs;
					for(int i = 0; i < pChangedIDMap->aryChangedDataIDs.GetSize(); i++) {
						EMRInfoChangedData *pChangedData = pChangedIDMap->aryChangedDataIDs.GetAt(i);
						anOldDataIDs.Add(pChangedData->nOldDataID);
						anNewDataIDs.Add(pChangedData->nNewDataID);
					}*/
					
					// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
					_RecordsetPtr prs = CreateParamRecordset(

						// (c.haag 2011-10-28) - PLID 46170 - Old and busted
						//"SELECT ID, Formula FROM EmrDataT WHERE ID IN ({INTARRAY});\r\n"
						//"SELECT ID, Formula FROM EmrDataT WHERE ID IN ({INTARRAY});",
						//anOldDataIDs, anNewDataIDs

						// (c.haag 2011-10-28) - PLID 46170 - New hotness
						"SELECT ID FROM EmrDataT WHERE EmrInfoID = {INT} AND Formula <> ''\r\n"
						"SELECT ID FROM EmrDataT WHERE EmrInfoID = {INT} AND Formula <> ''",
						nOldEMRInfoID, nNewEMRInfoID
						);
					while (!prs->eof) {
						mapOldFormulaDataIDs.SetAt(AdoFldLong(prs, "ID"), TRUE);
						prs->MoveNext();
					}
					prs = prs->NextRecordset(NULL);
					while (!prs->eof) {
						mapNewFormulaDataIDs.SetAt(AdoFldLong(prs, "ID"), TRUE);
						prs->MoveNext();
					}
				} else {
					// There are no ID's to map
				}

				// (c.haag 2007-08-18 10:54) - PLID 27112 - Use an iterator to do the state traversal
				CEmrTableStateIterator etsi(strState);
				long X,Y,nEmrDetailImageStampID,nEmrDetailImageStampPointer, nStampID;
				CString strData;
				// (z.manning 2010-02-18 09:41) - PLID 37427 - Added EmrDetailImageStampID
				while (etsi.ReadNextElement(X,Y,strData,nEmrDetailImageStampID,nEmrDetailImageStampPointer,nStampID))
				{
					//now remap the X and Y IDs

					// (c.haag 2008-11-26 12:04) - PLID 32041 - See if this was a formula cell before the info item changed
					BOOL bWasFormulaCell = FALSE;
					if (!mapOldFormulaDataIDs.Lookup(X,bWasFormulaCell)) {
						mapOldFormulaDataIDs.Lookup(Y,bWasFormulaCell);
					}

					for(int i = 0; i < pChangedIDMap->aryChangedDataIDs.GetSize(); i++) {

						EMRInfoChangedData *pChangedData = pChangedIDMap->aryChangedDataIDs.GetAt(i);

						long nOldDataID = pChangedData->nOldDataID;
						long nNewDataID = pChangedData->nNewDataID;

						if(X == nOldDataID) {
							X = nNewDataID;
						}
						else if(Y == nOldDataID) {
							Y = nNewDataID;
						}
					}

					TableElement te;
					//TES 3/17/2010 - PLID 37530 - Pass in -1 for the SourceStampID and SourceStampIndex
					TableRowID rowid(X,nEmrDetailImageStampID,nEmrDetailImageStampPointer,-1,-1);
					if(GetTableElement(&rowid, Y, te)) {

						// (c.haag 2008-11-26 12:05) - PLID 32041 - See if this is a formula cell now
						BOOL bIsFormulaCell = FALSE;
						if (!mapNewFormulaDataIDs.Lookup(X,bIsFormulaCell)) {
							mapNewFormulaDataIDs.Lookup(Y,bIsFormulaCell);
						}

						// (c.haag 2008-11-26 12:06) - PLID 32041 - If this used to be a formula cell,
						// clear out the value of te.
						if (bWasFormulaCell && !bIsFormulaCell) {
							te.LoadValueFromString("", NULL);
						}
						else if(m_bOwnTopic || !m_pParentTopic) {
							//The parent won't have other details, so don't pass it in.
							te.LoadValueFromString(strData, NULL);
						}
						else {
							te.LoadValueFromString(strData, m_pParentTopic->GetParentEMN());
						}

						//remap dropdown IDs
						// (c.haag 2008-01-11 17:23) - PLID 17936 - We now do this per-ID
						const long nIDCount = te.m_anDropdownIDs.GetSize();
						CString strTextValue;
						BOOL bWasDropdownNowText = FALSE;
						for (int nIDIndex = 0; nIDIndex < nIDCount; nIDIndex++)
						{
							long nDropdownID = te.m_anDropdownIDs.GetAt(nIDIndex);
							
							if(nDropdownID != -1)
							{
								BOOL bFound = FALSE;

								for(int i = 0; i < pChangedIDMap->aryChangedDataIDs.GetSize() && !bFound; i++)
								{
									EMRInfoChangedData *pChangedData = pChangedIDMap->aryChangedDataIDs.GetAt(i);
									if(pChangedData->nOldListType == LIST_TYPE_DROPDOWN && pChangedData->nNewListType == LIST_TYPE_TEXT) {
										// (z.manning 2011-04-27 13:20) - PLID 37604 - We now support changing dropdown columns to
										// text type columns so we need to know if that's the case for this list element.
										bWasDropdownNowText = TRUE;
									}

									for(int j = 0; j < pChangedData->aryChangedDropdownIDs.GetSize() && !bFound; j++)
									{
										EMRInfoChangedDropdown *pChangedDropdown = pChangedData->aryChangedDropdownIDs.GetAt(j);

										long nOldDropdownID = pChangedDropdown->nOldDropdownID;
										long nNewDropdownID = pChangedDropdown->nNewDropdownID;

										if(nDropdownID == nOldDropdownID) {
											te.m_anDropdownIDs.SetAt(nIDIndex, (nDropdownID = nNewDropdownID) );
											if(!strTextValue.IsEmpty()) {
												strTextValue += ", ";
											}
											strTextValue += pChangedDropdown->strOldData;
											bFound = TRUE;
										}
									}
								}
							}

						} // for (int nIDIndex = 0; nIDIndex < nIDCount; nIDIndex++) {

						if(bWasDropdownNowText) {
							// (z.manning 2011-05-02 10:57) - PLID 37604 - It is possible that this function is being called before
							// we reloaded the content for this detail such that it still this this column is a dropdown even though
							// we know it was changed to text. Rather than ensuring load content is called before this (which would
							// mean it could end up being called twice) let's just tell the column what its new type is.
							te.m_pColumn->nType = LIST_TYPE_TEXT;

							// (z.manning 2011-04-27 13:21) - PLID 37604 - If this table element's column used to be a dropdown
							// but now is text then let's get rid of the dropdown IDs and set the text value to what the output
							// text was when it was a dropdown.
							te.m_anDropdownIDs.RemoveAll();
							te.LoadValueFromString(strTextValue, GetParentEMN());
						}

						// (c.haag 2008-03-05 09:59) - PLID 17936 - Disregard the following code comment. That and
						// the code were necessary due to a bug in the EMR item edit dialog allowing a user to 
						// delete a dropdown item that was in use. However, I'm keeping this code commented out
						// because, some day, this might resurface. It's also educational for the tester to see
						// how exactly EmrTableDropdownInfoT and te.m_anDropdownIDs are related.
						//
						// Also, if we ever need to restore this code, keep in mind the query is a huge speed hit.

						/*
						// (c.haag 2008-03-04 09:36) - PLID 17936 - Right now, te.m_anDropdownIDs has all the new
						// EmrTableDropdownInfoT ID's. Now, just because every old ID has a corresponding new ID, 
						// that doesn't mean they're actually in data! The old implementation never checked for this
						// as it should have. We will do so now. First, run a query to find all the ID's that DO exist.
						// Second, go through the array of selected items to see which ones weren't in the query, and
						// remove them from the array.
						//
						// To reproduce this: Pop up a table with a dropdown column in it, then make a dropdown selection
						// that didn't exist before, then edit the item on the fly and delete the selection.
						//
						if (te.m_anDropdownIDs.GetSize() > 0) {
							_RecordsetPtr prs = CreateRecordset("SELECT ID FROM EmrTableDropdownInfoT WHERE ID IN (%s)", ArrayAsString(te.m_anDropdownIDs));
							CMap<long,long,BOOL,BOOL> mapExistingIDs;
							while (!prs->eof) {
								mapExistingIDs.SetAt(AdoFldLong(prs, "ID"), TRUE);
								prs->MoveNext();
							}
							prs->Close();
							for (int nIndex=0; nIndex < te.m_anDropdownIDs.GetSize(); nIndex++) {
								long nDropdownID = te.m_anDropdownIDs[nIndex];
								BOOL bDummy;
								if (!mapExistingIDs.Lookup(nDropdownID, bDummy)) {
									// If we get here, the selection was deleted from the master
									// selection list for the info item when it was changed. Quietly
									// drop the selection.
									te.m_anDropdownIDs.RemoveAt(nIndex);
									nIndex--;
								}
							}
						}*/

						SetTableElement(te);

						//recreate the new state
						// (c.haag 2007-08-18 08:58) - PLID 27111 - Append the state with the next element
						// (c.haag 2007-08-24 15:06) - PLID 27168 - te.GetValueAsString() returns an
						// UNformatted element, so use AppendTableStateWithUnformattedElement.
						AppendTableStateWithUnformattedElement(strNewState, X, Y, te.GetValueAsString(), nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID);
					}
				}

				SetState(_bstr_t(strNewState));
			}
		}
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}

void CEMNDetail::SetInkAdded()
{
	m_bInkHasBeenAdded = TRUE;
}

void CEMNDetail::SetInkErased()
{
	m_bInkHasBeenErased = TRUE;
}

void CEMNDetail::SetImageTextAdded()
{
	m_bImageTextHasBeenAdded = TRUE;
}

void CEMNDetail::SetImageTextRemoved()
{
	m_bImageTextHasBeenRemoved = TRUE;
}

// (r.gonet 05/02/2012) - PLID 49946 - For auditing
void CEMNDetail::SetImageStampModified()
{
	m_bImageTextHasBeenModified = TRUE;
}

// (j.jones 2011-07-22 17:26) - PLID 43504 - added optional connection pointer
BOOL CEMNDetail::GetIsOnLockedAndSavedEMN(OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	_ConnectionPtr pCon;
	if(lpCon) {
		pCon = lpCon;
	}
	else {
		pCon = GetRemoteData();
	}

	BOOL bIsLockedAndSaved = FALSE;
	{
		if(m_pParentTopic) {
			CEMN *pEMN = m_pParentTopic->GetParentEMN();
			if(pEMN) {
				bIsLockedAndSaved = pEMN->IsLockedAndSaved();
			}
			else {
				//no parent EMN? We have to find out for sure from data
				// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
				// (j.jones 2011-07-22 17:26) - PLID 43504 - use our passed-in connection
				if(!m_pParentTopic->IsTemplate() && m_pParentTopic->GetID() != -1
					&& ReturnsRecordsParam(pCon, "SELECT ID FROM EMRTopicsT WHERE ID = {INT} AND EMRID IN (SELECT ID FROM EMRMasterT WHERE Status = 2)", m_pParentTopic->GetID())) {
					bIsLockedAndSaved = TRUE;
				}
			}
		}
		else {
			//no parent topic? We have to find out for sure from data
			// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
			// (j.jones 2011-07-22 17:26) - PLID 43504 - use our passed-in connection
			// (a.walling 2013-05-30 14:04) - PLID 56944 - Missing a WHERE in the query below
			if(!m_bIsTemplateDetail && m_nEMRDetailID != -1
				&& ReturnsRecordsParam(pCon, "SELECT ID FROM EMRDetailsT WHERE ID = {INT} AND EMRID IN (SELECT ID FROM EMRMasterT WHERE Status = 2)", m_nEMRDetailID)) {
				bIsLockedAndSaved = TRUE;
			}
		}
	}

	return bIsLockedAndSaved;
}

BOOL CEMNDetail::IsActiveInfo()
{
	//TES 12/8/2006 - First off, if we're a template detail, then we're always valid by definitions.
	if(m_bIsTemplateDetail)
		return TRUE;
	
	//Next, if our cached value is valid, just use it.
	if(m_bIsActiveInfoValid)
		return m_bIsActiveInfo;

	//Nope?  Well, we better look it up in data then.
	// (a.walling 2010-10-19 09:45) - PLID 40965 - Use ReturnsRecordsParam
	m_bIsActiveInfo = ReturnsRecordsParam("SELECT ID FROM EmrInfoMasterT WHERE ID = {INT} AND ActiveEmrInfoID = {INT}", m_nEMRInfoMasterID, m_nEMRInfoID);
	//Our cached value is valid now.
	m_bIsActiveInfoValid = TRUE;
	//Return it.
	return m_bIsActiveInfo;
}

// (j.jones 2007-08-29 17:09) - PLID 27057 - need to know if the detail even knows if it is the active info
BOOL CEMNDetail::IsActiveInfoValid()
{
	if(m_bIsTemplateDetail)
		return TRUE;
	
	return m_bIsActiveInfoValid;
}

// (j.jones 2007-08-29 17:11) - PLID 27057 - added so we can update the m_bIsActiveInfo value externally
void CEMNDetail::SetIsActiveInfo(BOOL bIsActive)
{
	m_bIsActiveInfo = bIsActive;
	//we know our info is valid, so set it as such
	m_bIsActiveInfoValid = TRUE;	
}

void CEMNDetail::RefreshIsActiveInfo()
{
	m_bIsActiveInfoValid = FALSE;
}

// (j.jones 2007-01-15 11:44) - PLID 24027 - supported SourceDetailID
void CEMNDetail::SetSourceDetailID(long nSourceDetailID)
{
	m_sai.nSourceDetailID = nSourceDetailID;
}

void CEMNDetail::SetSourceDetail(CEMNDetail *pSourceDetail)
{
	m_sai.pSourceDetail = pSourceDetail;

	//also set the ID, if we have one
	if(pSourceDetail) {
		if(m_bIsTemplateDetail && pSourceDetail->m_nEMRTemplateDetailID != -1) {
			m_sai.nSourceDetailID = pSourceDetail->m_nEMRTemplateDetailID;
		}
		else if(!m_bIsTemplateDetail && pSourceDetail->m_nEMRDetailID != -1) {
			m_sai.nSourceDetailID = pSourceDetail->m_nEMRDetailID;
		}
	}
}

void CEMNDetail::ApplyEmrLinks()
{
	if(!m_pParentTopic) {
		//If we haven't been attached to a topic yet, then by extension we haven't been attached to an EMR, so we can't 
		// find any links.
		return;
	}
	//TES 1/24/2007 - PLID 24377 - On second thought, let's just have CEMR handle this.  That way, it can deal with 
	// cases where details are mass-added.
	m_pParentTopic->GetParentEMN()->GetParentEMR()->ApplyEmrLinks(this);
	/*///TES 1/23/2007 - PLID 24377 - If we're on a multi-select list, go through all our unchecked lists, and find out if
	// any data elements that they are linked to are already checked on this EMR.
	if(m_EMRInfoType == eitMultiList)
	{
		BOOL bStateChanged = FALSE;
		//Make sure that our array of elements has been generated.
		LoadContent();
		for(int i = 0; i < m_arListElements.GetSize(); i++) {
			ListElement le = m_arListElements[i];
			if(!le.bIsSelected && !le.bIsLabel) {
				//Load all of the data elements that are linked to this one in an array.
				//NOTE: EmrDataIDs are in fact considered to be "linked" to themselves.
				_RecordsetPtr rsLinks = CreateRecordset("SELECT EmrDataID FROM EmrItemLinkedDataT WHERE EmrLinkID IN "
					" (SELECT EmrLinkID FROM EmrItemLinkedDataT WHERE EmrDataID = %li)", le.nID);
				CArray<long,long> arLinkedIDs;
				while(!rsLinks->eof) {
					arLinkedIDs.Add(AdoFldLong(rsLinks, "EmrDataID"));
					rsLinks->MoveNext();
				}
				rsLinks->Close();
				if(arLinkedIDs.GetSize() > 0) {//Don't bother checking if there are no links.
					if(m_pParentTopic->GetParentEMN()->GetParentEMR()->IsAnyItemChecked(arLinkedIDs)) 
					{
						//An item that we're linked to is checked, so we need to be checked.
						le.bIsSelected = TRUE;
						m_arListElements.SetAt(i,le);					
						//We've now changed our state, so we'll need to update some things once we're done.
						bStateChanged = TRUE;
					}
				}
			}
		}

		if(bStateChanged) {
			//Generate our new state.
			CString strNewState;
			for(int i = 0; i < m_arListElements.GetSize(); i++) {
				if(m_arListElements[i].bIsSelected) {
					strNewState += AsString(GetListElement(i).nID) + "; ";
				}
			}
			ASSERT(strNewState.GetLength() > 2);//We couldn't have gotten here without something being checked.
			//Trim the last "; "
			strNewState = strNewState.Left(strNewState.GetLength()-2);
			
			SetState(_bstr_t(strNewState));
			m_pParentTopic->HandleDetailStateChange(this);

		}
	}*/
}

CRect CEMNDetail::GetClientArea()
{
	//TES 2/7/2007 - PLID 18159 - If our AdvDlg exists, use its actual size, otherwise use the default.
	if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
		CRect rc;
		m_pEmrItemAdvDlg->GetClientRect(&rc);
		m_pEmrItemAdvDlg->ClientToScreen(&rc);
		m_pEmrItemAdvDlg->GetParent()->SendMessage(NXM_CONVERT_RECT_FOR_DATA, (WPARAM)&rc);
		return rc;
	}
	else {
		return m_rcDefaultClientArea;
	}
}

// (a.walling 2011-05-25 17:57) - PLID 43847 - This is now no longer a property of the detail.
//void CEMNDetail::SetSizeImageToDetail(BOOL bSizeImageToDetail)
//{
//	m_bSizeImageToDetail = bSizeImageToDetail;
//}
//
//BOOL CEMNDetail::GetSizeImageToDetail()
//{
//	return m_bSizeImageToDetail;
//}

/* (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore zoom and pan offsets.
// (r.gonet 05/31/2011) - PLID 43896 - Sets the current zoom level of the image
void CEMNDetail::SetZoomLevel(double dZoomLevel)
{
	m_aisState.m_dZoomLevel = dZoomLevel;
	_variant_t varNewState = m_aisState.AsSafeArrayVariant();
	if(m_pParentTopic->GetParentEMN()->GetStatus() != 2) {
		RequestStateChange(varNewState);
	}
}

double CEMNDetail::GetZoomLevel()
{
	return m_aisState.m_dZoomLevel;
}

// (r.gonet 05/31/2011) - PLID 43896 - Sets the left viewport offset
void CEMNDetail::SetOffsetX(long nOffsetX)
{
	m_aisState.m_nOffsetX = nOffsetX;
	_variant_t varNewState = m_aisState.AsSafeArrayVariant();
	if(m_pParentTopic->GetParentEMN()->GetStatus() != 2) {
		RequestStateChange(varNewState);
	}
}

long CEMNDetail::GetOffsetX()
{
	return m_aisState.m_nOffsetX;
}

// (r.gonet 05/31/2011) - PLID 43896 - Sets the top viewport offset
void CEMNDetail::SetOffsetY(long nOffsetY)
{
	m_aisState.m_nOffsetY = nOffsetY;
	_variant_t varNewState = m_aisState.AsSafeArrayVariant();
	if(m_pParentTopic->GetParentEMN()->GetStatus() != 2) {
		RequestStateChange(varNewState);
	}
}

long CEMNDetail::GetOffsetY()
{
	return m_aisState.m_nOffsetY;
}
*/

// (a.walling 2007-04-06 12:26) - PLID 25454
long CEMNDetail::GetID()
{
	return m_nEMRDetailID;
}

// (j.jones 2012-11-15 11:01) - PLID 52819 - added GetTemplateDetailID, only needed on templates
long CEMNDetail::GetTemplateDetailID()
{
	return m_nEMRTemplateDetailID;
}

// (a.walling 2007-04-05 16:55) - PLID 25454 - Return this detail's HTML
CString CEMNDetail::GetHTML(BOOL bCheckSpawnedOutput /* = FALSE*/)
{
	try {
		// (a.walling 2008-10-23 09:49) - PLID 27552 - if bCheckSpawnedOutput, will return empty if we are a subdetail
		if (bCheckSpawnedOutput) {
			if (IsSubDetail()) {
				return "";
			}
		}

		CString strHTML;
		
		strHTML.Preallocate(0x1000);

		CString str;
		long nDetailID = m_nEMRDetailID;
		BOOL bUnsaved = (nDetailID == -1); // an unsaved detail will have an id of -1!
		if (bUnsaved) {
			nDetailID = reinterpret_cast<long>(this); // cast ourself (CEMNDetail*) to a long pointer.
		}

		// (a.walling 2007-10-18 15:34) - PLID 27664 - Details never need to ignore unsaved, the last saved
		// state stored in the topic object's cached HTML string.

		CString strPointer = bUnsaved ? "PT" : "ID"; // PT for pointer, ID for ID

		// (a.walling 2007-08-23 13:36) - PLID 27166 - need to set a class so we know
		// whether this detail has data or not (and can reflect that via a style)
		// (a.walling 2007-10-19 15:50) - PLID 27166 - this was an incorrect assumption
		// We need to use IsStateSet to be consistent with everywhere else in practice

		// (a.walling 2007-12-14 17:05) - PLID 28354
		long nHideDetails = GetRemotePropertyInt("EMRPreview_HideDetails", g_dhPreviewDisplayHideDefaults, 0, "<None>", true);

		BOOL bHideEmptyDetails = (nHideDetails & dhEmpty) ? TRUE : FALSE;
		BOOL bHideEmptyDetailsForPrint = (nHideDetails & dhEmptyPrint) ? TRUE : FALSE;
		
		BOOL bHideNarrativeDetails = (nHideDetails & dhNarrative) ? TRUE : FALSE;
		BOOL bHideNarrativeDetailsForPrint = (nHideDetails & dhNarrativePrint) ? TRUE : FALSE;

		EmrInfoType eitActive = m_EMRInfoType;

#ifdef DEBUG
		if (eitActive == eitImage) {
			_ASSERTE(m_varInfoBackgroundImageFilePath.vt != VT_EMPTY);
			_ASSERTE(m_varInfoBackgroundImageType.vt != VT_EMPTY);
		}
#endif

		// (a.walling 2009-12-08 13:41) - PLID 36225 - LoadAllNarratives should have been called prior to this, which would ensure that our
		// state has been properly updated.
		/*
		if (eitActive == eitNarrative) {
			// (a.walling 2008-03-25 13:41) - PLID 29117 - When we changed this to use rich formatting, we bypassed GetDataOutput.
			// However there is a critical line of code in there that ensures the narrative's AdvDlg has been created. This is because
			// without doing that, then the NxRichText will not have any values stored in its header, but pre-existing values would
			// still be parsed correctly, hence the delay in figuring this out. All we should need to do is ensure the dialog exists.

			// (a.walling 2009-11-06 15:50) - PLID 36225 - This also needs to occur to ensure that the fields are all filled out. Previously
			// we did this prior to grabbing the data output; however, this fails to take into account checking for a 'blank' narrative,
			// where the only data in the narrative are merge fields. In some situations this can make us think it is blank, when it is not.
			// So since we will do this anyway, do it now before checking for the state and blank-ness of the item.
			CWnd *pTopicWnd = m_pParentTopic->GetInterface();
			EnsureEmrItemAdvDlg(pTopicWnd);
		}
		*/

		CString strClass, strDetailClass = "detail ";
		// (a.walling 2007-12-18 13:50) - PLID 28354 - Narratives are a special case
		if (eitActive != eitNarrative) {
			if (IsStateSet(NULL, true)) {
				strClass = "hasstate ";
				strDetailClass += "detailhasstate ";
			} else {
				strClass = "nostate ";
				strDetailClass += "detailnostate ";

				// (a.walling 2009-01-13 15:55) - PLID 32107 - Respect the inkless image merge setting
				// (z.manning 2009-03-31 15:31) - PLID 33716 - This preference only relates to merging
				// and this function is not used for merging so we should not be checking this.
				/*BOOL bDontMergeInklessImages = TRUE;
				if (eitActive == eitImage) {
					bDontMergeInklessImages = GetRemotePropertyInt("EmnDontMergeInklessImages", 0, 0, GetCurrentUserName(), true) == 1;
				}*/

				if(bHideEmptyDetails) strDetailClass += "hidescreen ";
				if(bHideEmptyDetailsForPrint) strDetailClass += "hideprint ";
			}
		} else {
			// narratives
			
			// (a.walling 2007-12-14 16:53) - PLID 28354 - If there are any unfilled merge fields on a narrative
			// then IsStateSet() will be false. Otherwise it will be true. For our purposes, we want to always
			// show the narrative unless it is actually empty.

			if (IsStateSet(NULL, true)) {
				strClass = "hasstate ";
				strDetailClass += "detailhasstate ";
			} else {
				strClass = "nostate ";
				strDetailClass += "detailnostate ";
			}

			// now we have set the state variable appropriately. now apply our visibility styles
			// basically we will always show narratives unless they are actually blank.
			if (bHideEmptyDetails || bHideEmptyDetailsForPrint) {
				bool bBlank = true;
				if (m_varState.vt == VT_BSTR) {
					// (a.walling 2010-06-02 08:34) - PLID 38979 - use the 'merge' text, otherwise we will get the field names as part of the text!
					//TES 7/11/2012 - PLID 51500 - Differentiate between RTF and HTML narratives
					CString strPlainText;
					if(IsRtfNarrative()) {
						strPlainText = ConvertTextFormat(ConvertTextFormat(VarString(m_varState), tfNxRichText, tfRichTextForMerge), tfRichText, tfPlainText);
					}
					else {
						strPlainText = ConvertTextFormat(ConvertTextFormat(VarString(m_varState), tfHTML, tfHTMLForMerge), tfHTML, tfPlainText);
					}

					// (a.walling 2011-04-11 12:51) - PLID 42875 - This should consider a narrative to not be blank if it has any non-whitespace characters.
					unsigned char* uszPlainText = (unsigned char*)(LPCTSTR)strPlainText;
					unsigned char c;
					while (c = *uszPlainText++) {
						if (!::isspace(c)) {
							bBlank = false;
							break;
						}
					}
				}

				if (bBlank) {
					if(bHideEmptyDetails) strDetailClass += "hidescreen ";
					if(bHideEmptyDetailsForPrint) strDetailClass += "hideprint ";
				}
			}
		}


		// (a.walling 2007-12-14 17:09) - PLID 28354 - Check if we are used on a narrative
		
		// (a.walling 2008-11-19 13:55) - PLID 32101 - only need to check if we are a type that should be included in a narrative
		//TES 2/25/2010 - PLID 37535 - Allow tables that use the Smart Stamp format to be in narratives
		if ((bHideNarrativeDetailsForPrint && strDetailClass.Find("hideprint") == -1 || (bHideNarrativeDetails && strDetailClass.Find("hidescreen") == -1)) && m_pParentTopic && m_pParentTopic->GetParentEMN() && IsDetailLinkable(this)) {
			CArray<CEMNDetail*, CEMNDetail*> arNarratives;

			CEMN* pEMN = m_pParentTopic->GetParentEMN();

			for (int x = 0; x < pEMN->GetTotalDetailCount(); x++) {
				CEMNDetail* pPossibleNarrative = pEMN->GetDetail(x);

				if (pPossibleNarrative->m_EMRInfoType == eitNarrative) {
					arNarratives.Add(pPossibleNarrative);
				}
			}

			bool bFound = false;

			for (int i = 0; i < arNarratives.GetSize() && !bFound; i++) {
				CEMNDetail* pNarrative = arNarratives[i];

				if (pNarrative->GetStateVarType() != VT_BSTR) {
					// (a.walling 2008-02-22 17:59) - PLID 28354
					// if the vt is EMPTY, we are in the middle of preloading. This is for calculating
					// 'last saved' HTML for keeping the actual preview written to disk in sync with the
					// EMN as it exists in data, not necessarily how it exists in a partially-loaded or -saved
					// state in the interface.
					// so we can safely safe that this item is NOT used on a narrative. When the PostLoad is called,
					// these details will all be updated anyway.
					ASSERT(FALSE);
				} else {
					CArray<CEMNDetail*, CEMNDetail*> arDetails;
					pNarrative->GetLinkedDetails(arDetails, m_pParentTopic->GetParentEMN(), NULL, NULL, TRUE);

					for (int p = 0; p < arDetails.GetSize(); p++) {
						if (arDetails[p] == this) {
							// (a.walling 2008-07-01 09:21) - PLID 30571 - All items that will be hidden when printing
							// will now assume the grayed-out color previously associated with the now deprecated class
							// 'onnarrative'.
							//strDetailClass += "onnarrative ";

							bFound = true;
									
							break;
						}
					}

					// (a.walling 2012-03-14 22:51) - PLID 48896 - Details used as sentence output within a table dropdown that is used on a narrative 
					// should be considered included on that narrative, and display on the preview pane when printing / on screen as appropriate

					if (!bFound) {
						bool bHideIfIndirect = !!(nHideDetails & dhAlwaysHideIfIndirect);
						if (!bHideIfIndirect) {
							bHideIfIndirect = !!(GetPreviewFlags() & epfHideIfIndirect);
						}
						
						// (a.walling 2012-07-13 16:38) - PLID 48896
						if (bHideIfIndirect) {
							for (int p = 0; p < arDetails.GetSize() && !bFound; p++) {
								if (arDetails[p]->m_EMRInfoType != eitTable) {
									continue;
								}
								if (arDetails[p]->FindSpawnedDetail(this, true)) {
									bFound = true;
									break;
								}
							}
						}
					}

					if (bFound) {
						if(bHideNarrativeDetails && strDetailClass.Find("hidescreen") == -1) strDetailClass += "hidescreen ";
						if(bHideNarrativeDetailsForPrint && strDetailClass.Find("hideprint") == -1) strDetailClass += "hideprint ";
					}
				}
			}
		}

		// (a.walling 2008-02-08 09:12) - PLID 28857 - Use the most accurate info type
		if (eitActive == eitImage) {
			strDetailClass += "emnimage ";
		}

		// (a.walling 2008-06-30 17:03) - PLID 30571 - Hide if necessary
		if(GetPreviewFlags() & epfHideItem) {
			if (strDetailClass.Find("hideprint") == -1)
				strDetailClass += "hideprint ";
		}
				
		CString strTitleClass;
		if (GetPreviewFlags() & epfHideTitle) {
			//(r.wilson 1/23/2014) PLID 46427 - Using a new css class that hides the title using visibility: hidden; istead of display: none;
			// (a.walling 2014-05-01 13:01) - PLID 61258 - This causes all titles to retain their spacing! 
			// Reverting and addressing the underlying problem in PLID 62001
			//strTitleClass = "hideprintvisibility";
			strTitleClass = "hideprint";
		}

		// (a.walling 2008-07-01 11:27) - PLID 30571 - Check parent topics to see if we should hide ourselves
		if (strDetailClass.Find("hideprint") == -1) {
			CEMRTopic* pWatchTopic = m_pParentTopic;
			while (pWatchTopic) {
				if (pWatchTopic->GetPreviewFlags() & epfHideItem) {
					if (strDetailClass.Find("hideprint") == -1) strDetailClass += "hideprint ";
					break;
				}

				pWatchTopic = pWatchTopic->GetParentTopic();
			}
		}

		// (a.walling 2007-12-17 15:51) - PLID 28391
		m_bHTMLVisiblePrint = (strDetailClass.Find("hideprint") == -1) ? TRUE : FALSE;
		m_bHTMLVisible = (strDetailClass.Find("hidescreen") == -1) ? TRUE : FALSE;

		// (a.walling 2009-01-07 14:54) - PLID 31961 - floating elements

		// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
		// (a.walling 2009-07-06 10:48) - PLID 34793 - This is all handled by the parent object now
		/*
		if (m_nPreviewFlags & epfColumnOne) {
			strDetailClass += "floatleft ";
		} else if (m_nPreviewFlags & epfColumnTwo) {
			strDetailClass += "floatright ";
		}
		*/

		// (a.walling 2009-07-06 10:10) - PLID 34793 - Clearing is deprecated
		/*
		if ((m_nPreviewFlags & epfClearNone) == epfClearNone) {
			strDetailClass += "clearnone ";	
		} else {
			if (m_nPreviewFlags & epfClearRight) {
				strDetailClass += "clearright ";
			}
			if (m_nPreviewFlags & epfClearLeft) {
				strDetailClass += "clearleft ";
			}
		}
		*/

		// (a.walling 2009-01-08 14:07) - PLID 32660 - Align text right
		if (m_nPreviewFlags & epfTextRight) {
			strDetailClass += "textright ";
		}

		// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
		if (m_nPreviewFlags & epfPageBreakAfter) {
			strDetailClass += "ffafter ";
		} else if (m_nPreviewFlags & epfPageBreakBefore) {
			strDetailClass += "ffbefore ";
		}

		strDetailClass.TrimRight();
		strClass.TrimRight();

		// (a.walling 2007-10-18 15:38) - PLID 25548 - Ensure the label text is escaped
		// use multiple classes
		// (a.walling 2007-10-29 08:55) - PLID 27166 - Put the class in here directly rather than replace a special token
		// (a.walling 2008-06-02 16:13) - PLID 27716 - Use the merge field override text if available
		CString strName;
		{
			CString strDetailLabel;
			// (b.cardillo 2012-05-02 20:28) - PLID 49255 - Prefix the name with an asterisk if it's required and not filled in
			if (this->IsRequired() && !this->IsStateSet()) {
				strDetailLabel = _T("* ");
			}
			// Append the detail label text
			strDetailLabel += GetMergeFieldOverride().IsEmpty() ? GetLabelText() : GetMergeFieldOverride();
			// Clean it for html
			strName = ConvertToHTMLEmbeddable(strDetailLabel);
		}

		// (a.walling 2009-08-03 10:46) - PLID 34542 - Toggle for detail dash prefix
		BOOL bPrefixDash = GetRemotePropertyInt("EMRPreviewPrefixDash", TRUE, 0, "<None>", true);

		// (a.walling 2008-04-30 16:01) - PLID 29853 - Include a symbol (dash) in front of every detail.
		//&ndash;
		//&raquo;
		// (a.walling 2009-08-03 10:53) - PLID 34542 - No dash if preference is set
		str.Format(
			"<div class='%s' id='DetailDiv%s%li'><span class='%s'>\r\n"
			"<span class='%s'><h3><span style='font-size:1.0em; font-weight:bolder;'>%s</span><a name='DetailAnchor%s%li' href='nexemr://detail%s/?%li'>%s</a></h3></span>\r\n", 
			strDetailClass, strPointer, nDetailID, strClass, 
			strTitleClass, bPrefixDash ? "&ndash; " : "", strPointer, nDetailID, strPointer, nDetailID, strName);
		strHTML += str;

		CString strDetailHTML;

		// special handling for table output
		if (eitActive == eitTable) {
			try {
				strDetailHTML = GetTableDataOutputHTMLRaw();
			} NxCatchAllCall("Error generating table HTML", strDetailHTML = "Error");
		}
		else if (eitActive == eitImage)
		{
			try
			{
				if (m_pParentTopic && m_pParentTopic->GetParentEMN())
				{
					CEmrPreviewImageInfo pPreviewImageInfo;

					CString strTempImageFileOutput = m_pParentTopic->GetParentEMN()->GetDataOutput(this, NULL, false, false, NULL, ecfParagraph, &pPreviewImageInfo);

					if (strTempImageFileOutput.IsEmpty() || strTempImageFileOutput.CompareNoCase("<Missing Image>") == 0) {
						// failed to save the image
						// (a.walling 2009-01-13 14:45) - PLID 32107 - This also occurs if no image is set. We should not output this invalid html.
						strDetailHTML = "";
					}
					else
					{
						CStringArray arystrImageFiles;
						if(Is3DImage()) {
							// (z.manning 2011-09-26 11:13) - PLID 45664 - 3D models return multiple image filenames delimited by a pipe
							SplitString(strTempImageFileOutput, "|", &arystrImageFiles);
						}
						else {
							arystrImageFiles.Add(strTempImageFileOutput);
						}

						// we have an image file!!
						// (z.manning 2011-09-26 11:12) - PLID 45664 - We may have multiple image files to handle here
						for(int nImageFileIndex = 0; nImageFileIndex < arystrImageFiles.GetCount(); nImageFileIndex++)
						{
							CString strTempImageFile = arystrImageFiles.GetAt(nImageFileIndex);
							m_pParentTopic->GetParentEMN()->AddTempImageFileReference(strTempImageFile);

							// (a.walling 2007-10-12 13:45) - PLID 25605 - Use a function with exception handling for the image width expression

							// (a.walling 2009-07-06 14:26) - PLID 34793 - Unfortunately we have to remove the safe body with and just always
							// go 100% if we are in a column (this is ignored if we are IE7+)
							// (a.walling 2014-05-02 16:57) - PLID 62001 - This was already dead code; commenting
							/*BOOL bInColumn = FALSE;
							if (m_nPreviewFlags & (epfColumnOne|epfColumnTwo)) {
								bInColumn = TRUE;
							} else {
								CEMRTopic* pCheckColumnTopic = m_pParentTopic;
								while (pCheckColumnTopic != NULL && bInColumn == FALSE) {
									if (pCheckColumnTopic->m_nPreviewFlags & (epfColumnOne|epfColumnTwo)) {
										bInColumn = TRUE;
									}

									pCheckColumnTopic = pCheckColumnTopic->GetParentTopic();
								}
							}*/

							// (a.walling 2007-12-18 10:30) - PLID 28401 - Include a linebreak
							// (a.walling 2009-07-06 14:32) - PLID 34793 - This is the proper way to do it, now that IE7 is getting more popular
							// (a.walling 2013-08-15 10:38) - PLID 58049 - Remove nasty IE6 workarounds in EMR Preview Pane
							// (a.walling 2014-05-02 16:57) - PLID 62001 - Adding width:100% ensures the image fills the parent element that hasLayout; this worked before (incorrectly) due to a bug in IE7
							// (a.walling 2014-07-10 09:59) - PLID 62821 - Use the pPreviewImageInfo.nMaxWidth as the max width, set to 100% to not upscale if using the whole image.
							CString strMaxWidth;
							if (pPreviewImageInfo.nMaxWidth > 0) {
								strMaxWidth.Format("%lupx", pPreviewImageInfo.nMaxWidth);
							}
							else {
								strMaxWidth = "100%";
							}
							CString strCurrentImageHTML = FormatString("<br></br><img src=\"%s?%lu\" alt=\"%s\" style=\"border-style:none; width:100%%; max-width:%s;\"></img>", FileUtils::GetFileName(strTempImageFile), GetTickCount(), ConvertToHTMLEmbeddable(GetLabelText()), strMaxWidth);
							
							CString strImageAnchor;
							strImageAnchor.Format("<a name='DetailImageAnchor%s%li' href='nexemr://detail%s/?%li'>", strPointer, nDetailID, strPointer, nDetailID);

							strCurrentImageHTML = strImageAnchor + strCurrentImageHTML + "</a>";

							strDetailHTML += strCurrentImageHTML;
						}
					}
				}
				else {
					strDetailHTML = "<strong>NO PARENT EMN</strong>";
				}
			}
			catch(CNxPersonDocumentException *pException) {
				strDetailHTML = "Error";

				// (d.thompson 2013-07-01) - PLID 13764 - Specially handle exceptions regarding the person documents
				CPatientDocumentStorageWarningDlg dlg(pException->m_strPath);
				//No longer need the exception, clean it up
				pException->Delete();

				//Inform the user
				dlg.DoModal();
			} NxCatchAllCall("Error generating image HTML", strDetailHTML = "Error");
		}
		else if (eitActive == eitNarrative)
		{
			try
			{
				// (a.walling 2010-03-26 12:44) - PLID 29099 - Option to enable/disable interactive narratives
				bool bInteractiveNarrative = GetRemotePropertyInt("EMRPreviewEnableInteractiveNarrative", TRUE, 0, "<None>", true) ? true : false;

				// (a.walling 2010-03-24 20:20) - PLID 29293 - Send the override font boolean
				bool bOverrideDefaultFont = GetRemotePropertyInt("EMRPreviewOverrideDefaultNarrativeFont", TRUE, 0, "<None>", true) ? true : false;

				// (c.haag 2012-04-02) - PLID 49358 - Formatting for HTML narratives
				// (j.armen 2012-10-10 09:42) - PLID 53099 - Load as HTML instead of XML
				if (!IsRtfNarrative())
				{
					CString strDetailText = VarString(GetState(),"");

					CString strStatic = ConvertTextFormat(VarString(GetState(), ""), tfHTML, tfHTMLForMerge);

					// (j.armen 2012-12-07 10:38) - PLID 53904 - Only load the html here if we are showing interactive
					CString strActive;
					MSHTML::IHTMLDocument2Ptr pDoc;
					CoCreateInstance(CLSID_HTMLDocument, NULL, CLSCTX_INPROC_SERVER, IID_IHTMLDocument2, (void**)&pDoc);

					if(!pDoc)
						AfxThrowNxException("Invalid IHTMLDocument2Ptr in EMNDetail::GetHTML()");

					pDoc->write(Nx::SafeArray<VARIANT>::FromValue(_bstr_t("<body>" + strDetailText + "</body>")));

					MSHTML::IHTMLDocument3Ptr pDoc3 = pDoc;
					MSHTML::IHTMLElementCollectionPtr pCollection;

					// (z.manning 2013-09-11 12:28) - PLID 58473 - Special handling when the pref is set to not 
					// use the narrative's font.
					if(bOverrideDefaultFont)
					{
						// (z.manning 2013-09-11 12:29) - PLID 58473 - Loop through all elements and clear out the
						// font family (so it will go by the css instead).
						pDoc->get_all(&pCollection);
						for (long nElementIndex = 0; nElementIndex < pCollection->length; nElementIndex++) {
							MSHTML::IHTMLElementPtr pElement = pCollection->item(nElementIndex);
							pElement->style->fontFamily = g_cbstrNull;
						}

						// (z.manning 2013-09-11 12:29) - PLID 58473 - RTF narratives use percents for all the font sizes
						// when they are converted to HTML.
						ConvertHtmlElementsFixedPointFontSizeToPercentForTag(pDoc3, "SPAN", 12);
						ConvertHtmlElementsFixedPointFontSizeToPercentForTag(pDoc3, "A", 12);

						strStatic = VarString(pDoc->body->innerHTML);
						strStatic = ConvertTextFormat(strStatic, tfHTML, tfHTMLForMerge);
					}

					if(bInteractiveNarrative)
					{
						pCollection = pDoc3->getElementsByTagName("A");
						for (long i=0; i < pCollection->length; i++)
						{
							MSHTML::IHTMLElementPtr pElement = pCollection->item(i);
							//TES 7/9/2012 - PLID 49358 - This is a detail link, so we need to convert it to a preview-style link.  The preview will just be 
							// looking at the href and field attributes.
							pElement->setAttribute("href", FormatBstr("nexemr://narrative%s/?%li/%li", strPointer, nDetailID, i), 0);
						}

						// (j.armen 2012-10-10 09:43) - PLID 53099 - P elements need to have a margin of 0.  The control handles this via a style.
						//	Here, we will just apply manually.
						pCollection = pDoc3->getElementsByTagName("P");
						for (long i=0; i < pCollection->length; i++)
						{
							MSHTML::IHTMLElementPtr pElement = pCollection->item(i);
							pElement->style->margin = "0px";
						}

						// (j.armen 2012-11-27 15:23) - PLID 53904 - Save our active HTML at this point.  
						strActive = VarString(pDoc->body->innerHTML);
					}
					else
					{
						// (j.armen 2012-11-27 15:26) - PLID 53904 - If we do not allow interactive narratives,
						//	then just set the active to be the static text
						strActive = strStatic;
					}

					//TES 7/9/2012 - PLID 49358 - And format it to go on the preview pane.
					// (j.armen 2012-11-12 09:59) - PLID 53692 - HTML Narratives have a default font of Times New Roman 
					//	that is not specifically declared
					// (z.manning 2013-09-11 12:27) - PLID 58473 - We need to adhere to the preference to ignore
					// the narrative font.
					CString strFontStyle = bOverrideDefaultFont ? "" : " style='FONT-FAMILY: Times New Roman;'";
					CString strStaticClass = bOverrideDefaultFont ? " class='defaultFont'" : "";
					strDetailHTML.Format(
						"<div class='narrativetext' id='nar%lu' "
						"onmouseover='ShowNarrativeLinks(this);' "
						"onmouseout='HideNarrativeLinks(this);' "
						">"
						"<span id='textStatic'%s%s>%s</span>"
						"<span id='textActive'%s class='noscreen noprint'>%s</span>"
						"</div>"
						, GetUniqueSessionNum()
						, strFontStyle, strStaticClass, strStatic
						, strFontStyle, strActive);
				}
				// we don't want to set 'allow html' variables to true for narratives because it will give us an RTF file, actually.
				else if (m_pParentTopic && m_pParentTopic->GetParentEMN())
				{
					// (a.walling 2010-03-26 12:44) - PLID 29099 - Gather the links for the narrative fields
					CMapStringToString mapInteractiveAnchors;
					if (bInteractiveNarrative) {
						CArray<NarrativeField> arNarrativeFields;
						CString strNxRichText = VarString(m_varState);

						strNxRichText.TrimRight();

						//TES 6/29/2012 - PLID 50854 - New function that handles HTML narratives
						GetNarrativeFieldArray(strNxRichText, arNarrativeFields);

						for (int i = 0; i < arNarrativeFields.GetSize(); i++) {
							NarrativeField nf = arNarrativeFields[i];

							// (a.walling 2010-04-01 10:44) - PLID 38013 - Consolidating some constant text strings
							if (nf.strField != NFIELD_SPAWNING_ITEM) {
								if (!m_pParentTopic->GetParentEMN()->GetGenericMergeFieldValue(nf.strField, nf.strValue, nf.bIsValid)) {
									CString strAnchor;
									strAnchor.Format("href='nexemr://narrative%s/?%li/%li' title='%s'", strPointer, nDetailID, i, ConvertToHTMLEmbeddable(nf.strField));

									mapInteractiveAnchors.SetAt(arNarrativeFields[i].strField, strAnchor);
								}
							}
						}
					}
					
					// (a.walling 2008-03-17 16:09) - PLID 29117 - Use the HTML-Formatted version now!
					//strDetailHTML += ConvertToHTMLEmbeddable(m_pParentTopic->GetParentEMN()->GetSentence(this, NULL, false, false));
					// (a.walling 2007-04-17 12:59) - PLID 25454 - Line breaks aren't literal in HTML.
					// (a.walling 2007-10-24 09:36) - PLID 27820 - This is handled by ConvertToHTMLEmbeddable now
					// strDetailHTML.Replace("\r\n", "<br/>");

					CString strStaticNarrativeHTML = ConvertTextFormat(ConvertTextFormat(VarString(GetState(), ""), tfNxRichText, tfRichTextForMerge), tfRichText, tfHTML, bOverrideDefaultFont);

					if (!mapInteractiveAnchors.IsEmpty()) {
						// (a.walling 2010-03-26 12:44) - PLID 29099 - Generate the interactive narrative HTML
						CString strInteractiveNarrativeHTML = ConvertTextFormat(VarString(GetState(), ""), tfNxRichText, tfHTMLInteractive, bOverrideDefaultFont, &mapInteractiveAnchors);

						// (a.walling 2010-03-31 08:58) - PLID 29099 - Using noscreen / noprint to avoid additional styles attached to hidescreen / hideprint
						strDetailHTML.Format("<div class='narrativetext' id='nar%lu' "
							"onmouseover=\"ShowNarrativeLinks(this);\" "
							"onmouseout=\"HideNarrativeLinks(this);\" "
							">"
							"<span id='textStatic'>%s</span>"
							"<span id='textActive' class='noscreen noprint'>%s</span>"
							"</div>", GetUniqueSessionNum(), strStaticNarrativeHTML, strInteractiveNarrativeHTML);
					} else {
						strDetailHTML = strStaticNarrativeHTML;
					}
				}
				else strDetailHTML = "<strong>NO PARENT EMN</strong>";
			} NxCatchAllCall("Error getting narrative sentence HTML", strDetailHTML = "Error");
		} else {
			try {
				if (m_pParentTopic && m_pParentTopic->GetParentEMN()) {
					strDetailHTML = m_pParentTopic->GetParentEMN()->GetSentence(this, NULL, true, true);
					// (a.walling 2007-10-18 13:48) - PLID 25548 - Need to ensure this is escaped correctly
					// (a.walling 2007-10-18 17:48) - PLID 25548 - This whole HTML escaping this is all messed up.
					// Will need to investigate further, perhaps in a seperate item.
					// (a.walling 2007-10-19 15:59) - PLID 27820 - Escaping should be the responsibility of GetSentence
					// strDetailHTML = strDetailHTML;
					// (a.walling 2007-04-17 12:59) - PLID 25454 - Line breaks aren't literal in HTML.
					// (a.walling 2007-10-24 09:36) - PLID 27820 - This is handled by ConvertToHTMLEmbeddable now
					// strDetailHTML.Replace("\r\n", "<br/>");
				}
				else strDetailHTML = "<strong>NO PARENT EMN</strong>";
			} NxCatchAllCall("Error getting detail sentence HTML", strDetailHTML = "Error");
		}

		// don't include an empty content span
		if (strDetailHTML.CompareNoCase("<Missing Image>") == 0) {
			strDetailHTML.Empty();
		} else if (!strDetailHTML.IsEmpty()) {
			strHTML += "<span class='content'>" + strDetailHTML + " </span>\r\n";
		}

		strHTML += "</span></div>\r\n";

		// (a.walling 2008-10-23 09:49) - PLID 27552 - now we have to check for any items that this item may have spawned
		if (ehiasHasNoInfoItems != GetHasInfoActionsStatus() || eitActive == eitSingleList || eitActive == eitMultiList || eitActive == eitImage) {
			CEMN* pEMN = m_pParentTopic ? m_pParentTopic->GetParentEMN() : NULL;
			if (pEMN) {
				CArray<CEMNDetail*,CEMNDetail*> arypSpawnedDetails;

				CArray<CEMNDetail*,CEMNDetail*> arypEMNDetails;
				pEMN->GenerateTotalEMNDetailArray(&arypEMNDetails);

				for (int i = 0; i < arypEMNDetails.GetSize(); i++) {
					CEMNDetail* pDetail = arypEMNDetails[i];
					CEMNDetail* pSourceDetail = pDetail->GetSourceDetail();
					if (pDetail != NULL && pDetail->GetPreviewFlags() & epfSubDetail) {
						if (pSourceDetail == this) {
							// hit!
							arypSpawnedDetails.Add(pDetail);
						} else if (pSourceDetail == NULL) {
							// check for ID
							long nSourceDetailID = pDetail->GetSourceDetailID();

							if (nSourceDetailID != -1 && nSourceDetailID == GetID()) {
								// hit!
								arypSpawnedDetails.Add(pDetail);
							}
						}
					}
				}

				if (arypSpawnedDetails.GetSize() > 0) {
					//TRACE("Detail %li (%s) -- found %li spawned details\n", GetID(), GetLabelText(), arypSpawnedDetails.GetSize());

					SortDetailArray(arypSpawnedDetails);

					// (a.walling 2008-10-23 09:50) - PLID 27552 - Display all subdetails within a SubDetailDiv (class subdetailbucket)
					// (a.walling 2009-01-07 15:45) - PLID 31961 - These will be handled separately from the floating parent element
					CString strSubDetails;
					strSubDetails.Format(
						"<div class='subdetailbucket' id='SubDetailDiv%s%li'>\r\n", strPointer, nDetailID);

					for (int d = 0; d < arypSpawnedDetails.GetSize(); d++) {
						strSubDetails += arypSpawnedDetails[d]->GetHTML();
					}

					strSubDetails += "</div>\r\n";

					strHTML += strSubDetails;

					if (!GetHTMLVisible() || !GetHTMLVisiblePrint()) {
						// (a.walling 2008-10-23 09:50) - PLID 27552 - If we are displaying any other items, then this
						// detail may be 'visible' in the sense that it has some output that we don't want hidden.
						for (int d = 0; d < arypSpawnedDetails.GetSize(); d++) {
							if (arypSpawnedDetails[d]->GetHTMLVisible()) {
								m_bHTMLVisible = TRUE;
							}
							if (arypSpawnedDetails[d]->GetHTMLVisiblePrint()) {
								m_bHTMLVisiblePrint = TRUE;
							}
						}
					}
				}
			}
		}

		return strHTML;
	} NxCatchAll("Error in CEMNDetail::GetHTML");

	return "<p>Error</p>";
}

//TES 2/22/2010 - PLID 37463 - Utility function used by GetTableDataOutputHTMLRaw(), when calculating the Smart Stamps output.
// (a.walling 2010-03-26 18:00) - PLID 37923 - Take a TableRow* rather than an index
// (z.manning 2010-07-29 11:57) - PLID 39842 - I renamed this function as it's no longer specific to smart stamps.
// (z.manning 2010-07-30 10:27) - PLID 39842 - Added bHtml and bIsForPreview
// (a.walling 2012-07-13 10:30) - PLID 51479 - Added pFallbackParentEMN to the table element data output functions
CString CEMNDetail::GetTableElementDataOutput(TableRow* pTr, TableColumn *pTc, CMap<__int64, __int64, int, int> &mapTable, bool bHtml, bool bForPreview, CEMN* pFallbackParentEMN)
{
	//TES 2/22/2010 - PLID 37463 - This code is essentially copied from the non-Smart-Stamp version of GetTableDataOutputHTMLRaw(),
	// except that it doesn't return HTML, and its simplified a bit because we know that SmartStamps tables can't flip their rows and columns.
	// (c.haag 2008-10-22 11:47) - PLID 31760 - Get the row and column
	//TableRow* ptr = GetRowPtr(nRowIndex);
	TableRow* ptr = pTr;
	TableElement te;
	TableRowID *pRowID = &ptr->m_ID;
	int nColumnID = pTc->nID;
	// (c.haag 2007-08-20 09:03) - PLID 27118 - We now use a utility function to pull the key
	__int64 nKey = GetTableElementKey(ptr, nColumnID);
	int nTableIndex;
	if (mapTable.Lookup(nKey, nTableIndex)) {
		// Success, we have a table element.
		GetTableElementByIndex(nTableIndex, te);
	} else {
		// c.haag's comment follows:
		// If this cell has no element, assign it a default. This should
		// be the responsibility of TableElement::GetValueAsVariant. You
		// might think "But te was never explicitly assigned a value from
		// this function!". ::GetTableElement never assigned one, so neither
		// should we. With regards to generating data output, we must strive
		// to be as consistent as possible with the old behavior.
		te.m_pRow = ptr;
		te.m_pColumn = pTc;
	}
	
	//(e.lally 2011-12-08) PLID 46471 - We need to check for allergies and current medication 
	//	checked status and ignore the element data if it is a current med or allergy and is unchecked.
	if((IsCurrentMedicationsTable() || IsAllergiesTable()) && !te.m_pRow->m_bIsCurrentMedOrAllergyChecked){
		return "";
	}
	// (a.walling 2012-07-13 10:28) - PLID 51479 - Pass in calculated fallback parent EMN
	return GetTableElementDataOutput(&te, bHtml, bForPreview, pFallbackParentEMN);
}

// (z.manning 2010-08-18 11:52) - PLID 39842 - Added an overload that takes a table element
// (a.walling 2012-07-13 10:30) - PLID 51479 - Added pFallbackParentEMN to the table element data output functions
CString CEMNDetail::GetTableElementDataOutput(TableElement *pte, bool bHtml, bool bForPreview, CEMN* pFallbackParentEMN)
{
	//now the HTML
	CStringArray saTempFiles;
	// thankfully, the temp file problem does not actually happen, since both
	// HTML params in GetSentence are true within GetValueAsOutput.

	CString strData;
	// (a.walling 2011-05-31 11:56) - PLID 42448
	CString strTableElementOutput = pte->GetValueAsOutput(this, bHtml, saTempFiles, bForPreview);
	// (z.manning 2010-07-29 15:34) - PLID 39842 - Does this column have its own sentence format?
	if(pte->m_pColumn->m_strLongForm.IsEmpty()) {
		strData = strTableElementOutput;
	}
	else {
		// (z.manning 2010-07-29 15:36) - PLID 39842 - This column does have it's own sentence format so use that instead.
		// Unless the data element is blank in which case the entire value should be blank.
		if(!strTableElementOutput.IsEmpty()) {
			strData = pte->m_pColumn->m_strLongForm;
			strData.Replace(DATA_FIELD, strTableElementOutput);
			CEMN *pParentEMN = NULL;
			if (NULL != m_pParentTopic) {
				pParentEMN = m_pParentTopic->GetParentEMN();
			}
			if (NULL == pParentEMN) {
				// (a.walling 2012-07-13 10:33) - PLID 51479 - Again, check if we've been given a fallback CEMN to use
				pParentEMN = pFallbackParentEMN;
			}
			if (NULL == pParentEMN) {
				ASSERT(FALSE);
				ThrowNxException(CString(__FUNCTION__) + " was called, but the parent EMN object was NULL!");
			}
			ReplaceLongFormDemographicFields(strData, this, pParentEMN);

			// (z.manning 2011-11-04 14:04) - PLID 42765 - Handle the spawned items field for dropdowns
			if(strData.Find(SPAWNED_ITEMS_FIELD) != -1 && pte->m_pColumn->nType == LIST_TYPE_DROPDOWN)
			{
				SourceActionInfo saiTableRow(eaoEmrTableDropDownItem, -1, this, pte->m_pRow);
				CString strSpawnedDetailsSentenceFormats;
				for(int nDropdownIndex = 0; nDropdownIndex < pte->m_anDropdownIDs.GetCount(); nDropdownIndex++)
				{
					const long nDropdownID = pte->m_anDropdownIDs.GetAt(nDropdownIndex);
					CUnspawningSource usTableDropdown(this, TableSpawnInfo(nDropdownID, *pte->m_pRow, -1));
					CSpawningSourceInfo spawnInfo(&saiTableRow, &usTableDropdown);
					// (z.manning 2011-11-07 17:14) - PLID 46309 - Pass in the separator
					CString strCurrentSentenceFormat = pParentEMN->GetSpawnedDetailsSentenceFormats(&spawnInfo, pte->m_pColumn->m_strSpawnedItemsSeparator, bHtml);
					if(!strCurrentSentenceFormat.IsEmpty())
					{
						// (z.manning 2011-11-07 17:26) - PLID 46309 - Since we may have multiple dropdown items selected we
						// may need to put the spawned items separator between each group of spawned items.
						if(!strSpawnedDetailsSentenceFormats.IsEmpty()) {
							strSpawnedDetailsSentenceFormats += GetActualSpawnedItemsSepator(pte->m_pColumn->m_strSpawnedItemsSeparator);
						}
						strSpawnedDetailsSentenceFormats += strCurrentSentenceFormat;
					}
				}
				strData.Replace(SPAWNED_ITEMS_FIELD, strSpawnedDetailsSentenceFormats);
			}
			else {
				// (z.manning 2011-11-04 17:03) - PLID 42765 - Replace other spawning types
				strData.Replace(SPAWNED_ITEMS_FIELD, "");
			}

			// (r.gonet 04/29/2013) - PLID 44897 - Fill in the row name for the row we are currently on.
			if(!m_bTableRowsAsFields) {
				strData.Replace(ROW_NAME_FIELD, pte->m_pRow->strName);
			} else {
				strData.Replace(COLUMN_NAME_FIELD, pte->m_pRow->strName);
			}
		}
	}
	
	// (a.walling 2007-08-17 09:09) - PLID 27017 - Narratives in linked detail columns will
	// display as the {NXRTF file links
	if ((bHtml || bForPreview) && strData.Find("{NXRTF") != -1) {
		// whoa, it contains a linked RTF file! Since we don't currently have the capability
		// to parse RTF to HTML, we have to fall back to using the detail's name. It should
		// appear elsewhere in the file anyway.

		if (pte->m_pLinkedDetail) {
			long nDetailID = pte->m_pLinkedDetail->GetID();
			BOOL bUnsaved = (nDetailID == -1); // an unsaved detail will have an id of -1!
			if (bUnsaved) {
				nDetailID = reinterpret_cast<long>(this); // cast ourself (CEMNDetail*) to a long pointer.
			}

			CString strPointer = bUnsaved ? "PT" : "ID"; // PT for pointer, ID for ID
			// (a.walling 2007-10-18 13:51) - PLID 25548 - Ensure the label text is escaped
			strData.Format(
				"<a href='nexemr://detail%s/?%li'>%s</a>", strPointer, nDetailID, pte->m_pLinkedDetail->GetLabelText());
		} else {
			strData = "<Linked Detail>";
		}
	} else {
		// (a.walling 2007-10-18 13:55) - PLID 25548 - Ensure all text is escaped
		// (a.walling 2007-10-19 16:00) - PLID 27820 - Escaping should be the responsibility of GetValueAsOutput
	}

	// clean up any temp files that may have been created.
	for (int x = 0; x < saTempFiles.GetSize(); x++) {
		DeleteFileWhenPossible(saTempFiles[x]);
	}
	saTempFiles.RemoveAll();
	return strData;
}

// (a.walling 2012-03-14 22:51) - PLID 48896 - Do we spawn this detail eg via a table dropdown item?
bool CEMNDetail::FindSpawnedDetail(CEMNDetail* pTarget, bool bOnlyIfSpawnedItemsField)
{
	switch (m_EMRInfoType) 
	{
	case eitTable:
		{
			if (bOnlyIfSpawnedItemsField && !HasSpawnedItemsField()) {
				return false;
			}

			// (a.walling 2012-08-16 08:18) - PLID 52163 - Fixed the way spawned details were determined.
			// this prior (fixed) approach works fine, but still got all the spawned details up front, and then iterated through every detail in the EMN
			// this was entirely unnecessary, so now we can just implement what we need directly.
			//CEMNDetailArray arSpawnedDetails;
			//GetSpawnedDetails(arSpawnedDetails, pTarget);
			//return !arSpawnedDetails.IsEmpty() && arSpawnedDetails[arSpawnedDetails.GetCount() - 1] == pTarget;
			
			CEMNDetailArray arSpawnedDetails;
			foreach (TableElement te, m_arTableElements) 
			{
				TableElement* pte = &te;

				if (pte->m_pColumn->nType != LIST_TYPE_DROPDOWN) {
					continue;
				}

				if(bOnlyIfSpawnedItemsField && -1 == pte->m_pColumn->m_strLongForm.Find(SPAWNED_ITEMS_FIELD)) {
					continue;
				}

				SourceActionInfo saiTableRow(eaoEmrTableDropDownItem, -1, this, pte->m_pRow);
				for(int nDropdownIndex = 0; nDropdownIndex < pte->m_anDropdownIDs.GetCount(); nDropdownIndex++)
				{
					const long nDropdownID = pte->m_anDropdownIDs.GetAt(nDropdownIndex);
					CUnspawningSource usTableDropdown(this, TableSpawnInfo(nDropdownID, *pte->m_pRow, -1));
					CSpawningSourceInfo spawnInfo(&saiTableRow, &usTableDropdown);

					if (spawnInfo.SpawnedDetail(pTarget)) {
						return true;
					}
				}
			}
		}

		return false;
	default:
		// would be great to implement this for things other than tables
		ASSERT(FALSE);
		return false;
	}
}

// (a.walling 2012-03-14 22:51) - PLID 48896 - Find spawned details eg via a table dropdown item
// (a.walling 2012-08-16 08:18) - PLID 52163 - This now works as described, gathering the spawned details, taking in the check for whether <spawned items> is in the output as bool param
void CEMNDetail::GetSpawnedDetails(CEMNDetailArray& arSpawnedDetails, bool bOnlyIfSpawnedItemsField, CEMNDetail* pStopAt)
{
	switch (m_EMRInfoType) 
	{
	case eitTable:
		{
			foreach (TableElement te, m_arTableElements) 
			{
				TableElement* pte = &te;

				if (pte->m_pColumn->nType != LIST_TYPE_DROPDOWN) {
					continue;
				}

				if(bOnlyIfSpawnedItemsField && -1 == pte->m_pColumn->m_strLongForm.Find(SPAWNED_ITEMS_FIELD)) {
					continue;
				}

				SourceActionInfo saiTableRow(eaoEmrTableDropDownItem, -1, this, pte->m_pRow);
				for(int nDropdownIndex = 0; nDropdownIndex < pte->m_anDropdownIDs.GetCount(); nDropdownIndex++)
				{
					const long nDropdownID = pte->m_anDropdownIDs.GetAt(nDropdownIndex);
					CUnspawningSource usTableDropdown(this, TableSpawnInfo(nDropdownID, *pte->m_pRow, -1));
					CSpawningSourceInfo spawnInfo(&saiTableRow, &usTableDropdown);
					
					long nCountPre = arSpawnedDetails.GetCount();
					GetParentEMN()->GetSpawnedDetails(&spawnInfo, &arSpawnedDetails, pStopAt);
					if (pStopAt && nCountPre != arSpawnedDetails.GetCount()) {
						if (pStopAt == arSpawnedDetails[arSpawnedDetails.GetCount() - 1]) {
							return;
						}
					}
				}
			}
		}
		break;
	default:
		// would be great to implement this for things other than tables
		ASSERT(FALSE);
		break;
	}
}

// (a.walling 2007-04-10 16:38) - PLID 25454 - Return well-formatted HTML markup for tables
CString CEMNDetail::GetTableDataOutputHTMLRaw()
{
	CString strHtml, strNonHtml;	

	// (c.haag 2011-05-27) - PLID 42895 - Ensure the common list and table row data is loaded.
	// LoadContent is also called in GetTableDataOutputRaw so this does not add a performance
	// hit in a place there was none before.
	LoadContent();

	// (c.haag 2011-04-05) - PLID 43145 - In the past we used to just call GetTableDataOutputRaw
	// once. Now that we support selecting medications and allergies from "Common Lists", we need
	// to take what would have been just one table and split it into one table per common list; including
	// a table for row items that aren't in any common list
	if ((IsCurrentMedicationsTable() || IsAllergiesTable()) && m_CommonLists.GetListCount() > 0)
	{
		// This is an array of EmrDataIDs that acts as a filter for GetTableDataOutputRaw(). When passed
		// into that function, it knows to build an HTML table for ONLY those Data ID's.
		CArray<long,long> anEmrDataIDs;
		int i,j;

		// In preparation for creating HTML of what could be multiple tables of medications or allergies, we
		// first need to figure out which ones belong in the "main" Current Medications/Allergies HTML table
		// that we always used in the past, and which ones belong in their own Common-List-Speciifc HTML 
		// tables.
		//
		// We do the "figuring out" right here by iterating through all the items in all the common lists, and
		// assigning them to up to two maps.
		//
		// (c.haag 2011-04-11) - PLID 43155 - Map of Emr Data ID's that are in a common list that is
		// flagged as "Group in the preview pane" (most items should be in here; this is the default)
		CMap<long,long,BOOL,BOOL> mapGroupedEmrDataIDs;
		// (c.haag 2011-04-11) - PLID 43155 - Map of Emr Data ID's that are in a common list that is
		// NOT flagged as "Group in the preview pane"
		CMap<long,long,BOOL,BOOL> mapUngroupedEmrDataIDs;
		// Do for all common lists
		for (i=0; i < m_CommonLists.GetListCount(); i++) {
			CEmrInfoCommonList list = m_CommonLists.GetListByIndex(i);
			// Do for all common list items
			for (j=0; j < list.GetItemCount(); j++) {

				// (c.haag 2011-04-11) - PLID 43155 - Follow this logic carefully:
				//
				// If GetGroupOnPreviewPane is FALSE, that means the items in list will not appear in their own
				// group. They will instead appear with the other items that are not in any group (selected from
				// the "All" button). So, we do not want to set the entry at mapGroupedEmrDataIDs to TRUE. We will
				// instead track these in mapUngroupedEmrDataIDs.
				//
				// Having written that, you should know one other thing: It is entirely possible for one item to be
				// in multiple lists where some of those lists are grouped in the preview pane, but others are not.
				// That item will appear in both maps.
				//
				// There are four possible combinations for any given item:
				//
				// 1. The item will not appear in the preview pane because it's not selected
				// 2. The item appears only in the ungrouped table because it's either in an ungrouped list or not in any list
				//	(could have come from the "All" button)
				// 3. The item appears only in one or more grouped tables because it belongs only to one or more lists that
				//	appear in their own groups
				// 4. The item appears in both the ungrouped table and one or more grouped tables because it exists in both
				// grouped lists and ungrouped lists.
				//
				if (list.GetGroupOnPreviewPane())
				{
					mapGroupedEmrDataIDs.SetAt( list.GetItemByIndex(j).GetEmrDataID(), TRUE );
				}
				else 
				{
					mapUngroupedEmrDataIDs.SetAt( list.GetItemByIndex(j).GetEmrDataID(), TRUE );
				}
			}
		}

		// In this block of code, we build the "main" Current Medications/Allergies HTML table. This is the one
		// you're used to seeing in prior releases.
		{
			// Go through all the visible rows in this table detail and add the ones that belong in this HTML
			// table to anEmrDataIDs
			for (i=0; i < m_arTableRows.GetSize(); i++)
			{
				long nEmrDataID = m_arTableRows[i]->m_ID.nDataID;
				BOOL bDummy;
				
				if (// If the item is not in use by a common list that appears in the preview pane as its own group
					!mapGroupedEmrDataIDs.Lookup(nEmrDataID, bDummy)
					// OR the item is in use by a common list that is not grouped in the preview pane
					|| mapUngroupedEmrDataIDs.Lookup(nEmrDataID, bDummy)
					)
				{
					// Then add it to our "ungrouped" filter
					anEmrDataIDs.Add(nEmrDataID);
				}			
			}
			// (a.walling 2013-03-01 11:35) - PLID 55392 - ensure array is sorted for fast lookup
			boost::sort(anEmrDataIDs);
			// Now calculate the HTML table output. The generated HTML will only contain items that are contained
			// in anEmrDataIDs and have any row content.
			{
				CString strHtmlTemp;
				CString strNonHtmlTemp;
				// (c.haag 2011-04-12) - PLID 43245 - Pass in NULL for the table caption
				GetTableDataOutputRaw(strHtmlTemp, strNonHtmlTemp, NULL, &anEmrDataIDs, NULL, NULL);
				// Append GetTableDataOutputRaw's output to our output
				strHtml += strHtmlTemp;
				strNonHtml += strNonHtmlTemp;
				// Add an extra line break so any forward captions don't get sandwiched too closely to the end of this table.
				if (!strHtmlTemp.IsEmpty()) {
					strHtml += "<BR>";
					strNonHtml += "\n"; // (c.haag 2011-04-12) - PLID 43245 
				}
			}
		}

		// In this block of code, we go through each common list and calculate the HTML table output for the items that
		// correspond to it.
		for (i=0; i < m_CommonLists.GetListCount(); i++) 
		{
			CEmrInfoCommonList list = m_CommonLists.GetListByIndex(i);
			// (c.haag 2011-04-11) - PLID 43155 - Only generate the table if the list is appearing as its own group
			if (list.GetGroupOnPreviewPane())
			{				
				// Populate anEmrDataIDs with everything in the Common List. Don't worry; anEmrDataID's is simply a filter.
				// GetTableDataOutputRaw() will only build an HTML table with rows that are in anEmrDataIDs and that have
				// content in them.
				anEmrDataIDs.RemoveAll();
				for (int i=0; i < list.GetItemCount(); i++) {
					anEmrDataIDs.Add(list.GetItemByIndex(i).GetEmrDataID());
				}				
				// (a.walling 2013-03-01 11:35) - PLID 55392 - ensure array is sorted for fast lookup
				boost::sort(anEmrDataIDs);

				// (c.haag 2011-04-12) - PLID 43245 - Discern HTML from non-HTML captions
				CString strHTMLCaption = FormatString("<caption align=left><font color=%02x%02x%02x><b>%s</b></font></caption>"
					,GetRValue(list.GetColor()), GetGValue(list.GetColor()), GetBValue(list.GetColor()), list.GetName()
					);
				CString strCaption = list.GetName();
				{
					CString strHtmlTemp;
					CString strNonHtmlTemp;
					GetTableDataOutputRaw(strHtmlTemp, strNonHtmlTemp, NULL, &anEmrDataIDs, (LPCTSTR)strHTMLCaption, (LPCTSTR)strCaption);
					// Append GetTableDataOutputRaw's output to our output
					strHtml += strHtmlTemp;
					strNonHtml += strNonHtmlTemp;
					// Add an extra line break so any forward captions don't get sandwiched too closely to the end of this table.
					if (!strHtmlTemp.IsEmpty()) {
						strHtml += "<BR>";
						strNonHtml += "\n"; // (c.haag 2011-04-12) - PLID 43245 
					}
				}
			} else {
				// If we get here, ignore the list because we're not grouping it
			}
		}
	}
	else
	{
		// Not a system table; output the table normally
		// (c.haag 2011-04-12) - PLID 43245 - Pass in NULL for the table caption
		GetTableDataOutputRaw(strHtml, strNonHtml, NULL, NULL, NULL, NULL);
	}
	return strHtml;
}

// (z.manning 2010-07-30 15:39) - PLID 39842
CString CEMNDetail::GetTableDataOutputNonHTMLRaw()
{
	CString strHtml, strNonHtml;
	// (c.haag 2011-04-05) - PLID 43145 - Pass in NULL for the EmrDataIDs filter; we don't use it here
	// (c.haag 2011-04-12) - PLID 43245 - Pass in NULL for the table caption
	GetTableDataOutputRaw(strHtml, strNonHtml, NULL, NULL, NULL, NULL);
	return strNonHtml;
}

// (z.manning 2010-07-30 15:25) - PLID 39842 - Added a function to get non html table output as well
// (c.haag 2011-04-05) - PLID 43145 - We can now filter on EmrDataID's when building the <table> block. We also
// take in a table caption tag.
// (c.haag 2011-04-12) - PLID 43245 - We now discern HTML table captions from non-HTML table captions
//TES 3/22/2012 - PLID 48203 - Added pFallbackParentEMN as a parameter, it will be used if this function is called for a detail which
// has not yet been attached to an EMN
void CEMNDetail::GetTableDataOutputRaw(OUT CString &strHTML, OUT CString &strNonHTML, TableRowID* pSingleTableRowID, CArray<long,long>* paTableEmrDataIDs,
									   LPCTSTR szHTMLCaption, LPCTSTR szNonHTMLCaption, CEMN *pFallbackParentEMN /*= NULL*/)
{
	LoadContent();
	GetTableDataOutputRaw(m_strLongForm, strHTML, strNonHTML, pSingleTableRowID, paTableEmrDataIDs, szHTMLCaption, szNonHTMLCaption, pFallbackParentEMN);
}

// (z.manning 2010-08-10 09:24) - PLID 39497 - Added an overload to pass in a custom long form
// (c.haag 2011-04-05) - PLID 43145 - We can now filter on EmrDataID's when building the <table> block. We also
// take in a table caption tag.
// (c.haag 2011-04-12) - PLID 43245 - We now discern HTML table captions from non-HTML table captions
//TES 3/22/2012 - PLID 48203 - Added pFallbackParentEMN as a parameter, it will be used if this function is called for a detail which
// has not yet been attached to an EMN
void CEMNDetail::GetTableDataOutputRaw(IN CString strLongForm, OUT CString &strHTML, OUT CString &strNonHTML, TableRowID* pSingleTableRowID, CArray<long,long>* paTableEmrDataIDs,
									   LPCTSTR szHTMLCaption, LPCTSTR szNonHTMLCaption, CEMN *pFallbackParentEMN /*= NULL*/)
{
	//this function is going to calculate both the HTML and nonHTML strings at the same time

	// (a.walling 2010-04-27 13:38) - PLID 30921 - Ensure we load content before trying to access things like m_bDisableTableBorder
	//Make sure this detail has loaded its content (if its already loaded, function will have no effect.
	LoadContent();

	// (a.walling 2013-03-01 11:35) - PLID 55392 - assert if not sorted
#ifdef _DEBUG
	_ASSERTE(!paTableEmrDataIDs || boost::is_sorted(*paTableEmrDataIDs));
#endif

	//TES 2/22/2010 - PLID 37463 - Get our parent EMN.
	//TES 3/22/2012 - PLID 48203 - Moved the calculations up here, we'll use this parent EMN throughout the function.
	CEMN *pParentEMN = NULL;
	if (NULL != m_pParentTopic) {
		pParentEMN = m_pParentTopic->GetParentEMN();
	}
	if (NULL == pParentEMN) {
		//TES 3/22/2012 - PLID 48203 - Check if we've been given a fallback CEMN to use
		pParentEMN = pFallbackParentEMN;
		if(NULL == pParentEMN) {
			ASSERT(FALSE);
			ThrowNxException("::GetTableDataOutputRaw was called, but the parent EMN object was NULL!");
		}
	}

	// (z.manning 2010-08-11 09:28) - PLID 39497 - Added support for sentence format groups within the overall sentence format
	// for details. This code will replace any groups with a placeholder tag and store the text that should go there in a string
	// array. This will allow us to process the rest of the sentence format as normal and then substitute the replacement text
	// once we're ready.
	CStringArray arystrReplacementHTML, arystrReplacementNonHTML;
	CString strHTMLLongForm = strLongForm;
	// (a.walling 2011-06-20 17:56) - PLID 44215 - Need to keep track of set of data IDs to pass on to the recursive GetTableDataOutputRaw call
	//TES 3/22/2012 - PLID 48203 - Pass in the parent EMN
	BOOL bHasAtLeastOneGroup = ReplaceSentenceFormatGroupsWithPlaceholders(this, strHTMLLongForm, true, arystrReplacementHTML, pSingleTableRowID, paTableEmrDataIDs, pFallbackParentEMN);

	CString strNonHTMLLongForm = strLongForm;
	//TES 3/22/2012 - PLID 48203 - Pass in the parent EMN
	if(ReplaceSentenceFormatGroupsWithPlaceholders(this, strNonHTMLLongForm, false, arystrReplacementNonHTML, pSingleTableRowID, paTableEmrDataIDs, pFallbackParentEMN) != bHasAtLeastOneGroup) {
		// (z.manning 2010-08-10 13:09) - PLID 39497 - The HTML and non-HTML long forms are both based on the same
		// basic long form so it shouldn't be possible for one to have a group and the other to not.
		ASSERT(FALSE);
	}

	CString strState = VarString(GetState(), "");

	CString strTableStart;
	// (a.walling 2010-04-27 13:38) - PLID 30921 - Disable table border if option is selected
	if (m_bDisableTableBorder) {
		strTableStart.Format("<table class='NoBorder'>");
	} else {
		strTableStart = "<table>";
	}

	// (c.haag 2011-04-05) - PLID 43145 - Optional caption tag
	if (NULL != szHTMLCaption) {
		strTableStart += szHTMLCaption;
	}

	// (a.walling 2009-03-09 12:54) - PLID 32969 - Repeat table headers if it gets split across a page
	strTableStart += "<tr>\r\n<thead><th>&nbsp;</th>";

	//determines if a row should be added at all
	BOOL bRowHasData = FALSE;

	//determines if a table should be added at all
	BOOL bTableHasData = FALSE;

	long nLastSeparatorPosHTML = -1;
	long nLastSeparatorPosNonHTML = -1;

	// (a.walling 2010-04-14 11:33) - PLID 34406 - Also need to ensure any calculated fields are updated appropriately.
	// (z.manning 2012-03-28 11:02) - PLID 33710 - Calculated cells are now saved to data so we now only update them
	// when the table is changed.
	//CEmrItemAdvTableBase::UpdateCalculatedFields(this, NULL);

	//TES 2/22/2010 - PLID 37463 - If we're using the "Smart Stamps" long form, then this will work totally differently.
	// (z.manning 2010-07-26 12:08) - PLID 39842 - All tables now support different data formats
	switch(m_nDataFormat)
	{
	// (z.manning 2010-08-18 11:22) - PLID 39842 - emsfNumberList means table format when dealing with tables
	case emsfNumberList:
		{
			// (z.manning 2010-08-18 12:11) - PLID 39842 - For table-formatted tables, let's continue to do exactly
			// what they've always done for the non-HTML sentence format.
			CSortedTableElementArray aryTableElements;
			const int nElementCount = GetTableElementCount();
			for(int nElementIndex = 0; nElementIndex < nElementCount; nElementIndex++) {
				aryTableElements.Add(GetTableElementPtrByIndex(nElementIndex));
			}
			aryTableElements.Sort(this);
			for(nElementIndex = 0; nElementIndex < nElementCount; nElementIndex++)
			{
				TableElement *pte = aryTableElements.GetAt(nElementIndex);
				//(e.lally 2011-12-08) PLID 46471 - We do need to check for allergies and current medication 
				//	checked status and process the element data if it A. isn't a medication or allergy or B. is one and is also checked.
				if((!IsCurrentMedicationsTable() && !IsAllergiesTable()) || pte->m_pRow->m_bIsCurrentMedOrAllergyChecked){
					// (a.walling 2012-07-13 10:28) - PLID 51479 - Pass in calculated fallback parent EMN
					CString strDataNonHTML = GetTableElementDataOutput(pte, false, false, pParentEMN);
					if(!strDataNonHTML.IsEmpty()) {
						CString strElement;
						if(m_bTableRowsAsFields) {
							strElement.Format("%s: %s: %s", pte->m_pColumn->strName, pte->m_pRow->strName, strDataNonHTML);
						}
						else {
							strElement.Format("%s: %s: %s", pte->m_pRow->strName, pte->m_pColumn->strName, strDataNonHTML);
						}
						strElement = "*  " + strElement + "\r\n";

						strNonHTML += strElement;
					}
				}
			}
		}
		break;

	case emsfBulletList:
		// (z.manning 2010-07-27 12:36) - PLID 39842 - Start an unordered list if this table used bullet list format
		strHTML += "<ul>";
	case emsfText:
	case emsfList: {
		//TES 2/22/2010 - PLID 37463 - We need a line for each row.
		// (z.manning 2010-08-10 13:17) - PLID 39497 - This may be different for html vs. non-html
		CString strBasicRowHTML = strHTMLLongForm;
		CString strBasicRowNonHTML = strNonHTMLLongForm;
		//TES 2/22/2010 - PLID 37463 - Fill all the demographic fields.
		// (z.manning 2010-07-29 16:26) - PLID 39842 - Now have a utility function to do this
		//TES 3/22/2012 - PLID 48203 - Pass in the pParentEMN we calculated above
		ReplaceLongFormDemographicFields(strBasicRowHTML, this, pParentEMN);
		ReplaceTableLongFormDataField(strBasicRowHTML, this);
		ReplaceLongFormDemographicFields(strBasicRowNonHTML, this, pParentEMN);
		ReplaceTableLongFormDataField(strBasicRowNonHTML, this);
	
		CMap<__int64, __int64, int, int> mapTable; // This maps row/column combinations to elements
		// (c.haag 2007-08-20 09:03) - PLID 27118 - We now use a utility function to build mapTable
		PopulateTableElementMap(mapTable);
		
		// (a.walling 2013-03-13 12:34) - PLID 55633 - Figure out which rows actually have data
		boost::unordered_set<TableRow*> rowsWithData;
		for (int n = 0; n < m_arTableElements.GetSize(); ++n) {
			rowsWithData.insert(m_arTableElements[n].m_pRow);
		}

		std::vector<TableRow*> detailRows;
		detailRows.reserve(rowsWithData.size());

		const int nDetailRows = GetRowCount();
		
		// (a.walling 2013-03-13 12:34) - PLID 55633 - filter rows we are interested in to detailRows
		for(int nRow = 0; nRow < nDetailRows; nRow++)
		{
			// (a.walling 2010-03-26 18:00) - PLID 37923 - GetTableElementDataOutput_SmartStamp takes a TableRow* rather than an index
			TableRow* pTr = GetRowPtr(nRow);

			if (!pTr) {
				continue;
			}

			// (z.manning 2010-08-16 13:22) - PLID 39842 - Preserve the ability to only get sentence format for a specific row.
			if( !(pSingleTableRowID == NULL || (*pSingleTableRowID == pTr->m_ID) )) {
				continue;
			}
			
			// (a.walling 2013-03-13 12:34) - PLID 55633
			if (!rowsWithData.count(pTr)) {
				continue;
			}

			// (c.haag 2011-04-12) - PLID 43245 - Continue if we're filtering on a set of data items and this item doesn't meet the criteria
			// (a.walling 2013-03-01 11:35) - PLID 55392 - use fast binary search lookup rather than linear scan
			if( paTableEmrDataIDs != NULL && !boost::binary_search(*paTableEmrDataIDs, pTr->m_ID.nDataID) ) {
				continue;
			}

			detailRows.push_back(pTr);
		}

		// (c.haag 2011-04-12) - PLID 43245 - Flag whether this is the first valid row
		// (a.walling 2013-03-13 12:34) - PLID 55633 - Finally, we can simply scan through our detailRows vector
		bool bHasRow = false;
		for each(TableRow* pTr in detailRows) 
		{
			// (a.walling 2010-03-26 18:00) - PLID 37923 - GetTableElementDataOutput_SmartStamp takes a TableRow* rather than an index


			//TES 2/22/2010 - PLID 37463 - Pull the row, then replace the column fields with the actual values pulled from this row
			// and the appropriate hard-coded column.
			CString strWorkingHTMLRow = strBasicRowHTML;
			CString strWorkingNonHTMLRow = strBasicRowNonHTML;
			//TES 3/17/2010 - PLID 37463 - Track whether any of these elements have a value.
			bool bRowIsEmpty = true;
			const int nDetailColumns = GetColumnCount();

			// (z.manning 2010-09-02 10:39) - PLID 40206 - Handle the row/column name fields
			bool bReplacedFieldName = false;
			if(m_bTableRowsAsFields) {
				strWorkingHTMLRow.Replace(COLUMN_NAME_FIELD, pTr->strName);
				strWorkingNonHTMLRow.Replace(COLUMN_NAME_FIELD, pTr->strName);
			}
			else {
				strWorkingHTMLRow.Replace(ROW_NAME_FIELD, pTr->strName);
				strWorkingNonHTMLRow.Replace(ROW_NAME_FIELD, pTr->strName);
			}
			if(!pTr->strName.IsEmpty() && strWorkingHTMLRow != strBasicRowHTML) {
				bReplacedFieldName = true;
			}

			for(int nColumn = 0; nColumn < nDetailColumns; nColumn++) {
				TableColumn tc = GetColumn(nColumn);
				//(e.lally 2011-12-08) PLID 46471 - We don't need to check for allergies and current medication 
				//	checked status because this is an overloaded function that will do it internally
				// (a.walling 2012-07-13 10:28) - PLID 51479 - Pass in calculated fallback parent EMN
				CString strValue = GetTableElementDataOutput(pTr, &tc, mapTable, false, true, pParentEMN);
				CString strRowPre = strWorkingHTMLRow;
				strWorkingHTMLRow.Replace("<" + tc.strName + ">", strValue);
				strWorkingNonHTMLRow.Replace("<" + tc.strName + ">", strValue);
				if(!strValue.IsEmpty() && strWorkingHTMLRow != strRowPre) {
					bRowIsEmpty = false;
				}
				// (z.manning 2010-09-02 10:52) - PLID 40206 - If we have any data in this row and the
				// sentence format includes the row name then we consider this row non-empty.
				if(!strValue.IsEmpty() && bReplacedFieldName) {
					bRowIsEmpty = false;
				}
			}
			//TES 3/17/2010 - PLID 37463 - Check whether we actually replaced anything; if not, we don't want to add an ugly-looking
			// blank row.
			if(!bRowIsEmpty && strWorkingHTMLRow != strBasicRowHTML)
			{
				// (c.haag 2011-04-12) - PLID 43245 - Track the first row for later
				bHasRow = true;

				CString strNonHtmlRow = strWorkingNonHTMLRow;
				if(m_nDataFormat == emsfText) {
					// (z.manning 2010-07-27 10:43) - PLID 39842 - If the data format is text then use the data separator
					if(!strHTML.IsEmpty()) {
						strWorkingHTMLRow.Insert(0, m_strDataSeparator);
						nLastSeparatorPosHTML = strHTML.GetLength();
					}
					if(!strNonHTML.IsEmpty()) {
						strNonHtmlRow.Insert(0, m_strDataSeparator);
						nLastSeparatorPosNonHTML = strNonHTML.GetLength();
					}
				}
				else if(m_nDataFormat == emsfBulletList) {
					strHTML += "<li>";
					strNonHtmlRow = "*  " + strNonHtmlRow;
				}

				//TES 2/22/2010 - PLID 37463 - Those didn't ouput HTML, so convert it now, and add a new line.
				strHTML += ConvertToHTMLEmbeddable(strWorkingHTMLRow);
				strNonHTML += strNonHtmlRow;

				if(m_nDataFormat == emsfBulletList) {
					strHTML += "</li>";
					strNonHTML += "\r\n";
				}
				else if(m_nDataFormat == emsfList) {
					strHTML += "<br>";
					strNonHTML += "\r\n";
				}
			}
		}

		if(m_nDataFormat == emsfBulletList) {
			if(strHTML == "<ul>") {
				strHTML.Empty();
			}
			else {
				strHTML += "</ul>";
			}
		}
		else if(m_nDataFormat == emsfText) {
			if (nLastSeparatorPosHTML != -1) {
				ASSERT(strHTML.Mid(nLastSeparatorPosHTML, m_strDataSeparator.GetLength()) == m_strDataSeparator);
				if(!m_strDataSeparator.IsEmpty()) {
					strHTML.Delete(nLastSeparatorPosHTML, m_strDataSeparator.GetLength());
				}
				strHTML.Insert(nLastSeparatorPosHTML, m_strDataSeparatorFinal);
			}
			if (nLastSeparatorPosNonHTML != -1) {
				ASSERT(strNonHTML.Mid(nLastSeparatorPosNonHTML, m_strDataSeparator.GetLength()) == m_strDataSeparator);
				if(!m_strDataSeparator.IsEmpty()) {
					strNonHTML.Delete(nLastSeparatorPosNonHTML, m_strDataSeparator.GetLength());
				}
				strNonHTML.Insert(nLastSeparatorPosNonHTML, m_strDataSeparatorFinal);
			}
		}

		//TES 2/25/2010 - PLID 37463 - Trim the last <br>, if any.
		if(strHTML.GetLength() >= 4 && strHTML.Right(4) == "<br>") {
			strHTML = strHTML.Left(strHTML.GetLength()-4);
		}
		if(strNonHTML.GetLength() >= 2 && strNonHTML.Right(2) == "\r\n") {
			strNonHTML = strNonHTML.Left(strNonHTML.GetLength()-2);
		}

		// (c.haag 2011-04-14) - PLID 43245 - If we have at least one valid row, prepend the caption. Use the non-HTML 
		// caption because the tags in the HTML version would be directly output to the screen. The * acts as a "bullet"
		// in the caption.
		if (bHasRow && NULL != szNonHTMLCaption) {
			CString strFormattedCaption = CString("\n* ") + CString(szNonHTMLCaption) + "\n";
			strHTML = ConvertToHTMLEmbeddable(strFormattedCaption) + strHTML;
			strNonHTML = strFormattedCaption + strNonHTML;
		}

	}
	break;

	default:
		// (z.manning 2010-08-03 09:56) - Unhandled data format
		ASSERT(FALSE);
		break;
	}//end switch

	// (z.manning 2010-08-11 09:32) - PLID 39497 - Do we need to handle sentence format groups?
	if(bHasAtLeastOneGroup)
	{
		// (a.walling 2011-06-21 12:27) - PLID 44215 - The replacement text may be empty; in which case, ignore it.
		bool bHTMLWasEmpty = false;
		if(strHTML.IsEmpty()) {
			// (z.manning 2010-08-11 09:33) - PLID 39497 - We need to handle the case where all of the relevent
			// sentence format is within groups (which practically speaking, will be most of the time). In that
			// case our overall sentence format will be empty and all the relevant sentence format will be
			// in the setence format returned by the function to handle groups.
			bHTMLWasEmpty = true;
			strHTML = strHTMLLongForm;
			// (a.walling 2011-06-22 11:12) - PLID 44259 - Replace any long form demographic fields
			//TES 3/22/2012 - PLID 48203 - Pass in the pParentEMN we calculated above
			ReplaceLongFormDemographicFields(strHTML, this, pParentEMN);
			strHTML = ConvertToHTMLEmbeddable(strHTML);
		}
		// (z.manning 2010-08-11 09:38) - PLID 39497 - Look for the first group placeholder.
		int nReplaceStart = strHTML.Find(ConvertToHTMLEmbeddable(GROUP_PLACEHOLDER));
		int nStringIndex = 0;
		// (a.walling 2011-06-21 12:27) - PLID 44215 - The replacement text may be empty; in which case, ignore it.
		bool bHTMLReplacementEmpty = true;
		while(nReplaceStart != -1) {
			// (z.manning 2010-08-11 09:39) - PLID 39497 - Remove the placeholder
			strHTML.Delete(nReplaceStart, CString(ConvertToHTMLEmbeddable(GROUP_PLACEHOLDER)).GetLength());
			// (z.manning 2010-08-11 09:40) - PLID 39497 - Now insert the replacement text. The modular function is
			// used to pull from the array because there may have more group placeholders than elements in the array
			// such as in the case where we have groups plus relevant sentence data outside the group which means
			// that the entire sentence format (including the groups) are repeated for each row.
			CString strReplacement = arystrReplacementHTML.GetAt(nStringIndex % arystrReplacementHTML.GetSize());
			if (!strReplacement.IsEmpty()) {
				bHTMLReplacementEmpty = false;
				
				// (a.walling 2011-06-22 11:12) - PLID 44259 - Replace any long form demographic fields
				//TES 3/22/2012 - PLID 48203 - Pass in the pParentEMN we calculated above
				ReplaceLongFormDemographicFields(strReplacement, this, pParentEMN);
			}

			strHTML.Insert(nReplaceStart, strReplacement);
			nStringIndex++;
			// (z.manning 2010-08-11 09:42) - PLID 39497 - Look for the next group
			nReplaceStart = strHTML.Find(ConvertToHTMLEmbeddable(GROUP_PLACEHOLDER));
		}

		// (a.walling 2011-06-21 12:27) - PLID 44215 - The replacement text may be empty; in which case, ignore it.
		if (bHTMLWasEmpty && bHTMLReplacementEmpty) {
			strHTML.Empty();
		}

		// (z.manning 2010-08-11 09:37) - PLID 39497 - Now repeat the exact same process for the non-HTML sentence format.
		// (a.walling 2011-06-21 12:27) - PLID 44215 - The replacement text may be empty; in which case, ignore it.
		bool bNonHTMLWasEmpty = false;
		if(strNonHTML.IsEmpty()) {
			bNonHTMLWasEmpty = true;
			strNonHTML = strNonHTMLLongForm;
			
			// (a.walling 2011-06-22 11:12) - PLID 44259 - Replace any long form demographic fields
			//TES 3/22/2012 - PLID 48203 - Pass in the pParentEMN we calculated above
			ReplaceLongFormDemographicFields(strNonHTMLLongForm, this, pParentEMN);
		}
		nReplaceStart = strNonHTML.Find(GROUP_PLACEHOLDER);
		nStringIndex = 0;
		// (a.walling 2011-06-21 12:27) - PLID 44215 - The replacement text may be empty; in which case, ignore it.
		bool bNonHTMLReplacementEmpty = true;
		while(nReplaceStart != -1) {
			strNonHTML.Delete(nReplaceStart, CString(GROUP_PLACEHOLDER).GetLength());
			CString strReplacement = arystrReplacementNonHTML.GetAt(nStringIndex % arystrReplacementNonHTML.GetSize());
			strNonHTML.Insert(nReplaceStart, strReplacement);
			if (!strReplacement.IsEmpty()) {
				bNonHTMLReplacementEmpty = false;

				// (a.walling 2011-06-22 11:12) - PLID 44259 - Replace any long form demographic fields
				//TES 3/22/2012 - PLID 48203 - Pass in the pParentEMN we calculated above
				ReplaceLongFormDemographicFields(strReplacement, this, pParentEMN);
			}
			nStringIndex++;
			nReplaceStart = strNonHTML.Find(GROUP_PLACEHOLDER);
		}

		// (a.walling 2011-06-21 12:27) - PLID 44215 - The replacement text may be empty; in which case, ignore it.
		if (bNonHTMLWasEmpty && bNonHTMLReplacementEmpty) {
			strNonHTML.Empty();
		}
	}

	// (z.manning 2010-07-26 12:09) - PLID 39842 - This is actually table format when dealing with tables
	if(m_nDataFormat == emsfNumberList)
	{
		strHTML.Empty();
		CString strRow;
		// (a.walling 2007-08-17 08:27) - PLID 25454 - Looping through the table like this can have some 
		// terrible lookup times. So I'm implementing c.haag's map solution. Memory requirements increase,
		// but the worst case for large tables decreases significantly.
		//
		// (c.haag 2007-02-20 09:15) - PLID 24679 - CEMNDetail::GetTableElement is slow.
		// By building a CMap object with all of the table elements at the beginning, and
		// then using that map within the innermost loop, we get a performance gain.
		//
		// (c.haag 2008-10-22 10:31) - PLID 31760 - We need to be able to output table information
		// for flipped tables. So, rather than traversing all detail rows, all detail columns; we
		// need to traverse table rows, table columns.
		const int nDetailRows = GetRowCount();
		const int nDetailColumns = GetColumnCount();
		const int nHTMLRows = (m_bTableRowsAsFields) ? nDetailColumns : nDetailRows;
		const int nHTMLColumns = (m_bTableRowsAsFields) ? nDetailRows : nDetailColumns;
		CMap<__int64, __int64, int, int> mapTable; // This maps row/column combinations to elements

		// (c.haag 2007-08-20 09:03) - PLID 27118 - We now use a utility function to build mapTable
		PopulateTableElementMap(mapTable);

		// (a.walling 2009-12-29 08:52) - PLID 36716 - Keep track of actual rows so we know which are even/odd in display
		int nActualRowCount = 0;

		CStringArray saTempFiles;

		// (a.walling 2010-04-29 18:05) - PLID 29881 - Keep track of all of our table output
		CMap<__int64, __int64, CString, CString&> mapTableData;

		// (a.walling 2010-04-29 18:05) - PLID 29881 - Keep track of all columns that have data
		int* pnFilledColumnsArray = new int[nHTMLColumns];
		::ZeroMemory(pnFilledColumnsArray, sizeof(int)*nHTMLColumns);

		for(int i = 0; i < nHTMLRows; i++) {
			for(int j = 0; j < nHTMLColumns; j++) {
				// (c.haag 2008-10-22 11:47) - PLID 31760 - Get the row and column
				TableRow* ptr = GetRowPtr( m_bTableRowsAsFields ? j : i );
				TableColumn* ptc = GetColumnPtr ( m_bTableRowsAsFields ? i : j);
				TableElement te;

				// (c.haag 2011-04-05) - PLID 43145 - If we're filtering on EmrDataID's, ignore rows that don't meet the criteria.
				// If we're not filtering, then all rows are valid.
				// (a.walling 2013-03-01 11:35) - PLID 55392 - use fast binary search lookup rather than linear scan
				if (NULL == paTableEmrDataIDs || boost::binary_search(*paTableEmrDataIDs, ptr->m_ID.nDataID) )
				{
					// (z.manning 2010-04-14 10:20) - PLID 38175 - Keep track if this row is a label
					BOOL bRowIsLabel = m_bTableRowsAsFields ? ptc->m_bIsLabel : ptr->m_bIsLabel;
					
					// (a.walling 2010-04-29 18:05) - PLID 29881 - Keep track if this column is a label
					BOOL bColIsLabel = m_bTableRowsAsFields ? ptr->m_bIsLabel : ptc->m_bIsLabel;

					// GetTableElement(tr.nID, tc.nID, te);
					// (a.walling 2007-08-17 08:30) - PLID 25454 - Use the map instead of GetTableElement
					const int nColumnID = ptc->nID;
					// (c.haag 2007-08-20 09:03) - PLID 27118 - We now use a utility function to pull the key
					__int64 nKey = GetTableElementKey(ptr, nColumnID);
					int nTableIndex;
					if (mapTable.Lookup(nKey, nTableIndex)) {
						// Success, we have a table element.
						GetTableElementByIndex(nTableIndex, te);
					} else {
						// c.haag's comment follows:
						// If this cell has no element, assign it a default. This should
						// be the responsibility of TableElement::GetValueAsVariant. You
						// might think "But te was never explicitly assigned a value from
						// this function!". ::GetTableElement never assigned one, so neither
						// should we. With regards to generating data output, we must strive
						// to be as consistent as possible with the old behavior.
						te.m_pRow = ptr;
						te.m_pColumn = ptc;
					}

					//now the HTML
					// (z.manning 2010-08-02 15:18) - PLID 39842 - Moved this logic to its own function
					//(e.lally 2011-12-08) PLID 46471 - We don't need to check for allergies and current medication 
					//	checked status because this is an overloaded function that will do it internally
					// (a.walling 2012-07-13 10:28) - PLID 51479 - Pass in calculated fallback parent EMN
					CString strDataHTML = GetTableElementDataOutput(ptr, ptc, mapTable, true, true, pParentEMN);
					
					// (a.walling 2007-08-17 09:09) - PLID 27017 - Narratives in linked detail columns will
					// display as the {NXRTF file links
					if (strDataHTML.Find("{NXRTF") != -1) {
						// whoa, it contains a linked RTF file! Since we don't currently have the capability
						// to parse RTF to HTML, we have to fall back to using the detail's name. It should
						// appear elsewhere in the file anyway.

						if (te.m_pLinkedDetail) {
							long nDetailID = te.m_pLinkedDetail->GetID();
							BOOL bUnsaved = (nDetailID == -1); // an unsaved detail will have an id of -1!
							if (bUnsaved) {
								nDetailID = reinterpret_cast<long>(this); // cast ourself (CEMNDetail*) to a long pointer.
							}

							CString strPointer = bUnsaved ? "PT" : "ID"; // PT for pointer, ID for ID
							// (a.walling 2007-10-18 13:51) - PLID 25548 - Ensure the label text is escaped
							strDataHTML.Format(
								"<a href='nexemr://detail%s/?%li'>%s</a>", strPointer, nDetailID, ConvertToHTMLEmbeddable(te.m_pLinkedDetail->GetLabelText()));
						} else {
							strDataHTML = ConvertToHTMLEmbeddable("<Linked Detail>");
						}
					} else {
						// (a.walling 2007-10-18 13:55) - PLID 25548 - Ensure all text is escaped
						// (a.walling 2007-10-19 16:00) - PLID 27820 - Escaping should be the responsibility of GetValueAsOutput
					}

					
					// (a.walling 2010-04-29 18:05) - PLID 29881 - If the column is a label, or if the output is not empty, then
					// this column is 'filled'. Otherwise we still set the data in the map if it is a row label.
					if (!strDataHTML.IsEmpty() || bColIsLabel) {
						pnFilledColumnsArray[j]++;
						mapTableData.SetAt(nKey, strDataHTML);
					} else if (bRowIsLabel) {
						mapTableData.SetAt(nKey, strDataHTML);
					}

				}
			}
		}
		
		// clean up any temp files that may have been created.
		for (int x = 0; x < saTempFiles.GetSize(); x++) {
			DeleteFileWhenPossible(saTempFiles[x]);
		}
		saTempFiles.RemoveAll();

		// (c.haag 2011-04-05) - PLID 43145 - Flag whether this is the first valid row
		int nFirstHTMLRow = -1;
		// (a.walling 2010-04-29 18:05) - PLID 29881 - Now that we have all the output, iterate through again to build the table
		for(int i = 0; i < nHTMLRows; i++) {
			bool bFirstColumn = true;
			for(int j = 0; j < nHTMLColumns; j++) {
				// (a.walling 2010-04-29 18:05) - PLID 29881 - skip empty columns
				if (pnFilledColumnsArray[j] == 0) {
					continue;
				}

				// (c.haag 2008-10-22 11:47) - PLID 31760 - Get the row and column
				TableRow* ptr = GetRowPtr( m_bTableRowsAsFields ? j : i );
				TableColumn* ptc = GetColumnPtr ( m_bTableRowsAsFields ? i : j);
				TableElement te;
				
				// (c.haag 2011-04-05) - PLID 43145 - If we're filtering on EmrDataID's, ignore rows that don't meet the criteria.
				// If we're not filtering, then all rows are valid.
				// (a.walling 2013-03-01 11:35) - PLID 55392 - use fast binary search lookup rather than linear scan
				if (NULL == paTableEmrDataIDs || boost::binary_search(*paTableEmrDataIDs, ptr->m_ID.nDataID))
				{
					if (-1 == nFirstHTMLRow) {
						nFirstHTMLRow = i;
					}

					// GetTableElement(tr.nID, tc.nID, te);
					// (a.walling 2007-08-17 08:30) - PLID 25454 - Use the map instead of GetTableElement
					const int nColumnID = ptc->nID;
					// (c.haag 2007-08-20 09:03) - PLID 27118 - We now use a utility function to pull the key
					__int64 nKey = GetTableElementKey(ptr, nColumnID);

					// (z.manning 2010-04-14 10:20) - PLID 38175 - Keep track if this row is a label
					BOOL bRowIsLabel = m_bTableRowsAsFields ? ptc->m_bIsLabel : ptr->m_bIsLabel;
					
					// (a.walling 2010-04-29 18:05) - PLID 29881 - Keep track if this column is a label
					BOOL bColIsLabel = m_bTableRowsAsFields ? ptr->m_bIsLabel : ptc->m_bIsLabel;

					CString strTDClass;
					if(bRowIsLabel || bColIsLabel) {
						strTDClass = " class='label'";
					}

					//now output the row
					if (bFirstColumn) {
						//the row has changed

						//if the last row had data, output it
						if(bRowHasData) {
							strHTML += strRow + "</tr>"; // close the TR!
							// (a.walling 2009-12-29 08:52) - PLID 36716 - Keep track of actual rows so we know which are even/odd in display
							nActualRowCount++;
						}
					
						//now start a new row
						bRowHasData = FALSE;
						// (a.walling 2009-12-29 08:52) - PLID 36716 - mark even table rows for zebra coloring
						CString strTRClass;
						if ( (nActualRowCount % 2) == 0) {
							strTRClass = " class='even'";
						}
						else {
							strTRClass = " class='odd'";
						}
						// (c.haag 2008-10-22 11:47) - PLID 31760 - Get the name based on table orientation
						strRow.Format("<tr%s>\r\n<td%s><strong>%s</strong></td>", strTRClass, strTDClass, ConvertToHTMLEmbeddable( (m_bTableRowsAsFields) ? ptc->strName : ptr->strName ));
					}

					//if first line, output headers
					// (z.manning 2010-05-05 17:57) - PLID 38519 - We used to have a somewhat convaluted check to see if
					// were in the first row that would fail if the row name was blank. I fixed it to be much simpler and
					// work in all cases.
					// (c.haag 2011-04-05) - PLID 43145 - Now we use nFirstHTMLRow since i=0 may not be the first visible row
					if(nFirstHTMLRow == i) {
						strTableStart += FormatString("<th%s>", strTDClass);
						// (c.haag 2008-10-22 11:47) - PLID 31760 - Get the name based on table orientation
						strTableStart += ConvertToHTMLEmbeddable( (m_bTableRowsAsFields) ? ptr->strName : ptc->strName );
						strTableStart += "</th>\r\n";
					}			

					strRow += FormatString("<td%s>", strTDClass);
					// (a.walling 2010-04-29 18:05) - PLID 29881 - Lookup the data in the map. Output if it is non empty or part of a label.
					CString strDataHTML;
					// (z.manning 2010-04-14 10:22) - PLID 38175 - Always print label rows
					if(mapTableData.Lookup(nKey, strDataHTML) && (!strDataHTML.IsEmpty() || bRowIsLabel || bColIsLabel)) {
						strRow += strDataHTML;
						bRowHasData = TRUE;
						// (a.walling 2010-04-29 18:05) - PLID 29881 - Also ignore column labels as the table being 'empty'
						if(!bRowIsLabel && !bColIsLabel) {
							// (z.manning 2010-04-14 10:22) - PLID 38175 - If this is a label we do not want to count this
							// as the table being non-empty.
							bTableHasData = TRUE;
						}
					}
					else {
						// (a.walling 2007-12-19 08:57) - PLID 28436 - Cosmetic changes to EMR.CSS
						strRow += "&nbsp;";
					}
					strRow += "</td>\r\n";

					bFirstColumn = false;

				}
			} // for(int j = 0; j < nHTMLColumns; j++) {
		} // for(int i = 0; i < nHTMLRows; i++) {

		// (a.walling 2010-04-29 18:05) - PLID 29881 - Clean up
		delete[] pnFilledColumnsArray;
		pnFilledColumnsArray = NULL;

		//finish up
		{
			if(bRowHasData)
				strHTML += strRow + "</tr>"; // close the TR!

			// (a.walling 2009-03-09 12:54) - PLID 32969 - Repeat table headers if it gets split across a page
			// we do this by wrapping the headers in thead, and the rows in tbody (and css styles in emr.css)
			CString strTableHtml = strTableStart + "</tr></thead><tbody>" + strHTML + "</tbody></table>"; // close the TR!
			// (z.manning 2010-08-03 11:33) - PLID 39842 - There's no reason this should not be using the table's
			// sentence format. Merging in table format has always used it and now this does too.
			CString strReplacedLongForm = strLongForm;
			// (z.manning 2012-03-20 12:02) - PLID 49038 - We need to update demographic fields for tables too.
			//TES 3/22/2012 - PLID 48203 - Pass in the pParentEMN we calculated above
			ReplaceLongFormDemographicFields(strReplacedLongForm, this, pParentEMN);
			strHTML = ConvertToHTMLEmbeddable(strReplacedLongForm);
			strHTML.Replace(ConvertToHTMLEmbeddable(DATA_FIELD), strTableHtml);

			if(!bTableHasData)
				strHTML = "";
		}
	}
}

CEMNDetail* CEMNDetail::GetNextDetail()
{
	// (a.walling 2008-10-23 09:51) - PLID 27552 - this needs to return the next detail that is actually displayed in the topic (ignoring spawned details)
	if (m_pParentTopic) {
		CArray<CEMNDetail*, CEMNDetail*> arSortedDetails;
		m_pParentTopic->GetSortedDetailArray(arSortedDetails);

		for (int i = 0; i < arSortedDetails.GetSize(); i++) {
			CEMNDetail* pDetail = arSortedDetails.GetAt(i);

			if (pDetail == this) {
				// we've found our detail; the next detail in the array is our next item.
				int index = i + 1;

				if (index >= arSortedDetails.GetSize()) {
					// this was the last detail in the sort, so the next detail is NULL.
					return NULL;
				} else {
					CEMNDetail* pFirstDetail = NULL;
					
					BOOL bDisplayedElsewhere = TRUE;

					while (index < arSortedDetails.GetSize() && bDisplayedElsewhere) {
						pFirstDetail = arSortedDetails[index];

						if (pFirstDetail->IsSubDetail()) {
							bDisplayedElsewhere = TRUE;
							index++;				
						} else {
							bDisplayedElsewhere = FALSE;
						}
					}
					if (index >= arSortedDetails.GetSize()) {
						// this was the last detail in the sort, so the next detail is NULL.
						return NULL;
					} else {
						return arSortedDetails[index];
					}
				}
			}
		}

		// if we got here, there must be no details on the topic, or 'this' is not in the detail array!
		ASSERT(FALSE);
		return NULL;
	} else {
		return NULL; // no parent topic? we can't possibly know which detail is next.
	}
}

// (j.jones 2007-07-18 10:20) - PLID 26730 - returns whether or not the info item has Info actions,
// which it usually does not, such that we don't have to search for them later
EMNDetailHasInfoActionsStatus CEMNDetail::GetHasInfoActionsStatus()
{
	return m_eHasInfoActions;
}

// (j.jones 2011-05-04 10:26) - PLID 43527 - added mapDataIDsToSig, which maps a DataID
// to the Sig entered in the table
void CEMNDetail::PopulateCurrentMedicationIDArrays(IN OUT CArray<long,long>& anDataIDs,
												   OUT CArray<long,long>& anDrugListIDs,
												   OUT CStringArray& astrDrugListNames,
												   OUT CMap<long, long, CString, LPCTSTR> &mapDataIDsToSig,
												   OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	// (c.haag 2007-08-18 11:56) - PLID 27113 - This function fills several arrays with all
	// information relevant to the Rx column of this Current Medications table.
	const int nRxColumn = 0;
	if (!IsCurrentMedicationsTable()) {
		ASSERT(FALSE);
		ThrowNxException("Error 100: Called CEMNDetail::PopulateCurrentMedicationIDArrays on a non-medication detail!");
	}
	else if (m_arTableColumns.GetSize() == 0) {
		ASSERT(FALSE);
		ThrowNxException("Error 110: Called CEMNDetail::PopulateCurrentMedicationIDArrays on an invalid medication detail!");
	}
	else if (m_arTableColumns[nRxColumn]->strName != "Rx") {
		ASSERT(FALSE);
		ThrowNxException("Error 120: Called CEMNDetail::PopulateCurrentMedicationIDArrays on an invalid medication detail!");
	}

	//(e.lally 2011-12-08) PLID 46471 - We know this is the official column for indicating that the current medication is in use. Ensure it is flagged as such.
	//	I am not aware of a way in which this would not already be set.
	m_arTableColumns[nRxColumn]->m_bIsCurrentMedOrAllergyUsageCol = TRUE;

	const int nRxColumnDataID = m_arTableColumns[nRxColumn]->nID;
	const int nTableElements = m_arTableElements.GetSize();
	int i;

	// (j.jones 2011-05-04 10:30) - PLID 43527 - get the Sig column, if we have it
	TableColumn *ptcSig = GetColumnByListSubType(lstCurrentMedicationSig);

	// Populate anDataIDs
	anDataIDs.RemoveAll();
	for(i = 0; i < m_arTableElements.GetSize(); i++) {
		const TableElement teTmp = m_arTableElements[i];
		if(teTmp.m_pColumn->nID == nRxColumnDataID) { // If this in the Rx column
			if (teTmp.m_bChecked) { // If the box is checked

				long nDataID = teTmp.m_pRow->m_ID.nDataID;
				anDataIDs.Add(nDataID);

				// (j.jones 2011-05-04 10:27) - PLID 43527 - track the Sig
				TableElement teSig;
				if(ptcSig != NULL) {
					CString strSig = "";
					if(GetTableElement(&teTmp.m_pRow->m_ID, ptcSig->nID, teSig)) {
						strSig = teSig.GetValueAsString();
					}
					//set the map only if the Sig column exists
					mapDataIDsToSig.SetAt(nDataID, strSig);
				}
			}
		}
	}

	// Populate anDrugListIDs and astrDrugListNames
	//CString strFilter;
	anDrugListIDs.RemoveAll();
	astrDrugListNames.RemoveAll();
	/*
	for (i=0; i < anDataIDs.GetSize(); i++) {
		strFilter += FormatString("%d,", anDataIDs[i]);
	}
	*/

	//
	// Now populate the array
	//
	if (!anDataIDs.IsEmpty()) {
		_ConnectionPtr pCon;
		if(lpCon) pCon = lpCon;
		else pCon = GetRemoteData();
		//strFilter.TrimRight(",");
		// (c.haag 2007-02-02 16:19) - PLID 24561 - We no longer have a Name field in DrugList
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr prs = CreateParamRecordset(pCon, "SELECT DrugList.ID AS MedID, EMRDataT.ID AS DataID, EMRDataT.Data AS Name "
			"FROM DrugList "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"WHERE EMRDataID IN ({INTARRAY}) ORDER BY EMRDataT.Data", anDataIDs);

		//clear and re-add the data IDs to be in the same order as the other two arrays
		anDataIDs.RemoveAll();

		while (!prs->eof) {
			anDataIDs.Add(AdoFldLong(prs, "DataID"));
			anDrugListIDs.Add(AdoFldLong(prs, "MedID"));
			astrDrugListNames.Add(AdoFldString(prs, "Name", ""));
			prs->MoveNext();
		}
	}
}

void CEMNDetail::PopulateAllergyIDArrays(CArray<long,long>& anAllergyIDs, 
												   CArray<long,long>& anDataIDs,
												   CStringArray& astrAllergyNames,
												   OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	// (c.haag 2007-08-18 12:35) - PLID 27114 - This function fills several arrays with all
	// information relevant to the Yes column of this Allergies table.
	const int nYesColumn = 0;
	if (!IsAllergiesTable()) {
		ASSERT(FALSE);
		ThrowNxException("Error 100: Called CEMNDetail::PopulateAllergyIDArrays on a non-medication detail!");
	}
	else if (m_arTableColumns.GetSize() == 0) {
		ASSERT(FALSE);
		ThrowNxException("Error 110: Called CEMNDetail::PopulateAllergyIDArrays on an invalid medication detail!");
	}
	else if (m_arTableColumns[nYesColumn]->strName != "Yes") {
		ASSERT(FALSE);
		ThrowNxException("Error 120: Called CEMNDetail::PopulateAllergyIDArrays on an invalid medication detail!");
	}

	//(e.lally 2011-12-08) PLID 46471 - We know this is the official column for indicating an allergy is in use. Ensure it is flagged as such.
	//	I am not aware of a way in which this would not already be set.
	m_arTableColumns[nYesColumn]->m_bIsCurrentMedOrAllergyUsageCol = TRUE;

	const int nYesColumnDataID = m_arTableColumns[nYesColumn]->nID;
	const int nTableElements = m_arTableElements.GetSize();
	int i;

	// Populate anDataIDs
	anDataIDs.RemoveAll();
	for(i = 0; i < m_arTableElements.GetSize(); i++) {
		const TableElement teTmp = m_arTableElements[i];
		if(teTmp.m_pColumn->nID == nYesColumnDataID) { // If this in the Yes column
			if (teTmp.m_bChecked) { // If the box is checked
				anDataIDs.Add(teTmp.m_pRow->m_ID.nDataID);
			}
		}
	}

	// Populate anDrugListIDs and astrDrugListNames
	//CString strFilter;
	anAllergyIDs.RemoveAll();
	astrAllergyNames.RemoveAll();
	/*
	for (i=0; i < anDataIDs.GetSize(); i++) {
		strFilter += FormatString("%d,", anDataIDs[i]);
	}
	*/

	//
	// Now populate the array
	//
	if (!anDataIDs.IsEmpty()) {
		_ConnectionPtr pCon;
		if(lpCon) pCon = lpCon;
		else pCon = GetRemoteData();
		//strFilter.TrimRight(",");
		// (c.haag 2007-02-02 16:19) - PLID 24561 - We no longer have a Name field in DrugList
		// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
		_RecordsetPtr prs = CreateParamRecordset(pCon, "SELECT AllergyT.ID, EMRDataT.Data AS Name FROM AllergyT "
			"LEFT JOIN EMRDataT ON AllergyT.EMRDataID = EMRDataT.ID "
			"WHERE EMRDataID IN ({INTARRAY}) ORDER BY EMRDataT.Data", anDataIDs);
		while (!prs->eof) {
			anAllergyIDs.Add(AdoFldLong(prs, "ID"));
			astrAllergyNames.Add(AdoFldString(prs, "Name", ""));
			prs->MoveNext();
		}
	}
}

void CEMNDetail::PopulateTableElementMap(CMap<__int64, __int64, int, int>& mapTable)
{
	// (c.haag 2007-08-20 08:59) - PLID 27118 - This function creates a map of table
	// elements. The lookup key is the table position, and the value is the index in
	// m_arTableElements.
	const int nTableElements = m_arTableElements.GetSize();
	for(int n = 0; n < nTableElements; n++) {
		const TableElement& te = m_arTableElements[n]; // (a.walling 2013-03-13 12:34) - PLID 55633 - Release mode does not elide this copy, so just use a reference
		const __int64 nKey = GetTableElementKey(te);
		mapTable[nKey] = n;
	}
}

// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
// because they are now only calculated in the API, and not in Practice code
/*
// (j.jones 2007-08-27 12:16) - PLID 27056 - added E/M element calculation functions
BOOL CEMNDetail::GetUseEMCoding()
{
	return m_bUseEMCoding;
}

// (j.jones 2007-08-27 12:16) - PLID 27056 - added E/M element calculation functions
long CEMNDetail::GetEMCategoryID()
{
	return m_nEMCodeCategoryID;
}

// (j.jones 2011-03-09 09:05) - PLID 42283 - added GetEMCodeUseTableCategories
EMCodeUseTableCategories CEMNDetail::GetEMCodeUseTableCategories()
{
	return m_eEMCodeUseTableCategories;
}
*/

// (j.jones 2007-08-27 12:16) - PLID 27056 - added E/M element calculation functions
// (j.jones 2011-03-09 09:05) - PLID 42283 - this function now updates an existing array of tracked categories
// (j.jones 2013-02-13 14:46) - PLID 54668 - this is obsolete now, it's in the API
// (a.walling 2013-03-21 09:49) - PLID 55804 - Deleting code, check history if you need it
/*
void CEMNDetail::CalculateEMElements(CArray<ChecklistTrackedCategoryInfo*, ChecklistTrackedCategoryInfo*> &aryTrackedCategories);

// (j.jones 2007-08-27 16:13) - PLID 27056 - added E/M element calculation functions
// (j.jones 2011-03-09 09:05) - PLID 42283 - this function now updates an existing array of tracked categories
// (j.jones 2013-02-13 14:46) - PLID 54668 - this is obsolete now, it's in the API
// (a.walling 2013-03-21 09:49) - PLID 55804 - Deleting code, check history if you need it
void CEMNDetail::CalculateEMTableElements(CArray<ChecklistTrackedCategoryInfo*, ChecklistTrackedCategoryInfo*> &aryTrackedCategories);
*/

// (j.jones 2008-01-14 10:59) - PLID 18709 - added GetPatientID
long CEMNDetail::GetPatientID()
{
	//better be safe and bury this in if statements
	if(m_pParentTopic) {
		if(m_pParentTopic->GetParentEMN()) {
			if(m_pParentTopic->GetParentEMN()->GetParentEMR()) {
				return m_pParentTopic->GetParentEMN()->GetParentEMR()->GetPatientID();
			}
		}
	}

	return -1;
}

// (z.manning, 01/22/2008) - PLID 28690 - Selects/unslects the hot spot for the given ID.
void CEMNDetail::ToggleImageHotSpotSelection(long nHotSpotID)
{
	for(int nHotSpotIndex = 0; nHotSpotIndex < m_aryImageHotSpots.GetSize(); nHotSpotIndex++)
	{
		if(m_aryImageHotSpots.GetAt(nHotSpotIndex)->GetID() == nHotSpotID) {
			break;
		}
	}
	if(nHotSpotIndex >= m_aryImageHotSpots.GetSize()) {
		ThrowNxException("CEMNDetail::ToggleImageHotSpotSelection - Unable to find hot spot %li on EMNDetail %li", nHotSpotID, GetID());
	}

	CArray<long,long> arynSelectedHotSpotIDs;
	GetImageSelectedHotSpotIDs(arynSelectedHotSpotIDs);
	if(IsHotSpotSelected(nHotSpotID))
	{
		// (z.manning, 01/22/2008) - The hot spot was selected so unselect it.
		BOOL bFound = FALSE;
		for(int nSelectedHotSpotIndex = 0; nSelectedHotSpotIndex < arynSelectedHotSpotIDs.GetSize(); nSelectedHotSpotIndex++) {
			if(arynSelectedHotSpotIDs.GetAt(nSelectedHotSpotIndex) == nHotSpotID) {
				arynSelectedHotSpotIDs.RemoveAt(nSelectedHotSpotIndex);
				bFound = TRUE;
				nSelectedHotSpotIndex--;
			}
		}

		if(!bFound) {
			ThrowNxException("CEMNDetail::ToggleImageHotSpotSelection - Hot spot %li not found as selected on detail '%s'", nHotSpotID, GetLabelText());
		}

		m_aryImageHotSpots[nHotSpotIndex]->SetSelected(false);
	}
	else
	{
		// (z.manning, 01/22/2008) - The hot spot was unselected, so set it to selected.
		for(int nSelectedHotSpotIndex = 0; nSelectedHotSpotIndex < arynSelectedHotSpotIDs.GetSize(); nSelectedHotSpotIndex++) {
			if(arynSelectedHotSpotIDs.GetAt(nSelectedHotSpotIndex) == nHotSpotID) {
				ThrowNxException("CEMNDetail::ToggleImageHotSpotSelection - Unexpectedly found selected hot spot %li on detail '%s'", nHotSpotID, GetLabelText());
			}
		}

		m_aryImageHotSpots[nHotSpotIndex]->SetSelected(true);
		arynSelectedHotSpotIDs.Add(nHotSpotID);
	}

	CEmrItemAdvImageState ais;
	ais.CreateFromSafeArrayVariant(m_aisState->AsSafeArrayVariant());
	ais.SetSelectedHotSpots(arynSelectedHotSpotIDs);
	RequestStateChange(ais.AsSafeArrayVariant());
}

// (z.manning, 01/22/2008) - PLID 28690 - Returns true if the specified hot spot is selected and false if it's not.
BOOL CEMNDetail::IsHotSpotSelected(long nHotSpotID)
{
	CArray<long,long> arynSelectedHotSpotIDs;
	GetImageSelectedHotSpotIDs(arynSelectedHotSpotIDs);
	for(int nHotSpotIndex = 0; nHotSpotIndex < arynSelectedHotSpotIDs.GetSize(); nHotSpotIndex++) {
		if(arynSelectedHotSpotIDs.GetAt(nHotSpotIndex) == nHotSpotID) {
			return TRUE;
		}
	}

	return FALSE;
}

void CEMNDetail::SetListElement(int nIndex, ListElement &le)
{
	m_arListElements.SetAt(nIndex, le);
}


// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

// (a.walling 2008-07-01 09:23) - PLID 30570 - Set the preview flags for this detail
// (a.walling 2009-01-07 15:35) - PLID 31961 - Option to only refresh self
void CEMNDetail::SetPreviewFlags(DWORD dwFlags, BOOL bRefreshParent, BOOL bOnlyRefreshSelf)
{
	try {
		if (m_nPreviewFlags != dwFlags) {
			// (a.walling 2008-10-23 10:05) - PLID 27552 - Refresh the parent detail as well if we were or now are a subdetail
			BOOL bWasSubDetail = IsSubDetail();
			m_nPreviewFlags = dwFlags;
			BOOL bRefreshSubDetail = IsSubDetail() != bWasSubDetail;

			SetUnsaved();

			if (bRefreshParent || bOnlyRefreshSelf) {
				if (m_pParentTopic) {
					if (m_pParentTopic->GetParentEMN()) {
						CWnd *pWnd = m_pParentTopic->GetParentEMN()->GetInterface();

						// refresh ourself
						if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
							pWnd->SendMessage(NXM_UPDATE_EMR_PREVIEW, (WPARAM)FALSE, (LPARAM)this);
						}

						if (bRefreshParent) {

							if (bRefreshSubDetail) {
								// refresh subdetail's parent topic
								CEMNDetail* pParentDetail = GetSubDetailParent();
								CEMRTopic* pParentTopic = NULL; 
								if (pParentDetail) {
									pParentTopic = pParentDetail->GetSubDetailParentTopic();

									if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
										if (pParentTopic) {
											pWnd->SendMessage(NXM_UPDATE_EMR_PREVIEW, (WPARAM)TRUE, (LPARAM)pParentTopic);
										} else {
											pWnd->SendMessage(NXM_UPDATE_EMR_PREVIEW, (WPARAM)FALSE, (LPARAM)pParentDetail);
										}
									}			
								}

								// refresh our real topic
								if (pParentTopic == NULL || (pParentTopic != m_pParentTopic)) {
									if(GetSubDetailParentTopic() != NULL && pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
										pWnd->SendMessage(NXM_UPDATE_EMR_PREVIEW, (WPARAM)TRUE, (LPARAM)m_pParentTopic);
									}
								}

								if (pParentTopic != NULL && (pParentTopic != m_pParentTopic)) {
									pParentTopic->RefreshHTMLVisibility();
								}
							}

							if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
								pWnd->SendMessage(NXM_UPDATE_EMR_PREVIEW, (WPARAM)TRUE, (LPARAM)m_pParentTopic);
							}
							m_pParentTopic->RefreshHTMLVisibility();
						}
					}
				}
			}
		}
	} NxCatchAll("Error setting preview flags");
}

// (j.jones 2008-07-21 08:22) - PLID 30779 - returns true if there are any undeleted problems on the detail,
// also can optionally take in an EMRDataID
// (c.haag 2009-05-19 12:44) - PLID 34311 - Use new EMR problem structure
BOOL CEMNDetail::HasProblems(long nEMRDataID /*= -1*/)
{
	try {

		//we call LoadContent() to load if it an existing detail that is as-yet unloaded,
		// Also we don't have problems on templates, and thus never need to call LoadContent() on a template.
		if(!m_bIsTemplateDetail && m_nEMRDetailID != -1) {

			//it is a saved detail, so load the content
			LoadContent();
		}

		for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblemLink* pLink = m_apEmrProblemLinks.GetAt(i);
			CEmrProblem *pProblem = pLink->GetProblem();
			if(pProblem != NULL && !pProblem->m_bIsDeleted && !pLink->IsDeleted()) {

				if(nEMRDataID != -1) {
					
					//looking for a problem on a specific list item on this detail
					// (c.haag 2009-05-28 09:38) - PLID 34311 - These members now exist at the link level
					if(pLink->GetType() == eprtEmrDataItem && pLink->GetDataID() == nEMRDataID) {

						return TRUE;
					}
				}
				else {

					//looking for any problem on this detail
					return TRUE;
				}
			}
		}

	}NxCatchAll("Error in CEMNDetail::HasProblems");

	return FALSE;
}

// (j.jones 2008-07-21 08:22) - PLID 30779 - returns true if there are only undeleted, closed problems on the detail,
// also can optionally take in an EMRDataID
// (c.haag 2009-05-19 12:44) - PLID 34311 - Use new EMR problem structure
BOOL CEMNDetail::HasOnlyClosedProblems(long nEMRDataID /*= -1*/)
{
	try {

		//we call LoadContent() to load if it an existing detail that is as-yet unloaded,
		// Also we don't have problems on templates, and thus never need to call LoadContent() on a template.
		if(!m_bIsTemplateDetail && m_nEMRDetailID != -1) {

			//it is a saved detail, so load the content
			LoadContent();
		}

		BOOL bHasProblems = FALSE;
		BOOL bHasOnlyClosed = TRUE;

		for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
			CEmrProblemLink* pLink = m_apEmrProblemLinks.GetAt(i);
			CEmrProblem *pProblem = pLink->GetProblem();
			if(pProblem != NULL && !pProblem->m_bIsDeleted) {

				BOOL bHasProblem = FALSE;

				if(nEMRDataID != -1) {
					
					//looking for a problem on a specific list item on this detail
					// (c.haag 2009-05-28 09:38) - PLID 34311 - These members now exist at the link level
					if(pLink->GetType() == eprtEmrDataItem && pLink->GetDataID() == nEMRDataID) {
						bHasProblem = TRUE;
					}
				}
				else {

					//looking for any problem on this detail
					bHasProblem = TRUE;
				}

				if(bHasProblem) {

					bHasProblems = TRUE;
					
					if(pProblem->m_nStatusID != 2) {
						bHasOnlyClosed = FALSE;
					}
				}
			}
		}

		if(bHasProblems && bHasOnlyClosed) {
			return TRUE;
		}
		else {
			return FALSE;
		}

	}NxCatchAll("Error in CEMNDetail::HasOnlyClosedProblems");

	return FALSE;
}

// (c.haag 2008-07-24 12:15) - PLID 30826 - Returns TRUE if there is at least one saved problem for this detail
// or any list items. This does not check deleted problems.
// (c.haag 2009-05-19 12:45) - PLID 34311 - Use new problem link structure
BOOL CEMNDetail::DoesDetailOrChildrenHaveSavedProblems()
{
	const int nProblems = m_apEmrProblemLinks.GetSize();
	for (int i=0; i < nProblems; i++) {
		CEmrProblem* p = m_apEmrProblemLinks[i]->GetProblem();
		if (p && !p->m_bIsDeleted && p->m_nID > 0) {
			return TRUE;
		}
	}
	return FALSE;
}

// (z.manning 2008-10-07 15:52) - PLID 31561 - Lab ID
void CEMNDetail::SetLabID(const long nLabID)
{
	m_varLabID = nLabID;
}

// (z.manning 2008-10-07 15:52) - PLID 31561 - Lab ID
_variant_t CEMNDetail::GetLabID()
{
	return m_varLabID;
}

BOOL CEMNDetail::IsLabDetail()
{
	return (m_varLabID.vt == VT_I4);
}

// (a.walling 2008-10-23 09:48) - PLID 27552 - Whether this item should be displayed underneath
// another detail or not.
BOOL CEMNDetail::IsSubDetail()
{
	if (m_nPreviewFlags & epfSubDetail) {
		if (GetSourceDetail() != NULL) {
			return TRUE;
		}
		if (GetSourceDetailID() != -1) {
			return TRUE;
		}
	}

	return FALSE;
}

// (a.walling 2008-10-23 09:58) - PLID 27552 - Returns the parent of the subdetail (if any)
CEMNDetail* CEMNDetail::GetSubDetailParent()
{
	CEMNDetail* pParentDetail = NULL;
	if (GetSourceDetail() != NULL) {
		pParentDetail = GetSourceDetail();
	} else if (GetSourceDetailID() != -1) {				
		if (m_pParentTopic && m_pParentTopic->GetParentEMN()) {
			pParentDetail = m_pParentTopic->GetParentEMN()->GetDetailByID(GetSourceDetailID());
		}
	}

	return pParentDetail;
}

// (a.walling 2008-10-24 10:36) - PLID 27552 - Easier function to get the topic this detail is actually displayed under
CEMRTopic* CEMNDetail::GetSubDetailParentTopic()
{
	if (m_nPreviewFlags & epfSubDetail) {
		CEMNDetail* pParentDetail = GetSubDetailParent();
		if (pParentDetail) {
			return pParentDetail->GetSubDetailParentTopic();
		} else {
			return m_pParentTopic;
		}
	} else {
		return m_pParentTopic;
	}
}

BOOL CEMNDetail::IsSignatureDetail()
{
	// (z.manning 2016-01-04 10:23) - PLID 67788 - We are checking a content variable here
	// so we need to ensure content is loaded.
	LoadContent();

	return (m_aisState->m_eitImageTypeOverride == itSignature);
}

void CEMNDetail::SetWindowless()
{
	if(m_pEmrItemAdvDlg) {
		//TES 9/16/2009 - PLID 35529 - How can we be set to windowless if we already have a window?!
		if(!m_bWindowless) {
			AfxThrowNxException("SetWindowless() called for CEMNDetail with a valid EmrItemAdvDlg");
		}
	}
	m_bWindowless = true;
}

// (z.manning 2009-09-23 09:19) - PLID 33612
void CEMNDetail::OpenLabEntryDialog()
{
	try
	{
		if(m_EMRInfoType != eitText) {
			AfxThrowNxException("CEMNDetail::OpenLabEntryDialog - Tried to open lab for non text type detail");
		}

		// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
		EnsureEmrItemAdvDlg(GetParentTopic()->GetTopicWndRaw());
		((CEmrItemAdvTextDlg*)m_pEmrItemAdvDlg)->OpenLabEntryDialog();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2011-11-10 17:31) - PLID 37093
void CEMNDetail::GetSortedNarrativeFieldArray(OUT CArray<NarrativeField>& aryNarrativeFields)
{
	if(m_varState.vt == VT_NULL || m_varState.vt == VT_EMPTY) {
		return;
	}

	CString strNxRichText = VarString(m_varState);
	strNxRichText.TrimRight();
	//TES 6/29/2012 - PLID 50854 - New function that handles HTML narratives
	GetNarrativeFieldArray(strNxRichText, aryNarrativeFields);
}

//TES 7/9/2012 - PLID 51359 - Call this function to make the narrative new (HTML narratives store DetailIDs and DataIDs, and have GUIDs that
// need to be updated).
void CEMNDetail::DereferenceNarrative()
{
	if(m_varState.vt != VT_BSTR) {
		return;
	}
	if(m_EMRInfoType != eitNarrative) {
		return;
	}

	CString strNarrativeText = VarString(m_varState);
	if(!::IsRtfNarrative(strNarrativeText)) {
		//TES 7/9/2012 - PLID 51359 - OK, this is an HTML narrative.  There's a utility in NexTech.COM that will fix that up for us, 
		// so call that.
		NexTech_COM::IHtmlUtilsPtr pUtils;
		pUtils.CreateInstance("NexTech_COM.HtmlUtils");
		if(pUtils == NULL) {
			AfxThrowNxException("Invalid NexTech_Com pointer in CEMNDetail::DereferenceNarrative()");
		}
		SetState(pUtils->GetDereferencedXml(_bstr_t(strNarrativeText)));
	}
}

// (a.walling 2009-11-17 09:35) - PLID 36365
// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
void CEMNDetail::GetNarrativeFieldSet(COleSafeArray& saFieldValues, const CString& strOverrideRichText, bool* pbForceUpdate)
{
	ASSERT(m_EMRInfoType == eitNarrative);

	if ( (m_EMRInfoType != eitNarrative) || (m_pParentTopic == NULL) || (m_pParentTopic->GetParentEMN() == NULL) ) {
		return;
	}

	CEMN* pEMN = m_pParentTopic->GetParentEMN();

	CArray<NarrativeField> arNarrativeFields;
	CString strNxRichText;
	if (strOverrideRichText.IsEmpty()) {
		strNxRichText = VarString(m_varState);
	} else {
		strNxRichText = strOverrideRichText;
	}
	strNxRichText.TrimRight();
	//TES 6/29/2012 - PLID 50854 - New function that handles HTML narratives
	GetNarrativeFieldArray(strNxRichText, arNarrativeFields);

	//First, make an array of all our narrative fields.
	CArray<CEMNDetail*,CEMNDetail*> arAllDetails;
	GetAllEMNDetails(arAllDetails, true);

	// (a.walling 2010-05-27 09:31) - PLID 38910
	bool bForceUpdate = false;
	for (int i = 0; i < arNarrativeFields.GetSize(); i++)
	{
		// (z.manning 2011-11-10 15:00) - PLID 37093 - Don't update procedure fields here.
		// (j.jones 2012-12-27 15:25) - PLID 54369 - the above item was never finished, and this created broken, half-working code
		/*
		if(IsProcedureNarrativeField(arNarrativeFields[i].strField))
		{
			arNarrativeFields[i].bIsValid = true;
		}
		else
		*/
		{
			bool bValueCausesForcedUpdate = false;
			GetValueForNarrative(arNarrativeFields[i], &arAllDetails, &bValueCausesForcedUpdate);

			if (bValueCausesForcedUpdate) {
				bForceUpdate = true;
			}
		}
	}

	// (a.walling 2010-05-27 09:32) - PLID 38910
	if (pbForceUpdate) {
		*pbForceUpdate = bForceUpdate;
	}

	{
		DWORD dwSize = arNarrativeFields.GetSize();
		DWORD dwDims[] = { dwSize, 2 };
		saFieldValues.Create(VT_BSTR, 2, dwDims);

		BSTR* pSafeArrayData = NULL;
		saFieldValues.AccessData((LPVOID *)&pSafeArrayData);


		for (DWORD i = 0; i < dwSize; i++) {
			NarrativeField& nf(arNarrativeFields[i]);
			// field 1
			pSafeArrayData[(i * 2) + 0] = _bstr_t(nf.strField).Detach();

			// field 2
			pSafeArrayData[(i * 2) + 1] = nf.bIsValid ? _bstr_t(nf.strValue).Detach() : NULL;
		}

		saFieldValues.UnaccessData();
	}
}

// (a.walling 2009-11-17 08:36) - PLID 36365
void CEMNDetail::UpdateNarrativeFields(OPTIONAL CEMRItemAdvPopupWnd* pPopupWnd /* = NULL*/, const CString& strOverrideRichText /*= ""*/)
{
	if ( (m_EMRInfoType != eitNarrative) || (m_pParentTopic == NULL) || (m_pParentTopic->GetParentEMN() == NULL) ) {
		return;
	}

	if (m_pParentTopic != NULL && m_pParentTopic->GetParentEMN() && m_pParentTopic->GetParentEMN()->IsLockedAndSaved()) {
		// (a.walling 2010-01-14 11:26) - PLID 36880 - Don't need to update any narrative fields since nothing can change anyway!
		return;
	}

	// prevent reentry
	m_bUpdatingNarrativeFields = true;

	// (a.walling 2009-12-08 13:29) - PLID 36225 - We will use the global singleton if the actual one is not available.
	/*
	CWnd *pTopicWnd = m_pParentTopic->GetInterface();
	try {
		EnsureEmrItemAdvDlg(pTopicWnd);
	} catch (...) {
		m_bUpdatingNarrativeFields = false;
		throw;
	}
	*/

	if (!m_bUpdatingNarrativeFields) {
		// we've already done the deed, return.
		return;
	}
	
	/*
	if (pNarrativeHost == NULL) {
		m_bUpdatingNarrativeFields = false;
		ASSERT(FALSE);
		return;
	}
	*/

	try {

		COleSafeArray saFieldValues;
		// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
		bool bForceUpdate = false;
		GetNarrativeFieldSet(saFieldValues, strOverrideRichText, &bForceUpdate);

		UpdateNarrativeFieldsWithValues(&saFieldValues, pPopupWnd, bForceUpdate);

		saFieldValues.Clear();
	} catch (...) {
		m_bUpdatingNarrativeFields = false;
		throw;
	}
	
	m_bUpdatingNarrativeFields = false;
}

// (z.manning 2011-11-11 09:46) - PLID 37093 - Moved some of the code from CEMNDetail::UpdateNarrativeFields here
void CEMNDetail::UpdateNarrativeFieldsWithValues(COleSafeArray *psaFieldValues, CEMRItemAdvPopupWnd* pPopupWnd, bool bForceUpdate)
{
	CEmrItemAdvNarrativeDlg* pNarrativeHost = (CEmrItemAdvNarrativeDlg*)m_pEmrItemAdvDlg;
	if (pNarrativeHost == NULL) {
		pNarrativeHost = GetGlobalEmrItemAdvNarrativeDlg(this, egiLoading);
	}

	if (pNarrativeHost) {
		CEmrItemAdvNarrativeDlg* pNarrativeDlg = (CEmrItemAdvNarrativeDlg*)pNarrativeHost;
		// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
		pNarrativeDlg->UpdateNarrativeFields(*psaFieldValues, bForceUpdate);
	}

	CEMRItemAdvPopupWnd* pSinglePopupWnd = NULL;
	if (m_pPopupDlg && m_pPopupDlg->GetEMRItemAdvPopupWnd()) {
		pSinglePopupWnd = m_pPopupDlg->GetEMRItemAdvPopupWnd();
		// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
		pSinglePopupWnd->UpdateNarrativeFields(*psaFieldValues, bForceUpdate);
	}
	
	if (pPopupWnd && pPopupWnd != pSinglePopupWnd) {
		// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
		pPopupWnd->UpdateNarrativeFields(*psaFieldValues, bForceUpdate);
	}
}

// (a.walling 2009-11-17 09:35) - PLID 36365
// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
void CEMNDetail::GetValueForNarrative(NarrativeField& nf, CArray<CEMNDetail*,CEMNDetail*>* pallDetails, bool* pbForceUpdate)
{	
	CString strValue;

	if ( (m_pParentTopic == NULL) || (m_pParentTopic->GetParentEMN() == NULL) ) {
		ASSERT(FALSE);
	}

	CEMN* pEMN = m_pParentTopic->GetParentEMN();

	// (a.walling 2010-04-01 10:44) - PLID 38013 - Consolidating some constant text strings
	if (nf.strField == NFIELD_ITEM_SPAWNING_TEXT) {
		GetItemSpawningTextValueForNarrative(nf, false);
		return;
	} else if (nf.strField == NFIELD_ITEM_SPAWNING_TEXT_FURTHEST) {
		GetItemSpawningTextValueForNarrative(nf, true);
		return;
	} else if (nf.strField == NFIELD_SPAWNING_ITEM) {
		GetSpawningItemValueForNarrative(nf, pallDetails);
		return;
	} else if (nf.strField == NFIELD_EMN_SPAWNING_TEXT) {
		// (a.walling 2010-03-29 10:25) - PLID 37923 - Special handling for EMN Spawning Text, which is ephemeral in the sense that we do not touch it if it is not available
		NarrativeMergeField nmf;
		if (pEMN->m_mapGenericMergeFields.Lookup(nf.strField, nmf)) {
			// first off, if bFilled is false, we do not touch the value and we will leave it as is.
			if (!nmf.bFilled) {
				nf.bIsValid = true;
			} else {
				if (pEMN->IsTemplate()) {
					// (a.walling 2010-05-26 18:06) - PLID 38910 - This should be set to "", not the name of the field!
					nf.strValue = "";
					nf.bIsValid = true;
				} else {
					nf.strValue = nmf.GetValue();
					nf.bIsValid = nf.strValue.IsEmpty() ? false : true;
				}
			}

			return;
		}
	}

	// (z.manning 2011-11-10 15:12) - PLID 37093 - Need to handle procedure fields too
	// (j.jones 2012-12-27 15:25) - PLID 54369 - the above item was never finished, and this created broken, half-working code
	/*
	if(IsProcedureNarrativeField(nf.strField)) {
		pEMN->GetProcedureFieldValue(&nf);
	}
	// check generic fields
	else
	*/
	if (pEMN->GetGenericMergeFieldValue(nf.strField, nf.strValue, nf.bIsValid)) {
		return;
	}
	else {
		// couldn't find in generic maps, so this must be a detail or detail list item (the horror)
		long nElementID = -1;
		// (a.walling 2010-01-22 10:46) - PLID 37027 - Need to keep track of the list element's selected state
		bool bElementSelected = false;
		CEMNDetail* pDetail = GetDetailFromNarrativeMergeField(nf.strField, &nElementID, &bElementSelected, pallDetails);
		//TES 2/25/2010 - PLID 37535 - Allow tables that use the Smart Stamp format to be in narratives
		if (IsDetailLinkable(pDetail)) {
			// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
			GetValueForNarrativeDetail(nf, pDetail, nElementID, bElementSelected, pbForceUpdate);
		} else {
			// could not find detail
			nf.bIsValid = false;
		}
	}
}

// (a.walling 2009-11-17 09:35) - PLID 36365
// (a.walling 2010-01-22 10:42) - PLID 37027 - Pass in the selected state of the element
// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
void CEMNDetail::GetValueForNarrativeDetail(NarrativeField& nf, CEMNDetail* pDetail, long nElementID, bool bElementSelected, bool* pbForceUpdate)
{
	if ( (m_pParentTopic == NULL) || (m_pParentTopic->GetParentEMN() == NULL) ) {
		return;
	}

	CEMN* pEMN = m_pParentTopic->GetParentEMN();

	if (pEMN->IsTemplate()) {
		// (a.walling 2010-05-26 18:06) - PLID 38910 - This should be set to "", not the name of the field!
		nf.strValue = "";
		nf.bIsValid = true;
		return;
	}

	// (a.walling 2010-05-27 09:28) - PLID 38910
	bool bCheckForceUpdate = false;
	if (nf.strValue == nf.strField) {
		bCheckForceUpdate = true;
	}

	if (nElementID == -1) {
		if (nf.strFlags.Find("s") == -1) {
			// not sentence format
			nf.strValue = pEMN->GetDataOutput(pDetail, NULL, false, false);
		} else {
			// sentence format
			nf.strValue = pEMN->GetSentence(pDetail, NULL, false, false, ecfParagraph);
		}

		// (a.walling 2010-03-29 17:56) - PLID 37420 - If an item has no data output (that is, the sentence format has no <data> field for example), 
		// then its name would always show in the RichTextEditor itself, as if it was an unfilled field. However, it may be filled. So, we work 
		// around this by setting the field to not be valid if the output is empty and the state is set.
		if (nf.strValue.IsEmpty() && pDetail->IsStateSet()) {
			nf.bIsValid = false;
		} else {
			nf.bIsValid = true;
		}
	} else {
		// (a.walling 2010-01-22 10:30) - PLID 37027 - Only fill out the info if the element is selected!!
		if (bElementSelected) {
			// it's a list item!
			if (nf.strFlags.Find("s") == -1) {
				// not sentence format
				nf.strValue = pEMN->GetElementDataOutput(pDetail, nElementID, NULL, false, false);
			} else {
				// sentence format
				nf.strValue = pEMN->GetElementSentence(pDetail, nElementID, NULL, false, false);
			}
		
			nf.bIsValid = true;
		} else {
			nf.strValue.Empty();
			
			nf.bIsValid = false;
		}
	}
	
	// (a.walling 2010-05-27 09:28) - PLID 38910
	if (bCheckForceUpdate && nf.strValue == nf.strField) {
		bCheckForceUpdate = false;
	}

	if (pbForceUpdate) {
		*pbForceUpdate = bCheckForceUpdate;
	}
}

// (a.walling 2009-11-17 09:35) - PLID 36366 - Return the most appropriate detail based on the field name
// (a.walling 2010-01-22 10:46) - PLID 37027 - Also need to keep track of the list element's selected state
CEMNDetail* CEMNDetail::GetDetailFromNarrativeMergeField(const CString& strField, OPTIONAL long* pnElementID, OPTIONAL bool* pbElementSelected, CArray<CEMNDetail*,CEMNDetail*>* pallDetails)
{
	if (m_pParentTopic == NULL || m_pParentTopic->GetParentEMN() == NULL) {
		return NULL;
	}

	CArray<CEMNDetail*,CEMNDetail*> localDetails;
	if (!pallDetails) {
		GetAllEMNDetails(localDetails, true);

		pallDetails = &localDetails;
	}

	// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
	long i = 0;

	long nMySpawnedGroupID = GetSpawnedGroupID();

	//First, try to find one with our spawned group id. 
	//This includes checking if our spawned group is -1; obviously we want to prioritize others with a group of -1.
	CEMNDetail *pSpawnedGroupDetailExact = NULL;
	CEMNDetail *pSpawnedGroupDetailNoCase = NULL;
	long nTotalDetailCount = pallDetails->GetSize();
	for (i=0; i < nTotalDetailCount && !pSpawnedGroupDetailExact; i++) {
		CEMNDetail *pCheck = pallDetails->GetAt(i);
		//TES 2/25/2010 - PLID 37535 - Allow tables that use the Smart Stamp format to be in narratives
		if(IsDetailLinkable(pCheck)) {
			if(pCheck->GetSpawnedGroupID() == nMySpawnedGroupID) {
				if (pCheck->GetLabelText() == strField) {
					pSpawnedGroupDetailExact = pCheck;
					pSpawnedGroupDetailNoCase = pCheck;
				} else if (!pSpawnedGroupDetailNoCase && pCheck->GetLabelText().CompareNoCase(strField) == 0) {
					pSpawnedGroupDetailNoCase = pCheck;
				}
			}
		}
	}

	if (pSpawnedGroupDetailExact) {
		// we have an exact match
		return pSpawnedGroupDetailExact;
	} else if (pSpawnedGroupDetailNoCase) {
		// we have a match on all but case.
		return pSpawnedGroupDetailNoCase;
	}

	{
		//OK, just match the field name.
		CEMNDetail *pAnyDetailExact = NULL;
		CEMNDetail *pAnyDetailNoCase = NULL;
		for(i=0; i < nTotalDetailCount && !pAnyDetailExact; i++) {
			CEMNDetail *pCheck = pallDetails->GetAt(i);
			//TES 2/25/2010 - PLID 37535 - Allow tables that use the Smart Stamp format to be in narratives
			if(IsDetailLinkable(pCheck)) {
				long nCheckSpawnedGroupID = pCheck->GetSpawnedGroupID();
				// (a.walling 2010-01-06 14:53) - PLID 36777
				//
				// If we are not in a spawned group, we can match anything
				//   (we already matched against other non-spawned group details above)
				//
				// If we are in a spawned group, we can only match anything that is not in a spawned group
				//   (we already matched against details in the same spawned group above)
				//
				if ( (nMySpawnedGroupID == -1) || (nCheckSpawnedGroupID == -1) ) {
					if (nCheckSpawnedGroupID != nMySpawnedGroupID) { // we already know nothing exists that matches the spawned group
						if (pCheck->GetLabelText() == strField) {
							pAnyDetailExact = pCheck;
							pAnyDetailNoCase = pCheck;
						} else if (!pAnyDetailNoCase && pCheck->GetLabelText().CompareNoCase(strField) == 0) {
							pAnyDetailNoCase = pCheck;
						}
					}
				}
			}
		}

		if (pAnyDetailExact) {
			// we have an exact match
			return pAnyDetailExact;
		} else if (pAnyDetailNoCase) {
			// we have a match on all but case.
			return pAnyDetailNoCase;
		}
	}

	// if we got here, no match. Check to see if it may be a subfield.
	return GetDetailFromNarrativeMergeSubfield(strField, pnElementID, pbElementSelected, pallDetails);
}

// (a.walling 2009-11-17 09:35) - PLID 36366 - Return the most appropriate detail based on the field + subfield name
// (a.walling 2010-01-22 10:46) - PLID 37027 - Also need to keep track of the list element's selected state
CEMNDetail* CEMNDetail::GetDetailFromNarrativeMergeSubfield(const CString& strField, OPTIONAL long* pnElementID, OPTIONAL bool* pbElementSelected, CArray<CEMNDetail*,CEMNDetail*>* pallDetails)
{
	if (m_pParentTopic == NULL || m_pParentTopic->GetParentEMN() == NULL) {
		return NULL;
	}

	const CString& strLinkedField(strField);
	long nDash = strLinkedField.Find(" - ");

	if (nDash == -1) {
		return NULL;
	}

	CArray<CEMNDetail*,CEMNDetail*> localDetails;
	if (!pallDetails) {
		GetAllEMNDetails(localDetails, true);

		pallDetails = &localDetails;
	}

	long nMySpawnedGroupID = GetSpawnedGroupID();

	// (j.jones 2006-03-10 17:29) - PLID 19654 - if no item matches the returned name,
	// it is plausible we clicked on a subfield, so we must parse the name
	CEMNDetail* pSubfieldDetailExact = NULL;
	CEMNDetail* pSubfieldDetailNoCase = NULL;
	long nElementIDExact = -1;
	long nElementIDNoCase = -1;
	bool bElementSelectedExact = false;
	bool bElementSelectedNoCase = false;

	while(!pSubfieldDetailExact && nDash != -1) {
		CString strInfo = strLinkedField.Left(nDash);
		strInfo.TrimRight();
		CString strData = strLinkedField.Right(strLinkedField.GetLength() - nDash - 3);
		strData.TrimLeft();
		// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - for() loops
		long i = 0;

		//First, try to find one with our spawned group id.
		long nTotalDetailCount = pallDetails->GetSize();
		for (i=0; i < nTotalDetailCount && !pSubfieldDetailExact; i++) {
			CEMNDetail *pCheck = pallDetails->GetAt(i);

			//if we're clicking on a sub-field, then it can only be a list type
			if(pCheck->m_EMRInfoType == eitSingleList || pCheck->m_EMRInfoType == eitMultiList) {
				if(pCheck->GetSpawnedGroupID() == nMySpawnedGroupID) {
					bool bExactMatch = pCheck->GetLabelText() == strInfo;
					bool bNoCaseMatch = bExactMatch ? true : pCheck->GetLabelText().CompareNoCase(strInfo) == 0;

					if (bExactMatch || bNoCaseMatch) {
						//the label matches, but does the data?
						for(int j=0; j<pCheck->GetListElementCount() && !pSubfieldDetailExact; j++) {
							// (a.walling 2010-01-22 10:51) - PLID 37027 - Get the selected state
							ListElement le(pCheck->GetListElement(j));

							if (le.strName == strData) {
								pSubfieldDetailExact = pCheck;
								pSubfieldDetailNoCase = pCheck;

								nElementIDExact = le.nID;
								bElementSelectedExact = le.bIsSelected ? true : false;
							} else if (!pSubfieldDetailNoCase && le.strName.CompareNoCase(strData) == 0) {
								pSubfieldDetailNoCase = pCheck;

								nElementIDNoCase = le.nID;
								bElementSelectedNoCase = le.bIsSelected ? true : false;
							}
						}
					}
				}
			}
		}


		if (pSubfieldDetailExact) {
			// we have an exact match
			if (pnElementID) {
				*pnElementID = nElementIDExact;
			}
			// (a.walling 2010-01-22 10:51) - PLID 37027 - Return the selected state
			if (pbElementSelected) {
				*pbElementSelected = bElementSelectedExact;
			}
			return pSubfieldDetailExact;
		} else if (pSubfieldDetailNoCase) {
			// we have a match on all but case.
			if (pnElementID) {
				*pnElementID = nElementIDNoCase;
			}
			// (a.walling 2010-01-22 10:51) - PLID 37027 - Return the selected state
			if (pbElementSelected) {
				*pbElementSelected = bElementSelectedNoCase;
			}
			return pSubfieldDetailNoCase;
		}

		//OK, nothing yet, just match the field name.
		for(i=0; i < nTotalDetailCount && !pSubfieldDetailExact; i++) {
			CEMNDetail *pCheck = pallDetails->GetAt(i);

			//if we're clicking on a sub-field, then it can only be a list type
			if(pCheck->m_EMRInfoType == eitSingleList || pCheck->m_EMRInfoType == eitMultiList) {
				long nCheckSpawnedGroupID = pCheck->GetSpawnedGroupID();
				// (a.walling 2010-01-06 14:53) - PLID 36777
				//
				// If we are not in a spawned group, we can match anything
				//   (we already matched against other non-spawned group details above)
				//
				// If we are in a spawned group, we can only match anything that is not in a spawned group
				//   (we already matched against details in the same spawned group above)
				//
				if ( (nMySpawnedGroupID == -1) || (nCheckSpawnedGroupID == -1) ) {
					if(pCheck->GetSpawnedGroupID() != nMySpawnedGroupID) { // we already know nothing exists that matches the spawned group
						bool bExactMatch = pCheck->GetLabelText() == strInfo;
						bool bNoCaseMatch = bExactMatch ? true : pCheck->GetLabelText().CompareNoCase(strInfo) == 0;

						if (bExactMatch || bNoCaseMatch) {
							//the label matches, but does the data?
							for(int j=0; j<pCheck->GetListElementCount() && !pSubfieldDetailExact; j++) {								
								// (a.walling 2010-01-22 10:51) - PLID 37027 - Get the selected state
								ListElement le(pCheck->GetListElement(j));

								if (le.strName == strData) {
									pSubfieldDetailExact = pCheck;
									pSubfieldDetailNoCase = pCheck;

									nElementIDExact = le.nID;
									bElementSelectedExact = le.bIsSelected ? true : false;
								} else if (!pSubfieldDetailNoCase && le.strName.CompareNoCase(strData) == 0) {
									pSubfieldDetailNoCase = pCheck;

									nElementIDNoCase = le.nID;
									bElementSelectedNoCase = le.bIsSelected ? true : false;
								}
							}
						}
					}
				}
			}
		}

		if (pSubfieldDetailExact) {
			// we have an exact match
			if (pnElementID) {
				*pnElementID = nElementIDExact;
			}
			// (a.walling 2010-01-22 10:51) - PLID 37027 - Return the selected state
			if (pbElementSelected) {
				*pbElementSelected = bElementSelectedExact;
			}
			return pSubfieldDetailExact;
		} else if (pSubfieldDetailNoCase) {
			// we have a match on all but case.
			if (pnElementID) {
				*pnElementID = nElementIDNoCase;
			}
			// (a.walling 2010-01-22 10:51) - PLID 37027 - Return the selected state
			if (pbElementSelected) {
				*pbElementSelected = bElementSelectedNoCase;
			}
			return pSubfieldDetailNoCase;
		}

		//found nothing, try next dash, if one exists
		nDash = strLinkedField.Find(" - ", nDash+3);
	}

	return NULL;
}

// (a.walling 2009-11-17 11:31) - PLID 36365
void CEMNDetail::GetSpawningItemValueForNarrative(NarrativeField& nf, CArray<CEMNDetail*,CEMNDetail*>* pallDetails)
{
	if (m_pParentTopic == NULL || m_pParentTopic->GetParentEMN() == NULL) {
		return;
	}

	// (a.walling 2010-04-01 10:44) - PLID 38013 - Consolidating some constant text strings

	if(m_pParentTopic->GetParentEMN()->IsTemplate()) {
		//The spawning item for this narrative.
		CEmrDetailSource edsSpawning(CEmrDetailSource::dstUnknownSource, -1);
		if(GetSpawnedGroupID() != -1) {
			// (z.manning, 01/25/2007) - PLID 24383 - Make sure the action has not been deleted.
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			_RecordsetPtr rsSourceAction = CreateParamRecordset("SELECT SourceType, SourceID FROM EmrActionsT WHERE ID = {INT} AND Deleted = 0 ", GetSpawnedGroupID());
			if(!rsSourceAction->eof) {
				edsSpawning.m_edstSourceType = CEmrDetailSource::CalcSourceType((EmrActionObject)AdoFldLong(rsSourceAction,"SourceType"));
				edsSpawning.m_nSourceID = AdoFldLong(rsSourceAction, "SourceID");
			}
		}
	

		CArray<CEMNDetail*,CEMNDetail*> localDetails;
		if ((edsSpawning.m_edstSourceType == CEmrDetailSource::dstSpawnedByInfo) || (edsSpawning.m_edstSourceType == CEmrDetailSource::dstSpawnedByData) ) {
			if (!pallDetails) {
				GetAllEMNDetails(localDetails, true);
				pallDetails = &localDetails;
			}
		}

		if(edsSpawning.m_nSourceID == -1) {
			//TES 8/22/2007 - PLID 18492 - If we're on a template (which we are), we still want to show the name of the 
			// field here.
			//pDetail->AddFilledMergeField(NFIELD_SPAWNING_ITEM, "", "", false, true, false);
			// (a.walling 2010-04-01 10:44) - PLID 38013 - Consolidating some constant text strings
			// (a.walling 2010-05-26 18:06) - PLID 38910 - This should be set to "", not the name of the field!
			nf.strValue = "";
			nf.bIsValid = true;
		}
		else if(edsSpawning.m_edstSourceType == CEmrDetailSource::dstSpawnedByInfo) {

			//Find the item it was spawned by.
			// (j.jones 2007-03-09 09:59) - PLID 25142 - use EMRInfoMasterID
			long nEMRInfoMasterID = -2; //-2, so as not to be equal to the unused value of -1, though that should be impossible
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT EMRInfoMasterID FROM EMRInfoT WHERE ID = {INT}", edsSpawning.m_nSourceID);
			if(!rs->eof) {
				nEMRInfoMasterID = AdoFldLong(rs, "EMRInfoMasterID",-2);
			}
			rs->Close();
			CEMNDetail *pSpawning = NULL;
			long nTotalDetailCount = pallDetails->GetCount();
			for(int nItem = 0; nItem < nTotalDetailCount; nItem++) {
				CEMNDetail *pCheckDetail = pallDetails->GetAt(nItem);
				if(pCheckDetail->m_nEMRInfoMasterID == nEMRInfoMasterID)
					pSpawning = pCheckDetail;
			}
			if(pSpawning) {
				// (c.haag 2007-03-28 16:04) - PLID 25397 - GetSentence calls GetDataOutput, so pass in
				// the result of our call to GetDataOutput to GetSentence to avoid that redundant call
				//CString strDataOutput = GetDataOutput(pSpawning, NULL, false, false);
				//AddFilledMergeField(NFIELD_SPAWNING_ITEM, strDataOutput, GetSentence(pSpawning, NULL, false, false, ecfParagraph, strDataOutput), false, true, false, ub);
				GetValueForNarrativeDetail(nf, pSpawning);
			}
			else {
				//???
				ASSERT(FALSE);
				//AddFilledMergeField(NFIELD_SPAWNING_ITEM, edsSpawning.GetDisplayName(), edsSpawning.GetDisplayName(), false, true, false, ub);
				nf.strValue = edsSpawning.GetDisplayName();
				nf.bIsValid = true;
			}
		}
		else if(edsSpawning.m_edstSourceType == CEmrDetailSource::dstSpawnedByData) {
			//Find the item it was spawned by.
			// (j.jones 2007-03-09 09:59) - PLID 25142 - use EMRInfoMasterID
			long nEMRInfoMasterID = -2; //-2, so as not to be equal to the unused value of -1, though that should be impossible
			// (a.walling 2010-11-08 12:12) - PLID 40965 - Parameterized
			_RecordsetPtr rs = CreateParamRecordset("SELECT EMRInfoMasterID FROM EMRInfoT "
				"WHERE ID IN (SELECT EMRInfoID FROM EMRDataT WHERE ID = {INT})", edsSpawning.m_nSourceID);
			if(!rs->eof) {
				nEMRInfoMasterID = AdoFldLong(rs, "EMRInfoMasterID",-2);
			}
			rs->Close();
			CEMNDetail *pSpawning = NULL;
			long nTotalDetailCount = pallDetails->GetCount();
			for(int nItem = 0; nItem < nTotalDetailCount; nItem++) {
				CEMNDetail *pCheckDetail = pallDetails->GetAt(nItem);
				if(pCheckDetail->m_nEMRInfoMasterID == nEMRInfoMasterID)
					pSpawning = pCheckDetail;
			}
			if(pSpawning) {
				//AddFilledMergeField(NFIELD_SPAWNING_ITEM, GetElementDataOutput(pSpawning, edsSpawning.m_nSourceID, NULL, false, false), GetElementSentence(pSpawning, edsSpawning.m_nSourceID, NULL, false, false), false, true, false, ub);
				// (a.walling 2010-01-22 10:45) - PLID 37027 - If we are spawned from this, then it must be selected
				GetValueForNarrativeDetail(nf, pSpawning, edsSpawning.m_nSourceID, true);
			}
			else {
				//???
				// (c.haag 2007-09-12 10:51) - PLID 27360 - This used to trip an assertion if pSpawning
				// was NULL AND pDetail cannot use this EMN to maintain its narrative fields, which should
				// be extraordinarily rare if not impossible. We do not know for certain why it would be
				// NULL, but being so does not appear to cause any critical problems. The patch-equivalent
				// code is tripped, and more information may be found in the PL notes.
				//ASSERT(FALSE);
				//AddFilledMergeField(NFIELD_SPAWNING_ITEM, edsSpawning.GetDisplayName(), edsSpawning.GetDisplayName(), false, true, false, ub);
				
				nf.strValue = edsSpawning.GetDisplayName();
				nf.bIsValid = true;
			}
		}
		else {
			//AddFilledMergeField(NFIELD_SPAWNING_ITEM, edsSpawning.GetDisplayName(), edsSpawning.GetDisplayName(), false, true, false, ub);
			
			nf.strValue = edsSpawning.GetDisplayName();
			nf.bIsValid = true;
		}
	}
	else {
		// (a.walling 2009-11-25 09:05) - PLID 29071 - Spawning Item does not work. Pre-existing item. Probably hasn't worked for years.
//#pragma TODO("What about spawning item on a non-template?? Spawning Item doesn't even seem to work currently!!")		

		//AddAvailableMergeField(NFIELD_SPAWNING_ITEM, false, true, false, ub);
		nf.strValue = "";
		nf.bIsValid = false;
	}
}

// (a.walling 2010-04-01 12:13) - PLID 38013 - Gets the data from the item that directly or indirectly spawned this detail
void CEMNDetail::GetItemSpawningTextValueForNarrative(NarrativeField& nf, bool bFurthest)
{
	if (GetParentEMN() && GetParentEMN()->IsTemplate()) {
		nf.strValue = nf.strField;
		nf.bIsValid = true;
		return;
	}

	SourceActionInfo sai;
	CEMNDetail* pSpawningItem = FindSpawningItem(bFurthest, sai);

	if (pSpawningItem) {
		ASSERT(!sai.IsBlank());
		
		CEMN* pEMN = pSpawningItem->GetParentEMN();

		if (pEMN) {
			bool bFound = false;

			// ensure our table rows and etc are available
			pSpawningItem->LoadContent(FALSE, NULL);

			if (pSpawningItem->IsSmartStampTable()) {

				TableRow* pCurrentTableRow = NULL;

				// may not always be available in memory, so first just try to get it from what is loaded
				if (sai.GetDetailStampID() != -1) {
					EmrDetailImageStamp* pEmrDetailImageStamp = pSpawningItem->GetDetailImageStampByID(sai.GetDetailStampID());

					if (pEmrDetailImageStamp) {
						pCurrentTableRow = pSpawningItem->GetRowByDetailStamp(pEmrDetailImageStamp);
					}
				}

				if (pCurrentTableRow == NULL) {
					// this will only be available in the initial spawn
					pCurrentTableRow = sai.GetTableRow();
				}

				if (NULL != pCurrentTableRow) {
					// special case -- if a smart stamp table, just get the row we are interested in.						
					if (nf.strFlags.Find("s") == -1) {
						nf.strValue = pEMN->GetDataOutput(pSpawningItem, NULL, false, false, NULL, ecfParagraph, NULL, NULL, &pCurrentTableRow->m_ID);
					} else {
						CString strDataOutput = pEMN->GetDataOutput(pSpawningItem, NULL, false, false, NULL, ecfParagraph, NULL, NULL, &pCurrentTableRow->m_ID);
						nf.strValue = pEMN->GetSentence(pSpawningItem, NULL, false, false, ecfParagraph, strDataOutput);
					}

					nf.bIsValid = true;
				
					bFound = true;
				} else {					
					if (nf.strFlags.Find("s") == -1) {
						nf.strValue = pEMN->GetDataOutput(pSpawningItem, NULL, false, false, NULL, ecfParagraph, NULL, NULL);
					} else {
						nf.strValue = pEMN->GetSentence(pSpawningItem, NULL, false, false, ecfParagraph, NULL, NULL, NULL);
					}

					nf.bIsValid = true;
				}
			} else {
				if (nf.strFlags.Find("s") == -1) {
					nf.strValue = pEMN->GetDataOutput(pSpawningItem, NULL, false, false, NULL, ecfParagraph, NULL, NULL);
				} else {
					nf.strValue = pEMN->GetSentence(pSpawningItem, NULL, false, false, ecfParagraph, NULL, NULL, NULL);
				}

				nf.bIsValid = true;
			
				bFound = true;
			}
		}
	}
}

// (z.manning 2011-11-04 09:08) - PLID 42765 - Added an overload without the source action info param
CEMNDetail* CEMNDetail::FindSpawningItem(bool bFurthest)
{
	SourceActionInfo saiDummy;
	return FindSpawningItem(bFurthest, saiDummy);
}

CEMNDetail* CEMNDetail::FindSpawningItem(bool bFurthest, SourceActionInfo& sai)
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
		// well, this detail is not directly spawned. But perhaps it is indirectly spawned? We shall
		// check the topic.
		
		CEMRTopic* pParentTopic = GetOriginalParentTopic();
		if (!pParentTopic) {
			pParentTopic = GetParentTopic();
		}

		if (pParentTopic) {
			SourceActionInfo saiTopic;
			CEMNDetail* pTopicSpawningItem = pParentTopic->FindSpawningItem(bFurthest, saiTopic);
			if (pTopicSpawningItem) {
				sai = saiTopic;
				return pTopicSpawningItem;
			}
		}
	}

	// no luck
	return NULL;
}

void CEMNDetail::GetAllEMNDetails(CArray<CEMNDetail*,CEMNDetail*>& arAllDetails, bool bPrioritizeThisTopic)
{
	if (m_pParentTopic == NULL || m_pParentTopic->GetParentEMN() == NULL) {
		return;
	}

	CArray<CEMNDetail*,CEMNDetail*> arMostDetails;
	// (a.walling 2009-11-17 08:40) - PLID 36365 - Prioritize same-topic and sub-topics before other topics.
	// This will ignore m_pParentTopic
	if (bPrioritizeThisTopic) {
		m_pParentTopic->GenerateEMNDetailArray(&arAllDetails);
		m_pParentTopic->GetParentEMN()->GenerateTotalEMNDetailArray(&arMostDetails, m_pParentTopic);
		SortDetailArray(arMostDetails);
	} else {
		m_pParentTopic->GetParentEMN()->GenerateTotalEMNDetailArray(&arAllDetails);
	}

	SortDetailArray(arAllDetails);

	if (!arMostDetails.IsEmpty()) {
		arAllDetails.Append(arMostDetails);
		arMostDetails.RemoveAll();
	}
}

// (a.walling 2009-11-19 14:55) - PLID 36369 - Might this narrative contain this detail?
bool CEMNDetail::NarrativeMayHaveDetailField(CEMNDetail* pDetail)
{
	// (z.manning 2011-11-07 09:32) - PLID 42765 - We also need to check the narrative for the detail that spawned
	// it because it may have the spawned items field in it.
	CEMNDetail *pSpawningDetail = pDetail->FindSpawningItem(false);
	if(pSpawningDetail != NULL) {
		if(NarrativeMayHaveDetailField(pSpawningDetail->GetLabelText(), pSpawningDetail->GetSpawnedGroupID())) {
			return true;
		}
	}

	return NarrativeMayHaveDetailField(pDetail->GetLabelText(), pDetail->GetSpawnedGroupID());
}

// (a.walling 2009-11-19 14:55) - PLID 36369 - Might this narrative contain this detail?
bool CEMNDetail::NarrativeMayHaveDetailField(const CString& strField, long nSpawnedGroupID)
{
	long nMySpawnedGroupID = GetSpawnedGroupID();
	if (nMySpawnedGroupID != -1 && nSpawnedGroupID != -1 && nMySpawnedGroupID != nSpawnedGroupID) {
		return false;
	}

	CArray<NarrativeField> arNarrativeFields;
	CString strNxRichText = VarString(m_varState);
	strNxRichText.TrimRight();
	//TES 6/29/2012 - PLID 50854 - New function that handles HTML narratives
	GetNarrativeFieldArray(strNxRichText, arNarrativeFields);

	for (int i = 0; i < arNarrativeFields.GetSize(); i++)
	{
		NarrativeField& nf(arNarrativeFields[i]);

		if (strField.CompareNoCase(nf.strField) == 0) {
			return true;
		} else {
			long nDashIndex = nf.strField.Find(" - ");

			if (nDashIndex != -1 && _strnicmp(strField, nf.strField, min(nDashIndex, strField.GetLength())) == 0) {
				// nf.strField begins with strField
				return true;
			}
		}
	}

	return false;
}

// (a.walling 2009-11-17 09:35) - PLID 36365
void CEMNDetail::GetNarrativeLWFields(CMap<CString, LPCTSTR, long, long&>& mapFields, const CString& strOverrideRichText)
{
	CArray<NarrativeField> arNarrativeFields;
	CString strNxRichText;
	if (strOverrideRichText.IsEmpty()) {
		strNxRichText = VarString(m_varState);
	} else {
		strNxRichText = strOverrideRichText;
	}
	strNxRichText.TrimRight();
	//TES 6/29/2012 - PLID 50854 - New function that handles HTML narratives
	GetNarrativeFieldArray(strNxRichText, arNarrativeFields);

	for (int i = 0; i < arNarrativeFields.GetSize(); i++) {
		NarrativeField& nf(arNarrativeFields[i]);

		if (nf.strFlags.Find("w") != -1) {
			// we got one!
			mapFields[nf.strField]++;
		}
	}
}

// (a.walling 2009-11-18 13:15) - PLID 36367 - Sort based on field names
// (a.walling 2013-10-01 11:45) - PLID 58829 - Now using a functor predicate
struct CompareDetailFieldNames
	: public std::binary_function<CEMNDetail*, CEMNDetail*, bool>
{
	bool operator()(CEMNDetail* pDetailL, CEMNDetail* pDetailR)
	{
		if (pDetailL == pDetailR) {
			return false;
		}

		if (pDetailL && pDetailR) {
			return pDetailL->GetLabelText().CompareNoCase(pDetailR->GetLabelText()) < 0;
		} else {
			if (pDetailR) {
				// left is null, right is not, so we can say null < not-null
				return true;
			} else {
				// either right is null or both are null. either way, null is not less than null, and null is not less than not null.
				return false;
			}
		}
	}
};

// (a.walling 2009-11-19 14:21) - PLID 36367 - Requesting list of various merge fields from the host
void CEMNDetail::PopulateAvailableNarrativeFields(COleSafeArray& saFields, bool bIncludeLetterWriting, bool bIncludeEMR, bool bIncludeListItems)
{	
	if (m_pParentTopic == NULL || m_pParentTopic->GetParentEMN() == NULL) {
		return;
	}

	POSITION pos;

	struct AvailableNarrativeField
	{
		CString strField;
		CString strFlags;
		// (z.manning 2011-11-10 10:36) - PLID 46382 - Added a member for the actual field name so that we can display
		// something more user-friend in the menu if necessary. If the actual field name is blank then we use the display
		// name (strField) as the actual name.
		CString strActualField;
		
		//TES 7/5/2012 - PLID 50855 - We track the DetailID and DataID as well
		long nDetailID;
		long nDataID;

		AvailableNarrativeField() {
			nDetailID = nDataID = -1;
		}
	};

	CList<AvailableNarrativeField, AvailableNarrativeField&> listFields;
	
	// add generic fields
	if (bIncludeEMR)
	{
		CNarrativeMergeFieldMap& mapGenericFields(m_pParentTopic->GetParentEMN()->m_mapGenericMergeFields);
		pos = mapGenericFields.GetStartPosition();
		while (pos) {
			AvailableNarrativeField af;
			NarrativeMergeField nmf;
			mapGenericFields.GetNextAssoc(pos, af.strField, nmf);

			if (nmf.bIsLWMergeField) {
				continue;
			}

			// (a.walling 2010-04-01 10:00) - PLID 37923 - Put this 'special' field in its own group
			if (nmf.strName == NFIELD_EMN_SPAWNING_TEXT) {
				continue;
			}
		
			POSITION posInsert = listFields.GetTailPosition();
			while (posInsert) {
				// (a.walling 2010-01-08 11:50) - PLID 36819 - Now compares by case if case insensitive matches. Also the POSITION was incorrect leading to bad ordering.
				POSITION posActualInsert = posInsert;
				AvailableNarrativeField& afExisting(listFields.GetPrev(posInsert));

				int nCompare = af.strField.CompareNoCase(afExisting.strField);
				if (nCompare == 0) {
					nCompare = af.strField.Compare(afExisting.strField);
				}
				if (nCompare > 0) {
					posInsert = posActualInsert;
					break;
				}
			}
			if (posInsert) {
				listFields.InsertAfter(posInsert, af);
			} else {
				listFields.AddHead(af);
			}
		}

		// put in a separator (the generic fields are always available)
		if (listFields.GetCount() > 0) {
			AvailableNarrativeField afSeparator;
			listFields.AddTail(afSeparator);
		}

		// (z.manning 2011-11-10 14:18) - PLID 37093 - Handle procedure merge fields
		// (z.manning 2011-11-11 12:53) - PLID 37093 - THIS FEATURE WAS DESCOPED (leaving code for when we finish it)
		/*{
			AvailableNarrativeField afProc;
			afProc.strField = "Alternatives"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"Alternatives"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"Alternatives"; listFields.AddTail(afProc);
			afProc.strField = "Bandages"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"Bandages"; listFields.AddTail(afProc);
			afProc.strField = "Complications"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"Complications"; listFields.AddTail(afProc);
			afProc.strField = "Consent"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"Consent"; listFields.AddTail(afProc);
			afProc.strField = "Consent - Alt. Language"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"AltConsent"; listFields.AddTail(afProc);
			afProc.strField = "Hospital Stay"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"HospitalStay"; listFields.AddTail(afProc);
			afProc.strField = "Mini-Description"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"MiniDescription"; listFields.AddTail(afProc);
			afProc.strField = "PostOp"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"PostOp"; listFields.AddTail(afProc);
			afProc.strField = "PreOp"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"PreOp"; listFields.AddTail(afProc);
			afProc.strField = "Procedure Details"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"ProcDetails"; listFields.AddTail(afProc);
			afProc.strField = "Recovery"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"Recovery"; listFields.AddTail(afProc);
			afProc.strField = "Risks"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"Risks"; listFields.AddTail(afProc);
			afProc.strField = "Showering"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"Showering"; listFields.AddTail(afProc);
			afProc.strField = "Special Diet"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"SpecialDiet"; listFields.AddTail(afProc);
			afProc.strField = "The Day Of"; afProc.strActualField = CString(NFIELD_PROCEDURE_BASE)+"TheDayOf"; listFields.AddTail(afProc);

			// (z.manning 2011-11-10 14:35) - PLID 37093 - Need to load the custom section field names from data
			_RecordsetPtr prsCustomSectionNames = CreateParamRecordset(
				"SELECT ID, Name FROM CustomFieldsT WHERE ID >= {INT} AND ID <= {INT} ORDER BY ID"
				, 70, 79);
			for(short nCustomSectionIndex = 1; nCustomSectionIndex <= 10; nCustomSectionIndex++)
			{
				CString strDataField = "CustomSection" + AsString(nCustomSectionIndex);
				CString strDisplayField;
				if(!prsCustomSectionNames->eof) {
					const long nCustomFieldID = AdoFldLong(prsCustomSectionNames, "ID");
					if((nCustomFieldID - 69) == nCustomSectionIndex) {
						strDisplayField = AdoFldString(prsCustomSectionNames, "Name");
						prsCustomSectionNames->MoveNext();
					}
				}

				if(strDisplayField.IsEmpty()) {
					strDisplayField = "Custom Section " + AsString(nCustomSectionIndex);
				}

				afProc.strField = strDisplayField;
				afProc.strActualField = NFIELD_PROCEDURE_BASE + strDataField;
				listFields.AddTail(afProc);
			}

			AvailableNarrativeField afSeparator;
			listFields.AddTail(afSeparator);
		}*/

		// (a.walling 2010-04-01 10:04) - PLID 37923 - Include our 'special' fields.	
		{
			// (a.walling 2010-04-01 10:44) - PLID 38013 - Consolidating some constant text strings
			NarrativeMergeField nmf;
			if (mapGenericFields.Lookup(NFIELD_EMN_SPAWNING_TEXT, nmf)) {
				AvailableNarrativeField af;
				af.strField = NFIELD_EMN_SPAWNING_TEXT;
				listFields.AddTail(af);
			}

			// (a.walling 2010-04-01 10:40) - PLID 38013 - The 'Item Spawning Text' field
			{
				AvailableNarrativeField af;
				af.strField = NFIELD_ITEM_SPAWNING_TEXT;
				listFields.AddTail(af);
			}

			// (a.walling 2010-04-01 10:40) - PLID 38013 - The 'Item Spawning Text (Furthest)' field
			{
				AvailableNarrativeField af;
				af.strField = NFIELD_ITEM_SPAWNING_TEXT_FURTHEST;
				listFields.AddTail(af);
			}
		}
	}

	// letterwriting
	if (bIncludeLetterWriting) {
		// (a.walling 2010-04-01 10:04) - PLID 37923 - Just for completeness, add a separator here
		if (bIncludeEMR) {
			AvailableNarrativeField afSeparator;
			listFields.AddTail(afSeparator);
		}

		CNarrativeMergeFieldMap& mapLWFields(m_pParentTopic->GetParentEMN()->m_mapLWMergeFields);
		pos = mapLWFields.GetStartPosition();
		while (pos) {
			AvailableNarrativeField af;
			NarrativeMergeField nmf;
			mapLWFields.GetNextAssoc(pos, af.strField, nmf);

			if (!nmf.bIsLWMergeField) {
				continue;
			}

			af.strFlags += "w";
		
			POSITION posInsert = listFields.GetTailPosition();
			while (posInsert) {
				// (a.walling 2010-01-08 11:50) - PLID 36819 - Now compares by case if case insensitive matches. Also the POSITION was incorrect leading to bad ordering.
				POSITION posActualInsert = posInsert;
				AvailableNarrativeField& afExisting(listFields.GetPrev(posInsert));

				int nCompare = af.strField.CompareNoCase(afExisting.strField);
				if (nCompare == 0) {
					nCompare = af.strField.Compare(afExisting.strField);
				}
				if (nCompare > 0) {
					posInsert = posActualInsert;
					break;
				}
			}
			if (posInsert) {
				listFields.InsertAfter(posInsert, af);
			} else {
				listFields.AddHead(af);
			}
		}
	}

	// if we added something, put in a separator
	if (listFields.GetCount() > 0) {
		AvailableNarrativeField afSeparator;
		listFields.AddTail(afSeparator);
	}

	// add all the EMN Detail fields
	if (bIncludeEMR) {
		CArray<CEMNDetail*,CEMNDetail*> arAllDetails;
		GetAllEMNDetails(arAllDetails, false); // don't bother prioritizing
		boost::sort(arAllDetails, CompareDetailFieldNames()); // sort

		// (a.walling 2010-01-06 15:04) - PLID 36777
		long nMySpawnedGroupID = GetSpawnedGroupID();

		// (a.walling 2010-06-09 17:22) - PLID 38966 - Don't duplicate any emr fields
		void* dummy = NULL;
		CMapStringToPtr mapAddedFieldNames;
		mapAddedFieldNames.InitHashTable(79);

		for (int i = 0; i < arAllDetails.GetSize(); i++) {
			CEMNDetail* pDetail = arAllDetails[i];

			// (a.walling 2010-01-06 15:05) - PLID 36777 - Only show items that can be inserted (ie, filtering on spawned group IDs)
			long nCheckSpawnedGroupID = pDetail->GetSpawnedGroupID();
			if ( (nMySpawnedGroupID == -1) || (nCheckSpawnedGroupID == -1) || (nCheckSpawnedGroupID == nMySpawnedGroupID) ) {
				if (pDetail->m_EMRInfoType == eitSingleList || pDetail->m_EMRInfoType == eitMultiList) {
					AvailableNarrativeField af;

					af.strField = pDetail->GetLabelText();
					af.strFlags += "e";
					//TES 7/5/2012 - PLID 50855 - Set the DetailID
					af.nDetailID = pDetail->m_nEMRDetailID;
					
					// (a.walling 2010-06-09 17:22) - PLID 38966 - Don't duplicate any emr fields
					if (!mapAddedFieldNames.Lookup(af.strField, dummy)) {
						listFields.AddTail(af);
						mapAddedFieldNames.SetAt(af.strField, NULL);
					}

					if (bIncludeListItems) {
						for(int j = 0; j < pDetail->GetListElementCount(); j++) {
							ListElement le = pDetail->GetListElement(j);
							AvailableNarrativeField afle;
							afle.strField = pDetail->GetLabelText() + " - " + le.strName;
							afle.strFlags += "el";
							//TES 7/5/2012 - PLID 50855 - Set the DetailID and DataID
							afle.nDetailID = pDetail->m_nEMRDetailID;
							afle.nDataID = le.nID;
							
							// (a.walling 2010-06-09 17:22) - PLID 38966 - Don't duplicate any emr fields
							if (!mapAddedFieldNames.Lookup(afle.strField, dummy)) {
								listFields.AddTail(afle);
								mapAddedFieldNames.SetAt(afle.strField, NULL);
							}
						}
					}
				}
				//TES 2/25/2010 - PLID 37535 - Allow tables that use the Smart Stamp format to be in narratives
				// (z.manning 2010-07-26 14:58) - PLID 39848 - Allow all tables to be inserted in narratives
				else if (pDetail->m_EMRInfoType == eitText || pDetail->m_EMRInfoType == eitSlider ||
					(pDetail->m_EMRInfoType == eitTable))
				{
					AvailableNarrativeField af;

					af.strField = pDetail->GetLabelText();
					af.strFlags += "e";
					// (c.haag 2012-12-04) - PLID 54022 - We also need to assign the detail ID. If we don't
					// do this, then the connection between this field and its detail will be non-existent.
					af.nDetailID = pDetail->m_nEMRDetailID;
					
					// (a.walling 2010-06-09 17:22) - PLID 38966 - Don't duplicate any emr fields
					if (!mapAddedFieldNames.Lookup(af.strField, dummy)) {
						listFields.AddTail(af);
						mapAddedFieldNames.SetAt(af.strField, NULL);
					}
				}
			}
		}
	}

	/////

	// (z.manning 2011-11-10 10:46) - PLID 46382 - There are now 3 fields here
	DWORD dwSize = listFields.GetSize();
	//TES 7/5/2012 - PLID 50855 - There are 5 fields now
	DWORD dwDims[] = { dwSize, 5 };
	saFields.Create(VT_BSTR, 2, dwDims);

	// finally, populate the array
	BSTR* pSafeArrayData = NULL;
	saFields.AccessData((LPVOID *)&pSafeArrayData);

	int nFieldIndex = 0;
	pos = listFields.GetHeadPosition();
	while (pos)
	{
		AvailableNarrativeField& af(listFields.GetNext(pos));
		
		if (af.strField.IsEmpty() && af.strFlags.IsEmpty())
		{
			// separator
			// field 1
			pSafeArrayData[(nFieldIndex * 5) + 0] = NULL;

			// field 2
			pSafeArrayData[(nFieldIndex * 5) + 1] = NULL;

			// (z.manning 2011-11-10 10:47) - PLID 46382 - Added a 3rd field
			pSafeArrayData[(nFieldIndex * 5) + 2] = NULL;

			//TES 7/5/2012 - PLID 50855 - Added a 4th and 5th field
			pSafeArrayData[(nFieldIndex * 5) + 3] = NULL;
			pSafeArrayData[(nFieldIndex * 5) + 4] = NULL;
		}
		else
		{
			// field 1
			pSafeArrayData[(nFieldIndex * 5) + 0] = _bstr_t(af.strField).Detach();

			// field 2
			pSafeArrayData[(nFieldIndex * 5) + 1] = _bstr_t(af.strFlags).Detach();

			// (z.manning 2011-11-10 10:47) - PLID 46382 - Added a 3rd field
			pSafeArrayData[(nFieldIndex * 5) + 2] = _bstr_t(af.strActualField).Detach();

			//TES 7/5/2012 - PLID 50855 - Added DetailID and DataID
			CString strDetailID;
			if(af.nDetailID != -1) {
				strDetailID.Format("%li", af.nDetailID);
			}
			pSafeArrayData[(nFieldIndex * 5) + 3] = _bstr_t(strDetailID).Detach();

			CString strDataID;
			if(af.nDataID != -1) {
				strDataID.Format("%li", af.nDataID);
			}
			pSafeArrayData[(nFieldIndex * 5) + 4] = _bstr_t(strDataID).Detach();
		}

		nFieldIndex++;
	}

	listFields.RemoveAll();

	saFields.UnaccessData();
}

// (z.manning 2011-01-20 17:51) - PLID 42338
CEMNDetailArray* CEMNDetail::GetSmartStampImageDetails()
{
	return &m_arySmartStampImageDetails;
}

CEMNDetail* CEMNDetail::GetSmartStampTableDetail()
{
	return m_pSmartStampTableDetail;
}

// (z.manning 2011-01-20 12:15) - PLID 42338 - Support multiple images per smart stamp table
void CEMNDetail::SetSmartStampImageDetails(CEMNDetailArray &aryImages)
{
	CMap<long,long,long,long> mapRemovedDetailStampLoadedFromIDsToDetailStampPointer;
	CMap<long,long,bool,bool> mapRemovedImageInfoMasterIDs;

	// (z.manning 2011-01-20 12:47) - PLID 42338 - First find any current image pointers that will no longer
	// be linked to this table and remove them and release a reference.
	for(int nOldImageIndex = m_arySmartStampImageDetails.GetCount() - 1; nOldImageIndex >= 0; nOldImageIndex--)
	{
		CEMNDetail *pImage = m_arySmartStampImageDetails.GetAt(nOldImageIndex);
		if(!aryImages.Contains(pImage))
		{
			// (z.manning 2011-02-22 17:10) - PLID 42549 - Keep track of all the detail stamps we're removing from
			// any linked images. See the massive comment later in this function for why we do this.
			for(int nDetailStampIndex = 0; nDetailStampIndex < pImage->m_arypImageStamps.GetSize(); nDetailStampIndex++) {
				EmrDetailImageStamp *pDetailStamp = pImage->m_arypImageStamps.GetAt(nDetailStampIndex);
				if(pDetailStamp->nLoadedFromID != -1) {
					mapRemovedDetailStampLoadedFromIDsToDetailStampPointer.SetAt(pDetailStamp->nLoadedFromID, (long)pDetailStamp);
				}
			}

			// (z.manning 2011-02-23 11:16) - PLID 42549 - Keep track of the info master IDs that had been linked to this table.
			mapRemovedImageInfoMasterIDs.SetAt(pImage->m_nEMRInfoMasterID, true);
			pImage->__Release(FormatString("CEMNDetail::SetSmartStampImageDetails switching pointer to a different image (this table = 0x%08x)", (long)this));
			m_arySmartStampImageDetails.RemoveAt(nOldImageIndex);
		}
	}

	// (z.manning 2011-01-20 12:48) - PLID 42338 - Now find any images that should be linked to this table that
	// are not and add them to the array after adding a reference.
	for(int nNewImageIndex = 0; nNewImageIndex < aryImages.GetCount(); nNewImageIndex++)
	{
		CEMNDetail *pImage = aryImages.GetAt(nNewImageIndex);

		// (j.jones 2013-01-14 13:34) - PLID 54170 - Don't link to details that aren't actually
		// images. This would be bad data.
		if(pImage->m_EMRInfoType != eitImage) {
			//We will gracefully skip linking this non-image to the table,
			//but ASSERT so we still know when it happens to make sure there
			//are no new, unhandled causes of this data
			ASSERT(FALSE);
			continue;
		}

		if(!m_arySmartStampImageDetails.Contains(pImage))
		{
			pImage->__AddRef(FormatString("CEMNDetail::SetSmartStampImageDetails linking to image (this table = 0x%08x)", (long)this));
			m_arySmartStampImageDetails.Add(pImage);

			// (z.manning 2011-02-23 11:17) - PLID 42549 - Check and see we are adding a link to simply a differnt detail
			// pointer for the same item.
			bool bDummy;
			if(!mapRemovedImageInfoMasterIDs.Lookup(pImage->m_nEMRInfoMasterID, bDummy))
			{
				// (z.manning 2011-02-23 11:18) - PLID 42549 - This is the first time we have linked this table to this image.
				// Check and see if we have already loaded the table's content and if so then we need to refresh the table
				// since it's content is dependent on the image state (and the image may have remembered data).
				if(!m_bNeedToLoadContent && pImage->IsStateSet()) {
					pImage->RefreshSmartStampImagePostAdd();
				}
			}

			// (z.manning 2011-02-22 17:10) - PLID 42549 - Due to the convalution of EMR loading code, it is easily
			// possible that the same detail may get loaded more than once (e.g. it may get loaded "normally" and may
			// be loaded again by the EMNLoader). Let's say a smart stamp image was loaded twice. The real one was loaded
			// normally and then it was loaded again in the EMNLoader when it was loading all details for a bunch of topics
			// being spawned. The smart stamp table for that image may also be loaded in the EMNLoader and thus, at first
			// at least, the table will be linked to the image in the loader rather than the one that was already fully loaded
			// (the loader has no knowledge of this). The table will eventually link to the correct image once the loading
			// process is complete, but the smart stamp table's content may have been loaded when it was linked to the loader
			// version of the image which was never actually used on the EMN. Part of this issue is that there isn't much rhyme
			// or reason to when LoadContent is called on a detail-- it will be done the first time something is done to
			// that detail that requires content being loaded. In fact, there is one place in the EMNLoader itself that can
			// call LoadContent (whether or not it should be doing that is debatable, IMO). Anyway, to handle the issue of a table
			// possibly having had its content loaded using smart stamp pointers that never actually became part of the EMN
			// I added code to this function (which is the only place where smart stamp images can be linked to a table) to 
			// check and see if we replaced a smart stamp pointer with an equivalent pointer and if so to update all references
			// to that pointer. Note: We check for equivalent smart stamps based on the loaded from ID as opposed to just the 
			// ID itself because this can also happen with remembered smart stamps which may note yet be saved.
			for(int nDetailStampIndex = 0; nDetailStampIndex < pImage->m_arypImageStamps.GetSize(); nDetailStampIndex++)
			{
				EmrDetailImageStamp *pNewDetailStamp = pImage->m_arypImageStamps.GetAt(nDetailStampIndex);
				if(pNewDetailStamp->nLoadedFromID != -1) {
					long nDetailStampPointer = 0;
					if(mapRemovedDetailStampLoadedFromIDsToDetailStampPointer.Lookup(pNewDetailStamp->nLoadedFromID, nDetailStampPointer)) {
						EmrDetailImageStamp *pOldDetailStamp = (EmrDetailImageStamp*)nDetailStampPointer;
						if(pNewDetailStamp != pOldDetailStamp) {
							UpdateDetailStampPointer(pOldDetailStamp, pNewDetailStamp);
						}
					}
				}
			}
		}
	}
}

// (z.manning 2011-01-20 16:34) - PLID 42338 - Renamed this as it behaves differently now
void CEMNDetail::EnsureSmartStampImageDetail(CEMNDetail *pImage)
{
	// (z.manning 2011-01-20 16:34) - PLID 42338 - Back in the good ol' days it used to be a 1:1 ratio
	// between smart stamp images and tables in which case this function similarly to 
	// CEMNDetail::SetSmartStampTableDetail. However, we can now have multiple images pointing to the
	// same smart stamp table, so this function needs to do the same thing it used to do which is replace
	// the smart stamp image for this table (if there was one) with the given pImage. That is much more
	// complicated now that the table may link to multiple images. The way we're handling this is by
	// checking the EMR info master ID and making sure the same table isn't linked with the same image item
	// (which we are intentionally disallowing).

	if(pImage == NULL) {
		ASSERT(FALSE);
		AfxThrowNxException("CEMNDetail::EnsureSmartStampImageDetail - pImage is NULL");
	}

	CEMNDetailArray aryNewSmartStampImages;
	for(int nOldImageIndex = 0; nOldImageIndex < m_arySmartStampImageDetails.GetCount(); nOldImageIndex++)
	{
		CEMNDetail *pOldImage = m_arySmartStampImageDetails.GetAt(nOldImageIndex);
		if(pOldImage->m_nEMRInfoMasterID != pImage->m_nEMRInfoMasterID) {
			aryNewSmartStampImages.Add(pOldImage);
		}
	}

	aryNewSmartStampImages.Add(pImage);

	SetSmartStampImageDetails(aryNewSmartStampImages);
}

void CEMNDetail::ClearSmartStampImageDetails()
{
	CEMNDetailArray aryEmpty;
	SetSmartStampImageDetails(aryEmpty);
}

void CEMNDetail::ClearSmartStampImageDetailByEmrInfoMasterID(const long nEmrInfoMasterID)
{
	CEMNDetailArray aryNewSmartStampImages;
	for(int nOldImageIndex = 0; nOldImageIndex < m_arySmartStampImageDetails.GetCount(); nOldImageIndex++)
	{
		CEMNDetail *pOldImage = m_arySmartStampImageDetails.GetAt(nOldImageIndex);
		if(pOldImage->m_nEMRInfoMasterID != nEmrInfoMasterID) {
			aryNewSmartStampImages.Add(pOldImage);
		}
	}

	SetSmartStampImageDetails(aryNewSmartStampImages);
}

void CEMNDetail::SetSmartStampTableDetail(CEMNDetail *pTable)
{
	// (j.jones 2013-01-14 13:29) - PLID 54170 - If this is not an image, do not link it to a table.
	//This would be bad data if a non-image is somehow linked to a table.
	if(m_EMRInfoType != eitImage) {
		//assert, because while we are gracefully handling this situation,
		//we should still know when it happens to make sure there are no
		//new, unhandled causes of this data
		ASSERT(FALSE);

		//clear the smart stamp data so we don't try to keep linking
		//this detail for the remainder of this EMN session
		m_bSmartStampsEnabled = FALSE;
		m_nChildEMRDetailID = -1;
		m_nChildEMRInfoMasterID = -1;
		m_nChildEMRTemplateDetailID = -1;
		m_pSmartStampTableDetail = NULL;
		return;
	}

	// (j.jones 2010-03-02 11:33) - PLID 37594 - handle reference counts
	if(m_pSmartStampTableDetail != pTable) {
		//the table link changed
		if(m_pSmartStampTableDetail) {
			//we already had a table, now we're pointing to another one
			//(this should not be common outside of the loader)
			m_pSmartStampTableDetail->__Release(FormatString("CEMNDetail::SetSmartStampTableDetail switching pointer to a different table (this image = 0x%08x)", (long)this));
		}
		if(pTable) {
			//we're linking to a new table
			pTable->__AddRef(FormatString("CEMNDetail::SetSmartStampTableDetail linking to table (this image = 0x%08x)", (long)this));
		}
	}

	m_pSmartStampTableDetail = pTable;

	// (j.jones 2010-02-18 12:14) - PLID 37423 - the InkPicture needs to know if it is a SmartStamp Image	
	if(m_pEmrItemAdvDlg && IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
		//if we have an AdvImageDlg already, tell the InkPicture that the
		//SmartStamp Image property has changed
		((CEmrItemAdvImageDlg*)m_pEmrItemAdvDlg)->SetIsSmartStampImage(m_pSmartStampTableDetail ? TRUE : FALSE);
	}
}

// (j.jones 2010-02-17 15:32) - PLID 37318 - called when reloading an existing SmartStamp Image that
// is linked to a table, will potentially disconnect the image and the table if the link is now invalid
void CEMNDetail::EnsureLinkedSmartStampTableValid()
{
	try {

		if(m_pSmartStampTableDetail == NULL) {
			//there is no linked table, should there be?
			if(m_EMRInfoType == eitImage && m_bSmartStampsEnabled && m_nChildEMRInfoMasterID != -1
				&& m_pParentTopic != NULL && m_pParentTopic->GetParentEMN() != NULL
				&& !GetIsOnLockedAndSavedEMN()) {

				m_pParentTopic->GetParentEMN()->EnsureSmartStampLinks(this);
			}
			return;
		}

		//if we're here, we have a linked table, so confirm its validity

		if(m_EMRInfoType != eitImage || !m_bSmartStampsEnabled || m_nChildEMRInfoMasterID != m_pSmartStampTableDetail->m_nEMRInfoMasterID) {
			//this detail is either no longer an image, is no longer a SmartStamp image, or is now linked to a different table detail,
			//so our currently linked table detail is invalid!

			if(m_pParentTopic != NULL && m_pParentTopic->GetParentEMN() != NULL
				&& !m_bIsTemplateDetail && m_nEMRDetailID != -1
				//&& GetIsOnLockedAndSavedEMN()
				) {
				//if the EMN is locked, we can't change anything, period
				// (j.jones 2013-01-14 12:35) - PLID 54170 - Changed this to simply never
				// clear this data if it is on any saved EMN. It should be impossible
				// for a link to be broken on saved data unless it was bad data to begin
				// with (it may be possible if they brought an item up to date).
				// Regardless, we should never be auto-deleting saved patient data, period.
				return;
			}

			//clear the pointer to the table, also clear its reference to us
			// (a.walling 2010-05-17 15:40) - PLID 38582 - In this specific circumstance, we know we once had a link but are disabling it.
			// Due to reference counting insanity, we have to ensure that we maintain another reference here before clearing the link; otherwise
			// we will end up being deleted ourselves.
			__AddRef("CEMNDetail::ClearSmartStampImageDetailByEmrInfoMasterID Pre SetSmartStampImageDetail(NULL)");
			// (z.manning 2011-01-20 17:11) - PLID 42338 - Only clear this detail
			m_pSmartStampTableDetail->ClearSmartStampImageDetailByEmrInfoMasterID(m_nEMRInfoMasterID);
			__Release("CEMNDetail::ClearSmartStampImageDetailByEmrInfoMasterID Post SetSmartStampImageDetail(NULL)");
			if(m_pSmartStampTableDetail) {

				//the table has to be removed, as its rows reference this image
				CEMRTopic *pTopic = m_pSmartStampTableDetail->m_pParentTopic;
				if(pTopic) {
					pTopic->RemoveDetail(m_pSmartStampTableDetail);
					pTopic->GetParentEMN()->RemoveDetailFromPopup(m_pSmartStampTableDetail);

					CWnd *pWnd = pTopic->GetParentEMN()->GetInterface();
					if(pWnd && ::IsWindow(pWnd->GetSafeHwnd())) {
						pWnd->PostMessage(NXM_TOPIC_MODIFIED_CHANGED, (WPARAM)pTopic);
					}
				}

				// (j.jones 2010-03-02 11:33) - PLID 37594 - handle reference counts
				m_pSmartStampTableDetail->__Release("CEMNDetail::EnsureLinkedSmartStampTableValid clearing table pointer");
				m_pSmartStampTableDetail = NULL;
			}

			//if we call this on a saved item, reset the stored detail IDs
			//(reset both IDs, neither are valid anymore)
			m_nChildEMRTemplateDetailID = -1;
			m_nChildEMRDetailID = -1;

			//flag this item as being unsaved, as we have made changes
			SetUnsaved();

			//lastly, if this happened, try to add the new table
			if(m_pParentTopic != NULL && m_pParentTopic->GetParentEMN() != NULL) {
				m_pParentTopic->GetParentEMN()->EnsureSmartStampLinks(this);
			}
		}

	}NxCatchAllThread(__FUNCTION__);
}

// (z.manning 2010-02-17 17:00) - PLID 37412
// (z.manning 2011-09-08 15:27) - PLID 45335 - Added 3D fields
EmrDetailImageStamp* CEMNDetail::AddNewDetailImageStamp(const long nDetailImageStampID, TextString *pts, const long nOrderIndex, const BYTE nSmartStampRule)
{
	EmrDetailImageStamp stamp;
	stamp.nID = nDetailImageStampID;
	// (j.jones 2010-03-03 12:36) - PLID 37231 - nLoadedFromID is always a copy of nID
	// because we may be loading from a remembered value, and SetNew() will reset our nID
	// to -1, but the table will still need to find this stamp by the original ID
	stamp.nLoadedFromID = nDetailImageStampID;
	stamp.nStampID = pts->nStampID;
	stamp.nOrderIndex = nOrderIndex;
	stamp.eRule = (EMRSmartStampTableSpawnRule)nSmartStampRule;
	stamp.x = pts->x;
	stamp.y = pts->y;
	stamp.var3DPosX = pts->var3DPosX;
	stamp.var3DPosY = pts->var3DPosY;
	stamp.var3DPosZ = pts->var3DPosZ;
	stamp.varNormalX = pts->varNormalX;
	stamp.varNormalY = pts->varNormalY;
	stamp.varNormalZ = pts->varNormalZ;
	stamp.n3DHotSpotID = pts->n3DHotSpotID;
	return AddNewDetailImageStamp(stamp);
}

// (z.manning 2010-02-17 17:00) - PLID 37412
EmrDetailImageStamp* CEMNDetail::AddNewDetailImageStamp(const EmrDetailImageStamp source)
{
	EmrDetailImageStamp *pNewStamp = new EmrDetailImageStamp;
	*pNewStamp = source;
	m_arypImageStamps.Add(pNewStamp);
	return pNewStamp;
}

// (z.manning 2010-02-17 17:00) - PLID 37412
int CEMNDetail::GetImageStampCount() const
{
	return m_arypImageStamps.GetCount();
}

// (z.manning 2010-02-17 17:00) - PLID 37412
EmrDetailImageStamp* CEMNDetail::GetImageStampByIndex(const int nIndex) const
{
	if(nIndex < 0 || nIndex >= m_arypImageStamps.GetSize()) {
		AfxThrowNxException("CEMNDetail::GetImageStampByIndex - invalid index: %li (count = %li)", nIndex, m_arypImageStamps.GetSize());
	}
	return m_arypImageStamps.GetAt(nIndex);
}

// (z.manning 2010-03-17 14:15) - PLID 37439
EmrDetailImageStamp* CEMNDetail::GetDetailImageStampByID(const long nDetailStampID)
{
	CEMNDetail *pImage = NULL;
	if(IsSmartStampTable()) {
		// (z.manning 2011-01-21 10:04) - PLID 42338 - Support multiple images per smart stamp table
		pImage = m_arySmartStampImageDetails.GetDetailFromDetailStampID(nDetailStampID);
	}
	else if(m_EMRInfoType == eitImage) {
		pImage = this;
	}

	if(pImage == NULL) {
		return NULL;
	}

	for(int nDetailStampIndex = 0; nDetailStampIndex < pImage->m_arypImageStamps.GetSize(); nDetailStampIndex++)
	{
		EmrDetailImageStamp *pDetailStamp = pImage->m_arypImageStamps.GetAt(nDetailStampIndex);
		if(pDetailStamp->nID == nDetailStampID) {
			return pDetailStamp;
		}
	}
	return NULL;
}

// (z.manning 2011-01-27 15:52) - PLID 42335
CEmrDetailImageStampArray* CEMNDetail::GetDetailImageStamps()
{
	return &m_arypImageStamps;
}

// (z.manning 2010-02-18 15:41) - PLID 37412 - Returns true if the table is linked to a smart stamp image
BOOL CEMNDetail::IsSmartStampTable()
{
	// (z.manning 2011-01-20 15:23) - PLID 42338 - Handle multiple images per smart stamp table
	if(m_arySmartStampImageDetails.GetCount() > 0 && m_arySmartStampImageDetails.AreSmartStampsEnabled()) {
		return TRUE;
	}
	return FALSE;
}

// (a.walling 2010-08-23 16:10) - PLID 37923 - Returns true if the image is linked to a smart stamp table
BOOL CEMNDetail::IsSmartStampImage()
{
	if(m_pSmartStampTableDetail != NULL && m_bSmartStampsEnabled) {
		return TRUE;
	}
	return FALSE;
}

// (z.manning 2012-07-03 17:59) - PLID 51025 - True if this is an existing smart stamp image detail that is somehow not
// linked with a smart stamp table.
BOOL CEMNDetail::IsOrphanedSmartStampImage()
{
	if(m_nEMRDetailID != -1 && m_bSmartStampsEnabled && m_nChildEMRInfoMasterID != -1 && m_nChildEMRDetailID == -1) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (z.manning 2011-01-20 12:05) - PLID 42338
BOOL CEMNDetail::HasDetailStamp(EmrDetailImageStamp *pDetailStamp)
{
	return m_arypImageStamps.Contains(pDetailStamp);
}

// (z.manning 2010-02-23 12:29) - PLID 37412
_variant_t CEMNDetail::GetDetailImageStampAsVariantArray()
{
	return m_arypImageStamps.GetAsVariant();
}

// (j.jones 2010-02-25 14:35) - PLID 37318 - ReconfirmSmartStampLinks will simply ensure that
// if this detail is pointing to an image or table, that image/table is also pointing to us.
void CEMNDetail::ReconfirmSmartStampLinks()
{
	if(m_arySmartStampImageDetails.GetCount() > 0) {
		m_arySmartStampImageDetails.SetSmartStampTableDetail(this);
	}
	if(m_pSmartStampTableDetail) {
		m_pSmartStampTableDetail->EnsureSmartStampImageDetail(this);
	}
}

// (z.manning 2010-02-26 08:29) - PLID 37412
void CEMNDetail::UpdateTableDataOutput()
{
	// (z.manning 2010-02-26 08:31) - PLID 37412 - This code existed in a few different places already and
	// I needed it as well, so even though this goes against our principles of EMR coding, I moved the
	// code to its own function.
	CString strHTML, strNonHTML;
	EmrMultiSelectFormat emsf = emsfText;
	CString strSeparator = ", ";
	CString strSeparatorFinal = " and ";
	
	// (z.manning 2010-02-26 08:56) - PLID 37551 - Loading this from data every time seems like
	// overkill to me so I entered 37551 to look into optimizing this.
	_RecordsetPtr rs = CreateParamRecordset(
		"SELECT DataFormat, DataSeparator, DataSeparatorFinal FROM EMRInfoT WHERE ID = {INT} \r\n"
		, m_nEMRInfoID);
	if(!rs->eof) {
		emsf = (EmrMultiSelectFormat)AdoFldLong(rs, "DataFormat", emsfText);
		strSeparator = AdoFldString(rs, "DataSeparator", ", ");
		strSeparatorFinal = AdoFldString(rs, "DataSeparatorFinal", strSeparator == ", " ? " and " : strSeparator);
	}
	rs->Close();

	m_pParentTopic->GetParentEMN()->GetTableDataOutput(this,emsf,strSeparator,strSeparatorFinal,strHTML,strNonHTML,true);
	m_strSentenceHTML = strHTML;
	m_strSentenceNonHTML = strNonHTML;
	m_emsf = emsf;
	m_strSeparator = strSeparator;
	m_strSeparatorFinal = strSeparatorFinal;
}

// (z.manning 2010-03-02 12:59) - PLID 37571 - Updates all rows looking for a detail stamp pointer with
// the given detail stamp ID
void CEMNDetail::UpdateDetailStampIDByPointer(const long nDetailStampPointer, const long nDetailStampID)
{
	if(m_pSmartStampTableDetail != NULL)
	{
		for(int nRowIndex = 0; nRowIndex < m_pSmartStampTableDetail->m_arTableRows.GetSize(); nRowIndex++) {
			TableRow *ptr = m_pSmartStampTableDetail->m_arTableRows.GetAt(nRowIndex);
			// (c.haag 2012-10-26) - PLID 53440 - Use the new getter and setter functions
			if(ptr->m_ID.GetDetailImageStampObject() != NULL && (long)ptr->m_ID.GetDetailImageStampObject() == nDetailStampPointer) {
				ptr->m_ID.SetDetailImageStampID(nDetailStampID);
			}
		}
	}
}

// (z.manning 2010-02-26 09:40) - PLID 37412 - Updates all references of of detail stamp pointers
// in the table row array.
void CEMNDetail::UpdateDetailStampPointer(EmrDetailImageStamp *pExisting, EmrDetailImageStamp *pNew)
{
	CEMNDetail *pTable = NULL;
	if(m_pSmartStampTableDetail != NULL) {
		pTable = m_pSmartStampTableDetail;
	}
	else if(IsSmartStampTable()) {
		pTable = this;
	}

	if(pTable != NULL)
	{
		for(int nRowIndex = 0; nRowIndex < pTable->m_arTableRows.GetSize(); nRowIndex++) {
			TableRow *ptr = pTable->m_arTableRows.GetAt(nRowIndex);
			// (c.haag 2012-10-26) - PLID 53440 - Use the new getter and setter functions
			if(ptr->m_ID.GetDetailImageStampObject() != NULL && ptr->m_ID.GetDetailImageStampObject() == pExisting) {
				ptr->m_ID.SetDetailImageStamp(pNew);
			}
		}

		// (z.manning 2010-03-15 16:05) - Make sure we also update any references to detail stamp pointers in the
		// table state.
		_variant_t varTableState = pTable->GetState();
		CString strTableState;
		if(varTableState.vt == VT_BSTR) {
			strTableState = VarString(varTableState);
		}
		CEmrTableStateIterator etsi(strTableState);
		long nRow, nCol, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID;
		CString strData, strNewState;
		while(etsi.ReadNextElement(nRow, nCol, strData, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID))
		{
			if(pExisting != NULL && pNew != NULL && 
				(nEmrDetailImageStampPointer == (long)pExisting ||
				(pExisting->nID != -1 && pExisting->nID == nEmrDetailImageStampID)
				))
			{
				nEmrDetailImageStampPointer = (long)pNew;
				nEmrDetailImageStampID = pNew->nID;
			}
			::AppendTableStateWithUnformattedElement(strNewState, nRow, nCol, strData, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID);
		}
		pTable->SetState(_bstr_t(strNewState));
	}

	// (z.manning 2010-03-11 15:30) - PLID 37571 - We also need to update any source detail stamp pointers
	// on possible spawned objects.
	if(GetParentEMR() != NULL) {
		// (a.walling 2010-03-31 14:51) - PLID 38009 - Pass this to the EMR, along with our source EMN.
		GetParentEMR()->UpdateSourceDetailStampPointers(pExisting, pNew, m_pParentTopic->GetParentEMN());
		// (z.manning 2011-03-04 18:08) - PLID 42682 - We also need to update any pending actions.
		GetParentEMR()->UpdatePendingActionSourceDetailPointers(pExisting, pNew, GetParentEMN());
	}
}

// (z.manning 2010-03-11 11:18) - PLID 37412
// (a.walling 2012-10-31 17:17) - PLID 53552 - LastSavedDetail - LastSavedStamps no longer necessary
void CEMNDetail::CreateNewCopiesOfDetailStampPointers()
{
	// (z.manning 2010-03-11 11:21) - PLID 37412 - The point of this function to to create new copies
	// of all the detail image stamps normally stored in this detail. Theoretically, this should be done
	// in the EMNDetail's operator=, however, the detail stamp pointers are needed for spawning from smart
	// stamps and recreating the detail stamp pointers in operator= was causing other issues. So in the
	// case where we do need to have new copies of the pointers (such as when copying an EMN) then this 
	// function must be called manually.
	for(int nDetailStampIndex = 0; nDetailStampIndex < m_arypImageStamps.GetSize(); nDetailStampIndex++)
	{
		EmrDetailImageStamp *pDetailStampOld = m_arypImageStamps.GetAt(nDetailStampIndex);
		EmrDetailImageStamp *pDetailStampNew = new EmrDetailImageStamp(pDetailStampOld);
		m_arypImageStamps.SetAt(nDetailStampIndex, pDetailStampNew);
		UpdateDetailStampPointer(pDetailStampOld, pDetailStampNew);
		pDetailStampOld->Release();
	}
}

// (z.manning 2010-03-11 14:56) - PLID 37571 - Will reassasign the source detail stamp pointer to the given
// new pointer for any object in this detail that could have potentially been spawned.
void CEMNDetail::UpdateSourceDetailStampPointers(EmrDetailImageStamp *pDetailStampOld, EmrDetailImageStamp *pDetailStampNew)
{
	//update this detail
	m_sai.UpdateDetailStampPointerIfMatch(pDetailStampOld, pDetailStampNew);
}

BOOL CEMNDetail::GetReadOnly()
{
	if(m_bReadOnly) {
		return TRUE;
	}
	//TES 3/15/2010 - PLID 37757 - Smart Stamp tables on templates are always read-only
	else if(IsSmartStampTable() && m_bIsTemplateDetail) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

//TES 3/17/2010 - PLID 37530 - Function that returns the index of the given stamp within this detail, based on the stamp type
// (i.e., is this the first or second AK).
long CEMNDetail::GetStampIndexInDetailByType(IN EmrDetailImageStamp* pDetailStamp)
{
	// (z.manning 2011-02-18 12:05) - PLID 42338 - I moved the brunt of this code to CEMNDetailArray::GetStampIndexInDetailByType

	if(IsSmartStampImage()) {
		// (z.manning 2011-02-18 12:08) - PLID 42338 - This is a smart stamp image so we have to get the index
		// across all images that may be linked to the same smart stamp table.
		return m_pSmartStampTableDetail->GetSmartStampImageDetails()->GetStampIndexInDetailByType(pDetailStamp);
	}
	else {
		CEMNDetailArray aryDetails;
		aryDetails.Add(this);
		return aryDetails.GetStampIndexInDetailByType(pDetailStamp);
	}
}

// (j.jones 2010-03-19 12:22) - PLID 37318 - returns true if has m_arypImageStamps has content
BOOL CEMNDetail::HasImageStamps()
{
	return m_arypImageStamps.GetSize() > 0;
}

// (j.jones 2010-08-12 11:22) - PLID 39486 - calculates the SmartStamp Table's AutoNumber index for a given cell
long CEMNDetail::ParseAutoNumberIndexFromCell(CEMNDetail *pTableDetail, TableRowID *pRowID, long nAutoNumberColumnID)
{
	if(pTableDetail == NULL) {
		ThrowNxException("CEMNDetail::ParseAutoNumberIndexFromCell called with a NULL pTableDetail!");
	}

	if(pRowID == NULL) {
		ThrowNxException("CEMNDetail::ParseAutoNumberIndexFromCell called with a NULL pRowID!");
	}

	TableElement teAutoNumber;
	pTableDetail->GetTableElement(pRowID, nAutoNumberColumnID, teAutoNumber);
	CString strValue = teAutoNumber.GetValueAsString();

	//get the number
	long nAutoNumber = 0;
	int nSpace = strValue.ReverseFind(' ');
	if(nSpace != -1) {
		nAutoNumber = atoi(strValue.Right(strValue.GetLength() - nSpace - 1));
	}
	else {
		nAutoNumber = atoi(strValue);
	}

	return nAutoNumber;
}

// (j.jones 2010-08-12 12:09) - PLID 39486 - calculates a new AutoNumber index for a given SmartStamp Table cell,
// and returns it with the proper prefix
CString CEMNDetail::GenerateAutoNumberContentForCell(TableRow *ptr, TableColumn *ptcAutoNumber)
{
	CEMNDetail *pSmartStampTable = m_pSmartStampTableDetail;
	if(pSmartStampTable == NULL && IsSmartStampTable()) {
		pSmartStampTable = this;
	}

	if(pSmartStampTable == NULL) {
		ThrowNxException("CEMNDetail::GenerateAutoNumberContentForCell called with a NULL pSmartStampTable!");
	}

	if(ptcAutoNumber == NULL) {
		ThrowNxException("CEMNDetail::GenerateAutoNumberContentForCell called with a NULL ptcAutoNumber!");
	}

	CString strAutoNumberCellValue, strAutoNumberName;
	long nAutoNumber = -1;
		
	//we calculate per stamp - does this stamp already exist?
	// (c.haag 2012-10-26) - PLID 53440 - Use the new getter function
	long nStampID = ptr->m_ID.GetImageStampID();
			
	//see if any other row has this StampID
	long nHighestNumber = 0;
	for(int iRowIndex = 0; iRowIndex < pSmartStampTable->GetRowCount() && nAutoNumber == -1; iRowIndex++)
	{
		TableRow *ptrCur = pSmartStampTable->GetRowPtr(iRowIndex);
		if(ptrCur != NULL && ptrCur->IsSmartStampRow())
		{
			//first grab the number
			long nAutoNumberCur = ParseAutoNumberIndexFromCell(pSmartStampTable, &ptrCur->m_ID, ptcAutoNumber->nID);
			
			//track if it is the highest index
			if(nAutoNumberCur > nHighestNumber) {
				nHighestNumber = nAutoNumberCur;
			}

			//how are the numbers calculated?
			if(ptcAutoNumber->m_etantAutoNumberType == etantPerStamp)
			{
				//is it the same stamp?
				// (c.haag 2012-10-26) - PLID 53440 - Use the new getter function
				if(ptrCur->m_ID.GetImageStampID() == nStampID && nAutoNumberCur > 0) {
					//use this number
					nAutoNumber = nAutoNumberCur;
				}
			}
		}
	}

	if(ptcAutoNumber->m_etantAutoNumberType == etantPerStamp) {
		if(nAutoNumber == -1) {
			//if we didn't find the same stamp in another row,
			//use the highest number + 1
			nAutoNumber = nHighestNumber + 1;
		}
	}
	else if(ptcAutoNumber->m_etantAutoNumberType == etantPerRow) {
		nAutoNumber = nHighestNumber + 1;
	}

	//should be impossible
	ASSERT(nAutoNumber != -1);

	//apply the prefix
	strAutoNumberCellValue.Format("%s %li", ptcAutoNumber->m_strAutoNumberPrefix, nAutoNumber);
	//the prefix could have been blank, trim spaces if they exist
	strAutoNumberCellValue.TrimLeft();

	return strAutoNumberCellValue;
}

// (z.manning 2010-09-07 16:04) - PLID 40434 - Use these now for public access to m_bSaveTableColumnWidths
BOOL CEMNDetail::GetSaveTableColumnWidths()
{
	// (z.manning 2010-09-07 16:42) - PLID 40434 - We currently do not have the ability to save column widths
	// on flipped tables so no where in code should ever assume that they are or will be saved.
	if(m_bTableRowsAsFields && IsSmartStampTable()) {
		return FALSE;
	}

	return m_bSaveTableColumnWidths;
}
void CEMNDetail::SetSaveTableColumnWidths(const BOOL bSaveTableColumnWidths)
{
	m_bSaveTableColumnWidths = bSaveTableColumnWidths;
}

// (z.manning 2011-01-28 15:05) - PLID 42335 - This function
void CEMNDetail::RefreshSmartStampImagePostAdd()
{
	if(!IsSmartStampImage()) {
		// (z.manning 2011-01-28 15:08) - PLID 42335 - Shoudn't have been called then.
		ASSERT(FALSE);
		return;
	}

	if(m_bIsTemplateDetail || m_nEMRDetailID != -1) {
		return;
	}

	if(!HasImageStamps()) {
		// (z.manning 2011-02-23 12:12) - If there are no stamps on the image then at this point this function
		// doesn't do anything.
		return;
	}

	long nPatientID = -1;
	long nEmrGroupID = -1;
	if(GetParentEMR() != NULL) {
		nPatientID = GetParentEMR()->GetPatientID();
		nEmrGroupID = GetParentEMR()->GetID();
	}
	_variant_t varRememberedTableState = ::LoadEMRDetailStateDefault(NULL, m_pSmartStampTableDetail->m_nEMRInfoID, nPatientID, nEmrGroupID, -1, GetRemoteData(), m_nRememberedDetailID);

	// (z.manning 2011-01-28 11:47) - PLID 42335 - The newly added image may have remembered data so we
	// need to update the table appropriately. We'll do this by using the existing logic in RequestStateChange
	// but we do NOT want any spawning to occur because of this as that already would have happened when
	// the image was loaded. Only do this if the table already exists though. Otherwise everyting will be
	// handled during the normal load process.
	CArray<TableRow*,TableRow*> arypNewTableRows;
	HandleImageStateChange(GetState(), LoadEMRDetailStateBlank(eitImage), FALSE, varRememberedTableState, &arypNewTableRows);
}

// (z.manning 2011-02-23 13:40) - PLID 42549
void CEMNDetail::UpdateSmartStampTableAutoNumberValues()
{
	if(!IsSmartStampTable()) {
		return;
	}

	// (j.jones 2010-08-12 12:23) - PLID 39496 - if a new smart stamp image & table, verify the autonumber column
	// is filled in for all cells, it may be blank if we are remembering a state from an old table
	// (z.manning 2011-02-02 12:03) - PLID 42335 - Reworked this code a bit to handle multiple images per smart
	// stamp table (as well as fixing a couple things that were just flat out wrong).
	TableColumn *ptcAutoNumber = GetColumnByListSubType(lstSmartStampAutoNumber);
	if(ptcAutoNumber != NULL)
	{
		// (z.manning 2011-02-02 16:43) - PLID 42335 - First clear out the auto number value for each cell as
		// we want to redo the count based on the current table detail.
		for(int iCheckRowIndex = 0; iCheckRowIndex < m_arTableRows.GetSize(); iCheckRowIndex++)
		{
			TableRow *ptrCheck = m_arTableRows.GetAt(iCheckRowIndex);
			if(ptrCheck->IsSmartStampRow() && ptcAutoNumber->m_etantAutoNumberType == etantPerRow) {
				TableElement teBlank;
				teBlank.m_pRow = ptrCheck;
				teBlank.m_pColumn = ptcAutoNumber;
				teBlank.LoadValueFromString("", GetParentEMN());
				SetTableElement(teBlank, TRUE, FALSE);
			}
		}

		//check each cell
		for(int iCheckRowIndex = 0; iCheckRowIndex < m_arTableRows.GetSize(); iCheckRowIndex++)
		{
			TableRow *ptrCheck = m_arTableRows.GetAt(iCheckRowIndex);
			if(ptrCheck != NULL && ptrCheck->IsSmartStampRow())
			{
				//need to recalculate this value
				TableElement teAutoNumber;
				teAutoNumber.m_pRow = ptrCheck;
				teAutoNumber.m_pColumn = ptcAutoNumber;

				CString strAutoNumberCellValue = GenerateAutoNumberContentForCell(ptrCheck, ptcAutoNumber);

				teAutoNumber.LoadValueFromString(strAutoNumberCellValue, GetParentEMN());
				SetTableElement(teAutoNumber, TRUE, FALSE);
			}
		}
	}
}

// (z.manning 2011-02-02 17:55) - PLID 42335 - Returns true if something changed in the table, false otherwise
BOOL CEMNDetail::UpdateSmartStampTableQuantityValues()
{
	if(!IsSmartStampTable()) {
		return FALSE;
	}

	TableColumn *ptcQuantity = GetColumnByListSubType(lstSmartStampQuantity);
	if(ptcQuantity == NULL) {
		// (z.manning 2011-02-02 17:56) - PLID 42335 - The quantity column may be inactive or something.
		return FALSE;
	}

	BOOL bReturn = FALSE;
	for(int nRowIndex = 0; nRowIndex < m_arTableRows.GetSize(); nRowIndex++)
	{
		TableRow *ptr = m_arTableRows.GetAt(nRowIndex);
		// (c.haag 2012-10-26) - PLID 53440 - Use the new getter functions
		if(ptr->m_ID.GetDetailImageStampObject() != NULL)
		{
			if(ptr->m_ID.GetDetailImageStampObject()->eRule == esstsrIncreaseQuantity)
			{
				long nNewQuantity = m_arySmartStampImageDetails.GetTotalByGlobalStampID(ptr->m_ID.GetImageStampID());
				TableElement teQuantity;
				if(GetTableElement(&ptr->m_ID, ptcQuantity->nID, teQuantity))
				{
					long nOldQuantity = atol(teQuantity.GetValueAsString());
					if(nOldQuantity != nNewQuantity) {
						teQuantity.m_pRow = ptr;
						teQuantity.m_pColumn = ptcQuantity;
						teQuantity.LoadValueFromString(AsString(nNewQuantity), GetParentEMN());
						SetTableElement(teQuantity, TRUE, FALSE);
						bReturn = TRUE;
					}
				}
			}
		}
	}

	return bReturn;
}

// (z.manning 2011-02-25 11:22) - PLID 42335
// (z.manning 2011-05-11 09:19) - PLID 43568 - Added parameter for whether or not to refresh content
void CEMNDetail::RefreshSmartStampTableBuiltInData(BOOL bRefreshContent)
{
	if(IsSmartStampTable()) {
		UpdateSmartStampTableQuantityValues();
		UpdateSmartStampTableAutoNumberValues();
		if(bRefreshContent) {
			ReflectCurrentContent();
		}
		RecreateStateFromContent();
	}
}

// (z.manning 2011-02-03 12:20) - PLID 42336 - Returns true if the smart stamp table already has an image item
// that is a different detail but is the same info item (i.e. has the same EMR info master ID).
BOOL CEMNDetail::SmartStampTableAlreadyHasImageItem(CEMNDetail *pImage)
{
	if(!IsSmartStampTable()) {
		return FALSE;
	}

	CEMNDetail *pExistingImageWithSameInfoMasterID = GetSmartStampImageDetails()->GetDetailFromEmrInfoMasterID(pImage->m_nEMRInfoMasterID);
	if(pExistingImageWithSameInfoMasterID != NULL)
	{
		// (z.manning 2011-02-03 12:32) - PLID 42336 - We found an existing detail with the same info master ID, but
		// it could be a template detail that was loaded in the EMN loader but never added to the EMN in which case
		// we return FALSE as this is considered the same detail.
		if(pImage->m_nEMRTemplateDetailID != -1 && pImage->m_nEMRTemplateDetailID == pExistingImageWithSameInfoMasterID->m_nEMRTemplateDetailID)
		{
			BOOL bDetailIsOnEmn = FALSE;
			if(GetParentEMN() != NULL) {
				CEMNDetailArray arypAllEmnDetails;
				GetParentEMN()->GenerateTotalEMNDetailArray(&arypAllEmnDetails);
				if(arypAllEmnDetails.Contains(pExistingImageWithSameInfoMasterID)) {
					bDetailIsOnEmn = TRUE;
				}
			}

			if(!bDetailIsOnEmn) {
				return FALSE;
			}
		}

		return TRUE;
	}

	return FALSE;
}

//TES 3/18/2011 - PLID 41108 - Call to load all unloaded data for the detail (if you know you're going to be iterating through all of them).
void CEMNDetail::LoadAllSelectedDropdowns()
{
	//TES 3/18/2011 - PLID 41108 - Gather all the selected dropdown IDs for items that haven't already been loaded.
	CArray<long,long> arDropdownIDsToLoad;
	for(int i = 0; i < m_arTableElements.GetSize(); i++) {
		TableElement te = m_arTableElements[i];
		TableElementSelectedDropdownItems *pTesdi = m_arypTableSelectedDropdowns[i];
		//TES 5/25/2011 - PLID 41108 - If you select the "NULL" row it'll actually add a 0 to this array, so filter that out.
		CArray<long,long> anDropdownIDs;
		for(int j = 0; j < te.m_anDropdownIDs.GetSize(); j++) {
			if(te.m_anDropdownIDs[j] != 0) anDropdownIDs.Add(te.m_anDropdownIDs[j]);
		}
		if(anDropdownIDs.GetSize() == 0) {
			//TES 3/18/2011 - PLID 41108 - Nothing's selected, so we're done with this element.
			pTesdi->arGlassesOrderDataIDs.RemoveAll();
			pTesdi->bAssociatedDataLoaded = true;
		}
		else if(!pTesdi->bAssociatedDataLoaded) {
			arDropdownIDsToLoad.Append(anDropdownIDs);
		}
	}
	
	//TES 3/18/2011 - PLID 41108 - OK, have we found anything to load?
	if(arDropdownIDsToLoad.GetSize() > 0) {
		//TES 3/18/2011 - PLID 41108 - Yup, pull it all, and map it to the ID of the dropdown items.
		CMap<long,long,long,long> mapIDToGlassesOrderID;
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID, GlassesOrderDataID FROM EmrTableDropdownInfoT "
			"WHERE ID IN ({INTARRAY})", arDropdownIDsToLoad);
		while(!rs->eof) {
			long nGlassesOrderDataID = AdoFldLong(rs, "GlassesOrderDataID",-1);
			if(nGlassesOrderDataID != -1) {
				mapIDToGlassesOrderID.SetAt(AdoFldLong(rs, "ID"), nGlassesOrderDataID);
			}
			rs->MoveNext();
		}

		//TES 3/18/2011 - PLID 41108 - Now go through all our elements that are still unloaded, and set the GlassesOrderDataID based
		// on the map we just loaded up.
		for(int i = 0; i < m_arTableElements.GetSize(); i++) {
			TableElement te = m_arTableElements[i];
			TableElementSelectedDropdownItems *pTesdi = m_arypTableSelectedDropdowns[i];
			if(te.m_anDropdownIDs.GetSize() > 0 && !pTesdi->bAssociatedDataLoaded) {
				pTesdi->arGlassesOrderDataIDs.RemoveAll();
				for(int nDropdown = 0; nDropdown < te.m_anDropdownIDs.GetSize(); nDropdown++) {
					long nGlassesOrderDataID = -1;
					mapIDToGlassesOrderID.Lookup(te.m_anDropdownIDs[nDropdown], nGlassesOrderDataID);
					//TES 5/25/2011 - PLID 41108 - If you select the "NULL" row it'll actually add a 0 to this array, so filter that out.
					if(nGlassesOrderDataID != 0) {
						pTesdi->arGlassesOrderDataIDs.Add(nGlassesOrderDataID);
					}
				}
				//TES 3/18/2011 - PLID 41108 - This one's loaded now.
				pTesdi->bAssociatedDataLoaded = true;
			}
		}
	}
}

void CEMNDetail::LoadSingleSelectedDropdowns(int nIndex) 
{
	TableElement te = m_arTableElements[nIndex];
	TableElementSelectedDropdownItems *pTesdi = m_arypTableSelectedDropdowns[nIndex];
	//TES 3/18/2011 - PLID 41108 - Do we need to load anything?
	if(te.m_anDropdownIDs.GetSize() && !pTesdi->bAssociatedDataLoaded) {
		//TES 3/18/2011 - PLID 41108 - Yup, pull all the GlassesOrderDataIDs from data.
		pTesdi->arGlassesOrderDataIDs.RemoveAll();
		_RecordsetPtr rs = CreateParamRecordset("SELECT GlassesOrderDataID FROM EmrTableDropdownInfoT "
			"WHERE ID IN ({INTARRAY})", te.m_anDropdownIDs);
		while(!rs->eof) {
			long nGlassesOrderDataID = AdoFldLong(rs, "GlassesOrderDataID",-1);
			if(nGlassesOrderDataID != -1) {
				pTesdi->arGlassesOrderDataIDs.Add(nGlassesOrderDataID);
			}
			rs->MoveNext();
		}
	}
	//TES 3/18/2011 - PLID 41108 - This element has now been loaded.
	pTesdi->bAssociatedDataLoaded = true;
}

void CEMNDetail::GetSelectedGlassesOrderDataIDs(TableRow *pTr, TableColumn *pTc, CArray<long,long> &arSelectedIDs)
{
	//TES 3/18/2011 - PLID 41108 - Find the table element.
	int nElementIndex = -1;
	for(int i = 0; i < m_arTableElements.GetSize() && nElementIndex == -1; i++) {
		TableElement te = m_arTableElements[i];
		if(te.m_pRow == pTr && te.m_pColumn == pTc) {
			nElementIndex = i;
		}
	}

	if(nElementIndex != -1) {
		//TES 3/18/2011 - PLID 41108 - Now get the associated data, load it from the database if necessary, and output.
		TableElementSelectedDropdownItems *pTesdi = m_arypTableSelectedDropdowns[nElementIndex];
		if(!pTesdi->bAssociatedDataLoaded) {
			LoadSingleSelectedDropdowns(nElementIndex);
		}
		arSelectedIDs.Append(pTesdi->arGlassesOrderDataIDs);
	}
}

// (z.manning 2011-03-21 15:06) - PLID 30608 - Function to update any table elements that may need to autofill
// after the given row has been changed.
BOOL CEMNDetail::UpdateAutofillColumns(TableElement *pUpdatedTableElement)
{
	return UpdateAutofillColumnsByRow(pUpdatedTableElement->m_pRow, pUpdatedTableElement->m_pColumn);
}

// (z.manning 2011-03-21 15:06) - PLID 30608 - Function to update any table elements that may need to autofill
// after the given row has been changed.
BOOL CEMNDetail::UpdateAutofillColumnsByRow(TableRow *pUpdatedRow, TableColumn *pUpdatedColumn)
{
	if(m_bIsTemplateDetail) {
		// (z.manning 2011-03-21 16:22) - PLID 30608 - We do not currently allow autofilling on EMN templates.
		return FALSE;
	}
	if(GetParentEMN() != NULL) {
		if(GetParentEMN()->IsTemplate()) {
			// (z.manning 2011-03-21 16:22) - PLID 30608 - We do not currently allow autofilling on EMN templates.
			return FALSE;
		}
	}

	BOOL bChanged = FALSE;
	for(int nColumnIndex = 0; nColumnIndex < m_arTableColumns.GetSize(); nColumnIndex++)
	{
		TableColumn *ptc = m_arTableColumns.GetAt(nColumnIndex);

		BOOL bIsSameColumn = FALSE;
		if(pUpdatedColumn != NULL) {
			if(ptc->nID == pUpdatedColumn->nID) {
				bIsSameColumn = TRUE;
			}
		}

		// (z.manning 2011-03-21 15:59) - PLID 30608 - Do not autofill if the edited cell is also the autofill cell
		if(ptc->m_eAutofillType != etatNone && !bIsSameColumn && ptc->AllowAutofill())
		{
			DateTimeOptions dto = dtoInvalid;
			switch(ptc->m_eAutofillType)
			{
				case etatDateAndTime:
					dto = dtoDateTime;
					break;
				case etatDate:
					dto = dtoDate;
					break;
				case etatTime:
					dto = dtoTime;
					break;
				default:
					// (z.manning 2011-03-21 15:32) - PLID 30608 - Unhandled type
					ASSERT(FALSE);
					break;
			}

			if(dto != dtoInvalid) {
				CString strAutofillValue = FormatDateTimeForInterface(COleDateTime::GetCurrentTime(), DTF_STRIP_SECONDS, dto);
				TableElement te;
				// (a.walling 2011-08-24 12:35) - PLID 45171 - We only need to look for existing table elements
				if(!GetExistingTableElement(&pUpdatedRow->m_ID, ptc->nID, te)) {
					te.m_pRow = pUpdatedRow;
					te.m_pColumn = ptc;
				}

				BOOL bFill = TRUE;
				CString strCurrentCellValue = te.GetValueAsString();
				if(strAutofillValue == strCurrentCellValue) {
					bFill = FALSE;
				}
				else {
					// (z.manning 2011-03-22 15:49) - PLID 42954 - Added a preference to control table autofill behavior
					long nAutofillBehavior = GetRemotePropertyInt("EmrTableAutofillBehavior", 1, 0, GetCurrentUserName());
					if(nAutofillBehavior == 0) {
						// (z.manning 2011-03-22 15:48) - PLID 42954 - Only fill when the cell is blank
						if(strCurrentCellValue.IsEmpty()) {
							bFill = TRUE;
						}
						else {
							bFill = FALSE;
						}
					}
					else {
						// (z.manning 2011-03-22 15:49) - PLID 42954 - Always autofill no matter what.
						bFill = TRUE;
					}
				}

				if(bFill) {
					te.LoadValueFromString(strAutofillValue, GetParentEMN());
					SetTableElement(te, TRUE, FALSE);
					bChanged = TRUE;
				}
			}
		}
	}

	return bChanged;
}

// (j.jones 2011-04-28 14:39) - PLID 43122 - returns the primary provider ID on the EMN
// that has FloatEMRData set to true
long CEMNDetail::GetProviderIDForFloatingData()
{
	long nProviderIDForFloatingData = -1;
	if(m_pParentTopic != NULL && m_pParentTopic->GetParentEMN() != NULL) {
		//if multiple primary providers exist, this will return
		//the first one that has FloatEMRData set to true
		return m_pParentTopic->GetParentEMN()->GetProviderIDForFloatingData();
	}

	//we could not find one
	return -1;
}

// (a.walling 2011-05-31 10:20) - PLID 42448 - Gets the (possibly cached) dropdown data for the given IDs and puts them in mapData
// (j.jones 2013-02-20 15:43) - PLID 55217 - we now pass in the column ID
// (a.walling 2013-07-02 09:02) - PLID 57407 - CMap's ARG_VALUE should be const CString& instead of LPCTSTR so it can use CString reference counting
void CEMNDetail::GetDropdownData(const CArray<long, long>& arIDsToFind, CMap<long, long, CString, const CString&>& mapData)
{
	// (a.walling 2013-07-18 10:14) - PLID 57628 - NxCache::EmrTableDropdownInfo::GetDataFromDropdownID makes this function much, much simpler
	for(int i = 0; i < arIDsToFind.GetSize(); ++i) {
		long id = arIDsToFind[i];
		mapData.SetAt(
			id
			, Nx::Cache::EmrTableDropdownInfo::GetDataFromDropdownID(id)
		);
	}
}

// (z.manning 2011-05-27 09:36) - PLID 42131
BOOL CEMNDetail::HasTransformFormulas()
{
	for(int nColIndex = 0; nColIndex < GetColumnCount(); nColIndex++)
	{
		TableColumn *ptc = GetColumnPtr(nColIndex);
		if(ptc->HasTransformFormula()) {
			return TRUE;
		}
	}

	return FALSE;
}

// (z.manning 2012-03-27 17:49) - PLID 33710
BOOL CEMNDetail::HasCalculatedFields()
{
	for(int nRowIndex = 0; nRowIndex < m_arTableRows.GetCount(); nRowIndex++)
	{
		TableRow *ptr = m_arTableRows.GetAt(nRowIndex);
		if(ptr->IsCalculated()) {
			return TRUE;
		}
	}

	for(int nColIndex = 0; nColIndex < m_arTableColumns.GetCount(); nColIndex++)
	{
		TableColumn *ptc = m_arTableColumns.GetAt(nColIndex);
		if(ptc->IsCalculated()) {
			return TRUE;
		}
	}

	return FALSE;
}

// (j.jones 2011-07-15 13:56) - PLID 42111 - exposed m_strBackgroundImageFilePath
CString CEMNDetail::GetBackgroundImageFilePath()
{
	return m_strBackgroundImageFilePath;
}

// (z.manning 2011-09-26 09:38) - PLID 45664
void CEMNDetail::Get3DImageOutputData(OUT CImageArray *paryImages)
{
	// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
	EnsureEmrItemAdvDlg(GetParentTopic()->GetTopicWndRaw());
	if(Is3DImage()) {
		((CEmrItemAdvImageDlg*)m_pEmrItemAdvDlg)->Get3DModelOutputData(paryImages);
	}
}

// (z.manning 2011-10-25 12:56) - PLID 39401
// (a.wilson 2013-03-26 13:55) - PLID 55826 - if the stamps aren't loaded then load them.
CEmrItemStampExclusions* CEMNDetail::GetStampExclusions()
{
	if (!m_bStampExclusionsLoaded)
		ReloadStampExclusions();

	return &m_StampExclusions;
}

// (z.manning 2011-10-25 12:56) - PLID 39401
// (a.wilson 2013-03-26 11:00) - PLID 55826 - set loaded flag.
void CEMNDetail::ReloadStampExclusions()
{
	m_StampExclusions.LoadFromData(m_nEMRInfoMasterID);
	m_bStampExclusionsLoaded = true;	
}

// (z.manning 2011-11-15 17:14) - PLID 38130
BOOL CEMNDetail::ShouldSpawnFromRememberedValues()
{
	if((m_nInfoFlags & eifDontSpawnFromRememberedValues) == 0) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
BOOL CEMNDetail::IsRequired()
{
	if ((m_nInfoFlags & eifRequired) == eifRequired) {
		return TRUE;
	} else {
		return FALSE;
	}
}

// (z.manning 2011-11-16 12:38) - PLID 38130
BOOL CEMNDetail::WasStateRemembered()
{
	if(m_nRememberedDetailID == -1) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// (z.manning 2011-11-30 11:14) - PLID 42765 - Determines if anything in the current detail references the Spawned Items
// sentence format field.
BOOL CEMNDetail::HasSpawnedItemsField()
{
	if(m_EMRInfoType == eitSingleList || m_EMRInfoType == eitMultiList)
	{
		for(int nListElementIndex = 0; nListElementIndex < m_arListElements.GetCount(); nListElementIndex++)
		{
			ListElement le = m_arListElements.GetAt(nListElementIndex);
			if(le.strLongForm.Find(SPAWNED_ITEMS_FIELD) != -1) {
				return TRUE;
			}
		}
	}
	else if(m_EMRInfoType == eitTable)
	{
		for(int nTableColumnIndex = 0; nTableColumnIndex < m_arTableColumns.GetCount(); nTableColumnIndex++)
		{
			TableColumn *ptc = m_arTableColumns.GetAt(nTableColumnIndex);
			if(ptc->m_strLongForm.Find(SPAWNED_ITEMS_FIELD) != -1) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

// (z.manning 2012-03-27 17:47) - PLID 33710
void CEMNDetail::UpdateTableCalculatedFields()
{
	if(m_EMRInfoType == eitTable && HasCalculatedFields())
	{
		// (z.manning 2012-03-27 17:54) - PLID 33710 - The function to update a table's calculated cells takes
		// an EMR item adv object. First let's check to see if we were given an override to use (such as from
		// the popup).
		CEmrItemAdvTableBase *pAdvTableBase = m_pAdvTableBaseOverride;
		if(pAdvTableBase == NULL && m_pEmrItemAdvDlg != NULL) {
			// (z.manning 2012-03-27 17:57) - PLID 33710 - We don't have an override so use the current item's dlg.
			pAdvTableBase = (CEmrItemAdvTableDlg*)m_pEmrItemAdvDlg;
		}
		else {
			// (z.manning 2012-03-27 17:57) - PLID 33710 - We don't have a table to update, which is fine, we'll simply
			// recalculate the values for the detail and they will be loaded when the adv dlg is created later on.
		}

		CEmrItemAdvTableBase::UpdateCalculatedFields(this, pAdvTableBase);
	}
}

// (c.haag 2012-04-02) - PLID 49350 - Returns FALSE if this detail is not a NexTech RTF-formatted narrative
BOOL CEMNDetail::IsRtfNarrative() const
{
	// Return FALSE if this isn't a narrative
	if (eitNarrative != m_EMRInfoType) {
		return FALSE;
	}

	// Examine the state contents to determine if this is a NexTech RTF formatted narrative.
	CString strHeader(NXRTF_HEADER_BEGIN);
	CString strDetailText = VarString(m_varState, "");
	if (strDetailText.GetLength() < strHeader.GetLength() || strDetailText.Left(strHeader.GetLength()) != strHeader)	{		
		return FALSE;
	} else {
		return TRUE;
	}
}

// (a.walling 2012-10-31 17:17) - PLID 53550 - LastSavedDetail - create and populate the last saved detail!
Emr::LastSavedDetail CEMNDetail::CreateLastSavedDetail()
{
	Emr::LastSavedDetail d;

	d.m_nEMRDetailID = m_nEMRDetailID;
	d.m_nEMRTemplateDetailID = m_nEMRTemplateDetailID;
	d.m_strLabelText = m_strLabelText;
	d.m_strMergeFieldOverride = m_strMergeFieldOverride;
	
	// (a.walling 2012-12-18 11:41) - PLID 53550 - Use the window's position if available; m_rcDefaultClientArea is not updated as often as you might think (PLID 54240)
	if(IsWindow(m_pEmrItemAdvDlg->GetSafeHwnd())) {
		CRect rc;
		m_pEmrItemAdvDlg->GetClientRect(&rc);
		m_pEmrItemAdvDlg->ClientToScreen(&rc);
		m_pEmrItemAdvDlg->GetParent()->SendMessage(NXM_CONVERT_RECT_FOR_DATA, (WPARAM)&rc);
		d.m_rcDefaultClientArea = rc;
	} else {
		d.m_rcDefaultClientArea = m_rcDefaultClientArea;
	}

	{
		CString strAuditName;
		GetDetailNameAndDataForAudit(strAuditName, d.m_strAuditData);
		ASSERT(strAuditName == d.GetAuditName());
	}

	// (a.walling 2012-10-31 17:17) - PLID 53551 - Copy the selected hotspot IDs
	if (!m_aisState->m_strSelectedHotSpotData.IsEmpty()) {
		std::string selectedHotSpots = m_aisState->m_strSelectedHotSpotData;
		
		d.m_selectedHotSpotIDs.reserve(1 + boost::count(selectedHotSpots, ';'));

		typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
		
		tokenizer tokens(
			selectedHotSpots, 
			boost::char_separator<char>("; ")
		);

		for(tokenizer::iterator it = tokens.begin(); it != tokens.end(); ++it) {
			d.m_selectedHotSpotIDs.push_back(boost::lexical_cast<long>(*it));
		}
	}

	// (a.walling 2012-10-31 17:17) - PLID 53551 - Copy the value of the stamps
	{
		d.m_imageStamps.reserve(m_arypImageStamps.GetSize());
		foreach (EmrDetailImageStamp* pStamp, (CArray<EmrDetailImageStamp*,EmrDetailImageStamp*>&)m_arypImageStamps) {
			if (!pStamp) continue;

			d.m_imageStamps.push_back(*pStamp);
		}
	}

	// (a.walling 2012-10-31 17:17) - PLID 53551 - Copy the list elements to our simple structure
	{
		d.m_listElements.reserve(m_arListElements.GetSize());
		foreach (const ListElement& le, m_arListElements) {
			d.m_listElements.push_back(Emr::SimpleListElement(le.nID, le.nDataGroupID, le.bIsSelected));
		}
	}

	// (a.walling 2012-10-31 17:17) - PLID 53552 - Copy the table row, column, and element information to our simple structures
	{
		d.m_tableRows.reserve(m_arTableRows.GetSize());
		d.m_tableColumns.reserve(m_arTableColumns.GetSize());
		d.m_tableElements.reserve(m_arTableElements.GetSize());

		foreach (TableRow* pRow, m_arTableRows) {
			if (!pRow) continue;
			d.m_tableRows.push_back(Emr::SimpleTableRow(pRow->m_ID.nDataID, pRow->m_ID.GetDetailImageStampID()));
		}

		foreach (TableColumn* pCol, m_arTableColumns) {
			if (!pCol) continue;
			d.m_tableColumns.push_back(Emr::SimpleTableColumn(pCol->nID));
		}

		foreach (TableElement& te, m_arTableElements) {
			d.m_tableElements.push_back(Emr::SimpleTableElement(te.m_pRow->m_ID.nDataID, te.m_pRow->m_ID.GetDetailImageStampID(), te.m_pColumn->nID, te.GetValueAsVariant()));
		}
	}

	return d;
}

// (j.jones 2013-08-07 16:12) - PLID 42958 - If a signature is added by another user,
// we'll add that information to the detail for auditing purposes.
// It's cleared after the initial audit is saved.
// This information is not filled when loading an EMN, they're only for new details.
void CEMNDetail::SetSignatureAddedByAnotherUser(bool bAddedByAnotherUser, CString strAddedByUserName)
{
	m_bIsSignatureAddedByAnotherUser = bAddedByAnotherUser;
	m_strSignatureAddedByAnotherUsername = strAddedByUserName;
}

// (j.jones 2014-01-13 14:35) - PLID 49971 - If the image could not be loaded,
// track the path we failed to load, such that we won't warn again if we try to load
// the same invalid path.
// If this is filled, it does not necessarily mean the path is still invalid.
CString CEMNDetail::GetLastWarnedInvalidImageFilePath()
{
	return m_strLastWarnedInvalidImageFilePath;
}

void CEMNDetail::SetLastWarnedInvalidImageFilePath(CString strLastWarnedInvalidImageFilePath)
{
	m_strLastWarnedInvalidImageFilePath = strLastWarnedInvalidImageFilePath;
}
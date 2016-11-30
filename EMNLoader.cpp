// EMNLoader.cpp: implementation of the CEMNLoader class.
//
//////////////////////////////////////////////////////////////////////

//
// (c.haag 2007-08-29 09:09) - This class is responsible for loading EMR chart and template data
// during the initial load of a chart or template, or by spawning. Extended comments may be found
// in EMNLoader.h. A graphical flow chart is also available at:
//
// \\yoda\Shared\Development\Documentation\Projects\EMR Speed 8400\emrflow.bmp
//

#include "stdafx.h"
#include "EMR.h"
#include "EMN.h"
#include "EmrTopic.h"
#include "EMNLoader.h"
#include "EmrTreeWnd.h"
#include "InvVisionWebUtils.h"
#include "WoundCareCalculator.h" // (r.gonet 08/03/2012) - PLID 51948
#include "NxAlgorithm.h"
#include <boost/unordered_set.hpp>
#include "EMNDetailStructures.h"
#include "EMNDetail.h"
#include "NxCache.h"
#include "EmrItemAdvImageState.h"	// (j.armen 2014-07-21 16:32) - PLID 62836

// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2013-03-27 10:06) - PLID 55901 - Cache our oft-gigantic system table data items.
namespace Emr
{
namespace Cache
{
	struct SystemTable
	{
		_variant_t id;
		_variant_t revision;
		CEMNLoader::CEmrDataItemArray data;

		SystemTable()
			: id(g_cvarNull)
			, revision(g_cvarNull)
		{}

		explicit SystemTable(const CEMNLoader::CEMRInfoItem& info)
			: id(info.m_nID)
			, revision(info.m_varRevision)
		{
			if (info.m_paDataItems) {
				data.SetSize(info.m_paDataItems->GetSize());
				for (int x = 0; x < info.m_paDataItems->GetSize(); ++x) {
					data[x] = info.m_paDataItems->ElementAt(x);
				}
			}
		}

		bool IsNull() const
		{
			return id.vt != VT_I4 || revision.vt != (VT_UI1 | VT_ARRAY);
		}

		CEMNLoader::CEmrDataItemArray* CreateEmrDataItemArray()
		{
			std::auto_ptr<CEMNLoader::CEmrDataItemArray> pData(new CEMNLoader::CEmrDataItemArray);

			pData->SetSize(data.GetSize());
			for (int x = 0; x < data.GetSize(); ++x) {
				pData->SetAt(x, data[x]);
			}

			return pData.release();
		}
	};

	typedef shared_ptr<SystemTable> SystemTablePtr;

	namespace detail
	{
		CCriticalSection lock;
		SystemTablePtr medications;
		SystemTablePtr allergies;
		
		SystemTablePtr GetTable(SystemTablePtr* source)
		{
			SystemTablePtr table;
			{
				CSingleLock lock(&detail::lock, TRUE);
				if (!*source) {
					*source = make_shared<SystemTable>();
				}
				table = *source;
			}
			return table;
		}

		SystemTablePtr UpdateTable(SystemTablePtr* source, SystemTablePtr newTable)
		{
			CSingleLock lock(&detail::lock, TRUE);
			*source = newTable;
			return newTable;
		}
	}
	
	SystemTablePtr Medications()
	{
		return detail::GetTable(&detail::medications);
	}

	SystemTablePtr Allergies()
	{
		return detail::GetTable(&detail::allergies);
	}

	SystemTablePtr UpdateMedications(CEMNLoader::CEMRInfoItem& info)
	{
		return detail::UpdateTable(&detail::medications, make_shared<SystemTable>(info));
	}

	SystemTablePtr UpdateAllergies(CEMNLoader::CEMRInfoItem& info)
	{
		return detail::UpdateTable(&detail::allergies, make_shared<SystemTable>(info));
	}
}
}

// (c.haag 2007-07-19 12:34) - This macro will throw an exception if not invoked from the main thread
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

// (c.haag 2007-08-27 09:56) - Uncomment this macro for debug reporting of what is being loaded and in what order
//#define REPORT_LOAD_PROGRESS

// (a.walling 2010-03-09 14:08) - PLID 37640 - Moved to cpp
CEMNLoaderMultiMutex::CEMNLoaderMultiMutex(CEMNLoaderMutex** ppMutexes, DWORD nMutexes)
{
	// Build the mutex arrays
	m_ppMutexes = new CEMNLoaderMutex*[nMutexes];
	m_ppSyncObjects = new CSyncObject*[nMutexes];
	for (DWORD i=0; i < nMutexes; i++) {
		m_ppMutexes[i] = ppMutexes[i];
		m_ppSyncObjects[i] = &m_ppMutexes[i]->m_mtx;
	}
	m_nMutexes = nMutexes;
	m_pMultiLock = new CMultiLock(m_ppSyncObjects, m_nMutexes);
	m_nLockCount = 0;
	m_nOwnerThreadID = 0;
}

CEMNLoaderMultiMutex::~CEMNLoaderMultiMutex()
{
	if (m_pMultiLock) {
		delete m_pMultiLock;
	}
	if (m_ppMutexes) {
		delete[] m_ppMutexes;
	}
	if (m_ppSyncObjects) {
		delete[] m_ppSyncObjects;
	}
}

void CEMNLoaderMultiMutex::Lock()
{
	// (c.haag 2007-07-11 12:02) - PLID 26623 - CMultiLock does not support
	// reference counting. If you call X.Lock() twice in a row, and
	// then call X.Unlock() twice in a row; and then later in another thread
	// call X.Lock(), the call waits indefinitely. I proved this in two separate
	// test apps. However, if you have two CMultiLock objects that lock the
	// same mutexes, and you call X.Lock(), Y.Lock(), X.Unlock(), Y.Unlock();
	// and later in another thread call X.Lock(), then the thread will correctly
	// lock it. This forms the basis for our solution: Whenever we call Lock(), 
	// we first lock the mutexes locally. Once we have them, we know that nothing
	// else can lock our mutexes from anywhere in any other thread; ergo, no thread
	// except ourselves will be able to access any of our member variables; ergo,
	// we're free to maintain our own lock count. Lets also track the thread ID
	// in case something fishy goes on.
	CMultiLock mlLocal(m_ppSyncObjects, m_nMutexes);
	// Take exclusive access of all of the mutexes we care about
	mlLocal.Lock();
	// Now ensure that this thread has valid claim to the mutexes
	DWORD nCurrentThreadID = GetCurrentThreadId();
	if (m_nLockCount > 0) { ASSERT(m_nOwnerThreadID == nCurrentThreadID); }
	else { m_nOwnerThreadID = nCurrentThreadID; }
	// Only perform the multi-lock if our lock count is zero.
	if (0 == m_nLockCount) {
		DWORD i;
		// Lock all involved Mutexes
		m_pMultiLock->Lock();
		// Update the lock counters for each CEMNLoader mutex. This
		// also increments the lock count for each.
		for (i=0; i < m_nMutexes; i++) {
			m_ppMutexes[i]->Lock();
		}
	}
	// Now increment our lock count and release our local lock. Don't worry;
	// we still own the mutex because we called m_pMultiLock->Lock() earlier.
	m_nLockCount++;
	mlLocal.Unlock();
}

void CEMNLoaderMultiMutex::Unlock()
{
	// (c.haag 2007-07-11 12:11) - PLID 26623 - Take exclusive access of all
	// of our mutexes to protect our member variables from being changed by 
	// other threads while we're in this function
	CMultiLock mlLocal(m_ppSyncObjects, m_nMutexes);
	mlLocal.Lock();
	m_nLockCount--; // Update the lock count
	ASSERT(m_nLockCount >= 0); // Safety check

	// If the lock count is zero, we're free to unlock our mutexes
	if (0 == m_nLockCount) {
		// Update the lock counters for each CEMNLoader mutex 
		DWORD i;
		for (i=0; i < m_nMutexes; i++) {
			m_ppMutexes[i]->Unlock();
		}
		// Now release the multi-lock
		if (m_pMultiLock) {
			m_pMultiLock->Unlock();
		}
	}
	// Now release our local lock...after this line of code, we are completely
	// done with the mutexes we owned, and other threads are free to use them.
	mlLocal.Unlock();
}

// (a.walling 2010-03-09 14:08) - PLID 37640 - Moved to cpp
CHoldEMNLoaderMultiMutex::CHoldEMNLoaderMultiMutex(CEMNLoaderMutex** ppMutexes, DWORD nMutexes)
{
	if (NULL != (m_pmtx = new CEMNLoaderMultiMutex(ppMutexes, nMutexes))) {
		m_pmtx->Lock();
	} else {
		ASSERT(FALSE);
	}
	bAutoDeleteMutex = TRUE;
}

CHoldEMNLoaderMultiMutex::CHoldEMNLoaderMultiMutex(CEMNLoaderMultiMutex* pmtx) {
	if (NULL != (m_pmtx = pmtx)) {
		m_pmtx->Lock();
	}
	bAutoDeleteMutex = FALSE;		
}

inline CHoldEMNLoaderMultiMutex::~CHoldEMNLoaderMultiMutex()
{
	Release();
	if (m_pmtx && bAutoDeleteMutex) delete m_pmtx;
}

inline void CHoldEMNLoaderMultiMutex::Release()
{
	if (m_pmtx) m_pmtx->Unlock();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// (c.haag 2007-05-01 12:31) - PLID 25853 - We need an EMN object for preloading topics
// (c.haag 2007-05-30 13:27) - PLID 26050 - We now let the caller configure our loading behavior
// (j.jones 2011-04-28 14:39) - PLID 43122 - added nProviderIDForFloatingData as a required parameter (send -1 if you don't have a provider)
CEMNLoader::CEMNLoader(CEMN* pEMN, _ConnectionPtr& pCon, BOOL bLoadFromTemplate, BOOL bIsTemplate, long nMasterID, long nProviderIDForFloatingData)
{
	// (a.walling 2013-07-18 10:04) - PLID 57629 - Ensure NxCache is up to date
	Nx::Cache::Checkpoint();

	// (c.haag 2007-02-26 18:03) - PLID 24949 - Reference counting support. We
	// always start with 1.
	m_nRefCnt = 1;

	// (j.jones 2011-04-28 14:39) - PLID 43122 - cache the nProviderIDForFloatingData
	m_nProviderIDForFloatingData = nProviderIDForFloatingData;

	// (c.haag 2007-05-08 12:58) - PLID 25790 - Assign supplimental variables
	m_bLoadFromTemplate = bLoadFromTemplate;
	m_bIsTemplate = bIsTemplate;
	m_nMasterID = nMasterID;

	// (c.haag 2007-04-20 09:08) - PLID 25730 - Load pertinent EMR info ID's
	// (j.jones 2007-07-20 14:46) - PLID 26742 - Do not load them here, wait until
	// we load the data into the loader.
	// Default to -2 such that if they become -1, we know we tried to find
	// the IDs, and found none.
	//m_nActiveCurrentMedicationsInfoID = -2;
	//m_nActiveAllergiesInfoID = -2;

	// (c.haag 2007-04-24 13:38) - Reset all preloaded map population values.
	// These will be filled on a need-to-fill basis for efficient loading.
	m_bEmrInfoDefaultsPopulated = FALSE;
	m_bSelectMapPopulated = FALSE;
	m_bHotSpotMapPopulated = FALSE;
	m_bTemplateSelectMapPopulated = FALSE;
	m_bTemplateHotSpotMapPopulated = FALSE;
	m_bTableMapPopulated = FALSE;
	m_bTemplateTableMapPopulated = FALSE;
	m_bRememberedDetailMapPopulated = FALSE;
	m_bRememberCandidateMapsPopulated = FALSE;
	m_bEMRDataTItemsLoaded = FALSE;
	m_bAllStatesLoaded = FALSE;
	m_bAllEmrItemLinkedDataItemsLoaded = FALSE;
	m_bIgnoreLoadingActions = FALSE;		//DRT 7/26/2007 - PLID 26835

	// (c.haag 2007-08-10 17:50) - PLID 27049 - Default to TRUE so that the EMN
	// we are preloading will iterate through our list of Live details until
	// spawning takes place, at which point the EMN must iterate through its own
	// detail lists because spawned details are never included in preloader detail
	// iterations.
	SetManagingEMNDetailArray(TRUE);

	// (c.haag 2007-04-30 14:01) - PLID 25853 - Remember the EMN object for topic
	// creation
	if (NULL == (m_pEMN = pEMN)) {
		ThrowNxException("Could not create CEMNLoader object without a valid EMN!");
	}

	// (c.haag 2007-07-05 14:48) - PLID 26595 - Allocate a special mutex for simultaneously
	// locking access to internal details and EMR info data
	CEMNLoaderMutex* pMutexes[] = { &m_mtxEMRInfo, &m_mtxInternalDetails };
	m_pmtxInternalDetailsAndEMRInfo = new CEMNLoaderMultiMutex(pMutexes, 2);
}

CEMNLoader::~CEMNLoader()
{
	//
	// (c.haag 2007-02-27 09:23) - PLID 24949 - When this destructor is called,
	// it means that the EMN preloading is done. Most, if not all the details,
	// will be owned by a CEMN object. The rest of the details had source action
	// ID's and were never spawned. Those will be deleted here.
	//
	// With the exception of m_mapActions, every value we delete here should be
	// non-null. However, a null check is in place for completion.
	//
	POSITION pos;
	int i;

	//TRACE("Releasing EMN Loader object for EMN %s. Estimated minimum memory usage = %d bytes.\n",
	//	m_pEMN->GetDescription(), GetEstimatedMinimumMemoryUsage());

	// (c.haag 2007-09-10 13:00) - PLID 27340 - Free memory related to maps which are not directly
	// related to topic and detail objects
	Retire();

	// Clean up our topic map
	pos = m_mapDetailTopics.GetStartPosition();
	while (pos != NULL) {
		CEMNDetailArray* papDetails;
		long nTopic;
		m_mapDetailTopics.GetNextAssoc( pos, nTopic, papDetails );
		if (papDetails) delete papDetails;
	}

	// (c.haag 2011-03-17) - PLID 42895 - Clean up EMR Info Common Lists
	pos = m_mapCommonLists.GetStartPosition();
	while (NULL != pos)
	{
		CEmrInfoCommonListCollection* p;
		long nEmrInfoID;
		m_mapCommonLists.GetNextAssoc( pos, nEmrInfoID, p );
		if (p) delete p;
	}

	// (z.manning 2011-04-06 17:50) - PLID 43140
	pos = m_mapDetailIDToOrder.GetStartPosition();
	while (NULL != pos)
	{
		CEmrDataGroupOrderArray* p;
		long nDetailID;
		m_mapDetailIDToOrder.GetNextAssoc(pos, nDetailID, p);
		if (p != NULL) {
			delete p;
		}
	}

	// Go through and delete all of the details that we did not use in any topic
	// (c.haag 2007-07-05 15:24) - PLID 26595 - Don't use a protection mutex.
	// The destructor should be a mutex-free zone.
	int nDetails = m_apAllDetails.GetSize();
	for (i=0; i < nDetails; i++) {
		CEMNDetail* pDetail = m_apAllDetails[i]->m_pDetail;
		// (c.haag 2007-07-27 09:37) - PLID 26833 - Internal details should always have a
		// reference count of one
		// (a.walling 2009-10-12 16:05) - PLID 36023 - There is no reason for us to have any knowledge of the reference count
		/*if(1 != pDetail->Internal_GetRefCnt()) {
			AfxDebugBreak();
		}*/
		//pDetail->Release();
		// (a.walling 2009-10-12 16:05) - PLID 36023
		pDetail->__Release("~CEMNLoader AllDetails");
		delete m_apAllDetails[i];
	}

	// (c.haag 2007-04-30 12:57) - PLID 25853 - Go through and delete all of the
	// preloaded topic objects
	const int nTopics = m_apAllTopics.GetSize();
	for (i=0; i < nTopics; i++) {
		delete m_apAllTopics[i];
	}

	// (c.haag 2007-05-02 13:01) - PLID 25881 - Go through and delete all of the
	// template topic objects
	const int nTemplateTopics = m_apAllTemplateTopics.GetSize();
	for (i=0; i < nTemplateTopics; i++) {
		delete m_apAllTemplateTopics[i];
	}

	// (c.haag 2010-06-29 12:19) - PLID 39404
	for (i=0; i < m_apSmartStampTemplateItemPairs.GetSize(); i++) {
		delete m_apSmartStampTemplateItemPairs[i];
	}

	// (c.haag 2008-07-18 16:11) - PLID 30784 - Free memory allocated by EMR problem
	// preloading
	ClearEmrProblemMap(&m_mapEmrItemProblems);
	ClearEmrProblemMap(&m_mapEmrDataItemProblems);
	ClearEmrProblemMap(&m_mapEmrTopicProblems);
	ClearEmrProblemMap(&m_mapEMNProblems);
	ClearEmrProblemMap(&m_mapEMRProblems);
	ClearEmrProblemMap(&m_mapEmrDiagProblems);
	ClearEmrProblemMap(&m_mapEmrChargeProblems);
	ClearEmrProblemMap(&m_mapEmrMedicationProblems);

	// (c.haag 2007-07-05 14:49) - PLID 26595 - Delete our multi-lock mutex
	if (m_pmtxInternalDetailsAndEMRInfo) {
		delete m_pmtxInternalDetailsAndEMRInfo;
	}

	// (z.manning 2011-10-25 09:52) - PLID 39401 - Clean any stamp exclusion data
	pos = m_mapInfoIDToStampExclusions.GetStartPosition();
	while(pos != NULL)
	{
		long nEmrInfoID;
		CEmrItemStampExclusions *pStampExclusions = NULL;
		m_mapInfoIDToStampExclusions.GetNextAssoc(pos, nEmrInfoID, pStampExclusions);
		delete pStampExclusions;
	}

	// (c.haag 2007-08-03 15:05) - PLID 26833 - Some Live details may not have
	// been loaded onto an EMN. Those are the ones with a reference count of
	// two: One for us, and one reserved for the EMN. These were never used
	// outside the EMN loader, so delete them.
	// (a.walling 2007-10-12 12:55) - PLID 27017 - The preview may be holding on
	// to a reference here, so check for that before deleting.
	for (i=0; i < m_apLiveDetails.GetSize(); i++) {
		CEMNDetail* pLiveDetail = m_apLiveDetails[i];
		// (a.walling 2009-10-12 10:03) - PLID 36023 - HULK SMASH!@*(&(
		// Do not perform any special behavior based on the reference count! Instead, reference count appropriately.
		/*
		if (2 == pLiveDetail->GetRefCnt()) {
			// check the DetailsPendingUpdate array
			if (pLiveDetail->m_pParentTopic != NULL && pLiveDetail->m_pParentTopic->GetParentEMN() != NULL) {
				CEmrTreeWnd* pInterfaceWnd = (CEmrTreeWnd*)pLiveDetail->m_pParentTopic->GetParentEMN()->GetInterface();
				if (pInterfaceWnd) {
					if (pInterfaceWnd->IsDetailInPendingUpdateArray(pLiveDetail)) {
						continue;
					}
				}
			}

			m_apLiveDetails.RemoveAt(i);
			delete pLiveDetail;
			i--;
		}
		*/
		//pLiveDetail->Release();
		BOOL bFinalized = FALSE;
		BOOL bInFinalizedMap = m_mapFinalizedDetails.Lookup(pLiveDetail, bFinalized);
		if (bInFinalizedMap) {
#ifdef LOG_ALL_EMNDETAIL_REFCOUNTS
			CString strRefCntDebugString;
			if (!bFinalized) {
				strRefCntDebugString.Format("WARNING: live detail 0x%08x finalization discrepancy (finalized %li, in map %li)\n", pLiveDetail, bFinalized, bInFinalizedMap);				
			} else {
				strRefCntDebugString.Format("NOTE: live detail 0x%08x has already been finalized\n", pLiveDetail);
			}
			::OutputDebugString(strRefCntDebugString);
#endif

			// (a.walling 2009-10-23 13:00) - PLID 36023 - We keep a reference in the finalized map now
			pLiveDetail->__Release("~CEMNLoader Finalized Live Detail");
		}
		else {
			// (c.haag 2010-06-29 16:14) - PLID 39412 - Release the unfinalized live detail
			if (NULL == pLiveDetail->m_pParentTopic) {
				// If we get here, it never had a parent topic, so release it here.
				pLiveDetail->__Release("~CEMNLoader Unfinalized Live Detail");
			}
			else {
				// This has a parent topic; don't release it because the topic will do it later.
			}
		}

		// (a.walling 2009-10-12 16:05) - PLID 36023 - Release the live detail
		pLiveDetail->__Release("~CEMNLoader Live Details");
	}
}

//////////////////////////////////////////////////////////////////////
// Reference Counting
//////////////////////////////////////////////////////////////////////
//
// (c.haag 2007-04-24 08:52) - This object has one reference for every
// EMN topic plus one for the EMN itself
//
void CEMNLoader::AddRef()
{
	// (c.haag 2007-02-26 18:03) - PLID 24949 - Reference counting support
	m_mtxRefCnt.Lock();
	m_nRefCnt++;
	m_mtxRefCnt.Unlock();
}

void CEMNLoader::Release()
{
	// (c.haag 2007-02-26 18:03) - PLID 24949 - Reference counting support
	BOOL bDelete = FALSE;
	m_mtxRefCnt.Lock();
	if (0 == --m_nRefCnt) {
		bDelete = TRUE;
	}
	m_mtxRefCnt.Unlock();
	if (bDelete) { delete this; }
}

long CEMNLoader::GetRefCnt()
{
	// (c.haag 2007-02-27 09:51) - PLID 24949 - Returns the reference count
	long nResult;
	m_mtxRefCnt.Lock();
	nResult = m_nRefCnt;
	m_mtxRefCnt.Unlock();
	return nResult;
}


//////////////////////////////////////////////////////////////////////
// Preloading functions called from the main thread
//////////////////////////////////////////////////////////////////////


//DRT 9/13/2007 - PLID 27384 - The recordset passed in now contains data for all EMNs in the current EMR, not necessarily
//	just a single EMN.  Make sure we are properly filtering on the EMN ID as necessary.
void CEMNLoader::PreloadEmrDetails(_RecordsetPtr& rsDetails)
{
	//
	// (c.haag 2007-04-24 08:25) - PLID 26463 - Loads all of the non-deleted details for a patient EMN,
	// regardless of their source action ID / spawning behavior
	//
	//
	// The recordset passed into this function comes from a recordset created by one of the
	// following functions:
	//
	//		CEMN::LoadFromEmnID
	//		CEMR::LoadFromID
	//
	// The detail filter is:
	//
	//	WHERE EMRDetailsT.EMRID = %li AND EMRDetailsT.Deleted = 0 AND EMRTopicsT.Deleted = 0
	//
	// Meaning we only get undeleted details for the EMN we're loading. This is a straight-forward
	// load where every detail should go into the EMN. What we do is load the details into the internal
	// detail array (m_apAllDetails), and also into the Live detail array (m_apLiveDetails) because
	// they will all inevitably be added to the EMN. Please refer to EMNLoader.h for information on
	// "Live" details.
	//
	//
	// (c.haag 2007-07-03 10:21) - PLID 26523 - Take exclusive ownership of EMR info objects until this
	// function is done
	// (c.haag 2007-07-05 10:08) - PLID 26595 - Take exclusive ownership of internal details
	CHoldEMNLoaderMultiMutex mh(m_pmtxInternalDetailsAndEMRInfo);
	
	_ConnectionPtr pCon(rsDetails->GetActiveConnection());
	FieldsPtr f = rsDetails->Fields;
	//DRT 9/13/2007 - PLID 27384 - We now have the data for all EMNs in this EMR, so we need to filter.  The records
	//	 are sorted by EMN ID.
	while (!rsDetails->eof && AdoFldLong(rsDetails, "EMRID") == m_nMasterID) {
		CPreloadedDetail* pPreloadedDetail = new CPreloadedDetail;
		// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
		pPreloadedDetail->m_pDetail = CEMNDetail::CreateDetail(NULL, "Preloaded detail", FALSE);
		pPreloadedDetail->m_pDetail->SetOwnedByEMNLoader(TRUE); // This is used for debugging
						// and verification of a proper load by AssertIfPostInitialLoadIssuesExist()
		pPreloadedDetail->m_vDetailText = AdoFldVar(f, "Text");
		pPreloadedDetail->m_vDetailImageTextData = AdoFldVar(f, "ImageTextData");
		pPreloadedDetail->m_vDetailPrintData = AdoFldVar(f, "PrintData"); // (z.manning 2011-10-05 17:12) - PLID 45842
		pPreloadedDetail->m_vDetailInkData = AdoFldVar(f, "InkData");

		// (r.gonet 2013-04-08 17:48) - PLID 56150 - Technically, these serialized fields should all be NULL if there is no data for them but
		//  ensure that we handle cases of bad data where the field is an empty byte array.
		if(pPreloadedDetail->m_vDetailImageTextData.vt != VT_NULL && GetElementCountFromSafeArrayVariant(pPreloadedDetail->m_vDetailImageTextData) == 0) {
			pPreloadedDetail->m_vDetailImageTextData = g_cvarNull;
		}
		if(pPreloadedDetail->m_vDetailPrintData.vt != VT_NULL && GetElementCountFromSafeArrayVariant(pPreloadedDetail->m_vDetailPrintData) == 0) {
			pPreloadedDetail->m_vDetailPrintData = g_cvarNull;
		}
		if(pPreloadedDetail->m_vDetailInkData.vt != VT_NULL && GetElementCountFromSafeArrayVariant(pPreloadedDetail->m_vDetailInkData) == 0) {
			pPreloadedDetail->m_vDetailInkData = g_cvarNull;
		}

		pPreloadedDetail->m_vDetailInkImagePathOverride = AdoFldVar(f, "InkImagePathOverride");
		pPreloadedDetail->m_vDetailInkImageTypeOverride = AdoFldVar(f, "InkImageTypeOverride");
		/* (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore zoom and pan offsets.
		pPreloadedDetail->m_vDetailZoomLevel = f->Item["ZoomLevel"]->Value;
		pPreloadedDetail->m_vDetailOffsetX = f->Item["OffsetX"]->Value;
		pPreloadedDetail->m_vDetailOffsetY = f->Item["OffsetY"]->Value;*/
		pPreloadedDetail->m_vDetailSliderValue = AdoFldVar(f, "SliderValue");
		
		// (a.walling 2008-06-30 13:56) - PLID 29271 - Preview Pane flags
		pPreloadedDetail->m_vPreviewFlags = AdoFldVar(f, "PreviewFlags");
		// (c.haag 2007-05-01 12:32) - PLID 25853 - Populate the topic ID
		pPreloadedDetail->m_nTopicID = AdoFldLong(rsDetails, "EMRTopicID");

		// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
		// (z.manning 2011-01-25 10:31) - PLID 42336 - These were never actually used
		//pPreloadedDetail->m_vChildEMRInfoMasterID = f->Item["ChildEMRInfoMasterID"]->Value;
		//pPreloadedDetail->m_vSmartStampsEnabled = f->Item["SmartStampsEnabled"]->Value;
		//pPreloadedDetail->m_vChildEMRDetailID = f->Item["ChildEMRDetailID"]->Value;
		//pPreloadedDetail->m_vChildEMRTemplateDetailID = f->Item["ChildEMRTemplateDetailID"]->Value;
		//pPreloadedDetail->m_vParentEMRDetailID = f->Item["ParentEMRDetailID"]->Value;
		//pPreloadedDetail->m_vParentEMRTemplateDetailID = f->Item["ParentEMRTemplateDetailID"]->Value;
		CEMNDetailArray* papDetails;

		// Load everything about the detail from the recordset, but do not load the state.
		// We will load the state later in a thread.
		// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
		pPreloadedDetail->m_pDetail->LoadFromDetailRecordset(rsDetails, FALSE, pCon, FALSE);

		// (j.jones 2010-02-12 14:19) - PLID 37318 - ensure SmartStamp items point to each other
		EnsureInternalDetailSmartStampLinks(pPreloadedDetail->m_pDetail);

		CEMNLoaderMutex* pmtxProblems = GetEMRProblemsMutex();
		CHoldEMNLoaderMutex mhProblems(pmtxProblems);

		// (j.jones 2008-07-21 09:19) - PLID 30779 - Chris moved the problem loading into GetEmrProblems
		// (c.haag 2009-05-16 11:37) - PLID 34277 - Associate EMR objects with problem links rather than problems
		{
			CEmrProblemLinkAry *pAry = GetEmrProblemLinks(eprtEmrItem, pPreloadedDetail->m_pDetail->m_nEMRDetailID);
			if(pAry) {
				// (j.jones 2008-07-18 12:05) - PLID 30779 - problems are now stored in an array of objects
				for(int i = 0; i < pAry->GetSize(); i++) {
					CEmrProblemLink *pNewLink = new CEmrProblemLink(pAry->GetAt(i));
					pNewLink->UpdatePointersWithDetail(pPreloadedDetail->m_pDetail);
					pPreloadedDetail->m_pDetail->m_apEmrProblemLinks.Add(pNewLink);
				}
			}
		}

		// (j.jones 2008-07-24 13:59) - PLID 30779 - now load list item problems
		// (c.haag 2009-05-16 11:37) - PLID 34277 - Associate EMR objects with problem links rather than problems
		{
			CEmrProblemLinkAry *pAry = GetEmrProblemLinks(eprtEmrDataItem, pPreloadedDetail->m_pDetail->m_nEMRDetailID);
			if(pAry) {
				// (j.jones 2008-07-18 12:05) - PLID 30779 - problems are now stored in an array of objects
				for(int i = 0; i < pAry->GetSize(); i++) {
					CEmrProblemLink *pNewLink = new CEmrProblemLink(pAry->GetAt(i));
					pNewLink->UpdatePointersWithDetail(pPreloadedDetail->m_pDetail);
					pPreloadedDetail->m_pDetail->m_apEmrProblemLinks.Add(pNewLink);
				}
			}
		}

		// Add the detail to the master list and map
		m_apAllDetails.Add(pPreloadedDetail);
		m_apAllCEMNDetails.Add(pPreloadedDetail->m_pDetail); // (c.haag 2007-07-02 09:14) - PLID 26515 - Add to the "thin" version of the all details array
		if (pPreloadedDetail->m_pDetail->m_nEMRDetailID > 0) m_mapAllDetailsByID[pPreloadedDetail->m_pDetail->m_nEMRDetailID] = pPreloadedDetail;
		if (pPreloadedDetail->m_pDetail->m_nEMRTemplateDetailID > 0) m_mapAllDetailsByTemplateID[pPreloadedDetail->m_pDetail->m_nEMRTemplateDetailID] = pPreloadedDetail;

		// (c.haag 2007-07-26 19:19) - PLID 25239 - Create the "Live" version of the detail
		// (c.haag 2007-07-27 10:32) - PLID 26833 - Moved to here
		// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
		CEMNDetail* pLiveDetail = CEMNDetail::CreateDetail(NULL, "Live detail", FALSE);
		//pLiveDetail->AddRef(); // (c.haag 2007-08-02 17:08) - PLID 26833 - Add a reference now, and then release it later in FinalizeDetailTransitionToEMN
		// (a.walling 2009-10-12 16:05) - PLID 36023
		pLiveDetail->__AddRef("PreloadEmrDetails initial live detail");
		pLiveDetail->CopyFrom(*pPreloadedDetail->m_pDetail); // (a.walling 2012-10-31 17:17) - PLID 53550 - CEMNDetail::opreator= leads to very confusing semantics

		// (j.jones 2010-02-25 14:40) - PLID 37318 - ensure the SmartStamp links, if any, remain valid
		// (z.manning 2010-03-12 11:43) - PLID 37412 - This is a live detail
		ReconfirmSmartStampLinks_ByLiveDetail(pLiveDetail, NULL);

		m_apLiveDetails.Add(pLiveDetail);
		m_mapLiveDetailsByID[ pLiveDetail->m_nEMRDetailID ] = pLiveDetail;

		// Add the detail to our topic map
		if (!m_mapDetailTopics.Lookup(pPreloadedDetail->m_nTopicID, papDetails)) {
			m_mapDetailTopics[pPreloadedDetail->m_nTopicID] = papDetails = new CEMNDetailArray;
		}
		if (NULL != papDetails) {
			papDetails->Add(pLiveDetail);
		} else {
			ThrowNxException("Could not allocate data for CEMNLoader::PreloadEmrDetails!");
		}

		// (c.haag 2007-04-26 17:49) - PLID 26463 - Now populate our EmrInfoT map
		const long nEMRInfoID = AdoFldLong(f, "EmrInfoID");
		CEMRInfoItem* pItem = NULL;
		if (!m_mapInfoItems.Lookup(nEMRInfoID, pItem)) {
			if (NULL == (pItem = new CEMRInfoItem)) {
				ThrowNxException("Could not allocate data for CEMNLoader::PreloadEmrDetails!");
			}
			pItem->m_nID = nEMRInfoID;
			m_mapInfoItems[pItem->m_nID] = pItem;
			pItem->m_DataType = (EmrInfoType)AdoFldByte(f, "DataType");
			pItem->m_DataSubType = (EmrInfoSubType)AdoFldByte(f, "DataSubType");
			pItem->m_strBackgroundImageFilePath = AdoFldString(f, "Info_BackgroundImageFilePath", "");
			pItem->m_BackgroundImageType = (eImageType)AdoFldLong(f, "Info_BackgroundImageType", -1);
			pItem->m_strDefaultText = AdoFldString(f, "Info_DefaultText", "");
			pItem->m_dSliderMin = AdoFldDouble(f, "Info_SliderMin", 0.0);
			pItem->m_dSliderMax = AdoFldDouble(f, "Info_SliderMax", 10.0);
			pItem->m_dSliderInc = AdoFldDouble(f, "Info_SliderInc", 1.0);
			pItem->m_bAutoAlphabetizeListData = AdoFldBool(f, "Info_AutoAlphabetizeListData"); // (a.walling 2013-03-18 09:32) - PLID 55724 - Load AutoAlphabetizeListData for EMRInfoT records in the CEMNLoader
			pItem->m_varRevision = f->Item["Info_Revision"]->Value; // (a.walling 2013-03-27 10:05) - PLID 55900 - CEMNLoader needs to load the revision of EMRInfoT records
			pItem->m_strLongForm = AdoFldString(f, "Info_LongForm");
			pItem->m_nDataFormat = AdoFldLong(f, "DataFormat", 0);
			pItem->m_strDataSeparator = AdoFldString(f, "Info_DataSeparator", ", ");
			pItem->m_strDataSeparatorFinal = AdoFldString(f, "Info_DataSeparatorFinal", pItem->m_strDataSeparator == ", " ? " and " : pItem->m_strDataSeparator);
			pItem->m_bDisableTableBorder = AdoFldBool(f, "Info_DisableTableBorder", FALSE);
			pItem->m_bRememberForPatient = AdoFldBool(f, "Info_RememberForPatient");
			// (j.jones 2008-09-22 15:02) - PLID 31408 - supported RememberForEMR
			pItem->m_bRememberForEMR = AdoFldBool(f, "Info_RememberForEMR");
			// (c.haag 2008-10-16 11:27) - PLID 31709 - TableRowsAsFields
			pItem->m_bTableRowsAsFields = AdoFldBool(f, "TableRowsAsFields");

			// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
			pItem->m_nChildEMRInfoMasterID = AdoFldLong(f, "ChildEMRInfoMasterID", -1);
			pItem->m_bSmartStampsEnabled = AdoFldBool(f, "SmartStampsEnabled", FALSE);
			// (z.manning 2011-01-25 15:24) - PLID 42336 - These are both deprecated as we now support multiple images
			// for one smart stamp table.
			//pItem->m_nParentEMRDetailID = AdoFldLong(f, "ParentEMRDetailID", -1);
			//pItem->m_nParentEMRTemplateDetailID = AdoFldLong(f, "ParentEMRTemplateDetailID", -1);

			//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
			pItem->m_bHasGlassesOrderData = AdoFldBool(f, "HasGlassesOrderData", FALSE);
			pItem->m_golLens = (GlassesOrderLens)AdoFldLong(f, "GlassesOrderLens", (long)golInvalid);
			//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
			pItem->m_bHasContactLensData = AdoFldBool(f, "HasContactLensData", FALSE);
			// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
			pItem->m_bUseWithWoundCareCoding = AdoFldBool(f, "UseWithWoundCareCoding", FALSE);

			// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
			pItem->m_nInfoFlags = AdoFldLong(f, "InfoFlags", 0);

			// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
			// because they are now only calculated in the API, and not in Practice code
			/*
			// (j.jones 2007-08-27 10:35) - PLID 27056 - load the E/M coding data
			pItem->m_nEMCodeCategoryID = AdoFldLong(f, "EMCodeCategoryID", -1);
			// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_eEMCodeUseTableCategories
			pItem->m_eEMCodeUseTableCategories = (EMCodeUseTableCategories)AdoFldLong(f, "EMCodeUseTableCategories", (long)emcutcNone);
			pItem->m_bUseEMCoding = AdoFldBool(f, "Info_UseEMCoding", FALSE);
			pItem->m_emctEMCodingType = (EMCodingTypes)AdoFldLong(f, "EMCodingType", (long)emctUndefined);
			*/

			// (j.jones 2007-07-18 15:04) - PLID 26730 - load whether or not the info item has Info actions,
			// which it usually does not, such that we don't have to search for them later
			pItem->m_eHasInfoActions = AdoFldLong(f, "HasInfoActions", 0) == 1 ? ehiasHasInfoItems : ehiasHasNoInfoItems;

			// (j.jones 2007-10-02 17:25) - PLID 26810 - added the EMRInfoT.Name
			pItem->m_strLabelText = AdoFldString(f, "Name", "");

			// (a.walling 2008-06-30 12:37) - PLID 29271 - Preview Pane flags
			pItem->m_nPreviewFlags = AdoFldLong(f, "Info_PreviewFlags", 0);
		}
		rsDetails->MoveNext();
	}
}

// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

void CEMNLoader::PreloadEmrTemplateDetails(long nPatientID, _RecordsetPtr& rsTemplateDetails, CEMR* pParentEMR)
{
	//
	// (c.haag 2007-04-24 08:25) - PLID 26464 - Loads all of the non-deleted details for a template,
	// regardless of their source action ID / spawning behavior
	//
	// The recordset passed into this function comes from a recordset created by one of the
	// following functions:
	//
	//		CEMN::LoadFromTemplateID
	//		CEMN::CreateMintItemsLoader
	//
	// The detail filter is:
	//
	//		WHERE TemplateID = ? 
	//		OR EMRTemplateTopicID IN (SELECT SourceTemplateTopicID FROM EMRTemplateTopicsT WHERE TemplateID = ?)
	//
	// This is a tough one to follow because it involves spawning. Here is the breakdown:
	//
	//		WHERE TemplateID = ?
	//			All the details on the template either added manually by the user, or individually spawned by
	//			details added by the user
	//
	//		OR EMRTemplateTopicID IN (SELECT SourceTemplateTopicID FROM EMRTemplateTopicsT WHERE TemplateID = ?)
	//			If a user spawns another detail or template from within this EMN template, all of the details which correspond
	//			to the spawned detail or template will be brought in per this filter. For example, if you create a new template,
	//			and add one list detail that spawns items from templates, and then you check the box, then save your
	//			template, and close an open your template again, and your template ID is 824, the results will look
	//			something like:
	//
	//			20542	5212	636	Chief Complaint Plastic - Reconstructive
	//			21169	5212	636	Chief Complaint Plastic - Cosmetic
	//			21170	5212	636	Lesion Number
	//			...
	//			27974	5212	636	H: Part of Consult H&P Plastic Surgery
	//			28025	8771	824	Spawning Analysis Spawner
	//
	//		Where 824 is the ID of your new template, and 636 is the ID of the template you spawned from within your template.
	//		Now, the user may decide to uncheck the box later on. If that happens, the query will give you *exactly the same 
	//		results*. We won't know whether any of the details from template 636 will be spawned until CEMN::ProcessEmrActions
	//		is called. Still, we load them in anyway so that we have the data in memory for ProcessEmrActions to use to manufacture
	//		spawned details with on new patient charts.
	//
	// Here is the patch query in all its glory, run once per topic to load from:
	//
	//	_RecordsetPtr rsDetails = CreateRecordset(pConn, "SELECT EMRTemplateDetailsT.ID "
	//		"FROM EMRTemplateDetailsT INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID "
	//		"LEFT JOIN EMRInfoT ON EmrInfoMasterT.ActiveEMRInfoID = EmrInfoT.ID "
	//		"WHERE EMRTemplateDetailsT.EMRTemplateTopicID = %li %s"
	//		"ORDER BY EMRInfoT.Name", nTopicToLoadFrom, pLoadInfo->bLoadToTemplate?"":"AND EmrTemplateDetailsT.SourceActionID Is Null ");
	//
	// In the patch version of LoadEMRTopic: If we are loading to a template, we blindly take in all the details for a given topic. When creating
	// a patient chart, we only include details with NULL source action ID's.
	//
	// Also, we do not modify the Live details array. That is not done until we reach PreloadEmrTemplateTopics. Please
	// read EMNLoader.h for additional information about Live details.
	//
	//
	// (c.haag 2007-07-03 10:22) - PLID 26523 - Take exclusive ownership of EMR info objects until this
	// function is done
	// (c.haag 2007-07-05 10:08) - PLID 26595 - Take exclusive ownership of internal details
	CHoldEMNLoaderMultiMutex mh(m_pmtxInternalDetailsAndEMRInfo);

	_ConnectionPtr pCon( rsTemplateDetails->GetActiveConnection() );

	long nEMRGroupID = -1;

	// (c.haag 2007-04-25 15:35) - PLID 25758 - Make sure the m_mapRememberCandidates map is populated
	// so that we can assign remembered detail information to the preloaded detail (think of the
	// "Remember for this patient" EMR info flag)
	// (c.haag 2007-05-17 12:34) - Don't do it if we're loading into a template
	if (!m_bIsTemplate) {
		if(pParentEMR) {
			nEMRGroupID = pParentEMR->GetID();
		}
		EnsureRememberCandidateMaps(nPatientID, nEMRGroupID, pCon);
	}

	FieldsPtr f = rsTemplateDetails->Fields;
	while (!rsTemplateDetails->eof) {
		BOOL bRememberForPatient = AdoFldBool(f, "Info_RememberForPatient");
		// (j.jones 2008-09-22 15:02) - PLID 31408 - supported RememberForEMR
		BOOL bRememberForEMR = AdoFldBool(f, "Info_RememberForEMR");

		CPreloadedDetail* pPreloadedDetail = new CPreloadedDetail;
		// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
		pPreloadedDetail->m_pDetail = CEMNDetail::CreateDetail(NULL, "Template preloaded detail", FALSE);
		pPreloadedDetail->m_pDetail->SetOwnedByEMNLoader(TRUE); // This is used for debugging
						// and verification of a proper load by AssertIfPostInitialLoadIssuesExist()

		if(!m_bIsTemplate && (bRememberForPatient || bRememberForEMR)) {

			// (j.jones 2008-09-22 16:26) - PLID 31408 - Now we can remember for patient
			// OR remember for EMR, where "for patient" is any previous usage of the item
			// on the patient's account, and "for EMR" looks only for usages on other EMNs
			// on the same EMR.

			CRememberCandidate* pValue = NULL;
			if(bRememberForPatient) {

				CString strPatientKey;
				strPatientKey.Format("%d#%d", AdoFldLong(f, "EmrInfoMasterID"), AdoFldByte(f, "DataType"));

				if(m_mapRememberForPatientCandidates.Lookup(strPatientKey, (LPVOID&)pValue)) {
					pPreloadedDetail->m_vInfoRememberedDetailID = pValue->m_nDetailID;
					pPreloadedDetail->m_vInfoRememberedDetailInfoID = pValue->m_nEMRInfoID;
				}
			}
			else if(bRememberForEMR) {

				CString strEMRKey;			
				strEMRKey.Format("%d#%d#%d", AdoFldLong(f, "EmrInfoMasterID"), nEMRGroupID, AdoFldByte(f, "DataType"));

				if(m_mapRememberForEMRCandidates.Lookup(strEMRKey, (LPVOID&)pValue)) {
					pPreloadedDetail->m_vInfoRememberedDetailID = pValue->m_nDetailID;
					pPreloadedDetail->m_vInfoRememberedDetailInfoID = pValue->m_nEMRInfoID;
				}
			}
		}

		pPreloadedDetail->m_vInfoDefaultText = AdoFldVar(f, "DefaultText");
		pPreloadedDetail->m_vInfoSliderValue = AdoFldVar(f, "SliderValue");
		// (c.haag 2007-06-15 10:15) - PLID 26344 - Also remember the template ID
		pPreloadedDetail->m_nTemplateID = AdoFldLong(f, "TemplateID");

		// (c.haag 2007-04-24 13:31) - PLID 25758 - If we have a legitimate remembered detail, add it
		// to a map of remembered details. Later on, when LoadEmrDetailState is called, we will populate
		// the map values with CPreloadedDetail objects.
		if (!m_bIsTemplate && VT_I4 == pPreloadedDetail->m_vInfoRememberedDetailID.vt) {
			m_mapRememberedDetails[ VarLong(pPreloadedDetail->m_vInfoRememberedDetailID) ] = NULL;
		}
		// (c.haag 2007-05-03 13:11) - Load the template topic ID into pPreloadedDetail->m_nTopicID
		pPreloadedDetail->m_nTopicID = AdoFldLong(f, "EMRTemplateTopicID");

		// (z.manning 2011-10-06 16:49) - PLID 45842 - Print data
		pPreloadedDetail->m_vDetailPrintData = AdoFldVar(f, "PrintData");

		// (c.haag 2007-07-25 18:24) - PLID 25881 - Track the fact that the topic's detail has details
		// for use with PreloadEmrTemplateTopics
		m_mapTemplateTopicsWithDetails[pPreloadedDetail->m_nTopicID] = TRUE;
	
		// Load everything about the detail from the recordset, but do not load the state.
		// We will load the state later in the thread.
		// (j.jones 2007-04-12 14:22) - PLID 25604 - bIsInitialLoad is TRUE for this function
		// (j.jones 2007-07-24 17:05) - PLID 26742 - added parameter for pParentEmr
		// (j.jones 2008-09-23 09:42) - PLID 31408 - send the EMRGroupID
		// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
		pPreloadedDetail->m_pDetail->LoadFromTemplateDetailRecordset(rsTemplateDetails, FALSE, m_bIsTemplate,
			FALSE/*bIsNewTopic*/, TRUE, nPatientID, nEMRGroupID, pCon, FALSE, pParentEMR, this);

		// (j.jones 2010-02-12 14:19) - PLID 37318 - ensure SmartStamp items point to each other
		EnsureInternalDetailSmartStampLinks(pPreloadedDetail->m_pDetail);

		// Add the detail to the master list and map
		m_apAllDetails.Add(pPreloadedDetail);		
		m_apAllCEMNDetails.Add(pPreloadedDetail->m_pDetail); // (c.haag 2007-07-02 09:14) - PLID 26515 - Add to the "thin" version of the all details array
		if (pPreloadedDetail->m_pDetail->m_nEMRDetailID > 0) m_mapAllDetailsByID[pPreloadedDetail->m_pDetail->m_nEMRDetailID] = pPreloadedDetail;
		if (pPreloadedDetail->m_pDetail->m_nEMRTemplateDetailID > 0) m_mapAllDetailsByTemplateID[pPreloadedDetail->m_pDetail->m_nEMRTemplateDetailID] = pPreloadedDetail;

		// (c.haag 2007-04-23 11:43) - PLID 25761 - Now populate our EmrInfoT map
		const long nEMRInfoID = AdoFldLong(f, "EmrInfoID");
		CEMRInfoItem* pItem = NULL;
		if (!m_mapInfoItems.Lookup(nEMRInfoID, pItem)) {
			if (NULL == (pItem = new CEMRInfoItem)) {
				ThrowNxException("Could not allocate data for CEMNLoader::PreloadEmrTemplateDetails!");
			}
			pItem->m_nID = nEMRInfoID;
			m_mapInfoItems[pItem->m_nID] = pItem;
			pItem->m_DataType = (EmrInfoType)AdoFldByte(f, "DataType");
			pItem->m_DataSubType = (EmrInfoSubType)AdoFldByte(f, "DataSubType");
			pItem->m_strBackgroundImageFilePath = AdoFldString(f, "Info_BackgroundImageFilePath", "");
			pItem->m_BackgroundImageType = (eImageType)AdoFldLong(f, "Info_BackgroundImageType", -1);
			pItem->m_strDefaultText = AdoFldString(f, "Info_DefaultText", "");
			pItem->m_dSliderMin = AdoFldDouble(f, "Info_SliderMin", 0.0);
			pItem->m_dSliderMax = AdoFldDouble(f, "Info_SliderMax", 10.0);
			pItem->m_dSliderInc = AdoFldDouble(f, "Info_SliderInc", 1.0);
			pItem->m_bAutoAlphabetizeListData = AdoFldBool(f, "Info_AutoAlphabetizeListData"); // (a.walling 2013-03-18 09:32) - PLID 55724 - Load AutoAlphabetizeListData for EMRInfoT records in the CEMNLoader
			pItem->m_varRevision = f->Item["Info_Revision"]->Value; // (a.walling 2013-03-27 10:05) - PLID 55900 - CEMNLoader needs to load the revision of EMRInfoT records
			pItem->m_strLongForm = AdoFldString(f, "Info_LongForm");
			pItem->m_nDataFormat = AdoFldLong(f, "Info_DataFormat", 0);
			pItem->m_strDataSeparator = AdoFldString(f, "Info_DataSeparator", ", ");
			pItem->m_strDataSeparatorFinal = AdoFldString(f, "Info_DataSeparatorFinal", pItem->m_strDataSeparator == ", " ? " and " : pItem->m_strDataSeparator);
			pItem->m_bDisableTableBorder = AdoFldBool(f, "Info_DisableTableBorder", FALSE);
			pItem->m_bRememberForPatient = bRememberForPatient;
			// (j.jones 2008-09-22 15:02) - PLID 31408 - supported RememberForEMR
			pItem->m_bRememberForEMR = bRememberForEMR;
			// (c.haag 2008-10-16 11:27) - PLID 31709 - TableRowsAsFields
			pItem->m_bTableRowsAsFields = AdoFldBool(f, "TableRowsAsFields");

			// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
			pItem->m_nChildEMRInfoMasterID = AdoFldLong(f, "ChildEMRInfoMasterID", -1);
			pItem->m_bSmartStampsEnabled = AdoFldBool(f, "SmartStampsEnabled", FALSE);
			// (z.manning 2011-01-25 15:24) - PLID 42336 - These are both deprecated as we now support multiple images
			// for one smart stamp table.
			//pItem->m_nParentEMRDetailID = AdoFldLong(f, "ParentEMRDetailID", -1);
			//pItem->m_nParentEMRTemplateDetailID = AdoFldLong(f, "ParentEMRTemplateDetailID", -1);

			//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
			pItem->m_bHasGlassesOrderData = AdoFldBool(f, "HasGlassesOrderData", FALSE);
			pItem->m_golLens = (GlassesOrderLens)AdoFldLong(f, "GlassesOrderLens", (long)golInvalid);
			//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
			pItem->m_bHasContactLensData = AdoFldBool(f, "HasContactLensData", FALSE);
			// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding
			pItem->m_bUseWithWoundCareCoding = AdoFldBool(f, "UseWithWoundCareCoding", FALSE);

			// (z.manning 2011-11-15 17:04) - PLID 38130 - Added InfoFlags
			pItem->m_nInfoFlags = AdoFldLong(f, "InfoFlags", 0);

			// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
			// because they are now only calculated in the API, and not in Practice code
			/*
			// (j.jones 2007-08-27 10:35) - PLID 27056 - load the E/M coding data
			pItem->m_nEMCodeCategoryID = AdoFldLong(f, "EMCodeCategoryID", -1);
			// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_eEMCodeUseTableCategories
			pItem->m_eEMCodeUseTableCategories = (EMCodeUseTableCategories)AdoFldLong(f, "EMCodeUseTableCategories", (long)emcutcNone);
			pItem->m_bUseEMCoding = AdoFldBool(f, "Info_UseEMCoding", FALSE);
			pItem->m_emctEMCodingType = (EMCodingTypes)AdoFldLong(f, "EMCodingType", -1);
			*/

			// (j.jones 2007-07-18 15:04) - PLID 26730 - load whether or not the info item has Info actions,
			// which it usually does not, such that we don't have to search for them later
			pItem->m_eHasInfoActions = AdoFldLong(f, "HasInfoActions", 0) == 1 ? ehiasHasInfoItems : ehiasHasNoInfoItems;

			// (j.jones 2007-10-02 17:25) - PLID 26810 - added the EMRInfoT.Name
			pItem->m_strLabelText = AdoFldString(f, "Name", "");
			
			// (a.walling 2008-06-30 12:37) - PLID 29271 - Preview Pane flags
			pItem->m_nPreviewFlags = AdoFldLong(f, "Info_PreviewFlags", 0);
		}
		rsTemplateDetails->MoveNext();
	}

// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

	// (c.haag 2010-06-29 11:26) - PLID 39404 - Gather all internal details that have smart stamp links and put them
	// into elements in m_apSmartStampTemplateItemPairs. We need to use that array later in ReconfirmSmartStampLinks_ByLiveDetail
	// because it normally searches through live maps to connect table-image pairs, but spawned details do not exist
	// in those live maps. ReconfirmSmartStampLinks_ByLiveDetail will instead search through m_apSmartStampTemplateItemPairs.
	// (z.manning 2011-01-21 10:27) - PLID 42338 - Changed this to support multiple images per smart stamp table
	for (int i=0; i < m_apAllDetails.GetSize(); i++)
	{
		CEMNDetailArray* paryImages = m_apAllDetails[i]->m_pDetail->GetSmartStampImageDetails();
		CEMNDetail* pTable = m_apAllDetails[i]->m_pDetail->GetSmartStampTableDetail();
		if (paryImages->GetCount() > 0 || pTable)
		{
			// If we get here, this detail is no doubt a member of a smart stamp table-image pair. Add it to
			// m_apSmartStampTemplateItemPairs (but make sure it's never duplicated).
			BOOL bFound = FALSE;
			for (int j=0; j < m_apSmartStampTemplateItemPairs.GetSize() && !bFound; j++) 
			{
				CSmartStampImageTablePairs* pPair = m_apSmartStampTemplateItemPairs[j];
				if (pPair->m_paryInternalImages == paryImages || pPair->m_pInternalTable == pTable) {
					bFound = TRUE;
				}
			}
			if (!bFound) 
			{
				// Doesn't exist. Add it now. Deallocation takes place in the EMN loader destructor.
				CSmartStampImageTablePairs* pPair = new CSmartStampImageTablePairs;
				if (paryImages->GetCount() > 0) {
					pPair->m_paryInternalImages = paryImages;
					pPair->m_pInternalTable = m_apAllDetails[i]->m_pDetail;
				}
				else {
					pPair->m_pInternalTable = pTable;
					pPair->m_paryInternalImages = pTable->GetSmartStampImageDetails();
				}				
				m_apSmartStampTemplateItemPairs.Add(pPair);
			}
			else {
				// Already exists
			}
		}
	} // for (int i=0; i < m_apAllDetails.GetSize(); i++)
}

//DRT 9/13/2007 - PLID 27384 - The recordset passed in now contains data for all EMNs in the current EMR, not necessarily
//	just a single EMN.  Make sure we are properly filtering on the EMN ID as necessary.
void CEMNLoader::PreloadEmrTopics(ADODB::_RecordsetPtr& rsTopics)
{
	//
	// (c.haag 2007-04-30 12:27) - PLID 25853 - Loads all of the non-deleted topics for a patient EMN
	//
	// The recordset passed into this function comes from a recordset created by one of the
	// following functions:
	//
	//		CEMN::LoadFromEmnID
	//		CEMR::LoadFromID
	//
	// The topic filter is:
	//
	//	WHERE EmrTopicsT.EMRID = %d AND EmrTopicsT.Deleted = 0
	//
	// Meaning we only get undeleted topics for the EMN we are loading. This is a straight forward load where every
	// undeleted topic will go into the EMN.
	//
	_ConnectionPtr pCon( rsTopics->GetActiveConnection() );
	int i;

	// (c.haag 2007-07-03 09:28) - PLID 26523 - Take exclusive ownership of topics until this function is done
	// (c.haag 2007-07-10 10:12) - PLID 26595 - Also details
	CEMNLoaderMutex* pMutexes[] = { &m_mtxTopics, &m_mtxInternalDetails };
	CHoldEMNLoaderMultiMutex mh(pMutexes, 2);

	FieldsPtr f = rsTopics->Fields;
	//DRT 9/13/2007 - PLID 27384 - We now have the data for all EMNs in this EMR, so we need to filter.  The records
	//	 are sorted by EMN ID.
	while (!rsTopics->eof && AdoFldLong(rsTopics, "EMRID") == m_nMasterID) {
		CPreloadedTopic* pTopic = new CPreloadedTopic;
		pTopic->m_vName = f->Item["Name"]->Value;
		pTopic->m_nID = AdoFldLong(f, "ID");
		pTopic->m_vParentTopicID = f->Item["EmrParentTopicID"]->Value;
		pTopic->m_vTemplateTopicID = f->Item["EMRTemplateTopicID"]->Value;
		pTopic->m_vSourceActionID = f->Item["SourceActionID"]->Value;
		// (z.manning 2009-03-05 14:21) - PLID 33338 - SourceDataGroupID
		pTopic->m_vSourceDataGroupID = f->Item["SourceDataGroupID"]->Value;
		// (z.manning 2010-02-25 11:01) - PLID  37532 - SourceDetailImageStampID
		pTopic->m_vSourceDetailImageStampID = f->Item["SourceDetailImageStampID"]->Value;
		//TES 3/17/2010 - PLID 37530 - This is an EMN, so the SourceStampID and SourceStampIndex are NULL
		pTopic->m_vSourceStampID = g_cvarNull;
		pTopic->m_vSourceStampIndex = g_cvarNull;
		pTopic->m_vSourceActionType = f->Item["SourceType"]->Value;
		//DRT 9/25/2007 - PLID 27515 - Added SourceAction SourceID and SourceDataGroupID
		pTopic->m_vSourceActionSourceID = f->Item["SourceActionSourceID"]->Value;
		pTopic->m_vSourceActionSourceDataGroupID = f->Item["SourceActionSourceDataGroupID"]->Value;
		pTopic->m_vSourceActionSourceHotSpotGroupID = f->Item["EmrSpotGroupID"]->Value;
		pTopic->m_vSourceActionSourceTableDropdownGroupID = f->Item["SourceActionSourceTableDropdownGroupID"]->Value; // (z.manning 2009-02-13 10:11) - PLID 33070
		pTopic->m_vSourceDetailID = f->Item["SourceDetailID"]->Value;
		pTopic->m_vSourceTemplateTopicID = f->Item["SourceTemplateTopicID"]->Value;
		pTopic->m_vShowIfEmpty = f->Item["ShowIfEmpty"]->Value;
		pTopic->m_vTemplateID = f->Item["TemplateID"]->Value;
		pTopic->m_vTemplateOrderIndex = f->Item["TemplateOrderIndex"]->Value;
		pTopic->m_vOrderIndex = f->Item["OrderIndex"]->Value;
		pTopic->m_vSourceActionName = f->Item["SourceActionName"]->Value;
		pTopic->m_vSourceActionDestType = f->Item["SourceActionDestType"]->Value;
		pTopic->m_vSourceTemplateID = f->Item["SourceTemplateID"]->Value;
		// (a.walling 2008-06-30 12:42) - PLID 29271 - Preview Pane flags
		pTopic->m_vPreviewFlags = f->Item["PreviewFlags"]->Value;
		m_apAllTopics.Add(pTopic);
		m_mapAllTopicsByID[pTopic->m_nID] = pTopic;
		rsTopics->MoveNext();
	}

	// Now populate the subtopic ID list for each topic. Remember, we have not yet considered
	// spawning information. So, some of these subtopics may not even be used. We won't know
	// whether they will be used until LoadEMRTopic is called.
	const int nEMNTopics = m_apAllTopics.GetSize();
	for (i=0; i < nEMNTopics; i++) {
		CPreloadedTopic* pPreloadedTopic = m_apAllTopics[i];
		const long nParentTopicID = VarLong(pPreloadedTopic->m_vParentTopicID, -1);
		if (nParentTopicID != -1) {
			CPreloadedTopic* pParentTopic = m_mapAllTopicsByID[ nParentTopicID ];
			if (NULL == pParentTopic) {
				// This should never happen
				ASSERT(FALSE);
				ThrowNxException("CEMNLoader::PreloadEmrTopics detected a topic whose parent does not exist in the same EMN!");
			}
			pParentTopic->m_anSubTopicIDs.Add(pPreloadedTopic->m_nID);
		}
	}

	// Update the "Has Details" flag of our preloaded topic list.
	const int nEMNDetails = m_apAllDetails.GetSize();
	for (i=0; i < nEMNDetails; i++) {
		const long nTopicID = m_apAllDetails[i]->m_nTopicID;
		CPreloadedTopic* pTopic = m_mapAllTopicsByID[ nTopicID ];
		if (NULL == pTopic) {
			CString strError;
			strError.Format("CEMNLoader:PreloadEmrTopics found a detail that belongs to a topic outside this EMN! (Detail ID = %d Topic = %d)",
				m_apAllDetails[i]->m_pDetail->m_nEMRDetailID, nTopicID );
			ASSERT(FALSE);
			ThrowNxException(strError);
		} else {
			pTopic->m_bHasDetails = TRUE;
		}
	}

	for (i=0; i < nEMNTopics; i++) {
		CPreloadedTopic* pPreloadedTopic = m_apAllTopics[i];
		const long nParentTopicID = VarLong(pPreloadedTopic->m_vParentTopicID, -1);
		if (nParentTopicID == -1) {
			// (c.haag 2007-05-01 09:41) - This is a root level topic, so we need to
			// start the loading process for that now
			// (c.haag 2007-05-08 16:29) - PLID 25941 - We now maintain the CEMRTopic object in the
			// preloaded object so that it can be passed along to the main thread after the topic is
			// loaded
			//TES 4/15/2010 - PLID 24692 - Pass in the topic ID
			CEMRTopic* pNewTopic = pPreloadedTopic->m_pTopic = m_pEMN->AddTopic(VarString(pPreloadedTopic->m_vName, ""), pPreloadedTopic->m_nID);
			// (j.jones 2008-09-22 16:09) - PLID 31408 - send the EMR pointer
			// (z.manning 2010-08-20 09:50) - PLID 40190 - Pass in connection pointer
			pNewTopic->LoadFromPreloadedTopic(pCon, this, pPreloadedTopic->m_nID, 
				m_pEMN->GetParentEMR()->GetPatientID(), 
				(m_pEMN->GetInterface()) ? m_pEMN->GetInterface()->GetSafeHwnd() : NULL,
				m_pEMN->GetParentEMR());

			CEMNLoaderMutex* pmtxProblems = GetEMRProblemsMutex();
			CHoldEMNLoaderMutex mhProblems(pmtxProblems);
			
			// (j.jones 2008-07-23 13:46) - PLID 30789 - load the problems for this topic
			// (c.haag 2009-05-16 11:37) - PLID 34277 - Associate EMR objects with problem links rather than problems
			CEmrProblemLinkAry *pAry = GetEmrProblemLinks(eprtEmrTopic, pNewTopic->GetID());
			if(pAry) {
				for(int i = 0; i < pAry->GetSize(); i++) {
					CEmrProblemLink *pNewLink = new CEmrProblemLink(pAry->GetAt(i));
					pNewLink->UpdatePointersWithTopic(pNewTopic);
					pNewTopic->m_apEmrProblemLinks.Add(pNewLink);
				}
			}
		}
	}
}

// (z.manning 2009-02-13 10:11) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
// (z.manning 2009-03-04 14:38) - PLID 33338 - Use the new source action info class
void CEMNLoader::PreloadEmrTemplateTopics(ADODB::_RecordsetPtr& rsTemplateTopics, long nEmrTemplateID,
										  BOOL bIsNewTopic, HWND hWnd, SourceActionInfo &sai, BOOL bLoadHiddenTopics /*= FALSE*/, OPTIONAL IN long nPatientID /*= -1*/,
										  OPTIONAL IN CEMR *pEmr /*= NULL*/, BOOL bLoadRootTopics /*= TRUE*/, 
										  OPTIONAL IN long nSourceActionSourceID /*= -1*/, OPTIONAL IN long nSourceActionSourceDataGroupID /*= -1*/, OPTIONAL IN long nSourceActionSourceHotSpotGroupID /* = -1 */, OPTIONAL IN long nSourceActionSourceTableDropdownGroupID /* = -1 */)
{
	//
	// (c.haag 2007-05-02 12:50) - PLID 25881 - Loads all topics for a template
	//
	// The recordset passed into this function comes from a recordset created by one of the
	// following functions:
	//
	//		CEMN::LoadFromTemplateID
	//		CEMN::CreateMintItemsLoader
	//
	// The topic filter is:
	//
	//		WHERE EMRTemplateTopicsT.TemplateID = ? 
	//		OR EMRTemplateTopicsT.ID IN (SELECT SourceTemplateTopicID FROM EMRTemplateTopicsT WHERE TemplateID = ?) 
	//
	// This is a tough one to follow because it involves spawning. Here is the breakdown:
	//
	//		WHERE EMRTemplateTopicsT.TemplateID = ? 
	//			These are all the topics in the template either manually created by the user, or spawned by the user. 
	//
	//		OR EMRTemplateTopicID IN (SELECT SourceTemplateTopicID FROM EMRTemplateTopicsT WHERE TemplateID = ?)
	//			These are all the topics in the original template spawned by the user.
	//
	//		Put the two together, and it means that the query will have two records for every topic you spawned.
	//
	//		If you create a new template, and add one list detail that spawns items from templates, and then you check 
	//		the box, then save your template, and close an open your template again, and your template ID is 824, the 
	//		results will look something like:
	//
	//		5520	636	NULL	NULL	NULL	NULL	0	A/P	28	0	0	NULL	NULL	NULL		NULL	NULL
	//		5460	636	NULL	NULL	NULL	5453	0	Abdomen	6	0	0	NULL	NULL	NULL		NULL	NULL
	//		5574	636	NULL	NULL	NULL	5569	0	Abdomen/ Trunk 	4	0	0	NULL	NULL	NULL		NULL	NULL
	//		...
	//		8791	824	5520	24209	28025	NULL	0	A/P	28	0	0	636	A/P	0	Create mint	9	24209
	//		8788	824	5460	24209	28025	8784	0	Abdomen	6	0	0	636	Abdomen	0	Create mint	9	24209
	//		8799	824	5574	24209	28025	8794	0	Abdomen/ Trunk 	4	0	0	636	Abdomen/ Trunk 	0	Create mint	9	24209
	//	
	//		Notice that column 3 (SourceTemplateTopicID) for each topic corresponding to your template (824) refers to
	//		the topic ID's of topics from another template (624). In the interests of speed and completeness, we load
	//		all the topics, even if they won't be spawned. We won't know for certain until CEMN::ProcessEmrActions is called,
	//		and if it is, everything we need is already loaded in memory.
	//
	// In the patch version of LoadEMRTopic: Whichever topics we load depends on whether or not we are creating a patient chart,
	// as well as variables in pLoadInfo.
	//
	//
	// (c.haag 2007-05-31 12:52) - PLID 26175 - Added an optional flag for the loading of root topics
	//
	// The query used to create rsTemplateTopics contains ALL topics for the template defined in
	// nEmrTemplateID, even if they may not be spawned. It also contains ALL source template topics,
	// which can exist even outside of nEmrTemplateID.
	//
	// We add elements to the Live details array in this function. More comments can be read below. Please
	// read EMNLoader.h for additional information about Live details.
	//
	// No need to worry about sorting. The preload query sorts by OrderIndex, and everything is inserted
	// into m_apAllTemplateTopics in order of OrderIndex.
	//
	_ConnectionPtr pCon( rsTemplateTopics->GetActiveConnection() );
	int i;

	// (c.haag 2007-07-03 09:28) - PLID 26523 - Take exclusive ownership of topics until this function is done
	// (c.haag 2007-07-10 10:14) - PLID 26595 - Also details
	CEMNLoaderMutex* pMutexes[] = { &m_mtxTopics, &m_mtxInternalDetails };
	CHoldEMNLoaderMultiMutex mh(pMutexes, 2);

	FieldsPtr f = rsTemplateTopics->Fields;
	while (!rsTemplateTopics->eof) {
		// Load in topic information
		CPreloadedTemplateTopic* pTemplateTopic = new CPreloadedTemplateTopic;
		pTemplateTopic->m_nID = AdoFldLong(f, "EMRTemplateTopicID");
		pTemplateTopic->m_nTemplateID = AdoFldLong(f, "TemplateID");
		pTemplateTopic->m_vEMRParentTemplateTopicID = f->Item["EMRParentTemplateTopicID"]->Value;
		pTemplateTopic->m_vSourceTemplateTopicID = f->Item["SourceTemplateTopicID"]->Value;
		pTemplateTopic->m_vSourceActionID = f->Item["SourceActionID"]->Value;
		// (z.manning 2009-03-05 14:22) - PLID 33338 - SourceDataGroupID
		pTemplateTopic->m_vSourceDataGroupID = f->Item["SourceDataGroupID"]->Value;
		pTemplateTopic->m_vSourceActionType = f->Item["SourceType"]->Value;
		//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
		pTemplateTopic->m_vSourceStampID = f->Item["SourceStampID"]->Value;
		pTemplateTopic->m_vSourceStampIndex = f->Item["SourceStampIndex"]->Value;
		//DRT 9/26/2007 - PLID 27515 - Added SourceAction SourceID and SourceDataGroupID
		pTemplateTopic->m_vSourceActionSourceID = f->Item["SourceActionSourceID"]->Value;
		pTemplateTopic->m_vSourceActionSourceDataGroupID = f->Item["SourceActionSourceDataGroupID"]->Value;
		pTemplateTopic->m_vSourceActionSourceHotSpotGroupID = f->Item["EmrSpotGroupID"]->Value;
		pTemplateTopic->m_vSourceActionSourceTableDropdownGroupID = f->Item["SourceActionSourceTableDropdownGroupID"]->Value; // (z.manning 2009-02-13 10:13) - PLID 33070
		pTemplateTopic->m_vSourceDetailID = f->Item["SourceDetailID"]->Value;
		pTemplateTopic->m_vShowIfEmpty = f->Item["ShowIfEmpty"]->Value;
		pTemplateTopic->m_vName = f->Item["Name"]->Value;
		pTemplateTopic->m_vOrderIndex = f->Item["OrderIndex"]->Value;
		pTemplateTopic->m_vHideOnEMN = f->Item["HideOnEMN"]->Value;
		// (j.jones 2007-07-30 11:25) - PLID 26874 - we now include the template's "AddOnce" field
		pTemplateTopic->m_vTemplateAddOnce = f->Item["AddOnce"]->Value;
		// (c.haag 2007-07-24 16:30) - PLID 26344 - These three fields will be non-null
		// if this topic was spawned by a list detail somewhere in the template
		pTemplateTopic->m_vOriginalTemplateID = f->Item["OriginalTemplateID"]->Value;
		pTemplateTopic->m_vOriginalTopicName = f->Item["OriginalTopicName"]->Value;
		pTemplateTopic->m_vOriginalShowIfEmpty = f->Item["OriginalShowIfEmpty"]->Value;
		// (a.walling 2008-06-30 12:43) - PLID 29271 - Preview Pane flags
		pTemplateTopic->m_vPreviewFlags = f->Item["PreviewFlags"]->Value;
		m_apAllTemplateTopics.Add(pTemplateTopic);
		m_mapAllTemplateTopicsByID[pTemplateTopic->m_nID] = pTemplateTopic;

		// Load in action information if any is available. We'll need it later for
		// CEMNLoader::LoadEMRTopic.
		_variant_t vEmrActionID = f->Item["EmrActionID"]->Value;
		if (VT_NULL != vEmrActionID.vt) {
			// We'll have a PTA meeting here
			PreloadedTopicAction pta;
			pta.m_vEmrActionName = f->Item["EmrActionName"]->Value;
			pta.m_vEmrActionDestType = f->Item["EmrActionDestType"]->Value;
			m_mapEmrTemplateTopicActions[VarLong(vEmrActionID)] = pta;
		}
		rsTemplateTopics->MoveNext();
	}

	// (c.haag 2007-08-16 11:21) - PLID 26833 - Populate m_mapDetailTopics. For this map, the key
	// is the topic ID, and the value is an array of internal details. This is used later on in
	// LoadEMRTopic.
	for (i=0; i < m_apAllDetails.GetSize(); i++) {
		CPreloadedDetail* pPreloadedDetail = m_apAllDetails[i];
		// Add the detail to our topic map
		CEMNDetailArray* papDetails = NULL;
		if (!m_mapDetailTopics.Lookup(pPreloadedDetail->m_nTopicID, papDetails)) {
			m_mapDetailTopics[pPreloadedDetail->m_nTopicID] = papDetails = new CEMNDetailArray;
		}
		if (NULL != papDetails) {
			papDetails->Add(pPreloadedDetail->m_pDetail);
		} else {
			ThrowNxException("Could not allocate data for CEMNLoader::PreloadEmrTemplateTopics!");
		}
	}

	// Now populate the subtopic ID list for each template topic.
	const int nTemplateTopics = m_apAllTemplateTopics.GetSize();
	for (i=0; i < nTemplateTopics; i++) {
		CPreloadedTemplateTopic* pTemplateTopic = m_apAllTemplateTopics[i];

		const long nParentTopicID = VarLong(pTemplateTopic->m_vEMRParentTemplateTopicID, -1);
		if (nParentTopicID > 0) {
			CPreloadedTemplateTopic* pParentTopic = m_mapAllTemplateTopicsByID[ nParentTopicID ];
			if (NULL == pParentTopic) {
				//
				// If we get here, it means that this topic's parent no longer exists. You can
				// reach this impasse by doing the following:
				//
				// 1. Make a copy of the Consult H&P Plastic Surgery template
				// 2. Delete the HPI topic
				// 3. Spawn abdomen/ trunk
				// 4. Save
				// 5. Close and reopen
				//
				// Since we mean to update the parent topic's sub-topic array in this loop, and
				// the parent does not exist, then there's nothing we can do here. 
				//
			} else {
				pParentTopic->m_anSubTemplateTopicIDs.Add(pTemplateTopic->m_nID);
			}
		}
	}

	// (c.haag 2007-08-08 11:08) - PLID 26833 - We now need to populate the "Live" details array.
	// The live details array should consist of all details that would have been loaded by LoadEMRTopic
	// for all non-spawned topics and details. As for the spawned topics, they will be loaded in
	// CEMN::ProcessEmrActions either from this loader, or another CEMNLoader object created by
	// CEMN::ProcessEmrActions.
	//
	// To explain which details exactly qualify as Live details, we must look to what topics and details
	// that the 8300 code loads.
	//
	CMap<long,long,CPreloadedTemplateTopic*,CPreloadedTemplateTopic*> mapQualifyingLiveTopics;
	//
	// Root topics:
	//		WHERE EmrTemplateTopicsT.TemplateID = %li AND EmrTemplateTopicsT.EmrParentTemplateTopicID Is Null AND %s,
	//			(nTemplateID),
	//			(bLoadHiddenTopics?"1=1":"EmrTemplateTopicsT.SourceActionID Is Null")
	//
	// Despite the query, anything with a non-null source action ID cannot be a Live detail because it may be spawned
	// by CEMN::ProcessEmrActions
	//
	for (i=0; i < nTemplateTopics; i++) {
		CPreloadedTemplateTopic* pTemplateTopic = m_apAllTemplateTopics[i];
		if (pTemplateTopic->m_nTemplateID == m_nMasterID							// EmrTemplateTopicsT.TemplateID = %li
			&& VT_NULL == pTemplateTopic->m_vEMRParentTemplateTopicID.vt			// EmrTemplateTopicsT.EmrParentTemplateTopicID Is Null
			&& VT_NULL == pTemplateTopic->m_vSourceActionID.vt
			)
		{
			mapQualifyingLiveTopics[pTemplateTopic->m_nID] = pTemplateTopic;
		}
	}
	// Non-root topics:
	//		WHERE EMRParentTemplateTopicID = %li AND %s AND %s,
	//			(pLoadInfo->nID),
	//			(strSpawned),
	//			(pLoadInfo->bLoadToTemplate ? "1=1" : "HideOnEMN = 0")
	//
	// where strSpawned is defined as:
	//	if(pLoadInfo->bLoadHiddenTopics) {
	//		strSpawned = "1=1";
	//	}
	//	else {
	//		if(pLoadInfo->nOverrideSourceActionID != -1) {
	//			strSpawned.Format("(SourceActionID = %li OR SourceActionID Is Null)", pLoadInfo->nOverrideSourceActionID);
	//		}
	//		else {
	//			strSpawned = "SourceActionID Is Null";
	//		}
	//	}
	//
	// Despite the query, anything with a non-null source action ID cannot be a Live detail because it may be spawned
	// by CEMN::ProcessEmrActions. This really amounts to the same query as above, but with non-root topics
	//
	BOOL bMapChanged;
	do {
		bMapChanged = FALSE;
		for (i=0; i < nTemplateTopics; i++) {
			CPreloadedTemplateTopic* pTemplateTopic = m_apAllTemplateTopics[i];
			if (pTemplateTopic->m_nTemplateID == m_nMasterID
				&& VT_NULL != pTemplateTopic->m_vEMRParentTemplateTopicID.vt
				&& VT_NULL == pTemplateTopic->m_vSourceActionID.vt
				)
			{
				// Make sure it's not already in the map
				CPreloadedTemplateTopic* pDummy;
				if (!mapQualifyingLiveTopics.Lookup(pTemplateTopic->m_nID, pDummy)) {

					// Make sure its parent exists in the map
					if (mapQualifyingLiveTopics.Lookup(VarLong(pTemplateTopic->m_vEMRParentTemplateTopicID), pDummy)) {

						// Success, this qualifies as a live topic because every parent
						// also qualifies as a live topic (in addition to this topic meeting
						// the criteria as well)
						mapQualifyingLiveTopics[pTemplateTopic->m_nID] = pTemplateTopic;
						bMapChanged = TRUE;
					}
				}
			}
		}
	} while (bMapChanged);
	//
	// Now that we have our qualifying topics, proceed to find qualifying details and add them to the Live array
	//
	POSITION pos = mapQualifyingLiveTopics.GetStartPosition();
	while (pos != NULL) {
		CPreloadedTemplateTopic* pTemplateTopic = NULL;
		long nTemplateTopicID;
		mapQualifyingLiveTopics.GetNextAssoc( pos, nTemplateTopicID, pTemplateTopic );
		CEMNDetailArray* papTopicDetails = NULL;
		m_mapDetailTopics.Lookup( pTemplateTopic->m_nID, papTopicDetails );
		if (NULL != papTopicDetails) {
			// If it has details, add them to the live list
			const int nDetails = papTopicDetails->GetSize();
			for (int j=0; j < nDetails; j++) {
				//
				// Detail filter for each topic:
				//		WHERE EMRTemplateDetailsT.EMRTemplateTopicID = %li %s,
				//			(nTopicToLoadFrom),
				//			(pLoadInfo->bLoadToTemplate?"":"AND EmrTemplateDetailsT.SourceActionID Is Null ")
				//
				// Despite the query, anything with a non-null source action ID cannot be a Live detail because it may be spawned
				// by CEMN::ProcessEmrActions. We've already united the details with their parent topics in m_mapDetailTopics, so
				// all we need to check for her is that the source action ID is -1.
				//
				CEMNDetail* pDetail = papTopicDetails->GetAt(j);
				if (-1 == pDetail->GetSourceActionID()) {
					// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
					CEMNDetail* pNewDetail = CEMNDetail::CreateDetail(NULL, "PreloadEmrTemplateTopics new live detail", FALSE);
					//pNewDetail->AddRef(); // (c.haag 2007-08-02 17:08) - PLID 26833 - Add a reference now, and then release it later in FinalizeDetailTransitionToEMN
					// (a.walling 2009-10-12 16:05) - PLID 36023
					pNewDetail->__AddRef("PreloadEmrTemplateTopics add to m_apLiveDetails");
					pNewDetail->CopyFrom(*papTopicDetails->GetAt(j)); // (a.walling 2012-10-31 17:17) - PLID 53550 - CEMNDetail::opreator= leads to very confusing semantics

					// (j.jones 2010-03-10 12:07) - PLID 37641 - This code is misleading, the detail may not really end up
					// being a live detail, for example if it is in a spawned topic. A new copy will be created later,
					// and this "live" detail won't really be used. But if you are creating a new EMN from a template,
					// this really will be the live detail. Search for PLID 37641 to get to the corresponding code
					// that does this. (This logic not actually caused by or altered by 37641, that item just corrects the
					// reference counts.)

					m_apLiveDetails.Add(pNewDetail);
					m_mapLiveDetailsByTemplateID[ pNewDetail->m_nEMRTemplateDetailID ] = pNewDetail;

					// (j.jones 2010-03-10 11:11) - PLID 37318 - perform a special reassign of smartstamp links,
					// based on a template detail ID lookup
					ReconfirmSmartStampLinks_ByLiveDetail(pNewDetail, NULL);
				}
			}
		} else {
			// This template topic has no details
		}
	}

	// (c.haag 2007-08-02 16:58) - PLID 26833 - By the time we get here, we've completely built our Live
	// detail array. While the contents of the live details my change, the array itself will not. Now we
	// need to go through the array updating the source detail pointers to point to Live details. If we do
	// not do this, we risk adding internal EMN loader details to the EMN by accident.
	const int nLiveDetails = m_apLiveDetails.GetSize();
	for (i=0; i < nLiveDetails; i++) {
		m_apLiveDetails[i]->SetSourceDetail( GetLiveDetail(m_apLiveDetails[i]->GetSourceDetail()) );
	}

	// Update the "Has Details" flag of our preloaded topic list.
	// (c.haag 2007-07-25 18:24) - PLID 25881 - We now do it such that it conforms to how it was
	// done in the 8300 scope. A topic has details if one of the following is true:
	//	1. The topic actually has details
	//	2. The topic has a detail whose parent is related by source action ID
	//
	// Here is the equivalent query:
	//
	//		CASE WHEN EXISTS (SELECT ID FROM EmrTemplateDetailsT WHERE 
	//		EmrTemplateDetailsT.EmrTemplateTopicID = EmrTemplateTopicsT.ID OR 
	//		EmrTemplateDetailsT.EmrTemplateTopicID = EmrTemplateTopicsT.SourceTemplateTopicID) THEN 1 ELSE 0 END AS HasDetails, 
	//
	pos = m_mapTemplateTopicsWithDetails.GetStartPosition();
	while (pos != NULL) {
		long nTemplateTopicID;
		BOOL bHasDetails = FALSE;
		m_mapTemplateTopicsWithDetails.GetNextAssoc( pos, nTemplateTopicID, bHasDetails );
		if (bHasDetails) {
			CPreloadedTemplateTopic* pTemplateTopic = m_mapAllTemplateTopicsByID[ nTemplateTopicID ];
			if (pTemplateTopic) {
				pTemplateTopic->m_bHasDetails = TRUE;
			} else {
				ASSERT(FALSE);
				ThrowNxException("CEMNLoader::PreloadEmrTemplateTopics detected a topic whose parent does not exist in the same template!");
			}
		}
	}
	// Up to this point, we satisifed requirement #1 in the comments above. Now we satisfy #2. 
	for (i=0; i < nTemplateTopics; i++) {
		CPreloadedTemplateTopic* pTemplateTopic = m_apAllTemplateTopics[i];
		if (-1 != VarLong(pTemplateTopic->m_vSourceTemplateTopicID, -1)) {
			CPreloadedTemplateTopic* pSourceTemplateTopic = m_mapAllTemplateTopicsByID[VarLong(pTemplateTopic->m_vSourceTemplateTopicID, -1)];
			if (NULL != pSourceTemplateTopic) {
				pTemplateTopic->m_bHasDetails = pSourceTemplateTopic->m_bHasDetails;
			} else {
				ASSERT(FALSE);
				ThrowNxException("CEMNLoader::PreloadEmrTemplateTopics detected a topic whose parent does not exist in the same template!");
			}
		}
	}
	// We're done with this member variable forever
	m_mapTemplateTopicsWithDetails.RemoveAll();

	// Now begin the loading process for root-level topics. This is where the old version of the query filter
	// in CEMN::LoadFromTemplateID for the rsTopics recordset comes into play. It was:
	//
	// 1. WHERE EmrTemplateTopicsT.TemplateID = %(nEmrTemplateID) 
	// 2. AND EmrTemplateTopicsT.EmrParentTemplateTopicID Is Null
	// 3. AND %(bLoadHiddenTopics?"1=1":"EmrTemplateTopicsT.SourceActionID Is Null")
	//
	// Where % signs signify C++ variables
	//
	// (c.haag 2007-05-31 12:52) - PLID 26175 - Make sure the caller wants to load root topics
	if (bLoadRootTopics) {

		CArray<CEMRTopic*,CEMRTopic*> apRootTopicsToLoad;
		CArray<CPreloadedTemplateTopic*,CPreloadedTemplateTopic*> apPreloadedRootTopics;

		// First, figure out which topics qualify as loadable root topics, and add them to an array. We
		// cannot call LoadFromPreloadedTemplateTopic inside this loop because the EMN *must* have all of
		// the topics in its memory before we try to load any. The EMN will not know if the load is done
		// until it knows that all of its owned topics have been loaded.
		for (i=0; i < nTemplateTopics; i++) {
			CPreloadedTemplateTopic* pTemplateTopic = m_apAllTemplateTopics[i];

			// 1. Skip this topic if it's not actually part of this template (it must be the source topic
			// of another topic)
			if (pTemplateTopic->m_nTemplateID != nEmrTemplateID)
				continue;

			// 2. Skip this topic if it's not a root level topic
			if (VT_NULL != pTemplateTopic->m_vEMRParentTemplateTopicID.vt)
				continue;

			// 3. Skip this topic based on our hidden topic flag
			if (!bLoadHiddenTopics) {
				if (VT_NULL != pTemplateTopic->m_vSourceActionID.vt) {
					continue;
				}
			}

			// We're good to go for a root topic load
			// (c.haag 2007-05-08 16:32) - PLID 25941 - We now maintain the CEMRTopic object in the
			// preloaded object so that it can be passed along to the main thread after the topic is
			// loaded
			// (c.haag 2007-08-09 12:12) - PLID 25941 - Do not use AddTopic because it flags the topic
			// as having been unsaved, which is inconsistent with the patch. Instead, use InsertTopic.
			// You'll notice that when we call InsertTopic, we set the initial load parameter to true.
			// This is because we should always be in the initial load when this code is called.
			//CEMRTopic* pNewTopic = pTemplateTopic->m_pTopic = m_pEMN->AddTopic(""); // The name will be populated in the next function
			//TES 4/15/2010 - PLID 24692 - If this is a template, we may already have a position entry, otherwise, create a new one.
			TopicPositionEntry *tpe = NULL;
			if(m_pEMN->IsTemplate()) {
				tpe = m_pEMN->GetTopicPositionEntryByID(pTemplateTopic->m_nID);
				if(tpe == NULL) {
					tpe = new TopicPositionEntry;
					//TES 5/3/2010 - PLID 24692 - Assign the ID.
					tpe->nTopicID = pTemplateTopic->m_nID;
				}
			}
			else {
				tpe = new TopicPositionEntry;
			}
			CEMRTopic* pNewTopic = pTemplateTopic->m_pTopic = new CEMRTopic(m_pEMN, tpe);
			//TES 10/5/2009 - PLID 35755 - We don't want to re-calculate topic orders.
			m_pEMN->InsertTopic(pNewTopic, NULL, TRUE, FALSE);

			apRootTopicsToLoad.Add(pNewTopic);
			apPreloadedRootTopics.Add(pTemplateTopic);
		}

		// Now go through our array and actually load them.
		const int nRootTopicsToLoad = apRootTopicsToLoad.GetSize();
		for (i=0; i < nRootTopicsToLoad; i++) {
			CEMRTopic* pNewTopic = apRootTopicsToLoad[i];
			CPreloadedTemplateTopic* pTemplateTopic = apPreloadedRootTopics[i];

			//DRT 9/25/2007 - PLID 27515 - Pass down the extra SourceAction parameters as well
			// (z.manning 2009-02-13 10:13) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
			pNewTopic->LoadFromPreloadedTemplateTopic(pCon, this, pTemplateTopic->m_nID, FALSE,
				hWnd, sai, bLoadHiddenTopics, nPatientID, pEmr, TRUE, nSourceActionSourceID, nSourceActionSourceDataGroupID, nSourceActionSourceHotSpotGroupID, nSourceActionSourceTableDropdownGroupID);
		}
	}
}

// (c.haag 2011-03-17) - PLID 42895 - Loads EMR Info Common Lists
void CEMNLoader::PreloadEmrInfoCommonLists(ADODB::_RecordsetPtr rsCommonLists)
{
	FieldsPtr f = rsCommonLists->Fields;
	CEMRInfoItem* pInfoItem = NULL;

	// Do for all lists
	// We have to use MoveFirst here because of how CEMR::LoadFromID works. It actually pulls all the items for
	// all the EMN's for the same EMR in one big shot. It also pulls all the common list items in big shot. That's great;
	// but unlike CEMN::LoadFromEMNRecordsets, a loop is run for every EMN where a new CEMNLoader is generated
	// and records for that EMN are read in. Remember, we only loaded the common lists once...so once we read them
	// into the first EMN, the recordset is at EOF. Once we read the second, third, fourth EMN's...we need to go back 
	// to BOF to read the common lists over again.
	if (!(rsCommonLists->bof && rsCommonLists->eof)) {
		rsCommonLists->MoveFirst();
	}
	while (!rsCommonLists->eof)
	{
		const long nEmrInfoID = AdoFldLong(f, "EmrInfoID");
		CEmrInfoCommonListCollection* p = NULL;

		// Look up the collection to put this list in
		if (!m_mapCommonLists.Lookup(nEmrInfoID, p)) {
			p = new CEmrInfoCommonListCollection();
			m_mapCommonLists.SetAt(nEmrInfoID, p);
		}

		// Now add the list to the info item
		p->LoadList(f);

		// Go to the next list
		rsCommonLists->MoveNext();
	}
}

// (c.haag 2011-03-17) - PLID 42895 - Loads EMR Info Common List Items
void CEMNLoader::PreloadEmrInfoCommonListItems(ADODB::_RecordsetPtr rsCommonListItems)
{
	FieldsPtr f = rsCommonListItems->Fields;
	CEMRInfoItem* pInfoItem = NULL;

	// Do for all list items
	// We have to use MoveFirst here because of how CEMR::LoadFromID works. It actually pulls all the items for
	// all the EMN's for the same EMR in one big shot. It also pulls all the common list items in big shot. That's great;
	// but unlike CEMN::LoadFromEMNRecordsets, a loop is run for every EMN where a new CEMNLoader is generated
	// and records for that EMN are read in. Remember, we only loaded the common lists once...so once we read them
	// into the first EMN, the recordset is at EOF. Once we read the second, third, fourth EMN's...we need to go back 
	// to BOF to read the common lists over again. 
	if (!(rsCommonListItems->bof && rsCommonListItems->eof)) {
		rsCommonListItems->MoveFirst();
	}
	while (!rsCommonListItems->eof)
	{
		const long nEmrInfoID = AdoFldLong(f, "EmrInfoID");
		CEmrInfoCommonListCollection* p = NULL;

		// Look up the collection to put this list in
		if (!m_mapCommonLists.Lookup(nEmrInfoID, p)) {
			ThrowNxException("CEMNLoader::PreloadEmrInfoCommonListItems was called with invalid EmrInfoID %d!", nEmrInfoID);
		}

		// Now load in the item. We fully expect the fields list to have EmrDataT.Data in it.
		p->LoadListItem(f, TRUE);

		// Go to the next list
		rsCommonListItems->MoveNext();
	}
}

// (c.haag 2008-07-18 16:05) - PLID 30784 - Returns a map given an Emr Problem regarding type enumeration
CEmnLoaderProblemLinkMap* CEMNLoader::GetEmrProblemLinkMap(EMRProblemRegardingTypes type)
{
	if (!m_mtxEMRProblems.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to EMR problems before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetEmrProblemLinkMap was called without exclusive ownership to EMR problems");
	}
	switch (type)
	{
	case eprtEmrItem: return &m_mapEmrItemProblems; break;
	case eprtEmrDataItem: return &m_mapEmrDataItemProblems; break;
	case eprtEmrTopic: return &m_mapEmrTopicProblems; break;
	case eprtEmrEMN: return &m_mapEMNProblems; break;
	case eprtEmrEMR: return &m_mapEMRProblems; break;
	case eprtEmrDiag: return &m_mapEmrDiagProblems; break;
	case eprtEmrCharge: return &m_mapEmrChargeProblems; break;
	case eprtEmrMedication: return &m_mapEmrMedicationProblems; break;
	default:
		// If you hit this assertion, you need to add support for this type
		ASSERT(FALSE);
		return NULL;
	}
}

// (c.haag 2008-07-18 16:11) - PLID 30784 - Clears an EMR problem map
// (c.haag 2009-05-16 12:09) - PLID 34277 - Clears problem links instead of problems
void CEMNLoader::ClearEmrProblemMap(CEmnLoaderProblemLinkMap* pMap)
{
	if (NULL != pMap) {
		POSITION pos = pMap->GetStartPosition();
		while (pos != NULL) {
			CEmrProblemLinkAry* pAry;
			long nRegardingID;
			pMap->GetNextAssoc( pos, nRegardingID, pAry );
			if (pAry) delete pAry;
		}
		pMap->RemoveAll();
	}
}

// (c.haag 2008-07-18 15:39) - PLID 30784 - Loads all problems in the span of the EMN
// (including EMR-level problems)
// (c.haag 2009-05-19 10:11) - PLID 34277 - We now require an EMR object
void CEMNLoader::PreloadEmrProblems(ADODB::_RecordsetPtr& rsProblems, CEMR* pParentEMR)
{
	CHoldEMNLoaderMutex mh(&m_mtxEMRProblems);
	FieldsPtr f = rsProblems->Fields;
	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);

	// (c.haag 2009-05-16 11:19) - PLID 34277 - Traverse through the recordset, which
	// has one entry for every record in the Emr Problem Link table.
	//
	// The purpose of this loop is to populate all the EMR problem link maps; where we have
	// a unique map for each problem regarding type, and in each map, the key is the problem
	// regarding ID. However, we also have to load the per-problem information as well.
	// Problems are stored in the parent EMR object. Reference counting ensures that problems
	// only exist for as long as they need to. For loading purposes, we'll maintain a map of 
	// loaded problems.
	//
	CMap<long,long,CEmrProblem*,CEmrProblem*> mapLoadedProblems;
	ASSERT(NULL != pParentEMR);

	while (!rsProblems->eof && AdoFldLong(rsProblems, "EMRID") == m_nMasterID) {
		EMRProblemRegardingTypes RegardingType = (EMRProblemRegardingTypes)AdoFldLong(f, "EmrRegardingType");
		const long nEMRProblemLinkID = AdoFldLong(f, "EmrProblemLinkID");
		const long nRegardingID = AdoFldLong(f, "EmrRegardingID");
		const long nEMRProblemID = AdoFldLong(f, "ID");
		const long nEMRDataID = AdoFldLong(f, "EMRDataID", -1);
		CEmrProblem* pProblem = NULL;
		if (!mapLoadedProblems.Lookup(nEMRProblemID, pProblem)) {
			pProblem = pParentEMR->AllocateEmrProblem(f);
			mapLoadedProblems.SetAt(nEMRProblemID, pProblem);
		} else {
			// The problem itself was already loaded. If we get here, it means
			// this problem is linked with multiple items.
		}
		// (c.haag 2009-05-16 11:27) - PLID 34277 - Now create the problem link item. This will
		// add a reference to the problem.
		CEmrProblemLink pl(pProblem, nEMRProblemLinkID, RegardingType, nRegardingID, nEMRDataID);

		// (c.haag 2009-05-16 11:30) - PLID 34277 - Now add the link item to the proper map
		CEmnLoaderProblemLinkMap* pMap = GetEmrProblemLinkMap(RegardingType);
		CEmrProblemLinkAry* pary = NULL;

		// Add this problem to the proper EMR problem map
		if (NULL != pMap) {

			// Get the array. Create it if it doesn't exist
			pMap->Lookup(nRegardingID, pary);
			if (NULL == pary) {
				pary = new CEmrProblemLinkAry;
				pMap->SetAt(nRegardingID, pary);
			}
			// Add the element to the array
			if (NULL != pary) {
				pary->Add(pl);
			} else {
				ASSERT(FALSE); // This should never happen
			}

		} else {
			// The EMR problem regarding type is not supported in this loader
		}

		rsProblems->MoveNext();
	}

	// (c.haag 2009-05-16 11:32) - PLID 34277 - At this point, all the CEMRProblem objects
	// we created have one reference for the parent EMR object in memory, one reference for
	// every problem link item, and one reference which belongs to the map. Since the map is
	// being destroyed, we must dereference every problem once.
	POSITION pos = mapLoadedProblems.GetStartPosition();
	while (pos != NULL) {
		long nEMRProblemID;
		CEmrProblem* pProblem;
		mapLoadedProblems.GetNextAssoc( pos, nEMRProblemID, pProblem );
		// Should have at least three references before the call to
		// Release that follows a few lines down:
		// 1. The map where the problem is stored (this is the one we're releasing)
		// 2. One or more problem link objects in memory that the problem is linked to
		// 3. The CEMR object which manages problem allocations
		ASSERT(pProblem->GetRefCnt() >= 3);
		pProblem->Release();
	}
}

// (c.haag 2008-07-18 16:17) - PLID 30784 - Given a regarding type and ID, this function
// returns a list of problems for a given EMR object
// (c.haag 2009-05-16 11:19) - PLID 34277 - We now return an array of EMR problem links, not problems
CEmrProblemLinkAry* CEMNLoader::GetEmrProblemLinks(EMRProblemRegardingTypes RegardingType, long nRegardingID)
{
	CEmnLoaderProblemLinkMap* pMap = GetEmrProblemLinkMap(RegardingType);
	if (NULL != pMap) {
		CEmrProblemLinkAry* pary = NULL;
		if (pMap->Lookup(nRegardingID, pary)) {
			// Found problems
			return pary;
		} else {
			// This regarding ID has no problems
			return NULL;
		}
	} else {
		// Not supported
		return NULL;
	}
}

// (z.manning 2010-02-17 15:39) - PLID 37412
void CEMNLoader::PreloadDetailImageStamps(ADODB::_RecordsetPtr& rsDetailImageStamps)
{
	CHoldEMNLoaderMutex mhDetails(&m_mtxInternalDetails);

	FieldsPtr f = rsDetailImageStamps->Fields;

	CEmrDetailImageStampArray arypDetailStamps;
	// (z.manning 2010-03-03 08:22) - PLID 37412 - We may be loading multiple EMNs so make sure we filter
	// by EmrMasterT.ID (the detail stamps are ordered by EmrID).
	for(; !rsDetailImageStamps->eof && AdoFldLong(f,"EmrID") == m_nMasterID; rsDetailImageStamps->MoveNext()) {
		const long nDetailID = AdoFldLong(f, "EmrDetailID");
		CPreloadedDetail *pPreloadedDetail = GetPreloadedDetailByID(nDetailID);
		if(pPreloadedDetail != NULL) {
			pPreloadedDetail->m_arypDetailImageStamps.Add(new EmrDetailImageStamp(f));
		}
	}
}

// (c.haag 2011-03-18) - PLID 42895 - Returns a collection of EMR Info common lists
CEmrInfoCommonListCollection* CEMNLoader::GetEmrInfoCommonLists(long nEmrInfoID)
{
	CEmrInfoCommonListCollection* p = NULL;
	m_mapCommonLists.Lookup(nEmrInfoID, p);
	return p;
}

void CEMNLoader::SetManagingEMNDetailArray(BOOL bManage)
{
	// (c.haag 2007-08-10 17:53) - PLID 27049 - Changes m_bManagingEMNDetailArray
	// to TRUE if the EMN loader is responsible for managing detail iterating from
	// the EMN, or FALSE if the EMN is responsible for traversing its own topics to
	// iterate through all details.
	m_bManagingEMNDetailArray = bManage;
}

BOOL CEMNLoader::IsManagingEMNDetailArray() const
{
	// (c.haag 2007-08-10 17:54) - PLID 27049 - Returns TRUE if the EMN loader is 
	// responsible for managing detail iterating from the EMN, or FALSE if the EMN
	// is responsible for traversing its own topics to iterate through all details.
	return m_bManagingEMNDetailArray;
}

//////////////////////////////////////////////////////////////////////
// Master topic level load function called from a worker thread
//////////////////////////////////////////////////////////////////////

void CEMNLoader::LoadEMRTopic(EMRTopicLoadInfo *pLoadInfo, _Connection *lpCon, CEMR *pParentEmr)
{
	EMRLOGINDENT(1,"CEMNLoader::LoadEMRTopic called for topic ID %s", (pLoadInfo) ? AsString(pLoadInfo->nID) : "(null)"); // (c.haag 2010-05-19 9:04) - PLID 38759
	// (c.haag 2007-06-13 10:01) - Don't put a try-catch here...per legacy code, there is a catch
	// outside this function
	//try {
	//
	// (c.haag 2007-04-24 08:26) - PLID 25881, 25853 - This function loads the content of a single 
	// topic. This is an overload of the global version of LoadEMRTopic in EMR.cpp which we should 
	// attempt to depreciate.
	//
	// (c.haag 2007-07-19 09:29) - PLID 26744 - This function has been instrumented with catch-throws
	// for more precise exception handling (meaning easier debugging)
	//
	_ConnectionPtr pConn;
	try {
		pConn = lpCon;
		if (NULL == pLoadInfo) {
			ThrowNxException("The pLoadInfo value is invalid!"); 
		}
	} NxCatchAllThrowThread("Error in CEMNLoader::LoadEMRTopic() - Initialization");

	//The topic structure is much like the old tab structure but can either have
	//details or other topics underneath it. So to load a topic, we need to use
	//this CEMRTopic::LoadFromID() function recursively

	//load this topic's "topic-level" data
	// (c.haag 2007-05-03 10:51) - PLID 25881 - This query has been depreciated. We now pull template topic data in
	// PreloadEmrTemplateTopics

	// (c.haag 2007-05-01 12:34) - PLID 25853 - We used to handle both topics and template topics in this body of code.
	// Now we just handle template topics.
	if(pLoadInfo->bLoadFromTemplate) {
		CPreloadedTemplateTopic* pTemplateTopic;
		try {
			m_mtxTopics.Lock();
			// (c.haag 2007-07-03 09:30) - PLID 26523 - Take exclusive ownership of topics until this code section is done.
			pTemplateTopic = GetPreloadedTemplateTopicByID(pLoadInfo->nID);
			if (NULL == pTemplateTopic) ThrowNxException("Attempted to load template topic %d that was not in memory!", pLoadInfo->nID);
			const long nSourceTemplateTopicID = VarLong(pTemplateTopic->m_vSourceTemplateTopicID, -1);

			PreloadedTopicAction pta;
			BOOL bHasActionData = m_mapEmrTemplateTopicActions.Lookup(pLoadInfo->m_saiOverride.nSourceActionID, pta);

			if(!pLoadInfo->bIsNewTopic) {
				pLoadInfo->m_nOriginalTemplateTopicID = nSourceTemplateTopicID; //AdoFldLong(rsTopic, "SourceTemplateTopicID", -1);
				// (c.haag 2007-07-23 17:41) - PLID 26344 - This is assigned if this topic
				// was spawned onto its template by a list detail
				pLoadInfo->m_nOriginalTemplateID = VarLong(pTemplateTopic->m_vOriginalTemplateID, -1);
			}
			else {
				pLoadInfo->m_nOriginalTemplateTopicID = pLoadInfo->nID; // this is the topic id
				pLoadInfo->m_nOriginalTemplateID = pLoadInfo->m_nTemplateID; // set the original template to the one that that id is on
			}

			if(pLoadInfo->m_saiOverride.nSourceActionID != -1) {
				pLoadInfo->m_sai = pLoadInfo->m_saiOverride;
				//DRT 9/25/2007 - PLID 27515 - Added SourceAction SourceID and SourceDataGroupID
				pLoadInfo->m_nSourceActionSourceID = pLoadInfo->nOverrideSourceActionSourceID;
				pLoadInfo->m_nSourceActionSourceDataGroupID = pLoadInfo->nOverrideSourceActionSourceDataGroupID;
				pLoadInfo->m_nSourceActionSourceHotSpotGroupID = pLoadInfo->nOverrideSourceActionSourceHotSpotGroupID;
				// (z.manning 2009-02-13 10:13) - PLID 33070 - SourceActionSourceTableDropdownGroupID
				pLoadInfo->m_nSourceActionSourceTableDropdownGroupID = pLoadInfo->nOverrideSourceActionSourceTableDropdownGroupID;
			}
			else {
				//TES 3/17/2010 - PLID 37530 - Pass in SourceStampID and SourceStampIndex
				pLoadInfo->m_sai = SourceActionInfo((EmrActionObject)VarLong(pTemplateTopic->m_vSourceActionType,eaoInvalid), VarLong(pTemplateTopic->m_vSourceActionID,-1), -1, &TableRow(VarLong(pTemplateTopic->m_vSourceDataGroupID,-1), -1, VarLong(pTemplateTopic->m_vSourceStampID,-1), VarLong(pTemplateTopic->m_vSourceStampIndex,-1))); //AdoFldLong(rsTopic, "SourceActionID", -1);
				//DRT 9/25/2007 - PLID 27515 - Added SourceAction SourceID and SourceDataGroupID
				pLoadInfo->m_nSourceActionSourceID = VarLong(pTemplateTopic->m_vSourceActionSourceID, -1);
				pLoadInfo->m_nSourceActionSourceDataGroupID = VarLong(pTemplateTopic->m_vSourceActionSourceDataGroupID, -1);
				pLoadInfo->m_nSourceActionSourceHotSpotGroupID = VarLong(pTemplateTopic->m_vSourceActionSourceHotSpotGroupID, -1);
				// (z.manning 2009-02-13 10:14) - PLID 33070 - SourceActionSourceTableDropdownGroupID
				pLoadInfo->m_nSourceActionSourceTableDropdownGroupID = VarLong(pTemplateTopic->m_vSourceActionSourceTableDropdownGroupID, -1);
			}

			// (j.jones 2007-01-12 09:02) - PLID 24027 - supported SourceDetailID
			// this is different from SourceActionID in that SourceActionID would be the same
			// whether on a template or on a patient EMN, but SourceDetailID would be different
			// on a template than on an EMN
			if(!pLoadInfo->bIsNewTopic && pLoadInfo->bLoadToTemplate) {
				pLoadInfo->m_sai.nSourceDetailID = VarLong(pTemplateTopic->m_vSourceDetailID, -1); //AdoFldLong(rsTopic, "SourceDetailID", -1);
			}
			else {
				pLoadInfo->m_sai.nSourceDetailID = pLoadInfo->m_saiOverride.nSourceDetailID;
			}
					
			// (c.haag 2007-08-02 17:03) - PLID 26833 - Make sure we are giving back the Live source detail by
			// using GetLiveDetail; otherwise an internal detail could escape to the outside!
			pLoadInfo->m_sai.pSourceDetail = GetLiveDetail(pLoadInfo->m_saiOverride.pSourceDetail);

			pLoadInfo->m_strSourceActionName = (bHasActionData) ? VarString(pta.m_vEmrActionName, "") : ""; //AdoFldString(rsTopic, "SourceActionName", "");

			pLoadInfo->m_SourceActionDestType = (EmrActionObject)((bHasActionData) ? VarLong(pta.m_vEmrActionDestType, -1) : -1);//(EmrActionObject)AdoFldLong(rsTopic, "SourceActionDestType", -1);
					
			CString strName = VarString(pTemplateTopic->m_vName, ""); //AdoFldString(rsTopic, "Name","");
			BOOL bShowIfEmpty = VarBool(pTemplateTopic->m_vShowIfEmpty); //AdoFldBool(rsTopic, "ShowIfEmpty");
			if(pLoadInfo->m_nOriginalTemplateTopicID != -1 && pLoadInfo->m_sai.nSourceActionID != -1) {
				//We're loading from a template topic, which was spawned.  The Name and ShowIfEmpty values need to come from
				//the original topic, if possible.
				// (c.haag 2007-07-23 17:41) - PLID 26344 - These are assigned if this topic
				// was spawned onto its template by a list detail
				strName = VarString(pTemplateTopic->m_vOriginalTopicName, strName);
				bShowIfEmpty = VarBool(pTemplateTopic->m_vOriginalShowIfEmpty, bShowIfEmpty);
			}
			pLoadInfo->m_strName = strName;

#ifdef REPORT_LOAD_PROGRESS
			TRACE("Loading topic %s (%d)...\n", strName, pLoadInfo->nID);
#endif

			pLoadInfo->m_bShowIfEmpty = bShowIfEmpty;
					
			pLoadInfo->m_nTemplateTopicID = pTemplateTopic->m_nID; //AdoFldLong(rsTopic, "EMRTemplateTopicID",-1);
					
			pLoadInfo->m_nTemplateID = pTemplateTopic->m_nTemplateID; //AdoFldLong(rsTopic, "TemplateID", -1);
					
			pLoadInfo->m_nTemplateTopicOrderIndex = VarLong(pTemplateTopic->m_vOrderIndex, -1); //AdoFldLong(rsTopic, "TemplateOrderIndex", -1);
					
			if(pLoadInfo->bLoadToTemplate) {
				pLoadInfo->m_bHideOnEMN = VarBool(pTemplateTopic->m_vHideOnEMN, FALSE);//AdoFldBool(rsTopic, "HideOnEMN", FALSE);
			}
			else {
				pLoadInfo->m_bHideOnEMN = FALSE;
			}

			// (a.walling 2008-06-30 12:46) - PLID 29271 - Preview Pane flags
			pLoadInfo->m_nPreviewFlags = VarLong(pTemplateTopic->m_vPreviewFlags, 0);
		} NxCatchAllCallThrowThread("Error in CEMNLoader::LoadEMRTopic() - Reading template topic data", m_mtxTopics.Unlock());

		//now load the subtopics for this topic, which in turn will load its subtopics or EMNDetails
		//
		// (c.haag 2007-05-03 08:22) - PLID 25881 - Now begin the process for loading subtopics. I'm leaving
		// the original query here commented out for reference:
		//
		//  rsSubTopics = CreateRecordset(pConn, "SELECT ID, SourceActionID, SourceDetailID FROM EMRTemplateTopicsT "
		//	"WHERE EMRParentTemplateTopicID = %li AND %s AND %s ORDER BY OrderIndex", 
		//	pLoadInfo->nID, strSpawned, strHide);
		// 
		// No need to worry about sorting. The preload query sorts by OrderIndex, and everything is inserted
		// into m_apAllTemplateTopics and pTemplateTopic->m_anSubTopicIDs in order of OrderIndex.
		//
		// (c.haag 2007-05-03 09:50) - PLID 25881 - The following code is depreciated

		try {
			// (c.haag 2007-08-08 11:24) - PLID 27014 - Skip this code if we don't want to load subtopics
			if (pLoadInfo->bLoadSubTopics) {
				const int nSubTopics = pTemplateTopic->m_anSubTemplateTopicIDs.GetSize();
				for (int i=0; i < nSubTopics; i++) {
					CPreloadedTemplateTopic* pPreloadedSubTopic = GetPreloadedTemplateTopicByID(pTemplateTopic->m_anSubTemplateTopicIDs[i]);
					if (NULL == pPreloadedSubTopic) ThrowNxException("Attempted to load template subtopic %d that was not in memory!", pTemplateTopic->m_anSubTemplateTopicIDs[i]);

					// (c.haag 2007-05-03 09:32) - Begin the filtering process
					//
					// 1. WHERE EMRParentTemplateTopicID = %(pLoadInfoID)
					// 2. AND %(strSpawned)
					// 3. AND %(strHide)
					//
					// Where % signs signify C++ variables

					// 1. (This has already been done by virtue of how we iterate through this loop)

					// 2. Skip this topic if we are not loading hidden topics, and the source action ID is not null
					// or is not pLoadInfo->nOverrideSourceActionID if it's valid
					if (!pLoadInfo->bLoadHiddenTopics) {
						if (pLoadInfo->m_saiOverride.nSourceActionID != -1) {
							if (VT_NULL != pPreloadedSubTopic->m_vSourceActionID.vt && VarLong(pPreloadedSubTopic->m_vSourceActionID) != pLoadInfo->m_saiOverride.nSourceActionID) {
								continue;
							}
						} else {
							if (VT_NULL != pPreloadedSubTopic->m_vSourceActionID.vt) {
								continue;
							}
						}

						// (a.walling 2007-11-27 13:23) - PLID 28194 - Need to filter on SourceDetail as well as SourceAction
						if (pLoadInfo->m_saiOverride.nSourceDetailID != -1) {
							// SourceDetailID IS NULL OR SourceDetailID = %nOverrideSourceDetailID
							if (VT_NULL != pPreloadedSubTopic->m_vSourceDetailID.vt && VarLong(pPreloadedSubTopic->m_vSourceDetailID) != pLoadInfo->m_saiOverride.nSourceDetailID) {
								continue;
							}
						} else {
							// SourceDetailID IS NULL
							if (VT_NULL != pPreloadedSubTopic->m_vSourceDetailID.vt) {
								continue;
							}
						}
					}

					// 3. Skip this topic if we're loading to a patient chart and HideOnEMN is true
					if (!pLoadInfo->bLoadToTemplate && VarBool(pPreloadedSubTopic->m_vHideOnEMN)) {
						continue;
					}
					
					//Load this topic.
					// (c.haag 2007-05-08 16:29) - PLID 25941 - We now maintain the CEMRTopic object in the
					// preloaded object so that it can be passed along to the main thread after the subtopic is
					// loaded
					//TES 4/15/2010 - PLID 24692 - If this is a template, we may already have a position entry, otherwise, create a new one.
					TopicPositionEntry *tpe = NULL;
					if(m_pEMN->IsTemplate()) {
						tpe = m_pEMN->GetTopicPositionEntryByID(pPreloadedSubTopic->m_nID);
						if(tpe == NULL) {
							tpe = new TopicPositionEntry;
							//TES 5/3/2010 - PLID 24692 - Assign the ID.
							tpe->nTopicID = pPreloadedSubTopic->m_nID;
						}
					}
					else {
						tpe = new TopicPositionEntry;
					}
					CEMRTopic *pTopic = pPreloadedSubTopic->m_pTopic = new CEMRTopic(pLoadInfo->bLoadToTemplate, FALSE, tpe);
					long nSourceActionID = -1;
					long nSourceDataGroupID = -1;
					long nSourceDetailImageStampID = -1;
					EmrActionObject eaoActionType = eaoInvalid;
					long nSourceDetailID = -1;
					//DRT 9/27/2007 - PLID 27515 - Added SourceAction SourceID and SourceDataGroupID
					// (z.manning 2009-02-13 10:15) - PLID 33070 - SourceActionSourceTableDropdownGroupID
					//TES 3/17/2010 - PLID 37530 - SourceStampID and SourceStampIndex
					long nSourceActionSourceID = -1, nSourceActionSourceDataGroupID = -1, nSourceActionSourceHotSpotGroupID = -1, nSourceActionSourceTableDropdownGroupID = -1,
						nSourceStampID = -1, nSourceStampIndex = -1;
					if(pLoadInfo->bIsNewTopic && pLoadInfo->m_saiOverride.nSourceActionID != -1) {
						nSourceActionID = pLoadInfo->m_saiOverride.nSourceActionID;
						nSourceDataGroupID = pLoadInfo->m_saiOverride.GetDataGroupID();
						nSourceDetailImageStampID = pLoadInfo->m_saiOverride.GetDetailStampID();
						nSourceStampID = pLoadInfo->m_saiOverride.GetStampID();
						nSourceStampIndex = pLoadInfo->m_saiOverride.GetStampIndexInDetailByType();
						eaoActionType = pLoadInfo->m_saiOverride.eaoSourceType;
						nSourceActionSourceID = pLoadInfo->nOverrideSourceActionSourceID;
						nSourceActionSourceDataGroupID = pLoadInfo->nOverrideSourceActionSourceDataGroupID;
						nSourceActionSourceHotSpotGroupID = pLoadInfo->nOverrideSourceActionSourceHotSpotGroupID;
						nSourceActionSourceTableDropdownGroupID = pLoadInfo->nOverrideSourceActionSourceTableDropdownGroupID;
					}
					else {
						nSourceActionID = VarLong(pPreloadedSubTopic->m_vSourceActionID, -1);//  AdoFldLong(rsSubTopics, "SourceActionID",-1);
						nSourceDataGroupID = VarLong(pPreloadedSubTopic->m_vSourceDataGroupID, -1);
						eaoActionType = (EmrActionObject)VarLong(pPreloadedSubTopic->m_vSourceActionType, eaoInvalid);
						nSourceActionSourceID = VarLong(pPreloadedSubTopic->m_vSourceActionSourceID, -1);
						nSourceActionSourceDataGroupID = VarLong(pPreloadedSubTopic->m_vSourceActionSourceDataGroupID, -1);
						nSourceActionSourceHotSpotGroupID = VarLong(pPreloadedSubTopic->m_vSourceActionSourceHotSpotGroupID, -1);
						nSourceActionSourceTableDropdownGroupID = VarLong(pPreloadedSubTopic->m_vSourceActionSourceTableDropdownGroupID, -1);
					}

					// (j.jones 2007-01-22 17:16) - PLID 24366 - use the override for
					// source detail if loading to a patient EMN
					if((pLoadInfo->bIsNewTopic || !pLoadInfo->bLoadToTemplate)
						&& pLoadInfo->m_saiOverride.nSourceDetailID != -1) {
						nSourceDetailID = pLoadInfo->m_saiOverride.nSourceDetailID;
					}
					else {
						nSourceDetailID = VarLong(pPreloadedSubTopic->m_vSourceDetailID, -1); //AdoFldLong(rsSubTopics, "SourceDetailID",-1);
					}

					// (j.jones 2007-01-22 17:16) - PLID 24366 - use the override for
					// source detail if loading to a patient EMN
					CEMNDetail *pSourceDetail = NULL;
					SourceActionInfo sai;
					if((pLoadInfo->bIsNewTopic || !pLoadInfo->bLoadToTemplate)
						&& pLoadInfo->m_saiOverride.pSourceDetail) {

						// (c.haag 2007-08-02 17:03) - PLID 26833 - Make sure we are giving back the Live source detail by
						// using GetLiveDetail; otherwise an internal detail could escape to the outside!
						pSourceDetail = GetLiveDetail(pLoadInfo->m_saiOverride.pSourceDetail);

						// (j.jones 2007-01-22 17:31) - PLID 24027 - when this happens,
						// ensure our source detail ID is not the template detail ID
						nSourceDetailID = (pLoadInfo->bLoadToTemplate ? pLoadInfo->m_saiOverride.pSourceDetail->m_nEMRTemplateDetailID : pLoadInfo->m_saiOverride.pSourceDetail->m_nEMRDetailID);
						//TES 3/31/2010 - PLID 38002 - Construct the SourceActionInfo
						sai = SourceActionInfo(eaoActionType, nSourceActionID, nSourceDetailID, &TableRow(nSourceDataGroupID, nSourceDetailImageStampID, nSourceStampID, nSourceStampIndex));
					}
					else {
						//TES 3/31/2010 - PLID 38002 - We still want to reflect the fact that we have a source detail, so use 
						// the SourceActionInfo constructor that takes an ID instead of a pointer.
						sai = SourceActionInfo(eaoActionType, nSourceActionID, nSourceDetailID, &TableRow(nSourceDataGroupID,nSourceDetailImageStampID, nSourceStampID, nSourceStampIndex));
					}

					//pTopic->LoadFromTemplateTopicID(AdoFldLong(rsSubTopics, "ID"), pLoadInfo->bIsNewTopic, pLoadInfo->hWnd, nSourceActionID, nSourceDetailID, pSourceDetail, pLoadInfo->bLoadHiddenTopics, pLoadInfo->nPatientID, pConn, pParentEmr, this);
					//DRT 9/25/2007 - PLID 27515 - Added extra source action parameters
					// (z.manning 2009-02-13 10:16) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
					//TES 3/17/2010 - PLID 37530 - Added SourceStampID and SourceStampIndex
					//TES 3/31/2010 - PLID 38002 - Moved the SourceActionInfo contruction into the branch just above here.
					//SourceActionInfo sai(eaoActionType, nSourceActionID, pSourceDetail, &TableRow(nSourceDataGroupID,nSourceDetailImageStampID, nSourceStampID, nSourceStampIndex));
					pTopic->LoadFromPreloadedTemplateTopic(pConn, this, pPreloadedSubTopic->m_nID, pLoadInfo->bIsNewTopic, pLoadInfo->hWnd, sai, pLoadInfo->bLoadHiddenTopics, pLoadInfo->nPatientID, pParentEmr, TRUE, nSourceActionSourceID, nSourceActionSourceDataGroupID, nSourceActionSourceHotSpotGroupID, nSourceActionSourceTableDropdownGroupID);
					pLoadInfo->m_aryEMRTopics.Add(pTopic);
				}		
			}
			m_mtxTopics.Unlock();
		} NxCatchAllCallThrowThread("Error in CEMNLoader::LoadEMRTopic() - Reading template subtopic data", m_mtxTopics.Unlock(););

	} // if(pLoadInfo->bLoadFromTemplate) {

	// (c.haag 2007-05-01 12:35) - PLID 25853 - Everything in this conditional branch is much like the
	// old non-template version of the code in the above branch; except we do special handling with
	// preloaded topic data
	else {
		// (c.haag 2007-04-30 17:04) - PLID 25853 - Load in topic and sub-topic information
		// from memory
		// (c.haag 2007-07-03 09:30) - PLID 26523 - Take exclusive ownership of topics until this code section is done.
		CPreloadedTopic* pPreloadedTopic;
		try {
			m_mtxTopics.Lock();
			pPreloadedTopic = m_mapAllTopicsByID[ pLoadInfo->nID ];			
			if (NULL == pPreloadedTopic) {
				CString strError;
				strError.Format("CEMNLoader:LoadEMRTopic failed to find detail %d for loading!", pLoadInfo->nID );
				ThrowNxException(strError);
			}
			// (a.walling 2007-03-21 15:54) - PLID 25301
			// this used to just set -1, but that was causing issues.
			pLoadInfo->m_nOriginalTemplateTopicID = VarLong( pPreloadedTopic->m_vSourceTemplateTopicID, -1 );
			pLoadInfo->m_nOriginalTemplateID = VarLong(pPreloadedTopic->m_vSourceTemplateID, -1 );
			if(pLoadInfo->m_saiOverride.nSourceActionID != -1) {
				pLoadInfo->m_sai = pLoadInfo->m_saiOverride;
				//DRT 9/25/2007 - PLID 27515 - Added SourceAction SourceID and SourceDataGroupID
				pLoadInfo->m_nSourceActionSourceID = pLoadInfo->nOverrideSourceActionSourceID;
				pLoadInfo->m_nSourceActionSourceDataGroupID = pLoadInfo->nOverrideSourceActionSourceDataGroupID;
				pLoadInfo->m_nSourceActionSourceHotSpotGroupID = pLoadInfo->nOverrideSourceActionSourceHotSpotGroupID;
				// (z.manning 2009-02-13 10:16) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
				pLoadInfo->m_nSourceActionSourceTableDropdownGroupID = pLoadInfo->nOverrideSourceActionSourceTableDropdownGroupID;
			}
			else {
				//TES 3/17/2010 - PLID 37530 - Pass in SourceStampID and SourceStampIndex
				TableRow tr(VarLong(pPreloadedTopic->m_vSourceDataGroupID,-1), VarLong(pPreloadedTopic->m_vSourceDetailImageStampID,-1),
					VarLong(pPreloadedTopic->m_vSourceStampID,-1), VarLong(pPreloadedTopic->m_vSourceStampIndex,-1));
				pLoadInfo->m_sai = SourceActionInfo((EmrActionObject)VarLong(pPreloadedTopic->m_vSourceActionType,eaoInvalid), VarLong(pPreloadedTopic->m_vSourceActionID,-1), -1, &tr);
				//DRT 9/25/2007 - PLID 27515 - Added SourceAction SourceID and SourceDataGroupID
				pLoadInfo->m_nSourceActionSourceID = VarLong(pPreloadedTopic->m_vSourceActionSourceID, -1);
				pLoadInfo->m_nSourceActionSourceDataGroupID = VarLong(pPreloadedTopic->m_vSourceActionSourceDataGroupID, -1);
				pLoadInfo->m_nSourceActionSourceHotSpotGroupID = VarLong(pPreloadedTopic->m_vSourceActionSourceHotSpotGroupID, -1);
				// (z.manning 2009-02-13 10:17) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
				pLoadInfo->m_nSourceActionSourceTableDropdownGroupID = VarLong(pPreloadedTopic->m_vSourceActionSourceTableDropdownGroupID, -1);
			}
			// (j.jones 2007-01-12 09:02) - PLID 24027 - supported SourceDetailID
			// this is different from SourceActionID in that SourceActionID would be the same
			// whether on a template or on a patient EMN, but SourceDetailID would be different
			// on a template than on an EMN
			if(pLoadInfo->m_saiOverride.nSourceDetailID != -1) {
				pLoadInfo->m_sai.nSourceDetailID = pLoadInfo->m_saiOverride.nSourceDetailID;
			}
			else {
				pLoadInfo->m_sai.nSourceDetailID = VarLong(pPreloadedTopic->m_vSourceDetailID, -1);
			}
			// (c.haag 2007-08-02 17:03) - PLID 26833 - Make sure we are giving back the Live source detail by
			// using GetLiveDetail; otherwise an internal detail could escape to the outside!
			pLoadInfo->m_sai.pSourceDetail = GetLiveDetail(pLoadInfo->m_saiOverride.pSourceDetail);

			pLoadInfo->m_strSourceActionName = VarString(pPreloadedTopic->m_vSourceActionName, "");
			pLoadInfo->m_SourceActionDestType = (EmrActionObject)VarLong(pPreloadedTopic->m_vSourceActionDestType, -1);
			pLoadInfo->m_strName = VarString(pPreloadedTopic->m_vName, "");
			pLoadInfo->m_bShowIfEmpty = VarBool(pPreloadedTopic->m_vShowIfEmpty);
			pLoadInfo->m_nTemplateTopicID = VarLong(pPreloadedTopic->m_vTemplateTopicID, -1);
			pLoadInfo->m_nTemplateID = VarLong(pPreloadedTopic->m_vTemplateID, -1);
			pLoadInfo->m_nTemplateTopicOrderIndex = VarLong(pPreloadedTopic->m_vTemplateOrderIndex, -1);
			pLoadInfo->m_bHideOnEMN = FALSE;
			// (a.walling 2008-06-30 12:45) - PLID 29271 - Preview Pane flags
			pLoadInfo->m_nPreviewFlags = VarLong(pPreloadedTopic->m_vPreviewFlags, 0);
		} NxCatchAllCallThrowThread("Error in CEMNLoader::LoadEMRTopic() - Reading topic data", m_mtxTopics.Unlock());

		// Now load subtopics
		// (c.haag 2007-05-01 12:36) - PLID 25853 - We no longer query data to load subtopics. We
		// instead use preloaded topic information
		try {
			// (c.haag 2007-08-08 11:24) - PLID 27014 - Skip this code if we don't want to load subtopics
			if (pLoadInfo->bLoadSubTopics) {
				const int nSubTopics = pPreloadedTopic->m_anSubTopicIDs.GetSize();
				for (int i=0; i < nSubTopics; i++) {
					const long nSubTopicID = pPreloadedTopic->m_anSubTopicIDs[i];
					// (c.haag 2007-05-08 16:29) - PLID 25941 - We now maintain the CEMRTopic object in the
					// preloaded object so that it can be passed along to the main thread after the subtopic is
					// loaded
					CPreloadedTopic* pPreloadedSubTopic = m_mapAllTopicsByID[ nSubTopicID ];
					if (NULL == pPreloadedSubTopic) {
						ThrowNxException("Could not find preloaded subtopic %d!", nSubTopicID);
					}
					//TES 4/15/2010 - PLID 24692 - If we have an ID, and are loading from a template to a template (or from an EMN to an EMN)
					// then we may already have a position entry, otherwise, create a new one.
					TopicPositionEntry *tpeNew = NULL;
					if(nSubTopicID != -1 && pLoadInfo->bLoadFromTemplate == pLoadInfo->bLoadToTemplate) {
						tpeNew = m_pEMN->GetTopicPositionEntryByID(nSubTopicID);
					}
					if(tpeNew == NULL) {
						tpeNew = new TopicPositionEntry;
						tpeNew->nTopicID = nSubTopicID;
					}
					
					CEMRTopic *pTopic = pPreloadedSubTopic->m_pTopic = new CEMRTopic(FALSE, FALSE, tpeNew);
					// (c.haag 2007-05-01 09:37) - PLID 25853 - We already queried the topic data in the
					// preload, so have the topic load from that preloaded information
					// (z.manning 2010-08-20 09:50) - PLID 40190 - Pass in connection pointer
					pTopic->LoadFromPreloadedTopic(pConn, this, nSubTopicID,
						m_pEMN->GetParentEMR()->GetPatientID(),
						pLoadInfo->hWnd, pParentEmr);

					CEMNLoaderMutex* pmtxProblems = GetEMRProblemsMutex();
					CHoldEMNLoaderMutex mhProblems(pmtxProblems);

					// (j.jones 2008-08-22 12:12) - PLID 30789 - load the problems for this topic
					// (c.haag 2009-05-16 11:37) - PLID 34277 - Associate EMR objects with problem links rather than problems
					CEmrProblemLinkAry *pAry = GetEmrProblemLinks(eprtEmrTopic, pTopic->GetID());
					if(pAry) {
						for(int i = 0; i < pAry->GetSize(); i++) {
							CEmrProblemLink *pNewLink = new CEmrProblemLink(pAry->GetAt(i));
							pNewLink->UpdatePointersWithTopic(pTopic);
							pTopic->m_apEmrProblemLinks.Add(pNewLink);
						}
					}

					pLoadInfo->m_aryEMRTopics.Add(pTopic);
				}
			}
			m_mtxTopics.Unlock();
		} NxCatchAllCallThrowThread("Error in CEMNLoader::LoadEMRTopic() - Reading subtopic data", m_mtxTopics.Unlock(););
	}
	
	//now load the details for this topic
	//Keep track of what we need to figure our completion status.
	
	//TES 6/6/2008 - PLID 29416 - We no longer handle these items specially.
	// (c.haag 2007-04-20 09:11) - PLID 25730 - We can now pull EmrInfoID values from the preloader object
	// rather than querying the database
	// (j.jones 2007-07-20 14:41) - PLID 26742 - renamed these functions to differentiate from the EmrUtils function of the same name
	/*long nCurrentMedicationsInfoID = GetLoaderActiveCurrentMedicationsInfoID(pParentEmr, pConn);
	long nAllergiesInfoID = GetLoaderActiveAllergiesInfoID(pParentEmr, pConn);*/
	
	int nEmptyCount = 0;
	int nNonEmptyCount = 0;
	// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
	BOOL bHasUnfilledRequiredDetail = FALSE;
	int i;
	long nDetailCount = 0;

	// (c.haag 2007-07-05 10:08) - PLID 26595 - Take exclusive ownership of internal details from hereon
	if(pLoadInfo->bLoadFromTemplate)
	{
		long nTopicToLoadFrom = pLoadInfo->nID;
		BOOL bManufactureDetails = FALSE;
		try {
			m_mtxInternalDetails.Lock();			
			if(pLoadInfo->m_nOriginalTemplateTopicID != -1 && pLoadInfo->m_sai.nSourceActionID!= -1) {
				// (j.jones 2006-02-24 17:36) - PLID 19473 - if a spawned, remembered topic,
				//load the details from the original topic, not anything saved in the remembered topic
				nTopicToLoadFrom = pLoadInfo->m_nOriginalTemplateTopicID;

				// (c.haag 2007-08-31 17:01) - PLID 26833 - If we get here, it automatically means we cannot
				// pull from the existing "Live" detail pool. We must create them on the fly because we are
				// spawning them.
				bManufactureDetails = TRUE;
			}
		} NxCatchAllCallThrowThread("Error in CEMNLoader::LoadEMRTopic() - Calculating template nTopicToLoadFrom", m_mtxInternalDetails.Unlock());

		try
		{
			// (c.haag 2007-06-15 10:51) - PLID 26344 - Use nTopicToLoadFrom!
			CEMNDetailArray* papDetails = GetDetailArrayByTopicID(nTopicToLoadFrom);

			if (NULL != papDetails) 
			{
				// (z.manning 2011-02-21 17:09) - PLID 42446 - Let's put any parent details at the beginning of the array.
				papDetails->SortWithParentDetailsFirst();
				nDetailCount = papDetails->GetSize();
				for (i=0; i < nDetailCount; i++)
				{
					// (c.haag 2007-08-08 11:27) - PLID 26833 - This is now consistent with how we did things in
					// 8300. The detail recordset filter was
					//
					// WHERE EMRTemplateDetailsT.EMRTemplateTopicID = %li %s",
					// nTopicToLoadFrom, pLoadInfo->bLoadToTemplate?"":"AND EmrTemplateDetailsT.SourceActionID Is Null ");
					//
					CEMNDetail* pInternalDetail = papDetails->GetAt(i);

					// (z.manning 2010-05-06 17:55) - PLID 38527 - I rearranged the logic to determine if we should load a detail.
					BOOL bLoadDetail = FALSE;
					if(m_bIsTemplate) {
						// (z.manning 2010-05-06 17:55) - Always load all details on templates
						bLoadDetail = TRUE;
					}
					else {
						// (z.manning 2010-05-06 17:55) - Do not load spawned details on patient EMNs as that will be
						// handled when processing actions.
						if(pInternalDetail->GetSourceActionID() == -1) {
							bLoadDetail = TRUE;
						}

						// (z.manning 2010-05-06 17:56) - PLID 38527 - Special case for child details (i.e. smart stamp tables).
						// They do not have source action info stored but we still do not want to load them until their parent
						// image gets loaded.
						// (z.manning 2011-01-25 16:06) - PLID 42336 - Support multiple images per smart stamp table
						CEMNDetailArray aryParentDetails;
						GetParentDetailsByTemplateDetailID(pInternalDetail->m_nEMRTemplateDetailID, &aryParentDetails);
						for(int nParentDetailIndex = 0; nParentDetailIndex < aryParentDetails.GetSize(); nParentDetailIndex++) {
							CEMNDetail *pParentDetail = aryParentDetails.GetAt(nParentDetailIndex);
							if(pParentDetail != NULL) {
								if(pParentDetail->GetSourceActionID() == -1) {
									// (z.manning 2011-02-03 10:47) - PLID 42336 - We have at least one detail that is not
									// spawned so we need to load the table.
									bLoadDetail = TRUE;
									break;
								}
								else {
									bLoadDetail = FALSE;
								}
							}
						}
					}

					if (bLoadDetail)
					{
#ifdef REPORT_LOAD_PROGRESS
						TRACE("		Loading detail %s (%d)...\n", pInternalDetail->GetLabelText(), pInternalDetail->m_nEMRTemplateDetailID);
#endif
						// (c.haag 2007-08-08 11:38) - PLID 26833 - Given the template detail ID, we try to fetch
						// its Live equivalent from the Live detail array so that we can later pass it into 
						// pLoadInfo->m_aryEMNDetails.
						//
						// If one is found, that means the detail was added to this template by the user. If one is not found,
						// that means the detail is being spawned in some way in the initial load; and because a detail can
						// be spawned more that once on a template, we have to manufacture a new one.
						//
						CEMNDetail* pLiveDetail = NULL;
						// (j.jones 2010-03-15 09:53) - PLID 37641 - we may clear the "old" live detail later
						// (c.haag 2010-05-26 11:01) - PLID 38751 - "Manufactured" details should have never
						// been put into the live map in the first place. As for memory usage. the owning CEMRTopic 
						// clears it out much later. (On a side note, live details should never be deleted before the 
						// destructor.)
						//CEMNDetail *pDeadDetail = NULL;

						// Only search in the existing "Live" detail pool if we are not spawning
						if (!bManufactureDetails) {
							m_mapLiveDetailsByTemplateID.Lookup(pInternalDetail->m_nEMRTemplateDetailID, pLiveDetail);
						}
						else {
							// (c.haag 2010-05-26 11:01) - PLID 38751 - Undoing Josh's code. If we're manufacturing a detail,
							// we should have nothing to do with m_mapLiveDetailsByTemplateID. Manufactured details are not
							// live details (despite the fact they have a brief stint in a variable named pLiveDetail, which is probably
							// the root cause of the problems we have).
							// (j.jones 2010-03-10 12:03) - PLID 37641 - If bManufactureDetails is true,
							// our matching detail in the live detail array isn't really going to be a live detail!
							// We need to reduce its reference count by 1, because it's never truly going to be
							// added to a topic. Its only reference right now should be the live detail array itself.
							//m_mapLiveDetailsByTemplateID.Lookup(pInternalDetail->m_nEMRTemplateDetailID, pDeadDetail);

							//if we found pDeadDetail, we will release it later in this function,
							//(but only if we have not actually attached it to a topic)
						}

						if (NULL != pLiveDetail) {
							// (c.haag 2007-08-08 11:52) - Assert that the source action ID's match
							ASSERT(pLiveDetail->GetSourceActionID() == pInternalDetail->GetSourceActionID());
							LoadEMRTopicDetail(pLiveDetail, lpCon);

							// (j.jones 2010-02-25 14:40) - PLID 37318 - ensure the SmartStamp links, if any, remain valid
							// (z.manning 2010-03-12 11:43) - PLID 37412 - This is a live detail
							// (c.haag 2010-06-29 12:46) - PLID 39404 - If pLiveDetail was manufactured (and thus may not
							// necessarily be live), we need to pass its internal originator into this function so it knows not to
							// search the live template map.
							ReconfirmSmartStampLinks_ByLiveDetail(pLiveDetail, (bManufactureDetails) ? pInternalDetail : NULL);

						} else {
							_ConnectionPtr pCon(lpCon);
							CEMR *pEMR = NULL;
							if(m_pEMN)
								pEMR = m_pEMN->GetParentEMR();
							
							// Earlier on, we loaded everything about this detail except its
							// state. Load the state now.
							// (c.haag 2007-02-28 09:22) - PLID 24989 - Also load the states
							// of other details in the EMN which somehow affect the form 
							// appearance of this item
							// (c.haag 2007-05-21 09:46) - PLID 26050 - If all the detail states
							// have already been loaded, then there's nothing to do here
							if (!m_bAllStatesLoaded) {
								//TES 6/6/2008 - PLID 29416 - This no longer needs the system table info IDs.
								// (j.jones 2008-09-22 15:48) - PLID 31408 - send the EMR ID
								LoadEMRDetailStateDefaultCascaded(pInternalDetail,
									m_pEMN->GetParentEMR()->GetPatientID(),//pLoadInfo->nPatientID,
									m_pEMN->GetParentEMR()->GetID(),
									lpCon, NULL);
								// Now make sure the preloader object knows that the detail is in use
								SetUsed(pInternalDetail);
							}
							// (a.walling 2009-10-23 09:23) - PLID 36046 - Track construction in initial reference count
							pLiveDetail = CEMNDetail::CreateDetail(NULL, "LoadEMRTopic live detail from internal", FALSE);
							pLiveDetail->CopyFrom(*pInternalDetail); // (a.walling 2012-10-31 17:17) - PLID 53550 - CEMNDetail::opreator= leads to very confusing semantics

							// (c.haag 2010-05-26 11:01) - PLID 38751 - See if we're manufacturing (making new spawned) details.
							if (!bManufactureDetails) 
							{
								pLiveDetail->__AddRef("LoadEMRTopic add to m_apLiveDetails");
								// (j.jones 2010-03-15 09:34) - PLID 37318 - this is never tracked as a live detail
								m_apLiveDetails.Add(pLiveDetail);

								if(pLiveDetail->m_nEMRTemplateDetailID != -1) {
									// (c.haag 2010-05-26 11:01) - PLID 38751 - Undoing Josh's code. A "manufactured" detail
									// is no longer put into the live detail array; so the code below is moot.
									/*CEMNDetail *pOldDetail = NULL;
									// (j.jones 2010-03-15 09:39) - PLID 37641 - remove from our live details array, if it exists
									if(m_mapLiveDetailsByTemplateID.Lookup(pLiveDetail->m_nEMRTemplateDetailID, pOldDetail)) {									
										BOOL bFound = FALSE;
										for(int j=0; j<m_apLiveDetails.GetSize() && !bFound; j++) {
											if(m_apLiveDetails.GetAt(j) == pOldDetail) {
												bFound = TRUE;
												pOldDetail->__Release("LoadEMRTopic - old template detail obsolete");
												m_apLiveDetails.RemoveAt(j);
											}
										}
									}*/
									m_mapLiveDetailsByTemplateID[pLiveDetail->m_nEMRTemplateDetailID] = pLiveDetail;
								}
							}
							else {
								// (c.haag 2010-05-26 11:01) - PLID 38751 - Don't put this in the live array. This is a "manufactured"
								// detail that will get passed into pLoadInfo->m_aryEMNDetails, and then assigned to a CEMRTopic object.
								// That object is responsible for deleting the detail.
							}

							// (j.jones 2010-02-25 14:40) - PLID 37318 - ensure the SmartStamp links, if any, remain valid
							// (z.manning 2010-03-12 11:43) - PLID 37412 - This is a live detail
							// (c.haag 2010-06-29 12:46) - PLID 39404 - If pLiveDetail was manufactured (and thus may not
							// necessarily be live), we need to pass its internal originator into this function so it knows not to
							// search the live template map.
							ReconfirmSmartStampLinks_ByLiveDetail(pLiveDetail, (bManufactureDetails) ? pInternalDetail : NULL);

							// (a.walling 2012-10-31 17:17) - PLID 53550 - LastSavedDetail
							if (pInternalDetail->m_oLastSavedDetail && !pLiveDetail->m_oLastSavedDetail) {
								pLiveDetail->m_oLastSavedDetail = pLiveDetail->CreateLastSavedDetail();
							}
						}

						// (j.jones 2010-02-25 14:40) - PLID 37318 - ensure the SmartStamp links, if any, remain valid
						// (z.manning 2010-03-12 11:43) - PLID 37412 - This is a live detail
						//TES 9/12/2013 - PLID 58438 - If you look at the logic up above, you'll see that this function was just called with these 
						// same parameters, so there's no need to call it again. Moreover, in certain circumstances, calling this function twice
						// will cause issues with a detail being released even though it's still attached to a topic.
						//ReconfirmSmartStampLinks_ByLiveDetail(pLiveDetail, (bManufactureDetails) ? pInternalDetail : NULL);

						////////// Begin matching legacy code ////////////
						pLoadInfo->m_aryEMNDetails.Add( pLiveDetail );

						if(pLiveDetail->GetVisible()) {
							if(pLiveDetail->IsStateSet(NULL)) {
								nNonEmptyCount++;
							}
							else {
								nEmptyCount++;
								// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
								if (!bHasUnfilledRequiredDetail && pLiveDetail->IsRequired()) {
									bHasUnfilledRequiredDetail = TRUE;
								}
							}
						}

						// (c.haag 2010-05-26 11:01) - PLID 38751 - Deprecated
						// (j.jones 2010-03-10 12:03) - PLID 37641 - If we have pDeadDetail, release it now,
						// provided we have not attached it to a topic
						//if(pDeadDetail != NULL) {
						//	if(pDeadDetail->m_pParentTopic == NULL) {
						//		pDeadDetail->__Release("CEMNLoader::LoadEMRTopic - \"old\" live detail will not be used on any topic");
						//	}
						//}
					}
					////////// End matching legacy code ////////////
				} // for (int i=0; i < nDetails; i++) {
			} // if (NULL != papDetails) {
		} NxCatchAllCallThrowThread("Error in CEMNLoader::LoadEMRTopic() - Reading template details", m_mtxInternalDetails.Unlock(););
	}
	else
	{
		try
		{
			m_mtxInternalDetails.Lock();
			CEMNDetailArray* papDetails = GetDetailArrayByTopicID(pLoadInfo->nID);
			if (NULL != papDetails)
			{
				// (z.manning 2011-02-21 17:09) - PLID 42446 - Let's put any parent details at the beginning of the array.
				papDetails->SortWithParentDetailsFirst();
				nDetailCount = papDetails->GetSize();
				for (int i=0; i < nDetailCount; i++)
				{
					CEMNDetail* pLiveDetail = papDetails->GetAt(i);

					// (c.haag 2007-07-19 11:38) - PLID 26833 - Migrating all code related to
					// loading the internal detail into LoadEMRTopicDetail
					LoadEMRTopicDetail(pLiveDetail, lpCon);

					////////// Begin matching legacy code ////////////
					pLoadInfo->m_aryEMNDetails.Add( pLiveDetail );
				
#ifdef REPORT_LOAD_PROGRESS
					TRACE("		Loading detail %s (%d)...\n", pLiveDetail->GetLabelText(), pLiveDetail->m_nEMRDetailID);
#endif

					if(pLiveDetail->GetVisible()) {
						if(pLiveDetail->IsStateSet(NULL)) {
							nNonEmptyCount++;
						}
						else {
							nEmptyCount++;
							// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
							if (!bHasUnfilledRequiredDetail && pLiveDetail->IsRequired()) {
								bHasUnfilledRequiredDetail = TRUE;
							}
						}
					}
					////////// End matching legacy code ////////////
				} // for (int i=0; i < nDetails; i++) {
			} // if (NULL != papDetails) {
		} NxCatchAllCallThrowThread("Error in CEMNLoader::LoadEMRTopic() - Reading details", m_mtxInternalDetails.Unlock());
	}

	//Now, we also know our completion status.
	// (z.manning, 04/04/2008) - PLID 29495 - New status for topics with no details at all.
	if(nDetailCount == 0) {
		pLoadInfo->m_CompletionStatus = etcsBlank;
	}
	else if(nNonEmptyCount && !nEmptyCount) {
		pLoadInfo->m_CompletionStatus = etcsComplete;
	} 
	else if (nNonEmptyCount && bHasUnfilledRequiredDetail) {
		// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
		pLoadInfo->m_CompletionStatus = etcsRequired;
	}
	else if(nNonEmptyCount) {
		pLoadInfo->m_CompletionStatus = etcsPartiallyComplete;
	}
	else {
		pLoadInfo->m_CompletionStatus = etcsIncomplete;
	}

	// (c.haag 2007-04-26 10:50) - PLID 25790 - Now preload the EmrDataT records pertaining to
	// the CEMRInfo objects we loaded
	//DRT 1/21/2008 - PLID 28603 - We now use the generic function for loading the maps.
	try {
		//EnsureEmrDataTMap(lpCon, (m_bIsTemplate) ? TRUE : FALSE, pLoadInfo);
		EnsureEmrMapGeneric(lpCon, (m_bIsTemplate) ? TRUE : FALSE, pLoadInfo);
	} NxCatchAllCallThrowThread("Error in CEMNLoader::LoadEMRTopic() - Calling EnsureEmrMapGeneric", m_mtxInternalDetails.Unlock(););

	// (c.haag 2007-04-25 15:34) - PLID 25774 - Make sure that we have our EMR
	// action data preloaded for the later processing of EMR actions
	try {
		EnsureEmrActionData(pLoadInfo, lpCon);
	} NxCatchAllCallThrowThread("Error in CEMNLoader::LoadEMRTopic() - Calling EnsureEmrActionData", m_mtxInternalDetails.Unlock(););

	try {
		// (c.haag 2007-07-05 14:35) - PLID 26595 - All done with details
		m_mtxInternalDetails.Unlock();

		// (c.haag 2007-08-25 11:57) - PLID 25881 - The rest of the work is done by the caller
		// because it's possible for this object to be deleted at that time.
	} NxCatchAllThrowThread("Error in CEMNLoader::LoadEMRTopic() - Completion");

	EMRLOGINDENT(-1,"CEMNLoader::LoadEMRTopic END"); // (c.haag 2010-05-19 9:04) - PLID 38759
}

void CEMNLoader::LoadEMRTopicDetail(CEMNDetail* pLiveDetail, ADODB::_Connection *lpCon)
{
	// (c.haag 2007-07-27 09:40) - PLID 26833 - The caller has the intention of preparing 
	// pLiveDetail to be assigned to the EMN. This preparation includes:
	//	1. Getting the "Internal" detail which corresponds to the "Live" detail.
	//	2. Loading the state of the "Internal" detail
	//	3. Adding the "Live" detail to the pending detail map for final processing in
	//		FlushPendingEMNDetailWrites later in the main thread.
	//
	// The reason we go through all this trouble is that the EMN, not the EMN Loader, owns
	// "Live" details. We cannot touch the state of a "Live" detail here.
	//
	if (!m_mtxInternalDetails.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to internal details
		// before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::LoadEMRTopicDetail was called without exclusive ownership to details");
	}

	// (c.haag 2007-08-15 11:28) - PLID 26833 - Check to see if we already synchronized the
	// Live detail with its internal counterpart. If so, just quit here.
	BOOL bFinalized = FALSE;
	m_mapFinalizedDetails.Lookup(pLiveDetail, bFinalized);
	if (bFinalized) {
		return;
	}

	// (c.haag 2007-08-08 11:47) - Assert that this live detail has not been loaded before
	// (a.walling 2009-10-12 16:05) - PLID 36023 - We should have no special knowledge of the reference count 
	//ASSERT(2 == pLiveDetail->Internal_GetRefCnt());

	CEMNDetail* pInternalDetail = NULL;
	if (m_bLoadFromTemplate) {

		// (c.haag 2007-07-27 09:42) - PLID 26833 - Get the internal equivalent
		CPreloadedDetail* pPreloadedInternalDetail = NULL;		
		m_mapAllDetailsByTemplateID.Lookup(pLiveDetail->m_nEMRTemplateDetailID, pPreloadedInternalDetail);
		pInternalDetail = (pPreloadedInternalDetail) ? pPreloadedInternalDetail->m_pDetail : NULL;
		if (NULL == pInternalDetail) {
			ASSERT(FALSE);
			ThrowNxException("LoadEMRTopicDetail failed to find the internal template detail with ID %d!", pLiveDetail->m_nEMRTemplateDetailID);
		}

		// (j.jones 2007-07-20 14:41) - PLID 26742 - renamed these functions to differentiate from the EmrUtils function of the same name
		_ConnectionPtr pCon(lpCon);
		CEMR *pEMR = NULL;
		if(m_pEMN)
			pEMR = m_pEMN->GetParentEMR();
		
		// Earlier on, we loaded everything about this detail except its
		// state. Load the state now.
		// (c.haag 2007-02-28 09:22) - PLID 24989 - Also load the states
		// of other details in the EMN which somehow affect the form 
		// appearance of this item
		// (c.haag 2007-05-21 09:46) - PLID 26050 - If all the detail states
		// have already been loaded, then there's nothing to do here
		if (!m_bAllStatesLoaded) {
			//TES 6/6/2008 - PLID 29416 - This no longer needs the system table info IDs.
			// (j.jones 2008-09-22 15:48) - PLID 31408 - send the EMR ID
			LoadEMRDetailStateDefaultCascaded(pInternalDetail,
				m_pEMN->GetParentEMR()->GetPatientID(),//pLoadInfo->nPatientID,
				m_pEMN->GetParentEMR()->GetID(),
				lpCon, NULL);
			// Now make sure the preloader object knows that the detail is in use
			SetUsed(pInternalDetail);
		}

		// In CEMNDetail::LoadFromTemplateDetailRecordset, which is what this
		// code is based off, we called ApplyEmrLinks(). ApplyEmrLinks depends
		// on the state being set in all the details. That function is now called
		// in the post load.
	} else {

		// (c.haag 2007-07-27 09:42) - PLID 26833 - Get the internal equivalent
		CPreloadedDetail* pPreloadedInternalDetail = NULL;
		m_mapAllDetailsByID.Lookup(pLiveDetail->m_nEMRDetailID, pPreloadedInternalDetail);
		pInternalDetail = (pPreloadedInternalDetail) ? pPreloadedInternalDetail->m_pDetail : NULL;
		if (NULL == pInternalDetail) {
			ASSERT(FALSE);
			ThrowNxException("LoadEMRTopicDetail failed to find the internal detail with ID %d!", pLiveDetail->m_nEMRTemplateDetailID);
		}


		// Earlier on, we loaded everything about this detail except its
		// state. Load the state now.
		// (c.haag 2007-02-28 09:22) - PLID 24989 - Also load the states
		// of other details in the EMN which somehow affect the form 
		// appearance of this item
		// (c.haag 2007-05-21 09:46) - PLID 26050 - If all the detail states
		// have already been loaded, then there's nothing to do here
		if (!m_bAllStatesLoaded) {
			LoadEMRDetailStateCascaded(pInternalDetail, lpCon, NULL);
			// Now make sure the preloader object knows that the detail is in use
			SetUsed(pInternalDetail);
		}
	}

	// (c.haag 2007-08-15 11:02) - PLID 25239 - Remember the detail we loaded the state for so that
	// we can update the state of its counterpart in the "Live" array in FlushPendingEMNDetailWrites
	// from the main thread

	// (c.haag 2007-08-15 11:02) - Remember the detail
	AddPendingDetail(pLiveDetail);

	// (c.haag 2007-08-15 11:03) - PLID 25239 - Now remember all of its dependents because we
	// loaded their states too. We may need to reference those states when Narrative or
	// Table-related data is processed in the main thread (see PLID 27073 for one example with
	// CEMN::HandleNarrativeFieldRequest)
	CArray<CEMNDetail*, CEMNDetail*> apDependents;
	BuildDependentList(pInternalDetail, apDependents, lpCon);
	const int nDependents = apDependents.GetSize();
	for (int i=0; i < nDependents; i++) {
		CEMNDetail* pLiveDependent = NULL;

		// Details in the dependent array are Internal details. We have to find its Live counterpart
		// before we consider whether to add it to the pending detail map.
		if (m_bLoadFromTemplate) {
			m_mapLiveDetailsByTemplateID.Lookup(apDependents[i]->m_nEMRTemplateDetailID, pLiveDependent);
		} else {
			m_mapLiveDetailsByID.Lookup(apDependents[i]->m_nEMRDetailID, pLiveDependent);
		}

		if (NULL != pLiveDependent) {
			// We found the dependent detail in the Live detail array. Check to see whether we already
			// synchronized it with its Internal detail counterpart. If so, don't add it to the pending
			// detail map; or else it will try to synchronize again and disrupt our load-each-detail-once
			// flow of control.
			m_mapFinalizedDetails.Lookup(pLiveDependent, bFinalized);
			if (!bFinalized) {
				AddPendingDetail(pLiveDependent);
			}
		} else {
			// (c.haag 2007-08-15 10:59) - If we get here, we managed to come up with a dependent detail
			// which does not exist in the Live detail array. It won't cause any issues, missing data, or
			// crashes; it just means that we can't add it to the pending detail map. At some in the future,
			// a new detail object based on it will be created and assigned to the EMN through spawning.
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Topic functions
//////////////////////////////////////////////////////////////////////
	
CEMNLoader::CPreloadedTopic* CEMNLoader::GetPreloadedTopicByID(long nID)
{
	// (c.haag 2007-07-03 09:51) - PLID 26523 - We now require the caller to have exclusive
	// access to preloader topics
	if (!m_mtxTopics.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to topics before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetPreloadedTopicByID was called without exclusive ownership to topics");
	}

	//
	// (c.haag 2007-05-01 08:41) - PLID 25853 - Returns a preloaded non-template topic given a topic ID
	//
	CPreloadedTopic* p = m_mapAllTopicsByID[nID];
	if (p) {
		return p;
	} else {
		ASSERT(FALSE);
		return NULL;
	}
}

CEMNLoader::CPreloadedTemplateTopic* CEMNLoader::GetPreloadedTemplateTopicByID(long nID)
{
	// (c.haag 2007-07-03 09:51) - PLID 26523 - We now require the caller to have exclusive
	// access to preloader topics
	if (!m_mtxTopics.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to topics before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetPreloadedTemplateTopicByID was called without exclusive ownership to topics");
	}

	//
	// (c.haag 2007-05-02 15:44) - PLID 25881 - Returns a preloaded template topic given a topic ID
	//
	CPreloadedTemplateTopic* p = m_mapAllTemplateTopicsByID[nID];
	if (p) {
		return p;
	} else {
		// (c.haag 2007-05-30 11:53) - PLID 26157 - This may be legitimate if used outside
		// the CEMNLoader object; no need for a debug assertion.
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Detail functions
//////////////////////////////////////////////////////////////////////

CEMNDetail* CEMNLoader::GetDetailByID(long nID)
{
	//
	// (c.haag 2007-04-24 09:37) - PLID 26463 - Returns a detail given an ID
	// (c.haag 2007-07-05 09:33) - PLID 25239 - This is now for internal use only, and
	// we now use a lookup because we shouldn't edit the map here!
	//
	// (c.haag 2007-07-05 10:16) - PLID 26595 - We now require the caller to have exclusive
	// access to internal preloader detail data
	if (!m_mtxInternalDetails.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to internal details
		// before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetDetailByID was called without exclusive ownership to details");
	}
	CPreloadedDetail* p = NULL;
	m_mapAllDetailsByID.Lookup(nID, p);
	if (p) {
		return p->m_pDetail;
	} else {
		// (c.haag 2007-08-20 17:32) - PLID 27049 - If we get here, it's probably because we loaded
		// a remembered table detail state that included linked details in the form "I:<number>". Just
		return NULL;
	}
}


CEMNDetail* CEMNLoader::GetDetailByTemplateDetailID(long nID)
{
	//
	// (c.haag 2007-04-24 09:37) - PLID 26464 - Returns a detail given a template ID
	// (c.haag 2007-07-05 09:33) - PLID 25239 - This is now for internal use only, and
	// we now use a lookup because we shouldn't edit the map here!
	//
	// (c.haag 2007-07-05 10:16) - PLID 26595 - We now require the caller to have exclusive
	// access to internal preloader detail data
	if (!m_mtxInternalDetails.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to internal details
		// before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetDetailByTemplateDetailID was called without exclusive ownership to details");
	}
	CPreloadedDetail* p = NULL;
	m_mapAllDetailsByTemplateID.Lookup(nID, p);
	if (p) {
		return p->m_pDetail;
	} else {
		// (c.haag 2007-08-13 17:36) - PLID 27049 - If we get here, it's probably because we're trying
		// to find a dependent detail which was not loaded in the initial load. This can happen if you repeat
		// these steps:
		//		1. Make a new template
		//		2. Add a multi-select list that is capable of spawning EMR items from templates to the first topic
		//		3. Check the selection in that list which spawns EMR items from templates
		//		4. Add a table with a linked detail column in the first topic
		//		5. Choose one of the spawned details as a linked detail selection in one of the table rows
		//		6. Save the template
		//		7. Uncheck the selection in your multi-selection list that spawned everything. The table should look be the same.
		//		8. Save the template. The table should look be the same.
		//		9. Close the template
		//		10. Open the template
		//		<kerplaow>
		// 
		// The 8300 release threw assertions when this happened. Here, we will ignore it.
		//
		return NULL;
	}
}

// (z.manning 2011-01-25 15:58) - PLID 42336
// (z.manning 2012-07-03 17:49) - PLID 51025 - This function takes a detail pointer instead of just and ID now
void CEMNLoader::GetParentDetailsByDetailID(CEMNDetail *pChildDetail, OUT CEMNDetailArray *paryParentDetails)
{
	if(pChildDetail->m_nEMRDetailID == -1) {
		return;
	}

	if (!m_mtxInternalDetails.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to internal details
		// before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetParentDetailsByDetailID was called without exclusive ownership to details");
	}

	POSITION pos = m_mapAllDetailsByID.GetStartPosition();
	while(pos != NULL)
	{
		long nDetailID;
		CPreloadedDetail* pPreloadedDetail = NULL;
		m_mapAllDetailsByID.GetNextAssoc(pos, nDetailID, pPreloadedDetail);
		if(pPreloadedDetail != NULL)
		{
			if(pPreloadedDetail->m_pDetail->m_nChildEMRDetailID == pChildDetail->m_nEMRDetailID) {
				paryParentDetails->Add(pPreloadedDetail->m_pDetail);
			}
			// (z.manning 2012-07-03 17:50) - PLID 51025 - Also check for an "orphaned" smart stamp image, meaning we have
			// an existing smart stamp image that is somehow not linked to a smart stamp table. This situation really shoudn't
			// be happening, but somehow it has so let's handle it.
			if(pPreloadedDetail->m_pDetail->IsOrphanedSmartStampImage() && pPreloadedDetail->m_pDetail->m_nChildEMRInfoMasterID == pChildDetail->m_nEMRInfoMasterID)
			{
				paryParentDetails->Add(pPreloadedDetail->m_pDetail);
			}
		}
	}
}

// (z.manning 2011-01-25 15:58) - PLID 42336
void CEMNLoader::GetParentDetailsByTemplateDetailID(const long nChildTemplateDetailID, OUT CEMNDetailArray *paryParentDetails)
{
	if(nChildTemplateDetailID == -1) {
		return;
	}

	if (!m_mtxInternalDetails.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to internal details
		// before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetParentDetailsByTemplateDetailID was called without exclusive ownership to details");
	}

	POSITION pos = m_mapAllDetailsByTemplateID.GetStartPosition();
	while(pos != NULL)
	{
		long nTemplateDetailID;
		CPreloadedDetail* pPreloadedDetail = NULL;
		m_mapAllDetailsByTemplateID.GetNextAssoc(pos, nTemplateDetailID, pPreloadedDetail);
		if(pPreloadedDetail != NULL && pPreloadedDetail->m_pDetail->m_nChildEMRTemplateDetailID == nChildTemplateDetailID) {
			paryParentDetails->Add(pPreloadedDetail->m_pDetail);
		}
	}
}

long CEMNLoader::GetInitialLoadDetailCount() const
{
	// (j.jones 2008-10-30 17:04) - PLID 31869 - renamed to ASSERT_IN_SAFE_THREAD
	ASSERT_IN_SAFE_THREAD("CEMNLoader::GetInitialLoadDetailCount");
	//
	// (c.haag 2007-07-27 10:19) - PLID 26833 - Returns the number of details we have to
	// give to the CEMN object we're loading for. This is equal to the number of details
	// that we loaded at the start of the initial load which have no relationship whatsoever
	// with spawning.
	//
	return m_apLiveDetails.GetSize();
}

CEMNDetail* CEMNLoader::GetInitialLoadDetailByIndex(long nIndex) const
{
	// (j.jones 2008-10-30 17:04) - PLID 31869 - renamed to ASSERT_IN_SAFE_THREAD
	ASSERT_IN_SAFE_THREAD("CEMNLoader::GetInitialLoadDetailByIndex");
	//
	// (c.haag 2007-07-27 10:19) - PLID 26833 - Returns a detail owned by the CEMN object
	// we're loading for by its index.
	//
	// (c.haag 2007-07-27 09:57) - The caller should not assume that this detail has
	// a state or is fully loaded yet
	//
	CEMNDetail* p = NULL;
	if (nIndex < m_apLiveDetails.GetSize()) {
		return m_apLiveDetails[nIndex];
	}
	else {
		ASSERT(FALSE);
		return NULL;
	}
}

CEMNDetail* CEMNLoader::GetInitialLoadDetailByID(long nID) const
{
	// (j.jones 2008-10-30 17:04) - PLID 31869 - renamed to ASSERT_IN_SAFE_THREAD
	ASSERT_IN_SAFE_THREAD("CEMNLoader::GetInitialLoadDetailByID");
	//
	// (c.haag 2007-07-27 10:20) - PLID 26833 - Returns a detail owned by the CEMN object
	// we're loading for by its ID.
	//
	// (c.haag 2007-07-27 09:57) - The caller should not assume that this detail has
	// a state or is fully loaded yet
	//
	CEMNDetail* p = NULL;
	m_mapLiveDetailsByID.Lookup(nID, p);
	if (p) {
		return p;
	} else {
		ASSERT(FALSE);
		return NULL;		
	}
}

// (z.manning 2011-01-25 17:43) - PLID 42336
void CEMNLoader::GetInitialLoadParentDetailsByDetailID(const long nChildDetailID, OUT CEMNDetailArray *paryParentDetails)
{
	if(nChildDetailID == -1) {
		return;
	}

	POSITION pos = m_mapLiveDetailsByID.GetStartPosition();
	while(pos != NULL)
	{
		long nDetailID;
		CEMNDetail* pDetail = NULL;
		m_mapLiveDetailsByID.GetNextAssoc(pos, nDetailID, pDetail);
		if(pDetail != NULL && pDetail->m_nChildEMRDetailID == nChildDetailID) {
			paryParentDetails->Add(pDetail);
		}
	}
}

void CEMNLoader::GetInitialLoadParentDetailsByTemplateDetailID(const long nChildTemplateDetailID, OUT CEMNDetailArray *paryParentDetails)
{
	if(nChildTemplateDetailID == -1) {
		return;
	}

	POSITION pos = m_mapLiveDetailsByID.GetStartPosition();
	while(pos != NULL)
	{
		long nDetailID;
		CEMNDetail* pDetail = NULL;
		m_mapLiveDetailsByID.GetNextAssoc(pos, nDetailID, pDetail);
		if(pDetail != NULL && pDetail->m_nChildEMRTemplateDetailID == nChildTemplateDetailID) {
			paryParentDetails->Add(pDetail);
		}
	}
}

// (z.manning 2010-07-21 09:30) - PLID 39591
CEMNDetail* CEMNLoader::GetIntialLoadSmartStampImageByTemplateID(long nID) const
{
	CEMNDetail *pSmartStampImage = GetInitialLoadDetailByTemplateID(nID);
	// (z.manning 2010-07-21 09:32) - PLID 39591 - Only return the image if smart stamps are enabled
	if(pSmartStampImage != NULL && pSmartStampImage->m_bSmartStampsEnabled) {
		return pSmartStampImage;
	}
	return NULL;
}

// (z.manning 2011-02-18 14:29) - PLID 42336
CEMNDetail* CEMNLoader::GetSmartStampTableDetailByInfoMasterID(const long nTableInfoMasterID, const long nImageInfoMasterID) const
{
	if(nTableInfoMasterID == -1) {
		return NULL;
	}

	POSITION pos = m_mapLiveDetailsByID.GetStartPosition();
	while(pos != NULL)
	{
		long nDetailID;
		CEMNDetail* pDetail = NULL;
		m_mapLiveDetailsByID.GetNextAssoc(pos, nDetailID, pDetail);
		if(pDetail != NULL && pDetail->m_nEMRInfoMasterID == nTableInfoMasterID) {
			// (z.manning 2011-02-18 15:02) - PLID 42336 - A table can only be associated with the same image once
			// so make sure this table isn't already linked to this image before we return the table.
			if(pDetail->GetSmartStampImageDetails()->GetDetailFromEmrInfoMasterID(nImageInfoMasterID) == NULL) {
				return pDetail;
			}
		}
	}

	return NULL;
}

CEMNDetail* CEMNLoader::GetInitialLoadDetailByTemplateID(long nID) const
{
	// (j.jones 2008-10-30 17:04) - PLID 31869 - renamed to ASSERT_IN_SAFE_THREAD
	ASSERT_IN_SAFE_THREAD("CEMNLoader::GetInitialLoadDetailByTemplateID");
	//
	// (c.haag 2007-07-27 10:21) - PLID 26833 - Returns a detail owned by the CEMN object
	// we're loading for by its template ID.
	//
	// (c.haag 2007-07-27 09:57) - The caller should not assume that this detail has
	// a state or is fully loaded yet
	//
	CEMNDetail* p = NULL;
	m_mapLiveDetailsByTemplateID.Lookup(nID, p);
	if (p) {
		return p;
	} else {
		// (j.jones 2010-03-03 14:30) - PLID 37231 - there are now legitimate
		// cases where this can be NULL, no assertion necessary
		//ASSERT(FALSE);
		return NULL;
	}
}

CEMNLoader::CPreloadedDetail* CEMNLoader::GetPreloadedDetailByID(long nID)
{
	// (c.haag 2007-07-05 09:20) - PLID 26595 - We now require the caller to have exclusive
	// access to internal preloader detail data
	if (!m_mtxInternalDetails.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to internal details
		// before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetPreloadedDetailByID was called without exclusive ownership to details");
	}

	//
	// (c.haag 2007-04-27 10:22) - PLID 25790 - Returns a preloaded detail given an ID.
	// This gives the caller complete access to a detail and special preloaded fields
	// that are related to content and the info item
	//
	CPreloadedDetail* p = m_mapAllDetailsByID[nID];
	if (p) {
		return p;
	} else {
		// (j.jones 2010-03-03 14:30) - PLID 37231 - there are now legitimate
		// cases where this can be NULL, no assertion necessary
		//ASSERT(FALSE);
		return NULL;
	}
}

// (z.manning 2011-02-21 08:40) - PLID 42338 - Changed the return type to CEMNDetailArray
CEMNDetailArray* CEMNLoader::GetDetailArrayByTopicID(long nTopicID)
{
	// (c.haag 2007-02-27 09:49) - PLID 24949 - Returns a pointer to an array
	// of details that belong to a given topic ID
	CEMNDetailArray* papDetails = NULL;
	if (!m_mapDetailTopics.Lookup(nTopicID, papDetails)) {
		return NULL;
	} else {
		return papDetails;
	}
}

long CEMNLoader::GetTotalTemplateTopicCount()
{
	// (c.haag 2007-05-30 16:36) - PLID 26187 - Returns the template topic count
	// (c.haag 2007-07-03 10:14) - PLID 26523 - Take exclusive ownership of topics until this is done
	CHoldEMNLoaderMutex mh(&m_mtxTopics);
	long nCount = m_apAllTemplateTopics.GetSize();
	return nCount;
}

CEMNLoader::CPreloadedTemplateTopic* CEMNLoader::GetPreloadedTemplateTopicByIndex(long nIndex)
{
	// (c.haag 2007-07-03 09:51) - PLID 26523 - We now require the caller to have exclusive
	// access to preloader topics
	if (!m_mtxTopics.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to topics before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetPreloadedTemplateTopicByIndex was called without exclusive ownership to topics");
	}

	// (c.haag 2007-05-30 16:37) - PLID 26187 - Returns a preloaded template
	// topic by index
	CPreloadedTemplateTopic* p = m_apAllTemplateTopics[nIndex];
	if (p) {
		return p;
	} else {
		ASSERT(FALSE);
		return NULL;
	}	
}

void CEMNLoader::SetUsed(CEMNDetail* pDetail)
{
	// (c.haag 2007-02-26 18:03) - PLID 24949 - When a detail is assigned to a topic,
	// this must be called so that we know not to delete it
	m_mapUsedDetails[pDetail] = TRUE;
}

void CEMNLoader::SetStateLoaded(CEMNDetail* pDetail)
{
	// (c.haag 2007-07-05 10:08) - PLID 26595 - Take exclusive ownership of internal details
	CHoldEMNLoaderMutex mhDetails(&m_mtxInternalDetails);
	// (c.haag 2007-06-19 09:40) - PLID 26050 - Flags a detail as having a state that
	// was loaded by the preloader
	m_mapDetailsWithLoadedStates[pDetail] = TRUE;
}

BOOL CEMNLoader::WasStateLoaded(CEMNDetail* pDetail)
{
	// (c.haag 2007-07-05 10:08) - PLID 26595 - Take exclusive ownership of internal details
	CHoldEMNLoaderMutex mhDetails(&m_mtxInternalDetails);
	// (c.haag 2007-06-19 09:39) - PLID 26050 - Returns TRUE if we already loaded
	// (or tried to load) the state of a detail. The only time that this returns
	// TRUE and pDetail->m_varState.vt == VT_EMPTY is for Current Medications or
	// Allergies details that will be properly loaded in the post topic load.
	BOOL bLoaded = FALSE;
	if (!m_mapDetailsWithLoadedStates.Lookup(pDetail, bLoaded)) {
		return FALSE;
	}
	return bLoaded;
}

//////////////////////////////////////////////////////////////////////
// Detail State functions
//////////////////////////////////////////////////////////////////////

_variant_t CEMNLoader::LoadEMRDetailState(long nEMRDetailID, EmrInfoType nEMRInfoDatatype, OPTIONAL IN ADODB::_Connection *lpCon /*= NULL*/)
{
	// (c.haag 2007-07-05 10:08) - PLID 26595 - Take exclusive ownership of internal details
	CHoldEMNLoaderMutex mhDetails(&m_mtxInternalDetails);

	//
	// (c.haag 2007-04-27 09:41) - PLID 25768 - This is an overload of the global LoadEMRDetailState
	// function, but optimized for initial loads. Every place that used a query now uses member variables.
	// Unless otherwise commented, consider everything in this function as part of 25768.
	//
	// (c.haag 2007-06-25 11:29) - PLID 26444 - Except tables...they are for PLID 26444
	// (c.haag 2007-06-25 11:57) - PLID 26446 - Lists are for PLID 26446
	//
	_variant_t varNull;
	varNull.vt = VT_NULL;

	CPreloadedDetail* pPreloadedDetail = NULL;

	// (c.haag 2007-07-12 12:43) - PLID 25758 - First, attempt to get the preloaded detail from our
	// master detail list
	if (NULL == (pPreloadedDetail = m_mapAllDetailsByID[nEMRDetailID])) {

		// (c.haag 2007-07-12 12:44) - PLID 25758 - If we cannot find it, it *could* mean that it is a detail
		// from a completely unrelated chart whose state needs to be "Remembered" here.
		if (m_bLoadFromTemplate) {
			EnsureRememberedDetailMap(lpCon);
			m_mapRememberedDetails.Lookup(nEMRDetailID, pPreloadedDetail);
		}

		if (NULL == pPreloadedDetail) {
			// (c.haag 2007-04-24 12:55) - PLID 25768 - This should never happen, but if an optimization fails,
			// we should go back to the non-optimized code.
			// (j.jones 2007-08-02 10:25) - PLID 26912 - pass in NULL because we do not have an open recordset
			// (z.manning 2011-02-24 18:43) - PLID 42579 - This now takes an array of detail IDs
			CArray<long,long> arynDetailIDs;
			arynDetailIDs.Add(nEMRDetailID);
			return ::LoadEMRDetailState(NULL, arynDetailIDs, nEMRInfoDatatype, lpCon);
		}
	}

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();
	switch (nEMRInfoDatatype) {
	case eitText:
		//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
		return _variant_t(VarString(pPreloadedDetail->m_vDetailText, ""));

	case eitSingleList:
		{
			EnsureSelectMap(pCon);
			CArray<long,long>* paDataIDs = m_mapSelect[nEMRDetailID];
			long nDataID = (paDataIDs) ? paDataIDs->GetAt(0) : -1;
			if (nDataID > -1) {
				// Return the data id as a string
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
				return _variant_t(AsString(nDataID));
			} else {
				// No selection for this detail.  We used to not allow this, but now "no selection" is valid.
				return "";
			}
		}
		break;
	case eitMultiList:
		{
			// Return the list of data ids as a semicolon-delimited string
			CString strAns;
			EnsureSelectMap(pCon);
			CArray<long,long>* paDataIDs = m_mapSelect[nEMRDetailID];
			int nElements = (paDataIDs) ? paDataIDs->GetSize() : 0;
			for (int i=0; i < nElements; i++) {
				long nDataID = paDataIDs->GetAt(i);
				if (!strAns.IsEmpty()) {
					strAns += "; ";
				}
				strAns += AsString(nDataID);
			}
			// If strAns is empty, then there is no selection for this detail.  We used to not allow this, but now "no selection" is valid.
			//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
			return _variant_t(strAns);
		}

		break;
	case eitImage:
		{
			// Create a new object from the data
			CEmrItemAdvImageState ais;				
			//
			// (c.haag 2006-11-09 17:46) - PLID 23365 - Whenever we pull InkImageTypeOverride, we could
			// get a null value. If so, it means we pulled it from an EmrInfo item where the user must
			// choose the image to assign to the detail. In that event, we need to assign "blank default
			// values" to the image state. In our program, this means clearing the path and assigning the
			// image type of itDiagram.

			// (c.haag 2007-02-09 15:39) - PLID 23365 - The previous comment is wrong. The simple story
			// is this: EmrDetailsT.InkImageTypeOverride will be non-null if there is a detail-specific
			// image to override the EmrInfoT-specific image. If it is null, we must pull the image data
			// from EmrInfoT later on. What's important here is that ais reflects the state of the detail
			// as it is in data.
			//
			if (itUndefined == (ais.m_eitImageTypeOverride = (eImageType)VarLong(pPreloadedDetail->m_vDetailInkImageTypeOverride, -1))) {
				ais.m_strImagePathOverride.Empty();				
			} else {
				ais.m_strImagePathOverride = VarString(pPreloadedDetail->m_vDetailInkImagePathOverride, "");
			}
			ais.m_varInkData = pPreloadedDetail->m_vDetailInkData;
			ais.m_varTextData = pPreloadedDetail->m_vDetailImageTextData;
			ais.m_varPrintData = pPreloadedDetail->m_vDetailPrintData;
			ais.m_varDetailImageStamps = pPreloadedDetail->m_arypDetailImageStamps.GetAsVariant();

			//DRT 1/23/2008 - PLID 28698 - Load hotspots
			EnsureHotSpotMap(pCon);
			CArray<long,long>* paDataIDs = m_mapHotSpots[nEMRDetailID];
			if(paDataIDs) {
				//We have some selected IDs, pack them into a semi-colon delimited
				//	list that becomes the state.
				int nElements = paDataIDs->GetSize();
				CString strAns;
				for (int i = 0; i < nElements; i++) {
					long nDataID = paDataIDs->GetAt(i);
					strAns += AsString(nDataID) + ";";
				}

				ais.m_strSelectedHotSpotData = strAns;
			}
			else {
				//There was nothing mapped, so no hotspots are pre-selected
			}
			return ais.AsSafeArrayVariant();
		}
		break;
	case eitSlider:
		return pPreloadedDetail->m_vDetailSliderValue;

	case eitNarrative:
		//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
		return _variant_t(VarString(pPreloadedDetail->m_vDetailText, ""));

	case eitTable:
		{
			// (c.haag 2007-06-04 17:51) - PLID 26444 - We now get table state information from a preloaded map.
			CString strResult;
			EnsureTableMap(lpCon);
			strResult = m_mapTable[nEMRDetailID];
			//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
			return _variant_t(strResult);

			//the format is X;Y;Data;Repeat...;
		}
		break;
	default:
		// Unexpected
		ASSERT(FALSE);
		return varNull;
		break;
	}
}

void CEMNLoader::BuildTableDependentList(CEMNDetail* pDetail, OUT CArray<CEMNDetail*, CEMNDetail*>& apDependents,
										 ADODB::_Connection *lpCon)
{
	//
	// (c.haag 2007-07-16 09:59) - PLID 26651 - Go through the state checking the ID's of
	// linked detail columns, and add the details which correspond to those ID's to apDependents
	//

	//TES 6/6/2008 - PLID 29416 - We no longer handle these items specially.
	//DRT 10/8/2007 - PLID 27657 - If the state is VT_EMPTY and we're working with allergies or
	//	medications, we are not yet allowed to load the content.  There is special
	//	loading in place for those tables in CEMRTopic::PostLoad.
	/*if(pDetail->GetStateVarType() == VT_EMPTY && (pDetail->IsCurrentMedicationsTable() || pDetail->IsAllergiesTable())) {
		return;
	}*/

	//TES 9/28/2011 - PLID 45702 - This function only cares about linked details, which most tables don't have.  If we haven't yet populated 
	// the data items, then calling LoadContent() is going to load all of our rows and columns from data, which takes a while.  Since most 
	// tables don't have linked columns, we can try and save time here by just checking for the presence of linked columns, rather than loading
	// the entire table.
	CEMNLoaderMultiMutex* pEMNLoaderMutex = GetInternalDetailsAndEMRInfoMultiMutex();
	//TES 9/28/2011 - PLID 45702 - CEMNLoader's CEMNLoaderMultiMutex pointer has the same lifespan as CEMNLoader itself
	ASSERT(pEMNLoaderMutex != NULL);
	pEMNLoaderMutex->Lock();
	BOOL bDataItemsPopulated = FALSE;
	//TES 9/28/2011 - PLID 45702 - If there's no CEMRInfoItem object, then we know it hasn't loaded its Data IDs.
	CEMRInfoItem *pItem = GetEmrInfoItem(pDetail->m_nEMRInfoID);
	if(pItem) {
		bDataItemsPopulated = pItem->m_bDataItemsPopulated;
	}
	pEMNLoaderMutex->Unlock();
	if(!bDataItemsPopulated) {
		if(!ReturnsRecordsParam(lpCon, "SELECT TOP 1 ID FROM EmrDataT WHERE EMRDataT.ListType = {CONST} AND EmrDataT.EmrInfoID = {INT}", LIST_TYPE_LINKED, pDetail->m_nEMRInfoID)) {
			//TES 9/28/2011 - PLID 45702 - There are no linked detail columns, and thus no dependent details.  
			// So, since we've loaded 0 of 0 dependent details into the array, we are done.
			return;
		}
	}

	// First, we have to ensure the table content is loaded. Otherwise, we have no idea where
	// the linked columns are.
	pDetail->SetNeedSyncContentAndState();
	
	// (j.jones 2011-04-28 14:39) - PLID 43122 - pass in our ProviderIDForFloatingData, if we have one
	pDetail->LoadContent(FALSE, this, lpCon, TRUE, NULL, m_nProviderIDForFloatingData);

	// Second, get the state because it contains the ID's of linked details
	CString strState = VarString(pDetail->GetState(), "");

	// Third, traverse the state looking for the linked details ID's
	// (c.haag 2007-08-18 10:54) - PLID 27112 - Use an iterator to do the state traversal
	CEmrTableStateIterator etsi(strState);
	// (z.manning 2011-03-02 14:53) - PLID 42335 - Added stamp ID to table state
	long X, Y, nEmrDetailImageStampID, nEmrDetailImageStampPointer, nStampID;
	CString strData;
	while (etsi.ReadNextElement(X,Y,strData,nEmrDetailImageStampID,nEmrDetailImageStampPointer,nStampID))
	{
		// Get the table column
		TableColumn* pCol = pDetail->GetColumnByID(Y);
		if (NULL != pCol) {
			if (LIST_TYPE_LINKED == pCol->nType && strData.GetLength() > 2) {
				// If we get here, strData refers to a cell in the linked column, and it
				// actually has data in it. If so, data that beings with a T: means it's
				// a template detail. If it starts with an I:, it's an existing detail for
				// a patient chart.
				CString strPrefix = strData.Left(2);
				CString strSuffix = strData.Right( strData.GetLength() - 2 );
				long nDetailID = atol(strSuffix);
				CEMNDetail* pDetail = NULL;
				if ("T:" == strPrefix) {
					pDetail = GetDetailByTemplateDetailID(nDetailID);
				}
				else if ("I:" == strPrefix) {
					pDetail = GetDetailByID(nDetailID);
				}
				if (NULL != pDetail) {
					apDependents.Add(pDetail);
				} else {
					// (c.haag 2007-08-13 17:36) - PLID 27049 - If we get here, it's probably because we're trying
					// to find a dependent detail which was not loaded in the initial load. This can happen if you repeat
					// these steps:
					//		1. Make a new template
					//		2. Add a multi-select list that is capable of spawning EMR items from templates to the first topic
					//		3. Check the selection in that list which spawns EMR items from templates
					//		4. Add a table with a linked detail column in the first topic
					//		5. Choose one of the spawned details as a linked detail selection in one of the table rows
					//		6. Save the template
					//		7. Uncheck the selection in your multi-selection list that spawned everything. The table should look be the same.
					//		8. Save the template. The table should look be the same.
					//		9. Close the template
					//		10. Open the template
					//		<kerplaow>
					// 
					// The 8300 release threw assertions when this happened. Here, we will ignore it.
				}
			}
		} else {
			// If we get here, the column doesn't exist! Legacy code would just ignore this. So,
			// we will do the same.
		}
	}
}

// (c.haag 2007-07-20 13:13) - PLID 26651 - Added a connection pointer parameter
void CEMNLoader::BuildDependentList(CEMNDetail* pDetail, OUT CArray<CEMNDetail*, CEMNDetail*>& apDependents,
									ADODB::_Connection *lpCon)
{
	// (c.haag 2007-07-05 10:08) - PLID 26595 - Take exclusive ownership of internal details
	CHoldEMNLoaderMutex mhDetails(&m_mtxInternalDetails);

	// (c.haag 2007-07-02 09:08) - PLID 26515 - This function encapsulates the act of populating
	// the apDependents array with all details which depend on pDetail's appearance. The only time
	// we actually do any work is if the detail is a narrative, in which case narrative field
	// appearances depend on other details. It is those details we store in apDependents.
	switch (pDetail->m_EMRInfoType) {
	case eitText:
	case eitSingleList:
	case eitMultiList:
	case eitImage:
	case eitSlider:
		// These are details whose appearance is not affected by other details
		break;
	case eitTable:
		// (c.haag 2007-07-16 11:28) - PLID 26651 - We now support tables
		BuildTableDependentList(pDetail, apDependents, lpCon);
		break;
	case eitNarrative:
		// Narrative appearances are affected by other details;
		// (c.haag 2007-08-09 16:51) - PLID 27038 - Because we are passing it an array of
		// EMN details, we do not need to pass in an EMN object.
		// (c.haag 2007-08-15 11:58) - PLID 27073 - Added the connection object as a parameter
		// (c.haag 2007-10-23 09:58) - PLID 27833 - It's possible for narrative details to have
		// merge fields which correspond to multiple details. The legacy behavior of GetLinkedDetails
		// was to return the first one; but because it may not always return the same one, we must
		// request *all* qualifying details. To do so, we set the last parameter to TRUE.
		pDetail->GetLinkedDetails(apDependents, NULL, &m_apAllCEMNDetails, lpCon, TRUE);
		break;
	default:
		ASSERT(false); // If you get here, it means you need to add support for your new type
		break;
	}
}

// (c.haag 2007-07-02 09:16) - PLID 26515 - We no longer need the array of all EMN details input here
// now that we maintain them in a member list
void CEMNLoader::LoadEMRDetailStateCascaded(CEMNDetail* pDetail, ADODB::_Connection *lpCon,
											EMRTopicLoadInfo *pLoadInfo)
{
	// (c.haag 2007-07-05 10:08) - PLID 26595 - Take exclusive ownership of internal details
	CHoldEMNLoaderMutex mhDetails(&m_mtxInternalDetails);

	//
	// (c.haag 2007-02-28 12:28) - PLID 24989 - This function determines the state of pDetail,
	// loads it into pDetail, and then repeats for all details whose state affects the appearance
	// of pDetail
	//
	if (NULL == pDetail) {
		ASSERT(FALSE);
		return; 
	}

	// First, ensure the detail state has been loaded
	// (c.haag 2007-06-19 11:28) - PLID 26050 - Instead of using CEMNDetail::IsStateSet, which
	// can return FALSE even if we previously loaded a detail, use WasStateLoaded.
	if (WasStateLoaded(pDetail)) {
		// If we get here, the detail has already been loaded. Do nothing.
	} else {
		// If we get here, the state has not been loaded yet, so set it now
		pDetail->SetState(LoadEMRDetailState(pDetail->m_nEMRDetailID, 
			pDetail->m_EMRInfoType, lpCon));

		// (c.haag 2007-06-19 09:32) - PLID 26050 - Now flag the fact we've loaded the state of this detail
		SetStateLoaded(pDetail);
	}

	// Now build the dependent list based on the detail type
	// (c.haag 2007-07-02 09:11) - PLID 26515 - We now call BuildDependentList
	CArray<CEMNDetail*, CEMNDetail*> apDependents;
	BuildDependentList(pDetail, apDependents, lpCon);

	// Now load the states of all the dependents
	const int nDependents = apDependents.GetSize();
	for (int i=0; i < nDependents; i++) {
		CEMNDetail* pDependent = apDependents[i];
		// (c.haag 2007-06-19 11:28) - PLID 26050 - Instead of using CEMNDetail::IsStateSet, which
		// can return FALSE even if we previously loaded a detail, use WasStateLoaded.
		if (!WasStateLoaded(pDependent))
		{
			pDependent->SetState(LoadEMRDetailState(pDependent->m_nEMRDetailID,
				pDependent->m_EMRInfoType, lpCon));

			// (c.haag 2007-06-19 09:32) - PLID 26050 - Now flag the fact we've loaded the state of this dependent
			SetStateLoaded(pDependent);

			// (z.manning, 04/03/2008) - PLID 29539 - This is the dependent of used detail so we must mark this one used as well.
			SetUsed(pDependent);
		}
	}

	// We cannot put the narrative text of one detail inside another. In addition, narratives
	// are the only detail type that will propagate apEMNDetails. Ergo, we don't need to do
	// any recursion.
}

// (c.haag 2007-07-02 09:16) - PLID 26515 - We no longer need the array of all EMN details input here
// now that we maintain them in a member list
// (j.jones 2008-09-22 15:48) - PLID 31408 - added nEMRGroupID
void CEMNLoader::LoadEMRDetailStateDefaultCascaded(CEMNDetail* pDetail,
									   long nPatientID, long nEMRGroupID,
									   ADODB::_Connection *lpCon,
									   EMRTopicLoadInfo *pLoadInfo)
{
	// (c.haag 2007-07-05 10:08) - PLID 26595 - Take exclusive ownership of internal details
	CHoldEMNLoaderMutex mhDetails(&m_mtxInternalDetails);

	// (c.haag 2007-02-28 13:30) - PLID 24989 - This function determines the default state of pDetail,
	// loads it into pDetail, and then repeats for all details whose state affects the appearance
	// of pDetail
	//
	if (NULL == pDetail) {
		ASSERT(FALSE);
		return; 
	}

	//TES 6/6/2008 - PLID 29416 - Let's go ahead and load system tables states here, that will load the full state, and then 
	// the first column will be overwritten in PostLoadDetails() if necessary.

	/*// Current Medications items are always loaded in PostLoadDetails, so don't load it here
	bool bSyncWithCurrentMedications = (!pDetail->m_bIsTemplateDetail && pDetail->IsCurrentMedicationsTable() && (nCurrentMedicationsInfoID == pDetail->m_nEMRInfoID)) ? true : false;
	// (c.haag 2007-04-12 09:05) - PLID 25516 - Same with Allergies items
	bool bSyncWithAllergies = (!pDetail->m_bIsTemplateDetail && pDetail->IsAllergiesTable() && (nAllergiesInfoID == pDetail->m_nEMRInfoID)) ? true : false;
	if (bSyncWithCurrentMedications || bSyncWithAllergies) {
		// Empty the state so that CEMRTopic::PostLoadDetails will do the load for us
		_variant_t vEmpty;
		vEmpty.vt = VT_EMPTY;
		pDetail->SetState(vEmpty);
		// (c.haag 2007-06-19 09:32) - PLID 26050 - Even though the state is empty, we need to
		// flag the fact that we tried loading it for this detail.
		SetStateLoaded(pDetail, pLoadInfo);
		return;
	}*/

	// First, ensure the detail state has been loaded
	// (c.haag 2007-06-19 11:28) - PLID 26050 - Instead of using CEMNDetail::IsStateSet, which
	// can return FALSE even if we previously loaded a detail, use WasStateLoaded.
	if (WasStateLoaded(pDetail)) {
		// If we get here, the detail has already been loaded. Do nothing.
	} else {
		// (j.jones 2008-09-22 15:26) - PLID 31408 - send in the EMRGroupID
		// (z.manning 2011-11-16 12:56) - PLID 38130 - Pass in parameter for remembered detail ID
		pDetail->SetState(LoadEMRDetailStateDefault(pDetail->m_nEMRInfoID,
			nPatientID, nEMRGroupID, pDetail->m_nEMRTemplateDetailID, lpCon, pDetail->m_nRememberedDetailID));

		// (c.haag 2007-06-19 09:32) - PLID 26050 - Now flag the fact we've loaded the state of this detail
		SetStateLoaded(pDetail);
	}

	// Now build the dependent list based on the detail type
	// (c.haag 2007-07-02 09:11) - PLID 26515 - We now call BuildDependentList
	CArray<CEMNDetail*, CEMNDetail*> apDependents;
	BuildDependentList(pDetail, apDependents, lpCon);

	// Now load the states of all the dependents
	const int nDependents = apDependents.GetSize();
	for (int i=0; i < nDependents; i++) {
		CEMNDetail* pDependent = apDependents[i];

		//TES 6/6/2008 - PLID 29416 - Let's go ahead and load system tables states here, that will load the full state, and then 
		// the first column will be overwritten in PostLoadDetails() if necessary.
		/*bSyncWithCurrentMedications = (!pDependent->m_bIsTemplateDetail && pDependent->IsCurrentMedicationsTable() && (nCurrentMedicationsInfoID == pDependent->m_nEMRInfoID)) ? true : false;
		// (c.haag 2007-04-12 09:07) - PLID 25516 - Empty the state if this is an allergies item so that
		// CEMRTopic::PostLoadDetails will do the load for us
		bSyncWithAllergies = (!pDependent->m_bIsTemplateDetail && pDependent->IsAllergiesTable() && (nAllergiesInfoID == pDependent->m_nEMRInfoID)) ? true : false;
		if (bSyncWithCurrentMedications || bSyncWithAllergies)
		{
			// Empty the state so that CEMRTopic::PostLoadDetails will do the load for us
			_variant_t vEmpty;
			vEmpty.vt = VT_EMPTY;
			pDependent->SetState(vEmpty);
			// (c.haag 2007-06-19 09:32) - PLID 26050 - Even though the state is empty, we need to
			// flag the fact that we tried loading it for this dependent.
			SetStateLoaded(pDependent, pLoadInfo);
		}*/

		// (c.haag 2007-06-19 11:28) - PLID 26050 - Instead of using CEMNDetail::IsStateSet, which
		// can return FALSE even if we previously loaded a detail, use WasStateLoaded.
		if (!WasStateLoaded(pDependent))
		{
			// (j.jones 2008-09-22 15:26) - PLID 31408 - send in the EMRGroupID
			// (z.manning 2011-11-16 12:56) - PLID 38130 - Pass in parameter for remembered detail ID
			pDependent->SetState(LoadEMRDetailStateDefault(pDependent->m_nEMRInfoID,
				nPatientID, nEMRGroupID, pDependent->m_nEMRTemplateDetailID, lpCon, pDependent->m_nRememberedDetailID));

			// (c.haag 2007-06-19 09:32) - PLID 26050 - Now flag the fact we've loaded the state of this detail
			SetStateLoaded(pDependent);

			// (z.manning, 04/04/2008) - PLID 29539 - This is the dependent of used detail so we must mark this one used as well.
			SetUsed(pDependent);
		}
	}

	// We cannot put the narrative text of one detail inside another. In addition, narratives
	// are the only detail type that will propagate apEMNDetails. Ergo, we don't need to do
	// any recursion.
}

// (j.jones 2008-09-22 15:20) - PLID 31408 - added nEMRGroupID as a parameter
// (z.manning 2011-11-16 12:21) - PLID 38130 - Removed default parameters and added an output parameter for 
// the remembered detail ID from which we loaded the state.
_variant_t CEMNLoader::LoadEMRDetailStateDefault(long nEmrInfoID, long nPatientID, long nEMRGroupID, long nEmrTemplateDetailID, IN ADODB::_Connection *lpCon, OUT long &nRememberedDetailID)
{
	//
	// (c.haag 2007-04-24 08:43) - PLID 25759 - This is an overload of the global LoadEMRDetailStateDefault
	// function, but optimized for initial loads. Every place that used a query now uses member variables.
	// Unless otherwise commented, consider everything in this function as part of 25759.
	//
	_variant_t varNull;
	varNull.vt = VT_NULL;

	// (c.haag 2007-07-03 10:22) - PLID 26523 - Take exclusive ownership of EMR info objects until this
	// function is done
	// (c.haag 2007-07-05 10:08) - PLID 26595 - Take exclusive ownership of internal details
	CHoldEMNLoaderMultiMutex mh(m_pmtxInternalDetailsAndEMRInfo);

	CPreloadedDetail* pPreloadedDetail = m_mapAllDetailsByTemplateID[nEmrTemplateDetailID];
	if (NULL == pPreloadedDetail) {
		// (c.haag 2007-04-24 10:44) - PLID 25759 - This should never happen, but if an optimization fails,
		// we should go back to the non-optimized code.
		// (j.jones 2007-08-01 14:47) - PLID 26905 - we don't have an active recordset, so pass in NULL
		// (j.jones 2008-09-22 15:20) - PLID 31408 - send nEMRGroupID
		return ::LoadEMRDetailStateDefault(NULL, nEmrInfoID, nPatientID, nEMRGroupID, nEmrTemplateDetailID, lpCon, nRememberedDetailID);
	}
	CEMNDetail* pTemplateDetail = pPreloadedDetail->m_pDetail;
	CEMRInfoItem* pInfoItem = NULL;
	// (c.haag 2007-09-20 13:02) - PLID 27465 - Use Lookup() instead of []. If the lookup fails, pInfoItem will be NULL
	m_mapInfoItems.Lookup(nEmrInfoID, pInfoItem);
	if (NULL == pInfoItem) {
		// (c.haag 2007-04-24 10:44) - PLID 25759 - This should never happen, but if an optimization fails,
		// we should go back to the non-optimized code.
		// (j.jones 2007-08-01 14:47) - PLID 26905 - we don't have an active recordset, so pass in NULL
		// (j.jones 2008-09-22 15:20) - PLID 31408 - send nEMRGroupID
		return ::LoadEMRDetailStateDefault(NULL, nEmrInfoID, nPatientID, nEMRGroupID, nEmrTemplateDetailID, lpCon, nRememberedDetailID);
	}

	//TES 10/27/2004: If this is one of those items that should be remembered per patient, and if this patient already
	//has this item, then use that.
	// (c.haag 2007-01-25 08:13) - PLID 24416 - The patient ID may be -1 if this is a
	// template or there's simply no patient...in which case there is no reason to run
	// the rsExistingDetail query

	// (j.jones 2008-09-22 17:19) - PLID 31408 - supported m_bRememberForEMR, in this case we would have already
	// filled in pPreloadedDetail->m_vInfoRememberedDetailID properly earlier in the code

	if((nPatientID > 0 && pInfoItem->m_bRememberForPatient) || (nEMRGroupID != -1 && pInfoItem->m_bRememberForEMR)) {

		// (j.jones 2008-09-22 16:26) - PLID 31408 - Mow we can remember for patient
		// OR remember for EMR, where "for patient" is any previous usage of the item
		// on the patient's account, and "for EMR" looks only for usages on other EMNs
		// on the same EMR.

		if (pPreloadedDetail->m_vInfoRememberedDetailID.vt == VT_I4)
		{
			if(pPreloadedDetail->m_pDetail->IsSmartStampTable()) {
				// (z.manning 2011-02-24 18:49) - PLID 42579 - Smart stamp tables remembering is too complex for the loader
				// to handle at this time so use the global function to load its state.
				_variant_t varState;
				// (z.manning 2011-11-16 12:56) - PLID 38130 - Pass in parameter for remembered detail ID
				if(::TryLoadDetailStateFromExistingByInfoID(nEmrInfoID, pInfoItem->m_DataType, pInfoItem->m_DataSubType, nPatientID, nEMRGroupID, pInfoItem->m_bRememberForEMR, lpCon, varState, nRememberedDetailID)) {
					return varState;
				}
			}
			else {
				const long nEMNDetailID = VarLong(pPreloadedDetail->m_vInfoRememberedDetailID);
				const long nReturnedEMRInfoID = VarLong(pPreloadedDetail->m_vInfoRememberedDetailInfoID);
				// (z.manning 2011-11-16 13:03) - PLID 38130 - Set the remembered detail ID
				nRememberedDetailID = nEMNDetailID;
				if(nReturnedEMRInfoID != nEmrInfoID) {
					//if the InfoIDs differ, the load needs to map the old state to the new state
					// (z.manning 2011-02-24 18:54) - PLID 42579 - This now takes an array of detail IDs
					CArray<long,long> arynDetailIDs, arynInfoIDs;
					arynDetailIDs.Add(nEMNDetailID);
					arynInfoIDs.Add(nReturnedEMRInfoID);
					return LoadEMRDetailStateFromOldInfoID(arynDetailIDs, arynInfoIDs, nEmrInfoID, pInfoItem->m_DataType, pInfoItem->m_DataSubType, lpCon);
				}
				else {
					//if the InfoIDs are the same, then the normal load can be used
					return LoadEMRDetailState(nEMNDetailID, pInfoItem->m_DataType, lpCon);
				}
			}
		}
	}
	
	switch (pInfoItem->m_DataType) {
	case eitText:
		{
			if(nEmrTemplateDetailID == -1) {
				//TES 6/23/2004: Text types now have a default.
				CString strDefault = pInfoItem->m_strDefaultText;
				if(strDefault == "") {
					return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
				}
				else {
					return _bstr_t(strDefault);
				}
			}
			else {				
				if (pPreloadedDetail->m_vInfoDefaultText.vt == VT_NULL) {
					return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
				}
				else {
					return _bstr_t(VarString(pPreloadedDetail->m_vInfoDefaultText));
				}
			}
		}
		break;
	case eitSingleList:
		{
			if(nEmrTemplateDetailID == -1) {
				EnsureEmrInfoDefaultsMap(lpCon);
				CArray<long,long>* paDataIDs = pInfoItem->m_paDefaultDataIDs;
				long nDefault = (paDataIDs) ? paDataIDs->GetAt(0) : -1;
				if(nDefault == -1)
					return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
				else
					//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
					return _variant_t(AsString(nDefault));
			}
			else {
				// (c.haag 2007-06-26 12:11) - PLID 26459 - Load the default state for a list
				EnsureTemplateSelectMap(lpCon);
				CArray<long,long>* paDataIDs = m_mapTemplateSelect[nEmrTemplateDetailID];
				long nDefault = (paDataIDs) ? paDataIDs->GetAt(0) : -1;
				if(nDefault > -1) {
					//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
					return _variant_t(AsString(nDefault));
				}
				else {
					return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
				}
			}
			return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
		}
		break;
	case eitMultiList:
		// (j.jones 2004-10-22 14:47) - Load all the defaults into a semi-colon delimited list
		{
			if(nEmrTemplateDetailID == -1) {
				CString strDefault = "";
				EnsureEmrInfoDefaultsMap(lpCon);
				CArray<long,long>* paDataIDs = pInfoItem->m_paDefaultDataIDs;
				int nElements = (paDataIDs) ? paDataIDs->GetSize() : 0;
				for (int i=0; i < nElements; i++) {
					long nDefault = paDataIDs->GetAt(i);
					if(nDefault == -1)
						return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
					else {
						strDefault += AsString(nDefault);
						strDefault += "; ";
					}
				}
				strDefault.TrimRight("; ");
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
				return _variant_t(strDefault);
			}
			else {
				// (c.haag 2007-06-26 12:11) - PLID 26459 - Load the default state for a list
				CString strDefault = "";
				EnsureTemplateSelectMap(lpCon);
				CArray<long,long>* paDataIDs = m_mapTemplateSelect[nEmrTemplateDetailID];
				int nElements = (paDataIDs) ? paDataIDs->GetSize() : 0;
				for (int i=0; i < nElements; i++) {
					long nDefault = paDataIDs->GetAt(i);
					if(nDefault == -1)
						return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
					else {
						strDefault += AsString(nDefault);
						strDefault += "; ";
					}
				}
				strDefault.TrimRight("; ");
				//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
				return _variant_t(strDefault);
			}
			return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
		}
		break;
	case eitImage: {
		// Image type info items don't currently have default values.
		//return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
		//
		// (c.haag 2006-11-09 17:47) - PLID 23365 - Whenever we pull BackgroundImageType, we could
		// get a null value. If so, it means we pulled it from an EmrInfo item where the user must
		// choose the image to assign to the detail. In that event, we need to assign "blank default
		// values" to the image state. In our program, this means clearing the path and assigning the
		// image type of itDiagram.

		// (c.haag 2007-02-09 15:40) - PLID 23365 - The previous comment is wrong. The simple story
		// is that EmrInfoT.BackgroundImageType is NULL if there is no default image for the info
		// item. When the detail is added, it should have no image unless the detail itself is assigned
		// one in its InkImage*Override fields. The user will have to pick an image.
		//
		CEmrItemAdvImageState ais;
		if (itUndefined == (ais.m_eitImageTypeOverride = pInfoItem->m_BackgroundImageType)) {
			ais.m_strImagePathOverride.Empty();
		} else {
			ais.m_strImagePathOverride = pInfoItem->m_strBackgroundImageFilePath;
		}

		//DRT 1/23/2008 - PLID 28698 - Load the default state for hotspots
		if(nEmrTemplateDetailID == -1) {
			//nothing, leave it as-is.  We'd add code here if we made up a "default" type ability
			//	for hotspots (like the 'default' column on lists)
		}
		else {
			CString strDefault = "";
			EnsureTemplateHotSpotMap(lpCon);
			CArray<long,long>* paDataIDs = m_mapTemplateHotSpots[nEmrTemplateDetailID];
			int nElements = (paDataIDs) ? paDataIDs->GetSize() : 0;
			for (int i=0; i < nElements; i++) {
				long nDefault = paDataIDs->GetAt(i);
				strDefault += AsString(nDefault) + ";";
			}
			ais.m_strSelectedHotSpotData = strDefault;

			// (z.manning 2011-10-06 16:50) - PLID 45842 - Handle print data
			ais.m_varPrintData = pPreloadedDetail->m_vDetailPrintData;
		}

		return ais.AsSafeArrayVariant();

	   } break;
	case eitSlider:
		
		if(nEmrTemplateDetailID == -1) {
			//Sliders don't have defaults yet (though they probably should)
			return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
		}
		else {
			if (pPreloadedDetail->m_vInfoSliderValue.vt == VT_NULL) {
				return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
			}
			else {
				return pPreloadedDetail->m_vInfoSliderValue;
			}
		}
		break;
	case eitNarrative:
		{
			if(nEmrTemplateDetailID == -1) {
				//TES 6/23/2004: Text types now have a default.
				CString strDefault = pInfoItem->m_strDefaultText;
				if(strDefault == "") {
					return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
				}
				else {
					return _bstr_t(strDefault);
				}
			}
			else {
				if (pPreloadedDetail->m_vInfoDefaultText.vt == VT_NULL) {
					return LoadEMRDetailStateBlank(pInfoItem->m_DataType);
				}
				else {
					return _bstr_t(VarString(pPreloadedDetail->m_vInfoDefaultText));
				}
			}
		}
		break;
	case eitTable:
		{
			// (c.haag 2007-06-04 10:33) - PLID 25761 - We now get default table state information from
			// a preloaded map.
			CString strResult;
			EnsureTemplateTableMap(lpCon);
			strResult = m_mapTemplateTable[nEmrTemplateDetailID];
			//TES 11/7/2007 - PLID 27979 - VS2008 - Need to explicitly return a _variant_t.
			return _variant_t(strResult);
		}
		break;
	default:
		ASSERT(FALSE);
		return varNull;
		break;
	}
}

//////////////////////////////////////////////////////////////////////
// EMR Info content functions
//////////////////////////////////////////////////////////////////////
CEMNLoader::CEMRInfoItem* CEMNLoader::GetEmrInfoItem(long nEmrInfoID)
{
	//
	// (c.haag 2007-04-26 10:49) - PLID 25790 - Returns an EmrInfo object. This must be called after
	// the details have all been preloaded
	//
	// (c.haag 2007-07-03 10:23) - PLID 26523 - We now require the caller to have exclusive
	// access to preloader EMR info objects
	if (!m_mtxEMRInfo.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to EMR info objects
		// before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetEmrInfoItem was called without exclusive ownership to EMR info items");
	}
	// (c.haag 2007-09-20 13:05) - PLID 27465 - Use Lookup() instead of []. pInfoItem will be NULL if the lookup fails
	CEMRInfoItem* pInfoItem = NULL;
	m_mapInfoItems.Lookup(nEmrInfoID, pInfoItem);
	return pInfoItem;
}

//////////////////////////////////////////////////////////////////////
// Action functions
//////////////////////////////////////////////////////////////////////
MFCArray<EmrAction>* CEMNLoader::GetEMRInfoActions(CEMNDetail *pSourceDetail)
{
	//
	// (c.haag 2007-04-25 11:37) - PLID 25774 - Returns an array of EMR actions given a source detail.
	// This is usually called by CEMR::ProcessEMRInfoActions in the initial load.
	//
	// (c.haag 2007-07-03 10:23) - PLID 26523 - We now require the caller to have exclusive
	// access to preloader EMR info objects
	if (!m_mtxEMRInfo.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to EMR info actions
		// before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetEMRInfoActions was called without exclusive ownership to EMR info items");
	}
	CEMRInfoItem* pInfo = NULL;
	m_mapInfoItems.Lookup(pSourceDetail->m_nEMRInfoID, pInfo);
	if (NULL == pInfo) {
		// (c.haag 2007-04-25 10:13) - This should never happen
		ThrowNxException(FormatString("Could not find EmrInfoID %d for CEMNLoader::ProcessEMRInfoActions!", pSourceDetail->m_nEMRInfoID));
	}
	return &pInfo->m_arActions;
}

// (j.jones 2013-01-09 16:42) - PLID 54541 - this now takes in multiple data IDs and fills a provided action array
void CEMNLoader::GetEMRDataActions(IN CArray<long, long> &aryEMRDataIDs, OUT MFCArray<EmrAction> *parActions)
{
	if(parActions == NULL) {
		ThrowNxException("CEMNLoader::GetEMRDataActions was called with an invalid parActions array!");
	}

	// (c.haag 2007-07-03 10:57) - PLID 26523 - We now require the caller to have exclusive
	// access to data actions
	if (!m_mtxEMRDataActions.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to EMR data actions
		// before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetEMRDataActions was called without exclusive ownership to EMR data actions");
	}
	//
	// (c.haag 2007-04-25 11:37) - PLID 25774 - Returns an array of EMR actions given a data ID.
	// This is usually called by CEMR::ProcessEMRDataActions in the initial load.
	// (c.haag 2007-06-28 13:10) - PLID 25774 - This function should not modify the map. It now
	// has a const tag and does a lookup.
	// (j.jones 2013-01-09 16:51) - PLID 54541 - this now supports multiple data IDs
	for(int i=0; i<aryEMRDataIDs.GetSize(); i++) {
		MFCArray<EmrAction>* pResult = NULL;
		long nEMRDataID = aryEMRDataIDs.GetAt(i);
		if(m_mapActions.Lookup(nEMRDataID, pResult) && pResult != NULL) {
			for (int j=0; j < pResult->GetSize(); j++) {
				parActions->Add(pResult->GetAt(j));
			}
		}
	}
}

// (z.manning, 01/22/2008) - PLID 28690 - Returns an array of EMR actions given a hot spot ID.
MFCArray<EmrAction>* CEMNLoader::GetEMRImageHotSpotActions(CEMNDetail *pSourceDetail, long nHotSpotID)
{
	if (!m_mtxImageHotSpotActions.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to EMR hot spot actions
		// before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetEMRImageHotSpotActions was called without exclusive ownership to EMR hot spot actions");
	}

	MFCArray<EmrAction>* pResult = NULL;
	if(m_mapImageHotSpotActions.Lookup(nHotSpotID, pResult)) {
		return pResult;
	}
	else {
		return NULL;
	}
}

// (z.manning 2009-02-16 11:51) - PLID 33070 - Returns an array of EMR actions given a dropdown ID
MFCArray<EmrAction>* CEMNLoader::GetEMRTableDropdownItemActions(CEMNDetail *pSourceDetail, long nDropdownID)
{
	if(!m_mtxTableDropdownItemActions.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to EMR hot spot actions
		// before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetEMRTableDropdownItemActions was called without exclusive ownership to EMR table dropdown actions");
	}

	MFCArray<EmrAction>* paryActions = NULL;
	if(m_mapTableDropdownItemActions.Lookup(nDropdownID, paryActions)) {
		return paryActions;
	}
	else {
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Linked detail functions
//////////////////////////////////////////////////////////////////////
long CEMNLoader::GetEmrLinkedDataItemCount()
{
	//
	// (c.haag 2007-06-06 15:52) - PLID 26240 - Returns the number of elements in the
	// m_aEmrLinkedDataItems array
	//
	// (c.haag 2007-07-03 11:05) - PLID 26523 - Keep exclusive access of linked data
	// until we're done
	CHoldEMNLoaderMutex mh(&m_mtxLinkedDataItems);
	if (!m_bAllEmrItemLinkedDataItemsLoaded) {
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetEmrLinkedDataItemCount was called before loading the linked data item array!");
	}
	long nCount = m_aEmrLinkedDataItems.GetSize();
	return nCount;
}

CEMNLoader::EmrLinkedDataItem CEMNLoader::GetEmrLinkedDataItem(long nIndex)
{
	// (c.haag 2007-07-03 11:07) - PLID 26523 - We now require the caller to have exclusive
	// access to linked data items
	if (!m_mtxLinkedDataItems.HasBeenLockedInCurrentThread()) {
		// If you get this assertion, you didn't claim exclusive access to linked data objects
		// before-hand!
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetEmrLinkedDataItem was called without exclusive ownership to linked data items");
	}
	//
	// (c.haag 2007-06-06 15:52) - PLID 26240 - Returns an element in the m_aEmrLinkedDataItems
	// array
	//
	if (!m_bAllEmrItemLinkedDataItemsLoaded) {
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetEmrLinkedDataItem was called before loading the linked data item array!");
	}
	if (nIndex < 0 || nIndex >= m_aEmrLinkedDataItems.GetSize()) {
		ASSERT(FALSE);
		ThrowNxException("CEMNLoader::GetEmrLinkedDataItem was called with an invalid index!");
	}
	return m_aEmrLinkedDataItems[nIndex];
}

//////////////////////////////////////////////////////////////////////
// Internal Preloading functions
//////////////////////////////////////////////////////////////////////

void CEMNLoader::EnsureEmrInfoDefaultsMap(ADODB::_Connection *lpCon)
{
	// (c.haag 2007-07-03 10:22) - PLID 26523 - Take exclusive ownership of EMR info objects until this
	// function is done
	CHoldEMNLoaderMutex mh(&m_mtxEMRInfo);

	// (c.haag 2007-04-24 08:29) - PLID 25759 - Populates a map that associates EmrInfoID's with
	// default single/multi-select list selections. Only EmrInfoID's that exist in m_apAllDetails
	// are used in this map
	if (m_bEmrInfoDefaultsPopulated) return;

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	// (c.haag 2007-04-24 08:41) - Run the query, and populate the map. The key is EmrInfoID,
	// and the values are CArray<long,long>'s of EMRDataID's
	// (c.haag 2007-05-08 15:16) - PLID 25759 - Use BuildAllRelatedEmrInfoTQ instead of building
	// a list of EMR info ID's
	//DRT 7/18/2007 - PLID 26726 - Parameterized the query, added a function parameter to get the query parameters in order
	CSqlFragment sqlLoadSql = GetEnsureEmrInfoDefaultsMapSql();
	_RecordsetPtr prs = CreateParamRecordset(pCon, "{SQL}", sqlLoadSql);
	while (!prs->eof) {
		// (c.haag 2007-07-11 13:24) - PLID 26629 - This is now done in its own function
		ReadEmrInfoDefaultsMapRecord(prs);
		prs->MoveNext();
	}
	m_bEmrInfoDefaultsPopulated = TRUE;
}

//DRT 7/18/2007 - PLID 26726 - Parameterized the query, added a function parameter to get the query parameters in order
// (z.manning 2011-10-25 08:39) - PLID 39401 - Changed return type to SQL fragment
CSqlFragment CEMNLoader::GetEnsureEmrInfoDefaultsMapSql() const
{
	// (c.haag 2007-07-17 10:53) - PLID 26707 - This returns the SQL string to be used
	// later with ReadEmrInfoDefaultsMapRecord
	if (m_bLoadFromTemplate) {
		// (c.haag 2007-08-01 18:38) - PLID 26908 - Special handling when loading templates
		return CSqlFragment(
			"SELECT EMRInfoID, EMRDataID FROM EMRInfoDefaultsT \r\n"
			"LEFT JOIN EmrInfoMasterT ON EmrInfoMasterT.ActiveEmrInfoID = EMRInfoDefaultsT.EmrInfoID \r\n"
			"LEFT JOIN EmrTemplateDetailsT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID \r\n"
			"WHERE EMRTemplateTopicID IN ( \r\n"
			"	SELECT ID FROM EMRTemplateTopicsT WHERE TemplateID = {INT} \r\n"
			"	UNION \r\n"
			"	SELECT SourceTemplateTopicID FROM EMRTemplateTopicsT WHERE TemplateID = {INT} \r\n"
			"	) \r\n"
			, m_nMasterID, m_nMasterID);
	} else {
		//No parameters in this specific query, pass our array on to get them from the inside
		return CSqlFragment("SELECT EMRInfoID, EMRDataID FROM EMRInfoDefaultsT "
			"WHERE EmrInfoID IN ({SQL})", BuildAllRelatedEmrInfoTQ());
	}
}

void CEMNLoader::ReadEmrInfoDefaultsMapRecord(_RecordsetPtr& prs)
{
	//
	// (c.haag 2007-07-11 13:24) - PLID 26629 - This function pulls data from a query
	// which was intended to populate m_mapInfoItems with data
	//
	const long nEMRInfoID = AdoFldLong(prs, "EMRInfoID");
	const long nEMRDataID = AdoFldLong(prs, "EMRDataID");
	CArray<long,long>* paDataIDs = NULL;
	CEMRInfoItem* pInfoItem = NULL;
	if (!m_mapInfoItems.Lookup(nEMRInfoID, pInfoItem)) {
		ThrowNxException("CEMNLoader::ReadEmrInfoDefaultsMapRecord() failed to find the EMR info object!");
	}

	if (NULL == pInfoItem->m_paDefaultDataIDs) {
		pInfoItem->m_paDefaultDataIDs = new CArray<long,long>;
	}
	if (NULL != pInfoItem->m_paDefaultDataIDs) {
		pInfoItem->m_paDefaultDataIDs->Add(nEMRDataID);
	} else {
		ThrowNxException("CEMNLoader::ReadEmrInfoDefaultsMapRecord() failed to allocate pInfoItem object!");
	}
}

void CEMNLoader::EnsureSelectMap(_Connection *lpCon)
{
	//
	// (c.haag 2007-04-27 09:27) - PLID 26446 - Populates a map that associates EMR detail ID's
	// with single/multi-select list selections. Only details that exist in m_pAllDetails are used
	// in this map
	//
	if (m_bSelectMapPopulated) return;

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	// (c.haag 2007-04-27 09:29) - Run the query, and populate the map. The key is EMRDetailID,
	// and the values are CArray<long,long>'s of EMRDataID's
	// (c.haag 2007-07-17 15:05) - PLID 26708 - We now get the SQL statement from another function
	//DRT 7/18/2007 - PLID 26726 - Parameterized the query, added a function parameter to get the query parameters in order
	CDWordArray aryQueryParams;
	CString strLoadSql = GetEnsureSelectMapSql(&aryQueryParams);
	if(!strLoadSql.IsEmpty()) {
		_CommandPtr pCmd = OpenParamQuery(pCon, strLoadSql);
		for(int i = 0; i < aryQueryParams.GetSize(); i++) {
			//The text name is not used for anything
			AddParameterLong(pCmd, "ID", aryQueryParams.GetAt(i));
		}
		_RecordsetPtr prs = CreateRecordset(pCmd);

		while (!prs->eof) {
			// (c.haag 2007-07-11 13:24) - PLID 26629 - This is now done in its own function
			ReadSelectMapRecord(prs);
			prs->MoveNext();
		}
	}

	m_bSelectMapPopulated = TRUE;
}

void CEMNLoader::ReadSelectMapRecord(_RecordsetPtr& prs)
{
	//
	// (c.haag 2007-07-11 13:25) - PLID 26629 - This function pulls data from a query
	// which was intended to populate m_mapSelect with data
	//
	const long nDetailID = AdoFldLong(prs, "EMRDetailID");
	const long nEMRDataID = AdoFldLong(prs, "EMRDataID");
	CArray<long,long>* paDataIDs = NULL;
	if (!m_mapSelect.Lookup(nDetailID, paDataIDs)) {
		paDataIDs = new CArray<long,long>;
		m_mapSelect[nDetailID] = paDataIDs;
	}
	if (paDataIDs) {
		paDataIDs->Add(nEMRDataID);
	} else {
		ThrowNxException("CEMNLoader::ReadSelectMapRecord() failed to allocate paDataIDs object!");
	}
}

//DRT 7/18/2007 - PLID 26726 - Parameterized the query, added a function parameter to get the query parameters in order
CString CEMNLoader::GetEnsureSelectMapSql(CDWordArray *paryQueryParams) const
{
	// (c.haag 2007-07-17 11:04) - PLID 26707 - Returns the text for a query to be used later
	// with ReadSelectMapRecord
	// (c.haag 2007-05-08 15:13) - PLID 26446 - Use BuildAllRelatedListDetailQ instead of building
	// a list of EMR info ID's
	if (m_bLoadFromTemplate)  {
		// (c.haag 2007-08-29 15:02) - PLID 25758 - We must make sure not to double-populate the select map. 
		// Build a map of all detail ID's that already have entries in the select map, and ensure we omit them
		// from the query.
		CMap<long,long,BOOL,BOOL> mapPopulatedDetailIDs;
		GetSelectMapDetailIDs(mapPopulatedDetailIDs);

		// (c.haag 2007-07-17 11:49) - PLID 26708 - Use a different query for templates. The only
		// patient chart table details we should be loading here are ones that we're recalling
		// remembered values for. Go through all the preloaded details and build a list of all the
		// list details we're remembering
		CArray<long,long> anSelectDetailIDs;
		const int nDetails = m_apAllDetails.GetSize();
		for (int i=0; i < nDetails; i++) {
			CPreloadedDetail* pPreloadedDetail = m_apAllDetails[i];
			if (!m_bIsTemplate && VT_I4 == pPreloadedDetail->m_vInfoRememberedDetailID.vt) {
				if (eitSingleList == pPreloadedDetail->m_pDetail->m_EMRInfoType ||
					eitMultiList == pPreloadedDetail->m_pDetail->m_EMRInfoType)
				{
					const long nDetailID = VarLong(pPreloadedDetail->m_vInfoRememberedDetailID);
					BOOL bDummy;
					if (!mapPopulatedDetailIDs.Lookup(nDetailID, bDummy)) {
						anSelectDetailIDs.Add(nDetailID);
					}
				}
			}
		}
		if (0 == anSelectDetailIDs.GetSize()) {
			return "";
		} else {
			return FormatString("SELECT EMRDetailID, EMRDataID FROM EmrSelectT "
				"WHERE EMRDetailID IN (%s)", ArrayAsString(anSelectDetailIDs));
		}
	} else {
		//No specific parameters in this section, pass our array down
		return FormatString("SELECT EMRDetailID, EMRDataID FROM EmrSelectT "
			"WHERE EMRDetailID IN (%s)", BuildAllRelatedListDetailQ(paryQueryParams));
	}
}

void CEMNLoader::GetSelectMapDetailIDs(CMap<long,long,BOOL,BOOL>& mapDetails) const
{
	// (c.haag 2007-08-29 15:18) - PLID 25758 - This map populates mapDetails with ID's
	// used in the select map
	POSITION pos = m_mapSelect.GetStartPosition();
	while (pos != NULL) {
		CArray<long,long>* pDummy;
		long nDetailID;
		m_mapSelect.GetNextAssoc( pos, nDetailID, pDummy );
		mapDetails[nDetailID] = TRUE;
	}
}

void CEMNLoader::EnsureTemplateSelectMap(_Connection *lpCon)
{
	//
	// (c.haag 2007-04-24 08:30) - PLID 26459 - Populates a map that associates EMR template detail
	// ID's with default single/multi-select list selections. Only details that exist in m_apAllDetails
	// are used in this map
	//
	if (m_bTemplateSelectMapPopulated) return;

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	// (c.haag 2007-04-24 08:41) - Run the query, and populate the map. The key is EMRTemplateDetailID,
	// and the values are CArray<long,long>'s of EMRDataID's
	//DRT 7/18/2007 - PLID 26726 - Parameterized the query, added a function parameter to get the query parameters in order
	CDWordArray aryQueryParams;
	CString strLoadSql = GetEnsureTemplateSelectMapSql(&aryQueryParams);
	_CommandPtr pCmd = OpenParamQuery(pCon, strLoadSql);
	for(int i = 0; i < aryQueryParams.GetSize(); i++) {
		//The text name is not used for anything
		AddParameterLong(pCmd, "ID", aryQueryParams.GetAt(i));
	}
	_RecordsetPtr prs = CreateRecordset(pCmd);
	while (!prs->eof) {
		// (c.haag 2007-07-11 14:36) - PLID 26629 - This is now done in its own function
		ReadTemplateSelectMapRecord(prs);
		prs->MoveNext();
	}
	m_bTemplateSelectMapPopulated = TRUE;
}

void CEMNLoader::ReadTemplateSelectMapRecord(_RecordsetPtr& prs)
{
	//
	// (c.haag 2007-07-11 14:34) - PLID 26629 - This function pulls data from a query
	// which was intended to populate m_mapTemplateSelect with data
	//
	const long nTemplateDetailID = AdoFldLong(prs, "EMRTemplateDetailID");
	const long nEMRDataID = AdoFldLong(prs, "EMRDataID");
	CArray<long,long>* paDataIDs = NULL;
	if (!m_mapTemplateSelect.Lookup(nTemplateDetailID, paDataIDs)) {
		paDataIDs = new CArray<long,long>;
		m_mapTemplateSelect[nTemplateDetailID] = paDataIDs;
	}
	if (paDataIDs) {
		paDataIDs->Add(nEMRDataID);
	} else {
		ThrowNxException("CEMNLoader::ReadTemplateSelectMapRecord() failed to allocate paDataIDs object!");
	}
}

//DRT 7/18/2007 - PLID 26726 - Parameterized the query, added a function parameter to get the query parameters in order
CString CEMNLoader::GetEnsureTemplateSelectMapSql(CDWordArray *paryQueryParams) const
{
	// (c.haag 2007-07-17 11:00) - PLID 26707 - This function returns query text to be
	// later used with ReadTemplateSelectMapRecord
	//No parameters in this particular section, pass our array down
	return FormatString("SELECT EMRTemplateDetailID, EMRDataID FROM EmrTemplateSelectT "
		"WHERE EMRTemplateDetailID IN (%s)",
		// (c.haag 2007-05-08 15:13) - PLID 26459 - Use BuildAllRelatedListDetailQ instead of building
		// a list of EMR info ID's
		BuildAllRelatedListDetailQ(paryQueryParams));
}

void CEMNLoader::EnsureTableMap(ADODB::_Connection *lpCon)
{
	//
	// (c.haag 2007-06-04 17:49) - PLID 26444 - Populates a map that associated EMR detail ID's with
	// table strings. Only details that exist in m_apAllDetails are used in this map.
	//
	// You will notice that the query uses sorting. In pulling multiple details, we do not violate that
	// sorting. Detail-to-detail, the sorting is still the same. It's just now we have the details
	// interlaced about the result set. Given how we pull the data, that is a non-issue.
	//
	if (m_bTableMapPopulated) return;

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	// (c.haag 2007-06-04 17:49) - Run the query, and populate the map. The key is EMRDetailID, and the
	// values are CStrings of table state values
	// (c.haag 2007-07-17 15:05) - PLID 26708 - We now get the SQL statement from another function
	//DRT 7/18/2007 - PLID 26726 - Parameterized the query, added a function parameter to get the query parameters in order
	// (z.manning 2011-02-25 13:06) - PLID 42579 - Changed this now to make this query completely parameterized in all cases
	CSqlFragment sqlFragment = GetEnsureTableMapSql();
	if (!sqlFragment.IsEmpty()) {
		_RecordsetPtr prs = CreateParamRecordset(pCon, "{SQL}", sqlFragment);
		while (!prs->eof) {
			// (c.haag 2007-07-11 14:37) - PLID 26629 - This is now done in its own function
			ReadTableMapRecord(prs);
			prs->MoveNext();
		}
	}

	m_bTableMapPopulated = TRUE;
}

void CEMNLoader::ReadTableMapRecord(_RecordsetPtr& prs)
{
	//
	// (c.haag 2007-07-11 14:37) - PLID 26629 - This function pulls data from a query
	// which was intended to populate m_mapTable with data
	//
	const long nEmrDetailID = AdoFldLong(prs, "EMRDetailID");
	const long X = AdoFldLong(prs, "EMRDataID_X", -1);
	const long Y = AdoFldLong(prs, "EMRDataID_Y");
	const CString strData = AdoFldString(prs, "Data", "");
	// (z.manning 2010-02-18 09:33) - PLID 37427 - EmrDetailImageStampID
	const long nEmrDetailImageStampID = AdoFldLong(prs->GetFields(), "EmrDetailImageStampID", -1);
	// (z.manning 2011-03-02 15:00) - PLID 42335 - Added stamp ID
	const long nStampID = AdoFldLong(prs, "EmrImageStampID", -1);
	// (c.haag 2007-08-18 09:00) - PLID 27111 - Append the table element
	// corresponding to X,Y,strData to strCurrent
	AppendTableStateWithUnformattedElement(m_mapTable[nEmrDetailID], X, Y, strData, nEmrDetailImageStampID, NULL, nStampID);
}

//DRT 7/18/2007 - PLID 26726 - Parameterized the query, added a function parameter to get the query parameters in order
// (z.manning 2011-02-25 13:01) - PLID 42597 - Changed the return type to be a SQL fragment
CSqlFragment CEMNLoader::GetEnsureTableMapSql() const
{
	// (c.haag 2007-07-17 11:01) - PLID 26707 - Returns the SQL statement for a query to be used
	// later with ReadTableMapRecord
	// (c.haag 2007-06-27 12:27) - PLID 26473 - Filter out undeleted details with deleted topics
	if (m_bLoadFromTemplate)  {
		// (c.haag 2007-08-29 15:02) - PLID 25758 - We must make sure not to double-populate the table map. 
		// Build a map of all detail ID's that already have entries in the table map, and ensure we omit them
		// from the query.
		CMap<long,long,BOOL,BOOL> mapPopulatedDetailIDs;
		GetTableMapDetailIDs(mapPopulatedDetailIDs);

		// (c.haag 2007-07-17 11:36) - PLID 26708 - Use a different query for templates. The only
		// patient chart table details we should be loading here are ones that we're recalling
		// remembered values for. Go through all the preloaded details and build a list of all the
		// table details we're remembering
		CArray<long,long> anTableDetailIDs;
		const int nDetails = m_apAllDetails.GetSize();
		for (int i=0; i < nDetails; i++) {
			CPreloadedDetail* pPreloadedDetail = m_apAllDetails[i];
			if (!m_bIsTemplate && VT_I4 == pPreloadedDetail->m_vInfoRememberedDetailID.vt) {
				if (eitTable == pPreloadedDetail->m_pDetail->m_EMRInfoType)
				{
					const long nDetailID = VarLong(pPreloadedDetail->m_vInfoRememberedDetailID);
					BOOL bDummy;
					if (!mapPopulatedDetailIDs.Lookup(nDetailID, bDummy)) {
						anTableDetailIDs.Add(nDetailID);
					}
				}
			}
		}
		if (0 == anTableDetailIDs.GetSize()) {
			return CSqlFragment();
		} 
		else {
			// (z.manning 2010-02-18 09:32) - PLID 37427 - Added EmrDetailImageStampID
			// (z.manning 2010-02-18 16:19) - PLID 37412 - We need to left join the EmrDataT table for rows as tables rows
			// are no longer required to have an EmrDataID
			// (z.manning 2011-03-02 15:15) - PLID 42335 - Added EmrImageStampID
			return CSqlFragment(
				"SELECT EMRDetailsT.ID AS EMRDetailID, EMRDataID_X, EMRDataID_Y, EMRDetailTableDataT.Data, EmrDetailImageStampID "
				"	, EmrDetailImageStampsT.EmrImageStampID "
				"FROM EMRDetailTableDataT "
				"LEFT JOIN EmrDataT RowData ON EMRDetailTableDataT.EMRDataID_X = RowData.ID "
				"INNER JOIN EmrDataT ColumnData ON EMRDetailTableDataT.EMRDataID_Y = ColumnData.ID "
				"LEFT JOIN EmrDetailsT ON EmrDetailsT.ID = EMRDetailTableDataT.EMRDetailID "
				"LEFT JOIN EmrInfoT ON EmrInfoT.ID = EmrDetailsT.EmrInfoID "
				"LEFT JOIN EmrTopicsT ON EmrTopicsT.ID = EmrDetailsT.EmrTopicID "
				"LEFT JOIN EmrDetailImageStampsT ON EmrDetailTableDataT.EmrDetailImageStampID = EmrDetailImageStampsT.ID AND EmrDetailImageStampsT.Deleted = 0 "
				"WHERE EMRDetailsT.ID IN ({INTARRAY}) AND EMRDetailsT.Deleted = 0 "
				"AND EMRTopicsT.Deleted = 0 "
				"ORDER BY EmrDetailsT.ID "
				// (z.manning 2011-02-25 13:00) - PLID 42579 - No need to order by content when loading a table's state.
				, anTableDetailIDs);
		}
	}
	else {
		// (c.haag 2007-09-18 14:24) - PLID 27424 - This query is significantly faster than the one above. Unlike the one above,
		// we use a table variable. This alters the execution plan, at least in SQL 2005 express, to become much more efficient.
		// (z.manning 2010-02-18 09:32) - PLID 37427 - Added EmrDetailImageStampID
		// (z.manning 2010-02-18 16:19) - PLID 37412 - We need to left join the EmrDataT table for rows as tables rows
		// are no longer required to have an EmrDataID
		// (z.manning 2011-03-02 14:58) - PLID 42335 - Added image stamp ID
		return CSqlFragment("SET NOCOUNT ON "
			"DECLARE @tDetailIDsT TABLE (ID INT NOT NULL PRIMARY KEY) "
			"INSERT INTO @tDetailIDsT (ID) SELECT ID FROM EMRDetailsT WHERE EMRID = {INT} AND DELETED = 0 "
			"SET NOCOUNT OFF "
			"SELECT EMRDetailsT.ID AS EMRDetailID, EMRDataID_X, EMRDataID_Y, EMRDetailTableDataT.Data, EmrDetailImageStampID "
			"	, EmrDetailImageStampsT.EmrImageStampID "
			"FROM EMRDetailTableDataT "
			"LEFT JOIN EmrDataT RowData ON EMRDetailTableDataT.EMRDataID_X = RowData.ID "
			"INNER JOIN EmrDataT ColumnData ON EMRDetailTableDataT.EMRDataID_Y = ColumnData.ID "
			"LEFT JOIN EmrDetailImageStampsT ON EmrDetailTableDataT.EmrDetailImageStampID = EmrDetailImageStampsT.ID AND EmrDetailImageStampsT.Deleted = 0 "
			"LEFT JOIN EmrDetailsT ON EmrDetailsT.ID = EMRDetailTableDataT.EMRDetailID LEFT JOIN EmrInfoT ON EmrInfoT.ID = EmrDetailsT.EmrInfoID "
			"LEFT JOIN EmrTopicsT ON EmrTopicsT.ID = EmrDetailsT.EmrTopicID WHERE EmrInfoT.DataType = {CONST} AND EMRTopicsT.Deleted = 0 "
			"AND EMRDetailTableDataT.EmrDetailID IN (SELECT ID FROM @tDetailIDsT) "
			"ORDER BY EMRDetailTableDataT.EmrDetailID "
			// (z.manning 2011-02-25 13:18) - PLID 42579 - We don't need to load in order of content when loading a table's state.
			, m_nMasterID, eitTable);
	}
}

void CEMNLoader::GetTableMapDetailIDs(CMap<long,long,BOOL,BOOL>& mapDetails) const
{
	// (c.haag 2007-08-29 15:18) - PLID 25758 - This map populates mapDetails with ID's
	// used in the table map
	POSITION pos = m_mapTable.GetStartPosition();
	while (pos != NULL) {
		CString strDummy;
		long nDetailID;
		m_mapTable.GetNextAssoc( pos, nDetailID, strDummy );
		mapDetails[nDetailID] = TRUE;
	}
}

void CEMNLoader::EnsureTemplateTableMap(ADODB::_Connection *lpCon)
{
	//
	// (c.haag 2007-04-24 09:58) - PLID 25761 - Populates a map that associates EMR template detail
	// ID's with default table strings. Only details that exist in m_apAllDetails are used in this map.
	//
	// You will notice that the query uses sorting. In pulling multiple details, we do not violate that
	// sorting. Detail-to-detail, the sorting is still the same. It's just now we have the details
	// interlaced about the result set. Given how we pull the data, that is a non-issue.
	//
	if (m_bTemplateTableMapPopulated) return;

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	// (c.haag 2007-04-24 08:41) - Run the query, and populate the map. The key is EMRTemplateDetailID,
	// and the values are CString's of table state values
	//DRT 7/18/2007 - PLID 26726 - Parameterized the query, added a function parameter to get the query parameters in order
	CDWordArray aryQueryParams;
	CString strLoadSql = GetEnsureTemplateTableMapSql(&aryQueryParams);
	_CommandPtr pCmd = OpenParamQuery(pCon, strLoadSql);
	for(int i = 0; i < aryQueryParams.GetSize(); i++) {
		//The text name is not used for anything
		AddParameterLong(pCmd, "ID", aryQueryParams.GetAt(i));
	}
	_RecordsetPtr prs = CreateRecordset(pCmd);

	while (!prs->eof) {
		// (c.haag 2007-07-11 14:38) - PLID 26629 - This is now done in its own function
		ReadTemplateTableMapRecord(prs);
		prs->MoveNext();
	}

	m_bTemplateTableMapPopulated = TRUE;
}

void CEMNLoader::ReadTemplateTableMapRecord(_RecordsetPtr& prs)
{
	//
	// (c.haag 2007-07-11 14:38) - PLID 26629 - This function pulls data from a query
	// which was intended to populate m_mapTemplateTable with data
	//
	const long nEmrTemplateDetailID = AdoFldLong(prs, "EMRTemplateDetailID");
	const long X = AdoFldLong(prs, "EMRDataID_X");
	const long Y = AdoFldLong(prs, "EMRDataID_Y");
	const CString strData = AdoFldString(prs, "Data", "");
	// (c.haag 2007-08-18 09:00) - PLID 27111 - Append the table element
	// corresponding to X,Y,strData to strCurrent
	// (z.manning 2010-02-18 09:35) - PLID 37427 - Use -1 for EmrDetailImageStampID since template details
	// do not save their stamping.
	// (z.manning 2011-03-02 15:02) - PLID 42335 - Same idea for the global stamp ID
	AppendTableStateWithUnformattedElement(m_mapTemplateTable[nEmrTemplateDetailID], X, Y, strData, -1, NULL, -1);
}

//DRT 7/18/2007 - PLID 26726 - Parameterized the query, added a function parameter to get the query parameters in order
CString CEMNLoader::GetEnsureTemplateTableMapSql(CDWordArray *paryQueryParams) const
{
	// (c.haag 2007-07-17 10:58) - PLID 26707 - We call this function to get the query
	// to be used later with ReadTemplateTableMapRecord
	//No specific parameters in this section of the query, pass our array down
	return FormatString("SELECT EMRTemplateDetailID, EMRDataID_X, EMRDataID_Y, EMRTemplateTableDefaultsT.Data FROM EMRTemplateTableDefaultsT "
		"INNER JOIN EmrDataT RowData ON EMRTemplateTableDefaultsT.EMRDataID_X = RowData.ID "
		"INNER JOIN EmrDataT ColumnData ON EMRTemplateTableDefaultsT.EMRDataID_Y = ColumnData.ID "
		"INNER JOIN EmrTemplateDetailsT ON EmrTemplateDetailsT.ID = EMRTemplateTableDefaultsT.EMRTemplateDetailID "
		"INNER JOIN EmrInfoMasterT ON EmrTemplateDetailsT.EmrInfoMasterID = EmrInfoMasterT.ID "
		"INNER JOIN EmrInfoT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
		"WHERE EMRTemplateDetailID IN (%s) "
		"ORDER BY (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN EmrInfoT.DataSubType IN (1,2) "
		"AND RowData.ListType <> 2 THEN RowData.SortOrder ELSE -1 END ELSE RowData.SortOrder END), 	"
		"(CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN CASE WHEN EmrInfoT.DataSubType IN (1,2) AND RowData.ListType <> 2 THEN ''  "
		"ELSE RowData.Data END ELSE '' END), (CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN -1 ELSE ColumnData.SortOrder END), "
		"(CASE WHEN EmrInfoT.AutoAlphabetizeListData = 1 THEN ColumnData.Data ELSE '' END) "
		,
		// (c.haag 2007-05-08 15:13) - PLID 25761 - Use BuildAllRelatedTableDetailQ instead of building
		// a list of ID's
		BuildAllRelatedTableDetailQ(paryQueryParams));
}

// (j.jones 2008-09-22 16:13) - PLID 31408 - added nEMRGroupID
void CEMNLoader::EnsureRememberCandidateMaps(long nPatientID, long nEMRGroupID, ADODB::_Connection *lpCon)
{
	//
	// (c.haag 2007-04-25 14:00) - PLID 25758 - Populates a map of possible candidate details for
	// "Remember this patient's value" querying. We used to run queries to figure out what detail
	// states to load this information, but they were extremely slow. We now use a different
	// method which consists of pulling all patient details for all EMN's, and populating a map of 
	// details that can qualify as the ones to load states from. This is normally a horrifying idea,
	// but the more complicated we make a query and the more tables that reference it, the slower
	// it is going to run. Multiply that by every detail loaded in for the current chart (which can
	// be 1000 or more), and you have big problems. This method, on the other hand, is a relatively
	// simple blast of information that we load into memory. This is not good for network traffic,
	// but this is a case where client-side CPU processing can outperform server-side data processing.
	// This is called from PreloadEmrTemplateDetails in the main thread.
	//
	if (m_bRememberCandidateMapsPopulated) {
		return;
	}

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	// These are all of the undeleted details of all patient charts that have the Remember This
	// Item flag checked
	// (c.haag 2007-06-26 10:21) - PLID 25758 - Added a filter on RememberForPatient, and
	// made into a parameter query
	// (c.haag 2007-06-27 12:27) - PLID 26473 - Filter out undeleted details with deleted topics

	// (j.jones 2008-09-22 15:05) - PLID 31408 - need to also track the EMRGroupID

	//TES 11/5/2008 - PLID 31926 - This needs to hard-code the system tables to be "RememberForPatient" 
	// (and NOT "RememberForEMR"), because CEMRItemEntryDlg hardcodes them that way even though the data doesn't always
	// reflect it.  This should have been done in the course of PLID 29416 back in June, but I managed to overlook this
	// query.
	// (j.jones 2010-02-26 10:32) - PLID 37231 - fixed so we loaded the "remember" setting from the current version of the item,
	// which is what would be added to the new EMN, as opposed to loading the setting from the version of the item previously saved
	// (j.jones 2012-10-09 15:51) - PLID 53089 - EMRMasterT.Date is supposed to have no time value, and our remembering logic
	// depends on that. Force the date to load as date-only, to fix any cases where a time may have accidentally been saved.
	// (a.walling 2013-03-20 16:09) - PLID 55790 - Unrelated but not bothering reverting this just yet. Using const_int for daatasubtype params
	_RecordsetPtr prs = CreateParamRecordset("SELECT EmrDetailsT.ID, EmrInfoT.EmrInfoMasterID, EmrMasterT.EMRGroupID, EmrInfoT.DataType, "
		"dbo.AsDateNoTime(EmrMasterT.Date) AS EmrMasterDate, EmrMasterT.ID AS EmrMasterID, EmrInfoT.ID AS EmrInfoID, "
		"CASE WHEN EmrInfoT.DataSubType IN ({CONST_INT}, {CONST_INT}) THEN convert(bit,1) "
		"	ELSE EmrInfoT_Master.RememberForPatient END AS RememberForPatient, "
		"CASE WHEN EmrInfoT.DataSubType IN ({CONST_INT}, {CONST_INT}) THEN convert(bit,0) "
		"	ELSE EmrInfoT_Master.RememberForEMR END AS RememberForEMR "
		"FROM EmrDetailsT "
		"INNER JOIN EmrTopicsT ON EmrTopicsT.ID = EmrDetailsT.EmrTopicID "
		"INNER JOIN EmrMasterT ON EmrDetailsT.EmrID = EmrMasterT.ID "
		"INNER JOIN EmrInfoT ON EmrInfoT.ID = EmrDetailsT.EMRInfoID "
		"INNER JOIN EMRInfoMasterT ON EMRInfoT.EMRInfoMasterID = EMRInfoMasterT.ID "
		"INNER JOIN EMRInfoT EmrInfoT_Master ON EMRInfoMasterT.ActiveEMRInfoID = EmrInfoT_Master.ID "
		"WHERE PatientID = {INT} AND EMRDetailsT.Deleted = 0 AND EMRMasterT.Deleted = 0 "
		"AND EmrTopicsT.Deleted = 0 "
		"AND (CASE WHEN EmrInfoT.DataSubType IN ({CONST_INT}, {CONST_INT}) THEN convert(bit,1) "
		"	ELSE EmrInfoT_Master.RememberForPatient END = 1 "
		"OR CASE WHEN EmrInfoT.DataSubType IN ({CONST_INT}, {CONST_INT}) THEN convert(bit,0) "
		"	ELSE EmrInfoT_Master.RememberForEMR END = 1)",
		eistCurrentMedicationsTable, eistAllergiesTable, eistCurrentMedicationsTable, eistAllergiesTable, 
		nPatientID, eistCurrentMedicationsTable, eistAllergiesTable, eistCurrentMedicationsTable, eistAllergiesTable);

	FieldsPtr f = prs->Fields;
	
	while (!prs->eof) {		
		CRememberCandidate data;
		// The criteria for a detail qualifying to have the remembered state of another is
		// on the EmrInfoMasterID, and DataType. I found that formatting them into a string		
		// is easier than trying to make my own key object.
		data.m_nDetailID = AdoFldLong(f, "ID");
		data.m_dtEMRMaster = AdoFldDateTime(f, "EmrMasterDate");
		data.m_nEMRMasterID = AdoFldLong(f, "EmrMasterID");
		data.m_nEMRGroupID = AdoFldLong(f, "EMRGroupID");
		data.m_nEMRInfoID = AdoFldLong(f, "EmrInfoID");

		BOOL bRememberForPatient = AdoFldBool(f, "RememberForPatient");
		BOOL bRememberForEMR = AdoFldBool(f, "RememberForEMR");

		CRememberCandidate* pValue = NULL;
		BOOL bUseNewData = FALSE;

		// (j.jones 2008-09-22 16:25) - PLID 31408 - now we have two maps, and two key types
		if(bRememberForPatient) {
			CString strPatientKey;		
			strPatientKey.Format("%d#%d", AdoFldLong(f, "EmrInfoMasterID"), AdoFldByte(f, "DataType"));

			// If multiple details were candidates for a single remembering detail, then the
			// priorities in order are: Newest chart date, newest chart ID, newest detail ID		
			if (!m_mapRememberForPatientCandidates.Lookup(strPatientKey, (LPVOID&)pValue)) {
				m_mapRememberForPatientCandidates[strPatientKey] = pValue = new CRememberCandidate;
				bUseNewData = TRUE;
			}
			else {
				// (j.jones 2012-10-09 15:51) - PLID 53089 - EMRMasterT.Date is supposed to have no time value, and our remembering logic
				// depends on that. Our query earlier in this function forced the date to load as date-only, to fix any cases where a
				// time may have accidentally been saved.
				if (data.m_dtEMRMaster > pValue->m_dtEMRMaster) {
					bUseNewData = TRUE;
				}
				else if (data.m_dtEMRMaster == pValue->m_dtEMRMaster) {
					if (data.m_nEMRMasterID > pValue->m_nEMRMasterID)	{
						bUseNewData = TRUE;
					}
					else if (data.m_nEMRMasterID == pValue->m_nEMRMasterID) {
						if (data.m_nDetailID > pValue->m_nDetailID)	{
							bUseNewData = TRUE;
						}
					}
				}
			}
		}
		else if(bRememberForEMR) {
			CString strEMRKey;
			strEMRKey.Format("%d#%d#%d", AdoFldLong(f, "EmrInfoMasterID"), AdoFldLong(f, "EMRGroupID"), AdoFldByte(f, "DataType"));

			// If multiple details were candidates for a single remembering detail, then the
			// priorities in order are: Newest chart date, newest chart ID, newest detail ID		
			if (!m_mapRememberForEMRCandidates.Lookup(strEMRKey, (LPVOID&)pValue)) {
				m_mapRememberForEMRCandidates[strEMRKey] = pValue = new CRememberCandidate;
				bUseNewData = TRUE;
			}
			else {
				// (j.jones 2012-10-09 15:51) - PLID 53089 - EMRMasterT.Date is supposed to have no time value, and our remembering logic
				// depends on that. Our query earlier in this function forced the date to load as date-only, to fix any cases where a
				// time may have accidentally been saved.
				if (data.m_dtEMRMaster > pValue->m_dtEMRMaster) {
					bUseNewData = TRUE;
				}
				else if (data.m_dtEMRMaster == pValue->m_dtEMRMaster) {
					if (data.m_nEMRMasterID > pValue->m_nEMRMasterID)	{
						bUseNewData = TRUE;
					}
					else if (data.m_nEMRMasterID == pValue->m_nEMRMasterID) {
						if (data.m_nDetailID > pValue->m_nDetailID)	{
							bUseNewData = TRUE;
						}
					}
				}
			}
		}

		// If we get here, replace the existing values with newly read in ones, because
		// the values we just read in are superior to what we were going to remember before
		if (bUseNewData) {
			pValue->m_nDetailID = data.m_nDetailID;
			pValue->m_dtEMRMaster = data.m_dtEMRMaster;
			pValue->m_nEMRMasterID = data.m_nEMRMasterID;
			pValue->m_nEMRGroupID = data.m_nEMRGroupID;
			pValue->m_nEMRInfoID = data.m_nEMRInfoID;
		}
		prs->MoveNext();
	}
	m_bRememberCandidateMapsPopulated = TRUE;
}

void CEMNLoader::EnsureRememberedDetailMap(ADODB::_Connection *lpCon)
{
	//
	// (c.haag 2007-04-24 13:39) - PLID 25758 - Populates a map that associated EMN detail values with
	// preloaded detail objects. This map is only populated when loading a template with "Remember for
	// this patient" details where we need to load additional detail data when calling LoadEMRDetailState
	//
	if (m_bRememberedDetailMapPopulated) return;

	// (c.haag 2007-08-29 15:02) - We must make sure not to double-populate the maps. Build a map of
	// all detail ID's that already have entries in the maps, and ensure we omit them from the query.
	CMap<long,long,BOOL,BOOL> mapPopulatedDetailIDs;
	GetSelectMapDetailIDs(mapPopulatedDetailIDs);
	GetTableMapDetailIDs(mapPopulatedDetailIDs);
	//DRT 1/23/2008 - PLID 28698
	GetHotSpotMapDetailIDs(mapPopulatedDetailIDs);


	// (c.haag 2007-04-24 13:41) - We've already filled the m_mapRememberedDetails with keys; we just
	// need to populate it with values.
	CString strDetailIDs;
	POSITION pos = m_mapRememberedDetails.GetStartPosition();
	while (pos != NULL) {
		CPreloadedDetail* pDetail;
		BOOL bDummy;
		long nDetailID;
		m_mapRememberedDetails.GetNextAssoc( pos, nDetailID, pDetail );
		if (!mapPopulatedDetailIDs.Lookup(nDetailID, bDummy)) {
			strDetailIDs += AsString(nDetailID) + ",";
		}
	}
	strDetailIDs.TrimRight(",");

	if (!strDetailIDs.IsEmpty()) {
		_ConnectionPtr pCon;
		if(lpCon) pCon = lpCon;
		else pCon = GetRemoteData();
		_variant_t vrs;
		// (z.manning 2011-02-14 12:49) - PLID 42452 - Parameterized
		// (z.manning 2011-10-05 16:59) - PLID 45842 - Added PrintData
		_RecordsetPtr prs = CreateParamRecordset(pCon, 
			// (c.haag 2007-06-26 11:09) - PLID 25758 - General detail data
			"SELECT ID, Text, InkData, InkImagePathOverride, InkImageTypeOverride, ImageTextData, EmrDetailsT.PrintData, SliderValue "
			// (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore zoom and pan offsets.
			//"ZoomLevel, OffsetX, OffsetY "
			"FROM EMRDetailsT WHERE ID IN ({INTSTRING});\r\n"
			// (c.haag 2007-06-26 11:10) - PLID 25758 - Single and multi-select list data
			"SELECT EMRDetailID, EMRDataID FROM EmrSelectT WHERE EMRDetailID IN ({INTSTRING});\r\n"
			// (c.haag 2007-06-26 11:10) - PLID 25758 - Table data
			// (z.manning 2010-02-18 09:43) - PLID 37427 - Added EmrDetailImageStampID
			// (j.jones 2010-02-25 15:48) - PLID 37231 - We need to left join the EmrDataT table for rows as tables rows
			// are no longer required to have an EmrDataID
			// (z.manning 2011-03-02 15:03) - PLID 42335 - Added global stamp ID
			"SELECT EMRDetailsT.ID AS EMRDetailID, EMRDataID_X, EMRDataID_Y, EMRDetailTableDataT.Data, EmrDetailImageStampID "
			"	, EmrDetailImageStampsT.EmrImageStampID "
			"FROM EMRDetailTableDataT "
			"LEFT JOIN EmrDataT RowData ON EMRDetailTableDataT.EMRDataID_X = RowData.ID "
			"INNER JOIN EmrDataT ColumnData ON EMRDetailTableDataT.EMRDataID_Y = ColumnData.ID "
			"LEFT JOIN EmrDetailsT ON EmrDetailsT.ID = EMRDetailTableDataT.EMRDetailID "
			"LEFT JOIN EmrDetailImageStampsT ON EmrDetailTableDataT.EmrDetailImageStampID = EmrDetailImageStampsT.ID "
			"LEFT JOIN EmrInfoT ON EmrInfoT.ID = EmrDetailsT.EmrInfoID "
			"WHERE EmrInfoT.DataType = {CONST} AND EmrDetailsT.ID IN ({INTSTRING}) "
			"ORDER BY (CASE WHEN AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND RowData.ListType <> 2 THEN RowData.SortOrder ELSE -1 END ELSE RowData.SortOrder END), "
			" 	(CASE WHEN AutoAlphabetizeListData = 1 THEN CASE WHEN DataSubType IN (1,2) AND RowData.ListType <> 2 THEN '' ELSE RowData.Data END ELSE '' END), "
			"	(CASE WHEN AutoAlphabetizeListData = 1 THEN -1 ELSE ColumnData.SortOrder END), "
			"	(CASE WHEN AutoAlphabetizeListData = 1 THEN ColumnData.Data ELSE '' END); "
			//DRT 1/23/2008 - PLID 28698 - HotSpot data
			"SELECT EMRDetailID, EMRImageHotSpotID FROM EMRHotSpotSelectT WHERE EMRDetailID IN ({INTSTRING});\r\n"
			// (j.jones 2010-02-25 15:23) - PLID 37231 - load EMRDetailImageStampsT
			// (z.manning 2011-02-14 12:36) - PLID 42452 - Need to load in order by OrderIndex
			// (z.manning 2011-01-27 16:27) - PLID 42335 - Added UsedInTableData
			// (z.manning 2011-09-08 10:22) - PLID 45335 - Added 3D fields
			"SELECT EmrDetailImageStampsT.ID, EmrDetailImageStampsT.EmrDetailID, EmrDetailImageStampsT.EmrImageStampID, "
			"	EmrDetailImageStampsT.OrderIndex, EmrDetailImageStampsT.SmartStampTableSpawnRule, "
			"	EmrDetailImageStampsT.XPos, EmrDetailImageStampsT.YPos "
			"	, CONVERT(bit, CASE WHEN EmrDetailTableDataQ.EmrDetailImageStampID IS NULL THEN 0 ELSE 1 END) AS UsedInTableData \r\n"
			"	, XPos3D, YPos3D, ZPos3D, XNormal, YNormal, ZNormal, HotSpot3D \r\n"
			"FROM EmrDetailImageStampsT \r\n"
			"LEFT JOIN (SELECT DISTINCT EmrDetailTableDataT.EmrDetailImageStampID FROM EmrDetailTableDataT WHERE EmrDetailTableDataT.EmrDetailImageStampID IS NOT NULL) EmrDetailTableDataQ ON EmrDetailImageStampsT.ID = EmrDetailTableDataQ.EmrDetailImageStampID \r\n"
			"WHERE EmrDetailImageStampsT.Deleted = 0 AND EmrDetailImageStampsT.EmrDetailID IN ({INTSTRING}) \r\n"
			"ORDER BY EmrDetailImageStampsT.OrderIndex \r\n"
			, strDetailIDs
			, strDetailIDs
			, eitTable, strDetailIDs
			, strDetailIDs, strDetailIDs);

		FieldsPtr f = prs->Fields;
		while (!prs->eof) {
			CPreloadedDetail* pPreloadedDetail = new CPreloadedDetail;
			const long nID = VarLong(f->Item["ID"]->Value);
			pPreloadedDetail->m_vDetailText = f->Item["Text"]->Value;
			pPreloadedDetail->m_vDetailImageTextData = f->Item["ImageTextData"]->Value;
			pPreloadedDetail->m_vDetailPrintData = f->Item["PrintData"]->Value; // (z.manning 2011-10-05 17:12) - PLID 45842
			pPreloadedDetail->m_vDetailInkData = f->Item["InkData"]->Value;

			// (r.gonet 2013-04-08 17:48) - PLID 56150 - Technically, these serialized fields should all be NULL if there is no data for them but
			//  ensure that we handle cases of bad data where the field is an empty byte array.
			if(pPreloadedDetail->m_vDetailImageTextData.vt != VT_NULL && GetElementCountFromSafeArrayVariant(pPreloadedDetail->m_vDetailImageTextData) == 0) {
				pPreloadedDetail->m_vDetailImageTextData = g_cvarNull;
			}
			if(pPreloadedDetail->m_vDetailPrintData.vt != VT_NULL && GetElementCountFromSafeArrayVariant(pPreloadedDetail->m_vDetailPrintData) == 0) {
				pPreloadedDetail->m_vDetailPrintData = g_cvarNull;
			}
			if(pPreloadedDetail->m_vDetailInkData.vt != VT_NULL && GetElementCountFromSafeArrayVariant(pPreloadedDetail->m_vDetailInkData) == 0) {
				pPreloadedDetail->m_vDetailInkData = g_cvarNull;
			}

			pPreloadedDetail->m_vDetailInkImagePathOverride = f->Item["InkImagePathOverride"]->Value;
			pPreloadedDetail->m_vDetailInkImageTypeOverride = f->Item["InkImageTypeOverride"]->Value;
			/* (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore zoom and pan offsets.
			pPreloadedDetail->m_vDetailZoomLevel = f->Item["ZoomLevel"]->Value;
			pPreloadedDetail->m_vDetailOffsetX = f->Item["OffsetX"]->Value;
			pPreloadedDetail->m_vDetailOffsetY = f->Item["OffsetY"]->Value;*/
			pPreloadedDetail->m_vDetailSliderValue = f->Item["SliderValue"]->Value;
			m_mapRememberedDetails[nID] = pPreloadedDetail;
			prs->MoveNext();
		}

		// (c.haag 2007-06-26 10:50) - PLID 25758 - Now load information necessary for creating list states
		prs = prs->NextRecordset(&vrs);
		f = prs->Fields;
		while (!prs->eof) {
			const long nDetailID = AdoFldLong(prs, "EMRDetailID");
			CPreloadedDetail* pPreloadedDetail = m_mapRememberedDetails[nDetailID];
			if (NULL != pPreloadedDetail) {
				const long nEMRDataID = AdoFldLong(prs, "EMRDataID");
				CArray<long,long>* paDataIDs = NULL;
				if (!m_mapSelect.Lookup(nDetailID, paDataIDs)) {
					paDataIDs = new CArray<long,long>;
					m_mapSelect[nDetailID] = paDataIDs;
				}
				if (paDataIDs) {
					paDataIDs->Add(nEMRDataID);
				} else {
					ThrowNxException("EnsureRememberedDetailMap() failed to allocate paDataIDs object!");
				}

			} else {
				// This should never happen
				ThrowNxException("Failed to find preloaded remembered detail in EnsureRememberedDetailMap()!");
			}
			prs->MoveNext();
		}

		// (c.haag 2007-06-26 10:55) - PLID 25758 - Now load information necessary for creating table states
		prs = prs->NextRecordset(&vrs);
		f = prs->Fields;
		while (!prs->eof) {
			const long nEmrDetailID = AdoFldLong(prs, "EMRDetailID");
			// (z.manning 2010-02-18 13:52) - PLID 37404 - EmrDataID_X is now nullable so set the default as -1
			const long X = AdoFldLong(prs, "EMRDataID_X", -1);
			const long Y = AdoFldLong(prs, "EMRDataID_Y");
			const CString strData = AdoFldString(prs, "Data", "");
			const long nEmrDetailImageStampID = AdoFldLong(prs->GetFields(), "EmrDetailImageStampID", -1);
			// (z.manning 2011-03-02 15:04) - PLID 42335 - Added global stamp ID
			const long nStampID = AdoFldLong(prs, "EmrImageStampID", -1);
			// (c.haag 2007-08-18 09:00) - PLID 27111 - Append the table element
			// corresponding to X,Y,strData to strState
			AppendTableStateWithUnformattedElement(m_mapTable[nEmrDetailID], X, Y, strData, nEmrDetailImageStampID, NULL, nStampID);
			prs->MoveNext();
		}

		//DRT 1/23/2008 - PLID 28698 - Next query is for hot spots
		prs = prs->NextRecordset(&vrs);
		f = prs->Fields;
		while(!prs->eof) {
			const long nDetailID = AdoFldLong(f, "EMRDetailID");
			CPreloadedDetail* pPreloadedDetail = m_mapRememberedDetails[nDetailID];
			if (NULL != pPreloadedDetail) {
				const long nEMRHotSpotID = AdoFldLong(f, "EMRImageHotSpotID");
				CArray<long,long>* paDataIDs = NULL;
				if (!m_mapHotSpots.Lookup(nDetailID, paDataIDs)) {
					paDataIDs = new CArray<long,long>;
					m_mapHotSpots[nDetailID] = paDataIDs;
				}
				if (paDataIDs) {
					paDataIDs->Add(nEMRHotSpotID);
				} else {
					ThrowNxException("EnsureRememberedDetailMap() failed to allocate paDataIDs object for images!");
				}
			} else {
				// This should never happen
				ThrowNxException("Failed to find preloaded remembered image detail in EnsureRememberedDetailMap()!");
			}
			prs->MoveNext();

		}
		
		// (j.jones 2010-02-25 15:23) - PLID 37231 - load EMRDetailImageStampsT
		prs = prs->NextRecordset(&vrs);
		f = prs->Fields;
		while(!prs->eof) {
			const long nDetailID = AdoFldLong(f, "EMRDetailID");
			CPreloadedDetail* pPreloadedDetail = m_mapRememberedDetails[nDetailID];
			if (NULL != pPreloadedDetail) {
				//add the image stamp			
				pPreloadedDetail->m_arypDetailImageStamps.Add(new EmrDetailImageStamp(f));
			} else {
				// This should never happen
				ThrowNxException("Failed to find preloaded remembered image detail (stamps) in EnsureRememberedDetailMap()!");
			}
			prs->MoveNext();
		}
	}
	m_bRememberedDetailMapPopulated = TRUE;
}

void CEMNLoader::EnsureEmrActionData(EMRTopicLoadInfo *pLoadInfo, ADODB::_Connection *lpCon)
{
	//DRT 7/26/2007 - PLID 26835 - If we have been requested to ignore loading actions, just skip out of this
	//	function.
	if(GetIgnoreLoadingActions())
		return;

	//
	// (c.haag 2007-04-25 09:51) - PLID 25774 - Populates the existing map of CEMRInfoItems with per-item
	// action data, and also populates m_mapActions with per-list-item-selection action data. This is
	// used to minimize the number of database accesses when EMR actions are processed in the initial load
	//
	// (z.manning, 02/21/2008) - PLID 28690 - We populate m_mapImageHotSpotActions here as well.
	//
	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	CString strInfoFilter;
	CString strDataFilter;
	CString strHotSpotFilter;
	CString strTableDropdownItemFilter;
	int i;

	// (c.haag 2007-07-03 10:22) - PLID 26523 - Take exclusive ownership of EMR info objects until this
	// function is done
	// (c.haag 2007-07-03 10:57) - Also data actions
	// (c.haag 2007-07-10 09:43) - PLID 26595 - Do not include internal details because we should have
	// owned them by now.
	// (z.manning, 02/21/2008) - PLID 28690 - Also take ownership of EMR hot spot action mutex.
	// (z.manning 2009-02-16 10:51) - PLID 33070 - Also the table dropdown item action mutex
	CEMNLoaderMutex* pMutexes[] = { &m_mtxEMRInfo, &m_mtxEMRDataActions, &m_mtxImageHotSpotActions, &m_mtxTableDropdownItemActions };
	CHoldEMNLoaderMultiMutex mh(pMutexes, 4);

	// (z.manning, 01/22/2008) - PLID 28690 - The hot spots are stroed within info items, so we need
	// to be able to map a hot spot to its info item while loading actions.
	CMap<long,long,long,long> mapHotSpotIDToInfoID;

	// (a.walling 2013-03-20 16:09) - PLID 55790 - No more loading behavior
	{
		if (!m_bAllStatesLoaded) {
			// If we get here, lets go ahead and populate the action data for all the details we
			// haven't done already
			const int nDetails = m_apAllCEMNDetails.GetSize();
			// (j.jones 2007-07-20 14:41) - PLID 26742 - renamed these functions to differentiate from the EmrUtils function of the same name
			CEMR *pEMR = NULL;
			if(m_pEMN)
				pEMR = m_pEMN->GetParentEMR();
			for (i=0; i < nDetails; i++) {
				CEMNDetail* pDetail = m_apAllCEMNDetails[i];

				// (c.haag 2007-06-04 16:35) - PLID 26050 - Check whether we loaded the state
				// previously or not. If not, load it in a manner consistent with how we load
				// states from topic to topic
				// (c.haag 2007-06-19 11:28) - PLID 26050 - Instead of using CEMNDetail::IsStateSet, which
				// can return FALSE even if we previously loaded a detail, use WasStateLoaded.
				if (!WasStateLoaded(pDetail)) {
					if(pLoadInfo->bLoadFromTemplate) {
						//TES 6/6/2008 - PLID 29416 - This no longer needs the system table info IDs.
						// (j.jones 2008-09-22 15:48) - PLID 31408 - send the EMR ID
						LoadEMRDetailStateDefaultCascaded(pDetail, pLoadInfo->nPatientID, pLoadInfo->nEMRGroupID,
							lpCon, pLoadInfo);
					}
					else
					{
						LoadEMRDetailStateCascaded(pDetail, lpCon, pLoadInfo);
					}
				}

				// Now populate the info filter string
				if (eitSingleList == pDetail->m_EMRInfoType || eitMultiList == pDetail->m_EMRInfoType) {
					const CString strState = VarString(pDetail->GetState());
					if (!strState.IsEmpty()) {
						if (!m_mapEnsureEmrActionData_QueriedStates[strState]) {
							m_mapEnsureEmrActionData_QueriedStates[strState] = TRUE;
							strDataFilter += strState + ",";
						}
					}
				}
				// (z.manning, 01/22/2008) - PLID 28690 - Now get all the hot spot IDs for this detail.
				else if (pDetail->m_EMRInfoType == eitImage) {
					CArray<long,long> arynHotSpotIDs;
					// (c.haag 2008-11-26 16:52) - PLID 32267 - Pass in the loader and connection pointer
					// so we may avoid excess querying of data
					pDetail->GetImageSelectedHotSpotIDs(arynHotSpotIDs, this, pCon);
					for(int nHotSpotIndex = 0; nHotSpotIndex < arynHotSpotIDs.GetSize(); nHotSpotIndex++) {
						long nHotSpotID = arynHotSpotIDs.GetAt(nHotSpotIndex);
						if(!m_mapEnsureEmrActionData_QueriedHotSpotIDs[nHotSpotID]) {
							m_mapEnsureEmrActionData_QueriedHotSpotIDs[nHotSpotID] = TRUE;
							strHotSpotFilter += AsString(nHotSpotID) + ',';
						}
						if(!mapHotSpotIDToInfoID[nHotSpotID]) {
							mapHotSpotIDToInfoID[nHotSpotID] = pDetail->m_nEMRInfoID;
						}
					}
				}
				// (z.manning 2009-02-16 10:52) - PLID 33070 - Get the selected table dropdown items
				else if (pDetail->m_EMRInfoType == eitTable)
				{
					//TES 11/16/2011 - PLID 45702 - We need to make sure the content is loaded.
					pDetail->LoadContent(FALSE, this, lpCon, TRUE, NULL, m_nProviderIDForFloatingData);
					CArray<long,long> arynDropdownItemIDs;
					for(int nElementIndex = 0; nElementIndex < pDetail->GetTableElementCount(); nElementIndex++) {
						TableElement *pElement = pDetail->GetTableElementPtrByIndex(nElementIndex);
						if(pElement->m_pColumn->nType == LIST_TYPE_DROPDOWN) {
							for(int nSelDropdownIndex = 0; nSelDropdownIndex < pElement->m_anDropdownIDs.GetSize(); nSelDropdownIndex++) {
								long nDropdownID = pElement->m_anDropdownIDs.GetAt(nSelDropdownIndex);
								if(nDropdownID > 0) {
									BOOL bQueried = FALSE;
									BOOL bLookup = m_mapEnsureEmrActionData_QueriedTableDropdownItemIDs.Lookup(nDropdownID, bQueried);
									if(!bQueried || !bLookup) {
										m_mapEnsureEmrActionData_QueriedTableDropdownItemIDs.SetAt(nDropdownID, TRUE);
										strTableDropdownItemFilter += AsString(nDropdownID) + ',';
									}
								}
							}
						}
					}

				}
				// (c.haag 2007-06-25 16:48) - PLID 26050 - Don't query the same info item more
				// than once
				if (!m_mapEnsureEmrActionData_QueriedInfoIDs[pDetail->m_nEMRInfoID]) {
					m_mapEnsureEmrActionData_QueriedInfoIDs[pDetail->m_nEMRInfoID] = TRUE;
					strInfoFilter += AsString(pDetail->m_nEMRInfoID) + ",";
				}

				// Now make sure the preloader object knows that the detail is in use
				SetUsed(pDetail);
			}

			// Now we can officially say that all the states have been loaded
			m_bAllStatesLoaded = TRUE;
			
		} else {
			// If we get here, we've already loaded all the EMR action data possible.
			return;
		}
	}

	// Do nothing if we didn't get any data
	if (strInfoFilter.IsEmpty() && strDataFilter.IsEmpty())
		return;

	// Clean up the "where" strings and generate the final where clause
	strInfoFilter.TrimRight(",");
	strDataFilter.Replace(";",",");
	strDataFilter.TrimRight(",");
	strHotSpotFilter.TrimRight(',');
	strTableDropdownItemFilter.TrimRight(',');
	
	MFCArray<EmrAction> arActions;
	{
		// (z.manning 2009-02-16 11:31) - PLID 33070 - Added table dropdown item based actions
		CSqlFragment sqlTableDropdownSql;
		if(!strTableDropdownItemFilter.IsEmpty()) {
			sqlTableDropdownSql = CSqlFragment(
				"	OR "
				"	(SourceType = {CONST} AND SourceID IN ({INTSTRING})) "
				, eaoEmrTableDropDownItem, strTableDropdownItemFilter);
		}
		CSqlFragment sqlWhere(
			"("
			"	(SourceType = {CONST} AND SourceID IN ({INTSTRING})) "
			"	OR "
			"	(SourceType = {CONST} AND SourceID IN ({INTSTRING})) "
			"	OR "
			"	(SourceType = {CONST} AND SourceID IN ({INTSTRING})) "
			"{SQL} "
			") ",
			eaoEmrItem, 
			strInfoFilter,
			eaoEmrDataItem,
			strDataFilter,
			eaoEmrImageHotSpot,
			strHotSpotFilter, 
			sqlTableDropdownSql
			);

		// Now populate the action array with existing logic
		LoadActionInfo(sqlWhere, arActions, pCon);
	}
	
	// Now disperse the array results between our CEMRInfoItem objects and our action maps
	// for proper lookups in processing EMR actions
	const int nActions = arActions.GetSize();
	for (i=0; i < nActions; i++) {
		EmrAction& ea = arActions[i];
		if (eaoEmrItem == ea.eaoSourceType) {
			const long nEMRInfoID = ea.nSourceID;
			// (c.haag 2007-09-20 13:02) - PLID 27465 - Use Lookup() instead of []. If the lookup fails, pInfoItem will be NULL
			CEMRInfoItem* pInfo = NULL;
			m_mapInfoItems.Lookup(nEMRInfoID, pInfo);
			if (NULL == pInfo) {
				ThrowNxException("Attempted to assign an action to a non-existing EMR info item!");
			}
			pInfo->m_arActions.Add(ea);
		}
		else if (eaoEmrDataItem == ea.eaoSourceType) {
			const long nEMRDataID = ea.nSourceID;
			MFCArray<EmrAction>* paActions = NULL;
			if (!m_mapActions.Lookup(nEMRDataID, paActions)) {
				if (NULL == (paActions = new MFCArray<EmrAction>)) {
					ThrowNxException("Could not allocate an action array for CEMNLoader::EnsureEmrActionData!");
				}
				m_mapActions[ nEMRDataID ] = paActions;
			}
			if (paActions) {
				// (c.haag 2007-06-25 16:58) - PLID 26050 - Do an extra check to ensure
				// we don't add duplicate actions to paActions
				const int nActions = paActions->GetSize();
				BOOL bFound = FALSE;
				for (int i=0; i < nActions && !bFound; i++) {
					if (paActions->GetAt(i).nID == ea.nID) {
						bFound = TRUE;
					}
				}
				if (!bFound) {
					paActions->Add(ea);
				}
			} else {
				ThrowNxException("Could not get a valid action array for CEMNLoader::EnsureEmrActionData!");
			}
		}
		// (z.manning, 01/22/2008) - PLID 28690 - Handle hot spot actions.
		else if(eaoEmrImageHotSpot == ea.eaoSourceType) {
			const long nHotSpotID = ea.nSourceID;
			MFCArray<EmrAction>* paryActions = NULL;
			if(!m_mapImageHotSpotActions.Lookup(nHotSpotID, paryActions)) {
				paryActions = new MFCArray<EmrAction>;
				if(paryActions == NULL) {
					ThrowNxException("Could not allocate a hot spot action array for CEMNLoader::EnsureEmrActionData!");
				}
				m_mapImageHotSpotActions[nHotSpotID] = paryActions;
			}
			if(paryActions != NULL) {
				const int nActions = paryActions->GetSize();
				BOOL bFound = FALSE;
				for (int i = 0; i < nActions && !bFound; i++) {
					if(paryActions->GetAt(i).nID == ea.nID) {
						bFound = TRUE;
					}
				}
				if(!bFound) {
					paryActions->Add(ea);
				}
			} 
			else {
				ThrowNxException("Could not get a valid hot spot action array for CEMNLoader::EnsureEmrActionData!");
			}
		}
		// (z.manning 2009-02-16 11:37) - PLID 33070 - Table dropdown item actions
		else if(ea.eaoSourceType == eaoEmrTableDropDownItem)
		{
			const long nDropdownID = ea.nSourceID;
			MFCArray<EmrAction>*paryActions = NULL;
			if(!m_mapTableDropdownItemActions.Lookup(nDropdownID, paryActions)) {
				paryActions = new MFCArray<EmrAction>;
				if(paryActions == NULL) {
					ThrowNxException("Could not allocate a table dropdown action array for CEMNLoader::EnsureEmrActionData!");
				}
				m_mapTableDropdownItemActions.SetAt(nDropdownID, paryActions);
			}
			if(paryActions != NULL) {
				const int nActions = paryActions->GetSize();
				BOOL bFound = FALSE;
				for(int nActionIndex = 0; nActionIndex < nActions && !bFound; nActionIndex++) {
					EmrAction eaTemp = paryActions->GetAt(nActionIndex);
					if(eaTemp.nID == ea.nID) {
						bFound = TRUE;
					}
				}
				if(!bFound) {
					paryActions->Add(ea);
				}
			}
			else {
				ThrowNxException("Could not get a valid table dropdown action array for CEMNLoader::EnsureEmrActionData!");
			}
		}
	}
}

void CEMNLoader::EnsureEmrMapGeneric(ADODB::_Connection *lpCon, BOOL bLoadActionsType, EMRTopicLoadInfo *pLoadInfo)
{
	//DRT 1/21/2008 - PLID 28603 - Can I unify the 2 Ensure Map functions (EMRDataT and EMRHotSpots) into 1
	//	centralized function?  Or are there too many bits and pieces that differ?

	//
	// (c.haag 2007-04-26 09:46) - PLID 25790 - Populates the existing map of CEMRInfoItems with per-item
	// EmrDataT data. This is used in CEMNDetail::LoadContent, and must be called after the CEMRInfoItem
	// map is already populated (which is done in the mass preloading of details)
	//

	// (c.haag 2007-07-03 10:22) - PLID 26523 - Take exclusive ownership of EMR info objects until this
	// function is done
	// (c.haag 2007-07-10 09:43) - PLID 26595 - Do not take exclusive ownership of internal details; we
	// should have already owned them by now
	CHoldEMNLoaderMutex mh(&m_mtxEMRInfo);

	if (m_bEMRDataTItemsLoaded) {
		return;
	}

	// (a.walling 2013-03-11 15:10) - PLID 55572 - Still calling this m_prsEmrDataTMap so as not to confuse a commit with lots of variable name changes
	ADODB::_RecordsetPtr m_prsEmrDataTMap;

	// (a.walling 2013-03-27 10:06) - PLID 55901 - Load current cached instances
	Emr::Cache::SystemTablePtr pMedications = Emr::Cache::Medications();
	Emr::Cache::SystemTablePtr pAllergies = Emr::Cache::Allergies();

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	// (a.walling 2013-03-11 15:10) - PLID 55572 - Grab local connection from pool
	CNxAdoConnection localConnection(pCon);

	{
		// (j.jones 2011-04-28 14:39) - PLID 43122 - If the currently selected EMN provider has ProvidersT.FloatEMRData = 1,
		// load the data from EmrProviderFloatDataT. If we are sorting alphabetically, sort the "floated" items first, 
		// then the regular items. If not sorting alphabetically, sort the "floated" items in Count order DESC, and then
		// the regular items in their normal sort order.

		// (a.walling 2013-03-27 10:06) - PLID 55901 - If cached data is current, we can avoid loading them in this query
		CSqlFragment sqlVerifyCachedActiveSystemItemsData;
		{
			if (m_bLoadFromTemplate) {
				sqlVerifyCachedActiveSystemItemsData = CSqlFragment(
					"AND (\r\n"
					"(EmrInfoT.DataSubType NOT IN (1, 2))\r\n"
						"OR (\r\n"
							"(EmrInfoT.DataSubType = 1 AND (EmrInfoT.ID <> ISNULL({VT_I4}, -1) OR EmrInfoT.Revision <> ISNULL({VARBINARY}, 0x0000000000000000)))\r\n"
							"OR\r\n"
							"(EmrInfoT.DataSubType = 2 AND (EmrInfoT.ID <> ISNULL({VT_I4}, -1) OR EmrInfoT.Revision <> ISNULL({VARBINARY}, 0x0000000000000000)))\r\n"
						")\r\n"
					")\r\n"
					, pMedications->id, pMedications->revision
					, pAllergies->id, pAllergies->revision
				);
			}
		}

		// (z.manning, 05/23/2008) - PLID 30155 - Added EmrDataT.Formula and DecimalPlaces
		// (z.manning 2009-01-15 15:32) - PLID 32724 - Added InputMask
		// (z.manning 2010-02-16 14:35) - PLID 37230 - ListSubType
		// (c.haag 2010-02-24 14:14) - PLID 21301 - AutoAlphabetizeDropdown
		// (z.manning 2011-03-11) - PLID 42778 - Added HasDropdownElements
		//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
		// (z.manning 2011-03-21 11:27) - PLID 30608 - Added autofill type
		// (z.manning 2011-05-26 14:59) - PLID 43865 - Added DataFlags
		// (z.manning 2011-09-19 17:20) - PLID 41954 - Dropdown separators
		// (z.manning 2011-11-07 10:49) - PLID 46309 - SpawnedItemsSeparator
		// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
		
		CSqlFragment sqlLoadSql = CSqlFragment(
			// (a.walling 2013-03-25 14:13) - PLID 55845 - Declaring some parameters as local vars
			"DECLARE @LoadingID INT;\r\n"
			"DECLARE @FloatingProviderID INT;\r\n"
			"SET @LoadingID = {INT};\r\n"
			"SET @FloatingProviderID = {INT};\r\n"
			// (s.dhole 2013-06-27 09:07) - PLID 57335 Temp table to stored Topic id
			"{SQL}"
			// (z.manning 2011-10-25 09:36) - PLID 39401 - Added a second query to load per item stamp info
			// (a.walling 2013-03-11 15:10) - PLID 55572 - Prepend this query so we don't have to bother with moving first etc and can use a server cursor
			"SELECT EmrInfoT.ID AS EmrInfoID, StampID \r\n"
			"FROM EmrInfoStampExclusionsT \r\n"
			"INNER JOIN EmrInfoMasterT ON EmrInfoStampExclusionsT.EmrInfoMasterID = EmrInfoMasterT.ID \r\n"
			"INNER JOIN EmrInfoT ON EmrInfoMasterT.ID = EmrInfoT.EmrInfoMasterID \r\n"
			"WHERE EmrInfoT.ID IN ({SQL}) AND EmrInfoT.DataType = {CONST_INT}; \r\n" // (a.walling 2013-03-11 15:10) - PLID 55572 - using CONST_INT
			"\r\n"
			////////
			// (a.walling 2013-03-25 14:13) - PLID 55845 - Loading hotspots separately
			"SELECT EmrInfoT.ID AS EMRInfoID, "
			//DRT 1/22/2008 - PLID 28603 - Added hotspot data to the query.
			//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for hotspots
			// (z.manning 2011-07-25 12:56) - PLID 44649 - AddedImageHotSpotID
			"EMRImageHotSpotsT.ID AS HotSpotID, EMRImageHotSpotsT.EmrSpotGroupID, EMRImageHotSpotsT.Data AS HotSpotData, "
			"EMRImageHotSpotsT.AnatomicLocationID AS HotSpotLocationID, LabAnatomyT.Description AS HotSpotLocation, "
			"EMRImageHotSpotsT.AnatomicQualifierID AS HotSpotQualifierID, AnatomyQualifiersT.Name AS HotSpotQualifier, "
			"EMRImageHotSpotsT.AnatomySide AS HotSpotSide, ImageHotSpotID "
			"FROM EmrInfoT "
			// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
			//DRT 1/22/2008 - PLID 28603 - Added HotSpot data to the query
			//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for hotspots
			"INNER JOIN EMRImageHotSpotsT ON EmrInfoT.ID = EMRImageHotSpotsT.EMRInfoID "
			"LEFT JOIN LabAnatomyT ON EMRImageHotSpotsT.AnatomicLocationID = LabAnatomyT.ID "
			"LEFT JOIN AnatomyQualifiersT ON EMRImageHotSpotsT.AnatomicQualifierID = AnatomyQualifiersT.ID "
			"WHERE EmrInfoT.ID IN ({SQL}) "
			"AND EmrInfoT.DataType = {CONST_INT};\r\n"
			"\r\n"
			/////////
			// (a.walling 2013-03-25 14:13) - PLID 55845 - Now the EmrDatT load, without the joins and fields for hotspot stuff
			"SELECT EmrInfoT.ID AS EMRInfoID, EmrDataT.ID, EmrDataT.EmrDataGroupID, EmrDataT.Data, EmrDataT.IsLabel, EmrDataT.ListType, EmrDataT.IsGrouped, "
			"EmrDataT.LongForm AS DataLongForm, EmrDataT.SortOrder, EmrDataT.Formula, EmrDataT.DecimalPlaces, EmrDataT.DataFlags, "
			 // (a.walling 2013-03-11 15:10) - PLID 55572 - using CONST_INT
			"CASE WHEN {CONST_INT} = 1 THEN CASE WHEN EmrInfoT.DataType = 2 OR EmrInfoT.DataType = 3 THEN (SELECT Min(CASE WHEN DestType = 3 OR DestType = 9 THEN 1 ELSE 2 END) AS MinActionType FROM EMRActionsT WHERE SourceType = 4 AND SourceID = EmrDataT.ID AND Deleted = 0) ELSE NULL END ELSE NULL END AS ActionsType, "
			// (j.jones 2007-08-27 10:26) - PLID 27056 - added E/M coding data
			// (j.jones 2011-03-09 09:05) - PLID 42283 - added Data_EMCodeCategoryID
			// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of this, and the join to emrdatagroupst
			"EmrDataT.InputMask, EmrDataT.ListSubType, EmrDataT.AutoAlphabetizeDropdown, "
			// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
			"EmrDataT.AutoNumberType, EmrDataT.AutoNumberPrefix, "
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
			// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floating
			//"Convert(bit, CASE WHEN FloatedItemsQ.Count Is Not Null THEN 1 ELSE 0 END) AS IsFloated, "
			// (a.walling 2013-03-18 09:32) - PLID 55723 - Load float count and sort order for EmrDataT records in the CEMNLoader
			"FloatedItemsQ.Count AS FloatCount, "
			// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
			"EmrDataT.WoundCareDataType "
			"FROM EmrInfoT "
			"INNER JOIN EMRDataT ON EmrInfoT.ID = EmrDataT.EmrInfoID "
			// (a.walling 2013-03-21 10:01) - PLID 55805 - EM data is not needed, get rid of the join to emrdatagroupst
			// (j.jones 2011-04-28 14:39) - PLID 43122 - load the floating data info
			// (a.walling 2013-03-21 11:23) - PLID 55810 - The EmrProviderFloatDataT join can be skipped entirely if the providerid being scanned for is set to -1
			"LEFT JOIN (SELECT EMRDataGroupID, Count "
			"	FROM EmrProviderFloatDataT "
			"	WHERE ProviderID = @FloatingProviderID AND @FloatingProviderID <> -1) AS FloatedItemsQ ON EMRDataT.EmrDataGroupID = FloatedItemsQ.EMRDataGroupID "
			"WHERE EmrInfoT.ID IN ({SQL}) AND (EMRDataT.Inactive Is Null OR EMRDataT.Inactive = 0) "
			"AND EmrInfoT.DataType IN ({CONST_INT}, {CONST_INT}, {CONST_INT}) "
			"{SQL};"
			"  "
			, m_nMasterID
			, m_nProviderIDForFloatingData
			// (s.dhole 2013-07-03 12:46) - PLID 57335 load temp table if m_bLoadFromTemplate = 1
			,(m_bLoadFromTemplate?CSqlFragment("DECLARE @EMRTemplateTopicTemp TABLE(ID INT);"
			"SET NOCOUNT ON \r\n"
			"INSERT INTO @EMRTemplateTopicTemp \r\n"
			"SELECT  ID  FROM (SELECT ID FROM EMRTemplateTopicsT WHERE TemplateID = @LoadingID \r\n"
			"UNION \r\n"
			"SELECT SourceTemplateTopicID AS ID FROM EMRTemplateTopicsT WHERE TemplateID = @LoadingID  \r\n"
			") As tbl; \r\n"
			" \r\n"
			"SET NOCOUNT OFF \r\n"):CSqlFragment("")) 
			// (a.walling 2013-03-18 09:34) - PLID 55725 - Removed ORDER BY, handled on client now, can save several hundred ms and lets us process the results immediately
			, BuildAllRelatedEmrInfoTQ(), eitImage
			, BuildAllRelatedEmrInfoTQ(), eitImage
			//
			, bLoadActionsType
			// (c.haag 2007-05-08 13:06) - PLID 25790 - Use BuildAllRelatedEmrInfoTQ instead of building
			// a list of EMR info ID's
			, BuildAllRelatedEmrInfoTQ(), eitSingleList, eitMultiList, eitTable
			, sqlVerifyCachedActiveSystemItemsData
		);

		// (a.walling 2013-03-11 15:10) - PLID 55572 - Load up the recordset using server-side cursor
		localConnection.EnsureRemoteData(acDoNotAffirm);
		ADODB::_RecordsetPtr prsData = ::CreateParamRecordset(localConnection, ADODB::adUseServer, ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText, "{SQL}", sqlLoadSql);
		
		{
			// (z.manning 2011-10-25 09:53) - PLID 39401 - Load any stamp exclusion data and keep track of it in a map
			// for the time being.
			ADODB::_RecordsetPtr prsStampExclusions = prsData;
			for(; !prsStampExclusions->eof; prsStampExclusions->MoveNext())
			{
				const long nEmrInfoID = AdoFldLong(prsStampExclusions, "EmrInfoID");
				const long nStampID = AdoFldLong(prsStampExclusions, "StampID");
				CEmrItemStampExclusions *pStampExclusions = NULL;
				if(!m_mapInfoIDToStampExclusions.Lookup(nEmrInfoID, pStampExclusions)) {
					pStampExclusions = new CEmrItemStampExclusions();
					m_mapInfoIDToStampExclusions.SetAt(nEmrInfoID, pStampExclusions);
				}
				pStampExclusions->AddExclusion(nStampID);
			}
		}

		m_prsEmrDataTMap = prsData->NextRecordset(NULL);
	}


	// (a.walling 2013-03-25 14:13) - PLID 55845 - Loading hotspots separately
	// hotspot info
	{		
		// (c.haag 2007-05-09 11:19) - Seek to the start of the recordset
		FieldsPtr f = m_prsEmrDataTMap->Fields;

		// (a.walling 2013-03-04 16:48) - PLID 55450 - Gather the appropriate fields ahead of time; accounts for about 50% of load time
		// (a.walling 2013-03-11 15:10) - PLID 55572 - More fields
		ADODB::FieldPtr f_EMRInfoID = f->Item["EMRInfoID"];
		ADODB::FieldPtr f_EmrSpotGroupID = f->Item["EmrSpotGroupID"];
		ADODB::FieldPtr f_HotSpotData = f->Item["HotSpotData"];
		ADODB::FieldPtr f_HotSpotLocationID = f->Item["HotSpotLocationID"];
		ADODB::FieldPtr f_HotSpotLocation = f->Item["HotSpotLocation"];
		ADODB::FieldPtr f_HotSpotQualifierID = f->Item["HotSpotQualifierID"];
		ADODB::FieldPtr f_HotSpotQualifier = f->Item["HotSpotQualifier"];
		ADODB::FieldPtr f_HotSpotSide = f->Item["HotSpotSide"];
		ADODB::FieldPtr f_ImageHotSpotID = f->Item["ImageHotSpotID"];
		// (a.walling 2013-03-11 15:10) - PLID 55572 - More fields
		ADODB::FieldPtr f_HotSpotID = f->Item["HotSpotID"];

		CEMRInfoItem* pInfoItem = NULL;
		for (; !m_prsEmrDataTMap->eof; m_prsEmrDataTMap->MoveNext()) {
			const long nEMRInfoID = AdoFldLong(f_EMRInfoID);
			// (c.haag 2007-09-20 13:07) - PLID 27465 - Use Lookup() instead of []. If the lookup fails, pInfoItem will be NULL
			if (!pInfoItem || pInfoItem->m_nID != nEMRInfoID) {
				if (!m_mapInfoItems.Lookup(nEMRInfoID, pInfoItem)) {
					ThrowNxException("Failed to find EMR info item in CEMNLoader::EnsureEmrMapGeneric!");
				}
			}

			_ASSERTE(pInfoItem->m_DataType == eitImage);

			//DRT 1/21/2008 - PLID 28603 - Images are new with hotspot data.
			if (pInfoItem->m_bHotSpotItemsPopulated) {
				continue;
			}
			
			if (NULL == pInfoItem->m_paryHotSpotItems) {
				pInfoItem->m_paryHotSpotItems = new CArray<EmrImageHotSpotItem*, EmrImageHotSpotItem*>();
			}

			// Fill in the EMRHotSpot records
			EmrImageHotSpotItem *pData = NULL;

			_variant_t vID = f_HotSpotID->Value;
			if (VT_NULL != vID.vt) {
				pData = new EmrImageHotSpotItem;
				pData->m_nID = VarLong(vID);
				pData->m_nEMRHotSpotGroupID = AdoFldLong(f_EmrSpotGroupID);
				pData->m_strData = AdoFldString(f_HotSpotData, CString());
				//TES 2/11/2010 - PLID 37298 - Added Anatomic Location info for hotspots
				pData->m_nAnatomicLocationID = AdoFldLong(f_HotSpotLocationID, -1);
				pData->m_strAnatomicLocation = AdoFldString(f_HotSpotLocation, CString());
				pData->m_nAnatomicQualifierID = AdoFldLong(f_HotSpotQualifierID, -1);
				pData->m_strAnatomicQualifier = AdoFldString(f_HotSpotQualifier, "");
				pData->m_asSide = (AnatomySide)AdoFldLong(f_HotSpotSide);
				pData->m_n3DHotSpotID = AdoFldShort(f_ImageHotSpotID, -1);
				pInfoItem->m_paryHotSpotItems->Add(pData);
			}

			// (z.manning 2011-10-25 10:05) - PLID 39401 - Check and see if we have any stamp exlusions for this item.
			if(pInfoItem->m_pStampExclusions == NULL) {
				CEmrItemStampExclusions *pStampExclusions = NULL;
				if(m_mapInfoIDToStampExclusions.Lookup(nEMRInfoID, pStampExclusions)) {
					pInfoItem->m_pStampExclusions = new CEmrItemStampExclusions();
					*pInfoItem->m_pStampExclusions = *pStampExclusions;
				}
			}
		}

		m_prsEmrDataTMap = m_prsEmrDataTMap->NextRecordset(NULL);
	}

	// emrdatat info
	// (a.walling 2013-03-25 14:13) - PLID 55845 - Now loading EMRDataT info
	{
		// (c.haag 2007-05-09 11:19) - Seek to the start of the recordset
		FieldsPtr f = m_prsEmrDataTMap->Fields;

		// (a.walling 2013-03-04 16:48) - PLID 55450 - Gather the appropriate fields ahead of time; accounts for about 50% of load time
		// (a.walling 2013-03-11 15:10) - PLID 55572 - More fields
		ADODB::FieldPtr f_ID = f->Item["ID"];
		ADODB::FieldPtr f_EMRInfoID = f->Item["EMRInfoID"];
		ADODB::FieldPtr f_EmrDataGroupID = f->Item["EmrDataGroupID"];
		ADODB::FieldPtr f_Data = f->Item["Data"];
		ADODB::FieldPtr f_IsLabel = f->Item["IsLabel"];
		ADODB::FieldPtr f_ListType = f->Item["ListType"];
		ADODB::FieldPtr f_ListSubType = f->Item["ListSubType"];
		ADODB::FieldPtr f_AutoAlphabetizeDropdown = f->Item["AutoAlphabetizeDropdown"];
		ADODB::FieldPtr f_IsGrouped = f->Item["IsGrouped"];
		ADODB::FieldPtr f_DataLongForm = f->Item["DataLongForm"];
		ADODB::FieldPtr f_ActionsType = f->Item["ActionsType"];
		ADODB::FieldPtr f_Formula = f->Item["Formula"];
		ADODB::FieldPtr f_DecimalPlaces = f->Item["DecimalPlaces"];
		ADODB::FieldPtr f_InputMask = f->Item["InputMask"];
		ADODB::FieldPtr f_AutoNumberType = f->Item["AutoNumberType"];
		ADODB::FieldPtr f_AutoNumberPrefix = f->Item["AutoNumberPrefix"];
		ADODB::FieldPtr f_HasDropdownElements = f->Item["HasDropdownElements"];
		ADODB::FieldPtr f_HasActiveDropdownElements = f->Item["HasActiveDropdownElements"];
		ADODB::FieldPtr f_GlassesOrderDataType = f->Item["GlassesOrderDataType"];
		ADODB::FieldPtr f_GlassesOrderDataID = f->Item["GlassesOrderDataID"];
		ADODB::FieldPtr f_AutofillType = f->Item["AutofillType"];
		ADODB::FieldPtr f_DataFlags = f->Item["DataFlags"];
		//ADODB::FieldPtr f_IsFloated = f->Item["IsFloated"];
		ADODB::FieldPtr f_FloatCount = f->Item["FloatCount"]; // (a.walling 2013-03-18 09:32) - PLID 55723 - Load float count and sort order for EmrDataT records in the CEMNLoader
		ADODB::FieldPtr f_SortOrder = f->Item["SortOrder"];
		ADODB::FieldPtr f_DropdownSeparator = f->Item["DropdownSeparator"];
		ADODB::FieldPtr f_DropdownSeparatorFinal = f->Item["DropdownSeparatorFinal"];
		ADODB::FieldPtr f_SpawnedItemsSeparator = f->Item["SpawnedItemsSeparator"];
		ADODB::FieldPtr f_WoundCareDataType = f->Item["WoundCareDataType"];
		ADODB::FieldPtr f_ParentLabelID = f->Item["ParentLabelID"];

		static const CString cstrSep = ", ";

		// (a.walling 2013-03-20 16:09) - PLID 55790 - No more loading behavior
		// (c.haag 2007-05-09 11:11) - PLID 26050 - If the first visible topic has been completely loaded,
		// we're free to just brute force through the entire recordset and fill all the EmrInfo
		// items with action data. This is all done asynchronously, and should be done (or very
		// close to done) by the time the user decides to go to another topic.
		boost::unordered_set<CEMRInfoItem*> populatedInfoItems;
		CEMRInfoItem* pInfoItem = NULL;
		for (; !m_prsEmrDataTMap->eof; m_prsEmrDataTMap->MoveNext()) {
			const long nEMRInfoID = AdoFldLong(f_EMRInfoID);
			// (c.haag 2007-09-20 13:07) - PLID 27465 - Use Lookup() instead of []. If the lookup fails, pInfoItem will be NULL
			if (!pInfoItem || pInfoItem->m_nID != nEMRInfoID) {
				if (!m_mapInfoItems.Lookup(nEMRInfoID, pInfoItem)) {
					ThrowNxException("Failed to find EMR info item in CEMNLoader::EnsureEmrMapGeneric!");
				}
				populatedInfoItems.insert(pInfoItem);
			}

			// (c.haag 2007-05-09 11:15) - Skip this info item if we already populated it earlier
			//DRT 1/21/2008 - PLID 28603 - I combined 2 functions into 1 generic map loading function.  Unfortunately the
			//	specific data elements need to be worked out per type.  So that's here.  Previously this code was just after we made
			//	sure that it wasn't already loaded (and it still is now).  We now check for the type of info item we're loading, 
			//	and appropriately check for the required variables and load them if needed.  If in the future we need to map another 
			//	info type, just add another else here.
			//Optimization / design change.  While implementing this, I noticed that this loop
			//	was happening with all info items in the loaded EMR / Template.  But only the 3 types below can have legit
			//	EMRDataT records tied to them, so anytime you had an image, narrative, slider, text, what have you, it was
			//	incorrectly allocating memory for the m_paDataItems array, then never putting anything in it.  What a waste!
			//I've thus changed this code so the relevant arrays are only generated for the relevant types.
			_ASSERTE(pInfoItem->m_DataType == eitSingleList || pInfoItem->m_DataType == eitMultiList || pInfoItem->m_DataType == eitTable);

			if (pInfoItem->m_bDataItemsPopulated) {
				continue;
			}

			if (NULL == pInfoItem->m_paDataItems) {
				pInfoItem->m_paDataItems = new CEmrDataItemArray();
			}
			// Fill in the EmrDataT records
			EmrDataItem data;
			_variant_t vID = f_ID->Value;
			if (VT_I4 != vID.vt) {
				continue;
			}

			data.m_nID = VarLong(vID);
			data.m_nEMRDataGroupID = AdoFldLong(f_EmrDataGroupID);
			data.m_strData = AdoFldString(f_Data);
			data.m_bIsLabel = AdoFldBool(f_IsLabel);
			data.m_nListType = AdoFldLong(f_ListType);
			// (z.manning 2010-02-16 14:37) - PLID 37230 - ListSubType
			data.m_nListSubType = AdoFldByte(f_ListSubType, lstDefault);
			// (c.haag 2010-02-24 14:17) - PLID 21301 - AutoAlphabetizeDropdown
			data.m_bAutoAlphabetizeDropdown = AdoFldBool(f_AutoAlphabetizeDropdown);
			data.m_bIsGrouped = AdoFldBool(f_IsGrouped);
			data.m_strLongForm = AdoFldString(f_DataLongForm);
			data.m_nActionsType = AdoFldLong(f_ActionsType, -1);
			// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
			// (z.manning, 05/23/2008) - PLID 30155 - Added Formula and DecimalPlaces
			data.m_strFormula = AdoFldString(f_Formula);
			data.m_nDecimalPlaces = AdoFldByte(f_DecimalPlaces);
			// (z.manning 2009-01-15 15:32) - PLID 32724 - Added InputMask
			data.m_strInputMask = AdoFldString(f_InputMask, CString());
			// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
			data.m_etantAutoNumberType = (EEmrTableAutoNumberType)AdoFldByte(f_AutoNumberType, (BYTE)etantPerRow);
			data.m_strAutoNumberPrefix = AdoFldString(f_AutoNumberPrefix, CString());
			data.m_bHasDropdownElements = AdoFldBool(f_HasDropdownElements, TRUE); // (z.manning 2011-03-11) - PLID 42778
			data.m_bHasActiveDropdownElements = AdoFldBool(f_HasActiveDropdownElements, TRUE); // (z.manning 2011-03-11) - PLID 42778
			//TES 3/17/2011 - PLID 41108 - Added GlassesOrderDataType and GlassesOrderDataID
			data.m_GlassesOrderDataType = (GlassesOrderDataType)AdoFldLong(f_GlassesOrderDataType, (long)godtInvalid);
			data.m_nGlassesOrderDataID = AdoFldLong(f_GlassesOrderDataID, -1);
			data.m_eAutofillType = (EmrTableAutofillType)AdoFldByte(f_AutofillType, etatNone); // (z.manning 2011-03-21 11:28) - PLID 30608
			data.m_nFlags = AdoFldLong(f_DataFlags, 0);
			// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floated,
			// which will require that we bold the text
			data.m_nFloatCount = AdoFldLong(f_FloatCount, -1); // (a.walling 2013-03-18 09:32) - PLID 55723 - Load float count and sort order for EmrDataT records in the CEMNLoader
			data.m_nSortOrder = AdoFldLong(f_SortOrder, -1);
			data.m_strDropdownSeparator = AdoFldString(f_DropdownSeparator, cstrSep);
			data.m_strDropdownSeparatorFinal = AdoFldString(f_DropdownSeparatorFinal, cstrSep);
			data.m_strSpawnedItemsSeparator = AdoFldString(f_SpawnedItemsSeparator, cstrSep);
			// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
			data.m_ewccWoundCareDataType = (EWoundCareDataType)AdoFldLong(f_WoundCareDataType, wcdtNone);
			// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
			data.m_nParentLabelID = AdoFldLong(f_ParentLabelID, -1);
			pInfoItem->m_paDataItems->Add(data);
		}
		
		for each (CEMRInfoItem* pPopulatedItem in populatedInfoItems) {
			pPopulatedItem->SortDataItems(); // (a.walling 2013-03-18 09:34) - PLID 55725 - sort data items

			// (a.walling 2013-03-27 10:06) - PLID 55901 - Cache any updated system items
			if (m_bLoadFromTemplate) {
				switch (pPopulatedItem->m_DataSubType) {
					case eistCurrentMedicationsTable:
						pMedications = Emr::Cache::UpdateMedications(*pPopulatedItem);
						break;
					case eistAllergiesTable:
						pAllergies = Emr::Cache::UpdateAllergies(*pPopulatedItem);
						break;
					default:
						break;
				}
			}
		}

		// (a.walling 2013-03-27 10:06) - PLID 55901 - If we did not update a system item, and it is part of the info items we are loading, copy the cached data
		if (m_bLoadFromTemplate) {
			if (!pMedications->IsNull() && m_mapInfoItems.Lookup(pMedications->id, pInfoItem) && !populatedInfoItems.count(pInfoItem) ) {
				delete pInfoItem->m_paDataItems;
				pInfoItem->m_paDataItems = pMedications->CreateEmrDataItemArray();
				pInfoItem->m_bDataItemsPopulated = TRUE;
			}
			if (!pAllergies->IsNull() && m_mapInfoItems.Lookup(pAllergies->id, pInfoItem) && !populatedInfoItems.count(pInfoItem) ) {
				delete pInfoItem->m_paDataItems;
				pInfoItem->m_paDataItems = pAllergies->CreateEmrDataItemArray();
				pInfoItem->m_bDataItemsPopulated = TRUE;
			}
		}
	}
	
	// (c.haag 2007-05-09 11:17) - We're permanently done with the recordset
	m_prsEmrDataTMap->Close();
	m_prsEmrDataTMap = NULL;

	// (c.haag 2007-05-09 11:27) - PLID 26050 - Now flag all modified info items as having been populated
	// with actions
	// (c.haag 2007-07-02 11:37) - PLID 26050 - Some info items may have no data items. Make sure they are
	// flagged as having been populated (with nothing) so that LoadContent doesn't try to load what doesn't
	// exist
	POSITION pos = m_mapInfoItems.GetStartPosition();
	while (pos != NULL) {
		CEMRInfoItem* pInfo;
		long nEMRInfoID;
		m_mapInfoItems.GetNextAssoc( pos, nEMRInfoID, pInfo );
		// (c.haag 2007-09-20 12:59) - PLID 27465 - pInfo should never be NULL, but check anyway
		if (NULL != pInfo) {
			if (pInfo->m_DataType == eitSingleList || pInfo->m_DataType == eitMultiList || pInfo->m_DataType == eitTable) {
				pInfo->m_bDataItemsPopulated = TRUE;
			}
			else if (pInfo->m_DataType == eitImage) {
				pInfo->m_bHotSpotItemsPopulated = TRUE;
			}
		} 
	}

	// (c.haag 2007-05-09 11:18) - We're permanently done with this function
	m_bEMRDataTItemsLoaded = TRUE;
}

void CEMNLoader::EnsureEmrItemLinkedDataTArray(ADODB::_Connection *lpCon)
{
	// (c.haag 2007-07-03 11:08) - PLID 26523 - Keep exclusive access to the linked data
	// item array
	CHoldEMNLoaderMutex mh(&m_mtxLinkedDataItems);

	//
	// (c.haag 2007-06-06 15:22) - PLID 26240 - Load all possible EmrItemLinkedDataT
	// entries for this EMN / template
	//
	if (m_bAllEmrItemLinkedDataItemsLoaded)
		return;

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	_RecordsetPtr prs;
	if (m_bLoadFromTemplate) {
		//DRT 7/17/2007 - PLID 26716 - Switched to be a parameterized query for better server efficiency
		// (c.haag 2008-03-28 12:54) - PLID 29428 - I'm effectively encapsulating the existing query inside
		// a select query so that we include EmrItemLinkedDataT items corresponding to both EmrInfo records
		// rather than just the one that corresponds to this template for every "pair".
		// (j.dinatale 2013-03-13 16:29) - PLID 55631 - switched to joins instead of the nested INs
		_CommandPtr pCmd = OpenParamQuery(pCon, 
			"SELECT EmrItemLinkedDataT.ID, EmrItemLinkedDataT.EmrLinkID, EmrItemLinkedDataT.EmrDataID, EmrDataT.EmrDataGroupID, EmrDataT.EmrInfoID "
			"FROM EmrItemLinkedDataT "
			"INNER JOIN EmrDataT ON EmrItemLinkedDataT.EmrDataID = EmrDataT.ID "
			"INNER JOIN ( "
			"	SELECT EmrItemLinkedDataT.EmrLinkID FROM EmrItemLinkedDataT "
			"	INNER JOIN EmrDataT ON EmrItemLinkedDataT.EmrDataID = EmrDataT.ID "
			"	INNER JOIN ( "
			"		SELECT DISTINCT EMRInfoMasterT.ActiveEmrInfoID "
			"		FROM EMRTemplateDetailsT "
			"		LEFT JOIN EMRInfoMasterT ON EMRInfoMasterT.ID = EMRTemplateDetailsT.EMRInfoMasterID "
			"		LEFT JOIN ( "
						// (c.haag 2007-06-15 10:46) - PLID 26344 - Expanded the subquery to include related details
			"			SELECT DISTINCT SourceTemplateTopicID FROM EMRTemplateTopicsT WHERE TemplateID = ? "
			"		) SourceSubQ ON EMRTemplateDetailsT.EMRTemplateTopicID = SourceSubQ.SourceTemplateTopicID "
			"		WHERE TemplateID = ? OR SourceSubQ.SourceTemplateTopicID IS NOT NULL "
			"	) EMRInfoSubQ ON EmrDataT.EmrInfoID = EMRInfoSubQ.ActiveEmrInfoID "
			") LinkSubQ ON EmrItemLinkedDataT.EmrLinkID = LinkSubQ.EmrLinkID "
			"ORDER BY EmrItemLinkedDataT.EmrLinkID ");
		AddParameterLong(pCmd, "TemplateID", m_nMasterID);
		AddParameterLong(pCmd, "TemplateID", m_nMasterID);
		prs = CreateRecordset(pCmd);
	} else {
		//DRT 7/17/2007 - PLID 26716 - Switched to be a parameterized query for better server efficiency
		// (c.haag 2007-10-02 11:44) - PLID 26240 - We must search all EMN's for the EMR we are in
		// (c.haag 2008-03-28 12:57) - PLID 29428 - I'm effectively encapsulating the existing query inside
		// a select query so that we include EmrItemLinkedDataT items corresponding to both EmrInfo records
		// rather than just the one that corresponds to the patient chart for every "pair".
		// (j.dinatale 2013-03-13 16:29) - PLID 55631 - switched to joins instead of the nested INs
		_CommandPtr pCmd = OpenParamQuery(pCon, 
			"SELECT EmrItemLinkedDataT.ID, EmrItemLinkedDataT.EmrLinkID, EmrItemLinkedDataT.EmrDataID, EmrDataT.EmrDataGroupID, EmrDataT.EmrInfoID "
			"FROM EmrItemLinkedDataT "
			"INNER JOIN EmrDataT ON EmrItemLinkedDataT.EmrDataID = EmrDataT.ID "
			"INNER JOIN ("
			"	SELECT EmrItemLinkedDataT.EmrLinkID FROM EmrItemLinkedDataT "
			"	INNER JOIN EmrDataT ON EmrItemLinkedDataT.EmrDataID = EmrDataT.ID "
			"	INNER JOIN ( "
			"		SELECT DISTINCT EmrInfoID "
			"		FROM EMRDetailsT "
			"		INNER JOIN ( "
			"			SELECT ID FROM EmrMasterT WHERE EMRGroupID IN (SELECT EMRGroupID FROM EMRMasterT WHERE ID = ?) "
			"		) EMRMasterSubQ ON EMRDetailsT.EMRID = EMRMasterSubQ.ID "
			"	) EMRSubQ ON EmrDataT.EmrInfoID = EMRSubQ.EmrInfoID "
			") LinkSubQ ON EmrItemLinkedDataT.EmrLinkID = LinkSubQ.EmrLinkID "
			"ORDER BY EmrItemLinkedDataT.EmrLinkID ");
		AddParameterLong(pCmd, "EMNID", m_nMasterID);
		prs = CreateRecordset(pCmd);
	}
	FieldsPtr f = prs->Fields;
	while (!prs->eof) {
		EmrLinkedDataItem item;
		item.m_nID = AdoFldLong(f, "ID");
		item.m_nEmrLinkID = AdoFldLong(f, "EmrLinkID");
		item.m_nEmrDataID = AdoFldLong(f, "EmrDataID");
		item.m_nEmrDataGroupID = AdoFldLong(f, "EmrDataGroupID");
		item.m_nEmrInfoID = AdoFldLong(f, "EmrInfoID");
		m_aEmrLinkedDataItems.Add(item);
		prs->MoveNext();
	}

	m_bAllEmrItemLinkedDataItemsLoaded = TRUE;
}

//////////////////////////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////////////////////////

CEMNDetail* CEMNLoader::GetLiveDetail(const CEMNDetail* pDetail) const
{
	//
	// (c.haag 2007-08-02 16:54) - PLID 26833 - This function returns a Live detail given
	// a detail, regardless of whether it's Live or Internal. For EMN loader use only. 
	//
	// Additionally, when this is called, the contents of the return value MUST NOT BE
	// MODIFIED. It is only the pointer itself we are interested in.
	//
	// (c.haag 2007-08-03 10:56) - Don't use GetInitialLoadDetail...because it
	// requires you be in the main thread.
	//
	CEMNDetail* pResult = NULL;
	if (pDetail) {
		// (c.haag 2007-08-03 11:10) - If the detail is not owned by the Loader, then it
		// must be owned by the EMN. That makes it, by definition, a Live detail. So, just
		// return itself.
		if (!pDetail->IsOwnedByEMNLoader()) {
			return (CEMNDetail*)pDetail;
		}
		if (m_bLoadFromTemplate) {
			m_mapLiveDetailsByTemplateID.Lookup(pDetail->m_nEMRTemplateDetailID, pResult);
		} else {
			m_mapLiveDetailsByID.Lookup(pDetail->m_nEMRDetailID, pResult);
		}
		ASSERT(pResult);
	}
	return pResult;
}

//////////////////////////////////////////////////////////////////////
// Query building functions
//////////////////////////////////////////////////////////////////////

//DRT 7/18/2007 - PLID 26726 - Query is now parameterized, and returns the parameters added to the end of the array
// (z.manning 2011-10-25 08:39) - PLID 39401 - Changed return type to SQL fragment
CSqlFragment CEMNLoader::BuildAllRelatedEmrInfoTQ() const
{
	// (c.haag 2007-05-08 13:06) - PLID 25790 - This builds a subquery for use in a larger query that requires all
	// used EmrInfoID's for the loader
	// (c.haag 2007-06-15 10:43) - PLID 26344 - Expanded the subquery to include related details
	// (c.haag 2007-08-01 18:49) - PLID 26908 - GetEnsureEmrInfoDefaultsMapSql no longer calls this function.
	// If you change the query below, also check that function too!
	// (a.walling 2013-03-25 14:13) - PLID 55845 - Declaring some parameters as local vars, use @LoadingID
	if (m_bLoadFromTemplate) {
		// (c.haag 2007-07-31 09:05) - This query is slower than the one below it.
		/*
		return FormatString("SELECT ActiveEmrInfoID FROM EmrTemplateDetailsT LEFT JOIN EmrInfoMasterT ON EmrInfoMasterID = EmrInfoMasterT.ID "
			"WHERE TemplateID = ? "
			"OR EMRTemplateDetailsT.EMRTemplateTopicID IN (SELECT SourceTemplateTopicID FROM EMRTemplateTopicsT WHERE TemplateID = ?)"
			);
		*/
		// (s.dhole 2013-06-27 09:02) - PLID 57335 replace IN cluase with temp table
		return CSqlFragment(
			"SELECT ActiveEmrInfoID \r\n"
			"FROM EmrTemplateDetailsT \r\n"
			"LEFT JOIN EmrInfoMasterT ON EmrInfoMasterID = EmrInfoMasterT.ID \r\n"
			" WHERE EMRTemplateTopicID IN (	SELECT ID FROM @EMRTemplateTopicTemp)  \r\n"
		);
	} else {
		// (c.haag 2007-06-27 12:27) - PLID 26473 - Filter out undeleted details with deleted topics
		//Add parameters, in correct order
		return CSqlFragment(
			"SELECT EmrInfoID FROM EMRDetailsT \r\n"
			"INNER JOIN EmrTopicsT ON EmrTopicsT.ID = EMRDetailsT.EmrTopicID \r\n"
			"WHERE EMRDetailsT.EMRID = @LoadingID AND EMRDetailsT.Deleted = 0 AND EmrTopicsT.Deleted = 0 \r\n"
		);
	}
}

//DRT 7/18/2007 - PLID 26726 - Query is now parameterized, and returns the parameters added to the end of the array
CString CEMNLoader::BuildAllRelatedListDetailQ(CDWordArray *paryQueryParams) const
{
	// (c.haag 2007-05-08 13:19) - PLID 26459 - This builds a subquery for use in a larger query that requires
	// all used list details
	// (c.haag 2007-06-15 10:43) - PLID 26344 - Expanded the subquery to include related details
	if (m_bLoadFromTemplate) {
		//Add parameters, in correct order
		paryQueryParams->Add(m_nMasterID);
		paryQueryParams->Add(m_nMasterID);

		// (a.walling 2013-03-27 13:57) - PLID 55911 - Use INNER joins, and templateID filtering
		// same as BuildAllRelatedEmrInfoTQ, it is faster to join with EMRTemplateTopicsT and filter via the UNION rather than the subqueries
		return FormatString("SELECT EMRTemplateDetailsT.ID FROM EMRTemplateDetailsT "
			"INNER JOIN EMRTemplateTopicsT "
				"ON EMRTemplateDetailsT.EmrTemplateTopicID = EMRTemplateTopicsT.ID "
			"INNER JOIN EmrInfoMasterT ON EmrInfoMasterID = EmrInfoMasterT.ID "
			"INNER JOIN EmrInfoT ON EmrInfoT.ID = EmrInfoMasterT.ActiveEmrInfoID "
			"WHERE EmrInfoT.DataType IN (%d,%d) "
			"AND EMRTemplateDetailsT.EMRTemplateTopicID IN ( "
				"SELECT ID FROM EMRTemplateTopicsT WHERE TemplateID = ? "
				"UNION "
				"SELECT SourceTemplateTopicID FROM EMRTemplateTopicsT WHERE TemplateID = ? "
			") "
			,  eitSingleList, eitMultiList);
	} else {
		// (c.haag 2007-06-25 12:03) - PLID 26446 - Get results from a patient chart
		// (c.haag 2007-06-27 12:27) - PLID 26473 - Filter out undeleted details with deleted topics
		//Add parameters, in correct order
		paryQueryParams->Add(m_nMasterID);

		return FormatString("SELECT EMRDetailsT.ID FROM EMRDetailsT "
			"INNER JOIN EmrTopicsT ON EmrTopicsT.ID = EMRDetailsT.EmrTopicID "
			"LEFT JOIN EmrInfoT ON EmrInfoT.ID = EmrInfoID "
			"WHERE EmrInfoT.DataType IN (%d,%d) AND EMRDetailsT.EMRID = ? AND EMRDetailsT.Deleted = 0 "
			"AND EMRTopicsT.Deleted = 0 "
			, eitSingleList, eitMultiList);
	}
}

//DRT 1/23/2008 - PLID 28698 - Similar to the others, but for images
CString CEMNLoader::BuildAllRelatedImageDetailQ(CDWordArray *paryQueryParams) const
{
	// (c.haag 2007-05-08 13:19) - PLID 26459 - This builds a subquery for use in a larger query that requires
	// all used list details
	// (c.haag 2007-06-15 10:43) - PLID 26344 - Expanded the subquery to include related details
	if (m_bLoadFromTemplate) {
		//Add parameters, in correct order
		paryQueryParams->Add(m_nMasterID);
		paryQueryParams->Add(m_nMasterID);

		// (a.walling 2013-03-27 13:57) - PLID 55911 - Use INNER joins, and templateID filtering
		// same as BuildAllRelatedEmrInfoTQ, it is faster to join with EMRTemplateTopicsT and filter via the UNION rather than the subqueries
		return FormatString("SELECT EMRTemplateDetailsT.ID FROM EMRTemplateDetailsT "
			"INNER JOIN EMRTemplateTopicsT "
				"ON EMRTemplateDetailsT.EmrTemplateTopicID = EMRTemplateTopicsT.ID "
			"INNER JOIN EmrInfoMasterT ON EmrInfoMasterID = EmrInfoMasterT.ID "
			"INNER JOIN EmrInfoT ON EmrInfoT.ID = ActiveEmrInfoID "
			"WHERE EmrInfoT.DataType IN (%d) "
			"AND EMRTemplateDetailsT.EMRTemplateTopicID IN ( "
				"SELECT ID FROM EMRTemplateTopicsT WHERE TemplateID = ? "
				"UNION "
				"SELECT SourceTemplateTopicID FROM EMRTemplateTopicsT WHERE TemplateID = ? "
			") "
			,  eitImage);
	} else {
		// (c.haag 2007-06-25 12:03) - PLID 26446 - Get results from a patient chart
		// (c.haag 2007-06-27 12:27) - PLID 26473 - Filter out undeleted details with deleted topics
		//Add parameters, in correct order
		paryQueryParams->Add(m_nMasterID);

		return FormatString("SELECT EMRDetailsT.ID FROM EMRDetailsT "
			"INNER JOIN EmrTopicsT ON EmrTopicsT.ID = EMRDetailsT.EmrTopicID "
			"LEFT JOIN EmrInfoT ON EmrInfoT.ID = EmrInfoID "
			"WHERE EmrInfoT.DataType IN (%d) AND EMRDetailsT.EMRID = ? AND EMRDetailsT.Deleted = 0 "
			"AND EMRTopicsT.Deleted = 0 "
			, eitImage);
	}
}


//DRT 7/18/2007 - PLID 26726 - Query is now parameterized, and returns the parameters added to the end of the array
CString CEMNLoader::BuildAllRelatedTableDetailQ(CDWordArray *paryQueryParams) const
{
	// (c.haag 2007-05-08 13:24) - PLID 25761 - This builds a subquery for use in a larger query that requires
	// all used table details
	// (c.haag 2007-06-15 10:43) - PLID 26344 - Expanded the subquery to include related details
	if (m_bLoadFromTemplate) {
		//Add parameters, in correct order
		paryQueryParams->Add(m_nMasterID);
		paryQueryParams->Add(m_nMasterID);

		// (a.walling 2013-03-27 13:57) - PLID 55911 - Use INNER joins, and templateID filtering
		// same as BuildAllRelatedEmrInfoTQ, it is faster to join with EMRTemplateTopicsT and filter via the UNION rather than the subqueries
		// ALSO previously this did not group the conditional!!
		// so it was doing WHERE DataType = table and templateid = ? or SourceTemplateID = ?
		// insteead of WHERE DataType = table and (templateid = ? or SourceTemplateID = ?)
		return FormatString("SELECT EMRTemplateDetailsT.ID FROM EMRTemplateDetailsT "
			"INNER JOIN EMRTemplateTopicsT "
				"ON EMRTemplateDetailsT.EmrTemplateTopicID = EMRTemplateTopicsT.ID "
			"INNER JOIN EmrInfoMasterT ON EmrInfoMasterID = EmrInfoMasterT.ID "
			"INNER JOIN EmrInfoT ON EmrInfoT.ID = ActiveEmrInfoID "
			"WHERE EmrInfoT.DataType IN (%d) "
			"AND EMRTemplateDetailsT.EMRTemplateTopicID IN ( "
				"SELECT ID FROM EMRTemplateTopicsT WHERE TemplateID = ? "
				"UNION "
				"SELECT SourceTemplateTopicID FROM EMRTemplateTopicsT WHERE TemplateID = ? "
			") "
			, eitTable);
	} else {
		//Add parameters, in correct order
		paryQueryParams->Add(m_nMasterID);

		return FormatString("SELECT EMRDetailsT.ID FROM EMRDetailsT "
			"LEFT JOIN EmrInfoT ON EmrInfoT.ID = EmrInfoID "
			"WHERE EmrInfoT.DataType IN (%d) AND EMRID = ? ", eitTable);
	}
}

void CEMNLoader::FlushPendingEMNDetailWrites()
{
	// (j.jones 2008-10-30 17:04) - PLID 31869 - renamed to ASSERT_IN_SAFE_THREAD
	ASSERT_IN_SAFE_THREAD("CEMNLoader::FlushPendingEMNDetailWrites()");

	// (c.haag 2007-07-27 10:06) - PLID 26833 - This is called by CEMRTopic::PostLoadDetails when
	// a topic has been fully loaded.
	//
	// While an EMN is initially loaded, it needs access to all of its details even though they
	// have not been completely loaded. During this time, it accesses this loader object for its
	// details; specifically, it accesses an array of "Live" details. Because of this, the EMN
	// loader cannot freely access them in the loading thread. It must instead do all the work
	// it can do without actually touching the "Live" detail data in the thread, and then add
	// the detail to m_mapPendingDetails so that when the topic is fully loaded and control is
	// given back to the main thread, we can finish the "Live" detail load without worrying about
	// thread safety issues.
		
	// (c.haag 2007-07-10 09:47) - PLID 26595 - Take exclusive access to internal details
	CHoldEMNLoaderMutex mhDetails(&m_mtxInternalDetails);

	// (c.haag 2007-07-10 14:48) - PLID 25239 - The details we loaded since the last time
	// that FlushPendingEMNDetailWrites was called exist in m_mapPendingDetails. Traverse
	// through the map.
	// (z.manning 2010-03-12 11:53) - PLID 37412 - Please not that we must process these pending
	// details in order starting at index zero of the array.
	for(int nPendingDetailIndex = 0; nPendingDetailIndex < m_arypPendingDetails.GetSize(); nPendingDetailIndex++)
	{
		CEMNDetail* pPendingDetail = m_arypPendingDetails.GetAt(nPendingDetailIndex);
		if (pPendingDetail != NULL) {
			// (c.haag 2007-07-19 12:16) - PLID 26833 - Do all the work in here now
			FinalizeDetailTransitionToEMN(pPendingDetail);
		} 
		else {
			ASSERT(FALSE); // This should never happen
		}
	}
	m_arypPendingDetails.RemoveAll();
}

void CEMNLoader::FinalizeDetailTransitionToEMN(CEMNDetail* pLiveDetail)
{
	// (j.jones 2008-10-30 17:04) - PLID 31869 - renamed to ASSERT_IN_SAFE_THREAD
	ASSERT_IN_SAFE_THREAD("CEMNLoader::FinalizeDetailTransitionToEMN");

	//
	// (c.haag 2007-07-27 09:50) - PLID 26833 - This function was split from FlushPendingEMNDetailWrites.
	// The purpose of this function is to load the state of a "Live" detail, thereby rendering it fully
	// loaded and ready to assign to a CEMN topic.
	//
	CPreloadedDetail* pPreloadedInternalDetail = NULL;
	if (m_bLoadFromTemplate) {
		m_mapAllDetailsByTemplateID.Lookup(pLiveDetail->m_nEMRTemplateDetailID, pPreloadedInternalDetail);
	} else {
		m_mapAllDetailsByID.Lookup(pLiveDetail->m_nEMRDetailID, pPreloadedInternalDetail);
	}
	CEMNDetail* pInternalDetail = (pPreloadedInternalDetail) ? pPreloadedInternalDetail->m_pDetail : NULL;
	if (NULL == pInternalDetail) {
		ASSERT(FALSE);
		ThrowNxException("FinalizeDetailTransitionToEMN could not find the internal detail with ID %d!",
			(m_bLoadFromTemplate) ? pLiveDetail->m_nEMRTemplateDetailID : pLiveDetail->m_nEMRDetailID);
	}

	// Verify that the live detail's state is still empty as it was when created. If it's not, we risk 
	// overwriting good state data!
	// (c.haag 2010-06-02 12:37) - PLID 38974 - It's actually OK for the pLiveDetail state to not be empty.
	// This can happen if you have an unspawned multi-select template detail on one topic that is dependent
	// on a narrative in another topic such that the narrative is loaded first. In that case though, the
	// pLiveDetail state should be identical to the internal detail's state. If they are not, then we need to
	// throw an exception.
	if (VT_EMPTY != pLiveDetail->GetStateVarType()) {
		if (VARCMP_EQ != VariantCompare(&pLiveDetail->GetState(), &pInternalDetail->GetState())) {
			if (m_bLoadFromTemplate) {
				ThrowNxException("CEMNLoader::FinalizeDetailTransitionToEMN attempted to change the existent state of a live detail. Template Detail ID = %d.", pLiveDetail->m_nEMRTemplateDetailID);
			} else {
				ThrowNxException("CEMNLoader::FinalizeDetailTransitionToEMN attempted to change the existent state of a live detail. Detail ID  = %d.", pLiveDetail->m_nEMRDetailID);
			}
		}
	}

	// By this point, the internal detail should have had its state loaded. Here, all we do to complete
	// the load of the "Live" detail is to assign it the state of the internal detail. The exception to
	// this is system info items; they will be loaded later unless they were already saved.
	if (VT_EMPTY == pInternalDetail->GetStateVarType()) {
		//TES 6/6/2008 - PLID 29416 - We no longer handle these items specially.
		//ASSERT(pInternalDetail->IsCurrentMedicationsTable() || pInternalDetail->IsAllergiesTable());
		ASSERT(FALSE);
	}
	else {
		pLiveDetail->SetState( pInternalDetail->GetState() );
		// (z.manning 2011-11-16 17:26) - PLID 38130 - Also need to set the remembered detail ID
		if(pInternalDetail->m_nRememberedDetailID != -1) {
			pLiveDetail->m_nRememberedDetailID = pInternalDetail->m_nRememberedDetailID;
		}
	}

	// The Live detail is now totally loaded. Go ahead and release our reference to the Live detail.
	// This will leave it with one reference and that reference belongs to the EMN.
	// (a.walling 2009-10-12 10:23) - PLID 36023 - We should have no special knowledge of the reference count, let alone release based on it!
	/*
	if (2 == pLiveDetail->GetRefCnt()) {
		// (c.haag 2007-08-15 11:26) - PLID 26833 - Store the Live detail in our Finalized detail map
		// so we know never to try to finalize it again
		m_mapFinalizedDetails[pLiveDetail] = TRUE;
		pLiveDetail->Release();
	} else {
		// This should never happen. Don't release it in this case.
		ASSERT(FALSE);
	}
	*/

	//#pragma TODO("PLID 36023 - This has been working for a client, but I'm still wary. We have the pointer to a CEMNDetail object as the hash map key; we have no guarantee that this object could be deleted the pointer be garbage or even a new, unrelated detail. Perhaps we need to maintain a reference in the finalized detail map as well.")
	BOOL bDummy;
	if (!m_mapFinalizedDetails.Lookup(pLiveDetail, bDummy)) {
		m_mapFinalizedDetails[pLiveDetail] = TRUE;
		pLiveDetail->__AddRef("FinalizedDetailMap");
	}
	m_mapFinalizedDetails[pLiveDetail] = TRUE;
	//pLiveDetail->Release();
	// (a.walling 2009-10-12 16:05) - PLID 36023 - Release our live detail reference
	// (a.walling 2009-10-23 13:01) - PLID 36023 - Instead, now we increase the reference, so now we have one for the map and the all detail array
	//pLiveDetail->__Release("FinalizeDetailTransition");
}

BOOL CEMNLoader::AssertIfPostInitialLoadIssuesExist()
{
	// (c.haag 2007-08-04 09:44) - PLID 26945 - This function will raise a debug assertion if an 
	// unexpected condition is met after the intial load has finished. Returns TRUE if it is safe to
	// delete this object.
	BOOL bSafeToDelete = TRUE;

	// Check for locking issues
	long nTopicLocks = GetTopicsMutex()->GetLockCount();
	long nEmrInfoLocks = GetEMRInfoMutex()->GetLockCount();
	long nDataActionLocks = GetEMRDataActionsMutex()->GetLockCount();
	long nLinkedDataItemLocks = GetEMRLinkedDataItemsMutex()->GetLockCount();
	long nImageHotSpotActionLocks = GetImageHotSpotActionsMutex()->GetLockCount();
	
	ASSERT(0 == nTopicLocks);
	ASSERT(0 == nEmrInfoLocks);
	ASSERT(0 == nDataActionLocks);
	ASSERT(0 == nLinkedDataItemLocks);
	ASSERT(0 == nImageHotSpotActionLocks);
	
	// Check for data issues
	CArray<CEMNDetail*,CEMNDetail*> arDetails;
	const int nLiveDetails = m_apLiveDetails.GetSize();
	m_pEMN->GenerateTotalEMNDetailArray(&arDetails);
	const int nEMRDetails = arDetails.GetSize();
	int i;

	// Assert our reference count is one. When a topic is pending its load, it will
	// add a reference to the loader object. When the topic is done loading, it will release
	// its reference. The only remaining reference should belong to the CEMN object which
	// created us.
	ASSERT(1 == m_nRefCnt);

	// Assert that the reference count of every Live detail is one. When we created all
	// of the Live details at the very start of the initial load, we added a reference
	// to all of them. This brought them all to a total of two. We then released a 
	// reference for each one we loaded as we loaded it. If any Live details were left
	// unloaded, that could be trouble. At a minimum, there will be a memory leak.

	// (a.walling 2007-10-12 11:15) - PLID 27017 - It is possible the refcount may have increased
	// due to a statechange from the narrative during the load, so if the ref is two, ensure it
	// exists in the preview's DetailsPendingUpdate array.
#ifdef _DEBUG
	for (i=0; i < nLiveDetails; i++) {
		CEMNDetail* pLiveDetail = m_apLiveDetails[i];
		// (a.walling 2009-10-12 10:03) - PLID 36023 - HULK SMASH!@*(&(
		// Do not perform any special behavior based on the reference count! Instead, reference count appropriately.
		/*
		if (pLiveDetail->GetRefCnt() == 2) {
			// check the DetailsPendingUpdate array
			if (pLiveDetail->m_pParentTopic != NULL && pLiveDetail->m_pParentTopic->GetParentEMN() != NULL) {
				CEmrTreeWnd* pInterfaceWnd = (CEmrTreeWnd*)pLiveDetail->m_pParentTopic->GetParentEMN()->GetInterface();
				if (pInterfaceWnd) {
					if (!pInterfaceWnd->IsDetailInPendingUpdateArray(pLiveDetail)) {
						ASSERT(FALSE);
					}
				}
			}
		} else if (pLiveDetail->GetRefCnt() != 1) {
			ASSERT(FALSE);
		}
		*/
	}
#endif

	// Now assert that the EMN owns no Internal details (this will cause crashes if otherwise)
	for (i=0; i < nEMRDetails; i++) {
		CEMNDetail* pDetail = arDetails[i];
		CEMNDetail* pSourceDetail = pDetail->GetSourceDetail();

		// Check the source detail object
		if (NULL != pSourceDetail) {
			if (pSourceDetail->IsOwnedByEMNLoader()) {
				// (c.haag 2007-08-04 09:50) - If this is TRUE, that means an internal detail
				// object which was never supposed to persist outside this loader object somehow
				// made its way onto the EMN. This should never happen under working circumstances.
				// This also means that we can't delete this object, or else the EMN will be stuck
				// with a dangling EMN detail pointer.
				ASSERT(FALSE);
				bSafeToDelete = FALSE;
			}
		}

		// Check linked details
		TableElement te;
		const int nElements = pDetail->GetTableElementCount();
		for (int j=0; j < nElements; j++) {
			pDetail->GetTableElementByIndex(j, te);
			if (NULL != te.m_pLinkedDetail) {
				if (te.m_pLinkedDetail->IsOwnedByEMNLoader()) {
					// (c.haag 2007-08-04 09:50) - If this is TRUE, that means an internal detail
					// object which was never supposed to persist outside this loader object somehow
					// made its way onto the EMN. This should never happen under working circumstances.
					// This also means that we can't delete this object, or else the EMN will be stuck
					// with a dangling EMN detail pointer.
					ASSERT(FALSE);
					bSafeToDelete = FALSE;
				}
			}
		}
	}
	return bSafeToDelete;
}

void CEMNLoader::FreeEmrInfoMap()
{
	// (c.haag 2007-09-10 13:03) - PLID 27340 - Frees memory related to EmrInfo objects
	// (c.haag 2007-04-24 08:51) - PLID 26463 - Clean up our EMR info map
	POSITION pos = m_mapInfoItems.GetStartPosition();
	while (pos != NULL) {
		CEMRInfoItem* pInfo;
		long nEMRInfoID;
		m_mapInfoItems.GetNextAssoc( pos, nEMRInfoID, pInfo );
		if (pInfo) delete pInfo;
	}
	m_mapInfoItems.RemoveAll();
}

void CEMNLoader::FreeRememberedDetailMaps()
{
	// (c.haag 2007-09-10 13:03) - PLID 27340 - Frees memory related to Remembered details
	// (c.haag 2007-04-24 13:56) - PLID 25758 - Clean up our "Remember for
	// this patient" candidate map
	POSITION pos = m_mapRememberForPatientCandidates.GetStartPosition();
	while (pos != NULL) {
		CRememberCandidate* pCandidate;
		CString str;
		m_mapRememberForPatientCandidates.GetNextAssoc( pos, str, (LPVOID&)pCandidate );
		if (pCandidate) {
			delete pCandidate;
		}
	}
	m_mapRememberForPatientCandidates.RemoveAll();

	// (j.jones 2008-09-22 16:42) - PLID 31408 - added m_mapRememberForEMRCandidates
	pos = m_mapRememberForEMRCandidates.GetStartPosition();
	while (pos != NULL) {
		CRememberCandidate* pCandidate;
		CString str;
		m_mapRememberForEMRCandidates.GetNextAssoc( pos, str, (LPVOID&)pCandidate );
		if (pCandidate) {
			delete pCandidate;
		}
	}
	m_mapRememberForEMRCandidates.RemoveAll();

	// (c.haag 2007-04-24 13:56) - PLID 25758 - Clean up our remembered detail map
	pos = m_mapRememberedDetails.GetStartPosition();
	while (pos != NULL) {
		CPreloadedDetail* pDetail;
		long nDetailID;
		m_mapRememberedDetails.GetNextAssoc( pos, nDetailID, pDetail );
		if (pDetail) {
			delete pDetail;
		}
	}
	m_mapRememberedDetails.RemoveAll();
}

void CEMNLoader::FreeActionsMap()
{
	// (c.haag 2007-09-10 13:03) - PLID 27340 - Frees memory related to actions
	// (c.haag 2007-04-25 09:53) - PLID 25774 - Clean up our action map
	POSITION pos = m_mapActions.GetStartPosition();
	while (pos != NULL) {
		MFCArray<EmrAction>* pa;
		long nEMRDataID;
		m_mapActions.GetNextAssoc( pos, nEMRDataID, pa );
		if (pa) delete pa;
	}
	m_mapActions.RemoveAll();
}

// (z.manning, 02/21/2008) - PLID 28690 - Free memory used for the image hot spot actions map.
void CEMNLoader::FreeImageHotSpotActionsMap()
{
	POSITION pos = m_mapImageHotSpotActions.GetStartPosition();
	while (pos != NULL) {
		MFCArray<EmrAction>* paryActions;
		long nHotSpotID;
		m_mapImageHotSpotActions.GetNextAssoc(pos, nHotSpotID, paryActions);
		if(paryActions != NULL) {
			delete paryActions;
			paryActions = NULL;
		}
	}
	m_mapImageHotSpotActions.RemoveAll();
}

// (z.manning 2009-02-16 11:46) - PLID 33070
void CEMNLoader::FreeTableDropdownItemActionsMap()
{
	POSITION pos = m_mapTableDropdownItemActions.GetStartPosition();
	while (pos != NULL) {
		MFCArray<EmrAction>* paryActions;
		long nDropdownID;
		m_mapTableDropdownItemActions.GetNextAssoc(pos, nDropdownID, paryActions);
		if(paryActions != NULL) {
			delete paryActions;
			paryActions = NULL;
		}
	}
	m_mapTableDropdownItemActions.RemoveAll();
}

void CEMNLoader::FreeSelectMaps()
{
	// (c.haag 2007-09-10 13:03) - PLID 27340 - Frees memory related to default selections
	// (c.haag 2007-06-26 12:13) - PLID 26446 - Clean up our select map
	POSITION pos = m_mapSelect.GetStartPosition();
	while (pos != NULL) {
		CArray<long,long>* pDataIDs;
		long nDetailID;
		m_mapSelect.GetNextAssoc( pos, nDetailID, pDataIDs );
		if (pDataIDs) delete pDataIDs;
	}
	m_mapSelect.RemoveAll();

	//DRT 1/23/2008 - PLID 28698 - Cleanup our hotspot map
	pos = m_mapHotSpots.GetStartPosition();
	while(pos != NULL) {
		CArray<long, long> *pary;
		long nDetailID = -1;
		m_mapHotSpots.GetNextAssoc(pos, nDetailID, pary);
		if(pary) {
			delete pary;
		}
	}
	m_mapHotSpots.RemoveAll();

	// (c.haag 2007-04-24 08:51) - PLID 26459 - Clean up our template select map
	pos = m_mapTemplateSelect.GetStartPosition();
	while (pos != NULL) {
		CArray<long,long>* pDataIDs;
		long nTemplateDetailID;
		m_mapTemplateSelect.GetNextAssoc( pos, nTemplateDetailID, pDataIDs );
		if (pDataIDs) delete pDataIDs;
	}
	m_mapTemplateSelect.RemoveAll();

	//DRT 1/23/2008 - PLID 28698 - Cleanup our hotspot map
	pos = m_mapTemplateHotSpots.GetStartPosition();
	while (pos != NULL) {
		CArray<long,long>* pDataIDs;
		long nTemplateDetailID;
		m_mapTemplateHotSpots.GetNextAssoc( pos, nTemplateDetailID, pDataIDs );
		if (pDataIDs) {
			delete pDataIDs;
		}
	}
	m_mapTemplateHotSpots.RemoveAll();
}

void CEMNLoader::Retire()
{
	// (c.haag 2007-09-10 13:00) - PLID 27340 - When this is called, it means this loader object will
	// no longer be used to actively spawn or add data to the parent EMN. This will free all memory 
	// not directly related to topic and detail objects.
	FreeActionsMap();
	FreeEmrInfoMap();
	FreeRememberedDetailMaps();
	FreeSelectMaps();
	FreeImageHotSpotActionsMap(); // (z.manning, 03/10/2008) - PLID 28690
	FreeTableDropdownItemActionsMap(); // (z.manning 2009-02-16 11:55) - PLID 33070
}

DWORD CEMNLoader::GetEstimatedMinimumMemoryUsage() const
{
	// (c.haag 2007-05-15 12:45) - Returns the estimated minimum memory usage, in bytes, of this object
	DWORD nEstimatedSize = 
		(m_mapInfoItems.GetCount() * (sizeof(long) + sizeof(CEMRInfoItem))) +					// m_mapInfoItems
		(m_apAllDetails.GetSize() * sizeof(CPreloadedDetail)) +									// m_apAllDetails
		(m_mapAllDetailsByID.GetCount() * (sizeof(long) + sizeof(CPreloadedDetail*))) +			// m_mapAllDetailsByID
		(m_mapAllDetailsByTemplateID.GetCount() * (sizeof(long) + sizeof(CPreloadedDetail*))) + // m_mapAllDetailsByTemplateID
		(m_mapUsedDetails.GetCount() * (sizeof(CEMNDetail*) + sizeof(BOOL))) +					// m_mapUsedDetails
		(m_mapDetailTopics.GetCount() * (sizeof(long) + sizeof(CArray<CEMNDetail*,CEMNDetail*>))) +	// m_mapDetailTopics
		(m_mapEmrTemplateTopicActions.GetCount() * (sizeof(long) + sizeof(PreloadedTopicAction))) +	// m_mapEmrTemplateTopicActions
		(m_apAllTopics.GetSize() * sizeof(CPreloadedTopic)) +									// m_apAllTopics
		(m_mapAllTopicsByID.GetCount() * (sizeof(long) + sizeof(CPreloadedTopic*))) +			// m_mapAllTopicsByID
		(m_apAllTemplateTopics.GetSize() * sizeof(CPreloadedTemplateTopic)) +					// m_apAllTemplateTopics
		(m_mapAllTemplateTopicsByID.GetCount() * (sizeof(long) + sizeof(CPreloadedTemplateTopic*))) + // m_mapAllTemplateTopicsByID
		(m_mapSelect.GetCount() * (sizeof(long) + sizeof(CArray<long,long>))) +					// m_mapSelect
		(m_mapHotSpots.GetCount() * (sizeof(long) + sizeof(CArray<long,long>))) +				// m_mapHotSpots
		(m_mapTemplateSelect.GetCount() * (sizeof(long) + sizeof(CArray<long,long>))) +			// m_mapTemplateSelect
		(m_mapTemplateHotSpots.GetCount() * (sizeof(long) + sizeof(CArray<long,long>))) +		// m_mapTemplateHotSpots
		(m_mapTable.GetCount() * (sizeof(long) + sizeof(CString))) +							// m_mapTable
		(m_mapTemplateTable.GetCount() * (sizeof(long) + sizeof(CString))) +					// m_mapTemplateTable
		(m_mapRememberForPatientCandidates.GetCount() * (sizeof(long) + sizeof(CRememberCandidate))) +	// m_mapRememberForPatientCandidates
		(m_mapRememberForEMRCandidates.GetCount() * (sizeof(long) + sizeof(CRememberCandidate))) +	// m_mapRememberForEMRCandidates
		(m_mapRememberedDetails.GetCount() * (sizeof(long) + sizeof(CPreloadedDetail*))) +		// m_mapRememberedDetails
		(m_mapActions.GetCount() * (sizeof(long) + sizeof(MFCArray<EmrAction>))) +		// m_mapActions
		(m_mapImageHotSpotActions.GetCount() * (sizeof(long) + sizeof(MFCArray<EmrAction>))) +	// m_mapImageHotSpotActions
		// (z.manning 2009-02-16 11:49) - PLID 33070 - Table dropdown item actions
		(m_mapTableDropdownItemActions.GetCount() * (sizeof(long) + sizeof(MFCArray<EmrAction>))) +	// m_mapTableDropdownItemActions
		sizeof(CEMNLoader);

	POSITION pos;
	pos = m_mapInfoItems.GetStartPosition();
	while (pos != NULL) {
		CEMRInfoItem* pInfoItem;
		long nEMRInfoID;
		m_mapInfoItems.GetNextAssoc( pos, nEMRInfoID, pInfoItem );
		if (pInfoItem) {
			if (NULL != pInfoItem->m_paDataItems) {
				nEstimatedSize += pInfoItem->m_paDataItems->GetSize() * sizeof(EmrDataItem) + sizeof(CArray<EmrDataItem,EmrDataItem&>);
			}
			//DRT 1/21/2008 - PLID 28603 - Added array of image hotspots
			if (NULL != pInfoItem->m_paryHotSpotItems) {
				nEstimatedSize += pInfoItem->m_paryHotSpotItems->GetSize() * sizeof(EmrImageHotSpotItem) + sizeof(CArray<EmrImageHotSpotItem*, EmrImageHotSpotItem*>);
			}
		}
	}

	return nEstimatedSize;
}

//DRT 7/26/2007 - PLID 26835 - Setting this flag allows you to turn off spawning.  When it is true, 
//	EnsureEmrActionData() will do nothing.  This is currently used when you load an existing EMR from
//	data, and there is no point in spawning actions.
void CEMNLoader::SetIgnoreLoadingActions(BOOL bIgnoreLoadingActions)
{
	m_mtxIgnoreActions.Lock();
	m_bIgnoreLoadingActions = bIgnoreLoadingActions;
	m_mtxIgnoreActions.Unlock();
}

BOOL CEMNLoader::GetIgnoreLoadingActions()
{
	BOOL bRetVal;
	m_mtxIgnoreActions.Lock();
	bRetVal = m_bIgnoreLoadingActions;
	m_mtxIgnoreActions.Unlock();
	return bRetVal;
}

//DRT 1/23/2008 - PLID 28698 - Populates a map that associates EMR Detail IDs with image hotspot
//	selections.  Only details that exist in m_pAllDetails are used in this map.
void CEMNLoader::EnsureHotSpotMap(_Connection *lpCon)
{
	if (m_bHotSpotMapPopulated) return;

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	// (c.haag 2007-04-27 09:29) - Run the query, and populate the map. The key is EMRDetailID,
	// and the values are CArray<long,long>'s of EMRHotSpotID's
	// (c.haag 2007-07-17 15:05) - PLID 26708 - We now get the SQL statement from another function
	//DRT 7/18/2007 - PLID 26726 - Parameterized the query, added a function parameter to get the query parameters in order
	CDWordArray aryQueryParams;
	CString strLoadSql = GetEnsureHotSpotMapSql(&aryQueryParams);
	if(!strLoadSql.IsEmpty()) {
		_CommandPtr pCmd = OpenParamQuery(pCon, strLoadSql);
		for(int i = 0; i < aryQueryParams.GetSize(); i++) {
			//The text name is not used for anything
			AddParameterLong(pCmd, "ID", aryQueryParams.GetAt(i));
		}
		_RecordsetPtr prs = CreateRecordset(pCmd);

		while (!prs->eof) {
			ReadHotSpotMapRecord(prs);
			prs->MoveNext();
		}
	}

	m_bHotSpotMapPopulated = TRUE;
}

//DRT 1/23/2008 - PLID 28698 - This function pulls data from a query
// which was intended to populate m_mapHotSpots with data
void CEMNLoader::ReadHotSpotMapRecord(_RecordsetPtr& prs)
{
	const long nDetailID = AdoFldLong(prs, "EMRDetailID");
	const long nEMRHotSpotID = AdoFldLong(prs, "EMRImageHotSpotID");
	CArray<long,long>* paDataIDs = NULL;
	if (!m_mapHotSpots.Lookup(nDetailID, paDataIDs)) {
		paDataIDs = new CArray<long,long>;
		m_mapHotSpots[nDetailID] = paDataIDs;
	}
	if (paDataIDs) {
		paDataIDs->Add(nEMRHotSpotID);
	} else {
		ThrowNxException("CEMNLoader::ReadHotSpotMapRecord() failed to allocate paDataIDs object!");
	}
}

//DRT 1/23/2008 - PLID 28698 - Returns the text for a query to be used with ReadHotSpotMapRecord.
CString CEMNLoader::GetEnsureHotSpotMapSql(CDWordArray *paryQueryParams) const
{
	// (c.haag 2007-05-08 15:13) - PLID 26446 - Use BuildAllRelatedImageDetailQ instead of building
	// a list of EMR info ID's
	if (m_bLoadFromTemplate)  {
		// (c.haag 2007-08-29 15:02) - PLID 25758 - We must make sure not to double-populate the select map. 
		// Build a map of all detail ID's that already have entries in the select map, and ensure we omit them
		// from the query.
		CMap<long,long,BOOL,BOOL> mapPopulatedDetailIDs;
		GetHotSpotMapDetailIDs(mapPopulatedDetailIDs);

		// (c.haag 2007-07-17 11:49) - PLID 26708 - Use a different query for templates. The only
		// patient chart table details we should be loading here are ones that we're recalling
		// remembered values for. Go through all the preloaded details and build a list of all the
		// list details we're remembering
		CArray<long,long> anSelectDetailIDs;
		const int nDetails = m_apAllDetails.GetSize();
		for (int i=0; i < nDetails; i++) {
			CPreloadedDetail* pPreloadedDetail = m_apAllDetails[i];
			if (!m_bIsTemplate && VT_I4 == pPreloadedDetail->m_vInfoRememberedDetailID.vt) {
				if(pPreloadedDetail->m_pDetail->m_EMRInfoType == eitImage) {
					const long nDetailID = VarLong(pPreloadedDetail->m_vInfoRememberedDetailID);
					BOOL bDummy;
					if (!mapPopulatedDetailIDs.Lookup(nDetailID, bDummy)) {
						anSelectDetailIDs.Add(nDetailID);
					}
				}
			}
		}
		if (0 == anSelectDetailIDs.GetSize()) {
			return "";
		} else {
			return FormatString("SELECT EMRDetailID, EMRImageHotSpotID FROM EMRHotSpotSelectT "
				"WHERE EMRDetailID IN (%s)", ArrayAsString(anSelectDetailIDs));
		}
	} else {
		//No specific parameters in this section, pass our array down
		return FormatString("SELECT EMRDetailID, EMRImageHotSpotID FROM EMRHotSpotSelectT "
			"WHERE EMRDetailID IN (%s)", BuildAllRelatedImageDetailQ(paryQueryParams));
	}
}

//DRT 1/23/2008 - PLID 28698 - This map populates mapDetails with ID's used in the hot spot map
void CEMNLoader::GetHotSpotMapDetailIDs(CMap<long,long,BOOL,BOOL>& mapDetails) const
{
	POSITION pos = m_mapHotSpots.GetStartPosition();
	while (pos != NULL) {
		CArray<long,long>* pDummy;
		long nDetailID;
		m_mapHotSpots.GetNextAssoc( pos, nDetailID, pDummy );
		mapDetails[nDetailID] = TRUE;
	}
}

//DRT 1/23/2008 - PLID 28698 - Populates a map that associates EMR template detail
// ID's with default image hotspot selections. Only details that exist in m_apAllDetails
// are used in this map
void CEMNLoader::EnsureTemplateHotSpotMap(_Connection *lpCon)
{
	if (m_bTemplateHotSpotMapPopulated) return;

	_ConnectionPtr pCon;
	if(lpCon) pCon = lpCon;
	else pCon = GetRemoteData();

	// (c.haag 2007-04-24 08:41) - Run the query, and populate the map. The key is EMRTemplateDetailID,
	// and the values are CArray<long,long>'s of EMRImageHotSpotID's
	//DRT 7/18/2007 - PLID 26726 - Parameterized the query, added a function parameter to get the query parameters in order
	CDWordArray aryQueryParams;
	CString strLoadSql = GetEnsureTemplateHotSpotMapSql(&aryQueryParams);
	_CommandPtr pCmd = OpenParamQuery(pCon, strLoadSql);
	for(int i = 0; i < aryQueryParams.GetSize(); i++) {
		//The text name is not used for anything
		AddParameterLong(pCmd, "ID", aryQueryParams.GetAt(i));
	}
	_RecordsetPtr prs = CreateRecordset(pCmd);
	while (!prs->eof) {
		ReadTemplateHotSpotMapRecord(prs);
		prs->MoveNext();
	}
	m_bTemplateHotSpotMapPopulated = TRUE;
}

//DRT 1/23/2008 - PLID 28698 - This function pulls data from a query
// which was intended to populate m_mapTemplateHotSpots with data
void CEMNLoader::ReadTemplateHotSpotMapRecord(_RecordsetPtr& prs)
{
	const long nTemplateDetailID = AdoFldLong(prs, "EMRDetailID");
	const long nEMRHotSpotID = AdoFldLong(prs, "EMRImageHotSpotID");
	CArray<long,long>* paDataIDs = NULL;
	if (!m_mapTemplateHotSpots.Lookup(nTemplateDetailID, paDataIDs)) {
		paDataIDs = new CArray<long,long>;
		m_mapTemplateHotSpots[nTemplateDetailID] = paDataIDs;
	}
	if (paDataIDs) {
		paDataIDs->Add(nEMRHotSpotID);
	} else {
		ThrowNxException("CEMNLoader::ReadTemplateHotSpotMapRecord() failed to allocate paDataIDs object!");
	}
}

//DRT 1/23/2008 - PLID 28698 - This function returns query text to be later used with ReadTemplateHotSpotMapRecord
//No parameters in this particular section, pass our array down
CString CEMNLoader::GetEnsureTemplateHotSpotMapSql(CDWordArray *paryQueryParams) const
{
	return FormatString("SELECT EMRDetailID, EMRImageHotSpotID FROM EMRHotSpotTemplateSelectT "
		"WHERE EMRDetailID IN (%s)",
		// (c.haag 2007-05-08 15:13) - PLID 26459 - Use BuildAllRelatedListDetailQ instead of building
		// a list of EMR info ID's
		BuildAllRelatedImageDetailQ(paryQueryParams));
}

// (j.jones 2010-02-12 14:19) - PLID 37318 - called whenever any detail is added to the EMN, to determine
// whether the detail is a SmartStamp image or table, and links to its related item if it exists or
// creates a new table if the image was just added
// (z.manning 2010-03-12 11:54) - PLID 37412 - Renamed this function to indicate it ensures smart stamp
// link for internal (not live) details in the loader.
void CEMNLoader::EnsureInternalDetailSmartStampLinks(CEMNDetail *pDetail)
{
	try {

		//remember that when loading existing content, one of the items
		//is going to exist prior to the other, so each item needs to
		//search for its counterpart when added, and we must not make
		//a new table unless we are absolutely certain the image is new

		if(pDetail->m_EMRInfoType == eitImage && pDetail->m_bSmartStampsEnabled && pDetail->m_nChildEMRInfoMasterID != -1)
		{
			//this detail is a SmartStamp image linked to a table

			//store as pImage for ease of code readability
			CEMNDetail *pImage = pDetail;

			if(pImage->GetSmartStampTableDetail()) {
				//we are already linked to a table, so simply confirm it is also linked to us
				pImage->GetSmartStampTableDetail()->EnsureSmartStampImageDetail(pImage);
				return;
			}

			if(!m_bIsTemplate && pImage->m_nChildEMRDetailID != -1) {
				//see if we have the ID yet for the table
				CPreloadedDetail* pPreloadedInternalDetail = NULL;
				CEMNDetail *pTable = NULL;
				m_mapAllDetailsByID.Lookup(pImage->m_nChildEMRDetailID, pPreloadedInternalDetail);
				if(pPreloadedInternalDetail != NULL) {
					pTable = pPreloadedInternalDetail->m_pDetail;
				}
				// (z.manning 2011-01-21 10:41) - PLID 42338 - Support multiple images per smart stamp table
				if(pTable != NULL && pImage->m_nChildEMRInfoMasterID == pTable->m_nEMRInfoMasterID) {
					//link them up
					pImage->SetSmartStampTableDetail(pTable);
					pTable->EnsureSmartStampImageDetail(pImage);
				}

				//if we don't find anything, return, it means the table hasn't been loaded yet
				return;
			}
			else if(pImage->m_nChildEMRTemplateDetailID != -1) {
				//this could be a template, or loading from a template
				CPreloadedDetail* pPreloadedInternalDetail = NULL;
				CEMNDetail *pTable = NULL;
				m_mapAllDetailsByTemplateID.Lookup(pImage->m_nChildEMRTemplateDetailID, pPreloadedInternalDetail);
				if(pPreloadedInternalDetail) {
					pTable = pPreloadedInternalDetail->m_pDetail;
				}
				// (z.manning 2011-01-21 10:45) - PLID 42338 - Support multiple images per smart stamp table
				if(pTable != NULL) {
					if(pImage->m_nChildEMRInfoMasterID == pTable->m_nEMRInfoMasterID) {
						//link them up
						pImage->SetSmartStampTableDetail(pTable);
						pTable->EnsureSmartStampImageDetail(pImage);
					}
					else {
						//the SmartStamp setup has changed, these are no longer linked together,
						//and have not been updated in the template, so break the link
						pImage->m_nChildEMRTemplateDetailID = -1;
						// (z.manning 2011-01-25 16:11) - PLID 42336 - Parent detail IDs are now deprecated
						//pTable->m_nParentEMRTemplateDetailID = -1;
					}
				}

				//If m_nChildEMRTemplateDetailID is still not -1, return, this means the table hasn't been loaded yet,
				//but if m_nChildEMRTemplateDetailID is -1, we will continue, although this function will not
				//attempt to add a new table so continuing currently does nothing
				if(pImage->m_nChildEMRTemplateDetailID != -1) {
					return;
				}
			}
			// (z.manning 2012-07-03 17:50) - PLID 51025 - Also check for an "orphaned" smart stamp image, meaning we have
			// an existing smart stamp image that is somehow not linked to a smart stamp table. This situation really shoudn't
			// be happening, but somehow it has so let's handle it.
			else if(pImage->IsOrphanedSmartStampImage())
			{
				CEMNDetail *pTable = GetSmartStampTableDetailByInfoMasterID(pImage->m_nChildEMRInfoMasterID, pImage->m_nEMRInfoMasterID);
				if(pTable != NULL) {
					pImage->SetSmartStampTableDetail(pTable);
					pTable->EnsureSmartStampImageDetail(pImage);
				}
			}
			
			//truly new items shouldn't exist in the loader
		}
		else if(pDetail->m_EMRInfoType == eitTable)
		{
			//this detail is a table, does it need to be linked to an image?

			//store as pTable for ease of code readability
			CEMNDetail *pTable = pDetail;

			if(pTable->GetSmartStampImageDetails()->GetCount() > 0) {
				//we are already linked to a image, so simply confirm it is also linked to us
				pTable->GetSmartStampImageDetails()->SetSmartStampTableDetail(pTable);
				return;
			}

			if(!m_bIsTemplate && pTable->m_nEMRDetailID != -1)
			{
				// (z.manning 2011-01-25 16:28) - PLID 42336 - Now support multiple images for one smart stamp table
				CEMNDetailArray aryParentDetails;
				GetParentDetailsByDetailID(pTable, &aryParentDetails);
				for(int nParentDetailIndex = 0; nParentDetailIndex < aryParentDetails.GetSize(); nParentDetailIndex++)
				{
					//see if we have the ID yet for the image
					CEMNDetail *pImage = aryParentDetails.GetAt(nParentDetailIndex);
					if(pImage != NULL) {

						// (j.jones 2013-01-14 13:29) - PLID 54170 - If this is not an image, do not link it to a table.
						//This would be bad data if a non-image is somehow linked to a table.
						if(pImage->m_EMRInfoType != eitImage) {
							//assert, because while we are gracefully handling this situation,
							//we should still know when it happens to make sure there are no
							//new, unhandled causes of this data
							ASSERT(FALSE);

							//disable this setting
							pImage->m_bSmartStampsEnabled = FALSE;
							continue;
						}

						if(pImage->GetSmartStampTableDetail() == NULL
							&& pImage->m_bSmartStampsEnabled && pImage->m_nChildEMRInfoMasterID == pTable->m_nEMRInfoMasterID) {
							//link them up
							pImage->SetSmartStampTableDetail(pTable);
							pTable->EnsureSmartStampImageDetail(pImage);
						}
					}
				}
			}
			else if(pTable->m_nEMRTemplateDetailID != -1)
			{
				// (z.manning 2011-01-25 16:28) - PLID 42336 - Now support multiple images for one smart stamp table
				CEMNDetailArray aryParentDetails;
				GetParentDetailsByTemplateDetailID(pTable->m_nEMRTemplateDetailID, &aryParentDetails);
				for(int nParentDetailIndex = 0; nParentDetailIndex < aryParentDetails.GetSize(); nParentDetailIndex++)
				{
					//this could be a template, or loading from a template
					CEMNDetail *pImage = aryParentDetails.GetAt(nParentDetailIndex);
					if(pImage != NULL && pImage->GetSmartStampTableDetail() == NULL) {
						if(pImage->m_bSmartStampsEnabled && pImage->m_nChildEMRInfoMasterID == pTable->m_nEMRInfoMasterID) {
							//link them up
							pImage->SetSmartStampTableDetail(pTable);
							pTable->EnsureSmartStampImageDetail(pImage);
						}
						else {
							//the SmartStamp setup has changed, these are no longer linked together,
							//and have not been updated in the template, so break the link
							pImage->m_nChildEMRTemplateDetailID = -1;
							// (z.manning 2011-01-25 16:28) - PLID 42336 - Parent ID is deprecated
							//pTable->m_nParentEMRTemplateDetailID = -1;

							//Logically, we should re-ensure the image so it can try to auto-create the table,
							//but that is not going to happen in the loader. For posterity, try it anyways,
							//even though it will fundamentally do nothing.
							EnsureInternalDetailSmartStampLinks(pImage);
						}
					}
				}
			}

			//truly new items shouldn't exist in the loader
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-03-10 11:14) - PLID 37318 - added a special loader version of ReconfirmSmartStampLinks,
// which compares live details by template detail ID
// (c.haag 2010-06-29 12:09) - PLID 39404 - Added pInternalDetail. This should only be assigned when
// the detail being processed was potentially spawned.
void CEMNLoader::ReconfirmSmartStampLinks_ByLiveDetail(CEMNDetail *pDetail, CEMNDetail* pInternalDetail)
{
	// (z.manning 2011-01-21 10:49) - PLID 42338 - Support multiple images per smart stamp table
	CEMNDetailArray arySmartStampImages;
	arySmartStampImages.Copy(*(pDetail->GetSmartStampImageDetails()));
	for(int nImageIndex = 0; nImageIndex < arySmartStampImages.GetCount(); nImageIndex++)
	{
		CEMNDetail *pImage = arySmartStampImages.GetAt(nImageIndex);
		CEMNDetail *pLiveImage = NULL;
		if(pLiveImage == NULL && pImage->GetID() != -1) {
			m_mapLiveDetailsByID.Lookup(pImage->GetID(), pLiveImage);
		}
		if(pLiveImage == NULL && pImage->m_nEMRTemplateDetailID != -1) {			
			if (NULL != pInternalDetail) 
			{
				// (c.haag 2010-06-29 11:20) - PLID 39404 - If the detail was manufactured, use a different method to
				// find its spouse because manufactured details do not appear in the live array. In this method, we go
				// through the list of all known internal smart stamp pairs to find the one which corresponds to pInternalDetail.
				// From there, we can do a much more focused search to find pDetail's spouse.
				for (int i=0; i < m_apSmartStampTemplateItemPairs.GetSize(); i++) 
				{
					CSmartStampImageTablePairs* pPair = m_apSmartStampTemplateItemPairs[i];
					if (pPair->m_pInternalTable == pInternalDetail) 
					{
						// If we get here, we've found the internal smart stamp table-image pair that corresponds to pInternalDetail.
						// Make sure that this table exists in m_apManufacturedImages and that it's tied to an image if it exists.
						CEMNDetailArray *paryLiveImages = pPair->GetMatchingImageArray(pDetail);
						if(paryLiveImages != NULL) {
							pLiveImage = paryLiveImages->GetDetailFromEmrInfoMasterID(pDetail->m_nEMRInfoMasterID);
						}
					}
				}
			}
			else {
				m_mapLiveDetailsByTemplateID.Lookup(pImage->m_nEMRTemplateDetailID, pLiveImage);
			}
		}
		if(pLiveImage != NULL) {
			//reassign to the live image
			pDetail->EnsureSmartStampImageDetail(pLiveImage);
			// (z.manning 2012-02-10 09:02) - PLID 48063 - We just updated the smart stamp links on the live details so also
			// reconfirm the live detail links.
			pDetail->ReconfirmSmartStampLinks();
		}
		else {
			// (z.manning, 2011-03-14) - PLID 42796 - We did not find a live detail. However, we cannot leave
			// an internal detail linked to this live table so clear out the link. It's possible that the live
			// image just hasn't been loaded yet, but that's fine as the links will be re-established when
			// it does get loaded.
			pDetail->ClearSmartStampImageDetailByEmrInfoMasterID(pImage->m_nEMRInfoMasterID);
		}
	}

	if(pDetail->GetSmartStampTableDetail())
	{
		CEMNDetail *pTable = pDetail->GetSmartStampTableDetail();
		CEMNDetail *pLiveTable = NULL;
		if(pLiveTable == NULL && pTable->GetID() != -1) {
			m_mapLiveDetailsByID.Lookup(pTable->GetID(), pLiveTable);
		}
		if(pLiveTable == NULL && pTable->m_nEMRTemplateDetailID != -1) {
			if (NULL != pInternalDetail) {
				// (c.haag 2010-06-29 11:20) - PLID 39404 - If the detail was manufactured, use a different method to
				// find its spouse because manufactured details do not appear in the live array. In this method, we go
				// through the list of all known internal smart stamp pairs to find the one which corresponds to pInternalDetail.
				// From there, we can do a much more focused search to find pDetail's spouse.
				for (int i=0; i < m_apSmartStampTemplateItemPairs.GetSize(); i++) 
				{
					CSmartStampImageTablePairs* pPair = m_apSmartStampTemplateItemPairs[i];
					if (pPair->m_paryInternalImages->Contains(pInternalDetail)) 
					{
						// If we get here, we've found the internal smart stamp table-image pair that corresponds to pInternalDetail.
						// Make sure that this image exists in m_apManufacturedImages and that it's tied to a table if it exists.
						pLiveTable = pPair->GetMatchingTable(pTable->GetSmartStampImageDetails());
					}
				}
			}
			else {
				m_mapLiveDetailsByTemplateID.Lookup(pTable->m_nEMRTemplateDetailID, pLiveTable);
			}
		}
		if(pLiveTable != NULL) {
			//reassign to the live table
			pDetail->SetSmartStampTableDetail(pLiveTable);
			// (z.manning 2012-02-10 09:02) - PLID 48063 - We just updated the smart stamp links on the live details so also
			// reconfirm the live detail links.
			pDetail->ReconfirmSmartStampLinks();
		}
		else {
			// (z.manning, 2011-03-14) - PLID 42796 - We did not find a live detail. However, we cannot leave
			// an internal detail linked to this live image so clear out the link. It's possible that the live
			// table just hasn't been loaded yet, but that's fine as the links will be re-established when
			// it does get loaded.
			pDetail->SetSmartStampTableDetail(NULL);
		}
	}
}

// (z.manning 2010-03-12 11:51) - PLID 37412
void CEMNLoader::AddPendingDetail(CEMNDetail *pPendingDetail)
{
	if(pPendingDetail != NULL)
	{
		// (z.manning 2010-03-12 11:51) - PLID 37412 - Make sure we do not add the same detail twice here
		for(int nPendingDetailIndex = 0; nPendingDetailIndex < m_arypPendingDetails.GetSize(); nPendingDetailIndex++) {
			CEMNDetail* pTemp = m_arypPendingDetails.GetAt(nPendingDetailIndex);
			if(pTemp == pPendingDetail) {
				return;
			}
		}

		// (z.manning 2011-01-25 16:34) - PLID 42336 - Support multiple images for one smart stamp table
		CEMNDetailArray aryParentDetails, aryParentTemplateDetails;
		GetParentDetailsByDetailID(pPendingDetail, &aryParentDetails);
		GetParentDetailsByTemplateDetailID(pPendingDetail->m_nEMRTemplateDetailID, &aryParentTemplateDetails);
		if(aryParentDetails.GetSize() > 0 || aryParentTemplateDetails.GetSize() > 0) {
			// (z.manning 2010-03-12 11:51) - PLID 37412 - This detail has a parent detail so put it at the end
			// of the pending detail array.
			m_arypPendingDetails.Add(pPendingDetail);
		}
		else {
			m_arypPendingDetails.InsertAt(0, pPendingDetail);
		}
	}
}

// (z.manning 2011-02-23 09:21) - PLID 42549
void CEMNLoader::EnsureLinkedSmartStampImageStatesLoaded(CEMNDetail *pSmartStampTable, ADODB::_Connection *lpCon)
{
	if(!pSmartStampTable->IsSmartStampTable()) {
		return;
	}

	for(int nImageIndex = 0; nImageIndex < pSmartStampTable->GetSmartStampImageDetails()->GetSize(); nImageIndex++)
	{
		CEMNDetail *pImage = pSmartStampTable->GetSmartStampImageDetails()->GetAt(nImageIndex);
		if(!WasStateLoaded(pImage) && !pImage->IsStateSet(NULL))
		{
			// (z.manning 2011-07-19 11:07) - PLID 44621 - We may be using the loader when adding a new detail so check for that too.
			if(m_bLoadFromTemplate || pImage->m_nEMRDetailID == -1) {
				CEMR* pEmr = m_pEMN->GetParentEMR();
				if(pEmr == NULL) {
					AfxThrowNxException("CEMNLoader::EnsureLinkedSmartStampImageStatesLoaded - NULL EMR when trying to load default detail state for EmrInfoT.ID %li", pImage->m_nEMRInfoID);
				}
				// (z.manning 2011-11-16 12:56) - PLID 38130 - Pass in parameter for remembered detail ID
				pImage->SetState(LoadEMRDetailStateDefault(pImage->m_nEMRInfoID, pEmr->GetPatientID(), pEmr->GetID(), pImage->m_nEMRTemplateDetailID, lpCon, pImage->m_nRememberedDetailID));
			}
			else {
				pImage->SetState(LoadEMRDetailState(pImage->m_nEMRDetailID, pImage->m_EMRInfoType, lpCon));
			}
			SetStateLoaded(pImage);
			SetUsed(pImage);
		}
	}
}

// (z.manning 2011-04-06 16:12) - PLID 43140
void CEMNLoader::PreloadEmrDetailListOrders(ADODB::_RecordsetPtr &rsDetailListOrders)
{
	for(; !rsDetailListOrders->eof && AdoFldLong(rsDetailListOrders,"EmrID") == m_nMasterID; rsDetailListOrders->MoveNext())
	{
		const long nDetailID = AdoFldLong(rsDetailListOrders, "EmrDetailID");
		const long nDataGroupID = AdoFldLong(rsDetailListOrders, "EmrDataGroupID");
		const long nOrderIndex = AdoFldLong(rsDetailListOrders, "OrderIndex");
		// (j.jones 2011-04-29 10:51) - PLID 43122 - added IsFloated
		const BOOL bIsFloated = AdoFldBool(rsDetailListOrders, "IsFloated", FALSE);

		CEmrDataGroupOrderArray *paryDetailOrder = NULL;
		if(!m_mapDetailIDToOrder.Lookup(nDetailID, paryDetailOrder)) {
			paryDetailOrder = new CEmrDataGroupOrderArray;
		}

		EmrDataGroupOrder order;
		order.nDataGroupID = nDataGroupID;
		order.nOrderIndex = nOrderIndex;
		order.bIsFloated = bIsFloated;

		paryDetailOrder->Add(order);
		m_mapDetailIDToOrder.SetAt(nDetailID, paryDetailOrder);
	}
}

// (z.manning 2011-04-06 16:58) - PLID 43140 - This function will return the data group sort order
// array for a given detail ID.
CEMNLoader::CEmrDataGroupOrderArray* CEMNLoader::GetOrderArrayByDetailID(const long nDetailID)
{
	CEmrDataGroupOrderArray *paryDataGroupOrder;
	if(m_mapDetailIDToOrder.Lookup(nDetailID, paryDataGroupOrder)) {
		return paryDataGroupOrder;
	}

	return NULL;
}


// (a.walling 2013-03-18 09:34) - PLID 55725 - various predicates for sorting data items
namespace {

struct CompareEmrDataItems_BySortOrder
{
	bool operator()(const CEMNLoader::EmrDataItem& l, const CEMNLoader::EmrDataItem& r) const
	{
		// compare sort order
		if (l.m_nSortOrder < r.m_nSortOrder) {
			return true;
		} else {
			return false;
		}
	}
};

struct CompareEmrDataItems_ByData
{
	bool operator()(const CEMNLoader::EmrDataItem& l, const CEMNLoader::EmrDataItem& r) const
	{
		// compare data
		int cmp = l.m_strData.CompareNoCase(r.m_strData);
		if (cmp < 0) {
			return true;
		} else {
			return false;
		}
	}
};

struct CompareEmrDataItems_FloatedByData
{
	bool operator()(const CEMNLoader::EmrDataItem& l, const CEMNLoader::EmrDataItem& r) const
	{
		// partition floating from non-floating:

		// if l is floating and r is not, l < r
		if (l.m_nFloatCount != -1 && r.m_nFloatCount == -1) {
			return true;
		// if l is not floating and r is, l > r
		} else if (l.m_nFloatCount == -1 && r.m_nFloatCount != -1) {
			return false;
		}

		// compare data
		int cmp = l.m_strData.CompareNoCase(r.m_strData);
		if (cmp < 0) {
			return true;
		} else {
			return false;
		}
	}
};

struct CompareEmrDataItems_FloatCountBySortOrder
{
	bool operator()(const CEMNLoader::EmrDataItem& l, const CEMNLoader::EmrDataItem& r) const
	{
		if (l.m_nFloatCount != r.m_nFloatCount) {
			// if l.floatCount > r.floatCount, l < r
			return l.m_nFloatCount > r.m_nFloatCount;
		}

		return l.m_nSortOrder < r.m_nSortOrder;
	}
};

struct CompareEmrDataItems_SystemTableByData
{
	bool operator()(const CEMNLoader::EmrDataItem& l, const CEMNLoader::EmrDataItem& r) const
	{
		// first all Row types by data, then everything else by sort order

		if (l.m_nListType == 2) {
			if (r.m_nListType != 2) {
				return true;
			}
			
			// compare data
			int cmp = l.m_strData.CompareNoCase(r.m_strData);
			if (cmp < 0) {
				return true;
			} else {
				return false;
			}
		} else {
			if (r.m_nListType == 2) {
				return false;
			}

			return l.m_nSortOrder < r.m_nSortOrder;
		}
	}
};

}

// (a.walling 2013-03-18 09:34) - PLID 55725 - sort data items
void CEMNLoader::CEMRInfoItem::SortDataItems()
{
	if (!m_paDataItems || m_paDataItems->IsEmpty()) return;

	CArray<EmrDataItem,EmrDataItem&>& dataItems = (CArray<EmrDataItem,EmrDataItem&>&)(*m_paDataItems);

	if (m_DataType == eitSingleList || m_DataType == eitMultiList) {
		// consider floated items
		if (m_bAutoAlphabetizeListData) {			
			// sort all by !!float count, then data
			boost::sort(dataItems, CompareEmrDataItems_FloatedByData());
		} else {
			// sort all by float count desc, then sortorder
			boost::sort(dataItems, CompareEmrDataItems_FloatCountBySortOrder());
		}
	} else {
		if (m_bAutoAlphabetizeListData) {
			if (m_DataSubType == 1 || m_DataSubType == 2) {
				// system item
				boost::sort(dataItems, CompareEmrDataItems_SystemTableByData());
			} else {
				// compare data
				boost::sort(dataItems, CompareEmrDataItems_ByData());
			}
		} else {
			// compare sort order
			boost::sort(dataItems, CompareEmrDataItems_BySortOrder());
		}
	}
}

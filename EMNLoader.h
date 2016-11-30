// EMNLoader.h: interface for the CEMNLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMNLOADER_H__FF53C951_5B77_44CE_A77A_3FB62098C313__INCLUDED_)
#define AFX_EMNLOADER_H__FF53C951_5B77_44CE_A77A_3FB62098C313__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "EmrUtils.h"
#include "EmrInfoCommonListCollection.h"  // (c.haag 2011-03-17) - PLID 42895
#include "WoundCareCalculator.h"

// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray

// (j.jones 2013-05-16 15:41) - PLID 56596 - changed the EMNDetail.h include into a forward declare
class CEMNDetail;

class EMRTopicLoadInfo;

// (c.haag 2007-02-26 17:16) - PLID 24949 - This class maintains an array
// of CEMNDetail objects used only when loading EMR topics in a thread
// as a result of messages posted from either CEMRTopic::LoadFromTopicFields
// or CEMRTopic::LoadFromTemplateTopicFields.
//
// (c.haag 2007-04-23 17:37) - PLID 24949 - New updated description
//
//
// This class object exists during the initial load of a single EMN, and is
// responsible for:
//
// - Loading existing patient charts
// - Loading existing templates
// - Creating new patient charts from templates
//
// as fast as possible by following these rules:
// 
// - Minimize the number of round trips to the server
// - Load only the data that needs to be loaded
//
//
// This object is instantiated in LoadFromEmnRecordsets when loading existing charts,
// and LoadFromTemplateRecordsets when creating new charts from templates or loading
// templates. It uses reference counting -- one per topic plus one for the EMN. When
// the final topic is loaded, the last topic dereference will put the reference count
// at one. The final release is done in CEMN::PostInitialLoad().
//
// Every CEMNDetail and CEMRTopic object accessed by this object may or will genuinely
// belong to the EMN. So, don't discard any pointers of any kind unless you know for
// certain that they are temporary. 
//
//
// (c.haag 2007-04-23 17:40) - Version 1: This object is instantiated in the main
// thread. After instantiation, this object is immediately populated with ALL the
// undeleted details of a chart or template of the EMN of interest from data. This is
// also known as the "detail preload." After that, this object is passed to a worker 
// thread which loads topic data for each topic individually. The thread also loads the
// states of details whose states we have to know now rather than later on when a user
// goes to their parent topic. When loading templates, we use our own version of 
// LoadEMRDetailStateDefault for much faster loading.
//

//
// A parting analogy: Think of CEMNLoader as a local library, and the SQL server as the
// library of congress. When a segment of code wants to go to the library of congress, 
// it should go to the local library first, and save itself a nice long drive. However,
// the local libary, ideally, should not have any unread books, and we need to be careful
// that the local library takes up as little real estate as possible.
//

//
// At the start of the initial load, before the loading thread is launched, we populate
// the EMN loader's "Internal" detail array. The internal detail array is the set of all
// details, including potentially spawned details, that will or may be needed in the initial
// load of a chart or template. These are owned exclusively by the EMN loader, and are never
// assigned to the EMN. These details are used to assign data to "Live" details (explained
// below) which are assigned to the EMN. They are also used in determining which details are
// related to other details (narratives, linked table columns), and which details belong to
// which topics. Not only do we include details for this template/emn -- we also include the
// details of other templates which we spawn into this one. The purpose of this is to avoid
// standalone, redundant querying for those details in CEMN::ProcessEMRActions. It's possible
// that they will never be used; but we unfortunately will not know for sure until we load
// detail state information to see whether list elements are selected.
// 

//
// At the start of the initial load, before the loading thread is launched, we populate
// the EMN loader's "Live" detail array. The Live detail array is the set of all details
// with no source action ID's that will indisputably be added to the EMN or template during
// the initial load. When the EMN has a need to iterate through all details during the initial
// load to, say, populate narratives or spawning-related data, it will iterate through the
// Live detail array. The reason we do this is to keep the initial load as similar to the
// 8300 version. What 8300 did was do all the loading -before- any iterating took place.
// We cannot wait for everything to load at once, and because things load asynchronously
// while the user is interactive with the first topic, we must do this to maximize consistency
// with the prior release, and to ensure that as many interdependent details are synchronized
// as possible (i.e. hyperlinks in narratives that point to other details, and linked details
// in tables)
// 

// (c.haag 2007-07-09 10:42) - Here's a summary of how an initial load works:
// (c.haag 2007-07-27 10:15) - Version 2 - Took out intermediateary "Loaded" detail array
// (c.haag 2007-08-08 12:14) - Version 3 - Updated definition of Live details
//
// Part 1: Details and topics loaded in bulk (PreloadEmrDetails / PreloadEmrTemplateDetails)
//		m_apAllDetails is filled with CPreloadedDetail objects							<-- Owned by CEMNLoader
//		m_apAllCEMNDetails is filled with CEMNDetail objects that will be on the EMN	<-- Owned by CEMNLoader
//		m_apLiveDetails is filled with copies of CEMNDetail objects	that will be on		<-- Owned by CEMN
//			the EMN which have no relationship with spawning and appear on the EMN
//			regardless of whether ProcessEmrActions is called.
//		[EMN-owned details] is empty													<-- Owned by CEMN
//
//		m_apAllTopics or m_apAllTemplateTopics is filled with CPreloadedTopic objects	<-- Owned by CEMNLoader
//		regardless of whether they have anything to do with spawning
//		[EMN-owned topics] is empty														<-- Owned by CEMN
//		
//
// Part 2: Topics are loaded by the worker thread one at a time
//		m_apAllDetails' content is accessed by the loader
//		m_apAllCEMNDetails' content is accessed by the loader
//		m_apLiveDetails content is updated at the end of the topic load in the main thread
//		[EMN-owned details] starts getting filled with the same details in m_apLiveDetails as well
//		as spawned details which are created on a need-to-create basis by the EMN loader
//
//		m_apAllTopics' content is accessed by the loader
//		[EMN-owned topics] starts getting filled with brand new previously non-existent topics
//
//		* In this part, CEMN::GetTotalDetailCount, CEMN::GetDetail and CEMN::GenerateTotalEMNDetailArray
//		access only the CEMNLoader for details.
//
// Part 3: Initial load just ended
//		m_apAllDetails is dormant but still existent
//		m_apAllCEMNDetails is dormant but still existent
//		m_apLiveDetails is now dormant but still existent
//		[EMN-owned details] is populated
//
//		m_apAllTopics or m_apAllTemplateTopics is dormant but still existent
//		[EMN-owned topics] is populated
//
//
// 
// *** A graphical flowchart is available at \\yoda\Shared\Development\Documentation\Projects\EMR Speed 8400\emrflow.bmp
//

// (c.haag 2007-07-03 09:50) - PLID 26523 - This is basically like a mutex, except
// we do lock counting for debugging and safety checking
// (c.haag 2007-07-05 15:18) - PLID 26595 - We now include the thread ID for further
// safety checks. We are also friends with CEMNLoaderMultiMutex so that it can access
// internal mutex data
class CEMNLoaderMutex
{
	friend class CEMNLoaderMultiMutex;
private:
	CMutex m_mtx;
	CMutex m_mtxInternalData;
	DWORD m_nOwnerThreadID;
	int m_nLockCount;

public:
	CEMNLoaderMutex() {
		m_nLockCount = 0;
		m_nOwnerThreadID = 0;
	}
	void Lock() {
		CSyncObject* ppMutexes[] = { &m_mtx, &m_mtxInternalData };
		CMultiLock ml(ppMutexes, 2);
		DWORD nCurrentThreadID = GetCurrentThreadId();
		ml.Lock(); // Lock the main mutex and the lock count protection mutex
		if (m_nLockCount > 0) { ASSERT(m_nOwnerThreadID == nCurrentThreadID); }
		else { m_nOwnerThreadID = nCurrentThreadID; }
		m_nLockCount++; // Update the lock count
		m_mtx.Lock(); // Increment m_mtx's lock count
		ml.Unlock(); // Release the lock count protection mutex and dereference m_mtx's lock count
	}
	void Unlock() {
		CSyncObject* ppMutexes[] = { &m_mtx, &m_mtxInternalData };
		CMultiLock ml(ppMutexes, 2);
		ml.Lock(); // Lock the main mutex and the lock count protection mutex
		m_nLockCount--; // Update the lock count
		ASSERT(m_nLockCount >= 0); // Safety check
		m_mtx.Unlock(); // Deference m_mtx's lock
		ml.Unlock(); // Release both the lock count protection and m_mtx
	}
	int GetLockCount() { 
		int nLockCount;
		m_mtxInternalData.Lock();
		nLockCount = m_nLockCount;
		m_mtxInternalData.Unlock();
		return nLockCount;
	}
	BOOL HasBeenLockedInCurrentThread() {
		// (c.haag 2007-07-05 15:19) - PLID 26595 - Returns TRUE if this mutex
		// has been locked in the current thread
		BOOL bResult = FALSE;
		m_mtxInternalData.Lock();
		if (m_nLockCount > 0 && m_nOwnerThreadID == GetCurrentThreadId()) {
			bResult = TRUE;
		}
		m_mtxInternalData.Unlock();
		return bResult;
	}
};

// (c.haag 2007-07-03 09:25) - PLID 26523 - Gains access to a mutex
// and then releases it in its destruction. The purpose is to guarantee
// that a function or curly-braced code section cannot gracefully be 
// exited without letting go of the mutex.
class CHoldEMNLoaderMutex
{
private:
	CEMNLoaderMutex* m_pMtx;
public:
	CHoldEMNLoaderMutex(CEMNLoaderMutex* pMtx) {
		if (NULL != (m_pMtx = pMtx)) {
			m_pMtx->Lock();
		} else {
			ASSERT(FALSE);
		}
	}
	~CHoldEMNLoaderMutex()
	{
		Release();
	}
	void Release()
	{
		if (m_pMtx) m_pMtx->Unlock();
		m_pMtx = NULL;
	}
};

// (c.haag 2007-07-05 15:20) - PLID 26595 - This is basically a mutex
// that simultaneously locks and unlocks multiple single mutexes. Doing
//		Thread 1: CMutex(a); CMutex(c);
//		Thread 2: CMutex(c); CMutex(a);
// is bad because you could wind up with a race condition where thread
// #1 has a locked and thread #2 has b locked. To solve this problem,
class CEMNLoaderMultiMutex
{
private:
	CEMNLoaderMutex** m_ppMutexes;
	CSyncObject** m_ppSyncObjects;
	CMultiLock* m_pMultiLock;
	DWORD m_nMutexes;
	// (c.haag 2007-07-11 12:01) - PLID 26623 - Variables used for
	// tracking the lock count and locking thread ID
	DWORD m_nOwnerThreadID;
	int m_nLockCount;
public:
	// (a.walling 2010-03-09 14:08) - PLID 37640 - Moved to cpp
	CEMNLoaderMultiMutex(CEMNLoaderMutex** ppMutexes, DWORD nMutexes);
	~CEMNLoaderMultiMutex();
	void Lock();
	void Unlock();
};

// (c.haag 2007-07-05 12:50) - PLID 26595 - This is basically like
// CHoldEMNLoaderMutex except it handles multi-locking. Doing
//		Thread 1: CMutex(a); CMutex(c);
//		Thread 2: CMutex(c); CMutex(a);
// is bad because you could wind up with a race condition where thread
// #1 has a locked and thread #2 has b locked. To solve this problem,
// we use CMultiLock objects
class CHoldEMNLoaderMultiMutex
{
private:
	CEMNLoaderMultiMutex* m_pmtx;
	BOOL bAutoDeleteMutex;
public:
	// (a.walling 2010-03-09 14:08) - PLID 37640 - Moved to cpp
	CHoldEMNLoaderMultiMutex(CEMNLoaderMutex** ppMutexes, DWORD nMutexes);
	CHoldEMNLoaderMultiMutex(CEMNLoaderMultiMutex* pmtx);
	~CHoldEMNLoaderMultiMutex();
	void Release();
};

// (c.haag 2010-06-29 11:58) - PLID 39404 - This object stores a unique "internal"
// (meaning only the CEMNLoader uses it and never a live EMR in memory) smart stamp 
// image-table pair, and a list of "manufactured" (meaning for use in a live EMR but
// potentially created from the same internal details more than once) details that
// originated from the internal detail pair.
// (z.manning 2011-01-21 10:06) - PLID 42338 - Need to support multiple images per smart stamp table
class CSmartStampImageTablePairs
{
public:
	CEMNDetailArray *m_paryInternalImages;
	CEMNDetail* m_pInternalTable;
	// These arrays can have differing lengths throughout their lifetimes. Please see
	// other PL comments for details on how they are populated.
	CArray<CEMNDetailArray*,CEMNDetailArray*> m_apManufacturedImages;
	CArray<CEMNDetail*,CEMNDetail*> m_apManufacturedTables;

private:
	// Ensures that m_apManufacturedImages contains pManufacturedImage, and
	// returns its index in the array.
	int GetManufacturedImageArrayIndex(CEMNDetailArray *paryManufacturedImages)
	{
		for (int i=0; i < m_apManufacturedImages.GetSize(); i++) {
			if (m_apManufacturedImages[i] == paryManufacturedImages) {
				return i;
			}
		}
		m_apManufacturedImages.Add(paryManufacturedImages);
		return m_apManufacturedImages.GetSize()-1;
	}
	// Ensures that m_apManufacturedTables contains pManufacturedTable, and
	// returns its index in the array.
	int GetManufacturedTableIndex(CEMNDetail* pManufacturedTable)
	{
		for (int i=0; i < m_apManufacturedTables.GetSize(); i++) {
			if (m_apManufacturedTables[i] == pManufacturedTable) {
				return i;
			}
		}
		m_apManufacturedTables.Add(pManufacturedTable);
		return m_apManufacturedTables.GetSize()-1;
	}

public:
	// The following two functions are called numerous times during the course of spawning.
	// This class tracks every detail that is passed into these functions to allow subsequent
	// calls to the opposing function to succeed. For example:
	//
	// 1. Detail A is a smart stamp table with no linked image
	//    Detail B is a smart stamp image with no linked table
	//
	// 2. GetMatchingImage(A) called. Now:
	//    Detail A is a smart stamp table with no linked image; but it's being tracked by this class
	//    Detail B is a smart stamp image with no linked table
	//
	// 3. GetMatchingTable(B) called. Now:
	//    Detail A is a smart stamp table with no linked image; but it's being tracked by this class
	//    Detail B is a smart stamp image linked with A, and B is also being tracked by this class
	//
	// 4. GetMatchingImage(A) called. Now:
	//    Detail A is a smart stamp table linked with B, and A is also being tracked by this class
	//    Detail B is a smart stamp image linked with A, and B is also being tracked by this class
	//
	// All subsequent calls have no effect. Everything is happy.

	// Given a manufactured table detail, this will return its corresponding image if it exists. 
	CEMNDetailArray* GetMatchingImageArray(CEMNDetail* pManufacturedTable)
	{
		int nIndex = GetManufacturedTableIndex(pManufacturedTable);
		if (nIndex < m_apManufacturedImages.GetSize()) {
			return m_apManufacturedImages[nIndex];
		}
		else {
			return NULL;
		}
	}
	// Given a manufactured image detail, this will return its corresponding table if it exists. 
	CEMNDetail* GetMatchingTable(CEMNDetailArray* paryManufacturedImages)
	{
		int nIndex = GetManufacturedImageArrayIndex(paryManufacturedImages);
		if (nIndex < m_apManufacturedTables.GetSize()) {
			return m_apManufacturedTables[nIndex];
		}
		else {
			return NULL;
		}
	}
};

// (c.haag 2008-07-18 16:12) - PLID 30784 - This map stores arrays of EMR problems. The
// map key is the Regarding ID, and the value is the array of EMR problems.
// (c.haag 2009-05-16 11:17) - PLID 34277 - Instead of maintaining an array of EMR problems,
// we now maintain an array of links to EMR problems
typedef CMap<long,long,CEmrProblemLinkAry*,CEmrProblemLinkAry*> CEmnLoaderProblemLinkMap;

class CEMNLoader 
{
//////////////////////////////////////////////////////////////////////
// Reference Counting variables
//////////////////////////////////////////////////////////////////////
private:
	// This object gets one reference per topic/subtopic plus one for the parent EMN.
	// The final release is done in CEMN::PostInitialLoad().
	long m_nRefCnt;

//////////////////////////////////////////////////////////////////////

// (a.walling 2013-03-20 16:09) - PLID 55790 - No more loading behavior -- just be fast no matter what!
// EmrInfo variables
//////////////////////////////////////////////////////////////////////
public:
	// (c.haag 2007-04-26 09:41) - PLID 25790 - This structure describes a EmrDataT
	// record. This is used in CEMRInfoItem objects, primarily in calls to
	// CEMNDetail::LoadContent
	struct EmrDataItem
	{
		long m_nID;
		long m_nEMRDataGroupID;
		CString m_strData;
		BOOL m_bIsLabel;
		long m_nListType;
		BYTE m_nListSubType; // (z.manning 2010-02-16 14:35) - PLID 37230
		BOOL m_bAutoAlphabetizeDropdown; // (c.haag 2010-02-24 14:16) - PLID 21301 - EmrDataT.AutoAlphabetizeDropdown
		BOOL m_bIsGrouped;
		CString m_strLongForm;
		// The following are calculated variables used only in CEMNDetail::LoadContent
		long m_nActionsType;
		// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
		// (z.manning, 05/23/2008) - PLID 30155 - Added Formula and DecimalPlaces
		CString m_strFormula;
		BYTE m_nDecimalPlaces;
		// (z.manning 2009-01-15 15:30) - PLID 32724 - Added InputMask
		CString m_strInputMask;
		// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
		EEmrTableAutoNumberType m_etantAutoNumberType;
		CString m_strAutoNumberPrefix;
		BOOL m_bHasDropdownElements; // (z.manning 2011-03-11) - PLID 42778
		BOOL m_bHasActiveDropdownElements; // (z.manning 2011-03-17 15:07) - PLID 42778
		GlassesOrderDataType m_GlassesOrderDataType; //TES 3/17/2011 - PLID 41108
		long m_nGlassesOrderDataID; //TES 3/17/2011 - PLID 41108
		EmrTableAutofillType m_eAutofillType; // (z.manning 2011-03-21 11:21) - PLID 30608
		// (j.jones 2011-04-28 14:39) - PLID 43122 - track if this list item is floated,
		// which will require that we bold the text
		//BOOL m_bIsFloated;
		long m_nFloatCount; // (a.walling 2013-03-18 09:32) - PLID 55723 - Load float count and sort order for EmrDataT records in the CEMNLoader
		long m_nSortOrder;
		long m_nFlags; // (z.manning 2011-05-26 14:47) - PLID 43865
		// (z.manning 2011-09-19 17:14) - PLID 41954 - Dropdown separators
		CString m_strDropdownSeparator;
		CString m_strDropdownSeparatorFinal;
		CString m_strSpawnedItemsSeparator; // (z.manning 2011-11-07 10:45) - PLID 46309
		// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType, which marks this data item as special to CWoundCareCalculator
		EWoundCareDataType m_ewccWoundCareDataType;
		// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
		long m_nParentLabelID;

		// (a.walling 2012-10-12 15:05) - PLID 53165 - Added default constructor
		EmrDataItem()
			: m_nID(-1)
			, m_nEMRDataGroupID(-1)
			, m_bIsLabel(FALSE)
			, m_nListType(0)
			, m_nListSubType(0)
			, m_bAutoAlphabetizeDropdown(FALSE)
			, m_bIsGrouped(FALSE)
			, m_nActionsType(0)
			, m_nDecimalPlaces(0)
			, m_etantAutoNumberType(etantInvalid)
			, m_bHasDropdownElements(FALSE)
			, m_bHasActiveDropdownElements(FALSE)
			, m_GlassesOrderDataType(godtInvalid)
			, m_nGlassesOrderDataID(-1)
			, m_eAutofillType(etatNone)
			, m_nFlags(0)
			, m_ewccWoundCareDataType(wcdtNone)
			, m_nParentLabelID(-1)
			, m_nSortOrder(-1)
			, m_nFloatCount(-1)
		{}
	};

	// (z.manning 2011-04-06 17:17) - PLID 43140 - Created a class for an array of data items
	class CEmrDataItemArray : public CArray<EmrDataItem,EmrDataItem&>
	{
	public:
		BOOL FindByDataGroupID(const long nDataGroupID, OUT EmrDataItem *pItem)
		{
			for(int nItemIndex = 0; nItemIndex < GetSize(); nItemIndex++)
			{
				EmrDataItem temp = GetAt(nItemIndex);
				if(temp.m_nEMRDataGroupID == nDataGroupID) {
					*pItem = temp;
					return TRUE;
				}
			}
			return FALSE;
		}
	};

	//DRT 1/18/2008 - PLID 28603 - This is used to house HotSpots for images
	struct EmrImageHotSpotItem
	{
		long m_nID;
		long m_nEMRHotSpotGroupID;
		CString m_strData;
		//TES 2/10/2010 - PLID 37298 - Added Anatomic Location information
		long m_nAnatomicLocationID;
		CString m_strAnatomicLocation;
		long m_nAnatomicQualifierID;
		CString m_strAnatomicQualifier;
		AnatomySide m_asSide;
		short m_n3DHotSpotID; // (z.manning 2011-07-25 12:55) - PLID 44649
	};

	// (c.haag 2007-06-06 15:18) - PLID 26240 - This structure contains information
	// for EmrItemLinkedDataT and related EmrDataT records
	struct EmrLinkedDataItem
	{
		long m_nID;				// EmrItemLinkedDataT.ID
		long m_nEmrLinkID;		// EmrItemLinkedDataT.EmrLinkID
		long m_nEmrDataID;		// EmrItemLinkedDataT.EmrDataID
		long m_nEmrDataGroupID;	// EmrDataT.EmrDataGroupID
		long m_nEmrInfoID;		// EmrDataT.EmrInfoID
	};

	// (c.haag 2007-04-23 17:47) - PLID 26463 - This class describes an EMR info item
	// in data. One object is created for every unique EmrInfoID when PreloadEmrTemplateDetails
	// is called. We will use these later in LoadEMRDetailStateDefault, and delete them
	// in the destructor.
	class CEMRInfoItem
	{
	public:
		long m_nID;
		EmrInfoType m_DataType;
		EmrInfoSubType m_DataSubType;
		//CArray<long,long> m_arDataDefaults; // dead code
		CString m_strBackgroundImageFilePath;
		eImageType m_BackgroundImageType;
		CString m_strDefaultText;
		BOOL m_bRememberForPatient;
		BOOL m_bRememberForEMR;	// (j.jones 2008-09-22 15:05) - PLID 31408 - added
		BOOL m_bTableRowsAsFields; // (c.haag 2008-10-16 11:29) - PLID 31709
		BOOL m_bAutoAlphabetizeListData; // (a.walling 2013-03-18 09:30) - PLID 55724 - Load AutoAlphabetizeListData for EMRInfoT records in the CEMNLoader
		_variant_t m_varRevision; // (a.walling 2013-03-27 10:05) - PLID 55900 - CEMNLoader needs to load the revision of EMRInfoT records
		long m_nInfoFlags; // (z.manning 2011-11-15 17:03) - PLID 38130

		// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
		long m_nChildEMRInfoMasterID;
		BOOL m_bSmartStampsEnabled;
		// (z.manning 2011-06-30 17:22) - PLID 44399 - These IDs have nothing to do with with an EMR info item
		// (they are specific to EMN/template details).
		//long m_nChildEMRDetailID;
		//long m_nChildEMRTemplateDetailID;

		// (z.manning 2011-01-25 15:24) - PLID 42336 - These are both deprecated as we now support multiple images
		// for one smart stamp table.
		//long m_nParentEMRDetailID;
		//long m_nParentEMRTemplateDetailID;

		//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
		BOOL m_bHasGlassesOrderData;
		GlassesOrderLens m_golLens;
		//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
		BOOL m_bHasContactLensData;
		// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding, which marks this table as being available to be used in CWoundCareCalculator
		BOOL m_bUseWithWoundCareCoding;

		// (c.haag 2007-04-26 10:43) - PLID 25790 - Content variables
		double m_dSliderMin;
		double m_dSliderMax;
		double m_dSliderInc;
		CString m_strLongForm;
		long m_nDataFormat;
		CString m_strDataSeparator;
		CString m_strDataSeparatorFinal;
		BOOL m_bDisableTableBorder;
		// (c.haag 2007-04-23 17:55) - PLID 25759 - This contains a copy of part of the
		// EmrInfoDefaults table in memory. It maps an EmrInfo item to an array of selections.
		// This is populated only once, and on demand. Refer to EnsureEmrInfoDefaultsMap for
		// usage.
		CArray<long,long>* m_paDefaultDataIDs;
		// (c.haag 2007-04-25 09:47) - PLID 25774 - When this item is added to a chart or
		// template, it can perform certain actions. Those actions are tracked here.
		MFCArray<EmrAction> m_arActions;
		// (c.haag 2007-04-26 09:34) - PLID 25790 - This is an array of all the EmrDataT
		// records connected to this item. This is only used in CEMNDetail::LoadContent 
		// at the time of this comment, and the values are read in the proper order (refer
		// to the old LoadContent queries).
		CEmrDataItemArray* m_paDataItems;
		//DRT 1/18/2008 - PLID 28603 - All hotspots which are tied to this info record.  These will
		//	only exist for images.
		CArray<EmrImageHotSpotItem*, EmrImageHotSpotItem*> *m_paryHotSpotItems;

		// (c.haag 2007-05-09 11:13) - PLID 25790 - TRUE if m_paDataItems has been populated
		BOOL m_bDataItemsPopulated;
		//DRT 1/18/2008 - PLID 28603 - TRUE if m_paryHotSpots has been populated
		BOOL m_bHotSpotItemsPopulated;

		// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
		// because they are now only calculated in the API, and not in Practice code
		/*
		// (j.jones 2007-08-27 10:35) - PLID 27056 - added the E/M coding data
		long m_nEMCodeCategoryID;
		// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_eEMCodeUseTableCategories
		EMCodeUseTableCategories m_eEMCodeUseTableCategories;
		long m_bUseEMCoding;
		EMCodingTypes m_emctEMCodingType;
		*/

		// (j.jones 2007-07-18 15:08) - PLID 26730 - tracks whether or not the info item has Info actions,
		// which it usually does not, such that we don't have to search for them later
		EMNDetailHasInfoActionsStatus m_eHasInfoActions;

		// (j.jones 2007-10-02 17:25) - PLID 26810 - added the EMRInfoT.Name
		CString m_strLabelText;

		// (a.walling 2008-06-30 12:33) - PLID 29271 - Preview Pane flags
		DWORD m_nPreviewFlags;

		CEmrItemStampExclusions *m_pStampExclusions; // (z.manning 2011-10-25 09:57) - PLID 39401

		// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

	public:
		CEMRInfoItem() 
			: m_varRevision(g_cvarNull) // (a.walling 2013-03-27 10:05) - PLID 55900 - CEMNLoader needs to load the revision of EMRInfoT records
		{
			m_paDefaultDataIDs = NULL;
			m_paDataItems = NULL;
			m_paryHotSpotItems = NULL;
			m_bDataItemsPopulated = FALSE;
			m_bHotSpotItemsPopulated = FALSE;
			m_eHasInfoActions = ehiasUndetermined;
			m_pStampExclusions = NULL;
			m_bAutoAlphabetizeListData = FALSE;
			// Force the caller to fill in the rest
		}
		~CEMRInfoItem() {
			if (NULL != m_paDefaultDataIDs) { delete m_paDefaultDataIDs; }
			if (NULL != m_paDataItems) { delete m_paDataItems; }
			if (NULL != m_paryHotSpotItems) {
				for(int nHotSpotIndex = 0; nHotSpotIndex < m_paryHotSpotItems->GetSize(); nHotSpotIndex++) {
					if(m_paryHotSpotItems->GetAt(nHotSpotIndex) != NULL) {
						delete m_paryHotSpotItems->GetAt(nHotSpotIndex);
					}
				}
				delete m_paryHotSpotItems; 
			}
			if(m_pStampExclusions != NULL) { delete m_pStampExclusions; } // (z.manning 2011-10-25 09:57) - PLID 39401
		}

		void SortDataItems(); // (a.walling 2013-03-18 09:32) - PLID 55725 - sort data items
	};

private:
	CMap<long,long,CEMRInfoItem*,CEMRInfoItem*> m_mapInfoItems;

//////////////////////////////////////////////////////////////////////
// Detail wrapper variables
//////////////////////////////////////////////////////////////////////
public:
	// (c.haag 2007-04-23 17:50) - PLID 26463 - This class contains a CEMNDetail object
	// that will likely be assigned to a topic in a parent EMN (depending on whether
	// it spawns or doesn't need to spawn). It also contains other fields pertinent to
	// LoadEMRDetailStateDefault that we can read in during the detail preload, and save
	// ourselves the hassle of querying later.
	class CPreloadedDetail
	{
	public:
		CEMNDetail* m_pDetail;
	public:
		// (c.haag 2007-04-23 17:50) - PLID 25758 - "Remember This Value" Emr Info data
		_variant_t m_vInfoRememberedDetailID;
		_variant_t m_vInfoRememberedDetailInfoID;
		// (c.haag 2007-06-26 15:06) - PLID 25759 - General Emr Info data
		_variant_t m_vInfoDefaultText;
		_variant_t m_vInfoSliderValue;
		// (c.haag 2007-04-24 12:45) - PLID 25768 - Non-template detail data
		_variant_t m_vDetailText;
		_variant_t m_vDetailImageTextData;
		_variant_t m_vDetailInkData;
		_variant_t m_vDetailPrintData; // (z.manning 2011-10-05 17:11) - PLID 45842
		_variant_t m_vDetailInkImagePathOverride;
		_variant_t m_vDetailInkImageTypeOverride;
		/* (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore the zoom and pan offsets.
		// (r.gonet 05/31/2011) - PLID 43896 - Zoom level of the image.
		_variant_t m_vDetailZoomLevel;
		// (r.gonet 05/31/2011) - PLID 43896 - Offset from left of bitmap.
		_variant_t m_vDetailOffsetX;
		// (r.gonet 05/31/2011) - PLID 43896 - Offset from top of bitmap.
		_variant_t m_vDetailOffsetY;
		*/
		// (c.haag 2007-04-26 09:43) - PLID 25790 - Template and non-template detail data
		_variant_t m_vDetailSliderValue;
		// (c.haag 2007-04-27 10:15) - PLID 25790 - Non-template detail content data
		// (j.jones 2008-07-21 09:20) - PLID 30779 - removed problem info here
		//_variant_t m_vProblemID;
		//_variant_t m_vProblemDesc;
		//_variant_t m_vProblemStatusID;
		//_variant_t m_vProblemDeleted;
		// (c.haag 2007-05-01 09:59) - PLID 25853 - Maintain the topic ID
		// (c.haag 2007-05-02 13:29) - PLID 25881 - When loading from templates,
		// this is the template topic ID
		// (a.walling 2008-06-30 13:55) - PLID 29271 - Preview Pane flags
		_variant_t m_vPreviewFlags;
		long m_nTopicID;
		// (c.haag 2007-06-15 10:15) - PLID 26344 - The source template ID
		long m_nTemplateID;

		// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
		// (z.manning 2011-01-25 10:06) - PLID 42336 - None of these were ever actually being used.
		//_variant_t m_vChildEMRInfoMasterID;
		//_variant_t m_vSmartStampsEnabled;
		//_variant_t m_vChildEMRDetailID;
		//_variant_t m_vChildEMRTemplateDetailID;
		//_variant_t m_vParentEMRDetailID;
		//_variant_t m_vParentEMRTemplateDetailID;

		// (z.manning 2010-02-23 16:14) - PLID 37412 - Keep track of the detail image stamps for this preloaded detail
		CEmrDetailImageStampArray m_arypDetailImageStamps;

	public:
		CPreloadedDetail() {
			// (c.haag 2007-05-03 13:02) - I am intentionally not initializing any
			// of the member variants to null variants. They should be empty if they haven't
			// been assigned a value. This makes debugging easier; but more importantly,
			// it is not the responsibility of this class to do any kind of data refinement
			// or assumptions on data type. It is merely a carrier of data.
			m_nTopicID = -1; // ...except for this value. It must never be anything other than an integer.
			// (c.haag 2007-06-26 10:27) - PLID 26463 - The detail should default to null!
			m_pDetail = NULL;
			// (c.haag 2007-06-26 10:28) - PLID 26344 - So should the template ID
			m_nTemplateID = -1;
		}

		~CPreloadedDetail() {
			m_arypDetailImageStamps.Clear();
		}
	};

//////////////////////////////////////////////////////////////////////
// Detail variables
//////////////////////////////////////////////////////////////////////
private:
	// The master detail array. This contains all chart/template details
	// regardless of whether they will spawn. Deleted details are, of course,
	// not included
	CArray<CPreloadedDetail*,CPreloadedDetail*> m_apAllDetails;
	// (c.haag 2007-07-02 09:12) - PLID 26515 - This is like m_apAllDetails
	// but only contains CEMNDetail objects.
	CArray<CEMNDetail*, CEMNDetail*> m_apAllCEMNDetails;
	// (c.haag 2007-08-08 12:22) - PLID 26833 - "Live" details are the details
	// the EMN thinks should exist in the initial load before anyway spawning 
	// takes places. A "Live" detail is a placeholder for the permanent
	// detail that an EMN will own by the time the asynchronous preload is over. A
	// "Live" detail all the data except state data loaded into it. It is deleted
	// when an EMN gets its permanent equivalent in CEMRTopic::PostLoad.
	CArray<CEMNDetail*, CEMNDetail*> m_apLiveDetails;
	CMap<long, long, CEMNDetail*, CEMNDetail*> m_mapLiveDetailsByID;
	CMap<long, long, CEMNDetail*, CEMNDetail*> m_mapLiveDetailsByTemplateID;

	// The master detail map for quick lookups. This has the same content as
	// m_apAllDetails
	CMap<long, long, CPreloadedDetail*, CPreloadedDetail*> m_mapAllDetailsByID;
	CMap<long, long, CPreloadedDetail*, CPreloadedDetail*> m_mapAllDetailsByTemplateID;

private:
	// Map used to determine which details are owned by an EMR (refer to
	// the usage of the function SetUsed())
	CMap<CEMNDetail*,CEMNDetail*,BOOL,BOOL> m_mapUsedDetails;

private:
	// (c.haag 2007-06-19 09:41) - PLID 26050 - Map used to determine which
	// details we loaded, or tried to load states for
	CMap<CEMNDetail*,CEMNDetail*,BOOL,BOOL> m_mapDetailsWithLoadedStates;

private:
	// Map used to determine what topic ID's that details belong to
	// (z.manning 2011-02-21 08:40) - PLID 42338 - Changed the array type to CEMNDetailArray
	CMap<long, long, CEMNDetailArray*, CEMNDetailArray*> m_mapDetailTopics;

private:
	// (c.haag 2007-08-15 11:27) - PLID 26833 - This is a map of details waiting to be
	// assigned to an EMN. When we are loading a "Live" detail in LoadEMRTopic, we don't 
	// actually do anything to it. We instead load the state information of its "Internal"
	// counterpart, and then store it in this array. Later when FlushPendingEMNDetailWrites
	// is called, it looks up that array and copies the "Internal" detail state to the "Live"
	// state.
	// (z.manning 2010-03-12 10:15) - PLID 37412 - Not really sure why this was ever a map as it
	// was never used in a way where one would be beneficial, but this is now an array.
	//CMap<CEMNDetail*,CEMNDetail*,BOOL,BOOL> m_mapPendingDetails;
	CArray<CEMNDetail*,CEMNDetail*> m_arypPendingDetails;
	void AddPendingDetail(CEMNDetail *pPendingDetail);

	// (c.haag 2007-08-15 11:27) - PLID 26833 - When FlushPendingEMNDetailWrites is called,
	// we put the Live detail in this map so that we know never to try to synchronize its
	// state with its Internal detail counterpart again.
	CMap<CEMNDetail*,CEMNDetail*,BOOL,BOOL> m_mapFinalizedDetails;

//////////////////////////////////////////////////////////////////////
// Topic wrapper variables
//////////////////////////////////////////////////////////////////////
public:
	// (c.haag 2007-04-30 12:48) - PLID 25853 - This class contains topic information
	// read in from data to later transfer to CEMRTopic objects into an EMN.
	class CPreloadedTopic
	{
	public:
		long m_nID;
		_variant_t m_vName;
		_variant_t m_vParentTopicID;
		_variant_t m_vTemplateTopicID;
		_variant_t m_vSourceActionID;
		// (z.manning 2009-03-05 14:15) - PLID 33338 - Added SourceDataGroupID and source action type
		_variant_t m_vSourceDataGroupID;
		_variant_t m_vSourceDetailImageStampID; // (z.manning 2010-02-25 11:00) - PLID 37532
		_variant_t m_vSourceStampID; //TES 3/1/7/2010 - PLID 37530
		_variant_t m_vSourceStampIndex; //TES 3/1/7/2010 - PLID 37530
		_variant_t m_vSourceActionType;
		//DRT 9/25/2007 - PLID 27515
		_variant_t m_vSourceActionSourceID;
		_variant_t m_vSourceActionSourceDataGroupID;
		// (z.manning, 01/23/2008) - PLID 28690
		_variant_t m_vSourceActionSourceHotSpotGroupID;
		// (z.manning 2009-02-13 09:14) - PLID 33070 
		_variant_t m_vSourceActionSourceTableDropdownGroupID;

		_variant_t m_vSourceDetailID;
		_variant_t m_vSourceTemplateTopicID;
		_variant_t m_vShowIfEmpty;
		_variant_t m_vTemplateID;
		_variant_t m_vTemplateOrderIndex;
		_variant_t m_vOrderIndex;
		_variant_t m_vSourceActionName;
		_variant_t m_vSourceActionDestType;
		_variant_t m_vSourceTemplateID;
		// (a.walling 2008-06-30 12:33) - PLID 29271 - Preview Pane flags
		_variant_t m_vPreviewFlags;
	public:
		// Array of sub-topic ID's
		CArray<long,long> m_anSubTopicIDs;
	public:
		// True if this topic has details in it -- this is a calculated value
		BOOL m_bHasDetails;
	public:
		// (c.haag 2007-05-08 16:27) - PLID 25941 - We now track what CEMRTopic
		// object corresponds to the preloaded topic object. m_pTopic is assigned
		// a value in CEMNLoader::LoadEMRTopic when this topic is loaded
		CEMRTopic* m_pTopic;
	public:
		CPreloadedTopic()
		{
			// (c.haag 2007-05-03 13:02) - I am intentionally not initializing any
			// of the member variants to null variants. They should be empty if they haven't
			// been assigned a value. This makes debugging easier; but more importantly,
			// it is not the responsibility of this class to do any kind of data refinement
			// or assumptions on data type. It is merely a carrier of data.
			m_nID = -1; // ...except for this value. It must never be anything other than an integer.
			m_bHasDetails = FALSE;
			m_pTopic = NULL;
		}
	};

	// (c.haag 2007-05-02 12:45) - PLID 25881 - This class contains template
	// topic information read in from data. This is used either for loading templates,
	// or creating new patient charts.
	class CPreloadedTemplateTopic
	{
	public:
		// Important fields
		long m_nID;
		long m_nTemplateID;
	public:
		// Non-calculated fields used in CEMRTopic::LoadTemplateTopicFields
		_variant_t m_vSourceActionID;
		// (z.manning 2009-03-05 14:15) - PLID 33338 - Added SourceDataGroupID and source action type
		_variant_t m_vSourceDataGroupID;
		_variant_t m_vSourceActionType;
			//DRT 9/25/2007 - PLID 27515
		_variant_t m_vSourceActionSourceID;
		_variant_t m_vSourceActionSourceDataGroupID;
		// (z.manning, 01/23/2008) - PLID 28690
		_variant_t m_vSourceActionSourceHotSpotGroupID;
		_variant_t m_vSourceActionSourceTableDropdownGroupID; // (z.manning 2009-02-13 09:15) - PLID 33070
		_variant_t m_vSourceDetailID;
		_variant_t m_vSourceTemplateTopicID;
		_variant_t m_vSourceStampID; //TES 3/17/2010 - PLID 37530
		_variant_t m_vSourceStampIndex; //TES 3/17/2010 - PLID 37530
		// Non-calculated fields used in CEMRTopic::LoadTemplateTopicFields and CEMNLoader::LoadEMRTopic
		_variant_t m_vShowIfEmpty;
		_variant_t m_vEMRParentTemplateTopicID;
		_variant_t m_vName;
		// Non-calculated fields used in CEMNLoader::LoadEMRTopic
		_variant_t m_vOrderIndex;
		_variant_t m_vHideOnEMN;
		// (j.jones 2007-07-30 11:25) - PLID 26874 - we now include the template's "AddOnce" field
		_variant_t m_vTemplateAddOnce;
		// (c.haag 2007-07-23 17:31) - PLID 26344 - Some topics exist on templates because they
		// were spawned from single or multi-select items elsewhere on the template. In some
		// circumstances, we need to know what the template ID and topic names are of its original
		// counterpart.
		_variant_t m_vOriginalTemplateID;
		_variant_t m_vOriginalTopicName;
		_variant_t m_vOriginalShowIfEmpty;
		// (a.walling 2008-06-30 12:33) - PLID 29271 - Preview Pane flags
		_variant_t m_vPreviewFlags;
	public:
		// Array of sub-topic ID's
		CArray<long,long> m_anSubTemplateTopicIDs;
	public:
		// True if this topic has details in it -- this is a calculated value
		BOOL m_bHasDetails;
	public:
		// (c.haag 2007-05-08 16:28) - PLID 25941 - We now track what CEMRTopic
		// object corresponds to the preloaded template topic object. m_pTopic 
		// is assigned a value in CEMNLoader::LoadEMRTopic when this topic is loaded
		CEMRTopic* m_pTopic;
	public:
		CPreloadedTemplateTopic()
		{
			// (c.haag 2007-05-03 13:06) - I am intentionally not initializing any
			// of the member variants to null variants. They should be empty if they haven't
			// been assigned a value. This makes debugging easier; but more importantly,
			// it is not the responsibility of this class to do any kind of data refinement
			// or assumptions on data type. It is merely a carrier of data.
			m_nID = -1; // ...except for this value. It must never be anything other than an integer.
			m_bHasDetails = FALSE;
			m_pTopic = NULL;
		}
	};

	// (c.haag 2007-05-03 10:24) - PLID 25881 - CEMNLoader::LoadEMRTopic requires
	// EMR action related data for template topics. This is an element in a map used
	// to reference that information.
	struct PreloadedTopicAction
	{
	public:
		_variant_t m_vEmrActionName;
		_variant_t m_vEmrActionDestType;
		// long m_nEmrActionID; // No need for this; the key is the same thing
	};
	// The key in this map is the EMR action ID, and the element is action data
	CMap<long,long,PreloadedTopicAction,PreloadedTopicAction&> m_mapEmrTemplateTopicActions;

private:
	// (c.haag 2007-07-25 18:23) - PLID 25881 - This map tracks which template topics being
	// loaded have template details. This is used only at the beginning of the initial load.
	CMap<long,long,BOOL,BOOL> m_mapTemplateTopicsWithDetails;

public:
	// (z.manning 2011-04-06 16:59) - PLID 43140 - Simple struct to track a data group ID and an order index.
	struct EmrDataGroupOrder
	{
		long nDataGroupID;
		long nOrderIndex;
		// (j.jones 2011-04-29 10:51) - PLID 43122 - added IsFloated
		BOOL bIsFloated;
	};

	// (z.manning 2011-04-06 16:59) - PLID 43140 - Overloaded class for an array of data group order objects.
	class CEmrDataGroupOrderArray : public CArray<EmrDataGroupOrder,EmrDataGroupOrder&>
	{
	public:
		void operator=(CEmrDataGroupOrderArray &arySource)
		{
			this->RemoveAll();
			this->Append(arySource);
		}
	};

private:
	// (z.manning 2011-04-06 17:00) - PLID 43140 - Map to store the data group sort order array by detail ID.
	CMap<long,long, CEmrDataGroupOrderArray*,CEmrDataGroupOrderArray*> m_mapDetailIDToOrder;

public:
	// (z.manning 2011-04-06 16:58) - PLID 43140 - This function will return the data group sort order
	// array for a given detail ID.
	CEmrDataGroupOrderArray* GetOrderArrayByDetailID(const long nDetailID);

//////////////////////////////////////////////////////////////////////
// Detail variables
//////////////////////////////////////////////////////////////////////
private:
	// (c.haag 2007-04-30 12:53) - PLID 25853 - The master topic array. 
	// This contains all patient chart topics. Deleted topics are, of course, 
	// not included.
	CArray<CPreloadedTopic*,CPreloadedTopic*> m_apAllTopics;
	// The master topic map for quick lookups. This has the same content as m_apAllTopics
	CMap<long, long, CPreloadedTopic*, CPreloadedTopic*> m_mapAllTopicsByID;

	// (c.haag 2007-05-02 12:58) - PLID 25881 - The master template topic
	// array. This contains all template topics regardless of whether they
	// will spawn
	CArray<CPreloadedTemplateTopic*,CPreloadedTemplateTopic*> m_apAllTemplateTopics;
	// The master template topic map for quick lookups. This has the same content
	// as m_apAllTemplateTopics
	CMap<long, long, CPreloadedTemplateTopic*, CPreloadedTemplateTopic*> m_mapAllTemplateTopicsByID;

//////////////////////////////////////////////////////////////////////
// Utility variables
//////////////////////////////////////////////////////////////////////
private:
	// (c.haag 2007-05-09 10:00) - PLID 25790 - This recordset is created in EnsureEmrDataTMap. There
	// are so many records in the recordset, that we should only populate memory when necessary
	//ADODB::_RecordsetPtr m_prsEmrDataTMap; // (a.walling 2013-03-11 15:10) - PLID 55572 - Only used locally in EnsureEmrMapGeneric; remove member variable

	// (c.haag 2007-08-28 12:38) - PLID 24949 - Mutex for protecting m_nRefCnt
	CMutex m_mtxRefCnt;

	//DRT 7/26/2007 - PLID 26835 - Mutex for protecting the m_bIgnoreLoadingActions variable
	CMutex m_mtxIgnoreActions;

	// (c.haag 2007-07-03 09:22) - PLID 26523 - This mutex is used to let outside callers safely access
	// topic data from any thread. This applies to both patient chart and template topics.
	CEMNLoaderMutex m_mtxTopics;

	// (c.haag 2007-07-03 10:18) - PLID 26523 - This mutex is used to let outside callers safely access
	// EMR Info data from any thread.
	CEMNLoaderMutex m_mtxEMRInfo;

	// (c.haag 2007-07-03 10:56) - PLID 26523 - This mutex is used to let outside callers safely access
	// EMR action data from any thread.
	CEMNLoaderMutex m_mtxEMRDataActions;

	// (z.manning, 02/21/2008) - PLID 28690 - Added mutex for image hot spot actions.
	CEMNLoaderMutex m_mtxImageHotSpotActions;

	// (z.manning 2009-02-16 10:42) - PLID 33070 - Added a mutex for table dropdown item actions;
	CEMNLoaderMutex m_mtxTableDropdownItemActions;

	// (c.haag 2007-07-03 11:04) - PLID 26523 - This mutex is used to let outside callers safely access
	// EMR linked data items
	CEMNLoaderMutex m_mtxLinkedDataItems;

	// (c.haag 2007-07-05 09:09) - PLID 26595 - This mutex is used to let outside callers safely access
	// m_apAllDetails
	CEMNLoaderMutex m_mtxInternalDetails;

	// (c.haag 2008-07-18 16:18) - PLID 30784 - This mutex is used to let outside callers safely access
	// EMR problems
	CEMNLoaderMutex m_mtxEMRProblems;

	// (c.haag 2007-07-05 14:52) - PLID 26595 - This is a multi-lock mutex which controls
	// exclusive access for internal detail and EMR info-related data
	CEMNLoaderMultiMutex* m_pmtxInternalDetailsAndEMRInfo;

	//DRT 7/26/2007 - PLID 26835 - Setting this flag allows you to turn off spawning.  When it is true, 
	//	EnsureEmrActionData() will do nothing.  This is currently used when you load an existing EMR from
	//	data, and there is no point in spawning actions.
	BOOL m_bIgnoreLoadingActions;

	// (c.haag 2007-08-10 17:50) - PLID 27049 - Set to TRUE when the EMN will look to us
	// to get EMN details. Set to FALSE when the EMN must traverse its own topics to get
	// details like it normally does after the initial load.
	BOOL m_bManagingEMNDetailArray;

public:
	// (c.haag 2007-07-03 09:53) - PLID 26523 - Exposes the topics mutex to outside callers
	inline CEMNLoaderMutex* GetTopicsMutex() { return &m_mtxTopics; }
	// (c.haag 2007-07-03 10:24) - PLID 26523 - Exposes the EMR info item mutex to outside callers
	inline CEMNLoaderMutex* GetEMRInfoMutex() { return &m_mtxEMRInfo; }
	// (c.haag 2007-07-03 11:02) - PLID 26523 - Exposes the EMR data actions mutex to outside callers
	inline CEMNLoaderMutex* GetEMRDataActionsMutex() { return &m_mtxEMRDataActions; }
	// (z.manning, 02/21/2008) - PLID 28690 - Exposes the EMR hot spot actions mutex to outside callers
	inline CEMNLoaderMutex* GetImageHotSpotActionsMutex() { return &m_mtxImageHotSpotActions; }
	// (z.manning 2009-02-16 10:43) - PLID 33070
	inline CEMNLoaderMutex* GetTableDropdownItemActionsMutex() { return &m_mtxTableDropdownItemActions; }
	// (c.haag 2007-07-03 11:12) - PLID 26523 - Exposes the EMR linked data items mutex to outside callers
	inline CEMNLoaderMutex* GetEMRLinkedDataItemsMutex() { return &m_mtxLinkedDataItems; }
	// (c.haag 2007-07-05 09:10) - PLID 26595 - Exposes the internal EMN detail mutex to outside callers
	inline CEMNLoaderMutex* GetInternalDetailsMutex() { return &m_mtxInternalDetails; }
	// (c.haag 2007-07-05 14:46) - PLID 26595 - Exposes the mutex for managing both internal details and info
	// items simultaneously
	inline CEMNLoaderMultiMutex* GetInternalDetailsAndEMRInfoMultiMutex() { return m_pmtxInternalDetailsAndEMRInfo; }
	// (c.haag 2008-07-18 16:19) - PLID 30784 - Exposes the mutex for managing EMR problems
	inline CEMNLoaderMutex* GetEMRProblemsMutex() { return &m_mtxEMRProblems; }

	//DRT 7/26/2007 - PLID 26835 - Setting this flag allows you to turn off spawning.  When it is true, 
	//	EnsureEmrActionData() will do nothing.  This is currently used when you load an existing EMR from
	//	data, and there is no point in spawning actions.
	void SetIgnoreLoadingActions(BOOL bIgnoreLoadingActions);
	BOOL GetIgnoreLoadingActions();


private:
	//TES 6/6/2008 - PLID 29416 - We no longer handle these items specially.
	// (c.haag 2007-04-20 09:04) - PLID 25730 - The active Current Medication and Allergies info ID's
	/*long m_nActiveCurrentMedicationsInfoID;
	long m_nActiveAllergiesInfoID;*/

private:
	// (c.haag 2007-04-30 14:02) - PLID 25853 - The EMN using this loader object
	CEMN* m_pEMN;

private:
	// (c.haag 2007-04-24 12:15) - PLID 25759 - TRUE if the defaults section of EMR info items has been
	// populated
	BOOL m_bEmrInfoDefaultsPopulated;

	// (c.haag 2007-04-27 09:30) - PLID 26446 - This contains a copy of part of the
	// EmrSelectT table in memory. It maps EMN details to an array of selections. This is
	// populated only once, and on demand. Refer to EnsureSelectMap for usage.
	CMap<long,long,CArray<long,long>*,CArray<long,long>*> m_mapSelect;
	BOOL m_bSelectMapPopulated;

	//DRT 1/23/2008 - PLID 28698
	CMap<long,long,CArray<long,long>*,CArray<long,long>*> m_mapHotSpots;
	BOOL m_bHotSpotMapPopulated;

	// (c.haag 2007-04-23 17:57) - PLID 26459 - This contains a copy of part of the
	// EmrTemplateSelectT table in memory. It maps EMN template details to an array of
	// selections. This is populated only once, and on demand. Refer to EnsureTemplateSelectMap
	// for usage.
	CMap<long,long,CArray<long,long>*,CArray<long,long>*> m_mapTemplateSelect;
	BOOL m_bTemplateSelectMapPopulated;

	//DRT 1/23/2008 - PLID 28698
	CMap<long,long,CArray<long,long>*,CArray<long,long>*> m_mapTemplateHotSpots;
	BOOL m_bTemplateHotSpotMapPopulated;


	// (c.haag 2007-04-24 10:02) - PLID 25761 - This object maps EMN template details
	// to default table state strings. This is populated once, and on demand. Refer to
	// EnsureTemplateTableMap for usage.
	// (a.walling 2013-07-02 09:02) - PLID 57407 - CMap's ARG_VALUE should be const CString& instead of LPCTSTR so it can use CString reference counting
	CMap<long,long,CString,const CString&> m_mapTemplateTable;
	BOOL m_bTemplateTableMapPopulated;

	// (c.haag 2007-06-04 17:52) - PLID 26444 - This object maps EMN details to table
	// state strings. This is populated once, and on demand. Refer to EnsureTableMap for
	// usage.
	// (a.walling 2013-07-02 09:02) - PLID 57407 - CMap's ARG_VALUE should be const CString& instead of LPCTSTR so it can use CString reference counting
	CMap<long,long,CString,const CString&> m_mapTable;
	BOOL m_bTableMapPopulated;

	// (z.manning 2011-10-25 10:12) - PLID 39401 - Map to hold per-item stamp exclusions
	CMap<long,long,CEmrItemStampExclusions*,CEmrItemStampExclusions*> m_mapInfoIDToStampExclusions;

	// (c.haag 2007-04-25 13:56) - PLID 25758 - This map tracks a candidate list of details that are
	// candidates for being remembered when calculating the state of a detail in this chart. We used
	// to have a query do this, but it's just too slow for our current official content because we
	// have so many "remembering" details.
	// (j.jones 2008-09-22 16:14) - PLID 31408 - updated to handle RememberForEMR, we now include
	// the EMRGroupID in our class
	class CRememberCandidate {
	public:
		long m_nDetailID;
		COleDateTime m_dtEMRMaster;
		long m_nEMRMasterID;
		long m_nEMRGroupID;
		long m_nEMRInfoID;
	};
	CMapStringToPtr m_mapRememberForPatientCandidates;
	// (j.jones 2008-09-22 16:15) - PLID 31408 - added m_mapRememberForEMRCandidates
	CMapStringToPtr m_mapRememberForEMRCandidates;
	BOOL m_bRememberCandidateMapsPopulated;

	// (c.haag 2007-04-24 13:36) - PLID 25758 - This map tracks remembered detail information
	// during a template load so that we don't have to query for it when calling LoadEmrDetailState
	CMap<long,long,CPreloadedDetail*,CPreloadedDetail*> m_mapRememberedDetails;
	BOOL m_bRememberedDetailMapPopulated;

	// (c.haag 2007-04-25 09:48) - PLID 25774 - EMR list items preloaded into this object can have
	// actions per list item. This map tracks those actions.
	//
	// Unlike other preloaded data, this map is per-topic. The reason for this is we need to know
	// the state of related details when the data is queried
	//
	CMap<long,long,MFCArray<EmrAction>*,MFCArray<EmrAction>*> m_mapActions;

	// (z.manning, 02/21/2008) - PLID 28690 - Added a map to link hot spot IDs to the actions for that hot spot.
	CMap<long,long, MFCArray<EmrAction>*,MFCArray<EmrAction>*> m_mapImageHotSpotActions;

	// (z.manning 2009-02-16 10:47) - PLID 33070 - Added a map to link dropdown IDs to its actions.
	CMap<long,long, MFCArray<EmrAction>*,MFCArray<EmrAction>*> m_mapTableDropdownItemActions;

	// (c.haag 2007-04-26 09:52) - PLID 25790 - Set to true with m_aDataItems member of each CEMRInfoItem
	// element have been loaded
	BOOL m_bEMRDataTItemsLoaded;

	// (c.haag 2007-05-21 09:44) - PLID 26050 - TRUE if all the detail states have been loaded for the
	// entire EMN/template
	BOOL m_bAllStatesLoaded;

	// (c.haag 2007-06-06 15:27) - PLID 26240 - This is the array of linked data items. This is used to
	// minimize the number of queries run from CEMR::ApplyEmrLinksToBatch
	CArray<EmrLinkedDataItem,EmrLinkedDataItem&> m_aEmrLinkedDataItems;
	// (c.haag 2007-06-06 15:23) - PLID 26240 - TRUE if m_aEmrLinkedDataItems has been loaded for the
	// entire EMN/template
	BOOL m_bAllEmrItemLinkedDataItemsLoaded;

	// (c.haag 2008-07-18 15:44) - PLID 30784 - Preloaded problems are stored in these maps. Map key
	// is the regarding ID, map value is an array of problems which correspond to it.
	CEmnLoaderProblemLinkMap m_mapEmrItemProblems;
	CEmnLoaderProblemLinkMap m_mapEmrDataItemProblems;
	CEmnLoaderProblemLinkMap m_mapEmrTopicProblems;
	CEmnLoaderProblemLinkMap m_mapEMNProblems;
	CEmnLoaderProblemLinkMap m_mapEMRProblems;
	CEmnLoaderProblemLinkMap m_mapEmrDiagProblems;
	CEmnLoaderProblemLinkMap m_mapEmrChargeProblems;
	CEmnLoaderProblemLinkMap m_mapEmrMedicationProblems;

private:
	// (c.haag 2007-06-25 16:43) - PLID 26050 - Map of EmrInfoID's we've already queried from inside
	// EnsureEmrActionData
	CMap<long,long,BOOL,BOOL> m_mapEnsureEmrActionData_QueriedInfoIDs;

	// (c.haag 2007-06-25 16:43) - PLID 26050 - Map of states we've already queried from inside
	// EnsureEmrActionData
	CMap<CString,LPCTSTR,BOOL,BOOL> m_mapEnsureEmrActionData_QueriedStates;

	// (z.manning, 01/22/2008) - PLID 28690 - Map of hot spot IDs we've already queried from inside
	CMap<long,long,BOOL,BOOL> m_mapEnsureEmrActionData_QueriedHotSpotIDs;

	// (z.manning 2009-02-16 11:26) - PLID - Map of already queried table dropdown item IDs
	CMap<long,long,BOOL,BOOL> m_mapEnsureEmrActionData_QueriedTableDropdownItemIDs;

private:
	// (c.haag 2007-05-08 12:57) - PLID 25790 - Supplimental variables for further
	// optimized loading
	BOOL m_bLoadFromTemplate;	// True if we are loading from a template
	BOOL m_bIsTemplate;			// True if the EMR object operated on is a template
	long m_nMasterID;			// If loading a chart, this is the EmrMasterT ID
								// If loading a template, this is the EmrTemplateT ID

private:
	// (c.haag 2010-06-29 12:01) - PLID 39404 - The array of unique smart stamp image-table pairs. This is populated
	// only in PreloadEmrTemplateDetails. This array is used during LoadEMRTopic for the purpose of linking newly created
	// manufactured smart stamp images and tables together.
	CArray<CSmartStampImageTablePairs*,CSmartStampImageTablePairs*> m_apSmartStampTemplateItemPairs;

private:
	// (c.haag 2011-03-17) - PLID 42895 - Collection of all common lists that may be pertinent to the loaded content.
	// Key is EmrInfoID, value is the collection of lists.
	CMap<long,long,CEmrInfoCommonListCollection*,CEmrInfoCommonListCollection*> m_mapCommonLists;

public:
	// (c.haag 2007-05-01 12:29) - PLID 25853 - We now need an EMN object to work with topic preloading
	// (c.haag 2007-05-08 12:56) - PLID 25790 - We now require the template/chart information in advance
	// for further optimizations
	// (c.haag 2007-05-30 13:27) - PLID 26050 - We now let the caller configure our loading behavior
	// (j.jones 2011-04-28 14:39) - PLID 43122 - added nProviderIDForFloatingData as a required parameter (send -1 if you don't have a provider)
	CEMNLoader(CEMN* pEMN, ADODB::_ConnectionPtr& pCon, BOOL bLoadFromTemplate, BOOL bIsTemplate, long nMasterID, long nProviderIDForFloatingData);
	virtual ~CEMNLoader();

//////////////////////////////////////////////////////////////////////
// Reference Counting
//////////////////////////////////////////////////////////////////////
public:
	void AddRef();
	void Release();
	long GetRefCnt();

//////////////////////////////////////////////////////////////////////
// Preloading functions called from the main thread
//////////////////////////////////////////////////////////////////////
	// (c.haag 2007-04-24 08:25) - PLID 26463 - Loads all of the non-deleted details for a patient EMN,
	// regardless of their source action ID / spawning behavior
	void PreloadEmrDetails(ADODB::_RecordsetPtr& rsDetails);

	// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

	// (z.manning 2011-04-06 16:12) - PLID 43140
	void PreloadEmrDetailListOrders(ADODB::_RecordsetPtr &rsDetailListOrders);

	// (c.haag 2007-04-24 08:25) - PLID 26464 - Loads all of the non-deleted details for a template,
	// regardless of their source action ID / spawning behavior
	void PreloadEmrTemplateDetails(long nPatientID, ADODB::_RecordsetPtr& rsTemplateDetails, CEMR* pParentEMR);

public:
	// (c.haag 2007-04-30 12:27) - PLID 25853 - Loads all of the non-deleted topics for a patient EMN
	void PreloadEmrTopics(ADODB::_RecordsetPtr& rsTopics);

	// (c.haag 2007-05-02 12:50) - PLID 25881 - Loads all topics for a template
	// (c.haag 2007-05-31 12:52) - PLID 26175 - Added an optional flag for the loading of root topics
	//DRT 9/25/2007 - PLID 27515 - Added extra SourceAction parameters
	// (z.manning 2009-02-13 09:16) - PLID 33070 - Added nSourceActionSourceTableDropdownGroupID
	// (z.manning 2009-03-04 14:37) - PLID 33338 - Use the source action info class
	void PreloadEmrTemplateTopics(ADODB::_RecordsetPtr& rsTemplateTopics, long nEmrTemplateID,
		BOOL bIsNewTopic, HWND hWnd, SourceActionInfo &sai, BOOL bLoadHiddenTopics = FALSE, OPTIONAL IN long nPatientID = -1, OPTIONAL IN CEMR *pEmr = NULL,
		BOOL bLoadRootTopics = TRUE, OPTIONAL IN long nSourceActionSourceID = -1, OPTIONAL IN long nSourceActionSourceDataGroupID = -1, OPTIONAL IN long nSourceActionSourceHotSpotGroupID = -1, OPTIONAL IN long nSourceActionSourceTableDropdownGroupID = -1);

	// (c.haag 2011-03-17) - PLID 42895 - Loads EMR Info Common Lists
	void PreloadEmrInfoCommonLists(ADODB::_RecordsetPtr rsCommonLists);
	// (c.haag 2011-03-17) - PLID 42895 - Loads EMR Info Common List Items
	void PreloadEmrInfoCommonListItems(ADODB::_RecordsetPtr rsCommonListItems);

public:
	// (c.haag 2008-07-18 15:39) - PLID 30784 - Loads all problems in the span of the EMN
	// (including EMR-level problems)
	// (c.haag 2009-05-19 10:11) - PLID 34277 - We now require an EMR object
	void PreloadEmrProblems(ADODB::_RecordsetPtr& rsProblems, CEMR* pParentEmr);

	// (z.manning 2010-02-17 15:35) - PLID 37412 - Loads all detail image stamp data for the current EMN
	void PreloadDetailImageStamps(ADODB::_RecordsetPtr& rsDetailImageStamps);

public:
	// (c.haag 2011-03-18) - PLID 42895 - Returns a collection of EMR Info common lists
	CEmrInfoCommonListCollection* GetEmrInfoCommonLists(long nEmrInfoID);

public:
	// (c.haag 2007-08-10 17:49) - PLID 27049 - Functions for m_bManagingEMNDetailArray
	void SetManagingEMNDetailArray(BOOL bManage);
	BOOL IsManagingEMNDetailArray() const;

//////////////////////////////////////////////////////////////////////
// Master topic level load function called from a worker thread
//////////////////////////////////////////////////////////////////////
	// (c.haag 2007-04-24 08:26) - PLID 25881 - Loads the content of a single topic
	void LoadEMRTopic(EMRTopicLoadInfo *pLoadInfo, ADODB::_Connection *lpCon, CEMR *pParentEmr);

private:
	// (c.haag 2007-07-27 09:45) - PLID 26833 - This function prepares a live detail for
	// assignment to an EMN
	void LoadEMRTopicDetail(CEMNDetail* pLiveDetail, ADODB::_Connection *lpCon);

//////////////////////////////////////////////////////////////////////
// Topic functions
//////////////////////////////////////////////////////////////////////
public:
	// (c.haag 2007-05-01 08:41) - PLID 25853 - Returns a preloaded non-template topic given a topic ID
	CPreloadedTopic* GetPreloadedTopicByID(long nID);

	// (c.haag 2007-05-02 15:44) - PLID 25881 - Returns a preloaded template topic given a topic ID
	CPreloadedTemplateTopic* GetPreloadedTemplateTopicByID(long nID);

//////////////////////////////////////////////////////////////////////
// Detail functions
//////////////////////////////////////////////////////////////////////
private:
	// (c.haag 2007-04-24 10:47) - PLID 26463 - Returns the total count of all details
	// that we preloaded; even details that may not spawn
	// (c.haag 2007-07-05 09:33) - PLID 25239 - This is now for internal use only
	// (c.haag 2007-07-10 10:46) - PLID 25239 - This has been depreciated
	//long GetTotalDetailCount();

	// (c.haag 2007-04-24 09:37) - PLID 26463 - Returns a detail given an index
	// (c.haag 2007-07-05 09:33) - PLID 25239 - This is now for internal use only
	// (c.haag 2007-07-10 10:49) - PLID 25239 - This has been depreciated
	//CEMNDetail* GetDetailByIndex(long nIndex);

	// (c.haag 2007-04-24 09:37) - PLID 26463 - Returns a detail given an ID
	// (c.haag 2007-07-05 09:33) - PLID 25239 - This is now for internal use only
	CEMNDetail* GetDetailByID(long nID);

public:
	// (c.haag 2007-04-24 09:37) - PLID 26464 - Returns a detail given a template ID
	// (c.haag 2007-07-05 09:33) - PLID 25239 - This is now for internal use only
	CEMNDetail* GetDetailByTemplateDetailID(long nID);

	// (z.manning 2011-01-25 15:58) - PLID 42336
	// (z.manning 2012-07-03 17:49) - PLID 51025 - This function takes a detail pointer instead of just and ID now
	void GetParentDetailsByDetailID(CEMNDetail *pChildDetail, OUT CEMNDetailArray *paryParentDetails);
	void GetParentDetailsByTemplateDetailID(const long nChildTemplateDetailID, OUT CEMNDetailArray *paryParentDetails);

	// (c.haag 2007-07-05 09:15) - PLID 25239 - Returns the number of details we have to
	// give to the CEMN object we're loading for
	// (c.haag 2007-07-10 13:09) - This is the sum of all the "Live" details, which we
	// will "give" to the EMN later on, and "Loaded" details which we already gave to the
	// EMN.
	long GetInitialLoadDetailCount() const;

	// (c.haag 2007-07-27 10:19) - PLID 26833 - Returns a detail owned by the CEMN object
	// we're loading for by its index.
	// (c.haag 2007-07-27 09:57) - The caller should not assume that this detail has
	// a state or is fully loaded yet
	CEMNDetail* GetInitialLoadDetailByIndex(long nIndex) const;

	// (c.haag 2007-07-27 10:20) - PLID 26833 - Returns a detail owned by the CEMN object
	// we're loading for by its ID.
	// (c.haag 2007-07-27 09:57) - The caller should not assume that this detail has
	// a state or is fully loaded yet
	CEMNDetail* GetInitialLoadDetailByID(long nID) const;

	// (c.haag 2007-07-27 10:21) - PLID 26833 - Returns a detail owned by the CEMN object
	// we're loading for by its template ID.
	// (c.haag 2007-07-27 09:57) - The caller should not assume that this detail has
	// a state or is fully loaded yet
	CEMNDetail* GetInitialLoadDetailByTemplateID(long nID) const;
	// (z.manning 2010-07-21 09:30) - PLID 39591
	CEMNDetail* GetIntialLoadSmartStampImageByTemplateID(long nID) const;
	// (z.manning 2011-02-18 14:29) - PLID 42336
	CEMNDetail* GetSmartStampTableDetailByInfoMasterID(const long nTableInfoMasterID, const long nImageInfoMasterID) const;

	// (z.manning 2011-01-25 17:43) - PLID 42336
	void GetInitialLoadParentDetailsByDetailID(const long nChildDetailID, OUT CEMNDetailArray *paryParentDetails);
	void GetInitialLoadParentDetailsByTemplateDetailID(const long nChildTemplateDetailID, OUT CEMNDetailArray *paryParentDetails);

public:
	// (c.haag 2007-04-27 10:22) - PLID 25790 - Returns a preloaded detail given an ID.
	// This gives the caller complete access to a detail and special preloaded fields
	// that are related to content and the info item
	CPreloadedDetail* GetPreloadedDetailByID(long nID);

	// (c.haag 2007-07-05 09:56) - PLID 25239 - Returns a preloaded detail given a template ID
	// This gives the caller complete access to a detail and special preloaded fields
	// that are related to content and the info item
	// (c.haag 2007-07-10 10:52) - PLID 25239 - This has been depreciated
	//CPreloadedDetail* GetPreloadedDetailByTemplateID(long nID);

public:
	// Given a topic ID, return the list of details which belong to it
	// (z.manning 2011-02-21 08:40) - PLID 42338 - Changed the return type to CEMNDetailArray
	CEMNDetailArray* GetDetailArrayByTopicID(long nTopicID);

public:
	// (c.haag 2007-05-30 16:36) - PLID 26187 - Returns the template topic count
	long GetTotalTemplateTopicCount();

	// (c.haag 2007-05-30 16:37) - PLID 26187 - Returns a preloaded template
	// topic by index
	CPreloadedTemplateTopic* GetPreloadedTemplateTopicByIndex(long);

private:
	// When a detail is assigned to a topic, this must be called so that
	// we know not to delete it
	// (c.haag 2007-07-03 09:20) - PLID 26523 - This should not be called from outside this class
	void SetUsed(CEMNDetail* pDetail);

private: // (c.haag 2007-07-03 09:20) - PLID 26523 - These should not be called from outside this class
	// (c.haag 2007-06-19 09:40) - PLID 26050 - Flags a detail as having a state that
	// was loaded by the preloader
	// (c.haag 2007-07-05 11:48) - PLID 25239 - We now need the load info object
	void SetStateLoaded(CEMNDetail* pDetail);

	// (c.haag 2007-06-19 09:39) - PLID 26050 - Returns TRUE if we already loaded
	// (or tried to load) the state of a detail. The only time that this returns
	// TRUE and pDetail->m_varState.vt == VT_EMPTY is for Current Medications or
	// Allergies details that will be properly loaded in the post topic load.
	//TES 6/6/2008 - PLID 29416 - Correction - We no longer handle Current Medications and Allergies differently.
	BOOL WasStateLoaded(CEMNDetail* pDetail);

//////////////////////////////////////////////////////////////////////
// Detail State functions
//////////////////////////////////////////////////////////////////////
private:

	// (c.haag 2007-02-28 12:28) - PLID 24989 - This function determines the state of pDetail,
	// loads it into pDetail, and then repeats for all details whose state affects the appearance
	// of pDetail
	// (c.haag 2007-07-02 09:24) - PLID 26515 - We no longer need the array of all EMN details input here
	// now that we maintain them in a member list
	// (c.haag 2007-07-05 11:48) - PLID 25239 - We now need the load info object
	void LoadEMRDetailStateCascaded(CEMNDetail* pDetail, ADODB::_Connection *lpCon, EMRTopicLoadInfo* pLoadInfo);

	// (c.haag 2007-04-27 09:41) - PLID 25768 - This is an overload of the global LoadEMRDetailState
	// function, but optimized for initial loads. Every place that used a query now uses member variables.
	_variant_t LoadEMRDetailState(long nEMRDetailID, EmrInfoType nEMRInfoDatatype, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

private:
	// (c.haag 2007-02-28 13:30) - PLID 24989 - This function determines the default state of pDetail,
	// loads it into pDetail, and then repeats for all details whose state affects the appearance
	// of pDetail
	// (c.haag 2007-04-12 09:08) - PLID 25516 - Added nAllergiesInfoID, which is the official
	// Allergies item info ID, so that the function can do special handling for the official Allergies
	// item
	// (c.haag 2007-07-02 09:24) - PLID 26515 - We no longer need the array of all EMN details input here
	// now that we maintain them in a member list
	// (c.haag 2007-07-05 11:48) - PLID 25239 - We now need the load info object
	//TES 6/6/2008 - PLID 29416 - Took out the nCurrentMedicationsInfoID and nAllergiesInfoID, we no longer handle them
	// specially.
	// (j.jones 2008-09-22 15:48) - PLID 31408 - added nEMRGroupID
	void LoadEMRDetailStateDefaultCascaded(CEMNDetail* pDetail, long nPatientID, long nEMRGroupID,
											ADODB::_Connection *lpCon, EMRTopicLoadInfo* pLoadInfo);

	// (c.haag 2007-04-24 08:43) - PLID 25759 - This is an overload of the global LoadEMRDetailStateDefault
	// function, but optimized for initial loads
	// (j.jones 2008-09-22 15:20) - PLID 31408 - added nEMRGroupID as a parameter
	// (z.manning 2011-11-16 12:21) - PLID 38130 - Removed default parameters and added an output parameter for 
	// the remembered detail ID from which we loaded the state.
	_variant_t LoadEMRDetailStateDefault(long nEmrInfoID, long nPatientID, long nEMRGroupID, long nEmrTemplateDetailID, IN ADODB::_Connection *lpCon, OUT long &nRememberedDetailID);

public:
	// (z.manning 2011-02-23 09:21) - PLID 42549
	void EnsureLinkedSmartStampImageStatesLoaded(CEMNDetail *pSmartStampTable, ADODB::_Connection *lpCon);

//////////////////////////////////////////////////////////////////////
// EMR Info functions
//////////////////////////////////////////////////////////////////////
public:
	// (c.haag 2007-04-26 10:49) - PLID 25790 - Returns an EmrInfo object. This must be called after
	// the details have all been preloaded
	CEMRInfoItem* GetEmrInfoItem(long nEmrInfoID);

//////////////////////////////////////////////////////////////////////
// Action functions
//////////////////////////////////////////////////////////////////////
public:
	// (c.haag 2007-04-25 11:37) - PLID 25774 - Returns an array of EMR actions given a source detail.
	// This is usually called by CEMR::ProcessEMRInfoActions in the initial load.
	// (c.haag 2007-06-29 09:42) - PLID 25774 - Added "const" to ensure the map would not be modified
	MFCArray<EmrAction>* GetEMRInfoActions(CEMNDetail *pSourceDetail);

	// (c.haag 2007-04-25 11:37) - PLID 25774 - Returns an array of EMR actions given a data ID.
	// This is usually called by CEMR::ProcessEMRDataActions in the initial load.
	// (c.haag 2007-06-28 13:14) - PLID 25774 - Added "const" to ensure the map would not be modified
	// (j.jones 2013-01-09 16:42) - PLID 54541 - this now takes in multiple data IDs and fills a provided action array
	void GetEMRDataActions(IN CArray<long, long> &aryEMRDataIDs, OUT MFCArray<EmrAction> *parActions);

	// (z.manning, 01/22/2008) - PLID 28690 - Returns an array of EMR actions given a hot spot ID.
	MFCArray<EmrAction>* GetEMRImageHotSpotActions(CEMNDetail *pSourceDetail, long nHotSpotID);

	// (z.manning 2009-02-16 11:51) - PLID 33070 - Returns an array of EMR actions given a dropdown ID
	MFCArray<EmrAction>* GetEMRTableDropdownItemActions(CEMNDetail *pSourceDetail, long nDropdownID);

//////////////////////////////////////////////////////////////////////
// Linked detail functions
//////////////////////////////////////////////////////////////////////
public:
	// (c.haag 2007-06-06 15:52) - PLID 26240 - Returns the number of elements in the
	// m_aEmrLinkedDataItems array
	long GetEmrLinkedDataItemCount();

	// (c.haag 2007-06-06 15:52) - PLID 26240 - Returns an element in the m_aEmrLinkedDataItems
	// array
	EmrLinkedDataItem GetEmrLinkedDataItem(long nIndex);

//////////////////////////////////////////////////////////////////////
// EMR Problem functions
//////////////////////////////////////////////////////////////////////
public:
	// (c.haag 2008-07-18 16:17) - PLID 30784 - Given a regarding type and ID, this function
	// returns a list of problems for a given EMR object
	// (c.haag 2009-05-16 11:19) - PLID 34277 - Instead of returning an array of problems, we
	// now return an array of problem links
	CEmrProblemLinkAry* GetEmrProblemLinks(EMRProblemRegardingTypes RegardingType, long nRegardingID);

//////////////////////////////////////////////////////////////////////
// Internal Preloading functions
//////////////////////////////////////////////////////////////////////
private:
	// (c.haag 2007-04-24 08:29) - PLID 25759 - Populates a map that associates EmrInfoID's with
	// default single/multi-select list selections. Only EmrInfoID's that exist in m_mapInfoItems
	// are used in this map
	void EnsureEmrInfoDefaultsMap(ADODB::_Connection *lpCon);
	// (c.haag 2007-07-11 13:24) - PLID 26629 - This function pulls data from a query
	// which was intended to populate m_mapInfoItems with data
	void ReadEmrInfoDefaultsMapRecord(ADODB::_RecordsetPtr& prs);
	// (c.haag 2007-07-17 10:53) - PLID 26707 - This returns the SQL string to be used
	// later with ReadEmrInfoDefaultsMapRecord
	//DRT 7/18/2007 - PLID 26726 - Added query parameters array for SQL parameterization
	// (z.manning 2011-10-25 08:39) - PLID 39401 - Changed return type to SQL fragment
	CSqlFragment GetEnsureEmrInfoDefaultsMapSql() const;

	// (c.haag 2007-04-27 09:27) - PLID 26446 - Populates a map that associates EMR detail ID's
	// with single/multi-select list selections. Only details that exist in m_pAllDetails are used
	// in this map
	void EnsureSelectMap(ADODB::_Connection *lpCon);
	// (c.haag 2007-07-11 13:25) - PLID 26629 - This function pulls data from a query
	// which was intended to populate m_mapSelect with data
	void ReadSelectMapRecord(ADODB::_RecordsetPtr& prs);
	// (c.haag 2007-07-17 11:04) - PLID 26707 - Returns the text for a query to be used later
	// with ReadSelectMapRecord
	//DRT 7/18/2007 - PLID 26726 - Added query parameters array for SQL parameterization
	CString GetEnsureSelectMapSql(CDWordArray *paryQueryParams) const;
	// (c.haag 2007-08-29 15:18) - PLID 25758 - This map populates mapDetails with ID's
	// used in the select map
	void GetSelectMapDetailIDs(CMap<long,long,BOOL,BOOL>& mapDetails) const;

	// (c.haag 2007-04-24 08:30) - PLID 26459 - Populates a map that associates EMR template detail
	// ID's with default single/multi-select list selections. Only details that exist in m_apAllDetails
	// are used in this map
	void EnsureTemplateSelectMap(ADODB::_Connection *lpCon);
	// (c.haag 2007-07-11 14:34) - PLID 26629 - This function pulls data from a query
	// which was intended to populate m_mapTemplateSelect with data
	void ReadTemplateSelectMapRecord(ADODB::_RecordsetPtr& prs);
	// (c.haag 2007-07-17 11:00) - PLID 26707 - This function returns query text to be
	// later used with ReadTemplateSelectMapRecord
	//DRT 7/18/2007 - PLID 26726 - Added query parameters array for SQL parameterization
	CString GetEnsureTemplateSelectMapSql(CDWordArray *paryQueryParams) const;

	//DRT 1/23/2008 - PLID 28698 - Created a HotSpot map, and all relevant functions
	void EnsureHotSpotMap(ADODB::_Connection *lpCon);
	void CEMNLoader::ReadHotSpotMapRecord(ADODB::_RecordsetPtr& prs);
	CString GetEnsureHotSpotMapSql(CDWordArray *paryQueryParams) const;
	void GetHotSpotMapDetailIDs(CMap<long,long,BOOL,BOOL>& mapDetails) const;

	void EnsureTemplateHotSpotMap(ADODB::_Connection *lpCon);
	void ReadTemplateHotSpotMapRecord(ADODB::_RecordsetPtr& prs);
	CString GetEnsureTemplateHotSpotMapSql(CDWordArray *paryQueryParams) const;

	// (c.haag 2007-06-04 17:49) - PLID 26444 - Populates a map that associated EMR detail ID's with
	// table strings. Only details that exist in m_apAllDetails are used in this map.
	void EnsureTableMap(ADODB::_Connection *lpCon);
	// (c.haag 2007-07-11 14:37) - PLID 26629 - This function pulls data from a query
	// which was intended to populate m_mapTable with data
	void ReadTableMapRecord(ADODB::_RecordsetPtr& prs);
	// (c.haag 2007-07-17 11:01) - PLID 26707 - Returns the SQL statement for a query to be used
	// later with ReadTableMapRecord
	//DRT 7/18/2007 - PLID 26726 - Added query parameters array for SQL parameterization
	// (z.manning 2011-02-25 13:01) - PLID 42597 - Changed the return type to be a SQL fragment
	CSqlFragment GetEnsureTableMapSql() const;
	// (c.haag 2007-08-29 15:18) - PLID 25758 - This map populates mapDetails with ID's
	// used in the table map
	void GetTableMapDetailIDs(CMap<long,long,BOOL,BOOL>& mapDetails) const;

	// (c.haag 2007-04-24 09:58) - PLID 25761 - Populates a map that associates EMR template detail
	// ID's with default table strings. Only details that exist in m_apAllDetails
	// are used in this map
	void EnsureTemplateTableMap(ADODB::_Connection *lpCon);
	// (c.haag 2007-07-11 14:38) - PLID 26629 - This function pulls data from a query
	// which was intended to populate m_mapTemplateTable with data
	void ReadTemplateTableMapRecord(ADODB::_RecordsetPtr& prs);
	// (c.haag 2007-07-17 10:58) - PLID 26707 - We call this function to get the query
	// to be used later with ReadTemplateTableMapRecord
	//DRT 7/18/2007 - PLID 26726 - Added query parameters array for SQL parameterization
	CString GetEnsureTemplateTableMapSql(CDWordArray *paryQueryParams) const;

	// (c.haag 2007-04-25 14:00) - PLID 25758 - Populates a map of possible candidate details for
	// "Remember this patient's value" querying
	// (j.jones 2008-09-22 16:13) - PLID 31408 - added nEMRGroupID
	void EnsureRememberCandidateMaps(long nPatientID, long nEMRGroupID, ADODB::_Connection *lpCon);

	// (c.haag 2007-04-24 13:39) - PLID 25758 - Populates a map that associated EMN detail values with
	// preloaded detail objects. This map is only populated when loading a template with "Remember for
	// this patient" details where we need to load additional detail data when calling LoadEMRDetailState
	void EnsureRememberedDetailMap(ADODB::_Connection *lpCon);

	// (c.haag 2007-04-25 09:51) - PLID 25774 - Populates the existing map of CEMRInfoItems with per-item
	// action data, and also populates m_mapActions with per-list-item-selection action data. This is
	// used to minimize the number of database accesses when EMR actions are processed in the initial load
	// (z.manning, 02/21/2008) - PLID 28690 - This now populates m_mapImageHotSpotActions as well.
	void EnsureEmrActionData(EMRTopicLoadInfo *pLoadInfo, ADODB::_Connection *lpCon);

	//DRT 1/21/2008 - PLID 28603 - This is a more generic function for ensuring maps.  This now handles
	//	both the data map and the hot spot map.  If you add a new, simliar map, this function works
	//	on data type of the info item (single select, multi, image, etc).
	void EnsureEmrMapGeneric(ADODB::_Connection *lpCon, BOOL bLoadActionsType, EMRTopicLoadInfo *pLoadInfo);


	// (c.haag 2007-04-26 09:46) - PLID 25790 - Populates the existing map of CEMRInfoItems with per-item
	// EmrDataT data. This is used in CEMNDetail::LoadContent, and must be called after the CEMRInfoItem
	// map is already populated (which is done in the mass preloading of details)
	//DRT 1/21/2008 - PLID 28603 - Combined into EnsureEmrMapGeneric
	//void EnsureEmrDataTMap(ADODB::_Connection *lpCon, BOOL bLoadActionsType, EMRTopicLoadInfo *pLoadInfo);

public:
	// (c.haag 2007-06-06 15:22) - PLID 26240 - Load all possible EmrItemLinkedDataT
	// entries for this EMN / template. For efficiency purposes, this is actually called
	// from outside this class.
	void EnsureEmrItemLinkedDataTArray(ADODB::_Connection *lpCon);

//////////////////////////////////////////////////////////////////////
// Utility functions
//////////////////////////////////////////////////////////////////////
public:
	// (c.haag 2007-08-13 11:43) - PLID 27049 - Returns TRUE if we are loading from
	// template data.
	inline BOOL IsLoadingFromTemplate() const { return m_bLoadFromTemplate; }

private:
	// (c.haag 2007-08-02 16:54) - PLID 26833 - This function returns a Live detail given
	// a detail, regardless of whether it's Live or Internal. For EMN loader use only. 
	CEMNDetail* GetLiveDetail(const CEMNDetail* pDetail) const;

public:
	//TES 6/6/2008 - PLID 29416 - We no longer handle these items specially.
	/*// (c.haag 2007-04-20 09:06) - PLID 25730 - Returns the active Current Medications EMR info ID
	// (j.jones 2007-07-20 14:41) - PLID 26742 - renamed to differentiate from the EmrUtils function of the same name
	long GetLoaderActiveCurrentMedicationsInfoID(CEMR *pEMR, ADODB::_ConnectionPtr& pCon);

	// (c.haag 2007-04-20 09:06) - PLID 25730 - Returns the active Allergies EMR info ID
	// (j.jones 2007-07-20 14:41) - PLID 26742 - renamed to differentiate from the EmrUtils function of the same name
	long GetLoaderActiveAllergiesInfoID(CEMR *pEMR, ADODB::_ConnectionPtr& pCon);*/

public:
	// (c.haag 2008-07-18 16:05) - PLID 30784 - Returns a map given an Emr Problem regarding type enumeration
	// (c.haag 2009-05-16 12:42) - PLID 34277 - Now returns problem links
	CEmnLoaderProblemLinkMap* GetEmrProblemLinkMap(EMRProblemRegardingTypes type);

private:
	// (c.haag 2008-07-18 16:11) - PLID 30784 - Clears an EMR problem map
	void ClearEmrProblemMap(CEmnLoaderProblemLinkMap* pMap);

private:
	// (c.haag 2007-05-08 13:06) - PLID 25790 - This builds a subquery for use in a larger query that requires all
	// used EmrInfoID's for the loader
	//DRT 7/18/2007 - PLID 26726 - Added query parameters array for SQL parameterization
	// (z.manning 2011-10-25 08:39) - PLID 39401 - Changed return type to SQL fragment
	CSqlFragment BuildAllRelatedEmrInfoTQ() const;

	// (c.haag 2007-05-08 13:19) - PLID 26459 - This builds a subquery for use in a larger query that requires
	// all used list details
	//DRT 7/18/2007 - PLID 26726 - Added query parameters array for SQL parameterization
	CString BuildAllRelatedListDetailQ(CDWordArray *paryQueryParams) const;

	//DRT 1/23/2008 - PLID 28698
	CString BuildAllRelatedImageDetailQ(CDWordArray *paryQueryParams) const;

	// (c.haag 2007-05-08 13:24) - PLID 25761 - This builds a subquery for use in a larger query that requires
	// all used table details
	//DRT 7/18/2007 - PLID 26726 - Added query parameters array for SQL parameterization
	CString BuildAllRelatedTableDetailQ(CDWordArray *paryQueryParams) const;

private: // (c.haag 2007-07-03 09:20) - PLID 26523 - This should not be called from outside this class
	// (c.haag 2007-07-02 09:08) - PLID 26515 - This function encapsulates the act of populating
	// the apDependents array with all details which depend on pDetail's appearance. The only time
	// we actually do any work is if the detail is a narrative, in which case narrative field
	// appearances depend on other details. It is those details we store in apDependents.
	// (c.haag 2007-07-20 13:13) - PLID 26651 - Added a connection pointer parameter for use with
	// BuildTableDependentList
	void BuildDependentList(CEMNDetail* pDetail, OUT CArray<CEMNDetail*, CEMNDetail*>& apDependents,
		ADODB::_Connection *lpCon);
	// (c.haag 2007-07-16 09:59) - PLID 26651 - Go through the state checking the ID's of
	// linked detail columns, and add the details which correspond to those ID's to apDependents.
	// This is only called by BuildDependentList.
	void BuildTableDependentList(CEMNDetail* pDetail, OUT CArray<CEMNDetail*, CEMNDetail*>& apDependents,
		ADODB::_Connection *lpCon);

	// (j.jones 2010-02-12 14:19) - PLID 37318 - called whenever any detail is added to the EMN, to determine
	// whether the detail is a SmartStamp image or table, and links to its related item if it exists or
	// creates a new table if the image was just added
	// (z.manning 2010-03-12 11:54) - PLID 37412 - Renamed this function to indicate it ensures smart stamp
	// link for internal (not live) details in the loader.
	void EnsureInternalDetailSmartStampLinks(CEMNDetail *pDetail);

	// (j.jones 2010-03-10 11:14) - PLID 37318 - added a special loader version of ReconfirmSmartStampLinks,
	// which compares live details by template detail ID
	// (c.haag 2010-06-29 12:09) - PLID 39404 - Added pInternalDetail. This should only be assigned when
	// the detail being processed was potentially spawned.
	void ReconfirmSmartStampLinks_ByLiveDetail(CEMNDetail *pDetail, CEMNDetail* pInternalDetail);

public:
	// (c.haag 2007-07-05 12:21) - PLID 25239 - This function must be called from
	// CEMRTopic::PostLoadDetails for the purpose of writing loaded detail data from
	// the CEMNLoader object to a CEMN object. This should only be called from the main
	// thread.
	void FlushPendingEMNDetailWrites();

private:
	// (c.haag 2007-07-27 09:50) - PLID 26833 - This function was split from FlushPendingEMNDetailWrites.
	// The purpose of this function is to load the state of a "Live" detail, thereby rendering it fully
	// loaded and ready to assign to a CEMN topic.
	void FinalizeDetailTransitionToEMN(CEMNDetail* pLiveDetail);

public:
	// (c.haag 2007-08-04 09:44) - PLID 26945 - This function will raise a debug assertion if an 
	// unexpected condition is met after the intial load has finished. Returns TRUE if it is safe to
	// delete this object.
	BOOL AssertIfPostInitialLoadIssuesExist();

private:
	// (c.haag 2007-09-10 13:03) - PLID 27340 - These functions free up allocated memory not
	// directly related to topic and detail objects
	void FreeActionsMap();
	void FreeEmrInfoMap();
	void FreeRememberedDetailMaps();
	void FreeSelectMaps();
	void FreeImageHotSpotActionsMap(); // (z.manning, 02/21/2008) - PLID 28690
	void FreeTableDropdownItemActionsMap(); // (z.manning 2009-02-16 11:55) - PLID 33070

public:
	// (c.haag 2007-09-10 13:00) - PLID 27340 - When this is called, it means this loader object will
	// no longer be used to actively spawn or add data to the parent EMN. This will free all memory 
	// not directly related to topic and detail objects.
	void Retire();

public:
	// (c.haag 2007-05-15 12:45) - Returns the estimated minimum memory usage, in bytes, of this object
	DWORD GetEstimatedMinimumMemoryUsage() const;

	// (j.jones 2011-04-28 14:39) - PLID 43122 - cache the nProviderIDForFloatingData
	long m_nProviderIDForFloatingData;
};

#endif // !defined(AFX_EMNLoader_H__FF53C951_5B77_44CE_A77A_3FB62098C313__INCLUDED_)

//EMNDetail.h

#ifndef EMNDETAIL_H
#define EMNDETAIL_H

#pragma once

#include <boost/optional.hpp>
#include <boost/none.hpp>

#include "EMRUtils.h"
#include "EmrTopicWnd.h" // (b.savon 2012-06-07 15:31) - PLID 49144
// (a.walling 2010-03-08 09:46) - PLID 37640 - moved non-CEMNDetail stuff to EMNDetailStructure.cpp/.h
#include "EMNDetailStructures.h"
#include <vector>

#include <NxUILib/NxImage.h>

// (j.jones 2013-05-16 10:57) - PLID 56596 - removed a number of .h files

const long gc_nStaticTopMargin = -2;
const long gc_nStaticBorderWidth = 12;

class CEmrItemAdvDlg;
class CEmrItemAdvNarrativeDlg;
class CEMRTopic;
class CEMR;
class CEMN;
class CEMNDetail;
class CEMRItemAdvPopupDlg;
class CEMNLoader;
class CEMNSpawner;
class CEmrDebugDlg;
class TextString;
class CImageArray;
class CEmrItemAdvTableBase;
struct NarrativeField;	// (j.armen 2012-12-03 13:56) - PLID 52752 - Forward Declare
class CEmrInfoCommonListCollection;
class CEMRHotSpotArray;
class CEMRItemAdvPopupWnd;

// (a.walling 2012-10-31 17:17) - PLID 53550 - LastSavedDetail - Keep track of the detail's last saved state in a safer way
// No pointers, no references, copyable, etc.
namespace Emr
{
	// (a.walling 2012-10-31 17:17) - PLID 53551 - Simpler ListElement structure
	struct SimpleListElement
	{
		SimpleListElement(long nID = -1, long nDataGroupID = -1, BOOL bIsSelected = FALSE)
			: nID(nID)
			, nDataGroupID(nDataGroupID)
			, bIsSelected(bIsSelected)
		{}

		long nID;
		long nDataGroupID;
		BOOL bIsSelected;
	};

	// (a.walling 2012-10-31 17:17) - PLID 53552 - Simpler TableRow structure
	struct SimpleTableRow
	{
		SimpleTableRow(long nID = -1, long nEmrDetailImageStampID = -1)
			: nID(nID)
			, nEmrDetailImageStampID(nEmrDetailImageStampID)
		{}

		long nID;
		long nEmrDetailImageStampID;
	};

	// (a.walling 2012-10-31 17:17) - PLID 53552 - Simpler TableColumn structure
	struct SimpleTableColumn
	{
		SimpleTableColumn(long nID = -1)
			: nID(nID)
		{}

		long nID;
	};

	// (a.walling 2012-10-31 17:17) - PLID 53552 - Simpler TableElement structure
	struct SimpleTableElement
	{
		SimpleTableElement(long nRowID = -1, long nEmrDetailImageStampID = -1, long nColumnID = -1, const _variant_t& varValue = _variant_t())
			: row(nRowID, nEmrDetailImageStampID)
			, col(nColumnID)
			, varValue(varValue)
		{}

		SimpleTableRow row;
		SimpleTableColumn col;
		_variant_t varValue;
	};

	class LastSavedDetail
	{
	public:
		LastSavedDetail()
			: m_nEMRDetailID(-1)
			, m_nEMRTemplateDetailID(-1)
		{}

		long m_nEMRDetailID;
		long m_nEMRTemplateDetailID;

		const CString& GetLabelText() const
		{ return m_strLabelText; }

		const CString& GetMergeFieldName() const
		{ return m_strMergeFieldOverride.IsEmpty() ? m_strLabelText : m_strMergeFieldOverride; }

		const CRect& GetDefaultClientArea() const
		{ return m_rcDefaultClientArea; }

		const CString& GetAuditName() const
		{ return GetMergeFieldName(); }

		const CString& GetAuditData() const
		{ return m_strAuditData; }

		void GetDetailNameAndDataForAudit(CString& strName, CString& strData) const
		{
			strName = GetAuditName();
			strData = m_strAuditData;
		}
		
		CString m_strLabelText;
		CString m_strMergeFieldOverride;

		CRect m_rcDefaultClientArea;

		CString m_strAuditData;

		std::vector<long> m_selectedHotSpotIDs; // (a.walling 2012-10-31 17:17) - PLID 53551 - hotspot ids
		std::vector<EmrDetailImageStamp> m_imageStamps; // (a.walling 2012-10-31 17:17) - PLID 53551 - EmrDetailImageStamp types are safe to copy, no external references
		std::vector<SimpleListElement> m_listElements; // (a.walling 2012-10-31 17:17) - PLID 53551 - for comparing with ListElements

		// (a.walling 2012-10-31 17:17) - PLID 53552 - for comparing with TableRow, TableColumn, TableElement types
		std::vector<SimpleTableRow> m_tableRows;
		std::vector<SimpleTableColumn> m_tableColumns;
		std::vector<SimpleTableElement> m_tableElements;
	};

} // namespace Emr

// (a.walling 2011-08-11 16:43) - PLID 45021 - TableRow.m_pID is now TableRow.m_ID, which is not allocated on the heap.


// (a.walling 2009-10-13 13:57) - PLID 36046 - Ability to keep reference count history
#ifdef _DEBUG
// (a.walling 2009-11-03 17:23) - PLID 36023 / 36024 - Haven't noticed any issues with this, so I am turning off the tracing
//#define WATCH_EMNDETAIL_REFCOUNTS
//#define LOG_ALL_EMNDETAIL_REFCOUNTS
#endif

// (a.walling 2009-10-13 13:57) - PLID 36046 - Ability to keep reference count history and dump
#ifdef WATCH_EMNDETAIL_REFCOUNTS
class CEMNDetailManager
{
protected:
	CMap<CEMNDetail*, CEMNDetail*, BOOL, BOOL> m_mapAllDetails;
	CCriticalSection m_cs;

public:
	CEMNDetailManager() {

	};

	void RegisterDetail(CEMNDetail* newDetail);
	void UnregisterDetail(CEMNDetail* dyingDetail);
	DWORD GetCount();
	void DumpAll();
};

extern CEMNDetailManager g_EMNDetailManager;
#endif


// (a.walling 2010-03-08 09:46) - PLID 37640 - moved non-CEMNDetail stuff to EMNDetailStructure.cpp/.h

enum GlassesOrderLens;
class CEMNDetail
{
	// (a.walling 2007-08-31 10:04) - PLID 19106
	friend class CEmrItemAdvTableDlg;
	friend class CEMNDetailArray; // (z.manning 2011-01-20 17:35) - PLID 42338
	// (a.walling 2009-10-23 10:51) - PLID 36046 - Track construction in initial reference count
public:	
	// (c.haag 2011-05-31) - PLID 43875 - We now pass this into calls to AddRow
	enum EAddRowPlacement
	{
		aroSequential,
		aroInsertAt,
		aroAlphabetically,
	};

// (a.walling 2010-03-08 09:46) - PLID 37640 - These must be here in order to stay inline

// (a.walling 2010-03-12 09:03) - PLID 37640 - Moved CEMNDetail creation to cpp
#if defined(WATCH_EMNDETAIL_REFCOUNTS) || defined(LOG_ALL_EMNDETAIL_REFCOUNTS)
	static CEMNDetail* __CreateDetail(CEMRTopic* pParentTopic, char* szInitialRefDescription);

	static CEMNDetail* __CreateDetail(CEMRTopic* pParentTopic, char* szInitialRefDescription, BOOL bOwnTopicIfNull);

#define CreateDetail(pParentTopic, description, ...) __CreateDetail(pParentTopic, description, __VA_ARGS__)
#else
	static CEMNDetail* __CreateDetail(CEMRTopic* pParentTopic);

	static CEMNDetail* __CreateDetail(CEMRTopic* pParentTopic, BOOL bOwnTopicIfNull);

#define CreateDetail(pParentTopic, description, ...) __CreateDetail(pParentTopic, __VA_ARGS__)
#endif

private:
	// (a.walling 2009-10-23 10:51) - PLID 36046 - Track construction in initial reference count
#if defined(WATCH_EMNDETAIL_REFCOUNTS) || defined(LOG_ALL_EMNDETAIL_REFCOUNTS)
	CEMNDetail(CEMRTopic* pTopic, char* szInitialRefDescription, BOOL bOwnTopicIfNull);
#else
	CEMNDetail(CEMRTopic* pTopic, BOOL bOwnTopicIfNotNull);
#endif

	// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
	ADODB::_ConnectionPtr GetRemoteData();

	// (a.walling 2009-10-23 10:51) - PLID 36046 - This is just confusing. 
	// The only difference is that new CEMNDetail(NULL) sets m_bOwnTopic = TRUE, while CEMNDetail() does not.
	// this is all handled in the constructor itself now, and it's accessor macros for the reference count tracking.
					//This is for details that are being created and loaded independently, which will later be attached to a topic.
// (a.walling 2009-10-12 17:16) - PLID 36046 - Only delete via release so we can track properly
private:
	~CEMNDetail();

public:
#ifdef _DEBUG
	// (c.haag 2007-08-07 11:45) - PLID 26946 - This is used for debugging and developer testing with EMR
	// (c.haag 2007-09-17 10:35) - PLID 27401 - We now include the dialog itself
	// (c.haag 2007-09-17 13:07) - PLID 27408 - We now support suppressing address information
	void DebugReportMembers(CEmrDebugDlg* pDlg, BOOL bOnlyInclude8300Fields, BOOL bAllowAddresses);
#endif

public:
	// (a.walling 2012-10-31 17:17) - PLID 53550 - LastSavedDetail - Deprecating this, just use CopyFrom
	// overloading operator= for something as complex as a detail is just confusing.
	__declspec(deprecated("No assignment operator")) void operator =(CEMNDetail &edSource)
	{
		CopyFrom(edSource);
	}

	void CopyFrom(CEMNDetail& edSource);

	// (a.walling 2012-10-31 17:17) - PLID 53550 - LastSavedDetail - boost::optional emulates pointer semantics somewhat
	boost::optional<Emr::LastSavedDetail> m_oLastSavedDetail;
	Emr::LastSavedDetail CreateLastSavedDetail();

	// (a.walling 2010-04-01 10:31) - PLID 38013 - Added functions to get the parent topic, EMN, EMR
	CEMRTopic* GetParentTopic();
	CEMRTopic* GetOriginalParentTopic();
	CEMN* GetParentEMN();
	CEMR* GetParentEMR();

	// (a.walling 2010-04-01 10:31) - PLID 38013 - Added function to get the top-level parent topic
	CEMRTopic* GetTopLevelParentTopic();

	// (j.jones 2010-02-25 14:35) - PLID 37318 - ReconfirmSmartStampLinks will simply ensure that
	// if this detail is pointing to an image or table, that image/table is also pointing to us.
	void ReconfirmSmartStampLinks();

	void ShowDetailDlg() const;	//called by CEMRTopic to show the EMNDetail's EMRItemAdvDlg
	void HideDetailDlg() const;	//called by CEMRTopic to hide the EMNDetail's EMRItemAdvDlg

	//TES 10/9/2006 - PLID 22932 - We need to track which objects are already in our save string.
	// (j.jones 2007-01-11 14:28) - PLID 24027 - tracked strPostSaveSql, for sql statements to occur after the main save
	// (c.haag 2007-06-20 12:38) - PLID 26397 - We now store saved objects in a map for fast lookups
	// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
	Nx::Quantum::Batch GenerateSaveString(long nEMNTopicID, long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN OUT CMapPtrToPtr& mapSavedObjects, CDWordArray &arynModifiedEMRDetailIDs, BOOL bIsTopLevelSave);
	BOOL PropagateNewID(long nID, EmrSaveObjectType esotSaveType, long nObjectPtr, long &nAuditTransactionID);
	// (a.walling 2007-10-18 16:40) - PLID 27664 - Added array to gather all topics affected in the PostSaveUpdate cascade.
	void PostSaveUpdate(BOOL bTopLevelUpdate = FALSE, CArray<CEMRTopic*, CEMRTopic*> *parTopicsAffected = NULL);
	// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
	Nx::Quantum::Batch GenerateDeleteString(long &nAuditTransactionID, CStringArray &arystrErrors, CDWordArray &arynAffectedDetailIDs);
	// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
	void LogEmrObjectData(int nIndent, BOOL bForceDeletedFlagTrue, BOOL bInTopicPendingDeletionDetailsAry);

	// (j.jones 2011-07-22 17:26) - PLID 43504 - added optional connection pointer
	BOOL GetIsOnLockedAndSavedEMN(OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	// (r.gonet 06/10/2011) - PLID 30359 - Gets whether this detail is changing state currently.
	inline bool IsStateChanging()
	{
		return m_bStateChanging;
	}
	
protected:
	// (c.haag 2007-06-20 12:41) - PLID 26397 - We now store saved objects in a map for fast lookups.
	Nx::Quantum::Batch GenerateSaveString_Template(long nEMNTopicID, long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN CMapPtrToPtr& mapSavedObjects);
	// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs (not EMRTemplateDetailIDs, by the way, which is why this is only on this function, not the _Template version)
	Nx::Quantum::Batch GenerateSaveString_PatientEMN(long nEMNTopicID, long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN CMapPtrToPtr& mapSavedObjects, CDWordArray &arynModifiedEMRDetailIDs);
public:
	// (z.manning 2010-02-18 14:53) - PLID 37404 - Added pmapSavedObjects and strPostSaveSql parameters
	Nx::Quantum::Batch GenerateSaveString_State(const CString& strID, CStringArray &arystrErrors, Nx::Quantum::Batch& strPostSaveSql, IN CMapPtrToPtr *pmapSavedObjects);

protected:
	// (j.jones 2006-02-21 11:59) - PLID 19386 - stores the state of the detail when it was loaded (or last saved), for auditing purposes
	// (a.walling 2012-10-31 17:17) - PLID 53550 - CEMNDetail* m_pLastSavedDetail? ... never again
	//CEMNDetail *m_pLastSavedDetail;	

	void GetDetailNameAndDataForAudit(CString &strDetailName, CString &strDetailData);

protected:
	// (c.haag 2011-03-17) - PLID 42895 - Common list collection
	// (j.jones 2013-05-16 11:00) - PLID 56596 - changed this into a reference, in order to not require its .h file
	CEmrInfoCommonListCollection &m_CommonLists;
public:
	const CEmrInfoCommonListCollection& GetCommonLists() { return m_CommonLists; }

public:

public:

	// (a.walling 2012-06-22 14:01) - PLID 51150 - dead code
	//CRect ShowItemDialog(IN CWnd* pParentWindow);
	// (b.savon 2012-06-07 15:31) - PLID 49144 - Added optional device import settings
	// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicit CEmrTopicWnd parent
	BOOL ShowItemDialogInRgn(IN CEmrTopicWnd* pParentWindow, IN OUT CRgn &rgn, IN OUT CRect &rcArea, OPTIONAL OUT CRect *prc, const BOOL bForceRecalc, OPTIONAL IN LPCRECT prcOverrideRect = NULL, OPTIONAL IN const DeviceImportSettings &disSettings = DeviceImportSettings(gc_nStaticBorderWidth, gc_nStaticBorderWidth) );
	
	//TES 12/15/2005 - Call exactly one of these functions to initialize the detail.
	//For a new detail being added to an EMN or template.
	//TES 5/24/2006 - NOTE: The LoadFromInfoID function cannot be called on details that don't have a parent topic, as it asks
	//the parent topic for the Patient ID to use for loading the default state.
	// (j.jones 2007-01-10 14:51) - PLID 24027 - supported SourceDetailID
	// (j.jones 2007-07-25 10:40) - PLID 26803 - changed InfoID parameter name
	//DRT 8/2/2007 - PLID 26919 - Added nSourceActionSourceID
	//DRT 8/14/2007 - PLID 27067 - Added nSourceActionSourceDataGroupID
	//TES 10/8/2007 - PLID 27660 - Added bIsInitialLoad.
	// (z.manning 2009-02-13 09:19) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
	// (z.manning 2009-03-11 09:34) - PLID 33338 - Use the new source action info class
	// (j.jones 2010-06-21 11:36) - PLID 37981 - added ability to pass in generic table content
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	void LoadFromInfoID(long nLoadFromInfoID, BOOL bIsOnTemplate, BOOL bIsInitialLoad, SourceActionInfo &sai, OPTIONAL IN long nSourceActionSourceID = -1, long nSourceActionSourceDataGroupID = -1, long nSourceActionSourceHotSpotGroupID = -1, long nSourceActionSourceTableDropdownGroupID = -1, DevicePluginUtils::TableContent *pGenericTableContent = NULL);
	//TES 12/6/2006 - PLID 23724 - This loads from an EmrInfoMasterT.ID, it just looks up that records ActiveEmrInfoID, and calls
	//  LoadFromInfoID().
	// (j.jones 2007-01-10 14:52) - PLID 24027 - supported SourceDetailID
	// (j.jones 2007-07-25 10:40) - PLID 26803 - changed InfoMasterID parameter name
	//DRT 8/2/2007 - PLID 26919 - Added nSourceActionSourceID
	// (c.haag 2007-08-06 15:26) - PLID 26992 - Added an optional CEMNSpawner parameter for query-free accesses to EMR info data
	//DRT 8/14/2007 - PLID 27067 - Added nSourceActionSourceDataGroupID
	//TES 10/8/2007 - PLID 27660 - Added bIsInitialLoad.
	// (z.manning 2009-02-13 09:20) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
	// (z.manning 2009-03-11 09:34) - PLID 33338 - Use the new source action info class
	// (j.jones 2010-06-21 11:36) - PLID 37981 - added ability to pass in generic table content
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	void LoadFromInfoMasterID(long nLoadFromInfoMasterID, BOOL bIsOnTemplate, BOOL bIsInitialLoad, SourceActionInfo &sai, OPTIONAL IN long nSourceActionSourceID = -1, CEMNSpawner* pEMNSpawner = NULL, long nSourceActionSourceDataGroupID = -1, long nSourceActionSourceHotSpotGroupID = -1, long nSourceActionSourceTableDropdownGroupID = -1, DevicePluginUtils::TableContent *pGenericTableContent = NULL);
	//For an existing EMN Detail
	//TES 6/5/2006 - When calling from a thread, pass in the thread's connection.
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	void LoadFromDetailID(long nDetailID, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
	// (c.haag 2007-02-22 15:25) - PLID 24889 - This function loads in a detail given a recordset. The required fields are listed in the implementation
	// (c.haag 2007-02-27 09:01) - PLID 24949 - Added bLoadState. Set to TRUE when we want to load the detail state.
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	void LoadFromDetailRecordset(ADODB::_RecordsetPtr& rsInfo, BOOL bCloseRecordsetWhenDone, OPTIONAL IN ADODB::_Connection *lpCon = NULL, BOOL bLoadState = TRUE);
	//For an existing EMN Template Detail 
	//bIsNewDetail tells whether the detail being loaded from is the same detail it should be saved to (ignored when not on a template).
	//TES 5/24/2006 - If you call this on a detail that isn't attached to a parent topic, and bIsOnTemplate is FALSE, 
	//you MUST pass in nPatientID
	//TES 6/5/2006 - When calling from a thread, pass in the thread's connection.
	// (j.jones 2007-04-12 14:19) - PLID 25604 - added parameter for bIsInitialLoad
	// (j.jones 2008-09-23 09:41) - PLID 31408 - added parameter for EMRGroupID
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	void LoadFromTemplateDetailID(long nTemplateDetailID, BOOL bIsOnTemplate, BOOL bIsNewDetail, BOOL bIsInitialLoad, IN long nPatientID, IN long nEMRGroupID, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
	// (c.haag 2007-02-22 12:13) - PLID 24881 - This function loads in a detail given a recordset. The required fields are listed in the implementation
	// (c.haag 2007-02-27 09:01) - PLID 24949 - Added bLoadState. Set to TRUE when we want to load the detail state.
	// (j.jones 2007-04-12 14:19) - PLID 25604 - added parameter for bIsInitialLoad
	// (j.jones 2007-07-24 17:05) - PLID 26742 - added parameter for pParentEmr
	// (j.jones 2008-09-23 09:41) - PLID 31408 - added parameter for EMRGroupID
	// (z.manning 2011-01-26 11:33) - PLID 42336 - Added optional parameter for the EMN loader
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	void LoadFromTemplateDetailRecordset(ADODB::_RecordsetPtr& rsInfo, BOOL bCloseRecordsetWhenDone, BOOL bIsOnTemplate, BOOL bIsNewDetail, BOOL bIsInitialLoad, IN long nPatientID, IN long nEMRGroupID, OPTIONAL IN ADODB::_Connection *lpCon = NULL, BOOL bLoadState = TRUE, CEMR *pParentEmr = NULL, CEMNLoader *pEmnLoader = NULL);

protected:

	// (j.jones 2007-07-25 10:03) - PLID 26803 - created to handle taking in an InfoID or a MasterID, basically moving
	// the contents of LoadFromInfoID into this function
	// - called from LoadFromInfoID and LoadFromInfoMasterID
	//DRT 8/2/2007 - PLID 26919 - Added nSourceActionSourceID
	// (c.haag 2007-08-06 15:26) - PLID 26992 - Added an optional CEMNSpawner parameter for query-free accesses to EMR info data
	//DRT 8/14/2007 - PLID 27067 - Added nSourceActionSourceDataGroupID
	//TES 10/8/2007 - PLID 27660 - Added bIsInitialLoad.
	// (z.manning 2009-02-13 09:21) - PLID 33070 - Added SourceActionSourceTableDropdownGroupID
	// (z.manning 2009-03-11 09:35) - PLID 33338 - Use the new source action info class
	// (j.jones 2010-06-21 11:36) - PLID 37981 - added ability to pass in generic table content
	// (a.walling 2012-06-22 14:01) - PLID 51150 - No parent window param
	void LoadFromInfoOrMasterID(long nLoadFromInfoID, long nLoadFromInfoMasterID, BOOL bIsOnTemplate, BOOL bIsInitialLoad, SourceActionInfo &sai, OPTIONAL IN long nSourceActionSourceID = -1, CEMNSpawner* pEMNSpawner = NULL, long nSourceActionSourceDataGroupID = -1, long nSourceActionSourceHotSpotGroupID = -1, long nSourceActionSourceTableDropdownGroupID = -1, DevicePluginUtils::TableContent *pGenericTableContent = NULL);

public:
	// (a.walling 2014-08-18 10:09) - PLID 63029 - Returns true if LoadContent can load from the given loader (or calculated loader / spawner)
	bool IsAvailableInLoaderSpawner(CEMNLoader* pEMNLoader);

	//This loads the content (list elements, slider parameters, background image, basically whatever it is that defines the 
	//parameters of what the user can enter for this item, but NOT what the user has entered for this item, that's all in m_varState).
	// (c.haag 2007-07-02 12:18) - PLID 26515 - Added an optional CEMNLoader object so that we
	// can take advantage of it even if this detail has no parent topic
	// (c.haag 2007-07-20 13:16) - PLID 26651 - Added a connection pointer
	// (j.jones 2010-03-23 12:16) - PLID 37318 - added override for whether to call EnsureLinkedSmartStampTableValid()
	// (j.jones 2010-06-21 15:47) - PLID 37981 - added ability to pass in pDefaultGenericTableContent
	// (j.jones 2011-04-28 14:39) - PLID 43122 - added nProviderIDForFloatingData as an optional field
	void LoadContent(BOOL bForceLoadFromAdmin = FALSE, CEMNLoader* pEMNLoader = NULL,
		ADODB::_Connection *lpCon = NULL, BOOL bEnsureSmartStampTableLinked = TRUE, DevicePluginUtils::TableContent *pDefaultGenericTableContent = NULL,
		long nProviderIDForFloatingData = -1);

	// (z.manning 2011-02-14 09:52) - PLID 42446 - Added a function to just load a smart stamp table's content
	// (z.manning 2011-02-23 09:29) - PLID 42549 - Added params for the loader and a connection object
	void LoadSmartStampTableContent(CEMNLoader *pEmnLoader, ADODB::_Connection *lpCon);

	//Clears out current content
	void ClearContent();	

	//Reflects the current state on screen (what the user has selected).
	void ReflectCurrentState();

	//Reflects the current content on screen (what list elements/slider parameters/whatever are available for the user to select).
	void ReflectCurrentContent();

	//Used for lists and tables, recreates the m_varState variable based on the various content variables.
	void RecreateStateFromContent();

	// (z.manning 2011-03-21 15:06) - PLID 30608 - Function to update any table elements that may need to auto-fill
	// after the given row has been changed.
	BOOL UpdateAutofillColumns(TableElement *pUpdatedTableElement);
	BOOL UpdateAutofillColumnsByRow(TableRow *pUpdatedRow, TableColumn *pUpdatedColumn);

	//TES 1/23/2007 - PLID 24377 - After loading the default state, call this function to apply any links to this detail.
	// This function will only have an effect on multi-select lists, and will only check items, never uncheck them.
	void ApplyEmrLinks();

protected:
	//TES 2/22/2006 - This assumes that the state and content have been loaded, and makes sure they're in sync.  For example, some 
	//of the content variables, just for ease of use, have properties (like whether they're selected), which are actually part 
	//of the state. So this function reads that info out of m_varState (which is the ONLY official place for it) and copies 
	//it to the content variables, and also checks for any m_varState info which isn't valid (for example, selected list items 
	//that no longer exist, or slider values that are outside the valid parameters), and removes it from m_varState.
	void SyncContentAndState();


	// (j.jones 2006-08-18 08:32) - PLID 22078 - same concept as above, but
	// re-maps the state to match a changed info record
	// (c.haag 2011-10-28) - PLID 46170 - Added EMRInfoID parameters
	void SyncContentAndStateWithChangedInfo(long nOldEMRInfoID, long nNewEMRInfoID, EMRInfoChangedIDMap* pChangedIDMap);
	
public:
	// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicit CEmrTopicWnd parent
	void EnsureEmrItemAdvDlg(IN CEmrTopicWnd* pParentWindow, BOOL bForceLoadFromAdmin = FALSE);
	void ReloadEMRItemAdvDlg();
	void ResetEmrItemAdvDlg()
	{
		m_pEmrItemAdvDlg = NULL;
	}

	// (z.manning 2012-06-22 16:28) - PLID 48138 - Added function to ensure topic wnd and adv dialog
	void EnsureTopicWndAndItemAdvDlg();

	CEmrItemAdvDlg* GetEmrItemAdvDlg()
	{
		return m_pEmrItemAdvDlg;
	}

	BOOL SearchRegionForBestFit(IN const CRgn &rgnToFitInto, OUT CRect &rcBestOption, IN const CRect &rcArea);

public:
	CRect m_rcDefaultClientArea;
	//TES 2/7/2007 - PLID 18159 - This tries to get the client area from the dialog, if it doesn't exist it will return
	// m_rcDefaultClientArea
	CRect GetClientArea();
	const CRect& GetClientAreaDirect() const
	{
		return m_rcDefaultClientArea;
	};

	// (a.walling 2007-06-21 12:32) - PLID 22097 - Return the readonly status
	//TES 3/15/2010 - PLID 37757 - This function is now slightly more complicated, and thus implemented in the .cpp
	BOOL GetReadOnly();

protected:
	BOOL m_bReadOnly;
	BOOL m_bVisible;

	// (b.savon 2012-06-06 12:21) - PLID 49144 - Keep an DI flag and add a utility func for DI image imports
	BOOL m_bDeviceImport;
	BOOL IsSafeMove(CEmrTopicWnd* pTopicWnd, CRect rectDetail);
	
	// (a.walling 2007-12-17 15:48) - PLID 28391
	BOOL m_bHTMLVisible;
	BOOL m_bHTMLVisiblePrint;

	// (c.haag 2007-08-02 17:38) - These are used for debugging. Any detail that
	// is directly accessed by an EMN or EMN topic should *NOT* be owned by a
	// CEMNLoader object unless specifically commented.
protected:
	BOOL m_bOwnedByEMNLoader;
public:
	inline void SetOwnedByEMNLoader(BOOL bOwned) { m_bOwnedByEMNLoader = bOwned; }
	inline BOOL IsOwnedByEMNLoader() const { return m_bOwnedByEMNLoader; };

	// (a.walling 2007-12-17 15:55) - PLID 28391
	inline BOOL GetHTMLVisible() {return m_bHTMLVisible;};
	inline BOOL GetHTMLVisiblePrint() {return m_bHTMLVisiblePrint;};

	// (b.savon 2012-06-06 12:22) - PLID 49144 - Mutators/Accessors for DI flag
	inline void SetDeviceImport(BOOL bDeviceImport){ m_bDeviceImport = bDeviceImport;}
	inline BOOL IsDeviceImport(){ return m_bDeviceImport;	}

	// (a.walling 2008-06-30 16:19) - PLID 30571
	inline DWORD GetPreviewFlags() {return m_nPreviewFlags;};

	// (a.walling 2008-10-23 09:48) - PLID 27552 - Whether this item should be displayed underneath
	// another detail or not.
	BOOL IsSubDetail();

	// (a.walling 2008-10-23 09:58) - PLID 27552 - Returns the parent of the subdetail (if any)
	CEMNDetail* GetSubDetailParent();

	// (a.walling 2008-10-24 10:36) - PLID 27552 - Easier function to get the topic this detail is actually displayed under
	CEMRTopic* GetSubDetailParentTopic();

protected:
	// (c.haag 2007-08-06 10:53) - PLID 29654 - Details are assigned CEMNSpawner
	// objects for the purpose of more efficient processing in LoadContent
	CEMNSpawner* m_pEMNSpawner;
public:
	inline void SetEMNSpawner(CEMNSpawner* pEMNSpawner) { m_pEMNSpawner = pEMNSpawner; }
	inline CEMNSpawner* GetEMNSpawner() const { return m_pEMNSpawner; }

public:
	void SetReadOnly(BOOL bReadOnly);

	//If this is being set to visible while loading (that is, not because of a user action), then pass TRUE to bIsInitialLoad,
	//so that m_bUnsaved won't be set to TRUE.
	void SetVisible(BOOL bVisible, BOOL bRedraw, BOOL bIsInitialLoad);
	BOOL GetVisible() const;

protected:
	// (j.jones 2010-02-12 09:26) - PLID 37318 - for SmartStamps, image details have a pointer to a table
	// and tables have pointers to the image
	// (z.manning 2011-01-20 09:52) - PLID 42338 - We now support multiple smart stamp images for one table so
	// we no longer can have just a single pointer to the image detail.
	//CEMNDetail *m_pSmartStampImageDetail;
	CEMNDetailArray m_arySmartStampImageDetails;
	CEMNDetail *m_pSmartStampTableDetail;

	// (r.gonet 02/14/2012) - PLID 37682 - Store the current image filter here
	CTextStringFilterPtr m_pTextStringFilter;

public:
	// (j.jones 2010-02-12 09:26) - PLID 37318 - exposed functions for accessing m_pSmartStampImageDetail and m_pSmartStampTableDetail
	CEMNDetailArray* GetSmartStampImageDetails(); // (z.manning 2011-01-20 17:51) - PLID 42338
	CEMNDetail* GetSmartStampTableDetail();
	// (z.manning 2011-01-20 16:34) - PLID 42338 - Renamed this as it behaves differently now
	void EnsureSmartStampImageDetail(CEMNDetail *pImage);
	// (z.manning 2011-01-20 12:15) - PLID 42338 - Support multiple images per smart stamp table
	void SetSmartStampImageDetails(CEMNDetailArray &aryImages);
	void ClearSmartStampImageDetails();
	void ClearSmartStampImageDetailByEmrInfoMasterID(const long nEmrInfoMasterID);
	void SetSmartStampTableDetail(CEMNDetail *pTable);

	// (j.jones 2010-02-17 15:32) - PLID 37318 - called when reloading an existing SmartStamp Image that
	// is linked to a table, will potentially disconnect the image and the table if the link is now invalid
	void EnsureLinkedSmartStampTableValid();

	// (z.manning 2010-02-18 15:41) - PLID 37412 - Returns true if the table is linked to a smart stamp image
	BOOL IsSmartStampTable();
	// (a.walling 2010-08-23 16:10) - PLID 37923 - Returns true if the image is linked to a smart stamp table
	BOOL IsSmartStampImage();

	// (z.manning 2012-07-03 17:59) - PLID 51025 - True if this is an existing smart stamp image detail that is somehow not
	// linked with a smart stamp table.
	BOOL IsOrphanedSmartStampImage();

	// (z.manning 2011-01-20 12:05) - PLID 42338
	BOOL HasDetailStamp(EmrDetailImageStamp *pDetailStamp);

	// (z.manning 2010-02-18 15:41) - PLID 37412 - Returns true if the table is linked to a smart stamp image
	void UpdateDetailStampPointer(EmrDetailImageStamp *pExisting, EmrDetailImageStamp *pNew);
	// (z.manning 2010-03-11 11:18) - PLID 37412

	// (a.walling 2012-10-31 17:17) - PLID 53552 - LastSavedDetail - LastSavedStamps no longer necessary
	void CreateNewCopiesOfDetailStampPointers();

	// (z.manning 2010-03-11 14:56) - PLID 37571 - Will reassasign the source detail stamp pointer to the given
	// new pointer for any object in this detail that could have potentially been spawned.
	void UpdateSourceDetailStampPointers(EmrDetailImageStamp *pDetailStampOld, EmrDetailImageStamp *pDetailStampNew);

	// (j.jones 2010-03-19 12:22) - PLID 37318 - returns true if has m_arypImageStamps has content
	BOOL HasImageStamps();

	// (z.manning 2011-01-28 15:05) - PLID 42335
	void RefreshSmartStampImagePostAdd();

	// (z.manning 2011-02-02 17:55) - PLID 42335 - Returns true if something changed in the table, false otherwise
	BOOL UpdateSmartStampTableQuantityValues();
	// (z.manning 2011-02-23 13:40) - PLID 42549
	void UpdateSmartStampTableAutoNumberValues();

	// (z.manning 2011-02-25 11:22) - PLID 42335
	// (z.manning 2011-05-11 09:19) - PLID 43568 - Added parameter for whether or not to refresh content
	void RefreshSmartStampTableBuiltInData(BOOL bRefreshContent);

	// (z.manning 2011-02-03 12:20) - PLID 42336 - Returns true if the smart stamp table already has an image item
	// that is a different detail but is the same info item (i.e. has the same EMR info master ID).
	BOOL SmartStampTableAlreadyHasImageItem(CEMNDetail *pImage);

	// (z.manning 2011-03-02 15:19) - PLID 42335
	void UpdateTableStateWithNewDetailStampIDByStampID(const long nStampIDToLookFor, const long nNewDetailStampID);

public:
	// Various content-related properties
	long m_nEMRInfoID;
	long m_nEMRInfoMasterID;
	EmrInfoType m_EMRInfoType;
	BOOL m_bTableRowsAsFields; // (c.haag 2008-10-16 11:19) - PLID 31709

	EmrInfoSubType m_EMRInfoSubType; // (c.haag 2007-01-23 11:01) - PLID 24376 - We can now break types into subtypes for internal purposes

	// (j.armen 2014-07-22 08:58) - PLID 62836 - Helpers for determining if this detail is an image
	inline bool IsImage() { return Is2DImage() || Is3DImage(); }
	inline bool Is2DImage() { return m_EMRInfoType == eitImage && m_EMRInfoSubType == eistNone; }
	inline bool Is3DImage() { return m_EMRInfoType == eitImage && m_EMRInfoSubType == eist3DImage; }// (z.manning 2011-09-26 09:17) - PLID 44649

	CString GetLabelText() const;
	void SetLabelText(const CString &strLabelText);
	long m_nEMRDetailID;
	long m_nEMRSourceTemplateID;
	long m_nEMRTemplateDetailID;
	BOOL m_bIsTemplateDetail;
	BOOL m_bCreatedOnNewEMN;

	// (j.jones 2010-02-11 14:16) - PLID 37318 - added variables for SmartStamp images linked to tables
	long m_nChildEMRInfoMasterID;
	BOOL m_bSmartStampsEnabled;
	long m_nChildEMRDetailID;
	long m_nChildEMRTemplateDetailID;
	// (z.manning 2011-01-25 15:24) - PLID 42336 - These are both deprecated as we now support multiple images
	// for one smart stamp table.
	//long m_nParentEMRDetailID;
	//long m_nParentEMRTemplateDetailID;

	BOOL m_bContentHasBeenLoaded; //used to determine whether LoadContent has ever been called

	// (b.cardillo 2006-11-20 10:37) - PLID 22565 - This is loaded from the special 
	// ReconstructedEMRDetailsT table.  Values can be -1 (reconstructed by nextech), -2 (verified 
	// by the user but leave highlighted bkg), -3 (verified by the user).  If the record for this 
	// detail doesn't exist in the ReconstructedEMRDetailsT table, then the value in this member 
	// variable will be 0.
	long m_nReviewState;

	//Gets the index of this detail in the EMN.
	CString GetUniqueIdentifierAgainstEMN() const;

	//TES 3/17/2011 - PLID 41108 - Added Glasses Order data
	BOOL m_bHasGlassesOrderData;
	GlassesOrderLens m_golLens;
	//TES 4/10/2012 - PLID 43829 - Added HasContactLensData
	BOOL m_bHasContactLensData;
	// (r.gonet 08/03/2012) - PLID 51948 - Added UseWithWoundCareCoding, which tells Practice that this table detail
	//  can be used with the CWoundCareCalculator.
	BOOL m_bUseWithWoundCareCoding;

	// (z.manning 2011-11-16 12:34) - PLID 38130 - If this detail's state was remembered from another detail then
	// this variable has the remembered detail's ID.
	long m_nRememberedDetailID;

protected:
	long m_nInfoFlags; // (z.manning 2011-11-15 17:00) - PLID 38130

	// (a.wetta 2005-11-04 10:48) - PLID 18168 - This variable is used for table items only.
	// (z.manning 2010-09-07 16:03) - PLID 40434 - This is now longer public
	BOOL m_bSaveTableColumnWidths;

	// (r.gonet 06/10/2011) - PLID 30359 - True if we are currently changing the state, false if we are not.
	bool m_bStateChanging;

public:
	// (z.manning 2010-09-07 16:04) - PLID 40434 - Use these now for public access to m_bSaveTableColumnWidths
	BOOL GetSaveTableColumnWidths();
	void SetSaveTableColumnWidths(const BOOL bSaveTableColumnWidths);

	// (j.jones 2006-08-01 10:27) - PLID 21704 - used to store the width of the column with the row
	// names. All other column widths are in the TableColumn object.
	//DRT 7/10/2007 - plid 24105 - We no longer need this.
	//long m_nFirstColumnWidth;

	// (j.jones 2006-08-01 10:30) - PLID 21704 - resets the stored widths in memory to -1
	void ResetStoredColumnWidths();

	// (m.hancock 2006-06-06 12:52) - PLID 20519 - Keep the detailID of the source when copying a detail
	// (j.jones 2009-04-10 09:23) - PLID 33956 - renamed from m_nSourceEMRDetailID to m_nCopiedFromEMRDetailID
	long m_nCopiedFromEMRDetailID;

	// (j.jones 2004-12-20 15:08) - caches sentence format to minimize data access
	//right now only implemented with tables
	CString m_strSentenceHTML;
	CString m_strSentenceNonHTML;
	EmrMultiSelectFormat m_emsf;
	CString m_strSeparator;
	CString m_strSeparatorFinal;

	// (a.walling 2013-06-06 09:41) - PLID 57069 - Keep track of the last rendered image state so redundant generation can be avoided
	CEmrItemAdvImageRenderState m_lastImageRenderState;
	CString m_lastImageRenderDataOutput;

	// (a.walling 2013-06-27 13:15) - PLID 57348 - NxImageLib - Detail itself stores cached image references for data output
	NxImageLib::ISourceImagePtr m_pSourceImage;
	NxImageLib::ICachedImagePtr m_pCachedImage;

	// (c.haag 2008-06-05 13:26) - PLID 27240 - Variables used to track pre-UpdateInfo information
	BOOL m_bDetailWasBroughtUpToDate;
	CString m_strPreUpdatedDetailName;
	CString m_strPreUpdatedDetailData;

	// A single variable that holds the entire "current state" for this detail.  Eventually the contents 
	// of this variant are pulled out (and parsed if necessary) and written out to the database.
protected:
	_variant_t m_varState;

	BOOL m_bLoadedDetailImageStampsFromVariant; // (z.manning 2010-09-20 18:18) - PLID 40585
public:
	// (c.haag 2007-07-16 08:35) - PLID 26651 - Made this const
	_variant_t GetState() const;
	// (c.haag 2007-05-07 16:24) - PLID 26046 - Returns the variant type of the state
	VARTYPE GetStateVarType() { return m_varState.vt; }
	void SetState(_variant_t varNewState);

	// This is the advanced clean-interface view of this detail.
	CEmrItemAdvDlg *m_pEmrItemAdvDlg;

	//TES 9/16/2009 - PLID 35529 - We can now be set to "windowless" in cases, such as the EMR Analysis, where we are
	// being loaded from outside an EmrTreeWnd.
	void SetWindowless();

	// (z.manning 2011-09-23 14:44) - PLID 42648
	short GetItemAdvDlgScalingFactor() { return m_nItemAdvDlgScalingFactor; }
	void SetItemAdvDlgScalingFactor(const short nItemAdvDlgScalingFactor) { m_nItemAdvDlgScalingFactor = nItemAdvDlgScalingFactor; }

protected:
	bool m_bWindowless;

	// (z.manning 2011-09-23 14:43) - PLID 42648 - Added a variable to scale this detail's dialog when first loading it.
	short m_nItemAdvDlgScalingFactor;
public:

	//TES 3/11/2005 - Remember the parent for our AdvDlg.

	// (a.walling 2012-06-22 14:01) - PLID 51150 - parent is always the topic's window
	//CWnd *m_pParent;

	//TES 2/3/2006 - The SpawnedGroupID used to be its own field.  Now, however, it is the SourceActionID, but only if
	//the SourceActionID is for an action that is of desttype eaoMintItems.  That is because those are the only things
	//that we treat as a "spawned group"
	long GetSpawnedGroupID();
	//TES 3/18/2010 - PLID 37530 - Added, basically the same as GetSpawnedGroupID(), except instead of getting the action ID, it gets the index
	// of the spawning stamp within its detail, as that is needed to calculate the "name" of the spawning item.
	long GetSpawnedGroupStampIndex();
	// (a.walling 2010-04-05 13:16) - PLID 38060 - Gets the SpawnedGroup SourceActionInfo
	SourceActionInfo GetSpawnedGroupSourceActionInfo();

	long GetSourceActionID() const { return m_sai.nSourceActionID; }
	//DRT 8/2/2007 - PLID 26919 - We need to track the EMRActionsT.SourceID field
	long GetSourceActionSourceID()	const	{	return m_nSourceActionSourceID;	}
	//DRT 8/14/2007 - PLID 27067 - We need to track the EMRDataT.EMRDataGroupID field
	long GetSourceActionSourceDataGroupID()	const	{	return m_nSourceActionSourceDataGroupID;	}
	// (z.manning, 01/23/2008) - PLID 28690 - Also need to track hot spot group ID field
	long GetSourceActionSourceHotSpotGroupID() const { return m_nSourceActionSourceHotSpotGroupID; }
	// (z.manning 2009-02-13 09:21) - PLID 33070
	long GetSourceActionSourceTableDropdownGroupID() const { return m_nSourceActionSourceTableDropdownGroupID; }
	// (z.manning 2009-03-11 10:53) - PLID 33338
	// (z.manning 2010-02-24 16:47) - PLID 37532 - The group ID is within the TableRow object in SourceActionInfo
	long GetSourceDataGroupID() { return m_sai.GetDataGroupID(); }
	// (z.manning 2010-03-02 15:05) - PLID 37571
	TableRow* GetSourceTableRow() { return m_sai.GetTableRow(); }

	// (j.jones 2007-01-10 14:53) - PLID 24027 - supported SourceDetailID
	long GetSourceDetailID() const { return m_sai.nSourceDetailID; }
	CEMNDetail* GetSourceDetail() { return m_sai.pSourceDetail; }
	void SetSourceDetailID(long nSourceDetailID);
	void SetSourceDetail(CEMNDetail *pSourceDetail);

	// (z.manning 2009-03-23 15:55) - PLID 33089 - Added an accessor for the source action info
	SourceActionInfo GetSourceActionInfo() const { return m_sai; }
	//TES 3/15/2010 - PLID 37530 - Added
	inline void SetSourceActionInfo(const SourceActionInfo &sai) {m_sai = sai;}

	CEMNDetail* GetCopiedFromDetail() { return m_pCopiedFromDetail; }

	// (c.haag 2006-03-06 11:45) - PLID 19574 - This returns the name of the "spawned group"
	CString GetSpawnedGroupIDName();
	
	//For list items, fills the array with all currentlys selected EmrDataT.IDs.
	void GetSelectedValues(OUT CDWordArray &arSelectedDataIDs) const;

	// (z.manning 2011-11-16 16:10) - PLID 38130 - Function to call all of the below functions since we call them all in multiple places
	void CollectAndProcessActionsPostLoad(BOOL bIsInitialLoad, CEMNLoader* pEMNLoader = NULL, ADODB::_Connection *lpCon = NULL);
	// (z.manning 2011-11-16 15:51) - PLID 38130 - Created these functions to put some common code in
	void CollectAndProcessEmrDataActions(BOOL bIsInitialLoad, CEMNLoader* pEMNLoader);
	void CollectAndProcessImageHotSpotActions(BOOL bIsInitialLoad, CEMNLoader* pEMNLoader);
	// (z.manning 2009-02-16 12:11) - PLID 33072
	void CollectAndProcessTableDropdownActions(BOOL bIsInitialLoad, CEMNLoader* pEMNLoader);
	// (z.manning 2010-03-02 14:32) - PLID 37571
	void CollectAndProcessSmartStampImageActions(BOOL bIsInitialLoad, CEMNLoader* pEMNLoader);

	//Tells this detail that it is new, when it saves it will INSERT rather than UPDATE.
	void SetNew();
	BOOL IsNew(); // (z.manning 2012-07-16 16:40) - PLID 33710

	// (j.jones 2011-04-28 14:39) - PLID 43122 - returns the primary provider ID on the EMN
	// that has FloatEMRData set to true
	long GetProviderIDForFloatingData();

public:
	// Returns the merge field name for this object. If m_strMergeFieldOverride
	// has a non-zero length, this will return m_strMergeFieldOverride. Otherwise,
	// this will return the label text.
	CString GetMergeFieldName(BOOL bConvertToHeaderName) const;

public:
	// (c.haag 2007-01-23 11:08) - PLID 24376 - Returns TRUE if this detail is founded
	// on the internal Current Medications table item
	BOOL IsCurrentMedicationsTable() const;

	// (c.haag 2007-04-02 15:41) - PLID 25465 - Returns TRUE if this detail is founded
	// on the internal Allergies table item
	BOOL IsAllergiesTable() const;

	// (j.jones 2010-06-21 11:43) - PLID 37981 - added IsGenericTable
	BOOL IsGenericTable() const;

public:
	// This is for being able to reference the remainder of the item's EMN
	CEMRTopic *m_pParentTopic;
	// (j.jones 2006-02-20 15:09) - track the original topic, so if we swap topics, we can handle saving both
	CEMRTopic *m_pOriginalParentTopic;
	CString m_strLastSavedParentTopicName;
	//TES 2/3/2006 - Sometimes we are given a topic, but other times we create it ourself.  This tells us whether we "own" m_pParentTopic.
	BOOL m_bOwnTopic;

public:
	BOOL m_bNeedReposition;

	//Call this if this detail needs to reload its content when it next gets a chance.
	// (j.jones 2011-07-22 17:26) - PLID 43504 - added optional connection pointer
	void SetNeedContentReload(OPTIONAL IN ADODB::_Connection *lpCon = NULL);
	void SetNeedContentReload(BOOL bNeedsReload, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	BOOL GetNeedContentReload() const;

	// (j.jones 2007-07-25 17:32) - PLID 26810 - added m_bIsForcedReload 
	BOOL GetIsForcedReload() const;

protected:
	// (c.haag 2006-03-31 11:34) - PLID 19387 - See below
	BOOL m_bNeedToSyncContentAndState;

public:
	// (c.haag 2006-03-31 11:34) - PLID 19387 - Call this if you want LoadContent to
	// call SyncContentAndState at the end.
	void SetNeedSyncContentAndState();

	//Old emr only.
	// (c.haag 2004-07-02 10:16) - Set to true if the merge info icon button needs
	// to be refreshed
	BOOL m_bNeedRefreshMergeButton;

public:
	BOOL RequestStateChange(const _variant_t &varNewState);
	// (z.manning 2009-02-16 17:46) - PLID 33072 - Added an overload with a paramater for the old state
	// since tables states have already been recreated by the time this is called
	BOOL RequestStateChange(const _variant_t &varNewState, const _variant_t &varOldState);
	// (z.manning 2011-01-28 11:25) - PLID 42335 - Moved the image state change logic to its own function
	void HandleImageStateChange(const _variant_t &varNewState, const _variant_t &varOldState, BOOL bFullStateChange);
	// (z.manning 2011-02-23 12:31) - PLID 42549 - Added param for the table's remembered image state
	void HandleImageStateChange(const _variant_t &varNewState, const _variant_t &varOldState, BOOL bFullStateChange, const _variant_t &varRememberedTableState, OUT CArray<TableRow*,TableRow*> *parypNewTableRows);
	BOOL RequestContentChange(int nChangeType, int nDataID = -1, CString strOldName = "", int nNewPos = -1);
	// (a.walling 2012-04-02 08:29) - PLID 49304 - Removed a lot of dead code regarding moving items
	BOOL RequestRemove();

	// (z.manning 2012-03-27 17:38) - PLID 33710 - Can be set if we need to access the table wnd from within this class,
	// such as from the popup.
	CEmrItemAdvTableBase* m_pAdvTableBaseOverride;

	// (z.manning 2012-03-27 17:47) - PLID 33710
	void UpdateTableCalculatedFields();

	//Popup this item to be filled out immediately by the user.
	//TES 1/29/2008 - PLID 24157 - Added an optional parameter, to designate the parent of the popup dialog.
	// (c.haag 2008-11-12 11:37) - PLID 31693 - We now return whether the item was popped up, and
	// the result of the popup (IDOK or IDCANCEL).
	BOOL Popup(CWnd* pOverrideParent = NULL, INT_PTR* pnResult = NULL);

	// (z.manning 2010-02-26 18:10) - PLID 37230 - Tells us if the detail is currently popped up or not
	BOOL m_bIsPoppedUp;

protected:
	//TES 4/5/2007 - PLID 25456 - While the dialog is popped up, keep a pointer to it, we may want to access it in response
	// to various things (mainly a narrative field changing).
	CEMRItemAdvPopupDlg *m_pPopupDlg;

public:
	CEmrItemAdvDlg *CreateEmrItemAdvDlg();

public:
	// (c.haag 2007-10-04 15:27) - PLID 26858 - We may need to get this when updating
	// narrative pop-ups
	inline CEMRItemAdvPopupDlg* GetPopupDlg() { return m_pPopupDlg; }

public:
	// (b.cardillo 2012-03-28 21:54) - PLID 42207 - (additional) Put some common logic into its own function
	// This is a separate static function that checks whether the given narrative state header indicates that that narrative 
	// is "set" or not
	//TES 7/7/2012 - PLID 50854 - This now takes the full narrative text, not just the header, and works for RTF or HTML
	static BOOL IsStateSet_Narrative(const CString &strNarrativeText);
	// (z.manning 2011-10-07 10:47) - PLID 45664 - Added bForOutput
	BOOL IsStateSet(const _variant_t *pOverrideState = NULL, bool bForOutput = false);
	BOOL IsModified() const;
	//Tell this item that it's been saved.
	void SetSaved(BOOL bIsPostLoad = FALSE);
	// (j.jones 2010-03-02 10:19) - PLID 37318 - added parameter to force unsaved
	// after the load completes, because loading completion will call SetSaved()
	void SetUnsaved(BOOL bStayUnsavedPostLoad = FALSE);

public:
	void SetAllowEdit(BOOL bNewEditState);
	BOOL GetAllowEdit() const;

	//TES 1/31/2007 - PLID 24492 - Note to developers: any time that you call SetMergeNameConflict(TRUE) on a detail,
	// you should afterwards call CEMN::TryToOverrideMergeField(), unless the field already has an override.  The detail
	// doesn't call that itself, because TryToOverrideMergeField() may call SetMergeNameConflict(FALSE) on other details,
	// so if you are updating multiple details to conflict, you should call SetMergeNameConflict(TRUE) on all of them before
	// calling TryToOverrideMergeField().
	void SetMergeNameConflict(BOOL bConflicts);
	BOOL GetMergeNameConflict() {return m_bMergeNameConflicts;}

	//TES 12/8/2006 - PLID 23790 -Is this item the active EmrInfoT record for its EmrInfoMasterT? (Always TRUE for Template details).
	BOOL IsActiveInfo();
	// (j.jones 2007-08-29 17:09) - PLID 27057 - need to know if the detail even knows if it is the active info
	BOOL IsActiveInfoValid();
	// (j.jones 2007-08-29 17:11) - PLID 27057 - added so we can update the m_bIsActiveInfo value externally
	void SetIsActiveInfo(BOOL bIsActive);
	//TES 12/8/2006 - PLID 23790 -Clears the cached value.
	void RefreshIsActiveInfo();
protected:
	BOOL m_bAllowEdit;
	BOOL m_bModified;
	// (j.jones 2010-03-02 10:19) - PLID 37318 - added ability to force the detail
	// to remain unsaved after the load completes
	BOOL m_bStayUnsavedPostLoad;

	CString m_strLabelText;

	EmrActionObject m_SourceActionDestType;
	//DRT 8/2/2007 - PLID 26919 - We need to track the EMRActionsT.SourceID field.  This field is not saved, it's just for use during program execution.
	long m_nSourceActionSourceID;
	//DRT 8/14/2007 - PLID 27067 - This is EMRDataT.EMRDataGroupID, as joined to the EMRActionsT.SourceID.  This field is not saved, it's just used for use
	//	during program execution.
	long m_nSourceActionSourceDataGroupID;
	// (z.manning, 01/23/2008) - PLID 28690 - Similarly, we now track the hot spot group ID for the source action
	long m_nSourceActionSourceHotSpotGroupID;
	long m_nSourceActionSourceTableDropdownGroupID; // (z.manning 2009-02-13 09:22) - PLID 33070

	// (j.jones 2007-01-10 14:53) - PLID 24027 - supported SourceDetailID
	// (z.manning 2009-03-11 09:36) - PLID 33338 - Replaced source action ID, source detail ID and 
	// source detail pointer with the new source action info class
	SourceActionInfo m_sai;

	// (j.armen 2014-07-22 09:13) - PLID 62836 - Default Pen Color
	boost::optional<long> m_nDefaultPenColor;
	// (j.armen 2014-07-22 09:13) - PLID 62836 - Default Pen Size
	boost::optional<byte> m_nDefaultPenSizePercent;
public:
	// (j.armen 2014-07-23 14:57) - PLID 62837
	inline const boost::optional<long> GetDefaultPenColor() const { return m_nDefaultPenColor; }
	inline const boost::optional<byte> GetDefaultPenSizePercent() const { return m_nDefaultPenSizePercent; }

protected:
	CEMNDetail *m_pCopiedFromDetail;

	//Do we conflict with another item, merge-wise?
	BOOL m_bMergeNameConflicts;

	//TES 12/8/2006 - PLID 23790 - Are we the most up-to-date version of this master item?
	BOOL m_bIsActiveInfo;
	//And is that value (which is cached) valid?
	BOOL m_bIsActiveInfoValid;

	// (j.jones 2007-07-18 10:20) - PLID 26730 - tracks whether or not the info item has Info actions,
	// which it usually does not, such that we don't have to search for them later
	EMNDetailHasInfoActionsStatus m_eHasInfoActions;

	//TES 7/26/2007 - PLID 25091 - Remember what details we're linked to.
	// (c.haag 2008-11-25 09:40) - PLID 31693 - We now store detail pointers and their positions in
	// narrative states for narratives.
	CArray<LinkedDetailStruct,LinkedDetailStruct&> m_arLinkedDetails;
	bool m_bLinkedDetailsCached;

public:
	// (c.haag 2007-10-25 11:30) - PLID 26858 - This sets m_bLinkedDetailsCached
	// to false. The first successive call to GetLinkedDetails will calculate the
	// actual linked details and refresh the cache
	inline void InvalidateLinkedDetailCache() { m_bLinkedDetailsCached = false; }
	
	// (c.haag 2007-10-30 17:30) - PLID 27914 - If pDetail exists in the linked
	// detail cache, then it will be invalidated. Also optionally throws an assertion
	// if the detail is found.
	void InvalidateLinkedDetailCacheIfExists(CEMNDetail* pDetail, BOOL bAssertIfFound);

	// (c.haag 2007-10-30 17:34) - PLID 27914 - Compares the linked detail cache with
	// a calculated list of linked details. Returns TRUE if they match, FALSE if they
	// do not.
	void AssertLinkedDetailCacheUpToDate();

protected:
	// A local utility function specifically designed to make emr details look decent within the emr tab window.
	static BOOL IsRectBetterFit(const CRect &rcCompare, const BOOL bCompareIsPreferredSize, const CRect &rcCompareBounding, const CRect &rcCompareTo, const BOOL bCompareToIsPreferredSize, const CRect &rcCompareBoundingTo, const CRgn &rgnToFitInto, const CRgn &rgnToFitIntoInverse, IN const CRect &rcArea);

public:
	//Functions for accessing/updating table elements
	// (j.jones 2007-08-27 11:16) - PLID 27056 - added E/M coding data
	// (z.manning 2008-06-06 10:12) - PLID 30155 - Added formula and decimal places
	// (z.manning 2009-02-24 12:46) - PLID 33141 - Added group ID
	// (z.manning 2010-02-16 11:22) - PLID 37230 - This now returns an int for the index of the new row
	// (z.manning 2010-02-18 10:47) - PLID 37427 - Row ID is now a struct
	// (z.manning 2010-04-13 15:15) - PLID 38175 - Added IsLabel
	// (j.jones 2011-03-09 09:05) - PLID 42283 - added nEMCodeCategoryID
	//TES 3/17/2011 - PLID 41108 - Added godtGlassesOrderDataType and nGlassesOrderDataID
	// (z.manning 2011-05-26 14:45) - PLID 43865 - Added flags
	// (c.haag 2011-05-31) - PLID 43875 - We can now specify where the row goes. nPlacementIndex should be -1 unless placement is aroInsertAt, in which
	// case the row will be inserted at nPlacementIndex.
	// (a.walling 2011-08-11 15:49) - PLID 44987 - Returns a TableRow* now; also takes the table row ID as a const reference
	// (e.lally 2011-12-08) PLID 46471 - added IsCurrentMedOrAllergy. It flags the new row as belonging to a current medication or allergies table
	// (a.walling 2013-03-21 09:49) - PLID 55804 - EM coding stuff unused
	TableRow* AddRow(const TableRowID& rowid, const CString &strName, const CString strFormula, const BYTE nDecimalPlaces, const long nGroupID, BOOL bIsLabel, GlassesOrderDataType godtGlassesOrderDataType, long nGlassesOrderDataID, const long nFlags, EAddRowPlacement placement, long nPlacementIndex, BOOL IsCurrentMedOrAllergy);
	// (z.manning 2010-02-16 14:19) - PLID 37230 - Function to add a new row from a smart stamp
	//TES 3/17/2010 - PLID 37530 - Added a parameter to pass in this stamp's index within the detail (i.e., is this the first or second AK).
	int AddSmartStampRow(EmrDetailImageStamp *pDetailImageStamp, long nStampIndexInDetailByType);
	int GetRowCount() const;
	// (z.manning 2010-02-19 11:17) - PLID 37427 - You must use the pointer to reference table rows now
	//TableRow GetRow(int nIndex) const;
	// (c.haag 2007-08-20 12:49) - PLID 27121 - Returns a row pointer by index
	TableRow *GetRowPtr(int nIndex) const;
	// (z.manning 2010-02-18 10:54) - PLID 37427 - Row ID is now a struct
	TableRow *GetRowByID(TableRowID *pRowID) const;
	// (z.manning 2010-02-19 17:53) - PLID 37230 - Returns the row corresponding to the give stamp ID
	TableRow *GetRowByStampID(const long nStampID);
	TableRow *GetRowByDetailStamp(EmrDetailImageStamp *pDetailStamp);
	// (z.manning 2010-02-24 10:55) - PLID 37225 - Function to remove a row
	void RemoveTableRow(TableRow *ptr);

	//DRT 7/10/2007 - PLID 24105 - Removed width, it is tracked outside the columns now.
	// (j.jones 2007-08-27 11:16) - PLID 27056 - added E/M coding data
	// (z.manning, 05/23/2008) - PLID 30155 - Added Formula
	// (z.manning 2009-01-15 15:21) - PLID 32724 - Added InputMask
	// (z.manning 2010-02-16 14:30) - PLID 37230 - Added sub type
	// (c.haag 2010-02-24 15:33) - PLID 21301 - AutoAlphabetizeDropdown
	// (z.manning 2010-04-13 15:15) - PLID 38175 - Added IsLabel
	// (z.manning 2010-07-29 15:04) - PLID 36150 - Added sentence format
	// (j.jones 2010-08-11 15:52) - PLID 39496 - added AutoNumberType and AutoNumberPrefix
	// (j.jones 2011-03-09 09:05) - PLID 42283 - added nEMCodeCategoryID
	// (z.manning 2011-03-11) - PLID 42778 - Added bHasDropdownElements
	//TES 3/17/2011 - PLID 41108 - Added godtGlassesOrderDataType and nGlassesOrderDataID
	// (z.manning 2011-03-21 11:19) - PLID 30608 - Added auto-fill type
	// (z.manning 2011-05-26 14:46) - PLID 43865 - Added flags
	// (z.manning 2011-09-19 14:50) - PLID 41954 - Added dropdown separators
	// (z.manning 2011-11-07 10:44) - PLID 46309 - Added SpawnedItemsSeparator
	// (e.lally 2011-12-08) PLID 46471 - Added bIsCurrentMedOrAllergyUsageCol. It flags this column as being the official usage (checkbox) column for current medication and allergy details
	// (r.gonet 08/03/2012) - PLID 51948 - Added WoundCareDataType
	void AddColumn(long nID, const CString &strName, long nType, BOOL bIsGrouped, CString strFormula, BYTE nDecimalPlaces,
		CString strInputMask, BYTE nSubType, BOOL bAutoAlphabetizeDropdown, BOOL bIsLabel, CString strLongForm,
		EEmrTableAutoNumberType etantAutoNumberType, CString strAutoNumberPrefix, BOOL bHasDropdownElements, BOOL bHasActiveDropdownElements,
		GlassesOrderDataType godtGlassesOrderDataType, long nGlassesOrderDataID, BYTE nAutofillType, const long nFlags,
		LPCTSTR strDropdownSeparator, LPCTSTR strDropdownSeparatorFinal, LPCTSTR strSpawnedItemsSeparator, BOOL bIsCurrentMedOrAllergyUsageCol,
		EWoundCareDataType ewccWoundCareDataType);

	int GetColumnCount() const;
	TableColumn GetColumn(int nIndex) const;
	// (c.haag 2007-08-20 12:49) - PLID 27121 - Returns a column pointer by index
	TableColumn *GetColumnPtr(int nIndex) const;
	TableColumn *GetColumnByID(long nID) const;
	// (z.manning 2010-02-16 14:52) - PLID 37230 - Gets a column based on subtype
	TableColumn* GetColumnByListSubType(const EEmrDataElementListSubType eSubType);
	// (z.manning 2011-10-11 08:59) - PLID 42061 - The column sql functions now use CSqlFragment
	CSqlFragment GetColumnSql(long nColumnID) const;
	void SetColumnSql(long nColumnID, CSqlFragment &sql);
	void CalcComboSql(); // (z.manning, 04/10/2007) - PLID 25560

	//TES 3/1/2006 - If nRowID or nColumnID is invalid (maybe they edited the table), will return FALSE.
	// (c.haag 2007-08-20 09:24) - PLID 27118 - Made it const
	// (z.manning 2010-02-18 10:43) - PLID 37427 - Row ID is now a struct
	BOOL GetTableElement(TableRowID *pRowID, long nColumnID, OUT TableElement &te);
	// (a.walling 2011-08-24 12:35) - PLID 45171 - Unlike GetTableElement, this only gets existing elements
	BOOL GetExistingTableElement(TableRowID *pRowID, long nColumnID, OUT TableElement &te);
	// (c.haag 2009-03-05 17:05) - PLID 33367 - Added first two optional parameters
	// (z.manning 2011-05-31 14:50) - PLID 42131 - This now returns true if something actually changed.
	BOOL SetTableElement(TableElement te, BOOL bSearchForExisting = TRUE, BOOL bRecreateStateFromContent = TRUE);

	// (c.haag 2007-01-30 13:51) - PLID 24485 - These functions should only be called in rare circumstances
	// where you intend to do optimal table detail traversals
	int GetTableElementCount() const;
	BOOL GetTableElementByIndex(long nIndex, OUT TableElement &te) const;
	// (c.haag 2007-08-20 11:29) - PLID 27121 - Returns an actual pointer to a table element
	TableElement* GetTableElementPtrByIndex(long nIndex);
	// (z.manning 2010-02-16 15:34) - PLID 37230
	TableElement* GetTableElementByRowColPtr(TableRow *ptr, TableColumn *ptc);
	// (z.manning 2010-12-22 12:54) - PLID 41887
	void GetTableElementsByRow(TableRow *ptr, OUT CArray<TableElement,TableElement&> &aryTableElements);

	//Update any table elements linked to this detail.
	void UpdateTable(CEMNDetail *pDetail, BOOL bRemovingDetail);

	// (z.manning 2009-08-04) - PLID 31910 - Will iterate through the table state and remove any elements
	// where either the row or column ID matches nDataID
	void ClearTableStateByDataID(const long nDataID);

	//DRT 7/11/2007 - PLID 24105
	CTableColumnWidths* GetColumnWidths()	{	return &m_tcwWidths;	}

	//Functions for accessing list elements

	//Adds the given list element, if it's already been added, no effect.
	//TES 2/9/2007 - PLID 24671 - Added the DataGroupID
	//TES 3/17/2011 - PLID 41108 - Added godtGlassesOrderDataType and nGlassesOrderDataID
	// (j.jones 2011-04-28 14:39) - PLID 43122 - added bIsFloated
	// (z.manning 2011-11-07 17:01) - PLID 46309 - Added SpawnedItemsSeparator
	// (a.walling 2012-10-12 15:05) - PLID 53165 - ParentLabelID
	void AddListElement(long nID, long nDataGroupID, const CString &strName, BOOL bIsLabel, const CString &strLongForm, long nActionsType, 
		GlassesOrderDataType godtGlassesOrderDataType, long nGlassesOrderDataID, const BOOL bIsFloated, LPCTSTR strSpawnedItemsSeparator,
		long nParentLabelID);

	//Used when batch-adding list elements, will not refresh the screen between calls to BeginAddListElements and EndAddListElements().
	void BeginAddListElements();
	void EndAddListElements();
		
	CArray<ListElement,ListElement&>& GetListElements();
	int GetListElementCount();
	ListElement GetListElement(int nIndex);
	void SetListElement(int nIndex, ListElement &le);
	CString GetListElementIDsCommaDelimited();
	void RemoveListElement(long nID);
	ListElement* GetListElementByID(const long nListElementID); // (z.manning 2011-11-04 17:53) - PLID 42765

	//Functions for accessing image properties.
	DWORD GetImageStateVersion();
	eImageType GetImageType() const;
	//If bFullPath will calculate the actual path based on the image type; otherwise, will just return the raw path.
	CString GetImagePath(bool bFullPath);
	_variant_t GetInkData();
	void SetInkData(const _variant_t &varData);
	_variant_t GetImageTextData();
	// (r.gonet 05/27/2011) - PLID 30359 - New param to perform a state change or not.
	void SetImageTextData(const _variant_t &varData, bool bChangeState = true);
	// (r.gonet 02/13/2012) - PLID 37682 - Sets the current filter on the image stamps.
	void SetImageTextStringFilter(CTextStringFilterPtr pTextStringFilter);
	// (r.gonet 02/13/2012) - PLID 37682 - Gets the current filter on the image stamps.
	CTextStringFilterPtr GetImageTextStringFilter();
	// (r.gonet 02/13/2012) - PLID 37682 - Reflects the current filter onto the image. 
	void ReflectImageTextStringFilter();
	// (r.gonet 02/14/2012) - PLID 37682 - Opens the filter editor and sets the new filter up.
	BOOL EditImageStampFilter(CWnd *pParent);
	void SetImage(const CString &strPath, eImageType nType);
	void SetImagePrintData(const _variant_t &varData); // (z.manning 2011-10-05 15:51) - PLID 45842
	_variant_t GetImagePrintData(); // (z.manning 2011-10-05 17:16) - PLID 45842
	CEMRHotSpotArray* GetHotSpotArray();
	// (z.manning, 01/22/2008) - PLID 28690 - Get the list of selected hot spot IDs
	// (c.haag 2008-11-26 16:51) - PLID 32267 - We now support optional loader and connection pointers
	void GetImageSelectedHotSpotIDs(CArray<long,long> &arynHotSpotIDs, CEMNLoader* pEMNLoader = NULL,
			ADODB::_Connection *lpCon = NULL);
	//TES 2/15/2010 - PLID 37375 - Added a version that fills the array with CEMRHotSpots, instead of just the IDs.
	void GetImageSelectedHotSpots(CEMRHotSpotArray &arynHotSpots, CEMNLoader* pEMNLoader = NULL,
							 ADODB::_Connection *lpCon = NULL);
	//TES 2/15/2010 - PLID 37375 - For a given HotSpotID, looks up the corresponding CEMRHotSpot on this detail.
	CEMRHotSpot* GetHotSpotFromID(IN long nID);
	// (z.manning 2010-03-03 08:44) - PLID 37493 - Gets a hot spot for a given point
	// (z.manning 2011-09-09 11:20) - PLID 45335 - Renamed and reworked this function to also support 3D models.
	CEMRHotSpot* GetHotSpotFromDetailStamp(EmrDetailImageStamp *pDetailStamp);

	// (j.armen 2014-07-22 08:58) - PLID 62836 - Updates the state when the Pen Color or Size changes
	void OnCurrentPenColorChanged(long nPenColor);
	void OnCurrentPenSizeChanged(byte nPenSizePercent);

	// (z.manning, 01/22/2008) - PLID 28690 - Selects/unslects the hot spot for the given ID.
	void ToggleImageHotSpotSelection(long nHotSpotID);
	// (z.manning, 01/22/2008) - PLID 28690 - Returns true if the specified hot spot is selected and false if it's not.
	BOOL IsHotSpotSelected(long nHotSpotID);

	// (z.manning 2010-02-17 17:00) - PLID 37412
	// (z.manning 2011-09-08 15:24) - PLID 45335 - Added 3D fields
	EmrDetailImageStamp* AddNewDetailImageStamp(const long nDetailImageStampID, TextString *pts, const long nOrderIndex, const BYTE nSmartStampRule);
	EmrDetailImageStamp* AddNewDetailImageStamp(const EmrDetailImageStamp source);
	int GetImageStampCount() const;
	EmrDetailImageStamp* GetImageStampByIndex(const int nIndex) const;
	// (z.manning 2010-03-17 14:15) - PLID 37439
	EmrDetailImageStamp* GetDetailImageStampByID(const long nDetailStampID);
	// (z.manning 2011-01-27 15:52) - PLID 42335
	CEmrDetailImageStampArray* GetDetailImageStamps();

	// (z.manning 2010-03-02 12:59) - PLID 37571 - Updates all rows looking for a detail stamp pointer with
	// the given detail stamp ID
	void UpdateDetailStampIDByPointer(const long nDetailStampPointer, const long nDetailStampID);

	// (z.manning 2008-11-03 09:56) - PLID 31890 - Returns true if the detail is a built in signature
	// item meaning they right clicked to insert the signature (does not include signature info items).
	BOOL IsSignatureDetail();

	CEmrItemStampExclusions* GetStampExclusions(); // (z.manning 2011-10-25 12:55) - PLID 39401
	void ReloadStampExclusions(); // (z.manning 2011-10-25 15:05) - PLID 39401

	// (a.wetta 2007-02-23 14:49) - PLID 24511 - Add functions to access the m_bSizeImageToDetail member variable
	// (a.walling 2011-05-19 18:07) - PLID 43843 - Note this functionality has been broken for a while now
	// (a.walling 2011-05-25 17:57) - PLID 43847 - This is now no longer a property of the detail.
	/*void SetSizeImageToDetail(BOOL bSizeImageToDetail);
	BOOL GetSizeImageToDetail();*/

	/* (r.gonet 05/31/2011) - PLID 43896 - Put back in when we want to save and restore the zoom and pan offsets.
	// (r.gonet 05/31/2011) - PLID 43896 - Sets the current zoom level of the image
	void SetZoomLevel(double dZoomLevel);
	double GetZoomLevel();

	// (r.gonet 05/31/2011) - PLID 43896 - Sets the left viewport offset
	void SetOffsetX(long nOffsetX);
	long GetOffsetX();

	// (r.gonet 05/31/2011) - PLID 43896 - Sets the top viewport offset
	void SetOffsetY(long nOffsetY);
	long GetOffsetY();
	*/

	//for auditing
	void SetInkAdded();
	void SetInkErased();
	void SetImageTextAdded();
	void SetImageTextRemoved();
	// (r.gonet 05/02/2012) - PLID 49946 - Added
	void SetImageStampModified();

	double GetSliderMin();
	double GetSliderMax();
	double GetSliderInc();

	CString GetMergeFieldOverride() const;
	void SetMergeFieldOverride(CString strMergeFieldOverride);

	// (j.jones 2007-07-18 10:20) - PLID 26730 - returns whether or not the info item has Info actions,
	// which it usually does not, such that we don't have to search for them later
	EMNDetailHasInfoActionsStatus GetHasInfoActionsStatus();

	// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
	// because they are now only calculated in the API, and not in Practice code
	/*
	// (j.jones 2007-08-27 12:16) - PLID 27056 - added E/M element calculation functions
	BOOL GetUseEMCoding();
	long GetEMCategoryID();
	// (j.jones 2011-03-09 09:05) - PLID 42283 - added GetEMCodeUseTableCategories
	EMCodeUseTableCategories GetEMCodeUseTableCategories();
	*/
	
	// (j.jones 2011-03-09 09:05) - PLID 42283 - this function now updates an existing array of tracked categories
	// (j.jones 2013-02-13 14:46) - PLID 54668 - this is obsolete now, it's in the API
	//void CalculateEMElements(CArray<ChecklistTrackedCategoryInfo*, ChecklistTrackedCategoryInfo*> &aryTrackedCategories);
	// (j.jones 2007-08-27 16:13) - PLID 27056 - added E/M element calculation functions
	// (j.jones 2011-03-09 09:05) - PLID 42283 - this function updates to an existing array of tracked categories
	// (j.jones 2013-02-13 14:46) - PLID 54668 - this is obsolete now, it's in the API
	//void CalculateEMTableElements(CArray<ChecklistTrackedCategoryInfo*, ChecklistTrackedCategoryInfo*> &aryTrackedCategories);
	
	//TES 3/17/2010 - PLID 37530 - Function that returns the index of the given stamp within this detail, based on the stamp type
	// (i.e., is this the first or second AK).
	long GetStampIndexInDetailByType(IN EmrDetailImageStamp* pDetailStamp);
	
protected:

	// (j.jones 2007-08-27 16:13) - PLID 27056 - added E/M element calculation functions
	// (j.jones 2013-02-13 14:46) - PLID 54668 - this is obsolete now, it's in the API
	//long CalculateEMTableElements();

	//Type specific fields (these used to be in CEmrItemAdv__Dlg.  Now they're here.):	
	//List
	CArray<ListElement,ListElement&> m_arListElements;
	BOOL m_bAddingListElements;

	//Table
	CArray<TableColumn*,TableColumn*> m_arTableColumns; //All the columns in our table.
	CArray<TableRow*,TableRow*> m_arTableRows; //All the rows in our table

	CArray<TableElement,TableElement> m_arTableElements; //All the cells in our table, includes a pointer to a TableColumn and a pointer to a TableRow.

	//TES 3/18/2011 - PLID 41108 - Because selected dropdown IDs are stored in a column-delimited string, we can't easily load their associated
	// GlassesOrderDataIDs in the same query.  So we'll make them available on-demand, this array will store the ones that we've already loaded,
	// it will always be the same size as m_arTableElements
	struct TableElementSelectedDropdownItems {
		bool bAssociatedDataLoaded;
		CArray<long,long> arGlassesOrderDataIDs; //TES 3/18/2011 - PLID 41108 - Same size as anDropdownIDs
		TableElementSelectedDropdownItems() {bAssociatedDataLoaded = false;}
	};
	CArray<TableElementSelectedDropdownItems*,TableElementSelectedDropdownItems*> m_arypTableSelectedDropdowns;

	//TES 3/18/2011 - PLID 41108 - Loads the data for the specific element at the given index.
	void LoadSingleSelectedDropdowns(int nIndex);
public:
	//TES 3/18/2011 - PLID 41108 - Call to load all unloaded data for the detail (if you know you're going to be iterating through all of them).
	void LoadAllSelectedDropdowns();
	//TES 3/18/2011 - PLID 41108 - Gets the GlassesOrderDataIDs for the dropdown items selected at the given element.  Will load from 
	// data if necessary.
	void GetSelectedGlassesOrderDataIDs(TableRow *pTr, TableColumn *pTc, OUT CArray<long,long> &arSelectedIDs);
	
	// (a.walling 2011-05-31 10:20) - PLID 42448 - Gets the (possibly cached) dropdown data for the given IDs and puts them in mapData
	// (j.jones 2013-02-20 15:43) - PLID 55217 - we now pass in the column ID
	// (a.walling 2013-07-02 09:02) - PLID 57407 - CMap's ARG_VALUE should be const CString& instead of LPCTSTR so it can use CString reference counting
	static void GetDropdownData(const CArray<long, long>& arIDsToFind, CMap<long, long, CString, const CString&>& mapData);

	// (j.jones 2011-07-15 13:56) - PLID 42111 - exposed m_strBackgroundImageFilePath
	CString GetBackgroundImageFilePath();

	// (j.jones 2014-01-13 14:35) - PLID 49971 - If the image could not be loaded,
	// track the path we failed to load, such that we won't warn again if we try to load
	// the same invalid path.
	// If this is filled, it does not necessarily mean the path is still invalid.
	CString GetLastWarnedInvalidImageFilePath();
	void SetLastWarnedInvalidImageFilePath(CString strLastWarnedInvalidImageFilePath);

	// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

	// (j.jones 2013-08-07 16:12) - PLID 42958 - If a signature is added by another user,
	// we'll add that information to the detail for auditing purposes.
	// It's cleared after the initial audit is saved.
	// This information is not filled when loading an EMN, they're only for new details.
	void SetSignatureAddedByAnotherUser(bool bAddedByAnotherUser, CString strAddedByUserName);

protected:

	// (z.manning 2011-10-11 09:00) - PLID 42061 - This map now uses sql fragments
	CMap<long,long,CSqlFragment,CSqlFragment&> m_mapColumnKeysToQueries;
	//DRT 7/10/2007 - PLID 24105 - See notes on the class declaration
	CTableColumnWidths m_tcwWidths;

	//Image
	//TES 4/22/2004: The default image.
	CString m_strBackgroundImageFilePath;
	eImageType m_eitBackgroundImageType;

	// (j.armen 2014-07-21 16:32) - PLID 62836 - Forward declare
	scoped_ptr<CEmrItemAdvImageState> m_aisState;

	CEmrDetailImageStampArray m_arypImageStamps; // (z.manning 2010-02-17 11:43) - PLID 37412
	// (z.manning 2010-02-24 11:10) - PLID 37225 - Need to keep track of deleted image stamps
	CArray<long,long> m_arynDeletedImageStampIDs;

	// (a.walling 2009-01-13 13:51) - PLID 32107 - The paths above are not always in sync with the info item; they are the ones being used
	// by this item (for the most part). We want to track what the info item says.
	_variant_t m_varInfoBackgroundImageFilePath;
	_variant_t m_varInfoBackgroundImageType;

	// (a.wetta 2007-02-23 14:50) - PLID 24511 - This variable determines if an image item should size to the detail's full area
	// (a.walling 2011-05-25 17:57) - PLID 43847 - This is now no longer a property of the detail.
	//BOOL m_bSizeImageToDetail;

	//for auditing
	BOOL m_bInkHasBeenAdded;
	BOOL m_bInkHasBeenErased;
	BOOL m_bImageTextHasBeenAdded;
	BOOL m_bImageTextHasBeenRemoved;
	BOOL m_bImageTextHasBeenModified;

	// (j.jones 2014-01-13 14:35) - PLID 49971 - If the image could not be loaded,
	// track the path we failed to load, such that we won't warn again if we try to load
	// the same invalid path.
	// If this is filled, it does not necessarily mean the path is still invalid.
	CString m_strLastWarnedInvalidImageFilePath;

	// (j.jones 2013-08-07 16:12) - PLID 42958 - If a signature is added by another user,
	// these fields store that fact, and their username, until the initial audit is saved.
	// These are not filled when loading an EMN, they're only for new details.
	bool m_bIsSignatureAddedByAnotherUser;
	CString m_strSignatureAddedByAnotherUsername;

	//DRT 1/18/2008 - PLID 28603 - Image HotSpot data
	// (z.manning 2011-07-25 12:05) - PLID 44649 - Changed the type to CEMRHotSpotArray
	// (j.jones 2013-05-16 11:00) - PLID 56596 - changed this into a reference, in order to not require its .h file
	CEMRHotSpotArray &m_aryImageHotSpots;

	// (z.manning 2011-10-25 10:17) - PLID 39401 - Contains the data about any stamps that should be excluded from this detail
	CEmrItemStampExclusions m_StampExclusions;
	bool m_bStampExclusionsLoaded;	// (a.wilson 2013-03-26 11:10) - PLID 55826 - flag for knowing when exlusion stamps have been loaded.

	//Slider
	double m_dSliderMin;
	double m_dSliderMax;
	double m_dSliderInc;

	CString m_strMergeFieldOverride;
public:

	//stored EMRInfoT settings
	CString m_strLongForm;
	long m_nDataFormat;
	CString m_strDataSeparator;
	CString m_strDataSeparatorFinal;
	//TES 2/22/2010 - PLID 37463 - Variables for the Smart Stamps sentence format
	// (z.manning 2010-07-26 13:32) - PLID 39848 - These are no longer separate from the normal table sentence format stuff
	//CString m_strLongFormSmartStamp;
	//BOOL m_bUseLongFormSmartStamp;
	bool m_bDisableTableBorder;

	//TES 2/22/2010 - PLID 37463 - Utility function used by GetTableDataOutputHTMLRaw(), when calculating the Smart Stamps output.
	// (a.walling 2010-03-26 18:00) - PLID 37923 - Take a TableRow* rather than an index
	// (z.manning 2010-07-29 11:57) - PLID 39842 - I renamed this function as it's no longer specific to smart stamps.
	// (z.manning 2010-07-30 10:27) - PLID 39842 - Added bHtml and bIsForPreview
	// (a.walling 2012-07-13 10:30) - PLID 51479 - Added pFallbackParentEMN to the table element data output functions
	CString GetTableElementDataOutput(TableRow* pTr, TableColumn *pTc, CMap<__int64, __int64, int, int> &mapTable, bool bHtml, bool bForPreview, CEMN* pFallbackParentEMN);
	// (z.manning 2010-08-18 11:52) - PLID 39842 - Added an overload that takes a table element
	// (a.walling 2012-07-13 10:30) - PLID 51479 - Added pFallbackParentEMN to the table element data output functions
	CString GetTableElementDataOutput(TableElement *pte, bool bHtml, bool bForPreview, CEMN* pFallbackParentEMN);

	// (a.walling 2008-06-30 12:35) - PLID 29271 - Preview Pane flags
	DWORD m_nPreviewFlags;
	// (a.walling 2009-01-07 15:35) - PLID 31961 - Option to only refresh self
	void SetPreviewFlags(DWORD dwFlags, BOOL bRefreshParent = TRUE, BOOL bOnlyRefreshSelf = FALSE);

	// (j.jones 2013-04-16 10:28) - PLID 56300 - removed E/M coding fields,
	// because they are now only calculated in the API, and not in Practice code
	/*
	// (j.jones 2007-08-27 10:36) - PLID 27056 - added fields for E/M coding
	long m_nEMCodeCategoryID;
	// (j.jones 2011-03-09 09:05) - PLID 42283 - added m_eEMCodeUseTableCategories
	EMCodeUseTableCategories m_eEMCodeUseTableCategories;
	BOOL m_bUseEMCoding;
	EMCodingTypes m_emctEMCodingType;
	*/

public:

	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

	//TES 6/23/2008 - PLID 29416 - For "system" items (Allergies and Current Medications), we need to know if any details
	// have already had their "official" information is loaded, because if so, any future details need to load from the existing
	// detail rather than data.
	inline BOOL GetHasLoadedSystemInfo() {return m_bHasLoadedSystemInfo;}
	inline void SetHasLoadedSystemInfo() {m_bHasLoadedSystemInfo = TRUE;}
protected:
	BOOL m_bHasLoadedSystemInfo;

protected:
	//So we don't keep re-loading the values from data.
	BOOL m_bNeedToLoadContent;

	// (j.jones 2007-07-25 17:27) - PLID 26810 - track whether our m_bNeedToLoadContent
	// is a forced reload, as opposed to a regular chart creation
	BOOL m_bIsForcedReload;

protected:
	// (c.haag 2008-12-04 13:53) - PLID 31693 - Only used for GetEmrListOptionsForLinkedDetails. We need to track
	// not only the option names, but also their ID's.
	struct EmrDataNameAndID
	{
		CString strData;
		long nDataID;
	};
	typedef CArray<EmrDataNameAndID,EmrDataNameAndID&> CEmrDataNameAndIDAry;
	// (c.haag 2008-12-02 09:51) - PLID 32271 - Given an array of details, this function will take all the
	// corresponding EMR list items that do not have their content loaded, and store all of their options
	// in mapEmrDataT. I chose the uncanny function name to minimize the chance of a developer thinking
	// this is used anywhere besides GetLinkedDetails. If this seems confusing or unclear, please see how the map
	// is used in GetLinkedDetails.
	// (c.haag 2008-12-04 16:07) - PLID 31693 - This used to take in a map of CStringArrays. Now it takes in
	// a map of CEmrDataNameAndIDArys because we also need EMR Data ID's.
	void GetEmrListOptionsForLinkedDetails(ADODB::_Connection *lpCon, CEMN* pEMN, CArray<CEMNDetail*,CEMNDetail*>* paEMNDetails,
													   const CStringArray& saMergeFields,
													   CMap<long,long,CEmrDataNameAndIDAry*,CEmrDataNameAndIDAry*>& mapEmrDataT);

public:
	//Used for narrative and table items, to identify which details they are currently (not just potentially) linked to.
	// (c.haag 2007-02-28 13:18) - PLID 24989 - We now support getting linked details for a narrative given an array of EMN details
	// (c.haag 2007-08-09 16:47) - PLID 27038 - We now require an EMN object
	// (c.haag 2007-08-14 17:44) - PLID 27073 - Optional parameter for connection objects
	// (c.haag 2007-10-23 09:44) - PLID 27833 - Narrative fields are related to details by their label text.
	// It's possible for more than one detail to have the same label text on an EMN. If bIncludeNarrativeDuplicates
	// is FALSE, then this function will find the first qualifying detail (this is legacy behavior). If 
	// bIncludeNarrativeDuplicates is TRUE, then we will pull all qualifying details. This is a workaround 
	// for PLID 27833, and may be revisited in PLID 27840.
	// (c.haag 2008-12-04 10:48) - PLID 31693 - This function now returns extended detail information.
	// The original prototype now exists as a wrapper that only returns the detail pointers.
	void GetLinkedDetails(CArray<LinkedDetailStruct,LinkedDetailStruct&> &arDetails, CEMN* pEMN,
		CArray<CEMNDetail*,CEMNDetail*>* paEMNDetails = NULL,
		ADODB::_Connection *lpCon = NULL,
		BOOL bIncludeNarrativeDuplicates = FALSE,
		BOOL bIncludeDetailDuplicates = FALSE);

	// (c.haag 2008-12-04 10:49) - PLID 31693 - This only returns detail pointers instead of extended detail information
	void GetLinkedDetails(CArray<CEMNDetail*,CEMNDetail*> &arDetails, CEMN* pEMN,
		CArray<CEMNDetail*,CEMNDetail*>* paEMNDetails = NULL,
		ADODB::_Connection *lpCon = NULL,
		BOOL bIncludeNarrativeDuplicates = FALSE,
		BOOL bIncludeDetailDuplicates = FALSE);

	// (a.walling 2009-11-19 14:55) - PLID 36369 - Might this narrative contain this detail?
	bool NarrativeMayHaveDetailField(CEMNDetail* pDetail);
	bool NarrativeMayHaveDetailField(const CString& strField, long nSpawnedGroupID = -1);

	// (a.walling 2009-11-17 09:35) - PLID 36365
	void GetNarrativeLWFields(CMap<CString, LPCTSTR, long, long&>& mapFields, const CString& strOverrideRichText = "");

	// (a.walling 2009-11-17 09:35) - PLID 36365
	// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
	void GetValueForNarrative(NarrativeField& nf, CArray<CEMNDetail*,CEMNDetail*>* pallDetails = NULL, bool* pbForceUpdate = NULL);
	// (a.walling 2010-01-22 10:43) - PLID 37027 - Pass in the selected state of the element
	// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
	void GetValueForNarrativeDetail(NarrativeField& nf, CEMNDetail* pDetail, long nElementID = -1, bool bElementSelected = false, bool* pbForceUpdate = NULL);

	// (a.walling 2009-11-17 11:31) - PLID 36365
	void GetSpawningItemValueForNarrative(NarrativeField& nf, CArray<CEMNDetail*,CEMNDetail*>* pallDetails = NULL);

	// (a.walling 2010-04-01 12:13) - PLID 38013 - Gets the data from the item that directly or indirectly spawned this detail
	void GetItemSpawningTextValueForNarrative(NarrativeField& nf, bool bFurthest = false);
	
	// (a.walling 2010-04-01 12:17) - PLID 38013 - Returns the CEMNDetail that directly or indirectly spawned this detail
	// if bFurthest, we will keep traversing until we get to a non-spawned item or topic. Otherwise, it will return the first.
	CEMNDetail* FindSpawningItem(bool bFurthest, SourceActionInfo& sai);
	// (z.manning 2011-11-04 09:08) - PLID 42765 - Added an overload without the source action info param
	CEMNDetail* FindSpawningItem(bool bFurthest);

	// (z.manning 2011-11-30 11:14) - PLID 42765 - Determines if anything in the current detail references the Spawned Items
	// sentence format field.
	BOOL HasSpawnedItemsField();

	// (a.walling 2012-03-14 22:51) - PLID 48896 - Do we spawn this detail eg via a table dropdown item?
	bool FindSpawnedDetail(CEMNDetail* pTarget, bool bOnlyIfSpawnedItemsField);

	// (a.walling 2012-03-14 22:51) - PLID 48896 - Find spawned details eg via a table dropdown item
	// (a.walling 2012-08-16 08:18) - PLID 52163 - This now works as described, gathering the spawned details, taking in the check for whether <spawned items> is in the output as bool param
	void GetSpawnedDetails(CEMNDetailArray& arSpawnedDetails, bool bOnlyIfSpawnedItemsField, CEMNDetail* pStopAt);

	// (a.walling 2009-11-17 09:35) - PLID 36366 - Return the most appropriate detail based on the field name
	// (a.walling 2010-01-22 10:46) - PLID 37027 - Also need to keep track of the list element's selected state
	CEMNDetail* GetDetailFromNarrativeMergeField(const CString& strField, OPTIONAL long* pnElementID = NULL, OPTIONAL bool* pbElementSelected = NULL, CArray<CEMNDetail*,CEMNDetail*>* pallDetails = NULL);
	CEMNDetail* GetDetailFromNarrativeMergeSubfield(const CString& strField, OPTIONAL long* pnElementID = NULL, OPTIONAL bool* pbElementSelected = NULL, CArray<CEMNDetail*,CEMNDetail*>* pallDetails = NULL);

	void GetAllEMNDetails(CArray<CEMNDetail*,CEMNDetail*>& arAllDetails, bool bPrioritizeThisTopic);

	// (a.walling 2009-11-19 14:21) - PLID 36367 - Requesting list of various merge fields from the host
	void PopulateAvailableNarrativeFields(COleSafeArray& saFields, bool bIncludeLetterWriting, bool bIncludeEMR, bool bIncludeListItems);
	
	// (a.walling 2009-11-17 08:35) - PLID 36365
	void UpdateNarrativeFields(OPTIONAL CEMRItemAdvPopupWnd* pPopupWnd = NULL, const CString& strOverrideRichText = "");

	// (z.manning 2011-11-11 09:46) - PLID 37093
	void UpdateNarrativeFieldsWithValues(COleSafeArray *psaFieldValues, CEMRItemAdvPopupWnd* pPopupWnd, bool bForceUpdate);

	// (a.walling 2009-11-17 09:35) - PLID 36365
	// (a.walling 2010-05-27 09:25) - PLID 38910 - Parameter to force update
	void GetNarrativeFieldSet(COleSafeArray& saFieldValues, const CString& strOverrideRichText = "", bool* pbForceUpdate = NULL);

	// (z.manning 2011-11-10 17:31) - PLID 37093
	void GetSortedNarrativeFieldArray(OUT CArray<NarrativeField>& aryNarrativeFields);

	// (a.walling 2009-11-24 16:46) - PLID 36365
	bool IsUpdatingNarrativeFields() {
		return m_bUpdatingNarrativeFields;
	};

	//TES 7/9/2012 - PLID 51359 - Call this function to make the narrative new (HTML narratives store DetailIDs and DataIDs, and have GUIDs that
	// need to be updated).
	void DereferenceNarrative();
protected:
	bool m_bUpdatingNarrativeFields;

	//Used to prevent details from unspawning themselves.
public:
	BOOL GetAllowUnspawn() const;
	void SetAllowUnspawn(BOOL bAllowUnspawn);
protected:
	BOOL m_bAllowUnspawn;

protected:
	COLORREF m_clrHighlight;

	// (z.manning 2008-10-07 15:49) - PLID 31561 - A lab ID may be directly tied to an EMR detail
	_variant_t m_varLabID;

public:
	void ChangeHilightColor(COLORREF clrHighlight);

	// (z.manning 2008-10-07 15:51) - PLID 31561 - Lab ID
	void SetLabID(const long nLabID);
	_variant_t GetLabID();
	BOOL IsLabDetail();
	void OpenLabEntryDialog(); // (z.manning 2009-09-23 09:13) - PLID 33612

public:
	// (c.haag 2006-04-03 10:27) - PLID 19387 - We now persistently track the
	// "Current List" of a detail if a user edited its info item from a topic.
	CArray<long,long> m_CurList_naryDataIDs;
	CStringArray m_CurList_straryData;
	CStringArray m_CurList_straryLabels;
	CStringArray m_CurList_straryLongForms;
	CArray<long,long> m_CurList_naryActionsType;
	CArray<BOOL,BOOL> m_CurList_baryInactive;
	BOOL m_bCheckCurrentList; // This is true if the "Current List" variables must be used in LoadContent

public:

	// (j.jones 2008-07-18 11:06) - PLID 30779 - added problem class, and an array to track problems
	// (c.haag 2009-05-16 12:03) - PLID 34311 - We now track problem links instead of problems.
	CArray<CEmrProblemLink*, CEmrProblemLink*> m_apEmrProblemLinks;

	// (j.jones 2008-07-21 08:22) - PLID 30779 - returns true if there are any undeleted problems on the detail,
	// also can optionally take in an EMRDataID
	BOOL HasProblems(long nEMRDataID = -1);
	// (j.jones 2008-07-21 08:22) - PLID 30779 - returns true if there are only undeleted, closed problems on the detail,
	// also can optionally take in an EMRDataID
	BOOL HasOnlyClosedProblems(long nEMRDataID = -1);

	// (a.walling 2007-04-05 16:56) - PLID 25454
	long GetID();
	// (j.jones 2012-11-15 11:01) - PLID 52819 - added GetTemplateDetailID, only needed on templates
	long GetTemplateDetailID();

	// (a.walling 2007-10-18 15:34) - PLID 27664 - Details never need to ignore unsaved, the last saved
	// state stored in the topic object's cached HTML string.
	// (a.walling 2008-10-23 09:49) - PLID 27552 - if bCheckSpawnedOutput, will return empty if we are a subdetail
	CString GetHTML(BOOL bCheckSpawnedOutput = FALSE); // return this detail's HTML
	// (z.manning 2010-07-30 15:25) - PLID 39842 - Added a function to get non html table output as well
	// (c.haag 2011-04-05) - PLID 43145 - We can now filter on EmrDataID's when building the <table> block. We also
	// take in a table caption tag.
	// (c.haag 2011-04-12) - PLID 43245 - We now discern HTML table captions from non-HTML table captions
	//TES 3/22/2012 - PLID 48203 - Added pFallbackParentEMN as a parameter, it will be used if this function is called for a detail which
	// has not yet been attached to an EMN
	void GetTableDataOutputRaw(OUT CString &strHTML, OUT CString &strNonHTML, TableRowID* pSingleTableRowID, CArray<long,long>* paTableEmrDataIDs,
		LPCTSTR szHTMLCaption, LPCTSTR szNonHTMLCaption, CEMN *pFallbackParentEMN = NULL);
	// (z.manning 2010-08-10 09:24) - PLID 39497 - Added an overload to pass in a custom long form
	// (c.haag 2011-04-05) - PLID 43145 - We can now filter on EmrDataID's when building the <table> block. We also
	// take in a table caption tag.
	// (c.haag 2011-04-12) - PLID 43245 - We now discern HTML table captions from non-HTML table captions
	//TES 3/22/2012 - PLID 48203 - Added pFallbackParentEMN as a parameter, it will be used if this function is called for a detail which
	// has not yet been attached to an EMN
	void GetTableDataOutputRaw(IN CString strLongForm, OUT CString &strHTML, OUT CString &strNonHTML, TableRowID* pSingleTableRowID, CArray<long,long>* paTableEmrDataIDs,
		LPCTSTR szHTMLCaption, LPCTSTR szNonHTMLCaption, CEMN *pFallbackParentEMN = NULL);
	CString GetTableDataOutputHTMLRaw(); // Return well-formatted HTML markup for tables
	CString GetTableDataOutputNonHTMLRaw();

	// (z.manning 2011-09-26 09:38) - PLID 45664
	void Get3DImageOutputData(OUT CImageArray *paryImages);

	CEMNDetail* GetNextDetail(); // returns the next detail, ordered by Y then X

	//if editing a locked item creates a new copy, and we have unsaved items
	//using the old info item, make them use the new info item
	//TES 12/11/2006 - PLID 23790 - If pChangedIDMap is NULL, this function will go ahead and create it, based on the two InfoIDs.
	// Note that it will not create any Action mapping, because this particular function never uses that anyway.
	void UpdateInfoID(long nOldEMRInfoID, long nNewEMRInfoID, OPTIONAL IN EMRInfoChangedIDMap* pChangedIDMap = NULL, long nForceUpdateForDetailID = -1);

	// (j.jones 2008-01-14 10:59) - PLID 18709 - added GetPatientID
	long GetPatientID();

	// (z.manning 2010-02-23 12:29) - PLID 37412
	_variant_t GetDetailImageStampAsVariantArray();

public:
	// (c.haag 2007-08-13 09:47) - PLID 27049 - This function will go through all linked
	// table entries that are missing linked details, and try to fill them in based on what
	// is currently in our EMN. Returns TRUE if one or more table elements were changed.
	BOOL TryRecoverMissingLinkedDetails();

public:
	// (c.haag 2007-08-18 11:56) - PLID 27113 - This function fills several arrays with all
	// information relevant to the Rx column of this Current Medications table.
	// (j.jones 2011-05-04 10:26) - PLID 43527 - added mapDataIDsToSig, which maps a DataID
	// to the Sig entered in the table
	void PopulateCurrentMedicationIDArrays(IN OUT CArray<long,long>& anDataIDs,
										OUT CArray<long,long>& anDrugListIDs,
										OUT CStringArray& astrDrugListNames,
										OUT CMap<long, long, CString, LPCTSTR> &mapDataIDsToSig,
										OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	// (c.haag 2007-08-18 12:37) - PLID 27114 - This function fills several arrays with all
	// information relevant to the Yes column of this Allergies table
	void PopulateAllergyIDArrays(CArray<long,long>& anAllergyIDs, 
									   CArray<long,long>& anDataIDs,
									   CStringArray& astrAllergyNames,
									   OPTIONAL IN ADODB::_Connection *lpCon = NULL);

public:
	// (c.haag 2008-07-24 12:15) - PLID 30826 - Returns TRUE if there is at least one saved problem for this detail
	// or any list items. This does not check deleted problems.
	BOOL DoesDetailOrChildrenHaveSavedProblems();

	// (z.manning 2010-02-26 08:29) - PLID 37412
	void UpdateTableDataOutput();

	// (z.manning 2011-05-27 09:36) - PLID 42131
	BOOL HasTransformFormulas();

	// (z.manning 2012-03-27 17:49) - LPID 33710
	BOOL HasCalculatedFields();

	// (z.manning 2011-11-15 17:14) - PLID 38130
	BOOL ShouldSpawnFromRememberedValues();

	// (z.manning 2011-11-16 12:38) - PLID 38130
	BOOL WasStateRemembered();

	// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
	BOOL IsRequired();

	// (c.haag 2012-04-02) - PLID 49350 - Returns FALSE if this detail is not a NexTech RTF-formatted narrative
	BOOL IsRtfNarrative() const;

protected:

	// (j.jones 2010-08-12 11:22) - PLID 39486 - parses out the SmartStamp Table's existing AutoNumber index for a given cell
	long ParseAutoNumberIndexFromCell(CEMNDetail *pTableDetail, TableRowID *pRowID, long nAutoNumberColumnID);

	// (j.jones 2010-08-12 12:09) - PLID 39486 - calculates a new AutoNumber index for a given SmartStamp Table cell,
	// and returns it with the proper prefix
	CString GenerateAutoNumberContentForCell(TableRow *ptr, TableColumn *ptcAutoNumber);

public:

	// (c.haag 2007-08-20 08:59) - PLID 27118 - Returns a lookup key for a map generated
	// by PopulateTableElementMap
	inline __int64 GetTableElementKey(TableRow *ptr, long nColumnID) {
		return (((__int64)ptr) << 32) + (__int64)nColumnID;
	}
	inline __int64 GetTableElementKey(const TableElement& te) {
		return GetTableElementKey(te.m_pRow, te.m_pColumn);
	}
	inline __int64 GetTableElementKey(TableRow *ptr, TableColumn *ptc) {
		return GetTableElementKey(ptr, ptc->nID);
	}
	// (z.manning 2009-03-04 09:41) - PLID 33072
	inline TableRow* GetRowFromTableElementKey(const __int64 nKey) {
		return (TableRow*)(nKey >> 32);
	}

	// (c.haag 2007-08-20 08:59) - PLID 27118 - This function creates a map of table
	// elements. The lookup key is the table position, and the value is the index in
	// m_arTableElements.
	void PopulateTableElementMap(CMap<__int64, __int64, int, int>& mapTable);

	// (a.walling 2009-10-12 13:35) - PLID 36024 - Private now
private:
	// (c.haag 2007-05-22 11:53) - PLID 26095 - We now support reference counting
	// for memory management
	long m_nRefCnt;

#if defined(WATCH_EMNDETAIL_REFCOUNTS)
	CStringList m_listRefCountHistory;

	void AppendRefCountHistory(const CString& strDescription) {
		m_listRefCountHistory.AddTail(strDescription);
	};
#endif

public:
#if defined(WATCH_EMNDETAIL_REFCOUNTS)
	void DumpRefCountHistory() {
		POSITION pos = m_listRefCountHistory.GetHeadPosition();

		while (pos) {
			CString strDescription = m_listRefCountHistory.GetNext(pos);
			CString strOut;
			strOut.Format("\t%s", strDescription);
			::OutputDebugString(strOut);
		}
	};
#endif


	// (a.walling 2009-10-12 13:35) - PLID 36046 - Log/track reference counting
	void Explicit__AddRef(LPCTSTR szDescription) { 		
		CString strRefCntDebugString;
		strRefCntDebugString.Format("%lu \tTID 0x%08x: Detail 0x%08x\tAddRef -- %s\n", GetTickCount(), GetCurrentThreadId(), this, szDescription);
#ifdef LOG_ALL_EMNDETAIL_REFCOUNTS
		::OutputDebugString(strRefCntDebugString);
#endif
#ifdef WATCH_EMNDETAIL_REFCOUNTS
		AppendRefCountHistory(strRefCntDebugString);
#endif

		// this may itself cause an access violation. Which is good for debugging, probably.
		strRefCntDebugString.Format("\t%lu \tRefCnt %li -> %li\tID %li ('%s')\n", GetTickCount(), this->Internal__GetRefCnt(), this->Internal__GetRefCnt() + 1, this->GetID(), this->GetLabelText());
#ifdef LOG_ALL_EMNDETAIL_REFCOUNTS
		::OutputDebugString(strRefCntDebugString);
#endif
#ifdef WATCH_EMNDETAIL_REFCOUNTS
		AppendRefCountHistory(strRefCntDebugString);
#endif
		
		Internal__AddRef();
	};

	void Explicit__Release(LPCTSTR szDescription) {		
		CString strRefCntDebugString;		
		strRefCntDebugString.Format("%lu \tTID 0x%08x: Detail 0x%08x\tRelease -- %s\n", GetTickCount(), GetCurrentThreadId(), this, szDescription);

#ifdef LOG_ALL_EMNDETAIL_REFCOUNTS
		::OutputDebugString(strRefCntDebugString);
#endif
#ifdef WATCH_EMNDETAIL_REFCOUNTS
		AppendRefCountHistory(strRefCntDebugString);
#endif

		// this may itself cause an access violation. Which is good for debugging, probably.
		strRefCntDebugString.Format("\t%lu \tRefCnt %li -> %li\tID %li ('%s')\n", GetTickCount(), this->Internal__GetRefCnt(), this->Internal__GetRefCnt() - 1, this->GetID(), this->GetLabelText());
#ifdef LOG_ALL_EMNDETAIL_REFCOUNTS
		::OutputDebugString(strRefCntDebugString);
#endif
#ifdef WATCH_EMNDETAIL_REFCOUNTS
		AppendRefCountHistory(strRefCntDebugString);
#endif

		Internal__Release();
	};

	void Explicit__QuietAddRef() {
		Internal__AddRef();
	};

	void Explicit__QuietRelease() {
		Internal__Release();
	};

	long Internal__GetRefCnt() {
		return m_nRefCnt; 
	};

private:
	// (j.jones 2010-03-08 10:04) - PLID 37594 - moved the implementation of these two functions
	// to the .cpp file, as Internal__Release now calls __Release itself, which is defined below
	void Internal__AddRef();
	void Internal__Release();

#define __QuietRelease Explicit__QuietRelease

// (a.walling 2009-10-13 13:57) - PLID 36046 - Log/track reference counting
#if defined(WATCH_EMNDETAIL_REFCOUNTS) || defined(LOG_ALL_EMNDETAIL_REFCOUNTS)

#define __AddRef(description) Explicit__AddRef(description)

#define __Release(description) Explicit__Release(description)

#else

#define __AddRef(description) Explicit__QuietAddRef()

#define __Release(description) Explicit__QuietRelease()
	
#endif
};

#endif

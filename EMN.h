//EMN.h

#ifndef EMN_H
#define EMN_H

#pragma once

// (j.jones 2013-05-08 09:23) - PLID 56596 - removed a number of .h files
class CEMRTopic;
class CEMNDetail;
class CEMR;
class CConfigEMRPreviewDlg;
class CEmrCodingGroup;
struct CHeaderFooterInfo;
struct CPreviewFields;
class LoadEMNRecordsets_Objects;
struct SkippedTopic;
class CShowProgressFeedbackDlg;
class CProgressParameter;
class SourceActionInfo;
class CEMNDetailArray;
class CProcessEmrActionsResult;
struct ContactLensOrder;
class CEMRItemAdvMultiPopupDlg;
struct DetailPopup;
struct NewCropBrowserResult;
class EMNProvider;
class EMNLab;
class EMNProcedure;
class EMNTodo;

#include <NxSystemUtilitiesLib/NxThread.h>
#include "EmrUtils.h"
#include "EMNAppointment.h"
#include "EMNChargeArray.h"
#include "EmnCodingGroupInfo.h"
//#include "NxAPI.h" // (a.walling 2014-04-01 3:42) - PLID 61334 - #import of API in stdafx (and apparently other widely-used headers) causes crash in cl.exe

// (z.manning 2011-07-07 12:59) - PLID 44469 - Class to track an array of EMN coding group data
class CEmnCodingGroupInfoArray : public CArray<CEmnCodingGroupInfo*,CEmnCodingGroupInfo*>
{
public:
	CEmnCodingGroupInfo* FindByCodingGroupID(const long nCodingGroupID)
	{
		for(int nIndex = 0; nIndex < GetCount(); nIndex++)
		{
			CEmnCodingGroupInfo *pCodingInfo = GetAt(nIndex);
			if(pCodingInfo->m_nEmrCodingGroupID == nCodingGroupID) {
				return pCodingInfo;
			}
		}
		return NULL;
	}

	void ClearByIndex(const int nIndex)
	{
		delete GetAt(nIndex);
		RemoveAt(nIndex);
	}

	void ClearAll()
	{
		for(int nIndex = GetCount() - 1; nIndex >= 0; nIndex--) {
			ClearByIndex(nIndex);
		}
	}
};

// (j.jones 2012-11-30 14:03) - PLID 53966 - moved the EMNMedication class to its own file
class EMNMedication;

class EMNDiagCode {
public:
	// (j.jones 2008-07-23 10:20) - PLID 30819 - changed the original nID to nDiagCodeID,
	// then added a new nID for the actual record ID
	long nID;
	long nDiagCodeID;
	// (b.savon 2014-02-27 07:03) - PLID 60808 - Add ICD10
	long nDiagCodeID_ICD10;
	long nOrderIndex;

	// (j.jones 2007-01-11 10:21) - PLID 24027 - supported SourceDetailID
	// (z.manning 2009-02-26 12:07) - PLID 33141 - All the source action/detail stuff is now within a class
	SourceActionInfo sai;

	BOOL bIsNew;
	// ICD - 9
	CString strCode;
	CString strCodeDesc;
	// (r.farnworth 2014-02-27 08:42) - PLID 60785 - UPDATE - Add preview pane functionality for icd-10 diag code logic 
	// ICD - 10
	CString strCode_ICD10;
	CString strCodeDesc_ICD10;
	// (r.farnworth 2014-03-05 14:51) - PLID 60820 - SPAWN - Need to store information on matchtype.
	NexGEMMatchType MatchType;
	// (c.haag 2014-03-17) - PLID 60929 - Added QuickListID
	long nQuickListID;

	BOOL bHasMoved;
	BOOL bChanged;
	// (j.jones 2014-12-23 15:07) - PLID 64491 - added bReplaced
	BOOL bReplaced;

	// (z.manning 2009-08-18 09:54) - PLID 35207 - Added a pointer to an EMN that can be set to
	// use as the EMN for problem links in the assignment operator.
	CEMN *pEmnOverride;

	// (j.jones 2008-07-22 10:13) - PLID 30792 - added an array to track problems
	// (c.haag 2009-05-16 12:03) - PLID 34310 - We now track problem links instead of problems.
	CArray<CEmrProblemLink*, CEmrProblemLink*> m_apEmrProblemLinks;

	EMNDiagCode() {
		nID = -1;
		nDiagCodeID = -1;
		// (b.savon 2014-02-27 07:03) - PLID 60808 - Add ICD10
		nDiagCodeID_ICD10 = -1;
		nQuickListID = -1;
		nOrderIndex = -1;
		MatchType = nexgemtDone;
		bIsNew = TRUE;
		bChanged = FALSE;
		bHasMoved = FALSE;		
		pEmnOverride = NULL;
		// (j.jones 2014-12-23 15:07) - PLID 64491 - added bReplaced
		bReplaced = FALSE;
	}

	// (j.jones 2008-07-22 11:41) - PLID 30792 - remove all problems
	// (c.haag 2009-05-19 12:05) - PLID 34310 - Use new problem link structure
	~EMNDiagCode() {

		for(int i = 0; i < m_apEmrProblemLinks.GetSize(); i++) {
			delete m_apEmrProblemLinks[i];
		}
		m_apEmrProblemLinks.RemoveAll();
	}

	// (j.jones 2008-07-22 10:17) - PLID 30792 - required after I added m_aryEmrProblems
	// (z.manning 2009-08-18 10:15) - PLID 35207 - Moved the body of this function to EMN.cpp
	void operator =(EMNDiagCode &cSource);

	// (j.jones 2008-07-22 10:59) - PLID 30792 - returns true if there are any undeleted problems on the diag code
	// (c.haag 2009-05-19 12:06) - PLID 34310 - Use new problem linking structure
	BOOL HasProblems()
	{
		try {

			for(int i=0; i<m_apEmrProblemLinks.GetSize(); i++) {
				CEmrProblem *pProblem = m_apEmrProblemLinks.GetAt(i)->GetProblem();
				if(pProblem != NULL && !pProblem->m_bIsDeleted) {

					return TRUE;
				}
			}

		}NxCatchAll("Error in EMNDiagCode::HasProblems");

		return FALSE;
	}

	// (j.jones 2008-07-22 10:59) - PLID 30792 - returns true if there are only undeleted, closed problems on the diag code
	// (c.haag 2009-05-19 12:06) - PLID 34310 - Use new problem linking structure
	BOOL HasOnlyClosedProblems()
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

		}NxCatchAll("Error in EMNDiagCode::HasOnlyClosedProblems");

		return FALSE;
	}

	// (j.jones 2008-07-23 11:06) - PLID 30792 - returns true if any problems are marked as modified,
	// including deleted items
	// (c.haag 2009-05-19 12:06) - PLID 34310 - Use new problem linking structure
	BOOL HasChangedProblems()
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

		}NxCatchAll("Error in EMNDiagCode::HasChangedProblems");

		return FALSE;
	}

	// (r.gonet 2015-01-06 11:02) - PLID 64509 - Added.
	// Returns TRUE if any problem links are changed. FALSE otherwise.
	BOOL HasChangedProblemLinks();

	// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
	void LogEmrObjectData(int nIndent, BOOL bDeleted);
};

// (j.jones 2011-07-05 11:20) - PLID 43603 - tracked EMN statuses as a class so we can store the name 
class EMNStatus {
public:
	long nID;
	CString strName;

	EMNStatus() {
		nID = -1;
	}

	// (a.walling 2012-04-06 12:13) - PLID 49496 - EMN Status - auto convert to long
	operator long() const
	{
		return nID;
	}
};


// (z.manning, 05/07/2007) - PLID 25731 - We now store charts in a class so we can keep track of the name too.
class EMNChart {
public:
	long nID;
	CString strName;

	EMNChart() {
		nID = -1;
	}

	operator long() const
	{
		return nID;
	}
};

// (z.manning, 05/07/2007) - PLID 25731 - We now store categories in a class so we can keep track of the name too.
class EMNCategory {
public:
	long nID;
	CString strName;

	EMNCategory() {
		nID = -1;
	}

	operator long() const
	{
		return nID;
	}
};

// (z.manning, 05/07/2007) - PLID 25925 - Put locations in a class so we can track location name for auditing purposes.
class EMNLocation {
public:
	long nID;
	CString strName;
	CString strLogo; // (a.walling 2008-07-01 14:59) - PLID 30586 - Logo path
	long nLogoWidth; // (a.walling 2010-10-29 10:33) - PLID 31435 - Logo width

	EMNLocation() 
		: nID(-1)
		, nLogoWidth(100)
	{
	};
};

//DRT 8/7/2007 - PLID 27003 - Original source pulled out of RevokeEmrAction
class CActionAndSource {
public:
	EmrAction ea;
	// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
	SourceActionInfo sai;

	CActionAndSource() {
	}

	CActionAndSource(EmrAction _ea, SourceActionInfo _sai) {
		ea = _ea;
		sai = _sai;
	}

	// (a.walling 2008-10-02 09:07) - PLID 31564 - We defined an alternative constructor, so we
	// now need to define a copy constructor since the compiler won't do it for us anymore.
	CActionAndSource(const CActionAndSource& src) {
		ea = src.ea;
		sai = src.sai;
	}
};

// (b.eyers 2016-02-22) - PLID 68321 - keep track of discharge status
class EMNDischargeStatus {
public:
	long nID;
	CString strCode;
	CString strDesc;

	EMNDischargeStatus() {
		nID = -1;
	}

	operator long() const
	{
		return nID;
	}
};

// (c.haag 2008-07-30 17:46) - PLID 30897 - This class is used in the final stages of
// unspawning EMR items. It encapsulates both a CActionAndSource, as well the pointer
// to the EMR object being deleted.
class CDoomedEmrObject
{
public:
	CDoomedEmrObject() : m_bIsSkipped(false) {};

	CActionAndSource m_aas;

protected:
	// The Emr object. Use m_aas.ea.eaoDestType to determine which to use. This must
	// be protected so that the caller is forced to use the Get/Set functions.
	union {
		EMNCharge* m_pCharge;
		EMNDiagCode* m_pDiagCode;
		EMNMedication* m_pMedication;
		EMNProcedure* m_pProcedure;
		CEMRTopic* m_pEMRTopic;
		CEMNDetail* m_pEMNDetail;
		CEMN* m_pEMN;
		long m_nTaskID;
		EMNLab *m_pLab; // (z.manning 2008-10-09 09:53) - PLID 31628
	};

	// (a.walling 2010-03-31 15:34) - PLID 38008 - Currently only for spawned EMNs. This is because we may need
	// to perform some actions on these EMNs regardless of whether they are being unspawned or not.
	bool m_bIsSkipped;

public:	
	// (a.walling 2010-03-31 15:34) - PLID 38008 - Get the skipped state
	bool GetIsSkipped() const {
		return m_bIsSkipped;
	}

	// (a.walling 2010-03-31 15:34) - PLID 38008 - Set the skipped state; throw an exception if not valid.
	void SetIsSkipped(bool bIsSkipped) {
		if (eaoMint == m_aas.ea.eaoDestType) {
			m_bIsSkipped = bIsSkipped;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to set skipped state for a non-EMN action");
		}
	}

	EMNCharge* GetCharge() const {
		if (eaoCpt == m_aas.ea.eaoDestType) {
			return m_pCharge;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to get a charge from a non-charge action");
		}
	}
	void SetCharge(EMNCharge* pCharge) {
		if (eaoCpt == m_aas.ea.eaoDestType) {
			m_pCharge = pCharge;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to set a charge for a non-charge action");
		}
	}
	EMNDiagCode* GetDiagCode() const {
		// (b.savon 2014-07-14 12:48) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
		if (/*eaoDiag*/eaoDiagnosis == m_aas.ea.eaoDestType) {
			return m_pDiagCode;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to get a diagnosis code from a non-diagnosis action");
		}
	}
	void SetDiagCode(EMNDiagCode* pDiagCode) {
		// (b.savon 2014-07-14 12:48) - PLID 62706 - Deprecate the old Diag DestType and handle the changes Practice wide
		if (/*eaoDiag*/eaoDiagnosis == m_aas.ea.eaoDestType) {
			m_pDiagCode = pDiagCode;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to set a diagnosis code for a non-diagnosis action");
		}
	}
	EMNMedication* GetMedication() const {
		if (eaoMedication == m_aas.ea.eaoDestType) {
			return m_pMedication;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to get a medication from a non-medication action");
		}
	}
	void SetMedication(EMNMedication* pMedication) {
		if (eaoMedication == m_aas.ea.eaoDestType) {
			m_pMedication = pMedication;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to set a medication for a non-medication action");
		}
	}
	EMNProcedure* GetProcedure() const {
		if (eaoProcedure == m_aas.ea.eaoDestType) {
			return m_pProcedure;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to get a procedure from a non-procedure action");
		}
	}
	void SetProcedure(EMNProcedure* pProcedure) {
		if (eaoProcedure == m_aas.ea.eaoDestType) {
			m_pProcedure = pProcedure;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to set a procedure for a non-procedure action");
		}
	}
	CEMRTopic* GetTopic() const {
		if (eaoMintItems == m_aas.ea.eaoDestType) {
			return m_pEMRTopic;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to get a topic from a non-topic action");
		}
	}
	void SetTopic(CEMRTopic* pTopic) {
		if (eaoMintItems == m_aas.ea.eaoDestType) {
			m_pEMRTopic = pTopic;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to set a topic for a non-topic action");
		}
	}
	CEMNDetail* GetDetail() const {
		if (eaoEmrItem == m_aas.ea.eaoDestType) {
			return m_pEMNDetail;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to get an item from a non-item action");
		}
	}
	void SetDetail(CEMNDetail* pDetail)
	{
		if (eaoEmrItem == m_aas.ea.eaoDestType) {
			m_pEMNDetail = pDetail;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to set an item for a non-item action");
		}
	}
	CEMN* GetEMN() const {
		if (eaoMint == m_aas.ea.eaoDestType) {
			return m_pEMN;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to get an EMN from a non-EMN action");
		}
	}
	void SetEMN(CEMN* pEMN)
	{
		if (eaoMint == m_aas.ea.eaoDestType) {
			m_pEMN = pEMN;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to set an EMN for a non-EMN action");
		}
	}
	long GetTodoID() const {
		if (eaoTodo == m_aas.ea.eaoDestType) {
			return m_nTaskID;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to get a todo from a non-todo action");
		}
	}
	void SetTodoID(long nTaskID) {
		if (eaoTodo == m_aas.ea.eaoDestType) {
			m_nTaskID = nTaskID;
		} else {
			ThrowNxException("CDoomedEmrObject : Attempted to set a todo for a non-todo action");
		}
	}
	// (z.manning 2008-10-09 09:52) - PLID 31628 - Labs
	EMNLab* GetLab() const
	{
		if(eaoLab == m_aas.ea.eaoDestType) {
			return m_pLab;
		}
		else {
			ThrowNxException("CDoomedEmrObject : Attempted to get a lab from a non-lab action");
		}
	}
	void SetLab(EMNLab* pLab)
	{
		if(eaoLab == m_aas.ea.eaoDestType) {
			m_pLab = pLab;
		}
		else {
			ThrowNxException("CDoomedEmrObject : Attempted to set a lab for a non-lab action");
		}
	}
};

typedef CArray<CDoomedEmrObject, CDoomedEmrObject&> CDoomedEmrObjectAry;

//Used for code simplification in some of the narrative functions.
class NarrativeMergeField 
{
	// (c.haag 2007-03-29 17:18) - PLID 25423 - In addition to retaining a
	// detail's value and sentence form, we now support retaining the detail
	// itself. In the case that we do, calling GetValue() or GetSentenceFormat()
	// will calculate both values on the fly, and store them into strValue and
	// strSentenceFormat, and return them. Then the detail is set to NULL, and we
	// revert to being a run of the mill merge field object
private:
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//CEMNDetail* pPendingDetail;
	//CEMN* pPendingDetailEMN;

private:
	// (c.haag 2007-03-29 17:18) - PLID 25423 - These are now internally referenced
	// by this object. Outside objects have to go through GetValue() and GetSentenceFormat(),
	// and this object will calculate these values if necessary
	CString strValue;
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//CString strSentenceFormat;

public:
	CString strName;

	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//bool bEditable;
	//bool bHasSentenceForm;
	//bool bIsSubField;
	//bool bIsCaseSensitive;
	long nSpawnedGroupID; //The GetSpawnedGroupID() of the detail this field represents, if any.
	bool bFilled;
	// (j.jones 2008-01-11 13:31) - PLID 18709 - added bIsLWMergeField
	bool bIsLWMergeField;

public:
	// (j.jones 2008-01-11 13:31) - PLID 18709 - added bIsLWMergeField
	NarrativeMergeField(const CString &strNameIn, const CString &strValueIn, const CString &strSentenceFormatIn, bool bEditableIn,
		bool bHasSentenceFormIn, bool bIsSubFieldIn, bool bIsCaseSensitiveIn, long nSpawnedGroupIDIn, bool bFilledIn, bool bIsLWMergeFieldIn)
	{

		// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
		//pPendingDetail = NULL;
		//pPendingDetailEMN = NULL;
		strName = strNameIn;
		strValue = strValueIn;
		//strSentenceFormat = strSentenceFormatIn;
		//bEditable = bEditableIn;
		//bHasSentenceForm = bHasSentenceFormIn;
		//bIsSubField = bIsSubFieldIn;
		//bIsCaseSensitive = bIsCaseSensitiveIn;
		nSpawnedGroupID = nSpawnedGroupIDIn;
		bFilled = bFilledIn;
		bIsLWMergeField = bIsLWMergeFieldIn;
	}
	// (c.haag 2007-03-29 17:19) - PLID 25423 - Special overload for passing in a detail rather than string literals. The
	// literals will be calculated on demand.
	// (j.jones 2008-01-11 13:31) - PLID 18709 - added bIsLWMergeField
	NarrativeMergeField(const CString &strNameIn, CEMNDetail* pPendingDetailIn, CEMN* pPendingDetailEMNIn, bool bEditableIn,
		bool bHasSentenceFormIn, bool bIsSubFieldIn, bool bIsCaseSensitiveIn, long nSpawnedGroupIDIn, bool bFilledIn, bool bIsLWMergeFieldIn)
	{

		// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
		//pPendingDetail = pPendingDetailIn;
		//pPendingDetailEMN = pPendingDetailEMNIn;
		strName = strNameIn;
		//bEditable = bEditableIn;
		//bHasSentenceForm = bHasSentenceFormIn;
		//bIsSubField = bIsSubFieldIn;
		//bIsCaseSensitive = bIsCaseSensitiveIn;
		nSpawnedGroupID = nSpawnedGroupIDIn;
		bFilled = bFilledIn;
		bIsLWMergeField = bIsLWMergeFieldIn;
	}
	NarrativeMergeField()
	{

		// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
		//pPendingDetail = NULL;
		//pPendingDetailEMN = NULL;
		//bEditable = bHasSentenceForm = bIsSubField = bIsCaseSensitive = bIsLWMergeField = false;
		bFilled = false;
		bIsLWMergeField = false;
		nSpawnedGroupID = -1;
	}

public:
	// (c.haag 2007-03-29 17:20) - PLID 25423 - Other objects must now go through these functions
	// to get privately maintained values	

	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//CEMNDetail* GetPendingDetail() const;
	//CEMN* GetPendingDetailEMN() const;
	CString GetValue();
	//CString GetSentenceFormat();

	// (a.walling 2009-11-19 15:08) - PLID 36365 - Used to update this object
	void SetData(const CString& strValueIn, bool bIsFilledIn)
	{
		strValue = strValueIn;
		bFilled = bIsFilledIn;
	};
};

//TES 4/15/2010 - PLID 24692 - Entries in the linked list which we now use to maintain topic positioning.
struct TopicPositionEntry
{
	//TES 4/15/2010 - PLID 24692 - We link to the next item, the previous item, and our child (which may be the head of its own list of
	// subtopics).  There's no need to maintain a parent, CEMRTopics do a fine job of maintaining their parent, all we care about is their
	// order relative to their parent, whatever it may be.
	TopicPositionEntry *pNext;
	TopicPositionEntry *pPrev;
	TopicPositionEntry *pChild;
	//TES 4/15/2010 - PLID 24692 - This topic's identifying information.
	long nTopicID;
	//TES 5/11/2010 - PLID 24692 - I noticed this wasn't getting properly set everywhere, and I realized it was foolish to be maintaining this
	// everywhere when we only care about it when saving, so it can just get passed in there.
	//bool bIsTemplateTopic;
	//TES 5/3/2010 - PLID 24692 - Store the last saved position, for auditing.
	long nLastSavedOrderIndex;

	TopicPositionEntry() {
		pNext = pPrev = pChild = NULL;
		nTopicID = nLastSavedOrderIndex = -1;
	}
	
	//TES 4/15/2010 - PLID 24692 - Returns this topic's sort order relative to its parent.
	long GetSortOrder() {
		//TES 4/15/2010 - PLID 24692 - If we're at the head of the list, our order is 0, otherwise its our predecessor's order+1.
		if(pPrev) {
			return pPrev->GetSortOrder()+1;
		}
		else {
			return 0;
		}
	}

	//TES 4/15/2010 - PLID 24692 - Destroys all descendants (pNext and pChild) of this entry.
	void FreeDescendants() {
		try {
		if(pNext) {
			pNext->FreeDescendants();
			delete pNext;
			pNext = NULL;
		}
		if(pChild) {
			pChild->FreeDescendants();
			delete pChild;
			pChild = NULL;
		}
		}NxCatchAll("Error in FreeDescendants()");
	}

	//TES 4/15/2010 - PLID 24692 - Generates an update string for just the OrderIndex for this entry and all descendants.
	// NOTE that this only applies to updating, when creating new entries, CEMRTopic will get it from just its own entry in the list.
	//TES 5/11/2010 - PLID 24692 - Pass in whether or not this is a template (used to be maintained by each object, which was silly).
	// (a.walling 2014-01-30 00:00) - PLID 60543 - Quantize TopicPositionEntry
	Nx::Quantum::Batch GenerateSaveString(BOOL bIsTemplateTopic);

	//TES 4/15/2010 - PLID 24692 - Adds the specified entry to the end of this list.
	void AddTail(TopicPositionEntry *pTail) {
		if(pNext) {
			pNext->AddTail(pTail);
		}
		else {
			pNext = pTail;
			pTail->pPrev = this;
		}
		//TES 4/15/2010 - PLID 24692 - A check to make sure we haven't created an infinite list.
		ASSERT(!pNext || !pNext->HasEntry(this));
	}

	//TES 4/15/2010 - PLID 24692 - Takes the given entry out of our list, does NOT destroy it.
	//TES 5/12/2010 - PLID 24692 - It is recommended that you not call this function directly, instead call DetachTopicPositionEntry().
	// That handles the case where the first entry in the list is the one to be detached.
	bool Detach(TopicPositionEntry *pDetach) {
		if(pNext == pDetach) {
			//TES 4/15/2010 - PLID 24692 - Remove the item from the list, attach ourselves to its successor.
			TopicPositionEntry *pNewNext = pNext->pNext;
			pNext = pNewNext;
			if(pNewNext != NULL) {
				pNewNext->pPrev = this;
			}
			pDetach->pPrev = pDetach->pNext = NULL;
			//TES 4/15/2010 - PLID 24692 - We're done, make sure we haven't created an infinite list.
			ASSERT(!pNext || !pNext->HasEntry(this));
			return true;
		}
		else {
			if(pNext) {
				//TES 4/15/2010 - PLID 24692 - Ask our successor to detach it.
				if(pNext->Detach(pDetach)) {
					//TES 4/15/2010 - PLID 24692 - It did, so we're done.make sure we haven't created an infinite list.
					ASSERT(!pNext || !pNext->HasEntry(this));
					return true;
				}
			}
			if(pChild) {
				if(pChild == pDetach) {
					//TES 4/15/2010 - PLID 24692 - It was our child, so give ourselves a new child, if possible.
					TopicPositionEntry *pNewChild = pChild->pNext;
					pChild = pNewChild;
					if(pNewChild) {
						//TES 4/15/2010 - PLID 24692 - This is now the head of its list.
						pNewChild->pPrev = NULL;
					}
					//TES 4/15/2010 - PLID 24692 - Detach.
					pDetach->pPrev = pDetach->pNext = NULL;
					//TES 4/15/2010 - PLID 24692 - A check to make sure we haven't created an infinite list.
					ASSERT(!pChild || !pChild->HasEntry(this));
					return true;
				}
				else {
					//TES 4/15/2010 - PLID 24692 - Ask our child to detach.
					bool bReturn = pChild->Detach(pDetach);
					//TES 4/15/2010 - PLID 24692 - Check to make sure we haven't created an infinite list.
					ASSERT(!pChild || !pChild->HasEntry(this));
					//TES 4/15/2010 - PLID 24692 - We've tried everything, so we've succeeded iff the child succeeded.
					return bReturn;
				}
			}
			else {
				//TES 4/15/2010 - PLID 24692 - We couldn't find it.
				return false;
			}
		}
	}

	//TES 4/15/2010 - PLID 24692 - Inserts the given entry before the given entry (detaching it from its present position if it already exists).
	// This only checks the current list, not children.
	void InsertBefore(TopicPositionEntry *pBefore, TopicPositionEntry *pInsert) {
		if(this == pBefore) {
			//TES 4/15/2010 - PLID 24692 - It belongs before us, so stick it in there and update the links.
			TopicPositionEntry *pOldPrev = pPrev;
			pPrev = pInsert;
			pInsert->pNext = this;
			pInsert->pPrev = pOldPrev;
			if(pOldPrev) {
				pOldPrev->pNext = pInsert;
			}
		}
		else if(pNext) {
			//TES 4/15/2010 - PLID 24692 - Ask our successor.
			pNext->InsertBefore(pBefore, pInsert);
		}
		//TES 4/15/2010 - PLID 24692 - Check to make sure we haven't created an infinite list.
		ASSERT(!pNext || !pNext->HasEntry(this));
	}

	//TES 4/15/2010 - PLID 24692 - Find the entry corresponding to the given ID (NULL if it doesn't exist in this list).
	TopicPositionEntry* GetEntryByID(long nID)
	{
		//TES 4/15/2010 - PLID 24692 - Is it us?
		if(nTopicID == nID) {
			return this;
		}
		else {
			if(pNext) {
				//TES 4/15/2010 - PLID 24692 - Is it one of our successors?
				TopicPositionEntry *p = pNext->GetEntryByID(nID);
				if(p) {
					return p;
				}
			}
			if(pChild) {
				//TES 4/15/2010 - PLID 24692 - Does our child have it?
				TopicPositionEntry *p = pChild->GetEntryByID(nID);
				if(p) {
					return p;
				}
			}
			//TES 4/15/2010 - PLID 24692 - We don't have it.
			return NULL;
		}
	}

	//TES 4/15/2010 - PLID 24692 - Is the given entry in our list?
	bool HasEntry(TopicPositionEntry *tpe)
	{
		//TES 4/15/2010 - PLID 24692 - Is it us?
		if(this == tpe) return true;
		//TES 4/15/2010 - PLID 24692 - Is it one of our successors?
		else if(pNext && pNext->HasEntry(tpe)) return true;
		//TES 4/15/2010 - PLID 24692 - Does our child have it?
		else if(pChild && pChild->HasEntry(tpe)) return true;
		//TES 4/15/2010 - PLID 24692 - It doesn't exist.
		else return false;
	}

	//TES 4/15/2010 - PLID 24692 - For debugging purposes, output a string that has this topic's IDs, and all of it's successors and children's
	// IDs.
	CString GetOutputText()
	{
		//TES 4/15/2010 - PLID 24692 - Format: ID(Child->Child2)->Next->Next2
		if(pChild) {
			return AsString(nTopicID) + "(" + pChild->GetOutputText() + ")" + (pNext?"->"+pNext->GetOutputText():"");
		}
		else {
			return AsString(nTopicID) + (pNext?"->"+pNext->GetOutputText():"");
		}
	}

	//TES 5/3/2010 - PLID 24692 - Set the last saved order, for auditing.  Note that we don't pass it in, the item will calculate its
	// current order, and use it as the last saved order.
	void SetLastSavedOrderIndex(bool bRecurse = false)
	{
		nLastSavedOrderIndex = GetSortOrder();
		if(bRecurse) {
			if(pNext) pNext->SetLastSavedOrderIndex(true);
			if(pChild) pChild->SetLastSavedOrderIndex(true);
		}
	}

	//TES 5/12/2010 - PLID 24692 - Recurses back through pPrev, returns the first entry in thelist.
	TopicPositionEntry* GetListHead()
	{
		if(pPrev) {
			return pPrev->GetListHead();
		}
		else {
			return this;
		}
	}
};
	

// (a.walling 2009-11-17 16:48) - PLID 36365
typedef CMap<CString, LPCTSTR, NarrativeMergeField, NarrativeMergeField&> CNarrativeMergeFieldMap;

class CEmrItemAdvNarrativeDlg;

// (c.haag 2007-04-24 11:01) - PLID 26463 - We now use the official EMN loader
// object rather than the old preloaded detail array object
class CEMNLoader;
// (c.haag 2007-08-06 10:35) - PLID 26954 - We now use CEMNSpawner to preload
// spawning-related data in bulk
class CEMNSpawner;

struct GlassesOrder;

class CEMN
{
public:
	// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
	CEMN(CEMR *pParentEMR, ADODB::_ConnectionPtr pDefaultConnection = NULL);
	~CEMN();
	void operator =(CEMN &emnSource);
	// (z.manning 2009-08-19 15:55) - PLID 35285
	void CopyFromEmn(CEMN *pemnSource, BOOL bCopyEmr);

	// (j.jones 2005-11-28 17:24) - child topics and details will use this pointer as a
	// stepping stone to the CEMR, which will protect its own data accordingly
	CEMR* GetParentEMR();
	void SetParentEMR(CEMR* pEMR);
	
	// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
	ADODB::_ConnectionPtr GetRemoteData();

	// (c.haag 2007-10-02 08:49) - PLID 27024 - Ensures that the "retired" template loader;
	// that is, the CEMNLoader object that was used to load this template, no longer exists.
	// This is either called when a template is saved and the loader becomes out of date; or
	// when we're deleting the CEMN object.
	void EnsureNotRetiredTemplateLoader();

	// (c.haag 2007-08-31 10:55) - PLID 27268 - The following functions are used in creating 
	// queries to load EMR template information
	void AppendTemplateLoadSubQuery_rsPatientInfo(CString& strSql) const;
	void AppendTemplateLoadSubQuery_rsEMN(CString& strSql) const;
	void AppendTemplateLoadSubQuery_rsProcs(CString& strSql) const;
	void AppendTemplateLoadSubQuery_rsDiags(CString& strSql, const CString& strIncludePatientICD9) const;
	void AppendTemplateLoadSubQuery_rsCharges(CString& strSql) const;
	void AppendTemplateLoadSubQuery_rsMedications(CString& strSql) const;
	//TES 4/15/2010 - PLID 24692
	void AppendTemplateLoadSubQuery_rsTopicPositions(CString& strSql) const;

	// (j.armen 2014-07-23 12:27) - PLID 62837 - Declarations
	CString GetrsPreloadDetailsBasicQDeclarations() const;
	CString GetrsPreloadDetailsBasicQ() const;

	// (a.walling 2013-07-18 10:14) - PLID 57628 - Removed old EmrTableDropdownInfoT data maps and cache logic

	void AppendTemplateLoadSubQuery_rsPreloadDetails(CString& strSql) const;
	void AppendTemplateLoadSubQuery_rsPreloadTemplateTopics(CString& strSql) const;
	// (c.haag 2011-03-17) - PLID 42895 - Returns common lists of details in topics for a specified template
	void AppendTemplateLoadSubQuery_rsPreloadCommonLists(CString& strSql) const;
	// (c.haag 2011-03-17) - PLID 42895 - Returns common list items of details in topics for a specified template
	void AppendTemplateLoadSubQuery_rsPreloadCommonListItems(CString& strSql) const;

	// (c.haag 2014-07-17) - PLID 54905 - Called to read results from a recordset built from AppendLoadSubQuery_rsChargesWhichCodes
	void LoadChargesFromEmnRecordsets(long nEmnID, ADODB::_RecordsetPtr rsCharges, ADODB::_RecordsetPtr rsWhichCodes);
	//Load an existing EmrMasterT record.
	//DRT 7/27/2007 - PLID 26836 - Added bLoadInBackground parameter, see implementation for details.
	//ADODB::_RecordsetPtr LoadFromEmnRecordsets(long nEmnID, ADODB::_Recordset *lprsAllRecordsets, BOOL bLoadInBackground);
	//DRT 9/17/2007 - PLID 27384 - Send in all recordsets as parameters now.  No longer a return value needed
	// (j.jones 2008-10-31 13:00) - PLID 31869 - passed in a connection pointer
	void LoadFromEmnRecordsets(long nEmnID, LoadEMNRecordsets_Objects* pLero, BOOL bLoadInBackground, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	// (j.jones 2008-10-30 16:11) - PLID 31869 - passed in a connection pointer
	void LoadFromEmnID(long nEmnID, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
	// (s.dhole 2014-03-10 12:43) - PLID 61303
	BOOL CheckDuplicateDiagCode( long nDiagCodeID, long nDiagCodeID10);

	//Load an existing EmrTemplateT record (either as part of loading a template, or into a new EMN).
	// (j.jones 2007-01-11 10:21) - PLID 24027 - supported SourceDetailID
	//TES 2/26/2007 - PLID 24768 - Took out the bForceAddAsNew parameter, if we want to call SetNew() we need to do it
	// after the EMN is completely loaded.
	// (j.jones 2007-07-17 09:45) - PLID 26702 - added pSourceEMN, for when we create dummy EMNs
	// for popups, we can query the source EMN for key data
	// (z.manning 2009-03-04 14:20) - PLID 33338 - Use the new source action info class
	void LoadFromTemplateRecordsets(long nEmrTemplateID, BOOL bIsTemplate, SourceActionInfo &sai, BOOL bLoadHiddenTopics, ADODB::_Recordset *lprsAllRecordsets, OPTIONAL IN CEMN *pSourceEMN = NULL);
	// (z.manning, 04/12/2007) - PLID 25600 - Removed category ID as an optional parameter.
	// (j.jones 2007-07-17 09:45) - PLID 26702 - added pSourceEMN, for when we create dummy EMNs
	// for popups, we can query the source EMN for key data
	// (z.manning 2009-03-04 14:22) - PLID 33338 - Use the new source action info class
	void LoadFromTemplateID(long nEmrTemplateID, BOOL bIsTemplate, SourceActionInfo &sai, BOOL bLoadHiddenTopics = FALSE, OPTIONAL IN CEMN *pSourceEMN = NULL);
	//create a new EMN object with a collection ID
	void CreateNewTemplateEMN(long nCollectionID);

	//TES 2/3/2006 - Called by topics that have been loaded independently, loads only the information relevant to this topic.
	// (j.jones 2007-08-02 11:42) - PLID 26915 - added ability to pass in a connection
	void LoadFromTopic(CEMRTopic *pTopic, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	//TES 12/4/2006 - PLID 22304 - Is this EMN still in its initial load?
	BOOL IsLoading();

	//TES 2/20/2007 - PLID 24750 - We need to be able to tell the EMN that it is finished loading, this is for new templates
	// or generic EMNs, where they don't actually load anything from data.
	// (a.walling 2009-12-29 09:10) - PLID 36659 - Need to suppress preview generation when copying an EMN, for example
	void SetLoaded(bool bGeneratePreview = true);

	// (a.walling 2012-03-23 15:27) - PLID 49187
	long GetID() const
	{
		return m_nID;
	}

	// (a.walling 2012-03-23 15:27) - PLID 49187
	long GetSafeID() const
	{
		if (!this) return -1;
		return GetID();
	}

	// (a.walling 2012-06-11 09:27) - PLID 50922 - Update patient demographics
	void UpdatePatientDemographics(CWnd* pParentWnd);

	// (a.walling 2012-05-17 17:46) - PLID 50495 - EMR Date - just do this inline
	const COleDateTime& GetEMNDate() const
	{
		return m_dtEMNDate;
	}

	void SetEMNDate(const COleDateTime& dtDate);
	// (a.walling 2012-06-11 09:20) - PLID 50495 - EMR Date - Need to update the age in some situations
	void UpdateEMNDate(COleDateTime dtDate, CWnd* pParentWnd);

	// (a.walling 2012-06-07 08:53) - PLID 50920 - Dates - Modified, Created
	const COleDateTime& GetEMNModifiedDate() const
	{
		return m_dtEMNModifiedDate;
	}
	// (z.manning 2012-09-11 14:46) - PLID 52543
	void SetEMNModifiedDate(const COleDateTime &dtModifiedDate)
	{
		m_dtEMNModifiedDate = dtModifiedDate;
	}

	// (a.walling 2012-06-07 08:53) - PLID 50920 - Dates - Modified, Created
	const COleDateTime& GetEMNInputDate() const
	{
		return m_dtEMNInputDate;
	}

	// (z.manning 2010-01-13 10:40) - PLID 22672 - Age is now a string
	const CString& GetPatientAge() const
	{
		return m_strPatientAge;
	}
	void SetPatientAge(const CString& strPatientAge)
	{
		m_strPatientAge = strPatientAge;
	}

	BYTE GetPatientGender() const
	{
		return m_cbPatientGender;
	}
	void SetPatientGender(BYTE iPatientGender)
	{
		m_cbPatientGender = iPatientGender;
	}

	LPCTSTR GetPatientGenderName() const
	{
		switch (GetPatientGender()) {
			case 1:	return "Male";
			case 2: return "Female";
		}
		return "";
	}

	//m.hancock - 3/14/2006 - 19579 - Patient demographics shouldn't change after the EMN is locked or finished.
	const CString& GetPatientNameFirst() const
	{
		return m_strPatNameFirst;
	}
	void SetPatientNameFirst(const CString& strPatientNameFirst)
	{
		m_strPatNameFirst = strPatientNameFirst;
	}
	
	const CString& GetPatientNameMiddle() const
	{
		return m_strPatNameMiddle;
	}
	void SetPatientNameMiddle(const CString& strPatientNameMiddle)
	{
		m_strPatNameMiddle = strPatientNameMiddle;
	}
	
	const CString& GetPatientNameLast() const
	{
		return m_strPatNameLast;
	}
	void SetPatientNameLast(const CString& strPatientNameLast)
	{
		m_strPatNameLast = strPatientNameLast;
	}

	// (a.walling 2012-06-07 09:11) - PLID 50920 - Dates - Modified, Created
	CString GetPatientName() const;

	//DRT 7/13/2007 - PLID 26671
	void GetProcedures(OUT CArray<EMNProcedure, EMNProcedure> &arProcedures);
	// (z.manning 2011-11-10 15:22) - PLID 37093
	void GetProcedureIDs(OUT CArray<long,long> *parynProcedureIDs);

	// (j.jones 2007-01-11 10:22) - PLID 24027 - supported SourceDetailID
	// (z.manning 2009-02-27 10:39) - PLID 33141 - Use the new source action info class
	EMNProcedure* AddProcedure(long nProcedureID, CString strName, SourceActionInfo &sai);
	EMNProcedure* AddProcedure(long nProcedureID, CString strName);
	EMNProcedure* AddProcedure(EMNProcedure* pProcedure);

	void GetProviders(OUT CArray<long,long> &arProviderIDs);
	void SetProviders(const CArray<long,long> &arProviderIDs);

	// (j.jones 2007-07-17 10:42) - PLID 26702 - added ability to get/set the entire provider list,
	// not just the IDs of the providers
	void GetProviderList(OUT CArray<EMNProvider*,EMNProvider*> &aryProviders);
	void SetProviderList(const CArray<EMNProvider*,EMNProvider*> &aryProviders);

	// (j.gruber 2007-01-08 11:23) - PLID 23399 - Secondary providers
	void GetSecondaryProviders(OUT CArray<long,long> &arSecondaryProviderIDs);
	void SetSecondaryProviders(const CArray<long,long> &arSecondaryProviderIDs);

	// (d.lange 2011-03-23 09:04) - PLID 42136 - Assistant/Technician
	void GetTechnicians(OUT CArray<long,long> &arTechnicianIDs);
	void SetTechnicians(const CArray<long,long> &arTechnicianIDs);

	// (j.gruber 2009-05-07 16:38) - PLID 33688 - Other providers
	void GetOtherProviders(OUT CArray<EMNProvider*, EMNProvider*>  &arOtherProviderIDs);	
	void SetOtherProviders(const CArray<EMNProvider*,EMNProvider*> &arOtherProviderIDs);

	// (a.walling 2012-03-26 08:55) - PLID 49141 - use const references, ignore set if values are equivalent
	const CString& GetDescription()
	{
		return m_strDescription;
	}
	void SetDescription(const CString& strDescription);

	// (a.walling 2012-03-26 08:55) - PLID 49141 - use const references, ignore set if values are equivalent
	const CString& GetNotes()
	{
		return m_strNotes;
	}
	void SetNotes(const CString& strNotes);

	CString GetTableItemList();

	// (a.walling 2012-04-06 12:13) - PLID 49496 - EMN Status
	void SetStatus(EMNStatus emnStatus);
	const EMNStatus& GetStatus()
	{
		return m_Status; // auto-converts to long
	}

	// (j.jones 2011-04-28 14:39) - PLID 43122 - returns the primary provider ID on the EMN
	// that has FloatEMRData set to true
	long GetProviderIDForFloatingData();

	// (j.jones 2007-08-24 08:40) - PLID 27054 - added VisitTypeID
	long GetVisitTypeID();
	CString GetVisitTypeName();
	void SetVisitType(long nVisitTypeID, CString strVisitTypeName);

	//TES 7/10/2009 - PLID 25154 - Option to send billing info to HL7.
	bool GetSendBillToHL7();
	void SetSendBillToHL7(bool bSend);
	// (a.wilson 2013-06-13 15:12) - PLID 57165 - get the flag to determine if charges have changed at all.
	bool GetChargesChanged();
	void SetChargesChanged(bool bChanged);

	//TES 1/17/2014 - PLID 60397 - Option to hide the EMN title on the preview pane, per template
	bool GetHideTitleOnPreview();
	void SetHideTitleOnPreview(bool bHide);

	BOOL IsLockedAndSaved(); //returns TRUE if the EMN is locked and has been saved, meaning no further saves should be allowed
	
	long GetTopicCount();
	CEMRTopic* GetTopic(long nIndex);
	CEMRTopic* GetTopicByID(long nID);
	//This will try to find one that matches both nTemplateTopicID and nSpawnedGroupID first, but if that doesn't work will
	//just match nTemplateTopicID.
	// (a.walling 2010-04-05 13:15) - PLID 38060 - Also look for the spawned group table row
	// (a.walling 2010-04-05 13:50) - PLID 38060 - Now will only fallback to TemplateTopicIDOnly when necessary
	CEMRTopic* GetTopicByTemplateTopicID(long nTemplateTopicID, long nSpawnedGroupID, CEMNDetail *pSourceDetail, SourceActionInfo* pSpawnedGroupSourceActionInfo = NULL);
	// (a.walling 2010-04-05 13:50) - PLID 38060 - Actually performs the lookup
	CEMRTopic* GetTopicByTemplateTopicID_Internal(long nTemplateTopicID, long nSpawnedGroupID, CEMNDetail *pSourceDetail, SourceActionInfo* pSpawnedGroupSourceActionInfo );
	// (a.walling 2010-04-05 13:50) - PLID 38060 - Fallback if nothing found above
	CEMRTopic* GetTopicByTemplateTopicIDOnly(long nTemplateTopicID);
	void RemoveTopic(CEMRTopic* pTopic);
	//Takes the topic out of our array, but doesn't destroy it.
	void DetachTopic(CEMRTopic* pTopic);
	//Recursively gets all topics and subtopics and puts them in an array.
	void GetAllTopics(CArray<CEMRTopic*,CEMRTopic*> &arTopics);
	//Recursively gets all deleted topics and subtopics and puts them in an array.
	void GetAllDeletedTopics(CArray<CEMRTopic*,CEMRTopic*> &arDeletedTopics);
	//Recursively gets all subtopics for a given topic and puts them in an array.
	void AddTopicToList(CEMRTopic *pTopic, CArray<CEMRTopic*,CEMRTopic*> &arTopics);
	//Recursively gets all deleted subtopics for a given topic and puts them in an array.
	void AddDeletedTopicToList(CEMRTopic *pTopicToCheck, CArray<CEMRTopic*,CEMRTopic*> &arDeletedTopics);

	// (z.manning 2009-06-23 15:20) - PLID 34692 - This function will go through each non-new topic
	// (m_nID != -1) and ensure it's not deleted on the current EMN.
	void EnsureTopicsNotDeletedIfNotNew(CArray<CEMRTopic*,CEMRTopic*> &arypTopics);
	// (z.manning 2009-06-23 14:53) - PLID 34692 - Function to go through all topics on the EMN
	// and make sure that a topic with the given ID is not in any of the deleted topic arrays.
	void EnsureTopicNotDeleted(const long nTopicID);

	//lets us know if we have a pointer to a topic that is marked as to be deleted
	BOOL IsTopicMarkedDeleted(CEMRTopic *pTopicToCheck);

	// (a.walling 2012-07-09 12:35) - PLID 51441
	CEMRTopic* GetFirstDisplayedTreeTopic(OPTIONAL CEMNDetail *pDetailToIgnore = NULL, OPTIONAL BOOL bIgnoreBlankSubtopics = TRUE, OPTIONAL CEMRTopic* pTopicToIgnore = NULL);
	CEMRTopic* GetLastDisplayedTreeTopic(OPTIONAL CEMNDetail *pDetailToIgnore = NULL, OPTIONAL BOOL bIgnoreBlankSubtopics = TRUE, OPTIONAL CEMRTopic* pTopicToIgnore = NULL);

	//the following functions deal with all the details under all topics for this EMN

	//returns the count of all details under all topics for this EMN
	long GetTotalDetailCount();	
	//returns the detail based on the index of the GetTotalDetailCount
	CEMNDetail* GetDetail(long nIndex);
	//returns the detail by its saved EMRDetailsT.ID
	CEMNDetail* GetDetailByID(long nID);
	//returns the detail by its EMRTemplateDetailsT.ID
	CEMNDetail* GetDetailByTemplateDetailID(long nID);
	// (j.jones 2010-02-12 14:19) - PLID 37318 - added GetSmartStampImageDetail_ByTemplateDetailID which is
	// a clone of GetDetailByTemplateDetailID, but only returns a detail where m_pSmartStampTableDetail is NULL
	CEMNDetail* GetSmartStampImageDetail_ByTemplateDetailID(long nID);

	// (z.manning 2011-01-25 15:58) - PLID 42336
	void GetParentDetailsByDetailID(const long nChildDetailID, OUT CEMNDetailArray *paryParentDetails);
	void GetParentDetailsByTemplateDetailID(const long nChildTemplateDetailID, OUT CEMNDetailArray *paryParentDetails);

	// (z.manning 2011-01-28 12:26) - PLID 42336 - This function will look for any smart stamp table with the given
	// table info master ID as long as it is not already linked to an image with the given image info master ID.
	CEMNDetail* GetSmartStampTableDetailByInfoMasterID(const long nTableInfoMasterID, const long nImageInfoMasterID);

	// (j.jones 2010-03-11 09:51) - PLID 37318 - added an ability to reconfirm all SmartStamp links
	// on this EMN, after we've copied SmartStamps from another EMN (could be the EMN Copy feature, Import Topics,
	// using the loader, etc.)
	void ReconfirmSmartStampLinks_PostEMNCopy();

	//Finds the detail with this identifier, which is definitely unique against this EMN.
	CEMNDetail* GetDetailByUniqueIdentifier(const CString &strID);
	// (z.manning 2008-10-09 16:05) - PLID 31628 - Gets all details with the given lab ID
	void GetDetailsByLabID(IN const long nLabID, OUT CArray<CEMNDetail*,CEMNDetail*> &m_arypDetails);

	// (c.haag 2007-08-13 17:36) - PLID 27049 - These are like the other Get... functions except
	// that we are guaranteed a fully loaded detail that permanently resides in this EMN
	// when the initial load is done. You should call use this function when you actually
	// want a pointer to the detail to pass on to something else, such as 
	// TableElement::m_pLinkedDetail. The rest of the time, you should be using the other
	// Get... functions. Either way, if you actually change the detail data, those changes
	// will be carried to  the EMN.
	CEMNDetail* AssignDetail(long nIndex);
	CEMNDetail* AssignDetailByID(long nID);
	CEMNDetail* AssignDetailByTemplateDetailID(long nID);
	CEMNDetail* AssignDetailByUniqueIdentifier(const CString &strID);

	// (a.walling 2009-11-17 08:36) - PLID 36365 - Do not include pIgnoreTopic
	void GenerateTotalEMNDetailArray(CArray<CEMNDetail*,CEMNDetail*> *arypEMNDetails, CEMRTopic* pIgnoreTopic = NULL);
	void GenerateTotalEMNDeletedDetailArray(CArray<CEMNDetail*,CEMNDetail*> *arypEMNDeletedDetails);

	//Stores the temp .htm files so they can be deleted.
	CStringArray m_saTempFiles;

	// (a.walling 2014-07-08 14:19) - PLID 62812 - Use MFCArray
	MFCArray<EmrAction> m_arActionsToRevoke; // Actions that need to be revoked probably because they were deleted.

	// (c.haag 2007-03-28 17:12) - PLID 25397 - We optionally pass in the data output for optimizing the
	// execution of functions that need to get both individually
	// (a.walling 2007-04-12 11:22) - PLID 25605 - Need to handle rendering images to a temp file not involving a merge engine.
	// (c.haag 2008-02-22 10:39) - PLID 29064 - Added optional connection parameters to necessary functions
	CString GetSentence(CEMNDetail *pEMNDetail, CMergeEngine *pMi, bool bAllowHtml, bool bInHtml,
		EmrCategoryFormat Format = ecfParagraph, LPCTSTR szDataOutput = NULL, CEmrPreviewImageInfo* pPreviewImageInfo = NULL, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
	// (a.walling 2010-03-26 18:05) - PLID 37923 - Limit on a single table row
	CString GetDataOutput(CEMNDetail *pEMNDetail, CMergeEngine *pMi, bool bAllowHtml, bool bInHtml, OPTIONAL OUT bool *pbDataIsHtml = NULL, EmrCategoryFormat Format = ecfParagraph, CEmrPreviewImageInfo* pPreviewImageInfo = NULL, OPTIONAL IN ADODB::_Connection *lpCon = NULL, TableRowID* pSingleTableRowID = NULL);
	// (a.walling 2010-03-26 18:05) - PLID 37923 - Limit on a single table row
	// (z.manning 2010-08-09 15:25) - PLID 39842 - Added bAllowHtml
	void GetTableDataOutput(CEMNDetail *pEMNDetail, EmrMultiSelectFormat emsf, CString strSeparator, CString strSeparatorFinal, CString &strHTML, CString &strNonHTML, bool bAllowHtml, OPTIONAL OUT bool *pbDataIsHtml = NULL, EmrCategoryFormat Format = ecfParagraph, OPTIONAL IN ADODB::_Connection *lpCon = NULL, TableRowID* pSingleTableRowID = NULL);
	CString GetElementDataOutput(CEMNDetail *pEMNDetail, long nElementID, CMergeEngine *pMi, bool bAllowHtml, bool bInHtml, OPTIONAL OUT bool *pbDataIsHtml = NULL);
	CString GetElementSentence(CEMNDetail *pEMNDetail, long nElementID, CMergeEngine *pMi, bool bAllowHtml, bool bInHtml);

	void FillArrayFromSemiColonDelimitedIDList(IN OUT CDWordArray &arydw, IN const CString &strSemiColonDelimitedIDList);

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code


	//Call this when loading to more efficiently initialize all the narratives.
	// (j.jones 2008-10-30 17:24) - PLID 31869 - passed in a connection pointer
	//TES 7/22/2011 - PLID 44665 - Added a parameter specifying whether this is being called during the initial load of the EMN.
	void LoadAllNarratives(BOOL bInitialLoad, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	long EnsureLWMergeFields(CMap<CString, LPCTSTR, long, long&>& mapFields);

	BOOL GetGenericMergeFieldValue(const CString& strField, CString& strValue, bool& bIsValid);

	// (z.manning 2011-11-10 15:24) - PLID 37093
	// (j.jones 2012-12-27 15:25) - PLID 54369 - the above item was never finished, and this created broken, half-working code
	/*
	void GetProcedureFieldValue(NarrativeField *pnf);

	// (z.manning 2011-11-10 17:14) - PLID 37093
	void UpdateAllProcedureNarrativeFields();
	*/

	// (a.walling 2010-03-29 08:25) - PLID 37923 - Ensure the Spawning EMN Text field is populated
	bool EnsureSpawningEMNTextMergeField(CNarrativeMergeFieldMap* pmapGenericMergeFields = NULL, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	// (a.walling 2009-11-17 16:45) - PLID 36365	
	CNarrativeMergeFieldMap m_mapGenericMergeFields;

	CNarrativeMergeFieldMap m_mapLWMergeFields;

	//used when an item is changed from the administrator,
	//for all instances of that item in the EMN
	void MarkItemChanged(long nEMRInfoID);

	// (c.haag 2010-07-01 11:52) - PLID 36061 - This function creates and returns a new CEMNDetail corresponding to a lab
	CEMNDetail* CreateNewLabDetail(CEMRTopic *pTopic, EMNLab *pNewLab, BOOL bIsInitialLoad);

	//refreshes all details in the EMN
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

	//invalidates merge buttons across the entire EMN
	void InvalidateAllDetailMergeButtons();

	void TryToOverrideMergeField(CEMNDetail *pDetail,BOOL bOverrideAllWithName = TRUE);

	//Correctly sets the exclamation button on all items on the given EMN with the given merge name (converted to header).
	//If pDetailToIgnore is set, it will behave as if that detail doesn't exist.
	void UpdateMergeConflicts(const CString &strMergeFieldName, CEMNDetail *pDetailToIgnore = NULL);
	//TES 5/26/2006 - When loading, call this once at the end for more efficient updating of merge conflicts.
	void UpdateAllMergeConflicts();

	void GenerateTableItemList();

	//TES 10/9/2006 - PLID 22932 - We need to track which objects are already in our save string.
	// (j.jones 2007-01-11 14:28) - PLID 24027 - tracked strPostSaveSql, for sql statements to occur after the main save
	// (c.haag 2007-06-20 12:38) - PLID 26397 - We now store saved objects in a map for fast lookups
	// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
	// (j.jones 2012-09-27 15:11) - PLID 52820 - now we track a flag if something that contributed to drug interactions has changed,
	// such as new or deleted prescriptions, or new or deleted diagnosis codes
	Nx::Quantum::Batch GenerateSaveString(long nEMRID, long &nAuditTransactionID, Nx::Quantum::Batch& strPostSaveSql, CStringArray &arystrErrors, IN OUT CMapPtrToPtr& mapSavedObjects, CDWordArray &arynAffectedDetailIDs, OUT BOOL &bDrugInteractionsChanged, BOOL bIsTopLevelSave, BOOL bSaveRecordOnly);
	BOOL PropagateNewID(long nID, EmrSaveObjectType esotSaveType, long nObjectPtr, long &nAuditTransactionID);
	// (c.haag 2016-06-09 14:54) - PLID-66502 - Write pertinent information about this EMR object to NxLog. This is used to help pin down save errors.
	void LogEmrObjectData(int nIndent, BOOL bForceDeletedFlagTrue, BOOL bInEMRPendingDeleteEMNsAry);

	// (z.manning 2011-03-04 14:13) - PLID 42682
	void UpdateSourceDetailStampIDs(const long nOldDetailStampID, const long nNewDetailStampID);
	
	// (a.walling 2010-03-31 11:23) - PLID 38006 - Now we need to ensure any cross-EMN IDs are updated
	void PropagateCrossEMNIDs();

	// (a.walling 2007-10-18 16:34) - PLID 27664 - Added array to gather all topics affected in the PostSaveUpdate cascade.
	void PostSaveUpdate(CShowProgressFeedbackDlg* pProgressDlg, BOOL bTopLevelUpdate = FALSE, BOOL bUpdateRecordOnly = FALSE, CArray<CEMRTopic*, CEMRTopic*> *parTopicsAffected = NULL);

	// (c.haag 2012-10-17) - PLID 52863 - This must be called after PropagateNewID for all ID's so that we can assign ID values to EMRTodosT.
	void UpdateUnsavedTodos();

	// (b.cardillo 2009-06-03 15:10) - PLID 34370 - Keep track of all updated or deleted EMRDetailIDs
	Nx::Quantum::Batch GenerateDeleteString(long &nAuditTransactionID, CStringArray &arystrErrors, CDWordArray &arynAffectedDetailIDs);

	void SetOldAuditValue(CString strOldAuditValue);

	CString m_strNotes;	

	// (a.walling 2012-03-23 15:27) - PLID 49187
	long GetTemplateID()
	{
		return m_nTemplateID;
	}

	// (a.walling 2012-03-23 15:27) - PLID 49187
	long GetSafeTemplateID()
	{
		if (!this) return -1;
		return GetTemplateID();
	}

	long GetCollectionID();
	void SetCollectionID(long nCollectionID);

	//used to track whether details have been dragged between topics
	BOOL m_bDetailsHaveMoved;
	void SetDetailsHaveMoved(BOOL bDetailsHaveMoved);
	BOOL GetDetailsHaveMoved();

	// (j.jones 2007-01-11 10:22) - PLID 24027 - supported SourceDetailID
	// (z.manning 2009-03-04 14:24) - PLID 33338 - Use the new source action info class
	//TES 4/15/2010 - PLID 24692 - Added a parameter for the ID, used to update the topic position list.
	CEMRTopic* AddTopic(CString strTopicName, SourceActionInfo &sai, long nTopicID);
	CEMRTopic* AddTopic(CString strTopicName, long nTopicID);
	//TES 10/5/2009 - PLID 35755 - Added a parameter for whether this insertion should cause the topic order indexes
	// to be recalculated when saving.
	void InsertTopic(CEMRTopic *pTopicToInsert, CEMRTopic *pInsertBefore, BOOL bIsInitialLoad, BOOL bTopicArrayChanged);
	
	//TES 8/10/2010 - PLID 24692 - Gets the order index of the given topic within our list of topics (non-recursive).
	long GetChildOrderIndex(CEMRTopic *pChildTopic);

	int GetDiagCodeCount();
	EMNDiagCode* GetDiagCode(int nIndex);
	void AddDiagCode(EMNDiagCode *pCode);
	//TES 2/26/2014 - PLID 60807 - Added ICD10
	void RemoveDiagCode(long nICD9DiagID, long nICD10DiagID);
	// (a.walling 2007-08-06 10:57) - PLID 23714
	// (j.jones 2008-07-23 10:20) - PLID 30819 - changed to reference the diag code ID
	//TES 2/26/2014 - PLID 60807 - Added ICD10
	EMNDiagCode* GetDiagCodeByDiagID(long nICD9DiagCodeID, long nICD10DiagCodeID);
	//TES 2/28/2014 - PLID 61080 - Returns the index in m_aryDiagCodes, or -1
	long GetDiagCodeIndexByDiagID(long nICD9DiagCodeID, long nICD10DiagCodeID);
	//TES 2/26/2014 - PLID 60807 - A single ID might have multiple codes associated now, so this outputs an array
	void GetDiagCodesByDiagID(long nDiagCodeID, OUT CArray<EMNDiagCode*,EMNDiagCode*> &arCodes);

	int GetChargeCount();

	//DRT 1/11/2007 - PLID 24220 - Changed GetCharge to return a pointer, removed SetCharge
	EMNCharge* GetCharge(int nIndex);

	// (a.walling 2007-08-06 11:25) - PLID 23714
	EMNCharge* GetChargeByServiceID(long nServiceID);

	// (j.jones 2011-07-07 15:59) - PLID 38366 - TryUpdateCodingGroupByServiceID
	// will see if an EmrCodingGroup applies for the given ServiceID, then call
	// TryUpdateCodingGroup on the correct group. If the ServiceID is processed,
	// it will return TRUE, but if this function returns FALSE, the code was not
	// processed and still needs to be added normally.
	BOOL TryUpdateCodingGroupByServiceID(long nServiceID, BOOL bPopup, long nQuantityToIncrement = 1);
	// (j.jones 2011-07-07 15:59) - PLID 38366 - TryUpdateCodingGroup will update the tracked
	// quantity (usually by 1, but it can be negative), then process the group's rules for that
	// quantity, adding/removing the proper service code(s) as needed.
	void UpdateCodingGroup(CEmrCodingGroup *pCodingGroup, BOOL bPopup, long nQuantityToIncrement = 1);

	// (j.jones 2011-07-07 16:25) - PLID 38366 - ReflectCodingGroupCharges will take in a
	// given CEmrCodingGroup, CEmrCodingRange, and the current quantity in use for the group,
	// and update the charge list in the more info tab to display the correct charges for the
	// current range for the group at that quantity level
	// (z.manning 2011-07-11 14:29) - PLID 44469 - Changed quantity param to be a coding group info object
	void ReflectCodingRangeCharges(CEmrCodingGroup *pCodingGroup, CEmnCodingGroupInfo *pEmnCodingInfo, BOOL bPopup);

	// (z.manning 2011-07-12 16:29) - PLID 44469
	BOOL IsCptCodeInCodingGroup(const long nServiceID);

	void AddCharge(EMNCharge *pCharge);
	void AddLab(EMNLab* pLab); // (c.haag 2010-07-01 11:52) - PLID 36061
	// (j.jones 2007-01-11 10:22) - PLID 24027 - supported SourceDetailID
	// (j.jones 2008-06-04 16:20) - PLID 30255 - added nQuoteChargeID
	// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
	EMNCharge* AddCharge(EmrAction *pSourceAction, SourceActionInfo &sai);

	void RemoveCharge(EMNCharge *pCharge);
	void RemoveCharge(long nServiceID);

	int GetMedicationCount();
	// (a.walling 2007-10-01 08:43) - PLID 27568 - return a pointer to the medication
	EMNMedication* GetMedicationPtr(int nIndex);
	// (a.walling 2007-10-01 08:53) - PLID 27568 - This is now deprecated; use the pointer
	// from GetMedication
	//void SetMedication(int nIndex, IN EMNMedication em);
	void AddMedication(EMNMedication *pMedication);
	// (j.jones 2007-01-11 10:22) - PLID 24027 - supported SourceDetailID
	// (j.jones 2008-05-20 11:18) - PLID 30079 - added parameter to decide whether to show the prescription editor, and whether we checked allergies	
	EMNMedication* AddMedication(long nMedicationID, BOOL bShowPrescriptionEditor, BOOL bAlreadyCheckedAllergies, SourceActionInfo &sai);
	// (a.walling 2007-10-01 08:54) - PLID 27568 - Remove medications by pointer so there can be no ambiguity
	void RemoveMedication(EMNMedication* pMed);

	// (c.haag 2008-06-24 09:26) - PLID 17244 - Adds a todo alarm to data and creates an EMNTodo
	// object in memory for tracking purposes
	void AddTodo(const EmrAction& ea, SourceActionInfo &sai);
	// (c.haag 2008-06-24 09:39) - PLID 17244 - Takes a todo date (remind or deadline) and
	// increments it based on the type and interval of the todo spawner
	void MoveTodoDate(COleDateTime& dt, long nType, long nInterval);

	// (a.walling 2012-10-01 08:56) - PLID 52931 - Describes the template name and cache the associated procedure names and the owning collection
	struct DefaultMergeTemplate
	{
		CString templateName;
		CString procedureNames;
		CString collectionName;
	};

	// (a.walling 2012-10-01 08:56) - PLID 52931 - Queries or returns cached default merge template
	const DefaultMergeTemplate& GetDefaultMergeTemplate();

	// (a.walling 2012-10-01 08:56) - PLID 52931 - Manually set default merge template for this group
	void SetDefaultMergeTemplate(const CString& strTemplate);

	// (a.walling 2012-10-01 08:56) - PLID 52931 - Complex where clause used for datalists or other queries to get the default merge template
	CSqlFragment GetDefaultTemplateWhereClause();

	int GetProcedureCount();
	EMNProcedure* GetProcedure(int nIndex);
	void RemoveProcedure(long nProcedureID);
	// (a.walling 2007-08-06 10:57) - PLID 23714
	EMNProcedure* GetProcedureByID(long nID);

	// (z.manning 2008-10-24 12:56) - PLID 31628 - Removes a lab from the EMN
	void RemoveLab(const long nLabID);
	// (z.manning 2008-10-29 12:42) - PLID 31613 - Returns the lab object for the given ID
	EMNLab* GetLabByID(const long nLabID);

	int GetProviderCount();
	EMNProvider* GetProvider(int nIndex);
	void RemoveProvider(long nProviderID);
	// (a.walling 2007-08-06 10:57) - PLID 23714
	EMNProvider* GetProviderByID(long nID);

	// (j.gruber 2007-01-08 11:24) - PLID 23399 Secondary Providers
	int GetSecondaryProviderCount();
	EMNProvider* GetSecondaryProvider(int nIndex);
	void RemoveSecondaryProvider(long nProviderID);
	// (a.walling 2007-08-06 10:57) - PLID 23714
	EMNProvider* GetSecondaryProviderByID(long nID);

	// (d.lange 2011-03-23 09:04) - PLID 42136 - Assistant/Technician
	int GetTechnicianCount();
	EMNProvider* GetTechnician(int nIndex);

	// (j.gruber 2009-05-07 16:40) - PLID 33688 - Other Providers
	int GetOtherProviderCount();
	EMNProvider* GetOtherProvider(int nIndex);
	void RemoveOtherProvider(long nProviderID);
	EMNProvider* GetOtherProviderByID(long nID);

	// (z.manning 2009-03-04 15:59) - PLID 33338 - Replaced the calls to GetSource whatever with this
	SourceActionInfo GetSourceActionInfo();
	// (c.haag 2010-08-05 09:39) - PLID 39984 - Sets the source action info object. This function should
	// be called very sparingly and probably only during loading and initialization.
	void SetSourceActionInfo(const SourceActionInfo& sai) { m_sai = sai; }

	// (z.manning 2009-03-04 16:03) - PLID 33338 - Replaced the calls to SetSource<whatever> with this
	void ClearSourceActionInfo();
	
	void UpdateTableItemList();

	//TES 2/13/2006 - You can pass in TRUE for bRemovingItem, and the narratives will remove any fields related to pInfo.
	//TES 2/15/2007 - Josh added tables with linked details into this function long ago, but the fact that they aren't 
	// mentioned in the name of the function has caused confusion, so I'm renaming the function.
	//TES 1/23/2008 - PLID 24157 - I'm renaming this function from UpdateNarrativesAndLinkedTables to 
	// HandleDetailChange.  That reflects its real purpose, which is to do whatever needs to be done throughout 
	// the EMN to keep it in sync with whatever has changed about this individual detail.  That includes updating 
	// narratives, linked tables, and the multipopup dialog.  This name is still slightly misleading, as it also handles
	// changes of things that aren't details, but do appear on narratives, such as the EMN Date.  But as long as that's
	// understood, I still think this is the right name for the function.  While I was at it, I took out the never-used
	// bCreate parameter.
	//pInfo = the detail that changed (can be NULL, if what changed was a non-detail narrative field, such as the EMN date).
	// bRemovingItem = is pInfo about to be deleted from this EMN?
	void HandleDetailChange(CEMNDetail *pInfo, BOOL bRemovingItem = FALSE);

	//TES 4/11/2007 - PLID 25456 - This function is called by UpdateNarrativesAndLinkedTables, and updates a single narrative.
	//  I moved this into its own function to accommodate some changes being made in order to pop up narratives while spawning.
	//  It is the caller's responsibility to make sure that pNarrative is in fact a narrative.
	//TES 1/23/2008 - PLID 24157 - Took out bCreate, which is never actually used.
	void UpdateNarrative(CEMNDetail *pInfo, CEMNDetail *pNarrative, BOOL bRemovingItem);

	// (a.walling 2010-08-23 17:34) - PLID 37923
	void UpdateSpawningEMNTextMergeFieldNarratives(CStringArray& saErrors);


// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

	BOOL IsTemplate();

	//returns the "absolute index" of the given topic, necessary to uniquely identify topics that don't yet have an ID.
	int GetTopicIndex(CEMRTopic *pTopic);

	// (c.haag 2007-08-16 16:53) - PLID 27024 - This function creates an EMN loader object to only be used during spawning
	// (z.manning 2009-03-04 14:43) - PLID 33338 - Use the new source action info class
	CEMNLoader* CreateMintItemsLoader(const EmrAction& ea, CEMN *pEMN, SourceActionInfo &sai);

	// (j.jones 2010-02-12 14:19) - PLID 37318 - called whenever any detail is added to the EMN, to determine
	// whether the detail is a SmartStamp image or table, and links to its related item if it exists or
	// creates a new table if the image was jut added
	// (z.manning 2010-05-10 12:10) - PLID 38527 - Added bImageBeingSpawned
	void EnsureSmartStampLinks(CEMNDetail *pDetail, BOOL bImageBeingSpawned = FALSE);

	// (j.jones 2010-02-12 08:36) - PLID 37318 - called whenever a smart stamp image is added, creates the
	// linked table and returns a pointer to the detail representing that table
	CEMNDetail* PostSmartStampImageAdded(CEMNDetail *pImage);

	//TES 2/20/2006 - arSkippedTopics is an output array of all the topics that were spawned by this action, but were children
	//of topics that didn't exist.  CEMR is responsible for, once all actions have been processed, going through these skipped 
	//topics, and if their parents exist now, putting them there, and if not, putting them in an appropriate default location.
	// (c.haag 2007-08-06 10:31) - PLID 26954 - Added an optional parameter for preloaded spawning-related data
	// (c.haag 2008-06-17 10:12) - PLID 17842 - Added an optional parameter to allow the caller to track what was spawned
	// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
	void ProcessEmrAction(const EmrAction& ea, SourceActionInfo &sai, OUT CArray<SkippedTopic,SkippedTopic&> &arSkippedTopics, BOOL bIsInitialLoad, CProgressParameter *pProgress,
		CEMNSpawner* pEMNSpawner = NULL, CProcessEmrActionsResult* pEmrActionsResult = NULL);

	// (a.walling 2014-07-14 16:32) - PLID 62812 - use MFCArray

	//DRT 8/28/2007 - PLID 27218 - Split out of CEMN::ProcessEmrAction
	void ProcessEmrAction_eaoCpt(MFCArray<CActionAndSource> *paryActionsToSpawn, BOOL bIsInitialLoad, CProgressParameter *pProgress, CEMNSpawner* pEMNSpawner = NULL);
	void ProcessEmrAction_eaoDiagnosis(MFCArray<CActionAndSource> *paryActionsToSpawn, BOOL bIsInitialLoad, CProgressParameter *pProgress, CEMNSpawner* pEMNSpawner = NULL);
	// (j.jones 2013-01-09 09:58) - PLID 45446 - we now track the new prescription IDs
	void ProcessEmrAction_eaoMedication(MFCArray<CActionAndSource> *paryActionsToSpawn, BOOL bIsInitialLoad, CProgressParameter *pProgress, CEMNSpawner* pEMNSpawner = NULL, long *pnNewPrescriptionID = NULL);
	void ProcessEmrAction_eaoProcedure(MFCArray<CActionAndSource> *paryActionsToSpawn, BOOL bIsInitialLoad, CProgressParameter *pProgress, CEMNSpawner* pEMNSpawner = NULL);
	void ProcessEmrAction_eaoEmrItem(MFCArray<CActionAndSource> *paryActionsToSpawn, BOOL bIsInitialLoad, CProgressParameter *pProgress, CEMNSpawner* pEMNSpawner = NULL, CEMNDetail** ppNewDetail = NULL);
	// (c.haag 2008-06-17 09:53) - PLID 17842 - Added optional parameter for letting the caller get the newly added topics
	// (c.haag 2008-06-26 12:53) - PLID 27549 - Added pEMNParentMintLoader
	void ProcessEmrAction_eaoMintItems(MFCArray<CActionAndSource> *paryActionsToSpawn, OUT CArray<SkippedTopic,SkippedTopic&> &arSkippedTopics, BOOL bIsInitialLoad, CProgressParameter *pProgress,
		CEMNSpawner* pEMNSpawner = NULL, CArray<CEMRTopic*,CEMRTopic*>* papNewTopicsOut = NULL, CEMNLoader* pEMNParentMintLoader = NULL);
	// (c.haag 2008-06-24 08:48) - PLID 17244 - Spawn todo alarms
	void ProcessEmrAction_eaoTodo(MFCArray<CActionAndSource> *paryActionsToSpawn, BOOL bIsInitialLoad, CProgressParameter *pProgress, CEMNSpawner* pEMNSpawner = NULL);
	// (z.manning 2008-10-02 09:10) - PLID 21094 - Spawn labs
	void ProcessEmrAction_eaoLab(MFCArray<CActionAndSource> *paryActionsToSpawn, BOOL bIsInitialLoad, CProgressParameter *pProgress, CEMNSpawner* pEMNSpawner = NULL);

	// (c.haag 2008-07-21 12:06) - PLID 30725 - This function will spawn all problems for the given action onto
	// one or more EMR objects
	// (z.manning 2009-02-23 12:54) - PLID 33141 - Replaced source detail pointer with source action info class
	void ProcessEmrProblemActions(const EmrAction& ea, SourceActionInfo &sai, CEMNDetail* pNewDetail);
	void ProcessEmrProblemAction(const EmrAction& ea, const EmrProblemAction& epa, SourceActionInfo &sai, CEMNDetail* pNewDetail);
	// (c.haag 2008-07-21 12:18) - PLID 30725 - This function will associate a spawned service item with a problem.
	// This function expects that the service code is brand new and just spawned prior to this function call.
	void ProcessEmrProblemAction_eaoCpt(const EmrAction& ea, const EmrProblemAction& epa, SourceActionInfo &sai);
	// (c.haag 2008-07-21 12:33) - PLID 30725 - This function will associate a spawned diagnosis code with a problem.
	// This function expects that the diagnosis code is brand new and just spawned prior to this function call.
	// (b.savon 2014-07-23 15:19) - PLID 63013 - Rename
	void ProcessEmrProblemAction_eaoDiagnosis(const EmrAction& ea, const EmrProblemAction& epa, SourceActionInfo &sai);
	// (c.haag 2008-07-22 09:47) - PLID 30725 - This function will associate a spawned medication with a problem.
	// This function expects that the medication is brand new and just spawned prior to this function call.
	void ProcessEmrProblemAction_eaoMedication(const EmrAction& ea, const EmrProblemAction& epa, SourceActionInfo &sai);
	// (c.haag 2008-07-21 12:33) - PLID 30725 - This function will associate an EMR item with a problem. Items
	// themselves cannot spawn items; so we have to expect that pDestDetail was just spawned
	void ProcessEmrProblemAction_eaoEmrItem(const EmrProblemAction& epa, CEMNDetail* pDestDetail);
	// (c.haag 2008-07-21 13:51) - PLID 30725 - This function will associate an EMR data selection with a problem.
	// Selections themselves cannot be spawned, and the selection must be in the source detail
	void ProcessEmrProblemAction_eaoEmrDataItem(const EmrAction& ea, const EmrProblemAction& epa, CEMNDetail* pSourceDetail);
	// (z.manning 2011-11-14 14:56) - PLID 46231 - Added function for topic problem actions
	void ProcessEmrProblemAction_eaoMintItems(const EmrAction& ea, const EmrProblemAction& epa, SourceActionInfo &sai);

	// (a.walling 2014-07-14 16:32) - PLID 62812 - use MFCArray

	// (c.haag 2008-08-01 14:52) - PLID 30897 - This function gathers a list of all charge objects that will be removed
	// from an EMN as a result of one or more unspawning actions, and stores the charge object pointer and action-pertinent 
	// information in adeoDoomedObjects. That array will later be used in the actual removing of those charges.
	// (j.jones 2011-07-11 10:53) - PLID 38366 - this will also return an array of coding groups and the quantities to reduce
	// from each coding group
	void GetEmrObjectsToRevoke_eaoCpt(MFCArray<CActionAndSource> *paryActionsToRevoke, CDoomedEmrObjectAry& adeoDoomedObjects,
		CEmrCodingGroupArray &aryCodingGroupsToChange, CMap<CEmrCodingGroup*, CEmrCodingGroup*, long, long> &mapCodingGroupQuantitiesToSubtract);
	// (c.haag 2008-08-01 15:09) - PLID 30897 - This function gathers a list of all diagnosis codes that will be removed
	// from an EMN as a result of one or more unspawning actions, and stores the diagnosis object pointer and action-pertinent 
	// information in adeoDoomedObjects. That array will later be used in the actual removing of those diagnosis codes.
	void GetEmrObjectsToRevoke_eaoDiag(MFCArray<CActionAndSource> *paryActionsToRevoke, CDoomedEmrObjectAry& adeoDoomedObjects);
	// (c.haag 2008-08-01 15:15) - PLID 30897 - This function gathers a list of all medication objects that will be removed
	// from an EMN as a result of one or more unspawning actions, and stores the medication object pointer and action-pertinent 
	// information in adeoDoomedObjects. That array will later be used in the actual removing of those medications.
	void GetEmrObjectsToRevoke_eaoMedication(MFCArray<CActionAndSource> *paryActionsToRevoke, CDoomedEmrObjectAry& adeoDoomedObjects);
	// (c.haag 2008-08-01 15:20) - PLID 30897 - This function gathers a list of all procedure objects that will be removed
	// from an EMN as a result of one or more unspawning actions, and stores the procedure object pointer and action-pertinent 
	// information in adeoDoomedObjects. That array will later be used in the actual removing of those medications.
	void GetEmrObjectsToRevoke_eaoProcedure(MFCArray<CActionAndSource> *paryActionsToRevoke, CDoomedEmrObjectAry& adeoDoomedObjects);
	// (c.haag 2008-08-01 15:25) - PLID 30897 - This function gathers a list of all EMR items that will be removed
	// from an EMN as a result of one or more unspawning actions, and stores the EMN detail pointer and action-pertintent
	// information in adeoDoomedObjects. That array will later be used in the actual removing of those EMR items.
	void GetEmrObjectsToRevoke_eaoEmrItem(MFCArray<CActionAndSource> *paryActionsToRevoke, CDoomedEmrObjectAry& adeoDoomedObjects);
	// (c.haag 2008-08-01 15:34) - PLID 30897 - This function gathers a list of all EMR topics that will be removed
	// from an EMN as a result of one or more unspawning actions, and stores the EMN topic pointer and action-pertinent
	// information in adeoDoomedObjects. That array will later be used in the actual removing of those EMR items. If
	// a topic is removed and has children, its children will NOT be added to the array.
	void GetEmrObjectsToRevoke_eaoMintItems(MFCArray<CActionAndSource> *paryActionsToRevoke, CDoomedEmrObjectAry& adeoDoomedObjects);
	// (c.haag 2008-08-01 14:52) - PLID 30897 - This function gathers a list of all todo objects that will be removed
	// from an EMN as a result of one or more unspawning actions, and stores the todo ID and action-pertinent 
	// information in adeoDoomedObjects. That array will later be used in the actual removing of those todo alarms.
	void GetEmrObjectsToRevoke_eaoTodo(MFCArray<CActionAndSource> *paryActionsToRevoke, CDoomedEmrObjectAry& adeoDoomedObjects);
	// (z.manning 2008-10-08 17:32) - PLID 31628 - Ditto but for labs
	void GetEmrObjectsToRevoke_eaoLab(MFCArray<CActionAndSource> *paryActionsToRevoke, CDoomedEmrObjectAry& adeoDoomedObjects);

	// (c.haag 2008-08-07 10:15) - PLID 30979 - This function is now deprecated
	//void RevokeEmrAction(EmrAction ea, CEMNDetail *pSourceDetail);

	//DRT 8/7/2007 - PLID 27003 - Original source pulled out of RevokeEmrAction
	// (c.haag 2008-08-01 16:02) - PLID 30897 - These functions no longer searches for objects to delete. Those
	// objects are now passed into said functions function in adeoDoomedObjects
	// (j.jones 2011-07-11 10:53) - PLID 38366 - this will also process a given array of coding groups and the quantities to reduce
	// from each coding group, which were previously calculated
	void RevokeEmrActions_eaoCpt(const CDoomedEmrObjectAry& adeoDoomedObjects, CEmrCodingGroupArray &aryCodingGroupsToChange,
		CMap<CEmrCodingGroup*, CEmrCodingGroup*, long, long> &mapCodingGroupQuantitiesToSubtract);
	void RevokeEmrActions_eaoDiag(const CDoomedEmrObjectAry& adeoDoomedObjects);
	void RevokeEmrActions_eaoMedication(const CDoomedEmrObjectAry& adeoDoomedObjects);
	void RevokeEmrActions_eaoProcedure(const CDoomedEmrObjectAry& adeoDoomedObjects);
	void RevokeEmrActions_eaoEmrItem(const CDoomedEmrObjectAry& adeoDoomedObjects);
	void RevokeEmrActions_eaoMintItems(const CDoomedEmrObjectAry& adeoDoomedObjects);
	// (c.haag 2008-06-24 10:48) - PLID 17244 - Revoke todo actions
	void RevokeEmrActions_eaoTodo(const CDoomedEmrObjectAry& adeoDoomedObjects);
	// (z.manning 2008-10-08 16:46) - PLID 31628 - Revoke labs
	void RevokeEmrActions_eaoLab(const CDoomedEmrObjectAry& adeoDoomedObjects);

	// (c.haag 2008-08-07 09:55) - PLID 30979 - Given an array of actions and sources, unspawn all the actions.
	// This should only be called when someone has deleted data actions when editing an EMN template on the fly.
	void RevokeEmrActions(const MFCArray<CActionAndSource>& aryaas);
	void RevokeDeletedActions();
	void RevokeDeletedActionsForInfoID(long nEMRInfoID);

	// (c.haag 2008-07-21 17:50) - PLID 30799 - This will revoke all problems spawned through the given action that are
	// bound to the given source detail or a list selection within it.
	void RevokeEmrProblemAction(const EmrAction& ea, CEMNDetail* pSourceDetail);

	//After there are no more actions to process, this will be called, to make sure the topic gets put somewhere.
	// (a.walling 2007-10-26 09:16) - PLID 25548 - Made this return a pointer to the new topic
	CEMRTopic* ProcessSkippedTopic(CEMNDetail *pSourceDetail, SkippedTopic st);

	// (z.manning 2011-11-03 16:11) - PLID 42765 - Function to return an array of spawned details based on the 
	// give source action information.
	// (a.walling 2012-08-16 08:18) - PLID 52163 - No longer returning a bool, simply adds to the array when a detail matching pSpawningInfo is found
	void GetSpawnedDetails(IN CSpawningSourceInfo *pSpawningInfo, OUT CEMNDetailArray *parypSpawnedDetails, CEMNDetail* pStopAt = NULL);
	// (z.manning 2011-11-07 17:12) - PLID 46309 - Added param for the separator
	CString GetSpawnedDetailsSentenceFormats(IN CSpawningSourceInfo *pSpawningInfo, LPCTSTR strSeparator, const bool bHtml);

	//Sets the window that is the "interface" for this EMN, and will receive messages when it changes.
	// (a.walling 2011-10-20 14:23) - PLID 46075 - Clean up interaction with external interfaces
	void SetInterface(class CEmrTreeWnd* pEmrTreeWnd);
	class CEmrTreeWnd* GetInterface() const
	{
		return m_pInterface;
	}

	//TES 12/26/2006 - PLID 23400 - There can now be multiple provider IDs.
	//TES 12/28/2006 - PLID 23400 - Also, since everywhere that called this immediately set the results as the providers for
	// this EMN, I changed this function to just go ahead and set it itself, rather than just calculating.
	void LoadDefaultProviderIDs();

	// (d.lange 2011-03-24 10:23) - PLID 42987 - If the current user is an Assistant/Technician lets auto-fill the field on More Info
	void SetCurrentUserTechnicianID();

	// (z.manning, 05/07/2007) - PLID 25925 - We track location name now as well.
	// (a.walling 2008-07-01 17:29) - PLID 30586 - And logo
	// (a.walling 2010-10-29 10:33) - PLID 31435 - And logo width
	void SetLocation(long nLocationID, CString strLocationName, CString strLogoPath, long nLogoWidth);
	long GetLocationID();
	// (a.walling 2007-08-06 11:20) - PLID 23714
	// (a.walling 2012-04-09 09:03) - PLID 49515 - Location - Return const ref
	const EMNLocation& GetLocation()
	{
		return m_Location;
	}

	void SetCategory(long nCategoryID, CString strCategoryName);
	const EMNCategory& GetCategory()
	{
		return m_Category;
	}

	// (z.manning, 04/11/2007) - PLID 25569
	// (z.manning 2011-05-19 17:59) - PLID 33114 - SetChart is now returns a bool indicating whether or not it was successful.
	// Also added a bool for whether or not the chart permission check should be silent.
	BOOL SetChart(long nChartID, CString strChartName, BOOL bSlient = TRUE);
	const EMNChart& GetChart()
	{
		return m_Chart;
	}

	// (a.walling 2013-01-16 13:04) - PLID 54650 - Appointment linked with this EMN
	BOOL SetAppointment(const EMNAppointment& appt);
	const EMNAppointment& GetAppointment()
	{ 
		return m_Appointment; 
	}

	// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
	void SetDischargeStatus(long nStatusID, CString strCode, CString strDescription);
	void SetAdmissionTime(COleDateTime dtDate);
	void SetDischargeTime(COleDateTime dtDate);
	const EMNDischargeStatus& GetDischargeStatus()
	{
		return m_DischargeStatus;
	}
	const COleDateTime& GetAdmissionTime() const
	{
		return m_dtAdmissionTime;
	}
	const COleDateTime& GetDischargeTime() const
	{
		return m_dtDischargeTime;
	}

	// (d.thompson 2009-05-08) - PLID 29909 - Added ConfidentialInfo field
	void SetConfidentialInfo(CString strNewData);
	CString GetConfidentialInfo();

	// (c.haag 2009-08-10 11:18) - PLID 29160 - Added bMergeToPrinter
	void Merge(CString strTemplateFileName, BOOL bSaveInHistory, BOOL bMergeToPrinter);
	void EditWordTemplates(HWND hwnd, CString strDefaultFile = "", CString strDefaultPath = "");

	//Returns whether there are unsaved changes on this EMN.
	BOOL IsUnsaved();

	// (j.jones 2011-07-15 13:45) - PLID 42111 - takes in an image file name (could be a path),
	// and returns TRUE if any Image detail on this EMN references it
	BOOL IsImageFileInUse(const CString strFileName);

	// (a.walling 2008-06-27 15:10) - PLID 30482
	void SetParentEMRChanged();

	//Returns whether the "More Info" information for this EMN is unsaved (if this is true, IsUnsaved() will always also be true).
	BOOL IsMoreInfoUnsaved()
	{
		return m_bMoreInfoUnsaved;
	}

	// (j.jones 2007-01-05 11:41) - PLID 24070 - sets the more info topic as unsaved
	// (a.walling 2012-03-22 16:50) - PLID 49141 - This also calls NotifyMoreInfoChanged to notify the interface
	void SetMoreInfoUnsaved();

	// (a.walling 2012-03-22 16:50) - PLID 49141 - This will post a message to the interface window, if we have one
	void NotifyMoreInfoChanged();

	//TES 2/12/2014 - PLID 60470 - Added m_bCodesUnsaved
	BOOL IsCodesUnsaved()
	{
		return m_bCodesUnsaved;
	}

	void SetCodesUnsaved();
	void NotifyCodesChanged();

	//Tells this EMN that it has been saved.
	void SetSaved(BOOL bIsPostLoad = FALSE);

	// (j.jones 2008-07-23 15:07) - PLID 30789 - added ability to mark unsaved
	void SetUnsaved();

	//Tells this EMN that it is now new, when it saves it will INSERT, not UPDATE.
	// (a.walling 2007-09-27 14:02) - PLID 25548 - Option to retain the order index for topics
	void SetNew(BOOL bRetainOrderIndex = FALSE);

	// (j.jones 2007-01-23 09:14) - PLID 24027 - reassigns source detail IDs/pointers due to an EMN copy
	//TES 5/20/2014 - PLID 52705 - I added bResetIDs, so this function knows whether or not it should set everything to -1
	void UpdateSourceDetailsFromCopy(bool bResetIDs);

	// (z.manning 2010-03-11 14:56) - PLID 37571 - Will reassasign the source detail stamp pointer to the given
	// new pointer for any object in this EMN that could have potentially been spawned.
	// (a.walling 2010-03-31 14:46) - PLID 38009 - Pass in the source EMN; if we are not the source EMN, we do not need to recurse into topics
	void UpdateSourceDetailStampPointers(EmrDetailImageStamp *pDetailStampOld, EmrDetailImageStamp *pDetailStampNew, CEMN* pSourceEMN);

	// (j.jones 2007-01-23 11:00) - PLID 24027 - update the source details such that their pointers are set
	// and if bClearEraseSourceDetailID is TRUE, then also clear their detail IDs
	void UpdateSourceDetailPointers(BOOL bClearEraseSourceDetailID);

	//Called by a child topic when its loading is complete.
	void PostTopicLoad(CEMRTopic *pTopicLoaded);

	//The initial load of narratives doesn't include list items.  If they need a specific list item, they will ask us for it
	//by calling this function.
	// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated
	//void HandleNarrativeFieldRequest(const CString &strRequestedField, CEMNDetail *pNarrative);

	// (j.jones 2007-08-30 09:29) - PLID 27243 - Office Visit incrementing is no longer
	// used in the L2 EMR, it's in Custom Records only
	//void TryToIncrementOfficeVisit();
	//void CheckWarnOfficeVisitChanged();

	// (j.jones 2008-07-21 17:28) - PLID 30729 - add all of this EMN's problems,
	// its topics' problems, and problems on diagnoses and charges, to the passed-in array
	// (c.haag 2008-08-14 12:05) - PLID 30820 - Added bIncludeDeletedProblems
	void GetAllProblems(CArray<CEmrProblem*, CEmrProblem*> &aryProblems, BOOL bIncludeDeletedProblems = FALSE);

	// (j.jones 2009-05-21 15:58) - PLID 34325 - recurses through all children and returns all problem links within the entire EMN,
	// or just the problem links that reference pFilterProblem, if pFilterProblem is not NULL
	// Links that have been deleted are not returned by default, unless bIncludeDeletedLinks is TRUE
	void GetAllProblemLinks(CArray<CEmrProblemLink*, CEmrProblemLink*> &aryProblemLinks, CEmrProblem *pFilterProblem = NULL, BOOL bIncludeDeletedLinks = FALSE);

	// (j.jones 2008-07-22 10:11) - PLID 30789 - returns true if there are any undeleted problems on the EMN
	BOOL HasProblems();
	// (j.jones 2008-07-22 10:11) - PLID 30789 - returns true if there are only undeleted, closed problems on the EMN
	BOOL HasOnlyClosedProblems();
	// (j.jones 2008-07-23 15:19) - PLID 30789 - returns true if any problems are marked as modified,
	// including deleted items
	BOOL HasChangedProblems();
	// (c.haag 2008-07-24 09:49) - PLID 30826 - Returns TRUE if there is at least one saved problem for this EMN or any of
	// its children. This does not check deleted EMR objects.
	BOOL DoesEmnOrChildrenHaveSavedProblems();

	// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
	// Safely checks all topics and subtopics of this EMN to find if there are any visible details that are marked as required and aren't filled in.
	BOOL HasVisibleUnfilledRequiredDetails();
	
	// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
	// Static because it checks against data for the specified EMN, not any EMN loaded into memory.
	// Checks for the existence of any visible details belonging to the specified EMN that are marked as required and aren't filled in.
	static BOOL HasVisibleUnfilledRequiredDetails(long nEMNID);

	// (z.manning, 07/11/2006) - PLID 19291 - Rather than calling UpdateNarrative for every single
	// action that gets processed/revoked, it now prevents UpdateNarrative from working, instead
	// just reloading all narratives once the actions have all finished processing.
	//TES 2/15/2007 - PLID 24717 - Since UpdateNarratives() also updates tables with linked details, these functions need to
	// as well.  I added that, and also renamed the functions to avoid confusion in the future.
	//TES 1/23/2008 - PLID 24157 - Renamed UpdateNarrativesAndLinkedTables to HandleDetailChange, and updated these
	// function names accordingly.
	void LockHandlingDetailChanges();
	void UnlockHandlingDetailChanges();

	// (m.hancock 2006-10-06 16:34) - PLID 22302 - Audit when we close an EMN that has not been saved.
	void AuditCloseUnsaved();
	// (m.hancock 2006-11-28 16:37) - PLID 22302 - Audit when we close (without saving) an EMN that was marked for deletion.
	void AuditCloseUnsavedDeleted();

	// (j.jones 2007-09-11 17:41) - PLID 27243 - Office Visit incrementing is no longer
	// used in the L2 EMR, it's in Custom Records only
	//BOOL m_bAlreadyDeclinedOfficeVisitUpgrade;

	//TES 1/23/2007 - PLID 24377 - Used when processing EMR Links, this function returns TRUE if any one of the EmrDataT.IDs
	// in the array is selected on any detail in the EMN, otherwise it returns FALSE.
	//TES 2/6/2007 - PLID 24377 - arDetailsToIgnore is passed in when processing a batch of details, which we don't want
	// to link to each other.  The function will only return TRUE if one of the data IDs is checked on a detail that is NOT
	// in arDetailsToIgnore.
	BOOL IsAnyItemChecked(const CArray<long,long> &arDataIDs, const CArray<CEMNDetail*,CEMNDetail*> &arDetailsToIgnore);

	// (c.haag 2011-05-19) - PLID 43696 - Populates a map with all EmrDataID's that correspond to checked-off single-select
	// and multi-select list items. All details in mapDetailsToIgnore are ignored during the search.
	void GetAllCheckedItems(CMap<long,long,BOOL,BOOL>& mapDataIDs, const CMap<CEMNDetail*,CEMNDetail*,BOOL,BOOL> &mapDetailsToIgnore);

	// (c.haag 2007-01-23 11:40) - PLID 24376 - TRUE if we are traversing
	// the EMR for Current Medication details and making sure all of the states are the same
	BOOL m_bUpdatingCurrentMedicationDetails;

	// (c.haag 2007-04-05 11:26) - PLID 25516 - TRUE if we are traversing
	// the EMR for official Allergies details and making sure all of the states are the same
	BOOL m_bUpdatingAllergiesDetails;

	// (c.haag 2007-01-23 11:21) - PLID 24376 - Called when an official Current Medications
	// detail is changed. Details are in EMN.cpp
	void ProcessCurrentMedicationsChange(CEMNDetail* pSourceDetail);
	BOOL GetUpdatingCurrentMedicationDetails() const;
	void SetUpdatingCurrentMedicationDetails(BOOL bProcessing);

	// (c.haag 2009-03-05 17:07) - PLID 33367 - This is a utility function used in 
	// ApplyOfficialAllergies and ApplyOfficialCurrentMedications to efficiently
	// populate the state of a system table with values.
	// (j.jones 2011-05-04 14:44) - PLID 43527 - added ptrmapCurMedDataIDsToSig, which is NULL
	// when an allergies table, and a pointer to a map of data IDs to a current medications Sig
	// when this is the current medications table
	void BuildSystemTableState(const CArray<long,long>& aY,
									 const CArray<long,long>& aYSortOrder,
									 const CArray<long,long>& anCurDataIDs,
									 CEMNDetail* pDetail, 
									 CMap<long, long, CString, LPCTSTR> *ptrmapCurMedDataIDsToSig);

	// (c.haag 2007-01-29 09:40) - PLID 24376 - Call this function to get the
	// official current medications state for this EMN
	//TES 6/5/2008 - PLID 29416 - Changed the name of this function, there is no "Official State" for medications or allergies,
	// the only thing that's "official" is the first column (reason #2,982 this was a bad design).  This takes a detail whose
	// state has been set, and applies the official Current Medications to the first column.
	// (j.jones 2009-09-18 13:00) - PLID 35599 - added bUpdateExisting, if TRUE it means we want to load even on details
	// that have already been saved, which should be rare and only with a really good reason
	void ApplyOfficialCurrentMedications(CEMNDetail* pDetail, long nCurMedEmrInfoID, BOOL bUpdateExisting = FALSE, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
	// (c.haag 2009-05-15 10:02) - PLID 34271 - This function takes a given official current medications detail and synchronizes
	// the "Selected column" of its state with the patient's official current medications list as specified in anCurMeds
	// (j.jones 2009-09-18 13:00) - PLID 35599 - added bUpdateExisting, if TRUE it means we want to load even on details
	// that have already been saved, which should be rare and only with a really good reason
	// (j.jones 2011-05-04 14:09) - PLID 43527 - added mapDataIDsToSig, which tracks the Sig for each current medication
	void ApplyOfficialCurrentMedications(CEMNDetail* pDetail, long nCurMedEmrInfoID,
		const CArray<long,long>& anCurMeds,
		CMap<long, long, CString, LPCTSTR> &mapDataIDsToSig,
		BOOL bUpdateExisting = FALSE,
		OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	// (c.haag 2007-01-25 17:45) - PLID 24376 - This will traverse the EMN
	// updating the Current Medication details with the given Current Medications
	// state
	void UpdateAllMedicationListDetails(const _variant_t& varState,
		long nEmrInfoID, CEMNDetail* pDetailToIgnore);

	// (c.haag 2007-04-05 11:38) - PLID 25516 - Called when an official Allergies
	// detail is changed. Details are in EMN.cpp
	void ProcessAllergiesChange(CEMNDetail* pSourceDetail);
	BOOL GetUpdatingAllergiesDetails() const;
	void SetUpdatingAllergiesDetails(BOOL bProcessing);

	// (c.haag 2007-04-05 11:34) - PLID 25516 - Call this function to get the
	// official allergies state for this EMN.
	//TES 6/5/2008 - PLID 29416 - Like with CurrentMedications(), renamed (and redesigned) this function to take an existing
	// state, and modify just the first column based on Medications tab data.
	// (j.jones 2009-09-18 13:00) - PLID 35599 - added bUpdateExisting, if TRUE it means we want to load even on details
	// that have already been saved, which should be rare and only with a really good reason
	void ApplyOfficialAllergies(CEMNDetail* pDetailToIgnore, long nAllergiesEmrInfoID, BOOL bUpdateExisting = FALSE, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
	// (c.haag 2009-05-14 16:50) - PLID 34256 - This function takes a given official allergy detail and synchronizes the 
	// "Selected column" of its state with the patient's official allergy list as specified in anAllergies.
	// (j.jones 2009-09-18 13:00) - PLID 35599 - added bUpdateExisting, if TRUE it means we want to load even on details
	// that have already been saved, which should be rare and only with a really good reason
	void ApplyOfficialAllergies(CEMNDetail* pDetail, long nAllergiesEmrInfoID,
		const CArray<long,long>& anAllergies, BOOL bUpdateExisting = FALSE, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	// (c.haag 2007-04-05 11:44) - PLID 25516 - This function looks for all the
	// details on an EMN based on the nAllergies info ID (these are Allergies
	// table details) and updates their state to varState. This makes it possible
	// for changes in one Allergies detail to propagate to all of the details
	void UpdateAllAllergiesListDetails(const _variant_t& varState,
		long nAllergiesEmrInfoID, CEMNDetail* pDetailToIgnore);


	//TES 6/4/2008 - PLID 30196 - Does this EMN have anywhere on it an (undeleted) system table of the specified type?
	BOOL HasSystemTable(EmrInfoSubType eistType);

	// (j.jones 2007-01-29 13:28) - PLID 24353 - allow to set/get StartEditingTime
	void TryStartTrackingTime();	
	void TryStopTrackingTime();
	COleDateTime GetStartEditingTime();

	// (j.jones 2012-01-31 15:13) - PLID 47878 - HasChangesMadeThisSession will return the
	// value of m_bChangesMadeThisSession for this EMN.
	BOOL HasChangesMadeThisSession();

	// (j.jones 2007-09-17 17:49) - PLID 27396 - set the total time this EMN has been opened in previous sessions,
	// not counting the current session
	void SetTotalTimeOpened_PreviousSessions(COleDateTimeSpan dtTotalTimeOpened_PreviousSessions);
	void SetTotalTimeOpened_PreviousSessions(long nSeconds);

	// (j.jones 2007-09-17 17:49) - PLID 27396 - return the total time this EMN has been opened, in previous sessions AND this session
	COleDateTimeSpan GetTotalTimeSpanOpened();

	// (j.jones 2007-03-13 17:33) - PLID 25193 - return m_bInitialLoadPendingCompletion
	BOOL GetIsInitialLoadPendingCompletion();

	// (j.jones 2007-03-13 17:36) - PLID 25193 - called when the initial load has truly completed
	void PostInitialLoad();
	// (z.manning 2008-12-12 14:18) - PLID 32427 - This will tell us if we need to call PostInitalLoad
	// because it's pending and both spawning and handling detail changes is unlocked.
	BOOL NeedToCallPostInitialLoad();

	//TES 4/15/2010 - PLID 24692 - Functions for working with our linked list of topic positions.
	//TES 4/15/2010 - PLID 24692 - Get the entry in our linked list that corresponds to this ID (NULL if there isn't any).
	TopicPositionEntry* GetTopicPositionEntryByID(long nTopicID);

	//TES 3/18/2011 - PLID 42762 - Fills a GlassesOrder struct with any associated fields found in the EMN.  Will return TRUE if any values
	// were filled, otherwise FALSE.  strIgnoredData will be a user-friendly description of all the items that had data which could not
	// be included (because it was already there, if for example this EMN has two OD Spheres, the first will be added to goOrder, and 
	// the second will be desccribed in strIgnoredData).
	BOOL GetGlassesOrderData(OUT GlassesOrder &goOrder, OUT CString &strIgnoredData);

	//TES 4/11/2012 - PLID 49621 - Added, gathers any information on that could be loaded into a Contact Lens Order.  Same as GetGlassesOrderData
	BOOL GetContactLensOrderData(OUT ContactLensOrder &cloOrder, OUT CString &strIgnoredData);

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

public:
	// (c.haag 2007-10-30 17:29) - PLID 27914 - Returns TRUE if the destructor of this object is present in the call stack
	inline BOOL GetInDestructor() const { return m_bInDestructor; }

	// (a.walling 2012-07-09 12:35) - PLID 51441 - Reference to topic array
	CTopicArray& GetSubtopics()
	{
		return m_arypEMRTopics;
	}

	// (a.walling 2012-07-09 12:35) - PLID 51441 - Get next / prev EMN
	CEMN* GetNextEMN();
	CEMN* GetPrevEMN();

protected:	
	// (a.walling 2010-10-18 17:07) - PLID 40260 - Allow using a default connection
	ADODB::_ConnectionPtr m_pDefaultConnection;

	// (c.haag 2007-10-30 17:26) - PLID 27914 - TRUE if the destructor of this object is present in the call stack
	BOOL m_bInDestructor;

	CEMR* m_pParentEMR;
	//If we own m_pParentEMR (we created it and must destroy it), this is TRUE.
	BOOL m_bOwnParentEMR;
	//If we own our child topics (are responsible for destroying them), this is TRUE.
	BOOL m_bOwnChildren;
	
	//if m_bIsTemplate, then this will be an EmrTemplatesT record.  Otherwise, this will be an EmrMasterT record.
	long m_nID;
	BOOL m_bIsTemplate;

	COleDateTime m_dtEMNDate;
	// (a.walling 2012-06-07 08:53) - PLID 50920 - Dates - Modified, Created
	COleDateTime m_dtEMNModifiedDate;
	COleDateTime m_dtEMNInputDate;

	// (z.manning 2010-01-13 10:40) - PLID 22672 - Age is now a string
	CString m_strPatientAge;
	BYTE m_cbPatientGender;

	// (j.jones 2007-08-24 08:40) - PLID 27054 - added VisitTypeID
	long m_nVisitTypeID;
	CString m_strVisitTypeName;

	//TES 1/17/2014 - PLID 60397 - Option to hide the EMN title on the preview pane, per template
	bool m_bHideTitleOnPreview;

	// (z.manning 2009-03-04 14:26) - PLID 33338 - Use the new source aciton info class
	SourceActionInfo m_sai;

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

	// (c.haag 2007-03-13 12:34) - PLID 25239 - We now maintain the array of preloaded details
	CEMNLoader* m_pLoader;

	// (c.haag 2007-09-10 11:49) - PLID 27024 - When the initial load from a template is done, we move
	// m_pLoader to here. The reason for that is because m_pLoader contains information that can be used
	// to optimize the act of spawning items from templates.
	CEMNLoader* m_pRetiredTemplateLoader;
	
	//m.hancock - 3/14/2006 - 19579 - Patient demographics shouldn't change after the EMN is locked.
	//Necessary fields to store a patient's name at the time the EMR was closed, locked, or finished
	CString m_strPatNameFirst;
	CString m_strPatNameMiddle;
	CString m_strPatNameLast;

	CString m_strDescription;

	// (z.manning, 06/27/2006) - PLID 20896 - Used in case we want to show more information when
	// auditing the creation of an EMN (e.g. that it was copied from another EMN).
	CString m_strOldAuditValue;

	//TES 4/15/2010 - PLID 24692 - Our linked list of topic positions.
	TopicPositionEntry *m_pTopicPositionHead;

	CArray<CEMRTopic*, CEMRTopic*> m_arypEMRTopics;
	BOOL m_bTopicArrayChanged; //Set to TRUE whenever the order is changed, or any are added or removed.
	CArray<CEMRTopic*, CEMRTopic*> m_arypDeletedTopics;

	CArray<CEMRTopic*, CEMRTopic*> m_arypOtherSavedTopics;

	BOOL m_bNeedRegenerateTableItemList;
	CString m_strTableItemList;

	EMNLocation m_Location;

	// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
	EMNDischargeStatus m_DischargeStatus;
	COleDateTime m_dtAdmissionTime;
	COleDateTime m_dtDischargeTime;

	//EmrTemplateT.ID, for templates this will be identical to m_nID, for EMNs this will be EmrMasterT.TemplateID
	long m_nTemplateID;
	//0 = Active, 1 = Finished, 2 = Locked.  If m_bIsTemplate, will always be 0.
	// (j.jones 2011-07-05 11:21) - PLID 43603 - changed to a class to store the name as well
	EMNStatus m_Status;
	long m_nEMRCollectionID;

	boost::optional<DefaultMergeTemplate> m_defaultMergeTemplate;

	// (z.manning 2009-11-19 08:57) - PLID 35810 - Patient created status
	// - 0 = normal (created by practice, not the patient)
	// - 1 = patient owned (created by the patient, not yet finalized)
	// - 2 = practice owned (created by the patient, finalized)
	BYTE m_nPatientCreatedStatus;

	// (b.savon 2014-09-10 09:36) - PLID 62712 - Handle the ToDo creation preference when spawning diagnosis tuples in a different search mode
	MFCArray<EmrAction> m_aryDiagnosisActionsHeldForPostSaveProcessing; // Array for post processing
	void ProcessSpawnedDiagnosisForTodoCreationInMisalignedModes(const EmrAction &ea);

public:
	enum EPatientCreatedStatus {
		pcsInvalid = -1,
		pcsCreatedByPractice = 0,
		pcsCreatedByPatientNotFinalized = 1,
		pcsCreatedByPatientFinalized = 2,
	};

	// (z.manning 2009-11-19 09:20) - PLID 35810
	//(e.lally 2011-12-14) PLID 46968 - Moved this to be after the member variable and enum declarations
	BYTE GetPatientCreatedStatus();

	//(e.lally 2011-12-14) PLID 46968
	void SetPatientCreatedStatus(CEMN::EPatientCreatedStatus ePatientCreatedStatus);

	// (j.jones 2014-12-23 15:40) - PLID 64491 - exposed this publicly
	void AddDeletedDiagCode(EMNDiagCode *pDeletedCode);

protected:
	//TES 1/23/2008 - PLID 24157 - Changed UpdateNarrativesAndLinkedTables to HandleDetailChange, and updated these
	// associated member variables correspondingly.  
	long m_nDetailChangeLocks;
	//TES 2/15/2007 - PLID 24717 - We need to store the updates we've been asked to process while updating was locked, 
	// so we can process them efficiently when updating gets unlocked,
	//TES 1/23/2008 - PLID 24157 - To be clear: these represent details that have changed, what is pending is handling
	// that change (by updating narratives, for example).
	struct PendingChangedDetail
	{
		CEMNDetail *pDetail;
		//TES 1/23/2008 - PLID 24157 - bCreate was never set to TRUE.
		//BOOL bCreate;
		BOOL bRemovingItem;
	};
	CArray<PendingChangedDetail,PendingChangedDetail&> m_arPendingChangedDetails;

	// (a.wetta 2007-01-09 09:57) - PLID 14635 - Store the EMN category ID of a new EMN; to be used with the tabbed view in the EMR
	// (z.manning, 04/11/2007) - PLID 25569 - It now stores the category ID of any EMN.
	// (z.manning, 05/07/2007) - PLID 25731 - Now a class object so we can track the name as well.
	EMNCategory m_Category;

	// (z.manning, 04/11/2007) - PLID 25569 - Stores the chart ID of the EMN.
	// (z.manning, 05/07/2007) - PLID 25731 - Now a class object so we can track the name as well.
	EMNChart m_Chart;

	// (a.walling 2013-01-16 13:04) - PLID 54650 - Appointment linked with this EMN
	EMNAppointment m_Appointment;

	// (d.thompson 2009-05-08) - PLID 29909 - Confidential patient information, stored in the more info.  Also a flag
	//	for changed status.  The flag is used for auditing.  We don't want to audit the past value, as it is
	//	confidential.
	CString m_strConfidentialInfo;
	bool m_bConfidentialInfoChanged;

	//Not accessible from outside, only used when copying templates.
	BOOL m_bAddOnce;

	//TES 11/2/2009 - PLID 35808 - Called only by IsNexWebTemplate().
	//(e.lally 2011-05-04) PLID 43537 - Now a boolean flag for visible in NexWeb.
	BOOL m_bNexWebVisible;

	// (j.gruber 2012-08-31 14:42) - PLID 52285
	//called only by IsOMRTemplate
	BOOL m_bIsOMRTemplate;

	// (j.jones 2007-01-29 13:28) - PLID 24353 - track the StartEditingTime
	BOOL m_bIsTrackingTime;
	COleDateTime m_dtStartEditingTime;
	BOOL m_bChangesMadeThisSession;
	// (j.jones 2007-09-17 17:49) - PLID 27396 - track the total time this EMN has been opened in previous sessions,
	// not counting the current session
	COleDateTimeSpan m_dtTotalTimeOpened_PreviousSessions;

	// (c.haag 2010-07-07 12:49) - PLID 38886 - This array tracks all of the pEMNMintLoader objects for
	// every call to ProcessEmrAction_eaoMintItems in the call stack. The purpose of this is to make it
	// possible for the currently spawning template to search all previously spawned templates in the same
	// spawn chain for user-defined places to spawn topics.
	CArray<CEMNLoader**,CEMNLoader**> m_apeaoMintLoaderStack;

	// (c.haag 2008-07-23 13:40) - PLID 30820 - Deleted charges, diagnosis codes and medications are now tracked with
	// the original objects, not ID's.
	CArray<EMNProcedure*,EMNProcedure*> m_aryProcedures;
	//TES 2/8/2012 - PLID 19441 - Also procedures (not sure why that wasn't true before)
	CArray<EMNProcedure*,EMNProcedure*> m_aryDeletedProcedures;
	CArray<EMNDiagCode*,EMNDiagCode*> m_aryDiagCodes;// (j.jones 2012-12-31 13:54) - PLID 
	CArray<EMNDiagCode*,EMNDiagCode*> m_aryDeletedDiagCodes;
	
	CEMNChargeArray m_aryCharges;
	CEMNChargeArray m_aryDeletedCharges;
	//TES 6/15/2012 - PLID 50983 - Added, use this to determine when charges have been modified, rather than comparing against the database
	CEMNChargeArray m_aryOriginalCharges;

	// (j.jones 2011-07-14 08:37) - PLID 44509 - added maps for auditing charges by coding group
	CMap<long, long, long, long> m_mapChargeIDsTo_RemovedByCodingGroupID;
	CMap<long, long, long, long> m_mapChargeIDsTo_ModifiedByCodingGroupID;
	CMap<EMNCharge*, EMNCharge*, long, long> m_mapChargesTo_CreatedByCodingGroupID;

	CArray<EMNMedication*,EMNMedication*> m_aryMedications;
	CArray<EMNMedication*,EMNMedication*> m_aryDeletedMedications;
	CArray<EMNProvider*,EMNProvider*> m_aryProviders;
	//TES 2/8/2012 - PLID 19441 - Changed from a CArray<long,long>
	CArray<EMNProvider*,EMNProvider*> m_aryDeletedProviders;
	CArray<EMNLab*,EMNLab*> m_aryLabs; // (z.manning 2008-10-06 11:16) - PLID 21094

	// (j.gruber 2007-01-08 11:25) - PLID 23399 - Secondary Providers
	CArray<EMNProvider*,EMNProvider*> m_arySecondaryProviders;
	//TES 2/8/2012 - PLID 19441 - Changed from a CArray<long,long>
	CArray<EMNProvider*,EMNProvider*> m_aryDeletedSecondaryProviders;

	// (d.lange 2011-03-23 09:04) - PLID 42136 - Assistant/Technician
	CArray<EMNProvider*,EMNProvider*> m_aryTechnicians;
	//TES 2/8/2012 - PLID 19441 - Changed from a CArray<long,long>
	CArray<EMNProvider*,EMNProvider*> m_aryDeletedTechnicians;

	// (j.gruber 2009-05-07 16:41) - PLID 33688 - Other Providers
	CArray<EMNProvider*, EMNProvider*> m_aryOtherProviders;
	CArray<EMNProvider*, EMNProvider*> m_aryDeletedOtherProviders;

	// (z.manning 2011-07-07 13:04) - PLID 44469
	CEmnCodingGroupInfoArray m_arypCodingGroups;

	//TES 7/10/2009 - PLID 25154 - Option to send bill to HL7
	bool m_bSendBillToHL7;
	//(a.wilson 2103-6-13) PLID 57165 - flag to determine if any charges were changed during the EMNs session.
	bool m_bChargesChanged;

public:
	// (c.haag 2008-07-21 14:05) - PLID 30725 - Array of EMR problems
	// (c.haag 2009-05-16 12:03) - PLID 34310 - We now track problem links instead of problems.
	CArray<CEmrProblemLink*, CEmrProblemLink*> m_apEmrProblemLinks;

	// (z.manning 2011-07-14 10:08) - PLID 44469
	CEmnCodingGroupInfoArray* GetCodingGroupInfoArray();

public:
	// (c.haag 2010-07-07 12:49) - PLID 38886 - Adds an item to m_apeaoMintLoaderStack (see
	// comments next to the member variable declaration)
	void PusheaoMintItemsStack(CEMNLoader** p) { m_apeaoMintLoaderStack.Add(p); }
	// (c.haag 2010-07-07 12:49) - PLID 38886 - Pops an item from the m_apeaoMintLoaderStack (see
	// comments next to the member variable declaration)
	void PopeaoMintItemsStack() { m_apeaoMintLoaderStack.RemoveAt( m_apeaoMintLoaderStack.GetSize() - 1 ); }

protected:
	// (j.jones 2007-09-11 17:41) - PLID 27243 - Office Visit incrementing is no longer
	// used in the L2 EMR, it's in Custom Records only
	//Used to keep track of office visits.
	//CDWordArray m_dwaryOldOfficeVisitServiceIDs;
	//CDWordArray m_dwaryNewOfficeVisitServiceIDs;

	//TES 5/16/2006 - PLID 20639 - CEMNDetail uses a m_pLastSavedDetail to store all this info in one place.  
	//But a m_pLastSavedEMN would include a lot of things (like all the topics), which there is already a long-standing 
	//architecture to handle the auditing of.  So, I'm just putting in variables on a case-by-case basis.
	COleDateTime m_dtLastSavedDate;
	EMNLocation m_LastSavedLocation;
	// (a.walling 2013-01-16 13:28) - PLID 54652 - Keep track of last saved for auditing
	EMNAppointment m_LastSavedAppointment;
	// (j.jones 2011-07-05 11:21) - PLID 43603 - changed to a class to store the name as well
	EMNStatus m_LastSavedStatus;
	CString m_strLastSavedNotes;
	CString m_strLastSavedDescription;
	// (z.manning, 04/19/2007) - PLID 25714 - Chart and category last saves.
	// (z.manning, 05/07/2007) - PLID 25731 - Now use classes so we can track the name as well.
	EMNChart m_LastSavedChart;
	EMNCategory m_LastSavedCategory;

	// (j.jones 2007-06-14 11:38) - PLID 26276 - store the Completion Status
	EMNCompletionStatus m_ecsCompletionStatus;
	EMNCompletionStatus m_ecsLastSavedCompletionStatus;

	// (j.jones 2007-08-06 16:08) - PLID 26974 - store the last saved Patient Demographics
	CString m_strLastSavedPatNameFirst;
	CString m_strLastSavedPatNameMiddle;
	CString m_strLastSavedPatNameLast;
	BYTE m_cbLastSavedPatientGender;
	// (z.manning 2010-01-13 10:38) - PLID 22672 - Age is now a string
	CString m_strLastSavedPatientAge;
	// (j.jones 2007-08-24 08:40) - PLID 27054 - added VisitTypeID
	long m_nLastSavedVisitTypeID;
	CString m_strLastSavedVisitTypeName;

	//TES 1/17/2014 - PLID 60397 - Option to hide the EMN title on the preview pane, per template
	bool m_bLastSavedHideTitleOnPreview;

	// (j.jones 2007-06-14 11:58) - PLID 26276 - calculate the completion status from the loaded topics
	EMNCompletionStatus CalculateCompletionStatus();

	// (b.eyers 2016-02-22) - PLID 68321 - new fields discharge status, admission time, discharge time
	EMNDischargeStatus m_LastSavedDischargeStatus;
	COleDateTime m_dtLastSavedAdmissionTime;
	COleDateTime m_dtLastSavedDischargeTime;

	// (a.walling 2011-10-20 14:23) - PLID 46075 - Clean up interaction with external interfaces
	class CEmrTreeWnd* m_pInterface;

	// (j.armen 2014-01-28 15:46) - PLID 30574 - Ability to monitor when this variable changes
	__declspec(property(get=IsInitialLoadComplete,put=SetInitialLoadComplete)) BOOL m_bInitialLoadComplete;
	bool m_bInitialLoadCompleteInternal;
	void SetInitialLoadComplete(BOOL b) { m_bInitialLoadCompleteInternal = !!b; }

public:
	BOOL IsInitialLoadComplete() const { return !!m_bInitialLoadCompleteInternal; }

protected:
	// (j.jones 2007-03-13 17:32) - PLID 25193 - track that we are trying to mark the load complete,
	// as soon as the parent EMR is done spawning
	BOOL m_bInitialLoadPendingCompletion;


// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

	// (c.haag 2007-08-06 09:28) - PLID 26858 - This function populates arFields with general narrative fields.
	// (j.jones 2008-10-30 17:24) - PLID 31869 - passed in a connection pointer
	// (a.walling 2009-11-17 16:51) - PLID 36365
	void LoadGenericNarrativeFields(CNarrativeMergeFieldMap &mapGenericMergeFields, OPTIONAL IN ADODB::_Connection *lpCon = NULL);
	//Loads the EMN-specific narrative fields (in its own function because these fields may change, and therefore
	//need to be reloaded.

	//This overload appends the fields to the passed-in array.
	// (j.jones 2008-10-30 17:24) - PLID 31869 - passed in a connection pointer
	// (a.walling 2009-11-17 16:56) - PLID 36365
	void LoadEmnNarrativeFields(CNarrativeMergeFieldMap &mapGenericMergeFields, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

		
	// (a.walling 2009-11-18 12:10) - PLID 36365
	void LoadLWNarrativeFields(CNarrativeMergeFieldMap &mapGenericMergeFields, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	// (j.jones 2008-01-11 13:15) - PLID 18709 - takes in an array and loads in the names of all
	// available merge fields
	// (j.jones 2008-10-30 16:11) - PLID 31869 - passed in a connection pointer
	void LoadLWMergeFieldList(CStringSortedArrayNoCase &aryFieldList, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	// (j.jones 2012-07-24 12:49) - PLID 44349 - added filter for only billable codes
	CString GetServiceCodesOutput(BOOL bOnlyIncludeBillableCodes);
	CString GetDiagCodesOutput();
	CString GetMedicationsOutput(); // (a.walling 2006-12-10 23:00) - PLID 17298

	void RemoveChargeByIndex(int nIndex);
	void RemoveDiagCodeByIndex(int nIndex);
	void RemoveMedicationByIndex(int nIndex);
	void RemoveProcedureByIndex(int nIndex);
	void RemoveProviderByIndex(int nIndex);

	//DRT 1/15/2007 - PLID 24177
	// (a.walling 2014-01-30 00:00) - PLID 60543 - Quantize
	Nx::Quantum::Batch GenerateSaveString_EMRChargesToDiagCodesT(EMNCharge *pCharge, CString strIDValue);
	BOOL CompareChargeDiagCodes(EMNCharge *pCharge, OUT CString &strOldValue);		//comparison for auditing

	//TES 3/26/2009 - PLID 33262 - Medications now have associated diagnosis codes as well, so I copied the two functions
	// above.
	// (j.jones 2012-09-28 10:18) - PLID 52820 - now we track a flag if something that contributed to drug interactions has changed,
	// in this case, diagnosis codes on a prescription
	// (a.walling 2014-01-30 00:00) - PLID 60543 - Quantize
	Nx::Quantum::Batch GenerateSaveString_PatientMedicationDiagCodesT(EMNMedication *pMed, CString strIDValue, OUT BOOL &bDrugInteractionsChanged);
	BOOL CompareMedicationDiagCodes(EMNMedication *pMed, OUT CString &strOldValue);		//comparison for auditing

public:
	// (c.haag 2007-04-26 10:59) - PLID 25790 - Exposes access to the EMN loader
	CEMNLoader* GetEMNLoader();

	// (c.haag 2007-09-10 12:52) - PLID 27024 - Returns the retired EMN loader
	CEMNLoader* GetRetiredEMNTemplateLoader();

public:
	// (c.haag 2007-08-04 09:42) - PLID 26945 - This is called when the initial load is done with
	// the EMN loader, and we want to try to use the loader object in a post-load role in avoiding
	// the querying of data.
	void RetireInitialLoader();

public:
	void FindDetailsByMergeFieldName(const CString& strMergeFieldName, CArray<CEMNDetail*,CEMNDetail*>& arypEMNDetail);

	// (j.jones 2012-02-20 09:32) - PLID 47886 - exposed the CEMNChargeArray find function
	EMNCharge* FindByServiceID(const long nServiceID);

protected:

	void GenerateCommonMergeData(CList<MergeField,MergeField&> &listMergeFields);
	void GenerateCategoryMergeData(CList<MergeField,MergeField&> &listMergeFields);
public:
	void GenerateRelatedMergeData(CList<MergeField,MergeField&> &listMergeFields, POSITION posLastNonAlphabetized);
protected:
	void GenerateMergeData(class CMergeEngine &mi, OUT CString &strHeaders, OUT CString &strData);

	void GenerateTempMergeFiles(CMergeEngine &mi);
	void SortAllDetails(CList<CEMNDetail*,CEMNDetail*> *plistOrderedDetails);
	void RemoveTempMergeFiles();
	void GenerateTempMergeFile(long nCategoryID, EmrCategoryFormat fmt, const CString& strCatName, CMergeEngine &mi, CList<CEMNDetail*,CEMNDetail*> *plistOrderedDetails);
	CString GetParagraph(long nCategoryID, EmrCategoryFormat Format, CMergeEngine &mi, CList<CEMNDetail*,CEMNDetail*> *plistOrderedDetails);
	//Given the index in m_arypEMNDetails, outputs the html of that item in "sentence form"
	//Uses some CMergeInfo functions, so give it a reference to our object.
	//TES 6/29/04: Actually the only one it uses is to get a path for the image output, so allow null.  Also, I'm publicizing
	//it, because it seems like a useful utility function and anyway I want the emritemadvdlg to access it.

	void FixDuplicateMergeFieldNames(CArray<CEMNDetail*,CEMNDetail*>& aryduplpEMNDetail);

	BOOL m_bUnsaved;
	BOOL m_bMoreInfoUnsaved;
	//TES 2/12/2014 - PLID 60470 - Added m_bCodesUnsaved
	BOOL m_bCodesUnsaved;

	// (a.walling 2012-03-12 12:43) - PLID 48712 - Always regenerate the preview pane upon load now
	//BOOL m_bEMRPreviewLoaded;
	BOOL m_bNeedToSavePreview;
	// (a.walling 2008-08-12 10:59) - PLID 31037 - Has the preview been rebuilt for this session?
	BOOL m_bIsPreviewCurrent;

public:	
	// (a.walling 2008-08-12 10:59) - PLID 31037 - Has the preview been rebuilt for this session?
	inline BOOL IsPreviewCurrent() { return m_bIsPreviewCurrent; };

	// (r.farnworth 2014-02-26 09:47) - PLID 60746 - Deleting a problem from the Codes dialog was causing MoreInfo to be unsaved. 
	// We need to call this in order to mark only one as unsaved.
	void SetVariableOnlyUnsaved() { m_bUnsaved = TRUE; };

	// (a.walling 2008-06-12 13:24) - PLID 27301 - Moved to public
	BOOL WarnOfDuplicateMergeNames();

	// (a.walling 2007-04-05 16:41) - PLID 25454
	//(e.lally 2009-10-05) PLID 32503 - Added parameter to override the hidden demographics to display when printing/faxing.
	// (a.walling 2013-03-13 08:40) - PLID 55632 - Ignore unsaved is no longer applicable
	CString GenerateHTMLString(BOOL bIgnoreUnsaved, BOOL bOverrideHiddenDemographics); // generate the HTML string for the entire EMN

	// (b.savon 2012-05-23 17:12) - PLID 48092
	CString GenerateMedAllergyHTML();
	
	// generate the HTML file, optionally telling the preview to reload and/or copying to documents,
	// and optionally ignoring unsaved details/topics
	//(e.lally 2009-10-05) PLID 32503 - Added parameter to override the hidden demographics to display when printing/faxing.
	// (a.walling 2010-01-13 12:55) - PLID 36840 - Added parameter to prevent overwriting an existing file
	// (a.walling 2013-09-05 11:24) - PLID 58369 - Returns path to newly-generated html file
	CString GenerateHTMLFile(BOOL bSendMessage = TRUE, BOOL bCopyToDocuments = TRUE, BOOL bIgnoreUnsaved_Unused = FALSE, BOOL bOverrideHiddenDemographics = FALSE, BOOL bFailIfExists = FALSE); 

	// (a.walling 2007-07-11 16:55) - PLID 26640 - Return HTML for More Info
	CString GenerateMoreInfoHTML();

	// (a.walling 2007-07-12 15:11) - PLID 26640 - Returns HTML for header (patient demographics, location)
	// (a.walling 2013-03-13 08:40) - PLID 55632 - Ignore unsaved is no longer applicable
	CString GenerateHeaderHTML();

	// (a.walling 2008-10-06 16:52) - PLID 31430 - Generate header and footer html for printing
	// (a.walling 2008-10-14 11:50) - PLID 31404 - Consolidated into single function
	// (a.walling 2008-11-13 15:35) - PLID 32024 - Made print header functions static, accepting either a CEMN pointer or ID
	static void GeneratePrintHeaderFooterHTML(IN CEMN* pEMN, IN long nID, OUT CString& strHeader, OUT CString& strFooter);

	// (a.walling 2008-10-14 11:50) - PLID 31404 - Replace customized fields in the header/footer strings
	// (a.walling 2008-11-13 15:35) - PLID 32024 - Made print header functions static
	static void ReplaceHeaderFooterFields(IN OUT CString& strHTML, IN const CPreviewFields& pf);

	// (a.walling 2008-10-15 10:22) - PLID 31404 - Build the header/footer HTML
	// (a.walling 2008-11-13 15:35) - PLID 32024 - Made print header functions static
	static void BuildPrintHeader(IN OUT CString& strHTML, IN const CHeaderFooterInfo& hfi);
	static void BuildPrintFooter(IN OUT CString& strHTML, IN const CHeaderFooterInfo& hfi);
	
	CEMRTopic* GetTopicByPointer(long nTopicPtr); // returns the CEMRTopic* that matches nTopicPtr
	CEMNDetail* GetDetailByPointer(long nDetailPtr); // returns the CEMNDetail* that matches nDetailPtr
	
	// (a.walling 2012-10-04 12:41) - PLID 52878 - Verify that the given emr object pointer is a child
	CEMRTopic* VerifyPointer(CEMRTopic* pTopic);
	CEMNDetail* VerifyPointer(CEMNDetail* pDetail);

	// (a.walling 2007-04-25 12:38) - PLID 25549 - Fill array with sorted topics
	// (a.walling 2007-10-18 15:35) - PLID 25549 - No longer necessary
	//void GetSortedTopicArray(CArray<CEMRTopic*, CEMRTopic*> &arTopics);

	void SetNeedToSavePreview(BOOL bSet = TRUE);
	BOOL GetNeedToSavePreview();

	// (a.walling 2007-04-12 12:18) - PLID 25605 - Add a reference to the temp image file
	void AddTempImageFileReference(CString str);

	// (b.savon 2014-03-07 10:13) - PLID 60826 - SPAWN - Check if any unmatched diagcodes
	BOOL HasUnmatchedDiagnosisCodes(long nWarnIfUnmatchedPreference);

protected:
	// (a.walling 2007-04-12 12:18) - PLID 25605 - Track the temp files we've created and those we need to move next save

	// (a.walling 2012-03-12 12:43) - PLID 48712 - Renamed m_arTempImageFile to m_arTempPreviewFiles
	//CStringArray m_arTempPreviewFiles;
	CStringArray m_arTempPreviewFiles;

	// (a.walling 2013-03-14 08:51) - PLID 55640 - Background CDO thread
	NxThread m_cdoThread;
	void WaitForCDOThread();

	// (a.walling 2007-10-15 16:30) - PLID 27664 - Last saved HTML for more info and header
	// (a.walling 2013-05-01 10:33) - PLID 55632 - Removed all the last saved html (m_strLastSavedMoreInfoHTML, m_strLastSavedHeaderHTML)

	//CStringArray m_arTempImageFilesPendingMove;

	long m_nRefCnt;

public:
	long AddRef() { return ++m_nRefCnt; }
	long Release(void) {
		long nRefCnt = --m_nRefCnt;
		if (0 == nRefCnt) delete this;
		return nRefCnt;
	}

	//TES 4/11/2007 - PLID 25456 - The EMN needs to know if a detail that is tied to the EMN is popped up, that way it can,
	// if that detail is a narrative, have it update itself even if spawning is in progress.
	// (a.walling 2007-08-22 14:51) - PLID 27160 - Handle multiple popped up details
	void AddPoppedUpDetail(CEMNDetail *pDetail);
	void RemovePoppedUpDetail(CEMNDetail *pDetail);

	//DRT 8/3/2007 - PLID 26914
	void EnsureCompletelyLoaded();
	
protected:
	//TES 4/11/2007 - PLID 25456 - The EMN needs to know if a detail that is tied to the EMN is popped up, that way it can,
	// if that detail is a narrative, have it update itself even if spawning is in progress.
	// (a.walling 2007-08-23 11:15) - PLID 27160 - We may have more than one popped up detail now
	CList<CEMNDetail*, CEMNDetail*> m_lstPoppedUpDetails;

	//TES 1/9/2008 - PLID 24157 - Rather than a detail popping itself up, it should call this function and pass itself
	// as the parameter (this function will increment the reference count).
	void AddDetailToPopup(CEMNDetail *pDetail, CEMNDetail *pSourceDetail);
public:
	// (c.haag 2009-07-02 10:16) - PLID 34760 - Now made available for CEMRTopic
	void RemoveDetailFromPopup(CEMNDetail *pDetail);

protected:
	//TES 1/9/2008 - PLID 24157 - This is our dialog that pops up multiple details at once.
	CEMRItemAdvMultiPopupDlg *m_pMultiPopupDlg;

	//TES 1/28/2008 - PLID 28673 - We track all of the DetailPopup information that we are keeping alive for 
	// possible re-popup, so we can destroy it at the appropriate time.
	CArray<DetailPopup*,DetailPopup*> m_arDetailPopups;

	//TES 1/23/2008 - PLID 28673 - Popups up the MultiPopupDlg, as well as doing any preparation and cleanup
	// necessary for that.
	//TES 1/28/2008 - PLID 28673 - Added a pInitWithDetails parameter, needs to do slightly different things if you're
	// restoring a previous popup as compared to creating a new one.
	void PopupMultiPopupDlg(DetailPopup* pInitWithDetails = NULL);

	// (a.walling 2008-05-22 12:56) - PLID 22049
	CString m_strWriteToken;
	_variant_t m_varRevision;

	// (a.walling 2008-06-27 16:54) - PLID 30482
	BOOL m_bParentEMRChanged;

protected:
	// (c.haag 2008-07-14 11:23) - PLID 30696 - This array tracks all the EMN-specific todo alarms
	// created through the "New ToDo" button or by spawning while the EMN is unsaved
	CArray<EMNTodo*,EMNTodo*> m_apCreatedTodosWhileUnsaved;

	// (c.haag 2008-07-14 11:23) - PLID 30696 - This array tracks all the EMN-specific todo alarms
	// deleted from the More Info's Todo list or by unspawning while the EMN is unsaved
	CArray<EMNTodo*,EMNTodo*> m_apDeletedTodosWhileUnsaved;

public:
	//TES 1/16/2008 - PLID 24157 - Called once all actions have been processed, allows the EMN to popup its details.
	void ProcessEmrActionsComplete();
	//TES 1/16/2008 - PLID 24157 - If this EMN's popup dialog is currently open, will return a pointer to it, otherwise
	// returns NULL.
	CEMRItemAdvMultiPopupDlg* GetOpenMultiPopupDlg();

	//TES 1/23/2008 - PLID 28673 - This function creates an multipopup dialog, and initializes it to the tree of
	// details represented by pDetailPopup.  If there is an existing multipopupdlg, which there should never be,
	// this function has no affect.
	void RestoreMultiPopup(DetailPopup* pDetailPopup);

	// (a.walling 2008-05-22 12:52) - PLID 22049 - EMR Multi-User - Request write access and token from data.
	BOOL RequestWriteToken(OUT CWriteTokenInfo& wtInfo, OPTIONAL IN BOOL bExpropriate = FALSE);

	// (a.walling 2008-05-22 12:52) - PLID 22049 - EMR Multi-User - Release write access and clear the token in data.
	BOOL ReleaseWriteToken();

	// (a.walling 2008-05-22 12:52) - PLID 22049 - EMR Multi-User - Verifies that the stored write token matches that in data and retrieves information
	// bIsOldRevision -- if the EMN has been modified since last opened by this user.
	// Other parameters are returned with information about the currently held user.
	BOOL VerifyWriteToken(OUT CWriteTokenInfo& wtInfo);

	// (a.walling 2008-05-22 12:52) - PLID 22049 - EMR Multi-User - Return the write token
	CString GetWriteToken();

	// (a.walling 2008-05-29 15:52) - PLID 22049 - EMR Multi-User - Return the revision
	_variant_t GetRevision();

	// (a.walling 2008-05-29 14:44) - PLID 22049 - EMR Multi-User - Sets the stored revision of this EMN
	void SetRevision(_variant_t &var);

	// (a.walling 2014-08-27 18:02) - PLID 63502 - Force reload of revision when modified by API. This is a dirty hack.
	void ForceReloadRevision();

	// (a.walling 2008-05-22 12:52) - PLID 22049 - EMR Multi-User - Is this EMN writable?
	BOOL IsWritable();

	// (j.jones 2010-04-02 15:34) - PLID 37980 - added function to determine if the EMN is editable,
	// based on permissions, writeable token, and locked status
	BOOL CanBeEdited();

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code


public:
	// (c.haag 2009-06-29 17:55) - PLID 34750 - Called to delete an EMN detail. This used to be called
	// by the topic at the CEMR level; now we pass through the CEMN level first to make sure that the
	// equivalent pop-up object is also deleted.
	void DeleteEMNDetail(CEMNDetail *pDetail);

public:
	// (c.haag 2008-07-14 11:19) - PLID 30696 - This function is called when a todo is either spawned or
	// added to the EMN by means of the "New ToDo" button. We track the todo ID for the unlikely event 
	// that theuser wants to cancel out of the EMN.
	// (z.manning 2009-02-26 15:27) - PLID 33141 - Added SourceDataGroupID
	// (z.manning 2010-02-24 17:04) - PLID 37532 - Replaced SourceDataGroupID with ptrSourceTableRow
	// (c.haag 2012-10-17) - PLID 52863 - Overload for not-spawned todo alarms
	void AddCreatedTodoWhileUnsaved(long nTodoID, _variant_t vRemind, _variant_t vDone, _variant_t vDeadline, 
		_variant_t vEnteredBy, _variant_t vPersonID, _variant_t vNotes, _variant_t vPriority, _variant_t vTask,
		_variant_t vLocationID, _variant_t vCategoryID, _variant_t vRegardingType,
		const CArray<long,long>& anAssignTo, _variant_t vAssignToNames);

	// (c.haag 2012-10-17) - PLID 52863 - Overload for spawned todo alarms
	void AddCreatedTodoWhileUnsaved(long nTodoID, const EmrAction& ea, const SourceActionInfo &sai,
		_variant_t vRemind, _variant_t vDone, _variant_t vDeadline, _variant_t vEnteredBy,
		_variant_t vPersonID, _variant_t vNotes, _variant_t vLocationID, _variant_t vRegardingType,
		const CArray<long,long>& anAssignTo, _variant_t vAssignToNames);

	// (c.haag 2008-07-14 11:21) - PLID 30696 - This function is called when a todo is either unspawned or
	// deleted from the EMN by means of the "More Info" todo list
	// (z.manning 2009-02-26 15:27) - PLID 33141 - Added SourceDataGroupID
	void AddDeletedTodoWhileUnsaved(long nTodoID, CEMNDetail* pSourceDetail, long nSourceActionID,
		TableRow *ptrSourceTableRow, _variant_t vRemind, _variant_t vDone, _variant_t vDeadline, _variant_t vEnteredBy,
		_variant_t vPersonID, _variant_t vNotes, _variant_t vPriority, _variant_t vTask,
		_variant_t vLocationID, _variant_t vCategoryID, _variant_t vRegardingID, _variant_t vRegardingType,
		const CArray<long,long>& anAssignTo, _variant_t vAssignToNames);

public:
	// (c.haag 2008-07-09 13:15) - PLID 30648 - Appends apTodos with this EMN's list of todos spawned prior to having been saved
	// (c.haag 2008-07-14 11:39) - PLID 30696 - Now appends apTodos with todos either spawned, or created from the "New ToDo" button
	void GenerateCreatedTodosWhileUnsavedList(CArray<EMNTodo*,EMNTodo*>& apTodos);

	// (c.haag 2008-07-09 13:15) - PLID 30648 - Appends apTodos with this EMN's list of todos unspawned prior to having been saved
	// (c.haag 2008-07-14 11:56) - PLID 30696 - Now appends apTodos with todos either unspawned, or deleted from the More Info topic.
	void GenerateDeletedTodosWhileUnsavedList(CArray<EMNTodo*,EMNTodo*>& apTodos);

	// (c.haag 2008-07-16 11:30) - PLID 30752 - Copies todo alarms from one EMN to another. This will invoke a prompt for which the
	// user has to choose which alarms to copy.
	void CopyTodoAlarms(CEMN* pSourceEMN);

	// (c.haag 2008-07-23 12:16) - PLID 30820 - Populate apProblems with a list of all deleted problems for this object and
	// all its children. If a child or related EMR object is deleted, all its problems are considered deleted as well.
	// (j.jones 2009-05-29 09:46) - PLID 34301 - returns links now, not problems
	void GetAllDeletedEmrProblemLinks(CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblemLinks, BOOL bIncludeThisObject);

	// (c.haag 2008-07-25 16:10) - PLID 30826 - Returns TRUE if there is at least one saved problem for a given charge
	BOOL DoesChargeHaveSavedProblems(EMNCharge* pCharge);

	// (c.haag 2008-07-25 16:10) - PLID 30826 - Returns TRUE if there is at least one saved problem for a given diagnosis code
	BOOL DoesDiagCodeHaveSavedProblems(EMNDiagCode* pDiagCode);

	// (c.haag 2008-07-25 16:10) - PLID 30826 - Returns TRUE if there is at least one saved problem for a given diagnosis code
	BOOL DoesMedicationHaveSavedProblems(EMNMedication* pMedication);

	// (j.jones 2012-11-21 14:04) - PLID 53818 - added ReloadMedicationsFromData, which is allowed to be called 
	// on either and EMN or a template, and will call the correct function accordingly
	// (j.jones 2013-02-07 09:25) - PLID 55045 - now takes in an optional parameter to disable
	// showing drug interactions when the requery finishes
	void ReloadMedicationsFromData(BOOL bForceReload, BOOL bShowDrugInteractions = TRUE);

	// (c.haag 2014-07-16) - PLID 54905 - Reloads only the charges section of this EMN
	void ReloadChargesFromData();

	// (z.manning 2013-09-17 13:38) - PLID 58450
	void CheckSaveEMNForDrugInteractions(BOOL bAlreadySavedAnInteractionChange);

protected:
	// (z.manning 2013-09-17 14:18) - PLID 58450 - Variables used when we have to delay checking drug interactions
	// because we're in the middle of spawning.
	BOOL m_bNeedToHandleCheckSaveEMNForDrugInteractionsOnDetailUnlock;
	BOOL m_bAlreadySavedAnInteractionChange_WhenUnlockingDetail;

protected:

	// (j.jones 2009-04-01 15:21) - PLID 33736 - shared this function between loading from an EMN, and ReloadPatientMedicationsFromData
	void AddMedicationFromRecordset(ADODB::_RecordsetPtr &rsMedications, OPTIONAL IN ADODB::_Connection *lpCon = NULL);

	// (j.jones 2012-11-21 14:19) - PLID 53818 - shared this function from loading a template and ReloadTemplateMedicationsFromData
	void AddTemplateMedicationFromRecordset(BOOL bIsTemplate, ADODB::_RecordsetPtr &rsMedications);

	// (j.jones 2009-04-01 15:03) - PLID 33736 - added ability to reload medications from data
	// if bForceReload is FALSE, we will only perform this reload if we have any medication objects already loaded
	// (j.jones 2013-02-07 09:25) - PLID 55045 - now takes in an optional parameter to disable
	// showing drug interactions when the requery finishes
	void ReloadPatientMedicationsFromData(BOOL bForceReload, BOOL bShowDrugInteractions = TRUE);

	// (j.jones 2012-11-21 14:04) - PLID 53818 - added ReloadTemplateMedicationsFromData
	// if bForceReload is FALSE, we will only perform this reload if we have any medication objects already loaded
	void ReloadTemplateMedicationsFromData(BOOL bForceReload);

public:
	// (c.haag 2009-05-13 15:22) - PLID 34256 - Added the ability to resynchronize all allergy details
	// for all possible EMN's with the patient's official allergies.
	// (j.jones 2009-09-18 13:12) - PLID 35599 - this should only be done from NewCrop usage, so I renamed accordingly
	// (z.manning 2011-09-22 12:34) - PLID 45623 - Moved this here from CEMR
	// (j.jones 2013-02-12 15:38) - PLID 55139 - this is now callable outside of NewCrop, so I renamed it back
	void ReloadPatientAllergiesFromData(long nOldAllergiesInfoID, long nNewAllergiesInfoID);

	// (c.haag 2009-05-15 09:57) - PLID 34271 - Added the ability to resynchronize all current medications details
	// for all possible EMN's with the patient's official current medications.
	// (j.jones 2009-09-18 13:12) - PLID 35599 - this should only be done from NewCrop usage, so I renamed accordingly
	// (c.haag 2010-02-18 11:16) - PLID 37424 - We now include the entire browser result so that we can also read which
	// patient prescriptions were added in the same session
	// (z.manning 2011-09-22 12:34) - PLID 45623 - Moved this here from CEMR
	void ReloadPatientCurrentMedicationsFromData_ForNewCrop(NewCropBrowserResult* pNCBR);	

	// (c.haag 2009-05-13 15:25) - PLID 34256 - Added ability to load patient allergies from data
	// and resynchronize all official Allergies details with it
	// (j.jones 2009-09-18 13:12) - PLID 35599 - this should only be done from NewCrop usage, so I renamed accordingly
	// (j.jones 2013-02-12 15:38) - PLID 55139 - this is now callable outside of NewCrop, so I renamed it back
	void CEMN::ReloadPatientAllergiesFromData(long nOldAllergiesInfoID, long nNewAllergiesInfoID, const CArray<long,long>& anAllergies);

	// (c.haag 2009-05-15 10:00) - PLID 34271 - Added ability to load patient current medications from data
	// and resynchronize all official Current Medications details with it
	// (j.jones 2009-09-18 13:12) - PLID 35599 - this should only be done from NewCrop usage, so I renamed accordingly
	// (j.jones 2011-05-04 14:33) - PLID 43527 - added mapDataIDsToSig, which tracks the Sig for each current medication
	void ReloadPatientCurrentMedicationsFromData_ForNewCrop(long nOldCurrentMedicationsInfoID, long nNewCurrentMedicationsInfoID,
		const CArray<long,long>& anCurMeds, CMap<long, long, CString, LPCTSTR> &mapDataIDsToSig);

	// (z.manning 2009-07-07 11:39) - PLID 34078 - Returns true if the EMN has at least one e-prescribed medication.
	BOOL HasEPrescriptions();

	//TES 11/2/2009 - PLID 35808 - Returns true if this EMN is a template, and if its NexWebUUID is not null.
	BOOL IsNexWebTemplate();

	// (j.gruber 2012-08-31 14:42) - PLID 52285 - OMR Form
	BOOL IsOMRTemplate();

	// (a.walling 2009-10-28 14:05) - PLID 35989 - Find the best topic for a new signature (either a *signature* topic, or the very last topic on an EMN)
	// (z.manning 2011-10-28 17:52) - PLID 44594 - Moved this to CEMN from CEMRPreviewCtrlDlg
	CEMRTopic* FindAppropriateSignatureTopic(bool* pbIsSignatureTopic);

	// (b.spivey, March 06, 2012) - PLID 48581 - Need to find and remove conflicting codes. 
	bool FindAndRemoveEMChargeConflicts(long nServiceID, CString strServiceCode);

	// (r.gonet 08/03/2012) - PLID 51027 - Perform Wound Care Auto Coding, which will take a table that
	//  can be used with Wound Care Coding (EMRInfoT.UseWithWoundCareCoding = 1) and perform some computation
	//  on columns associated with Wound Care Coding (EmrDataT.WoundCareDataType). Then if specific conditions are
	//  met, performs condition specific actions.
	void PerformWoundCareAutoCoding();

	// (j.jones 2013-06-18 13:23) - PLID 47217 - added ability to remove all signature details from new EMNs
	void RemoveNewSignatureDetails();

	// (c.haag 2016-06-09 14:54) - PLID-66502 - Returns TRUE if a charge was modified
	BOOL WasChargeChanged(EMNCharge* pCharge);
};

// (c.haag 2010-07-07 12:49) - PLID 38886 - The CEMN object has an array that tracks all of 
// the pEMNMintLoader objects for every call to ProcessEmrAction_eaoMintItems in the call stack. 
// The purpose of this is to make it possible for the currently spawning template to search all
// previously spawned templates in the same spawn chain for user-defined places to spawn topics.
// This class will add a pointer to a pEMNMintLoader to this array, and remove it once the pEMNMintLoader
// leaves scope. We retain a pointer to it because its actual value can change through its lifetime,
// and we want to save ourselves the trouble of having to keep up.
class CEmrMintItemsLoaderPreserver
{
private:
	CEMN* m_pEMN;
public:
	CEmrMintItemsLoaderPreserver(CEMN* pEMN, CEMNLoader** ppEMNMintLoader)
	{
		m_pEMN = pEMN;
		pEMN->PusheaoMintItemsStack(ppEMNMintLoader);
	}
	~CEmrMintItemsLoaderPreserver()
	{
		m_pEMN->PopeaoMintItemsStack();
	}
};

////
/// Inline functions


inline long CEMN::GetTopicCount()
{
	return m_arypEMRTopics.GetSize();
}

inline CEMRTopic* CEMN::GetTopic(long nIndex)
{
#ifdef _DEBUG
	if(nIndex < 0 || nIndex >= m_arypEMRTopics.GetSize()) {
		ASSERT(FALSE);
		AfxThrowNxException("CEMN::GetTopic() called with invalid index!");
	}
#endif
	return m_arypEMRTopics[nIndex]; // calls AfxThrowInvalidArgException if invalid index 
}

#endif
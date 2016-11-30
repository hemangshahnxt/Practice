//
// (c.haag 2008-11-25 11:28) - PLID 31693 - This class holds functionality mutual to
// the CEmrItemAdvNarrativeBase and CEMRItemAdvPopupWnd classes.
//
#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "EmrItemAdvNarrativeBase.h"
#include "EmnDetail.h"
#include "EmrTopic.h"
#include "EMN.h"
#include "EMNDetail.h"

#include <set>
#include <foreach.h>

CEmrItemAdvNarrativeBase::CEmrItemAdvNarrativeBase(void)
{
}

CEmrItemAdvNarrativeBase::~CEmrItemAdvNarrativeBase(void)
{
}

// (c.haag 2008-11-25 10:37) - PLID 31693 - Sorting function for SortDetailsByPositions
static int QSortDetailsByPositionsProc(const void *pa, const void *pb)
{
	LinkedDetailStruct* pls1 = (LinkedDetailStruct*)pa;
	LinkedDetailStruct* pls2 = (LinkedDetailStruct*)pb;
	if (pls1->nStartPos < pls2->nStartPos) {
		return -1;
	} else if (pls1->nStartPos > pls2->nStartPos) {
		return 1;
	} else {
		return 0;
	}
}
// (c.haag 2008-11-25 10:37) - PLID 31693 - Sort arDetails by narrative positions
void CEmrItemAdvNarrativeBase::SortDetailsByPositions(CArray<LinkedDetailStruct,LinkedDetailStruct&>& arDetails)
{
	// Sort the array
	qsort(arDetails.GetData(), arDetails.GetSize(), sizeof(LinkedDetailStruct), QSortDetailsByPositionsProc);
}

// (c.haag 2008-11-25 12:00) - PLID 31693 - Used to determine whether the narrative window is valid
// after a field pop-up occurs on a popped up narrative
struct EnumChildParam {
	HWND hWndNarrative;
	BOOL bWindowFound;
};
static BOOL CALLBACK EnumNarrativeBaseChildProc(HWND hwndChild, LPARAM lParam) 
{ 
	EnumChildParam* pParam = (EnumChildParam*)lParam;
	if (hwndChild == pParam->hWndNarrative) {
		pParam->bWindowFound = TRUE;
	}
	return TRUE; // The documentation says this is not used, but an MSDN code sample returns TRUE.
}

// (c.haag 2008-11-25 11:34) - PLID 31693 - Handles when a user clicks on a narrative field
void CEmrItemAdvNarrativeBase::HandleLinkRichTextCtrl(LPCTSTR strMergeField, CEMNDetail* pNarrativeDetail,
													  HWND hWndNarrative, CWnd* pWndPopupParent)
{
	try {
		// (a.walling 2009-11-19 11:52) - PLID 36366 - Just use the narrative itself to find the appropriate detail for this field
		{
			CString strFieldDummy;
			CString strValueDummy;
			bool bIsValidDummy;

			CString strField = strMergeField;

			// (z.manning 2011-11-10 16:26) - PLID 37093 - Handle procedure fields
			// (j.jones 2012-12-27 15:25) - PLID 54369 - the above item was never finished, and this created broken, half-working code
			/*
			if(IsProcedureNarrativeField(strField)) {
				MessageBox(hWndNarrative, "This field is tied to procedure content and can be edited from the Procedure tab in the Administrator module.", "Practice", MB_ICONINFORMATION);
				return;
			}
			*/

			// (a.walling 2010-03-29 10:43) - PLID 37923 - Special handling for the EMN Spawning Text, also fixed existing typo
			// (a.walling 2010-04-01 10:44) - PLID 38013 - Consolidating some constant text strings
			// (a.walling 2010-04-01 14:38) - PLID 38013 - Special message for Item Spawning Text
			// (a.walling 2010-04-20 13:42) - PLID 38013 - Check for some sentinel values here first.
			if (strField == NFIELD_EMN_SPAWNING_TEXT) {
				MessageBox(hWndNarrative, "This field is built-in. It's value comes from the item that spawns this EMN.", "Practice", MB_ICONINFORMATION);
				return;
			} else if (strField == NFIELD_ITEM_SPAWNING_TEXT || strField == NFIELD_ITEM_SPAWNING_TEXT_FURTHEST) {
				MessageBox(hWndNarrative, "This field is built-in. It's value comes from the item that directly or indirectly spawned this narrative.", "Practice", MB_ICONINFORMATION);
				return;
			}

			if (pNarrativeDetail && pNarrativeDetail->m_pParentTopic && pNarrativeDetail->m_pParentTopic->GetParentEMN()) {
				if (pNarrativeDetail->m_pParentTopic->GetParentEMN()->GetGenericMergeFieldValue(strMergeField, strValueDummy, bIsValidDummy)) {
					// We can't do much here.
					MessageBox(hWndNarrative, "This field is built-in. You may change its value in the More Info section of the EMN.", "Practice", MB_ICONINFORMATION);
					return;
				}
			}

			//DRT 3/10/2006 - PLID 19584 - Previously this was doing an assert, and if the assertion
			//	failed, we tried to popup (with a NULL pointer!) anyways.  If we know the ptr is bad, 
			//	we'll throw an exception.  t.schneider and I discussed this, he does not believe
			//	there is any legitimate way of getting in into this state, but as early as sometime last 
			//	week there were known bugs which may have caused it.  From this point forward we'll know
			//	for sure.
			CEMNDetail* pDetail = pNarrativeDetail->GetDetailFromNarrativeMergeField(strMergeField);
			if (NULL != pDetail) {
				HandleLinkRichTextCtrl(pDetail, pNarrativeDetail, hWndNarrative, pWndPopupParent);
			} else {
				//If you get this to happen, then we know there is some way to get a narrative which contains a field
				//	which does not have a matching name anywhere on this EMN.
				CString str;
				str.Format("Narrative could not popup the detail '%s'.  It may have been removed from this EMN.", strMergeField);
				AfxThrowNxException(str);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2012-04-02) - PLID 49346 - Handles when a user clicks on a narrative field. We actually take in the detail object
// instead of a merge field like the other overload does.
void CEmrItemAdvNarrativeBase::HandleLinkRichTextCtrl(CEMNDetail* pDetail, CEMNDetail* pNarrativeDetail,
													  HWND hWndNarrative, CWnd* pWndPopupParent)
{
	try {
		BOOL bUseNarrativeFieldPopupJumping = GetRemotePropertyInt("EMRNarrativeFieldPopupJumping",1,0,GetCurrentUserName(),TRUE) ? TRUE : FALSE;
		// (a.walling 2012-04-23 14:09) - PLID 49932 - Popping up details sequentially from a narrative may end up popping up incorrect (or less correct) details
		// The problem is that details are linked to the narrative via text, and the context of the EMN can change what that text links to. The issue this 
		// addresses in particular is when a prior item changes things (spawning, etc) that causes the following items to refer to more accurate items. For example, 
		// there may be a 'Duration' field. This could refer to a Duration item on another topic. However, I click on a field beforehand, that spawns a new item 
		// Duration. Now this *other* Duration is a better fit, but the sequential popup would still pop up the old one.

		if (!bUseNarrativeFieldPopupJumping) {
			PopupDetail(pDetail, pNarrativeDetail, hWndNarrative, pWndPopupParent);
		} else {
			PopupDetailSequence(pDetail, pNarrativeDetail, hWndNarrative, pWndPopupParent);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-04-23 14:09) - PLID 49932 - Pops up a single detail. Returns 0 if failure or could not popup; otherwise returns the dialog result (IDOK, etc)
INT_PTR CEmrItemAdvNarrativeBase::PopupDetail(CEMNDetail* pDetail, 
	CEMNDetail* pNarrativeDetail,
	HWND hWndNarrative,
	CWnd* pWndPopupParent)
{
	// (c.haag 2008-11-12 11:35) - PLID 31693 - We now track whether anything was popped up at all;
	// and if so, whether it was a success
	INT_PTR nResult;
	BOOL bWasPoppedUp = pDetail->Popup(pWndPopupParent, &nResult);

	//TES 1/17/2008 - PLID 24157 - The detail's state probably changed, our parent dialog should make sure
	// that it is up to date
	if (NULL != pWndPopupParent) {
		CWnd* pWnd = CWnd::FromHandlePermanent(hWndNarrative);
		pWndPopupParent->SendMessage(NXM_EMR_POPUP_POST_STATE_CHANGED, (WPARAM)pWnd, (LPARAM)pDetail);
	}

	// (c.haag 2008-11-25 11:47) - PLID 31693 - Factoring this comment into account:
		//TES 1/17/2008 - PLID 24157 - It is possible that popping up this detail may ultimately result in this
		// window being destroyed, and the address of our parent dialog being overwritten with garbage.  Therefore,
		// remember it now, so we can still fire off a message as we are destroyed.
	// We need to also check this object's validity. To do this, we enumerate all the parent window's
	// children and look for this one. If it doesn't exist, abort the loop immediately.
	if (NULL != pWndPopupParent) {
		EnumChildParam param;
		memset(&param, 0, sizeof(param));
		param.hWndNarrative = hWndNarrative;
		EnumChildWindows(pWndPopupParent->GetSafeHwnd(), EnumNarrativeBaseChildProc, (LPARAM)&param);
		if (!param.bWindowFound) {
			// Did not find the narrative window; it may have been destroyed.
			return FALSE;
		} else {
			// The narrative window is still a child, so it should still be valid
		}
	}

	if (!bWasPoppedUp) {
		return FALSE;
	}

	return nResult;
}

// (a.walling 2012-04-23 14:09) - PLID 49932 - Pops up a sequence of details in the narrative starting from pStartDetail
// now we figure out what each narrative field should refer to immediately before popup, rather than trying to determine them all at once beforehand.
void CEmrItemAdvNarrativeBase::PopupDetailSequence(CEMNDetail* pStartDetail, 
	CEMNDetail* pNarrativeDetail,
	HWND hWndNarrative,
	CWnd* pWndPopupParent)
{
	try {
		std::set<CEMNDetail*> poppedUpDetails;
		std::set<CString> poppedUpFields;

		bool bPoppedUp = false;

		do {
			bPoppedUp = false;

			// (j.armen 2012-12-03 13:56) - PLID 52752 - Changed template for Narrative Field Arrays
			CArray<NarrativeField> arNarrativeFields;
			CString strNxRichText = VarString(pNarrativeDetail->GetState(), "");

			strNxRichText.TrimRight();

			//TES 6/6/2012 - PLID 50540 - Call GetNarrativeFieldArray(), which handles both RTF and HTML
			GetNarrativeFieldArray(strNxRichText, arNarrativeFields);

			bool bFoundStart = false;

			if (!pStartDetail) {
				bFoundStart = true;
			}

			foreach (const NarrativeField& nf, arNarrativeFields) {

				//TES 6/6/2012 - PLID 50854 - nEnd and nStart are always -1 for HTML narratives, so if that is the case, and nf.strField
				// is not empty, then proceed with this detail.
				if (nf.nEnd <= nf.nStart && (nf.nEnd != -1 || nf.nStart != -1 || nf.strField.IsEmpty())) {
					continue;
				}

				if (poppedUpFields.count(nf.strField)) {
					continue;
				}
				// (j.jones 2012-12-27 15:25) - PLID 54369 - commented out IsProcedureNarrativeField, it no longer exists
				if (/*IsProcedureNarrativeField(nf.strField) ||*/ nf.strField == NFIELD_EMN_SPAWNING_TEXT || nf.strField == NFIELD_ITEM_SPAWNING_TEXT || nf.strField == NFIELD_ITEM_SPAWNING_TEXT_FURTHEST) {					
					poppedUpFields.insert(nf.strField);
					continue;
				}
				
				if (pNarrativeDetail && pNarrativeDetail->m_pParentTopic && pNarrativeDetail->m_pParentTopic->GetParentEMN()) {
					CString strValueDummy;
					bool bIsValidDummy;
					if (pNarrativeDetail->m_pParentTopic->GetParentEMN()->GetGenericMergeFieldValue(nf.strField, strValueDummy, bIsValidDummy)) {
						poppedUpFields.insert(nf.strField);
						// We can't do much here.
						continue;
					}
				}

				CEMNDetail* pCheckDetail = pNarrativeDetail->GetDetailFromNarrativeMergeField(nf.strField);

				if (!pCheckDetail) {
					continue;
				}
				
				if (pCheckDetail == pStartDetail) {
					bFoundStart = true;
				}
				
				if (poppedUpDetails.count(pCheckDetail)) {
					continue;
				}

				if (!bFoundStart) {
					continue;
				}

				poppedUpDetails.insert(pCheckDetail);

				bPoppedUp = true;

				if (IDOK != PopupDetail(pCheckDetail, pNarrativeDetail, hWndNarrative, pWndPopupParent)) {
					return;
				}

				// break out of here and move to the next one
				break;
			}
		} while (bPoppedUp);
	} NxCatchAll(__FUNCTION__);
}
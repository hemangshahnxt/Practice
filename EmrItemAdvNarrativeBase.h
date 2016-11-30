#pragma once

// (j.jones 2013-05-16 15:41) - PLID 56596 - changed the EMNDetail.h include into a forward declare
class CEMNDetail;

//
// (c.haag 2008-11-25 11:28) - PLID 31693 - This class holds functionality mutual to
// the CEmrItemAdvNarrativeBase and CEMRItemAdvPopupWnd classes.
//
class CEmrItemAdvNarrativeBase
{
public:
	CEmrItemAdvNarrativeBase(void);
	virtual ~CEmrItemAdvNarrativeBase(void);

protected:
	// (c.haag 2008-11-25 10:37) - PLID 31693 - Sort arDetails by narrative positions
	static void SortDetailsByPositions(CArray<LinkedDetailStruct,LinkedDetailStruct&>& arDetails);

public:
	// (c.haag 2008-11-25 11:34) - PLID 31693 - Handles when a user clicks on a narrative field
	static void HandleLinkRichTextCtrl(LPCTSTR strMergeField, // The merge field the user clicked on
		CEMNDetail* pNarrativeDetail, // The narrative detail containing the merge field
		HWND hWndNarrative, // The narrative detail's window handle
		CWnd* pWndPopupParent); // If the narrative is popped up, this is the parent window

	// (c.haag 2012-04-02) - PLID 49346 - Handles when a user clicks on a narrative field
	static void HandleLinkRichTextCtrl(CEMNDetail* pDetail, 
		CEMNDetail* pNarrativeDetail,
		HWND hWndNarrative,
		CWnd* pWndPopupParent);

	// (a.walling 2012-04-23 14:09) - PLID 49932 - Pops up a single detail. Returns 0 if failure or could not popup; otherwise returns the dialog result (IDOK, etc)
	static INT_PTR PopupDetail(CEMNDetail* pDetail, 
		CEMNDetail* pNarrativeDetail,
		HWND hWndNarrative,
		CWnd* pWndPopupParent);

	// (a.walling 2012-04-23 14:09) - PLID 49932 - Pops up a sequence of details in the narrative starting from pStartDetail
	static void PopupDetailSequence(CEMNDetail* pStartDetail, 
		CEMNDetail* pNarrativeDetail,
		HWND hWndNarrative,
		CWnd* pWndPopupParent);
};

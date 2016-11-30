// HistoryTabsDropSource.cpp: implementation of the CHistoryTabsDropSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "practice.h"
#include "HistoryTabsDropSource.h"
#include "HistoryDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHistoryTabsDropSource::CHistoryTabsDropSource()
{
	m_bIsPacket = FALSE;
	m_strDragFilename = "";
	m_wndParentHistoryDlg = NULL;
	m_hIcon = NULL;
}

CHistoryTabsDropSource::~CHistoryTabsDropSource()
{
	// (a.walling 2008-09-22 15:24) - PLID 31471
	if (m_hIcon) {
		::DestroyIcon(m_hIcon);
		m_hIcon = NULL;
	}
}

BOOL CHistoryTabsDropSource::OnBeginDrag(CWnd* pWnd) {	
	// Get the cursor position
	POINT ptCursorPos;
	GetCursorPos(&ptCursorPos);

	// Create a drag image
	if (m_bIsPacket) {
		m_ilDragImage.Create(IDB_DRAG_MULTI_FILES, 32, 38, RGB(255, 0, 0));
	// (a.walling 2007-07-19 11:23) - PLID 26748 - Support .docx, and also support extensions that may not be all lowercase
	// (a.walling 2007-10-12 14:04) - PLID 26342 - Also support macro-enabled 2007 documents
	} else if (m_hIcon) {
		// (a.walling 2008-09-22 15:04) - PLID 31471 - Use the passed in HICON
		m_ilDragImage.Create(32, 32, ILC_COLOR32, 1, 2);
		m_ilDragImage.Add(m_hIcon);
	} else if ( (m_strDragFilename.Right(4).CompareNoCase(".doc") == 0) || (m_strDragFilename.Right(5).CompareNoCase(".docx") == 0) || (m_strDragFilename.Right(5).CompareNoCase(".docm") == 0) ) {
		// (j.fouts 2012-05-22 10:22) - PLID 49610 - The new WordDoc Icon size is 32x32
		m_ilDragImage.Create(IDB_DRAG_WORD, 32, 32, RGB(255, 0, 0));
	} else {
		m_ilDragImage.Create(IDB_DRAG_FILE, 26, 32, RGB(255, 0, 0));
	}

	// Change the cursor to the drag image
	m_ilDragImage.BeginDrag(0, CPoint(0, 0));
	m_ilDragImage.DragEnter(CWnd::GetDesktopWindow(), ptCursorPos);

	return TRUE;
}

SCODE CHistoryTabsDropSource::GiveFeedback(DWORD dwDropEffect) {
	// Move the drag image
    POINT ptCursorPos;
	GetCursorPos(&ptCursorPos);	

	m_ilDragImage.DragMove(ptCursorPos); //move the drag image to those coordinates

	// Redraw the area behind the drag image on the parent window if needed
	if (m_wndParentHistoryDlg->m_bDragDropNeedToRefresh) {
		m_ilDragImage.DragShowNolock(FALSE);
		m_wndParentHistoryDlg->UpdateWindow();
		m_ilDragImage.DragShowNolock(TRUE);
		m_wndParentHistoryDlg->m_bDragDropNeedToRefresh = FALSE;
	}

	return DRAGDROP_S_USEDEFAULTCURSORS;
}

SCODE CHistoryTabsDropSource::QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState) {
	if (bEscapePressed)
		return DRAGDROP_S_CANCEL;
	
	if (!(dwKeyState &MK_LBUTTON)) {
		// End dragging image
        m_ilDragImage.DragLeave(CWnd::GetDesktopWindow());
        m_ilDragImage.EndDrag();
        m_ilDragImage.DeleteImageList(); 

		return DRAGDROP_S_DROP;
	}

	return S_OK;
}

// (r.gonet 08/22/2011) - PLID 45555 - Added

// LabCustomFieldsView.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "LabCustomField.h"
#include "LabCustomFieldTemplate.h"
#include "LabCustomFieldsView.h"

using namespace NXDATALIST2Lib;

// (r.gonet 09/21/2011) - PLID 45555 - 3000 controls allowed maximum
#define MIN_IDC_CUSTOM_CONTROLS 1000
#define MAX_IDC_CUSTOM_CONTROLS 3999
#define SB_WHEELUP 33000
#define SB_WHEELDOWN 33001

// CLabCustomFieldsView dialog

IMPLEMENT_DYNAMIC(CLabCustomFieldsView, CNxDialog)

// (r.gonet 09/21/2011) - PLID 45555 - Create a new view that will show the custom lab field controls.
CLabCustomFieldsView::CLabCustomFieldsView(CWnd* pParent, CCFTemplateInstance *pTemplateInstance, CLabCustomFieldControlManager::EEditMode emEditMode/*=emNone*/)
	: CNxDialog(CLabCustomFieldsView::IDD, pParent)
{
	// The scrolling speed is set to moderate
	m_szPixelsPerScrollTick.SetSize(10, 10);
	// Initialize scroll position to the top left
	m_szScrollPos = CSize(0, 0);
	m_emEditMode = emEditMode;
	m_pTemplateInstance = pTemplateInstance;
	// We may or may not have an instance. But that's okay. If things are null, we are robust enough to display nothing.
	if(pTemplateInstance != NULL) {
		// Assuming we do have an instance, create a go between between us and the fields in the instance.
		m_pControlManager = new CLabCustomFieldControlManager(this, MIN_IDC_CUSTOM_CONTROLS, MAX_IDC_CUSTOM_CONTROLS, pTemplateInstance, emEditMode);
	} else {
		m_pControlManager = NULL;
	}
	m_bShowControls = false;
}

// (r.gonet 09/21/2011) - PLID 45555
CLabCustomFieldsView::~CLabCustomFieldsView()
{
	if(m_pControlManager != NULL) {
		delete m_pControlManager;
	}
}

void CLabCustomFieldsView::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLabCustomFieldsView, CNxDialog)
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// CLabCustomFieldsView message handlers

// (r.gonet 09/21/2011) - PLID 45555 - Handle mouse wheel scrolling
LRESULT CLabCustomFieldsView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_KEYDOWN: 
		{
			WORD wScrollNotify = 0xFFFF;

			switch (wParam) 
			{ 
				case VK_UP: 
					wScrollNotify = SB_LINEUP; 
					break; 
	 
				case VK_PRIOR: 
					wScrollNotify = SB_PAGEUP; 
					break; 
	 
				case VK_NEXT: 
					wScrollNotify = SB_PAGEDOWN; 
					break; 
	 
				case VK_DOWN: 
					wScrollNotify = SB_LINEDOWN; 
					break; 
	 
				case VK_HOME: 
					wScrollNotify = SB_TOP; 
					break; 
	 
				case VK_END: 
					wScrollNotify = SB_BOTTOM; 
					break; 
			} 
	 
			if (wScrollNotify != -1) 
				SendMessage(WM_VSCROLL, MAKELONG(wScrollNotify, 0), 0L); 
	 
			break; 
		}
		break;
	case WM_MOUSEWHEEL:
		try {
			short zDelta = HIWORD(wParam);
			if (zDelta > 0) {
				// The user scrolled "forward", or "up", or "away from the user"
				for (long z = 0; z < zDelta; z += WHEEL_DELTA) {
					// Scroll up vertically
					int nNotches = zDelta / WHEEL_DELTA;
					OnVScroll(SB_WHEELUP, nNotches, NULL);
				}
			} else if (zDelta < 0) {
				// The user scrolled "back", or "down", or "nearer to the user"
				for (long z = 0; z > zDelta; z -= WHEEL_DELTA) {
					// Scroll down vertically
					int nNotches = zDelta / WHEEL_DELTA;
					OnVScroll(SB_WHEELDOWN, nNotches, NULL);
				}
			}
			// Since we handled it by scrolling, then don't let anyone else handle the scrollwheel message
			return FALSE;
		} NxCatchAll("CLabCustomFieldsView::WindowProc:WM_MOUSEWHEEL");
		break;

	default:
		break;
	}
	return CWnd::WindowProc(message, wParam, lParam);
}

// (r.gonet 09/21/2011) - PLID 45555
BOOL CLabCustomFieldsView::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		// (r.gonet 09/21/2011) - PLID 45555 - Initialization made simple. Tell the instance to go recreate the fields. Also reset the scrollbars.
		RefreshFields();
	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CLabCustomFieldsView::IsScrollBarVisible(HWND hWndParent, int nBar)
{
	SCROLLBARINFO sbi;
	sbi.cbSize = sizeof(SCROLLBARINFO);
	::GetScrollBarInfo(hWndParent, nBar == SB_HORZ ? OBJID_HSCROLL : OBJID_VSCROLL, &sbi);
	return ((sbi.rgstate[0] & STATE_SYSTEM_INVISIBLE) == 0);
}

// (r.gonet 09/21/2011) - PLID 45555 - Handle vertical scrolling
void CLabCustomFieldsView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	try {
		const int lineOffset = 1;
		CRect rcThisWindow;
		this->GetParent()->GetWindowRect(rcThisWindow);

		int nMinScrollPos, nMaxScrollPos;
		GetScrollRange(SB_VERT, &nMinScrollPos, &nMaxScrollPos);

		double dPages;
		long nDeltaY;
		switch(nSBCode)
		{
		case SB_LINEUP:
			// Up arrow button on scrollbar was pressed.
			ScrollTo(m_szScrollPos.cx, m_szScrollPos.cy - lineOffset);
			break;

		case SB_LINEDOWN:
			// Down arrow button on scrollbar was pressed.
			ScrollTo(m_szScrollPos.cx, m_szScrollPos.cy + lineOffset);
			break;

		case SB_PAGEUP:
			// User clicked inbetween up arrow and thumb.
			dPages = m_pControlManager->GetBoundingBox().Height() / (double)m_pControlManager->GetDrawingRect().Height();
			nDeltaY = -(long)(nMaxScrollPos / dPages);
			ScrollTo(m_szScrollPos.cx, m_szScrollPos.cy + nDeltaY);
			break;

		case SB_PAGEDOWN:
			// User clicked inbetween thumb and down arrow.
			dPages = m_pControlManager->GetBoundingBox().Height() / (double)m_pControlManager->GetDrawingRect().Height();
			nDeltaY = (long)(nMaxScrollPos / dPages);
			ScrollTo(m_szScrollPos.cx, m_szScrollPos.cy + nDeltaY);
			break;

		case SB_THUMBTRACK:
			// Scrollbar thumb is being dragged.
			ScrollTo(m_szScrollPos.cx, nPos);
			break;

		case SB_THUMBPOSITION:
			// Scrollbar thumb was released.
			ScrollTo(m_szScrollPos.cx, nPos);
			break;

		case SB_TOP:
			ScrollTo(m_szScrollPos.cx, nMinScrollPos);
			break;

		case SB_BOTTOM:
			ScrollTo(m_szScrollPos.cx, nMaxScrollPos);
			break;

		case SB_WHEELUP:
			ScrollTo(m_szScrollPos.cx, m_szScrollPos.cy - (abs((long)nPos) * lineOffset));
			break;

		case SB_WHEELDOWN:
			ScrollTo(m_szScrollPos.cx, m_szScrollPos.cy + (abs((long)nPos) * lineOffset));
			break;

		default:
			// We don't process other scrollbar messages.
			return;
		}
	} NxCatchAll(__FUNCTION__);
	CNxDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

// (r.gonet 09/21/2011) - PLID 45555 - Handle horizontal scrolling
void CLabCustomFieldsView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	try {
		const int lineOffset = 1;
		CRect rcThisWindow;
		this->GetParent()->GetWindowRect(rcThisWindow);

		int nMinScrollPos, nMaxScrollPos;
		GetScrollRange(SB_HORZ, &nMinScrollPos, &nMaxScrollPos);

		switch(nSBCode)
		{
		case SB_LINEUP:
			// Up arrow button on scrollbar was pressed.
			ScrollBy(-lineOffset, 0);
			break;

		case SB_LINEDOWN:
			// Down arrow button on scrollbar was pressed.
			ScrollBy(lineOffset, 0);
			break;

		case SB_PAGELEFT:
			// User clicked inbetween left arrow and thumb.
			ScrollBy(-(long)(m_pControlManager->GetDrawingSize().cx / (double)m_szPixelsPerScrollTick.cx) + lineOffset, 0);
			break;

		case SB_PAGERIGHT:
			// User clicked inbetween thumb and right arrow.
			ScrollBy((long)(rcThisWindow.Width() / (double)m_szPixelsPerScrollTick.cx) - lineOffset, 0);
			break;

		case SB_THUMBTRACK:
			// Scrollbar thumb is being dragged.
			ScrollBy(nPos - m_szScrollPos.cx, 0);
			break;

		case SB_THUMBPOSITION:
			// Scrollbar thumb was released.
			ScrollBy(nPos - m_szScrollPos.cx, 0);
			break;

		case SB_LEFT:
			ScrollBy(m_szScrollPos.cx - (m_szScrollPos.cx - nMinScrollPos), 0);
			break;

		case SB_RIGHT:
			ScrollBy(nMaxScrollPos - m_szScrollPos.cx, 0);
			break;

		default:
			// We don't process other scrollbar messages.
			return;
		}
	} NxCatchAll(__FUNCTION__);
	CNxDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

// (r.gonet 09/21/2011) - PLID 45555 - Scroll the page by a certain number of ticks in either direction.
void CLabCustomFieldsView::ScrollBy(long nDeltaX, long nDeltaY)
{
	// Scroll the window if needed.
	if ( nDeltaX != 0 )
	{
		int nMinScrollPos, nMaxScrollPos;
		GetScrollRange(SB_HORZ, &nMinScrollPos, &nMaxScrollPos);
		
		long nNextPos = m_szScrollPos.cx + nDeltaX;
		// Don't go out of bounds
		if(m_szScrollPos.cx + nDeltaX < nMinScrollPos) {
			nNextPos = 0;
		} else if(m_szScrollPos.cx + nDeltaX > nMaxScrollPos + 1) {
			nNextPos = nMaxScrollPos + 1;
		}
		nDeltaX = nNextPos - m_szScrollPos.cx;
		m_szScrollPos.cx += nDeltaX;
		SetScrollPos(SB_HORZ, m_szScrollPos.cx, TRUE);
		ScrollWindow(-nDeltaX*m_szPixelsPerScrollTick.cx, 0);
	}

	if ( nDeltaY != 0 )
	{
		int nMinScrollPos, nMaxScrollPos;
		GetScrollRange(SB_VERT, &nMinScrollPos, &nMaxScrollPos);
		
		long nNextPos = m_szScrollPos.cy + nDeltaY;
		// Don't go out of bounds
		if(m_szScrollPos.cy + nDeltaY < nMinScrollPos) {
			nNextPos = 0;
		} else if(m_szScrollPos.cy + nDeltaY > nMaxScrollPos + 1) {
			nNextPos = nMaxScrollPos + 1;
		}
		nDeltaY = nNextPos - m_szScrollPos.cy;
		m_szScrollPos.cy += nDeltaY;
		SetScrollPos(SB_VERT, m_szScrollPos.cy, TRUE);
		ScrollWindow(0, -nDeltaY*m_szPixelsPerScrollTick.cy);
	}
}

// (r.gonet 09/21/2011) - PLID 45555 - Scroll the page to a certain number ticks
void CLabCustomFieldsView::ScrollTo(long nX, long nY)
{
	// Scroll the window if needed.
	{
		int nMinScrollPos, nMaxScrollPos;
		GetScrollRange(SB_HORZ, &nMinScrollPos, &nMaxScrollPos);

		// Don't go out of bounds
		if(nX < nMinScrollPos) {
			nX = 0;
		} else if(nX > nMaxScrollPos + 1) {
			nX = nMaxScrollPos + 1;
		}
		long nDeltaX = nX - m_szScrollPos.cx;
		if(nDeltaX != 0) {
			m_szScrollPos.cx = nX;
			SetScrollPos(SB_HORZ, m_szScrollPos.cx, TRUE);
			ScrollWindow(-nDeltaX*m_szPixelsPerScrollTick.cx, 0);
		}
	}

	{
		int nMinScrollPos, nMaxScrollPos;
		GetScrollRange(SB_VERT, &nMinScrollPos, &nMaxScrollPos);
		
		// Don't go out of bounds
		if(nY < nMinScrollPos) {
			nY = 0;
		} else if(nY > nMaxScrollPos + 1) {
			nY = nMaxScrollPos + 1;
		}
		long nDeltaY = nY - m_szScrollPos.cy;
		if(nDeltaY != 0) {
			m_szScrollPos.cy = nY;
			SetScrollPos(SB_VERT, m_szScrollPos.cy, TRUE);
			ScrollWindow(0, -nDeltaY*m_szPixelsPerScrollTick.cy);
		}
	}
}

void CLabCustomFieldsView::ShowControls(bool bShow)
{
	if(m_bShowControls != bShow) {
		m_bShowControls = bShow;
		RefreshFields();
	}
}

// (r.gonet 09/21/2011) - PLID 45555 - Allow replacement of the shown template instance after creating the dialog.
void CLabCustomFieldsView::SetTemplateInstance(CCFTemplateInstance *pTemplateInstance)
{
	m_pTemplateInstance = pTemplateInstance;
	delete m_pControlManager;
	if(pTemplateInstance != NULL) {
		m_pControlManager = new CLabCustomFieldControlManager(this, MIN_IDC_CUSTOM_CONTROLS, MAX_IDC_CUSTOM_CONTROLS, pTemplateInstance, m_emEditMode);
	} else {
		m_pControlManager = NULL;
	}
}

// (r.gonet 09/21/2011) - PLID 45555 - Redraw all of the fields' controls.
void CLabCustomFieldsView::RefreshFields(bool bRedrawOnly/*= false*/)
{
	if(m_pTemplateInstance == NULL || m_pControlManager == NULL) {
		ShowScrollBar(SB_BOTH, FALSE);
		return;
	}

	// Reset the scrollbar positions and tick amounts
	m_szScrollPos = CSize(0, 0);
	SetScrollPos(SB_HORZ, m_szScrollPos.cx, TRUE);
	SetScrollPos(SB_VERT, m_szScrollPos.cy, TRUE);
	CRect rcWindowRect;
	GetClientRect(&rcWindowRect);
	m_pControlManager->SetDrawingRect(CRect(10, 0, rcWindowRect.Width() - 20, rcWindowRect.Height() - 20));
	
	// (r.gonet 09/21/2011) - PLID 45555 - Recreate the controls
	if(m_bShowControls && !bRedrawOnly) {
		m_pControlManager->RemoveAllControls();
		if(m_emEditMode == CLabCustomFieldControlManager::emDefaultValues) {
			m_pTemplateInstance->GetTemplate()->CreateControls(*m_pControlManager);
		} else {
			m_pTemplateInstance->CreateControls(*m_pControlManager);
		}
	}
	// Calculate the ideal scroll range based on the updated controls.
	long nScrollableHeight = m_pControlManager->GetBoundingBox().bottom - m_pControlManager->GetDrawingRect().Height();
	nScrollableHeight = max(nScrollableHeight, 0);
	long nTotalTicks = (long)(nScrollableHeight / (double)m_szPixelsPerScrollTick.cy);
	if(nTotalTicks == 0) {
		ShowScrollBar(SB_VERT, FALSE);
	} else {
		ShowScrollBar(SB_VERT, TRUE);
	}
	SetScrollRange(SB_VERT, 0, nTotalTicks);

	// Same for horizontal
	long nScrollableWidth = m_pControlManager->GetBoundingBox().right - m_pControlManager->GetDrawingRect().Width();
	nScrollableWidth = max(nScrollableWidth, 0);
	nTotalTicks = (long)(nScrollableWidth / (double)m_szPixelsPerScrollTick.cx);
	if(nTotalTicks == 0) {
		ShowScrollBar(SB_HORZ, FALSE);
	} else {
		ShowScrollBar(SB_HORZ, TRUE);
	}
	SetScrollRange(SB_HORZ, 0, nTotalTicks);

	// Adding scrollbars may have messed our window size up
	GetClientRect(&rcWindowRect);
	m_pControlManager->SetDrawingRect(CRect(10, 0, rcWindowRect.Width() - 20, rcWindowRect.Height() - 20));
} 

// (r.gonet 09/21/2011) - PLID 45555 - Save the control values back to the fields.
bool CLabCustomFieldsView::SyncControlsToFields()
{
	if(m_pControlManager == NULL) {
		// Hmm
		return true;
	}

	return m_pControlManager->SyncControlsToFields();
}

// (r.gonet 09/21/2011) - PLID 45555 - If we ever get resized, which we do in some odd circumstances, redraw the controls to account for the new size.
void CLabCustomFieldsView::OnSize(UINT nType, int cx, int cy)
{
	CNxDialog::OnSize(nType, cx, cy);
	try {
		RefreshFields(true);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/21/2011) - PLID 45555 - Focus in on the view if the user clicks on the background in order to do scrolling with the mouse.
void CLabCustomFieldsView::OnLButtonDown(UINT nFlags, CPoint point)
{
	try {
		SetFocus();
	} NxCatchAll(__FUNCTION__);
	CNxDialog::OnLButtonDown(nFlags, point);
}

// (r.gonet 09/21/2011) - PLID 45555 - Scrolls to the given field.
void CLabCustomFieldsView::EnsureFieldInView(CLabCustomFieldPtr pField)
{
	if(m_pControlManager && pField) {
		int nMinScrollPosX, nMaxScrollPosX;
		GetScrollRange(SB_HORZ, &nMinScrollPosX, &nMaxScrollPosX);
		int nMinScrollPosY, nMaxScrollPosY;
		GetScrollRange(SB_VERT, &nMinScrollPosY, &nMaxScrollPosY);
		CPoint ptScrolledPosition(
			m_szPixelsPerScrollTick.cx * (GetScrollPos(SB_HORZ) - nMinScrollPosX),
			m_szPixelsPerScrollTick.cy * (GetScrollPos(SB_VERT) - nMinScrollPosY));
		CRect rcClippedRect(ptScrolledPosition, m_pControlManager->GetDrawingSize());

		CRect rcFieldRect = CRect(0,0,0,0);
		m_pControlManager->GetFieldRectangle(pField, rcFieldRect);
		rcFieldRect.OffsetRect(ptScrolledPosition);

		if (IsScrollBarVisible(GetSafeHwnd(), SB_HORZ)) {
			if(rcFieldRect.left < rcClippedRect.left || rcFieldRect.right > rcClippedRect.right) {
				ScrollTo(rcFieldRect.left / m_szPixelsPerScrollTick.cx, m_szScrollPos.cy);
			}
		}
		if (IsScrollBarVisible(GetSafeHwnd(), SB_VERT)) {
			if(rcFieldRect.top < rcClippedRect.top || rcFieldRect.bottom > rcClippedRect.bottom) {
				ScrollTo(m_szScrollPos.cx, rcFieldRect.top / m_szPixelsPerScrollTick.cy);
			}
		}
	}
}
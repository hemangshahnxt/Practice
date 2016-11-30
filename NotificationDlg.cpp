// NotificationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NotificationDlg.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNotificationDlg dialog


CNotificationDlg::CNotificationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNotificationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNotificationDlg)
		m_bMouseOver = false;
		m_bWindowClicked = false;
	//}}AFX_DATA_INIT
}


void CNotificationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNotificationDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNotificationDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNotificationDlg)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNotificationDlg message handlers

void CNotificationDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect rWindow;
	GetWindowRect(rWindow);
	ScreenToClient(rWindow);
	if(point.x >= rWindow.left && point.x <= rWindow.right
		&& point.y >= rWindow.top && point.y <= rWindow.bottom) {
		//We want to draw it bold.
		if(!m_bMouseOver) {
			m_bMouseOver = TRUE;
			Invalidate();
		}
	}
	else {
		//Draw it normal.
		if(m_bMouseOver) {
			m_bMouseOver = FALSE;
			Invalidate();
		}
	}
	if(nFlags & MK_LBUTTON) {
		if(point.x <= rWindow.left || point.x >= rWindow.right
			|| point.y <= rWindow.top || point.y >= rWindow.bottom) {
			//We're not in the window any more.
			if(m_bWindowClicked) {
				m_bWindowClicked = FALSE;
			}
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CNotificationDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	//Move the window as if it was a depressed button.
	CRect rWindow, rClient;
	GetWindowRect(rWindow);
	GetClientRect(rClient);
	if(point.x >= rClient.left && point.x <= rClient.right
		&& point.y >= rClient.top && point.y <= rClient.bottom) {
		if(!m_bWindowClicked) {
			m_bWindowClicked = TRUE;
		}
	}
	

	CDialog::OnLButtonDown(nFlags, point);
}

void CNotificationDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(m_bWindowClicked) {
		CRect rWindow, rClient;
		GetWindowRect(rWindow);
		rClient = rWindow;
		ScreenToClient(rClient);
		if(point.x >= rClient.left && point.x <= rClient.right
		&& point.y >= rClient.top && point.y <= rClient.bottom) {
			ReleaseNotifications();
		}
		m_bWindowClicked = FALSE;
	}

	CDialog::OnLButtonUp(nFlags, point);
}

void CNotificationDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect rWindow;
	GetClientRect(rWindow);

	// (a.walling 2007-08-07 16:42) - PLID 26996 - We were passing GetDC() as a parameter to these DrawText..
	// functions! Each GetDC() call (usually) needs a ReleaseDC(). This was causing 2 leaked GDI objects
	// each time this was called.

	// (j.jones 2008-05-01 16:23) - PLID 29874 - set background color to transparent for the DrawTextOnDialog calls

	CDC *pDC = GetDC();
	if(m_bMouseOver) {
		//We have to do the vertical centering ourselves.
		CRect rHeight(0,0,rWindow.Width(), rWindow.Height());
		DrawTextOnDialog(this, pDC, rHeight, m_strCaption, dtsTextBold, true, DT_CENTER, false, true, 0);

		DrawTextOnDialog(this, pDC, CRect(0,(rWindow.Height()/2)-(rHeight.Height()/2),rWindow.Width(), (rWindow.Height()/2)+(rHeight.Height()/2)), m_strCaption, dtsTextBold, false, DT_CENTER, false, false, 0);
	}
	else{
		CRect rHeight(0,0,rWindow.Width(), rWindow.Height());
		DrawTextOnDialog(this, pDC, rHeight, m_strCaption, dtsText, true, DT_CENTER, false, true, 0);
		
		DrawTextOnDialog(this, pDC, CRect(0,(rWindow.Height()/2)-(rHeight.Height()/2),rWindow.Width(), (rWindow.Height()/2)+(rHeight.Height()/2)), m_strCaption, dtsText, false, DT_CENTER, false, false, 0);
	}

	ReleaseDC(pDC);
	
	// Do not call CDialog::OnPaint() for painting messages
}


void CNotificationDlg::AddNotification(Notification stNotification)
{
	//First, update our array.
	bool bArrayUpdated = false;
	for(int i = 0; i < m_arNotifications.GetSize(); i++) {
		if(m_arNotifications.GetAt(i).nNotificationType == stNotification.nNotificationType) {
			bArrayUpdated = true;
			Notification stnNew;
			stnNew.bIncrement = stNotification.bIncrement;
			stnNew.nCount = stNotification.bIncrement ? m_arNotifications.GetAt(i).nCount + stNotification.nCount : stNotification.nCount;
			stnNew.nNotificationType = stNotification.nNotificationType;
			stnNew.strMessage = stNotification.strMessage;
			CString strCount;
			strCount.Format("%li", stnNew.nCount);
			stnNew.strMessage.Replace("#NOTIFICATION_COUNT#", strCount);
			m_arNotifications.SetAt(i, stnNew);
		}
	}
	if(!bArrayUpdated) {
		//Add it.
		CString strCount;
		strCount.Format("%li", stNotification.nCount);
		stNotification.strMessage.Replace("#NOTIFICATION_COUNT#", strCount);
		m_arNotifications.Add(stNotification);
	}

	//Generate the on-screen text.
	Refresh();
	
}


void CNotificationDlg::ClearNotifications(DWORD dwTypesToClear /*= -1*/)
{
	if(dwTypesToClear == -1) {
		m_arNotifications.RemoveAll();
	}
	else {
		CArray<Notification, Notification&> arNew;
		for(int i = 0; i < m_arNotifications.GetSize(); i++) {
			if(!(m_arNotifications.GetAt(i).nNotificationType & dwTypesToClear) ) {
				arNew.Add(m_arNotifications.GetAt(i));
			}
		}
		m_arNotifications.RemoveAll();
		for(i=0; i < arNew.GetSize(); i++) m_arNotifications.Add(arNew.GetAt(i));
	}
	if(m_arNotifications.GetSize()) {
		Refresh();
	}
	else {
		m_strCaption = "";
		m_bWindowClicked = FALSE;
		m_bMouseOver = FALSE;
		ShowWindow(SW_HIDE);
	}
}

void CNotificationDlg::ReleaseNotifications()
{
	DWORD nTypes = 0;
	for(int i = 0; i < m_arNotifications.GetSize(); i++) {
		nTypes |= m_arNotifications.GetAt(i).nNotificationType;
	}
	GetMainFrame()->PostMessage(NXM_NOTIFICATION_CLICKED, nTypes);
}

LRESULT CNotificationDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	
	case WM_MOUSELEAVE:
	{
		//Draw it normal.
		if(m_bMouseOver) {
			m_bMouseOver = FALSE;
			Invalidate();
		}
		if(m_bWindowClicked) {
			m_bWindowClicked = FALSE;
			CRect rWindow;
			GetWindowRect(rWindow);
		}
		return 0;
	}
	case WM_SETCURSOR:
		{
			CPoint point;
			GetCursorPos(&point);
			if(LOWORD(lParam) == HTCLIENT) {
				SetCursor(GetLinkCursor());
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.dwHoverTime = 1;
				tme.hwndTrack = GetSafeHwnd();
				TrackMouseEvent(&tme);
				return TRUE;
			}
		}
	break;
	}


	return CDialog::WindowProc(message, wParam, lParam);
}

void CNotificationDlg::OnOK() 
{
	ClearNotifications();
}

void CNotificationDlg::OnCancel() 
{
	ClearNotifications();
}

void CNotificationDlg::Refresh()
{
	//Set up our message.
	m_strCaption = "";
	for(int i = 0; i < m_arNotifications.GetSize(); i++) {
		m_strCaption += m_arNotifications.GetAt(i).strMessage + "\n";
	}
	m_strCaption.TrimRight("\n");


	// (b.cardillo 2005-03-03 16:09) - PLID 15827 - Only show if the active window is our 
	// owner, or a descendent of our owner, or (failing that) if the user wants to show 
	// notifications anyway.
	// (a.walling 2012-10-10 11:09) - PLID 53027 - GetActiveWindow will return null if this thread is not active
	if (GetActiveWindow() ||
		GetRemotePropertyInt("AlwaysShowNotifications", 1, 0, GetCurrentUserName(), true))
	{
		// (b.cardillo 2005-03-03 11:48) - PLID 15826 - Made it so every time a notification should 
		// pop up, it re-checks to make sure it's in the work area.  It checks even if the window 
		// is already visible.  If it IS already visible, it tries not to move it, as long as the 
		// current position is legal (i.e. in the work area).
		//Position ourselves near the lower-right corner of our parent window.

		// Get our ideal position
		CRect rcNew;
		{
			CRect rSelfRect;
			GetWindowRect(rSelfRect);
			if (IsWindowVisible()) {
				// We were already visible, so our ideal is not to move
				rcNew.SetRect(rSelfRect.TopLeft(), rSelfRect.BottomRight());
			} else {
				CRect rcDesktop;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
				// Subtract 5 extra pixels so that we're not flush against the lower-right edge
				rcNew = CRect(rcDesktop.BottomRight() - rSelfRect.Size() - CSize(5,5), rSelfRect.Size());
			}
		}

		// Now we have our ideal, we have to make sure we're still on the screen
		{
			CRect rcDesktop;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
			if (rcDesktop.PtInRect(rcNew.TopLeft())) {
				// Ok, we're at least partially in the work area, see if we're fully in the work area
				if (rcDesktop.PtInRect(rcNew.BottomRight())) {
					// Yep, we're in there so all is well: our ideal rect is legal
				} else {
					// Off the bottom or right edge.  Just shift us back on.
					long dx = rcNew.right - rcDesktop.right;
					long dy = rcNew.bottom - rcDesktop.bottom;
					if (dx > 0) {
						rcNew.OffsetRect(-dx - 5, 0);
					}
					if (dy > 0) {
						rcNew.OffsetRect(0, -dy - 5);
					}
				}
			} else {
				// The topleft isn't even in the work area, so forget our parent 
				// window, just put the notification near the lower-right corner 
				// of the work area itself.
				rcNew = CRect(rcDesktop.BottomRight() - rcNew.Size() - CSize(5,5), rcNew.Size());
			}
		}

		// Now move it to the new position and show it
		MoveWindow(rcNew);
		ShowWindow(SW_SHOWNOACTIVATE);
	}
	//Make sure the new caption is painted.
	Invalidate();
}

// (j.jones 2011-03-15 15:28) - PLID 42738 - added a function to see if
// any notifications exist of a given type, and renamed the "all" function
// for clarity
BOOL CNotificationDlg::HasNotification(int nNotificationType)
{
	//return TRUE only if there is a notification for our requested type
	for(int i=0; i<m_arNotifications.GetSize(); i++) {
		if(((Notification)m_arNotifications.GetAt(i)).nNotificationType == nNotificationType) {
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CNotificationDlg::HasAnyNotifications()
{
	if (m_arNotifications.GetSize()) {
		return TRUE;
	} else {
		return FALSE;
	}
}

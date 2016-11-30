// NxDialog.cpp : implementation file
//

#include "stdafx.h"
#include "NxDialog.h"
#include "NxStandard.h"
#include "NxModalParentDlg.h"
#include <WindowUtils.h>

using namespace NXDATALISTLib;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
// CNxDialog dialog

// (a.walling 2010-06-09 16:03) - PLID 39087 - Most of this is all in CNexTechDialog now. However some practice-specific stuff is kept here.

IMPLEMENT_DYNAMIC(CNxDialog, CNexTechDialog);

BEGIN_MESSAGE_MAP(CNxDialog, CNexTechDialog)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)	
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// (a.walling 2011-12-21 16:10) - PLID 46648 - Dialogs must set a parent
CNxDialog::CNxDialog(CWnd* pParent)
	: CNexTechDialog(0, pParent)
{
}

// (a.walling 2011-12-21 16:10) - PLID 46648 - Dialogs must set a parent
CNxDialog::CNxDialog(int IDD, CWnd* pParent)
	: CNexTechDialog(IDD, pParent)
{
}

// (j.armen 2012-06-05 16:04 ) - PLID 50805 - Constructor which also allows for the dialog to recall it's size and position on init
CNxDialog::CNxDialog(int IDD, CWnd* pParent, const CString& strSizeAndPositionConfigRT)
	: CNexTechDialog(IDD, pParent)
	, m_strSizeAndPositionConfigRT(strSizeAndPositionConfigRT)
{
}

CNxDialog::~CNxDialog()
{
}

void CNxDialog::Print(CDC * pDC, CPrintInfo * pInfo)
{
	if (m_nfoPrintInfo.m_bReady) 
	{	
		CRect rcDest = pInfo->m_rectDraw;
		SetAspect(rcDest, GetAspect(m_nfoPrintInfo.m_rectSurface));

		// We happen to know m_bmpNew uses the halftone palette so select it here.  If we didn't 
		// know that for sure then we'd have to store the palette it does use in a member variable 
		// whenever we get m_bmpNew and then use that member variable here.
		// (b.cardillo 2004-03-16 13:16) - Notice, we call CreateHalftonePalette here instead of 
		// just using theApp.m_palette like everywhere else.  For some insane reason the app's 
		// member variable just doesn't work here!  I debugged it for a while and had no luck.  The 
		// only explanation I can come up with is that somehow that shared palette is currently 
		// selected into someone else's device context so when we try to use it, our SelectPalette 
		// doesn't actually select it.  In any case, using the official halftone palette instead 
		// works just fine here.
		CPalette pal;
		pal.Attach(::CreateHalftonePalette(NULL));
		CPalette *pOldPal = pDC->SelectPalette(&pal, TRUE);
		pDC->RealizePalette();

		// Draw the bitmap that is just a screenshot
		//JMM - 8-24-2004 - PLID 13930 - Made it use this function because it wasn't printing out at all on most dialogs
		//DrawDIBitmapInRect(pDC, rcDest, (HBITMAP)m_bmpNew);
		// (z.manning, 3/18/2006, PLID 19521) - DrawDIBitmapInRect didn't always work for printing
		// screenshots, so now this one is used.
		DrawDIBitmapInRectForPrinting(pDC, rcDest, (HBITMAP)m_bmpNew);

		// Unselect the halftone palette so that when pal goes out of scope the palette can be 
		// correctly released back to the system.
		pDC->SelectPalette(pOldPal, TRUE);

		// (c.haag 2006-04-26 16:38) - PLID 20303 - Delete the half-tone palette
		DeleteObject(pal.Detach());

/*
this doesn't work yet
		if (!result)
		{
			if (IDYES != MessageBox
				("The installed printer driver does not support printing this document\r\n"
				"Nextech can simulate your driver, but this may take a long time.\r\n"
				"If you choose no, you will still get a lower quality image.\r\n"
				"You you like to continue printing in high quality?",
				"Continue?", MB_YESNO))
			{
				result = StretchBlt(pDC->GetSafeHdc(), 
					rcDest.left, rcDest.top, rcDest.Width(), rcDest.Height(), 
					m_dcExtra.GetSafeHdc(), 
					0, 0, bitmapinfo.bmiHeader.biWidth, bitmapinfo.bmiHeader.biHeight, 
					SRCCOPY);
			}

			else
			{
				result = StretchDIBitsSlow(pDC->GetSafeHdc(), 
					rcDest.left, rcDest.top, rcDest.Width(), rcDest.Height(), 
					0, 0, bitmapinfo.bmiHeader.biWidth, bitmapinfo.bmiHeader.biHeight, 
					p, &bitmapinfo, DIB_RGB_COLORS, SRCCOPY);
			}
		}
*/
		
/*	DRT - This makes no sense to give an error message under every circumstance ... leaving this commented out until I can find out why		
		if (IDYES == MessageBox(
			"The installed printer driver does not support printing this document\r\n"
			"You you like to continue printing in low quality?",
			"Continue?", MB_YESNO))
		{
			result = StretchBlt(pDC->GetSafeHdc(), 
				rcDest.left, rcDest.top, rcDest.Width(), rcDest.Height(), 
				m_dcExtra.GetSafeHdc(), 
				0, 0, bitmapinfo.bmiHeader.biWidth, bitmapinfo.bmiHeader.biHeight, 
				SRCCOPY);
		}
*/
	}
}

void CNxDialog::Capitalize(int ID)
{
	CString		value, oldvalue;
	int			x1, 
				x2;
	static bool IsRunning = false;
	
	if (IsRunning)
		return;
	IsRunning = true;
	CEdit *tmpEdit = (CEdit *)GetDlgItem(ID);
	GetDlgItemText (ID, value);
	oldvalue = value;
	SetUppersInString(tmpEdit, value);
	// (c.haag 2006-04-12 16:36) - PLID 20096 - Don't set the text unless it
	// changed because we don't want to fire an EN_CHANGE event if nothing
	// actually changed.
	if (oldvalue != value) {
		tmpEdit->GetSel (x1, x2);
		tmpEdit->SetWindowText (value);
		SetDlgItemText (ID, value);
		tmpEdit->SetSel (x1, x2);
	}
	IsRunning = false;
}

// (d.moore 2007-04-23 11:46) - PLID 23118 - Capitalizes all text for a DlgItem.
void CNxDialog::CapitalizeAll(int ID)
{
	CString		value, oldvalue;
	int			x1, 
				x2;
	static bool IsRunning = false;
	
	if (IsRunning)
		return;
	IsRunning = true;
	CEdit *tmpEdit = (CEdit *)GetDlgItem(ID);
	GetDlgItemText (ID, value);
	oldvalue = value;
	value.MakeUpper();
	// (c.haag 2006-04-12 16:36) - PLID 20096 - Don't set the text unless it
	// changed because we don't want to fire an EN_CHANGE event if nothing
	// actually changed.
	if (oldvalue != value) {
		tmpEdit->GetSel (x1, x2);
		tmpEdit->SetWindowText (value);
		// This seems to just reproduce the effect of the previous line.
		// Originally copied from CNexTechDialog::Capitalize()
		// SetDlgItemText (ID, value);
		tmpEdit->SetSel (x1, x2);
	}
	IsRunning = false;
}

void CNxDialog::FormatItem(int ID, CString format)
{
	CString value;
	GetDlgItemText(ID, value);
	FormatItemText(GetDlgItem(ID), value, format);
}

LPUNKNOWN CNxDialog::BindNxDataListCtrl(UINT nID, bool bAutoRequery /*= true*/)
{
	// Just call the more robust version with the default 
	// data pointer being the global remote data
	// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
	// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - GetRemoteDataOrDataSnapshot -> GetRemoteDataSnapshot
	return BindNxDataListCtrl(nID, GetRemoteDataSnapshot(), bAutoRequery);
}

//DRT 12/08/2005 - Added for version 2.0 of the datalist.
LPUNKNOWN CNxDialog::BindNxDataList2Ctrl(UINT nID, bool bAutoRequery /*= true*/)
{
	// Just call the more robust version with the default 
	// data pointer being the global remote data
	// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
	// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - GetRemoteDataOrDataSnapshot -> GetRemoteDataSnapshot
	return BindNxDataList2Ctrl(nID, GetRemoteDataSnapshot(), bAutoRequery);
}

LPUNKNOWN CNxDialog::BindNxDataListCtrl(UINT nID, LPUNKNOWN pDataConn, bool bAutoRequery /*= true*/)
{
	return ::BindNxDataListCtrl(this, nID, pDataConn, bAutoRequery);
}

//DRT 4/17/2008 - PLID 29678 - Since all dialogs now derive from NxDialog, we need the 4 parameter option
//	here.
LPUNKNOWN CNxDialog::BindNxDataListCtrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, bool bAutoRequery)
{
	return ::BindNxDataListCtrl(pParent, nID, pDataConn, bAutoRequery);
}

//DRT 4/17/2008 - PLID 29678 - Since all dialogs now derive from NxDialog, we need the 4 parameter option
//	here.
LPUNKNOWN CNxDialog::BindNxDataList2Ctrl(CWnd *pParent, UINT nID, LPUNKNOWN pDataConn, bool bAutoRequery)
{
	return ::BindNxDataList2Ctrl(pParent, nID, pDataConn, bAutoRequery);
}

LPUNKNOWN CNxDialog::BindNxDataList2Ctrl(UINT nID, LPUNKNOWN pDataConn, bool bAutoRequery /*= true*/)
{
	return ::BindNxDataList2Ctrl(this, nID, pDataConn, bAutoRequery);
}

// (j.jones 2008-12-30 11:30) - PLID 32585 - added OnKickIdle to handle when a modal NxDialog is idle
LRESULT CNxDialog::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
	//WM_KICKIDLE is received by a dialog when that dialog is idle

	try {

		//lParam is the idle count

		//tell mainfrm to pump the next barcode message
		if(GetMainFrame()) {
			GetMainFrame()->Idle((int)lParam);
		}

	}NxCatchAll("Error in CNexTechDialog::OnKickIdle");

	return 0;
}

// (z.manning 2010-07-07 14:30) - PLID 36511
void CNxDialog::EndDialogAndModalParent(int nResult)
{
	if(GetParent() != NULL && IsWindow(GetParent()->GetSafeHwnd()) && GetParent()->IsKindOf(RUNTIME_CLASS(CNxModalParentDlg))) {
		((CNxModalParentDlg*)GetParent())->EndDialog(nResult);
	}
	else {
		this->EndDialog(nResult);
	}
}

// (j.armen 2012-06-05 16:04 ) - PLID 50805 - If calling before CNxDialog::OnInitDialog, set bRecallNow = false
//	as the size will be recalled on CNxDialog::OnInitDialog.  If after CNxDialog::OnInitDialog, set bRecallNow = true
//	so that this function knows that it is responsible for recalling the dialog size
void CNxDialog::SetRecallSizeAndPosition(const CString& strSizeAndPositionConfigRT, bool bRecallNow)
{
	m_strSizeAndPositionConfigRT = strSizeAndPositionConfigRT;
	if(bRecallNow)
		RecallSizeAndPosition();
}

// (j.armen 2012-06-05 16:04 ) - PLID 50805 - Function for recalling dialog size and position from data
void CNxDialog::RecallSizeAndPosition()
{
	// If we don't have a config entry, we are done here
	if(m_strSizeAndPositionConfigRT.IsEmpty())
		return;

	// Cache the property
	g_propManager.CachePropertiesInBulk("NxDialogSize-%s", propText,
		"(Username = '<None>' OR Username = '%s') AND (Name LIKE 'NxDialogSize-%s')",
		m_strSizeAndPositionConfigRT, _Q(GetCurrentUserComputerName()), _Q(m_strSizeAndPositionConfigRT));

	// By default, we will prefer to center windows
	CenterWindow();

	// Get the current window settings just in case so that we have a default param
	CRect rc;
	GetWindowRect(rc);

	//Max, Left, Top, Width, Height
	CArray<long, long> ary;
	ary.SetSize(5);
	ary[0] = IsZoomed();
	ary[1] = rc.left;
	ary[2] = rc.top;
	ary[3] = rc.Width();
	ary[4] = rc.Height();

	// Get the stored property
	CString strConfig = GetRemotePropertyText(
		"NxDialogSize-" + m_strSizeAndPositionConfigRT, 
		ArrayAsString(ary),
		propText,
		GetCurrentUserComputerName());

	// Convert into an array
	StringAsArray(strConfig, ary);

	// if it's not size of 5, then there is bad data, return now.  The defaults of the dlg will be used.
	if(ary.GetSize() != 5)
		return;

	if(ary[0]) {
		// This is the Maximize flag, if it's true and the dlg can be maximized, then do so
		if(GetStyle() & WS_MAXIMIZEBOX) {
			ShowWindow(SW_MAXIMIZE);
		}
		else {
			//Either bad data, or the dialog was previously able to be maximized, but can be no longer, therefore ignore.
			//We will continue with the default size of the dlg
		}
	}
	else {
		// Store whether or not we are resizeable.
		// If we are resizeable, we will restore size and position.  If not, then we will only restore position.
		// This handles the case where a non-resizeable dialog's size is changed
		bool bAllowSizing = IsResizeable();
		CRect rcRemembered = CRect(
			ary[1], ary[2], 
			ary[1] + (bAllowSizing ? ary[3] : rc.Width()), 
			ary[2] + (bAllowSizing ? ary[4] : rc.Height()));

		// Get the work area available to use
		CRect rcDesktop;
		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		if (GetMonitorInfo(MonitorFromPoint(rc.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi)) {
			rcDesktop = mi.rcWork;
		}
		else {
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
		}

		// Use our remembered coords releative to the desktop
		rcRemembered.top += rcDesktop.top;
		rcRemembered.bottom += rcDesktop.top;
		rcRemembered.left += rcDesktop.left;
		rcRemembered.right += rcDesktop.left;
		

		// Go through the remembered settings vs. the desktop availablity.  If the window is off the screen, then resize/position it accordingly
		if(rcDesktop.right < rcRemembered.right)
		{
			rcRemembered.left = rcDesktop.right - rcRemembered.Width();
			rcRemembered.right = rcDesktop.right;
			if(bAllowSizing && rcDesktop.left > rcRemembered.left) {
				rcRemembered.left = rcDesktop.left;
			}
		}

		if(rcDesktop.bottom < rcRemembered.bottom)
		{
			rcRemembered.top = rcDesktop.bottom - rcRemembered.Height();
			rcRemembered.bottom = rcDesktop.bottom;
			if(bAllowSizing && rcDesktop.top > rcRemembered.top) {
				rcRemembered.top = rcDesktop.top;
			}
		}

		if(rcDesktop.left > rcRemembered.left)
		{
			rcRemembered.right = rcDesktop.left + rcRemembered.Width();
			rcRemembered.left = rcDesktop.left;
			if(bAllowSizing && rcDesktop.right < rcRemembered.right) {
				rcRemembered.right = rcDesktop.right;
			}
		}

		if(rcDesktop.top > rcRemembered.top)
		{
			rcRemembered.bottom = rcDesktop.top + rcRemembered.Height();
			rcRemembered.top = rcDesktop.top;
			if(bAllowSizing && rcDesktop.bottom < rcRemembered.bottom) {
				rcRemembered.bottom = rcDesktop.bottom;
			}
		}

		// Finally, move the window
		MoveWindow(rcRemembered);		
	}
}

// (j.armen 2012-06-05 16:04 ) - PLID 50805 - Function to save the size and position of a dialog
void CNxDialog::SaveSizeAndPosition()
{
	// If there is no config entry, then nothing we can do here
	if(m_strSizeAndPositionConfigRT.IsEmpty())
		return;

	//Max, Left, Top, Width, Height
	//As long as the window is not minimized, then we can save the property
	if(!IsIconic())
	{
		CRect rc;
		GetWindowRect(rc);

		// Store the value in the cordinate space of the 
		CRect rcDesktop;
		MONITORINFO mi;
		mi.cbSize = sizeof(MONITORINFO);
		if (GetMonitorInfo(MonitorFromPoint(rc.TopLeft(), MONITOR_DEFAULTTONEAREST), &mi)) {
			rcDesktop = mi.rcWork;
		}
		else {
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
		}

		// Save our coords as if the dlg was at 0,0
		CArray<long, long> ary;
		ary.SetSize(5);
		ary[0] = IsZoomed();
		ary[1] = rc.left - rcDesktop.left;
		ary[2] = rc.top - rcDesktop.top;
		ary[3] = rc.Width();
		ary[4] = rc.Height();

		SetRemotePropertyText("NxDialogSize-" + m_strSizeAndPositionConfigRT,
			ArrayAsString(ary),
			propText,
			GetCurrentUserComputerName());
	}
}

// (j.armen 2012-06-05 16:04 ) - PLID 50805 - OnInitDialog, recall the size and position
BOOL CNxDialog::OnInitDialog()
{
	CNexTechDialog::OnInitDialog();

	try
	{
		RecallSizeAndPosition();
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (j.armen 2012-06-05 16:04 ) - PLID 50805 - OnDestroy, save the size and position
void CNxDialog::OnDestroy()
{
	try
	{
		SaveSizeAndPosition();
	}NxCatchAll(__FUNCTION__);

	CNexTechDialog::OnDestroy();
}

///

// (a.walling 2013-02-05 12:15) - PLID 55018 - CNxModelessDialog - CNxDialog-derived class for modeless dialogs, both owned or not.

IMPLEMENT_DYNAMIC(CNxModelessDialog, CNxDialog);

BEGIN_MESSAGE_MAP(CNxModelessDialog, CNxDialog)
END_MESSAGE_MAP()

CNxModelessDialog::CNxModelessDialog(CWnd* pParent)
	: CNxDialog(pParent)
{}

CNxModelessDialog::CNxModelessDialog(int IDD, CWnd *pParent)
	: CNxDialog(IDD, pParent)
{}

CNxModelessDialog::CNxModelessDialog(int IDD, CWnd *pParent, const CString& strSizeAndPositionConfigRT)
	: CNxDialog(IDD, pParent, strSizeAndPositionConfigRT)
{}

CNxModelessDialog::~CNxModelessDialog()
{}

// will always add the WS_EX_APPWINDOW exstyle
// override and bypass if you want to avoid this
void CNxModelessDialog::PreSubclassWindow()
{
	ModifyStyleEx(0, WS_EX_APPWINDOW, 0);

	__super::PreSubclassWindow();
}

// hides, but does not destroy, the dialog
void CNxModelessDialog::CloseDialog()
{
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}

// show or hide the dialog, calling CloseDialog if SW_HIDE, otherwise adapting the standard ShowWindow commands
void CNxModelessDialog::ShowDialog(int nCmd)
{
	if (nCmd == SW_HIDE) {
		CloseDialog();
	} else {
		ShowWindow(nCmd);
	}
}

///

IMPLEMENT_DYNAMIC(CNxModelessOwnedDialog, CNxModelessDialog);

BEGIN_MESSAGE_MAP(CNxModelessOwnedDialog, CNxModelessDialog)
	ON_WM_CLOSE()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_ACTIVATE()
	ON_WM_ACTIVATEAPP()
	ON_MESSAGE(WM_ACTIVATETOPLEVEL, OnActivateTopLevel)
END_MESSAGE_MAP()

CNxModelessOwnedDialog::CNxModelessOwnedDialog(CWnd* pParent)
	: CNxModelessDialog(pParent)
{}

CNxModelessOwnedDialog::CNxModelessOwnedDialog(int IDD, CWnd *pParent)
	: CNxModelessDialog(IDD, pParent)
{}

CNxModelessOwnedDialog::CNxModelessOwnedDialog(int IDD, CWnd *pParent, const CString& strSizeAndPositionConfigRT)
	: CNxModelessDialog(IDD, pParent, strSizeAndPositionConfigRT)
{}

CNxModelessOwnedDialog::~CNxModelessOwnedDialog()
{}

void CNxModelessOwnedDialog::OnClose()
{
	CloseDialog();
	__super::OnClose();
}

BOOL CNxModelessOwnedDialog::DestroyWindow()
{
	PassFocus();
	return __super::DestroyWindow();
}

// overrides to ensure PassFocus is called
void CNxModelessOwnedDialog::OnOK()
{
	CNxModelessOwnedDialog::EndDialog(IDOK);
}

// overrides to ensure PassFocus is called
void CNxModelessOwnedDialog::OnCancel()
{
	CNxModelessOwnedDialog::EndDialog(IDCANCEL);
}

// overrides to ensure PassFocus is called
void CNxModelessOwnedDialog::EndDialog(int nResult)
{
	::SetWindowLongPtr(*this, DWL_MSGRESULT, nResult);
	CloseDialog();
}

// calls PassFocus, and then hides the window. Without PassFocus, the owner will be activated.
void CNxModelessOwnedDialog::CloseDialog()
{
	PassFocus();

	__super::CloseDialog();
}

void CNxModelessOwnedDialog::ShowWindow(int nCmd)
{
	// (a.walling 2014-05-06 08:44) - PLID 57388 - Pass focus on anything that does not show and activate. Previously just handled SW_SHOW.
	switch (nCmd) {
	case SW_SHOWNORMAL:
	case SW_SHOWMAXIMIZED:
	case SW_SHOW:
	case SW_RESTORE:
	case SW_SHOWDEFAULT:
		break;
	default:
		PassFocus();
		break;
	}

	__super::ShowWindow(nCmd);
}

// Passes focus to next top-level window in the thread.
void CNxModelessOwnedDialog::PassFocus()
{
	if (!GetWindow(GW_OWNER)) {
		// not an owned window!
		return;
	}

	// set focus / active window elsewhere
	CWnd* pActiveWindow = GetActiveWindow();

	if (pActiveWindow != this) {
		return;
	}

	struct FindNextTopLevelWindowToActivate
	{
		FindNextTopLevelWindowToActivate(HWND exclude = NULL)
			: exclude(exclude)
			, target(NULL)
		{}

		HWND exclude;
		HWND target;

		BOOL operator()(HWND hwnd)
		{
			// ignore the exclude window, and also ignore if not visible or topmost window
			if (hwnd == exclude || !::IsWindowVisible(hwnd) || (WS_EX_TOPMOST & ::GetWindowLongPtr(hwnd, GWL_EXSTYLE)) || ::IsIconic(hwnd) ) {
				return TRUE;
			}

			target = hwnd;
			return FALSE;
		}
	};

	FindNextTopLevelWindowToActivate f(*pActiveWindow);

	//WindowUtils::EnumThreadWindows(GetCurrentThreadId(), &f);
	// Might as well just activate the next top-level window to be consistent with usual windows, otherwise one of our threads' windows may just
	// pop up over the current dialog
	WindowUtils::EnumWindows(&f);

	if (!f.target) {
		f.target = AfxGetMainWnd()->GetSafeHwnd();
	}

	::SetActiveWindow(f.target);
	::SetFocus(f.target);
}

// (a.walling 2014-06-09 09:37) - PLID 57388 - Placeholders - useful for debugging without rebuilding everything

void CNxModelessOwnedDialog::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	__super::OnWindowPosChanging(lpwndpos);
}

void CNxModelessOwnedDialog::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
	__super::OnActivate(nState, pWndOther, bMinimized);
}

void CNxModelessOwnedDialog::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	__super::OnActivateApp(bActive, dwThreadID);
}

LRESULT CNxModelessOwnedDialog::OnActivateTopLevel(WPARAM wParam, LPARAM lParam)
{
	return __super::OnActivateTopLevel(wParam, lParam);
}


// (a.walling 2013-11-11 14:17) - PLID 59412 - so we don't have to write the if (*lppNewSel == NULL) { SafeSetComPtr / etc boilerplate over and over again.
void CNxDialog::RequireDataListSel(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(lppNewSel && *lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (a.walling 2013-11-27 15:55) - PLID 59857 - AutoSet as member of CNxDialog
bool CNxDialog::AutoSet(int id, NXB_TYPE type, DWORD attributes)
{
	return AutoSet(GetDlgItem(id), type, attributes);
}

// (a.walling 2013-11-27 15:55) - PLID 59857 - AutoSet as member of CNxDialog
bool CNxDialog::AutoSet(CWnd* pWnd, NXB_TYPE type, DWORD attributes)
{
	if (!pWnd->GetSafeHwnd()) {
		return false;
	}

	// ensure we have a CNexTechIconButton; then we just call the CNxIconButton function, regardless of whether this
	// is an actual CNxIconButton or not, since it has no new members or virtual overrides beyond CNexTechIconButton

	//CNxIconButton* pNxib = static_cast<CNxIconButton*>(boost::polymorphic_downcast<CNexTechIconButton*>(pWnd));

	// dynamic_cast is not too expensive really, not that big of a deal for a one-time thing like this
	CNxIconButton* pNxib = static_cast<CNxIconButton*>(boost::polymorphic_cast<CNexTechIconButton*>(pWnd));

	return pNxib->AutoSet(type, attributes);
}

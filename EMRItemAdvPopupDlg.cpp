// EMRItemAdvPopupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EMRItemAdvPopupDlg.h"
#include "EMRItemAdvListDlg.h"
#include "EmrUtils.h"
#include "EMNDetail.h"
#include "EMRTopic.h"
#include "EMN.h"
#include "EmrItemAdvTableDlg.h"
#include "EmrTreeWnd.h"
#include "EMR.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CPracticeApp theApp;

using namespace ADODB;

//DRT 7/29/2008 - PLID 30824 - removed an errant "using namespace NXDATALISTLIB" from here -- the table
//	type now uses NXDATALIST2Lib, but regardless there are no references to any datalists in this file.
/////////////////////////////////////////////////////////////////////////////
// CEMRItemAdvPopupDlg dialog

CEMRItemAdvPopupDlg::CEMRItemAdvPopupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEMRItemAdvPopupDlg::IDD, pParent)
{
	m_nCurPenColor = 0;
	m_fltCurPenSize = 53;
	m_bIsEraserState = FALSE;
	m_EMRItemAdvPopupWnd = NULL;
	m_bIsDetailIndependent = FALSE;
	m_strCustomImageStamps = "";
	m_brBackground.CreateSolidBrush(GetNxColor(GNC_EMR_ITEM_BG, 0));
	//{{AFX_DATA_INIT(CEMRItemAdvPopupDlg)
		m_clrHilightColor = 0;
		m_pDetail = NULL;
		// (a.walling 2009-10-13 14:31) - PLID 36024
		m_pRealDetail = NULL;
	//}}AFX_DATA_INIT
}

CEMRItemAdvPopupDlg::~CEMRItemAdvPopupDlg()
{	
	// (a.walling 2009-10-13 13:51) - PLID 36024
	if (m_pDetail) {
		m_pDetail->__Release("~CEMRItemAdvPopupDlg release existing detail");
	}
	if (m_pRealDetail) {
		m_pRealDetail->__Release("~CEMRItemAdvPopupDlg release existing real detail");
	}
}

void CEMRItemAdvPopupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEMRItemAdvPopupDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_PLACEHOLDER, m_nxstaticPlaceholder);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}

BEGIN_EVENTSINK_MAP(CEMRItemAdvPopupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEMRItemAdvPopupDlg)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CEMRItemAdvPopupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEMRItemAdvPopupDlg)
	ON_WM_ERASEBKGND()
	ON_REGISTERED_MESSAGE(NXM_PRINTCLIENTINRECT, OnPrintClientInRect)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_MESSAGE(NXM_EMR_POPUP_RESIZED, OnPopupResized)
	ON_MESSAGE(NXM_EMR_POPUP_POST_STATE_CHANGED, OnPostStateChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEMRItemAdvPopupDlg message handlers

BOOL CEMRItemAdvPopupDlg::OnInitDialog() 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		CNxDialog::OnInitDialog();

		// NxPropManager cache
		// (r.gonet 02/14/2013) - PLID 40017 - Added a bulk cache.
		g_propManager.CachePropertiesInBulk("EMRItemAdvPopupDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'EMRRememberPoppedUpTableColumnWidths' " // (r.gonet 02/14/2013) - PLID 40017
			")",
			_Q(GetCurrentUserName()));

		// (c.haag 2008-04-28 11:56) - PLID 29806 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (a.walling 2012-06-07 11:59) - PLID 46649 - Use some aero glass
		UpdateCompositionFrame();

		m_pDetail->LoadContent();
		//TES 4/6/2007 - PLID 25456 - We can only lock spawning if this detail is on an independent copy.
		if(m_bIsDetailIndependent) m_pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->LockSpawning();

		m_EMRItemAdvPopupWnd = new CEMRItemAdvPopupWnd;
		m_pDetail->m_pAdvTableBaseOverride = m_EMRItemAdvPopupWnd;

		// Set member variables for the window
		m_EMRItemAdvPopupWnd->m_clrHilightColor = m_clrHilightColor;
		m_EMRItemAdvPopupWnd->m_nCurPenColor = m_nCurPenColor;
		m_EMRItemAdvPopupWnd->m_fltCurPenSize = m_fltCurPenSize;
		m_EMRItemAdvPopupWnd->m_bIsEraserState = m_bIsEraserState;
		m_EMRItemAdvPopupWnd->m_strLinkedItemList = m_strLinkedItemList;
		// (r.gonet 02/14/2013) - PLID 40017 - The popup window needs to know if it should save the column widths when the user resizes the columns in the popup.
		BOOL bRememberPoppedUpTableColumnWidths = GetRemotePropertyInt("EmrRememberPoppedUpTableColumnWidths", 0, 0, "<None>", true) != 0 ? TRUE : FALSE;
		m_EMRItemAdvPopupWnd->SetRememberPoppedUpTableColumnWidths(bRememberPoppedUpTableColumnWidths);

		//TES 8/20/2007 - PLID 25456 - Because this dialog has a child window, which in turn has a control on it 
		// (for image, table, and narrative types, anyway), we need to call this function so that keyboard controls, among
		// others, will get passed to those controls rather than eaten by the dialog.  The function is s undocumented, but 
		// I've found several forum posts and such online (mostly dealing with property sheets, but it's the same issue),
		// indicating that this is the "correct" solution.
		InitControlContainer();

		CreateControlsWindow();

		// (a.wetta 2007-03-09 10:15) - PLID 24757 - Make sure that the popup window is on top so that
		// its controls get the focus first and not the OK and Cancel button on the dialog.
		// (a.walling 2009-06-22 10:42) - PLID 34635 - Like most SetWindoPos things having to do with resizing or 
		// repositioning, pass in the SWP_NOACTIVATE flag as well. CenterWindow() does this for example.
		m_EMRItemAdvPopupWnd->SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEMRItemAdvPopupDlg::OnDestroy()
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		CDialog::OnDestroy();

		if (m_EMRItemAdvPopupWnd) {
			if(IsWindow(m_EMRItemAdvPopupWnd->GetSafeHwnd())) {
				m_EMRItemAdvPopupWnd->DestroyWindow();
			}
			delete m_EMRItemAdvPopupWnd;
		}
	} NxCatchAll("Error in OnDestroy");
}

void CEMRItemAdvPopupDlg::OnOK() 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		if(m_pDetail->m_EMRInfoType == eitImage) {
			//The detail may want to remember the user options.
			UpdateRememberedPenState();
			if(m_EMRItemAdvPopupWnd->m_Image != NULL) {
				// (a.wetta 2007-04-10 10:21) - PLID 25532 - Get the custom stamps
				m_strCustomImageStamps = (LPCSTR)m_EMRItemAdvPopupWnd->m_Image->CustomStamps;
			}
			else if(m_EMRItemAdvPopupWnd->m_p3DControl != NULL) {
				// (z.manning 2011-09-07 16:54) - PLID 44693
				m_strCustomImageStamps = (LPCTSTR)m_EMRItemAdvPopupWnd->m_p3DControl->CustomStamps;
			}
		}
		
		CDialog::OnOK();
	} NxCatchAll("CEMRItemAdvPopupDlg::OnOK");
}

void CEMRItemAdvPopupDlg::OnCancel()
{
	try {
		// (a.walling 2009-01-12 12:35) - PLID 29800 - Get the image properties regardless of whether the popup was cancelled or not
		if(m_pDetail->m_EMRInfoType == eitImage) {
			//The detail may want to remember the user options.
			UpdateRememberedPenState();

			if(m_EMRItemAdvPopupWnd->m_Image != NULL) {
				// (a.walling 2009-01-12 12:37) - PLID 29800 - All we really need is the custom stamps, but we might as well get all the
				// properties while we are at it for posterity.
				m_strCustomImageStamps = (LPCSTR)m_EMRItemAdvPopupWnd->m_Image->CustomStamps;
			}
			else if(m_EMRItemAdvPopupWnd->m_p3DControl != NULL) {
				// (z.manning 2011-09-07 16:54) - PLID 44693
				m_strCustomImageStamps = (LPCTSTR)m_EMRItemAdvPopupWnd->m_p3DControl->CustomStamps;
			}
		}
		
		CNxDialog::OnCancel();
	} NxCatchAll("CEMRItemAdvPopupDlg::OnCancel");
}


void CEMRItemAdvPopupDlg::CreateControlsWindow()
{
	// (j.jones 2006-07-25 10:13) - PLID 20925 - get the working area so the dialog fits on the screen
	// without being underneath the taskbar
	CRect rc;
	SystemParametersInfo(SPI_GETWORKAREA, 0, rc, 0);
	long nScreenSizeX = rc.Width();
	long nScreenSizeY = rc.Height();

	// (j.jones 2009-12-16 14:17) - PLID 31021 - we once again take a minimum size
	//(e.lally 2012-03-30) PLID 49319 - Moved the declaration up some.
	CSize szMin;
	szMin.cx = 200;
	szMin.cy = 200;

	// Calculate the max amount of space the controls can take up
	CSize szMax;
	if(m_pDetail->m_EMRInfoType == eitText) {
		CRect rc;
		GetWindowRect(&rc);
		szMax = CSize(rc.Width(),rc.Height());
	}
	else if(m_pDetail->m_EMRInfoType == eitSingleList || m_pDetail->m_EMRInfoType == eitMultiList) {
		szMax = CSize(nScreenSizeX - 60 - GetSystemMetrics(SM_CXVSCROLL), nScreenSizeY - 60 - GetSystemMetrics(SM_CYHSCROLL));
	}
	else if(m_pDetail->m_EMRInfoType == eitImage) {
		szMax = CSize(nScreenSizeX - 60 - GetSystemMetrics(SM_CXVSCROLL), nScreenSizeY - 60 - GetSystemMetrics(SM_CYHSCROLL));
	}
	else if(m_pDetail->m_EMRInfoType == eitSlider) {
		CRect rc;
		GetWindowRect(&rc);
		szMax = CSize(rc.Width(),rc.Height());
	}
	else if(m_pDetail->m_EMRInfoType == eitTable) {
		szMax = CSize(nScreenSizeX - 60 - GetSystemMetrics(SM_CXVSCROLL), nScreenSizeY - 60 - GetSystemMetrics(SM_CYHSCROLL));
		//(e.lally 2012-03-30) PLID 49319 - Increase the min width of table details
		szMin.cx = 400;
	}
	else if(m_pDetail->m_EMRInfoType == eitNarrative) {
		//TES 3/5/2008 - PLID 28827 - I changed the narrative positioning code to be the same as that for text and sliders,
		// but it turns out that that logic ends up setting the size to szMax, when embedded in this dialog.  So, I'm 
		// just passing in the "ideal" narrative size as the maximum.
		szMax.cx = 668;
		szMax.cy = 325;
	}


	if(szMin.cx > szMax.cx) {
		szMax.cx = szMin.cx;
	}
	if(szMin.cy > szMax.cy) {
		szMax.cy = szMin.cy;
	}
	
	CRect rcPlaceholder;
	GetDlgItem(IDC_PLACEHOLDER)->GetWindowRect(&rcPlaceholder);

	// (a.walling 2008-01-18 12:14) - PLID 14982 - Pass in the real detail pointer
	m_EMRItemAdvPopupWnd->Initialize(this, this, rcPlaceholder, m_pDetail, m_pRealDetail, m_bIsDetailIndependent, szMax, szMin);
}

void CEMRItemAdvPopupDlg::ResizeWindowToControls()
{
	// Get controls window's size
	CRect rcControlsWnd, rcNew;
	m_EMRItemAdvPopupWnd->GetWindowRect(&rcControlsWnd);
	ScreenToClient(&rcControlsWnd);

	// Get the dimensions of the buttons
	CRect rcOK, rcCancel;
	GetDlgItem(IDOK)->GetWindowRect(&rcOK);
	GetDlgItem(IDCANCEL)->GetWindowRect(&rcCancel);

	long nWidth, nHeight;
	//reset the width
	nWidth = 80;
	nHeight = rcOK.Height();

	rcNew = rcControlsWnd;
	rcNew.bottom += nHeight + 10;
	
	//now move the OK and Cancel buttons
	if(m_bIsDetailIndependent) {
		rcOK.left = rcNew.left + 5;
		rcOK.right = rcOK.left + nWidth;
		rcOK.top = rcNew.bottom - nHeight - 5;
		rcOK.bottom = rcOK.top + nHeight;

		rcCancel.left = rcNew.right - nWidth - 5;
		rcCancel.right = rcNew.right - 5;
		rcCancel.top = rcNew.bottom - nHeight - 5;
		rcCancel.bottom = rcCancel.top + nHeight;

		if(rcOK.right > rcCancel.left) {
			rcOK.right = (rcNew.Width() / 2) - 1;
			rcCancel.left = (rcNew.Width() / 2) + 1;
		}
	}
	else {
		//TES 4/6/2007 - PLID 25456 - If the detail isn't independent, then their changes can't be cancelled (because
		// the detail is tied to the EMN, so it already may have affect spawning, for example).  So, hide the Cancel
		// button, rename the OK button to Close, and center it.
		//TES 9/12/2007 - PLID 25456 - Don't disable the button, doing so will also stop the X button from working.
		//GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
		GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
		SetDlgItemText(IDOK, "Close");
		m_btnOK.AutoSet(NXB_CLOSE);
		rcOK.left = rcNew.Width()/2 - nWidth/2;
		rcOK.right = rcOK.left + nWidth;
		rcOK.top = rcNew.bottom - nHeight - 5;
		rcOK.bottom = rcOK.top + nHeight;
	}

	GetDlgItem(IDOK)->MoveWindow(rcOK);
	GetDlgItem(IDCANCEL)->MoveWindow(rcCancel);

	CSize szBorder;
	CalcWindowBorderSize(this, &szBorder);

	rcNew.right += szBorder.cx;
	rcNew.bottom += szBorder.cy;

	// (j.jones 2006-07-13 13:36) - PLID 20925 - this used to use &wndTopMost and SWP_NOMOVE,
	// which caused the popup dialog to occasionally display on the Patient NexEMR tab.
	// (a.wetta 2007-04-18 13:05) - PLID 24324 - When the custom stamps are changed everything in the dialog is 
	// repositioned, and because SWP_NOMOVE was taken out, the window would move around the screen when it refreshed.
	// This had originally been taken out due to the problem j.jones describes above, but it seems that it didn't need
	// to be taken out to solve his problem.  When I put it back in, it fixed the refreshing and did not
	// cause the previous problem to come back.
	// (a.walling 2009-06-22 10:42) - PLID 34635 - Like most SetWindoPos things having to do with resizing or 
	// repositioning, pass in the SWP_NOACTIVATE flag as well. CenterWindow() does this for example.
	SetWindowPos(NULL,0,0,rcNew.Width(),rcNew.Height(),SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);

	//m.hancock - 3/2/2006 - PLID 19521 - Re-center the dialog because we may have loaded
	//an image that extends past the bottom portion of the screen.
	CenterWindow();

}

// (a.walling 2009-11-19 15:08) - PLID 36365 - Deprecated massive amounts of recordset-related narrative code

// (a.walling 2008-01-18 12:16) - PLID 14982 - Added parameter for the real detail
void CEMRItemAdvPopupDlg::SetDetail(CEMNDetail* pDetail, BOOL bIsDetailIndependent, CEMNDetail* pRealDetail /* = NULL*/)
{
	// (a.walling 2009-10-13 13:51) - PLID 36024
	if (m_pDetail) {
		m_pDetail->__Release("CEMRItemAdvPopupDlg::SetDetail release existing detail");
	}
	if (m_pRealDetail) {
		m_pRealDetail->__Release("CEMRItemAdvPopupDlg::SetDetail release existing real detail");
	}
	m_pDetail = pDetail;
	m_pRealDetail = pRealDetail;
	m_bIsDetailIndependent = bIsDetailIndependent;

	// (a.walling 2009-10-13 13:52) - PLID 36024
	if (m_pDetail) {
		m_pDetail->__AddRef("CEMRItemAdvPopupDlg::SetDetail detail");
	}
	if (m_pRealDetail) {
		m_pRealDetail->__AddRef("CEMRItemAdvPopupDlg::SetDetail real detail");
	}
}

CEMNDetail* CEMRItemAdvPopupDlg::GetDetail()
{
	return m_pDetail;
}

// (a.walling 2007-08-22 17:00) - PLID 27160 - Call to reflect current state (only needed for narratives currently)
void CEMRItemAdvPopupDlg::ReflectCurrentState()
{
	if (m_EMRItemAdvPopupWnd && IsWindow(m_EMRItemAdvPopupWnd->GetSafeHwnd())) {
		if (m_pDetail && m_pDetail->m_EMRInfoType == eitNarrative) {
			// it's a narrative, ensure it's up to date.
			m_EMRItemAdvPopupWnd->ReflectNarrativeState();
		} else {
			// we don't need to do anything here.
		}
	}
}

//TES 6/3/2008 - PLID 29098 - Like ReflectCurrentState(), currently only needed for narratives.
void CEMRItemAdvPopupDlg::ReflectCurrentContent()
{
	if(m_EMRItemAdvPopupWnd && IsWindow(m_EMRItemAdvPopupWnd->GetSafeHwnd())) {
		if (m_pDetail && m_pDetail->m_EMRInfoType == eitNarrative) {
			// it's a narrative, ensure it's up to date.
			m_EMRItemAdvPopupWnd->ReflectNarrativeContent();
		} else {
			// we don't need to do anything here.
		}
	}
}

//TES 1/15/2008 - PLID 24157 - Instead of calling ResizeWindowToControls() directly, CEMRItemAdvPopupWnd now fires a
// message, which we need to handle.
LRESULT CEMRItemAdvPopupDlg::OnPopupResized(WPARAM wParam, LPARAM lParam)
{
	ResizeWindowToControls();
	return 0;
}

//TES 1/15/2008 - PLID 24157 - CEMRItemAdvPopupWnd will fire this message any time the detail's state changes.
LRESULT CEMRItemAdvPopupDlg::OnPostStateChanged(WPARAM wParam, LPARAM lParam)
{
	CEMNDetail *pDetail = (CEMNDetail*)lParam;
	//TES 1/15/2008 - PLID 24157 - If this is a single-select list, and the preference to auto-dismiss is set, then
	// auto-dismiss.
	//TES 1/17/2008 - PLID 24157 - But only if this is our detail (we occasionally might get this for other details,
	// for example if our detail is a narrative and the user clicks on a link).
	if(pDetail == m_pDetail) {
		if(pDetail->m_EMRInfoType == eitSingleList && pDetail->IsStateSet()) {
			if(GetRemotePropertyInt("EmnPopupAutoDismiss", 1, 0, GetCurrentUserName(), true)) {
				OnOK();
			}
		}
		else if(pDetail->IsLabDetail()) {
			// (z.manning 2009-09-23 09:11) - PLID 33612 - If we get this from a lab detail we need
			// to dismiss this dialog and open the lab entry dialog.
			// (z.manning 2009-10-19 10:06) - PLID 33612 - Open the lab entry dialog BEFORE we close the popup dialog.
			// (z.manning 2009-10-28 17:50) - PLID 33612 - Use the 'real' detail so we have access
			// to the tree wnd.
			if(m_pRealDetail != NULL) {
				m_pRealDetail->OpenLabEntryDialog();
			}
			else {
				pDetail->OpenLabEntryDialog();
			}
			OnOK();
		}
	}
	return 0;
}

//DRT 4/30/2008 - PLID 29771 - Handle painting the background appropriately
BOOL CEMRItemAdvPopupDlg::OnEraseBkgnd(CDC* pDC) 
{
	CRect rcClient;
	GetClientRect(rcClient);
	//

	if (IsCompositing())
	{
		pDC->FillSolidRect(rcClient, RGB(0,0,0));
	} else {
		pDC->FillRect(rcClient, &m_brBackground);
	}

	//TRUE = No more painting
	return TRUE;
}

// (z.manning 2010-05-05 16:45) - PLID 38503 - Moved some repeated code to its own function
void CEMRItemAdvPopupDlg::UpdateRememberedPenState()
{
	if(m_EMRItemAdvPopupWnd->m_Image != NULL)
	{
		m_nCurPenColor = m_EMRItemAdvPopupWnd->m_Image->GetCurrentPenColor();
		m_fltCurPenSize = m_EMRItemAdvPopupWnd->m_Image->GetCurrentPenSize();
		m_bIsEraserState = m_EMRItemAdvPopupWnd->m_Image->GetIsEraserState();
	}
}

void CEMRItemAdvPopupDlg::OnSize(UINT nType, int cx, int cy)
{
	CNxDialog::OnSize(nType, cx, cy);

	// (a.walling 2012-06-07 11:59) - PLID 46649 - Use some aero glass
	UpdateCompositionFrame();
}

// (a.walling 2012-06-07 11:59) - PLID 46649 - Use some aero glass
LRESULT CEMRItemAdvPopupDlg::OnPrintClientInRect(WPARAM wParam, LPARAM lParam)
{
	if (!IsCompositing()) {
		return CNxDialog::OnPrintClientInRect(wParam, lParam);
	}

	HDC hDC = (HDC)wParam;
	CRect rcTarget((LPRECT)lParam);
	rcTarget.OffsetRect(-rcTarget.left, -rcTarget.top);

	::SetBkColor(hDC, RGB(0,0,0));
	::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, rcTarget, NULL, 0, NULL);

	return 0x7FFFFFFF; // let caller know we handled it.
}

// (a.walling 2012-06-07 11:59) - PLID 46649 - Use some aero glass
void CEMRItemAdvPopupDlg::UpdateCompositionFrame()
{
	if (IsCompositionEnabled()) {
		if (m_EMRItemAdvPopupWnd->GetSafeHwnd()) {
			CRect rcControls;
			m_EMRItemAdvPopupWnd->GetWindowRect(&rcControls);
			ScreenToClient(&rcControls);

			if (EnableGlass(&rcControls)) {
				m_btnOK.EnableComposition();
				m_btnCancel.EnableComposition();
				return;
			}
		}
	}

	m_btnOK.EnableComposition(false);
	m_btnCancel.EnableComposition(false);
}

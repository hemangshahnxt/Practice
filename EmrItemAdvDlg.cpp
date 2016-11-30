// EmrItemAdvDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "PracticeRc.h"
#include "EmrItemAdvDlg.h"
#include "EmrItemAdvImageDlg.h"
#include "EMN.h"
#include "EMRTopic.h"
#include "EmrTopicWnd.h"
#include "EmrProblemEditDlg.h"
#include "AuditTrail.h"
#include "DontShowdlg.h"
#include "EMRProblemChooserDlg.h"
#include "EmrTreeWnd.h"
#include "WindowlessUtils.h"
#include "EmrFrameWnd.h"
#include "NxOccManager.h"
#include "SelectStampDlg.h"
#include <MiscSystemUtils.h>
#include <WindowlessTimer.h>
#include <boost/bind.hpp>
// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI
#include <NxAdvancedUILib/NxAutoHideBar.h>
#include <NxSystemUtilitiesLib/dynamic_ptr.h>
#include "EmrColors.h"
#include "EMR.h"
#include "EMNDetail.h"
#include "EMRProblemListDlg.h"
#include "EmrItemAdvImageState.h"	// (j.armen 2014-07-21 16:32) - PLID 62836

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - Various functions replaced with windowless-safe versions.

// (a.walling 2012-04-02 08:29) - PLID 49304 - Removed a lot of dead code regarding moving items

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvDlg dialog

#define HOVER_TICK 102
#define KILL_TIP_NCMOVE 104

using namespace ADODB;


// (a.walling 2012-04-03 15:20) - PLID 49377 - Only one instance will move at a time
CEmrItemAdvDlg* CEmrItemAdvDlg::g_pMovingDlg = NULL;
CEmrItemAdvDlg* CEmrItemAdvDlg::g_pSizingDlg = NULL;

////

// (a.walling 2011-11-11 11:11) - PLID 46624 - Emr item borders - no default borders in styles
// (a.walling 2011-11-11 11:11) - PLID 46634 - Create this window with the given client area
BOOL CEmrItemAdvDlg::CreateWithClientArea(DWORD dwExStyleExtra, DWORD dwStyleExtra, const CRect& rect, CWnd* pParentWnd, UINT nID)
{
	dwExStyleExtra |= WS_EX_CONTROLPARENT; // | WS_EX_NOPARENTNOTIFY;
	dwStyleExtra |= WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_GROUP | WS_TABSTOP;

	CRect rcAdjusted(rect);

	::AdjustWindowRectEx(&rcAdjusted, dwStyleExtra, FALSE, dwExStyleExtra);

	static CString g_strWindowClass;

	if (g_strWindowClass.IsEmpty()) {
		g_strWindowClass = AfxRegisterWndClass(CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW), GetDefaultBackgroundBrush(), NULL);
	}

	// (a.walling 2012-02-22 14:53) - PLID 48320 - ASSERT if this fails

	ASSERT(pParentWnd->GetSafeHwnd());

	BOOL bRet = CreateEx(
		dwExStyleExtra,
		g_strWindowClass, 
		NULL, 
		dwStyleExtra, 
		rcAdjusted, 
		pParentWnd, 
		nID
	);

	ASSERT(bRet);

	return bRet;
}

// (a.walling 2011-11-11 11:11) - PLID 46634 - Now derived from CWnd
CEmrItemAdvDlg::CEmrItemAdvDlg(class CEMNDetail *pDetail)
	: m_pDetail(pDetail)
	, m_bAutoDelete(true)
{
	m_pToolTipWnd = NULL;
	m_pBtnMergeStatus = NULL;
	m_bShowMergeStatusButton = FALSE;
	m_pBtnProblemStatus = NULL;
	m_bEnableTooltips = true;
	m_bGhostly = false;
	m_bIsTemplate = false;
	m_bMergeNameConflicts = FALSE;
	m_bIsLoading = TRUE;
	m_bPreventContentReload = FALSE;
	m_pProblemChecker = new CTableChecker(NetUtils::EMRProblemsT);
	SetNeedToRepositionControls(TRUE);
	//DRT 4/24/2008 - PLID 29771 - Set the background color
	// (d.thompson 2011-05-10) - PLID 43123 - Make the emr item background color configurable
	//m_brBackground.CreateSolidBrush(GetBackgroundColor());

	//{{AFX_DATA_INIT(CEmrItemAdvDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

}

CEmrItemAdvDlg::~CEmrItemAdvDlg()
{
	if (m_pDetail && this == m_pDetail->GetEmrItemAdvDlg()) {
		m_pDetail->ResetEmrItemAdvDlg();
	}
	m_pDetail = NULL;

	//
	// (c.haag 2006-11-14 15:04) - PLID 23158 - Delete our EMR problem
	// table checker listener
	//
	if (NULL != m_pProblemChecker) {
		delete m_pProblemChecker;
		m_pProblemChecker = NULL;
	}
	if(m_pToolTipWnd) {		
		// (a.walling 2010-06-23 17:22) - PLID 39330 - No longer necessary
		//DeleteObject(m_pToolTipWnd->m_hBrush);
		if (m_pToolTipWnd->GetSafeHwnd()) {
			m_pToolTipWnd->DestroyWindow();
		}
		delete m_pToolTipWnd;
		m_pToolTipWnd = NULL;
	}
}

BEGIN_MESSAGE_MAP(CEmrItemAdvDlg, CWnd)
	//{{AFX_MSG_MAP(CEmrItemAdvDlg)
	ON_WM_ERASEBKGND()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_DESTROY()
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_WM_MOVING()
	ON_WM_CONTEXTMENU()
	ON_WM_GETMINMAXINFO()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_NCRBUTTONDOWN()
	ON_WM_NCRBUTTONUP()
	ON_BN_CLICKED(MERGE_STATUS_IDC, OnButtonClickedMergeConflict)
	ON_BN_CLICKED(PROBLEM_STATUS_IDC, OnButtonClickedProblem)
	ON_WM_MOUSEACTIVATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	ON_WM_CTLCOLOR()
	ON_MESSAGE(NXM_EMN_ON_SHOW_TOPIC, OnShowTopic)
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrItemAdvDlg message handlers

BOOL CEmrItemAdvDlg::EnsureMergeStatusButton()
{
	if (!m_pBtnMergeStatus) {
		if (!(m_pBtnMergeStatus = new CNxIconButton)) {
			return FALSE;
		}
		//TES 1/29/2008 - PLID 28673 - Use a #defined IDC, not a numeric literal.
		// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
		if (!m_pBtnMergeStatus->Create("   ",WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_GROUP|BS_PUSHBUTTON, 
			CRect(0,0,0,0), this, MERGE_STATUS_IDC)) {
			return FALSE;
		}
		m_pBtnMergeStatus->SetIcon(IDI_MERGECONFLICT);
		extern CPracticeApp theApp;
		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
		m_pBtnMergeStatus->SetFont(CFont::FromHandle(EmrFonts::GetRegularFont()));
	}
	return TRUE;
}

BOOL CEmrItemAdvDlg::EnsureNotMergeStatusButton()
{
	if(m_pBtnMergeStatus) {
		if (m_pBtnMergeStatus->GetSafeHwnd()) {
			m_pBtnMergeStatus->DestroyWindow();
		}
		delete m_pBtnMergeStatus;
		m_pBtnMergeStatus = NULL;
	}
	return TRUE;
}

BOOL CEmrItemAdvDlg::EnsureProblemStatusButton()
{
	if (!m_pBtnProblemStatus) {
		if (!(m_pBtnProblemStatus = new CNxIconButton)) {
			return FALSE;
		}
		//TES 1/29/2008 - PLID 28673 - Use a #defined IDC, not a numeric literal.
		// (a.walling 2011-11-11 11:11) - PLID 46627 - Just create windows with an empty rect initially rather than a 1x1 rect
		if (!m_pBtnProblemStatus->Create("   ",WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_GROUP|BS_PUSHBUTTON, 
			CRect(0,0,0,0), this, PROBLEM_STATUS_IDC)) {
			return FALSE;
		}
		// (a.walling 2008-05-02 09:02) - PLID 29842 - This needs to stay capped at 16, the classic size
		m_pBtnProblemStatus->SetIcon(IDI_EMR_PROBLEM_FLAG, 16, TRUE);
		extern CPracticeApp theApp;
		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
		m_pBtnProblemStatus->SetFont(CFont::FromHandle(EmrFonts::GetRegularFont()));
	}
	return TRUE;
}

BOOL CEmrItemAdvDlg::EnsureNotProblemStatusButton()
{
	if(m_pBtnProblemStatus) {
		if (m_pBtnProblemStatus->GetSafeHwnd()) {
			m_pBtnProblemStatus->DestroyWindow();
		}
		delete m_pBtnProblemStatus;
		m_pBtnProblemStatus = NULL;
	}
	return TRUE;
}

int CEmrItemAdvDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (a.walling 2008-05-23 12:56) - PLID 30099
		if (CWnd::OnCreate(lpCreateStruct) == -1)
			return -1;

		// (a.walling 2011-11-11 11:11) - PLID 46621 - Unify font handing for emr items
		SetFont(CFont::FromHandle(EmrFonts::GetRegularFont()), FALSE);

		ReflectCurrentContent();
	} NxCatchAll("Error in OnCreate");
	
	// Return success
	return 0;
}

void CEmrItemAdvDlg::OnSize(UINT nType, int cx, int cy) 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		CWnd::OnSize(nType, cx, cy);
		
		// Remember our label rect so we can tell below whether it has changed
		CRect rcLabelPrior(0,0,0,0);
		if (IsControlValid(&m_wndLabel)) {
			GetControlChildRect(this, &m_wndLabel, &rcLabelPrior);
		}
		
		// Here's the important part: tell the derived class it's time to rearrange itself
		CRect rcWindow;
		GetWindowRect(&rcWindow);
		RepositionControls(CSize(rcWindow.Width(), rcWindow.Height()), FALSE);

		// (b.cardillo 2006-02-22 10:44) - PLID 19376 - Now since the control positions have likely 
		// changed we need to invalidate the area around the controls; also the label too, but only 
		// if it actually HAS changed.
		RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_NOCHILDREN|RDW_ERASE);
		/*if (IsControlValid(&m_wndLabel)) {
			CRect rcLabel;
			GetControlChildRect(this, &m_wndLabel, &rcLabel);
			if (!rcLabelPrior.EqualRect(rcLabel)) {
				m_wndLabel.RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ERASE);
			}
		}*/

		//if not loading, then we manually resized this item
		if(!m_bIsLoading) {
			//set our parent topic as unsaved, then tell the interface to reflect that
			m_pDetail->SetUnsaved();
			m_pDetail->m_pParentTopic->SetUnsaved();
			GetParent()->SendMessage(NXM_EMR_ITEM_CHANGED, (WPARAM)m_pDetail);		
		}
	} NxCatchAll("Error in OnSize");
}

void CEmrItemAdvDlg::OnSizing(UINT nSide, LPRECT lpRect)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (b.cardillo 2006-06-20 12:47) - PLID 13897 - Snap to grid if the shift key is not held 
		// down.  NOTE: If we ever make the snapping to the grid optional or make it so the user 
		// can specify how big the blocks of the grid are, we will need to move this code and put 
		// it in the CEmrTopicWnd class instead, because there it can be made aware of user 
		// preferences and such.  Here, we are meant to be more universal, and so it is the 
		// appropriate place for this code right now.
		if (GetRemotePropertyInt("EMRAutoAlignToGrid", 1, 0, GetCurrentUserName(), TRUE) == 1 && !IsKeyDown(VK_SHIFT)) {
			CWnd *pParent = GetParent();
			if (pParent->GetSafeHwnd()) {
				// Get our desired rect relative to our parent (we'll adjust off the borders below 
				// thus making it our desired client area relative to our parent)
				CRect rcClientWithinParent(lpRect);
				pParent->ScreenToClient(&rcClientWithinParent);
				// Get the widths of each of our four borders
				CRect rcBorderWidths;
				{
					// Get the client area and full window area in screen coordinates
					CRect rcWindow, rcClient;
					{
						// The full window
						GetWindowRect(&rcWindow);
						// The client
						GetClientRect(&rcClient);
						ClientToScreen(&rcClient);
					}
					// Calculate the border widths
					rcBorderWidths.left = rcClient.left - rcWindow.left;
					rcBorderWidths.top = rcClient.top - rcWindow.top;
					rcBorderWidths.right = rcWindow.right - rcClient.right;
					rcBorderWidths.bottom = rcWindow.bottom - rcClient.bottom;
					// The client better be within the window
					ASSERT(rcBorderWidths.left >= 0 && rcBorderWidths.top >= 0 && rcBorderWidths.right >= 0 && rcBorderWidths.bottom >= 0);
				}
				// Now adjust off the borders, thus making it a client area
				rcClientWithinParent.DeflateRect(rcBorderWidths);

				// Figure out how much each side needs to be changed
				CRect rcChangeSides(0,0,0,0);
				switch (nSide) {
				case WMSZ_LEFT:
					if (rcClientWithinParent.left < 0) {
						rcChangeSides.left = -rcClientWithinParent.left;
					} else {
						rcChangeSides.left = MulDiv(rcClientWithinParent.left, 1, SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK - rcClientWithinParent.left;
					}
					break;
				case WMSZ_TOP:
					if (rcClientWithinParent.top < 0) {
						rcChangeSides.top = -rcClientWithinParent.top;
					} else {
						rcChangeSides.top = MulDiv(rcClientWithinParent.top, 1, SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK - rcClientWithinParent.top;
					}
					break;
				case WMSZ_RIGHT:
					rcChangeSides.right = MulDiv(rcClientWithinParent.right, 1, SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK - rcClientWithinParent.right;
					break;
				case WMSZ_BOTTOM:
					rcChangeSides.bottom = MulDiv(rcClientWithinParent.bottom, 1, SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK - rcClientWithinParent.bottom;
					break;
				case WMSZ_TOPLEFT:
					if (rcClientWithinParent.top < 0) {
						rcChangeSides.top = -rcClientWithinParent.top;
					} else {
						rcChangeSides.top = MulDiv(rcClientWithinParent.top, 1, SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK - rcClientWithinParent.top;
					}					
					if (rcClientWithinParent.left < 0) {
						rcChangeSides.left = -rcClientWithinParent.left;
					} else {
						rcChangeSides.left = MulDiv(rcClientWithinParent.left, 1, SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK - rcClientWithinParent.left;
					}
					break;
				case WMSZ_TOPRIGHT:					
					if (rcClientWithinParent.top < 0) {
						rcChangeSides.top = -rcClientWithinParent.top;
					} else {
						rcChangeSides.top = MulDiv(rcClientWithinParent.top, 1, SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK - rcClientWithinParent.top;
					}
					rcChangeSides.right = MulDiv(rcClientWithinParent.right, 1, SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK - rcClientWithinParent.right;
					break;
				case WMSZ_BOTTOMLEFT:
					rcChangeSides.bottom = MulDiv(rcClientWithinParent.bottom, 1, SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK - rcClientWithinParent.bottom;
					
					if (rcClientWithinParent.left < 0) {
						rcChangeSides.left = -rcClientWithinParent.left;
					} else {
						rcChangeSides.left = MulDiv(rcClientWithinParent.left, 1, SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK - rcClientWithinParent.left;
					}
					break;
				case WMSZ_BOTTOMRIGHT:
					rcChangeSides.bottom = MulDiv(rcClientWithinParent.bottom, 1, SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK - rcClientWithinParent.bottom;
					rcChangeSides.right = MulDiv(rcClientWithinParent.right, 1, SIZE_EMR_SNAP_TO_GRID_BLOCK) * SIZE_EMR_SNAP_TO_GRID_BLOCK - rcClientWithinParent.right;
					break;
				default:
					ASSERT(FALSE);
					break;
				}

				// Now make the adjustments
				lpRect->left += rcChangeSides.left;
				lpRect->top += rcChangeSides.top;
				lpRect->right += rcChangeSides.right;
				lpRect->bottom += rcChangeSides.bottom;
			}
		}


		// (a.walling 2012-03-29 08:04) - PLID 49297 - Notify the parent if it is a CEmrTopicWnd
		if (dynamic_ptr<CEmrTopicWnd> pTopicWnd = GetParent()) {
			// (a.walling 2012-04-02 08:29) - PLID 49304 - Unified size/move handler
			pTopicWnd->HandleItemSizeMove(this, nSide, lpRect);
		}

	} NxCatchAll("Error in OnSizing");

	// Call the base class implementation
	CWnd::OnSizing(nSide, lpRect);
}

void CEmrItemAdvDlg::OnDestroy() 
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		if (this != (CEmrItemAdvDlg*)GetGlobalEmrItemAdvNarrativeDlg(NULL, egiWindowless) && this != (CEmrItemAdvDlg*)GetGlobalEmrItemAdvNarrativeDlg(NULL, egiLoading)) {
			// (a.walling 2007-08-09 13:37) - PLID 26781 - Save changes to the detail
			if (m_pDetail) {
				CRect rc;
				GetClientRect(rc);
				ClientToScreen(rc);
				//TES 9/16/2009 - PLID 35529 - Don't send this message if this is the global, windowless dialog.
				// In that case, the parent is undefined, probably MainFrm, and in any case is not expecting this message.
				// (a.walling 2009-12-08 15:22) - PLID 36225 - We now have two global CEmrItemAdvNarrativeDlgs
				if (GetParent() && IsWindow(GetParent()->GetSafeHwnd())) {
					GetParent()->SendMessage(NXM_CONVERT_RECT_FOR_DATA, (WPARAM)&rc);

					m_pDetail->m_rcDefaultClientArea = rc;
				}
	 
				if (this == m_pDetail->GetEmrItemAdvDlg()) {
					m_pDetail->ResetEmrItemAdvDlg();
				}
			}
		} else {
			m_pDetail = NULL;
		}

		CWnd::OnDestroy();

		DestroyContent();

		// (c.haag 2006-03-10 13:15) - PLID 18984 - Don't leave tooltip windows hanging around
		//
		if (m_pToolTipWnd && IsWindow(m_pToolTipWnd->GetSafeHwnd())) {
			//DRT 10/9/2007 - PLID 27710 - This should have been deleting the brush as well!			
			// (a.walling 2010-06-23 17:22) - PLID 39330 - No longer necessary
			//DeleteObject(m_pToolTipWnd->m_hBrush);
			m_pToolTipWnd->DestroyWindow();
			delete m_pToolTipWnd;
			m_pToolTipWnd = NULL;
		}
	} NxCatchAll("Error in OnDestroy");
}

BOOL CEmrItemAdvDlg::RepositionControls(IN OUT CSize &szArea, BOOL bCalcOnly)
{
	// (c.haag 2006-03-22 09:16) - PLID 19786 - Reset the need to reposition
	// controls. Inherited objects will need to set this value if their repositioning
	// fails.
	//
	SetNeedToRepositionControls(FALSE);

	// Usually derived class overrides this.
	return TRUE;
}

void CEmrItemAdvDlg::ReflectCurrentContent()
{
	// Usually derived class overrides this.
	
	// Make sure our merge status icon button exists. It will be hidden in
	// most cases, but the object itself must persist.
	EnsureMergeStatusButton();

	// Make sure our problem status icon button exists. It will be hidden in
	// most cases, but the object itself must persist.
	EnsureProblemStatusButton();

	// (c.haag 2006-03-22 09:16) - PLID 19786 - At this point, we have all of our controls, but they are
	// not positioned in the proper places. We cannot assume the caller has already determined and calculated
	// the size of this detail. So, we will flag the fact that we need to have our controls repositioned.
	//
	SetNeedToRepositionControls(TRUE);
}

void CEmrItemAdvDlg::DestroyContent()
{
	// Usually derived class overrides this.
	EnsureNotMergeStatusButton();
	EnsureNotProblemStatusButton();
}

void CEmrItemAdvDlg::ReflectCurrentState()
{
	if(IsControlValid(&m_wndLabel)) {
		CString strLabel = GetLabelText(TRUE);
		strLabel.Replace("&", "&&");
		CString strCurrentLabel;
		GetControlText(&m_wndLabel, strCurrentLabel);
		if(strLabel != strCurrentLabel) {
			m_wndLabel.SetWindowText(strLabel);
			//m_wndLabel.Invalidate();
			CRect rcWindow;
			GetWindowRect(&rcWindow);
			RepositionControls(CSize(rcWindow.Width(), rcWindow.Height()), FALSE);
		}
	}

	if(m_pToolTipWnd && m_pToolTipWnd->IsWindowVisible() && m_pToolTipWnd->m_strTip != GetToolTipText()) {
		m_pToolTipWnd->m_strTip = GetToolTipText();
		//TES 10/17/2005 - We don't want to trim whitespace characters, we just want to make sure
		//that we're not displaying an entirely blank tooltip.
		CString strTrimmed = m_pToolTipWnd->m_strTip;
		while (strTrimmed.GetLength())
		{
			char c = strTrimmed[ strTrimmed.GetLength() - 1 ];
			if (c == '\r' || c == '\n' || c == '\t')
			{
				strTrimmed = strTrimmed.Left( strTrimmed.GetLength() - 1 );
			}
			else {
				break;
			}
		}

		if (strTrimmed.IsEmpty())
		{
			m_pToolTipWnd->m_strTip = strTrimmed;
			// If it's visible, make it invisible.
			if(m_pToolTipWnd->IsWindowVisible()) {								
				m_pToolTipWnd->ShowWindow(SW_HIDE);
			}				}
		else {
			m_pToolTipWnd->Refresh();
		}
	}
}

void CEmrItemAdvDlg::UpdateStatusButtonAppearances()
{
	UpdateMergeStatusButtonAppearance();
	UpdateProblemStatusButtonAppearance();
}

void CEmrItemAdvDlg::UpdateMergeStatusButtonAppearance()
{
	//TES 1/30/2007 - PLID 24492 - This is no longer necessary, any code that sets MergeNameConflicts to TRUE then calls
	// TryToOverrideMergeField(), therefore, if it was possible to override this merge field, it would already be overridden.
	/*if(m_bMergeNameConflicts && m_pDetail->GetMergeFieldOverride().IsEmpty()) {
		m_pDetail->m_pParentTopic->GetParentEMN()->TryToOverrideMergeField(m_pDetail);
	}*/

	// (j.jones 2006-04-05 17:01) - used to determine if we need to repaint the button
	BOOL bIsAlreadyShown = m_bShowMergeStatusButton;

	// (z.manning, 01/24/2008) - PLID 28690 - Make sure the merge button exists.
	EnsureMergeStatusButton();
	//TES 12/16/2005 - We can't be going through every detail on the EMN from way down here!
	//Just show the button if either we have an override, or our merge name conflicts.
	if(m_bMergeNameConflicts) {
		// (j.jones 2006-04-05 17:02) - to properly redraw the icon we need to hide if already showing,
		// show again, and invalidate later
		if(bIsAlreadyShown)
			m_pBtnMergeStatus->ShowWindow(SW_HIDE);
		m_pBtnMergeStatus->SetIcon(IDI_MERGECONFLICT);
		m_pBtnMergeStatus->ShowWindow(SW_SHOW);
		m_bShowMergeStatusButton = TRUE;
	}
	else if(!m_pDetail->GetMergeFieldOverride().IsEmpty()) {
		// (j.jones 2006-04-05 17:02) - to properly redraw the icon we need to hide if already showing,
		// show again, and invalidate later
		if(bIsAlreadyShown)
			m_pBtnMergeStatus->ShowWindow(SW_HIDE);
		m_pBtnMergeStatus->SetIcon(IDI_MERGEOVERRIDE);
		m_pBtnMergeStatus->ShowWindow(SW_SHOW);
		m_bShowMergeStatusButton = TRUE;
	}
	else {
		m_pBtnMergeStatus->ShowWindow(SW_HIDE);
		m_bShowMergeStatusButton = FALSE;
	}


	// (j.jones 2006-08-21 16:17) - PLID 21986 - if the merge status button is shown,
	// but we're on a locked EMN, continue to show the button but make it read only
	if(m_bShowMergeStatusButton) {
		CEMN *pEMN = m_pDetail->m_pParentTopic->GetParentEMN();

		// (j.jones 2009-12-17 10:10) - PLID 31903 - double-checked read only status, not just locked
		//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
		m_pBtnMergeStatus->EnableWindow(pEMN->GetStatus() != 2 && !m_pDetail->GetReadOnly());
	}

	if(m_bShowMergeStatusButton && bIsAlreadyShown) {
		// (j.jones 2006-04-05 17:02) -  if we changed the icon, but the button was already displayed,
		//we need to ensure it gets redrawn
		Invalidate();
	}
}

// (c.haag 2008-07-29 16:03) - PLID 30878 - Optional parameter for repositioning controls. Defaults
// to FALSE so we don't break legacy code
void CEmrItemAdvDlg::UpdateProblemStatusButtonAppearance(BOOL bRepositionControls /* = FALSE */)
{
	// (z.manning, 01/24/2008) - PLID 28690 - Make sure the status button exists.
	EnsureProblemStatusButton();
	// (c.haag 2006-06-30 16:37) - PLID 19944 - Make sure the problem status button is visible when
	// necessary. A value of -1 means no problem, -2 means an unsaved problem, and a positive value
	// means a value saved in data.
	// (j.jones 2008-07-21 08:23) - PLID 30779 - call HasProblems() instead
	if (!m_pDetail->HasProblems()) {
		m_pBtnProblemStatus->ShowWindow(SW_HIDE);
	} else {
		if (m_pBtnProblemStatus->IsWindowVisible()) {
			// (c.haag 2006-07-05 09:24) - Following Josh's comment, we need to hide the button before
			// setting the icon so that when we show it again, it redraws properly
			m_pBtnProblemStatus->ShowWindow(SW_HIDE);
		}
		// (j.jones 2008-07-21 09:36) - PLID 30779 - call HasOnlyClosedProblems
		if (m_pDetail->HasOnlyClosedProblems()) {
			// (a.walling 2008-05-02 09:02) - PLID 29842 - This needs to stay capped at 16, the classic size
			m_pBtnProblemStatus->SetIcon(IDI_EMR_PROBLEM_CLOSED, 16, TRUE);
		} else { // show the open problem flag if any problems are open
			// (a.walling 2008-05-02 09:02) - PLID 29842 - This needs to stay capped at 16, the classic size
			m_pBtnProblemStatus->SetIcon(IDI_EMR_PROBLEM_FLAG, 16, TRUE);
		}
		m_pBtnProblemStatus->ShowWindow(SW_SHOW);
	}

	// If bRepositionControls is true, then we need to reposition all the form elements regardless of
	// problem button visibility.
	if (bRepositionControls) {
		CRect rWindow;
		GetWindowRect(rWindow);
		RepositionControls(CSize(rWindow.Width(),rWindow.Height()),FALSE);
	}

	// Make sure any changes get drawn
	Invalidate();
}

BOOL CEmrItemAdvDlg::IsMergeButtonVisible() const
{
	return (m_pBtnMergeStatus && IsWindow(m_pBtnMergeStatus->GetSafeHwnd()) && m_bShowMergeStatusButton);
}

BOOL CEmrItemAdvDlg::IsProblemButtonVisible() const
{
	// (j.jones 2008-07-21 08:30) - PLID 30779 - use HasProblems()
	return (m_pBtnProblemStatus && IsWindow(m_pBtnProblemStatus->GetSafeHwnd()) && m_pDetail->HasProblems());
}

// (a.walling 2007-11-05 16:08) - PLID 27980 - VS2008 - Use LRESULT instead of UINT
LRESULT CEmrItemAdvDlg::OnNcHitTest(CPoint point)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {				
		// (a.walling 2012-04-02 08:29) - PLID 49304 - Return HTTRANSPARENT if moving
		if (this == CEmrItemAdvDlg::g_pMovingDlg) {
			return HTTRANSPARENT;
		}

		if (m_pDetail->GetAllowEdit()) {
			// See if the point is over the label area
			CRect rcLabel;
			GetControlWindowRect(this, &m_wndLabel, &rcLabel);
			if (rcLabel.PtInRect(point)) {
				// It is, so override mfc and behave as if the label area is the caption (i.e. user can drag from there)
				return HTCAPTION;
			}
		}
	} NxCatchAll("Error in OnNcHitTest");

	// Just let mfc handle it
	return CWnd::OnNcHitTest(point);
}

BOOL CEmrItemAdvDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		//First off, if we're showing a tooltip, and the mouse is over the tip, then ignore.
		CPoint ptCursor;
		GetCursorPos(&ptCursor);
		if(m_pToolTipWnd) {
			CRect rTip;
			m_pToolTipWnd->GetWindowRect(rTip);
			if(rTip.PtInRect(ptCursor)) {
				return CWnd::OnSetCursor(pWnd, nHitTest, message);
			}
		}

		if (nHitTest == HTCAPTION && ::GetCapture() == NULL) {
			// (a.walling 2012-03-09 11:22) - PLID 48765 - Made this static
			static HICON hCursorHand = AfxGetApp()->LoadCursor(IDC_HAND_CUR);
			SetCursor(hCursorHand);
			return TRUE;
		}
	} NxCatchAll("Error in OnSetCursor");
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CEmrItemAdvDlg::OnMoving(UINT nSide, LPRECT lpRect)
{

	// (a.walling 2012-04-02 08:29) - PLID 49304 - Use the unified size/move handler

	if(m_pToolTipWnd) {
		//First, hide the tooltip because we're moving.
		m_pToolTipWnd->ShowWindow(SW_HIDE);

		// (a.wetta 2006-11-09 11:18) - There is no reason to re-align the tool tip yet. It's hidden and will be re-aligned 
		// in the timer code.  No reason to constantly keep re-aligning it especially because it causes problems.
	}
	
	// (a.walling 2012-03-29 08:04) - PLID 49297 - Notify the parent if it is a CEmrTopicWnd
	if (dynamic_ptr<CEmrTopicWnd> pTopicWnd = GetParent()) {
		pTopicWnd->HandleItemSizeMove(this, 0, lpRect);
	}

	__super::OnMoving(nSide, lpRect);
}

// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Just gives a friendly name for each review status.
CString GetReconstructedDetailReviewStatusName(const long nReviewStatus)
{
	switch (nReviewStatus) {
	case -1:
		return "Reconstructed by NexTech";
		break;
	case -2:
		return "Verified by User Leave Highlighted";
		break;
	case -3:
		return "Verified by User";
		break;
	case 0:
		return "{Normal Data, No Review Status}";
		break;
	default:
		ASSERT(FALSE);
		CString str;
		str.Format("{Unknown Review Status %li}", nReviewStatus);
		return str;
		break;
	}
}

// (b.cardillo 2006-11-20 10:37) - PLID 22565 - This function is the one place that officially updates 
// the ReconstructedEMRDetailsT data.
void ChangeReconstructedReviewState(const long nEMRDetailID, const long nCurrentlyKnownReviewStatus, const long nChangeToNewReviewStatus)
{
	if (nEMRDetailID != -1) {
		// Good, we're applying this to a real detail
		if (nCurrentlyKnownReviewStatus != nChangeToNewReviewStatus) {
			// Good we're actually changing the status
			if (nChangeToNewReviewStatus == -1 || nChangeToNewReviewStatus == -2 || nChangeToNewReviewStatus == -3) {
				// Good we're changing to an allowed status
				_RecordsetPtr prs = CreateRecordset(
					"SET NOCOUNT ON \r\n"
					"BEGIN TRAN \r\n"
					"DECLARE @nDetailID INT \r\n"
					"SET @nDetailID = %li \r\n"
					"DECLARE @nExpRevStat INT \r\n"
					"SET @nExpRevStat = %li \r\n"
					"DECLARE @nCurRevStat INT \r\n"
					"SET @nCurRevStat = (SELECT ReviewStatus FROM ReconstructedEMRDetailsT WITH(UPDLOCK) WHERE EMRDetailID = @nDetailID) \r\n"
					"IF @nCurRevStat = @nExpRevStat BEGIN \r\n"
					"  DECLARE @nRowCount INT, @nError INT \r\n"
					"  UPDATE ReconstructedEMRDetailsT SET ReviewStatus = %li WHERE EMRDetailID = @nDetailID AND ReviewStatus = @nExpRevStat\r\n"
					"  SELECT @nError = @@ERROR, @nRowCount = @@ROWCOUNT \r\n"
					"  IF @nError <> 0 BEGIN \r\n"
					"    ROLLBACK TRAN \r\n"
					"    SET NOCOUNT OFF \r\n"
					"    RETURN \r\n"
					"  END ELSE IF @nRowCount <> 1 BEGIN \r\n"
					"    ROLLBACK TRAN \r\n"
					"    SET NOCOUNT OFF \r\n"
					"    RAISERROR('Attempted to write to exactly one ReconstructedEMRDetailsT record but %%li would have been written!  This transaction has been rolled back.', 16, 1, @nRowCount) \r\n"
					"    RETURN \r\n"
					"  END \r\n"
					"END ELSE BEGIN \r\n"
					"  DECLARE @strValText NVARCHAR(10) \r\n"
					"  SET @strValText = COALESCE(CONVERT(NVARCHAR, @nCurRevStat), 'NULL') \r\n"
					"  ROLLBACK TRAN \r\n"
					"  SET NOCOUNT OFF \r\n"
					"  RAISERROR('Current ReviewStatus %%s differs from the expected value %%li for this detail!  The review status for this detail may have been changed by another user.  This transaction has been rolled back.', 16, 1, @strValText, @nExpRevStat) \r\n"
					"END \r\n"
					"COMMIT TRAN \r\n"
					"SET NOCOUNT OFF \r\n"
					"SELECT [Last] + ', ' + [First] + ' ' + [Middle] AS FullName, ID AS PatientID FROM PersonT WHERE ID = (SELECT A.PatientID FROM EMRMasterT A WHERE A.ID = (SELECT B.EMRID FROM EMRDetailsT B WHERE B.ID = @nDetailID)) \r\n"
					, nEMRDetailID, nCurrentlyKnownReviewStatus, nChangeToNewReviewStatus);
				// Audit the change appropriately
				CString strPatientName;
				long nPatientID = -1;
				if (!prs->eof) {
					strPatientName = VarString(prs->GetFields()->GetItem("FullName")->GetValue(), "{Null Patient Name}");
					nPatientID = VarLong(prs->GetFields()->GetItem("PatientID")->GetValue(), -1);
				} else {
					strPatientName = "{Unknown Patient Name}";
				}
				AuditEvent(nPatientID, strPatientName, BeginNewAuditEvent(), aeiReconstructedEMRDetailReviewState, nEMRDetailID, 
					GetReconstructedDetailReviewStatusName(nCurrentlyKnownReviewStatus), 
					GetReconstructedDetailReviewStatusName(nChangeToNewReviewStatus), 2, aetChanged);
			} else {
				ThrowNxException("ChangeReconstructedReviewState: Cannot change ReviewStatus to invalid status %li!", nChangeToNewReviewStatus);
			}
		} else {
			ThrowNxException("ChangeReconstructedReviewState: Cannot set ReviewStatus to its own current value!");
		}
	} else {
		ThrowNxException("ChangeReconstructedReviewState: Cannot change ReviewStatus for invalid detail!");
	}
}

void CEmrItemAdvDlg::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	try {
		//{
		//	CNxMenu testMenu;
		//	CMenu testPopup;

		//	testMenu.CreatePopupMenu();

		//	testPopup.CreatePopupMenu();

		//	testPopup.AppendMenu(MF_CHECKED, 1600, "Checkmate");

		//	testMenu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT_PTR)testPopup.GetSafeHmenu(), "Popup");

		//	CPoint pt;
		//	GetMessagePos(&pt);
		//	long nResult = testMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		//	return;
		//}

		// Just use an enum instead of using const variables or number literals
		enum {
			miRemove = 1,
			miEdit,
			miMergeName,
			//TES 12/8/2006 - PLID 23790 - Give option to update an out-of-date item.
			miBringUpToDate,
			// (j.jones 2008-07-21 08:30) - PLID 30779 - add ability to add a new problem always,
			// and add the ability to edit existing problems
			miNewProblem,
			miEditProblem,
			miExistingProblem, // (c.haag 2009-05-27 14:39) - PLID 34249
			miResetColumns,
			miClearInk,
			// (d.thompson 2009-03-04) - PLID 32891 - Remove background image
			miRemoveBackgroundImage,
			miRestoreSize,
			// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Added these two enum values so they could be used in the menu below.
			miReconstructionVerified,
			miReconstructionVerifiedKeepHighlight,
			miPopup, // (a.walling 2007-06-21 15:55) - PLID 22097 - Popup this item
			// (a.walling 2008-07-01 09:37) - PLID 30570 - Preview flag to hide title or item
			miHideTitle,
			miHideItem,
			// (a.walling 2009-01-08 13:53) - PLID 32659 - Set float/clear detail options
			// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
			miColumnOne,
			miColumnTwo,
			/*
			miClearLeft,
			miClearRight,
			*/
			// (a.walling 2009-01-08 14:07) - PLID 32660 - Align text right
			miTextRight,
			// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
			miPageBreakBefore,
			miPageBreakAfter,
			miDisplayUnderSpawningDetail, // (a.walling 2008-10-23 10:07) - PLID 31808 - Display as subdetail
			// (c.haag 2009-02-17 17:52) - PLID 31327 - HotSpot click enabling
			miEnableHotSpotClicks,
			miDisableHotSpotClicks,
			// (c.haag 2007-04-18 16:49) - PLID 25075 - This is the first available item to use in storing
			// other custom values
			miFirstAvailableMenuItem,
			// (c.haag 2011-03-30) - PLID 43046 - Be able to shrink Current Medication and Allergy tables to fixed heights
			// now that we hide unselected items.
			miResetTableHeight,
			// (c.haag 2011-03-30) - PLID 43047 - Be able to edit common lists
			miEditCommonLists,
			//TES 2/24/2012 - PLID 45127 - Option to change existing stamps to a different type
			miChangeStamp,
			//TES 3/8/2012 - PLID 48733 - Option to change all stamps of a given type to a different type
			miChangeAllStamps,
			// (a.walling 2012-07-13 16:38) - PLID 48896
			miHideIfIndirectlyOnNarrative, 
			// (j.armen 2013-01-16 16:41) - PLID 54412
			miHideItemOnIPad,
		};

		// (c.haag 2006-04-03 15:38) - PLID 19890 - There's nothing a read-only user
		// can do with pop-ups
		// (a.walling 2007-06-21 15:50) - PLID 22097 - Not true anymore, we can Popup details if we desire
		// (useful for obscured details in locked EMNs, or to see an image in full screen)
		// (a.walling 2007-11-28 11:22) - PLID 28044 - Also check for expired
		if (!CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) || (g_pLicense->HasEMR(CLicense::cflrSilent) != 2)) {
			// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
			CNxMenu mnu;

			mnu.CreatePopupMenu();
			mnu.AppendMenu(MF_BYPOSITION|MF_ENABLED, miPopup, "Popup This &Item");
		
			CPoint pt = CalcContextMenuPos(pWnd, pos);
			long nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

			if (nResult == miPopup) {
				try {
					m_pDetail->Popup();
				} NxCatchAll("Error popping up detail!");
			}

			return;
		}

		if(m_pToolTipWnd) {
			m_pToolTipWnd->ShowWindow(SW_HIDE);
		}

		CEMN *pEMN = m_pDetail->m_pParentTopic->GetParentEMN();
		BOOL bIsLockedEMN = pEMN->GetStatus() == 2;

		// (c.haag 2007-04-18 16:54) - PLID 25705 - Used for choosing details from the popup menu
		CArray<CEMNDetail*,CEMNDetail*> apDetailMenuItems;

		//determine where the cursor lies
		if (pos.x != -1 && IsControlValid(&m_wndLabel)) {
			CRect rc;
			GetControlWindowRect(this, &m_wndLabel, &rc);

			//TES 3/6/2012 - PLID 45127 - Track if we right-click on a stamp
			long nRightClickedStampIndex = -1;
			long nRightClickedStampID = -1;

			// Create the menu
			// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
			CNxMenu mnu;
			mnu.CreatePopupMenu();
			CMenu mnuPosition;
			//create a sub-menu for the spawned items
			CMenu pSubMenu1;

			BOOL bShowMenu = FALSE;

			//TES 3/8/2012 - PLID 45127 - Moved the Change Stamp options to the top of the menu
			if(!m_pDetail->GetReadOnly() && !m_bIsTemplate && 
				m_pDetail->m_EMRInfoType == eitImage  && !bIsLockedEMN && !m_pDetail->GetReadOnly()
				//TES 10/9/2006 - PLID 22920 - Don't show these options if there is no valid image showing.
				&& ((CEmrItemAdvImageDlg*)m_pDetail->m_pEmrItemAdvDlg)->HasValidImage()) 
			{
				//TES 2/24/2012 - PLID 45127 - If they have right-clicked on a stamp, give them the option to change its type.
				nRightClickedStampIndex = ((CEmrItemAdvImageDlg*)m_pDetail->m_pEmrItemAdvDlg)->GetStampIndexFromPoint(pos.x, pos.y);
				if(nRightClickedStampIndex > -1) {
					mnu.AppendMenu(MF_SEPARATOR);
					CNxInkPictureText nipt;
					CEmrItemAdvImageState ais;
					ais.CreateFromSafeArrayVariant(m_pDetail->GetState());
					nipt.LoadFromVariant(ais.m_varTextData);
					// (r.gonet 05/02/2012) - PLID 49949 - We don't allow changes to image stamps because it is not a thing somebody would do and it creates difficulties.
					if(!nipt.GetTextString(nRightClickedStampIndex).ImageInfo.bHasImage) {
						//TES 2/24/2012 - PLID 45127 - Include the stamp text, in case there are two overlapping stamps, to make it clear
						// which one is about to get changed.
						CString strStamp = nipt.GetStringByIndex(nRightClickedStampIndex);
						//TES 2/24/2012 - PLID 45127 - Trim the dot (if any).
						strStamp.TrimLeft((char)149);
						strStamp.TrimLeft(" ");
						mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miChangeStamp, "Change Stamp From " + strStamp + " To ...");

						//TES 3/8/2012 - PLID 48733 - See if this stamp is Increase Quantity, and if there are multiple instances of it on this
						// image, and if so give the Change All Stamps option
						nRightClickedStampID = nipt.GetStampIDByIndex(nRightClickedStampIndex);
						EMRImageStamp *pGlobalStamp = GetMainFrame()->GetEMRImageStampByID(nRightClickedStampID);
						//TES 3/30/2012 - PLID 48733 - pGlobalStamp might be NULL, if it's a free-text stamp
						if(pGlobalStamp && pGlobalStamp->eSmartStampTableSpawnRule == esstsrIncreaseQuantity) {
							bool bOneFound = false;
							bool bMultipleFound = false;
							for(int nStamp = 0; nStamp < m_pDetail->GetImageStampCount() && !bMultipleFound; nStamp++) {
								if(m_pDetail->GetImageStampByIndex(nStamp)->nStampID == nRightClickedStampID) {
									if(!bOneFound) bOneFound = true;
									else bMultipleFound = true;
								}
							}
							if(bMultipleFound) {
								mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miChangeAllStamps, "Change All " + strStamp + " Stamps To ...");
							}
						}

						mnu.AppendMenu(MF_SEPARATOR);
					}
				}
			}
			// (z.manning 2008-11-03 10:27) - PLID 31890 - Signature details are always read only so enable
			// the remove option for them as long as the EMN is non-locked and writable.
			//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
			// (j.jones 2010-05-12 17:27) - PLID 38523 - like signature details, SmartStamp tables
			// are read only on templates and also need to be removeable (the writable & lock check is just for extra safety)
			// However nothing should be removeable if the Topic is read only due to being spawned on a template.
			if ((!m_pDetail->GetReadOnly()
				|| (m_pDetail->IsSignatureDetail() && pEMN->IsWritable() && !bIsLockedEMN)
				|| (m_pDetail->IsSmartStampTable() && m_pDetail->m_bIsTemplateDetail && pEMN->IsWritable() && !bIsLockedEMN)
				)
				&& rc.PtInRect(pos)
				&& !(m_pDetail->GetParentTopic() != NULL && m_pDetail->GetParentTopic()->GetSourceActionID() != -1
					&& m_pDetail->m_bIsTemplateDetail))
			{				
				// Add a rename and delete menu items, these are always available
				CString strMenuRemove;
				{
					// (b.cardillo 2012-05-02 20:28) - PLID 49255 - Use the detail label text as before, that is without any special modifier text around it
					CString strLabel = GetLabelText(FALSE);
					if (!strLabel.IsEmpty()) {
						strLabel = strLabel.Left(20);
						strLabel.Replace("&", "&&");
						strMenuRemove.Format("&Remove '%s'", strLabel);
					} else {
						strMenuRemove = "&Remove";
					}
				}
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miRemove, strMenuRemove);

				bShowMenu = TRUE;
			}

			if(bShowMenu)
				mnu.AppendMenu(MF_SEPARATOR);

			// (j.jones 2006-08-21 16:21) - PLID 21986 - changing the merge name is no longer
			// allowed on locked EMNs
			// (a.walling 2008-06-09 13:47) - PLID 22049 - If we are readonly, we should not be able to change this
			// (j.jones 2009-12-17 10:10) - PLID 31903 - double-checked read only status, not just locked
			//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
			if(!bIsLockedEMN && !m_pDetail->GetReadOnly()) {
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION|(m_pDetail->GetReadOnly() ? MF_DISABLED|MF_GRAYED : 0), miMergeName, "Change &Merge Name");
			}

			//TES 12/8/2006 - PLID 23790 - If this template is out of date, give them the option to bring it up to date.
			// (j.jones 2010-02-17 17:27) - PLID 37318 - if part of a SmartStamp image/table connection, disallow this
			// (j.jones 2010-08-25 17:29) - PLID 37981 - disallow if a generic table
			// (z.manning 2011-01-21 09:51) - PLID 42338 - Support multiple images per smart stamp table
			if(!m_pDetail->GetReadOnly() && !m_bIsTemplate && !m_pDetail->IsActiveInfo()
				&& m_pDetail->GetSmartStampImageDetails()->GetCount() == 0
				&& m_pDetail->GetSmartStampTableDetail() == NULL
				&& !m_pDetail->IsGenericTable()) {

				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miBringUpToDate, "&Bring item up to date");
			}
			bShowMenu = TRUE;

			// (c.haag 2006-06-29 15:06) - PLID 19977 - We can now assign a problem to a detail
			//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
			if(!m_pDetail->GetReadOnly() && !m_bIsTemplate) {
				mnu.AppendMenu(MF_SEPARATOR);
				// (j.jones 2008-07-21 08:30) - PLID 30779 - add ability to add a new problem always,
				// and add the ability to edit existing problems
				// (j.jones 2008-08-12 14:41) - PLID 30854 - disable the add option, but not the update option,
				// if the EMN is not writeable
				mnu.AppendMenu(MF_STRING|MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), miNewProblem, "Link with New &Problem");
				// (c.haag 2009-05-27 14:36) - PLID 34249 - Link with other problems
				{
					CEMRTopic *pTopic = m_pDetail->m_pParentTopic;
					CEMN* pEMN = (pTopic) ? pTopic->GetParentEMN() : NULL;
					CEMR* pEMR = (pEMN) ? pEMN->GetParentEMR() : NULL;
					if (NULL != pEMR) {
						mnu.AppendMenu(MF_STRING|MF_BYPOSITION|(pEMN->IsWritable() ? 0 : (MF_DISABLED|MF_GRAYED)), miExistingProblem, "Link with Existing Problems");
					}
				}
				if (m_pDetail->HasProblems()) {
					mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miEditProblem, "Update Problem &Information");
				}
			}

			//TES 8/17/2006 - PLID 22100 - Don't let them reset column widths on locked EMNs.
			// (a.walling 2008-08-25 10:29) - PLID 23138 - Also don't allow this on readonly EMNs
			//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
			if(m_pDetail->GetSaveTableColumnWidths() && m_pDetail->m_EMRInfoType == eitTable && !bIsLockedEMN && !m_pDetail->GetReadOnly()) {
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miResetColumns, "Reset &Column Widths");
			}

			// (j.jones 2006-09-18 10:24) - PLID 22555 - if an unlocked EMN, allow the user to clear all the ink
			// (a.walling 2008-08-21 17:11) - PLID 23138 - Don't allow Erase All Ink if we are readonly!
			//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
			if(m_pDetail->m_EMRInfoType == eitImage  && !bIsLockedEMN && !m_pDetail->GetReadOnly()
				//TES 10/9/2006 - PLID 22920 - Don't show these options if there is no valid image showing.
				&& ((CEmrItemAdvImageDlg*)m_pDetail->m_pEmrItemAdvDlg)->HasValidImage())
			{
				// (z.manning 2011-09-15 09:54) - PLID 45335 - Separate the options for 3D images
				if(m_pDetail->Is3DImage())
				{
					mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miClearInk, "Erase All Stamps");
				}
				else
				{
					mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miClearInk, "Erase All Ink");

					// (d.thompson 2009-03-04) - PLID 32891 - Let the user remove the image from the detail entirely
					//	if they don't want it anymore.
					// (d.thompson 2009-03-24) - PLID 33643 - Some time later, it was decided this should be controlled
					//	by a preference to enable it.
					if(GetRemotePropertyInt("EMNDetail_AllowRemoveBackgroundImage", 0, 0, GetCurrentUserName(), true) != 0) {
						mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miRemoveBackgroundImage, "Remove Back&ground Image");
					}

					// (c.haag 2009-02-17 17:42) - PLID 31327 - Let the user stamp on hotspots
					if (((CEmrItemAdvImageDlg*)m_pDetail->m_pEmrItemAdvDlg)->GetEnableHotSpotClicks()) {
						mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miDisableHotSpotClicks, "Enable &Writing on Hotspots");
					} else {
						mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miEnableHotSpotClicks, "Disable &Writing on Hotspots");
					}
					
					//TES 10/9/2006 - PLID 22614 - If the size has been changed, let them set it back to the correct size, based on
					// the image.
					long nIdealWidth = 0, nIdealHeight = 0;
					((CEmrItemAdvImageDlg*)m_pDetail->m_pEmrItemAdvDlg)->GetIdealDimensions(nIdealWidth, nIdealHeight);
					CRect rActualSize;
					m_pDetail->m_pEmrItemAdvDlg->GetWindowRect(&rActualSize);
					if(nIdealWidth != rActualSize.Width() || nIdealHeight != rActualSize.Height()) {
						mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miRestoreSize, "Restore &Original Size of Image");
					}
				}
			}

			// (c.haag 2006-02-24 14:56) - PLID 17195 - We cannot allow users to edit read-only
			// details, otherwise, it defeats the purpose of being "read only" and locked EMN's can
			// be modified
			//TES 3/15/2010 - PLID 37757 - Check our detail's read-only status
			//(e.lally 2010-05-03) PLID 15155 - Be doubly sure it's not locked, but don't worry about Edit Mode anymore.
			if(!bIsLockedEMN && !m_pDetail->GetReadOnly()) {
				mnu.AppendMenu(MF_SEPARATOR);
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miEdit, "&Edit");				
				// (c.haag 2011-03-30) - PLID 43047 - Be able to edit common lists
				if (m_pDetail->IsCurrentMedicationsTable() || m_pDetail->IsAllergiesTable()) {
					mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miEditCommonLists, "Edit Common &Lists...");
				}
			}

			// (j.jones 2005-11-08 11:59) - PLID 17698 - show a list of all spawned items
			// (j.jones 2006-08-17 10:05) - PLID 21979 - except when on a locked EMN			
			// (z.manning, 02/20/2007) - PLID 24525 - Also make sure we're not read only.
			// (a.walling 2007-06-21 16:10) - PLID 22097 - bPopupSpawnedDetails lets us know if this was added to the menu
			//TES 3/15/2010 - PLID 37757 - Check our detail's read-only status
			BOOL bPopupSpawnedDetails = FALSE;
			if( (m_pDetail->m_EMRInfoType == eitSingleList || m_pDetail->m_EMRInfoType == eitMultiList)
				&& !bIsLockedEMN && !m_pDetail->GetReadOnly() ) 
			{		

				//look for EMR items spawned by the data from this item, not from the existence of the item itself
				//_RecordsetPtr rs = CreateRecordset("SELECT ID FROM EMRActionsT "
				//	"WHERE DestType = %li AND SourceType = %li AND SourceID IN (SELECT ID FROM EMRDataT WHERE EMRInfoID = %li)", 
				//	eaoEmrItem, eaoEmrDataItem, m_pDetail->m_nEMRInfoID);

				// (c.haag 2007-04-18 16:16) - PLID 25705 - The above query failed for two reasons:
				//
				// 1. It did not consider EMRActionsT.Deleted
				// 2. It did not consider other versions of the EMR item
				//
				// (z.manning 2008-10-24 12:47) - PLID 31561 - Also include labs as a dest type since
				// spawning labs can also lead to spawned details.
				_RecordsetPtr rs = CreateParamRecordset("SELECT EMRActionsT.ID FROM EMRActionsT "
					"WHERE Deleted = 0 AND DestType IN ({INT}, {INT}) AND SourceType = {INT} AND SourceID IN ( "
					"	SELECT EMRDataT.ID FROM EMRDataT "
					"	INNER JOIN EMRInfoT ON EMRInfoT.ID = EMRDataT.EMRInfoID "
					"	WHERE EMRInfoMasterID IN ( "
					"		SELECT EMRInfoMasterID FROM EMRInfoT WHERE ID = {INT} "
					"	) "
					") ",
					eaoEmrItem, eaoLab, eaoEmrDataItem, m_pDetail->m_nEMRInfoID);

				BOOL bSeparatorAdded = FALSE;

				//create a sub-menu for the spawned items
				//CMenu pSubMenu1;
				pSubMenu1.CreatePopupMenu();

				int iAddedItems = 0;

				while(!rs->eof) {

					long nSourceActionID = AdoFldLong(rs, "ID");

					BOOL bFound = FALSE;

					long nTotalDetailCount = pEMN->GetTotalDetailCount();
					for(int i=nTotalDetailCount-1; i>=0 && !bFound; i--) {

						//only add links for the spawned EMR info items that we can pop up, which means no narratives
						//TES 4/6/2007 - PLID 25456 - Narratives can pop up now.
						CEMNDetail *pEMNDetail = pEMN->GetDetail(i);
						//DRT 10/6/2008 - PLID 31400 - Included a 'visible' check.  Noone is sure why we showed
						//	everything before -- the old behavior just looked for any spawned details, whether
						//	they were *actually* spawned or not.  The original item (PLID 17698) does
						//	not seem to indicate a particular need for those, and I have to assume it
						//	was an oversight.  Now this will only show up if you've actually spawned something.
						if (pEMNDetail->GetVisible() && pEMNDetail->GetSourceActionID() == nSourceActionID)
						{
							if(!bSeparatorAdded) {
								mnu.AppendMenu(MF_SEPARATOR);
								bSeparatorAdded = TRUE;
								bPopupSpawnedDetails = TRUE; // (a.walling 2007-06-21 16:10) - PLID 22097 - Let other places know if this was added to the menu
							}

							// (j.jones 2005-11-08 12:15) - I multiplied the nDestID by 100 so to not cause conflicts
							// with our hardcoded values for remove/edit/mergename menu items
							// (c.haag 2007-04-18 16:50) - PLID 25705 - Use miFirstAvailableMenuItem as our base ordinal
							// instead
							CString strName = pEMNDetail->GetMergeFieldOverride();
							if(strName.IsEmpty())
								strName = pEMNDetail->GetLabelText();
							pSubMenu1.InsertMenu(iAddedItems, MF_BYPOSITION, miFirstAvailableMenuItem + iAddedItems/*pEMNDetail->m_nEMRInfoID * 100*/, strName);
							apDetailMenuItems.Add(pEMNDetail);
							iAddedItems++;

							bFound = TRUE;
						}
					}

					rs->MoveNext();
				}
				rs->Close();

				if(bSeparatorAdded) {
					//means we did indeed create a menu
					mnu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT)pSubMenu1.m_hMenu, "Popup Spawned Details...");
				}
			}

			// (a.walling 2007-06-21 15:53) - PLID 22097 - Show an option to Popup the detail
			{
				// should not be adding this separator if we are on a locked EMN
				// (PopupSpawnedDetails will never be true on a locked EMN either)
				//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
				if (!bPopupSpawnedDetails && !bIsLockedEMN && !m_pDetail->GetReadOnly())
					mnu.AppendMenu(MF_SEPARATOR);
				mnu.AppendMenu(MF_BYPOSITION|MF_ENABLED, miPopup, "Popup This &Item");
				bShowMenu = TRUE;
			}
			
			// (b.cardillo 2006-11-20 10:37) - PLID 22565 - If this is reconstructed detail, give the user the 
			// option of verifying it.
			if (m_pDetail->m_nReviewState == -1 || m_pDetail->m_nReviewState == -2) {
				if (mnu.GetMenuItemCount() > 0) {
					mnu.AppendMenu(MF_SEPARATOR);
				}
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miReconstructionVerified, "Reconstruction &Verified");
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miReconstructionVerifiedKeepHighlight, "Reconstruction Verified &Keep Highlighted");
				if (m_pDetail->m_nReviewState == -2) {
					mnu.EnableMenuItem(miReconstructionVerifiedKeepHighlight, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
				}
				bShowMenu = TRUE;
			}

			// (a.walling 2008-07-01 09:39) - PLID 30570 - Show preview flag options
			//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
			if (!bIsLockedEMN && !m_pDetail->GetReadOnly()) {				
				if (mnu.GetMenuItemCount() > 0) {
					mnu.AppendMenu(MF_SEPARATOR);
				}
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION|((m_pDetail->GetPreviewFlags() & epfHideTitle) ? MF_CHECKED : MF_UNCHECKED), miHideTitle, "Hide &Title When Printing Preview");
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION|((m_pDetail->GetPreviewFlags() & epfHideItem) ? MF_CHECKED : MF_UNCHECKED), miHideItem, "Hide Entire &Item When Printing Preview");
				
				// (a.walling 2008-10-23 10:08) - PLID 31808 - Menu option to toggle subdetail display
				// (a.walling 2008-10-24 11:04) - PLID 31808 - Disable if we are not spawned, eh?
				DWORD dwMenuFlags = (m_pDetail->GetPreviewFlags() & epfSubDetail) ? MF_CHECKED : MF_UNCHECKED;
				CEMNDetail* pParentDetail = m_pDetail->GetSubDetailParent();
				if (pParentDetail == NULL) {
					dwMenuFlags |= (MF_DISABLED|MF_GRAYED);
				}
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION|dwMenuFlags, miDisplayUnderSpawningDetail, "Display &Under Spawning Item");

				// (a.walling 2012-07-13 16:38) - PLID 48896
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION|((m_pDetail->GetPreviewFlags() & epfHideIfIndirect) ? MF_CHECKED : MF_UNCHECKED), miHideIfIndirectlyOnNarrative, "Allow Hide if Indirectl&y Included on a Narrative");

				// (j.armen 2013-01-16 16:41) - PLID 54412
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION|((m_pDetail->GetPreviewFlags() & epfHideOnIPad) ? MF_CHECKED : MF_UNCHECKED), miHideItemOnIPad, "Hide Item on iPa&d");

				{
					// (a.walling 2009-01-08 13:40) - PLID 32659 - Prepare menu for floating elements
					mnuPosition.CreatePopupMenu();
					long nSub = 0;
					DWORD nPreviewFlags = m_pDetail->GetPreviewFlags();

					// clear both is default
					// (a.walling 2009-07-06 08:41) - PLID 34793 - Clearing is deprecated
					/*
					BOOL bClearLeft = TRUE;
					BOOL bClearRight = TRUE;

					if ((nPreviewFlags & epfClearNone) == epfClearNone) {
						bClearLeft = FALSE;
						bClearRight = FALSE;
					} else {
						if (nPreviewFlags & epfClearLeft) {
							bClearLeft = TRUE;
							bClearRight = FALSE;
						} else if (nPreviewFlags & epfClearRight) {
							bClearLeft = FALSE;
							bClearRight = TRUE;
						}
					}
					*/
					
					// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|((nPreviewFlags & epfColumnOne) ? MF_CHECKED : MF_UNCHECKED), miColumnOne, "Column One");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|((nPreviewFlags & epfColumnTwo) ? MF_CHECKED : MF_UNCHECKED), miColumnTwo, "Column Two");
					
					// (a.walling 2009-07-06 08:41) - PLID 34793 - Clearing is deprecated
					/*
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|(bClearLeft ? MF_CHECKED : MF_UNCHECKED), miClearLeft, "Clear Left");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|(bClearRight ? MF_CHECKED : MF_UNCHECKED), miClearRight, "Clear Right");
					*/

					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					// (a.walling 2009-01-08 14:07) - PLID 32660 - Align text right
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|((nPreviewFlags & epfTextRight) ? MF_CHECKED : MF_UNCHECKED), miTextRight, "Align Text Right");

					// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|((nPreviewFlags & epfPageBreakBefore) ? MF_CHECKED : MF_UNCHECKED), miPageBreakBefore, "New Page Before");
					mnuPosition.InsertMenu(nSub++, MF_BYPOSITION|((nPreviewFlags & epfPageBreakAfter) ? MF_CHECKED : MF_UNCHECKED), miPageBreakAfter, "New Page After");

					mnu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT_PTR)mnuPosition.GetSafeHmenu(), "Positioning");

					// (c.haag 2011-03-30) - PLID 43046 - Be able to shrink Current Medication and Allergy tables to fixed heights
					// now that we hide unselected items.
					if (m_pDetail->IsCurrentMedicationsTable() || m_pDetail->IsAllergiesTable()) 
					{
						mnu.AppendMenu(MF_SEPARATOR);
						mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miResetTableHeight, "Reset Table &Height");
					}
				}
				bShowMenu = TRUE;
			}

			if(bShowMenu) {
				CPoint pt = CalcContextMenuPos(pWnd, pos);
				long nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
				switch (nResult) {
				case miRemove:
					// (j.jones 2008-07-21 08:30) - PLID 30779 - use HasProblems()
					// (c.haag 2008-07-22 15:23) - PLID 30779 - We can't use HasProblems and HasOnlyClosedProblems because
					// unsaved problems don't count against removing details. We need to be more specific.
					/*if (m_pDetail->HasProblems()) {
						// (j.jones 2008-07-21 09:36) - PLID 30779 - call HasOnlyClosedProblems
						if (m_pDetail->HasOnlyClosedProblems()) {
							MsgBox(MB_ICONINFORMATION, "You may not remove this detail because it has closed problems.");
						} else {
							MsgBox(MB_ICONINFORMATION, "You may not remove this detail because it has active problems.");
						}
					} else {
						m_pDetail->RequestRemove();
					}*/
					// (c.haag 2008-07-24 13:02) - PLID 30826 - Do permissions checking
					if (!CanCurrentUserDeleteEmrProblems())
					{
						long nSavedOpenProblems = 0;
						long nSavedClosedProblems = 0;
						// (c.haag 2009-05-16 14:21) - PLID 34312 - Go through EMR problem links rather than just problems
						CArray<CEmrProblemLink*,CEmrProblemLink*>& apProblems = m_pDetail->m_apEmrProblemLinks; // There is no ::GetAllProblems for CEMNDetail; what's there is there
						const int nProblems = apProblems.GetSize();
						for (int i=0; i < nProblems; i++) {
							CEmrProblem* p = apProblems[i]->GetProblem();
							if (p->m_nID > 0) {
								if (p->m_nStatusID == 2) {
									// This is a closed problem
									nSavedClosedProblems++;
								} else {
									// Anything that is not closed is open
									nSavedOpenProblems++;
								}
							}
						}
						if (nSavedOpenProblems > 0) {
							// The detail has problems that are saved in data and are open (and possibly a few that are closed, but that is irrelevant here)
							MsgBox(MB_ICONINFORMATION, "You may not remove this detail because it corresponds to active problems in your data.");
							break;
						}
						else if (nSavedClosedProblems > 0) {
							// The detail has problems that are saved in data, but they are all closed
							MsgBox(MB_ICONINFORMATION, "You may not remove this detail because it corresponds to closed problems in your data.");
							break;
						}
					}
					m_pDetail->RequestRemove();
					// NOTE: Do nothing else after calling RequestRemove because it's possible we don't 
					// even exist anymore after its return.
					break;
				case miEdit: {
					// (a.walling 2008-03-25 08:11) - PLID 28811 - Pass our detail pointer to the now static function
					OpenItemEntryDlg(m_pDetail, m_bIsTemplate);
					break;
				}
				case miMergeName: {
					OnButtonClickedMergeConflict();
					break;
				}
				case miBringUpToDate: {

					if (m_pDetail->IsCurrentMedicationsTable()) {
						// (c.haag 2007-02-01 12:09) - PLID 24376 - Invoke a different one-time only warning for the Current Medications
						// detail. If we bring it "up to date", it will load the default Current Medications state, and then synchronize
						// the item with the More Info medications section.
						// (j.jones 2009-09-21 09:20) - PLID 35599 - we now do not pull from the patient's account when updating existing current
						// medication or allergy items, but since this message doesn't actually say that, we will leave it as-is
						// (j.jones 2010-11-12 08:56) - PLID 29075 - added a note that the topic will be immediately saved
						if(IDNO == DontShowMeAgain(this, "The Current Medications item has been edited since it was added to this EMN.  "
							"Bringing it up to date will apply those edits to this detail, and may result in changes to the selections made on the detail. "
							"The changes will take effect immediately and cannot be undone. "
							"In addition, the topic will be saved immediately after bringing up to date.\r\n\r\n"
							"Are you sure you wish to continue?", "BringCurrentMedicationsItemUpToDate", "Bring item up to date", FALSE, TRUE)) {
							break;
						}
					} 
					else if (m_pDetail->IsAllergiesTable()) {
						// (c.haag 2007-04-05 13:33) - PLID 25516 - Invoke a different one-time only warning for the Allergies
						// detail. If we bring it "up to date", it will load the up to date Allergies content and try to populate
						// it based on other up-to-date Allergies details or the patient's allergy list in the Medications tab.
						// (j.jones 2009-09-21 09:20) - PLID 35599 - we now do not pull from the patient's account when updating existing current
						// medication or allergy items, but since this message doesn't actually say that, we will leave it as-is
						// (j.jones 2010-11-12 08:56) - PLID 29075 - added a note that the topic will be immediately saved
						if(IDNO == DontShowMeAgain(this, "The Allergies item has been edited since it was added to this EMN.  "
							"Bringing it up to date will apply those edits to this detail, and may result in changes to the selections made on the detail. "
							"The changes will take effect immediately and cannot be undone. "
							"In addition, the topic will be saved immediately after bringing up to date.\r\n\r\n"
							"Are you sure you wish to continue?", "BringAllergiesItemUpToDate", "Bring item up to date", FALSE, TRUE)) {
							break;
						}
					}
					else {
						//TES 12/11/2006 - PLID 23790 - First, make sure they know what they're doing.
						// (j.jones 2010-11-12 08:56) - PLID 29075 - added a note that the topic will be immediately saved
						if(IDNO == DontShowMeAgain(this, "This item has been edited since it was added to this EMN.  "
							"Bringing it up to date will apply those edits (for example, adding or removing options to a list item) to this detail.  "
							"This change will take effect immediately and cannot be undone, and may result in changes to the value of this item "
							"(if, for example, a checkbox which is selected on this detail does not exist in the updated version of the item). "
							"In addition, the topic will be saved immediately after bringing up to date.\r\n\r\n"
							"Are you sure you wish to continue?", "BringItemUpToDate", "Bring item up to date", FALSE, TRUE)) {
							break;
						}
					}
					// (a.walling 2008-03-25 08:14) - PLID 28811 - Pass the detail pointer to the now static function
					UpdateItem(m_pDetail);
					break;
				}

				case miNewProblem: {
					NewProblem(eprtEmrItem);
					break;
				}

				case miEditProblem: {
					EditProblem(eprtEmrItem);
					break;
				}

				// (c.haag 2009-05-27 14:34) - PLID 34249 - We can now link this item with multiple problems
				case miExistingProblem: {
					LinkProblems(eprtEmrItem);
					break;
				}

				case miResetColumns: {
					m_pDetail->SetSaveTableColumnWidths(FALSE);

					m_pDetail->ResetStoredColumnWidths();

					// (j.jones 2006-07-31 11:03) - PLID 21699 - mark the item as unsaved
					m_pDetail->SetUnsaved();
					m_pDetail->m_pParentTopic->SetUnsaved();
					GetParent()->SendMessage(NXM_EMR_ITEM_CHANGED, (WPARAM)m_pDetail);	

					CRect rWindow;
					GetWindowRect(rWindow);
					RepositionControls(CSize(rWindow.Width(),rWindow.Height()),FALSE);
					break;
				}
				case miClearInk: {

					//grab the existing ink
					CEmrItemAdvImageState ais;
					ais.CreateFromSafeArrayVariant(m_pDetail->GetState());

					// (a.walling 2011-07-22 09:45) - PLID 44612 - Also check for text data
					if(ais.m_varInkData.vt > VT_NULL || ais.m_varTextData.vt > VT_NULL) {
						// (z.manning 2008-12-01 11:59) - PLID 32182 - Warn them before erasing ink.
						CString strInkWord = m_pDetail->Is3DImage() ? "stamps" : "ink";
						CString strDontShowPropName = m_pDetail->Is3DImage() ? "EmrEraseAllInkWarning3D" : "EmrEraseAllInkWarning";
						int nResult = DontShowMeAgain(this, "Are you sure you want to erase all " + strInkWord + " on this image?", strDontShowPropName, "Erase All", FALSE, TRUE);
						if(nResult == IDNO) {
							break;
						}
					}
					
					//clear out the ink
					VariantClear(&ais.m_varInkData);
					m_pDetail->SetInkErased();
					//TES 1/22/2007 - PLID 18159 - Also the text.
					VariantClear(&ais.m_varTextData);
					m_pDetail->SetImageTextRemoved();

					//assign the new state
					m_pDetail->SetInkData(ais.m_varInkData);
					m_pDetail->SetImageTextData(ais.m_varTextData);
					break;
				}
				// (d.thompson 2009-03-04) - PLID 32891 - Ability to remove the background image from the detail
				case miRemoveBackgroundImage: 
					{
						//Get the existing state to work from
						CEmrItemAdvImageState ais;
						ais.CreateFromSafeArrayVariant(m_pDetail->GetState());
						//wipe it all from the state
						ais.m_eitImageTypeOverride = itForcedBlank;
						ais.m_strImagePathOverride = "";

						//clear out the ink & text if necessary
						if(ais.m_varInkData.vt != VT_EMPTY && ais.m_varInkData.vt != NULL) {
							VariantClear(&ais.m_varInkData);
							m_pDetail->SetInkErased();
						}
						if(ais.m_varTextData.vt != VT_EMPTY && ais.m_varTextData.vt != VT_NULL) {
							VariantClear(&ais.m_varTextData);
							m_pDetail->SetImageTextRemoved();
						}
						//now assign the new state
						m_pDetail->RequestStateChange(ais.AsSafeArrayVariant());
					}
					break;
				case miRestoreSize: {
					// (a.walling 2011-05-25 17:57) - PLID 43847 - Call RestoreIdealSize
					((CEmrItemAdvImageDlg*)this)->RestoreIdealSize(false);
				}
				break;

				case miChangeStamp: 
				case miChangeAllStamps: {
					//TES 2/24/2012 - PLID 45127 - Pop up the dialog to allow them to select a new stamp yet.
					CSelectStampDlg dlg(this);
					//TES 3/28/2012 - PLID 49294 - We need to tell the dialog what image we are on
					dlg.m_nImageEmrInfoMasterID = m_pDetail->m_nEMRInfoMasterID;
					if(IDOK == dlg.DoModal()) {
						//TES 2/24/2012 - PLID 45127 - Actually change the stamp.
						CNxInkPictureText nipt;
						nipt.LoadFromVariant(m_pDetail->GetImageTextData());
						TextString tsSelected = nipt.GetTextString(nRightClickedStampIndex);
						dlg.GetSelectedStamp(tsSelected.nStampID, tsSelected.str, tsSelected.strTypeName, tsSelected.color);
						for(int nStamp = 0; nStamp < nipt.GetStringCount(); nStamp++) {
							//TES 3/8/2012 - PLID 48733 - Replace if this is the stamp we right-clicked on OR it has the same type, if
							// the Change All option was selected
							if(nStamp == nRightClickedStampIndex || (nResult == miChangeAllStamps && nipt.GetStampIDByIndex(nStamp) == nRightClickedStampID)) {
								TextString ts = nipt.GetTextString(nStamp);
								ts.nStampID = tsSelected.nStampID;
								ts.str = tsSelected.str;
								ts.strTypeName = tsSelected.strTypeName;
								ts.color = tsSelected.color;
								nipt.SetTextString(nStamp, ts);
							}
						}
						//TES 3/6/2012 - PLID 45127 - The detail will handle everything when it sees the new text data.
						m_pDetail->SetImageTextData(nipt.GetAsVariant());
						
					}
				}

				break;
				case miReconstructionVerified:
				case miReconstructionVerifiedKeepHighlight:
					{
						// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Confirm with the user and then write the new validated state to data.
						CString strAddMsg;
						long nNewReviewStatus;
						if (nResult == miReconstructionVerifiedKeepHighlight) {
							strAddMsg = " but the detail will remain highlighted white";
							nNewReviewStatus = -2;
						} else {
							strAddMsg = "";
							nNewReviewStatus = -3;
						}
						int nMsgAns = MessageBox("You have chosen to verify this reconstructed detail as correct in its current form.  If you proceed you will no longer be reminded for this detail" + strAddMsg + ".", "Reconstruction Reminder", MB_OKCANCEL|MB_ICONINFORMATION);
						if (nMsgAns == IDOK) {
							try {
								// Effect the change in the data
								ChangeReconstructedReviewState(m_pDetail->m_nEMRDetailID, m_pDetail->m_nReviewState, nNewReviewStatus);
								// Now that we've succeeded, store the new value in the detail in memory
								m_pDetail->m_nReviewState = nNewReviewStatus;
								// If we're on a topic container window, let it know that the topic holding this 
								// detail has changed its color and so maybe the color of the topic needs to 
								// change.  NOTE: This is a simple way of doing things so that we don't have to 
								// invent a whole bunch of extra logic just to handle this reconstruction data. 
								// This topic-coloring feature is not an important enough aspect of the 
								// reconstruction notification system to warrant such massive changes, so if this 
								// turns out to be not robust for some reason (but I think it is) then we should 
								// just take out the topic-coloring for reconstructed details.
								// (a.walling 2012-06-22 14:01) - PLID 51150 - Use the EMN's interface wnd
								if (m_pDetail->m_pParentTopic && m_pDetail->m_pParentTopic->GetInterfaceWnd()) {
									m_pDetail->m_pParentTopic->GetInterfaceWnd()->PostMessage(NXM_TOPIC_MODIFIED_CHANGED, (WPARAM)m_pDetail->m_pParentTopic, 0);
								}
								// Now refresh all active views.  This is another quick implementation that if 
								// this were an official feature we would need to do more efficiently.  The goal 
								// is to cause the NexEMR tab of the patients module to refresh, since we've 
								// just changed data that could cause one of its EMNs to no longer be light pink.
								CMainFrame *pMainFrm = GetMainFrame();
								if (pMainFrm->GetSafeHwnd()) {
									pMainFrm->UpdateAllViews();
								}
								// Reflect it on screen
								CRect rc;
								GetWindowRect(&rc);
								::InvalidateRect(NULL, rc, TRUE);
							} NxCatchAll("miReconstructionVerified");
						}
					}
				break;
				case miPopup:
					{
						// (a.walling 2007-06-21 16:01) - PLID 22097 - Need to popup this detail then
						try {
							m_pDetail->Popup();
						} NxCatchAll("Error popping up detail!");
					}
					break;
				case miHideTitle:
					{
						// (a.walling 2008-07-01 09:51) - PLID 30570
						m_pDetail->SetPreviewFlags(m_pDetail->GetPreviewFlags() ^ epfHideTitle);
					}
					break;
				case miHideItem:
					{
						// (a.walling 2008-07-01 09:51) - PLID 30570
						m_pDetail->SetPreviewFlags(m_pDetail->GetPreviewFlags() ^ epfHideItem);
					}
					break;
				case miDisplayUnderSpawningDetail:
					{
						// (a.walling 2008-10-23 10:11) - PLID 31808 - Toggle display as subdetail
						m_pDetail->SetPreviewFlags(m_pDetail->GetPreviewFlags() ^ epfSubDetail);
					}
					break;
				case miHideIfIndirectlyOnNarrative:
					{
						// (a.walling 2012-07-13 16:38) - PLID 48896
						m_pDetail->SetPreviewFlags(m_pDetail->GetPreviewFlags() ^ epfHideIfIndirect);
					}
					break;
				case miHideItemOnIPad:
					{
						// (j.armen 2013-01-16 16:41) - PLID 54412
						m_pDetail->SetPreviewFlags(m_pDetail->GetPreviewFlags() ^ epfHideOnIPad);
					}
					break;
				case miColumnOne:
					{
						// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
						// (a.walling 2009-01-08 14:01) - PLID 32659 - Toggle float/clear options
						DWORD nPreviewFlags = m_pDetail->GetPreviewFlags();
						nPreviewFlags &= ~epfColumnTwo;
						nPreviewFlags ^= epfColumnOne;

						m_pDetail->SetPreviewFlags(nPreviewFlags, TRUE, FALSE);
					}
					break;
				case miColumnTwo:
					{
						// (a.walling 2009-07-06 10:14) - PLID 34793 - epfFloatLeft is now epfColumnOne, epfFloatRight is now epfColumnTwo
						// (a.walling 2009-01-08 14:01) - PLID 32659 - Toggle float/clear options
						DWORD nPreviewFlags = m_pDetail->GetPreviewFlags();
						nPreviewFlags &= ~epfColumnOne;
						nPreviewFlags ^= epfColumnTwo;

						m_pDetail->SetPreviewFlags(nPreviewFlags, TRUE, FALSE);
					}
					break;
				
				// (a.walling 2009-07-06 08:41) - PLID 34793 - Clearing is deprecated
				/*
				case miClearLeft:
					{
						// (a.walling 2009-01-08 14:01) - PLID 32659 - Toggle float/clear options
						DWORD nPreviewFlags = m_pDetail->GetPreviewFlags();
						
						if ((nPreviewFlags & epfClearNone) == 0) { // both
							nPreviewFlags ^= epfClearRight;
						} else {
							if ((nPreviewFlags & epfClearNone) == epfClearNone) { // none
								nPreviewFlags &= ~epfClearRight;
								nPreviewFlags |= epfClearLeft;
							} else if (nPreviewFlags & epfClearRight) { // right
								nPreviewFlags &= ~epfClearRight;
								nPreviewFlags &= ~epfClearLeft;
							} else if (nPreviewFlags & epfClearLeft) { // left
								nPreviewFlags |= epfClearNone;
							}
						}

						m_pDetail->SetPreviewFlags(nPreviewFlags, FALSE, TRUE);
					}
					break;
				case miClearRight:
					{
						// (a.walling 2009-01-08 14:01) - PLID 32659 - Toggle float/clear options
						DWORD nPreviewFlags = m_pDetail->GetPreviewFlags();
				
						if ((nPreviewFlags & epfClearNone) == 0) { // both
							nPreviewFlags ^= epfClearLeft;
						} else {
							if ((nPreviewFlags & epfClearNone) == epfClearNone) { // none
								nPreviewFlags &= ~epfClearLeft;
								nPreviewFlags |= epfClearRight;
							} else if (nPreviewFlags & epfClearLeft) { // left
								nPreviewFlags &= ~epfClearLeft;
								nPreviewFlags &= ~epfClearRight;
							} else if (nPreviewFlags & epfClearRight) { // right
								nPreviewFlags |= epfClearNone;
							}
						}		

						m_pDetail->SetPreviewFlags(nPreviewFlags, FALSE, TRUE);
					}
					break;
				*/
				case miTextRight:
					{
						// (a.walling 2009-01-08 14:07) - PLID 32660 - Align text right
						DWORD nPreviewFlags = m_pDetail->GetPreviewFlags();
						nPreviewFlags ^= epfTextRight;
						m_pDetail->SetPreviewFlags(nPreviewFlags, FALSE, TRUE);
					}
					break;
				case miPageBreakBefore:
					{
						// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
						DWORD nPreviewFlags = m_pDetail->GetPreviewFlags();
						nPreviewFlags &= ~epfPageBreakAfter;
						nPreviewFlags ^= epfPageBreakBefore;
						m_pDetail->SetPreviewFlags(nPreviewFlags, FALSE, TRUE);
					}
					break;
				case miPageBreakAfter:
					{
						// (a.walling 2010-08-31 18:20) - PLID 36148 - Page breaks
						DWORD nPreviewFlags = m_pDetail->GetPreviewFlags();
						nPreviewFlags &= ~epfPageBreakBefore;
						nPreviewFlags ^= epfPageBreakAfter;
						m_pDetail->SetPreviewFlags(nPreviewFlags, FALSE, TRUE);
					}
					break;
				case miEnableHotSpotClicks: // (c.haag 2009-02-17 17:52) - PLID 31327
					if (NULL != m_pDetail && NULL != m_pDetail->m_pEmrItemAdvDlg) {
						((CEmrItemAdvImageDlg*)m_pDetail->m_pEmrItemAdvDlg)->SetEnableHotSpotClicks(TRUE);
					}
					break;
				case miDisableHotSpotClicks: // (c.haag 2009-02-17 17:52) - PLID 31327
					if (NULL != m_pDetail && NULL != m_pDetail->m_pEmrItemAdvDlg) {
						((CEmrItemAdvImageDlg*)m_pDetail->m_pEmrItemAdvDlg)->SetEnableHotSpotClicks(FALSE);
					}
					break;
				case miResetTableHeight: 
					// (c.haag 2011-03-30) - PLID 43046 - Resize the table so it can fit at least ten standard rows.
					// This is intended to help a client vertically size down a Current Medications or Allergies item so
					// that they don't have to scroll all the way down the topic and pull the bottom back up. They're
					// still responsible for doing any fine tuning beyond this.
					{
						CRect rcOld;
						GetWindowRect(&rcOld);
						int nNewHeight = 350;

						// Increase the height if a merge button is present. Sure it looks silly that if you reset the height,
						// assign a problem, and then reset the height again that the detail looks bigger...but all we care
						// about is the space made available to the table.
						if (IsMergeButtonVisible() || IsProblemButtonVisible()) {
							CSize szMergeBtn;
							long cnMergeBtnMargin = 5; // Amount of space above and below the merge button
							CClientDC dc(this);
							CSize sz(LONG_MAX, LONG_MAX);
							CalcControlIdealDimensions(&dc, m_pBtnMergeStatus, szMergeBtn);
							nNewHeight += cnMergeBtnMargin + szMergeBtn.cy;
						}
						
						// Set the height. Do not set the SWP_NOREDRAW flag because we need to redraw the region that
						// is being revealed with this table shrinking.
						SetWindowPos(NULL, 0, 0, rcOld.Width(), nNewHeight, SWP_NOMOVE|SWP_NOZORDER);

						// (a.walling 2012-04-02 08:29) - PLID 49304 - Handled via OnSizing
					}					
					break;
				// (c.haag 2011-03-30) - PLID 43047 - Be able to edit common lists
				case miEditCommonLists:
					OpenItemEntryDlg(m_pDetail, m_bIsTemplate, eEmrItemEntryDlgBehavior_EditCommonLists);
					break;

				case 0:
					// The user canceled, do nothing
					break;
				default:
					// (c.haag 2007-04-18 16:55) - PLID 25705 - Old crazy implementation
					/*if(nResult < 100) {
						//not a EMRInfo item
						ASSERT(FALSE);
					}*/

					//if not locked
					if(pEMN->GetStatus() != 2) {

						// (c.haag 2007-04-18 16:52) - PLID 25705 - No need for all this crazyness. We
						// stored the detail object before the popup menu was tracked, just extract it there
						long nIndexToPopUp = nResult - miFirstAvailableMenuItem;
						if (nIndexToPopUp >= 0 && nIndexToPopUp < apDetailMenuItems.GetSize()) {
							CEMNDetail* pDetail = apDetailMenuItems[nIndexToPopUp];
							pDetail->Popup();
						}

						/*
						long nIDToPopup = nResult / 100;

						BOOL bFound = FALSE;

						CEMN *pEMN = m_pDetail->m_pParentTopic->GetParentEMN();


						long nTotalDetailCount = pEMN->GetTotalDetailCount();
						for(int i=nTotalDetailCount-1; i>=0 && !bFound; i--) {

							CEMNDetail *pEMNDetail = pEMN->GetDetail(i);

							//Is this the correct InfoID, and does its SourceActionID represent one of our checkboxes?
							if(pEMNDetail->m_nEMRInfoID == nIDToPopup &&
								pEMNDetail->GetSourceActionID() != -1 &&
								ReturnsRecords("SELECT ID FROM EmrActionsT WHERE ID = %li AND SourceType = %li AND SourceID IN "
								"(SELECT ID FROM EmrDataT WHERE EmrInfoID = %li)",
								pEMNDetail->GetSourceActionID(), eaoEmrDataItem, m_pDetail->m_nEMRInfoID)) {

								bFound = TRUE;

								pEMNDetail->Popup();
							}
						}*/
					}
					break;
				}
			}
		}
	} NxCatchAll("CEmrItemAdvDlg::OnContextMenu");
}

void CEmrItemAdvDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// Call the normal implementation to ensure *lpMMI is filled properly
		CWnd::OnGetMinMaxInfo(lpMMI);

		// Then extend the maximum allowed size to be essentially unlimited (we use MAX_LONG 
		// because currently windows has no flag that indicates "unlimited").
		lpMMI->ptMaxTrackSize = CPoint(LONG_MAX, LONG_MAX);
	} NxCatchAll("Error in OnGetMinMaxInfo");
}

void CEmrItemAdvDlg::OnNcRButtonDown(UINT nHitTest, CPoint point)
{
	// (b.savon 2012-06-01 11:14) - PLID 49351 - Moved this code to OnNcRButtonUp(...)

	////DRT 8/21/2007 - PLID 27133 - Added try/catch
	//try {
	//	// If we're allowing edit, that means we're interpreting the label area as the window caption (so 
	//	// it can be grabbed and moved from there) but that means the system won't send the context menu 
	//	// message if the user right-clicks there because to the system, the user right-clicked on a non-
	//	// client area (like a border or something, in this case on the window caption), so we have to 
	//	// manually treat the right-click as the context menu.
	//	if (m_pDetail->GetAllowEdit() && nHitTest == HTCAPTION) {
	//		// Do the context menu
	//		OnContextMenu(this, point);
	//		// And since the result may very well be that we don't exist anymore, DO NOT do that standard 
	//		// nc-right-click functionality.
	//	} else {
	//		// Normal case, call the standard functionality
	//		CWnd::OnNcRButtonDown(nHitTest, point);
	//	}
	//} NxCatchAll("Error in OnNcRButtonDown");

	try{
		CWnd::OnNcRButtonDown(nHitTest, point);
	}NxCatchAll("Error in OnNcRButtonDown");

}

// (b.savon 2012-06-01 11:14) - PLID 49351 - Handle the Right click properly in Edit Mode
void CEmrItemAdvDlg::OnNcRButtonUp(UINT nHitTest, CPoint point)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// If we're allowing edit, that means we're interpreting the label area as the window caption (so 
		// it can be grabbed and moved from there) but that means the system won't send the context menu 
		// message if the user right-clicks there because to the system, the user right-clicked on a non-
		// client area (like a border or something, in this case on the window caption), so we have to 
		// manually treat the right-click as the context menu.
		if (m_pDetail->GetAllowEdit() && nHitTest == HTCAPTION) {
			// Do the context menu
			OnContextMenu(this, point);
			// And since the result may very well be that we don't exist anymore, DO NOT do that standard 
			// nc-right-click functionality.
		} else {
			// Normal case, call the standard functionality
			CWnd::OnNcRButtonUp(nHitTest, point);
		}
	} NxCatchAll("Error in OnNcRButtonUp");
}

int CEmrItemAdvDlg::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (b.cardillo 2004-07-07 14:13) - PLID 13341 - Whenever the user clicks on us we want to bring 
		// ourselves to the front, just like top-level windows do.
		// Even if CWnd::OnMouseActivate() is going to return that we don't want to be activated, we 
		// still want to bring the window to the top because the user clicked on it (the primary example 
		// of this situation is when the user is dragging the window to move or size it)
		BringWindowToTop();

		// (b.cardillo 2006-06-20 13:04) - PLID 13897 - Remember the window rect RELATIVE to the 
		// mouse position so that later we can reverse it.
		{
			// Get window rect relative to the screen
			CRect rcWindow;
			GetWindowRect(&rcWindow);
			// Get the mouse pos relative to the screen
			CPoint point(GetMessagePos());
			// Get each side of the rect's offset from the mouse pos, and store it in a RECT structure
			m_rcLastLButtonDownWindowAdj.SetRect(point.x - rcWindow.left, point.y - rcWindow.top, point.x - rcWindow.right, point.y - rcWindow.bottom);
		}
	} NxCatchAll("Error in OnMouseActivate");

	// Call the base class
	return CWnd::OnMouseActivate(pDesktopWnd, nHitTest, message);;
}

void CEmrItemAdvDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		if(m_pToolTipWnd)m_pToolTipWnd->ShowWindow(SW_HIDE);
		KillTimer(HOVER_TICK);
	} NxCatchAll("Error in OnLButtonDown");

	CWnd::OnLButtonDown(nFlags, point);
}

void CEmrItemAdvDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		//The mouse just moved, so reset the timer.
		KillTimer(HOVER_TICK);
		//Set a tooltip timer.
		SetTimer(HOVER_TICK, 50, NULL);
	} NxCatchAll("Error in OnMouseMove");

	CWnd::OnMouseMove(nFlags, point);
}

void CEmrItemAdvDlg::OnTimer(UINT nIDEvent)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		switch (nIDEvent) {
		case HOVER_TICK:
			{
				try {
					try {
						// (a.wetta 2006-07-28 14:23) - The tooltip causes whatever currently has the focus to lose focus which can cause problems.
						// The tooltip doesn't really need to have focus anyway, so record what has focus right now and set it back to it after
						// we're done working with the tooltip.
						CWnd* wndFocus = GetFocus();
						if(!m_bEnableTooltips) {
							KillTimer(HOVER_TICK);
							KillTimer(KILL_TIP_NCMOVE);
							if(m_pToolTipWnd) {
								m_pToolTipWnd->ShowWindow(SW_HIDE);
							}
						}
						else {
							//TES 9-23-03: Don't show the tip if the mouse button down.
							if(GetAsyncKeyState(VK_LBUTTON) & 0xE000 ||	GetAsyncKeyState(VK_RBUTTON) & 0xE000) {				
								if(m_pToolTipWnd) {
									m_pToolTipWnd->ShowWindow(SW_HIDE);
								}
							}
							else {
								//Is the mouse still over us?
								// (c.haag 2006-12-29 11:22) - PLID 23977 - This code used to check if the
								// mouse was over any part of this detail. I could have sworn this used
								// to not happen, but in any case, this is fixed now.
								POINT pointCursor;
								GetCursorPos(&pointCursor);

								CRect rLabelWindow;
								GetControlWindowRect(this, &m_wndLabel, &rLabelWindow);

								CRect rThisWindow;
								GetWindowRect(&rThisWindow);

								rLabelWindow.right = rThisWindow.right;

								if(rLabelWindow.PtInRect(pointCursor)) {
									if(!m_pToolTipWnd) {

										//We'll create it docked to the top right of the label, but at least 5 pixels inside our window..
										CRect rLabel;
										GetControlWindowRect(this, &m_wndLabel, &rLabel);

										// (a.wetta 2006-11-07 11:29) - PLID 22626 - Create the tool tip alignment preference information
										ToolTipSmartAlignInfo *ttsai = new ToolTipSmartAlignInfo;
										// First align preference
										ttsai->arybPrefBottom.Add(true);
										ttsai->arybPrefLeft.Add(true);
										// Second align preference
										ttsai->arybPrefBottom.Add(false);
										ttsai->arybPrefLeft.Add(false);
										// Third align preference
										ttsai->arybPrefBottom.Add(true);
										ttsai->arybPrefLeft.Add(false);
										
										// (a.walling 2010-06-23 17:06) - PLID 39330 - Call CNxToolTip's static helper classes to create this
										m_pToolTipWnd = CNxToolTip::CreateNxToolTip(this, &rLabel.TopLeft(), -5, -50);

										// (a.wetta 2007-06-15 09:04) - PLID 24268 - Make sure that the smart align only uses the three align
											// preferences above so that the tool tip never covers the EMR item itself
										m_pToolTipWnd->SetPosition(rLabel.left-5, rLabel.top, true, true, true, ttsai, true);
									}

									if(!m_pToolTipWnd->IsWindowVisible() || m_pToolTipWnd->m_strTip != GetToolTipText()) {

										// (c.haag 2003-10-01 09:09) - If there is no tip text, our
										// goal is to hide the tooltip.

										// (c.haag 2004-12-28 11:22) - Trim off whitespace related
										// characters. I would use CString::TrimRight for this but
										// it hasn't worked from my experience.
										//TES 10/17/2005 - We don't want to trim whitespace characters, we just want to make sure
										//that we're not displaying an entirely blank tooltip.
										CString strToolTipText = GetToolTipText();
										CString strTrimmed = strToolTipText;
										while (strTrimmed.GetLength())
										{
											char c = strTrimmed[ strTrimmed.GetLength() - 1 ];
											if (c == '\r' || c == '\n' || c == '\t')
											{
												strTrimmed = strTrimmed.Left( strTrimmed.GetLength() - 1 );
											}
											else {
												break;
											}
										}

										if (strTrimmed.IsEmpty())
										{
											// If it's visible, make it invisible.
											if(m_pToolTipWnd->IsWindowVisible()) {								
												m_pToolTipWnd->ShowWindow(SW_HIDE);
											}
											// If it's invisible, don't do anything since that's
											// what we want.

											// Now empty the tool tip text because we should always leave
											// this case statement having m_strTip equal to GetTextForTip().
											m_pToolTipWnd->m_strTip.Empty();
										}
										else
										{
											m_pToolTipWnd->m_strTip = strToolTipText;
											//This will tell it to set its position and everything.
											m_pToolTipWnd->PrepareWindow();
											//Now, move it for when it next shows up.
											CRect rLabel;
											GetControlWindowRect(this, &m_wndLabel, &rLabel);

											// (a.wetta 2006-11-07 11:29) - PLID 22626 - Create the tool tip alignment preference information
											ToolTipSmartAlignInfo *ttsai = new ToolTipSmartAlignInfo;
											// First align preference
											ttsai->arybPrefBottom.Add(true);
											ttsai->arybPrefLeft.Add(true);
											// Second align preference
											ttsai->arybPrefBottom.Add(false);
											ttsai->arybPrefLeft.Add(false);
											// Third align preference
											ttsai->arybPrefBottom.Add(true);
											ttsai->arybPrefLeft.Add(false);
											
											// (a.wetta 2007-06-15 09:04) - PLID 24268 - Make sure that the smart align only uses the three align
											// preferences above so that the tool tip never covers the EMR item itself
											m_pToolTipWnd->SetPosition(rLabel.left-5, rLabel.top, true, true, true, ttsai, true);
											m_pToolTipWnd->Refresh();
											CRect rParent;
											GetParent()->GetWindowRect(rParent);
											m_pToolTipWnd->ShowWindow(m_bEnableTooltips && rLabel.top > rParent.top ? SW_SHOWNOACTIVATE : SW_HIDE);
											//Start the timers to dismiss the tooltip.
											SetTimer(KILL_TIP_NCMOVE, 100, NULL); //If they move the mouse out of the window.
										}
									}
								}
								else {
									if(m_pToolTipWnd) {
										m_pToolTipWnd->ShowWindow(SW_HIDE);
										KillTimer(KILL_TIP_NCMOVE);
									}
								}
							}
							KillTimer(HOVER_TICK);
						}
						if (wndFocus && IsWindow(wndFocus->GetSafeHwnd()))
							wndFocus->SetFocus();
					//TES 2/26/2007 - PLID 24924 - If we get an error, then it is reasonable to assume that we'll keep getting
					// it every time the trigger is fired.  Therefore, we need to kill the timer, and turn off tooltips, so that
					// the user isn't bombarded with this error every 50 ms until they move their mouse off of the label.  Also,
					// we can't use NxCatchAllCall(), because that doesn't execute the code until after the user dismisses the
					// error message, meaning that they could end up with dozens of error messages popped up on top of each other.
					// So, we will just disable tooltips, and then re-throw the exception so it can be handled by our standard
					// exception handler.
					}catch(CNxException *e) {
						KillTimer(HOVER_TICK);
						m_bEnableTooltips = false; 
						throw e;
					}
					catch(_com_error e) {
						KillTimer(HOVER_TICK);
						m_bEnableTooltips = false;
						throw e;
					}
					catch(...) {
						KillTimer(HOVER_TICK);
						m_bEnableTooltips = false;
						throw;
					}
				}NxCatchAllCall("Error in CEmrItemAdvDlg::OnTimer() : HOVER_TICK", KillTimer(HOVER_TICK)); 
			}
			break;
		case KILL_TIP_NCMOVE:
			{
				// (a.wetta 2006-07-28 14:23) - The tooltip causes whatever currently has the focus to lose focus which can cause problems.
				// The tooltip doesn't really need to have focus anyway, so record what has focus right now and set it back to it after
				// we're done working with the tooltip.
				CWnd* wndFocus = GetFocus();
				//Is the mouse still over us?
				
				POINT pointCursor;
				GetCursorPos(&pointCursor);

				CRect rLabelWindow;
				GetControlWindowRect(this, &m_wndLabel, &rLabelWindow);

				CRect rThisWindow;
				GetWindowRect(&rThisWindow);

				rLabelWindow.right = rThisWindow.right;

				// (c.haag 2006-12-29 11:22) - PLID 23977 - This code used to check if the
				// mouse was over any part of this detail. I could have sworn this used
				// to not happen, but in any case, this is fixed now.
				if(!rLabelWindow.PtInRect(pointCursor)) {
					CRect rTip;
					if(m_pToolTipWnd) {
						m_pToolTipWnd->GetWindowRect(rTip);
						if(!rTip.PtInRect(pointCursor)) {
							m_pToolTipWnd->ShowWindow(SW_HIDE);
							KillTimer(KILL_TIP_NCMOVE);
						}
					}
					else {
						KillTimer(KILL_TIP_NCMOVE);
					}
				}
				if (wndFocus && IsWindow(wndFocus->GetSafeHwnd()))
					wndFocus->SetFocus();
			}
			break;
		}
	} NxCatchAll("Error in OnTimer");
}


CString CEmrItemAdvDlg::GetToolTipText()
{
	CString strText;

	// (c.haag 2007-05-17 10:18) - PLID 26046 - Use GetStateVarType to get the detail state type
	if(m_pDetail->GetStateVarType() == VT_NULL || m_pDetail->GetStateVarType() == VT_BSTR && VarString(m_pDetail->GetState()).IsEmpty()) {
		//if no data is entered, pull the default sentence format, and do not replace any of the fields
		m_pDetail->LoadContent(); //will do nothing if already loaded
		if(m_pDetail->m_EMRInfoType == 4)
			strText = "";
		else
			//TES 2/26/2010 - PLID 37463 - Check whether to use the "Smart Stamps" long form
			// (z.manning 2010-07-26 15:18) - PLID 39848 - All tables now use the same long form
			strText = m_pDetail->m_strLongForm;
	}
	else {
		//otherwise call GetSentence() which will return a properly formatted sentence, based on the data selected
		strText = m_pDetail->m_pParentTopic->GetParentEMN()->GetSentence(m_pDetail, NULL, false, false);;
	}

	return strText; // good place to add some debug info
}
void CEmrItemAdvDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		if(!bShow) {
			if(m_pToolTipWnd) {
				m_pToolTipWnd->ShowWindow(SW_HIDE);
				KillTimer(KILL_TIP_NCMOVE);
			}
		}
	} NxCatchAll("Error in OnShowWindow");
	CWnd::OnShowWindow(bShow, nStatus);
}

void CEmrItemAdvDlg::OnButtonClickedMergeConflict()
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {

		BOOL bDone = FALSE;
		if (!m_pDetail) return;

		CEMN *pEMN = m_pDetail->m_pParentTopic->GetParentEMN();
		BOOL bIsLockedEMN = pEMN->GetStatus() == 2;

		// (j.jones 2009-12-17 10:10) - PLID 31903 - should not be allowed if read only
		//TES 3/15/2010 - PLID 37757 - Check our detail's read only status
		if(bIsLockedEMN || m_pDetail->GetReadOnly()) {
			//assert, just because we should not allow the user to perform an action
			//that gets this far, so find out how we got here
			ASSERT(FALSE);
			return;
		}

		CString strResult = m_pDetail->GetMergeFieldName(FALSE);
		while (!bDone)
		{
			if (IDOK == InputBox(this, "Please enter a merge field name for this item", strResult, "")) {

				switch (IsValidEMRMergeFieldName(strResult))
				{
				case einrOK:
					bDone = TRUE;
					break;
				case einrTooLong:
					MsgBox("You may not enter a field name greater than %d characters long.", MAX_EMR_ITEM_NAME_MERGE_LENGTH);
					break;
				case einrReservedName:
					MsgBox("The field name you have entered is reserved for system use. Please choose another.");
					break;
				}
			}
			else {
				return;
			}
		}
		if (strResult != m_pDetail->GetMergeFieldName(FALSE))
		{
			// Let the user know that empty names are synonymous with non-overriding
			if (!strResult.GetLength()) {
				MsgBox("You have chosen an empty name. The merge field for the '%s' detail will now default to its display name.", m_pDetail->GetLabelText());
			}

			// If the result is the same as the label text, we will treat that as a sign
			// the user wants the name to be synonymous with the label, hence we will not
			// have an override.
			if (strResult == m_pDetail->GetLabelText()) {
				strResult.Empty();
			}

			// Make sure every other tab reflects the new merge button icon state, if any,
			// by ensuring they will reposition their controls the next time they are
			// visible.
			MergeOverrideChanged moc;
			moc.strOldName = m_pDetail->GetMergeFieldName(TRUE);
			// Now assign our new override value
			m_pDetail->SetMergeFieldOverride(strResult);
			moc.strNewName = m_pDetail->GetMergeFieldName(TRUE);
			GetParent()->SendMessage(NXM_MERGE_OVERRIDE_CHANGED, (WPARAM)m_pDetail, (LPARAM)&moc);
				
			//TES 3/16/2005 - This is our label now, so make sure it's on screen.
			ReflectCurrentState();
		}
	} NxCatchAll("Error in OnButtonClickedMergeConflict");
}

void CEmrItemAdvDlg::OnButtonClickedProblem()
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {

		EditProblem(eprtEmrItem);

	} NxCatchAll("Error in OnButtonClickedProblem");
}

// (j.jones 2008-07-21 08:36) - PLID 30779 - added NewProblem because we can now have multiple
// problems per detail
void CEmrItemAdvDlg::NewProblem(EMRProblemRegardingTypes eprtType, long nEMRDataID /*= -1*/)
{
	try {

		if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
			return;
		}

		CEMN *pEMN = m_pDetail->m_pParentTopic->GetParentEMN();

		// (j.jones 2008-07-28 09:54) - PLID 30854 - confirm that we have exclusive access to the EMN,
		// this is independent of locking
		if(!pEMN->IsWritable()) {
			//prompt, but do not attempt to take access, let the user decide what to do
			AfxMessageBox("You do not currently have access to this EMN. You must take write access to the EMN to be able to add problems.");
			return;
		}

		// (c.haag 2006-06-29 15:07) - PLID 19977 - We can now mark a detail as a problem.
		CEMRProblemEditDlg dlg(this);

		CString strValue;
		if(m_pDetail->GetStateVarType() == VT_NULL || m_pDetail->GetStateVarType() == VT_BSTR && VarString(m_pDetail->GetState()).IsEmpty()) {
			//TES 2/26/2010 - PLID 37463 - Check whether to use the "Smart Stamps" long form
			// (z.manning 2010-07-26 15:18) - PLID 39848 - All tables use the regular long form
			strValue = m_pDetail->m_strLongForm;
		} else {
			CStringArray saDummy;
			// (c.haag 2006-11-14 10:49) - PLID 23543 - If the info type is an image, we may get a debug assertion
			// failure when calling GetSentence. Rather than try to get a sentence, just return a sentinel value.
			if (eitImage == m_pDetail->m_EMRInfoType) {
				strValue = "<image>";
			} else {
				// (c.haag 2008-02-22 13:53) - PLID 29064 - GetSentence may access the database when doing calculations on
				// dropdown table columns. Make sure we pass in our connection object so it won't try to use the global one
				// which belongs to the main thread.
				strValue = ::GetSentence(m_pDetail, NULL, false, false, saDummy, ecfParagraph, NULL, NULL, NULL);
			}
		}

		// (j.jones 2008-07-25 10:21) - PLID 30727 - added an ID parameter
		// (j.jones 2009-05-22 13:50) - PLID 34250 - renamed this function and sent in the value of the detail
		// (c.haag 2009-05-26 10:06) - PLID 34312 - We now use the new problem linking structure
		dlg.AddLinkedObjectInfo(-1, eprtType, m_pDetail->GetMergeFieldOverride().IsEmpty() ? m_pDetail->GetLabelText() : m_pDetail->GetMergeFieldOverride(), strValue, 
			m_pDetail->m_nEMRDetailID, NULL, nEMRDataID);

		// (c.haag 2006-11-13 14:56) - PLID 22052 - If the EMN is locked, we can't edit it. Therefore,
		// any changes made to the problem cannot be pooled in with the mass EMN update query. What
		// we must instead do is save the changes on the fly from the dialog itself		
		CEMR* pEMR = (pEMN) ? pEMN->GetParentEMR() : NULL;
		const BOOL bEMNIsLocked = (pEMN->GetStatus() == 2) ? TRUE : FALSE;
		if (bEMNIsLocked) {
			dlg.SetWriteToData(TRUE); // This will force the dialog to save the problem details to data
		}
		dlg.SetPatientID(pEMR->GetPatientID());

		if (IDOK == dlg.DoModal()) {
			// (c.haag 2006-12-27 11:25) - PLID 23158 - If the problem was deleted, update the
			// detail so that the button is properly updated
			if (!dlg.ProblemWasDeleted()) {

				COleDateTime dtInvalid;
				dtInvalid.SetStatus(COleDateTime::invalid);

				// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
				long nDiagICD9CodeID = -1, nDiagICD10CodeID = -1;
				dlg.GetProblemDiagCodeIDs(nDiagICD9CodeID, nDiagICD10CodeID);

				// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
				// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
				// (c.haag 2009-05-26 10:07) - PLID 34312 - Create the problem
				// (b.spivey, October 22, 2013) - PLID 58677 - added codeID
				// (s.tullis 2015-02-23 15:44) - PLID 64723
				// (r.gonet 2015-03-09 18:21) - PLID 65008 - Pass DoNotShowOnProblemPrompt.
				CEmrProblem *pProblem = pEMR->AllocateEmrProblem(dlg.GetProblemID(), pEMR->GetPatientID(), dlg.GetProblemDesc(), COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), dlg.GetOnsetDate(),
					dlg.GetProblemStatusID(), nDiagICD9CodeID, nDiagICD10CodeID, dlg.GetProblemChronicityID(), !bEMNIsLocked, dlg.GetProblemCodeID(),dlg.GetProblemDoNotShowOnCCDA(), 
					dlg.GetProblemDoNotShowOnProblemPrompt());
				// (c.haag 2009-05-26 10:07) - PLID 34312 - Create the problem link
				CEmrProblemLink* pNewLink = new CEmrProblemLink(pProblem, -1, eprtType, m_pDetail->m_nEMRDetailID, nEMRDataID);
				pNewLink->UpdatePointersWithDetail(m_pDetail);
				m_pDetail->m_apEmrProblemLinks.Add(pNewLink);
				pProblem->Release();

				if(!bEMNIsLocked) {
					m_pDetail->SetUnsaved();
					m_pDetail->m_pParentTopic->SetUnsaved();
					GetParent()->SendMessage(NXM_EMR_ITEM_CHANGED, (WPARAM)m_pDetail);
				}
				// (c.haag 2009-06-01 13:17) - PLID 34312 - We need to flag the EMR as unsaved because it is now the "manager" of problems
				pEMR->SetUnsaved();

				// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
				GetParent()->PostMessage(NXM_EMR_PROBLEM_CHANGED);
			}

			// Make sure the problem label is on the screen if necessary
			CRect rWindow;
			GetWindowRect(rWindow);
			ReflectCurrentState();
			UpdateProblemStatusButtonAppearance();

			// Now resize the window so that the problem flag button can be seen
			// (c.haag 2006-12-13 08:59) - PLID 22052 - Do not do any repositioning if the EMN is locked
			// or else it will set the modified flag
			if(!bEMNIsLocked) {
				// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
				if(m_pDetail->m_pParentTopic->GetTopicWnd() && IsWindow(m_pDetail->m_pParentTopic->GetTopicWnd()->GetSafeHwnd())) {
					// (a.wetta 2007-02-23 14:46) - PLID 24511 - When updating an image we don't want it to resize itself, 
					// so tell the image to size to the detail, aka don't go back to the original dimensions
					// (a.walling 2011-05-25 17:57) - PLID 43847 - This is now no longer a property of the detail.
					//m_pDetail->SetSizeImageToDetail(TRUE);
					// (j.jones 2007-07-26 09:31) - PLID 24686 - renamed this function
					m_pDetail->m_pParentTopic->GetTopicWnd()->RepositionDetailsInTopicByInfoID(m_pDetail->m_nEMRInfoID, FALSE);
					//m_pDetail->SetSizeImageToDetail(FALSE);
				}
			}
		}

	}NxCatchAll("Error in CEmrItemAdvDlg::NewProblem");
}

void CEmrItemAdvDlg::EditProblem(EMRProblemRegardingTypes eprtType, long nEMRDataID /*= -1*/)
{
	try {

		// (c.haag 2006-11-14 15:10) - PLID 23158 - Ensure the problem-related content of m_pDetail 
		// is up to date. If it's not, we need to update the button appearance, too.
		if (CheckForProblemDataChanges()) {
			// Now check if someone deleted the problem; if they have, we have to scold; er, warn
			// the user
			if (!m_pDetail->HasProblems()) {
				MsgBox("A problem has been deleted from your data. You will not be able to edit it.");
				UpdateProblemStatusButtonAppearance();
				return;
			} else {
				// (c.haag 2006-12-27 15:02) - PLID 23158 - We could have been the one to modify our own problem,
				// so do not raise a prompt.
				//MsgBox("This problem has been modified since it was loaded. Please review the problem details thoroughly before saving your changes.");
				UpdateProblemStatusButtonAppearance();
			}
		}

		// (j.jones 2008-07-17 17:11) - PLID 30729 - Rather than call the problem edit dialog,
		// we now open the problem list, from which you can pick a problem to edit.
		// However when called from here, it will open modally, and we provide the memory objects
		// of all the problems on this detail.

		//close the current problem list, if there is one
		CMainFrame *pFrame = GetMainFrame();
		if(pFrame) {
			pFrame->SendMessage(NXM_EMR_DESTROY_PROBLEM_LIST);
		}

		CEMRProblemListDlg dlg(this);
		dlg.SetDefaultFilter(m_pDetail->GetPatientID(), eprtType, m_pDetail->GetID(), m_pDetail->GetMergeFieldOverride().IsEmpty() ? m_pDetail->GetLabelText() : m_pDetail->GetMergeFieldOverride());

		//we will filter either on the detail, or the requested list item
		// (c.haag 2009-05-26 10:13) - PLID 34312 - Feed an array of problem links to the dialog
		CArray<CEmrProblemLink*, CEmrProblemLink*> apProblemLinks;
		if(eprtType == eprtEmrDataItem && nEMRDataID != -1) {
			//find the matching EMRDataID, if we have one			
			for(int i=0; i<m_pDetail->m_apEmrProblemLinks.GetSize(); i++) {
				CEmrProblemLink *pLink = m_pDetail->m_apEmrProblemLinks.GetAt(i);
				if(pLink != NULL && pLink->GetDataID() == nEMRDataID) {
					apProblemLinks.Add(pLink);
				}
			}
			dlg.LoadFromProblemList(GetParent(), &apProblemLinks);
		}
		else {
			dlg.LoadFromProblemList(GetParent(), &m_pDetail->m_apEmrProblemLinks);
		}
		dlg.DoModal();

	}NxCatchAll("Error in CEmrItemAdvDlg::EditProblem");
}

// (c.haag 2009-05-27 14:41) - PLID 34249 - This function is called when the user wants to link this EMR object
// with one or more existing problems
void CEmrItemAdvDlg::LinkProblems(EMRProblemRegardingTypes eprtType, long nEMRDataID /*= -1*/)
{
	// We require problem create permissions to create new links
	if (!CheckCurrentUserPermissions(bioEMRProblems, sptCreate)) {
		return;
	}

	CArray<CEmrProblem*,CEmrProblem*> aryAllProblems;
	CArray<CEmrProblem*,CEmrProblem*> aryEMRProblems;
	CArray<CEmrProblem*,CEmrProblem*> arySelectionsInMemory;
	CArray<long,long> arynSelectionsInData;
	int i;

	CEMRTopic *pTopic = m_pDetail->m_pParentTopic;
	CEMN* pEMN = (pTopic) ? pTopic->GetParentEMN() : NULL;
	CEMR* pEMR = (pEMN) ? pEMN->GetParentEMR() : NULL;
	ASSERT(NULL != pEMR);

	// Pull ALL problems, including deleted ones, because the chooser dialog will run a query
	// that must be filtered by both deleted and non-deleted problems in memory
	pEMR->GetAllProblems(aryAllProblems, TRUE);
	for (i=0; i < m_pDetail->m_apEmrProblemLinks.GetSize(); i++) {
		EnsureProblemInArray(aryEMRProblems, m_pDetail->m_apEmrProblemLinks[i]->GetProblem());
	}
	CEMRProblemChooserDlg dlg(aryAllProblems, aryEMRProblems, pEMR->GetPatientID(), this);
	if (dlg.HasProblems()) {
		if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
			// User chose at least one problem
			
			// First, go through the problems that already exist in memory (and owned
			// by this EMR), and create problem links for them.
			for (i=0; i < arySelectionsInMemory.GetSize(); i++) {
				CEmrProblemLink* pNewLink = new CEmrProblemLink(arySelectionsInMemory[i], -1, eprtType, m_pDetail->GetID(), nEMRDataID);
				pNewLink->UpdatePointersWithDetail(m_pDetail);
				m_pDetail->m_apEmrProblemLinks.Add(pNewLink);
			}
			// Next, go through the problems that exist in data and not in memory, create
			// problem objects for them, and then links for those. Here's the neat part:
			// If the problem is already in memory, but linked with nothing; then the EMR
			// is smart enough to just give you the problem already in memory when calling
			// the function to allocate it.
			// (z.manning 2009-05-27 12:52) - PLID 34297 - Added patient ID
			// (b.spivey November 11, 2013) - PLID 58677 - Add CodeID
			// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
			// (s.tullis 2015-02-23 15:44) - PLID 64723 
			// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
			if (arynSelectionsInData.GetSize() > 0) {
				_RecordsetPtr prs = CreateRecordset("SELECT EMRProblemsT.ID, Description, StatusID, "
					"EnteredDate, ModifiedDate, OnsetDate, DiagCodeID, DiagCodeID_ICD10, ChronicityID, EmrProblemActionID, PatientID, "
					"EMRProblemsT.CodeID, EmrProblemsT.DoNotShowOnCCDA, EmrProblemsT.DoNotShowOnProblemPrompt "
					"FROM EMRProblemsT "
					"WHERE Deleted = 0 AND ID IN (%s)", ArrayAsString(arynSelectionsInData));
				while (!prs->eof) {
					CEmrProblem* pNewProblem = pEMR->AllocateEmrProblem(prs->Fields);
					CEmrProblemLink* pNewLink = new CEmrProblemLink(pNewProblem, -1, eprtType, m_pDetail->GetID(), nEMRDataID);
					pNewLink->UpdatePointersWithDetail(m_pDetail);
					m_pDetail->m_apEmrProblemLinks.Add(pNewLink);
					pNewProblem->Release();
					prs->MoveNext();
				}
			}

			// Now update the interface
			const BOOL bEMNIsLocked = (pEMN->GetStatus() == 2) ? TRUE : FALSE;
			if(!bEMNIsLocked) {
				m_pDetail->SetUnsaved();
				m_pDetail->m_pParentTopic->SetUnsaved();
				GetParent()->SendMessage(NXM_EMR_ITEM_CHANGED, (WPARAM)m_pDetail);
			}
			// (c.haag 2009-05-30 14:25) - PLID 34249 - The EMR is the memory manager
			// of EMR problems in memory; and therefore must be flagged unsaved.
			pEMR->SetUnsaved();
			GetParent()->PostMessage(NXM_EMR_PROBLEM_CHANGED);

			// Make sure the problem label is on the screen if necessary
			CRect rWindow;
			GetWindowRect(rWindow);
			ReflectCurrentState();
			UpdateProblemStatusButtonAppearance();

			// Now resize the window so that the problem flag button can be seen
			// (c.haag 2006-12-13 08:59) - PLID 22052 - Do not do any repositioning if the EMN is locked
			// or else it will set the modified flag
			if(!bEMNIsLocked) {
				// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
				if(m_pDetail->m_pParentTopic->GetTopicWnd() && IsWindow(m_pDetail->m_pParentTopic->GetTopicWnd()->GetSafeHwnd())) {
					// (a.wetta 2007-02-23 14:46) - PLID 24511 - When updating an image we don't want it to resize itself, 
					// so tell the image to size to the detail, aka don't go back to the original dimensions
					// (a.walling 2011-05-25 17:57) - PLID 43847 - This is now no longer a property of the detail.
					//m_pDetail->SetSizeImageToDetail(TRUE);
					// (j.jones 2007-07-26 09:31) - PLID 24686 - renamed this function
					m_pDetail->m_pParentTopic->GetTopicWnd()->RepositionDetailsInTopicByInfoID(m_pDetail->m_nEMRInfoID, FALSE);
					//m_pDetail->SetSizeImageToDetail(FALSE);
				}
			}

		}  // if (IDOK == dlg.Open(arySelectionsInMemory, arynSelectionsInData)) {
		else {
			// User changed their mind
		}
	} else {
		// Dialog has no visible problems
		AfxMessageBox("There are no available problems to choose from.");
	}
}

//TES 3/23/2010 - PLID 37757 - This dialog doesn't maintain its own ReadOnly flag, so I changed the function name from
// SetReadOnly() to ReflectReadOnlyStatus()
void CEmrItemAdvDlg::ReflectReadOnlyStatus(BOOL bReadOnly)
{
	//TES 3/15/2010 - PLID 37757 - I see no reason this should maintain its own ReadOnly flag, it should just be checking the detail.
	//m_bReadOnly = bReadOnly;
	//TES 10/12/2004 - We still want them to be able to fix merge field conflicts.
	// (j.jones 2006-08-21 16:20) - PLID 21986 - we've since decided we won't allow this,
	// but let UpdateMergeStatusButtonAppearance() handle it
	if(m_pBtnMergeStatus && m_pBtnMergeStatus->GetSafeHwnd()) {
		UpdateMergeStatusButtonAppearance();
	}
}

void CEmrItemAdvDlg::SetGhostly(BOOL bGhostly)
{
	m_bGhostly = bGhostly;
}

HBRUSH CEmrItemAdvDlg::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		//DRT 5/13/2008 - PLID 29771 - Moved the review state handling code into the GetBackground* functions.
		if(nCtlColor == CTLCOLOR_DLG /*&& pWnd->GetSafeHwnd() == m_hWnd*/) {
			pDC->SetBkColor(GetBackgroundColor());
			return GetBackgroundBrush();
		}

		//DRT 4/24/2008 - PLID 29771 - The static label title should always be given the right background color
		if(nCtlColor == CTLCOLOR_STATIC) {
			pDC->SetBkColor(GetBackgroundColor());
			return GetBackgroundBrush();
		}
	} NxCatchAll(__FUNCTION__);

	return CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CEmrItemAdvDlg::SetCanEditContent(BOOL bCanEditContent)
{
	m_bCanEditContent = bCanEditContent;
}

void CEmrItemAdvDlg::SetIsTemplate(BOOL bIsTemplate)
{
	m_bIsTemplate = bIsTemplate;
}

// (c.haag 2008-01-22 09:34) - PLID 28686 - We now allow the caller to define special dialog behaviors
// (a.walling 2008-03-25 08:10) - PLID 28811 - This is now a static function. This is to prevent reentrancy problems due to UpdateItem which may very well
// destroy the CEmrItemAdvDlg that we are in. 
void CEmrItemAdvDlg::OpenItemEntryDlg(CEMNDetail* pDetail, BOOL bIsTemplate, EEmrItemEntryDlgBehavior behavior /*= eEmrItemEntryDlgBehavior_Normal */, LPVOID pBehaviorData /* = NULL */)
{
	// (c.haag 2006-04-04 11:06) - PLID 19890 - We consider permissions now
	if(!CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
		return;
	}

	// (a.walling 2009-06-30 14:00) - PLID 34759 - as a safeguard, ensure this is up to date before editing the item
	pDetail->RefreshIsActiveInfo();
	//TES 12/14/2006 - PLID 23833 - If they're editing an old version, we need to give them the option to edit the new version.
	if(!pDetail->IsActiveInfo()) {

		// (j.jones 2010-02-17 17:30) - PLID 37318 - if a detail is part of a SmartStamp image/table connection, you cannot "bring up to date"
		// (d.thompson 2010-08-30) - PLID 40107 - Fixed typo in msg box
		// (z.manning 2011-01-21 09:51) - PLID 42338 - Support multiple images per smart stamp table
		if(pDetail->GetSmartStampImageDetails()->GetCount() > 0 || pDetail->GetSmartStampTableDetail() != NULL) {
			AfxMessageBox("The detail you are editing is on an outdated version, and cannot be edited, because it has been changed since it was added to this EMN.\n"
				"The ability to bring an item up to date is not allowed on SmartStamp detail items. You will need to remove then re-add the SmartStamp image and table if you wish to use a newer version.");
			return;
		}

		// (j.jones 2010-11-12 08:56) - PLID 29075 - added a note that the topic will be immediately saved
		if(IDNO == MsgBox(MB_YESNO, "The detail you are editing is on an outdated version, and cannot be edited, because it has been changed since it was added to this EMN.  Would you like to update this detail with the most recent changes?\n"
			"If Yes, this detail will immediately have all the most recent changes applied to it, which cannot be undone. "
			"In addition, the topic will be saved immediately after bringing up to date.")) {
			return;
		}
		else {
			// (a.walling 2008-03-25 08:14) - PLID 28811 - Pass the detail pointer to the now static function
			UpdateItem(pDetail);
		}
	}

	CEmrItemEntryDlg dlg(NULL);
	if(pDetail->m_pParentTopic->GetParentEMN()) {
		dlg.SetCurrentEMN(pDetail->m_pParentTopic->GetParentEMN());
		dlg.SetDlgParentWnd(pDetail->m_pParentTopic->GetParentEMN()->GetInterface());
	}

	// (j.jones 2006-08-28 14:50) - PLID 22220 - if not a template, tell the dialog
	// we're editing from a given detail ID
	if(!pDetail->m_bIsTemplateDetail) {
		// (z.manning 2011-04-28 15:00) - PLID 37604 - We now pass in the detail pointer.
		dlg.m_pCalledFromDetail = pDetail;
	}

	dlg.m_bIsEditingOnEMN = !pDetail->m_bIsTemplateDetail;
	dlg.m_bIsEditingOnTemplate = pDetail->m_bIsTemplateDetail;

	// (z.manning, 08/29/05, PLID 17249)
	// By populating the current list, we tell the EMR item entry dlg to maintain
	// both the current and admin lists for single and multi selects.
	// List type of 1 corresponds to normal data list element (all other types are table-related).

	// (j.jones 2006-02-09 16:27) - only set this field if we are editing an existing list,
	// because the code will reload values from the admin list, not a current list
	if(pDetail->m_EMRInfoType == eitSingleList || pDetail->m_EMRInfoType == eitMultiList) {
		dlg.m_bMaintainCurrentList = TRUE;

		//TES 2/13/2006 - Track which ones are currently selected.
		CDWordArray dwaSelected;
		pDetail->GetSelectedValues(dwaSelected);
		// (c.haag 2006-03-03 12:57) - PLID 19557 - We should have as many resulting records as we
		// have list element ID's. Otherwise, the Current List in EmrItemEntryDlg and the detail on
		// the EMN will look different.
		int nDetailListElements = pDetail->GetListElementCount();
		_RecordsetPtr prs = CreateRecordset("SELECT Data, CASE WHEN ID IN (%s) THEN 1 ELSE 0 END AS Selected "
			"FROM EmrDataT WHERE ID IN (%s) AND ListType = 1", ArrayAsString(dwaSelected), pDetail->GetListElementIDsCommaDelimited());
		ASSERT(nDetailListElements == prs->GetRecordCount()); // Warn the developer if they don't match up
		while(!prs->eof) {
			CurrentListItem cli;
			cli.strData = AdoFldString(prs, "Data");
			cli.bSelected = AdoFldLong(prs, "Selected") ? true : false;
			cli.bSelectedOnTopic = cli.bSelected; // (a.walling 2008-02-07 10:58) - PLID 14982 - This will always match unless we are editing from a popped up detail
			dlg.m_aryCurrentList.Add(cli);
			prs->MoveNext();
		}
	} else if (pDetail->m_EMRInfoType == eitTable) {
		//
		// (c.haag 2006-03-06 16:46) - PLID 19580 - If we are opening the EmrItemEntryDlg for a
		// table item that's already in an open EMR, we need to get the state for every detail
		// in the entire EMR that has the same EmrInfoID as this item, and store them all in
		// an array that we pass into the EmrItemEntryDlg.
		//
		// The reason for this is that if a user wants to change the data type of a table column,
		// we need to check each existing detail to see if there is data in that column. If there
		// is, the user must be prevented from changing the column type.
		//
		CEMRTopic *pTopic = pDetail->m_pParentTopic;
		CEMN* pEMN = (pTopic) ? pTopic->GetParentEMN() : NULL;
		CEMR* pEMR = (pEMN) ? pEMN->GetParentEMR() : NULL;
		dlg.m_bMaintainCurrentTable = TRUE;

		// (j.jones 2010-02-17 11:53) - PLID 37318 - if a smartstamp table, disallow changing the data type
		// (z.manning 2011-01-21 09:51) - PLID 42338 - Support multiple images per smart stamp table
		if(pDetail->GetSmartStampImageDetails()->GetCount() > 0) {
			dlg.m_bPreventTypeChange = TRUE;
		}

		if (NULL != pEMR) {
			int nEMNs = pEMR->GetEMNCount();
			for (int i=0; i < nEMNs; i++) {
				pEMN = pEMR->GetEMN(i);
				int nTopics = pEMN->GetTopicCount();
				for (int j=0; j < nTopics; j++) {
					//
					// Go through each topic looking for details with the same EmrInfoID, and then add
					// them to dlg.m_apCurrentTableDetails. This function is recursive since topics
					// can have subtopics.
					//
					BuildCurrentTableStateArray(pEMN->GetTopic(j), pDetail->m_nEMRInfoID, dlg.m_apCurrentTableDetails);
				}
			}
		} else {
			ASSERT(FALSE);
		}		
	}
	//DRT 2/26/2008 - PLID 28603 - We need to pass in the current state of any images, so we can't do crazy things like delete them
	//	while they're in use.
	else if(pDetail->m_EMRInfoType == eitImage) {
		//Tell it we don't want any in-use hotspots deleted
		dlg.m_bMaintainCurrentImage = true;

		// (j.jones 2010-02-17 11:53) - PLID 37318 - if a smartstamp image, disallow changing the data type and smart stamp table
		if(pDetail->GetSmartStampTableDetail()) {
			dlg.m_bPreventTypeChange = TRUE;
			dlg.m_bPreventSmartStampTableChange = TRUE;
		}

		//Now send it all our details for this info ID.  Works the same as tables above recursively.
		CEMRTopic *pTopic = pDetail->m_pParentTopic;
		CEMN* pEMN = (pTopic) ? pTopic->GetParentEMN() : NULL;
		CEMR* pEMR = (pEMN) ? pEMN->GetParentEMR() : NULL;
		if (NULL != pEMR) {
			int nEMNs = pEMR->GetEMNCount();
			for (int i=0; i < nEMNs; i++) {
				pEMN = pEMR->GetEMN(i);
				int nTopics = pEMN->GetTopicCount();
				for (int j=0; j < nTopics; j++) {
					//
					// Go through each topic looking for details with the same EmrInfoID, and then add
					// them to dlg.m_aryCurrentImageDetails. This function is recursive since topics
					// can have subtopics.
					//
					BuildCurrentImageArray(pEMN->GetTopic(j), pDetail->m_nEMRInfoID, dlg.m_aryCurrentImageDetails);
				}
			}
		} else {
			ASSERT(FALSE);
		}
	}
	dlg.m_bIsCurrentDetailTemplate = bIsTemplate;

	//Remember the current InfoID, so we'll be able to tell if it changed.
	long nInfoID = pDetail->m_nEMRInfoID;
	// (c.haag 2008-01-22 11:23) - PLID 28686 - Include behavior information
	if(IDOK == dlg.OpenWithMasterID(pDetail->m_nEMRInfoMasterID, behavior, pBehaviorData)) {

		long nDataType = dlg.GetDataType();

		//RefreshContent on all copies of this EMRInfo item on the entire EMR, not just at a topic or EMN level

		// (j.jones 2006-08-17 15:53) - PLID 22078 - don't update from the dialog
		// unless the InfoID didn't change, or the detail is unsaved, or we are on a template
		if((nDataType == 2 || nDataType == 3) 
			&& (nInfoID == dlg.GetID() || pDetail->m_nEMRDetailID == -1 || bIsTemplate)) {

			// (j.jones 2006-03-29 13:11) - SetLabelText must be prior to the list being cleared out,
			// so we can properly remove the old label's list items

			// (c.haag 2006-03-31 14:47) - PLID 19387 - My code has probably made this redundant,
			// but I'll leave it in pending a more optimized implementation.

			pDetail->SetLabelText(dlg.GetLabelText());
			pDetail->m_strLongForm = dlg.GetLongForm();
			pDetail->m_nDataFormat = dlg.GetDataFormat();
			pDetail->m_strDataSeparator = dlg.GetDataSeparator();
			pDetail->m_strDataSeparatorFinal = dlg.GetDataSeparatorFinal();
			// (a.walling 2008-07-01 09:13) - PLID 30570 - Put this here for completeness
			// (z.manning 2010-04-15 15:53) - PLID 36224 - This shouldn't have ever been here because
			// we also save preview flags per detail.
			//pDetail->m_nPreviewFlags = dlg.GetPreviewFlags();

			if (dlg.m_bMaintainCurrentList) {
				dlg.GetCurrentList(
					&pDetail->m_CurList_naryDataIDs,
					&pDetail->m_CurList_straryData,
					&pDetail->m_CurList_straryLabels,
					&pDetail->m_CurList_straryLongForms,
					&pDetail->m_CurList_naryActionsType,
					&pDetail->m_CurList_baryInactive
					);
				pDetail->m_bCheckCurrentList = true;
			}
		}

		BOOL bForceSaveTopic = FALSE;

		// (j.jones 2006-08-17 15:26) - PLID 22078 - It is possible we edited an item that
		// exists on a saved EMN and a new item was created, so we must see if any items
		// should switch to use the new version of that item.
		// This function will update the info ID if the detail is unsaved OR if this is a template
		if(nInfoID != dlg.GetID()) {
				
			// (j.jones 2006-08-28 14:51) - PLID 22220 - also call UpdateInfoID for this detail only, if saved
			if(!pDetail->m_bIsTemplateDetail && pDetail->m_nEMRDetailID != -1) {
				//I realize it looks silly to call this for the detail, then pass in the detail ID, but I made
				//UpdateInfoID have a check to absolutely not run unless it is a template detail or unsaved detail,
				//so passing in the ID is a confirmation that we definitely want to update
				pDetail->UpdateInfoID(nInfoID, dlg.GetID(), dlg.GetChangedIDMap(), pDetail->m_nEMRDetailID);

				// (j.jones 2010-11-11 17:59) - PLID 29075 - force the topic to save
				bForceSaveTopic = TRUE;
			}

			pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->UpdateInfoID(nInfoID, dlg.GetID(), dlg.GetChangedIDMap());
		}

		// (c.haag 2006-03-31 11:13) - PLID 19387 - The TRUE parameter means that we want to call
		// SyncContentAndState at the end of LoadContent
		//
		// (j.jones 2006-08-17 15:52) - PLID 22078 - be sure to call RefreshContent on the EMRInfoID
		// now set in the EmrItemEntryDlg. It may be the same InfoID we started with, but if it's not,
		// it would mean we didn't update that InfoID anyways
		// (j.jones 2007-07-26 09:20) - PLID 24686 - converted to a ByInfoID function name
		pDetail->m_pParentTopic->GetParentEMN()->GetParentEMR()->RefreshContentByInfoID(dlg.GetID(), TRUE);

		// (c.haag 2006-04-04 13:42) - PLID 19883 - Now post a message to the main
		// frame to update the EMR details. We can't do it from here because if the
		// item type changed, we have to destroy this dialog. Even if we didn't, I
		// think it's generally safer to update details outside of the dialog of an
		// existing detail than anywhere else.
		//
		// The act of posting this message causes CMainFrame to later call
		// PostEmrItemAdvDlgItemSaved() in EmrUtils.cpp.
		//
		// The only risk here is if another EMR message is processed before 
		// NXM_EMRITEMADVDLG_ITEM_SAVED. There doesn't appear to be any reason for
		// it in the current implementation, but this should be documented nonetheless.
		//
		GetMainFrame()->PostMessage(NXM_EMRITEMADVDLG_ITEM_SAVED, (WPARAM)pDetail);

		// (j.jones 2010-11-11 17:26) - PLID 29075 - force the topic to save
		if(bForceSaveTopic) {
			CEmrTreeWnd* pTreeWnd = pDetail->GetParentEMN()->GetInterface();
			if(pTreeWnd) {
				pTreeWnd->SaveEMR(esotTopic, (long)pDetail->m_pParentTopic, TRUE);
			}
		}
	} else {
		// (a.walling 2009-06-30 16:39) - PLID 34759 - For safety's sake, ensure the IsActiveInfo properties are invalidated
		if (pDetail) {
			pDetail->RefreshIsActiveInfo();
		}
	}
}

// (b.cardillo 2012-05-02 20:28) - PLID 49255 - Added parameter
// Require the caller to indicate if he wants the official label text, or the text with any special 
// modifiers attached (right now the only modifier is an asterisk if it's a required detail that 
// hasn't been filled in)
CString CEmrItemAdvDlg::GetLabelText(BOOL bIncludeSpecialText)
{
	if(m_pDetail) {
		CString strRequiredFlag;
		if (bIncludeSpecialText) {
			// (b.cardillo 2012-05-02 20:28) - PLID 49255 - If allowed, prefix the name with an asterisk if it's required and not filled in
			if (m_pDetail->IsRequired() && !m_pDetail->IsStateSet()) {
				strRequiredFlag = _T("* ");
			}
		}
		CString strOverride = m_pDetail->GetMergeFieldName(FALSE);
		if(strOverride.IsEmpty()) {
			return strRequiredFlag + m_pDetail->GetLabelText();
		}
		else {
			return strRequiredFlag + strOverride;
		}
	}
	else {
		return "";
	}
}

void CEmrItemAdvDlg::SetMergeNameConflict(BOOL bConflicts)
{
	m_bMergeNameConflicts = bConflicts;
	CRect rWindow;
	GetWindowRect(rWindow);
	RepositionControls(CSize(rWindow.Width(),rWindow.Height()),FALSE);
}

// (j.jones 2008-08-15 14:43) - PLID 30779 - pass in a value determining if this is called during a save
BOOL CEmrItemAdvDlg::CheckForProblemDataChanges(BOOL bIsSaving /*= FALSE*/)
{
	BOOL bModified = FALSE;
	//
	// (c.haag 2006-11-14 15:10) - PLID 23158 - This function determines whether
	// any problem in the system has changed since last time it was called, and
	// if any has changed, the problem content is reloaded from data. Any changes
	// previously made to the problem will be overwritten.
	//
	// Returns true if the detail was changed
	//
	if (NULL == m_pDetail || m_pDetail->m_nEMRDetailID == -1) {
		return FALSE;
	}

	//
	// (c.haag 2006-11-14 15:23) - Make sure there is a valid problem ID
	//
	// (j.jones 2008-07-21 08:30) - PLID 30779 - use HasProblems()
	if(!m_pDetail->HasProblems()) {
		return FALSE;
	}
	
	// (c.haag 2006-11-14 15:16) - Do nothing if the data hasn't changed to our
	// knowledge
	if (!m_pProblemChecker->Changed()) {
		return FALSE;
	}

	try {
		// (c.haag 2006-11-14 15:18) - If we get here, we know that a problem has changed,
		// so pull the latest information from data and repopulate m_pDetail

		// (j.jones 2008-07-21 09:53) - PLID 30779 - now we support multiple problems

		// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
		// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
		// (c.haag 2009-05-12 12:25) - PLID 34234 - Problem linking table
		// (c.haag 2009-05-20 11:02) - PLID 34312 - Added EMRProblemLinkID and EmrProblemActionID
		// (z.manning 2009-05-27 10:11) - PLID 34297 - Added patient ID
		// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
		// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
		// (a.walling 2014-07-23 09:09) - PLID 63003 - Filter on EMRProblemsT.PatientID when possible
		// (s.tullis 2015-02-27 10:35) - PLID 64723 - Added DoNotShowOnCCDA
		// (r.gonet 2015-03-09 18:21) - PLID 65008 - Added DoNotShowOnProblemPrompt.
		_RecordsetPtr prs = CreateParamRecordset("SELECT EMRProblemsT.ID, Description, StatusID, EmrProblemLinkT.ID AS EmrProblemLinkID, "
			"EnteredDate, ModifiedDate, OnsetDate, EMRRegardingType, EMRRegardingID, EMRDataID, DiagCodeID, DiagCodeID_ICD10, ChronicityID, "
			"EmrProblemActionID, EmrProblemsT.PatientID, EMRProblemsT.CodeID, EMRProblemsT.DoNotShowOnCCDA, EMRProblemsT.DoNotShowOnProblemPrompt "
			"FROM EMRProblemsT "
			"INNER JOIN EMRProblemLinkT ON EMRProblemLinkT.EMRProblemID = EMRProblemsT.ID "
			"WHERE PatientID = {INT} AND ((EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT}) "
			"OR (EMRRegardingType = {CONST_INT} AND EMRRegardingID = {INT})) "
			"AND Deleted = 0", m_pDetail->GetPatientID(), eprtEmrItem, m_pDetail->m_nEMRDetailID, eprtEmrDataItem, m_pDetail->m_nEMRDetailID);
		while(!prs->eof) {
			
			long nEMRProblemLinkID = AdoFldLong(prs, "EMRProblemLinkID");
			EMRProblemRegardingTypes eprtTypeID = (EMRProblemRegardingTypes)AdoFldLong(prs, "EMRRegardingType");
			long nEMRRegardingID = AdoFldLong(prs, "EMRRegardingID", -1);
			long nEMRDataID = AdoFldLong(prs, "EMRDataID", -1);

			// (j.jones 2008-08-15 14:37) - PLID 30779 - See if it is in our list by ID,
			// but also track if we find a matching new, unsaved problem. Do not stop looping
			// if bFoundUnsaved is true, but DO stop looping if bFoundByID is true.
			// (c.haag 2009-05-20 11:03) - PLID 34312 - Apply this logic to problem links now
			BOOL bFoundByID = FALSE;
			BOOL bFoundUnsaved = FALSE;
			for(int i=0; i<m_pDetail->m_apEmrProblemLinks.GetSize() && !bFoundByID; i++) {

				CEmrProblemLink *pLink = m_pDetail->m_apEmrProblemLinks.GetAt(i);
				if (pLink->GetID() == nEMRProblemLinkID) {
					// if the problem has been modified at all, do not reload from data
					CEmrProblem* pProblem = pLink->GetProblem();
					if (!pProblem->m_bIsModified) {
						pProblem->ReloadFromData(prs->Fields);
						bModified = TRUE;
					}
					bFoundByID = TRUE;										
				}

				// (j.jones 2008-08-15 14:36) - PLID 30779 - If there is an unsaved problem,
				// we can sometimes have this function hit when we are saving, in which case
				// we will have to compare the description, status, and type, and track
				// if it matched. If so, it is almost assuredly the same problem.
				// (c.haag 2009-05-19 11:02) - PLID 34312 - Apply this logic to problem links
				// instead of problems
				else if(pLink->GetID() == -1 && bIsSaving) {

					//compare the key data, including an exact description comparison
					//(we can't check times, because they can be changed by the server,
					//and we can't check regarding ID, incase it was -1 as well)
					CEmrProblem* pProblem = pLink->GetProblem();
					FieldsPtr f = prs->Fields;
					// (j.jones 2014-02-24 15:44) - PLID 61010 - EMR problems now have ICD-9 and 10 IDs
					if(pProblem->m_strDescription == AdoFldString(f, "Description")
						&& pProblem->m_nStatusID == AdoFldLong(f, "StatusID")
						&& pProblem->m_nDiagICD9CodeID == AdoFldLong(f, "DiagCodeID", -1)
						&& pProblem->m_nDiagICD10CodeID == AdoFldLong(f, "DiagCodeID_ICD10", -1)
						&& pProblem->m_nChronicityID == AdoFldLong(f, "ChronicityID", -1)
						&& pLink->GetType() == (EMRProblemRegardingTypes)AdoFldLong(f, "EmrRegardingType")) {

						//If found, do nothing, it is PropagateNewIDs responsibility
						//to update the ID. Plus, we can't be certain that we *really*
						//found the right item yet.
						bFoundUnsaved = TRUE;							
					}
				}
			}

			//If we found by ID, we're set for sure, but if we didn't find by ID
			//but found a matching unsaved item, assume success. it would only fail
			//if someone else just added the same problem with the same description
			//while we were saving it as well, and we cannot reliably handle that on
			//the fly like this. We would, however, check this function again when loading
			//the problem list, so you would never know it failed.
			if(bFoundByID || bFoundUnsaved) {
				prs->MoveNext();
				continue;
			}

			//if we're still here, it wasn't in our list, so add it
			// (a.walling 2009-05-04 09:49) - PLID 28495 - Diag code
			// (a.walling 2009-05-04 09:50) - PLID 33751 - Chronicity
			CEMN *pEMN = m_pDetail->m_pParentTopic->GetParentEMN();
			if (NULL != pEMN) {
				CEMR* pEMR = pEMN->GetParentEMR();
				if (NULL != pEMR) {
					// Get the problem for the problem link
					CEmrProblem* pProblem = pEMR->AllocateEmrProblem(prs->Fields);
					// Create the problem link
					CEmrProblemLink* pNewLink = new CEmrProblemLink(pProblem, nEMRProblemLinkID, eprtTypeID, nEMRRegardingID, nEMRDataID);
					pNewLink->UpdatePointersWithDetail(m_pDetail);
					m_pDetail->m_apEmrProblemLinks.Add(pNewLink);
					// Release our local reference to the problem
					pProblem->Release();
				} else {
					ThrowNxException("CEmrItemAdvDlg::CheckForProblemDataChanges attempted to add a problem without a valid EMR");
				}
			} else {
				ThrowNxException("CEmrItemAdvDlg::CheckForProblemDataChanges attempted to add a problem without a valid EMN");
			}
			bModified = TRUE;

			prs->MoveNext();
		}
		prs->Close();

		if(bModified) {
			// (j.jones 2008-07-24 08:35) - PLID 30729 - change the EMR problem icon based on whether we have problems
			GetParent()->PostMessage(NXM_EMR_PROBLEM_CHANGED);
		}

	} NxCatchAll("Error updating problem detail information");

	// We should still return bModified, even on failure, in case we changed one or more
	// detail fields
	return bModified;
}

LRESULT CEmrItemAdvDlg::OnShowTopic(WPARAM wParam, LPARAM lParam)
{
	//DRT 8/21/2007 - PLID 27133 - Added try/catch
	try {
		// (c.haag 2006-11-14 15:10) - PLID 23158 - Ensure the problem-related content of m_pDetail 
		// is up to date. If it's not, we need to update the button appearance, too.
		if (CheckForProblemDataChanges()) {
			UpdateProblemStatusButtonAppearance();
		}
	} NxCatchAll("Error in OnShowTopic");
	return 0;
}

// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Determine the correct background color for each possible review state value.
COLORREF CEmrItemAdvDlg::GetReviewStateColor()
{
	if (m_pDetail) {
		if (m_pDetail->m_nEMRDetailID != -1) {
			return GetReconstructedEMRDetailReviewStateColor(m_pDetail->m_nReviewState);
		} else {
			// Not an actual detail, so the review state is meaningless
			ASSERT(m_pDetail->m_nReviewState == 0); // This just ASSERTs that the CEMRDetail class is doing it's job right and setting its m_nReviewState to 0 when it's not a real detail.
			return -1;
		}
	} else {
		// No detail to look at, this is strange, but obviously there's nothing to color
		ASSERT(FALSE);
		return -1;
	}
}

//DRT 6/4/2008 - PLID 30269 - This is the same structure as GetReviewStateColor, used to save memory in the background brush.
HBRUSH CEmrItemAdvDlg::GetReviewStateBrush()
{
	if (m_pDetail) {
		if (m_pDetail->m_nEMRDetailID != -1) {
			return GetReconstructedEMRDetailReviewStateBrush(m_pDetail->m_nReviewState);
		} else {
			// Not an actual detail, so the review state is meaningless
			ASSERT(m_pDetail->m_nReviewState == 0); // This just ASSERTs that the CEMRDetail class is doing it's job right and setting its m_nReviewState to 0 when it's not a real detail.
			return (HBRUSH)GetStockObject(NULL_BRUSH);
		}
	} else {
		// No detail to look at, this is strange, but obviously there's nothing to color
		ASSERT(FALSE);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}
}

// (a.walling 2008-03-25 08:13) - PLID 28811 - This is now a static function
void CEmrItemAdvDlg::UpdateItem(CEMNDetail* pDetail)
{
	BOOL bForceSaveTopic = FALSE;

	//First, figure out what the new InfoID is.
	long nOldInfoID = pDetail->m_nEMRInfoID;
	long nNewInfoID = VarLong(GetTableField("EmrInfoMasterT INNER JOIN EmrInfoT ON EmrInfoMasterT.ID = EmrInfoT.EmrInfoMasterID", "ActiveEmrInfoID", "EmrInfoT.ID", nOldInfoID));

	// (c.haag 2009-08-17 10:11) - PLID 35246 - This belongs in a transaction
	{
		CSqlTransaction sqlTrans("CEmrItemAdvDlg_UpdateItem");
		sqlTrans.Begin();

		//Now, set that in data (otherwise, the detail will re-set itself to the old info ID.
		if(pDetail->m_nEMRDetailID != -1) {
			ExecuteSql("UPDATE EmrDetailsT SET EmrInfoID = %li WHERE ID = %li", 
				nNewInfoID, pDetail->m_nEMRDetailID);

			// (j.jones 2010-11-11 17:26) - PLID 29075 - force the topic to save
			bForceSaveTopic = TRUE;
		}
		//Now, tell the detail to reload its content.
		pDetail->UpdateInfoID(nOldInfoID, nNewInfoID, NULL, pDetail->m_nEMRDetailID);

		// Commit the transaction
		sqlTrans.Commit();

		pDetail->SetNeedContentReload();
		pDetail->SetNeedSyncContentAndState();
		//Now, post a message that will call PostEmrItemEntryDlgSaved(), see Chris's note in OpenEmrItemEntryDlg()
		// for why we don't call it directly.
		GetMainFrame()->PostMessage(NXM_EMRITEMADVDLG_ITEM_SAVED, (WPARAM)pDetail);
		//Tell it that it's now up to date.
		pDetail->RefreshIsActiveInfo();

		// (j.jones 2010-11-11 17:26) - PLID 29075 - force the topic to save
		if(bForceSaveTopic) {
			// We used to call CEmrTreeWnd::SaveEMR here. However, that has all sorts of possible prompts
			// that allow the user to potentially cancel the save and if that happens then this detail
			// has bad data that may result in data being lost. Let's call the save function directly
			// to make it as likely as possible that this save happens to ensure data integrity. If the
			// EMN has other changes then it will still require a full save at some point and if it doesn't
			// have changes then we should be fine anyway.
			SaveEMRObject(esotTopic, (long)pDetail->m_pParentTopic, TRUE);
		}
	}
}

//DRT 4/24/2008 - PLID 29771 - Fill the background with the appropriate color
BOOL CEmrItemAdvDlg::OnEraseBkgnd(CDC* pDC)
{
	// (a.walling 2012-06-14 15:06) - PLID 51002 - Exclude the windowless controls when erasing the background
	CNxOleControlContainer* pNxCtrlCont = polymorphic_downcast<CNxOleControlContainer*>(GetControlContainer());

	BOOL bTouchedClipRegion = FALSE;

	if (pNxCtrlCont) {		
		bTouchedClipRegion = pNxCtrlCont->ExcludeWindowlessClipRegion(pDC);
	}

	//For proper coloring between "normal" and "needs reviewed" states, we're just doing it in OnCtlColor, but we need to call CWnd::OnEraseBkgnd, otherwise
	//	the NxDialog will color them white.

	CRect rcClient;
	GetClientRect(&rcClient);

	//pDC->FillSolidRect(rcClient, RGB(rand() % 255, rand() % 255, rand() % 255));
	//pDC->FillRect(rcClient, GetBackgroundBrush());
	::FillRect(*pDC, &rcClient, GetBackgroundBrush());

	if (bTouchedClipRegion) {
		pDC->SelectClipRgn(NULL);
	}

	return TRUE;
}

bool CEmrItemAdvDlg::IsHighColor()
{
	CClientDC dc(this);	
	return dc.GetDeviceCaps(BITSPIXEL) > 8;
}

COLORREF CEmrItemAdvDlg::GetRegularTextColor()
{
	return RGB(0, 0, 0);
}

COLORREF CEmrItemAdvDlg::GetGhostlyTextColor()
{
	if(IsHighColor())
		return EMR_GHOSTLY_16_BIT_GRAY;
	else
		return EMR_GHOSTLY_8_BIT_GRAY;
}

COLORREF CEmrItemAdvDlg::GetSpawnItemTextColor()
{
	if(m_bGhostly) {
		// (j.jones 2007-06-25 16:07) - PLID 26365 - use different colors
		// based on color depth of the screen
		if(IsHighColor())
			return EMR_GHOSTLY_16_BIT_RED;
		else
			return EMR_GHOSTLY_8_BIT_RED;
	}
	else {
		//make dark red

		// (j.jones 2007-06-25 16:07) - PLID 26365 - no need
		// to change here because it converts to maroon in 256 colors,
		// and the only other option is the red we use for Ghostly red
		return RGB(174,0,0);
	}
}

//DRT 5/13/2008 - PLID 29771 - These are designed to handle the background coloring needs of this dialog and all
//	derived elements.  Since there can be a "real" background color, and we also have to worry about the 
//	review state color, it is safest to do it in 1 function.
COLORREF CEmrItemAdvDlg::GetBackgroundColor()
{
	COLORREF clrReview = GetReviewStateColor();
	if(clrReview == -1) {
		return GetDefaultBackgroundColor();
	}
	else {
		return clrReview;
	}
}

HBRUSH CEmrItemAdvDlg::GetBackgroundBrush()
{
	COLORREF clrReview = GetReviewStateColor();
	if(clrReview == -1) {
		return GetDefaultBackgroundBrush();
	}
	else {
		//DRT 6/4/2008 - PLID 30269 - Just use our existing brushes
		return GetReviewStateBrush();
	}
}

HBRUSH CEmrItemAdvDlg::GetDefaultBackgroundBrush()
{
	// (a.walling 2012-06-14 15:44) - PLID 50719 - EmrColors
	return EmrColors::Item::Background();
}

COLORREF CEmrItemAdvDlg::GetDefaultParentBackgroundColor()
{
	// (a.walling 2012-06-14 15:44) - PLID 50719 - EmrColors
	return EmrColors::Topic::Background(!!m_bIsTemplate);
}

COLORREF CEmrItemAdvDlg::GetDefaultBackgroundColor()
{
	// (d.thompson 2011-05-10) - PLID 43123 - Allow the background color to be configurable
	//return (COLORREF)GetRemotePropertyInt("EMRItemColor", GetNxColor(GNC_EMR_ITEM_BG, 0), 0, GetCurrentUserName(), true);
	// (a.walling 2011-10-20 21:22) - PLID 46079 - Facelift - Panes
	// (a.walling 2012-06-14 15:44) - PLID 50719 - EmrColors
	return EmrColors::Item::Background();
}

COLORREF CEmrItemAdvDlg::GetBorderColor()
{
	return RGB(0x8e, 0xbf, 0xff);
}

void CEmrItemAdvDlg::HandleProblemChange(CEmrProblem *pChangedProblem)
{
	//TES 10/30/2008 - PLID 31269 - Just update our problem status button
	UpdateProblemStatusButtonAppearance();
}

// (a.walling 2011-11-11 11:11) - PLID 46624 - Emr item borders - drawing the border!
void CEmrItemAdvDlg::OnNcPaint()
{
	DWORD style = GetStyle();

	CWindowDC dcWin(this);

	CRect rectClient;
	GetClientRect(rectClient);

	CRect rectWindow;
	GetWindowRect(rectWindow);

	CRect rectBorder = rectWindow;

	ScreenToClient(rectWindow);

	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dcWin.ExcludeClipRect(rectClient);

	rectBorder.OffsetRect(-rectBorder.left, -rectBorder.top);
	
	COLORREF border = GetBorderColor();

	// (a.walling 2011-11-11 11:11) - PLID 46624 - this means we are in edit mode or remote, so no corners
	if (!NxGdi::IsRemote() && !(style & WS_THICKFRAME)) {
		COLORREF background = GetDefaultParentBackgroundColor();

		/*
		COLORREF avg2 = lerpColor(background, border, 0.60);
		COLORREF avg1 = lerpColor(background, border, 0.30);

		dcWin.SetPixelV(rectBorder.left, rectBorder.top, background);
		dcWin.SetPixelV(rectBorder.left + 1, rectBorder.top, avg1);
		dcWin.SetPixelV(rectBorder.left, rectBorder.top + 1, avg1);
		dcWin.SetPixelV(rectBorder.left + 1 + 1, rectBorder.top, avg2);
		dcWin.SetPixelV(rectBorder.left, rectBorder.top + 1 + 1, avg2);

		dcWin.SetPixelV(rectBorder.right - 1, rectBorder.top, background);
		dcWin.SetPixelV(rectBorder.right - 1 - 1, rectBorder.top, avg1);
		dcWin.SetPixelV(rectBorder.right - 1, rectBorder.top + 1, avg1);
		dcWin.SetPixelV(rectBorder.right - 1 - 1 - 1, rectBorder.top, avg2);
		dcWin.SetPixelV(rectBorder.right - 1, rectBorder.top + 1 + 1, avg2);

		dcWin.SetPixelV(rectBorder.left, rectBorder.bottom - 1, background);
		dcWin.SetPixelV(rectBorder.left + 1, rectBorder.bottom - 1, avg1);
		dcWin.SetPixelV(rectBorder.left, rectBorder.bottom - 1 - 1, avg1);
		dcWin.SetPixelV(rectBorder.left + 1 + 1, rectBorder.bottom - 1, avg2);
		dcWin.SetPixelV(rectBorder.left, rectBorder.bottom - 1 - 1 - 1, avg2);

		dcWin.SetPixelV(rectBorder.right - 1, rectBorder.bottom - 1, background);
		dcWin.SetPixelV(rectBorder.right - 1 - 1, rectBorder.bottom - 1, avg1);
		dcWin.SetPixelV(rectBorder.right - 1, rectBorder.bottom - 1 - 1, avg1);
		dcWin.SetPixelV(rectBorder.right - 1 - 1 - 1, rectBorder.bottom - 1, avg2);
		dcWin.SetPixelV(rectBorder.right - 1, rectBorder.bottom - 1 - 1 - 1, avg2);
		*/
		
		/*
		COLORREF avg1 = lerpColor(background, border, 0.20);

		// keep in mind that GDI accumulates several operations per thread and runs them
		// all at once when the buffer is full or gdiflush is called or other internal events, etc
		dcWin.SetPixelV(rectBorder.left, rectBorder.top, avg1);

		dcWin.SetPixelV(rectBorder.right - 1, rectBorder.top, avg1);

		dcWin.SetPixelV(rectBorder.left, rectBorder.bottom - 1, avg1);

		dcWin.SetPixelV(rectBorder.right - 1, rectBorder.bottom - 1, avg1);
		*/

		static CPen borderPen(PS_SOLID, 1, border);

		dcWin.SetBkColor(background);
		//dcWin.FillSolidRect(rectBorder, background);
		CGdiObject* pOldBrush = dcWin.SelectStockObject(NULL_BRUSH);
		CGdiObject* pOldPen = dcWin.SelectObject(&borderPen);
		dcWin.RoundRect(rectBorder, CPoint(4,4));
		dcWin.SelectObject(pOldPen);
		dcWin.SelectObject(pOldBrush);
	} else {

		dcWin.FillSolidRect(rectBorder, border);

		CGdiObject* pOldBrush = dcWin.SelectStockObject(NULL_BRUSH);
		CGdiObject* pOldPen = dcWin.SelectStockObject(BLACK_PEN);
		dcWin.Rectangle(rectBorder);
		dcWin.SelectObject(pOldPen);
		dcWin.SelectObject(pOldBrush);
	}

	dcWin.SelectClipRgn(NULL);
}

void CEmrItemAdvDlg::OnPaint()
{
	//DWORD style = GetStyle();

	//if (style & WS_THICKFRAME) {
	//	CWnd::OnPaint();
	//	return;
	//}

	CPaintDC dc(this);

	//if (dc.m_ps.fErase) {
	//	ASSERT(FALSE);
	//	dc.FillSolidRect(&dc.m_ps.rcPaint, GetBackgroundColor());
	//}

	if (PaintWindowlessControls(&dc)) {
		return;
	}

	Default();

	//CRect rectClient;
	//GetClientRect(rectClient);
	//
	//COLORREF clientBackground = GetDefaultBackgroundColor();

	//COLORREF parentBackground = GetDefaultParentBackgroundColor();

	//COLORREF avg = RGB(
	//	((GetRValue(parentBackground) + GetRValue(clientBackground)) / 2), 
	//	((GetGValue(parentBackground) + GetGValue(clientBackground)) / 2), 
	//	((GetBValue(parentBackground) + GetBValue(clientBackground)) / 2)
	//	);

	//dc.SetPixelV(rectClient.left, rectClient.top, avg);
	//dc.SetPixelV(rectClient.right - 1, rectClient.top, avg);
	//dc.SetPixelV(rectClient.left, rectClient.bottom - 1, avg);
	//dc.SetPixelV(rectClient.right - 1, rectClient.bottom - 1, avg);

	return;
}

LRESULT CEmrItemAdvDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2011-11-11 11:11) - PLID 46632 - WindowlessUtils - forward extra messages to windowless controls
	LRESULT lResult = 0;
	if (HandleExtraWindowlessMessages(m_pCtrlCont, message, wParam, lParam, &lResult)) {
		return lResult;
	}
	if (message == WM_SIZE || message == WM_MOVE) {
		lResult = CWnd::WindowProc(message, wParam, lParam);
		if (dynamic_ptr<CEmrTopicWnd> pTopicWnd = GetParent()) {
			pTopicWnd->NotifyItemPosChanged(this);
		}
		return lResult;
	}
	return CWnd::WindowProc(message, wParam, lParam);
}

CEmrFrameWnd* CEmrItemAdvDlg::GetEmrFrameWnd() const
{
	return polymorphic_downcast<CEmrFrameWnd*>(GetTopLevelFrame());
}

BOOL CEmrItemAdvDlg::PreTranslateMessage(MSG* pMsg)
{
	// (a.walling 2012-02-27 12:48) - PLID 48415 - Use __super to pass to base class, which at this time is CNxDialogMenuSupport<CWnd> to support CMFCPopupMenu
	// allow tooltip messages to be filtered
	if (__super::PreTranslateMessage(pMsg))
		return TRUE;
	
	// filter both messages to dialog and from children
	return PreTranslateInput(pMsg);
}

// (a.walling 2012-03-05 15:56) - PLID 46075 - Auto deletes when destroyed
void CEmrItemAdvDlg::PostNcDestroy()
{
	if (m_bAutoDelete) {
		delete this;
	}	
}

// (a.walling 2012-03-29 08:04) - PLID 49297 - These inlines make it a bit easier to check what sides are being used in a size operation
inline bool SizingLeftBorder(UINT hit) {
	return hit == HTLEFT || hit == HTTOPLEFT || hit == HTBOTTOMLEFT;
}

inline bool SizingRightBorder(UINT hit) {
	return hit == HTRIGHT || hit == HTTOPRIGHT || hit == HTBOTTOMRIGHT;
}

inline bool SizingTopBorder(UINT hit) {
	return hit == HTTOP || hit == HTTOPLEFT || hit == HTTOPRIGHT;
}

inline bool SizingBottomBorder(UINT hit) {
	return hit == HTBOTTOM || hit == HTBOTTOMLEFT || hit == HTBOTTOMRIGHT;
}

static DWORD GetMenuShowDelay()
{	
	static DWORD dwShowDelay = 0;
	if (0 == dwShowDelay) {
		SystemParametersInfo(SPI_GETMENUSHOWDELAY, 0, &dwShowDelay, 0);
		if (0 == dwShowDelay) {
			dwShowDelay = 200;
		}
	}

	return dwShowDelay;
}

namespace {

	struct MouseHoverParams
	{
		MouseHoverParams()
			: time(0)
			, width(0)
			, height(0)
		{
			if (!SystemParametersInfo(SPI_GETMOUSEHOVERTIME, 0, &time, 0)) {
				time = 400;
			}
			if (!SystemParametersInfo(SPI_GETMOUSEHOVERWIDTH, 0, &width, 0)) {
				width = 4;
			}
			if (!SystemParametersInfo(SPI_GETMOUSEHOVERHEIGHT, 0, &height, 0)) {
				height = 4;
			}

			time /= 2;
		}

		UINT time;
		UINT width;
		UINT height;
	};

	// (a.walling 2012-04-03 08:29) - PLID 49377 - timer to check hovering etc
	struct HoverTimer
	{
		HoverTimer()
			: m_pt(LONG_MIN, LONG_MIN)
			, m_trackedPt(LONG_MIN, LONG_MIN)
			, m_pWnd(NULL)
		{
		}

		WindowlessTimer::InstancePtr m_instance;
		CPoint m_pt;
		CPoint m_trackedPt;
		CWnd* m_pWnd;
		MouseHoverParams hoverParams;


		void OnMouseMove(CWnd* pWndHit, MSG& msg)
		{
			if (!m_instance) {
				return;
			}

			m_trackedPt = msg.pt;

			CPoint ptDiff(
				abs(m_trackedPt.x - m_pt.x)
				, abs(m_trackedPt.y - m_pt.y)
			);

			if ((UINT)ptDiff.x > hoverParams.width || (UINT)ptDiff.y > hoverParams.height) {
				Reset();
			}
		}

		void Reset()
		{
			m_pt.SetPoint(LONG_MIN, LONG_MIN);
			m_trackedPt = m_pt;
			m_pWnd = NULL;
			m_instance.reset();
		}

		bool IsTracking(CWnd* pWnd) const
		{
			return m_pWnd && (m_pWnd == pWnd);
		}

		void OnHover(CWnd* pTrackWnd, WindowlessTimer::CallbackFn callbackFn, UINT multiplier = 1)
		{
			Reset();
			m_pWnd = pTrackWnd;
			::GetMessagePos(&m_pt);
			m_trackedPt = m_pt;

			m_instance = WindowlessTimer::OnElapsed(
				hoverParams.time * multiplier, 
				boost::bind(&HoverTimer::DispatchOnce, this, callbackFn),
				false
			);
		}

		static void DebugTrace(LPCTSTR sz)
		{
			TRACE("HoverTimer - %s\n", sz);
		}

	protected:
		void DispatchOnce(WindowlessTimer::CallbackFn callbackFn)
		{
			try {
				callbackFn();
			} NxCatchAllIgnore();
			Reset();
		}
	};
}

namespace {
	struct TopicHoverTimer : public HoverTimer
	{
		TopicHoverTimer()			
			: m_pLastOverTopic(NULL)
			, m_nLastOverTopicTick(0)
		{
		}

		void OnHoverTreeWnd(CEmrTreeWnd* pTreeWnd)
		{
			pTreeWnd->OnDragHover(m_trackedPt);
		}

		void OnHoverAutoHideBar(CNxAutoHideBar* pAutoHideBar)
		{
			pAutoHideBar->OnDragHover(m_trackedPt);
		}

		void OnHoverTopicHeader(CEmrTopicHeaderWnd* pTopicHeaderWnd)
		{
			CEMRTopic* pTopic = pTopicHeaderWnd->GetTopic();
			if (!pTopic) {
				return;
			}
			// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
			if (pTopic->GetTopicWnd() && pTopic->GetTopicWnd()->IsWindowVisible()) {
				return;
			}

			pTopicHeaderWnd->GetEmrTopicView()->OnTopicHeaderClicked(pTopicHeaderWnd->GetTopic());
		}

		void OnHoverDisabledTopicHeader(CEmrTopicHeaderWnd* pTopicHeaderWnd)
		{
			CEMRTopic* pTopic = pTopicHeaderWnd->GetTopic();
			if (!pTopic) {
				return;
			}
			// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
			if (!pTopic->GetTopicWnd() || !pTopic->GetTopicWnd()->IsWindowVisible()) {
				return;
			}

			pTopicHeaderWnd->GetEmrTopicView()->OnTopicHeaderClicked(pTopicHeaderWnd->GetTopic());
		}

		CEMRTopic* m_pLastOverTopic;
		DWORD m_nLastOverTopicTick;
		void SetLastOverTopic(CEMRTopic* pTopic)
		{
			if (pTopic == m_pLastOverTopic) {
				return;
			}

			m_pLastOverTopic = pTopic;
			m_nLastOverTopicTick = GetTickCount();
		}
	};
	
	// (a.walling 2012-04-03 16:57) - PLID 49398 - Scrolls periodicially when within scroll margins
	struct TopicViewScroller
	{
		TopicViewScroller(CEmrTopicView* pTopicView)
			: m_pTopicView(pTopicView)
		{
		}

		bool IsScrolling()
		{
			if (m_instance) {
				return true;
			}

			return false;
		}

		CEmrTopicView* m_pTopicView;		
		WindowlessTimer::InstancePtr m_instance;
		
		void CheckScroll(BOOL bDoScroll)
		{
			CPoint pt;
			::GetMessagePos(&pt);

			if (!TryScrollTopicView(pt, bDoScroll)) {
				m_instance.reset();
				return;
			}

			if (!m_instance) {
				m_instance = WindowlessTimer::OnElapsed(90, boost::bind(&TopicViewScroller::CheckScroll, this, TRUE), false);
			}
		}

		// (a.walling 2012-04-03 16:57) - PLID 49398 - Check for scrolling
		bool TryScrollTopicView(CPoint pt, BOOL bDoScroll)
		{
			const long cnScrollMargin = 32;
			CRect rcTopicView;
			m_pTopicView->GetClientRect(&rcTopicView);
			CRect rcTopicViewScrollArea = rcTopicView;
			rcTopicView.DeflateRect(cnScrollMargin, cnScrollMargin);

			m_pTopicView->ScreenToClient(&pt);

			if (!rcTopicViewScrollArea.PtInRect(pt) || rcTopicView.PtInRect(pt)) {
				return false;
			}

			// alright, in the scrolling margins

			UINT nScrollCode = 0xFFFF;

			if (pt.x < rcTopicView.left) {
				nScrollCode = MAKEWORD(SB_LINEUP, HIBYTE(nScrollCode));
			} else if (pt.x >= rcTopicView.right) {
				nScrollCode = MAKEWORD(SB_LINEDOWN, HIBYTE(nScrollCode));
			}

			if (pt.y < rcTopicView.top) {
				nScrollCode = MAKEWORD(LOBYTE(nScrollCode), SB_LINEUP);
			} else if (pt.y >= rcTopicView.bottom) {
				nScrollCode = MAKEWORD(LOBYTE(nScrollCode), SB_LINEDOWN);
			}

			// do the actual scroll at first so scrolling can be accelerated by wiggling the mouse, same as most places in windows
			if (static_cast<CView*>(m_pTopicView)->OnScroll(nScrollCode, 0, bDoScroll)) {
				return true;
			} else {
				return false;
			}
		}
	};
}

// (a.walling 2012-03-29 08:04) - PLID 49297 - Custom handling of the SC_SIZE syscommand
void CEmrItemAdvDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// (a.walling 2012-03-29 08:04) - PLID 49297 - The 4 low bits of the nID in a WM_SYSCOMMAND message are used internally, so mask them out to get the real ID
	UINT nCommandID = nID & 0xFFF0;

	// (a.walling 2012-04-02 08:29) - PLID 49304 - Handle moving and sizing

	// (a.walling 2012-04-03 08:29) - PLID 49377 - timer to check hovering etc
	TopicHoverTimer hoverTimer;

	try {
		bool bIsTemplate = false;
		if (CEmrFrameWnd* pFrameWnd = GetEmrFrameWnd()) {
			bIsTemplate = pFrameWnd->IsTemplate();
		}

		// (a.walling 2012-04-02 08:29) - PLID 49304 - Ensure we don't clear out any temp CWnd* while in this loop
		AfxTempMapLock lockTempMaps;

		CWnd* pOriginalParent = GetParent();
		CEmrTopicWnd* pOriginalTopicWnd = dynamic_cast<CEmrTopicWnd*>(pOriginalParent);
		CEMRTopic* pOriginalTopic = pOriginalTopicWnd ? pOriginalTopicWnd->GetTopic() : NULL;
		CEmrTopicView* pTopicView = pOriginalTopicWnd ? pOriginalTopicWnd->GetEmrTopicView() : NULL;

		TopicViewScroller topicViewScroller(pTopicView);

		if (!pOriginalTopic || !pTopicView) {
			nCommandID = 0; // break out of the switch
		}

		// (a.walling 2012-03-29 13:48) - PLID 49297 - Filter out syscommands that don't make any sense
		switch (nCommandID) {
			case SC_SIZE:
			case SC_MOVE:
				if (GetStyle() & WS_THICKFRAME) {
					break;
				} else {
					return;
				}
			case SC_NEXTWINDOW:
			case SC_PREVWINDOW:
				__super::OnSysCommand(nID, lParam);
			default:
				return;
		}
		
		m_bPreventContentReload = TRUE;

		CPoint ptOriginal(lParam);
		CRect rcOriginal, rcOriginalChild;

		GetWindowRect(&rcOriginal);
		rcOriginalChild = rcOriginal;

		pOriginalParent->ScreenToClient(&rcOriginalChild);	

		CPoint ptOffset = ptOriginal - rcOriginal.TopLeft();

		// ensure the window is updated
		BringWindowToTop();
		RedrawWindow(NULL, 0, RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_FRAME);

		// (a.walling 2012-03-29 08:04) - PLID 49297 - Although not officially documented, the hit test code is packed into the 4 low bits of the nID
		UINT nHitTest = HTCAPTION;
		if (SC_SIZE == nCommandID) {
			nHitTest = nID & 0x000F;
			// the packed hit test code is simply offset by 9; this is how some code I ran across calculates it
			nHitTest += (HTLEFT - WMSZ_LEFT);
		}

		// clip the cursor, capture the mouse input, and send the WM_ENTERSIZEMOVE message
		ClipCursor(NULL);
		SetCapture();
		SendMessage(WM_ENTERSIZEMOVE);

		CSize szMinSize = 0;
		MINMAXINFO minMaxInfo = {0};
		SendMessage(WM_GETMINMAXINFO, (WPARAM)0, (LPARAM)&minMaxInfo);
		if (minMaxInfo.ptMinTrackSize.x > 0) {
			szMinSize.cx = minMaxInfo.ptMinTrackSize.x;
		}
		if (minMaxInfo.ptMinTrackSize.y > 0) {
			szMinSize.cy = minMaxInfo.ptMinTrackSize.y;
		}

		if (szMinSize.cx < 32) {
			szMinSize.cx = 32;
		}
		if (szMinSize.cy < 32) {
			szMinSize.cy = 32;
		}
		

		CRect rcLast = rcOriginal;
		CRect rcLastClient = rcLast;
		GetParent()->ScreenToClient(&rcLastClient);
		CRect rcMin = rcOriginal;
		

		if (SC_SIZE == nCommandID) {
			CEmrItemAdvDlg::g_pSizingDlg = this;
			if (SizingLeftBorder(nHitTest)) {
				rcMin.left = rcMin.right - szMinSize.cx;
			} else if (SizingRightBorder(nHitTest)) {
				rcMin.right = rcMin.left + szMinSize.cx;
			}

			if (SizingTopBorder(nHitTest)) {
				rcMin.top = rcMin.bottom - szMinSize.cy;
			} else if (SizingBottomBorder(nHitTest)) {
				rcMin.bottom = rcMin.top + szMinSize.cy;
			}
		}

		if (nCommandID == SC_MOVE) {
			// (a.walling 2012-04-02 08:29) - PLID 49304 - Set m_pMovingDlg
			CEmrItemAdvDlg::g_pMovingDlg = this;
		}

		MSG msg = {0};

		while (true) {
			if (msg.message == WM_MOUSEMOVE) {				
				if (!AfxGetApp()->PreTranslateMessage(&msg)) {
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
					msg.message = WM_NULL;
				}
			}

			// if we lost capture at some point, break out of the loop
			if (this != GetCapture()) {
				break;
			}

			if (!::IsWindow(pOriginalParent->GetSafeHwnd())) {
				break;
			}

			if (!::GetMessage(&msg, NULL, 0, 0)) {
				break;
			}

			bool bFiltered = false;
			// always call message filters in a modal loop, or morbo the code monster will destroy you
			if (::CallMsgFilter(&msg, /*MSGF_SIZE*/ 4)) { // the 4 is for MSGF_SIZE
				bFiltered = true;
				msg.message = WM_NULL;
			}

			// exit the loop if we get any keydown or lbuttonup messages
			if (msg.message == WM_LBUTTONUP || (msg.message == WM_KEYDOWN && msg.wParam != VK_SHIFT) ) {
				break;
			}

			if (bFiltered) {
				continue;
			}

			// pretranslate, then translate and dispatch;
			if (msg.message != WM_MOUSEMOVE) {
				if (!AfxGetApp()->PreTranslateMessage(&msg)) {
					::TranslateMessage(&msg);
					::DispatchMessage(&msg);
				}
				msg.message = WM_NULL;
				continue;
			}

			// at this point we are handling a WM_MOUSEMOVE message (or a special NXM_EMN_DETAIL_DRAG_BEGIN notification)

			//the MSG.pt is always in screen coords
			CPoint pt = msg.pt;
			
			// (a.walling 2012-04-02 08:29) - PLID 49304 - Lots of logic between the size and move operations are similar, so keep the actual operations within this block
			if (SC_SIZE == nCommandID) {
				int diffX = 0;
				int diffY = 0;

				diffX = msg.pt.x - ptOriginal.x;
				diffY = msg.pt.y - ptOriginal.y;

				if (diffX == 0 && diffY == 0) {
					continue;
				}

				CRect rcNew = rcOriginal;
				
				// adjust the new rect with the deltas
				if (SizingLeftBorder(nHitTest)) {
					rcNew.left += diffX;
				} else if (SizingRightBorder(nHitTest)) {
					rcNew.right += diffX;
				}

				if (SizingTopBorder(nHitTest)) {
					rcNew.top += diffY;
				} else if (SizingBottomBorder(nHitTest)) {
					rcNew.bottom += diffY;
				}

				// and constrain to the min rect we calculated
				if (rcNew.left > rcMin.left) {
					rcNew.left = rcMin.left;
				}
				if (rcNew.right < rcMin.right) {
					rcNew.right = rcMin.right;
				}
				if (rcNew.top > rcMin.top) {
					rcNew.top = rcMin.top;
				}
				if (rcNew.bottom < rcMin.bottom) {
					rcNew.bottom = rcMin.bottom;
				}

				// prepare and send the WM_SIZING message which might modify the new rect
				WPARAM nSizingHitTest = HTNOWHERE;
				if (nHitTest >= HTLEFT && nHitTest <= HTBOTTOMRIGHT) {
					nSizingHitTest = WMSZ_LEFT + (nHitTest - HTLEFT);
				}
				SendMessage(WM_SIZING, nSizingHitTest, (LPARAM)&rcNew);

				if (rcNew == rcLast) {
					continue;
				}

				rcLast = rcNew;

				// finally, map back to the parent's client coords and set the pos
				pOriginalParent->ScreenToClient(&rcNew);

				SetWindowPos(NULL, rcNew.left, rcNew.top, rcNew.Width(), rcNew.Height(), 0);
				GetParent()->RedrawWindow(NULL, 0, RDW_ERASENOW | RDW_UPDATENOW | RDW_ALLCHILDREN);
			} else if (SC_MOVE == nCommandID) {
				// (a.walling 2012-04-02 08:29) - PLID 49304 - Now we can handle the move, which is much more complex than the size, since we can traverse parents and etc

				// (a.walling 2012-04-03 16:57) - PLID 49398 - If we are scrolling, break out of here
				topicViewScroller.CheckScroll(FALSE);

				if (topicViewScroller.IsScrolling()) {
					hoverTimer.Reset();
					hoverTimer.SetLastOverTopic(NULL);
					continue;
				}

				CWnd* pWndTopic = NULL;
				CWnd* pWndHit = CWnd::WindowFromPoint(pt);

				hoverTimer.OnMouseMove(pWndHit, msg);

				// ensure we did not get a stale temp map entry
				if (pWndHit && !::IsWindow(pWndHit->GetSafeHwnd())) {
					pWndHit = NULL;
				}

				CWnd* pCurrentParent = GetParent();
				CEmrTopicWnd* pCurrentTopicWnd = dynamic_cast<CEmrTopicWnd*>(pCurrentParent);
				
				ASSERT(pCurrentParent == pCurrentTopicWnd);

				CEmrTopicWnd* pNewTopicWnd = pCurrentTopicWnd;
				CEmrTopicWnd* pHitTopicWnd = NULL;				

				// let's see if we are in the topic view
				if (pWndHit != pTopicView && !pTopicView->IsChild(pWndHit)) {
					// nope

					hoverTimer.SetLastOverTopic(NULL);

					// check the treewnd?
					CEmrTreeWnd* pTreeWnd = GetEmrFrameWnd()->GetEmrTreeWnd();
					if (pTreeWnd && pTreeWnd->IsChild(pWndHit)) {
						// it is in the tree!
						if (!hoverTimer.IsTracking(pWndHit)) {
							// show treewnd topic on hover
							hoverTimer.OnHover(pWndHit, boost::bind(&TopicHoverTimer::OnHoverTreeWnd, &hoverTimer, pTreeWnd));
						}
					} else {
						// check for autohidebar

						if (dynamic_ptr<CNxAutoHideBar> pAutoHideBar = pWndHit) {
							if (!hoverTimer.IsTracking(pAutoHideBar)) {
								// slide out autohide pane on hover
								hoverTimer.OnHover(pAutoHideBar, boost::bind(&TopicHoverTimer::OnHoverAutoHideBar, &hoverTimer, pAutoHideBar));
							}
						}
					}
				} else {
					CPoint ptInTopicView = pt;
					pTopicView->ScreenToClient(&ptInTopicView);

					pWndHit = pTopicView->ChildWindowFromPoint(ptInTopicView, CWP_SKIPINVISIBLE);
					CEmrTopicWnd* pTopicWnd = dynamic_cast<CEmrTopicWnd*>(pWndHit);

					// (a.walling 2012-04-03 08:20) - PLID 49377 - If a topic header, see if we need to start a hover timer
					if (!pTopicWnd) {
						CEmrTopicHeaderWnd* pTopicHeaderWnd = dynamic_cast<CEmrTopicHeaderWnd*>(pWndHit);

						if (pTopicHeaderWnd && pTopicHeaderWnd->GetTopic()->GetParentEMN() == pOriginalTopic->GetParentEMN()) {

							hoverTimer.SetLastOverTopic(pTopicHeaderWnd->GetTopic());
									
							if (!hoverTimer.IsTracking(pTopicHeaderWnd)) {
								// (a.walling 2012-06-22 14:01) - PLID 51150 - Explicitly get the topic wnd
								if ( (!bIsTemplate || pTopicHeaderWnd->GetTopic()->GetSourceActionID() == -1) && (!pTopicHeaderWnd->GetTopic()->GetTopicWnd() || !pTopicHeaderWnd->GetTopic()->GetTopicWnd()->IsWindowVisible()) ) {
									// show topic on hover
									hoverTimer.OnHover(pTopicHeaderWnd, boost::bind(&TopicHoverTimer::OnHoverTopicHeader, &hoverTimer, pTopicHeaderWnd), 2);
								} else {
									// hide topic on hover
									hoverTimer.OnHover(pTopicHeaderWnd, boost::bind(&TopicHoverTimer::OnHoverDisabledTopicHeader, &hoverTimer, pTopicHeaderWnd), 3);
								}
							}
						}
					} else {
						if (pTopicWnd->GetTopic() && pTopicWnd->GetTopic()->GetParentEMN() == pOriginalTopic->GetParentEMN()) {

							hoverTimer.SetLastOverTopic(pTopicWnd->GetTopic());

							// alright, same EMN -- check that it is not a spawned topic in a template
							if (!bIsTemplate || pTopicWnd == pCurrentTopicWnd || pTopicWnd->GetTopic()->GetSourceActionID() == -1) {
								// we are good to go then
								pNewTopicWnd = pTopicWnd;
								pHitTopicWnd = pNewTopicWnd;
							}
						}
					}
				}

				bool bChanged = false;
				bool bChangedParent = false;
				if (pHitTopicWnd && (pNewTopicWnd != pCurrentTopicWnd) && (pNewTopicWnd->GetTopic() == hoverTimer.m_pLastOverTopic) && ((GetTickCount() - hoverTimer.m_nLastOverTopicTick) >= hoverTimer.hoverParams.time) ) {
					// move to new topic wnd

					// now move the detail to the new topic.
					pCurrentTopicWnd->GetTopic()->DetachDetail(m_pDetail, FALSE); // (a.walling 2009-10-13 14:48) - PLID 36024 - don't release ref

					this->SetParent(pNewTopicWnd);

					pNewTopicWnd->GetTopic()->AddDetail(m_pDetail, FALSE, FALSE);

					// (a.walling 2007-12-17 16:11) - PLID 28391 - Refresh the HTML visibility states of the topics now
					pCurrentTopicWnd->GetTopic()->RefreshHTMLVisibility();
					pNewTopicWnd->GetTopic()->RefreshHTMLVisibility();

					//we track per EMN if any detail has moved, to make saving faster
					pCurrentTopicWnd->GetTopic()->GetParentEMN()->SetDetailsHaveMoved(TRUE);

					bChanged = true;
					bChangedParent = true;
				}
				
				CPoint ptNewTopLeft = pt - ptOffset;
				CRect rcNew(ptNewTopLeft, rcLast.Size());
				
				// this should constrain it to the proper dimensions at least
				SendMessage(WM_MOVING, (WPARAM)0, (LPARAM)&rcNew);

				if (bChanged) {						
					pNewTopicWnd->GetTopic()->SetUnsaved();
					pCurrentTopicWnd->GetTopic()->SetUnsaved();

					GetEmrFrameWnd()->GetEmrTreeWnd()->PostMessage(NXM_TOPIC_MODIFIED_CHANGED, (WPARAM)pNewTopicWnd->GetTopic(), 0);
					GetEmrFrameWnd()->GetEmrTreeWnd()->PostMessage(NXM_TOPIC_MODIFIED_CHANGED, (WPARAM)pCurrentTopicWnd->GetTopic(), 0);
				}

				rcLast = rcNew;
				CRect rcNewClient = rcNew;

				CWnd* pParent = GetParent();

				pParent->ScreenToClient(&rcNewClient);

				if (bChanged || (rcLastClient != rcNewClient)) {
					m_pDetail->SetUnsaved();
					bChanged = true;
				}
				rcLastClient = rcNewClient;

				if (bChanged) {
					GetParent()->ScreenToClient(&rcNew);
					SetWindowPos(NULL, rcNew.left, rcNew.top, rcNew.Width(), rcNew.Height(), 0);
					GetParent()->RedrawWindow(NULL, 0, RDW_ERASENOW | RDW_UPDATENOW | RDW_ALLCHILDREN);
					if (bChangedParent) {
						RedrawWindow(NULL, 0, RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_FRAME);
					}
				}

				//if (pNewTopicWnd != pCurrentTopicWnd) {
				//	pTopicView->ForceRefreshTopicWindowPositions();
				//}
			}
		}

		m_bPreventContentReload = FALSE;
		CEmrItemAdvDlg::g_pMovingDlg = NULL;
		CEmrItemAdvDlg::g_pSizingDlg = NULL;

		// now notify that the modal loop has exited and release capture
		SendMessage(WM_EXITSIZEMOVE);
		if (this == GetCapture()) {
			ReleaseCapture();
		}
		
		// finally notify the parent
		GetParent()->SendMessage(NXM_EMN_DETAIL_DRAG_END, (WPARAM)this, (LPARAM)0);

		if (SC_MOVE == nCommandID) {
			if (dynamic_ptr<CEmrTopicWnd> pFinalParentTopicWnd = GetParent()) {
				if (pTopicView->GetActiveTopicWnd() != pFinalParentTopicWnd->shared_from_this()) {
					pTopicView->SetActiveTopicWnd(pFinalParentTopicWnd->shared_from_this());
					//pTopicView->EnsureTopicInView(pFinalParentTopicWnd->GetTopic());
				}
			}
		}
		pTopicView->ForceRefreshTopicWindowPositions();

		if (msg.message != WM_NULL) {				
			if (!AfxGetApp()->PreTranslateMessage(&msg)) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
				msg.message = WM_NULL;
			}
		}

	} NxCatchAll(__FUNCTION__);
	
	m_bPreventContentReload = FALSE;
	CEmrItemAdvDlg::g_pMovingDlg = NULL;
	CEmrItemAdvDlg::g_pSizingDlg = NULL;
}

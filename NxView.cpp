// NxView.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "PracticeDoc.h"
#include "NxView.h"
#include "MainFrm.h"
#include "GlobalUtils.h"
#include "DocBar.h"
#include "SchedulerView.h"
#include "ScheduledPatientsDlg.h"
#include "PreferenceUtils.h"
#include "MoveUpListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNxView

IMPLEMENT_DYNCREATE(CNxView, CView)

CNxView::CNxView()
	: m_brBackground(CNxDialog::m_sbrBackground)
{
	m_bContactsToolBar	= false;
	m_bPatientsToolBar	= false;
	m_bDateToolBar		= false;
	m_bMarketToolBar	= false;
	m_bCheckSecurityWhenActivated = TRUE;
	m_bIsExplicitlyClosing = FALSE;
	m_bPreviousActiveState = FALSE;
	GetMainFrame()->AddView(this);
	m_dtLastSecurityCheck.SetStatus(COleDateTime::invalid);

	// (a.walling 2008-05-22 11:45) - PLID 27648 - Initialize the brush with default dialog background color
	// (a.walling 2008-06-05 09:50) - PLID 30289 - Use the static shared brush
	if (CNxDialog::m_sbrBackground.GetSafeHandle() == NULL) {
		CNxDialog::m_sbrBackground.CreateSolidBrush(CNxDialog::GetSolidBackgroundColor());
	}
}

CNxView::~CNxView()
{
	GetMainFrame()->RemoveView(this);
}


BEGIN_MESSAGE_MAP(CNxView, CView)
	//{{AFX_MSG_MAP(CNxView)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNxView drawing

void CNxView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CNxView diagnostics

#ifdef _DEBUG
void CNxView::AssertValid() const
{
	CView::AssertValid();
}

void CNxView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CPracticeDoc* CNxView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPracticeDoc)));
	return (CPracticeDoc*)m_pDocument;
}
#endif //_DEBUG
/////////////////////////////////////////////////////////////////////////////
// CNxView message handlers

// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
void CNxView::UpdateView(bool bForceRefresh)
{
}

void CNxView::ShowToolBars()
{
	CMainFrame *pMain = GetMainFrame();

	//hide first, then show, otherwise wierd positioning happens
	if (!m_bPatientsToolBar)
		pMain->ShowPatientBar(false);
	if (!m_bContactsToolBar)
		pMain->ShowContactBar(false);
	if (!m_bMarketToolBar)
		pMain->ShowDoctorBar(false);
//	if (!m_bDateToolBar)
//		pMain->ShowDateBar(false);

	if (m_bPatientsToolBar)
		pMain->ShowPatientBar(true);
	if (m_bContactsToolBar)
		pMain->ShowContactBar(true);
	if (m_bMarketToolBar)
		pMain->ShowDoctorBar(true);
	
//	if (m_bDateToolBar)
//		pMain->ShowDateBar(true);

	GetMainFrame()->RecalcLayout();
}

BOOL CNxView::CheckPermissions()
{
	// (c.haag 2003-10-02 16:28) - We should never actually get here
	// becauase it means we never wrote a CheckPermissions() function
	// for this view.
	ASSERT(FALSE);
	return 1;
}

static HWND GetMoveUpDlg()
{
	CSchedulerView* pView = (CSchedulerView*)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
	if (!pView)	return NULL;
	return pView->m_MoveUpDlg.GetSafeHwnd();
}

static HWND GetScheduledPatientsDlg()
{
	CSchedulerView* pView = (CSchedulerView*)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
	if (!pView)	return NULL;
	if (!pView->m_pdlgScheduledPatients) return NULL;
	return pView->m_pdlgScheduledPatients->GetSafeHwnd();
}

void CNxView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	if(bActivate)
		GetMainFrame()->UpdateOpenModuleList(this);

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	// (c.haag 2003-10-03 09:41) - Disregard everything if we are closing as
	// a result of posting a NXM_EXPLICIT_DESTROY_VIEW to the mainframe.
	if (!m_bIsExplicitlyClosing)
	{
		if (bActivate)
		{
			// (c.haag 2003-10-01 17:51) - If we are active now, and we weren't active
			// prior to this function call, lets do stuff.
			if (!m_bPreviousActiveState)
			{
				// (c.haag 2003-10-01 17:51) - Check to see if we need to check
				// permissions. This allows us to make sure we check only when
				// we have to.
				if (m_bCheckSecurityWhenActivated)
				{
					// (c.haag 2003-10-03 09:42) - Figure out how long it has been
					// since our last security check.
					COleDateTimeSpan dtsElapsed;
					if (m_dtLastSecurityCheck.GetStatus() == COleDateTime::invalid)
						dtsElapsed.SetStatus(COleDateTimeSpan::invalid);
					else
						dtsElapsed = COleDateTime::GetCurrentTime() - m_dtLastSecurityCheck;
					m_bCheckSecurityWhenActivated = FALSE;

					// (c.haag 2003-10-03 09:26) - Don't do any checking if the mainframe did
					// the checking for us.
					if (!GetMainFrame()->m_bAlreadyCheckedModulePermissions)
					{
						BOOL bCheckPermissions = FALSE;

						// (c.haag 2003-10-01 17:53) - We have to do this additional
						// check. The reason is that when the frame window becomes
						// inactive, it doesn't keep track of what view was active
						// while the frame was active. We need to track that manually
						// so we know what view to not ask the user permission for.
						
						if (pFrame->m_pLastActivatedView != this &&
							(dtsElapsed.GetStatus() == COleDateTimeSpan::invalid || dtsElapsed.GetTotalSeconds()/60 > GetRemotePropertyInt("ActiveViewPWExpireMinutes", 5, 0, "<None>", true)))
						{
							// This is true if another view had focus
							bCheckPermissions = TRUE;

						}
						else if (dtsElapsed.GetStatus() == COleDateTimeSpan::invalid || dtsElapsed.GetTotalSeconds()/60 > GetRemotePropertyInt("ActiveAppPWExpireMinutes", 5, 0, "<None>", true))
						{
							// (c.haag 2003-11-13 10:31) - If the todo alarm, messenger dialog
							// or move up list are active, don't force a permission check!
							CWnd* pWnd = CWnd::GetActiveWindow();

							// (a.walling 2007-05-04 09:58) - PLID 4850 - MessagerDlg is now a pointer
							if (pWnd && pWnd->GetSafeHwnd() != GetMainFrame()->m_dlgToDoAlarm.GetSafeHwnd() &&
								GetMainFrame()->m_pdlgMessager != NULL &&
								pWnd->GetSafeHwnd() != GetMainFrame()->m_pdlgMessager->GetSafeHwnd() &&
								pWnd->GetSafeHwnd() != GetMainFrame()->m_dlgAlert.GetSafeHwnd() &&
								pWnd->GetSafeHwnd() != GetMoveUpDlg() &&
								pWnd->GetSafeHwnd() != GetScheduledPatientsDlg())
							{
								// This is true if another application had focus
								bCheckPermissions = TRUE;
							}
						}
						
						if (bCheckPermissions)
						{
							if (!CheckPermissions())
							{
								// Close the module
								MsgBox("You do not have authorization to use this module. This module will now close.");
								// (c.haag 2003-10-03 09:36) - We need this to be set because this window can be
								// activated after NXM_EXPLICIT_DESTROY_VIEW is posted.
								m_bIsExplicitlyClosing = TRUE;
								pFrame->PostMessage(NXM_EXPLICIT_DESTROY_VIEW, 0, (LPARAM)this);
							}
						}
					}

					// Remember when we did this
					m_dtLastSecurityCheck = COleDateTime::GetCurrentTime();
				}
			}
			// (c.haag 2003-10-01 17:57) - Tell the mainframe that we are
			// the currently active view in case the app is inactivated later on.
			pFrame->m_pLastActivatedView = this;
		}
		else
		{
			// (c.haag 2003-10-01 17:57) - If we used to be active, and we're not
			// active anymore, we need the user to enter the password to activate
			// us again. There is one exception to this rule: And that is if the
			// app itself becomes inactive.
			if (m_bPreviousActiveState)
			{
				m_bCheckSecurityWhenActivated = TRUE;
			}
		}
	}
	m_bPreviousActiveState = bActivate;

	if (bActivate)
	{	
		ShowToolBars();
		//TES 2/11/2004: This is now handled (more correctly) by CChildFrame::OnMDIActivate()
		/*if (pActivateView != pDeactiveView)
			UpdateView();//Added by BVB on 11/3/99*/
	}

	// (a.walling 2011-08-04 14:36) - PLID 44788 - Notifies active dialog when the parent view is activated/deactivated
	CNxDialog* pActiveSheet = GetActiveSheet();
	if (pActiveSheet) {
		pActiveSheet->OnParentViewActivate(bActivate, pActivateView, pDeactiveView);
	}

	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CNxView::ResetSecurityTimer()
{
	m_dtLastSecurityCheck = COleDateTime::GetCurrentTime();
}

int CNxView::ShowPrefsDlg()
{
	// (c.haag 2004-03-16 09:56) PLID 11371 - By default, we will open the preference
	// dialog to the properties of the current module. Individual modules may overload
	// this function and open the preferences dialog on a per-tab basis.
	CString strModuleName = GetCurrentModuleName();
	if (strModuleName == SCHEDULER_MODULE_NAME)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piSchedulerModule);
	else if (strModuleName == CONTACT_MODULE_NAME)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piContactsModule);
	else if (strModuleName == LETTER_MODULE_NAME)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piLetterWModule);
	else if (strModuleName == MARKET_MODULE_NAME)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piMarketingModule);
	else if (strModuleName == INVENTORY_MODULE_NAME)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piInventoryModule);
	else if (strModuleName == FINANCIAL_MODULE_NAME)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piFinancialModule);
	else if (strModuleName == REPORT_MODULE_NAME)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piReportsModule);
	else if (strModuleName == ADMIN_MODULE_NAME)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piAdminModule);
	else if (strModuleName == SURGERY_CENTER_MODULE_NAME)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piAscModule);
	// (d.thompson 2009-11-16) - PLID 36134
	else if (strModuleName == LINKS_MODULE_NAME)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piLinksModule);

	else
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase());
}

BOOL CNxView::OnEraseBkgnd(CDC* pDC) 
{
	// As an NxView we expect to be entirely covered by a dialog, so we don't want to 
	// erase our background because the dialog will erase ITS background and that will 
	// cover everything.
	// (b.cardillo 2004-03-22 14:13) - PLID 11519 - If we were to erase our own background 
	// it would cause a flicker because our erase would be immediately followed by the 
	// dialog's erase.  This was happening in letter writing and reports, since these are 
	// the only modules right now that use CNxView directly (most use CNxTabView which 
	// already overrides this handler and doesn't erase the background).
	return TRUE;
}

// (a.walling 2008-05-22 11:45) - PLID 27648 - Need to handle WM_CTLCOLOR messages for 'dialog' backgrounds.
HBRUSH CNxView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	switch (nCtlColor)
	{	case CTLCOLOR_DLG:
		//make anything (STATIC text) with WS_EX_TRANSPARENT
		//appear transparent without subclassing it
		return (HBRUSH)m_brBackground;
		break;
	}
	
	return CView::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CNxView::PreCreateWindow( CREATESTRUCT& cs )
{
	BOOL bRet = CView::PreCreateWindow(cs);
	
	// (a.walling 2012-03-02 10:43) - PLID 48589 - Remove the client edge style to get rid of borders
	if (cs.dwExStyle & WS_EX_CLIENTEDGE) {
		cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	}

	return bRet;
}

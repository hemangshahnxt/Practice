// ChildFrm.cpp : implementation of the CChildFrame class
//

#include "stdafx.h"
#include "Practice.h"
#include "NxView.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "nxmessagedef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CChildFrame)
	ON_WM_CLOSE()
	ON_WM_SETFOCUS()
	ON_WM_ERASEBKGND()
	ON_WM_MDIACTIVATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	m_strChildType = "";
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CMDIFrameWnd* pParentWnd, CCreateContext* pContext)
{
	// Get the type now, because we need to use it pretty much immediately
	ASSERT(pContext && pContext->m_pNewDocTemplate && pContext->m_pNewDocTemplate->IsKindOf(RUNTIME_CLASS(CMultiDocTemplate)));
	if (pContext && pContext->m_pNewDocTemplate && pContext->m_pNewDocTemplate->IsKindOf(RUNTIME_CLASS(CMultiDocTemplate))) {
		pContext->m_pNewDocTemplate->GetDocString(m_strChildType, CDocTemplate::docName);

		// (a.walling 2010-01-27 16:59) - PLID 37089 - Set the title of this child frame
		SetTitle(m_strChildType);
	}

	return CMDIChildWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, pContext);
}

//BOOL CChildFrame::OnCreateClient( LPCREATESTRUCT /*lpcs*/,
//	CCreateContext* pContext)
//{ 
//	return m_wndSplitter.Create( this,
//		2, 2,                 // TODO: adjust the number of rows, columns
//		CSize( 10, 10 ),      // TODO: adjust the minimum pane size
//		pContext );
//} 

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
		| FWS_ADDTOTITLE | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE;

	BOOL bRet = CMDIChildWnd::PreCreateWindow(cs);

	// (a.walling 2012-03-02 10:43) - PLID 48589 - Remove the client edge style to get rid of borders
	if (cs.dwExStyle & WS_EX_CLIENTEDGE) {
		cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	}

	return bRet;
}

/////////////////////////////////////////////////////////////////////////////
// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}
					
void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChildFrame message handlers

void CChildFrame::OnClose() 
{
	if (GetActiveView()->SendMessage(NXM_ALLOW_CLOSE) != AC_CANNOT_CLOSE) {
		CMDIChildWnd::OnClose();
	}
}

bool CChildFrame::IsOfType(LPCTSTR strModuleName)
{
	if(!this) return false;						//RC - because of the use of MDI frame in ReportViewer
	if(!m_strChildType) return false; 
	if (!strcmp(strModuleName, m_strChildType)){
		return true;
	} else {
		return false;
	}
}

void CChildFrame::SetType(LPCTSTR strModuleName)
{
	// We've added code that should detect and set this variable on creation of the window, so this 
	// function is obsolete.  Instead of removing all calls to this function though, I'm adding an 
	// ASSERT to confirm that it's never being called with the incorrect value.
	ASSERT(m_strChildType == strModuleName);
	m_strChildType = strModuleName;

	// (a.walling 2010-01-27 16:59) - PLID 37089 - Set the title of this child frame
	SetTitle(m_strChildType);
}

const CString & CChildFrame::GetType()
{
	return m_strChildType;
}

BOOL CChildFrame::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd *pActivateWnd, CWnd *pDeactivateWnd)
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	if(bActivate && pActivateWnd != pDeactivateWnd) {
		if(GetActiveView() && GetActiveView()->IsKindOf(RUNTIME_CLASS(CNxView))) {
			//(a.wilson 2011-10-5) PLID 38789 - If we are dealing with the marketing module,
			//we do not want to force refresh because it cause Practice to lock up for X minutes
			//depending on the size of the database.
			bool bRefreshModule = true;

			if(this->IsOfType(MARKET_MODULE_NAME))	//if marketing module, do not refresh.
			{
				bRefreshModule = false;
			}
			((CNxView*)GetActiveView())->UpdateView(bRefreshModule);
		}
	}
}

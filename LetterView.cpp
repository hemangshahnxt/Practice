// AdminView.cpp : implementation file
//

#include "stdafx.h"
#include "LetterView.h"
#include "GlobalUtils.h"
#include "nxmessagedef.h"
#include "Groups.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLetterView

IMPLEMENT_DYNCREATE(CLetterView, CNxView)

CLetterView::CLetterView()
	: m_frame(this)
{
	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Letter_Writing_Module/overview_of_the_letter_writing_module.htm";
	m_bPatientsToolBar = true;
	m_bContactsToolBar = false;
}

CLetterView::~CLetterView()
{
}


BEGIN_MESSAGE_MAP(CLetterView, CNxView)
	//{{AFX_MSG_MAP(CLetterView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_MERGE_ALL_FIELDS, OnMergeAllFields)
	ON_COMMAND(IDM_UPDATE_VIEW, OnUpdateView)
	ON_UPDATE_COMMAND_UI(ID_MERGE_ALL_FIELDS, OnUpdateMergeAllFields)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLetterView drawing

void CLetterView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CLetterView diagnostics

#ifdef _DEBUG
void CLetterView::AssertValid() const
{
	CNxView::AssertValid();
}

void CLetterView::Dump(CDumpContext& dc) const
{
	CNxView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLetterView message handlers

BOOL CLetterView::CheckPermissions()
{
	if(!g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrUse))
		return FALSE;

	if (!UserPermission(LetterWritingModuleItem)) {
		return FALSE;
	}
	return TRUE;
}

int CLetterView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxView::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_frame.Create(IDD_LETTERS, this))
		AfxMessageBox ("Failed!");
	m_frame.ShowWindow(SW_SHOW);

//	Reportdb.Open(GetPracPath(true) + "\\Reports.mdb");

	return 0;
}

void CLetterView::OnSize(UINT nType, int cx, int cy) 
{
	CNxView::OnSize(nType, cx, cy);
	
	RECT rect;

	GetParent()->GetClientRect (&rect);
	m_frame.MoveWindow(&rect);
}

void CLetterView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
//	m_frame.UpdateToolBars(bActivate & 1);
	CNxView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

LRESULT CLetterView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
	case NXM_ALLOW_CLOSE:
		return m_frame.CheckAllowClose();
		break;
	case NXM_PRE_CLOSE:
		m_frame.PreClose();
	}
	return CNxView::WindowProc(message, wParam, lParam);
}

bool g_bMergeAllFields = false;

void CLetterView::OnMergeAllFields() 
{
	if (g_bMergeAllFields) {
		g_bMergeAllFields = false;
	} else {
		g_bMergeAllFields = true;
	}
}

void CLetterView::OnUpdateMergeAllFields(CCmdUI* pCmdUI) 
{
	if (g_bMergeAllFields) {
		pCmdUI->SetCheck(0);
	} else {
		pCmdUI->SetCheck(1);
	}
}

void CLetterView::OnUpdateView() 
{
	//JMJ - 12/3/2003 - it now always refreshes in the regular update view handler
	//m_frame.m_groupEditor.DoRefresh();
}

void CLetterView::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	m_frame.m_groupEditor.UpdateView(bForceRefresh);
	m_frame.UpdateView(bForceRefresh);
}
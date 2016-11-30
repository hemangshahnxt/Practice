// AdminView.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ReportView.h"

#include "GlobalUtils.h"
#include "MainFrm.h"
#include "nxmessagedef.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReportView

IMPLEMENT_DYNCREATE(CReportView, CNxView)

CReportView::CReportView()
	: m_frame(this)
{
	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Reports_Module/run_reports.htm";
}

CReportView::~CReportView()
{
}


BEGIN_MESSAGE_MAP(CReportView, CNxView)
	//{{AFX_MSG_MAP(CReportView)
	ON_WM_CREATE()
	ON_WM_SIZE()
//	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint) // b.cardillo 3-20-2002: not necessary because MFC realizes that the ON_COMMAND is here so it automatically enables
	ON_COMMAND(ID_FILE_PRINT, OnPrint)
//	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrintPreview) // b.cardillo 3-20-2002: not necessary because MFC realizes that the ON_COMMAND is here so it automatically enables
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnPrintPreview)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReportView drawing

void CReportView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CReportView diagnostics

#ifdef _DEBUG
void CReportView::AssertValid() const
{
	CNxView::AssertValid();
}

void CReportView::Dump(CDumpContext& dc) const
{
	CNxView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CReportView message handlers
#include "RegSvr32.h"

BOOL CReportView::CheckPermissions()
{
	if (!UserPermission(ReportModuleItem)) {
		return FALSE;
	}
	return TRUE;
}

// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the frame to get the active tab
short CReportView::GetActiveTab()
{
	if (IsWindow(m_frame.GetSafeHwnd())) {
		return m_frame.GetActiveTab();
	} else {
		// The frame isn't initialized
		return -1;
	}
}

// (c.haag 2009-01-12 16:45) - PLID 32683 - Sets the active tab in the reports sheet.
// Returns TRUE on success, and FALSE on failure.
// (a.walling 2010-11-26 13:08) - PLID 40444 - Renamed
BOOL CReportView::SetActiveTab(short tab)
{
	ASSERT(tab < ReportsModule::Last_Tab);

	if (IsWindow(m_frame.GetSafeHwnd())) {
		return m_frame.SetActiveTab((ReportsModule::Tab)tab);
	} else {
		// The frame isn't initialized
		return FALSE;
	}
}

// (c.haag 2009-01-12 16:56) - PLID 32683 - Clears the current report batch
void CReportView::ClearReportBatch()
{
	if (IsWindow(m_frame.GetSafeHwnd())) {
		m_frame.ClearSelection();
	} else {
		// The frame isn't initialized
	}
}

// (c.haag 2009-01-12 16:56) - PLID 32683 - Adds a report to the current batch
void CReportView::AddReportToBatch(long nReportID)
{
	if (IsWindow(m_frame.GetSafeHwnd())) {
		m_frame.AddReportFromExternalModule(nReportID);
	} else {
		// The frame isn't initialized
	}
}

int CReportView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxView::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	// Try to create the dialog
	BOOL bSuccess = m_frame.Create(IDD_REPORTS, this);

	// If it didn't create and the suspect ocx is there, make sure it's registered and try again
	if (!bSuccess) {
		CString strPath;
		if (GetSystemPath(strPath) && DoesExist(strPath ^ "Tab32x20.ocx")) {
			RegSvr32(strPath ^ "Tab32x20.ocx");
			bSuccess = m_frame.Create(IDD_REPORTS, this);
		}
	} 

	if (!bSuccess) {
		// Tell the user what happened and return failure
		// This is likely because of a missing ocx, probably the 
		// Tab32x20.  We checked above to try to automatically 
		// register the files if they are there, but if they're 
		// just not there, what are we supposed to do?
		MsgBox(MB_ICONEXCLAMATION|MB_OK,
			"The Reports Module could not be loaded.  Some system or "
			"program files may be missing or corrupt.  Please ensure "
			"that Practice has been properly installed.");
		return -1;
	}

	// We're good to go so show the created frame
	m_frame.ShowWindow(SW_SHOW);

	// Return success
	return 0;
}

void CReportView::OnSize(UINT nType, int cx, int cy) 
{
	CNxView::OnSize(nType, cx, cy);
	
	RECT rect;

	GetParent()->GetClientRect (&rect);
	m_frame.MoveWindow(&rect);

	
}

void CReportView::OnPrint() 
{
	CView::OnFilePrint();
	//m_frame.Print(false);
}

void CReportView::OnPrintPreview() 
{
	m_frame.Print(true);
}

void CReportView::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	m_frame.UpdateView(bForceRefresh);
}


/*
	  void CPadView::OnUpdateWordWrap(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsWordWrap());
}

void CPadView::OnWordWrap()
{
	CWaitCursor wait;
	SetWordWrap(!IsWordWrap());
	m_bDefWordWrap = IsWordWrap();
}
*/

BOOL CReportView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	m_frame.Print(pInfo->m_bPreview ? 1 : 0, pInfo);
	return false;
}


LRESULT CReportView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
	case NXM_ALLOW_CLOSE:
		return m_frame.CheckAllowClose();
		break;
	case NXM_ADD_REPORT:
		m_frame.PostMessage(message, wParam, lParam);
		return 0;
	}
	return CNxView::WindowProc(message, wParam, lParam);
}

LRESULT CReportView::OnTableChanged(WPARAM wParam, LPARAM lParam) {
	try {
		if (m_frame) {
			m_frame.OnTableChanged(wParam, lParam);
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}
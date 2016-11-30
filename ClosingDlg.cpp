// ClosingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ClosingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClosingDlg dialog


CClosingDlg::CClosingDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CClosingDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClosingDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_hWndNextWindow = NULL;
}


void CClosingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClosingDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CClosingDlg, CNxDialog)
	//{{AFX_MSG_MAP(CClosingDlg)
	ON_WM_ACTIVATE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClosingDlg message handlers

void CClosingDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CNxDialog::OnActivate(nState, pWndOther, bMinimized);
	
//	if (nState == WA_ACTIVE) {
//		if (pWndOther) {
//			m_hWndNextWindow = pWndOther->GetNextWindow()->GetSafeHwnd();
//		}
//	}	
}

BOOL CClosingDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (a.walling 2007-05-07 10:38) - PLID 4850 - Override the title
	if (m_strTitle.GetLength())
		SetWindowText(m_strTitle);
	
	SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CClosingDlg::OnDestroy() 
{
//	if (m_hWndNextWindow ) {
//		::BringWindowToTop(m_hWndNextWindow);
//	}
	CNxDialog::OnDestroy();
}

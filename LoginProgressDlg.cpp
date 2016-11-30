// LoginProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "LoginProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoginProgressDlg dialog


CLoginProgressDlg::CLoginProgressDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLoginProgressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoginProgressDlg)
	m_strLastMsg = _T("");
	//}}AFX_DATA_INIT
}


void CLoginProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoginProgressDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Text(pDX, IDC_STATIC_LAST_MSG, m_strLastMsg);
	DDX_Control(pDX, IDC_STATIC_LAST_MSG, m_nxstaticLastMsg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoginProgressDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLoginProgressDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoginProgressDlg message handlers

void CLoginProgressDlg::SetProgress(long nProgress /* 0 - 100 */, const CString& str)
{
	m_strLastMsg = str;
	m_progress.SetPos(nProgress);
	UpdateData(FALSE);
	ProcessMessages();
}

void CLoginProgressDlg::ProcessMessages()
{
	extern CPracticeApp theApp;

	// for tracking the idle time state
	MSG msg;

	// acquire and dispatch messages until a WM_QUIT message is received.
	while (::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE)) {

		// pump message, but quit on WM_QUIT
		if (!theApp.PumpMessage()) {
			theApp.ExitInstance();
			return;
		}
	}
}

BOOL CLoginProgressDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	m_progress.SetRange(0,100);	
	m_progress.SetPos(0);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

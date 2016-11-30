// UpdatingPendingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "UpdatingPendingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUpdatingPendingDlg dialog


CUpdatingPendingDlg::CUpdatingPendingDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUpdatingPendingDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdatingPendingDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUpdatingPendingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdatingPendingDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_UPDATE_PENDING_PROGRESS, m_nxstaticUpdatePendingProgress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUpdatingPendingDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUpdatingPendingDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdatingPendingDlg message handlers

void CUpdatingPendingDlg::OnOK() 
{
	//This function has been intentionally left blank.
}

void CUpdatingPendingDlg::OnCancel() 
{
	//This function has been intentionally left blank.
}

BOOL CUpdatingPendingDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//DRT 4/10/2007 - PLID 25564 - Removed PeekAndPump in favor of a global version.

void CUpdatingPendingDlg::SetProgressMessage(int nCurrentPos, int nMax)
{
	CString strMessage;
	strMessage.Format("Updating appointment %i of %i ...", nCurrentPos, nMax);
	SetDlgItemText(IDC_UPDATE_PENDING_PROGRESS, strMessage);
	PeekAndPump();
}

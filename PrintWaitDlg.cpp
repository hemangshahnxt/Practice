// PrintWaitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PrintWaitDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (z.manning, 05/21/2008) - PLID 30050 - Created
/////////////////////////////////////////////////////////////////////////////
// CPrintWaitDlg dialog


CPrintWaitDlg::CPrintWaitDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPrintWaitDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPrintWaitDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPrintWaitDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrintWaitDlg)
	DDX_Control(pDX, IDC_LABEL, m_nxstaticLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrintWaitDlg, CDialog)
	//{{AFX_MSG_MAP(CPrintWaitDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrintWaitDlg message handlers

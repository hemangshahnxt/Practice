// ProcessRptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ProcessRptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcessRptDlg dialog


CProcessRptDlg::CProcessRptDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessRptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProcessRptDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CProcessRptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcessRptDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_DESCRIPTION_LABEL, m_nxstaticDescriptionLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcessRptDlg, CDialog)
	//{{AFX_MSG_MAP(CProcessRptDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcessRptDlg message handlers

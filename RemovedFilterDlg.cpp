// RemovedFilterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "letterwriting.h"
#include "RemovedFilterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRemovedFilterDlg dialog


CRemovedFilterDlg::CRemovedFilterDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRemovedFilterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRemovedFilterDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CRemovedFilterDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRemovedFilterDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_REMOVAL_EXPLANATION, m_nxstaticRemovalExplanation);
	DDX_Control(pDX, IDOK, m_btnOk);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRemovedFilterDlg, CNxDialog)
	//{{AFX_MSG_MAP(CRemovedFilterDlg)
	ON_BN_CLICKED(IDC_REMOVED_FILTER_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRemovedFilterDlg message handlers

BOOL CRemovedFilterDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/25/2008) - PLID 29795 - Set button style
	m_btnOk.AutoSet(NXB_CLOSE);
	
	SetDlgItemText(IDC_REMOVAL_EXPLANATION, m_strCaption);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRemovedFilterDlg::OnHelp() 
{
	OpenManual(m_strHelpLocation, m_strHelpBookmark);
}

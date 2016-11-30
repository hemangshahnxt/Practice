// UpdatePendingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "UpdatePendingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUpdatePendingDlg dialog


CUpdatePendingDlg::CUpdatePendingDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUpdatePendingDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdatePendingDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUpdatePendingDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdatePendingDlg)
	DDX_Control(pDX, IDC_ALWAYS, m_btnAlways);
	DDX_Control(pDX, IDOK, m_btnYes);
	DDX_Control(pDX, IDCANCEL, m_btnNo);
	DDX_Control(pDX, IDC_MESSAGE, m_nxstaticMessage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUpdatePendingDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUpdatePendingDlg)
	ON_BN_CLICKED(IDC_ALWAYS, OnAlways)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdatePendingDlg message handlers

void CUpdatePendingDlg::OnAlways() 
{
	//DRT - 1/29/03 - This button says "Always Update Without Asking", so why are we turning ON the checkbox to ask them?
	//SetRemotePropertyInt("AlwaysUpdatePending", 1, 0, "<None>");
	SetRemotePropertyInt("AlwaysUpdatePending", 0, 0, "<None>");
	CDialog::OnOK();
}

BOOL CUpdatePendingDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnAlways.AutoSet(NXB_OK);
	m_btnYes.AutoSet(NXB_OK);
	m_btnNo.AutoSet(NXB_CANCEL);
	
	SetDlgItemText(IDC_MESSAGE, m_strMessage);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CUpdatePendingDlg::OnOK() 
{
	
	CDialog::OnOK();
}

// UpdatePendingConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "UpdatePendingConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUpdatePendingConfigDlg dialog


CUpdatePendingConfigDlg::CUpdatePendingConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUpdatePendingConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUpdatePendingConfigDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUpdatePendingConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUpdatePendingConfigDlg)
	DDX_Control(pDX, IDC_REMEMBER, m_btnRemember);
	DDX_Control(pDX, IDOK, m_btnOut);
	DDX_Control(pDX, IDC_NOSHOW, m_btnNoShow);
	DDX_Control(pDX, IDCANCEL, m_btnDoNothing);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUpdatePendingConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUpdatePendingConfigDlg)
	ON_BN_CLICKED(IDC_NOSHOW, OnNoshow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUpdatePendingConfigDlg message handlers

BOOL CUpdatePendingConfigDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnOut.AutoSet(NXB_OK);
		m_btnNoShow.AutoSet(NXB_OK);
		m_btnDoNothing.AutoSet(NXB_CANCEL);

	}NxCatchAll("CUpdatePendingConfigDlg::OnInitDialog");

	return TRUE;
}

void CUpdatePendingConfigDlg::OnOK() 
{
	m_nUpdateType = 1;
	if(IsDlgButtonChecked(IDC_REMEMBER)) {
		SetRemotePropertyInt("UpdatePendingType", m_nUpdateType, 0, "<None>");
	}
	CDialog::OnOK();
}

void CUpdatePendingConfigDlg::OnNoshow() 
{
	m_nUpdateType = 2;
	if(IsDlgButtonChecked(IDC_REMEMBER)) {
		SetRemotePropertyInt("UpdatePendingType", m_nUpdateType, 0, "<None>");
	}
	CDialog::OnOK();
}

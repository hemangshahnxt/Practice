// LicenseUpdateCodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "LicenseUpdateCodeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLicenseUpdateCodeDlg dialog


CLicenseUpdateCodeDlg::CLicenseUpdateCodeDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLicenseUpdateCodeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLicenseUpdateCodeDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLicenseUpdateCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLicenseUpdateCodeDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_LICENSE_UPDATE_CODE, m_nxeditLicenseUpdateCode);
	DDX_Control(pDX, IDC_LICENSE_UPDATE_CODE_CAPTION, m_nxstaticLicenseUpdateCodeCaption);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLicenseUpdateCodeDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLicenseUpdateCodeDlg)
	ON_BN_CLICKED(IDC_DOWNLOAD_UPDATE_CODE, OnDownloadUpdateCode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLicenseUpdateCodeDlg message handlers

void CLicenseUpdateCodeDlg::OnOK() 
{
	GetDlgItemText(IDC_LICENSE_UPDATE_CODE, m_strUpdateCode);
	CDialog::OnOK();
}

BOOL CLicenseUpdateCodeDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	SetDlgItemText(IDC_LICENSE_UPDATE_CODE_CAPTION, "In order to initialize your license, please enter an update code below.  This code can be obtained from NexTech Technical Support at 888-417-8464, or if you have an internet connection click Download to automatically download an update code for your office.\nNote: Update codes are not case-sensitive");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLicenseUpdateCodeDlg::OnDownloadUpdateCode() 
{
	EndDialog(ID_DOWNLOAD_CODE);
}

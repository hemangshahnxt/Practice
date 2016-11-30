// NexwebFTPSettingsDlg.cpp : implementation file
//

//(e.lally 2009-04-30) PLID 34123 - Depreciated

#include "stdafx.h"
#include "practice.h"
#include "NexwebFTPSettingsDlg.h"
/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNexwebFTPSettingsDlg dialog


CNexwebFTPSettingsDlg::CNexwebFTPSettingsDlg(CWnd* pParent)
	: CNxDialog(CNexwebFTPSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNexwebFTPSettingsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNexwebFTPSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexwebFTPSettingsDlg)
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_FTP_SITE, m_nxeditFtpSite);
	DDX_Control(pDX, IDC_FTP_USERNAME, m_nxeditFtpUsername);
	DDX_Control(pDX, IDC_FTPPASSWORD, m_nxeditFtppassword);
	DDX_Control(pDX, IDC_FTP_CONFIRM_PASSWORD, m_nxeditFtpConfirmPassword);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexwebFTPSettingsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNexwebFTPSettingsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexwebFTPSettingsDlg message handlers

void CNexwebFTPSettingsDlg::OnOK() 
{
	//first check to make sure the password and confirm password are correct

	CString strPass, strConfirm;

	GetDlgItemText(IDC_FTPPASSWORD, strPass);
	GetDlgItemText(IDC_FTP_CONFIRM_PASSWORD, strConfirm);

	if (strPass.Compare(strConfirm) != 0) {
		AfxMessageBox("The password and confirm password boxes are not the same.");
		return;
	}
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	//otherwise we are good, let's save
	CString strUserName, strSite;
	GetDlgItemText(IDC_FTP_USERNAME, strUserName);
	GetDlgItemText(IDC_FTP_SITE, strSite);

	SetRemotePropertyText("NexwebFTPUserName", strUserName, 0, "<None>");
	SetRemotePropertyText("NexwebFTPPassword", EncryptString(strPass, "ENCRYPT"), 0, "<None>");
	SetRemotePropertyText("NexwebFTPSite", strSite, 0, "<None>");
	
	CDialog::OnOK();
}

BOOL CNexwebFTPSettingsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	//load the information

	SetDlgItemText(IDC_FTP_USERNAME, GetRemotePropertyText("NexwebFTPUserName", "", 0, "<None>", true));
	SetDlgItemText(IDC_FTP_SITE, GetRemotePropertyText("NexwebFTPSite", "", 0, "<None>", true));
	SetDlgItemText(IDC_FTPPASSWORD, EncryptString(GetRemotePropertyText("NexwebFTPPassword", "", 0, "<None>", true), "DECRYPT"));
	SetDlgItemText(IDC_FTP_CONFIRM_PASSWORD, EncryptString(GetRemotePropertyText("NexwebFTPPassword", "", 0, "<None>", true), "DECRYPT"));	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
*/
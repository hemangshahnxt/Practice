// NexSyncConnectionSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "NexSyncConnectionSettingsDlg.h"


// (z.manning 2009-10-08 15:46) - PLID 35574 - Created
// CNexSyncConnectionSettingsDlg dialog

IMPLEMENT_DYNAMIC(CNexSyncConnectionSettingsDlg, CNxDialog)

CNexSyncConnectionSettingsDlg::CNexSyncConnectionSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexSyncConnectionSettingsDlg::IDD, pParent)
{
	m_eAccountType = natGoogle;
}

CNexSyncConnectionSettingsDlg::~CNexSyncConnectionSettingsDlg()
{
}

// (c.haag 2010-05-20 13:38) - PLID 38744 - Update the text and color of the password button
void CNexSyncConnectionSettingsDlg::UpdatePasswordBtnAppearance()
{
	if (m_strPassword.IsEmpty()) {
		m_btnPassword.SetWindowText("Set Password");
		m_btnPassword.SetTextColor(RGB(255, 0, 0));
	}
	else {
		m_btnPassword.SetWindowText("Change Password");			
		m_btnPassword.SetTextColor(0x008000);
	}
}

void CNexSyncConnectionSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NEXSYNC_SERVER, m_nxeditServer);
	DDX_Control(pDX, IDC_NEXSYNC_USER, m_nxeditUser);
	DDX_Text(pDX, IDC_NEXSYNC_SERVER, m_strServer);
	DDX_Text(pDX, IDC_NEXSYNC_USER, m_strUser);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NEXSYNC_GOOGLE, m_btnGoogle);
	DDX_Control(pDX, IDC_NEXSYNC_OTHER, m_btnOther);
	DDX_Control(pDX, IDC_BTN_NEXSYNC_PASSWORD, m_btnPassword);
}

BEGIN_MESSAGE_MAP(CNexSyncConnectionSettingsDlg, CNxDialog)
	ON_EN_CHANGE(IDC_NEXSYNC_USER, OnEnChangeNexSyncUser)
	ON_BN_CLICKED(IDC_NEXSYNC_GOOGLE, OnGoogleRadioBtn)
	ON_BN_CLICKED(IDC_NEXSYNC_OTHER, OnOtherRadioBtn)
	ON_BN_CLICKED(IDC_BTN_NEXSYNC_PASSWORD, OnNexSyncPasswordBtn)
END_MESSAGE_MAP()


// CNexSyncConnectionSettingsDlg message handlers

BOOL CNexSyncConnectionSettingsDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_nxeditServer.SetLimitText(1000);
		m_nxeditUser.SetLimitText(255);
		//m_nxeditPassword.SetLimitText(255); // (c.haag 2010-05-20 13:38) - PLID 38744 - Deprecated

		// (c.haag 2010-05-20 13:38) - PLID 38744 - Added a button for changing the password
		m_btnPassword.AutoSet(NXB_MODIFY);
		UpdatePasswordBtnAppearance();

		// (z.manning 2010-02-08 15:02) - PLID 37142 - Account type
		switch(m_eAccountType)
		{
			case natOther:
				CheckDlgButton(IDC_NEXSYNC_OTHER, BST_CHECKED);
				break;

			case natGoogle:
				CheckDlgButton(IDC_NEXSYNC_GOOGLE, BST_CHECKED);
				break;
		}

		UpdateData(FALSE);
		UpdateAddressField();

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CNexSyncConnectionSettingsDlg::OnOK()
{
	try
	{
		UpdateData(TRUE);

		if(IsDlgButtonChecked(IDC_NEXSYNC_GOOGLE) == BST_CHECKED) {
			m_eAccountType = natGoogle;
		}
		else {
			m_eAccountType = natOther;
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CNexSyncConnectionSettingsDlg::SetServer(const CString strServer)
{
	m_strServer = strServer;
}

void CNexSyncConnectionSettingsDlg::SetUser(const CString strUser)
{
	m_strUser = strUser;
}

void CNexSyncConnectionSettingsDlg::SetPassword(const CString strPassword)
{
	m_strPassword = strPassword;
}

CString CNexSyncConnectionSettingsDlg::GetServer()
{
	return m_strServer;
}

CString CNexSyncConnectionSettingsDlg::GetUser()
{
	return m_strUser;
}

CString CNexSyncConnectionSettingsDlg::GetPassword()
{
	return m_strPassword;
}

// (z.manning 2010-02-08 14:27) - PLID 37142
void CNexSyncConnectionSettingsDlg::SetAccountType(ENexSyncAccountTypes eType)
{
	m_eAccountType = eType;
}

// (z.manning 2010-02-08 16:39) - PLID 37142
ENexSyncAccountTypes CNexSyncConnectionSettingsDlg::GetAccountType()
{
	return m_eAccountType;
}

// (z.manning 2010-02-08 16:39) - PLID 37142
void CNexSyncConnectionSettingsDlg::OnEnChangeNexSyncUser()
{
	try
	{
		UpdateAddressField();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-02-08 16:39) - PLID 37142
void CNexSyncConnectionSettingsDlg::UpdateAddressField()
{
	if(IsDlgButtonChecked(IDC_NEXSYNC_GOOGLE) == BST_CHECKED) {
		m_nxeditServer.SetReadOnly(TRUE);
		UpdateData(TRUE);
		m_strServer.Format("https://www.google.com/calendar/dav/%s/events/", m_strUser);
		UpdateData(FALSE);
	}
	else {
		m_nxeditServer.SetReadOnly(FALSE);
	}
}

// (z.manning 2010-02-08 16:39) - PLID 37142
void CNexSyncConnectionSettingsDlg::OnGoogleRadioBtn()
{
	try
	{
		UpdateAddressField();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-02-08 16:39) - PLID 37142
void CNexSyncConnectionSettingsDlg::OnOtherRadioBtn()
{
	try
	{
		UpdateAddressField();

	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-05-20 13:38) - PLID 38744 - Updates m_strPassword
void CNexSyncConnectionSettingsDlg::OnNexSyncPasswordBtn()
{
	try {
		CString strResult;
		if (IDOK == InputBoxLimited(this, "Please enter your password", strResult, "",255,true,false,NULL))
		{
			m_strPassword = strResult;
			UpdatePasswordBtnAppearance();
		}
	}
	NxCatchAll(__FUNCTION__);
}


// SSRSSetup.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SSRSSetupDlg.h"
#include "AdministratorRc.h"
#include "NxAPI.h"

// CSSRSSetupDlg dialog

IMPLEMENT_DYNAMIC(CSSRSSetupDlg, CNxDialog)

CSSRSSetupDlg::CSSRSSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(IDD_SSRS_SETUP_DLG, pParent)
{

}

CSSRSSetupDlg::~CSSRSSetupDlg()
{
}

void CSSRSSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_EDIT_WINDOWS_SSRS_USERNAME, m_nxeUsername);
	DDX_Control(pDX, IDC_EDIT_WINDOWS_SSRS_PASSWORD, m_nxePassword);
}


BEGIN_MESSAGE_MAP(CSSRSSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CSSRSSetupDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSSRSSetupDlg message handlers
BOOL CSSRSSetupDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_nxeUsername.SetLimitText(64); //Chose 64 for practicality per Microsoft.
		m_nxePassword.SetLimitText(127); //Chose 127 per Microsoft

		LoadSSRSUsernameAndPassword();

	}
	NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CSSRSSetupDlg::LoadSSRSUsernameAndPassword()
{
	NexTech_Accessor::_SSRSSettingsPtr pSSRSSettings(__uuidof(NexTech_Accessor::SSRSSettings));
	pSSRSSettings = GetAPI()->GetSSRSSettings(GetAPISubkey(), GetAPILoginToken());

	if (pSSRSSettings != NULL) {
		m_strCurrentUsername = AsString(pSSRSSettings->username);
		m_strCurrentPassword = AsString(pSSRSSettings->password);

		m_nxeUsername.SetWindowText(m_strCurrentUsername);
		m_nxePassword.SetWindowText(m_strCurrentPassword);
	}
}

void CSSRSSetupDlg::OnBnClickedOk()
{
	try {

		CString strPlainTextPassword, strUsername;
		m_nxePassword.GetWindowText(strPlainTextPassword);
		m_nxeUsername.GetWindowText(strUsername);

		//Silently truncates any username and password combo that has only spaces or has leading or trailing spaces 
		strUsername = strUsername.Trim();
		strPlainTextPassword = strPlainTextPassword.Trim(); 

		if (strUsername.IsEmpty() && strPlainTextPassword.IsEmpty() && HasUsernameOrPasswordChanged(strUsername, strPlainTextPassword)) {
			//They had a password when they opened the window and cleared it out -- clear the table
			ExecuteSql("DELETE FROM SSRSAuthenticationT;");
		}else if (!strUsername.IsEmpty() && !strPlainTextPassword.IsEmpty() && HasUsernameOrPasswordChanged(strUsername, strPlainTextPassword)) {
			ExecuteParamSql(R"(
					DELETE FROM SSRSAuthenticationT;				
					INSERT INTO SSRSAuthenticationT (Username, EncryptedPassword) VALUES ({STRING}, {VARBINARY});
				)", strUsername, EncryptStringToVariant(strPlainTextPassword)
			);
		}
		else if ((!strUsername.IsEmpty() && strPlainTextPassword.IsEmpty()) || (strUsername.IsEmpty() && !strPlainTextPassword.IsEmpty())) {
			MessageBox("Username / Password cannot be empty.", "Nextech", MB_ICONINFORMATION);
			return;
		}
		else {
			//Opened with creds, closed with same creds.
		}
	}
	NxCatchAll(__FUNCTION__);

	CNxDialog::OnOK();
}

bool CSSRSSetupDlg::HasUsernameOrPasswordChanged(const CString& strUsername, const CString& strPlainTextPassword)
{
	return m_strCurrentUsername.CompareNoCase(strUsername) != 0 || m_strCurrentPassword != strPlainTextPassword;
}

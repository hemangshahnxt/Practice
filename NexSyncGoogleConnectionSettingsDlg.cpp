// NexSyncGoogleConnectionSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "NexSyncGoogleConnectionSettingsDlg.h"
#include "NxUtils.h"


// (d.singleton 2013-02-28 16:10) - PLID 55377 - Created
// CNexSyncGoogleConnectionSettingsDlg dialog
// (d.singleton 2013-02-28 16:10) - PLID 55377 we now use google api,  removed all caldev specific code

IMPLEMENT_DYNAMIC(CNexSyncGoogleConnectionSettingsDlg, CNxDialog)

CNexSyncGoogleConnectionSettingsDlg::CNexSyncGoogleConnectionSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexSyncGoogleConnectionSettingsDlg::IDD, pParent)
{
	m_strRefreshToken = "";
	m_strUserName = "";
}

CNexSyncGoogleConnectionSettingsDlg::~CNexSyncGoogleConnectionSettingsDlg()
{
}

// (d.singleton 2013-02-28 15:36) - PLID 55377 no longer use password changed to authcode and refreshtoken
void CNexSyncGoogleConnectionSettingsDlg::UpdateAuthCodeBtnAppearance()
{
	if (m_strRefreshToken.IsEmpty()) {
		SetDlgItemText(IDC_STATIC_IS_AUTHORIZED, "");
		m_btnAuthCode.SetTextColor(RGB(255, 0, 0));	
	}
	else
	{
		CString strCalendarText;

		// (z.manning 2013-04-22 12:14) - PLID 56362 - Let's try and load the name of the primary calendar corresponding
		// to our refresh token (the primary calendar name is just about always the Google account name).
		// (d.singleton 2013-07-03 17:00) - PLID 57446 - moved to own function.  since caldav needs it too			
		strCalendarText = "  Calendar name: " + m_strUserName;


		SetDlgItemText(IDC_STATIC_IS_AUTHORIZED, "Your Google account has been authorized." + strCalendarText);		
		m_btnAuthCode.SetTextColor(0x008000);
	}
}

void CNexSyncGoogleConnectionSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_GENERATE_CODE, m_btnAuthCode);
	DDX_Control(pDX, IDC_BTN_AUTHORIZE_CODE, m_btnRefToken);
	DDX_Control(pDX, IDC_BTN_AUTHORIZE_HELP, m_btnHelp);
	DDX_Control(pDX, IDC_AUTHORIZATION_CODE, m_nxeditAuthCode);
}

BEGIN_MESSAGE_MAP(CNexSyncGoogleConnectionSettingsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_GENERATE_CODE, OnNexSyncAuthCodeBtn)
	ON_BN_CLICKED(IDC_BTN_AUTHORIZE_CODE, &CNexSyncGoogleConnectionSettingsDlg::OnBnClickedBtnAuthorizeCode)
	ON_BN_CLICKED(IDC_BTN_AUTHORIZE_HELP, &CNexSyncGoogleConnectionSettingsDlg::OnBnClickedBtnHelp)
END_MESSAGE_MAP()


// CNexSyncGoogleConnectionSettingsDlg message handlers

BOOL CNexSyncGoogleConnectionSettingsDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CLOSE);		
		
		// (d.singleton 2013-02-28 15:36) - PLID 55377 no longer use password changed to auth code
		m_btnAuthCode.AutoSet(NXB_MODIFY);
		m_btnRefToken.AutoSet(NXB_MODIFY);
		m_btnHelp.AutoSet(NXB_QUESTION);
		// (d.singleton 2013-02-28 15:36) - PLID 55377 set upper limit, 250 should always be much more than enough
		m_nxeditAuthCode.SetLimitText(250);
		UpdateAuthCodeBtnAppearance();

		UpdateData(FALSE);

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CNexSyncGoogleConnectionSettingsDlg::OnCancel()
{
	try
	{
		UpdateData(TRUE);

		// (d.singleton 2013-02-28 16:08) - PLID 55377 warn them if they try click OK while there is still an auth code entered ( there will only be one if they never authorized it )
		CString strAuthCode = "";
		GetDlgItemText(IDC_AUTHORIZATION_CODE, strAuthCode);
		if(!strAuthCode.IsEmpty()) {
			if(IDYES == MessageBox("Are you sure you wish to close the connection settings without authorizing the currently entered code?", 
				"Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				CNxDialog::OnOK();
			}
			else {
				return;
			}
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2013-02-28 16:08) - PLID 55375 we no longer use password,  we instead now need to get an authorization code from google.
void CNexSyncGoogleConnectionSettingsDlg::OnNexSyncAuthCodeBtn()
{
	try {
		NexTech_COM::IGoogleOAuth2AuthenticatorPtr pNxOAuth2Authenticator;
		pNxOAuth2Authenticator.CreateInstance(_T("NexTech_COM.GoogleOAuth2"));
		
		if(pNxOAuth2Authenticator) {
			pNxOAuth2Authenticator->GetAuthorizationCode();
		}
		else {
			ThrowNxException("Error initializing Authenticator::Library not registered");
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (d.singleton 2013-02-28 16:08) - PLID 55375 once we have an auth code we need to authorize the code and return a refresh token we can then store
void CNexSyncGoogleConnectionSettingsDlg::OnBnClickedBtnAuthorizeCode()
{
	try {
		CString strAuthCode;
		GetDlgItemText(IDC_AUTHORIZATION_CODE, strAuthCode);
		if(strAuthCode.IsEmpty()) {
			AfxMessageBox("Please fill in an Authorization code before trying to authorize.");
			return;
		}
		NexTech_COM::IGoogleOAuth2AuthenticatorPtr pNxOAuth2Authenticator;
		pNxOAuth2Authenticator.CreateInstance(_T("NexTech_COM.GoogleOAuth2"));

		//create a failed message to show if we return false or on exception
		CString strFailedMessage = "Practice could not authorize your NexSync account with Google.  Please contact NexTech for assistance";
		if(pNxOAuth2Authenticator) {			
			_bstr_t strRefreshToken;
			try {
				//have a bool check just in case it return false instead of an exception
				if(!pNxOAuth2Authenticator->GetRefreshToken(_bstr_t(strAuthCode), strRefreshToken.GetAddress())) {
					AfxMessageBox(strFailedMessage);
				}
			} catch(_com_error e) {
				Log("OnBnClickedBtnAuthorizeCode: %s (%s)", (LPCTSTR)e.ErrorMessage(), (LPCTSTR)e.Description());
				AfxMessageBox(strFailedMessage);
				return;
			}
			m_strRefreshToken = VarString(strRefreshToken);
			//give message letting them know of success
			MessageBox("Your Google account has been successfully authorized.", "Practice", MB_ICONINFORMATION|MB_OK);
			SetDlgItemText(IDC_AUTHORIZATION_CODE, "");
			// (d.singleton 2013-07-03 17:26) - PLID 57446 - unify our authentication for nexsync between CalDav and GoogleApi
			RetrieveAuthorizedGoogleAccount();
			UpdateAuthCodeBtnAppearance();
		}
		else {
			ThrowNxException("Error initializing Authenticator::Library not registered");
		}
	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2013-02-28 16:08) - PLID 55377 added help button to give instructions on authorizing
void CNexSyncGoogleConnectionSettingsDlg::OnBnClickedBtnHelp()
{
	try {
		CString strHelpText = "In order to authorize Practice to sync with your Google calendar please complete the following steps. \r\n\r\n"
			"1. Log out of all Google accounts \r\n"
			"2. Click the \"Generate Code\" button.  This will open up your default internet browser to Google's Login page. \r\n"
			"3. Log in with the Google account whose calendar Practice will sync with. \r\n"
			"4. Google will ask you to allow Practice to modify your Google calendar,  click allow. \r\n"
			"5. Google will provide you with an Authorization code.  Copy this code into the \"Authorization Code\" text box on the NexSync Connection Settings window. \r\n"
			"6. Once you have pasted in the code click the \"Authorize Code\" button. \r\n\r\n"
			"This will provide Practice with the needed permissions to access and sync with your Google calendar.  If you have any questions please contact NexTech Technical Support. \r\n";

		MessageBox(strHelpText, "NexTech Practice", MB_ICONQUESTION|MB_OK);
	}NxCatchAll(__FUNCTION__);
}

void CNexSyncGoogleConnectionSettingsDlg::SetRefreshToken(const CString strRefreshToken)
{
	m_strRefreshToken = strRefreshToken;
}

CString CNexSyncGoogleConnectionSettingsDlg::GetRefreshToken()
{
	return m_strRefreshToken;
}

void CNexSyncGoogleConnectionSettingsDlg::SetNexSyncUserName(const CString strUserName)
{
	m_strUserName = strUserName;
}

CString CNexSyncGoogleConnectionSettingsDlg::GetNexSyncUserName()
{
	return m_strUserName;
}

// (d.singleton 2013-07-03 17:26) - PLID 57446 - unify our authentication for nexsync between CalDav and GoogleApi
void CNexSyncGoogleConnectionSettingsDlg::RetrieveAuthorizedGoogleAccount()
{
	try {
		NexTech_COM::IGoogleOAuth2AuthenticatorPtr pNxOAuth2Authenticator;
		pNxOAuth2Authenticator.CreateInstance(_T("NexTech_COM.GoogleOAuth2"));
		m_strUserName = (LPCTSTR)pNxOAuth2Authenticator->GetPrimaryCalendarNameFromRefreshToken(_bstr_t(m_strRefreshToken));
	}NxCatchAllIgnore();
}
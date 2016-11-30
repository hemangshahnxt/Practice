#pragma once


// (d.singleton 2013-02-28 16:10) - PLID 55377 - Created
// CNexSyncGoogleConnectionSettingsDlg dialog
// (d.singleton 2013-02-28 16:10) - PLID 55377 we now use google api,  removed all caldev specific code

class CNexSyncGoogleConnectionSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexSyncGoogleConnectionSettingsDlg)

public:
	CNexSyncGoogleConnectionSettingsDlg(CWnd* pParent);   // standard constructor
	virtual ~CNexSyncGoogleConnectionSettingsDlg();

	void SetRefreshToken(const CString strAuthCode);

	// (d.singleton 2013-02-28 15:49) - PLID 55377 no longer use any of the below functions,  took them out
	
	CString GetRefreshToken();
	// (d.singleton 2013-07-03 17:00) - PLID 57446 - unify connection settings
	CString GetNexSyncUserName();
	void SetNexSyncUserName(CString strUserName);
	
// Dialog Data
	enum { IDD = IDD_NEXSYNC_GOOGLE_CONNECTION_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	
	// (d.singleton 2013-02-28 15:36) - PLID 55377 no longer use password changed to auth code
	void UpdateAuthCodeBtnAppearance();
	// (d.singleton 2013-07-03 17:00) - PLID 57446 - get the google account that was authorized
	void RetrieveAuthorizedGoogleAccount();

	// (d.singleton 2013-02-28 15:36) - PLID 55377 no lnoger use server and user,  renamed password to auth code	
	CString m_strRefreshToken;
	// (d.singleton 2013-02-28 15:46) - PLID 55377 there is only one account type,  its google

	// (d.singleton 2013-07-03 17:00) - PLID 57446 - unify CalDav and google api connection settings.
	CString m_strUserName;

	CNxIconButton m_btnAuthCode;
	CNxIconButton m_btnRefToken;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnHelp;
	CNxEdit m_nxeditAuthCode;	

	DECLARE_MESSAGE_MAP()
	// (d.singleton 2013-02-28 15:36) - PLID 55377 no lnoger use server and user,  renamed password to auth code
	void OnNexSyncAuthCodeBtn();
	afx_msg void OnBnClickedBtnAuthorizeCode();
	afx_msg void OnBnClickedBtnHelp();

public:
	
};

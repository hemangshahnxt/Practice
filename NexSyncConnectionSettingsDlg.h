#pragma once


// (z.manning 2009-10-08 15:46) - PLID 35574 - Created
// CNexSyncConnectionSettingsDlg dialog

// (z.manning 2010-02-08 13:58) - PLID 37142 - We now track the account type
enum ENexSyncAccountTypes {
	natOther = 0,
	natGoogle,
};

class CNexSyncConnectionSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CNexSyncConnectionSettingsDlg)

public:
	CNexSyncConnectionSettingsDlg(CWnd* pParent);   // standard constructor
	virtual ~CNexSyncConnectionSettingsDlg();

	void SetServer(const CString strServer);
	void SetUser(const CString strUser);
	void SetPassword(const CString strPassword);
	void SetAccountType(ENexSyncAccountTypes eType); // (z.manning 2010-02-08 14:27) - PLID 37142

	CString GetServer();
	CString GetUser();
	CString GetPassword();
	ENexSyncAccountTypes GetAccountType();

// Dialog Data
	enum { IDD = IDD_NEXSYNC_CONNECTION_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	void UpdateAddressField();

	// (c.haag 2010-05-20 13:38) - PLID 38744 - Update the text and color of the password button
	void UpdatePasswordBtnAppearance();

	CString m_strServer;
	CString m_strUser;
	CString m_strPassword;
	ENexSyncAccountTypes m_eAccountType;

	CNxEdit m_nxeditServer;
	CNxEdit m_nxeditUser;	
	//CNxEdit m_nxeditPassword; // (c.haag 2010-05-20 13:38) - PLID 38744 - Replaced with a password button
	CNxIconButton m_btnPassword;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	NxButton m_btnGoogle;
	NxButton m_btnOther;	

	DECLARE_MESSAGE_MAP()
	void OnEnChangeNexSyncUser();
	void OnGoogleRadioBtn();
	void OnOtherRadioBtn();
	void OnNexSyncPasswordBtn();
};

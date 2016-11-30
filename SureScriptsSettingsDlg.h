#pragma once

//TES 4/2/2009 - PLID 33376 - Created
// CSureScriptsSettingsDlg dialog



class CSureScriptsSettingsDlg : public CNxDialog
{

public:
	CSureScriptsSettingsDlg(CWnd* pParent);   // standard constructor
	virtual ~CSureScriptsSettingsDlg();

// Dialog Data
	enum { IDD = IDD_SURESCRIPTS_SETTINGS_DLG };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	NxButton m_checkEnableSureScripts;
	CNxEdit m_nxeClientID;
	CNxEdit m_nxeAuthKey;
	CNxEdit m_nxeURL;
	NxButton m_checkNotify;
	CNxIconButton m_btnSelectProviders;
	NxButton m_grpCommSettings;
	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	//TES 4/3/2009 - PLID 33376 - Track the initial settings, so we can tell if anything changed when we save.
	BOOL m_bOldSureScriptsEnabled;
	CString m_strOldClientID;
	CString m_strOldAuthKey;
	CString m_strOldURL;
	BOOL m_bOldNotify;
	
	CArray<int,int> m_arNotifyProviders;
	//TES 4/10/2009 - PLID 33889 - Track whether the array changes, so we know when we save whether 
	// we need to update our notifications.
	bool m_bNotifyProvidersChanged;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnEnableSureScripts();
	afx_msg void OnSureScriptsNotify();
	afx_msg void OnSelectProviders();
	virtual void OnOK();
};

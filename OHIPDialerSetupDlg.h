#pragma once

// COHIPDialerSetupDlg dialog

// (j.jones 2009-03-09 13:28) - PLID 32405 - created

class COHIPDialerSetupDlg : public CNxDialog
{

public:
	COHIPDialerSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~COHIPDialerSetupDlg();

// Dialog Data
	enum { IDD = IDD_OHIP_DIALER_SETUP_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnSend;
	CNxIconButton	m_btnBrowseDownload;
	CNxIconButton	m_btnBrowseTT;
	NxButton		m_checkChangePassword;
	// (s.dhole 2011-01-17 14:06) - PLID 42145 Added checkbox for TeraTerm/NXModem
	NxButton		m_checkUseTeraTerm;
	NxButton		m_checkDeleteReports;
	// (b.spivey, June 26th, 2014) - PLID 62603 - Checkbox to enable/disable AutoDial. 
	NxButton		m_checkEnableAutoDialer;
	NxButton		m_checkShowTerminal;
	CNxEdit			m_editDownloadPath;
	CNxEdit			m_editPhoneNumber;
	CNxEdit			m_editCommPort;
	CNxEdit			m_editUsername;
	CNxEdit			m_editPassword;
	CNxEdit			m_editNewPassword;
	CNxEdit			m_editTTLocation;
	// (j.jones 2009-08-14 10:54) - PLID 34914 - added per-provider options
	NxButton		m_checkSubmitPerProvider;
	CNxStatic		m_nxstaticProviderLabel;

protected:

	BOOL Save();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// (j.jones 2009-08-14 10:54) - PLID 34914 - added per-provider options
	NXDATALIST2Lib::_DNxDataListPtr m_ProviderCombo;
	long m_nCurProviderID;
	BOOL m_bNeedSaveProviderData;

	// (j.jones 2009-08-14 11:12) - PLID 34914 - used for provider info. saving
	BOOL ValidateAndCacheCurrentProviderInfo();

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	afx_msg void OnBtnSend();
	afx_msg void OnBtnBrowseDownload();
	afx_msg void OnBtnBrowseTt();
	afx_msg void OnCheckChangePassword();
	// (j.jones 2009-08-14 10:35) - PLID 34914 - added per-provider login information
	afx_msg void OnCheckSubmitPerProvider();
	DECLARE_EVENTSINK_MAP()
	void OnSelChosenOhipDialerProviderCombo(LPDISPATCH lpRow);

	void EnableAllControls(bool bEnableControls);
	afx_msg void OnBnClickedCheckEnableAutoDialer();

public:
	// (s.dhole 2011-01-17 14:06) - PLID 42145 Added checkbox for TeraTerm/NXModem
	afx_msg void OnBnClickedCheckTeraterm();
	afx_msg void OnEnKillfocusEditTeratermLoc();
};

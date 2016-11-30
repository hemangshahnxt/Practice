#pragma once

// (d.lange 2010-05-19 14:10) - PLID 38536 - Created
// CDevicePluginSetupDlg dialog

class CDevicePluginSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CDevicePluginSetupDlg)

public:
	CDevicePluginSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CDevicePluginSetupDlg();

// Dialog Data
	enum { IDD = IDD_DEVICES_SETUP_DLG };

	CString m_strDeviceExportPath;
	CString m_strDeviceExePath;
	CString m_strDeviceAddlPath;
	CString m_strPluginName;
	long m_nLaunchSetting;

	// (b.savon 2012-02-07 15:11) - PLID 48019 - Keep track of the clean plugin description
	// (j.gruber 2013-04-22 13:16) - PLID 56032 - not needed anymore
	//CString m_strPlugin;

protected:
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxEdit m_nxeditImportPath;
	CNxEdit m_nxeditExportPath;
	CNxEdit m_nxeditExePath;
	CNxEdit m_nxeditPluginName;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnBrowseAddl();
	afx_msg void OnBnClickedBtnBrowseExport();
	afx_msg void OnBnClickedBtnBrowseExe();
	afx_msg void OnEnChangeEditPluginName();
	afx_msg void OnEnChangeDeviceExportPath();
	afx_msg void OnEnChangeExePath();
	afx_msg void OnEnChangeDeviceAddlPath();
	afx_msg void OnBnClickedOk();
};

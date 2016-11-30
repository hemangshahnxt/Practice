#pragma once


// CICCPAdvSettingsDlg dialog
// (z.manning 2015-09-30 13:58) - PLID 67255 - Created

class CICCPAdvSettingsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CICCPAdvSettingsDlg)

public:
	CICCPAdvSettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CICCPAdvSettingsDlg();

	BOOL m_bChanged;
	BOOL m_bRestartNeeded;

// Dialog Data
	enum { IDD = IDD_ICCP_ADV_SETTINGS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog() override;
	virtual void OnCancel() override;

	CNxIconButton m_btnClose;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnEnKillfocusIccpComPort();
	afx_msg void OnEnKillfocusIccpDeviceProperitesOverride();
	afx_msg void OnEnChangeIccpComPort();
	afx_msg void OnEnChangeIccpDeviceProperitesOverride();
};

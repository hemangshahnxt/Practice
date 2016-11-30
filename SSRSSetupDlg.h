#pragma once


// CSSRSSetupDlg dialog

class CSSRSSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSSRSSetupDlg)
private:

	CNxIconButton	m_btnClose;
	CNxEdit m_nxeUsername;
	CNxEdit m_nxePassword;

	CString m_strCurrentUsername;
	CString m_strCurrentPassword;

	void LoadSSRSUsernameAndPassword();
	bool HasUsernameOrPasswordChanged(const CString& strUsername, const CString& strPlainTextPassword);

public:
	CSSRSSetupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSSRSSetupDlg();
	virtual BOOL OnInitDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SSRS_SETUP_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

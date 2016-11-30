// (f.gelderloos 2013-08-14 14:49) - PLID 57914 Abstracting common functionality
#pragma once
// NxChangePasswordDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNxChangePasswordDlg dialog
// (f.gelderloos 2013-08-14 14:49) - PLID 57914 Abstracting this class for common-functionality
class CNxChangePasswordDlg : public CNxDialog
{
	// Construction
public:
	CNxChangePasswordDlg(long nPersonID, CString strUserName, CWnd* pParent, boost::function<struct NexWebPasswordComplexity*()> fnComplexity);   // standard constructor


	// Dialog Data
	//{{AFX_DATA(CNxChangePasswordDlg)
	enum { IDD = IDD_NEXWEB_PASSWORD_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditNexwebNewPassword;
	CNxEdit	m_nxeditNexwebConfirmPassword;
	//}}AFX_DATA


	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxChangePasswordDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// (j.gruber 2009-01-06 17:49) - PLID 32480 - auto generate a password
	// Implementation
protected:

	// (b.savon 2012-09-05 17:06) - PLID 52464 - Display valid banner
	CString GetPasswordRequirementBanner();
	CString FormatBannerParameter(long nValue);

	long m_nPersonID;
	CString m_strUserName;
	BOOL ValidateAndSave();
	// Generated message map functions
	//{{AFX_MSG(CNxChangePasswordDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();

	// (j.armen 2013-10-03 07:08) - PLID 57914 - Return true on success
	virtual bool UpdateData(const CString &strPass) = 0;
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedGenerateNewPassword();
	boost::function<struct NexWebPasswordComplexity*()> m_fnComplexity;
};
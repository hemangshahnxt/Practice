#if !defined(AFX_LICENSEINFODLG_H__12C59E51_F76C_423C_AE5F_A9006E6D2DD7__INCLUDED_)
#define AFX_LICENSEINFODLG_H__12C59E51_F76C_423C_AE5F_A9006E6D2DD7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LicenseInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLicenseInfoDlg dialog

class CLicenseInfoDlg : public CNxDialog
{
// Construction
public:
	CLicenseInfoDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLicenseInfoDlg)
	enum { IDD = IDD_LICENSE_INFO_DLG };
	CNxIconButton	m_btnClose;
	CNxEdit	m_nxeditLicenseInfoBox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLicenseInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLicenseInfoDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedAnalyticsUsers();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LICENSEINFODLG_H__12C59E51_F76C_423C_AE5F_A9006E6D2DD7__INCLUDED_)

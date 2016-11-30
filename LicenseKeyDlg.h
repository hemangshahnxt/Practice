#if !defined(AFX_LICENSEKEYDLG_H__D67DC689_82AF_4705_BFC7_3010E6894D10__INCLUDED_)
#define AFX_LICENSEKEYDLG_H__D67DC689_82AF_4705_BFC7_3010E6894D10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LicenseKeyDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLicenseKeyDlg dialog

class CLicenseKeyDlg : public CNxDialog
{
// Construction
public:
	CLicenseKeyDlg(CWnd* pParent);   // standard constructor

	CString m_strMessage;
	int m_nLicenseKey;
	
	// (a.walling 2011-04-13 12:20) - PLID 40262
	CString m_strServer;
	CString m_strOfficialServer;

// Dialog Data
	//{{AFX_DATA(CLicenseKeyDlg)
	enum { IDD = IDD_LICENSE_KEY_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditEnterLicenseKey;
	CNxStatic	m_nxstaticLicenseKeyCaption;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLicenseKeyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLicenseKeyDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	// (a.walling 2011-04-13 12:20) - PLID 40262
	afx_msg void OnBnClickedLicenseLaunchNxdock();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LICENSEKEYDLG_H__D67DC689_82AF_4705_BFC7_3010E6894D10__INCLUDED_)

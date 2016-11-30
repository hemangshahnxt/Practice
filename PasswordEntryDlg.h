#if !defined(AFX_PASSWORDENTRYDLG_H__644428A6_48B2_11D2_9840_00104B318376__INCLUDED_)
#define AFX_PASSWORDENTRYDLG_H__644428A6_48B2_11D2_9840_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PasswordEntryDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPasswordEntryDlg dialog

class CPasswordEntryDlg : public CNxDialog
{
// Construction
public:
	bool OpenPassword(CString &strPass, const CString& strWindowText = "Please Enter Password");
	CPasswordEntryDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPasswordEntryDlg)
	enum { IDD = IDD_USER_PASSWORD_DLG };
	CString	m_EnteredPassword;
	CNxEdit	m_nxeditPasswordBox;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPasswordEntryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_strWindowText;

	// Generated message map functions
	//{{AFX_MSG(CPasswordEntryDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PASSWORDENTRYDLG_H__644428A6_48B2_11D2_9840_00104B318376__INCLUDED_)

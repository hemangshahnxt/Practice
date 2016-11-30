#if !defined(AFX_CHANGEPASSWORDDLG_H__6773BA94_CA75_4E22_96E9_4446670EC8D1__INCLUDED_)
#define AFX_CHANGEPASSWORDDLG_H__6773BA94_CA75_4E22_96E9_4446670EC8D1__INCLUDED_
 
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChangePasswordDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChangePasswordDlg dialog

class CChangePasswordDlg : public CNxDialog
{
// Construction
public:
	CChangePasswordDlg(CWnd* pParent);   // standard constructor
	void SetUserID(long nID);
	void SetUserName(const CString& strName);
	CString GetNewPassword();
	void SetLocationID(long nID); // (b.savon 2015-12-16 09:29) - PLID 67718

// Dialog Data
	//{{AFX_DATA(CChangePasswordDlg)
	enum { IDD = IDD_CHANGE_PASSWORD };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditOldPassword;
	CNxEdit	m_nxeditSelfPassword;
	CNxEdit	m_nxeditConfirmPassword;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangePasswordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	long m_nUserID;
	long m_nLocationID; // (b.savon 2015-12-16 09:29) - PLID 67718
	CString m_strUserName;
	CString m_strNewPassword;

	// Generated message map functions
	//{{AFX_MSG(CChangePasswordDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHANGEPASSWORDDLG_H__6773BA94_CA75_4E22_96E9_4446670EC8D1__INCLUDED_)

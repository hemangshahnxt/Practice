#if !defined(AFX_USERWARNINGDLG_H__A04A01F3_515A_11D2_80D3_00104B2FE914__INCLUDED_)
#define AFX_USERWARNINGDLG_H__A04A01F3_515A_11D2_80D3_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UserWarningDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUserWarningDlg dialog

class CUserWarningDlg : public CNxDialog
{
// Construction
public:
	// (a.walling 2010-07-01 16:15) - PLID 18081 - Warning categories - backgroundColor parameter (0 for default)
	BOOL DoModalWarning(const CString& strFmt, BOOL bKeepWarning, const CString& strTitle, const CString& strCaption, COLORREF backgroundColor, int nWarningCategoryID, const CString& strWarningCategoryName);

	// This checks to see if a warning should be given for a patient,
	// and if so, gives it.
	BOOL DoModalWarning(long nPatientID, BOOL bKeepWarning);

	CUserWarningDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	CString m_strCaption;

	//{{AFX_DATA(CUserWarningDlg)
	enum { IDD = IDD_USER_WARNING_DLG };
	NxButton	m_chkKeepWarning;
	CString	m_strWarningMsg;
	BOOL	m_bKeepWarning;
	CString	m_strTitleLabel;
	CNxEdit	m_nxeditWarningMsg;
	CNxStatic	m_nxstaticTitleLabel;
	CNxIconButton m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUserWarningDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// (a.walling 2010-05-17 12:02) - PLID 18081 - Warning categories - Override edit background color
	COLORREF m_colorWarning;
	int m_nWarningCategoryID;
	CString m_strWarningCategoryName;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUserWarningDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnKeepWarningCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_USERWARNINGDLG_H__A04A01F3_515A_11D2_80D3_00104B2FE914__INCLUDED_)

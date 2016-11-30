#if !defined(AFX_LOGINPROGRESSDLG_H__625653F2_DF6E_46B6_B41A_485E637BD153__INCLUDED_)
#define AFX_LOGINPROGRESSDLG_H__625653F2_DF6E_46B6_B41A_485E637BD153__INCLUDED_

#include "practicerc.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoginProgressDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLoginProgressDlg dialog

class CLoginProgressDlg : public CNxDialog
{
// Construction
public:
	CLoginProgressDlg(CWnd* pParent);   // standard constructor

	void SetProgress(long nProgress /* 0 - 100 */, const CString& str);


// Dialog Data
	//{{AFX_DATA(CLoginProgressDlg)
	enum { IDD = IDD_LOGIN_PROGRESS };
	CProgressCtrl	m_progress;
	CString	m_strLastMsg;
	CNxStatic	m_nxstaticLastMsg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoginProgressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ProcessMessages();

	// Generated message map functions
	//{{AFX_MSG(CLoginProgressDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGINPROGRESSDLG_H__625653F2_DF6E_46B6_B41A_485E637BD153__INCLUDED_)

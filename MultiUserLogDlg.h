//PLID 26192	6/6/08	r.galicki	-	Allows for log management of multiple users

#if !defined(AFX_MULTIUSERLOGDLG_H__CA606324_5D79_400F_AC23_3E58D793E947__INCLUDED_)
#define AFX_MULTIUSERLOGDLG_H__CA606324_5D79_400F_AC23_3E58D793E947__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiUserLogDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMultiUserLogDlg dialog

class CMultiUserLogDlg : public CNxDialog
{
// Construction
public:
	CMultiUserLogDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMultiUserLogDlg)
	enum { IDD = IDD_MULTI_USER_LOG };
	CNxIconButton	m_btnLogInOut;
	CNxIconButton	m_btnCancel;
	CNxEdit			m_edtPassword;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiUserLogDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pUserList;

	// Generated message map functions
	//{{AFX_MSG(CMultiUserLogDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdateLog();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTIUSERLOGDLG_H__CA606324_5D79_400F_AC23_3E58D793E947__INCLUDED_)

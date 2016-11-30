#if !defined(AFX_OVERRIDEUSERDLG_H__B85D98C1_223A_49B6_B9A8_C8965B5E2353__INCLUDED_)
#define AFX_OVERRIDEUSERDLG_H__B85D98C1_223A_49B6_B9A8_C8965B5E2353__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OverrideUserDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COverrideUserDlg dialog

//DRT 6/7/2007 - PLID 25892

class COverrideUserDlg : public CNxDialog
{
// Construction
public:
	COverrideUserDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(COverrideUserDlg)
	enum { IDD = IDD_OVERRIDE_USER_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditOverridePassword;
	//}}AFX_DATA

	CString m_strOverrideWhereClause;			//To override the where clause of the user list.  This is given directly to the datalist WhereClause property
	long m_nApprovedUserID;						//The user ID of the user who successfully logged in.
	CString m_strApprovedUserName;				//The username of the user who successfully logged in


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COverrideUserDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pUserList;

	// Generated message map functions
	//{{AFX_MSG(COverrideUserDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OVERRIDEUSERDLG_H__B85D98C1_223A_49B6_B9A8_C8965B5E2353__INCLUDED_)

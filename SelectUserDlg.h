#if !defined(AFX_SELECTUSERDLG_H__3E25CC85_2C08_4609_B412_8F9A09AF4EBA__INCLUDED_)
#define AFX_SELECTUSERDLG_H__3E25CC85_2C08_4609_B412_8F9A09AF4EBA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectUserDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectUserDlg dialog

class CSelectUserDlg : public CNxDialog
{
// Construction
public:
	CSelectUserDlg(CWnd* pParent);   // standard constructor

	CString m_strCaption;
	long	m_nExcludeUser;
	long	m_nSelectedUser;
	bool	m_bAllowNone;	// PLID 22458 - Whether the (None) option should appear in our dialog
	NXDATALISTLib::_DNxDataListPtr	m_pUserList;
	CString m_strUserWhereClause;

// Dialog Data
	//{{AFX_DATA(CSelectUserDlg)
	enum { IDD = IDD_SELECT_USER };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticSelectUserCaption;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectUserDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectUserDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedUserList(short nFlags);
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTUSERDLG_H__3E25CC85_2C08_4609_B412_8F9A09AF4EBA__INCLUDED_)

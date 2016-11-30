#if !defined(AFX_CHOOSEADMINDLG_H__D75431EC_E13F_438C_84E1_A522FD45FDA7__INCLUDED_)
#define AFX_CHOOSEADMINDLG_H__D75431EC_E13F_438C_84E1_A522FD45FDA7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChooseAdminDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CChooseAdminDlg dialog

class CChooseAdminDlg : public CNxDialog
{
// Construction
public:

	NXDATALISTLib::_DNxDataListPtr m_UserList;

	CChooseAdminDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChooseAdminDlg)
	enum { IDD = IDD_CHOOSE_ADMIN_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChooseAdminDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	BOOL SaveAdministrator();

	// Generated message map functions
	//{{AFX_MSG(CChooseAdminDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDblClickCellUserList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHOOSEADMINDLG_H__D75431EC_E13F_438C_84E1_A522FD45FDA7__INCLUDED_)

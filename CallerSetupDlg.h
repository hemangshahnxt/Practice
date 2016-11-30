#if !defined(AFX_CALLERSETUPDLG_H__6BEC273D_20DE_43DA_86BD_F311CEE348C8__INCLUDED_)
#define AFX_CALLERSETUPDLG_H__6BEC273D_20DE_43DA_86BD_F311CEE348C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CallerSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCallerSetupDlg dialog

class CCallerSetupDlg : public CNxDialog
{
// Construction
public:
	void Save(int nID);
	void Load();
	CCallerSetupDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCallerSetupDlg)
	enum { IDD = IDD_CALLER_SETUP };
	CNxEdit	m_nxeditStartMsg;
	CNxEdit	m_nxeditMsg;
	CNxEdit	m_nxeditLocalCode;
	CNxEdit	m_nxeditOutside;
	CNxEdit	m_nxeditUser1;
	CNxEdit	m_nxeditUser2;
	CNxEdit	m_nxeditUser3;
	CNxEdit	m_nxeditUser4;
	CNxEdit	m_nxeditUser5;
	CNxEdit	m_nxeditUser6;
	CNxEdit	m_nxeditUser7;
	CNxEdit	m_nxeditUser8;
	CNxEdit	m_nxeditUser9;
	CNxEdit	m_nxeditUser0;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRename;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr m_listGroup;
	NXDATALISTLib::_DNxDataListPtr m_listUser;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCallerSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCallerSetupDlg)
		virtual void OnCancel();
		virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnRename();
	afx_msg void OnAdd();
	afx_msg void OnSelChangedGroupList(long nNewSel);
	afx_msg void OnSelChosenUserList(long nRow);
	afx_msg void OnDelete();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CALLERSETUPDLG_H__6BEC273D_20DE_43DA_86BD_F311CEE348C8__INCLUDED_)

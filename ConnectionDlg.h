#if !defined(AFX_CONNECTIONDLG_H__17354245_920B_4922_9B0D_8C6E8070BF13__INCLUDED_)
#define AFX_CONNECTIONDLG_H__17354245_920B_4922_9B0D_8C6E8070BF13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConnectionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CConnectionDlg dialog

class CConnectionDlg : public CNxDialog
{
// Construction
public:
	CConnectionDlg(CWnd* pParent);

// Dialog Data
	//{{AFX_DATA(CConnectionDlg)
	enum { IDD = IDD_CONNECT_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditSqlservername;
	CNxEdit	m_nxeditSqlserverip;
	CNxEdit	m_nxeditNxserverip;
	CNxEdit	m_nxeditSharedpath;
	CNxStatic	m_nxstaticMessage;
	//}}AFX_DATA

	CString m_sqlServerName, 
			m_sqlServerIP, 
			m_nxServerIP, 
			m_sharedPath,
			m_connectionType,
			m_message;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConnectionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_conTypeList;
	int SaveNxReg(long nID, CString subkey, CString &value);
	CString LoadNxReg(CString subkey);
	// Generated message map functions
	//{{AFX_MSG(CConnectionDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTIONDLG_H__17354245_920B_4922_9B0D_8C6E8070BF13__INCLUDED_)

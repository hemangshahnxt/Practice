// PalmDialogDlg.h : header file
//

#include "Address.h"
#include "Purposes.h"
#include "globalutils.h"

#if !defined(AFX_PALMDIALOGDLG_H__1EC78C23_E1E4_4AAD_AF09_262249E08353__INCLUDED_)
#define AFX_PALMDIALOGDLG_H__1EC78C23_E1E4_4AAD_AF09_262249E08353__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPalmDialogDlg dialog

class CPalmDialogDlg : public CNxDialog
{
protected:
	DECLARE_DYNAMIC(CPalmDialogDlg)
// Construction
public:
	CPalmDialogDlg(CWnd* pParent);	// standard constructor

	CAddress m_AddressDlg;
	CPurposes m_PurposesDlg;

	long GetUserID();
	void SetUserID(long ID);
	
	//These are public functions that our child dialogs and call if they accidentally get OK or Cancel messages.
	void OK();
	void Cancel();
// Dialog Data
	//{{AFX_DATA(CPalmDialogDlg)
	enum { IDD = IDD_PALMDIALOG_DIALOG };
	CTabCtrl	m_Tab;
	BOOL	m_bAuditSyncs;
	CNxEdit	m_nxeditEditDatabasename;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	NxButton	m_btnAuditSync;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPalmDialogDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	
// Implementation
protected:
	ADODB::_RecordsetPtr m_rs;
	NXDATALISTLib::_DNxDataListPtr m_dlUsers;
	long m_nPUserID;

	// Generated message map functions
	//{{AFX_MSG(CPalmDialogDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PALMDIALOGDLG_H__1EC78C23_E1E4_4AAD_AF09_262249E08353__INCLUDED_)

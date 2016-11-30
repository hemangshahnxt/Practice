#if !defined(AFX_STATEMENTARNOTESCONFIGDLG_H__4E5A7AA1_BF56_4EBD_82BD_EBB52B077ACF__INCLUDED_)
#define AFX_STATEMENTARNOTESCONFIGDLG_H__4E5A7AA1_BF56_4EBD_82BD_EBB52B077ACF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StatementARNotesConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStatementARNotesConfigDlg dialog

class CStatementARNotesConfigDlg : public CNxDialog
{
// Construction
public:
	CStatementARNotesConfigDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStatementARNotesConfigDlg)
	enum { IDD = IDD_STATEMENT_AR_NOTES_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditThirtyDayNote;
	CNxEdit	m_nxeditSixtyDayNote;
	CNxEdit	m_nxeditNinetyDayNote;
	CNxEdit	m_nxeditNinetyPlusNote;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStatementARNotesConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStatementARNotesConfigDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATEMENTARNOTESCONFIGDLG_H__4E5A7AA1_BF56_4EBD_82BD_EBB52B077ACF__INCLUDED_)

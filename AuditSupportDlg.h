#if !defined(AFX_AUDITSUPPORTDLG_H__7D459FE3_5B51_42CD_AECB_1E794DE5BCF8__INCLUDED_)
#define AFX_AUDITSUPPORTDLG_H__7D459FE3_5B51_42CD_AECB_1E794DE5BCF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AuditSupportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAuditSupportDlg dialog

class CAuditSupportDlg : public CNxDialog
{
// Construction
public:
	CAuditSupportDlg(CWnd* pParent);   // standard constructor
	CString m_strText;

// Dialog Data
	//{{AFX_DATA(CAuditSupportDlg)
	enum { IDD = IDD_AUDIT_SUPPORT_DLG };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxEdit	m_nxeditAuditText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAuditSupportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAuditSupportDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDITSUPPORTDLG_H__7D459FE3_5B51_42CD_AECB_1E794DE5BCF8__INCLUDED_)

#if !defined(AFX_ENTERACTIVEDATEDLG_H__264BE631_8E8F_412A_AEFB_0721203A3297__INCLUDED_)
#define AFX_ENTERACTIVEDATEDLG_H__264BE631_8E8F_412A_AEFB_0721203A3297__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EnterActiveDateDlg.h : header file
//

#include <afxdtctl.h>

/////////////////////////////////////////////////////////////////////////////
// CEnterActiveDateDlg dialog

class CEnterActiveDateDlg : public CNxDialog
{
// Construction
public:
	CEnterActiveDateDlg(CWnd* pParent);   // standard constructor

	CString m_strPrompt;
	COleDateTime m_dtDate;
	bool m_bAllowCancel;
	bool m_bAllowPastDate;
	// (z.manning, 02/11/2008) - PLID 28885 - You can now specify the window's title.
	CString m_strWindowTitle;

// Dialog Data
	//{{AFX_DATA(CEnterActiveDateDlg)
	enum { IDD = IDD_ENTER_ACTIVE_DATE };
	CDateTimeCtrl	m_dtcActiveDate;
	CNxStatic	m_nxstaticPromptLabel;
	CNxIconButton	m_btnEadOk;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEnterActiveDateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEnterActiveDateDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnEadOk();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENTERACTIVEDATEDLG_H__264BE631_8E8F_412A_AEFB_0721203A3297__INCLUDED_)

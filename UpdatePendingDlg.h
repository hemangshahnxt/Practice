#if !defined(AFX_UPDATEPENDINGDLG_H__EF2BF8B7_137C_4ACB_9F4E_8D0A7BD6AC23__INCLUDED_)
#define AFX_UPDATEPENDINGDLG_H__EF2BF8B7_137C_4ACB_9F4E_8D0A7BD6AC23__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UpdatePendingDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUpdatePendingDlg dialog

class CUpdatePendingDlg : public CNxDialog
{
// Construction
public:
	CUpdatePendingDlg(CWnd* pParent);   // standard constructor

	CString m_strMessage;
// Dialog Data
	//{{AFX_DATA(CUpdatePendingDlg)
	enum { IDD = IDD_UPDATE_PENDING };
	CNxIconButton	m_btnAlways;
	CNxIconButton	m_btnYes;
	CNxIconButton	m_btnNo;
	CNxStatic	m_nxstaticMessage;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUpdatePendingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUpdatePendingDlg)
	afx_msg void OnAlways();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UPDATEPENDINGDLG_H__EF2BF8B7_137C_4ACB_9F4E_8D0A7BD6AC23__INCLUDED_)

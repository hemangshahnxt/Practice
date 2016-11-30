#if !defined(AFX_PROMPTDLG_H__DF3190CA_1F6B_46F6_942D_7602CA4F2D7C__INCLUDED_)
#define AFX_PROMPTDLG_H__DF3190CA_1F6B_46F6_942D_7602CA4F2D7C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PromptDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPromptDlg dialog

class CPromptDlg : public CNxDialog
{
// Construction
public:
	CPromptDlg(CWnd* pParent);   // standard constructor
	CString m_strPrompt;

	// (z.manning, 04/30/2008) - PLID 29860 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CPromptDlg)
	enum { IDD = IDD_ENTER_PROMPT };
	CNxEdit	m_nxeditPromptText;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPromptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	

	// Generated message map functions
	//{{AFX_MSG(CPromptDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnPreview();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROMPTDLG_H__DF3190CA_1F6B_46F6_942D_7602CA4F2D7C__INCLUDED_)

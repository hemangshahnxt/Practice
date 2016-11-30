#if !defined(AFX_CODELINKDLG_H__8EB310AB_0F8F_42E2_B90D_081A32616579__INCLUDED_)
#define AFX_CODELINKDLG_H__8EB310AB_0F8F_42E2_B90D_081A32616579__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CodeLinkDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCodeLinkDlg dialog

class CCodeLinkDlg : public CNxDialog
{
// Construction
public:
	CCodeLinkDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCodeLinkDlg)
	enum { IDD = IDD_CODELINK_DIALOG };
	NxButton	m_btnEnableCodeLink;
	CNxEdit	m_nxeditEditCodelinkPath;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCodeLinkDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCodeLinkDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnBrowseCodelinkPath();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CODELINKDLG_H__8EB310AB_0F8F_42E2_B90D_081A32616579__INCLUDED_)

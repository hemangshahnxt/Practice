#if !defined(AFX_MERGEPROMPTDLG_H__103C0209_24D6_472D_A4E6_842B5CFE336E__INCLUDED_)
#define AFX_MERGEPROMPTDLG_H__103C0209_24D6_472D_A4E6_842B5CFE336E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MergePromptDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMergePromptDlg dialog

class CMergePromptDlg : public CNxDialog
{
// Construction
public:
	CMergePromptDlg(CWnd* pParent);   // standard constructor

	bool m_bDirectToPrinter;
	CString m_strDocumentType;

// Dialog Data
	//{{AFX_DATA(CMergePromptDlg)
	enum { IDD = IDD_MERGE_PROMPT_DLG };
	NxButton	m_btnMergeDirect;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxStatic	m_nxstaticMergePrompt;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMergePromptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMergePromptDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MERGEPROMPTDLG_H__103C0209_24D6_472D_A4E6_842B5CFE336E__INCLUDED_)

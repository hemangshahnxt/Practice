#pragma once

// NxInputDlg.h : header file
//

#include "NxInputMask.h"

/////////////////////////////////////////////////////////////////////////////
// CNxInputDlg dialog

class CNxInputDlg : public CNxDialog
{
// Construction
public:
	bool m_bPassword;
	bool m_bBrowseBtn;
	LPCTSTR m_strCancelText;
	unsigned long m_nMaxChars;
	BOOL m_bIsNumeric;
	const CString &m_strPrompt;
	const CString &m_strOther;
	CString &m_strResult;
	// (z.manning 2015-08-13 16:55) - PLID 67248 - Added input mask and cue banner
	CString m_strInputMask;
	CString m_strCueBanner;

	// (c.haag 2007-09-11 16:16) - PLID 27353 - We now take an optional parent parameter
	// (a.walling 2011-12-21 16:10) - PLID 46648 - Dialogs must set a parent!
	CNxInputDlg(CWnd* pParent, const CString &strPrompt, CString &strResult, const CString &strOther, bool bPassword = false, bool bBrowseBtn = false, LPCTSTR strCancelBtnText = NULL, unsigned long nMaxChars = -1, BOOL bIsNumeric = FALSE);

// Dialog Data
	// (a.walling 2008-10-13 10:23) - PLID 28040 - Made this a proper NxDialog
	//{{AFX_DATA(CNxInputDlg)
		// NOTE: the ClassWizard will add data members here
		CNxStatic m_lblPrompt;
		CNxEdit m_editResult;
		CNxEdit m_editNumResult;
		CMaskedEdit m_editMasked;

		CNxIconButton m_nxibBrowse;

		CNxIconButton m_nxibOK;
		CNxIconButton m_nxibOther;
		CNxIconButton m_nxibCancel;		
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxInputDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNxInputDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAbort();
	afx_msg void OnBrowseBtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


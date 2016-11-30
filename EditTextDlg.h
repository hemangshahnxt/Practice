#if !defined(AFX_EDITTEXTDLG_H__F363D2C2_5A83_4C78_861F_B54C2007299F__INCLUDED_)
#define AFX_EDITTEXTDLG_H__F363D2C2_5A83_4C78_861F_B54C2007299F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditTextDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditTextDlg dialog
// (a.walling 2008-07-18 12:32) - PLID 30720 - Reusable dialog to edit text

class CEditTextDlg : public CNxDialog
{
// Construction
public:
	CEditTextDlg(CWnd* pParent, const CString& strText, const CString& strTitle, COLORREF m_color = 0);   // standard constructor

	CString GetText();

// Dialog Data
	//{{AFX_DATA(CEditTextDlg)
	enum { IDD = IDD_EDIT_TEXT_DLG };
	CNxIconButton	m_nxibOK;
	CNxIconButton	m_nxibCancel;
	CEdit	m_text; // purposefully NOT an NxEdit
	CNxColor	m_nxcolor;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditTextDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CString m_strText;
	CString m_strTitle;
	COLORREF m_color;
	// Generated message map functions
	//{{AFX_MSG(CEditTextDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITTEXTDLG_H__F363D2C2_5A83_4C78_861F_B54C2007299F__INCLUDED_)

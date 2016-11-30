#if !defined(AFX_EMRTEXTMACROBOXDLG_H__6F274426_051F_4EFB_947E_CE3CC736E253__INCLUDED_)
#define AFX_EMRTEXTMACROBOXDLG_H__6F274426_051F_4EFB_947E_CE3CC736E253__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrTextMacroBoxDlg.h : header file
//
// (c.haag 2008-06-06 09:25) - PLID 26038 - Initial implementation
//
#define	EMR_TEXT_MACRO_EDIT_IDC	1000

/////////////////////////////////////////////////////////////////////////////
// CEmrTextMacroBoxDlg dialog

class CEmrTextMacroBoxDlg : public CNxDialog
{
// Construction
public:
	CEmrTextMacroBoxDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEmrTextMacroBoxDlg)
	enum { IDD = IDD_EMR_TEXT_MACRO_BOX };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrTextMacroBoxDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CNxEdit m_edit;
	CNxStatic m_wndLabel;

protected:
	CBrush m_brMacroBackground;
	CNxIconButton m_btnSpellCheck;
	CFont* m_pButtonFont;

protected:
	void RepositionControls();

public:
	void SetLabelText(const CString& strLabel);
	CString GetTextData();
	void SetTextData(const CString& strTextData);

protected:
	COLORREF GetBackgroundColor();
	HBRUSH GetBackgroundBrush();

	// Generated message map functions
	//{{AFX_MSG(CEmrTextMacroBoxDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSpellCheck();
	afx_msg void OnChangeEditTextData();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRTEXTMACROBOXDLG_H__6F274426_051F_4EFB_947E_CE3CC736E253__INCLUDED_)

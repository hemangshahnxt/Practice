#if !defined(AFX_NXMODALPARENTDLG_H__B0E43948_846A_4F1F_BC71_63D3A621558C__INCLUDED_)
#define AFX_NXMODALPARENTDLG_H__B0E43948_846A_4F1F_BC71_63D3A621558C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NxModalParentDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CNxModalParentDlg dialog

class CNxModalParentDlg : public CNxDialog
{
// Construction
public:
	CNxModalParentDlg(CWnd* pParent, CNxDialog * pChild, CString &str);
	// (j.fouts 2013-01-21 09:29) - PLID 54703 - Added a constructor that takes the size as a paramater
	CNxModalParentDlg(CWnd* pParent, CNxDialog *child, CString &str, const CRect& rectSize);

// Dialog Data
	//{{AFX_DATA(CNxModalParentDlg)
	enum { IDD = IDD_NX_MODAL_PARENT_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNxModalParentDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (j.fouts 2013-01-21 09:29) - PLID 54703 - Store the size
	CRect m_rectSize;
	// (j.fouts 2013-01-21 09:29) - PLID 54703 - Track how we created this, if we should use the default size or not
	bool m_bUseDefaultSize;
	CNxDialog *m_pChild;
	CString m_title;
	// Generated message map functions
	//{{AFX_MSG(CNxModalParentDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NXMODALPARENTDLG_H__B0E43948_846A_4F1F_BC71_63D3A621558C__INCLUDED_)

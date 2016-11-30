#if !defined(AFX_CLOSINGDLG_H__37B1A6F6_573A_11D3_A38F_00C04F42E33B__INCLUDED_)
#define AFX_CLOSINGDLG_H__37B1A6F6_573A_11D3_A38F_00C04F42E33B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ClosingDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CClosingDlg dialog

class CClosingDlg : public CNxDialog
{
// Construction
public:
	CClosingDlg(CWnd* pParent);   // standard constructor
	inline void SetClosingTitle(CString strTitle) {m_strTitle = strTitle;};

// Dialog Data
	//{{AFX_DATA(CClosingDlg)
	enum { IDD = IDD_CLOSING_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClosingDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HWND m_hWndNextWindow;
	CString m_strTitle;

	// Generated message map functions
	//{{AFX_MSG(CClosingDlg)
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLOSINGDLG_H__37B1A6F6_573A_11D3_A38F_00C04F42E33B__INCLUDED_)

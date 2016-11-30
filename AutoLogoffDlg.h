#if !defined(AFX_AUTOLOGOFFDLG_H__33EFCDFE_2819_4ABD_9DC4_0A0DBD58AD25__INCLUDED_)
#define AFX_AUTOLOGOFFDLG_H__33EFCDFE_2819_4ABD_9DC4_0A0DBD58AD25__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AutoLogoffDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAutoLogoffDlg dialog

class CAutoLogoffDlg : public CNxDialog
{
// Construction
public:
	void ExtendInactivityTimer();
	void ResetInactivityTimer();
	void ClosePractice();
	CAutoLogoffDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAutoLogoffDlg)
	enum { IDD = IDD_AUTOLOGOFF };
	CNxIconButton	m_btnClose;
	CNxStatic	m_nxstaticTimeleft;
	CNxStatic	m_nxstaticAutologoffOptions;
	//}}AFX_DATA

	// (a.walling 2009-06-08 10:24) - PLID 34520 - Ensure no PHI is on screen
	void MinimizeAll();
	static BOOL CALLBACK MinimizeAllProc(HWND hwnd, LPARAM lParam);	
	struct MinimizeAllInfo
	{
		HWND hwndExclude;
		DWORD dwProcessID;
		DWORD dwThreadID;
	};



// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAutoLogoffDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bIgnoreExtensions;
	long m_nSeconds;
	unsigned long m_dwExpiration;

	// (a.walling 2009-06-08 09:44) - PLID 34520 - The countdown must be included in the timeout.
	DWORD GetAdjustedTimeoutMs();

	// Generated message map functions
	//{{AFX_MSG(CAutoLogoffDlg)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnBtnDontclose();
	afx_msg void OnBtnClose();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUTOLOGOFFDLG_H__33EFCDFE_2819_4ABD_9DC4_0A0DBD58AD25__INCLUDED_)

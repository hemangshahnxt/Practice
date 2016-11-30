// (j.jones 2005-01-28 10:02) - //{{AFX_INCLUDES()
//}}AFX_INCLUDES

#if !defined(AFX_LOGINDLG_H__235F14F3_4265_11D2_983A_00104B318376__INCLUDED_)
#define AFX_LOGINDLG_H__235F14F3_4265_11D2_983A_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CLoginDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLoginDlg dialog


// (a.walling 2008-04-16 17:42) - PLID 29691 - Animate window function
typedef BOOL (WINAPI * AnimateWindowType) (HWND, IN DWORD, DWORD);

class CLoginDlg : public CNxDialog
{
// Construction
public:
	void GiveFocus(UINT nID);
	void ApplyLocationFilter(long nLocId);
	long ChooseDefaultLocation();
	bool ChooseUser(const CString &strUsername);
	// (a.walling 2007-11-06 17:19) - PLID 27998 - VS2008 - This is obsolete and unreferenced
	// CString ConvertText(CString,CString);
	CString GetSystemName();
	CString m_CurrentUserPassword;

	// (j.jones 2015-04-20 09:11) - PLID 63694 - renamed for clarity
	bool LoadInitialLoginInfo();

	// (z.manning 2015-12-03 10:45) - PLID 67637 - Added password param
	void DisallowLogin(const CString &strPassword);
	BOOL m_bAllowLogin;
	BOOL m_bNexTechLoginAvailable;

	// (a.walling 2007-05-04 10:08) - PLID 4850 - TRUE if we are switching users, FALSE if initial login
	// Also, if this is TRUE then we are a modal dialog. If it is FALSE, we are actually the m_pMainWnd!
	BOOL m_bRelogin;

	HBITMAP m_image;
	CBrush m_brBrush;
	long m_nBackColor;

public:
	CLoginDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoginDlg();

// Dialog Data
	// (a.walling 2009-01-21 15:54) - PLID 32657 - CNxIconButtons for login screen buttons
	//{{AFX_DATA(CLoginDlg)
	enum { IDD = IDD_LOGIN };
	CString	m_Password;
	CProgressCtrl m_progLogin;
	NXDATALISTLib::_DNxDataListPtr m_lstUser;
	NXDATALISTLib::_DNxDataListPtr m_lstLocation;
	CNxStatic	m_nxstaticLoginLocationLabel;
	CNxStatic	m_nxstaticUsernameLabel;
	CNxStatic	m_nxstaticPasswordLabel;
	CNxStatic	m_nxstaticLoginStatusText;

	CNxIconButton m_nxibLogin;
	CNxIconButton m_nxibViewLicense;
	CNxIconButton m_nxibExit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoginDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	// (a.walling 2010-07-15 14:18) - PLID 38608 - Handle WM_TIMECHANGE
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL DoPromptForLicenseAgreement();
	//DRT 5/21/2008 - PLID 30117 - Removed Agn Beta NDA Prompt from the login
	// (d.thompson 2012-02-17) - PLID 48194
	UINT ChooseDefaultUser();
	// (d.thompson 2012-02-17) - PLID 48194
	CString GetCurrentWinLoginName();

	// (a.walling 2008-04-16 17:42) - PLID 29691 - Animate window function
	HMODULE m_User32;
	AnimateWindowType m_pfnAnimateWindow;

	// (a.walling 2009-12-16 16:07) - PLID 35784 - Are we in the process of logging in?
	bool m_bIsLoggingIn;

	// (a.walling 2007-11-06 17:16) - PLID 27998 - VS2008 - NcHitTest needs an LRESULT

	// (a.walling 2008-04-16 17:44) - PLID 29691 - Add ShowWindow handler for animation
	// Generated message map functions
	//{{AFX_MSG(CLoginDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectionChangeUserNameCmb(long iNewRow);
	afx_msg void OnExitPracBtn();
	afx_msg void OnViewLicense();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnSelSetLocationCombo(long nRow);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	// (a.walling 2010-06-09 16:03) - PLID 39087 - Handle this here in a generic fashion
	afx_msg LRESULT OnPrintClientInRect(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg LRESULT OnLoginProgress(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGINDLG_H__235F14F3_4265_11D2_983A_00104B318376__INCLUDED_)

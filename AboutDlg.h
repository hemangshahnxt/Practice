/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

#pragma once

#include "NxGdiPlusUtils.h"

// (z.manning, 05/15/2008) - PLID 30050 - Inherit from CNxDialog rather than NxDialog
class CAboutDlg : public CNxDialog
{
public:							 
	static CString GetFileVersion (CString path);
	static DWORD GetPracticeVersion();
	static CString GetFileDateAsString(const CString &strFilePathName);

	CAboutDlg(CWnd* pParent);
	bool SetDlgItemVersion(int nID, long nVer);
	bool SetDlgItemFileDate(int nID, const CString &strFilePathName);
	bool SetDlgItemDllFileDate(int nID, const CString &strFileName);
	
	// (a.walling 2008-09-18 17:01) - PLID 28040
	//bool SetDlgItemDatabaseDate(int nID, const CString &strFilePathName);

	BOOL m_bSupportExp;

	// (a.walling 2008-05-27 12:44) - PLID 29673 - Keep the bitmap handy for drawing the practice icon
	Gdiplus::Bitmap* m_pIcon;
	// (d.thompson 2010-03-05) - PLID 37656 - Do the same with the "Software for Better Patient Care" logo
	Gdiplus::Bitmap* m_pBetterCareIcon;

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CNxStatic	m_strSupportExpires;
	CNxEdit	m_nxeditUserNameBox;
	CNxEdit	m_nxeditLocNameBox;
	CNxEdit	m_nxeditMacNameBox;
	CNxEdit	m_nxeditOckPathLabel;
	CNxEdit	m_nxeditLocalPathLabel;
	CNxEdit	m_nxeditNxserver;
	CNxEdit	m_nxeditNextBackupLabel;
	CNxEdit	m_nxeditSharedPathLabel;
	// (a.walling 2012-09-04 17:32) - PLID 52449 - Include session path
	CNxEdit	m_nxeditSessionPathLabel;
	CNxEdit	m_nxeditMailingAddressLabel;
	CNxStatic	m_nxstaticProductVersionLabel;
	CNxStatic	m_nxstaticProductVersionText;
	CNxStatic	m_nxstaticOsVersion;
	CNxStatic	m_nxstaticNumbers;
	CNxStatic	m_nxstaticSqlVersion;
	CNxStatic	m_nxstaticOtherInfo; // (a.walling 2010-01-27 16:21) - PLID 37108 - Box for BSD license-mandated acknowledgements
	CNxIconButton m_btnOK;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
// (s.dhole 2013-07-01 18:32) - PLID 57404
	void RefreshClockDisplay();
	BOOL IsSQLServerLocalHost();
	SYSTEMTIME AddTimeOffset(SYSTEMTIME s,double timeOffset);
	int m_nLastServerTimeSeconds;
	LONGLONG   m_nServerTimeOffset;
	UINT m_nTimerRefresh;
	
	
	
	SYSTEMTIME GetLocalTime(SYSTEMTIME s);
	SYSTEMTIME GetLocalMachineTime();
	COleDateTime m_dtLastLocalDateTime;
	COleDateTime m_dtServerDate;
	
// Implementation
protected:
	// (a.walling 2008-05-27 12:46) - PLID 29673 - Added OnPaint and OnDestroy handlers
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEmail();
	afx_msg void OnDependencies();
	// (a.walling 2012-09-04 17:32) - PLID 52449 - Handle path clicks
	afx_msg void OnClickLocalPath();
	afx_msg void OnClickSharedPath();
	afx_msg void OnClickSessionPath();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBtnBackupManager();
	afx_msg void OnBtnLicense();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	// (a.walling 2012-09-04 17:32) - PLID 52449 - Handle path link cursor
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	// (s.dhole 2013-09-20 10:08) - PLID 57404
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

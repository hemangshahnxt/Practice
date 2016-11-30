#if !defined(AFX_SAVEREPORTDLG_H__1C101DAD_0A13_4556_A11F_3E3540F68753__INCLUDED_)
#define AFX_SAVEREPORTDLG_H__1C101DAD_0A13_4556_A11F_3E3540F68753__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveReportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSaveReportDlg dialog

class CSaveReportDlg : public CNxDialog
{
// Construction
public:
	CSaveReportDlg(CWnd* pParent);   // standard constructor
	CSaveReportDlg(CWnd* pParent, CString strTitle, long nID, bool bIsCustom, bool m_bIsNew);   // standard constructor
	CString m_strTitle;
	bool m_bIsNew;
	long m_nID;
	bool m_bIsCustom;
	bool m_bUpdateTitle;


	// (z.manning, 04/28/2008) - PLID 29807 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CSaveReportDlg)
	enum { IDD = IDD_SAVECUSTOMREPORT };
	CNxEdit	m_nxeditReporttitle;
	CNxIconButton	m_btnSaveReport;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnNoSaveReport;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaveReportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSaveReportDlg)
	afx_msg void OnSavereport();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnNosavereport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVEREPORTDLG_H__1C101DAD_0A13_4556_A11F_3E3540F68753__INCLUDED_)

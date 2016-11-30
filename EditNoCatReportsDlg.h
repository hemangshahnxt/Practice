#if !defined(AFX_EDITNOCATREPORTSDLG_H__08334E91_BFB5_4394_BD5E_1C56F6CCB628__INCLUDED_)
#define AFX_EDITNOCATREPORTSDLG_H__08334E91_BFB5_4394_BD5E_1C56F6CCB628__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditNoCatReportsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditNoCatReportsDlg dialog

class CEditNoCatReportsDlg : public CNxDialog
{
// Construction
public:
	CEditNoCatReportsDlg(CWnd* pParent);   // standard constructor
	NXDATALISTLib::_DNxDataListPtr m_ReportList;
	
	//TS 12/24/02: I moved this to globalreportutils
	//BOOL IsStatement(long nID);

	// (z.manning, 04/28/2008) - PLID 29807 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CEditNoCatReportsDlg1)
	enum { IDD = IDD_EDITNOCATREPORTS };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnEditReport;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditNoCatReportsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditNoCatReportsDlg)
	afx_msg void OnEditnocatreport();
	virtual BOOL OnInitDialog();
	afx_msg void OnCloseeditreportdlg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITNOCATREPORTSDLG_H__08334E91_BFB5_4394_BD5E_1C56F6CCB628__INCLUDED_)

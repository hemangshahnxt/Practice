#if !defined(AFX_EDITREPORTDLG_H__3AE5D493_F6CD_4392_B27A_B11BDF98598D__INCLUDED_)
#define AFX_EDITREPORTDLG_H__3AE5D493_F6CD_4392_B27A_B11BDF98598D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditReportDlg.h : header file
//

#include "ReportInfo.h"
#include "VerifyInstructionsDlg.h"

// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "CRDesignerCtrl.tlb"
//TES 11/7/2007 - PLID 27979 - VS2008 -  FindText is already a macro, and we don't use the imported FindText anyway.
#import "craxdrt.tlb" exclude("IFontDisp") rename("FindText","CRAXDRT_FindText")

/////////////////////////////////////////////////////////////////////////////
// CEditReportDlg dialog

class CEditReportDlg : public CNxDialog
{
// Construction
public:
	CEditReportDlg(CWnd* pParent);   // standard constructor
	CEditReportDlg(CWnd* pParent, CReportInfo *CurrReport);   
	CEditReportDlg(CWnd* pParent, CReportInfo *CurrReport, CString strOpenFileName, CString strSaveFileName, bool bIsCustom, long nNumber, bool bIsNew);
	long SaveReport();
	bool m_bIsCustom;
	CString m_strOpenFileName;
	CString m_strSaveFileName;
	bool  m_bSaved;
	bool m_bIsNew;

	long m_nCustomReportID;

private:

	CReportInfo *m_CurrReport;
	long m_nTop;
	long m_nBottom;
	CPoint m_ptSaveBtn;
	CPoint m_ptVerifyBtn;
	CRDESIGNERCTRLLib::ICRDesignerCtrlPtr m_designer; 		
	CRAXDRT::IApplicationPtr  m_Application;
	CRAXDRT::IReportPtr  m_Report;
	CVerifyInstructionsDlg  m_InstructDlg;



	

	// (z.manning, 04/28/2008) - PLID 29807 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CEditReportDlg)
	enum { IDD = IDD_EDITREPORT };
	CNxIconButton	m_btnSave;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditReportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditReportDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSavereport();
	afx_msg void OnClose();
	afx_msg void OnVerifyreport();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITREPORTDLG_H__3AE5D493_F6CD_4392_B27A_B11BDF98598D__INCLUDED_)

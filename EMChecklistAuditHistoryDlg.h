#if !defined(AFX_EMCHECKLISTAUDITHISTORYDLG_H__DF2C89FA_6141_41F1_9C26_A932CAF96218__INCLUDED_)
#define AFX_EMCHECKLISTAUDITHISTORYDLG_H__DF2C89FA_6141_41F1_9C26_A932CAF96218__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMChecklistAuditHistoryDlg.h : header file
//

// (j.jones 2007-08-24 10:51) - PLID 27152 - created

/////////////////////////////////////////////////////////////////////////////
// CEMChecklistAuditHistoryDlg dialog

class CEMChecklistAuditHistoryDlg : public CNxDialog
{
// Construction
public:
	CEMChecklistAuditHistoryDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_AuditList;

	long m_nChecklistID;

// Dialog Data
	//{{AFX_DATA(CEMChecklistAuditHistoryDlg)
	enum { IDD = IDD_EM_CHECKLIST_AUDIT_HISTORY };
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCopyOutput; // (b.cardillo 2010-01-07 13:26) - PLID 35780
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMChecklistAuditHistoryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEMChecklistAuditHistoryDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCopyOutputBtn(); // (b.cardillo 2010-01-07 13:26) - PLID 35780
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMCHECKLISTAUDITHISTORYDLG_H__DF2C89FA_6141_41F1_9C26_A932CAF96218__INCLUDED_)

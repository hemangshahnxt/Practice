#if !defined(AFX_PASTSUPERBILLSDLG_H__DD080290_99DF_4BA7_B0C3_0EDCAFF263EE__INCLUDED_)
#define AFX_PASTSUPERBILLSDLG_H__DD080290_99DF_4BA7_B0C3_0EDCAFF263EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PastSuperbillsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPastSuperbillsDlg dialog
//TES 9/3/2008 - PLID 31222 - Created
class CPastSuperbillsDlg : public CNxDialog
{
// Construction
public:
	CPastSuperbillsDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPastSuperbillsDlg)
	enum { IDD = IDD_PAST_SUPERBILLS_DLG };
	CNxIconButton	m_nxbPrint;
	CNxIconButton	m_nxbOpen;
	CNxIconButton	m_nxbClose;
	CDateTimePicker	m_dtpPrintedDateTo;
	CDateTimePicker	m_dtpPrintedDateFrom;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPastSuperbillsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pPastSuperbillsList;

	//TES 9/3/2008 - PLID 31222 - Requeries the list of superbills based on the current date range.
	void RefreshSuperbillsList();

	// Generated message map functions
	//{{AFX_MSG(CPastSuperbillsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClosePastSuperbills();
	afx_msg void OnDatetimechangePrintedDateFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangePrintedDateTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelChangingPastSuperbillsList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangedPastSuperbillsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnOpenSelected();
	afx_msg void OnPrintSelected();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PASTSUPERBILLSDLG_H__DD080290_99DF_4BA7_B0C3_0EDCAFF263EE__INCLUDED_)

#if !defined(AFX_VOIDSUPERBILLDLG_H__4979A38B_B87F_4B61_83D1_DDFCDD78343C__INCLUDED_)
#define AFX_VOIDSUPERBILLDLG_H__4979A38B_B87F_4B61_83D1_DDFCDD78343C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VoidSuperbillDlg.h : header file
//

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CVoidSuperbillDlg dialog

class CVoidSuperbillDlg : public CNxDialog
{
// Construction
public:
	CVoidSuperbillDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CVoidSuperbillDlg)
	enum { IDD = IDD_SUPERBILL_VOID_DLG };
	CDateTimePicker	m_dtpFrom;
	CDateTimePicker	m_dtpTo;
	CNxIconButton	m_btnVoid;
	CNxIconButton	m_btnUnvoid;
	CNxIconButton	m_btnClose;
	NxButton	m_btnHideVoid;
	NxButton	m_btnIncludeApplied;
	NxButton	m_btnDateFilter;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVoidSuperbillDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_listSuperbills;
	void RequeryWithFilters();

	// Generated message map functions
	//{{AFX_MSG(CVoidSuperbillDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDateFilter();
	afx_msg void OnVoid();
	afx_msg void OnChangeTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUnvoid();
	afx_msg void OnIncludeApplied();
	afx_msg void OnRButtonDownListSuperbills(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnMarkVoid();
	afx_msg void OnMarkUnvoid();
	afx_msg void OnHideVoid();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VOIDSUPERBILLDLG_H__4979A38B_B87F_4B61_83D1_DDFCDD78343C__INCLUDED_)

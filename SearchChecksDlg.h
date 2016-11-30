#if !defined(AFX_SEARCHCHECKSDLG_H__DC58028E_AAA8_422A_8CD6_316714FC0513__INCLUDED_)
#define AFX_SEARCHCHECKSDLG_H__DC58028E_AAA8_422A_8CD6_316714FC0513__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SearchChecksDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSearchChecksDlg dialog

class CSearchChecksDlg : public CNxDialog
{
// Construction
public:
	CSearchChecksDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSearchChecksDlg)
	enum { IDD = IDD_SEARCH_CHECKS_DLG };
	NxButton	m_btnSearchChecks;
	NxButton	m_btnCheckFilter;
	NxButton	m_btnFilterPatient;
	NxButton	m_btnSearchCC;
	CNxEdit	m_nxeditTextFilterSearch;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSearchChecksDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pPaymentList;
	NXDATALISTLib::_DNxDataListPtr m_pPatientList;

	void RequeryPaymentsWithFilters();
	void EnsureControls();

	// Generated message map functions
	//{{AFX_MSG(CSearchChecksDlg)
	afx_msg void OnTextFilterGo();
	afx_msg void OnSelChosenPatientFilterList(long nRow);
	afx_msg void OnSearchCc();
	afx_msg void OnSearchChecks();
	virtual BOOL OnInitDialog();
	afx_msg void OnPatientFilter();
	afx_msg void OnTextFilter();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnCloseButton();
	afx_msg void OnRButtonDownPayList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEARCHCHECKSDLG_H__DC58028E_AAA8_422A_8CD6_316714FC0513__INCLUDED_)

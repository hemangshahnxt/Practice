#if !defined(AFX_CSearchReportDescDlg_H__2726141B_F9F6_47A5_9B5A_77D262B3764A__INCLUDED_)
#define AFX_CSearchReportDescDlg_H__2726141B_F9F6_47A5_9B5A_77D262B3764A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CSearchReportDescDlg.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CSearchReportDescDlg dialog

class CSearchReportDescDlg : public CNxDialog
{
// Construction
public:
	CSearchReportDescDlg(CWnd* pParent);   // standard constructor
	CView* m_pReportView;

	// (z.manning, 04/28/2008) - pLID 29807 - Added NxIconButton
// Dialog Data
	//{{AFX_DATA(CSearchReportDescDlg)
	enum { IDD = IDD_SEARCH_REPORT_DESC };
	CNxEdit	m_nxeditSearchText;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSearchReportDescDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pReportList;
	CStringArray m_arySearchParts;
	void AddReport(long nRepID);
	CString GetTabNameFromCategory(CString strCat);
	void BuildSearchArray(CString strSearchString);
	void ClearSearchArray();

	// Generated message map functions
	//{{AFX_MSG(CSearchReportDescDlg)
	afx_msg void OnSearchButton();
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblClickCellReportDescList(long nRowIndex, short nColIndex);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRButtonDownReportDescList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CSearchReportDescDlg_H__2726141B_F9F6_47A5_9B5A_77D262B3764A__INCLUDED_)

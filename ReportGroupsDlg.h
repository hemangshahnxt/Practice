#if !defined(AFX_REPORTGROUPSDLG_H__7D28F953_CD8E_496C_ADF1_C79DA2CF2477__INCLUDED_)
#define AFX_REPORTGROUPSDLG_H__7D28F953_CD8E_496C_ADF1_C79DA2CF2477__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ReportGroupsDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CReportGroupsDlg dialog

class CReportGroupsDlg : public CNxDialog
{
// Construction
public:
	CReportGroupsDlg(CWnd* pParent);   // standard constructor
	NXDATALISTLib::_DNxDataListPtr m_pGroupList;
	NXDATALISTLib::_DNxDataListPtr m_pAvailList;
	NXDATALISTLib::_DNxDataListPtr m_pCurrentList;
	NXDATALISTLib::_DNxDataListPtr m_pTabList;


	void MoveOneLeft();
	void MoveOneRight(); 

	void ShowNewReportsMessage();
// Dialog Data
	//{{AFX_DATA(CReportGroupsDlg)
	enum { IDD = IDD_REPORT_GROUPS_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnRename;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnMoveOneRight;
	CNxIconButton	m_btnMoveOneLeft;
	CNxIconButton	m_btnMoveAllRight;
	CNxIconButton	m_btnMoveAllLeft;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReportGroupsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReportGroupsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedReportGroupList(short nFlags);
	afx_msg void OnSelChosenReportGroupList(long nRow);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnCloseReportGroups();
	afx_msg void OnAddReportGroup();
	afx_msg void OnDeleteReportGroup();
	afx_msg void OnDefaultCheck();
	virtual void OnCancel();
	afx_msg void OnMoveRight();
	afx_msg void OnMoveLeft();
	afx_msg void OnMoveAllRight();
	afx_msg void OnMoveAllLeft();
	afx_msg void OnSelChosenTabList(long nRow);
	afx_msg void OnDblClickCellReportsForGroup(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellAllReports(long nRowIndex, short nColIndex);
	afx_msg void OnRenameReportGroup();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REPORTGROUPSDLG_H__7D28F953_CD8E_496C_ADF1_C79DA2CF2477__INCLUDED_)

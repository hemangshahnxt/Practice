#if !defined(AFX_LOGMANAGEMENTDLG_H__80FF5A7C_2E6C_4949_A7F2_16584A1D9FBD__INCLUDED_)
#define AFX_LOGMANAGEMENTDLG_H__80FF5A7C_2E6C_4949_A7F2_16584A1D9FBD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogManagementDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLogManagementDlg dialog

class CLogManagementDlg : public CNxDialog
{

// PLID 26192	6/9/08	r.galicki	-	Added "Add New Entry" button
// Construction
public:
	long m_nUserID;
	CLogManagementDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLogManagementDlg)
	enum { IDD = IDD_LOG_MANAGEMENT };
	CNxIconButton	m_btnAddEntry;
	NxButton	m_btnAllDates;
	NxButton	m_btnPromptLog;
	NxButton	m_btnDateRangeOptions;
	CDateTimePicker	m_dtpFromCtrl;
	CDateTimePicker	m_dtpToCtrl;
	CNxStatic	m_nxstaticLabelFromdate;
	CNxStatic	m_nxstaticLabelTodate;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogManagementDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pTimeList;
	NXDATALISTLib::_DNxDataListPtr m_pUserList;
	void RequeryTime();
	void DeleteLog();
	void AddLog();
	void EditLog();

	// Generated message map functions
	//{{AFX_MSG(CLogManagementDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAlldates();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenUserList(long nRow);
	afx_msg void OnDateRangeOptions();
	afx_msg void OnRequeryFinishedUserList(short nFlags);
	afx_msg void OnPromptLog();
	afx_msg void OnRButtonUpTimeList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnDblClickCellTimeList(long nRowIndex, short nColIndex);
	afx_msg void OnBtnAddNew();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGMANAGEMENTDLG_H__80FF5A7C_2E6C_4949_A7F2_16584A1D9FBD__INCLUDED_)

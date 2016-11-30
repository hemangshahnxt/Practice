#if !defined(AFX_PENDINGCASEHISTORIESDLG_H__A991AFE9_9BB8_47D2_8C9B_0A34F6CD8FA1__INCLUDED_)
#define AFX_PENDINGCASEHISTORIESDLG_H__A991AFE9_9BB8_47D2_8C9B_0A34F6CD8FA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PendingCaseHistoriesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPendingCaseHistoriesDlg dialog

class CPendingCaseHistoriesDlg : public CNxDialog
{
// Construction
public:
	CPendingCaseHistoriesDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_List;

	long m_nProductID;
	long m_nLocationID;

// Dialog Data
	//{{AFX_DATA(CPendingCaseHistoriesDlg)
	enum { IDD = IDD_PENDING_CASE_HISTORIES_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxStatic	m_nxstaticProductName;
	CNxStatic	m_nxstaticLocationName;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPendingCaseHistoriesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPendingCaseHistoriesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnLeftClickPendingCasesList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PENDINGCASEHISTORIESDLG_H__A991AFE9_9BB8_47D2_8C9B_0A34F6CD8FA1__INCLUDED_)

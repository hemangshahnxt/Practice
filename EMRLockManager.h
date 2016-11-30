#if !defined(AFX_EMRLOCKMANAGER_H__E0724837_5CAD_4E5D_B31B_C168ABDB2A60__INCLUDED_)
#define AFX_EMRLOCKMANAGER_H__E0724837_5CAD_4E5D_B31B_C168ABDB2A60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRLockManager.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEMRLockManager dialog

#include "EmrRc.h"

class CEMRLockManager : public CNxDialog
{
// Construction
public:
	CEMRLockManager(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEMRLockManager)
	enum { IDD = IDD_LOCKMANAGER };
	CNxIconButton	m_btnApply;
	CNxIconButton	m_btnOK;
	CNxStatic	m_nxstaticEmrlockStatus;
	// (j.jones 2011-07-08 12:02) - PLID 42878 - added a refresh button
	CNxIconButton	m_btnRefresh;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRLockManager)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	// (z.manning 2011-05-20 11:13) - PLID 33114 - Removed this variable at it served no real purpose only made the code more confusing
	//CString m_strWhere;
	long m_nDays;

	HICON m_hIconPreview; // (a.walling 2010-01-11 12:11) - PLID 31482
	class CEMRPreviewPopupDlg* m_pEMRPreviewPopupDlg;

	void AppendWhereClause(); // append the aging clause to the where clause (ie days > 30)
	long NumSelected(); // number of selected items
	long NumChecked(); // number of checked items
	// (z.manning 2016-01-13 15:33) - PLID 67778 - Changed return type to boolean
	BOOL LockSelected(); // lock the selected EMNs!
	void GotoPatient(long nPatID); // set the patient module to browse to this patient

	// (a.walling 2010-01-11 12:52) - PLID 31482 - Show the emn preview popup
	// (z.manning 2012-09-10 15:42) - PLID 52543 - Added modified date
	void ShowPreview(long nPatID, long nEMNID, COleDateTime dtEmnModifiedDate);

	// (z.manning 2011-06-24 17:49) - PLID 35566
	void ToggleSecondaryProviderColum();

	// Generated message map functions
	//{{AFX_MSG(CEMRLockManager)
	virtual BOOL OnInitDialog();
	afx_msg void OnRButtonDownEmrLockList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnCheckSelected();
	afx_msg void OnUncheckSelected();
	afx_msg void OnGotoPatient();
	afx_msg void OnGotoEMN();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRequeryFinishedEmrLockList(short nFlags);
	afx_msg void OnLeftClickEmrLockList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnApply();
	// (a.walling 2010-01-11 12:25) - PLID 31482	
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedLockManagerShowSecondaryProvider(); // (z.manning 2011-06-24 17:37) - PLID 35566
	// (j.jones 2011-07-08 12:02) - PLID 42878 - added a refresh button
	afx_msg void OnBtnRefreshLockMgr();
	afx_msg void OnSize(UINT nType, int cx, int cy); // (r.goldschmidt 2014-07-15 16:12) - PLID 62649 - Make the EMR Lock Manager resizable
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRLOCKMANAGER_H__E0724837_5CAD_4E5D_B31B_C168ABDB2A60__INCLUDED_)

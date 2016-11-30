#if !defined(AFX_SCHEDULERSTATUSDLG_H__415705E4_0C12_43A5_B589_279F5DFF8176__INCLUDED_)
#define AFX_SCHEDULERSTATUSDLG_H__415705E4_0C12_43A5_B589_279F5DFF8176__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SchedulerStatusDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSchedulerStatusDlg dialog

class CSchedulerStatusDlg : public CNxDialog
{
// Construction
public:
	CSchedulerStatusDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSchedulerStatusDlg)
	enum { IDD = IDD_SCHEDULER_STATUS_DLG };
	CNxIconButton	m_btnAddStatus;
	CNxIconButton	m_btnDeleteStatus;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSchedulerStatusDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_dlStatus;
	CDWordArray m_adwEditedStatusIDs;
	CDWordArray m_adwDeletedStatusIDs;
	CStringArray m_astrDeletedStatusNames;

	BOOL IsSystemStatus(long nID);

	// Generated message map functions
	//{{AFX_MSG(CSchedulerStatusDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedMultipurposeDurationList(short nFlags);
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnDelete();
	afx_msg void OnEditingFinishingMultipurposeDurationList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnEditingFinishedMultipurposeDurationList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingStartingMultipurposeDurationList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnLeftClickMultipurposeDurationList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCHEDULERSTATUSDLG_H__415705E4_0C12_43A5_B589_279F5DFF8176__INCLUDED_)

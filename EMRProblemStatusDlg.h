#if !defined(AFX_EMRPROBLEMSTATUSDLG_H__3FAC1C94_6058_4A17_8F4A_7B615BF6F1F7__INCLUDED_)
#define AFX_EMRPROBLEMSTATUSDLG_H__3FAC1C94_6058_4A17_8F4A_7B615BF6F1F7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRProblemStatusDlg.h : header file
//
#include "AdministratorRc.h"

/////////////////////////////////////////////////////////////////////////////
// CEMRProblemStatusDlg dialog

class CEMRProblemStatusDlg : public CNxDialog
{
// Construction
public:
	CEMRProblemStatusDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEMRProblemStatusDlg)
	enum { IDD = IDD_EMR_PROBLEM_STATUS_DLG };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnRemove;
	CNxIconButton	m_btnAdd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRProblemStatusDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL m_bModified;

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlStatus;

protected:
	CNxColor m_nxcTop;

protected:
	CArray<long, long> m_anDeletedStatus;

protected:
	long m_nParentStatusID;

public:
	long GetParentStatusID() const;
	void SetParentStatusID(long nParentStatusID);

protected:
	BOOL DoesStatusExist(const CString& strStatus);
	void Save();

	// Generated message map functions
	//{{AFX_MSG(CEMRProblemStatusDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAddStatus();
	afx_msg void OnSelChangedListMasterProblemStatus(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnBtnRemoveStatus();
	virtual void OnOK();
	afx_msg void OnEditingFinishedListMasterProblemStatus(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedListMasterProblemStatus(short nFlags);
	afx_msg void OnEditingStartingListMasterProblemStatus(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRPROBLEMSTATUSDLG_H__3FAC1C94_6058_4A17_8F4A_7B615BF6F1F7__INCLUDED_)

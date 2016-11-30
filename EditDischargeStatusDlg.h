#if !defined(AFX_EDITDISCHARGESTATUSDLG_H__41D02174_7FA4_4371_94E4_C2E44CAD87DE__INCLUDED_)
#define AFX_EDITDISCHARGESTATUSDLG_H__41D02174_7FA4_4371_94E4_C2E44CAD87DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditDischargeStatusDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditDischargeStatusDlg dialog

class CEditDischargeStatusDlg : public CNxDialog
{
// Construction
public:
	CEditDischargeStatusDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_DischargeStatusList;

// Dialog Data
	//{{AFX_DATA(CEditDischargeStatusDlg)
	enum { IDD = IDD_EDIT_DISCHARGE_STATUS_DLG };
	CNxIconButton	m_btnAddDischargeStatus;
	CNxIconButton	m_btnDeleteDischargeStatus;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditDischargeStatusDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditDischargeStatusDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAddStatus();
	afx_msg void OnBtnDeleteStatus();
	afx_msg void OnEditingFinishingDischargeStatusList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedDischargeStatusList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITDISCHARGESTATUSDLG_H__41D02174_7FA4_4371_94E4_C2E44CAD87DE__INCLUDED_)

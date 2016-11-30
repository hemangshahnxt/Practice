#if !defined(AFX_PHASESTATUSDLG_H__BD10EF66_954C_41C2_882C_621F2444D150__INCLUDED_)
#define AFX_PHASESTATUSDLG_H__BD10EF66_954C_41C2_882C_621F2444D150__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhaseStatusDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPhaseStatusDlg dialog

class CPhaseStatusDlg : public CNxDialog
{
// Construction
public:
	CPhaseStatusDlg(CWnd* pParent);   // standard constructor

	// (z.manning, 05/01/2008) - PLID 29864 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CPhaseStatusDlg)
	enum { IDD = IDD_PHASE_STATUS_DLG };
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhaseStatusDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_statusList;
	// Generated message map functions
	//{{AFX_MSG(CPhaseStatusDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishedStatusList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingStatusList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingStartingStatusList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	virtual void OnCancel();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHASESTATUSDLG_H__BD10EF66_954C_41C2_882C_621F2444D150__INCLUDED_)

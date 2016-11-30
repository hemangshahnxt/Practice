#if !defined(AFX_PROCEDURELADDERASSIGNMENT_H__8CCC0C6B_9672_438C_9AA0_86096219BE55__INCLUDED_)
#define AFX_PROCEDURELADDERASSIGNMENT_H__8CCC0C6B_9672_438C_9AA0_86096219BE55__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcedureLadderAssignment.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProcedureLadderAssignment dialog

// (d.moore 2007-08-24) - PLID 23497 - This dialog lets the user assign a ladder
//  to multiple procedures at once.

class CProcedureLadderAssignment : public CNxDialog
{
// Construction
public:
	CProcedureLadderAssignment(CWnd* pParent);   // standard constructor

	void SetSelectedProcedure(long nProcID);
	long GetSelectedProcedure();


// Dialog Data
	//{{AFX_DATA(CProcedureLadderAssignment)
	enum { IDD = IDD_PROCEDURE_LADDER_ASSIGNMENT };
	CNxLabel	m_lblProcedure;
	CNxLabel	m_lblAvailable;
	CNxLabel	m_lblSelected;
	CNxIconButton	m_btnMoveProcRight;
	CNxIconButton	m_btnMoveProcLeft;
	CNxIconButton	m_btnSelLadder;
	CNxIconButton	m_btnDeselLadder;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcedureLadderAssignment)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	
	long m_nProcedureID;

	NXDATALIST2Lib::_DNxDataListPtr m_pProcedureList;
	NXDATALIST2Lib::_DNxDataListPtr m_pAvailLaderList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelLadderList;

	void EnableControls(BOOL bEnable);
	void RequeryLadderLists();

	// Generated message map functions
	//{{AFX_MSG(CProcedureLadderAssignment)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelLadder();
	afx_msg void OnDeselLadder();
	afx_msg void OnRequeryFinishedDlProcedures(short nFlags);
	afx_msg void OnSelChangedDlProcedures(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnClose();
	afx_msg void OnMoveProcLeft();
	afx_msg void OnMoveProcRight();
	afx_msg void OnDblClickCellDlAvailLadders(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellDlSelectedLadders(LPDISPATCH lpRow, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	//(e.lally 2010-07-28) PLID 36199
	void EditingFinishedDlSelectedLadders(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishingDlSelectedLadders(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCEDURELADDERASSIGNMENT_H__8CCC0C6B_9672_438C_9AA0_86096219BE55__INCLUDED_)

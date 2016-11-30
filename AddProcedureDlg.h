#if !defined(AFX_ADDPROCEDUREDLG_H__E29AB405_43D1_4649_B847_6475DB09B040__INCLUDED_)
#define AFX_ADDPROCEDUREDLG_H__E29AB405_43D1_4649_B847_6475DB09B040__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddProcedureDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddProcedureDlg dialog

class CAddProcedureDlg : public CNxDialog
{
// Construction
public:
	CAddProcedureDlg(CWnd* pParent);
	CArray<int, int> m_arProcIDs;
	CArray<int, int> m_arProcGroupIDs;
	CArray<CString, CString> m_arProcString;
	CArray<CString, CString> m_arProcGroupString;	
	bool m_bProcedures; //Did they select procedures? (as opposed to procedure groups)
	bool m_bAllowProcGroups; //Will we allow them to select procedure groups? 
	CString m_strProcWhereClause; //If you pass in a where clause, it will override any other where clause.
	CString m_strGroupWhereClause; //If you pass in a where clause, it will override any other where clause.

	virtual void Refresh();
	// Dialog Data
	//{{AFX_DATA(CAddProcedureDlg)
	enum { IDD = IDD_ADD_PROCEDURE_DLG };
	NxButton	m_btnProcedures;
	NxButton	m_btnProcedureGroups;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddProcedureDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_procedureList;
	NXDATALISTLib::_DNxDataListPtr m_pProcGroupList;
	void UpdateArray();
	// Generated message map functions
	//{{AFX_MSG(CAddProcedureDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnLeftClickProcedureList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnProcGroups();
	afx_msg void OnProcedures();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void LoadFromArray();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDPROCEDUREDLG_H__E29AB405_43D1_4649_B847_6475DB09B040__INCLUDED_)

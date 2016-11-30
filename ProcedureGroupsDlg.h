#if !defined(AFX_PROCEDUREGROUPSDLG_H__1BA06E32_DD8E_44A8_827C_1477A640F59F__INCLUDED_)
#define AFX_PROCEDUREGROUPSDLG_H__1BA06E32_DD8E_44A8_827C_1477A640F59F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcedureGroupsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProcedureGroupsDlg dialog

class CProcedureGroupsDlg : public CNxDialog
{
// Construction
public:
	CProcedureGroupsDlg(CWnd* pParent);   // standard constructor

	virtual void Refresh();
	void EnableAppropriateWindows();

	long m_nProcGroupID;

// Dialog Data
	//{{AFX_DATA(CProcedureGroupsDlg)
	enum { IDD = IDD_PROCEDURE_GROUPS };
	CNxIconButton	m_btSel;
	CNxIconButton	m_btDesel;
	CNxIconButton	m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcedureGroupsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALISTLib::_DNxDataListPtr m_pProcGroups;
	NXDATALISTLib::_DNxDataListPtr m_pAvailProcs;
	NXDATALISTLib::_DNxDataListPtr m_pSelectProcs;
	NXDATALISTLib::_DNxDataListPtr m_pLadderTemplates;


	// Generated message map functions
	//{{AFX_MSG(CProcedureGroupsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangedProcGroupCombo(long nNewSel);
	afx_msg void OnSelProc();
	afx_msg void OnDeselProc();
	afx_msg void OnDblClickCellAvailProcs(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectProcs(long nRowIndex, short nColIndex);
	afx_msg void OnSelChosenLadder(long nRow);
	afx_msg void OnEditProcGroups();
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCEDUREGROUPSDLG_H__1BA06E32_DD8E_44A8_827C_1477A640F59F__INCLUDED_)

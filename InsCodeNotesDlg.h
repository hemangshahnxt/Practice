#if !defined(AFX_INSCODENOTESDLG_H__3AB46639_5E36_409A_BB85_58CB6EE9851A__INCLUDED_)
#define AFX_INSCODENOTESDLG_H__3AB46639_5E36_409A_BB85_58CB6EE9851A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InsCodeNotesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInsCodeNotesDlg dialog

class CInsCodeNotesDlg : public CNxDialog
{
// Construction
public:
	CInsCodeNotesDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInsCodeNotesDlg)
	enum { IDD = IDD_INS_CODE_NOTES_DLG };
	NxButton	m_btnSvcNotes;
	NxButton	m_btnDiagNotes;
	CNxIconButton	m_btnRemoveAllCode;
	CNxIconButton	m_btnRemoveCode;
	CNxIconButton	m_btnAddAllCode;
	CNxIconButton	m_btnAddCode;
	CNxIconButton	m_btnRemoveAllInsCo;
	CNxIconButton	m_btnRemoveInsCo;
	CNxIconButton	m_btnAddAllInsCo;
	CNxIconButton	m_btnAddInsCo;
	CNxEdit	m_nxeditInsCodeNote;
	CNxStatic	m_nxstaticUnselectedCodeLabel;
	CNxStatic	m_nxstaticSelectedCodeLabel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnApplyCodeNote;
	NxButton	m_btnInsCptGroupbox;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsCodeNotesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_pInsCoAvail, m_pInsCoSelect, m_pCodeAvail, m_pCodeSelect;
	// (a.wilson 02/18/2014) PLID 60770 - add diagnosis search.
	NXDATALIST2Lib::_DNxDataListPtr m_pDiagnosisSearch;
	bool m_bShowingServiceCodes;

	// Generated message map functions
	//{{AFX_MSG(CInsCodeNotesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnApplyCodeNote();
	afx_msg void OnServiceCodeNotes();
	afx_msg void OnDiagCodeNotes();
	afx_msg void OnNotesAddInsCo();
	afx_msg void OnNotesAddAllInsCo();
	afx_msg void OnNotesRemoveInsCo();
	afx_msg void OnNotesRemoveAllInsCo();
	afx_msg void OnNotesAddCode();
	afx_msg void OnNotesAddAllCode();
	afx_msg void OnNotesRemoveCode();
	afx_msg void OnNotesRemoveAllCode();
	afx_msg void OnSelChangedCodeNotesUnselectedIns(long nNewSel);
	afx_msg void OnSelChangedCodeNotesSelectedIns(long nNewSel);
	afx_msg void OnSelChangedUnselectedCodeList(long nNewSel);
	afx_msg void OnSelChangedSelectedCodeList(long nNewSel);
	afx_msg void OnDblClickCellCodeNotesUnselectedIns(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellCodeNotesSelectedIns(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellSelectedCodeList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellUnselectedCodeList(long nRowIndex, short nColIndex);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void SelChosenCodeNotesDiagnosisSearch(LPDISPATCH lpRow);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSCODENOTESDLG_H__3AB46639_5E36_409A_BB85_58CB6EE9851A__INCLUDED_)

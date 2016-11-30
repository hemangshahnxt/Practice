#if !defined(AFX_INACTIVATEMULTIPLEPATIENTSDLG_H__A1B41749_2131_4E21_8CED_6F3E1DD4A507__INCLUDED_)
#define AFX_INACTIVATEMULTIPLEPATIENTSDLG_H__A1B41749_2131_4E21_8CED_6F3E1DD4A507__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InactivateMultiplePatientsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CInactivateMultiplePatientsDlg dialog

class CInactivateMultiplePatientsDlg : public CNxDialog
{
// Construction
public:
	CInactivateMultiplePatientsDlg(CWnd* pParent);   // standard constructor
	NXDATALISTLib::_DNxDataListPtr  m_pFilterList;
	NXDATALISTLib::_DNxDataListPtr  m_pInactivateList;
	NXDATALISTLib::_DNxDataListPtr  m_pIntervalList;
	NXDATALISTLib::_DNxDataListPtr  m_pUserList;

	BOOL AddNewFilter();
	void InactivatePatients();
	BOOL ValidateData();
	void SaveRemindSettings();
	CBrush m_brush;

// Dialog Data
	//{{AFX_DATA(CInactivateMultiplePatientsDlg)
	enum { IDD = IDD_INACTIVATE_MULTI_PATS };
	NxButton	m_btnRemindMonthly;
	CNxEdit	m_nxeditPatsInList;
	CNxIconButton	m_btnAddToInactiveList;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnEditFilterList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInactivateMultiplePatientsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInactivateMultiplePatientsDlg)
	afx_msg void OnRButtonUpPatsToInactivateList(long nRow, short nCol, long x, long y, long nFlags);
	virtual void OnOK();
	afx_msg void OnEditFilter();
	afx_msg void OnSelChosenFilterList(long nRow);
	afx_msg void OnRequeryFinishedFilterList(short nFlags);
	virtual BOOL OnInitDialog();
	afx_msg void OnRemove();
	afx_msg void OnAddToInactivateList();
	afx_msg void OnRemindMonthly();
	afx_msg void OnRequeryFinishedUserRemindList(short nFlags);
	virtual void OnCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INACTIVATEMULTIPLEPATIENTSDLG_H__A1B41749_2131_4E21_8CED_6F3E1DD4A507__INCLUDED_)

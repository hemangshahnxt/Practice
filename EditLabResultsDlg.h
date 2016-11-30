#if !defined(AFX_EDITLABRESULTSDLG_H__1000A836_4C6E_4BC8_9830_59654109577B__INCLUDED_)
#define AFX_EDITLABRESULTSDLG_H__1000A836_4C6E_4BC8_9830_59654109577B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditLabResultsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditLabResultsDlg dialog

class CEditLabResultsDlg : public CNxDialog
{
// Construction
public:
	CEditLabResultsDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2007-07-20 12:59) - PLID 26749 - added variable to track the current selection
	long m_nCurResultIDInUse;

// Dialog Data
	//{{AFX_DATA(CEditLabResultsDlg)
	enum { IDD = IDD_EDIT_LAB_RESULTS };
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnEdit;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnMoveUp; //TES 7/31/2013 - PLID 54573
	CNxIconButton	m_btnMoveDown; //TES 7/31/2013 - PLID 54573
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditLabResultsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pResultList;

	//TES 7/31/2013 - PLID 54573 - Enable/disable buttons based on the current selection.
	void UpdateControls();

	// Generated message map functions
	//{{AFX_MSG(CEditLabResultsDlg)
	afx_msg void OnAddResult();
	afx_msg void OnDeleteResult();
	afx_msg void OnEditResult();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnEditingFinishingLabResultList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedLabResultList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	// (j.jones 2007-07-20 12:53) - PLID 26749 - added ability to disallow changes to data
	afx_msg void OnEditingStartingLabResultList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnSelChangedLabResultList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnMoveFlagUp();
	afx_msg void OnMoveFlagDown();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITLABRESULTSDLG_H__1000A836_4C6E_4BC8_9830_59654109577B__INCLUDED_)

#if !defined(AFX_CONFIGUREIMPLEMENTATIONLADDERDLG_H__DB0AE9EA_C180_41CA_9861_30D4E23ADA2E__INCLUDED_)
#define AFX_CONFIGUREIMPLEMENTATIONLADDERDLG_H__DB0AE9EA_C180_41CA_9861_30D4E23ADA2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigureImplementationLadderDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CConfigureImplementationLadderDlg dialog

class CConfigureImplementationLadderDlg : public CNxDialog
{
// Construction
public:
	CConfigureImplementationLadderDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigureImplementationLadderDlg)
	enum { IDD = IDD_CONFIGURE_IMPLEMENTATION_LADDER_DLG };
	CNxIconButton	m_btnCopy;
	CNxIconButton	m_btnEditLadder;
	CNxIconButton	m_btn_OK;
	CNxIconButton	m_btn_Up;
	CNxIconButton	m_btn_NewStep;
	CNxIconButton	m_btn_AddLadder;
	CNxIconButton	m_btn_EditStep;
	CNxIconButton	m_btn_Down;
	CNxIconButton	m_btn_DeleteStep;
	CNxIconButton	m_btn_DeleteLadder;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigureImplementationLadderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;
	NXDATALIST2Lib::_DNxDataListPtr m_pLadderTemplateList;
	NXDATALIST2Lib::_DNxDataListPtr m_pLadderTemplateTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pStepList;

	void Load();
	void CheckButtonStatus();

	// Generated message map functions
	// (j.gruber 2007-12-11 11:12) - PLID 28324 - add ability to edit ladder name
	// (j.gruber 2008-05-22 10:39) - PLID 28611 - added ability to copy a ladder template
	//{{AFX_MSG(CConfigureImplementationLadderDlg)
	afx_msg void OnNewLadder();
	afx_msg void OnDeleteLadder();
	afx_msg void OnDeleteStep();
	afx_msg void OnDown();
	afx_msg void OnEditStep();
	afx_msg void OnUp();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnNewImplementationStepTemplate();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelChangingImplementationLadderList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenImplementationLadderList(LPDISPATCH lpRow);
	afx_msg void OnSelChosenLadderStepsList(LPDISPATCH lpRow);
	afx_msg void OnSelChangingLadderStepsList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenLadderType(LPDISPATCH lpRow);
	afx_msg void OnSelChangingLadderType(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnRequeryFinishedImplementationLadderList(short nFlags);
	afx_msg void OnSelChangedLadderStepsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnEditImplementationLadderTemplate();
	afx_msg void OnCopyImplementationLadderTemplate();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGUREIMPLEMENTATIONLADDERDLG_H__DB0AE9EA_C180_41CA_9861_30D4E23ADA2E__INCLUDED_)

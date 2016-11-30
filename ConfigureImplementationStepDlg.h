#if !defined(AFX_CONFIGUREIMPLEMENTATIONSTEPDLG_H__305A60CD_7F7E_4F5B_B45C_1BC5CF6FDE17__INCLUDED_)
#define AFX_CONFIGUREIMPLEMENTATIONSTEPDLG_H__305A60CD_7F7E_4F5B_B45C_1BC5CF6FDE17__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConfigureImplementationStepDlg.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CConfigureImplementationStepDlg dialog

class CConfigureImplementationStepDlg : public CNxDialog
{
// Construction
public:
	CConfigureImplementationStepDlg(long nStepID, long nLadderID, bool bIsTemplate, CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CConfigureImplementationStepDlg)
	enum { IDD = IDD_CONFIGURE_IMPLEMENTATION_STEP_DLG };
	CNxLabel	m_nxlActionLabel;
	CNxIconButton	m_btn_Cancel;
	CNxIconButton	m_btn_OK;
	CNxEdit	m_nxeditStep;
	CNxEdit	m_nxeditName;
	CNxEdit	m_nxeditNote;
	CNxEdit	m_nxeditTimeLength;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfigureImplementationStepDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;
	NXDATALIST2Lib::_DNxDataListPtr m_pCategoryList;
	NXDATALIST2Lib::_DNxDataListPtr m_pAssignToList;
	NXDATALIST2Lib::_DNxDataListPtr m_pPriorityList;
	NXDATALIST2Lib::_DNxDataListPtr m_pActionList;
	NXDATALIST2Lib::_DNxDataListPtr m_pActionItemList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDocCategoryList;
	NXDATALIST2Lib::_DNxDataListPtr m_pTimeIntervalList;

	long m_nStepID;
	long m_nLadderID;
	bool m_bIsTemplate;

	void Load();
	void SaveActionItems(long nStepID, long nActionID);
	void LoadActionItemDatalist(long nActionID);
	void AddToStringArray(CString strStringToAdd);
	void AddToIDArray(long nIDToAdd);

	CDWordArray m_dwActionItemIDs;
	CDWordArray m_dwDeletedActionItemIDs;
	CStringArray m_strActionItemPaths;
	CStringArray m_strDeletedActionItemPaths;
	BOOL SelectMultiPurposes(long nActionID);
	CString GetActionItemText(long nActionID);
	void LoadActionItems(long nActionID);
	// Generated message map functions
	//{{AFX_MSG(CConfigureImplementationStepDlg)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnSelChosenImplementationStepAction(LPDISPATCH lpRow);
	afx_msg void OnSelChosenImplementationActionItemList(LPDISPATCH lpRow);
	afx_msg void OnSelChangingImplementationActionItemList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnAutoActivate();
	afx_msg void OnAfterXTime();
	afx_msg void OnActivateImmediately();
	afx_msg void OnSelChangingTimeInterval(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnStepCreateTodo();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONFIGUREIMPLEMENTATIONSTEPDLG_H__305A60CD_7F7E_4F5B_B45C_1BC5CF6FDE17__INCLUDED_)

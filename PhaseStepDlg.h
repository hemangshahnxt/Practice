#if !defined(AFX_PHASESTEPDLG_H__886B5B52_5206_4BD1_A1D5_70294F1D9ACC__INCLUDED_)
#define AFX_PHASESTEPDLG_H__886B5B52_5206_4BD1_A1D5_70294F1D9ACC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhaseStepDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPhaseStepDlg dialog

class CPhaseStepDlg : public CNxDialog
{
// Construction
public:
	void EnableWindow(BOOL bEnable);
	CPhaseStepDlg();

	bool m_bIsTracked;

	long m_nStepTemplateID, m_nLadderTemplateID;

	// (j.jones 2008-11-17 17:22) - PLID 30926 - added m_checkOpenPICWhenSelected
	// (j.jones 2008-11-26 10:45) - PLID 30830 - added m_nxstaticAssignToMultiple
// Dialog Data
	//{{AFX_DATA(CPhaseStepDlg)
	enum { IDD = IDD_PHASE_STEP_DLG };
	CNxStatic	m_nxstaticAssignToMultiple;
	NxButton	m_checkOpenPICWhenSelected;
	NxButton	m_btnImmediate;
	NxButton	m_btnTimeAfter;
	NxButton	m_btnAsk;
	NxButton	m_btnNever;
	NxButton	m_btnAllowSkip;
	NxButton	m_btnCreateTodo;
	CComboBox	m_cbPriority;
	CComboBox	m_cbIntervalType;
	CNxIconButton	m_okBtn;
	CNxIconButton	m_cancelBtn;
	CNxEdit	m_nxeditName;
	CNxEdit	m_nxeditNote;
	CNxEdit	m_nxeditIntervalNumber;
	CNxEdit	m_nxeditStep;
	CNxStatic	m_nxstaticPrevStepLabel;
	CNxStatic	m_nxstaticCriteriaList;
	CNxStatic	m_nxstaticMergeScopeLabel;
	CNxIconButton	m_nxbMergeScopeHelp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhaseStepDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void LoadData();
	void LoadActionList();
	void LoadItemList();
	void LoadAction(long id, _bstr_t action);
	bool Save();
	void LoadActivateData();
	void EditCriteriaList();

	long	m_maxStep,
	        m_nTemplateID;
	bool	m_new, m_bActionIdsChanged;

	long	m_nOrigAction;
	// (j.armen 2012-01-23 08:59) - PLID 47707 - Replaced CDWordArray with CArray<long, long>
	CArray<long, long> m_aryOrigActionIDs, m_aryActionIDs;
	CString m_strItemList;



	CString m_strPrompt;

	NXDATALISTLib::_DNxDataListPtr m_action,
					m_item;

	NXDATALISTLib::_DNxDataListPtr m_pUserList;

	NXDATALISTLib::_DNxDataListPtr m_pTodoCategoryList;

	//TES 7/16/2010 - PLID 39705
	NXDATALIST2Lib::_DNxDataListPtr m_pMergeScope;

	// (j.jones 2008-11-26 10:50) - PLID 30830 - track the IDs of users we're assigning this step to
	CArray<long, long> m_aryAssignToIDs, m_aryOriginalAssignToIDs;
	CString m_strUserLinkText;

	// (j.jones 2008-11-26 11:30) - PLID 30830 - detects if m_aryAssignToIDs matches m_aryOriginalAssignToIDs
	BOOL HasUserIDListChanged();

	// (j.jones 2008-11-26 11:17) - PLID 30830 - RefreshUserCombo will display or hide
	// the user combo box or hyperlink, based on how many users this step is assigned to
	void RefreshUserCombo();

	void RefreshItemDisplay(); //Set the drop-down/hyperlink based on m_dwaActionIds.

	void ReflectCurrentAction(); //This just enables/disables the Activate area based on whether the current step allows it.

	// (j.jones 2008-11-26 11:37) - PLID 30830 - added OnSelChosenAssignUsers
	// Generated message map functions
	//{{AFX_MSG(CPhaseStepDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangedAction(long nNewSel);
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnKillfocusStep();
	afx_msg void OnImmediate();
	afx_msg void OnTimeAfter();
	afx_msg void OnAsk();
	afx_msg void OnNever();
	afx_msg void OnPrompt();
	afx_msg void OnSelChosenItem(long nRow);
	afx_msg void OnRequeryFinishedAssignUsers(short nFlags);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSelChosenAction(long nRow);
	afx_msg void OnStepCreateTodo();
	afx_msg void OnRequeryFinishedTodoCategory(short nFlags);
	afx_msg void OnSelChosenTodoCategory(long nRow);
	afx_msg void OnSelChosenAssignUsers(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMergeScopeHelp();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHASESTEPDLG_H__886B5B52_5206_4BD1_A1D5_70294F1D9ACC__INCLUDED_)

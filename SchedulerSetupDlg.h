#include "commondialog.h"
#if !defined(AFX_SCHEDULERSETUPDLG_H__B3130A8A_8A0F_43CC_9FEE_38274A8E8950__INCLUDED_)
#define AFX_SCHEDULERSETUPDLG_H__B3130A8A_8A0F_43CC_9FEE_38274A8E8950__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SchedulerSetupDlg.h : header file
//
#include "Client.h"
/////////////////////////////////////////////////////////////////////////////
// CSchedulerSetupDlg dialog

class CSchedulerSetupDlg : public CNxDialog
{
// Construction
public:
	CSchedulerSetupDlg(CWnd* pParent);   // standard constructor

	CBrush m_brush;

	virtual void Refresh();
// Dialog Data
	//{{AFX_DATA(CSchedulerSetupDlg)
	enum { IDD = IDD_SCHEDULER_SETUP_DLG };
	CNxIconButton	m_btnInactivateType;
	CNxIconButton	m_btnInactiveTypes;
	CNxIconButton	m_btnDuration;
	CNxColor	m_typeColor;
	CCommonDialog60	m_ctrlColorPicker;
	CNxIconButton	m_btnNewType;
	CNxIconButton	m_btnDeleteType;
	CNxIconButton	m_btnRenameType;
	CNxIconButton	m_btnNewPurpose;
	CNxIconButton	m_btnDeletePurpose;
	CNxIconButton	m_btnRenamePurpose;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnDeselectAll;
	CNxIconButton	m_btnResource;
	CNxIconButton	m_btnTemplates;
	CNxIconButton	m_btnTemplateCollections; // (z.manning 2014-12-03 09:51) - PLID 64205
	CNxIconButton	m_btnReplace;
	CNxIconButton	m_btnStatus;
	CNxIconButton	m_btnBookingAlarms;
	CNxIconButton	m_btnPreferences;
	CNxIconButton	m_btnCopyTo;
	CNxIconButton	m_btnMergeInto;
	CNxIconButton   m_btnCodeLink;  // (j.gruber 2010-07-20 14:09) - PLID 30481 - created
	CNxEdit	m_nxeditEditDefaultArrivalMins;
	CNxEdit	m_nxeditEditDefaultTypeDuration;
	CNxStatic	m_nxstaticDefaultArrivalTimeLabel;
	CNxStatic	m_nxstaticDefaultArrivalTimeLabel2;
	CNxStatic	m_nxstaticDefaultDurationLabel;
	CNxIconButton	m_btnRecallSetup; // (j.armen 2012-02-22 12:31) - PLID 48304
	// (j.jones 2012-04-10 15:15) - PLID 44174 - added ability to merge resources
	CNxIconButton	m_btnMergeResources;
	// (s.tullis 2014-12-02 10:51) - PLID 64125 - added scheduler mix rules
	CNxIconButton   m_btnScheduleMixRules;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSchedulerSetupDlg)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALISTLib::_DNxDataListPtr m_procedureList,
					m_typeCombo,
					m_categoryCombo,
					m_purposeCombo;

	bool GetPurposeID(long &id);
	bool GetTypeID(long &id);
	void AddCategory(short id, LPCSTR str);

	CTableChecker	m_purposeChecker, 
					m_typeChecker, 
					m_purposeTypeChecker;

	BOOL			m_bNeedToSaveDuration;
	BOOL			m_bNeedToSaveArrivalMins;

	// (a.walling 2008-09-02 17:54) - PLID 23457 - Overwrite or merge procedure info into another appointment type
	void CopyProcedureInfo(BOOL bOverwrite);
	

	// (a.walling 2008-09-03 08:57) - PLID 23457 - Handlers for copy/merge buttons
	// Generated message map functions
	//{{AFX_MSG(CSchedulerSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnResource();
	afx_msg void OnTemplates();
	afx_msg void OnTemplateCollections();
	afx_msg void OnNewPurpose();
	afx_msg void OnNewType();
	afx_msg void OnDeletePurpose();
	afx_msg void OnClickTypeColor();
	afx_msg void OnPreferences();
	afx_msg void OnSelChosenCategoryCombo(long nRow);
	afx_msg void OnSelChosenPurposeCombo(long nRow);
	afx_msg void OnSelChosenTypeCombo(long nRow);
	afx_msg void OnEditingFinishedProcedureList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnDeleteType();
	afx_msg void OnLButtonUpProcedureList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedTypeCombo(short nFlags);
	afx_msg void OnRequeryFinishedProcedureList(short nFlags);
	afx_msg void OnReplace();
	afx_msg void OnRenameType();
	afx_msg void OnRenamePurpose();
	afx_msg void OnSelectAll();
	afx_msg void OnDeselectAll();
	afx_msg void OnEnableUpdate();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSchedulerDuration();
	afx_msg void OnEditStatuses();
	afx_msg void OnChangeEditDefaultTypeDuration();
	afx_msg void OnKillfocusEditDefaultTypeDuration();
	afx_msg void OnChangeEditDefaultArrivalMins();
	afx_msg void OnKillfocusEditDefaultArrivalMins();
	afx_msg void OnInactivateType();
	afx_msg void OnInactiveTypes();
	afx_msg void OnBookingAlarms();
	afx_msg void OnCopyTo();
	afx_msg void OnMergeInto();
	afx_msg void OnBnClickedRecallSetup(); // (j.armen 2012-02-22 12:57) - PLID 48304
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSetfocusEditDefaultTypeDuration();
	// (a.walling 2010-06-15 14:14) - PLID 39184 - Set the cursel if right button is down on a non-highlighted row
	void RButtonDownProcedureList(long nRow, short nCol, long x, long y, long nFlags);
	// (a.walling 2010-06-15 14:14) - PLID 39184 - Display a popup menu to assign and edit resource sets to types and type-purposes
	void RButtonUpProcedureList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedConfTypeCodeLink();
	// (j.jones 2012-04-10 15:15) - PLID 44174 - added ability to merge resources
	afx_msg void OnBtnMergeResources();
	// (s.tullis 2014-12-02 10:51) - PLID 64125 - added scheduler mix rules
	afx_msg void OnBtnScheduleMixRules();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCHEDULERSETUPDLG_H__B3130A8A_8A0F_43CC_9FEE_38274A8E8950__INCLUDED_)

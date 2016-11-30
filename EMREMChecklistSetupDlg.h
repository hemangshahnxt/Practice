#if !defined(AFX_EMREMCHECKLISTSETUPDLG_H__FF16BD57_F1B5_4DBB_A7CC_9773FC7976E6__INCLUDED_)
#define AFX_EMREMCHECKLISTSETUPDLG_H__FF16BD57_F1B5_4DBB_A7CC_9773FC7976E6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMREMChecklistSetupDlg.h : header file
//

// (j.jones 2007-08-16 10:51) - PLID 27055 - created

// (j.jones 2007-08-27 08:51) - PLID 27056 - moved struct defines to EmrUtils.h

#include "EmrUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistSetupDlg dialog

class CEMREMChecklistSetupDlg : public CNxDialog
{
// Construction
public:
	CEMREMChecklistSetupDlg(CWnd* pParent);   // standard constructor
	~CEMREMChecklistSetupDlg();

	NXDATALIST2Lib::_DNxDataListPtr m_VisitTypeCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_Checklist;
	NXDATALIST2Lib::_DNxDataListPtr m_ColumnList;

	// (j.jones 2007-08-29 10:47) - PLID 27135 - added for permission purposes
	BOOL m_bReadOnly;

// Dialog Data
	//{{AFX_DATA(CEMREMChecklistSetupDlg)
	enum { IDD = IDD_EMR_EM_CHECKLIST_SETUP_DLG };
	CNxIconButton	m_btnShowAuditHistory;
	CNxIconButton	m_btnMoveColDown;
	CNxIconButton	m_btnMoveColUp;
	CNxIconButton	m_btnCreateDelete;
	CNxIconButton	m_btnEditVisitTypes;
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnDeleteColumn;
	CNxIconButton	m_btnAddColumn;
	CNxIconButton	m_btnAddCodingLevel;
	NxButton	m_checkShowInactiveVisits;
	CNxStatic	m_nxstaticColumnListLabel;
	CNxStatic	m_nxstaticChecklistSetupLabel;
	CNxStatic	m_nxstaticChecklistApprovalLabel;
	CNxStatic	m_nxstaticNoChecklistLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMREMChecklistSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (c.haag 2007-08-30 13:08) - PLID 27058 - TRUE if this window is being
	// created as a modeless window
	BOOL m_bIsModeless;

public:
	// (c.haag 2007-08-30 13:02) - PLID 27058 - Defines whether this window should be created
	// as a modal or a modeless dialog
	void SetModeless(BOOL bModeless);

	// (c.haag 2007-08-30 13:04) - PLID 27058 - Returns TRUE if this dialog should exist as a
	// modeless dialog
	BOOL IsModeless() const;
	
protected:
	void Load(long nChecklistID = -1);

	//will remove all rows, and rebuild them from memory,
	//properly storing their pointers in any coding levels that reference the rows
	void RebuildChecklistRows();

	//will remove all columns, and rebuild them from memory
	//properly storing their indexes in the column info objects
	void RebuildChecklistColumns();

	//should we load the checklist for the first visit in the list?
	BOOL m_bLoadFirstVisitType;

	ChecklistInfo *m_pChecklistInfo;

	void ClearChecklistInfo();

	//removes one checklist rule, and its details, from memory
	void RemoveOneChecklistElementRule(ChecklistElementRuleInfo *pInfo);

	//copies the details and description only from pSourceRule into pDestRule,
	//for the purposes of pre-filling data
	void CopyRuleDetails(ChecklistElementRuleInfo *pSourceRule, ChecklistElementRuleInfo *&pDestRule);

	//used when creating a new rule, this function will try to auto-populate the rule
	//contents with data from a previous rule in the same column
	BOOL TryPopulateRuleFromPriorRow(ChecklistElementRuleInfo *pInfo);

	//given a column index, find and return the matching ChecklistColumnInfo object
	ChecklistColumnInfo* FindColumnInfoObjectByColIndex(short nCol);

	//given a column ID, find and return the matching ChecklistColumnInfo object
	ChecklistColumnInfo* FindColumnInfoObjectByColumnID(long nID);

	//given a datalist row, find and return the matching ChecklistCodingLevelInfo object
	ChecklistCodingLevelInfo* FindCodingLevelInfoObjectByRowPtr(NXDATALIST2Lib::IRowSettingsPtr pRow);

	//shows/hides checklist editing controls
	void ToggleChecklistControls(BOOL bShowChecklist);

	// (j.jones 2007-08-29 10:48) - PLID 27135 - disable checklist controls if it is read-only
	void SetChecklistControlsReadOnly();

	//will scan all elements, and if any are unapproved, will show the warning label
	void CheckShowApprovalWarning();

	//allows re-ordering columns
	void SwapColumnOrders(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2);

protected:
	// (c.haag 2007-08-30 12:41) - PLID 27058 - We need to perform special handling with
	// these functions if we are modeless
	void OnOK();
	void OnCancel();

	// (c.haag 2007-08-30 13:04) - PLID 27058 - If this dialog is to be modeless, we alter the window style here
	void PreSubclassWindow();

	// Generated message map functions
	//{{AFX_MSG(CEMREMChecklistSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenVisitTypesCombo(LPDISPATCH lpRow);
	afx_msg void OnBtnEditVisitTypes();
	afx_msg void OnCheckShowInactiveVisitTypesForChecklist();
	afx_msg void OnLeftClickEmChecklistSetupList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedVisitTypesCombo(short nFlags);
	afx_msg void OnRButtonDownEmChecklistSetupList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBtnAddColumn();
	afx_msg void OnBtnDeleteColumn();
	afx_msg void OnEditingFinishingEmChecklistColumnList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedEmChecklistColumnList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnBtnAddCodingLevel();
	afx_msg void OnButtonCreateDelete();
	afx_msg void OnBtnMoveColumnUp();
	afx_msg void OnBtnMoveColumnDown();
	// (j.jones 2007-08-24 11:03) - PLID 27152 - added ability to view the checklist history
	afx_msg void OnBtnShowAuditHistory();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMREMCHECKLISTSETUPDLG_H__FF16BD57_F1B5_4DBB_A7CC_9773FC7976E6__INCLUDED_)

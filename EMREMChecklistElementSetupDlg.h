#if !defined(AFX_EMREMCHECKLISTELEMENTSETUPDLG_H__4B71CAC6_0F03_40F3_9239_D730D5F118BB__INCLUDED_)
#define AFX_EMREMCHECKLISTELEMENTSETUPDLG_H__4B71CAC6_0F03_40F3_9239_D730D5F118BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMREMChecklistElementSetupDlg.h : header file
//

#include "EMREMChecklistSetupDlg.h"

// (j.jones 2007-08-17 15:31) - PLID 27104 - created

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistElementSetupDlg dialog

class CEMREMChecklistElementSetupDlg : public CNxDialog
{
// Construction
public:
	CEMREMChecklistElementSetupDlg(CWnd* pParent);   // standard constructor
	~CEMREMChecklistElementSetupDlg();

	NXDATALIST2Lib::_DNxDataListPtr m_DetailList;

	ChecklistElementRuleInfo *m_pParentRuleInfo;	//the pointer holding the checklist's rule information
	ChecklistElementRuleInfo *m_pLocalRuleInfo;		//the pointer holding our local copy of the rule information

	ChecklistInfo *m_pChecklist;	//the pointer to our checklist parent
	
	BOOL m_bIsPreLoaded;	//true if we are pre-filling a new rule with the previous rule

// Dialog Data
	//{{AFX_DATA(CEMREMChecklistElementSetupDlg)
	enum { IDD = IDD_EMR_EM_CHECKLIST_ELEMENT_SETUP_DLG };
	NxButton	m_btnAllDetails;
	NxButton	m_btnAnyDetails;
	NxButton	m_btnApproved;
	CNxIconButton	m_btnEditEMCategories;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnAdd;
	CNxEdit	m_nxeditEditRuleDescription;
	CNxStatic	m_nxstaticCategoryCompletionLabel;
	CNxStatic	m_nxstaticRulesPrefilledLabel;
	CNxStatic	m_nxstaticDescriptionChangedLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMREMChecklistElementSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	BOOL Save();

	//given a datalist row, delete the detail at that row
	void DeleteRuleDetail(NXDATALIST2Lib::IRowSettingsPtr pRowToDelete);

	//clear the contents of passed-in rule info, do not deallocate the rule info!
	void ClearRuleContents(ChecklistElementRuleInfo *pRule);

	//copies the contents of pSourceRule into pDestRule, overwriting pDestRule
	void CopyRuleInfo(ChecklistElementRuleInfo *pSourceRule, ChecklistElementRuleInfo *&pDestRule);

	void HidePreLoadedLabel();
	
	//generates a description of the contents of this rule
	void GenerateRuleDescription();
	//stores the generated description of the contents of this rule
	CString m_strCurGeneratedRuleDesc;
	//true if we are displaying the warning label for a changed description
	BOOL m_bShowDescriptionChangedWarning;
	//determines if we show or hide the description warning label
	void DisplayDescriptionChangedWarning(BOOL bShow);
	//compares our stored description to the displayed description, optionally recalculating the stored description
	void CheckShowDescriptionChangedWarning(BOOL bRecalculateRuleDesc);
	//potentially updates the description with our changes
	void RecalculateAndApplyNewDescription();

	//will uncheck the approved box if checked
	void CheckUndoApproval();

	// (j.jones 2007-08-29 11:36) - PLID 27135 - changed the approval code for permission purposes
	void ReflectElementApprovalChange();

	// Generated message map functions
	//{{AFX_MSG(CEMREMChecklistElementSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnEditingFinishingChecklistElementRuleList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedChecklistElementRuleList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnChangeEditRuleDescription();
	virtual void OnOK();
	afx_msg void OnBtnAddNewElementRule();
	afx_msg void OnBtnDeleteElementRule();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBtnEditEmCats();
	afx_msg void OnCheckElementApproved();
	// (j.jones 2007-09-18 16:38) - PLID 27397 - added any/all radio buttons
	afx_msg void OnRadioAllDetails();
	afx_msg void OnRadioAnyDetails();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMREMCHECKLISTELEMENTSETUPDLG_H__4B71CAC6_0F03_40F3_9239_D730D5F118BB__INCLUDED_)

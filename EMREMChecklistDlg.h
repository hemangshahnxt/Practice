#if !defined(AFX_EMREMCHECKLISTDLG_H__8CDBC790_2755_4D11_9701_7A3E7D206C10__INCLUDED_)
#define AFX_EMREMCHECKLISTDLG_H__8CDBC790_2755_4D11_9701_7A3E7D206C10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMREMChecklistDlg.h : header file
//

// (j.jones 2007-08-27 08:39) - PLID 27056 - created

#include "EmrUtils.h"
#include "GlobalAuditUtils.h"

// (j.jones 2013-05-16 14:35) - PLID 56596 - changed EMN.h to a forward declare
class CEMN;

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistDlg dialog

class CEMREMChecklistDlg : public CNxDialog
{
// Construction
public:
	CEMREMChecklistDlg(CWnd* pParent);   // standard constructor
	~CEMREMChecklistDlg();

	NXDATALIST2Lib::_DNxDataListPtr m_Checklist;

	CString m_strVisitTypeName;
	long m_nVisitTypeID;
	long m_nChecklistID;
	// (j.jones 2013-04-22 16:21) - PLID 56372 - added a read only flag
	BOOL m_bIsReadOnly;

	ChecklistCodingLevelInfo *m_pCodingLevelToUse;

	CEMN *m_pEMN;

	// (j.jones 2007-08-30 10:15) - PLID 27221 - added a pending audit info array, for approval audits
	// (j.jones 2013-04-22 10:57) - PLID 54596 - obsolete, this dialog now audits before closing
	//CArray<CPendingAuditInfo*, CPendingAuditInfo*> m_aryPendingEMAuditInfo;

// Dialog Data
	//{{AFX_DATA(CEMREMChecklistDlg)
	enum { IDD = IDD_EMR_EM_CHECKLIST_DLG };
		// NOTE: the ClassWizard will add data members here
	CNxEdit	m_nxeditEditCodingLevel;
	// (j.jones 2013-04-24 11:25) - PLID 54596 - renamed the button variables
	// for clarity, because they aren't displayed as OK/Cancel
	CNxIconButton m_btnAddCharge;
	CNxIconButton m_btnClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMREMChecklistDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	BOOL Load();

	ChecklistInfo *m_pChecklistInfo;

	void ClearChecklistInfo();

	//given a column index, find and return the matching ChecklistColumnInfo object by the category column
	ChecklistColumnInfo* FindColumnInfoObjectByCategoryColIndex(short nCategoryCol);

	//given a column index, find and return the matching ChecklistColumnInfo object by the checkbox
	ChecklistColumnInfo* FindColumnInfoObjectByCheckboxColIndex(short nCheckboxCol);

	//given a column ID, find and return the matching ChecklistColumnInfo object
	ChecklistColumnInfo* FindColumnInfoObjectByColumnID(long nID);

	//given a datalist row, find and return the matching ChecklistCodingLevelInfo object
	ChecklistCodingLevelInfo* FindCodingLevelInfoObjectByRowPtr(NXDATALIST2Lib::IRowSettingsPtr pRow);

	//given a datalist row and a column info ptr, find the rule info object if one exists
	ChecklistElementRuleInfo* FindElementRuleInfoObject(NXDATALIST2Lib::IRowSettingsPtr pRow, ChecklistColumnInfo *pColInfo);

	// (j.jones 2013-04-19 16:04) - PLID PLID 54596 - finds a rule by rule ID
	ChecklistElementRuleInfo* FindElementRuleInfoObjectByID(long nRuleID);

	// (j.jones 2007-09-28 08:48) - PLID 27547 - when tracking category element counts,
	// we now track the details that contribute to those counts
	CArray<ChecklistTrackedCategoryInfo*, ChecklistTrackedCategoryInfo*> m_aryTrackedCategories;
	
	//fill in the m_aryTrackedCategories structure with data from the EMN
	void PopulateTrackedCategories();
	
	//clear the m_aryTrackedCategories structure
	void ClearTrackedCategories();
	
	//this is the key function that colorizes cells based on E/M elements and the cell rules
	void CalculatePassedRulesAndColorCells();

	//this function re-colorizes a coding level cell based on the approval status of rules,
	//and also potentially unchecks the approval status if rule approvals have changed
	void UpdateCodingLevelStatus(ChecklistCodingLevelInfo *pCodingLevelInfo);

	//based on what is approved by the user, reflect the highest CPT code in the interface
	void FindAndReflectBestServiceCode();

	// (j.jones 2007-08-29 16:13) - PLID 27057 - added ability to check for out-of-date details
	BOOL VerifyAllDetailsUpToDate();

	// (b.spivey, February 24, 2012) - PLID 38409 - Added Approve/Unapprove functions. 
	void ApproveCell(NXDATALIST2Lib::IRowSettingsPtr pRow, ChecklistElementRuleInfo* &pRuleInfo); 
	void UnapproveCell(NXDATALIST2Lib::IRowSettingsPtr pRow, ChecklistElementRuleInfo* &pRuleInfo);

	// Generated message map functions
	//{{AFX_MSG(CEMREMChecklistDlg)
	virtual BOOL OnInitDialog();
	// (j.jones 2007-09-27 16:51) - PLID 27547 - added ability to pop up a dialog when an approval is attempted
	afx_msg void OnEditingStartingEmChecklist(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedEmChecklist(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	virtual void OnOK();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void LeftClickEmChecklist(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMREMCHECKLISTDLG_H__8CDBC790_2755_4D11_9701_7A3E7D206C10__INCLUDED_)

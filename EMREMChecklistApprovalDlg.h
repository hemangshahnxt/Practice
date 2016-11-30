#if !defined(AFX_EMREMCHECKLISTAPPROVALDLG_H__61946B44_DDA4_4940_B77A_A86F02ED4F58__INCLUDED_)
#define AFX_EMREMCHECKLISTAPPROVALDLG_H__61946B44_DDA4_4940_B77A_A86F02ED4F58__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMREMChecklistApprovalDlg.h : header file
//

// (j.jones 2007-09-27 16:39) - PLID 27547 - created

#include "EmrUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistApprovalDlg dialog

class CEMREMChecklistApprovalDlg : public CNxDialog
{
// Construction
public:
	CEMREMChecklistApprovalDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_CategoryList;
	NXDATALIST2Lib::_DNxDataListPtr m_DetailList;

	// (b.spivey, February 24, 2012) - PLID 38409 - Removed checkbox

	ChecklistElementRuleInfo *m_pRuleInfo;	//stores the info for the rule we're approving

	// (j.jones 2013-04-23 16:30) - PLID 56372 - added a read only status
	BOOL m_bIsReadOnly;

	// (j.jones 2007-09-28 10:29) - PLID 27547 - when tracking category element counts,
	// we now track the details that contribute to those counts
	CArray<ChecklistTrackedCategoryInfo*, ChecklistTrackedCategoryInfo*> *m_aryTrackedCategories;

// Dialog Data
	//{{AFX_DATA(CEMREMChecklistApprovalDlg)
	enum { IDD = IDD_EMR_EM_CHECKLIST_APPROVAL_DLG };
	NxButton	m_btnApproveCheck;
	CNxStatic	m_nxstaticCodingLevelDesc;
	CNxStatic	m_nxstaticRuleDesc;
	CNxStatic	m_nxstaticCategoryListLabel;
	CNxStatic	m_nxstaticNoCategoriesLabel;
	CNxStatic	m_nxstaticDetailListLabel;
	CNxStatic	m_nxstaticRulePassedLabel;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMREMChecklistApprovalDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void Load();

	// Generated message map functions
	//{{AFX_MSG(CEMREMChecklistApprovalDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMREMCHECKLISTAPPROVALDLG_H__61946B44_DDA4_4940_B77A_A86F02ED4F58__INCLUDED_)

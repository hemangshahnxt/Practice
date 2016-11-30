#if !defined(AFX_EMREMCHECKLISTCODESETUPDLG_H__F0E1D6EF_09E9_4947_B9FC_93F72A510E09__INCLUDED_)
#define AFX_EMREMCHECKLISTCODESETUPDLG_H__F0E1D6EF_09E9_4947_B9FC_93F72A510E09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMREMChecklistCodeSetupDlg.h : header file
//

#include "EMREMChecklistSetupDlg.h"

// (j.jones 2007-08-17 15:32) - PLID 27104 - created

/////////////////////////////////////////////////////////////////////////////
// CEMREMChecklistCodeSetupDlg dialog

class CEMREMChecklistCodeSetupDlg : public CNxDialog
{
// Construction
public:
	CEMREMChecklistCodeSetupDlg(CWnd* pParent);   // standard constructor

	NXDATALIST2Lib::_DNxDataListPtr m_ServiceCodeCombo;

	long m_nCodingLevelID;			//the ID in data for this coding level
	long m_nServiceID;				//the ServiceID of the CPT code
	CString m_strServiceCode;		//the CPT Code number
	long m_nColumnsRequired;		//the number of columns required to satisfy this row
	long m_nMinimumTimeRequired;	// (j.jones 2007-09-17 10:05) - PLID 27396 - the minimum time required to satisfy this row (0 for not required)
	CString m_strDescription;		//the description of this coding level
	BOOL m_bApproved;				//tracks if the coding level has been approved
	COleDateTime m_dtApproved;		//if approved, when was it approved?
	long m_nApprovalUserID;			//if approved, the user ID who approved it
	CString m_strApprovalUserName;	//if approved, the username who approved it

	ChecklistInfo *m_pChecklist;	//the passed in pointer to the checklist

// Dialog Data
	//{{AFX_DATA(CEMREMChecklistCodeSetupDlg)
	enum { IDD = IDD_EMR_EM_CHECKLIST_CODE_SETUP_DLG };
	NxButton	m_btnApproved;
	NxButton	m_btnRequireMin;
	CNxEdit	m_nxeditEditColumnsRequired;
	CNxEdit	m_nxeditEditTimeRequired;
	CNxEdit	m_nxeditEditCodeDescription;
	CNxStatic	m_nxstaticDescriptionChangedLabel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMREMChecklistCodeSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	long m_nPendingServiceID;	//used for inactive codes

	COleDateTime m_dtCurApproved;
	long m_nCurApprovalUserID;
	CString m_strCurApprovalUserName;

	void CheckUndoApproval();	//will uncheck the approved box if checked

	// (j.jones 2007-08-29 11:36) - PLID 27135 - changed the approval code for permission purposes
	void ReflectCodingLevelApprovalChange();

	//generates a description of this coding level
	void GenerateCodingLevelDescription();
	//stores the generated description of the contents of this coding level
	CString m_strCurGeneratedCodingLevelDesc;
	//true if we are displaying the warning label for a changed description
	BOOL m_bShowDescriptionChangedWarning;
	//determines if we show or hide the description warning label
	void DisplayDescriptionChangedWarning(BOOL bShow);
	//compares our stored description to the displayed description, optionally recalculating the stored description
	void CheckShowDescriptionChangedWarning(BOOL bRecalculateDesc);
	//potentially updates the description with our changes
	void RecalculateAndApplyNewDescription();

	// Generated message map functions
	//{{AFX_MSG(CEMREMChecklistCodeSetupDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChosenServiceCodeCombo(LPDISPATCH lpRow);
	afx_msg void OnChangeEditColumnsRequired();
	afx_msg void OnCheckCodingLevelApproved();
	afx_msg void OnChangeEditCodeDescription();
	afx_msg void OnTrySetSelFinishedServiceCodeCombo(long nRowEnum, long nFlags);
	virtual void OnOK();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	// (j.jones 2007-09-17 10:08) - PLID 27396 - added minimum time
	afx_msg void OnCheckRequireMinimumTime();
	afx_msg void OnChangeEditTimeRequired();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMREMCHECKLISTCODESETUPDLG_H__F0E1D6EF_09E9_4947_B9FC_93F72A510E09__INCLUDED_)

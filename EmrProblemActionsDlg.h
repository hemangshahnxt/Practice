#if !defined(AFX_EMRPROBLEMACTIONSDLG_H__AF4D0053_0BD8_4539_9637_5BA7909504BC__INCLUDED_)
#define AFX_EMRPROBLEMACTIONSDLG_H__AF4D0053_0BD8_4539_9637_5BA7909504BC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EmrProblemActionsDlg.h : header file
//
// (c.haag 2008-07-17 10:52) - PLID 30723 - Initial implementation
// (c.haag 2014-07-22) - PLID 62789 - Refactored new problem entry. Problems are now entered via modal dialog
//
#include "EmrUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CEmrProblemActionsDlg dialog

class CEmrProblemActionsDlg : public CNxDialog
{
// Construction
public:
	CEmrProblemActionsDlg(CProblemActionAry* paryProblemActions, EmrActionObject SourceType,
		EmrActionObject DestType, CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEmrProblemActionsDlg)
	enum { IDD = IDD_EMR_PROBLEM_ACTIONS };
	CNxIconButton	m_btnDeleteProblem;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnAddProblem;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEmrProblemActionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlProblems;

protected:
	// When a problem is spawned, it can either be spawned for the source object, or
	// the destination object. This enumeration tells us if we support either, or whether
	// we are only allowed to associate the problem with either the source or destination
	// object.
	enum EAssocMode {
		eSupportEitherAssoc = 0L,
		eSupportSourceAssocOnly,
		eSupportDestAssocOnly
	} m_AssocMode;

protected:
	CProblemActionAry* m_paryProblemActions; // The array of problems from which we load and save from
	EmrActionObject m_SourceType; // The type of object spawning the action
	EmrActionObject m_DestType; // The type of object being spawned

protected:
	// Generated message map functions
	//{{AFX_MSG(CEmrProblemActionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAddProblemAction();
	afx_msg void OnBtnEditProblemAction();
	afx_msg void OnBtnDeleteProblemAction();
	virtual void OnOK();
	afx_msg void OnSelChangedListActionProblems(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
//	void EditingFinishedListActionProblems(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void EditingFinishingListActionProblems(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRPROBLEMACTIONSDLG_H__AF4D0053_0BD8_4539_9637_5BA7909504BC__INCLUDED_)

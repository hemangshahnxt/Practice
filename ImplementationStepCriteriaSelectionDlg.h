#if !defined(AFX_IMPLEMENTATIONSTEPCRITERIASELECTIONDLG_H__4EC16A04_D5F1_4887_9A8D_0C99932D00EE__INCLUDED_)
#define AFX_IMPLEMENTATIONSTEPCRITERIASELECTIONDLG_H__4EC16A04_D5F1_4887_9A8D_0C99932D00EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImplementationStepCriteriaSelectionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImplementationStepCriteriaSelectionDlg dialog

class CImplementationStepCriteriaSelectionDlg : public CNxDialog
{
// Construction
public:
	CImplementationStepCriteriaSelectionDlg(long nListType, long nStepID, CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CImplementationStepCriteriaSelectionDlg)
	enum { IDD = IDD_IMPLEMENTATION_STEP_CRITERIA_SELECT };
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImplementationStepCriteriaSelectionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pCriteriaList;
	long m_nListType;
	long m_nStepID;

	// Generated message map functions
	//{{AFX_MSG(CImplementationStepCriteriaSelectionDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPLEMENTATIONSTEPCRITERIASELECTIONDLG_H__4EC16A04_D5F1_4887_9A8D_0C99932D00EE__INCLUDED_)

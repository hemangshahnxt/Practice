#if !defined(AFX_PROCEDUREDISCOUNTSETUPDLG_H__77321896_342F_47FC_A863_AEED69B852B4__INCLUDED_)
#define AFX_PROCEDUREDISCOUNTSETUPDLG_H__77321896_342F_47FC_A863_AEED69B852B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcedureDiscountSetupDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProcedureDiscountSetupDlg dialog

class CProcedureDiscountSetupDlg : public CNxDialog
{
// Construction
public:
	CProcedureDiscountSetupDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_List;

	long m_ProcedureID;
	CString m_strProcedureName;

	CDWordArray m_dwaryOccurrencesToAdd;
	CDWordArray m_dwaryOccurrencesToDelete;

// Dialog Data
	//{{AFX_DATA(CProcedureDiscountSetupDlg)
	enum { IDD = IDD_PROCEDURE_DISCOUNT_SETUP_DLG };
	CNxIconButton	m_btnAddDiscount;
	CNxIconButton	m_btnDeleteDiscount;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcedureDiscountSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProcedureDiscountSetupDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnAddDiscount();
	afx_msg void OnBtnDeleteDiscount();
	afx_msg void OnRButtonDownDiscountList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishingDiscountList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCEDUREDISCOUNTSETUPDLG_H__77321896_342F_47FC_A863_AEED69B852B4__INCLUDED_)

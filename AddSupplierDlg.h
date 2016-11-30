#if !defined(AFX_ADDSUPPLIERDLG_H__5C718604_70EA_4E6F_923B_A2DF47841837__INCLUDED_)
#define AFX_ADDSUPPLIERDLG_H__5C718604_70EA_4E6F_923B_A2DF47841837__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddSupplierDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddSupplierDlg dialog

class CAddSupplierDlg : public CNxDialog
{
// Construction
public:
	NXDATALISTLib::_DNxDataListPtr m_SupplierList;
	long m_ProductID;
	CAddSupplierDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddSupplierDlg)
	enum { IDD = IDD_ADD_SUPPLIER };
	CNxIconButton m_btnAddNewSupplier;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddSupplierDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddSupplierDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnAddNewSupplier();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDSUPPLIERDLG_H__5C718604_70EA_4E6F_923B_A2DF47841837__INCLUDED_)

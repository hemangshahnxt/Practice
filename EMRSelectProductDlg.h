#if !defined(AFX_EMRSELECTPRODUCTDLG_H__C44610F4_3D19_4947_A44B_12C7EA923ED6__INCLUDED_)
#define AFX_EMRSELECTPRODUCTDLG_H__C44610F4_3D19_4947_A44B_12C7EA923ED6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRSelectProductDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEMRSelectProductDlg dialog

class CEMRSelectProductDlg : public CNxDialog
{
// Construction
public:
	CEMRSelectProductDlg(CWnd* pParent);   // standard constructor

	NXDATALISTLib::_DNxDataListPtr m_ProductList;

	CString m_strProductName;
	long m_nSelectedProductID;
	long m_nSelectedCategoryID;
	BOOL m_bAutoAddAction;

	BOOL m_bDisableInvAction;

// Dialog Data
	//{{AFX_DATA(CEMRSelectProductDlg)
	enum { IDD = IDD_EMR_SELECT_PRODUCT_DLG };
	NxButton	m_btnAutoSelectAction;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnAddCategory;
	CNxIconButton m_btnCancel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRSelectProductDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEMRSelectProductDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnAddCategory();
	afx_msg void OnSelChosenActionProductCombo(long nRow);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRSELECTPRODUCTDLG_H__C44610F4_3D19_4947_A44B_12C7EA923ED6__INCLUDED_)

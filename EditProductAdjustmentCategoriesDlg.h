#if !defined(AFX_EDITPRODUCTADJUSTMENTCATEGORIESDLG_H__163D72B4_AAD6_4B1E_8B9C_622C8EA1E534__INCLUDED_)
#define AFX_EDITPRODUCTADJUSTMENTCATEGORIESDLG_H__163D72B4_AAD6_4B1E_8B9C_622C8EA1E534__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditProductAdjustmentCategoriesDlg.h : header file
//

//TES 6/24/2008 - PLID 26142 - Created
/////////////////////////////////////////////////////////////////////////////
// CEditProductAdjustmentCategoriesDlg dialog

class CEditProductAdjustmentCategoriesDlg : public CNxDialog
{
// Construction
public:
	CEditProductAdjustmentCategoriesDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditProductAdjustmentCategoriesDlg)
	enum { IDD = IDD_EDIT_PRODUCT_ADJUSTMENT_CATEGORIES_DLG };
	CNxIconButton	m_nxbEdit;
	CNxIconButton	m_nxbDelete;
	CNxIconButton	m_nxbClose;
	CNxIconButton	m_nxbAdd;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditProductAdjustmentCategoriesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pCategories;

	void EnableButtons();

	// Generated message map functions
	//{{AFX_MSG(CEditProductAdjustmentCategoriesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangedProductAdjustmentCategories(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnEditingFinishingProductAdjustmentCategories(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedProductAdjustmentCategories(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnAddCategory();
	afx_msg void OnEditCategory();
	afx_msg void OnDeleteCategory();
	afx_msg void OnCloseProductAdjustmentCategories();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITPRODUCTADJUSTMENTCATEGORIESDLG_H__163D72B4_AAD6_4B1E_8B9C_622C8EA1E534__INCLUDED_)

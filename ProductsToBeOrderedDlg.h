#if !defined(AFX_PRODUCTSTOBEORDEREDDLG_H__CFF1B40E_13CC_482C_8EA8_917F3D5984F2__INCLUDED_)
#define AFX_PRODUCTSTOBEORDEREDDLG_H__CFF1B40E_13CC_482C_8EA8_917F3D5984F2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProductsToBeOrderedDlg.h : header file
//

//TES 7/22/2008 - PLID 30802 - Created, largely copied from CProductsToBeReturnedDlg
/////////////////////////////////////////////////////////////////////////////
// CProductsToBeOrderedDlg dialog
class CInvOrderDlg;

class CProductsToBeOrderedDlg : public CNxDialog
{
// Construction
public:
	CProductsToBeOrderedDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProductsToBeOrderedDlg)
	enum { IDD = IDD_PRODUCTS_TO_BE_ORDERED };
	CNxIconButton	m_nxbOrderSelected;
	CNxIconButton	m_nxbOrderAll;
	CNxIconButton	m_nxbClose;
	//}}AFX_DATA

	CInvOrderDlg *m_pInvOrderDlg;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProductsToBeOrderedDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	NXDATALIST2Lib::_DNxDataListPtr m_pLocationList, m_pSupplierList, m_pProductList;

	long m_nFirstSupplierID, m_nFirstLocationID;

	//TES 7/22/2008 - PLID 30802 - Reflect the currently selected location/supplier.
	void RequeryProductList();

	// Generated message map functions
	//{{AFX_MSG(CProductsToBeOrderedDlg)
	afx_msg void OnCloseToBeOrdered();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangingLocationList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenLocationList(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedLocationList(short nFlags);
	afx_msg void OnSelChangingSupplierList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenSupplierList(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedSupplierList(short nFlags);
	afx_msg void OnRequeryFinishedProductsToBeOrdered(short nFlags);
	afx_msg void OnEditingFinishedProductsToBeOrdered(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnOrderAll();
	afx_msg void OnOrderSelected();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRODUCTSTOBEORDEREDDLG_H__CFF1B40E_13CC_482C_8EA8_917F3D5984F2__INCLUDED_)

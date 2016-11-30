#if !defined(AFX_PRODUCTSTOBERETURNEDDLG_H__422D6328_55CA_46C8_AF90_0A5278762820__INCLUDED_)
#define AFX_PRODUCTSTOBERETURNEDDLG_H__422D6328_55CA_46C8_AF90_0A5278762820__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProductsToBeReturnedDlg.h : header file
//

//TES 6/23/2008 - PLID 26152 - Created
/////////////////////////////////////////////////////////////////////////////
// CProductsToBeReturnedDlg dialog

class CProductsToBeReturnedDlg : public CNxDialog
{
// Construction
public:
	CProductsToBeReturnedDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProductsToBeReturnedDlg)
	enum { IDD = IDD_PRODUCTS_TO_BE_RETURNED };
	CNxIconButton	m_nxbReturnSelected;
	CNxIconButton	m_nxbReturnAll;
	CNxIconButton	m_nxbClose;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProductsToBeReturnedDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSupplierList;
	NXDATALIST2Lib::_DNxDataListPtr m_pProductList;

	//TES 6/23/2008 - PLID 26152 - Initial selections in the supplier and location lists.
	long m_nFirstSupplierID;
	long m_nFirstLocationID;

	//TES 6/23/2008 - PLID 26152 - Filter the list of product items based on the currently selected supplier and location.
	void RequeryProductList();

	// Generated message map functions
	//{{AFX_MSG(CProductsToBeReturnedDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRequeryFinishedSupplierList(short nFlags);
	afx_msg void OnSelChangingSupplierList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenSupplierList(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedProductsToBeReturned(short nFlags);
	afx_msg void OnEditingFinishedProductsToBeReturned(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnCloseProductsToReturn();
	afx_msg void OnSelChangingLocationList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenLocationList(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedLocationList(short nFlags);
	afx_msg void OnNewReturnAll();
	afx_msg void OnNewReturnSelected();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRODUCTSTOBERETURNEDDLG_H__422D6328_55CA_46C8_AF90_0A5278762820__INCLUDED_)

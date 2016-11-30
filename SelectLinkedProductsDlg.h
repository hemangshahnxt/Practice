#if !defined(AFX_SELECTLINKEDPRODUCTSDLG_H__98021F07_F00E_48AB_9030_78A4D5DBB572__INCLUDED_)
#define AFX_SELECTLINKEDPRODUCTSDLG_H__98021F07_F00E_48AB_9030_78A4D5DBB572__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectLinkedProductsDlg.h : header file
//

//TES 7/16/2008 - PLID 27983 - Created
/////////////////////////////////////////////////////////////////////////////
// CSelectLinkedProductsDlg dialog

//TES 7/16/2008 - PLID 27983 - Our output structure.
struct ProductToBill {
	long nProductID; //TES 7/16/2008 - PLID 27983 - The ID of the product.
	CArray<long,long> arProductItemIDs; //TES 7/16/2008 - PLID 27983 - Any ProductItemsT records the user selected from this dialog.
	long nQty; //TES 7/16/2008 - PLID 27983 - The quantity they wish to add, ignored if arProductItemIDs has any records.

	ProductToBill()
	{
		nProductID = nQty = -1;
	}

	ProductToBill(const ProductToBill &ptbSource)
	{
		nProductID = ptbSource.nProductID;
		arProductItemIDs.RemoveAll();
		for(int i = 0; i < ptbSource.arProductItemIDs.GetSize(); i++) {
			arProductItemIDs.Add(ptbSource.arProductItemIDs[i]);
		}
		nQty = ptbSource.nQty;
	}
	void ProductToBill::operator =(ProductToBill &ptbSource)
	{
		nProductID = ptbSource.nProductID;
		arProductItemIDs.RemoveAll();
		for(int i = 0; i < ptbSource.arProductItemIDs.GetSize(); i++) {
			arProductItemIDs.Add(ptbSource.arProductItemIDs[i]);
		}
		nQty = ptbSource.nQty;
	}
};

class CSelectLinkedProductsDlg : public CNxDialog
{
// Construction
public:
	CSelectLinkedProductsDlg(CWnd* pParent);   // standard constructor

	//TES 7/16/2008 - PLID 27983 - Input variables
	long m_nCptID;
	double m_dDefaultQty;
	long m_nLocationID;
	CString m_strCptName;

	// (j.jones 2010-11-24 08:55) - PLID 41549 - added a flag for when billing a package,
	// as they are not permitted to change the quantity
	BOOL m_bIsPackage;

	//TES 7/16/2008 - PLID 27983 - Output variables.
	CArray<ProductToBill,ProductToBill&> m_arProductsToBill;

	//TES 7/16/2008 - PLID 27983 - Change our various messages if we're on a case history.
	bool m_bIsCaseHistory;

// Dialog Data
	//{{AFX_DATA(CSelectLinkedProductsDlg)
	enum { IDD = IDD_SELECT_LINKED_PRODUCTS_DLG };
	CNxStatic	m_nxsTitle;
	CNxIconButton	m_nxbOK;
	CNxIconButton	m_nxbCancel;
	CNxIconButton	m_nxbUnbillProduct;
	CNxStatic	m_nxsSelectLinkedProductsCaption;
	CNxIconButton	m_nxbBillProduct;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectLinkedProductsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pAvailableProducts, m_pProductsToBill;

	void EnableButtons();
	void BillProduct(long nProductID = -1, long nProductItemID = -1);
	void UnbillProduct();
	//TES 7/16/2008 - PLID 27983 - Refresh the screen to be in sync with m_arProductsToBill
	void RefreshProductsToBill();

	//(c.copits 2010-09-30) PLID 40317 - Allow duplicate UPC codes for FramesData certification.
	NXDATALIST2Lib::IRowSettingsPtr GetBestUPCProduct(_variant_t vBarcode);

	// Generated message map functions
	//{{AFX_MSG(CSelectLinkedProductsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBillProduct();
	afx_msg void OnSelChangedLinkableProducts(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnDblClickCellLinkableProducts(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnSelChangedProductsToBill(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnDblClickCellProductsToBill(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnUnbillProduct();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnEditingStartingProductsToBill(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishingProductsToBill(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedProductsToBill(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnRequeryFinishedLinkableProducts(short nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTLINKEDPRODUCTSDLG_H__98021F07_F00E_48AB_9030_78A4D5DBB572__INCLUDED_)

#if !defined(AFX_LINKPRODUCTSTOSERVICESDLG_H__A7F8E7DE_C96F_4849_AE68_325F3A99A01A__INCLUDED_)
#define AFX_LINKPRODUCTSTOSERVICESDLG_H__A7F8E7DE_C96F_4849_AE68_325F3A99A01A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LinkProductsToServicesDlg.h : header file
//

//TES 7/16/2008 - PLID 27983 - Created
/////////////////////////////////////////////////////////////////////////////
// CLinkProductsToServicesDlg dialog

class CLinkProductsToServicesDlg : public CNxDialog
{
// Construction
public:
	CLinkProductsToServicesDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLinkProductsToServicesDlg)
	enum { IDD = IDD_LINK_PRODUCTS_TO_SERVICES };
	CNxStatic	m_nxsDescription;
	CNxIconButton	m_nxbUnlinkProduct;
	CNxIconButton	m_nxbLinkProduct;
	CNxIconButton	m_nxbClose;
	NxButton	m_nxbIncludeUnlinked;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLinkProductsToServicesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pCptList, m_pAvailableProducts, m_pLinkedProducts;

	//TES 7/16/2008 - PLID 27983 - The currently selected CPT Code (maintained while the list requeries.
	long m_nCurrentCptID;

	//TES 7/16/2008 - PLID 27983 - Enable the left and right buttons.
	void EnableButtons();
	//TES 7/16/2008 - PLID 27983 - Filter the product lists based on the currently selected CPT Code.
	void ReflectCurrentCpt();

	//TES 7/16/2008 - PLID 27983 - Actually link/unlink the currently selected product.
	void LinkProduct();
	void UnlinkProduct();

	// Generated message map functions
	//{{AFX_MSG(CLinkProductsToServicesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnIncludeUnlinked();
	afx_msg void OnRequeryFinishedCptList(short nFlags);
	afx_msg void OnSelChosenCptList(LPDISPATCH lpRow);
	afx_msg void OnSelChangedAvailableProducts(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnDblClickCellAvailableProducts(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnClose();
	afx_msg void OnLinkProduct();
	afx_msg void OnSelChangedLinkedProducts(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnDblClickCellLinkedProducts(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnUnlinkProduct();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LINKPRODUCTSTOSERVICESDLG_H__A7F8E7DE_C96F_4849_AE68_325F3A99A01A__INCLUDED_)

#if !defined(AFX_INVCONSIGNMENTPURCHASEDLG_H__F4321C41_C691_4594_A039_728D18841C69__INCLUDED_)
#define AFX_INVCONSIGNMENTPURCHASEDLG_H__F4321C41_C691_4594_A039_728D18841C69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvConsignmentPurchaseDlg.h : header file
//
// (c.haag 2007-11-29 10:33) - PLID 28006 - Initial implementation

/////////////////////////////////////////////////////////////////////////////
// CInvConsignmentPurchaseDlg dialog

class CInvConsignmentPurchaseDlg : public CNxDialog
{
// Construction
public:
	CInvConsignmentPurchaseDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInvConsignmentPurchaseDlg)
	enum { IDD = IDD_INV_CONSIGNMENT_PURCHASE_DLG };
	CNxStatic	m_nxstaticTotal;
	CNxStatic	m_nxstaticTotalAmount;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvConsignmentPurchaseDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2007-11-29 17:01) - PLID 28237 - TRUE when the dialog has been initialized
	BOOL m_bInitialized;

public:
	// (c.haag 2007-11-29 17:01) - PLID 28237 - TRUE if we are in the middle of processing
	// a barcode scannage
	BOOL m_bIsScanning;

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_PurchaseList;		//the list of consignment items the user wishes to purchase
	NXDATALIST2Lib::_DNxDataListPtr m_ConsignmentCombo;	//the dropdown list of filtered consigned items
	NXDATALIST2Lib::_DNxDataListPtr m_CategoryCombo;	//the dropdown list of all categories that are in use by any serialized product
	NXDATALIST2Lib::_DNxDataListPtr m_ProductCombo;		//the dropdown list of all active serializeable products
	NXDATALIST2Lib::_DNxDataListPtr m_SupplierCombo;	//the dropdown list of all suppliers
	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo;	//the dropdown list of all active, managed locations

protected:
	// (c.haag 2007-11-29 17:00) - PLID 28237 - Returns the formatted string of the
	// query used for the inventory list filter as well as barcode searches
	CString GetBaseQuery();

	// (c.haag 2007-12-05 09:06) - PLID 28237 - Returns the ID of the currently
	// selected location
	long GetLocationID();

	// (c.haag 2007-12-05 09:06) - PLID 28237 - Returns the name of a location given its ID
	CString GetLocationName(long nLocationID);

	// (c.haag 2007-11-28 16:11) - PLID 28006 - Bind all datalist members to
	// their corresponding form controls and add sentinel values where necessary
	void InitDatalists();

	// (c.haag 2007-11-28 17:32) - PLID 28006 - Updates the filter on
	// the consignment dropdown and requeries it. This code was imported
	// from InvConsignmentDlg.cpp
	void ReFilterConsignmentCombo();

	// (c.haag 2007-11-29 08:57) - PLID 28006 - Calculates the total of all
	// purchased items and populates the total static form control with it
	void CalculateTotal();

protected:
	// (c.haag 2007-11-29 09:32) - PLID 28006 - Validate the data before saving.
	// This is actually simple; all we do is make sure the list isn't empty. The
	// only editable field in the list is the amount, and the editing handlers already
	// prevent negative values or other weird values from happening.
	BOOL Validate();

	// (c.haag 2007-11-29 09:36) - PLID 28006 - This function will save the consignment
	// purchases, or throw an exception on failure
	void Save();

protected:
	// Generated message map functions
	//{{AFX_MSG(CInvConsignmentPurchaseDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnSelChosenConsignCategoryCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenConsignLocationCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenConsignProductCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenConsignSupplierCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenConsignmentCombo(LPDISPATCH lpRow);
	afx_msg void OnEditingFinishingPurchaseList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnRButtonDownPurchaseList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRemoveReturnItem();
	afx_msg void OnEditingFinishedPurchaseList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVCONSIGNMENTPURCHASEDLG_H__F4321C41_C691_4594_A039_728D18841C69__INCLUDED_)

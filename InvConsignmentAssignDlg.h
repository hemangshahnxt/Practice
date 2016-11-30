#if !defined(AFX_INVCONSIGNMENTASSIGNDLG_H__9B105160_EB7E_44D1_8FE5_CDAC491070A2__INCLUDED_)
#define AFX_INVCONSIGNMENTASSIGNDLG_H__9B105160_EB7E_44D1_8FE5_CDAC491070A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvConsignmentAssignDlg.h : header file
//
// (c.haag 2007-11-29 10:33) - PLID 28050 - Initial implementation

/////////////////////////////////////////////////////////////////////////////
// CInvConsignmentAssignDlg dialog

class CInvConsignmentAssignDlg : public CNxDialog
{
// Construction
public:
	CInvConsignmentAssignDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CInvConsignmentAssignDlg)
	enum { IDD = IDD_INV_CONSIGNMENT_ASSIGN_DLG };
	CNxIconButton	m_removeAllBtn;
	CNxIconButton	m_removeBtn;
	CNxIconButton	m_addAllBtn;
	CNxIconButton	m_addBtn;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvConsignmentAssignDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2007-11-29 16:16) - PLID 28236 - TRUE when the dialog has been initialized
	BOOL m_bInitialized;

public:
	// (c.haag 2007-11-29 16:16) - PLID 28236 - TRUE if we are in the middle of processing
	// a barcode scannage
	BOOL m_bIsScanning;


protected:
	NXDATALIST2Lib::_DNxDataListPtr m_InventoryList;	//the list of available inventory items to move to consignment
	NXDATALIST2Lib::_DNxDataListPtr m_ConsignmentList;	//the list of selected inventory items to move to consignment
	NXDATALIST2Lib::_DNxDataListPtr m_CategoryCombo;	//the dropdown list of all categories that are in use by any serialized product
	NXDATALIST2Lib::_DNxDataListPtr m_ProductCombo;		//the dropdown list of all active serializeable products
	NXDATALIST2Lib::_DNxDataListPtr m_SupplierCombo;	//the dropdown list of all suppliers
	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo;	//the dropdown list of all active, managed locations

protected:
	// (c.haag 2007-11-29 16:26) - PLID 28236 - Returns the formatted string of the
	// query used for the inventory list filter as well as barcode searches
	CString GetBaseQuery();

	// (c.haag 2007-12-05 08:18) - PLID 28236 - Returns the ID of the currently
	// selected location
	long GetLocationID();

	// (c.haag 2007-12-05 08:18) - PLID 28236 - Returns the name of a location given its ID
	CString GetLocationName(long nLocationID);

	// (c.haag 2007-11-29 10:20) - PLID 28050 - Bind all datalist members to
	// their corresponding form controls and add sentinel values where necessary
	void InitDatalists();

	// (c.haag 2007-11-29 10:24) - PLID 28050 - Updates the filter on
	// the inventory list and requeries it. This code was copied almost
	// verbatim from InvConsignmentPurchaseDlg.cpp
	void ReFilterInventoryList();

	// (c.haag 2007-11-29 15:24) - PLID 28050 - Updates the enabled state of icon buttons
	void UpdateButtons();

	// (c.haag 2007-11-29 12:02) - PLID 28050 - Ensure that it's OK to save the user's
	// selections
	BOOL Validate();

	// (c.haag 2007-11-29 12:04) - PLID 28050 - This function will save the consigment
	// transfer, or throw an exception on failure
	void Save();

protected:
	// Generated message map functions
	//{{AFX_MSG(CInvConsignmentAssignDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnAddAll();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnRemoveAll();
	afx_msg void OnDblClickCellInventoryList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnDblClickCellConsignmentList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnRequeryFinishedInventoryList(short nFlags);
	afx_msg void OnSelChosenConsignCategoryCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenConsignLocationCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenConsignProductCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenConsignSupplierCombo(LPDISPATCH lpRow);
	afx_msg void OnCurSelWasSetConsignmentList();
	afx_msg void OnCurSelWasSetInventoryList();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVCONSIGNMENTASSIGNDLG_H__9B105160_EB7E_44D1_8FE5_CDAC491070A2__INCLUDED_)

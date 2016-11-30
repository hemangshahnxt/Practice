#if !defined(AFX_ORDERDLG_H__1793AEF9_4ABE_4189_896D_28F117FACDBC__INCLUDED_)
#define AFX_ORDERDLG_H__1793AEF9_4ABE_4189_896D_28F117FACDBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OrderDlg.h : header file
//
#include "InvEditOrderDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CInvOrderDlg dialog

// (c.haag 2010-01-14 10:20) - PLID 30503 - Added iocPOBox (and iocCostBeforeDiscounts was missing too)
enum InvOrderColumns {
	iocID = 0,
	iocSupplier = 1,
	iocPOBox = 2,
	iocDescription = 3,
	iocTrackingNum = 4,
	iocDatePlaced = 5,
	iocDateArrived = 6,
	iocDateDue = 7,
	iocTotalCost = 8,
	iocCostBeforeDiscounts = 9,
	iocNotes = 10,
};

class CInvOrderDlg : public CNxDialog
{
// Construction
public:
	//TES 6/18/2008 - PLID 29578 - This function is dead code
	//BOOL ReceiveProductItems(long OrderID);
	CInvOrderDlg(CWnd* pParent);   // standard constructor
	virtual ~CInvOrderDlg();

	NXDATALISTLib::_DNxDataListPtr m_SupplierFilter;

	// (c.haag 2007-11-09 10:54) - PLID 27992 - Supplier filter for product returns
	NXDATALIST2Lib::_DNxDataListPtr m_SupplierFilterReturns;

	virtual void Refresh();
	void RefreshOrders(); // (c.haag 2007-11-14 10:37) - PLID 27992 - Requery the order list
	void RefreshReturns(); // (c.haag 2007-11-14 09:49) - PLID 27992 - Requery the return list
	
	// (j.jones 2008-02-07 10:20) - PLID 28851 - there are two types of auto-ordering,
	// purchased inventory and consignment, so require a boolean to be sent in to determine
	// which "auto order" to call
	void CreateAutoOrder(long supplierID, BOOL bConsignmentOrder);
	
	// (c.haag 2007-12-05 14:49) - PLID 28286 - Creates an order
	// (j.jones 2008-03-18 14:44) - PLID 29309 - added appt. and location IDs as optional parameters
	void CreateOrder(BOOL bSaveAllAsReceived, long nApptID = -1, long nLocationID = -1);
	// (r.wilson 2012-2-17 ) - PLID 47393 
	void CInvOrderDlg::CreateFramesOrder(BOOL bSaveAllAsReceived, long nApptID /*= -1*/, long nLocationID /*= -1*/);

	BOOL ValidateUU_UOStatus(long nOrderID);

	//TES 7/22/2008 - PLID 30802 - Provide access to our CInvEditOrderDlg.
	CInvEditOrderDlg* GetEditOrderDlg();

	//(r.wilson 2-20-2012) PLID 48222
	void CInvOrderDlg::SetReturnedOrderId(long Id);
	
	//(r.wilson 2-20-2012) PLID 48222
	void CInvOrderDlg::PrintOrderLabels(BOOL bIsFramesLabel,long nOrderId);

// Dialog Data
	// (j.gruber 2008-06-30 09:40) - PLID 29205 - added date options
	// (j.jones 2009-03-17 13:32) - PLID 32831 - added m_btnSupplierStatements
	// (j.jones 2009-03-17 13:38) - PLID 32832 - added m_btnReconcileConsignment
	// (c.haag 2010-01-14 10:07) - PLID 30503 - Added m_chkShowPONumber
	//{{AFX_DATA(CInvOrderDlg)
	enum { IDD = IDD_INVORDER };
	CNxIconButton	m_nxbToBeOrdered;
	NxButton	m_chkUseDateFilter;
	NxButton	m_chkShowPONumber;
	CDateTimePicker	m_dtTo;
	CDateTimePicker	m_dtFrom;
	CNxIconButton	m_nxbToBeReturned;
	CNxIconButton	m_returnBtn;
	CNxIconButton	m_needBtn;
	CNxIconButton	m_createBtn;
	CNxStatic	m_nxstaticLabelReturnShow;
	CNxStatic	m_nxstaticLabelReturnSupplier;
	CNxLabel	m_nxlDateLabel;
	CNxIconButton	m_btnSupplierStatements;
	CNxIconButton	m_btnReconcileConsignment;
	CNxIconButton m_ReceiveFrameOrderBtn;
	// (s.tullis 2014-08-12 14:09) - PLID 63241 - The InvOrderDlg needs a CTableChecker object for Suppliers.
	CTableChecker m_SuppliersChecker;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvOrderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	CBrush m_brush;
	long m_rightClicked;
	NXDATALISTLib::_DNxDataListPtr m_list;
	// (c.haag 2007-11-09 10:35) - PLID 27992 - Added a list for inventory returns
	NXDATALIST2Lib::_DNxDataListPtr m_listReturns;
	CInvEditOrderDlg *m_pEditOrderDlg;
	void SetWhereClause();
	// (c.haag 2007-11-09 10:49) - PLID 27992 - This is just like SetWhereClause but for returns
	void SetWhereClause_Returns();
	//DRT 12/4/2007 - PLID 28235
	void CInvOrderDlg::ReceiveCurrentRow(BSTR bstr = NULL);
	// (s.dhole 2010-07-16 16:47) - PLID 28183 It would be nice to be able to print Avery 8167 labels from the order screen by right clicking print, and then have it come up with an option to select which products need labels, and to print the labels quantity based on the # received.
    void CInvOrderDlg::PrintOrderLabels(BOOL bIsFramesLabel);
	// (s.tullis 2014-08-12 14:09) - PLID 63241 - The InvOrderDlg needs a CTableChecker object for Suppliers.
	void EnsureUpdatedSuppliers();
	// (j.gruber 2008-06-30 10:54) - PLID 29205 - date filters
	long m_nDateFilterID;
	COleDateTime m_dtToDate;
	COleDateTime m_dtFromDate;
	BOOL CheckDates();
	// (j.gruber 2008-06-30 11:42) - PLID 30564 - changed the radio buttons to datalists
	NXDATALIST2Lib::_DNxDataListPtr m_pOrderTypeFilterList;
	NXDATALIST2Lib::_DNxDataListPtr m_pReturnTypeFilterList;
	long m_nOrderTypeFilterID;
	long m_nReturnTypeFilterID;

	// (c.haag 2010-01-14 10:16) - PLID 30503 - This utility function will show or hide columns based on preference.
	void ReflectColumnAppearances();

	//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
	ADODB::_RecordsetPtr GetBestUPCProduct(CString strCode);

	// (j.jones 2009-03-17 13:32) - PLID 32831 - added OnBtnSupplierStatements
	// (j.jones 2009-03-17 13:38) - PLID 32832 - added OnBtnReconcileConsignment
	// Generated message map functions
	//{{AFX_MSG(CInvOrderDlg)
	virtual BOOL OnInitDialog();
	
	afx_msg void OnRButtonUpOrderList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnNeed();
	afx_msg void OnCreateOrder();
	afx_msg void OnDblClickCellOrderList(long nRowIndex, short nColIndex);
	afx_msg void OnSelChosenSupplierFilter(long nRow);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBtnNewReturn();
	afx_msg void OnDblClickCellReturnList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnSelChosenSupplierFilterReturn(LPDISPATCH lpRow);
	afx_msg void OnRButtonDownReturnList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnOpenReturnGroup();
	afx_msg void OnDeleteReturnGroup();
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTrySetSelFinishedSupplierFilter(long nRowEnum, long nFlags);
	afx_msg void OnTrySetSelFinishedSupplierFilterReturn(long nRowEnum, long nFlags);
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnToBeReturned();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSelChangingInvOrderReturnFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenInvOrderReturnFilter(LPDISPATCH lpRow);
	afx_msg void OnInvOrderUseDate();
	afx_msg void OnSelChangingInvOrderOrderFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenInvOrderOrderFilter(LPDISPATCH lpRow);
	afx_msg void OnDatetimechangeInvOrderToDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeInvOrderFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnToBeOrdered();
	afx_msg void OnBtnSupplierStatements();
	afx_msg void OnBtnReconcileConsignment();
	afx_msg void OnBnClickedRecieveFrames();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedCheckShowponumber();
	//afx_msg void OnBnClickedRecieveFrames();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ORDERDLG_H__1793AEF9_4ABE_4189_896D_28F117FACDBC__INCLUDED_)

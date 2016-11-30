#if !defined(AFX_INVALLOCATIONUSAGEDLG_H__EA06B4A9_38AF_434A_8186_6ECC69A64CC9__INCLUDED_)
#define AFX_INVALLOCATIONUSAGEDLG_H__EA06B4A9_38AF_434A_8186_6ECC69A64CC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvAllocationUsageDlg.h : header file
//

// (j.jones 2007-11-27 09:40) - PLID 28196 - created

#include "InvUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CInvAllocationUsageDlg dialog

class CInvAllocationUsageDlg : public CNxDialog
{
// Construction
public:
	CInvAllocationUsageDlg(CWnd* pParent);   // standard constructor
	~CInvAllocationUsageDlg();	//standard destructor

	//SetAllocationInfo takes in either a pointer to allocation info, or an ID
	void SetAllocationInfo(InvUtils::AllocationMasterInfo *pAllocationMasterInfo);
	void SetAllocationInfo(long nAllocationID);

	//GetAllocationInfo returns the pointer to the allocation info
	InvUtils::AllocationMasterInfo* GetAllocationInfo();

	BOOL m_bIsCompletedByBill;			//TRUE if a bill is completing this allocation
	BOOL m_bIsCompletedByCaseHistory;	//TRUE if a case history is completing this allocation

	//SetInitialProductInfo takes in a product ID, name, and optional quantity and populates the
	//"Initial" member variables for all three
	// (j.jones 2008-06-11 15:31) - PLID 28379 - added ability to force selection of a detail
	//TES 7/16/2008 - PLID 27983 - Added ability to force selection of multiple details by ProductItemID
	void SetInitialProductInfo(long nProductID, CString strProductName, double dblQuantity = 1.0, long nAutoUseDetailID = -1, OPTIONAL IN CArray<long,long> *parAutoUseProductItemIDs = NULL);

	BOOL m_bForceCompletion;	//set to true if we want the user to finalize all remaining products and close the allocation

	BOOL m_bFreeInfoObject;		//if FALSE, destruction of this dialog will not clear m_pAllocationMasterInfo
	BOOL m_bSaveToData;			//if FALSE, clicking OK on this dialog will not commit to data

	//track pointers of details marked as "billed" in the current edit session
	CArray<InvUtils::AllocationDetailInfo*, InvUtils::AllocationDetailInfo*> m_paryDetailsBilled;

	void OnRadioBarcodeChanged();

// Dialog Data
	//{{AFX_DATA(CInvAllocationUsageDlg)
	enum { IDD = IDD_INV_ALLOCATION_USAGE_DLG };
	CNxIconButton	m_btnSaveLeaveUncompleted;
	CNxIconButton	m_btnSaveAndComplete;
	CNxIconButton	m_btnCancel;
	NxButton	m_radioBarcodeReleased;
	NxButton	m_radioBarcodeUsed;
	CNxEdit	m_nxeditAllocPatientName;
	CNxEdit	m_nxeditLocationName;
	CNxEdit	m_nxeditCreateDate;
	CNxEdit	m_nxeditApptInfo;
	CNxEdit	m_nxeditAllocationNotes;
	CNxStatic	m_nxstaticHeaderLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvAllocationUsageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CBrush m_brush;	//used for label painting

	NXDATALIST2Lib::_DNxDataListPtr m_AllocationList;	//the list of allocated products

	InvUtils::AllocationMasterInfo* m_pAllocationMasterInfo;	//a struct of the allocation info.

	long m_nAllocationID;	//the ID of the allocation info

	long m_nInitialProductID;		 //an optional passed-in ID to tell us what product fired this allocation
	CString m_strInitialProductName; //an optional passed-in string to tell us what product fired this allocation
	double m_dblInitialQuantity;	 //an optional, passed-in field to determine what the original quantity requested was
	
	// (j.jones 2008-06-11 15:31) - PLID 28379 - added ability to force selection of a detail
	long m_nInitialAutoUseDetailID;
	void TryResolveInitialDetailID();

	//TES 7/16/2008 - PLID 27983 - Added an optional list of ProductItemIDs to pre-fill as "used" on the allocation.
	CArray<long,long>* m_parInitialAutoUseProductItemIDs;

	//TryPopulateAllocationInfo will populate m_pAllocationMasterInfo based on m_nAllocationID,
	//if m_nAllocationID is not -1 and m_pAllocationMasterInfo is NULL
	void TryPopulateAllocationInfo();

	//LoadAllocationInfo will call TryPopulateAllocationInfo,
	//and then reflect the contents of m_pAllocationMasterInfo on screen
	//(returns FALSE if it couldn't be loaded)
	BOOL LoadAllocationInfo();

	//TryResolveInitialProductID will try to auto "use" the passed in m_nInitialProductID,
	//if one was given to us, if only one product matches on the allocation
	void TryResolveInitialProductID();

	//TryAddDetailToBilledList will add a given detail object to the
	//m_paryDetailsBilled list if it isn't already in the list
	void TryAddDetailToBilledList(InvUtils::AllocationDetailInfo *pInfo);
	//TryRemoveDetailFromBilledList will remove a given detail object from
	//the m_paryDetailsBilled list, if it is in the list
	void TryRemoveDetailFromBilledList(InvUtils::AllocationDetailInfo *pInfo);

	//VerifyAllocationCompletedOnClose will see if the allocation is
	//fully completed, force it to be completed if not, and return FALSE
	//if the user refused to complete the allocation. TRUE otherwise.
	BOOL VerifyAllocationCompletedOnClose();

	//CheckForUnbilledProducts will see if there are any products in the list
	//marked as used, but not billed
	BOOL CheckForUnbilledProducts();

	//ResolveAllUnresolvedProducts will mark any Active products as Used or Released, based on the parameter
	//TES 7/21/2008 - PLID 29478 - This now can return FALSE, in which case the user was told the reason for the failure.
	BOOL ResolveAllUnresolvedProducts(BOOL bMarkUsed);

	// (j.jones 2008-02-19 16:17) - PLID 28948 - split into two save functions
	BOOL SaveCompleted();
	BOOL SaveUncompleted();

	//PromptForQtyChange will take in a boolean for whether we're using or releasing a given product,
	//takes in the product name and current quantity, and then passes back the quantity they are using,
	//which must be greater than zero and not greater than the current quantity.
	//Return FALSE if the user cancelled, return TRUE if we are returning a valid new quantity.
	BOOL PromptForQtyChange(BOOL bUsed, CString strProductName, double dblCurQuantity, double &dblNewQuantity);

	//TES 7/21/2008 - PLID 29478 - Selects a ProductItemsT record to assign to the specified row.  A return value of FALSE means
	// that the user cancelled, and was told that this would prevent them from changing the item's status.
	BOOL PromptSelectItem(LPDISPATCH pRow);

	//TrySplitByQuantity will take in a row, and if it has multiple quantity, prompt for the amt. to use,
	//and create a new row with the balance - returns FALSE if the process should be aborted
	BOOL TrySplitByQuantity(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bMarkingUsed);

	//takes in an AllocationDetailInfo pointer and adds a new datalist row for it
	void AddDatalistRowFromDetail(InvUtils::AllocationDetailInfo *pInfo, BOOL bDisableIfBilled = TRUE);

	// (j.jones 2007-12-13 08:57) - PLID 27988 - If we are trying to bill a product,
	// and the bill/case is requiring a specific product, ensure we are only billing
	// one type of ProductID, not more than one. If so, warn (if !bSilent), and return FALSE;
	BOOL CanBillThisProduct(long nProductID, BOOL bSilent = FALSE);

	//TES 7/18/2008 - PLID 29478 - We may pop up the ProductItemsDlg, in which case we'll want to not handle barcode scans,
	// and pass in a list of ProductItemIDs that are in use on the dialog.
	BOOL m_bDisableBarcode;
	CString GetExistingProductItemWhereClause(long nServiceID);

	// (j.jones 2008-02-19 14:23) - PLID 28948 - added OnBtnSaveAndComplete and OnBtnSaveLeaveUncompleted

	// Generated message map functions
	//{{AFX_MSG(CInvAllocationUsageDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEditingFinishedAllocatedItemsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingStartingAllocatedItemsList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishingAllocatedItemsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnRadioBarcodeUsed();
	afx_msg void OnRadioBarcodeReleased();
	afx_msg void OnBtnSaveAndComplete();
	afx_msg void OnBtnSaveLeaveUncompleted();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVALLOCATIONUSAGEDLG_H__EA06B4A9_38AF_434A_8186_6ECC69A64CC9__INCLUDED_)

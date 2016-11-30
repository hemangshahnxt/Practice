#if !defined(AFX_PRODUCTITEMSDLG_H__FA159757_0E33_4E08_B5EB_1D53DF7C7BB5__INCLUDED_)
#define AFX_PRODUCTITEMSDLG_H__FA159757_0E33_4E08_B5EB_1D53DF7C7BB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProductItemsDlg.h : header file
//

#define PI_ENTER_DATA	0
#define PI_READ_DATA	1
#define	PI_SELECT_DATA	2
#define PI_RETURN_DATA	3	// (j.jones 2008-06-06 09:14) - PLID 27110

/////////////////////////////////////////////////////////////////////////////
// CProductItemsDlg dialog

// (c.haag 2007-11-08 16:18) - PLID 28025 - Limited various member variables to being protected 
class CProductItemsDlg : public CNxDialog
{
// Construction
protected:
	NXDATALISTLib::_DNxDataListPtr m_List;
	NXDATALISTLib::_DNxDataListPtr m_SelectedList;

public:
	BOOL m_bAllowQtyGrow;			//determines whether or not the user is warned when the quantity is increased
	BOOL m_bDisallowQtyChange;		//disables the ability to change the quantity
	BOOL m_bIsAdjustment;			//really is only used to change what the labels say
	BOOL m_bIsTransfer;				//also used to change what the labels say
	long m_NewItemCount;			//count of items received in an order, that need info filled in
	long m_CountOfItemsNeeded;		//count of items to be relieved
	long m_ProductID;				//the ProductT.ID
	long m_nOrderDetailID;			//TES 6/18/2008 - PLID 29578 - Changed from OrderID to OrderDetailID
	CString m_strCreatingAdjustmentID;	// (j.jones 2009-03-09 12:17) - PLID 33096 - added m_strCreatingAdjustmentID
	long m_nLocationID;				//the location to filter on, if any
	BOOL m_bDisallowLocationChange;	// PLID 28438 - If TRUE, the location column will be read-only

	// (c.haag 2008-06-04 10:54) - PLID 29013 - If TRUE, then the user will not be able to select
	// < No Location > as a location.
	BOOL m_bDisallowNonLocations;

	long m_PrevItemCount;			//count of items that existed before ChangeQuantity to allow them to be removed if cancelling

	BOOL m_bUseUU;					//passed in to determine if we are using UU/UO
	BOOL m_bSerialPerUO;
	long m_nConversion;

	// (j.jones 2007-11-09 09:30) - PLID 28030 - added setting for the default consignment value on new entries
	// (c.haag 2007-12-03 11:32) - PLID 28204 - Rather than a boolean consignment, this is now an integer flag
	long m_nDefaultStatus;

	// (j.jones 2007-11-08 11:04) - PLID 28041 - added optional display text override fields
	CString m_strOverrideTitleBarText;	//if filled, will override the title bar text with its contents
	CString m_strOverrideSelectQtyText;	//if filled, will override the "quantity to be billed" text with its contents,
										//only when the entry type is PI_SELECT_DATA
	// (c.haag 2007-11-14 09:04) - PLID 28025 - Override for the "Please select an item..." text
	CString m_strOverrideSelectItemText;

	CString m_strWhere;				//used in billing to remove products from the list that as assgined but unsaved

	CDWordArray m_adwProductItemIDs;//when selecting items, this list will propagate with the IDs of the selected items

	// (j.jones 2009-07-09 17:09) - PLID 32684 - m_bDeclareNewProductItemID will control whether
	// the Save() function adds a declaration for @nNewProductItemID when generating sql batches
	BOOL m_bDeclareNewProductItemID;

	//DRT 1/8/2008 - PLID 28473
	long GetLabelForItemStatus();

	// (a.walling 2008-03-20 10:04) - PLID 29333 - Change the quantity of items to enter data for
	void ChangeQuantity(long nNewQuantity);
	void GetQuantity(long &nQuantity, long &nConversion);

protected:
	// (c.haag 2007-11-08 16:08) - PLID 28025 - When items are selected and the dialog is dismissed,
	// every entry in m_adwProductItemIDs also has a corresponding serial number and expiration date
	// in these variant arrays. Here are the available variant types:
	//
	// VT_NULL - The corresponding product item no longer exists in data or is invalid
	// not VT_NULL - The corresponding product item is valid
	CVariantArray m_avProductItemSerials;
	CVariantArray m_avProductItemExpDates;
	// (c.haag 2008-03-11 12:45) - PLID 29255 - We now allow callers to fetch the status value as well
	CVariantArray m_avProductItemStatus;

public:
	// (c.haag 2007-11-08 17:02) - PLID 28025 - These functions provide access to the variant
	// arrays above
	_variant_t GetSelectedProductItemSerialNum(int nIndex);
	_variant_t GetSelectedProductItemExpDate(int nIndex);
	// (c.haag 2008-03-11 12:46) - PLID 29255 - Returns the status of a selected product item.
	// A valid return value is of type VT_I4.
	_variant_t GetSelectedProductItemStatus(int nIndex);
	// (c.haag 2008-02-15 09:47) - PLID 28286 - Returns the number of selected product items.
	long GetSelectedProductItemCount() const;

public:
	BOOL m_bUseSerial, m_bUseExpDate;
	int m_EntryType;				//defines if we are viewing items, entering items, or relieving items
	//TES 7/7/2008 - PLID 24726 - Track whether our serial number is actually a lot number.  There doesn't seem to be any
	// reason for this to be public at the present time.
	// (j.jones 2010-05-07 08:59) - PLID 36454 - this is public now, a variant so we know if it is or is not provided to us
	_variant_t m_varSerialNumIsLotNum;

public:
	// (c.haag 2007-12-07 15:05) - PLID 28286 - Added a silent flag. At the time of this implementation,
	// this can only happen if a user has entered an order for items already received...in which case
	// the verification has already happened once before.
	BOOL Save(BOOL bSilent = FALSE);

protected:
	// (c.haag 2007-12-05 16:18) - PLID 28286 - This was a feature for 28025 moved into its own function
	void RememberProductItemValues(NXDATALISTLib::_DNxDataListPtr& dlList);

public:
	CProductItemsDlg(CWnd* pParent);   // standard constructor

protected:
	void SetColumnWidths();
	void ShowRelevantFields();
	void ConfigureDisplay();
	void PreLoadValues();

	BOOL VerifyLocationChange(long nOldLocationID, long nNewLocationID);
	// (c.haag 2007-12-07 15:05) - PLID 28286 - Added a silent flag. At the time of this implementation,
	// this can only happen if a user has entered an order for items already received...in which case
	// the verification has already happened once before.
	BOOL VerifyLocationSave(BOOL bSilent);


public:
	//DRT 11/12/2007 - PLID 27999 - This boolean controls whether the SQL code is executed immediately in the attempt to close the dialog, 
	//	or whether it is saved in m_strSavedDataEntryQuery.
	bool m_bSaveDataEntryQuery;

	//DRT 11/6/2007 - PLID 27999 - See comments on PI_ENTER_DATA_GENERATE_SQL.  This is ONLY used if you are in
	//	that entry type.
	CString m_strSavedDataEntryQuery;

	//DRT 11/6/2007 - PLID 28018  - Allow the dialog to be created in a modeless manner.  Note that the dialog will
	//	still actually "act" modal -- that is, it will disable the parent so that no input can be given to a window
	//	other than this one.  This is the same behavior as billing.  This was designed to work for the 2 PI_ENTER... types, 
	//	and has not been tested for any other varieties of this dialog.
	bool m_bModeless;

	//DRT 11/7/2007 - PLID 28022 - This function will check through the datalist and determine how many of the rows currently
	//	contain 'valid' text, by checking both the serial number and expiration date.  This function is only valid when
	//	in the PI_ENTER... methods.
	long CountValidRows();

	//DRT 11/9/2007 - PLID 28054 - This applies to PI_ENTER... data entry types only.  If this is set, you will get a close button
	//	instead of an OK/Cancel button.
	bool m_bUseCloseOnEntryType;

	//DRT 11/20/2007 - PLID 28138 - This tells the dialog to fire off OnOK() after any barcode scan happens.  It is not recommended that
	//	you use this without m_bSaveDataEntryQuery = true.  This is designed primarily for the receive order screen, so the user
	//	can work in a "scan only" mode, choosing product barcode then serial number.
	bool m_bCloseAfterScan;

	//DRT 11/20/2007 - PLID 28138 - I had to add this to combat some scanning issues with this dialog.
	bool m_bCurrentlyReceivingScans;

	// (c.haag 2007-12-05 12:41) - PLID 28237 - Allow the dialog to include allocated product items whose
	// allocations are incomplete
	bool m_bAllowActiveAllocations;

	// (c.haag 2007-12-17 13:42) - PLID 28286 - Allow the dialog to disable editing product item statuses
	bool m_bDisallowStatusChange;

	// (j.jones 2008-02-29 11:56) - PLID 29125 - added an ID that will let us show serial numbers on an allocation that is completed
	long m_nLinkedAllocationID;

	// (j.jones 2009-02-11 08:49) - PLID 32871 - takes in an optional override color
	COLORREF m_clrBkg;

protected:
// Dialog Data	
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	// (j.jones 2009-02-11 08:52) - PLID 32851 - added m_bkg
	//{{AFX_DATA(CProductItemsDlg)
	enum { IDD = IDD_PRODUCT_ITEMS };
	CNxStatic	m_nxsLotNumLabel;
	CNxIconButton	m_nxbApplyLotNumToSelected;
	CNxIconButton	m_nxbApplyLotNum;
	CNxStatic	m_staticScanAs;
	NxButton	m_radioScanGeneral;
	NxButton	m_radioScanConsignment;
	CNxIconButton	m_btnUnselectProduct;
	CNxIconButton	m_btnSelectProduct;
	CNxIconButton	m_btnApplyExpDate;
	CNxIconButton	m_btnApplyDateToSelected;
	CNxIconButton	m_btnCloseProductItems;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CDateTimePicker	m_dtExpDate;
	CNxEdit	m_nxeditQuantity;
	CNxStatic	m_nxstaticSelectItemLabel;
	CNxStatic	m_nxstaticQuantityLabel;
	CNxStatic	m_nxstaticExpDateLabel;
	CNxStatic	m_nxstaticSelectedCountLabel;
	CNxColor	m_bkg;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProductItemsDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	//DRT 11/7/2007 - PLID 28022 - Helper functions to centralize what text is considered 'invalid'.
	bool IsSerialNumberTextValid(CString strText);
	bool IsExpirationTextValid(CString strText);

	// (c.haag 2007-12-05 12:47) - PLID 28237 - Filter a query by allocations based on m_bAllowActiveAllocations.
	CString GetAllocationWhereClause() const;

	// (d.thompson 2009-08-17) - PLID 18899
	void UpdateSelectedItemsCount();

public:
	// (c.haag 2007-12-17 13:43) - PLID 28236 - Allows the caller to set a uniform product item status
	// for every inventory item. Using a long instead of the proper enum to avoid dependency issues
	// with invutils.h
	void SetAllProductItemStatuses(long /* InvUtils::ProductItemStatus */ pis);

public:
	// (c.haag 2008-06-25 11:42) - PLID 28438 - This will change the internal location ID of the dialog,
	// as well as the location ID of every list entry
	void ChangeLocationID(long nLocationID);

protected:
	// (c.haag 2008-06-25 11:42) - PLID 28438 - This will change the location ID of every entry
	// in the specified list
	void ChangeLocationID(NXDATALISTLib::_DNxDataListPtr& dlList, long nLocationID);

protected:
	// (z.manning 2008-07-07 12:49) - PLID 30602 - Added RButtonDown handler for item list
	// Generated message map functions
	//{{AFX_MSG(CProductItemsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBtnApplyExpDate();
	afx_msg void OnBtnApplyExpDateToSelected();
	afx_msg void OnDblClickCellItemList(long nRowIndex, short nColIndex);
	afx_msg void OnBtnSelectProduct();
	afx_msg void OnBtnUnselectProduct();
	afx_msg void OnDblClickCellSelectedItemList(long nRowIndex, short nColIndex);
	afx_msg void OnKillfocusQuantity();
	afx_msg void OnEditingFinishingItemList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishedItemList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBtnApplyLotNum();
	afx_msg void OnBtnApplyLotNumToSelected();
	afx_msg void OnRButtonDownItemList(long nRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRODUCTITEMSDLG_H__FA159757_0E33_4E08_B5EB_1D53DF7C7BB5__INCLUDED_)

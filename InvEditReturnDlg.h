#if !defined(AFX_INVEDITRETURNDLG_H__1F57717C_F1B7_437E_991B_248FC82A6AE4__INCLUDED_)
#define AFX_INVEDITRETURNDLG_H__1F57717C_F1B7_437E_991B_248FC82A6AE4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvEditReturnDlg.h : header file
//
// (c.haag 2007-11-06 16:51) - PLID 27994 - Initial implementation
#include "inventoryrc.h"

/////////////////////////////////////////////////////////////////////////////
// CInvEditReturnDlg dialog

//TES 6/23/2008 - PLID 26152 - Used to initialize the dialog with a list of ProductItems
struct ProductItemReturnInfo {
	long nProductItemID;
	long nProductID;
	CString strProductName;
	CString strSerialNum;
	_variant_t varExpDate;
	//DRT 7/23/2008 - plid 30815 - We need the cost to be available coming in so it can be defaulted for the amount due
	_variant_t varProductLastCost;
};

class CInvEditReturnDlg : public CNxDialog
{
// Construction
public:
	CInvEditReturnDlg(CWnd* pParent);   // standard constructor
	~CInvEditReturnDlg();

	// (j.jones 2009-02-09 15:10) - PLID 32706 - added m_nxstaticTotalItemsReturned
	// (j.jones 2009-02-10 09:57) - PLID 32827 - added m_btnTrackFedex
// Dialog Data
	//{{AFX_DATA(CInvEditReturnDlg)
	enum { IDD = IDD_INVEDITRETURN };
	CNxIconButton	m_btnConfigureReasons;
	CNxIconButton	m_btnConfigureMethods;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnPTSSPreview;
	CString	m_strNotes;
	CString	m_strDescription;
	CString	m_strTrackingNumber;
	CNxEdit	m_nxeditDescription;
	CNxEdit	m_nxeditTrackingNumber;
	CNxEdit	m_nxeditNotes;
	NxButton m_btnVendorConfirmed;
	CNxEdit m_nxeditConfirmationNumber;
	CString m_strConfirmationNumber;
	CNxStatic m_nxstaticTotalItemsReturned;
	CNxIconButton m_btnTrackFedex;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvEditReturnDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (c.haag 2007-11-13 14:37) - PLID 27996 - TRUE when the dialog has been initialized
	BOOL m_bInitialized;

public:
	// (c.haag 2007-11-13 10:33) - PLID 27996 - TRUE if we are in the middle of processing
	// a barcode scannage
	BOOL m_bIsScanning;

protected:
	// (c.haag 2007-11-07 13:24) - The return group ID represented by this dialog
	long m_nID;

protected:
	// (c.haag 2007-11-08 09:55) - When opening an existing return, this is the ID to load
	long m_nReturnGroupIDToLoad;

	//TES 6/23/2008 - PLID 26152 - Lets us create a new return based off some existing serialized items.
	long m_nInitialLocationID;
	long m_nInitialSupplierID;
	CArray<ProductItemReturnInfo,ProductItemReturnInfo&> m_arInitialProductItems;

protected:
	// (c.haag 2007-11-28 10:28) - Normally when the Items dropdown is finished requerying,
	// we want to warn the user if it's empty so that they don't think it's still requerying.
	// However, when opening an existing return, the Item dropdown is requeried automatically
	// based on the select supplier and location ID's. We don't want to warn the user if it's
	// empty at that point because it would be awkward to have it appear when the user didn't
	// actually do anything
	BOOL m_bWarnIfItemDropdownEmpty;

protected:
	// (c.haag 2008-01-07 17:39) - Return group ID fields
	long m_nLocationID;
	long m_nSupplierID;
	long m_nReturnMethodID;

	// (c.haag 2007-11-07 15:03) - If this return group exists in data, these are
	// the fields in data as they were when the dialog was opened
	CString m_strOldDescription;
	CString m_strOldNotes;
	long m_nOldLocationID;
	long m_nOldSupplierID;
	_variant_t m_vOldReturnDate;
	long m_nOldReturnMethodID;
	CString m_strOldTrackingNumber;
	_variant_t m_vOldCompletedDate;
	CString m_strOldConfirmationNumber;
	// (c.haag 2007-11-12 12:42) - PLID 28028 - Retained for auditing only
	CString m_strOldLocationName;
	CString m_strOldSupplierName;
	CString m_strOldReturnMethodName;
	BOOL m_bOldVendorConfirmed;
	_variant_t m_vOldDateConfirmed;

	class COldReturnItem
	{
	public:
		double m_dQuantity;
		COleCurrency m_cyCreditAmt;
		long m_nReasonID;
		BOOL m_bCompleted;
		_variant_t m_vCompletedDate;
		COleCurrency m_cyAmtReceived;
		CString m_strNotes;
		//TES 2/19/2008 - PLID 28954 - Added ReturnedFor
		long m_nReturnedFor;
	public:
		// (c.haag 2007-11-12 13:28) - PLID 28028 - Retained for auditing only
		CString m_strReasonName;
		CString m_strProductName;
		CString m_strSerialNumber;
		long m_nProductItemID;
	};
	// Key is the return item ID, value is the data payload
	CMap<long,long,COldReturnItem*,COldReturnItem*> m_mapOldReturnItems;

	// (c.haag 2007-11-07 12:27) - The array of deleted returns
	CArray<long,long> m_anDeletedReturns;

protected:
	// (c.haag 2007-11-08 10:06) - If an exception was thrown from InitDialog,
	// set this flag to prevent the user from being able to save the return group
	BOOL m_bErrorOnInit;

protected:
	// (c.haag 2007-11-07 09:16) - The locations dropdown
	NXDATALIST2Lib::_DNxDataListPtr m_dlLocationCombo;

	// (c.haag 2007-11-06 17:15) - The supplier dropdown
	NXDATALIST2Lib::_DNxDataListPtr m_dlSupplierCombo;

	// (c.haag 2007-11-06 17:15) - The inventory item selection dropdown
	// (filtered by supplier)
	NXDATALIST2Lib::_DNxDataListPtr m_dlItemCombo;

	// (c.haag 2007-11-08 09:03) - The return method combo
	NXDATALIST2Lib::_DNxDataListPtr m_dlReturnMethodCombo;

	// (c.haag 2007-11-07 11:17) - The list of items being returned
	NXDATALIST2Lib::_DNxDataListPtr m_dlReturnItems;

	// (c.haag 2008-03-03 12:01) - PLID 29176 - The list of possible return reasons
	NXDATALIST2Lib::_DNxDataListPtr m_dlDefReturnReason;

	// (c.haag 2008-03-03 12:01) - PLID 29176 - The list of possible "return for"'s
	NXDATALIST2Lib::_DNxDataListPtr m_dlDefReturnFor;


protected:
	// (c.haag 2007-11-08 09:06) - The date at which the return was placed
	NXTIMELib::_DNxTimePtr m_nxtReturnDate;

	//TES 1/26/2009 - PLID 32824 - The date that the vendor confirmed receipt.
	NXTIMELib::_DNxTimePtr m_nxtDateConfirmed;

protected:
	// (c.haag 2007-11-13 10:13) - PLID 27996 - This utility function removes all items from the
	// return list
	void CInvEditReturnDlg::ClearReturnList();

	// (c.haag 2007-11-09 15:00) - PLID 27994 - This utility function return an interface-friendly
	// name for a return row
	CString CalculateNameForInterface(NXDATALIST2Lib::IRowSettingsPtr& pRow);

	// (c.haag 2007-11-08 15:43) - PLID 27994 - This function compares two variants
	// that should have time values or null values in them
	BOOL DoDateTimesMatch(const _variant_t& v1, const _variant_t& v2);

	// (c.haag 2007-11-08 15:37) - PLID 27994 - This function compares a time variant
	// with an NxTime value. This is specialized for this dialog.
	BOOL DoDateTimesMatch(const _variant_t& vTime, NXTIMELib::_DNxTimePtr& nxt);

protected:
	// (c.haag 2007-11-07 09:30) - PLID 27994 - This is a utility function that
	// requeries the item dropdown given the current location and supplier filters
	void RequeryItemList();

	// (c.haag 2007-11-07 10:23) - PLID 27994 - This utility function returns the
	// currently selected location ID
	long GetLocationID() const;

	// (c.haag 2008-01-07 17:30) - PLID 27994 - This utility function sets the
	// location ID
	void SetLocationID(long nID);

	// (c.haag 2007-11-07 10:23) - PLID 27994 - This utility function returns the
	// currently selected supplier ID
	long GetSupplierID() const;

	// (c.haag 2008-01-07 17:30) - PLID 27994 - This utility function sets the
	// supplier ID
	void SetSupplierID(long nID);

	// (c.haag 2007-11-08 09:22) - PLID 27994 - This utility function returns the
	// currently selected return method ID
	long GetReturnMethodID() const;

	// (c.haag 2008-01-07 17:30) - PLID 27994 - This utility function sets the
	// return method ID
	void SetReturnMethodID(long nID);

	// (c.haag 2007-11-07 12:04) - PLID 27944 - Sets the default values of a new
	// returned item row
	void SetNewRowDefaults(LPDISPATCH lpNewRow);

protected:
	// (c.haag 2007-11-08 14:56) - PLID 27944 - Returns TRUE if any group fields have
	// changed since the dialog was opened and loaded an existing return group
	BOOL HasDialogGroupDataChanged();

	// (c.haag 2007-11-08 15:04) - PLID 27944 - Returns TRUE if any item fields have
	// changed since the dialog was opened and loaded an existing return group
	// (c.haag 2007-11-12 17:45) - We now report whether product adjustment records
	// are retroactively updated
	BOOL HasDialogItemDataChanged(BOOL& bWillAlterExistingProductAdjustments);

	// (c.haag 2007-11-07 15:14) - PLID 27944 - Returns TRUE if anything in the group
	// or any line items has changed, and data needs to be written.
	// (c.haag 2007-11-12 17:45) - We now report whether product adjustment records
	// are retroactively updated
	BOOL HasDialogDataChanged(BOOL& bWillAlterExistingProductAdjustments);

protected:
	// (c.haag 2007-11-08 12:49) - PLID 27994 - Returns TRUE if all the group-level form
	// data is valid and writeable to data
	BOOL ValidateDialogGroupData();

	// (c.haag 2007-11-08 12:58) - PLID 27994 - Returns TRUE if all the individual return item
	// data is valid and can be written to data
	BOOL ValidateDialogItemData();

	// (c.haag 2007-11-07 15:14) - PLID 27944 - Returns TRUE if the dialog data, as
	// is, is valid and writeable to data
	BOOL ValidateDialogData();

	// (c.haag 2007-11-07 15:13) - PLID 27944 - Returns TRUE if data has changed
	// and is valid for saving. Returns FALSE if nothing changed, or there is bad data.
	BOOL Validate(BOOL& bAllowClose);

protected:
	// (c.haag 2007-11-07 13:22) - PLID 27994 - This utility function is used to
	// generate SQL statements for creating a new return group. This also assigns
	// a value to the procedure variable @nReturnGroupID to be used later
	void GenerateSaveNewReturnGroupSql(CString& strSaveString, long& nAuditTransactionID);

	// (c.haag 2007-11-07 13:22) - PLID 27994 - This utility function is used to
	// generate SQL statements for saving an existing return group. This also assigns
	// a value to the procedure variable @nReturnGroupID to be used later
	void GenerateUpdateExistingReturnGroupSql(CString& strSaveString, long& nAuditTransactionID);

protected:
	// (c.haag 2007-11-07 12:44) - PLID 27994 - This utility function is used to
	// generate SQL statements to add return items to data
	void GenerateSaveNewReturnItemsSql(CString& strSaveString, long& nAuditTransactionID);

	// (c.haag 2007-11-08 15:51) - PLID 27944 - This utility function is used to
	// generate SQL statements to update existing return items in data
	void GenerateUpdateExistingReturnItemsSql(CString& strSaveString, long& nAuditTransactionID);

	// (c.haag 2007-11-07 15:01) - PLID 27994 - This untility function generates an
	// SQL statement to delete all desired return items from data
	void GenerateDeleteExistingReturnItemsSql(CString& strSaveString, long& nAuditTransactionID);

protected:
	// (c.haag 2007-11-07 12:32) - PLID 27994 - This utility function is the central
	// point for saving changes to data
	void Save();

	// (j.jones 2008-09-08 10:22) - PLID 30538 - warns if cancelling a new return,
	// or an existing one you made changes to, then returns true if the user wants
	// to cancel, false if they don't want to cancel
	BOOL WarnCancelDialog();

protected:
	// (c.haag 2007-11-08 09:56) - PLID 27994 - Loads existing return group data
	void LoadReturnGroup();

	// (c.haag 2007-11-08 09:56) - PLID 27994 - Loads existing return group line item data
	void LoadReturnItems();

protected:
	// (c.haag 2007-11-08 09:57) - PLID 27994 - Loads an existing return group with items
	void Load();

public:
	// (c.haag 2007-11-08 09:56) - PLID 27944 - Defines a return group to load when opening
	// the dialog
	inline void SetLoadID(long nReturnGroupID) { m_nReturnGroupIDToLoad = nReturnGroupID; }

	//TES 6/23/2008 - PLID 26152 - Lets the dialog be initialized with an array of ProductItemIDs.  It is the caller's
	// responsibility to ensure that the given parameters are a valid combination, otherwise errors or bad data may
	// result.
	void SetLoadItems(long nLocationID, long nSupplierID, const CArray<ProductItemReturnInfo,ProductItemReturnInfo&> &arProductItems);

protected:
	// (c.haag 2008-01-07 11:56) - PLID 27996 - If a barcode is scanned, this function will warn
	// the user if the supplier dropdown selection is inconsistent with the product's supplier,
	// and resolve the inconsistency by selecting the product and clearing the list
	BOOL EnsureItemFilterCurrentForBarcodeScan(long nSupplierID);

	//TES 1/26/2009 - PLID 32824 - Synchronize the Vendor Confirmed checkbox with its linked controls.
	void EnsureConfirmedControls();

protected:

	// (j.jones 2009-02-09 15:11) - PLID 32706 - added a total item count label
	void UpdateTotalItemCountLabel();

	//(c.copits 2010-09-21) PLID 40317 - Allow duplicate UPC codes for FramesData certification
	NXDATALIST2Lib::IRowSettingsPtr GetBestUPCProductFromItemReturnList(_variant_t vBarcode);
	ADODB::_RecordsetPtr GetBestUPCProductFromData(CString strBarcode);

	// (j.jones 2009-02-10 10:32) - PLID 32827 - added OnBtnReturnTrackFedex
	// Generated message map functions
	//{{AFX_MSG(CInvEditReturnDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnOkBtn();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnCancelBtn();
	afx_msg void OnRemoveReturnItem();
	afx_msg void OnRequeryFinishedLocationReturnList(short nFlags);
	afx_msg void OnSelChosenLocationReturnList(LPDISPATCH lpRow);
	afx_msg void OnSelChosenSupplierReturnList(LPDISPATCH lpRow);
	afx_msg void OnSelChosenItemReturnList(LPDISPATCH lpRow);
	afx_msg void OnRButtonDownList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnSelChangingSupplierReturnList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingLocationReturnList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnBtnDeleteReturn();
	afx_msg void OnEditingStartingList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishingList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnBtnConfigureReturnReasons();
	afx_msg void OnBtnConfigureReturnMethods();
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEditingFinishedList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnPTSSPreview();
	afx_msg void OnRequeryFinishedItemReturnList(short nFlags);
	afx_msg void OnRequeryFinishedSupplierReturnList(short nFlags);
	afx_msg void OnSelChosenReturnMethodList(LPDISPATCH lpRow);
	afx_msg void OnRequeryFinishedReturnReasonList(short nFlags);
	afx_msg void OnSelChangingReturnReasonList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingReturnForList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnVendorConfirmed();
	afx_msg void OnBtnReturnTrackFedex();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVEDITRETURNDLG_H__1F57717C_F1B7_437E_991B_248FC82A6AE4__INCLUDED_)

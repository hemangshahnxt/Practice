#pragma once

// CInvReconciliationDlg dialog

// (j.jones 2009-01-07 11:30) - PLID 26141 - created

enum InvRecSaveFlags {	//used by the Save() function

	irsfInvalid = -1,

	irsfSimpleSave = 0,
	irsfCompleted = 1,
	irsfCancelled = 2,
};

// (j.jones 2009-01-15 11:18) - PLID 32684 - ProductNeedingAdjustments will track
// the info. for every product that needs an adjustment, and the quantities involved
struct ProductNeedingAdjustments {

	long nProductID;
	CString strProductName;
	double dblCalculated;
	double dblUserCounted;
	BOOL bAdjust;
	CArray<long, long> aryProductItemIDsToRemove;
	CString strSqlBatch;
};

struct ReconciledProductItem {

	long nProductItemID;
	long nInternalID;
	CString strSerialNum;
	CString strSerializedText;
	BOOL bCounted;
	CString strNotes;
};

struct ReconciledProduct {

	long nProductID;
	long nInternalID;
	CString strName;
	CString strBarcode;
	double dblCalculatedAmt;
	_variant_t varUserCount;
	BOOL bAdjust;
	CString strNotes;

	CArray<ReconciledProductItem*, ReconciledProductItem*> aryProductItems;
	
	//used for filtering purposes
	CArray<long long> arySupplierIDs;
	long nCategoryID;
};

class CInvReconciliationDlg : public CNxDialog
{

public:
	CInvReconciliationDlg(CWnd* pParent);   // standard constructor
	~CInvReconciliationDlg();

	long m_nID;	//the InvReconciliationsT.ID, -1 if a new reconciliation
	long m_nLocationID;
	CString m_strLocationName;	// (j.jones 2009-07-09 13:23) - PLID 34834 - required for auditing

	BOOL m_bCanEdit;	// (j.jones 2009-07-09 09:34) - PLID 34826 - added m_bCanEdit which is true if the user has permissions to adjust

// Dialog Data
	enum { IDD = IDD_INV_RECONCILIATION_DLG };
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnCancelReconciliation;
	CNxIconButton m_btnSaveCompleted;
	CNxIconButton m_btnSaveUncompleted;
	CNxStatic m_nxstaticDateLabel;
	CNxEdit	m_nxeditNotes;
	CNxStatic m_nxstaticLocationLabel;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	NXDATALIST2Lib::_DNxDataListPtr m_ProductList;
	NXDATALIST2Lib::_DNxDataListPtr m_SupplierCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_CategoryCombo;

	//this array tracks everything we loaded, so we can filter without losing what data we changed
	CArray<ReconciledProduct*, ReconciledProduct*> m_aryProducts;

	//RefilterList will apply the contents of m_aryProducts to the list, taking the supplier
	//and category filter into account
	void RefilterList();

	void InitNew();
	void Load();
	BOOL Save(InvRecSaveFlags irsfSaveFlag);
	BOOL Validate(InvRecSaveFlags irsfSaveFlag);

	BOOL m_bIsClosed;	//tracks whether the reconciliation is closed (completed or cancelled)

	COleDateTime m_dtStartDate;	//tracks the time the count started, which is when they first opened the dialog	

	BOOL m_bIsBarcodeScanning; //tracks if we are currently barcode scanning

	//this function will take in a product pointer that has product items under it,
	//and return the count of how many product items under it are checked
	long CountCheckedProductItemsUnderParent(ReconciledProduct *pProduct);

	// (j.jones 2009-01-15 11:00) - PLID 32684 - PrepareAdjustments should be called only when completing
	// a reconciliation, it will show the user which products would be adjusted and let them uncheck the
	// "adjust" box for products if they wish. If they don't have permission to make adjustments, or
	// they cancel this screen, this function will return FALSE and abort saving.
	// If adjustments need to be made, the arypProductsNeedingAdjustments array will be filled.
	BOOL PrepareAdjustments(CArray<ProductNeedingAdjustments*, ProductNeedingAdjustments*> &arypProductsNeedingAdjustments);

	// (j.jones 2009-01-15 11:22) - PLID 32684 - clear the adjustment array
	void ClearAdjustmentArray(CArray<ProductNeedingAdjustments*, ProductNeedingAdjustments*> &arypProductsNeedingAdjustments);

	// (j.jones 2009-01-15 14:26) - PLID 32684 - added function to save adjustments
	void CreateAdjustment(CString &strSqlBatch, long &nAuditTransactionID, long nProductID, CString strInvRecProductID, CArray<ProductNeedingAdjustments*, ProductNeedingAdjustments*> &arypProductsNeedingAdjustments);

	//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
	BOOL GetBestUPCProduct(ReconciledProduct *pProduct, CString strBarcode);

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnCancel();
	afx_msg void OnBtnCancelReconciliation();
	afx_msg void OnBtnSaveCompleted();
	afx_msg void OnBtnSaveUncompleted();
	void OnEditingStartingProductList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void OnEditingFinishingProductList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedProductList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);	
	void OnRButtonDownProductList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	virtual LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	void OnSelChosenInvrecSupplierCombo(LPDISPATCH lpRow);
	void OnSelChosenInvrecCategoryCombo(LPDISPATCH lpRow);
};

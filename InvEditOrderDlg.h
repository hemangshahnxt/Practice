#if !defined(AFX_INVEDITORDERDLG_H__0F52E14C_40F6_4995_BDBE_A76712D2B0FC__INCLUDED_)
#define AFX_INVEDITORDERDLG_H__0F52E14C_40F6_4995_BDBE_A76712D2B0FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Client.h"

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NXTIMELib;
// InvEditOrderDlg.h : header file
//
class CInvOrderDlg;
/////////////////////////////////////////////////////////////////////////////
// CInvEditOrderDlg dialog

// (j.jones 2008-02-07 10:50) - PLID 28851 - added rules for AddItem
enum AddItemRule {
	airNoRule = 0,		//will tell AddItem() to add normally
	airPurchasedInv,	//will force AddItem() to add as purchased inventory
	airConsignment,		//will force AddItem() to add as consignment
};

//TES 7/22/2008 - PLID 30802 - Used to pass in allocations to initialize the order with
struct AllocationDetail {
	long nAllocationDetailID;
	long nProductID;
	double dQty;
};

class CInvEditOrderDlg : public CNxDialog
{
// Construction
public:
	CInvEditOrderDlg(CInvOrderDlg *pParent,BOOL bFrameOrder = FALSE);
	~CInvEditOrderDlg();	// (j.jones 2007-12-19 14:28) - PLID 28393
	
	// (j.jones 2009-02-16 10:16) - PLID 33085 - changed Refresh() to LoadOrder()
	BOOL LoadOrder();

	// (c.haag 2007-12-05 14:44) - PLID 28286 - If the latter is FALSE, then
	// we treat this as a brand new order and expect that nothing has arrived.
	// If it's TRUE, then we use this dialog to let the user enter inventory
	// items from a previous order that wasn't entered into the system.
	// (j.jones 2008-03-18 14:44) - PLID 29309 - added appt. and location IDs as optional parameters
	//TES 7/22/2008 - PLID 30802 - Added optional supplier ID.
	void DoFakeModal(long OrderID, BOOL bSaveAllAsReceived, long nApptID = -1, long nLocationID = -1, long nSupplierID = -1);

	//TES 6/13/2008 - PLID 28078 - We now give the option to do "real" modal.
	//TES 7/22/2008 - PLID 30802 - Added optional supplier ID.
	int DoModal(long OrderID, BOOL bSaveAllAsReceived, long nApptID = -1, long nLocationID = -1, long nSupplierID = -1);

	//TES 7/22/2008 - PLID 30802 - Call before DoModal(), and the order will load the products from the given allocation
	// details (assuming you pass in -1 for the OrderID in DoModal).
	void SetAllocationDetails(const CArray<AllocationDetail,AllocationDetail&> &arAllocationDetails);

	// (j.jones 2008-02-07 10:20) - PLID 28851 - there are two types of auto-ordering,
	// purchased inventory and consignment, so require a boolean to be sent in to determine
	// which "auto order" to call
	void AutoOrder(long supplier, BOOL bConsignmentOrder);

	BOOL ValidateUU_UOStatus(long nOrderDetailID);
	

	// (j.jones 2008-06-19 10:06) - PLID 10394 - added m_btnApplyDiscounts
	// (j.jones 2009-01-23 09:57) - PLID 32822 - added m_btnEditOrderMethods
	// (j.jones 2009-02-10 09:57) - PLID 32827 - added m_btnTrackFedex
	// (j.jones 2009-02-10 16:37) - PLID 32871 - added color controls
// Dialog Data
	//{{AFX_DATA(CInvEditOrderDlg)
	enum { IDD = IDD_INVEDITORDER };
	CNxStatic	m_nxsLinkedAllocationsLabel;
	CNxIconButton	m_btnApplyDiscounts;
	NxButton	m_btnCCOnFile;
	NxButton	m_btnShipTo;
	CNxIconButton	m_edtShipMethodBtn;
	CNxIconButton	m_edtCCTypeList;
	CNxIconButton	m_btnAutoConsign;
	CNxIconButton	m_btnAutoPurchInv;
	NxButton	m_checkTaxShipping;
	CNxIconButton	m_defaultTaxBtn;
	CNxIconButton	m_cancelBtn;
	CNxIconButton	m_okBtn;
	CNxIconButton	m_printBtn;
	CNxIconButton	m_deleteBtn;
	CNxIconButton	m_btnFrameToProduct; 
	CNxEdit	m_nxeditDescription;
	CNxEdit	m_nxeditPurchaseOrder;
	CNxEdit	m_nxeditOrderContactName;
	CNxEdit	m_nxeditOrderCcName;
	CNxEdit	m_nxeditOrderCcNumber;
	CNxEdit	m_nxeditOrderCcExpDate;
	CNxEdit	m_nxeditTrackingNumber;
	CNxEdit	m_nxeditTax;
	CNxEdit	m_nxeditExtraCost;
	CNxEdit	m_nxeditNotes;
	CNxStatic	m_nxstaticLinkedApptLabel;
	CNxStatic	m_nxstaticSubtotal;
	CNxStatic	m_nxstaticTaxtotal;
	CNxStatic	m_nxstaticExtracost;
	CNxStatic	m_nxstaticTotal;
	// (j.gruber 2009-03-03 15:17) - PLID 30199 - added total qty
	CNxStatic	m_nxstaticQty;
	CNxIconButton	m_btnEditOrderMethods;
	NxButton m_btnVendorConfirmed;
	CNxEdit m_nxeditVendorConfNum;
	CNxIconButton m_btnTrackFedex;
	CNxColor m_bkg1;
	CNxColor m_bkg2;
	CNxColor m_bkg3;
	CNxColor m_bkg4;
	//}}AFX_DATA

	//r.wilson plid 47393
	BOOLEAN m_bFrameOrderReception;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvEditOrderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL


// Implementation
protected:
	bool							m_bTotalUpdateable;
	CInvOrderDlg						*m_pParent;
	long							m_id,
									m_rightClicked;
	CString							m_strDefaultWhereClause;

	NXDATALISTLib::_DNxDataListPtr 	m_location, 
									m_supplier, 
									m_item, 
									m_list;

	int m_iSupplierSel;

	CTableChecker					m_taxChecker,
									m_supplierChecker,
									m_locationChecker;

	NXTIMELib::_DNxTimePtr						m_nxtDate, m_nxtArrivalDate;
	NXTIMELib::_DNxTimePtr						m_nxtDateConfirmed;				// (d.thompson 2009-01-23) - PLID 32823

	//TES 6/13/2008 - PLID 28078 - Were we called modally?
	bool m_bIsModal;
	//TES 6/13/2008 - PLID 28078 - Initializes the screen, called from either DoFakeModal() or OnInitDialog().
	void Initialize();

	// (j.jones 2008-03-03 17:52) - PLID 29181 - added locationID as a parameter
	void SaveOrder(long nLocationID);
	// (j.jones 2008-03-20 12:16) - PLID 29311 - now returns TRUE if we just completely received the order
	//TES 7/23/2008 - PLID 30802 - This now outputs a list of allocation IDs that got updated, in order to update the user
	// (outside of the transaction).
	// (j.jones 2008-09-26 10:49) - PLID 30636 - takes in nAddToAllocationID, and if not -1, will
	// add all products into the allocation if they were not previously received in a prior save
	// I also added nAuditTransactionID, which isn't used by anything but the allocation changes right now.
	BOOL ApplyItemsToOrder(long nLocationID, OUT CArray<long,long> &arUpdatedAllocationIDs, long nAddToAllocationID, CString strAllocPatientName, long nAllocPatientID, long nAuditTransactionID);
	double GetTax();
	void SetTax(double tax);
	void UpdateTotal(int nRow = -1);
	// (j.jones 2008-02-20 13:17) - PLID 28983 - renamed this function to more accurately explain what it does,
	// now that we have the ToOrder column that is updated within this function
	void UpdateItemListColorsAndToOrderAmt();

	// (j.jones 2008-02-07 16:47) - PLID 28851 - added bIsConsignment
	// (j.jones 2008-02-08 14:12) - PLID 28851 - scratch that, we have now removed this functionality
	/*
	void AddToItemOnOrder(long productID, long quantity, BOOL bIsConsignment);
	void AddToItemReceived(long productID, long quantity, BOOL bIsConsignment);
	void RemoveUnsavedItemFromOrder(long row);
	void AddUnsavedItemsToItemsOnOrder();
	*/

	// (c.haag 2008-01-10 17:44) - PLID 28592 - Added nDefaultStatus so that the General/Consignment status in the product item dialog
	// would be consistent with that of the product's default
	//TES 6/18/2008 - PLID 29578 - Changed the first parameter from OrderID to nOrderDetailID
	BOOL ReceiveProductItems(long nOrderDetailID, long ProductID, long Quantity, long nDefaultStatus);
	BOOL ValidateSaveAndClose();
	void DoDeleteOrder();
	bool IsOrderReceived();
	// (c.haag 2007-12-05 15:35) - PLID 28286 - This adds a row and returns the index of the newly added row.
	// (j.jones 2008-02-07 10:49) - PLID 28851 - added an override to force ordering as purchased inventory or as consignment
	// (a.walling 2008-03-20 09:21) - PLID 29333 - added a flag to control whether item is added as a new row if it exists
	// or quantity is upped by 1. Default is TRUE, meaning default behaviour does not change.
	//TES 7/22/2008 - PLID 30802 - Added optional default quantity, and Allocation Detail to link to.
	long AddItem(long nSelRow, AddItemRule airRule, BOOL bAddIfExists = TRUE, double dQty = -1.0, long nInvAllocationDetailID = -1);
	// (c.haag 2007-12-05 15:58) - PLID 28286 - This function is called when a new item is created
	void OnNewItemAdded(long nNewRow, long nItemComboRow);
	// (c.haag 2007-12-06 08:33) - PLID 28286 - Display the product item dialog for serialized items
	void ShowSerializedItemWindow(long nRow);
	
	std::vector<long> m_aryOrderDetailsToDelete;

	// (c.haag 2007-12-06 09:34) - PLID 28286 - Safely removes a list row
	void RemoveListRow(long nRow);

	// (c.haag 2008-02-15 11:23) - PLID 28286 - Ensures a string element is in a string array
	void EnsureInArray(CStringArray& astr, const CString& str);

	//DRT 3/8/2007 - PLID 24823 - This is enabled when we start scanning a barcode, and disabled
	//	once complete, so that only one scan happens at a time.
	BOOL m_bIsScanningBarcode;

	// (c.haag 2007-12-05 14:35) - PLID 28286 - TRUE if the user is entering supplies
	// that arrived from an order than was never entered into Practice beforehand
	BOOL m_bSaveAllAsReceived;

	// (c.haag 2007-12-06 09:04) - PLID 28286 - This is a non-negative value if the product item
	// dialog is visible
	long m_nVisibleProductItemDlgRow;

	// (j.gruber 2008-02-27 14:43) - PLID 28955 - added fields to the order dlg
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationSoldTo;
	NXDATALIST2Lib::_DNxDataListPtr m_pCCTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pShipMethodList;

	NXDATALIST2Lib::_DNxDataListPtr m_pLinkedAllocations;

	// (j.jones 2009-01-23 09:57) - PLID 32822 - added order method
	NXDATALIST2Lib::_DNxDataListPtr m_pOrderMethodCombo;

	CString m_strCreditCardNumber;

	// (j.jones 2008-03-05 11:18) - PLID 28981 - split ReorderAmount into General and Consignment functions
	long ReorderAmountPurchased(double dblMinAmountNeeded, long nReorderQuantity);
	long ReorderAmountConsignment(double dblMinAmountNeeded);

	// (j.jones 2008-03-18 14:46) - PLID 29309 - added ability to link an order with an appt.
	long m_nApptID;
	long m_nDefLocationID;
	long m_nOrigLinkedApptID;

	//TES 7/22/2008 - PLID 30802 - Added ability to specify a default supplier
	long m_nDefSupplierID;

	void DisplayLinkedApptData();

	//support the appt. hyperlink
	CRect m_rcApptLinkLabel;
	CString m_strApptLinkText;
	void DrawApptLinkLabel(CDC *pdc);

	// (j.jones 2008-03-19 09:09) - PLID 29309 - handles when the appt. hyperlink is clicked
	void OnClickApptLink();

	//cached the adv. inventory flag
	BOOL m_bHasAdvInventory;

	// (j.jones 2008-03-19 11:40) - PLID 29316 - cache the appt. text for auditing
	CString m_strOldApptLinkText;

	// (j.jones 2008-02-07 10:02) - PLID 28851 - split the Auto-order function into two,
	// one for purchased inventory, one for consignment

	// (j.jones 2008-06-19 10:06) - PLID 10394 - added abilities to apply a percent off
	// or discount amount to all items
	void PromptApplyPercentOffToAll();
	void PromptApplyDiscountAmountToAll();

	//TES 7/22/2008 - PLID 30802 - A list of allocation details that we are being asked to pull the products from.
	CArray<AllocationDetail,AllocationDetail&> m_arAllocationDetails;

	//TES 7/22/2008 - PLID 30802 - Pull the data in m_arAllocationDetails, and add the corresponding products to the order.
	void LoadProductsFromAllocationDetails();

	//TES 7/22/2008 - PLID 30802 - An array of lists of allocation detail IDs, each order detail stores an index in this
	// array in the datalist, so it knows which allocation detail IDs it's tied to (since it may have more than one).
	CArray<CArray<long,long>*,CArray<long,long>*> m_arAllocationDetailIDs;

	//TES 7/22/2008 - PLID 30802 - Internal, appends the given detail to the list with the given "ID"
	void AddAllocationDetailToList(long nAllocationDetailID, long nAllocationDetailListID);

	//TES 7/22/2008 - PLID 30802 - Internal, creates a new list of detail IDs, adds the passed-in detail to it, then returns its
	// "ID" (not a data ID, just used in memory during this session).
	long GetNewAllocationDetailList(long nInitialAllocationDetailID);

	//TES 7/22/2008 - PLID 30802 - De-allocates and clears out our list of allocation detail IDs.
	void ClearAllocationDetailIDs();

	//TES 7/22/2008 - PLID 30802 - Added ability to hide the linked appointment label, or the list of linked allocations,
	// as only one should be used for any given order.
	bool m_bShowApptLinkLabel;
	void HideLinkedAllocations();
	void ShowLinkedAllocations();
	//TES 7/22/2008 - PLID 30802 - Requeries the list, then shows/hides it based on whether it has any records.
	void RequeryLinkedAllocations();

	//TES 8/13/2008 - PLID 30165 - Several places call the event handler directly, we need a separate function, with an
	// extra parameter.  bLoading indicates whether we're loading an existing order, and therefore shouldn't treat the 
	// selected supplier as "new"
	void HandleNewSupplier(long nSupplierRow, bool bLoading = false);

	//TES 8/13/2008 - PLID 30165 - A function to remember the CC On File value for the currently selected supplier, called
	// a couple different locations.
	void RememberCCOnFile();

	// (j.jones 2008-09-26 09:52) - PLID 30636 - added function to prompt to add to an existing allocation
	// and populates nFoundAllocationID and strAllocPatientName with the results, if any
	// (a.walling 2010-01-21 15:57) - PLID 37024
	void FindApptAllocationID(long nAppointmentID, OUT long &nFoundAllocationID, OUT CString &strAllocPatientName, OUT long& nAllocPatientID);

	// (j.jones 2008-09-26 11:07) - PLID 30636 - added function to insert an order detail into an allocation
	// bIsSerialized should be TRUE if the product tracks serial numbers or exp. dates, and will force a quantity of 1
	// nRecordsToCreate should be 1 unless bIsSerialized = TRUE, and determines how many times the product should be added
	// strProductName, strPatientName, and nQuantity are for auditing, so we don't need extra recordsets
	// (a.walling 2010-01-21 16:01) - PLID 37024
	void InsertOrderDetailIntoAllocation(long nOrderDetailID, long nAllocationID, BOOL bIsSerialized, long nRecordsToCreate,
										CString strProductName, CString strPatientName, long nPatientID, long nQuantity, long nAuditTransactionID);

	// (d.thompson 2009-01-23) - PLID 32823
	void EnsureConfirmedControls();

	// (j.jones 2009-02-10 12:13) - PLID 32870 - added m_straryIgnoredBarcodes, which will track any barcodes
	// we received in this screen and did not process (or warn about)
	CStringArray m_straryIgnoredBarcodes;

	// (j.jones 2009-02-10 16:42) - PLID 32871 - ConfigureDisplay will change the background colors if auto-receiving
	void ConfigureDisplay();

	//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
	long GetBestUPCProduct(_variant_t varCode);

	// (b.spivey, September 21, 2011) - PLID 45265 - Need a way to check in multiple places for the barcode message handler. 
	BOOL FramesDataExists(CString varCode);

	// (j.jones 2008-06-19 10:06) - PLID 10394 - added OnBtnApplyDiscounts
	// (j.jones 2008-07-03 16:41) - PLID 30624 - converted OnSelChangedLocation to OnSelChosenLocation
	// (j.jones 2009-02-10 10:32) - PLID 32827 - added OnBtnOrderTrackFedex
	// Generated message map functions
	//{{AFX_MSG(CInvEditOrderDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDelete();
	afx_msg void OnPrintPrev();
	afx_msg void OnDefault();
	afx_msg void OnKillfocusTax();
	afx_msg void OnKillfocusExtraCost();
	afx_msg void OnRButtonUpList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingFinishedList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnRequeryFinishedList(short nFlags);
	afx_msg void OnCancelBtn();
	virtual void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnOkBtn();
	afx_msg void OnAddItem(long nSelRow);
	afx_msg void OnRequeryFinishedItem(short nFlags);
	afx_msg void OnTrySetSelFinishedLocation(long nRowEnum, long nFlags);
	afx_msg void OnKillFocusDateDue();
	afx_msg void OnKillFocusDate();
	afx_msg void OnSelChosenSupplier(long nRow);
	afx_msg void OnCheckTaxShipping();
	afx_msg LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDblClickCellList(long nRowIndex, short nColIndex);
	afx_msg LRESULT OnPostEditProductItemDlg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDownList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditingStartingList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnAutoPurchInv();
	afx_msg void OnAutoConsign();
	afx_msg void OnEditShipMethod();
	afx_msg void OnEditCcType();
	afx_msg void OnEnableShipTo();
	afx_msg void OnUpdateOrderCcExpDate();
	afx_msg void OnKillfocusOrderCcExpDate();
	afx_msg void OnSelChosenSoldTo(LPDISPATCH lpRow);
	afx_msg void OnOrderCcOnFile();
	afx_msg void OnKillfocusOrderCcNumber();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSelChangingLocation(long FAR* nNewSel);
	afx_msg void OnSelChangingSoldTo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnBtnApplyDiscounts();
	afx_msg void OnSelChosenLocation(long nRow);
	afx_msg void OnLeftClickLinkedAllocations(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnEditOrderMethods();
	afx_msg void OnVendorConfirmedChkClicked();
	afx_msg void OnBtnOrderTrackFedex();
	afx_msg void OnBnClickedFrameToProduct();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVEDITORDERDLG_H__0F52E14C_40F6_4995_BDBE_A76712D2B0FC__INCLUDED_)

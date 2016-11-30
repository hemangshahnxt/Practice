#include "client.h"
#if !defined(AFX_INVEDITDLG_H__3331FE60_A8BF_11D2_8E52_00AA0064A698__INCLUDED_)
#define AFX_INVEDITDLG_H__3331FE60_A8BF_11D2_8E52_00AA0064A698__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InvEditDlg.h : header file
//

// (c.haag 2008-02-08 12:07) - PLID 28852 - Added events for consignment controls

/////////////////////////////////////////////////////////////////////////////
// CInvEditDlg dialog

class CInvEditDlg : public CNxDialog
{

// Construction
public:

	enum ETrackType {
		ENotTrackable,
		ETrackOrders,
		ETrackQuantity,
	}m_eTrackStatus;
	void ResetTrackingRadios();
	// (c.haag 2008-02-07 17:10) - PLID 28852 - This will show or hide all of the consignment total
	// edit boxes and labels
	void ShowConsignmentTotalBoxes();
	//TES 6/16/2008 - PLID 27973 - Added a parameter for whether it should show the "Reassign Item Locations" button.
	void ShowItemsBtn(BOOL bCanReassignItems = FALSE);
	void ShowSerializedPerUOCheck();
	void RefreshSuppliers();
	CInvEditDlg(CWnd* pParent);   // standard constructor
	virtual ~CInvEditDlg();
	BOOL LoadSingleItem(long nItemID);
	virtual void Refresh();

	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	CBrush m_brBlue, m_brDark;
	bool m_bPopup;	//Is this window popped up? (generally from billing dlg)

	// (j.jones 2008-06-10 11:27) - PLID 27665 - added m_checkFacilityFee
// Dialog Data
	//{{AFX_DATA(CInvEditDlg)
	enum { IDD = IDD_INVEDITOR };
	NxButton	m_nxbSerialAsLot;
	NxButton	m_checkFacilityFee;
	CNxIconButton	m_btnAddNewSupplier;
	CNxIconButton	m_btnLinkProducts;
	NxButton	m_btnDefaultConsignment;
	CNxIconButton	m_btnNextLocation;
	CNxIconButton	m_btnPrevLocation;
	CNxIconButton	m_btnTransferProductItems;
	NxButton	m_checkSerializedPerUO;
	CNxIconButton	m_btnRemoveCategory;
	CNxIconButton	m_btnSelectCategory;
	CNxIconButton	m_btnEditMultipleRevCodes;
	NxButton	m_checkMultipleRevCodes;
	NxButton	m_checkSingleRevCode;
	CNxIconButton	m_btnAdvRevCodeSetup;
	CNxIconButton	m_btnPendingCases;
	CNxIconButton	m_btnTransfer;
	NxButton	m_checkEquipment;
	CNxIconButton	m_btnEquipmentMaintenance;
	CNxIconButton	m_btnHistory;
	CNxIconButton	m_btnShowItems;
	NxButton	m_checkHasSerial;
	NxButton	m_checkHasExpDate;
	NxButton	m_checkUseUU;
	CNxIconButton	m_btnDeleteSupplier;
	CNxIconButton	m_btnAddSupplier;
	CNxIconButton	m_btnMakeDefault;
	CNxIconButton	m_inactiveItems;
	NxButton	m_radioNotTrackable;
	NxButton	m_radioTrackOrders;
	NxButton	m_radioTrackQuantity;
	CNxIconButton	m_markItemInactiveButton;
	NxButton	m_Taxable2;
	CNxIconButton	m_prevItem;
	CNxIconButton	m_nextItem;
	CNxIconButton	m_deleteItemBtn;
	CNxIconButton	m_newItemBtn;
	CNxIconButton	m_adjustBtn;
	NxButton	m_taxable;
	NxButton	m_billable;
	CNxEdit	m_nxeditName;
	CNxEdit	m_nxeditUnitdesc;
	CNxEdit	m_nxeditLastCost;
	CNxEdit	m_nxeditInvShopFee;
	CNxEdit	m_nxeditPrice;
	CNxEdit	m_nxeditBarcode;
	CNxEdit	m_nxeditNotes;
	CNxEdit	m_nxeditConversionEdit;
	CNxEdit	m_nxeditUuEdit;
	CNxEdit	m_nxeditUoEdit;
	CNxEdit	m_nxeditActual;
	CNxEdit	m_nxeditActualUo;
	CNxEdit	m_nxeditAvailInv;
	CNxEdit	m_nxeditAvailUo;
	CNxEdit	m_nxeditOrder;
	CNxEdit	m_nxeditOrderUo;
	CNxEdit	m_nxeditReorderpoint;
	CNxEdit	m_nxeditReorderpointUo;
	CNxEdit	m_nxeditReorderquantity;
	CNxEdit	m_nxeditReorderquantityUo;
	CNxEdit	m_nxeditCategorybox;
	CNxEdit	m_nxeditInsCode;
	CNxEdit	m_nxeditCatalogEdit;
	CNxEdit	m_nxeditActualConsignment;
	CNxEdit	m_nxeditActualUoConsignment;
	CNxEdit	m_nxeditAvailConsignment;
	CNxEdit	m_nxeditAvailUoConsignment;
	CNxEdit	m_nxeditOrderConsignment;
	CNxEdit	m_nxeditOrderUoConsignment;
	CNxEdit	m_nxeditReorderpointConsignment;
	CNxEdit	m_nxeditReorderpointUoConsignment;
	CNxEdit	m_nxeditReorderquantityConsignment;
	CNxEdit	m_nxeditReorderquantityUoConsignment;
	CNxStatic	m_nxstaticCatFilterLabel;
	CNxStatic	m_nxstaticProductFilterLabel;
	CNxStatic	m_nxstaticLastCostText;
	CNxStatic	m_nxstaticInvShopFeeLabel;
	CNxStatic	m_nxstaticPriceText;
	CNxStatic	m_nxstaticBarcodeText;
	CNxStatic	m_nxstaticUb92CptLabel;
	CNxStatic	m_nxstaticConversionText;
	CNxStatic	m_nxstaticConversionText2;
	CNxStatic	m_nxstaticUuText;
	CNxStatic	m_nxstaticUoText;
	CNxStatic	m_nxstaticLocCaption;
	CNxStatic	m_nxstaticUuLabel;
	CNxStatic	m_nxstaticUoLabel;
	CNxStatic	m_nxstaticActualText;
	CNxStatic	m_nxstaticAvailText;
	CNxStatic	m_nxstaticOrderText;
	CNxStatic	m_nxstaticReorderpointText;
	CNxStatic	m_nxstaticReorderquantityText;
	CNxStatic	m_nxstaticRespText;
	CNxStatic	m_nxstaticSupplierLabel;
	CNxStatic	m_nxstaticCatalogLabel;
	CNxStatic	m_nxstaticUuLabelConsignment;
	CNxStatic	m_nxstaticUoLabelConsignment;
	CNxStatic	m_nxstaticLabelPurchasedInv;
	CNxStatic	m_nxstaticLabelConsignment;
	CNxIconButton	m_btnViewFramesData; // (z.manning 2010-06-21 10:23) - PLID 39257
	//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
	NxButton	m_CheckWarranty;
	CString		m_strWarranty;
	// (s.dhole 2011-04-01 12:05) - PLID 42157
	NxButton	m_CheckIsFrameData;
	NxButton	m_nxbIsContactLens;
	//}}AFX_DATA
	// (s.dhole 2012-03-14 14:14) - PLID  48888
	CNxIconButton	m_btnViewContactLensData;
	CNxIconButton m_btnPrintBarcode;
	// (j.dinatale 2012-06-13 18:00) - PLID 32795
	CNxIconButton m_btnNDCInfo;
	CNxIconButton m_btnManageOwners; // (j.fouts 2012-08-10 09:09) - PLID 50934 - Added a button to manage owners of an item
	// (j.jones 2013-07-16 09:33) - PLID 57566 - added NOC label
	CNxLabel	m_nxlNOC;
	// (j.jones 2016-04-07 10:22) - NX-100075 - added ability to remember the last charge provider
	NxButton	m_checkRememberChargeProvider;

	// (a.wilson 2014-5-5) PLID 61831 - remove the old ServiceT.SendOrderingPhy field code.

	// (j.gruber 2012-10-26 13:10) - PLID 53239
	COleCurrency m_cyShopFee;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInvEditDlg)
	protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnBarcodeScan(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnReposition(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void FormatDlgCurrency (int nID);
	void Save (int nID);
	long GetItem();
	void LoadUsersForLocation();
	void LoadUserInfo();
	void UpdateArrows(); //Disable the left and right arrows as appropriate
	void SecureControls(); // Disable controls if access is denied
	void CheckEmptyInventory();//Disables all fields if inventory is empty
	void RefreshItemsAndCategoriesLists();
	void AddAllCategoriesEntry();
	void AddNoCategoryEntry();
	void AddDefaultFilters();
	void SelChosenCategoryList(long nRow);

	// (j.dinatale 2012-06-15 09:56) - PLID 51000 - default claim note on inventory items
	void EnterClaimNote(long nProductID, CString strProductName);

	void ShowSerializedItems(long nLocationID);
	void EnableAll(BOOL isEnabled);//Enables or disables editable fields. Used when no inventory items are active

	//this will be called by all three radio buttons
	void OnTrackable();

	void UpdateLocationArrows();
	
	NXDATALISTLib::_DNxDataListPtr m_item, 
					m_category_list,
					m_resp,
					m_UB92_Category,
					m_Supplier_List,
					m_Supplier_Combo,
					m_pLocations,
					m_DefaultProviderCombo;

	// (j.jones 2013-07-16 08:46) - PLID 57566 - supported NOC type
	NXDATALIST2Lib::_DNxDataListPtr m_NOCCombo;

	bool			m_refreshing,
					m_bRequery;

	CTableChecker	m_supplierChecker,m_CPTCategoryChecker,m_UB92CatChecker,m_userChecker,m_providerChecker,m_LocationChecker;
	// (z.manning 2010-07-16 12:55) - PLID 39347 - Added an product table checker
	CTableChecker	m_tcProductChecker;

	long			m_nLocID; //The currently selected location (_not_ necessarily the logged in location).

	long			m_nNextItemID;	//(a.wilson 2011-9-22) PLID 33502

	void HandleChangedConversionEdit();

	//(c.copits 2010-11-02) PLID 38598 - Warranty tracking system
	void UpdateWarrantyControls();

	//r.wilson 3/7/2012 PLID 48351
	void PrintFrameLabels();

	// (j.jones 2013-07-16 10:49) - PLID 57566 - added NOC label
	void PopupNOCDescription();

	// (j.jones 2015-03-03 09:05) - PLID 64964 - reloads the category list and re-selects the current product
	void ReloadCategoryList();

	// (j.jones 2008-06-10 11:31) - PLID 27665 - added OnCheckFacilityFee
	// Generated message map functions
	//{{AFX_MSG(CInvEditDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdjust();
	afx_msg void OnBillable();
	afx_msg void OnDeleteItem();
	afx_msg void OnNewItem();
	
	afx_msg void OnTaxable();
	afx_msg void OnSelChosenRespList(long nRow);
	afx_msg void OnPrevItem();
	afx_msg void OnNextItem();
	afx_msg void OnTaxable2();
	afx_msg void OnSelChosenCategoryList(long nRow);
	afx_msg void OnMarkItemInactive();
	afx_msg void OnInactiveItems();
	afx_msg void OnRadioNotTrackable();
	afx_msg void OnRadioTrackOrders();
	afx_msg void OnRadioTrackQuantity();
	afx_msg void OnSelChosenUb92Categories(long nRow);
	afx_msg void OnAddSupplier();
	afx_msg void OnMakeDefault();
	afx_msg void OnEditingFinishedSupplierList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedSupplierList(short nFlags);
	afx_msg void OnRButtonDownSupplierList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnDeleteSupplier();
	afx_msg void OnCheckUseUu();
	afx_msg void OnCheckSerialNum();
	afx_msg void OnCheckExpDate();
	afx_msg void OnBtnShowItems();
	afx_msg void OnSelChosenSupplierCombo(long nRow);
	afx_msg void OnSelChosenLocationList(long nRow);
	afx_msg void OnHistory();
	afx_msg void OnSelChosenItem(long nRow);
	afx_msg void OnRequeryFinishedItem(short nFlags);
	afx_msg void OnKillfocusReorderpointUo();
	afx_msg void OnKillfocusReorderquantityUo();
	afx_msg void OnBtnEquipmentMaintenance();
	afx_msg void OnEquipment();
	afx_msg void OnKillfocusReorderquantity();
	afx_msg void OnSelChosenDefaultInvProvider(long nRow);
	afx_msg void OnTransfer();
	afx_msg void OnPendingCh();
	afx_msg void OnAdvRevcodeInvSetup();
	afx_msg void OnCheckSingleRevCode();
	afx_msg void OnCheckMultipleRevCodes();
	afx_msg void OnBtnEditMultipleRevCodes();
	afx_msg void OnBtnSelectCategory();
	afx_msg void OnBtnRemoveCategory();
	afx_msg void OnCheckSerializedPerUo();
	afx_msg void OnBtnAddNewSupplier();
	afx_msg void OnBtnTransferProductItems();
	afx_msg void OnSelChangingItem(long FAR* nNewSel);
	afx_msg void OnSelChangingRespList(long FAR* nNewSel);
	afx_msg void OnPrevLocation();
	afx_msg void OnNextLocation();
	afx_msg void OnDefaultConsignment();
	afx_msg void OnBtnLinkProducts();
	afx_msg void OnKillfocusReorderpointUoConsignment();
	afx_msg void OnCheckFacilityFee();
	afx_msg void OnSerialNumberAsLotNumber();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedViewFramesData(); // (z.manning 2010-06-21 10:23) - PLID 39257
	//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
	afx_msg void OnWarranty();
	afx_msg void OnEnKillfocusWarranty();
	afx_msg void OnBnClickedCheckIsFrameData();
	afx_msg void OnIsContactLens();
	afx_msg void OnBnClickedButtonPrintBc();
	afx_msg void OnBnClickedViewContactInvData();
	afx_msg void OnBnClickedInvNdcDefBtn();
	// (j.fouts 2012-08-10 09:09) - PLID 50934 - Added a button to manage owners of an item
	afx_msg void OnBnClickedManageOwners();
	afx_msg void OnBnClickedEditShopFees(); // (j.gruber 2012-10-29 16:02) - PLID 53239
	// (j.jones 2013-07-16 08:46) - PLID 57566 - supported NOC type
	void OnSelChangingNocProductCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void OnSelChosenNocProductCombo(LPDISPATCH lpRow);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	// (r.gonet 05/22/2014) - PLID 61832 - Open the charge level claim providers config dialog
	afx_msg void OnBnClickedInventoryConfigureChargeLevelProviders();
	// (j.jones 2016-04-07 10:22) - NX-100075 - added ability to remember the last charge provider
	afx_msg void OnCheckRememberChargeProvider();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INVEDITDLG_H__3331FE60_A8BF_11D2_8E52_00AA0064A698__INCLUDED_)

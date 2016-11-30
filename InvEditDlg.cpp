// InvEditDlg.cpp : implementation file
//
// (c.haag 2008-02-08 11:59) - PLID 28852 - Added Consignment edit boxes to the UI.
// These consist of all edit controls ending with _CONSIGNMENT, as well as new
// labels which are only visible for consignment items. There's too many places
// where these are referenced to reasonably comment them all.
//
// (c.haag 2008-02-21 09:24) - PLID 28852 - Per inventory discussions with clients, we
// are hiding all Consignment reorder quantity fields. We may reintroduce them someday;
// but for now we will comment them out.
//
#include "stdafx.h"
#include "InventoryRc.h"
#include "InvEditDlg.h"
#include "InvNew.h"
#include "InvAdj.h"
#include "InvUtils.h"
#include "Client.h"
#include "Barcode.h"
#include "GlobalDataUtils.h"
#include "GlobalFinancialUtils.h"
#include "InactiveServiceItems.h"
#include "AddSupplierDlg.h"
#include "ProductItemsDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "globalreportutils.h"
#include "AuditTrail.h"
#include "EquipmentMaintenanceDlg.h"
#include "InternationalUtils.h"
#include "InvTransferDlg.h"
#include "PendingCaseHistoriesDlg.h"
#include "AdvRevCodeSetupDlg.h"
#include "MultipleRevCodesDlg.h"
#include "CategorySelectDlg.h"
#include "ProductItemTransferDlg.h"
#include "SuggestedSalesDlg.h"
#include "dontshowdlg.h"
#include "GlobalSchedUtils.h"
#include "TodoUtils.h"
#include "LinkProductsToServicesDlg.h"
#include "InvFramesDataDlg.h"
#include "InvLabelReportDlg.h"
#include "InvFramesDataDlg.h"
#include "InvContactLensDataDlg.h"
#include "NDCDefSetupDlg.h"	// (j.dinatale 2012-06-12 13:20) - PLID 32795
#include "InvManageOwnersDlg.h"
#include "EditShopFeesDlg.h" // (j.gruber 2012-10-26 15:44) - PLID 53239
#include "ConfigureChargeLevelProviderDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate windows message range (WM_USER -> 0x7FFF)
#define IDM_REPOSITION	WM_USER + 0xD1

#define ShowBox(id,val) GetDlgItem(id)->ShowWindow(val ? SW_SHOW : SW_HIDE)
//#define ShowBox(id, val) GetDlgItem(id)->PostMessage(WM_SHOWWINDOW, 0);

//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
#define MAX_WARRANTY_DAYS	3650				// 10 year warranty max.

enum DefaultCategoryFilter{
	dcfAllCategories = -1,
	dcfNoCategory = -2,
};

// (j.jones 2013-07-16 09:20) - PLID 57566 - added NOC combo
enum NOCColumns {
	noccID = 0,
	noccName,
};

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;
////////////////////////////////////////
CInvEditDlg::CInvEditDlg(CWnd* pParent)
	: CNxDialog(CInvEditDlg::IDD, pParent),
	m_tcProductChecker(NetUtils::Products), // (z.manning 2010-07-16 12:56) - PLID 39347
	m_supplierChecker(NetUtils::Suppliers),
	m_CPTCategoryChecker(NetUtils::CPTCategories),
	m_UB92CatChecker(NetUtils::UB92Cats),
	m_userChecker(NetUtils::Coordinators),
	m_providerChecker(NetUtils::Providers),
	m_LocationChecker(NetUtils::LocationsT)
{
	//{{AFX_DATA_INIT(CInvEditDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "System_Setup/Inventory_Setup/add_a_new_item.htm";

	m_refreshing = false;
	m_changed = false;
	m_bPopup = false;

	m_brBlue.CreateSolidBrush(PaletteColor(0x00FFDBDB));
	m_brDark.CreateSolidBrush(PaletteColor(0x00A0A0A0));
	
	//(a.wilson 2011-9-22) PLID 33502 - default to -1;
	m_nNextItemID = -1;
}

CInvEditDlg::~CInvEditDlg()
{
	InvUtils::SaveDefaultParameters();
}

BEGIN_MESSAGE_MAP(CInvEditDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvEditDlg)
	ON_BN_CLICKED(IDC_ADJUST, OnAdjust)
	ON_BN_CLICKED(IDC_BILLABLE, OnBillable)
	ON_BN_CLICKED(IDC_DELETE_ITEM, OnDeleteItem)
	ON_BN_CLICKED(IDC_NEW_ITEM, OnNewItem)
	ON_BN_CLICKED(IDC_TAXABLE, OnTaxable)
	ON_BN_CLICKED(IDC_PREV_ITEM, OnPrevItem)
	ON_BN_CLICKED(IDC_NEXT_ITEM, OnNextItem)
	ON_BN_CLICKED(IDC_TAXABLE2, OnTaxable2)
	ON_BN_CLICKED(IDC_MARK_ITEM_INACTIVE, OnMarkItemInactive)
	ON_BN_CLICKED(IDC_INACTIVE_ITEMS, OnInactiveItems)
	ON_BN_CLICKED(IDC_RADIO_NOT_TRACKABLE, OnRadioNotTrackable)
	ON_BN_CLICKED(IDC_RADIO_TRACK_ORDERS, OnRadioTrackOrders)
	ON_BN_CLICKED(IDC_RADIO_TRACK_QUANTITY, OnRadioTrackQuantity)
	ON_BN_CLICKED(IDC_ADD_SUPPLIER, OnAddSupplier)
	ON_BN_CLICKED(IDC_MAKE_DEFAULT, OnMakeDefault)
	ON_BN_CLICKED(IDC_DELETE_SUPPLIER, OnDeleteSupplier)
	ON_BN_CLICKED(IDC_CHECK_USE_UU, OnCheckUseUu)
	ON_BN_CLICKED(IDC_CHECK_SERIAL_NUM, OnCheckSerialNum)
	ON_BN_CLICKED(IDC_CHECK_EXP_DATE, OnCheckExpDate)
	ON_BN_CLICKED(IDC_BTN_SHOW_ITEMS, OnBtnShowItems)
	ON_BN_CLICKED(IDC_HISTORY, OnHistory)
	ON_EN_KILLFOCUS(IDC_REORDERPOINT_UO, OnKillfocusReorderpointUo)
	ON_EN_KILLFOCUS(IDC_REORDERQUANTITY_UO, OnKillfocusReorderquantityUo)
	ON_BN_CLICKED(IDC_BTN_EQUIPMENT_MAINTENANCE, OnBtnEquipmentMaintenance)
	ON_BN_CLICKED(IDC_EQUIPMENT, OnEquipment)
	ON_EN_KILLFOCUS(IDC_REORDERQUANTITY, OnKillfocusReorderquantity)
	ON_BN_CLICKED(IDC_TRANSFER_ITEM, OnTransfer)
	ON_BN_CLICKED(IDC_PENDING_CH, OnPendingCh)
	ON_BN_CLICKED(IDC_ADV_REVCODE_INV_SETUP, OnAdvRevcodeInvSetup)
	ON_BN_CLICKED(IDC_CHECK_SINGLE_REV_CODE, OnCheckSingleRevCode)
	ON_BN_CLICKED(IDC_CHECK_MULTIPLE_REV_CODES, OnCheckMultipleRevCodes)
	ON_BN_CLICKED(IDC_BTN_EDIT_MULTIPLE_REV_CODES, OnBtnEditMultipleRevCodes)
	ON_BN_CLICKED(IDC_BTN_SELECT_CATEGORY, OnBtnSelectCategory)
	ON_BN_CLICKED(IDC_BTN_REMOVE_CATEGORY, OnBtnRemoveCategory)
	ON_BN_CLICKED(IDC_CHECK_SERIALIZED_PER_UO, OnCheckSerializedPerUo)
	ON_BN_CLICKED(IDC_BTN_ADD_NEW_SUPPLIER, OnBtnAddNewSupplier)
	ON_BN_CLICKED(IDC_BTN_TRANSFER_PRODUCT_ITEMS, OnBtnTransferProductItems)
	ON_BN_CLICKED(IDC_PREV_LOCATION, OnPrevLocation)
	ON_BN_CLICKED(IDC_NEXT_LOCATION, OnNextLocation)
	ON_BN_CLICKED(IDC_DEFAULT_CONSIGNMENT, OnDefaultConsignment)
	ON_BN_CLICKED(IDC_BTN_LINK_PRODUCTS, OnBtnLinkProducts)
	ON_EN_KILLFOCUS(IDC_REORDERPOINT_UO_CONSIGNMENT, OnKillfocusReorderpointUoConsignment)
	ON_BN_CLICKED(IDC_CHECK_FACILITY_FEE, OnCheckFacilityFee)
	ON_BN_CLICKED(IDC_TRACKABLE, OnTrackable)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_MESSAGE(IDM_REPOSITION, OnReposition)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_SERIAL_NUMBER_AS_LOT_NUMBER, OnSerialNumberAsLotNumber)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_VIEW_FRAMES_DATA, &CInvEditDlg::OnBnClickedViewFramesData)
	//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
	ON_BN_CLICKED(IDC_CHECK_WARRANTY, OnWarranty)
	ON_EN_KILLFOCUS(IDC_WARRANTY, &CInvEditDlg::OnEnKillfocusWarranty)
	ON_BN_CLICKED(IDC_CHECK_IS_FRAME_DATA, &CInvEditDlg::OnBnClickedCheckIsFrameData) // (s.dhole 2011-04-01 12:58) - PLID 43101
	ON_BN_CLICKED(IDC_IS_CONTACT_LENS, &CInvEditDlg::OnIsContactLens)
	ON_BN_CLICKED(IDC_BUTTON_PRINT_BC, &CInvEditDlg::OnBnClickedButtonPrintBc)
	ON_BN_CLICKED(IDC_VIEW_CONTACT_INV_DATA, &CInvEditDlg::OnBnClickedViewContactInvData)
	ON_BN_CLICKED(IDC_INV_NDC_DEF_BTN, &CInvEditDlg::OnBnClickedInvNdcDefBtn)
	ON_BN_CLICKED(IDC_MANAGE_OWNERS, &CInvEditDlg::OnBnClickedManageOwners)
	ON_BN_CLICKED(IDC_EDIT_SHOP_FEES, &CInvEditDlg::OnBnClickedEditShopFees)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_INVENTORY_CONFIGURE_CHARGE_LEVEL_PROVIDERS, &CInvEditDlg::OnBnClickedInventoryConfigureChargeLevelProviders)
	ON_BN_CLICKED(IDC_CHECK_REMEMBER_CHARGE_PROVIDER, OnCheckRememberChargeProvider)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvEditDlg message handlers

BEGIN_EVENTSINK_MAP(CInvEditDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvEditDlg)
	ON_EVENT(CInvEditDlg, IDC_RESP_LIST, 16 /* SelChosen */, OnSelChosenRespList, VTS_I4)
	ON_EVENT(CInvEditDlg, IDC_CATEGORY_LIST, 16 /* SelChosen */, OnSelChosenCategoryList, VTS_I4)
	ON_EVENT(CInvEditDlg, IDC_UB92_CATEGORIES, 16 /* SelChosen */, OnSelChosenUb92Categories, VTS_I4)
	ON_EVENT(CInvEditDlg, IDC_SUPPLIER_LIST, 10 /* EditingFinished */, OnEditingFinishedSupplierList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CInvEditDlg, IDC_SUPPLIER_LIST, 18 /* RequeryFinished */, OnRequeryFinishedSupplierList, VTS_I2)
	ON_EVENT(CInvEditDlg, IDC_SUPPLIER_LIST, 6 /* RButtonDown */, OnRButtonDownSupplierList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInvEditDlg, IDC_SUPPLIER_COMBO, 16 /* SelChosen */, OnSelChosenSupplierCombo, VTS_I4)
	ON_EVENT(CInvEditDlg, IDC_LOCATION_LIST, 16 /* SelChosen */, OnSelChosenLocationList, VTS_I4)
	ON_EVENT(CInvEditDlg, IDC_ITEM, 16 /* SelChosen */, OnSelChosenItem, VTS_I4)
	ON_EVENT(CInvEditDlg, IDC_ITEM, 18 /* RequeryFinished */, OnRequeryFinishedItem, VTS_I2)
	ON_EVENT(CInvEditDlg, IDC_DEFAULT_INV_PROVIDER, 16 /* SelChosen */, OnSelChosenDefaultInvProvider, VTS_I4)
	ON_EVENT(CInvEditDlg, IDC_ITEM, 1 /* SelChanging */, OnSelChangingItem, VTS_PI4)
	ON_EVENT(CInvEditDlg, IDC_RESP_LIST, 1 /* SelChanging */, OnSelChangingRespList, VTS_PI4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CInvEditDlg, IDC_NOC_PRODUCT_COMBO, 1, OnSelChangingNocProductCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvEditDlg, IDC_NOC_PRODUCT_COMBO, 16, OnSelChosenNocProductCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()


void CInvEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvEditDlg)
	DDX_Control(pDX, IDC_SERIAL_NUMBER_AS_LOT_NUMBER, m_nxbSerialAsLot);
	DDX_Control(pDX, IDC_CHECK_FACILITY_FEE, m_checkFacilityFee);
	DDX_Control(pDX, IDC_BTN_ADD_NEW_SUPPLIER, m_btnAddNewSupplier);
	DDX_Control(pDX, IDC_BTN_LINK_PRODUCTS, m_btnLinkProducts);
	DDX_Control(pDX, IDC_DEFAULT_CONSIGNMENT, m_btnDefaultConsignment);
	DDX_Control(pDX, IDC_NEXT_LOCATION, m_btnNextLocation);
	DDX_Control(pDX, IDC_PREV_LOCATION, m_btnPrevLocation);
	DDX_Control(pDX, IDC_BTN_TRANSFER_PRODUCT_ITEMS, m_btnTransferProductItems);
	DDX_Control(pDX, IDC_CHECK_SERIALIZED_PER_UO, m_checkSerializedPerUO);
	DDX_Control(pDX, IDC_BTN_REMOVE_CATEGORY, m_btnRemoveCategory);
	DDX_Control(pDX, IDC_BTN_SELECT_CATEGORY, m_btnSelectCategory);
	DDX_Control(pDX, IDC_BTN_EDIT_MULTIPLE_REV_CODES, m_btnEditMultipleRevCodes);
	DDX_Control(pDX, IDC_CHECK_MULTIPLE_REV_CODES, m_checkMultipleRevCodes);
	DDX_Control(pDX, IDC_CHECK_SINGLE_REV_CODE, m_checkSingleRevCode);
	DDX_Control(pDX, IDC_ADV_REVCODE_INV_SETUP, m_btnAdvRevCodeSetup);
	DDX_Control(pDX, IDC_PENDING_CH, m_btnPendingCases);
	DDX_Control(pDX, IDC_TRANSFER_ITEM, m_btnTransfer);
	DDX_Control(pDX, IDC_EQUIPMENT, m_checkEquipment);
	DDX_Control(pDX, IDC_BTN_EQUIPMENT_MAINTENANCE, m_btnEquipmentMaintenance);
	DDX_Control(pDX, IDC_HISTORY, m_btnHistory);
	DDX_Control(pDX, IDC_BTN_SHOW_ITEMS, m_btnShowItems);
	DDX_Control(pDX, IDC_CHECK_SERIAL_NUM, m_checkHasSerial);
	DDX_Control(pDX, IDC_CHECK_EXP_DATE, m_checkHasExpDate);
	DDX_Control(pDX, IDC_CHECK_USE_UU, m_checkUseUU);
	DDX_Control(pDX, IDC_DELETE_SUPPLIER, m_btnDeleteSupplier);
	DDX_Control(pDX, IDC_ADD_SUPPLIER, m_btnAddSupplier);
	DDX_Control(pDX, IDC_MAKE_DEFAULT, m_btnMakeDefault);
	DDX_Control(pDX, IDC_INACTIVE_ITEMS, m_inactiveItems);
	DDX_Control(pDX, IDC_RADIO_NOT_TRACKABLE, m_radioNotTrackable);
	DDX_Control(pDX, IDC_RADIO_TRACK_ORDERS, m_radioTrackOrders);
	DDX_Control(pDX, IDC_RADIO_TRACK_QUANTITY, m_radioTrackQuantity);
	DDX_Control(pDX, IDC_MARK_ITEM_INACTIVE, m_markItemInactiveButton);
	DDX_Control(pDX, IDC_TAXABLE2, m_Taxable2);
	DDX_Control(pDX, IDC_PREV_ITEM, m_prevItem);
	DDX_Control(pDX, IDC_NEXT_ITEM, m_nextItem);
	DDX_Control(pDX, IDC_DELETE_ITEM, m_deleteItemBtn);
	DDX_Control(pDX, IDC_NEW_ITEM, m_newItemBtn);
	DDX_Control(pDX, IDC_ADJUST, m_adjustBtn);
	DDX_Control(pDX, IDC_TAXABLE, m_taxable);
	DDX_Control(pDX, IDC_BILLABLE, m_billable);
	DDX_Control(pDX, IDC_NAME, m_nxeditName);
	DDX_Control(pDX, IDC_UNITDESC, m_nxeditUnitdesc);
	DDX_Control(pDX, IDC_LAST_COST, m_nxeditLastCost);
	DDX_Control(pDX, IDC_INV_SHOP_FEE, m_nxeditInvShopFee);
	DDX_Control(pDX, IDC_PRICE, m_nxeditPrice);
	DDX_Control(pDX, IDC_BARCODE, m_nxeditBarcode);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_CONVERSION_EDIT, m_nxeditConversionEdit);
	DDX_Control(pDX, IDC_UU_EDIT, m_nxeditUuEdit);
	DDX_Control(pDX, IDC_UO_EDIT, m_nxeditUoEdit);
	DDX_Control(pDX, IDC_ACTUAL, m_nxeditActual);
	DDX_Control(pDX, IDC_ACTUAL_UO, m_nxeditActualUo);
	DDX_Control(pDX, IDC_AVAIL_INV, m_nxeditAvailInv);
	DDX_Control(pDX, IDC_AVAIL_UO, m_nxeditAvailUo);
	DDX_Control(pDX, IDC_ORDER, m_nxeditOrder);
	DDX_Control(pDX, IDC_ORDER_UO, m_nxeditOrderUo);
	DDX_Control(pDX, IDC_REORDERPOINT, m_nxeditReorderpoint);
	DDX_Control(pDX, IDC_REORDERPOINT_UO, m_nxeditReorderpointUo);
	DDX_Control(pDX, IDC_REORDERQUANTITY, m_nxeditReorderquantity);
	DDX_Control(pDX, IDC_REORDERQUANTITY_UO, m_nxeditReorderquantityUo);
	DDX_Control(pDX, IDC_CATEGORYBOX, m_nxeditCategorybox);
	DDX_Control(pDX, IDC_INS_CODE, m_nxeditInsCode);
	DDX_Control(pDX, IDC_CATALOG_EDIT, m_nxeditCatalogEdit);
	DDX_Control(pDX, IDC_ACTUAL_CONSIGNMENT, m_nxeditActualConsignment);
	DDX_Control(pDX, IDC_ACTUAL_UO_CONSIGNMENT, m_nxeditActualUoConsignment);
	DDX_Control(pDX, IDC_AVAIL_CONSIGNMENT, m_nxeditAvailConsignment);
	DDX_Control(pDX, IDC_AVAIL_UO_CONSIGNMENT, m_nxeditAvailUoConsignment);
	DDX_Control(pDX, IDC_ORDER_CONSIGNMENT, m_nxeditOrderConsignment);
	DDX_Control(pDX, IDC_ORDER_UO_CONSIGNMENT, m_nxeditOrderUoConsignment);
	DDX_Control(pDX, IDC_REORDERPOINT_CONSIGNMENT, m_nxeditReorderpointConsignment);
	DDX_Control(pDX, IDC_REORDERPOINT_UO_CONSIGNMENT, m_nxeditReorderpointUoConsignment);
	DDX_Control(pDX, IDC_REORDERQUANTITY_CONSIGNMENT, m_nxeditReorderquantityConsignment);
	DDX_Control(pDX, IDC_REORDERQUANTITY_UO_CONSIGNMENT, m_nxeditReorderquantityUoConsignment);
	DDX_Control(pDX, IDC_CAT_FILTER_LABEL, m_nxstaticCatFilterLabel);
	DDX_Control(pDX, IDC_PRODUCT_FILTER_LABEL, m_nxstaticProductFilterLabel);
	DDX_Control(pDX, IDC_LAST_COST_TEXT, m_nxstaticLastCostText);
	DDX_Control(pDX, IDC_INV_SHOP_FEE_LABEL, m_nxstaticInvShopFeeLabel);
	DDX_Control(pDX, IDC_PRICE_TEXT, m_nxstaticPriceText);
	DDX_Control(pDX, IDC_BARCODE_TEXT, m_nxstaticBarcodeText);
	DDX_Control(pDX, IDC_UB92_CPT_LABEL, m_nxstaticUb92CptLabel);
	DDX_Control(pDX, IDC_CONVERSION_TEXT, m_nxstaticConversionText);
	DDX_Control(pDX, IDC_CONVERSION_TEXT2, m_nxstaticConversionText2);
	DDX_Control(pDX, IDC_UU_TEXT, m_nxstaticUuText);
	DDX_Control(pDX, IDC_UO_TEXT, m_nxstaticUoText);
	DDX_Control(pDX, IDC_LOC_CAPTION, m_nxstaticLocCaption);
	DDX_Control(pDX, IDC_UU_LABEL, m_nxstaticUuLabel);
	DDX_Control(pDX, IDC_UO_LABEL, m_nxstaticUoLabel);
	DDX_Control(pDX, IDC_ACTUAL_TEXT, m_nxstaticActualText);
	DDX_Control(pDX, IDC_AVAIL_TEXT, m_nxstaticAvailText);
	DDX_Control(pDX, IDC_ORDER_TEXT, m_nxstaticOrderText);
	DDX_Control(pDX, IDC_REORDERPOINT_TEXT, m_nxstaticReorderpointText);
	DDX_Control(pDX, IDC_REORDERQUANTITY_TEXT, m_nxstaticReorderquantityText);
	DDX_Control(pDX, IDC_RESP_TEXT, m_nxstaticRespText);
	DDX_Control(pDX, IDC_SUPPLIER_LABEL, m_nxstaticSupplierLabel);
	DDX_Control(pDX, IDC_CATALOG_LABEL, m_nxstaticCatalogLabel);
	DDX_Control(pDX, IDC_UU_LABEL_CONSIGNMENT, m_nxstaticUuLabelConsignment);
	DDX_Control(pDX, IDC_UO_LABEL_CONSIGNMENT, m_nxstaticUoLabelConsignment);
	DDX_Control(pDX, IDC_LABEL_PURCHASED_INV, m_nxstaticLabelPurchasedInv);
	DDX_Control(pDX, IDC_LABEL_CONSIGNMENT, m_nxstaticLabelConsignment);
	DDX_Control(pDX, IDC_VIEW_FRAMES_DATA, m_btnViewFramesData);
	//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
	DDX_Control(pDX, IDC_CHECK_WARRANTY, m_CheckWarranty);
	DDX_Text(pDX, IDC_WARRANTY, m_strWarranty);
	DDV_MaxChars(pDX, m_strWarranty, 4);
	DDX_Control(pDX, IDC_CHECK_IS_FRAME_DATA, m_CheckIsFrameData); // (s.dhole 2011-04-01 12:58) - PLID 43101
	DDX_Control(pDX, IDC_IS_CONTACT_LENS, m_nxbIsContactLens);	
	DDX_Control(pDX, IDC_BUTTON_PRINT_BC, m_btnPrintBarcode);// (r.wilson 3/7/2012) - PLID 48351
	DDX_Control(pDX, IDC_VIEW_CONTACT_INV_DATA , m_btnViewContactLensData);
	DDX_Control(pDX, IDC_INV_NDC_DEF_BTN, m_btnNDCInfo);
	DDX_Control(pDX, IDC_MANAGE_OWNERS, m_btnManageOwners);
	DDX_Control(pDX, IDC_NOC_PRODUCT_LABEL, m_nxlNOC);
	DDX_Control(pDX, IDC_CHECK_REMEMBER_CHARGE_PROVIDER, m_checkRememberChargeProvider);
	//}}AFX_DATA_MAP
}

/*
(e.lally 2005-07-05) - This is a utility function - Currently setting a datalist's 
current selection to an invalid row fails silently. 
This may not be the case in the future. Rather than
have statements fail all over the place, we can have this function fail when
that change is made.
*/
BOOL SetCurrentSelection(_DNxDataListPtr& pTempDataList, long nRow)
{
	pTempDataList->PutCurSel(nRow);

	if(pTempDataList->CurSel == nRow)
		return TRUE;
	else
		return FALSE;

}

/* (e.lally 2005-07-05) - This is a utility function - There are a lot of places in here 
that were trying to set the current selection to 0, just to check if it failed and set it to -1. 
We really need to avoid doing in a lot of different places. 
This was done in order to check for an empty datalist.
Since datalists load asynchronously and GetRowCount does not wait for the datalist to
finish loading, this was the way to ensure the datalist was actually empty. Until a better solution
is found, we need to limit where we attempt to set a datalist to an invalid value.
*/
BOOL DetectEmptyList(_DNxDataListPtr& pTempDataList)
{
	//Do a quick check for a non-empty list
	if (pTempDataList->GetRowCount() > 0) {
		return FALSE;
	}
	//Since datalists load asynchronously, we have to force any requeries to finish before
	//	we can check it. Here we want to attempt to set the CurSel to a possibly invalid row 
	//  and then check if it was silently set to sriNoRow. In the future this may not silently
	//	throw an error and would need to be reworked.
	else {
		long nCurrentSelection = pTempDataList->CurSel;

		SetCurrentSelection(pTempDataList, 0);
		if(pTempDataList->CurSel == sriNoRow)
			return TRUE;
		else{
			SetCurrentSelection(pTempDataList, nCurrentSelection);
			return FALSE;
		}
	}

}

BOOL CInvEditDlg::OnInitDialog() 
{	
	CNxDialog::OnInitDialog();

	try {

		IRowSettingsPtr pRow;

		// (j.jones 2009-08-26 08:49) - PLID 35124 - Load all common properties into the
		// NxPropManager cache
		g_propManager.CachePropertiesInBulk("InvEditDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'UpdateSurgeryPrices' OR "
			"Name = 'UpdateSurgeryPriceWhen' OR "
			"Name = 'UpdatePreferenceCardPrices' OR "
			"Name = 'UpdatePreferenceCardPriceWhen' "
			")",
			_Q(GetCurrentUserName()));

		if(!IsSurgeryCenter(false)) {
			//if we're not showing Surgery Center stuff, hide the following:

			//multi-supplier utils
			ShowBox(IDC_ADD_SUPPLIER, FALSE);
			ShowBox(IDC_DELETE_SUPPLIER, FALSE);
			ShowBox(IDC_MAKE_DEFAULT, FALSE);
			ShowBox(IDC_SUPPLIER_LIST, FALSE);
			ShowBox(IDC_EQUIPMENT, FALSE);
			ShowBox(IDC_BTN_EQUIPMENT_MAINTENANCE, FALSE);
			ShowBox(IDC_PENDING_CH, FALSE);
		}
		else {
			//if we are showing Surgery Center stuff, show all except the following:

			//legacy single-supplier and catalog info
			ShowBox(IDC_SUPPLIER_LABEL, FALSE);
			ShowBox(IDC_SUPPLIER_COMBO, FALSE);
			ShowBox(IDC_BTN_ADD_NEW_SUPPLIER, FALSE);
			ShowBox(IDC_CATALOG_LABEL, FALSE);
			ShowBox(IDC_CATALOG_EDIT, FALSE);
		}

		// (j.fouts 2012-08-23 17:31) - PLID 50934 - Make sure this is only available on internal
		if(IsNexTechInternal())
		{
			m_btnManageOwners.ShowWindow(SW_SHOW);
			m_btnManageOwners.EnableWindow(TRUE);
			// (d.thompson 2014-10-28 11:43) - PLID 64008 - 11.4 added a "configure charge level providers" button and put it over top of manage owners.  Since
			//	that feature is useless to internal, I'm just going to hide it.
			GetDlgItem(IDC_INVENTORY_CONFIGURE_CHARGE_LEVEL_PROVIDERS)->ShowWindow(SW_HIDE);
		} else {
			m_btnManageOwners.ShowWindow(SW_HIDE);
			m_btnManageOwners.EnableWindow(FALSE);
		}

		//DRT 4/19/2004 - If they do not have a NexSpa liense, hide the Shop Fee fields
		if(!IsSpa(FALSE)) {
			GetDlgItem(IDC_INV_SHOP_FEE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_INV_SHOP_FEE)->ShowWindow(SW_HIDE);
			// (j.gruber 2012-12-04 11:08) - PLID 53239 - also the ellipsis button
			GetDlgItem(IDC_EDIT_SHOP_FEES)->ShowWindow(SW_HIDE);
			
		}

		m_adjustBtn.AutoSet(NXB_MODIFY);
		m_newItemBtn.AutoSet(NXB_NEW);
		m_deleteItemBtn.AutoSet(NXB_DELETE);
		m_markItemInactiveButton.AutoSet(NXB_MODIFY);
		m_btnTransfer.AutoSet(NXB_DELETE);
		m_btnHistory.AutoSet(NXB_PRINT_PREV);
		m_btnPrevLocation.AutoSet(NXB_LEFT);
		m_btnNextLocation.AutoSet(NXB_RIGHT);
		// (c.haag 2008-04-24 16:26) - PLID 29778 - NxIconify additional buttons
		m_btnRemoveCategory.AutoSet(NXB_DELETE);
		m_btnAddSupplier.AutoSet(NXB_NEW);
		m_btnDeleteSupplier.AutoSet(NXB_DELETE);
		m_btnMakeDefault.AutoSet(NXB_MODIFY);
		m_btnSelectCategory.AutoSet(NXB_MODIFY);
		m_btnAdvRevCodeSetup.AutoSet(NXB_MODIFY);
		m_btnEditMultipleRevCodes.AutoSet(NXB_MODIFY);
		// (j.fouts 2012-09-10 11:10) - PLID 50934 -  Added owners button
		m_btnManageOwners.AutoSet(NXB_MODIFY);

		// (j.dinatale 2012-06-13 18:02) - PLID 32795
		m_btnNDCInfo.AutoSet(NXB_MODIFY);

		m_item = BindNxDataListCtrl(IDC_ITEM, false);
		m_category_list = BindNxDataListCtrl(IDC_CATEGORY_LIST);
		m_resp = BindNxDataListCtrl(IDC_RESP_LIST, false);

		m_UB92_Category = BindNxDataListCtrl(IDC_UB92_CATEGORIES);

		if(IsSurgeryCenter(false)) {
			//if surgery center, bind the MultiSupplier list		
			m_Supplier_List = BindNxDataListCtrl(IDC_SUPPLIER_LIST,false);
		}
		else {
			//if not surgery center, bind the SingleSupplier list
			m_Supplier_Combo = BindNxDataListCtrl(IDC_SUPPLIER_COMBO);
		}

		m_DefaultProviderCombo = BindNxDataListCtrl(IDC_DEFAULT_INV_PROVIDER);

		// (j.jones 2013-07-16 08:46) - PLID 57566 - supported NOC type
		m_NOCCombo = BindNxDataList2Ctrl(IDC_NOC_PRODUCT_COMBO, false);

		//add NOC options for <Default>, No, and Yes
		{
			NXDATALIST2Lib::IRowSettingsPtr pNOCRow = m_NOCCombo->GetNewRow();
			pNOCRow->PutValue(noccID, (long)noctDefault);
			pNOCRow->PutValue(noccName, "<Default>");
			m_NOCCombo->AddRowAtEnd(pNOCRow, NULL);
			pNOCRow = m_NOCCombo->GetNewRow();
			pNOCRow->PutValue(noccID, (long)noctNo);
			pNOCRow->PutValue(noccName, "No");
			m_NOCCombo->AddRowAtEnd(pNOCRow, NULL);
			pNOCRow = m_NOCCombo->GetNewRow();
			pNOCRow->PutValue(noccID, (long)noctYes);
			pNOCRow->PutValue(noccName, "Yes");
			m_NOCCombo->AddRowAtEnd(pNOCRow, NULL);
		}

		// (j.jones 2013-07-16 08:46) - PLID 57566 - added NOC label
		m_nxlNOC.SetColor(GetNxColor(GNC_INVENTORY, 0));
		m_nxlNOC.SetType(dtsHyperlink);
		m_nxlNOC.SetSingleLine(true);
		m_nxlNOC.SetText("NOC Code");

		_variant_t var;
		var.vt = VT_NULL;

		// (j.jones 2011-06-24 15:13) - PLID 22586 - now we have an option to "use default",
		// and one to literally default to no provider
		pRow = m_DefaultProviderCombo->GetRow(-1);
		pRow->PutValue(0,var);
		pRow->PutValue(1,_variant_t(" {Use Default Bill Provider}"));
		m_DefaultProviderCombo->AddRow(pRow);
		pRow = m_DefaultProviderCombo->GetRow(-1);
		pRow->PutValue(0,(long)-2);
		pRow->PutValue(1,_variant_t(" {Use No Provider}"));
		m_DefaultProviderCombo->AddRow(pRow);	

		m_pLocations = BindNxDataListCtrl(IDC_LOCATION_LIST);
		m_nLocID = GetCurrentLocation();

		LoadUsersForLocation();

		m_prevItem.AutoSet(NXB_LEFT);
		m_nextItem.AutoSet(NXB_RIGHT);

		//Since we have to re-add this row every time
			//a requery is done, we should make this its own function
		AddDefaultFilters();

		//if surgery center, add row to UB92 combo
		pRow = m_UB92_Category->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, (LPCTSTR)"<None>");
		pRow->PutValue(2, (LPCTSTR)"<No Category Selected>");
		m_UB92_Category->AddRow(pRow);

		if(!IsSurgeryCenter(false)) {
			//if not surgery center, add row to supplier combo
			IRowSettingsPtr pNoneRow;
			pNoneRow = m_Supplier_Combo->GetRow(-1);
			pNoneRow->PutValue(0, (long)-1);
			pNoneRow->PutValue(1, (LPCTSTR)"<No Supplier Selected>");
			m_Supplier_Combo->InsertRow(pNoneRow, 0);
		}

		m_category_list->SetSelByColumn(0, InvUtils::GetDefaultCategory());

		m_refreshing = true;
		//(e.lally 2006-08-02) PLID 21733 - We don't need to specifically
		//call refresh and update arrows here. Both are done by other handlers.
		SelChosenCategoryList(-1);
		m_refreshing = false;

		long nDefProductID = InvUtils::GetDefaultItem();
		if(nDefProductID > 0)
			m_item->SetSelByColumn(0, nDefProductID);
		else
			SetCurrentSelection(m_item, 0);

		//TS: This irritates me, but it seems necessary.
		//(e.lally 2006-08-02) PLID 21733- actually, now that it is in the OnRequeryFinished event
			//handler for the item list, it no longer seems necessary as it is getting called
			//twice here.
		//UpdateArrows();

		// Disable controls if we don't have security access
		SecureControls();

		if(m_bPopup) {
			//we're in a popup window (usually from BillingModuleDlg).  A few things might need changed to look right.
			SetDlgItemText(IDC_BTN_EQUIPMENT_MAINTENANCE, "Equipment Maint.");
			//TES 1/27/2004: Don't let them view any reports.
			GetDlgItem(IDC_HISTORY)->EnableWindow(FALSE);

			Refresh();
		}

		//(e.lally 2005-6-2) PLID 16607 - Need to handle no inventory items - disable all fields until item is added
		if(DetectEmptyList(m_item)){
			EnableAll(FALSE);
		}
		else{
			EnableAll(TRUE);
		}

		//DRT 2/25/2008 - PLID 28876 - I moved this from Refresh() to here because it should only
		//	happen once.  We reposition based on the license settings, and if they update their 
		//	license, they'll have to restart the module.
		//Note:  I cannot explain why, but the code will not function if it's inside OnInitDialog.  It's
		//	like something in the loading of an inventory dialog forces all changes to positioning to
		//	be abandoned at the end of this function.  Posting a message to update it seems to do the trick.
		PostMessage(IDM_REPOSITION, 0, 0);

	}NxCatchAll("Error in OnInitDialog");

	return FALSE;
}

//DRT 2/25/2008 - PLID 28876 - I moved this from Refresh() to here because it should only
//	happen once.  We reposition based on the license settings, and if they update their 
//	license, they'll have to restart the module.
LRESULT CInvEditDlg::OnReposition(WPARAM wParam, LPARAM lParam)
{
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(IsSurgeryCenter(false) || g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			//We have chosen that it always shows, regardless of the difference in qty
		}
		else {
			//Hide the available boxes
			GetDlgItem(IDC_AVAIL_INV)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_AVAIL_UO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_AVAIL_CONSIGNMENT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_AVAIL_UO_CONSIGNMENT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_AVAIL_TEXT)->ShowWindow(SW_HIDE);

			//Now to further complicate things, we need to shove the boxes around, because available
			//	is below actual, and we don't want to have a big gap.
			//1)  Get the current coords of the actual boxes
			CRect rcActual, rcActualConsign, rcActualUO, rcActualUOConsign, rcActualLabel;
			GetDlgItem(IDC_ACTUAL)->GetWindowRect(&rcActual);
			GetDlgItem(IDC_ACTUAL_UO)->GetWindowRect(&rcActualUO);
			GetDlgItem(IDC_ACTUAL_CONSIGNMENT)->GetWindowRect(&rcActualConsign);
			GetDlgItem(IDC_ACTUAL_UO_CONSIGNMENT)->GetWindowRect(&rcActualUOConsign);
			GetDlgItem(IDC_ACTUAL_TEXT)->GetWindowRect(&rcActualLabel);
			ScreenToClient(&rcActual);
			ScreenToClient(&rcActualUO);
			ScreenToClient(&rcActualConsign);
			ScreenToClient(&rcActualUOConsign);
			ScreenToClient(&rcActualLabel);

			//2)  Get the current coords of the avail boxes
			CRect rcAvail, rcAvailConsign, rcAvailUO, rcAvailUOConsign, rcAvailLabel;
			GetDlgItem(IDC_AVAIL_INV)->GetWindowRect(&rcAvail);
			GetDlgItem(IDC_AVAIL_UO)->GetWindowRect(&rcAvailUO);
			GetDlgItem(IDC_AVAIL_CONSIGNMENT)->GetWindowRect(&rcAvailConsign);
			GetDlgItem(IDC_AVAIL_UO_CONSIGNMENT)->GetWindowRect(&rcAvailUOConsign);
			GetDlgItem(IDC_AVAIL_TEXT)->GetWindowRect(&rcAvailLabel);
			ScreenToClient(&rcAvail);
			ScreenToClient(&rcAvailUO);
			ScreenToClient(&rcAvailConsign);
			ScreenToClient(&rcAvailUOConsign);
			ScreenToClient(&rcAvailLabel);

			//3)  Shift all the actual down to avail
			GetDlgItem(IDC_ACTUAL)->SetWindowPos(NULL, rcAvail.left, rcAvail.top, rcAvail.Width(), rcAvail.Height(), NULL);
			GetDlgItem(IDC_ACTUAL_UO)->SetWindowPos(NULL, rcAvailUO.left, rcAvailUO.top, rcAvailUO.Width(), rcAvailUO.Height(), NULL);
			GetDlgItem(IDC_ACTUAL_CONSIGNMENT)->SetWindowPos(NULL, rcAvailConsign.left, rcAvailConsign.top, rcAvailConsign.Width(), rcAvailConsign.Height(), NULL);
			GetDlgItem(IDC_ACTUAL_UO_CONSIGNMENT)->SetWindowPos(NULL, rcAvailUOConsign.left, rcAvailUOConsign.top, rcAvailUOConsign.Width(), rcAvailUOConsign.Height(), NULL);
			GetDlgItem(IDC_ACTUAL_TEXT)->SetWindowPos(NULL, rcAvailLabel.left, rcAvailLabel.top, rcAvailLabel.Width(), rcAvailLabel.Height(), NULL);
			InvalidateRect(&rcAvailLabel);

			//4)  Shift the uu/uo labels down to the actual positions
			GetDlgItem(IDC_UU_LABEL)->SetWindowPos(NULL, rcActual.left, rcActual.top, rcActual.Width(), rcActual.Height(), NULL);
			GetDlgItem(IDC_UO_LABEL)->SetWindowPos(NULL, rcActualUO.left, rcActualUO.top, rcActualUO.Width(), rcActualUO.Height(), NULL);
			GetDlgItem(IDC_UU_LABEL_CONSIGNMENT)->SetWindowPos(NULL, rcActualConsign.left, rcActualConsign.top, rcActualConsign.Width(), rcActualConsign.Height(), NULL);
			GetDlgItem(IDC_UO_LABEL_CONSIGNMENT)->SetWindowPos(NULL, rcActualUOConsign.left, rcActualUOConsign.top, rcActualUOConsign.Width(), rcActualUOConsign.Height(), NULL);
		}

		return 0;
}

void CInvEditDlg::EnableAll(BOOL bIsEnabled)
{
	try {

		//ShowBox(IDC_CATALOG, bIsEnabled);
		GetDlgItem(IDC_CATEGORY_LIST)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_PREV_ITEM)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_ITEM)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_NEXT_ITEM)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_MARK_ITEM_INACTIVE)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_DELETE_ITEM)->EnableWindow(bIsEnabled);

		GetDlgItem(IDC_NAME)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_UNITDESC)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_LAST_COST)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_TAXABLE)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_TAXABLE2)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_INV_SHOP_FEE)->EnableWindow(bIsEnabled);
		// (j.gruber 2012-12-04 11:10) - PLID 53239 - edit shop fee
		GetDlgItem(IDC_EDIT_SHOP_FEES)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_PRICE)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_BARCODE)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_CHECK_SINGLE_REV_CODE)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_UB92_CPT_CATEGORIES)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_CHECK_MULTIPLE_REV_CODES)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_BTN_EDIT_MULTIPLE_REV_CODES)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_ADV_REVCODE_INV_SETUP)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_NOTES)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_CHECK_USE_UU)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_CONVERSION_EDIT)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_UU_EDIT)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_UO_EDIT)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_LOCATION_LIST)->EnableWindow(bIsEnabled);
		if(!bIsEnabled){
			//Only disable the arrows, otherwise it is assumed that they
			//will be properly updated.
			GetDlgItem(IDC_PREV_LOCATION)->EnableWindow(FALSE);
			GetDlgItem(IDC_NEXT_LOCATION)->EnableWindow(FALSE);
		}
		GetDlgItem(IDC_BILLABLE)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_RADIO_NOT_TRACKABLE)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_RADIO_TRACK_ORDERS)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_RADIO_TRACK_QUANTITY)->EnableWindow(bIsEnabled);

		GetDlgItem(IDC_REORDERPOINT)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_REORDERPOINT_CONSIGNMENT)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_REORDERPOINT_UO)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_REORDERPOINT_UO_CONSIGNMENT)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_REORDERQUANTITY)->EnableWindow(bIsEnabled);
		//GetDlgItem(IDC_REORDERQUANTITY_CONSIGNMENT)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_REORDERQUANTITY_UO)->EnableWindow(bIsEnabled);
		//GetDlgItem(IDC_REORDERQUANTITY_UO_CONSIGNMENT)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_RESP_LIST)->EnableWindow(bIsEnabled);

		GetDlgItem(IDC_BTN_SELECT_CATEGORY)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_BTN_REMOVE_CATEGORY)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_INS_CODE)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_DEFAULT_INV_PROVIDER)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_SUPPLIER_LIST)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_ADD_SUPPLIER)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_DELETE_SUPPLIER)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_CHECK_EXP_DATE)->EnableWindow(bIsEnabled);
		// (j.jones 2010-01-29 09:19) - PLID 36705 - enable the catalog only if there is a selected single-select supplier
		if(!IsSurgeryCenter(false) && m_Supplier_Combo->GetCurSel() != -1) {
			GetDlgItem(IDC_CATALOG_EDIT)->EnableWindow(bIsEnabled);
		}
		else {
			GetDlgItem(IDC_CATALOG_EDIT)->EnableWindow(FALSE);
		}
		GetDlgItem(IDC_SUPPLIER_COMBO)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_BTN_ADD_NEW_SUPPLIER)->EnableWindow(bIsEnabled);
		
		GetDlgItem(IDC_CHECK_EXP_DATE)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_CHECK_SERIAL_NUM)->EnableWindow(bIsEnabled);
		//TES 7/3/2008 - PLID 24726 - Added
		GetDlgItem(IDC_SERIAL_NUMBER_AS_LOT_NUMBER)->EnableWindow(bIsEnabled && IsDlgButtonChecked(IDC_CHECK_SERIAL_NUM));
		GetDlgItem(IDC_CHECK_SERIALIZED_PER_UO)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_BTN_SHOW_ITEMS)->EnableWindow(bIsEnabled);
		//DRT 11/8/2007 - PLID 28042
		// (a.walling 2008-02-15 16:39) - PLID 28946 - Disable if unlicensed
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		GetDlgItem(IDC_DEFAULT_CONSIGNMENT)->EnableWindow(g_pLicense->HasCandAModule(CLicense::cflrSilent) && bIsEnabled);
		GetDlgItem(IDC_BTN_TRANSFER_PRODUCT_ITEMS)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_EQUIPMENT)->EnableWindow(bIsEnabled);

		// (j.jones 2016-04-07 10:22) - NX-100075 - added ability to remember the last charge provider
		GetDlgItem(IDC_CHECK_REMEMBER_CHARGE_PROVIDER)->EnableWindow(bIsEnabled);

		// (a.walling 2007-04-24 13:57) - PLID 25356
		// (a.wetta 2007-05-16 10:55) - PLID 25960 - This button has been removed
		//GetDlgItem(IDC_BTN_INV_SUGGESTED_SALES)->EnableWindow(bIsEnabled);

		//(c.copits 2010-11-02) PLID 38598 - Warranty tracking system
		GetDlgItem(IDC_CHECK_WARRANTY)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_WARRANTY)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_WARRANTY_TEXT)->EnableWindow(bIsEnabled);
		// (s.dhole 2011-04-01 12:59) - PLID 43101 Make sure that uncheck check obx and desable button
		GetDlgItem(IDC_CHECK_IS_FRAME_DATA)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_VIEW_FRAMES_DATA)->EnableWindow(bIsEnabled);
		// (s.dhole 2012-03-14 14:15) - PLID 48888
		GetDlgItem(IDC_VIEW_CONTACT_INV_DATA)->EnableWindow(bIsEnabled);
		//TES 5/24/2011 - PLID 43828
		

		//(r.wilson 3/7/2012) PLID 48351 - enable print barcode button
		GetDlgItem(IDC_BUTTON_PRINT_BC)->EnableWindow(bIsEnabled);

		GetDlgItem(IDC_IS_CONTACT_LENS)->EnableWindow(bIsEnabled);
		// (j.fouts 2012-08-23 17:32) - PLID 50934 - Internal Inventory Owners button
		GetDlgItem(IDC_MANAGE_OWNERS)->EnableWindow(bIsEnabled);
		// (r.gonet 05/23/2014) - PLID 61832 - Configure Charge Level Providers button
		GetDlgItem(IDC_INVENTORY_CONFIGURE_CHARGE_LEVEL_PROVIDERS)->EnableWindow(bIsEnabled);

	}NxCatchAll("Error in CInvEditDlg::EnableAll");	// (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::LoadUsersForLocation()
{
	try {

		CString where;
		IRowSettingsPtr pRow;

		where.Format("PersonT.Archived = 0 AND UserLocationT.LocationID = %i", m_nLocID);

		m_resp->WhereClause = _bstr_t(where);
		m_resp->Requery();

		pRow = m_resp->GetRow(-1);
		pRow->PutValue(0,_variant_t());
		pRow->PutValue(1,_variant_t(" <No User> "));
		m_resp->AddRow(pRow);

	}NxCatchAll("Error in CInvEditDlg::LoadUsersForLocation"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::LoadUserInfo()
{
	try {

		//for the time being, we only allow 1 user per location per item - although the data can support many
		_RecordsetPtr rs = CreateRecordset("SELECT UserID AS UserID FROM ProductResponsibilityT "
			"WHERE LocationID = %i AND ProductID = %i",
			m_nLocID,
			GetItem());
		if (rs->eof)
			m_resp->SetSelByColumn(0, variant_t());
		else m_resp->SetSelByColumn(0, rs->Fields->Item["UserID"]->Value);

	}NxCatchAll("Error in CInvEditDlg::LoadUserInfo"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

long CInvEditDlg::GetItem()
{
	// (j.jones 2007-12-05 18:09) - PLID 28292 - added "no row" handling and exception handling
	try {

		if(m_item->CurSel == -1) {
			return -1;
		}

		return VarLong(m_item->GetValue(m_item->CurSel, 0), -1);

	}NxCatchAll("Error in CInvEditDlg::GetItem()");

	return -1;
}

#define SetBox(id,text) SetDlgItemVar(id,rs->Fields->Item[text]->Value,true,true)

void CInvEditDlg::Refresh()
{
	CWaitCursor wait;

	try {

		m_refreshing = true;
		
		// (b.eyers 2015-07-10) - PLID 24060 - set bools for both category and product table checkers
		bool bCPTCategory = m_CPTCategoryChecker.Changed();
		bool bProduct = m_tcProductChecker.Changed();

		if (m_supplierChecker.Changed()) {
			if(IsSurgeryCenter(false)) {
				//if surgery center, MultiSupplier list
				m_Supplier_List->Requery();
			}
			else {
				//if not surgery center, single supplier list
				m_Supplier_Combo->Requery();
					
				//if not surgery center, add row to supplier combo
				IRowSettingsPtr pNoneRow;
				pNoneRow = m_Supplier_Combo->GetRow(-1);
				pNoneRow->PutValue(0, (long)-1);
				pNoneRow->PutValue(1, (LPCTSTR)"<No Supplier Selected>");
				m_Supplier_Combo->InsertRow(pNoneRow, 0);
			}
		}

		if (bCPTCategory|| bProduct) { // (b.eyers 2015-07-10) - PLID 24060 - if category or product table checker came in, reload the category list
		//if (m_CPTCategoryChecker.Changed()) {
			// (j.jones 2015-03-03 09:05) - PLID 64964 - moved the category reload
			// to a modular function
			ReloadCategoryList();
		}

		if(m_UB92CatChecker.Changed()) {
			m_UB92_Category->Requery();
			IRowSettingsPtr pRow;
			pRow = m_UB92_Category->GetRow(-1);
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, (LPCTSTR)"<None>");
			pRow->PutValue(2, (LPCTSTR)"<No Category Selected>");
			m_UB92_Category->AddRow(pRow);
		}
		
		if(m_providerChecker.Changed()) {
			m_DefaultProviderCombo->Requery();

			IRowSettingsPtr pRow;
			_variant_t var;
			var.vt = VT_NULL;
			
			// (j.jones 2011-06-24 15:13) - PLID 22586 - now we have an option to "use default",
			// and one to literally default to no provider
			pRow = m_DefaultProviderCombo->GetRow(-1);
			pRow->PutValue(0,var);
			pRow->PutValue(1,_variant_t(" {Use Default Bill Provider}"));
			m_DefaultProviderCombo->AddRow(pRow);
			pRow = m_DefaultProviderCombo->GetRow(-1);
			pRow->PutValue(0,(long)-2);
			pRow->PutValue(1,_variant_t(" {Use No Provider}"));
			m_DefaultProviderCombo->AddRow(pRow);	
		}

		bool bLoadUsersForLocation = m_userChecker.Changed();

		if(m_LocationChecker.Changed()) {
			m_pLocations->Requery();

			if(-1 == m_pLocations->SetSelByColumn(0, m_nLocID)) {
				SetCurrentSelection(m_pLocations, 0);
				m_nLocID = VarLong(m_pLocations->GetValue(m_pLocations->CurSel,0));

				// (a.walling 2010-07-07 16:05) - PLID 39570 - Need to refresh the users for this location, too, since the location has changed.
				LoadUsersForLocation();
				bLoadUsersForLocation = false;
			}
		}

		if(bLoadUsersForLocation) {
			LoadUsersForLocation();
		}

		if (m_item->CurSel == -1) {
			SetCurrentSelection(m_item, 0);
			if(m_item->CurSel == 0){
				InvUtils::SetDefault(GetItem(), InvUtils::GetDefaultCategory());
			}
		}

		if (m_item->CurSel == -1) {
			//no inventory items
			SetDlgItemText(IDC_NAME,"");
			SetDlgItemText(IDC_UNITDESC,"");
			SetDlgItemText(IDC_PRICE,"");
			SetDlgItemText(IDC_REORDERQUANTITY,"");
			//SetDlgItemText(IDC_REORDERQUANTITY_CONSIGNMENT,"");
			SetDlgItemText(IDC_REORDERPOINT,"");
			SetDlgItemText(IDC_REORDERPOINT_CONSIGNMENT,"");
			SetDlgItemText(IDC_ACTUAL,"");
			SetDlgItemText(IDC_ACTUAL_CONSIGNMENT,"");
			//DRT 2/8/2008 - PLID 28876
			SetDlgItemText(IDC_AVAIL_INV,"");
			SetDlgItemText(IDC_AVAIL_CONSIGNMENT,"");
			SetDlgItemText(IDC_NOTES,"");
			SetDlgItemText(IDC_ORDER,"");
			SetDlgItemText(IDC_ORDER_CONSIGNMENT,"");
			SetDlgItemText(IDC_LAST_COST,"");
			SetDlgItemText(IDC_BARCODE,"");
			SetDlgItemText(IDC_INS_CODE,"");
			SetDlgItemText(IDC_INV_SHOP_FEE, "");
			m_billable.SetCheck(FALSE);
			// (j.jones 2008-06-10 11:28) - PLID 27665 - added m_checkFacilityFee
			m_checkFacilityFee.SetCheck(FALSE);
			m_checkEquipment.SetCheck(FALSE);
			ShowBox(IDC_BTN_EQUIPMENT_MAINTENANCE, FALSE);
			m_taxable.SetCheck(FALSE);
			m_Taxable2.SetCheck(FALSE);
			m_radioNotTrackable.SetCheck(FALSE);
			m_radioTrackOrders.SetCheck(FALSE);
			m_radioTrackQuantity.SetCheck(FALSE);
			SetDlgItemText(IDC_CATEGORYBOX, "");
			SetCurrentSelection(m_DefaultProviderCombo, sriNoRow);
			m_DefaultProviderCombo->Enabled = FALSE;
			OnBillable();
			OnTaxable();
			OnTaxable2();
			OnTrackable();
//			OnEquipment();

			//DRT 11/8/2007 - PLID 28042
			m_btnDefaultConsignment.SetCheck(FALSE);

			//UB92 Categories
			SetCurrentSelection(m_UB92_Category, sriNoRow);
			m_UB92_Category->Enabled = FALSE;
			m_checkSingleRevCode.SetCheck(FALSE);
			m_checkMultipleRevCodes.SetCheck(FALSE);
			m_UB92_Category->Enabled = FALSE;
			m_btnEditMultipleRevCodes.EnableWindow(FALSE);

			//Unit Of Order / Unit Of Usage utilities
			m_checkUseUU.SetCheck(FALSE);
			SetDlgItemText(IDC_UU_EDIT,"");
			SetDlgItemText(IDC_UO_EDIT,"");
			SetDlgItemText(IDC_CONVERSION_EDIT,"");
			SetDlgItemText(IDC_REORDERQUANTITY,"");
			//SetDlgItemText(IDC_REORDERQUANTITY_CONSIGNMENT,"");
			SetDlgItemText(IDC_REORDERPOINT,"");
			SetDlgItemText(IDC_REORDERPOINT_CONSIGNMENT,"");
			SetDlgItemText(IDC_ACTUAL_UO,"");
			SetDlgItemText(IDC_ACTUAL_UO_CONSIGNMENT,"");
			//DRT 2/8/2008 - PLID 28876
			SetDlgItemText(IDC_AVAIL_UO,"");
			SetDlgItemText(IDC_AVAIL_UO_CONSIGNMENT,"");
			SetDlgItemText(IDC_ORDER_UO,"");
			SetDlgItemText(IDC_ORDER_UO_CONSIGNMENT,"");
			SetDlgItemText(IDC_REORDERPOINT_UO,"");
			SetDlgItemText(IDC_REORDERPOINT_UO_CONSIGNMENT,"");
			SetDlgItemText(IDC_REORDERQUANTITY_UO,"");
			//SetDlgItemText(IDC_REORDERQUANTITY_UO_CONSIGNMENT,"");
			OnCheckUseUu();

			if(IsSurgeryCenter(false)) {
				//multi-supplier utils
				GetDlgItem(IDC_ADD_SUPPLIER)->EnableWindow(FALSE);
				GetDlgItem(IDC_MAKE_DEFAULT)->EnableWindow(FALSE);
				m_Supplier_List->Clear();
			}
			else {
				//if not surgery center, set the following defaults:

				//legacy single-supplier and catalog info
				SetCurrentSelection(m_Supplier_Combo, sriNoRow);
				SetDlgItemText(IDC_CATALOG_EDIT, "");
				GetDlgItem(IDC_CATALOG_EDIT)->EnableWindow(FALSE);
			}

			//serialized / expirable products
			m_checkHasSerial.SetCheck(FALSE);
			//TES 7/3/2008 - PLID 24726 - Added
			m_nxbSerialAsLot.SetCheck(FALSE);
			//TES 7/3/2008 - PLID 24726 - Since Has Serial Number is unchecked, this should be disabled.
			m_nxbSerialAsLot.EnableWindow(FALSE);
			m_checkHasExpDate.SetCheck(FALSE);
			ShowBox(IDC_BTN_SHOW_ITEMS, FALSE);
			ShowBox(IDC_BTN_TRANSFER_PRODUCT_ITEMS, FALSE);
			//DRT 11/8/2007 - PLID 28042
			ShowBox(IDC_DEFAULT_CONSIGNMENT, FALSE);

			// (a.walling 2007-04-24 14:02) - PLID 25356
			// (a.wetta 2007-05-16 10:55) - PLID 25960 - This button has been removed
			//GetDlgItem(IDC_BTN_INV_SUGGESTED_SALES)->EnableWindow(FALSE);

			// (j.jones 2016-04-07 10:22) - NX-100075 - added ability to remember the last charge provider
			m_checkRememberChargeProvider.SetCheck(FALSE);

			//(c.copits 2010-11-02) PLID 38598 - Warranty tracking system
			SetDlgItemText(IDC_WARRANTY, "");
			GetDlgItem(IDC_WARRANTY)->EnableWindow(FALSE);
			m_CheckWarranty.SetCheck(FALSE);
			GetDlgItem(IDC_CHECK_WARRANTY)->EnableWindow(FALSE);

			// (j.jones 2013-07-16 11:08) - PLID 57566 - clear the NOC code
			m_NOCCombo->PutCurSel(NULL);
			GetDlgItem(IDC_NOC_PRODUCT_COMBO)->EnableWindow(FALSE);
			
			m_refreshing = false;
			return;
		}

		EnsureRemoteData();

		// (j.jones 2008-02-28 10:40) - PLID 28080 - converted item_sql into GetInventoryItemSql()
		// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
		_RecordsetPtr rs = CreateParamRecordset(InvUtils::GetInventoryItemSql(GetItem(), m_nLocID));

		//(e.lally 2008-03-11) - c.haag wants to put in a new item to check for an empty rs, but
		//I think we should put in a check now and throw a helpful exception if the recordset
		//is empty (because of bad data). I am not putting this into a PL item, because it should be part of
		// PLID 28852 since that is the first place this rs is used, even if there was not previously a check for this.
		if(rs->eof){
			ThrowNxException("The product information could not be loaded!");
		}

		SetBox(IDC_NAME,			"Name");
		SetBox(IDC_UNITDESC,		"UnitDesc");
		SetBox(IDC_PRICE,			"Price");
		FormatDlgCurrency(IDC_PRICE);

		// (c.haag 2008-02-07 17:50) - PLID 28852 - Split into consignment
		// and purchased inventory tallies.
		//
		const double dConsignmentActual = AdoFldDouble(rs, "ActualConsignment",0);
		const double dConsignmentAvail = AdoFldDouble(rs, "AvailConsignment",0);

		const double dPurchActual = AdoFldDouble(rs, "ActualPurchasedInv",0);
		const double dPurchAvail = AdoFldDouble(rs, "AvailPurchasedInv",0);

		//DRT 2/25/2008 - PLID 28876 - Reinstituted the licensing.  Now, if you have either ASC or
		//	the new advanced inventory license, the available box will show.  If you do not, it
		//	will never show.
		//Set the boxes either way, we're justing fighting the visuals here.
		SetDlgItemVar(IDC_ACTUAL,dPurchActual,true,true);
		SetDlgItemVar(IDC_ACTUAL_CONSIGNMENT,dConsignmentActual,true,true);
		//DRT 2/8/2008 - PLID 28876
		SetDlgItemVar(IDC_AVAIL_INV, dPurchAvail, true, true);
		SetDlgItemVar(IDC_AVAIL_CONSIGNMENT, dConsignmentAvail, true, true);

		// (c.haag 2008-02-08 09:56) - PLID 28852 - Split into consignment and
		// purchased inventory tallies.
		const long nConsignmentOrdered = AdoFldLong(rs, "OrderedConsignment",0);
		const long nPurchOrdered = AdoFldLong(rs, "OrderedPurchasedInv",0);

		SetBox(IDC_REORDERQUANTITY,	"ReOrderQuantity");
		//SetBox(IDC_REORDERQUANTITY_CONSIGNMENT, "ConsignmentReorderQuantity");
		SetBox(IDC_REORDERPOINT,	"ReOrderPoint");
		SetBox(IDC_REORDERPOINT_CONSIGNMENT,	"ConsignmentReOrderPoint");
		SetBox(IDC_NOTES,			"Notes");
		SetDlgItemVar(IDC_ORDER,nPurchOrdered,true,true);
		SetDlgItemVar(IDC_ORDER_CONSIGNMENT,nConsignmentOrdered,true,true);
		SetBox(IDC_LAST_COST,		"LastCost");
		FormatDlgCurrency(IDC_LAST_COST);
		SetBox(IDC_BARCODE,			"Barcode");
		SetBox(IDC_INS_CODE,		"InsCode");

		// (j.jones 2013-07-16 11:08) - PLID 57566 - set the NOC code,
		//if NULL, it's Default, otherwise it's a boolean for Yes/No
		GetDlgItem(IDC_NOC_PRODUCT_COMBO)->EnableWindow(TRUE);
		_variant_t varNOCType = rs->Fields->Item["NOCType"]->Value;
		NOCTypes eType = noctDefault;
		if(varNOCType.vt == VT_BOOL) {
			BOOL bNOCType = VarBool(varNOCType);
			eType = bNOCType ? noctYes : noctNo;
		}
		m_NOCCombo->SetSelByColumn(noccID, (long)eType);

		// (j.gruber 2012-10-26 12:57) - PLID 53239
		m_cyShopFee = AdoFldCurrency(rs, "AdjustedShopFee");
		if (m_cyShopFee < COleCurrency(0,0)) {
			//this means that we have multiple
			SetDlgItemText(IDC_INV_SHOP_FEE, "<Multiple>");
			//grey out the field
			GetDlgItem(IDC_INV_SHOP_FEE)->EnableWindow(FALSE);
		}
		else {
			SetDlgItemText(IDC_INV_SHOP_FEE, FormatCurrencyForInterface(m_cyShopFee));
			GetDlgItem(IDC_INV_SHOP_FEE)->EnableWindow(TRUE);
		}

		m_billable.SetCheck(rs->Fields->Item["Billable"]->Value.boolVal);
		m_taxable.SetCheck(rs->Fields->Item["Taxable1"]->Value.boolVal);
		m_Taxable2.SetCheck(rs->Fields->Item["Taxable2"]->Value.boolVal);
		BOOL bIsEquipment = rs->Fields->Item["IsEquipment"]->Value.boolVal;
		//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
		m_CheckWarranty.SetCheck(rs->Fields->Item["WarrantyActive"]->Value.boolVal);
		SetBox(IDC_WARRANTY, "WarrantyDays");

		// (z.manning 2010-06-18 16:28) - PLID 39257 - Is this a Frames product?
		_variant_t varFramesDataID = rs->GetFields()->GetItem("FramesDataID")->GetValue();
		// (c.haag 2010-08-16 09:29) - PLID 39424 - Keep the box hidden if not licensed 
		// (s.dhole 2011-04-01 12:59) - PLID 43101 Check Glasses and frame license, if any of license available than allow them to add frame data
		// change CLicense::cflrUse to CLicense::cflrSilent
		if((g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)) ||
		 (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcFrames, CLicense::cflrSilent))) 
			{
			m_CheckIsFrameData.SetCheck(varFramesDataID.vt != VT_NULL);
			//TES 5/24/2011 - PLID 43828 - Changed to show/hide instead of enable/disable
			GetDlgItem(IDC_VIEW_FRAMES_DATA)->ShowWindow(varFramesDataID.vt != VT_NULL ? SW_SHOW : SW_HIDE);
			//r.wilson 3/7/2012 PLID 48351 - Determine if we should show or hide the Print barcode button (This is based on whether the item is a frame or not)
			GetDlgItem(IDC_BUTTON_PRINT_BC)->ShowWindow(varFramesDataID.vt != VT_NULL ? SW_SHOW : SW_HIDE);
			GetDlgItem(IDC_CHECK_IS_FRAME_DATA)->ShowWindow(SW_SHOW );

			// (s.dhole 2012-05-14 17:20) - PLID 48888  should enabl frame checkbox
			GetDlgItem(IDC_CHECK_IS_FRAME_DATA)->EnableWindow(TRUE);
			}
		else
			{
			ShowBox(IDC_CHECK_IS_FRAME_DATA, FALSE);
			ShowBox(IDC_VIEW_FRAMES_DATA, FALSE);
			//(r.wilson 3/7/2012) PLID 48351
			ShowBox(IDC_BUTTON_PRINT_BC, FALSE);
			//m_CheckIsFrameData.SetCheck(FALSE);
			}

		//TES 5/24/2011 - PLID 43828 - If we have the Glasses Order license, load the Is Contact Lens setting, unless we have Frames Data
		// (s.dhole 2012-03-14 14:15) - PLID 48888
		if (g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)) {
			if(m_CheckIsFrameData.GetCheck()) {
				GetDlgItem(IDC_IS_CONTACT_LENS)->ShowWindow(SW_HIDE);
				// (s.dhole 2012-03-14 14:15) - PLID 48888
				GetDlgItem(IDC_VIEW_CONTACT_INV_DATA)->ShowWindow( SW_HIDE );
				// we should uncheck contact now
				CheckDlgButton(IDC_IS_CONTACT_LENS, BST_UNCHECKED);
			}
			else {
				GetDlgItem(IDC_IS_CONTACT_LENS)->ShowWindow(SW_SHOWNA);
				if(AdoFldBool(rs, "IsContactLens")) {
					CheckDlgButton(IDC_IS_CONTACT_LENS, BST_CHECKED);
					GetDlgItem(IDC_VIEW_CONTACT_INV_DATA)->ShowWindow( SW_SHOW);
					GetDlgItem(IDC_CHECK_IS_FRAME_DATA)->ShowWindow(SW_HIDE);
					GetDlgItem(IDC_BUTTON_PRINT_BC)->ShowWindow(SW_HIDE);
				}
				else {
					
					CheckDlgButton(IDC_IS_CONTACT_LENS, BST_UNCHECKED);
					GetDlgItem(IDC_VIEW_CONTACT_INV_DATA)->ShowWindow(AdoFldBool(rs, "IsContactLens") ? SW_SHOWNA : SW_HIDE);
				}
				
				
			}
		}
		else {
			ShowBox(IDC_IS_CONTACT_LENS, FALSE);
		}
		// (a.walling 2007-04-24 14:03) - PLID 25356
		// (a.wetta 2007-05-16 10:55) - PLID 25960 - This button has been removed
		//GetDlgItem(IDC_BTN_INV_SUGGESTED_SALES)->EnableWindow(m_billable.GetCheck() ? TRUE : FALSE);

		if(IsSurgeryCenter(false)) {
			m_checkEquipment.SetCheck(bIsEquipment);
			ShowBox(IDC_BTN_EQUIPMENT_MAINTENANCE, bIsEquipment);
		}
		else {
			if(bIsEquipment) {
				//DRT 5/27/2004 - PLID 12570 - This item was previously equipment, but
				//	they got rid of ASC for some reason.  Reset it to a normal item type.
				MsgBox("This item is marked as 'Equipment', however, you no longer have access to the Surgery Center module.  "
					"This item will be reset to a non-equipment status.");
				ExecuteSql("UPDATE ProductT SET IsEquipment = 0 WHERE ID = %li", GetItem());
				bIsEquipment = FALSE;
			}
		}

		// (j.jones 2008-06-10 11:28) - PLID 27665 - added m_checkFacilityFee
		m_checkFacilityFee.SetCheck(AdoFldBool(rs, "FacilityFee"));

		// (j.jones 2016-04-07 10:22) - NX-100075 - added ability to remember the last charge provider
		m_checkRememberChargeProvider.SetCheck(AdoFldBool(rs, "RememberChargeProvider"));

		m_DefaultProviderCombo->SetSelByColumn(0,rs->Fields->Item["ProviderID"]->Value);

		int TrackableStatus = rs->Fields->Item["TrackableStatus"]->Value.lVal;

		switch(TrackableStatus) {
		case 0:
			m_radioNotTrackable.SetCheck(TRUE);
			m_radioTrackOrders.SetCheck(FALSE);
			m_radioTrackQuantity.SetCheck(FALSE);
			m_eTrackStatus = ENotTrackable;
			break;
		case 1:
			m_radioNotTrackable.SetCheck(FALSE);
			m_radioTrackOrders.SetCheck(TRUE);
			m_radioTrackQuantity.SetCheck(FALSE);
			m_eTrackStatus = ETrackOrders;
			break;
		case 2:
			m_radioNotTrackable.SetCheck(FALSE);
			m_radioTrackOrders.SetCheck(FALSE);
			m_radioTrackQuantity.SetCheck(TRUE);
			m_eTrackStatus = ETrackQuantity;
			break;
		}

		OnBillable();
		OnTaxable();
		OnTaxable2();
		OnTrackable();
		OnEquipment();

		//(e.lally 2007-03-06) PLID 25054- Need to refresh our location arrows. Ideally, this would be included
			//in a function for location specific information that get called from the refresh and other places.
		UpdateLocationArrows();

		//UB92 Categories
		int iRevCodeUse = VarLong(rs->Fields->GetItem("RevCodeUse")->Value,0);
		if(iRevCodeUse == 1) {
			//single
			m_checkSingleRevCode.SetCheck(TRUE);
			m_checkMultipleRevCodes.SetCheck(FALSE);
			m_UB92_Category->Enabled = TRUE;
			m_btnEditMultipleRevCodes.EnableWindow(FALSE);
		}
		else if(iRevCodeUse == 2) {
			//multiple
			m_checkSingleRevCode.SetCheck(FALSE);
			m_checkMultipleRevCodes.SetCheck(TRUE);
			m_UB92_Category->Enabled = FALSE;
			if (GetCurrentUserPermissions(bioInvItem) & (SPT___W________ANDPASS))
				m_btnEditMultipleRevCodes.EnableWindow(TRUE);
		}
		else {
			//none
			m_checkSingleRevCode.SetCheck(FALSE);
			m_checkMultipleRevCodes.SetCheck(FALSE);
			m_UB92_Category->Enabled = FALSE;
			m_btnEditMultipleRevCodes.EnableWindow(FALSE);
		}

		m_UB92_Category->SetSelByColumn(0, rs->Fields->Item["UB92Category"]->Value);

		//Unit Of Order / Unit Of Usage utilities
		m_checkUseUU.SetCheck(AdoFldBool(rs, "UseUU",FALSE));
		OnCheckUseUu();
		SetBox(IDC_UO_EDIT, "UnitOfOrder");
		SetBox(IDC_UU_EDIT, "UnitOfUsage");		
		SetBox(IDC_CONVERSION_EDIT,"Conversion");

		//DRT 11/8/2007 - PLID 28042
		m_btnDefaultConsignment.SetCheck(AdoFldBool(rs, "DefaultConsignment") == 0 ? 0 : 1);

		// (c.haag 2008-02-08 09:41) - PLID 28852 - Split into consignment and
		// purchased inventory tallies.
		const double dActualConsignmentUO = AdoFldDouble(rs, "ActualConsignmentUO",0.0);
		const double dAvailConsignmentUO = AdoFldDouble(rs, "AvailConsignmentUO",0.0);

		const double dActualPurchUO = AdoFldDouble(rs, "ActualPurchasedInvUO",0.0);
		const double dAvailPurchUO = AdoFldDouble(rs, "AvailPurchasedInvUO",0.0);

		//if(IsSurgeryCenter(false)) {

			//if the available stock is not the same as the actual stock,
			//change the data that is shown

			SetDlgItemVar(IDC_ACTUAL_UO,dActualPurchUO,true,true);
			SetDlgItemVar(IDC_ACTUAL_UO_CONSIGNMENT,dActualConsignmentUO,true,true);
			//DRT 2/8/2008 - PLID 28876
			SetDlgItemVar(IDC_AVAIL_UO, dAvailPurchUO, true, true);
			SetDlgItemVar(IDC_AVAIL_UO_CONSIGNMENT, dAvailConsignmentUO, true, true);
		/*}
		//DRT 2/8/2008 - PLID 28876 - Now open to all users
		else {
			SetDlgItemVar(IDC_ACTUAL_UO,dActualPurchUO,true,true);
			SetDlgItemVar(IDC_ACTUAL_UO_CONSIGNMENT,dActualConsignmentUO,true,true);
			//DRT 2/8/2008 - PLID 28876
			SetDlgItemVar(IDC_AVAIL_UO, dAvailPurchUO, true, true);
			SetDlgItemVar(IDC_AVAIL_UO_CONSIGNMENT, dAvailConsignmentUO, true, true);
		}*/

		// (c.haag 2008-02-08 10:03) - PLID 28852 - Split the order tallies into
		// purchased inv. and consignment
		const double dConsignmentOrderUO = AdoFldDouble(rs, "OrderedConsignmentUO",0.0);
		const double dPurchOrderUO = AdoFldDouble(rs, "OrderedPurchasedInvUO",0.0);
		SetDlgItemVar(IDC_ORDER_UO,dPurchOrderUO,true,true);
		SetDlgItemVar(IDC_ORDER_UO_CONSIGNMENT,dConsignmentOrderUO,true,true);

		SetBox(IDC_REORDERPOINT_UO,"ReOrderPointUO");
		SetBox(IDC_REORDERPOINT_UO_CONSIGNMENT,"ConsignmentReOrderPointUO");
		SetBox(IDC_REORDERQUANTITY_UO,"ReOrderQuantityUO");
		//SetBox(IDC_REORDERQUANTITY_UO_CONSIGNMENT,"ConsignmentReorderQuantityUO");

		if(IsSurgeryCenter(false)) {
			//if surgery center, load the following:

			//multi-supplier utils
			CString temp;
			temp.Format("ProductT.ID = %li", GetItem());
			m_Supplier_List->WhereClause = _bstr_t(temp);
			
			RefreshSuppliers();

			if ((GetCurrentUserPermissions(bioInvItem) & (SPT___W________ANDPASS)))
			{
				GetDlgItem(IDC_ADD_SUPPLIER)->EnableWindow();
				GetDlgItem(IDC_MAKE_DEFAULT)->EnableWindow();
			}
		}
		else {
			//if not surgery center, load the following:

			//legacy single-supplier and catalog info
			long nSupplier = AdoFldLong(rs, "SupplierID", -1);
			if(nSupplier != -1) {

				// (j.jones 2010-01-29 09:18) - PLID 36705 - enable the catalog edit
				GetDlgItem(IDC_CATALOG_EDIT)->EnableWindow(TRUE);
			
				if(m_Supplier_Combo->SetSelByColumn(0, nSupplier) == -1) {				
					// (z.manning, 05/27/2008) - PLID 30166 - Need to handle cases where this product's supplier
					// is inactive.
					_RecordsetPtr prsSupplier = CreateParamRecordset("SELECT Company FROM PersonT WHERE ID = {INT} ", nSupplier);
					if(prsSupplier->eof) {
						// (z.manning, 05/27/2008) - This product is associated with a non-existent provider.
						ASSERT(FALSE);

						// (j.jones 2010-01-29 09:18) - PLID 36705 - disable the catalog edit
						GetDlgItem(IDC_CATALOG_EDIT)->EnableWindow(FALSE);
					}
					else {
						m_Supplier_Combo->PutComboBoxText(_bstr_t(AdoFldString(prsSupplier, "Company", "")));
					}					
				}
			}
			else {
				// (j.jones 2010-01-29 09:18) - PLID 36705 - disable the catalog edit
				GetDlgItem(IDC_CATALOG_EDIT)->EnableWindow(FALSE);
				//(e.lally 2010-08-31) PLID 40020 - Since we were blanking out the combo box text, I am assuming we meant
				//to update the cursel to no supplier too.
				m_Supplier_Combo->SetSelByColumn(0, (long)-1);
				m_Supplier_Combo->PutComboBoxText("");
			}
			SetBox(IDC_CATALOG_EDIT, "Catalog");
		}

		//serialized / expirable products
		m_checkHasSerial.SetCheck(AdoFldBool(rs, "HasSerialNum",FALSE));
		//TES 7/3/2008 - PLID 24726 - Added
		m_nxbSerialAsLot.SetCheck(AdoFldBool(rs, "SerialNumIsLotNum",FALSE));
		//TES 7/3/2008 - PLID 24726 - Enable based on HasSerialNum
		m_nxbSerialAsLot.EnableWindow(IsDlgButtonChecked(IDC_CHECK_SERIAL_NUM));
		m_checkHasExpDate.SetCheck(AdoFldBool(rs, "HasExpDate",FALSE));
		m_checkSerializedPerUO.SetCheck(AdoFldBool(rs, "SerialPerUO",FALSE));

		// (a.wilson 2014-5-5) PLID 61831 - remove the old ServiceT.SendOrderingPhy field code.

		ShowSerializedPerUOCheck();		

		//TES 6/16/2008 - PLID 27973 - Check whether the amount on hand is different from the amount on hand when checked
		// with the standard calculation.
		BOOL bCanReassignLocations = FALSE;
		if(AdoFldDouble(rs, "Actual") != AdoFldDouble(rs, "Actual_StandardCalc")) {
			//TES 6/16/2008 - PLID 27973 - This should be a very rare circumstance, it's only possible if they have bad data.
			// If we do get here, determine whether the Actual and Actual_StandardCalc are the same for all locations.  If they
			// are, then the Reassign Item Locations button can be used to fix this case.
			// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
			_RecordsetPtr rsAllLocs = CreateParamRecordset(InvUtils::GetInventoryItemSql(GetItem(), -1));
			if(AdoFldDouble(rsAllLocs, "Actual") == AdoFldDouble(rsAllLocs, "Actual_StandardCalc")) {
				bCanReassignLocations = TRUE;
			}
		}

		ShowItemsBtn(bCanReassignLocations);


		// (j.jones 2015-03-03 09:10) - PLID 64964 - load all the category names
		CString strCategoryNames;
		std::vector<long> aryCategoryIDs;
		long nDefaultCategoryID = -1;
		LoadServiceCategories(GetItem(), aryCategoryIDs, strCategoryNames, nDefaultCategoryID);
		SetDlgItemText(IDC_CATEGORYBOX, strCategoryNames);

		LoadUserInfo();

		//(c.copits 2010-11-02) PLID 38598 - Warranty tracking system
		UpdateWarrantyControls();

		//(e.lally 2007-06-04) PLID 13338 - This really shouldn't be needed if everything is functioning
			//correctly, but let's go ahead and disable controls if we don't have security access
		SecureControls();

		// (z.manning 2010-07-28 16:54) - PLID 39347 - Do this last because other parts of this function depend on
		// getting the ID from this combo and if we requery it that value will be temporarily unavailable.
		if(bProduct) // (b.eyers 2015-07-10) - PLID 24060 - this was set already so don't need to call this check twice
		{
			// (z.manning 2010-07-16 14:46) - PLID 39347 - If products changed then we need to reload the product list
			
			long nItemID = GetItem();
			SelChosenCategoryList(m_category_list->GetCurSel());
			if(nItemID != -1) {
				m_item->TrySetSelByColumn(0, nItemID);
			}
		}
	}
	NxCatchAll("Could not load item info");
	m_refreshing = false;
}

static bool g_bSavingBarcode = false;

void CInvEditDlg::Save(int nID)
{
	try {

		CString table, field, value, original;
		long AuditPriority = aepMedium;
		long AuditItem;

		GetDlgItemText(nID, value);
		original = value;

		switch (nID)
		{	case IDC_NAME: {
				field = "Name";
				value.TrimRight();
				if(value.GetLength() == 0) {
					AfxMessageBox("You cannot change the product's name to be blank. The name will be reset.");
					_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ServiceT WHERE ID = %li",GetItem());
					if(!rs->eof) {
						SetDlgItemText(IDC_NAME,AdoFldString(rs, "Name",""));
					}
					rs->Close();
					return;
				}

				if(value.GetLength() > 255) {
					value = value.Left(255);
					AfxMessageBox("Your description is longer then the maximum amount (255) and has been shortened.\n"
					"Please double-check the description and make changes as needed.");
				}

				_RecordsetPtr rs2 = CreateRecordset("SELECT COUNT (Name) AS Num "
					"FROM ServiceT WHERE Name = '%s' AND ID <> %li",_Q(value), GetItem());
				if (rs2->Fields->Item["num"]->Value.lVal)
				{	rs2->Close();
					// (j.politis 2015-07-08 14:48) - PLID 66481 - You can't enter a product name that is also a service code name.
					AfxMessageBox ("A service code or product already exists with this name. (It may be marked inactive.)");
					_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ServiceT WHERE ID = %li",GetItem());
					if(!rs->eof) {
						SetDlgItemText(IDC_NAME,AdoFldString(rs, "Name",""));
					}
					rs->Close();
					return;
				}
				rs2->Close();

				AuditItem = aeiProductServiceName;
				AuditPriority = aepHigh;
				}
				break;
			case IDC_UNITDESC:
				field = "UnitDesc";
				if (value.GetLength() > 50) {
					value = value.Left(50);
					AfxMessageBox("Your description is longer then the maximum amount (50) and has been shortened.\n"
					"Please double-check the description and make changes as needed.");
					SetDlgItemText(IDC_UNITDESC, value);
				}
				AuditItem = aeiProductUnitDesc;
				AuditPriority = aepLow;
				break;
			case IDC_PRICE: {
				CString strPrice = value;
				BOOL bFailed = FALSE;
				if(!IsValidCurrencyText(strPrice)) {
					AfxMessageBox("You have entered an invalid amount for the sale price of this item.");
					bFailed = TRUE;
				}

				//m.hancock - 9/30/2005 - PLID 17753 - Exception when the Sales Price / Fee field is erased for a product in Inventory
				//When the value was set as "", it was found to be an invalid currency type and would throw a debug assertion failure and an exception.
				//This way, if we check to see if the entry is a valid number, then format it for currency and check it, we can avoid several bugs.
				FormatDlgCurrency(IDC_PRICE);
				GetDlgItemText(IDC_PRICE, strPrice);

				if(ParseCurrencyFromInterface(strPrice) < COleCurrency(0,0)) {
					AfxMessageBox("Practice does not allow a negative amount for a product price.");
					bFailed = TRUE;
				}

				if(bFailed) {
					_RecordsetPtr rs = CreateRecordset("SELECT Price FROM ServiceT WHERE ID = %li",GetItem());
					if(!rs->eof) {
						SetDlgItemText(IDC_PRICE,FormatCurrencyForInterface(AdoFldCurrency(rs, "Price",COleCurrency(0,0))));
					}
					rs->Close();
					return;
				}

				//m.hancock - 9/30/2005 - PLID 17753 - We've already formatted, so no need to do it again
				FormatDlgCurrency(IDC_PRICE);
				
				//m.hancock - 9/30/2005 - PLID 17753 - Changed the passed string to strPrice because passing value would throw exceptions when value was ""
				COleCurrency cyNewPrice = ParseCurrencyFromInterface(strPrice);
				strPrice = FormatCurrencyForSql(cyNewPrice);

				value = "Convert(money,'" + strPrice + "')";
				field = "Price";
				//if we are changing the price, then check against existing surgeries
				if(!IsRecordsetEmpty("SELECT ID FROM ServiceT WHERE ID = %li AND Price <> %s",GetItem(),value)) { 
					// (z.manning 2010-10-27 15:30) - PLID 40619 - I moved most of this logic to its own function
					CParamSqlBatch sqlBatch;
					if(ReturnsRecords("SELECT ID FROM SurgeryDetailsT WHERE ServiceID = %li AND Amount <> %s",GetItem(),value)) {
						InvUtils::UpdateSurgeriesForPriceChange(this, sqlBatch, GetItem(), cyNewPrice);
					}
					if(ReturnsRecords("SELECT ID FROM PreferenceCardDetailsT WHERE ServiceID = %li AND Amount <> Convert(money,'%s')",GetItem(),strPrice)) {
						InvUtils::UpdatePrefCardsForPriceChange(this, sqlBatch, GetItem(), cyNewPrice);
					}
					sqlBatch.Execute(GetRemoteData());
				}
				AuditItem = aeiProductServicePrice;
				AuditPriority = aepHigh;
				}
				break;
			case IDC_REORDERPOINT: {
					if (value == "")
					{	value = "0";
						SetDlgItemInt(nID, 0);
					}
					AuditItem = aeiProductReOrderPoint;

					long qty = GetDlgItemInt(IDC_REORDERPOINT);
					SetDlgItemInt(IDC_REORDERPOINT, qty);

					//audit if the field is different
					_RecordsetPtr rs = CreateRecordset("SELECT ReorderPoint FROM ProductLocationInfoT WHERE ProductID = %li AND LocationID = %li AND ReorderPoint <> %s",GetItem(),m_nLocID,value);
					if(!rs->eof) {
						CString oldValue = AsString(rs->Fields->Item["ReorderPoint"]->Value);
						long AuditID = -1;
						AuditID = BeginNewAuditEvent();
						if(AuditID != -1) {
							// (c.haag 2008-02-18 11:11) - PLID 28866 - Include the location and UU/UO if necessary in the audit description
							CString strOldDesc = FormatString("Location: %s   Value: %s",
								VarString(m_pLocations->GetValue(m_pLocations->CurSel, 1)),
								oldValue);
							CString strNewDesc = original;
							if (m_checkUseUU.GetCheck()) {
								strOldDesc += " (UU)";
								strNewDesc += " (UU)";
							}
							AuditEvent(-1, CString(m_item->GetValue(m_item->CurSel, 1).bstrVal),AuditID,AuditItem,GetItem(),strOldDesc,strNewDesc,AuditPriority);
						}
					}
					rs->Close();

					ExecuteSql("UPDATE ProductLocationInfoT SET ReorderPoint = %s WHERE ProductID = %li AND LocationID = %li",
					value, GetItem(), m_nLocID);

					if(m_checkUseUU.GetCheck()) {
						long nConv = GetDlgItemInt(IDC_CONVERSION_EDIT);
						double dblQtyUO = double(qty) / (nConv == 0 ? 1 : nConv);
						CString strQtyUO;
						strQtyUO.Format("%g",dblQtyUO);

						SetDlgItemText(IDC_REORDERPOINT_UO, strQtyUO);
					}

					// (c.haag 2008-02-29 09:17) - PLID 29157 - Now that the reorder point has changed,
					// we need to update the inventory todo alarms
					// (j.jones 2008-09-16 09:28) - PLID 31380 - EnsureInventoryTodoAlarms now supports multiple products,
					// though in this particular case, it really is only one product
					CArray<long, long> aryProductIDs;
					aryProductIDs.Add(GetItem());
					//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
					InvUtils::EnsureInventoryTodoAlarms(aryProductIDs, m_nLocID, false);
					return;
				}
				break;
			// (c.haag 2008-02-08 11:27) - PLID 28852 - Consignment reorder points. Auditing
			// related code is PL 28866.
			case IDC_REORDERPOINT_CONSIGNMENT: {
					if (value == "")
					{	value = "0";
						SetDlgItemInt(nID, 0);
					}

					// (c.haag 2008-02-08 16:02) - PLID 28866 - Audit if the field changed. Also, since 
					// we have to pull it from data anyway, don't update the data if value isn't changing
					CString strNewValue = value;
					CString strOldValue;
					
					_RecordsetPtr rs = CreateParamRecordset("SELECT ConsignmentReorderPoint FROM ProductLocationInfoT WHERE ProductID = {INT} AND LocationID = {INT}", GetItem(), m_nLocID);
					if(!rs->eof) {

						// If we get here, we found the original Consignment Reorder Point value
						strOldValue = AsString(rs->Fields->Item["ConsignmentReorderPoint"]->Value);
						rs->Close();

						// Compare the two values
						if (atol(strOldValue) != atol(strNewValue)) {

							// If we get here, they're different. Update the data.
							ExecuteSql("UPDATE ProductLocationInfoT SET ConsignmentReorderPoint = %s WHERE ProductID = %li AND LocationID = %li",
								strNewValue, GetItem(), m_nLocID);

							long AuditID = BeginNewAuditEvent();
							if(AuditID != -1) {
								CString strOldDesc = FormatString("Location: %s   Value: %s",
									VarString(m_pLocations->GetValue(m_pLocations->CurSel, 1)),
									strOldValue);

								if (m_checkUseUU.GetCheck()) {
									strOldDesc += " (UU)";
									strNewValue += " (UU)";
								}
								AuditEvent(-1, VarString(m_item->GetValue(m_item->CurSel, 1),""),AuditID,aeiConsignmentReorderPoint,GetItem(),strOldDesc,strNewValue,aepMedium);
							}

							// (c.haag 2008-02-29 09:17) - PLID 29157 - Now that the reorder point has changed,
							// we need to update the inventory todo alarms
							// (j.jones 2008-09-16 09:28) - PLID 31380 - EnsureInventoryTodoAlarms now supports multiple products,
							// though in this particular case, it really is only one product
							CArray<long, long> aryProductIDs;
							aryProductIDs.Add(GetItem());
							//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
							InvUtils::EnsureInventoryTodoAlarms(aryProductIDs, m_nLocID, false);

						} else {
							// They match. Do nothing.
						}
					} else {
						
						// If we get here, there are no ProductLocationInfoT records which match this product and location!
						rs->Close();
						ThrowNxException("Attempted to update the Consignment Reorder Point of a non-existent record!");
					}

					if(m_checkUseUU.GetCheck()) {
						long nConv = GetDlgItemInt(IDC_CONVERSION_EDIT);
						double dblQtyUO = double( atol(value) ) / (nConv == 0 ? 1 : nConv);
						CString strQtyUO;
						strQtyUO.Format("%g",dblQtyUO);

						SetDlgItemText(IDC_REORDERPOINT_UO_CONSIGNMENT, strQtyUO);
					}
					return;
				}
				break;
			case IDC_REORDERQUANTITY: {
					field = "ReorderQuantity";
					if (value == "")
					{	value = "0";
						SetDlgItemInt(nID, 0);
					}
					AuditItem = aeiProductReorderQuantity;

					long qty = GetDlgItemInt(IDC_REORDERQUANTITY);
					SetDlgItemInt(IDC_REORDERQUANTITY, qty);

					//audit if the field is different
					_RecordsetPtr rs = CreateRecordset("SELECT ReorderQuantity FROM ProductLocationInfoT WHERE ProductID = %li AND LocationID = %li AND ReorderQuantity <> %s",GetItem(),m_nLocID,value);
					if(!rs->eof) {
						CString oldValue = AsString(rs->Fields->Item["ReorderQuantity"]->Value);
						long AuditID = -1;
						AuditID = BeginNewAuditEvent();
						if(AuditID != -1) {
							// (c.haag 2008-02-18 11:11) - PLID 28866 - Include the location and UU/UO if necessary in the audit description
							CString strOldDesc = FormatString("Location: %s   Value: %s",
								VarString(m_pLocations->GetValue(m_pLocations->CurSel, 1)),
								oldValue);
							CString strNewDesc = original;
							if (m_checkUseUU.GetCheck()) {
								strOldDesc += " (UU)";
								strNewDesc += " (UU)";
							}
							AuditEvent(-1, CString(m_item->GetValue(m_item->CurSel, 1).bstrVal),AuditID,AuditItem,GetItem(),strOldDesc,strNewDesc,AuditPriority);
						}
					}
					rs->Close();

					ExecuteSql("UPDATE ProductLocationInfoT SET ReorderQuantity = %s WHERE ProductID = %li AND LocationID = %li",
					value, GetItem(), m_nLocID);

					if(m_checkUseUU.GetCheck()) {
						long nConv = GetDlgItemInt(IDC_CONVERSION_EDIT);
						double dblQtyUO = double(qty) / (nConv == 0 ? 1 : nConv);
						CString strQtyUO;
						strQtyUO.Format("%g",dblQtyUO);

						SetDlgItemText(IDC_REORDERQUANTITY_UO, strQtyUO);
					}
					return;
				}
				break;

			// (c.haag 2008-02-08 11:38) - PLID 28852 - Consignment reorder quantity. Auditing
			// is done in PLID 28866.
			/*case IDC_REORDERQUANTITY_CONSIGNMENT: {
					if (value == "")
					{	value = "0";
						SetDlgItemInt(nID, 0);
					}

					// (c.haag 2008-02-08 16:14) - PLID 28866 - Audit if the field changed. Also, since 
					// we have to pull it from data anyway, don't update the data if value isn't changing
					CString strNewValue = value;
					CString strOldValue;

					_RecordsetPtr rs = CreateParamRecordset("SELECT ConsignmentReorderQuantity FROM ProductLocationInfoT WHERE ProductID = {INT} AND LocationID = {INT}", GetItem(), m_nLocID);
					if(!rs->eof) {

						// If we get here, we found the original Consignment Reorder Quantity value
						strOldValue = AsString(rs->Fields->Item["ConsignmentReorderQuantity"]->Value);
						rs->Close();

						// Compare the two values
						if (atol(strOldValue) != atol(strNewValue)) {

							// If we get here, they're different. Update the data.
							ExecuteSql("UPDATE ProductLocationInfoT SET ConsignmentReorderQuantity = %s WHERE ProductID = %li AND LocationID = %li",
								strNewValue, GetItem(), m_nLocID);

							long AuditID = BeginNewAuditEvent();
							if(AuditID != -1) {
								CString strOldDesc = FormatString("Location: %s   Value: %s",
									VarString(m_pLocations->GetValue(m_pLocations->CurSel, 1)),
									strOldValue);
								if (GetDlgItemInt(IDC_CONVERSION_EDIT) != 1) {
									strOldDesc += " (UU)";
									strNewValue += " (UU)";
								}
								AuditEvent(VarString(m_item->GetValue(m_item->CurSel, 1),""),AuditID,aeiConsignmentReorderQuantity,GetItem(),strOldDesc,strNewValue,aepMedium);
							}

						} else {
							// They match. Do nothing.
						}
					} else {
						
						// If we get here, there are no records which match this product and location!
						rs->Close();
						ThrowNxException("Attempted to update the Consignment Reorder Quantity of a non-existent record!");
					}


					if(m_checkUseUU.GetCheck()) {
						long nConv = GetDlgItemInt(IDC_CONVERSION_EDIT);
						double dblQtyUO = double( atol(value) ) / (nConv == 0 ? 1 : nConv);
						CString strQtyUO;
						strQtyUO.Format("%g",dblQtyUO);

						SetDlgItemText(IDC_REORDERQUANTITY_UO_CONSIGNMENT, strQtyUO);
					}
					return;
				}
				break;*/

			case IDC_BARCODE:
				{
					if(!g_bSavingBarcode) {
						g_bSavingBarcode = true;
						if (!value.IsEmpty())
						{
							CString strBarCode = "";
							_RecordsetPtr prs = CreateRecordset("SELECT Barcode FROM ServiceT WHERE ID = %li",GetItem());
							if(!prs->eof) {
								strBarCode = AdoFldString(prs, "Barcode","");
							}
							prs->Close();

							//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
							// If the barcode is already in use by an existing inventory item, save it with a warning.
							prs = CreateRecordset("SELECT Name FROM ProductT INNER JOIN ServiceT ON ServiceT.ID = ProductT.ID WHERE Barcode = '%s' AND ProductT.ID <> %d", _Q(value), GetItem());
							if (!prs->eof)
							{
								//MsgBox("The barcode '%s' already exists for the product '%s'", value, AdoFldString(prs, "Name"));
								CString strWarn;
								strWarn.Format("The barcode '%s' already exists for the product '%s'.\n"
									"Would you like to save this barcode anyway?", value, AdoFldString(prs, "Name"));
								if( IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO) ) {
									GetDlgItem(IDC_BARCODE)->SetWindowText(strBarCode);
									GetDlgItem(IDC_BARCODE)->SetFocus();
									g_bSavingBarcode = false;
									return;
								}
							}
							prs->Close();
 
							//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
							// If the barcode is already in use by an existing service code, save it with a warning.									
							prs = CreateRecordset("SELECT Code FROM CPTCodeT INNER JOIN ServiceT ON ServiceT.ID = CPTCodeT.ID WHERE Barcode = '%s'", _Q(value));
							if (!prs->eof)
							{
								//MsgBox("The barcode '%s' already exists for the service code '%s'", value, AdoFldString(prs, "Code"));
								CString strWarn;
								strWarn.Format("The barcode '%s' already exists for the service code '%s'\n"
									"Would you like to save this barcode anyway?", value, AdoFldString(prs, "Code"));
								if( IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO) ) {
									GetDlgItem(IDC_BARCODE)->SetWindowText(strBarCode);
									GetDlgItem(IDC_BARCODE)->SetFocus();
									g_bSavingBarcode = false;
									return;
								}
							}
							prs->Close();
						}
						field = "Barcode";
						AuditItem = aeiProductBarcode;
						g_bSavingBarcode = false;
					}
					else {
						return;
					}
				}
				break;

			case IDC_INS_CODE: {
				field = "InsCode";
				if (value.GetLength() > 20) {
					value = value.Left(20);
					AfxMessageBox("Your insurance code is longer then the maximum amount (20) and has been shortened.\n"
					"Please double-check the insurance code and make changes as needed.");
					SetDlgItemText(IDC_INS_CODE, value);
				}
				AuditItem = aeiProductInsCode;
				AuditPriority = aepLow;

				// (j.jones 2008-05-27 14:08) - PLID 27799 - see if there are charges for this product that have a blank
				// insurance code, and if so, ask the user if they want to apply the new code to the existing charges
				_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ChargesT.ID) AS TotalCharges, "
					"Sum(CASE WHEN BillsT.EntryType = 2 THEN 1 ELSE 0 END) AS NumOnQuotes "
					"FROM ProductT "
					"INNER JOIN ChargesT ON ProductT.ID = ChargesT.ServiceID "
					"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"WHERE ChargesT.ItemCode = '' "
					"AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
					"AND ProductT.ID = {INT}", GetItem());
				if(!rs->eof) {
					long nTotalCharges = AdoFldLong(rs, "TotalCharges", 0);
					long nNumOnQuotes = AdoFldLong(rs, "NumOnQuotes", 0);

					if(nTotalCharges > 0) {
						CString strMessage;
						strMessage.Format("This product has %li charges not currently tracking any insurance code.", nTotalCharges);
						if(nNumOnQuotes > 0) {
							CString strQuote;
							strQuote.Format(" %li of these charges are on quotes.", nNumOnQuotes);
							strMessage += strQuote;
						}

						strMessage += "\nWould you like to update all of these charges to use this new insurance code?";

						if(IDYES == MessageBox(strMessage, "Practice", MB_YESNO|MB_ICONQUESTION)) {

							//update the charges
							ExecuteParamSql("UPDATE ChargesT SET ItemCode = {STRING} WHERE ItemCode = '' AND ServiceID = {INT}", value, GetItem());

							//audit that we mass-updated
							CString strAmtChanged;
							strAmtChanged.Format("Updated %li charges for product '%s' that had no insurance code.", nTotalCharges, VarString(m_item->GetValue(m_item->CurSel, 1), ""));

							long nAuditID = BeginNewAuditEvent();
							AuditEvent(-1, "", nAuditID, aeiUpdateChargedProductInsCode, GetItem(), strAmtChanged, value, aepHigh, aetChanged);
						}
					}
				}
				rs->Close();

				break;
			}

			case IDC_CATALOG_EDIT: {

					//saves in MultiSupplierT regardless of the Surgery Center license

					if(m_Supplier_Combo->GetCurSel() != -1) {
						int supplier = m_Supplier_Combo->GetValue(m_Supplier_Combo->GetCurSel(), 0).lVal;

						//audit if the field is different
						_RecordsetPtr rs = CreateRecordset("SELECT Catalog FROM MultiSupplierT WHERE ProductID = %li AND SupplierID = %li AND Catalog <> '%s'", GetItem(), supplier, _Q(value));
						if(!rs->eof) {
							CString oldValue = AsString(rs->Fields->Item["Catalog"]->Value);
							long AuditID = -1;
							AuditID = BeginNewAuditEvent();
							if(AuditID != -1) {
								AuditEvent(-1, CString(m_item->GetValue(m_item->CurSel, 1).bstrVal),AuditID,aeiProductCatalog,GetItem(),oldValue,original,AuditPriority);
							}
						}
						rs->Close();

						ExecuteSql("UPDATE MultiSupplierT SET Catalog = '%s' WHERE ProductID = %li AND SupplierID = %li",_Q(value),GetItem(),supplier);
						
						// (a.walling 2007-08-06 12:39) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
						m_tcProductChecker.Refresh();
					}					
					return;
				}
				break;
			case IDC_UU_EDIT:
				field = "UnitOfUsage";
				AuditItem = aeiProductUnitOfUsage;
				AuditPriority = aepLow;
				break;
			case IDC_UO_EDIT:
				field = "UnitOfOrder";
				AuditItem = aeiProductUnitOfOrder;
				AuditPriority = aepLow;
				break;
			case IDC_CONVERSION_EDIT:
				field = "Conversion";
				if (value == "")
				{	value = "1";
					SetDlgItemInt(nID, 1);
				}
				AuditItem = aeiProductConversion;
				break;
			case IDC_LAST_COST: {

				CString strNewCost;// = value; //m.hancock - 9/30/2005 - PLID 17753 - We don't need the value just yet since we'll grab it when we format the currency

				COleCurrency cyOldCost = COleCurrency(0,0);

				//m.hancock - 9/30/2005 - PLID 17753 - Exception when the Last Cost field is erased for a product in Inventory
				//I changed the code to reflect the proper handling of the text like the Shop Fee field
				FormatDlgCurrency(IDC_LAST_COST);
				GetDlgItemText(IDC_LAST_COST, strNewCost);

				_RecordsetPtr rs = CreateRecordset("SELECT LastCost FROM ProductT WHERE ID = %li",GetItem());
				if(!rs->eof) {
					cyOldCost = AdoFldCurrency(rs, "LastCost",COleCurrency(0,0));					
				}
				rs->Close();

				if(!IsValidCurrencyText(strNewCost)) {
					AfxMessageBox("You have entered an invalid amount for the last cost of this item.");
					SetDlgItemText(IDC_LAST_COST,FormatCurrencyForInterface(cyOldCost));
					return;
				}

				//m.hancock - 9/30/2005 - PLID 17753 - We've already formatted, so no need to do it again
				//FormatDlgCurrency(IDC_PRICE);

				COleCurrency cyCost = ParseCurrencyFromInterface(strNewCost);
				strNewCost = FormatCurrencyForSql(cyCost);

				value = "Convert(money,'" + strNewCost + "')";
				field = "LastCost";

				//the product may be using UU/UO, meaning the cyCost passed in would be reduced by the conversion rate (for surgeries)
				rs = CreateRecordset("SELECT Conversion FROM ProductT WHERE UseUU = 1 AND ID = %li",GetItem());
				if(!rs->eof) {
					long nConversion = AdoFldLong(rs, "Conversion",1);
					if(nConversion == 0)
						cyCost = COleCurrency(0,0);
					else {
						//reduce by the conversion rate
						cyCost /= nConversion;
						RoundCurrency(cyCost);
					}
				}
				rs->Close();

				//if we are changing the cost, and are ASC, then check against existing preference cards
				// (z.manning, 05/08/2007) - PLID 25929 - We converted strCostToCheck to be per UU, so we need
				// to make sure we compare it against LastCostPerUU and not LastCost (which is per UO).
				// (j.jones 2009-08-26 08:53) - PLID 35124 - this is now for PreferenceCardsT only
				if(IsSurgeryCenter(false) && !IsRecordsetEmpty("SELECT ID FROM ProductT WHERE ID = %li AND LastCostPerUU <> Convert(money, '%s')",GetItem(),FormatCurrencyForSql(cyCost))) { 

					//still need to get the surgery settings, for defaults
					long nUpdateSurgeryPrices = GetRemotePropertyInt("UpdateSurgeryPrices",1,0,"<None>",TRUE);
					long nUpdateSurgeryPriceWhen = GetRemotePropertyInt("UpdateSurgeryPriceWhen",1,0,"<None>",TRUE);
					long nUpdatePreferenceCardPrices = GetRemotePropertyInt("UpdatePreferenceCardPrices",nUpdateSurgeryPrices,0,"<None>",TRUE);
					long nUpdatePreferenceCardPriceWhen = GetRemotePropertyInt("UpdatePreferenceCardPriceWhen",nUpdateSurgeryPriceWhen,0,"<None>",TRUE);

					//3 means do nothing, so skip the check
					if(nUpdatePreferenceCardPrices != 3
						&& !(nUpdatePreferenceCardPrices == 2 && IsRecordsetEmpty("SELECT ID FROM PreferenceCardDetailsT WHERE ServiceID = %li AND ServiceID IN (SELECT ID FROM ProductT WHERE LastCostPerUU = Cost)",GetItem()))
						&& !(nUpdatePreferenceCardPrices == 1 && IsRecordsetEmpty("SELECT ID FROM PreferenceCardDetailsT WHERE ServiceID = %li AND Cost <> Convert(money, '%s')",GetItem(),FormatCurrencyForSql(cyCost)))) {

						//2 means to always update, so don't prompt. 1 means to prompt.
						if(nUpdatePreferenceCardPrices == 2 || (nUpdatePreferenceCardPrices == 1 && IDYES==MessageBox("There are preference cards that use this product but list a different cost for it.\n"
							"Do you wish to update these preference cards to match this new cost?","Practice",MB_ICONQUESTION|MB_YESNO))) {

							if(nUpdatePreferenceCardPriceWhen == 1)
								ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Cost = Convert(money, {STRING}) WHERE ServiceID = {INT}",FormatCurrencyForSql(cyCost),GetItem());
							else {
								ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Cost = Convert(money, {STRING}) WHERE ServiceID = {INT} AND ServiceID IN (SELECT ID FROM ProductT WHERE LastCostPerUU = Cost)",FormatCurrencyForSql(cyCost),GetItem());
							}
						}
					}
				}

				AuditItem = aeiProductLastCost;
				AuditPriority = aepHigh;
				}
				break;
			case IDC_NOTES:
				field = "Notes";
				if (value.GetLength() > 1000)
				{	value = value.Left(1000);
					SetDlgItemText(IDC_NOTES, value);
					AfxMessageBox("The notes you typed in are longer than the maximum value (1000)."
						"\nThese will be truncated. Please check the notes and reword them if necessary.");
						
				}
				AuditItem = aeiProductNotes;
				AuditPriority = aepLow;
				break;
			case IDC_INV_SHOP_FEE: {
				CString strNewCost;
				COleCurrency cyOldCost = COleCurrency(0,0);
				FormatDlgCurrency(IDC_INV_SHOP_FEE);
				GetDlgItemText(IDC_INV_SHOP_FEE, strNewCost);

				// (j.gruber 2012-10-26 13:11) - PLID 53239 - reconfigured this for shop fees per location
				cyOldCost = m_cyShopFee;
				CString strOldCost = "";
				if (m_cyShopFee < COleCurrency(0,0)) {
					strOldCost = "<Multiple>";
				}
				else {
					strOldCost = FormatCurrencyForInterface(cyOldCost);
				}
			
				if(!IsValidCurrencyText(strNewCost)) {
					AfxMessageBox("You have entered an invalid amount for the shop fee of this item.");
					SetDlgItemText(IDC_INV_SHOP_FEE, strOldCost);
					return;
				}

				COleCurrency cyCost = ParseCurrencyFromInterface(strNewCost);
				strNewCost = FormatCurrencyForSql(cyCost);

				//if we have an enabled dialog, it means we only have one shop fee for all locations	
				//there is a slight possibility that someone else updated the data while we already have this code opened, but that is super slight and auding will reflect what happened, so we are OK with it
				CString strOldValue;
				_RecordsetPtr prs = CreateParamRecordset("SELECT top 1 ShopFee FROM ServiceLocationInfoT INNER JOIN LocationsT ON ServiceLocationInfoT.LocationID = LocationsT.ID WHERE ServiceID = {INT} AND LocationsT.Managed = 1 AND LocationsT.Active = 1", GetItem());
				if(!prs->eof) {
					strOldValue = "For All Active, Managed Locations: " + FormatCurrencyForInterface(AdoFldCurrency(prs->Fields,"ShopFee"));
				}

				ExecuteParamSql(" UPDATE ServiceLocationInfoT SET ShopFee = CONVERT(money, {STRING}) "
					" FROM ServiceLocationInfoT INNER JOIN LocationsT ON ServiceLocationInfoT.LocationID = LocationsT.ID "					
					" WHERE ServiceLocationInfoT.ServiceID = {INT} "
					" AND LocationsT.Managed = 1 AND LocationsT.Active = 1 "
					, 	strNewCost, GetItem());				

				//audit the record of this changing
				if(m_cyShopFee != cyCost) {
					long nAuditID = BeginNewAuditEvent();
					
					AuditEvent(-1, CString(m_item->GetValue(m_item->CurSel, 1).bstrVal), nAuditID, aeiShopFee, GetItem(), strOldValue, FormatCurrencyForInterface(cyCost), aepHigh, aetChanged);
					m_cyShopFee =  cyCost;
				}

		
				return;
				}
				break;

			//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
			
			case IDC_CHECK_WARRANTY: {
				// Warranty Checkbox
				field = "WarrantyActive";
				if (m_CheckWarranty.GetCheck()) {
					value = "1";
					original = "0"; // old value
					AuditItem = aeiInvWarrantyActive;
				}
				else {
					value = "0";
					original = "1"; // old value
					AuditItem = aeiInvWarrantyNotActive;
				}
				} // case IDC_CHECK_WARRANTY
				break;

			case IDC_WARRANTY: {
				// Number of warranty days
				// Get old value for either auditing or restoring
				CString strOldWarranty;
				_RecordsetPtr rs = CreateRecordset("SELECT WarrantyDays FROM ProductT WHERE ID = %li", GetItem());
				if(!rs->eof) {
					strOldWarranty = AsString(rs->Fields->Item["WarrantyDays"]->Value);
				}
				rs->Close();
				// Check validity of new value
				int nDays = atoi(value);
				if ((value.IsEmpty()) || (nDays < 1) || (nDays > MAX_WARRANTY_DAYS) || (!IsNumeric(value))) {
					CString strMessage;
					strMessage.Format("A valid warranty period must be within 1 - %d day(s).", MAX_WARRANTY_DAYS);
					AfxMessageBox(strMessage);
					SetDlgItemText(IDC_WARRANTY, strOldWarranty);
					return;
				}
				else {
					// Save values for audit
					//original = strOldWarranty;
					AuditItem = aeiInvWarrantyDays;
					field = "WarrantyDays";
					original = value;
				}
				} // case IDC_WARRANTY
				break;
			
			default:
				return;
		}

		switch (nID)
		{	case IDC_PRICE:
			case IDC_NAME:
			case IDC_BARCODE:
			case IDC_INV_SHOP_FEE:
				table = "ServiceT";
				break;
			default:
				table = "ProductT";
				break;
		}

		//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
		if (nID != IDC_PRICE && nID != IDC_REORDERQUANTITY && nID != IDC_REORDERPOINT && nID != IDC_LAST_COST && nID != IDC_INV_SHOP_FEE
			&& nID != IDC_CHECK_WARRANTY && nID != IDC_WARRANTY)
			value = '\'' + _Q(value) + '\'';

		try	{
			//audit if the field is different
			_RecordsetPtr rs = CreateRecordset("SELECT %s FROM %s WHERE ID = %li AND %s <> %s",field,table,GetItem(),field,value);		
			if(!rs->eof) {
				CString oldValue = AsString(rs->Fields->Item[_bstr_t(field)]->Value);
				//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
				//Save states for check box
				if (nID == IDC_CHECK_WARRANTY) {
					oldValue = original; // old value
					original = value;    // new value
				}
				long AuditID = -1;
				AuditID = BeginNewAuditEvent();
				if(AuditID != -1) {
					AuditEvent(-1, CString(m_item->GetValue(m_item->CurSel, 1).bstrVal),AuditID,AuditItem,GetItem(),oldValue,original,AuditPriority);
				}
			}
			//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
			// If the field is not different for the warranty value, then we must still audit it.
			// (that is, if no value previously existed, we still need to audit)
			else {
				// Warranty duration must now have initial value
				// (the recordset for IDC_WARRANTY might indeed return NULL)
				if ((rs->eof) && (nID == IDC_WARRANTY)) {
					long AuditID = -1;
					AuditID = BeginNewAuditEvent();
					if (AuditID != -1) {
						// Save warranty duration
						CString oldValue = "";
						original = value;
						AuditEvent(-1, CString(m_item->GetValue(m_item->CurSel, 1).bstrVal),AuditID,AuditItem,GetItem(),oldValue,original,AuditPriority);
					}
				}
			}
			rs->Close();

			ExecuteSql("UPDATE %s SET %s = %s WHERE ID = %li",
				table, field, value, GetItem());
			// (a.walling 2007-08-06 12:39) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			m_tcProductChecker.Refresh(GetItem());
		}NxCatchAll("Could not save inventory changes");

		if (nID == IDC_NAME)
			m_item->Value[m_item->CurSel][1] = _bstr_t(original);

	}NxCatchAll("Error preparing to save.");

	// (c.haag 2008-02-08 12:04) - PLID 28852 - Include consignment boxes too
	if(nID == IDC_REORDERPOINT_UO || nID == IDC_REORDERQUANTITY_UO || nID == IDC_CONVERSION_EDIT ||
		nID == IDC_REORDERPOINT_UO_CONSIGNMENT /*|| nID == IDC_REORDERQUANTITY_UO_CONSIGNMENT*/)
		//TODO: don't refresh, just call a function to calculate the changes

		//if this is removed, then the call to Refresh() needs to be commented back in the
		//OnCheckUseUU function
		Refresh();	
}

void CInvEditDlg::OnNewItem() 
{
	try {

		if (UserPermission(NewItem))
		{	
		// (s.dhole 2012-04-30 09:49) - PLID 50053 
		enum {
			miNewProduct = -11,
			miNewFrame = -12,
			miNewContact = -13,
		};
		int nResult = 0;
		// (s.dhole 2012-04-30 09:49) - PLID 50053 Check  Glasses and Frame license 
		if((g_pLicense && g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)) ||
			(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcFrames, CLicense::cflrSilent))) {
				CMenu mnu;
				mnu.m_hMenu = CreatePopupMenu();
				long nIndex = 0;
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, miNewProduct, "New Product");
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, miNewFrame, "New Frame ");
				// (s.dhole 2012-04-30 09:49) - PLID 50053 Contact will available with glasses license
				if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)){
					mnu.InsertMenu(nIndex++, MF_BYPOSITION, miNewContact, "New Contact Lens");
				}
				CRect rc;
				CWnd *pWnd = GetDlgItem(IDC_NEW_ITEM);
				if (pWnd) {
					pWnd->GetWindowRect(&rc);
					nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL);
				} else {
					CPoint pt;
					GetCursorPos(&pt);
					nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
				}
			}
			else{
				nResult = -11;
			}
			CString  strFinalName  ;
			
			if (nResult == miNewProduct){
				CInvNew dlg;
				long nCategory = -1;
				if(m_category_list->GetCurSel() != sriNoRow) {
					_variant_t varCategory = m_category_list->Value[m_category_list->CurSel][0];
					if (varCategory.vt == VT_I4) {
						nCategory = VarLong(varCategory);
					}
					if (nCategory < 0) {
						nCategory = -1;
					}
					nResult = dlg.DoModal(nCategory);
					strFinalName = dlg.m_strFinalName;
				}
			}
			// (s.dhole 2012-03-15 17:50) - PLID 50053 New Frame
			else if (nResult == miNewFrame){
				// Set new Frame to true
				CInvFramesDataDlg dlg(-1,TRUE, TRUE, this);
				nResult = dlg.DoModal();
				if (nResult ==IDOK)				{
					strFinalName =  dlg.m_strFinalProductName;
					nResult = dlg.m_nProductID;
				}
				else{
				nResult =-1;
				}
			}
			// (s.dhole 2012-04-30 09:54) - PLID 50053
			// (s.dhole 2012-04-30 09:54) - PLID 48888 New contact lens
			else if (nResult == miNewContact){
				//(S.DHOLE ) 2012-06-11  plid  48856 Parent
				CInvContactLensDataDlg dlg(this);
				dlg.m_nProductID =-1;
				dlg.m_strFinalName="";
				nResult = dlg.DoModal();
				if (nResult ==IDOK){
					strFinalName =  dlg.m_strFinalName;
					nResult = dlg.m_nProductID;
				}
				else{
					nResult =-1;
				}
			}
			if (nResult >0 )
			{
				//DRT 5/26/2004 - PLID 12593 - This was not waiting on the requery to finish.  However, 
				//	we're just adding a name/id pair to the datalist, and we already have the ID, so 
				//	why bother requerying?  Changed this to just grab the name out and insert the row 
				//	ourself.  We do not need to worry about the category, because when you are filtered
				//	on a category and make a new item, the new item automatically gets that category.
				IRowSettingsPtr pRow = m_item->GetRow(sriNoRow);
				pRow->PutValue(0, (long)nResult);
				pRow->PutValue(1, _bstr_t(strFinalName ));
				int nRow = m_item->AddRow(pRow);
				if(nRow == sriNoRow) {
					//can't find it, just reset to the first item
					MsgBox("Failed to select the new item in the list.  The selection will be reset to the first item.");
					SetCurrentSelection(m_item, 0);
					OnSelChosenItem(m_item->GetCurSel());
				}
				else {
					SetCurrentSelection(m_item, nRow);
				}
				
				// (a.walling 2007-08-06 12:40) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
				m_tcProductChecker.Refresh(nResult);
				if(m_item->CurSel != -1){
					EnableAll(TRUE);
				}
				UpdateView();
				UpdateArrows();
			}
		}

	}NxCatchAll("Error in CInvEditDlg::OnNewItem"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

// (s.dhole 2012-03-13 11:51) - PLID 
void CInvEditDlg::OnDeleteItem() 
{
	try {

		if (!UserPermission(DeleteItem))
			return;

		_RecordsetPtr rs;
		long item = GetItem();

		//(a.wilson 2011-9-22) PLID 33502 - get the next item id for the list.
		long nRowIndex = m_item->GetCurSel();
		if(nRowIndex > -1 && nRowIndex + 1 < m_item->GetRowCount()){
			IRowSettingsPtr pRow = m_item->GetRow(nRowIndex + 1);
			if (pRow) {
				m_nNextItemID = VarLong(pRow->GetValue(0), -1);
			}
		}
		
		EnsureRemoteData();
		// (j.jones 2007-12-06 09:40) - PLID 28136 - ensured that even deleted charges are checked, and updated the warning accordingly
		// (z.manning, 05/21/2008) - PLID 27042 - Need to also check EMN templates
		if(!IsRecordsetEmpty("SELECT TOP 1 ServiceID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE ServiceID = %li", item)
			// (c.haag 2009-10-12 12:56) - PLID 35722 - Check MailSentServiceCptT
			|| !IsRecordsetEmpty("SELECT TOP 1 ID FROM MailSentServiceCptT WHERE ServiceID = %li", item)
			|| !IsRecordsetEmpty("SELECT TOP 1 ServiceID FROM EMRChargesT WHERE ServiceID = %li", item)
			|| !IsRecordsetEmpty("SELECT TOP 1 ServiceID FROM EMRTemplateChargesT WHERE ServiceID = %li", item)
			|| !IsRecordsetEmpty("SELECT TOP 1 ID FROM ProductItemsT INNER JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID WHERE ProductID = %li",item)
			// (j.jones 2007-11-07 10:09) - PLID 27987 - disallow deleting inventory allocations (unless they are themselves deleted)
			|| !IsRecordsetEmpty("SELECT TOP 1 ID FROM PatientInvAllocationDetailsT WHERE ProductID = %li AND Status <> %li", item, InvUtils::iadsDeleted)
			// (j.jones 2009-01-13 14:54) - PLID 26141 - disallow deleting products that are part of inventory reconciliations
			// unless they were not counted at all
			// (j.jones 2009-01-15 10:06) - PLID 32684 - also account for adjustments, though there shouldn't be any if the count is NULL
			|| !IsRecordsetEmpty("SELECT TOP 1 ID FROM InvReconciliationProductsT WHERE ProductID = %li AND "
				"(CountedAmount IS NOT NULL OR InvAdjustmentID IS NOT NULL)", item)
			// (s.dhole 2011-04-01 12:05) - PLID 43101 If frame data use in any Glassess order than you can not delete this product
			// (s.dhole 2012-04-30 15:53) - PLID 49518 we do save all Frame product id into  GlassesOrderT
			||	!IsRecordsetEmpty("Select TOP 1 GlassesOrderT.ID  AS RecCount  FROM   GlassesOrderT INNER JOIN ProductT ON GlassesOrderT.FrameProductID = ProductT.ID  "
			" WHERE ProductT.ID =  %li ", item)
			//TES 4/25/2011 - PLID 41113 - Can't delete items that are in the "Items to Bill" section of a Glasses Order
			|| !IsRecordsetEmpty("SELECT TOP 1 ServiceID FROM GlassesOrderServiceT WHERE ServiceID = %li", item)
			// (j.jones 2009-08-27 10:12) - PLID 35124 - check Preference Cards
			|| !IsRecordsetEmpty("SELECT PreferenceCardID FROM PreferenceCardDetailsT WHERE ServiceID = %li", item)
			//TES 5/25/2011 - PLID 43737 - Disallow deleting if it's on a contact lens order
			//// (s.dhole, 06/18/2012) - PLID 50929  
			//|| !IsRecordsetEmpty("SELECT ProductID FROM ContactLensOrderInfoT WHERE ProductID = %li", item)
			//(c.copits 2011-07-14) PLID 43314 - Error deleting a product tied to a service code.
			|| !IsRecordsetEmpty("SELECT TOP 1 CptID FROM ServiceToProductLinkT WHERE ProductID = %li", item)) {
				// (j.dinatale 2012-04-17 17:26) - PLID 49078 - changed glasses order to optical order
			if(IDYES==MessageBox("You cannot delete an item from inventory if any of the following is true:\n\n"
				"- It is charged to a patient account; even if the charge is deleted\n"
				"- It exists in an EMR record (including EMN templates); even if the record is deleted\n"
				"- It is assigned to a patient photo\n"
				"- It has been counted in a reconciliation\n"
				"- It exists in a Preference Card\n"
				"- It exists in an Optical Order\n" //PLID 43101 If frame data use in any Glassess order than you can not delete this product
				"- It exists in an inventory allocation for a patient\n"
				"- It is linked to a Service\n\n" //(c.copits 2011-07-14) PLID 43314 - Error deleting a product tied to a service code.
				"However, you can mark an item inactive. Would you like to do this now?","Practice",MB_YESNO|MB_ICONEXCLAMATION)) {
				OnMarkItemInactive();
			}
			return;
		}

		CArray<long, long> aryAppointmentIDsToUpdate;

		if (IDYES == MessageBox("This will delete all related item, tracking, order, return and surgery information, Are you sure?", NULL, MB_YESNO)) {

			// (j.jones 2008-03-24 16:11) - PLID 29388 - we need to track the appointment IDs for all orders we may be altering,
			// even if we aren't necessarily deleting them
			_RecordsetPtr rs = CreateParamRecordset(
				"SELECT AppointmentID FROM OrderAppointmentsT "
				"WHERE OrderID IN (SELECT OrderID FROM OrderDetailsT WHERE ProductID = {INT}) "
				"GROUP BY AppointmentID "
				"SELECT FramesDataID FROM ProductT WHERE ProductT.ID = {INT} "
				, item, item);
			while(!rs->eof) {
				long nAppointmentID = AdoFldLong(rs, "AppointmentID", -1);
				if(nAppointmentID != -1) {
					aryAppointmentIDsToUpdate.Add(nAppointmentID);
				}
				rs->MoveNext();
			}

			rs = rs->NextRecordset(NULL);
			long nFramesDataID = AdoFldLong(rs->GetFields(), "FramesDataID", -1);

			rs->Close();

			// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
			CSqlTransaction trans("DeleteInventory");
			trans.Begin();
			// (c.haag 2007-11-13 16:05) - PLID 27994 - Handle returns
			ExecuteSql("DELETE FROM SupplierReturnItemsT WHERE ProductID = %d OR ProductItemID IN (SELECT ID FROM ProductItemsT WHERE ProductID = %d)", item, item);
			ExecuteSql("DELETE FROM SupplierReturnGroupsT WHERE ID NOT IN (SELECT ReturnGroupID FROM SupplierReturnItemsT)");
			// (j.jones 2007-12-11 12:23) - PLID 27988 - DO NOT delete billed allocations - should be impossible
			// for this data to exist and not be caught by the above checks, if so we DO want the FK violation from the
			// allocation deletion
			//ExecuteParamSql("DELETE FROM ChargedAllocationDetailsT WHERE ChargeID IN (SELECT ID FROM LineItemT WHERE Deleted = 1) AND AllocationDetailID IN (SELECT ID FROM PatientInvAllocationDetailsT WHERE ProductID = {INT})");
			// (j.jones 2007-11-07 10:18) - PLID 27987 - handle inventory allocations
			//(can't get here unless the only allocations for this product are deleted)
			ExecuteParamSql("DELETE FROM PatientInvAllocationDetailsT WHERE ProductID = {INT}", item);
			//clean up any empty allocations we may have just created
			ExecuteParamSql("DELETE FROM PatientInvAllocationsT WHERE ID NOT IN (SELECT AllocationID FROM PatientInvAllocationDetailsT)");

			// (j.jones 2009-01-13 15:01) - PLID 26141 - delete from the inv. reconciliation,
			// shouldn't happen unless there is no count for this product
			ExecuteParamSql("DELETE FROM InvReconciliationProductItemsT WHERE InvReconciliationProductID IN (SELECT ID FROM InvReconciliationProductsT WHERE ProductID  = {INT})", item);
			ExecuteParamSql("DELETE FROM InvReconciliationProductsT WHERE ProductID  = {INT}", item);

			// (a.wetta 2007-03-30 10:22) - PLID 24872 - Also delete any rule links associated with the service code
			ExecuteSql("DELETE FROM CommissionRulesLinkT WHERE ServiceID = %li", item);
			ExecuteSql("DELETE FROM CommissionT WHERE ServiceID = %li", item);
			ExecuteSql("DELETE FROM SurgeryDetailsT WHERE ServiceID = %li", item);
			// (c.haag 2007-12-04 09:05) - PLID 28264 - Delete from the status history table
			ExecuteSql("DELETE FROM ProductItemsStatusHistoryT WHERE ProductItemID IN (SELECT ID FROM ProductItemsT WHERE ProductID = %d)", item);
			// (j.jones 2008-06-06 10:00) - PLID 27110 - delete referencing product items first
			ExecuteParamSql("DELETE FROM ProductItemsT WHERE ProductID = {INT} AND ReturnedFrom Is Not Null", item);
			ExecuteSql("DELETE FROM ProductItemsT WHERE ProductID = %li", item);				
			ExecuteSql("DELETE FROM OrderDetailsT WHERE ProductID = %li", item);
			// (j.jones 2008-03-21 09:38) - PLID 29309 - delete from OrderAppointmentsT
			// (j.jones 2008-03-21 09:38) - PLID 29316 - no need to audit the link deletion when the order is deleted
			ExecuteParamSql("DELETE FROM OrderAppointmentsT WHERE OrderID NOT IN (SELECT OrderID FROM OrderDetailsT)");
			// (c.haag 2008-06-11 14:37) - PLID 28474 - Also need to delete order todo alarms
			TodoDelete(FormatString("RegardingType = %d AND RegardingID IN "
				"(SELECT ID FROM OrderT WHERE ID NOT IN (SELECT OrderID FROM OrderDetailsT))", ttInvOrder));
			ExecuteParamSql("DELETE FROM OrderT WHERE ID NOT IN (SELECT OrderID FROM OrderDetailsT)");
			ExecuteSql("DELETE FROM ProductAdjustmentsT WHERE ProductID = %li", item);				
			ExecuteSql("DELETE FROM ProductLocationTransfersT WHERE ProductID = %li", item);
			ExecuteSql("DELETE FROM ProductResponsibilityT WHERE ProductID = %li", item);
			ExecuteSql("DELETE FROM EquipmentMaintenanceT WHERE ProductID = %li", item);
			ExecuteSql("UPDATE ProductT SET DefaultMultiSupplierID = NULL WHERE ID = %li", item);
			ExecuteSql("DELETE FROM MultiSupplierT WHERE ProductID = %li", item);
			ExecuteSql("DELETE FROM ServiceRevCodesT WHERE ServiceID = %li", item);
			ExecuteSql("DELETE FROM ProductLocationInfoT WHERE ProductID = %li", item);
			// (a.walling 2007-04-02 09:57) - PLID 25356 - Delete any suggested sales
			ExecuteSql("DELETE FROM SuggestedSalesT WHERE MasterServiceID = %li", item);
			ExecuteSql("DELETE FROM SuggestedSalesT WHERE ServiceID = %li", item);
			// (a.walling 2007-07-25 16:12) - PLID 15998 - Delete from sales
			ExecuteSql("DELETE FROM SaleItemsT WHERE ServiceID = %li", item);

			// (j.jones 2010-08-03 08:58) - PLID 39912 - clear out InsCoServicePayGroupLinkT
			ExecuteParamSql("DELETE FROM InsCoServicePayGroupLinkT WHERE ServiceID = {INT}", item);

			// (j.jones 2007-10-18 08:30) - PLID 27757 - handle deleting anesth/facility setups, even though
			// they are never linked to products, but they are linked to ServiceT, so do it for posterity
			ExecuteSql("DELETE FROM LocationFacilityFeesT WHERE FacilityFeeSetupID IN (SELECT ID FROM FacilityFeeSetupT WHERE ServiceID = %li)", item);
			ExecuteSql("DELETE FROM FacilityFeeSetupT WHERE ServiceID = %li", item);
			ExecuteSql("DELETE FROM LocationAnesthesiaFeesT WHERE AnesthesiaSetupID IN (SELECT ID FROM AnesthesiaSetupT WHERE ServiceID = %li)", item);
			ExecuteSql("DELETE FROM AnesthesiaSetupT WHERE ServiceID = %li", item);

			// (j.jones 2008-05-02 16:38) - PLID 29519 - delete reward discounts
			ExecuteParamSql("DELETE FROM RewardDiscountsT WHERE ServiceID = {INT}", item);
			
			// (j.gruber 2010-07-20 14:50) - PLID 30481 - delete from the ApptTypeLink
			ExecuteParamSql("DELETE FROM ApptTypeServiceLinkT WHERE ServiceID = {INT} ", item);

			// (j.gruber 2011-05-06 16:38) - PLID 43550 - delete from conversion groups
			ExecuteParamSql("DELETE FROM ApptServiceConvServicesT WHERE ServiceID = {INT} ", item);

			// (j.jones 2013-04-10 11:36) - PLID 56126 - deleted from linked/blocked tables
			// (r.gonet 02/20/2014) - PLID 60778 - Renamed the table to remove the reference to ICD-9
			ExecuteParamSql("DELETE FROM CPTDiagnosisGroupsT WHERE ServiceID = {INT} ", item);
			ExecuteParamSql("DELETE FROM BlockedCPTCodesT WHERE ServiceID1 = {INT} OR ServiceID2 = {INT}", item, item);

			// (j.jones 2013-04-11 16:40) - PLID 12136 - delete from MultiFeeItemsT
			ExecuteParamSql("DELETE FROM MultiFeeItemsT WHERE ServiceID = {INT} ", item);
			
			ExecuteSql("DELETE FROM ProductT WHERE ID = %li", item);

			// (j.gruber 2012-12-04 08:50) - PLID 48566 - delete from ServiceLocationInfoT
			ExecuteParamSql("DELETE FROM ServiceLocationInfoT WHERE ServiceLocationInfoT.ServiceID = {INT}", item);

			// (a.wilson 2014-5-5) PLID 61831 - clear out ChargeLevelProviderConfigT where service was used.
			ExecuteParamSql("DELETE FROM ChargeLevelProviderConfigT WHERE ServiceID = {INT}", item);

			// (j.jones 2015-02-26 10:52) - PLID 65063 - clear ServiceMultiCategoryT
			ExecuteParamSql("DELETE FROM ServiceMultiCategoryT WHERE ServiceID = {INT}", item);

			ExecuteSql("DELETE FROM ServiceT WHERE ServiceT.ID = %li", item);
			// (z.manning 2010-06-24 16:44) - PLID 39311
			if(nFramesDataID != -1) {
				ExecuteParamSql("DELETE FROM FramesDataT WHERE ID = {INT}", nFramesDataID);
			}

			long AuditID = -1;
			AuditID = BeginNewAuditEvent();
			if(AuditID != -1) {
				CString name, newValue;
				GetDlgItemText(IDC_NAME,name);
				AuditEvent(-1, CString(m_item->GetValue(m_item->CurSel, 1).bstrVal),AuditID,aeiProductDeleted,item,name,"<Deleted>",aepHigh,aetDeleted);
			}

			trans.Commit();

			m_item->Requery();
			m_bRequery = false;

			// (c.haag 2008-02-28 10:35) - PLID 29115 - Update inventory todo alarms
			const long nInvTodoTransactionID = InvUtils::BeginInventoryTodoAlarmsTransaction();
			try {
				InvUtils::AddToInventoryTodoAlarmsTransaction(nInvTodoTransactionID, InvUtils::eInvTrans_ProductID, item);
				//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
				InvUtils::CommitInventoryTodoAlarmsTransaction(nInvTodoTransactionID, false);
			}
			NxCatchAllSilentCallThrow(InvUtils::RollbackInventoryTodoAlarmsTransaction(nInvTodoTransactionID));


			// (a.walling 2007-08-06 12:41) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			m_tcProductChecker.Refresh(item);
			// (c.haag 2007-11-14 11:19) - PLID 28094 - Send a table checker to invalidate all orders as more than one order may be
			// tied to the product
			CClient::RefreshTable(NetUtils::OrderT, -1);

			// (j.jones 2008-03-24 15:25) - PLID 29388 - need to update all appointments linked to orders and allocations we affected
			for(int i=0;i<aryAppointmentIDsToUpdate.GetSize();i++) {
				long nAppointmentID = aryAppointmentIDsToUpdate.GetAt(i);
				TrySendAppointmentTablecheckerForInventory(nAppointmentID, FALSE);
			}
			
			//(e.lally) PLID 16607 - check if this was the last item in:
				//the active inventory
				//the current category
				//neither
			//disabling the fields/buttons only if it was the last active item
			CheckEmptyInventory();
			
			//(a.wilson 2011-9-22) PLID 33502 - update later, once requery is finished.
			//UpdateView();
		}
		return;
	} NxCatchAll("Error deleting item.");
}

/*
When deleting or making an item inactive, we need to be able to check if its removal
makes the current list empty. If so, then we need to check if it was the last active inventory item 
to be removed or the last of its category. 
*/
void CInvEditDlg::CheckEmptyInventory()
{
	try {

		//check if the item list is empty
		if(DetectEmptyList(m_item)){
			long nCurrentCategory = m_category_list->CurSel;
			long nCurCategoryID = VarLong(m_category_list->GetValue(nCurrentCategory,0));
			//item list is empty, now check if All categories were selected
			if(nCurCategoryID != sriNoRow){
				//Check if that was the last item in all the categories too
				if(IsRecordsetEmpty("SELECT ServiceT.ID FROM ServiceT "
					"LEFT JOIN CategoriesT ON CategoriesT.ID = ServiceT.Category "
					"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
					"WHERE Active = 1")) {
					//there are no inventory items, disable all fields/buttons and return
						//because there is no need to continue.
					EnableAll(FALSE);
					return;
				}

				//check to see if the current category now has no items
				if(IsRecordsetEmpty("SELECT ServiceT.ID FROM CategoriesT "
					"INNER JOIN ServiceT ON CategoriesT.ID = ServiceT.Category "
					"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
					"WHERE CategoriesT.ID = %li AND Active = 1",nCurCategoryID)) {
					//this category has no active inventory items, remove it from the list
					m_category_list->RemoveRow(nCurrentCategory);
					//select the All Categories entry
					if(SetCurrentSelection(m_category_list, 0))
						OnSelChosenCategoryList(0);
				}	
			
			}//end if All categories not selected
			else{ //last item in list of all categories. No active inventory.
				EnableAll(FALSE);
			}
		}//end if current list is empty

	}NxCatchAll("Error in CInvEditDlg::CheckEmptyInventory"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::RefreshItemsAndCategoriesLists()
{
	try {

		//requery the category list
		m_category_list->Requery();
		m_category_list->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		
		AddDefaultFilters();
		SetCurrentSelection(m_category_list, 0);
		//Simulate the selection which requeries the item list
		SelChosenCategoryList(0);

	}NxCatchAll("Error in CInvEditDlg::RefreshItemsAndCategoriesLists"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnAdjust() 
{
	try {

		CInvAdj dlg(this);

		if(!CheckCurrentUserPermissions(bioInvItem, sptDynamic0))
			return;

		if (m_item->CurSel == -1)
			return;//no inventory items, or serious NxDataList Problem, just don't crash

		if (dlg.DoModal(GetItem(), m_nLocID) == IDOK)
			UpdateView();

	}NxCatchAll("Error in CInvEditDlg::OnAdjust"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnBillable() 
{
	try {

		//(e.lally 2007-06-04) PLID 13338 - Check write permissions before enabling these fields
		if (!(GetCurrentUserPermissions(bioInvItem) & (sptWrite))){
			GetDlgItem(IDC_BILLABLE)->EnableWindow(FALSE);
			if(!m_refreshing){
				//somehow the user got here without write permission, undo this action
				m_billable.SetCheck(m_billable.GetCheck() == TRUE ? FALSE: TRUE);
			}
			return;
		}

		BOOL billable = m_billable.GetCheck();

		GetDlgItem(IDC_PRICE)->EnableWindow(billable);
		GetDlgItem(IDC_PRICE_TEXT)->EnableWindow(billable);
		GetDlgItem(IDC_TAXABLE)->EnableWindow(billable);
		GetDlgItem(IDC_TAXABLE2)->EnableWindow(billable);
		// (a.walling 2007-04-24 14:03) - PLID 25356
		// (a.wetta 2007-05-16 10:55) - PLID 25960 - This button has been removed
		//GetDlgItem(IDC_BTN_INV_SUGGESTED_SALES)->EnableWindow(billable);

		if (GetCurrentUserPermissions(bioInvItem) & (SPT___W________ANDPASS))				
			m_DefaultProviderCombo->Enabled = billable;

		if (!m_refreshing) {
			
			ExecuteSql("UPDATE ProductLocationInfoT SET Billable = %li WHERE ProductID = %li AND LocationID = %li",
				billable, GetItem(), m_nLocID);
			// (a.walling 2007-08-06 12:41) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			m_tcProductChecker.Refresh(GetItem());
		}

	}NxCatchAll("Error in CInvEditDlg::OnBillable"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnTaxable() 
{
	try {

		BOOL taxable = m_taxable.GetCheck();

		if (!m_refreshing) {
			ExecuteSql("UPDATE ServiceT SET Taxable1 = %li WHERE ID = %li",
				taxable, GetItem());
			// (a.walling 2007-08-06 12:41) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			m_tcProductChecker.Refresh(GetItem());
		}

	}NxCatchAll("Could not update taxable 1 status");
}

void CInvEditDlg::OnTaxable2() 
{
	try {

		BOOL taxable = m_Taxable2.GetCheck();

		if (!m_refreshing) {
			ExecuteSql("UPDATE ServiceT SET Taxable2 = %li WHERE ID = %li",
				taxable, GetItem());
			// (a.walling 2007-08-06 12:41) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			m_tcProductChecker.Refresh(GetItem());
		}

	}NxCatchAll("Could not update taxable 2 status");
}

void CInvEditDlg::OnTrackable() 
{
	try {

		long TrackableStatus = 0;
		BOOL bTrackable = FALSE;

		if(m_radioNotTrackable.GetCheck()) {
			TrackableStatus = 0;
			bTrackable = FALSE;
		}
		if(m_radioTrackOrders.GetCheck()) {
			TrackableStatus = 1;
			bTrackable = FALSE;
		}
		if(m_radioTrackQuantity.GetCheck()) {
			TrackableStatus = 2;
			bTrackable = TRUE;
		}

		ShowBox(IDC_ACTUAL,					bTrackable);
		//DRT 2/8/2008 - PLID 28876
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(IsSurgeryCenter(false) || g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			ShowBox(IDC_AVAIL_INV,				bTrackable);
			ShowBox(IDC_AVAIL_TEXT,				bTrackable);
		}
		ShowBox(IDC_ACTUAL_TEXT,			bTrackable);
		ShowBox(IDC_ADJUST,					bTrackable);
		ShowBox(IDC_HISTORY,				bTrackable);
		ShowBox(IDC_REORDERPOINT_TEXT,		bTrackable);
		ShowBox(IDC_REORDERPOINT,			bTrackable);
		ShowBox(IDC_REORDERQUANTITY,		bTrackable);
		ShowBox(IDC_REORDERQUANTITY_TEXT,	bTrackable);
		ShowBox(IDC_ORDER,					bTrackable);
		ShowBox(IDC_ORDER_TEXT,				bTrackable);
		ShowBox(IDC_RESP_LIST,				bTrackable);
		ShowBox(IDC_RESP_TEXT,				bTrackable);
		ShowBox(IDC_TRANSFER_ITEM,			bTrackable);

		//PLID 13360 Don't show the serial number or expiration date boxes for non-tracked items
		ShowBox(IDC_CHECK_SERIAL_NUM,		bTrackable);
		//TES 7/3/2008 - PLID 24726 - Added
		ShowBox(IDC_SERIAL_NUMBER_AS_LOT_NUMBER,	bTrackable);
		ShowBox(IDC_CHECK_EXP_DATE,		bTrackable);
		
		if(IsSurgeryCenter(false))
			ShowBox(IDC_PENDING_CH,				bTrackable);
		m_pLocations->SetSelByColumn(0, m_nLocID);


		BOOL bEnableUUUO = FALSE;
		if(bTrackable && m_checkUseUU.GetCheck()) {
			bEnableUUUO = TRUE;		
		}

		//Quantity setup
		ShowBox(IDC_UO_LABEL, bEnableUUUO);
		ShowBox(IDC_UU_LABEL, bEnableUUUO);
		ShowBox(IDC_ACTUAL_UO, bEnableUUUO);
		//DRT 2/8/2008 - PLID 28876
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(IsSurgeryCenter(false) || g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			ShowBox(IDC_AVAIL_UO, bEnableUUUO);
		}
		ShowBox(IDC_ORDER_UO, bEnableUUUO);
		ShowBox(IDC_REORDERPOINT_UO, bEnableUUUO);
		ShowBox(IDC_REORDERQUANTITY_UO, bEnableUUUO);

		// (c.haag 2008-02-07 17:31) - PLID 28852 - Show consignment total boxes
		ShowConsignmentTotalBoxes();

		if (!m_refreshing) {
			ExecuteSql("UPDATE ProductLocationInfoT SET TrackableStatus = %li WHERE ProductID = %li AND LocationID = %li",
				TrackableStatus, GetItem(), m_nLocID);
			// (a.walling 2007-08-06 12:42) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			m_tcProductChecker.Refresh(GetItem());
		}

	}NxCatchAll("Error in CInvEditDlg::OnTrackable"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnSelChosenRespList(long nRow) 
{
	try {

		if(m_item->CurSel==-1)
			return;

		//(e.lally 2006-09-11) PLID 22457 - It shouldn't be possible to have an invalid user selection now,
		//but if there is, we will return rather than let this error out.
		if(nRow < 0){
			//This shouldn't be possible
			ASSERT(FALSE);
			return;
		}

		variant_t var = m_resp->Value[nRow][0];

		EnsureRemoteData();
		try
		{
			// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
			CSqlTransaction trans("InventoryResponsibility");
			trans.Begin();
			
			ExecuteSql("DELETE FROM ProductResponsibilityT WHERE LocationID = %i AND ProductID = %i",
				m_nLocID, GetItem());
			if (var.vt == VT_I4)
				ExecuteSql("INSERT INTO ProductResponsibilityT (UserID, LocationID, ProductID) "
					"SELECT %i, %i, %i", var.lVal, m_nLocID, GetItem());

			trans.Commit();
			//(e.lally 2007-05-07) PLID 25253 - Warn the user if a responsible user is assigned but no supplier exists
			//Make sure this isn't the No User selection
			if(var.vt == VT_I4 ){
				BOOL bAttemptWarning = FALSE;
				//If they have the surgery center, check the supplier list, otherwise check the supplier combo.
				if(IsSurgeryCenter(false)){
					if(m_Supplier_List->GetRowCount() == 0)
						bAttemptWarning = TRUE;
				}
				else if(m_Supplier_Combo->GetCurSel() == -1)
					bAttemptWarning = TRUE;

				if(bAttemptWarning)
					DontShowMeAgain(this, "This item has been assigned a user responsible for ordering but does not have a supplier selected.", "InvEditDlgRespUserNoSupplier", "Practice");
			}
			return;
		}
		NxCatchAll("Could not save user selected for item");

	}NxCatchAll("Error selecting user from the list");
}

void CInvEditDlg::FormatDlgCurrency(int nID)
{
	try {

		COleCurrency cy;
		CString str;

		GetDlgItemText(nID, str);
		cy = ParseCurrencyFromInterface(str);
		//TES 11/7/2007 - PLID 27979 - VS2008 - VS 2008 doesn't like this syntax, and there's no need for it.
		//if (cy.m_status == COleCurrency::CurrencyStatus::valid)
		if (cy.m_status == COleCurrency::valid)
			str = FormatCurrencyForInterface(cy);
		else str = FormatCurrencyForInterface(COleCurrency(0,0));

		SetDlgItemText(nID, str);

	}NxCatchAll("Error in CInvEditDlg::FormatDlgCurrency"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

BOOL CInvEditDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {

		int nID;

		switch (HIWORD(wParam))
		{	case EN_CHANGE:
				switch (nID = LOWORD(wParam))
				{	case IDC_NAME:
					case IDC_UNITDESC:
					case IDC_BARCODE:
					case IDC_INS_CODE:
					case IDC_PRICE:
					case IDC_LAST_COST:
					case IDC_INV_SHOP_FEE:
					case IDC_CATALOG_EDIT:
					case IDC_REORDERPOINT:
					case IDC_REORDERPOINT_CONSIGNMENT:
					case IDC_REORDERQUANTITY:
					//case IDC_REORDERQUANTITY_CONSIGNMENT:
					case IDC_REORDERPOINT_UO:
					case IDC_REORDERPOINT_UO_CONSIGNMENT:
					case IDC_REORDERQUANTITY_UO:
					//case IDC_REORDERQUANTITY_UO_CONSIGNMENT:
					case IDC_UO_EDIT:
					case IDC_UU_EDIT:
					case IDC_CONVERSION_EDIT:
					case IDC_NOTES:
						if (!m_refreshing)
							m_changed = true;
					default:
						break;
				}
			break;
			case EN_KILLFOCUS:
				switch (nID = LOWORD(wParam))
				{	case IDC_PRICE:
					case IDC_LAST_COST:
					case IDC_INV_SHOP_FEE:
					case IDC_BARCODE:
					case IDC_INS_CODE:
					case IDC_NAME:
					case IDC_UNITDESC:
					case IDC_CATALOG_EDIT:
					case IDC_REORDERPOINT:
					case IDC_REORDERPOINT_CONSIGNMENT:
					case IDC_NOTES:
					case IDC_UO_EDIT:
					case IDC_UU_EDIT:				
						if (m_changed)
							Save(nID);
						m_changed = false;
						break;

					//TES 5/5/2006 - Extra handling for this box.
					case IDC_CONVERSION_EDIT:
						if(m_changed) {
							//This function calls Save().
							HandleChangedConversionEdit();
						}
						m_changed = false;
						break;
					default:
						m_changed = false;
						break;
				}

			break;
		}

	}NxCatchAll("Error in CInvEditDlg::OnCommand"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
	
	return CNxDialog::OnCommand(wParam, lParam);
}

LRESULT CInvEditDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {

		// Don't accept scans if the user does not have write access
		if (!(GetCurrentUserPermissions(bioInvItem) & (SPT___W________ANDPASS)))
			return 0;

		// (v.maida - 2014-08-11 16:00) - PLID 55761 - Check if there's already a barcode present and, if so, verify that the user wants to overwrite the current barcode.
		CString strBarcodeText;
		GetDlgItem(IDC_BARCODE)->GetWindowText(strBarcodeText);
		if (!strBarcodeText.IsEmpty()) {
			if (IDYES == MessageBox("Are you sure you want to replace the current product's barcode with the scanned product's barcode?", "Replace Current Barcode?", MB_YESNO | MB_ICONQUESTION))
			{
				// (a.walling 2007-11-08 17:37) - PLID 27476 - Need to convert this correctly from a bstr
				_bstr_t bstr = (BSTR)lParam;
				GetDlgItem(IDC_BARCODE)->SetWindowText((LPCTSTR)bstr);
				Save(IDC_BARCODE);
				GetDlgItem(IDC_BARCODE)->SetFocus();
				((CNxEdit*)GetDlgItem(IDC_BARCODE))->SetSel(0, -1);
				return 0;
			}
		}
		else { // there is no barcode for the user to replace with this scan, so don't bother warning them
			_bstr_t bstr = (BSTR)lParam;
			GetDlgItem(IDC_BARCODE)->SetWindowText((LPCTSTR)bstr);
			Save(IDC_BARCODE);
			GetDlgItem(IDC_BARCODE)->SetFocus();
			((CNxEdit*)GetDlgItem(IDC_BARCODE))->SetSel(0, -1);
			return 0;
		}

	}NxCatchAll("Error in CInvEditDlg::OnBarcodeScan"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling

	return 0;
}

void CInvEditDlg::OnPrevItem() 
{
	try {

		long curSel = m_item->CurSel;
		if(curSel > 0) curSel--;
		SetCurrentSelection(m_item, curSel);
		OnSelChosenItem(curSel);

	}NxCatchAll("Error in CInvEditDlg::OnPrevItem"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnNextItem() 
{
	try {

		long curSel = m_item->CurSel;
		if(curSel < m_item->GetRowCount()-1) curSel++;
		SetCurrentSelection(m_item, curSel);
		OnSelChosenItem(curSel);

	}NxCatchAll("Error in CInvEditDlg::OnNextItem"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::UpdateArrows(){

	try {

		if(m_item->GetRowCount() == 0){//This _is_ possible, though unlikely
			m_nextItem.EnableWindow(FALSE);
			m_prevItem.EnableWindow(FALSE);
			return;
		}
		if(m_item->CurSel == m_item->GetRowCount()-1){//We're on the last row
			m_nextItem.EnableWindow(FALSE);
		}
		else{//We're not on the last row
			m_nextItem.EnableWindow(TRUE);
		}
		if(m_item->CurSel == 0){//We're on the first row
			m_prevItem.EnableWindow(FALSE);
		}
		else{//We're not on the first row
			m_prevItem.EnableWindow(TRUE);
		}

	}NxCatchAll("Error in CInvEditDlg::UpdateArrows"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnSelChosenCategoryList(long nRow) 
{
	try{
		SelChosenCategoryList(nRow);
		//make sure that the tab shows the same information as the item that is selected in the datalist
		if(!m_refreshing)
			Refresh();
		//(e.lally 2006-08-02) This does not seem necessary since the OnRequeryFinished event for the item
		//list does this already. Right now it is getting called twice.
		//UpdateArrows();
		
	}NxCatchAll("Error in OnSelChosenCategoryList");
}

void CInvEditDlg::SelChosenCategoryList(long nRow)
{
	try{
		//list
		_variant_t value;

		if(m_category_list->CurSel == -1)
			m_category_list->CurSel = 0;

		if(m_category_list->CurSel == -1) {
			m_item->WhereClause = _bstr_t("ServiceT.Active = 1");
			m_item->Requery();
			m_bRequery = false;
			return;
		}

		// (j.jones 2015-03-03 09:23) - PLID 64964 - this now filters on ServiceMultiCategoryT
		value = m_category_list->GetValue(m_category_list->CurSel, 0);
		if(VarLong(value)== dcfAllCategories)
			m_item->WhereClause = _bstr_t("ServiceT.Active = 1");
		else if(VarLong(value) == dcfNoCategory)
			m_item->WhereClause = _bstr_t("ServiceT.Active = 1 AND ServiceT.ID NOT IN (SELECT ServiceID FROM ServiceMultiCategoryT) ");
		else {
			m_item->WhereClause = _bstr_t(FormatString("ServiceT.Active = 1 "
				"AND ServiceT.ID IN (SELECT ServiceID FROM ServiceMultiCategoryT WHERE CategoryID %s)",
				InvUtils::Descendants(VarLong(value))));
		}
		m_item->Requery();
		m_bRequery = false;

		if(m_item->CurSel == -1)
			SetCurrentSelection(m_item, 0);
		
		if (nRow != -1)
			InvUtils::SetDefault(InvUtils::GetDefaultItem(), m_category_list->GetValue(m_category_list->CurSel, 0).lVal);

	}NxCatchAll("Error in SelChosenCategoryList");
}


void CInvEditDlg::OnMarkItemInactive() 
{
	try {

		if (!UserPermission(DeleteItem))
			return;

		_RecordsetPtr rs;
		long item = GetItem();
		if(item == -1)
			return;

		if(MessageBox("Deactivating an inventory item will make it unavailable in the inventory module, as well as a bill or quote. However, you can still view it in reports."
			"\nYou can re-activate this item by clicking 'Inactive Inventory Items' at the bottom of this screen.'"
			"\n\nAre you SURE you wish to deactivate this inventory item?", "Deactivate Inventory Item", MB_YESNO|MB_ICONEXCLAMATION) == IDNO) {
				return;
		}

		if(!IsRecordsetEmpty("SELECT SurgeryID FROM SurgeryDetailsT WHERE ServiceID = %li",item)) {
			MessageBox("This inventory item exists in at least one Surgery. Please remove it from all Surgeries before deactivating.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (j.jones 2009-08-27 10:12) - PLID 35124 - check Preference Cards
		if(!IsRecordsetEmpty("SELECT PreferenceCardID FROM PreferenceCardDetailsT WHERE ServiceID = %li",item)) {
			MessageBox("This inventory item exists in at least one Preference Card. Please remove it from all Preference Cards before deactivating.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (a.walling 2007-07-27 09:00) - PLID 15998 - Check to see if it has a sale applied
		rs = CreateRecordset("SELECT COUNT(*) AS SaleCount FROM SaleItemsT WHERE ServiceID = %li GROUP BY ServiceID", item);
		if (!rs->eof) {
			long nSaleCount = AdoFldLong(rs, "SaleCount", 0);
			if(nSaleCount > 0) {
				CString strMessage;
				strMessage.Format("This inventory item exists in %li sale%s. Marking it as inactive will remove its discount information from all sales. Would you like to continue inactivating this code?", nSaleCount, nSaleCount > 0 ? "s" : "");
				if (IDNO == MessageBox(strMessage,"Practice",MB_YESNO|MB_ICONINFORMATION))
					return;
			}
		}

		// (j.gruber 2010-07-21 11:02) - PLID 30481 - make sure its not linked to any appt types
		rs = CreateRecordset("Select AptTypeT.Name AS Name FROM ApptTypeServiceLinkT LEFT JOIN AptTypeT ON ApptTypeServiceLinkT.AptTypeID = AptTypeT.ID WHERE ApptTypeServiceLinkT.ServiceID = %li", item);
		if(!rs->eof){			
			CString strMessage;
			CString strNames;
			while (!rs->eof) {
				strNames += AdoFldString(rs->GetFields(), "Name", "") + "\r\n";				
				rs->MoveNext();
			}
			strMessage.Format("This Product is linked to the following Appointment Type(s):\r\n%sPlease remove it before deactivating this Product.", strNames);
			MessageBox(strMessage,"Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		//TES 4/20/2011 - PLID 43337 - If this product is on any allocations which have yet to be ordered, don't allow it to be inactivated.
		rs = CreateParamRecordset("SELECT PatientInvAllocationDetailsT.ID "
			"FROM PatientInvAllocationDetailsT INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"INNER JOIN PersonT ON PatientInvAllocationsT.PatientID = PersonT.ID "
			"INNER JOIN ProductLocationInfoT ON PatientInvAllocationsT.LocationID = ProductLocationInfoT.LocationID "
			"	AND PatientInvAllocationDetailsT.ProductID = ProductLocationInfoT.ProductID "
			"AND PatientInvAllocationDetailsT.OrderDetailID Is Null "
			"AND PatientInvAllocationsT.Status = {INT} "
			"AND PersonT.Archived = 0 "
			"AND ProductLocationInfoT.TrackableStatus <> 0 "
			"AND PatientInvAllocationDetailsT.ProductID = {INT}",
			InvUtils::iasActive, item);
		if(!rs->eof) {
			MsgBox("This item is in use on active Patient Allocations.  Please complete the allocations, or remove this product "
				"from them, before marking it as Inactive.");
			return;
		}

		//(e.lally) PLID 16629 - The audit was for the wrong item. It was removing the item first.
		CString strInvItem = VarString(m_item->GetValue(m_item->CurSel, 1));
		// (a.walling 2007-07-27 09:06) - PLID 15998 - Remove sale info
		CString strSql = BeginSqlBatch();
		AddStatementToSqlBatch(strSql, "UPDATE ServiceT SET Active = 0 WHERE ID = %li",item);
		AddStatementToSqlBatch(strSql, "DELETE FROM SaleItemsT WHERE ServiceID = %li",item);
		// (c.haag 2008-04-24 10:04) - PLID 29770 - Delete all inventory todo alarms for this item
		// (c.haag 2008-06-11 11:08) - PLID 30328 - Also factor in TodoAssignToT
		AddStatementToSqlBatch(strSql, "DELETE FROM TodoAssignToT WHERE TaskID IN (SELECT TaskID FROM TodoList WHERE RegardingID = %li AND RegardingType IN (%li,%li))", item, ttPurchasedInvItem, ttConsignmentInvItem);
		AddStatementToSqlBatch(strSql, "DELETE FROM TodoList WHERE RegardingID = %li AND RegardingType IN (%li,%li)", item, ttPurchasedInvItem, ttConsignmentInvItem);
		ExecuteSqlBatch(strSql);
		m_item->RemoveRow(m_item->CurSel);

		long AuditID = -1;
		AuditID = BeginNewAuditEvent();
		if(AuditID != -1) {			
			AuditEvent(-1, strInvItem,AuditID,aeiProductServiceActive,item,"","<Marked Inactive>",aepHigh);
		}

		// (a.walling 2007-08-06 12:42) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		m_tcProductChecker.Refresh(item);

		//(e.lally) PLID 16607 - check if this was the last item in:
			//the active inventory
			//the current category
			//neither
		//disabling the fields/buttons only if it was the last active item
		CheckEmptyInventory();

		UpdateView();
		UpdateArrows();
		return;

	} NxCatchAll("Could not mark item inactive.");	
}

void CInvEditDlg::OnInactiveItems() 
{
	try {

		CInactiveServiceItems dlg(this);
		dlg.m_ServiceType = 2;
		dlg.DoModal();
		
		if(dlg.m_Changed) {
			//check if the list was empty
			if(DetectEmptyList(m_item)){
				EnableAll(TRUE);
			}

			RefreshItemsAndCategoriesLists();

			// (a.walling 2007-08-06 12:43) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			// Multiple can change, just leave it.
			m_tcProductChecker.Refresh();
			UpdateView();
		}

	}NxCatchAll("Error in CInvEditDlg::OnInactiveItems"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}


void CInvEditDlg::ResetTrackingRadios()
{
	try {

		switch (m_eTrackStatus) {
			case ENotTrackable:
				m_radioNotTrackable.SetCheck(TRUE);
				m_radioTrackOrders.SetCheck(FALSE);
				m_radioTrackQuantity.SetCheck(FALSE);
			break;

			case ETrackOrders:
				m_radioNotTrackable.SetCheck(FALSE);
				m_radioTrackOrders.SetCheck(TRUE);
				m_radioTrackQuantity.SetCheck(FALSE);
			break;

			case ETrackQuantity:
				m_radioNotTrackable.SetCheck(FALSE);
				m_radioTrackOrders.SetCheck(FALSE);
				m_radioTrackQuantity.SetCheck(TRUE);
			break;
		}

	}NxCatchAll("Error in CInvEditDlg::ResetTrackingRadios"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}



void CInvEditDlg::OnRadioNotTrackable() 
{
	try {

		//check to see if the serialze items and expiration dates are trackable and if not then 
		//tell them they need to uncheck them first
		if (m_checkHasSerial.GetCheck() || m_checkHasExpDate.GetCheck() ) {
			// (z.manning, 10/06/2006) - PLID 22905 - If the location that they're trying to mark
			// not trackable currently has zero items, then there's no harm in letting them do so.
			long nCurLocCount = InvUtils::GetAvailableProductItemCount(GetItem(), m_nLocID);
			if(nCurLocCount != 0) {
				CString strMsg = FormatString("This product still has %li item(s) at this location that are tracking "
					"serial numbers and/or expiration dates. You must remove these items or assign them to another "
					"location before you can stop tracking their quantity.", nCurLocCount);
				MessageBox(strMsg);
				ResetTrackingRadios();			
				return;
			}
		}

		//otherwise set the new tracking status and then do the normal process
		m_eTrackStatus = ENotTrackable;
		OnTrackable();	

	}NxCatchAll("Error in CInvEditDlg::OnRadioNotTrackable"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnRadioTrackOrders() 
{
	try {

		if (m_checkHasSerial.GetCheck() || m_checkHasExpDate.GetCheck() ) {
			// (z.manning, 10/06/2006) - PLID 22905 - If the location that they're trying to mark
			// not trackable currently has zero items, then there's no harm in letting them do so.
			long nCurLocCount = InvUtils::GetAvailableProductItemCount(GetItem(), m_nLocID);
			if(nCurLocCount != 0) {
				CString strMsg = FormatString("This product still has %li item(s) at this location that are tracking "
					"serial numbers and/or expiration dates. You must remove these items or assign them to another "
					"location before you can stop tracking their quantity.", nCurLocCount);
				MessageBox(strMsg);
				ResetTrackingRadios();			
				return;
			}
		}

		//otherwise set the new tracking status and then do the normal process
		m_eTrackStatus = ETrackOrders;
		OnTrackable();

	}NxCatchAll("Error in CInvEditDlg::OnRadioTrackOrders"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnRadioTrackQuantity() 
{
	try {

		m_eTrackStatus = ETrackQuantity;
		OnTrackable();

	}NxCatchAll("Error in CInvEditDlg::OnRadioTrackQuantity"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnSelChosenUb92Categories(long nRow) 
{
	if(nRow == -1)
		return;

	if (!m_refreshing)
	{	try
		{
			int category = m_UB92_Category->GetValue(nRow, 0).lVal;

			//Check whether they have "<none>" selected
			if(category == -1){
				ExecuteSql("UPDATE ServiceT SET UB92Category = NULL WHERE ID = %li", GetItem());
				SetCurrentSelection(m_UB92_Category, sriNoRow); //We don't want it to show "<none>"
			}
			else{
				ExecuteSql("UPDATE ServiceT SET UB92Category = %li WHERE ID = %li", category, GetItem());
			}
		}
		NxCatchAll("Could not save category.");	
	}	
}

void CInvEditDlg::RefreshSuppliers()
{
	try {

		//this takes care of refreshing the supplier list and selecting the primary supplier
		m_Supplier_List->Requery();

	}NxCatchAll("Error in CInvEditDlg::RefreshSuppliers"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnAddSupplier() 
{
	try {

		CAddSupplierDlg dlg(this);
		dlg.m_ProductID = GetItem();
		if(dlg.DoModal()==IDOK)
			RefreshSuppliers();

		// (a.walling 2007-08-06 12:43) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		m_tcProductChecker.Refresh(GetItem());

	}NxCatchAll("Error in CInvEditDlg::OnAddSupplier"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnMakeDefault() 
{
	try {
	
		if(m_Supplier_List->CurSel == -1)
			return;

		int supplier = m_Supplier_List->GetValue(m_Supplier_List->CurSel, 0).lVal;
		ExecuteSql("UPDATE ProductT SET DefaultMultiSupplierID = %li WHERE ID = %li", supplier, GetItem());

		RefreshSuppliers();

		// (a.walling 2007-08-06 12:43) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		m_tcProductChecker.Refresh(GetItem());
	}
	NxCatchAll("Could not make default supplier.");
}

void CInvEditDlg::OnEditingFinishedSupplierList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(nRow==-1)
		return;

	try {

		if(nCol==3) {
			ExecuteSql("UPDATE MultiSupplierT SET Catalog = '%s' WHERE ID = %li", _Q(CString(varNewValue.bstrVal)), m_Supplier_List->GetValue(nRow,0).lVal);
		}

	}NxCatchAll("Error saving catalog number.");
}

void CInvEditDlg::OnRequeryFinishedSupplierList(short nFlags) 
{
	try {

		//we need to select the row that is the primary
		_RecordsetPtr rs;
		rs = CreateRecordset("SELECT DefaultMultiSupplierID FROM ProductT WHERE ID = %li", GetItem());
		if(!rs->eof && m_Supplier_List->GetRowCount()>0){
			IRowSettingsPtr pRow;
			pRow = m_Supplier_List->GetRow(m_Supplier_List->FindByColumn(0, rs->Fields->Item["DefaultMultiSupplierID"]->Value, 0, false));
			pRow->ForeColor = RGB(255,0,0);
		}
		rs->Close();

	}NxCatchAll("Error in CInvEditDlg::OnRequeryFinishedSupplierList"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnRButtonDownSupplierList(long nRow, short nCol, long x, long y, long nFlags) 
{
	// TODO: Add your control notification handler code here
	
}

void CInvEditDlg::OnDeleteSupplier() 
{
	try {

		if(m_Supplier_List->CurSel == -1)
			return;

		//get the supplier ID
		long supplier = m_Supplier_List->GetValue(m_Supplier_List->CurSel, 0).lVal;
		long item = GetItem();

		// (c.haag 2008-01-30 10:05) - PLID 27994 - Forbid the user from removing a supplier if it's tied
		// to a return for this item.
		if (ReturnsRecords("SELECT 1 FROM SupplierReturnGroupsT "
			"INNER JOIN SupplierReturnItemsT ON SupplierReturnItemsT.ReturnGroupID = SupplierReturnGroupsT.ID "
			"WHERE SupplierReturnGroupsT.SupplierID IN (SELECT SupplierID FROM MultiSupplierT WHERE ID = %d) "
			"   AND (SupplierReturnItemsT.ProductID = %d OR SupplierReturnItemsT.ProductItemID IN (SELECT ID FROM ProductItemsT WHERE ProductID = %d)  )", supplier, item, item))
		{
			MessageBox("This supplier currently has inventory returns associated with it, and cannot be removed from the list.", "Practice", MB_ICONSTOP);
			return;			
		}


		if(IDNO==MessageBox("This will clear out both the selected supplier and catalog information for this item.\nAre you sure you wish to remove this supplier from this item?","Practice",MB_ICONEXCLAMATION|MB_YESNO))
			return;

		CString strOld = CString(m_Supplier_List->GetValue(m_Supplier_List->CurSel, 2).bstrVal);

		//delete the supplier from MultiSupplierT
		ExecuteSql("UPDATE ProductT SET DefaultMultiSupplierID = NULL WHERE DefaultMultiSupplierID = %li", supplier);
		ExecuteSql("DELETE FROM MultiSupplierT WHERE ID = %li", supplier);

		//if there is no primary supplier, select one
		if(!IsRecordsetEmpty("SELECT ID FROM ProductT WHERE DefaultMultiSupplierID Is Null AND ID = %li", item)) {

			_RecordsetPtr rs = CreateRecordset("SELECT Top 1 ID FROM MultiSupplierT WHERE ProductID = %li", item);
			if(!rs->eof) {

				supplier = AdoFldLong(rs, "ID",-1);
				
				if(supplier > 0)
					ExecuteSql("UPDATE ProductT SET DefaultMultiSupplierID = %li WHERE ID = %li", supplier, item);
			}
			else
				ExecuteSql("UPDATE ProductT SET DefaultMultiSupplierID = NULL WHERE ID = %li", item);
		}

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, CString(m_item->GetValue(m_item->CurSel, 1).bstrVal), nAuditID, aeiProductSupplier, VarLong(m_item->GetValue(m_item->CurSel, 0)), strOld, "<Deleted>", aepMedium, aetChanged);

		//(e.lally 2007-05-07) PLID 25253 - Warn the user if a responsible user is assigned but no supplier exists now
		//Make sure this isn't the No User selection
		variant_t varUser;
		varUser.vt = VT_NULL;
		if(m_resp->CurSel != -1)
			varUser = m_resp->Value[m_resp->CurSel][0];
		if(varUser.vt == VT_I4 ){
			BOOL bAttemptWarning = FALSE;
			//They must have the surgery center to have this delete button, check the supplier list
			//Is this the last supplier
			if(m_Supplier_List->GetRowCount() == 1)
				bAttemptWarning = TRUE;

			if(bAttemptWarning)
				DontShowMeAgain(this, "This item has a user responsible for ordering assigned to it, but all suppliers have been removed.", "InvEditDlgLastSupplierWithRespUser", "Practice");

		}

		RefreshSuppliers();

		// (a.walling 2007-08-06 12:43) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		// could have affected several items, so leave it.
		m_tcProductChecker.Refresh();
	}
	NxCatchAll("Could not delete supplier.");
}

void CInvEditDlg::OnCheckUseUu() 
{
	try {

		BOOL bEnabled = FALSE;

		if(m_checkUseUU.GetCheck())
			bEnabled = TRUE;

		//UU/UO setup

		ShowBox(IDC_UO_TEXT, bEnabled);
		ShowBox(IDC_UO_EDIT, bEnabled);
		ShowBox(IDC_UU_TEXT, bEnabled);
		ShowBox(IDC_UU_EDIT, bEnabled);
		ShowBox(IDC_CONVERSION_TEXT, bEnabled);
		ShowBox(IDC_CONVERSION_EDIT, bEnabled);
		ShowBox(IDC_CONVERSION_TEXT2, bEnabled);

		if(!bEnabled)
			ShowBox(IDC_CHECK_SERIALIZED_PER_UO, FALSE);
		else {
			ShowSerializedPerUOCheck();
		}

		//Quantity setup

		BOOL bShowQuantities = FALSE;

		if(m_radioTrackQuantity.GetCheck() && bEnabled)
			bShowQuantities = TRUE;

		ShowBox(IDC_UO_LABEL, bShowQuantities);
		ShowBox(IDC_UU_LABEL, bShowQuantities);
		ShowBox(IDC_ACTUAL_UO, bShowQuantities);
		//DRT 2/8/2008 - PLID 28876
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(IsSurgeryCenter(false) || g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			ShowBox(IDC_AVAIL_UO, bShowQuantities);
		}
		ShowBox(IDC_ORDER_UO, bShowQuantities);
		ShowBox(IDC_REORDERPOINT_UO, bShowQuantities);
		ShowBox(IDC_REORDERQUANTITY_UO, bShowQuantities);

		CRect rc;

		if (m_refreshing) {

			if(bEnabled)
				SetDlgItemText(IDC_LAST_COST_TEXT,"Last Cost (UO)");
			else
				SetDlgItemText(IDC_LAST_COST_TEXT,"Last Cost");

			GetDlgItem(IDC_LAST_COST_TEXT)->GetWindowRect(rc);
			ScreenToClient(rc);
			InvalidateRect(rc);

			if(bEnabled)
				SetDlgItemText(IDC_PRICE_TEXT,"Sales Price/Fee (UU)");
			else
				SetDlgItemText(IDC_PRICE_TEXT,"Sales Price/Fee");

			GetDlgItem(IDC_PRICE_TEXT)->GetWindowRect(rc);
			ScreenToClient(rc);
			InvalidateRect(rc);
		}

		if (!m_refreshing) {

			ExecuteSql("UPDATE ProductT SET UseUU = %li WHERE ID = %li",
				(bEnabled ? 1 : 0), GetItem());
			// (a.walling 2007-08-06 12:44) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			m_tcProductChecker.Refresh(GetItem());
			
			// (j.jones 2006-05-11 09:50) - removed because it will be called in HandleChangedConversionEdit
			//Refresh();

			HandleChangedConversionEdit();
		}

	}NxCatchAll("Error in CInvEditDlg::OnCheckUseUu"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnCheckSerialNum() 
{
	if (!m_refreshing)
	try
	{
		BOOL bIsCheckingSerialNum = m_checkHasSerial.GetCheck();
		m_nxbSerialAsLot.EnableWindow(bIsCheckingSerialNum);
		BOOL bWeCheckedLotNumber = FALSE;

		BOOL bContinue = TRUE;
		BOOL bUpdateView = FALSE;

		if(bIsCheckingSerialNum) {

			//first we must go through each location and ensure they are adjusted up to zero
		
			_RecordsetPtr rs = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1");

			while(!rs->eof) {

				long nLocID = AdoFldLong(rs, "ID");
				CString strLocName = AdoFldString(rs, "Name","");

				double dblAmtOnHand = 0.0;
				double dblAllocated = 0.0;

				// (j.jones 2007-12-18 11:31) - PLID 28037 - CalcAmtOnHand changed to return allocation information,
				// which for now is unused in this function
				if(InvUtils::CalcAmtOnHand(GetItem(), nLocID, dblAmtOnHand, dblAllocated)) {
					
					if(dblAmtOnHand < 0.0) {

						bContinue = FALSE;

						CString str;
						str.Format("You have a negative amount on hand at %s.\n"
								"Having a serial number requires that you cannot have negative items in stock.\n"
								"You must first enter an adjustment to bring the amount on hand to at least zero.\n\n"
								"Would you like to enter an adjustment for %s now?",strLocName,strLocName);

						if(IDYES == MessageBox(str,"Practice",MB_YESNO|MB_ICONEXCLAMATION)) {

							CInvAdj dlg(this);

							if(CheckCurrentUserPermissions(bioInvItem, sptDynamic0)) {
								if(m_item->CurSel != sriNoRow) {
									//make an adjustment, which requires an amount to be at least 0 in stock
									if (IDOK == dlg.DoModal(GetItem(), nLocID)) {
										bUpdateView = TRUE;
										bContinue = TRUE;
									}
									else {
										AfxMessageBox("You must adjust these items so there is not a negative amount in stock, prior to enabling serial number tracking.");
									}
								}
							}
						}

						if(!bContinue) {
							// (j.jones 2010-05-07 08:47) - PLID 36454 - save the current lot number value as well
							if(bWeCheckedLotNumber) {
								CheckDlgButton(IDC_SERIAL_NUMBER_AS_LOT_NUMBER, BST_UNCHECKED);
							}
							ExecuteParamSql("UPDATE ProductT SET HasSerialNum = {INT}, SerialNumIsLotNum = {INT} WHERE ID = {INT}",
								(!bIsCheckingSerialNum ? 1 : 0), IsDlgButtonChecked(IDC_SERIAL_NUMBER_AS_LOT_NUMBER) ? 1 : 0, GetItem());

							m_checkHasSerial.SetCheck(!bIsCheckingSerialNum);
							m_nxbSerialAsLot.EnableWindow(IsDlgButtonChecked(IDC_CHECK_SERIAL_NUM));
							UpdateView();
							return;
						}
					}
				}

				rs->MoveNext();
			}
			rs->Close();

			//now we can set the status in the data
			//TES 2/14/2008 - PLID 29012 - No we can't!!  If we set this here, then any time we call CalcAmtOnHand(),
			// it will calculate by looking at the product items.  But we haven't finished setting up the product items
			// yet, so CalcAmtOnHand() will return incorrect values and we'll end up with bad data.  We need to wait
			// until we've done everything else first.
			/*ExecuteSql("UPDATE ProductT SET HasSerialNum = %li WHERE ID = %li",
				(bIsCheckingSerialNum ? 1 : 0), GetItem());*/

			//now we know each location has a non-negative amount in stock, but now run through again
			//and warn them (one warning) if any location has a non-whole number in stock		

			long nTotalAmtOnHand = 0;

			CString strWarning = "";

			rs = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1");

			while(!rs->eof) {

				long nLocID = AdoFldLong(rs, "ID");
				CString strLocName = AdoFldString(rs, "Name","");

				double dblAmtOnHand = 0.0;
				double dblAllocated = 0.0;

				// (j.jones 2007-12-18 11:31) - PLID 28037 - CalcAmtOnHand changed to return allocation information,
				// which for now is unused in this function
				if(InvUtils::CalcAmtOnHand(GetItem(), nLocID, dblAmtOnHand, dblAllocated)) {
					
					if(dblAmtOnHand > 0.0 && (long)dblAmtOnHand != dblAmtOnHand) {
						if(!strWarning.IsEmpty())
							strWarning += ", ";
						strWarning += strLocName;
					}

					nTotalAmtOnHand += (long)dblAmtOnHand;
				}

				rs->MoveNext();
			}
			rs->Close();

			if(!strWarning.IsEmpty()) {
				strWarning = "WARNING - you have a non-whole number in stock at " + strWarning + ".\n"
						"Having a serial number requires that you only have a whole number of items in stock.\n"
						"You will only be able to assign serial numbers to products in increments of 1.";
				AfxMessageBox(strWarning);
			}

			//now we can prompt the user to enter items (the CProductItemsDlg will determine which amounts to create per location)
		
			if(nTotalAmtOnHand > 0) {

				// (j.jones 2007-11-21 16:40) - PLID 28037 - ensure we account for allocated items,
				// but only used ones, not active ones, for this particular calculation
				_RecordsetPtr rs2 = CreateParamRecordset("SELECT Count(ID) AS CountOfProdItems FROM ProductItemsT WHERE "
					"ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
					"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
					"			    WHERE Status = {INT} "
					"				AND ProductItemID Is Not Null) "
					"AND ProductID = {INT} AND ProductItemsT.Deleted = 0",
					InvUtils::iadsUsed, GetItem());
				long CountOfProdItems = 0;
				if(!rs2->eof) {
					CountOfProdItems = AdoFldLong(rs2, "CountOfProdItems",0);
				}
				rs2->Close();

				long Diff = nTotalAmtOnHand - CountOfProdItems;
				
				//now let's tell the user what we're doing
				int nResult = 0;
				CString str;
				
				if(Diff > 0) {
				
					str.Format("By enabling the ability to track serial numbers, you must first enter in data for the %li non-numbered items you have in stock at all locations.\n"
						"Do you wish to enter in this data now, or leave it blank for the time being?\n\n"
						"By clicking 'Yes', you will be prompted to enter in serial numbers for all your non-numbered items in stock.\n"
						"By clicking 'No', each existing non-numbered item will be tracked as numberless (the serial numbers can be edited by clicking the 'Show Items' button).\n",nTotalAmtOnHand);

					nResult = MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNOCANCEL);
					if(nResult == IDCANCEL)
						bContinue = FALSE;
					else if(nResult == IDNO) {
						CWaitCursor pWait;

						CString strLocation = "NULL";
						// (c.haag 2008-07-01 17:32) - PLID 30594 - This preference has been deprecated.
						/*if(GetRemotePropertyInt("DefaultSerializedItemsToNoLocation",0,0,"<None>",TRUE) == 1) {
							//if we default to no location, this is easy, just create blank records for all products

							for(int i=0;i<Diff;i++) {
								//TES 6/18/2008 - PLID 29578 - Changed OrderID to OrderDetailID.
								ExecuteSql("INSERT INTO ProductItemsT (ID, ProductID, SerialNum, ExpDate, OrderDetailID, LocationID) VALUES (%li, %li, NULL, NULL, NULL, NULL)",
									NewNumber("ProductItemsT","ID"), GetItem());
							}
						}
						else*/ {
							//to use the default locations, we need to determine the amount to create per location
	
							rs = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND Active = 1 AND typeID = 1");

							while(!rs->eof) {

								long nLocID = AdoFldLong(rs, "ID");

								double dblAmtOnHand = 0.0;
								double dblAllocated = 0.0;

								// (j.jones 2007-12-18 11:31) - PLID 28037 - CalcAmtOnHand changed to return allocation information,
								// which for now is unused in this function
								if(InvUtils::CalcAmtOnHand(GetItem(), nLocID, dblAmtOnHand, dblAllocated)) {

									// (j.jones 2007-11-21 16:40) - PLID 28037 - ensure we account for allocated items
									_RecordsetPtr rs2 = CreateParamRecordset("SELECT Count(ID) AS CountOfProdItems FROM ProductItemsT WHERE "
										"ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
										"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
										"			    WHERE (Status = {INT} OR Status = {INT}) "
										"				AND ProductItemID Is Not Null) "
										"AND ProductID = {INT} AND ProductItemsT.Deleted = 0 AND ProductItemsT.LocationID = {INT}",
										InvUtils::iadsActive, InvUtils::iadsUsed, GetItem(), nLocID);
									long LocCountOfProdItems = 0;
									if(!rs2->eof) {
										LocCountOfProdItems = AdoFldLong(rs2, "CountOfProdItems",0);
									}
									rs2->Close();

									long LocDiff = (long)dblAmtOnHand - LocCountOfProdItems;

									//finally, now add the amount of product items per location!
									for(int i=0;i<LocDiff;i++) {
										//TES 6/18/2008 - PLID 29578 - Changed OrderID to OrderDetailID.
										// (b.spivey, February 17, 2012) - PLID 48080 - Removed the ID from this insert statement, as it is now an identity. 
										ExecuteSql("INSERT INTO ProductItemsT (ProductID, SerialNum, ExpDate, OrderDetailID, LocationID) VALUES (%li, NULL, NULL, NULL, %li)",
											GetItem(), nLocID);
									}
								}

								rs->MoveNext();
							}
							rs->Close();
						}								
						AfxMessageBox("For purposes of inventory control, these numberless items will still need to be assigned to bills, but they will not be reported on.");
					}
					else if(nResult == IDYES) {

						// (j.jones 2010-05-07 08:47) - PLID 36454 - if lot number is not enabled, ask if they wish to enable it
						if(!IsDlgButtonChecked(IDC_SERIAL_NUMBER_AS_LOT_NUMBER)) {
							if(IDYES == MsgBox(MB_YESNO, "Do you wish to treat this item's Serial Numbers as Lot Numbers?\n"
								"If a Serial Number is treated as a Lot Number, you will not be warned or "
								"prevented from entering duplicate serial numbers for units of this product.")) {
								//we will save later
								bWeCheckedLotNumber = TRUE;
								CheckDlgButton(IDC_SERIAL_NUMBER_AS_LOT_NUMBER, BST_CHECKED);
							}
						}
						
						// (j.jones 2006-05-11 10:25) - this only adjusts by Unit Of Usage,
						// only Orders can enter information by UO

						// (d.thompson 2009-10-21) - PLID 36015 - Create your own dialog, don't
						//	use the shared one anymore.
						//CProductItemsDlg& dlg = GetMainFrame()->GetProductItemsDlg();
						CProductItemsDlg dlg(this);
							
						//adding new items (-2 for NewItemCount and LocationID will tell the dialog to auto-create the right amount of items
						dlg.m_EntryType = PI_ENTER_DATA;
						dlg.m_NewItemCount = -2;
						dlg.m_bUseSerial = m_checkHasSerial.GetCheck();
						dlg.m_varSerialNumIsLotNum = IsDlgButtonChecked(IDC_SERIAL_NUMBER_AS_LOT_NUMBER) ? g_cvarTrue : g_cvarFalse;
						dlg.m_bUseExpDate = m_checkHasExpDate.GetCheck();
						dlg.m_ProductID = GetItem();
						dlg.m_nLocationID = -2;
						//TES 6/18/2008 - PLID 29578 - Changed OrderID to OrderDetailID.
						dlg.m_nOrderDetailID = -1;
						dlg.m_bDisallowQtyChange = TRUE;
						dlg.m_bAllowQtyGrow = FALSE;
						dlg.m_bIsAdjustment = TRUE;
						dlg.m_bDisallowLocationChange = TRUE; // (c.haag 2008-06-25 12:27) - PLID 28438 - Ensure the location column is read-only; we have no business transferring items here
						dlg.m_bSaveDataEntryQuery = false;	// (j.jones 2009-04-01 09:39) - PLID 33559 - make sure this is set to false, since we are borrowing an existing dialog from mainframe
						dlg.m_strSavedDataEntryQuery = ""; // (j.jones 2009-07-09 17:59) - PLID 34842 - make sure this gets cleared!

						if(IDCANCEL == dlg.DoModal()) {
							bContinue = FALSE;
							AfxMessageBox("Without entering in the serial numbers for existing data, the 'Use Serial Number' setting cannot be changed.");
						}
					}
				}
				else if(Diff == 0) {
					AfxMessageBox("On the following screen, you may begin entering the serial numbers for your tracked items.");
					//use -1 to indicate showing ALL items
					ShowSerializedItems(-1);
				}
			}

			//TES 2/14/2008 - PLID 29012 - NOW we can update the data, since we've filled in the productitems appropriately.
			// (j.jones 2010-05-07 08:47) - PLID 36454 - save the current lot number value as well
			ExecuteParamSql("UPDATE ProductT SET HasSerialNum = {INT}, SerialNumIsLotNum = {INT} WHERE ID = {INT}",
				(bIsCheckingSerialNum ? 1 : 0), IsDlgButtonChecked(IDC_SERIAL_NUMBER_AS_LOT_NUMBER) ? 1 : 0, GetItem());			
		}
		else {
			//we're UNchecking the box

			// (j.jones 2007-11-07 10:38) - PLID 27987 - disallow this if the product has a "product item"
			// in any allocation, even if the allocation is marked deleted - but only if that item has a serial number
			if(ReturnsRecords("SELECT TOP 1 ID FROM PatientInvAllocationDetailsT WHERE ProductID = %li AND ProductItemID IN (SELECT ID FROM ProductItemsT WHERE SerialNum Is Not Null AND SerialNum <> '')", GetItem())) {
				AfxMessageBox("This product has serial numbered items in use on a patient allocation (it may be deleted).\n"
					"The serial number setting cannot be disabled for this product.");

				m_checkHasSerial.SetCheck(!bIsCheckingSerialNum);
				m_nxbSerialAsLot.EnableWindow(IsDlgButtonChecked(IDC_CHECK_SERIAL_NUM));
				return;
			}

			// (j.jones 2009-01-13 17:34) - PLID 26141 - forbid from deleting if referenced in a reconciliation
			if(ReturnsRecords("SELECT TOP 1 ID FROM InvReconciliationProductItemsT WHERE ProductItemID IN (SELECT ID FROM ProductItemsT WHERE SerialNum Is Not Null AND SerialNum <> '' AND ProductID = %li)", GetItem())) {
				AfxMessageBox("This product has serial numbered items that have been referenced in an Inventory Reconciliation.\n"
					"The serial number setting cannot be disabled for this product.");

				m_checkHasSerial.SetCheck(!bIsCheckingSerialNum);
				m_nxbSerialAsLot.EnableWindow(IsDlgButtonChecked(IDC_CHECK_SERIAL_NUM));
				return;
			}

			ExecuteParamSql("UPDATE ProductT SET HasSerialNum = {INT} WHERE ID = {INT}",
				(bIsCheckingSerialNum ? 1 : 0), GetItem());

			// (j.jones 2007-11-21 16:40) - PLID 28037 - ensure we account for allocated items
			_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) AS CountOfProdItems FROM ProductItemsT WHERE "
				"ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
				"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
				"			    WHERE (Status = {INT} OR Status = {INT}) "
				"				AND ProductItemID Is Not Null) "
				"AND ProductID = {INT} AND ProductItemsT.Deleted = 0",
				InvUtils::iadsActive, InvUtils::iadsUsed, GetItem());
			long CountOfProdItems = 0;
			if(!rs->eof) {
				CountOfProdItems = AdoFldLong(rs, "CountOfProdItems",0);
			}
			rs->Close();

			if(CountOfProdItems > 0) {

				CString str;
				str.Format("You have a total of %li serial-numbered items in stock (some may be blank) at all locations.\n"
					"Do you wish to clear this data?\n\n"
					"By clicking 'Yes', the serial-numbered data will be deleted.\n"
					"By clicking 'No', you will not be prompted to enter serial numbers on future orders, \n"
					"but you will be prompted to assign existing ones to future bills until the list is empty.\n"
					"(Blank numbers will still be cleared.)",CountOfProdItems);
				int nResult = MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNOCANCEL);
				if(nResult == IDCANCEL)
					bContinue = FALSE;
				else if(nResult == IDNO) {
					//only clear blank items, and only if HasExpDate is unchecked
					if(!m_checkHasExpDate.GetCheck()) {
						// (j.jones 2007-11-07 10:49) - PLID 27987 - make absolutely sure we do not delete allocated items
						// (c.haag 2007-11-13 16:19) - PLID 27994 - Ensure we do not delete ProductItems associated with returns
						// (c.haag 2007-12-04 09:06) - PLID 28264 - Also delete ProductItemsStatusHistoryT records
						// (b.cardillo 2012-08-20 21:01) - PLID 52232 - Don't look at nulls when checking for not in SupplierReturnItemsT
						ExecuteSql("DELETE FROM ProductItemsStatusHistoryT WHERE ProductItemID IN (SELECT ID FROM ProductItemsT "
							"WHERE ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
							"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
							"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
							"AND ProductID = %li AND SerialNum Is NULL AND ExpDate Is NULL)", GetItem());
						// (j.jones 2008-06-06 10:00) - PLID 27110 - delete referencing product items first
						ExecuteParamSql("DELETE FROM ProductItemsT WHERE ReturnedFrom Is Not Null "
							"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
							"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
							"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
							"AND ProductID = {INT} AND SerialNum Is NULL AND ExpDate Is NULL", GetItem());
						ExecuteSql("DELETE FROM ProductItemsT "
							"WHERE ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
							"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
							"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
							"AND ProductID = %li AND SerialNum Is NULL AND ExpDate Is NULL", GetItem());
					}
				}
				else if(nResult == IDYES) {
					if(IDYES == MessageBox("Are you SURE you wish to permanently delete this data?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
						//first update (because they may have valid exp dates!)
						// (b.cardillo 2012-08-20 21:01) - PLID 52232 - Don't look at nulls when checking for not in SupplierReturnItemsT
						ExecuteSql("UPDATE ProductItemsT SET SerialNum = NULL "
							"WHERE ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
							"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
							"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
							"AND ProductID = %li", GetItem());
						if(!m_checkHasExpDate.GetCheck()) {
							// (j.jones 2007-11-07 10:49) - PLID 27987 - make absolutely sure we do not delete allocated items
							// (c.haag 2007-11-13 16:19) - PLID 27994 - Ensure we do not delete ProductItems associated with returns
							// (c.haag 2007-12-04 09:06) - PLID 28264 - Also delete ProductItemsStatusHistoryT records
							ExecuteSql("DELETE FROM ProductItemsStatusHistoryT WHERE ProductItemID IN (SELECT ID FROM ProductItemsT "
								"WHERE ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
								"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
								"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
								"AND ProductID = %li AND SerialNum Is NULL AND ExpDate Is NULL)", GetItem());
							// (j.jones 2008-06-06 10:00) - PLID 27110 - delete referencing product items first
							ExecuteParamSql("DELETE FROM ProductItemsT WHERE ReturnedFrom Is Not Null "
								"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
								"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
								"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
								"AND ProductID = {INT} AND SerialNum Is NULL AND ExpDate Is NULL", GetItem());
							ExecuteSql("DELETE FROM ProductItemsT "
								"WHERE ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
								"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
								"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
								"AND ProductID = %li AND SerialNum Is NULL AND ExpDate Is NULL", GetItem());
						}
					}
					else {
						//back out
						bContinue = FALSE;
						AfxMessageBox("The 'Has Serial Number' status will be left unchanged.");
					}
				}
			}
		}

		if(!bContinue) {

			// (j.jones 2010-05-07 08:47) - PLID 36454 - save the current lot number value as well
			if(bWeCheckedLotNumber) {
				CheckDlgButton(IDC_SERIAL_NUMBER_AS_LOT_NUMBER, BST_UNCHECKED);
			}
			ExecuteParamSql("UPDATE ProductT SET HasSerialNum = {INT}, SerialNumIsLotNum = {INT} WHERE ID = {INT}",
				(!bIsCheckingSerialNum ? 1 : 0), IsDlgButtonChecked(IDC_SERIAL_NUMBER_AS_LOT_NUMBER) ? 1 : 0, GetItem());
			
			m_checkHasSerial.SetCheck(!bIsCheckingSerialNum);
			m_nxbSerialAsLot.EnableWindow(IsDlgButtonChecked(IDC_CHECK_SERIAL_NUM));
		}

		// (a.walling 2007-08-06 12:44) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		m_tcProductChecker.Refresh(GetItem());

		ShowSerializedPerUOCheck();

		ShowItemsBtn();

		if(bUpdateView)
			UpdateView();

		//TES 7/3/2008 - PLID 24726 - There are two places that return before getting here.  One calls UpdateView(), which 
		// will enable this box appropriately, and the other undoes the user's change, so there's no need for this box 
		// to be updated.
		m_nxbSerialAsLot.EnableWindow(IsDlgButtonChecked(IDC_CHECK_SERIAL_NUM));

	}NxCatchAll("Could not update 'Has Serial Number' status");
}

void CInvEditDlg::OnCheckExpDate() 
{
	if (!m_refreshing)
	try
	{
		BOOL bIsCheckingExpDate = m_checkHasExpDate.GetCheck();		

		BOOL bContinue = TRUE;
		BOOL bUpdateView = FALSE;
		
		if(bIsCheckingExpDate) {

			//DRT 5/8/2006 - PLID 20480 - used after the loop
			long nTotalAmtOnHand = 0;
			CString strWarning = "";

			//first we must go through each location and ensure they are adjusted up to zero

			_RecordsetPtr rs = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1");

			while(!rs->eof) {

				long nLocID = AdoFldLong(rs, "ID");
				CString strLocName = AdoFldString(rs, "Name","");

				double dblAmtOnHand = 0.0;
				double dblAllocated = 0.0;

				// (j.jones 2007-12-18 11:31) - PLID 28037 - CalcAmtOnHand changed to return allocation information,
				// which for now is unused in this function
				if(InvUtils::CalcAmtOnHand(GetItem(), nLocID, dblAmtOnHand, dblAllocated)) {
					
					if(dblAmtOnHand < 0.0) {

						bContinue = FALSE;

						CString str;
						str.Format("You have a negative amount on hand at %s.\n"
								"Having an expiration date requires that you cannot have negative items in stock.\n"
								"You must first enter an adjustment to bring the amount on hand to at least zero.\n\n"
								"Would you like to enter an adjustment for %s now?",strLocName,strLocName);

						if(IDYES == MessageBox(str,"Practice",MB_YESNO|MB_ICONEXCLAMATION)) {

							CInvAdj dlg(this);

							if(CheckCurrentUserPermissions(bioInvItem, sptDynamic0)) {
								if(m_item->CurSel != sriNoRow) {
									//make an adjustment, which requires an amount to be at least 0 in stock
									if (IDOK == dlg.DoModal(GetItem(), nLocID)) {
										bUpdateView = TRUE;
										bContinue = TRUE;
									}
									else {
										AfxMessageBox("You must adjust these items so there is not a negative amount in stock, prior to enabling expiration dates.");
									}
								}
							}
						}

						if(!bContinue) {
							ExecuteSql("UPDATE ProductT SET HasExpDate = %li WHERE ID = %li",
								(!bIsCheckingExpDate ? 1 : 0), GetItem());
							m_checkHasExpDate.SetCheck(!bIsCheckingExpDate);
							UpdateView();
							return;
						}

						//DRT 5/8/2006 - PLID 20480 - Since we joined the 2 loops into one, if an adjustment was made, we need to re-calculate the quantity
						//	at this point in time.
						//TODO:  Can this be calculated with what we know?

						// (j.jones 2007-12-18 11:31) - PLID 28037 - CalcAmtOnHand changed to return allocation information,
						// which for now is unused in this function
						InvUtils::CalcAmtOnHand(GetItem(), nLocID, dblAmtOnHand, dblAllocated);
					}


					//DRT 5/8/2006 - PLID 20480 - This code previously was looping over all active, managed locations to find negatives... then closing the recordset, 
					//	immediately opening another recordset that was 100% identical, and adding a warning.  I joined the 2 together.
					//We no longer have to check the quantity here, because we've just done it above.

					//now we know each location has a non-negative amount in stock, but now run through again
					//and warn them (one warning) if any location has a non-whole number in stock		
					if(dblAmtOnHand > 0.0 && (long)dblAmtOnHand != dblAmtOnHand) {
						if(!strWarning.IsEmpty())
							strWarning += ", ";
						strWarning += strLocName;
					}

					nTotalAmtOnHand += (long)dblAmtOnHand;

				}

				rs->MoveNext();
			}
			rs->Close();

			//now we can set the status in data
			//TES 2/14/2008 - PLID 29012 - No we can't!!  If we set this here, then any time we call CalcAmtOnHand(),
			// it will calculate by looking at the product items.  But we haven't finished setting up the product items
			// yet, so CalcAmtOnHand() will return incorrect values and we'll end up with bad data.  We need to wait
			// until we've done everything else first.
			/*ExecuteSql("UPDATE ProductT SET HasExpDate = %li WHERE ID = %li",
				(bIsCheckingExpDate ? 1 : 0), GetItem());*/

			if(!strWarning.IsEmpty()) {
				strWarning = "WARNING - you have a non-whole number in stock at " + strWarning + ".\n"
						"Having an expiration date requires that you only have a whole number of items in stock.\n"
						"You will only be able to assign expiration dates to products in increments of 1.";
				AfxMessageBox(strWarning);
			}

			//now we can prompt the user to enter items (the CProductItemsDlg dialog will auto-calculate the amounts per location)
			if(nTotalAmtOnHand > 0) {

				// (j.jones 2007-11-21 16:40) - PLID 28037 - ensure we account for allocated items,
				// but only used ones, not active ones, for this particular calculation
				_RecordsetPtr rs2 = CreateParamRecordset("SELECT Count(ID) AS CountOfProdItems FROM ProductItemsT WHERE "
					"ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
					"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
					"			    WHERE Status = {INT} "
					"				AND ProductItemID Is Not Null) "
					"AND ProductID = {INT} AND ProductItemsT.Deleted = 0",
					InvUtils::iadsUsed, GetItem());
				long CountOfProdItems = 0;
				if(!rs2->eof) {
					CountOfProdItems = AdoFldLong(rs2, "CountOfProdItems",0);
				}
				rs2->Close();

				long Diff = nTotalAmtOnHand - CountOfProdItems;

				//now let's tell the user what we're doing
				int nResult = 0;
				CString str;

				if(Diff > 0) {

					str.Format("By enabling the ability to track expiration dates, you must first enter in data for the %li non-dated items you have in stock.\n"
						"Do you wish to enter in this data now, or leave it blank for the time being?\n\n"
						"By clicking 'Yes', you will be prompted to enter in exp. dates for all your non-dated items in stock.\n"
						"By clicking 'No', each existing non-dated item will be tracked as date-less (the exp. dates can be edited by clicking the 'Show Items' button).\n",Diff);

					nResult = MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNOCANCEL);
					if(nResult == IDCANCEL)
						bContinue = FALSE;
					else if(nResult == IDNO) {
						CWaitCursor pWait;

						CString strLocation = "NULL";
						// (c.haag 2008-07-01 17:32) - PLID 30594 - This preference has been deprecated.
						/*if(GetRemotePropertyInt("DefaultSerializedItemsToNoLocation",0,0,"<None>",TRUE) == 1) {
							//if we default to no location, this is easy, just create blank records for all products

							for(int i=0;i<Diff;i++) {
								//TES 6/18/2008 - PLID 29578 - Changed OrderID to OrderDetailID.
								ExecuteSql("INSERT INTO ProductItemsT (ID, ProductID, SerialNum, ExpDate, OrderDetailID, LocationID) VALUES (%li, %li, NULL, NULL, NULL, NULL)",
									NewNumber("ProductItemsT","ID"), GetItem());
							}
						}
						else*/ {
							//to use the default locations, we need to determine the amount to create per location
	
							rs = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1");

							while(!rs->eof) {

								long nLocID = AdoFldLong(rs, "ID");

								double dblAmtOnHand = 0.0;
								double dblAllocated = 0.0;

								// (j.jones 2007-12-18 11:31) - PLID 28037 - CalcAmtOnHand changed to return allocation information,
								// which for now is unused in this function
								if(InvUtils::CalcAmtOnHand(GetItem(), nLocID, dblAmtOnHand, dblAllocated)) {

									// (j.jones 2007-11-21 16:40) - PLID 28037 - ensure we account for allocated items
									_RecordsetPtr rs2 = CreateParamRecordset("SELECT Count(ID) AS CountOfProdItems FROM ProductItemsT WHERE "
										"ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
										"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
										"			    WHERE (Status = {INT} OR Status = {INT}) "
										"				AND ProductItemID Is Not Null) "
										"AND ProductID = {INT} AND ProductItemsT.Deleted = 0 AND ProductItemsT.LocationID = {INT}",
										InvUtils::iadsActive, InvUtils::iadsUsed, GetItem(), nLocID);
									long LocCountOfProdItems = 0;
									if(!rs2->eof) {
										LocCountOfProdItems = AdoFldLong(rs2, "CountOfProdItems",0);
									}
									rs2->Close();

									long LocDiff = (long)dblAmtOnHand - LocCountOfProdItems;

									//finally, now add the amount of product items per location!
									for(int i=0;i<LocDiff;i++) {
										// (b.spivey, February 17, 2012) - PLID 48080 - Removed the ID from this insert statement, as it is now an identity. 
										ExecuteSql("INSERT INTO ProductItemsT (ProductID, SerialNum, ExpDate, OrderDetailID, LocationID) VALUES (%li, NULL, NULL, NULL, %li)",
											GetItem(), nLocID);
									}
								}

								rs->MoveNext();
							}
							rs->Close();
						}
						AfxMessageBox("For purposes of inventory control, these date-less items will still need to be assigned to bills, but they will not be reported on.");
					}
					else if(nResult == IDYES) {

						// (j.jones 2006-05-11 10:25) - this only adjusts by Unit Of Usage,
						// only Orders can enter information by UO

						// (d.thompson 2009-10-21) - PLID 36015 - Create your own dialog, don't
						//	use the shared one anymore.
						//CProductItemsDlg& dlg = GetMainFrame()->GetProductItemsDlg();
						CProductItemsDlg dlg(this);
							
						//adding new items (-2 for NewItemCount and LocationID will tell the dialog to auto-create the right amount of items
						dlg.m_EntryType = PI_ENTER_DATA;
						dlg.m_NewItemCount = -2;
						dlg.m_bUseSerial = m_checkHasSerial.GetCheck();
						dlg.m_varSerialNumIsLotNum = IsDlgButtonChecked(IDC_SERIAL_NUMBER_AS_LOT_NUMBER) ? g_cvarTrue : g_cvarFalse;
						dlg.m_bUseExpDate = m_checkHasExpDate.GetCheck();
						dlg.m_ProductID = GetItem();
						dlg.m_nLocationID = -2;
						//TES 6/18/2008 - PLID 29578 - Changed OrderID to OrderDetailID.
						dlg.m_nOrderDetailID = -1;
						dlg.m_bDisallowQtyChange = TRUE;
						dlg.m_bAllowQtyGrow = FALSE;
						dlg.m_bIsAdjustment = TRUE;
						dlg.m_bDisallowLocationChange = TRUE; // (c.haag 2008-06-25 12:27) - PLID 28438 - Ensure the location column is read-only; we have no business transferring items here
						dlg.m_bSaveDataEntryQuery = false;	// (j.jones 2009-04-01 09:39) - PLID 33559 - make sure this is set to false, since we are borrowing an existing dialog from mainframe
						dlg.m_strSavedDataEntryQuery = ""; // (j.jones 2009-07-09 17:59) - PLID 34842 - make sure this gets cleared!

						if(IDCANCEL == dlg.DoModal()) {
							bContinue = FALSE;
							AfxMessageBox("Without entering in the expiration dates for existing data, the 'Use Exp. Date' setting cannot be changed.");
						}
					}
				}
				else if(Diff == 0) {
					AfxMessageBox("On the following screen, you may begin entering the expiration dates for your tracked items.");
					//use -1 to indicate showing ALL items
					ShowSerializedItems(-1);
				}
			}
			
			//TES 2/14/2008 - PLID 29012 - NOW we can update the data, since we've filled in the productitems appropriately.
			ExecuteSql("UPDATE ProductT SET HasExpDate = %li WHERE ID = %li",
				(bIsCheckingExpDate ? 1 : 0), GetItem());

			
		}
		else {
			//we're UNchecking the box

			// (j.jones 2007-11-07 10:38) - PLID 27987 - disallow this if the product has a "product item"
			// in any allocation, even if the allocation is marked deleted - but only if that item has an exp. date
			if(ReturnsRecords("SELECT TOP 1 ID FROM PatientInvAllocationDetailsT WHERE ProductID = %li AND ProductItemID IN (SELECT ID FROM ProductItemsT WHERE ExpDate Is Not Null)", GetItem())) {
				AfxMessageBox("This product has expireable items in use on a patient allocation (it may be deleted).\n"
					"The expiration date setting cannot be disabled for this product.");

				m_checkHasExpDate.SetCheck(!bIsCheckingExpDate);
				return;
			}

			// (j.jones 2009-01-13 17:34) - PLID 26141 - forbid from deleting if referenced in a reconciliation
			if(ReturnsRecords("SELECT TOP 1 ID FROM InvReconciliationProductItemsT WHERE ProductItemID IN (SELECT ID FROM ProductItemsT WHERE ExpDate Is Not Null AND ProductID = %li)", GetItem())) {
				AfxMessageBox("This product has expireable items that have been referenced in an Inventory Reconciliation.\n"
					"The expiration date setting cannot be disabled for this product.");

				m_checkHasExpDate.SetCheck(!bIsCheckingExpDate);
				return;
			}

			ExecuteSql("UPDATE ProductT SET HasExpDate = %li WHERE ID = %li",
				(bIsCheckingExpDate ? 1 : 0), GetItem());

			// (j.jones 2007-11-21 16:40) - PLID 28037 - ensure we account for allocated items
			_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) AS CountOfProdItems FROM ProductItemsT WHERE "
				"ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
				"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
				"			    WHERE (Status = {INT} OR Status = {INT}) "
				"				AND ProductItemID Is Not Null) "
				"AND ProductID = {INT} AND ProductItemsT.Deleted = 0",
				InvUtils::iadsActive, InvUtils::iadsUsed, GetItem());
			long CountOfProdItems = 0;
			if(!rs->eof) {
				CountOfProdItems = AdoFldLong(rs, "CountOfProdItems",0);
			}
			rs->Close();

			if(CountOfProdItems > 0) {
				CString str;
				str.Format("You have a total of %li dated items in stock (some may be blank) at all locations.\n"
					"Do you wish to clear this data?\n\n"
					"By clicking 'Yes', the expiration date information will be deleted.\n"
					"By clicking 'No', you will not be prompted to enter expiration dates on future orders, \n"
					"but you will be prompted to assign existing ones to future bills until the list is empty.\n"
					"(Blank dates will still be cleared.)",CountOfProdItems);
				int nResult = MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNOCANCEL);
				if(nResult == IDCANCEL)
					bContinue = FALSE;
				else if(nResult == IDNO) {
					//only clear blank items, and only if HasSerialNum is unchecked
					if(!m_checkHasSerial.GetCheck()) {
						// (j.jones 2007-11-07 10:49) - PLID 27987 - make absolutely sure we do not delete allocated items
						// (c.haag 2007-11-13 16:19) - PLID 27994 - Ensure we do not delete ProductItems associated with returns
						// (c.haag 2007-12-04 09:08) - PLID 28264 - Also delete ProductItemsStatusHistoryT records
						// (b.cardillo 2012-08-20 21:01) - PLID 52232 - Don't look at nulls when checking for not in SupplierReturnItemsT
						ExecuteSql("DELETE FROM ProductItemsStatusHistoryT WHERE ProductItemID IN (SELECT ID FROM ProductItemsT "
							"WHERE ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
							"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
							"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
							"AND ProductID = %li AND SerialNum Is NULL AND ExpDate Is NULL)", GetItem());
						// (j.jones 2008-06-06 10:00) - PLID 27110 - delete referencing product items first
						ExecuteParamSql("DELETE FROM ProductItemsT WHERE ReturnedFrom Is Not Null "
							"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
							"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
							"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
							"AND ProductID = {INT} AND SerialNum Is NULL AND ExpDate Is NULL", GetItem());
						ExecuteSql("DELETE FROM ProductItemsT "
							"WHERE ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
							"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
							"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
							"AND ProductID = %li AND SerialNum Is NULL AND ExpDate Is NULL", GetItem());
					}
				}
				else if(nResult == IDYES) {
					if(IDYES == MessageBox("Are you SURE you wish to permanently delete this data?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
						//first update (because they may have valid serial numbers!)
						// (b.cardillo 2012-08-20 21:01) - PLID 52232 - Don't look at nulls when checking for not in SupplierReturnItemsT
						ExecuteSql("UPDATE ProductItemsT SET ExpDate = NULL "
							"WHERE ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
							"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
							"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
							"AND ProductID = %li", GetItem());
						if(!m_checkHasSerial.GetCheck()) {
							// (j.jones 2007-11-07 10:49) - PLID 27987 - make absolutely sure we do not delete allocated items
							// (c.haag 2007-11-13 16:19) - PLID 27994 - Ensure we do not delete ProductItems associated with returns
							// (c.haag 2007-12-04 09:08) - PLID 28264 - Also delete ProductItemsStatusHistoryT records
							ExecuteSql("DELETE FROM ProductItemsStatusHistoryT WHERE ProductItemID IN (SELECT ID FROM ProductItemsT "
								"WHERE ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
								"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
								"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
								"AND ProductID = %li AND SerialNum Is NULL AND ExpDate Is NULL)", GetItem());
							// (j.jones 2008-06-06 10:00) - PLID 27110 - delete referencing product items first
							ExecuteParamSql("DELETE FROM ProductItemsT WHERE ReturnedFrom Is Not Null "
								"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
								"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
								"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
								"AND ProductID = {INT} AND SerialNum Is NULL AND ExpDate Is NULL", GetItem());
							ExecuteSql("DELETE FROM ProductItemsT "
								"WHERE ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
								"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
								"AND ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
								"AND ProductID = %li AND SerialNum Is NULL AND ExpDate Is NULL", GetItem());
						}
					}
					else {
						//back out
						bContinue = FALSE;
						AfxMessageBox("The 'Has Exp. Date' status will be left unchanged.");
					}
				}
			}					
		}

		if(!bContinue) {
			ExecuteSql("UPDATE ProductT SET HasExpDate = %li WHERE ID = %li",
				(!bIsCheckingExpDate ? 1 : 0), GetItem());			
			
			m_checkHasExpDate.SetCheck(!bIsCheckingExpDate);
		}

		// (a.walling 2007-08-06 12:44) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		m_tcProductChecker.Refresh(GetItem());

		ShowSerializedPerUOCheck();

		ShowItemsBtn();

		if(bUpdateView)
			UpdateView();

	}NxCatchAll("Could not update 'Has Exp. Date' status");
}

void CInvEditDlg::OnBtnShowItems() 
{
	try {

		ShowSerializedItems(m_nLocID);

	}NxCatchAll("Error in CInvEditDlg::OnBtnShowItems"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::ShowConsignmentTotalBoxes()
{
	// (c.haag 2008-02-07 17:07) - PLID 28852 - This will show or hide all of the consignment total
	// edit boxes and labels
	BOOL bShowUU = FALSE;
	BOOL bShowUO = FALSE;

	if(m_radioTrackQuantity.GetCheck()) {

		if(m_checkHasSerial.GetCheck() || m_checkHasExpDate.GetCheck() ||
			!IsRecordsetEmpty("SELECT ID FROM ProductItemsT WHERE "
				"ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
				"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
				"			    WHERE (Status = %li OR Status = %li) "
				"				AND ProductItemID Is Not Null) "
				"AND ProductID = %li AND ProductItemsT.Deleted = 0", InvUtils::iadsActive, InvUtils::iadsUsed, GetItem()))
		{
			bShowUU = TRUE;
			if (m_checkUseUU.GetCheck()) {
				bShowUO = TRUE;
			}
		}
	}

	// (a.walling 2008-02-15 16:42) - PLID 28946 - Don't show if unlicensed
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if (!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
		bShowUU = bShowUO = FALSE;
	}

	ShowBox(IDC_LABEL_PURCHASED_INV, bShowUU);
	ShowBox(IDC_LABEL_CONSIGNMENT, bShowUU);
	ShowBox(IDC_UU_LABEL_CONSIGNMENT, bShowUO);
	ShowBox(IDC_UO_LABEL_CONSIGNMENT, bShowUO);
	ShowBox(IDC_ACTUAL_CONSIGNMENT, bShowUU);
	//DRT 2/8/2008 - PLID 28876
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if(IsSurgeryCenter(false) || g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
		ShowBox(IDC_AVAIL_CONSIGNMENT, bShowUU);
		ShowBox(IDC_AVAIL_UO_CONSIGNMENT, bShowUO);
	}
	ShowBox(IDC_ACTUAL_UO_CONSIGNMENT, bShowUO);
	ShowBox(IDC_ORDER_CONSIGNMENT, bShowUU);
	ShowBox(IDC_ORDER_UO_CONSIGNMENT, bShowUO);
	ShowBox(IDC_REORDERPOINT_CONSIGNMENT, bShowUU);
	ShowBox(IDC_REORDERPOINT_UO_CONSIGNMENT, bShowUO);
	//ShowBox(IDC_REORDERQUANTITY_CONSIGNMENT, bShowUU);
	//ShowBox(IDC_REORDERQUANTITY_UO_CONSIGNMENT, bShowUO);

	// (j.jones 2008-02-18 14:16) - PLID 28981 - if we show any consignment fields, we need to rename the reorderpoint label
	if(bShowUU || bShowUO) {
		SetDlgItemText(IDC_REORDERPOINT_TEXT, "Reorder Pt. / Consign. Level");
	}
	else {
		SetDlgItemText(IDC_REORDERPOINT_TEXT, "Reorder Point");
	}
}

void CInvEditDlg::ShowItemsBtn(BOOL bCanReassignLocations /*= FALSE*/)
{
	try {

		//show the button if either box is checked OR we have valid data in stock
		// (c.haag 2008-02-07 17:05) - PLID 28852 - Also show/hide the consignment totals
		ShowConsignmentTotalBoxes();

		if(m_checkHasSerial.GetCheck() || m_checkHasExpDate.GetCheck() ||
			!IsRecordsetEmpty("SELECT ID FROM ProductItemsT WHERE "
				"ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
				"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
				"			    WHERE (Status = %li OR Status = %li) "
				"				AND ProductItemID Is Not Null) "
				"AND ProductID = %li AND ProductItemsT.Deleted = 0", InvUtils::iadsActive, InvUtils::iadsUsed, GetItem())) {
			ShowBox(IDC_BTN_SHOW_ITEMS, TRUE);
			//TES 6/16/2008 - PLID 27973 - Only show the button if we were told that we can.
			ShowBox(IDC_BTN_TRANSFER_PRODUCT_ITEMS, bCanReassignLocations);
		}
		else {
			ShowBox(IDC_BTN_SHOW_ITEMS, FALSE);
			ShowBox(IDC_BTN_TRANSFER_PRODUCT_ITEMS, FALSE);
		}

		//DRT 11/12/2007 - PLID 28042 - I do not want the existence of old product items to influence this (because if you are just leaving old ones around, you
		//	aren't ordering new ones as serialized / expired.  Thus, this is split off of that check.
		// (a.walling 2008-02-15 16:40) - PLID 28946 - Hide if unlicensed
		if(m_checkHasSerial.GetCheck() || m_checkHasExpDate.GetCheck()) {
			//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
			// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
			ShowBox(IDC_DEFAULT_CONSIGNMENT, g_pLicense->HasCandAModule(CLicense::cflrSilent) && TRUE);
		}
		else {
			ShowBox(IDC_DEFAULT_CONSIGNMENT, FALSE);
		}

		//(c.copits 2010-11-02) PLID 38598 - Warranty tracking system
		UpdateWarrantyControls();

	}NxCatchAll("Error in CInvEditDlg::ShowItemsBtn"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnSelChosenSupplierCombo(long nRow) 
{
	try {

		//this function will only be called if non-surgery center,
		//BUT the multi-supplier data-structure exists so we will fill that in as well

		if(m_item->CurSel == -1) {
			SetDlgItemText(IDC_CATALOG_EDIT, "");
			GetDlgItem(IDC_CATALOG_EDIT)->EnableWindow(FALSE);
			return;
		}

		if(nRow == -1) {
			SetDlgItemText(IDC_CATALOG_EDIT, "");
			GetDlgItem(IDC_CATALOG_EDIT)->EnableWindow(FALSE);
			return;
		}

		GetDlgItem(IDC_CATALOG_EDIT)->EnableWindow(TRUE);

		if (!m_refreshing) {

			int supplier = m_Supplier_Combo->GetValue(nRow, 0).lVal;

			/*
			//old supplier saving technique			
			if(supplier == -1){
				ExecuteSql("UPDATE ProductT SET Supplier = NULL WHERE ID = %i", GetItem());
				m_Supplier_Combo->CurSel = -1;
			}
			else{
				ExecuteSql("UPDATE ProductT SET Supplier = %li WHERE ID = %i", supplier, GetItem());
			}
			*/

			//for auditing
			CString strOld;
			_RecordsetPtr rs = CreateRecordset("SELECT Company FROM PersonT WHERE ID = "
				"(SELECT MultiSupplierT.SupplierID FROM ProductT INNER JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID "
				"WHERE ProductT.ID = %li)", GetItem());
			if(!rs->eof && rs->Fields->Item["Company"]->Value.vt != VT_NULL)
				strOld = CString(rs->Fields->Item["Company"]->Value.bstrVal);

			//new supplier saving technique

			//delete the supplier from MultiSupplierT
			ExecuteSql("UPDATE ProductT SET DefaultMultiSupplierID = NULL WHERE ID = %li", GetItem());
			ExecuteSql("DELETE FROM MultiSupplierT WHERE ProductID = %li", GetItem());

			CString strCatalog;
			GetDlgItemText(IDC_CATALOG_EDIT,strCatalog);

			if(supplier != -1 && strCatalog != "" && IDYES == MessageBox("Do you wish to clear out the catalog number for this item?","Practice",MB_ICONQUESTION|MB_YESNO)) {
				strCatalog = "";
				SetDlgItemText(IDC_CATALOG_EDIT,strCatalog);
			}

			if(supplier == -1) {
				SetCurrentSelection(m_Supplier_Combo, sriNoRow);
				
				SetDlgItemText(IDC_CATALOG_EDIT, "");
				GetDlgItem(IDC_CATALOG_EDIT)->EnableWindow(FALSE);
				//(e.lally 2007-05-07) PLID 25253 - Warn the user if a responsible user is assigned but no supplier exists now
				//Make sure this isn't the No User selection
				variant_t varUser = m_resp->Value[m_resp->CurSel][0];
				if(varUser.vt == VT_I4){
					DontShowMeAgain(this, "This item has a user responsible for ordering assigned to it, but the supplier was removed.", "InvEditDlgLastSupplierWithRespUser", "Practice");
				}
			}
			else {				
				ExecuteSql("INSERT INTO MultiSupplierT (SupplierID, ProductID, Catalog) VALUES (%li,%li,'%s')", supplier, GetItem(), _Q(strCatalog));

				_RecordsetPtr rs2 = CreateRecordset("SELECT ID FROM MultiSupplierT WHERE SupplierID = %li AND ProductID = %li", supplier, GetItem());
				if(!rs2->eof) {
					long nNewMultiSupplierID = AdoFldLong(rs2, "ID");
					ExecuteSql("UPDATE ProductT SET DefaultMultiSupplierID = %li WHERE ID = %li", nNewMultiSupplierID, GetItem());
				}
				rs2->Close();
			}

			//auditing
			CString strNew;
			if(m_Supplier_Combo->CurSel != -1)
				strNew = CString(m_Supplier_Combo->GetValue(m_Supplier_Combo->CurSel, 1).bstrVal);
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, CString(m_item->GetValue(m_item->CurSel, 1).bstrVal), nAuditID, aeiProductSupplier, VarLong(m_item->GetValue(m_item->CurSel, 0)), strOld, strNew, aepMedium, aetChanged);

			// (a.walling 2007-08-06 12:44) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			m_tcProductChecker.Refresh(GetItem());
		}

	}NxCatchAll("Error in CInvEditDlg::OnSelChosenSupplierCombo"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnSelChosenLocationList(long nRow) 
{
	try {

		if(nRow == -1) {
			SetCurrentSelection(m_pLocations, 0);
			nRow = 0;
		}

		m_nLocID = VarLong(m_pLocations->GetValue(nRow, 0));

		// (a.walling 2010-07-07 16:05) - PLID 39570 - Need to refresh the users for this location, too, since the location has changed.
		LoadUsersForLocation();
		Refresh();

	}NxCatchAll("Error in CInvEditDlg::OnSelChosenLocationList()"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnHistory() 
{
	try {

		CWaitCursor cuWait;
		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(274)]);
		infReport.nLocation = m_nLocID;
		infReport.nDateRange = -1; //All dates.
		infReport.nDetail = 1;
		//Now, filter on the current product.
		if(infReport.strFilterString.IsEmpty()) {
			infReport.strFilterString.Format(" {ProductProfitQ.ProdID} = %li ", VarLong(m_item->GetValue(m_item->CurSel, 0)));
		}
		else {
			infReport.strFilterString.Format(" OR {ProductProfitQ.ProdID} = %li ", VarLong(m_item->GetValue(m_item->CurSel, 0)));
		}
		//Set up the parameters.
		CPtrArray paParams;
		CRParameterInfo *paramInfo;
		
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = GetCurrentUserName();
		paramInfo->m_Name = "CurrentUserName";
		paParams.Add(paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = "01/01/1000";
		paramInfo->m_Name = "DateFrom";
		paParams.Add((void *)paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = "12/31/5000";
		paramInfo->m_Name = "DateTo";
		paParams.Add((void *)paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data.Format("%li", m_nLocID);
		paramInfo->m_Name = "LocID";
		paParams.Add((void *)paramInfo);

		infReport.strReportFile += "Dtld";


		//Made new function for running reports - JMM 5-28-04
		RunReport(&infReport, &paParams,  true, (CWnd *)this);
		ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves

	}NxCatchAll("Error in CInvEditDlg::OnHistory()"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnSelChosenItem(long nRow) 
{
	try { 

		InvUtils::SetDefault(GetItem(), InvUtils::GetDefaultCategory());

		UpdateView();
		UpdateArrows();

	}NxCatchAll("Error in CInvEditDlg::OnSelChosenItem"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling

}

void CInvEditDlg::OnRequeryFinishedItem(short nFlags) 
{
	try{
		//(a.wilson 2011-9-22) PLID 33502 - make the new selection after a deletion of a product item
		if (m_nNextItemID != -1) {
			long nRow = m_item->FindByColumn(0, m_nNextItemID, 0, false);
			m_item->PutCurSel(nRow);
			m_nNextItemID = -1;
		}
		if(m_item->GetRowCount() == 0 && m_category_list->CurSel > 0){
			AfxMessageBox("The category filter selected does not contain any products. The selection will be "
				"reset to all categories.");
			SetCurrentSelection(m_category_list, 0);
			SelChosenCategoryList(-1);
			Refresh();
			return;
		}
		//(a.wilson 2011-9-22) PLID 33502 - now update the view.
		UpdateView();
		UpdateArrows();

	}NxCatchAll("Error in OnRequeryFinishedItem");
}

void CInvEditDlg::OnKillfocusReorderpointUo() 
{
	try {

		//it is unlikely that a user will type in a decimal value here,
		//but it is allowed, as the reorder point may only be when they
		//are at half a box, etc.

		//first retrieve and format the UO Qty
		CString strQtyUO;
		GetDlgItemText(IDC_REORDERPOINT_UO,strQtyUO);
		double dblQtyUO = atof(strQtyUO);
		strQtyUO.Format("%g",dblQtyUO);
		SetDlgItemText(IDC_REORDERPOINT_UO, strQtyUO);

		//now set the UU Qty
		long nQtyUU = (long)(GetDlgItemInt(IDC_CONVERSION_EDIT) * dblQtyUO);
		SetDlgItemInt(IDC_REORDERPOINT,nQtyUU);

		//now save, which will re-calculate UO the proper way
		Save(IDC_REORDERPOINT);
		m_changed = false;

	}NxCatchAll("Error in CInvEditDlg::OnKillfocusReorderpointUo"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnKillfocusReorderpointUoConsignment() 
{
	// (c.haag 2008-02-08 11:47) - PLID 28852 - After the reorder point UO has changed,
	// we need to adjust the non-UO as well.
	try {

		//it is unlikely that a user will type in a decimal value here,
		//but it is allowed, as the reorder point may only be when they
		//are at half a box, etc.

		//first retrieve and format the UO Qty
		CString strQtyUO;
		GetDlgItemText(IDC_REORDERPOINT_UO_CONSIGNMENT,strQtyUO);
		double dblQtyUO = atof(strQtyUO);
		strQtyUO.Format("%g",dblQtyUO);
		SetDlgItemText(IDC_REORDERPOINT_UO_CONSIGNMENT, strQtyUO);

		//now set the UU Qty
		long nQtyUU = (long)(GetDlgItemInt(IDC_CONVERSION_EDIT) * dblQtyUO);
		SetDlgItemInt(IDC_REORDERPOINT_CONSIGNMENT,nQtyUU);

		//now save, which will re-calculate UO the proper way
		Save(IDC_REORDERPOINT_CONSIGNMENT);
		m_changed = false;

	}NxCatchAll("Error in CInvEditDlg::OnKillfocusReorderpointUoConsignment");
}

void CInvEditDlg::OnKillfocusReorderquantityUo() 
{
	try {

		//This should not ever be a decimal. The software can
		//handle it - the order will always round up and only
		//order in whole numbers - but it really doesn't make
		//any sense to support a decimal here.

		//NOTE: if we ever choose to support a non-whole number,
		//just model this code off of the code in
		//OnKillfocusReorderpointUo().

		//first retrieve the UO Qty
		long nQtyUO = GetDlgItemInt(IDC_REORDERQUANTITY_UO);

		//now set the UU Qty to be the UO Qty * Conversion
		long nQtyUU = (long)(GetDlgItemInt(IDC_CONVERSION_EDIT) * nQtyUO);
		SetDlgItemInt(IDC_REORDERQUANTITY,nQtyUU);

		//now save
		Save(IDC_REORDERQUANTITY);
		m_changed = false;

	}NxCatchAll("Error in CInvEditDlg::OnKillfocusReorderquantityUo"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

/*
void CInvEditDlg::OnKillfocusReorderquantityUoConsignment() 
{
	// (c.haag 2008-02-08 11:48) - PLID 28852 - After the reorder quantity UO has changed,
	// we need to adjust the non-UO as well.
	try {

		//This should not ever be a decimal. The software can
		//handle it - the order will always round up and only
		//order in whole numbers - but it really doesn't make
		//any sense to support a decimal here.

		//NOTE: if we ever choose to support a non-whole number,
		//just model this code off of the code in
		//OnKillfocusReorderpointUo().

		//first retrieve the UO Qty
		long nQtyUO = GetDlgItemInt(IDC_REORDERQUANTITY_UO_CONSIGNMENT);

		//now set the UU Qty to be the UO Qty * Conversion
		long nQtyUU = (long)(GetDlgItemInt(IDC_CONVERSION_EDIT) * nQtyUO);
		SetDlgItemInt(IDC_REORDERQUANTITY_CONSIGNMENT,nQtyUU);

		//now save
		Save(IDC_REORDERQUANTITY_CONSIGNMENT);
		m_changed = false;

	}NxCatchAll("Error in CInvEditDlg::OnKillfocusReorderquantityUoConsignment");
}*/

void CInvEditDlg::OnKillfocusReorderquantity() 
{
	try {

		//The user could type in a number that gives a decimal
		//ReorderQuantityUO value, which is not allowed. So
		//if they are using the UU/UO stuff, we need to auto-update
		//to support a whole number UO value.

		//we only need to do these calculations if UseUU is checked
		if(m_checkUseUU.GetCheck()) {

			//first retrieve the UU Qty
			long nQtyUU = GetDlgItemInt(IDC_REORDERQUANTITY);

			//now set the UO Qty
			long nConv = GetDlgItemInt(IDC_CONVERSION_EDIT);
			double dblQtyUO = double(nQtyUU) / (nConv == 0 ? 1 : nConv);

			long nQtyUO = 1;
			CString strQtyUO;
			strQtyUO.Format("%g",dblQtyUO);
			if(strQtyUO.Find(".") != -1) {
				//is not a whole number!
				long rounded = (long)dblQtyUO;
				if(rounded < dblQtyUO)
					nQtyUO = rounded + 1;
				else
					nQtyUO = rounded;

				// (j.jones 2004-07-12 09:59) - if the window is not visible (for example
				// if you are only tracking orders and not quantities), then don't warn
				if(GetDlgItem(IDC_REORDERQUANTITY)->IsWindowVisible()) {

					CString strMsg;
					strMsg.Format("You entered a Unit of Usage Reorder Quantity of %li, but your Unit of Order / Unit of Usage\n"
						"conversion ratio is %li, which would generate a non-whole Unit of Order Reorder Quantity of %s.\n\n"
						"Practice will now round up your Unit of Order Reorder Quantity to %li, and in turn update\n"
						"your Unit of Usage Reorder Quantity to %li.",nQtyUU,nConv,strQtyUO,nQtyUO,nQtyUO*nConv);
					
					AfxMessageBox(strMsg);
				}

				nQtyUU = nQtyUO * nConv;
				SetDlgItemInt(IDC_REORDERQUANTITY,nQtyUU);
			}
			else
				//is a whole number, easy
				nQtyUO = (long)dblQtyUO;
			
			SetDlgItemInt(IDC_REORDERQUANTITY_UO, nQtyUO);
		}

		//now save, which will re-calculate UO the proper way
		Save(IDC_REORDERQUANTITY);
		m_changed = false;		

	}NxCatchAll("Error saving reorder quantity.");
}

/*
void CInvEditDlg::OnKillfocusReorderquantityConsignment() 
{
	// (c.haag 2008-02-08 11:49) - PLID 28852 - This is called to save any
	// changes made to the consignment reorder quantity
	try {

		//The user could type in a number that gives a decimal
		//ReorderQuantityUO value, which is not allowed. So
		//if they are using the UU/UO stuff, we need to auto-update
		//to support a whole number UO value.

		//we only need to do these calculations if UseUU is checked
		if(m_checkUseUU.GetCheck()) {

			//first retrieve the UU Qty
			long nQtyUU = GetDlgItemInt(IDC_REORDERQUANTITY_CONSIGNMENT);

			//now set the UO Qty
			long nConv = GetDlgItemInt(IDC_CONVERSION_EDIT);
			double dblQtyUO = double(nQtyUU) / (nConv == 0 ? 1 : nConv);

			long nQtyUO = 1;
			CString strQtyUO;
			strQtyUO.Format("%g",dblQtyUO);
			if(strQtyUO.Find(".") != -1) {
				//is not a whole number!
				long rounded = (long)dblQtyUO;
				if(rounded < dblQtyUO)
					nQtyUO = rounded + 1;
				else
					nQtyUO = rounded;

				if(GetDlgItem(IDC_REORDERQUANTITY_CONSIGNMENT)->IsWindowVisible()) {

					CString strMsg;
					strMsg.Format("You entered a Unit of Usage Reorder Quantity of %li, but your Unit of Order / Unit of Usage\n"
						"conversion ratio is %li, which would generate a non-whole Unit of Order Reorder Quantity of %s.\n\n"
						"Practice will now round up your Unit of Order Reorder Quantity to %li, and in turn update\n"
						"your Unit of Usage Reorder Quantity to %li.",nQtyUU,nConv,strQtyUO,nQtyUO,nQtyUO*nConv);
					
					AfxMessageBox(strMsg);
				}

				nQtyUU = nQtyUO * nConv;
				SetDlgItemInt(IDC_REORDERQUANTITY_CONSIGNMENT,nQtyUU);
			}
			else
				//is a whole number, easy
				nQtyUO = (long)dblQtyUO;
			
			SetDlgItemInt(IDC_REORDERQUANTITY_UO_CONSIGNMENT, nQtyUO);
		}

		//now save, which will re-calculate UO the proper way
		Save(IDC_REORDERQUANTITY_CONSIGNMENT);
		m_changed = false;		

	}NxCatchAll("Error in CInvEditDlg::OnKillfocusReorderquantityConsignment");
}*/

void CInvEditDlg::HandleChangedConversionEdit() 
{
	try {

		long nConv = GetDlgItemInt(IDC_CONVERSION_EDIT);
		if(nConv == 0) {
			nConv = 1;
			SetDlgItemInt(IDC_CONVERSION_EDIT, nConv);
		}

		//if this is removed, then the call to Refresh() needs to be commented back in the
		//OnCheckUseUU function
		Save(IDC_CONVERSION_EDIT);

		OnKillfocusReorderquantity();

		//JMJ 3/29/2004 - now silently run the reorder quantity fix for products in other locations
		_RecordsetPtr rs = CreateRecordset("SELECT ReorderQuantity, LocationID FROM ProductLocationInfoT WHERE ProductID = %li",GetItem());
		while(!rs->eof) {
			long nReorderQuantity = AdoFldLong(rs, "ReorderQuantity",0);

			double dblQtyUO = double(nReorderQuantity) / (nConv == 0 ? 1 : nConv);

			long nQtyUO = 1;
			CString strQtyUO;
			strQtyUO.Format("%g",dblQtyUO);
			if(strQtyUO.Find(".") != -1) {
				//is not a whole number!
				long rounded = (long)dblQtyUO;
				if(rounded < dblQtyUO)
					nQtyUO = rounded + 1;
				else
					nQtyUO = rounded;

				nReorderQuantity = nQtyUO * nConv;

				ExecuteSql("UPDATE ProductLocationInfoT SET ReorderQuantity = %li WHERE ProductID = %li AND LocationID = %li",nReorderQuantity,GetItem(),AdoFldLong(rs, "LocationID"));
			}

			rs->MoveNext();
		}
		rs->Close();
	
	}NxCatchAll("Error changing conversion factor.");
}

void CInvEditDlg::OnBtnEquipmentMaintenance() 
{
	try {

		if (m_item->CurSel == -1)
			return;

		CEquipmentMaintenanceDlg dlg(this);
		dlg.m_ProductID = GetItem();
		dlg.DoModal();

	}NxCatchAll("Error in CInvEditDlg::OnBtnEquipmentMaintenance"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnEquipment() 
{
	try {

		BOOL bIsEquipment = m_checkEquipment.GetCheck();

		long ProductID = GetItem();

		if (!m_refreshing && bIsEquipment) {
			//check for various invalid/unnecessary settings, and correct them

			BOOL bWarn = FALSE;

			CString str = "Marking an item as a piece of equipment will invalidate the following options: \n\n";

			//Track Quantity
			if(m_radioTrackQuantity.GetCheck()) {
				str += " - 'Track Quantity' will be changed to 'Track Orders' and disabled.\n\n";
				bWarn = TRUE;
			}

			//Use UU/UO
			if(m_checkUseUU.GetCheck()) {
				str += " - 'Use Unit Of Order / Unit Of Usage' will be unchecked and disabled.\n\n";
				bWarn = TRUE;
			}

			//Has Serial Num
			if(m_checkHasSerial.GetCheck()) {
				//TES 7/3/2008 - PLID 24726 - Added the "Treat as Lot Number (not unique)" checkbox.
				str += " - 'Has Serial Number' and 'Treat as Lot Number (not unique)' will be unchecked and disabled.\n\n";
				bWarn = TRUE;
			}

			//Has Exp Date
			if(m_checkHasExpDate.GetCheck()) {
				str += " - 'Has Exp. Date' will be unchecked and disabled.\n\n";
				bWarn = TRUE;
			}

			//Serialized Per UO
			if(m_checkSerializedPerUO.GetCheck()) {
				str += " - The Serialized 'Per UO' checkbox will be unchecked and disabled.\n\n";
				bWarn = TRUE;
			}

			str += "Are you sure you wish to continue?";

			if(bWarn) {
				if(IDNO == MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
					m_checkEquipment.SetCheck(FALSE);
					return;
				}
				else {
					//okay, clear those checkboxes and save the changes

					if(m_radioTrackQuantity.GetCheck())
						ExecuteSql("UPDATE ProductLocationInfoT SET TrackableStatus = 1 WHERE TrackableStatus = 2 AND ProductID = %li",ProductID);

					ExecuteSql("UPDATE ProductT SET UseUU = 0, HasSerialNum = 0, HasExpDate = 0, SerialPerUO = 0 WHERE ID = %li",ProductID);
				}
			}
		}

		ShowBox(IDC_BTN_EQUIPMENT_MAINTENANCE, bIsEquipment);

		// Only enable the controls to interact with if the user has permissions to
		// interact with any control at all
		if ((GetCurrentUserPermissions(bioInvItem) & (SPT___W________ANDPASS)))
		{
			GetDlgItem(IDC_RADIO_TRACK_QUANTITY)->EnableWindow(!bIsEquipment);
			GetDlgItem(IDC_CHECK_USE_UU)->EnableWindow(!bIsEquipment);
			GetDlgItem(IDC_CHECK_SERIAL_NUM)->EnableWindow(!bIsEquipment);
			//TES 7/3/2008 - PLID 24726 - Added (enabled based on Has Serial Number)
			GetDlgItem(IDC_SERIAL_NUMBER_AS_LOT_NUMBER)->EnableWindow(!bIsEquipment && IsDlgButtonChecked(IDC_CHECK_SERIAL_NUM));
			GetDlgItem(IDC_CHECK_EXP_DATE)->EnableWindow(!bIsEquipment);
			GetDlgItem(IDC_CHECK_SERIALIZED_PER_UO)->EnableWindow(!bIsEquipment);
		}

		if (!m_refreshing) {
			
			ExecuteSql("UPDATE ProductT SET IsEquipment = %li WHERE ID = %li",
				bIsEquipment, ProductID);
			// (a.walling 2007-08-06 12:45) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			m_tcProductChecker.Refresh(ProductID);
			Refresh();
		}

	}NxCatchAll("Error in CInvEditDlg::OnEquipment"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::SecureControls()
{
	try {

		//(e.lally 2007-06-04) PLID 13338 - There is no write with pass permission so only check write perm.
		// Disable all the controls if the user does not have access
		if (!(GetCurrentUserPermissions(bioInvItem) & (sptWrite)))
		{
			GetDlgItem(IDC_NAME)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_SERIAL_NUM)->EnableWindow(FALSE);
			//TES 7/3/2008 - PLID 24726 - Added
			GetDlgItem(IDC_SERIAL_NUMBER_AS_LOT_NUMBER)->EnableWindow(FALSE);
			GetDlgItem(IDC_UNITDESC)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_EXP_DATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_LAST_COST)->EnableWindow(FALSE);
			GetDlgItem(IDC_INV_SHOP_FEE)->EnableWindow(FALSE);
			// (j.gruber 2012-12-04 11:11) - PLID 53239 - edit shop fee
			GetDlgItem(IDC_EDIT_SHOP_FEES)->EnableWindow(FALSE);
			GetDlgItem(IDC_EQUIPMENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_BARCODE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INS_CODE)->EnableWindow(FALSE);
			GetDlgItem(IDC_BILLABLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_PRICE)->EnableWindow(FALSE);
			GetDlgItem(IDC_TAXABLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_TAXABLE2)->EnableWindow(FALSE);
			GetDlgItem(IDC_UB92_CATEGORIES)->EnableWindow(FALSE);
			GetDlgItem(IDC_NOTES)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_USE_UU)->EnableWindow(FALSE);
			GetDlgItem(IDC_CONVERSION_EDIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_UU_EDIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_UO_EDIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_SERIALIZED_PER_UO)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_NOT_TRACKABLE)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_TRACK_ORDERS)->EnableWindow(FALSE);
			GetDlgItem(IDC_RADIO_TRACK_QUANTITY)->EnableWindow(FALSE);
			//(e.lally 2007-06-04) PLID 13338 - Read only permission should not affect our location navigation
			//GetDlgItem(IDC_LOCATION_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_REORDERPOINT)->EnableWindow(FALSE);
			GetDlgItem(IDC_REORDERPOINT_CONSIGNMENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_REORDERPOINT_UO)->EnableWindow(FALSE);
			GetDlgItem(IDC_REORDERPOINT_UO_CONSIGNMENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_REORDERQUANTITY_UO)->EnableWindow(FALSE);
			//GetDlgItem(IDC_REORDERQUANTITY_UO_CONSIGNMENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_REORDERQUANTITY)->EnableWindow(FALSE);
			//GetDlgItem(IDC_REORDERQUANTITY_CONSIGNMENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_RESP_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_INACTIVE_ITEMS)->EnableWindow(FALSE);
			GetDlgItem(IDC_ADD_SUPPLIER)->EnableWindow(FALSE);
			GetDlgItem(IDC_DELETE_SUPPLIER)->EnableWindow(FALSE);
			GetDlgItem(IDC_MAKE_DEFAULT)->EnableWindow(FALSE);
			GetDlgItem(IDC_SUPPLIER_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_SUPPLIER_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_ADD_NEW_SUPPLIER)->EnableWindow(FALSE);
			GetDlgItem(IDC_CATALOG_EDIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_SINGLE_REV_CODE)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_MULTIPLE_REV_CODES)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_SELECT_CATEGORY)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_REMOVE_CATEGORY)->EnableWindow(FALSE);
			GetDlgItem(IDC_DEFAULT_INV_PROVIDER)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_EDIT_MULTIPLE_REV_CODES)->EnableWindow(FALSE);
			GetDlgItem(IDC_ADV_REVCODE_INV_SETUP)->EnableWindow(FALSE);
			//DRT 11/8/2007 - PLID 28042
			GetDlgItem(IDC_DEFAULT_CONSIGNMENT)->EnableWindow(FALSE);

			// (j.jones 2016-04-07 10:22) - NX-100075 - added ability to remember the last charge provider
			GetDlgItem(IDC_CHECK_REMEMBER_CHARGE_PROVIDER)->EnableWindow(FALSE);

			//(c.copits 2010-11-02) PLID 38598 - Warranty tracking system
			GetDlgItem(IDC_CHECK_WARRANTY)->EnableWindow(FALSE);
			GetDlgItem(IDC_WARRANTY)->EnableWindow(FALSE);
			GetDlgItem(IDC_WARRANTY_TEXT)->EnableWindow(FALSE);
			// (s.dhole 2011-04-01 12:59) - PLID 43101 Make sure that uncheck check obx and desable button
			GetDlgItem(IDC_CHECK_IS_FRAME_DATA)->EnableWindow(FALSE);
			GetDlgItem(IDC_VIEW_FRAMES_DATA)->EnableWindow(FALSE);
			//(r.wilson 3/7/2012) PLID 48351 - Don't let user without permission access this
			GetDlgItem(IDC_BUTTON_PRINT_BC)->EnableWindow(FALSE);
			//TES 5/24/2011 - PLID 43828
			GetDlgItem(IDC_IS_CONTACT_LENS)->EnableWindow(FALSE);
			// (s.dhole 2012-03-14 14:15) - PLID 48888
			GetDlgItem(IDC_VIEW_CONTACT_INV_DATA)->EnableWindow(FALSE);
			// (j.fouts 2012-08-23 17:33) - PLID 50934 - Permissions on Manage Owners Dlg
			GetDlgItem(IDC_MANAGE_OWNERS)->EnableWindow(FALSE);
		}

	}NxCatchAll("Error in CInvEditDlg::SecureControls"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

BOOL IsWindowAnEditBoxOrTree(HWND hWnd)
{
	TCHAR strClass[10];
	if (GetClassName(hWnd, strClass, 9) == 4 && stricmp(strClass, _T("EDIT")) == 0) {
		return TRUE;
	} else if(strstr(strupr(strClass), "TREE")) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

HBRUSH CInvEditDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	try {

		//to avoid drawing problems with transparent text and disabled items
		//override the NxDialog way of doing text with a non-grey background
		//NxDialog relies on the NxColor to draw the background, then draws text transparently
		//instead, we actually color the background of the STATIC text
		if (nCtlColor == CTLCOLOR_STATIC && !IsWindowAnEditBoxOrTree(pWnd->GetSafeHwnd())) {
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			if(pWnd->GetDlgCtrlID() == IDC_CAT_FILTER_LABEL ||
				pWnd->GetDlgCtrlID() == IDC_PRODUCT_FILTER_LABEL) {
				//These two are on a different-colored NxColor.
				pDC->SetBkColor(PaletteColor(0x00A0A0A0));
				return m_brDark;
			}
			else {
				pDC->SetBkColor(PaletteColor(0x00FFDBDB));
				return m_brBlue;
			}
		}

	}NxCatchAll("Error in CInvEditDlg::OnCtlColor"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
	*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CInvEditDlg::OnSelChosenDefaultInvProvider(long nRow) 
{
	try {

		if(m_item->CurSel == -1)
			return;

		
		_variant_t varProvID = g_cvarNull;

		// (j.jones 2011-06-24 15:18) - PLID 22586 - we now support -2 for literally having "no" provider
		// on charges created with this item, but will save NULL instead of -1 which means we aren't
		// specifying any special provider
		if(nRow != -1) {
			_variant_t var = m_DefaultProviderCombo->GetValue(nRow,0);
			if(var.vt == VT_I4 && VarLong(var) != -1) {
				varProvID = var;
			}
		}

		ExecuteParamSql("UPDATE ServiceT SET ProviderID = {VT_I4} WHERE ID = {INT}", varProvID, GetItem());

	}NxCatchAll("Error setting default provider.");
}

BOOL CInvEditDlg::LoadSingleItem(long nItemID)
{
	try {

		long nSel = m_item->SetSelByColumn(0, (long)nItemID);

		if(nSel == -1) {
			//item could not be found - maybe we need to remove the category filter
			SetCurrentSelection(m_category_list, sriNoRow);
			OnSelChosenCategoryList(-1);

			nSel = m_item->SetSelByColumn(0, (long)nItemID);
		}


		if(nSel != -1) {
			//something was selected
			Refresh();
			return TRUE;
		}
		else {
			//we failed even after resetting the filter
			return FALSE;
		}

	} NxCatchAll("Error in CInvEditDlg::LoadSingleItem()"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling

	return FALSE;
}

void CInvEditDlg::OnTransfer() 
{
	try {

		CInvTransferDlg dlg(this);

		if(!CheckCurrentUserPermissions(bioInvItem, sptDynamic0))
			return;

		if (m_item->CurSel == -1)
			return;	//no inventory item selected

	//DRT 5/8/2006 - We check for this in the OnInitDialog of the transfer dialog, we do not need to check for it here as well.
		//if they don't have more than one active, managed location, they can't use this feature
	/*	if(IsRecordsetEmpty("SELECT ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND ID <> %li",m_nLocID)) {
			AfxMessageBox("You must have more than one managed location to use this feature.");
			return;
		}
	*/
		if (dlg.DoModal(GetItem(),m_nLocID) == IDOK)
			UpdateView();

	}NxCatchAll("Error in CInvEditDlg::OnTransfer"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnPendingCh() 
{
	try {

		if (m_item->CurSel == -1)
			return;	//no inventory item selected

		CPendingCaseHistoriesDlg dlg(this);
		dlg.m_nProductID = GetItem();
		dlg.m_nLocationID = m_nLocID;
		dlg.DoModal();

		Refresh();

	}NxCatchAll("Error in CInvEditDlg::OnPendingCh"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnAdvRevcodeInvSetup() 
{
	try {

		CAdvRevCodeSetupDlg dlg(this);
		dlg.m_bIsInv = TRUE;
		dlg.DoModal();

		//reflect any change that could have been made
		Refresh();

	}NxCatchAll("Error in CInvEditDlg::OnAdvRevcodeInvSetup"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnCheckSingleRevCode() 
{
	try {

		if (m_item->CurSel == -1)
			return;	//no inventory item selected

		long RevCodeUse = 0;

		if(m_checkSingleRevCode.GetCheck()) {
			//single
			RevCodeUse = 1;
			m_checkMultipleRevCodes.SetCheck(FALSE);
			m_UB92_Category->Enabled = TRUE;
			m_btnEditMultipleRevCodes.EnableWindow(FALSE);
		}
		else {
			//none
			RevCodeUse = 0;
			m_checkMultipleRevCodes.SetCheck(FALSE);
			m_UB92_Category->Enabled = FALSE;
			m_btnEditMultipleRevCodes.EnableWindow(FALSE);
		}
		
		if(m_refreshing)
			return;

		ExecuteSql("UPDATE ServiceT SET RevCodeUse = %li WHERE ID = %li",
			RevCodeUse, GetItem());
		
		// (a.walling 2007-08-06 12:45) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		m_tcProductChecker.Refresh(GetItem());

	}NxCatchAll("Could not update revenue code setup (single).");
}

void CInvEditDlg::OnCheckMultipleRevCodes() 
{
	try {

		if (m_item->CurSel == -1)
			return;	//no inventory item selected

		long RevCodeUse = 0;

		if(m_checkMultipleRevCodes.GetCheck()) {
			//multiple
			RevCodeUse = 2;
			m_checkSingleRevCode.SetCheck(FALSE);
			m_UB92_Category->Enabled = FALSE;
			m_btnEditMultipleRevCodes.EnableWindow(TRUE);
		}
		else {
			//none
			RevCodeUse = 0;
			m_checkSingleRevCode.SetCheck(FALSE);
			m_UB92_Category->Enabled = FALSE;
			m_btnEditMultipleRevCodes.EnableWindow(FALSE);
		}
		
		if(m_refreshing)
			return;

		ExecuteSql("UPDATE ServiceT SET RevCodeUse = %li WHERE ID = %li",
			RevCodeUse, GetItem());
		
		// (a.walling 2007-08-06 12:45) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		m_tcProductChecker.Refresh(GetItem());

	}NxCatchAll("Could not update revenue code setup (multiple).");
}

void CInvEditDlg::OnBtnEditMultipleRevCodes() 
{
	try {

		if (m_item->CurSel == -1)
			return;	//no inventory item selected

		CMultipleRevCodesDlg dlg(this);
		dlg.m_nServiceID = GetItem();
		dlg.m_bIsInv = TRUE;
		dlg.DoModal();

		//reflect any change that could have been made
		Refresh();

	}NxCatchAll("Error in CInvEditDlg::OnBtnEditMultipleRevCodes"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnBtnSelectCategory() 
{
	try {

		if (m_item->CurSel == -1)
			return;	//no inventory item selected

		long nServiceID = GetItem();

		// (j.jones 2015-03-03 08:44) - PLID 64964 - products can now have multiple categories
		CString strOldCategoryNames;
		std::vector<long> aryOldCategoryIDs;
		long nOldDefaultCategoryID = -1;
		LoadServiceCategories(nServiceID, aryOldCategoryIDs, strOldCategoryNames, nOldDefaultCategoryID);

		// (j.jones 2015-02-27 16:22) - PLID 64962 - added bAllowMultiSelect, true for inventory items
		// (j.jones 2015-03-02 15:36) - PLID 64970 - added strItemType
		CCategorySelectDlg dlg(this, true, "inventory item");
				
		// (j.jones 2015-03-02 08:55) - PLID 64962 - this dialog supports multiple categories
		if (aryOldCategoryIDs.size() > 0) {
			dlg.m_aryInitialCategoryIDs.insert(dlg.m_aryInitialCategoryIDs.end(), aryOldCategoryIDs.begin(), aryOldCategoryIDs.end());
			dlg.m_nInitialDefaultCategoryID = nOldDefaultCategoryID;
		}

		bool bCategoryChanged = false;

		// (j.jones 2015-03-02 10:18) - PLID 64962 - this now is just an OK/Cancel dialog		
		if (IDOK == dlg.DoModal()) {	//save it
			

			// (j.jones 2015-03-03 08:44) - PLID 64964 - now save multiple categories, and an optional default

			//save & audit using the global function, which uses the API
			std::vector<long> aryServiceIDs;
			aryServiceIDs.push_back(nServiceID);
			UpdateServiceCategories(aryServiceIDs, dlg.m_arySelectedCategoryIDs, dlg.m_nSelectedDefaultCategoryID, false);

			//now load the proper display string
			CString strNewCategoryNames;
			LoadServiceCategories(dlg.m_arySelectedCategoryIDs, dlg.m_nSelectedDefaultCategoryID, strNewCategoryNames);

			SetDlgItemText(IDC_CATEGORYBOX, strNewCategoryNames);
						
			if (strOldCategoryNames != strNewCategoryNames) {
				bCategoryChanged = true;
			}

			m_bRequery = true; //requery item list on next item selection
		}

		// (j.jones 2015-03-03 08:58) - PLID 64964 - streamlined a lot of code by
		// just reloading the category list if the product's categories changed,
		// since the category list filters by whether products are in it
		if (m_CPTCategoryChecker.Changed() || bCategoryChanged) {

			// (j.jones 2015-03-03 09:05) - PLID 64964 - moved the category reload
			// to a modular function
			ReloadCategoryList();
		}

	} NxCatchAll("Error changing category.");
}

void CInvEditDlg::OnBtnRemoveCategory() 
{
	try {

		if (m_item->CurSel == -1)
			return;	//no inventory item selected

		if(IDNO == MessageBox("Are you sure you wish to remove this item's categories?","Practice",MB_ICONQUESTION|MB_YESNO))
			return;

		long nServiceID = GetItem();

		bool bHadCategories = ReturnsRecordsParam("SELECT TOP 1 ServiceID FROM ServiceMultiCategoryT WHERE ServiceID = {INT}", nServiceID) ? true : false;
		
		// (j.jones 2015-03-03 09:03) - PLID 64964 - use the global function that calls the API
		std::vector<long> aryServiceIDs;
		aryServiceIDs.push_back(nServiceID);
		std::vector<long> aryCategoryIDs;
		UpdateServiceCategories(aryServiceIDs, aryCategoryIDs, -1, false);

		SetDlgItemText(IDC_CATEGORYBOX, "");

		// (j.jones 2015-03-03 08:58) - PLID 64964 - streamlined a lot of code by
		// just reloading the category list if the product's categories changed,
		// since the category list filters by whether products are in it
		if (m_CPTCategoryChecker.Changed() || bHadCategories) {

			// (j.jones 2015-03-03 09:05) - PLID 64964 - moved the category reload
			// to a modular function
			ReloadCategoryList();
		}

	} NxCatchAll("Error removing category.");
}

void CInvEditDlg::OnCheckSerializedPerUo() 
{
	try {

		ExecuteSql("UPDATE ProductT SET SerialPerUO = %li WHERE ID = %li",
				(m_checkSerializedPerUO.GetCheck() ? 1 : 0), GetItem());
		
	}NxCatchAll("Error in OnCheckSerializedPerUo");
}

// Returns -1 if no row selected
// Throws exceptions if the value in the specified column of the current row is not a VT_I4
long GetLongFromInvCurRow(_DNxDataList *lpdl, short nCol)
{
	_DNxDataListPtr pdl(lpdl);
	long nCurSel = pdl->GetCurSel();
	if (nCurSel != -1) {
		return VarLong(pdl->GetValue(nCurSel, nCol));
	} else {
		return -1;
	}
}

void CInvEditDlg::OnBtnAddNewSupplier() 
{
	try {

		// get the current supplier ID if there is one
		long nCurSupplierID = GetLongFromInvCurRow(m_Supplier_Combo, 0);

		bool bTableCheckerChanged = m_supplierChecker.m_changed;
		// by the time this variable receives the message that this table has changed, we've already done our refresh so
		// it think it doesn't need to update the list, setting this variable to true ensures that we will refresh
		// the lists of one is added, if a new contact is not added, then we reset the variable to whatever it was when
		// originally
		m_supplierChecker.m_changed = true;
		
		// this will create a contact and return the ID of the contact that was added
		long nNewSupplierID = GetMainFrame()->AddContact(GetMainFrame()->dctSupplier);
		if(nNewSupplierID == -1){
			m_supplierChecker.m_changed = bTableCheckerChanged;
		}

		if(nNewSupplierID != -1 && nCurSupplierID == -1){
			// a new contact was successfully added and they didn't have a supplier selected before
			// so set it to the new supplier
			OnSelChosenSupplierCombo(m_Supplier_Combo->SetSelByColumn(0, nNewSupplierID));
		}
		GetDlgItem(IDC_SUPPLIER_COMBO)->SetFocus();

	}NxCatchAll("Error in CInvEditDlg::OnBtnAddNewSupplier"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::ShowSerializedPerUOCheck() {

	try {

		BOOL bHasSerial = m_checkHasSerial.GetCheck();
		BOOL bHasExpDate = m_checkHasExpDate.GetCheck();

		if(m_checkUseUU.GetCheck() && (m_checkHasSerial.GetCheck() || m_checkHasExpDate.GetCheck())) {

			CString str;

			if(bHasSerial && bHasExpDate)
				str = "Enter Serial / Exp. Date Per Unit Of Order";
			else if(!bHasSerial && bHasExpDate)
				str = "Enter Exp. Date Per Unit of Order";
			else if(bHasSerial && !bHasExpDate)
				str = "Enter Serial Number Per Unit of Order";

			SetDlgItemText(IDC_CHECK_SERIALIZED_PER_UO,str);

			ShowBox(IDC_CHECK_SERIALIZED_PER_UO, TRUE);

			CRect rc;
			GetDlgItem(IDC_CHECK_SERIALIZED_PER_UO)->GetWindowRect(rc);
			ScreenToClient(rc);
			InvalidateRect(rc);
		}
		else {
			ShowBox(IDC_CHECK_SERIALIZED_PER_UO, FALSE);
		}

	}NxCatchAll("Error in CInvEditDlg::ShowSerializedPerUOCheck"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::ShowSerializedItems(long nLocationID)
{
	try {

		// (d.thompson 2009-10-21) - PLID 36015 - Create your own dialog, don't
		//	use the shared one anymore.
		//CProductItemsDlg& dlg = GetMainFrame()->GetProductItemsDlg();
		CProductItemsDlg dlg(this);
		dlg.m_EntryType = PI_READ_DATA;
		dlg.m_ProductID = GetItem();
		dlg.m_nLocationID = nLocationID;
		dlg.m_bUseSerial = m_checkHasSerial.GetCheck();
		dlg.m_varSerialNumIsLotNum = IsDlgButtonChecked(IDC_SERIAL_NUMBER_AS_LOT_NUMBER) ? g_cvarTrue : g_cvarFalse;
		dlg.m_bUseExpDate = m_checkHasExpDate.GetCheck();
		dlg.m_bDisallowLocationChange = TRUE; // (c.haag 2008-06-25 12:06) - PLID 28438 - Don't let the user change locations
		dlg.m_bSaveDataEntryQuery = false;	// (j.jones 2009-04-01 09:39) - PLID 33559 - make sure this is set to false, since we are borrowing an existing dialog from mainframe
		dlg.m_strSavedDataEntryQuery = ""; // (j.jones 2009-07-09 17:59) - PLID 34842 - make sure this gets cleared!
		dlg.DoModal();

	}NxCatchAll("Error in CInvEditDlg::ShowSerializedItems"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnBtnTransferProductItems() 
{
	try {

		CProductItemTransferDlg dlg(this);
		dlg.m_ProductID = GetItem();
		dlg.m_bUseSerial = m_checkHasSerial.GetCheck();
		dlg.m_bUseExpDate = m_checkHasExpDate.GetCheck();
		dlg.DoModal();

		//TES 6/16/2008 - PLID 27973 - The amount on hand may have changed, and we may want to hide this button now.
		Refresh();

	}NxCatchAll("Error in CInvEditDlg::OnBtnTransferProductItems"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

/*
(e.lally 2005-07-05) - Rather than reproduce this code everywhere, why not make it a function
*/
void CInvEditDlg::AddAllCategoriesEntry()
{
	try{
		IRowSettingsPtr pRow;
		_variant_t varID = (long) dcfAllCategories;
		pRow = m_category_list->GetRow(sriGetNewRow);
		pRow->PutValue(0,varID);
		pRow->PutValue(1,_variant_t(" {All Categories}"));
		m_category_list->AddRow(pRow);
	}NxCatchAll("Error in AddAllCategoriesEntry");
}

void CInvEditDlg::AddNoCategoryEntry()
{
	try{
		IRowSettingsPtr pRow;
		_variant_t varID = (long) dcfNoCategory;
		pRow = m_category_list->GetRow(sriGetNewRow);
		pRow->PutValue(0,varID);
		pRow->PutValue(1,_variant_t(" {No Category}"));
		m_category_list->AddRow(pRow);
	}NxCatchAll("Error in AddAllCategoriesEntry");
}

void CInvEditDlg::AddDefaultFilters() 
{
	try{
		//(e.lally 2006-08-02) PLID 18112 - Users want a way to
		//identify items with no category assigned.
		// (j.jones 2015-03-03 09:20) - PLID 64964 - this now looks at ServiceMultiCategoriesT
		if(ReturnsRecords("SELECT ServiceT.ID FROM ServiceT "
			"INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"LEFT JOIN ServiceMultiCategoryT ON ServiceT.ID = ServiceMultiCategoryT.ServiceID "
			"WHERE ServiceT.Active = 1 AND ServiceMultiCategoryT.ServiceID Is Null")){
			AddNoCategoryEntry();
		}
		AddAllCategoriesEntry();

	}NxCatchAll("Error in AddDefaultFilters");
	
}

void CInvEditDlg::OnSelChangingItem(long FAR* nNewSel) 
{
	try {
		if(*nNewSel == -1) {
			*nNewSel = m_item->CurSel;
		}
	}NxCatchAll("Error in CInvEditDlg::OnSelChangingItem"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnSelChangingRespList(long FAR* nNewSel) 
{
	try {
		if(*nNewSel == -1) {
			*nNewSel = m_resp->CurSel;
		}
	}NxCatchAll("Error in CInvEditDlg::OnSelChangingRespList"); // (j.jones 2007-12-06 08:50) - PLID 28292 - added exception handling
}

void CInvEditDlg::OnPrevLocation() 
{
	//(e.lally 2007-03-06) PLID 25054 - Add arrows to navigate locations
	try{
		long nPrevRow = m_pLocations->GetCurSel() -1;
		if(nPrevRow < 0){
			//This should never happen. This button should not be enabled.
			ASSERT(FALSE);
			return;
		}
		m_nLocID = VarLong(m_pLocations->GetValue(nPrevRow, 0));
		//Ideally, we would have a separate function to refresh just the location
				//specific information here and it would Update our location arrows.
				//Instead, we have to refresh everything.
		
		// (a.walling 2010-07-07 16:05) - PLID 39570 - Need to refresh the users for this location, too, since the location has changed.
		LoadUsersForLocation();
		Refresh();
	}NxCatchAll("Error loading previous location");
	
}

void CInvEditDlg::OnNextLocation() 
{
	//(e.lally 2007-03-06) PLID 25054 - Add arrows to navigate locations
	try{
		long nNextRow = m_pLocations->GetCurSel() +1;
		if(nNextRow >= m_pLocations->GetRowCount()){
			//This should never happen. This button should not be enabled!
			ASSERT(FALSE);
			return;
		}
		m_nLocID = VarLong(m_pLocations->GetValue(nNextRow, 0));
		//Ideally, we would have a separate function to refresh just the location
				//specific information here and it would Update our location arrows.
				//Instead, we have to refresh everything.
		
		// (a.walling 2010-07-07 16:05) - PLID 39570 - Need to refresh the users for this location, too, since the location has changed.
		LoadUsersForLocation();
		Refresh();
	}NxCatchAll("Error loading next location");
}

void CInvEditDlg::UpdateLocationArrows()
{
	//(e.lally 2007-03-06) PLID 25054 - Reflect arrow statuses for selected location
	try{
		//The dropdown is currently a datalist 1. So we'll use that syntax for now.
		long nCurRow = m_pLocations->GetCurSel();
		if(m_pLocations->GetRowCount() == 0){
			//This should not be possible. We have no locations in the list.
			ASSERT(FALSE);
			m_btnNextLocation.EnableWindow(FALSE);
			m_btnPrevLocation.EnableWindow(FALSE);
			return;
		}
		if(nCurRow == sriNoRow) {
			//Yikes, we have no current selection!
			ASSERT(FALSE);
			//Ideally, we would have a separate function to refresh just the location information,
				//without the potential of the location list still requerying.
				//Refreshing is too dangerous because there is the potential to  be caught in an infinite loop.
				//Once we get a function to reload the location specific info, we can set this to the
				//first location in the list and refresh that info, but for now the user will lose the arrows.
			m_btnNextLocation.EnableWindow(FALSE);
			m_btnPrevLocation.EnableWindow(FALSE);
			return;
		}
		if(nCurRow+1 == m_pLocations->GetRowCount()){
			//We're on the last row, disable the next arrow
			m_btnNextLocation.EnableWindow(FALSE);
		}
		else{
			//We're not on the last row, enable the next arrow
			m_btnNextLocation.EnableWindow(TRUE);
		}
		if(nCurRow == 0){
			//We're on the first row, disable the previous arrow
			m_btnPrevLocation.EnableWindow(FALSE);
		}
		else{
			//We're not on the first row, enable the previous arrow
			m_btnPrevLocation.EnableWindow(TRUE);
		}

		/*
		I already had most of this code to handle a datalist2, so I'll save it here for when
			we convert this dropdown. It will need to be updated to reflect any changes above.
		//Get the currently selected row
		IRowSettingsPtr pCurRow = m_pLocations->GetCurSel();
		if(m_pLocations->GetRowCount() == 0){
			//This should not be possible. We have no locations in the list.
			ASSERT(FALSE);
			m_btnNextLocation.EnableWindow(FALSE);
			m_btnPrevLocation.EnableWindow(FALSE);
			return;
		}
		if(pCurRow == NULL) {
			//Yikes, we have no current selection!
			ASSERT(FALSE);
			//Let's just select the first row since we know there is one.
			m_pLocations->PutCurSel(m_pLocations->GetFirstRow());
			HandleSelChangedLocation(-1);
			return;
		}
		if(pCurRow->IsSameRow(m_pLocations->GetLastRow())){
			//We're on the last row, disable the next arrow
			m_btnNextLocation.EnableWindow(FALSE);
		}
		else{
			//We're not on the last row, enable the next arrow
			m_btnNextLocation.EnableWindow(TRUE);
		}
		if(pCurRow->IsSameRow(m_pLocations->GetFirstRow())){
			//We're on the first row, disable the previous arrow
			m_btnPrevLocation.EnableWindow(FALSE);
		}
		else{
			//We're not on the first row, enable the previous arrow
			m_btnPrevLocation.EnableWindow(TRUE);
		}
		*/

	}NxCatchAll("CInvEditDlg::Error in UpdateLocationArrows");
}

// (a.walling 2007-04-24 13:36) - PLID 25356 - Edit suggested sales for this inventory item
// (a.wetta 2007-05-16 10:48) - PLID 25960 - Suggested sales are no long on this tab
/*void CInvEditDlg::OnBtnSuggestedSales() 
{
	try {
		long nProductID = GetItem();

		ASSERT(nProductID != -1);

		CSuggestedSalesDlg dlg;

		dlg.m_nServiceID = GetItem();
		GetDlgItemText(IDC_NAME, dlg.m_strDescription);

		dlg.m_nColor = 0x00FFDBDB;

		dlg.DoModal();
	} NxCatchAll("Error in OnBtnSuggestedSales");
}*/

//DRT 11/8/2007 - PLID 28042 - Save the default consignment status.
void CInvEditDlg::OnDefaultConsignment() 
{
	try {
		BOOL bChecked = m_btnDefaultConsignment.GetCheck();

		ExecuteParamSql("UPDATE ProductT SET DefaultConsignment = {INT} WHERE ProductT.ID = {INT}", bChecked != FALSE ? 1 : 0, GetItem());

	} NxCatchAll("Error in OnDefaultConsignment");
}

// (j.jones 2007-11-13 10:35) - PLID 27983 - added ability to link products to service codes
void CInvEditDlg::OnBtnLinkProducts() 
{
	try {
		//TES 7/16/2008 - PLID 27983 - Just pop up the configuration dialog.
		CLinkProductsToServicesDlg dlg(this);
		dlg.DoModal();

	} NxCatchAll("Error in CInvEditDlg::OnBtnLinkProducts");
}

// (j.jones 2008-06-10 11:31) - PLID 27665 - added OnCheckFacilityFee
void CInvEditDlg::OnCheckFacilityFee() 
{
	try {

		ExecuteParamSql("UPDATE ServiceT SET FacilityFee = {INT} WHERE ID = {INT}",
				(m_checkFacilityFee.GetCheck() ? 1 : 0), GetItem());
		
	}NxCatchAll("Error in OnCheckFacilityFee");
}

void CInvEditDlg::OnSerialNumberAsLotNumber() 
{
	if (!m_refreshing)
	try {

		//TES 7/3/2008 - PLID 24726 - Added this checkbox.  We just need to explain to the user the implications of what they're
		// doing, then go ahead and update the data.
		BOOL bCheckingBox = IsDlgButtonChecked(IDC_SERIAL_NUMBER_AS_LOT_NUMBER);
		if(bCheckingBox) {
			if(IDYES != MsgBox(MB_YESNO, "By choosing to treat this item's Serial Numbers as Lot Numbers, you will no longer be warned or "
				"prevented from entering duplicate serial numbers for units of this product.  Are you sure you wish to do this?")) {
				CheckDlgButton(IDC_SERIAL_NUMBER_AS_LOT_NUMBER, BST_UNCHECKED);
				return;
			}
		}
		else  {
			if(IDYES != MsgBox(MB_YESNO, "By choosing not to treat this item's Serial Numbers as Lot Numbers, you will no longer be allowed "
				"to enter duplicate serial numbers for units of this product.\r\n\r\n"
				"However, all existing numbers, including any duplicates, will be left unchanged.  Are you sure you wish to do this?")) {
				CheckDlgButton(IDC_SERIAL_NUMBER_AS_LOT_NUMBER, BST_CHECKED);
				return;
			}
		}

		ExecuteParamSql("UPDATE ProductT SET SerialNumIsLotNum = {BIT} WHERE ID = {INT}", bCheckingBox, GetItem());


	}NxCatchAll("Error in CInvEditDlg::OnSerialNumberAsLotNumber()");
}

// (z.manning 2010-06-21 09:26) - PLID 39257
void CInvEditDlg::OnBnClickedViewFramesData()
{
	try
	{
		// (j.gruber 2010-06-23 17:07) - PLID 39323 - made editable be true
		//take permissions into account
		BOOL bEditable = TRUE;
		if (!(GetCurrentUserPermissions(bioInvItem) & (sptWrite))) {
			bEditable = FALSE;
		}		
		// (s.dhole 2012-04-30 10:24) - PLID 46662 Set new frame to false
		CInvFramesDataDlg dlgFramesData(GetItem(),FALSE, bEditable, this);
		dlgFramesData.m_strFinalProductName= VarString(m_item->GetValue(m_item->CurSel, 1), "");;
		dlgFramesData.DoModal();

	}NxCatchAll(__FUNCTION__);
}

//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
// This function will save the check box that corresponds to the warranty being active
// when the user actually clicks it.
void CInvEditDlg::OnWarranty() 
{
	try {
		
		// Save checkbox value
		Save(IDC_CHECK_WARRANTY);

		// Enable or disable warranty text box
		if (m_CheckWarranty.GetCheck()) {
			GetDlgItem(IDC_WARRANTY)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_WARRANTY)->EnableWindow(FALSE);
		}

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
// This function will save the text box that corresponds to the number of warranty days
// when the focus is lost.
void CInvEditDlg::OnEnKillfocusWarranty()
{
	try {

		Save(IDC_WARRANTY);

	} NxCatchAll(__FUNCTION__);

}

//(c.copits 2010-11-02) PLID 38598 - Warranty tracking system
// This method determines what warranty controls should be shown under what conditions.
void CInvEditDlg::UpdateWarrantyControls()
{
	try {
		//(c.copits 2010-10-22) PLID 38598 - Warranty tracking system
		if (m_checkHasSerial.GetCheck()) {
			ShowBox(IDC_WARRANTY_TEXT, TRUE);
			ShowBox(IDC_CHECK_WARRANTY, TRUE);
			ShowBox(IDC_WARRANTY, TRUE);
			if (m_CheckWarranty.GetCheck()) {
				GetDlgItem(IDC_WARRANTY)->EnableWindow(TRUE);
			}
			else {
				GetDlgItem(IDC_WARRANTY)->EnableWindow(FALSE);
			}
		}
		else {
			ShowBox(IDC_WARRANTY_TEXT, FALSE);
			ShowBox(IDC_CHECK_WARRANTY, FALSE);
			ShowBox(IDC_WARRANTY, FALSE);
		}
	} NxCatchAll(__FUNCTION__);
}

// (s.dhole 2011-04-01 13:39) - PLID 43101 Control check unchek functionality
void CInvEditDlg::OnBnClickedCheckIsFrameData()
{
	try {
		long item = GetItem();
		if (!m_CheckIsFrameData.GetCheck()) {
			// check if thsi fram use in any glasses order
			_RecordsetPtr rs = CreateParamRecordset("Select count(GlassesOrderT.ID)  AS RecCount  FROM  ProductT INNER JOIN FramesDataT ON ProductT.FramesDataID =FramesDataT.ID  INNER JOIN "
			" GlassesFramesDataT ON GlassesFramesDataT.fpc =FramesDataT.fpc  INNER JOIN GlassesOrderT \r\n"
			" ON GlassesOrderT.GlassesFramesDataID = GlassesFramesDataT.ID \r\n"
			" WHERE ProductT.ID=  {INT} ", item );
			if ((!rs->eof) &&  (AdoFldLong(rs, "RecCount", -1)>0) )
			{
				// Refuse to delete frame data
				m_CheckIsFrameData.SetCheck(TRUE);

				//(r.wilson 3/7/2012) PLID 48351 
				GetDlgItem(IDC_BUTTON_PRINT_BC)->ShowWindow(SW_SHOWNA);

				//TES 5/24/2011 - PLID 43828 - Changed to show/hide instead of enable/disable, and added the Is Contact Lens checkbox
				GetDlgItem(IDC_VIEW_FRAMES_DATA)->ShowWindow(SW_SHOWNA);
				GetDlgItem(IDC_IS_CONTACT_LENS)->ShowWindow(SW_HIDE);
							
				AfxMessageBox(FormatString ("There are %d active or deleted Glasses orders using this Frame Data. You cannot remove a Frame Data that is in use." ,AdoFldLong(rs, "RecCount",0) )) ;
				return; 
			}
			else
			{
				// Ask user that he realy want to delete farme data
				CString strWarn;
				strWarn.Format("You are about to delete the Frame Data associated with this product, which is an UNRECOVERABLE operation.\r\n\r\n"
					"Are you sure you want to remove Frame Data from this product?");
				if( IDYES == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO) ) {
					// set this flag to true
					m_CheckIsFrameData.SetCheck(TRUE);

					//TES 5/24/2011 - PLID 43828 - Changed to show/hide instead of enable/disable, and added the Is Contact Lens checkbox
					GetDlgItem(IDC_VIEW_FRAMES_DATA)->ShowWindow(SW_SHOWNA);
					GetDlgItem(IDC_IS_CONTACT_LENS)->ShowWindow(SW_HIDE);
					ExecuteParamSql(  " SET XACT_ABORT ON \r\n"
						" BEGIN TRAN \r\n"
						" Declare @FramesDataID int; \r\n"
						" Select @FramesDataID =FramesDataID FROM  ProductT WHERE ID= {INT}; \r\n"
						" Update ProductT SET   FramesDataID=NULL  Where ID= {INT}; \r\n"
						" DELETE FROM  FramesDataT  Where ID = @FramesDataID; \r\n"
						" COMMIT TRAN ", item ,item );	
						// if no error then uncheck checkbox
						m_CheckIsFrameData.SetCheck(FALSE);
						//TES 5/24/2011 - PLID 43828 - Changed to show/hide instead of enable/disable, and added the Is Contact Lens checkbox
						GetDlgItem(IDC_VIEW_FRAMES_DATA)->ShowWindow(SW_HIDE);
						GetDlgItem(IDC_IS_CONTACT_LENS)->ShowWindow(SW_SHOWNA);
						//(r.wilson 3/7/2012) PLID 48351  - Hide the the Print barcode button if this is not a frame
						GetDlgItem(IDC_BUTTON_PRINT_BC)->ShowWindow(SW_HIDE);
					
				}
				else{
					m_CheckIsFrameData.SetCheck(TRUE);
					//(r.wilson 3/7/2012) PLID 48351 
					GetDlgItem(IDC_BUTTON_PRINT_BC)->ShowWindow(SW_SHOWNA);
					//TES 5/24/2011 - PLID 43828 - Changed to show/hide instead of enable/disable, and added the Is Contact Lens checkbox
					GetDlgItem(IDC_VIEW_FRAMES_DATA)->ShowWindow(SW_SHOWNA);
					GetDlgItem(IDC_IS_CONTACT_LENS)->ShowWindow(SW_HIDE);
					
				}

			}
		}
		else {
				// check checkbox 
				m_CheckIsFrameData.SetCheck(FALSE);
				//TES 5/24/2011 - PLID 43828 - Changed to show/hide instead of enable/disable, and added the Is Contact Lens checkbox
				GetDlgItem(IDC_VIEW_FRAMES_DATA)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_IS_CONTACT_LENS)->ShowWindow(SW_SHOWNA);

			// When user create frame data, add record into  FramesDataT and refrence to associated product
				ExecuteParamSql("  if exists ( Select * from ProductT WHERE  FramesDataID IS NULL AND ID= {INT} ) \r\n"
				  " BEGIN \r\n"
				  " SET XACT_ABORT ON \r\n"
				  " BEGIN TRAN \r\n"
				  " Declare @id int ;\r\n"
				  " insert into FramesDataT  (IsCatalog,StyleNew,ChangedPrice,FrontPrice,HalfTemplesPrice,TemplesPrice,CompletePrice,FPC) \r\n"
				  " Select 0,0,0,0,0,0,0,  (Select 'NXT'+ Cast (ISNull((Select max(ID)   From FramesDataT  ),0) +1 AS VARCHAR)) ;  \r\n" 
				  " SELECT @id = SCOPE_IDENTITY(); \r\n"
				  " Update ProductT SET   FramesDataID=@id  Where ID= {INT}; \r\n"
				  " COMMIT TRAN \r\n"
				  " END ", item , item );	
					// if no error then check checkbox
					m_CheckIsFrameData.SetCheck(TRUE);
					// (s.dhole 2012-05-14 10:03) - PLID 50354 Set Billabled and TrackQuantity
					m_billable.SetCheck(TRUE);
					m_radioNotTrackable.SetCheck(FALSE);
					m_radioTrackOrders.SetCheck(FALSE);
					m_radioTrackQuantity.SetCheck(TRUE);
					OnBillable();
					OnTrackable() ;
					//TES 5/24/2011 - PLID 43828 - Changed to show/hide instead of enable/disable, and added the Is Contact Lens checkbox
					GetDlgItem(IDC_VIEW_FRAMES_DATA)->ShowWindow(SW_SHOWNA);
					GetDlgItem(IDC_IS_CONTACT_LENS)->ShowWindow(SW_HIDE);
					//(r.wilson 3/7/2012)PLID 48351 
					GetDlgItem(IDC_BUTTON_PRINT_BC)->ShowWindow(SW_SHOWNA);
		}
	} NxCatchAll(__FUNCTION__);
}

void CInvEditDlg::OnIsContactLens()
{
	try {
		if(IsDlgButtonChecked(IDC_IS_CONTACT_LENS)) {
			//TES 5/24/2011 - PLID 43828 - Turning it on is always fine
			// (s.dhole 2012-03-14 14:15) - PLID 48888
				CNxParamSqlArray params;
				CString strSqlBatch = BeginSqlBatch();
			// (s.dhole 2012-03-15 17:50) - PLID 48856 change update to support child table
			AddParamStatementToSqlBatch(strSqlBatch,params, "DECLARE @GlassesContactLensDataID INT");
			AddParamStatementToSqlBatch(strSqlBatch,params, "INSERT INTO GlassesContactLensDataT (ProductName,Tint,LensesPerBox)  "
				"VALUES ('','','') ");
			AddParamStatementToSqlBatch(strSqlBatch,params, "SET @GlassesContactLensDataID = (SELECT CONVERT(int, SCOPE_IDENTITY()));  ");
			AddParamStatementToSqlBatch(strSqlBatch,params, "UPDATE ProductT SET GlassesContactLensDataID = @GlassesContactLensDataID, IsContactLens = 1 WHERE ID = {INT}", GetItem());
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, params);
			// (s.dhole 2012-05-14 10:03) - PLID 50354 Set Billabled and TrackQuantity
			m_billable.SetCheck(TRUE);
			m_radioNotTrackable.SetCheck(FALSE);
			m_radioTrackOrders.SetCheck(FALSE);
			m_radioTrackQuantity.SetCheck(TRUE);
			OnBillable();
			OnTrackable();
			// (s.dhole 2012-03-14 14:15) - PLID 48888
			GetDlgItem(IDC_VIEW_CONTACT_INV_DATA)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_CHECK_IS_FRAME_DATA)->EnableWindow(FALSE);
			AuditEvent(-1, CString(m_item->GetValue(m_item->CurSel, 1).bstrVal), BeginNewAuditEvent(), aeiInvIsContactLens, GetItem(), "No", "Yes", aepMedium);
			
			// (s.dhole 2012-03-14 14:15) - PLID 48888
			GetDlgItem(IDC_CHECK_IS_FRAME_DATA)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_VIEW_CONTACT_INV_DATA)->ShowWindow(SW_SHOWNA);
		}
		else {
			//TES 5/24/2011 - PLID 43737 - Check if it's in use on an order
			//// (s.dhole, 06/18/2012) - PLID 50929  
			_RecordsetPtr rsOrders = CreateParamRecordset("SELECT Count(*) AS RecordCount "
				"FROM GlassesOrderServiceT  WHERE ServiceID = {INT}", GetItem());
			long nCount = 0;
			if(!rsOrders->eof) {
				nCount = AdoFldLong(rsOrders, "RecordCount", 0);
			}
			if(nCount > 0) {
				//TES 5/24/2011 - PLID 43737 - Don't let them do it!
				MsgBox("There are %i active or deleted Contact Lens orders using this product, this value cannot be changed.", nCount);
				CheckDlgButton(IDC_IS_CONTACT_LENS, BST_CHECKED);
			}
			else {
				// (s.dhole 2012-05-14 09:30) - PLID 48888 show warning
				CString strWarnMsg;
				strWarnMsg.Format("You are about to delete the Contact Lens Data associated with this product, which is an UNRECOVERABLE operation.\r\n\r\n"
					"Are you sure you want to remove  Contact Lens Data from this product?");
				if( IDYES == MessageBox(strWarnMsg, "Practice", MB_ICONQUESTION|MB_YESNO) ) {
					//TES 5/24/2011 - PLID 43828 - We're good to go
					// (s.dhole 2012-03-15 17:50) - PLID 48888 Delete GlassesContactLensDataT if user uncheck contact lents checkbos
					ExecuteParamSql(  " SET XACT_ABORT ON \r\n"
					" BEGIN TRAN \r\n"
					" Declare @GlassesContactLensDataID int; \r\n"
					" Select @GlassesContactLensDataID =GlassesContactLensDataID FROM  ProductT WHERE ID= {INT}; \r\n"
					" Update ProductT SET  IsContactLens = 0, GlassesContactLensDataID =NULL Where ID= {INT}; \r\n"
					" DELETE FROM  GlassesContactLensDataT  Where ID = @GlassesContactLensDataID; \r\n"
					" COMMIT TRAN ", GetItem() ,GetItem() );
					GetDlgItem(IDC_CHECK_IS_FRAME_DATA)->EnableWindow(TRUE);
					GetDlgItem(IDC_CHECK_IS_FRAME_DATA)->ShowWindow(SW_SHOWNA );
					
					GetDlgItem(IDC_VIEW_CONTACT_INV_DATA)->ShowWindow(SW_HIDE);
					AuditEvent(-1, CString(m_item->GetValue(m_item->CurSel, 1).bstrVal), BeginNewAuditEvent(), aeiInvIsContactLens, GetItem(), "Yes", "No", aepMedium);
				}
				else{
					m_nxbIsContactLens.SetCheck(TRUE);
				}
			
			}
		}
	} NxCatchAll(__FUNCTION__);
}

//r.wilson 3/7/2012 PLID 48351 - Function brings up the dialog that allows the user to print the current frame
void CInvEditDlg::PrintFrameLabels()
{
	BOOL bIsFramesLabel = TRUE;
	int nProductId = -1;
	try
	{
		nProductId = VarLong(m_item->GetValue(m_item->GetCurSel(),0),-1);
		CInvLabelReportDlg  dlg(this,nProductId);		
		dlg.SetIsFramesLabel(bIsFramesLabel); 
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);

}
void CInvEditDlg::OnBnClickedButtonPrintBc()
{
	// TODO: Add your control notification handler code here
	PrintFrameLabels();
}

// (s.dhole 2012-03-14 14:30) - PLID 48888
void CInvEditDlg::OnBnClickedViewContactInvData()
{
	try
	{
		CInvContactLensDataDlg  dlg(this);
		dlg.m_strFinalName= VarString(m_item->GetValue(m_item->CurSel, 1), "");;
		dlg.m_nProductID =VarLong(m_item->GetValue(m_item->GetCurSel(),0),-1);
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-06-13 16:43) - PLID 32795 - NDC default info!
void CInvEditDlg::OnBnClickedInvNdcDefBtn()
{
	try{
		if (!CheckCurrentUserPermissions(bioAdminBilling, sptRead))
			return;

		long nProductID = VarLong(m_item->GetValue(m_item->GetCurSel(),0),-1);
		CString strProductName = VarString(m_item->GetValue(m_item->CurSel, 1), "");

		if(nProductID > 0){
			enum MenuOptions {
				moClaimNote = 1,
				moNDCDefInfo,
			};

			BOOL bClaimNoteInUse = FALSE;
			_RecordsetPtr rs = CreateParamRecordset("SELECT Convert(bit, CASE WHEN LTRIM(RTRIM(ClaimNote)) = '' THEN 0 ELSE 1 END) AS ClaimNoteInUse "
				"FROM ServiceT WHERE ID = {INT}", nProductID);
			if(!rs->eof) {
				bClaimNoteInUse = VarBool(rs->Fields->Item["ClaimNoteInUse"]->Value);
			}
			rs->Close();

			// (j.dinatale 2012-06-12 13:14) - PLID 51000 - added a menu option for the default NDC info
			CMenu mnu;
			mnu.m_hMenu = CreatePopupMenu();
			mnu.InsertMenu(0, MF_BYPOSITION |(bClaimNoteInUse ? MF_CHECKED : 0), moClaimNote, "Edit Claim &Note...");
			mnu.InsertMenu(1, MF_BYPOSITION, moNDCDefInfo, "NDC &Defaults...");

			CRect rc;
			CWnd *pWnd = GetDlgItem(IDC_PROCEDURE_DESCRIPTION);
			CPoint pt;
			GetCursorPos(&pt);
			if (pWnd) {
				pWnd->GetWindowRect(&rc);
				pt.SetPoint(rc.right, rc.top);
			}

			long nMenuChoice = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD , pt.x, pt.y, this, NULL);
			mnu.DestroyMenu();

			if(nMenuChoice == moClaimNote){
				EnterClaimNote(nProductID, strProductName);
			}else if(nMenuChoice == moNDCDefInfo){
				CNDCDefSetupDlg dlg(nProductID, true, this);
				if(dlg.DoModal() == IDOK){
					m_tcProductChecker.Refresh(nProductID);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-06-15 09:56) - PLID 51000 - default claim note on inventory items
void CInvEditDlg::EnterClaimNote(long nProductID, CString strProductName)
{
	if(nProductID == -1) {
		return;
	}

	CString strOldClaimNote = "";
	_RecordsetPtr rs = CreateParamRecordset("SELECT ClaimNote FROM ServiceT WHERE ID = {INT}", nProductID);
	if(!rs->eof) {
		strOldClaimNote = VarString(rs->Fields->Item["ClaimNote"]->Value, "");
		strOldClaimNote.TrimLeft(); strOldClaimNote.TrimRight();
	}
	rs->Close();

	CString strNewClaimNote = strOldClaimNote;
	if(IDOK == InputBoxLimited(this, "Enter a default billing note to add to charges with \"Claim\" checked:", strNewClaimNote, "", 80, false, false, NULL)) {
		strNewClaimNote.TrimLeft();
		strNewClaimNote.TrimRight();

		//ensure it's not > 80 characters
		if(strNewClaimNote.GetLength() > 80) {
			strNewClaimNote = strNewClaimNote.Left(80);
		}

		if(strNewClaimNote.CompareNoCase(strOldClaimNote) != 0) {
			ExecuteParamSql("UPDATE ServiceT SET ClaimNote = {STRING} WHERE ID = {INT}", strNewClaimNote, nProductID);

			//audit this
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, strProductName, nAuditID, aeiDefClaimNote, nProductID, "Product: " + strOldClaimNote, strNewClaimNote, aepLow, aetChanged);

			//send a tablechecker
			m_tcProductChecker.Refresh(nProductID);
		}
	}
}

// (j.fouts 2012-08-10 09:12) - PLID 50934 - Added a button to manage owners for equipment
void CInvEditDlg::OnBnClickedManageOwners()
{
	try
	{
		CInvManageOwnersDlg dlgManageOwners(this, GetItem());
		dlgManageOwners.DoModal();
	}
	NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-10-29 16:02) - PLID 53239
void CInvEditDlg::OnBnClickedEditShopFees()
{
	try {
		CString strTmp;
		COleCurrency cyItem;
		GetDlgItemText(IDC_PRICE, strTmp);
		cyItem = ParseCurrencyFromInterface(strTmp);

		CEditShopFeesDlg dlg(GetItem(), CString(m_item->GetValue(m_item->CurSel, 1).bstrVal), cyItem, GetNxColor(GNC_INVENTORY, 0));
		dlg.DoModal();

		//reload the dialog so it refreshes
		UpdateView();
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-07-16 08:46) - PLID 57566 - supported NOC type
void CInvEditDlg::OnSelChangingNocProductCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-07-16 08:46) - PLID 57566 - supported NOC type
void CInvEditDlg::OnSelChosenNocProductCombo(LPDISPATCH lpRow)
{
	try {

		if(m_item->CurSel == -1) {
			return;
		}

		NOCTypes eType = noctDefault;
		NOCTypes eOldType = noctDefault;
		_variant_t varType = g_cvarNull;

		long nProductID = VarLong(m_item->GetValue(m_item->GetCurSel(),0),-1);
		CString strProductName = VarString(m_item->GetValue(m_item->CurSel, 1), "");
		
		_RecordsetPtr rs = CreateParamRecordset("SELECT NOCType FROM ServiceT WHERE ID = {INT}", nProductID);
		if(!rs->eof) {
			//if NULL, it's Default, otherwise it's a boolean for Yes/No
			_variant_t var = rs->Fields->Item["NOCType"]->Value;
			if(var.vt == VT_BOOL) {
				BOOL bNOCType = VarBool(var);
				eOldType = bNOCType ? noctYes : noctNo;
			}
		}
		rs->Close();

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			eType = (NOCTypes)VarLong(pRow->GetValue(noccID), (long)noctDefault);
			if(eType == noctYes) {
				varType = g_cvarTrue;
			}
			else if(eType == noctNo) {
				varType = g_cvarFalse;
			}
		}

		//save the setting
		ExecuteParamSql("UPDATE ServiceT SET NOCType = {VT_BOOL} WHERE ID = {INT}", varType, nProductID);

		//audit this
		CString strOldValue, strNewValue;
		switch(eOldType) {
			case noctYes:
				strOldValue = "Yes";
				break;
			case noctNo:
				strOldValue = "No";
				break;
			case noctDefault:
			default:
				strOldValue = "<Default>";
				break;
		}
		switch(eType) {
			case noctYes:
				strNewValue = "Yes";
				break;
			case noctNo:
				strNewValue = "No";
				break;
			case noctDefault:
			default:
				strNewValue = "<Default>";
				break;
		}

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, strProductName, nAuditID, aeiProductNOCType, nProductID, strProductName + ": " + strOldValue, strNewValue, aepLow, aetChanged);

		//If they selected "Yes", ask if they want to enter a claim note.
		//Prompt even if a claim note already exists.
		if(eType == noctYes && IDYES == MessageBox("A 'Not Otherwise Classified' (NOC) code requires a billing note with the \"Claim\" box checked.\n\n"
			"A default claim note can be entered for this product by clicking the \"NDC Info\" button "
			"and selecting \"Edit Claim Note\". This note will automatically add to bills with the \"Claim\" box checked.\n\n"
			"Would you like to enter a default claim note now?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
			EnterClaimNote(nProductID, strProductName);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-07-16 08:46) - PLID 57566 - supported NOC type
LRESULT CInvEditDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_NOC_PRODUCT_LABEL:
			PopupNOCDescription();
			break;
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.jones 2013-07-16 08:46) - PLID 57566 - supported NOC type
BOOL CInvEditDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	CRect rcNOC;
	GetDlgItem(IDC_NOC_LABEL)->GetWindowRect(rcNOC);
	ScreenToClient(&rcNOC);

	if (rcNOC.PtInRect(pt)) {
		SetCursor(GetLinkCursor());
		return TRUE;
	}

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (j.jones 2013-07-16 10:49) - PLID 57566 - added NOC label
void CInvEditDlg::PopupNOCDescription()
{
	try {

		if(m_item->CurSel == -1) {
			return;
		}

		long nProductID = VarLong(m_item->GetValue(m_item->GetCurSel(),0),-1);
		CString strProductName = VarString(m_item->GetValue(m_item->CurSel, 1), "");
		
		//run a recordset to use the exact same SQL logic the ebilling export will use		
		BOOL bIsNOCCode = ReturnsRecordsParam("SELECT ServiceT.ID FROM ProductT "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"WHERE ProductT.ID = {INT} AND ProductT.InsCode <> '' "
			"AND ("
				" Name LIKE '%Not Otherwise Classified%' "
				" OR Name LIKE '%Not Otherwise%' "
				" OR Name LIKE '%Unlisted%' "
				" OR Name LIKE '%Not listed%' "
				" OR Name LIKE '%Unspecified%' "
				" OR Name LIKE '%Unclassified%' "
				" OR Name LIKE '%Not otherwise specified%' "
				" OR Name LIKE '%Non-specified%' "
				" OR Name LIKE '%Not elsewhere specified%' "
				" OR Name LIKE '%Not elsewhere%' "
				" OR Name LIKE '%nos' "
				" OR Name LIKE '%nos %' "
				" OR Name LIKE '%nos;%' "
				" OR Name LIKE '%nos,%' "
				" OR Name LIKE '%noc' "
				" OR Name LIKE '%noc %' "
				" OR Name LIKE '%noc;%' "
				" OR Name LIKE '%noc,%' "
			")", nProductID);
		
		CString strMessage;
		strMessage.Format("A \"Not Otherwise Classified\" (NOC) code is any product that has an Insurance Code entered and has the following words or phrases in the name:\n\n"
			"- Not Otherwise Classified \n"
			"- Not Otherwise \n"
			"- Unlisted \n"
			"- Not Listed \n"
			"- Unspecified \n"
			"- Unclassified \n"
			"- Not Otherwise Specified \n"
			"- Non-specified \n"
			"- Not Elsewhere Specified \n"
			"- Not Elsewhere \n"
			"- Nos (Note: Includes \"nos\", \"nos;\", \"nos,\") \n"
			"- Noc (Note: Includes \"noc\", \"noc;\", \"noc,\") \n\n"
			"If a product has an Insurance Code and any of these names, Practice will treat the product as an NOC code, unless specifically noted in this setting.\n\n"
			"This product, %s, defaults to %sbeing reported as an NOC code. "
			"If this default is not correct, change the NOC code setting to Yes or No to force NOC on or off for this product.\n\n"
			"NOC codes also require a description to be sent in ANSI claims in Loop 2400 SV101-7. "
			"Charges for NOC codes will need a billing note entered with the \"Claim\" box checked in order to enter this description. "
			"Practice will automatically report this note in the correct field for NOC codes.\n\n"
			"A default claim note can be entered for this product by clicking the \"NDC Info\" button "
			"and selecting \"Edit Claim Note\". This note will automatically add to bills with the \"Claim\" box checked.",
			strProductName, bIsNOCCode ? "" : "not ");

		MsgBox(strMessage);

	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-5-5) PLID 61831 - remove old checkbox code and create new button to open config.
void CInvEditDlg::OnBnClickedInventoryConfigureChargeLevelProviders()
{
	try {
		if (m_item->CurSel == -1) {
			// (r.gonet 05/22/2014) - PLID 61832 - No product, we have a problem.
			return;
		}

		long nProductID = VarLong(m_item->GetValue(m_item->GetCurSel(), 0), -1);
		if (nProductID == -1) {
			// (r.gonet 05/22/2014) - PLID 61832 - No product, we have a problem
			return;
		}
		// (r.gonet 05/22/2014) - PLID 61832 - Open the charge level claim providers config dialog
		CConfigureChargeLevelProviderDlg dlg(this, nProductID, CConfigureChargeLevelProviderDlg::EServiceType::Product);
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-03 09:05) - PLID 64964 - reloads the category list and re-selects the current product
void CInvEditDlg::ReloadCategoryList()
{
	try {

		// (j.jones 2015-03-03 09:05) - PLID 64964 - this code was copied in several places,
		// I moved it to one function

		long nCategoryID = -1;
		if (m_category_list->GetCurSel() != -1)
			nCategoryID = m_category_list->GetValue(m_category_list->GetCurSel(), 0);

		long nItemID = -1;
		if (m_item->GetCurSel() != -1)
			nItemID = m_item->GetValue(m_item->GetCurSel(), 0);

		//Requery the list of categories
		m_category_list->Requery();
		AddDefaultFilters();

		if (m_category_list->SetSelByColumn(0, nCategoryID) == -1)
			SetCurrentSelection(m_category_list, 0);

		//(e.lally 2006-08-02) PLID 21733 - we definitely don't need to call refresh here
		//but we will still update arrows until we are sure we don't need to.
		SelChosenCategoryList(-1);
		UpdateArrows();
		if (m_item->SetSelByColumn(0, nItemID) == -1) {
			// (b.eyers 2015-07-16) - PLID 66542 - the product doesn't exist in the product list
			// which means this was a new product that selected a different category then what was being filtered on
			// reset category filter to All, requery the product list, try to select the product again
			//SetCurrentSelection(m_item, 0);
			SetCurrentSelection(m_category_list, 0); 
			SelChosenCategoryList(-1);
			if (m_item->SetSelByColumn(0, nItemID) == -1) 
				SetCurrentSelection(m_item, 0);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-04-07 10:22) - NX-100075 - added ability to remember the last charge provider
void CInvEditDlg::OnCheckRememberChargeProvider()
{
	try {

		long nProductID = GetItem();
		if (nProductID != -1) {

			ExecuteParamSql("UPDATE ServiceT SET RememberChargeProvider = {INT} WHERE ID = {INT}",
				m_checkRememberChargeProvider.GetCheck() ? 1 : 0, nProductID);

			m_tcProductChecker.Refresh(nProductID);
		}

	}NxCatchAll(__FUNCTION__);
}

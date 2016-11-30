// InvEditOrderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "InvEditOrderDlg.h"
#include "InvOrderDlg.h"
#include "InvUtils.h"
#include "GlobalDataUtils.h"
#include "Reports.h"
#include "ReportInfo.h"
#include "GlobalFinancialUtils.h"
#include "ProductItemsDlg.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "GlobalReportUtils.h"
#include "MsgBox.h"
#include "BarCode.h"
#include "EditComboBox.h"
#include "EditCreditCardsDlg.h"
#include "SelectApptDlg.h"
#include "GlobalSchedUtils.h"
#include "TodoUtils.h"
#include "InvPatientAllocationDlg.h"
#include "SingleSelectDlg.h"
#include "InvOrderFramesConvertDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXTIMELib;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (j.jones 2009-02-11 08:54) - PLID 32871 - added defines for color
#define STANDARD_INV_COLOR	RGB(219, 219, 255)
#define AUTO_RECEIVE_COLOR	RGB(245, 227, 156)

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define	IDM_DELETE		42701
#define	IDM_RECEIVE		42702
#define IDM_PARTIAL		42703
#define IDM_UNRECEIVE	42704 //for trigger happy clickers

enum OrderListColumn
{
	O_ID = 0,
	O_ProductID,
	O_Name,
	O_Catalog,
	O_Quantity, 
	O_Amount, 	
	O_ExtraCost, 
	O_PercentOff,				// (j.jones 2008-06-19 10:12) - PLID 10394 - added discount abilities
	O_Discount,
	O_DateReceived, 
	O_Total,
	O_Conversion,
	O_UseUU,
	O_ForStatus,				//DRT 11/8/2007 - PLID 27984
								// (c.haag 2007-12-03 11:05) - PLID 28204 - This used to be O_ForConsignment,
								// which was boolean; but now we use an integer status flag called O_ForStatus
	O_HasSerialNum,				//DRT 11/8/2007 - PLID 27984
	O_SerialNumIsLotNum,		//TES 7/3/2008 - PLID 24726
	O_HasExpDate,				//DRT 11/8/2007 - PLID 27984
	O_DlgPtr,					// (c.haag 2007-12-05 15:09) - PLID 28286 - This is a CProductItemsDlg object in special cases
	O_AllocationDetailListID,	//TES 7/22/2008 - PLID 30802 - Tracks which Allocation Details are tied to this order detail.
	O_SourceDetailID		,	//TES 7/23/2008 - PLID 30802 - Only used when splitting details, tracks which detail the new detail was split from.

};

enum ItemListColumn
{
	I_ID = 0,
	I_Name,
	I_ActualPurchased,			// (j.jones 2008-02-07 11:35) - PLID 28851 - split actual, ordered, reorderpoint, reorder quantity, and surplus between purchased inv. and consignment
	I_AvailPurchased,			//DRT 2/7/2008 - PLID 28854
	I_OrderedPurchased,
	I_ReorderPointPurchased,
	I_ReorderQuantityPurchased,
	I_ToOrderPurchased,			// (j.jones 2008-02-20 10:07) - PLID 28983 - added ToOrder column
	I_ActualConsign,
	I_AvailConsign,				//DRT 2/7/2008 - PLID 28854
	I_OrderedConsign,
	I_ReorderPointConsign,
	I_ToOrderConsign,			// (j.jones 2008-02-20 10:07) - PLID 28983 - added ToOrder column
	I_LastCost,
	I_SurplusPurchased,
	I_SurplusConsign,
	I_TrackableStatus,
	I_Conversion,
	I_UseUU,
	I_Catalog,
	I_Barcode,		//DRT 3/7/2007 - PLID 24823 - Added barcode, hidden.
	I_DefaultConsignment,		//DRT 11/8/2007 - PLID 27984
	I_HasSerialNum,				//DRT 11/8/2007 - PLID 27984
	I_SerialNumIsLotNum,		//TES 7/3/2008 - PLID 24726
	I_HasExpDate,				//DRT 11/8/2007 - PLID 27984
	I_SerialPerUO,				// (c.haag 2007-12-07 16:59) - PLID 28286
};

enum SupplierListColumn
{
	S_ID = 0,
	S_Company,
	S_Address,
	S_City,
	S_State,
	S_Zip,
	S_Phone,
	S_Fax,
	S_CCNumber,
	S_Account,			//DRT 9/8/2008 - PLID 30314
};

enum LocationListColumn
{
	L_ID = 0,
	L_Name,
	L_Tax
};

// (j.jones 2009-01-23 10:01) - PLID 32822 - added order method list
enum OrderMethodColumn
{
	omcID = 0,
	omcName,
};

static COleDateTime GetDate()
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return COleDateTime(dt.GetYear(), dt.GetMonth(), dt.GetDay(), 0, 0, 0);
}

static _variant_t GetFieldValue(_RecordsetPtr &rs, LPCSTR str)
{
	return rs->Fields->Item[str]->Value;
}

static void SetFieldValue(_RecordsetPtr &rs, LPCSTR str, _variant_t &value)
{
	rs->Fields->Item[str]->Value = value;
}

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CInvEditOrderDlg dialog


CInvEditOrderDlg::CInvEditOrderDlg(CInvOrderDlg * pParent,BOOL bFrameOrder)
	: CNxDialog(CInvEditOrderDlg::IDD, NULL),
	m_taxChecker(NetUtils::Tax), m_supplierChecker(NetUtils::Suppliers),
	m_locationChecker(NetUtils::LocationsT)
{
	m_pParent = pParent;

	m_bTotalUpdateable = true;
	m_bIsScanningBarcode = FALSE;	//DRT 3/8/2007 - PLID 24823
	//{{AFX_DATA_INIT(CInvEditOrderDlg)
		m_iSupplierSel = -1;
	//}}AFX_DATA_INIT
	m_bSaveAllAsReceived = FALSE;
	m_nVisibleProductItemDlgRow = -1;
	// (j.jones 2008-03-18 14:46) - PLID 29309 - added ability to link an order with an appt.
	m_nApptID = -1;
	m_nDefLocationID = -1;
	m_nOrigLinkedApptID = -1;
	m_nDefSupplierID = -1;
	
	m_strApptLinkText = "";
	m_rcApptLinkLabel.top = m_rcApptLinkLabel.bottom = m_rcApptLinkLabel.left = m_rcApptLinkLabel.right = 0;

	m_strOldApptLinkText = "";

	m_bHasAdvInventory = FALSE;

	m_bIsModal = false;

	m_bShowApptLinkLabel = true;
	
	//(r.wilson 2012-2-17) plid 47393
	m_bFrameOrderReception = bFrameOrder;
	//m_bFrameOrderReception = FALSE;

	m_aryOrderDetailsToDelete.clear();
}

// (j.jones 2007-12-19 14:28) - PLID 28393 - added destructor
CInvEditOrderDlg::~CInvEditOrderDlg()
{
	try {

		//TES 7/22/2008 - PLID 30802 - Deallocate our lists of allocation details.
		ClearAllocationDetailIDs();

		//unregister for barcode messages
		if(GetMainFrame()) {
			if(!GetMainFrame()->UnregisterForBarcodeScan(this)) {
				MsgBox("Error unregistering for barcode scans.");
			}
		}

	}NxCatchAll("Error in CInvEditOrderDlg::~CInvEditOrderDlg");
}

void CInvEditOrderDlg::ShowSerializedItemWindow(long nRow)
{
	// (c.haag 2007-12-06 08:32) - PLID 28286 - Display the product item dialog for
	// serialized items
	CProductItemsDlg* pDlg = (CProductItemsDlg*)VarLong(m_list->Value[nRow][O_DlgPtr], NULL);
	if (NULL != pDlg) {
		// Display the dialog
		m_nVisibleProductItemDlgRow = nRow;

		// (a.walling 2008-03-20 11:36) - PLID 29333 - Change the quantity for the dialog if this row's quantity has updated
		if (m_bSaveAllAsReceived) {
			pDlg->ChangeQuantity(VarLong(m_list->Value[nRow][O_Quantity]));
		}
		pDlg->ShowWindow(SW_SHOW);
		//pDlg->DoModal();
	} else {
		// This should never happen
		ASSERT(FALSE);
	}
}

void CInvEditOrderDlg::RemoveListRow(long nRow)
{
	// (c.haag 2007-12-06 09:34) - PLID 28286 - Safely removes a list row
	if (nRow >= 0) {
		CProductItemsDlg* pDlg = (CProductItemsDlg*)VarLong(m_list->Value[nRow][O_DlgPtr], NULL);
		if (NULL != pDlg) {
			_variant_t vNull;
			vNull.vt = VT_NULL;
			m_list->Value[nRow][O_DlgPtr] = vNull;
			pDlg->DestroyWindow();
			delete pDlg;
		}
		//track which detail ID to delete
		long nOrderDetailID = VarLong(m_list->Value[nRow][O_ID], -1);
		if (nOrderDetailID != -1) {
			m_aryOrderDetailsToDelete.push_back(nOrderDetailID);
		}
		m_list->RemoveRow(nRow);
	}
}

void CInvEditOrderDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvEditOrderDlg)
	DDX_Control(pDX, IDC_LINKED_ALLOCATIONS_LABEL, m_nxsLinkedAllocationsLabel);
	DDX_Control(pDX, IDC_BTN_APPLY_DISCOUNTS, m_btnApplyDiscounts);
	DDX_Control(pDX, IDC_ORDER_CC_ON_FILE, m_btnCCOnFile);
	DDX_Control(pDX, IDC_ENABLE_SHIP_TO, m_btnShipTo);
	DDX_Control(pDX, IDC_EDIT_SHIP_METHOD, m_edtShipMethodBtn);
	DDX_Control(pDX, IDC_EDIT_CC_TYPE, m_edtCCTypeList);
	DDX_Control(pDX, IDC_AUTO_CONSIGN, m_btnAutoConsign);
	DDX_Control(pDX, IDC_AUTO_PURCH_INV, m_btnAutoPurchInv);
	DDX_Control(pDX, IDC_CHECK_TAX_SHIPPING, m_checkTaxShipping);
	DDX_Control(pDX, IDC_DEFAULT, m_defaultTaxBtn);
	DDX_Control(pDX, IDC_CANCEL_BTN, m_cancelBtn);
	DDX_Control(pDX, IDC_OK, m_okBtn);
	DDX_Control(pDX, IDC_PRINT_PREV, m_printBtn);
	DDX_Control(pDX, IDC_DELETE, m_deleteBtn);
	DDX_Control(pDX, IDC_DESCRIPTION, m_nxeditDescription);
	DDX_Control(pDX, IDC_PURCHASE_ORDER, m_nxeditPurchaseOrder);
	DDX_Control(pDX, IDC_ORDER_CONTACT_NAME, m_nxeditOrderContactName);
	DDX_Control(pDX, IDC_ORDER_CC_NAME, m_nxeditOrderCcName);
	DDX_Control(pDX, IDC_ORDER_CC_NUMBER, m_nxeditOrderCcNumber);
	DDX_Control(pDX, IDC_ORDER_CC_EXP_DATE, m_nxeditOrderCcExpDate);
	DDX_Control(pDX, IDC_TRACKING_NUMBER, m_nxeditTrackingNumber);
	DDX_Control(pDX, IDC_TAX, m_nxeditTax);
	DDX_Control(pDX, IDC_EXTRA_COST, m_nxeditExtraCost);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_LINKED_APPT_LABEL, m_nxstaticLinkedApptLabel);
	DDX_Control(pDX, IDC_SUBTOTAL, m_nxstaticSubtotal);
	DDX_Control(pDX, IDC_TAXTOTAL, m_nxstaticTaxtotal);
	DDX_Control(pDX, IDC_EXTRACOST, m_nxstaticExtracost);
	DDX_Control(pDX, IDC_TOTAL, m_nxstaticTotal);
	DDX_Control(pDX, IDC_EDIT_ORDER_METHODS, m_btnEditOrderMethods);
	DDX_Control(pDX, IDC_VENDOR_CONFIRMED_CHK, m_btnVendorConfirmed);
	DDX_Control(pDX, IDC_VENDOR_CONF_NUM, m_nxeditVendorConfNum);
	DDX_Control(pDX, IDC_BTN_ORDER_TRACK_FEDEX, m_btnTrackFedex);
	DDX_Control(pDX, IDC_BACKGROUND, m_bkg1);
	DDX_Control(pDX, IDC_BACKGROUND2, m_bkg2);
	DDX_Control(pDX, IDC_BACKGROUND3, m_bkg3);
	DDX_Control(pDX, IDC_BACKGROUND4, m_bkg4);
	DDX_Control(pDX, IDC_INV_ORDER_TOTAL_QTY, m_nxstaticQty);
	DDX_Control(pDX, IDC_FRAME_TO_PRODUCT, m_btnFrameToProduct); 
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvEditOrderDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvEditOrderDlg)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_BN_CLICKED(IDC_PRINT_PREV, OnPrintPrev)
	ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
	ON_EN_KILLFOCUS(IDC_TAX, OnKillfocusTax)
	ON_EN_KILLFOCUS(IDC_EXTRA_COST, OnKillfocusExtraCost)
	ON_BN_CLICKED(IDC_CANCEL_BTN, OnCancelBtn)
	ON_BN_CLICKED(IDC_OK, OnOkBtn)
	ON_BN_CLICKED(IDC_CHECK_TAX_SHIPPING, OnCheckTaxShipping)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_MESSAGE(NXM_POST_EDIT_PRODUCT_ITEMS_DLG, OnPostEditProductItemDlg)
	ON_BN_CLICKED(IDC_AUTO_PURCH_INV, OnAutoPurchInv)
	ON_BN_CLICKED(IDC_AUTO_CONSIGN, OnAutoConsign)
	ON_BN_CLICKED(IDC_EDIT_SHIP_METHOD, OnEditShipMethod)
	ON_BN_CLICKED(IDC_EDIT_CC_TYPE, OnEditCcType)
	ON_BN_CLICKED(IDC_ENABLE_SHIP_TO, OnEnableShipTo)
	ON_EN_UPDATE(IDC_ORDER_CC_EXP_DATE, OnUpdateOrderCcExpDate)
	ON_EN_KILLFOCUS(IDC_ORDER_CC_EXP_DATE, OnKillfocusOrderCcExpDate)
	ON_BN_CLICKED(IDC_ORDER_CC_ON_FILE, OnOrderCcOnFile)
	ON_EN_KILLFOCUS(IDC_ORDER_CC_NUMBER, OnKillfocusOrderCcNumber)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BTN_APPLY_DISCOUNTS, OnBtnApplyDiscounts)
	ON_BN_CLICKED(IDC_EDIT_ORDER_METHODS, OnEditOrderMethods)
	ON_BN_CLICKED(IDC_VENDOR_CONFIRMED_CHK, OnVendorConfirmedChkClicked)
	ON_BN_CLICKED(IDC_BTN_ORDER_TRACK_FEDEX, OnBtnOrderTrackFedex)
	//}}AFX_MSG_MAP		
	ON_BN_CLICKED(IDC_FRAME_TO_PRODUCT, &CInvEditOrderDlg::OnBnClickedFrameToProduct)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CInvEditOrderDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvEditOrderDlg)
	ON_EVENT(CInvEditOrderDlg, IDC_LIST, 7 /* RButtonUp */, OnRButtonUpList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInvEditOrderDlg, IDC_LIST, 10 /* EditingFinished */, OnEditingFinishedList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CInvEditOrderDlg, IDC_LIST, 9 /* EditingFinishing */, OnEditingFinishingList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CInvEditOrderDlg, IDC_LIST, 18 /* RequeryFinished */, OnRequeryFinishedList, VTS_I2)
	ON_EVENT(CInvEditOrderDlg, IDC_ITEM, 16 /* SelChosen */, OnAddItem, VTS_I4)
	ON_EVENT(CInvEditOrderDlg, IDC_ITEM, 18 /* RequeryFinished */, OnRequeryFinishedItem, VTS_I2)
	ON_EVENT(CInvEditOrderDlg, IDC_LOCATION, 20 /* TrySetSelFinished */, OnTrySetSelFinishedLocation, VTS_I4 VTS_I4)
	ON_EVENT(CInvEditOrderDlg, IDC_DATE_DUE, 1 /* KillFocus */, OnKillFocusDateDue, VTS_NONE)
	ON_EVENT(CInvEditOrderDlg, IDC_DATE, 1 /* KillFocus */, OnKillFocusDate, VTS_NONE)
	ON_EVENT(CInvEditOrderDlg, IDC_SUPPLIER, 16 /* SelChosen */, OnSelChosenSupplier, VTS_I4)
	ON_EVENT(CInvEditOrderDlg, IDC_LIST, 3 /* DblClickCell */, OnDblClickCellList, VTS_I4 VTS_I2)
	ON_EVENT(CInvEditOrderDlg, IDC_LIST, 4 /* LButtonDown */, OnLButtonDownList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInvEditOrderDlg, IDC_LIST, 8 /* EditingStarting */, OnEditingStartingList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CInvEditOrderDlg, IDC_SOLD_TO, 16 /* SelChosen */, OnSelChosenSoldTo, VTS_DISPATCH)
	ON_EVENT(CInvEditOrderDlg, IDC_LOCATION, 1 /* SelChanging */, OnSelChangingLocation, VTS_PI4)
	ON_EVENT(CInvEditOrderDlg, IDC_SOLD_TO, 1 /* SelChanging */, OnSelChangingSoldTo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvEditOrderDlg, IDC_LOCATION, 16 /* SelChosen */, OnSelChosenLocation, VTS_I4)
	ON_EVENT(CInvEditOrderDlg, IDC_LINKED_ALLOCATIONS, 19 /* LeftClick */, OnLeftClickLinkedAllocations, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CInvEditOrderDlg message handlers

BOOL CInvEditOrderDlg::OnInitDialog() 
{
//	IRowSettingsPtr pRow;
	try{
		CNxDialog::OnInitDialog();
		
		// (r.wilson - 2/6/2012) PLID 47394
		g_propManager.CachePropertiesInBulk("INVEDITORDERDLG", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'Inventory_AutoPopulateOrderDesc' "			
			")",
			_Q(GetCurrentUserName()));

		m_defaultTaxBtn.SetTextColor(0x0);
		m_cancelBtn.AutoSet(NXB_CANCEL);
		m_okBtn.AutoSet(NXB_OK);
		// (c.haag 2008-04-29 11:14) - PLID 29820 - We now have a printer icon
		m_printBtn.AutoSet(NXB_PRINT_PREV);

		// (j.jones 2009-02-10 16:42) - PLID 32871 - change the background colors if auto-receiving
		ConfigureDisplay();

		m_deleteBtn.AutoSet(NXB_DELETE);

		// (j.jones 2008-02-07 10:02) - PLID 28851 - split the Auto-order function into two,
		// one for purchased inventory, one for consignment
		// (c.haag 2008-05-01 14:59) - PLID 29820 - Since the Auto features modify the order,
		// they need to have modify styles.
		//m_btnAutoPurchInv.SetTextColor(0x0);
		//m_btnAutoConsign.SetTextColor(0x0);
		m_btnAutoPurchInv.AutoSet(NXB_MODIFY);
		m_btnAutoConsign.AutoSet(NXB_MODIFY);
		// (c.haag 2008-05-20 12:31) - PLID 29820 - Default Tax as well
		m_defaultTaxBtn.AutoSet(NXB_MODIFY);

		// (j.jones 2008-06-19 10:06) - PLID 10394 - added m_btnApplyDiscounts
		m_btnApplyDiscounts.AutoSet(NXB_MODIFY);

		// (b.spivey, September 20, 2011) - PLID 45265 - Added m_btnFrameToProduct 
		m_btnFrameToProduct.AutoSet(NXB_NEW); 
		
		m_location = BindNxDataListCtrl(IDC_LOCATION,	true);
		m_supplier = BindNxDataListCtrl(IDC_SUPPLIER,	true);
	//	m_category = BindNxDataListCtrl(IDC_CATEGORY,	true);
		m_item	   = BindNxDataListCtrl(IDC_ITEM,		false);
		m_list	   = BindNxDataListCtrl(IDC_LIST,		false);

		// (j.jones 2008-03-19 09:26) - PLID 29309 - cached the adv. inventory flag
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		m_bHasAdvInventory = g_pLicense->HasCandAModule(CLicense::cflrSilent);
			
		// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
		//(e.lally 2008-10-08) PLID 31621 - Datalist2.0 controls need to use BindNxDataList2Ctrl
		m_pLocationSoldTo = BindNxDataList2Ctrl(IDC_SOLD_TO, true);
		m_pShipMethodList = BindNxDataList2Ctrl(IDC_SHIPPING_METHOD_LIST, true);
		
		// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
		//(e.lally 2008-10-08) PLID 31621 - Datalist2.0 controls need to use BindNxDataList2Ctrl
		m_pCCTypeList = BindNxDataList2Ctrl(IDC_CC_TYPE_LIST, true);	
		
		//TES 7/22/2008 - PLID 30802 - A list of allocation details that are tied to this order.
		m_pLinkedAllocations = BindNxDataList2Ctrl(IDC_LINKED_ALLOCATIONS, false);

		// (j.jones 2009-01-23 09:57) - PLID 32822 - added order method
		m_pOrderMethodCombo = BindNxDataList2Ctrl(IDC_ORDER_METHOD_COMBO, true);

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOrderMethodCombo->GetNewRow();
		pRow->PutValue(omcID, (long)-1);
		pRow->PutValue(omcName, _variant_t(" <None>"));
		m_pOrderMethodCombo->AddRowSorted(pRow, NULL);
		m_pOrderMethodCombo->PutCurSel(pRow);

		// (c.haag 2007-12-11 13:17) - PLID 28204 - Set the combo source for the status column
		CString strStatusSource = FormatString("%d;Purchased Inv.;%d;Consignment", 
			InvUtils::pisPurchased, InvUtils::pisConsignment);
		m_list->GetColumn(O_ForStatus)->ComboSource = _bstr_t(strStatusSource);

		// (a.walling 2008-02-15 12:22) - PLID 28946 - Hide the consignment button if not licensed
		if (!m_bHasAdvInventory) {
			m_btnAutoConsign.EnableWindow(FALSE);
			m_btnAutoConsign.ShowWindow(SW_HIDE);

			// and prevent the dropdown from being edited
			m_list->GetColumn(O_ForStatus)->ColumnStyle = csVisible | csFixedWidth;
			m_list->GetColumn(O_ForStatus)->Editable = VARIANT_FALSE;
			m_list->GetColumn(O_ForStatus)->StoredWidth = 0;

			// (a.walling 2008-02-18 14:50) - PLID 28946 - Also hide the various columns in the item dropdown
	#define HIDE_COL(name) m_item->GetColumn(name)->ColumnStyle = csVisible | csFixedWidth; \
		m_item->GetColumn(name)->Editable = VARIANT_FALSE; \
		m_item->GetColumn(name)->StoredWidth = 0;

			HIDE_COL(I_ActualConsign);
			HIDE_COL(I_AvailConsign);
			HIDE_COL(I_OrderedConsign);
			HIDE_COL(I_ReorderPointConsign);
			HIDE_COL(I_ToOrderConsign);
			HIDE_COL(I_SurplusConsign);
			HIDE_COL(I_DefaultConsignment);

			for (int c = 0; c < m_item->GetColumnCount(); c++) {
				IColumnSettingsPtr pCol = m_item->GetColumn(c);
				if (pCol) {
					CString str = (LPCTSTR)(pCol->ColumnTitle);
					if (str.Find("(Purch)") != -1) {
						str.Replace("(Purch)", "");
						str.TrimRight();
						pCol->ColumnTitle = _bstr_t(str);
					}
				}
			}

			// (a.walling 2008-02-18 15:04) - PLID 28946 - Resize the drop down as well to previous value
			m_item->DropDownWidth = 543;
		}

		m_supplier->CurSel = 0;
		m_iSupplierSel = m_supplier->CurSel;

	//	pRow = m_category->GetRow(-1);
	//	pRow->Value[0] = _variant_t();
	//	pRow->Value[1] = _variant_t(" All Categories");
	//	m_category->AddRow(pRow);

		m_nxtDate = GetDlgItemUnknown(IDC_DATE);
		m_nxtArrivalDate = GetDlgItemUnknown(IDC_DATE_DUE);
		// (d.thompson 2009-01-23) - PLID 32823
		m_nxtDateConfirmed = GetDlgItemUnknown(IDC_DATE_CONFIRMED);

		//r.wilson 3/16/2012 PLID 47393 
		//r.wilson 4/13/2012 PLID 47393  - Updated FROM, WHERE, and Group BY clauses
		// (j.dinatale 2013-02-25 10:55) - PLID 55251 - removed code that was changing what the supplier list was requerying with,
		//		we want all suppliers at all times to show up here.

		m_strDefaultWhereClause = AsString(m_supplier->GetWhereClause());

		//DRT 3/7/2007 - PLID 24823 - We want to receive barcode messages.  Don't care if it
		//	failed, the user will have received a message and there's nothing we can do about it.
		GetMainFrame()->RegisterForBarcodeScan(this);

		// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
		//add the no selection values to shipping list
		pRow = m_pShipMethodList->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _variant_t("<No Shipping Method>"));
		m_pShipMethodList->AddRowSorted(pRow, NULL);


		// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
		pRow = m_pCCTypeList->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _variant_t("<None>"));
		m_pCCTypeList->AddRowSorted(pRow, NULL);

		// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
		CheckDlgButton(IDC_ENABLE_SHIP_TO, 0);
		GetDlgItem(IDC_LOCATION)->EnableWindow(FALSE);

		// (b.spivey, September 20, 2011) - PLID 45266 - If they don't have a glasses license, we don't show the button. 
		if(!g_pLicense->CheckForLicense(CLicense::lcFrames, CLicense::cflrSilent)){
			GetDlgItem(IDC_FRAME_TO_PRODUCT)->EnableWindow(FALSE);
			GetDlgItem(IDC_FRAME_TO_PRODUCT)->ShowWindow(SW_HIDE); 

			// (b.spivey, September 22, 2011) - PLID 45266 - If we don't have the license, we should really reset the size so 
			//	 people who won't even use this feature will be unaffected by the UI change. 
			CWnd *pWnd = NULL;
			pWnd = GetDlgItem(IDC_AUTO_PURCH_INV); // I set the target Y coordinate to this button, which should be the 
												   //   lowest point in this region. 
			if (pWnd) {
				CRect rcTarget;
				pWnd->GetWindowRect(&rcTarget);	// Get control's position.
				ScreenToClient(&rcTarget);   		// Convert from absolute screen coordinates to dialog-relative coordinates.
				// (b.spivey, September 22, 2011) - PLID 45266 - We need a little bit of an offset here. The actual size of the
				//   NxColor control includes the negative space. 
				long nTargetY = rcTarget.bottom + 10;  

				//Move the color control
				pWnd = NULL; 
				pWnd = GetDlgItem(IDC_BACKGROUND2);
				if(pWnd){
					CRect rc;
					pWnd->GetWindowRect(&rc);	// Get control's position.
					ScreenToClient(&rc);   		// Convert from absolute screen coordinates to dialog-relative coordinates.	
					long height = nTargetY - rc.top; // We want the NxColor control to get shorter. 
					long width = rc.right - rc.left; //the actual width of the control. 
					pWnd->MoveWindow(	// Resize window
						rc.left,    // x. remains unchanged
						rc.top,		// y. remains unchanged
						width,	    // new width
						height, 	// new height
						TRUE);
					}

				//Move the datalist. 
				pWnd = NULL; 
				pWnd = GetDlgItem(IDC_LIST); 
				if(pWnd){
					CRect rc;
					pWnd->GetWindowRect(&rc);	// Get control's position.
					ScreenToClient(&rc);   		// Convert from absolute screen coordinates to dialog-relative coordinates.	
					long height = rc.bottom - nTargetY; // We want the datalist to get taller. 
					long width = rc.right - rc.left; //the actual width of the control. 
					pWnd->MoveWindow(	// Resize window
						rc.left,    // x. remains unchanged
						nTargetY,	// y. remains unchanged
						width,	    // new width
						height, 	// new height
						TRUE);
					}
				}
			UpdateWindow();
		}

		// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
		((CNxEdit*)GetDlgItem(IDC_ORDER_CONTACT_NAME))->SetLimitText(500);
		((CNxEdit*)GetDlgItem(IDC_PURCHASE_ORDER))->SetLimitText(255);
		// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
		((CNxEdit*)GetDlgItem(IDC_ORDER_CC_NAME))->SetLimitText(500);
		((CNxEdit*)GetDlgItem(IDC_ORDER_CC_NUMBER))->SetLimitText(50);
		// (d.thompson 2009-01-23) - PLID 32823 - Vendor confirmation number
		m_nxeditVendorConfNum.SetLimitText(50);

		m_strCreditCardNumber = "";

		// (j.jones 2008-03-19 09:05) - PLID 29309 - calculate the hyperlink rectangle	
		if(m_bHasAdvInventory) {
			CWnd *pWnd;

			pWnd = GetDlgItem(IDC_LINKED_APPT_LABEL);
			if (pWnd->GetSafeHwnd()) {
				// Get the position of the is hotlinks
				pWnd->GetWindowRect(m_rcApptLinkLabel);
				ScreenToClient(&m_rcApptLinkLabel);

				// Hide the static text that was there
				pWnd->ShowWindow(SW_HIDE);
			}
		}
		else {
			GetDlgItem(IDC_LINKED_APPT_LABEL)->ShowWindow(SW_HIDE);

			//TES 7/22/2008 - PLID 30802 - No Adv. Inventory, no Allocations.
			HideLinkedAllocations();
		}

		//TES 6/13/2008 - PLID 28078 - We are now sometimes called modally, so if we were, initialize the screen.
		if(m_bIsModal) {
			Initialize();
		}

		return TRUE;
	}NxCatchAll(__FUNCTION__);
	return FALSE; 
}

double CInvEditOrderDlg::GetTax()
{
	CString str;
	GetDlgItemText(IDC_TAX, str);
	str.TrimRight('%');
	return 1.0 + atof(str) / 100.0;
}

void CInvEditOrderDlg::SetTax(double tax)
{
	CString str;
	str.Format("%g%%", (tax - 1.0) * 100.0);
	SetDlgItemText(IDC_TAX, str);
}

// (j.jones 2009-02-16 10:16) - PLID 33085 - changed Refresh() to LoadOrder()
BOOL CInvEditOrderDlg::LoadOrder()
{
	_RecordsetPtr	rs;
	IRowSettingsPtr	pRow;

	ASSERT(m_id != -1);//cannot load an old order if it is a new one

	try
	{
		//load order
		// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
		// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
		// (j.jones 2008-03-18 17:09) - PLID 29309 - added appointment ID (there should never be more than one)
		// (j.jones 2009-01-23 10:26) - PLID 32822 - added OrderMethodID
		// (d.thompson 2009-01-23) - PLID 32823 - Added VendorConfirmed, ConfirmationDate, ConfirmationNumber
		rs = CreateParamRecordset ("SELECT TrackingID, Description, Notes, ExtraCost, Supplier, "
			"DateDue, TaxShipping, LocationID, Date, Tax, PurchaseOrderNum, LocationSoldTo, ContactName, "
			" CCName, CCNumber, CCTypeID, CCExpDate, ShipMethodID, OrderAppointmentsT.AppointmentID, OrderMethodID, "
			" VendorConfirmed, ConfirmationDate, ConfirmationNumber "
			"FROM OrderT "
			"LEFT JOIN OrderAppointmentsT ON OrderT.ID = OrderAppointmentsT.OrderID "
			"WHERE OrderT.ID = {INT}", m_id);

		SetDlgItemVar(IDC_TRACKING_NUMBER,	GetFieldValue(rs, "TrackingID"),true, true);
		_variant_t varDate = GetFieldValue(rs, "Date");
		if(varDate.vt == VT_DATE) {
			m_nxtDate->SetDateTime(VarDateTime(varDate));
		}
		else {
			m_nxtDate->Clear();
		}
		SetDlgItemVar(IDC_DESCRIPTION,		GetFieldValue(rs, "Description"),true, true);
		SetDlgItemVar(IDC_NOTES,			GetFieldValue(rs, "Notes"),		true, true);
		SetDlgItemVar(IDC_EXTRA_COST,		GetFieldValue(rs, "ExtraCost"),	true, true);
		varDate = GetFieldValue(rs, "DateDue");
		if(varDate.vt == VT_DATE) {
			m_nxtArrivalDate->SetDateTime(VarDateTime(varDate));
		}
		else {
			m_nxtArrivalDate->Clear();
		}

		// (j.jones 2008-03-18 17:14) - PLID 29309 - load the AppointmentID
		m_nApptID = AdoFldLong(rs, "AppointmentID",-1);
		m_nOrigLinkedApptID = m_nApptID;
		DisplayLinkedApptData();
		// (j.jones 2008-03-19 11:50) - PLID 29316 - cache the resulting text for auditing
		if(m_nOrigLinkedApptID != -1) {
			m_strOldApptLinkText = m_strApptLinkText;
		}

		//(e.lally 2005-11-04) PLID 18152 - We need to account for inactive suppliers.
		//if the current where clause isn't what we expect it to be, requery. This would happen if the
			//last order we looked at had an inactive supplier.
		//TES 11/6/2007 - PLID 27981 - VS2008 - Need to convert _bstr_t to CString
		if(CString((LPCTSTR)m_supplier->WhereClause) != m_strDefaultWhereClause){
			m_supplier->PutWhereClause(_bstr_t(m_strDefaultWhereClause));
			m_supplier->Requery();
		}
		//Unfortunately, we can't assume there is a supplier for this order
		if(GetFieldValue(rs, "Supplier").vt != VT_NULL){
			long nSupplierID = AdoFldLong(rs, "Supplier");
			//if this supplier isn't in the list, check if it is inactive
			if(m_supplier->SetSelByColumn(S_ID,nSupplierID) == -1){
				//The order may have an inactive supplier
				_RecordsetPtr rsSup = CreateRecordset("SELECT Company From PersonT WHERE ID = %li", nSupplierID);
				if(!rsSup->eof){
					//Add just this inactive supplier
					CString strSupFilter ="";
					strSupFilter.Format(" OR PersonID = %li", nSupplierID);
					CString strWhereClause = m_strDefaultWhereClause + strSupFilter;
					//Set the where clause and requery. We should only have to requery twice if the user opens
						//two orders in a row with inactive suppliers.
					m_supplier->WhereClause = _bstr_t(strWhereClause);
					m_supplier->Requery();
				}
				else{
					m_supplier->PutCurSel(0);
				}
			}
		}//check for NULL supplier

		m_supplier->SetSelByColumn(S_ID, GetFieldValue(rs, "Supplier"));
		m_iSupplierSel = m_supplier->CurSel;

		if(m_location->SetSelByColumn(0, GetFieldValue(rs, "LocationID")) == -1) {
			//This is probably an inactive location.
			_RecordsetPtr rsLocName = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE ID = %li", VarLong(GetFieldValue(rs, "LocationID"), -1));
			if(!rsLocName->eof) {
				m_location->PutComboBoxText(_bstr_t(AdoFldString(rsLocName, "Name")));

			}
		}

		m_checkTaxShipping.SetCheck(AdoFldBool(rs, "TaxShipping",FALSE));
		SetTax(GetFieldValue(rs, "Tax").dblVal);


		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pLocationSoldTo->SetSelByColumn(0, GetFieldValue(rs, "LocationSoldTo"));
		if(pRow == NULL) {
			//This is probably an inactive location.
			_RecordsetPtr rsLocName = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE ID = %li", VarLong(GetFieldValue(rs, "LocationSoldTo"), -1));
			if(!rsLocName->eof) {
				m_pLocationSoldTo->PutComboBoxText(_bstr_t(AdoFldString(rsLocName, "Name")));

			}
		}

		// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
		SetDlgItemVar(IDC_PURCHASE_ORDER,		GetFieldValue(rs, "PurchaseOrderNum"),true, true);
		SetDlgItemVar(IDC_ORDER_CONTACT_NAME,		GetFieldValue(rs, "ContactName"),true, true);
		// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
		SetDlgItemVar(IDC_ORDER_CC_NAME,		GetFieldValue(rs, "CCName"),true, true);

		m_strCreditCardNumber = DecryptStringFromVariant(rs->Fields->Item["CCNumber"]->Value);
		if (m_strCreditCardNumber.CompareNoCase("ON FILE") == 0) {
			CheckDlgButton(IDC_ORDER_CC_ON_FILE, 1);
			GetDlgItem(IDC_ORDER_CC_NUMBER)->EnableWindow(FALSE);
			SetDlgItemText(IDC_ORDER_CC_NUMBER, "ON FILE");
		}
		else {
			CheckDlgButton(IDC_ORDER_CC_ON_FILE, 0);
			GetDlgItem(IDC_ORDER_CC_NUMBER)->EnableWindow(TRUE);
		
			if (m_strCreditCardNumber.GetLength() <= 4) {
				SetDlgItemText(IDC_ORDER_CC_NUMBER, m_strCreditCardNumber);
			}
			else {
				//mask everything but the last 4 digits
				CString strTemp;
				for (int i = 0; i < m_strCreditCardNumber.GetLength() - 4; i++) {
					strTemp += "X";
				}

				strTemp += m_strCreditCardNumber.Right(4);

				SetDlgItemText(IDC_ORDER_CC_NUMBER, strTemp);
			}
		}
		
		COleDateTime dtNULL;
		dtNULL.SetDate(1899,12,31);
		COleDateTime dtExp = AdoFldDateTime(rs, "CCExpDate", dtNULL);
		if (dtExp != dtNULL) {
			GetDlgItem(IDC_ORDER_CC_EXP_DATE)->SetWindowText(dtExp.Format("%m/%y"));
		}
		else {
			SetDlgItemText(IDC_ORDER_CC_EXP_DATE, "");
		}

		//shipping method
		// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
		m_pShipMethodList->SetSelByColumn(0, AdoFldLong(rs, "ShipMethodID", -1));

		// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
		long nCCTypeID = AdoFldLong(rs, "CCTypeID", -1);
		if (nCCTypeID != -1) {
			pRow = m_pCCTypeList->SetSelByColumn(0, nCCTypeID);
			if (pRow == NULL) {
				//could be inactive
				_RecordsetPtr rsCCTypeName = CreateRecordset("SELECT CardName FROM CreditCardNamesT WHERE ID = %li", nCCTypeID);
				if (!rsCCTypeName->eof) {
					m_pCCTypeList->PutComboBoxText(_bstr_t(AdoFldString(rsCCTypeName, "CardName", "")));
				}
			}
			
		}
		else {
			//they don't have one selected, so put that as the selection
			m_pCCTypeList->SetSelByColumn(0, (long) -1);
		}

		// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
		if (AdoFldLong(rs, "LocationID", -1) == AdoFldLong(rs, "LocationSoldTo", -1)) {
			CheckDlgButton(IDC_ENABLE_SHIP_TO, FALSE);
			GetDlgItem(IDC_LOCATION)->EnableWindow(FALSE);
		}
		else {
			CheckDlgButton(IDC_ENABLE_SHIP_TO, TRUE);
			GetDlgItem(IDC_LOCATION)->EnableWindow(TRUE);
		}

		// (j.jones 2009-01-23 10:26) - PLID 32822 - added order method
		m_pOrderMethodCombo->SetSelByColumn(omcID, AdoFldLong(rs, "OrderMethodID", -1));

		// (d.thompson 2009-01-23) - PLID 32823 - Added vendor confirmation fields
		CheckDlgButton(IDC_VENDOR_CONFIRMED_CHK, AdoFldBool(rs, "VendorConfirmed"));
		SetDlgItemText(IDC_VENDOR_CONF_NUM, AdoFldString(rs, "ConfirmationNumber"));
		_variant_t varConfDate = rs->Fields->Item["ConfirmationDate"]->Value;
		if(varConfDate.vt == VT_DATE) {
			m_nxtDateConfirmed->SetDateTime(VarDateTime(varConfDate));
		}
		else {
			m_nxtDateConfirmed->Clear();
		}
		EnsureConfirmedControls();

		rs->Close();

		//line items
		CString strWhere;
		strWhere.Format("OrderID = %i AND OrderDetailsT.Deleted = 0", m_id);
		m_list->PutWhereClause(_bstr_t(strWhere));
		m_list->Requery();


		if(m_bHasAdvInventory) {
			//TES 7/22/2008 - PLID 30802 - Linked Allocations.  We set the FROM clause here for added clarity and use of the enums.
			// We want all allocation details that are tied to one of our order details, are either "To Be Ordered" or are tied to
			// a received detail of ours (in which case they've already been ordered), and aren't on a deleted alllocation.
			CString strQuery;
			strQuery.Format("(SELECT PatientInvAllocationsT.ID, NULL AS ParentID, "
				"PatientInvAllocationsT.ID AS AllocationID, "
				"PatientsT.PersonID, Last + ', ' + First + ' ' + Middle AS PatientName, "
				"CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) + convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS ApptDateTime "
				"FROM PatientInvAllocationsT "
				"INNER JOIN PatientsT ON PatientInvAllocationsT.PatientID = PatientsT.PersonID "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"INNER JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
				"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
				"LEFT JOIN UsersT ON PatientInvAllocationsT.CompletedBy = UsersT.PersonID "
				"WHERE PatientInvAllocationsT.Status <> %li AND PatientInvAllocationsT.ID IN "
				"(SELECT PatientInvAllocationDetailsT.AllocationID FROM PatientInvAllocationDetailsT INNER JOIN OrderDetailsT ON "
				"PatientInvAllocationDetailsT.OrderDetailID = OrderDetailsT.ID "
				"WHERE (PatientInvAllocationDetailsT.Status = %li OR OrderDetailsT.DateReceived Is Not Null) AND OrderDetailsT.OrderID = %li) "
				"UNION "
				"SELECT NULL AS ID, PatientInvAllocationDetailsT.AllocationID AS ParentID, "
				"PatientInvAllocationDetailsT.AllocationID, "
				"PatientInvAllocationsT.PatientID AS PersonID, "
				"ServiceT.Name + ' (' + Convert(nvarchar, Sum(Quantity)) + ')' AS ProductDesc, "
				"NULL AS ApptDateTime "
				"FROM PatientInvAllocationDetailsT "
				"INNER JOIN OrderDetailsT ON PatientInvAllocationDetailsT.OrderDetailID = OrderDetailsT.ID "
				"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
				"INNER JOIN ProductT ON PatientInvAllocationDetailsT.ProductID = ProductT.ID "
				"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
				"WHERE PatientInvAllocationsT.Status <> %li AND (PatientInvAllocationDetailsT.Status = %li OR OrderDetailsT.DateReceived Is Not Null) "
				"AND OrderDetailsT.OrderID = %li "
				"GROUP BY ProductT.ID, ServiceT.Name, PatientInvAllocationDetailsT.AllocationID, PatientInvAllocationsT.PatientID) "
				"AS AllocationsQ", InvUtils::iasDeleted, InvUtils::iadsOrder, m_id, InvUtils::iasDeleted, InvUtils::iadsOrder, m_id);

			m_pLinkedAllocations->PutFromClause(_bstr_t(strQuery));
			RequeryLinkedAllocations();
		}

		// (j.jones 2009-02-16 10:17) - PLID 33085 - this function now has return values
		return TRUE;

	}NxCatchAll("Error in CInvEditOrderDlg::LoadOrder");

	// (j.jones 2009-02-16 10:17) - PLID 33085 - this function now has return values
	return FALSE;
}

void CInvEditOrderDlg::OnDelete() 
//deletes an order - not a line item
{
	if (m_id != -1 && CheckCurrentUserPermissions(bioInvOrder, sptDelete)) {
		
		//DRT 10/2/03 - PLID 9467 - They cannot delete an order that has serialized items that have been adjusted
		//off (marked 'deleted')
		//JMJ 1/27/04 - PLID 10465 - They cannot delete an order that has serialized items that have been billed
		// (j.jones 2007-11-07 10:53) - PLID 27987 - disallow deleting an order that has serialized items that
		// are allocated, even if the allocation is deleted
		//(also I combined these three queries into one)
		//TES 6/18/2008 - PLID 29578 - Changed ProductItemsT.OrderID to ProductItemsT.OrderDetailID.
		// (j.jones 2008-09-10 14:06) - PLID 31320 - ensured that if the product item ID is referenced by another product,
		// you cannot delete the product item
		if(ReturnsRecords("SELECT ID FROM ProductItemsT WHERE OrderDetailID IN (SELECT ID FROM OrderDetailsT WHERE OrderID = %li) AND "
			"(Deleted = 1 "
			" OR ID IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			" OR ID IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
			" OR ID IN (SELECT ReturnedFrom FROM ProductItemsT) "
			" OR ReturnedFrom Is Not Null "
			")", m_id)) {
			MsgBox("There are items on this order that have either been adjusted, billed, returned from, or allocated to patients.\n"
				"You cannot delete this order.");
			return;
		}

		// (j.jones 2009-01-13 17:34) - PLID 26141 - forbid from deleting if referenced in a reconciliation
		if(ReturnsRecords("SELECT TOP 1 ID FROM InvReconciliationProductItemsT WHERE ProductItemID IN (SELECT ID FROM ProductItemsT WHERE OrderDetailID IN (SELECT ID FROM OrderDetailsT WHERE OrderID = %li))", m_id)) {
			MsgBox("There are items on this order that have been referenced in an Inventory Reconciliation.\n"
				"You cannot delete this order.");
			return;
		}

		// (j.jones 2008-03-18 15:30) - PLID 29309 - warn if the order is linked to an appointment
		CString strWarning = "Are you sure you want to cancel this order?";
		_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name "
			"FROM PersonT "
			"INNER JOIN AppointmentsT ON PersonT.ID = AppointmentsT.PatientID "
			"INNER JOIN OrderAppointmentsT ON AppointmentsT.ID = OrderAppointmentsT.AppointmentID "
			"WHERE OrderAppointmentsT.OrderID = {INT}", m_id);
		if(!rs->eof) {
			strWarning.Format("This order is linked to an appointment for patient '%s'.\n\n"
				"Are you sure you want to cancel this order?", AdoFldString(rs, "Name",""));
		}

		if (IDYES == MessageBox(strWarning, "Cancel Order?", MB_YESNO))
		{
			DoDeleteOrder();
		}
	}
	else
		OnCancelBtn();
}

void CInvEditOrderDlg::OnPrintPrev() 
{
	if (!m_changed || IDOK == AfxMessageBox("Before previewing, this order will be saved and closed.", MB_OKCANCEL))
	{
		if(ValidateSaveAndClose()) {
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(245)]);
			infReport.nExtraID = m_id;

			// (j.gruber 2008-02-29 12:39) - PLID 29127 - need to make a parameter for our encryted string
			CPtrArray params;
			CRParameterInfo *tmpParam;
			
			tmpParam = new CRParameterInfo;
			tmpParam->m_Name = "OrderCCNumber";
			tmpParam->m_Data = m_strCreditCardNumber;
			params.Add((void *)tmpParam);

			//Made new function for running reports - JMM 5-28-04
			RunReport(&infReport, &params, true, (CWnd *)this, "Order");

			// (j.jones 2008-05-27 10:19) - PLID 30170 - cleanup the parameter list
			ClearRPIParameterList(&params);
		}
	}

}

// (j.jones 2008-03-03 17:52) - PLID 29181 - added locationID as a parameter
void CInvEditOrderDlg::SaveOrder(long nLocationID)
{	
	long	supplier, 
			user,
			nSoldToLocation,
			nShippingMethodID,
			nCCTypeID,
			nOrderMethodID,
			nVendorConfirmed;		// (d.thompson 2009-01-23) - PLID 32823
	
	CString trackingID, 
			desc,
			notes, 
			extra, 
			taxStr,
			sql,
			strPurchaseOrder,
			strContactName,
			strCCName,
			strCCExpDate,
			strCCNumber,
			strVendorConfNum,		// (d.thompson 2009-01-23) - PLID 32823
			strConfDate;			// (d.thompson 2009-01-23) - PLID 32823

	COleDateTime dt;

	COleDateTime dtExpDate;

	double tax;

	supplier = m_supplier->Value[m_supplier->CurSel][S_ID];
	user = GetCurrentUserID();

	// (j.jones 2008-03-03 17:59) - PLID 29181 - I removed the nLocationID calculation to the 
	// ValidateSaveAndClose function, because ApplyItemsToOrder also needs it

	// (j.jones 2009-01-23 10:30) - PLID 32822 - added order method
	nOrderMethodID = -1;
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOrderMethodCombo->GetCurSel();
		if(pRow) {
			nOrderMethodID = VarLong(pRow->GetValue(omcID), -1);
		}
	}	

	// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
	if (m_pLocationSoldTo->IsComboBoxTextInUse) {
		_RecordsetPtr rsSoldToID = CreateRecordset("SELECT LocationSoldTo FROM OrderT WHERE ID = %li", m_id);

		nSoldToLocation = AdoFldLong(rsSoldToID, "LocationSoldTo");		
	}
	else {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLocationSoldTo->CurSel;
		if (pRow) {
			nSoldToLocation = VarLong(pRow->GetValue(0));
		}
		else {
			//this couldn't happen
			ASSERT(FALSE);
			ThrowNxException("Could not find location ID");
		}
	}

	// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
	if (m_pShipMethodList->IsComboBoxTextInUse && m_id > -1) {
		_RecordsetPtr rsShipMethodID = CreateRecordset("SELECT ShipMethodID FROM OrderT WHERE ID = %li", m_id);
		if (!rsShipMethodID->eof) {
			nShippingMethodID = AdoFldLong(rsShipMethodID, "ShipMethodID", -1);
		}
		else {
			//couldn't find the shipping info, set it to -1
			nShippingMethodID = -1;
		}
	}
	else {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pShipMethodList->CurSel;
		if (pRow) {
			nShippingMethodID = VarLong(pRow->GetValue(0));
		}
		else {
			//this shouldn't happen, but no big deal if it does
			nShippingMethodID = -1;		
		}
	}

	// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
	if (m_pCCTypeList->IsComboBoxTextInUse && m_id > -1) {
		_RecordsetPtr rsCCType = CreateRecordset("SELECT CCTypeID FROM OrderT WHERE ID = %li", m_id);
		if (!rsCCType->eof) {
			nCCTypeID = AdoFldLong(rsCCType, "CCTypeID", -1);
		}
		else {
			//couldn't find CC info, set it to -1
			nCCTypeID = -1;
		}
	}
	else {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCTypeList->CurSel;
		if (pRow) {
			nCCTypeID = VarLong(pRow->GetValue(0));
		}
		else {
			//this shouldn't happen, but could if they set a CC type inactive and chose it on a new order, set it to -1 if they do that
			nCCTypeID = -1;
		}
	}

	// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
	GetDlgItemText(IDC_PURCHASE_ORDER, strPurchaseOrder);
	GetDlgItemText(IDC_ORDER_CONTACT_NAME, strContactName);
	// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
	GetDlgItemText(IDC_ORDER_CC_NAME, strCCName);
	GetDlgItemText(IDC_ORDER_CC_EXP_DATE, strCCExpDate);	

	strCCExpDate.TrimLeft();
	strCCExpDate .TrimRight();
	if (!strCCExpDate.IsEmpty() && strCCExpDate.Find("#") == -1) {
	
		CString strMonth = strCCExpDate.Left(strCCExpDate.Find("/",0));
		CString strYear = "20" + strCCExpDate.Right(strCCExpDate.Find("/",0));
		if(strMonth=="12") {
			//the method we use to store dates acts funky with December, so
			//we cannot just increase the month by 1. However, we know the last
			//day in December is always 31, so it's an easy fix.
			dtExpDate.SetDate(atoi(strYear),atoi(strMonth),31);
		}
		else {
			//this method works well for all other months. Set the date to be
			//the first day of the NEXT month, then subtract one day.
			//The result will always be the last day of the month entered.
			COleDateTimeSpan dtSpan;
			dtSpan.SetDateTimeSpan(1,0,0,0);
			dtExpDate.SetDate(atoi(strYear),atoi(strMonth)+1,1);
			dtExpDate = dtExpDate - dtSpan;
		}
		strCCExpDate = "'" + FormatDateTimeForSql(dtExpDate, dtoDate) + "'";
	}
	else {
		strCCExpDate = "NULL";
	}

	GetDlgItemText(IDC_TRACKING_NUMBER, trackingID);
	GetDlgItemText(IDC_DESCRIPTION, desc);
	GetDlgItemText(IDC_NOTES, notes);
	GetDlgItemText(IDC_EXTRA_COST, extra);
	
	trackingID = trackingID.Left(50);
	desc = desc.Left(50);
	notes = notes.Left(2000);

	CString strDate, strArrivalDate;
	if(m_nxtDate->GetStatus() == 1) {//valid
		strDate = "'" + _Q(FormatDateTimeForSql(m_nxtDate->GetDateTime())) + "'";
	}
	else {
		strDate = "NULL";
	}
	if(m_nxtArrivalDate->GetStatus() == 1) {//valid
		strArrivalDate = "'" + _Q(FormatDateTimeForSql(m_nxtArrivalDate->GetDateTime())) + "'";
	}
	else {
		strArrivalDate = "NULL";
	}

	BOOL bTaxShipping = m_checkTaxShipping.GetCheck();

	GetDlgItemText(IDC_TAX, taxStr);
	taxStr.TrimRight('%');
	tax = atof(taxStr);
	tax = 1.0 + tax / 100.0;

	trackingID = _Q(trackingID);
	desc = _Q(desc);
	notes = _Q(notes);

	// (d.thompson 2009-01-23) - PLID 32823 - Save new fields for vendor confirmation
	if(IsDlgButtonChecked(IDC_VENDOR_CONFIRMED_CHK)) {
		nVendorConfirmed = 1;
	}
	else {
		nVendorConfirmed = 0;
	}

	//regardless of the flag we want the value in here
	GetDlgItemText(IDC_VENDOR_CONF_NUM, strVendorConfNum);

	//regardless of the flag, we need to format a date or NULL
	if(m_nxtDateConfirmed->GetStatus() == 1) {
		strConfDate = "'" + FormatDateTimeForSql(m_nxtDateConfirmed->GetDateTime(), dtoDate) + "'";
	}
	else {
		strConfDate = "NULL";
	}

	if (m_id == -1)//new order
	{	m_id = NewNumber("OrderT", "ID");

		// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
		// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
		// (j.jones 2009-01-23 10:31) - PLID 32822 - added OrderMethodID
		// (d.thompson 2009-01-23) - PLID 32823 - Added VendorConfirmed, ConfirmationDate, ConfirmationNumber
		ExecuteSql ("INSERT INTO OrderT ("
			"ID, Supplier, TrackingID, Date, Description, Notes, "
			"ExtraCost, CreatedBy, LocationID, DateDue, Tax, TaxShipping, "
			" PurchaseOrderNum, LocationSoldTo, ContactName, CCName, CCNumber, "
			" CCTypeID, CCExpDate, ShipMethodID, OrderMethodID, VendorConfirmed,  "
			" ConfirmationDate, ConfirmationNumber "
			") SELECT "
			"%i, %i, \'%s\', %s, \'%s\', \'%s\', "
			"Convert(money,'%s'), %i, %i, %s, %g, %li, "
			"'%s', %li, '%s', '%s', %s, %s, %s, %s, %s, "
			"%li, %s, '%s' "
			, m_id, supplier, trackingID, strDate, desc, notes, 
			_Q(FormatCurrencyForSql(ParseCurrencyFromInterface(extra))), user, nLocationID, strArrivalDate, tax, (bTaxShipping ? 1 : 0),
			_Q(strPurchaseOrder), nSoldToLocation,  _Q(strContactName), _Q(strCCName), EncryptStringForSql(m_strCreditCardNumber),
			nCCTypeID == -1 ? "NULL" : AsString(nCCTypeID),
			strCCExpDate, 
			nShippingMethodID == -1 ? "NULL" : AsString(nShippingMethodID),
			nOrderMethodID == -1 ? "NULL" : AsString(nOrderMethodID), nVendorConfirmed, strConfDate, _Q(strVendorConfNum));
	}
	else
	{	// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
		// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
		// (j.jones 2009-01-23 10:31) - PLID 32822 - added OrderMethodID
		// (d.thompson 2009-01-23) - PLID 32823 - Added VendorConfirmed, ConfirmationDate, ConfirmationNumber
		ExecuteSql ("UPDATE OrderT "
			"SET Supplier = %i, TrackingID = \'%s\', Date = %s, Description = \'%s\', Notes = \'%s\', "
			"ExtraCost = Convert(money,'%s'), LocationID = %i, DateDue = %s, Tax = %g, TaxShipping = %li, "
			" PurchaseOrderNum = '%s', LocationSoldTo = %li, ContactName = '%s', CCName = '%s', "
			" CCNumber = %s, CCTypeID = %s, CCExpDate = %s, ShipMethodID = %s, OrderMethodID = %s, "
			" VendorConfirmed = %li, ConfirmationDate = %s, ConfirmationNumber = '%s' "
			"WHERE ID = %i",
			supplier, trackingID, strDate, desc, notes, 
			_Q(FormatCurrencyForSql(ParseCurrencyFromInterface(extra))), nLocationID, strArrivalDate, tax, (bTaxShipping ? 1 : 0),
			_Q(strPurchaseOrder), nSoldToLocation,  _Q(strContactName), _Q(strCCName), EncryptStringForSql(m_strCreditCardNumber),
			nCCTypeID == -1 ? "NULL" : AsString(nCCTypeID),
			strCCExpDate, 
			nShippingMethodID == -1 ? "NULL" : AsString(nShippingMethodID),
			nOrderMethodID == -1 ? "NULL" : AsString(nOrderMethodID), nVendorConfirmed, strConfDate, _Q(strVendorConfNum),
			m_id);
	}

	// (j.jones 2008-03-18 17:37) - PLID 29309 - save the linked appt. info. if it has changed
	if(m_nApptID != m_nOrigLinkedApptID) {

		if(m_nOrigLinkedApptID == -1 && m_nApptID != -1) {
			//create the link
			ExecuteParamSql("INSERT INTO OrderAppointmentsT (OrderID, AppointmentID) VALUES ({INT},{INT})", m_id, m_nApptID);

			// (j.jones 2008-03-19 11:38) - PLID 29316 - audit this
			CString strOld;
			strOld.Format("Order: %s, <No Linked Appointment>", desc);
			
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiInvOrderApptLinkCreated, m_id, strOld, m_strApptLinkText, aepMedium, aetCreated);
		}
		else if(m_nOrigLinkedApptID != -1 && m_nApptID == -1) {
			//delete the link			
			ExecuteParamSql("DELETE FROM OrderAppointmentsT WHERE OrderID = {INT}", m_id);

			// (j.jones 2008-03-19 11:38) - PLID 29316 - audit this
			CString strOld;
			strOld.Format("Order: %s, %s", desc, m_strOldApptLinkText);

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiInvOrderApptLinkDeleted, m_id, strOld, "<No Linked Appointment>", aepMedium, aetDeleted);
		}
		else {
			//change the link
			ExecuteParamSql("UPDATE OrderAppointmentsT SET AppointmentID = {INT} WHERE OrderID = {INT}", m_nApptID, m_id);

			// (j.jones 2008-03-19 11:38) - PLID 29316 - audit this			
			CString strOld;
			strOld.Format("Order: %s, %s", desc, m_strOldApptLinkText);
			
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiInvOrderApptLinkChanged, m_id, strOld, m_strApptLinkText, aepMedium, aetChanged);
		}

		m_nOrigLinkedApptID = m_nApptID;
		// (j.jones 2008-03-19 11:50) - PLID 29316 - cache the resulting text for auditing
		m_strOldApptLinkText = m_strApptLinkText;
	}
}

// (j.jones 2008-03-03 17:52) - PLID 29181 - added locationID as a parameter
// (j.jones 2008-03-20 12:16) - PLID 29311 - now returns TRUE if we just completely received the order
//TES 7/23/2008 - PLID 30802 - This now outputs a list of allocation IDs that got updated, in order to update the user
// (outside of the transaction).
// (j.jones 2008-09-26 10:49) - PLID 30636 - takes in nAddToAllocationID, and if not -1, will
// add all products into the allocation if they were not previously received in a prior save
// (a.walling 2010-01-21 16:00) - PLID 37024
BOOL CInvEditOrderDlg::ApplyItemsToOrder(long nLocationID, OUT CArray<long,long> &arUpdatedAllocationIDs, long nAddToAllocationID, CString strAllocPatientName, long nAllocPatientID, long nAuditTransactionID)
{
	//TES 7/23/2008 - PLID 30802 - Why is this try/catch block here?  The code is called from within a transaction, but
	// this block was causing it to get committed even if there were errors saving the details!
	//try {
		
		//delete deleted items
		if (m_aryOrderDetailsToDelete.size() > 0)
		{
			//safety check: don't delete any items that had received product items
			//this is a second check incase someone did this while you took forever entering serial numbers
			if (InvUtils::HasOrderDetailReceivedProductItems(m_aryOrderDetailsToDelete)) {
				//someone has already received a product
				AfxMessageBox("At least one product you deleted has already been received. You may need to close this window and attempt editing the order again to ensure the order status has been updated.", MB_ICONERROR | MB_OK);
				return FALSE;
			}

			ExecuteParamSql("UPDATE OrderDetailsT SET Deleted = 1, ModifiedBy = {INT} "
				"WHERE OrderID = {INT} AND Deleted = 0 AND ID IN ({INTVECTOR})", GetCurrentUserID(), m_id, m_aryOrderDetailsToDelete);

			//TES 7/22/2008 - PLID 30802 - Clear out any allocation details that happen to have been linked to the details 
			// we just deleted
			ExecuteParamSql("UPDATE PatientInvAllocationDetailsT SET OrderDetailID = NULL WHERE OrderDetailID IN (SELECT ID FROM "
				"OrderDetailsT WHERE Deleted = 1 AND ID IN ({INTVECTOR}))", m_aryOrderDetailsToDelete);
		}

	
		//next update existing records
		_RecordsetPtr rs;
		long row;
		_variant_t var;
		_variant_t varNull;
		varNull.ChangeType(VT_NULL);

		BOOL bFullyReceived = TRUE;
		BOOL bMarkedItemsReceived = FALSE;

		//TES 7/23/2008 - PLID 30802 - We need to track all the details that we receive, so that we can then update any
		// associated allocations.
		CArray<InvUtils::ReceivedOrderDetailInfo,InvUtils::ReceivedOrderDetailInfo&> arReceivedDetails;

		rs = CreateParamRecordset("SELECT ID, DateReceived FROM OrderDetailsT WHERE OrderID = {INT} AND Deleted = 0", m_id);
		while (!rs->eof)
		{
			BOOL bReceived = FALSE;

			if(GetFieldValue(rs, "DateReceived").vt == VT_DATE)
				bReceived = TRUE;

			long DetailID = rs->Fields->Item["ID"]->Value.lVal;
			CString idStr;
			idStr.Format("%i", DetailID);
			row = m_list->SearchByColumn(O_ID, _bstr_t(idStr), 0, 0);

			if(row == -1) {
				rs->MoveNext();
				continue;
			}

			CString catalog = VarString(m_list->Value[row][O_Catalog],"");

			long nUseUU = VarLong(m_list->GetValue(row, O_UseUU), 0);
			long nConversion = VarLong(m_list->GetValue(row, O_Conversion), 1);
			
			//get the correct quantity
			long qty = VarLong(m_list->Value[row][O_Quantity],0);
			if(nUseUU != 0) {
				 qty *= nConversion;
			}

			//get the date received, and receive product items as needed
			CString strDateReceived = "NULL";

			var = m_list->Value[row][O_DateReceived];
			if (var.vt == VT_DATE) {
				// (c.haag 2008-01-10 17:44) - PLID 28592 - Added the status flag to the call to ReceiveProductItems
				//TES 6/18/2008 - PLID 29578 - ReceiveProductItems now takes an OrderDetailID rather than an OrderID
				if(!bReceived && !ReceiveProductItems(DetailID, VarLong(m_list->Value[row][O_ProductID]),qty,VarLong(m_list->Value[row][O_ForStatus]))) {
					_RecordsetPtr rsProdName = CreateRecordset("SELECT Name FROM ServiceT WHERE ID = %li",VarLong(m_list->Value[row][O_ProductID]));
					CString str = "This item will not be marked received. You cannot receive this item without entering the required information.";
					if(!rsProdName->eof) {
						str.Format("'%s' will not be marked received. You cannot receive '%s' without entering the required information.",CString(rsProdName->Fields->Item["Name"]->Value.bstrVal),CString(rsProdName->Fields->Item["Name"]->Value.bstrVal));
					}
					rsProdName->Close();
					AfxMessageBox(str);
				}
				else {
					strDateReceived = "'" + FormatDateTimeForSql(COleDateTime(var.date), dtoDate) + "'";

					// (j.jones 2008-03-20 12:36) - PLID 29311 - track that we marked at least one item received
					if(!bReceived) {
						bMarkedItemsReceived = TRUE;
					}
				}
			}
			else {
				// (j.jones 2008-03-20 12:36) - PLID 29311 - track that this order is not fully received
				bFullyReceived = FALSE;
			}

			COleCurrency cyExtraCost = COleCurrency(0,0);
			if(m_list->Value[row][O_ExtraCost].vt == VT_CY) {
				cyExtraCost = m_list->Value[row][O_ExtraCost].cyVal;
			}

			// (j.jones 2008-06-19 11:07) - PLID 10394 - added discount fields
			double dblPercentOff = VarDouble(m_list->GetValue(row, O_PercentOff), 0.0);
			COleCurrency cyDiscount = VarCurrency(m_list->GetValue(row, O_Discount), COleCurrency(0,0));

			if (DetailID != -1) {
				//safety checks: make sure we're not reducing the quantity ordered or clearing the date received
				//of product that has already been received
				_RecordsetPtr prs = CreateParamRecordset("SELECT OrderDetailsT.QuantityOrdered "
					"FROM OrderDetailsT "
					"LEFT JOIN ProductItemsT ON OrderDetailsT.ID = ProductItemsT.OrderDetailID "
					"WHERE OrderDetailsT.ID = {INT} "
					"AND (DateReceived Is Not Null OR ProductItemsT.OrderDetailID Is Not Null)", DetailID);
				if (!prs->eof) {
					if (strDateReceived == "NULL") {
						//you can't mark an item unreceived if it has already been received
						CString strMessage;
						strMessage.Format("The product %s has already been received and cannot be marked pending. You may need to close this window and attempt receiving the order again to ensure the order status has been updated.", VarString(m_list->GetValue(row, O_Name)));
						AfxMessageBox(strMessage, MB_ICONERROR | MB_OK);
						return FALSE;
					}
					else if (qty < VarLong(prs->Fields->Item["QuantityOrdered"]->Value, 0)) {
						//you can't reduce an item's quantity ordered if it has already been received
						CString strMessage;
						strMessage.Format("The product %s has already been received and cannot have its ordered quantity reduced. You may need to close this window and attempt to edit the order again to ensure the order status has been updated.", VarString(m_list->GetValue(row, O_Name)));
						AfxMessageBox(strMessage, MB_ICONERROR | MB_OK);
						return FALSE;
					}
				}
				prs->Close();
			}

			//now update
			//DRT 11/8/2007 - PLID 27984 - Save the 'For Consignment' value.
			// (c.haag 2007-12-03 11:06) - PLID 28204 - ForConsignment is now ForStatus
			ExecuteSql("UPDATE OrderDetailsT SET ProductID = %li, Catalog = '%s', QuantityOrdered = %li, "
				"Amount = Convert(money,'%s'), ExtraCost = Convert(money,'%s'), "
				"PercentOff = %g, Discount = Convert(money,'%s'), "
				"DateReceived = %s, ModifiedBy = %li, UseUU = %li, Conversion = %li, ForStatus = %li "
				"WHERE ID = %li",
				m_list->Value[row][O_ProductID].lVal,_Q(catalog),qty,
				FormatCurrencyForSql(m_list->Value[row][O_Amount]), FormatCurrencyForSql(cyExtraCost),
				dblPercentOff, FormatCurrencyForSql(cyDiscount),
				strDateReceived,GetCurrentUserID(),
				nUseUU, nConversion, VarLong(m_list->Value[row][O_ForStatus]), DetailID);

			//if the update was successful and a product was received, 
			//and it hasn't been received before, update the last cost
			if(!bReceived && strDateReceived != "NULL") {
				//update the last cost
				InvUtils::TryUpdateLastCostByProduct(
					VarLong(m_list->Value[row][O_ProductID],-1),
					VarCurrency(m_list->Value[row][O_Amount],COleCurrency(0,0)),
					VarDateTime(m_list->Value[row][O_DateReceived],COleDateTime::GetCurrentTime()));
			}

			// (j.jones 2008-09-26 11:03) - PLID 30636 - add these products to the linked allocation,
			// provided that they have either not been received, or were only received during this save.
			if(nAddToAllocationID != -1
				&& (strDateReceived == "NULL" || (!bReceived && strDateReceived != "NULL"))) {

				//see if we need to insert it multiple times
				BOOL bHasSerial = VarBool(m_list->Value[row][O_HasSerialNum], FALSE);
				BOOL bHasExp = VarBool(m_list->Value[row][O_HasExpDate], FALSE);
				BOOL bIsSerialized = FALSE;
				if(bHasSerial || bHasExp) {
					bIsSerialized = TRUE;
				}

				long nRecordsToCreate = 1;
				if(bIsSerialized) {
					nRecordsToCreate = qty;
				}

				InsertOrderDetailIntoAllocation(DetailID, nAddToAllocationID, bIsSerialized, nRecordsToCreate,
					VarString(m_list->Value[row][O_Name], ""), strAllocPatientName, nAllocPatientID, bIsSerialized ? 1 : qty, nAuditTransactionID);

				//track that we updated the allocation
				bool bMatched = false;
				for(int nAllocation = 0; nAllocation < arUpdatedAllocationIDs.GetSize() && !bMatched; nAllocation++) {
					if(arUpdatedAllocationIDs[nAllocation] == nAddToAllocationID) {
						bMatched = true;
					}
				}
				if(!bMatched) {
					arUpdatedAllocationIDs.Add(nAddToAllocationID);
				}
			}

			//DRT 11/13/2007 - PLID 28052 - Reworked the auditing.  Previously we audited in the OnEditingFinished, but that didn't respect
			//	cancel behavior!  So I'm moving it here.  We only audit the received date if indeed they set it, or it has changed.
			//There are 2 possible audits -- (1) that we received an item, (2) that they changed the date.
			if(!bReceived && strDateReceived != "NULL") {

				//TES 7/23/2008 - PLID 30802 - Track this detail so that we can update linked allocations.
				InvUtils::ReceivedOrderDetailInfo rodi;
				rodi.nOrderDetailID = DetailID;
				rodi.nQuantity = qty;
				rodi.nSourceOrderDetailID = VarLong(m_list->Value[row][O_SourceDetailID], -1);
				rodi.bReceived = true;
				arReceivedDetails.Add(rodi);

				//Not previously received, so audit the quantity, since it's new
				long nAuditID = BeginNewAuditEvent();
				CString strAudit;
				strAudit.Format("Received quantity %li for date '%s'.", VarLong(m_list->Value[row][O_Quantity]), FormatDateTimeForInterface(VarDateTime(m_list->Value[row][O_DateReceived]), NULL, dtoDate));
				AuditEvent(-1, VarString(m_list->Value[row][O_Name]), nAuditID, aeiOrderReceived, VarLong(m_list->Value[row][O_ID]), "", strAudit, aepMedium, aetChanged);
			}
			else if(bReceived && m_list->Value[row][O_DateReceived] != rs->Fields->Item["DateReceived"]->Value) {
				//It was previously received, but the value changed!
				long nAuditID = BeginNewAuditEvent();
				CString strOld = "<None>", strNew = "Pending";
				_variant_t varOld = rs->Fields->Item["DateReceived"]->Value;
				_variant_t varNew = m_list->Value[row][O_DateReceived];
				if(varOld.vt == VT_DATE) {
					COleDateTime dt = VarDateTime(varOld);
					strOld = FormatDateTimeForInterface(dt, NULL, dtoDate);
				}
				if(varNew.vt == VT_DATE) {
					COleDateTime dt = VarDateTime(varNew);
					strNew = FormatDateTimeForInterface(dt, NULL, dtoDate);
				}

				CString strNewAudit;
				strNewAudit.Format("Date changed to '%s' for quantity %li.", strNew, VarLong(m_list->Value[row][O_Quantity]));
				AuditEvent(-1, VarString(m_list->Value[row][O_Name]), nAuditID, aeiOrderReceived, VarLong(m_list->Value[row][O_ID]), strOld, strNewAudit, aepMedium, aetChanged);
			}


			rs->MoveNext();
		}
		rs->Close();

		//last, add new items
		for (int i = 0; i < m_list->GetRowCount(); i++)	{
			if (m_list->Value[i][O_ID].lVal == -1) {

				CString catalog = VarString(m_list->Value[i][O_Catalog],"");

				long nUseUU = VarLong(m_list->GetValue(i, O_UseUU), 0);
				long nConversion = VarLong(m_list->GetValue(i, O_Conversion), 1);
				
				//get the correct quantity
				long qty = VarLong(m_list->Value[i][O_Quantity],0);
				if(nUseUU != 0) {
					 qty *= nConversion;
				}

				//TES 6/18/2008 - PLID 29578 - We need to create the OrderDetailsT record first, so that we can pass the ID
				// in to the ProductItems dialog, now that ProductItemsT has an OrderDetailID, rather than an OrderID.  Set
				// the DateReceived to NULL for the moment.
				COleCurrency cyExtraCost = COleCurrency(0,0);
				if(m_list->Value[i][O_ExtraCost].vt == VT_CY) {
					cyExtraCost = m_list->Value[i][O_ExtraCost].cyVal;
				}

				// (j.jones 2008-06-19 11:07) - PLID 10394 - added discount fields
				double dblPercentOff = VarDouble(m_list->GetValue(i, O_PercentOff), 0.0);
				COleCurrency cyDiscount = VarCurrency(m_list->GetValue(i, O_Discount), COleCurrency(0,0));

				long nOrderDetailID = NewNumber("OrderDetailsT","ID");
				//DRT 11/8/2007 - PLID 27984 - Save the 'For Consignment' value.
				// (c.haag 2007-12-03 11:07) - PLID 28204 - This is now 'For Status'
				ExecuteSql("INSERT INTO OrderDetailsT (ID, OrderID, ProductID, Catalog, QuantityOrdered, "
					"Amount, ExtraCost, PercentOff, Discount, "
					"DateReceived, ModifiedBy, Conversion, UseUU, ForStatus) "
					"VALUES (%li, %li, %li, '%s', %li, "
					"Convert(money,'%s'), Convert(money,'%s'), %g, Convert(money,'%s'), "
					"NULL, %li, %li, %li, %li)",
					nOrderDetailID, m_id, VarLong(m_list->GetValue(i ,O_ProductID)), _Q(catalog), qty,
					FormatCurrencyForSql(VarCurrency(m_list->GetValue(i, O_Amount))), FormatCurrencyForSql(cyExtraCost),
					dblPercentOff, FormatCurrencyForSql(cyDiscount),
					GetCurrentUserID(), nConversion, nUseUU, 
					VarLong(m_list->GetValue(i ,O_ForStatus)));

				//TES 7/22/2008 - PLID 30802 - We need to update any Allocation Details which we were created from, so that
				// they now point to us, assuming they are still flagged "To Be Ordered"
				long nAllocationDetailListID = VarLong(m_list->Value[i][O_AllocationDetailListID], -1);
				if(nAllocationDetailListID != -1) {
					CString strIDList;
					for(int i = 0; i < m_arAllocationDetailIDs[nAllocationDetailListID]->GetSize(); i++) {
						strIDList += FormatString("%li,", m_arAllocationDetailIDs[nAllocationDetailListID]->GetAt(i));
					}
					strIDList.TrimRight(",");
					ASSERT(!strIDList.IsEmpty());
					ExecuteSql("UPDATE PatientInvAllocationDetailsT SET OrderDetailID = %li "
						"WHERE Status = %i AND ID IN (%s)", 
						nOrderDetailID, InvUtils::iadsOrder, strIDList);
				}

				// (j.jones 2008-09-26 11:03) - PLID 30636 - add these products to the linked allocation
				if(nAllocationDetailListID == -1 && nAddToAllocationID != -1) {

					//see if we need to insert it multiple times
					BOOL bHasSerial = VarBool(m_list->Value[i][O_HasSerialNum], FALSE);
					BOOL bHasExp = VarBool(m_list->Value[i][O_HasExpDate], FALSE);
					BOOL bIsSerialized = FALSE;
					if(bHasSerial || bHasExp) {
						bIsSerialized = TRUE;
					}

					long nRecordsToCreate = 1;
					if(bIsSerialized) {
						nRecordsToCreate = qty;
					}

					InsertOrderDetailIntoAllocation(nOrderDetailID, nAddToAllocationID, bIsSerialized, nRecordsToCreate,
						VarString(m_list->Value[i][O_Name], ""), strAllocPatientName, nAllocPatientID, bIsSerialized ? 1 : qty, nAuditTransactionID);

					//track that we updated the allocation
					bool bMatched = false;
					for(int nAllocation = 0; nAllocation < arUpdatedAllocationIDs.GetSize() && !bMatched; nAllocation++) {
						if(arUpdatedAllocationIDs[nAllocation] == nAddToAllocationID) {
							bMatched = true;
						}
					}
					if(!bMatched) {
						arUpdatedAllocationIDs.Add(nAddToAllocationID);
					}
				}
				
				//get the date received, and receive product items as needed
				CString strDateReceived = "NULL";

				//TES 7/23/2008 - PLID 30802 - Track which ProductItemsT records we add, so that we can update linked allocations
				// accordingly.
				CArray<long,long> arProductItemsReceived;
				var = m_list->Value[i][O_DateReceived];
				if (var.vt == VT_DATE) {
					// (c.haag 2007-12-06 12:42) - PLID 28286 - If this is the kind of order where the user scans
					// all the serialized items in all at once, don't call ReceiveProductItems because we already
					// have the serialized information in memory
					if (m_bSaveAllAsReceived) {

						CProductItemsDlg* pDlg = (CProductItemsDlg*)VarLong(m_list->Value[i][O_DlgPtr], NULL);

						// (a.walling 2008-03-20 11:37) - PLID 29333 - Ensure data is available for all the product items
						BOOL bSuccess = TRUE;
						if (NULL != pDlg) {
							for(int nProductItem = 0; nProductItem < pDlg->m_adwProductItemIDs.GetSize(); nProductItem++) {
								arProductItemsReceived.Add(pDlg->m_adwProductItemIDs[nProductItem]);
							}

							// If we get here, it's definitely a serialized item. Update the order ID
							// and commit the save to data
							pDlg->m_bSaveDataEntryQuery = false;
							pDlg->m_strSavedDataEntryQuery = ""; // (j.jones 2009-07-09 17:59) - PLID 34842 - make sure this gets cleared!
							//TES 6/18/2008 - PLID 29578 - This now takes an OrderDetailID rather than an OrderID
							pDlg->m_nOrderDetailID = nOrderDetailID;
							bSuccess = pDlg->Save(TRUE /* Quiet save */);
						}

						if (bSuccess) {
							strDateReceived = "'" + FormatDateTimeForSql(COleDateTime(var.date), dtoDate) + "'";

							// (j.jones 2008-03-20 12:36) - PLID 29311 - track that we marked at least one item received
							bMarkedItemsReceived = TRUE;

						} else {
							_RecordsetPtr rsProdName = CreateRecordset("SELECT Name FROM ServiceT WHERE ID = %li",VarLong(m_list->Value[i][O_ProductID]));
							CString str = "This item will not be marked received. You cannot receive this item without entering the required information.";
							if(!rsProdName->eof) {
								str.Format("'%s' will not be marked received. You cannot receive '%s' without entering the required information.",CString(rsProdName->Fields->Item["Name"]->Value.bstrVal),CString(rsProdName->Fields->Item["Name"]->Value.bstrVal));
							}
							rsProdName->Close();
							AfxMessageBox(str);
						}
					}
					// (c.haag 2008-01-10 17:44) - PLID 28592 - Added the status flag to the call to ReceiveProductItems
					//TES 6/18/2008 - PLID 29578 - ReceiveProductItems now takes an OrderDetailID rather than an OrderID.
					else if(!ReceiveProductItems(nOrderDetailID, VarLong(m_list->Value[i][O_ProductID]),qty,VarLong(m_list->Value[i][O_ForStatus]))) {
						_RecordsetPtr rsProdName = CreateRecordset("SELECT Name FROM ServiceT WHERE ID = %li",VarLong(m_list->Value[i][O_ProductID]));
						CString str = "This item will not be marked received. You cannot receive this item without entering the required information.";
						if(!rsProdName->eof) {
							str.Format("'%s' will not be marked received. You cannot receive '%s' without entering the required information.",CString(rsProdName->Fields->Item["Name"]->Value.bstrVal),CString(rsProdName->Fields->Item["Name"]->Value.bstrVal));
						}
						rsProdName->Close();
						AfxMessageBox(str);
					}
					else {
						strDateReceived = "'" + FormatDateTimeForSql(COleDateTime(var.date), dtoDate) + "'";

						// (j.jones 2008-03-20 12:36) - PLID 29311 - track that we marked at least one item received
						bMarkedItemsReceived = TRUE;
					}
				}
				else {
					// (j.jones 2008-03-20 12:36) - PLID 29311 - track that this order is not fully received
					bFullyReceived = FALSE;
				}

				//TES 6/18/2008 - PLID 29578 - If the DateReceived should be something other than NULL, update it now.
				if(strDateReceived != "NULL") {
					ExecuteSql("UPDATE OrderDetailsT SET DateReceived = %s WHERE ID = %li", strDateReceived, nOrderDetailID);

					//throw an exception if we are about to create more ProductItemsT records than we ordered
					if (ReturnsRecordsParam("SELECT Count(*) AS TotalCount, QuantityOrdered, OrderDetailID "
						"FROM ProductItemsT "
						"INNER JOIN OrderDetailsT ON ProductItemsT.OrderDetailID = OrderDetailsT.ID "
						"WHERE ProductItemsT.OrderDetailID = {INT} AND ProductItemsT.Deleted = 0 "
						"GROUP BY QuantityOrdered, OrderDetailID "
						"HAVING Count(*) > QuantityOrdered", nOrderDetailID))
					{
						//this shouldn't be likely unless two people are doing something crazy like a partial receive at the same time on the same item
						ThrowNxException(FormatString("Error saving order, more product items were saved for OrderDetailID %li than were ordered.", nOrderDetailID));
					}
				}


				if(strDateReceived != "NULL") {
					//the TryUpdateLastCostByProduct function did have these values embedded,
					//but it caused an internal compiler error. Weird!

					const COleCurrency cyZero = COleCurrency(0,0);
					const COleDateTime dtNow = COleDateTime::GetCurrentTime();
					
					//update the last cost
					InvUtils::TryUpdateLastCostByProduct(
						VarLong(m_list->Value[i][O_ProductID],-1),
						VarCurrency(m_list->Value[i][O_Amount], cyZero),
						VarDateTime(m_list->Value[i][O_DateReceived],dtNow));

					//DRT 11/13/2007 - PLID 28052 - Need to audit the 'date received' value.
					long nAuditID = BeginNewAuditEvent();
					CString strAudit;
					strAudit.Format("Received quantity %li for date '%s'.", VarLong(m_list->Value[i][O_Quantity]), FormatDateTimeForInterface(VarDateTime(m_list->Value[i][O_DateReceived]), NULL, dtoDate));
					AuditEvent(-1, VarString(m_list->Value[i][O_Name]), nAuditID, aeiOrderReceived, VarLong(m_list->Value[i][O_ID]), "", strAudit, aepMedium, aetChanged);

					//TES 7/23/2008 - PLID 30802 - Track this received detail, in order to update linked allocations.
					InvUtils::ReceivedOrderDetailInfo rodi;
					rodi.nOrderDetailID = nOrderDetailID;
					rodi.nQuantity = qty;
					rodi.nSourceOrderDetailID = VarLong(m_list->Value[i][O_SourceDetailID], -1);
					rodi.bReceived = true;
					arReceivedDetails.Add(rodi);
				}
			}
		}

		//to do list
		{
			// (c.haag 2008-02-07 13:17) - PLID 28853 - Renamed from ChargeInventoryQuantity to EnsureInventoryTodoAlarms
			// because that's a closer description to what it actually does. Also removed unused quantity parameter.
			// (j.jones 2008-03-03 17:51) - PLID 29181 - to ensure that we use the order location, and not the logged in location,
			// I made the Order's location ID be a parameter to this function
			// (j.jones 2008-09-16 09:30) - PLID 31380 - now EnsureInventoryTodoAlarms supports running for all products
			// in one call
			CArray<long, long> aryProductIDs;
			for (i = 0; i < m_list->GetRowCount(); i++) {
				aryProductIDs.Add(VarLong(m_list->Value[i][O_ProductID]));
			}

			rs = CreateParamRecordset("SELECT ProductID "
				"FROM OrderDetailsT INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
				"WHERE OrderID = {INT} AND OrderDetailsT.Deleted = 1", m_id);
			while (!rs->eof)
			{
				aryProductIDs.Add(AdoFldLong(rs, "ProductID"));
				rs->MoveNext();
			}

			if(aryProductIDs.GetSize() > 0) {
				//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
				InvUtils::EnsureInventoryTodoAlarms(aryProductIDs, nLocationID, true);
			}
		}


		//TES 7/23/2008 - PLID 30802 - Now pass all that received detail information we tracked into 
		// UpdateLinkedAllocationDetails(), which will take care of everything for us.
		// (j.jones 2008-09-26 11:05) - PLID 30636 - if we added into an allocation during this save,
		// this function will also handle the newly updated allocation
		InvUtils::UpdateLinkedAllocationDetails(arReceivedDetails, arUpdatedAllocationIDs);

		// (j.jones 2008-03-20 12:30) - PLID 29311 - bMarkedItemsReceived will represent
		// that we just now received at least one item, and bFullyReceived indicates that
		// the entire order has been received. This, we return false if we merely edited
		// an already-received order
		return (bFullyReceived && bMarkedItemsReceived);

	//TES 7/23/2008 - PLID 30802 - Why is this try/catch block here?  The code is called from within a transaction, but
	// this block was causing it to get committed even if there were errors saving the details!
	//}NxCatchAll("Error in ApplyItemsToOrder()");

	return FALSE;
}

void CInvEditOrderDlg::OnOkBtn() 
{
  ValidateSaveAndClose();
}

void CInvEditOrderDlg::OnCancel() 
{
	try {
		

		// (c.haag 2007-12-06 09:23) - PLID 28286 - Go through all the rows and delete
		// product item dialogs in they exist
		if (NULL != m_list) {
			const long nRows = m_list->GetRowCount();
			for (long i=0; i < nRows; i++) {
				CProductItemsDlg* pDlg = (CProductItemsDlg*)VarLong(m_list->Value[i][O_DlgPtr], NULL);
				if (NULL != pDlg) {
					_variant_t vNull;
					vNull.vt = VT_NULL;
					m_list->Value[i][O_DlgPtr] = vNull;
					pDlg->DestroyWindow();
					delete pDlg;
				}
			}
		}

		//TES 6/13/2008 - PLID 28078 - We are now sometimes called modally, so actually cancel in that case.
		//r.wilson 4/13/2012 PLID 47393  - ... Also if this is a frame reception we need to close it
		if(m_bIsModal || m_bFrameOrderReception) {
			//r.wilson PLID 47393
			m_bFrameOrderReception = FALSE;

			CNxDialog::OnCancel();
		}
		else {
			GetMainFrame()->EnableWindow(TRUE);
			ShowWindow(SW_HIDE);
		}
		
	}
	NxCatchAll("Error in CInvEditOrderDlg::OnCancel");
}

void CInvEditOrderDlg::OnCancelBtn() 
{
	OnCancel();
}

void CInvEditOrderDlg::OnDefault() 
{
	try {

		if (m_taxChecker.Changed() || m_locationChecker.Changed())
		{	
			long id;
			bool bUseText;
			_bstr_t strText;
			bUseText = m_location->IsComboBoxTextInUse == 0 ? false : true;
			if(bUseText) strText = m_location->ComboBoxText;
			else id = VarLong(m_location->Value[m_location->CurSel][0]);
			
			m_location->Requery();
			if(bUseText) {
				m_location->PutComboBoxText(strText);
			}
			else {
				m_location->SetSelByColumn(0, id);
			}
		}

		DOUBLE dTax = 1;
		if(m_location->IsComboBoxTextInUse) {
			_RecordsetPtr rsTax = CreateRecordset("SELECT TaxRate FROM LocationsT WHERE ID = (SELECT LocationID FROM OrderT WHERE ID = %li)", m_id);
			if(!rsTax->eof) {
				dTax = AdoFldDouble(rsTax, "TaxRate", 1);
			}
		}
		else {
			dTax = VarDouble(m_location->Value[m_location->CurSel][2]);
		}
		SetTax(dTax);
		UpdateTotal();

	}NxCatchAll("Error in OnDefault()");
}

// (j.jones 2008-02-07 16:47) - PLID 28851 - added bIsConsignment
// (j.jones 2008-02-08 14:12) - PLID 28851 - scratch that, we have now removed this functionality
/*
void CInvEditOrderDlg::AddToItemOnOrder(long productID, long quantity, BOOL bIsConsignment)
//maintains a correct on order count for items not yet saved
{
	try {
	
		long row = m_item->FindByColumn(I_ID, productID, 0, FALSE);
		if (row != -1 && VarLong(m_item->Value[row][I_TrackableStatus]) == 2) {
			// (j.jones 2008-02-07 12:28) - PLID 28851 - need to update based on the consignment status
			if(bIsConsignment) {
				quantity += VarLong(m_item->Value[row][I_OrderedConsign]);
				m_item->Value[row][I_OrderedConsign] = quantity;
			}
			else {
				//all other statii use the general values
				quantity += VarLong(m_item->Value[row][I_OrderedPurchased]);
				m_item->Value[row][I_OrderedPurchased] = quantity;
			}
			UpdateItemListColors();
		}

	}NxCatchAll("Error in AddToItemOnOrder()");
}
*/

// (j.jones 2008-02-07 16:47) - PLID 28851 - added bIsConsignment
// (j.jones 2008-02-08 14:12) - PLID 28851 - scratch that, we have now removed this functionality
/*
void CInvEditOrderDlg::AddToItemReceived(long productID, long quantity, BOOL bIsConsignment)
//maintains a correct on order count for items received
{
	try {
	
		long row = m_item->FindByColumn(I_ID, productID, 0, FALSE);
		if (row != -1 && VarLong(m_item->Value[row][I_TrackableStatus]) == 2)
		{
			// (j.jones 2008-02-07 12:28) - PLID 28851 - need to update based on the consignment status
			if(bIsConsignment) {
				double Actual = quantity + VarDouble(m_item->Value[row][I_ActualConsign],0.0);
				long Ordered = VarLong(m_item->Value[row][I_OrderedConsign]) - quantity;
				m_item->Value[row][I_OrderedConsign] = Ordered;
				m_item->Value[row][I_ActualConsign] = Actual;

				//DRT 2/8/2008 - PLID 28854 - Do the same for avail fields
				double dblAvail = quantity + VarDouble(m_item->Value[row][I_AvailConsign], 0.0);
				m_item->Value[row][I_AvailConsign] = dblAvail;
			}
			else {
				//all other statii use the general values
				double Actual = quantity + VarDouble(m_item->Value[row][I_ActualPurchased],0.0);
				long Ordered = VarLong(m_item->Value[row][I_OrderedPurchased]) - quantity;
				m_item->Value[row][I_OrderedPurchased] = Ordered;
				m_item->Value[row][I_ActualPurchased] = Actual;

				//DRT 2/8/2008 - PLID 28854 - Do the same for avail fields
				double dblAvail = quantity + VarDouble(m_item->Value[row][I_AvailPurchased], 0.0);
				m_item->Value[row][I_AvailPurchased] = dblAvail;
			}			
			UpdateItemListColors();
		}

	}NxCatchAll("Error in AddToItemReceived()");
}
*/

// (j.jones 2008-02-08 14:13) - PLID 28851 - this function has been removed,
// we no longer dynamically update the product totals
/*
void CInvEditOrderDlg::RemoveUnsavedItemFromOrder(long row)
//this function has to reference the list by row id - it is the only unique datalist field (unsaved items are all ID -1)
{
	try {

		// (j.jones 2008-02-07 16:47) - PLID 28851 - track whether this was consignment
		BOOL bIsConsignment = VarLong(m_list->Value[row][O_ForStatus]) == InvUtils::odsConsignment;
		
		AddToItemOnOrder(							//we can add a negative to subtract
			VarLong(m_list->Value[row][O_ProductID]), 
			0 - VarLong(m_list->Value[row][O_Quantity]),
			bIsConsignment);

		//if received
		if(m_list->GetValue(row,O_DateReceived).vt == VT_DATE) {
			AddToItemReceived(VarLong(m_list->Value[row][O_ProductID]), 
				0 - VarLong(m_list->Value[row][O_Quantity]),
				bIsConsignment);
		}

	}NxCatchAll("Error in RemoveUnsavedItemFromOrder()");
}
*/

// (j.jones 2008-02-08 14:13) - PLID 28851 - this function has been removed,
// we no longer dynamically update the product totals
/*
void CInvEditOrderDlg::AddUnsavedItemsToItemsOnOrder()
{
	try {
	
		for (int i = 0; i < m_list->GetRowCount(); i++) {	//for each item in the order
			if (-1 == VarLong(m_list->Value[i][O_ID])) {	//if the item is not saved

				BOOL bIsConsignment = VarLong(m_list->Value[i][O_ForStatus]) == InvUtils::odsConsignment;

				AddToItemOnOrder(							//update the quanity in the item combo
					VarLong(m_list->Value[i][O_ProductID]), 
					VarLong(m_list->Value[i][O_Quantity]),
					bIsConsignment);
			}
		}
	
	}NxCatchAll("Error in AddUnsavedItemsToItemsOnOrder()");
}
*/

void CInvEditOrderDlg::OnRButtonUpList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if(nRow == -1) {
		return;
	}
	bool bIsLineReceived = (m_list->GetValue(nRow, O_DateReceived).vt == VT_DATE);
	if (!bIsLineReceived || (g_userPermission[MarkReceived] || g_userPermission[DeleteReceived]))
	{	
		
		m_list->PutCurSel(nRow);

		CMenu menPopup;
		CPoint pt;
		CWnd* pList;

		menPopup.m_hMenu = CreatePopupMenu();
		
		if (g_userPermission[MarkReceived])
		{
			if (!bIsLineReceived)
			{	menPopup.InsertMenu(0, MF_BYPOSITION, IDM_RECEIVE,	"Mark Received");
				if (m_list->Value[nRow][O_Quantity].lVal > 1)
					menPopup.InsertMenu(1, MF_BYPOSITION, IDM_PARTIAL,	"Partial Received");
			}
			else menPopup.InsertMenu(0, MF_BYPOSITION, IDM_UNRECEIVE, "Mark Pending");
		}

		if (!bIsLineReceived || g_userPermission[DeleteReceived])
		{
			menPopup.InsertMenu(2, MF_BYPOSITION, IDM_DELETE,	"Delete");
		}

		pList = GetDlgItem(IDC_LIST);
		if (pList != NULL) 
		{	pt.x = x;
			pt.y = y;
			pList->ClientToScreen(&pt);
			menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
		else HandleException(NULL, "An error ocurred while creating menu");
		
		m_rightClicked = nRow;
	}
}

// (j.jones 2008-03-18 14:44) - PLID 29309 - added appt. and location IDs as optional parameters
//TES 7/22/2008 - PLID 30802 - Added an optional default supplier
void CInvEditOrderDlg::DoFakeModal(long OrderID, BOOL bSaveAllAsReceived, long nApptID /*= -1*/, long nLocationID /*= -1*/, long nSupplierID /*= -1*/)
{
	// (c.haag 2007-12-05 14:44) - PLID 28286 - If bSaveAllAsReceived is FALSE, then
	// we treat this as a brand new order and expect that nothing has arrived.
	// If it's TRUE, then we use this dialog to let the user enter inventory
	// items from a previous order that wasn't entered into the system.
	m_bSaveAllAsReceived = bSaveAllAsReceived;	

	// (j.jones 2009-02-10 16:42) - PLID 32871 - change the background colors if auto-receiving
	ConfigureDisplay();

	// (j.jones 2008-03-18 14:46) - PLID 29309 - added m_nApptID and m_nDefLocationID
	m_nApptID = nApptID;
	m_nDefLocationID = nLocationID;
	m_nOrigLinkedApptID = -1;

	//TES 7/22/2008 - PLID 30802 - Added an optional default supplier
	m_nDefSupplierID = nSupplierID;
	

	//put us in a modal-like state
	GetMainFrame()->EnableWindow(FALSE);
	EnableWindow(TRUE);

	//load last item, category
//	m_category->SetSelByColumn(0, InvUtils::GetDefaultCategory());
//	OnSelChangedCategory(-1);

	if (m_supplierChecker.Changed())
		m_supplier->Requery();

	//(e.lally 2005-11-04) PLID 18152 - Add ability for suppliers to be inactive
	//if the current where clause isn't what we expect it to be, requery. This would happen if the
	//last order we looked at had an inactive supplier.
	//TES 11/6/2007 - PLID 27981 - VS2008 - Need to convert _bstr_t to CString
	else if(CString((LPCTSTR)m_supplier->WhereClause) != m_strDefaultWhereClause){
		m_supplier->PutWhereClause(_bstr_t(m_strDefaultWhereClause));
		m_supplier->Requery();
	}

	m_item->SetSelByColumn(0, InvUtils::GetDefaultItem());

	// (c.haag 2008-02-11 14:38) - PLID 28286 - By default, the auto button is enabled

	// (a.walling 2008-02-15 12:22) - PLID 28946 - Disable consignment button if not licensed
	GetDlgItem(IDC_AUTO_CONSIGN)->EnableWindow(m_bHasAdvInventory);
	GetDlgItem(IDC_AUTO_PURCH_INV)->EnableWindow(TRUE);
	GetDlgItem(IDC_AUTO_CONSIGN)->ShowWindow(m_bHasAdvInventory);
	GetDlgItem(IDC_AUTO_PURCH_INV)->ShowWindow(TRUE);

	m_aryOrderDetailsToDelete.clear();

	if ((m_id = OrderID) != -1) {
		
		// (j.jones 2009-02-16 10:16) - PLID 33085 - changed Refresh() to LoadOrder(), which now has return values
		if(!LoadOrder()) {
			//can only be false if an exception is raised, so cancel, and return
			OnCancelBtn();
			return;
		}

		m_changed = false;

		// (c.haag 2008-06-25 11:08) - PLID 28438 - If an order is at least partially received, we forbid
		// the user from changing location-related information
		BOOL bEnable;
		if (InvUtils::IsOrderPartiallyOrFullyReceived(OrderID)) {
			bEnable = FALSE;
		} else {
			bEnable = TRUE;
		}
		GetDlgItem(IDC_SOLD_TO)->EnableWindow(bEnable);
		GetDlgItem(IDC_ENABLE_SHIP_TO)->EnableWindow(bEnable);
		GetDlgItem(IDC_LOCATION)->EnableWindow(bEnable);

		//TES 7/22/2008 - PLID 30802 - Set the supplier here, the other branch sets it through the Initialize() function.
		//TES 8/13/2008 - PLID 30165 - We need to tell this function not to treat this supplier as "new" (and therefore not
		// to remember its "On File" value, but just to load the value of this existing order.
		HandleNewSupplier(-1, true);
		m_iSupplierSel = m_supplier->CurSel;
	}
	else//load defaults
	{	
		// (c.haag 2008-06-25 11:37) - PLID 28438 - Ensure the location dropdowns are enabled
		// (although the ship to 
		GetDlgItem(IDC_SOLD_TO)->EnableWindow(TRUE);
		GetDlgItem(IDC_ENABLE_SHIP_TO)->EnableWindow(TRUE);
		// GetDlgItem(IDC_LOCATION)->EnableWindow(TRUE); // This is taken care of in Initialize()

		//TES 6/13/2008 - PLID 28078 - Moved into its own function, we are now actually called modally sometimes, so in that
		// case, this code also needed to be available in OnInitDialog().
		Initialize();

		//TES 7/22/2008 - PLID 30802 - If we were given allocation details to load, do it now.
		if(m_arAllocationDetails.GetSize()) {
			LoadProductsFromAllocationDetails();
		}
	}

	ShowWindow(SW_SHOW);//show window last, so they don't see what we are doing
}

//TES 7/22/2008 - PLID 30802 - Added an optional default supplier
int CInvEditOrderDlg::DoModal(long OrderID, BOOL bSaveAllAsReceived, long nApptID /*= -1*/, long nLocationID /*= -1*/, long nSupplierID /*= -1*/)
{
	//TES 6/13/2008 - PLID 28078 - We can now be called as actually modal, so just set our member variables, and do it.
	m_id = OrderID;
	m_nApptID = nApptID;
	m_nDefLocationID = nLocationID;
	m_bSaveAllAsReceived = bSaveAllAsReceived;
	m_nDefSupplierID = nSupplierID;
	m_bIsModal = true;

	return CNxDialog::DoModal();
}

//TES 6/13/2008 - PLID 28078 - Moved into its own function, called from different places depending whether this dialog is
// modal.
void CInvEditOrderDlg::Initialize()
{
	//Clear the old items
	m_list->Clear();
	//TES 7/22/2008 - PLID 30802 - Clear out our list of allocation details.
	ClearAllocationDetailIDs();

	// (j.jones 2008-09-25 09:30) - PLID 31501 - clear the allocation list
	m_pLinkedAllocations->Clear();
	m_bShowApptLinkLabel = true;
	HideLinkedAllocations();
	InvalidateRect(&m_rcApptLinkLabel);

	m_nOrigLinkedApptID = -1;
	m_strApptLinkText = "< No Linked Appointment >";
	m_strOldApptLinkText = m_strApptLinkText;

	// (j.jones 2009-01-23 10:39) - PLID 32822 - requery the order method list, it may have changed,
	// but this isn't necessary if it is a modal dialog
	if(!m_bIsModal) {
		m_pOrderMethodCombo->Requery();
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOrderMethodCombo->GetNewRow();
		pRow->PutValue(omcID, (long)-1);
		pRow->PutValue(omcName, _variant_t(" <None>"));
		m_pOrderMethodCombo->AddRowSorted(pRow, NULL);
		m_pOrderMethodCombo->PutCurSel(pRow);
	}

	// (d.thompson 2009-01-23) - PLID 32823 - Make sure our controls are properly setup
	CheckDlgButton(IDC_VENDOR_CONFIRMED_CHK, FALSE);
	SetDlgItemText(IDC_VENDOR_CONF_NUM, "");
	m_nxtDateConfirmed->Clear();
	EnsureConfirmedControls();

	// (j.jones 2008-03-18 16:07) - PLID 29309 - supported a default location ID
	if(m_nDefLocationID != -1) {
		if(m_location->SetSelByColumn(0, m_nDefLocationID) == -1) {
			m_location->SetSelByColumn(0, GetCurrentLocation());
		}
	}
	else {
		m_location->SetSelByColumn(0, GetCurrentLocation());
	}

	if(m_nDefLocationID != -1) {
		if(m_pLocationSoldTo->SetSelByColumn(0, m_nDefLocationID) == NULL) {
			m_pLocationSoldTo->SetSelByColumn(0, GetCurrentLocation());
		}
	}
	else {
		m_pLocationSoldTo->SetSelByColumn(0, GetCurrentLocation());
	}

	//TES 7/22/2008 - PLID 30802 - Set the default supplier if we were given one.
	if(m_nDefSupplierID != -1) {
		m_supplier->SetSelByColumn(0, m_nDefSupplierID);
		m_iSupplierSel = m_supplier->CurSel;		
	}

	// (j.jones 2008-09-24 10:17) - PLID 20583 - Ensured that we always
	// handle the supplier change, because we might have not requeried the
	// product list yet. This function does handle the case where m_iSupplierSel
	// may be -1, though it is improbable for that to occur.
	HandleNewSupplier(m_iSupplierSel);

	DisplayLinkedApptData();

	m_nxtDate->SetDateTime(COleDateTime::GetCurrentTime());
	SetDlgItemText (IDC_TAX, "0%");
	SetDlgItemText (IDC_EXTRA_COST, FormatCurrencyForInterface(COleCurrency(0,0)));
	SetDlgItemText (IDC_SUBTOTAL, FormatCurrencyForInterface(COleCurrency(0,0)));
	SetDlgItemText (IDC_TAXTOTAL, FormatCurrencyForInterface(COleCurrency(0,0)));
	SetDlgItemText (IDC_EXTRACOST, FormatCurrencyForInterface(COleCurrency(0,0)));
	SetDlgItemText (IDC_TOTAL, FormatCurrencyForInterface(COleCurrency(0,0)));
	SetDlgItemText (IDC_DESCRIPTION, "");
	SetDlgItemText (IDC_TRACKING_NUMBER, "");
	// (j.gruber 2009-03-03 14:50) - PLID 30199 - added total quantity
	SetDlgItemText (IDC_INV_ORDER_TOTAL_QTY, "0");
	// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
	SetDlgItemText (IDC_PURCHASE_ORDER, "");
	SetDlgItemText (IDC_ORDER_CONTACT_NAME, "");
	// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
	SetDlgItemText (IDC_ORDER_CC_NAME, "");
	SetDlgItemText (IDC_ORDER_CC_EXP_DATE, "");
	// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
	m_pShipMethodList->SetSelByColumn(0,(long)-1);
	// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
	m_pCCTypeList->SetSelByColumn(0,(long)-1);
	m_strCreditCardNumber = "";

	m_aryOrderDetailsToDelete.clear();
	
	//TES 8/13/2008 - PLID 30165 - Remember the "CC On File" value for the current supplier.
	RememberCCOnFile();

	CheckDlgButton(IDC_ENABLE_SHIP_TO, FALSE);
	GetDlgItem(IDC_LOCATION)->EnableWindow(FALSE);
	
	// (c.haag 2007-12-05 17:26) - PLID 28286 - If this is a same-day order, then
	// we need to customize certain fields.
	if (m_bSaveAllAsReceived) {
		COleDateTime dtToday(COleDateTime::GetCurrentTime());
		dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());
		m_nxtArrivalDate->SetDateTime(dtToday);
		// (j.jones 2008-02-07 10:42) - PLID 28851 - split the "auto" feature into two buttons
		GetDlgItem(IDC_AUTO_PURCH_INV)->EnableWindow(FALSE);
		GetDlgItem(IDC_AUTO_CONSIGN)->EnableWindow(FALSE);
	}
	//r.wilson 2/22/2012 - PLID 47393
	if(m_bFrameOrderReception)
	{
		//GetDlgItem(IDC_AUTO_PURCH_INV)->EnableWindow(TRUE);
		//GetDlgItem(IDC_AUTO_CONSIGN)->EnableWindow(TRUE);
		GetDlgItem(IDC_AUTO_PURCH_INV)->ShowWindow(FALSE);
		GetDlgItem(IDC_AUTO_CONSIGN)->ShowWindow(FALSE);
	}
	else {
		m_nxtArrivalDate->Clear();
	}
	SetDlgItemText (IDC_NOTES, "");
	m_changed = true;
}

void CInvEditOrderDlg::OnKillfocusExtraCost() 
{
	COleCurrency cost;
	CString val;

	GetDlgItemText(IDC_EXTRA_COST, val);
	cost = ParseCurrencyFromInterface(val);
	//TES 11/08/2007 - PLID 27979 - VS2008 - VS 2008 doesn't like this syntax, and there's no need for it.
	//if (cost.GetStatus() != COleCurrency::CurrencyStatus::valid)
	if (cost.GetStatus() != COleCurrency::valid)
		cost = COleCurrency(0,0);
	SetDlgItemText(IDC_EXTRA_COST, FormatCurrencyForInterface(cost));
	UpdateTotal();
}

void CInvEditOrderDlg::OnKillfocusTax() 
{
	double tax;
	CString val;

	GetDlgItemText(IDC_TAX, val);
	val.TrimRight('%');
	tax = atof(val);
	if (tax < 0 || tax >= 100)
		tax = 0;

	val.Format("%g%%", tax);
	SetDlgItemText(IDC_TAX, val);
	UpdateTotal();
}

void CInvEditOrderDlg::UpdateTotal(int nRow/* = -1*/)
{
	COleCurrency	amount,
					totalAmount,
					taxamount,
					orderExtra,
					itemExtra,
					total;
	double			tax = GetTax();
	long			quantity, nTotalQuantity = 0;
	CString			str;
	BOOL			bTaxShipping;

	double dblPercentOff = 0.0;
	COleCurrency cyDiscount = COleCurrency(0,0);

	if (!m_bTotalUpdateable)
		return;

	try
	{
		GetDlgItemText(IDC_EXTRA_COST, str);
		orderExtra = ParseCurrencyFromInterface(str);

		bTaxShipping = m_checkTaxShipping.GetCheck();

		for (long i = 0; i < m_list->GetRowCount(); i++)
		{	amount = m_list->Value[i][O_Amount].cyVal;
			itemExtra = m_list->Value[i][O_ExtraCost];
			quantity = m_list->Value[i][O_Quantity];

			// (j.jones 2008-06-19 10:50) - PLID 10394 - added discount abilities
			dblPercentOff = VarDouble(m_list->Value[i][O_PercentOff]);
			cyDiscount = VarCurrency(m_list->Value[i][O_Discount]);

			total = (amount * quantity) + itemExtra;
			
			//logic is odd but this is how billing does it, and it works
			total = (total * (100000 - dblPercentOff * 1000));
			total = total / long(100000);

			total -= cyDiscount;
			
			RoundCurrency(total);

			totalAmount += total;			
			m_list->Value[i][O_Total] = _variant_t(total);

			// (j.gruber 2009-03-03 14:52) - PLID 30199 - add the quantity into the total
			nTotalQuantity += quantity;
		}

		//DO NOT CALCULATE TAX UNTIL THIS POINT. We need to break out of the idea that tax is computed
		//per line item and the total is just the sum of those items. In "the real world" tax is not
		//computed until the end!
		
		SetDlgItemText(IDC_SUBTOTAL, FormatCurrencyForInterface(totalAmount));
		
		// (a.walling 2012-03-12 17:33) - PLID 48839 - Use CalculateTax
		taxamount = CalculateTax(totalAmount, tax);
		RoundCurrency(taxamount);		
		SetDlgItemText(IDC_TAXTOTAL, FormatCurrencyForInterface(taxamount));

		// (j.gruber 2009-03-03 14:52) - PLID 30199 - added total quantity
		SetDlgItemText(IDC_INV_ORDER_TOTAL_QTY, AsString(nTotalQuantity));

		if(bTaxShipping) {
			// (a.walling 2012-03-12 17:33) - PLID 48839 - Use CalculateTax
			COleCurrency taxOrder = CalculateTax(orderExtra, tax);
			RoundCurrency(taxOrder);
			orderExtra += taxOrder;
		}

		totalAmount += taxamount;
		totalAmount += orderExtra;
		SetDlgItemText(IDC_EXTRACOST, FormatCurrencyForInterface(orderExtra));
		SetDlgItemText(IDC_TOTAL, FormatCurrencyForInterface(totalAmount));
	}NxCatchAll("Could not update total");
}

void CInvEditOrderDlg::OnEditingFinishingList(long nRow, short nCol, const VARIANT FAR& varOldValue, 
		LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if (*pbCommit == FALSE)//user hit escape
			return;

		switch (nCol)
		{
			case O_Catalog:
				break;
			//DRT 11/8/2007 - PLID 27984 - For consistency, adding my new consignment column, which has no particular worries about this functionality.
			// (c.haag 2007-12-03 11:10) - PLID 28204 - Now it's a status field...and because it's no longer a boolean, we need to do validation here.
			case O_ForStatus: {
				// (c.haag 2007-12-03 11:19) - PLID 28204 - This used to be a consignment bit, but now we have
				// to change it to be a more general status value. I copied Don's code from OnEditingStarting
				// and made proper adjustments (*pbCommit = FALSE, etc)
				if (InvUtils::odsConsignment == VarLong(*pvarNewValue)) {
					//DRT 11/8/2007 - PLID 27984 - If the particular item is not serial or expirable, consignment status is not allowed.
					BOOL bHasSerial = VarBool(m_list->Value[nRow][O_HasSerialNum], FALSE);
					BOOL bHasExp = VarBool(m_list->Value[nRow][O_HasExpDate], FALSE);
					if(!bHasSerial && !bHasExp) {
						//Do not allow to edit consignment if it's not a serialized / expirable item
						*pbCommit = FALSE;
						AfxMessageBox("You may only mark items for consignment if they have serial numbers or expire.");
						return;
					}
				}

				// (j.jones 2008-02-07 12:48) - PLID 28851 - update the on order amount to remove the old value
				//decrement the current amount
				//if received

				// (j.jones 2008-02-07 16:47) - PLID 28851 - track whether this was consignment
				// (j.jones 2008-02-08 14:13) - PLID 28851 - these functions have been removed,
				// we no longer dynamically update the product totals
				/*
				BOOL bIsConsignment = VarLong(m_list->Value[nRow][O_ForStatus]) == InvUtils::odsConsignment;
				if(m_list->GetValue(nRow,O_DateReceived).vt == VT_DATE) {
					AddToItemReceived(VarLong(m_list->Value[nRow][O_ProductID]), 
						0 - VarLong(m_list->Value[nRow][O_Quantity]), bIsConsignment);
				}
				else {
					AddToItemOnOrder(
						VarLong(m_list->Value[nRow][O_ProductID]), 
						0 - VarLong(m_list->Value[nRow][O_Quantity]), bIsConsignment);
				}
				*/
				}

				break;
			case O_Quantity:
				if (pvarNewValue->vt != VT_I4 || pvarNewValue->intVal < 0)
					*pbCommit = FALSE;
				else //update on order quantity
				{	//technically we need to treat ordered items differently

					// (j.jones 2008-02-07 16:47) - PLID 28851 - track whether this was consignment
					// (j.jones 2008-02-08 14:13) - PLID 28851 - these functions have been removed,
					// we no longer dynamically update the product totals
					/*
					BOOL bIsConsignment = VarLong(m_list->Value[nRow][O_ForStatus]) == InvUtils::odsConsignment;
					AddToItemOnOrder(
						VarLong(m_list->Value[nRow][O_ProductID]), 
						VarLong(pvarNewValue) - VarLong(varOldValue), bIsConsignment);
					*/
				}
				break;
			// (j.jones 2008-06-19 10:46) - PLID 10394 - added percent off and discount columns
			case O_PercentOff:
				if (pvarNewValue->vt != VT_R8 || VarDouble(pvarNewValue, 0.0) < 0.0 || VarDouble(pvarNewValue, 0.0) > 100.0) {
					*pbCommit = FALSE;
				}
				break;
			case O_Amount:			
			case O_ExtraCost:
			case O_Discount:
				if (pvarNewValue->vt != VT_CY || COleCurrency(pvarNewValue->cyVal) < COleCurrency(0,0))
					*pbCommit = FALSE;
				break;
			case O_DateReceived:
			{
				//for auditing
				CString strOld;
				try {
					if (m_list->CurSel >= 0 && VarLong(m_list->Value[m_list->CurSel][O_ID]) > 0) {
						_RecordsetPtr rs = CreateRecordset("SELECT DateReceived FROM OrderDetailsT WHERE ID = %li", long(m_list->GetValue(m_list->CurSel, 0).lVal));
						if(!rs->eof && rs->Fields->Item["DateReceived"]->Value.vt != VT_NULL) {

							// (c.haag 2008-05-21 15:47) - PLID 30082 - We no longer allow users to unreceive orders
							BOOL bDateValid = TRUE;
							if (VT_DATE != pvarNewValue->vt) {
								bDateValid = FALSE;
							} else {
								COleDateTime dt(VarDateTime(*pvarNewValue));
								if (dt.m_dt <= 0) {
									bDateValid = FALSE;
								}
							}

							if (!bDateValid) {
								*pbCommit = FALSE;
								AfxMessageBox("You may not reset the received date of an order item that is marked as received in your data.\n"
									"\n"
									"If the received information for this order is incorrect, you must delete this order and create a new one to reflect the correct values.", MB_ICONERROR);
								break;
							}

							COleDateTime dt = COleDateTime(rs->Fields->Item["DateReceived"]->Value.date);
							strOld = FormatDateTimeForSql(dt, dtoDate);
						}
					}
				} NxCatchAll("Error auditing order received");
				//

				CString str  = strUserEntered;
				str.TrimRight();
				if (str == "")
				{	_variant_t varNull;
					varNull.ChangeType(VT_NULL);
					*pvarNewValue = varNull;
					if(varOldValue.vt == VT_DATE) {
						//we're marking it as not received
						long productID = VarLong(m_list->Value[nRow][O_ProductID]);

						// (j.jones 2008-02-07 16:47) - PLID 28851 - track whether this was consignment					

						// (j.jones 2008-02-08 14:13) - PLID 28851 - these functions have been removed,
						// we no longer dynamically update the product totals
						/*
						BOOL bIsConsignment = VarLong(m_list->Value[nRow][O_ForStatus]) == InvUtils::odsConsignment;
						AddToItemReceived(productID, 0 - VarLong(m_list->Value[nRow][O_Quantity]), bIsConsignment);
						*/
					}
				}
				else
				{	COleDateTime dt;
					dt.ParseDateTime(str);
					if (dt.GetStatus() == COleDateTime::valid) {
						if(dt > COleDateTime::GetCurrentTime() && IDNO==MessageBox("The received date is in the future, are you sure you wish to change this?","Practice",MB_ICONQUESTION|MB_YESNO)) {
							*pbCommit  = FALSE;
							return;
						}
						*pvarNewValue = _variant_t(dt, VT_DATE);
						if(varOldValue.vt != VT_DATE) {
							//we're marking it received, not just changing the date
							long productID = VarLong(m_list->Value[nRow][O_ProductID]);

							// (j.jones 2008-02-07 16:47) - PLID 28851 - track whether this was consignment

							// (j.jones 2008-02-08 14:13) - PLID 28851 - these functions have been removed,
							// we no longer dynamically update the product totals
							/*
							BOOL bIsConsignment = VarLong(m_list->Value[nRow][O_ForStatus]) == InvUtils::odsConsignment;
							AddToItemReceived(productID, VarLong(m_list->Value[nRow][O_Quantity]), bIsConsignment);
							*/
						}
					}
					else {
						*pbCommit = FALSE;
						return;
					}
				}
				//DRT 11/13/2007 - PLID 28052 - Auditing is now done when we save, not immediately.
				break;
			}
			default:
				ASSERT(FALSE);
		}
	}
	NxCatchAll("Error in CInvEditOrderDlg::OnEditingFinishingList");
}

void CInvEditOrderDlg::OnEditingFinishedList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	//try to remember why i did this, I don't think -2 is a possible value -BVB
	if (m_list->Value[nRow][0].lVal == -2)
		return;

	m_changed = true;

	switch (nCol) {

		// (j.jones 2008-06-19 10:46) - PLID 10394 - added percent off and discount columns
		case O_Quantity:
		case O_Amount:		
		case O_ExtraCost:
		case O_PercentOff:
		case O_Discount:
			UpdateTotal(nRow);
			break;
		case O_Catalog:
		case O_DateReceived:
		//DRT 11/8/2007 - PLID 27984 - For consistency, adding my new consignment column, which has no particular worries about this functionality.
		// (c.haag 2007-12-03 11:10) - PLID 28204 - Now it's a status field
			break;
		case O_ForStatus:
			// (j.jones 2008-02-07 12:48) - PLID 28851 - update the on order amount to add the new value
			//add to the current amount
			//if received
			if(m_list->GetValue(nRow,O_DateReceived).vt == VT_DATE) {

				// (j.jones 2008-02-07 16:47) - PLID 28851 - track whether this was consignment

				// (j.jones 2008-02-08 14:13) - PLID 28851 - these functions have been removed,
				// we no longer dynamically update the product totals
				/*
				BOOL bIsConsignment = VarLong(m_list->Value[nRow][O_ForStatus]) == InvUtils::odsConsignment;
				AddToItemReceived(VarLong(m_list->Value[nRow][O_ProductID]), 
					VarLong(m_list->Value[nRow][O_Quantity]), bIsConsignment);
				*/
			}
			else {

				// (j.jones 2008-02-07 16:47) - PLID 28851 - track whether this was consignment
				// (j.jones 2008-02-08 14:13) - PLID 28851 - these functions have been removed,
				// we no longer dynamically update the product totals
				/*
				BOOL bIsConsignment = VarLong(m_list->Value[nRow][O_ForStatus]) == InvUtils::odsConsignment;
				AddToItemOnOrder(
					VarLong(m_list->Value[nRow][O_ProductID]), 
					VarLong(m_list->Value[nRow][O_Quantity]),
					bIsConsignment);
				*/
			}
			// (c.haag 2007-12-17 13:46) - PLID 28286 - If this is a retroactively-finishing order, we have
			// to synchronize the status of this row with that of the corresponding product item dialog
			if (m_bSaveAllAsReceived) {
				CProductItemsDlg* pDlg = (CProductItemsDlg*)VarLong(m_list->Value[nRow][O_DlgPtr], NULL);
				if (NULL != pDlg) {
					pDlg->SetAllProductItemStatuses(VarLong(m_list->Value[nRow][O_ForStatus]));
				}
			}
			break;
		default:
			ASSERT(FALSE);
	}
}

void CInvEditOrderDlg::OnRequeryFinishedList(short nFlags)
{	
	UpdateTotal();
}

// (j.jones 2008-03-05 11:18) - PLID 28981 - split ReorderAmount into Purchased and Consignment functions
long CInvEditOrderDlg::ReorderAmountPurchased(double dblMinAmountNeeded, long nReorderQuantity)
{
	//given we need to order at least dblMinAmountNeeded items
	//and we can only order amounts divisible by multiple
	//this function will tell us the amount to order
	long nResult = 1;

	if (nReorderQuantity < 1)
		nReorderQuantity = 1;

	nResult = ((long)(dblMinAmountNeeded / nReorderQuantity)) * nReorderQuantity;

	if (nResult <= dblMinAmountNeeded)
		nResult += nReorderQuantity;

	return nResult;
}

long CInvEditOrderDlg::ReorderAmountConsignment(double dblMinAmountNeeded)
{
	//consignment is not ordered in increments of anything but 1,
	//so all we need to do is see if dblMinAmountNeeded is less than 1.0,
	//and if it is, return 1
	long nResult =(long)(dblMinAmountNeeded);

	if(nResult < 1) {
		return 1;
	}
	else {
		return nResult;
	};
}

// (j.jones 2008-02-07 10:49) - PLID 28851 - added an override to force ordering as purchased inventory or as consignment
// (a.walling 2008-03-20 09:21) - PLID 29333 - added a flag to control whether item is added as a new row if it exists
// or quantity is upped by 1. Default is TRUE, meaning default behaviour does not change.
//TES 7/22/2008 - PLID 30802 - Added optional parameters for the quantity to add, and an associated Allocation Detail.
long CInvEditOrderDlg::AddItem(long nSelRow, AddItemRule airRule, BOOL bAddIfExists /*= TRUE*/, double dQty /*= -1.0*/, long nAllocationDetailID /*= -1*/)
{
	// (c.haag 2007-12-05 15:35) - PLID 28286 - Moved code from OnAddItem to this function.
	// This returns the index of the newly added row.
	IRowSettingsPtr	pRow = NULL;
	long			quantity,//quanity to order
					ordered,
					conversion,
					UseUU,						
					ForStatus;			//DRT 11/8/2007 - PLID 27984
										// (c.haag 2007-12-03 11:11) - PLID 28204 - This
										// used to be ForConsignment, but now it's an all-purpose
										// "ForStatus" value
	COleCurrency	amount, 
					total;
	double			tax,
					reorderPoint,
					actual,
					avail;
	CString			val, 
					name,
					catalog;

	if (nSelRow == -1)
		return -1;

	m_changed = true;

	amount = VarCurrency(m_item->Value[nSelRow][I_LastCost],COleCurrency(0,0));	
	conversion = VarLong(m_item->Value[nSelRow][I_Conversion],0);
	UseUU = VarLong(m_item->Value[nSelRow][I_UseUU],0);
	catalog = VarString(m_item->Value[nSelRow][I_Catalog],"");

	// (j.jones 2008-02-07 10:55) - PLID 28851 - the passed in airRule may force a status
	// that overrides the default value for the product
	if(airRule == airPurchasedInv) {
		ForStatus = InvUtils::odsPurchased;
	}
	else if(airRule == airConsignment) {
		ForStatus = InvUtils::odsConsignment;
	}
	else {
		// (j.jones 2008-02-07 10:55) - PLID 28851 - add normally, which means to follow the
		// product's default consignment status
		
		// (a.walling 2008-02-15 12:22) - PLID 28946 - Hide the consignment info if not licensed
		//DRT 11/8/2007 - PLID 27984 - Load the default consignment setting
		// (c.haag 2007-12-03 11:11) - PLID 28204 - Consignment is now part of the "Status"
		ForStatus = (m_bHasAdvInventory && VarBool(m_item->Value[nSelRow][I_DefaultConsignment])) ? InvUtils::odsConsignment : InvUtils::odsPurchased;
	}

	//PLID 28851 - now use ForStatus to determine whether use consignment or general values
	if(ForStatus == InvUtils::odsConsignment) {
		//use the consignment values

		// (j.jones 2008-02-15 17:06) - PLID 28852 - we do not use a reorder qty. for consignment
		quantity = 1; //VarLong(m_item->Value[nSelRow][I_ReorderQuantityConsign],1);
		ordered = VarLong(m_item->Value[nSelRow][I_OrderedConsign],0);
		actual = VarDouble(m_item->Value[nSelRow][I_ActualConsign],0.0);
		avail = VarDouble(m_item->Value[nSelRow][I_AvailConsign],0.0);
		reorderPoint = VarDouble(m_item->Value[nSelRow][I_ReorderPointConsign],0.0);
	}
	else {
		//otherwise use the general values, which is currently the standard
		//behavior when adding a normal product		
		quantity = VarLong(m_item->Value[nSelRow][I_ReorderQuantityPurchased],1);
		ordered = VarLong(m_item->Value[nSelRow][I_OrderedPurchased],0);
		actual = VarDouble(m_item->Value[nSelRow][I_ActualPurchased],0.0);
		avail = VarDouble(m_item->Value[nSelRow][I_AvailPurchased],0.0);
		reorderPoint = VarDouble(m_item->Value[nSelRow][I_ReorderPointPurchased],0.0);
	}

	// (j.jones 2008-02-14 12:01) - PLID 28864 - added option for when to order products, when the
	// actual (0) hits the reorder point, or when the available (1) hits it
	if(dQty != -1.0) {
		//TES 7/22/2008 - PLID 30802 - We were given a default quantity, just use that.
		quantity = (long)dQty;
		//TES 7/22/2008 - PLID 30802 - Round up.
		if(dQty - (double)quantity > 0.0) {
			quantity++;
		}
	}
	else {
		long nOrderByOnHandAmount = GetRemotePropertyInt("InvItem_OrderByOnHandAmount", 0, 0, "<None>", true);
		BOOL bOrderByActual = (nOrderByOnHandAmount == 0);

		double dblValueToCheck = 0.0;
		if(bOrderByActual) {
			dblValueToCheck = actual;
		}
		else {
			dblValueToCheck = avail;
		}

		// (j.jones 2008-02-19 12:29) - PLID 28981 - now consignment orders
		// only when under the reorderpoint, not at it, and it has no reorder quantity
		if(ForStatus == InvUtils::odsConsignment) {
			if (dblValueToCheck + ordered < reorderPoint) {
				quantity = ReorderAmountConsignment(reorderPoint - dblValueToCheck - ordered);
			}
		}
		else { //purchased inventory (or any other non-consignment status)
			//if we need to order more than 1x standard reorder, do it automatically	
			if (dblValueToCheck + ordered + quantity <= reorderPoint) {
				quantity = ReorderAmountPurchased(reorderPoint - dblValueToCheck - ordered, quantity);
			}
		}

		// (c.haag 2008-02-15 10:18) - PLID 28286 - If this a retroactively completed order,
		// we cannot use the same quantity calculations we do when placing new, incomplete
		// orders. We instead force the default quantity to one. This enforces a pattern of
		// the user choosing a product, and if it's serialized, they scan in all the serials
		// when OnNewItemAdded is called.		
		if (m_bSaveAllAsReceived) {
			quantity = 1;
		} 
	}

	GetDlgItemText(IDC_TAX, val);
	val.TrimRight('%');
	tax = atof(val);
	if (tax < 0 || tax >= 100)
		tax = 0;

	name = (LPCSTR)_bstr_t(m_item->Value[nSelRow][I_Name]);
	GetDlgItemText(IDC_DESCRIPTION, val);
	// (r.wilson 2/6/2012) - PLID 47394 Now checking the preference
	if (val == "" && GetRemotePropertyInt("Inventory_AutoPopulateOrderDesc", 1, 0, GetCurrentUserName(), true))
	{
		SetDlgItemText(IDC_DESCRIPTION, name);
	}
	
	long nNewRow = -1;

	// (a.walling 2008-03-20 11:37) - PLID 29333 - Increase the qty, but only if the row exists and we are flagged to behave as such
	if (!bAddIfExists) {
		nNewRow = m_list->FindByColumn(O_ProductID, m_item->Value[nSelRow][I_ID], NULL, VARIANT_FALSE);
		if (nNewRow >= 0) {
			if (pRow = m_list->GetRow(nNewRow)) {
				//TES 7/24/2008 - PLID 30802 - Increment by the actual quantity being added, not 1
				pRow->Value[O_Quantity] = (long)(VarLong(pRow->Value[O_Quantity]) + quantity);
				if(nAllocationDetailID != -1) {
					//TES 7/22/2008 - PLID 30802 - We were given an allocation detail to associate, does this row already
					// have a list of allocation detail IDs?
					if(pRow->Value[O_AllocationDetailListID].vt == VT_I4) {
						//TES 7/22/2008 - PLID 30802 - It does, add it.
						AddAllocationDetailToList(nAllocationDetailID, VarLong(pRow->Value[O_AllocationDetailListID]));
					}
					else {
						//TES 7/22/2008 - PLID 30802 - Nope, start a new list with this detail.
						pRow->PutValue(O_AllocationDetailListID, GetNewAllocationDetailList(nAllocationDetailID));
					}
				}
			}
		}
	}
	
	if (pRow == NULL) {
		pRow = m_list->GetRow(-1);
		pRow->Value[O_ID] = _variant_t(-1L);//don't get a real ID yet
		pRow->Value[O_ProductID] = m_item->Value[nSelRow][I_ID];
		pRow->Value[O_Name] = _bstr_t(name);
		pRow->Value[O_Catalog] = _bstr_t(catalog);
		pRow->Value[O_Quantity] = quantity;
		pRow->Value[O_Conversion] = conversion;
		pRow->Value[O_UseUU] = UseUU;
		pRow->Value[O_Amount] = _variant_t(amount);
		pRow->Value[O_ExtraCost] = _variant_t(COleCurrency(0,0));
		// (j.jones 2008-06-19 11:14) - PLID 10394 - added discount fields
		pRow->Value[O_PercentOff] = (double)0.0;
		pRow->Value[O_Discount] = _variant_t(COleCurrency(0,0));

		// (c.haag 2007-12-05 15:02) - PLID 28286 - If this is the kind of order where products
		// are immediately received, set the received date to now
		if (m_bSaveAllAsReceived) {
			COleDateTime dtToday(COleDateTime::GetCurrentTime());
			dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());
			pRow->Value[O_DateReceived] = _variant_t((DATE)dtToday, VT_DATE);
		} else {
			pRow->Value[O_DateReceived] = _variant_t();
		}
		// (c.haag 2007-12-05 15:38) - PLID 28286 - The product item dialog pointer is NULL by default
		_variant_t vNull;
		vNull.vt = VT_NULL;
		pRow->Value[O_DlgPtr] = vNull;

		//DRT 11/8/2007 - PLID 27984 - Load the default consignment setting
		// (c.haag 2007-12-03 11:17) - PLID 28204 - Changed from O_ForConsignment to O_ForStatus
		pRow->Value[O_ForStatus] = ForStatus;

		//DRT 11/8/2007 - PLID 27984 - Update the serial/exp status in the main list
		BOOL bHasSerial = VarBool(m_item->Value[nSelRow][I_HasSerialNum], FALSE);
		//TES 7/3/2008 - PLID 24726 - Added
		BOOL bSerialNumIsLotNum = VarBool(m_item->Value[nSelRow][I_SerialNumIsLotNum], FALSE);
		BOOL bHasExp = VarBool(m_item->Value[nSelRow][I_HasExpDate], FALSE);
		pRow->Value[O_HasSerialNum] = bHasSerial == FALSE ? g_cvarFalse : g_cvarTrue;
		pRow->Value[O_SerialNumIsLotNum] = bSerialNumIsLotNum == FALSE ? g_cvarFalse : g_cvarTrue;
		pRow->Value[O_HasExpDate] = bHasExp == FALSE ? g_cvarFalse : g_cvarTrue;

		if(nAllocationDetailID == -1) {
			pRow->Value[O_AllocationDetailListID] = vNull;
		}
		else {
			//TES 7/22/2008 - PLID 30802 - Start a new list, using the passed-in detail.
			pRow->Value[O_AllocationDetailListID] = GetNewAllocationDetailList(nAllocationDetailID);
		}

		//TES 7/23/2008 - PLID 30802 - This detail wasn't split off from another detail.
		pRow->Value[O_SourceDetailID] = vNull;

		nNewRow = m_list->AddRow(pRow);
	}

	UpdateTotal(nNewRow);

	// (j.jones 2008-02-08 14:45) - PLID 28851 - we have removed the functionality to dynamically
	// update the product dropdown values
	/*
	if(VarLong(m_item->Value[nSelRow][I_TrackableStatus]) == 2) {
		// (j.jones 2008-02-07 12:28) - PLID 28851 - need to update based on the consignment status
		if(ForStatus == InvUtils::odsConsignment) {
			quantity += VarLong(m_item->Value[nSelRow][I_OrderedConsign]);
			m_item->Value[nSelRow][I_OrderedConsign] = quantity;
		}
		else {
			//all other statii use the general values
			quantity += VarLong(m_item->Value[nSelRow][I_OrderedPurchased]);
			m_item->Value[nSelRow][I_OrderedPurchased] = quantity;
		}
		UpdateItemListColors();
	}
	*/

	// (c.haag 2007-12-05 15:58) - PLID 28286 - Post-addition handling
	OnNewItemAdded(nNewRow, nSelRow);

	return nNewRow;
}

void CInvEditOrderDlg::OnAddItem(long nSelRow) 
{
	try {
		// (c.haag 2007-12-05 16:20) - PLID 28286 - Moved business logic into ::AddItem
		// (j.jones 2008-02-07 10:49) - PLID 28851 - no override needed here, just add normally
		AddItem(nSelRow, airNoRule);
	}NxCatchAll("Error in OnAddItem()");
}

BOOL CInvEditOrderDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {

	switch (wParam) 
	{	case EN_CHANGE:
			m_changed = true;
			break;

		case IDM_DELETE:
			{
				if(m_rightClicked == -1)
					break;	//error case

				long nProductID = VarLong(m_list->GetValue(m_rightClicked, O_ProductID));
				long nDetailID = VarLong(m_list->GetValue(m_rightClicked, O_ID));

				//DRT 10/2/03 - PLID 9467 - They cannot delete an order that has serialized items that have been adjusted
				//off (marked 'deleted')
				// (j.jones 2007-11-07 11:17) - PLID 27987 - disallow deleting order details that have serialized items that
				// are allocated, even if the allocation is deleted
				//(also I combined these three queries into one)
				//TES 6/25/2008 - PLID 29578 - ProductItemsT now has an OrderDetailID instead of an OrderID.
				if(ReturnsRecords("SELECT ID FROM ProductItemsT WHERE ProductID = %li "
					"AND OrderDetailID = %li "
					"AND (Deleted = 1 "
					" OR ID IN (SELECT ProductItemID FROM ChargedProductItemsT) "
					" OR ID IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null) "
					")", nProductID, nDetailID)) {
					MsgBox("You have items which have either been adjusted, billed, or allocated to patients.\n"
						"You cannot delete this product from this order.");
					break;
				}

				// (c.haag 2008-05-21 15:48) - PLID 30082 - Don't let users delete rows that are marked as received.
				// While doing so would only harm serialized items in that ProductItemsT is not changed; it's better
				// for support purposes, and as a general rule, to not allow for it across the board.
				if (VarLong(m_list->Value[m_rightClicked][O_ID]) > 0) {
					_RecordsetPtr prs = CreateParamRecordset("SELECT ID FROM OrderDetailsT WHERE ID = {INT} AND DateReceived IS NOT NULL",
						VarLong(m_list->Value[m_rightClicked][O_ID]));
					if (!prs->eof) {
						// The order detail is "received" in data. Don't allow the user to delete it
						AfxMessageBox("You may not delete an order item that is marked as received in your data.\n"
							"\n"
							"If the received information for this order is incorrect, you must delete this order and create a new one to reflect the correct values.", MB_ICONERROR);
						break;

					} else {
						// The order detail is not "received" in data; so don't stop the user from editing the quantity
					}
				}

				if (m_list->GetValue(m_rightClicked, O_DateReceived).vt == VT_DATE && !UserPermission(DeleteReceived))
					break;
				if (IDYES == MessageBox("Are you sure you want to delete this item?", "Delete Item?", MB_YESNO))
				{	
					//reflects the deleted items in the item combo on order totals
					//as we don't actually delete the items till the user hits save, we have to change this dynamically

					// (j.jones 2008-02-08 14:13) - PLID 28851 - this function has been removed,
					// we no longer dynamically update the product totals
					/*
					RemoveUnsavedItemFromOrder(m_rightClicked);
					*/
					UpdateItemListColorsAndToOrderAmt();
					RemoveListRow(m_rightClicked);
					UpdateTotal();
					m_changed = true;
				}
			}
			break;

		case IDM_RECEIVE: {

			if (!UserPermission(MarkReceived))
				break;

			//safety check: has another user already received these products?
			if (InvUtils::HasOrderDetailReceivedProductItems(VarLong(m_list->Value[m_rightClicked][O_ID], -1))) {
				//someone has already received this product
				AfxMessageBox("This product has already been received. You may need to close this window and attempt receiving the order again to ensure the order status has been updated.", MB_ICONERROR | MB_OK);
				break;
			}

			if(!ValidateUU_UOStatus(VarLong(m_list->Value[m_rightClicked][O_ID])))
				break;

			// (j.jones 2008-04-01 12:07) - PLID 28430 - moved to be AFTER the validate UU/UO check
			m_list->Value[m_rightClicked][O_DateReceived] = _variant_t(GetDate(), VT_DATE);
			m_changed = true;
			
			//update list
			long productID = VarLong(m_list->Value[m_rightClicked][O_ProductID]);
			// (j.jones 2008-02-07 16:47) - PLID 28851 - track whether this was consignment
			// (j.jones 2008-02-08 14:13) - PLID 28851 - these functions have been removed,
			// we no longer dynamically update the product totals
			/*
			BOOL bIsConsignment = VarLong(m_list->Value[m_rightClicked][O_ForStatus]) == InvUtils::odsConsignment;
			AddToItemReceived(productID, VarLong(m_list->Value[m_rightClicked][O_Quantity]), bIsConsignment);
			*/
			
			//DRT 11/13/2007 - PLID 28052 - Auditing is now done when we save, not immediately.
			break;
		}

		case IDM_UNRECEIVE:
		{
			//DRT 10/2/03 - PLID 9467 - They cannot delete an order that has serialized items that have been adjusted
			//		off (marked 'deleted')
			if(m_rightClicked == -1)
				break;	//error case

			long nProductID = VarLong(m_list->GetValue(m_rightClicked, O_ProductID));
			long nDetailID = VarLong(m_list->GetValue(m_rightClicked, O_ID));

			// (c.haag 2008-05-21 15:48) - PLID 30082 - If this order detail is already flagged as completed in data,
			// we cannot let the user undo the action
			if (VarLong(m_list->Value[m_rightClicked][O_ID]) > 0) {
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT ID FROM OrderDetailsT WHERE ID = {INT} AND DateReceived IS NOT NULL",
					VarLong(m_list->Value[m_rightClicked][O_ID]));
				if (!prs->eof) {
					// The order detail is "received" in data. Don't allow the user to unreceive it
					AfxMessageBox("You may not unreceive an order item that is marked as received in your data.\n"
						"\n"
						"If the received information for this order is incorrect, you must delete this order and create a new one to reflect the correct values.", MB_ICONERROR);
					break;
				} else {
					// The order detail is not "received" in data. So, it must also not have ProductItemsT records tied to it;
					// so we're fine.
				}
			}

			//TES 6/18/2008 - PLID 29578 - ProductItemsT now has an OrderDetailID instead of an OrderID.
			if(ReturnsRecords("SELECT ID FROM ProductItemsT WHERE OrderDetailID = %li AND Deleted = 1", nDetailID)) {
				MsgBox("You have items which have been adjusted off from this order.  You cannot delete this order.");
				break;
			}

			//TES 6/18/2008 - PLID 29578 - ProductItemsT now has an OrderDetailID instead of an OrderID.
			if(ReturnsRecords("SELECT ID FROM ProductItemsT WHERE OrderDetailID = %li AND ID IN (SELECT ProductItemID FROM ChargedProductItemsT)", nDetailID)) {
				MsgBox("You have items which have been billed from this order.  You cannot delete this order.");
				break;
			}

			// (j.jones 2007-11-21 16:51) - PLID 28037 - ensure we account for allocated items
			//TES 6/18/2008 - PLID 29578 - ProductItemsT now has an OrderDetailID instead of an OrderID.
			if(ReturnsRecords("SELECT ID FROM ProductItemsT WHERE OrderDetailID = %li AND ID IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null)", nDetailID)) {
				MsgBox("You have items which have been allocated from this order.  You cannot delete this order.");
				break;
			}

			if (!UserPermission(MarkReceived))
				break;
			m_list->Value[m_rightClicked][O_DateReceived] = _variant_t();
			m_changed = true;

			// (j.jones 2008-02-07 16:47) - PLID 28851 - track whether this was consignment
			// (j.jones 2008-02-08 14:13) - PLID 28851 - these functions have been removed,
			// we no longer dynamically update the product totals
			/*
			BOOL bIsConsignment = VarLong(m_list->Value[m_rightClicked][O_ForStatus]) == InvUtils::odsConsignment;
			// (j.jones 2008-02-07 16:57) - PLID 28862 - fixed so the dropdown is updated upon un-receiving a product
			AddToItemReceived(nProductID, 0 - VarLong(m_list->Value[m_rightClicked][O_Quantity]), bIsConsignment);
			*/
		}
		break;

		case IDM_PARTIAL:
		{	CString strResult;
			long quantity, total;

			if (!UserPermission(MarkReceived))
				break;

			//safety check: has another user already received these products?
			if (InvUtils::HasOrderDetailReceivedProductItems(VarLong(m_list->Value[m_rightClicked][O_ID], -1))) {
				//someone has already received this product
				AfxMessageBox("This product has already been received. You may need to close this window and attempt receiving the order again to ensure the order status has been updated.", MB_ICONERROR | MB_OK);
				break;
			}

			if (IDOK != InputBox(this, "How many arrived?", strResult, ""))
				return true;
			quantity = atoi(strResult);
			total = m_list->Value[m_rightClicked][O_Quantity].lVal;

			if(quantity > 0 && !ValidateUU_UOStatus(VarLong(m_list->Value[m_rightClicked][O_ID]))) {
				break;
			}

			if (quantity > 0 && quantity < total)
			{
				IRowSettingsPtr pRow;
				//decrease current row
				m_list->Value[m_rightClicked][O_Quantity] = total - quantity;
				//add new row
				pRow = m_list->GetRow(m_rightClicked);
				//TES 7/23/2008 - PLID 30802 - Track which detail this detail was originally split off from.
				long nSourceDetailID = VarLong(pRow->GetValue(O_ID),-1);
				if(nSourceDetailID == -1) nSourceDetailID = VarLong(pRow->GetValue(O_SourceDetailID),-1);

				//TES 7/24/2008 - PLID 30802 - Make sure the allocation detail list is NULL - because we track the source
				// detail ID, then InvUtils::UpdateLinkedAllocationDetailIDs will make sure that any details that need to
				// move over to this order detail will do so.
				long nAllocationDetailListID = VarLong(m_list->Value[m_rightClicked][O_AllocationDetailListID],-1); 
				m_list->Value[m_rightClicked][O_AllocationDetailListID] = g_cvarNull;

				m_list->InsertRow(pRow, m_rightClicked);
				m_list->Value[m_rightClicked][O_ID] = -1L;
				m_list->Value[m_rightClicked][O_Quantity] = quantity;
				m_list->Value[m_rightClicked][O_ExtraCost] = _variant_t(COleCurrency(0,0));
				
				// (j.jones 2008-06-19 11:22) - PLID 10394 - Only carry over the percent discount,
				// as that would likely would stay the same, the dollar amount should be zero. 
				// If the new total is negative, they just won't be allowed to save.
				m_list->Value[m_rightClicked][O_Discount] = _variant_t(COleCurrency(0,0));
				
				m_list->Value[m_rightClicked][O_DateReceived] = _variant_t(GetDate(), VT_DATE);

				//TES 7/23/2008 - PLID 30802 - Fill the SourceDetailID column.
				m_list->Value[m_rightClicked][O_SourceDetailID] = nSourceDetailID == -1 ? g_cvarNull : nSourceDetailID;

				//TES 7/24/2008 - PLID 30802 - Now fill this received detail with the correct allocation detail list.
				m_list->Value[m_rightClicked][O_AllocationDetailListID] = nAllocationDetailListID == -1 ? g_cvarNull : nAllocationDetailListID;

				m_changed = true;

				UpdateTotal();
			}
			else if (quantity == total)//technically not valid, but try to guess what the user intended
			{	m_list->Value[m_rightClicked][O_DateReceived]	= _variant_t(GetDate(), VT_DATE);
				m_changed = true;
			}

			//update list
			if(quantity > 0 && quantity <= total) {
				long productID = VarLong(m_list->Value[m_rightClicked][O_ProductID]);
				// (j.jones 2008-02-07 16:47) - PLID 28851 - track whether this was consignment
				// (j.jones 2008-02-08 14:13) - PLID 28851 - these functions have been removed,
				// we no longer dynamically update the product totals
				/*
				BOOL bIsConsignment = VarLong(m_list->Value[m_rightClicked][O_ForStatus]) == InvUtils::odsConsignment;
				AddToItemReceived(productID, quantity, bIsConsignment);
				*/
			}
			else
				MsgBox("You entered an invalid value. Please enter a number between %li and %li.", 1, total);

			break;
		}

		return true;
	}

	}NxCatchAll("Error in CInvEditOrderDlg::OnCommand");

	return CNxDialog::OnCommand(wParam, lParam);
}

// (j.jones 2008-02-07 10:20) - PLID 28851 - there are two types of auto-ordering,
// purchased inventory and consignment, so require a boolean to be sent in to determine
// which "auto order" to call
void CInvEditOrderDlg::AutoOrder(long supplier, BOOL bConsignmentOrder)
{
	m_supplier->SetSelByColumn(S_ID, supplier);
	m_iSupplierSel = m_supplier->CurSel;
	HandleNewSupplier(m_supplier->CurSel);

	//order based on the boolean
	if(bConsignmentOrder) {
		OnAutoConsign();
	}
	else {
		OnAutoPurchInv();
	}
}

// (j.jones 2008-02-20 13:17) - PLID 28983 - renamed this function to more accurately explain what it does,
// now that we have the ToOrder column that is updated within this function
void CInvEditOrderDlg::UpdateItemListColorsAndToOrderAmt()
{
	try {
		IRowSettingsPtr pRow;
		int count = m_item->GetRowCount();
		long	ordered;		
		double	reorderPoint,
				actual,
				available;

		// (j.jones 2008-02-14 11:42) - PLID 28864 - added option for when to order products, when the
		// actual (0) hits the reorder point, or when the available (1) hits it
		long nOrderByOnHandAmount = GetRemotePropertyInt("InvItem_OrderByOnHandAmount", 0, 0, "<None>", true);
		BOOL bOrderByActual = (nOrderByOnHandAmount == 0);

		for (int i = 0; i < count; i++) {
			
			pRow = m_item->Row[i];

			// (j.jones 2006-10-12 14:59) - PLID 23006 - If null, it means
			// this item only tracks orders, not quantity, so color black
			if(pRow->Value[I_ActualPurchased].vt == VT_NULL && pRow->Value[I_ActualConsign].vt == VT_NULL) {
				continue;
			}

			// (j.jones 2008-02-07 16:27) - PLID 28851 - color code the cells appropriately,
			// general vs. consignment, and color the product name with the more severe color

			long nProductOrderStatus = 0; //0 - none, 1 - ordered, 2 - needs ordered

			COLORREF cNeedOrder = RGB(255,0,0);
			COLORREF cOnOrder = RGB(255,159,15);
			COLORREF cNormal = RGB(0,0,0);

			//first, check the general info
			{
				actual = VarDouble(pRow->Value[I_ActualPurchased],0.0);
				available = VarDouble(pRow->Value[I_AvailPurchased],0.0);
				ordered = VarLong(pRow->Value[I_OrderedPurchased],0);
				reorderPoint = VarDouble(pRow->Value[I_ReorderPointPurchased],0.0);
				long reorderQuantity = VarLong(pRow->Value[I_ReorderQuantityPurchased],1);
				if(reorderQuantity <= 0) {
					reorderQuantity = 1;
				}

				// (j.jones 2008-02-14 11:45) - PLID 28864 - use the preference to calculate where
				// we look at actual or available
				double dblValueToCheck = 0.0;
				if(bOrderByActual) {
					dblValueToCheck = actual;
				}
				else {
					dblValueToCheck = available;
				}

				COLORREF cColor = cNormal;

				long nToOrder = 0;

				// (j.jones 2008-02-19 12:23) - PLID 28981 - for purchased inventory,
				// we order when under the reorder point or equal to the reorder point,
				// so we continue with the existing logic of ordering when
				// dblValueToCheck + ordered is <= reorderPoint
				if (dblValueToCheck + ordered <= reorderPoint && reorderPoint != 0) {
					//JJ - if the total in stock plus the amount ordered is not enough,
					//color the row red, warning, must order, etc.
					cColor = cNeedOrder;

					nProductOrderStatus = 2;

					// (j.jones 2008-02-20 13:07) - PLID 28983 - update the "ToOrder" column appropriately
					nToOrder = ReorderAmountPurchased(reorderPoint - dblValueToCheck - ordered, reorderQuantity);					
				}
				else if (dblValueToCheck <= reorderPoint && reorderPoint != 0) {
					//JJ - if the above is not true, then everything is okay OR this case is true
					//the amt. on hand is less than reorder, but dblValueToCheck + ordered may be enough
					//color orange, for caution, but it won't auto-order
					cColor = cOnOrder;
					nProductOrderStatus = 1;
				}
				else if (dblValueToCheck > reorderPoint) {
					//JJ - normally we don't need to color this, but if we dynamically change the order
					//we will need to set the color to black
					cColor = cNormal;

					nProductOrderStatus = 0;
				}

				pRow->PutCellForeColor(I_ActualPurchased, cColor);
				pRow->PutCellForeColor(I_AvailPurchased, cColor);
				pRow->PutCellForeColor(I_OrderedPurchased, cColor);
				pRow->PutCellForeColor(I_ReorderPointPurchased, cColor);
				pRow->PutCellForeColor(I_ToOrderPurchased, cColor);

				// (j.jones 2008-02-20 13:07) - PLID 28983 - update the "ToOrder" column appropriately
				pRow->PutValue(I_ToOrderPurchased, nToOrder);
			}

			//now check the consignment info
			{
				actual = VarDouble(pRow->Value[I_ActualConsign],0.0);
				available = VarDouble(pRow->Value[I_AvailConsign],0.0);
				ordered = VarLong(pRow->Value[I_OrderedConsign],0);
				reorderPoint = VarDouble(pRow->Value[I_ReorderPointConsign],0.0);				

				// (j.jones 2008-02-14 11:45) - PLID 28864 - use the preference to calculate where
				// we look at actual or available
				double dblValueToCheck = 0.0;
				if(bOrderByActual) {
					dblValueToCheck = actual;
				}
				else {
					dblValueToCheck = available;
				}

				COLORREF cColor = cNormal;

				long nToOrder = 0;

				// (j.jones 2008-02-19 12:23) - PLID 28981 - for consignment,
				// we only order when under the reorder point, not when we're at
				// that level, so only order when dblValueToCheck + ordered is < reorderPoint,
				// not equal to reorderPoint
				if (dblValueToCheck + ordered < reorderPoint && reorderPoint != 0) {
					//JJ - if the total in stock plus the amount ordered is not enough,
					//color the row red, warning, must order, etc.
					cColor = cNeedOrder;

					nProductOrderStatus = 2;

					// (j.jones 2008-02-20 13:07) - PLID 28983 - update the "ToOrder" column appropriately
					nToOrder = ReorderAmountConsignment(reorderPoint - dblValueToCheck - ordered);
				}
				else if (dblValueToCheck < reorderPoint && reorderPoint != 0) {
					//JJ - if the above is not true, then everything is okay OR this case is true
					//the amt. on hand is less than reorder, but dblValueToCheck + ordered may be enough
					//color orange, for caution, but it won't auto-order
					cColor = cOnOrder;
					
					if(nProductOrderStatus < 1) {
						nProductOrderStatus = 1;
					}
				}
				else if (dblValueToCheck >= reorderPoint) {
					//JJ - normally we don't need to color this, but if we dynamically change the order
					//we will need to set the color to black
					cColor = cNormal;
				}

				pRow->PutCellForeColor(I_ActualConsign, cColor);
				pRow->PutCellForeColor(I_AvailConsign, cColor);
				pRow->PutCellForeColor(I_OrderedConsign, cColor);
				pRow->PutCellForeColor(I_ReorderPointConsign, cColor);
				pRow->PutCellForeColor(I_ToOrderConsign, cColor);

				// (j.jones 2008-02-20 13:07) - PLID 28983 - update the "ToOrder" column appropriately
				pRow->PutValue(I_ToOrderConsign, nToOrder);
			}

			//now color the name
			if(nProductOrderStatus == 2) {
				pRow->PutCellForeColor(I_Name, cNeedOrder);
			}
			else if(nProductOrderStatus == 1) {
				pRow->PutCellForeColor(I_Name, cOnOrder);
			}
			else {
				pRow->PutCellForeColor(I_Name, cNormal);
			}
		}
	}NxCatchAll("CInvEditOrderDlg::UpdateItemListColorsAndToOrderAmt");
}
void CInvEditOrderDlg::OnRequeryFinishedItem(short nFlags) 
{
	try {

		// (j.jones 2008-02-08 14:13) - PLID 28851 - this function has been removed,
		// we no longer dynamically update the product totals
		/*
		AddUnsavedItemsToItemsOnOrder();
		*/

		UpdateItemListColorsAndToOrderAmt();

	}NxCatchAll("Error in CInvEditOrderDlg::OnRequeryFinishedItem()");
}

// (c.haag 2008-01-10 17:44) - PLID 28592 - Added nDefaultStatus so that the Purchased/Consignment status in the product item dialog
// would be consistent with that of the product's default
//TES 6/18/2008 - PLID 29578 - Changed the first parameter from OrderID to nOrderDetailID
BOOL CInvEditOrderDlg::ReceiveProductItems(long nOrderDetailID, long ProductID, long Quantity, long nDefaultStatus) {

	try {

		//TES 7/3/2008 - PLID 24726 - Added SerialNumIsLotNum
		_RecordsetPtr rs = CreateParamRecordset("SELECT "
			"ServiceT.Name, "
			"ProductT.HasSerialNum, ProductT.SerialNumIsLotNum, ProductT.HasExpDate, ProductT.UseUU, ProductT.SerialPerUO, ProductT.Conversion "
			"FROM ProductT "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"WHERE ProductT.ID = {INT} AND (ProductT.HasSerialNum = 1 OR ProductT.HasExpDate = 1)", ProductID);

		if(rs->eof)
			//if no applicable products, return true and move on
			return TRUE;

		std::vector<CString> aryAlreadyReceivedProducts;

		//loop through all products on this order with one of these properties
		while(!rs->eof) {
			
			BOOL bHasSerial, bSerialNumIsLotNum, bHasExpDate, bUseUU, bSerialPerUO;

			bHasSerial = AdoFldBool(rs, "HasSerialNum",FALSE);
			bSerialNumIsLotNum = AdoFldBool(rs, "SerialNumIsLotNum",FALSE);
			bHasExpDate = AdoFldBool(rs, "HasExpDate",FALSE);
			bUseUU = AdoFldBool(rs, "UseUU",FALSE);
			bSerialPerUO = AdoFldBool(rs, "SerialPerUO",FALSE);
			long Conversion = AdoFldLong(rs, "Conversion",1);

			//safety check: has another user already received these products?
			if (InvUtils::HasOrderDetailReceivedProductItems(nOrderDetailID)) {
				//someone has already received this product
				CString strMessage;
				strMessage.Format("The product %s has already been received. You may need to close this window and attempt receiving the order again to ensure the order status has been updated.", AdoFldString(rs, "Name"));
				AfxMessageBox(strMessage, MB_ICONERROR | MB_OK);
				return FALSE;
			}

			// (d.thompson 2009-10-21) - PLID 36015 - Create your own dialog, don't
			//	use the shared one anymore.
			//CProductItemsDlg& dlg = GetMainFrame()->GetProductItemsDlg();
			CProductItemsDlg dlg(this);
			//DRT 11/15/2007 - PLID 28008 - All ENTER_DATA types must not allow change of qty
			dlg.m_bDisallowQtyChange = TRUE;
			dlg.m_bUseSerial = bHasSerial;
			dlg.m_bUseExpDate = bHasExpDate;
			dlg.m_ProductID = ProductID;
			// (r.farnworth 2016-03-24 14:19) - PLID 68627 - We need to be passing the location ID along
			dlg.m_nLocationID = VarLong(m_location->GetValue(m_location->GetCurSel(), L_ID));
			//TES 6/18/2008 - PLID 29578 - This takes an OrderDetailID, not an OrderID
			dlg.m_nOrderDetailID = nOrderDetailID;
			dlg.m_NewItemCount = Quantity;
			dlg.m_bUseUU = bUseUU;
			dlg.m_bSerialPerUO = bSerialPerUO;
			dlg.m_nConversion = Conversion;
			dlg.m_nDefaultStatus = nDefaultStatus; // (c.haag 2008-01-11 11:49) - PLID 28592 - Assign the default Purchased/Consignment status
			// (c.haag 2008-06-04 10:53) - PLID 29013 - Don't let the user select "< No Location >"
			dlg.m_bDisallowNonLocations = TRUE;
			dlg.m_bDisallowLocationChange = TRUE; // (c.haag 2008-06-25 12:29) - PLID 28438 - Ensure the location column is read-only; every location must be the same
			dlg.m_bSaveDataEntryQuery = false;	// (j.jones 2009-04-01 09:39) - PLID 33559 - make sure this is set to false, since we are borrowing an existing dialog from mainframe
			dlg.m_strSavedDataEntryQuery = ""; // (j.jones 2009-07-09 17:59) - PLID 34842 - make sure this gets cleared!

			if(IDCANCEL == dlg.DoModal())
				return FALSE;

			rs->MoveNext();
		}

		rs->Close();

		return TRUE;

	}NxCatchAll("Error in ReceiveProductItems");

	return FALSE;
}
void CInvEditOrderDlg::OnTrySetSelFinishedLocation(long nRowEnum, long nFlags) 
{
	try {
		//This is probably an inactive location.
		_RecordsetPtr rsLocName = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = (SELECT LocationID FROM OrderT WHERE ID = %li)", m_id);
		if(!rsLocName->eof) {
			m_location->PutComboBoxText(_bstr_t(AdoFldString(rsLocName, "Name")));
		}
	}NxCatchAll("Error in CInvEditOrderDlg::OnTrySetSelFinishedLocation()");
}

void CInvEditOrderDlg::OnKillFocusDateDue() 
{
	if(m_nxtArrivalDate->GetStatus() == 2) {//invalid
		m_nxtArrivalDate->Clear();
	}
	
}

void CInvEditOrderDlg::OnKillFocusDate() 
{
	//TES 6/4/03:  This is semi-odd functionality, but it's always worked like this.
	if(m_nxtDate->GetStatus() != 1) {//invalid
		m_nxtDate->SetDateTime(COleDateTime::GetCurrentTime());
	}
}

//returns TRUE if the save succeeds, FALSE otherwise
//I like this name so much, I borrowed it from the scheduler.
BOOL CInvEditOrderDlg::ValidateSaveAndClose()
{
	//r.wilson 
	BOOL m_bFrameOrderReceptionTmp = FALSE;

	try {
		

		if (!m_list->GetRowCount())//just delete an empty order
		{	
			if(IDYES == MsgBox(MB_YESNO, "You cannot save an order with no items.  Would you like to delete this order?")) {
				DoDeleteOrder();
				OnCancelBtn();
			}
			return FALSE; //Whether they delete it or not, it has failed validation.
		}
		if (m_supplier->CurSel == -1)
		{	AfxMessageBox("You cannot create an order without selecting a supplier");
			return FALSE;
		}
		if (m_location->CurSel == -1 && !m_location->IsComboBoxTextInUse)
		{	AfxMessageBox("You cannot create an order without selecting a location");
			return FALSE;
		}

		//validate that it is safe to delete items
		if (m_aryOrderDetailsToDelete.size() > 0)
		{
			//safety check: don't delete any items that had received product items
			if (InvUtils::HasOrderDetailReceivedProductItems(m_aryOrderDetailsToDelete)) {
				//someone has already received a product
				AfxMessageBox("At least one product you deleted has already been received. You may need to close this window and attempt editing the order again to ensure the order status has been updated.", MB_ICONERROR | MB_OK);
				return FALSE;
			}
		}

		COleDateTime dtDate, dtDue;
		CString str;

		COleDateTime dtOld;
		dtOld.SetDateTime(1753,1,1,1,1,1);
		//first check to see if the order date is valid
		if(m_nxtDate->GetStatus() != 1) {
			AfxMessageBox("Please enter a valid order date before saving.");
			return FALSE;
		}
		else {
			//now check to see if the order date is in the future, which isn't necessarily illegal
			dtDate = m_nxtDate->GetDateTime();
			if(dtDate < dtOld) {
				AfxMessageBox("Please enter a date greater than 1753.");
				return FALSE;
			}

			if(dtDate > COleDateTime::GetCurrentTime()) {
				if(IDNO==MessageBox("Your order date is in the future. Are you sure you wish to save?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return FALSE;
				}
			}
		}	

		// (d.thompson 2009-01-23) - PLID 32823 - Ensure that if they checked the "vendor confirmed" button, that they provide a date.
		if(IsDlgButtonChecked(IDC_VENDOR_CONFIRMED_CHK)) {
			if(m_nxtDateConfirmed->GetStatus() != 1) {
				AfxMessageBox("You must provide a confirmation date if you enable the vendor confirmation option.");
				return FALSE;
			}
		}

		// (d.thompson 2009-01-27) - PLID 32823 - Ensure the confirmation date is not stupidly old
		COleDateTime dtConf = m_nxtDateConfirmed->GetDateTime();
		if(dtConf < dtOld) {
			AfxMessageBox("Please enter a confirmation date greater than 1753.");
			return FALSE;
		}

		//lastly, check to see if the date due is earlier than the order date, which is not allowed at all
		if ((m_nxtArrivalDate->GetStatus() == 1) && m_nxtArrivalDate->GetDateTime() < dtDate) {
			AfxMessageBox("Your date due is earlier than your order date. Please fix these dates before saving.");
			return FALSE;
		}

		CString strItems = "";
		for(int i=0;i<m_list->GetRowCount();i++) {

			// (j.jones 2008-06-19 11:06) - PLID 10394 - disallow saving negative totals
			if(VarCurrency(m_list->GetValue(i,O_Total)) < COleCurrency(0,0)) {
				AfxMessageBox("At least one product has a negative total cost. You must correct this before saving.");
				return FALSE;
			}

			if(VarLong(m_list->GetValue(i,O_Quantity),-1) == 0) {
				CString str = VarString(m_list->GetValue(i,O_Name),"");
				str += "\n";
				strItems += str;
			}
		}

		if(!strItems.IsEmpty()) {
			CString str;
			str.Format("You are ordering a zero quantity for:\n\n%s\nDo you still wish to save?",strItems);
			if(IDNO == MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
				return FALSE;
			}
		}

	}NxCatchAll("Could not validate inventory order.");


	//r.wilson 
	m_bFrameOrderReceptionTmp = m_bFrameOrderReception;
	m_bFrameOrderReception = FALSE;
	
	long nAuditTransactionID = -1;

	try {

		CWaitCursor pWait;

		EnsureRemoteData();

		// (j.jones 2008-03-03 17:54) - PLID 29181 - SaveOrder and ApplyItemsToOrder
		// both need location ID, which may be calculated from the database if the location
		// was inactive. So do that once, outside the transaction, and pass in the value
		// to be used.

		long nLocationID = -1;

		//moved SaveOrder logic into this outer function
		if(m_location->IsComboBoxTextInUse) {
			if (IsDlgButtonChecked(IDC_ENABLE_SHIP_TO)) {
				_RecordsetPtr rsLocID = CreateRecordset("SELECT LocationID FROM OrderT WHERE ID = %li", m_id);
				//This will throw an exception if anything goes wrong; orders have to have locations.
				nLocationID = AdoFldLong(rsLocID, "LocationID");			
			}
			else {

				_RecordsetPtr rsSoldToID = CreateRecordset("SELECT LocationSoldTo FROM OrderT WHERE ID = %li", m_id);
				nLocationID = AdoFldLong(rsSoldToID, "LocationSoldTo");			
			}		
		}
		else {
			nLocationID = m_location->Value[m_location->CurSel][L_ID];
		}

		//we wouldn't have reached this point if the location ID was invalid, but check anyways
		if(nLocationID == -1) {
			ThrowNxException("Tried to save with no location selected!");
		}

		// (j.jones 2008-03-20 12:13) - PLID 29311 - track whether we fully received this order
		BOOL bFullyReceived = FALSE;

		// (j.jones 2008-09-25 14:56) - PLID 30636 - Are we changing our appointment link to
		// link to one with an open allocation? If so, ask if we should add our products to it.
		long nAddToAllocationID = -1;
		CString strPatientName = "";
		long nPatientID = -1;
		if(m_nApptID != -1 && m_nOrigLinkedApptID != m_nApptID) {
			//FindApptAllocationID will search for allocations and prompt the user
			//to add the products to one - it doesn't actually add the products yet,
			//it only returns the selected allocation ID.
			// (a.walling 2010-01-21 15:59) - PLID 37024
			FindApptAllocationID(m_nApptID, nAddToAllocationID, strPatientName, nPatientID);

			//ApplyItemsToOrder will take in the nAddToAllocationID and
			//add the products to that allocation, and will handle if
			//the products are received.

			//however, it will not put previously-received products into
			//the allocation, so we should warn about that
			if(nAddToAllocationID != -1 && IsOrderReceived()) {
				AfxMessageBox("This order currently has products that are marked as received. "
					"Practice will not add previously-received products to the allocation. "
					"Only unreceived products, and products being currently saved as received, "
					"will be added to the allocation.");
			}
		}

		//cache this because SaveOrder will change it
		long nOrigLinkedApptID = m_nOrigLinkedApptID;

		//TES 7/23/2008 - PLID 30802 - We need to know which allocation IDs (if any) get updated by our save, so that we
		// can tell the user about them after committing the transaction.
		CArray<long,long> arUpdatedAllocationIDs;

		try {
			// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
			CSqlTransaction trans("SaveInventoryOrder");
			
			trans.Begin();
			SaveOrder(nLocationID);
			// (j.jones 2008-09-26 12:41) - PLID 30636 - I added parameters to support adding to allocations,
			// including nAuditTransactionID, which right now is not used by anything but allocation adding
			bFullyReceived = ApplyItemsToOrder(nLocationID, arUpdatedAllocationIDs, nAddToAllocationID, strPatientName, nPatientID, nAuditTransactionID);

			trans.Commit();
			
			// (r.wilson 2-20-2012) PLID 48222
			/*Calling the function below will prompt the user to print a frame label so we only
			 want to get in there if this is indeed a frame order.*/
			if(m_bFrameOrderReceptionTmp)
			{
				m_pParent->SetReturnedOrderId(m_id);
			}


		}NxCatchAllCall("Could not save order", 
			if(nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}

			return FALSE;
		);

		//TES 7/23/2008 - PLID 30802 - Now pop up the allocations.
		if(arUpdatedAllocationIDs.GetSize()) {
			//TES 7/23/2008 - PLID 30802 - Let them know what's going on.
			// (j.jones 2008-09-26 12:56) - PLID 30636 - tweaked the message if we saved into an allocation
			if(nAddToAllocationID != -1) {
				MsgBox("The changes made to this order (i.e. adding to an allocation, receiving products) "
					"have caused details to be updated on %i allocation(s).  Each updated allocation will now "
					"be displayed, so that you can review and confirm the changes.", arUpdatedAllocationIDs.GetSize());
			}
			else {
				MsgBox("Marking this order received has caused details to be updated on %i allocation(s).  Each updated allocation will now "
					"be displayed, so that you can review and confirm the changes.", arUpdatedAllocationIDs.GetSize());
			}
		}
		for(int nAllocation = 0; nAllocation < arUpdatedAllocationIDs.GetSize(); nAllocation++) {
			CInvPatientAllocationDlg dlg(this);
			dlg.m_nID = arUpdatedAllocationIDs[nAllocation];
			dlg.DoModal();
		}

		// (c.haag 2008-06-11 14:45) - PLID 28474 - Update any todo alarms related to the order's Date Due
		InvUtils::UpdateOrderTodoAlarms(m_id);

		// (c.haag 2007-11-14 11:07) - PLID 28094 - Fire a table checker
		CClient::RefreshTable(NetUtils::OrderT, m_id);

		// (j.jones 2008-03-24 15:25) - PLID 29388 - need to update the linked appointment
		if(m_nApptID != -1) {
			TrySendAppointmentTablecheckerForInventory(m_nApptID, FALSE);
		}
		//also need to update the old appointment, if it changed
		if(nOrigLinkedApptID != -1 && nOrigLinkedApptID != m_nApptID) {
			TrySendAppointmentTablecheckerForInventory(nOrigLinkedApptID, FALSE);
		}

		//TES 6/13/2008 - PLID 28078 - m_pParent can now be NULL.
		if(m_pParent) m_pParent->UpdateView();
		OnCancelBtn();

		// (j.jones 2008-03-20 12:13) - PLID 29311 - if we just received the last product on the
		// order, and it is linked to an appointment, ask if they wish to create an allocation
		// (j.jones 2008-09-29 13:27) - PLID 30636 - don't ask if we already added to an allocation
		if(m_bHasAdvInventory
			&& m_nApptID != -1
			&& nAddToAllocationID == -1
			&& bFullyReceived
			&& IDYES == MessageBox("This completed order has a linked appointment. Would you like to create an allocation with these products for that appointment?",
			"Practice", MB_ICONQUESTION|MB_YESNO)) {

			InvUtils::TryCreateAllocationFromOrder(m_id);					
		}

		return TRUE;
	
	}NxCatchAll("Error in CInvEditOrderDlg::ValidateSaveAndClose");

	return FALSE;

}


void CInvEditOrderDlg::OnSelChosenSupplier(long nRow) 
{
	try {
		HandleNewSupplier(nRow);

	}NxCatchAll("Error in CInvEditOrderDlg::OnSelChosenSupplier()");

}


void CInvEditOrderDlg::HandleNewSupplier(long nSupplierRow, bool bLoading /*= false*/)
{ 
	try {
		//if no items in the list, then it's okay
		if(m_list->GetRowCount() > 0 && m_supplier->CurSel != m_iSupplierSel) {

			// (c.haag 2008-05-21 15:49) - PLID 30082 - Don't let them clear the list if at least one item is marked
			// as received, or it will throw ProductItemsT out of sync with the order history
			if (m_id > 0) {
				_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM OrderDetailsT WHERE OrderID = {INT} AND DateReceived IS NOT NULL", m_id);
				if (!prs->eof) {
					// At least one order detail is "received" in data; so prevent the supplier change
					AfxMessageBox("You may not add items from another supplier to an order which has at least one item that is marked as received in your data.\n"
						"\n"
						"If you wish to add items from another supplier, you must first create a new order.", MB_ICONERROR);
					m_supplier->CurSel = m_iSupplierSel;
					return;
				}
			}

			if(IDYES == MessageBox("You have already started to order items from the previously selected supplier.\n"
				"If you continue, all items will be removed from this order.\n\n"
				"Do you wish to clear out this order and select the new supplier?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {

				//reflects the deleted items in the item combo on order totals
				//as we don't actually delete the items till the user hits save, we have to change this dynamically
				// (j.jones 2008-02-08 14:13) - PLID 28851 - this function has been removed,
				// we no longer dynamically update the product totals
				/*
				for(int i=0; i<m_list->GetRowCount(); i++) {
					RemoveUnsavedItemFromOrder(i);								
				}
				*/
				UpdateItemListColorsAndToOrderAmt();

				m_list->Clear();
				//TES 7/22/2008 - PLID 30802 - Clear out our stored lists of allocation detail IDs.
				ClearAllocationDetailIDs();
				UpdateTotal();
				m_changed = true;
			}
			else {
				m_supplier->CurSel = m_iSupplierSel;
				return;
			}
		}

	}NxCatchAll("Error changing supplier.");	

	if(m_supplier->CurSel == -1) {
		m_supplier->CurSel = m_iSupplierSel;		
	}
	if(m_supplier->CurSel == -1) {
		m_supplier->CurSel = 0;
	}
	if(m_supplier->CurSel == -1 && m_supplier->GetRowCount() == 0) {
		AfxMessageBox("You have no suppliers to choose from. Please enter a Supplier in the Contacts module.");
		return;
	}

	m_iSupplierSel = m_supplier->CurSel;

	CString		from;
	long		loc,
				supplier;
	_variant_t	value;

	try {

		if(m_location->CurSel == -1 && m_id != -1) {
			//This must be the same inactive location we started with.
			_RecordsetPtr rsLocID = CreateRecordset("SELECT LocationID FROM OrderT WHERE ID = %li", m_id);
			//This will throw an exception if anything goes wrong; orders have to have locations.
			// (j.jones 2008-07-03 16:45) - PLID 30624 - throw a unique exception if eof, this
			// should not be allowed
			if(rsLocID->eof) {
				ThrowNxException("You must have a ship to location selected!");
			}
			loc = AdoFldLong(rsLocID, "LocationID");
		}
		else {
			loc = VarLong(m_location->GetValue(m_location->GetCurSel(), 0));
		}

		supplier = VarLong(m_supplier->Value[m_supplier->CurSel][0]);

		// (c.haag 2007-11-07 10:59) - PLID 27994 - We now pull the from clause from a utility function
		from = InvUtils::GetItemDropdownSqlFromClause(loc, supplier);

		//TES 5/28/2008 - PLID 30165 - Remember whether this supplier had "On File" checked on the last order we entered for
		// them.
		//TES 8/13/2008 - PLID 30165 - Only do this if we're not loading an existing order!
		if(!bLoading) {
			RememberCCOnFile();
		}

	}NxCatchAll("Error in CInvEditOrderDlg::HandleNewSupplier()");

	try
	{	long id = -1;

		if (m_item->CurSel != -1) {
			id = m_item->Value[m_item->CurSel][0].lVal;
		}
		m_item->FromClause = _bstr_t(from);
		m_item->Requery();
	}NxCatchAll("Could not set item drop down for supplier and location");
}

void CInvEditOrderDlg::DoDeleteOrder()
{
	try
	{
		// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
		CSqlTransaction trans("InvOrderDelete");

		trans.Begin();
		ExecuteSql ("UPDATE OrderDetailsT SET ModifiedBy = %li "
			"WHERE OrderID = %i AND Deleted = 0", 
			GetCurrentUserID(), m_id);
		// (c.haag 2008-06-11 14:55) - PLID 28474 - Also delete todo alarms
		TodoDelete(FormatString("RegardingType = %d AND RegardingID = %d", ttInvOrder, m_id));
		ExecuteSql ("UPDATE OrderDetailsT SET Deleted = 1 WHERE OrderID = %li", m_id);
		ExecuteSql ("UPDATE OrderT SET Deleted = 1 WHERE ID = %li", m_id);
		//TES 7/22/2008 - PLID 30802 - Clear out any allocation details that happen to have been linked to the details 
		// we just deleted
		ExecuteSql("UPDATE PatientInvAllocationDetailsT SET OrderDetailID = NULL WHERE OrderDetailID IN (SELECT ID FROM "
			"OrderDetailsT WHERE OrderID = %li)", m_id);
		// (j.jones 2008-03-18 15:38) - PLID 29309 - delete from OrderAppointmentsT
		// (j.jones 2008-03-19 12:21) - PLID 29316 - no need to audit the link deletion when the order is deleted
		ExecuteParamSql("DELETE FROM OrderAppointmentsT WHERE OrderID = {INT}", m_id);
		// (c.haag 2007-12-04 09:09) - PLID 28264 - Delete records from ProductItemsStatusHistoryT
		//TES 6/18/2008 - PLID 29578 - ProductItemsT now has an OrderDetailID, not an OrderID
		ExecuteSql ("DELETE FROM ProductItemsStatusHistoryT WHERE ProductItemID IN (SELECT ID FROM ProductItemsT WHERE OrderDetailID IN (SELECT ID FROM OrderDetailsT WHERE OrderID = %d))", m_id);
		// (j.jones 2008-06-06 10:00) - PLID 27110 - delete referencing product items first
		// (j.jones 2008-09-10 14:06) - PLID 31320 - Scratch that, we should not try to delete products items
		// that have references, and we should catch it earlier in this code. Do not try to delete here.
		//ExecuteParamSql("DELETE FROM ProductItemsT WHERE ReturnedFrom IN (SELECT ID FROM ProductItemsT WHERE OrderDetailID IN (SELECT ID FROM OrderDetailsT WHERE OrderID = %li))", id);
		ExecuteSql ("DELETE FROM ProductItemsT WHERE OrderDetailID IN (SELECT ID FROM OrderDetailsT WHERE OrderID = %li)", m_id);
		trans.Commit();

		long AuditID = -1;
		AuditID = BeginNewAuditEvent();
		if(AuditID != -1) {
			_RecordsetPtr rs = CreateRecordset("SELECT Description, Date, Company FROM OrderT "
			"LEFT JOIN PersonT ON OrderT.Supplier = PersonT.ID WHERE OrderT.ID = %li", m_id);
			
			if(!rs->eof) {
				//(e.lally 2006-11-13) PLID 23438 - Orders with no supplier will have a null value that must be accounted for.
				//The interface prevents new data from having no supplier, but the tables do not.
				CString strDesc;
				strDesc.Format("Order: %s, Supplier: %s",AdoFldString(rs, "Description"),
					AdoFldString(rs, "Company", "<None>"));

				AuditEvent(-1, AdoFldString(rs, "Description"),AuditID,aeiOrderDeleted,m_id,strDesc,"<Deleted>",aepHigh,aetDeleted);
			
			}
			rs->Close();
		}

		// (c.haag 2007-11-14 11:07) - PLID 28094 - Fire a table checker
		CClient::RefreshTable(NetUtils::OrderT, m_id);

		// (j.jones 2008-03-24 15:25) - PLID 29388 - need to update the linked appointment
		// we haven't saved the order, so we need to send the tablecheckers for the previously loaded appt. ID
		if(m_nOrigLinkedApptID != -1) {
			TrySendAppointmentTablecheckerForInventory(m_nOrigLinkedApptID, FALSE);
		}

		//TES 6/13/2008 - PLID 28078 - m_pParent can now be NULL.
		if(m_pParent) m_pParent->UpdateView();
		OnCancelBtn();
		return;
	}
	NxCatchAll("Could not delete order");
}

bool CInvEditOrderDlg::IsOrderReceived()
{
	//The order is received if any line on it is received.
	for(int i = 0; i < m_list->GetRowCount(); i++) {
		if(m_list->GetValue(i, O_DateReceived).vt == VT_DATE) {
			return true;
		}
	}
	return false;
}

void CInvEditOrderDlg::OnCheckTaxShipping() 
{
	UpdateTotal();
}

BOOL CInvEditOrderDlg::ValidateUU_UOStatus(long nOrderDetailID)
{
	try {

		//Run through each item and ensure that its UseUU status and Conversion ratio
		//is identical to the item's current settings. If not, ask the user how to proceed.

		// (j.jones 2008-04-01 12:03) - PLID 28430 - if the conversion changed but the
		// OrderDetailsT.UseUU is not enabled, we need not warn
		_RecordsetPtr rs = CreateRecordset("SELECT OrderDetailsT.ID, "
			"OrderDetailsT.UseUU AS OrderUseUU, OrderDetailsT.Conversion AS OrderConversion, "
			"ProductT.UseUU AS ProductUseUU, ProductT.Conversion AS ProductConversion "
			"FROM OrderDetailsT "
			"INNER JOIN ProductT ON OrderDetailsT.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"WHERE OrderDetailsT.ID = %li AND Deleted = 0 AND DateReceived Is Null "
			"AND ((OrderDetailsT.UseUU <> ProductT.UseUU) OR (OrderDetailsT.UseUU <> 0 AND OrderDetailsT.Conversion <> ProductT.Conversion))", nOrderDetailID);

		while(!rs->eof) {
			//for each item returned, there's a difference, so inform the user of the difference,
			//and give the option to use the old data, the new data, or cancel receiving altogether
			long nOrderDetailID = AdoFldLong(rs, "ID");
			BOOL bOrderUseUU = AdoFldBool(rs, "OrderUseUU",FALSE);
			BOOL bProductUseUU = AdoFldBool(rs, "ProductUseUU",FALSE);
			long nOrderConversion = AdoFldLong(rs, "OrderConversion",1);
			long nProductConversion = AdoFldLong(rs, "ProductConversion",1);

			CString strWarning, strOrder1, strOrder2, strProduct1, strProduct2;

			if(bOrderUseUU) {
				strOrder1.Format("enabled with a Conversion rate of %li",nOrderConversion);
				strOrder2.Format("a Conversion rate of %li",nOrderConversion);
			}
			else { 
				strOrder1 = "disabled";
				strOrder2 = "no UU/UO conversion";
			}

			if(bProductUseUU) {
				strProduct1.Format("enabled with a Conversion rate of %li",nProductConversion);
				strProduct2.Format("a Conversion rate of %li",nProductConversion);
			}
			else {
				strProduct1 = "disabled";
				strProduct2 = "no UU/UO conversion";
			}

			if(!bOrderUseUU && !bProductUseUU) {
				//no need to warn
				return TRUE;
			}

			strWarning.Format("This product was ordered with the 'Use Unit Of Order / Unit Of Usage' tracking %s,\n"
				"but now currently has this tracking %s.\n\n"
				"Do you wish to receive this product with the configuration in which it was ordered?\n\n"
				"'Yes' will receive the product with %s.\n"
				"'No' will receive the product with %s.\n"
				"'Cancel' will cancel marking this product received.",strOrder1,strProduct1,strOrder2,strProduct2);

			int nResult = MessageBox(strWarning,"Practice",MB_ICONEXCLAMATION|MB_YESNOCANCEL);
			if(nResult == IDCANCEL) {
				return FALSE;
			}
			else if(nResult == IDNO) {
				//if 'No', they want to receive the product with the current configuration
				m_list->Value[m_rightClicked][O_UseUU] = bProductUseUU ? (long)1 : (long)0;
				m_list->Value[m_rightClicked][O_Conversion] = nProductConversion;
			}

			rs->MoveNext();
		}
		rs->Close();

		return TRUE;

	}NxCatchAll("Error validating UU/UO status.");

	return FALSE;
}

//DRT 3/7/2007 - PLID 24823 - We are receiving a barcode transmission.
LRESULT CInvEditOrderDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {

		CString str;
		//We do not need to deallocate this parameter, the mainfrm handling of WM_BARCODE_SCAN does it for us.
		// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
		_bstr_t bstrCode = (BSTR)lParam;

		//Convert BSTR to variant
		// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
		_variant_t varCode(bstrCode);
		//varCode.SetString((const char*)bstrCode);

		//Only allow 1 scan at a time.  There is some user interaction in this function, 
		//	so we'll stop all scanning until it finishes.
		if(m_bIsScanningBarcode) {

			// (j.jones 2009-02-10 12:13) - PLID 32870 - track that we ignored this barcode
			m_straryIgnoredBarcodes.Add(VarString(varCode));
			return 0;
		}
		// (c.haag 2007-12-06 09:00) - PLID 28286 - Ignore scans if the serialized item window is open
		if (-1 != m_nVisibleProductItemDlgRow) {
			// (j.jones 2009-02-10 12:13) - PLID 32870 - do not track that we skipped this barcode
			return 0;
		}

		long nLocationID = -1;
		if(m_location->CurSel == -1) {
			//This must be the same inactive location we started with.
			_RecordsetPtr rsLocID = CreateRecordset("SELECT LocationID FROM OrderT WHERE ID = %li", m_id);
			//This will throw an exception if anything goes wrong; orders have to have locations.
			nLocationID = AdoFldLong(rsLocID, "LocationID");
		}
		else {
			nLocationID = VarLong(m_location->GetValue(m_location->GetCurSel(), 0));
		}

		CWaitCursor wc;

		//
		//Our algorithm will be:
		//  0)  If the tracking number box has input focus, scan the code in there as a tracking number.
		//	1)  Search the list of items for the currently selected supplier, select if found.
		//	2)  If not found, see if we already have any items selected.  If yes, we just quit
		//		where we are.
		//	3)  If nothing is already selected, do a database lookup for any active product with this
		//		barcode.  If we find nothing, quit.
		//	4)  If we find something with just 1 supplier, select that supplier, requery the item list,
		//		and select the product.
		//	5)  If the product has 0 or > 1 suppliers, inform the user appropriately that they'll need to
		//	either assign a supplier or manually switch their order to the supplier they want.

		// (a.walling 2008-02-21 13:50) - PLID 29053 - Set tracking number if it has focus
		CNxEdit* pTrackingEditWnd = (CNxEdit*)GetDlgItem(IDC_TRACKING_NUMBER);
		CWnd* pFocusWnd = GetFocus();
		if (pFocusWnd == pTrackingEditWnd || pFocusWnd->GetSafeHwnd() == pTrackingEditWnd->GetSafeHwnd()) {
			pTrackingEditWnd->SetSel(0, -1);
			// using ReplaceSel allows Undo to still function.
			pTrackingEditWnd->ReplaceSel((LPCTSTR)bstrCode, TRUE);
			return 0;
		}

		//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
		//long nRow = m_item->FindByColumn(I_Barcode, varCode, 0, VARIANT_FALSE);
		long nRow = GetBestUPCProduct(varCode);
		if(nRow >= 0) {
			//Found it, so add it to the list.
			// (j.jones 2008-02-07 10:49) - PLID 28851 - no override needed here, just add normally
			// (a.walling 2008-03-20 09:23) - PLID 29333 - Flag to increase qty rather than add a new row
			AddItem(nRow, airNoRule, FALSE);
		}
		else {
			//Not found in this list.  Is there anything already in the list?  We don't want to mess up
			//	an order already in progress.
			if(m_list->GetRowCount() == 0) {
				//There is no order in progress, so this must be the first item.  If it exists on just 1 supplier, 
				//	go ahead and select that supplier, then select the item.  This becomes a sort of "short-cut".
				//	If it exists on more items, we'll just have to inform them to pick a supplier first, because
				//	it doesn't make any sense for us to guess.
				// (j.jones 2008-02-15 16:39) - PLID 28971 - properly filtered this recordset
				// so items that aren't trackable don't appear in the resultset
				_RecordsetPtr prsItem = CreateParamRecordset("SELECT ProductT.ID, COUNT(SupplierID) AS Cnt, MAX(MultiSupplierT.SupplierID) AS MaxSupplier "
					"FROM ProductT "
					"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
					"LEFT JOIN MultiSupplierT ON ProductT.ID = MultiSupplierT.ProductID "
					"LEFT JOIN PersonT ON MultiSupplierT.SupplierID = PersonT.ID "
					"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
					"WHERE Barcode = {STRING} AND ServiceT.Active = 1 "
					"AND TrackableStatus > 0 AND ProductLocationInfoT.LocationID = {INT} "
					"GROUP BY ProductT.ID", VarString(varCode), nLocationID);
				if(!prsItem->eof) {
					long nCnt = AdoFldLong(prsItem, "Cnt", 0);

					if(nCnt == 0) {
						//No suppliers, we cannot select this item.  Inform the user.
						MessageBox("The barcode you have scanned exists for a product in your system.  However, that product "
							"is not tied to any suppliers, and thus cannot be placed onto an order.\r\n"
							"Please add a supplier to your product before attempting to place an order.", "Inventory Order", MB_OK);
					}
					else if(nCnt > 1) {
						//More than 1 supplier.  We cannot guess, the user will have to pick a supplier themselves.
						MessageBox("The barcode you have scanned exists for a product in your system.  However, that product "
							"is tied to more than one supplier.\r\n"
							"Please select the supplier you wish to order from first, and then scan the barcode.", "Inventory Order", MB_OK);
					}
					else if(nCnt == 1) {
						//There is exactly 1 supplier for the scanned product.  There is nothing selected on our order currently.  We
						//	treat this as a "short-cut", and we'll just select the supplier for you.  Our query pulled out the max supplier
						//	ID (which in this case is the only supplier ID), so we don't need a second recordset.
						//If this asserts for NULL, then the item has a single supplier which is not the default, which is bad data.
						long nSupplierID = AdoFldLong(prsItem, "MaxSupplier");

						//select the supplier
						long nSupplierRow = m_supplier->SetSelByColumn(S_ID, (long)nSupplierID);
						if(nSupplierRow == -1) {
							//This will happen if your supplier is inactive.  We do not allow creating orders for inactive suppliers.
							MessageBox("The barcode you have scanned exists for a product in your system.  However, that product "
								"is tied to an inactive supplier.  NexTech Practice cannot create orders for inactive suppliers.  Please update "
								"the product to an active supplier and try scanning the code again.", "Inventory Order", MB_OK);
						}
						else {
							//Select the supplier.  This will cause the item list to requery
							HandleNewSupplier(nSupplierRow);

							//Wait on the item list to finish
							m_item->WaitForRequery(dlPatienceLevelWaitIndefinitely);

							//Now select by barcode
							//(c.copits 2010-09-24) PLID 40317 - Allow duplicate UPC codes for FramesData certification
							//long nItemRow = m_item->FindByColumn(I_Barcode, varCode, 0, VARIANT_FALSE);
							long nItemRow = GetBestUPCProduct(varCode);
							if(nItemRow >= 0) {
								//Found it, so add it to the list.
								// (j.jones 2008-02-07 10:49) - PLID 28851 - no override needed here, just add normally
								AddItem(nItemRow, airNoRule);
							}
							else {
								//This should not happen.  We filter out inactive products, so the only way to get here
								//	is if we detected a supplier/item combo, but somehow the requery failed to find it.
								ASSERT(FALSE);
								// (j.jones 2009-02-10 12:13) - PLID 32870 - track that we ignored this barcode
								m_straryIgnoredBarcodes.Add(VarString(varCode));
							}
						}
					}
				}
				else if(!FramesDataExists(VarString(varCode))) {
					// (b.spivey, September 14, 2011) - PLID 45265 - If a frame exists in framedata, we should create it.
						//eof, nothing found for this barcode, we do nothing

					// (j.jones 2009-02-10 12:13) - PLID 32870 - track that we ignored this barcode
					m_straryIgnoredBarcodes.Add(VarString(varCode));
				}
			}
			else if(!FramesDataExists(VarString(varCode))){
				// (b.spivey, September 21, 2011) - PLID 45265 - It only ever returns true if a frame was created, so 
				//		so if we get here it was either already created for a different supplier, or the client didn't 
				//		want to create it. 

				//An order is in progress, we can do nothing more

				// (j.jones 2009-02-10 12:13) - PLID 32870 - track that we ignored this barcode
				m_straryIgnoredBarcodes.Add(VarString(varCode));
			}
		}

		// (j.jones 2009-02-10 12:17) - PLID 32870 - if we have any ignored barcodes, warn the user
		// about them, but do not warn if there are still queued barcodes coming in from MainFrm

		BOOL bHasQueuedBarcodes = FALSE;
		if(GetMainFrame()) {
			bHasQueuedBarcodes = GetMainFrame()->HasQueuedBarcodes();
		}

		if(!bHasQueuedBarcodes && m_straryIgnoredBarcodes.GetSize() > 0) {
			//we have no more barcodes coming, and we have barcodes we ignored, so warn
			//the user of the first 10 barcodes that were skipped

			CString strWarn;
			if(m_straryIgnoredBarcodes.GetSize() == 1) {
				strWarn.Format("A barcode you scanned, %s, was not processed by this order. Please ensure your order is correct and is not missing any information.", m_straryIgnoredBarcodes.GetAt(0));
			}
			else {
				strWarn.Format("%li of the barcodes you scanned were not processed by this order. The barcodes that were skipped were:\n", m_straryIgnoredBarcodes.GetSize());
				int i=0;
				for(i=0; i<m_straryIgnoredBarcodes.GetSize() && i<10; i++) {
					strWarn += m_straryIgnoredBarcodes.GetAt(i);
					strWarn += "\n";
				}
				if(m_straryIgnoredBarcodes.GetSize() > 10) {
					strWarn += "<More...>\n";
				}
				strWarn += "\nPlease ensure your order is correct and is not missing any information.";
			}

			m_straryIgnoredBarcodes.RemoveAll();
			
			AfxMessageBox(strWarn);
		}

	}NxCatchAll("Error in CInvEditOrderDlg::OnBarcodeScan");

	//We are finished, allow new barcode scans to happen
	m_bIsScanningBarcode = FALSE;

	return 0;
}

void CInvEditOrderDlg::OnNewItemAdded(long nNewRow, long nItemComboRow)
{
	// (c.haag 2007-12-05 15:58) - PLID 28286 - This function is called when a new item is created

	// (c.haag 2007-12-05 14:59) - PLID 28286 - If this is the kind of order where
	// input items are automatically marked as received, and this is a serialized or
	// expirable item, then we need to prompt the user for a barcode for the product item.
	if (m_bSaveAllAsReceived) {
		BOOL bHasSerial = VarBool(m_list->Value[nNewRow][O_HasSerialNum], FALSE);
		//TES 7/3/2008 - PLID 24726 - We also need to pass in whether the serial number is a lot number.
		BOOL bSerialNumIsLotNum = VarBool(m_list->Value[nNewRow][O_SerialNumIsLotNum], FALSE);
		BOOL bHasExp = VarBool(m_list->Value[nNewRow][O_HasExpDate], FALSE);
		if (bHasSerial || bHasExp) {

			// Yes, we must invoke the product items prompt. You may think we just want
			// a modal dialog...but we need a persistent dialog because nothing is saved
			// until the very end. I'm copying off Don's design in the ReceiveOrderDlg.
			CProductItemsDlg* pDlg = (CProductItemsDlg*)VarLong(m_list->Value[nNewRow][O_DlgPtr], NULL);
			if (NULL == pDlg) {

				// The dialog doesn't exist yet. Create it now.
				pDlg = new CProductItemsDlg(this);
				pDlg->m_bModeless = true;

				// (j.jones 2009-02-11 08:53) - PLID 32871 - pass in our override color
				pDlg->m_clrBkg = AUTO_RECEIVE_COLOR;

				// (a.walling 2008-02-15 12:22) - PLID 28946 - Hide the consignment info if not licensed
				// Set up item-specific properties
				pDlg->m_bUseSerial = bHasSerial;
				pDlg->m_bUseExpDate = bHasExp;
				pDlg->m_ProductID = VarLong(m_list->Value[nNewRow][O_ProductID]);
				//TES 6/18/2008 - PLID 29578 - This now expects an OrderDetailId, not an OrderID
				pDlg->m_nOrderDetailID = VarLong(m_list->Value[nNewRow][O_ID]);
				pDlg->m_bUseUU = VarLong(m_list->Value[nNewRow][O_UseUU],0);
				pDlg->m_bSerialPerUO = VarBool(m_item->Value[nItemComboRow][I_SerialPerUO],FALSE);
				pDlg->m_nConversion = VarLong(m_list->Value[nNewRow][O_Conversion],1);
				//DRT 7/14/2008 - PLID 30688 - The default status needs to reference the ITEM row, not the
				//	order row.  This was previously pulling the default consignment from a random (and
				//	potentially invalid!!) row in the item list.
				pDlg->m_nDefaultStatus = (m_bHasAdvInventory && VarBool(m_item->Value[nItemComboRow][I_DefaultConsignment])) ? InvUtils::odsConsignment : InvUtils::odsPurchased;
				pDlg->m_NewItemCount = VarLong(m_list->Value[nNewRow][O_Quantity],1) * (pDlg->m_bUseUU ? pDlg->m_nConversion : 1);

				// Set dialog behavior
				pDlg->m_EntryType = PI_ENTER_DATA;
				pDlg->m_bSaveDataEntryQuery = true; // Don't let the dialog actually save anything;
													// we don't want that to happen until after the
													// order itself has been saved
				pDlg->m_strSavedDataEntryQuery = ""; // (j.jones 2009-07-09 17:59) - PLID 34842 - make sure this gets cleared!
				pDlg->m_bDisallowQtyChange = TRUE;	// (c.haag 2008-01-10 13:20) - Per Don: All ENTER_DATA types must not allow change of qty
				//DRT 7/14/2008 - PLID 30688 - We're unclear why this was set to not allow status changes.  I think
				//	it was just a miscommunication with c.haag that was never noticed in implementation.  You 
				//	must be allowed to change these, this dialog is the only important source for status when saving.
				pDlg->m_bDisallowStatusChange = false;
				pDlg->m_bDisallowLocationChange = TRUE; // (c.haag 2008-06-25 12:31) - PLID 28438 - The user may not change individual serialized item locations

				// Get the location
				if (m_location->CurSel >= 0) {
					pDlg->m_nLocationID = m_location->Value[m_location->CurSel][L_ID];
				}

				// Put it back in the dialog
				m_list->Value[nNewRow][O_DlgPtr] = (long)pDlg;

				// Create the dialog
				pDlg->Create(IDD_PRODUCT_ITEMS, this);
			}
			//TES 10/24/2008 - PLID 31810 - We want to set the m_bCloseAfterScan member to the appropriate value
			// even if the dialog exists already, so I moved this out of the if(NULL == pDlg) branch.
			// (c.haag 2008-02-15 11:08) - If the conversion is 1, then the user should be able to do scan-product-scan-serial-scan-product-scan-serial
			// but if the conversion rate is not 1, then the user has to scan multiple times; so it should not try to auto-close.
			if (1 == pDlg->m_nConversion) {
				pDlg->m_bCloseAfterScan = true;
			} else {
				pDlg->m_bCloseAfterScan = false;
			}

			// Show the window
			ShowSerializedItemWindow(nNewRow);

		} // if (bHasSerial || bHasExp) {

	} // if (m_bSaveAllAsReceived) {
}

void CInvEditOrderDlg::OnDblClickCellList(long nRowIndex, short nColIndex) 
{
	try {
		// (c.haag 2007-12-06 08:10) - PLID 28286 - If this is a situation where serialized
		// items arrived but no order was placed beforehand, the act of double-clicking on
		// a row should open the serialized item list
		if (nRowIndex < 0 || nColIndex < 0) {
			return;
		}
		if (m_bSaveAllAsReceived) {
			IRowSettingsPtr pRow = m_list->GetRow(nRowIndex);
			if (NULL != pRow && (VarBool(m_list->Value[nRowIndex][O_HasSerialNum], FALSE) || VarBool(m_list->Value[nRowIndex][O_HasExpDate], FALSE)) ) {
				ShowSerializedItemWindow(nRowIndex);
			}
		}
	}
	NxCatchAll("Error in CInvEditOrderDlg::OnDblClickCellList");
}

void CInvEditOrderDlg::EnsureInArray(CStringArray& astr, const CString& str)
{
	// (c.haag 2008-02-15 11:23) - PLID 28286 - Ensures a string element is in a string array
	const long nExisting = astr.GetSize();
	BOOL bFound = FALSE;
	for (long n=0; n < nExisting && !bFound; n++) {
		if (astr[n] == str) {
			bFound = TRUE;
		}
	}
	if (!bFound) {
		astr.Add(str);
	}
}

LRESULT CInvEditOrderDlg::OnPostEditProductItemDlg(WPARAM wParam, LPARAM lParam)
{
	try {
		// (c.haag 2007-12-06 09:12) - PLID 28286 - This message is sent when the
		// product item dialog is dismissed
		if (NULL == m_list) {
			ThrowNxException("Called CInvEditOrderDlg::OnPostEditProductItemDlg when the dialog has no list!");
		}

		CProductItemsDlg* pDlg = (CProductItemsDlg*)VarLong(m_list->Value[m_nVisibleProductItemDlgRow][O_DlgPtr], NULL);
		long nProductID = VarLong(m_list->Value[m_nVisibleProductItemDlgRow][O_ProductID]);
		BOOL bSerialNumIsLotNum = VarBool(m_list->Value[m_nVisibleProductItemDlgRow][O_SerialNumIsLotNum]);
		BOOL bIsRowValid = TRUE;

		//
		// Now check for duplicate serial numbers
		//
		CStringArray astrDuplicates;
		if (pDlg->m_bUseSerial) {
			// (c.haag 2008-02-15 10:10) - Do validation on all the serial numbers, don't assume
			// there's one and only one at all times.
			const long nSerialNums = pDlg->GetSelectedProductItemCount();
			long nSerial;
			if (0 == nSerialNums) {

				// If we get here, there are no serial numbers, so do not let the user add a new row.
				RemoveListRow(m_nVisibleProductItemDlgRow);
				bIsRowValid = FALSE;

			} else if(!bSerialNumIsLotNum) {//TES 7/3/2008 - PLID 24726 - Don't check duplicates if it's a Lot Number.
				for (nSerial=0; nSerial < nSerialNums && bIsRowValid; nSerial++) {
					// Do for all serial numbers in the new row

					_variant_t vSerialNum = pDlg->GetSelectedProductItemSerialNum(nSerial);
					if (!VarString(vSerialNum, "").IsEmpty()) {
						const long nItems = m_list->GetRowCount();
						for (long i=0; i < nItems; i++) {
							// Do for all items in the list

							if (i != m_nVisibleProductItemDlgRow) {
								CProductItemsDlg* pRowDlg = (CProductItemsDlg*)VarLong(m_list->Value[i][O_DlgPtr], NULL);
								if (NULL != pRowDlg && VarLong(m_list->Value[i][O_ProductID]) == nProductID) {
									const long nRowSerialNums = pRowDlg->GetSelectedProductItemCount();
									for (long nRowSerial=0; nRowSerial < nRowSerialNums; nRowSerial++) {
										// Do for all serial numbers for the item "i"

										_variant_t vRowSerialNum = pRowDlg->GetSelectedProductItemSerialNum(nRowSerial);
										if (VarString(vRowSerialNum) == VarString(vSerialNum)) {

											// If we get here, the serials match. Ensure the serial is in the duplicate array
											EnsureInArray(astrDuplicates, VarString(vSerialNum)); 

										} else {
											// The selected serial number and the number for this row are different
										}
									}
								}							
							}
						}

					} // if (!VarString(vSerialNum, "").IsEmpty()) {
					else {
						// (c.haag 2008-02-15 11:00) - The serial was empty or invalid; just skip it
					}

				} // for (long nSerial=0; nSerial < nSerialNums; nSerial++) {

				// Check for duplicates in data
				if (bIsRowValid) {
					CString strFilter;
					for (nSerial=0; nSerial < nSerialNums; nSerial++) {
						_variant_t vSerialNum = pDlg->GetSelectedProductItemSerialNum(nSerial);
						if (!VarString(vSerialNum, "").IsEmpty()) {
							strFilter += FormatString("'%s',", VarString(vSerialNum, ""));
						}
					}
					strFilter.TrimRight(",");		
					_RecordsetPtr prs = CreateRecordset("SELECT SerialNum FROM ProductItemsT WHERE ProductID = %d AND SerialNum IN (%s) AND Deleted = 0 GROUP BY SerialNum", VarLong(m_list->Value[m_nVisibleProductItemDlgRow][O_ProductID]), strFilter);
					while (!prs->eof) {
						EnsureInArray(astrDuplicates, AdoFldString(prs, "SerialNum"));
						prs->MoveNext();
					}

					if (astrDuplicates.GetSize() > 0) {
						// Warn the user about the duplicates
						CString strMsg = "The following serial numbers already exist in this order or in your existing data:\n\n";
						const long nDuplicates = min(astrDuplicates.GetSize(), 20);
						for (long nDup=0; nDup < nDuplicates; nDup++) {
							strMsg += astrDuplicates[nDup] + "\n";
						}
						if (astrDuplicates.GetSize() > 20) {
							strMsg += "<more>\n";
						}
						strMsg += "\n\nAre you sure you wish to commit your changes?";
										
						if (IDNO == MessageBox(strMsg, "Serialized Items", MB_YESNO | MB_ICONWARNING)) {
							RemoveListRow(m_nVisibleProductItemDlgRow);
							bIsRowValid = FALSE;
						}
					}
				}
			} // else if(!pDlg->m_bSerialNumIsLotNum) {

		} // if (pDlg->m_bUseSerial) {

		// If the row is still valid, make the quantity column a hyperlink to open
		// the product item dialog
		if (bIsRowValid) {
			IRowSettingsPtr pRow = m_list->GetRow(m_nVisibleProductItemDlgRow);
			pRow->CellLinkStyle[O_Quantity] = dlLinkStyleTrue;
			// Ensure that if the dialog is ever opened again, that it will not auto-close
			// when someone scans something.
			pDlg->m_bCloseAfterScan = false;

			// (a.walling 2008-03-20 11:29) - PLID 29333 - If they cancelled, ensure the quantity is up to date
			if (lParam == IDCANCEL && m_bSaveAllAsReceived) {
				long nQuantity, nConversion;
				pDlg->GetQuantity(nQuantity, nConversion);

				long nRealQuantity = nQuantity / nConversion;

				pRow->Value[O_Quantity] = nRealQuantity;
			}
		}
	}
	NxCatchAll("Error in CInvEditOrderDlg::OnPostEditProductItemDlg");

	// Reset the visible product item dialog row so OnBarcodeScan will handle
	// scans again
	m_nVisibleProductItemDlgRow = -1;

	return 0;
}

void CInvEditOrderDlg::OnLButtonDownList(long nRow, short nCol, long x, long y, long nFlags) 
{
	try {
		// (c.haag 2007-12-06 08:10) - PLID 28286 - If this is a situation where serialized
		// items arrived but no order was placed beforehand, the act of clicking on the
		// quantity column should open the product item dialog rather than letting the user
		// edit the quantity
		if (nRow >= 0 && m_bSaveAllAsReceived && O_Quantity == nCol) {
			IRowSettingsPtr pRow = m_list->GetRow(nRow);
			if (NULL != pRow && (VarBool(m_list->Value[nRow][O_HasSerialNum], FALSE) || VarBool(m_list->Value[nRow][O_HasExpDate], FALSE)) ) {
				ShowSerializedItemWindow(nRow);
			}
		}
	}
	NxCatchAll("Error in CInvEditOrderDlg::OnLButtonDownList");	
}

void CInvEditOrderDlg::OnEditingStartingList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		// (c.haag 2007-12-06 08:10) - PLID 28286 - If this is a situation where serialized
		// items arrived but no order was placed beforehand, do not let the user edit the quantity
		if (nRow >= 0 && m_bSaveAllAsReceived && O_Quantity == nCol) {
			IRowSettingsPtr pRow = m_list->GetRow(nRow);
			if (NULL != pRow && (VarBool(m_list->Value[nRow][O_HasSerialNum], FALSE) || VarBool(m_list->Value[nRow][O_HasExpDate], FALSE)) ) {
				*pbContinue = FALSE;
				return;
			}
		}
		// (c.haag 2008-05-21 16:04) - PLID 30082 - Do not let the user change the quantity for an
		// item that exists in data as having been received. While doing so would only harm serialized
		// items in that ProductItemsT is not changed; it's better for support purposes, and as a general 
		// rule, to not allow for it across the board. A user should never do this in the first place.
		// They can mark it as unreceived, save it, and make necessary corrections at that time.
		if (nRow >= 0 && O_Quantity == nCol && VarLong(m_list->Value[nRow][O_ID]) > 0) {
			_RecordsetPtr prs = CreateParamRecordset("SELECT ID FROM OrderDetailsT WHERE ID = {INT} AND DateReceived IS NOT NULL",
				VarLong(m_list->Value[nRow][O_ID]));

			if (!prs->eof) {
				// The order detail is "received" in data. Don't allow the user to change the quantity.
				AfxMessageBox("You may not change the quantity of an order item that is marked as received in your data.\n"
					"\n"
					"If the received information for this order is incorrect, you must delete this order and create a new one to reflect the correct values.", MB_ICONERROR);
				*pbContinue = FALSE;

			} else {
				// The order detail is not "received" in data; so don't stop the user from editing the quantity
			}
		}
	}
	NxCatchAll("Error in CInvEditOrderDlg::OnEditingStartingList");	
}

// (j.jones 2008-02-07 10:02) - PLID 28851 - split the Auto-order function into two,
// this one is for purchased inventory
void CInvEditOrderDlg::OnAutoPurchInv() 
{
	try {

		IRowSettingsPtr pRow;
		int count = m_item->GetRowCount();
		double dblSurplus;

		// (j.jones 2008-02-14 11:42) - PLID 28864 - added option for when to order products, when the
		// actual (0) hits the reorder point, or when the available (1) hits it
		long nOrderByOnHandAmount = GetRemotePropertyInt("InvItem_OrderByOnHandAmount", 0, 0, "<None>", true);
		BOOL bOrderByActual = (nOrderByOnHandAmount == 0);

		//compare products' purchased inventory amount against the purchased inventory reorder point,
		//then tell OnAddItem to order it as purchased inventory, using the purchased inventory reorder quantity

		//update total is slow, so only do it 1 time
		m_bTotalUpdateable = false;
		for (int i = 0; i < count; i++) {
			
			pRow = m_item->Row[i];

			// (j.jones 2006-10-12 14:59) - PLID 23006 - If null, it means
			// this item only tracks orders, so never auto-order
			if(pRow->Value[I_ActualPurchased].vt == VT_NULL) {
				continue;
			}

			long nProductID = VarLong(m_item->Value[i][I_ID]);

			long nQuantity = VarLong(m_item->Value[i][I_ReorderQuantityPurchased],1);
			long nOrdered = VarLong(m_item->Value[i][I_OrderedPurchased],0);
			double dblActual = VarDouble(m_item->Value[i][I_ActualPurchased],0.0);
			double dblAvail = VarDouble(m_item->Value[i][I_AvailPurchased],0.0);
			double dblReorderPoint = VarDouble(m_item->Value[i][I_ReorderPointPurchased],0.0);

			// (j.jones 2008-02-14 11:57) - PLID 28864 - use the preference to calculate where
			// we look at actual or available
			double dblValueToCheck = 0.0;
			if(bOrderByActual) {
				dblValueToCheck = dblActual;
			}
			else {
				dblValueToCheck = dblAvail;
			}

			dblSurplus = dblValueToCheck + nOrdered - dblReorderPoint;

			// (j.jones 2008-02-19 12:23) - PLID 28981 - for purchased inventory,
			// we order when under the reorder point or equal to the reorder point,
			// so we continue with the existing logic of ordering when dblSurplus is <= 0
			if (dblSurplus <= 0 && dblReorderPoint != 0) {

				//we need to order this item, but is it already in the list?
				//if so, skip it

				BOOL bFound = FALSE;
				for(int j=0; j<m_list->GetRowCount() && !bFound; j++) {

					long nListProductID = VarLong(m_list->Value[j][O_ProductID]);
					if(nListProductID == nProductID) {
						//it exists in the list, but is it NOT consignment?
						if(VarLong(m_list->Value[j][O_ForStatus]) != InvUtils::odsConsignment) {
							//it is not consignment, so we will skip this
							bFound = TRUE;
						}
					}
				}

				if(!bFound) {
					//it's not in the list, so order it, and
					//force AddItem to add as purchased inventory
					AddItem(i, airPurchasedInv);
				}
			}
		}
		m_bTotalUpdateable = true;
		UpdateTotal();

	}NxCatchAll("Error in OnAutoPurchInv()");
}

// (j.jones 2008-02-07 10:02) - PLID 28851 - split the Auto-order function into two,
// this one is for consignment
void CInvEditOrderDlg::OnAutoConsign() 
{
	try {

		IRowSettingsPtr pRow;
		int count = m_item->GetRowCount();
		double dblSurplus = 0.0;

		// (j.jones 2008-02-14 11:42) - PLID 28864 - added option for when to order products, when the
		// actual (0) hits the reorder point, or when the available (1) hits it
		long nOrderByOnHandAmount = GetRemotePropertyInt("InvItem_OrderByOnHandAmount", 0, 0, "<None>", true);
		BOOL bOrderByActual = (nOrderByOnHandAmount == 0);

		//compare products' consignment amount against the consignment reorder point,
		//then tell OnAddItem to order it as consignment, using the consignment reorder quantity
		
		//update total is slow, so only do it 1 time
		m_bTotalUpdateable = false;
		for (int i = 0; i < count; i++) {
			
			pRow = m_item->Row[i];

			// (j.jones 2006-10-12 14:59) - PLID 23006 - If null, it means
			// this item only tracks orders, so never auto-order
			if(pRow->Value[I_ActualConsign].vt == VT_NULL) {
				continue;
			}

			long nProductID = VarLong(m_item->Value[i][I_ID]);

			// (j.jones 2008-02-15 17:06) - PLID 28852 - we do not use a reorder qty. for consignment
			long nQuantity = 1; //VarLong(m_item->Value[i][I_ReorderQuantityConsign],1);
			long nOrdered = VarLong(m_item->Value[i][I_OrderedConsign],0);
			double dblActual = VarDouble(m_item->Value[i][I_ActualConsign],0.0);
			double dblAvail = VarDouble(m_item->Value[i][I_AvailConsign],0.0);
			double dblReorderPoint = VarDouble(m_item->Value[i][I_ReorderPointConsign],0.0);

			// (j.jones 2008-02-14 11:45) - PLID 28864 - use the preference to calculate where
			// we look at actual or available
			double dblValueToCheck = 0.0;
			if(bOrderByActual) {
				dblValueToCheck = dblActual;
			}
			else {
				dblValueToCheck = dblAvail;
			}

			dblSurplus = dblValueToCheck + nOrdered - dblReorderPoint;

			// (j.jones 2008-02-19 12:23) - PLID 28981 - for consignment,
			// we only order when under the reorder point, not when we're at
			// that level, so only order when dblSurplus is < 0, not equal to 0
			if (dblSurplus < 0 && dblReorderPoint != 0) {

				//we need to order this item, but is it already in the list?
				//if so, skip it

				BOOL bFound = FALSE;
				for(int j=0; j<m_list->GetRowCount() && !bFound; j++) {

					long nListProductID = VarLong(m_list->Value[j][O_ProductID]);
					if(nListProductID == nProductID) {
						//it exists in the list, but is it consignment?
						if(VarLong(m_list->Value[j][O_ForStatus]) == InvUtils::odsConsignment) {
							//it is, so we will skip this
							bFound = TRUE;
						}
					}
				}

				if(!bFound) {
					//it's not in the list, so order it, and
					//force AddItem to add as consignment
					AddItem(i, airConsignment);
				}
			}
		}
		m_bTotalUpdateable = true;
		UpdateTotal();

	}NxCatchAll("Error in OnAutoConsign()");
}

// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
void CInvEditOrderDlg::OnEditShipMethod() 
{
	try {
		_variant_t varValue;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pShipMethodList->CurSel;
		if (pRow) {
			varValue = pRow->GetValue(0);
		}

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 18, m_pShipMethodList, "Edit Shipping Methods").DoModal();

		pRow = m_pShipMethodList->GetNewRow();
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _variant_t("<No Shipping Method>"));
		m_pShipMethodList->AddRowSorted(pRow, NULL);

		pRow = m_pShipMethodList->SetSelByColumn(0, varValue);
		
		if (pRow == NULL) {
			m_pShipMethodList->SetSelByColumn(0, (long)-1);
		}

		GetDlgItem(IDC_SHIPPING_METHOD_LIST)->SetFocus();
	}NxCatchAll("Error in OnEditShipMethod");
	
}

// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
void CInvEditOrderDlg::OnEditCcType() 
{
	try {
		long nCreditCardID; 
		CString strCCName;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCCTypeList->CurSel;
		if(pRow){
			nCreditCardID = VarLong(pRow->GetValue(0));
		}
		else{
			nCreditCardID = -2;

			if (m_pCCTypeList->IsComboBoxTextInUse) {
				strCCName = AsString(m_pCCTypeList->ComboBoxText);
			}
		}


		
		CEditCreditCardsDlg dlg(this);
		if(IDOK == dlg.DoModal()){
			// - requery the cc combo if they OK'd to save (even if no changes were actually made).
			m_pCCTypeList->Requery();

			pRow = m_pCCTypeList->GetNewRow();
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, _variant_t("<None>"));
			m_pCCTypeList->AddRowSorted(pRow, NULL);
		}

		if (nCreditCardID != -2) {
			pRow = m_pCCTypeList->SetSelByColumn(0, nCreditCardID);
			if (pRow == NULL) {
				//could be inactive
				_RecordsetPtr rsCCTypeName = CreateRecordset("SELECT CardName FROM CreditCardNamesT WHERE ID = %li", nCreditCardID);
				if (!rsCCTypeName->eof) {
					m_pCCTypeList->PutComboBoxText(_bstr_t(AdoFldString(rsCCTypeName, "CardName", "")));
				}
			}
			
		}
		else {
			//they don't have one selected, so put that as the selection
			if (!strCCName.IsEmpty()) {
				m_pCCTypeList->ComboBoxText = (LPCTSTR)strCCName;
			}
			else {
				m_pCCTypeList->SetSelByColumn(0, (long) -1);
			}
		}
	}NxCatchAll("Error in OnEditCCType");
	
}

// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
void CInvEditOrderDlg::OnEnableShipTo() 
{
	try {
		// (c.haag 2008-05-21 15:51) - PLID 30082 - If there's at least one received item for
		// this order, don't let the user change the "Ship To" checkbox.
		if (m_id > 0) {
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM OrderDetailsT WHERE OrderID = {INT} AND DateReceived IS NOT NULL", m_id);
			if (!prs->eof) {
				// At least one order detail is "received" in data; so prevent the location change
				AfxMessageBox("You may not reconfigure the \"Ship To\" location of an order which has at least one item that is marked as received in your data.\n"
					"\n"
					"If the received information for this order is incorrect, you must delete this order and create a new one to reflect the correct values.", MB_ICONERROR);

				if (IsDlgButtonChecked(IDC_ENABLE_SHIP_TO)) {
					CheckDlgButton(IDC_ENABLE_SHIP_TO, 0);
				} else {
					CheckDlgButton(IDC_ENABLE_SHIP_TO, 1);
				}
				return;
			} 
		}

		if (IsDlgButtonChecked(IDC_ENABLE_SHIP_TO)) {

			//enable the list
			GetDlgItem(IDC_LOCATION)->EnableWindow(TRUE);
		}
		else {
			
			GetDlgItem(IDC_LOCATION)->EnableWindow(FALSE);

			//set the location back to the ship to
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pLocationSoldTo->CurSel;
			if (pRow) {
				m_location->SetSelByColumn(0, VarLong(pRow->GetValue(0), -1));
				OnSelChosenLocation(VarLong(pRow->GetValue(0), -1));
			}
			else {
				//there is an inactive location?
				m_location->PutComboBoxText(m_pLocationSoldTo->GetComboBoxText());
			}
		}
	}NxCatchAll("Error in OnEnableShipTo");
}

// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
void CInvEditOrderDlg::OnUpdateOrderCcExpDate() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CNxDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here

	try {
		CString strExpDate;
		GetDlgItem(IDC_ORDER_CC_EXP_DATE)->GetWindowText(strExpDate);
		if (!strExpDate.IsEmpty()) {
			FormatItemText(GetDlgItem(IDC_ORDER_CC_EXP_DATE), strExpDate, "##/##nn");
		}
	}NxCatchAll("Error in OnUpdateOrderCCExpDate");
	
}

// (j.gruber 2008-02-27 10:07) - PLID 29103 - add CC fields to the order dlg
void CInvEditOrderDlg::OnKillfocusOrderCcExpDate() 
{
	try {
		COleDateTime dt1,dt2;
		CString strMonth, strYear, strExpDate;
		int iMonth, iYear;

		GetDlgItem(IDC_ORDER_CC_EXP_DATE)->GetWindowText(strExpDate);
		if(strExpDate != "" && strExpDate != "##/##"){ //TS 6/6/2001: Don't go through this if they haven't entered an exp. date.
			//set the date to test, to be the first day of the next month
			int iIndex = strExpDate.Find("/",0);
			strMonth = strExpDate.Left(iIndex);
			strYear = strExpDate.Right(strExpDate.GetLength()-(iIndex+1));
			if(strYear.GetLength() == 2) {
				strYear = "20" + strYear;
			}

			iMonth = atoi(strMonth);
			iYear = atoi(strYear);

			if(iMonth < 1 || iMonth > 12 || strExpDate.Find("#")!=-1) {
				MsgBox("The date you entered was invalid.");
				GetDlgItem(IDC_ORDER_CC_EXP_DATE)->SetWindowText("##/##");
				return;
			}
			else if(iMonth==12) {
				//the method we use to store dates acts funky with December, so
				//we cannot just increase the month by 1. Just make it 1/1 of the
				//following year.
				if(dt2.SetDate(iYear+1,1,1)==0) {
					dt1 = COleDateTime::GetCurrentTime();
					if(dt1 >= dt2) {
						MsgBox("This credit card has expired. The date cannot be accepted.");
						GetDlgItem(IDC_ORDER_CC_EXP_DATE)->SetWindowText("##/##");
						return;
					}
				}
			}
			else {
				//this method works well for all other months. Set the date to be
				//the first day of the NEXT month.
				COleDateTimeSpan dtSpan;
				dtSpan.SetDateTimeSpan(1,0,0,0);
				if(dt2.SetDate(iYear,iMonth+1,1)==0) {
					dt1 = COleDateTime::GetCurrentTime();
					if(dt1 >= dt2) {
						MsgBox("This credit card has expired. The date cannot be accepted.");
						GetDlgItem(IDC_ORDER_CC_EXP_DATE)->SetWindowText("##/##");
						return;
					}
				}
			}
		}
		else {
			//set it to blank since its either blank or # signs
			SetDlgItemText(IDC_ORDER_CC_EXP_DATE, "");
		}
	}NxCatchAll("Error in OnKillFocusOrderCCExpDate");
	
}

// (j.gruber 2008-02-27 10:07) - PLID 28955 - added fields to the order dlg
void CInvEditOrderDlg::OnSelChosenSoldTo(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			//check to see if the "if different" button is checked
			if (IsDlgButtonChecked(IDC_ENABLE_SHIP_TO)) {
				//they are different, so don't do anything
			}
			else {
				//they are the same, so change the ship to also
				m_location->SetSelByColumn(0, VarLong(pRow->GetValue(0)));
				OnSelChosenLocation(VarLong(pRow->GetValue(0)));
			}
		}	
	}NxCatchAll("Error in OnSelChosenSoldTo");
}


// (j.gruber 2008-02-29 15:57) - PLID 29103 - added CC fields
void CInvEditOrderDlg::OnOrderCcOnFile() 
{
	try {
		//TES 5/28/2008 - PLID 30165 - We want to remember this choice, for this supplier.
		long nSupplierID = VarLong(m_supplier->Value[m_supplier->CurSel][S_ID]);

		if (IsDlgButtonChecked(IDC_ORDER_CC_ON_FILE)) {
			
			CheckDlgButton(IDC_ORDER_CC_ON_FILE, 1);
			GetDlgItem(IDC_ORDER_CC_NUMBER)->EnableWindow(FALSE);
			SetDlgItemText(IDC_ORDER_CC_NUMBER, "ON FILE");
			m_strCreditCardNumber = "ON FILE";
			
			//TES 5/28/2008 - PLID 30165 - Remember this choice, for this supplier.
			SetRemotePropertyInt("Supplier_CcOnFile", 1, nSupplierID, "<None>");
		}
		else {
			CheckDlgButton(IDC_ORDER_CC_ON_FILE, 0);
						
			GetDlgItem(IDC_ORDER_CC_NUMBER)->EnableWindow(TRUE);
			
			//TES 5/28/2008 - PLID 30165 - Remember this choice, for this supplier.
			SetRemotePropertyInt("Supplier_CcOnFile", 0, nSupplierID, "<None>");
			
		}
	}NxCatchAll("Error in OnOrderCCOnFile");
	
}

// (j.gruber 2008-02-29 15:57) - PLID 29103 - added CC fields
void CInvEditOrderDlg::OnKillfocusOrderCcNumber() 
{
	try {
		CString strCCNumber;
		GetDlgItemText(IDC_ORDER_CC_NUMBER, strCCNumber);
		CString strMasked;

		for (int i = 0; i < m_strCreditCardNumber.GetLength() - 4; i++) {
			strMasked += "X";
		}

		strMasked += m_strCreditCardNumber.Right(4);


		if (strCCNumber != strMasked && strCCNumber != m_strCreditCardNumber) {
		
			if (strCCNumber.Find("X") != -1) {
				//they didn't change the whole number
				MsgBox("When changing the credit card number, you must re-enter the entire number.\n"
					  "The card number will be set back to the previous value.");
	
				SetDlgItemText(IDC_ORDER_CC_NUMBER, strMasked);
			}
			else {
				m_strCreditCardNumber = strCCNumber;
			}
		}
	}NxCatchAll("Error in OnKillFocusOrderCCNumber");		
}

// (j.jones 2008-03-18 17:07) - PLID 29309 - display the linked appt. information
void CInvEditOrderDlg::DisplayLinkedApptData()
{
	try {

		// (j.jones 2008-03-18 16:36) - PLID 29309 - the appt. linking is adv. inv. only
		if(m_bHasAdvInventory) {
			m_strApptLinkText = "< No Linked Appointment >";
			if(m_nApptID != -1) {
				_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name, AppointmentsT.Date, AppointmentsT.StartTime "
					"FROM AppointmentsT "
					"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
					"WHERE AppointmentsT.ID = {INT}", m_nApptID);
				if(!rs->eof) {
					CString strPatientName = AdoFldString(rs, "Name","");
					strPatientName.TrimRight();
					COleDateTime dtApptDate = AdoFldDateTime(rs, "Date");
					COleDateTime dtApptTime = AdoFldDateTime(rs, "StartTime");
					m_strApptLinkText.Format("Linked to patient '%s', Appointment Date: %s %s", strPatientName, FormatDateTimeForInterface(dtApptDate, NULL, dtoDate), FormatDateTimeForInterface(dtApptTime, DTF_STRIP_SECONDS, dtoTime));
				}
				rs->Close();
			}
			//no need to update the hyperlink label now, OnPaint() will do it,
			//just invalidate it so we know for sure it will repaint
			InvalidateRect(m_rcApptLinkLabel);
		}

	}NxCatchAll("Error in CInvEditOrderDlg::DisplayLinkedApptData");
}

// (j.jones 2008-03-19 09:09) - PLID 29309 - handles when the appt. hyperlink is clicked
void CInvEditOrderDlg::OnClickApptLink()
{
	try {

		if(!m_bHasAdvInventory) {
			//no license? silently return
			return;
		}

		//pop up a list of all appointments that that are not cancelled,
		//not marked out or no show, and are on today or the next two weeks
		
		// (j.jones 2008-09-24 16:32) - PLID 31493 - we now allow selecting appts.
		// that are linked to other orders as well, such that one appointment can
		// now be linked to more than one order

		CString strWhere;
		/*
		//this where clause will still return the results we need even if either of our parameters are -1
		strWhere.Format("PatientID <> -25 "
			"AND Date >= Convert(datetime,(Convert(nvarchar,GetDate(),1))) AND AppointmentsT.Status <> 4 AND ShowState NOT IN (2,3) "
			"AND Date < DATEADD(day, 22, Convert(datetime,(Convert(nvarchar,GetDate(),1)))) "
			"AND (AppointmentsT.ID = %li OR AppointmentsT.ID NOT IN (SELECT AppointmentID FROM OrderAppointmentsT WHERE OrderID <> %li))",
			m_nApptID);
		*/

		strWhere.Format("PatientID <> -25 "
			"AND Date >= Convert(datetime,(Convert(nvarchar,GetDate(),1))) AND AppointmentsT.Status <> 4 AND ShowState NOT IN (2,3) "
			"AND Date < DATEADD(day, 22, Convert(datetime,(Convert(nvarchar,GetDate(),1)))) ");

		long nNewApptID = -1;

		if(ReturnsRecords("SELECT ID FROM AppointmentsT WHERE %s", strWhere)) {
			CSelectApptDlg dlg(this);
			dlg.m_strWhere = strWhere;
			dlg.m_bAllowMultiSelect = FALSE;
			dlg.m_bShowPatientName = TRUE;
			dlg.m_bShowNoneSelectedRow = TRUE;
			dlg.m_strLabel = "Please select an appointment to associate with this order.";
			int nReturn = dlg.DoModal();
			if(nReturn == IDOK) {
				if(dlg.m_arSelectedIds.GetSize() > 0) {
					nNewApptID = dlg.m_arSelectedIds.GetAt(0);
				}
				else {
					//they selected nothing, so return
					return;
				}
			}
			else {
				return;
			}
		}
		else {
			AfxMessageBox("There are no available appointments within the next three weeks that are not marked Out, No Show, Cancelled, or linked with another inventory order.\n\n"
				"If you wish to create an order for an appointment that is more than three weeks away, you must do so by right-clicking the appointment in the scheduler.");
			return;
		}

		// (j.jones 2008-09-25 10:57) - PLID 31493 - if they selected a new appt.,
		// warn the user if the appt. already has an order that is not this order
		// (m_id of -1 is fine)
		if(m_nApptID != nNewApptID && nNewApptID != -1) {
			_RecordsetPtr rsOrders = CreateParamRecordset("SELECT ID FROM OrderAppointmentsT "
				"WHERE AppointmentID = {INT} AND OrderID <> {INT}", nNewApptID, m_id);
			if(!rsOrders->eof) {
				CString strWarning = "The selected appointment is already linked to another order. "
					"Are you sure you wish to link the current order to this appointment as well?";
				if(rsOrders->GetRecordCount() > 1) {
					strWarning = "The selected appointment is already linked to multiple orders. "
						"Are you sure you wish to link the current order to this appointment as well?";
				}

				int nRes = MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO);
				if(nRes != IDYES) {
					return;
				}
			}
			rsOrders->Close();
		}

		//now update the appointment ID, which may be -1 now
		m_nApptID = nNewApptID;

		//now update the text
		DisplayLinkedApptData();

	}NxCatchAll("Error in CInvEditOrderDlg::OnClickApptLink");
}

void CInvEditOrderDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	try {

		// (j.jones 2008-03-19 09:11) - PLID 29309 - are we in the hyperlink?
		//TES 7/22/2008 - PLID 30802 - Also check that we're showing the hyperlink.
		if(m_bHasAdvInventory && m_bShowApptLinkLabel && m_rcApptLinkLabel.PtInRect(point)) {
			OnClickApptLink();
		}
	
	}NxCatchAll("Error in CInvEditOrderDlg::OnLButtonDown");

	CNxDialog::OnLButtonDown(nFlags, point);
}

void CInvEditOrderDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	try {

		// (j.jones 2008-03-19 09:11) - PLID 29309 - are we in the hyperlink?
		//TES 7/22/2008 - PLID 30802 - Also check that we're showing the hyperlink.
		if(m_bHasAdvInventory && m_rcApptLinkLabel.PtInRect(point)) {
			OnClickApptLink();
		}
	
	}NxCatchAll("Error in CInvEditOrderDlg::OnLButtonDblClk");

	CNxDialog::OnLButtonDblClk(nFlags, point);
}

void CInvEditOrderDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// (j.jones 2008-03-19 09:15) - PLID 29309 - redraw the hyperlink
	DrawApptLinkLabel(&dc);
}

// (j.jones 2008-03-19 09:17) - PLID 29309 - update the hyperlink text
void CInvEditOrderDlg::DrawApptLinkLabel(CDC *pdc)
{
	try {

		//do not draw if we don't have the license
		if(!m_bHasAdvInventory) {
			//silently return
			return;
		}

		//TES 7/22/2008 - PLID 30802 - Also don't draw if we've chosen to hide this label.
		if(!m_bShowApptLinkLabel) {
			return;
		}

		// (j.jones 2008-05-01 15:19) - PLID 29874 - Set background color to transparent
		DrawTextOnDialog(this, pdc, m_rcApptLinkLabel, m_strApptLinkText, dtsHyperlink, false, DT_LEFT, true, false, 0);

	}NxCatchAll("Error in CInvEditOrderDlg::DrawApptLinkLabel");
}

BOOL CInvEditOrderDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {

		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		// (j.jones 2008-03-19 09:14) - PLID 29309 - are we in the hyperlink?
		//TES 7/22/2008 - PLID 30802 - Also check that we're showing the hyperlink.
		if (m_bHasAdvInventory && m_bShowApptLinkLabel && m_rcApptLinkLabel.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

	}NxCatchAll("Error in CInvEditOrderDlg::OnSetCursor");
	
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CInvEditOrderDlg::OnSelChangingLocation(long FAR* nNewSel) 
{
	try {
		// (c.haag 2008-05-21 15:51) - PLID 30082 - Don't allow the user to change the location if at least
		// one order detail has been saved and received. While doing so would only harm serialized
		// items in that ProductItemsT is not changed; it's better for support purposes, and as a general rule,
		// to not allow for it across the board.
		const long nCurSel = m_location->CurSel;
		if (*nNewSel != nCurSel && m_id > 0) {
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM OrderDetailsT WHERE OrderID = {INT} AND DateReceived IS NOT NULL", m_id);
			if (!prs->eof) {
				// At least one order detail is "received" in data; so prevent the location change
				AfxMessageBox("You may not change the \"Ship To\" location of an order which has at least one item that is marked as received in your data.\n"
					"\n"
					"If the received information for this order is incorrect, you must delete this order and create a new one to reflect the correct values.", MB_ICONERROR);
				*nNewSel = nCurSel;

			} else {
				// No order details are "received" in data; so don't stop the user from changing the location
			}
		}

	}
	NxCatchAll("Error in CInvEditOrderDlg::OnSelChangingLocation");
}

void CInvEditOrderDlg::OnSelChangingSoldTo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
		// (c.haag 2008-05-21 15:51) - PLID 30082 - Don't allow the user to change the location if at least
		// one order detail has been saved and received. While doing so would only harm serialized
		// items in that ProductItemsT is not changed; it's better for support purposes, and as a general rule,
		// to not allow for it across the board.
		if (NULL == lppNewSel) {
			return;
		}
		else if (NULL == *lppNewSel) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
		else if (m_id > 0 && (*lppNewSel != lpOldSel) && !IsDlgButtonChecked(IDC_ENABLE_SHIP_TO)) {
			_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM OrderDetailsT WHERE OrderID = {INT} AND DateReceived IS NOT NULL", m_id);
			if (!prs->eof) {
				// At least one order detail is "received" in data; so prevent the location change
				AfxMessageBox("You may not change the \"Sold To\" location of an order which is bound to the \"Ship To\" location, and has at least one item that is marked as received in your data.\n"
					"\n"
					"If the received information for this order is incorrect, you must delete this order and create a new one to reflect the correct values.", MB_ICONERROR);

				SafeSetCOMPointer(lppNewSel, lpOldSel);
			} else {
				// No order details are "received" in data; so don't stop the user from changing the location
			}
		}
	}
	NxCatchAll("Error in CInvEditOrderDlg::OnSelChangingSoldTo");
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CInvEditOrderDlg::OnOK()
{
	//Eat the message
}

// (j.jones 2008-06-19 10:06) - PLID 10394 - added ability to apply discounts to an entire order
void CInvEditOrderDlg::OnBtnApplyDiscounts() 
{
	try {

		//pop up a menu of percent off or discount amount
		enum {
			miPercent = -1,
			miDiscount = -2,
		};

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, miPercent, "Apply a &Percent Off");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, miDiscount, "Apply a &Discount Amount");
		
		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_BTN_APPLY_DISCOUNTS);
		int nResult = 0;
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
		}

		switch(nResult) {
			case miPercent:
				
				PromptApplyPercentOffToAll();
				break;

			case miDiscount:
				
				PromptApplyDiscountAmountToAll();
				break;
		}

	}NxCatchAll("Error in CInvEditOrderDlg::OnBtnApplyDiscounts");	
}

// (j.jones 2008-06-19 10:06) - PLID 10394 - added abilities to apply a percent off
// or discount amount to all items
void CInvEditOrderDlg::PromptApplyPercentOffToAll()
{
	try {

		//prompt for a percent off
		CString strPercent;
		int ipRet = InputBox(this, "Enter a percent off:", strPercent, "", false, false, NULL, FALSE);
		while(ipRet != IDOK || atof(strPercent) <= 0.0 || atof(strPercent) > 100.0) {
			if(ipRet == IDCANCEL) {
				return;
			}
			else if(atof(strPercent) <= 0.0) {
				AfxMessageBox("Please enter a percent off greater than zero.");
			}
			else if(atof(strPercent) > 100.0) {
				AfxMessageBox("Please enter a percent off that is not greater than 100.");
			}
			ipRet = InputBox(this, "Enter a percent off:", strPercent, "", false, false, NULL, FALSE);
		}

		double dblPercentOff = atof(strPercent);

		//see if any item has a percent off already, and ask if they want to
		//overwrite it, skip it, or cancel applying a percentage

		BOOL bHasPercentOff = FALSE;

		int i = 0;
		for (i = 0; i<m_list->GetRowCount() && !bHasPercentOff; i++)	{

			double dblExistingPercentOff = VarDouble(m_list->Value[i][O_PercentOff]);
			if(dblExistingPercentOff != 0.0) {
				bHasPercentOff = TRUE;
			}
		}

		BOOL bOverwrite = FALSE;

		if(bHasPercentOff) {
			
			//warn about this
			int ioRet = MessageBox("At least one product in the order already has a percent off entered. Would you like to overwrite the existing percentage off?\n\n"
				"Click 'YES' if you wish to overwrite with the new percent off.\n"
				"Click 'NO' if you wish to only add the percent off to products that do not have one.\n"
				"Click 'CANCEL' if you wish to not add a percent off to this order.", "Practice", MB_ICONQUESTION|MB_YESNOCANCEL);

			if(ioRet == IDCANCEL) {
				return;
			}
			else if(ioRet == IDYES) {
				bOverwrite = TRUE;
			}
		}

		//now update each row
		for (i = 0; i<m_list->GetRowCount(); i++)	{

			if(!bOverwrite) {
				//if they don't wish to overwrite, skip this row if it has a percent off
				double dblExistingPercentOff = VarDouble(m_list->Value[i][O_PercentOff]);

				if(dblExistingPercentOff != 0.0) {
					continue;
				}
			}

			m_list->PutValue(i, O_PercentOff, (double)dblPercentOff);
		}

		UpdateTotal();

	}NxCatchAll("Error in CInvEditOrderDlg::PromptApplyPercentOffToAll");
}

// (j.jones 2008-06-19 10:06) - PLID 10394 - added abilities to apply a percent off
// or discount amount to all items
void CInvEditOrderDlg::PromptApplyDiscountAmountToAll()
{
	try {

		//prompt for a discount amount
		CString strAmount;
		int idRet = InputBox(this, "Enter a discount amount:", strAmount, "", false, false, NULL, FALSE);
		COleCurrency cyDiscountAmount;
		BOOL bContinue = TRUE;
		while(bContinue) {

			bContinue = FALSE;

			if(idRet == IDCANCEL) {
				return;
			}

			if(!cyDiscountAmount.ParseCurrency(strAmount)) {
				AfxMessageBox("The discount amount you entered is invalid, please enter a valid amount.");
				bContinue = TRUE;
			}
			else if(cyDiscountAmount <= COleCurrency(0,0)) {
				AfxMessageBox("Please enter a discount amount greater than zero.");
				bContinue = TRUE;
			}

			if(bContinue) {
				idRet = InputBox(this, "Enter a discount amount:", strAmount, "", false, false, NULL, FALSE);
			}
		}

		//loop through and apply the discount until we have none left
		int i = 0;
		for (i = 0; i<m_list->GetRowCount(); i++)	{

			COleCurrency cyExistingDiscount = VarCurrency(m_list->Value[i][O_Discount]);
			COleCurrency cyTotal = VarCurrency(m_list->Value[i][O_Total]);

			if(cyTotal > COleCurrency(0,0)) {
				//apply the discount
				if(cyTotal >= cyDiscountAmount) {
					cyExistingDiscount += cyDiscountAmount;
					cyTotal -= cyDiscountAmount;
					cyDiscountAmount = COleCurrency(0,0);

					m_list->PutValue(i, O_Discount, _variant_t(cyExistingDiscount));
					m_list->PutValue(i, O_Total, _variant_t(cyTotal));
				}
				else {
					cyExistingDiscount += cyTotal;
					cyDiscountAmount -= cyTotal;					
					cyTotal = COleCurrency(0,0);

					m_list->PutValue(i, O_Discount, _variant_t(cyExistingDiscount));
					m_list->PutValue(i, O_Total, _variant_t(cyTotal));
				}
			}
		}

		UpdateTotal();

		//warn if we have anything left over
		if(cyDiscountAmount > COleCurrency(0,0)) {
			CString str;
			str.Format("%s of your discount amount could not be applied to the order, as there was not enough balance left to discount.", FormatCurrencyForInterface(cyDiscountAmount, TRUE, TRUE));
			AfxMessageBox(str);
		}

	}NxCatchAll("Error in CInvEditOrderDlg::PromptApplyDiscountAmountToAll");
}

// (j.jones 2008-07-03 16:41) - PLID 30624 - converted OnSelChangedLocation to OnSelChosenLocation
void CInvEditOrderDlg::OnSelChosenLocation(long nRow) 
{
	try {

		//force a selection
		if(nRow == -1) {
			nRow = m_location->SetSelByColumn(0, GetCurrentLocationID());
			if(nRow == -1) {
				m_location->CurSel = 0;
				nRow = m_location->CurSel;
				if(nRow == -1) {
					//should be impossible
					ThrowNxException("No Ship To locations available!");
					return;
				}
			}
		}

		HandleNewSupplier(-1);

	}NxCatchAll("Error in CInvEditOrderDlg::OnSelChosenLocation");
}

//TES 7/22/2008 - PLID 30802 - Call before DoModal(), and the order will load the products from the given allocation
// details (assuming you pass in -1 for the OrderID in DoModal).
void CInvEditOrderDlg::SetAllocationDetails(const CArray<AllocationDetail,AllocationDetail&> &arAllocationDetails)
{
	//TES 7/22/2008 - PLID 30802 - Copy into our member variable.
	m_arAllocationDetails.RemoveAll();
	for(int i = 0; i < arAllocationDetails.GetSize(); i++) {
		// (a.walling 2008-10-02 09:25) - PLID 31567 - Must add a copy of a const object
		m_arAllocationDetails.Add(AllocationDetail(arAllocationDetails[i]));
	}
}

//TES 7/22/2008 - PLID 30802 - Internal, goes through our passed-in list of allocation details and adds corresponding
// products to the order.
void CInvEditOrderDlg::LoadProductsFromAllocationDetails()
{
	CString strDetailIDString;
	for(int i = 0; i < m_arAllocationDetails.GetSize(); i++) {
		AllocationDetail ad = m_arAllocationDetails[i];
		long nItemRow = m_item->FindByColumn(0, ad.nProductID, 0, VARIANT_FALSE);
		if(nItemRow == -1) {
			//TES 7/22/2008 - PLID 30802 - This shouldn't happen!
			AfxThrowNxException("Could not find product %li while loading order from allocation!", ad.nProductID);
		}
		else {
			//TES 7/22/2008 - PLID 30802 - Add it in.
			AddItem(nItemRow, airNoRule, FALSE, ad.dQty, ad.nAllocationDetailID);
			strDetailIDString += FormatString("%li,", ad.nAllocationDetailID);
		}
	}

	strDetailIDString.TrimRight(",");
	if(!strDetailIDString.IsEmpty()) {
		//TES 7/22/2008 - PLID 30802 - Set the from clause to pull these details (as long as their status is To Be Ordered
		// and they're not an a deleted allocation).
		CString strQuery;
		strQuery.Format("(SELECT PatientInvAllocationsT.ID, NULL AS ParentID, "
			"PatientInvAllocationsT.ID AS AllocationID, "
			"PatientsT.PersonID, Last + ', ' + First + ' ' + Middle AS PatientName, "
			"CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) + convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS ApptDateTime "
			"FROM PatientInvAllocationsT "
			"INNER JOIN PatientsT ON PatientInvAllocationsT.PatientID = PatientsT.PersonID "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"INNER JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
			"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
			"LEFT JOIN UsersT ON PatientInvAllocationsT.CompletedBy = UsersT.PersonID "
			"WHERE PatientInvAllocationsT.Status <> %li AND PatientInvAllocationsT.ID IN "
			"(SELECT PatientInvAllocationDetailsT.AllocationID FROM PatientInvAllocationDetailsT "
			"WHERE PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationDetailsT.ID IN (%s)) "
			"UNION "
			"SELECT NULL AS ID, PatientInvAllocationDetailsT.AllocationID AS ParentID, "
			"PatientInvAllocationDetailsT.AllocationID, "
			"PatientInvAllocationsT.PatientID AS PersonID, "
			"ServiceT.Name + ' (' + Convert(nvarchar, Sum(Quantity)) + ')' AS ProductDesc, "
			"NULL AS ApptDateTime "
			"FROM PatientInvAllocationDetailsT "
			"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"INNER JOIN ProductT ON PatientInvAllocationDetailsT.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"WHERE PatientInvAllocationsT.Status <> %li AND PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationDetailsT.ID IN (%s) "
			"GROUP BY ProductT.ID, ServiceT.Name, PatientInvAllocationDetailsT.AllocationID, PatientInvAllocationsT.PatientID) "
			"AS AllocationsQ", InvUtils::iasDeleted, InvUtils::iadsOrder, strDetailIDString, InvUtils::iasDeleted, InvUtils::iadsOrder, strDetailIDString);

		m_pLinkedAllocations->PutFromClause(_bstr_t(strQuery));
		RequeryLinkedAllocations();
	}

	m_arAllocationDetails.RemoveAll();
}

//TES 7/22/2008 - PLID 30802 - Internal, appends the given detail to the list with the given "ID"
void CInvEditOrderDlg::AddAllocationDetailToList(long nAllocationDetailID, long nAllocationDetailListID)
{
	//TES 7/22/2008 - PLID 30802 - The ID is just the array index, add this detail to the array at that index.
	if(nAllocationDetailListID < 0 || nAllocationDetailListID >= m_arAllocationDetailIDs.GetSize()) {
		AfxThrowNxException("Invalid list ID %li found in CInvEditOrderDlg::AddAllocationDetailToList()", nAllocationDetailListID);
	}
	m_arAllocationDetailIDs[nAllocationDetailListID]->Add(nAllocationDetailID);
}

//TES 7/22/2008 - PLID 30802 - Internal, creates a new list of detail IDs, adds the passed-in detail to it, then returns its
// "ID" (not a data ID, just used in memory during this session).
long CInvEditOrderDlg::GetNewAllocationDetailList(long nInitialAllocationDetailID)
{
	//TES 7/22/2008 - PLID 30802 - Make a new array, add this detail to it, then return its index.
	CArray<long,long> *parNewList = new CArray<long,long>;
	parNewList->Add(nInitialAllocationDetailID);
	m_arAllocationDetailIDs.Add(parNewList);
	return m_arAllocationDetailIDs.GetSize()-1;
}

//TES 7/22/2008 - PLID 30802 - De-allocates and clears out our list of allocation detail IDs.
void CInvEditOrderDlg::ClearAllocationDetailIDs()
{
	//TES 7/22/2008 - PLID 30802 - Go through and delete all the arrays in our master array, then clear it out.
	for(int i = 0; i < m_arAllocationDetailIDs.GetSize(); i++) {
		delete m_arAllocationDetailIDs[i];
	}
	m_arAllocationDetailIDs.RemoveAll();
}

void CInvEditOrderDlg::OnLeftClickLinkedAllocations(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		//TES 7/22/2008 - PLID 30802 - Open up the allocation (copied from CInvAllocationsDlg)

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (j.jones 2007-11-08 15:54) - PLID 28003 - The Patient/Product column is the only hyperlinked
		// column, although that is just an arbitrary decision.
		if(nCol == 4) {
			//edit the allocation

			//alcAllocationID represents the master allocation ID
			//regardless of whether they clicked on a parent row or child row
			long nAllocationID = VarLong(pRow->GetValue(2));
			
			CInvPatientAllocationDlg dlg(this);
			dlg.m_nID = nAllocationID;
			if(dlg.DoModal() == IDOK) {
				//requery the list to reflect any changes to the allocation
				RequeryLinkedAllocations();
			}
		}

	}NxCatchAll("Error in CInvEditOrderDlg::OnLeftClickLinkedAllocations(");
}

void CInvEditOrderDlg::RequeryLinkedAllocations() 
{
	try {
		//TES 7/22/2008 - PLID 30802 - This has to be synchronous, because it can result in showing/hiding controls,
		// which looks weird if we do it asynchronously.
		m_pLinkedAllocations->Requery();
		m_pLinkedAllocations->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

		//TES 7/22/2008 - PLID 30802 - If there are linked allocations, hide the linked appointment label, otherwise,
		// hide the linked allocations.
		if(m_pLinkedAllocations->GetRowCount()) {
			m_bShowApptLinkLabel = false;

			ShowLinkedAllocations();
		}
		else {
			m_bShowApptLinkLabel = true;

			HideLinkedAllocations();
		}

		InvalidateRect(&m_rcApptLinkLabel);

	}NxCatchAll("Error in CInvEditOrderDlg::RequeryLinkedAllocations()");
}

void CInvEditOrderDlg::HideLinkedAllocations()
{
	//TES 7/22/2008 - PLID 30802 - Hide the datalist and label.
	GetDlgItem(IDC_LINKED_ALLOCATIONS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_LINKED_ALLOCATIONS_LABEL)->ShowWindow(SW_HIDE);

	//TES 7/22/2008 - PLID 30802 - Stretch out the notes to cover where the allocations were.
	CRect rcLinkedAllocations;
	GetDlgItem(IDC_LINKED_ALLOCATIONS)->GetWindowRect(rcLinkedAllocations);
	CRect rcNotes;
	GetDlgItem(IDC_NOTES)->GetWindowRect(rcNotes);
	rcNotes.SetRect(rcNotes.left, rcNotes.top, rcLinkedAllocations.right, rcNotes.bottom);
	ScreenToClient(&rcNotes);
	GetDlgItem(IDC_NOTES)->MoveWindow(&rcNotes);

	// (j.jones 2008-09-25 09:57) - PLID 31502 - the linked appt. label is not
	// shown at the same time as allocations, but we should resize it anyways
	// for accuracy
	GetDlgItem(IDC_LINKED_APPT_LABEL)->GetWindowRect(m_rcApptLinkLabel);
	m_rcApptLinkLabel.SetRect(m_rcApptLinkLabel.left, m_rcApptLinkLabel.top, rcNotes.right, m_rcApptLinkLabel.bottom);
	ScreenToClient(&m_rcApptLinkLabel);
	GetDlgItem(IDC_LINKED_APPT_LABEL)->MoveWindow(&m_rcApptLinkLabel);
}

void CInvEditOrderDlg::ShowLinkedAllocations()
{
	//TES 7/22/2008 - PLID 30802 - Shrink the notes to 10 pixels left of the allocations.
	CRect rcLinkedAllocations;
	GetDlgItem(IDC_LINKED_ALLOCATIONS)->GetWindowRect(rcLinkedAllocations);
	CRect rcNotes;
	GetDlgItem(IDC_NOTES)->GetWindowRect(rcNotes);
	rcNotes.SetRect(rcNotes.left, rcNotes.top, rcLinkedAllocations.left-10, rcNotes.bottom);
	ScreenToClient(&rcNotes);
	GetDlgItem(IDC_NOTES)->MoveWindow(&rcNotes);

	//TES 7/22/2008 - PLID 30802 - Show the datalist and label.
	GetDlgItem(IDC_LINKED_ALLOCATIONS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_LINKED_ALLOCATIONS_LABEL)->ShowWindow(SW_SHOW);

	// (j.jones 2008-09-25 09:57) - PLID 31502 - the linked appt. label is not
	// shown at the same time as allocations, but we should resize it anyways
	// for accuracy
	GetDlgItem(IDC_LINKED_APPT_LABEL)->GetWindowRect(m_rcApptLinkLabel);
	m_rcApptLinkLabel.SetRect(m_rcApptLinkLabel.left, m_rcApptLinkLabel.top, rcNotes.right, m_rcApptLinkLabel.bottom);
	ScreenToClient(&m_rcApptLinkLabel);
	GetDlgItem(IDC_LINKED_APPT_LABEL)->MoveWindow(&m_rcApptLinkLabel);
}

void CInvEditOrderDlg::RememberCCOnFile()
{
	long nSupplierID = -1;
	if(m_supplier->CurSel != -1) {
		nSupplierID = VarLong(m_supplier->GetValue(m_supplier->CurSel,S_ID));
	}
	long nCCOnFile = -1;
	if(nSupplierID != -1) {
		nCCOnFile = GetRemotePropertyInt("Supplier_CcOnFile", -1, nSupplierID, "<None>");
	}
	if(nCCOnFile != -1) {
		if(nCCOnFile == 0 && IsDlgButtonChecked(IDC_ORDER_CC_ON_FILE)) {
			CheckDlgButton(IDC_ORDER_CC_ON_FILE, BST_UNCHECKED);
			//TES 5/28/2008 - PLID 30165 - The OnOrderCcOnFile() handler leaves the CC # field untouched.  That makes
			// sense for manually unchecking the box, as your focus (both metaphorical and programatic) is right there
			// next to the box, but in this case, we don't want to leave "ON FILE" (or an old CC#) in that field.
			SetDlgItemText(IDC_ORDER_CC_NUMBER, "");
			OnOrderCcOnFile();
		}
		else if(nCCOnFile == 1 && !IsDlgButtonChecked(IDC_ORDER_CC_ON_FILE)) {
			CheckDlgButton(IDC_ORDER_CC_ON_FILE, BST_CHECKED);
			OnOrderCcOnFile();
		}
	}
	else {
		//TES 8/13/2008 - PLID 30165 - Default to off
		CheckDlgButton(IDC_ORDER_CC_ON_FILE, 0);
					
		
		GetDlgItem(IDC_ORDER_CC_NUMBER)->EnableWindow(TRUE);
		
		
		CString strCcNumber;
		GetDlgItemText(IDC_ORDER_CC_NUMBER, strCcNumber);
		if(strCcNumber == "ON FILE") {
			SetDlgItemText(IDC_ORDER_CC_NUMBER, "");
		}
	}
}

// (j.jones 2008-09-26 09:52) - PLID 30636 - added function to prompt to add to an existing allocation
// and populates nFoundAllocationID and strAllocPatientName with the results, if any
// (a.walling 2010-01-21 15:57) - PLID 37024
void CInvEditOrderDlg::FindApptAllocationID(long nAppointmentID, OUT long &nFoundAllocationID, OUT CString &strAllocPatientName, OUT long& nAllocPatientID)
{
	try {

		nFoundAllocationID = -1;
		strAllocPatientName = "";
		nAllocPatientID = -1;

		_RecordsetPtr rsAllocation = CreateParamRecordset("SELECT "
			"PatientInvAllocationsT.ID, PatientInvAllocationsT.InputDate, "
			"Min(Last + ', ' + First + ' ' + Middle) AS PatientName, "
			"Min(PersonT.ID) AS PatientID, " // (a.walling 2010-01-21 15:55) - PLID 37024
			"Min(ServiceT.Name) AS FirstProduct, "
			"Sum(PatientInvAllocationDetailsT.Quantity) AS SumProductQty "
			"FROM PatientInvAllocationsT "
			"INNER JOIN PatientInvAllocationDetailsT ON PatientInvAllocationsT.ID = PatientInvAllocationDetailsT.AllocationID "
			"INNER JOIN ServiceT ON PatientInvAllocationDetailsT.ProductID = ServiceT.ID "
			"INNER JOIN PersonT ON PatientInvAllocationsT.PatientID = PersonT.ID "
			"WHERE PatientInvAllocationsT.Status = {INT} "
			"AND PatientInvAllocationDetailsT.Status IN ({INT}, {INT}) "
			"AND PatientInvAllocationsT.AppointmentID = {INT} "
			"GROUP BY PatientInvAllocationsT.ID, PatientInvAllocationsT.InputDate",
			InvUtils::iasActive,
			InvUtils::iadsActive, InvUtils::iadsOrder,
			nAppointmentID);
		if(!rsAllocation->eof) {

			//not applied until an allocation is chosen
			//CString strPatientName = AdoFldString(rsAllocation, "FirstProduct", "");
			// (a.walling 2010-01-21 15:53) - PLID 37024 - This should be PatientName
			CString strPatientName = AdoFldString(rsAllocation, "PatientName", "");
			// (a.walling 2010-01-21 15:55) - PLID 37024
			long nPatientID = AdoFldLong(rsAllocation, "PatientID", -1);

			long nRecordCount = rsAllocation->GetRecordCount();
			CString strWarning;
			if(nRecordCount == 1) {
				//there is only one open allocation linked to this appointment,
				//so prompt to use it
				COleDateTime dt = AdoFldDateTime(rsAllocation, "InputDate");
				CString strFirstProduct = AdoFldString(rsAllocation, "FirstProduct", "");
				double dblTotal = AdoFldDouble(rsAllocation, "SumProductQty", 0.0);

				strWarning.Format("The appointment you are linking this order to has "
					"an open inventory allocation linked to it.\n\n"
					"Created: %s\n"
					"First Product: %s\n"
					"Total Products: %g\n\n"
					"Would you like to add the products from this order to this patient's allocation?",
					FormatDateTimeForInterface(dt, NULL, dtoDate), strFirstProduct, dblTotal);
			}
			else {
				//there are multiple open allocations linked to the appt.
				//this will be uncommon, but technically possible
				strWarning.Format("The appointment you are linking this order to has "
					"multiple open inventory allocations linked to it.\n\n"
					"Would you like to add the products from this order to one of these allocations?\n"
					"(If so, you will be given a choice of which allocation to use.)");
			}

			if(IDYES == MessageBox(strWarning, "Practice", MB_YESNO|MB_ICONQUESTION)) {
				
				//they want to do this, so let's get the allocation ID

				if(nRecordCount == 1) {
					//we only have one allocation, so grab its ID
					nFoundAllocationID = AdoFldLong(rsAllocation, "ID");
				}
				else {
					//we have multiple allocations, so we need to display a list,
					//and allow them to pick only one from that list

					//no need to requery when we already have the data in an open recordset,
					//instead we'll build our own select list, and while the multi-select dialog
					//will run it as a query, it will be of virtually no drain on the server's
					//resources

					CString strFrom;
					while(!rsAllocation->eof) {
						long nID = AdoFldLong(rsAllocation, "ID");
						COleDateTime dt = AdoFldDateTime(rsAllocation, "InputDate");
						CString strFirstProduct = AdoFldString(rsAllocation, "FirstProduct", "");
						double dblTotal = AdoFldDouble(rsAllocation, "SumProductQty", 0.0);

						CString str;
						str.Format("SELECT %li AS ID, 'Input Date: %s, First Product: %s, Total Products: %g' AS Description",
							nID, FormatDateTimeForInterface(dt, NULL, dtoDate), strFirstProduct, dblTotal);

						if(!strFrom.IsEmpty()) {
							strFrom += " UNION ";
						}
						strFrom += str;

						rsAllocation->MoveNext();
					}

					strFrom = "(" + strFrom + ") AS ProductsQ";

					CSingleSelectDlg dlg(this);
					int nRet = dlg.Open(strFrom, "", "ID", "Description",
						"Select an Allocation to link with:");

					if(nRet == IDOK) {
						nFoundAllocationID = dlg.GetSelectedID();
					}
				}
			}

			if(nFoundAllocationID != -1) {
				//only if we selected the allocation will we apply the name
				// (a.walling 2010-01-21 15:55) - PLID 37024
				strAllocPatientName = strPatientName;
				nAllocPatientID = nPatientID;
			}
		}
		rsAllocation->Close();

		return;

	}NxCatchAll("Error in CInvEditOrderDlg::FindApptAllocationID");

	//if an error occurred, reset the variables
	nFoundAllocationID = -1;
	strAllocPatientName = "";
}

// (j.jones 2008-09-26 11:07) - PLID 30636 - added function to insert an order detail into an allocation
// bIsSerialized should be TRUE if the product tracks serial numbers or exp. dates, and will force a quantity of 1
// nRecordsToCreate should be 1 unless bIsSerialized = TRUE, and determines how many times the product should be added
// strProductName, strPatientName, and nQuantity are for auditing, so we don't need extra recordsets
// (a.walling 2010-01-21 16:01) - PLID 37024
void CInvEditOrderDlg::InsertOrderDetailIntoAllocation(long nOrderDetailID, long nAllocationID, BOOL bIsSerialized, long nRecordsToCreate,
													   CString strProductName, CString strPatientName, long nPatientID, long nQuantity, long nAuditTransactionID)
{
	//this is called from ApplyItemsToOrder, which is itself called in a transaction, so do not
	//have any messageboxes or exception handling in this function

	//ApplyItemsToOrder does not batch saving or auditing, so we will continue that behavior here,
	//though I would really like to see that changed in the future.

	if(nOrderDetailID == -1) {
		ThrowNxException("CInvEditOrderDlg::InsertOrderDetailIntoAllocation called with an invalid Order Detail ID!");
	}

	if(nAllocationID == -1) {
		ThrowNxException("CInvEditOrderDlg::InsertOrderDetailIntoAllocation called with an invalid Allocation ID!");
	}

	if(bIsSerialized && nRecordsToCreate < 1) {
		//this should never be called with nRecordsToCreate = 0 and bIsSerialized = TRUE
		ASSERT(FALSE);
	}

	if(!bIsSerialized && nRecordsToCreate > 1) {
		//if bIsSerialized = FALSE, nRecordsToCreate is ignored, so why was a value > 1 passed in?
		ASSERT(FALSE);
	}

	//If serialized, we can't have a quantity other than one, so if the product was ordered with
	//a quantity greater than one, we need to add a record for each individual item.
	//The caller is responsible for telling us how many records to create, so we will insert
	//the product record that many times, provided that the item is serialized. If this function
	//was called improperly, the ASSERTs above should have caught that.
	long nTimesToInsert = 1;
	if(bIsSerialized && nRecordsToCreate > 1) {
		nTimesToInsert = nRecordsToCreate;
	}

	CString strSqlBatch;
	for(int i=0; i<nRecordsToCreate; i++) {

		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO PatientInvAllocationDetailsT "
			"(AllocationID, ProductID, ProductItemID, Quantity, Status, Notes, OrderDetailID) "
			"SELECT %li, ProductID, NULL, CASE WHEN %li = 0 THEN QuantityOrdered ELSE 1 END, %li, '', ID "
			"FROM OrderDetailsT WHERE ID = %li",
			nAllocationID, bIsSerialized ? 1 : 0, InvUtils::iadsOrder, nOrderDetailID);

		//audit the creation of each new detail
		{
			CString strDesc = strProductName;
			if(!bIsSerialized && nQuantity != 1) {
				//only show the quantity if not 1, which is impossible on a serialized item
				CString str;
				str.Format(", Quantity: %li", nQuantity);
				strDesc += str;
			}

			//audit the new detail
			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiInvAllocationDetailCreated, nAllocationID, "", strDesc, aepMedium, aetCreated);
		}
	}

	if(!strSqlBatch.IsEmpty()) {
		ExecuteSqlBatch(strSqlBatch);
	}

	if(nAuditTransactionID != -1) {
		CommitAuditTransaction(nAuditTransactionID);
		nAuditTransactionID = -1;
	}
}

// (j.jones 2009-01-23 09:56) - PLID 32822 - added ability to edit order methods
void CInvEditOrderDlg::OnEditOrderMethods()
{
	try {
		_variant_t varValue;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOrderMethodCombo->CurSel;
		if (pRow) {
			varValue = pRow->GetValue(omcID);
		}

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 64, m_pOrderMethodCombo, "Edit Order Methods").DoModal();

		pRow = m_pOrderMethodCombo->GetNewRow();
		pRow->PutValue(omcID, (long)-1);
		pRow->PutValue(omcName, _variant_t(" <None>"));
		m_pOrderMethodCombo->AddRowSorted(pRow, NULL);

		pRow = m_pOrderMethodCombo->SetSelByColumn(omcID, varValue);
		
		if (pRow == NULL) {
			m_pOrderMethodCombo->SetSelByColumn(omcID, (long)-1);
		}

	}NxCatchAll("Error in CInvEditOrderDlg::OnEditOrderMethods");
}

// (d.thompson 2009-01-23) - PLID 32823
void CInvEditOrderDlg::EnsureConfirmedControls() {
	BOOL bEnable = FALSE;
	if(IsDlgButtonChecked(IDC_VENDOR_CONFIRMED_CHK)) {
		//When checked, we enable the other controls
		bEnable = TRUE;
	}
	else {
		//Disable the controls, we don't need to do anything else with the data
		bEnable = FALSE;
	}

	GetDlgItem(IDC_DATE_CONFIRMED)->EnableWindow(bEnable);
	GetDlgItem(IDC_VENDOR_CONF_NUM)->EnableWindow(bEnable);
}

// (d.thompson 2009-01-23) - PLID 32823
void CInvEditOrderDlg::OnVendorConfirmedChkClicked()
{
	try {
		EnsureConfirmedControls();
		if(IsDlgButtonChecked(IDC_VENDOR_CONFIRMED_CHK)) {
			//additionally, set the value of the "date" to today.  This will not overwrite
			//	if for some reason the user previously had something in the field.
			if(m_nxtDateConfirmed->GetStatus() != 1) {
				m_nxtDateConfirmed->SetDateTime(COleDateTime::GetCurrentTime());
			}
		}
		else {
			//Nothing particular to do if they are disabling
		}
	} NxCatchAll("Error in OnVendorConfirmedChkClicked");
}

// (j.jones 2009-02-10 10:32) - PLID 32827 - added OnBtnOrderTrackFedex
void CInvEditOrderDlg::OnBtnOrderTrackFedex()
{
	try {

		//get the current tracking number
		CString strTrackingNumber;
		GetDlgItemText(IDC_TRACKING_NUMBER, strTrackingNumber);
		strTrackingNumber.Replace(" ", "");
		if(strTrackingNumber.IsEmpty()) {
			AfxMessageBox("This order has no tracking number entered. It cannot be tracked online.");
			return;
		}

		CString strHyperlink = GetRemotePropertyText("InvFedExHyperlink", "http://www.fedex.com/Tracking?ascend_header=1&clienttype=dotcom&track=y&cntry_code=us&language=english&tracknumbers={TRACKINGNUMBER}", 0, "<None>", true);
		if(strHyperlink.IsEmpty() || strHyperlink.Find("{TRACKINGNUMBER}") == -1) {
			//this shouldn't really be possible because the preferences should have enforced this
			ASSERT(FALSE);
			AfxMessageBox("The preference for the FedEx Tracking hyperlink is not valid. Please make sure you have a valid hyperlink "
				"entered in the Inventory preferences, that includes the text {TRACKINGNUMBER} as a placeholder for the tracking number.");
			return;
		}

		if(strHyperlink.Replace("{TRACKINGNUMBER}", strTrackingNumber) == 0) {
			//this ought to be impossible if the above checks didn't detect a bad link,
			//so assert if we get here to find out how that could happen
			ASSERT(FALSE);
			AfxMessageBox("Practice could not successfully replace the {TRACKINGNUMBER} placeholder with this order's tracking number.\n"
				"Please ensure that the preference for the FedEx Tracking hyperlink is valid, and includes the text {TRACKINGNUMBER} as a placeholder for the tracking number.");
			return;
		}

		ShellExecute(NULL, NULL, strHyperlink, NULL, NULL, SW_SHOW);
		
	}NxCatchAll("Error in CInvEditOrderDlg::OnBtnOrderTrackFedex");
}

// (j.jones 2009-02-10 16:42) - PLID 32871 - ConfigureDisplay will change the background colors if auto-receiving
void CInvEditOrderDlg::ConfigureDisplay()
{
	try {

		//the defaults are "Order" for the window text, and the standard inventory blue color
		COLORREF clrBkg = STANDARD_INV_COLOR;
		CString strWindowText = "Order";

		//but if auto-receiving, we'll change to a yellowish color, and change the window text
		if(m_bSaveAllAsReceived) {
			clrBkg = AUTO_RECEIVE_COLOR;
			strWindowText = "Order (Auto-Receiving Products)";
		}

		// r.wilson PLID 47393
		if(m_bFrameOrderReception)
		{
			clrBkg = STANDARD_INV_COLOR;
			strWindowText = "Frame Order (Auto-Receiving Products)";
		}

		m_bkg1.SetColor(clrBkg);
		m_bkg2.SetColor(clrBkg);
		m_bkg3.SetColor(clrBkg);
		m_bkg4.SetColor(clrBkg);
		SetWindowText(strWindowText);

	}NxCatchAll("Error in CInvEditOrderDlg::ConfigureDisplay");
}

//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
// This function will likely be updated to pick the most suitable
// UPC code in response to a barcode scan. Practice now allows multiple
// products to have the same UPC codes. Further, products can share UPC codes
// with service codes (however, service codes cannot share UPC codes).

// Current behavior: Returns first matching product.

long CInvEditOrderDlg::GetBestUPCProduct(_variant_t varCode) 
{
	long nRow;
	
	try {

		nRow = m_item->FindByColumn(I_Barcode, varCode, 0, VARIANT_FALSE);
	} NxCatchAll(__FUNCTION__);

	return nRow;
}

void CInvEditOrderDlg::OnBnClickedFrameToProduct()
{
	// (b.spivey, September 20, 2011) - PLID 45266 - Very simple: if they click OK on the Frame to Convert dlg, 
	//	 requery the items list. 
	try{
		if(CheckCurrentUserPermissions(bioInvItem, sptCreate)){
			// (b.spivey, November 22, 2011) - PLID 45266 - Whoops, forgot my VarType calls 
			long nSupplierID = VarLong(m_supplier->GetValue(m_supplier->CurSel, S_ID), -1); 
			CString strSupplierName = VarString(m_supplier->GetValue(m_supplier->CurSel, S_Company), "");

			CInvOrderFramesConvertDlg dlg(nSupplierID, strSupplierName, "", this); 
			if(dlg.DoModal() == IDOK){
				m_item->Requery(); 
				// (b.spivey, September 21, 2011) - PLID 45266 - get the row index of our new product. 
				long nRow = m_item->SetSelByColumn(I_ID, dlg.m_nProductID);
				if(nRow != -1){
					// (b.spivey, September 21, 2011) - PLID 45266 - Should not already exist. 
					AddItem(nRow, airNoRule, FALSE); 
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, September 21, 2011) - PLID 45265 - Avoiding copying code. We need to call this in more than one spot of 
//	 the barcode message. 
BOOL CInvEditOrderDlg::FramesDataExists(CString varCode)
{
	// (b.spivey, September 22, 2011) - PLID 45265 - If there is not a product yet for this frame, we should create it. 
	_RecordsetPtr prs = CreateParamRecordset("SELECT FramesDataT.ID AS FrameID, ProductT.ID AS ProductID FROM FramesDataT "
		"LEFT JOIN ProductT ON FramesDataT.ID = ProductT.FramesDataID "
		"WHERE UPC = {STRING} AND IsCatalog = 0 ", varCode);

	// (b.spivey, September 22, 2011) - PLID 45265 - The only way to get this is if there is NOT YET a product created. 
	if(prs->eof){
		// (b.spivey, November 28, 2011) - PLID 45265 - Fixed unsafe code. 
		long nSupplierID = VarLong(m_supplier->GetValue(m_supplier->CurSel, S_ID)); 
		CString strSupplierName = VarString(m_supplier->GetValue(m_supplier->CurSel, S_Company));

		CInvOrderFramesConvertDlg dlg(nSupplierID, strSupplierName, varCode, this); 

		//They clicked OK.
		if(dlg.DoModal() == IDOK){
			m_item->Requery(); 
			// (b.spivey, September 21, 2011) - PLID 45265 - get the row index of our new product. 
			long nRow = m_item->SetSelByColumn(I_ID, dlg.m_nProductID);
			if(nRow != -1){
				// (b.spivey, September 21, 2011) - PLID 45265 - Should not already exist. 
				AddItem(nRow, airNoRule, FALSE); 
				return TRUE; 
			}
		}
	}
	//Either there is already a product, they clicked cancel, or there is no frames data. All these cases lead to "ignore it." 
	return FALSE; 
}

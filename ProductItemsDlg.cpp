// ProductItemsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ProductItemsDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "barcode.h"
#include "GlobalDrawingUtils.h"
#include "invutils.h"
#include "inventoryrc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_INDEX			0
#define COLUMN_SERIAL_NUMBER	1
#define COLUMN_EXP_DATE			2
// (c.haag 2007-12-03 11:37) - PLID 28204 - This used to be a consignment column, but now
// it's a general status column
#define COLUMN_STATUS			3
#define COLUMN_LOCATION_ID		4
// (j.jones 2008-02-29 12:29) - PLID 29125 - column to determine if the item is in our linked allocation
#define COLUMN_LINKED_ALLOCATION 5

//DRT 11/7/2007 - PLID 28022
#define NEW_SERIAL_NUM_TEXT	"[Enter New Serial Number]"
#define NEW_EXP_DATE_TEXT	"[Enter New Exp. Date]"
#define NO_DATA_TEXT		"<No Data>"

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CProductItemsDlg dialog


// (c.haag 2003-08-01 17:52) - These defaults also need to be
// set in CMainFrame::GetProductItemsDlg()
// (d.thompson 2009-10-21) - PLID 36015 - I got rid of CMainFrame::GetProductItemsDlg().
CProductItemsDlg::CProductItemsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CProductItemsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProductItemsDlg)
	m_bUseSerial = TRUE;
	m_varSerialNumIsLotNum = g_cvarNull;
	m_bUseExpDate = TRUE;
	m_NewItemCount = 0;
	m_EntryType = PI_ENTER_DATA;
	m_CountOfItemsNeeded = 1;
	m_strWhere = "";
	m_bAllowQtyGrow = FALSE;
	m_bDisallowQtyChange = FALSE;
	m_bIsAdjustment = FALSE;
	m_bIsTransfer = FALSE;
	m_bUseUU = FALSE;
	m_bSerialPerUO = FALSE;
	m_nConversion = 1;
	m_nLocationID = -1;
	m_bModeless = false;
	m_strOverrideTitleBarText = "";	// (j.jones 2007-11-08 11:05) - PLID 28041 - added this field
	m_strOverrideSelectQtyText = ""; // (j.jones 2007-11-08 11:05) - PLID 28041 - added this field
	m_nDefaultStatus = 0;	// (j.jones 2007-11-09 09:30) - PLID 28030 - added this field
							// (c.haag 2007-12-03 11:36) - PLID 28204 - Changed from m_bDefaultConsignment
							// to m_nDefaultStatus (general status field)
	m_bUseCloseOnEntryType = false;	//DRT 11/9/2007 - PLID 28054
	m_bSaveDataEntryQuery = false;	//DRT 11/12/2007 - PLID 27999
	m_bCloseAfterScan = false;		//DRT 11/20/2007 - PLID 28138
	m_bCurrentlyReceivingScans = false;	//DRT 11/20/2007 - PLID 28138
	m_bAllowActiveAllocations = false; // (c.haag 2007-12-05 12:43) - PLID 28237
	m_bDisallowStatusChange = false; // (c.haag 2007-12-17 13:40) - PLID 28286
	m_nLinkedAllocationID = -1; // (j.jones 2008-02-29 11:57) - PLID 29125

	m_PrevItemCount = -1; // (a.walling 2008-03-20 11:35) - PLID 29333

	m_strCreatingAdjustmentID = "NULL"; // (j.jones 2009-03-09 12:17) - PLID 33096

	m_adwProductItemIDs.RemoveAll();
	// (c.haag 2008-06-04 10:55) - PLID 29013 - By default, we allow users to select "< No Location >"
	m_bDisallowNonLocations = FALSE;
	// (c.haag 2007-11-08 16:12) - PLID 28025 - I have no idea why the above RemoveAll is there;
	// there should be nothing in it at construction time
	// (c.haag 2008-06-25 11:19) - PLID 28438 - Added m_bDisallowLocationChange
	m_bDisallowLocationChange = FALSE;

	m_clrBkg = RGB(219, 219, 255);

	m_bDeclareNewProductItemID = TRUE;	// (j.jones 2009-07-09 17:09) - PLID 32684

										//}}AFX_DATA_INIT	
}

CString CProductItemsDlg::GetAllocationWhereClause() const
{
	// (c.haag 2007-12-05 12:47) - PLID 28237 - Filter a query by allocations based on m_bAllowActiveAllocations.
	// If m_bAllowActiveAllocations is false, which is the default, then we do not allow any existing
	// allocations. If the flag is true, we will allow allocations as long as they are active and not used.

	// (j.jones 2008-02-29 11:58) - PLID 29125 - if we have a valid m_nLinkedAllocationID, then
	// we should allow the contents of that allocation to show in the list (though this will be used
	// in a larger where clause that may later hide some of the allocation's products)
	CString strLinkedAllocation = "";
	if (m_nLinkedAllocationID != -1) {
		strLinkedAllocation.Format(" AND PatientInvAllocationsT.ID <> %li ", m_nLinkedAllocationID);
	}

	if (!m_bAllowActiveAllocations) {
		return FormatString(
			"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
			"	%s "
			"	AND PatientInvAllocationsT.Status <> %li "
			"		AND (PatientInvAllocationDetailsT.Status IN (%li,%li))) "
			, strLinkedAllocation, InvUtils::iasDeleted, InvUtils::iadsActive, InvUtils::iadsUsed);
	}
	else {
		return FormatString(
			"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
			"	%s "
			"	AND PatientInvAllocationsT.Status <> %li "
			"		AND (PatientInvAllocationDetailsT.Status = %li)) "
			, strLinkedAllocation, InvUtils::iasDeleted, InvUtils::iadsUsed);
	}
}

void CProductItemsDlg::SetAllProductItemStatuses(long /* InvUtils::ProductItemStatus */ pis)
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		  // (c.haag 2007-12-17 13:43) - PLID 28236 - Allows the caller to set a uniform product item status
		  // for every inventory item. Using a long instead of the proper enum to avoid dependency issues
		  // with invutils.h
		const long nRows = m_List->GetRowCount();
		for (long i = 0; i < nRows; i++) {
			IRowSettingsPtr pRow = m_List->GetRow(i);
			pRow->PutValue(COLUMN_STATUS, pis);
		}
	} NxCatchAll("Error in CProductItemsDlg:SetAllProductItemStatuses");
}

void CProductItemsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProductItemsDlg)
	DDX_Control(pDX, IDC_LOT_NUM_LABEL, m_nxsLotNumLabel);
	DDX_Control(pDX, IDC_BTN_APPLY_LOT_NUM_TO_SELECTED, m_nxbApplyLotNumToSelected);
	DDX_Control(pDX, IDC_BTN_APPLY_LOT_NUM, m_nxbApplyLotNum);
	DDX_Control(pDX, IDC_STATIC_SCAN_AS, m_staticScanAs);
	DDX_Control(pDX, IDC_RADIO_SCAN_GENERAL, m_radioScanGeneral);
	DDX_Control(pDX, IDC_RADIO_SCAN_CONSIGNMENT, m_radioScanConsignment);
	DDX_Control(pDX, IDC_BTN_UNSELECT_PRODUCT, m_btnUnselectProduct);
	DDX_Control(pDX, IDC_BTN_SELECT_PRODUCT, m_btnSelectProduct);
	DDX_Control(pDX, IDC_BTN_APPLY_EXP_DATE, m_btnApplyExpDate);
	DDX_Control(pDX, IDC_BTN_APPLY_DATE_TO_SELECTED, m_btnApplyDateToSelected);
	DDX_Control(pDX, IDC_CLOSE_PRODUCT_ITEMS, m_btnCloseProductItems);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EXPDATE, m_dtExpDate);
	DDX_Control(pDX, IDC_QUANTITY, m_nxeditQuantity);
	DDX_Control(pDX, IDC_SELECT_ITEM_LABEL, m_nxstaticSelectItemLabel);
	DDX_Control(pDX, IDC_QUANTITY_LABEL, m_nxstaticQuantityLabel);
	DDX_Control(pDX, IDC_EXP_DATE_LABEL, m_nxstaticExpDateLabel);
	DDX_Control(pDX, IDC_PRODUCT_SELECTED_COUNT, m_nxstaticSelectedCountLabel);
	DDX_Control(pDX, IDC_BACKG, m_bkg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProductItemsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CProductItemsDlg)
	ON_BN_CLICKED(IDC_BTN_APPLY_EXP_DATE, OnBtnApplyExpDate)
	ON_BN_CLICKED(IDC_BTN_APPLY_DATE_TO_SELECTED, OnBtnApplyExpDateToSelected)
	ON_BN_CLICKED(IDC_BTN_SELECT_PRODUCT, OnBtnSelectProduct)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_PRODUCT, OnBtnUnselectProduct)
	ON_EN_KILLFOCUS(IDC_QUANTITY, OnKillfocusQuantity)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_BN_CLICKED(IDC_CLOSE_PRODUCT_ITEMS, OnClose)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BTN_APPLY_LOT_NUM, OnBtnApplyLotNum)
	ON_BN_CLICKED(IDC_BTN_APPLY_LOT_NUM_TO_SELECTED, OnBtnApplyLotNumToSelected)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

_variant_t CProductItemsDlg::GetSelectedProductItemSerialNum(int nIndex)
{
	// (c.haag 2007-11-08 17:17) - PLID 28025 - Returns the serial number of a selected product item
	// given an index
	if (nIndex < 0 || nIndex >= m_avProductItemSerials.GetSize()) {
		ASSERT(FALSE);
		ThrowNxException("Invalid index given for CProductItemsDlg::GetSelectedProductItemSerialNum!");
	}
	else {
		return m_avProductItemSerials[nIndex];
	}
}

_variant_t CProductItemsDlg::GetSelectedProductItemExpDate(int nIndex)
{
	// (c.haag 2007-11-08 17:17) - PLID 28025 - Returns the expiration date of a selected product item
	// given an index
	if (nIndex < 0 || nIndex >= m_avProductItemExpDates.GetSize()) {
		ASSERT(FALSE);
		ThrowNxException("Invalid index given for CProductItemsDlg::GetSelectedProductItemExpDate!");
	}
	else {
		return m_avProductItemExpDates[nIndex];
	}
}

_variant_t CProductItemsDlg::GetSelectedProductItemStatus(int nIndex)
{
	// (c.haag 2008-03-11 12:46) - PLID 29255 - Returns the status of a selected product item.
	// A valid return value is of type VT_I4.
	if (nIndex < 0 || nIndex >= m_avProductItemStatus.GetSize()) {
		ASSERT(FALSE);
		ThrowNxException("Invalid index given for CProductItemsDlg::GetSelectedProductItemStatus!");
	}
	else {
		return m_avProductItemStatus[nIndex];
	}
}

long CProductItemsDlg::GetSelectedProductItemCount() const
{
	// (c.haag 2008-02-15 09:47) - PLID 28286 - Returns the number of selected product items. There
	// should always be as many entries for serial items as there are expiration dates.
	ASSERT(m_avProductItemSerials.GetSize() == m_avProductItemExpDates.GetSize());
	// (c.haag 2008-03-11 13:44) - PLID 29255 - Ensure we also have the same number of status elements
	ASSERT(m_avProductItemSerials.GetSize() == m_avProductItemStatus.GetSize());
	return m_avProductItemSerials.GetSize();
}

/////////////////////////////////////////////////////////////////////////////
// CProductItemsDlg message handlers

BOOL CProductItemsDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		//
		//DRT 11/7/2007 - PLID 28020 - Setup so that we are receiving barcode scan messages.  Note:  This dialog was originally
		//	created as a member variable of the mainfrm, and the only reason we did that was to send barcode messages.  We now have
		//	this new architecture, which is more flexible.
		//TODO:  Do I remove from mainfrm?
		if (m_bCurrentlyReceivingScans == false && !GetMainFrame()->RegisterForBarcodeScan(this)) {
			AfxMessageBox("Failed to register for barcode messages.  You may be unable to use a barcode scanner at this time.");
		}
		else {
			m_bCurrentlyReceivingScans = true;
		}

		// (j.jones 2009-02-11 08:49) - PLID 32871 - color the background
		m_bkg.SetColor(m_clrBkg);

		// (c.haag 2008-04-29 11:41) - PLID 29820 - NxIconify the buttons
		m_btnApplyExpDate.AutoSet(NXB_MODIFY);
		m_btnApplyDateToSelected.AutoSet(NXB_MODIFY);
		m_btnCloseProductItems.AutoSet(NXB_CLOSE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		//TES 7/3/2008 - PLID 24726 - Added buttons for applying lot numbers
		m_nxbApplyLotNum.AutoSet(NXB_MODIFY);
		m_nxbApplyLotNumToSelected.AutoSet(NXB_MODIFY);

		// (a.walling 2008-05-13 15:48) - PLID 27591 - variants no longer necessary for datetime pickers
		m_dtExpDate.SetValue((COleDateTime::GetCurrentTime()));

		m_List = BindNxDataListCtrl(this, IDC_ITEM_LIST, GetRemoteData(), FALSE);
		m_SelectedList = BindNxDataListCtrl(this, IDC_SELECTED_ITEM_LIST, GetRemoteData(), FALSE);

		CString strLocationList;
		// (c.haag 2008-06-04 11:28) - PLID 29013 - If we don't allow non-locations, then don't
		// include "<No Location>" in the combo
		if (m_bDisallowNonLocations) {
			strLocationList = "SELECT ID, Name FROM LocationsT WHERE Active = 1 AND Managed = 1 AND TypeID = 1";
		}
		else {
			strLocationList = "SELECT -1 AS ID, '<No Location>' AS Name "
				"UNION SELECT ID, Name FROM LocationsT WHERE Active = 1 AND Managed = 1 AND TypeID = 1";
		}

		m_List->GetColumn(COLUMN_LOCATION_ID)->ComboSource = _bstr_t(strLocationList);
		m_SelectedList->GetColumn(COLUMN_LOCATION_ID)->ComboSource = _bstr_t(strLocationList);

		// (c.haag 2007-12-03 14:53) - PLID 28204 - Set the combo source for the status column
		CString strStatusSource = FormatString("%d;Purchased Inv.;%d;Consignment",
			InvUtils::pisPurchased, InvUtils::pisConsignment);
		m_List->GetColumn(COLUMN_STATUS)->ComboSource = _bstr_t(strStatusSource);
		m_SelectedList->GetColumn(COLUMN_STATUS)->ComboSource = _bstr_t(strStatusSource);

		m_btnSelectProduct.AutoSet(NXB_DOWN);
		m_btnUnselectProduct.AutoSet(NXB_UP);

		ConfigureDisplay();

		// (a.walling 2008-02-15 12:22) - PLID 28946 - Hide the consignment info if not licensed
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if (!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			m_List->GetColumn(COLUMN_STATUS)->Editable = VARIANT_FALSE;
			m_SelectedList->GetColumn(COLUMN_STATUS)->Editable = VARIANT_FALSE;

			m_List->GetColumn(COLUMN_STATUS)->StoredWidth = 0;
			m_SelectedList->GetColumn(COLUMN_STATUS)->StoredWidth = 0;

			m_bDisallowStatusChange = TRUE;
		}

		// (c.haag 2008-06-05 15:59) - PLID 28311 - Make it so that, by default, serials are scanned
		// in as the default status of the dialog
		if ((long)InvUtils::odsConsignment == m_nDefaultStatus) {
			SetDlgItemCheck(IDC_RADIO_SCAN_CONSIGNMENT, 1);
		}
		else {
			SetDlgItemCheck(IDC_RADIO_SCAN_GENERAL, 1);
		}

		// (c.haag 2008-06-05 15:52) - PLID 28311 - If the entry type is PI_ENTER_DATA, then barcode scans
		// factor in the radio button to assign the item type. If it's not, then hide the radio buttons.
		//TES 1/27/2010 - PLID 36800 - Also need to hide if they don't have the Consignment & Allocation module.
		int nShowBarcodeRadio = ((PI_ENTER_DATA == m_EntryType) && g_pLicense->HasCandAModule(CLicense::cflrSilent)) ? SW_SHOW : SW_HIDE;
		GetDlgItem(IDC_STATIC_SCAN_AS)->ShowWindow(nShowBarcodeRadio);
		GetDlgItem(IDC_RADIO_SCAN_GENERAL)->ShowWindow(nShowBarcodeRadio);
		GetDlgItem(IDC_RADIO_SCAN_CONSIGNMENT)->ShowWindow(nShowBarcodeRadio);

		//DRT 11/9/2007 - PLID 28054 - This used to be in the READ_DATA case, I moved it up here, it's now used in the ENTER... types, if turned on.
		if (m_EntryType == PI_READ_DATA || ((m_EntryType == PI_ENTER_DATA) && m_bUseCloseOnEntryType)) {
			//TES 3/8/2004: If we're in PI_READ_DATA mode, it is impossible to Cancel, so don't give them the option.
			GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
			GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CLOSE_PRODUCT_ITEMS)->ShowWindow(SW_SHOW);
		}


		//if we are adding data, enter in a prompt for data entry for every item received
		if (m_EntryType == PI_ENTER_DATA) {
			if (m_bIsAdjustment)
				SetDlgItemText(IDC_QUANTITY_LABEL, "Quantity added:");
			else
				SetDlgItemText(IDC_QUANTITY_LABEL, "Quantity received:");

			//if using UU/UO, they may want to enter in serial numbers per unit of order,
			//so only enter in the amount ordered in terms of UO
			if (m_bUseUU && m_bSerialPerUO && m_NewItemCount != -2) {
				m_NewItemCount /= m_nConversion;
			}

			if (m_NewItemCount != -2)
				SetDlgItemInt(IDC_QUANTITY, m_NewItemCount);

			//if the m_NewItemCount is not -2, then this is simple, just add a row for each item
			if (m_NewItemCount != -2) {

				for (int i = 0; i<m_NewItemCount; i++) {
					IRowSettingsPtr pRow;
					pRow = m_List->GetRow(-1);
					pRow->PutValue(COLUMN_INDEX, (long)(i + 1));
					_variant_t var;
					var.vt = VT_NULL;
					if (m_bUseSerial)
						pRow->PutValue(COLUMN_SERIAL_NUMBER, _bstr_t(NEW_SERIAL_NUM_TEXT));
					else
						pRow->PutValue(COLUMN_SERIAL_NUMBER, var);
					if (m_bUseExpDate)
						pRow->PutValue(COLUMN_EXP_DATE, _bstr_t(NEW_EXP_DATE_TEXT));
					else
						pRow->PutValue(COLUMN_EXP_DATE, var);

					// (j.jones 2007-11-09 09:28) - PLID 28030 - added Consignment column
					// (c.haag 2007-12-03 11:37) - PLID 28204 - Changed consignment bit to status integer
					//_variant_t varConsign;
					//varConsign.vt = VT_BOOL;
					//varConsign.boolVal = m_bDefaultConsignment;
					pRow->PutValue(COLUMN_STATUS, m_nDefaultStatus);

					// (z.manning 2015-11-04 15:13) - PLID 67405 - We used to pull the location from data based on the order
					// detail ID. However, that was wrong because if we're receiving an order then they may have changed the
					// receiving location in memory. Besides, the order receive dialog sets the location ID already.
					pRow->PutValue(COLUMN_LOCATION_ID, (long)m_nLocationID);

					// (j.jones 2008-02-29 16:38) - PLID 29125 - always false here
					_variant_t varFalse = VARIANT_FALSE;
					pRow->PutValue(COLUMN_LINKED_ALLOCATION, varFalse);

					m_List->InsertRow(pRow, m_List->GetRowCount());
				}
			}
			else {
				//however, if the m_NewItemCount is -2, that means that the dialog is responsible for determining
				//how many items need to be added, and which locations they need to be added to

				long nTotal = 0;

				_RecordsetPtr rs = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1");

				while (!rs->eof) {

					long nLocID = AdoFldLong(rs, "ID");

					double dblAmtOnHand = 0.0;
					double dblAllocated = 0.0;

					// (j.jones 2007-12-18 11:38) - PLID 28037 - CalcAmtOnHand changed to return allocation information,
					// which for now is unused in this code
					if (InvUtils::CalcAmtOnHand(m_ProductID, nLocID, dblAmtOnHand, dblAllocated)) {

						// (j.jones 2007-11-26 09:31) - PLID 28037 - supported allocated items
						_RecordsetPtr rs2 = CreateRecordset("SELECT Count(ID) AS CountOfProdItems FROM ProductItemsT "
							"WHERE ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
							"%s "
							"AND ProductID = %li AND ProductItemsT.Deleted = 0 "
							"AND ProductItemsT.LocationID = %li",
							GetAllocationWhereClause(), m_ProductID, nLocID);
						long LocCountOfProdItems = 0;
						if (!rs2->eof) {
							LocCountOfProdItems = AdoFldLong(rs2, "CountOfProdItems", 0);
						}
						rs2->Close();

						long LocDiff = (long)dblAmtOnHand - LocCountOfProdItems;

						//finally, now add the amount of product items per location!
						for (int i = 0; i<LocDiff; i++) {

							IRowSettingsPtr pRow;
							pRow = m_List->GetRow(-1);
							pRow->PutValue(COLUMN_INDEX, (long)(m_List->GetRowCount() + 1));
							_variant_t var;
							var.vt = VT_NULL;
							if (m_bUseSerial)
								pRow->PutValue(COLUMN_SERIAL_NUMBER, _bstr_t(NEW_SERIAL_NUM_TEXT));
							else
								pRow->PutValue(COLUMN_SERIAL_NUMBER, var);
							if (m_bUseExpDate)
								pRow->PutValue(COLUMN_EXP_DATE, _bstr_t(NEW_EXP_DATE_TEXT));
							else
								pRow->PutValue(COLUMN_EXP_DATE, var);

							// (j.jones 2007-11-09 09:28) - PLID 28030 - added Consignment column
							// (c.haag 2007-12-03 11:37) - PLID 28204 - Changed consignment bit to status integer
							//_variant_t varConsign;
							//varConsign.vt = VT_BOOL;
							//varConsign.boolVal = m_bDefaultConsignment;
							pRow->PutValue(COLUMN_STATUS, m_nDefaultStatus);

							pRow->PutValue(COLUMN_LOCATION_ID, (long)nLocID);

							// (j.jones 2008-02-29 16:38) - PLID 29125 - always false here
							_variant_t varFalse = VARIANT_FALSE;
							pRow->PutValue(COLUMN_LINKED_ALLOCATION, varFalse);

							m_List->InsertRow(pRow, m_List->GetRowCount());
						}

						nTotal += LocDiff;
					}

					rs->MoveNext();
				}
				rs->Close();

				SetDlgItemInt(IDC_QUANTITY, nTotal);
			}
		}
		//if we are not adding data, we're retrieving it, so query accordingly
		else {
			// (j.jones 2008-06-06 09:14) - PLID 27110 - added PI_RETURN_DATA, which is for the most part the same as PI_SELECT_DATA
			if (m_EntryType == PI_SELECT_DATA || m_EntryType == PI_RETURN_DATA) {
				// (j.jones 2007-11-08 11:09) - PLID 28041 - supported an optional override for the "quantity to be billed" text,
				// for use only when the entry type is PI_SELECT_DATA or PI_RETURN_DATA
				// (no else clause needed for this because the "select" text is defaulted in the resources
				if (!m_strOverrideSelectQtyText.IsEmpty()) {
					SetDlgItemText(IDC_QUANTITY_LABEL, m_strOverrideSelectQtyText);
				}
				SetDlgItemInt(IDC_QUANTITY, m_CountOfItemsNeeded);
			}
			else {
				//DRT 11/9/2007 - PLID 28054 - I moved this up above, it's now useful in many cases.
				if (!(GetCurrentUserPermissions(bioInvItem) & (SPT___W________ANDPASS))) {
					m_List->Enabled = FALSE;
				}
			}

			if (m_bIsAdjustment)
				SetDlgItemText(IDC_QUANTITY_LABEL, "Quantity removed:");

			if (m_bIsTransfer)
				SetDlgItemText(IDC_QUANTITY_LABEL, "Quantity moved:");

			CString str, strLoc;

			// (j.jones 2005-02-23 10:17) - If the given location ID is -1, show all items, otherwise,
			// show only items for that location and those items with no location
			if (m_nLocationID != -1)
				strLoc.Format("AND (ProductItemsT.LocationID = %li OR ProductItemsT.LocationID Is Null)", m_nLocationID);

			// (j.jones 2007-11-21 15:29) - PLID 28037 - ensured we hide allocated items

			// (a.walling 2008-02-20 16:15) - PLID 28879 - Hide consignment items when adjusting off
			CString strHideConsignmentItems;
			if (m_bIsAdjustment && m_EntryType == PI_SELECT_DATA) {
				strHideConsignmentItems.Format("AND ProductItemsT.Status = %li", InvUtils::pisPurchased);
			}

			// (j.jones 2008-06-06 09:14) - PLID 27110 - added PI_RETURN_DATA, which in this case
			// will not filter on available items and instead be fully dependent on the m_strWhere clause
			if (m_EntryType == PI_RETURN_DATA) {
				str.Format("ProductItemsT.ProductID = %li "
					"AND ProductItemsT.Deleted = 0 "
					"%s", m_ProductID, strLoc);
			}
			else {
				str.Format("ProductItemsT.ProductID = %li AND ChargesT.ID Is Null AND CaseHistoryDetailsT.ID Is Null "
					"AND ProductItemsT.Deleted = 0 "
					"%s "
					"%s "
					"%s", m_ProductID, strHideConsignmentItems, GetAllocationWhereClause(), strLoc);
			}
			if (m_strWhere != "") {
				str = str + " AND " + m_strWhere;
			}

			// (j.jones 2007-11-09 09:32) - PLID 28030 - added Consignment
			// (c.haag 2007-12-03 11:47) - PLID 28204 - Changed consignment to general status integer
			// (j.jones 2008-02-29 12:23) - PLID 29125 - added column for whether the product is in our
			// allocation specified by m_nLinkedAllocationID

			CString strLinkedAllocationCheck = "0";
			if (m_nLinkedAllocationID != -1) {
				strLinkedAllocationCheck.Format("CASE WHEN ProductItemsT.ID IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
					"	WHERE Status = %li AND AllocationID = %li) THEN 1 ELSE 0 END",
					InvUtils::iadsUsed, m_nLinkedAllocationID);
			}

			_RecordsetPtr rs = CreateRecordset("SELECT ProductItemsT.ID, "
				"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
				"ProductItemsT.Status, ProductItemsT.LocationID, "
				"Convert(bit, %s) AS IsInLinkedAllocation "
				"FROM ProductItemsT "
				"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
				"LEFT JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
				"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
				"WHERE %s", strLinkedAllocationCheck, str);

			while (!rs->eof) {
				IRowSettingsPtr pRow;
				pRow = m_List->GetRow(-1);

				//ID
				pRow->PutValue(COLUMN_INDEX, rs->Fields->Item["ID"]->Value);

				//Serial Number
				pRow->PutValue(COLUMN_SERIAL_NUMBER, _bstr_t(AdoFldString(rs, "SerialNum", NO_DATA_TEXT)));

				//Exp. Date
				_variant_t var = rs->Fields->Item["ExpDate"]->Value;
				if (var.vt == VT_DATE) {
					pRow->PutValue(COLUMN_EXP_DATE, var);
				}
				else {
					pRow->PutValue(COLUMN_EXP_DATE, NO_DATA_TEXT);
				}

				// (j.jones 2007-11-09 09:28) - PLID 28030 - added Consignment column
				// (c.haag 2007-12-03 11:42) - PLID 28204 - Now it's a general status column
				//_variant_t varConsign = rs->Fields->Item["Consignment"]->Value;
				pRow->PutValue(COLUMN_STATUS, rs->Fields->Item["Status"]->Value);

				long nLocID = AdoFldLong(rs, "LocationID", -1);

				pRow->PutValue(COLUMN_LOCATION_ID, nLocID);

				// (j.jones 2008-02-29 12:27) - PLID 29125 - grab the IsInLinkedAllocation field
				BOOL bIsLinkedAllocation = AdoFldBool(rs, "IsInLinkedAllocation");
				pRow->PutValue(COLUMN_LINKED_ALLOCATION, rs->Fields->Item["IsInLinkedAllocation"]->Value);

				if (nLocID == -1 && GetRemotePropertyInt("GrayOutLocationlessItems", 0, 0, "<None>", TRUE) == 1) {
					pRow->PutForeColor(RGB(96, 96, 96));
				}

				// (j.jones 2008-02-29 17:02) - PLID 29125 - if we have a m_nLinkedAllocationID, 
				// we should try to auto-select the row if it is an allocation row
				if (bIsLinkedAllocation && m_SelectedList->GetRowCount() < m_CountOfItemsNeeded) {
					m_SelectedList->AddRow(pRow);
				}
				else {
					m_List->AddRow(pRow);
				}

				rs->MoveNext();
			}
			rs->Close();
		}

		// (j.jones 2009-01-16 11:00) - PLID 32749 - PreLoadValues() needs to be called AFTER the unselected list is loaded
		PreLoadValues();

		// (d.thompson 2009-08-17) - PLID 18899 - Update the selection count
		UpdateSelectedItemsCount();

	}NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CProductItemsDlg::OnOK()
{
	try {

		//if we are adding data, save
		if (m_EntryType == PI_ENTER_DATA) {
			if (!Save())
				return;
			// (c.haag 2007-12-05 16:32) - PLID 28286 - If we are saving data, we want to provide
			// the caller with the ability to also get the serial numbers and expiration dates.
			RememberProductItemValues(m_List);
		}
		//else, return the selected ProductItemID
		// (j.jones 2008-06-06 09:14) - PLID 27110 - added PI_RETURN_DATA, which is for the most part the same as PI_SELECT_DATA
		else if (m_EntryType == PI_SELECT_DATA || m_EntryType == PI_RETURN_DATA) {

			const long count = m_SelectedList->GetRowCount();

			if (count != m_CountOfItemsNeeded && m_bDisallowQtyChange) {
				CString strItem;
				if (m_CountOfItemsNeeded == 1)
					strItem = "item";
				else
					strItem = "items";

				CString str;
				str.Format("You must select %li %s before continuing.", m_CountOfItemsNeeded, strItem);
				AfxMessageBox(str);
				return;
			}

			if (count > m_CountOfItemsNeeded) {
				//JJ - I purposely did not reference m_bAllowQtyGrow here
				//because it should auto-adjust the quantity when you select items
				//then, they will get a message if they add items but then manually reduce the quantity
				CString str, strItemsSel, strItemsToRem;
				if (count == 1)
					strItemsSel = "item";
				else
					strItemsSel = "items";

				if (count - m_CountOfItemsNeeded == 1)
					strItemsToRem = "item";
				else
					strItemsToRem = "items";

				str.Format("You selected %li %s from the list but only specified that you need %li.\n"
					"Do you wish to increase the quantity to %li? If not, please unselect %li %s.",
					count, strItemsSel, m_CountOfItemsNeeded, count, count - m_CountOfItemsNeeded, strItemsToRem);
				if (IDNO == MessageBox(str, "Practice", MB_ICONQUESTION | MB_YESNO))
					return;
				m_CountOfItemsNeeded = count;
				SetDlgItemInt(IDC_QUANTITY, m_CountOfItemsNeeded);
			}
			else if (count < m_CountOfItemsNeeded && count > 0) {
				CString str, strItemsSel, strItemsToRem;
				if (count == 1)
					strItemsSel = "item";
				else
					strItemsSel = "items";

				if (m_CountOfItemsNeeded - count == 1)
					strItemsToRem = "item";
				else
					strItemsToRem = "items";

				str.Format("You selected only %li %s from the list but specified that you need %li.\n"
					"Do you wish to reduce the quantity to %li? If not, please select %li more %s.",
					count, strItemsSel, m_CountOfItemsNeeded, count, m_CountOfItemsNeeded - count, strItemsToRem);
				if (IDNO == MessageBox(str, "Practice", MB_ICONQUESTION | MB_YESNO))
					return;
				else {
					m_CountOfItemsNeeded = count;
					SetDlgItemInt(IDC_QUANTITY, m_CountOfItemsNeeded);
				}
			}
			else if (count == 0) {
				AfxMessageBox("You have not selected any items from the list. Please make your selection or click 'Cancel'.");
				return;
			}

			//validate the expiration dates
			if (m_bUseExpDate) {
				BOOL bExpired = FALSE;
				for (int i = 0; i<m_SelectedList->GetRowCount(); i++) {
					COleDateTime dtExp;
					_variant_t varExpDate = m_SelectedList->GetValue(i, COLUMN_EXP_DATE);
					CString strExpDate;
					if (varExpDate.vt != VT_DATE) {
						strExpDate = VarString(varExpDate, "");
						dtExp.ParseDateTime(strExpDate);
					}
					else
						dtExp = varExpDate.date;
					COleDateTime dtToday;
					COleDateTime dtNow;
					dtNow = COleDateTime::GetCurrentTime();
					dtToday.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());
					if (!(varExpDate.vt == VT_BSTR && VarString(varExpDate, "") == NO_DATA_TEXT))
						if (dtExp < dtToday) {
							bExpired = TRUE;
						}
				}

				if (bExpired) {
					if (IDNO == MessageBox("At least one item you have selected has already expired. Do you still wish to use these items?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
						return;
					}
				}
			}

			// (c.haag 2007-11-08 16:58) - PLID 28025 - If we are selecting data, we want to provide
			// the caller with the ability to also get the serial numbers and expiration dates.
			RememberProductItemValues(m_SelectedList);
		}

		m_PrevItemCount = -1;

		//DRT 11/6/2007 - PLID 28018 - If we're operating modelessly, just hide the dialog.
		if (!m_bModeless) {
			CNxDialog::OnOK();
		}
		else {
			ShowWindow(SW_HIDE);

			//DRT 11/7/2007 - PLID 28018 - This, again, is just like billing.  When we close the dialog, we must let our parent know that 
			//	the dialog has closed.
			if (GetParent()) {
				//Give ourselves as the pointer, so just in case they've got multiple, they'll know which one completed.
				GetParent()->PostMessage(NXM_POST_EDIT_PRODUCT_ITEMS_DLG, (WPARAM)this, IDOK);
			}
		}
	}
	NxCatchAll("Error in CProductItemsDlg::OnOK");
}

void CProductItemsDlg::OnCancel()
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		  //whether we are adding data or selecting an item, we must warn them that they should not cancel
		if (m_EntryType == PI_ENTER_DATA) {
			//DRT 11/9/2007 - PLID 28054 - If we are closing instead of cancelling... this should just pretend like this hit OK instead of cancel.
			if (m_bUseCloseOnEntryType) {
				OnOK();
				return;
			}

			if (IDNO == MessageBox("You have not saved these changes, are you sure you wish to cancel?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
				return;
			}
		}
		else {

		}

		// (a.walling 2008-03-20 11:36) - PLID 29333 - If cancelling, we can remove the new items
		if (m_PrevItemCount != -1) {
			for (int i = 0; i < m_List->GetRowCount(); i++) {
				if (VarLong(m_List->Value[i][COLUMN_INDEX], -1) > m_PrevItemCount) {
					m_List->RemoveRow(i);
					i--;
				}
			}

			ASSERT(m_PrevItemCount == m_List->GetRowCount());
			m_NewItemCount = m_PrevItemCount;
			m_PrevItemCount = -1;

			SetDlgItemInt(IDC_QUANTITY, m_NewItemCount);
		}

		//DRT 11/6/2007 - PLID 28018 - If we're operating modelessly, just hide the dialog.
		if (!m_bModeless) {
			CNxDialog::OnCancel();
		}
		else {
			ShowWindow(SW_HIDE);

			//DRT 11/7/2007 - PLID 28018 - This, again, is just like billing.  When we close the dialog, we must let our parent know that 
			//	the dialog has closed.
			if (GetParent()) {
				//Give ourselves as the pointer, so just in case they've got multiple, they'll know which one completed.
				GetParent()->PostMessage(NXM_POST_EDIT_PRODUCT_ITEMS_DLG, (WPARAM)this, IDCANCEL);
			}
		}
	} NxCatchAll("Error in CProductItemsDlg:OnCancel");
}

// (c.haag 2007-12-07 15:05) - PLID 28286 - Added a silent flag. At the time of this implementation,
// this can only happen if a user has entered an order for items already received...in which case
// the verification has already happened once before.
BOOL CProductItemsDlg::Save(BOOL bSilent /* = FALSE */)
{
	try {

		//only called when adding new data

		BOOL bExpired = FALSE;

		//DRT 11/6/2007 - PLID 28008 - If we are allowing a quantity change, then we want to just skip the rows that are non-data.  However, to
		//	simply do that, I had to combine these 2 iterations over the datalist into one iteration.
		//DRT 11/6/2007 - PLID 27999 - In order to properly generate m_strSavedDataEntryQuery, we will batch this loop of 
		//	queries into a single execution.  That also will speed things up a little bit.
		CString strSql = BeginSqlBatch();

		// (j.jones 2009-07-09 17:08) - PLID 32684 - required for inv. reconciliation adjusting,
		// where we will be calling this function several times in a row
		// (b.spivey, February 17, 2012) - PLID 48080 - Removed declaration and incrementing logic for @nNewProductItemID, 
		//   as ProductItemsT.ID is an identity now. 

		//if using UU/UO, they may want to enter in serial numbers per unit of order,
		//but we need to save them per unit of usage
		long nReps = 1;
		if (m_bUseUU && m_bSerialPerUO) {
			nReps = m_nConversion;
		}

		//DRT 11/16/2007 - PLID 19682 - Keep track of # to be saved in case we need to warn.
		long nCountToSave = 0;

		//loop through all rows, and if any row does not have data in it, warn the user and stop the process
		for (int i = 0; i<m_List->GetRowCount(); i++) {
			//DRT 11/6/2007 - PLID 28008 - It's now possible we won't save this row.
			bool bSaveThisRow = true;

			{
				CString strSerNum = VarString(m_List->GetValue(i, COLUMN_SERIAL_NUMBER), "");

				_variant_t varExpDate = m_List->GetValue(i, COLUMN_EXP_DATE);
				CString strExpDate;
				if (varExpDate.vt != VT_DATE)
					strExpDate = VarString(varExpDate, "");
				else
					strExpDate = FormatDateTimeForSql(varExpDate, dtoDate);

				strSerNum.TrimRight();
				strExpDate.TrimRight();
				//only check for serial numbers if we are using them
				if (m_bUseSerial && !IsSerialNumberTextValid(strSerNum)) {
					//DRT 11/6/2007 - PLID 28008 - If we are allowing a qty change, then these items will just be skipped past.
					if (m_bDisallowQtyChange) {
						if (!bSilent) { AfxMessageBox("Please enter a Serial Number for all items."); }
						return FALSE;
					}
					else {
						//We are allowed to skip.  Nothing will be saved to data.  Just continue the for loop, which will cause nothing
						//	to be generated for this specific row.
						bSaveThisRow = false;
					}
				}
				//only check for expiration dates if we are using them
				if (m_bUseExpDate && !IsExpirationTextValid(strExpDate)) {
					//DRT 11/6/2007 - PLID 28008 - If we are allowing a qty change, then these items will just be skipped past.
					if (m_bDisallowQtyChange) {
						if (!bSilent) { AfxMessageBox("Please enter an Expiration Date for all items."); }
						return FALSE;
					}
					else {
						//We are allowed to skip.  Nothing will be saved to data.  Just continue the for loop, which will cause nothing
						//	to be generated for this specific row.
						bSaveThisRow = false;
					}
				}
				//validate the expiration date
				if (m_bUseExpDate) {
					COleDateTime dtExp;
					dtExp.ParseDateTime(strExpDate);
					//DRT 11/12/2007 - PLID 28008 - Previously we never got here if invalid, but we can now.  Since we allow
					//	invalid strings, we do not want to warn that the product is expired.
					if (dtExp.GetStatus() == COleDateTime::valid) {
						COleDateTime dtToday;
						COleDateTime dtNow;
						dtNow = COleDateTime::GetCurrentTime();
						dtToday.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());
						if (dtExp < dtToday) {
							bExpired = TRUE;
						}
					}
				}
			}



			//DRT 11/6/2007 - PLID 28008 - All code below here is what used to be in the secondary loop below.  Since I made that a batched SQL statement
			//	in PLID 27999, there's really no reason it cannot be shifted up here.  We just need to take care not to execute it until the below checks
			//	have completed.
			if (bSaveThisRow) {
				nCountToSave++;
				CString strSerNum;
				CString strExpDate;
				CString strOrderDetailID;
				CString strCreatingAdjustmentID;

				if (m_bUseSerial)
					strSerNum = "'" + _Q(CString(m_List->GetValue(i, COLUMN_SERIAL_NUMBER).bstrVal)) + "'";
				else
					strSerNum = "NULL";

				if (m_bUseExpDate) {
					COleDateTime dtExpDate;
					_variant_t var = m_List->GetValue(i, COLUMN_EXP_DATE);
					if (var.vt == VT_DATE)
						dtExpDate = var.date;
					else
						dtExpDate.ParseDateTime(CString(var.bstrVal));

					if (dtExpDate.GetStatus() == COleDateTime::valid)
						strExpDate = "'" + _Q(FormatDateTimeForSql(dtExpDate, dtoDate)) + "'";
					else
						strExpDate = "NULL";
				}
				else
					strExpDate = "NULL";

				//TES 6/18/2008 - PLID 29578 - We now have an OrderDetailID rather than an OrderID
				if (m_nOrderDetailID > -1) {
					strOrderDetailID.Format("%li", m_nOrderDetailID);
				}
				else {
					strOrderDetailID = "NULL";
				}

				// (j.jones 2007-11-09 09:35) - PLID 28030 - added Consignment
				// (c.haag 2007-12-03 11:43) - PLID 28204 - Changed from consignment to a more general status field
				long nStatus = VarLong(m_List->GetValue(i, COLUMN_STATUS), InvUtils::pisPurchased);

				long nLocationID = VarLong(m_List->GetValue(i, COLUMN_LOCATION_ID), -1);

				CString strLocationID = "NULL";

				if (nLocationID != -1)
					strLocationID.Format("%li", nLocationID);

				//will only be greater than 1 if we entered per UO
				for (int j = 0; j<nReps; j++) {
					CString str;
					// (j.jones 2007-11-09 09:34) - PLID 28030 - added Consignment
					// (c.haag 2007-12-03 11:47) - PLID 28204 - Change consignment bit to status integer
					//TES 6/18/2008 - PLID 29578 - ProductItemsT now takes an OrderDetailID, not an an OrderID
					// (j.jones 2009-03-09 12:16) - PLID 33096 - added CreatingAdjustmentID 
					// (b.spivey, February 17, 2012) - PLID 48080 - Removed ID, as it is an identity now. 
					str.Format("INSERT INTO ProductItemsT (ProductID, SerialNum, ExpDate, Status, OrderDetailID, LocationID, AdjustmentID, CreatingAdjustmentID) "
						"VALUES (%li, %s, %s, %li, %s, %s, %s, %s);\r\n",
						m_ProductID, strSerNum, strExpDate, nStatus, strOrderDetailID, strLocationID, m_strCreatingAdjustmentID, m_strCreatingAdjustmentID);
					AddStatementToSqlBatch(strSql, str);

					//Now increment our "new id" counter
					// (b.spivey, February 17, 2012) - PLID 48080 - Removed incrementing logic.  
				}
			}
		}

		//DRT 11/16/2007 - PLID 19682 - If the item is a UU type, we MUST save an evenly divisible number of items per conversion rate.  For example, 
		//	if there are 4 items being ordered, and a conversion rate of 10:1, then there are 40 total serial numbers on this dialog.  We cannot allow
		//	them to dismiss this dialog with nCountToSave % m_nConversion != 0 (so either 0, 10, 20, 30, or 40 in this case).
		if (!m_bDisallowQtyChange && m_bUseUU && !m_bSerialPerUO && (nCountToSave % m_nConversion != 0)) {
			if (!bSilent) {
				AfxMessageBox("Because you are ordering a product with different units of usage, you must fill in at least 1 full unit of order.  For example, if "
					"you are ordering 2 of a product which has 10 units of usage per unit of order, you must fill in exactly 0, 10, or 20 rows of the serialized data.");
			}
			return FALSE;
		}

		if (bExpired) {
			if (!bSilent) {
				if (IDNO == MessageBox("At least one item entered has already expired. Do you still wish to save?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
					return FALSE;
				}
			}
			else {
				// (c.haag 2007-12-07 15:06) - PLID 28286 - We're silent...but assume the user wants to say "yes"
			}
		}

		// (j.jones 2005-02-21 17:10) - keep a count of how many items we have set to be per location,
		// then match them up with the amounts on hand per location. If we are going to save with a mismatch,
		// warn the user first.
		if (!VerifyLocationSave(bSilent))
			return FALSE;

		//DRT 11/6/2007 - PLID 27999 - Now that we have generated a SQL statement -- If we are just on the standard data entry type, just execute
		//	it all now.  If we're on the new Generate entry type, then set the member variable that holds it, and do no data execution at this time.
		if (m_bSaveDataEntryQuery) {
			m_strSavedDataEntryQuery = strSql;
		}
		else {
			//Execute immediately
			if (!strSql.IsEmpty()) {

				//safety check: has another user already received these products?
				if (m_nOrderDetailID != -1 && InvUtils::HasOrderDetailReceivedProductItems(m_nOrderDetailID)) {
					//someone has already received this product
					//***ignore the silent flag in this case***
					AfxMessageBox("This product has already been received. You must cancel out of this window and attempt receiving the order again to ensure the order status has been updated.", MB_ICONERROR | MB_OK);
					return FALSE;
				}

				ExecuteSqlBatch(strSql);
			}
		}

		return TRUE;

	}NxCatchAll("Error saving items.");

	return FALSE;
}

void CProductItemsDlg::OnBtnApplyExpDate()
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		  //clicking this button will auto-apply one exp. date
		  //to all items in the list. The dates can be edited later on
		  //if the user wishes to do so.

		_variant_t var = m_dtExpDate.GetValue();

		//We don't want times. Just dates.
		COleDateTime dt = var.date;
		dt.SetDateTime(dt.GetYear(), dt.GetMonth(), dt.GetDay(), 0, 0, 0);
		var = dt;

		/*
		CString str;
		str.Format("This action will update all the products in this list to use %s as the expiration date.\n"
		"Are you sure you wish to do this?", FormatDateTimeForInterface(dt, NULL, dtoDate));
		if(IDNO == MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
		return;
		}
		*/

		for (int i = 0; i < m_List->GetRowCount(); i++) {
			m_List->PutValue(i, COLUMN_EXP_DATE, var);
		}
	} NxCatchAll("Error in CProductItemsDlg:OnBtnApplyExpDate");
}

void CProductItemsDlg::OnBtnApplyExpDateToSelected()
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		if (m_List->CurSel == -1) {
			AfxMessageBox("You must select at least one product from the list first.");
			return;
		}

		//clicking this button will auto-apply one exp. date
		//to all selected items in the list. The dates can be edited later on
		//if the user wishes to do so.

		_variant_t var = m_dtExpDate.GetValue();

		//We don't want times. Just dates.
		COleDateTime dt = var.date;
		dt.SetDateTime(dt.GetYear(), dt.GetMonth(), dt.GetDay(), 0, 0, 0);
		var = dt;

		long i = 0;
		long p = m_List->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;


		/*
		//loop once for the count
		while (p) {

		i++;

		m_List->GetNextSelEnum(&p, &pDisp);

		IRowSettingsPtr pRow(pDisp);

		pDisp->Release();
		}

		//now prompt

		CString str;
		str.Format("This action will update the %li selected products to use %s as the expiration date.\n"
		"Are you sure you wish to do this?", i, FormatDateTimeForInterface(dt, NULL, dtoDate));
		if(IDNO == MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
		return;
		}

		//now actually update the expiration date

		i = 0;
		p = m_List->GetFirstSelEnum();
		*/

		while (p) {

			i++;

			m_List->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			pRow->PutValue(COLUMN_EXP_DATE, var);

			pDisp->Release();
		}
	} NxCatchAll("Error in CProductItemsDlg:OnBtnApplyExpDateToSelected");
}

BEGIN_EVENTSINK_MAP(CProductItemsDlg, CNxDialog)
	//{{AFX_EVENTSINK_MAP(CProductItemsDlg)
	ON_EVENT(CProductItemsDlg, IDC_ITEM_LIST, 3 /* DblClickCell */, OnDblClickCellItemList, VTS_I4 VTS_I2)
	ON_EVENT(CProductItemsDlg, IDC_SELECTED_ITEM_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedItemList, VTS_I4 VTS_I2)
	ON_EVENT(CProductItemsDlg, IDC_ITEM_LIST, 9 /* EditingFinishing */, OnEditingFinishingItemList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CProductItemsDlg, IDC_ITEM_LIST, 10 /* EditingFinished */, OnEditingFinishedItemList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CProductItemsDlg, IDC_ITEM_LIST, 6 /* RButtonDown */, OnRButtonDownItemList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CProductItemsDlg::OnDblClickCellItemList(long nRowIndex, short nColIndex)
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		  // (j.jones 2008-06-06 09:14) - PLID 27110 - added PI_RETURN_DATA, which is for the most part the same as PI_SELECT_DATA
		if (m_EntryType != PI_SELECT_DATA && m_EntryType != PI_RETURN_DATA) {
			return;
		}

		if (nRowIndex == -1)
			return;

		m_SelectedList->TakeCurrentRow(m_List);

		if (m_bAllowQtyGrow) {
			long count = m_SelectedList->GetRowCount();
			if (count > 0)
				m_CountOfItemsNeeded = count;
			SetDlgItemInt(IDC_QUANTITY, m_CountOfItemsNeeded);
		}

		// (d.thompson 2009-08-17) - PLID 18899 - Update the selection count
		UpdateSelectedItemsCount();
	} NxCatchAll("Error in CProductItemsDlg:OnDblClickCellItemList");
}

void CProductItemsDlg::OnBtnSelectProduct()
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		if (m_List->GetCurSel() != -1)
			m_SelectedList->TakeCurrentRow(m_List);

		if (m_bAllowQtyGrow) {
			long count = m_SelectedList->GetRowCount();
			if (count > 0)
				m_CountOfItemsNeeded = count;
			SetDlgItemInt(IDC_QUANTITY, m_CountOfItemsNeeded);
		}

		// (d.thompson 2009-08-17) - PLID 18899 - Update the selection count
		UpdateSelectedItemsCount();
	} NxCatchAll("Error in CProductItemsDlg:OnBtnSelectProduct");
}

void CProductItemsDlg::OnBtnUnselectProduct()
{
	try { // (b.eyers 2015-05-20) - PLID 28048
		if (m_SelectedList->GetCurSel() != -1)
			m_List->TakeCurrentRow(m_SelectedList);

		if (m_bAllowQtyGrow) {
			long count = m_SelectedList->GetRowCount();
			if (count > 0)
				m_CountOfItemsNeeded = count;
			SetDlgItemInt(IDC_QUANTITY, m_CountOfItemsNeeded);
		}

		// (d.thompson 2009-08-17) - PLID 18899 - Update the selection count
		UpdateSelectedItemsCount();
	} NxCatchAll("Error in CProductItemsDlg:OnBtnUnselectProduct");
}

void CProductItemsDlg::OnDblClickCellSelectedItemList(long nRowIndex, short nColIndex)
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		if (nRowIndex == -1)
			return;

		m_List->TakeCurrentRow(m_SelectedList);

		if (m_bAllowQtyGrow) {
			long count = m_SelectedList->GetRowCount();
			if (count > 0)
				m_CountOfItemsNeeded = count;
			SetDlgItemInt(IDC_QUANTITY, m_CountOfItemsNeeded);
		}

		// (d.thompson 2009-08-17) - PLID 18899 - Update the selection count
		UpdateSelectedItemsCount();
	} NxCatchAll("Error in CProductItemsDlg:OnDblClickCellSelectedItemList");
}

void CProductItemsDlg::SetColumnWidths() {

	try { // (b.eyers 2015-05-20) - PLID 28048

		  //only show the columns that we are using

		  // (j.jones 2007-11-09 09:53) - PLID 28030 - updated to handle the consignment column

		  //TES 7/7/2008 - PLID 24726 - Change the column title if our serial number is actually a lot number.
		  // (j.jones 2010-05-07 09:05) - PLID 36454 - SerialNumIsLotNum is now a variant
		if (m_bUseSerial && VarBool(m_varSerialNumIsLotNum, FALSE)) {
			m_List->GetColumn(COLUMN_SERIAL_NUMBER)->PutColumnTitle(_bstr_t("Lot Number"));
			m_SelectedList->GetColumn(COLUMN_SERIAL_NUMBER)->PutColumnTitle(_bstr_t("Lot Number"));
		}

		//use serial number, exp. date, and index
		//DRT 11/6/2007 - PLID 27999 - New generation type acts the same as entry type
		// (c.haag 2007-12-03 11:44) - PLID 28204 - Changed all consignment columns to status columns
		if (m_bUseSerial && m_bUseExpDate && m_EntryType == PI_ENTER_DATA) {
			m_List->GetColumn(COLUMN_INDEX)->PutStoredWidth(8);
			m_List->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(35);
			m_List->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(18);
			m_List->GetColumn(COLUMN_STATUS)->PutStoredWidth(21);
			m_List->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(18);
		}
		//use only exp. date. and index
		else if (!m_bUseSerial && m_bUseExpDate && m_EntryType == PI_ENTER_DATA) {
			m_List->GetColumn(COLUMN_INDEX)->PutStoredWidth(8);
			m_List->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(0);
			m_List->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(50);
			m_List->GetColumn(COLUMN_STATUS)->PutStoredWidth(21);
			m_List->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(21);
		}
		//use only serial number and index
		else if (m_bUseSerial && !m_bUseExpDate && m_EntryType == PI_ENTER_DATA) {
			m_List->GetColumn(COLUMN_INDEX)->PutStoredWidth(8);
			m_List->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(50);
			m_List->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(0);
			m_List->GetColumn(COLUMN_STATUS)->PutStoredWidth(21);
			m_List->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(21);
		}
		//use serial number and exp. date, but no index
		else if (m_bUseSerial && m_bUseExpDate && m_EntryType != PI_ENTER_DATA) {
			m_List->GetColumn(COLUMN_INDEX)->PutStoredWidth(0);
			m_List->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(40);
			m_List->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(18);
			m_List->GetColumn(COLUMN_STATUS)->PutStoredWidth(21);
			m_List->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(21);

			// (j.jones 2008-06-06 09:14) - PLID 27110 - added PI_RETURN_DATA, which is for the most part the same as PI_SELECT_DATA
			if (m_EntryType == PI_SELECT_DATA || m_EntryType == PI_RETURN_DATA) {
				m_SelectedList->GetColumn(COLUMN_INDEX)->PutStoredWidth(0);
				m_SelectedList->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(40);
				m_SelectedList->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(18);
				m_SelectedList->GetColumn(COLUMN_STATUS)->PutStoredWidth(21);
				m_SelectedList->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(21);
			}
		}
		//use only exp. date.
		else if (!m_bUseSerial && m_bUseExpDate && m_EntryType != PI_ENTER_DATA) {
			m_List->GetColumn(COLUMN_INDEX)->PutStoredWidth(0);
			m_List->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(0);
			m_List->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(53);
			m_List->GetColumn(COLUMN_STATUS)->PutStoredWidth(21);
			m_List->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(26);

			// (j.jones 2008-06-06 09:14) - PLID 27110 - added PI_RETURN_DATA, which is for the most part the same as PI_SELECT_DATA
			if (m_EntryType == PI_SELECT_DATA || m_EntryType == PI_RETURN_DATA) {
				m_SelectedList->GetColumn(COLUMN_INDEX)->PutStoredWidth(0);
				m_SelectedList->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(0);
				m_SelectedList->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(53);
				m_SelectedList->GetColumn(COLUMN_STATUS)->PutStoredWidth(21);
				m_SelectedList->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(26);
			}
		}
		//use only serial number
		else if (m_bUseSerial && !m_bUseExpDate && m_EntryType != PI_ENTER_DATA) {
			m_List->GetColumn(COLUMN_INDEX)->PutStoredWidth(0);
			m_List->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(53);
			m_List->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(0);
			m_List->GetColumn(COLUMN_STATUS)->PutStoredWidth(21);
			m_List->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(26);
			// (j.jones 2008-06-06 09:14) - PLID 27110 - added PI_RETURN_DATA, which is for the most part the same as PI_SELECT_DATA
			if (m_EntryType == PI_SELECT_DATA || m_EntryType == PI_RETURN_DATA) {
				m_SelectedList->GetColumn(COLUMN_INDEX)->PutStoredWidth(0);
				m_SelectedList->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(53);
				m_SelectedList->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(0);
				m_SelectedList->GetColumn(COLUMN_STATUS)->PutStoredWidth(21);
				m_SelectedList->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(26);
			}
		}
	} NxCatchAll("Error in CProductItemsDlg:SetColumnWidths");
}

void CProductItemsDlg::ShowRelevantFields() {

	try { // (b.eyers 2015-05-20) - PLID 28048

		  //TES 7/17/2008 - PLID 24726 - Need to load SerialNumIsLotNum.  This isn't the most efficient way in the world to do
		  // this, but many of the places that call this dialog weren't really in a positioni to know whether this flag should
		  // be set.
		  // (j.jones 2010-05-07 09:05) - PLID 36454 - SerialNumIsLotNum is now a variant, if it is not null we don't need to load
		if (m_varSerialNumIsLotNum.vt != VT_BOOL) {
			_RecordsetPtr rs = CreateParamRecordset("SELECT SerialNumIsLotNum FROM ProductT WHERE ID = {INT}", m_ProductID);
			if (!rs->eof) {
				m_varSerialNumIsLotNum = rs->Fields->Item["SerialNumIsLotNum"]->Value;
			}
			rs->Close();
		}

		CRect rcItems, rcSelected, rcTopLabel, rcExpDateLabel, rcLotNumLabel, rcSelCountLabel;
		GetDlgItem(IDC_ITEM_LIST)->GetWindowRect(rcItems);
		ScreenToClient(rcItems);
		GetDlgItem(IDC_EXP_DATE_LABEL)->GetWindowRect(rcExpDateLabel);
		ScreenToClient(rcExpDateLabel);
		GetDlgItem(IDC_SELECT_ITEM_LABEL)->GetWindowRect(rcTopLabel);
		ScreenToClient(rcTopLabel);
		GetDlgItem(IDC_SELECTED_ITEM_LIST)->GetWindowRect(rcSelected);
		ScreenToClient(rcSelected);
		// (d.thompson 2009-08-17) - PLID 18899 - Added for future use, but we don't use this right now.
		GetDlgItem(IDC_PRODUCT_SELECTED_COUNT)->GetWindowRect(rcSelCountLabel);
		ScreenToClient(rcSelCountLabel);
		//TES 7/3/2008 - PLID 24726 - There are also controls now for applying lot numbers
		GetDlgItem(IDC_LOT_NUM_LABEL)->GetWindowRect(rcLotNumLabel);
		ScreenToClient(rcLotNumLabel);

		//if we are not showing the exp. date OR we aren't entering data,
		//hide the exp. date utilities
		//DRT 11/6/2007 - PLID 27999 - New generation type acts the same as entry type
		if (!m_bUseExpDate || m_EntryType != PI_ENTER_DATA) {
			GetDlgItem(IDC_EXP_DATE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EXPDATE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_APPLY_EXP_DATE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_APPLY_DATE_TO_SELECTED)->ShowWindow(SW_HIDE);
		}
		//TES 7/3/2008 - PLID 24726 - If we aren't treating serial numbers as lot numbers OR we aren't entering data,
		// hide the lot number utilities
		// (j.jones 2010-05-07 09:05) - PLID 36454 - SerialNumIsLotNum is now a variant
		if (!VarBool(m_varSerialNumIsLotNum, FALSE) || m_EntryType != PI_ENTER_DATA) {
			GetDlgItem(IDC_LOT_NUM_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LOT_NUMBER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_APPLY_LOT_NUM)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_APPLY_LOT_NUM_TO_SELECTED)->ShowWindow(SW_HIDE);
		}

		//set the window captions and behavior
		// (j.jones 2008-06-06 09:14) - PLID 27110 - added PI_RETURN_DATA, which is for the most part the same as PI_SELECT_DATA
		if (m_EntryType == PI_SELECT_DATA || m_EntryType == PI_RETURN_DATA) {
			// (j.jones 2007-11-08 11:06) - PLID 28041 - supported an optional override for the title bar text
			if (!m_strOverrideTitleBarText.IsEmpty()) {
				SetWindowText(m_strOverrideTitleBarText);
			}
			else {
				SetWindowText("Select Item");
			}
			m_List->GetColumn(COLUMN_SERIAL_NUMBER)->PutEditable(VARIANT_FALSE);
			m_List->GetColumn(COLUMN_EXP_DATE)->PutEditable(VARIANT_FALSE);
			// (j.jones 2007-11-09 09:53) - PLID 28030 - added the Consignment column
			// (c.haag 2007-12-03 11:45) - PLID 28204 - Now a status column
			m_List->GetColumn(COLUMN_STATUS)->PutEditable(VARIANT_FALSE);
			m_List->GetColumn(COLUMN_LOCATION_ID)->PutEditable(VARIANT_FALSE);

			if (m_bDisallowQtyChange)
				((CNxEdit*)GetDlgItem(IDC_QUANTITY))->SetReadOnly(TRUE);

			try {
				_RecordsetPtr rs = CreateRecordset("SELECT Name, HasSerialNum, HasExpDate FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID WHERE ServiceT.ID = %li", m_ProductID);
				if (!rs->eof) {
					CString strName = AdoFldString(rs, "Name", "this product");
					CString msg;
					msg.Format("Please select an item for '%s'.", strName);
					GetDlgItem(IDC_SELECT_ITEM_LABEL)->SetWindowText(msg);

					//these booleans don't get used again except for SetColumnWidths, which is exactly why we need to load them
					m_bUseSerial = AdoFldBool(rs, "HasSerialNum", FALSE);
					m_bUseExpDate = AdoFldBool(rs, "HasExpDate", FALSE);
				}
				rs->Close();

				if (!m_bUseSerial) {
					//even though the serial number may not be marked, we have to see if there ARE serial numbers to use!
					// (j.jones 2007-11-26 09:21) - PLID 28037 - supported allocated items

					// (j.jones 2008-06-06 09:14) - PLID 27110 - added PI_RETURN_DATA, which in this case should not filter out ChargedProductItemsT
					if (m_EntryType == PI_RETURN_DATA && !m_strWhere.IsEmpty()) {

						//if a return, see if any of the items we are returning have serial numbers
						if (!IsRecordsetEmpty("SELECT ID FROM ProductItemsT "
							"WHERE SerialNum Is Not Null "
							"AND %s "
							"AND Deleted = 0 AND ProductID = %li",
							m_strWhere, m_ProductID)) {

							m_bUseSerial = TRUE;
						}
					}
					else if (m_EntryType != PI_RETURN_DATA) {

						if (!IsRecordsetEmpty("SELECT ID FROM ProductItemsT "
							"WHERE SerialNum Is Not Null "
							"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
							"%s "
							"AND Deleted = 0 AND ProductID = %li",
							GetAllocationWhereClause(), m_ProductID)) {

							m_bUseSerial = TRUE;
						}
					}
				}

				if (!m_bUseExpDate) {
					//even though the exp. date may not be marked, we have to see if there ARE dates to use!
					// (j.jones 2007-11-26 09:21) - PLID 28037 - supported allocated items
					if (!IsRecordsetEmpty("SELECT ID FROM ProductItemsT "
						"WHERE ExpDate Is Not Null "
						"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
						"%s "
						"AND Deleted = 0 AND ProductID = %li",
						GetAllocationWhereClause(), m_ProductID)) {

						m_bUseExpDate = TRUE;
					}
				}

				// (c.haag 2007-11-14 09:02) - PLID 28025 - Final override for the select item label
				if (!m_strOverrideSelectItemText.IsEmpty()) {
					SetDlgItemText(IDC_SELECT_ITEM_LABEL, m_strOverrideSelectItemText);
				}

			}NxCatchAll("Error loading product name.");
		}
		//DRT 11/6/2007 - PLID 27999 - New generation type acts the same as entry type
		else if (m_EntryType == PI_ENTER_DATA) {

			// (j.jones 2007-11-08 11:06) - PLID 28041 - supported an optional override for the title bar text
			if (!m_strOverrideTitleBarText.IsEmpty()) {
				SetWindowText(m_strOverrideTitleBarText);
			}
			else {

				CString str;
				if (m_nLocationID > -1) {
					CString strLocName;
					_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", m_nLocationID);
					if (!rs->eof) {
						strLocName = AdoFldString(rs, "Name", "");
					}
					rs->Close();

					str.Format("Enter Item Information (%s)", strLocName);
				}
				else {
					str.Format("Enter Item Information");
				}

				SetWindowText(str);
			}

			_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM ServiceT INNER JOIN ProductT ON "
				"ServiceT.ID = ProductT.ID WHERE ProductT.ID = {INT}", m_ProductID);

			LONG nVerticalAdjustment = 0;

			if (!rs->eof) {
				CString strName = AdoFldString(rs, "Name", "this product");
				CString msg;
				msg.Format("Please enter the data for %s.", strName);

				CWnd* pItmLbl = GetDlgItem(IDC_SELECT_ITEM_LABEL);
				pItmLbl->SetWindowText(msg); // Set the text

				CRect pItemLblRect;
				pItmLbl->GetWindowRect(pItemLblRect);
				ScreenToClient(&pItemLblRect);

				// Calculate the size needed to fit the text in IDC_SELECT_ITEM_LABEL
				CClientDC dc(pItmLbl);
				CFont* pOldFont = dc.SelectObject(pItmLbl->GetFont());
				CSize SizeNeeded = dc.GetTextExtent(msg);
				dc.SelectObject(pOldFont);

				// We will use this to get the available width
				CWnd* pBkgnd = GetDlgItem(IDC_BACKG);
				CRect pBkgndRect;
				pBkgnd->GetWindowRect(pBkgndRect);
				ScreenToClient(&pBkgndRect);

				LONG nAvailableWidth = pBkgndRect.Width();
				LONG nLinesNeeded = SizeNeeded.cx / nAvailableWidth + (SizeNeeded.cx % nAvailableWidth != 0);// Given the name limit of 255 characters, this will never be more than 3 lines

				if ((nAvailableWidth - pItemLblRect.left) < SizeNeeded.cx)
				{
					// Text doesn't fit horizontally, we should move the label further to the left so we have enough room
					pItemLblRect.left = pBkgndRect.left + 10;
					pItemLblRect.right = nAvailableWidth;
					if (nLinesNeeded > 1)
					{// Only adjust vertically if more than 1 line is needed
						nVerticalAdjustment = (SizeNeeded.cy * nLinesNeeded) - pItemLblRect.Height();
						pItemLblRect.bottom = pItemLblRect.top + (SizeNeeded.cy * nLinesNeeded);
					}

					pItmLbl->MoveWindow(&pItemLblRect);
				}
			}
			rs->Close();

			((CNxEdit*)GetDlgItem(IDC_QUANTITY))->SetReadOnly(TRUE);

			// (j.jones 2010-05-07 09:05) - PLID 36454 - SerialNumIsLotNum is now a variant
			if (VarBool(m_varSerialNumIsLotNum, FALSE)) {
				//TES 7/3/2008 - PLID 24726 - stretch the list to reach the bottom of the screen but below the lot num
				GetDlgItem(IDC_ITEM_LIST)->SetWindowPos(NULL, rcItems.left, rcLotNumLabel.bottom + 5 + nVerticalAdjustment, rcItems.Width(), (rcSelected.bottom - (rcLotNumLabel.bottom + 5)) - nVerticalAdjustment, SWP_NOZORDER);
			}
			else {
				if (m_bUseExpDate)
					//stretch the list to reach the bottom of the screen but below the exp. date
					GetDlgItem(IDC_ITEM_LIST)->SetWindowPos(NULL, rcItems.left, rcExpDateLabel.bottom + 5 + nVerticalAdjustment, rcItems.Width(), (rcSelected.bottom - (rcExpDateLabel.bottom + 5)) - nVerticalAdjustment, SWP_NOZORDER);
				else
					//stretch the list to reach the bottom of the screen and to the top of the exp. date
					GetDlgItem(IDC_ITEM_LIST)->SetWindowPos(NULL, rcItems.left, rcItems.top + nVerticalAdjustment, rcItems.Width(), (rcSelected.bottom - rcItems.top) - nVerticalAdjustment, SWP_NOZORDER);
			}

			//now hide the selected list and related items
			GetDlgItem(IDC_BTN_SELECT_PRODUCT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_UNSELECT_PRODUCT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SELECTED_ITEM_LIST)->ShowWindow(SW_HIDE);
			// (d.thompson 2009-08-17) - PLID 18899
			GetDlgItem(IDC_PRODUCT_SELECTED_COUNT)->ShowWindow(SW_HIDE);

			// This is a list of controls that might be affected by resizing IDC_SELECT_ITEM_LABEL vertically
			const UINT adjustedControls[] =
			{
				IDC_QUANTITY_LABEL,
				IDC_QUANTITY,
				IDC_EXP_DATE_LABEL,
				IDC_EXPDATE,
				IDC_BTN_APPLY_EXP_DATE,
				IDC_BTN_APPLY_DATE_TO_SELECTED,
				IDC_LOT_NUM_LABEL,
				IDC_LOT_NUMBER,
				IDC_BTN_APPLY_LOT_NUM,
				IDC_BTN_APPLY_LOT_NUM_TO_SELECTED,
				0
			};

			// Adjust vertical position of each of the affected controls
			for (const UINT* pControl = adjustedControls; *pControl; ++pControl)
			{
				CWnd* pWndControl = GetDlgItem(*pControl);
				CRect pControlRect;
				pWndControl->GetWindowRect(pControlRect);
				ScreenToClient(&pControlRect);
				pControlRect.top = pControlRect.top + nVerticalAdjustment;
				pControlRect.bottom = pControlRect.bottom + nVerticalAdjustment;
				pWndControl->MoveWindow(pControlRect, true);
			}
		}
		else if (m_EntryType == PI_READ_DATA) {

			// (j.jones 2007-11-08 11:06) - PLID 28041 - supported an optional override for the title bar text
			if (!m_strOverrideTitleBarText.IsEmpty()) {
				SetWindowText(m_strOverrideTitleBarText);
			}
			else {

				CString str;
				if (m_nLocationID > -1) {
					CString strLocName;
					_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", m_nLocationID);
					if (!rs->eof) {
						strLocName = AdoFldString(rs, "Name", "");
					}
					rs->Close();

					str.Format("Item Information (%s)", strLocName);
				}
				else {
					str.Format("Item Information");
				}

				SetWindowText(str);
			}

			// (j.jones 2007-11-09 09:53) - PLID 28030 - added the Consignment column
			// (c.haag 2007-12-03 11:45) - PLID 28204 - Now a status column
			m_List->GetColumn(COLUMN_STATUS)->PutEditable(VARIANT_FALSE);

			GetDlgItem(IDC_SELECT_ITEM_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUANTITY_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_QUANTITY)->ShowWindow(SW_HIDE);

			//stretch the list to fill the screen
			GetDlgItem(IDC_ITEM_LIST)->SetWindowPos(NULL, rcItems.left, rcTopLabel.top, rcItems.Width(), (rcSelected.bottom - rcTopLabel.top), SWP_NOZORDER);

			//now hide the selected list and related items
			GetDlgItem(IDC_BTN_SELECT_PRODUCT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_UNSELECT_PRODUCT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SELECTED_ITEM_LIST)->ShowWindow(SW_HIDE);
			// (d.thompson 2009-08-17) - PLID 18899
			GetDlgItem(IDC_PRODUCT_SELECTED_COUNT)->ShowWindow(SW_HIDE);

			if (!m_bUseSerial) {
				//even though the serial number may not be marked, we have to see if there ARE serial numbers to use!
				// (j.jones 2007-11-26 09:21) - PLID 28037 - supported allocated items
				if (!IsRecordsetEmpty("SELECT ID FROM ProductItemsT "
					"WHERE SerialNum Is Not Null "
					"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
					"%s "
					"AND Deleted = 0 AND ProductID = %li",
					GetAllocationWhereClause(), m_ProductID)) {

					m_bUseSerial = TRUE;
				}
			}

			if (!m_bUseExpDate) {
				//even though the exp. date may not be marked, we have to see if there ARE dates to use!
				// (j.jones 2007-11-26 09:21) - PLID 28037 - supported allocated items
				if (!IsRecordsetEmpty("SELECT ID FROM ProductItemsT "
					"WHERE ExpDate Is Not Null "
					"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
					"%s "
					"AND Deleted = 0 AND ProductID = %li",
					GetAllocationWhereClause(), m_ProductID)) {

					m_bUseSerial = TRUE;
				}
			}
		}
		// (c.haag 2007-12-17 13:40) - PLID 28286 - Final override for disabling status editing
		if (m_bDisallowStatusChange) {
			m_List->GetColumn(COLUMN_STATUS)->PutEditable(VARIANT_FALSE);
		}
		// (c.haag 2008-06-25 11:20) - PLID 28438 - Toggle the location column read-only status.
		// This overrides any previous setting.
		if (m_bDisallowLocationChange) {
			m_List->GetColumn(COLUMN_LOCATION_ID)->PutEditable(VARIANT_FALSE);
		}
	} NxCatchAll("Error in CProductItemsDlg:ShowRelevantFields");
}

void CProductItemsDlg::ConfigureDisplay() {

	try { // (b.eyers 2015-05-20) - PLID 28048
		ShowRelevantFields();

		SetColumnWidths();
	} NxCatchAll("Error in CProductItemsDlg:ConfigureDisplay");
}

void CProductItemsDlg::OnKillfocusQuantity()
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		if (GetDlgItemInt(IDC_QUANTITY) == 0)
			SetDlgItemInt(IDC_QUANTITY, 1);

		m_CountOfItemsNeeded = GetDlgItemInt(IDC_QUANTITY);
	} NxCatchAll("Error in CProductItemsDlg:OnKillfocusQuantity");
}


void CProductItemsDlg::PreLoadValues() {

	try { // (b.eyers 2015-05-20) - PLID 28048

		  //at one shot, we propagate the selected list and delete the items in the list,
		  //but only if we have values for the selected list
		CString strWhere;
		for (int i = 0; i < m_adwProductItemIDs.GetSize(); i++) {
			CString str;
			str.Format("ProductItemsT.ID = %li OR ", (long)m_adwProductItemIDs.GetAt(i));
			strWhere += str;
		}
		m_adwProductItemIDs.RemoveAll();
		strWhere.TrimRight("OR ");

		if (strWhere != "") {

			// (j.jones 2007-11-09 09:32) - PLID 28030 - added Consignment
			// (c.haag 2007-12-03 11:46) - PLID 28204 - Changed Consignment to Status
			// (j.jones 2008-02-29 17:11) - PLID 29125 - added column for whether the product is in our
			// allocation specified by m_nLinkedAllocationID

			CString strLinkedAllocationCheck = "0";
			if (m_nLinkedAllocationID != -1) {
				strLinkedAllocationCheck.Format("CASE WHEN ProductItemsT.ID IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
					"	WHERE Status = %li AND AllocationID = %li) THEN 1 ELSE 0 END",
					InvUtils::iadsUsed, m_nLinkedAllocationID);
			}

			_RecordsetPtr rs = CreateRecordset("SELECT ProductItemsT.ID, "
				"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
				"ProductItemsT.Status, ProductItemsT.LocationID, "
				"Convert(bit, %s) AS IsInLinkedAllocation "
				"FROM ProductItemsT WHERE (%s) AND Deleted = 0", strLinkedAllocationCheck, strWhere);

			while (!rs->eof) {

				// (j.jones 2009-01-16 11:01) - PLID 32749 - remove the row from the unselected list if it's there
				long nOldRow = m_List->FindByColumn(COLUMN_INDEX, rs->Fields->Item["ID"]->Value, 0, FALSE);
				if (nOldRow != -1) {
					m_List->RemoveRow(nOldRow);
				}

				IRowSettingsPtr pRow;
				pRow = m_SelectedList->GetRow(-1);

				//ID
				pRow->PutValue(COLUMN_INDEX, rs->Fields->Item["ID"]->Value);

				//Serial Number
				pRow->PutValue(COLUMN_SERIAL_NUMBER, _bstr_t(AdoFldString(rs, "SerialNum", NO_DATA_TEXT)));

				//Exp. Date
				_variant_t var = rs->Fields->Item["ExpDate"]->Value;
				CString strDate = NO_DATA_TEXT;
				if (var.vt == VT_DATE) {
					strDate = FormatDateTimeForInterface(VarDateTime(var), NULL, dtoDate);
				}
				pRow->PutValue(COLUMN_EXP_DATE, _bstr_t(strDate));

				// (j.jones 2007-11-09 09:28) - PLID 28030 - added Consignment column
				// (c.haag 2007-12-03 11:45) - PLID 28204 - Now a more general Status column
				//_variant_t varConsign = rs->Fields->Item["Consignment"]->Value;
				pRow->PutValue(COLUMN_STATUS, rs->Fields->Item["Status"]->Value);

				long nLocID = AdoFldLong(rs, "LocationID", -1);

				pRow->PutValue(COLUMN_LOCATION_ID, nLocID);

				// (j.jones 2008-02-29 12:27) - PLID 29125 - grab the IsInLinkedAllocation field
				BOOL bIsLinkedAllocation = AdoFldBool(rs, "IsInLinkedAllocation");
				pRow->PutValue(COLUMN_LINKED_ALLOCATION, rs->Fields->Item["IsInLinkedAllocation"]->Value);

				if (nLocID == -1 && GetRemotePropertyInt("GrayOutLocationlessItems", 0, 0, "<None>", TRUE) == 1) {
					pRow->PutForeColor(RGB(96, 96, 96));
				}

				m_SelectedList->AddRow(pRow);

				rs->MoveNext();
			}
			rs->Close();
		}
	} NxCatchAll("Error in CProductItemsDlg:PreLoadValues");
}

void CProductItemsDlg::OnEditingFinishingItemList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue)
{
	try {

		if (nCol == COLUMN_SERIAL_NUMBER && m_EntryType == PI_READ_DATA) {
			CString serNum;
			if (pvarNewValue->vt == VT_BSTR) {
				serNum = CString(pvarNewValue->bstrVal);
				serNum.TrimLeft();
				serNum.TrimRight();
				if (serNum.GetLength() == 0)
					serNum = "NULL";
				else
					serNum = "'" + _Q(serNum) + "'";
			}
			else
				serNum = "NULL";

			if (serNum == "NULL" && IDNO == MessageBox("You are removing the serial number from this product. Are you sure you wish to do this?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
				*pbCommit = FALSE;
			}
		}

		if (nCol == COLUMN_SERIAL_NUMBER) {
			//check for duplicates
			CString serNum;
			if (pvarNewValue->vt == VT_BSTR) {
				serNum = CString(pvarNewValue->bstrVal);
				serNum.TrimLeft();
				serNum.TrimRight();
				if (serNum.GetLength() > 0) {

					// Check whether the old and new values are matching serial numbers. If they are, then
					// stop processing this action.
					BOOL bMatchesOldValue = FALSE;
					if (varOldValue.vt == VT_BSTR) {
						CString strOld = VarString(varOldValue);
						strOld.TrimLeft();
						strOld.TrimRight();
						if (strOld == serNum) {
							*pbCommit = FALSE;
							return;
						}
					}

					//TES 7/3/2008 - PLID 24726 - If we're treating serial numbers as lot numbers, then don't check for 
					// duplicates
					// (j.jones 2010-05-07 09:05) - PLID 36454 - SerialNumIsLotNum is now a variant
					if (!VarBool(m_varSerialNumIsLotNum, FALSE)) {
						// (c.haag 2008-03-20 13:26) - PLID 29335 - Search for the item in an existing row					
						long row = m_List->FindByColumn(COLUMN_SERIAL_NUMBER, _bstr_t(serNum), 0, FALSE);
						if ((row != nRow && row != -1) || (m_List->GetRowCount() - 1 > nRow && m_List->FindByColumn(COLUMN_SERIAL_NUMBER, _bstr_t(serNum), nRow + 1, FALSE) != -1 && m_List->FindByColumn(COLUMN_SERIAL_NUMBER, _bstr_t(serNum), nRow + 1, FALSE) != nRow)) {
							// If we get here, it was in the list
							AfxMessageBox("This serial number already exists in the list.", MB_ICONERROR | MB_OK);
							*pbCommit = FALSE;
						}
						else {
							// It's not in the list; check the data.
							if (InvUtils::IsProductItemSerialNumberInUse(m_ProductID, serNum)) {
								// The serial is in data for this product
								*pbCommit = FALSE;

							}
							else {
								// The serial is not in data for this product
							}
						}
					}

					// (c.haag 2008-03-18 17:26) - PLID 29306 - If we're entering serial numbers, we should
					// check to see whether the code exists as a product barcode. If it does, then
					// Barcode_CheckExistenceOfProductCode will warn the user about it.
					// (c.haag 2008-03-20 13:29) - PLID 29335 - The check for duplicate serials will always
					// precede this warning
					if (FALSE != *pbCommit) {
						InvUtils::Barcode_CheckExistenceOfProductCode(serNum, FALSE);
					}
				}
			}
		}

		if (nCol == COLUMN_EXP_DATE) {
			if (pvarNewValue->vt == VT_DATE) {
				COleDateTime dt = pvarNewValue->date;
				if (dt.m_status == COleDateTime::invalid || dt.m_dt == 0.00 || dt.GetYear() < 1900) {
					*pbCommit = FALSE;
				}
			}
			else if (pvarNewValue->vt == VT_BSTR) {
				COleDateTime dt;
				if (!dt.ParseDateTime(CString(pvarNewValue->bstrVal)) || dt.m_status == COleDateTime::invalid
					|| dt.m_dt == 0.00 || dt.GetYear() < 1900) {
					*pbCommit = FALSE;
				}
			}
			else {
				*pbCommit = FALSE;
			}
		}

		if (m_EntryType == PI_READ_DATA && nCol == COLUMN_LOCATION_ID) {

			long nNewLocationID = VarLong(*pvarNewValue, -1);
			long nOldLocationID = VarLong(varOldValue, -1);

			//before saving, warn them if something is amiss (it likely will be)
			if (!VerifyLocationChange(nOldLocationID, nNewLocationID)) {
				*pbCommit = FALSE;
			}
		}

	}NxCatchAll("Error editing values.");
}

void CProductItemsDlg::OnEditingFinishedItemList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	if (!bCommit)
		return;

	if (m_EntryType == PI_READ_DATA) {
		//only when they are reading (and editing) existing data will we save when they are done editing

		try {

			if (nCol == COLUMN_SERIAL_NUMBER) {
				CString serNum;
				if (varNewValue.vt == VT_BSTR) {
					serNum = CString(varNewValue.bstrVal);
					serNum.TrimLeft();
					serNum.TrimRight();
					if (serNum.GetLength() == 0 || serNum == NO_DATA_TEXT)
						serNum = "NULL";
					else
						serNum = "'" + _Q(serNum) + "'";
				}
				else
					serNum = "NULL";

				//update
				ExecuteSql("UPDATE ProductItemsT SET SerialNum = %s WHERE ID = %li", serNum, m_List->GetValue(nRow, COLUMN_INDEX).lVal);

				// Now that data has been written, the user may not cancel the dialog (which is safe because we're under an entry type of PI_READ_DATA)
				GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
			}
			else if (nCol == COLUMN_EXP_DATE) {
				CString strDate;
				if (varNewValue.vt == VT_DATE) {
					strDate = "'" + _Q(FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate)) + "'";
				}
				else if (varNewValue.vt == VT_BSTR) {
					COleDateTime dt;
					if (!dt.ParseDateTime(CString(varNewValue.bstrVal)) || dt.m_status == COleDateTime::invalid
						|| dt.m_dt == 0.00 || dt.GetYear() < 1900) {
						strDate = "NULL";
					}
					else {
						strDate = "'" + _Q(FormatDateTimeForSql(dt, dtoDate)) + "'";
					}
				}
				else {
					strDate = "NULL";
				}

				//update
				ExecuteSql("UPDATE ProductItemsT SET ExpDate = %s WHERE ID = %li", strDate, VarLong(m_List->GetValue(nRow, COLUMN_INDEX)));

				// Now that data has been written, the user may not cancel the dialog (which is safe because we're under an entry type of PI_READ_DATA)
				GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
			}
			else if (nCol == COLUMN_LOCATION_ID) {

				long nLocationID = VarLong(varNewValue, -1);

				CString strLocationID = "NULL";

				if (nLocationID != -1)
					strLocationID.Format("%li", nLocationID);

				if (nLocationID == -1 && GetRemotePropertyInt("GrayOutLocationlessItems", 0, 0, "<None>", TRUE) == 1) {
					IRowSettingsPtr(m_List->GetRow(nRow))->PutForeColor(RGB(96, 96, 96));
				}
				else {
					IRowSettingsPtr(m_List->GetRow(nRow))->PutForeColor(RGB(0, 0, 0));
				}

				//update
				ExecuteSql("UPDATE ProductItemsT SET LocationID = %s WHERE ID = %li", strLocationID, VarLong(m_List->GetValue(nRow, COLUMN_INDEX)));

				// Now that data has been written, the user may not cancel the dialog (which is safe because we're under an entry type of PI_READ_DATA)
				GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
			}

		}NxCatchAll("Error saving changes to the product information.");
	}
}

LRESULT CProductItemsDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {
		// (c.haag 2007-12-06 09:47) - PLID 28286 - Extra safeguard
		if (!IsWindowVisible()) {
			return 0;
		}

		bool bNoAvailRows = false;

		// (a.walling 2007-11-12 11:03) - PLID 27476 - We need to correctly cast the BSTR lParam
		_bstr_t bstr = (BSTR)lParam;

		// (c.haag 2003-08-01 17:59) - Check if the item already exists
		long nRow = m_List->FindByColumn(COLUMN_SERIAL_NUMBER, (LPCTSTR)bstr, 0, TRUE);
		if (nRow == -1)
		{
			// (j.jones 2007-12-18 10:29) - PLID 28037 - if they are selecting data,
			// but scanned an unavailable serial number, see if it is used elsewhere
			// (j.jones 2008-06-06 09:25) - PLID 27110 - do not search if it is a return,
			// only when selecting
			if (m_EntryType == PI_SELECT_DATA) {

				//before we check for the serial number, make sure it's just not already selected

				//TES 7/7/2008 - PLID 24726 - Actually what we'll do here is, gather all the selected product items, and pass
				// them into Barcode_CheckExistenceOfSerialNumber(), and let it filter them out.
				CArray<long, long> arSelectedIDs;
				for (long nSelRow = 0; nSelRow < m_SelectedList->GetRowCount(); nSelRow++) {
					arSelectedIDs.Add(VarLong(m_SelectedList->GetValue(nSelRow, COLUMN_INDEX)));
				}

				long nProductItemID = -1;
				long nProductID = -1;
				_variant_t varExpDate = g_cvarNull;
				CString strProductName = "";
				//Barcode_CheckExistenceOfSerialNumber will tell them why they can't use the serial number,
				//if it is a legitimate serial number in the system
				if (InvUtils::Barcode_CheckExistenceOfSerialNumber(AsString(bstr), m_nLocationID, FALSE, nProductItemID, nProductID, strProductName, varExpDate, NULL, FALSE, FALSE, TRUE, TRUE, &arSelectedIDs)) {
					//hmm, if it returned true, then it should be available, so warn accordingly if the product ID matches
					if (m_ProductID == nProductID) {
						AfxMessageBox("The serial number you entered exists for this product, but is not currently in the list.\n"
							"(It may have just become available for use.)\n\n"
							"Please close and reopen this selection screen if you wish to use this serial number.");
					}
				}

				//Regardless of it being selected already, in use by a patient,
				//or just not available on this screen, we can't use it, so
				//leave this function, do not close the dialog.
				return 0;
			}
			else {
				// (j.jones 2010-05-07 09:05) - PLID 36454 - SerialNumIsLotNum is now a variant
				if (VarBool(m_varSerialNumIsLotNum, FALSE)) {
					//TES 7/3/2008 - PLID 24726 - If we're treating serial numbers as lot numbers, then just put the number
					// up here, so they can easily apply it to all.
					SetDlgItemText(IDC_LOT_NUMBER, (LPCTSTR)bstr);
					// (c.haag 2008-03-18 17:26) - PLID 29306 - If we're entering serial numbers, we should
					// check to see whether the code exists as a product barcode. If it does, then
					// Barcode_CheckExistenceOfProductCode will warn the user about it.
					InvUtils::Barcode_CheckExistenceOfProductCode((LPCTSTR)bstr, TRUE);
				}
				else {
					// (c.haag 2008-03-20 13:09) - PLID 29335 - Don't let the user add this serial if it's
					// already in use in data for this product.
					if (InvUtils::IsProductItemSerialNumberInUse(m_ProductID, (LPCTSTR)bstr)) {
						return 0;
					}

					// (c.haag 2008-03-19 10:27) - PLID 29306 - True if form data had changed
					BOOL bCheckForProductCode = FALSE;


					// (c.haag 2003-08-01 17:59) - Nope, add it if there's an available slot.
					nRow = m_List->FindByColumn(COLUMN_SERIAL_NUMBER, NEW_SERIAL_NUM_TEXT, 0, FALSE);
					if (nRow != -1) {
						bCheckForProductCode = TRUE;
					}
					else
					{
						nRow = m_List->FindByColumn(COLUMN_SERIAL_NUMBER, "", 0, FALSE);
						if (nRow != -1) {
							bCheckForProductCode = TRUE;
						}
						else {
							bNoAvailRows = true;
						}
					}

					// (c.haag 2008-03-18 17:26) - PLID 29306 - If we're entering serial numbers, we should
					// check to see whether the code exists as a product barcode. If it does, then
					// Barcode_CheckExistenceOfProductCode will warn the user about it.
					if (bCheckForProductCode) {

						// (z.manning 2008-07-07 11:23) - PLID 30602 - If the barcode does exist then see
						// if they want to clear it out.
						CString strProduct;
						if (InvUtils::Barcode_CheckExistenceOfProductCode((LPCTSTR)bstr, TRUE, strProduct, TRUE)) {
							CString strMessage = FormatString(
								"You have scanned the serial number '%s'; but the serial number also "
								"matches the barcode for the product '%s'.\n\n"
								"Would you like to rescan this serial number?",
								(LPCTSTR)bstr, strProduct);
							if (MessageBox(strMessage, NULL, MB_YESNO) == IDYES) {
								return 0;
							}
						}

						// (c.haag 2008-06-05 16:00) - PLID 28311 - Also assign the inventory type.
						// We only support radio buttons when the entry type is PI_ENTER_DATA
						if (PI_ENTER_DATA == m_EntryType) {
							if (((CButton*)GetDlgItem(IDC_RADIO_SCAN_GENERAL))->GetCheck()) {
								m_List->PutValue(nRow, COLUMN_STATUS, (long)InvUtils::pisPurchased);
							}
							else if (((CButton*)GetDlgItem(IDC_RADIO_SCAN_CONSIGNMENT))->GetCheck()) {
								m_List->PutValue(nRow, COLUMN_STATUS, (long)InvUtils::pisConsignment);
							}
						}
						// (j.jones 2010-10-22 09:09) - PLID 29342 - if we are reading data, we
						// can manually change serial numbers, previously doing so via a barcode
						// scan didn't actually save
						else if (PI_READ_DATA == m_EntryType) {
							_variant_t varSerialNumber = g_cvarNull;
							CString serNum = (LPCTSTR)bstr;
							serNum.TrimLeft();
							serNum.TrimRight();
							if (serNum.GetLength() != 0 && serNum != NO_DATA_TEXT) {
								varSerialNumber = (LPCTSTR)serNum;
							}

							//update
							ExecuteParamSql("UPDATE ProductItemsT SET SerialNum = {VT_BSTR} WHERE ID = {INT}", varSerialNumber, VarLong(m_List->GetValue(nRow, COLUMN_INDEX)));

							// Now that data has been written, the user may not cancel the dialog (which is safe because we're under an entry type of PI_READ_DATA)
							GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
						}

						m_List->PutValue(nRow, COLUMN_SERIAL_NUMBER, (LPCTSTR)bstr);
					}
				}

			}
		}
		else {
			//DRT 11/20/2007 - PLID 28142 - Actually select this row to the bottom if we're in the select type
			// (j.jones 2008-06-06 09:14) - PLID 27110 - added PI_RETURN_DATA, which is for the most part the same as PI_SELECT_DATA
			if (m_EntryType == PI_SELECT_DATA || m_EntryType == PI_RETURN_DATA) {
				//The row is highlighted by our FindByColumn above, just fake a click on the button
				OnBtnSelectProduct();
			}
		}

		//DRT 11/20/2007 - PLID 28138 - If they want to close the dialog after a scan, do so now.
		//DRT 12/14/2007 - PLID 28138 - Don't do this at all if we're on the UU types.  They require
		//	a specific portion of codes entered.  Don't want to keep warning them.
		if (m_bCloseAfterScan && !(!m_bDisallowQtyChange && m_bUseUU && !m_bSerialPerUO)) {
			//DRT 11/20/2007 - PLID 28138 - If they scan, and there's no room for something new, then we must warn them.
			if (bNoAvailRows) {
				AfxMessageBox("You have scanned a barcode, but there are no available slots to enter the new serial number.  Please re-check your product count.");
			}
			else {
				OnOK();
			}
		}
	} NxCatchAll("Error in OnBarcodeScan");

	return 0;
}

void CProductItemsDlg::OnClose()
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		OnOK();
	} NxCatchAll("Error in CProductItemsDlg:OnClose");
}

BOOL CProductItemsDlg::VerifyLocationChange(long nOldLocationID, long nNewLocationID) {

	if (nOldLocationID == nNewLocationID)
		return TRUE;

	double dblQuantityOldLoc = 0.0, dblQuantityNewLoc = 0.0;
	double dblAllocatedOldLoc = 0.0, dblAllocatedNewLoc = 0.0;

	CString strWarnings = "";

	CString strWhere;
	// (j.jones 2007-11-26 09:27) - PLID 28037 - ensured we hide allocated items
	strWhere.Format("ProductItemsT.ProductID = %li AND ChargesT.ID Is Null AND CaseHistoryDetailsT.ID Is Null "
		"AND ProductItemsT.Deleted = 0 "
		"%s "
		, m_ProductID, GetAllocationWhereClause());
	if (m_strWhere != "")
		strWhere = strWhere + " AND " + m_strWhere;

	//warn about changes from the old location

	// (j.jones 2007-12-18 11:40) - PLID 28037 - CalcAmtOnHand changed to return allocation information
	if (nOldLocationID != -1 && InvUtils::CalcAmtOnHand(m_ProductID, nOldLocationID, dblQuantityOldLoc, dblAllocatedOldLoc)) {

		//dblQuantityOldLoc now stores how many items are in stock for the location we're switching from

		// (j.jones 2007-11-26 09:21) - PLID 28037 - supported allocated items
		_RecordsetPtr rs = CreateRecordset("SELECT Count(ProductItemsT.ID) AS CountProducts FROM ProductItemsT "
			"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"LEFT JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
			"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
			"WHERE %s AND ProductItemsT.LocationID = %li", strWhere, nOldLocationID);

		if (!rs->eof) {
			long nCountProducts = AdoFldLong(rs, "CountProducts", 0);
			//account for the change we are about to make
			nCountProducts--;

			if ((double)nCountProducts != dblQuantityOldLoc) {

				_RecordsetPtr rsLoc = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li", nOldLocationID);
				if (!rsLoc->eof) {

					CString strLocName = AdoFldString(rsLoc, "Name", "");

					CString strAllocated = "";
					if (dblAllocatedOldLoc > 0.0) {
						strAllocated.Format(" (%g allocated to patients)", dblAllocatedOldLoc);
					}

					CString strWarn;
					strWarn.Format("- You currently have %g in stock%s at %s, but with this change you \n"
						"  will have %li serial numbered / expiration date items associated with %s.\n\n",
						dblQuantityOldLoc, strAllocated, strLocName, nCountProducts, strLocName);

					strWarnings += strWarn;
				}
				rsLoc->Close();
			}
		}
		rs->Close();
	}

	//warn about changes to the new location

	// (j.jones 2007-12-18 11:40) - PLID 28037 - CalcAmtOnHand changed to return allocation information
	if (nNewLocationID != -1 && InvUtils::CalcAmtOnHand(m_ProductID, nNewLocationID, dblQuantityNewLoc, dblAllocatedNewLoc)) {

		//dblQuantityNewLoc now stores how many items are in stock for the location we're switching to

		_RecordsetPtr rs = CreateRecordset("SELECT Count(ProductItemsT.ID) AS CountProducts FROM ProductItemsT "
			"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"LEFT JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
			"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
			"WHERE %s AND ProductItemsT.LocationID = %li", strWhere, nNewLocationID);

		if (!rs->eof) {
			long nCountProducts = AdoFldLong(rs, "CountProducts", 0);
			//account for the change we are about to make
			nCountProducts++;

			if ((double)nCountProducts != dblQuantityNewLoc) {

				_RecordsetPtr rsLoc = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li", nNewLocationID);
				if (!rsLoc->eof) {

					CString strLocName = AdoFldString(rsLoc, "Name", "");

					CString strAllocated = "";
					if (dblAllocatedNewLoc > 0.0) {
						strAllocated.Format(" (%g allocated to patients)", dblAllocatedNewLoc);
					}

					CString strWarn;
					strWarn.Format("- You currently have %g in stock%s at %s, but with this change you \n"
						"  will have %li serial numbered / expiration date items associated with %s.\n\n",
						dblQuantityNewLoc, strAllocated, strLocName, nCountProducts, strLocName);

					strWarnings += strWarn;
				}
				rsLoc->Close();
			}
		}
		rs->Close();
	}

	if (strWarnings.IsEmpty()) {
		return TRUE;
	}

	strWarnings = "The location assignment for this product has triggered the following warnings:\n\n" + strWarnings +
		"When billing this product, you will only be able to choose serial numbered / expiration date items\n"
		"that are associated with the location of the bill, and items unassociated with any location.\n\n"
		"Are you sure you wish to make this change?";

	if (IDYES == MessageBox(strWarnings, "Practice", MB_YESNO | MB_ICONQUESTION)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// (c.haag 2007-12-07 15:05) - PLID 28286 - Added a silent flag. At the time of this implementation,
// this can only happen if a user has entered an order for items already received...in which case
// the verification has already happened once before.
BOOL CProductItemsDlg::VerifyLocationSave(BOOL bSilent)
{
	// (j.jones 2005-02-21 17:10) - keep a count of how many items we have set to be per location,
	// then match them up with the amounts on hand per location. If we are going to save with a mismatch,
	// warn the user first.

	if (m_NewItemCount != -2) {
		//NewItemCount will be a real number if this is from an order or from an adjustment.
		//if so, just compare the amount we are adding to a given location
		//and complain if we changed any of the products to go to a different location
		long nQtyRightLoc = 0;
		long nQtyOtherLoc = 0;

		for (int i = 0; i<m_List->GetRowCount(); i++) {
			long nLocID = VarLong(m_List->GetValue(i, COLUMN_LOCATION_ID), -1);
			if (nLocID == m_nLocationID)
				nQtyRightLoc++;
			else if (nLocID != -1)
				nQtyOtherLoc++;
		}

		CString str, strLocName;
		_RecordsetPtr rs = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li", m_nLocationID);
		if (!rs->eof) {
			strLocName = AdoFldString(rs, "Name", "");
		}
		rs->Close();

		CString strWarning = "";

		//Warn if the amount of items configured to go into the location match the amount being added,
		//unless we default items to no location. If we do have that default, and "some" of the items are set
		//to go to our location, well, warn that too, because that's plain weird.
		// (c.haag 2008-07-01 17:37) - PLID 30594 - "Default Serialized Items To No Location" has been deprecated. Because
		// we always want to act like the preference value is 0, the logic simplifies to
		// "0 != 1 || nQtyRightLoc > 0" = "1 || nQtyRightLoc > 0" = "1" = always true
		if (nQtyRightLoc < m_NewItemCount/* && (GetRemotePropertyInt("DefaultSerializedItemsToNoLocation",0,0,"<None>",TRUE) != 1
										 || nQtyRightLoc > 0)*/) {

			CString str;
			str.Format("- You are adding %li items at %s but have only configured %li to be assigned to this location.\n\n",
				m_NewItemCount, strLocName, nQtyRightLoc);

			strWarning += str;
		}

		//in all cases, warn if any items are in a location other than the order location ("no location" items excluded)
		if (nQtyOtherLoc > 0) {

			CString str;

			if (strWarning.IsEmpty()) {
				str.Format("- You are adding %li items at %s but have configured %li to be assigned to other locations.\n\n",
					m_NewItemCount, strLocName, nQtyOtherLoc);
			}
			else {
				str.Format("- Additionally, you have configured %li items to be assigned to other locations.\n\n", nQtyOtherLoc);
			}

			strWarning += str;
		}

		if (strWarning.IsEmpty()) {
			return TRUE;
		}

		strWarning = "The location assignments for this product have triggered the following warnings:\n\n" + strWarning +
			"When billing this product, you will only be able to choose serial numbered / expiration date items\n"
			"that are associated with the location of the bill, and items unassociated with any location.\n\n"
			"Are you sure you wish to save these items to these locations?";

		if (!bSilent) {
			if (IDYES == MessageBox(strWarning, "Practice", MB_YESNO | MB_ICONQUESTION)) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
		else {
			// (c.haag 2007-12-07 15:06) - PLID 28286 - We're silent...but assume the user wants to say "yes"
			return TRUE;
		}
	}
	else {
		//otherwise, the saved amount on hand is accurate

		//we must compare the numbers for each location

		CString str, strWarning;
		_RecordsetPtr rs = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1");
		while (!rs->eof) {

			long nLocID = AdoFldLong(rs, "ID");
			CString strLocName = AdoFldString(rs, "Name", "");

			long nQtySetToLoc = 0;

			double dblAmtOnHand = 0.0;
			double dblAllocated = 0.0;

			// (j.jones 2007-12-18 11:42) - PLID 28037 - CalcAmtOnHand changed to return allocation information,
			//but that is unused in this function
			InvUtils::CalcAmtOnHand(m_ProductID, nLocID, dblAmtOnHand, dblAllocated);

			// (j.jones 2007-11-26 09:26) - PLID 28037 - supported allocated items
			_RecordsetPtr rs2 = CreateRecordset("SELECT Count(ID) AS CountOfProdItems FROM ProductItemsT "
				"WHERE ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
				"%s "
				"AND ProductID = %li AND ProductItemsT.Deleted = 0 "
				"AND ProductItemsT.LocationID = %li",
				GetAllocationWhereClause(), m_ProductID, nLocID);
			long LocCountOfProdItems = 0;
			if (!rs2->eof) {
				LocCountOfProdItems = AdoFldLong(rs2, "CountOfProdItems", 0);
			}
			rs2->Close();

			long LocDiff = (long)dblAmtOnHand - LocCountOfProdItems;

			for (int i = 0; i<m_List->GetRowCount(); i++) {
				long nLocIDToCompare = VarLong(m_List->GetValue(i, COLUMN_LOCATION_ID), -1);
				if (nLocID == nLocIDToCompare)
					nQtySetToLoc++;
			}

			//Warn if the amount of items configured to go into this location match the amount needed,
			//unless we default items to no location. If we do have that default, and "some" of the items are set
			//to go to our location, well, warn that too, because that's plain weird.
			// (c.haag 2008-07-01 17:37) - PLID 30594 - "Default Serialized Items To No Location" has been deprecated. Because
			// we always want to act like the preference value is 0, the logic simplifies to
			// "0 != 1 || nQtyRightLoc > 0" = "1 || nQtyRightLoc > 0" = "1" = always true
			if (nQtySetToLoc != LocDiff /*&& (GetRemotePropertyInt("DefaultSerializedItemsToNoLocation",0,0,"<None>",TRUE) != 1
										|| nQtySetToLoc > 0)*/) {

				CString str;
				str.Format("- You are entering %li items at %s but have configured %li to be assigned to this location.\n\n",
					(long)LocDiff, strLocName, nQtySetToLoc);

				strWarning += str;
			}

			rs->MoveNext();
		}
		rs->Close();

		if (strWarning.IsEmpty()) {
			return TRUE;
		}

		strWarning = "The location assignments for this product have triggered the following warnings:\n\n" + strWarning +
			"When billing this product, you will only be able to choose serial numbered / expiration date items\n"
			"that are associated with the location of the bill, and items unassociated with any location.\n\n"
			"Are you sure you wish to save these items to these locations?";

		if (!bSilent) {
			if (IDYES == MessageBox(strWarning, "Practice", MB_YESNO | MB_ICONQUESTION)) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
		else {
			// (c.haag 2007-12-07 15:06) - PLID 28286 - We're silent...but assume the user wants to say "yes"
			return TRUE;
		}
	}
	return TRUE;
}

BOOL CProductItemsDlg::DestroyWindow()
{
	try {
		//DRT 11/6/2007 - PLID 28018 - These used to precede CNxDialog::OnOK and ::OnCancel.  Being here works
		//	for modeless or modal approaches.
		if (m_List != NULL) m_List.Release();
		if (m_SelectedList != NULL) m_SelectedList.Release();

		//DRT 11/7/2007 - PLID 28020 - No longer receive barcode scan messages.
		if (m_bCurrentlyReceivingScans && GetMainFrame()->UnregisterForBarcodeScan(this)) {
			m_bCurrentlyReceivingScans = false;
		}

	} NxCatchAll("Error in DestroyWindow");

	return CNxDialog::DestroyWindow();
}

void CProductItemsDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CNxDialog::OnShowWindow(bShow, nStatus);

	try {
		//DRT 11/7/2007 - PLID 28018 - If we're operating in a fake-modeless manner, we actually want to just disable
		//	the parent dialog and enable our own.  This makes the dialog "look" modal (like Billing).  Do the reverse
		//	when we quit.  You must have set a parent when creating the dialog for this to function.
		//Note:  You may be able to leave the parent NULL and get a true modeless window, though it was not designed
		//	to work that way and has not been tested that way.
		if (m_bModeless && GetParent()) {
			if (bShow) {
				GetParent()->EnableWindow(FALSE);
				EnableWindow();

				//DRT 11/7/2007 - PLID 28020 - No longer receive barcode scan messages.
				if (m_bCurrentlyReceivingScans == false && GetMainFrame()->RegisterForBarcodeScan(this)) {
					m_bCurrentlyReceivingScans = true;
				}
			}
			else {
				EnableWindow(FALSE);
				GetParent()->EnableWindow(TRUE);

				//DRT 11/7/2007 - PLID 28020 - No longer receive barcode scan messages.
				if (m_bCurrentlyReceivingScans && GetMainFrame()->UnregisterForBarcodeScan(this)) {
					m_bCurrentlyReceivingScans = false;
				}
			}
		}

	} NxCatchAll("Error in OnShowWindow");
}

long CProductItemsDlg::CountValidRows()
{
	//DRT 11/7/2007 - PLID 28022 - This will loop through the datalist and count the valid rows entered.  This is only valid
	//	for the entry type (the other types are not looking for entered data).  Will return 0 for all non-valid types.
	//Expiration dates in the past are considered valid.
	long nValidRows = 0;

	try { // (b.eyers 2015-05-20) - PLID 28048

		if (m_EntryType == PI_ENTER_DATA) {
			//Again, I'm not a huge fan of the datalist1 looping, but doing it to be consistent with the rest of the file.
			for (int i = 0; i < m_List->GetRowCount(); i++) {
				//This is copied mostly from the Save() function.
				CString strSerNum = VarString(m_List->GetValue(i, COLUMN_SERIAL_NUMBER), "");

				_variant_t varExpDate = m_List->GetValue(i, COLUMN_EXP_DATE);
				CString strExpDate;
				if (varExpDate.vt != VT_DATE)
					strExpDate = VarString(varExpDate, "");
				else
					strExpDate = FormatDateTimeForSql(varExpDate, dtoDate);

				strSerNum.TrimRight();
				strExpDate.TrimRight();
				//only check for serial numbers if we are using them
				if (m_bUseSerial && !IsSerialNumberTextValid(strSerNum)) {
					//The text is not valid, so we will not increment
				}
				else if (m_bUseExpDate && !IsExpirationTextValid(strExpDate)) {
					//The text is not valid, so we will not increment.  If the s/n is already invalid, we skip this entirely.
				}
				else {
					//It must be valid, increment our count
					nValidRows++;
				}
			}
		}
	} NxCatchAll("Error in CProductItemsDlg:CountValidRows");

	return nValidRows;
}

bool CProductItemsDlg::IsSerialNumberTextValid(CString strText)
{
	//DRT 11/7/2007 - PLID 28022 - Helper functions to centralize what text is considered 'invalid'.
	if (strText == "" || strText == NEW_SERIAL_NUM_TEXT || strText == NO_DATA_TEXT) {
		return false;
	}
	else {
		return true;
	}
}

bool CProductItemsDlg::IsExpirationTextValid(CString strText)
{
	//DRT 11/7/2007 - PLID 28022 - Helper functions to centralize what text is considered 'invalid'.  I don't know why
	//	<No Data> and '' are not included here, I've seen them in certain areas.  Just copied this code from the Save()
	//	function.
	if (strText == NEW_EXP_DATE_TEXT) {
		return false;
	}
	else {
		return true;
	}
}

void CProductItemsDlg::RememberProductItemValues(NXDATALISTLib::_DNxDataListPtr& dlList)
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		  // (c.haag 2007-12-05 16:18) - PLID 28286 - Moved to its own function
		int i;
		m_avProductItemSerials.RemoveAll();
		m_avProductItemExpDates.RemoveAll();
		m_avProductItemStatus.RemoveAll(); // (c.haag 2008-03-11 12:45) - PLID 29255 - Also remove corresponding Purchased Inv./Consignment status values

		long count = dlList->GetRowCount();
		for (i = 0; i < count; i++) {
			m_adwProductItemIDs.Add(VarLong(dlList->GetValue(i, COLUMN_INDEX)));
			// (c.haag 2007-11-08 16:10) - PLID 28025 - Include serial numbers and expiration dates.
			// In the list, some of these can be sentinel values which aren't legitimate for passing
			// along to other functions outside this class. Some examples include "<No Data>" and
			// "[Enter New Exp. Date]" (refer to the top of this source). There is a precedent for 
			// treating these values as special, even if the user manually enters them in. So, we will
			// treat them as special as well, and set them to null variants. We don't need to check
			// the m_bUseSerial or m_bUseExpDate flags because the list should have already taken those
			// things into account.
			_variant_t v, vNull;
			vNull.vt = VT_NULL;
			v = dlList->GetValue(i, COLUMN_SERIAL_NUMBER);
			if (VT_BSTR == v.vt && !IsSerialNumberTextValid(VarString(v))) {
				// If we get here, the serial number is a special sentinel value. We treat these
				// as nulls.
				v = vNull;
			}
			else {
				// This is not a special value; add it as is
			}
			m_avProductItemSerials.Add(v);

			v = dlList->GetValue(i, COLUMN_EXP_DATE);
			if (VT_BSTR == v.vt && (!IsExpirationTextValid(VarString(v)) || NO_DATA_TEXT == VarString(v))) {
				// If we get here, the expiration date is a special sentinel value. We treat these
				// as nulls.
				v = vNull;
			}
			else {
				// This is not a special value; add it as is
			}
			m_avProductItemExpDates.Add(v);

			// (c.haag 2008-03-11 12:43) - PLID 29255 - We can now get the Purchased Inv./Consignment status
			v = dlList->GetValue(i, COLUMN_STATUS);
			m_avProductItemStatus.Add(v);
		}
	} NxCatchAll("Error in CProductItemsDlg:RememberProductItemValues");
}

//DRT 1/8/2008 - PLID 28473 - This will review the item statuses for each item.  It will then return:
//	-1	A mix of types.
//	0	All are destined for 'Purchased Inventory'
//	1	All are destined for 'Consignment'
//
//Add more as we add new status.  I used -1 because I wanted to keep 0/1 to match with data (ProductItemsT.Status = 0 is purchased inv).
//	This should ONLY be called for the EnterData type.  You will always get -1 otherwise.  No rows will also return -1, as that is 
//	undefined.
long CProductItemsDlg::GetLabelForItemStatus()
{
	long nGeneral = 0;
	long nConsign = 0;

	if (m_EntryType == PI_ENTER_DATA) {
		//Again, I'm not a huge fan of the datalist1 looping, but doing it to be consistent with the rest of the file.
		for (int i = 0; i < m_List->GetRowCount(); i++) {
			//This is copied mostly from the Save() function.
			long nStatus = VarLong(m_List->GetValue(i, COLUMN_STATUS));
			if (nStatus == 0) {
				nGeneral++;
			}
			else if (nStatus == 1) {
				nConsign++;
			}
		}
	}

	if (nGeneral == 0 && nConsign != 0) {
		//There are consignment, no general.
		return 1;
	}
	else if (nConsign == 0 && nGeneral != 0) {
		//There are general, no consign
		return 0;
	}
	else {
		//Catch all, both values are 0, or non-PI_ENTER_DATA entry type will all go here.
		return -1;
	}
}

// (a.walling 2008-03-20 10:04) - PLID 29333 - Change the quantity of items to enter data for
void CProductItemsDlg::ChangeQuantity(long nNewQuantity)
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		  //if using UU/UO, they may want to enter in serial numbers per unit of order,
		  //so only enter in the amount ordered in terms of UO
		  /*if(m_bUseUU && m_bSerialPerUO && m_NewItemCount != -2) {
		  m_NewItemCount /= m_nConversion;
		  }
		  */

		m_PrevItemCount = m_NewItemCount;

		if (m_bUseUU && !m_bSerialPerUO) {
			nNewQuantity *= m_nConversion;
		}

		if (nNewQuantity == m_NewItemCount) {
			return;
		}

		if (m_NewItemCount != -2)
			SetDlgItemInt(IDC_QUANTITY, nNewQuantity);

		long nLocID = -1;

		//unless the preference specifies otherwise, set the location to the order location (or the passed-in location)
		// (c.haag 2008-07-01 17:44) - PLID 30594 - This preference has been deprecated
		//if(GetRemotePropertyInt("DefaultSerializedItemsToNoLocation",0,0,"<None>",TRUE) != 1) {
		nLocID = m_nLocationID;
		//}

		//if the m_NewItemCount is not -2, then this is simple, just add a row for each item
		ASSERT(m_NewItemCount != -2);

		long nItemsToAdd = nNewQuantity - m_NewItemCount;

		for (int i = m_NewItemCount; i < m_NewItemCount + nItemsToAdd; i++) {
			IRowSettingsPtr pRow;
			pRow = m_List->GetRow(-1);
			pRow->PutValue(COLUMN_INDEX, (long)(i + 1));
			_variant_t var;
			var.vt = VT_NULL;
			if (m_bUseSerial)
				pRow->PutValue(COLUMN_SERIAL_NUMBER, _bstr_t(NEW_SERIAL_NUM_TEXT));
			else
				pRow->PutValue(COLUMN_SERIAL_NUMBER, var);
			if (m_bUseExpDate)
				pRow->PutValue(COLUMN_EXP_DATE, _bstr_t(NEW_EXP_DATE_TEXT));
			else
				pRow->PutValue(COLUMN_EXP_DATE, var);

			// (j.jones 2007-11-09 09:28) - PLID 28030 - added Consignment column
			// (c.haag 2007-12-03 11:37) - PLID 28204 - Changed consignment bit to status integer
			//_variant_t varConsign;
			//varConsign.vt = VT_BOOL;
			//varConsign.boolVal = m_bDefaultConsignment;
			pRow->PutValue(COLUMN_STATUS, m_nDefaultStatus);

			pRow->PutValue(COLUMN_LOCATION_ID, (long)nLocID);

			// (j.jones 2008-02-29 16:38) - PLID 29125 - always false here
			_variant_t varFalse = VARIANT_FALSE;
			pRow->PutValue(COLUMN_LINKED_ALLOCATION, varFalse);

			m_List->InsertRow(pRow, m_List->GetRowCount());
		}

		m_NewItemCount = nNewQuantity;
	} NxCatchAll("Error in CProductItemsDlg:ChangeQuantity");
}

// (a.walling 2008-03-20 11:33) - PLID 29333 - Returns the quantity (per product) and the conversion rate (for number of items)
void CProductItemsDlg::GetQuantity(long &nQuantity, long &nConversion)
{
	try { // (b.eyers 2015-05-20) - PLID 28048
		nQuantity = m_NewItemCount;
		nConversion = (m_bUseUU && !m_bSerialPerUO) ? m_nConversion : 1;
	} NxCatchAll("Error in CProductItemsDlg:GetQuantity");
}

// (c.haag 2008-06-25 11:42) - PLID 28438 - This will change the internal location ID of the dialog,
// as well as the location ID of every list entry
void CProductItemsDlg::ChangeLocationID(long nLocationID)
{
	try { // (b.eyers 2015-05-20) - PLID 28048
		  // Change the member location ID
		m_nLocationID = nLocationID;

		// Update the location ID in all the list rows
		if (IsWindow(GetSafeHwnd())) {
			ChangeLocationID(m_List, nLocationID);
			ChangeLocationID(m_SelectedList, nLocationID);
		}
	} NxCatchAll("Error in CProductItemsDlg:ChangeLocationID");
}

// (c.haag 2008-06-25 11:42) - PLID 28438 - This will change the location ID of every entry
// in the specified list
void CProductItemsDlg::ChangeLocationID(NXDATALISTLib::_DNxDataListPtr& dlList, long nLocationID)
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		long nCount = dlList->GetRowCount();
		for (long i = 0; i < nCount; i++) {
			dlList->PutValue(i, COLUMN_LOCATION_ID, nLocationID);
		}
	} NxCatchAll("Error in CProductItemsDlg:ChangeLocationID");
}

void CProductItemsDlg::OnBtnApplyLotNum()
{
	try {
		//TES 7/3/2008 - PLID 24726 - clicking this button will auto-apply one lot number
		//to all items in the list. The lot numbers can be edited later on
		//if the user wishes to do so.

		CString strLotNumber;
		GetDlgItemText(IDC_LOT_NUMBER, strLotNumber);
		strLotNumber.TrimLeft();
		strLotNumber.TrimRight();

		// (c.haag 2008-03-18 17:26) - PLID 29306 - If we're entering serial numbers, we should
		// check to see whether the code exists as a product barcode. If it does, then
		// Barcode_CheckExistenceOfProductCode will warn the user about it.
		if (!strLotNumber.IsEmpty()) {
			InvUtils::Barcode_CheckExistenceOfProductCode(strLotNumber, FALSE);
		}

		for (int i = 0; i<m_List->GetRowCount(); i++) {
			m_List->PutValue(i, COLUMN_SERIAL_NUMBER, _bstr_t(strLotNumber));
		}
	}NxCatchAll("Error in CProductItemsDlg::OnBtnApplyLotNum()");
}

void CProductItemsDlg::OnBtnApplyLotNumToSelected()
{
	try {
		if (m_List->CurSel == -1) {
			AfxMessageBox("You must select at least one product from the list first.");
			return;
		}

		//TES 7/3/2008 - PLID 24726 - clicking this button will auto-apply one lot number
		//to all selected items in the list. The lot numbers can be edited later on
		//if the user wishes to do so.

		CString strLotNumber;
		GetDlgItemText(IDC_LOT_NUMBER, strLotNumber);
		strLotNumber.TrimLeft();
		strLotNumber.TrimRight();

		// (c.haag 2008-03-18 17:26) - PLID 29306 - If we're entering serial numbers, we should
		// check to see whether the code exists as a product barcode. If it does, then
		// Barcode_CheckExistenceOfProductCode will warn the user about it.
		if (!strLotNumber.IsEmpty()) {
			InvUtils::Barcode_CheckExistenceOfProductCode(strLotNumber, FALSE);
		}

		long i = 0;
		long p = m_List->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;


		/*
		//loop once for the count
		while (p) {

		i++;

		m_List->GetNextSelEnum(&p, &pDisp);

		IRowSettingsPtr pRow(pDisp);

		pDisp->Release();
		}

		//now prompt

		CString str;
		str.Format("This action will update the %li selected products to use %s as the expiration date.\n"
		"Are you sure you wish to do this?", i, FormatDateTimeForInterface(dt, NULL, dtoDate));
		if(IDNO == MessageBox(str,"Practice",MB_ICONQUESTION|MB_YESNO)) {
		return;
		}

		//now actually update the expiration date

		i = 0;
		p = m_List->GetFirstSelEnum();
		*/

		while (p) {

			i++;

			m_List->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			pRow->PutValue(COLUMN_SERIAL_NUMBER, _bstr_t(strLotNumber));

			pDisp->Release();
		}

	}NxCatchAll("CProductItemsDlg::OnBtnApplyLotNumToSelected()");
}

void CProductItemsDlg::OnRButtonDownItemList(long nRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		if (nRow == -1 || nCol == -1) {
			return;
		}

		m_List->PutCurSel(nRow);

		// (z.manning 2008-07-07 13:03) - PLID 30602 - If the serial number column is editable
		// then popup a right click menu with an option to clear the serial number.
		if (m_List->GetColumn(COLUMN_SERIAL_NUMBER)->GetEditable())
		{
			CMenu mnu;
			mnu.CreatePopupMenu();

			enum MenuOptions
			{
				moClearSerialNum = 1,
			};

			mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, moClearSerialNum, "&Clear Serial Number");

			CPoint pt(x, y);
			CWnd *pWnd = GetDlgItem(IDC_ITEM_LIST);
			if (pWnd != NULL) {
				pWnd->ClientToScreen(&pt);
			}

			int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, this, NULL);
			switch (nResult)
			{
			case moClearSerialNum:
				m_List->PutValue(nRow, COLUMN_SERIAL_NUMBER, "");

				// (j.jones 2011-07-22 12:12) - PLID 42439 - if we are in READ_DATA mode, we need to save immediately
				if (m_EntryType == PI_READ_DATA) {
					ExecuteParamSql("UPDATE ProductItemsT SET SerialNum = NULL WHERE ID = {INT}", VarLong(m_List->GetValue(nRow, COLUMN_INDEX)));

					//disabling the cancel button is worthless in READ_DATA mode,
					//but that is what we do in OnEditingFinished, so I will do the same here
					GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
				}
				break;
			}
		}

	}NxCatchAll("CProductItemsDlg::OnRButtonDownItemList");
}

// (d.thompson 2009-08-17) - PLID 18899 - Updates the count of all selected items on the dialog.
void CProductItemsDlg::UpdateSelectedItemsCount()
{
	try { // (b.eyers 2015-05-20) - PLID 28048

		long nRows = m_SelectedList->GetRowCount();
		SetDlgItemText(IDC_PRODUCT_SELECTED_COUNT, FormatString("%li Items Selected", nRows));
	} NxCatchAll("Error in CProductItemsDlg:UpdateSelectedItemsCount");
}

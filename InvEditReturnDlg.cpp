// InvEditReturnDlg.cpp : implementation file
//
// (c.haag 2007-11-06 16:51) - PLID 27994 - Initial version

#include "stdafx.h"
#include "InvEditReturnDlg.h"
#include "GlobalUtils.h"
#include "InvUtils.h"
#include "audittrail.h"
#include "internationalutils.h"
#include "EditComboBox.h"
#include "barcode.h"
#include "reportinfo.h"
#include "reports.h"
#include "globalreportutils.h"
#include "MsgBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// Sentinel ID's
#define ID_NEW_RETURN_GROUP			-1L
#define ID_NEW_RETURN_ITEM			-1L

// Command ID's
#define ID_REMOVE_RETURN_ITEM		100

// Timer ID's
#define	IDT_REQUERY_ITEM_COMBO		1500

// Datalist column enumerations

enum {
	eclLoc_LocationID = 0,
	eclLoc_Name,
} ELocationListColumns;

enum {
	eclSup_PersonID = 0,
	eclSup_Company,
	eclSup_Address,
	eclSup_City,
	eclSup_State,
	eclSup_Zip,
	eclSup_WorkPhone,
	eclSup_Fax,
	eclSup_PaymentMethod,
} ESupplierListColumns;

enum {
	eclItem_ID = 0,
	eclItem_Name,
	eclItem_Actual,
	eclItem_Ordered,
	eclItem_ReorderPoint,
	eclItem_ReorderQuantity,
	eclItem_Cost,
	eclItem_Surplus,
	eclItem_TrackableStatus,
	eclItem_Conversion,
	eclItem_UseUU,
	eclItem_CatalogNumber,
	eclItem_Barcode,
	eclItem_HasSerialNum,
	eclItem_HasExpDate,
} EItemListColumns;

enum {
	eclRet_ReturnItemID = 0,
	eclRet_ProductID,
	eclRet_ProductItemID,
	eclRet_Name,
	eclRet_SerialNum,
	eclRet_ExpDate,
	eclRet_Quantity,
	eclRet_ReasonID,
	eclRet_ReturnedFor,
	eclRet_ProductAdjustmentID,
	eclRet_Completed,
	eclRet_CompletedDate,
	eclRet_CreditAmt,
	eclRet_AmountReceived,
	eclRet_Notes
} EReturnListColumns;

enum {
	eclMet_ID = 0,
	eclMet_Method
} EReturnMethodColumns;

// (c.haag 2008-03-03 12:02) - PLID 29176 - Column enumerations for the
// "return reason" and "return for" dropdowns
enum {
	eclReason_ID = 0,
	eclReason_Name
} EReturnReasonColumns;

enum {
	eclRetFor_ID = 0,
	eclRetFor_Name
} EReturnForColumns;

/////////////////////////////////////////////////////////////////////////////
// CInvEditReturnDlg dialog


CInvEditReturnDlg::CInvEditReturnDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvEditReturnDlg::IDD, NULL)
{
	//{{AFX_DATA_INIT(CInvEditReturnDlg)
	m_strNotes = _T("");
	m_strDescription = _T("");
	m_strTrackingNumber = _T("");
	//}}AFX_DATA_INIT
	m_nLocationID = -1;
	m_nSupplierID = -1;
	m_nReturnMethodID = -1;
	m_nOldLocationID = -1;
	m_nOldSupplierID = -1;
	m_nOldReturnMethodID = -1;

	m_nID = ID_NEW_RETURN_GROUP; // By default, this is a new return group
	m_nReturnGroupIDToLoad = ID_NEW_RETURN_GROUP;

	// This flag is set to true if an exception is thrown on initialization
	m_bErrorOnInit = FALSE;

	// (c.haag 2007-11-13 13:06) - PLID 27996 - Reset our scanning and initialized
	// flags
	m_bInitialized = FALSE;
	m_bIsScanning = FALSE;

	// Soon after the dialog is initialized, the Item dropdown is requeried. Normally,
	// we warn the user if the Item dropdown is empty so that they don't think it's still
	// being populated; but it makes no sense to warn them if they didn't instigate the
	// requery by choosing a supplier or location. This will be set to TRUE after they
	// do so.
	m_bWarnIfItemDropdownEmpty = FALSE;

	//TES 6/23/2007 - PLID 26152 - Initialize default IDs
	m_nInitialLocationID = -1;
	m_nInitialSupplierID = -1;
}

CInvEditReturnDlg::~CInvEditReturnDlg()
{
	// (c.haag 2007-11-08 15:23) - PLID 27994 - Memory cleanup
	POSITION pos = m_mapOldReturnItems.GetStartPosition();
	while (NULL != pos) {
		long nReturnItemID = -1;
		COldReturnItem* pItem = NULL;
		m_mapOldReturnItems.GetNextAssoc(pos, nReturnItemID, pItem);
		if (NULL != pItem) {
			delete pItem;
		}
	}
}

void CInvEditReturnDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvEditReturnDlg)
	DDX_Control(pDX, IDC_BTN_CONFIGURE_RETURN_REASONS, m_btnConfigureReasons);
	DDX_Control(pDX, IDC_BTN_CONFIGURE_RETURN_METHODS, m_btnConfigureMethods);
	DDX_Control(pDX, IDC_CANCEL_BTN, m_btnCancel);
	DDX_Control(pDX, IDC_OK, m_btnOK);
	DDX_Control(pDX, IDC_BTN_DELETE_RETURN, m_btnDelete);
	DDX_Control(pDX, IDC_PTSS_PREVIEW, m_btnPTSSPreview);
	DDX_Text(pDX, IDC_NOTES, m_strNotes);
	DDV_MaxChars(pDX, m_strNotes, 2000);
	DDX_Text(pDX, IDC_DESCRIPTION, m_strDescription);
	DDV_MaxChars(pDX, m_strDescription, 255);
	DDX_Text(pDX, IDC_TRACKING_NUMBER, m_strTrackingNumber);
	DDV_MaxChars(pDX, m_strTrackingNumber, 255);
	DDX_Control(pDX, IDC_DESCRIPTION, m_nxeditDescription);
	DDX_Control(pDX, IDC_TRACKING_NUMBER, m_nxeditTrackingNumber);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_VENDOR_CONFIRMED, m_btnVendorConfirmed);
	DDX_Control(pDX, IDC_CONFIRMATION_NUMBER, m_nxeditConfirmationNumber);
	DDX_Text(pDX, IDC_CONFIRMATION_NUMBER, m_strConfirmationNumber);
	DDX_Control(pDX, IDC_TOTAL_ITEMS_RETURNED_LABEL, m_nxstaticTotalItemsReturned);
	DDX_Control(pDX, IDC_BTN_RETURN_TRACK_FEDEX, m_btnTrackFedex);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CInvEditReturnDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvEditReturnDlg)
	ON_BN_CLICKED(IDC_OK, OnOkBtn)
	ON_BN_CLICKED(IDC_CANCEL_BTN, OnCancelBtn)
	ON_COMMAND(ID_REMOVE_RETURN_ITEM, OnRemoveReturnItem)
	ON_BN_CLICKED(IDC_BTN_DELETE_RETURN, OnBtnDeleteReturn)
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_RETURN_REASONS, OnBtnConfigureReturnReasons)
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_RETURN_METHODS, OnBtnConfigureReturnMethods)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_PTSS_PREVIEW, OnPTSSPreview)
	ON_BN_CLICKED(IDC_VENDOR_CONFIRMED, OnVendorConfirmed)
	ON_BN_CLICKED(IDC_BTN_RETURN_TRACK_FEDEX, OnBtnReturnTrackFedex)
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

void CInvEditReturnDlg::ClearReturnList()
{
	// (c.haag 2007-11-13 10:13) - PLID 27996 - This utility function removes all items from the
	// return list
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlReturnItems->GetFirstRow();
	while (NULL != pRow) {
		long nReturnItemID = VarLong(pRow->GetValue( eclRet_ReturnItemID ), -1);
		if (-1 != nReturnItemID) {
			// (c.haag 2008-01-09 10:06) - I cannot think of a reason why the ID would already be in the array,
			// but do this extra check to ensure there is no redundant auditing
			if (!IsIDInArray(nReturnItemID, m_anDeletedReturns)) {
				m_anDeletedReturns.Add(nReturnItemID);
			}
		}
		pRow = pRow->GetNextRow();
	}
	m_dlReturnItems->Clear();

	// (j.jones 2009-02-09 15:11) - PLID 32706 - added a total item count label
	UpdateTotalItemCountLabel();
}

CString CInvEditReturnDlg::CalculateNameForInterface(NXDATALIST2Lib::IRowSettingsPtr& pRow)
{
	// (c.haag 2007-11-09 15:00) - PLID 27994 - This utility function return an interface-friendly
	// name for a return row
	CString strItemName;
	if (VT_NULL != pRow->GetValue(eclRet_ProductItemID).vt) {
		// This is a serialized product
		CString strSerialNum = VarString( pRow->GetValue(eclRet_SerialNum), "" );
		_variant_t vExpDate = pRow->GetValue(eclRet_ExpDate);
		if (!strSerialNum.IsEmpty()) {
			strItemName.Format("%s - #%s", VarString( pRow->GetValue(eclRet_Name), "" ), strSerialNum);
		} else if (VT_DATE == vExpDate.vt) {
			strItemName.Format("%s - Exp. %s", VarString( pRow->GetValue(eclRet_Name), "" ), FormatDateTimeForInterface(VarDateTime(vExpDate), dtoDate));
		} else {
			// I don't think this should be possible, but ensure we cover our bases
			strItemName.Format("%s - (no serial number)", VarString( pRow->GetValue(eclRet_Name), "" ));
		}
	} else if (VT_NULL != pRow->GetValue(eclRet_ProductID).vt) {
		// This is a non-serialized product
		strItemName.Format("%s", VarString( pRow->GetValue(eclRet_Name), "" ));
	}
	return strItemName;
}

BOOL CInvEditReturnDlg::DoDateTimesMatch(const _variant_t& v1, const _variant_t& v2)
{
	// Try matching on variant type
	if (v1.vt != v2.vt) {
		return FALSE;
	}
	else if (VT_NULL == v1.vt) {
		return TRUE; // Both are NULL
	}
	else if (VT_DATE == v1.vt) {
		if (VarDateTime(v1) == VarDateTime(v2)) {
			return TRUE; // Both are matching times
		} else {
			return FALSE; // The times don't match
		}
	}
	else {
		// This should never happen!
		ThrowNxException("Attempted invalid date comparison");
	}
}

BOOL CInvEditReturnDlg::DoDateTimesMatch(const _variant_t& vTime, NXTIMELib::_DNxTimePtr& nxt)
{
	// (c.haag 2007-11-08 15:37) - PLID 27994 - This function compares a time variant
	// with an NxTime value. This is specialized for this dialog.

	// Get the NxTime value represented by a variant
	_variant_t vnxt;
	switch (nxt->GetStatus()) {
	case 1: // This is the only valid, non-empty status
		vnxt = _variant_t(nxt->GetDateTime(), VT_DATE);
		break;
	default:
		vnxt.vt = VT_NULL;
		break;
	}
	return DoDateTimesMatch(vTime, vnxt);
}

long CInvEditReturnDlg::GetLocationID() const
{
	// (c.haag 2007-11-07 10:21) - PLID 27994 - This utility function returns the
	// currently selected location ID
	return m_nLocationID;
}

void CInvEditReturnDlg::SetLocationID(long nID)
{
	// (c.haag 2008-01-07 17:30) - PLID 27994 - This utility function sets the location ID
	m_nLocationID = nID;
	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
	m_dlLocationCombo->TrySetSelByColumn_Deprecated(eclLoc_LocationID, nID);
}

long CInvEditReturnDlg::GetSupplierID() const
{
	// (c.haag 2007-11-07 10:23) - PLID 27994 - This utility function returns the
	// currently selected supplier ID
	return m_nSupplierID;
}

void CInvEditReturnDlg::SetSupplierID(long nID)
{
	// (c.haag 2008-01-07 17:30) - PLID 27994 - This utility function sets the
	// supplier ID
	m_nSupplierID = nID;
	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
	m_dlSupplierCombo->TrySetSelByColumn_Deprecated(eclSup_PersonID, nID);
}

long CInvEditReturnDlg::GetReturnMethodID() const
{
	// (c.haag 2007-11-08 09:22) - PLID 27994 - This utility function returns the
	// currently selected return method ID
	return m_nReturnMethodID;
}

void CInvEditReturnDlg::SetReturnMethodID(long nID)
{
	// (c.haag 2008-01-07 17:30) - PLID 27994 - This utility function sets the
	// return method ID
	m_nReturnMethodID = nID;
	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
	m_dlReturnMethodCombo->TrySetSelByColumn_Deprecated(eclMet_ID, nID);
}

void CInvEditReturnDlg::SetNewRowDefaults(LPDISPATCH lpNewRow)
{
	// (c.haag 2007-11-07 12:02) - PLID 27994 - Sets the defaults for a new return list item
	NXDATALIST2Lib::IRowSettingsPtr pNewRow(lpNewRow);
	_variant_t vNull;
	vNull.vt = VT_NULL;
	pNewRow->Value[eclRet_ReturnItemID] = ID_NEW_RETURN_ITEM;
	// (c.haag 2008-03-03 12:49) - PLID 29176 - Now defaults to the return reason selection
	if (NULL != m_dlDefReturnReason->CurSel) {
		pNewRow->Value[eclRet_ReasonID] = m_dlDefReturnReason->CurSel->Value[eclReason_ID];
	} else {
		pNewRow->Value[eclRet_ReasonID] = vNull;
	}
	pNewRow->Value[eclRet_CreditAmt] = _variant_t(COleCurrency(0,0));
	pNewRow->Value[eclRet_ProductAdjustmentID] = vNull;
	pNewRow->Value[eclRet_Completed] = _variant_t(VARIANT_FALSE, VT_BOOL);
	pNewRow->Value[eclRet_CompletedDate] = vNull;
	pNewRow->Value[eclRet_AmountReceived] = _variant_t(COleCurrency(0,0));
	pNewRow->Value[eclRet_Notes] = "";

	// (c.haag 2008-03-03 12:48) - PLID 29176 - Now defaults to the returned for selection
	//TES 2/19/2008 - PLID 28954 - Added ReturnedFor
	if (NULL != m_dlDefReturnFor->CurSel) {
		pNewRow->Value[eclRet_ReturnedFor] = m_dlDefReturnFor->CurSel->Value[eclRetFor_ID];
	} else {
		pNewRow->Value[eclRet_ReturnedFor] = vNull;
	}
}

void CInvEditReturnDlg::RequeryItemList()
{
	// (c.haag 2007-11-07 09:30) - PLID 27994 - This is a utility function that
	// requeries the item dropdown given the current location and supplier filters
	long nLocationID = GetLocationID();
	long nSupplierID = GetSupplierID();

	// Do nothing if either filter dropdown has no valid selection
	if (-1 == nLocationID || -1 == nSupplierID) {
		return;
	}

	// Preserve the previous selections
	long nItemListPreRequerySelID;
	if (NULL == m_dlItemCombo->CurSel) {
		nItemListPreRequerySelID = -1;
	} else {
		nItemListPreRequerySelID = m_dlItemCombo->CurSel->GetValue( eclItem_ID );
	}

	// Now update the from clause
	CString strFrom = InvUtils::GetItemDropdownSqlFromClause(nLocationID, nSupplierID);
	m_dlItemCombo->FromClause = _bstr_t(strFrom);
	m_dlItemCombo->WhereClause = "Actual > 0 AND ID IN (SELECT ID FROM ServiceT WHERE Active = 1)";

	// Do the requery and try to reselect the previous item ID if available
	m_dlItemCombo->Requery();
	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
	m_dlItemCombo->TrySetSelByColumn_Deprecated(eclItem_ID, nItemListPreRequerySelID);
}

BOOL CInvEditReturnDlg::HasDialogGroupDataChanged()
{
	// (c.haag 2007-11-08 14:56) - PLID 27944 - Returns TRUE if any group fields have
	// changed since the dialog was opened and loaded an existing return group
	if (m_strDescription != m_strOldDescription) {
		return TRUE;
	}
	if (m_strNotes != m_strOldNotes) {
		return TRUE;
	}
	if (GetLocationID() != m_nOldLocationID) {
		return TRUE;
	}
	if (GetSupplierID() != m_nOldSupplierID) {
		return TRUE;
	}
	if (!DoDateTimesMatch(m_vOldReturnDate, m_nxtReturnDate)) {
		return TRUE;
	}
	if (GetReturnMethodID() != m_nOldReturnMethodID) {
		return TRUE;
	}
	if (m_strTrackingNumber != m_strOldTrackingNumber) {
		return TRUE;
	}
	if((IsDlgButtonChecked(IDC_VENDOR_CONFIRMED)?TRUE:FALSE) != m_bOldVendorConfirmed) {
		return TRUE;
	}
	if(!DoDateTimesMatch(m_vOldDateConfirmed, m_nxtDateConfirmed)) {
		return TRUE;
	}
	if(m_strConfirmationNumber != m_strOldConfirmationNumber) {
		return TRUE;
	}

	// If we get here, then nothing has changed
	return FALSE;
}

BOOL CInvEditReturnDlg::HasDialogItemDataChanged(BOOL& bWillAlterExistingProductAdjustments)
{
	// (c.haag 2007-11-08 15:04) - PLID 27944 - Returns TRUE if any item fields have
	// changed since the dialog was opened and loaded an existing return group
	if (m_anDeletedReturns.GetSize() > 0) {
		return TRUE; // If we deleted anything, the data was definitely changed
	}

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlReturnItems->GetFirstRow();
	while (NULL != pRow) {
		const long nReturnItemID = VarLong(pRow->GetValue( eclRet_ReturnItemID ), -1);
		if (ID_NEW_RETURN_ITEM == nReturnItemID) {
			return TRUE; // If any items were entered as new, the data definitely changed
		}

		COldReturnItem* pori;
		m_mapOldReturnItems.Lookup(nReturnItemID, pori);
		if (NULL != pori) {
			// Quantity
			if (pori->m_dQuantity != VarDouble(pRow->GetValue( eclRet_Quantity ))) {
				bWillAlterExistingProductAdjustments = TRUE;
				return TRUE;
			}
			// Credit amount
			if (pori->m_cyCreditAmt != VarCurrency(pRow->GetValue( eclRet_CreditAmt ))) {
				bWillAlterExistingProductAdjustments = TRUE;
				return TRUE;
			}
			// Return reason ID
			if (pori->m_nReasonID != VarLong(pRow->GetValue( eclRet_ReasonID ), -1)) {
				return TRUE;
			}
			// Completed
			if (pori->m_bCompleted != VarBool(pRow->GetValue( eclRet_Completed ))) {
				return TRUE;
			}
			// Completed date
			if (!DoDateTimesMatch(pori->m_vCompletedDate, pRow->GetValue( eclRet_CompletedDate ))) {
				return TRUE;
			}
			// Amount received
			if (pori->m_cyAmtReceived != VarCurrency(pRow->GetValue( eclRet_AmountReceived ))) {
				return TRUE;
			}
			// Notes
			if (pori->m_strNotes != VarString(pRow->GetValue( eclRet_Notes ))) {
				return TRUE;
			}
			//TES 2/19/2008 - PLID 28954 - ReturnedFor
			if (pori->m_nReturnedFor != VarLong(pRow->GetValue( eclRet_ReturnedFor ),-1)) {
				return TRUE;
			}

		} else {
			ASSERT(FALSE); // Every existing return item should exist in the legacy map
		}

		pRow = pRow->GetNextRow();
	}

	// If we get here, then nothing has changed
	return FALSE;
}

BOOL CInvEditReturnDlg::HasDialogDataChanged(BOOL& bWillAlterExistingProductAdjustments)
{
	// (c.haag 2007-11-07 15:14) - PLID 27944 - Returns TRUE if anything in the group
	// or any line items has changed, and data needs to be written.

	if (ID_NEW_RETURN_GROUP == m_nID) {
		// If this is a new group, return TRUE because we've changed from nothing
		return TRUE;
	}

	// Return TRUE if any group-level data has changed
	if (HasDialogGroupDataChanged()) {
		return TRUE;
	}

	// Return TRUE if any item-level data has changed
	if (HasDialogItemDataChanged(bWillAlterExistingProductAdjustments)) {
		return TRUE;
	}

	return FALSE;
}

BOOL CInvEditReturnDlg::ValidateDialogGroupData()
{
	// (c.haag 2007-11-08 12:49) - PLID 27994 - Returns TRUE if all the group-level form
	// data is valid and writeable to data

	// Description is auto-limited in size by the edit box. We do not allow empty strings.
	if (m_strDescription.IsEmpty()) {
		MessageBox("Please enter a description for this return.", "Inventory Returns", MB_ICONERROR);
		GetDlgItem(IDC_DESCRIPTION)->SetFocus();
		return FALSE;
	}

	// Date placed (Return Date). Requires a date. Time enums: 1 = valid, 2 = invalid, 3 = empty
	if (1 != m_nxtReturnDate->GetStatus()) {
		MessageBox("Please enter a valid date into the 'Date Placed' field", "Inventory Returns", MB_ICONERROR);
		GetDlgItem(IDC_DATE)->SetFocus();
		return FALSE;
	}
	const COleDateTime dtReturn = COleDateTime(m_nxtReturnDate->GetDateTime());
	if (dtReturn.m_dt <= 0 || dtReturn.GetYear() >= 2222) {
		MessageBox("Please enter a valid date into the 'Date Placed' field", "Inventory Returns", MB_ICONERROR);
		GetDlgItem(IDC_DATE)->SetFocus();
		return FALSE;
	}

	COleDateTime dtToday = COleDateTime::GetCurrentTime();
	dtToday.SetDate( dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay() );
	if (dtReturn > dtToday) {
		if (IDNO == MessageBox("You have entered a return date in the future. Are you sure you wish to continue?", "Inventory Returns", MB_YESNO | MB_ICONWARNING)) {
			return FALSE;
		}
	}

	// Tracking number is auto-limited in size by the edit box. We allow empty strings.

	// Return method dropdown must have a selection
	if (-1 == GetReturnMethodID()) {
		MessageBox("Please choose a method for this return", "Inventory Returns", MB_ICONERROR);
		GetDlgItem(IDC_RETURN_METHOD_LIST)->SetFocus();
		return FALSE;
	}
	
	// Supplier must not be invalid
	if (-1 == GetSupplierID()) {
		MessageBox("Please choose a supplier for this return.", "Inventory Returns", MB_ICONERROR);
		GetDlgItem(IDC_SUPPLIER_RETURN_LIST)->SetFocus();
		return FALSE;
	}

	// Location must not be invalid
	if (-1 == GetLocationID()) {
		MessageBox("Please choose a location for this return.", "Inventory Returns", MB_ICONERROR);
		GetDlgItem(IDC_LOCATION_RETURN_LIST)->SetFocus();
		return FALSE;
	}

	// Notes are limited in size by the form, and may be empty

	//TES 1/26/2009 - PLID 32824 - If they checked the "vendor confirmed" button, they must give a date of confirmation.
	if(IsDlgButtonChecked(IDC_VENDOR_CONFIRMED)) {
		if(m_nxtDateConfirmed->GetStatus() != 1) {
			AfxMessageBox("You must provide a confirmation date if you enable the vendor confirmation option.");
			return FALSE;
		}
	}
	if(m_nxtDateConfirmed->GetStatus() == 1) {
		const COleDateTime dtConfirmed = COleDateTime(m_nxtDateConfirmed->GetDateTime());
		if (dtConfirmed.m_dt <= 0 || dtConfirmed.GetYear() >= 2222) {
			MessageBox("Please enter a valid date into the 'Confirmed on:' field", "Inventory Returns", MB_ICONERROR);
			GetDlgItem(IDC_CONFIRMED_DATE)->SetFocus();
			return FALSE;
		}
	}

	// Validation has passed
	return TRUE;
}

BOOL CInvEditReturnDlg::ValidateDialogItemData()
{
	// (c.haag 2007-11-08 12:58) - PLID 27994 - Returns TRUE if all the individual return item
	// data is valid and can be written to data

	// Fail if there are no items in the list
	if (0 == m_dlReturnItems->GetRowCount()) {
		MessageBox("Before you save this return group, please add at least one item to the return list below by choosing a Supplier, Location, and a selection from the Item dropdown.", "Inventory Returns", MB_ICONERROR);
		return FALSE;
	}

	// Maps non-serializable products to quantities
	CMap<long,long,double,double> mapCurrentQuantity;

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlReturnItems->GetFirstRow();
	while (NULL != pRow) {

		// Calculate the name of the item for the warning box
		CString strItemName = CalculateNameForInterface(pRow);
		if (VT_NULL != pRow->GetValue(eclRet_ProductItemID).vt) {
			// This is a serialized product
		} else if (VT_NULL != pRow->GetValue(eclRet_ProductID).vt) {
			// This is a non-serialized product
			
			// Add the quantity for this product; we will need it to ensure the user is not trying to return more than what they have
			long nProductID = VarLong( pRow->GetValue(eclRet_ProductID) );
			double dProductQuantity = VarDouble(pRow->GetValue(eclRet_Quantity));
			double dExistingQuantity = 0;
			if (!mapCurrentQuantity.Lookup(nProductID, dExistingQuantity)) {
				mapCurrentQuantity.SetAt(nProductID, dProductQuantity);
			} else {
				dExistingQuantity += dProductQuantity;
				mapCurrentQuantity.SetAt(nProductID, dExistingQuantity);
			}
		} 	

		// Quantity is handled outside this loop and in OnEditingFinishingList

		// Credit is handled in OnEditingFinishingList

		// Reason
		_variant_t vReason = pRow->GetValue(eclRet_ReasonID);
		if (VarLong(vReason, -1) <= 0) {
			MessageBox(FormatString("Please select a return reason for the item '%s'", strItemName), "Inventory Returns", MB_ICONERROR);
			m_dlReturnItems->CurSel = pRow;
			return FALSE;
		}

		// Completed can't be messed up and requires no validation

		// Completed date, however, can be messed up if it's inconsistent with the completed checkbox
		BOOL bCompleted = VarBool(pRow->GetValue( eclRet_Completed ) );
		COleDateTime dtInvalid;
		dtInvalid.SetStatus( COleDateTime::invalid );
		COleDateTime dtCompletedDate = VarDateTime( pRow->GetValue( eclRet_CompletedDate ), dtInvalid );
		if (!bCompleted && COleDateTime::valid == dtCompletedDate.GetStatus()) {
			// If we get here, the item is marked as incomplete but it has a completed date
			MessageBox(FormatString("The item '%s' is marked as incomplete, but has a completed date. Please mark the item as complete or remove the date.", strItemName), "Inventory Returns", MB_ICONERROR);
			m_dlReturnItems->CurSel = pRow;
			return FALSE;
		} else if (bCompleted && COleDateTime::invalid == dtCompletedDate.GetStatus()) {
			// If we get here, the item is marked as complete but it has no completion date
			MessageBox(FormatString("Please enter a completion date for the item '%s'.", strItemName), "Inventory Returns", MB_ICONERROR);
			m_dlReturnItems->CurSel = pRow;
			return FALSE;
		} else {
			// If we get here, the date and checkbox are consistent. Now we need to make sure the
			// date itself is valid.
			if (bCompleted) {
				// We should have a valid completed date
				COleDateTime dtToday = COleDateTime::GetCurrentTime();
				dtToday.SetDate( dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay() );
				if (dtCompletedDate > dtToday) {
					// If we get here, the completed date is in the future. I don't know why a user would
					// want to do this, but it doesn't actually cause any problems. Just ask them if they
					// made a mistake.
					if (IDNO == MessageBox(FormatString("The item '%s' has a completed date in the future. Are you sure you wish to save this?", strItemName), "Inventory Returns", MB_YESNO | MB_ICONWARNING)) {
						m_dlReturnItems->CurSel = pRow;
						return FALSE;
					}
				} 
				else if (dtCompletedDate < COleDateTime(m_nxtReturnDate->GetDateTime())) {
					// The return date, which was validated before this function was called, should not be after the
					// completed date of any item under normal circumstances. Warn the user.
					if (IDNO == MessageBox(FormatString("The Completed Date for the item '%s' occurs before the date at which this return was placed. Are you sure you wish to save this?", strItemName), "Inventory Returns", MB_YESNO | MB_ICONWARNING)) {
						m_dlReturnItems->CurSel = pRow;
						return FALSE;
					}
				}
				else {
					// The completed date is either today, or in the past. The datalist doesn't let users
					// enter bad date values, so we're be fine.
				}
			} else {
				// No date, nothing to validate
			}
		}

		// Amount received
		if (VarCurrency(pRow->GetValue( eclRet_AmountReceived )) > VarCurrency(pRow->GetValue( eclRet_CreditAmt ))) {
			// If we get here, the practice received more than they were due for this item. I don't know why
			// this would happen, but confront the user about it.
			if (IDNO == MessageBox(FormatString("The item '%s' has an Amount Received value that exceeds the Amount Due. Are you sure you wish to save this?", strItemName), "Inventory Returns", MB_YESNO | MB_ICONWARNING)) {
				m_dlReturnItems->CurSel = pRow;
				return FALSE;
			} else {
				// The amount received is legitimate
			}
		}
		else if (VarBool(pRow->GetValue( eclRet_Completed )) && VarCurrency(pRow->GetValue( eclRet_AmountReceived )) != VarCurrency(pRow->GetValue( eclRet_CreditAmt ))) {
			// If we get here, the item was marked completed but the amount received is not equal to the credit due. Warn the user.
			if (IDNO == MessageBox(FormatString("The item '%s' has been marked as completed, but the amount received does not match the amount due. Are you sure you wish to save this?", strItemName), "Inventory Returns", MB_YESNO | MB_ICONWARNING)) {
				m_dlReturnItems->CurSel = pRow;
				return FALSE;
			} else {
				// The amount received matches the amount due, as it should for completed returns
			}
		}

		// Notes do not need validation

		//TES 2/19/2008 - PLID 28954 - They need a value for the ReturnedFor column.
		_variant_t vReturnedFor = pRow->GetValue(eclRet_ReturnedFor);
		if (vReturnedFor.vt != VT_I4) {
			MessageBox(FormatString("Please select a value in the Returned For column for the item '%s'", strItemName), "Inventory Returns", MB_ICONERROR);
			m_dlReturnItems->CurSel = pRow;
			return FALSE;
		}

		pRow = pRow->GetNextRow();
	}

	// Now we need to go through our non-serialized products and see if the return quantities exceed the amount on hand.
	// If any of them do, then we need to prevent the user from saving this return.
	//
	// We cannot rely on the item dropdown to get the on hand count because it filters out products where the on hand
	// is less than or, equal to zero. Additionally, this may be an existing return, so we need to subtract the quantity
	// already being returned for this existing return for the product.
	//
	if (mapCurrentQuantity.GetCount() > 0) {
		POSITION posCurrent = mapCurrentQuantity.GetStartPosition();
		CMap<long,long,double,double> mapDataQuantity;
		CArray<long,long> anProductIDs;
		long nProductID = -1;
		double dCurrentQuantity = 0;
		double dDataQuantity = 0;

		// Populate anProductID's with all unique product ID's for which we have quantities for
		while (NULL != posCurrent) {
			mapCurrentQuantity.GetNextAssoc(posCurrent, nProductID, dCurrentQuantity);
			anProductIDs.Add(nProductID);
		}
		// Now populate mapCurrentQuantity with the current on hand quantities according to the
		// current data. If this is an existing return group, we have to "ignore" the values of that
		// return group by adding the existing return quantity to the on hand column.
		CString strSql = FormatString("SELECT ID, COALESCE(Actual, 0) %s AS OnHand, UseUU, Conversion FROM %s WHERE ID IN (%s)"
			,(ID_NEW_RETURN_GROUP != m_nID) ? 
				"+ (SELECT COALESCE(Sum(Quantity), 0) FROM SupplierReturnItemsT "
				"INNER JOIN SupplierReturnGroupsT ON SupplierReturnItemsT.ReturnGroupID = SupplierReturnGroupsT.ID "
				"WHERE SupplierReturnGroupsT.ID = " + AsString(m_nID) + " "
				"AND SupplierReturnGroupsT.LocationID = " + AsString(GetLocationID()) + " "
				"AND SupplierReturnItemsT.ProductID = ItemQ.ID) " : " "
			,InvUtils::GetItemDropdownSqlFromClause(GetLocationID(), GetSupplierID()) + " "
			,ArrayAsString(anProductIDs)
			);
		_RecordsetPtr prs = CreateRecordsetStd(strSql);
		FieldsPtr f = prs->Fields;
		while (!prs->eof) {
			double dOnHand = AdoFldDouble(f, "OnHand");
			// Check whether we specify different units of order. Inventory returns always deal with unit of usage, so
			// we have to convert the reported on-hand here.
			if (AdoFldLong(f, "UseUU") != 0) {
				dOnHand *= (double)AdoFldLong(f, "Conversion");
			}
			mapDataQuantity.SetAt( AdoFldLong(f, "ID") , dOnHand );
			prs->MoveNext();
		}
		prs->Close();

		// Now go through every product and compare the quantities we want to return with the
		// on hand quantities in data.
		posCurrent = mapCurrentQuantity.GetStartPosition();
		while (NULL != posCurrent) {

			// Get the next product ID and the quantity we want to return
			mapCurrentQuantity.GetNextAssoc(posCurrent, nProductID, dCurrentQuantity);

			// Now get the on hand quantity as it is in data
			if (!mapDataQuantity.Lookup(nProductID, dDataQuantity)) {
				// If we get here, the product somehow eluded our filter. I don't know how this
				// could ever happen other than an untimely item deletion
				ASSERT(FALSE);
				dDataQuantity = 0;
			}

			if (-1 == LooseCompareDouble(dDataQuantity, dCurrentQuantity, 0.001)) {
				// If we get here, we're trying to return more than we have
				pRow = m_dlReturnItems->FindByColumn(eclRet_ProductID, nProductID, NULL, VARIANT_FALSE);
				CString strProduct = CalculateNameForInterface(pRow);
				CString strMsg = FormatString("You have elected to return %g units of '%s', but you only have %g returnable units on hand. "
					"Please remove or change the return quantity for this item.",
					dCurrentQuantity, strProduct, dDataQuantity);
				MessageBox(strMsg, "Inventory Returns", MB_ICONERROR);
				m_dlReturnItems->CurSel = pRow;
				return FALSE;
			} else {
				// We're returning less than or equal to what we have. No problem.
			}

		}
	} // if (mapCurrentQuantity.GetSize() > 0) {

	// All items have passed validation
	return TRUE;
}

BOOL CInvEditReturnDlg::ValidateDialogData()
{
	// (c.haag 2007-11-08 12:47) - PLID 27994 - Returns TRUE if all the form data on the dialog is
	// safe to save. First, check to see if an error occured on startup. If so, we can't really
	// trust any loaded or queried values, and we should not let the user save the return group.
	if (m_bErrorOnInit) {
		MessageBox("An error occured when initializing this window, and this return cannot be saved. "
			"Please press the 'Cancel' button to dismiss the return window, and try again.",
			"Inventory Returns", MB_ICONERROR);
		return FALSE;
	}

	// Now do group-level validation
	if (!ValidateDialogGroupData()) {
		return FALSE;
	}

	// Now do line-item validation
	if (!ValidateDialogItemData()) {
		return FALSE;
	}

	return TRUE;
}

BOOL CInvEditReturnDlg::Validate(BOOL& bAllowClose)
{
	// (c.haag 2007-11-07 15:08) - PLID 27994 - Perform pre-save validation

	// First, check for bad data in the dialog forms
	if (!ValidateDialogData()) {
		bAllowClose = FALSE; // Don't let the dialog close
		return FALSE; // Return FALSE meaning that we will not save
	}

	// Second, check to see if anything actually changed
	BOOL bWillAlterExistingProductAdjustments = FALSE;
	if (!HasDialogDataChanged(bWillAlterExistingProductAdjustments)) {
		bAllowClose = TRUE; // Let the dialog close
		return FALSE; // Return FALSE meaning that we will not save
	}

	// (c.haag 2007-11-12 17:44) - PLID 27994 - Next, check to see if any changes
	// will alter existing products and adjustments, and warn the user about it
	if (bWillAlterExistingProductAdjustments) {
		if (IDNO == MessageBox("You have changed information in one or more existing return items that will result in "
			"corresponding on-hand quantities being retroactively updated. All tracking-based product "
			"lists and reports will reflect your changes.\n\nDo you wish to continue?",
			"Inventory Returns",
			MB_YESNO | MB_ICONWARNING))
		{
			// Good, we scared them away
			bAllowClose = FALSE; // Don't let the dialog close
			return FALSE; // Return FALSE meaning that we will not save
		}
		else {
			// They want to save their changes after all
		}
	}

	return TRUE; // Validation has passed
}

void CInvEditReturnDlg::GenerateSaveNewReturnGroupSql(CString& strSaveString, long& nAuditTransactionID)
{
	// (c.haag 2007-11-07 13:22) - PLID 27994 - This utility function is used to
	// generate SQL statements for creating a new return group. This also assigns
	// a value to the procedure variable @nReturnGroupID to be used later
	ASSERT(ID_NEW_RETURN_GROUP == m_nID);

	//TES 1/26/2009 - PLID 32824 - Added VendorConfirmed, ConfirmationDate, ConfirmationNumber
	AddStatementToSqlBatch(strSaveString,
		"INSERT INTO SupplierReturnGroupsT (Description, Notes, LocationID, SupplierID, "
			"ReturnDate, ReturnMethodID, TrackingNumber, VendorConfirmed, ConfirmationDate, ConfirmationNumber) "
			"VALUES ('%s', '%s', %d, %d, "
			"%s, %s, '%s', %i, %s, '%s')\r\n"
		"SET @nReturnGroupID = SCOPE_IDENTITY()\r\n",
		_Q(m_strDescription), _Q(m_strNotes), GetLocationID(), GetSupplierID()
		,(1 != m_nxtReturnDate->GetStatus()) ? "NULL" : "'" + _Q(FormatDateTimeForSql(m_nxtReturnDate->GetDateTime())) + "'"
		,(-1 == GetReturnMethodID()) ? "NULL" : AsString(GetReturnMethodID())
		,_Q(m_strTrackingNumber), IsDlgButtonChecked(IDC_VENDOR_CONFIRMED)?1:0, 
		(1 != m_nxtDateConfirmed->GetStatus()) ? "NULL" : "'" + _Q(FormatDateTimeForSql(m_nxtDateConfirmed->GetDateTime())) + "'",
		_Q(m_strConfirmationNumber)
		);

	// (c.haag 2007-11-12 14:50) - PLID 28028 - Audit the creation of the group
	if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
	AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnGroupCreated, -1, "", m_strDescription, aepHigh, aetCreated);
}

void CInvEditReturnDlg::GenerateUpdateExistingReturnGroupSql(CString& strSaveString, long& nAuditTransactionID)
{
	ASSERT(ID_NEW_RETURN_GROUP != m_nID);

	// (c.haag 2007-11-07 13:22) - PLID 27994 - This utility function is used to
	// generate SQL statements for saving an existing return group. This also assigns
	// a value to the procedure variable @nReturnGroupID to be used later
	// (c.haag 2007-11-12 15:39) - PLID 28028 - Added auditing support
	// (j.jones 2008-09-08 10:40) - PLID 30538 - if you add anything to this function,
	// please add support for it in HasDialogGroupDataChanged()
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	AddStatementToSqlBatch(strSaveString, "SET @nReturnGroupID = %d\r\n", m_nID);
	CString strAuditPrefix = FormatString("Group: '%s'   ", m_strOldDescription);

	// Description
	if (m_strDescription != m_strOldDescription) {
		AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnGroupsT SET Description = '%s' WHERE ID = @nReturnGroupID ",
			_Q(m_strDescription));
		if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
		AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnGroupDescription, m_nID, strAuditPrefix, m_strDescription, aepMedium, aetChanged);
	}
	// Notes
	if (m_strNotes != m_strOldNotes) {
		AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnGroupsT SET Notes = '%s' WHERE ID = @nReturnGroupID ",
			_Q(m_strNotes));
		if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
		AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnGroupNotes, m_nID, strAuditPrefix + m_strOldNotes, m_strNotes, aepMedium, aetChanged);
	}
	// Location ID
	if (GetLocationID() != m_nOldLocationID) {
		AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnGroupsT SET LocationID = %d WHERE ID = @nReturnGroupID ",
			GetLocationID());
		// (c.haag 2007-11-12 12:49) - PLID 28028 - Do a little extra work to get auditing names
		CString strLocationName;
		pRow = m_dlLocationCombo->FindByColumn(eclLoc_LocationID, GetLocationID(), NULL, VARIANT_FALSE);
		if (NULL != pRow) { strLocationName = VarString( pRow->GetValue(eclLoc_Name), ""); }
		else { /* If we get here, the location vanished from our combo or the location ID is -1. Neither should ever happen */ }
		if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
		AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnGroupLocation, m_nID, strAuditPrefix + m_strOldLocationName, strLocationName, aepMedium, aetChanged);
	}
	// Supplier ID
	if (GetSupplierID() != m_nOldSupplierID) {
		AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnGroupsT SET SupplierID = %d WHERE ID = @nReturnGroupID ",
			GetSupplierID());
		// (c.haag 2007-11-12 12:49) - PLID 28028 - Do a little extra work to get auditing names
		CString strSupplierName;
		pRow = m_dlSupplierCombo->FindByColumn(eclSup_PersonID, GetSupplierID(), NULL, VARIANT_FALSE);
		if (NULL != pRow) { strSupplierName = VarString( pRow->GetValue(eclSup_Company), ""); }
		else { /* If we get here, the supplier vanished from our combo or the supplier ID is -1. Neither should ever happen */ }
		if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
		AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnGroupSupplier, m_nID, strAuditPrefix + m_strOldSupplierName, strSupplierName, aepMedium, aetChanged);
	}
	// Return date
	if (!DoDateTimesMatch(m_vOldReturnDate, m_nxtReturnDate)) {
		AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnGroupsT SET ReturnDate = %s WHERE ID = @nReturnGroupID ",
			"'" + _Q(FormatDateTimeForSql(m_nxtReturnDate->GetDateTime())) + "'");
		// (c.haag 2007-11-12 12:52) - PLID 28028 - Do a little extra work to get auditing names
		CString strOldDate = FormatDateTimeForInterface(VarDateTime(m_vOldReturnDate), dtoDate); // Return dates can never be NULL
		CString strNewDate = FormatDateTimeForInterface(m_nxtReturnDate->GetDateTime());
		if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
		AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnGroupReturnDate, m_nID, strAuditPrefix + strOldDate, strNewDate, aepMedium, aetChanged);
	}
	// Return method
	if (GetReturnMethodID() != m_nOldReturnMethodID) {
		AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnGroupsT SET ReturnMethodID = %d WHERE ID = @nReturnGroupID ",
			GetReturnMethodID());
		// (c.haag 2007-11-12 13:06) - PLID 28028 - Do a little work to get auditing names
		CString strReturnMethodName;
		pRow = m_dlReturnMethodCombo->FindByColumn(eclMet_ID, GetReturnMethodID(), NULL, VARIANT_FALSE);
		if (NULL != pRow) { strReturnMethodName = VarString( pRow->GetValue(eclMet_Method), ""); }
		else { /* If we get here, the return method vanished or the ID is -1. Neither should ever happen */ }
		if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
		AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnGroupReturnMethod, m_nID, strAuditPrefix + m_strOldReturnMethodName, strReturnMethodName, aepMedium, aetChanged);		
	}
	// Tracking number
	if (m_strTrackingNumber != m_strOldTrackingNumber) {
		AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnGroupsT SET TrackingNumber = '%s' WHERE ID = @nReturnGroupID ",
			_Q(m_strTrackingNumber));
		if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
		AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnGroupTrackingNumber, m_nID, strAuditPrefix + m_strOldTrackingNumber, m_strTrackingNumber, aepMedium, aetChanged);
	}
	//TES 1/26/2009 - PLID 32824 - Vendor Confirmed
	if((IsDlgButtonChecked(IDC_VENDOR_CONFIRMED)?TRUE:FALSE) != m_bOldVendorConfirmed) {
		AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnGroupsT SET VendorConfirmed = %i WHERE ID = @nReturnGroupID ",
			IsDlgButtonChecked(IDC_VENDOR_CONFIRMED)?1:0);
		if(-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
		AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnGroupVendorConfirmed, m_nID, strAuditPrefix + (m_bOldVendorConfirmed?"Yes":"No"), IsDlgButtonChecked(IDC_VENDOR_CONFIRMED)?"Yes":"No", aepMedium, aetChanged);
	}
	//TES 1/26/2009 - PLID 32824 - Confirmation Date
	if(!DoDateTimesMatch(m_vOldDateConfirmed, m_nxtDateConfirmed)) {
		AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnGroupsT SET ConfirmationDate = %s WHERE ID = @nReturnGroupID ",
			(m_nxtDateConfirmed->GetStatus() != 1) ? "NULL" : "'" + _Q(FormatDateTimeForSql(m_nxtDateConfirmed->GetDateTime())) + "'");
		if(-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
		AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnGroupConfirmationDate, m_nID, strAuditPrefix + (m_vOldDateConfirmed.vt == VT_DATE ? FormatDateTimeForInterface(VarDateTime(m_vOldDateConfirmed)) : ""), (m_nxtDateConfirmed->GetStatus() != 1) ? "" : FormatDateTimeForInterface(m_nxtDateConfirmed->GetDateTime()), aepMedium, aetChanged);
	}
	//TES 1/26/2009 - PLID 32824 - Confirmation Number
	if(m_strConfirmationNumber != m_strOldConfirmationNumber) {
		AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnGroupsT SET ConfirmationNumber = '%s' WHERE ID = @nReturnGroupID ",
			_Q(m_strConfirmationNumber));
		if(-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
		AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnGroupConfirmationNumber, m_nID, strAuditPrefix + m_strOldConfirmationNumber, m_strConfirmationNumber, aepMedium, aetChanged);
	}
}

void CInvEditReturnDlg::GenerateSaveNewReturnItemsSql(CString& strSaveString, long& nAuditTransactionID)
{
	// (c.haag 2007-11-07 12:44) - PLID 27994 - This utility function is used to
	// generate SQL statements to add return items to data
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlReturnItems->GetFirstRow();
	while (NULL != pRow) {
		const long nReturnItemID = VarLong(pRow->GetValue( eclRet_ReturnItemID ), -1);
		if (ID_NEW_RETURN_ITEM == nReturnItemID) {
			// We found a new row. Lets do the insert.
			long nProductID = VarLong(pRow->GetValue( eclRet_ProductID )); // Must not be null
			long nProductItemID = VarLong(pRow->GetValue( eclRet_ProductItemID ), -1); // Can be null
			long nReturnReasonID = VarLong(pRow->GetValue( eclRet_ReasonID )); // Must not be null
			
			// If we have a valid serialized product, then ensure the Product ID in data
			// is NULL because that field is mutually exclusive against ProductItemID
			if (-1 != nProductItemID) {
				nProductID = -1;
			}
			if (-1 == nProductItemID && -1 == nProductID) {
				// This should never happen
				ASSERT(FALSE);
				ThrowNxException("Attempted to save a return that is not associated with a product!");
			}

			// Create the product adjustment record
			AddStatementToSqlBatch(strSaveString, "SET @nProductAdjustmentID = (SELECT COALESCE(MAX(ID),0) + 1 FROM ProductAdjustmentsT)");
			//TES 6/26/2008 - PLID 30522 - Assign this to the system "Returned to supplier" category
			AddStatementToSqlBatch(strSaveString,
				"INSERT INTO ProductAdjustmentsT "
					"(ID, ProductID, [Date], Login, "
					"Quantity, Amount, LocationID, Notes, ProductAdjustmentCategoryID "
				") VALUES ( "
					"@nProductAdjustmentID, %d, %s, %d, "
					"%g, Convert(money,'%s'), %d, '%s', %li "
				")"

				,VarLong(pRow->GetValue( eclRet_ProductID ))
				,(1 != m_nxtReturnDate->GetStatus()) ? "NULL" : "'" + _Q(FormatDateTimeForSql(m_nxtReturnDate->GetDateTime())) + "'"
				,GetCurrentUserID()

				,VarDouble(pRow->GetValue( eclRet_Quantity )) * -1.0
				,_Q(FormatCurrencyForSql(VarCurrency(pRow->GetValue( eclRet_CreditAmt )) * -1.0))
				,GetLocationID()
				,_Q(FormatString("Supplier return for '%s'", CalculateNameForInterface(pRow)))
				,InvUtils::g_nReturnedToSupplierID

				);

			// Delete the serialized item
			if (-1 != nProductItemID) {
				// (j.jones 2008-06-02 15:45) - PLID 28076 - now we track the adjustment ID
				//TES 6/23/2008 - PLID 26152 - Now that this item is being returned, we don't 
				// need it to be flagged as To Be Returned
				AddStatementToSqlBatch(strSaveString, "UPDATE ProductItemsT SET Deleted = 1, AdjustmentID = @nProductAdjustmentID, "
					"ToBeReturned = 0 WHERE ID = %d", nProductItemID);
			}

			// Now create the return item itself
			//TES 2/19/2008 - PLID 28954 - Added ReturnedFor
			AddStatementToSqlBatch(strSaveString,
				"INSERT INTO SupplierReturnItemsT "
					"(ReturnGroupID, "
					"ProductID, ProductItemID, Quantity, "
					"ReturnReasonID, CreditAmount, ProductAdjustmentID, "
					"Completed, CompletedDate, AmountReceived, Notes, ReturnedFor "
				") VALUES ("
					"@nReturnGroupID, "
					"%s, %s, %g, "
					"%d, Convert(money,'%s'), @nProductAdjustmentID, "
					"%d, %s, Convert(money,'%s'), '%s', %li "
				")"

				, (-1 == nProductID) ? "NULL" : AsString(nProductID)
				, (-1 == nProductItemID) ? "NULL" : AsString(nProductItemID)
				, VarDouble(pRow->GetValue( eclRet_Quantity ))
				
				, nReturnReasonID
				, _Q(FormatCurrencyForSql(VarCurrency(pRow->GetValue( eclRet_CreditAmt ))))
				
				,(VarBool(pRow->GetValue( eclRet_Completed ))) ? 1 : 0
				,(VT_NULL == pRow->GetValue( eclRet_CompletedDate ).vt) ? "NULL" : "'" + _Q(FormatDateTimeForSql( VarDateTime(pRow->GetValue( eclRet_CompletedDate )) )) + "'"
				,_Q(FormatCurrencyForSql(VarCurrency(pRow->GetValue( eclRet_AmountReceived ))))
				,_Q(VarString(pRow->GetValue( eclRet_Notes ), "")),
				VarLong(pRow->GetValue( eclRet_ReturnedFor ))

				);
			// (c.haag 2007-11-12 14:52) - PLID 28028 - Audit the creation of the item
			CString strItemAuditName;
			CString strDescription = (ID_NEW_RETURN_GROUP == m_nID) ? m_strDescription : m_strOldDescription;
			if (-1 == nProductItemID) { // Audit a non-serialized item
				strItemAuditName.Format("Group: '%s' Item: '%s' Qty: %g", strDescription, VarString(pRow->GetValue( eclRet_Name ), ""), VarDouble(pRow->GetValue( eclRet_Quantity )));
			} else { // Audit a serialized item
				strItemAuditName.Format("Group: '%s' Item: '%s' Serial: '%s' Qty: %g", 
					strDescription, VarString(pRow->GetValue( eclRet_Name ), ""), VarString(pRow->GetValue( eclRet_SerialNum ), ""), VarDouble(pRow->GetValue( eclRet_Quantity )));
			}

			if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
			AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnItemCreated, -1, "", strItemAuditName, aepHigh, aetCreated);

		} else {
			// Existing row in data. Do nothing.
		}
		pRow = pRow->GetNextRow();
	}
}

void CInvEditReturnDlg::GenerateUpdateExistingReturnItemsSql(CString& strSaveString, long& nAuditTransactionID)
{
	// (c.haag 2007-11-08 15:51) - PLID 27944 - This utility function is used to
	// generate SQL statements to update existing return items in data
	// (j.jones 2008-09-08 10:40) - PLID 30538 - if you add anything to this function,
	// please add support for it in HasDialogItemDataChanged()
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlReturnItems->GetFirstRow();
	while (NULL != pRow) {
		const long nReturnItemID = VarLong(pRow->GetValue( eclRet_ReturnItemID ), -1);
		if (ID_NEW_RETURN_ITEM != nReturnItemID) {
			COldReturnItem* pori;
			m_mapOldReturnItems.Lookup(nReturnItemID, pori);
			if (NULL != pori) {

				CString strItemAuditName;
				CString strDescription = (ID_NEW_RETURN_GROUP == m_nID) ? m_strDescription : m_strOldDescription;
				const long nProductItemID = VarLong(pRow->GetValue( eclRet_ProductItemID ), -1);
				if (-1 == nProductItemID) { // Audit a non-serialized item
					strItemAuditName.Format("Group: '%s' Item: '%s'    ", strDescription, VarString(pRow->GetValue( eclRet_Name ), ""), VarDouble(pRow->GetValue( eclRet_Quantity )));
				} else { // Audit a serialized item
					strItemAuditName.Format("Group: '%s' Item: '%s' Serial: '%s'    ", 
						strDescription, VarString(pRow->GetValue( eclRet_Name ), ""), VarString(pRow->GetValue( eclRet_SerialNum ), ""), VarDouble(pRow->GetValue( eclRet_Quantity )));
				}

				// Quantity
				if (pori->m_dQuantity != VarDouble(pRow->GetValue( eclRet_Quantity ))) {
					AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnItemsT SET Quantity = %g WHERE ID = %d",
						VarDouble(pRow->GetValue( eclRet_Quantity )), nReturnItemID);
					AddStatementToSqlBatch(strSaveString, "UPDATE ProductAdjustmentsT SET Quantity = %g WHERE ID IN (SELECT ProductAdjustmentID FROM SupplierReturnItemsT WHERE ID = %d)",
						VarDouble(pRow->GetValue( eclRet_Quantity )) * -1, nReturnItemID);

					// (c.haag 2007-11-12 13:18) - PLID 28028 - Extra name calculations for auditing
					CString strOldQty = strItemAuditName + FormatString("%g", pori->m_dQuantity);
					CString strNewQty = FormatString("%g", VarDouble(pRow->GetValue( eclRet_Quantity )));
					if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
					AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnItemQuantity, nReturnItemID, strOldQty, strNewQty, aepMedium, aetChanged);
				}
				// Credit amount
				if (pori->m_cyCreditAmt != VarCurrency(pRow->GetValue( eclRet_CreditAmt ))) {
					AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnItemsT SET CreditAmount = Convert(money,'%s') WHERE ID = %d",
						_Q(FormatCurrencyForSql(VarCurrency(pRow->GetValue( eclRet_CreditAmt )))), nReturnItemID);
					AddStatementToSqlBatch(strSaveString, "UPDATE ProductAdjustmentsT SET Amount = Convert(money,'%s') WHERE ID IN (SELECT ProductAdjustmentID FROM SupplierReturnItemsT WHERE ID = %d)",
						_Q(FormatCurrencyForSql(VarCurrency(pRow->GetValue( eclRet_CreditAmt )) * -1.0)), nReturnItemID);

					// (c.haag 2007-11-12 13:18) - PLID 28028 - Extra name calculations for auditing
					CString strOldCy = strItemAuditName + FormatCurrencyForInterface(pori->m_cyCreditAmt);
					CString strNewCy = FormatCurrencyForInterface(VarCurrency(pRow->GetValue( eclRet_CreditAmt )));
					if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
					AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnItemCreditAmount, nReturnItemID, strOldCy, strNewCy, aepMedium, aetChanged);
				}
				// Return reason ID
				long nReturnReasonID = VarLong(pRow->GetValue( eclRet_ReasonID ), -1);
				if (pori->m_nReasonID != nReturnReasonID) {
					AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnItemsT SET ReturnReasonID = %s WHERE ID = %d",
						AsString(nReturnReasonID), nReturnItemID);

					// (c.haag 2007-11-12 13:21) - PLID 28028 - Extra name calculations for auditing
					CString strOldReasonName = strItemAuditName + pori->m_strReasonName;
					CString strNewReasonName = VarString(pRow->GetOutputValue( eclRet_ReasonID ),"");
					if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
					AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnItemReturnReason, nReturnItemID, strOldReasonName, strNewReasonName, aepMedium, aetChanged);
				}
				// Completed
				BOOL bCompleted = VarBool(pRow->GetValue( eclRet_Completed ));
				if (pori->m_bCompleted != bCompleted) {
					AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnItemsT SET Completed = %d WHERE ID = %d",
						(VarBool(pRow->GetValue( eclRet_Completed ))) ? 1 : 0, nReturnItemID);

					// (c.haag 2007-11-15 09:30) - PLID 28028 - Extra name calculations for auditing
					CString strOld = strItemAuditName + ((pori->m_bCompleted) ? "Completed" : "Incomplete");
					CString strNew = (bCompleted) ? "Completed" : "Incomplete";
					if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
					AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnItemCompleted, nReturnItemID, strOld, strNew, aepMedium, aetChanged);
				}
				// Completed date
				if (!DoDateTimesMatch(pori->m_vCompletedDate, pRow->GetValue( eclRet_CompletedDate ))) {
					AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnItemsT SET CompletedDate = %s WHERE ID = %d",
						VT_NULL == pRow->GetValue(eclRet_CompletedDate).vt ? "NULL" : "'" + _Q(FormatDateTimeForSql(VarDateTime(pRow->GetValue(eclRet_CompletedDate)))) + "'",
						nReturnItemID);

					// (c.haag 2007-11-15 09:35) - PLID 28028 - Extra name calculations for auditing
					CString strOld = strItemAuditName + ((VT_NULL == pori->m_vCompletedDate.vt) ? "" : FormatDateTimeForInterface(VarDateTime(pori->m_vCompletedDate), dtoDate)); // Return dates can never be NULL
					CString strNew = (VT_NULL == pRow->GetValue(eclRet_CompletedDate).vt) ? "" : FormatDateTimeForInterface(VarDateTime(pRow->GetValue(eclRet_CompletedDate)));
					if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
					AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnItemCompletedDate, nReturnItemID, strOld, strNew, aepMedium, aetChanged);
				}
				// Amount received
				if (pori->m_cyAmtReceived != VarCurrency(pRow->GetValue( eclRet_AmountReceived ))) {
					AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnItemsT SET AmountReceived = Convert(money,'%s') WHERE ID = %d",
						_Q(FormatCurrencyForSql(VarCurrency(pRow->GetValue( eclRet_AmountReceived )))), nReturnItemID);

					// (c.haag 2007-11-15 09:40) - PLID 28028 - Extra name calculations for auditing
					CString strOld = strItemAuditName + FormatCurrencyForInterface(pori->m_cyAmtReceived);
					CString strNew = FormatCurrencyForInterface(VarCurrency(pRow->GetValue( eclRet_AmountReceived )));
					if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
					AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnItemAmountReceived, nReturnItemID, strOld, strNew, aepMedium, aetChanged);
				}
				// Notes
				if (pori->m_strNotes != VarString(pRow->GetValue( eclRet_Notes ))) {
					AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnItemsT SET Notes = '%s' WHERE ID = %d",
						_Q(VarString(pRow->GetValue( eclRet_Notes ))), nReturnItemID);

					// (c.haag 2007-11-19 16:42) - PLID 28028 - Perform auditing
					CString strOld = strItemAuditName + pori->m_strNotes;
					CString strNew = VarString(pRow->GetValue( eclRet_Notes ));
					if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
					AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnItemNotes, nReturnItemID, strOld, strNew, aepMedium, aetChanged);
				}
				//TES 2/19/2008 - PLID 28954 - ReturnedFor
				if (pori->m_nReturnedFor != VarLong(pRow->GetValue( eclRet_ReturnedFor ))) {
					AddStatementToSqlBatch(strSaveString, "UPDATE SupplierReturnItemsT SET ReturnedFor = %li WHERE ID = %d",
						VarLong(pRow->GetValue( eclRet_ReturnedFor )), nReturnItemID);
					CString strOld = strItemAuditName + ((pori->m_nReturnedFor == InvUtils::irfCredit) ? "Credit" : ((pori->m_nReturnedFor == InvUtils::irfExchange) ? "Exchange" : "<none>"));
					CString strNew = ((pori->m_nReturnedFor == InvUtils::irfCredit) ? "Credit" : "Exchange");
					if(-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
					AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnItemReturnedFor, nReturnItemID, strOld, strNew, aepMedium, aetChanged);
				}

			} else {
				ASSERT(FALSE); // Every existing return item should exist in the legacy map
			}
		} else {
			// New row in data. Do nothing.
		}
		pRow = pRow->GetNextRow();
	}
}

void CInvEditReturnDlg::GenerateDeleteExistingReturnItemsSql(CString& strSaveString, long& nAuditTransactionID)
{
	ASSERT(ID_NEW_RETURN_GROUP != m_nID);

	// (c.haag 2007-11-07 15:01) - PLID 27994 - Go through all the deleted ID's and delete them from data
	if (0 == m_anDeletedReturns.GetSize()) {
		return; // Nothing to do if nothing was deleted
	}

	// This statement will remove all the records
	// (c.haag 2007-11-12 15:29) - PLID 28028 - Generate audit information before doing the actual deleting.
	// We can get all we need from the original items map.
	for (int i=0; i < m_anDeletedReturns.GetSize(); i++) {
		long nReturnItemID = m_anDeletedReturns[i];
		COldReturnItem* pItem = NULL;

		// Get the item data in its original form
		if (!m_mapOldReturnItems.Lookup(nReturnItemID, pItem)) {
			ThrowNxException("Attempted to generate delete information for a non-existent product return item!");
		} else {
			// Found it. Ready to audit
		}

		CString strItemAuditName;
		CString strDescription = (ID_NEW_RETURN_GROUP == m_nID) ? m_strDescription : m_strOldDescription;
		if (-1 == pItem->m_nProductItemID) {
			// Audit non-serialized item information
			strItemAuditName.Format("Group: '%s' Item: '%s' Qty: %g", strDescription, pItem->m_strProductName, pItem->m_dQuantity);
		} else {
			// Audit serialized item information
			strItemAuditName.Format("Group: '%s' Item: '%s' Serial: '%s' Qty: %g",
				strDescription, pItem->m_strProductName, pItem->m_strSerialNumber, pItem->m_dQuantity);
		}
		if (-1 == nAuditTransactionID) { nAuditTransactionID = BeginAuditTransaction(); }
		AuditEvent(-1, "", nAuditTransactionID, aeiSupReturnItemDeleted, nReturnItemID, strItemAuditName, "", aepHigh, aetDeleted);
	}

	// (c.haag 2007-11-12 16:43) - PLID 27994 - We can't delete the adjustment before the return item, because of tabular relationships,
	// and we can't do it the opposite way because the product adjustment ID's are in the return items. So, use a table variable to get
	// the job done.
	AddStatementToSqlBatch(strSaveString, "DECLARE @tProductAdjustmentIDs TABLE (ID INT NOT NULL)");
	AddStatementToSqlBatch(strSaveString, "INSERT INTO @tProductAdjustmentIDs (ID) SELECT ProductAdjustmentID FROM SupplierReturnItemsT WHERE ID IN (%s)", ArrayAsString(m_anDeletedReturns));
	AddStatementToSqlBatch(strSaveString, "DELETE FROM SupplierReturnItemsT WHERE ID IN (%s)", ArrayAsString(m_anDeletedReturns));
	// (j.jones 2008-06-02 15:57) - PLID 28076 - clear the adjustment ID from these product items
	AddStatementToSqlBatch(strSaveString, "UPDATE ProductItemsT SET Deleted = 0, AdjustmentID = NULL WHERE AdjustmentID IN (SELECT ID FROM @tProductAdjustmentIDs) OR ID IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ID IN (%s))", ArrayAsString(m_anDeletedReturns));
	AddStatementToSqlBatch(strSaveString, "DELETE FROM ProductAdjustmentsT WHERE ID IN (SELECT ID FROM @tProductAdjustmentIDs)");
}

void CInvEditReturnDlg::Save()
{
	// (c.haag 2007-11-07 12:32) - PLID 27994 - This utility function is the central
	// point for saving changes to data
	CString strSaveString = BeginSqlBatch();
	long nAuditTransactionID = -1;

	// Begin by putting together any SQL lines related to creating, modifying, or deleting
	// this return group
	// (c.haag 2007-11-14 10:58) - PLID 27992 - Have the NOCOUNT mode be ON until we get to the end
	AddStatementToSqlBatch(strSaveString, "SET NOCOUNT ON\r\n"
										"DECLARE @nReturnGroupID INT\r\n"
										"DECLARE @nProductAdjustmentID INT\r\n");
	if (ID_NEW_RETURN_GROUP == m_nID) {
		// Generate a SQL statement to create this group
		GenerateSaveNewReturnGroupSql(strSaveString, nAuditTransactionID);
	} else {
		// Generate a SQL statement to update this group
		GenerateUpdateExistingReturnGroupSql(strSaveString, nAuditTransactionID);
	}

	// Now build the query string to write new items to data
	GenerateSaveNewReturnItemsSql(strSaveString, nAuditTransactionID);
	if (ID_NEW_RETURN_GROUP != m_nID) {

		// Now continue to build it with update statements for existing return items
		GenerateUpdateExistingReturnItemsSql(strSaveString, nAuditTransactionID);

		// Finally, deleting return items
		GenerateDeleteExistingReturnItemsSql(strSaveString, nAuditTransactionID);
	}

	if (!strSaveString.IsEmpty()) {
		// Now commit the save, being aware that the caller is receptive to exceptions
		try {
			// (c.haag 2007-11-14 10:00) - PLID 27992 - Send a table checker after a successful
			// save. To do this right, we must ensure we have the new group ID by pulling it from
			// the query. 
			// (c.haag 2008-03-07 12:54) - PLID 29170 - This PL item is now responsible for the task of the following comment
			// (c.haag 2007-12-24 09:14) - PLID 28120 - We need to remember the group ID in case we're doing
			// a print preview, so store it in m_nID instead of a local variable.
			AddStatementToSqlBatch(strSaveString, "SET NOCOUNT OFF\r\nSELECT @nReturnGroupID AS NewGroupID");
#ifdef _DEBUG
			//(e.lally 2008-04-10)- Switched to our CMsgBox dialog
			CMsgBox dlg(this);
			dlg.msg = strSaveString;
			dlg.DoModal();
#endif
			_RecordsetPtr prs = CreateRecordsetStd("BEGIN TRAN \r\n" + strSaveString + "COMMIT TRAN \r\n");
			if (ID_NEW_RETURN_GROUP == m_nID && !prs->eof) {
				m_nID = AdoFldLong(prs, "NewGroupID");
			}

			CommitAuditTransaction(nAuditTransactionID);

			// (c.haag 2007-11-14 09:56) - PLID 27992 - Now throw a table checker
			CClient::RefreshTable(NetUtils::SupplierReturnGroupsT, m_nID);

		} NxCatchAllSilentCallThrow(
			if (-1 != nAuditTransactionID) { RollbackAuditTransaction(nAuditTransactionID); }
			);
	} else {
		// If we get here, the user simply opened the dialog and pressed OK
	}
}

void CInvEditReturnDlg::LoadReturnGroup()
{
	// (c.haag 2007-11-08 10:00) - PLID 27994 - Loads existing return group data
	// (c.haag 2007-11-12 12:34) - PLID 28028 - We need to join on some tables and get names for auditing purposes
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT SupplierReturnGroupsT.*, LocationsT.Name AS Location, PersonT.Company, SupplierReturnMethodT.Method "
		"FROM SupplierReturnGroupsT "
		"LEFT JOIN LocationsT ON LocationsT.ID = SupplierReturnGroupsT.LocationID "
		"LEFT JOIN PersonT ON PersonT.ID = SupplierReturnGroupsT.SupplierID "
		"LEFT JOIN SupplierReturnMethodT ON SupplierReturnMethodT.ID = SupplierReturnGroupsT.ReturnMethodID "
		"WHERE SupplierReturnGroupsT.ID = {INT}", m_nReturnGroupIDToLoad);
	FieldsPtr f = prs->Fields;
	_variant_t varDate;
	if (prs->eof) {
		ThrowNxException("Attempted to load a group that no longer exists!");
	}

	m_strDescription = m_strOldDescription = AdoFldString(f, "Description");
	m_strNotes = m_strOldNotes = AdoFldString(f, "Notes");

	SetLocationID( m_nOldLocationID = AdoFldLong(f, "LocationID") );
	m_strOldLocationName = AdoFldString(f, "Location", "");

	SetSupplierID( m_nOldSupplierID = AdoFldLong(f, "SupplierID") );
	m_strOldSupplierName = AdoFldString(f, "Company", "");

	varDate = m_vOldReturnDate = f->Item["ReturnDate"]->Value;
	if(varDate.vt == VT_DATE) {
		m_nxtReturnDate->SetDateTime(VarDateTime(varDate));
	} else {
		m_nxtReturnDate->Clear();
	}

	SetReturnMethodID( m_nOldReturnMethodID = AdoFldLong(f, "ReturnMethodID") );
	m_strOldReturnMethodName = AdoFldString(f, "Method", "");

	m_strTrackingNumber = m_strOldTrackingNumber = AdoFldString(f, "TrackingNumber");

	//TES 1/26/2009 - PLID 32824 - Added vendor confirmation fields.
	m_bOldVendorConfirmed = AdoFldBool(f, "VendorConfirmed");
	CheckDlgButton(IDC_VENDOR_CONFIRMED, m_bOldVendorConfirmed?BST_CHECKED:BST_UNCHECKED);
	m_strConfirmationNumber = m_strOldConfirmationNumber = AdoFldString(f, "ConfirmationNumber");
	m_vOldDateConfirmed  = f->Item["ConfirmationDate"]->Value;
	if(m_vOldDateConfirmed.vt == VT_DATE) {
		m_nxtDateConfirmed->SetDateTime(VarDateTime(m_vOldDateConfirmed));
	}
	//TES 1/26/2009 - PLID 32824 - Make sure the controls are enabled properly.
	EnsureConfirmedControls();

	// Update the form controls
	UpdateData(FALSE);
}

void CInvEditReturnDlg::LoadReturnItems()
{
	// (c.haag 2007-11-08 09:56) - PLID 27994 - Loads existing return group line item data

	// Because all return item information is in a datalist, all we really need to do is set
	// the filter, and requery it
	m_dlReturnItems->WhereClause = _bstr_t(FormatString("SupplierReturnItemsT.ReturnGroupID = %d", m_nReturnGroupIDToLoad));
	m_dlReturnItems->Requery();
	m_dlReturnItems->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

	// Now that all the data has loaded, populate our map of old return items for the purpose
	// of comparing old and new items with
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlReturnItems->GetFirstRow();
	while (NULL != pRow) {
		COldReturnItem* pori = new COldReturnItem;
		pori->m_dQuantity = VarDouble(pRow->GetValue( eclRet_Quantity ));
		pori->m_cyCreditAmt = VarCurrency(pRow->GetValue( eclRet_CreditAmt ));
		pori->m_nReasonID = VarLong(pRow->GetValue( eclRet_ReasonID ));
		// (c.haag 2007-11-12 13:29) - PLID 28028 - Get the reason name and product names for auditing
		pori->m_strReasonName = VarString(pRow->GetOutputValue( eclRet_ReasonID ),"");
		pori->m_strProductName = VarString(pRow->GetValue( eclRet_Name ));
		pori->m_strSerialNumber = VarString(pRow->GetValue( eclRet_SerialNum ),"");
		pori->m_nProductItemID = VarLong(pRow->GetValue( eclRet_ProductItemID ), -1);
		pori->m_bCompleted = VarBool(pRow->GetValue( eclRet_Completed ));
		pori->m_vCompletedDate = pRow->GetValue( eclRet_CompletedDate );
		pori->m_cyAmtReceived = VarCurrency(pRow->GetValue( eclRet_AmountReceived ));
		pori->m_strNotes = VarString(pRow->GetValue( eclRet_Notes ));
		//TES 2/19/2008 - PLID 28954 - Added ReturnedFor
		pori->m_nReturnedFor = VarLong(pRow->GetValue( eclRet_ReturnedFor ), -1);

		m_mapOldReturnItems.SetAt( pRow->GetValue( eclRet_ReturnItemID ), pori );
		pRow = pRow->GetNextRow();
	}
}

void CInvEditReturnDlg::Load()
{
	// (c.haag 2007-11-08 09:57) - PLID 27994 - Loads an existing return group with items. This
	// is a utility function run in OnInitDialog.
	CWaitCursor wc;

	// Load group-specific information
	LoadReturnGroup();

	// Load information for all items
	LoadReturnItems();

	// The load is finished. Go ahead an update our internal ID
	m_nID = m_nReturnGroupIDToLoad;
}

/////////////////////////////////////////////////////////////////////////////
// CInvEditReturnDlg message handlers

BOOL CInvEditReturnDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (c.haag 2007-11-13 13:02) - PLID 27996 - Setup so that we are receiving barcode scan messages
		if(!GetMainFrame()->RegisterForBarcodeScan(this)) {
			MessageBox("Failed to register for barcode messages. You may be unable to use a barcode scanner at this time.", "Inventory Returns", MB_OK);
		}

		// (c.haag 2007-11-06 17:30) - PLID 27994 - Set button colors
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnOK.AutoSet(NXB_OK);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnPTSSPreview.AutoSet(NXB_PRINT_PREV);
		// (c.haag 2008-03-07 12:54) - PLID 29170 - This PL item is now responsible for the task of the following comment
		// (c.haag 2007-11-19 09:35) - PLID 28120 - Set button colors for the print preview
		m_btnPTSSPreview.AutoSet(NXB_PRINT, NXIB_TEXTCOLOR|NXIB_TEXT);

		// (j.jones 2008-09-10 10:15) - PLID 30537 - bulk cache our preferences
		g_propManager.CachePropertiesInBulk("InvEditReturnDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'DefaultSupplierReturnMethod' OR "
			"Name = 'DefaultSupplierReturnReason' OR "
			"Name = 'DefaultSupplierReturnFor' "
			")",
			_Q(GetCurrentUserName()));

		// (c.haag 2007-11-07 09:15) - Bind and requery the locations combo.
		// This is used to filter the item combo, and also associate the return
		// group with a location
		m_dlLocationCombo = BindNxDataList2Ctrl(IDC_LOCATION_RETURN_LIST, true);

		// (c.haag 2007-11-06 16:53) - Bind and requery the supplier
		// datalist. This is used to filter the item combo, and also associate
		// the return group with a single supplier
		m_dlSupplierCombo = BindNxDataList2Ctrl(IDC_SUPPLIER_RETURN_LIST, true);
		
		BOOL bRequeryItemList = FALSE;
		//TES 6/23/2008 - PLID 26152 - If we were given a default supplier, use it.
		if(m_nInitialSupplierID != -1) {
			SetSupplierID(m_nInitialSupplierID);
			bRequeryItemList = TRUE;
		}
		else {
			// (c.haag 2008-02-15 08:42) -  PLID 27994 - Auto-select the first supplier
			_RecordsetPtr prs = CreateRecordset("SELECT TOP 1 PERSONID FROM SupplierT INNER JOIN PersonT ON SupplierT.PersonID = PersonT.ID WHERE Archived = 0 ORDER BY Company");
			if (!prs->eof) {
				SetSupplierID(AdoFldLong(prs, "PersonID"));
				bRequeryItemList = TRUE;
			}
		}

		// (c.haag 2007-11-06 17:08) - The item filter is only used for choosing
		// items to add to the list
		m_dlItemCombo = BindNxDataList2Ctrl(IDC_ITEM_RETURN_LIST, false);

		// (c.haag 2007-11-08 09:03) - The return method list lets a user choose how
		// an item is being returned
		m_dlReturnMethodCombo = BindNxDataList2Ctrl(IDC_RETURN_METHOD_LIST, true);

		// (c.haag 2008-03-03 11:35) - PLID 29176 - This dropdown will act as the default-selection-decider for
		// return reasons when new items are added to the list
		m_dlDefReturnReason = BindNxDataList2Ctrl(IDC_RETURN_REASON_LIST, true);

		// (c.haag 2008-03-03 11:48) - PLID 29176 - This dropdown will act as the default-selection-decider for
		// return for's when new items are added to the list
		// (j.jones 2008-09-10 09:11) - PLID 30537 - if the contents or IDs change for this list,
		// you must change the 'default' selection list in Preferences as well
		m_dlDefReturnFor = BindNxDataList2Ctrl(IDC_RETURN_FOR_LIST, false);
		NXDATALIST2Lib::IRowSettingsPtr pNewRow;
		pNewRow = m_dlDefReturnFor->GetNewRow();
		pNewRow->Value[eclRetFor_ID] = 0L;
		pNewRow->Value[eclRetFor_Name] = "Credit";
		NXDATALIST2Lib::IRowSettingsPtr pRowCredit = m_dlDefReturnFor->AddRowSorted(pNewRow, NULL);
		pNewRow = m_dlDefReturnFor->GetNewRow();
		pNewRow->Value[eclRetFor_ID] = 1L;
		pNewRow->Value[eclRetFor_Name] = "Exchange";
		m_dlDefReturnFor->AddRowSorted(pNewRow, NULL);		

		// (j.jones 2008-09-10 10:22) - PLID 30537 - load our defaults for Return Reason and Return For
		long nDefaultSupplierReturnReason = GetRemotePropertyInt("DefaultSupplierReturnReason", -1, 0, "<None>", false);
		if(nDefaultSupplierReturnReason != -1) {
			//if this fails, OnRequeryFinished will pick the first row
			m_dlDefReturnReason->SetSelByColumn(eclReason_ID, nDefaultSupplierReturnReason);
		}

		long nDefaultSupplierReturnFor = GetRemotePropertyInt("DefaultSupplierReturnFor", 0, 0, "<None>", false);
		if(nDefaultSupplierReturnFor != -1) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlDefReturnFor->SetSelByColumn(eclRetFor_ID, nDefaultSupplierReturnFor);
			if(pRow == NULL) {
				//Default selection is "Credit"
				m_dlDefReturnFor->CurSel = pRowCredit;
			}
		}
		else {
			//Default selection is "Credit"
			m_dlDefReturnFor->CurSel = pRowCredit;
		}

		// (c.haag 2007-11-07 11:17) - The list of returning/returned items
		m_dlReturnItems = BindNxDataList2Ctrl(IDC_LIST, false);
		// Assign embedded combo SQL's through code; otherwise they will not query
		m_dlReturnItems->GetColumn(eclRet_ReasonID)->ComboSource = _bstr_t("SELECT ID, Reason AS Name FROM SupplierReturnReasonT ORDER BY Reason");

		// (c.haag 2007-11-08 09:07) - The date when the return was placed (defaults to today)
		m_nxtReturnDate = GetDlgItemUnknown(IDC_DATE);
		COleDateTime dt = COleDateTime::GetCurrentTime();
		dt.SetDate(dt.GetYear(),dt.GetMonth(),dt.GetDay());
		m_nxtReturnDate->SetDateTime(dt);

		//TES 1/26/2009 - PLID 32824 - Set up confirmation controls.
		m_nxtDateConfirmed = BindNxTimeCtrl(this, IDC_CONFIRMED_DATE);
		m_nxeditConfirmationNumber.SetLimitText(50);

		// (c.haag 2007-11-08 09:58) - Check to see if we are loading an existing group.
		if (ID_NEW_RETURN_GROUP != m_nReturnGroupIDToLoad) {
			// Yes, this is an existing group. Load everything (bearing in mind things are
			// still requerying)
			Load();
		} else {

			// (j.jones 2008-09-10 10:15) - PLID 30537 - load the DefaultSupplierReturnMethod 
			// preference only on new returns
			long nDefaultSupplierReturnMethod = GetRemotePropertyInt("DefaultSupplierReturnMethod", -1, 0, "<None>", false);
			if(nDefaultSupplierReturnMethod != -1) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlReturnMethodCombo->SetSelByColumn(eclMet_ID, nDefaultSupplierReturnMethod);
				if (NULL == pRow) {
					m_nReturnMethodID = -1;	//perhaps we have since deleted the method that used to be our default
				} else {
					m_nReturnMethodID = VarLong(pRow->Value[eclMet_ID]);
				}
			}

			//TES 6/23/2008 - PLID 26152 - If we were given a default location, use it.
			if(m_nInitialLocationID == -1) {
				// This is a new return group. Try to set the desired default location ID ahead of time
				SetLocationID( GetCurrentLocationID() );
			}
			else {
				SetLocationID(m_nInitialLocationID);
			}

			// And then disable the delete button
			GetDlgItem(IDC_BTN_DELETE_RETURN)->EnableWindow(FALSE);

			//TES 1/26/2009 - PLID 32824 - Also make sure the Confirmed controls are all on the same page.
			EnsureConfirmedControls();
		}

		// (c.haag 2008-02-15 09:10) - PLID 27994 - Requery the item list if we have a supplier selected.
		// Don't warn the user if the list is empty here -- it would be very awkward if the first supplier
		// never had anything unreturnable and the user is continually told that every time they open this
		// dialog.
		if (bRequeryItemList) {
			RequeryItemList();
		}

		//TES 6/23/2008 - PLID 26152 - Add any product items we were initialized with.
		for(int i = 0; i < m_arInitialProductItems.GetSize(); i++) {
			ProductItemReturnInfo piri = m_arInitialProductItems[i];
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlReturnItems->GetNewRow();
			SetNewRowDefaults(pNewRow);
			//TES 6/23/2008 - PLID 26152 - SetNewRowDefaults may have failed to set the return reason, because
			// the OnRequeryFinished handler for that datalist won't have been called yet.  So, if it didn't get set,
			// wait for that list to requery (shouldn't be long), and pull the first ID out of it, that's what it would
			// be set to by the OnRequeryFinished() handler anyway.
			if(pNewRow->Value[eclRet_ReasonID].vt == VT_NULL) {
				m_dlDefReturnReason->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
				if(m_dlDefReturnReason->GetRowCount() > 0) {
					pNewRow->Value[eclRet_ReasonID] = m_dlDefReturnReason->GetFirstRow()->Value[eclReason_ID];
				}
			}
			pNewRow->Value[eclRet_ProductID] = piri.nProductID;
			pNewRow->Value[eclRet_ProductItemID] = piri.nProductItemID;
			pNewRow->Value[eclRet_Name] = _bstr_t(piri.strProductName);
			pNewRow->Value[eclRet_SerialNum] = _bstr_t(piri.strSerialNum);
			pNewRow->Value[eclRet_ExpDate] = piri.varExpDate;
			pNewRow->Value[eclRet_Quantity] = 1.0;
			//DRT 7/23/2008 - PLID 30815 - add the last cost as the default amount due
			pNewRow->Value[eclRet_CreditAmt] = piri.varProductLastCost;
			m_dlReturnItems->AddRowSorted(pNewRow, NULL);

			//TES 6/23/2008 - PLID 26152 - If the description is blank update it to the first product.
			if (m_strDescription.IsEmpty()) {
				m_strDescription = piri.strProductName;
				UpdateData(FALSE);
			}
		}

		// (c.haag 2007-11-13 14:37) - PLID 27996 - We're offically initialized
		m_bInitialized = TRUE;

		// (j.jones 2009-02-09 15:11) - PLID 32706 - added a total item count label
		UpdateTotalItemCountLabel();
	}
	NxCatchAllCall("Error in CInvEditReturnDlg::OnInitDialog()", m_bErrorOnInit = TRUE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInvEditReturnDlg::OnOkBtn() 
{
	try {
		CWaitCursor wc;

		// (c.haag 2007-11-07 12:31) - PLID 27994 - Save changes and dismiss the window
		BOOL bAllowClose = TRUE;

		// Update the form-related member variables
		UpdateData(TRUE);

		// Validate the data
		if (Validate(bAllowClose)) {
			// Save changes
			Save();
		} else {
			// Either validation failed, or nothing changed in the dialog
		}

		// Dismiss the window if an exception has not been thrown, and either nothing
		// changed or validation passed and data was saved.
		if (bAllowClose) {
			CDialog::OnOK();
		}

	} NxCatchAll("Error in CInvEditReturnDlg::OnOk()");	
}

void CInvEditReturnDlg::OnCancelBtn() 
{
	try {
		// (c.haag 2007-11-06 17:55) - PLID 27994 - Dismiss the dialog without
		// saving any changes

		// (j.jones 2008-09-08 10:22) - PLID 30538 - warn if cancelling a new return,
		// or an existing one you made changes to
		if(!WarnCancelDialog()) {
			return;
		}

		CDialog::OnCancel();

	} NxCatchAll("Error in CInvEditReturnDlg::OnCancelBtn()");
}

void CInvEditReturnDlg::OnRemoveReturnItem()
{
	try {
		// (c.haag 2007-11-07 12:21) - PLID 27994 - This function is called when an item
		// is removed from the list
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlReturnItems->CurSel;
		if (NULL == pRow) {
			return; // This should never happen
		}

		// If the row was pulled from data, add it to the deleted ID array
		long nReturnItemID = VarLong(pRow->GetValue( eclRet_ReturnItemID ), -1);
		if (-1 != nReturnItemID) {
			// (c.haag 2008-01-09 10:06) - I cannot think of a reason why the ID would already be in the array,
			// but do this extra check to ensure there is no redundant auditing
			if (!IsIDInArray(nReturnItemID, m_anDeletedReturns)) {
				m_anDeletedReturns.Add(nReturnItemID);
			}
		}

		// Remove the row from the list
		m_dlReturnItems->RemoveRow(pRow);

		// (j.jones 2009-02-09 15:11) - PLID 32706 - added a total item count label
		UpdateTotalItemCountLabel();

	} NxCatchAll("Error in CInvEditReturnDlg::OnRemoveReturnItem()");
}

BEGIN_EVENTSINK_MAP(CInvEditReturnDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvEditReturnDlg)
	ON_EVENT(CInvEditReturnDlg, IDC_LOCATION_RETURN_LIST, 18 /* RequeryFinished */, OnRequeryFinishedLocationReturnList, VTS_I2)
	ON_EVENT(CInvEditReturnDlg, IDC_LOCATION_RETURN_LIST, 16 /* SelChosen */, OnSelChosenLocationReturnList, VTS_DISPATCH)
	ON_EVENT(CInvEditReturnDlg, IDC_SUPPLIER_RETURN_LIST, 16 /* SelChosen */, OnSelChosenSupplierReturnList, VTS_DISPATCH)
	ON_EVENT(CInvEditReturnDlg, IDC_ITEM_RETURN_LIST, 16 /* SelChosen */, OnSelChosenItemReturnList, VTS_DISPATCH)
	ON_EVENT(CInvEditReturnDlg, IDC_LIST, 6 /* RButtonDown */, OnRButtonDownList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInvEditReturnDlg, IDC_SUPPLIER_RETURN_LIST, 1 /* SelChanging */, OnSelChangingSupplierReturnList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvEditReturnDlg, IDC_LOCATION_RETURN_LIST, 1 /* SelChanging */, OnSelChangingLocationReturnList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvEditReturnDlg, IDC_LIST, 8 /* EditingStarting */, OnEditingStartingList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CInvEditReturnDlg, IDC_LIST, 9 /* EditingFinishing */, OnEditingFinishingList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CInvEditReturnDlg, IDC_LIST, 10 /* EditingFinished */, OnEditingFinishedList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CInvEditReturnDlg, IDC_ITEM_RETURN_LIST, 18 /* RequeryFinished */, OnRequeryFinishedItemReturnList, VTS_I2)
	ON_EVENT(CInvEditReturnDlg, IDC_SUPPLIER_RETURN_LIST, 18 /* RequeryFinished */, OnRequeryFinishedSupplierReturnList, VTS_I2)
	ON_EVENT(CInvEditReturnDlg, IDC_RETURN_METHOD_LIST, 16 /* SelChosen */, OnSelChosenReturnMethodList, VTS_DISPATCH)
	ON_EVENT(CInvEditReturnDlg, IDC_RETURN_REASON_LIST, 18 /* RequeryFinished */, OnRequeryFinishedReturnReasonList, VTS_I2)
	ON_EVENT(CInvEditReturnDlg, IDC_RETURN_REASON_LIST, 1 /* SelChanging */, OnSelChangingReturnReasonList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvEditReturnDlg, IDC_RETURN_FOR_LIST, 1 /* SelChanging */, OnSelChangingReturnForList, VTS_DISPATCH VTS_PDISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInvEditReturnDlg::OnRequeryFinishedSupplierReturnList(short nFlags) 
{
	try {
		// Ensure we had a completed requery
		if (NXDATALIST2Lib::dlRequeryFinishedCompleted == nFlags) {

			// (c.haag 2007-11-28 08:50) - PLID 27996 - When the requery is done, proceed to requery the
			// item dropdown. It won't have any net effect unless the location dropdown is finished requerying
			RequeryItemList();
		}
		else {
			// If we get here, an error or cancellation occured.
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnRequeryFinishedSupplierReturnList");
}

void CInvEditReturnDlg::OnRequeryFinishedLocationReturnList(short nFlags) 
{
	try {

		// Ensure we had a completed requery
		if (NXDATALIST2Lib::dlRequeryFinishedCompleted == nFlags) {

			// (c.haag 2007-11-07 09:22) - PLID 27994 - Select either the current location, or the
			// first location in the list
			if (m_dlLocationCombo->GetRowCount() > 0) {

				if (NULL == m_dlLocationCombo->CurSel) {
					if (NULL == m_dlLocationCombo->FindByColumn(eclLoc_LocationID, GetCurrentLocationID(), NULL, VARIANT_TRUE)) {
						// We did not find the current location ID in the dropdown. Default to the first row
						m_dlLocationCombo->CurSel = m_dlLocationCombo->GetFirstRow();
					} else {
						// We successfully selected the current location ID
					}
				} else {
					// There's already a current selection. Don't need to do anything
				}

				// Now requery the item list. This will have no effect unless the supplier dropdown
				// has already finished requerying.
				RequeryItemList();

			} else {
				// There are no locations to select!
			}
		}
		else {
			// If we get here, an error or cancellation occured.
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnRequeryFinishedLocationReturnList");
	
}

void CInvEditReturnDlg::OnSelChosenLocationReturnList(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		// (c.haag 2007-11-07 09:31) - PLID 27994 - Requery the item list
		// (c.haag 2007-11-28 08:55) - Also ensure that if the selection resulted
		// in the Item dropdown being cleared out because no items meet the criteria,
		// then the user is warned about it instead of thinking that the dropdown is
		// still requerying when it's not
		m_bWarnIfItemDropdownEmpty = TRUE;
		if (NULL == pRow) {
			m_nLocationID = -1; // This should never happen, but be prepared
		} else {
			m_nLocationID = VarLong(pRow->Value[eclLoc_LocationID]);
		}
		RequeryItemList();
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnSelChosenLocationReturnList");
}

void CInvEditReturnDlg::OnSelChosenSupplierReturnList(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		// (c.haag 2007-11-07 09:32) - PLID 27994 - Requery the item list
		// (c.haag 2007-11-28 08:55) - Also ensure that if the selection resulted
		// in the Item dropdown being cleared out because no items meet the criteria,
		// then the user is warned about it instead of thinking that the dropdown is
		// still requerying when it's not
		m_bWarnIfItemDropdownEmpty = TRUE;
		if (NULL == pRow) {
			m_nSupplierID = -1; // This should never happen, but be prepared
		} else {
			m_nSupplierID = VarLong(pRow->Value[eclSup_PersonID]);
		}
		RequeryItemList();
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnSelChosenSupplierReturnList");
}

void CInvEditReturnDlg::OnSelChosenReturnMethodList(LPDISPATCH lpRow) 
{
	try {
		// (c.haag 2008-01-07 17:47) - PLID 27994 - Update the member variable indicating
		// the return method ID
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (NULL == pRow) {
			m_nReturnMethodID = -1; // This should never happen, but be prepared
		} else {
			m_nReturnMethodID = VarLong(pRow->Value[eclMet_ID]);
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnSelChosenReturnMethodList");
	
}

void CInvEditReturnDlg::OnSelChosenItemReturnList(LPDISPATCH lpRow) 
{
	try {
		// (c.haag 2007-11-07 10:01) - PLID 27994 - Add an item to the return list
		if (NULL == lpRow) {
			// Not sure why this would happen, but lets not do anything
			return;
		}

		// (c.haag 2007-11-07 10:16) - Before we do *anything else*, we must first grasp
		// what kind of item we want to return. Is it serializable, or not?
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		const BOOL bHasSerialNum = VarBool(pRow->GetValue( eclItem_HasSerialNum ), FALSE);
		const BOOL bHasExpDate = VarBool(pRow->GetValue( eclItem_HasExpDate ), FALSE);
		const long nProductID = VarLong(pRow->GetValue( eclItem_ID ));
		//DRT 7/23/2008 - PLID 30815 - Added the cost as the default Amount Due value.
		COleCurrency cyCost = VarCurrency(pRow->GetValue(eclItem_Cost), COleCurrency(0, 0));

		_variant_t vNull;
		vNull.vt = VT_NULL;

		// Update the form-related member variables
		UpdateData(TRUE);

		if (bHasSerialNum || bHasExpDate) {
			// Yes, it has a serial number. Bring up the product items picker dialog. Unlike
			// InvAdj.cpp, we do NOT want the one from CMainFrame, per Don.
			CProductItemsDlg dlg(this);
			CDWordArray& anProductItemIDs = dlg.m_adwProductItemIDs;
			dlg.m_EntryType = PI_SELECT_DATA; // We're just selecting data
			dlg.m_CountOfItemsNeeded = 1;	// We need at least one item
			dlg.m_bUseSerial = bHasSerialNum;	// Using serial numbers
			dlg.m_bUseExpDate = bHasExpDate;	// Using expiration dates
			dlg.m_ProductID = nProductID; // Product ID
			dlg.m_nLocationID = GetLocationID(); // The selection of the location filter should never be different from the item location
			//TES 6/18/2008 - PLID 29578 - Changed from OrderID to OrderDetailID
			dlg.m_nOrderDetailID = -1; // Not associated with an order
			dlg.m_bDisallowQtyChange = FALSE; // Allow the quantity to change
			dlg.m_bAllowQtyGrow = TRUE; // Allow the quantity to grow
			dlg.m_bIsAdjustment = FALSE; // We are not making an adjustment here; we do that when the return is done
			dlg.m_strOverrideTitleBarText = "Select Item(s)"; // Custom title bar text
			dlg.m_strOverrideSelectQtyText = "Quantity to return"; // Custom quantity text
			dlg.m_strOverrideSelectItemText.Format("Please select one or more items for '%s'.",
				VarString(pRow->GetValue( eclItem_Name )));
			// (c.haag 2008-01-07 17:41) - Exclude all items already in the return list
			NXDATALIST2Lib::IRowSettingsPtr pExistingRow = m_dlReturnItems->GetFirstRow();
			CArray<long,long> anExistingIDs;
			while (NULL != pExistingRow) {
				if (VT_NULL != pExistingRow->Value[eclRet_ProductItemID].vt) {
					anExistingIDs.Add( VarLong(pExistingRow->Value[eclRet_ProductItemID]) );
				}
				pExistingRow = pExistingRow->GetNextRow();
			}
			if (anExistingIDs.GetSize() > 0) {
				dlg.m_strWhere += CString("ProductItemsT.ID NOT IN (") + ArrayAsString(anExistingIDs) + ")";
			}

			
			if (IDCANCEL == dlg.DoModal() || 0 == anProductItemIDs.GetSize() /* Should never be zero, but safety first */) {
				// The user elected not to add any serialized items. We're done.
				return;
			} else {
				// The user chose one or more serialized items. Add them to the list.
				const long nProductItems = anProductItemIDs.GetSize();
				for (long i=0; i < nProductItems; i++) {

					// Check to see if the serialized item is already in the list. If so skip it.
					if (NULL != m_dlReturnItems->FindByColumn(eclRet_ProductItemID, (long)anProductItemIDs[i], NULL, VARIANT_FALSE)) {
						// It already exists. Do nothing with intentional silence.
					} else {
						NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlReturnItems->GetNewRow();
						SetNewRowDefaults(pNewRow);
						pNewRow->Value[eclRet_ProductID] = VarLong(pRow->GetValue( eclItem_ID ));
						pNewRow->Value[eclRet_ProductItemID] = (long)anProductItemIDs[i];
						pNewRow->Value[eclRet_Name] = pRow->GetValue( eclItem_Name );
						pNewRow->Value[eclRet_SerialNum] = dlg.GetSelectedProductItemSerialNum( i );
						pNewRow->Value[eclRet_ExpDate] = dlg.GetSelectedProductItemExpDate( i );
						pNewRow->Value[eclRet_Quantity] = 1.0;
						//DRT 7/23/2008 - PLID 30815 - Add the cost as the default amount due
						pNewRow->Value[eclRet_CreditAmt] = _variant_t(cyCost);
						m_dlReturnItems->AddRowSorted(pNewRow, NULL);

						// (c.haag 2007-11-08 09:31) - Update the current description if it's blank
						if (m_strDescription.IsEmpty()) {
							m_strDescription = VarString(pRow->GetValue( eclItem_Name ), "");
						}
					}
				}
			}

		} else {
			// The user chose a non-serialized product. Add it to the list, even if it already exists.
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlReturnItems->GetNewRow();
			SetNewRowDefaults(pNewRow);
			pNewRow->Value[eclRet_ProductID] = nProductID;
			pNewRow->Value[eclRet_ProductItemID] = vNull;
			pNewRow->Value[eclRet_Name] = pRow->GetValue( eclItem_Name );
			pNewRow->Value[eclRet_SerialNum] = vNull;
			pNewRow->Value[eclRet_ExpDate] = vNull;
			pNewRow->Value[eclRet_Quantity] = 1.0;
			//DRT 7/23/2008 - PLID 30815 - Add the cost as the default amount due
			pNewRow->Value[eclRet_CreditAmt] = _variant_t(cyCost);
			m_dlReturnItems->AddRowSorted(pNewRow, NULL);

			// (c.haag 2007-11-08 09:31) - Update the current description if it's blank
			if (m_strDescription.IsEmpty()) {
				m_strDescription = VarString(pRow->GetValue( eclItem_Name ), "");
			}
		}

		// Now update the form fields in case the description was updated
		UpdateData(FALSE);

		// (j.jones 2009-02-09 15:11) - PLID 32706 - added a total item count label
		UpdateTotalItemCountLabel();
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnSelChosenItemReturnList");
}

void CInvEditReturnDlg::OnRButtonDownList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		// (c.haag 2007-11-07 12:19) - PLID 27994 - This event is fired when the user right-clicks on the list
		if (NULL == lpRow) {
			return; // Return if an actual item was not clicked on
		}

		// Ensure the row is selected
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		m_dlReturnItems->CurSel = pRow;

		// Invoke the right click pop-up menu
		CMenu menu;
		CPoint pt;
		GetCursorPos(&pt);
		menu.m_hMenu = CreatePopupMenu();
		menu.InsertMenu(-1, MF_BYPOSITION, ID_REMOVE_RETURN_ITEM, "&Remove item");
		menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		menu.DestroyMenu();
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnRButtonDownList");
}

void CInvEditReturnDlg::OnSelChangingSupplierReturnList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		// (c.haag 2007-11-08 12:31) - PLID 27994 - Called when the supplier selection is changing
		if (lpOldSel == *lppNewSel) {
			// If the selections match, do nothing
		}
		else if (NULL == *lppNewSel) {
			// Undo NULL selections (not that it matters if lpOldSel is NULL though)
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		} 
		else if (m_dlReturnItems->GetRowCount() > 0) {
			if (IDYES == MessageBox("You have already elected to return items from the "
				"previously selected supplier. If you continue, all items will be "
				"removed from this group.\n\n"
				"Do you wish to clear out your existing returns and select a new supplier?",
				"Inventory Returns",
				MB_YESNO | MB_ICONEXCLAMATION))
			{
				// The user wants to commit the supplier change. Remove all rows from the list
				ClearReturnList();
				// (c.haag 2007-11-13 15:50) - PLID 27994 - Because we invoked a message box, 
				// OnSelChosen... is not going to be called because CListWnd::OnCaptureChanged
				// won't be called after this event is handled. That means we need to set the
				// selection now and do a delayed requery of the item combo.
				OnSelChosenSupplierReturnList(*lppNewSel);
				SetTimer(IDT_REQUERY_ITEM_COMBO, 1, NULL);
			}
			else {
				// The user changed their mind
				SafeSetCOMPointer(lppNewSel, lpOldSel);
			}
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnSelChangingSupplierReturnList");
}

void CInvEditReturnDlg::OnSelChangingLocationReturnList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		// (c.haag 2007-11-08 12:41) - PLID 27994 - Called when the location selection is changing
		if (lpOldSel == *lppNewSel) {
			// If the selections match, do nothing
		}
		else if (NULL == *lppNewSel) {
			// Undo NULL selections (not that it matters if lpOldSel is NULL though)
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		} 
		else if (m_dlReturnItems->GetRowCount() > 0) {
			if (IDYES == MessageBox("You have already elected to return items from the "
				"previously selected location. If you continue, all items will be "
				"removed from this group.\n\n"
				"Do you wish to clear out your existing returns and select a new location?",
				"Inventory Returns",
				MB_YESNO | MB_ICONEXCLAMATION))
			{
				// The user wants to commit the supplier change. Remove all rows from the list
				ClearReturnList();
				// (c.haag 2007-11-13 15:50) - PLID 27994 - Because we invoked a message box, 
				// OnSelChosen... is not going to be called because CListWnd::OnCaptureChanged
				// won't be called after this event is handled. That means we need to set the
				// selection now and do a delayed requery of the item combo.
				OnSelChosenLocationReturnList(*lppNewSel);
				SetTimer(IDT_REQUERY_ITEM_COMBO, 1, NULL);
			}
			else {
				// The user changed their mind
				SafeSetCOMPointer(lppNewSel, lpOldSel);
			}
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnSelChangingLocationReturnList");
}

void CInvEditReturnDlg::OnBtnDeleteReturn() 
{
	try {
		// (c.haag 2007-11-08 16:01) - PLID 27994 - This function is called when a user wants to
		// delete a return
		ASSERT(ID_NEW_RETURN_GROUP != m_nID);

		// (c.haag 2007-11-13 17:39) - PLID 28036 - Check permissions
		if(!CheckCurrentUserPermissions(bioInvSupplierReturn, sptDelete)) {
			return;
		}
		// Delete the group and quit out of the dialog
		if (!InvUtils::DeleteReturnGroup(m_nID, TRUE /* Ask the user if they're sure */, this /* Message boxes appear above this window */)) {
			// The user changed their mind. Don't dismiss the window.
			return;
		}
		CDialog::OnOK();
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnBtnDeleteReturn");
}

void CInvEditReturnDlg::OnEditingStartingList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		// (c.haag 2007-11-09 09:40) - PLID 27994 - The purpose of handling this event is to prevent users
		// from editing the quantity of serialized items. For consistent with consignment code, the behavior
		// will be totally silent
		if (NULL == lpRow || eclRet_Quantity != nCol) {
			return; // Ignore non-rows or anything not related to quantity
		}
	
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (VT_NULL != pRow->GetValue(eclRet_ProductItemID).vt) {
			// If this row has a valid product item ID, then it must be a serialized item. Therefore,
			// don't allow the user to edit the quantity.
			*pbContinue = FALSE;
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnEditingStartingList");
}

void CInvEditReturnDlg::OnEditingFinishingList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		// (c.haag 2007-11-09 13:10) - PLID 27994 - This event is fired when the user is completing a list edit
		if (*pbCommit == FALSE) {
			return; // User hit escape
		} else if (NULL == lpRow) {
			return; // Should never happen
		}

		// Quantity validation
		if (eclRet_Quantity == nCol) {
			if (VarDouble(*pvarNewValue, 0) <= 0) {
				// We don't allow non-positive quantities
				*pbCommit = FALSE;
			}
		}
		// Credit validation
		else if (eclRet_CreditAmt == nCol) {
			if (VarCurrency(*pvarNewValue, COleCurrency(0,0)) < COleCurrency(0,0)) {
				// We don't allow negative credits
				*pbCommit = FALSE;
			}
		}
		// Amount received validation
		else if (eclRet_AmountReceived == nCol) {
			if (VarCurrency(*pvarNewValue, COleCurrency(0,0)) < COleCurrency(0,0)) {
				// We don't allow negative received amounts
				*pbCommit = FALSE;
			}
		}
		// Completed Date
		else if (eclRet_CompletedDate == nCol) {
			if (VT_DATE == pvarNewValue->vt && pvarNewValue->date <= 0) {
				// Rather than letting them enter 12/30/1899 or some other absured date,
				// just null the value
				_variant_t vNull;
				vNull.vt = VT_NULL;
				*pvarNewValue = vNull;
			}
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnEditingFinishingList");
}

void CInvEditReturnDlg::OnBtnConfigureReturnReasons() 
{
	try {

		long nOldReturnReason = -1;
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlDefReturnReason->GetCurSel();
			if(pRow != NULL) {
				nOldReturnReason = VarLong(pRow->GetValue(eclReason_ID), -1);
			}
		}

		// (c.haag 2007-11-12 08:28) - PLID 28010 - Allow users to configure return reasons
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		//#define INVEDITRETURN_REASON_COMBO	60
		CEditComboBox(this, 60, "Edit Return Reasons").DoModal();

		// Now we need to find all of the return item rows that have deleted return reasons, and reset
		// those reasons to NULL's.
		if (m_dlReturnItems->GetRowCount() > 0) {

			// Gather a list of all the return ID's we have in use
			CArray<long,long> anReturnIDs;
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlReturnItems->GetFirstRow();
			while (NULL != pRow) {
				_variant_t v = pRow->GetValue(eclRet_ReasonID);
				if (VT_NULL != v.vt) {
					BOOL bFound = FALSE;
					long nReasonID = VarLong(v);
					int i;

					// Add nReasonID to the array if it doesn't exist
					for (i=0; i < anReturnIDs.GetSize() && !bFound; i++) {
						if (anReturnIDs[i] == nReasonID) {
							// Found it
							bFound = TRUE;
						} else {
							// Keep looking
						}
					}
					if (!bFound) {
						anReturnIDs.Add(VarLong(v));
					} else {
						// The ID already exists in the array. Don't duplicate it.
					}
				}
				pRow = pRow->GetNextRow();
			}

			// Now anReturnIDs contains a unique list of all return ID's in use by the current list. If it's not
			// empty (it can be if a user added items but did not designate return reasons), then we must figure out
			// which ones were deleted from data, and assign NULL values to them
			if (anReturnIDs.GetSize() > 0) {

				// If we get here, we have reason ID's in use. Run the query to find the ones which are actually in data.
				// Don't use a param recordset since we're using a generated string.
				_RecordsetPtr prs = CreateRecordset("SELECT ID FROM SupplierReturnReasonT WHERE ID IN (%s)", ArrayAsString(anReturnIDs));
				FieldsPtr f = prs->Fields;
				int i;
				while (!prs->eof) {
					long nExistingID = AdoFldLong(f, "ID");
					BOOL bFound = FALSE;

					// Remove the existing ID from the array if it's in data
					for (i=0; i < anReturnIDs.GetSize() && !bFound; i++) {
						if (anReturnIDs[i] == nExistingID) {
							// Found it. Now remove it.
							anReturnIDs.RemoveAt(i);
							bFound = TRUE;
						} else {
							// Keep looking
						}
					}

					prs->MoveNext();
				}


				// Now anReturnIDs contains a list of ID's that no longer exist in data. Go through the array,
				// and for each ID, nullify it in the list
				_variant_t vNull;
				vNull.vt = VT_NULL;
				for (i=0; i < anReturnIDs.GetSize(); i++) {
					while (NULL != (pRow = m_dlReturnItems->FindByColumn(eclRet_ReasonID, anReturnIDs[i], NULL, VARIANT_FALSE))) {
						pRow->Value[ eclRet_ReasonID ] = vNull;
					}
				} // for (i=0; i < anReturnIDs.GetSize(); i++) {
			} // if (anReturnIDs.GetSize() > 0) {
		} // if (m_dlReturnItems->GetRowCount() > 0) {

		// Now requery the embedded reason dropdown
		m_dlReturnItems->GetColumn(eclRet_ReasonID)->ComboSource = _bstr_t("SELECT ID, Reason AS Name FROM SupplierReturnReasonT ORDER BY Reason");

		// (c.haag 2008-03-03 12:46) - PLID 29176 - Also requery the default dropdown
		m_dlDefReturnReason->Requery();

		if(nOldReturnReason != -1) {
			m_dlDefReturnReason->SetSelByColumn(eclReason_ID, nOldReturnReason);
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnBtnConfigureReturnReasons");
}

void CInvEditReturnDlg::OnBtnConfigureReturnMethods() 
{
	try {
		// (c.haag 2007-11-12 08:28) - PLID 28010 - Allow users to configure methods of return
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		// #define INVEDITRETURN_METHOD_COMBO	61
		long nMethodID = GetReturnMethodID();
		CEditComboBox(this, 61, m_dlReturnMethodCombo, "Edit Return Methods").DoModal();

		// When the dialog is dismissed, the method combo is requeried automatically.
		// We must re-establish the old selection if it still exists.
		m_dlReturnMethodCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		if (-1 != nMethodID) {
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_dlReturnMethodCombo->TrySetSelByColumn_Deprecated(eclMet_ID, nMethodID);
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnBtnConfigureReturnMethods");
}

BOOL CInvEditReturnDlg::EnsureItemFilterCurrentForBarcodeScan(long nProductID)
{
	// (c.haag 2008-01-07 11:56) - PLID 27996 - If a barcode is scanned, this function will warn
	// the user if the supplier dropdown selection is inconsistent with the product's supplier,
	// and resolve the inconsistency by selecting the product and clearing the list
	//
	// If a product has no suppliers, then this function will return FALSE.
	// If a product has one or more suppliers, and none match the currently selected supplier, 
	//		this function will let the user either change the selected supplier and return TRUE
	//		if they do, or FALSE if they cancel out
	// If a product has a supplier that matches the currently selected supplier, this function
	//		return TRUE
	//
	CArray<long,long> anSupplierIDs;
	const long nCurrentSupplierID = GetSupplierID();
	long nDefaultSupplierID = -1;

	// Read in the supplier ID's for the given product
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT SupplierID FROM MultiSupplierT WHERE ID IN (SELECT DefaultMultiSupplierID FROM ProductT WHERE ID = {INT})\r\n"
		// Order by ID so that we know the first supplier is always the eldest
		"SELECT SupplierID FROM MultiSupplierT WHERE ProductID = {INT} ORDER BY ID ",
		nProductID, nProductID);
	if (!prs->eof) {
		nDefaultSupplierID = AdoFldLong(prs, "SupplierID");
	}
	prs = prs->NextRecordset(NULL);
	while (!prs->eof) {
		anSupplierIDs.Add( AdoFldLong(prs, "SupplierID") );
		prs->MoveNext();
	}
	prs->Close();

	// Check if the product has no suppliers. If that is the case, we fail.
	if (0 == anSupplierIDs.GetSize()) {
		return FALSE;
	}

	// If any of the supplier ID's match our current supplier ID, we don't need to
	// change anything.
	int i;
	for (i=0; i < anSupplierIDs.GetSize(); i++) {
		if (anSupplierIDs[i] == nCurrentSupplierID) {
			return TRUE;
		}
	}

	// If we get here, the product's supplier list is mutually exclusive from our currently
	// selected supplier. Warn the user that the only way to proceed is to clear the list if
	// anything exists, and then change the supplier selection.
	if (m_dlReturnItems->GetRowCount() > 0) {
		if (IDYES == MessageBox(
			"The barcode you have scanned corresponds to an item that exists for a "
			"supplier or location different from the ones selected. If you continue, the active "
			"supplier and location will be updated, and all return items will be removed from this "
			"group\n\n"
			"Do you wish to clear out your existing returns and add the scanned item to the return list?",
			"Inventory Returns",
			MB_YESNO | MB_ICONEXCLAMATION))
		{
			// Yes, proceed with clearing out the list
			ClearReturnList();
		} 
		else {
			// Don't clear the list. Abort.
			return FALSE;
		}
	}
	// Update the supplier list selection and requery the item list. If there is no default
	// supplier, then pick the first one in the list. Because we did ORDER BY ID earlier, we
	// know it will be the first supplier entered in by the user.
	if (-1 == nDefaultSupplierID) {
		SetSupplierID(anSupplierIDs[0]);
	} else {
		SetSupplierID(nDefaultSupplierID);
	}
	RequeryItemList();

	// We are now certain that the item filter is consistent
	return TRUE;
}

// (c.haag 2007-11-13 10:31) - PLID 27996 - Prevent multiple scans at one time with this fake mutex
class CInvReturnScanInProgress
{
private:
	CInvEditReturnDlg& m_dlgReturn;

public:
	CInvReturnScanInProgress(CInvEditReturnDlg& dlg) : m_dlgReturn(dlg)
	{
	  // Flag the fact that we are scanning
	  m_dlgReturn.m_bIsScanning = TRUE;
	}
	virtual ~CInvReturnScanInProgress() {
	  // Flag the fact that we are done scanning
	  m_dlgReturn.m_bIsScanning = FALSE;
	}
};

LRESULT CInvEditReturnDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	// Quit if we're still initializing
	if (!m_bInitialized) {
		return 0;
	}

	// Quit if we are scanning
	if (m_bIsScanning) {
		return 0;
	}

	CInvReturnScanInProgress sip(*this);
	try {
		// (c.haag 2007-12-17 15:54) - PLID 27996 - Handle barcode scans
		_bstr_t bstr = (BSTR)lParam;
		_variant_t vBarcode(bstr);
		CString strBarcode(VarString(vBarcode));
		CWaitCursor wc;

		// Don't think this should ever happen, but check anyway
		if (strBarcode.IsEmpty()) {
			return 0;
		}

		// (a.walling 2008-02-21 13:50) - PLID 29053 - Set tracking number if it has focus
		CNxEdit* pTrackingEditWnd = (CNxEdit*)GetDlgItem(IDC_TRACKING_NUMBER);
		CWnd* pFocusWnd = GetFocus();
		if (pFocusWnd == pTrackingEditWnd || pFocusWnd->GetSafeHwnd() == pTrackingEditWnd->GetSafeHwnd()) {
			pTrackingEditWnd->SetSel(0, -1);
			// using ReplaceSel allows Undo to still function.
			pTrackingEditWnd->ReplaceSel((LPCTSTR)bstr, TRUE);
			return 0;
		}

		// Check our products dropdown for the barcode, if we find it, there's no need to access data.
		//(c.copits 2010-09-21) PLID 40317 - Allow duplicate UPC codes for FramesData certification.
		//NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlItemCombo->FindByColumn(eclItem_Barcode, vBarcode, NULL, VARIANT_TRUE);
		NXDATALIST2Lib::IRowSettingsPtr pRow = GetBestUPCProductFromItemReturnList(vBarcode);
		if (NULL != pRow) {
			// We found the barcode in the product datalist. We've already auto-selected it; fire the
			// selection chosen event and we're done.
			OnSelChosenItemReturnList(pRow);
			return 0;
		}

		// We did not find the product. So, the possibilities are that it
		// is a barcode for a product that doesn't exist in the items dropdown,
		// it's a barcode for a serialized item, or it's an unrecognized barcode.
		// In any event, we'll have to search data. Note how we do not filter on
		// location or supplier; we will confront the user if they differ from our
		// current selections.

		//TES 7/7/2008 - PLID 24726 - Pass in the product items that we've already added to the return to 
		// Barcode_CheckExistenceofSerialNumber(), so it knows not to return any of them.
		CArray<long,long> arSelectedProductItemIDs;
		NXDATALIST2Lib::IRowSettingsPtr pExistingRow = m_dlReturnItems->GetFirstRow();
		while(pExistingRow) {
			// (j.jones 2009-01-26 12:23) - PLID 32855 - this didn't check for NULL, so safely
			// check for it now, using the same logic we use in the rest of this dialog
			if (VT_NULL != pExistingRow->Value[eclRet_ProductItemID].vt) {
				arSelectedProductItemIDs.Add(VarLong(pExistingRow->GetValue(eclRet_ProductItemID)));
			}
			pExistingRow = pExistingRow->GetNextRow();
		}

		CString strProductName;
		_variant_t vExpDate;
		long nProductID = -1;
		long nProductItemID = -1;
		InvUtils::Barcode_CheckExistenceOfSerialNumberResultEx info;
		// Use the utility function to find a qualifying product item ID
		if (!InvUtils::Barcode_CheckExistenceOfSerialNumber(
			// Inputs
			strBarcode,			// Scanned barcode
			GetLocationID(),	// Use our selected location. We only return from one location at a time.
			FALSE,				// Verbose results (if it fails, pop up saying why it did)
			// Outputs
			nProductItemID,
			nProductID,
			strProductName,
			vExpDate, 
			&info,
			FALSE, FALSE, TRUE, TRUE, //Defaults
			&arSelectedProductItemIDs //The product items already on the return
			))
		{
			// If we get here, we did not find an available serialized item to return. This
			// is for one of two reasons -- either the barcode simply did not exist in ProductItemsT.SerialNum;
			// or it does/did exist, but it's unavailable. If the latter is true, then nProductItemID
			// is a valid value, and the user has, by now, been confronted with a pop-up saying
			// why the item exists but cannot be returned. Check for this case, and return from
			// here if it's true.
			if (-1 != nProductItemID) {
				// If we get here, the utility found a product item in data, but it cannot be
				// returned because it's in use, adjusted, returned, or for some other reason.
				return 0;
			}

			// If we get here, we could not find any matches by product item. Try searching by product.
			// Before you bring out your torches and pitchforks regarding the simple filtering of this
			// query, keep in mind that the intelligent filtering, based on existing allocations and
			// other factors, is done in CProductItemsDlg.
			//(c.copits 2010-09-21) PLID 40317 - Allow duplicate UPC codes for FramesData certification
			//_RecordsetPtr prsProducts = CreateParamRecordset("SELECT ProductT.ID, ServiceT.Name FROM ProductT "
			//	"INNER JOIN ServiceT ON ServiceT.ID = ProductT.ID "
			//	"WHERE ServiceT.Barcode = {STRING} AND ServiceT.Active = 1 "
			//	"AND (ProductT.HasSerialNum <> 0 OR ProductT.HasExpDate <> 0) ", strBarcode);
			_RecordsetPtr prsProducts = GetBestUPCProductFromData(strBarcode);
			if (!prsProducts->eof) {
				// We found a match. Bring up the product item picker.
				nProductID = AdoFldLong(prsProducts, "ID");
				strProductName = AdoFldString(prsProducts, "Name");
				prsProducts->Close();

				CProductItemsDlg dlg(this);
				CDWordArray& anProductItemIDs = dlg.m_adwProductItemIDs;
				dlg.m_EntryType = PI_SELECT_DATA; // We're just selecting data
				dlg.m_CountOfItemsNeeded = 1;	// We need at least one item
				dlg.m_bUseSerial = info.bProductHasSerialNum;	// Using serial numbers
				dlg.m_bUseExpDate = info.bProductHasExpDate;	// Using expiration dates
				dlg.m_ProductID = nProductID; // Product ID
				dlg.m_nLocationID = GetLocationID(); // Location ID
				//TES 6/18/2008 - PLID 29578 - Changed from OrderID to OrderDetailID
				dlg.m_nOrderDetailID = -1; // Not associated with an order
				dlg.m_bDisallowQtyChange = FALSE; // Allow the quantity to change
				dlg.m_bAllowQtyGrow = TRUE; // Allow the quantity to grow
				dlg.m_bIsAdjustment = FALSE; // We are not making an adjustment here
				dlg.m_strOverrideTitleBarText = "Select Item(s)"; // Custom title bar text
				dlg.m_strOverrideSelectQtyText = "Quantity to return"; // Custom quantity text
				dlg.m_strOverrideSelectItemText.Format("Please select one or more items for '%s'.",
					strProductName);
				// (c.haag 2008-01-07 11:51) - Exclude all items already in the return list
				NXDATALIST2Lib::IRowSettingsPtr pExistingRow = m_dlReturnItems->GetFirstRow();
				CArray<long,long> anExistingIDs;
				while (NULL != pExistingRow) {
					if (VT_NULL != pExistingRow->Value[eclRet_ProductItemID].vt) {
						anExistingIDs.Add( VarLong(pExistingRow->Value[eclRet_ProductItemID]) );
					}
					pExistingRow = pExistingRow->GetNextRow();
				}
				if (anExistingIDs.GetSize() > 0) {
					dlg.m_strWhere += CString("ProductItemsT.ID NOT IN (") + ArrayAsString(anExistingIDs) + ")";
				}


				if (IDCANCEL == dlg.DoModal() || 0 == anProductItemIDs.GetSize()) {
					// The user elected not to add any product items. We're done.
					return 0;
				} else {
					// The user chose one or more product items. Add them to the list...but
					// before we commit the change, ensure that the supplier is consistent 
					// with that of this return group						
					if (!EnsureItemFilterCurrentForBarcodeScan(nProductID)) {
						// If we get here, they either could not be made consistent, or the
						// user is unwilling to make them consistent
						return 0;
					}

					const long nProductItems = anProductItemIDs.GetSize();
					_RecordsetPtr prsProductItems = CreateRecordset(
						"SELECT ProductItemsT.ID, ProductItemsT.SerialNum, ExpDate, ProductT.LastCost "
						"FROM ProductItemsT "
						"LEFT JOIN LocationsT ON LocationsT.ID = ProductItemsT.LocationID "
						"LEFT JOIN ProductT ON ProductT.ID = ProductItemsT.ProductID "
						"WHERE ProductItemsT.ID IN (%s)", ArrayAsString(anProductItemIDs));
					FieldsPtr f = prsProductItems->Fields;
					while (!prsProductItems->eof) {

						// Check to see if the serialized item is already in the list. If so skip it. This
						// really should never happen since we filtered them out in the where clause to begin
						// with, but I'm going to leave this layer of redundancy here nonetheless.
						if (NULL != m_dlReturnItems->FindByColumn(eclRet_ProductItemID, AdoFldLong(f, "ID"), NULL, VARIANT_FALSE)) {
							// It already exists. Do nothing with intentional silence.
						} else {
							NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlReturnItems->GetNewRow();
							SetNewRowDefaults(pNewRow);
							pNewRow->Value[eclRet_ProductID] = nProductID;
							pNewRow->Value[eclRet_ProductItemID] = f->Item["ID"]->Value;
							pNewRow->Value[eclRet_Name] = _bstr_t(strProductName);
							pNewRow->Value[eclRet_SerialNum] = f->Item["SerialNum"]->Value;
							pNewRow->Value[eclRet_ExpDate] = f->Item["ExpDate"]->Value;
							pNewRow->Value[eclRet_Quantity] = 1.0;
							//DRT 7/23/2008 - PLID 30815 - Add cost as the default amount due
							pNewRow->Value[eclRet_CreditAmt] = f->Item["LastCost"]->Value;
							m_dlReturnItems->AddRowSorted(pNewRow, NULL);

							// Update the current description if it's blank
							if (m_strDescription.IsEmpty()) {
								m_strDescription = strProductName;
							}
						}
						prsProductItems->MoveNext();
					}

					// (j.jones 2009-02-09 15:11) - PLID 32706 - added a total item count label
					UpdateTotalItemCountLabel();
				}

			} // if (!prs->eof) {
			else 
			{
				// We could not find any matches for either products or product items in data.
				// Silently fail out.
				return 0;
			}


		} // if (!InvUtils::Barcode_CheckExistenceOfSerialNumber(...
		else 
		{
			// If we get here, we found a returnable product item in data. Check to see if the
			// serialized item is already in the list. If so skip it.
			//
			// We intentionally do this before EnsureItemFilterCurrentForBarcodeScan. It's
			// entirely possible that the same barcode exist for two different product items;
			// but having the barcode already exist in the list trumps any other check or filter.
			//
			if (NULL != m_dlReturnItems->FindByColumn(eclRet_ProductItemID, nProductItemID, NULL, VARIANT_FALSE)) {
				// It already exists. Do nothing with intentional silence.
				return 0;
			} else {

				// Before we commit the change, ensure that the product supplier is consistent
				// with those of this return group						
				if (!EnsureItemFilterCurrentForBarcodeScan(nProductID)) {
					// If we get here, they either could not be made consistent, or the
					// user is unwilling to make them consistent
					return 0;
				}

				NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_dlReturnItems->GetNewRow();
				SetNewRowDefaults(pNewRow);
				pNewRow->Value[eclRet_ProductID] = nProductID;
				pNewRow->Value[eclRet_ProductItemID] = nProductItemID;
				pNewRow->Value[eclRet_Name] = _bstr_t(strProductName);
				pNewRow->Value[eclRet_SerialNum] = vBarcode;
				pNewRow->Value[eclRet_ExpDate] = vExpDate;
				pNewRow->Value[eclRet_Quantity] = 1.0;
				//DRT 7/23/2008 - PLID 30815 - Add cost as the default amount due
				pNewRow->Value[eclRet_CreditAmt] = info.varProductLastCost;
				m_dlReturnItems->AddRowSorted(pNewRow, NULL);

				// Update the current description if it's blank
				if (m_strDescription.IsEmpty()) {
					m_strDescription = strProductName;
				}

				// (j.jones 2009-02-09 15:11) - PLID 32706 - added a total item count label
				UpdateTotalItemCountLabel();
			}
		}
		//TES 6/23/2008 - PLID 30476 - Now update the form fields in case the description was updated (just setting 
		// m_strDescription won't cut it.
		UpdateData(FALSE);
	
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnBarcodeScan");
	return 0;
}

void CInvEditReturnDlg::OnDestroy() 
{
	try {
		CNxDialog::OnDestroy();
	
		// (c.haag 2007-11-13 13:05) - PLID 27996 - No longer receive barcode scan messages
		GetMainFrame()->UnregisterForBarcodeScan(this);
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnDestroy");
}

void CInvEditReturnDlg::OnTimer(UINT nIDEvent) 
{
	try {
		// (c.haag 2007-11-13 15:54) - PLID 27994 - When this timer expires, we requery the
		// item dropdown
		if (IDT_REQUERY_ITEM_COMBO == nIDEvent) {
			KillTimer(nIDEvent);
			RequeryItemList();
		}
		CNxDialog::OnTimer(nIDEvent);
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnTimer");
}

void CInvEditReturnDlg::OnEditingFinishedList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		// (c.haag 2007-11-15 10:20) - PLID 27994 - Make it so if an item is marked as completed,
		// then set the completed date to today's date
		if (NULL == lpRow) {
			return; // This should never happen
		}

		if (eclRet_Completed == nCol) {
			if (VarBool(varNewValue)) {
				// If we get here, the user checked the completion box in the affirmative. Update
				// the completed date if we have none
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				if (VT_NULL == pRow->GetValue(eclRet_CompletedDate).vt) {
					COleDateTime dtToday = COleDateTime::GetCurrentTime();
					dtToday.SetDate( dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay() );
					pRow->Value[ eclRet_CompletedDate ] = _variant_t(dtToday, VT_DATE);
				}
			}
			else {
				// If we get here, the user checked the completion box in the negative. Remove
				// the completed date
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				_variant_t vNull;
				vNull.vt = VT_NULL;
				pRow->Value[ eclRet_CompletedDate ] = vNull;				
			}
		}
		// (j.jones 2009-02-09 15:11) - PLID 32706 - added a total item count label
		else if(nCol == eclRet_Quantity) {
			UpdateTotalItemCountLabel();
		}
		
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnEditingFinishedList");
}

void CInvEditReturnDlg::OnPTSSPreview()
{
	try {
		// (c.haag 2008-03-07 12:08) - PLID 29170 - We no longer use the Allergan report; we now use the generic return report
		// (c.haag 2007-11-16 17:18) - PLID 28120 - Run the Allergan Product
		// Transfer Summary Sheet.

		// Update the form-related member variables
		UpdateData(TRUE);

		// (c.haag 2008-03-07 12:08) - PLID 29170 - Since we're no longer producing a return form,
		// we now include both complete and incomplete items
		// (c.haag 2008-03-06 16:02) - This report will only display incomplete items. Go through the list to check whether
		// they are all complete, and if so, don't run the report because it will be blank.
		/*NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlReturnItems->GetFirstRow();
		BOOL bExistIncompleteItems = FALSE;
		while (NULL != pRow && !bExistIncompleteItems) {
			if (!VarBool(pRow->GetValue( eclRet_Completed ))) {
				bExistIncompleteItems = TRUE;
			}
			pRow = pRow->GetNextRow();
		}

		if (!bExistIncompleteItems) {
			MessageBox("There are no incomplete line items in this return. Because the report includes only incomplete line items, "
				"the report cannot be run.", "Inventory Returns", MB_ICONSTOP | MB_OK);
			return;
		}*/

		// Warn the user if we need to save
		BOOL bWillAlterExistingProductAdjustments = FALSE;
		if (HasDialogDataChanged(bWillAlterExistingProductAdjustments)) {
			if (IDCANCEL == MessageBox("Before generating the report, this return group will be saved and closed.", "Inventory Returns", MB_ICONINFORMATION | MB_OKCANCEL)) {
				return;
			}

			// Do the save
			CWaitCursor wc;
			BOOL bAllowContinue; // Dummy variable

			// Validate the data
			if (Validate(bAllowContinue)) {
				// Save changes
				Save();
			} else {
				// We know data changed (otherwise we wouldn't be trying to save), so
				// we must conclude that the validation failed!
				return;
			}
		}

		// When we get here, the dialog data has been saved, and we're ready to generate
		// the report. Dismiss the dialog and generate the report.
		EndDialog(IDOK);


		CWaitCursor wc;
		// (c.haag 2008-03-07 12:09) - PLID 29170 - We've depreciated the Allergan report and we're
		// now using the generic return report
		/*CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(617)]);
		infReport.nExtraID = m_nID;
		RunReport(&infReport, true, this, "Allergan Product Transfer Summary Sheet");*/

		CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(616)]);
		CPtrArray paramList;

		// Add our date parameters as the defaults. This is necessary, and also consistent with the
		// inventory view class.
		CRParameterInfo *pFilter = new CRParameterInfo;
		pFilter->m_Name = "DateFrom";
		pFilter->m_Data = "01/01/1000";
		paramList.Add((void *)pFilter);

		pFilter = new CRParameterInfo;
		pFilter->m_Name = "DateTo";
		pFilter->m_Data = "12/31/5000";
		paramList.Add((void *)pFilter);

		infReport.nExtraID = m_nID;

		RunReport(&infReport, &paramList, true, this, "Supplier Returns");
		ClearRPIParameterList(&paramList);
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnPTSSPreview");
}

void CInvEditReturnDlg::OnRequeryFinishedItemReturnList(short nFlags) 
{
	try {
		// (c.haag 2007-11-27 15:44) - PLID 27996 - When the item combo has
		// requeried, and it has no items in it, warn the user

		// Ensure we had a completed requery
		if (NXDATALIST2Lib::dlRequeryFinishedCompleted == nFlags) {

			// Check if we have no results
			if (0 == m_dlItemCombo->GetRowCount()) {

				// We have no results. If we need to warn the user, do so now
				if (m_bWarnIfItemDropdownEmpty) {

					if (NULL != m_dlSupplierCombo->CurSel && NULL != m_dlLocationCombo->CurSel) {
						CString strSupplier = VarString(m_dlSupplierCombo->CurSel->GetValue( eclSup_Company ), "");
						CString strLocation = VarString(m_dlLocationCombo->CurSel->GetValue( eclLoc_Name ), "");
						MessageBox(
							FormatString("There are no on-hand items available for the supplier '%s' at the location '%s'.", strSupplier, strLocation),
							"Inventory Returns", MB_ICONINFORMATION | MB_OK);
					} else {
						// This should never happen because the item list filters on supplier and location
					}
				}
				else {
					// If we get here, we don't want to warn the user. This is because it would be awkward for the message
					// box to tell the user that the Item dropdown is empty if the user hadn't actually selected anything yet.
				}
			}
			else {
				// There is now at least one item in the Item dropdown; so there's no need for a warning
			}
		}
		else {
			// If we get here, an error or cancellation occured. Don't throw any warnings.
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnRequeryFinishedItemReturnList");
}

void CInvEditReturnDlg::OnRequeryFinishedReturnReasonList(short nFlags) 
{
	try {
		// (c.haag 2008-03-03 11:59) - PLID 29176 - This is called when the Return Reason list is finished requerying.
		// All we do here is try to select the first item in the list
		if (m_dlDefReturnReason->GetRowCount() > 0 && m_dlDefReturnReason->CurSel == NULL) {
			m_dlDefReturnReason->CurSel = m_dlDefReturnReason->GetFirstRow();
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnRequeryFinishedReturnReasonList");
}

void CInvEditReturnDlg::OnSelChangingReturnReasonList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		// (c.haag 2008-03-03 12:58) - PLID 29176 - Ensure the user cannot make a non-selection
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnSelChangingReturnReasonList");
}

void CInvEditReturnDlg::OnSelChangingReturnForList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		// (c.haag 2008-03-03 12:58) - PLID 29176 - Ensure the user cannot make a non-selection
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll("Error in CInvEditReturnDlg::OnSelChangingReturnForList");
}

//DRT 6/2/2008 - PLID 30230 - Added OnCancel handler to keep behavior the same as pre-NxDialog changes
void CInvEditReturnDlg::OnCancel()
{
	//Eat the message
}

void CInvEditReturnDlg::OnOK()
{
	//Eat the message
	
}

void CInvEditReturnDlg::SetLoadItems(long nLocationID, long nSupplierID, const CArray<ProductItemReturnInfo,ProductItemReturnInfo&> &arProductItems)
{
	//TES 6/23/2008 - PLID 26152 - Load these variables, this is used by CProductsToBeReturnedDlg.
	m_nInitialLocationID = nLocationID;
	m_nInitialSupplierID = nSupplierID;
	for(int i = 0; i < arProductItems.GetSize(); i++) {
		// (a.walling 2008-10-02 09:25) - PLID 31567 - Must add a copy of a const object
		m_arInitialProductItems.Add(ProductItemReturnInfo(arProductItems[i]));
	}
}

// (j.jones 2008-09-08 10:22) - PLID 30538 - warns if cancelling a new return,
// or an existing one you made changes to, then returns true if the user wants
// to cancel, false if they don't want to cancel
BOOL CInvEditReturnDlg::WarnCancelDialog()
{
	try {

		if(ID_NEW_RETURN_GROUP == m_nID) {
			//it is a new return, so warn accordingly
			BOOL bChoice = (MessageBox("Are you sure you wish to cancel entering this new supplier return?",
				"Practice", MB_ICONQUESTION|MB_YESNO) == IDYES);
			return bChoice;
		}
		else {

			//first update our variables (nothing uses them but saving & auditing so it's ok to do now)
			UpdateData(TRUE);

			//now check if anything has changed
			BOOL bWillAlterExistingProductAdjustments = FALSE;	//not used in this function
			if(HasDialogDataChanged(bWillAlterExistingProductAdjustments)) {
				//they changed something, so warn accordingly
				BOOL bChoice = (MessageBox("You have made changes to this supplier return. Are you sure you wish to cancel saving these changes?",
					"Practice", MB_ICONQUESTION|MB_YESNO) == IDYES);
				return bChoice;
			}
			else {
				//just silently return TRUE if they didn't change anything
				return TRUE;
			}
		}

		//shouldn't be possible to get to this line of code, assert if we do,
		//and return TRUE if we do to allow cancelling
		ASSERT(FALSE);
		return TRUE;

	}NxCatchAll("Error in CInvEditReturnDlg::WarnCancelDialog");

	return FALSE;
}

void CInvEditReturnDlg::EnsureConfirmedControls() 
{
	//TES 1/26/2009 - PLID 32824 - Make sure the date and number are linked to the checkbox.
	BOOL bEnable = FALSE;
	if(IsDlgButtonChecked(IDC_VENDOR_CONFIRMED)) {
		//When checked, we enable the other controls
		bEnable = TRUE;
	}
	else {
		//Disable the controls, we don't need to do anything else with the data
		bEnable = FALSE;
	}

	GetDlgItem(IDC_CONFIRMED_DATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_CONFIRMATION_NUMBER)->EnableWindow(bEnable);
}
void CInvEditReturnDlg::OnVendorConfirmed()
{
	try {
		//TES 1/26/2009 - PLID 32824 - First, update the enabled state of controls.
		EnsureConfirmedControls();
		if(IsDlgButtonChecked(IDC_VENDOR_CONFIRMED)) {
			//TES 1/26/2009 - PLID 32824 - Additionally, set the value of the "date" to today.  This will not overwrite
			//	if for some reason the user previously had something in the field.
			if(m_nxtDateConfirmed->GetStatus() != 1) {
				m_nxtDateConfirmed->SetDateTime(COleDateTime::GetCurrentTime());
			}
		}
		else {
			//Nothing particular to do if they are disabling
		}
	}NxCatchAll("Error in CInvEditReturnDlg::OnVendorConfirmed()");
}

// (j.jones 2009-02-09 15:11) - PLID 32706 - added a total item count label
void CInvEditReturnDlg::UpdateTotalItemCountLabel()
{
	try {
		
		double dblCount = 0.0;

		//add up the total quantity on this return
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlReturnItems->GetFirstRow();
		while(pRow) {
			dblCount += VarDouble(pRow->GetValue(eclRet_Quantity), 0.0);
			pRow = pRow->GetNextRow();
		}

		//now update the label
		CString strCount;
		strCount.Format("%g", dblCount);
		m_nxstaticTotalItemsReturned.SetWindowText(strCount);
		m_nxstaticTotalItemsReturned.Invalidate();

	}NxCatchAll("Error in CInvEditReturnDlg::UpdateTotalItemCountLabel");
}

// (j.jones 2009-02-10 10:32) - PLID 32827 - added OnBtnReturnTrackFedex
void CInvEditReturnDlg::OnBtnReturnTrackFedex()
{
	try {

		//get the current tracking number
		CString strTrackingNumber;
		GetDlgItemText(IDC_TRACKING_NUMBER, strTrackingNumber);
		strTrackingNumber.Replace(" ", "");
		if(strTrackingNumber.IsEmpty()) {
			AfxMessageBox("This return has no tracking number entered. It cannot be tracked online.");
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
			AfxMessageBox("Practice could not successfully replace the {TRACKINGNUMBER} placeholder with this return's tracking number.\n"
				"Please ensure that the preference for the FedEx Tracking hyperlink is valid, and includes the text {TRACKINGNUMBER} as a placeholder for the tracking number.");
			return;
		}

		ShellExecute(NULL, NULL, strHyperlink, NULL, NULL, SW_SHOW);
		
	}NxCatchAll("Error in CInvEditReturnDlg::OnBtnReturnTrackFedex");
}

//(c.copits 2010-09-21) PLID 40317 - Allow duplicate UPC codes for FramesData certification.
// This function will likely be updated to pick the most suitable
// UPC code in response to a barcode scan. Practice now allows multiple
// products to have the same UPC codes. Further, products can share UPC codes
// with service codes (however, service codes cannot share UPC codes).

// Current behavior: returns the first matching UPC code from the item dropdown list (IDC_ITEM_RETURN_LIST)
NXDATALIST2Lib::IRowSettingsPtr CInvEditReturnDlg::GetBestUPCProductFromItemReturnList(_variant_t vBarcode)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow;

// Here it already exists in the IDC_ITEM_RETURN_LIST dropdown (in other words, it has already been scanned in this dialog)
try {

	pRow = m_dlItemCombo->FindByColumn(eclItem_Barcode, vBarcode, NULL, VARIANT_TRUE);

} NxCatchAll(__FUNCTION__);

return pRow;
}

//(c.copits 2010-09-21) PLID 40317 - Allow duplicate UPC codes for FramesData certification
// This function will likely be updated to pick the most suitable
// UPC code in response to a barcode scan. Practice now allows multiple
// products to have the same UPC codes. Further, products can share UPC codes
// with service codes (however, service codes cannot share UPC codes).

// Current behavior: Returns a recordset containing all matching UPCs.

_RecordsetPtr CInvEditReturnDlg::GetBestUPCProductFromData(CString strBarcode)
{
// Here we query the data to find the item by product.

_RecordsetPtr prsProducts;

try {

	prsProducts = CreateParamRecordset("SELECT ProductT.ID, ServiceT.Name FROM ProductT "
				"INNER JOIN ServiceT ON ServiceT.ID = ProductT.ID "
				"WHERE ServiceT.Barcode = {STRING} AND ServiceT.Active = 1 "
				"AND (ProductT.HasSerialNum <> 0 OR ProductT.HasExpDate <> 0) ", strBarcode);
} NxCatchAll(__FUNCTION__);

return prsProducts;
}
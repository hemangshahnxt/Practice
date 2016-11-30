//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
#include "stdafx.h"
#include "InvInternalManagementRequestDlg.h"
#include "InvUtils.h"

#include "InventoryRc.h"
#include "InvEditDlg.h"

using namespace NXDATALIST2Lib;
using namespace ADODB;

// CInvInternalManagementDlg dialog

IMPLEMENT_DYNAMIC(CInvInternalManagementRequestDlg, CNxDialog)

CInvInternalManagementRequestDlg::CInvInternalManagementRequestDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvInternalManagementRequestDlg::IDD, pParent)
{

}

CInvInternalManagementRequestDlg::~CInvInternalManagementRequestDlg()
{
}

BEGIN_MESSAGE_MAP(CInvInternalManagementRequestDlg, CNxDialog)
	ON_BN_CLICKED(IDC_INV_MANAGEMENT_CANCEL_BUTTON, &CInvInternalManagementRequestDlg::OnBnClickedBtnCancel)
	ON_BN_CLICKED(IDC_INV_MANAGEMENT_MAKE_REQUEST_BUTTON, &CInvInternalManagementRequestDlg::OnBnClickedInvManagementMakeRequestButton)
	ON_BN_CLICKED(IDC_INDEFINITE, &CInvInternalManagementRequestDlg::OnBnClickedIndefinite)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CInvInternalManagementRequestDlg, CNxDialog)
	ON_EVENT(CInvInternalManagementRequestDlg, IDC_INV_REQUEST_CATEGORY_COMBO, 16, CInvInternalManagementRequestDlg::SelChosenInvRequestCategoryCombo, VTS_DISPATCH)
	ON_EVENT(CInvInternalManagementRequestDlg, IDC_INV_REQUEST_SELECTED_ITEM_COMBO, 16, CInvInternalManagementRequestDlg::SelChosenInvRequestSelectedItemCombo, VTS_DISPATCH)
	ON_EVENT(CInvInternalManagementRequestDlg, IDC_INV_REQUEST_USER_COMBO, 18, CInvInternalManagementRequestDlg::RequeryFinishedInvRequestUserCombo, VTS_I2)
	ON_EVENT(CInvInternalManagementRequestDlg, IDC_INV_REQUEST_SELECTED_ITEM_COMBO, 18, CInvInternalManagementRequestDlg::RequeryFinishedInvRequestSelectedItemCombo, VTS_I2)
	ON_EVENT(CInvInternalManagementRequestDlg, IDC_INV_REQUEST_CATEGORY_COMBO, 18, CInvInternalManagementRequestDlg::RequeryFinishedInvRequestCategoryCombo, VTS_I2)
END_EVENTSINK_MAP()

void CInvInternalManagementRequestDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INV_REQUEST_FROM_DATETIMEPICKER, m_DateFrom);
	DDX_Control(pDX, IDC_INV_REQUEST_UNTIL_DATETIMEPICKER, m_DateTo);
	DDX_Control(pDX, IDC_INDEFINITE, m_Indefinite); // (j.fouts 2012-05-08 08:43) - PLID 50210
}

//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
BOOL CInvInternalManagementRequestDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		CString strFrom, strWhere, strGroupBy;
		long nRequestedBy = rbNoRequestedBy;		// For checking editing permission
		m_nItemID = itNoItem;
		m_nExistingRecipient = rtNoRecipient;
		m_nServiceTCategory = ctNoCategory;
		long nCategoriesTParent = ctNoCategory;

		m_UserCombo = BindNxDataList2Ctrl(IDC_INV_REQUEST_USER_COMBO, GetRemoteData(), false);
		m_CategoryCombo = BindNxDataList2Ctrl(IDC_INV_REQUEST_CATEGORY_COMBO, GetRemoteData(), false);
		m_ProductCombo = BindNxDataList2Ctrl(IDC_INV_REQUEST_SELECTED_ITEM_COMBO, GetRemoteData(), false);

		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_INV_MANAGEMENT_MAKE_REQUEST_BUTTON))->AutoSet(NXB_OK);
		((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_INV_MANAGEMENT_CANCEL_BUTTON))->AutoSet(NXB_CANCEL);

		COleDateTime dtNow = COleDateTime::GetCurrentTime();

		m_DateTo.SetValue((dtNow));
		m_DateFrom.SetValue((dtNow));

		// Are we editing an existing request?
		if (m_nRequestID != rtNewRequest) {
			// (j.luckoski 2012-04-23 08:57) - PLID 49305 - Convert ItemStatus to Int so it doesn't cause VARTYPE errors.
			// (j.fouts 2012-05-08 09:36) - PLID 50210 - Added Indefinite
			_RecordsetPtr rsRequest = CreateParamRecordset(
				"SELECT ProductID, RequestFrom, RequestTo, Recipient, RequestedBy, CategoryID, "
				"ServiceT.Category AS ServiceTCategory, CONVERT(INT,ItemStatus) AS ItemStatus, Indefinite "
				"FROM ProductRequestsT " 
				"LEFT JOIN ServiceT ON ProductRequestsT.ProductID = ServiceT.ID "
				"WHERE ProductRequestsT.ID = {INT} "
				, m_nRequestID);
			if (!rsRequest->eof) {
				COleDateTime dtFrom, dtTo;
				BOOL bIndefinite;

				m_nItemID = AdoFldLong(rsRequest, "ProductID", itNoItem);
				dtFrom = AdoFldDateTime(rsRequest, "RequestFrom");
				dtTo = AdoFldDateTime(rsRequest, "RequestTo");
				m_nExistingRecipient = AdoFldLong(rsRequest, "Recipient", rtNoRecipient);
				nRequestedBy = AdoFldLong(rsRequest, "RequestedBy", rbNoRequestedBy);
				m_nCategoryID = AdoFldLong(rsRequest, "CategoryID", ctNoCategory);
				m_nServiceTCategory = AdoFldLong(rsRequest, "ServiceTCategory", ctNoCategory);
				// (j.luckoski 2012-04-23 08:58) - PLID 49305 - If Item is checked out then disable user combo,
				// category combo, and item combo so item can't be changed.
				m_nItemStatus = AdoFldLong(rsRequest, "ItemStatus", 0);
				// (j.fouts 2012-05-08 09:38) - PLID 50210 - Load the Indefinite flag
				bIndefinite = AdoFldBool(rsRequest, "Indefinite");

				if(m_nItemStatus == 3) {
					GetDlgItem(IDC_INV_REQUEST_USER_COMBO)->EnableWindow(FALSE);
					GetDlgItem(IDC_INV_REQUEST_CATEGORY_COMBO)->EnableWindow(FALSE);
					GetDlgItem(IDC_INV_REQUEST_SELECTED_ITEM_COMBO)->EnableWindow(FALSE);
				}

				if (dtFrom.GetStatus() == COleDateTime::valid) {
					m_DateFrom.SetValue(_variant_t(dtFrom));
				}

				if (dtTo.GetStatus() == COleDateTime::valid) {
					m_DateTo.SetValue(_variant_t(dtTo));
				}

				if(bIndefinite)
				{
					m_DateTo.EnableWindow(FALSE);
					CheckDlgButton(IDC_INDEFINITE, BST_CHECKED);
				}
				
			}
			else {
				AfxMessageBox("Error: Request data could not be read.\n"
							  "Cannot edit request.");
				return drtDialogErrorNoReadExistingData;
			}
			rsRequest->Close();
		}

		// User datalist
		strFrom = "(SELECT *, Upper(UsersT.UserName) AS UserNameUppercase, Lower(UsersT.UserName) AS UserNameLowercase FROM UsersT WHERE UsersT.PersonID > 0) UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID ";
		strWhere = "PersonT.Archived = 0";
		m_UserCombo->FromClause = _bstr_t(strFrom);
		m_UserCombo->WhereClause = _bstr_t(strWhere);
		m_UserCombo->Requery();

		// Category datalist
		// String comparisons in MSSQL are case insensitive
		_RecordsetPtr rsRequest = CreateParamRecordset(
			"SELECT ID FROM CategoriesT "
			"WHERE Name LIKE 'NexTech Tracked Inventory'"
			);
		if (!rsRequest->eof) {
			nCategoriesTParent = AdoFldLong(rsRequest, "ID", ctNoCategory);
		}
		rsRequest->Close();
		if (nCategoriesTParent == ctNoCategory) {
			CString strMessage;
			strMessage.Format(
				"Category 'NexTech Tracked Inventory' does not exist.\n"
				"This category must exist and be a parent category for all request categories."
				);
			MessageBox(strMessage, MB_OK);
		}
		strFrom = "ProductT INNER JOIN (SELECT * FROM ServiceT WHERE Active = 1) AS ServiceT ON ProductT.ID = ServiceT.ID INNER JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID";
		strGroupBy = "CategoriesT.Name, CategoriesT.ID";
		m_CategoryCombo->FromClause = _bstr_t(strFrom);
		strWhere.Format("CategoriesT.Parent = %li ", nCategoriesTParent);
		m_CategoryCombo->WhereClause = _bstr_t(strWhere);
		m_CategoryCombo->GroupByClause = _bstr_t(strGroupBy);
		m_CategoryCombo->Requery();
		
		// Product datalist
		strFrom =	"ProductT INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
					"INNER JOIN ProductRequestableT ON ProductT.ID = ProductRequestableT.ProductID ";
		strWhere = "ServiceT.Active = 1 AND IsRequestable = 1 ";

		m_ProductCombo->FromClause = _bstr_t(strFrom);
		m_ProductCombo->WhereClause = _bstr_t(strWhere);
		// Requery for product datalist done automatically when the category requery is performed
		
		// Datalist values are set to default in RequeryFinished methods

		ApplyPermissions(nRequestedBy);

		GetDlgItem(IDC_INV_REQUEST_USER_COMBO)->SetFocus();

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
//(e.lally 2012-01-05) PLID 46492 - Removed success message boxes.
void CInvInternalManagementRequestDlg::OnBnClickedInvManagementMakeRequestButton()
{
	try {

		COleDateTime dtFrom, dtTo;
		CString strFromDate, strToDate;

		// Pull information from sources for saving		
		long nProductID = itNoItem;
		long nCategoryID = ctNoCategory;	
		long nRecipient = rtNoRecipient;
		bool bIsCategory = false;
	
		IRowSettingsPtr pRow;

		// Get user ID of recipient & verify integrity
		pRow = m_UserCombo->GetCurSel();
		if (pRow) {
			nRecipient = VarLong(pRow->GetValue(ulID));
		}
		
		if (!pRow || nRecipient < 0) {
			AfxMessageBox("Error: Need a valid recipient.");
			return;
		}
	
		// Get item ID & verify integrity
		pRow = m_ProductCombo->GetCurSel();
		if (pRow) {
			nProductID = VarLong(pRow->GetValue(ilID));
			if (nProductID == dpfAnyItem) {
				bIsCategory = true;
			}
		}
		else {
			AfxMessageBox("Error: Need a valid item.");
			return;
		}

		// Valid item?
		if ((nProductID < 0) && (nProductID != dpfAnyItem)) {
			AfxMessageBox("Error: Need a valid item.");
			return;
		}

		// Get category
		pRow = m_CategoryCombo->GetCurSel();
		if (pRow) {
			nCategoryID = VarLong(pRow->GetValue(clID));
		}
		else {
			AfxMessageBox("Error: Need to specify a category.");
			return;
		}

		// Check times
		dtFrom = m_DateFrom.GetValue();
		dtTo = m_DateTo.GetValue();

		// Get rid of time part of date before saving
		dtFrom = AsDateNoTime(dtFrom);
		dtTo = AsDateNoTime(dtTo);

		// (j.fouts 2012-05-07 14:44) - PLID 50210 - Check if this is an indefinite checkout
		BOOL bIndefinite = m_Indefinite.GetCheck();

		//Only check the date range if this is not indefinite
		if(!bIndefinite)
		{
			// Invalid date range selected?
			if (dtFrom > dtTo) {
				AfxMessageBox("Error: Invalid date range. 'Until' must be greater than 'From'.");
				return;
			}
		}

		// Valid times?
		if (dtFrom.GetStatus() != COleDateTime::valid) {
			AfxMessageBox("Error: Invalid 'From' time.");
			return;
		}

		// (j.fouts 2012-05-08 10:41) - PLID 50210 - To Date only needs to be valid if this is not indefinte
		if(!bIndefinite)
		{
			if (dtTo.GetStatus() != COleDateTime::valid) {
				AfxMessageBox("Error: Invalid 'To' time.");
				return;
			}
		}

		// (j.fouts 2012-05-08 10:41) - PLID 50210 - This only matters if this is not indefinite
		if(!bIndefinite)
		{
			// Same day for both? (Or, the user forgot to change the defaults)
			COleDateTime dtNow = COleDateTime::GetCurrentTime();
			dtNow = AsDateNoTime(dtNow);
			if ((dtFrom == dtTo) && (dtNow == dtFrom)) {
				CString strMessage;
				strMessage.Format("Are you sure you want to borrow and return on the same day (today)?");
				if (MessageBox(strMessage, NULL, MB_YESNO) == IDNO) {
					return;
				}
			}
		}
	
		// Check & Save for specific item (new request)
		// Check whether a specific item (non-category) overlaps in an already requested time
		if (m_nRequestID == rtNewRequest && !bIsCategory) {

			_RecordsetPtr rsCheckRequest;
			// (j.fouts 2012-05-08 10:41) - PLID 50210 - Create the query based on whether this is an indefinite checkout of not
			if(bIndefinite)
			{	
				rsCheckRequest = CreateParamRecordset(
					"SELECT ID FROM ProductRequestsT WHERE ProductID = {INT} "
					"AND ActualEnd IS NULL "
					"AND ItemStatus <> 1 "
					"AND NOT ((Indefinite = 0 AND RequestFrom < {OLEDATETIME} AND RequestTo < {OLEDATETIME} ))",
					nProductID, dtFrom, dtFrom);

			} else {
				rsCheckRequest = CreateParamRecordset(
					"SELECT ID FROM ProductRequestsT WHERE ProductID = {INT} "
					"AND ActualEnd IS NULL "
					"AND ItemStatus <> 1 "
					"AND NOT ((Indefinite = 0 AND RequestFrom < {OLEDATETIME} AND RequestTo < {OLEDATETIME} ) "
					"OR (Indefinite = 0 AND RequestFrom > {OLEDATETIME} AND RequestTo > {OLEDATETIME}) "
					"OR (Indefinite = 1 AND RequestFrom > {OLEDATETIME}))",
					nProductID, dtFrom, dtFrom, dtTo, dtTo, dtTo);
			}

			if(!rsCheckRequest->eof) {
				CString strMessage;
				strMessage.Format("This item has already been requested inside that date range.\n"
								  "Please try selecting a different item.");
				AfxMessageBox(strMessage);	
				rsCheckRequest->Close();
				return;
			}
			rsCheckRequest->Close();
				
			// Save actual item request (non-category)
			// New request
			//(e.lally 2012-01-05) PLID 46492 - Default to pending request status
			// (j.fouts 2012-05-07 16:13) - PLID 50210 - Added bIndefinite
			_RecordsetPtr rsRequest = CreateParamRecordset(
				"INSERT INTO ProductRequestsT (ProductID, CategoryID, Recipient, RequestedBy, RequestFrom, RequestTo, ItemStatus, Indefinite) "
				"VALUES ({INT}, NULL, {INT}, {INT}, {OLEDATETIME}, {OLEDATETIME}, 0, {BIT})"
				, nProductID, nRecipient, GetCurrentUserID(), dtFrom, dtTo, bIndefinite);
			// Reminder: Do not close connections for inserts...
			//MessageBox("Request entered.", MB_OK);
		}
		// Check & Save for a specific item (modified request)
		else if (m_nRequestID != rtNewRequest && !bIsCategory) {
			
			// Edit existing request?
			_RecordsetPtr rsCheckRequest;
			// (j.fouts 2012-05-08 10:42) - PLID 50210 - Create the query based on whether this is an indefinite checkout of not
			if(bIndefinite)
			{	
				
				rsCheckRequest = CreateParamRecordset(
					"SELECT ID FROM ProductRequestsT WHERE ProductID = {INT} "
					"AND ID <> {INT} "
					"AND ActualEnd IS NULL "
					"AND ItemStatus <> 1 "
					"AND NOT ((Indefinite = 0 AND RequestFrom < {OLEDATETIME} AND RequestTo < {OLEDATETIME} ))",
					nProductID, m_nRequestID, dtFrom, dtFrom);

			} else {
				rsCheckRequest = CreateParamRecordset(
					"SELECT ID FROM ProductRequestsT WHERE ProductID = {INT} "
					"AND ID <> {INT} "
					"AND ActualEnd IS NULL "
					"AND ItemStatus <> 1 "
					"AND NOT ((Indefinite = 0 AND RequestFrom < {OLEDATETIME} AND RequestTo < {OLEDATETIME} ) "
					"OR (Indefinite = 0 AND RequestFrom > {OLEDATETIME} AND RequestTo > {OLEDATETIME}) "
					"OR (Indefinite = 1 AND RequestFrom > {OLEDATETIME}))",
					nProductID, m_nRequestID, dtFrom, dtFrom, dtTo, dtTo, dtTo);
			}
			
			if(!rsCheckRequest->eof) {
				CString strMessage;
				strMessage.Format("This item has already been requested inside that date range.\n"
								  "Please try selecting a different item.");
				AfxMessageBox(strMessage);	
				rsCheckRequest->Close();
				return;
			}
			rsCheckRequest->Close();
			
			// Update existing request for specific item
			// (j.luckoski 2012-04-23 09:00) - PLID 49305 - Store item status.
			long nStatus = 0;
			if(m_nItemID == nProductID) {
				nStatus = m_nItemStatus;
			}
			
			// (j.luckoski 2012-04-23 09:01) - PLID 49305 - Return status to 0 if not the same item after request change
			// (j.fouts 2012-05-08 10:43) - PLID 50210 - Added Indefinite
			_RecordsetPtr rsRequest = CreateParamRecordset(
				"UPDATE ProductRequestsT "
				"SET ProductID = {INT}, Recipient = {INT}, "
				"RequestFrom = {OLEDATETIME}, RequestTo = {OLEDATETIME}, "
				"Indefinite = {BIT}, "
				"CategoryID = NULL, ItemStatus = {INT} "
				"WHERE ProductRequestsT.ID = {INT} "
				,nProductID, nRecipient, dtFrom, dtTo, bIndefinite, nStatus ,m_nRequestID);
			//MessageBox("Request updated.", MB_OK);
		}
		// Save New Category Request
		// No checking needed for category overlaps
		else if (m_nRequestID == rtNewRequest && bIsCategory) { 
			//(e.lally 2012-01-05) PLID 46492 - Default to pending request status
			// (j.fouts 2012-05-08 10:43) - PLID 50210 - Added Indefinite
			_RecordsetPtr rsRequest = CreateParamRecordset(
				"INSERT INTO ProductRequestsT (ProductID, CategoryID, Recipient, RequestedBy, RequestFrom, RequestTo, ItemStatus, Indefinite) "
				"VALUES (NULL, {INT}, {INT}, {INT}, {OLEDATETIME}, {OLEDATETIME}, 0, {BIT})", nCategoryID, nRecipient, GetCurrentUserID(), dtFrom, dtTo, bIndefinite);
			// Reminder: Do not close connections for inserts...
			//MessageBox("Request entered.", MB_OK);
		}
		// Modify existing request for category (m_nRequestID != rtNewRequest && bIsCategory)
		else {
			// (j.fouts 2012-05-08 10:43) - PLID 50210 - Added Indefinite
			_RecordsetPtr rsRequest = CreateParamRecordset(
				"UPDATE ProductRequestsT "
				"SET CategoryID = {INT}, Recipient = {INT}, "
				"RequestFrom = {OLEDATETIME}, RequestTo = {OLEDATETIME}, "
				"Indefinite = {BIT}, "
				"ProductID = NULL "
				"WHERE ProductRequestsT.ID = {INT} "
				,nCategoryID, nRecipient, dtFrom, dtTo, bIndefinite, m_nRequestID);
			//MessageBox("Request updated.", MB_OK);
		}

		CNxDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::OnBnClickedBtnCancel()
{
	try {

		CNxDialog::OnCancel();

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::SelChosenInvRequestCategoryCombo(LPDISPATCH lpRow)
{
	try {

		if (lpRow) {
			SelChosenCategoryList(lpRow);
		}

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::SelChosenCategoryList(LPDISPATCH lpRow)
{
	try {
		if (lpRow) {
			_variant_t value;

			m_CategoryCombo->GetCurSel();

			IRowSettingsPtr pCatRow(lpRow);
			value = pCatRow->GetValue(clID);

			m_ProductCombo->WhereClause = _bstr_t("ServiceT.Active = 1 AND IsRequestable = 1 AND Category " 
				+ InvUtils::Descendants(value.lVal));

			m_ProductCombo->Requery();
			
		}
		
	} NxCatchAll(__FUNCTION__);
	
}

//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::UpdateItemNotes()
{
	try {

		IRowSettingsPtr pRow;
		
		pRow = m_ProductCombo->GetCurSel();

		if (pRow) {

			long nProductID = VarLong(pRow->GetValue(ilID));

			_RecordsetPtr rs = CreateParamRecordset("SELECT Notes FROM ProductT WHERE ID = {INT}", nProductID);

			if (!rs->eof) {
				CString strNotes = AdoFldString(rs, "Notes", "");
				SetDlgItemText(IDC_INV_REQUEST_NOTES_EDIT, strNotes);
			}
			else {
				// If data could not be read
				SetDlgItemText(IDC_INV_REQUEST_NOTES_EDIT, "");
			}

			rs->Close();
		}

		else {
			// Blank out notes if no item selected
			SetDlgItemText(IDC_INV_REQUEST_NOTES_EDIT, "");
		}

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::SelChosenInvRequestSelectedItemCombo(LPDISPATCH lpRow)
{
	try {

		if (lpRow) {
			UpdateItemNotes();
		}

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::AddAnyItemEntry()
{
	try {
		IRowSettingsPtr pRow;
		_variant_t varID = (long) dpfAnyItem;
		// No need to add another (user clicks on category twice)
		pRow = m_ProductCombo->FindByColumn(ilID, dpfAnyItem, NULL, VARIANT_FALSE);
		if (!pRow) {
			pRow = m_ProductCombo->GetNewRow();
			if (pRow) {
				pRow->PutValue(ilID, varID);
				pRow->PutValue(ilName ,_variant_t("<Any Item>"));
				m_ProductCombo->AddRowBefore(pRow, m_ProductCombo->GetFirstRow());
				m_ProductCombo->PutCurSel(pRow);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::RemoveAnyItemEntry()
{
	try {
		IRowSettingsPtr pRow;
		pRow = m_ProductCombo->FindByColumn(ilID, dpfAnyItem, NULL, VARIANT_FALSE);
		if (pRow) {
			m_ProductCombo->RemoveRow(pRow);
		}
	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-04-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::SetRequest(long nRequestID)
{

	m_nRequestID = nRequestID;

}

//(c.copits 2011-05-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::SetCategory(long nCategoryID)
{

	m_nCategoryID = nCategoryID;

}

//(c.copits 2011-05-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::ApplyPermissions(long nRequestedBy)
{
	try {
		// Administrators can edit all requests
		if (IsCurrentUserAdministrator()) {
			return;
		}

		// Check for an attempt for one person to edit another person's request
		if (nRequestedBy != rbNoRequestedBy)
		{
			long nCurrentID = GetCurrentUserID();

			// Users can only edit their own requests
			if ((m_nRequestID != rtNewRequest) && (nCurrentID != nRequestedBy)) {

				GetDlgItem(IDC_INV_MANAGEMENT_MAKE_REQUEST_BUTTON)->EnableWindow(FALSE);
				m_UserCombo->Enabled = VARIANT_FALSE;
				m_CategoryCombo->Enabled = VARIANT_FALSE;
				m_ProductCombo->Enabled = VARIANT_FALSE;
				GetDlgItem(IDC_INV_REQUEST_FROM_DATETIMEPICKER)->EnableWindow(FALSE);
				GetDlgItem(IDC_INV_REQUEST_UNTIL_DATETIMEPICKER)->EnableWindow(FALSE);
				GetDlgItem(IDC_INV_REQUEST_NOTES_EDIT)->EnableWindow(FALSE);
				// (j.fouts 2012-06-25 09:46) - PLID 50210 - Should not be able to change indefinite status either
				GetDlgItem(IDC_INDEFINITE)->EnableWindow(FALSE);

				MessageBox("Since you did not enter this request, editing is not allowed.", MB_OK);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-05-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::InitialQueryProductCombo()
{
	try {
		long nCategoryID = ctNoCategory;
		IRowSettingsPtr pRow = m_CategoryCombo->GetCurSel();
		if (pRow) {
			nCategoryID = VarLong(pRow->GetValue(clID));
			CString strWhere;
			strWhere.Format("ServiceT.Active = 1 AND IsRequestable = 1 AND Category = %li", nCategoryID);
			m_ProductCombo->WhereClause = _bstr_t(strWhere);
			m_ProductCombo->Requery();
		}
	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-05-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::RequeryFinishedInvRequestUserCombo(short nFlags)
{
	try {
		// Set current user as default
		IRowSettingsPtr pRow;
		// If existing request
		if (m_nRequestID != rtNewRequest && m_nExistingRecipient > rtNoRecipient) {
			pRow = m_UserCombo->FindByColumn(ulID, m_nExistingRecipient, NULL, VARIANT_FALSE);
			if (pRow) {
				m_UserCombo->PutCurSel(pRow);
			}
		}
		else {
			// If new request
			long nCurrentID = GetCurrentUserID();
			pRow = m_UserCombo->FindByColumn(0, GetCurrentUserID(), NULL, VARIANT_FALSE);
			if (pRow) {
				m_UserCombo->PutCurSel(pRow);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-05-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::RequeryFinishedInvRequestSelectedItemCombo(short nFlags)
{
	try {
		AddAnyItemEntry();
		// Set on an existing product
		if (m_nRequestID != rtNewRequest) {
			IRowSettingsPtr pRow;
			pRow = m_ProductCombo->FindByColumn(ilID, m_nItemID, NULL, VARIANT_FALSE);
			if (pRow) {
				m_ProductCombo->PutCurSel(pRow);
			}
		}
		UpdateItemNotes();
	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-05-12) PLID 43243 - Implement request dialog functionality
void CInvInternalManagementRequestDlg::RequeryFinishedInvRequestCategoryCombo(short nFlags)
{
	try {
		// All this stuff is for when the initial requery is completed and the dialog is opened for the first time
		IRowSettingsPtr pRow;

		// Has an item been selected before this dialog opened? (Or, is an item selected in a previous request)
		// (Editing an existing request, so select the current product & category)
		if (m_nItemID != itNoItem) {
			if (m_nServiceTCategory != ctNoCategory) {
				pRow = m_CategoryCombo->FindByColumn(clID, m_nServiceTCategory, NULL, VARIANT_FALSE);
				m_CategoryCombo->PutCurSel(pRow);
			}

			InitialQueryProductCombo();
		}

		// Select default category if one was passed in
		if ((m_nItemID == itNoItem) && (m_nCategoryID != ctNoCategory)) {
			pRow = m_CategoryCombo->FindByColumn(clID, m_nCategoryID, NULL, VARIANT_FALSE);
			m_CategoryCombo->PutCurSel(pRow);
			if (pRow) {
				InitialQueryProductCombo();
			}
		}
		// Just put default on first category
		else if ((m_nItemID == itNoItem) && (m_nCategoryID == ctNoCategory)) {
			pRow = m_CategoryCombo->GetFirstRow();
			m_CategoryCombo->PutCurSel(pRow);
			if (pRow) {
				InitialQueryProductCombo();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-05-07 14:40) - PLID 50210 - Handle clicking the Indefinite Checkbox
void CInvInternalManagementRequestDlg::OnBnClickedIndefinite()
{
	try
	{
		//Enable/Disable the Until Date
		m_DateTo.EnableWindow(!m_Indefinite.GetCheck());
	}
	NxCatchAll(__FUNCTION__);
}

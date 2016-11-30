// UpdateProductsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "UpdateProductsDlg.h"
#include "InternationalUtils.h"
#include "AuditTrail.h"
#include "InvUtils.h"
#include "CategorySelectDlg.h"
#include "GlobalFinancialUtils.h"

// (j.jones 2010-01-11 10:27) - PLID 26786 - created

// CUpdateProductsDlg dialog

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

enum ProductListColumns {

	plcID = 0,
	plcName,
	plcCategory,
	plcSupplier,
	plcTax1,
	plcTax2,
	plcBillable,
	plcTrackable,
	plcFrame,
	plcCost,
	plcReorderPoint,
	plcReorderQty,
	plcRespUser,
	plcRevCode,
	plcShopFee,
	plcRemCharProv,
	plcInsCode
};

// (j.jones 2010-01-13 10:21) - PLID 36847 - added location combo
enum LocationComboColumns {

	lccID =0,
	lccName,
};

enum UserComboColumns {

	uccID = 0,
	uccUsername,
};

enum SupplierComboColumns {

	sccID = 0,
	sccSupplier,
};

enum RevCodeComboColumns {

	rcccID = 0,
	rcccCode,
	rcccDescription,
};


CUpdateProductsDlg::CUpdateProductsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUpdateProductsDlg::IDD, pParent)
{
	m_nDefaultCategoryID = -1;
}

CUpdateProductsDlg::~CUpdateProductsDlg()
{
}

void CUpdateProductsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_APPLY_PRODUCT_CHANGES, m_btnApply);
	DDX_Control(pDX, IDC_BTN_CLOSE_PRODUCT_UPDATER, m_btnClose);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE, m_btnSelectOne);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL, m_btnSelectAll);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE, m_btnUnselectOne);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL, m_btnUnselectAll);
	DDX_Control(pDX, IDC_CHECK_UPDATE_CATEGORY, m_checkUpdateCategory);
	DDX_Control(pDX, IDC_CHECK_UPDATE_SUPPLIER, m_checkUpdateSupplier);
	DDX_Control(pDX, IDC_CHECK_UPDATE_TAXABLE_1, m_checkUpdateTaxable1);
	DDX_Control(pDX, IDC_CHECK_UPDATE_TAXABLE_2, m_checkUpdateTaxable2);
	DDX_Control(pDX, IDC_CHECK_UPDATE_REV_CODE, m_checkUpdateRevCode);
	DDX_Control(pDX, IDC_CHECK_UPDATE_SHOP_FEE, m_checkUpdateShopFee);
	DDX_Control(pDX, IDC_CHECK_UPDATE_SHOP_FEE_LOC, m_checkUpdateShopFeeLoc); // (j.gruber 2012-10-29 13:47) - PLID 53241
	DDX_Control(pDX, IDC_CHECK_UPDATE_BILLABLE, m_checkUpdateBillable);
	DDX_Control(pDX, IDC_CHECK_UPDATE_TRACKABLE, m_checkUpdateTrackable);
	DDX_Control(pDX, IDC_CHECK_UPDATE_REORDER_POINT, m_checkUpdateReorderPoint);
	DDX_Control(pDX, IDC_CHECK_UPDATE_REORDER_QTY, m_checkUpdateReorderQty);
	DDX_Control(pDX, IDC_CHECK_UPDATE_USER_RESP, m_checkUpdateUserResp);
	DDX_Control(pDX, IDC_CHECK_UPDATE_INSURANCE_CODE, m_checkUpdateInsuranceCode);
	DDX_Control(pDX, IDC_PROD_TAXABLE, m_checkTaxable1);
	DDX_Control(pDX, IDC_PROD_TAXABLE2, m_checkTaxable2);
	DDX_Control(pDX, IDC_PROD_SHOP_FEE, m_nxeditShopFee);
	DDX_Control(pDX, IDC_PROD_SHOP_FEE_LOC, m_nxeditShopFeeLoc); // (j.gruber 2012-10-29 13:47) - PLID 53241
	DDX_Control(pDX, IDC_PROD_BILLABLE, m_checkBillable);
	DDX_Control(pDX, IDC_PROD_RADIO_NOT_TRACKABLE, m_radioNotTrackable);
	DDX_Control(pDX, IDC_PROD_RADIO_TRACK_ORDERS, m_radioTrackOrders);
	DDX_Control(pDX, IDC_PROD_RADIO_TRACK_QUANTITY, m_radioTrackQuantity);
	DDX_Control(pDX, IDC_PROD_REORDERPOINT, m_nxeditReorderPoint);
	DDX_Control(pDX, IDC_PROD_REORDERQUANTITY, m_nxeditReorderQty);
	DDX_Control(pDX, IDC_PROD_INSURANCE_CODE, m_nxeditInsuranceCode);
	DDX_Control(pDX, IDC_BTN_SELECT_CATEGORY, m_btnSelectCategory);
	DDX_Control(pDX, IDC_BTN_REMOVE_CATEGORY, m_btnRemoveCategory);
	DDX_Control(pDX, IDC_CATEGORY_BOX, m_editCategory);
	DDX_Control(pDX, IDC_CHECK_UPDATE_REMEMBER_CHARGE_PROVIDER, m_checkUpdateRememberChargeProvider);
	DDX_Control(pDX, IDC_PROD_REMEMBER_CHARGE_PROVIDER, m_checkRememberChargeProvider);
}


BEGIN_MESSAGE_MAP(CUpdateProductsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_APPLY_PRODUCT_CHANGES, OnBtnApplyProductChanges)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE, OnBtnSelectOne)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL, OnBtnSelectAll)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE, OnBtnUnselectOne)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL, OnBtnUnselectAll)
	ON_BN_CLICKED(IDC_BTN_CLOSE_PRODUCT_UPDATER, OnBtnCloseProductUpdater)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_CATEGORY, OnCheckUpdateCategory)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_SUPPLIER, OnCheckUpdateSupplier)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_TAXABLE_1, OnCheckUpdateTaxable1)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_TAXABLE_2, OnCheckUpdateTaxable2)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_REV_CODE, OnCheckUpdateRevCode)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_SHOP_FEE, OnCheckUpdateShopFee)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_BILLABLE, OnCheckUpdateBillable)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_TRACKABLE, OnCheckUpdateTrackable)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_REORDER_POINT, OnCheckUpdateReorderPoint)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_REORDER_QTY, OnCheckUpdateReorderQty)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_USER_RESP, OnCheckUpdateUserResp)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_SHOP_FEE_LOC, OnBnClickedCheckUpdateShopFeeLoc)
	ON_BN_CLICKED(IDC_BTN_SELECT_CATEGORY, OnBtnSelectCategory)
	ON_BN_CLICKED(IDC_BTN_REMOVE_CATEGORY, OnBtnRemoveCategory)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_REMEMBER_CHARGE_PROVIDER, OnCheckUpdateRememberChargeProvider)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_INSURANCE_CODE, OnCheckUpdateInsuranceCode)
END_MESSAGE_MAP()


// CUpdateProductsDlg message handlers
BOOL CUpdateProductsDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnApply.AutoSet(NXB_MODIFY);
		m_btnSelectOne.AutoSet(NXB_DOWN);
		m_btnSelectAll.AutoSet(NXB_DDOWN);
		m_btnUnselectOne.AutoSet(NXB_UP);
		m_btnUnselectAll.AutoSet(NXB_UUP);

		m_UnselectedList = BindNxDataList2Ctrl(IDC_UPDATE_PRODUCT_UNSELECTED_LIST, false);
		m_SelectedList = BindNxDataList2Ctrl(IDC_UPDATE_PRODUCT_SELECTED_LIST, false);
		m_SupplierCombo = BindNxDataList2Ctrl(IDC_SUPPLIER_COMBO);
		m_RevCodeCombo = BindNxDataList2Ctrl(IDC_PROD_REV_CODE_COMBO);
		// (j.jones 2010-01-13 10:12) - PLID 36847 - added per-location controls
		m_LocationCombo = BindNxDataList2Ctrl(IDC_UPDATE_PROD_LOCATION_COMBO);
		m_UserCombo = BindNxDataList2Ctrl(IDC_PROD_USER_RESP_COMBO, false);

		//If they do not have a NexSpa license, hide the Shop Fee fields
		if(!IsSpa(FALSE)) {
			GetDlgItem(IDC_CHECK_UPDATE_SHOP_FEE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PROD_SHOP_FEE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PROD_SHOP_FEE)->ShowWindow(SW_HIDE);
			// (j.gruber 2012-10-29 13:50) - PLID 53241 - and per location ones
			GetDlgItem(IDC_CHECK_UPDATE_SHOP_FEE_LOC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PROD_SHOP_FEE_LABEL_LOC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PROD_SHOP_FEE_LOC)->ShowWindow(SW_HIDE);
			IColumnSettingsPtr pShopCol1 = m_UnselectedList->GetColumn(plcShopFee);
			pShopCol1->PutColumnStyle(csVisible | csFixedWidth);
			pShopCol1->PutStoredWidth(0);
			IColumnSettingsPtr pShopCol2 = m_SelectedList->GetColumn(plcShopFee);
			pShopCol2->PutColumnStyle(csVisible | csFixedWidth);
			pShopCol2->PutStoredWidth(0);
		}

		// If they do not have a Frames license, hide the Frame column
		if (!g_pLicense->CheckForLicense(CLicense::lcFrames, CLicense::cflrUse))
		{
			IColumnSettingsPtr pFrameUnselCol = m_UnselectedList->GetColumn(plcFrame);
			pFrameUnselCol->PutColumnStyle(csVisible | csFixedWidth);
			pFrameUnselCol->PutStoredWidth(0);
			IColumnSettingsPtr pFrameSelCol = m_SelectedList->GetColumn(plcFrame);
			pFrameSelCol->PutColumnStyle(csVisible | csFixedWidth);
			pFrameSelCol->PutStoredWidth(0);
		}

		//set some reasonable defaults
		m_radioTrackQuantity.SetCheck(TRUE);
		m_checkBillable.SetCheck(TRUE);
		m_nxeditShopFee.SetWindowText(FormatCurrencyForInterface(COleCurrency(0,0)));
		m_nxeditShopFeeLoc.SetWindowText(FormatCurrencyForInterface(COleCurrency(0,0))); // (j.gruber 2012-10-29 14:37) - PLID 53241
		SetDlgItemInt(IDC_PROD_REORDERPOINT, 1);
		SetDlgItemInt(IDC_PROD_REORDERQUANTITY, 1);

		// (j.jones 2010-01-13 10:12) - PLID 36847 - select the current location
		m_LocationCombo->SetSelByColumn(lccID, GetCurrentLocationID());
		
		//show just that location's users
		CString strUserWhere;
		strUserWhere.Format("PersonT.Archived = 0 AND UsersT.PersonID > 0 AND UserLocationT.LocationID = %li", GetCurrentLocationID());
		m_UserCombo->PutWhereClause((LPCTSTR)strUserWhere);
		m_UserCombo->Requery();

		//refresh the product info. for this location
		RefreshProductLists(GetCurrentLocationID());

		//add the "No User" row
		IRowSettingsPtr pUserRow = m_UserCombo->GetNewRow();
		pUserRow->PutValue(uccID, (long)-1);
		pUserRow->PutValue(uccUsername, " <No Assigned User>");
		m_UserCombo->AddRowSorted(pUserRow, NULL);
		
		//add the "No Supplier" row
		IRowSettingsPtr pSupplierRow = m_SupplierCombo->GetNewRow();
		pSupplierRow->PutValue(sccID, (long)-1);
		pSupplierRow->PutValue(sccSupplier, " <No Supplier>");
		m_SupplierCombo->AddRowSorted(pSupplierRow, NULL);

		//add the "No Revenue Code" row
		IRowSettingsPtr pRevRow = m_RevCodeCombo->GetNewRow();
		pRevRow->PutValue(rcccID, (long)-1);
		pRevRow->PutValue(rcccCode, " <None>");
		pRevRow->PutValue(rcccDescription, " <No Revenue Code>");
		m_RevCodeCombo->AddRowSorted(pRevRow, NULL);

		//disable all the fields
		OnCheckUpdateCategory();
		OnCheckUpdateSupplier();
		OnCheckUpdateTaxable1();
		OnCheckUpdateTaxable2();
		OnCheckUpdateRevCode();
		OnCheckUpdateShopFee();
		OnCheckUpdateInsuranceCode();
		// (j.jones 2010-01-13 10:12) - PLID 36847 - added per-location controls
		OnCheckUpdateBillable();
		OnCheckUpdateTrackable();
		OnCheckUpdateReorderPoint();
		OnCheckUpdateReorderQty();
		OnCheckUpdateUserResp();
		OnBnClickedCheckUpdateShopFeeLoc(); // (j.gruber 2012-10-29 14:38) - PLID 53240
		OnCheckUpdateRememberChargeProvider(); // (j.jones 2016-04-07 11:54) - NX-100076

	} NxCatchAll(__FUNCTION__);

	return FALSE;
}

void CUpdateProductsDlg::OnBtnApplyProductChanges()
{
	long nAuditTransactionID = -1;

	try {

		// (j.jones 2010-01-13 10:21) - PLID 36847 - get the currently selected location ID
		long nLocationID = -1;
		CString strLocationName;
		{
			IRowSettingsPtr pRow = m_LocationCombo->GetCurSel();
			if(pRow == NULL) {		
				//abort this process, force them to select a location
				AfxMessageBox("You must select a location before applying changes.");
				return;
			}

			nLocationID = VarLong(pRow->GetValue(lccID));
			strLocationName = VarString(pRow->GetValue(lccName));
		}

		CString strSqlBatch;
		CString strGlobalItemsToBeChanged;
		CString strLocationItemsToBeChanged;	// (j.jones 2010-01-13 10:59) - PLID 36847

		//get all our selected product IDs
		CString strProductIDs;
		std::vector<long> aryServiceIDs;
		{
			IRowSettingsPtr pSelectedRow = m_SelectedList->GetFirstRow();
			while(pSelectedRow) {

				long nProductID = VarLong(pSelectedRow->GetValue(plcID));
				if(!strProductIDs.IsEmpty()) {
					strProductIDs += ",";
				}
				strProductIDs += AsString(nProductID);
				aryServiceIDs.push_back(nProductID);

				// (j.jones 2010-01-13 10:24) - PLID 36847 - handle our per-location fields
				// (j.gruber 2012-10-29 13:35) - PLID 53241 - added shop fee per location

				if(m_checkUpdateBillable.GetCheck() || m_checkUpdateTrackable.GetCheck() || m_checkUpdateReorderPoint.GetCheck()
					|| m_checkUpdateReorderQty.GetCheck() || m_checkUpdateUserResp.GetCheck() || m_checkUpdateShopFeeLoc.GetCheck()) {

					// (j.jones 2010-01-13 10:24) - PLID 36847 - if we update any per-location record,
					//we have to ensure the ProductLocationInfoT record exists, so check for each product,
					//and create with default values, as we will shortly be updating those values
				
					AddStatementToSqlBatch(strSqlBatch, "IF NOT EXISTS "
						"(SELECT ProductID FROM ProductLocationInfoT WHERE ProductID = %li AND LocationID = %li) "
						"BEGIN "
						"INSERT INTO ProductLocationInfoT (ProductID, LocationID, Billable, TrackableStatus) "
						"SELECT ID, %li, "
						"COALESCE((SELECT TOP 1 Billable FROM ProductLocationInfoT WHERE ProductID = ProductT.ID), 0), "
						"COALESCE((SELECT TOP 1 TrackableStatus FROM ProductLocationInfoT WHERE ProductID = ProductT.ID), 0) FROM ProductT "
						"WHERE ProductT.ID = %li "
						"END",
						nProductID, nLocationID, nLocationID, nProductID);

					// (j.gruber 2012-10-29 13:36) - PLID 53241 - do ServiceLocationInfoT, I don't expect this to actually add anything, but just in case
					AddStatementToSqlBatch(strSqlBatch, "IF NOT EXISTS "
						"(SELECT ServiceID FROM ServiceLocationInfoT WHERE ServiceID = %li AND LocationID = %li) "
						"BEGIN "
						"INSERT INTO ServiceLocationInfoT (ServiceID, LocationID, ShopFee) "
						"SELECT ServiceT.ID, %li, "
						"COALESCE((SELECT TOP 1 ShopFee FROM ServiceLocationInfoT WHERE ServiceID = ServiceT.ID), 0) "
						" FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
						"WHERE ServiceT.ID = %li "
						"END",
						nProductID, nLocationID, nLocationID, nProductID);

				}

				pSelectedRow = pSelectedRow->GetNextRow();
			}
		}

		if(strProductIDs.IsEmpty()) {
			AfxMessageBox("There are no products in the selected list. No changes will be made.");
			return;
		}

		//now build our save strings

		//save the global fields (the ones that are not per-location)

		//Category
		if(m_checkUpdateCategory.GetCheck()) {
			
			// (j.jones 2015-03-16 09:15) - PLID 64972 - added ability to select multiple categories,
			// all saving will be done later in this function
			strGlobalItemsToBeChanged += "- Category\n";
		}

		//Supplier
		if(m_checkUpdateSupplier.GetCheck()) {
			IRowSettingsPtr pSupRow = m_SupplierCombo->GetCurSel();
			if(pSupRow == NULL) {
				AfxMessageBox("You have chosen to update the supplier, but no supplier selection has been made. "
					"Please correct this before applying changes.");

				//rollback our audits
				if(nAuditTransactionID != -1) {
					RollbackAuditTransaction(nAuditTransactionID);
				}
				return;
			}

			//in all cases, we want to clear out all existing suppliers
			AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductT SET DefaultMultiSupplierID = NULL WHERE ID IN (%s)", strProductIDs);
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MultiSupplierT WHERE ProductID IN (%s)", strProductIDs);

			long nSupplierID = VarLong(pSupRow->GetValue(sccID), -1);
			//if they want "no supplier", we don't need to do anything else,
			//just add the new supplier if they picked one
			if(nSupplierID != -1) {
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO MultiSupplierT (SupplierID, ProductID) "
					"SELECT %li, ID FROM ProductT WHERE ID IN (%s)", nSupplierID, strProductIDs);

				//now we have to make sure it's the default for each product
				//the select statement will only return one record since we are clearing the existing
				//data and adding one new entry all in one execute batch
				AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductT SET DefaultMultiSupplierID = "
					"(SELECT MultiSupplierT.ID FROM MultiSupplierT WHERE MultiSupplierT.ProductID = ProductT.ID) "
					"WHERE ProductT.ID IN (%s)", strProductIDs);
			}

			//audit for each product that changed
			IRowSettingsPtr pSelectedRow = m_SelectedList->GetFirstRow();
			while(pSelectedRow) {

				long nProductID = VarLong(pSelectedRow->GetValue(plcID));
				CString strProductName = VarString(pSelectedRow->GetValue(plcName));
				CString strOldSupplierName = VarString(pSelectedRow->GetValue(plcSupplier), "<No Supplier>");

				CString strNewValue = "<No Supplier>";
				if(nSupplierID != -1) {
					strNewValue = VarString(pSupRow->GetValue(sccSupplier));
				}

				if(strOldSupplierName != strNewValue) {

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(-1, strProductName, nAuditTransactionID, aeiProductSupplier, nProductID, strOldSupplierName, strNewValue, aepMedium, aetChanged);
				}

				pSelectedRow = pSelectedRow->GetNextRow();
			}

			strGlobalItemsToBeChanged += "- Supplier\n";
		}

		//Taxable 1
		if(m_checkUpdateTaxable1.GetCheck()) {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET Taxable1 = %li WHERE ID IN (%s)",
				m_checkTaxable1.GetCheck() ? 1 : 0, strProductIDs);

			strGlobalItemsToBeChanged += "- Taxable 1\n";
		}

		//Taxable 2
		if(m_checkUpdateTaxable2.GetCheck()) {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET Taxable2 = %li WHERE ID IN (%s)",
				m_checkTaxable2.GetCheck() ? 1 : 0, strProductIDs);

			strGlobalItemsToBeChanged += "- Taxable 2\n";
		}

		//Revenue Code
		if(m_checkUpdateRevCode.GetCheck()) {

			IRowSettingsPtr pRevRow = m_RevCodeCombo->GetCurSel();
			if(pRevRow == NULL) {
				AfxMessageBox("You have chosen to update the revenue code, but no revenue code selection has been made. "
					"Please correct this before applying changes.");

				//rollback our audits
				if(nAuditTransactionID != -1) {
					RollbackAuditTransaction(nAuditTransactionID);
				}
				return;
			}

			long nRevCodeID = VarLong(pRevRow->GetValue(rcccID), -1);

			//in all cases, we won't clear out any selections made for "multiple",
			//just toggle the RevCodeUse field

			if(nRevCodeID == -1) {
				//they want no code at all (we will clear this code, and unselect "single")
				AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET RevCodeUse = 0, UB92Category = NULL WHERE ID IN (%s)", strProductIDs);
			}
			else {				
				AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET RevCodeUse = 1, UB92Category = %li WHERE ID IN (%s)", nRevCodeID, strProductIDs);
			}

			strGlobalItemsToBeChanged += "- Revenue Code\n";
		}

		//Shop Fee // (j.gruber 2012-10-29 13:39) - PLID  53241 - All Locations
		if(IsSpa(FALSE) && m_checkUpdateShopFee.GetCheck()) {
			CString strShopFee;
			GetDlgItemText(IDC_PROD_SHOP_FEE, strShopFee);

			COleCurrency cyShopFee;
			if(!cyShopFee.ParseCurrency(strShopFee) || cyShopFee.GetStatus() == COleCurrency::invalid) {
				AfxMessageBox("The shop fee you have entered is invalid. Please correct this before applying changes.");

				//rollback our audits
				if(nAuditTransactionID != -1) {
					RollbackAuditTransaction(nAuditTransactionID);
				}
				return;
			}

			SetDlgItemText(IDC_PROD_SHOP_FEE, FormatCurrencyForInterface(cyShopFee));

			AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceLocationInfoT SET ShopFee = Convert(money, '%s') "
				" FROM ServiceLocationInfoT INNER JOIN LocationsT ON ServiceLocationInfoT.LocationID = LocationsT.ID "
				" WHERE ServiceID IN (%s) AND LocationsT.Managed = 1 AND LocationsT.Active = 1 ",
				FormatCurrencyForSql(cyShopFee), strProductIDs);

			//Get our list of locations
			CArray<long,long> aryLocations;
			NXDATALIST2Lib::IRowSettingsPtr pLocRow = m_LocationCombo->GetFirstRow();
			while (pLocRow) {
				aryLocations.Add(VarLong(pLocRow->GetValue(lccID)));
				pLocRow = pLocRow->GetNextRow();
			}

			//audit for each product that changed
			IRowSettingsPtr pSelectedRow = m_SelectedList->GetFirstRow();
			while(pSelectedRow) {

				long nProductID = VarLong(pSelectedRow->GetValue(plcID));
				CString strProductName = VarString(pSelectedRow->GetValue(plcName));
				COleCurrency cyOldShopFee = VarCurrency(pSelectedRow->GetValue(plcShopFee), COleCurrency(0,0));
				CString strOldValue = InvUtils::GetServiceLocationShopFeeAuditString(nProductID, &aryLocations);

				if(cyOldShopFee != cyShopFee) {

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					
					AuditEvent(-1, strProductName, nAuditTransactionID, aeiShopFee, nProductID, strOldValue, "All Locations: " + FormatCurrencyForInterface(cyShopFee), aepMedium, aetChanged);
				}

				pSelectedRow = pSelectedRow->GetNextRow();
			}

			strGlobalItemsToBeChanged += "- Shop Fee\n";
		}

		// (j.jones 2016-04-07 11:54) - NX-100076 - added ability to mass-update Remember Charge Provider
		if (m_checkUpdateRememberChargeProvider.GetCheck()) {

			AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET RememberChargeProvider = %li WHERE ID IN (%s)",
				m_checkRememberChargeProvider.GetCheck() ? 1 : 0, strProductIDs);

			strGlobalItemsToBeChanged += "- Remember Charge Provider\n";
		}

		// (j.jones 2010-01-13 10:24) - PLID 36847 - handle our per-location fields

		//Billable
		if(m_checkUpdateBillable.GetCheck()) {

			AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductLocationInfoT SET Billable = %li WHERE LocationID = %li AND ProductID IN (%s)",
				m_checkBillable.GetCheck() ? 1 : 0, nLocationID, strProductIDs);

			strLocationItemsToBeChanged += "- Billable\n";
		}

		//Trackable
		if(m_checkUpdateTrackable.GetCheck()) {

			long nTrackableStatus = -1;
			if(m_radioNotTrackable.GetCheck()) {
				nTrackableStatus = 0;
			}
			else if(m_radioTrackOrders.GetCheck()) {
				nTrackableStatus = 1;
			}
			else if(m_radioTrackQuantity.GetCheck()) {
				nTrackableStatus = 2;
			}

			if(nTrackableStatus == -1) {
				//impossible unless no radio button was selected
				AfxMessageBox("You have chosen to update the trackable status, but no tracking selection has been made. "
					"Please correct this before applying changes.");

				//rollback our audits
				if(nAuditTransactionID != -1) {
					RollbackAuditTransaction(nAuditTransactionID);
				}
				return;
			}

			AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductLocationInfoT SET TrackableStatus = %li WHERE LocationID = %li AND ProductID IN (%s)",
				nTrackableStatus, nLocationID, strProductIDs);

			strLocationItemsToBeChanged += "- Trackable Status\n";
		}

		//Reorder Point
		if(m_checkUpdateReorderPoint.GetCheck()) {

			//if they entered nothing, we treat that as zero
			long nReorderPoint = GetDlgItemInt(IDC_PROD_REORDERPOINT);

			AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductLocationInfoT SET ReorderPoint = %li WHERE LocationID = %li AND ProductID IN (%s)",
				nReorderPoint, nLocationID, strProductIDs);

			//audit for each product that changed
			IRowSettingsPtr pSelectedRow = m_SelectedList->GetFirstRow();
			while(pSelectedRow) {

				long nProductID = VarLong(pSelectedRow->GetValue(plcID));
				CString strProductName = VarString(pSelectedRow->GetValue(plcName));
				long nOldReorderPoint = VarLong(pSelectedRow->GetValue(plcReorderPoint), 0);

				if(nOldReorderPoint != nReorderPoint) {

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(-1, strProductName, nAuditTransactionID, aeiProductReOrderPoint, nProductID, AsString(nOldReorderPoint), AsString(nReorderPoint), aepMedium, aetChanged);
				}

				pSelectedRow = pSelectedRow->GetNextRow();
			}

			strLocationItemsToBeChanged += "- Reorder Point\n";
		}

		//Reorder Quantity
		if(m_checkUpdateReorderQty.GetCheck()) {

			//if they entered nothing, we treat that as zero
			long nReorderQty = GetDlgItemInt(IDC_PROD_REORDERQUANTITY);

			AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductLocationInfoT SET ReorderQuantity = %li WHERE LocationID = %li AND ProductID IN (%s)",
				nReorderQty, nLocationID, strProductIDs);

			//audit for each product that changed
			IRowSettingsPtr pSelectedRow = m_SelectedList->GetFirstRow();
			while(pSelectedRow) {

				long nProductID = VarLong(pSelectedRow->GetValue(plcID));
				CString strProductName = VarString(pSelectedRow->GetValue(plcName));
				long nOldReorderQty = VarLong(pSelectedRow->GetValue(plcReorderQty), 0);

				if(nOldReorderQty != nReorderQty) {

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(-1, strProductName, nAuditTransactionID, aeiProductReorderQuantity, nProductID, AsString(nOldReorderQty), AsString(nReorderQty), aepMedium, aetChanged);
				}

				pSelectedRow = pSelectedRow->GetNextRow();
			}

			strLocationItemsToBeChanged += "- Reorder Quantity\n";
		}

		//User Responsible For Ordering
		if(m_checkUpdateUserResp.GetCheck()) {

			IRowSettingsPtr pUserRow = m_UserCombo->GetCurSel();
			if(pUserRow == NULL) {
				AfxMessageBox("You have chosen to update the assigned user, but no user selection has been made. "
					"Please correct this before applying changes.");

				//rollback our audits
				if(nAuditTransactionID != -1) {
					RollbackAuditTransaction(nAuditTransactionID);
				}
				return;
			}

			long nUserID = VarLong(pUserRow->GetValue(uccID), -1);

			//in all cases, clear the currently assigned user
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ProductResponsibilityT WHERE LocationID = %li AND ProductID IN (%s)",
				nLocationID, strProductIDs);

			if(nUserID != -1) {
				//they picked a user, save it
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ProductResponsibilityT (ProductID, LocationID, UserID) "
					"SELECT ID, %li, %li FROM ProductT WHERE ID IN (%s)", nLocationID, nUserID, strProductIDs);
			}

			strLocationItemsToBeChanged += "- User Responsible For Ordering\n";
		}

		// (j.gruber 2012-10-29 13:41) - PLID 53241 - Shop Fee Per location
		if(IsSpa(FALSE) && m_checkUpdateShopFeeLoc.GetCheck()) {
			CString strShopFee;
			GetDlgItemText(IDC_PROD_SHOP_FEE_LOC, strShopFee);

			COleCurrency cyShopFee;
			if(!cyShopFee.ParseCurrency(strShopFee) || cyShopFee.GetStatus() == COleCurrency::invalid) {
				AfxMessageBox("The shop fee per location you have entered is invalid. Please correct this before applying changes.");

				//rollback our audits
				if(nAuditTransactionID != -1) {
					RollbackAuditTransaction(nAuditTransactionID);
				}
				return;
			}

			SetDlgItemText(IDC_PROD_SHOP_FEE_LOC, FormatCurrencyForInterface(cyShopFee));

			AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceLocationInfoT SET ShopFee = Convert(money, '%s') WHERE ServiceID IN (%s) AND LocationID = %li",
				FormatCurrencyForSql(cyShopFee), strProductIDs, nLocationID);

			//audit for each product that changed
			IRowSettingsPtr pSelectedRow = m_SelectedList->GetFirstRow();			
			while(pSelectedRow) {

				long nProductID = VarLong(pSelectedRow->GetValue(plcID));
				CString strProductName = VarString(pSelectedRow->GetValue(plcName));
				COleCurrency cyOldShopFee = VarCurrency(pSelectedRow->GetValue(plcShopFee), COleCurrency(0,0));
				

				if(cyOldShopFee != cyShopFee) {

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(-1, strProductName, nAuditTransactionID, aeiShopFee, nProductID, strLocationName + ": " + FormatCurrencyForInterface(cyOldShopFee), FormatCurrencyForInterface(cyShopFee), aepMedium, aetChanged);
				}

				pSelectedRow = pSelectedRow->GetNextRow();
			}

			strLocationItemsToBeChanged += "- Shop Fee\n";
		}

		//Insurance Code
		if (m_checkUpdateInsuranceCode.GetCheck())
		{
			CString strInsuranceCode;
			GetDlgItemText(IDC_PROD_INSURANCE_CODE, strInsuranceCode);

			AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductT SET InsCode = '%s' WHERE ID IN (%s)",
				strInsuranceCode, strProductIDs);

			//audit for each product that changed
			IRowSettingsPtr pSelectedRow = m_SelectedList->GetFirstRow();
			while (pSelectedRow) {

				long nProductID = VarLong(pSelectedRow->GetValue(plcID));
				CString strProductName = VarString(pSelectedRow->GetValue(plcName));
				CString strOldInsuranceCode = VarString(pSelectedRow->GetValue(plcInsCode));

				if (strOldInsuranceCode != strInsuranceCode) {

					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(-1, strProductName, nAuditTransactionID, aeiUpdateChargedProductInsCode, nProductID, strOldInsuranceCode, strInsuranceCode, aepMedium, aetChanged);
				}

				pSelectedRow = pSelectedRow->GetNextRow();
			}

			strGlobalItemsToBeChanged += "- Insurance Code\n";
		}

		// (j.jones 2015-03-16 09:54) - PLID 64972 - if they checked only category,
		// the batch really will be empty
		if (strSqlBatch.IsEmpty() && !m_checkUpdateCategory.GetCheck()) {

			//nothing was selected to be updated!
			AfxMessageBox("No fields have been selected to be updated. No changes will be made.");

			//should be impossible, but rollback anyways
			if (nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			return;
		}
			
		//warn first
		CString strWarning;
		if(!strGlobalItemsToBeChanged.IsEmpty()) {
			strWarning.Format("You are about to update the following fields for all selected products:\n"
				"%s\n", strGlobalItemsToBeChanged);
		}

		if(!strLocationItemsToBeChanged.IsEmpty()) {		
			if(!strWarning.IsEmpty()) {
				CString strNextWarning;
				strNextWarning.Format("In addition, the following fields will be updated for the location '%s':\n"
					"%s\n", strLocationName, strLocationItemsToBeChanged);
				strWarning += strNextWarning;
			}
			else {
				strWarning.Format("You are about to update the following fields for all selected products "
					"for the location '%s':\n"
					"%s\n", strLocationName, strLocationItemsToBeChanged);
			}
		}

		strWarning += "Are you sure you wish to continue?";

		if(IDNO == MessageBox(strWarning, "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {

			//rollback our audits
			if(nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
			return;
		}

		CWaitCursor pWait;

		if (!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);

			if (nAuditTransactionID != -1) {
				CommitAuditTransaction(nAuditTransactionID);
				nAuditTransactionID = -1;
			}

			//refresh network code
			CClient::RefreshTable(NetUtils::Products);
		}

		// (j.jones 2015-03-16 09:30) - PLID 64972 - update en masse using the API,
		// this will also audit and send a tablechecker
		if (m_checkUpdateCategory.GetCheck()) {
			UpdateServiceCategories(aryServiceIDs, m_aryCategoryIDs, m_nDefaultCategoryID, false);
		}

		//reload the product lists
		RefreshProductLists(nLocationID);

		//do not unselect or alter any of the settings, they may wish to re-apply them
		//to other products (especially the per-location settings)

	} NxCatchAllCall(__FUNCTION__,
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

void CUpdateProductsDlg::OnBtnSelectOne()
{
	try {

		m_SelectedList->TakeCurrentRowAddSorted(m_UnselectedList, NULL);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnBtnSelectAll()
{
	try {

		m_SelectedList->TakeAllRows(m_UnselectedList);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnBtnUnselectOne()
{
	try {

		m_UnselectedList->TakeCurrentRowAddSorted(m_SelectedList, NULL);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnBtnUnselectAll()
{
	try {

		m_UnselectedList->TakeAllRows(m_SelectedList);

	} NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CUpdateProductsDlg, CNxDialog)
	ON_EVENT(CUpdateProductsDlg, IDC_UPDATE_PRODUCT_UNSELECTED_LIST, 3, OnDblClickCellUnselectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CUpdateProductsDlg, IDC_UPDATE_PRODUCT_SELECTED_LIST, 3, OnDblClickCellSelectedList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CUpdateProductsDlg, IDC_UPDATE_PROD_LOCATION_COMBO, 16, CUpdateProductsDlg::OnSelChosenLocationCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CUpdateProductsDlg::OnDblClickCellUnselectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		
		m_UnselectedList->PutCurSel(pRow);
		
		if(pRow) {
			OnBtnSelectOne();
		}

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnDblClickCellSelectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		
		m_SelectedList->PutCurSel(pRow);
		
		if(pRow) {
			OnBtnUnselectOne();
		}

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnBtnCloseProductUpdater()
{
	try {

		CNxDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-01-13 10:12) - PLID 36847 - added per-location controls
void CUpdateProductsDlg::OnSelChosenLocationCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {		
			pRow = m_LocationCombo->SetSelByColumn(lccID, GetCurrentLocationID());
			if(pRow == NULL) {
				//should not be possible
				ThrowNxException("Current location is not available!");
			}
		}

		long nLocationID = VarLong(pRow->GetValue(lccID));

		//show just that location's users
		CString strUserWhere;
		strUserWhere.Format("PersonT.Archived = 0 AND UsersT.PersonID > 0 AND UserLocationT.LocationID = %li", nLocationID);
		if(CString((LPCTSTR)m_UserCombo->WhereClause) != strUserWhere) {

			//try to re-select the same user
			long nUserID = -1;			
			{
				IRowSettingsPtr pSelRow = m_UserCombo->GetCurSel();
				if(pSelRow) {
					nUserID = VarLong(pSelRow->GetValue(uccID), -1);
				}
			}

			m_UserCombo->PutWhereClause((LPCTSTR)strUserWhere);
			m_UserCombo->Requery();

			//add the "No User" row
			IRowSettingsPtr pUserRow = m_UserCombo->GetNewRow();
			pUserRow->PutValue(uccID, (long)-1);
			pUserRow->PutValue(uccUsername, " <No Assigned User>");
			m_UserCombo->AddRowSorted(pUserRow, NULL);

			if(nUserID != -1) {
				m_UserCombo->SetSelByColumn(uccID, nUserID);
			}
		}

		//refresh the product info. for this location
		RefreshProductLists(nLocationID);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnCheckUpdateCategory()
{
	try {

		BOOL bEnabled = m_checkUpdateCategory.GetCheck();
		m_btnSelectCategory.EnableWindow(bEnabled);
		m_btnRemoveCategory.EnableWindow(bEnabled);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnCheckUpdateSupplier()
{
	try {

		BOOL bEnabled = m_checkUpdateSupplier.GetCheck();
		m_SupplierCombo->PutEnabled(bEnabled);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnCheckUpdateTaxable1()
{
	try {

		BOOL bEnabled = m_checkUpdateTaxable1.GetCheck();
		m_checkTaxable1.EnableWindow(bEnabled);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnCheckUpdateTaxable2()
{
	try {

		BOOL bEnabled = m_checkUpdateTaxable2.GetCheck();
		m_checkTaxable2.EnableWindow(bEnabled);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnCheckUpdateRevCode()
{
	try {

		BOOL bEnabled = m_checkUpdateRevCode.GetCheck();
		m_RevCodeCombo->PutEnabled(bEnabled);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnCheckUpdateShopFee()
{
	try {
		// (j.gruber 2012-10-29 12:44) - PLID 53241 - check to see if they want to leave this checked
		if (CheckConflict(IDC_CHECK_UPDATE_SHOP_FEE) == IDC_CHECK_UPDATE_SHOP_FEE) {
			BOOL bEnabled = m_checkUpdateShopFee.GetCheck();
			m_nxeditShopFee.EnableWindow(bEnabled);
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-01-13 10:12) - PLID 36847 - added per-location controls
void CUpdateProductsDlg::OnCheckUpdateBillable()
{
	try {

		BOOL bEnabled = m_checkUpdateBillable.GetCheck();
		m_checkBillable.EnableWindow(bEnabled);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnCheckUpdateTrackable()
{
	try {

		BOOL bEnabled = m_checkUpdateTrackable.GetCheck();
		m_radioNotTrackable.EnableWindow(bEnabled);
		m_radioTrackOrders.EnableWindow(bEnabled);
		m_radioTrackQuantity.EnableWindow(bEnabled);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnCheckUpdateReorderPoint()
{
	try {

		BOOL bEnabled = m_checkUpdateReorderPoint.GetCheck();
		m_nxeditReorderPoint.EnableWindow(bEnabled);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnCheckUpdateReorderQty()
{
	try {

		BOOL bEnabled = m_checkUpdateReorderQty.GetCheck();
		m_nxeditReorderQty.EnableWindow(bEnabled);

	} NxCatchAll(__FUNCTION__);
}

void CUpdateProductsDlg::OnCheckUpdateUserResp()
{
	try {

		BOOL bEnabled = m_checkUpdateUserResp.GetCheck();
		m_UserCombo->PutEnabled(bEnabled);

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-01-13 10:12) - PLID 36847 - the products need to reload by location
void CUpdateProductsDlg::RefreshProductLists(long nLocationID)
{
	try {

		CWaitCursor pWait;

		// (j.jones 2010-01-13 10:12) - PLID 36847 - refresh the product info. for this location
		//remember that it is possible that ProductLocationInfoT may not exist
		// (j.gruber 2012-10-29 13:58) - PLID 53241 - add serviceLocationInfoT
		CString strProductWhere;
		strProductWhere.Format("ServiceT.Active = 1 AND (ProductLocationInfoT.LocationID IS NULL OR ProductLocationInfoT.LocationID = %li) AND (ServiceLocationInfoT.LocationID IS NULL OR ServiceLocationInfoT.LocationID = %li)", nLocationID, nLocationID);

		//retain the current selections
		CString strProductIDs;
		IRowSettingsPtr pSelectedRow = m_SelectedList->GetFirstRow();
		while(pSelectedRow) {

			long nProductID = VarLong(pSelectedRow->GetValue(plcID));
			if(!strProductIDs.IsEmpty()) {
				strProductIDs += ",";
			}
			strProductIDs += AsString(nProductID);

			pSelectedRow = pSelectedRow->GetNextRow();
		}

		CString strIn, strNotIn;
		if(!strProductIDs.IsEmpty()) {
			//they had selected some products, let's keep them selected upon refreshing
			strIn.Format(" AND ProductT.ID IN (%s)", strProductIDs);
			strNotIn.Format(" AND ProductT.ID NOT IN (%s)", strProductIDs);
			
			CString strUnselectedWhere = strProductWhere + strNotIn;
			CString strSelectedWhere = strProductWhere + strIn;

			m_UnselectedList->PutWhereClause((LPCTSTR)strUnselectedWhere);
			m_UnselectedList->Requery();
			m_SelectedList->PutWhereClause((LPCTSTR)strSelectedWhere);
			m_SelectedList->Requery();
		}
		else {
			//nothing selected, so show all products in the unselected list,
			//and nothing at all in the selected list
			m_UnselectedList->PutWhereClause((LPCTSTR)strProductWhere);
			m_UnselectedList->Requery();
			m_SelectedList->Clear();
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-10-29 12:31) - PLID 53241
void CUpdateProductsDlg::OnBnClickedCheckUpdateShopFeeLoc()
{
	try {
		//check to make sure they want to keep this checked
		if (CheckConflict(IDC_CHECK_UPDATE_SHOP_FEE_LOC) == IDC_CHECK_UPDATE_SHOP_FEE_LOC) {
			BOOL bEnabled = m_checkUpdateShopFeeLoc.GetCheck();
			m_nxeditShopFeeLoc.EnableWindow(bEnabled);
		}
	}NxCatchAll(__FUNCTION__);
}
// (j.gruber 2012-10-29 12:45) - PLID 53241
long CUpdateProductsDlg::CheckConflict(long nIDBeingChecked)
{
	//find out other ID
	long nOtherID;
	switch (nIDBeingChecked)  {
		case IDC_CHECK_UPDATE_SHOP_FEE_LOC:
			nOtherID = IDC_CHECK_UPDATE_SHOP_FEE;
		break;

		case IDC_CHECK_UPDATE_SHOP_FEE:
			nOtherID = IDC_CHECK_UPDATE_SHOP_FEE_LOC;
		break;
	}

	//now see if they are both checked
	if (IsDlgButtonChecked(nIDBeingChecked) && IsDlgButtonChecked(nOtherID)) {

		//they'll have to choose which one the want checked, since they can't have both
		long nResult = MsgBox(MB_YESNOCANCEL, "Shop fee can only be updated per location or for all locations, you cannot check both.\n"
			"Would you like to update the shop fee at all locations?\n\n\n"
			"Click Yes to update shop fees at all locations to the same value.\n"
			"Click No to update just the shot fee at the currently selected location."
			"Click cancel to uncheck both shop fee boxes.");

		if (nResult == IDYES) {
			//check the all locations
			CheckDlgButton(IDC_CHECK_UPDATE_SHOP_FEE, 1);
			m_nxeditShopFee.EnableWindow(TRUE);
			//uncheck the per location
			CheckDlgButton(IDC_CHECK_UPDATE_SHOP_FEE_LOC, 0);
			m_nxeditShopFeeLoc.EnableWindow(FALSE);
			
			return IDC_CHECK_UPDATE_SHOP_FEE;
		}
		else if (nResult == IDNO) {
			CheckDlgButton(IDC_CHECK_UPDATE_SHOP_FEE, 0);
			m_nxeditShopFee.EnableWindow(FALSE);
			//check the per location
			CheckDlgButton(IDC_CHECK_UPDATE_SHOP_FEE_LOC, 1);
			m_nxeditShopFeeLoc.EnableWindow(TRUE);

			return IDC_CHECK_UPDATE_SHOP_FEE_LOC;
		}
		else {
			CheckDlgButton(IDC_CHECK_UPDATE_SHOP_FEE, 0);
			m_nxeditShopFee.EnableWindow(FALSE);
			//uncheck the per location
			CheckDlgButton(IDC_CHECK_UPDATE_SHOP_FEE_LOC, 0);
			m_nxeditShopFeeLoc.EnableWindow(FALSE);

			return -1;
		}
	}
	else {
		//just return the one they checked
		return nIDBeingChecked;
	}

}

// (j.jones 2015-03-16 09:15) - PLID 64972 - added ability to select multiple categories
void CUpdateProductsDlg::OnBtnSelectCategory()
{
	try {

		CCategorySelectDlg dlg(this, true, "inventory item");

		if (m_aryCategoryIDs.size() > 0) {
			dlg.m_aryInitialCategoryIDs.insert(dlg.m_aryInitialCategoryIDs.end(), m_aryCategoryIDs.begin(), m_aryCategoryIDs.end());
			dlg.m_nInitialDefaultCategoryID = m_nDefaultCategoryID;
		}


		if (IDOK == dlg.DoModal()) {

			//this isn't saving anything, just setting our member variables
			m_aryCategoryIDs.clear();
			m_aryCategoryIDs.insert(m_aryCategoryIDs.end(), dlg.m_arySelectedCategoryIDs.begin(), dlg.m_arySelectedCategoryIDs.end());
			m_nDefaultCategoryID = dlg.m_nSelectedDefaultCategoryID;

			//now load the proper display string
			CString strNewCategoryNames;
			LoadServiceCategories(m_aryCategoryIDs, m_nDefaultCategoryID, strNewCategoryNames);

			SetDlgItemText(IDC_CATEGORY_BOX, strNewCategoryNames);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-16 09:15) - PLID 64972 - added ability to select multiple categories
void CUpdateProductsDlg::OnBtnRemoveCategory()
{
	try {

		//no need to warn, just clear the categories, they have not been applied yet
		m_aryCategoryIDs.clear();
		m_nDefaultCategoryID = -1;

		SetDlgItemText(IDC_CATEGORY_BOX, "");

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-04-07 11:54) - NX-100076 - added ability to mass-update Remember Charge Provider
void CUpdateProductsDlg::OnCheckUpdateRememberChargeProvider()
{
	try {

		BOOL bEnabled = m_checkUpdateRememberChargeProvider.GetCheck();
		m_checkRememberChargeProvider.EnableWindow(bEnabled);

	}NxCatchAll(__FUNCTION__);
}


void CUpdateProductsDlg::OnCheckUpdateInsuranceCode()
{
	try
	{
		BOOL bEnabled = m_checkUpdateInsuranceCode.GetCheck();
		m_nxeditInsuranceCode.EnableWindow(bEnabled);
	} NxCatchAll(__FUNCTION__);
}

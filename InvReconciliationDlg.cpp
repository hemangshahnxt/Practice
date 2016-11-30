// InvReconciliationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InvReconciliationDlg.h"
#include "SingleSelectDlg.h"
#include "InvUtils.h"
#include "barcode.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "InvReconciliationAdjustDlg.h"
#include "GlobalFinancialUtils.h"
#include "AuditTrail.h"
#include "GlobalAuditUtils.h"
#include "InvView.h"

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (j.jones 2009-01-07 11:30) - PLID 26141 - created

// CInvReconciliationDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

extern CPracticeApp theApp;

enum ProductListColumns {

	plcID,				//this is used for the parent/child linking, and shouldn't be referenced other than for that purpose
	plcParentID,		//this is used for the parent/child linking, and shouldn't be referenced other than for that purpose
	plcProductID,		//ProductT.ID
	plcProductItemID,	//ProductItemsT.ID, NULL on parent rows
	plcInternalID,		//NULL on new reconciliations, otherwise InvReconciliationProductsT.ID on parent rows, and InvReconciliationProductItemsT.ID on child rows
	plcBarcode,			//ServiceT.Barcode
	plcSerialNum,		//ProductItemsT.SerialNum, NULL on parent rows
	plcCounted,			//Whether a product item was counted or not
	plcProductText,		//Either the ServiceT.Name or the ProductItemsT.SerialNum, used for description purposes only	
	plcCalculatedAmt,	//The calculated amount in stock
	plcUserCount,		//The amount the user has counted so far
	plcDifference,		//The difference between plcCalculatedAmt and plcUserCount
	plcAdjust,			// (j.jones 2009-01-15 10:14) - PLID 32684 - tracks whether or not we should adjust the product, or already did
	plcNotes,			//notes on the given product or product item
};

enum SupplierComboColumns {

	sccID = 0,
	sccName,
};

enum CategoryComboColumns {

	cccID = 0,
	cccName,
};

CInvReconciliationDlg::CInvReconciliationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvReconciliationDlg::IDD, pParent)
{
	m_nID = -1;
	m_nLocationID = -1;
	m_bIsBarcodeScanning = FALSE;
	m_bIsClosed = FALSE;
	m_bCanEdit = TRUE;
}

CInvReconciliationDlg::~CInvReconciliationDlg()
{
	try {

		//clear our loaded data
		for(int i=m_aryProducts.GetSize()-1; i>=0; i--) {
			for(int j=m_aryProducts.GetAt(i)->aryProductItems.GetSize()-1; j>=0; j--) {
				delete m_aryProducts.GetAt(i)->aryProductItems.GetAt(j);
			}
			m_aryProducts.GetAt(i)->aryProductItems.RemoveAll();
			delete m_aryProducts.GetAt(i);
		}
		m_aryProducts.RemoveAll();

		//unregister for barcode scans
		if(GetMainFrame()) {
			GetMainFrame()->UnregisterForBarcodeScan(this);
		}

	}NxCatchAll("Error in CInvReconciliationDlg::~CInvReconciliationDlg");
}

void CInvReconciliationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_CANCEL_RECONCILIATION, m_btnCancelReconciliation);
	DDX_Control(pDX, IDC_BTN_SAVE_COMPLETED, m_btnSaveCompleted);
	DDX_Control(pDX, IDC_BTN_SAVE_UNCOMPLETED, m_btnSaveUncompleted);
	DDX_Control(pDX, IDC_INV_REC_DATE_LABEL, m_nxstaticDateLabel);
	DDX_Control(pDX, IDC_EDIT_INV_REC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_PRODUCT_LOCATION_LABEL, m_nxstaticLocationLabel);
}

BEGIN_MESSAGE_MAP(CInvReconciliationDlg, CNxDialog)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BTN_CANCEL_RECONCILIATION, OnBtnCancelReconciliation)
	ON_BN_CLICKED(IDC_BTN_SAVE_COMPLETED, OnBtnSaveCompleted)
	ON_BN_CLICKED(IDC_BTN_SAVE_UNCOMPLETED, OnBtnSaveUncompleted)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
END_MESSAGE_MAP()

// CInvReconciliationDlg message handlers

BOOL CInvReconciliationDlg::OnInitDialog() 
{		
	try {

		CNxDialog::OnInitDialog();

		m_btnCancel.AutoSet(NXB_CANCEL);
		//this isn't really a deletion, but it is similar, and it's the best icon for this button
		m_btnCancelReconciliation.AutoSet(NXB_DELETE);
		m_btnSaveCompleted.AutoSet(NXB_OK);
		m_btnSaveUncompleted.AutoSet(NXB_CLOSE);

		m_nxeditNotes.SetLimitText(2000);

		//register for barcode scans
		if(GetMainFrame()) {
			GetMainFrame()->RegisterForBarcodeScan(this);
		}

		m_ProductList = BindNxDataList2Ctrl(IDC_PRODUCT_LIST, false);
		m_SupplierCombo = BindNxDataList2Ctrl(IDC_INVREC_SUPPLIER_COMBO, true);
		m_CategoryCombo = BindNxDataList2Ctrl(IDC_INVREC_CATEGORY_COMBO, true);

		{
			IRowSettingsPtr pRow = m_SupplierCombo->GetNewRow();
			pRow->PutValue(sccID, (long)-1);
			pRow->PutValue(sccName, " <All Suppliers>");
			m_SupplierCombo->AddRowSorted(pRow, NULL);
			m_SupplierCombo->SetSelByColumn(sccID, (long)-1);
		}

		{
			IRowSettingsPtr pRow = m_CategoryCombo->GetNewRow();
			pRow->PutValue(cccID, (long)-1);
			pRow->PutValue(cccName, " <All Categories>");
			m_CategoryCombo->AddRowSorted(pRow, NULL);
			m_CategoryCombo->SetSelByColumn(cccID, (long)-1);
		}

		//call InitNew() or Load() based on m_nID
		if(m_nID == -1) {
			InitNew();
		}
		else {
			Load();
		}

		// (j.jones 2009-07-09 10:05) - PLID 34826 - just for posterity, if
		// the reconciliation is closed, make sure m_bCanEdit is FALSE
		if(m_bIsClosed) {
			//if closed, we definitely can't edit
			m_bCanEdit = FALSE;
		}

		//if the reconciliation has been closed, disable saving buttons,
		//OnEditingStarting will handle the datalist
		// (j.jones 2009-07-09 09:34) - PLID 34826 - supported permissions
		if(m_bIsClosed || !m_bCanEdit) {
			m_btnSaveUncompleted.EnableWindow(FALSE);
			m_btnSaveCompleted.EnableWindow(FALSE);
			m_btnCancelReconciliation.EnableWindow(FALSE);
			m_nxeditNotes.SetReadOnly(TRUE);
		}

	}NxCatchAll("Error in CInvReconciliationDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInvReconciliationDlg::InitNew()
{
	try {

		ASSERT(m_nID == -1);

		CWaitCursor pWait;

		m_bIsClosed = FALSE;

		SetWindowText("New Inventory Reconciliation");

		//start tracking the time from right now
		m_dtStartDate = COleDateTime::GetCurrentTime();

		//set the date label
		CString strDate;
		strDate.Format("Start Date: %s", FormatDateTimeForInterface(m_dtStartDate, NULL, dtoDate));
		m_nxstaticDateLabel.SetWindowText(strDate);
		m_nxstaticDateLabel.SetFont(&theApp.m_boldFont);

		//if the location ID is -1, we need to assign one, otherwise we need to get the name
		if(m_nLocationID != -1) {
			_RecordsetPtr rsLoc = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", m_nLocationID);
			if(!rsLoc->eof) {
				m_strLocationName = AdoFldString(rsLoc, "Name");
			}
			else {
				ASSERT(FALSE);
				m_nLocationID = -1;
			}
			rsLoc->Close();
		}

		if(m_nLocationID == -1) {		
			//use CreateRecordset since there are no parameters
			_RecordsetPtr rsLoc = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1");
			if(rsLoc->eof) {
				//this should be impossible, how are they logged in?
				ASSERT(FALSE);
				ThrowNxException("There are no managed locations.");
			}
			else if(rsLoc->GetRecordCount() == 1) {
				//use the one managed location
				m_nLocationID = AdoFldLong(rsLoc, "ID");
				m_strLocationName = AdoFldString(rsLoc, "Name");
			}
			else if(rsLoc->GetRecordCount() > 1) {
				CSingleSelectDlg dlg(this);
				dlg.PreSelect(GetCurrentLocationID());
				if(IDOK == dlg.Open("LocationsT", "Managed = 1 AND Active = 1 AND TypeID = 1", "ID", "Name", "Select which Location you are reconciling inventory items for:")) {

					m_nLocationID = dlg.GetSelectedID();
					m_strLocationName = dlg.GetSelectedDisplayValue();
				}
				else {
					AfxMessageBox("You must choose a location in order to start a new Inventory Reconciliation.");
					//close this screen
					CNxDialog::OnCancel();
					return;
				}
			}
			rsLoc->Close();
		}

		if(m_nLocationID == -1) {
			//should be impossible
			ThrowNxException("No location was selected.");
		}

		//make sure they aren't creating two active reconciliations for the same location
		_RecordsetPtr rsCheck = CreateParamRecordset("SELECT ID FROM InvReconciliationsT "
			"WHERE LocationID = {INT} AND CompleteDate Is Null AND CancelDate IS Null", m_nLocationID);
		if(!rsCheck->eof) {
			AfxMessageBox("There is already an active Inventory Reconciliation in progress for this location.\n"
				"You may not have two active Reconciliations at the same location at the same time.");
			//close this screen
			CNxDialog::OnCancel();
			return;
		}
		rsCheck->Close();

		CString strLocationLabel;
		strLocationLabel.Format("Trackable Products for %s", m_strLocationName);
		m_nxstaticLocationLabel.SetWindowText(strLocationLabel);

		//now that we have a location ID, generate a query for all active products
		//that we track quantity for at this location
		// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized
		CSqlFragment sqlInClause(
			"SELECT ProductT.ID FROM ProductT "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
			"WHERE Active = 1 AND TrackableStatus = 2 AND LocationID = {INT}", m_nLocationID);

		// (j.jones 2009-01-07 16:31) - PLID 32648 - added the ability to run GetInventoryItemSql
		// with a query for an IN clause
		// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
		CSqlFragment sqlInventoryItemSql = InvUtils::GetInventoryItemSql(sqlInClause, m_nLocationID);
		
		//select all the product information
		_RecordsetPtr rs = CreateParamRecordset("SELECT InventoryItemsQ.ProductID, "
			"InventoryItemsQ.Name, InventoryItemsQ.Actual AS CalculatedAmount, InventoryItemsQ.Barcode, "
			"InventoryItemsQ.CategoryID "
			"FROM ({SQL}) AS InventoryItemsQ "
			""
			//find all the supplier IDs for all products
			"SELECT SupplierID, ProductID FROM MultiSupplierT "
			""
			//now find all the product items that have not been used,
			//which means we will also include ones that are on active allocations
			//also, only include serial numbered product items
			"SELECT ProductItemsT.ProductID, ProductItemsT.ID AS ProductItemID, "
			"ProductItemsT.SerialNum, "
			"CASE WHEN SerialNum Is Null THEN '<No Serial Num.>' ELSE SerialNum END + "
			//show the status text only if they have the adv. inv. license
			"	CASE WHEN {INT} = 0 THEN '' "
			"		WHEN ProductItemsT.Status = {INT} THEN '  (Purchased Inventory)' "
			"		WHEN ProductItemsT.Status = {INT} THEN '  (Consignment)' "
			"		WHEN ProductItemsT.Status = {INT} THEN '  (Warranty)' "
			"		ELSE '' END "
			"	AS SerializedText, "
			"Convert(bit, 0) AS Counted "			
			"FROM ProductItemsT "
			"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
			"WHERE ServiceT.Active = 1 AND TrackableStatus = 2 AND ProductLocationInfoT.LocationID = {INT} "
			"AND ProductT.HasSerialNum = 1 "
			"AND ProductItemsT.SerialNum Is Not Null "
			"AND ProductItemsT.Deleted = 0 "
			"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"	WHERE Status = {INT} AND ProductItemID Is Not Null) "
			"AND (ProductItemsT.LocationID = {INT} OR ProductItemsT.LocationID Is Null) ",
			sqlInventoryItemSql,
			//show the status text only if they have the adv. inv. license
			g_pLicense->HasCandAModule(CLicense::cflrSilent) ? 1 : 0, InvUtils::pisPurchased, InvUtils::pisConsignment, InvUtils::pisWarranty,
			m_nLocationID, InvUtils::iadsUsed, m_nLocationID);

		while(!rs->eof) {

			ReconciledProduct *pNew = new ReconciledProduct;

			pNew->nProductID = AdoFldLong(rs, "ProductID");
			pNew->nInternalID = -1;
			pNew->strName = AdoFldString(rs, "Name");
			pNew->strBarcode = AdoFldString(rs, "Barcode", "");
			pNew->dblCalculatedAmt = AdoFldDouble(rs, "CalculatedAmount", 0.0);
			pNew->varUserCount = g_cvarNull;
			pNew->bAdjust = TRUE;
			pNew->strNotes = "";
			pNew->nCategoryID = AdoFldLong(rs, "CategoryID", -1);

			m_aryProducts.Add(pNew);

			rs->MoveNext();
		}

		rs = rs->NextRecordset(NULL);

		//now load all the supplier IDs
		while(!rs->eof) {

			long nProductID = AdoFldLong(rs, "ProductID");
			long nSupplierID = AdoFldLong(rs, "SupplierID");

			BOOL bFound = FALSE;
			for(int i=0; i<m_aryProducts.GetSize() && !bFound; i++) {

				ReconciledProduct *pProduct = m_aryProducts.GetAt(i);

				if(pProduct->nProductID == nProductID) {

					bFound = TRUE;

					//if there were a duplicate supplier ID, it would mean
					//bad data, but it's irrelevant in our usage here
					pProduct->arySupplierIDs.Add(nSupplierID);
				}
			}

			rs->MoveNext();
		}
		
		rs = rs->NextRecordset(NULL);

		while(!rs->eof) {

			long nProductID = AdoFldLong(rs, "ProductID");

			//find the product in our list
			ReconciledProduct *pProduct = NULL;
			for(int i=0; i<m_aryProducts.GetSize() && pProduct == NULL; i++) {
				if(m_aryProducts.GetAt(i)->nProductID == nProductID) {
					pProduct = m_aryProducts.GetAt(i);
				}
			}

			if(pProduct) {

				ReconciledProductItem *pNew = new ReconciledProductItem;

				pNew->nProductItemID = AdoFldLong(rs, "ProductItemID");
				pNew->nInternalID = -1;
				pNew->strSerialNum = AdoFldString(rs, "SerialNum", "");
				pNew->strSerializedText = AdoFldString(rs, "SerializedText", "");
				pNew->bCounted = FALSE;
				pNew->strNotes = "";

				pProduct->aryProductItems.Add(pNew);
			}
			else {
				//our recordset returned a product item, but not a matching product
				ASSERT(FALSE);
			}

			rs->MoveNext();
		}
		rs->Close();

		//RefilterList will apply the contents of m_aryProducts to the list, taking the supplier
		//and category filter into account
		RefilterList();

	}NxCatchAll("Error in CInvReconciliationDlg::InitNew");
}

void CInvReconciliationDlg::Load()
{
	try {

		ASSERT(m_nID != -1);

		CWaitCursor pWait;

		//this first recordset has to be called separate from the others because we need
		//to load m_nLocationID for use in GetInventoryItemSql()

		// (j.jones 2009-01-15 09:59) - PLID 32684 - added the Adjust box, which is NULL on product items,
		// and if the reconciliation is completed we will only show it as checked if an adjustment was created
		_RecordsetPtr rs = CreateParamRecordset("SELECT StartDate, LocationID, LocationsT.Name, "
			"Convert(bit, CASE WHEN CompleteDate IS NULL THEN 0 ELSE 1 END) AS Completed, "
			"Convert(bit, CASE WHEN CancelDate IS NULL THEN 0 ELSE 1 END) AS Cancelled, "
			"InvReconciliationsT.Notes "
			"FROM InvReconciliationsT "
			"INNER JOIN LocationsT ON InvReconciliationsT.LocationID = LocationsT.ID "
			"WHERE InvReconciliationsT.ID = {INT}", m_nID);

		if(rs->eof) {
			ThrowNxException("Tried to load Inventory Reconciliations that didn't exist! (ID = %li)", m_nID);
		}

		m_dtStartDate = AdoFldDateTime(rs, "StartDate");
		m_nLocationID = AdoFldLong(rs, "LocationID");
		m_strLocationName = AdoFldString(rs, "Name");

		CString strNotes = AdoFldString(rs, "Notes");
		m_nxeditNotes.SetWindowText(strNotes);

		//there are two ways a reconciliation can be "closed", it can be cancelled or completed
		BOOL bCompleted = AdoFldBool(rs, "Completed", FALSE);
		BOOL bCancelled = AdoFldBool(rs, "Cancelled", FALSE);
		m_bIsClosed = bCompleted || bCancelled;

		CString strWindowText = "Edit Inventory Reconciliation";
		if(bCompleted) {
			strWindowText = "Closed Inventory Reconciliation";
		}
		else if(bCancelled) {
			strWindowText = "Cancelled Inventory Reconciliation";
		}
		SetWindowText(strWindowText);

		// (j.jones 2009-01-15 10:12) - PLID 32684 - if completed, rename the "Adjust?" column to "Adjusted?"
		// to indicate that the checkbox represents whether an adjustment was made
		if(bCompleted) {
			IColumnSettingsPtr pAdjustColumn = m_ProductList->GetColumn(plcAdjust);
			if(pAdjustColumn) {
				pAdjustColumn->PutColumnTitle("Adjusted?");
			}
		}

		//set the date label
		CString strDate;
		strDate.Format("Start Date: %s", FormatDateTimeForInterface(m_dtStartDate, NULL, dtoDate));
		m_nxstaticDateLabel.SetWindowText(strDate);
		m_nxstaticDateLabel.SetFont(&theApp.m_boldFont);

		CString strLocationLabel;
		strLocationLabel.Format("Trackable Products for %s", m_strLocationName);
		m_nxstaticLocationLabel.SetWindowText(strLocationLabel);

		rs->Close();

		//now that we have a location ID, generate a query for products
		//that we track quantity for at this location (independent of whether
		//they exist in this reconciliation or not)
		// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized
		CSqlFragment sqlInClause("SELECT ProductT.ID FROM ProductT "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
			"WHERE TrackableStatus = 2 AND LocationID = {INT}", m_nLocationID);

		// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
		// (j.jones 2009-01-15 09:59) - PLID 32684 - added the Adjust box, which is NULL on product items,
		// and if the reconciliation is completed we will only show it as checked if an adjustment was created
		rs = CreateParamRecordset(
			//select all the product information, filter on products that are active now,
			//or were previously saved
			"SELECT InventoryItemsQ.ProductID, InvReconciliationProductsT.ID AS InternalID, "
			"	InventoryItemsQ.Name, CalculatedAmount, InventoryItemsQ.Actual AS CurrentCalculatedAmount, "
			"	InventoryItemsQ.Barcode, CountedAmount, "
			"	Convert(bit, "
			"		CASE WHEN InvReconciliationsT.CompleteDate Is Not Null THEN "
			"			(CASE WHEN InvReconciliationProductsT.InvAdjustmentID Is Null THEN 0 ELSE 1 END) "
			"		ELSE (CASE WHEN InvReconciliationProductsT.Adjust IS Null THEN 1 ELSE InvReconciliationProductsT.Adjust END) "
			"		END "
			"	) AS Adjust, "
			"	InvReconciliationProductsT.Notes, "
			"	InventoryItemsQ.CategoryID "
			"	FROM ({SQL}) AS InventoryItemsQ "
			"	LEFT JOIN (SELECT * FROM InvReconciliationProductsT WHERE InvReconciliationID = {INT}) AS InvReconciliationProductsT ON InventoryItemsQ.ProductID = InvReconciliationProductsT.ProductID "
			"	LEFT JOIN InvReconciliationsT ON InvReconciliationProductsT.InvReconciliationID = InvReconciliationsT.ID "
			"	WHERE InventoryItemsQ.Active = 1 OR InvReconciliationProductsT.ID Is Not Null "
			""
			//find all the supplier IDs for all products
			"SELECT SupplierID, ProductID FROM MultiSupplierT "
			""
			//find all the product items that have already been saved on this reconciliation
			"SELECT ServiceT.ID AS ProductID, ProductItemsT.ID AS ProductItemID, InvReconciliationProductItemsT.ID AS InternalID, "
			"	ProductItemsT.SerialNum, "
			"	CASE WHEN SerialNum Is Null THEN '<No Serial Num.>' ELSE SerialNum END + "
			//show the status text only if they have the adv. inv. license
			"	CASE WHEN {INT} = 0 THEN '' "
			"		WHEN ProductItemsT.Status = {INT} THEN '  (Purchased Inventory)' "
			"		WHEN ProductItemsT.Status = {INT} THEN '  (Consignment)' "
			"		WHEN ProductItemsT.Status = {INT} THEN '  (Warranty)' "
			"		ELSE '' END "
			"	AS SerializedText, "
			"	InvReconciliationProductItemsT.Counted, InvReconciliationProductItemsT.Notes "
			"	FROM InvReconciliationsT "
			"	INNER JOIN InvReconciliationProductsT ON InvReconciliationsT.ID = InvReconciliationProductsT.InvReconciliationID "
			"	INNER JOIN InvReconciliationProductItemsT ON InvReconciliationProductsT.ID = InvReconciliationProductItemsT.InvReconciliationProductID "
			"	INNER JOIN ServiceT ON InvReconciliationProductsT.ProductID = ServiceT.ID "
			"	INNER JOIN ProductT ON ServiceT.ID = ProductT.ID "
			"	INNER JOIN ProductItemsT ON InvReconciliationProductItemsT.ProductItemID = ProductItemsT.ID "
			"	WHERE InvReconciliationsT.ID = {INT} "
			""
			//now find all the product items that have not been counted yet,
			//and have not been used, which means we will also include ones
			//that are on active allocations
			//also, only include serial numbered product items
			"SELECT ProductItemsT.ProductID, ProductItemsT.ID AS ProductItemID, "
			"ProductItemsT.SerialNum, "
			"CASE WHEN SerialNum Is Null THEN '<No Serial Num.>' ELSE SerialNum END + "
			//show the status text only if they have the adv. inv. license
			"	CASE WHEN {INT} = 0 THEN '' "
			"		WHEN ProductItemsT.Status = {INT} THEN '  (Purchased Inventory)' "
			"		WHEN ProductItemsT.Status = {INT} THEN '  (Consignment)' "
			"		WHEN ProductItemsT.Status = {INT} THEN '  (Warranty)' "
			"		ELSE '' END "
			"	AS SerializedText, "
			"Convert(bit, 0) AS Counted "
			"FROM ProductItemsT "
			"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
			"WHERE ServiceT.Active = 1 AND TrackableStatus = 2 AND ProductLocationInfoT.LocationID = {INT} "
			"AND ProductT.HasSerialNum = 1 "
			"AND ProductItemsT.SerialNum Is Not Null "
			"AND ProductItemsT.Deleted = 0 "
			"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"	WHERE Status = {INT} AND ProductItemID Is Not Null) "
			"AND (ProductItemsT.LocationID = {INT} OR ProductItemsT.LocationID Is Null) "
			"AND ProductT.ID NOT IN (SELECT ProductID FROM InvReconciliationProductsT WHERE InvReconciliationID = {INT}) ",
			InvUtils::GetInventoryItemSql(sqlInClause, m_nLocationID), m_nID,
			//show the status text only if they have the adv. inv. license
			g_pLicense->HasCandAModule(CLicense::cflrSilent) ? 1 : 0, InvUtils::pisPurchased, InvUtils::pisConsignment, InvUtils::pisWarranty,
			m_nID,
			g_pLicense->HasCandAModule(CLicense::cflrSilent) ? 1 : 0, InvUtils::pisPurchased, InvUtils::pisConsignment, InvUtils::pisWarranty,
			m_nLocationID, InvUtils::iadsUsed, m_nLocationID, m_nID);

		BOOL bHasCompletedProducts = FALSE;

		//load all the products
		while(!rs->eof) {

			ReconciledProduct *pNew = new ReconciledProduct;

			pNew->nProductID = AdoFldLong(rs, "ProductID");
			//nInternalID could be -1 if we didn't save a count for it
			pNew->nInternalID = AdoFldLong(rs, "InternalID", -1);
			pNew->strName = AdoFldString(rs, "Name");
			pNew->strBarcode = AdoFldString(rs, "Barcode", "");

			//if we have no internal ID, then use the active count
			if(pNew->nInternalID == -1) {
				pNew->dblCalculatedAmt = AdoFldDouble(rs, "CurrentCalculatedAmount", 0.0);
			}
			else {
				//otherwise, use the stored count
				pNew->dblCalculatedAmt = AdoFldDouble(rs, "CalculatedAmount", 0.0);

				//track that we loaded at least one completed product
				bHasCompletedProducts = TRUE;
			}

			pNew->varUserCount = rs->Fields->Item["CountedAmount"]->Value;
			pNew->bAdjust = AdoFldBool(rs, "Adjust", TRUE);
			pNew->strNotes = AdoFldString(rs, "Notes", "");
			pNew->nCategoryID = AdoFldLong(rs, "CategoryID", -1);

			m_aryProducts.Add(pNew);

			rs->MoveNext();
		}

		rs = rs->NextRecordset(NULL);

		//now load all the supplier IDs
		while(!rs->eof) {

			long nProductID = AdoFldLong(rs, "ProductID");
			long nSupplierID = AdoFldLong(rs, "SupplierID");

			BOOL bFound = FALSE;
			for(int i=0; i<m_aryProducts.GetSize() && !bFound; i++) {

				ReconciledProduct *pProduct = m_aryProducts.GetAt(i);

				if(pProduct->nProductID == nProductID) {

					bFound = TRUE;

					//if there were a duplicate supplier ID, it would mean
					//bad data, but it's irrelevant in our usage here
					pProduct->arySupplierIDs.Add(nSupplierID);
				}
			}

			rs->MoveNext();
		}
		
		rs = rs->NextRecordset(NULL);

		//load all the product items already counted
		while(!rs->eof) {

			long nProductID = AdoFldLong(rs, "ProductID");

			//find the product in our list
			ReconciledProduct *pProduct = NULL;
			for(int i=0; i<m_aryProducts.GetSize() && pProduct == NULL; i++) {
				if(m_aryProducts.GetAt(i)->nProductID == nProductID) {
					pProduct = m_aryProducts.GetAt(i);
				}
			}

			if(pProduct) {

				ReconciledProductItem *pNew = new ReconciledProductItem;

				pNew->nProductItemID = AdoFldLong(rs, "ProductItemID");
				pNew->nInternalID = AdoFldLong(rs, "InternalID");
				pNew->strSerialNum = AdoFldString(rs, "SerialNum", "");
				pNew->strSerializedText = AdoFldString(rs, "SerializedText", "");
				pNew->bCounted = AdoFldBool(rs, "Counted", FALSE);
				pNew->strNotes = AdoFldString(rs, "Notes", "");

				pProduct->aryProductItems.Add(pNew);
			}
			else {
				//our recordset returned a product item, but not a matching product
				ASSERT(FALSE);
			}

			rs->MoveNext();
		}

		rs = rs->NextRecordset(NULL);

		//load all the product items we haven't counted
		while(!rs->eof) {

			long nProductID = AdoFldLong(rs, "ProductID");

			//find the product in our list
			ReconciledProduct *pProduct = NULL;
			for(int i=0; i<m_aryProducts.GetSize() && pProduct == NULL; i++) {
				if(m_aryProducts.GetAt(i)->nProductID == nProductID) {
					pProduct = m_aryProducts.GetAt(i);
				}
			}

			if(pProduct) {

				ReconciledProductItem *pNew = new ReconciledProductItem;

				pNew->nProductItemID = AdoFldLong(rs, "ProductItemID");
				pNew->nInternalID = -1;
				pNew->strSerialNum = AdoFldString(rs, "SerialNum", "");
				pNew->strSerializedText = AdoFldString(rs, "SerializedText", "");
				pNew->bCounted = FALSE;
				pNew->strNotes = "";

				pProduct->aryProductItems.Add(pNew);
			}
			else {
				//our recordset returned a product item, but not a matching product
				ASSERT(FALSE);
			}

			rs->MoveNext();
		}

		rs->Close();

		//warn if they have some already-saved problems, only if this is an open reconciliation
		if(bHasCompletedProducts && !m_bIsClosed && m_bCanEdit) {
			MessageBox("Some of the products on this reconciliation have already been counted. "
				"You will not be able to change the count on these products.", "Practice", MB_ICONINFORMATION|MB_YESNO);
		}

		//RefilterList will apply the contents of m_aryProducts to the list, taking the supplier
		//and category filter into account
		RefilterList();

	}NxCatchAll("Error in CInvReconciliationDlg::Load");
}

BOOL CInvReconciliationDlg::Save(InvRecSaveFlags irsfSaveFlag)
{
	long nAuditTransactionID = -1;
	CArray<ProductNeedingAdjustments*, ProductNeedingAdjustments*> arypProductsNeedingAdjustments;

	try {

		//if closed, Save() should have never been called
		// (j.jones 2009-07-09 09:34) - PLID 34826 - supported permissions
		if(m_bIsClosed || !m_bCanEdit) {
			ASSERT(FALSE);
			return FALSE;
		}

		if(m_aryProducts.GetSize() == 0) {
			AfxMessageBox("There are no trackable products for this location. Nothing can be saved.");
			return FALSE;
		}

		//validate the data before saving
		if(!Validate(irsfSaveFlag)) {
			return FALSE;
		}

		// (j.jones 2009-01-15 11:00) - PLID 32684 - prepare adjustments, the user will need to approve them first
		if(!PrepareAdjustments(arypProductsNeedingAdjustments)) {
			//clear the adjustment array before returning
			ClearAdjustmentArray(arypProductsNeedingAdjustments);
			return FALSE;
		}

		CString strNotes;
		m_nxeditNotes.GetWindowText(strNotes);
		
		CString strCompleteDate = "NULL";
		CString strCompletedBy = "NULL";
		CString strCancelDate = "NULL";
		CString strCancelledBy = "NULL";

		if(irsfSaveFlag == irsfCompleted) {
			strCompleteDate = "GetDate()";
			strCompletedBy.Format("%li", GetCurrentUserID());
		}
		else if(irsfSaveFlag == irsfCancelled) {
			strCancelDate = "GetDate()";
			strCancelledBy.Format("%li", GetCurrentUserID());
		}

		CString strSqlBatch;

		if(m_nID == -1) {

			//create a new reconciliation

			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewInvRecID INT");
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewInvRecProductID INT");
			// (j.jones 2009-01-15 14:27) - PLID 32684 - added adjustments
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewProductAdjustmentID INT");
			// (b.spivey, February 17, 2012) - PLID 48080 - Removed the declaration of @nNewProductItemID

			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InvReconciliationsT (LocationID, StartDate, StartedBy, "
				"CompleteDate, CompletedBy, CancelDate, CancelledBy, Notes) "
				"VALUES (%li, Convert(datetime, '%s'), %li, %s, %s, %s, %s, '%s')",
				m_nLocationID, FormatDateTimeForSql(m_dtStartDate, dtoDateTime), GetCurrentUserID(),
				strCompleteDate, strCompletedBy, strCancelDate, strCancelledBy, _Q(strNotes));

			AddStatementToSqlBatch(strSqlBatch, "SET @nNewInvRecID = convert(int, SCOPE_IDENTITY())");

			// (j.jones 2009-07-09 13:23) - PLID 34834 - audit the creation
			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			AuditEvent(-1, "", nAuditTransactionID, aeiInvReconciliationStarted, m_nLocationID, "", "Started for location: " + m_strLocationName, aepMedium, aetCreated);

			//if completed or cancelled, audit as such
			if(irsfSaveFlag == irsfCompleted) {
				AuditEvent(-1, "", nAuditTransactionID, aeiInvReconciliationCompleted, m_nLocationID, "", "Completed for location: " + m_strLocationName, aepMedium, aetChanged);
			}
			else if(irsfSaveFlag == irsfCancelled) {
				AuditEvent(-1, "", nAuditTransactionID, aeiInvReconciliationCancelled, m_nLocationID, "", "Cancelled for location: " + m_strLocationName, aepMedium, aetChanged);
			}

			//now save each product
			for(int i=0; i<m_aryProducts.GetSize(); i++) {

				ReconciledProduct *pProduct = m_aryProducts.GetAt(i);

				long nProductID = pProduct->nProductID;
				double dblCalculatedAmt = pProduct->dblCalculatedAmt;
				_variant_t varUserCount = pProduct->varUserCount;
				CString strCountedAmt = "NULL";
				if(varUserCount.vt == VT_R8) {
					strCountedAmt.Format("%g", VarDouble(varUserCount));
				}
				CString strProductNotes = pProduct->strNotes;
				BOOL bAdjust = pProduct->bAdjust;

				//we will only save a record if we entered a count, or we're finishing this allocation (complete or cancel)

				if(varUserCount.vt == VT_R8 || irsfSaveFlag == irsfCompleted || irsfSaveFlag == irsfCancelled) {

					// (j.jones 2009-01-15 10:21) - PLID 32684 - added Adjust
					AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InvReconciliationProductsT "
						"(InvReconciliationID, ProductID, CalculatedAmount, CountedAmount, Notes, Adjust) "
						"VALUES (@nNewInvRecID, %li, %g, %s, '%s', %li)", nProductID, dblCalculatedAmt, strCountedAmt, _Q(strProductNotes), bAdjust ? 1 : 0);

					AddStatementToSqlBatch(strSqlBatch, "SET @nNewInvRecProductID = convert(int, SCOPE_IDENTITY())");

					// (j.jones 2009-07-09 13:23) - PLID 34834 - audit that we counted this product, if we actually did so
					if(varUserCount.vt == VT_R8) {
						CString strOld, strNew;
						strOld.Format("Listed On Hand: %g (%s)", dblCalculatedAmt, m_strLocationName);
						strNew.Format("Physical Count: %g", VarDouble(varUserCount));
						AuditEvent(-1, pProduct->strName, nAuditTransactionID, aeiInvReconciliationProductCounted, nProductID, strOld, strNew, aepMedium, aetChanged);
					}

					// (j.jones 2009-01-15 14:25) - PLID 32684 - add product adjustments, if needed
					if(arypProductsNeedingAdjustments.GetSize() > 0) {
						CreateAdjustment(strSqlBatch, nAuditTransactionID, nProductID, "@nNewInvRecProductID", arypProductsNeedingAdjustments);
					}

					//now save each product Item
					for(int j=0; j<pProduct->aryProductItems.GetSize(); j++) {

						ReconciledProductItem *pProductItem = pProduct->aryProductItems.GetAt(j);

						long nProductItemID = pProductItem->nProductItemID;
						BOOL bCounted = pProductItem->bCounted;
						CString strProductItemNotes = pProductItem->strNotes;

						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InvReconciliationProductItemsT "
							"(InvReconciliationProductID, ProductItemID, Counted, Notes) "
							"VALUES (@nNewInvRecProductID, %li, %li, '%s')", nProductItemID, bCounted ? 1 : 0, _Q(strProductItemNotes));
					}
				}
			}
		}
		else {
			//update the existing reconciliation

			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewInvRecProductID INT");
			// (j.jones 2009-01-15 14:27) - PLID 32684 - added adjustments
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewProductAdjustmentID INT");
			// (b.spivey, February 17, 2012) - PLID 48080 - Removed declaration of @nNewProductItemID

			AddStatementToSqlBatch(strSqlBatch, "UPDATE InvReconciliationsT SET "
				"CompleteDate = %s, CompletedBy = %s, CancelDate = %s, CancelledBy = %s, Notes = '%s' "
				"WHERE ID = %li",
				strCompleteDate, strCompletedBy, strCancelDate, strCancelledBy, _Q(strNotes), m_nID);

			// (j.jones 2009-07-09 13:23) - PLID 34834 - if completed or cancelled, audit as such
			if(irsfSaveFlag == irsfCompleted) {

				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, "", nAuditTransactionID, aeiInvReconciliationCompleted, m_nLocationID, "", "Completed for location: " + m_strLocationName, aepMedium, aetChanged);
			}
			else if(irsfSaveFlag == irsfCancelled) {
				
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(-1, "", nAuditTransactionID, aeiInvReconciliationCancelled, m_nLocationID, "", "Cancelled for location: " + m_strLocationName, aepMedium, aetChanged);
			}

			//save each product
			for(int i=0; i<m_aryProducts.GetSize(); i++) {

				ReconciledProduct *pProduct = m_aryProducts.GetAt(i);

				long nInternalProductID = pProduct->nInternalID;
				double dblCalculatedAmt = pProduct->dblCalculatedAmt;
				long nProductID = pProduct->nProductID;
				_variant_t varUserCount = pProduct->varUserCount;
				CString strCountedAmt = "NULL";
				if(varUserCount.vt == VT_R8) {
					strCountedAmt.Format("%g", VarDouble(varUserCount));
				}
				CString strProductNotes = pProduct->strNotes;
				BOOL bAdjust = pProduct->bAdjust;

				if(nInternalProductID == -1) {

					//this product was not previously saved, so save it as new			

					//we will only save a record if we entered a count, or we're finishing this allocation (complete or cancel)

					if(varUserCount.vt == VT_R8 || irsfSaveFlag == irsfCompleted || irsfSaveFlag == irsfCancelled) {

						// (j.jones 2009-01-15 10:21) - PLID 32684 - added Adjust
						AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InvReconciliationProductsT "
							"(InvReconciliationID, ProductID, CalculatedAmount, CountedAmount, Notes, Adjust) "
							"VALUES (%li, %li, %g, %s, '%s', %li)", m_nID, nProductID, dblCalculatedAmt, strCountedAmt, _Q(strProductNotes), bAdjust ? 1 : 0);

						AddStatementToSqlBatch(strSqlBatch, "SET @nNewInvRecProductID = convert(int, SCOPE_IDENTITY())");

						// (j.jones 2009-07-09 13:23) - PLID 34834 - audit that we counted this product, if we actually did so
						if(varUserCount.vt == VT_R8) {
							if(nAuditTransactionID == -1) {
								nAuditTransactionID = BeginAuditTransaction();
							}
							CString strOld, strNew;
							strOld.Format("Listed On Hand: %g (%s)", dblCalculatedAmt, m_strLocationName);
							strNew.Format("Physical Count: %g", VarDouble(varUserCount));
							AuditEvent(-1, pProduct->strName, nAuditTransactionID, aeiInvReconciliationProductCounted, nProductID, strOld, strNew, aepMedium, aetChanged);
						}

						// (j.jones 2009-01-15 14:25) - PLID 32684 - add product adjustments, if needed
						if(arypProductsNeedingAdjustments.GetSize() > 0) {
							CreateAdjustment(strSqlBatch, nAuditTransactionID, nProductID, "@nNewInvRecProductID", arypProductsNeedingAdjustments);
						}

						//now save each product Item
						for(int j=0; j<pProduct->aryProductItems.GetSize(); j++) {

							ReconciledProductItem *pProductItem = pProduct->aryProductItems.GetAt(j);

							long nProductItemID = pProductItem->nProductItemID;
							BOOL bCounted = pProductItem->bCounted;
							CString strProductItemNotes = pProductItem->strNotes;

							AddStatementToSqlBatch(strSqlBatch, "INSERT INTO InvReconciliationProductItemsT "
								"(InvReconciliationProductID, ProductItemID, Counted, Notes) "
								"VALUES (@nNewInvRecProductID, %li, %li, '%s')", nProductItemID, bCounted ? 1 : 0, _Q(strProductItemNotes));
						}
					}

				}
				else {				

					//we will only save a record if we entered a count, or we're finishing this allocation (complete or cancel)

					if(varUserCount.vt == VT_R8 || irsfSaveFlag == irsfCompleted || irsfSaveFlag == irsfCancelled) {

						//the only field we can change on existing records is the notes field
						AddStatementToSqlBatch(strSqlBatch, "UPDATE InvReconciliationProductsT "
							"SET Notes = '%s' WHERE ID = %li",
							_Q(strProductNotes), nInternalProductID);

						// (j.jones 2009-01-15 14:25) - PLID 32684 - add product adjustments, if needed
						if(arypProductsNeedingAdjustments.GetSize() > 0) {
							CreateAdjustment(strSqlBatch, nAuditTransactionID, nProductID, AsString(nInternalProductID), arypProductsNeedingAdjustments);
						}

						//now save each product Item
						for(int j=0; j<pProduct->aryProductItems.GetSize(); j++) {

							ReconciledProductItem *pProductItem = pProduct->aryProductItems.GetAt(j);

							long nInternalProductItemID = pProductItem->nInternalID;

							ASSERT(nInternalProductItemID != -1);

							CString strProductItemNotes = pProductItem->strNotes;

							//the only field we can change on existing records is the notes field
							AddStatementToSqlBatch(strSqlBatch, "UPDATE InvReconciliationProductItemsT "
								"SET Notes = '%s' WHERE ID = %li", _Q(strProductItemNotes), nInternalProductItemID);
						}
					}
				}
			}
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

	}NxCatchAllCall("Error in CInvReconciliationDlg::Save - Saving",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}

		//only return false if it failed to save
		return FALSE;
	);

	try {

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

	}NxCatchAllCall("Error in CInvReconciliationDlg::Save - Auditing",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}

		//return true if we get here
		return TRUE;
	);

	try {
		
		//clear the adjustment array before returning
		ClearAdjustmentArray(arypProductsNeedingAdjustments);

		Log("CInvReconciliationDlg::Save - Cleanup - ClearAdjustmentArray success.");

		//we may have made changes to the product totals,
		//if so, update the view if we are viewing the inventory module
		if(GetMainFrame()) {
			CNxTabView *pView = GetMainFrame()->GetActiveView();
			// (j.jones 2011-10-04 13:47) - PLID 45803 - make sure that pView is not NULL
			if(pView && pView->IsKindOf(RUNTIME_CLASS(CInvView))) {
				pView->UpdateView();
			}
		}

		return TRUE;

	}NxCatchAll("Error in CInvReconciliationDlg::Save - Cleanup");

	return TRUE;
}

void CInvReconciliationDlg::OnCancel()
{
	try {

		//this is the regular IDCANCEL behavior, and the "Close Without Saving" button

		//warn if we have never saved a new reconciliation
		if(m_nID == -1) {
			if(IDNO == MessageBox("You have not saved this reconciliation. If you cancel now, nothing will be tracked.\n\n"
				"Are you sure you wish to cancel?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		CNxDialog::OnCancel();

	}NxCatchAll("Error in CInvReconciliationDlg::OnCancel");
}

void CInvReconciliationDlg::OnBtnCancelReconciliation()
{
	try {

		//this option will close out the reconciliation and mark it as cancelled,
		//thus reflecting that it is no longer in progress, but was never finished

		//if they never saved, tell them nothing will be tracked
		if(m_nID == -1) {
			if(IDNO == MessageBox("Because this reconciliation has never been saved, cancelling now will not track this reconciliation as having been started.\n\n"
				"Are you sure you wish to cancel?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}

			//cancel
			CNxDialog::OnCancel();
			return;
		}
		else {
			if(IDNO == MessageBox("Cancelling this reconciliation will close this reconciliation permanently, reflecting that a count was never fully completed.\n"				
				"You will be able to view this reconciliation, but you will not be able to edit it again in the future.\n\n"
				"Any products that have previously been adjusted by this reconciliation will not have those adjustments removed.\n\n"
				"Are you sure you wish to cancel?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		if(!Save(irsfCancelled)) {
			return;
		}

		CNxDialog::OnOK();

	}NxCatchAll("Error in CInvReconciliationDlg::OnBtnCancelReconciliation");
}

void CInvReconciliationDlg::OnBtnSaveCompleted()
{
	try {

		//this option will close out the reconciliation and mark it as completed

		if(IDNO == MessageBox("Completing this reconciliation will close this reconciliation permanently, storing the current information as the final count of all products. "
			"You will be able to view this reconciliation, but you will not be able to edit it again in the future.\n\n"
			"Are you sure you are ready to mark this reconciliation as completed?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		if(!Save(irsfCompleted)) {
			return;
		}

		CNxDialog::OnOK();

	}NxCatchAll("Error in CInvReconciliationDlg::OnBtnSaveCompleted");
}

void CInvReconciliationDlg::OnBtnSaveUncompleted()
{
	try {

		//this option will simply save the changes to the reconciliation,
		//but will not mark it as completed, so the user can come back
		//later and make more changes

		if(!Save(irsfSimpleSave)) {
			return;
		}

		CNxDialog::OnOK();

	}NxCatchAll("Error in CInvReconciliationDlg::OnBtnSaveUncompleted");
}

BEGIN_EVENTSINK_MAP(CInvReconciliationDlg, CNxDialog)
	ON_EVENT(CInvReconciliationDlg, IDC_PRODUCT_LIST, 8, OnEditingStartingProductList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CInvReconciliationDlg, IDC_PRODUCT_LIST, 9, OnEditingFinishingProductList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CInvReconciliationDlg, IDC_PRODUCT_LIST, 10, OnEditingFinishedProductList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CInvReconciliationDlg, IDC_PRODUCT_LIST, 6, OnRButtonDownProductList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CInvReconciliationDlg, IDC_INVREC_SUPPLIER_COMBO, 16, OnSelChosenInvrecSupplierCombo, VTS_DISPATCH)
	ON_EVENT(CInvReconciliationDlg, IDC_INVREC_CATEGORY_COMBO, 16, OnSelChosenInvrecCategoryCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CInvReconciliationDlg::OnEditingStartingProductList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//do not allow editing if closed
		// (j.jones 2009-07-09 09:34) - PLID 34826 - supported permissions
		if(m_bIsClosed || !m_bCanEdit) {
			*pbContinue = FALSE;
			return;
		}

		if(nCol != plcNotes) {
			//if we have an internal ID, disable editing everything except notes
			if(VarLong(pRow->GetValue(plcInternalID), -1) != -1) {
				*pbContinue = FALSE;
				return;
			}
		}

		// (j.jones 2009-01-15 10:14) - PLID 32684 - disable editing the adjustment column on product items
		if(nCol == plcUserCount || nCol == plcAdjust) {

			//if this is a child row, disable editing this column
			if(pRow->GetParentRow()) {
				*pbContinue = FALSE;
				return;
			}
		}

		if(nCol == plcCounted) {

			//if this is a parent row, disable editing this column
			if(pRow->GetParentRow() == NULL) {
				*pbContinue = FALSE;
				return;
			}
		}

	}NxCatchAll("Error in CInvReconciliationDlg::OnEditingStartingProductList");
}

void CInvReconciliationDlg::OnEditingFinishingProductList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == plcUserCount) {

			//validate that the user count is not negative
			if(pvarNewValue->vt == VT_R8 && VarDouble(pvarNewValue) < 0) {

				*pbCommit = FALSE;
				AfxMessageBox("You cannot have a negative count.");
				return;
			}

			//disallow a non-whole number if there are product items
			if(pRow->GetFirstChildRow() && pvarNewValue->vt == VT_R8
				&& VarDouble(pvarNewValue) != (double)(long)VarDouble(pvarNewValue)) {

				*pbCommit = FALSE;
				AfxMessageBox("You must enter a whole number for products that have serial numbered / exp. dated items.");
				return;
			}

			//if the new value is 0, and the old value is NULL, and the string they entered is empty,
			//then all they did was click in and out of a blank cell - so leave it blank,
			//otherwise the datalist is going to change it to 0
			CString strEntered = strUserEntered;
			if(pvarNewValue->vt == VT_R8 && VarDouble(pvarNewValue) == 0.0
				&& varOldValue.vt == VT_NULL && strEntered.IsEmpty()) {

				*pvarNewValue = g_cvarNull;
				*pbCommit = FALSE;
				return;
			}
		}

	}NxCatchAll("Error in CInvReconciliationDlg::OnEditingFinishingProductList");
}

void CInvReconciliationDlg::OnEditingFinishedProductList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(!bCommit) {
			return;
		}

		IRowSettingsPtr pParentRow = pRow->GetParentRow();

		//find the product and product item pointer

		ReconciledProduct *pProduct = NULL;
		ReconciledProductItem *pProductItem = NULL;

		for(int i=0; i<m_aryProducts.GetSize() && pProduct == NULL; i++) {
			
			ReconciledProduct *pProductToCompare = m_aryProducts.GetAt(i);

			long nProductID = -1;
			if(pParentRow) {
				nProductID = VarLong(pParentRow->GetValue(plcProductID));
			}
			else {
				nProductID = VarLong(pRow->GetValue(plcProductID));
			}

			ASSERT(nProductID != -1);

			if(pProductToCompare->nProductID == nProductID) {

				pProduct = pProductToCompare;

				//if we have a parent row, we know this row is a product item
				if(pParentRow) {
					for(int j=0; j<pProduct->aryProductItems.GetSize() && pProductItem == NULL; j++) {
				
						ReconciledProductItem *pProductItemToCompare = pProduct->aryProductItems.GetAt(j);

						if(pProductItemToCompare->nProductItemID == VarLong(pRow->GetValue(plcProductItemID))) {

							pProductItem = pProductItemToCompare;
						}
					}
				}
			}
		}

		//if they edited the user count on a parent row, update the difference
		if(nCol == plcUserCount && pParentRow == NULL) {

			//if the user count is NULL, the difference should be too,
			//though it should be impossible for this to be the case
			//in this function
			if(varNewValue.vt != VT_R8) {
				pRow->PutValue(plcDifference, g_cvarNull);
			}
			else {			
				double dblCalculatedAmt = VarDouble(pRow->GetValue(plcCalculatedAmt), 0.0);
				double dblUserCounted = VarDouble(varNewValue);

				pRow->PutValue(plcDifference, dblCalculatedAmt - dblUserCounted);

				//track this in the product pointer
				if(pProduct) {
					pProduct->varUserCount = dblUserCounted;
				}
				else {
					ASSERT(FALSE);
				}
			}
		}

		//if they changed the Counted checkbox on a child row,
		//update the parent's counted value, and the difference
		if(nCol == plcCounted && pParentRow != NULL) {

			ASSERT(pProduct);
			ASSERT(pProductItem);

			BOOL bCounted = VarBool(varNewValue);

			double dblCalculatedAmt = pProduct->dblCalculatedAmt;
			double dblUserCounted = VarDouble(pProduct->varUserCount, 0.0);

			//it is possible for the "counted" checked items to not add up to the
			//current user count, so we should only change the value if the current
			//user count matches the "counted" amounts prior to our change
			long nCurCounted = CountCheckedProductItemsUnderParent(pProduct);

			//now track the counted status in the product item pointer
			if(pProductItem) {
				pProductItem->bCounted = bCounted;
			}
			else {
				ASSERT(FALSE);
			}

			//only update the count if it matches the user count
			if((double)nCurCounted == dblUserCounted) {
				if(bCounted) {
					//if they checked the box, increment the count
					dblUserCounted += 1.0;
				}
				else {
					//if they unchecked the box, decrement the count				
					dblUserCounted -= 1.0;
				}

				//update the count and the difference
				pParentRow->PutValue(plcUserCount, dblUserCounted);
				pParentRow->PutValue(plcDifference, dblCalculatedAmt - dblUserCounted);

				//track the count in the parent
				if(pProduct) {
					pProduct->varUserCount = dblUserCounted;
				}
				else {
					ASSERT(FALSE);
				}
			}
		}

		//sync up the adjust column
		if(nCol == plcAdjust && pParentRow == NULL) {
			if(pProduct) {
				pProduct->bAdjust = VarBool(varNewValue);
			}
			else {
				ASSERT(FALSE);
			}			
		}

		//sync up the notes column
		if(nCol == plcNotes && pParentRow == NULL) {
			if(pProduct) {
				pProduct->strNotes = VarString(varNewValue, "");
			}
			else {
				ASSERT(FALSE);
			}			
		}
		else if(nCol == plcNotes && pParentRow != NULL) {
			if(pProductItem) {
				pProductItem->strNotes = VarString(varNewValue, "");
			}
			else {
				ASSERT(FALSE);
			}			
		}

	}NxCatchAll("Error in CInvReconciliationDlg::OnEditingFinishedProductList");
}

LRESULT CInvReconciliationDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {

		//if closed, do nothing
		// (j.jones 2009-07-09 09:34) - PLID 34826 - supported permissions
		if(m_bIsClosed || !m_bCanEdit) {
			return 0;
		}

		if(m_bIsBarcodeScanning) {
			return 0;
		}

		m_bIsBarcodeScanning = TRUE;

		CString strBarcode = (LPCTSTR)_bstr_t((BSTR)lParam);

		BOOL bFound = FALSE;

		//see if it is a product barcode, and if so, increment the quantity counted
		for(int i=0; i<m_aryProducts.GetSize() && !bFound; i++) {

			ReconciledProduct *pProduct = m_aryProducts.GetAt(i);

			//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
			//if(pProduct->strBarcode.CompareNoCase(strBarcode) == 0) {
			if(GetBestUPCProduct(pProduct, strBarcode)) {

				//found it
				bFound = TRUE;

				//increment the quantity
				double dblCalculatedAmt = pProduct->dblCalculatedAmt;
				double dblUserCounted = VarDouble(pProduct->varUserCount, 0.0);
				dblUserCounted += 1.0;

				//update the count
				pProduct->varUserCount = dblUserCounted;

				//find and update the row
				BOOL bFoundRow = FALSE;
				IRowSettingsPtr pProductRow = m_ProductList->GetFirstRow();
				while(pProductRow && !bFoundRow) {

					if(VarLong(pProductRow->GetValue(plcProductID)) == pProduct->nProductID) {
						bFoundRow = TRUE;
					}
					else {
						pProductRow = pProductRow->GetNextRow();
					}
				}

				if(bFoundRow) {
					//if we found the row, display it and update its content

					//expand the row, if it has children, and select this row
					if(pProductRow->GetFirstChildRow()) {
						pProductRow->PutExpanded(VARIANT_TRUE);
					}
					m_ProductList->PutCurSel(pProductRow);

					//update the count and the difference
					pProductRow->PutValue(plcUserCount, dblUserCounted);
					pProductRow->PutValue(plcDifference, dblCalculatedAmt - dblUserCounted);
				}
				else {
					//we could not find the row, it must be filtered out, so warn the user
					CString str;
					str.Format("The product '%s' is not currently visible in your filter. Its count has been increased to %g.", pProduct->strName, dblUserCounted);
					AfxMessageBox(str);
				}

				continue;
			}

			//see if it is a product serial number, and if so, check the box,
			//and increment the quantity counted

			for(int j=0; j<pProduct->aryProductItems.GetSize() && !bFound; j++) {

				ReconciledProductItem *pProductItem = pProduct->aryProductItems.GetAt(j);

				if(pProductItem->strSerialNum.CompareNoCase(strBarcode) == 0) {

					//found it
					bFound = TRUE;

					double dblCalculatedAmt = pProduct->dblCalculatedAmt;
					double dblUserCounted = VarDouble(pProduct->varUserCount, 0.0);

					//see if it is already checked
					if(!pProductItem->bCounted) {
											
						//try to increment the quantity of the parent row						

						//it is possible for the "counted" checked items to not add up to the
						//current user count, so we should only change the value if the current
						//user count matches the "counted" amounts prior to our change
						long nCurCounted = CountCheckedProductItemsUnderParent(pProduct);

						//now update the product pointer
						pProductItem->bCounted = TRUE;

						//only update the count if it matches the user count
						if((double)nCurCounted == dblUserCounted) {
						
							dblUserCounted += 1.0;

							//update the count and the difference
							pProduct->varUserCount = dblUserCounted;							
						}
					}

					//find and update the row
					BOOL bFoundProductRow = FALSE;					
					IRowSettingsPtr pProductRow = m_ProductList->GetFirstRow();					
					while(pProductRow && !bFoundProductRow) {

						if(VarLong(pProductRow->GetValue(plcProductID)) == pProduct->nProductID) {
							bFoundProductRow = TRUE;
						}
						else {
							pProductRow = pProductRow->GetNextRow();
						}
					}

					if(bFoundProductRow) {

						//expand the parent and select this row						
						m_ProductList->PutCurSel(pProductRow);
						pProductRow->PutExpanded(VARIANT_TRUE);

						//update the product counts
						pProductRow->PutValue(plcUserCount, dblUserCounted);
						pProductRow->PutValue(plcDifference, dblCalculatedAmt - dblUserCounted);

						//now find the product item

						BOOL bFoundProductItemRow = FALSE;
						IRowSettingsPtr pProductItemRow = pProductRow->GetFirstChildRow();
						while(pProductItemRow && !bFoundProductItemRow) {

							if(VarLong(pProductItemRow->GetValue(plcProductItemID)) == pProductItem->nProductItemID) {
								bFoundProductItemRow = TRUE;
							}
							else {
								pProductItemRow = pProductItemRow->GetNextRow();
							}
						}

						if(bFoundProductItemRow) {						
							//select and update this row
							m_ProductList->PutCurSel(pProductItemRow);
							pProductItemRow->PutValue(plcCounted, pProductItem->bCounted ? g_cvarTrue : g_cvarFalse);
						}
						else {
							//this should be impossible
							ASSERT(FALSE);
						}
					}
					else {
						//we could not find the row, it must be filtered out, so warn the user
						CString str;
						str.Format("The product '%s' is not currently visible in your filter. Its count has been updated to %g.", pProduct->strName, dblUserCounted);
						AfxMessageBox(str);
					}
				}
			}
		}

	} NxCatchAll("Error in CInvReconciliationDlg::OnBarcodeScan");

	m_bIsBarcodeScanning = FALSE;

	return 0;
}
void CInvReconciliationDlg::OnRButtonDownProductList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//disallow this on a closed reconciliation
		// (j.jones 2009-07-09 09:34) - PLID 34826 - supported permissions
		if(m_bIsClosed || !m_bCanEdit) {
			return;
		}

		m_ProductList->CurSel = pRow;

		//if this row has a parent, disable right-clicking, it has no value here
		if(pRow->GetParentRow()) {
			return;
		}

		//add an ability to remove the row
		enum {
			eClearCount = 1,
		};

		CString strClearText;
		strClearText.Format("&Clear the Physical Count for %s", VarString(pRow->GetValue(plcProductText)));

		//if the row doesn't have a count yet, show the opton, but gray it out
		//also show grayed out if the product has already been saved
		DWORD dwEnabled = 0;
		//should be impossible for the user count to be null and the difference not to be, but check anyways
		if((pRow->GetValue(plcUserCount).vt == VT_R8 || pRow->GetValue(plcDifference).vt == VT_R8)
			&& VarLong(pRow->GetValue(plcInternalID), -1) == -1) {
			dwEnabled = MF_ENABLED;
		}
		else {
			dwEnabled = MF_GRAYED;
		}

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(dwEnabled|MF_STRING|MF_BYPOSITION, eClearCount, strClearText);

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eClearCount) {

			//clear the count, and the difference
			pRow->PutValue(plcUserCount, g_cvarNull);
			pRow->PutValue(plcDifference, g_cvarNull);

			//uncheck all children
			{
				IRowSettingsPtr pProductItemRow = pRow->GetFirstChildRow();
				while(pProductItemRow) {

					pProductItemRow->PutValue(plcCounted, _variant_t(VARIANT_FALSE, VT_BOOL));

					pProductItemRow = pProductItemRow->GetNextRow();
				}
			}

			//clear it in the tracked pointers
			BOOL bFound = FALSE;
			for(int i=0; i<m_aryProducts.GetSize() && !bFound; i++) {
			
				ReconciledProduct *pProduct = m_aryProducts.GetAt(i);

				if(pProduct->nProductID == VarLong(pRow->GetValue(plcProductID))) {
					bFound = TRUE;
					
					pProduct->varUserCount = g_cvarNull;

					for(int j=0; j<pProduct->aryProductItems.GetSize(); j++) {

						ReconciledProductItem *pProductItem = pProduct->aryProductItems.GetAt(j);

						pProductItem->bCounted = FALSE;
					}
				}
			}
		}

	} NxCatchAll("Error in CInvReconciliationDlg::OnRButtonDownProductList");
}

//this function will take in a product pointer that has product items under it,
//and return the count of how many product items under it are checked
long CInvReconciliationDlg::CountCheckedProductItemsUnderParent(ReconciledProduct *pProduct)
{
	try {

		if(pProduct == NULL) {
			//no product
			ASSERT(FALSE);
			return 0;
		}

		if(pProduct->aryProductItems.GetSize() == 0) {
			//no product items
			ASSERT(FALSE);
			return 0;
		}

		long nCount = 0;

		for(int i=0; i<pProduct->aryProductItems.GetSize(); i++) {

			ReconciledProductItem *pProductItem = pProduct->aryProductItems.GetAt(i);

			if(pProductItem->bCounted) {
				nCount++;
			}
		}

		return nCount;

	} NxCatchAll("Error in CInvReconciliationDlg::CountCheckedProductItemsUnderParent");

	return 0;
}

BOOL CInvReconciliationDlg::Validate(InvRecSaveFlags irsfSaveFlag)
{
	try {

		//for every product that has serial numbers below it, validate that
		//either the user count matches the calculated amount, or that the
		//user count matches the number of checked off serial numbers

		CString strMismatchedProducts;
		long nFirstMismatchedProductID = -1;
		long nCountMismatchedProducts = 0;

		//we also want to count how many products were not counted at all
		long nUncountedProducts = 0;

		BOOL bWillNotSaveNotes = FALSE;

		for(int i=0; i<m_aryProducts.GetSize(); i++) {

			ReconciledProduct *pProduct = m_aryProducts.GetAt(i);

			double dblUserCounted = 0.0;
			if(pProduct->varUserCount.vt == VT_R8) {
				//track the counted amount
				dblUserCounted = VarDouble(pProduct->varUserCount, 0.0);
			}
			else {
				//if there is no count, track this item as uncounted
				nUncountedProducts++;
			}

			if(!bWillNotSaveNotes && pProduct->varUserCount.vt != VT_R8 && irsfSaveFlag != irsfCompleted && irsfSaveFlag != irsfCancelled
				&& !pProduct->strNotes.IsEmpty()) {

				//If the product is not counted, and the reconciliation is not completed or cancelled,
				//then it will not be saved. So if the user entered notes, we need to warn them that
				//they will not be saved.
					
				bWillNotSaveNotes = TRUE;
			}
			
			//are there child items?
			if(pProduct->aryProductItems.GetSize() > 0) {

				if(!bWillNotSaveNotes && pProduct->varUserCount.vt != VT_R8
					&& irsfSaveFlag != irsfCompleted && irsfSaveFlag != irsfCancelled) {

					//If the product is not counted, and the reconciliation is not completed or cancelled,
					//then it will not be saved. So if the user entered notes, we need to warn them that
					//they will not be saved.

					for(int j=0; j<pProduct->aryProductItems.GetSize(); j++) {

						ReconciledProductItem *pProductItem = pProduct->aryProductItems.GetAt(j);

						if(!pProductItem->strNotes.IsEmpty()) {				
							bWillNotSaveNotes = TRUE;
						}
					}
				}

				//if the user counted all the items (or more), make sure all the products are checked
				if(dblUserCounted >= pProduct->dblCalculatedAmt) {

					for(int j=0; j<pProduct->aryProductItems.GetSize(); j++) {

						ReconciledProductItem *pProductItem = pProduct->aryProductItems.GetAt(j);

						//check each product item
						pProductItem->bCounted = TRUE;

						//check it in the list, if it is currently displayed
						IRowSettingsPtr pProductItemRow = m_ProductList->FindByColumn(plcProductItemID, pProductItem->nProductItemID, m_ProductList->GetFirstRow(), FALSE);
						if(pProductItemRow) {
							pProductItemRow->PutValue(plcCounted, _variant_t(VARIANT_TRUE, VT_BOOL));
						}
					}
				}
				else {
					//see how many are checked
					long nCountChecked = CountCheckedProductItemsUnderParent(pProduct);

					if((double)nCountChecked != dblUserCounted) {

						nCountMismatchedProducts++;

						//track the first mismatched product ID
						if(nFirstMismatchedProductID == -1) {
							nFirstMismatchedProductID = pProduct->nProductID;
						}

						//track the first 10 product names
						if(nCountMismatchedProducts > 10) {
							strMismatchedProducts += "\n<More>";
						}
						else {
							if(!strMismatchedProducts.IsEmpty()) {
								strMismatchedProducts += "\n";
							}
							CString strProductLine;
							strProductLine.Format("%s (%g counted, %li checked)", pProduct->strName, dblUserCounted, nCountChecked);
							strMismatchedProducts += strProductLine;
						}
					}
				}
			}
		}

		if(nCountMismatchedProducts > 0) {

			//select the first row that is mismatched
			IRowSettingsPtr pRow = m_ProductList->SetSelByColumn(plcProductID, (long)nFirstMismatchedProductID);
			//expand the row (if it exists, could be filtered out)
			if(pRow) {
				if(pRow->GetFirstChildRow()) {
					pRow->PutExpanded(VARIANT_TRUE);
				}
				else {
					//should be impossible
					ASSERT(FALSE);
				}
			}

			CString strWarning;
			strWarning.Format("The following products have a different amount of serial numbers checked off than the amount entered in the User Count field:\n\n"
				"%s\n\n"
				"You cannot save this reconciliation without identifying which serial numbers are in stock.", strMismatchedProducts);

			AfxMessageBox(strWarning);
			return FALSE;
		}

		//warn if some product notes will not be saved
		if(bWillNotSaveNotes) {
			if(IDNO == MessageBox("At least one product in this reconciliation has notes entered for it, but no count entered.\n"
				"These notes will not be saved if no count is entered for the product.\n\n"
				"Are you sure you wish to continue saving?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return FALSE;
			}
		}

		//if completing, warn about how many uncounted products they have
		if(irsfSaveFlag == irsfCompleted && nUncountedProducts > 0) {
			
			CString strWarning;
			strWarning.Format("There are %li products in this reconciliation where no count has been entered.\n\n"
				"Are you sure you wish to complete this reconciliation and ignore these products?", nUncountedProducts);

			if(IDNO == MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return FALSE;
			}
		}

		return TRUE;

	} NxCatchAll("Error in CInvReconciliationDlg::Validate");

	return FALSE;
}

// (j.jones 2009-01-15 11:00) - PLID 32684 - PrepareAdjustments should be called only when completing
// a reconciliation, it will show the user which products would be adjusted and let them uncheck the
// "adjust" box for products if they wish. If they don't have permission to make adjustments, or
// they cancel this screen, this function will return FALSE and abort saving
// If adjustments need to be made, the arypProductsNeedingAdjustments array will be filled.
BOOL CInvReconciliationDlg::PrepareAdjustments(CArray<ProductNeedingAdjustments*, ProductNeedingAdjustments*> &arypProductsNeedingAdjustments)
{
	try {

		CWaitCursor pWait;
		CArray<long, long> aryProductIDs;

		//first we need to find all mismatched products that have the Adjust checkbox set
		for(int i=0; i<m_aryProducts.GetSize(); i++) {

			ReconciledProduct *pProduct = m_aryProducts.GetAt(i);

			//we will skip products with no count and those that already have an internal ID (would have already been adjusted)
			if(pProduct->varUserCount.vt == VT_R8 && pProduct->nInternalID == -1) {

				double dblUserCounted = VarDouble(pProduct->varUserCount, 0.0);

				if(pProduct->dblCalculatedAmt - dblUserCounted != 0.0 && pProduct->bAdjust) {
					//this product does indeed need to be adjusted, add it to our list

					ProductNeedingAdjustments *pNew = new ProductNeedingAdjustments;
					pNew->nProductID = pProduct->nProductID;
					pNew->strProductName = pProduct->strName;
					pNew->dblCalculated = pProduct->dblCalculatedAmt;
					pNew->dblUserCounted = dblUserCounted;
					pNew->bAdjust = TRUE;
					pNew->strSqlBatch = "";

					//track this ID for checking on hand amounts later
					// (j.armen 2012-01-04 10:33) - PLID 29253 - Track ID's in an array
					aryProductIDs.Add(pProduct->nProductID);

					//add all the unchecked serial numbers, if any exist, to the removal array
					for(int j=0; j<pProduct->aryProductItems.GetSize(); j++) {

						ReconciledProductItem *pProductItem = pProduct->aryProductItems.GetAt(j);

						if(!pProductItem->bCounted) {
							pNew->aryProductItemIDsToRemove.Add(pProductItem->nProductItemID);
						}
					}

					arypProductsNeedingAdjustments.Add(pNew);
				}
			}
		}

		//if no adjustments are needed, we can return TRUE and move on
		if(arypProductsNeedingAdjustments.GetSize() == 0) {
			return TRUE;
		}

		ASSERT(!aryProductIDs.IsEmpty());

		//before prompting to adjust, we must verify each product's on-hand amount 
		// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
		_RecordsetPtr rs = CreateParamRecordset("SELECT "
			"InventoryItemsQ.Name, InventoryItemsQ.ProductID, InventoryItemsQ.Actual "
			"FROM ({SQL}) AS InventoryItemsQ "
			"ORDER BY InventoryItemsQ.Name", InvUtils::GetInventoryItemSql(aryProductIDs, m_nLocationID));

		while(!rs->eof) {

			long nProductID = AdoFldLong(rs, "ProductID");
			double dblActual = AdoFldDouble(rs, "Actual", 0.0);
			CString strName = AdoFldString(rs, "Name", "");

			//find the product in our master array (not the adjustment array)
			BOOL bFound = FALSE;
			for(int i=0; i<m_aryProducts.GetSize() && !bFound; i++) {

				ReconciledProduct *pProduct = m_aryProducts.GetAt(i);

				if(pProduct->nProductID == nProductID) {
					bFound = TRUE;

					//compare the calculated quantities
					if(dblActual != pProduct->dblCalculatedAmt) {
						
						//they differ, which means someone changed the product's
						//on hand amount while this reconciliation was open!

						double dblCounted = VarDouble(pProduct->varUserCount, 0.0);
						double dblDifference = pProduct->dblCalculatedAmt - dblCounted;

						CString strWarning;
						strWarning.Format("The product '%s' is about to be adjusted based on a physical count of %g compared to an on hand count of %g.\n"
							"However, since opening this reconciliation the on hand count for this product has been changed to %g.\n\n"
							"Do you still wish to continue adjusting and %s %g products?\n\n"
							"If you do not continue, the reconciliation will not be saved, the count for this product will be cleared, and the on hand amount will be reloaded.",
							strName, dblCounted, pProduct->dblCalculatedAmt,
							dblActual,
							dblDifference > 0 ? "remove" : "add", abs(dblDifference));

						if(IDNO == MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {

							CWaitCursor pWait;

							//reset this product's on hand amount, and clear its count
							pProduct->dblCalculatedAmt = dblActual;
							pProduct->varUserCount = g_cvarNull;

							//completely reload this product's items

							for(int j=pProduct->aryProductItems.GetSize()-1; j>=0; j--) {
								delete pProduct->aryProductItems.GetAt(j);
							}
							pProduct->aryProductItems.RemoveAll();

							_RecordsetPtr rsProductItems = CreateParamRecordset("SELECT ProductItemsT.ID, "			
								"ProductItemsT.SerialNum, "
								"CASE WHEN SerialNum Is Null THEN '<No Serial Num.>' ELSE SerialNum END + "
								//show the status text only if they have the adv. inv. license
								"	CASE WHEN {INT} = 0 THEN '' "
								"		WHEN ProductItemsT.Status = {INT} THEN '  (Purchased Inventory)' "
								"		WHEN ProductItemsT.Status = {INT} THEN '  (Consignment)' "
								"		WHEN ProductItemsT.Status = {INT} THEN '  (Warranty)' "
								"		ELSE '' END "
								"	AS SerializedText "	
								"FROM ProductItemsT "
								"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
								"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
								"INNER JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
								"WHERE ServiceT.Active = 1 AND TrackableStatus = 2 AND ProductLocationInfoT.LocationID = {INT} "
								"AND ProductT.HasSerialNum = 1 "
								"AND ProductItemsT.SerialNum Is Not Null "
								"AND ProductItemsT.Deleted = 0 "
								"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
								"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
								"	WHERE Status = {INT} AND ProductItemID Is Not Null) "
								"AND (ProductItemsT.LocationID = {INT} OR ProductItemsT.LocationID Is Null) "
								"AND ProductT.ID = {INT}",
								//show the status text only if they have the adv. inv. license
								g_pLicense->HasCandAModule(CLicense::cflrSilent) ? 1 : 0, InvUtils::pisPurchased, InvUtils::pisConsignment, InvUtils::pisWarranty,
								m_nLocationID, InvUtils::iadsUsed, m_nLocationID,
								nProductID);

							while(!rsProductItems->eof) {

								ReconciledProductItem *pNew = new ReconciledProductItem;

								pNew->nProductItemID = AdoFldLong(rsProductItems, "ID");
								pNew->nInternalID = -1;
								pNew->strSerialNum = AdoFldString(rsProductItems, "SerialNum", "");
								pNew->strSerializedText = AdoFldString(rsProductItems, "SerializedText", "");
								pNew->bCounted = FALSE;
								pNew->strNotes = "";

								pProduct->aryProductItems.Add(pNew);

								rsProductItems->MoveNext();
							}
							rsProductItems->Close();

							RefilterList();

							//select this product (if it's in our filters)
							IRowSettingsPtr pRow = m_ProductList->SetSelByColumn(plcProductID, nProductID);
							if(pRow) {
								m_ProductList->EnsureRowInView(pRow);
							}

							//remind the user what they need to do
							CString strWarn;
							strWarn.Format("The product '%s' has been reloaded and its count cleared. You must re-count this product to reconcile it.", strName);
							AfxMessageBox(strWarn);
							return FALSE;
						}
					}
				}
			}

			rs->MoveNext();
		}
		rs->Close();

		//now show the user exactly which products will be adjusted
		CInvReconciliationAdjustDlg dlg(this);
		dlg.m_nLocationID = m_nLocationID;
		dlg.m_paryProductsNeedingAdjustments = &arypProductsNeedingAdjustments;
		if(dlg.DoModal() == IDCANCEL) {
			//they cancelled, so quit now
			return FALSE;
		}

		//all needed changes to data will be tracked in the strSqlBatch field
		//in each ProductNeedingAdjustments pointer

		//if they didn't cancel, they want to continue and make adjustments,
		//but they may have changed the Adjust status of some products,
		//so update the interface accordingly
		int i=0;
		for(int i=0; i<arypProductsNeedingAdjustments.GetSize(); i++) {

			ProductNeedingAdjustments *pPNA = (ProductNeedingAdjustments*)arypProductsNeedingAdjustments.GetAt(i);
			IRowSettingsPtr pRow = m_ProductList->FindByColumn(plcProductID, pPNA->nProductID, m_ProductList->GetFirstRow(), FALSE);
			if(pRow) {

				//this should always find the parent first
				ASSERT(pRow->GetParentRow() == NULL);

				//update the adjust status
				if(pPNA->bAdjust) {
					pRow->PutValue(plcAdjust, _variant_t(VARIANT_TRUE, VT_BOOL));
				}
				else {
					pRow->PutValue(plcAdjust, _variant_t(VARIANT_FALSE, VT_BOOL));
				}
			}

			//update the tracked pointer (remember the row may be filtered out)
			BOOL bFound = FALSE;
			for(int j=0; j<m_aryProducts.GetSize() && !bFound; j++) {
			
				ReconciledProduct *pProduct = m_aryProducts.GetAt(j);

				if(pProduct->nProductID == pPNA->nProductID) {
					bFound = TRUE;
					
					pProduct->bAdjust = pPNA->bAdjust;
				}
			}
		}

		return TRUE;

	} NxCatchAll("Error in CInvReconciliationDlg::PrepareAdjustments");

	return FALSE;
}

// (j.jones 2009-01-15 11:22) - PLID 32684 - clear the adjustment array
void CInvReconciliationDlg::ClearAdjustmentArray(CArray<ProductNeedingAdjustments*, ProductNeedingAdjustments*> &arypProductsNeedingAdjustments)
{
	try {

		int i=0;
		for(int i=arypProductsNeedingAdjustments.GetSize() - 1; i>=0; i--) {
			
			ProductNeedingAdjustments *pPNA = (ProductNeedingAdjustments*)arypProductsNeedingAdjustments.GetAt(i);
			delete pPNA;
		}
		arypProductsNeedingAdjustments.RemoveAll();

	}NxCatchAll("Error in CInvReconciliationDlg::ClearAdjustmentArray");
}

// (j.jones 2009-01-15 14:26) - PLID 32684 - added function to save adjustments
void CInvReconciliationDlg::CreateAdjustment(CString &strSqlBatch, long &nAuditTransactionID, long nProductID, CString strInvRecProductID, CArray<ProductNeedingAdjustments*, ProductNeedingAdjustments*> &arypProductsNeedingAdjustments)
{
	try {

		//see if the product ID is in our adjustment array, and is still flagged to need an adjustment
		for(int i=0; i<arypProductsNeedingAdjustments.GetSize(); i++) {

			ProductNeedingAdjustments *pPNA = (ProductNeedingAdjustments*)arypProductsNeedingAdjustments.GetAt(i);

			if(pPNA->nProductID == nProductID
				&& pPNA->bAdjust
				&& pPNA->dblCalculated != pPNA->dblUserCounted) {

				//found the product, and we do indeed have to adjust it

				double dblAdjQuantity = pPNA->dblUserCounted - pPNA->dblCalculated;

				//calculate the est. cost
				_RecordsetPtr rs = CreateParamRecordset("SELECT LastCostPerUU AS LastCost "
					"FROM ProductT WHERE ID = {INT}", nProductID);
				COleCurrency cyLastCost = COleCurrency(0,0);
				if(!rs->eof) {
					cyLastCost = AdoFldCurrency(rs, "LastCost",COleCurrency(0,0));
				}
				rs->Close();

				COleCurrency cyAdjCost = -CalculateAmtQuantity(cyLastCost,dblAdjQuantity);

				AddStatementToSqlBatch(strSqlBatch, "SET @nNewProductAdjustmentID = (SELECT COALESCE(MAX(ID),0) + 1 FROM ProductAdjustmentsT)");
				//TES 6/24/2008 - PLID 26142 - Save the category they've selected.
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ProductAdjustmentsT "
					"(ID, ProductID, Date, Login, Quantity, Amount, LocationID, Notes, ProductAdjustmentCategoryID) "
					"VALUES (@nNewProductAdjustmentID, %li, '%s', %li, %g, Convert(money,'%s'), %li, 'Created from Inventory Reconciliation', %li)",
					nProductID, FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), 
					GetCurrentUserID(), dblAdjQuantity, _Q(FormatCurrencyForSql(cyAdjCost)), m_nLocationID, InvUtils::g_nAdjByInvReconciliation);

				//link this to the product InvReconciliationProductsT
				AddStatementToSqlBatch(strSqlBatch, "UPDATE InvReconciliationProductsT SET InvAdjustmentID = @nNewProductAdjustmentID "
					"WHERE ID = %s", strInvRecProductID);

				//now handle the product items
				if(pPNA->aryProductItemIDsToRemove.GetSize() > 0) {
					CString strIDs;
					for(int i=0;i<pPNA->aryProductItemIDsToRemove.GetSize();i++) {
						if(!strIDs.IsEmpty()) {
							strIDs += ",";
						}
						strIDs += AsString((long)(pPNA->aryProductItemIDsToRemove.GetAt(i)));
					}

					//other adjustment code says that now that this item has been adjusted out of existence, it shouldn't be flagged
					// To Be Returned any more, so I am replicating that logic here
					AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductItemsT SET Deleted = 1, AdjustmentID = @nNewProductAdjustmentID, "
						"ToBeReturned = 0 WHERE ID IN (%s)", strIDs);
				}

				//append any special sql code from the ProductNeedingAdjustments object
				//(any product items that need to be created would be in here
				strSqlBatch += pPNA->strSqlBatch;

				//audit this
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				CString strNew;
				strNew.Format("%g adjusted through Inventory Reconciliation", dblAdjQuantity);
				AuditEvent(-1, pPNA->strProductName, nAuditTransactionID, aeiProductAdjustment, nProductID, "", strNew, aepMedium, aetChanged);
			}
		}

	}NxCatchAll("Error in CInvReconciliationDlg::CreateAdjustment");
}

void CInvReconciliationDlg::OnSelChosenInvrecSupplierCombo(LPDISPATCH lpRow)
{
	try {
		
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_SupplierCombo->SetSelByColumn(sccID, (long)-1);
		}

		RefilterList();

	}NxCatchAll("Error in CInvReconciliationDlg::OnSelChosenInvrecSupplierCombo");
}

void CInvReconciliationDlg::OnSelChosenInvrecCategoryCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_CategoryCombo->SetSelByColumn(cccID, (long)-1);
		}

		RefilterList();

	}NxCatchAll("Error in CInvReconciliationDlg::OnSelChosenInvrecCategoryCombo");
}

//RefilterList will apply the contents of m_aryProducts to the list, taking the supplier
//and category filter into account
void CInvReconciliationDlg::RefilterList()
{
	try {

		CWaitCursor pWait;

		m_ProductList->Clear();

		long nSupplierID = -1;
		long nCategoryID = -1;
		
		IRowSettingsPtr pSupplierRow = m_SupplierCombo->GetCurSel();
		if(pSupplierRow) {
			nSupplierID = VarLong(pSupplierRow->GetValue(sccID), -1);
		}
		
		IRowSettingsPtr pCategoryRow = m_CategoryCombo->GetCurSel();
		if(pCategoryRow) {
			nCategoryID = VarLong(pCategoryRow->GetValue(cccID), -1);
		}

		//add our array into the list, per the supplier & category filters
		for(int i=0; i<m_aryProducts.GetSize(); i++) {

			ReconciledProduct *pProduct = m_aryProducts.GetAt(i);

			//if we have a category ID, disregard products not using it
			if(nCategoryID != -1 && pProduct->nCategoryID != nCategoryID) {
				continue;
			}

			//if we have a supplier ID, disregard products not using it
			if(nSupplierID != -1) {

				BOOL bHasSupplier = FALSE;

				//we only want to include the product if it can use this supplier
				for(int j=0; j<pProduct->arySupplierIDs.GetSize() && !bHasSupplier; j++) {
					if(pProduct->arySupplierIDs.GetAt(j) == nSupplierID) {
						bHasSupplier = TRUE;						
					}
				}

				if(!bHasSupplier) {
					continue;
				}
			}

			IRowSettingsPtr pProductRow = m_ProductList->GetNewRow();

			pProductRow->PutValue(plcID, pProduct->nProductID);
			pProductRow->PutValue(plcParentID, g_cvarNull);
			pProductRow->PutValue(plcProductID, pProduct->nProductID);
			pProductRow->PutValue(plcProductItemID, g_cvarNull);
			pProductRow->PutValue(plcInternalID, pProduct->nInternalID);
			pProductRow->PutValue(plcBarcode, _bstr_t(pProduct->strBarcode));
			pProductRow->PutValue(plcSerialNum, g_cvarNull);
			pProductRow->PutValue(plcCounted, g_cvarNull);
			pProductRow->PutValue(plcProductText, _bstr_t(pProduct->strName));
			pProductRow->PutValue(plcCalculatedAmt, pProduct->dblCalculatedAmt);
			pProductRow->PutValue(plcUserCount, pProduct->varUserCount);
			if(pProduct->varUserCount.vt == VT_R8) {
				pProductRow->PutValue(plcDifference, (pProduct->dblCalculatedAmt - VarDouble(pProduct->varUserCount)));
			}
			else {
				pProductRow->PutValue(plcDifference, g_cvarNull);
			}
			pProductRow->PutValue(plcAdjust, pProduct->bAdjust ? g_cvarTrue : g_cvarFalse);
			pProductRow->PutValue(plcNotes, _bstr_t(pProduct->strNotes));

			//if we have an internal ID, we cannot edit the row (except for notes)
			//so color it gray (do not color it gray if this is a closed reconciliation)
			if(pProduct->nInternalID != -1 && !m_bIsClosed) {
				pProductRow->PutForeColor(RGB(128,128,128));
			}

			m_ProductList->AddRowSorted(pProductRow, NULL);

			//and now load the product items, if we have any
			for(int j=0; j<pProduct->aryProductItems.GetSize(); j++) {

				ReconciledProductItem *pProductItem = pProduct->aryProductItems.GetAt(j);

				IRowSettingsPtr pProductItemRow = m_ProductList->GetNewRow();

				pProductItemRow->PutValue(plcID, g_cvarNull);
				pProductItemRow->PutValue(plcParentID, pProduct->nProductID);
				pProductItemRow->PutValue(plcProductID, pProduct->nProductID);
				pProductItemRow->PutValue(plcProductItemID, pProductItem->nProductItemID);
				pProductItemRow->PutValue(plcInternalID, pProductItem->nInternalID);
				pProductItemRow->PutValue(plcBarcode, g_cvarNull);
				pProductItemRow->PutValue(plcSerialNum, _bstr_t(pProductItem->strSerialNum));
				pProductItemRow->PutValue(plcCounted, pProductItem->bCounted ? g_cvarTrue : g_cvarFalse);
				pProductItemRow->PutValue(plcProductText, _bstr_t(pProductItem->strSerializedText));
				pProductItemRow->PutValue(plcCalculatedAmt, g_cvarNull);
				pProductItemRow->PutValue(plcUserCount, g_cvarNull);
				pProductItemRow->PutValue(plcDifference, g_cvarNull);
				pProductItemRow->PutValue(plcAdjust, g_cvarNull);
				pProductItemRow->PutValue(plcNotes, _bstr_t(pProductItem->strNotes));

				//if we have an internal ID, we cannot edit the row (except for notes)
				//so color it gray (do not color it gray if this is a closed reconciliation)
				if(pProductItem->nInternalID != -1 && !m_bIsClosed) {
					pProductItemRow->PutForeColor(RGB(128,128,128));
				}

				m_ProductList->AddRowSorted(pProductItemRow, pProductRow);
			}
		}

	}NxCatchAll("Error in CInvReconciliationDlg::RefilterList");
}

//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
// This function will likely be updated to pick the most suitable
// UPC code in response to a barcode scan. Practice now allows multiple
// products to have the same UPC codes. Further, products can share UPC codes
// with service codes (however, service codes cannot share UPC codes).

// Current behavior: Returns TRUE for first matching inventory code.

BOOL CInvReconciliationDlg::GetBestUPCProduct(ReconciledProduct *pProduct, CString strBarcode) 
{
	try {
		if(pProduct->strBarcode.CompareNoCase(strBarcode) == 0) {
			return TRUE;
		}
	} NxCatchAll(__FUNCTION__);

	return FALSE;
}
// InvConsignmentPurchaseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "inventoryrc.h"
#include "InvConsignmentPurchaseDlg.h"
#include "InternationalUtils.h"
#include "InvUtils.h"
#include "childfrm.h"
#include "InvView.h"
#include "barcode.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* (c.haag 2007-11-28 17:16) - PLID 28006 - Initial creation. The purpose
of this dialog is to allow users to purchase serialized items and transfer
them into regular inventory. */

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// Command ID's
#define ID_REMOVE_PURCHASE_ITEM		100

// (c.haag 2007-11-28 17:14) - PLID 28006 - Datalist column enumerations 
enum CategoryComboColumn {

	cccID = 0,
	cccName,
};

enum ProductComboColumn {

	pccID = 0,
	pccCategory,
	pccSupplier,
	pccProductName,
	pccPrice,
	pccBarcode,
};

enum SupplierComboColumn {

	sccID = 0,
	sccName,
};

enum LocationComboColumn {

	lccID = 0,
	lccName,
};

enum ConsignmentComboListColumn {
	cclcID = 0, // ProductItemT.ID
	cclcProductName,
	cclcSerialNum,
	cclcExpDate,
	cclcLocationName,
	cclcLastCost,
	cclcAllocated,
	cclcProductID, // (c.haag 2008-02-26 17:19) - PLID 28853 - Product ID
	cclcLocationID, // (c.haag 2008-02-26 17:31) - PLID 28853 - LocationID
}; 

enum PurchaseListColumn {
	plcID = 0, // ProductItemT.ID
	plcProductName,
	plcSerialNum,
	plcExpDate,
	plcLocationName,
	plcAmount,
	plcAllocated,
	plcProductID, // (c.haag 2008-02-26 17:19) - PLID 28853 - Product ID
	plcLocationID, // (c.haag 2008-02-26 17:31) - PLID 28853 - Location ID
};

/////////////////////////////////////////////////////////////////////////////
// CInvConsignmentPurchaseDlg dialog

CInvConsignmentPurchaseDlg::CInvConsignmentPurchaseDlg(CWnd* pParent)
	: CNxDialog(CInvConsignmentPurchaseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInvConsignmentPurchaseDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// (c.haag 2007-11-29 16:52) - PLID 28237 - Reset our scanning and initialized
	// flags
	m_bInitialized = FALSE;
	m_bIsScanning = FALSE;
}


void CInvConsignmentPurchaseDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvConsignmentPurchaseDlg)
	DDX_Control(pDX, IDC_STATIC_TOTAL, m_nxstaticTotal);
	DDX_Control(pDX, IDC_STATIC_TOTAL_AMOUNT, m_nxstaticTotalAmount);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}

CString CInvConsignmentPurchaseDlg::GetBaseQuery()
{
	// (c.haag 2007-11-29 16:26) - PLID 28237 - Returns the formatted string of the
	// query used for the inventory list filter as well as barcode searches.
	// I copied this from CInvConsignmentDlg::OnInitDialog and removed unnecessary fields.
	// (c.haag 2007-12-03 11:22) - PLID 28204 - Replacing consignment flag with status bit
	return "(SELECT ProductItemsT.ID, ProductItemsT.ProductID, ServiceT.Name AS ProductName, "
		"LocationsT.ID AS LocationID, Coalesce(LocationsT.Name, '<No Location>') AS LocName, "
		"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
		"ServiceT.Category, "
		"ProductT.LastCost, "
		// (c.haag 2007-12-10 15:19) - PLID 28006 - Added an allocation bit. Keep in mind that, technically,
		// if the status is iasCompleted/iadsUsed, it's allocated too; but we filter out used allocations.
		"CONVERT(BIT, CASE WHEN ProductItemsT.ID IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
		"			AND PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) +
		"			AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) +
		"			) "
		"THEN 1 ELSE 0 END) AS Allocated "
		"FROM ProductItemsT "
		"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
		"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
		"LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
		"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
		"LEFT JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineItemT ON ChargedProductItemsT.ChargeID = LineItemT.ID "
		"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
		"WHERE ServiceT.Active = 1 "
		"	AND ProductItemsT.Status = " + AsString((long)InvUtils::pisConsignment) +
		"	AND ProductItemsT.Deleted = 0 "
		"	AND LineItemT.ID Is Null "
		"	AND CaseHistoryDetailsT.ID Is Null "
		"	AND ProductItemsT.ID NOT IN ("
		"		SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
		"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
		"			AND PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasCompleted) +
		"			AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsUsed) +
		"	)"
		") AS ProductItemsQ";
}

long CInvConsignmentPurchaseDlg::GetLocationID()
{
	// (c.haag 2007-12-05 09:06) - PLID 28237 - Returns the ID of the currently
	// selected location
	if (NULL == m_LocationCombo) {
		// No combo? No selection.
		return -1;
	} else {
		return (NULL == m_LocationCombo->CurSel) ? -1 : VarLong(m_LocationCombo->CurSel->GetValue( lccID ), -1);
	}
}

CString CInvConsignmentPurchaseDlg::GetLocationName(long nLocationID)
{
	// (c.haag 2007-12-05 09:06) - PLID 28237 - Returns the name of a location given its ID
	if (NULL == m_LocationCombo) {
		// No combo? No selection.
		return "";
	}
	if (-1 == nLocationID) {
		// If the location ID is invalid, return an empty string
		return "";
	} else {
		// Search for the row containing the location ID
		IRowSettingsPtr pRow = m_LocationCombo->FindByColumn( lccID, nLocationID, NULL, VARIANT_FALSE );
		if (NULL != pRow) {
			// We found it
			return VarString(pRow->GetValue( lccName ), "");
		} else {
			// We didn't find it
			return "";
		}
	}
}

void CInvConsignmentPurchaseDlg::InitDatalists()
{
	// (c.haag 2007-11-28 16:11) - PLID 28006 - Bind all datalist members to
	// their corresponding form controls and add sentinel values where necessary
	m_PurchaseList = BindNxDataList2Ctrl(IDC_PURCHASE_LIST, false);
	m_ConsignmentCombo = BindNxDataList2Ctrl(IDC_CONSIGNMENT_COMBO, false);
	m_CategoryCombo = BindNxDataList2Ctrl(IDC_CONSIGN_CATEGORY_COMBO, true);
	m_ProductCombo = BindNxDataList2Ctrl(IDC_CONSIGN_PRODUCT_COMBO, true);
	m_SupplierCombo = BindNxDataList2Ctrl(IDC_CONSIGN_SUPPLIER_COMBO, true);
	m_LocationCombo = BindNxDataList2Ctrl(IDC_CONSIGN_LOCATION_COMBO, true);

	// (c.haag 2007-11-28 17:09) - Now update various combos with sentinel
	// wildcard values (copied from CInvConsignmentDlg::OnInitDialog)
	IRowSettingsPtr pRow;
	pRow = m_CategoryCombo->GetNewRow();
	pRow->PutValue(cccID, (long)-1);
	pRow->PutValue(cccName, _bstr_t(" <All Categories>"));
	m_CategoryCombo->AddRowSorted(pRow, NULL);
	m_CategoryCombo->SetSelByColumn(cccID, (long)-1);

	pRow = m_ProductCombo->GetNewRow();
	pRow->PutValue(pccID, (long)-1);
	pRow->PutValue(pccCategory, g_cvarNull);
	pRow->PutValue(pccSupplier, g_cvarNull);
	pRow->PutValue(pccProductName, _bstr_t(" <All Products>"));
	pRow->PutValue(pccPrice, g_cvarNull);
	pRow->PutValue(pccBarcode, g_cvarNull);
	m_ProductCombo->AddRowSorted(pRow, NULL);
	m_ProductCombo->SetSelByColumn(pccID, (long)-1);

	pRow = m_SupplierCombo->GetNewRow();
	pRow->PutValue(sccID, (long)-1);
	pRow->PutValue(sccName, _bstr_t(" <All Suppliers>"));
	m_SupplierCombo->AddRowSorted(pRow, NULL);
	m_SupplierCombo->SetSelByColumn(sccID, (long)-1);

	pRow = m_LocationCombo->GetNewRow();
	pRow->PutValue(lccID, (long)-1);
	pRow->PutValue(lccName, _bstr_t(" <All Locations>"));
	m_LocationCombo->AddRowSorted(pRow, NULL);
	m_LocationCombo->SetSelByColumn(lccID, (long)-1);

	// (c.haag 2007-11-28 17:21) - Set up the From clause for the consignment combo. 
	m_ConsignmentCombo->FromClause = _bstr_t(GetBaseQuery());

	// Now that the From clause is set up, go ahead and refilter/requery it
	ReFilterConsignmentCombo();
}

void CInvConsignmentPurchaseDlg::ReFilterConsignmentCombo()
{
	// (c.haag 2007-11-28 17:32) - PLID 28006 - Updates the filter on
	// the consignment dropdown and requeries it. This code was imported
	// from InvConsignmentDlg.cpp
	IRowSettingsPtr pRow;
	CString strWhere;

	// Filter on products
	pRow = m_ProductCombo->GetCurSel();
	if(pRow != NULL) {
		long nProductID = VarLong(pRow->GetValue(pccID), -1);
		if(nProductID != -1) {
			CString strProduct;
			strProduct.Format("ProductID = %li", nProductID);
			
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strProduct;
		}
	}

	// Filter on supplier
	pRow = m_SupplierCombo->GetCurSel();
	if(pRow != NULL) {
		long nSupplierID = VarLong(pRow->GetValue(sccID), -1);
		if(nSupplierID != -1) {
			CString strSupplier;

			//The design is intentional that the user could filter on a supplier that
			//doesn't match the same product they are filtering on - there just won't
			//be any results.

			strSupplier.Format("ProductID IN (SELECT ProductID FROM MultiSupplierT WHERE SupplierID = %li)", nSupplierID);
			
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strSupplier;
		}
	}

	// Filter on location
	pRow = m_LocationCombo->GetCurSel();
	if(pRow != NULL) {
		long nLocationID = VarLong(pRow->GetValue(lccID), -1);
		if(nLocationID != -1) {
			CString strLocation;

			//Some product items can, in theory, be location-less, and
			//this filter will always include those, because a location-less
			//Product Item is considered to be "all locations".

			strLocation.Format("(LocationID Is Null OR LocationID = %li)", nLocationID);				
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strLocation;
		}
	}

	// Filter on category
	pRow = m_CategoryCombo->GetCurSel();
	if(pRow != NULL) {
		long nCategoryID = VarLong(pRow->GetValue(cccID), -1);
		if(nCategoryID != -1) {
			CString strCategory;
			strCategory.Format("Category %s", InvUtils::Descendants(nCategoryID));
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strCategory;
		}
	}

	// Now update the filter filter and requery the list
	m_ConsignmentCombo->WhereClause = _bstr_t(strWhere);
	m_ConsignmentCombo->Requery();
}

void CInvConsignmentPurchaseDlg::CalculateTotal()
{
	// (c.haag 2007-11-29 08:57) - PLID 28006 - Calculates the total of all
	// purchased items and populates the total static form control with it
	COleCurrency cyTotal = COleCurrency(0,0);

	IRowSettingsPtr pRow = m_PurchaseList->GetFirstRow();
	while (NULL != pRow) {
		cyTotal += VarCurrency(pRow->Value[plcAmount]);
		pRow = pRow->GetNextRow();
	}

	GetDlgItem(IDC_STATIC_TOTAL_AMOUNT)->SetWindowText(FormatCurrencyForInterface(cyTotal));
}

BOOL CInvConsignmentPurchaseDlg::Validate()
{
	// (c.haag 2007-11-29 09:32) - PLID 28006 - Validate the data before saving.
	// First we make sure that the list isn't empty. Then we warn the user if any items
	// are allocated; that is, on hold for patients. Items that are "in use" never show
	// up as they were filtered out earlier.

	if (0 == m_PurchaseList->GetRowCount()) {
		MessageBox("You must have at least one item in the purchase list before you can save this transaction.", "Consignment Purchase", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// (c.haag 2007-12-10 12:17) - PLID 28006 - Now warn the user about any items that are allocated
	// and put on hold for patients
	IRowSettingsPtr pRow = m_PurchaseList->GetFirstRow();
	CArray<long,long> anAllocatedProductItemIDs;
	while (NULL != pRow) {
		if (VarBool(pRow->Value[plcAllocated])) {
			// If this is true, the item is allocated and on hold for a patient
			anAllocatedProductItemIDs.Add(VarLong(pRow->Value[plcID]));
		} else {
			// The item is not allocated and on hold for a patient
		}
		pRow = pRow->GetNextRow();
	}
	if (anAllocatedProductItemIDs.GetSize() > 0) {
		// If we get here, at least one item is allocated and on hold for a patient. Gather
		// pertinent information about the allocations and display them in a message box

		// (j.jones 2008-02-18 09:20) - PLID 28948 - excluded released items
		CString strMsg = "The following items are allocated for patients:\r\n\r\n";
		_RecordsetPtr prs = CreateRecordset(
			"SELECT TOP 11 ServiceT.Name, ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName "
			"FROM ProductItemsT "
			"LEFT JOIN ServiceT ON ProductItemsT.ProductID = ServiceT.ID "
			"LEFT JOIN PatientInvAllocationDetailsT ON ProductItemsT.ID = PatientInvAllocationDetailsT.ProductItemID "
			"LEFT JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"LEFT JOIN PersonT ON PatientInvAllocationsT.PatientID = PersonT.ID "
			"WHERE ProductItemsT.ID IN (%s) "
			"AND PatientInvAllocationsT.Status <> %li "
			"AND PatientInvAllocationDetailsT.Status NOT IN (%li, %li) "
			, ArrayAsString(anAllocatedProductItemIDs), InvUtils::iasDeleted, InvUtils::iadsDeleted, InvUtils::iadsReleased);
		long nItems = 0;

		// Read in information for the first ten available records
		while (!prs->eof && nItems < 10) {
			CString strProduct = AdoFldString(prs, "Name", "");
			CString strSerial = AdoFldString(prs, "SerialNum", "");
			_variant_t vExpDate = prs->Fields->Item["ExpDate"]->Value;
			CString strPatientName = AdoFldString(prs, "PatientName", "");

			CString str = strProduct;
			if (!strSerial.IsEmpty()) { str += " - " + strSerial; }
			if (VT_DATE == vExpDate.vt) { str += " (" + FormatDateTimeForInterface(vExpDate, 0, dtoDate) + ") "; }
			str += ":   " + strPatientName;

			strMsg += str + "\r\n";

			prs->MoveNext();
			nItems++;
		}
		if (!prs->eof) {
			// If there are more than ten records, truncate the message
			strMsg += "<more>\r\n";
		}
		strMsg += "\r\nAre you sure you wish to continue?";

		if (IDNO == MessageBox(strMsg, "Consignment Purchase", MB_YESNO | MB_ICONWARNING)) {
			// The user changed their mind
			return FALSE;
		}
	}
	else {
		// None of the items are allocated and on hold for patients. Go forward with the regular warning.
		CString strWarning = FormatString("Are you sure you wish to purchase %d item(s) from consignment?", m_PurchaseList->GetRowCount());
		if (IDNO == MessageBox(strWarning, "Consignment Purchase", MB_YESNO | MB_ICONQUESTION)) {
			// The user changed their mind
			return FALSE;
		}
	}

	// Validation is successful
	return TRUE;
}

void CInvConsignmentPurchaseDlg::Save()
{
	// (c.haag 2007-11-29 09:36) - PLID 28006 - This function will save the consignment
	// purchases, or throw an exception on failure.
	
	long nAuditTransactionID = -1; // (c.haag 2007-12-04 09:42) - PLID 28006 - Auditing
	try {
		CString strSaveString = BeginSqlBatch();
		nAuditTransactionID = BeginAuditTransaction();

		// Build the SQL string for each item individually
		IRowSettingsPtr pRow = m_PurchaseList->GetFirstRow();

		// (c.haag 2008-02-26 17:26) - PLID 28853 - In the loop, we also need to gather a map
		// of productid-locationid combinations for generating todo alarms with
		CMap<__int64,__int64,BOOL,BOOL> mapInvTodos;

		while (NULL != pRow) {
			const long nID = VarLong(pRow->Value[plcID]);

			// (c.haag 2007-12-04 08:58) - PLID 28264 - Update the history table (this
			// is part of extended auditing). Do this before the update because we need
			// the old ProductItem status before it's overwritten
			AddStatementToSqlBatch(strSaveString,
				"INSERT INTO ProductItemsStatusHistoryT (ProductItemID, OldStatus, NewStatus, UserID, Notes, Amount) "
				"SELECT ID, Status, %d, %d, '%s', CONVERT(MONEY, '%s') FROM ProductItemsT WHERE ID = %d ",
				InvUtils::pisPurchased, GetCurrentUserID(),
				_Q("Purchased from Consignment"), 
				FormatCurrencyForSql( VarCurrency(pRow->Value[plcAmount]) ),
				nID);

			// (c.haag 2007-12-03 11:22) - PLID 28204 - This used to be Consignment, but
			// now it's a general Status field
			AddStatementToSqlBatch(strSaveString,
				"UPDATE ProductItemsT SET Status = %d WHERE ID = %d",
				InvUtils::pisPurchased, nID);
			
			CString strName = VarString(pRow->Value[plcProductName]);
			CString strSerial = (VT_NULL == pRow->Value[plcSerialNum].vt) ? "" : VarString(pRow->Value[plcSerialNum]);
			CString strExpDate = (VT_NULL == pRow->Value[plcExpDate].vt) ? "" : FormatDateTimeForInterface( VarDateTime(pRow->Value[plcExpDate]), NULL, dtoDate, false );
			if (!strSerial.IsEmpty()) {
				strName += " - " + strSerial;
			}
			if (!strExpDate.IsEmpty()) {
				strName += " - " + strExpDate;
			}
			AuditEvent(-1, strName, nAuditTransactionID, aeiProductItemStatus, nID, "Consignment", "Purchased Inv.", aepMedium, aetChanged);

			// (c.haag 2008-02-26 17:27) - PLID 28853 - Update the map
			const __int64 nProductID = (__int64)VarLong(pRow->Value[plcProductID]);
			const __int64 nLocationID = (__int64)VarLong(pRow->Value[plcLocationID], -1);
			if (nLocationID > 0) {
				mapInvTodos.SetAt(nProductID + (nLocationID<<32), TRUE);
			}

			pRow = pRow->GetNextRow();
		}

		// Now write to data
		ExecuteSql("BEGIN TRAN \r\n" + strSaveString + "COMMIT TRAN \r\n");
		// Do the auditing
		CommitAuditTransaction(nAuditTransactionID);

		// (j.jones 2008-09-16 09:35) - PLID 31380 - EnsureInventoryTodoAlarms now supports being called
		// with multiple products at once, but with one location, so we need to track arrays of product IDs
		// per location ID, and call this function only once per needed location
		CArray<long, long> aryLocationIDs;
		CMap<long, long, CArray<long, long>*, CArray<long, long>*> mapProductArrayToLocationIDs;
		CArray<CArray<long, long>*, CArray<long, long>*> aryProductArrays;

		// (c.haag 2008-02-26 17:26) - PLID 28853 - Update the inventory todo alarms
		// for all the products in the map
		POSITION pos = mapInvTodos.GetStartPosition();
		__int64 key;
		BOOL bDummy;
		while (NULL != pos) {

			mapInvTodos.GetNextAssoc(pos, key, bDummy);
			const long nProductID = (long)(key & 0xFFFFFFFF);
			const long nLocationID = (long)(key >> 32);

			BOOL bFoundLocID = FALSE;
			int i=0;
			for(i=0; i<aryLocationIDs.GetSize() && !bFoundLocID; i++) {
				if((long)(aryLocationIDs.GetAt(i)) == nLocationID) {
					bFoundLocID = TRUE;

					//track in the product array
					CArray<long, long> *pAry = NULL;
					mapProductArrayToLocationIDs.Lookup(nLocationID, pAry);
					if(pAry) {
						pAry->Add((long)nProductID);
					}
					else {
						//the array should exist
						ASSERT(FALSE);
					}
				}
			}
			if(!bFoundLocID) {
				//track this LocationID
				aryLocationIDs.Add(nLocationID);

				//add a new array
				CArray<long, long> *pAry = new CArray<long, long>;
				pAry->Add(nProductID);
				aryProductArrays.Add(pAry);

				mapProductArrayToLocationIDs.SetAt(nLocationID, pAry);
			}
		}

		// (j.jones 2008-09-16 09:46) - PLID 31380 - now call EnsureInventoryTodoAlarms
		// for each location, for all products needed for that location
		int i=0;
		for(i=0; i<aryLocationIDs.GetSize(); i++) {
			long nLocationID = (long)(aryLocationIDs.GetAt(i));

			CArray<long, long> *pAry = NULL;
			mapProductArrayToLocationIDs.Lookup(nLocationID, pAry);
			if(pAry) {
				//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
				InvUtils::EnsureInventoryTodoAlarms(*pAry, nLocationID, false);
			}
			else {
				//the array should exist
				ASSERT(FALSE);
			}
		}

		//now clear our arrays
		mapProductArrayToLocationIDs.RemoveAll();
		for(i=aryProductArrays.GetSize() - 1; i>=0; i--) {			
			CArray<long, long> *pAry = (CArray<long, long>*)aryProductArrays.GetAt(i);
			pAry->RemoveAll();
			delete pAry;
		}
		aryProductArrays.RemoveAll();
		aryLocationIDs.RemoveAll();

	} NxCatchAllSilentCallThrow(
			if (-1 != nAuditTransactionID) { RollbackAuditTransaction(nAuditTransactionID); }
			);
}

BEGIN_MESSAGE_MAP(CInvConsignmentPurchaseDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvConsignmentPurchaseDlg)
	ON_COMMAND(ID_REMOVE_PURCHASE_ITEM, OnRemoveReturnItem)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvConsignmentPurchaseDlg message handlers

BOOL CInvConsignmentPurchaseDlg::OnInitDialog() 
{
	// (c.haag 2007-11-28 16:16) - PLID 28006 - Dialog initialization
	try {
		// (c.haag 2007-11-29 16:55) - PLID 28237 - Setup so that we are receiving barcode scan messages
		if(!GetMainFrame()->RegisterForBarcodeScan(this)) {
			MessageBox("Failed to register for barcode messages. You may be unable to use a barcode scanner at this time.", "Consignment Purchase", MB_OK);
		}

		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 10:52) - PLID 29820 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Bind and initialize all datalists
		InitDatalists();

		// Set fonts
		extern CPracticeApp theApp;
		GetDlgItem(IDC_STATIC_TOTAL)->SetFont(&theApp.m_boldFont);
		GetDlgItem(IDC_STATIC_TOTAL_AMOUNT)->SetFont(&theApp.m_boldFont);
	
		// (c.haag 2007-11-29 16:55) - PLID 28237 - We're offically initialized
		m_bInitialized = TRUE;

	} NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInvConsignmentPurchaseDlg::OnDestroy() 
{
	try {
		CNxDialog::OnDestroy();
	
		// (c.haag 2007-11-29 16:56) - PLID 28237 - No longer receive barcode scan messages
		GetMainFrame()->UnregisterForBarcodeScan(this);
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnDestroy");
}

void CInvConsignmentPurchaseDlg::OnOK() 
{
	// (c.haag 2007-11-28 17:42) - PLID 28006 - Save changes and dismiss
	// the dialog
	try {

		// First, validate our changes. If validation does not pass, then
		// leave the dialog open.
		if (!Validate()) {
			return;
		}

		// Now save the purchase. This will throw an exception on failure.
		Save();

		// (c.haag 2008-02-27 10:02) - PLID 28852 - Now update the inventory view because
		// it displays consignment totals
		CChildFrame *p = GetMainFrame()->GetActiveViewFrame();
		if (p) {
			CNxTabView *pView = (CNxTabView *)p->GetActiveView();
			CChildFrame *pFrame = GetMainFrame()->GetActiveViewFrame();
			if (pView && pFrame->IsOfType(INVENTORY_MODULE_NAME)) {
				((CInvView*)pView)->UpdateView();
			}
		}

		// Dismiss the dialog
		CDialog::OnOK();
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnOK");
}

void CInvConsignmentPurchaseDlg::OnCancel() 
{
	// (c.haag 2007-11-28 17:42) - PLID 28006 - Dismiss all desired changes
	try {
		CDialog::OnCancel();
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnCancel");
}

BEGIN_EVENTSINK_MAP(CInvConsignmentPurchaseDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvConsignmentPurchaseDlg)
	ON_EVENT(CInvConsignmentPurchaseDlg, IDC_CONSIGN_CATEGORY_COMBO, 16 /* SelChosen */, OnSelChosenConsignCategoryCombo, VTS_DISPATCH)
	ON_EVENT(CInvConsignmentPurchaseDlg, IDC_CONSIGN_LOCATION_COMBO, 16 /* SelChosen */, OnSelChosenConsignLocationCombo, VTS_DISPATCH)
	ON_EVENT(CInvConsignmentPurchaseDlg, IDC_CONSIGN_PRODUCT_COMBO, 16 /* SelChosen */, OnSelChosenConsignProductCombo, VTS_DISPATCH)
	ON_EVENT(CInvConsignmentPurchaseDlg, IDC_CONSIGN_SUPPLIER_COMBO, 16 /* SelChosen */, OnSelChosenConsignSupplierCombo, VTS_DISPATCH)
	ON_EVENT(CInvConsignmentPurchaseDlg, IDC_CONSIGNMENT_COMBO, 16 /* SelChosen */, OnSelChosenConsignmentCombo, VTS_DISPATCH)
	ON_EVENT(CInvConsignmentPurchaseDlg, IDC_PURCHASE_LIST, 9 /* EditingFinishing */, OnEditingFinishingPurchaseList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CInvConsignmentPurchaseDlg, IDC_PURCHASE_LIST, 6 /* RButtonDown */, OnRButtonDownPurchaseList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInvConsignmentPurchaseDlg, IDC_PURCHASE_LIST, 10 /* EditingFinished */, OnEditingFinishedPurchaseList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInvConsignmentPurchaseDlg::OnSelChosenConsignCategoryCombo(LPDISPATCH lpRow) 
{
	// (c.haag 2007-11-28 17:43) - PLID 28006 - Handle when the user makes a selection
	// in the category combo
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL == pRow) {
			// Select the sentinel value and refilter the combo
			m_CategoryCombo->SetSelByColumn(cccID, (long)-1);
		}
		ReFilterConsignmentCombo();
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnSelChosenConsignCategoryCombo");
}

void CInvConsignmentPurchaseDlg::OnSelChosenConsignLocationCombo(LPDISPATCH lpRow) 
{
	// (c.haag 2007-11-28 17:44) - PLID 28006 - Handle when the user makes a selection
	// in the location combo
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL == pRow) {
			// Select the sentinel value and refilter the combo
			m_LocationCombo->SetSelByColumn(lccID, (long)-1);
		}
		ReFilterConsignmentCombo();
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnSelChosenConsignLocationCombo");
}

void CInvConsignmentPurchaseDlg::OnSelChosenConsignProductCombo(LPDISPATCH lpRow) 
{
	// (c.haag 2007-11-28 17:45) - PLID 28006 - Handle when the user makes a selection
	// in the product combo
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL == pRow) {
			// Select the sentinel value and refilter the combo
			m_ProductCombo->SetSelByColumn(pccID, (long)-1);
		}
		ReFilterConsignmentCombo();
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnSelChosenConsignProductCombo");
}

void CInvConsignmentPurchaseDlg::OnSelChosenConsignSupplierCombo(LPDISPATCH lpRow) 
{
	// (c.haag 2007-11-28 17:45) - PLID 28006 - Handle when the user makes a selection
	// in the supplier combo
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL == pRow) {
			// Select the sentinel value and refilter the combo
			m_SupplierCombo->SetSelByColumn(sccID, (long)-1);
		}
		ReFilterConsignmentCombo();
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnSelChosenConsignSupplierCombo");
}

void CInvConsignmentPurchaseDlg::OnSelChosenConsignmentCombo(LPDISPATCH lpRow) 
{
	// (c.haag 2007-11-28 17:46) - PLID 28006 - Add the selected consignment item to
	// the purchase list
	try {
		IRowSettingsPtr pComboRow(lpRow);
		if (NULL != pComboRow) {

			// Make sure it's not already in the list
			if (NULL == m_PurchaseList->FindByColumn(plcID, VarLong(pComboRow->Value[cclcID]), NULL, VARIANT_TRUE)) {

				// Add it to the list
				IRowSettingsPtr pNewRow = m_PurchaseList->GetNewRow();
				pNewRow->Value[plcID] = pComboRow->Value[cclcID];
				pNewRow->Value[plcProductName] = pComboRow->Value[cclcProductName];
				pNewRow->Value[plcSerialNum] = pComboRow->Value[cclcSerialNum];
				pNewRow->Value[plcExpDate] = pComboRow->Value[cclcExpDate];
				pNewRow->Value[plcLocationName] = pComboRow->Value[cclcLocationName];
				pNewRow->Value[plcAmount] = pComboRow->Value[cclcLastCost];
				pNewRow->Value[plcAllocated] = pComboRow->Value[cclcAllocated];
				// (c.haag 2008-02-26 17:20) - PLID 28853 - Include the product and location ID
				pNewRow->Value[plcProductID] = pComboRow->Value[cclcProductID];
				pNewRow->Value[plcLocationID] = pComboRow->Value[cclcLocationID];
				m_PurchaseList->AddRowSorted(pNewRow, NULL);

				// Now recalculate the total amount
				CalculateTotal();

			} else {
				// It was already in the list. Warn the user about it.
				MessageBox("This item already exists in the purchase list.", "Consignment Purchase", MB_OK | MB_ICONWARNING);
			}
		}
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnSelChosenConsignmentCombo");
}

void CInvConsignmentPurchaseDlg::OnEditingFinishingPurchaseList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	// (c.haag 2007-11-29 09:03) - PLID 28006 - This event is fired when the user has finished
	// editing a row in the purchase list
	try {
		if (*pbCommit == FALSE) {
			return; // User hit escape
		} else if (NULL == lpRow) {
			return; // Should never happen
		}

		// Amount validation
		else if (plcAmount == nCol) {
			if (VarCurrency(*pvarNewValue, COleCurrency(0,0)) < COleCurrency(0,0)) {
				// We don't allow negative received amounts
				*pbCommit = FALSE;
			}
		}
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnEditingFinishingPurchaseList");
}

void CInvConsignmentPurchaseDlg::OnRButtonDownPurchaseList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	// (c.haag 2007-11-29 09:05) - PLID 28006 - This event is fired when a user right-clicks on the
	// purchase list.
	try {
		if (NULL == lpRow) {
			return; // Return if an actual item was not clicked on
		}

		// Ensure the row is selected
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		m_PurchaseList->CurSel = pRow;

		// Invoke the right click pop-up menu
		CMenu menu;
		CPoint pt;
		GetCursorPos(&pt);
		menu.m_hMenu = CreatePopupMenu();
		menu.InsertMenu(-1, MF_BYPOSITION, ID_REMOVE_PURCHASE_ITEM, "&Remove item");
		menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		menu.DestroyMenu();
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnRButtonDownPurchaseList");
}

void CInvConsignmentPurchaseDlg::OnEditingFinishedPurchaseList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	// (c.haag 2007-11-29 09:23) - PLID 28006 - This event is fired	when the editing of an item is
	// finished
	try {
		if (NULL == lpRow) {
			return; // Should never happen
		}

		// If the amount column was edited, update the total amount at the bottom of the dialog
		if (plcAmount == nCol) {
			CalculateTotal();
		}
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnEditingFinishedPurchaseList");
}

void CInvConsignmentPurchaseDlg::OnRemoveReturnItem()
{
	// (c.haag 2007-11-29 09:08) - PLID 28006 - This command is sent when the user wants
	// to remove an item from the purchase list
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_PurchaseList->CurSel;
		if (NULL == pRow) {
			return; // This should never happen
		}

		// Since this dialog is write-only, we don't need to worry about removing existing
		// data from the database. Just remove the row.
		m_PurchaseList->RemoveRow(pRow);

		// Now recalculate the total
		CalculateTotal();
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnRemoveReturnItem");
}

// (c.haag 2007-11-29 16:56) - PLID 28237 - Prevent multiple scans at one time with this fake mutex
class CPurchaseScanInProgress
{
private:
	CInvConsignmentPurchaseDlg& m_dlg;

public:
	CPurchaseScanInProgress(CInvConsignmentPurchaseDlg& dlg);
	~CPurchaseScanInProgress();
};

CPurchaseScanInProgress::CPurchaseScanInProgress(CInvConsignmentPurchaseDlg& dlg) : m_dlg(dlg)
{
	// Flag the fact that we are scanning
	m_dlg.m_bIsScanning = TRUE;
}

CPurchaseScanInProgress::~CPurchaseScanInProgress()
{
	// Flag the fact that we are done scanning
	m_dlg.m_bIsScanning = FALSE;
}

LRESULT CInvConsignmentPurchaseDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	// (c.haag 2007-11-29 16:56) - PLID 28237 - Handle barcode scans

	// Quit if we're still initializing
	if (!m_bInitialized) {
		return 0;
	}

	// Quit if we are scanning
	if (m_bIsScanning) {
		return 0;
	}

	CPurchaseScanInProgress sip(*this);
	try {
		_bstr_t bstr = (BSTR)lParam;
		_variant_t vBarcode(bstr);
		CString strBarcode(VarString(vBarcode));
		long nProductItemID = -1;

		// First, try to get the value from the consignment item dropdown
		IRowSettingsPtr pRow = m_ConsignmentCombo->FindByColumn(cclcSerialNum, vBarcode, NULL, VARIANT_FALSE);
		CArray<long,long> arExistingProductItemIDs;
		if (NULL != pRow) {
			// We found it! No need to query data. Do a check to see whether it's already in the item list.
			// If it's not, then add it. If it is, then do not.
			//TES 7/3/2008 - PLID 24726 - Since serial numbers are no longer necessarily unique, keep looping until we
			// find one that we haven't added yet.
			BOOL bAdded = FALSE;
			while(!bAdded && NULL != pRow) {
				if (NULL == m_PurchaseList->FindByColumn(plcID, VarLong(pRow->Value[ cclcID ]), NULL, VARIANT_FALSE)) {
					// If the item doesn't already exist in the selected list, add it
					bAdded = TRUE;
					IRowSettingsPtr pNewRow;
					pNewRow = m_PurchaseList->GetNewRow();
					pNewRow->PutValue(plcID, pRow->Value[ cclcID ]);
					pNewRow->PutValue(plcProductName, pRow->Value[ cclcProductName ]);
					pNewRow->PutValue(plcSerialNum, vBarcode);
					pNewRow->PutValue(plcExpDate, pRow->Value[ cclcExpDate ]);
					pNewRow->PutValue(plcLocationName, pRow->Value[ cclcLocationName ]);
					pNewRow->PutValue(plcAmount, pRow->Value[ cclcLastCost ]);
					pNewRow->PutValue(plcAllocated, pRow->Value[ cclcAllocated ]);
					// (c.haag 2008-02-26 17:20) - PLID 28853 - Include the product and location ID
					pNewRow->PutValue(plcProductID, pRow->Value[ cclcProductID ]);
					pNewRow->PutValue(plcLocationID, pRow->Value[ cclcLocationID ]);
					m_PurchaseList->AddRowSorted(pNewRow, NULL);
				} else {
					// It already exists in the selected list
					//TES 7/7/2008 - PLID 24726 - Remember this ID, we'll need it below.
					arExistingProductItemIDs.Add(VarLong(pRow->GetValue(plcID)));
					pRow = pRow->GetNextRow();

					// (j.jones 2010-10-21 10:09) - PLID 40564 - do not use FindByColumn again, because
					// it can restart at the beginning of the list, and potentially cause an infinite loop
					// use another while loop instead
					BOOL bFound = FALSE;
					while(pRow && !bFound) {
						CString strSerialNum = VarString(pRow->GetValue(plcSerialNum), "");
						if(strSerialNum.CompareNoCase(VarString(vBarcode, "")) == 0) {
							//found the next row
							bFound = TRUE;
						}
						else {
							//doesn't match, move along
							pRow = pRow->GetNextRow();
						}
					}
				}
			}
			return 0;

		} else {
			// It wasn't in the consignment combo
		}

		// If we haven't found it by now, search through data
		InvUtils::Barcode_CheckExistenceOfSerialNumberResultEx info;
		CString strProductName;
		_variant_t vExpDate;
		long nProductID = -1;

		// Use the utility function to find a qualifying product or product item ID
		// (c.haag 2008-06-18 12:10) - PLID 30427 - We now tell it to only find consignment items
		//TES 7/7/2008 - PLID 24726 - Pass in the product items that we've already purchased to 
		// Barcode_CheckExistenceofSerialNumber(), so it knows not to return any of them.
		if (!InvUtils::Barcode_CheckExistenceOfSerialNumber(strBarcode, -1, FALSE, nProductItemID, nProductID, strProductName, vExpDate, &info, TRUE /* Allow active allocations */, FALSE, FALSE /* Don't allow general items */, TRUE, &arExistingProductItemIDs) || -1 == nProductItemID)
		{
			// If we get here, we could not find any valid matches by product item. Try searching by product.
			_RecordsetPtr prs = CreateParamRecordset("SELECT ProductT.ID, ServiceT.Name FROM ProductT "
				"INNER JOIN ServiceT ON ServiceT.ID = ProductT.ID "
				"WHERE ServiceT.Barcode = {STRING} AND ServiceT.Active = 1 "
				"AND (ProductT.HasSerialNum <> 0 OR ProductT.HasExpDate <> 0) ", strBarcode);
			if (!prs->eof) {
				// We found a match. Bring up the product item picker.
				nProductID = AdoFldLong(prs, "ID");
				strProductName = AdoFldString(prs, "Name");
				prs->Close();

				CProductItemsDlg dlg(this);
				CDWordArray& anProductItemIDs = dlg.m_adwProductItemIDs;
				dlg.m_EntryType = PI_SELECT_DATA; // We're just selecting data
				dlg.m_CountOfItemsNeeded = 1;	// We need at least one item
				dlg.m_bUseSerial = info.bProductHasSerialNum;	// Using serial numbers
				dlg.m_bUseExpDate = info.bProductHasExpDate;	// Using expiration dates
				dlg.m_ProductID = nProductID; // Product ID
				dlg.m_nLocationID = -1; // Location ID
				//TES 6/18/2008 - PLID 29578 - Changed OrderID to OrderDetailID.
				dlg.m_nOrderDetailID = -1; // Not associated with an order
				dlg.m_bDisallowQtyChange = FALSE; // Allow the quantity to change
				dlg.m_bAllowQtyGrow = TRUE; // Allow the quantity to grow
				dlg.m_bIsAdjustment = FALSE; // We are not making an adjustment here
				dlg.m_strOverrideTitleBarText = "Select Item(s)"; // Custom title bar text
				dlg.m_strOverrideSelectQtyText = "Quantity to purchase"; // Custom quantity text
				dlg.m_strOverrideSelectItemText.Format("Please select one or more items for '%s'.",
					strProductName);
				dlg.m_strWhere = FormatString("ProductItemsT.Status = %d", InvUtils::pisConsignment);

				// (c.haag 2008-01-07 10:27) - Exclude all items already selected in this dialog's
				// purchase list
				IRowSettingsPtr pExistingRow = m_PurchaseList->GetFirstRow();
				CArray<long,long> anExistingIDs;
				while (NULL != pExistingRow) {
					anExistingIDs.Add( VarLong(pExistingRow->Value[plcID]) );
					pExistingRow = pExistingRow->GetNextRow();
				}
				if (anExistingIDs.GetSize() > 0) {
					dlg.m_strWhere += CString(" AND ProductItemsT.ID NOT IN (") + ArrayAsString(anExistingIDs) + ")";
				}

				dlg.m_bAllowActiveAllocations = true;
				
				if (IDCANCEL == dlg.DoModal() || 0 == anProductItemIDs.GetSize() /* Should never be zero, but safety first */) {
					// The user elected not to add any product items. We're done.
					return 0;
				} else {
					// The user chose one or more product items. Add them to the list. Note how we only
					// check the iasActive/iadsActive combination to determine whether an item is allocated.
					// An item can also be allocated if it's completed/used; but the product item dialog already
					// filters those out. We just need the active allocation flag to warn the user that they're
					// transferring something reserved for a patient.
					const long nProductItems = anProductItemIDs.GetSize();
					_RecordsetPtr prs = CreateRecordset(
						"SELECT ProductItemsT.ID, ProductItemsT.ProductID, ProductItemsT.SerialNum, ProductItemsT.LocationID, "
						"LocationsT.Name AS LocationName, ExpDate, LastCost, "
						"CONVERT(BIT, CASE WHEN ProductItemsT.ID IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
						"		INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
						"		WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
						"			AND PatientInvAllocationsT.Status = " + AsString((long)InvUtils::iasActive) +
						"			AND PatientInvAllocationDetailsT.Status = " + AsString((long)InvUtils::iadsActive) +
						"			) "
						"THEN 1 ELSE 0 END) AS Allocated "
						"FROM ProductItemsT "
						"LEFT JOIN LocationsT ON LocationsT.ID = ProductItemsT.LocationID "
						"LEFT JOIN ProductT ON ProductT.ID = ProductItemsT.ProductID "
						"WHERE ProductItemsT.ID IN (%s)", ArrayAsString(anProductItemIDs));
					FieldsPtr f = prs->Fields;
					while (!prs->eof) {
						if (NULL == m_PurchaseList->FindByColumn(plcID, AdoFldLong(f, "ID"), NULL, VARIANT_FALSE)) {
							pRow = m_PurchaseList->GetNewRow();
							pRow->PutValue(plcID, f->Item["ID"]->Value);
							pRow->PutValue(plcProductName, _bstr_t(strProductName));
							pRow->PutValue(plcSerialNum, f->Item["SerialNum"]->Value);
							pRow->PutValue(plcExpDate, f->Item["ExpDate"]->Value);
							pRow->PutValue(plcLocationName, f->Item["LocationName"]->Value);
							pRow->PutValue(plcAmount, f->Item["LastCost"]->Value);
							pRow->PutValue(plcAllocated, f->Item["Allocated"]->Value);
							// (c.haag 2008-02-26 17:20) - PLID 28853 - Include the product and location ID
							pRow->PutValue(plcProductID, f->Item["ProductID"]->Value);
							pRow->PutValue(plcLocationID, f->Item["LocationID"]->Value);
							m_PurchaseList->AddRowSorted(pRow, NULL);
						} else {
							// The item already exists in the list
						}
						prs->MoveNext();
					}
				}

			} // if (!prs->eof) {
			else {
				// We could not find any matches for either products or product items in data
				return 0;
			}

		}  // if (!InvUtils::Barcode_CheckExistenceOfSerialNumber(strBarcode, GetLocationID(), FALSE, nProductItemID, nProductID, strProductName, vExpDate, &info, TRUE /* Allow active allocations */) || -1 == nProductItemID)
		else {
			// We found a match, but we need to be sure the item is in consignment first
			if (InvUtils::pisConsignment == info.pisStatus) {
				// Now add it to the list if it's not there
				if (NULL == m_PurchaseList->FindByColumn(plcID, nProductItemID, NULL, VARIANT_FALSE)) {
					pRow = m_PurchaseList->GetNewRow();
					pRow->PutValue(plcID, nProductItemID);
					pRow->PutValue(plcProductName, _bstr_t(strProductName));
					pRow->PutValue(plcSerialNum, vBarcode);
					pRow->PutValue(plcExpDate, vExpDate);
					pRow->PutValue(plcLocationName, _bstr_t(GetLocationName(info.nProductLocationID)));
					pRow->PutValue(plcAmount, info.varProductLastCost);
					pRow->PutValue(plcAllocated, (InvUtils::stAllocated == info.stStatus) ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL) );
					// (c.haag 2008-02-27 09:44) - PLID 28853 - Include the product and location ID
					pRow->PutValue(plcProductID, nProductID);
					pRow->PutValue(plcLocationID, info.nProductLocationID);
					m_PurchaseList->AddRowSorted(pRow, NULL);
				} else {
					// This item already exists in the selected list
				}
			}
			else {
				// The item is not in consignment. Nice try!
			}
		}
	}
	NxCatchAll("Error in CInvConsignmentPurchaseDlg::OnBarcodeScan");
	return 0;
}

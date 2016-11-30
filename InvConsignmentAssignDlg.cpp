// InvConsignmentAssignDlg.cpp : implementation file
//

#include "stdafx.h"
#include "inventoryrc.h"
#include "InvConsignmentAssignDlg.h"
#include "InvUtils.h"
#include "childfrm.h"
#include "InvView.h"
#include "InternationalUtils.h"
#include "barcode.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* (c.haag 2007-11-29 10:22) - PLID 28050 - Initial creation. The purpose
of this dialog is to allow users to transfer serialized items from regular
inventory to consignment. */

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (c.haag 2007-11-29 10:23) - PLID 28050 - Datalist column enumerations 
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

enum InventoryListColumn {
	ilcID = 0, // ProductItemT.ID
	ilcProductName,
	ilcSerialNum,
	ilcExpDate,
	ilcLocationName,
	ilcAllocated,
	ilcProductID, // (c.haag 2008-02-26 17:10) - PLID 28853 - Product ID
	ilcLocationID, // (c.haag 2008-02-26 17:40) - PLID 28853 - Location ID
}; 

enum ConsignmentListColumn {
	clcID = 0, // ProductItemT.ID
	clcProductName,
	clcSerialNum,
	clcExpDate,
	clcLocationName,
	clcAllocated,
	clcProductID, // (c.haag 2008-02-26 17:10) - PLID 28853 - Product ID
	clcLocationID, // (c.haag 2008-02-26 17:40) - PLID 28853 - Location ID
}; 

/////////////////////////////////////////////////////////////////////////////
// CInvConsignmentAssignDlg dialog


CInvConsignmentAssignDlg::CInvConsignmentAssignDlg(CWnd* pParent)
	: CNxDialog(CInvConsignmentAssignDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInvConsignmentAssignDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// (c.haag 2007-11-29 16:11) - PLID 28236 - Reset our scanning and initialized
	// flags
	m_bInitialized = FALSE;
	m_bIsScanning = FALSE;
}


void CInvConsignmentAssignDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvConsignmentAssignDlg)
	DDX_Control(pDX, IDC_REMOVE_ALL, m_removeAllBtn);
	DDX_Control(pDX, IDC_REMOVE, m_removeBtn);
	DDX_Control(pDX, IDC_ADD_ALL, m_addAllBtn);
	DDX_Control(pDX, IDC_ADD, m_addBtn);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}

CString CInvConsignmentAssignDlg::GetBaseQuery()
{
	// (c.haag 2007-11-29 16:26) - PLID 28236 - Returns the formatted string of the
	// query used for the inventory list filter as well as barcode searches.
	// I copied this from CInvConsignmentDlg::OnInitDialog and removed unnecessary fields.
	// (c.haag 2007-12-03 12:31) - PLID 28204 - Replacing consignment flag with status bit
	return "(SELECT ProductItemsT.ID, ProductItemsT.ProductID, ServiceT.Name AS ProductName, "
		"LocationsT.ID AS LocationID, Coalesce(LocationsT.Name, '<No Location>') AS LocName, "
		"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
		"ServiceT.Category, "
		"ProductT.LastCost, "
		// (c.haag 2007-12-10 15:19) - PLID 28050 - Added an allocation bit. Keep in mind that, technically,
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
		"	AND ProductItemsT.Status <> " + AsString((long)InvUtils::pisConsignment) +
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

long CInvConsignmentAssignDlg::GetLocationID()
{
	// (c.haag 2007-12-05 08:18) - PLID 28236 - Returns the ID of the currently
	// selected location
	if (NULL == m_LocationCombo) {
		// No combo? No selection.
		return -1;
	} else {
		return (NULL == m_LocationCombo->CurSel) ? -1 : VarLong(m_LocationCombo->CurSel->GetValue( lccID ), -1);
	}
}

CString CInvConsignmentAssignDlg::GetLocationName(long nLocationID)
{
	// (c.haag 2007-12-05 08:18) - PLID 28236 - Returns the name of a location given its ID
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

void CInvConsignmentAssignDlg::InitDatalists()
{
	// (c.haag 2007-11-29 10:20) - PLID 28050 - Bind all datalist members to
	// their corresponding form controls and add sentinel values where necessary
	m_InventoryList = BindNxDataList2Ctrl(IDC_INVENTORY_LIST, false);
	m_ConsignmentList = BindNxDataList2Ctrl(IDC_CONSIGNMENT_LIST, false);
	m_CategoryCombo = BindNxDataList2Ctrl(IDC_CONSIGN_CATEGORY_COMBO, true);
	m_ProductCombo = BindNxDataList2Ctrl(IDC_CONSIGN_PRODUCT_COMBO, true);
	m_SupplierCombo = BindNxDataList2Ctrl(IDC_CONSIGN_SUPPLIER_COMBO, true);
	m_LocationCombo = BindNxDataList2Ctrl(IDC_CONSIGN_LOCATION_COMBO, true);

	// (c.haag 2007-11-29 10:20) - Now update various combos with sentinel
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


	// (c.haag 2007-11-29 10:24) - Set up the From clause for the inventory list. 
	m_InventoryList->FromClause = _bstr_t(GetBaseQuery());

	// Now that the From clause is set up, go ahead and refilter/requery it
	ReFilterInventoryList();
}

void CInvConsignmentAssignDlg::ReFilterInventoryList()
{
	// (c.haag 2007-11-29 10:24) - PLID 28050 - Updates the filter on
	// the inventory list and requeries it. This code was copied almost
	// verbatim from InvConsignmentPurchaseDlg.cpp
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
	m_InventoryList->WhereClause = _bstr_t(strWhere);
	m_InventoryList->Requery();
}

void CInvConsignmentAssignDlg::UpdateButtons()
{
	// (c.haag 2007-11-29 15:24) - PLID 28050 - Updates the enabled state of icon buttons
	if (0 == m_InventoryList->GetRowCount()) {
		GetDlgItem(IDC_ADD_ALL)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_ADD_ALL)->EnableWindow(TRUE);
	}

	if (0 == m_ConsignmentList->GetRowCount()) {
		GetDlgItem(IDC_REMOVE_ALL)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_REMOVE_ALL)->EnableWindow(TRUE);
	}

	if (NULL == m_InventoryList->CurSel) {
		GetDlgItem(IDC_ADD)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_ADD)->EnableWindow(TRUE);
	}

	if (NULL == m_ConsignmentList->CurSel) {
		GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
	}
}

BOOL CInvConsignmentAssignDlg::Validate()
{
	// (c.haag 2007-11-29 12:02) - PLID 28050 - Ensure that it's OK to save the user's
	// selections. Because none of the columns are editable, and the dialog is write-only
	// as far as data is concerned, The only way this dialog can fail validation is if
	// nothing is selected.
	if (0 == m_ConsignmentList->GetRowCount()) {
		MessageBox("You must have at least one item in the consignment list before you can save this transfer.", "Transfer to Consignment", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	// (c.haag 2007-12-10 12:17) - PLID 28006 - Now warn the user about any items that are allocated
	// and put on hold for patients
	IRowSettingsPtr pRow = m_ConsignmentList->GetFirstRow();
	CArray<long,long> anAllocatedProductItemIDs;
	while (NULL != pRow) {
		if (VarBool(pRow->Value[clcAllocated])) {
			// If this is true, the item is allocated and on hold for a patient
			anAllocatedProductItemIDs.Add(VarLong(pRow->Value[clcID]));
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

		if (IDNO == MessageBox(strMsg, "Transfer to Consignment", MB_YESNO | MB_ICONWARNING)) {
			// The user changed their mind
			return FALSE;
		}
	}
	else {
		// None of the items are allocated and on hold for patients. Go forward with the regular warning.
		CString strWarning = FormatString("Are you sure you wish to transfer %d item(s) to consignment?", m_ConsignmentList->GetRowCount());
		if (IDNO == MessageBox(strWarning, "Transfer to Consignment", MB_YESNO | MB_ICONQUESTION)) {
			// The user changed their mind
			return FALSE;
		}
	}

	return TRUE;
}

void CInvConsignmentAssignDlg::Save()
{
	long nAuditTransactionID = -1; // (c.haag 2007-12-04 09:42) - PLID 28050 - Auditing
	try {
		CString strSaveString = BeginSqlBatch();
		nAuditTransactionID = BeginAuditTransaction();

		// (c.haag 2007-11-29 12:04) - PLID 28050 - This function will save the consigment
		// transfer, or throw an exception on failure
		// (c.haag 2007-12-03 12:34) - PLID 28204 - Replacing consignment bit with status flag

		// (c.haag 2007-12-04 09:58) - PLID 28050 - For auditing purposes, we need to get the
		// old status for every item
		IRowSettingsPtr pRow = m_ConsignmentList->GetFirstRow();
		CMap<long,long,long,long> mapIDToOldStatus;
		CString strFilter;
		while (NULL != pRow) {
			strFilter += FormatString("%d,", VarLong(pRow->Value[clcID]));
			pRow = pRow->GetNextRow();
		}
		strFilter.TrimRight(",");
		_RecordsetPtr prs = CreateRecordset("SELECT ID, Status FROM ProductItemsT WHERE ID IN (%s)",
			strFilter);
		FieldsPtr f = prs->Fields;
		while (!prs->eof) {
			mapIDToOldStatus.SetAt( AdoFldLong(f, "ID"), AdoFldLong(f, "Status") );
			prs->MoveNext();
		}
		prs->Close();

		// Build the SQL string for each item individually

		// (c.haag 2008-02-26 17:26) - PLID 28853 - In the loop, we also need to gather a map
		// of productid-locationid combinations for generating todo alarms with
		CMap<__int64,__int64,BOOL,BOOL> mapInvTodos;

		pRow = m_ConsignmentList->GetFirstRow();
		while (NULL != pRow) {
			const long nID = VarLong(pRow->Value[clcID]);

			// (c.haag 2007-12-04 08:58) - PLID 28264 - Update the history table (this
			// is part of extended auditing). Do this before the update because we need
			// the old ProductItem status before it's overwritten
			// (j.jones 2008-02-18 12:03) - PLID 28987 - changed the note from "Assigned To Consignment" to "Moved To Consignment"
			AddStatementToSqlBatch(strSaveString,
				"INSERT INTO ProductItemsStatusHistoryT (ProductItemID, OldStatus, NewStatus, UserID, Notes) "
				"SELECT ID, Status, %d, %d, '%s' FROM ProductItemsT WHERE ID = %d ",
				InvUtils::pisConsignment, GetCurrentUserID(), _Q("Moved to Consignment"), VarLong(pRow->Value[clcID]));

			AddStatementToSqlBatch(strSaveString,
				"UPDATE ProductItemsT SET Status = %d WHERE ID = %d",
				InvUtils::pisConsignment, nID);

			CString strOld;
			long nOldStatus;
			if (mapIDToOldStatus.Lookup(nID, nOldStatus)) {
				switch ((InvUtils::ProductItemStatus)nOldStatus) {
				case InvUtils::pisPurchased: strOld = "Purchased Inv."; break;
				case InvUtils::pisConsignment: strOld = "Consignment"; break; // This should never happen
				case InvUtils::pisWarranty: strOld = "Warranty"; break;
				default: strOld = ""; break; // This should never happen
				}
			} else {
				// The product item record must have been deleted before this function ran
			}

			CString strName = VarString(pRow->Value[clcProductName]);
			CString strSerial = (VT_NULL == pRow->Value[clcSerialNum].vt) ? "" : VarString(pRow->Value[clcSerialNum]);
			CString strExpDate = (VT_NULL == pRow->Value[clcExpDate].vt) ? "" : FormatDateTimeForInterface( VarDateTime(pRow->Value[clcExpDate]), NULL, dtoDate, false );
			if (!strSerial.IsEmpty()) {
				strName += " - " + strSerial;
			}
			if (!strExpDate.IsEmpty()) {
				strName += " - " + strExpDate;
			}
			AuditEvent(-1, strName, nAuditTransactionID, aeiProductItemStatus, nID, strOld, "Consignment", aepMedium, aetChanged);

			// (c.haag 2008-02-26 17:27) - PLID 28853 - Update the map
			const __int64 nProductID = (__int64)VarLong(pRow->Value[clcProductID]);
			const __int64 nLocationID = (__int64)VarLong(pRow->Value[clcLocationID], -1);
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

BEGIN_MESSAGE_MAP(CInvConsignmentAssignDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvConsignmentAssignDlg)
	ON_BN_CLICKED(IDC_ADD_ALL, OnAddAll)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_REMOVE_ALL, OnRemoveAll)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvConsignmentAssignDlg message handlers

BOOL CInvConsignmentAssignDlg::OnInitDialog() 
{
	// (c.haag 2007-11-29 10:21) - PLID 28050 - Dialog initialization
	try {
		// (c.haag 2007-11-29 16:11) - PLID 28236 - Setup so that we are receiving barcode scan messages
		if(!GetMainFrame()->RegisterForBarcodeScan(this)) {
			MessageBox("Failed to register for barcode messages. You may be unable to use a barcode scanner at this time.", "Transfer to Consignment", MB_OK);
		}

		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 10:52) - PLID 29820 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
	
		// Bind and initialize all datalists
		InitDatalists();

		// Set up the transfer button icons
		m_addBtn.AutoSet(NXB_DOWN);
		m_addAllBtn.AutoSet(NXB_DDOWN);
		m_removeBtn.AutoSet(NXB_UP);
		m_removeAllBtn.AutoSet(NXB_UUP);

		// Now update the button enabled states
		UpdateButtons();

		// (c.haag 2007-11-29 16:11) - PLID 28236 - We're offically initialized
		m_bInitialized = TRUE;
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInvConsignmentAssignDlg::OnDestroy() 
{
	try {
		CNxDialog::OnDestroy();
	
		// (c.haag 2007-11-29 16:12) - PLID 28236 - No longer receive barcode scan messages
		GetMainFrame()->UnregisterForBarcodeScan(this);
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnDestroy");
}

void CInvConsignmentAssignDlg::OnOK() 
{
	// (c.haag 2007-11-29 10:32) - PLID 28050 - Save changes and dismiss
	// the dialog
	try {	
		// First, validate our changes. If validation does not pass, then
		// leave the dialog open.
		if (!Validate()) {
			return;
		}

		// Now save the transfer. This will throw an exception on failure.
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
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnOK");
}

void CInvConsignmentAssignDlg::OnCancel() 
{
	// (c.haag 2007-11-29 10:32) - PLID 28050 - Dismiss all desired changes
	try {

		CDialog::OnCancel();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnCancel");	
}

void CInvConsignmentAssignDlg::OnAddAll() 
{
	// (c.haag 2007-11-29 11:05) - PLID 28050 - Move all rows from the
	// regular inventory list to the consignment list
	try {
		IRowSettingsPtr pRow = m_InventoryList->GetFirstRow();
		while (NULL != pRow) {
			if (NULL == m_ConsignmentList->FindByColumn(clcID, VarLong(pRow->Value[ilcID]), NULL, VARIANT_FALSE)) {
				m_ConsignmentList->AddRowSorted(pRow, NULL);
			}
			pRow = pRow->GetNextRow();
		}
		// Now update the button appearances
		UpdateButtons();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnAddAll");
}

void CInvConsignmentAssignDlg::OnAdd() 
{
	// (c.haag 2007-11-29 11:07) - PLID 28050 - Add all selected regular inventory rows
	// to the consignment list
	try {
		// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
		IRowSettingsPtr pCurSelRow = m_InventoryList->GetFirstSelRow();
		while (pCurSelRow != NULL) {
			IRowSettingsPtr pRow = pCurSelRow;
			pCurSelRow = pCurSelRow->GetNextSelRow();
			if (NULL == m_ConsignmentList->FindByColumn(clcID, VarLong(pRow->Value[ilcID]), NULL, VARIANT_FALSE)) {
				m_ConsignmentList->AddRowSorted(pRow, NULL);
			}
			// (b.cardillo 2008-07-15 16:27) - PLID 30741 - This used to be a memory leak because we weren't releasing the IDispatch pointer, 
			// so the reference count would never drop to 0.  Now that we don't use the IDispatch pointers directly, there's no risk of that.
		}
		// Now update the button appearances
		UpdateButtons();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnAdd");
	
}

void CInvConsignmentAssignDlg::OnRemove() 
{
	// (c.haag 2007-11-29 11:10) - PLID 28050 - Remove all selected consignment rows
	try {
		// Gather the list of selected rows
		CArray<LPDISPATCH,LPDISPATCH> apdispRows;
		// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
		IRowSettingsPtr pCurSelRow = m_ConsignmentList->GetFirstSelRow();
		while (pCurSelRow) {
			LPDISPATCH lpDisp = ((IDispatchPtr)pCurSelRow).Detach();
			pCurSelRow = pCurSelRow->GetNextSelRow();
			if (lpDisp) {
				apdispRows.Add(lpDisp);
			}
		}

		// Now clear out the rows
		const int nRows = apdispRows.GetSize();
		for (int i=0; i < nRows; i++) {
			IRowSettingsPtr pRow(apdispRows[i]);
			m_ConsignmentList->RemoveRow(pRow);
		}
		
		// (b.cardillo 2008-07-18 10:29) - PLID 30778 - We weren't releasing our last reference to the 
		// row(s): the array elements.
		// Release the array elements and empty the array
		SafeEmptyArrayOfCOMPointers(apdispRows);

		// Now update the button appearances
		UpdateButtons();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnRemove");
}

void CInvConsignmentAssignDlg::OnRemoveAll() 
{
	// (c.haag 2007-11-29 11:05) - PLID 28050 - Clear the consignment list
	try {
		m_ConsignmentList->Clear();

		// Now update the button appearances
		UpdateButtons();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnRemoveAll");	
}

BEGIN_EVENTSINK_MAP(CInvConsignmentAssignDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvConsignmentAssignDlg)
	ON_EVENT(CInvConsignmentAssignDlg, IDC_INVENTORY_LIST, 3 /* DblClickCell */, OnDblClickCellInventoryList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CInvConsignmentAssignDlg, IDC_CONSIGNMENT_LIST, 3 /* DblClickCell */, OnDblClickCellConsignmentList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CInvConsignmentAssignDlg, IDC_INVENTORY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedInventoryList, VTS_I2)
	ON_EVENT(CInvConsignmentAssignDlg, IDC_CONSIGN_CATEGORY_COMBO, 16 /* SelChosen */, OnSelChosenConsignCategoryCombo, VTS_DISPATCH)
	ON_EVENT(CInvConsignmentAssignDlg, IDC_CONSIGN_LOCATION_COMBO, 16 /* SelChosen */, OnSelChosenConsignLocationCombo, VTS_DISPATCH)
	ON_EVENT(CInvConsignmentAssignDlg, IDC_CONSIGN_PRODUCT_COMBO, 16 /* SelChosen */, OnSelChosenConsignProductCombo, VTS_DISPATCH)
	ON_EVENT(CInvConsignmentAssignDlg, IDC_CONSIGN_SUPPLIER_COMBO, 16 /* SelChosen */, OnSelChosenConsignSupplierCombo, VTS_DISPATCH)
	ON_EVENT(CInvConsignmentAssignDlg, IDC_CONSIGNMENT_LIST, 28 /* CurSelWasSet */, OnCurSelWasSetConsignmentList, VTS_NONE)
	ON_EVENT(CInvConsignmentAssignDlg, IDC_INVENTORY_LIST, 28 /* CurSelWasSet */, OnCurSelWasSetInventoryList, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInvConsignmentAssignDlg::OnDblClickCellInventoryList(LPDISPATCH lpRow, short nColIndex) 
{
	// (c.haag 2007-11-29 11:38) - PLID 28050 - Add the selected inventory list item to the
	// consignment list
	try {
		// Ensure the row is selected
		IRowSettingsPtr pRow(lpRow);
		m_InventoryList->CurSel = pRow;

		// Now do the addition
		OnAdd();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnDblClickCellInventoryList");
}

void CInvConsignmentAssignDlg::OnDblClickCellConsignmentList(LPDISPATCH lpRow, short nColIndex) 
{
	// (c.haag 2007-11-29 11:39) - PLID 28050 - Removes the selected consignment list item
	try {
		// Ensure the row is selected
		IRowSettingsPtr pRow(lpRow);
		m_ConsignmentList->CurSel = pRow;

		// Now do the removal
		OnRemove();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnDblClickCellConsignmentList");
}

void CInvConsignmentAssignDlg::OnRequeryFinishedInventoryList(short nFlags) 
{
	// (c.haag 2007-11-29 11:53) - PLID 28050 - Called when the inventory list is finished
	// requerying. Here, all we do are update the buttons, even if the requery failed.
	try {
		UpdateButtons();
	} NxCatchAll("Error in CInvConsignmentAssignDlg::OnRequeryFinishedInventoryList");
}

void CInvConsignmentAssignDlg::OnSelChosenConsignCategoryCombo(LPDISPATCH lpRow) 
{
	// (c.haag 2007-11-29 11:58) - PLID 28050 - Called when the category combo selection
	// has changed
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL == pRow) {
			// Select the sentinel value and refilter the combo
			m_CategoryCombo->SetSelByColumn(cccID, (long)-1);
		}
		ReFilterInventoryList();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnSelChosenConsignCategoryCombo");
}

void CInvConsignmentAssignDlg::OnSelChosenConsignLocationCombo(LPDISPATCH lpRow) 
{
	// (c.haag 2007-11-29 11:59) - PLID 28050 - Called when the location combo selection
	// has changed
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL == pRow) {
			// Select the sentinel value and refilter the combo
			m_LocationCombo->SetSelByColumn(lccID, (long)-1);
		}
		ReFilterInventoryList();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnSelChosenConsignLocationCombo");	
}

void CInvConsignmentAssignDlg::OnSelChosenConsignProductCombo(LPDISPATCH lpRow) 
{
	// (c.haag 2007-11-29 12:00) - PLID 28050 - Called when the product combo selection
	// has changed
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL == pRow) {
			// Select the sentinel value and refilter the combo
			m_ProductCombo->SetSelByColumn(pccID, (long)-1);
		}
		ReFilterInventoryList();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnSelChosenConsignProductCombo");
}

void CInvConsignmentAssignDlg::OnSelChosenConsignSupplierCombo(LPDISPATCH lpRow) 
{
	// (c.haag 2007-11-29 12:01) - PLID 28050 - Called when the supplier combo selection
	// has changed
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL == pRow) {
			// Select the sentinel value and refilter the combo
			m_SupplierCombo->SetSelByColumn(sccID, (long)-1);
		}
		ReFilterInventoryList();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnSelChosenConsignSupplierCombo");
}

void CInvConsignmentAssignDlg::OnCurSelWasSetConsignmentList() 
{
	// (c.haag 2007-11-29 15:31) - PLID 28050 - Called when the user changes the
	// selection in the consignment list
	try {
		UpdateButtons();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnCurSelWasSetConsignmentList");	
}

void CInvConsignmentAssignDlg::OnCurSelWasSetInventoryList() 
{
	// (c.haag 2007-11-29 15:31) - PLID 28050 - Called when the user changes the
	// selection in the inventory list
	try {
		UpdateButtons();
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnCurSelWasSetInventoryList");
}

// (c.haag 2007-11-29 16:13) - PLID 28236 - Prevent multiple scans at one time with this fake mutex
class CAssignScanInProgress
{
private:
	CInvConsignmentAssignDlg& m_dlg;

public:
	CAssignScanInProgress(CInvConsignmentAssignDlg& dlg);
	~CAssignScanInProgress();
};

CAssignScanInProgress::CAssignScanInProgress(CInvConsignmentAssignDlg& dlg) : m_dlg(dlg)
{
	// Flag the fact that we are scanning
	m_dlg.m_bIsScanning = TRUE;
}

CAssignScanInProgress::~CAssignScanInProgress()
{
	// Flag the fact that we are done scanning
	m_dlg.m_bIsScanning = FALSE;
}


LRESULT CInvConsignmentAssignDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	// (c.haag 2007-11-29 16:13) - PLID 28236 - Handle barcode scans

	// Quit if we're still initializing
	if (!m_bInitialized) {
		return 0;
	}

	// Quit if we are scanning
	if (m_bIsScanning) {
		return 0;
	}

	CAssignScanInProgress sip(*this);
	try {
		_bstr_t bstr = (BSTR)lParam;
		_variant_t vBarcode(bstr);
		CString strBarcode(VarString(vBarcode));
		long nProductItemID = -1;

		// First, attempt to get the value from the inventory list
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_InventoryList->FindByColumn(ilcSerialNum, vBarcode, NULL, VARIANT_FALSE);
		if (NULL != pRow) {
			//TES 7/3/2008 - PLID 24726 - Since the same serial number may exist more than once, go through the top list
			// until we find one that hasn't been added to the bottom list.
			BOOL bAdded = FALSE;
			while(!bAdded && NULL != pRow) {
				// Wow, we found it! No need to query data. Check to see whether the item is already in the list. If
				// it's not, then add it. Otherwise, do not.
				if (NULL == m_ConsignmentList->FindByColumn(clcID, VarLong(pRow->Value[ ilcID ]), NULL, VARIANT_FALSE)) {
					bAdded = TRUE;
					IRowSettingsPtr pNewRow;
					pNewRow = m_ConsignmentList->GetNewRow();
					pNewRow->PutValue(clcID, VarLong(pRow->Value[ ilcID ]));
					pNewRow->PutValue(clcProductName, pRow->Value[ ilcProductName ]);
					pNewRow->PutValue(clcSerialNum, vBarcode);
					pNewRow->PutValue(clcExpDate, pRow->Value[ ilcExpDate ]);
					pNewRow->PutValue(clcLocationName, pRow->Value[ ilcLocationName ]);
					pNewRow->PutValue(clcAllocated, pRow->Value[ ilcAllocated ]);
					// (c.haag 2008-02-26 17:06) - PLID 28853 - Get the product and location ID
					pNewRow->PutValue(clcProductID, pRow->Value[ ilcProductID ]);
					pNewRow->PutValue(clcLocationID, pRow->Value[ ilcLocationID ]);
					m_ConsignmentList->AddRowSorted(pNewRow, NULL);
				} else {
					// It already exists in the assigned list
					//TES 7/3/2008 - PLID 24726 - Advance to the next row.
					pRow = pRow->GetNextRow();
					// (j.jones 2010-10-21 10:09) - PLID 40564 - do not use FindByColumn again, because
					// it can restart at the beginning of the list, and potentially cause an infinite loop
					// use another while loop instead
					BOOL bFound = FALSE;
					while(pRow && !bFound) {
						CString strSerialNum = VarString(pRow->GetValue(clcSerialNum), "");
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
			// It wasn't in the inventory list
		}

		// If we haven't found it by now, search through data
		InvUtils::Barcode_CheckExistenceOfSerialNumberResultEx info;
		CString strProductName;
		_variant_t vExpDate;
		long nProductID = -1;

		//TES 7/7/2008 - PLID 24726 - Pass in the product items that we've already assigned to 
		// Barcode_CheckExistenceofSerialNumber(), so it knows not to return any of them.
		IRowSettingsPtr pSelectedRow = m_ConsignmentList->GetFirstRow();
		CArray<long,long> arSelectedProductItems;
		while(pSelectedRow) {
			arSelectedProductItems.Add(VarLong(pSelectedRow->GetValue(clcID)));
			pSelectedRow = pSelectedRow->GetNextRow();
		}

		// Use the utility function to find a qualifying product or product item ID
		// (c.haag 2008-06-18 12:10) - PLID 30427 - We now tell it to only find purchased inventory items
		if (!InvUtils::Barcode_CheckExistenceOfSerialNumber(strBarcode, -1, FALSE, nProductItemID, nProductID, strProductName, vExpDate, &info, TRUE /* Allow active allocations */, FALSE, TRUE, FALSE /* Don't allow consignment items */, &arSelectedProductItems) || -1 == nProductItemID) {
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
				dlg.m_strOverrideSelectQtyText = "Quantity to assign"; // Custom quantity text
				dlg.m_strOverrideSelectItemText.Format("Please select one or more items for '%s'.",
					strProductName);
				dlg.m_strWhere = FormatString("ProductItemsT.Status <> %d", InvUtils::pisConsignment);

				// (c.haag 2008-01-07 09:49) - Exclude all items already in consignment, and further
				// exclude all items already selected in this dialog's consignment list
				IRowSettingsPtr pExistingRow = m_ConsignmentList->GetFirstRow();
				CArray<long,long> anExistingIDs;
				while (NULL != pExistingRow) {
					anExistingIDs.Add( VarLong(pExistingRow->Value[clcID]) );
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
					_RecordsetPtr prs = CreateRecordset("SELECT ProductItemsT.ID, ProductItemsT.ProductID, ProductItemsT.LocationID, "
						"ProductItemsT.SerialNum, LocationsT.Name AS LocationName, ExpDate, LastCost, "
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
						if (NULL == m_ConsignmentList->FindByColumn(clcID, AdoFldLong(f, "ID"), NULL, VARIANT_FALSE)) {
							pRow = m_ConsignmentList->GetNewRow();
							pRow->PutValue(clcID, f->Item["ID"]->Value);
							pRow->PutValue(clcProductName, _bstr_t(strProductName));
							pRow->PutValue(clcSerialNum, f->Item["SerialNum"]->Value);
							pRow->PutValue(clcExpDate, f->Item["ExpDate"]->Value);
							pRow->PutValue(clcLocationName, f->Item["LocationName"]->Value);
							pRow->PutValue(clcAllocated, f->Item["Allocated"]->Value);
							// (c.haag 2008-02-26 17:11) - PLID 28853 - Get the product and location ID
							pRow->PutValue(clcProductID, f->Item["ProductID"]->Value);
							pRow->PutValue(clcLocationID, f->Item["LocationID"]->Value);
							m_ConsignmentList->AddRowSorted(pRow, NULL);
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

		} // if (!InvUtils::Barcode_CheckExistenceOfSerialNumber(strBarcode, GetLocationID(), FALSE, nProductItemID, nProductID, strProductName, vExpDate, &info, TRUE /* Allow active allocations */) || -1 == nProductItemID) {
		else {
			// We found a match, but we need to make sure the item is not in consignment first
			if (InvUtils::pisConsignment != info.pisStatus) {
				// Now add it to the list if it's not there
				if (NULL == m_ConsignmentList->FindByColumn(clcID, nProductItemID, NULL, VARIANT_FALSE)) {
					IRowSettingsPtr pRow;
					pRow = m_ConsignmentList->GetNewRow();
					pRow->PutValue(clcID, nProductItemID);
					pRow->PutValue(clcProductName, _bstr_t(strProductName));
					pRow->PutValue(clcSerialNum, vBarcode);
					pRow->PutValue(clcExpDate, vExpDate);
					pRow->PutValue(clcLocationName, _bstr_t(GetLocationName(info.nProductLocationID)));
					pRow->PutValue(clcAllocated, (InvUtils::stAllocated == info.stStatus) ? _variant_t(VARIANT_TRUE, VT_BOOL) : _variant_t(VARIANT_FALSE, VT_BOOL) );
					// (c.haag 2008-02-26 17:17) - PLID 28853 - Add the product and location ID
					pRow->PutValue(clcProductID, nProductID);
					pRow->PutValue(clcLocationID, info.nProductLocationID);
					m_ConsignmentList->AddRowSorted(pRow, NULL);
				} else {
					// This item already exists in the selected list
				}
			} else {
				// The item is in consignment; ignore it.
			}
		}
	}
	NxCatchAll("Error in CInvConsignmentAssignDlg::OnBarcodeScan");
	return 0;
}

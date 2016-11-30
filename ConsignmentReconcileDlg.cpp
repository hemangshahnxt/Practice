// ConsignmentReconcileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "ConsignmentReconcileDlg.h"
#include "InvUtils.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"

// CConsignmentReconcileDlg dialog

// (j.jones 2009-03-17 11:35) - PLID 32832 - created

using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum SupplierComboColumns {

	sccID = 0,
	sccName,
};

enum LocationComboColumns {

	lccID = 0,
	lccName,
};

enum ProductListColumns {

	plcProductItemID = 0,
	plcProductID,	
	plcProductName,
	plcCategory,
	plcSupplierName,
	plcSerialNum,
	plcExpDate,	
	plcOldPaid,
	plcPaid,
	plcOldPaidDate,
	plcPaidDate,
	plcOrigOrderCost,
	plcOldPaidAmount,
	plcPaidAmount,
	plcOldInvoiceNum,
	plcInvoiceNum,
	plcDateOrdered,
	plcDateReceived,
	plcStatus,
	plcPatientName,
	plcDateUsed,
	plcProviderName,
	plcLocationName,
	plcNotes,
};

CConsignmentReconcileDlg::CConsignmentReconcileDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConsignmentReconcileDlg::IDD, pParent)
{

}

CConsignmentReconcileDlg::~CConsignmentReconcileDlg()
{
}

void CConsignmentReconcileDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_DISPLAY_CONSIGN_RESULTS, m_btnDisplayResults);
	DDX_Control(pDX, IDC_RADIO_SHOW_USED, m_radioUsed);
	DDX_Control(pDX, IDC_RADIO_SHOW_AVAIL, m_radioAvail);
	DDX_Control(pDX, IDC_RADIO_SHOW_ALL_USEDSTATUS_PRODUCTS, m_radioAll_UsedAvail);
	DDX_Control(pDX, IDC_RADIO_SHOW_UNPAID, m_radioUnpaid);
	DDX_Control(pDX, IDC_RADIO_SHOW_PAID, m_radioPaid);
	DDX_Control(pDX, IDC_RADIO_SHOW_ALL_PAYSTATUS_PRODUCTS, m_radioAll_UnpaidPaid);
}


BEGIN_MESSAGE_MAP(CConsignmentReconcileDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BTN_DISPLAY_CONSIGN_RESULTS, OnBtnDisplayConsignResults)
END_MESSAGE_MAP()


// CConsignmentReconcileDlg message handlers

BOOL CConsignmentReconcileDlg::OnInitDialog() 
{		
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnDisplayResults.AutoSet(NXB_INSPECT);

		m_ProductList = BindNxDataList2Ctrl(IDC_CONSIGN_PRODUCT_LIST, false);
		m_SupplierCombo = BindNxDataList2Ctrl(IDC_CONSIGN_SUPPLIER_COMBO, true);
		m_LocationCombo = BindNxDataList2Ctrl(IDC_CONSIGN_LOCATION_COMBO, true);

		//add an "all suppliers" row
		{
			IRowSettingsPtr pRow = m_SupplierCombo->GetNewRow();
			pRow->PutValue(sccID, (long)-1);
			pRow->PutValue(sccName, _bstr_t(" <All Suppliers>"));
			m_SupplierCombo->AddRowSorted(pRow, NULL);			
		}
		m_SupplierCombo->SetSelByColumn(sccID, (long)-1);

		//add an "all locations" row
		{
			IRowSettingsPtr pRow = m_LocationCombo->GetNewRow();
			pRow->PutValue(lccID, (long)-1);
			pRow->PutValue(lccName, _bstr_t(" <All Locations>"));
			m_LocationCombo->AddRowSorted(pRow, NULL);			
		}
		m_LocationCombo->SetSelByColumn(lccID, (long)-1);

		//set our defaults
		m_radioUsed.SetCheck(TRUE);
		m_radioUnpaid.SetCheck(TRUE);

		CString strFrom;
		//copied and modified from the overview tab so statuses are calculated the same way,
		//except here we ignore adjusted & returned items, and non-consignment items
		strFrom.Format("(SELECT ProductItemsT.ID, ProductItemsT.ProductID, ServiceT.Name AS ProductName, "
			"ProductItemsT.Paid, ProductItemsT.PaidDate, ProductItemsT.PaidAmount, ProductItemsT.InvoiceNum, "
			"CASE WHEN OrderDetailsT.ID Is Null THEN Convert(money, '0.00') ELSE "
			//calculate the line total for the order line, divided by the per-unit quanitity
			"	Round(Convert(money,((((OrderDetailsT.ExtraCost + ((CASE WHEN OrderDetailsT.UseUU = 1 THEN Convert(int,OrderDetailsT.QuantityOrdered / Convert(float,OrderDetailsT.Conversion)) ELSE OrderDetailsT.QuantityOrdered END) * OrderDetailsT.Amount)) * ((100-Convert(float,OrderDetailsT.PercentOff))/100)) - OrderDetailsT.Discount)) / OrderDetailsT.QuantityOrdered),2) "
			"END AS OrigOrderCost, "
			"LocationsT.ID AS LocationID, Coalesce(LocationsT.Name, '<No Location>') AS LocName, "
			"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
			"ServiceT.Category, "
			"ProductItemsT.OrderDetailID, "
			"OrderDetailsT.DateReceived, "
			"CASE WHEN LineItemT.ID Is Not Null THEN %li "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN %li "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
			"	(CASE WHEN AllocatedItemsQ.Status = %li THEN %li ELSE %li END) "
			"ELSE %li END AS StatusType, "
			"CASE WHEN LineItemT.ID Is Not Null THEN 'Billed' "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN 'Case History' "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
			"	(CASE WHEN AllocatedItemsQ.Status = %li THEN 'Used & Not Billed' ELSE 'Allocated' END) "
			"ELSE 'Available' END AS Status, "
			"CASE WHEN LineItemT.ID Is Not Null THEN ChargedPersonT.Last + ', ' + ChargedPersonT.First + ' ' + ChargedPersonT.Middle "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CasePersonT.Last + ', ' + CasePersonT.First + ' ' + CasePersonT.Middle "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN AllocatedPersonT.Last + ', ' + AllocatedPersonT.First + ' ' + AllocatedPersonT.Middle "
			"ELSE '' END AS PatientName, "
			"CASE WHEN LineItemT.ID Is Not Null THEN LineItemT.Date "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CaseHistoryT.SurgeryDate "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
			"	(CASE WHEN AllocatedItemsQ.Status = %li THEN AllocatedItemsQ.CompletionDate ELSE AllocatedItemsQ.InputDate END) "
			"ELSE NULL END AS DateUsed, "
			"AllocatedItemsQ.Notes, "
			"CASE WHEN ChargesT.ID Is Not Null THEN ChargesT.DoctorsProviders "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CaseHistoryT.ProviderID "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN AllocatedItemsQ.ProviderID "
			"ELSE NULL END AS ProviderID, "
			"CASE WHEN ChargesT.ID Is Not Null THEN ChargeProviderPersonT.Last + ', ' + ChargeProviderPersonT.First + ' ' + ChargeProviderPersonT.Middle "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CaseProviderPersonT.Last + ', ' + CaseProviderPersonT.First + ' ' + CaseProviderPersonT.Middle "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN AllocatedItemsQ.ProviderName "
			"ELSE NULL END AS ProviderName, "
			"OrderT.Date AS DateOrdered, "
			"OrderT.Supplier AS SupplierID, "
			"SupplierPersonT.Company AS SupplierName "
			"FROM ProductItemsT "
			"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
			"LEFT JOIN (SELECT PatientInvAllocationsT.PatientID, ProductItemID, PatientInvAllocationsT.InputDate, Coalesce(AppointmentsT.StartTime, PatientInvAllocationsT.CompletionDate) AS CompletionDate, PatientInvAllocationDetailsT.Status, PatientInvAllocationDetailsT.Notes, "
			"		   PatientInvAllocationsT.ProviderID, ProviderPersonT.Last + ', ' + ProviderPersonT.First + ' ' + ProviderPersonT.Middle AS ProviderName "
			"		   FROM PatientInvAllocationDetailsT "
			"		   INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"		   LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
			"		   LEFT JOIN PersonT ProviderPersonT ON PatientInvAllocationsT.ProviderID = ProviderPersonT.ID "
			"		   WHERE ((PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationsT.Status = %li) OR (PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationsT.Status = %li)) "
			"		   ) AS AllocatedItemsQ ON ProductItemsT.ID = AllocatedItemsQ.ProductItemID "
			"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"LEFT JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineItemT ON ChargedProductItemsT.ChargeID = LineItemT.ID "
			"LEFT JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
			"LEFT JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
			"LEFT JOIN PersonT AS AllocatedPersonT ON AllocatedItemsQ.PatientID = AllocatedPersonT.ID "
			"LEFT JOIN PersonT AS ChargedPersonT ON LineItemT.PatientID = ChargedPersonT.ID "
			"LEFT JOIN PersonT AS CasePersonT ON CaseHistoryT.PersonID = CasePersonT.ID "
			"LEFT JOIN PersonT AS ChargeProviderPersonT ON ChargesT.DoctorsProviders = ChargeProviderPersonT.ID "
			"LEFT JOIN PersonT AS CaseProviderPersonT ON CaseHistoryT.ProviderID = CaseProviderPersonT.ID "
			"LEFT JOIN OrderDetailsT ON ProductItemsT.OrderDetailID = OrderDetailsT.ID "
			"LEFT JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
			"LEFT JOIN PersonT AS SupplierPersonT ON OrderT.Supplier = SupplierPersonT.ID "
			"WHERE ProductItemsT.ReturnedFrom Is Null AND ServiceT.Active = 1 "
			"AND ProductItemsT.Deleted = 0 AND ProductItemsT.Status = %li) AS ProductItemsQ",
			InvUtils::ostUsed, InvUtils::ostUsed, InvUtils::iadsUsed, InvUtils::ostUsed, InvUtils::ostAllocated, InvUtils::ostAvailable,
			InvUtils::iadsUsed, InvUtils::iadsUsed,	InvUtils::iadsActive, InvUtils::iasActive, InvUtils::iadsUsed, InvUtils::iasCompleted,
			InvUtils::pisConsignment);
		
		m_ProductList->PutFromClause(_bstr_t(strFrom));

		//go ahead and show the results for our default filters,
		//which will automatically show all used, unpaid consignment
		//products for the current location
		OnBtnDisplayConsignResults();

	}NxCatchAll("Error in CConsignmentReconcileDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CConsignmentReconcileDlg::OnOk()
{
	try {

		if(!Save(FALSE)) {
			return;
		}

		CNxDialog::OnOK();

	}NxCatchAll("Error in CConsignmentReconcileDlg::OnOk");
}

void CConsignmentReconcileDlg::OnCancel()
{
	try {

		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		COleCurrency cyInvalid;
		cyInvalid.SetStatus(COleCurrency::invalid);

		BOOL bChanged = FALSE;

		//warn them if anything changed
		IRowSettingsPtr pRow = m_ProductList->GetFirstRow();
		while(pRow && !bChanged) {
	
			BOOL bOldPaid = VarBool(pRow->GetValue(plcOldPaid), FALSE);
			BOOL bPaid = VarBool(pRow->GetValue(plcPaid), FALSE);			
			COleDateTime dtOldPaidDate = VarDateTime(pRow->GetValue(plcOldPaidDate), dtInvalid);
			COleDateTime dtPaidDate = VarDateTime(pRow->GetValue(plcPaidDate), dtInvalid);
			COleCurrency cyOldPaidAmount = VarCurrency(pRow->GetValue(plcOldPaidAmount), cyInvalid);
			COleCurrency cyPaidAmount = VarCurrency(pRow->GetValue(plcPaidAmount), cyInvalid);
			CString strOldInvoiceNum = VarString(pRow->GetValue(plcOldInvoiceNum), "");
			CString strInvoiceNum = VarString(pRow->GetValue(plcInvoiceNum), "");

			//did anything change?
			if(bOldPaid != bPaid || dtOldPaidDate.m_dt != dtPaidDate.m_dt || cyOldPaidAmount != cyPaidAmount || strOldInvoiceNum != strInvoiceNum) {

				//we will stop looping
				bChanged = TRUE;
			}

			pRow = pRow->GetNextRow();
		}

		if(bChanged && IDNO == MessageBox("If you cancel, the changes you have made to the current product list will not be saved.\n\n"
			"Are you sure you wish to cancel these changes?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
			
			return;
		}

		CNxDialog::OnCancel();

	}NxCatchAll("Error in CConsignmentReconcileDlg::OnCancel");
}

void CConsignmentReconcileDlg::OnBtnDisplayConsignResults()
{
	try {

		//this save call will warn if changes will be made
		if(!Save(TRUE)) {
			return;
		}

		CString strWhere;

		IRowSettingsPtr pSupplierRow = m_SupplierCombo->GetCurSel();
		if(pSupplierRow != NULL) {
			long nSupplierID = VarLong(pSupplierRow->GetValue(sccID), -1);
			if(nSupplierID != -1) {
				CString strSql;
				//filter on the supplier the product item was ordered from
				strSql.Format("SupplierID = %li", nSupplierID);
				if(!strWhere.IsEmpty()) {
					strWhere += " AND ";
				}
				strWhere += strSql;
			}
		}

		IRowSettingsPtr pLocationRow = m_LocationCombo->GetCurSel();
		if(pLocationRow != NULL) {
			long nLocationID = VarLong(pLocationRow->GetValue(lccID), -1);
			if(nLocationID != -1) {
				CString strSql;
				//filter on the location the product item belongs to
				strSql.Format("LocationID = %li", nLocationID);
				if(!strWhere.IsEmpty()) {
					strWhere += " AND ";
				}
				strWhere += strSql;
			}
		}

		if(m_radioUsed.GetCheck()) {
			CString strSql;
			//filter on only products that have been used
			strSql.Format("StatusType <> %li", InvUtils::ostAvailable);
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strSql;
		}
		else if(m_radioAvail.GetCheck()) {
			CString strSql;
			//filter on only products that have not been used
			strSql.Format("StatusType = %li", InvUtils::ostAvailable);
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strSql;
		}

		if(m_radioUnpaid.GetCheck()) {
			CString strSql;
			//filter on only products that haven't been paid for
			strSql.Format("Paid = 0");
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strSql;
		}
		else if(m_radioPaid.GetCheck()) {
			CString strSql;
			//filter on only products that have been paid for
			strSql.Format("Paid = 1");
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strSql;
		}

		m_ProductList->PutWhereClause(_bstr_t(strWhere));
		m_ProductList->Requery();

	}NxCatchAll("Error in CConsignmentReconcileDlg::OnBtnDisplayConsignResults");
}

BOOL CConsignmentReconcileDlg::Save(BOOL bWarnBeforeSaving)
{
	long nAuditTransactionID = -1;

	try {

		CWaitCursor pWait;

		//loop through the list and build the changes that need made

		CString strSqlBatch;

		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);

		COleCurrency cyInvalid;
		cyInvalid.SetStatus(COleCurrency::invalid);

		IRowSettingsPtr pRow = m_ProductList->GetFirstRow();
		while(pRow) {

			long nProductItemID = VarLong(pRow->GetValue(plcProductItemID));			
			BOOL bOldPaid = VarBool(pRow->GetValue(plcOldPaid), FALSE);
			BOOL bPaid = VarBool(pRow->GetValue(plcPaid), FALSE);
			COleDateTime dtOldPaidDate = VarDateTime(pRow->GetValue(plcOldPaidDate), dtInvalid);
			COleDateTime dtPaidDate = VarDateTime(pRow->GetValue(plcPaidDate), dtInvalid);
			COleCurrency cyOldPaidAmount = VarCurrency(pRow->GetValue(plcOldPaidAmount), cyInvalid);
			COleCurrency cyPaidAmount = VarCurrency(pRow->GetValue(plcPaidAmount), cyInvalid);
			CString strOldInvoiceNum = VarString(pRow->GetValue(plcOldInvoiceNum), "");
			CString strInvoiceNum = VarString(pRow->GetValue(plcInvoiceNum), "");
			CString strProductName = VarString(pRow->GetValue(plcProductName), "");
			CString strSerialNum = VarString(pRow->GetValue(plcSerialNum), "");
			COleDateTime dtExpDate = VarDateTime(pRow->GetValue(plcExpDate), dtInvalid);

			CString strProdItemInfo;
			if(!strSerialNum.IsEmpty()) {
				strProdItemInfo += "Serial Num: ";
				strProdItemInfo += strSerialNum;
			}
			if(dtExpDate.GetStatus() != COleDateTime::invalid) {
				if(!strProdItemInfo.IsEmpty()) {
					strProdItemInfo += ", ";
				}
				strProdItemInfo += "Exp. Date: ";
				strProdItemInfo += FormatDateTimeForInterface(dtExpDate);
			}

			//see if we need to save anything
			if(bOldPaid != bPaid || dtOldPaidDate.m_dt != dtPaidDate.m_dt || cyOldPaidAmount != cyPaidAmount || strOldInvoiceNum != strInvoiceNum) {

				//warn if the product is paid but has no date
				if(bPaid && dtPaidDate.GetStatus() == COleDateTime::invalid) {
					CString strWarn;
					strWarn.Format("The product %s (%s) is marked as being paid, but has no paid date. Please correct this before saving.",
						strProductName, strProdItemInfo);
					AfxMessageBox(strWarn);
					return FALSE;
				}

				CString strPaidDate = "NULL";
				if(dtPaidDate.GetStatus() != COleDateTime::invalid) {
					strPaidDate.Format("Convert(datetime, '%s')", _Q(FormatDateTimeForSql(dtPaidDate)));
				}

				CString strPaidAmount = "NULL";
				if(cyPaidAmount.GetStatus() != COleCurrency::invalid) {
					strPaidAmount.Format("Convert(money, '%s')", _Q(FormatCurrencyForSql(cyPaidAmount)));
				}

				AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductItemsT SET Paid = %li, "
					"PaidDate = %s, PaidAmount = %s, InvoiceNum = '%s' "
					"WHERE ID = %li", bPaid ? 1 : 0, strPaidDate, strPaidAmount, _Q(strInvoiceNum), nProductItemID);

				//now for auditing
				if(bOldPaid != bPaid) {

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}

					CString strOld = strProdItemInfo;
					if(bOldPaid) {
						strOld += " - Paid";
					}
					else {
						strOld += " - Unpaid";
					}

					CString strNew;
					if(bPaid) {
						strNew = "Paid";
					}
					else {
						strNew = "Unpaid";
					}

					AuditEvent(-1, strProductName, nAuditTransactionID, aeiConsignmentPaid, nProductItemID, strOld, strNew, aepMedium, aetChanged);
				}

				if(dtOldPaidDate.m_dt != dtPaidDate.m_dt) {

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}

					CString strOld = strProdItemInfo;
					if(dtOldPaidDate.GetStatus() != COleDateTime::invalid) {
						strOld += " - ";
						strOld += FormatDateTimeForInterface(dtOldPaidDate);
					}
					else {
						strOld += " - No Paid Date";
					}

					CString strNew;
					if(dtPaidDate.GetStatus() != COleDateTime::invalid) {
						strNew = FormatDateTimeForInterface(dtPaidDate);
					}
					else {
						strNew = "No Paid Date";
					}

					AuditEvent(-1, strProductName, nAuditTransactionID, aeiConsignmentPaidDate, nProductItemID, strOld, strNew, aepMedium, aetChanged);
				}

				if(cyOldPaidAmount != cyPaidAmount) {

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}

					CString strOld = strProdItemInfo;
					if(cyOldPaidAmount.GetStatus() != COleCurrency::invalid) {
						strOld += " - ";
						strOld += FormatCurrencyForInterface(cyOldPaidAmount);
					}
					else {
						strOld += " - No Paid Amount";
					}

					CString strNew;
					if(cyPaidAmount.GetStatus() != COleCurrency::invalid) {
						strNew = FormatCurrencyForInterface(cyPaidAmount);
					}
					else {
						strNew = "No Paid Amount";
					}

					AuditEvent(-1, strProductName, nAuditTransactionID, aeiConsignmentPaidAmount, nProductItemID, strOld, strNew, aepMedium, aetChanged);
				}

				if(strOldInvoiceNum != strInvoiceNum) {

					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}

					CString strOld = strProdItemInfo;
					if(!strOldInvoiceNum.IsEmpty()) {
						strOld += " - ";
						strOld += strOldInvoiceNum;
					}
					else {
						strOld += " - No Invoice Number";
					}

					CString strNew;
					if(!strInvoiceNum.IsEmpty()) {
						strNew = strInvoiceNum;
					}
					else {
						strNew = "No Invoice Number";
					}
					
					AuditEvent(-1, strProductName, nAuditTransactionID, aeiConsignmentInvoiceNum, nProductItemID, strOld, strNew, aepMedium, aetChanged);
				}
			}
			
			pRow = pRow->GetNextRow();
		}

		if(!strSqlBatch.IsEmpty()) {

			//warn before comittting
			if(bWarnBeforeSaving && IDNO == MessageBox("The changes you have made to the current product list will be saved.\n\n"
				"Are you sure you wish to continue?", "Practice", MB_ICONQUESTION|MB_YESNO)) {

				//rollback our sudit
				if(nAuditTransactionID != -1) {
					RollbackAuditTransaction(nAuditTransactionID);
				}

				return FALSE;
			}

			ExecuteSqlBatch(strSqlBatch);
		}

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

		//now update the old data to match the new data, if something changed
		if(!strSqlBatch.IsEmpty()) {
			pRow = m_ProductList->GetFirstRow();
			while(pRow) {

				pRow->PutValue(plcOldPaid, pRow->GetValue(plcPaid));
				pRow->PutValue(plcOldPaidDate, pRow->GetValue(plcPaidDate));
				pRow->PutValue(plcOldPaidAmount, pRow->GetValue(plcPaidAmount));
				pRow->PutValue(plcOldInvoiceNum, pRow->GetValue(plcInvoiceNum));
				
				pRow = pRow->GetNextRow();
			}
		}

		return TRUE;

	}NxCatchAllCall("Error in CConsignmentReconcileDlg::Save",

		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);

	return FALSE;
}

BEGIN_EVENTSINK_MAP(CConsignmentReconcileDlg, CNxDialog)
ON_EVENT(CConsignmentReconcileDlg, IDC_CONSIGN_PRODUCT_LIST, 10, OnEditingFinishedConsignProductList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
ON_EVENT(CConsignmentReconcileDlg, IDC_CONSIGN_PRODUCT_LIST, 8, OnEditingStartingConsignProductList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
ON_EVENT(CConsignmentReconcileDlg, IDC_CONSIGN_PRODUCT_LIST, 9, OnEditingFinishingConsignProductList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CConsignmentReconcileDlg::OnEditingFinishedConsignProductList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		
		if(nCol == plcPaid) {
			BOOL bPaid = VarBool(varNewValue, FALSE);
			//if they uncheck the paid column, clear the paid date and invoice num. columns
			if(!bPaid) {
				pRow->PutValue(plcPaidDate, g_cvarNull);
				pRow->PutValue(plcPaidAmount, g_cvarNull);
				pRow->PutValue(plcInvoiceNum, "");
			}
			else {
				//if they did check the paid column, auto-enter today's date
				COleDateTime dtNow = COleDateTime::GetCurrentTime();
				dtNow.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());
				pRow->PutValue(plcPaidDate, _variant_t(dtNow, VT_DATE));

				//and update the paid amount to be the order amount (if we have one)
				pRow->PutValue(plcPaidAmount, pRow->GetValue(plcOrigOrderCost));
			}
			return;
		}

	}NxCatchAll("Error in CConsignmentReconcileDlg::OnEditingFinishedConsignProductList");
}

void CConsignmentReconcileDlg::OnEditingStartingConsignProductList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//disallow editing the paid date, amount, and invoice number if the paid column is unchecked
		if(!VarBool(pRow->GetValue(plcPaid), FALSE)
			&& (nCol == plcPaidDate || nCol == plcPaidAmount || nCol == plcInvoiceNum)) {
			
			*pbContinue = FALSE;
			return;
		}

	}NxCatchAll("Error in CConsignmentReconcileDlg::OnEditingStartingConsignProductList");
}

void CConsignmentReconcileDlg::OnEditingFinishingConsignProductList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//if editing the paid date, clear it if they enter an invalid date, and continue
		if(nCol == plcPaidDate) {

			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);

			COleDateTime dtNewDate = VarDateTime(pvarNewValue, dtInvalid);
			if(dtNewDate.GetStatus() == COleDateTime::invalid || dtNewDate.m_dt <= 0) {
				*pvarNewValue = g_cvarNull;
				return;
			}
		}

	}NxCatchAll("Error in CConsignmentReconcileDlg::OnEditingFinishingConsignProductList");
}

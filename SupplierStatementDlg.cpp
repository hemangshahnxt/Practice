// SupplierStatementDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SupplierStatementDlg.h"
#include "InternationalUtils.h"
#include "InvUtils.h"

// CSupplierStatementDlg dialog

// (j.jones 2009-03-17 11:30) - PLID 32831 - created

using namespace NXDATALIST2Lib;

enum SupplierComboColumns {

	sccID = 0,
	sccName,
};

enum OrderComboColumn {

	occID = 0,
	occSupplierID,
	occDate,
	occFirstReceived,
	occCompany,
	occDescription,
	occTotalQuantity,
};

enum OrderedProductListColumn {

	oplcID = 0,
	oplcParentID,
	oplcOrderDetailID,
	oplcProductItemID,
	oplcProductID,
	oplcProductName,
	oplcQtyOrdered,
	oplcQtyReceived,
	oplcQtyBackOrdered,
	oplcCatalogNum,
	oplcUnitCost,
	oplcTotalAmount,
};

CSupplierStatementDlg::CSupplierStatementDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSupplierStatementDlg::IDD, pParent)
{

}

CSupplierStatementDlg::~CSupplierStatementDlg()
{
}

void CSupplierStatementDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_DISPLAY_STATEMENT_RESULTS, m_btnDisplayResults);
	DDX_Control(pDX, IDC_RADIO_FILTER_BY_ORDER, m_radioOrderFilter);
	DDX_Control(pDX, IDC_RADIO_FILTER_BY_DATES, m_radioDateFilter);
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_ORDER_DETAIL, m_radioGroupByOrderDetail);
	DDX_Control(pDX, IDC_RADIO_GROUP_BY_SERIAL_NUM, m_radioGroupByProductItem);
	DDX_Control(pDX, IDC_DT_SUPPLIER_STATEMENT_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_DT_SUPPLIER_STATEMENT_TO, m_dtTo);
}


BEGIN_MESSAGE_MAP(CSupplierStatementDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_BTN_DISPLAY_STATEMENT_RESULTS, OnBtnDisplayStatementResults)
	ON_BN_CLICKED(IDC_RADIO_FILTER_BY_ORDER, OnRadioFilterByOrder)
	ON_BN_CLICKED(IDC_RADIO_FILTER_BY_DATES, OnRadioFilterByDates)
END_MESSAGE_MAP()


// CSupplierStatementDlg message handlers

BOOL CSupplierStatementDlg::OnInitDialog() 
{		
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnDisplayResults.AutoSet(NXB_INSPECT);

		m_OrderedProductList = BindNxDataList2Ctrl(IDC_ORDERED_PRODUCT_LIST, false);
		m_ReturnedProductList = BindNxDataList2Ctrl(IDC_RETURNED_PRODUCT_LIST, false);
		m_SupplierCombo = BindNxDataList2Ctrl(IDC_SUPPLIER_COMBO, true);
		m_OrderCombo = BindNxDataList2Ctrl(IDC_ORDER_COMBO, false);

		//build our order combo in code, only show received orders,
		//but include all products on them, even if they are inactive
		m_OrderCombo->PutFromClause("(SELECT OrderT.ID, PersonT.Company, OrderT.Date, Max(OrderDetailsT.DateReceived) AS LastReceived, OrderT.Description, "
			"Sum(CASE WHEN OrderDetailsT.UseUU = 1 THEN Convert(int,(OrderDetailsT.QuantityOrdered / Convert(float,OrderDetailsT.Conversion))) ELSE OrderDetailsT.QuantityOrdered END) AS TotalQuantityOrdered, "
			"PersonT.ID AS SupplierID "
			"FROM OrderT "
			"INNER JOIN OrderDetailsT ON OrderT.ID = OrderDetailsT.OrderID "
			"INNER JOIN ServiceT ON OrderDetailsT.ProductID = ServiceT.ID "
			"LEFT JOIN SupplierT ON OrderT.Supplier = SupplierT.PersonID "
			"LEFT JOIN PersonT ON SupplierT.PersonID = PersonT.ID "
			"WHERE OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 "
			"AND OrderDetailsT.DateReceived Is Not Null "
			"GROUP BY OrderT.ID, PersonT.Company, PersonT.ID, OrderT.Date, OrderT.Description) AS OrdersQ");
		m_OrderCombo->Requery();

		//add an "all suppliers" row
		{
			IRowSettingsPtr pRow = m_SupplierCombo->GetNewRow();
			pRow->PutValue(sccID, (long)-1);
			pRow->PutValue(sccName, _bstr_t(" <All Suppliers>"));
			m_SupplierCombo->AddRowSorted(pRow, NULL);			
		}
		m_SupplierCombo->SetSelByColumn(sccID, (long)-1);

		//set our defaults
		m_radioOrderFilter.SetCheck(TRUE);
		m_radioGroupByOrderDetail.SetCheck(TRUE);
		
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		m_dtTo.SetValue(dtNow);

		COleDateTimeSpan dtMonth;
		dtMonth.SetDateTimeSpan(30, 0, 0, 0);
		dtNow -= dtMonth;
		m_dtFrom.SetValue(dtNow);
		GetDlgItem(IDC_DT_SUPPLIER_STATEMENT_FROM)->EnableWindow(FALSE);
		GetDlgItem(IDC_DT_SUPPLIER_STATEMENT_TO)->EnableWindow(FALSE);

		CString strFrom;
		//build our ordered product list, which should show every line item from an order,
		//expandable to show product items that came in, including where they are now (could be returned)
		strFrom.Format("(SELECT ProductItemsT.ID, ProductItemsT.ProductID, ServiceT.Name AS ProductName, "
			"LocationsT.ID AS LocationID, Coalesce(LocationsT.Name, '<No Location>') AS LocName, "
			"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
			"ProductItemsT.Status AS ProductItemStatus, ServiceT.Category, "
			"ProductItemsT.OrderDetailID, "
			"CASE WHEN ProductItemsT.Status = %li THEN ProductItemsT.Paid ELSE NULL END AS ConsignmentPaid, "
			"CASE WHEN ProductItemsT.Status = %li THEN ProductItemsT.PaidDate ELSE NULL END AS ConsignmentPaidDate, "
			"CASE WHEN ProductItemsT.Status = %d THEN 'Consignment' ELSE CASE WHEN ProductItemsT.Status = %d THEN 'Warranty' ELSE 'Purchased Inv.' END END AS ProductType, "
			"OrderDetailsT.DateReceived, "
			"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN %li "
			"WHEN ProductItemsT.Deleted = 1 THEN %li "
			"WHEN LineItemT.ID Is Not Null THEN %li "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN %li "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
			"	(CASE WHEN AllocatedItemsQ.Status = %li THEN %li ELSE %li END) "
			"ELSE %li END AS StatusType, "
			"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN 'Returned' "
			"WHEN ProductItemsT.Deleted = 1 THEN CASE WHEN ProductAdjustmentCategoriesT.Name Is Null THEN 'Adjusted' "
			"	ELSE 'Adjusted - ' + ProductAdjustmentCategoriesT.Name END "
			"WHEN LineItemT.ID Is Not Null THEN 'Billed' "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN 'Case History' "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
			"	(CASE WHEN AllocatedItemsQ.Status = %li THEN 'Used & Not Billed' ELSE 'Allocated' END) "
			"ELSE 'Available' END AS Status, "
			"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN '' "
			"WHEN ProductItemsT.Deleted = 1 THEN '' "
			"WHEN LineItemT.ID Is Not Null THEN ChargedPersonT.Last + ', ' + ChargedPersonT.First + ' ' + ChargedPersonT.Middle "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CasePersonT.Last + ', ' + CasePersonT.First + ' ' + CasePersonT.Middle "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN AllocatedPersonT.Last + ', ' + AllocatedPersonT.First + ' ' + AllocatedPersonT.Middle "
			"ELSE '' END AS PatientName, "
			"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN SupplierReturnGroupsT.ReturnDate "
			"WHEN ProductItemsT.Deleted = 1 THEN ProductAdjustmentsT.Date "
			"WHEN LineItemT.ID Is Not Null THEN LineItemT.Date "
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
			"LEFT JOIN ProductAdjustmentsT ON ProductItemsT.AdjustmentID = ProductAdjustmentsT.ID "
			"LEFT JOIN ProductAdjustmentCategoriesT ON ProductAdjustmentsT.ProductAdjustmentCategoryID = ProductAdjustmentCategoriesT.ID "
			"INNER JOIN OrderDetailsT ON ProductItemsT.OrderDetailID = OrderDetailsT.ID "
			"INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
			"LEFT JOIN PersonT AS SupplierPersonT ON OrderT.Supplier = SupplierPersonT.ID "
			"WHERE ProductItemsT.ReturnedFrom Is Null "
			// (b.cardillo 2012-08-20 21:01) - PLID 52232 - Don't look at nulls when checking for not in SupplierReturnItemsT (wasn't 
			// sure if this code was being maintained or not, as it's currently unused, but figured better safe than sorry)
			"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM SupplierReturnItemsT WHERE ProductItemID IS NOT NULL) "
			") AS ProductItemsQ",
			InvUtils::pisConsignment, InvUtils::pisConsignment, InvUtils::pisConsignment, InvUtils::pisWarranty, InvUtils::ostAdjusted, InvUtils::ostAdjusted, InvUtils::ostUsed, InvUtils::ostUsed, InvUtils::iadsUsed, InvUtils::ostUsed, InvUtils::ostAllocated, InvUtils::ostAvailable,
			InvUtils::iadsUsed, InvUtils::iadsUsed,	InvUtils::iadsActive, InvUtils::iasActive, InvUtils::iadsUsed, InvUtils::iasCompleted);

		m_OrderedProductList->PutFromClause(_bstr_t(strFrom));

		//now build our returned product list, build our ordered product list,
		//which should show every line item from a return for a given order,
		//expandable to show the product items information
		strFrom.Format("(SELECT ProductItemsT.ID, ProductItemsT.ProductID, ServiceT.Name AS ProductName, "
			"LocationsT.ID AS LocationID, Coalesce(LocationsT.Name, '<No Location>') AS LocName, "
			"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
			"ProductItemsT.Status AS ProductItemStatus, ServiceT.Category, "
			"ProductItemsT.OrderDetailID, "
			"CASE WHEN ProductItemsT.Status = %li THEN ProductItemsT.Paid ELSE NULL END AS ConsignmentPaid, "
			"CASE WHEN ProductItemsT.Status = %li THEN ProductItemsT.PaidDate ELSE NULL END AS ConsignmentPaidDate, "
			"CASE WHEN ProductItemsT.Status = %d THEN 'Consignment' ELSE CASE WHEN ProductItemsT.Status = %d THEN 'Warranty' ELSE 'Purchased Inv.' END END AS ProductType, "
			"OrderDetailsT.DateReceived, "
			"SupplierReturnGroupsT.ReturnDate, "			
			"OrderT.Date AS DateOrdered, "
			"OrderT.Supplier AS SupplierID, "
			"SupplierPersonT.Company AS SupplierName "
			"FROM ProductItemsT "
			"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
			"INNER JOIN SupplierReturnItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
			"INNER JOIN SupplierReturnGroupsT ON SupplierReturnItemsT.ReturnGroupID = SupplierReturnGroupsT.ID "
			"LEFT JOIN ProductAdjustmentsT ON ProductItemsT.AdjustmentID = ProductAdjustmentsT.ID "
			"LEFT JOIN ProductAdjustmentCategoriesT ON ProductAdjustmentsT.ProductAdjustmentCategoryID = ProductAdjustmentCategoriesT.ID "
			"INNER JOIN OrderDetailsT ON ProductItemsT.OrderDetailID = OrderDetailsT.ID "
			"INNER JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
			"LEFT JOIN PersonT AS SupplierPersonT ON OrderT.Supplier = SupplierPersonT.ID "
			"LEFT JOIN ProductAdjustmentsT CreatingProductAdjustmentsT ON ProductItemsT.CreatingAdjustmentID = CreatingProductAdjustmentsT.ID "
			"WHERE ProductItemsT.ReturnedFrom Is Null "
			"AND ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null "
			") AS ProductItemsQ",
			InvUtils::pisConsignment, InvUtils::pisConsignment, InvUtils::pisConsignment, InvUtils::pisWarranty);

		m_ReturnedProductList->PutFromClause(_bstr_t(strFrom));

	}NxCatchAll("Error in CSupplierStatementDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CSupplierStatementDlg::OnOk()
{
	try {

		CNxDialog::OnOK();

	}NxCatchAll("Error in CSupplierStatementDlg::OnOk");
}

void CSupplierStatementDlg::OnCancel()
{
	try {

		CNxDialog::OnCancel();

	}NxCatchAll("Error in CSupplierStatementDlg::OnCancel");
}

void CSupplierStatementDlg::OnBtnDisplayStatementResults()
{
	try {

		CString strWhere;

		if(m_radioDateFilter.GetCheck()) {
			
			COleDateTime dtFrom = m_dtFrom.GetValue();
			COleDateTime dtTo = m_dtTo.GetValue();

			//if filtering by order date, don't allow a from date > to date, or invalid dates
			if(dtFrom.GetStatus() == COleDateTime::invalid) { 
				AfxMessageBox("You must have a valid From date entered in order to display results for date range of orders.");
				return;
			}
			if(dtTo.GetStatus() == COleDateTime::invalid) { 
				AfxMessageBox("You must have a valid To date entered in order to display results for date range of orders.");
				return;
			}
			if(dtTo < dtFrom) { 
				AfxMessageBox("You cannot have a From date greater than your To date.");
				return;
			}

			CString strDateFilter;
			strDateFilter.Format("(DateOrdered >= '%s' AND DateOrdered < DATEADD(day, 1, '%s'))", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));				
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strDateFilter;
		}		
		else if(m_radioOrderFilter.GetCheck()) {
			
			IRowSettingsPtr pRow = m_OrderCombo->GetCurSel();
			if(pRow == NULL) {
				//if filtering by order, require an order to be selected
				AfxMessageBox("You must have an Order selected in order to display results for a specific order.");
				return;
			}
			else {
				long nOrderID = VarLong(pRow->GetValue(occID), -1);
				if(nOrderID != -1) {
					CString strOrder;
					strOrder.Format("OrderDetailID IN (SELECT ID FROM OrderDetailsT WHERE OrderID = %li)", nOrderID);
					if(!strWhere.IsEmpty()) {
						strWhere += " AND ";
					}
					strWhere += strOrder;
				}
			}
		}

	}NxCatchAll("Error in CSupplierStatementDlg::OnBtnDisplayStatementResults");
}

void CSupplierStatementDlg::OnRadioFilterByOrder()
{
	try {

		BOOL bOrderFilter = m_radioOrderFilter.GetCheck();
		m_OrderCombo->Enabled = bOrderFilter;
		GetDlgItem(IDC_DT_SUPPLIER_STATEMENT_FROM)->EnableWindow(!bOrderFilter);
		GetDlgItem(IDC_DT_SUPPLIER_STATEMENT_TO)->EnableWindow(!bOrderFilter);

	}NxCatchAll("Error in CSupplierStatementDlg::OnRadioFilterByOrder");
}

void CSupplierStatementDlg::OnRadioFilterByDates()
{
	try {

		OnRadioFilterByOrder();

	}NxCatchAll("Error in CSupplierStatementDlg::OnRadioFilterByDates");
}

BEGIN_EVENTSINK_MAP(CSupplierStatementDlg, CNxDialog)
ON_EVENT(CSupplierStatementDlg, IDC_SUPPLIER_COMBO, 16, OnSelChosenSupplierCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CSupplierStatementDlg::OnSelChosenSupplierCombo(LPDISPATCH lpRow)
{
	try {

		long nSupplierID = -1;

		IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			nSupplierID = VarLong(pRow->GetValue(sccID), -1);
		}

		//refilter our order combo
		CString strWhere;
		if(nSupplierID != -1) {
			strWhere.Format("SupplierID = %li", nSupplierID);
		}

		//get the currently selected order ID, if we have one
		long nOrderID = -1;
		IRowSettingsPtr pOrderRow = m_OrderCombo->GetCurSel();
		if(pOrderRow) {
			nOrderID = VarLong(pOrderRow->GetValue(occID), -1);
		}

		m_OrderCombo->PutWhereClause(_bstr_t(strWhere));
		m_OrderCombo->Requery();

		if(nOrderID != -1) {
			m_OrderCombo->SetSelByColumn(occID, nOrderID);
		}

	}NxCatchAll("Error in CSupplierStatementDlg::OnSelChosenSupplierCombo");
}

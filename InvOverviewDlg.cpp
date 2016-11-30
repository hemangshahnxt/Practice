// InvOverviewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InvOverviewDlg.h"
#include "InvUtils.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2007-11-06 12:02) - PLID 27989 - created

using namespace NXDATALIST2Lib;
using namespace InvUtils;

//DRT 11/28/2007 - PLID 28215 - Cleaned up a few things.  Moved some text to defines in InvUtils.  Moved an enum to InvUtils.  Added InvUtils here.

// (j.jones 2009-01-12 16:56) - PLID 32703 - added Color defines since they are used more than once
#define COLOR_AVAILABLE	RGB(0, 112, 0)
#define COLOR_ALLOCATED	RGB(0, 0, 192)
#define COLOR_USED		RGB(180, 0, 180)
#define COLOR_ADJUSTED	RGB(128, 64, 0)
#define COLOR_BLACK		RGB(0, 0, 0)

/////////////////////////////////////////////////////////////////////////////
// CInvOverviewDlg dialog

enum OverviewListColumn {

	olcID = 0,
	olcProductID,
	olcProductName,
	olcSerialNum,
	olcExpDate,
	olcType,	
	olcDateReceived,
	olcDateOrdered,	//TES 9/3/2008 - PLID 29779 - Added.
	olcConsignmentPaidDate,	// (j.jones 2009-03-19 09:05) - PLID 33579
	olcPatientName,
	olcStatus,
	olcStatusDate,
	olcNotes,	// (j.jones 2008-01-16 16:42) - PLID 28640
	olcLocationName,	// (j.jones 2008-03-18 17:33) - PLID 29308 - moved these columns to the end
	olcProviderID,		// (j.jones 2008-03-05 16:59) - PLID 29202
	olcProviderName,
	olcColor,	// (j.jones 2009-01-12 16:57) - PLID 32703
};

enum TypeComboColumn {

	tccID = 0,
	tccName,
};

enum StatusComboColumn {

	stccID = 0,
	stccName,
};

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

// (j.jones 2008-03-05 16:09) - PLID 29202 - added provider filter
enum ProviderComboColumn {

	provccID = 0,
	provccName,
};

//TES 9/3/2008 - PLID 31237 - Added options for which date to filter on.
enum DateFilterTypeColumns {
	dftcID = 0,
	dftcName = 1,
};

// (j.jones 2009-02-09 16:29) - PLID 32873 - added order column enum
enum OrderComboColumn {

	occID = 0,
	occDate,
	occFirstReceived,
	occCompany,
	occDescription,
	occTotalQuantity,
};

CInvOverviewDlg::CInvOverviewDlg(CWnd* pParent)
	: CNxDialog(CInvOverviewDlg::IDD, pParent),
	m_CategoryChecker(NetUtils::CPTCategories),
	m_ProductChecker(NetUtils::Products),
	m_SupplierChecker(NetUtils::Suppliers),
	m_LocationChecker(NetUtils::LocationsT),
	m_ProviderChecker(NetUtils::Providers),
	m_OrderChecker(NetUtils::OrderT)
{
	//{{AFX_DATA_INIT(CInvOverviewDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CInvOverviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvOverviewDlg)
	DDX_Control(pDX, IDC_RADIO_ALL_OVERVIEW_DATES, m_radioAllDates);
	DDX_Control(pDX, IDC_RADIO_OVERVIEW_DATE_RANGE, m_radioDateRange);
	DDX_Control(pDX, IDC_DT_OVERVIEW_FROM, m_DateFrom);
	DDX_Control(pDX, IDC_DT_OVERVIEW_TO, m_DateTo);
	DDX_Control(pDX, IDC_TOTAL_OVERVIEW_ROWS_LABEL, m_nxstaticTotalOverviewRowsLabel);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CInvOverviewDlg, IDC_DT_OVERVIEW_FROM, 2 /* Change */, OnChangeDtOverviewFrom, VTS_NONE)
//	ON_EVENT(CInvOverviewDlg, IDC_DT_OVERVIEW_TO, 2 /* Change */, OnChangeDtOverviewTo, VTS_NONE)

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
BEGIN_MESSAGE_MAP(CInvOverviewDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvOverviewDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DT_OVERVIEW_FROM, OnChangeDtOverviewFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DT_OVERVIEW_TO, OnChangeDtOverviewTo)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_RADIO_ALL_OVERVIEW_DATES, OnRadioAllOverviewDates)
	ON_BN_CLICKED(IDC_RADIO_OVERVIEW_DATE_RANGE, OnRadioOverviewDateRange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvOverviewDlg message handlers

BOOL CInvOverviewDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {

		m_brush.CreateSolidBrush(PaletteColor(0x00FFDBDB));

		m_OverviewList = BindNxDataList2Ctrl(IDC_OVERVIEW_LIST, false);
		m_TypeCombo = BindNxDataList2Ctrl(IDC_OVERVIEW_PRODUCT_TYPE_COMBO, false);
		m_StatusCombo = BindNxDataList2Ctrl(IDC_OVERVIEW_STATUS_COMBO, false);
		m_CategoryCombo = BindNxDataList2Ctrl(IDC_OVERVIEW_CATEGORY_COMBO, true);
		//note: the product combo filters on only products with "HasSerialNum" or "HasExpDate" currently set on them
		m_ProductCombo = BindNxDataList2Ctrl(IDC_OVERVIEW_PRODUCT_COMBO, true);
		m_SupplierCombo = BindNxDataList2Ctrl(IDC_OVERVIEW_SUPPLIER_COMBO, true);
		m_LocationCombo = BindNxDataList2Ctrl(IDC_OVERVIEW_LOCATION_COMBO, true);
		// (j.jones 2008-03-05 16:09) - PLID 29202 - added provider filter
		m_ProviderCombo = BindNxDataList2Ctrl(IDC_OVERVIEW_PROVIDER_COMBO, true);
		// (j.jones 2009-02-09 16:06) - PLID 32873 - added order filter
		m_OrderCombo = BindNxDataList2Ctrl(IDC_OVERVIEW_ORDER_COMBO, false);

		//build our order combo in code, only show received orders that have product items for active products
		m_OrderCombo->PutFromClause("(SELECT OrderT.ID, PersonT.Company, OrderT.Date, Max(OrderDetailsT.DateReceived) AS LastReceived, OrderT.Description, "
			"Sum(CASE WHEN OrderDetailsT.UseUU = 1 THEN Convert(int,(OrderDetailsT.QuantityOrdered / Convert(float,OrderDetailsT.Conversion))) ELSE OrderDetailsT.QuantityOrdered END) AS TotalQuantityOrdered "
			"FROM OrderT "
			"INNER JOIN OrderDetailsT ON OrderT.ID = OrderDetailsT.OrderID "
			"INNER JOIN ServiceT ON OrderDetailsT.ProductID = ServiceT.ID "
			"LEFT JOIN SupplierT ON OrderT.Supplier = SupplierT.PersonID "
			"LEFT JOIN PersonT ON SupplierT.PersonID = PersonT.ID "
			"WHERE OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0 "
			"AND OrderDetailsT.DateReceived Is Not Null "
			"AND ServiceT.Active = 1 "
			"AND OrderDetailsT.ID IN (SELECT OrderDetailID FROM ProductItemsT WHERE ReturnedFrom Is Null) "
			"GROUP BY OrderT.ID, PersonT.Company, PersonT.ID, OrderT.Date, OrderT.Description) AS OrdersQ");
		m_OrderCombo->Requery();

		//add the "all orders" row
		IRowSettingsPtr pRow = m_OrderCombo->GetNewRow();
		pRow->PutValue(occID, (long)-1);
		pRow->PutValue(occDate, g_cvarNull);
		pRow->PutValue(occFirstReceived, g_cvarNull);
		pRow->PutValue(occCompany, g_cvarNull);
		pRow->PutValue(occDescription, " <All Orders>");
		pRow->PutValue(occTotalQuantity, g_cvarNull);
		m_OrderCombo->AddRowSorted(pRow, NULL);
		m_OrderCombo->SetSelByColumn(occID, (long)-1);

		//TES 9/3/2008 - PLID 31237 - Added options for which date to filter on.
		m_pDateFilterTypes = BindNxDataList2Ctrl(IDC_DATE_FILTER_TYPES, false);

		//build the type combo
		// (j.jones 2009-03-18 17:53) - PLID 33579 - split consignment into three filter options
		pRow = m_TypeCombo->GetNewRow();
		pRow->PutValue(tccID, (long)optAll);
		pRow->PutValue(tccName, _bstr_t(" <All Product Types>"));
		m_TypeCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_TypeCombo->GetNewRow();
		pRow->PutValue(tccID, (long)optPurchased);
		pRow->PutValue(tccName, _bstr_t("Purchased Inventory"));
		m_TypeCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_TypeCombo->GetNewRow();		
		pRow->PutValue(tccID, (long)optConsignmentAll);
		pRow->PutValue(tccName, _bstr_t("Consignment - All"));
		m_TypeCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_TypeCombo->GetNewRow();		
		pRow->PutValue(tccID, (long)optConsignmentPaid);
		pRow->PutValue(tccName, _bstr_t("Consignment - Paid"));
		m_TypeCombo->AddRowAtEnd(pRow, NULL);
		pRow = m_TypeCombo->GetNewRow();		
		pRow->PutValue(tccID, (long)optConsignmentUnpaid);
		pRow->PutValue(tccName, _bstr_t("Consignment - Unpaid"));
		m_TypeCombo->AddRowAtEnd(pRow, NULL);
		m_TypeCombo->SetSelByColumn(tccID, (long)optAll);

		// (j.jones 2009-01-12 17:31) - PLID 32703 - BuildStatusCombo is called
		// in Refresh() prior to refiltering the list
		//BuildStatusCombo();

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

		// (j.jones 2008-03-05 16:11) - PLID 29202 - added a provider filter
		pRow = m_ProviderCombo->GetNewRow();
		pRow->PutValue(provccID, (long)-1);
		pRow->PutValue(provccName, _bstr_t(" <All Providers>"));
		m_ProviderCombo->AddRowSorted(pRow, NULL);
		m_ProviderCombo->SetSelByColumn(provccID, (long)-1);

		//TES 9/3/2008 - PLID 31237 - Build the list of date filter options, must match the date filter options in reportinfocallback
		// for the Inventory Overview report (618).
		pRow = m_pDateFilterTypes->GetNewRow();
		pRow->PutValue(dftcID, (long)1);
		pRow->PutValue(dftcName, _bstr_t("Status Date"));
		m_pDateFilterTypes->AddRowAtEnd(pRow, NULL);
		pRow = m_pDateFilterTypes->GetNewRow();
		pRow->PutValue(dftcID, (long)2);
		pRow->PutValue(dftcName, _bstr_t("Date Received"));
		m_pDateFilterTypes->AddRowAtEnd(pRow, NULL);
		pRow = m_pDateFilterTypes->GetNewRow();
		pRow->PutValue(dftcID, (long)3);
		pRow->PutValue(dftcName, _bstr_t("Date Ordered"));
		m_pDateFilterTypes->AddRowAtEnd(pRow, NULL);
		m_pDateFilterTypes->CurSel = m_pDateFilterTypes->GetFirstRow();

		// (j.jones 2009-01-12 17:35) - PLID 32703 - moved the From Clause generation to its own function,
		// which is called in Refresh()

		//we default to showing all products, all statuses, all dates,
		//most of these are handled by the SetSelByColumn calls earlier up in code
		m_radioAllDates.SetCheck(TRUE);

		//but also default the date controls to the past 30 days
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan dtSpan30;
		dtSpan30.SetDateTimeSpan(30, 0, 0, 0);
		// (a.walling 2008-05-13 15:52) - PLID 27591 - variants no longer necessary for datetimepicker
		m_DateTo.SetValue((dtNow));
		dtNow = dtNow - dtSpan30;
		m_DateFrom.SetValue((dtNow));

		//the list will filter & requery in Refresh()

	}NxCatchAll("Error in CInvOverviewDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInvOverviewDlg::Refresh()
{
	try {

		// (j.jones 2009-01-12 17:30) - PLID 32703 - the Color preference may have changed, so we need to
		//rebuild the status combo - this function will re-select the previous selection if one exists
		BuildStatusCombo();

		// (j.jones 2009-01-12 17:35) - PLID 32703 - for the Color preference, the From clause
		// needs to be rebuilt upon refresh, so I did so in its own function
		BuildFromClause();

		BOOL bProductCheckerChanged = FALSE;

		if(m_ProductChecker.Changed()) {

			//track this, because the category list must also refresh if the product changed
			bProductCheckerChanged = TRUE;

			//requery, but try to reselect the current row

			IRowSettingsPtr pRow = m_ProductCombo->GetCurSel();

			_variant_t varOld = g_cvarNull;

			if(pRow) {
				varOld = pRow->GetValue(pccID);
			}

			//note: the product combo filters on only products with "HasSerialNum" or "HasExpDate" currently set on them
			m_ProductCombo->Requery();

			//add the "all products" row
			pRow = m_ProductCombo->GetNewRow();
			pRow->PutValue(pccID, (long)-1);
			pRow->PutValue(pccCategory, g_cvarNull);
			pRow->PutValue(pccSupplier, g_cvarNull);
			pRow->PutValue(pccProductName, _bstr_t(" <All Products>"));
			pRow->PutValue(pccPrice, g_cvarNull);
			pRow->PutValue(pccBarcode, g_cvarNull);
			m_ProductCombo->AddRowSorted(pRow, NULL);

			//now set the selection

			pRow = NULL;
			if(varOld.vt != VT_NULL) {
				pRow = m_ProductCombo->SetSelByColumn(pccID, varOld);
			}

			if(pRow == NULL) {
				//select the "all products" row
				m_ProductCombo->SetSelByColumn(pccID, (long)-1);
			}
		}

		//refresh the categories if products changed as well as categories
		if(m_CategoryChecker.Changed() || bProductCheckerChanged) {

			//requery, but try to reselect the current row

			IRowSettingsPtr pRow = m_CategoryCombo->GetCurSel();

			_variant_t varOld = g_cvarNull;

			if(pRow) {
				varOld = pRow->GetValue(cccID);
			}

			//note: the category combo filters on only categories linked to at least one serialized product
			m_CategoryCombo->Requery();

			//add the "all categories" row
			pRow = m_CategoryCombo->GetNewRow();
			pRow->PutValue(cccID, (long)-1);
			pRow->PutValue(cccName, _bstr_t(" <All Categories>"));
			m_CategoryCombo->AddRowSorted(pRow, NULL);

			//now set the selection

			pRow = NULL;
			if(varOld.vt != VT_NULL) {
				pRow = m_CategoryCombo->SetSelByColumn(cccID, varOld);
			}

			if(pRow == NULL) {
				//select the "all categories" row
				m_CategoryCombo->SetSelByColumn(cccID, (long)-1);
			}
		}

		if(m_SupplierChecker.Changed()) {

			//requery, but try to reselect the current row

			IRowSettingsPtr pRow = m_SupplierCombo->GetCurSel();

			_variant_t varOld = g_cvarNull;

			if(pRow) {
				varOld = pRow->GetValue(sccID);
			}

			m_SupplierCombo->Requery();

			//add the "all suppliers" row
			pRow = m_SupplierCombo->GetNewRow();
			pRow->PutValue(sccID, (long)-1);
			pRow->PutValue(sccName, _bstr_t(" <All Suppliers>"));
			m_SupplierCombo->AddRowSorted(pRow, NULL);

			//now set the selection

			pRow = NULL;
			if(varOld.vt != VT_NULL) {
				pRow = m_SupplierCombo->SetSelByColumn(sccID, varOld);
			}

			if(pRow == NULL) {
				//select the "all suppliers" row
				m_SupplierCombo->SetSelByColumn(sccID, (long)-1);
			}
		}

		if(m_LocationChecker.Changed()) {

			//requery, but try to reselect the current row

			IRowSettingsPtr pRow = m_LocationCombo->GetCurSel();

			_variant_t varOld = g_cvarNull;

			if(pRow) {
				varOld = pRow->GetValue(lccID);
			}

			m_LocationCombo->Requery();

			//add the "all locations" row
			pRow = m_LocationCombo->GetNewRow();
			pRow->PutValue(lccID, (long)-1);
			pRow->PutValue(lccName, _bstr_t(" <All Locations>"));
			m_LocationCombo->AddRowSorted(pRow, NULL);

			//now set the selection

			pRow = NULL;
			if(varOld.vt != VT_NULL) {
				pRow = m_LocationCombo->SetSelByColumn(lccID, varOld);
			}

			if(pRow == NULL) {
				//select the "all locations" row
				m_LocationCombo->SetSelByColumn(lccID, (long)-1);
			}
		}

		// (j.jones 2008-03-05 16:12) - PLID 29202 - added provider filter
		if(m_ProviderChecker.Changed()) {

			//requery, but try to reselect the current row

			IRowSettingsPtr pRow = m_ProviderCombo->GetCurSel();

			_variant_t varOld = g_cvarNull;

			if(pRow) {
				varOld = pRow->GetValue(provccID);
			}

			m_ProviderCombo->Requery();

			//add the "all locations" row
			pRow = m_ProviderCombo->GetNewRow();
			pRow->PutValue(provccID, (long)-1);
			pRow->PutValue(provccName, _bstr_t(" <All Providers>"));
			m_ProviderCombo->AddRowSorted(pRow, NULL);

			//now set the selection

			pRow = NULL;
			if(varOld.vt != VT_NULL) {
				pRow = m_ProviderCombo->SetSelByColumn(provccID, varOld);
			}

			if(pRow == NULL) {
				//select the "all providers" row
				m_ProviderCombo->SetSelByColumn(provccID, (long)-1);
			}
		}

		// (j.jones 2009-02-10 08:50) - PLID 32873 - support the order tablechecker
		if(m_OrderChecker.Changed()) {

			//requery, but try to reselect the current row

			IRowSettingsPtr pRow = m_OrderCombo->GetCurSel();

			_variant_t varOld = g_cvarNull;

			if(pRow) {
				varOld = pRow->GetValue(occID);
			}

			m_OrderCombo->Requery();

			//add the "all orders" row
			IRowSettingsPtr pNewRow = m_OrderCombo->GetNewRow();
			pNewRow->PutValue(occID, (long)-1);
			pNewRow->PutValue(occDate, g_cvarNull);
			pNewRow->PutValue(occFirstReceived, g_cvarNull);
			pNewRow->PutValue(occCompany, g_cvarNull);
			pNewRow->PutValue(occDescription, " <All Orders>");
			pNewRow->PutValue(occTotalQuantity, g_cvarNull);
			m_OrderCombo->AddRowSorted(pNewRow, NULL);

			//now set the selection

			pRow = NULL;
			if(varOld.vt != VT_NULL) {
				pRow = m_OrderCombo->SetSelByColumn(occID, varOld);
			}

			if(pRow == NULL) {
				//select the "all orders" row
				m_OrderCombo->SetSelByColumn(occID, (long)-1);
			}
		}

		//reload the list
		ReFilterOverviewList();

	}NxCatchAll("Error in CInvOverviewDlg::Refresh");
}

//re-filters the overview list with the user's selections
void CInvOverviewDlg::ReFilterOverviewList()
{
	// (c.haag 2007-12-03 10:58) - PLID 28204 - Replacing references to "Consignment" with "ProductItemStatus"
	try {

		//build the where clause
		CString strWhere;

		//filter on product types
		CString strType;
		IRowSettingsPtr pRow = m_TypeCombo->GetCurSel();
		if(pRow != NULL) {
			OverviewProductType optType = (OverviewProductType)VarLong(pRow->GetValue(tccID), (long)optAll);
			// (j.jones 2009-03-18 17:53) - PLID 33579 - split consignment into three filter options
			if(optType == optConsignmentAll) {
				strType.Format("ProductItemStatus = %d", pisConsignment);
			}
			else if(optType == optConsignmentPaid) {
				strType.Format("ProductItemStatus = %d AND ConsignmentPaid = 1", pisConsignment);
			}
			else if(optType == optConsignmentUnpaid) {
				strType.Format("ProductItemStatus = %d AND ConsignmentPaid = 0", pisConsignment);
			}
			else if(optType == optPurchased) {
				strType.Format("ProductItemStatus = %d", pisPurchased);
			}
		}

		if(!strType.IsEmpty()) {
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strType;
		}

		//add our status filters
		CString strStatus;
		OverviewStatusType ostType = ostAll;
		pRow = m_StatusCombo->GetCurSel();
		if(pRow != NULL) {
			ostType = (OverviewStatusType)VarLong(pRow->GetValue(stccID), (long)ostAll);
			if(ostType != ostAll && ostType != ostOnHand) {
				strStatus.Format("StatusType = %li", ostType);
			}
			else if(ostType == ostOnHand) {
				//"on hand" means Available OR Allocated
				strStatus.Format("(StatusType = %li OR StatusType = %li)", ostAvailable, ostAllocated);
			}

			//also enable/disable the date controls
			if(ostType == ostAvailable) {
				//for "available", dates are meaningless
				/*m_radioAllDates.EnableWindow(FALSE);
				m_radioDateRange.EnableWindow(FALSE);
				m_DateFrom.EnableWindow(FALSE);
				m_DateTo.EnableWindow(FALSE);
				GetDlgItem(IDC_DATE_FILTER_TYPES)->EnableWindow(FALSE);*/

				//TES 9/3/2008 - PLID 31237 - Not true any more, it was only meaningless for the Status Date, so
				// disable just that option (at least make it look disabled, OnSelChanging will make sure it actually is.).
				IRowSettingsPtr pDateRow = m_pDateFilterTypes->CurSel;
				ASSERT(pDateRow != NULL);
				if(VarLong(pDateRow->GetValue(dftcID)) == 1) {//Status Date
					//TES 9/3/2008 - PLID 31237 - We're already filtered on it, so change to the next date option, and
					// reset to all dates.
					m_pDateFilterTypes->SetSelByColumn(dftcID, (long)2); //Date Received
					m_radioAllDates.SetCheck(BST_CHECKED);
					m_radioDateRange.SetCheck(BST_UNCHECKED);
				}
				pDateRow = m_pDateFilterTypes->FindByColumn(dftcID, (long)1, NULL, g_cvarFalse);
				ASSERT(pDateRow != NULL);
				pDateRow->BackColor = RGB(200,200,200);
				pDateRow->BackColorSel = RGB(200,200,200);
				pDateRow->ForeColor = RGB(0,0,0);
				pDateRow->ForeColorSel = RGB(0,0,0);
			}
			else {
				/*m_radioAllDates.EnableWindow(TRUE);
				m_radioDateRange.EnableWindow(TRUE);*/
			
				//TES 9/3/2008 - PLID 31237 - Re-enable the Status Date row.
				IRowSettingsPtr pDateRow = m_pDateFilterTypes->FindByColumn(dftcID, (long)1, NULL, g_cvarFalse);
				ASSERT(pDateRow != NULL);
				//TES 9/3/2008 - PLID 31237 - We want to reset this to the default values, so get a new row and use its colors.
				IRowSettingsPtr pNewRow = m_pDateFilterTypes->GetNewRow();
				pDateRow->BackColor = pNewRow->BackColor;
				pDateRow->BackColorSel = pNewRow->BackColorSel;
				pDateRow->ForeColor = pNewRow->ForeColor;
				pDateRow->ForeColorSel = pNewRow->ForeColorSel;
			}
			//TES 9/3/2008 - PLID 31237 - The date range now just needs to look at the radio button, not the status filter.
			m_DateFrom.EnableWindow(m_radioDateRange.GetCheck());
			m_DateTo.EnableWindow(m_radioDateRange.GetCheck());
			GetDlgItem(IDC_DATE_FILTER_TYPES)->EnableWindow(m_radioDateRange.GetCheck());

		}

		if(!strStatus.IsEmpty()) {
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strStatus;
		}

		//filter on products
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

		//filter on supplier
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

		//filter on location
		pRow = m_LocationCombo->GetCurSel();
		if(pRow != NULL) {
			long nLocationID = VarLong(pRow->GetValue(lccID), -1);
			if(nLocationID != -1) {
				CString strLocation;

				//Some product items can, in theory, be location-less, and a
				//location-less Product Item is considered to be "all locations".
				//However, we intentionally want this filter to only look at those
				//linked to a specific location.

				strLocation.Format("LocationID = %li", nLocationID);				
				if(!strWhere.IsEmpty()) {
					strWhere += " AND ";
				}
				strWhere += strLocation;
			}
		}

		// (j.jones 2008-03-05 16:16) - PLID 29202 - filter on provider only
		// if we are filtering on Allocated or Used
		if(ostType == ostAllocated || ostType == ostUsed) {
			//enable the provider filter
			m_ProviderCombo->Enabled = VARIANT_TRUE;

			//and now actually filter by it
			pRow = m_ProviderCombo->GetCurSel();
			if(pRow != NULL) {
				long nProviderID = VarLong(pRow->GetValue(provccID), -1);
				if(nProviderID != -1) {
					CString strProvider;

					//Some allocations might not have providers,
					//same with charges, but in this filter we will
					//only find those records that have providers

					strProvider.Format("ProviderID = %li", nProviderID);				
					if(!strWhere.IsEmpty()) {
						strWhere += " AND ";
					}
					strWhere += strProvider;
				}
			}
		}
		else {
			//disable the provider filter
			m_ProviderCombo->Enabled = VARIANT_FALSE;
		}

		//filter on category
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

		// (j.jones 2009-02-09 17:45) - PLID 32873 - added order filter
		pRow = m_OrderCombo->GetCurSel();
		if(pRow != NULL) {
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

		//filter on dates, but don't bother if we are filtering on only Available items
		//TES 9/3/2008 - PLID 31237 - Actually, go ahead and bother no matter what status we're filtering on.
		if(m_radioDateRange.GetCheck()) {
			CString strDates;

			//TES 9/3/2008 - PLID 31237 - Figure out which date they're filtering on.
			IRowSettingsPtr pDateRow = m_pDateFilterTypes->CurSel;
			ASSERT(pDateRow != NULL); //We set this in OnInitDialog(), and use OnSelChanging() to keep it from being unset.
			long nDateFilter = VarLong(pDateRow->GetValue(dftcID));
			CString strDate;
			if(nDateFilter == 1) {
				//Status Date
				strDate = "DateUsed";
			}
			else if(nDateFilter == 2) {
				//Date Received
				strDate = "DateReceived";
			}
			else if(nDateFilter == 3) {
				//Date Ordered
				strDate = "DateOrdered";
			}
			else {
				//Unknown date filter option!
				ASSERT(FALSE);
			}


			//We're not going to yell at them if the "to" date is before the "for" date,
			//instead just show them that there won't be any results.

			//Also, we are specifically filtering out null dates here. When a
			//date filter is in use, we're only going to show products that
			//have a DateUsed.

			CString strFromDate, strToDate;
			// (a.walling 2008-05-13 15:52) - PLID 27591 - VarDateTime no longer necessary
			strFromDate = FormatDateTimeForSql((m_DateFrom.GetValue()), dtoDate);
			strToDate = FormatDateTimeForSql((m_DateTo.GetValue()), dtoDate);
			strDates.Format("(%s Is Not Null AND (%s >= '%s' AND %s < DATEADD(day, 1, '%s')))", strDate, strDate, strFromDate, strDate, strToDate);				
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strDates;
		}

		m_OverviewList->PutWhereClause(_bstr_t(strWhere));

		//set the label to say "Loading..."
		SetDlgItemText(IDC_TOTAL_OVERVIEW_ROWS_LABEL, "Matching Products:  Loading...");

		//now requery the list
		m_OverviewList->Requery();

	}NxCatchAll("Error in CInvOverviewDlg::ReFilterOverviewList");
}


BEGIN_EVENTSINK_MAP(CInvOverviewDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvOverviewDlg)
	ON_EVENT(CInvOverviewDlg, IDC_OVERVIEW_PRODUCT_COMBO, 16 /* SelChosen */, OnSelChosenOverviewProductCombo, VTS_DISPATCH)
	ON_EVENT(CInvOverviewDlg, IDC_OVERVIEW_SUPPLIER_COMBO, 16 /* SelChosen */, OnSelChosenOverviewSupplierCombo, VTS_DISPATCH)
	ON_EVENT(CInvOverviewDlg, IDC_OVERVIEW_LOCATION_COMBO, 16 /* SelChosen */, OnSelChosenOverviewLocationCombo, VTS_DISPATCH)
	ON_EVENT(CInvOverviewDlg, IDC_OVERVIEW_LIST, 18 /* RequeryFinished */, OnRequeryFinishedOverviewList, VTS_I2)
	ON_EVENT(CInvOverviewDlg, IDC_OVERVIEW_CATEGORY_COMBO, 16 /* SelChosen */, OnSelChosenOverviewCategoryCombo, VTS_DISPATCH)
	ON_EVENT(CInvOverviewDlg, IDC_OVERVIEW_PRODUCT_TYPE_COMBO, 16 /* SelChosen */, OnSelChosenOverviewProductTypeCombo, VTS_DISPATCH)
	ON_EVENT(CInvOverviewDlg, IDC_OVERVIEW_STATUS_COMBO, 16 /* SelChosen */, OnSelChosenOverviewStatusCombo, VTS_DISPATCH)
	ON_EVENT(CInvOverviewDlg, IDC_OVERVIEW_PROVIDER_COMBO, 16 /* SelChosen */, OnSelChosenOverviewProviderCombo, VTS_DISPATCH)
	ON_EVENT(CInvOverviewDlg, IDC_DATE_FILTER_TYPES, 1 /* SelChanging */, OnSelChangingDateFilterTypes, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvOverviewDlg, IDC_DATE_FILTER_TYPES, 16 /* SelChosen */, OnSelChosenDateFilterTypes, VTS_DISPATCH)
	ON_EVENT(CInvOverviewDlg, IDC_OVERVIEW_ORDER_COMBO, 16, OnSelChosenOverviewOrderCombo, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP	
END_EVENTSINK_MAP()

void CInvOverviewDlg::OnSelChosenOverviewProductCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//select the "all products" row (we don't have to confirm that it succeeds)
			m_ProductCombo->SetSelByColumn(pccID, (long)-1);
		}
		
		ReFilterOverviewList();

	}NxCatchAll("Error in CInvOverviewDlg::OnSelChosenOverviewProductCombo");
}

void CInvOverviewDlg::OnSelChosenOverviewSupplierCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//select the "all suppliers" row (we don't have to confirm that it succeeds)
			m_SupplierCombo->SetSelByColumn(sccID, (long)-1);
		}

		//this function doesn't try to filter the product dropdown, only the overview list

		ReFilterOverviewList();

	}NxCatchAll("Error in CInvOverviewDlg::OnSelChosenOverviewSupplierCombo");
}

void CInvOverviewDlg::OnSelChosenOverviewLocationCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//select the "all locations" row (we don't have to confirm that it succeeds)
			m_LocationCombo->SetSelByColumn(lccID, (long)-1);
		}

		//this function doesn't try to filter the product dropdown, only the overview list

		ReFilterOverviewList();

	}NxCatchAll("Error in CInvOverviewDlg::OnSelChosenOverviewLocationCombo");
}

HBRUSH CInvOverviewDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	try {
	
		if (nCtlColor == CTLCOLOR_STATIC)
		{
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x00FFDBDB));
			return m_brush;
		}
		
	}NxCatchAll("Error in CInvOverviewDlg::OnCtlColor");

	return hbr;
	*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CInvOverviewDlg::OnRadioAllOverviewDates() 
{
	//this will handle enabling/disabling the date controls
	ReFilterOverviewList();
}

void CInvOverviewDlg::OnRadioOverviewDateRange()
{
	//this will handle enabling/disabling the date controls
	ReFilterOverviewList();
}

void CInvOverviewDlg::OnChangeDtOverviewFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ReFilterOverviewList();

	*pResult = 0;
}

void CInvOverviewDlg::OnChangeDtOverviewTo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	ReFilterOverviewList();
	
	*pResult = 0;
}

void CInvOverviewDlg::OnRequeryFinishedOverviewList(short nFlags) 
{
	try {

		//change the label to reflect the count of rows

		long nCount = m_OverviewList->GetRowCount();

		CString str;
		str.Format("Matching Products:  %li", nCount);
		SetDlgItemText(IDC_TOTAL_OVERVIEW_ROWS_LABEL, str);

		// (j.jones 2008-03-18 17:53) - PLID 29308 - band-aid fix for a dl2 sizing issue,
		// this will ensure the horizontal scrollbar is properly calculated
		m_OverviewList->CalcColumnWidthFromData(olcProviderName, TRUE, TRUE);

	}NxCatchAll("Error in CInvOverviewDlg::OnRequeryFinishedOverviewList");
}

//DRT 11/16/2007 - PLID 27990 - Used by the view to get the filters when previewing the report
// (j.jones 2008-03-05 17:13) - PLID 29202 - added provider filters
//TES 9/3/2008 - PLID 31237 - Added parameters for which date they're filtering on, left unchanged if bUseDateFilter is false.
// (j.jones 2009-02-09 16:05) - PLID 32873 - added OrderID filter
void CInvOverviewDlg::GetCurrentFilters(long &nProductID, long &nSupplierID, long &nLocationID, long &nType, long &nProviderID,
										   bool &bUseDateFilter, short &nDateFilter, CString &strDateFilter, COleDateTime &dtFrom, 
										   COleDateTime &dtTo, long &nCategoryID, long &nProductType, long &nOrderID)
{

	//Status
	nType = (long)ostAll;

	IRowSettingsPtr pRow = m_StatusCombo->GetCurSel();
	if(pRow != NULL) {
		nType = VarLong(pRow->GetValue(stccID), (long)ostAll);
	}

	//filter on products
	nProductID = -1;
	pRow = m_ProductCombo->GetCurSel();
	if(pRow != NULL) {
		nProductID = VarLong(pRow->GetValue(pccID), -1);
	}

	//filter on supplier
	nSupplierID = -1;
	pRow = m_SupplierCombo->GetCurSel();
	if(pRow != NULL) {
		nSupplierID = VarLong(pRow->GetValue(sccID), -1);
	}

	//filter on location
	nLocationID = -1;
	pRow = m_LocationCombo->GetCurSel();
	if(pRow != NULL) {
		nLocationID = VarLong(pRow->GetValue(lccID), -1);
	}

	//filter on dates, but don't bother if we are filtering on only Available items
	bUseDateFilter = false;
	//TES 9/3/2008 - PLID 31237 - Actually, go ahead and bother no matter what status we're filtering on.
	if(m_radioDateRange.GetCheck()) {
		bUseDateFilter = true;

		//TES 9/3/2008 - PLID 31237 - There are now options as to which date to filter on.
		IRowSettingsPtr pRow = m_pDateFilterTypes->CurSel;
		ASSERT(pRow != NULL); //We set in OnInitDialog(), and use OnSelChanging() to keep it from being unset.
		nDateFilter = (short)VarLong(pRow->GetValue(dftcID));
		strDateFilter = VarString(pRow->GetValue(dftcName));


		// (a.walling 2008-05-13 15:52) - PLID 27591 - VarDateTime no longer necessary
		dtFrom = (m_DateFrom.GetValue());
		dtTo = (m_DateTo.GetValue());
	}

	// (j.jones 2008-03-05 17:13) - PLID 29202 - added provider filters,
	// only used if filtering on allocated or used
	nProviderID = -1;
	if(nType == (long)ostAllocated || nType == (long)ostUsed) {
		pRow = m_ProviderCombo->GetCurSel();
		if(pRow != NULL) {
			nProviderID = VarLong(pRow->GetValue(provccID), -1);
		}
	}

	//filter on category
	nCategoryID = -1;
	pRow = m_CategoryCombo->GetCurSel();
	if(pRow != NULL) {
		nCategoryID = VarLong(pRow->GetValue(cccID), -1);
	}

	//filter on product type
	nProductType = (long)optAll;

	pRow = m_TypeCombo->GetCurSel();
	if(pRow != NULL) {
		nProductType = VarLong(pRow->GetValue(tccID), (long)optAll);
	}

	// (j.jones 2009-02-10 08:49) - PLID 32873 - supported order filter
	pRow = m_OrderCombo->GetCurSel();
	if(pRow != NULL) {
		nOrderID = VarLong(pRow->GetValue(occID), -1);
	}
}

void CInvOverviewDlg::OnSelChosenOverviewCategoryCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//select the "all categories" row (we don't have to confirm that it succeeds)
			m_CategoryCombo->SetSelByColumn(cccID, (long)-1);
		}
		
		ReFilterOverviewList();

	}NxCatchAll("Error in CInvOverviewDlg::OnSelChosenOverviewCategoryCombo");
}

void CInvOverviewDlg::OnSelChosenOverviewProductTypeCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//select the "all types" row (we don't have to confirm that it succeeds)
			m_TypeCombo->SetSelByColumn(tccID, (long)optAll);
		}
		
		ReFilterOverviewList();

	}NxCatchAll("Error in CInvOverviewDlg::OnSelChosenOverviewProductTypeCombo");
}

void CInvOverviewDlg::OnSelChosenOverviewStatusCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//select the "all statii" row (we don't have to confirm that it succeeds)
			m_StatusCombo->SetSelByColumn(stccID, (long)ostAll);
		}
		
		ReFilterOverviewList();

	}NxCatchAll("Error in CInvOverviewDlg::OnSelChosenOverviewStatusCombo");
}

// (j.jones 2008-03-05 16:57) - PLID 29202 - added provider filter
void CInvOverviewDlg::OnSelChosenOverviewProviderCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//select the "all providers" row (we don't have to confirm that it succeeds)
			m_ProviderCombo->SetSelByColumn(provccID, (long)-1);
		}

		ReFilterOverviewList();

	}NxCatchAll("Error in CInvOverviewDlg::OnSelChosenOverviewProviderCombo");
}

void CInvOverviewDlg::OnSelChangingDateFilterTypes(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		//TES 9/3/2008 - PLID 31237 - Don't let them select NULL.
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
		else {
			IRowSettingsPtr pRow(*lppNewSel);
			if(VarLong(pRow->GetValue(dftcID)) == 1) { //Status Date
				//TES 9/3/2008 - PLID 31237 - They're trying to select Status Date.  If we're currently filtering on the status
				// "Available", that filter is disabled.
				IRowSettingsPtr pStatusRow = m_StatusCombo->GetCurSel();
				if(pStatusRow != NULL) {
					OverviewStatusType ostType = (OverviewStatusType)VarLong(pStatusRow->GetValue(stccID), (long)ostAll);
					if(ostType == ostAvailable) {
						//TES 9/3/2008 - PLID 31237 - We are indeed filtering on the status "Available"  Deny!
						SafeSetCOMPointer(lppNewSel, lpOldSel);
					}
				}
			}
		}

	}NxCatchAll("Error in CInvOverviewDlg::OnSelChangingDateFilterTypes()");
}

void CInvOverviewDlg::OnSelChosenDateFilterTypes(LPDISPATCH lpRow) 
{
	try {
		//TES 9/3/2008 - PLID 31237 - Requery the list with the new filter option
		ReFilterOverviewList();
	}NxCatchAll("Error in CInvOverviewDlg::OnSelChosenDateFilterTypes()");
}


// (j.jones 2009-01-12 17:10) - PLID 32703 - added function to build the status combo
void CInvOverviewDlg::BuildStatusCombo()
{
	try {

		//store the previous value if there was one
		_variant_t varOld = g_cvarNull;
		{
			IRowSettingsPtr pOldRow = m_StatusCombo->GetCurSel();
			if(pOldRow) {
				varOld = pOldRow->GetValue(stccID);
			}
		}

		m_StatusCombo->Clear();

		// (j.jones 2009-01-12 17:14) - PLID 32703 - check the coloring preference
		BOOL bColorRows = GetRemotePropertyInt("InvOverviewColorRows", 1, 0, GetCurrentUserName(), true);

		IRowSettingsPtr pRow = m_StatusCombo->GetNewRow();
		pRow->PutValue(stccID, (long)ostAll);
		pRow->PutValue(stccName, _bstr_t(ostAll_Text));
		//if colored, this row is black
		m_StatusCombo->AddRowAtEnd(pRow, NULL);

		pRow = m_StatusCombo->GetNewRow();
		pRow->PutValue(stccID, (long)ostAvailable);
		pRow->PutValue(stccName, _bstr_t(ostAvailable_Text));
		if(bColorRows) {
			pRow->PutForeColor(COLOR_AVAILABLE);
		}
		m_StatusCombo->AddRowAtEnd(pRow, NULL);

		pRow = m_StatusCombo->GetNewRow();		
		pRow->PutValue(stccID, (long)ostOnHand);
		pRow->PutValue(stccName, _bstr_t(ostOnHand_Text));
		//if colored, this row is black
		m_StatusCombo->AddRowAtEnd(pRow, NULL);

		pRow = m_StatusCombo->GetNewRow();		
		pRow->PutValue(stccID, (long)ostAllocated);
		pRow->PutValue(stccName, _bstr_t(ostAllocated_Text));
		if(bColorRows) {
			pRow->PutForeColor(COLOR_ALLOCATED);
		}
		m_StatusCombo->AddRowAtEnd(pRow, NULL);

		pRow = m_StatusCombo->GetNewRow();
		pRow->PutValue(stccID, (long)ostUsed);
		pRow->PutValue(stccName, _bstr_t(ostUsed_Text));
		if(bColorRows) {
			pRow->PutForeColor(COLOR_USED);
		}
		m_StatusCombo->AddRowAtEnd(pRow, NULL);

		pRow = m_StatusCombo->GetNewRow();
		pRow->PutValue(stccID, (long)ostAdjusted);		
		pRow->PutValue(stccName, _bstr_t(ostAdjusted_Text));
		if(bColorRows) {
			pRow->PutForeColor(COLOR_ADJUSTED);
		}
		m_StatusCombo->AddRowAtEnd(pRow, NULL);

		if(varOld.vt != VT_NULL) {
			m_StatusCombo->SetSelByColumn(stccID, varOld);
		}

		IRowSettingsPtr pNewRow = m_StatusCombo->GetCurSel();
		if(pNewRow == NULL) {
			m_StatusCombo->SetSelByColumn(stccID, (long)ostAll);
		}

	}NxCatchAll("Error in CInvOverviewDlg::BuildStatusCombo");
}

// (j.jones 2009-01-12 17:34) - PLID 32703 - added function to build the from clause
void CInvOverviewDlg::BuildFromClause()
{
	try {

		// (j.jones 2009-01-12 16:56) - PLID 32703 - added the Color column, which is based on a preference,
		// and black otherwise
		CString strColor;
		BOOL bColorRows = GetRemotePropertyInt("InvOverviewColorRows", 1, 0, GetCurrentUserName(), true);
		if(bColorRows) {
			//color the row based on the StatusType, which means we need to clone the StatusType logic here
			strColor.Format("CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN %li "
				"WHEN ProductItemsT.Deleted = 1 THEN %li "
				"WHEN LineItemT.ID Is Not Null THEN %li "
				"WHEN CaseHistoryDetailsT.ID Is Not Null THEN %li "
				"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
				"	(CASE WHEN AllocatedItemsQ.Status = %li THEN %li ELSE %li END) "
				"ELSE %li END ",
				COLOR_ADJUSTED,
				COLOR_ADJUSTED,
				COLOR_USED,
				COLOR_USED,
				InvUtils::iadsUsed, COLOR_USED, COLOR_ALLOCATED,
				COLOR_AVAILABLE);
		}
		else {
			strColor.Format("%li", COLOR_BLACK);
		}

		// (j.jones 2007-11-09 10:41) - PLID 27989 - placed the from clause in code
		// for easier reading & editability, and also to use the allocation and consignment statii

		//DRT 11/16/2007 - PLID 27990 - I pretty much just copied this query (and made a few minor tweaks in adding fields) to the reports
		//	module for the preview report.  If you change this query, you should go change that code as well.
		// (j.jones 2007-11-29 10:35) - PLID 28196 - accounted for completed allocations as "used",
		// though the status is superceded if also billed or on a case history

		// (c.haag 2007-12-03 10:58) - PLID 28204 - Replacing references to "consignment" with "status"

		// (j.jones 2008-01-16 16:56) - PLID 28640 - added support for allocation notes

		// (j.jones 2008-03-05 16:39) - PLID 29202 - added ProviderID and name, only a part of allocations,
		// charges, and case histories

		//TES 5/27/2008 - PLID 29459 - Updated to use the linked appointment date (if any) as the "Status Date"

		// (j.jones 2008-06-02 16:16) - PLID 28076 - adjusted products now use the adjustment date,
		// but it can be null if it is old data prior to when we started tracking adjustment IDs

		// (j.jones 2008-06-06 10:07) - PLID 27110 - If a product is returned from a bill, we use a dummy product
		// as a placeholder on that bill, which references the original product with ProductItemsT.ReturnedFrom.
		// This list needs to ignore all ProductItems with a ReturnedFrom value.

		//TES 6/18/2008 - PLID 29578 - Updated (and simplified) the query now that ProductItemsT has an OrderDetailID instead
		// of an OrderID
		//TES 6/25/2008 - PLID 26142 - Added the Adjustment category (if any).
		//TES 9/3/2008 - PLID 29779 - Added DateOrdered
		// (j.jones 2009-01-12 17:05) - PLID 32703 - added Color
		// (j.jones 2009-02-09 12:56) - PLID 32775 - renamed 'Charged' to 'Billed'
		// (j.jones 2009-02-09 17:49) - PLID 32873 - added OrderDetailID
		// (j.jones 2009-03-09 12:54) - PLID 33096 - product items created from adjustments can now use the adjustment date
		// as the date received, but order date will remain null
		// (j.jones 2009-03-19 08:49) - PLID 33579 - added consignment paid information
		CString strFrom;
		strFrom.Format("(SELECT ProductItemsT.ID, ProductItemsT.ProductID, ServiceT.Name AS ProductName, "
			"%s AS Color, "
			"LocationsT.ID AS LocationID, Coalesce(LocationsT.Name, '<No Location>') AS LocName, "
			"ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
			"ProductItemsT.Status AS ProductItemStatus, ServiceT.Category, "
			"ProductItemsT.OrderDetailID, "
			"CASE WHEN ProductItemsT.Status = %li THEN ProductItemsT.Paid ELSE NULL END AS ConsignmentPaid, "
			"CASE WHEN ProductItemsT.Status = %li THEN ProductItemsT.PaidDate ELSE NULL END AS ConsignmentPaidDate, "
			"CASE WHEN ProductItemsT.Status = %d THEN 'Consignment' ELSE CASE WHEN ProductItemsT.Status = %d THEN 'Warranty' ELSE 'Purchased Inv.' END END AS ProductType, "
			"CASE WHEN CreatingProductAdjustmentsT.Date Is Not Null THEN CreatingProductAdjustmentsT.Date ELSE OrderDetailsT.DateReceived END AS DateReceived, "
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
			"OrderT.Date AS DateOrdered "			
			"FROM ProductItemsT "
			"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
			"LEFT JOIN SupplierReturnItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
			"LEFT JOIN SupplierReturnGroupsT ON SupplierReturnItemsT.ReturnGroupID = SupplierReturnGroupsT.ID "
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
			"LEFT JOIN OrderDetailsT ON ProductItemsT.OrderDetailID = OrderDetailsT.ID "
			"LEFT JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
			"LEFT JOIN ProductAdjustmentsT CreatingProductAdjustmentsT ON ProductItemsT.CreatingAdjustmentID = CreatingProductAdjustmentsT.ID "
			"WHERE ProductItemsT.ReturnedFrom Is Null AND ServiceT.Active = 1) AS ProductItemsQ",
			strColor, pisConsignment, pisConsignment, pisConsignment, pisWarranty, ostAdjusted, ostAdjusted, ostUsed, ostUsed, InvUtils::iadsUsed, ostUsed, ostAllocated, ostAvailable,
			InvUtils::iadsUsed, InvUtils::iadsUsed,	InvUtils::iadsActive, InvUtils::iasActive, InvUtils::iadsUsed, InvUtils::iasCompleted);

		m_OverviewList->PutFromClause(_bstr_t(strFrom));

	}NxCatchAll("Error in CInvOverviewDlg::BuildFromClause");
}

// (j.jones 2009-02-09 16:28) - PLID 32873 - added overview filter
void CInvOverviewDlg::OnSelChosenOverviewOrderCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//select the "all orders" row (we don't have to confirm that it succeeds)
			m_OrderCombo->SetSelByColumn(occID, (long)-1);
		}
		
		ReFilterOverviewList();

	}NxCatchAll("Error in CInvOverviewDlg::OnSelChosenOverviewOrderCombo");
}

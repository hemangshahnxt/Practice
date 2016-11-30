// InvSerialTrackerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "inventoryrc.h"
#include "InvSerialTrackerDlg.h"
#include "InvUtils.h"
#include "DateTimeUtils.h"
#include "InvEditDlg.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;

/////////////////////////////////////////////////////////////////////////////
// CInvSerialTrackerDlg dialog

enum SerialListColumn {

	slcID = 0,
	slcProductName,
	slcSerialNumber,
	slcExpDate,
	slcType,
	slcStatus,
	slcBilledTo,
	slcChargeDate,
	slcProductID,
	slcNotes,	// (j.jones 2008-01-16 16:42) - PLID 28640
};

//(e.lally 2008-01-30) PLID 28627 - Added column enums for all filters
enum SerialFilterColumn {
	sfcID =0,
	sfcPrimarySort, //(e.lally 2008-01-30) PLID 28627 - Added primary sort
};

enum PatientFilterColumn {

	pafcID = 0,
	pafcName,
	pafcUserDefId,
	pafcBirthdate,
	pafcSocSec,
	pafcPrimarySort, //(e.lally 2008-01-30) PLID 28627 - Added primary sort
	
};

enum ProductFilterColumn {

	prfcID = 0,
	prfcProduct,
	prfcPrimarySort, //(e.lally 2008-01-30) PLID 28627 - Added primary sort
	
};

enum CategoryFilterColumn {

	cfcID = 0,
	cfcCategory,
	cfcPrimarySort, //(e.lally 2008-01-30) PLID 28627 - Added primary sort
	
};

enum StatusFilterColumn {

	stfcID = 0,
	stfcType,
	
};


CInvSerialTrackerDlg::CInvSerialTrackerDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvSerialTrackerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInvSerialTrackerDlg)
	m_dtEnd = COleDateTime::GetCurrentTime();
	m_dtStart = COleDateTime::GetCurrentTime();
	m_bFilterOnDates = FALSE;
	m_bFilterOnConsignment = FALSE;
	m_nDefaultPatientID = -1;
	m_bFilterOnConsignment = FALSE;
	m_vtNull.vt = VT_NULL;
	//}}AFX_DATA_INIT
}


void CInvSerialTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvSerialTrackerDlg)
	DDX_Control(pDX, IDC_CHECK_FILTER_CONSIGNMENT, m_checkConsignment);
	DDX_Control(pDX, IDC_CHECK_FILTERONCHGDATES, m_btnFilterDate);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_DateTimeCtrl(pDX, IDC_ENDDATE, m_dtEnd);
	DDX_DateTimeCtrl(pDX, IDC_STARTDATE, m_dtStart);
	DDX_Check(pDX, IDC_CHECK_FILTERONDATES, m_bFilterOnDates);
	DDX_Check(pDX, IDC_CHECK_FILTER_CONSIGNMENT, m_bFilterOnConsignment);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvSerialTrackerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvSerialTrackerDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_STARTCHARGEDATE, OnDatetimechangeStartdate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_ENDCHARGEDATE, OnDatetimechangeEnddate)
	ON_BN_CLICKED(IDC_CHECK_FILTERONDATES, OnCheckFilterOnDates)
	ON_BN_CLICKED(IDC_CHECK_FILTER_CONSIGNMENT, OnCheckFilterConsignment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvSerialTrackerDlg message handlers

BOOL CInvSerialTrackerDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (c.haag 2008-04-29 11:52) - PLID 29820 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
	
		m_CategoryCombo = BindNxDataListCtrl(this,IDC_CATEGORY_LIST,GetRemoteData(),true);
		m_ProductCombo = BindNxDataListCtrl(this,IDC_ITEM,GetRemoteData(),false);
		m_PatientCombo = BindNxDataListCtrl(this,IDC_PATIENT_COMBO,GetRemoteData(),true);
		m_SerialCombo = BindNxDataListCtrl(this,IDC_SERIAL_COMBO,GetRemoteData(),true);
		m_SerialList = BindNxDataListCtrl(this,IDC_LIST_SERIALIZED,GetRemoteData(),false);
		m_StatusCombo = BindNxDataListCtrl(this, IDC_TRACKER_STATUS_LIST, GetRemoteData(), false);

		//DRT 11/28/2007 - PLID 28215 - Build the status combo
		// (j.jones 2007-11-30 13:09) - PLID 27989 - added the On Hand filter
		//(e.lally 2008-01-30) PLID 28627 - Updated to use column enums
		IRowSettingsPtr pRow = NULL;
		pRow = m_StatusCombo->GetRow(sriGetNewRow);
		pRow->PutValue(stfcID, (long)InvUtils::ostAll);
		pRow->PutValue(stfcType, _bstr_t(InvUtils::ostAll_Text));
		m_StatusCombo->AddRow(pRow);
		pRow = m_StatusCombo->GetRow(sriGetNewRow);
		pRow->PutValue(stfcID, (long)InvUtils::ostAvailable);
		pRow->PutValue(stfcType, _bstr_t(InvUtils::ostAvailable_Text));
		m_StatusCombo->AddRow(pRow);
		pRow = m_StatusCombo->GetRow(sriGetNewRow);
		pRow->PutValue(stfcID, (long)InvUtils::ostOnHand);
		pRow->PutValue(stfcType, _bstr_t(InvUtils::ostOnHand_Text));
		m_StatusCombo->AddRow(pRow);
		pRow = m_StatusCombo->GetRow(sriGetNewRow);
		pRow->PutValue(stfcID, (long)InvUtils::ostAllocated);
		pRow->PutValue(stfcType, _bstr_t(InvUtils::ostAllocated_Text));
		m_StatusCombo->AddRow(pRow);		
		pRow = m_StatusCombo->GetRow(sriGetNewRow);
		pRow->PutValue(stfcID, (long)InvUtils::ostUsed);
		pRow->PutValue(stfcType, _bstr_t(InvUtils::ostUsed_Text));
		m_StatusCombo->AddRow(pRow);
		pRow = m_StatusCombo->GetRow(sriGetNewRow);
		pRow->PutValue(stfcID, (long)InvUtils::ostAdjusted);		
		pRow->PutValue(stfcType, _bstr_t(InvUtils::ostAdjusted_Text));
		m_StatusCombo->AddRow(pRow);
		m_StatusCombo->SetSelByColumn(stfcID, (long)InvUtils::ostAll);

		// (a.walling 2008-02-18 16:38) - PLID 28946 - Hide advanced inventory options if unlicensed
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if (!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			GetDlgItem(IDC_TRACKER_STATUS_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_TRACKER_STATUS_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CHECK_FILTER_CONSIGNMENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_CHECK_FILTER_CONSIGNMENT)->ShowWindow(SW_HIDE);
		}

		// (j.jones 2007-11-13 15:21) - PLID 28081 - completely rewrote the from clause and then
		// moved it to code for easier readibility and ability to use InvUtils defines
		//DRT 11/28/2007 - PLID 28215 - We had a 'Status Name' field, but not an ID, so I added one.
		// (j.jones 2007-11-29 11:02) - PLID 28196 - accounted for completed allocations as "used",
		// though the status is superceded if also billed or on a case history
		// (c.haag 2007-12-03 11:54) - PLID 28204 - Changed Consignment bit to Status integer flag
		// (j.jones 2008-01-16 16:56) - PLID 28640 - added support for allocation notes
		//TES 5/27/2008 - PLID 29459 - Updated to use the linked appointment date (if any) as the "Date Used"
		// (j.jones 2008-06-02 16:16) - PLID 28076 - adjusted products now use the adjustment date,
		// but it can be null if it is old data prior to when we started tracking adjustment IDs
		// (j.jones 2008-06-06 10:07) - PLID 27110 - If a product is returned from a bill, we use a dummy product
		// as a placeholder on that bill, which references the original product with ProductItemsT.ReturnedFrom.
		// This screen needs to ignore all ProductItems with a ReturnedFrom value.
		//TES 6/25/2008 - PLID 26142 - Added the Adjustment category (if any).
		// (j.jones 2009-02-09 12:56) - PLID 32775 - renamed 'Charged' to 'Billed'
		CString strFrom;
		strFrom.Format("(SELECT ProductItemsT.ID, ProductItemsT.ProductID, ServiceT.Name AS ProductName, "
			"ServiceT.Category, ProductItemsT.SerialNum, ProductItemsT.ExpDate, ProductItemsT.Status AS ProductItemStatus, "
			"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN %li "
			"WHEN ProductItemsT.Deleted = 1 THEN %li "
			"WHEN LineItemT.ID Is Not Null THEN %li "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN %li "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
			"	(CASE WHEN AllocatedItemsQ.Status = %li THEN %li ELSE %li END) "
			"ELSE %li END AS StatusTypeID, "
			"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN 'Returned' "
			"WHEN ProductItemsT.Deleted = 1 THEN CASE WHEN ProductAdjustmentCategoriesT.Name Is Null THEN 'Adjusted' "
			"	ELSE 'Adjusted - ' + ProductAdjustmentCategoriesT.Name END "
			"WHEN LineItemT.ID Is Not Null THEN 'Billed' "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN 'Case History' "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN "
			"	(CASE WHEN AllocatedItemsQ.Status = %li THEN 'Used & Not Billed' ELSE 'Allocated' END) "
			"ELSE 'Available' END AS StatusName, "
			"CASE WHEN (ProductItemsT.Deleted = 1 AND SupplierReturnItemsT.ID Is Not Null) THEN NULL "
			"WHEN ProductItemsT.Deleted = 1 THEN NULL "
			"WHEN LineItemT.ID Is Not Null THEN ChargedPersonT.ID "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN CasePersonT.ID "
			"WHEN AllocatedItemsQ.ProductItemID Is Not Null THEN AllocatedPersonT.ID "
			"ELSE NULL END AS PatientID, "
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
			"AllocatedItemsQ.Notes "
			"FROM ProductItemsT "
			"INNER JOIN ProductT ON ProductItemsT.ProductID = ProductT.ID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"LEFT JOIN LocationsT ON ProductItemsT.LocationID = LocationsT.ID "
			"LEFT JOIN SupplierReturnItemsT ON ProductItemsT.ID = SupplierReturnItemsT.ProductItemID "
			"LEFT JOIN SupplierReturnGroupsT ON SupplierReturnItemsT.ReturnGroupID = SupplierReturnGroupsT.ID "
			"LEFT JOIN (SELECT PatientInvAllocationsT.PatientID, ProductItemID, InputDate, COALESCE(AppointmentsT.StartTime, PatientInvAllocationsT.CompletionDate) AS CompletionDate, PatientInvAllocationDetailsT.Status, PatientInvAllocationDetailsT.Notes FROM PatientInvAllocationDetailsT "
			"		   INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"		   LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
			"		   WHERE ((PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationsT.Status = %li) OR (PatientInvAllocationDetailsT.Status = %li AND PatientInvAllocationsT.Status = %li)) "
			"		   ) AS AllocatedItemsQ ON ProductItemsT.ID = AllocatedItemsQ.ProductItemID "
			"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"LEFT JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineItemT ON ChargedProductItemsT.ChargeID = LineItemT.ID "
			"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
			"LEFT JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
			"LEFT JOIN PersonT AS AllocatedPersonT ON AllocatedItemsQ.PatientID = AllocatedPersonT.ID "
			"LEFT JOIN PersonT AS ChargedPersonT ON LineItemT.PatientID = ChargedPersonT.ID "
			"LEFT JOIN PersonT AS CasePersonT ON CaseHistoryT.PersonID = CasePersonT.ID "
			"LEFT JOIN ProductAdjustmentsT ON ProductItemsT.AdjustmentID = ProductAdjustmentsT.ID "
			"LEFT JOIN ProductAdjustmentCategoriesT ON ProductAdjustmentsT.ProductAdjustmentCategoryID = ProductAdjustmentCategoriesT.ID "
			"WHERE ProductItemsT.ReturnedFrom Is Null) AS SerialTrackerQ",
			InvUtils::ostAdjusted, InvUtils::ostAdjusted, InvUtils::ostUsed, InvUtils::ostUsed, InvUtils::iadsUsed, InvUtils::ostUsed, InvUtils::ostAllocated, InvUtils::ostAvailable,	//StatusTypeID fields
			InvUtils::ostUsed, InvUtils::ostUsed, InvUtils::iadsActive, InvUtils::iasActive, InvUtils::iadsUsed, InvUtils::iasCompleted);

		m_SerialList->PutFromClause(_bstr_t(strFrom));

		m_dtStart = m_dtEnd = COleDateTime::GetCurrentTime();
		UpdateData(FALSE);

	}NxCatchAll("Error in CInvSerialTrackerDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CInvSerialTrackerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvSerialTrackerDlg)
	ON_EVENT(CInvSerialTrackerDlg, IDC_PATIENT_COMBO, 16 /* SelChosen */, OnSelChosenPatientCombo, VTS_I4)
	ON_EVENT(CInvSerialTrackerDlg, IDC_CATEGORY_LIST, 16 /* SelChosen */, OnSelChosenCategoryList, VTS_I4)
	ON_EVENT(CInvSerialTrackerDlg, IDC_ITEM, 16 /* SelChosen */, OnSelChosenItem, VTS_I4)
	ON_EVENT(CInvSerialTrackerDlg, IDC_CATEGORY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedCategoryList, VTS_I2)
	ON_EVENT(CInvSerialTrackerDlg, IDC_ITEM, 18 /* RequeryFinished */, OnRequeryFinishedItem, VTS_I2)
	ON_EVENT(CInvSerialTrackerDlg, IDC_PATIENT_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedPatientCombo, VTS_I2)
	ON_EVENT(CInvSerialTrackerDlg, IDC_SERIAL_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedSerialCombo, VTS_I2)
	ON_EVENT(CInvSerialTrackerDlg, IDC_SERIAL_COMBO, 16 /* SelChosen */, OnSelChosenSerialCombo, VTS_I4)
	ON_EVENT(CInvSerialTrackerDlg, IDC_LIST_SERIALIZED, 3 /* DblClickCell */, OnDblClickCellListSerialized, VTS_I4 VTS_I2)
	ON_EVENT(CInvSerialTrackerDlg, IDC_TRACKER_STATUS_LIST, 16 /* SelChosen */, OnSelChosenTrackerStatusList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInvSerialTrackerDlg::OnSelChosenPatientCombo(long nRow) 
{
	Refilter();
}


void CInvSerialTrackerDlg::OnSelChosenSerialCombo(long nRow) 
{
	Refilter();
}

void CInvSerialTrackerDlg::OnDatetimechangeStartdate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	Refilter();
}

void CInvSerialTrackerDlg::OnDatetimechangeEnddate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	Refilter();
}

void CInvSerialTrackerDlg::OnSelChosenCategoryList(long nRow) 
{
	//(e.lally 2008-01-30) PLID 28627 - Updated to use DL enum
	if(m_CategoryCombo->CurSel == sriNoRow)
		m_CategoryCombo->CurSel = 0;

	// Requery the item list
	
	//JMJ - 7/14/2003 - we want to only show items that track serial numbers, track exp. dates, OR 
	//have any serial number or exp. date data, whether in stock or not
	//(e.lally 2008-01-30) PLID 28627 - updated to column enums, check on VT_NULL instead of VT_EMPTY 
	COleVariant v = m_CategoryCombo->GetValue(m_CategoryCombo->CurSel, cfcID);
	if (v.vt == VT_NULL)
		m_ProductCombo->WhereClause = _bstr_t("(HasSerialNum = 1 OR HasExpDate = 1 OR "
		"ServiceT.ID IN (SELECT ProductID FROM ProductItemsT WHERE Deleted = 0)) AND ServiceT.Active = 1");
	else m_ProductCombo->WhereClause = _bstr_t("(HasSerialNum = 1 OR HasExpDate = 1 OR "
		"ServiceT.ID IN (SELECT ProductID FROM ProductItemsT WHERE Deleted = 0)) AND ServiceT.Active = 1 AND Category " 
		+ InvUtils::Descendants(v.lVal));
	m_ProductCombo->Requery();
}

void CInvSerialTrackerDlg::OnSelChosenItem(long nRow) 
{
	Refilter();
}

void CInvSerialTrackerDlg::Refilter(NXDATALISTLib::_DNxDataListPtr dlFinishedRequerying /*= NULL*/)
{
	// (j.jones 2007-11-13 15:28) - PLID 28081 - removed the restriction on deleted product items
	CString strWhere = "(SerialNum IS NOT NULL OR ExpDate IS NOT NULL) ";
	CString str;

	// Don't do any refiltering if anything is still requerying
	if (dlFinishedRequerying != m_CategoryCombo && m_CategoryCombo->IsRequerying()) return;
	if (dlFinishedRequerying != m_ProductCombo && m_ProductCombo->IsRequerying()) return;
	if (dlFinishedRequerying != m_PatientCombo && m_PatientCombo->IsRequerying())	return;

	// Cancel the requerying of the serialized list if it's requerying
	if (m_SerialList->IsRequerying())
		m_SerialList->CancelRequery();

	// Now build the new where clause for the list
	//(e.lally 2008-01-30) PLID 28627 - These checks are now sort safe by checking the values
		// - Updated to use column enums
	if (m_SerialCombo->CurSel >= 0 && VarString(m_SerialCombo->GetValue(m_SerialCombo->CurSel, sfcID)) != AllSerialNumsValue)
	{
		str.Format(" AND SerialNum = '%s'", VarString(m_SerialCombo->GetValue(m_SerialCombo->CurSel, sfcID)));
		strWhere += str;
	}
	if (m_CategoryCombo->CurSel >= 0 && m_CategoryCombo->GetValue(m_CategoryCombo->CurSel, cfcID).vt != VT_NULL)
	{
		str.Format(" AND Category = %d", VarLong(m_CategoryCombo->GetValue(m_CategoryCombo->CurSel, cfcID)));
		strWhere += str;
	}
	if (m_ProductCombo->CurSel >= 0 && m_ProductCombo->GetValue(m_ProductCombo->CurSel, prfcID).vt != VT_NULL)
	{
		str.Format(" AND ProductID = %d", VarLong(m_ProductCombo->GetValue(m_ProductCombo->CurSel, prfcID)));
		strWhere += str;
	}
	if (m_PatientCombo->CurSel >= 0 && m_PatientCombo->GetValue(m_PatientCombo->CurSel, pafcID).vt != VT_NULL)
	{
		str.Format(" AND PatientID = %d", VarLong(m_PatientCombo->GetValue(m_PatientCombo->CurSel, pafcID)));
		strWhere += str;
	}
	//DRT 11/28/2007 - PLID 28215 - Ability to filter on status type
	//(e.lally 2008-01-30) PLID 28627 - Updated to use column enums
	InvUtils::OverviewStatusType ost = InvUtils::ostAll;
	if(m_StatusCombo->CurSel > 0) {
		ost = (InvUtils::OverviewStatusType)VarLong(m_StatusCombo->GetValue(m_StatusCombo->CurSel, stfcID));

		//If it's all, we do nothing.  Otherwise, we filter
		if(ost != InvUtils::ostAll && ost != InvUtils::ostOnHand) {
			str.Format(" AND StatusTypeID = %li", (long)ost);
			strWhere += str;
		}
		else if(ost == InvUtils::ostOnHand) {
			// (j.jones 2007-11-30 13:12) - PLID 27989 - "on hand" means Available OR Allocated
			str.Format(" AND (StatusTypeID = %li OR StatusTypeID = %li)", (long)InvUtils::ostAvailable, (long)InvUtils::ostAllocated);
			strWhere += str;
		}
	}

	UpdateData(TRUE);
	if (m_bFilterOnDates)
	{
		// (j.jones 2007-11-13 15:32) - PLID 28081 - converted to be a date used filter,
		// not just a charge date filter, also fixed so it properly ran through the end date
		str.Format(" AND DateUsed Is Not Null AND DateUsed >= '%s' AND DateUsed < DATEADD(day, 1, '%s')",
			FormatDateTimeForSql(m_dtStart, dtoDate), FormatDateTimeForSql(m_dtEnd, dtoDate));
		strWhere += str;
	}
	// (j.jones 2007-11-13 15:00) - PLID 28081 - added ability to filter on Consignment products
	// (c.haag 2007-12-03 11:53) - PLID 28204 - Changed the Consignment bit to a Status flag
	if (m_bFilterOnConsignment)
	{
		strWhere += FormatString(" AND ProductItemStatus = %d", InvUtils::pisConsignment);
	}

	m_SerialList->WhereClause = (LPCTSTR)strWhere;

	// Now requery the list
	m_SerialList->Requery();
}

//(e.lally 2008-01-30) PLID 28627 - Updated to use column enums
void CInvSerialTrackerDlg::OnRequeryFinishedCategoryList(short nFlags) 
{
	IRowSettingsPtr pRow = m_CategoryCombo->GetRow(sriGetNewRow);
	pRow->PutValue(cfcID,_variant_t(m_vtNull));
	pRow->PutValue(cfcCategory,_variant_t(AllCategoriesValue));
	pRow->PutValue(cfcPrimarySort, _variant_t((long)0));
	m_CategoryCombo->AddRow(pRow);
	m_CategoryCombo->CurSel = 0;

	// Invoke the selection event so that the item dropdown is requeried
	OnSelChosenCategoryList(0);
	Refilter(m_CategoryCombo);
}

//(e.lally 2008-01-30) PLID 28627 - Updated to use column enums
void CInvSerialTrackerDlg::OnRequeryFinishedItem(short nFlags) 
{
	IRowSettingsPtr pRow = m_ProductCombo->GetRow(sriGetNewRow);
	pRow->PutValue(prfcID,_variant_t(m_vtNull));
	pRow->PutValue(prfcProduct,_variant_t(AllProductsValue));
	pRow->PutValue(prfcPrimarySort,_variant_t((long)0));
	m_ProductCombo->AddRow(pRow);
	m_ProductCombo->CurSel = 0;
	Refilter(m_ProductCombo);
}

//(e.lally 2008-01-30) PLID 28627 - Updated to use column enums
void CInvSerialTrackerDlg::OnRequeryFinishedPatientCombo(short nFlags) 
{
	IRowSettingsPtr pRow = m_PatientCombo->GetRow(sriGetNewRow);
	pRow->Value[pafcID] = _variant_t(m_vtNull);
	pRow->Value[pafcName] = _bstr_t(AllPatientsValue);
	pRow->Value[pafcPrimarySort] = (long) 0;

	m_PatientCombo->AddRow(pRow);
	if(m_nDefaultPatientID == sriNoRow)
		m_PatientCombo->CurSel = 0;
	else
		m_PatientCombo->SetSelByColumn(pafcID, m_nDefaultPatientID);
	Refilter(m_PatientCombo);
}

//(e.lally 2008-01-30) PLID 28627 - Updated to use column enums
void CInvSerialTrackerDlg::OnRequeryFinishedSerialCombo(short nFlags) 
{
	IRowSettingsPtr pRow = m_SerialCombo->GetRow(sriGetNewRow);
	pRow->Value[sfcID] = _bstr_t(AllSerialNumsValue);
	pRow->Value[sfcPrimarySort] = _variant_t((long) 0);
	m_SerialCombo->AddRow(pRow);
	m_SerialCombo->CurSel = 0;
	Refilter(m_SerialCombo);
}

void CInvSerialTrackerDlg::OnCheckFilterOnDates() 
{
	UpdateData(TRUE);
	GetDlgItem(IDC_STARTDATE)->EnableWindow(m_bFilterOnDates);
	GetDlgItem(IDC_ENDDATE)->EnableWindow(m_bFilterOnDates);
	Refilter();
}

void CInvSerialTrackerDlg::OnDblClickCellListSerialized(long nRowIndex, short nColIndex) 
{
	try {

		// (j.jones 2007-12-04 14:28) - PLID 28274 - leave this function if the user
		// didn't double-click a valid row
		//(e.lally 2008-01-30) PLID 28627 - Updated to use DL enum
		if(nRowIndex == sriNoRow) {
			return;
		}

		//try to open this item in the inv module
		long nItemID = VarLong(m_SerialList->GetValue(nRowIndex, slcProductID));

		CMainFrame *p = GetMainFrame();
		CNxTabView *pView;

		if(p->FlipToModule(INVENTORY_MODULE_NAME)) {

			pView = (CNxTabView *)p->GetOpenView(INVENTORY_MODULE_NAME);
			if (pView) 
			{
				pView->SetActiveTab(InventoryModule::ItemTab);

				BOOL bSet = ((CInvEditDlg*)pView->GetActiveSheet())->LoadSingleItem(nItemID);

				if(!bSet) {
					// (j.jones 2007-12-06 08:55) - PLID 28292 - changed the message to indicate
					// the product is likely inactive
					MsgBox("The product could not be loaded, it may be inactive.");
				}
				else {
					//close the dialog
					OnOK();
				}
			}
		}
	} NxCatchAll("Error in CInvSerialTrackerDlg::OnDblClickCellListSerialized()");
}

// (j.jones 2007-11-13 15:00) - PLID 28081 - added ability to filter on Consignment products
void CInvSerialTrackerDlg::OnCheckFilterConsignment() 
{
	try {
		
		UpdateData(TRUE);
		Refilter();

	}NxCatchAll("Error in CInvSerialTrackerDlg::OnCheckFilterConsignment");
}

//DRT 11/28/2007 - PLID 28215
void CInvSerialTrackerDlg::OnSelChosenTrackerStatusList(long nRow) 
{
	try {
		//(e.lally 2008-01-30) PLID 28627 - Updated to use DL enum
		if(m_StatusCombo->CurSel == sriNoRow)
			m_StatusCombo->CurSel = 0;

		Refilter();
	} NxCatchAll("Error in OnSelChosenTrackerStatusList");
}

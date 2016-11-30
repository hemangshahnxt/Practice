// InvOrderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "InvOrderDlg.h"
#include "InvEditOrderDlg.h"
#include "DontShowDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "ProductItemsDlg.h"
#include "AuditTrail.h"
#include "InvUtils.h"
#include "DateTimeUtils.h"
#include "GlobalReportUtils.h"
#include "ReceiveOrderDlg.h"
#include "InventoryRc.h"
#include "InvEditReturnDlg.h"
#include "Barcode.h"
#include "GlobalSchedUtils.h"
#include "TodoUtils.h"
#include "ProductsToBeReturnedDlg.h"
#include "InvOrderDateFilterPickerDlg.h"
#include "ProductsToBeOrderedDlg.h"
#include "SupplierStatementDlg.h"
#include "ConsignmentReconcileDlg.h"
#include "invlabelreportdlg.h"// (s.dhole 2010-07-16 16:47) - PLID 28183 It would be nice to be able to print Avery 8167 labels from the order screen by right clicking print, and then have it come up with an option to select which products need labels, and to print the labels quantity based on the # received.
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



static COleDateTime GetDate()
{
	COleDateTime dt = COleDateTime::GetCurrentTime();
	return COleDateTime(dt.GetYear(), dt.GetMonth(), dt.GetDay(), 0, 0, 0);
}

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CInvOrderDlg dialog

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define	IDM_RECEIVE		52702
// (c.haag 2008-05-21 15:44) - PLID 30082 - We no longer support unreceiving orders because it can
// corrupt serialized data and cause the Inventory Values report to go out of sync with the Physical
// Inventory Tally report.
//#define IDM_UNRECEIVE	52703
#define	IDM_DELETE		52701
// (s.dhole 2010-07-16 16:47) - PLID 28183 It would be nice to be able to print Avery 8167 labels from the order screen by right clicking print, and then have it come up with an option to select which products need labels, and to print the labels quantity based on the # received.
#define	IDM_PRINT_LABEL		52707

// (c.haag 2007-11-09 16:40) - PLID 27992 - Menu items for return groups
#define IDM_OPEN_RETURN_GROUP	52704
#define IDM_DELETE_RETURN_GROUP	52705

// (b.spivey, October 20, 2011) - PLID 46073 - Menu item for frames labels. 
#define IDM_PRINT_FRAMES_LABEL	52708

typedef enum {
	eclRet_ReturnGroupID = 0,
	eclRet_SupplierID = 1,
	eclRet_LocationID = 2,
	eclRet_SupplierName = 3,
	eclRet_Description = 4,
	eclRet_TrackingNumber = 5,
	eclRet_DatePlaced = 6,
	eclRet_DateCompleted = 7,
	eclRet_TotalCredit = 8,
	eclRet_Notes = 9,
} EReturnListColumns;

typedef enum {
	eclSup_ID,
	eclSup_Name
} ESupplierDropdownColumns;

// (j.gruber 2008-06-30 14:53) - PLID 30564 - changed the radio buttons to datalists
enum ReturnTypeFilterColumn {
	rtfcID = 0,
	rtfcName = 1,
};

enum OrderTypeFilterColumn {
	otfcID = 0,
	otfcName = 1,
};

enum {
	ORDER_PENDING=1,
	ORDER_RECEIVED=2,
	ORDER_ALL=3,
};

enum {
	RETURN_PENDING=1,
	RETURN_COMPLETED=2,
	RETURN_ALL=3,
};
// (s.tullis 2014-08-12 14:09) - PLID 63241 - The InvOrderDlg needs a CTableChecker object for Suppliers.
CInvOrderDlg::CInvOrderDlg(CWnd* pParent)
	: CNxDialog(CInvOrderDlg::IDD, pParent), m_SuppliersChecker(NetUtils::Suppliers)
{
	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Inventory_Module/create_an_order.htm";

	
	

	m_pEditOrderDlg = NULL;
	//{{AFX_DATA_INIT(CInvOrderDlg)
	//}}AFX_DATA_INIT

	// (j.gruber 2008-06-30 10:53) - PLID 29205 - added date filters
	m_nDateFilterID = DATE_CREATED;

	// (j.gruber 2008-06-30 11:43) - PLID 30564 - changed radio buttons to datalists
	m_nOrderTypeFilterID = ORDER_PENDING;
	m_nReturnTypeFilterID = RETURN_PENDING;
}

CInvOrderDlg::~CInvOrderDlg()
{
	try {
		if (m_pEditOrderDlg)
			delete m_pEditOrderDlg;
	} NxCatchAll("Error in ~CInvOrderDlg");
}

void CInvOrderDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvOrderDlg)
	DDX_Control(pDX, IDC_TO_BE_ORDERED, m_nxbToBeOrdered);
	DDX_Control(pDX, IDC_INV_ORDER_USE_DATE, m_chkUseDateFilter);
	DDX_Control(pDX, IDC_INV_ORDER_TO_DATE, m_dtTo);
	DDX_Control(pDX, IDC_INV_ORDER_FROM_DATE, m_dtFrom);
	DDX_Control(pDX, IDC_TO_BE_RETURNED, m_nxbToBeReturned);
	DDX_Control(pDX, IDC_BTN_NEW_RETURN, m_returnBtn);
	DDX_Control(pDX, IDC_NEED, m_needBtn);
	DDX_Control(pDX, IDC_CREATE, m_createBtn);
	DDX_Control(pDX, IDC_LABEL_RETURN_SHOW, m_nxstaticLabelReturnShow);
	DDX_Control(pDX, IDC_LABEL_RETURN_SUPPLIER, m_nxstaticLabelReturnSupplier);
	DDX_Control(pDX, IDC_INV_ORDER_DATE_LABEL, m_nxlDateLabel);
	DDX_Control(pDX, IDC_BTN_SUPPLIER_STATEMENTS, m_btnSupplierStatements);
	DDX_Control(pDX, IDC_BTN_RECONCILE_CONSIGNMENT, m_btnReconcileConsignment);
	DDX_Control(pDX, IDC_CHECK_SHOWPONUMBER, m_chkShowPONumber);
	DDX_Control(pDX,IDC_RECIEVE_FRAMES,m_ReceiveFrameOrderBtn);

	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvOrderDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvOrderDlg)
	ON_BN_CLICKED(IDC_NEED, OnNeed)
	ON_BN_CLICKED(IDC_CREATE, OnCreateOrder)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_NEW_RETURN, OnBtnNewReturn)
	ON_COMMAND(IDM_OPEN_RETURN_GROUP, OnOpenReturnGroup)
	ON_COMMAND(IDM_DELETE_RETURN_GROUP, OnDeleteReturnGroup)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_BN_CLICKED(IDC_TO_BE_RETURNED, OnToBeReturned)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)	
	ON_BN_CLICKED(IDC_INV_ORDER_USE_DATE, OnInvOrderUseDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_INV_ORDER_TO_DATE, OnDatetimechangeInvOrderToDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_INV_ORDER_FROM_DATE, OnDatetimechangeInvOrderFromDate)
	ON_BN_CLICKED(IDC_TO_BE_ORDERED, OnToBeOrdered)
	ON_BN_CLICKED(IDC_BTN_SUPPLIER_STATEMENTS, OnBtnSupplierStatements)
	ON_BN_CLICKED(IDC_BTN_RECONCILE_CONSIGNMENT, OnBtnReconcileConsignment)
	ON_BN_CLICKED(IDC_CHECK_SHOWPONUMBER, &CInvOrderDlg::OnBnClickedCheckShowponumber)
	//}}AFX_MSG_MAP		
	ON_BN_CLICKED(IDC_RECIEVE_FRAMES, &CInvOrderDlg::OnBnClickedRecieveFrames)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvOrderDlg message handlers

BEGIN_EVENTSINK_MAP(CInvOrderDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvOrderDlg)
	ON_EVENT(CInvOrderDlg, IDC_ORDER_LIST, 7 /* RButtonUp */, OnRButtonUpOrderList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInvOrderDlg, IDC_ORDER_LIST, 3 /* DblClickCell */, OnDblClickCellOrderList, VTS_I4 VTS_I2)
	ON_EVENT(CInvOrderDlg, IDC_SUPPLIER_FILTER, 16 /* SelChosen */, OnSelChosenSupplierFilter, VTS_I4)
	ON_EVENT(CInvOrderDlg, IDC_RETURN_LIST, 3 /* DblClickCell */, OnDblClickCellReturnList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CInvOrderDlg, IDC_SUPPLIER_FILTER_RETURN, 16 /* SelChosen */, OnSelChosenSupplierFilterReturn, VTS_DISPATCH)
	ON_EVENT(CInvOrderDlg, IDC_RETURN_LIST, 6 /* RButtonDown */, OnRButtonDownReturnList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CInvOrderDlg, IDC_SUPPLIER_FILTER, 20 /* TrySetSelFinished */, OnTrySetSelFinishedSupplierFilter, VTS_I4 VTS_I4)
	ON_EVENT(CInvOrderDlg, IDC_SUPPLIER_FILTER_RETURN, 20 /* TrySetSelFinished */, OnTrySetSelFinishedSupplierFilterReturn, VTS_I4 VTS_I4)
	ON_EVENT(CInvOrderDlg, IDC_INV_ORDER_RETURN_FILTER, 1 /* SelChanging */, OnSelChangingInvOrderReturnFilter, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvOrderDlg, IDC_INV_ORDER_RETURN_FILTER, 16 /* SelChosen */, OnSelChosenInvOrderReturnFilter, VTS_DISPATCH)
	ON_EVENT(CInvOrderDlg, IDC_INV_ORDER_ORDER_FILTER, 1 /* SelChanging */, OnSelChangingInvOrderOrderFilter, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvOrderDlg, IDC_INV_ORDER_ORDER_FILTER, 16 /* SelChosen */, OnSelChosenInvOrderOrderFilter, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CInvOrderDlg::OnInitDialog() 
{
	try { // (c.haag 2007-11-09 10:39) - PLID 27992 - Added try/catch
		CNxDialog::OnInitDialog();

		// (c.haag 2010-01-14 10:13) - PLID 30503 - Bulk caching
		g_propManager.CachePropertiesInBulk("INVORDERDLG", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'InvItem_OrderByOnHandAmount' OR "
			"Name = 'InvOrder_ShowPOBoxColumn' "
			")",
			_Q(GetCurrentUserName()));

		// (c.haag 2010-01-14 10:24) - PLID 30503 - Check the Show P.O. Number box based on preference
		if (GetRemotePropertyInt("InvOrder_ShowPOBoxColumn", 1, 0, GetCurrentUserName(), true)) {
			m_chkShowPONumber.SetCheck(1);
		} else {
			m_chkShowPONumber.SetCheck(0);
		}

		// (c.haag 2007-11-09 13:22) - PLID 27992 - Set new return button color
		m_returnBtn.AutoSet(NXB_NEW);
		m_createBtn.AutoSet(NXB_NEW);
		m_needBtn.AutoSet(NXB_PRINT_PREV);

		//TES 6/23/2008 - PLID 26152 - Set this to NXB_NEW, as it is designed to result in a new return.
		m_nxbToBeReturned.AutoSet(NXB_NEW);

		//TES 7/22/2008 - PLID 30802 - Set this to NXB_NEW, as it is designed to result in a new order
		m_nxbToBeOrdered.AutoSet(NXB_NEW);

		// (j.jones 2009-03-17 13:32) - PLID 32831 - added m_btnSupplierStatements
		m_btnSupplierStatements.AutoSet(NXB_INSPECT);

		// (j.jones 2009-03-17 13:38) - PLID 32832 - added m_btnReconcileConsignment
		m_btnReconcileConsignment.AutoSet(NXB_MODIFY);

		// (c.haag 2008-04-24 16:31) - PLID 29778 - Removed old text color
		//m_needBtn.SetTextColor(0xFF00FF);

		m_brush.CreateSolidBrush(PaletteColor(0x00FFDBDB));

		m_SupplierFilter = BindNxDataListCtrl(IDC_SUPPLIER_FILTER);

		IRowSettingsPtr pRow = m_SupplierFilter->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,_bstr_t("<All Suppliers>"));
		m_SupplierFilter->InsertRow(pRow,0);

		m_SupplierFilter->SetSelByColumn(0,(long)-1);

		m_list = BindNxDataListCtrl(IDC_ORDER_LIST, false);

		// (j.gruber 2008-06-30 11:28) - PLID 30564 - changed radios to datalists
		//m_pending.SetCheck(TRUE);
		//(e.lally 2008-10-08) PLID 31619 - Datalist2.0 controls need to use BindNxDataList2Ctrl
		m_pOrderTypeFilterList = BindNxDataList2Ctrl(IDC_INV_ORDER_ORDER_FILTER, false);

		NXDATALIST2Lib::IRowSettingsPtr pOrderRow = m_pOrderTypeFilterList->GetNewRow();
		if (pOrderRow) {
			pOrderRow->PutValue(otfcID, (long)ORDER_PENDING);
			pOrderRow->PutValue(otfcName, _variant_t("Pending"));
			m_pOrderTypeFilterList->AddRowAtEnd(pOrderRow, NULL);
		}

		pOrderRow = m_pOrderTypeFilterList->GetNewRow();
		if (pOrderRow) {
			pOrderRow->PutValue(otfcID, (long)ORDER_RECEIVED);
			pOrderRow->PutValue(otfcName, _variant_t("Received"));
			m_pOrderTypeFilterList->AddRowAtEnd(pOrderRow, NULL);
		}

		pOrderRow = m_pOrderTypeFilterList->GetNewRow();
		if (pOrderRow) {
			pOrderRow->PutValue(otfcID, (long)ORDER_ALL);
			pOrderRow->PutValue(otfcName, _variant_t("All"));
			m_pOrderTypeFilterList->AddRowAtEnd(pOrderRow, NULL);
		}
		//set it to be pending
		m_pOrderTypeFilterList->SetSelByColumn(otfcID, (long)ORDER_PENDING);

		// (j.gruber 2008-06-30 09:37) - PLID 29205 - added created and completed date options
		m_nxlDateLabel.SetText("Created Date");
		m_nxlDateLabel.SetType(dtsDisabledHyperlink);
		m_nxlDateLabel.EnableWindow(FALSE);


		//setup the date values 
		CheckDlgButton(IDC_INV_ORDER_USE_DATE, 0);
		COleDateTime dtMin, dtNow;
		dtMin.SetDate(1800,01,01);
		dtNow = COleDateTime::GetCurrentTime();
		m_dtTo.SetMinDate(dtMin);
		m_dtTo.SetValue(dtNow);
		m_dtTo.EnableWindow(FALSE);
		m_dtFrom.SetMinDate(dtMin);
		m_dtFrom.SetValue(dtNow );
		m_dtFrom.EnableWindow(FALSE);

		//and the member variables
		m_dtToDate = dtNow;
		m_dtFromDate = dtNow;		

		SetWhereClause();

		// (c.haag 2007-11-09 10:53) - PLID 27992 - Bind the return supplier filter
		m_SupplierFilterReturns = BindNxDataList2Ctrl(IDC_SUPPLIER_FILTER_RETURN);
		NXDATALIST2Lib::IRowSettingsPtr pRowReturn = m_SupplierFilterReturns->GetNewRow();
		pRowReturn->PutValue(eclSup_ID, -1L);
		pRowReturn->PutValue(eclSup_Name, _bstr_t("<All Suppliers>"));
		m_SupplierFilterReturns->AddRowBefore(pRowReturn, m_SupplierFilterReturns->GetFirstRow());
		m_SupplierFilterReturns->CurSel = pRowReturn;

		// (c.haag 2007-11-09 10:37) - PLID 27992 - Bind the return list
		m_listReturns = BindNxDataList2Ctrl(IDC_RETURN_LIST, false);
		//m_pendingReturn.SetCheck(TRUE);
		// (j.gruber 2008-06-30 11:36) - PLID 30564 - changed the radio buttons to datalist
		//(e.lally 2008-10-08) PLID 31619 - Datalist2.0 controls need to use BindNxDataList2Ctrl
		m_pReturnTypeFilterList = BindNxDataList2Ctrl(IDC_INV_ORDER_RETURN_FILTER, false);

		NXDATALIST2Lib::IRowSettingsPtr pReturnRow = m_pReturnTypeFilterList->GetNewRow();
		if (pReturnRow) {
			pReturnRow->PutValue(rtfcID, (long)RETURN_PENDING);
			pReturnRow->PutValue(rtfcName, _variant_t("Pending"));
			m_pReturnTypeFilterList->AddRowAtEnd(pReturnRow, NULL);
		}

		pReturnRow = m_pReturnTypeFilterList->GetNewRow();
		if (pReturnRow) {
			pReturnRow->PutValue(rtfcID, (long)RETURN_COMPLETED);
			pReturnRow->PutValue(rtfcName, _variant_t("Completed"));
			m_pReturnTypeFilterList->AddRowAtEnd(pReturnRow, NULL);
		}

		pReturnRow = m_pReturnTypeFilterList->GetNewRow();
		if (pReturnRow) {
			pReturnRow->PutValue(rtfcID, (long)RETURN_ALL);
			pReturnRow->PutValue(rtfcName, _variant_t("All"));
			m_pReturnTypeFilterList->AddRowAtEnd(pReturnRow, NULL);
		}
		//set it to be pending
		m_pReturnTypeFilterList->SetSelByColumn(rtfcID, (long)RETURN_PENDING);

		SetWhereClause_Returns();

		// (a.walling 2008-02-15 15:30) - PLID 28946 - Hide and disable the return controls,
		// then resize the order list.
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if (!g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			/*
			//(e.lally 2010-09-23) PLID 40563 - The "Returns" feature is now available to everyone with inventory.

			GetDlgItem(IDC_LABEL_RETURN_SHOW)->EnableWindow(FALSE);
			GetDlgItem(IDC_LABEL_RETURN_SUPPLIER)->EnableWindow(FALSE);
			GetDlgItem(IDC_RETURN_LIST)->EnableWindow(FALSE);
			GetDlgItem(IDC_SUPPLIER_FILTER_RETURN)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_NEW_RETURN)->EnableWindow(FALSE);
			
			
			GetDlgItem(IDC_LABEL_RETURN_SHOW)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LABEL_RETURN_SUPPLIER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RETURN_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SUPPLIER_FILTER_RETURN)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_NEW_RETURN)->ShowWindow(SW_HIDE);
			*/

			//TES 6/23/2008 - PLID 26152 - Added
			// (j.jones 2011-07-15 12:42) - PLID 42678 - This was mistakenly commented out,
			// because returns are now available to everybody, however the products to be
			// returned feature is solely used with allocations only, as it is the only
			// way you can flag a product to be returned!
			GetDlgItem(IDC_TO_BE_RETURNED)->EnableWindow(FALSE);
			GetDlgItem(IDC_TO_BE_RETURNED)->ShowWindow(SW_HIDE);
			
			// (j.jones 2009-03-17 16:39) - PLID 32831 - hide the supplier statement feature
			GetDlgItem(IDC_BTN_SUPPLIER_STATEMENTS)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_SUPPLIER_STATEMENTS)->ShowWindow(SW_HIDE);

			// (j.jones 2009-03-17 16:39) - PLID 32832 - hide the consignment reconciling feature
			GetDlgItem(IDC_BTN_RECONCILE_CONSIGNMENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_RECONCILE_CONSIGNMENT)->ShowWindow(SW_HIDE);

			//TES 7/22/2008 - PLID 30802 - Not return-related, but based on allocations, and so part of AdvInventory.
			GetDlgItem(IDC_TO_BE_ORDERED)->ShowWindow(SW_HIDE);

			
			//(e.lally 2010-09-23) PLID 40563 - The "Returns" feature is now available to everyone with inventory.
			//So we can resize the return list to the bottom instead of the order list.
			CRect rcReconcileConsignment, rcReturnList;
			GetDlgItem(IDC_BTN_RECONCILE_CONSIGNMENT)->GetWindowRect(rcReconcileConsignment);
			GetDlgItem(IDC_RETURN_LIST)->GetWindowRect(rcReturnList);

			ScreenToClient(rcReconcileConsignment);
			ScreenToClient(rcReturnList);

			//We can extend this to the bottom of the buttons since none are showing.
			rcReturnList.bottom = rcReconcileConsignment.bottom;
			GetDlgItem(IDC_RETURN_LIST)->MoveWindow(rcReturnList);
			
			GetControlPositions();
		}

		// (c.haag 2010-01-14 10:15) - PLID 30503 - Show or hide columns based on preferences
		ReflectColumnAppearances();

		//r.wilson
		m_ReceiveFrameOrderBtn.AutoSet(NXB_NEW);
	}
	NxCatchAll("Error in CInvOrderDlg::OnInitDialog");
	
	return TRUE;  
}

void CInvOrderDlg::SetWhereClause()
{
	CString where = "OrderT.Deleted = 0 AND OrderDetailsT.Deleted = 0";

	if (m_nOrderTypeFilterID == ORDER_PENDING)
		where += " AND ReceivedQ.OrderID IS NOT NULL";
	else if (m_nOrderTypeFilterID == ORDER_RECEIVED)
		where += " AND ReceivedQ.OrderID IS NULL";


	long nSupplierID = -1;
	if(m_SupplierFilter->CurSel != -1)
		nSupplierID = VarLong(m_SupplierFilter->GetValue(m_SupplierFilter->CurSel,0),-1);

	if(nSupplierID != -1) {
		CString str;
		str.Format(" AND OrderT.Supplier = %li",nSupplierID);
		where += str;
	}

	// (j.gruber 2008-06-30 10:54) - PLID 29205 - Date Filters
	//get the from date and to dates
	if (IsDlgButtonChecked(IDC_INV_ORDER_USE_DATE)) {
		COleDateTime dtTo, dtFrom;
		dtTo = m_dtTo.GetDateTime();
		dtFrom = m_dtFrom.GetDateTime();
		COleDateTimeSpan dtSpan(1,0,0,0);
		dtTo = dtTo + dtSpan;
	
		if (m_nDateFilterID == DATE_CREATED) {
			where += FormatString(" AND Date >= '%s' AND Date < '%s' ", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));
		}
		else if (m_nDateFilterID == DATE_RECEIVED) {
			//filter on any part of the order received, not just the whole thing
			where += FormatString(" AND OrderDetailsT.DateReceived >= '%s' AND OrderDetailsT.DateReceived < '%s' ", FormatDateTimeForSql(dtFrom, dtoDate), FormatDateTimeForSql(dtTo, dtoDate));			
		}	
	}


	//TES 11/6/2007 - PLID 27981 - VS2008 - Need to convert _bstr_t to CString
	if (CString((LPCTSTR)m_list->WhereClause) != where)
	{	m_list->WhereClause = _bstr_t(where);
		m_list->Requery();
	}
}

void CInvOrderDlg::SetWhereClause_Returns()
{
	// (c.haag 2007-11-09 10:40) - PLID 27992 - This is just like SetWhereClause
	// but it pertains to return-related controls
	CString strWhere;

	// (j.gruber 2008-06-30 11:43) - PLID 30564 - change the radio buttons to datalists
	if (m_nReturnTypeFilterID == RETURN_PENDING) {
		// For pending returns, we want all the groups that have at least one incomplete return item
		strWhere = "SupplierReturnGroupsT.ID IN (SELECT ReturnGroupID FROM SupplierReturnItemsT WHERE Completed = 0)";
	} else if (m_nReturnTypeFilterID == RETURN_COMPLETED) {
		// For received returns, we want all the groups that have no incomplete return items
		strWhere = "SupplierReturnGroupsT.ID NOT IN (SELECT ReturnGroupID FROM SupplierReturnItemsT WHERE Completed = 0)";
	} else {
		strWhere = "(1=1)";
	}


	NXDATALIST2Lib::IRowSettingsPtr pSupRow = m_SupplierFilterReturns->CurSel;
	long nSupplierID = VarLong(pSupRow->GetValue(eclSup_ID), -1);
	if (-1 == nSupplierID) {
		// Filter on all suppliers
	} else {
		strWhere += FormatString(" AND SupplierID = %d", nSupplierID);
	}

	CString strCurrentWhere = (LPCTSTR)m_listReturns->WhereClause;
	if (strWhere != strCurrentWhere) {
		m_listReturns->WhereClause = _bstr_t(strWhere);
		m_listReturns->Requery();
	}
}
// (s.tullis 2014-09-02 09:10) - PLID 63241 - The InvOrderDlg needs a CTableChecker object for Suppliers.
void CInvOrderDlg::Refresh()
{
	try{
		RefreshOrders();
		RefreshReturns();
		EnsureUpdatedSuppliers();
	}NxCatchAll(__FUNCTION__)
}

void CInvOrderDlg::RefreshOrders()
{
	// (c.haag 2007-11-14 10:37) - PLID 27992 - Requery the order list
	m_list->Requery();	
}

void CInvOrderDlg::RefreshReturns()
{
	// (c.haag 2007-11-14 09:49) - PLID 27992 - Requery the return list
	m_listReturns->Requery();
}

// (j.jones 2008-03-18 14:44) - PLID 29309 - added appt. and location IDs as optional parameters
void CInvOrderDlg::CreateOrder(BOOL bSaveAllAsReceived, long nApptID /*= -1*/, long nLocationID /*= -1*/)
{
	// (c.haag 2007-12-05 14:49) - PLID 28286 - This code was moved from OnCreateOrder.
	if (UserPermission(PlaceOrder))
	{	if (!m_pEditOrderDlg)
		{	m_pEditOrderDlg = new CInvEditOrderDlg(this);
			m_pEditOrderDlg->Create(IDD_INVEDITORDER);
		}
		m_pEditOrderDlg->DoFakeModal(-1, bSaveAllAsReceived, nApptID, nLocationID);
	}
}

/* (r.wilson 2012-2-20) PLID 48222 Clone of CreateOrder(...) but the difference is that if you get too this function 
			then you know you are doing a frame order.  Therefore any logic after doing a frame order can happen at
			the bottom of the function.
*/
void CInvOrderDlg::CreateFramesOrder(BOOL bSaveAllAsReceived, long nApptID /*= -1*/, long nLocationID /*= -1*/)
{
		//r.wilson - If you get here then you know that the order is a frame order
		//r.wilson - Soon this will be a separate permission for FrameOrdersOnly
		if (UserPermission(PlaceOrder))
		{	if (!m_pEditOrderDlg)
			{	m_pEditOrderDlg = new CInvEditOrderDlg(this,TRUE);
				m_pEditOrderDlg->Create(IDD_INVEDITORDER);
			}
			
			m_pEditOrderDlg->m_bFrameOrderReception = TRUE;
			m_pEditOrderDlg->DoFakeModal(-1, bSaveAllAsReceived, nApptID, nLocationID);
			
		}
}

/*( r.wilson 2012-20-2 ) PLID 42888 Function allows the InvOrderdlg to set the saved orderId from a frame order.
		This is also where we promm*/
void CInvOrderDlg::SetReturnedOrderId(long nOrderId)
{	
	if( IDYES == MessageBox("Would you like to print labels for this order now?","Frame Labels?",MB_ICONQUESTION|MB_YESNO))
	{
		PrintOrderLabels(TRUE,nOrderId);
	}
}

// (j.jones 2008-02-07 10:20) - PLID 28851 - there are two types of auto-ordering,
// purchased inventory and consignment, so require a boolean to be sent in to determine
// which "auto order" to call
void CInvOrderDlg::CreateAutoOrder(long supplierID, BOOL bConsignmentOrder)
{
	OnCreateOrder();
	m_pEditOrderDlg->AutoOrder(supplierID, bConsignmentOrder);
}

void CInvOrderDlg::OnCreateOrder()
{
	// (c.haag 2007-12-05 14:50) - PLID 28286 - This function is a message
	// handler; so we need exception handling.
	try {
		// (c.haag 2007-12-05 14:51) - PLID 28286 - The value of FALSE
		// tells the function to create a standard order
		CreateOrder(FALSE);
	}
	NxCatchAll("Error in CInvOrderDlg::OnCreateOrder");
}

void CInvOrderDlg::OnRButtonUpOrderList(long nRow, short nCol, long x, long y, long nFlags) 
{
	if (nRow != -1)
	{	
		m_list->PutCurSel(nRow);
		
		CMenu menPopup;
		CPoint pt;
		CWnd* pList;

		menPopup.m_hMenu = CreatePopupMenu();
		
		if ((GetCurrentUserPermissions(bioInvOrderReceived) & SPT___W________ANDPASS))
		{
			if (m_list->Value[nRow][iocDateArrived].vt != VT_DATE)
				menPopup.InsertMenu(0, MF_BYPOSITION, IDM_RECEIVE,	"Mark Received");
			// (c.haag 2008-05-21 15:44) - PLID 30082 - We no longer support unreceiving orders
			//else menPopup.InsertMenu(0, MF_BYPOSITION, IDM_UNRECEIVE, "Mark Pending");
		}

		if(GetCurrentUserPermissions(bioInvOrder) & SPT_____D______ANDPASS)
			menPopup.InsertMenu(2, MF_BYPOSITION, IDM_DELETE,	"Delete");
		// (s.dhole 2010-07-16 16:47) - PLID 28183 It would be nice to be able to print Avery 8167 labels from the order screen by right clicking print, and then have it come up with an option to select which products need labels, and to print the labels quantity based on the # received.
		menPopup.InsertMenu(3, MF_BYPOSITION, IDM_PRINT_LABEL,	"Print Labels For Order");

		// (b.spivey, October 21, 2011) - PLID 46073 - If they don't have the license, we don't want them using this feature. 
		if (g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)){
			menPopup.InsertMenu(4, MF_BYPOSITION, IDM_PRINT_FRAMES_LABEL, "Print Labels For Frames Order"); 
		}

		pList = GetDlgItem(IDC_ORDER_LIST);
		if (pList != NULL) 
		{	pt.x = x;
			pt.y = y;
			pList->ClientToScreen(&pt);
			menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
		else HandleException(NULL, "An error ocurred while creating menu");
		
		m_rightClicked = nRow;
	}
}

BOOL CInvOrderDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int id;

	switch (wParam) 
	{	
	case IDM_DELETE: {

			try {

			if(!CheckCurrentUserPermissions(bioInvOrder, sptDelete))
				break;

			id = m_list->Value[m_rightClicked][iocID].lVal;
			
			if(m_rightClicked == -1)
				break;	//error case

			//DRT 10/2/03 - PLID 9467 - They cannot delete an order that has serialized items that have been adjusted
			//off (marked 'deleted')
			//JMJ 1/27/04 - PLID 10465 - They cannot delete an order that has serialized items that have been billed
			// (j.jones 2007-11-07 10:53) - PLID 27987 - disallow deleting an order that has serialized items that
			// are allocated, even if the allocation is deleted
			//(also I combined these three queries into one)
			//TES 6/18/2008 - PLID 29578 - ProductItemsT now has an OrderDetailID instead of an OrderID
			// (j.jones 2008-09-10 13:59) - PLID 31320 - ensured that if the product item ID is referenced by another product,
			// you cannot delete the product item
			if(ReturnsRecords("SELECT ID FROM ProductItemsT WHERE OrderDetailID IN (SELECT ID FROM OrderDetailsT WHERE OrderID = %li) AND "
				"(Deleted = 1 "
				" OR ID IN (SELECT ProductItemID FROM ChargedProductItemsT) "
				" OR ID IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null)"
				" OR ID IN (SELECT ReturnedFrom FROM ProductItemsT) "
				" OR ReturnedFrom Is Not Null "
				")", id)) {
				MsgBox("There are items on this order that have either been adjusted, billed, returned from, or allocated to patients.\n"
					"You cannot delete this order.");
				break;
			}

			// (j.jones 2009-01-13 17:34) - PLID 26141 - forbid from deleting if referenced in a reconciliation
			if(ReturnsRecords("SELECT TOP 1 ID FROM InvReconciliationProductItemsT WHERE ProductItemID IN (SELECT ID FROM ProductItemsT WHERE OrderDetailID IN (SELECT ID FROM OrderDetailsT WHERE OrderID = %li))", id)) {
				MsgBox("There are items on this order that have been referenced in an Inventory Reconciliation.\n"
					"You cannot delete this order.");
				break;
			}

			// (j.jones 2008-03-18 15:30) - PLID 29309 - warn if the order is linked to an appointment
			CString strWarning = "Are you sure you want to delete this order?";
			_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name, OrderAppointmentsT.AppointmentID "
				"FROM PersonT "
				"INNER JOIN AppointmentsT ON PersonT.ID = AppointmentsT.PatientID "
				"INNER JOIN OrderAppointmentsT ON AppointmentsT.ID = OrderAppointmentsT.AppointmentID "
				"WHERE OrderAppointmentsT.OrderID = {INT}", id);
			long nAppointmentID = -1;
			if(!rs->eof) {

				// (j.jones 2008-03-24 16:03) - PLID 29388 - track this ID so we can send a tablechecker later
				nAppointmentID = AdoFldLong(rs, "AppointmentID",-1);

				strWarning.Format("This order is linked to an appointment for patient '%s'.\n\n"
					"Are you sure you want to delete this order?", AdoFldString(rs, "Name",""));
			}

			DontShowMeAgain(this, "Warning: If you delete an order, the amount on hand for the associated products WILL be changed.",
				"InvDeleteOrder");
			if (IDYES == MessageBox(strWarning, "Delete Order?", MB_YESNO))
			{	try
				{	
					// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
					CSqlTransaction trans("InvOrderDelete");
					trans.Begin();

					ExecuteSql ("UPDATE OrderDetailsT SET ModifiedBy = %i "
						"WHERE OrderID = %i AND Deleted = 0",
						GetCurrentUserID(), id);
					// (c.haag 2008-06-11 14:55) - PLID 28474 - Also delete todo alarms
					TodoDelete(FormatString("RegardingType = %d AND RegardingID = %d", ttInvOrder, id));
					ExecuteSql ("UPDATE OrderDetailsT SET Deleted = 1 WHERE OrderID = %i", 
						id);
					//TES 7/22/2008 - PLID 30802 - Clear out any allocation details that happen to have been linked to the details 
					// we just deleted
					ExecuteSql("UPDATE PatientInvAllocationDetailsT SET OrderDetailID = NULL WHERE OrderDetailID IN "
						"(SELECT ID FROM OrderDetailsT WHERE OrderID = %i)", id);
					ExecuteSql ("UPDATE OrderT SET Deleted = 1 WHERE ID = %i", id);
					// (j.jones 2008-03-18 15:38) - PLID 29309 - delete from OrderAppointmentsT
					// (j.jones 2008-03-19 12:21) - PLID 29316 - no need to audit the link deletion when the order is deleted
					ExecuteParamSql("DELETE FROM OrderAppointmentsT WHERE OrderID = {INT}", id);
					// (c.haag 2007-12-11 11:52) - PLID 28264 - Delete from the status history table
					//TES 6/18/2008 - PLID 29578 - ProductItemsT now has an OrderDetailID rather than an OrderID
					ExecuteSql("DELETE FROM ProductItemsStatusHistoryT WHERE ProductItemID IN (SELECT ID FROM ProductItemsT WHERE OrderDetailID IN (SELECT ID FROM OrderDetailsT WHERE OrderID = %d))", id);
					// (j.jones 2008-06-06 10:00) - PLID 27110 - delete referencing product items first
					// (j.jones 2008-09-10 14:03) - PLID 31320 - Scratch that, we should not try to delete products items
					// that have references, and we should catch it earlier in this code. Do not try to delete here.
					//ExecuteParamSql("DELETE FROM ProductItemsT WHERE ReturnedFrom IN (SELECT ID FROM ProductItemsT WHERE OrderDetailID IN (SELECT ID FROM OrderDetailsT WHERE OrderID = %li))", id);
					ExecuteSql ("DELETE FROM ProductItemsT WHERE OrderDetailID IN (SELECT ID FROM OrderDetailsT WHERE OrderID = %li)", id);
					trans.Commit();

					m_list->RemoveRow(m_rightClicked);

					long AuditID = -1;
					AuditID = BeginNewAuditEvent();
					if(AuditID != -1) {

						_RecordsetPtr rs = CreateRecordset("SELECT Description, Date, Company FROM OrderT "
						"LEFT JOIN PersonT ON OrderT.Supplier = PersonT.ID WHERE OrderT.ID = %li", id);
						
						if(!rs->eof) {
							//(e.lally 2006-11-13) PLID 23438 - Orders with no supplier will have a null value that must be accounted for.
							//The interface prevents new data from having no supplier, but the tables do not.
							CString strDesc;
							strDesc.Format("Order: %s, Supplier: %s",AdoFldString(rs, "Description"),
								AdoFldString(rs, "Company", "<None>"));

							AuditEvent(-1, AdoFldString(rs, "Description"),AuditID,aeiOrderDeleted,id,strDesc,"<Deleted>",aepHigh,aetDeleted);
						
						}
						rs->Close();
					}

					// (c.haag 2007-11-14 11:09) - PLID 28094 - Fire a table checker
					CClient::RefreshTable(NetUtils::OrderT, id);

					// (j.jones 2008-03-24 15:25) - PLID 29388 - need to update the linked appointment
					if(nAppointmentID != -1) {
						TrySendAppointmentTablecheckerForInventory(nAppointmentID, FALSE);
					}

					break;
				}	NxCatchAll("Could not delete item");
			}

			}NxCatchAll("Error in CInvOrderDlg::OnCommand : IDM_DELETE")
			break;
		}

		case IDM_RECEIVE:
			//DRT 12/4/2007 - PLID 28235
			ReceiveCurrentRow();
			break;
		case IDM_PRINT_LABEL:
			// (s.dhole 2010-07-16 16:47) - PLID 28183 It would be nice to be able to print Avery 8167 labels from the order screen by right clicking print, and then have it come up with an option to select which products need labels, and to print the labels quantity based on the # received.
			PrintOrderLabels(FALSE);
			break;
		case IDM_PRINT_FRAMES_LABEL:
			// (b.spivey, October 21, 2011) - PLID 46073 - Frames labels report. 
			PrintOrderLabels(TRUE);
			break; 
		// (c.haag 2008-05-21 15:44) - PLID 30082 - We no longer support unreceiving orders
		/*case IDM_UNRECEIVE:
			if (!CheckCurrentUserPermissions(bioInvOrderReceived, sptWrite))
				break;

			id = m_list->Value[m_rightClicked][iocID].lVal;
			try
			{				
				if(m_rightClicked == -1)
					break;	//error case
				
				//DRT 10/2/03 - PLID 9467 - They cannot delete an order that has serialized items that have been adjusted
				//off (marked 'deleted')
				//JMJ 1/27/04 - PLID 10465 - They cannot delete an order that has serialized items that have been billed
				// (j.jones 2007-11-07 10:53) - PLID 27987 - disallow deleting an order that has serialized items that
				// are allocated, even if the allocation is deleted
				//(also I combined these three queries into one)
				if(ReturnsRecords("SELECT ID FROM ProductItemsT WHERE OrderID = %li AND "
					"(Deleted = 1 "
					" OR ID IN (SELECT ProductItemID FROM ChargedProductItemsT) "
					" OR ID IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT WHERE ProductItemID Is Not Null)"
					")", id)) {
					MsgBox("There are items on this order that have either been adjusted, billed, or allocated to patients.\n"
						"You cannot delete this order.");
					break;
				}

				ExecuteSql("UPDATE OrderDetailsT SET DateReceived = NULL "
					"WHERE OrderID = %i AND Deleted = 0 AND DateReceived IS NOT NULL", 
					id);

				// (c.haag 2007-12-11 11:52) - PLID 28264 - Delete from the status history table
				ExecuteSql("DELETE FROM ProductItemsStatusHistoryT WHERE ProductItemID IN (SELECT ID FROM ProductItemsT WHERE OrderID = %d)", id);
				ExecuteSql ("DELETE FROM ProductItemsT WHERE OrderID = %li", id);

				//TODO: should the Last Cost be updated to the previous cost? Ewww.

				//auditing
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1)
					AuditEvent(CString(m_list->GetValue(m_list->CurSel, iocDescription).bstrVal), nAuditID, aeiOrderReceived, VarLong(m_list->GetValue(m_list->CurSel, 0)), "Received", "UnReceived", aepMedium, aetChanged);

				if(m_received.GetCheck())
					m_list->RemoveRow(m_rightClicked);
				else
					RefreshOrders();

				// (c.haag 2007-11-14 11:07) - PLID 28094 - Fire a table checker for the order record
				CClient::RefreshTable(NetUtils::OrderT, id);

				// (j.jones 2008-03-24 15:25) - PLID 29388 - need to update the linked appointment
				//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
				if(BetaHasAdvInventory()) {
					_RecordsetPtr rs = CreateParamRecordset("SELECT AppointmentID "
						"FROM OrderAppointmentsT "
						"WHERE OrderID = {INT}", id);
					if(!rs->eof) {
						long nAppointmentID = AdoFldLong(rs, "AppointmentID",-1);
						if(nAppointmentID != -1) {
							TrySendAppointmentTablecheckerForInventory(nAppointmentID, FALSE);
						}
					}
					rs->Close();
				}

			}	NxCatchAll("Could not mark received");
			break;*/

	}	
	return CNxDialog::OnCommand(wParam, lParam);
}

// (s.dhole 2010-07-16 16:47) - PLID 28183 It would be nice to be able to print Avery 8167 labels from the order screen by right clicking print, and then have it come up with an option to select which products need labels, and to print the labels quantity based on the # received.
// (b.spivey, October 21, 2011) - PLID 46073 - Updated function to use a bool to decide which version of the report. 
void CInvOrderDlg::PrintOrderLabels(BOOL bIsFramesLabel)
{
	try	{
		int id = m_list->Value[m_rightClicked][iocID].lVal;
		CInvLabelReportDlg  dlg(this);
		dlg.SetOrderID(id);
		dlg.SetIsFramesLabel(bIsFramesLabel); 
		dlg.DoModal();
	} NxCatchAll("Error in PrintOrderLabels");
}
//DRT 12/4/2007 - PLID 28235 - Pulled this out of OnCommand, IDM_RECEIVE.  It's the same code, but now
//	you can specify that it came from a barcode scan, and that will be passed into the dialog.
//Note that due to the way this works, you must set m_rightClicked before calling this function.
void CInvOrderDlg::ReceiveCurrentRow(BSTR bstr /*= NULL*/)
{
	try	{
		if (!CheckCurrentUserPermissions(bioInvOrderReceived, sptWrite))
			return;

		int id = m_list->Value[m_rightClicked][iocID].lVal;

		//Before we do anything, first run through each item and ensure that its UseUU status and Conversion ratio
		//is identical to the item's current settings. If not, ask the user how to proceed.
		if(!ValidateUU_UOStatus(id)) {
			return;
		}


		//
		//DRT 11/8/2007 - PLID 19682 - When receiving an order, we now want to pop up the receive order dialog.  This will allow
		//	users to more easily barcode scan what was received.  This dialog allows partial receiving, which the old behavior
		//	did not allow here (you had to edit the order).
		//
		CReceiveOrderDlg dlg(this);
		dlg.SetOrderID(id);
		dlg.m_bstrFromBarcode = bstr;
		if(dlg.DoModal() == IDOK) {
			//Success!  Either partially or fully received.
			RefreshOrders();
		}
		else {
			//Failed to receive the order, or user cancelled.  We don't want to do a thing.
		}
	} NxCatchAll("Error in ReceiveCurrentRow");
}

void CInvOrderDlg::OnNeed() 
{
	try {
		
		CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(180)]);
		//filter the report on only the current location
		infReport.nLocation = GetCurrentLocationID();

		CPtrArray paParams;
		CRParameterInfo *paramInfo;		
		paramInfo = new CRParameterInfo;
		
		CString tmp;

		// (j.jones 2008-02-14 14:03) - PLID 28864 - sent the ordering preference to the report,
		// so it can have a note that tells the user how the "needs ordered" conclusion was made
		long nOption = GetRemotePropertyInt("InvItem_OrderByOnHandAmount", 0, 0, "<None>", true);

		//0 = we compared "Actual" stock to the Reorderpoint
		//1 = we compared the "Available" stock to the Reorderpoint
		tmp.Format("%li", nOption);
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = "OrderRule";
		paParams.Add(paramInfo);

		//Made new function for running reports - JMM 5-28-04
		RunReport(&infReport, &paParams, true, (CWnd *)this, "Inventory Items To By Ordered");
		ClearRPIParameterList(&paParams);
		
	}NxCatchAll("Error running report.");
}

void CInvOrderDlg::OnDblClickCellOrderList(long nRowIndex, short nColIndex) 
{	
	BOOL bCanEdit;
	if(m_list->GetValue(nRowIndex, iocDateArrived).vt == VT_NULL) {
		bCanEdit = CheckCurrentUserPermissions(bioInvOrder, sptWrite);
	}
	else {
		bCanEdit = CheckCurrentUserPermissions(bioInvOrderReceived, sptWrite);
	}

	if (bCanEdit)
	{	if (nRowIndex != -1)
		{	if (!m_pEditOrderDlg)
			{	m_pEditOrderDlg = new CInvEditOrderDlg(this);
				m_pEditOrderDlg->Create(IDD_INVEDITORDER);
			}
			m_pEditOrderDlg->DoFakeModal(m_list->GetValue(nRowIndex, iocID).lVal, FALSE);
		}
	}
}

/*BOOL CInvOrderDlg::ReceiveProductItems(long OrderID)
{
	try {

		//TODO: why not have it so if they add all the serial numbers for one item,
		//but not another item, it will save and mark that product received but not the cancelled one?

		//TES 6/18/2008 - PLID 29578 - ProductItemsT now has an OrderDetailID rather than an OrderID
		_RecordsetPtr rs = CreateRecordset("SELECT ProductID, "
			"QuantityOrdered AS QuantityOrdered, "
			"HasSerialNum, HasExpDate, OrderDetailsT.Conversion, OrderDetailsT.UseUU, SerialPerUO, "
			"OrderDetailsT.ID AS OrderDetailID FROM OrderDetailsT "
			"INNER JOIN ProductT ON OrderDetailsT.ProductID = ProductT.ID WHERE OrderID = %li AND (HasSerialNum = 1 OR HasExpDate = 1) AND DateReceived Is Null",OrderID);

		if(rs->eof)
			//if no applicable products, return true and move on
			return TRUE;

		//loop through all products on this order with one of these properties
		while(!rs->eof) {
			
			BOOL bHasSerial, bHasExpDate, bUseUU, bSerialPerUO;
			long ProductID, QtyOrdered, Conversion;

			bHasSerial = AdoFldBool(rs, "HasSerialNum",FALSE);
			bHasExpDate = AdoFldBool(rs, "HasExpDate",FALSE);
			bUseUU = AdoFldBool(rs, "UseUU",FALSE);
			bSerialPerUO = AdoFldBool(rs, "SerialPerUO",FALSE);
			ProductID = AdoFldLong(rs, "ProductID");
			QtyOrdered = AdoFldLong(rs, "QuantityOrdered",0);
			Conversion = AdoFldLong(rs, "Conversion",1);

			//TES 6/18/2008 - PLID 29578 - Pull the detail out of our recordset.
			long nOrderDetailID = AdoFldLong(rs, "OrderDetailID");

			CProductItemsDlg& dlg = GetMainFrame()->GetProductItemsDlg();
			//DRT 11/15/2007 - PLID 28008 - All ENTER_DATA types must not allow change of qty
			dlg.m_bDisallowQtyChange = TRUE;
			dlg.m_bUseSerial = bHasSerial;
			dlg.m_bUseExpDate = bHasExpDate;
			dlg.m_ProductID = ProductID;
			//TES 6/18/2008 - PLID 29578 - This now takes an OrderDetailID rather than an OrderID
			dlg.m_nOrderDetailID = nOrderDetailID;
			dlg.m_NewItemCount = QtyOrdered;
			dlg.m_bUseUU = bUseUU;
			dlg.m_bSerialPerUO = bSerialPerUO;
			dlg.m_nConversion = Conversion;

			if(IDCANCEL == dlg.DoModal())
				return FALSE;

			rs->MoveNext();
		}

		rs->Close();

		return TRUE;

	}NxCatchAll("Error in ReceiveProductItems");

	return FALSE;
}*/

void CInvOrderDlg::OnSelChosenSupplierFilter(long nRow) 
{
	try {

		if(nRow == -1)
			//select "all suppliers"
			m_SupplierFilter->SetSelByColumn(0,(long)-1);

		SetWhereClause();

	}NxCatchAll("Error selecting supplier.");
}

HBRUSH CInvOrderDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetExStyle() & WS_EX_TRANSPARENT) {
		extern CPracticeApp theApp;
		pDC->SelectPalette(&theApp.m_palette, FALSE);
		pDC->RealizePalette();
		pDC->SetBkColor(PaletteColor(0x00FFDBDB));
		return m_brush;
	} else {
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	*/
	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CInvOrderDlg::ValidateUU_UOStatus(long nOrderID)
{
	//Run through each item and ensure that its UseUU status and Conversion ratio
	//is identical to the item's current settings. If not, ask the user how to proceed.

	// (j.jones 2008-04-01 11:57) - PLID 28430 - if the conversion changed but the
	// OrderDetailsT.UseUU is not enabled, we need not warn
	_RecordsetPtr rs = CreateRecordset("SELECT OrderDetailsT.ID, ServiceT.Name, "
		"OrderDetailsT.UseUU AS OrderUseUU, OrderDetailsT.Conversion AS OrderConversion, "
		"ProductT.UseUU AS ProductUseUU, ProductT.Conversion AS ProductConversion "
		"FROM OrderDetailsT "
		"INNER JOIN ProductT ON OrderDetailsT.ProductID = ProductT.ID "
		"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
		"WHERE OrderID = %li AND Deleted = 0 AND DateReceived Is Null "
		"AND ((OrderDetailsT.UseUU <> ProductT.UseUU) OR (OrderDetailsT.UseUU <> 0 AND OrderDetailsT.Conversion <> ProductT.Conversion))", nOrderID);

	BOOL bChanged = FALSE;

	while(!rs->eof) {
		//for each item returned, there's a difference, so inform the user of the difference,
		//and give the option to use the old data, the new data, or cancel receiving altogether
		long nOrderDetailID = AdoFldLong(rs, "ID");
		CString strName = AdoFldString(rs, "Name","");
		BOOL bOrderUseUU = AdoFldBool(rs, "OrderUseUU",FALSE);
		BOOL bProductUseUU = AdoFldBool(rs, "ProductUseUU",FALSE);
		long nOrderConversion = AdoFldLong(rs, "OrderConversion",1);
		long nProductConversion = AdoFldLong(rs, "ProductConversion",1);

		CString strWarning, strOrder1, strOrder2, strProduct1, strProduct2;

		if(bOrderUseUU) {
			strOrder1.Format("enabled with a Conversion rate of %li",nOrderConversion);
			strOrder2.Format("a Conversion rate of %li",nOrderConversion);
		}
		else { 
			strOrder1 = "disabled";
			strOrder2 = "no UU/UO conversion";
		}

		if(bProductUseUU) {
			strProduct1.Format("enabled with a Conversion rate of %li",nProductConversion);
			strProduct2.Format("a Conversion rate of %li",nProductConversion);
		}
		else {
			strProduct1 = "disabled";
			strProduct2 = "no UU/UO conversion";
		}

		if(!bOrderUseUU && !bProductUseUU) {
			//no need to warn
			return TRUE;
		}

		CString strWarning2 = "";
		if(bChanged) {
			//inform them that cancelling will not undo previous changes in this loop
			strWarning2 = "(Conversion rate changes to previous products in this Order will be committed.)";
		}

		strWarning.Format("The product '%s' was ordered with the 'Use Unit Of Order / Unit Of Usage' tracking %s,\n"
			"but now currently has this tracking %s.\n\n"
			"Do you wish to receive this product with the configuration in which it was ordered?\n\n"
			"'Yes' will receive the product with %s.\n"
			"'No' will receive the product with %s.\n"
			"'Cancel' will cancel marking this order received entirely. %s",strName,strOrder1,strProduct1,strOrder2,strProduct2,strWarning2);

		int nResult = MessageBox(strWarning,"Practice",MB_ICONEXCLAMATION|MB_YESNOCANCEL);
		if(nResult == IDCANCEL) {
			return FALSE;
		}
		else if(nResult == IDNO) {
			//if 'No', they want to receive the order with the current configuration
			bChanged = TRUE;
			ExecuteSql("UPDATE OrderDetailsT SET UseUU = %li, Conversion = %li WHERE ID = %li",bProductUseUU ? 1 : 0, nProductConversion, nOrderDetailID);

			// (c.haag 2007-11-14 11:07) - PLID 28094 - Fire a table checker for the order record
			CClient::RefreshTable(NetUtils::OrderT, nOrderID);

			// (j.jones 2008-03-24 15:25) - PLID 29388 - we do NOT need to send table checkers for a linked appointment
			// here, because the changes here have no bearing on the appt.
		}

		rs->MoveNext();
	}
	rs->Close();

	return TRUE;
}

void CInvOrderDlg::OnBtnNewReturn() 
{
	// (c.haag 2007-11-09 10:44) - PLID 27992 - This event occurs when the user wants to
	// create a new return
	try {
		// (c.haag 2007-11-13 17:39) - PLID 28036 - Check permissions
		if(!CheckCurrentUserPermissions(bioInvSupplierReturn, sptCreate)) {
			return;
		}

		CInvEditReturnDlg dlg(this);
		if (IDOK == dlg.DoModal()) {
			RefreshReturns();
		}
	} 
	NxCatchAll("Error in CInvOrderDlg::OnBtnNewReturn");
}

void CInvOrderDlg::OnDblClickCellReturnList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		// (c.haag 2007-11-09 10:45) - PLID 27992 - This event is fired when the user wants
		// to open an existing return
		if (NULL == lpRow) {
			return;
		}

		// Ensure the selection is set
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		m_listReturns->CurSel = pRow;

		// Now open the selected group
		OnOpenReturnGroup();
	}

	NxCatchAll("Error in CInvOrderDlg::OnDblClickCellReturnList");
}

void CInvOrderDlg::OnSelChosenSupplierFilterReturn(LPDISPATCH lpRow) 
{
	try {
		// (c.haag 2007-11-09 11:32) - PLID 27992 - Update the return list filter
		if (NULL == lpRow) {
			m_SupplierFilterReturns->SetSelByColumn(eclSup_ID, -1L);
		}
		SetWhereClause_Returns();
	} NxCatchAll("Error in CInvOrderDlg::OnSelChosenSupplierFilterReturn");
}

void CInvOrderDlg::OnRButtonDownReturnList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		// (c.haag 2007-11-09 15:05) - PLID 27992 - Let the user do stuff with supplier returns
		if (NULL == lpRow) {
			return;
		}

		// Ensure the selection is set
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		m_listReturns->CurSel = pRow;

		// Bring up the pop-up menu
		CMenu menu;
		CPoint pt;
		GetCursorPos(&pt);
		menu.m_hMenu = CreatePopupMenu();
		menu.InsertMenu(-1, MF_BYPOSITION, IDM_OPEN_RETURN_GROUP, "&Open Return");
		menu.InsertMenu(-1, MF_BYPOSITION, IDM_DELETE_RETURN_GROUP, "&Delete Return");
		menu.SetDefaultItem(IDM_OPEN_RETURN_GROUP, FALSE);
		menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		menu.DestroyMenu();
	}
	NxCatchAll("Error in CInvOrderDlg::OnRButtonDownReturnList");	
}

void CInvOrderDlg::OnOpenReturnGroup()
{
	try {
		// (c.haag 2007-11-09 16:43) - PLID 27992 - Open the selected return group
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_listReturns->CurSel);
		if (NULL == pRow) {
			return;
		}

		// (c.haag 2007-11-13 17:38) - PLID 28036 - Check permissions
		if(!CheckCurrentUserPermissions(bioInvSupplierReturn, sptWrite)) {
			return;
		}

		CInvEditReturnDlg dlg(this);
		long nReturnGroupID = VarLong(pRow->GetValue(eclRet_ReturnGroupID));
		dlg.SetLoadID(nReturnGroupID);
		if (IDOK == dlg.DoModal()) {
			RefreshReturns();
		}
	}
	NxCatchAll("Error in CInvOrderDlg::OnOpenReturnGroup");
}

void CInvOrderDlg::OnDeleteReturnGroup()
{
	try {
		// (c.haag 2007-11-09 16:43) - PLID 27992 - Delete the selected return group
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_listReturns->CurSel);
		if (NULL == pRow) {
			return;
		}

		// (c.haag 2007-11-13 17:38) - PLID 28036 - Check permissions
		if(!CheckCurrentUserPermissions(bioInvSupplierReturn, sptDelete)) {
			return;
		}

		long nReturnGroupID = VarLong(pRow->GetValue(eclRet_ReturnGroupID));
		if (InvUtils::DeleteReturnGroup(nReturnGroupID, TRUE /* Ask the user if they're sure */, this /* Message boxes appear above this window */)) {
			RefreshReturns();
		}
	}
	NxCatchAll("Error in CInvOrderDlg::OnDeleteReturnGroup");
}

LRESULT CInvOrderDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		// (c.haag 2007-11-14 09:47) - PLID 27992 - Table checker support
		// for supplier returns. Because we don't use extended table checkers
		// (which is a lot of work for almost no noticeable benefit), we always
		// have to query data to see how a return item has changed. And, since
		// we're querying data, we may as well query it once; call a requery on
		// the return list.
		if (wParam == NetUtils::SupplierReturnGroupsT) {
			RefreshReturns();
		}
		// (c.haag 2007-11-14 11:20) - PLID 28094 - Table checker support
		// for inventory order changes. As with the reasoning above, we will just
		// do one requery.
		else if (wParam == NetUtils::OrderT) {
			RefreshOrders();
		}
		// (c.haag 2007-11-16 12:47) - PLID 27992 - Listen for changes to suppliers.
		// If anything changes, refresh all supplier combos and lists
		else if (wParam == NetUtils::Suppliers) {

			// (s.tullis 2014-08-12 14:09) - PLID 63241 - The InvOrderDlg needs a CTableChecker object for Suppliers.
			EnsureUpdatedSuppliers();
			
		}
	} 
	NxCatchAll("Error in CInvOrderDlg::OnTableChanged");
	return 0;
}

void CInvOrderDlg::OnTrySetSelFinishedSupplierFilter(long nRowEnum, long nFlags) 
{
	try {
		// (c.haag 2007-11-16 13:05) - PLID 27992 - This is fired after we get a table checker
		// and try to update the selection of the order supplier dropdown
		RefreshOrders();
	
	} NxCatchAll("Error in CInvOrderDlg::OnTrySetSelFinishedSupplierFilter");
}

void CInvOrderDlg::OnTrySetSelFinishedSupplierFilterReturn(long nRowEnum, long nFlags) 
{
	try {
		// (c.haag 2007-11-16 13:06) - PLID 27992 - This is fired after we get a table checker
		// and try to update the selection of the return supplier dropdown
		RefreshReturns();

	} NxCatchAll("Error in CInvOrderDlg::OnTrySetSelFinishedSupplierFilterReturn");
	
}

//DRT 11/29/2007 - PLID 28235 - Added barcoding
LRESULT CInvOrderDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {
		_bstr_t bstr = (BSTR)lParam;
		CString strCode((LPCTSTR)bstr);

		//If the user scanned a barcode here, what could they possibly want?  We're going to guess and assume they are
		//	receiving an order.  For simplicity, we are only going to look at pending orders.  If they scan something
		//	that is already received, it's ignored for these purposes.
		//Note that we group by OrderT.ID, because the product can exist multiple times on the same order.  We would want
		//	those to pop up.
		//(c.copits 2010-09-24) PLID 40317 - Allow duplicate UPC codes for FramesData certification
		_RecordsetPtr prs = GetBestUPCProduct(strCode);
		//_RecordsetPtr prs = CreateParamRecordset("SELECT OrderT.ID "
		//	"FROM ServiceT "
		//	"LEFT JOIN OrderDetailsT ON ServiceT.ID = OrderDetailsT.ProductID "
		//	"LEFT JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
		//	"WHERE ServiceT.Barcode = {STRING} AND OrderDetailsT.Deleted = 0 AND OrderT.Deleted = 0 AND OrderDetailsT.DateReceived IS NULL "
		//	"GROUP BY OrderT.ID", strCode);
		if(!prs->eof) {
			//There are some pending orders for the product... how many?  If there are more than 1, we have no choice but to force the user to choose
			//	manually.
			if(prs->GetRecordCount() > 1) {
				AfxMessageBox("There are multiple orders with the product you have scanned.  Please open the correct order that you are receiving, then scan the product again.");
			}
			else {
				//There is 1 and only 1 order, so create it
				long nOrderID = AdoFldLong(prs, "ID");

				//We'll use existing functionality, so to do that, highlight the row and pretend it was double clicked on
				long nRow = m_list->FindByColumn(iocID, (long)nOrderID, 0, VARIANT_TRUE);
				if(nRow > sriNoRow) {
					//If the row was found, fake the right click / mark received
					m_rightClicked = nRow;
					//DRT 12/4/2007 - PLID 28235 - Pass the barcode through to the receive so the dialog that results can have it ready.
					ReceiveCurrentRow((BSTR)lParam);
				}
				else {
					//The row was not found.  The user must be filtering outside of our current list!  This could happen if they were
					//	filtering on another supplier, for example.
					//We will just inform the user in this case.  This should happen very, very rarely, and they'll need to expand their
					//	view to see this order.  It may be confusing otherwise to have orders pop up when you're filtering on a different
					//	supplier, etc.
					AfxMessageBox("An order was found for the barcode you have scanned, but that order is not in the current view.  Please change "
						"your filters so that all orders can be seen and scan the barcode again.");
				}
			}
		}
		else {
			//Part (b).  There are no orders for this particular product.  In this case, we assume that the user wants to create an order for
			//	this product.
			// (c.haag 2007-12-05 14:52) - PLID 28286 - Sometimes users will get product items without having entered
			// an actual order into the system. That means if the user scans any serialized products followed by unrecognized
			// serial numbers for product items, we presume they should be added to inventory immediately. So, call CreateOrder
			// with a value of TRUE.
			// (c.haag 2007-12-17 12:45) - PLID 28286 - For this release, we've decided that any barcode scan from the order tab that does
			// not pertain to an existing order should be treated as an effort to create an order for already-arrived items.
			CreateOrder(TRUE);
			//And then simulate this barcode being scanned there
			if(m_pEditOrderDlg && m_pEditOrderDlg->IsWindowVisible()) {
				m_pEditOrderDlg->SendMessage(WM_BARCODE_SCAN, wParam, lParam);
			}
			else {
				//I don't think this should be possible.  Maybe if you VERY quickly hit escape or cancel on the dialog?
				ASSERT(FALSE);
			}
		}

	} NxCatchAll("Error in OnBarcodeScan");

	return 0;
}

void CInvOrderDlg::OnToBeReturned() 
{
	try {
		//TES 6/23/2008 - PLID 26152 - Check their permission to create returns now; this dialog has no purpose if you can't
		// create a return.
		if(!CheckCurrentUserPermissions(bioInvSupplierReturn, sptCreate)) {
			return;
		}

		//TES 6/23/2008 - PLID 26152 - Just open up the dialog.
		CProductsToBeReturnedDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll("Error in CInvOrderDlg::OnToBeReturned()");
}

// (j.gruber 2008-06-30 15:00) - PLID 29205 - added created date and received date filters
BOOL CInvOrderDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		GetDlgItem(IDC_INV_ORDER_DATE_LABEL)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt) && IsDlgButtonChecked(IDC_INV_ORDER_USE_DATE)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}NxCatchAll("Error in OnSetCursor");
	
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}


LRESULT CInvOrderDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_INV_ORDER_DATE_LABEL:
			{
				CInvOrderDateFilterPickerDlg dlg(m_nDateFilterID);
				dlg.SetFilterID(m_nDateFilterID);
				long nResult = dlg.DoModal();				
				if (nResult == IDOK) {

					m_nDateFilterID = dlg.GetFilterID();
					if (m_nDateFilterID == DATE_CREATED) {
						m_nxlDateLabel.SetText("Created Date");
					}
					else if (m_nDateFilterID == DATE_RECEIVED) {
						m_nxlDateLabel.SetText("Received Date");
					}

					SetWhereClause();
				}
			}
		break;
		
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in OnLabelClick");
	return 0;
}

// (j.gruber 2008-06-30 11:21) - PLID 30564 - changed the radio buttons to drop downs
void CInvOrderDlg::OnSelChangingInvOrderReturnFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll("Error in CInvOrderDlg::OnSelChangingInvOrderReturnFilter");
	
}

void CInvOrderDlg::OnSelChosenInvOrderReturnFilter(LPDISPATCH lpRow) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			m_nReturnTypeFilterID = VarLong(pRow->GetValue(rtfcID));
			SetWhereClause_Returns();
		}

	}NxCatchAll("Error in CInvOrderDlg::OnSelChosenInvOrderReturnFilter");
	
}

void CInvOrderDlg::OnInvOrderUseDate() 
{
	
	try { 
		if (IsDlgButtonChecked(IDC_INV_ORDER_USE_DATE)) {

			//enable the dates and refresh the datalist
			m_nxlDateLabel.EnableWindow(TRUE);
			m_nxlDateLabel.SetType(dtsHyperlink);

			//not sure why, but its overwriting itself, so I had to redo the text
			if (m_nDateFilterID == DATE_CREATED) {
				m_nxlDateLabel.SetText("Created Date");
			}
			else if (m_nDateFilterID == DATE_RECEIVED) {
				m_nxlDateLabel.SetText("Received Date");
			}
			else {
				ASSERT(FALSE);
				m_nxlDateLabel.SetText("");
			}
			m_dtTo.EnableWindow(TRUE);
			m_dtFrom.EnableWindow(TRUE);
			SetWhereClause();
		}
		else {

			//disable all the windows and refresh
			m_nxlDateLabel.EnableWindow(FALSE);
			m_nxlDateLabel.SetType(dtsDisabledHyperlink);
			//not sure why, but its overwriting itself, so I had to redo the text
			if (m_nDateFilterID == DATE_CREATED) {
				m_nxlDateLabel.SetText("Created Date");
			}
			else if (m_nDateFilterID == DATE_RECEIVED) {
				m_nxlDateLabel.SetText("Received Date");
			}
			else {
				ASSERT(FALSE);
				m_nxlDateLabel.SetText("");
			}

			m_dtTo.EnableWindow(FALSE);
			m_dtFrom.EnableWindow(FALSE);
			SetWhereClause();
		}
			
	}NxCatchAll("Error in CInvOrderDlg::OnInvOrderUseDate");
	
}

// (j.gruber 2008-06-30 11:23) - PLID 30564 - changed radios to drop downs
void CInvOrderDlg::OnSelChangingInvOrderOrderFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll("Error in CInvOrderDlg::OnSelChangingInvOrderOrderFilter");
	
	
}

void CInvOrderDlg::OnSelChosenInvOrderOrderFilter(LPDISPATCH lpRow) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			m_nOrderTypeFilterID = VarLong(pRow->GetValue(otfcID));
			SetWhereClause();
		}

	}NxCatchAll("Error in CInvOrderDlg::OnSelChosenInvOrderOrderFilter");
	
}

BOOL CInvOrderDlg::CheckDates() {


	try {
		COleDateTime dtTo, dtFrom;

		dtTo = m_dtTo.GetDateTime();
		dtFrom = m_dtFrom.GetDateTime();
		

		if (dtTo < dtFrom) {
			MsgBox("The To date must be after the From date.");
			return FALSE;
		}
		
		if (dtFrom.GetStatus() != COleDateTime::valid) {
			MsgBox("Please enter a valid From date.");
			return FALSE;
		}

		if (dtFrom.GetStatus() != COleDateTime::valid) {
			MsgBox("Please enter a valid To date.");
			return FALSE;
		}

		return TRUE;

	}NxCatchAll("Error in CInvOrderDlg::CheckDates");

	return FALSE;
}

void CInvOrderDlg::OnDatetimechangeInvOrderToDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try {
		if (CheckDates()) {
			SetWhereClause();
			m_dtToDate = m_dtTo.GetDateTime();
		}
		else {
			//set it back
			m_dtTo.SetValue(m_dtToDate);
		}
	}NxCatchAll("Error in CInvOrderDlg::OnDatetimechangeInvOrderToDate");
	
	*pResult = 0;
}

void CInvOrderDlg::OnDatetimechangeInvOrderFromDate(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try { 
		if (CheckDates()) {
			SetWhereClause();
			m_dtFromDate = m_dtFrom.GetDateTime();
		}
		else {
			//set it back
			m_dtFrom.SetValue(m_dtFromDate);
		}
	}NxCatchAll("Error in CInvOrderDlg::OnDatetimechangeInvOrderFromDate");
	
	*pResult = 0;
}

void CInvOrderDlg::OnToBeOrdered() 
{
	try {
		//TES 7/22/2008 - PLID 30802 - Check their permission to create returns now; this dialog has no purpose if you can't
		// create a return.
		if(!CheckCurrentUserPermissions(bioInvOrder, sptCreate)) {
			return;
		}

		//TES 7/22/2008 - PLID 30802 - Just open up the dialog.
		CProductsToBeOrderedDlg dlg(this);
		dlg.m_pInvOrderDlg = this;
		dlg.DoModal();

	}NxCatchAll("Error in CInvOrderDlg::OnToBeReturned()");
}

//TES 7/22/2008 - PLID 30802 - Provide access to our CInvEditOrderDlg.
CInvEditOrderDlg* CInvOrderDlg::GetEditOrderDlg()
{
	if (!m_pEditOrderDlg)
	{	m_pEditOrderDlg = new CInvEditOrderDlg(this);
		m_pEditOrderDlg->Create(IDD_INVEDITORDER);
	}
	return m_pEditOrderDlg;
}

// (j.jones 2009-03-17 13:32) - PLID 32831 - added support for supplier statements
void CInvOrderDlg::OnBtnSupplierStatements()
{
	try {

		//ON HOLD - not currently released
		/*
		if(!CheckCurrentUserPermissions(bioSupplierStatements, sptWrite)) {
			return;
		}

		CSupplierStatementDlg dlg;
		dlg.DoModal();
		*/

	}NxCatchAll("Error in CInvOrderDlg::OnBtnSupplierStatements");
}

// (j.jones 2009-03-17 13:38) - PLID 32832 - added support for reconciling consignment
void CInvOrderDlg::OnBtnReconcileConsignment()
{
	try {

		if(!CheckCurrentUserPermissions(bioReconcileConsignment, sptWrite)) {
			return;
		}

		CConsignmentReconcileDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll("Error in CInvOrderDlg::OnBtnReconcileConsignment");
}

// (c.haag 2010-01-14 10:25) - PLID 30503 - Fired when the user checks the Show P.O. Number checkbox
void CInvOrderDlg::OnBnClickedCheckShowponumber()
{
	try {
		SetRemotePropertyInt("InvOrder_ShowPOBoxColumn", m_chkShowPONumber.GetCheck() ? 1 : 0, 0, GetCurrentUserName());
		ReflectColumnAppearances();
	}
	NxCatchAll(__FUNCTION__);
}


// (c.haag 2010-01-14 10:16) - PLID 30503 - This utility function will show or hide columns based on preference.
void CInvOrderDlg::ReflectColumnAppearances()
{
	if (m_chkShowPONumber.GetCheck()) {
		// Make sure the modified date and modified by fields have the csVisible flag
		IColumnSettingsPtr pCol;
		pCol = m_list->GetColumn(iocPOBox);
		pCol->PutColumnStyle(csVisible | csWidthData);
		pCol->PutStoredWidth(50);
	} 
	else {
		IColumnSettingsPtr pCol;
		pCol = m_list->GetColumn(iocPOBox);
		pCol->PutColumnStyle(csVisible | csFixedWidth);
		pCol->PutStoredWidth(0);
	}
}

//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
// This function will likely be updated to pick the most suitable
// UPC code in response to a barcode scan. Practice now allows multiple
// products to have the same UPC codes. Further, products can share UPC codes
// with service codes (however, service codes cannot share UPC codes).

// Current behavior: Returns first matching product.

_RecordsetPtr CInvOrderDlg::GetBestUPCProduct(CString strCode)
{
	_RecordsetPtr prs;

	try {

		prs = CreateParamRecordset("SELECT OrderT.ID "
					"FROM ServiceT "
					"LEFT JOIN OrderDetailsT ON ServiceT.ID = OrderDetailsT.ProductID "
					"LEFT JOIN OrderT ON OrderDetailsT.OrderID = OrderT.ID "
					"WHERE ServiceT.Barcode = {STRING} AND OrderDetailsT.Deleted = 0 AND OrderT.Deleted = 0 AND OrderDetailsT.DateReceived IS NULL "
					"GROUP BY OrderT.ID", strCode);


	} NxCatchAll(__FUNCTION__);

	return prs;
}

void CInvOrderDlg::OnBnClickedRecieveFrames()
{
	try{
		// r.wilson PLID 47393 2012-2-17
		CreateFramesOrder(TRUE,-1,-1);
	}NxCatchAll("Error in OnBnClickedRecieveFrames");
}

/* (r.wilson 2012-20-2) PLID 48222 
	This function will print an order given a Order Id
	Note: (The function that this is based on will print at the current selected position on the datalist) */
void CInvOrderDlg::PrintOrderLabels(BOOL bIsFramesLabel,long nOrderId)
{
	try	{
		CInvLabelReportDlg  dlg(this);
		dlg.SetOrderID(nOrderId);
		dlg.SetIsFramesLabel(bIsFramesLabel); 
		dlg.DoModal();
	} NxCatchAll("Error in PrintOrderLabels");
}


// (s.tullis 2014-08-12 14:09) - PLID 63241 - The InvOrderDlg needs a CTableChecker object for Suppliers.
void CInvOrderDlg::EnsureUpdatedSuppliers(){

	try{
	
		if (m_SuppliersChecker.Changed()){
			// Refresh the order supplier filter and try to reselect the original value
			long nOldSel = m_SupplierFilter->CurSel;
			IRowSettingsPtr pRow = (-1 == nOldSel) ? NULL : m_SupplierFilter->GetRow(nOldSel);
			nOldSel = (NULL == pRow) ? -1 : VarLong(pRow->GetValue(0));
			m_SupplierFilter->Requery();
			pRow = m_SupplierFilter->GetRow(-1);
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, _bstr_t("<All Suppliers>"));
			m_SupplierFilter->InsertRow(pRow, 0);
			if (NXDATALISTLib::sriNoRowYet_WillFireEvent != m_SupplierFilter->TrySetSelByColumn(0, nOldSel)) {
				RefreshOrders();
			}

			// Refresh the return supplier filter and try to reselect the original value
			NXDATALIST2Lib::IRowSettingsPtr pReturnRow = m_SupplierFilterReturns->CurSel;
			long nOldReturnSel = (NULL == pReturnRow) ? -1 : VarLong(pReturnRow->GetValue(eclSup_ID));
			m_SupplierFilterReturns->Requery();
			pReturnRow = m_SupplierFilterReturns->GetNewRow();
			pReturnRow->PutValue(eclSup_ID, -1L);
			pReturnRow->PutValue(eclSup_Name, _bstr_t("<All Suppliers>"));
			m_SupplierFilterReturns->AddRowBefore(pReturnRow, m_SupplierFilterReturns->GetFirstRow());
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			if (NXDATALIST2Lib::sriNoRowYet_WillFireEvent != m_SupplierFilterReturns->TrySetSelByColumn_Deprecated(eclSup_ID, nOldReturnSel)) {
				RefreshReturns();
			}

		}
	}NxCatchAll(__FUNCTION__)


}


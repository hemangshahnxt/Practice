// InvVisionWebDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "InvVisionWebDlg.h"
#include "InvVisionWebUtils.h"
#include "VisionWebOrderDlg.h"
#include "InventoryRc.h"
#include "VisionWebSetupDlg.h"
#include "InvVisionWebUtils.h"
#include "VisionWebSetupDlg.h"
#include "VisionWebOrderHistory.h"
#include "VisionWebServiceSetupDlg.h"
#include "InvGlassesCatalogSetupDlg.h"
#include "AuditTrail.h"
#include "InvGlassesOrderStatusDlg.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
#include "Reports.h"
#include "ContactLensOrderForm.h" // (j.dinatale 2012-03-02 17:04) - PLID 48527
#include "BillingModuleDlg.h"	// (j.dinatale 2012-03-15 14:53) - PLID 47413
#include "PatientView.h"	// (j.dinatale 2012-03-15 14:53) - PLID 47413
#include "FinancialDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;
// CInvVisionWebDlg dialog
// (s.dhole 2010-09-24 12:48) - PLID 40538 Create a VisionWeb tab in Inventory to track orders
 
// (s.dhole 2010-11-09 15:18) - PLID 41397 
UINT CheckingOrderStatusThread(LPVOID pParam);
HANDLE g_hStopCheckingOrderStatus = NULL;


// (s.dhole 2010-11-09 15:18) - PLID 41397 
//  (s.dhole 2010-11-09 15:18) - PLID 41397  - Thread data for launching the PopulateList thread
	// (s.dhole 2011-02-16 13:33) - PLID 41397 Added Cleanup function, for Was causing memeory leak
class CVisionWebOrderThreadData {
public:
	HWND m_hWnd;
	CArray<VisionWebOrderstruct*, VisionWebOrderstruct*> m_arynOrderIDs;
	long m_nUserID;
	CString m_strURL;
	CString m_strUserID;
	CString m_strUserPSW;
	CString m_strRefID;

	void CleanUp()
	{
		while(m_arynOrderIDs.GetSize() > 0)
		{
			if(m_arynOrderIDs.GetAt(0) != NULL) {
				delete m_arynOrderIDs.GetAt(0);
			}
			m_arynOrderIDs.RemoveAt(0);
		}
	}

};
//  (s.dhole 2010-11-09 15:18) - PLID 41397  - Thread data for launching the PopulateList thread
struct VisionWebOrderThreadstruct{
	public:
		int  nOrderId;
		CString   strVisionWebMessage;
		COleDateTime UpdateDate;
	};

//  (s.dhole 2010-11-09 15:18) - PLID 40538  Order table enum
// (s.dhole 2011-03-31 15:16) - PLID 43040
enum VisionWebOrderListColumns {
	vwolPatientName = 0,
	vwolPatientID,
	vwolOrderID,
	vwolVisionWebOrderNumber,
	vwolInvoiceNo,// (s.dhole 2012-05-08 09:49) - PLID 50131
	vwolOptician, // (j.dinatale 2012-04-19 09:14) - PLID 49758 - added Optician to the optical order tab
	vwolOrderStatusName,
	vwolOrderStatusID,
	vwolOrderTypeName,
	vwolOrderType,
	vwolCreateDate,
	vwolSubmitDate,
	vwolWebVisionSupplierName,
	vwolWebVisionSupplierID,
	vwolDescription,
	vwolMessage,
	vwolOrderTypeColor,
	vwolOrderStatusColor,
	vwolVisionWebExchangeID,
	vwolLastUpdatDate,
	vwolGlassesOrderProcessTypeID,// (s.dhole 2011-03-31 15:16) - PLID 43040
	vwolLensRxID,// (s.dhole 2011-05-04 17:22) - PLID 42953  LensRXID
	vwolLastApptDate, //(c.copits 2011-09-29) PLID 43742 - Add Last Appointment Date as a column on the Glasses Order tab
	vwolBeenBilled,	// (j.dinatale 2012-03-19 14:33) - PLID 48932 - Added a "Has been billed" column
};

//  (s.dhole 2010-11-09 15:18) - PLID 40538  Patient  dropdown  enum
enum VisionWebPatinetListColumns {
	vwpPatientID =0, 
	vwplast,
	vwpfirst, 
	vwpMiddle,
	vwpPatientName,
	vwpUserDefinedID, 
	vwpPatientHomePhone, 
	vwpPatientBirthDate, 
};

//  (s.dhole 2010-11-09 15:18) - PLID 40538  order status dropdown  enum
enum VisionWebOrderStatusColumns {
		vwosOrderStatusID = 0,
		vwosOrderStatusName,
		vwosOrderStatusOrder,
		vwosOrderStatusColor,
	};
//  (s.dhole 2010-11-09 15:18) - PLID 40538  Supplier  dropdown  enum
enum VisionWebSupplierColumns {
	vwsSupplierID =0 ,
	vwsSupplierName,
	vwsvisionwebSupplierCode,
};

//  (s.dhole 2010-11-09 15:18) - PLID 40538  Order type  dropdown  enum this box is hidden sincce there is only one item in dropdown for first client
// (j.dinatale 2012-05-02 14:29) - PLID 49758 - repurposed this dropdown
namespace VisionWebOpticianDD{
	enum VisionWebOpticianColumns {
		OpticianID = 0,
		OpticianName,
	};
};


//  (s.dhole 2010-11-09 15:18) - PLID 40538  date dropdown enum
enum VisionWebOrderDateFilterTypeColumns {
	vwdftcID = 0,
	vwddftcName = 1,
};

//  (s.dhole 2010-11-09 15:18) - PLID 41397   
enum VisionWebOrderStatusCheck {
	vwStatusCheckNone = 0,
	vwStatusCheckSuccessful = 2,
	vwStatusCheckCanceled = 3,
	vwStatusCheckError = 4,
};

// (j.dinatale 2012-05-02 14:29) - PLID 56214 - location dropdown
namespace VisionWebLocationDD{
	enum VisionWebLocationColumns {
		ID = 0,
		Name,
	};
};

long m_nOrderID=-1;
long m_nselectedPatient=-1;

IMPLEMENT_DYNAMIC(CInvVisionWebDlg, CNxDialog)

CInvVisionWebDlg::CInvVisionWebDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvVisionWebDlg::IDD, pParent)
{
	m_pThread = NULL;
	m_nOpticianID = -1;
}

CInvVisionWebDlg::~CInvVisionWebDlg()
{
}
// (s.dhole 2011-02-16 13:33) - PLID 41397 Need To call Cleanup for thread, Was causing memeory leak
void CInvVisionWebDlg::OnDestroy() 
{
	try
	{
		StopCheckOrderStatusThread() ;
		// (s.dhole 2011-05-16 12:12) - PLID 41986 Release Bitmap
		if (m_hBitmap)
			DeleteObject(m_hBitmap);
		CNxDialog::OnDestroy();	
	}NxCatchAll("CInvVisionWebDlg::OnDestroy");
	
}


void CInvVisionWebDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RADIO_ALL_VISIONWEB_DATES, m_radioAllOrderDates);
	DDX_Control(pDX, IDC_RADIO_VISIONWEB_DATE_RANGE, m_radioOrderDateRange);
	DDX_Control(pDX, IDC_DT_VISIONWEB_FROM, m_OrderDateFrom);
	DDX_Control(pDX, IDC_DT_VISIONWEB_TO, m_OrderDateTo);
	DDX_Control(pDX, IDC_NEW_VISIONWEB_ORDER, m_NewVisionWebOrderBtn);
	DDX_Control(pDX, IDC_BTN_CHECK_ORDER_STATUS , m_ChckBatchOrderStatusBtn);
	DDX_Control(pDX, IDC_BTN_GLASSES_CATALOG_SETUP , m_CatalogSetupBtn);
	DDX_Control(pDX, IDC_ORDER_STATUS_PROGRESS_BAR, m_ctrlOrderStatusProgressBar);
	DDX_Control(pDX, IDC_ORDER_STATUS_PROGRESS_TEXT, m_nxstaticOrderStatusProgressText);
	DDX_Control(pDX, IDC_NOT_BILLED_ORDERS, m_radioFilterNotBilled);
	DDX_Control(pDX, IDC_BILLED_ORDERS, m_radioFilterBilled);
	DDX_Control(pDX, IDC_ALL_ORDERS, m_radioFilterAll);
}

BEGIN_EVENTSINK_MAP(CInvVisionWebDlg, CNxDialog)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_COMBO_PATIENT, 18 /* RequeryFinished */, CInvVisionWebDlg::OnRequeryFinishedPatientList, VTS_I2)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_COMBO_SUPPLIER, 18 /* RequeryFinished */, CInvVisionWebDlg::OnRequeryFinishedSupplierList, VTS_I2)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_COMBO_PATIENT, 1 /* SelChanging */, CInvVisionWebDlg::OnSelChangingOrderPatientCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_COMBO_PATIENT, 16 /* TrySetSelFinished */, CInvVisionWebDlg::OnSelChosenOrderPatientCombo, VTS_DISPATCH)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_COMBO_SUPPLIER, 1 /* SelChanging */, CInvVisionWebDlg::OnSelChangingOrderSupplierCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_COMBO_SUPPLIER, 16 /* TrySetSelFinished */, CInvVisionWebDlg::OnSelChosenOrderSupplierCombo, VTS_DISPATCH)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_ORDER_STATUS, 1 /* SelChanging */, CInvVisionWebDlg::OnSelChangingOrderStatusCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_ORDER_STATUS, 16 /* TrySetSelFinished */, CInvVisionWebDlg::OnSelChosenOrderStatusCombo, VTS_DISPATCH)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_COMBO_OPTICIAN, 1 /* SelChanging */, CInvVisionWebDlg::OnSelChangingOpticianCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_COMBO_OPTICIAN, 16 /* TrySetSelFinished */, CInvVisionWebDlg::OnSelChosenOpticianCombo, VTS_DISPATCH)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_DATE_FILTER_TYPES, 1 /* SelChanging */, CInvVisionWebDlg::OnSelChangingOrderDateFilterTypes, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_DATE_FILTER_TYPES, 16 /* TrySetSelFinished */, CInvVisionWebDlg::OnSelChosenOrderDateFilterTypes, VTS_DISPATCH)
	
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_ORDER_LIST, 18 /* RequeryFinished */,CInvVisionWebDlg::OnRequeryFinishedOrderList, VTS_I2)
	// (a.walling 2010-09-29) - no PLID - Fixed broken compilation
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_ORDER_LIST, 16, CInvVisionWebDlg::SelChosenVisionwebList, VTS_DISPATCH)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_ORDER_LIST, 3, CInvVisionWebDlg::DblClickCellVisionwebOrderList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CInvVisionWebDlg, IDC_VISIONWEB_ORDER_LIST, 6 /* RButtonDown */, CInvVisionWebDlg::OnRButtonDownVisionwebOrderList,  VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CVisionWebSetupDlg, IDC_VISIONWEB_COMBO_OPTICIAN, 18 /* RequeryFinished */, OnRequeryFinishedOptician, VTS_I2)
	ON_EVENT(CInvVisionWebDlg, IDC_VISION_WEB_LOCATION_FILTER, 1, CInvVisionWebDlg::SelChangingVisionWebLocationFilter, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvVisionWebDlg, IDC_VISION_WEB_LOCATION_FILTER, 16, CInvVisionWebDlg::SelChosenVisionWebLocationFilter, VTS_DISPATCH)
	ON_EVENT(CInvVisionWebDlg, IDC_VISION_WEB_LOCATION_FILTER, 18, CInvVisionWebDlg::RequeryFinishedVisionWebLocationFilter, VTS_I2)
END_EVENTSINK_MAP()
 
BEGIN_MESSAGE_MAP(CInvVisionWebDlg, CNxDialog)
	ON_WM_CONTEXTMENU()
	ON_WM_DESTROY()
	ON_COMMAND(ID_VISIONWEB_ORDERPOPUP_STATUS_CHECK, OnCheckOrderStatus)
	ON_COMMAND(ID_ORDERPOPUP_STATUSHISTORY, OnShowOrderStatusHistory)
	ON_COMMAND(ID_VISIONWEB_ORDERPOPUP_EDIT, OnEditOrder)
	ON_COMMAND(ID_VISIONWEB_ORDERPOPUP_DELETE, OnDeleteOrder)
	ON_COMMAND(ID_REPORT_GLASSESRX, OnGlassesReportRx)
	ON_COMMAND(ID_REPORT_GLASSESORDER, OnGlassesReport)
	ON_COMMAND(ID_VISIONWEB_ORDERPOPUP_GOTOPATIENT, OnGoToPatient)
	ON_COMMAND(ID_VISIONWEB_ORDERPOPUP_BILLORDER, OnBillOrder)
	
	ON_BN_CLICKED(IDC_NEW_VISIONWEB_ORDER, OnCreateVisionWebOrder)
	ON_BN_CLICKED(IDC_BTN_VISIONWEB_ACCOUNT_SETUP, OnClickVisionWebAccountSetup)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DT_VISIONWEB_FROM, OnChangeOrderDtOverviewFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DT_VISIONWEB_TO, OnChangeOrderDtOverviewTo)
	ON_BN_CLICKED(IDC_RADIO_ALL_VISIONWEB_DATES, OnRadioOrderAllOverviewDates)
	ON_BN_CLICKED(IDC_RADIO_VISIONWEB_DATE_RANGE, OnRadioOrderOverviewDateRange)
	ON_BN_CLICKED(IDC_BTN_CHECK_ORDER_STATUS, OnCheckBatchOrderStatus)
	ON_MESSAGE(NXM_ORDER_BATCHSTATUS_LIST_PROCESS_DATA, OnOrderStatusProcessData) 
	ON_MESSAGE(NXM_ORDER_BATCHSTATUS_SET_PROGRESS_MIN_MAX, OnSetOrderStatusProgressMinMax)
	ON_MESSAGE(NXM_ORDER_BATCHSTATUS_SET_PROGRESS_POSITION, OnSetOrderStatusProgressPosition)
	ON_MESSAGE(NXM_ORDER_BATCHSTATUS_SET_PROGRESS_TEXT, OnSetOrderStatusProgressText)
	ON_MESSAGE(NXM_ORDER_BATCHSTATUS_LIST_REQUERY_FINISHED, OnSetOrderStatusListRequeryFinished)
	ON_MESSAGE(NXM_VISIONWEB_ORDER_DLG_CLOSED, OnVisionWebOrderDlgClosed)
	ON_MESSAGE(NXM_CONTACT_ORDER_DLG_CLOSED, OnVisionWebOrderDlgClosed)
	ON_BN_CLICKED(IDC_BTN_GLASSES_CATALOG_SETUP, &CInvVisionWebDlg::OnBnClickedBtnGlassesCatalogSetup)
	ON_COMMAND(ID_VISIONWEB_ORDERPOPUP_CHANGE_ORDER_STATUS, &CInvVisionWebDlg::OnVisionwebOrderpopupChangeOrderStatus)
	ON_BN_CLICKED(IDC_ALL_ORDERS, &CInvVisionWebDlg::OnBnClickedAllOrders)
	ON_BN_CLICKED(IDC_NOT_BILLED_ORDERS, &CInvVisionWebDlg::OnBnClickedNotBilledOrders)
	ON_BN_CLICKED(IDC_BILLED_ORDERS, &CInvVisionWebDlg::OnBnClickedBilledOrders)
END_MESSAGE_MAP()


BOOL CInvVisionWebDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		CInvVisionWebDlg::SetVisionWebDefaultValue(); // (s.dhole 2010-12-07 12:44) - PLID 41281
		m_bBachOrderStatusChecking=FALSE ;
		m_NewVisionWebOrderBtn.AutoSet(NXB_NEW);
		m_ChckBatchOrderStatusBtn.AutoSet(NXB_MODIFY);
		m_CatalogSetupBtn.AutoSet(NXB_MODIFY); // (s.dhole 2011-03-14 18:02) -  PLID 42835  Dialog to Maintain Glasses Catalog custom items
		m_VisionWebOrderList= BindNxDataList2Ctrl(IDC_VISIONWEB_ORDER_LIST ,GetRemoteData(), false);
		m_VisionWebPatientCombo = BindNxDataList2Ctrl(IDC_VISIONWEB_COMBO_PATIENT,GetRemoteData(),false);
		m_VisionWebSupplierCombo = BindNxDataList2Ctrl(IDC_VISIONWEB_COMBO_SUPPLIER , GetRemoteData(),  false);
		m_VisonWebLocationList = BindNxDataList2Ctrl(IDC_VISION_WEB_LOCATION_FILTER, true); // (j.dinatale 2013-04-12 14:49) - PLID 56214	

		// (j.dinatale 2012-05-02 12:26) - PLID 49758 - optician combo
		m_VisionWebOpticianCombo= BindNxDataList2Ctrl(IDC_VISIONWEB_COMBO_OPTICIAN, false);
		m_VisionWebOpticianCombo->FromClause =
			"( "
			"	SELECT -1 AS OpticianID, '< All >' AS OpticianName "
			"	UNION "
			"	SELECT OpticianID, "
			"	(PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle) AS OpticianName "
			"	FROM PersonT "
			"	INNER JOIN ( "
			"		SELECT OpticianID AS OpticianID "
			"		FROM GlassesOrderT "
			"		WHERE OpticianID IS NOT NULL "
			"	UNION "
			"		SELECT PersonID AS OpticianID "
			"		FROM ProvidersT "
			"		LEFT JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
			"		WHERE Optician = 1 AND Archived = 0 "
			"	) OpticiansT ON PersonT.ID = OpticiansT.OpticianID "
			") OpticiansSubQ";
		m_VisionWebOpticianCombo->Requery();


		m_VisionWebOrderStatusCombo = BindNxDataList2Ctrl(IDC_VISIONWEB_ORDER_STATUS,true);  //(r.wilson 4/11/2012) PLID 43741 - Turned auto requery to TRUE
		m_VisionWebOrderDateFilterCombo= BindNxDataList2Ctrl(IDC_VISIONWEB_DATE_FILTER_TYPES,false);
		// (s.dhole 2011-05-16 12:12) - PLID 41986 Load bmp to Setup   button
		m_hBitmap = ::LoadBitmap(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_VISIONWEB_SETUP_LOGO));
			((CButton*)GetDlgItem(IDC_BTN_VISIONWEB_ACCOUNT_SETUP))->SetBitmap(m_hBitmap);

		m_nselectedPatient=-1;

		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		//load Supplier List
		// (s.dhole 2011-03-29 17:16) - PLID 43040 Now user can select non vision web supplier for any order 
		// (s.dhole 2011-04-21 11:45) - PLID 43040 we need to show inactive Supplier only it use in order
		// (s.dhole 2012-03-01 09:26) - PLID 48547 Allow non supplier to filter -2
		m_VisionWebSupplierCombo->FromClause = _bstr_t("(SELECT -1 AS SupplierID,' < All Suppliers >' AS SupplierName, '' AS GlassesSupplierCode "
			" UNION "
			" SELECT -2 AS SupplierID,' < None >' AS SupplierName, '' AS GlassesSupplierCode "
			" UNION "
			" SELECT SupplierT.PersonID AS SupplierID , PersonT.Company  AS SupplierName, SupplierT.VisionWebID  AS GlassesSupplierCode  "
			" FROM  PersonT INNER JOIN SupplierT ON PersonT.ID = SupplierT.PersonID "
            "  WHERE SupplierT.PersonID IN (Select SupplierID From GlassesOrderT WHERE ISDelete=0 )) AS SupplierQ ");
		m_VisionWebSupplierCombo ->Requery(); 
	
	 

		//load Order types list
		IRowSettingsPtr pNewRow = NULL;
		// (j.dinatale 2012-05-02 15:39) - PLID 49758 - repurposed this list
		/*IRowSettingsPtr pNewRow = m_VisionWebOrderTypeCombo->GetNewRow();
		pNewRow->PutValue(vwotOrderTypeID, -1);
		pNewRow->PutValue(vwotOrderTypeName, " < All >");		
		m_VisionWebOrderTypeCombo->AddRowAtEnd(pNewRow,NULL);
			pNewRow = m_VisionWebOrderTypeCombo->GetNewRow();
			pNewRow->PutValue(vwotOrderTypeID, vwotSpectacleLens);
			pNewRow->PutValue(vwotOrderTypeName, _bstr_t(GetVisionWebOrderTypeDescription(vwotSpectacleLens)));
			pNewRow->BackColor = GetVisionWebOrderTypeColor(vwotSpectacleLens);
			m_VisionWebOrderTypeCombo->AddRowAtEnd(pNewRow,NULL);*/
		//TES 9/27/2010 - PLID 40539 - Changed this to loop through the enums
		/*for(int i = vwotSpectacleLens; i < vwot_LastEnum; i++) {
			pNewRow = m_VisionWebOrderTypeCombo->GetNewRow();
			pNewRow->PutValue(vwotOrderTypeID, i);
			pNewRow->PutValue(vwotOrderTypeName, _bstr_t(GetVisionWebOrderTypeDescription((VisionWebOrderType)i)));
			pNewRow->BackColor = GetVisionWebOrderTypeColor((VisionWebOrderType)i);
			m_VisionWebOrderTypeCombo->AddRowAtEnd(pNewRow,NULL);
		}*/

		//IRowSettingsPtr pDesiredTopRow = m_VisionWebOrderTypeCombo->FindByColumn(vwotOrderTypeID, -1L, NULL, VARIANT_FALSE);   // Cast as long
		//m_VisionWebOrderTypeCombo->CurSel =pDesiredTopRow ;
		
		//(r.wilson 4/11/2012) PLID 43741 - Add a row to the status combobox to show All statuses			
		pNewRow = m_VisionWebOrderStatusCombo->GetNewRow();
		pNewRow->PutValue(vwosOrderStatusID, -1L); // Cast as long
		pNewRow->PutValue(vwosOrderStatusName, " < All >");
		pNewRow->PutValue(vwosOrderStatusOrder, 0L);
		m_VisionWebOrderStatusCombo->AddRowBefore(pNewRow,	m_VisionWebOrderStatusCombo->GetTopRow());
		m_VisionWebOrderStatusCombo->CurSel = m_VisionWebOrderStatusCombo->GetTopRow();

		//load Data filter field
		pNewRow = m_VisionWebOrderDateFilterCombo->GetNewRow();
		pNewRow->PutValue(vwdftcID, 1L);// Cast as long
		pNewRow->PutValue(vwddftcName, "Create Date");
		m_VisionWebOrderDateFilterCombo->AddRowAtEnd(pNewRow,NULL);
		pNewRow = m_VisionWebOrderDateFilterCombo->GetNewRow();
		pNewRow->PutValue(vwdftcID, 2L);// Cast as long
		pNewRow->PutValue(vwddftcName, "Submit Date");
		m_VisionWebOrderDateFilterCombo->AddRowAtEnd(pNewRow,NULL);
		pNewRow = m_VisionWebOrderDateFilterCombo->GetNewRow();
		pNewRow->PutValue(vwdftcID, 3L);// Cast as long
		pNewRow->PutValue(vwddftcName, "Last Update Date");
		m_VisionWebOrderDateFilterCombo->AddRowAtEnd(pNewRow,NULL);

		// (j.dinatale 2013-04-12 14:51) - PLID 56214 - add the <All> option for our filter
		pNewRow = m_VisonWebLocationList->GetNewRow();
		pNewRow->PutValue(VisionWebLocationDD::ID, -1);
		pNewRow->PutValue(VisionWebLocationDD::Name, " < All >");
		m_VisonWebLocationList->AddRowSorted(pNewRow, NULL);
		
		IRowSettingsPtr pDesiredTopRow = m_VisionWebOrderDateFilterCombo->FindByColumn(vwdftcID, 1L, NULL, VARIANT_FALSE);// Cast as long
		m_VisionWebOrderDateFilterCombo->CurSel  = pDesiredTopRow ;

		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		// (s.dhole 2011-05-05 10:57) - PLID 42953 Added GlassesOrderT.LensRxID
		//(c.copits 2011-09-29) PLID 43742 - Add Last Appointment Date as a column on the Glasses Order tab

		// (s.dhole 2012-02-16 12:08) - PLID 48033 Supplier can be optional
		// (j.dinatale 2012-02-20 17:15) - PLID 47402 - Added a been billed column to determine if the order has been billed
		// (j.dinatale 2012-03-14 11:38) - PLID 47402 - reimplemented the been billed filter because we changed our structure
		// (j.dinatale 2012-04-19 09:14) - PLID 49758 - added Optician to the query
		// (j.dinatale 2012-05-02 14:39) - PLID 49758 - Added OpticianID to the query so we can filter on it
		// (s.dhole 2012-05-08 09:45) - PLID 50131 Added GlassesOrderT.InvoiceNo 
		// (j.dinatale 2013-04-12 15:16) - PLID 56214 - added location ID
		m_VisionWebOrderList->FromClause= _bstr_t( "(SELECT    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle + ' (' + CAST(PatientsT.UserDefinedID AS nvarchar(100)) + ') ' AS PatientName, "
			" GlassesOrderT.PersonID AS PatientID,  GlassesOrderT.ID AS OrderID,  "
			" (SELECT Max(AppointmentsT.Date) FROM AppointmentsT WHERE AppointmentsT.PatientID = PersonT.ID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4) AS LastApptDate, "
			"  GlassesOrderStatusT.OrderStatusName AS OrderStatusName, GlassesOrderT.GlassesOrderStatusID ,  "
			" CASE GlassesOrderT.GlassesOrderType WHEN 1 THEN 'Spectacle Lens Order (SP)' WHEN 2 THEN 'Frame Order (FR)' WHEN 3 THEN 'Contact Lens Patient (CP)' "
			" WHEN 4 THEN 'Contact Lens Office Order (CO)' WHEN 5 THEN 'Spectacle Lens Frame Order (CSF)' END AS OrderTypeName,  "
			" GlassesOrderT.GlassesOrderType , GlassesOrderT.OrderCreateDate, GlassesOrderT.OrderUploadDate , GlassesOrderT.UpdateDate , "
			" SupplierPersonT.Company AS GlassesSupplierName, GlassesOrderT.VisionWebOrderExchangeID, GlassesOrderT.GlassesOrderNumber, "
			" GlassesOrderT.SupplierID , GlassesOrderT.Description, GlassesOrderT.GlassesMessage AS Message, "
			" GlassesOrderStatusT.Color AS OrderStatusColor, CASE GlassesOrderT.GlassesOrderType WHEN 1 THEN 16777215 WHEN 2 THEN 9764585 WHEN 3 THEN 16774329 "
			" WHEN 4 THEN 16699591 WHEN 5 THEN 9751038 END AS OrderTypeColor ,GlassesOrderT.GlassesOrderProcessTypeID,GlassesOrderT.LensRxID, "
			" (CASE WHEN BilledOrdersQ.BilledGlassesOrderID IS NULL THEN 0 ELSE 1 END) AS BeenBilled, "
			" GlassesOrderT.OpticianID AS OpticianID, "
			" CASE WHEN GlassesOrderT.OpticianID IS NULL THEN '' ELSE Opticians.Last + ', ' + Opticians.First + ' ' + Opticians.Middle END AS Optician ,"
			" GlassesOrderT.InvoiceNo, "
			" GlassesOrderT.LocationID "
			" FROM SupplierT INNER JOIN "
			" SupplierT AS GlassesSupplierT ON SupplierT.PersonID = GlassesSupplierT.PersonID INNER JOIN "
			" PersonT AS SupplierPersonT ON SupplierT.PersonID = SupplierPersonT.ID RIGHT OUTER JOIN "
			" GlassesOrderT INNER JOIN "
			" PatientsT ON GlassesOrderT.PersonID = PatientsT.PersonID INNER JOIN "
			" PersonT ON PatientsT.PersonID = PersonT.ID ON GlassesSupplierT.PersonID = GlassesOrderT.SupplierID "
			" LEFT JOIN PersonT Opticians ON GlassesOrderT.OpticianID = Opticians.ID "
			" LEFT OUTER JOIN ( "
			"	SELECT DISTINCT GlassesOrderT.ID AS BilledGlassesOrderID FROM GlassesOrderT "
			"	LEFT JOIN BillsT ON GlassesOrderT.BillID = BillsT.ID "
			"	LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
			"	INNER JOIN ( "
			"		SELECT DISTINCT BillID "
			"		FROM "
			"		LineItemT "
			"		INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			" 		WHERE LineItemT.Deleted = 0 "
			"	) ChargesSubQ ON BillsT.ID = ChargesSubQ.BillID "
			"	WHERE BillsT.Deleted = 0 AND BillCorrectionsT.ID IS NULL "
			" ) AS BilledOrdersQ ON  "
			" GlassesOrderT.ID = BilledOrdersQ.BilledGlassesOrderID "
			" INNER JOIN GlassesOrderStatusT ON GlassesOrderT.GlassesOrderStatusID = GlassesOrderStatusT.ID"
			" WHERE     (GlassesOrderT.IsDelete = 0) ) as GlassesOrderQ");
		/*m_VisionWebOrderList->FromClause= _bstr_t(
			"(SELECT    PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle + ' (' + CAST(PatientsT.UserDefinedID AS nvarchar(100)) + ') ' AS PatientName,  "
			"			 GlassesOrderT.PersonID AS PatientID,  GlassesOrderT.ID AS OrderID,   "
			"			 (SELECT Max(AppointmentsT.Date) FROM AppointmentsT WHERE AppointmentsT.PatientID = PersonT.ID AND AppointmentsT.Date < GetDate() AND AppointmentsT.Status <> 4) AS LastApptDate,  "
			"			 GlassesOrderT.GlassesOrderStatusID ,GlassesOrderStatusT.ID ,GlassesOrderStatusT.OrderStatusName ,   "
			"			 CASE GlassesOrderT.GlassesOrderType WHEN 1 THEN 'Spectacle Lens Order (SP)' WHEN 2 THEN 'Frame Order (FR)' WHEN 3 THEN 'Contact Lens Patient (CP)'  "
			"			 WHEN 4 THEN 'Contact Lens Office Order (CO)' WHEN 5 THEN 'Spectacle Lens Frame Order (CSF)' END AS OrderTypeName,   "
			"			 GlassesOrderT.GlassesOrderType , GlassesOrderT.OrderCreateDate, GlassesOrderT.OrderUploadDate , GlassesOrderT.UpdateDate ,  "
			"			 SupplierPersonT.Company AS GlassesSupplierName, GlassesOrderT.VisionWebOrderExchangeID, GlassesOrderT.GlassesOrderNumber,  "
			"			 GlassesOrderT.SupplierID , GlassesOrderT.Description, GlassesOrderT.GlassesMessage AS Message,  "
			"			 GlassesOrderStatusT.Color AS OrderStatusColor, CASE GlassesOrderT.GlassesOrderType WHEN 1 THEN 16777215 WHEN 2 THEN 9764585 WHEN 3 THEN 16774329  "
			"			 WHEN 4 THEN 16699591 WHEN 5 THEN 9751038 END AS OrderTypeColor ,GlassesOrderT.GlassesOrderProcessTypeID,GlassesOrderT.LensRxID,  "
			"			 (CASE WHEN BilledOrdersQ.BilledGlassesOrderID IS NULL THEN 0 ELSE 1 END) AS BeenBilled  "
			"			 FROM SupplierT INNER JOIN  "
			"			 SupplierT AS GlassesSupplierT ON SupplierT.PersonID = GlassesSupplierT.PersonID INNER JOIN  "
			"			 PersonT AS SupplierPersonT ON SupplierT.PersonID = SupplierPersonT.ID RIGHT OUTER JOIN  "
			"			 GlassesOrderT INNER JOIN  "
			"			 PatientsT ON GlassesOrderT.PersonID = PatientsT.PersonID INNER JOIN  "
			"			 PersonT ON PatientsT.PersonID = PersonT.ID ON GlassesSupplierT.PersonID = GlassesOrderT.SupplierID  "
			"			INNER JOIN GlassesOrderStatusT ON GlassesOrderStatusT.ID = GlassesOrderT.GlassesOrderStatusID  "
			"			 LEFT OUTER JOIN (  "
			"				SELECT DISTINCT GlassesOrderT.ID AS BilledGlassesOrderID FROM GlassesOrderT  "
			"				LEFT JOIN BillsT ON GlassesOrderT.BillID = BillsT.ID  "
			"				LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID  "
			"				INNER JOIN (  "
			"					SELECT DISTINCT BillID  "
			"					FROM  "
			"					LineItemT  "
			"					INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID  "
			"			 		WHERE LineItemT.Deleted = 0  "
			"				) ChargesSubQ ON BillsT.ID = ChargesSubQ.BillID  "
			"				WHERE BillsT.Deleted = 0 AND BillCorrectionsT.ID IS NULL  "
			"			 ) AS BilledOrdersQ ON   "
			"			 GlassesOrderT.ID = BilledOrdersQ.BilledGlassesOrderID 			 "
			"			 WHERE     (GlassesOrderT.IsDelete = 0) ) as GlassesOrderQ"); */

		// (j.dinatale 2012-02-08 15:00) - PLID 47402 - store the last selection for the billed status filter
		long nOrderBilledFilter = GetRemotePropertyInt("GlassesOrderBilledStatusFilter", 0, 0, GetCurrentUserName(), true);
		m_radioFilterAll.SetCheck(FALSE);
		m_radioFilterBilled.SetCheck(FALSE);
		m_radioFilterNotBilled.SetCheck(FALSE);

		// (j.dinatale 2012-02-08 15:00) - PLID 47402 - set the proper check and set the where clause accordingly
		if(nOrderBilledFilter == 2){
			// not billed
			m_radioFilterNotBilled.SetCheck(TRUE);
			m_VisionWebOrderList->PutWhereClause(_bstr_t("BeenBilled = 1"));
		}else{
			if(nOrderBilledFilter == 1){
				// only billed
				m_radioFilterBilled.SetCheck(TRUE);
				m_VisionWebOrderList->PutWhereClause(_bstr_t("BeenBilled = 0"));
			}else{
				// all orders
				m_radioFilterAll.SetCheck(TRUE);
			}
		}

		m_VisionWebOrderList->Requery();


		m_radioAllOrderDates.SetCheck(TRUE);
		// set date range by one day
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTimeSpan dtSpan30;
		dtSpan30.SetDateTimeSpan(1, 0, 0, 0);
		m_OrderDateTo.SetValue((dtNow));
		dtNow = dtNow - dtSpan30;
		m_OrderDateFrom.SetValue((dtNow));
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		//load patient List
		m_VisionWebPatientCombo->FromClause = _bstr_t("(SELECT -1 AS PatientID, NULL AS UserDefinedID, "
			"' < All Patients > ' AS Last,'' AS First, '' AS Middle , ' < All Patients > ' AS PatientName,  "
			" NULL AS HomePhone, NULL AS BirthDate "
			" UNION "
			" SELECT PatientsT.PersonID AS PatientID, PatientsT.UserDefinedID, PersonT.Last, PersonT.First, PersonT.Middle," 
			" PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, PersonT.HomePhone, PersonT.BirthDate "
			" FROM PatientsT INNER JOIN "
			" PersonT ON PatientsT.PersonID = PersonT.ID INNER JOIN "
			" GlassesOrderT ON GlassesOrderT.PersonID = PatientsT.PersonID "
			" WHERE   (PersonT.Archived = 0) AND (PatientsT.PersonID <> - 25) "
			" AND (PatientsT.CurrentStatus <> 4) AND (GlassesOrderT.IsDelete=0 )  ) AS GlassesPatientQ");
	    m_VisionWebPatientCombo->Requery(); 

	// (s.dhole 2010-11-12 17:01) - PLID 41470 Disabled all buttons
		if (!(GetCurrentUserPermissions(bioInvGlassesOrderTab) & ( sptWrite))) 
		{
			GetDlgItem(IDC_BTN_CHECK_ORDER_STATUS)->EnableWindow(FALSE);
			GetDlgItem(IDC_NEW_VISIONWEB_ORDER)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_VISIONWEB_ACCOUNT_SETUP )->EnableWindow(FALSE);
			// (s.dhole 2011-04-05 17:22) - PLID 42845
			GetDlgItem(IDC_BTN_GLASSES_CATALOG_SETUP )->EnableWindow(FALSE);
			// (s.dhole 2012-05-01 08:58) - PLID 50088 remove  InvEMNToGlassesOrderDlg Code
		}
		}NxCatchAll("Error in CInvVisionWebDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
// (s.dhole 2010-12-07 12:44) - PLID 41281 set default url data
// we will check default property settings for all vision web url and if those are missing or empty than insert default value
void CInvVisionWebDlg::SetVisionWebDefaultValue()
{
	try{
	g_propManager.BulkCache("VisionWebSetup",propbitText, FormatString(
		"(Username = '<None>') AND ("
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' OR "
		"Name = '%s' " 
		")", VISIONWEBSERVICE_USER_ID,VISIONWEBSERVICE_USER_PASSWORD,VISIONWEBSERVICE_REFID,
		VISIONWEBSERVICE_USER_ACCOUNT_URL,VISIONWEBSERVICE_CHANGE_PASSWORD_URL,VISIONWEBSERVICE_ORDER_URL
		,VISIONWEBSERVICE_ORDER_STATUS_URL,VISIONWEBSERVICE_CATALOG_URL,VISIONWEBSERVICE_ERROR_URL
		,VISIONWEBSERVICE_SUPPLIER_URL,VISIONWEBSERVICE_ORDER_ERROR_DETAIL_URL)); 
		
		// (c.haag 2015-11-18) - PLID 67575 - Use secure http instead of regular http for production mode. Testing mode still uses
		// visionwebqa.com on an unsecure http connection; there is code elsewhere which fixes the URL for testing mode.
		CInvVisionWebDlg::SetVisionWebProperty(VISIONWEBSERVICE_REFID,"NEXTECH");
		CInvVisionWebDlg::SetVisionWebProperty(VISIONWEBSERVICE_USER_ACCOUNT_URL,"https://www.visionweb.com/services/services/UserAccountsService");
		CInvVisionWebDlg::SetVisionWebProperty(VISIONWEBSERVICE_CHANGE_PASSWORD_URL,"https://www.visionweb.com/services/services/ChangePasswordService");
		CInvVisionWebDlg::SetVisionWebProperty(VISIONWEBSERVICE_ORDER_URL,"https://www.visionweb.com/vwservices/services/SPOrderService");
		CInvVisionWebDlg::SetVisionWebProperty(VISIONWEBSERVICE_ORDER_STATUS_URL,"https://services.visionweb.com/VWOrderTracking.asmx");
		CInvVisionWebDlg::SetVisionWebProperty(VISIONWEBSERVICE_CATALOG_URL,"https://www.visionweb.com/services/services/StandardCatalogService");
		CInvVisionWebDlg::SetVisionWebProperty(VISIONWEBSERVICE_ERROR_URL,"https://www.visionweb.com/services/services/ErrorSupportService");
		CInvVisionWebDlg::SetVisionWebProperty(VISIONWEBSERVICE_SUPPLIER_URL,"https://www.visionweb.com/services/services/UserAccountsService");
		CInvVisionWebDlg::SetVisionWebProperty(VISIONWEBSERVICE_ORDER_ERROR_DETAIL_URL,"https://www.visionweb.com/services/services/ErrorSupportService");
	}NxCatchAll("Error in CInvVisionWebDlg::SetVisionWebDefaultValue");
}

// (s.dhole 2010-12-07 12:44) - PLID 41281 set default url data
void CInvVisionWebDlg::SetVisionWebProperty(LPCTSTR strPropertyName,CString  strPropertyValue) 
{
	try{
	CString strOldValue= GetRemotePropertyText(strPropertyName, "", 0, "<None>", true);

	if (strOldValue.IsEmpty())	{
#ifdef _DEBUG
		strPropertyValue.Replace("visionweb.com","visionwebqa.com");
		// (c.haag 2015-11-18) - PLID 67575 - If we're in debug mode we should also not be using secure HTTP
		strPropertyValue.Replace("https://", "http://");
#endif
		SetRemotePropertyText (strPropertyName, strPropertyValue , 0, "<None>");
		}
	}NxCatchAll("Error in CInvVisionWebDlg::SetVisionWebProperty");		

}

// (s.dhole 2011-02-15 10:23) - PLID 40535 Set right click menu
//Added this event to replace right click call as we do other places in practice
void CInvVisionWebDlg::OnRButtonDownVisionwebOrderList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow==NULL)
		return;
	try {
		m_VisionWebOrderList->PutCurSel(pRow);
		// Yes, so show the context menu
		CMenu mnu;
		mnu.LoadMenu(IDR_VISIONWEB_TAB);
		CMenu *pmnuSub = mnu.GetSubMenu(0);
		if (pmnuSub) {
			CPoint pt;	
			GetCursorPos(&pt);
			// Hide certain items if we're not on a row
			long nOrderStatus=VarLong(pRow->GetValue(vwolOrderStatusID),-1) ;

			// (j.dinatale 2012-02-22 17:08) - PLID 48326 - need the patient ID and go to patient logic
			long nPatientID = VarLong(pRow->GetValue(vwolPatientID), -1);
			if(nPatientID == -1){
				pmnuSub->EnableMenuItem(ID_VISIONWEB_ORDERPOPUP_GOTOPATIENT, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
			}

			// (j.dinatale 2012-03-13 11:01) - PLID 47413 - control the bill order menu option 
			// (j.dinatale 2013-02-26 12:44) - PLID 55336 - hit the sql server only once
			long nOrderID = VarLong(pRow->GetValue(vwolOrderID), -1);
			_RecordsetPtr rsOrderInfo = CreateParamRecordset(
				"SET NOCOUNT ON; "
				"DECLARE @nOrderID INT; "
				"SET @nOrderID = {INT}; "

				"SELECT TOP 1 1 FROM GlassesOrderT "
				"LEFT JOIN BillsT ON GlassesOrderT.BillID = BillsT.ID "
				"LEFT JOIN BillCorrectionsT ON BillsT.ID = BillCorrectionsT.OriginalBillID "
				"INNER JOIN ( "
				"	SELECT DISTINCT BillID "
				"	FROM "
				"	LineItemT "
				"	INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"	WHERE LineItemT.Deleted = 0 "
				") ChargesSubQ ON BillsT.ID = ChargesSubQ.BillID "
				"WHERE BillsT.Deleted = 0 AND BillCorrectionsT.ID IS NULL AND GlassesOrderT.ID = @nOrderID; \r\n\r\n"

				"SELECT UserSetOrderStatus FROM GlassesOrderT WHERE GlassesOrderT.ID = @nOrderID; \r\n"
				"SET NOCOUNT OFF; ", nOrderID
			);

			if(!rsOrderInfo->eof){
				pmnuSub->EnableMenuItem(ID_VISIONWEB_ORDERPOPUP_BILLORDER, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
			}

			rsOrderInfo = rsOrderInfo->NextRecordset(NULL);

			// (j.dinatale 2013-02-26 12:39) - PLID 55336 - need to keep track if the user changed the status on a visionweb order
			if(!rsOrderInfo->eof){
				bool bUserSetStatus = !!AdoFldBool(rsOrderInfo, "UserSetOrderStatus", FALSE);
				if(bUserSetStatus){
					pmnuSub->EnableMenuItem(ID_VISIONWEB_ORDERPOPUP_STATUS_CHECK, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
				}
			}

			rsOrderInfo->Close();

			//TES 5/25/2011 - PLID 43842 - I don't see why they should ever be prevented from viewing the history
			/*if(nOrderStatus== vwOSPending )
				pmnuSub->EnableMenuItem(ID_ORDERPOPUP_STATUSHISTORY, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);*/
			
			if( nOrderStatus == /*vwOSSubmitted*/ GetOrderStatusAsInt("Submitted") )
				pmnuSub->EnableMenuItem(ID_VISIONWEB_ORDERPOPUP_DELETE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

			if( nOrderStatus == /*vwOSRejected*/ GetOrderStatusAsInt("Rejected") )	{
				//TES 5/25/2011 - PLID 43842 - I don't see why they should ever be prevented from viewing the history
				//pmnuSub->EnableMenuItem(ID_ORDERPOPUP_STATUSHISTORY, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
				pmnuSub->EnableMenuItem(ID_VISIONWEB_ORDERPOPUP_STATUS_CHECK, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
			}
			//If we are running batch status update, we should disable this menu	
			if(nOrderStatus == /*vwOSReceived*/ GetOrderStatusAsInt("Received") || m_bBachOrderStatusChecking==TRUE){
				pmnuSub->EnableMenuItem(ID_VISIONWEB_ORDERPOPUP_STATUS_CHECK, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);	
			}

			// (s.dhole 2010-11-12 17:01) - PLID 41470 Disabled all menu items except Edit
			if (!(GetCurrentUserPermissions(bioInvGlassesOrderTab) & ( sptWrite))) {
			   pmnuSub->EnableMenuItem(ID_VISIONWEB_ORDERPOPUP_DELETE, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
			   pmnuSub->EnableMenuItem(ID_VISIONWEB_ORDERPOPUP_STATUS_CHECK, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
				// (s.dhole 2011-04-05 17:18) - PLID 43077 Disabled  status change
			   pmnuSub->EnableMenuItem(ID_VISIONWEB_ORDERPOPUP_CHANGE_ORDER_STATUS, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
			}
			pmnuSub->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
			pmnuSub->DestroyMenu() ;
		}
	} NxCatchAll("CInvVisionWebDlg::OnRButtonDownVisionwebOrderList");

}




// (s.dhole 2010-11-09 15:18) - PLID 40535  mark orde as recived
//void CInvVisionWebDlg::OnRecivedOrder()
//{
//	try{
//		IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
//		if (pRow!=NULL)
//		{
//			//change message since change contextmenu functionality
//			int nResult = MessageBox("Are you sure you want to mark this order as Received?", NULL, MB_YESNO);
//			CWaitCursor pWait;
//			if(nResult == IDYES)
//			{
//			// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses	
//			long nOrderId=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
//			_RecordsetPtr rsOrder = CreateParamRecordset("SELECT     ID, GlassesOrderType, getdate () as dt "
//			" FROM  GlassesOrderT WHERE ID = {INT} ",nOrderId);
//			if(!rsOrder->eof) 
//				{
//				long nUserID = (long)GetCurrentUserID();	
//				long OrderType=AdoFldByte (rsOrder->Fields, "GlassesOrderType");
//				COleDateTime dt=AdoFldDateTime  (rsOrder->Fields, "dt");
//				_variant_t varUpdateDate= COleVariant(dt);
//				
//				CParamSqlBatch batch;
//				// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
//				batch.Add ("UPDATE GlassesOrderHistoryT SET IsActive=0 WHERE IsActive<>0 AND GlassesOrderID={INT}", nOrderId); // we should update only active record
//				batch.Add("INSERT INTO  GlassesOrderHistoryT "
//				"(GlassesOrderID,GlassesOrderType,GlassesOrderStatus,  Note,IsActive, UpdateDate,UserID)"
//				"VALUES ({INT},{INT},{INT},{STRING},1,{VT_DATE},{INT})"
//				, nOrderId,OrderType,vwOSReceived,"Marked as Received",varUpdateDate, nUserID);
//				batch.Add( "UPDATE GlassesOrderT SET  GlassesOrderStatus={INT}, UpdateDate={VT_DATE} "
//					" WHERE ID={INT}" 
//					,vwOSReceived,varUpdateDate, nOrderId);
//				batch.Execute(GetRemoteData());
//				pRow->PutValue(vwolMessage, _variant_t(""));
//				pRow->PutValue(vwolLastUpdatDate, varUpdateDate);
//				pRow->PutValue(vwolOrderStatusID,  (long)vwOSReceived);
//				pRow->PutValue(vwolOrderStatusName,  _variant_t("Received"));
//				pRow->ForeColor =  4194368;
//			}
//			}
//		}
//	}NxCatchAll("Error in CInvVisionWebDlg::OnRecivedOrder");
//}

// (s.dhole 2010-11-09 15:18) - PLID 40535  Open  order history dialog
void CInvVisionWebDlg::OnShowOrderStatusHistory()
{
	try{

		IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
		if (pRow!=NULL)
		{
			long nOrder=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
			CString  strVisionWebOrderID=VarString(pRow->GetValue(vwolVisionWebOrderNumber),"") ;
			if (nOrder>0)
			{
			CVisionWebOrderHistory dlg(this);
			dlg.m_nVisionWebOrderID = nOrder;
			dlg.m_strVisionWebOrderID =strVisionWebOrderID;
			dlg.DoModal();
			}
		}
		
	}NxCatchAll("Error in CInvVisionWebDlg::OnShowOrderStatusHistory");
}

// (s.dhole 2010-11-09 15:18) - PLID 40535  Call check order status routin
// (s.dhole 2011-02-16 13:40) -  rename variable InvVisionWebUtil to pInvVisionWebUtil
void CInvVisionWebDlg::OnCheckOrderStatus()
{
try{
	CWaitCursor pWait;
	IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
	if (pRow!=NULL){
		long nOrder=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
		if (nOrder>0){
			CInvVisionWebUtils *pInvVisionWebUtil = new CInvVisionWebUtils();
			CString strMsg;
			COleDateTime dt;
			dt.SetStatus(COleDateTime::invalid);

			// (c.haag 2015-11-18) - PLID 67575 - If CheckSubmittedOrderStatus returned FALSE meaning
			// an error occurred or the order never existed (???), but we still get a message and a valid date,
			// we should be updating the screen. If we don't do this, users are not notified that an error
			// occurred unless they refresh the list.
			BOOL bResult = pInvVisionWebUtil->CheckSubmittedOrderStatus(nOrder, strMsg, dt);
			if (bResult || (COleDateTime::valid == dt.GetStatus() && !strMsg.IsEmpty()))
			{
				pRow->PutValue(vwolMessage, _variant_t(strMsg));
				pRow->PutValue(vwolLastUpdatDate, COleVariant(dt));
				// Added message to prompt user about status
				AfxMessageBox(FormatString("Order #: %d \nStatus: %s",nOrder,strMsg)); 
			}
			delete pInvVisionWebUtil; //destroy object
		}
	}
}NxCatchAll("Error in CInvVisionWebDlg::OnCheckOrderStatus");
}

// (s.dhole 2010-11-09 15:18) - PLID 40535  open order for edit screen
void CInvVisionWebDlg::OnEditOrder()
{
	try{
		IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
		if (pRow!=NULL)
		{
			_variant_t ss=pRow->GetValue(vwolOrderID);
			long nOrder=VarLong(pRow->GetValue(vwolOrderID),-1) ;	

			long nOrderType = (long)VarShort(pRow->GetValue(vwolOrderType));
			if(nOrderType == vwotContactLensPatient){
				// (j.dinatale 2012-03-07 11:09) - PLID 48527 - new dialog for contact orders
				// (j.dinatale 2012-03-21 14:55) - PLID 49079 - open our new modeless windows
				GetMainFrame()->OpenContactLensOrderForm(this, nOrder);
			}else{
				// (c.haag 2010-11-16 11:37) - PLID 41124 - We now open the
				// order dialog modelessly
				GetMainFrame()->OpenVisionWebOrderDlg(this, nOrder);
			}
		}
	}NxCatchAll("Error in CInvVisionWebDlg::OnEditOrder");
}
 // (s.dhole 2011-03-21 18:56) - PLID 42898  Show Glasses report 	
void CInvVisionWebDlg::OnGlassesReport()
{
	try{
		IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
		if (pRow!=NULL)
		{	
		// (s.dhole 2012-05-21 13:49) - PLID 50531 Show contact Lens report 
		long nType =	 VarShort (pRow->GetValue(vwolOrderType),-1); 
		if (nType==vwotContactLensPatient ){
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(729)]);
			CPrintInfo prInfo;
			CPtrArray aryParams;
			infReport.nExtraID = VarLong(pRow->GetValue(vwolOrderID),-1);  
			RunReport(&infReport, &aryParams, TRUE, (CWnd*)this, "Contact lens Order",  &prInfo);
			ClearRPIParameterList(&aryParams);		//clear our parameters now that the report is done
			}
		else {
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(704)]);
			CPrintInfo prInfo;
			CPtrArray aryParams;
			infReport.nExtraID = VarLong(pRow->GetValue(vwolOrderID),-1);  
			RunReport(&infReport, &aryParams, TRUE, (CWnd*)this, "Glasses Order",  &prInfo);
			ClearRPIParameterList(&aryParams);		//clear our parameters now that the report is done
			}
		}
	}NxCatchAll(__FUNCTION__);
}



// (s.dhole 2011-05-05 09:54) - PLID 42953 - Show Glasses Rx Report
void CInvVisionWebDlg::OnGlassesReportRx()
{
	try{
		IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();		
		if (pRow!=NULL)
		{			
			long nType = VarShort(pRow->GetValue(vwolOrderType),-1);
			long nOrderId = VarLong(pRow->GetValue(vwolOrderID),-1); 			
			//(7/9/2012) r.wilson PLID 51423 - Print glasses report (Always goes to print preview screen)
			ShowPrescriptionReport(nOrderId, nType, this);		
		}
	}NxCatchAll(__FUNCTION__);
}
	; 
// (s.dhole 2010-11-09 15:18) - PLID 40535  mark order as delete
void CInvVisionWebDlg::OnDeleteOrder()
{
	try{
		IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
		if (pRow!=NULL)
		{	
			//change message since change contextmenu functionality
			int nResult = MessageBox("Are you sure you want to delete this order?", NULL, MB_YESNO);
			CWaitCursor pWait;
			if(nResult == IDYES)
			{
				long nOrderId=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
				// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
				//TES 11/24/2010 - PLID 40864 - Added extra information for auditing.
				_RecordsetPtr rsOrder = CreateParamRecordset("SELECT GlassesOrderT.ID, GlassesOrderType, "
					"Description, PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName   "
					"FROM GlassesOrderT LEFT JOIN PersonT ON GlassesOrderT.PersonID = PersonT.ID "
					"WHERE GlassesOrderT.ID = {INT} ",nOrderId);
				if(!rsOrder->eof) 
					{
					long nUserID = (long)GetCurrentUserID();	
					long OrderType=AdoFldByte (rsOrder->Fields, "GlassesOrderType");
					CString strDescription = AdoFldString(rsOrder->Fields, "Description");
					CParamSqlBatch batch;
					// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
					batch.Add ("UPDATE GlassesOrderHistoryT SET IsActive=0 WHERE IsActive<>0  AND GlassesOrderID={INT}", nOrderId);
					batch.Add("INSERT INTO  GlassesOrderHistoryT "
					"(GlassesOrderID,GlassesOrderType,GlassesOrderStatusID,  Note,IsActive, UpdateDate,UserID)"
					"VALUES ({INT},{INT},{INT},{STRING},1,Getdate(),{INT})"
					, nOrderId,OrderType,/*vwOSDeleted*/GetOrderStatusAsInt("Deleted"), "Delete order", nUserID);
					// (r.wilson 4/9/2012) plid 43741 - Set GlassesOrderStatusID instead of GlassesOrderStatus				
					batch.Add( "UPDATE GlassesOrderT SET  GlassesOrderStatusID={INT},IsDelete=1  ,UpdateDate=Getdate() "
						" WHERE ID={INT}" 
						,/*vwOSDeleted*/GetOrderStatusAsInt("Deleted"), nOrderId);
					batch.Execute(GetRemoteData());
					m_VisionWebOrderList->RemoveRow( pRow);
					
					IRowSettingsPtr pRowPatientList = m_VisionWebPatientCombo->GetCurSel();
					if (pRowPatientList!=NULL)
						m_nselectedPatient=VarLong(pRowPatientList->GetValue(vwpPatientID),-1) ;	
					else 
						m_nselectedPatient=-1;
					m_VisionWebPatientCombo->Requery(); 
					//TES 11/24/2010 - PLID 40864 - Audit this deletion
					AuditEvent(AdoFldLong(rsOrder->Fields, "PersonID", -1), AdoFldString(rsOrder->Fields, "PatientName",""), BeginNewAuditEvent(), aeiGlassesOrderDelete, nOrderId, AdoFldString(rsOrder->Fields, "Description"), "<Deleted>", aepMedium);
				}
			}
		}
		
	}NxCatchAll("Error in CInvVisionWebDlg::OnDeleteOrder");
}

//  (s.dhole 2010-11-09 15:18) - PLID 40538 apply  filter
void CInvVisionWebDlg::OnRequeryFinishedPatientList(short nFlags) 
{
	try{
		IRowSettingsPtr pDesiredTopRow = m_VisionWebPatientCombo->FindByColumn(vwpPatientID, m_nselectedPatient, NULL, VARIANT_FALSE);
		if (pDesiredTopRow != NULL)
			m_VisionWebPatientCombo->CurSel =pDesiredTopRow ;
		else
			m_VisionWebPatientCombo->CurSel =m_VisionWebPatientCombo->TopRow ;
		CInvVisionWebDlg::ReFilterVisionWebOrderList ();
	}NxCatchAll("Error in CInvVisionWebDlg::OnRequeryFinishedPatientList");
}
//  (s.dhole 2010-11-09 15:18) - PLID 40538 apply  filter
void CInvVisionWebDlg::OnRequeryFinishedSupplierList(short nFlags) 
{
	try{
		IRowSettingsPtr pDesiredTopRow = m_VisionWebSupplierCombo->FindByColumn(vwsSupplierID, -1L, NULL, VARIANT_FALSE); // Cast as long
		if (pDesiredTopRow != NULL)
			m_VisionWebSupplierCombo->CurSel =pDesiredTopRow ;
		else
			m_VisionWebSupplierCombo->CurSel =m_VisionWebSupplierCombo->TopRow ;
	}NxCatchAll("Error in CInvVisionWebDlg::OnRequeryFinishedSupplierList");
}
//  (s.dhole 2010-11-09 15:18) - PLID 40538 apply  filter
void CInvVisionWebDlg::OnRequeryFinishedOrderList(short nFlags) 
{
	try{
		// (j.dinatale 2012-05-02 18:26) - PLID 49758 - need this here to protect from overwriting our current saved selection
		if(nFlags == dlRequeryFinishedCanceled)
			return;

		IRowSettingsPtr pRow = m_VisionWebOrderList->FindByColumn(vwolOrderID, m_nOrderID, NULL, VARIANT_FALSE);
		if (pRow  != NULL)
		{
			m_VisionWebOrderList->CurSel =pRow  ;
			m_VisionWebOrderList->HighlightVisible =TRUE; // check with TOM This is to make sure selected Column is visible
			// (s.dhole 2011-03-29 17:34) - PLID 43040 This code will Requery Supplier DD if selected order supplier is missing from list
			pRow = m_VisionWebSupplierCombo->FindByColumn(vwsSupplierID, VarLong(pRow->GetValue(vwolWebVisionSupplierID),-1), NULL, VARIANT_FALSE); // Cast as long
			if (pRow==NULL)
				m_VisionWebSupplierCombo->Requery(); 
		}

		m_nOrderID=-1; // Reset ordeer variable
		
	}NxCatchAll("Error in CInvVisionWebDlg::OnRequeryFinishedOrderList()");
}
//  (s.dhole 2010-11-09 15:18) - PLID 40538 apply  filter
void CInvVisionWebDlg::OnRadioOrderAllOverviewDates() 
{
	try{
	//this will handle enabling/disabling the date controls
	IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
	if (pRow!=NULL)
		m_nOrderID=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
	else 
		m_nOrderID=-1;
		
	ReFilterVisionWebOrderList();
	}NxCatchAll("Error in CInvVisionWebDlg::OnRadioOrderAllOverviewDates()");
}
//  (s.dhole 2010-11-09 15:18) - PLID 40538 apply  filter
void CInvVisionWebDlg::OnRadioOrderOverviewDateRange()
{
	try{
	//this will handle enabling/disabling the date controls
	IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
	if (pRow!=NULL)
		m_nOrderID=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
	else 
		m_nOrderID=-1;
	ReFilterVisionWebOrderList();
	}NxCatchAll("Error in CInvVisionWebDlg::OnRadioOrderOverviewDateRange()");
}
//  (s.dhole 2010-11-09 15:18) - PLID 40538 apply  filter
void CInvVisionWebDlg::OnChangeOrderDtOverviewFrom(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try{
	IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
	if (pRow!=NULL)
		m_nOrderID=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
	else 
		m_nOrderID=-1;

	ReFilterVisionWebOrderList();

	*pResult = 0;
	}NxCatchAll("Error in CInvVisionWebDlg::OnChangeOrderDtOverviewFrom()");
}
//  (s.dhole 2010-11-09 15:18) - PLID 40538 apply  filter
void CInvVisionWebDlg::OnChangeOrderDtOverviewTo(NMHDR* pNMHDR, LRESULT* pResult) 
{
try {
	IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
	if (pRow!=NULL)
		m_nOrderID=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
	else 
		m_nOrderID=-1;

	ReFilterVisionWebOrderList();
	
	*pResult = 0;
	}NxCatchAll("Error in CInvVisionWebDlg::OnChangeOrderDtOverviewTo");
}
//  (s.dhole 2010-11-09 15:18) - PLID 40535 
void CInvVisionWebDlg::Refresh()
{
	try {
		if(!m_VisionWebOrderList->IsRequerying()){
		IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
		if (pRow!=NULL)
			m_nOrderID=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
		else 
			m_nOrderID=-1;
		}
		
		if(!m_VisionWebPatientCombo->IsRequerying()){
		IRowSettingsPtr pRowPatientList = m_VisionWebPatientCombo->GetCurSel();
		if (pRowPatientList!=NULL)
			m_nselectedPatient=VarLong(pRowPatientList->GetValue(vwpPatientID),-1) ;	
		else 
			m_nselectedPatient=-1;
		}
		// need to refresh patient dropdown 
		m_VisionWebPatientCombo->Requery(); 

		if(!m_VisionWebOpticianCombo->IsRequerying()){
		// (j.dinatale 2012-05-02 16:02) - PLID 49758 - keep track of what is currently selected
		NXDATALIST2Lib::IRowSettingsPtr pOptRow = m_VisionWebOpticianCombo->GetCurSel();
		if(pOptRow){
			m_nOpticianID = VarLong(pOptRow->GetValue(VisionWebOpticianDD::OpticianID), -1) ;
		}else{
			m_nOpticianID = -1;
		}
		}
		// (j.dinatale 2012-05-02 15:20) - PLID 49758 - refresh the optician list
		m_VisionWebOpticianCombo->Requery();
		//ReFilterVisionWebOrderList();
		
		}NxCatchAll("Error in CInvVisionWebDlg::ReFilterVisionWebOrderList");
}
//  (s.dhole 2010-11-09 15:18) - PLID 40538 apply filter to order table
void CInvVisionWebDlg::ReFilterVisionWebOrderList() 
{
	try{
		CString strWhere;
		CString strPatient;
		IRowSettingsPtr pRow = m_VisionWebPatientCombo->GetCurSel();
		if(pRow != NULL) {
			long nPatientID = VarLong(pRow->GetValue(vwpPatientID));
			if(nPatientID != -1) {
				strPatient.Format("PatientID = %d", nPatientID);
			}
		}

		if(!strPatient.IsEmpty()) {
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strPatient;
		}
		
		pRow = m_VisionWebSupplierCombo->GetCurSel();
		CString strSupplier;
		if(pRow != NULL) {
			long nSupplier= VarLong(pRow->GetValue(vwsSupplierID));
			// (s.dhole 2012-03-01 09:26) - PLID 48547 Allow none supplier filter
			if(nSupplier == -2) {
				strSupplier= "SupplierID IS NULL ";
			}
			else if(nSupplier != -1) {
				strSupplier.Format("SupplierID = %d", nSupplier);
			}
		}

		if(!strSupplier.IsEmpty()) {
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strSupplier;
		}

		// this is hiden dropdown and will have only one row due to first client will have only one order type
		// (j.dinatale 2012-05-02 14:38) - PLID 49758 - filter optician as well, repurposed the order type list
		pRow = m_VisionWebOpticianCombo->GetCurSel();
		CString strOpticianFilter;
		if(pRow != NULL) {
			long nOpticianID = pRow->GetValue(VisionWebOpticianDD::OpticianID);
			// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
			if(nOpticianID != -1) {
				strOpticianFilter.Format("OpticianID = %li", nOpticianID);
			}
		}

		if(!strOpticianFilter.IsEmpty()) {
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strOpticianFilter;
		}

		
		 pRow = m_VisionWebOrderStatusCombo->GetCurSel();
		CString strOrderstatus;
		if(pRow != NULL) {
			long nOrderStatus = pRow->GetValue(vwosOrderStatusID);
			// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
			if(nOrderStatus != -1) {
				// (r.wilson 4/9/2012) plid 43741 - Updated the Colum to be GlassesOrderStatusID 
				strOrderstatus.Format("GlassesOrderStatusID = %d", nOrderStatus);
			}
		}

		if(!strOrderstatus.IsEmpty()) {
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strOrderstatus;
		}
		if(m_radioOrderDateRange.GetCheck()) {
			CString strDates;
			pRow = m_VisionWebOrderDateFilterCombo->CurSel;
			long nDateFilter = VarLong(pRow->GetValue(0));
			CString strDate;
			if(nDateFilter == 1) {
				//Status Create Date
				strDate = "OrderCreateDate";
			}
			else if(nDateFilter == 2) {
				//Submit Received
				strDate = "OrderUploadDate";
			}

			else if(nDateFilter == 3) {
				//Submit Received
				strDate = "UpdateDate";
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
			strFromDate = FormatDateTimeForSql((m_OrderDateFrom.GetValue()), dtoDate);
			strToDate = FormatDateTimeForSql((m_OrderDateTo.GetValue()), dtoDate);
			strDates.Format("(%s Is Not Null AND (%s >= '%s' AND %s < DATEADD(day, 1, '%s')))", strDate, strDate, strFromDate, strDate, strToDate);				
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strDates;
		}

		// (j.dinatale 2012-02-20 17:25) - PLID 47402 - filter on whether we are billed or not, if we want both dont filter at all
		if(m_radioFilterBilled.GetCheck()){
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}

			strWhere += "BeenBilled = 1";
		}else{
			if(m_radioFilterNotBilled.GetCheck()){
				if(!strWhere.IsEmpty()) {
					strWhere += " AND ";
				}

				strWhere += "BeenBilled = 0";
			}
		}

		// (j.dinatale 2013-04-12 15:16) - PLID 56214 - filter on the selected location
		pRow = m_VisonWebLocationList->GetCurSel();
		CString strLocation;
		if(pRow != NULL) {
			long nLocationID = VarLong(pRow->GetValue(VisionWebLocationDD::ID));
			if(nLocationID != -1) {
				strLocation.Format("LocationID = %li", nLocationID);
			}
		}

		if(!strLocation.IsEmpty()) {
			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strLocation;
		}

		m_OrderDateFrom.EnableWindow(m_radioOrderDateRange.GetCheck());
		m_OrderDateTo.EnableWindow(m_radioOrderDateRange.GetCheck());
		GetDlgItem(IDC_VISIONWEB_DATE_FILTER_TYPES)->EnableWindow(m_radioOrderDateRange.GetCheck());
		m_VisionWebOrderList->PutWhereClause(_bstr_t(strWhere));
		m_VisionWebOrderList->Requery(); 
			
	}NxCatchAll("Error in CInvVisionWebDlg::ReFilterVisionWebOrderList");
}

// (s.dhole 2010-11-09 15:18) - PLID 40535  open Account setup dialog
void CInvVisionWebDlg::OnClickVisionWebAccountSetup()
{
	try{	
		CVisionWebSetupDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll("Error in CInvVisionWebDlg::OnClickVisionWebAccountSetup");

}

// (s.dhole 2010-11-09 15:18) - PLID 40535   this will call from New order button
void CInvVisionWebDlg::OnCreateVisionWebOrder()
{
	try{
		//TES 5/24/2011 - PLID 43737 - Pop out a menu to decide which type of order to create
		enum {
			miGlassesOrder = -11,
			miContactLensOrder = -12,
		};

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, miGlassesOrder, "Glasses Order");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, miContactLensOrder, "Contact Lens Order");

		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_NEW_VISIONWEB_ORDER);
		int nResult = 0;
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, rc.right, rc.top, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this, NULL);
		}

		VisionWebOrderType vwot = vwotSpectacleLens;
		switch(nResult) {
			case miGlassesOrder:
				{
					vwot = vwotSpectacleLens;
					GetMainFrame()->OpenVisionWebOrderDlg(this, -1, vwot);
				}
				break;
			case miContactLensOrder:
				{
					// (j.dinatale 2012-03-21 15:05) - PLID 49079 - use our new modeless windows!
					GetMainFrame()->OpenContactLensOrderForm(this, -1);
				}
				break;
			default:
				//TES 5/24/2011 - PLID 43737 - They must have clicked off of the menu
				return;
		}
				
		// (c.haag 2010-11-16 11:37) - PLID 41124 - We now open the
		// order dialog modelessly
		//TES 5/24/2011 - PLID 43737 - Pass in the order type
		//GetMainFrame()->OpenVisionWebOrderDlg(this, -1, vwot);
	}
	NxCatchAll(__FUNCTION__);
}


void CInvVisionWebDlg::OnSelChangingOrderPatientCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
		// (s.dhole 2011-02-18 16:08) - PLID 40538 stop selecting null patient
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
		
	}NxCatchAll("Error in CInvVisionWebDlg::OnSelChangingOrderPatientCombo()")
}


void CInvVisionWebDlg::OnSelChosenOrderPatientCombo(LPDISPATCH lpRow)
{
	try {
	IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
	if (pRow!=NULL)
		m_nOrderID=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
	else 
		m_nOrderID=-1;

	ReFilterVisionWebOrderList();
	}NxCatchAll("Error in CInvVisionWebDlg::OnSelChosenOrderPatientCombo()")
}

void CInvVisionWebDlg::OnSelChangingOrderSupplierCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
		
		// (s.dhole 2011-02-18 16:08) - PLID 40538 do not allow null supplier selection
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
		
	}NxCatchAll("Error in CInvVisionWebDlg::OnSelChangingOrderSupplierCombo()")
}


void CInvVisionWebDlg::OnSelChosenOrderSupplierCombo(LPDISPATCH lpRow)
{
try {
	IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
	if (pRow!=NULL)
		m_nOrderID=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
	else 
		m_nOrderID=-1;

	ReFilterVisionWebOrderList();
	}NxCatchAll("Error in CInvVisionWebDlg::OnSelChosenOrderSupplierCombo()")
}


void CInvVisionWebDlg::OnSelChangingOrderStatusCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
		// (s.dhole 2011-02-18 16:08) - PLID 40538 do not allow null status selection
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
		

	}NxCatchAll("Error in CInvVisionWebDlg::OnSelChangingOrderStatusCombo()")
}



void CInvVisionWebDlg::OnSelChosenOrderStatusCombo(LPDISPATCH lpRow)
{
	try {
	IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
	if (pRow!=NULL)
		m_nOrderID=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
	else 
		m_nOrderID=-1;

	ReFilterVisionWebOrderList();
	}NxCatchAll("Error in CInvVisionWebDlg::OnSelChosenOrderStatusCombo()")
}


void CInvVisionWebDlg::OnSelChangingOrderDateFilterTypes(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
		
		// (s.dhole 2011-02-18 16:08) - PLID 40538 do not allow null date selection
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
		
	}NxCatchAll("Error in CInvVisionWebDlg::OnSelChangingOrderDateFilterTypes()")
}


void CInvVisionWebDlg::OnSelChosenOrderDateFilterTypes(LPDISPATCH lpRow)
{
	try {
	IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
	if (pRow!=NULL)
		m_nOrderID=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
	else 
		m_nOrderID=-1;

	ReFilterVisionWebOrderList();
	}NxCatchAll("Error in CInvVisionWebDlg::OnSelChosenOrderDateFilterTypes()")
}

void CInvVisionWebDlg::OnSelChangingOpticianCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
		// (s.dhole 2011-02-18 16:08) - PLID 40538 stop selecting null order type
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
		
	}NxCatchAll("Error in CInvVisionWebDlg::OnSelChangingOrderTypeCombo()")
}

// (j.dinatale 2012-05-02 12:26) - PLID 49758 - filter on optician
void CInvVisionWebDlg::OnSelChosenOpticianCombo(LPDISPATCH lpRow)
{
	try {
	IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
	if (pRow!=NULL)
		m_nOrderID=VarLong(pRow->GetValue(vwolOrderID),-1) ;	
	else 
		m_nOrderID=-1;

	ReFilterVisionWebOrderList();
	}NxCatchAll("Error in CInvVisionWebDlg::OnSelChosenOrderTypeCombo()")
}


// CInvVisionWebDlg message handlers

void CInvVisionWebDlg::SelChosenVisionwebList(LPDISPATCH lpRow)
{
	// TODO: Add your message handler code here
}
//  (s.dhole 2010-11-09 15:18) - PLID 41397 
//change cancel text and button text
void CInvVisionWebDlg::OnCheckBatchOrderStatus()
{
	try {
		if  (m_bBachOrderStatusChecking== FALSE){
			m_bBachOrderStatusChecking=TRUE ;
			StopCheckOrderStatusThread() ;
			SetDlgItemText(IDC_BTN_CHECK_ORDER_STATUS, "Cancel Checking Status For All Pending Orders"); 
			m_ChckBatchOrderStatusBtn.AutoSet(NXB_CANCEL);
			CheckPendingOrderList();
		}
		else
		{
			m_bBachOrderStatusChecking=FALSE ;
			SetDlgItemText(IDC_ORDER_STATUS_PROGRESS_TEXT, "");
			m_ctrlOrderStatusProgressBar.SetPos((short)0);
			SetDlgItemText(IDC_BTN_CHECK_ORDER_STATUS, "Check Status For All Pending Orders");
			
			StopCheckOrderStatusThread() ;
			m_ChckBatchOrderStatusBtn.AutoSet(NXB_MODIFY);
		}
		
	} NxCatchAll(__FUNCTION__);
}
//  (s.dhole 2010-11-09 15:18) - PLID 41397  - Thread data for launching the PopulateList thread

void CInvVisionWebDlg::CheckPendingOrderList()
{
try
	{
	g_hStopCheckingOrderStatus = CreateEvent(NULL, 0, FALSE, NULL);
	
	CVisionWebOrderThreadData *pData = new CVisionWebOrderThreadData;
	pData->m_hWnd = GetSafeHwnd();
	pData->m_nUserID = (long)GetCurrentUserID();
	
	pData->m_strURL = GetRemotePropertyText(VISIONWEBSERVICE_ORDER_STATUS_URL, "", 0, "<None>", true);
	pData->m_strUserID = GetRemotePropertyText(VISIONWEBSERVICE_USER_ID, "", 0, "<None>", true);
	CString strPassword= "";
	_variant_t vPass = GetRemotePropertyImage(VISIONWEBSERVICE_USER_PASSWORD, 0, "<None>", false);
	if (vPass.vt != VT_EMPTY && vPass.vt != VT_NULL) {
		strPassword = DecryptStringFromVariant(vPass);
	} else {
		strPassword = "";
	}	
	pData->m_strUserPSW  =strPassword;
	pData->m_strRefID =GetRemotePropertyText(VISIONWEBSERVICE_REFID, "", 0, "<None>", true);
	// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
	// (j.dinatale 2013-02-26 13:12) - PLID 55336 - ignore orders that have been manually set by the user
	_RecordsetPtr rsOrder = CreateRecordset("SELECT ID, GlassesOrderNumber,GlassesOrderType ,GlassesOrderStatusID"
		" FROM  GlassesOrderT WHERE IsDelete=0 AND UserSetOrderStatus = 0 AND GlassesOrderStatusID IN (2,4) AND GlassesOrderNumber IS NOT NULL ");
	while(!rsOrder->eof) {
		// Added new constructer to load data and convet to pointer
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		VisionWebOrderstruct  *pVisionWebOrder =new VisionWebOrderstruct(AdoFldLong(rsOrder, "ID"),AdoFldString (rsOrder->Fields, "GlassesOrderNumber","")
			,AdoFldByte (rsOrder->Fields, "GlassesOrderType"),AdoFldLong (rsOrder->Fields, "GlassesOrderStatusID"));
		
		pData->m_arynOrderIDs.Add(pVisionWebOrder) ;
		rsOrder->MoveNext();
	}

	m_pThread = AfxBeginThread(CheckingOrderStatusThread, (LPVOID)pData, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
	m_pThread->m_bAutoDelete = false;
	m_pThread->ResumeThread();
	}NxCatchAll(__FUNCTION__);
}

//  (s.dhole 2010-11-09 15:18) - PLID 41397  - Moved the logic to stop the thread out of Clear into its own function.
void CInvVisionWebDlg::StopCheckOrderStatusThread()
{
	CWaitCursor pWait;
	//////////////////////////
	// Kill the search
	if (m_pThread) {
		SetEvent(g_hStopCheckingOrderStatus);
		//Some time Webservice take more time(15 to 16 Sec) i will increse this till 25 Sec
		DWORD dwReturn = WaitForSingleObject(m_pThread->m_hThread, 25000);
		if(dwReturn == WAIT_TIMEOUT) {
			//There is absolutely no reason this thread shouldn't have been able to be completed in 25 seconds,
			//and it has to be terminated because we're about to delete memory which it references.  Our only
			//options are to TerminateThread and hope for the best, or just wait infinitely long for the thread.
			//We chose this option.
			::TerminateThread(m_pThread->m_hThread, 0);
		}

		// Now that we no longer are using this thread, we need
		// to catch and ignore any result processing messages we may have received.
		MSG msg;
		while(PeekMessage(&msg, GetSafeHwnd(), NXM_ORDER_BATCHSTATUS_LIST_PROCESS_DATA, NXM_ORDER_BATCHSTATUS_SET_PROGRESS_TEXT, PM_REMOVE))
		{
			if(msg.message == NXM_ORDER_BATCHSTATUS_LIST_PROCESS_DATA) {
				VisionWebOrderThreadstruct *pVisionWebOrderThreadData = (VisionWebOrderThreadstruct*)msg.wParam;
				delete pVisionWebOrderThreadData ; //  remove refrence
			}
		}

		//Now we need to delete the thread (remember, when we created, we told MFC not to auto-delete it).
		delete m_pThread;
		m_pThread = NULL;
		CloseHandle(g_hStopCheckingOrderStatus);
		g_hStopCheckingOrderStatus = NULL;
	}
}
//  (s.dhole 2010-11-09 15:18) - PLID 41397   
UINT CheckingOrderStatusThread(LPVOID pParam)
{
	int  OrderStatusProcess= vwStatusCheckNone;
	// since we are creating COM object instant in this thread, we need to call this function
	OleInitialize(NULL); 
	CVisionWebOrderThreadData *pThreadData = (CVisionWebOrderThreadData*)pParam;
	try {
		CString strVisionWebOrderId, strExchangeId,strCreationDate, strNumber, strReference , strType , strErrorCode; 
		CString strErrorDescription,strErrormessage;
		_ConnectionPtr pVisionWebOrderConn;
		// Open an new connection based on the existing one
		{
			pVisionWebOrderConn = GetThreadRemoteData();
		}
		long nUserID = pThreadData->m_nUserID;	
		CString strURL = pThreadData->m_strURL;
		CString strUserID = pThreadData->m_strUserID;
		CString strPSW = pThreadData->m_strUserPSW;
		CString strRefId = pThreadData->m_strRefID;
		BOOL bContinue = TRUE;
		PostMessage(pThreadData->m_hWnd, NXM_ORDER_BATCHSTATUS_SET_PROGRESS_MIN_MAX, (WPARAM)0, (LPARAM)100);
		int nSize = pThreadData->m_arynOrderIDs.GetSize();
		
		CInvVisionWebUtils *pVisionWebUtils= new CInvVisionWebUtils;
		
		for (int i = 0; i < nSize && bContinue ; i++) {
			// Change as pointer and rename variable
			VisionWebOrderstruct  *pVisionWebOrder =(VisionWebOrderstruct*)pThreadData->m_arynOrderIDs[i] ; 
			CString strMsg;
			COleDateTime dt;
			strMsg.Format("Checking order status of %d/%d..  Order #: %d  ",i,nSize,pVisionWebOrder->nOrderId);
			PostMessage(pThreadData->m_hWnd, NXM_ORDER_BATCHSTATUS_SET_PROGRESS_TEXT, (WPARAM)strMsg.AllocSysString(), 0);
			strMsg="";
			if (!pVisionWebUtils->UpdateOrderStatus(pVisionWebOrder->nOrderId,nUserID,pVisionWebOrder->nVisionWebOrderType, pVisionWebOrder->nVisionWebOrderStatus,
				pVisionWebOrder->strVisionWebOrderId,strURL, strUserID,strPSW, strRefId,strMsg,dt, pVisionWebOrderConn))
			{
				OrderStatusProcess= vwStatusCheckError;
			}
			VisionWebOrderThreadstruct *pVisionWebOrderThreadData =new VisionWebOrderThreadstruct();
			pVisionWebOrderThreadData->nOrderId = pVisionWebOrder->nOrderId;
			pVisionWebOrderThreadData->strVisionWebMessage =strMsg	;
			pVisionWebOrderThreadData->UpdateDate =dt;
			PostMessage(pThreadData->m_hWnd, NXM_ORDER_BATCHSTATUS_LIST_PROCESS_DATA, (WPARAM)pVisionWebOrderThreadData, 0);
			PostMessage(pThreadData->m_hWnd, NXM_ORDER_BATCHSTATUS_SET_PROGRESS_POSITION, (WPARAM)(short)(((double)(i+1)/(double)nSize)*100.0), 0);
			if (WaitForSingleObject(g_hStopCheckingOrderStatus, 0) != WAIT_TIMEOUT) 
				{
				bContinue = FALSE;
				if (OrderStatusProcess==0  || OrderStatusProcess==vwStatusCheckError)
					OrderStatusProcess= vwStatusCheckCanceled;
				}
		}
		delete pVisionWebUtils; //destroy object
	}NxCatchAllThread("Error in CInvVisionWebDlg::CheckingOrderStatusThread");
	if (OrderStatusProcess==0)
		OrderStatusProcess = vwStatusCheckSuccessful;
	PostMessage(pThreadData->m_hWnd, NXM_ORDER_BATCHSTATUS_LIST_REQUERY_FINISHED, 0, OrderStatusProcess );
	pThreadData->CleanUp();  // Call cleanup befor call delete
	delete pThreadData;
	OleUninitialize();
	return 0;
}

//  (s.dhole 2010-11-09 15:18) - PLID 41397 
LRESULT CInvVisionWebDlg::OnOrderStatusProcessData(WPARAM wParam, LPARAM lParam)
{
	try
	{
	VisionWebOrderThreadstruct *pVisionWebOrderThreadData = (VisionWebOrderThreadstruct*)wParam;
	IRowSettingsPtr pRow =  m_VisionWebOrderList->FindByColumn(vwolOrderID, (long)pVisionWebOrderThreadData->nOrderId , 0, VARIANT_FALSE);
	if (pRow!=NULL){ 
		pRow->PutValue(vwolMessage,_variant_t( pVisionWebOrderThreadData->strVisionWebMessage));
		pRow->PutValue(vwolLastUpdatDate,COleVariant( pVisionWebOrderThreadData->UpdateDate));
		}
	delete pVisionWebOrderThreadData; // remove refrence
	}NxCatchAll(__FUNCTION__);
	return 0;
}


//  (s.dhole 2010-11-09 15:18) - PLID 41397 
//- wParam = Min, lparam = Max.  Both need to be able to fit in a 'short' integer.
LRESULT CInvVisionWebDlg::OnSetOrderStatusProgressMinMax(WPARAM wParam, LPARAM lParam)
{
	try {
		//Constrain the max value to a 'short'
		long nMax = (long)lParam;
		if(nMax > (long)MAXSHORT) {
			nMax = MAXSHORT;
		}
		m_ctrlOrderStatusProgressBar.SetRange((short)wParam, (short)nMax);
		//Default the range to the min
		m_ctrlOrderStatusProgressBar.SetPos(0);

	} NxCatchAll(__FUNCTION__);

	return 0;
}
//  (s.dhole 2010-11-09 15:18) - PLID 41397  - wParam is a 'short' integer for the current position, from the range given
LRESULT CInvVisionWebDlg::OnSetOrderStatusProgressPosition(WPARAM wParam, LPARAM lParam)
{
	try {
		long nNewPos = (long)wParam;
		//Constrain the new value to a 'short'
		if(nNewPos > (long)MAXSHORT) {
			nNewPos = MAXSHORT;
		}
		m_ctrlOrderStatusProgressBar.SetPos((short)nNewPos);
	} NxCatchAll(__FUNCTION__);
	return 0;
}

//  (s.dhole 2010-11-09 15:18) - PLID 41397  - wParam is a BSTR, allocated by the Post'er.  This function will free the string.
LRESULT CInvVisionWebDlg::OnSetOrderStatusProgressText(WPARAM wParam, LPARAM lParam)
{
	try {
		//Memory management:  The BSTR was allocated by the caller, and must be freed by us.
		BSTR bstr = (BSTR)wParam;
		CString strText(bstr);
		SetDlgItemText(IDC_ORDER_STATUS_PROGRESS_TEXT, strText);
		SysFreeString(bstr);
	} NxCatchAll(__FUNCTION__);

	return 0;
}

//  (s.dhole 2010-11-09 15:18) - PLID 41397  change text to show result
//Text Change
LRESULT CInvVisionWebDlg::OnSetOrderStatusListRequeryFinished(WPARAM wParam, LPARAM lParam)
{
	try
	{
		m_bBachOrderStatusChecking=FALSE;
		VisionWebOrderStatusCheck  ststus =(VisionWebOrderStatusCheck)lParam ; 
		if (ststus==vwStatusCheckNone || ststus==vwStatusCheckSuccessful) 
			SetDlgItemText(IDC_ORDER_STATUS_PROGRESS_TEXT, "The status of all pending orders have now been updated with the VisionWeb Order tracking system information.");
		else if ( ststus==vwStatusCheckError) 
			SetDlgItemText(IDC_ORDER_STATUS_PROGRESS_TEXT, "An error occurred while updating order status. Please check VisionWeb Message column for detail information.");
		else if ( ststus==vwStatusCheckCanceled) 
			SetDlgItemText(IDC_ORDER_STATUS_PROGRESS_TEXT, "To update the status of all pending orders please click the \"Check Status For All Pending Orders\" button.");
		m_ctrlOrderStatusProgressBar.SetPos((short)0);
		SetDlgItemText(IDC_BTN_CHECK_ORDER_STATUS, "Check Status For All Pending Orders"); // Change button text
		m_ChckBatchOrderStatusBtn.AutoSet(NXB_MODIFY);
		// Destroy thread refrence
		StopCheckOrderStatusThread();  
	}NxCatchAll(" CFirstAvailList::OnOrderStatusListRequeryFinished");
	return 0;
}

// (c.haag 2010-11-16 11:37) - PLID 41124 - This message is received when the VisionWeb order dialog
// invoked by this window has closed. We do the same post-close handling that pre-existing code does.
LRESULT CInvVisionWebDlg::OnVisionWebOrderDlgClosed(WPARAM wParam, LPARAM lParam)
{
	try {
		// wParam = The dialog "return value"
		if (IDOK == wParam) {
			m_nOrderID=(long)lParam;
			if (NULL != m_VisionWebOrderList) {
				// (s.dhole 2010-11-09 15:18) - PLID 40535  to refresh patient list  
				
				IRowSettingsPtr pRowPatientList = m_VisionWebPatientCombo->GetCurSel();
				if (pRowPatientList!=NULL)
					m_nselectedPatient=VarLong(pRowPatientList->GetValue(vwpPatientID),-1) ;	
				else 
					m_nselectedPatient=-1;
				m_VisionWebPatientCombo->Requery();  
				//ReFilterVisionWebOrderList();
			}

			if(m_VisionWebOpticianCombo){
				// (j.dinatale 2012-05-02 16:02) - PLID 49758 - keep track of what is currently selected
				NXDATALIST2Lib::IRowSettingsPtr pOptRow = m_VisionWebOpticianCombo->GetCurSel();
				if(pOptRow){
					m_nOpticianID = VarLong(pOptRow->GetValue(VisionWebOpticianDD::OpticianID), -1) ;
				}else{
					m_nOpticianID = -1;
				}
				// (j.dinatale 2012-05-02 15:20) - PLID 49758 - refresh the optician list
				m_VisionWebOpticianCombo->Requery();
			}
		}
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

void CInvVisionWebDlg::DblClickCellVisionwebOrderList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		CInvVisionWebDlg::OnEditOrder();
		}
	NxCatchAll(__FUNCTION__);
}

// (s.dhole 2011-03-14 18:04) - PLID PLID 42835  Dialog to Maintain Glasses Catalog custom items
void CInvVisionWebDlg::OnBnClickedBtnGlassesCatalogSetup()
{
	// TODO: Add your control notification handler code here
	// (s.dhole 2011-03-14 11:17) - PLID 42795 Setup Glasses Catalogs
	try {
	 CInvGlassesCatalogSetupDlg dlg (this);
	 dlg.DoModal(); 
	 	}
	NxCatchAll(__FUNCTION__);
}

// (s.dhole 2011-04-11 17:13) - PLID 43077 Call Manage Order status Dlg
// (j.dinatale 2013-02-26 11:56) - PLID 52849 - formatted this function
void CInvVisionWebDlg::OnVisionwebOrderpopupChangeOrderStatus()
{
	try{
		IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
		if (pRow != NULL){	
			CInvGlassesOrderStatusDlg dlg(this);
			long nOrderId = VarLong(pRow->GetValue(vwolOrderID), -1);
			bool bIsVisionWeb = (VarLong(pRow->GetValue(vwolGlassesOrderProcessTypeID), -1) == 2);
			dlg.m_strCaption = FormatString("%s;  Order #: %li ", VarString(pRow->GetValue(vwolPatientName)), nOrderId);
			dlg.m_IsVisionWeb = bIsVisionWeb;

			if (dlg.DoModal() == IDOK) {
				int nStatusID = dlg.m_nOrderStatus ;
				CString strStatus = dlg.m_strOrderStatusMsg ;
				_RecordsetPtr rsOrder = CreateParamRecordset("SELECT ID, GlassesOrderType, GETDATE() AS dt, UserSetOrderStatus AS BeenWarned FROM GlassesOrderT WHERE ID = {INT} ", nOrderId);

				if(!rsOrder->eof){
					long nUserID = GetCurrentUserID();	
					long OrderType = AdoFldByte(rsOrder->Fields, "GlassesOrderType");
					COleDateTime dt = AdoFldDateTime(rsOrder->Fields, "dt");
					_variant_t varUpdateDate = COleVariant(dt);
					bool bHasBeenWarned = !!AdoFldBool(rsOrder, "BeenWarned", FALSE);

					// (j.dinatale 2013-02-26 12:01) - PLID 52849 - need to warn if this is a vision web order because we want the user to know
					//		that we wont be automagically updating the order status anymore
					if(bIsVisionWeb && !bHasBeenWarned){
						long nResult = this->MessageBox(
							"If you manually set the order status of a Vision Web order, the order's status will no longer "
							"be updated automatically via Vision Web. Are you sure you want to continue?", 
							"Vision Web Order Status Warning", MB_YESNO | MB_ICONWARNING);

						if(IDNO == nResult){
							return;
						}
					}

					CSqlFragment sql;
					// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
					sql += CSqlFragment(
						"UPDATE GlassesOrderHistoryT SET IsActive = 0 WHERE IsActive <> 0 AND GlassesOrderID = {INT}; \r\n"
						"INSERT INTO GlassesOrderHistoryT "
						"(GlassesOrderID, GlassesOrderType, GlassesOrderStatusID, MessageType, Note, IsActive, UpdateDate, UserID)"
						"VALUES ({INT}, {INT}, {INT}, {STRING}, {STRING}, 1, {VT_DATE}, {INT}) \r\n", 
						nOrderId, nOrderId, OrderType, nStatusID, "Custom", strStatus, varUpdateDate, nUserID
					); // we should update only active record

					// (s.dhole 2011-05-26 10:51) - PLID  43077  We need to update Message inn order table
					// (j.dinatale 2013-02-26 11:56) - PLID 52849 - need to update our new flag to indicate that the user manually set the status
					if (VarLong(pRow->GetValue(vwolGlassesOrderProcessTypeID), -1) == 2){
						sql += CSqlFragment("UPDATE GlassesOrderT SET GlassesOrderStatusID = {INT}, UpdateDate = {VT_DATE}, GlassesMessage = {STRING}, "
							"UserSetOrderStatus = 1 WHERE ID = {INT}", nStatusID, varUpdateDate, strStatus, nOrderId);
					} else {
						// (s.dhole 2011-04-26 15:57) - PLID 43077 added GlassesOrderProcessTypeID=1 non VisionWeb process flag
						sql += CSqlFragment("UPDATE GlassesOrderT SET GlassesOrderStatusID = {INT}, UpdateDate = {VT_DATE}, GlassesMessage = {STRING}, "
							"GlassesOrderProcessTypeID = 1, UserSetOrderStatus = 1 WHERE ID = {INT}", nStatusID, varUpdateDate, strStatus, nOrderId);
					}
					ExecuteParamSql(sql);

					IRowSettingsPtr pStatusRow = m_VisionWebOrderStatusCombo->FindByColumn(vwosOrderStatusID, nStatusID, NULL, VARIANT_FALSE);   // Cast as long
					pRow->PutValue(vwolMessage, _variant_t(strStatus));
					pRow->PutValue(vwolLastUpdatDate, varUpdateDate);
					pRow->PutValue(vwolOrderStatusID, nStatusID);
					pRow->PutValue(vwolOrderStatusName, _variant_t(pStatusRow->GetValue(vwosOrderStatusName)));
					pRow->ForeColor = pStatusRow->ForeColor;
				}
			}
		}
	} NxCatchAll("CInvVisionWebDlg::OnVisionwebOrderpopupChangeOrderStatus");
}

// (j.dinatale 2012-02-20 17:25) - PLID 47402 - dont filter our orders
void CInvVisionWebDlg::OnBnClickedAllOrders()
{
	try{
		m_radioFilterAll.SetCheck(TRUE);
		m_radioFilterBilled.SetCheck(FALSE);
		m_radioFilterNotBilled.SetCheck(FALSE);

		SetRemotePropertyInt("GlassesOrderBilledStatusFilter", 0, 0, GetCurrentUserName());

		ReFilterVisionWebOrderList();
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-02-20 17:25) - PLID 47402 - filter only not billed orders
void CInvVisionWebDlg::OnBnClickedNotBilledOrders()
{
	try{
		m_radioFilterAll.SetCheck(FALSE);
		m_radioFilterBilled.SetCheck(FALSE);
		m_radioFilterNotBilled.SetCheck(TRUE);

		SetRemotePropertyInt("GlassesOrderBilledStatusFilter", 2, 0, GetCurrentUserName());

		ReFilterVisionWebOrderList();
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-02-20 17:25) - PLID 47402 - filter only billed orders
void CInvVisionWebDlg::OnBnClickedBilledOrders()
{
	try{
		m_radioFilterAll.SetCheck(FALSE);
		m_radioFilterBilled.SetCheck(TRUE);
		m_radioFilterNotBilled.SetCheck(FALSE);

		SetRemotePropertyInt("GlassesOrderBilledStatusFilter", 1, 0, GetCurrentUserName());

		ReFilterVisionWebOrderList();
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-02-22 17:24) - PLID 48326 - go to patient plox!
void CInvVisionWebDlg::OnGoToPatient()
{
	try{
		IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();

		if(pRow){
			long nPatientID = VarLong(pRow->GetValue(vwolPatientID), -1);

			if (nPatientID != -1 && GetMainFrame()) {
				GetMainFrame()->GotoPatient(nPatientID);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-03-15 09:47) - PLID 47413 - lets bill it!
void CInvVisionWebDlg::OnBillOrder()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_VisionWebOrderList->CurSel;

		if (GetMainFrame()->IsBillingModuleOpen(true)) {
			return;
		}

		if(pRow == NULL) {
			return;
		}

		long nBillInsuredPartyID = -1;
		long nPatientID = VarLong(pRow->GetValue(vwolPatientID), -1);
		long nGlassesOrderID = VarLong(pRow->GetValue(vwolOrderID), -1);

		if(nPatientID == -1 || nGlassesOrderID == -1){
			return;
		}

		if(!CheckCurrentUserPermissions(bioBill,sptCreate)){
			return;
		}

		if(!ReturnsRecordsParam("SELECT TOP 1 1 FROM GlassesOrderServiceT WHERE GlassesOrderID = {INT}", nGlassesOrderID)){
			MessageBox("This order cannot be billed because it has no services or products associated with it.", "NexTech Practice", MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		// (j.dinatale 2012-04-16 13:48) - PLID 47413 - no longer need a context menu since we can have vision and patient resp right on the optical order
		//Now just flip to the patient's module and set the active Patient
		if(GetMainFrame()) {
			GetMainFrame()->GotoPatient(nPatientID);
			g_Modules[Modules::Patients]->ActivateTab(PatientsModule::BillingTab);

			//now open a bill
			CPatientView* pPatView = (CPatientView*)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
			if(pPatView){
				CBillingModuleDlg *pBillingDlg = pPatView->GetBillingDlg();

				if(pBillingDlg){
					pBillingDlg->m_pFinancialDlg = pPatView->GetFinancialDlg();
					pBillingDlg->m_nPatientID = GetActivePatientID();
					pBillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1);

					pBillingDlg->PostMessage(NXM_BILL_GLASSES_ORDER, nGlassesOrderID, -1);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CInvVisionWebDlg::OnRequeryFinishedOptician(short nFlags)
{
	try{
		// (j.dinatale 2012-05-02 15:42) - PLID 49758 - find the last selection on the optician list
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_VisionWebOpticianCombo->FindByColumn(VisionWebOpticianDD::OpticianID, m_nOpticianID, NULL, VARIANT_FALSE);
		if(pRow){
			m_VisionWebOpticianCombo->CurSel = pRow;
		}else{
			m_VisionWebOpticianCombo->CurSel = m_VisionWebOpticianCombo->TopRow;
		}

		ReFilterVisionWebOrderList();
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2013-04-12 15:16) - PLID 56214 - Location Filter! Pow!
void CInvVisionWebDlg::SelChangingVisionWebLocationFilter(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try{
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2013-04-12 15:16) - PLID 56214 - Location Filter! Pow!
void CInvVisionWebDlg::SelChosenVisionWebLocationFilter(LPDISPATCH lpRow)
{
	try{
		IRowSettingsPtr pRow = m_VisionWebOrderList->GetCurSel();
		if (pRow!=NULL)
			m_nOrderID = VarLong(pRow->GetValue(vwolOrderID), -1);	
		else 
			m_nOrderID = -1;

		ReFilterVisionWebOrderList();
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2013-04-12 15:16) - PLID 56214 - Location Filter! Pow!
void CInvVisionWebDlg::RequeryFinishedVisionWebLocationFilter(short nFlags)
{
	try{
		IRowSettingsPtr pAllRow = m_VisonWebLocationList->FindByColumn(VisionWebLocationDD::ID, -1, NULL, VARIANT_FALSE);// Cast as long
		m_VisonWebLocationList->CurSel  = pAllRow ;
	}NxCatchAll(__FUNCTION__);
}

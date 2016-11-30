// VisionWebOrderHistory.cpp : implementation file
//
// (s.dhole 2010-11-08 17:30) - PLID 41384 new DLG
#include "stdafx.h"
#include "Practice.h"
#include "VisionWebOrderHistory.h"


// CVisionWebOrderHistory dialog

IMPLEMENT_DYNAMIC(CVisionWebOrderHistory, CNxDialog)

CVisionWebOrderHistory::CVisionWebOrderHistory(CWnd* pParent /*=NULL*/)
	: CNxDialog(CVisionWebOrderHistory::IDD, pParent)
{
		
}

BOOL CVisionWebOrderHistory::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try {
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		// (r.wilson 5/11/2102) PLID  43741 - Changed column name from GlassesOrderStatus to GlassesOrderStatusID.  Also joined a table
		SetWindowText(FormatString("VisionWeb Order History :-   Order #:%d ;  VisionWeb Order #:%s ", m_nVisionWebOrderID,m_strVisionWebOrderID)); 
			m_BtnVisionWebOrderHistoryClose.AutoSet(NXB_CLOSE);
			m_VisionWebOrderHistoryList= BindNxDataList2Ctrl(IDC_VISIONWEB_ORDER_STATUS_LIST  ,GetRemoteData(), false);
			m_VisionWebOrderHistoryList->FromClause = _bstr_t(FormatString("(SELECT GlassesOrderHistoryT.UpdateDate, GlassesOrderT.ID, "
			" GlassesOrderT.Description, GlassesOrderHistoryT.GlassesOrderType,  "
			" GlassesOrderHistoryT.GlassesOrderStatusID,  "
			" GlassesOrderStatusT.OrderStatusName AS OrderStatusName,  "
			" GlassesOrderHistoryT.MessageType, GlassesOrderHistoryT.Note,  "
			" GlassesOrderHistoryT.IsActive, CASE GlassesOrderHistoryT.IsActive  "
			" WHEN 0 THEN '' ELSE 'Active' END AS ActiveStatus,  "
			" UsersT.UserName, GlassesOrderHistoryT.ID AS GlassesOrderHistoryID,  "
			" GlassesOrderHistoryT.SupplierTrackingId,  "
			" CONVERT(DATETIME , CASE GlassesOrderHistoryT.SupplierTimeStamp  WHEN '' THEN  NULL ELSE GlassesOrderHistoryT.SupplierTimeStamp END) AS SupplierTimeStamp ,"
			" GlassesOrderT.GlassesOrderNumber "
			" FROM GlassesOrderHistoryT INNER JOIN "
			" GlassesOrderT ON GlassesOrderHistoryT.GlassesOrderID = GlassesOrderT.ID INNER JOIN "
			" UsersT ON GlassesOrderHistoryT.UserID = UsersT.PersonID INNER JOIN " 
			" GlassesOrderStatusT ON GlassesOrderHistoryT.GlassesOrderStatusID = GlassesOrderStatusT.ID "
			" WHERE GlassesOrderT.IsDelete=0 AND GlassesOrderT.ID = %d ) AS GlassesOrderStatusQ ",m_nVisionWebOrderID)) ;
			m_VisionWebOrderHistoryList->Requery(); 
	}NxCatchAll("Error in CVisionWebOrderHistory::OnInitDialog");
	return FALSE;
}
CVisionWebOrderHistory::~CVisionWebOrderHistory()
{
}

void CVisionWebOrderHistory::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
		DDX_Control(pDX, IDCANCEL, m_BtnVisionWebOrderHistoryClose);
		DDX_Control(pDX, IDC_NEW_VISIONWEB_ORDER, m_BtnVisionWebOrderHistoryClose);
		
}


BEGIN_MESSAGE_MAP(CVisionWebOrderHistory, CNxDialog)
END_MESSAGE_MAP()


// CVisionWebOrderHistory message handlers

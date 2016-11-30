// InvVisionWebLocationDlg.cpp : implementation file

// (s.dhole 2011-03-18 08:54) - PLID 42897 standalone interface to assign VisionWeb location and ID to specific supplier
#include "stdafx.h"
#include "Practice.h"
#include "InvVisionWebLocationDlg.h"
#include "InventoryRc.h"
#include "InternationalUtils.h"


using namespace NXDATALIST2Lib;
using namespace ADODB;
// CInvVisionWebLocationDlg dialog

IMPLEMENT_DYNAMIC(CInvVisionWebLocationDlg, CNxDialog)

// enum for supplier location list
enum VisionWebOrderListColumns {
	vwslID = 0,
	vwslLocationName,
	vwslLocationCode,
	vwslLocationID,
};

CInvVisionWebLocationDlg::CInvVisionWebLocationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvVisionWebLocationDlg::IDD, pParent)
{}

CInvVisionWebLocationDlg::~CInvVisionWebLocationDlg()
{}


BEGIN_MESSAGE_MAP(CInvVisionWebLocationDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_ADD_LOCATION , OnBtnAddLocation)
	ON_WM_CLOSE() 
	ON_BN_CLICKED(IDCANCEL, &CInvVisionWebLocationDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CInvVisionWebLocationDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		extern CPracticeApp theApp;
		GetDlgItem(IDC_SELECTED_SUPPLIERNAME)->SetFont(&theApp.m_boldFont);
		// (s.dhole 2012-05-25 17:55) - PLID 50617
		SetDlgItemText(IDC_SELECTED_SUPPLIERNAME, ConvertToControlText(FormatString("Supplier/Lab: %s",  m_strSupplier)));
		m_btnAddLocation.AutoSet(NXB_NEW);
		m_btnClose.AutoSet(NXB_CLOSE);
		GetDlgItem(IDC_BTN_ADD_LOCATION )->EnableWindow(FALSE);
		GetDlgItem(IDC_LOCATION_VISIONWEB_CODE)->EnableWindow(FALSE);
		m_nxeditLocationCode.SetLimitText(8);
		m_LocationListCombo  = BindNxDataList2Ctrl(IDC_VISONWEB_LOCATION_COMBO ,GetRemoteData(),FALSE );
		m_SupplierLocationList= BindNxDataList2Ctrl(IDC_VISIONWEB_SUPPLIER_LOCATION_LIST ,GetRemoteData(),FALSE );
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		CString  strFrom;
		strFrom.Format (" (SELECT  -1 AS  ID,'' AS  Name   \r\n"
						  " UNION \r\n"
						  " SELECT    ID, Name FROM LocationsT \r\n"
						  " WHERE     (Active = 1) AND (TypeID = 1)  \r\n"
						  " and ID not in (SELECT     LocationID  \r\n"
						  " FROM GlassesSupplierLocationsT WHERE LocationID Is Not Null AND (SupplierID = %d ))  ) AS LocationQ  \r\n"
						  ,m_nSupplierID);
		m_LocationListCombo ->FromClause =_bstr_t(strFrom);
		m_LocationListCombo ->Requery(); 
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		strFrom.Format ("(SELECT LocationsT.Name, GlassesSupplierLocationsT.ID, GlassesSupplierLocationsT.LocationID,  \r\n"
						" GlassesSupplierLocationsT.VisionWebAccountID \r\n"
						" FROM GlassesSupplierLocationsT INNER JOIN \r\n"
						" LocationsT ON GlassesSupplierLocationsT.LocationID = LocationsT.ID \r\n"
						" WHERE (GlassesSupplierLocationsT.SupplierID =  %d) AND   LocationsT.TypeID = 1 AND LocationsT.Active = 1 ) AS SupplerLocationQ " ,m_nSupplierID );
		m_SupplierLocationList ->FromClause =_bstr_t(strFrom);
		m_SupplierLocationList ->Requery() ;
		
	}NxCatchAll("Error in CInvVisionWebLocationDlg::OnInitDialog");
	return TRUE;
}




void CInvVisionWebLocationDlg::OnClose() 
{
	CNxDialog::OnCancel();	
}

void CInvVisionWebLocationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOCATION_VISIONWEB_CODE  , m_nxeditLocationCode);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_BTN_ADD_LOCATION, m_btnAddLocation);
	DDX_Control(pDX, IDC_SELECTED_SUPPLIERNAME, m_nxstaticSupplierName);
}

BEGIN_EVENTSINK_MAP(CInvVisionWebLocationDlg, CNxDialog)
	ON_EVENT(CInvVisionWebLocationDlg, IDC_VISIONWEB_SUPPLIER_LOCATION_LIST, 9, CInvVisionWebLocationDlg::EditingFinishingVisionwebSupplierLocationList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CInvVisionWebLocationDlg, IDC_VISONWEB_LOCATION_COMBO, 18 /* RequeryFinished */, CInvVisionWebLocationDlg::OnRequeryFinishedLocationComboList, VTS_I2)
	ON_EVENT(CInvVisionWebLocationDlg, IDC_VISONWEB_LOCATION_COMBO, 16, CInvVisionWebLocationDlg::SelChosenComboLocation, VTS_DISPATCH)
	ON_EVENT(CInvVisionWebLocationDlg, IDC_VISIONWEB_SUPPLIER_LOCATION_LIST, 6 /* RButtonDown */, CInvVisionWebLocationDlg::OnRButtonDownSupplierLocation, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
END_EVENTSINK_MAP()




// (s.dhole 2010-11-15 13:31) - PLID  41743 Assign Location to supplier
void CInvVisionWebLocationDlg::OnBtnAddLocation()
{
try{
		IRowSettingsPtr pRow= m_LocationListCombo->GetCurSel() ;
		long nLocationID =0;
		CString strLocationName;
		if(pRow != NULL) {
			nLocationID   = VarLong(pRow->GetValue(0));
			strLocationName= VarString(pRow->GetValue(1));
		}
		else{
			return;
		}
		if (nLocationID ==-1)
			return;
		CString  strSupplierLocationCode;
		GetDlgItemText(IDC_LOCATION_VISIONWEB_CODE, strSupplierLocationCode);	 
		strSupplierLocationCode.TrimRight();
		strSupplierLocationCode.TrimLeft();
		strSupplierLocationCode.Replace(" ","") ;
		if(strSupplierLocationCode.IsEmpty() || (atol(strSupplierLocationCode) <=0 )  ) {
			AfxMessageBox("Invalid VisionWeb account code\nPlease make sure that the VisionWeb account code you entered is the same as the code you received from VisionWeb during location registration.");
			return;
		}
		strSupplierLocationCode.Left(8); 
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		ExecuteParamSql("if not exists(SELECT * FROM GlassesSupplierLocationsT  WHERE LocationID = {INT} AND SupplierID = {INT}  ) \r\n"
					" BEGIN  \r\n"
					" INSERT INTO GlassesSupplierLocationsT (  SupplierID ,LocationID,VisionWebAccountID) \r\n"
					" VALUES ({INT},{INT}, {INT} )  \r\n"
					" END  \r\n"
					" ELSE  \r\n"
					" BEGIN  \r\n"
					" UPDATE GlassesSupplierLocationsT SET VisionWebAccountID ={INT}    \r\n"
					" WHERE  LocationID = {INT} AND SupplierID = {INT}   \r\n"
					" END  \r\n",
					nLocationID,m_nSupplierID,m_nSupplierID ,nLocationID, atol(strSupplierLocationCode),atol(strSupplierLocationCode) ,nLocationID,m_nSupplierID);

		_RecordsetPtr rs = CreateParamRecordset("SELECT * FROM GlassesSupplierLocationsT  WHERE LocationID = {INT} AND SupplierID = {INT}  "
			,nLocationID,m_nSupplierID);
		if(!rs->eof){ 
			m_LocationListCombo->RemoveRow(  pRow);
			m_LocationListCombo->CurSel=m_LocationListCombo->FindByColumn(0,-1L , NULL, VARIANT_FALSE); // Cast ID when it pass to FindByColumn as -1L
			pRow = m_SupplierLocationList->GetNewRow();
			pRow->PutValue(vwslID, AdoFldLong(rs, "ID"));
			pRow->PutValue(vwslLocationID, nLocationID);
			pRow->PutValue(vwslLocationName ,_variant_t( strLocationName) );
			pRow->PutValue(vwslLocationCode , VarLong(atol(strSupplierLocationCode)));
			m_SupplierLocationList->AddRowSorted(pRow ,NULL);
		}
		SetDlgItemText (IDC_LOCATION_VISIONWEB_CODE, CString("") );	 
		m_nxeditLocationCode.EnableWindow(FALSE);
		m_btnAddLocation.EnableWindow(FALSE);
	}NxCatchAll("Error in  CInvVisionWebLocationDlg::OnBtnAddLocation");
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 Remove location  to supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvVisionWebLocationDlg::RemoveLocation()
{
try{
		IRowSettingsPtr pRow= m_SupplierLocationList->GetCurSel();
		long nID =0;
		if(pRow != NULL) 
			nID  = VarLong(pRow->GetValue(vwslID));
		else
			return;
		_RecordsetPtr rs = CreateParamRecordset("SELECT Count(SupplierID) as recCount   FROM GlassesOrderT  \r\n"
			" WHERE  GlassesSupplierLocationID = {INT} ",	 nID);
		if(!rs->eof){ 
			 if (AdoFldLong(rs, "recCount",0)>0 ){
				AfxMessageBox(FormatString( "There are %d active or deleted orders using this location. You cannot remove a location that is in use.",AdoFldLong(rs, "recCount",0)) );
				return;
			 }
		}
		ExecuteParamSql(" DELETE  FROM GlassesSupplierLocationsT  WHERE ID = {INT} ",nID );
		m_LocationListCombo ->Requery(); 
		m_SupplierLocationList ->RemoveRow(pRow) ; 
	}NxCatchAll("Error in CInvVisionWebLocationDlg::RemoveLocation");
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 
// Create right click menu
void CInvVisionWebLocationDlg::GenerateMenu(int MenuTypeID, CPoint &pt) 
{
	try
	{
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, MenuTypeID, "Remove");
		int nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN| TPM_RIGHTBUTTON |TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
		if (nCmdId==menuSupplierLocation)
			RemoveLocation();
		mnu.DestroyMenu();
	}NxCatchAll("Error in CInvVisionWebLocationDlg::GenerateMenu");
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebLocationDlg::OnRButtonDownSupplierLocation(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL)
			return;
		// this was missing code, need to select row when user click 
		m_SupplierLocationList->PutCurSel(pRow);
		CPoint pt;
		GetCursorPos(&pt);
		GenerateMenu(menuSupplierLocation,pt);
	}NxCatchAll("Error in CInvVisionWebLocationDlg::OnRButtonDownSupplierLocation");	
}



// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebLocationDlg::EditingFinishingVisionwebSupplierLocationList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try{
		IRowSettingsPtr pRow(lpRow);
		long nID =0;
		if(pRow != NULL) 
			nID   = VarLong(pRow->GetValue(vwslID));
		else
			*pbCommit = FALSE;
		long  nCode =VarLong(*pvarNewValue );
		
		if (nCode  <=0 )   
		{
			// (s.dhole 2012-05-25 17:55) - PLID  50617
			 //s.dhole Fix capitalzation
			AfxMessageBox("Invalid VisionWeb supplier/lab location account code\nPlease make sure that the VisionWeb account code you entered is the same as the code you received from VisionWeb during location registration.");
			*pbCommit = FALSE;
			return;
		}
	// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		ExecuteParamSql("UPDATE GlassesSupplierLocationsT SET VisionWebAccountID ={INT}    \r\n"
					" WHERE  ID = {INT}  ",nCode  ,nID );
	}NxCatchAll("Error in CInvVisionWebLocationDlg::EditingFinishingVisionwebSupplierLocationList");
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebLocationDlg::SelChosenComboLocation(LPDISPATCH lpRow)
{
	try{
	IRowSettingsPtr pRow= m_LocationListCombo->CurSel;
	if (pRow !=NULL)
		 if (VarLong(pRow->GetValue(0)) >-1){
			m_nxeditLocationCode.EnableWindow(TRUE);
			m_btnAddLocation.EnableWindow(TRUE);
		}
		 else{
			m_nxeditLocationCode.EnableWindow(FALSE);
			m_btnAddLocation.EnableWindow(FALSE);
		}
	}NxCatchAll("Error in CInvVisionWebLocationDlg::SelChosenComboLocation");
}
// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebLocationDlg::OnRequeryFinishedLocationComboList(short nFlags)
{
	try{
		IRowSettingsPtr pRow= m_LocationListCombo->FindByColumn(0,-1L , NULL, VARIANT_FALSE); // Cast ID when it pass to FindByColumn as -1L
		if (pRow !=NULL)
			m_LocationListCombo->CurSel = m_LocationListCombo->FindByColumn(0,-1L , NULL, VARIANT_FALSE); // Cast ID when it pass to FindByColumn as -1L
		m_nxeditLocationCode.EnableWindow(FALSE);
		m_btnAddLocation.EnableWindow(FALSE);
		SetDlgItemText(IDC_LOCATION_VISIONWEB_CODE ,CString(""));
		}NxCatchAll("Error in CInvVisionWebLocationDlg::OnRequeryFinishedLocationComboList");
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebLocationDlg::OnBnClickedCancel()
{
	try{ 
		CNxDialog::OnCancel();	
	}NxCatchAll("Error in CInvVisionWebLocationDlg::OnBnClickedCancel");
}

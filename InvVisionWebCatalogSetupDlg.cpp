// InvVisionWebCatalogSetupDlg.cpp : implementation file
// (s.dhole 2010-11-15 13:31) - PLID  41743 VisionWeb Suppler Catalog Setup

#include "stdafx.h"
#include "Practice.h"
#include "InvVisionWebCatalogSetupDlg.h"
#include "InvVisionWebUtils.h"
#include "MultiSelectDlg.h"
#include "InventoryRc.h"
#include "InternationalUtils.h"


using namespace NXDATALIST2Lib;
using namespace ADODB;
// CInvVisionWebCatalogSetupDlg dialog

IMPLEMENT_DYNAMIC(CInvVisionWebCatalogSetupDlg, CNxDialog)

// enum for supplier location list
enum VisionWebOrderListColumns {
	vwslID = 0,
	vwslLocationName,
	vwslLocationCode,
	vwslLocationID,
};

CInvVisionWebCatalogSetupDlg::CInvVisionWebCatalogSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvVisionWebCatalogSetupDlg::IDD, pParent)
{}

CInvVisionWebCatalogSetupDlg::~CInvVisionWebCatalogSetupDlg()
{}


BEGIN_MESSAGE_MAP(CInvVisionWebCatalogSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_ADD_LOCATION , OnBtnAddLocation)
	ON_BN_CLICKED(IDC_BTN_VISIONWEBSYNC, OnBtnVisionWebSync)
	ON_BN_CLICKED( IDC_BTN_FRAME_TYPE, OnBtnAddFrameType)
	ON_BN_CLICKED( IDC_BTN_DESIGN , OnBtnAddDesigns)
	ON_BN_CLICKED( IDC_BTN_MATERIAL, OnBtnAddMaterials)
	ON_BN_CLICKED( IDC_BTN_TREATMENTS, OnBtnAddTreatments)
	ON_WM_CLOSE() 
	ON_BN_CLICKED(IDCANCEL, &CInvVisionWebCatalogSetupDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CInvVisionWebCatalogSetupDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		g_propManager.CachePropertiesInBulk("VisionWebCatalog", propDateTime,
			"(Username = '<None>') AND ("
			"Name = 'VisionWebCatalogSyncDate'"
			")");
		COleDateTime dtNow= COleDateTime::GetCurrentTime();  // to get advance date as default date
		COleDateTimeSpan dtDays;
		dtDays.SetDateTimeSpan(200,0,0,0);
		dtNow +=dtDays;
		COleDateTime dtSyncDate= GetRemotePropertyDateTime("VisionWebCatalogSyncDate",&dtNow,0,"<None>");
		if (dtSyncDate.GetStatus()==COleDateTime::valid && dtSyncDate<dtNow) {
			SetDlgItemText(IDC_STATIC_CATALOG_SYNC, FormatString("Catalog last updated on: %s", FormatDateTimeForInterface(  dtSyncDate,dtoNaturalDatetime )));
		}
		extern CPracticeApp theApp;
		GetDlgItem(IDC_SELECTED_SUPPLIERNAME)->SetFont(&theApp.m_boldFont);
		SetDlgItemText(IDC_SELECTED_SUPPLIERNAME, ConvertToControlText(FormatString("Supplier: %s",  m_strSupplier)));
		m_btnAddFrameType.AutoSet(NXB_NEW);
		m_btnAddDesigns.AutoSet(NXB_NEW);
		m_btnAddMaterials.AutoSet(NXB_NEW);
		m_btnAddTreatments.AutoSet(NXB_NEW);
		m_btnAddLocation.AutoSet(NXB_NEW);
		m_btnVisionWebSync.AutoSet(NXB_REFRESH);
		m_btnClose.AutoSet(NXB_CLOSE);
		GetDlgItem(IDC_BTN_ADD_LOCATION )->EnableWindow(FALSE);
		GetDlgItem(IDC_LOCATION_VISIONWEB_CODE)->EnableWindow(FALSE);
		m_nxeditLocationCode.SetLimitText(8);

		m_SelectedFrameTypeList = BindNxDataList2Ctrl(IDC_SELECTED_FRAMETYPE_LIST,GetRemoteData(),FALSE);
		m_SelectedDesignsList = BindNxDataList2Ctrl(IDC_SELECTED_DESIGNS_LIST,GetRemoteData(),FALSE);
		m_SelectedMaterialsList = BindNxDataList2Ctrl(IDC_SELECTED_MATERIALS_LIST,GetRemoteData(),FALSE);
		m_SelectedTreatmentsList = BindNxDataList2Ctrl(IDC_SELECTED_TREATMENTS_LIST,GetRemoteData(),FALSE);
		m_SupplierLocationList = BindNxDataList2Ctrl(IDC_VISIONWEB_SUPPLIER_LOCATION_LIST,GetRemoteData(),FALSE );
		m_LocationListCombo  = BindNxDataList2Ctrl(IDC_VISONWEB_LOCATION_COMBO ,GetRemoteData(),FALSE );
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		CString  strWhere;
		m_SelectedFrameTypeList->FromClause = _bstr_t("GlassesCatalogFrameTypesT");
		strWhere.Format( "ID  in ( Select GlassesCatalogFrameTypesID From GlassesSupplierCatalogFrameTypesT "
				" Where  IsActive=1 AND  SupplierID =%d ) ", m_nSupplierID );
		m_SelectedFrameTypeList->PutWhereClause(_bstr_t(strWhere));
	    m_SelectedFrameTypeList->Requery(); 
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		m_SelectedDesignsList->FromClause = _bstr_t("GlassesCatalogDesignsT");
		strWhere.Format( "ID in ( Select GlassesCatalogDesignsID From GlassesSupplierCatalogDesignsT "
				" Where  IsActive=1 AND SupplierID =%d ) ", m_nSupplierID );
		m_SelectedDesignsList->PutWhereClause(_bstr_t(strWhere));
	    m_SelectedDesignsList->Requery(); 
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		m_SelectedMaterialsList->FromClause = _bstr_t("GlassesCatalogMaterialsT");
		strWhere.Format( "ID in ( Select GlassesCatalogMaterialID From GlassesSupplierCatalogMaterialsT "
				" Where IsActive=1 AND  SupplierID =%d ) ", m_nSupplierID );
		m_SelectedMaterialsList->PutWhereClause(_bstr_t(strWhere));
	    m_SelectedMaterialsList->Requery(); 
		
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		m_SelectedTreatmentsList->FromClause = _bstr_t("GlassesCatalogTreatmentsT");
		strWhere.Format( "ID in ( Select GlassesCatalogTreatmentsID From GlassesSupplierCatalogTreatmentsT "
				" Where  IsActive=1 AND SupplierID =%d ) ", m_nSupplierID );
		m_SelectedTreatmentsList->PutWhereClause(_bstr_t(strWhere));
	    m_SelectedTreatmentsList->Requery(); 
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
		
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::OnInitDialog");
	return TRUE;
}




void CInvVisionWebCatalogSetupDlg::OnClose() 
{
	CNxDialog::OnCancel();	
}

void CInvVisionWebCatalogSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOCATION_VISIONWEB_CODE  , m_nxeditLocationCode);
	DDX_Control(pDX, IDC_BTN_FRAME_TYPE, m_btnAddFrameType);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_BTN_DESIGN  , m_btnAddDesigns);
	DDX_Control(pDX, IDC_BTN_MATERIAL,  m_btnAddMaterials );
	DDX_Control(pDX, IDC_BTN_TREATMENTS , m_btnAddTreatments);
	DDX_Control(pDX, IDC_BTN_VISIONWEBSYNC, m_btnVisionWebSync);
	DDX_Control(pDX, IDC_BTN_ADD_LOCATION, m_btnAddLocation);
	DDX_Control(pDX, IDC_SELECTED_SUPPLIERNAME, m_nxstaticSupplierName);
}

BEGIN_EVENTSINK_MAP(CInvVisionWebCatalogSetupDlg, CNxDialog)
	ON_EVENT(CInvVisionWebCatalogSetupDlg, IDC_VISIONWEB_SUPPLIER_LOCATION_LIST, 9, CInvVisionWebCatalogSetupDlg::EditingFinishingVisionwebSupplierLocationList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CInvVisionWebCatalogSetupDlg, IDC_VISONWEB_LOCATION_COMBO, 18 /* RequeryFinished */, CInvVisionWebCatalogSetupDlg::OnRequeryFinishedLocationComboList, VTS_I2)
	ON_EVENT(CInvVisionWebCatalogSetupDlg, IDC_VISONWEB_LOCATION_COMBO, 16, CInvVisionWebCatalogSetupDlg::SelChosenComboLocation, VTS_DISPATCH)
	ON_EVENT(CInvVisionWebCatalogSetupDlg, IDC_SELECTED_FRAMETYPE_LIST, 6 /* RButtonDown */, CInvVisionWebCatalogSetupDlg::OnRButtonDownFrameTypeList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CInvVisionWebCatalogSetupDlg, IDC_SELECTED_DESIGNS_LIST, 6 /* RButtonDown */, CInvVisionWebCatalogSetupDlg::OnRButtonDownDesignsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CInvVisionWebCatalogSetupDlg, IDC_SELECTED_MATERIALS_LIST, 6 /* RButtonDown */, CInvVisionWebCatalogSetupDlg::OnRButtonDownMaterialsList , VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CInvVisionWebCatalogSetupDlg, IDC_SELECTED_TREATMENTS_LIST, 6 /* RButtonDown */, CInvVisionWebCatalogSetupDlg::OnRButtonDownTreatmentsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CInvVisionWebCatalogSetupDlg, IDC_VISIONWEB_SUPPLIER_LOCATION_LIST, 6 /* RButtonDown */, CInvVisionWebCatalogSetupDlg::OnRButtonDownSupplierLocation, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
END_EVENTSINK_MAP()


// (s.dhole 2010-11-15 13:31) - PLID  41743 Assign frame type to supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvVisionWebCatalogSetupDlg::OnBtnAddFrameType()
{
	try{
		CMultiSelectDlg dlg;
		CString strFrom = "GlassesCatalogFrameTypesT";
		CString strWhere = FormatString("ID NOT IN \r\n"
			"(SELECT     GlassesCatalogFrameTypesID FROM GlassesSupplierCatalogFrameTypesT \r\n"
			" WHERE  IsActive=1 AND SupplierID =%d )", m_nSupplierID );
		CStringArray straryFields;
		CStringArray straryNames;
		straryFields.Add("FrameTypeCode");
		straryNames.Add("Code");
		//make sure user select atleast one item before click on ok button
		if(IDOK == dlg.Open(strFrom, strWhere, "ID", "FrameTypeName", "Select one or more Frame Type:",1,-1,&straryFields,&straryNames)) {
			CString strFilter = dlg.GetMultiSelectIDString();
			if (!strFilter.IsEmpty()){
				strFilter.Replace(" ", ", ");

				CString strSqlBatch = BeginSqlBatch();
				// we will mark all selected item to active  and rest of record w will insert .
				AddStatementToSqlBatch(strSqlBatch, "UPDATE GlassesSupplierCatalogFrameTypesT SET IsActive=1 \r\n"
					" WHERE SupplierID =%d AND GlassesCatalogFrameTypesID IN (%s)  \r\n"
				,m_nSupplierID,strFilter);

				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO GlassesSupplierCatalogFrameTypesT (GlassesCatalogFrameTypesID, SupplierID, IsActive)  \r\n"
				" SELECT ID, %d as  SupplierID , 1 AS IsActive FROM  GlassesCatalogFrameTypesT  \r\n"
				" WHERE  ID IN (%s) AND ID NOT IN   \r\n"
				" ( SELECT GlassesCatalogFrameTypesID FROM GlassesSupplierCatalogFrameTypesT    \r\n"
				" WHERE IsActive<>0 AND SupplierID =%d AND GlassesCatalogFrameTypesID  IN (%s))  \r\n"
				,m_nSupplierID,strFilter,m_nSupplierID,strFilter);

				if(!strSqlBatch.IsEmpty()) {
					ExecuteSqlBatch(strSqlBatch);
					m_SelectedFrameTypeList->Requery(); 
					}
			}
		}
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::OnBtnAddFrameType");
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 Assign frame type to supplier
void CInvVisionWebCatalogSetupDlg::RemoveFrameType()
{
	try{
		IRowSettingsPtr pRow= m_SelectedFrameTypeList->GetCurSel() ;
		long nID =0;
		if(pRow != NULL) 
			nID= VarLong(pRow->GetValue(0));
		else
			return;
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		_RecordsetPtr rs = CreateParamRecordset("SELECT Count(GlassesCatalogFrameTypeID) as recCount   FROM GlassesOrderT  "
			" WHERE SupplierID={INT} AND GlassesCatalogFrameTypeID = {INT} \r\n"	,m_nSupplierID	, nID );
		if(!rs->eof){ 
			 if (AdoFldLong(rs, "recCount",0)>0 ){
				AfxMessageBox(FormatString ("There are %d active or deleted orders using this Frame Type code. You cannot remove a Frame Type that is in use." ,AdoFldLong(rs, "recCount",0) )) ;
				return;
			 }
			 else{
				 ExecuteParamSql("DELETE  FROM GlassesSupplierCatalogFrameTypesT  WHERE IsActive=1  AND GlassesCatalogFrameTypesID = {INT} AND SupplierID = {INT}",nID, m_nSupplierID);
			 }	
		}
	m_SelectedFrameTypeList ->Requery(); 		
		
		
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::RemoveFrameType");
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 Assign Design to supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvVisionWebCatalogSetupDlg::OnBtnAddDesigns()
{
	try{
		CMultiSelectDlg dlg;
		CString strFrom = "GlassesCatalogDesignsT";
		CString strWhere = FormatString("ID NOT IN "
			"(SELECT     GlassesCatalogDesignsID FROM  GlassesSupplierCatalogDesignsT "
			" WHERE  IsActive=1 AND   SupplierID =  %d )", m_nSupplierID );
		CStringArray straryFields;
		CStringArray straryNames;
		straryFields.Add("DesignCode");
		straryNames.Add("Code");
		//make sure user select atleast one item before click on ok button
		if(IDOK == dlg.Open(strFrom, strWhere, "ID", "DesignName", "Select one or more Design:",1,-1,&straryFields,&straryNames)) {
			CString strFilter = dlg.GetMultiSelectIDString();
			if (!strFilter.IsEmpty()){
				strFilter.Replace(" ", ", ");
				CString strSqlBatch = BeginSqlBatch();
				AddStatementToSqlBatch(strSqlBatch, "UPDATE GlassesSupplierCatalogDesignsT SET IsActive=1 \r\n"
					" WHERE SupplierID =%d  and GlassesCatalogDesignsID IN (%s)  \r\n"
				,m_nSupplierID,strFilter);
				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO GlassesSupplierCatalogDesignsT (GlassesCatalogDesignsID, SupplierID, IsActive)  \r\n"
				" SELECT ID, %d as  SupplierID , 1 AS IsActive FROM  GlassesCatalogDesignsT  \r\n"
				" WHERE  ID IN (%s) AND ID NOT IN   \r\n"
				" ( SELECT GlassesCatalogDesignsID FROM GlassesSupplierCatalogDesignsT    \r\n"
				" WHERE IsActive<>0 AND SupplierID =%d AND GlassesCatalogDesignsID  IN (%s))  \r\n"
				,m_nSupplierID,strFilter,m_nSupplierID,strFilter);
				if(!strSqlBatch.IsEmpty()) {
					ExecuteSqlBatch(strSqlBatch);
					m_SelectedDesignsList->Requery(); 
					}
			}
		}
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::OnBtnAddDesigns");
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 Remove Design From supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvVisionWebCatalogSetupDlg::RemoveDesigns()
{
	try{
		IRowSettingsPtr pRow= m_SelectedDesignsList->GetCurSel() ;
		long nID =0;
		if(pRow != NULL) 
			nID= VarLong(pRow->GetValue(0));
		else
			return;
		_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) as recCount   FROM GlassesOrderT  \r\n"
			" WHERE  SupplierID={INT} AND ( RightGlassesOrderOtherInfoID IN  (Select ID From GlassesOrderOtherInfoT \r\n"
			" WHERE GlassesCatalogDesignsID = {INT}) \r\n"
			" OR \r\n"
			" leftGlassesOrderOtherInfoID IN (Select ID From GlassesOrderOtherInfoT \r\n"
			" WHERE GlassesCatalogDesignsID = {INT})) \r\n",m_nSupplierID, nID , nID );
		if(!rs->eof){ 
			 if (AdoFldLong(rs, "recCount",0)>0 ){
				AfxMessageBox(FormatString ("There are %d active or deleted orders using this Design code. You cannot remove a Design that is in use." ,AdoFldLong(rs, "recCount",0) )) ;
				return;
			 }
			 else{
				 ExecuteParamSql("DELETE  FROM GlassesSupplierCatalogDesignsT WHERE IsActive=1  AND GlassesCatalogDesignsID = {INT} AND SupplierID = {INT}",nID, m_nSupplierID);
			 }
		}
	
	m_SelectedDesignsList->Requery(); 
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::RemoveDesigns");
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 Assign Material to supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvVisionWebCatalogSetupDlg::OnBtnAddMaterials()
{
	try{
		CMultiSelectDlg dlg;
		CString strFrom = "GlassesCatalogMaterialsT";
		CString strWhere = FormatString("ID NOT IN \r\n"
			"(SELECT GlassesCatalogMaterialID  FROM GlassesSupplierCatalogMaterialsT \r\n"
			" WHERE  IsActive=1 AND   SupplierID =  %d )"
			, m_nSupplierID );
		CStringArray straryFields;
		CStringArray straryNames;
		straryFields.Add("MaterialCode");
		straryNames.Add("Code");
		//make sure user select atleast one item before click on ok button
		if(IDOK == dlg.Open(strFrom, strWhere, "ID", "MaterialName", "Select one or more Material:",1,-1,&straryFields,&straryNames)) {
			CString strFilter = dlg.GetMultiSelectIDString();
			if (!strFilter.IsEmpty()){
			strFilter.Replace(" ", ", ");
			CString strSqlBatch = BeginSqlBatch();
			AddStatementToSqlBatch(strSqlBatch, "UPDATE GlassesSupplierCatalogMaterialsT SET IsActive=1 \r\n"
				" WHERE SupplierID =%d AND GlassesCatalogMaterialID IN (%s)  \r\n"
			,m_nSupplierID,strFilter);

			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO GlassesSupplierCatalogMaterialsT (GlassesCatalogMaterialID, SupplierID, IsActive)  \r\n"
			" SELECT ID, %d as  SupplierID , 1 AS IsActive FROM  GlassesCatalogMaterialsT  \r\n"
			" WHERE  ID IN (%s) AND ID NOT IN   \r\n"
			" ( SELECT GlassesCatalogMaterialID FROM GlassesSupplierCatalogMaterialsT    \r\n"
			" WHERE IsActive<>0 AND SupplierID =%d AND GlassesCatalogMaterialID  IN (%s))  \r\n"
			,m_nSupplierID,strFilter,m_nSupplierID,strFilter);

			if(!strSqlBatch.IsEmpty()) {
				ExecuteSqlBatch(strSqlBatch);
				m_SelectedMaterialsList->Requery(); 
				}
			}
		}
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::OnBtnAddMaterials");
	
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 Remove Materials From supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvVisionWebCatalogSetupDlg::RemoveMaterials()
{
	try{
		IRowSettingsPtr pRow= m_SelectedMaterialsList->GetCurSel() ;
		long nID =0;
		if(pRow != NULL) 
			nID= VarLong(pRow->GetValue(0));
		else
			return;
		_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) as recCount   FROM GlassesOrderT \r\n"
			" WHERE SupplierID={INT} AND (RightGlassesOrderOtherInfoID IN  (Select ID From GlassesOrderOtherInfoT \r\n"
			" WHERE GlassesCatalogMaterialsID  = {INT}) \r\n"
			" OR \r\n"
			" leftGlassesOrderOtherInfoID IN (Select ID From GlassesOrderOtherInfoT \r\n"
			" WHERE GlassesCatalogMaterialsID  = {INT})) \r\n",m_nSupplierID ,nID , nID );
		if(!rs->eof)
		{ 
			 if (AdoFldLong(rs, "recCount",0)>0 ){
				AfxMessageBox(FormatString ("There are %d active or deleted orders using this Material code. You cannot remove a Material that is in use.",AdoFldLong(rs, "recCount",0) )) ;
				return;
			 }
			 else{
				 ExecuteParamSql("DELETE FROM GlassesSupplierCatalogMaterialsT WHERE IsActive=1 AND   GlassesCatalogMaterialID = {INT} AND SupplierID = {INT}",nID, m_nSupplierID);
			 }
		}
		
		m_SelectedMaterialsList->Requery();
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::RemoveMaterials");
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 Assign treatment to supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvVisionWebCatalogSetupDlg::OnBtnAddTreatments()
{
	try{
		CMultiSelectDlg dlg;
		CString strFrom = "GlassesCatalogTreatmentsT";
		CString strWhere = FormatString("ID NOT IN "
			"(SELECT     GlassesCatalogTreatmentsID FROM GlassesSupplierCatalogTreatmentsT "
			" WHERE  IsActive=1 AND   SupplierID =  %d )", m_nSupplierID );
		CStringArray straryFields;
		CStringArray straryNames;
		straryFields.Add("TreatmentCode");
		straryNames.Add("Code");
		//make sure user select atleast one item before click on ok button
		if(IDOK == dlg.Open(strFrom, strWhere, "ID", "TreatmentName", "Select one or more Treatment:",1,-1,&straryFields,&straryNames)) {
			CString strFilter = dlg.GetMultiSelectIDString();
			if (!strFilter.IsEmpty()){
				strFilter.Replace(" ", ", ");
				CString strSqlBatch = BeginSqlBatch();
				AddStatementToSqlBatch(strSqlBatch, "UPDATE GlassesSupplierCatalogTreatmentsT SET IsActive=1 \r\n"
					" WHERE SupplierID =%d AND SupplierID =%d and GlassesCatalogTreatmentsID IN (%s)  \r\n"
				,m_nSupplierID,m_nSupplierID,strFilter);

				AddStatementToSqlBatch(strSqlBatch, "INSERT INTO GlassesSupplierCatalogTreatmentsT (GlassesCatalogTreatmentsID, SupplierID, IsActive)  \r\n"
				" SELECT ID, %d as  SupplierID , 1 AS IsActive FROM  GlassesCatalogTreatmentsT  \r\n"
				" WHERE  ID IN (%s) AND ID NOT IN   \r\n"
				" ( SELECT GlassesCatalogTreatmentsID FROM GlassesSupplierCatalogTreatmentsT    \r\n"
				" WHERE IsActive<>0 AND SupplierID =%d AND GlassesCatalogTreatmentsID  IN (%s))  \r\n"
				,m_nSupplierID,strFilter,m_nSupplierID,strFilter);

				if(!strSqlBatch.IsEmpty()) {
					ExecuteSqlBatch(strSqlBatch);
					m_SelectedTreatmentsList->Requery(); 
				}
			}
		}
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::OnBtnAddTreatments");
	
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 Remove Treatments  from  supplier
void CInvVisionWebCatalogSetupDlg::RemoveTreatments()
{
	try{
		IRowSettingsPtr pRow= m_SelectedTreatmentsList->GetCurSel() ;
		long nID =0;
		if(pRow != NULL) 
			nID= VarLong(pRow->GetValue(0));
		else
			return;

		//TES 2/23/2011 - PLID 40539 - Check if this treatment is in use on the current order.
		bool bMatched = false;
		for(int i = 0; i < m_dwaCurrentOrderTreatments.GetSize() && !bMatched; i++) {
			if(m_dwaCurrentOrderTreatments[i] == nID) {
				bMatched = true;
			}
		}
		if(bMatched) {
			MsgBox("This treatment is in use on the order which is currently being edited.  Please remove the treatment from this order before "
				"removing it from the catalog");
			return;
		}
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) as recCount   FROM GlassesOrderT   \r\n"
			" WHERE SupplierID={INT} AND (RightGlassesOrderOtherInfoID IN (SELECT     GlassesOrderOtherInfoT.ID \r\n"
				" FROM  GlassesOrderTreatmentsT INNER JOIN  \r\n"
				" GlassesOrderOtherInfoT ON GlassesOrderTreatmentsT.GlassesOrderOtherInfoID = GlassesOrderOtherInfoT.ID \r\n"
				" WHERE GlassesOrderTreatmentsT.GlassesCatalogTreatmentID = {INT}) \r\n"
				" OR \r\n"
				" leftGlassesOrderOtherInfoID IN (SELECT     GlassesOrderOtherInfoT.ID \r\n"
				" FROM GlassesOrderTreatmentsT INNER JOIN  \r\n"
				" GlassesOrderOtherInfoT ON GlassesOrderTreatmentsT.GlassesOrderOtherInfoID = GlassesOrderOtherInfoT.ID \r\n"
				" WHERE GlassesOrderTreatmentsT.GlassesCatalogTreatmentID = {INT})) \r\n", m_nSupplierID,nID , nID );
		if(!rs->eof){ 
			if (AdoFldLong(rs, "recCount",0)>0 ){
			AfxMessageBox(FormatString("There are %d active or deleted orders using this Treatment code. You cannot remove a Treatment that is in use.",AdoFldLong(rs, "recCount",0) )) ;
			return;
			}
			else {
			 ExecuteParamSql("DELETE  FROM GlassesSupplierCatalogTreatmentsT  WHERE IsActive=1  AND GlassesCatalogTreatmentsID = {INT} AND SupplierID = {INT}",nID, m_nSupplierID);
			}	
		}
		
		m_SelectedTreatmentsList->Requery();
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::RemoveTreatments");
}



// (s.dhole 2010-11-15 13:31) - PLID  41743 Assign Location to supplier
void CInvVisionWebCatalogSetupDlg::OnBtnAddLocation()
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
	}NxCatchAll("Error in  CInvVisionWebCatalogSetupDlg::OnBtnAddLocation");
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 Remove location  to supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvVisionWebCatalogSetupDlg::RemoveLocation()
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
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::RemoveLocation");
}

// (s.dhole 2010-11-15 13:31) - PLID 40728 Sync  catalog information with visionWeb
void CInvVisionWebCatalogSetupDlg::OnBtnVisionWebSync()
{
try {
	CWaitCursor wc;
	CInvVisionWebUtils *VisionWebUtils= new CInvVisionWebUtils;
	if (VisionWebUtils->UpdateVisionWebCatalog() ==TRUE){
		COleDateTime dt ;
		_RecordsetPtr rsDT = CreateParamRecordset("SELECT  GetDate() as dt  ");
		if(!rsDT->eof) {
			dt= AdoFldDateTime( rsDT->Fields,"dt");
			SetRemotePropertyDateTime("VisionWebCatalogSyncDate",dt,0,"<None>");
			SetDlgItemText(IDC_STATIC_CATALOG_SYNC, FormatString("Catalog last updated on: %s", FormatDateTimeForInterface(  dt,dtoNaturalDatetime )));
		}
		m_SelectedFrameTypeList->Requery(); 
		m_SelectedDesignsList->Requery();
		m_SelectedMaterialsList ->Requery();
		m_SelectedTreatmentsList->Requery();
		AfxMessageBox("Catalog items have been successfully updated from the VisionWeb catalog database.");
	}
	else{
		AfxMessageBox("Failed to update Catalog items from the VisionWeb catalog database");
	}
	//Destroy object
	delete VisionWebUtils; 
}NxCatchAll(__FUNCTION__);
}



// (s.dhole 2010-11-15 13:31) - PLID  41743 
// Create right click menu
void CInvVisionWebCatalogSetupDlg::GenerateMenu(int MenuTypeID, CPoint &pt) 
{
	try
	{
		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, MenuTypeID, "Remove");
		int nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN| TPM_RIGHTBUTTON |TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
		switch(nCmdId) {
			case menuFrameType:
				RemoveFrameType();
				break;
			case menuDesign:
				RemoveDesigns();
				break;
			case menuMaterial:
				RemoveMaterials();
				break;
			case menuTreatment:
				RemoveTreatments();
				break;
			case menuSupplierLocation:
				RemoveLocation();
				break;
		}
		mnu.DestroyMenu();
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::GenerateMenu");
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebCatalogSetupDlg::OnRButtonDownSupplierLocation(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
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
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::OnRButtonDownSupplierLocation");	
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebCatalogSetupDlg::OnRButtonDownFrameTypeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL)
			return;
		// this was missing code, need to select row when user click 
		m_SelectedFrameTypeList->PutCurSel(pRow); 
		CPoint pt;
		GetCursorPos(&pt);
		GenerateMenu(menuFrameType,pt);
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::OnRButtonDownFrameTypeList");	
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebCatalogSetupDlg::OnRButtonDownDesignsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL)
			return;
		m_SelectedDesignsList->PutCurSel(pRow);
		CPoint pt;
		GetCursorPos(&pt);
		GenerateMenu(menuDesign,pt);
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::OnRButtonDownDesignsList");	
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebCatalogSetupDlg::OnRButtonDownTreatmentsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL)
			return;
		m_SelectedTreatmentsList->PutCurSel(pRow);
		CPoint pt;
		GetCursorPos(&pt);
		GenerateMenu(menuTreatment,pt );
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::OnRButtonDownTreatmentsList");	
}


void CInvVisionWebCatalogSetupDlg::OnRButtonDownMaterialsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL)
			return;
		m_SelectedMaterialsList->PutCurSel(pRow);
		CPoint pt;
		GetCursorPos(&pt);
		GenerateMenu(menuMaterial ,pt);
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::OnRButtonDownMaterialsList");	
}
// CInvVisionWebCatalogSetupDlg message handlers
// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebCatalogSetupDlg::EditingFinishingVisionwebSupplierLocationList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try{
		IRowSettingsPtr pRow(lpRow);
		long nID =0;
		if(pRow != NULL) 
			nID   = VarLong(pRow->GetValue(vwslID));
		else
			*pbCommit = FALSE;

		long  nCode =VarLong(*pvarNewValue );
		//strCode.TrimRight();
		//strCode.TrimLeft();
		//strCode.Replace(" ","") ;
		if (nCode  <=0 )   
		{
			AfxMessageBox("Invalid VisionWeb supplier location account code\nPlease make sure that the VisionWeb account code you entered is the same as the code you received from VisionWeb during location registration.");
			*pbCommit = FALSE;
			return;
		}
	// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		ExecuteParamSql("UPDATE GlassesSupplierLocationsT SET VisionWebAccountID ={INT}    \r\n"
					" WHERE  ID = {INT}  ",nCode  ,nID );
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::EditingFinishingVisionwebSupplierLocationList");
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebCatalogSetupDlg::SelChosenComboLocation(LPDISPATCH lpRow)
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
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::SelChosenComboLocation");
}
// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebCatalogSetupDlg::OnRequeryFinishedLocationComboList(short nFlags)
{
	try{
		IRowSettingsPtr pRow= m_LocationListCombo->FindByColumn(0,-1L , NULL, VARIANT_FALSE); // Cast ID when it pass to FindByColumn as -1L
		if (pRow !=NULL)
			m_LocationListCombo->CurSel = m_LocationListCombo->FindByColumn(0,-1L , NULL, VARIANT_FALSE); // Cast ID when it pass to FindByColumn as -1L
		m_nxeditLocationCode.EnableWindow(FALSE);
		m_btnAddLocation.EnableWindow(FALSE);
		SetDlgItemText(IDC_LOCATION_VISIONWEB_CODE ,CString(""));
		}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::OnRequeryFinishedLocationComboList");
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvVisionWebCatalogSetupDlg::OnBnClickedCancel()
{
	try{ 
	CNxDialog::OnCancel();	
	}NxCatchAll("Error in CInvVisionWebCatalogSetupDlg::OnBnClickedCancel");
}

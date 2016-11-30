// InvGlassesCatalogSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "InvGlassesCatalogSetupDlg.h"
#include "InventoryRc.h"
#include "MultiSelectDlg.h"
#include "InvVisionWebUtils.h"
#include "InternationalUtils.h"
#include "InvGlassesItemDlg.h"
#include "GlassesCatalogBillingSetupDlg.h"
#include "SingleSelectMultiColumnDlg.h"	// (j.dinatale 2013-03-26 10:08) - PLID 53425

// (s.dhole 2011-03-14 18:10) - PLID 42795 New Catalog setup Dialog
// Most of the code I move from VisionWebCatalogSetup 
// CInvGlassesCatalogSetupDlg dialog


using namespace NXDATALIST2Lib;
using namespace ADODB;

IMPLEMENT_DYNAMIC(CInvGlassesCatalogSetupDlg, CNxDialog)


	// Addedd menuNone as non selected item for popup menu
	enum MenuType
	{
		menuNone=0,
		menuFrameType,
		menuDesign,
		menuMaterial,
		menuTreatment,

	};

enum VisionWebSupplierListColumns {
	vwslID = 0,
	vwslSupplierName,
	vwslCode,
	vwslType,
};
CInvGlassesCatalogSetupDlg::CInvGlassesCatalogSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvGlassesCatalogSetupDlg::IDD, pParent)
{

}

CInvGlassesCatalogSetupDlg::~CInvGlassesCatalogSetupDlg()
{
}

void CInvGlassesCatalogSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_PREV_SUPPLIER, m_btnPrevSupplier);
		DDX_Control(pDX,IDC_NEXT_SUPPLIER , m_btnNextSupplier);
		DDX_Control(pDX,IDC_BTN_DESIGN, m_btnAddDesigns);
		DDX_Control(pDX, IDCANCEL, m_btnClose);
		DDX_Control(pDX, IDC_BTN_MATERIAL, m_btnAddMaterials);
		DDX_Control(pDX, IDC_BTN_TREATMENTS,m_btnAddTreatments );
		DDX_Control(pDX, IDC_BTN_FRAME_TYPE,m_btnAddFrameType );
		DDX_Control(pDX,IDC_BTN_VISIONWEBSYNC ,m_btnVisionWebSync );
		DDX_Control(pDX, IDC_BILLING_SETUP, m_btnBillingSetup);
		DDX_Control(pDX, IDC_BUTTON_COPY_SETUP, m_btnCopySetup);
}



BEGIN_MESSAGE_MAP(CInvGlassesCatalogSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_PREV_SUPPLIER, OnSupplierPrevious)
	ON_BN_CLICKED(IDC_NEXT_SUPPLIER, OnSupplierNext)
	ON_BN_CLICKED( IDC_BTN_FRAME_TYPE, OnBtnAddFrameType)
	ON_BN_CLICKED( IDC_BTN_DESIGN , OnBtnAddDesigns)
	ON_BN_CLICKED( IDC_BTN_MATERIAL, OnBtnAddMaterials)
	ON_BN_CLICKED( IDC_BTN_TREATMENTS, OnBtnAddTreatments)
	ON_BN_CLICKED(IDC_BTN_VISIONWEBSYNC, OnBtnVisionWebSync)
	ON_BN_CLICKED(IDCANCEL, &CInvGlassesCatalogSetupDlg::OnBnClickedCancel)
	ON_WM_CLOSE() 
	
	ON_BN_CLICKED(IDC_BILLING_SETUP, &CInvGlassesCatalogSetupDlg::OnBillingSetup)
	ON_BN_CLICKED(IDC_BUTTON_COPY_SETUP, &CInvGlassesCatalogSetupDlg::OnBnClickedButtonCopySetup)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CInvGlassesCatalogSetupDlg, CNxDialog)
	ON_EVENT(CInvGlassesCatalogSetupDlg, IDC_SELECTED_FRAMETYPE_LIST, 6 /* RButtonDown */, CInvGlassesCatalogSetupDlg::OnRButtonDownFrameTypeList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CInvGlassesCatalogSetupDlg, IDC_SELECTED_DESIGNS_LIST, 6 /* RButtonDown */, CInvGlassesCatalogSetupDlg::OnRButtonDownDesignsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CInvGlassesCatalogSetupDlg, IDC_SELECTED_MATERIALS_LIST, 6 /* RButtonDown */, CInvGlassesCatalogSetupDlg::OnRButtonDownMaterialsList , VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CInvGlassesCatalogSetupDlg, IDC_SELECTED_TREATMENTS_LIST, 6 /* RButtonDown */, CInvGlassesCatalogSetupDlg::OnRButtonDownTreatmentsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CInvGlassesCatalogSetupDlg, IDC_SELECTED_SUPPLIER_LIST, 1, CInvGlassesCatalogSetupDlg::SelChangingSelectedSupplierList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvGlassesCatalogSetupDlg, IDC_SELECTED_SUPPLIER_LIST, 16, CInvGlassesCatalogSetupDlg::SelChosenSelectedSupplierList, VTS_DISPATCH)
END_EVENTSINK_MAP()


BOOL CInvGlassesCatalogSetupDlg::OnInitDialog() 
{	
	CNxDialog::OnInitDialog();
	try {
		m_btnPrevSupplier.AutoSet(NXB_LEFT);
		m_btnNextSupplier.AutoSet(NXB_RIGHT);
		m_btnAddFrameType.AutoSet(NXB_NEW);
		m_btnAddDesigns.AutoSet(NXB_NEW);
		m_btnAddMaterials.AutoSet(NXB_NEW);
		m_btnAddTreatments.AutoSet(NXB_NEW);
		m_btnVisionWebSync.AutoSet(NXB_REFRESH);
		//TES 5/19/2011 - PLID 43698 - Added
		m_btnBillingSetup.AutoSet(NXB_MODIFY);
		m_btnCopySetup.AutoSet(NXB_MODIFY);	// (j.dinatale 2013-03-26 10:08) - PLID 53425

		m_btnClose.AutoSet(NXB_CLOSE);

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
		GetDlgItem(IDC_STATIC_CATALOG_SYNC)->SetFont(&theApp.m_boldFont  );
		m_SelectedSupplierCombo = BindNxDataList2Ctrl(IDC_SELECTED_SUPPLIER_LIST,GetRemoteData(),FALSE );
		m_SelectedDesignsList = BindNxDataList2Ctrl(IDC_SELECTED_DESIGNS_LIST ,GetRemoteData(),FALSE);
		m_SelectedFrameTypeList = BindNxDataList2Ctrl(IDC_SELECTED_FRAMETYPE_LIST ,GetRemoteData(),FALSE);
		m_SelectedMaterialsList = BindNxDataList2Ctrl(IDC_SELECTED_MATERIALS_LIST,GetRemoteData(),FALSE);
		m_SelectedTreatmentsList = BindNxDataList2Ctrl(IDC_SELECTED_TREATMENTS_LIST,GetRemoteData(),FALSE);
		m_SelectedSupplierCombo->FromClause = _bstr_t("(SELECT SupplierT.PersonID, PersonT.Company,SupplierT.VisionWebID , Case When SupplierT.VisionWebID IS NULL OR SupplierT.VisionWebID ='' THEN '' ELSE 'VisionWeb' End Type "
			" FROM SupplierT INNER JOIN PersonT ON SupplierT.PersonID = PersonT.ID "
			" WHERE  PersonT.Archived=0) AS SupplierQ ");
		m_SelectedSupplierCombo ->Requery(); 
		m_SelectedSupplierCombo ->WaitForRequery(dlPatienceLevelWaitIndefinitely) ; 
		IRowSettingsPtr pRow =m_SelectedSupplierCombo->GetFirstRow();
		// (s.dhole 2011-03-30 12:01) - PLID 43039
		if (m_nSupplierID>0) 
		{ 
			pRow =m_SelectedSupplierCombo->FindByColumn(vwslID, m_nSupplierID, NULL, g_cvarTrue);
		}
		// (s.dhole 2011-03-30 12:01) PLID 43039
		if (pRow!=NULL)
			m_SelectedSupplierCombo->CurSel =pRow;
		UpdateSupplierArrows();
		m_SelectedFrameTypeList->FromClause = _bstr_t("GlassesCatalogFrameTypesT INNER JOIN GlassesOrderProcessTypeT ON GlassesCatalogFrameTypesT.GlassesOrderProcessTypeID =GlassesOrderProcessTypeT.ID ");
		m_SelectedDesignsList->FromClause = _bstr_t("GlassesCatalogDesignsT INNER JOIN GlassesOrderProcessTypeT ON GlassesCatalogDesignsT.GlassesOrderProcessTypeID =GlassesOrderProcessTypeT.ID");
		m_SelectedMaterialsList->FromClause = _bstr_t("GlassesCatalogMaterialsT INNER JOIN GlassesOrderProcessTypeT ON GlassesCatalogMaterialsT.GlassesOrderProcessTypeID =GlassesOrderProcessTypeT.ID");
		m_SelectedTreatmentsList->FromClause = _bstr_t("GlassesCatalogTreatmentsT INNER JOIN GlassesOrderProcessTypeT ON GlassesCatalogTreatmentsT.GlassesOrderProcessTypeID =GlassesOrderProcessTypeT.ID");
		SelChosenSelectedSupplierList(pRow);

	// (j.jones 2016-03-02 15:10) - PLID 47279 - replaced an generic error with __FUNCTION__
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CInvGlassesCatalogSetupDlg::OnSupplierPrevious() 
{
	try {
		IRowSettingsPtr pRow1 = m_SelectedSupplierCombo->GetCurSel();
		if (pRow1!=NULL){
			IRowSettingsPtr pRow2 = pRow1->GetPreviousRow();
			m_SelectedSupplierCombo->CurSel = pRow2;
			SelChosenSelectedSupplierList(pRow2);
		}
		else{
			UpdateSupplierArrows(); //  update naviagation button
		}
	}NxCatchAll(__FUNCTION__);
}

void CInvGlassesCatalogSetupDlg::OnSupplierNext() 
{
	try {
		IRowSettingsPtr pRow1 = m_SelectedSupplierCombo->GetCurSel();
		if (pRow1!=NULL) {
			IRowSettingsPtr pRow2 = pRow1->GetNextRow();
			m_SelectedSupplierCombo->CurSel = pRow2;
			SelChosenSelectedSupplierList(pRow2);
		}
		else{
			UpdateSupplierArrows();//  update naviagation button
		}
	}NxCatchAll(__FUNCTION__);
}



void CInvGlassesCatalogSetupDlg::UpdateSupplierArrows()
{
	try{
		BOOL bNextEnabled = TRUE, bPreviousEnabled = TRUE;
		IRowSettingsPtr pCurRow= m_SelectedSupplierCombo->GetCurSel();
		if (pCurRow==NULL)
		{
			m_btnPrevSupplier.EnableWindow(FALSE);
			m_btnNextSupplier.EnableWindow(FALSE);
		}
		else
		{
			if (pCurRow->GetPreviousRow() == NULL) {
				bPreviousEnabled = FALSE;
			}
			if (pCurRow->GetNextRow() == NULL) {
				bNextEnabled=FALSE;
			}
			m_btnPrevSupplier.EnableWindow(bPreviousEnabled);
			m_btnNextSupplier.EnableWindow(bNextEnabled);
		}
	// (j.jones 2016-03-02 15:10) - PLID 47279 - replaced an incorrect dialog name with __FUNCTION__
	}NxCatchAll(__FUNCTION__);
}

void CInvGlassesCatalogSetupDlg::GenerateMenu(int MenuTypeID, CPoint &pt)
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

		}
		mnu.DestroyMenu();
	}NxCatchAll(__FUNCTION__);
}



// (s.dhole 2010-11-15 13:31) - PLID  41743 Assign frame type to supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvGlassesCatalogSetupDlg::OnBtnAddFrameType()
{
	try{
		CInvGlassesItemDlg dlg(this);
		CString strFrom = "GlassesCatalogFrameTypesT";
		// (s.dhole 2011-04-11 12:58) - PLID 42835 Converting IsChecked to bit
		CString strSql= FormatString("( SELECT CAST(0 As BIT) AS IsChecked, GlassesCatalogFrameTypesT.ID, GlassesCatalogFrameTypesT.FrameTypeName AS Name, GlassesCatalogFrameTypesT.FrameTypeCode As Code, \r\n"
		" GlassesCatalogFrameTypesT.GlassesOrderProcessTypeID, GlassesOrderProcessTypeT.ProcessName  \r\n"
		" FROM GlassesCatalogFrameTypesT INNER JOIN  \r\n"
		" GlassesOrderProcessTypeT ON GlassesCatalogFrameTypesT.GlassesOrderProcessTypeID = GlassesOrderProcessTypeT.ID \r\n"
		" WHERE GlassesCatalogFrameTypesT.ID NOT IN \r\n"
		"(SELECT     GlassesCatalogFrameTypesID FROM  GlassesSupplierCatalogFrameTypesT \r\n"
		" WHERE  IsActive=1 AND   SupplierID =  %d )) AS ItemQ_", m_nSupplierID );
		
		//make sure user select atleast one item before click on ok button
		// (s.dhole 2011-04-11 12:58) - PLID 42835 change display name
		if(IDOK == dlg.Open(  "GlassesCatalogFrameTypesT", "FrameTypeName", "FrameTypeCode" ,strSql , "Frame Type")) {
			CString strFilter = dlg.GetMultiSelectIDString();
			if (!strFilter.IsEmpty()){
				strFilter.Replace(" ", ", ");
				CString strSqlBatch = BeginSqlBatch();
				AddStatementToSqlBatch(strSqlBatch, "UPDATE GlassesSupplierCatalogFrameTypesT SET IsActive=1 \r\n"
					" WHERE SupplierID =%d  and GlassesCatalogFrameTypesID IN (%s)  \r\n"
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
	}NxCatchAll(__FUNCTION__);
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 Assign frame type to supplier
void CInvGlassesCatalogSetupDlg::RemoveFrameType()
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
		
		
	}NxCatchAll(__FUNCTION__);
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 Assign Design to supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvGlassesCatalogSetupDlg::OnBtnAddDesigns()
{
	try{
		CInvGlassesItemDlg dlg(this);
		CString strFrom = "GlassesCatalogDesignsT";
		// (s.dhole 2011-04-11 12:58) - PLID 42835 Converting IsChecked to bit
		CString strSql= FormatString("( SELECT CAST(0 As BIT) AS IsChecked, GlassesCatalogDesignsT.ID, GlassesCatalogDesignsT.DesignName AS Name, GlassesCatalogDesignsT.DesignCode As Code, \r\n"
		" GlassesCatalogDesignsT.GlassesOrderProcessTypeID, GlassesOrderProcessTypeT.ProcessName  \r\n"
		" FROM GlassesCatalogDesignsT INNER JOIN  \r\n"
		" GlassesOrderProcessTypeT ON GlassesCatalogDesignsT.GlassesOrderProcessTypeID = GlassesOrderProcessTypeT.ID \r\n"
		" WHERE GlassesCatalogDesignsT.ID NOT IN \r\n"
		"(SELECT     GlassesCatalogDesignsID FROM  GlassesSupplierCatalogDesignsT \r\n"
		" WHERE  IsActive=1 AND   SupplierID =  %d )) AS ItemQ_", m_nSupplierID );
		
		//make sure user select atleast one item before click on ok button
		if(IDOK == dlg.Open(  "GlassesCatalogDesignsT", "DesignName", "DesignCode" ,strSql , "Design")) {
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
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 Remove Design From supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvGlassesCatalogSetupDlg::RemoveDesigns()
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
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 Assign Material to supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvGlassesCatalogSetupDlg::OnBtnAddMaterials()
{
	try{
		CInvGlassesItemDlg dlg(this);
		CString strFrom = "GlassesCatalogMaterialsT";
		// (s.dhole 2011-04-11 12:58) - PLID 42835 Converting IsChecked to bit
		CString strSql= FormatString("( SELECT CAST(0 As BIT) AS IsChecked, GlassesCatalogMaterialsT.ID, GlassesCatalogMaterialsT.MaterialName AS Name, GlassesCatalogMaterialsT.MaterialCode As Code, \r\n"
		" GlassesCatalogMaterialsT.GlassesOrderProcessTypeID, GlassesOrderProcessTypeT.ProcessName  \r\n"
		" FROM GlassesCatalogMaterialsT INNER JOIN  \r\n"
		" GlassesOrderProcessTypeT ON GlassesCatalogMaterialsT.GlassesOrderProcessTypeID = GlassesOrderProcessTypeT.ID \r\n"
		" WHERE GlassesCatalogMaterialsT.ID NOT IN \r\n"
		"(SELECT     GlassesCatalogMaterialID FROM  GlassesSupplierCatalogMaterialsT \r\n"
		" WHERE  IsActive=1 AND   SupplierID =  %d )) AS ItemQ_", m_nSupplierID );
		
		//make sure user select atleast one item before click on ok button
		if(IDOK == dlg.Open(  "GlassesCatalogMaterialsT", "MaterialName", "MaterialCode" ,strSql , "Material")) {
			CString strFilter = dlg.GetMultiSelectIDString();
			if (!strFilter.IsEmpty()){
				strFilter.Replace(" ", ", ");
				CString strSqlBatch = BeginSqlBatch();
				AddStatementToSqlBatch(strSqlBatch, "UPDATE GlassesSupplierCatalogMaterialsT SET IsActive=1 \r\n"
					" WHERE SupplierID =%d  and GlassesCatalogMaterialID IN (%s)  \r\n"
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
	}NxCatchAll(__FUNCTION__);
	
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 Remove Materials From supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvGlassesCatalogSetupDlg::RemoveMaterials()
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
	}NxCatchAll(__FUNCTION__);
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 Assign treatment to supplier
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CInvGlassesCatalogSetupDlg::OnBtnAddTreatments()
{
	try{
		CInvGlassesItemDlg dlg(this);
		CString strFrom = "GlassesCatalogTreatmentsT";
		// (s.dhole 2011-04-11 12:58) - PLID 42835 Converting IsChecked to bit
		CString strSql= FormatString("( SELECT CAST(0 As BIT) AS IsChecked, GlassesCatalogTreatmentsT.ID, GlassesCatalogTreatmentsT.TreatmentName AS Name, GlassesCatalogTreatmentsT.TreatmentCode As Code, \r\n"
		" GlassesCatalogTreatmentsT.GlassesOrderProcessTypeID, GlassesOrderProcessTypeT.ProcessName  \r\n"
		" FROM GlassesCatalogTreatmentsT INNER JOIN  \r\n"
		" GlassesOrderProcessTypeT ON GlassesCatalogTreatmentsT.GlassesOrderProcessTypeID = GlassesOrderProcessTypeT.ID \r\n"
		" WHERE GlassesCatalogTreatmentsT.ID NOT IN \r\n"
		"(SELECT     GlassesCatalogTreatmentsID FROM  GlassesSupplierCatalogTreatmentsT \r\n"
		" WHERE  IsActive=1 AND   SupplierID =  %d )) AS ItemQ_", m_nSupplierID );
		
		//make sure user select atleast one item before click on ok button
		if(IDOK == dlg.Open(  "GlassesCatalogTreatmentsT", "TreatmentName", "TreatmentCode" ,strSql , "Treatment")) {
			CString strFilter = dlg.GetMultiSelectIDString();
			if (!strFilter.IsEmpty()){
				strFilter.Replace(" ", ", ");
				CString strSqlBatch = BeginSqlBatch();
				AddStatementToSqlBatch(strSqlBatch, "UPDATE GlassesSupplierCatalogTreatmentsT SET IsActive=1 \r\n"
					" WHERE SupplierID =%d  and GlassesCatalogTreatmentsID IN (%s)  \r\n"
				,m_nSupplierID,strFilter);
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
	}NxCatchAll(__FUNCTION__);
	
}

// (s.dhole 2010-11-15 13:31) - PLID  41743 Remove Treatments  from  supplier
void CInvGlassesCatalogSetupDlg::RemoveTreatments()
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
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2010-11-15 13:31) - PLID 40728 Sync  catalog information with visionWeb
void CInvGlassesCatalogSetupDlg::OnBtnVisionWebSync()
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
void CInvGlassesCatalogSetupDlg::OnRButtonDownFrameTypeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
			return;
		// this was missing code, need to select row when user click 
		m_SelectedFrameTypeList->PutCurSel(pRow); 
		CPoint pt;
		GetCursorPos(&pt);
		GenerateMenu(menuFrameType,pt);
	}NxCatchAll(__FUNCTION__);
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvGlassesCatalogSetupDlg::OnRButtonDownDesignsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
			return;
		m_SelectedDesignsList->PutCurSel(pRow);
		CPoint pt;
		GetCursorPos(&pt);
		GenerateMenu(menuDesign,pt);
	}NxCatchAll(__FUNCTION__);
}


// (s.dhole 2010-11-15 13:31) - PLID  41743 
void CInvGlassesCatalogSetupDlg::OnRButtonDownTreatmentsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
			return;
		m_SelectedTreatmentsList->PutCurSel(pRow);
		CPoint pt;
		GetCursorPos(&pt);
		GenerateMenu(menuTreatment,pt );
	}NxCatchAll(__FUNCTION__);
}


void CInvGlassesCatalogSetupDlg::OnRButtonDownMaterialsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
			return;
		m_SelectedMaterialsList->PutCurSel(pRow);
		CPoint pt;
		GetCursorPos(&pt);
		GenerateMenu(menuMaterial ,pt);
	}NxCatchAll(__FUNCTION__);
}
void CInvGlassesCatalogSetupDlg::OnBnClickedCancel()
{
	try{ 
		CNxDialog::OnCancel();	
	}NxCatchAll(__FUNCTION__);
}
void CInvGlassesCatalogSetupDlg::SelChangingSelectedSupplierList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		// (s.dhole 2011-02-18 16:08) - PLID 40538 stop selecting null patient
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
		
	}NxCatchAll(__FUNCTION__);
}

void CInvGlassesCatalogSetupDlg::SelChosenSelectedSupplierList(LPDISPATCH lpRow)
{
try {
	IRowSettingsPtr pRow(lpRow);
	if (pRow!=NULL)
		m_nSupplierID=VarLong(pRow->GetValue(vwslID ),-1) ;	
	else 
		m_nSupplierID=-1;
	ReLoadAllDataList();
	UpdateSupplierArrows();

	//(c.copits 2011-10-04) PLID 45112 - Clicking Glasses Catalog with an inactive supplier generates errors
	UpdateSupplierButtons();

	}NxCatchAll(__FUNCTION__);
}

void CInvGlassesCatalogSetupDlg::ReLoadAllDataList()
{
try {
		CString  strWhere;
		strWhere.Format( " GlassesCatalogFrameTypesT.ID  in ( Select GlassesCatalogFrameTypesID From GlassesSupplierCatalogFrameTypesT "
				" Where  IsActive=1 AND  SupplierID =%d ) ", m_nSupplierID );
		m_SelectedFrameTypeList->PutWhereClause(_bstr_t(strWhere));
	    m_SelectedFrameTypeList->Requery(); 
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		
		strWhere.Format( " GlassesCatalogDesignsT.ID in ( Select GlassesCatalogDesignsID From GlassesSupplierCatalogDesignsT "
				" Where  IsActive=1 AND SupplierID =%d ) ", m_nSupplierID );
		m_SelectedDesignsList->PutWhereClause(_bstr_t(strWhere));
	    m_SelectedDesignsList->Requery(); 
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		
		strWhere.Format( " GlassesCatalogMaterialsT.ID in ( Select GlassesCatalogMaterialID From GlassesSupplierCatalogMaterialsT "
				" Where IsActive=1 AND  SupplierID =%d ) ", m_nSupplierID );
		m_SelectedMaterialsList->PutWhereClause(_bstr_t(strWhere));
	    m_SelectedMaterialsList->Requery(); 
		
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		
		strWhere.Format( " GlassesCatalogTreatmentsT .ID in ( Select GlassesCatalogTreatmentsID From GlassesSupplierCatalogTreatmentsT "
				" Where  IsActive=1 AND SupplierID =%d ) ", m_nSupplierID );
		m_SelectedTreatmentsList->PutWhereClause(_bstr_t(strWhere));
	    m_SelectedTreatmentsList->Requery(); 
	}NxCatchAll(__FUNCTION__);
}

void CInvGlassesCatalogSetupDlg::OnBillingSetup()
{
	try {
		//TES 5/20/2011 - PLID 43698 - Just open up the dialog
		CGlassesCatalogBillingSetupDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-10-04) PLID 45112 - Clicking Glasses Catalog with an inactive supplier generates errors
void CInvGlassesCatalogSetupDlg::UpdateSupplierButtons()
{
	// Disable buttons if invalid supplier selection
	IRowSettingsPtr pRow = m_SelectedSupplierCombo->GetCurSel();
	{
		if (pRow) {
		
			long nSupplierID = -1;
			nSupplierID = VarLong(pRow->GetValue(vwslID),-1);
			if (nSupplierID < 0) {
				GetDlgItem(IDC_BTN_DESIGN)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_MATERIAL)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_TREATMENTS)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_FRAME_TYPE)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_BTN_DESIGN)->EnableWindow(TRUE);
				GetDlgItem(IDC_BTN_MATERIAL)->EnableWindow(TRUE);
				GetDlgItem(IDC_BTN_TREATMENTS)->EnableWindow(TRUE);
				GetDlgItem(IDC_BTN_FRAME_TYPE)->EnableWindow(TRUE);
			}
		}
		else {
				GetDlgItem(IDC_BTN_DESIGN)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_MATERIAL)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_TREATMENTS)->EnableWindow(FALSE);
				GetDlgItem(IDC_BTN_FRAME_TYPE)->EnableWindow(FALSE);
		}
	}
}

// (j.dinatale 2013-03-26 10:08) - PLID 53425 - Copy lab setup of design, materials, treatment from 1 lab company to another
void CInvGlassesCatalogSetupDlg::OnBnClickedButtonCopySetup()
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_SelectedSupplierCombo->CurSel;
		if(!pRow){
			return;
		}

		long nSupplierID = VarLong(pRow->GetValue(vwslID), -1);
		if(nSupplierID < 0){
			AfxMessageBox("The currently selected supplier is not valid. Please verify you have a supplier selected and try again.");
			return;
		}

		//1. Columns to Select (order matters)
		CStringArray aryColumns;
		aryColumns.Add("SupplierT.PersonID");
		aryColumns.Add("PersonT.Company");
		//2. Column Headers (order matters)
		CStringArray aryColumnHeaders;
		aryColumnHeaders.Add("ID");
		aryColumnHeaders.Add("Patient ID");
		//3. Sort order for columns (order matters)
		CSimpleArray<short> arySortOrder;
		arySortOrder.Add(-1);
		arySortOrder.Add(0);

		CString strWhere;
		strWhere.Format(" PersonT.ID > 0 AND PersonT.Archived = 0 AND SupplierT.PersonID <> %li ", nSupplierID);

		// Open the dialog
		CSingleSelectMultiColumnDlg dlg(this);
		HRESULT hr = dlg.Open(" SupplierT INNER JOIN PersonT ON SupplierT.PersonID = PersonT.ID ",		/*From*/
			strWhere,																					/*Where*/
			aryColumns,																					/*Select*/
			aryColumnHeaders,																			/*Column Names*/
			arySortOrder,																				/*Sort Order*/
			"[1]",																						/*Display Columns*/
			"Please select a supplier to copy from.",													/*Description*/
			"Select a Supplier"																			/*Title Bar Header*/
			);

		// If they clicked OK, be sure to check if they made a valid selection
		if(hr == IDOK){
			CVariantArray varySelectedValues;
			dlg.GetSelectedValues(varySelectedValues);

			long nSupplierIDToCopy;
			if(!(varySelectedValues.GetSize() > 0 && (nSupplierIDToCopy = VarLong(varySelectedValues.GetAt(0), -1)) > 0 )){
				// If not, tell the user
				AfxMessageBox("You did not select a valid supplier. Please verify your selection and try again.");
				return;
			}else{
				// make sure the user is ok with the overwriting part
				if(IDNO == MessageBox("You are about to overwrite the current supplier's setup. Are you sure you want to continue?", "Practice", MB_ICONQUESTION | MB_YESNO)){
					return;
				}
			}

			CSqlFragment sqlCopy;
			sqlCopy.Create(
				"DECLARE @SupplierIDToCopy INT; "
				"DECLARE @SupplierID INT; "

				"SET @SupplierIDToCopy = {INT}; "
				"SET @SupplierID = {INT}; "

				"DELETE FROM GlassesSupplierCatalogFrameTypesT WHERE SupplierID = @SupplierID; "
				"DELETE FROM GlassesSupplierCatalogDesignsT WHERE SupplierID = @SupplierID; "
				"DELETE FROM GlassesSupplierCatalogMaterialsT WHERE SupplierID = @SupplierID; "
				"DELETE FROM GlassesSupplierCatalogTreatmentsT WHERE SupplierID = @SupplierID; "

				"INSERT INTO GlassesSupplierCatalogFrameTypesT(SupplierID, GlassesCatalogFrameTypesID, IsActive) "
				"	SELECT @SupplierID, GlassesCatalogFrameTypesID, IsActive FROM GlassesSupplierCatalogFrameTypesT WHERE SupplierID = @SupplierIDToCopy; "

				"INSERT INTO GlassesSupplierCatalogDesignsT(SupplierID, GlassesCatalogDesignsID, IsActive) "
				"	SELECT @SupplierID, GlassesCatalogDesignsID, IsActive FROM GlassesSupplierCatalogDesignsT WHERE SupplierID = @SupplierIDToCopy; "

				"INSERT INTO GlassesSupplierCatalogMaterialsT(SupplierID, GlassesCatalogMaterialID, IsActive) "
				"	SELECT @SupplierID, GlassesCatalogMaterialID, IsActive FROM GlassesSupplierCatalogMaterialsT WHERE SupplierID = @SupplierIDToCopy; "

				"INSERT INTO GlassesSupplierCatalogTreatmentsT(SupplierID, GlassesCatalogTreatmentsID, IsActive) "
				"	SELECT @SupplierID, GlassesCatalogTreatmentsID, IsActive FROM GlassesSupplierCatalogTreatmentsT WHERE SupplierID = @SupplierIDToCopy; ",
				nSupplierIDToCopy, nSupplierID
			);

			ExecuteParamSql(sqlCopy);
			ReLoadAllDataList();
		}
	}NxCatchAll(__FUNCTION__);
}

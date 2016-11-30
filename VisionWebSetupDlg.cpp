// VisionWebSetupDlg.cpp : implementation file
//
// (s.dhole 2010-11-02 15:42) - PLID 41281  VisionWeb service Url and refid setup
#include "stdafx.h"
#include "Practice.h"
#include "VisionWebSetupDlg.h"
#include "InventoryRc.h"
#include "MultiSelectDlg.h"
#include "InvVisionWebLocationDlg.h"
#include "VisionWebServiceSetupDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;
#ifdef _DEBUG
#endif
// CVisionWebSetupDlg dialog
// (a.vengrofski 2010-09-20 11:45) - PLID 40541 - Created

IMPLEMENT_DYNAMIC(CVisionWebSetupDlg, CNxDialog)

//  Enum for supplier list
enum VisionWebSupplierListColumns {
	vwSupplierID =0, 
	vwSupplierName ,
	vwSupplierCode , 
};

CVisionWebSetupDlg::CVisionWebSetupDlg(CWnd* pParent /*=NULL*/)
: CNxDialog(CVisionWebSetupDlg::IDD, pParent)
{}

CVisionWebSetupDlg::~CVisionWebSetupDlg()
{}

void CVisionWebSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_VISIONWEB_SETUP_CLOSE, m_CloseBtn);
	DDX_Control(pDX, IDC_BTN_ADD_SUPPLIER, m_AddSupplierBtn);
	DDX_Control(pDX, IDC_BTN_SUPPLIER_CATALOG, m_SupplierCatalogBtn);
	DDX_Control(pDX, IDC_BTN_VISIONWEB_SETUP,m_SetupVisionWebURLBtn  );
	DDX_Control(pDX, IDC_VISIONWEB_USERNAME , m_nxeUserID);
	DDX_Control(pDX, IDC_VISIONWEB_PASSWORD, m_nxePassword);
	DDX_Control(pDX, IDC_SUPPLIER_VISIONWEB_CODE, m_nxeSupplierCode);
}

BEGIN_MESSAGE_MAP(CVisionWebSetupDlg, CNxDialog)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SUPPLIERPOPUP_REMOVE, OnRemoveSupplier)
	ON_COMMAND(ID_SUPPLIERPOPUP_CATALOG, OnCatalogSetup)
	ON_EN_KILLFOCUS(IDC_VISIONWEB_USERNAME, &CVisionWebSetupDlg::OnEnKillfocusVisionwebUsername)
	ON_EN_KILLFOCUS(IDC_VISIONWEB_PASSWORD, &CVisionWebSetupDlg::OnEnKillfocusVisionwebPassword)
	ON_EN_KILLFOCUS(IDC_SUPPLIER_VISIONWEB_CODE, &CVisionWebSetupDlg::OnEnKillfocusSupplierCode)
	ON_BN_CLICKED(IDC_BTN_ADD_SUPPLIER, &CVisionWebSetupDlg::OnBtnClickedAddSupplier)
	ON_BN_CLICKED(IDC_BTN_VISIONWEB_SETUP, &CVisionWebSetupDlg::OnBtnClickedVisionWebUrlSetup)
	ON_BN_CLICKED(IDC_BTN_SUPPLIER_CATALOG, &CVisionWebSetupDlg::OnBnClickedBtnSupplierCatalog)
	ON_BN_CLICKED(IDC_BTN_VISIONWEB_SETUP_CLOSE, &CVisionWebSetupDlg::OnBtnClose)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CVisionWebSetupDlg, CNxDialog)
	ON_EVENT(CVisionWebSetupDlg, IDC_COMBO_SUPPLIER, 18 /* RequeryFinished */, OnRequeryFinishedSupplierCombo, VTS_I2)
	ON_EVENT(CVisionWebSetupDlg, IDC_COMBO_SUPPLIER, 16 /* TrySetSelFinished */, OnSelChosenSupplierCombo, VTS_DISPATCH)
	ON_EVENT(CVisionWebSetupDlg, IDC_VISIONWEB_SUPPLIER_LIST, 9, CVisionWebSetupDlg::EditingFinishingVisionwebSupplierList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CVisionWebSetupDlg, IDC_VISIONWEB_SUPPLIER_LIST, 1, CVisionWebSetupDlg::SelChangingVisionwebSupplierList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CVisionWebSetupDlg, IDC_VISIONWEB_SUPPLIER_LIST, 2, CVisionWebSetupDlg::SelChangedVisionwebSupplierList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CVisionWebSetupDlg, IDC_VISIONWEB_SUPPLIER_LIST, 6 /* RButtonDown */, CVisionWebSetupDlg::OnRButtonDownVisionwebSupplierList,  VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()
// CVisionWebSetupDlg message handlers

BOOL CVisionWebSetupDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();
		m_CloseBtn.AutoSet(NXB_CLOSE);
		m_AddSupplierBtn.AutoSet(NXB_NEW);
		m_SupplierCatalogBtn.AutoSet(NXB_MODIFY);
		m_SetupVisionWebURLBtn.AutoSet(NXB_MODIFY);
		m_nxeUserID.SetLimitText(20);
		m_nxePassword.SetLimitText(36);
		m_nxeSupplierCode.SetLimitText(4);
		m_VisionWebSupplierList = BindNxDataList2Ctrl(IDC_VISIONWEB_SUPPLIER_LIST ,GetRemoteData(), TRUE);
		m_VisionWebSupplierCombo = BindNxDataList2Ctrl(IDC_COMBO_SUPPLIER,GetRemoteData(), FALSE);
		g_propManager.BulkCache( "VisionWebSetup",propbitText,
			FormatString("(Username = '<None>') AND (Name = '%s' OR Name = '%s')",  
			VISIONWEBSERVICE_USER_ID,VISIONWEBSERVICE_USER_PASSWORD)  );
		CString strVisionWebUserID = GetRemotePropertyText(VISIONWEBSERVICE_USER_ID, "", 0, "<None>", true);
		_variant_t vPass = GetRemotePropertyImage(VISIONWEBSERVICE_USER_PASSWORD, 0, "<None>", false);
		CString strVisionWebPassword ;
		if (vPass.vt != VT_EMPTY && vPass.vt != VT_NULL) {
			strVisionWebPassword= DecryptStringFromVariant(vPass);
		} else {
			strVisionWebPassword= "";
		}	
		SetDlgItemText(IDC_VISIONWEB_USERNAME, strVisionWebUserID);
		SetDlgItemText(IDC_VISIONWEB_PASSWORD, strVisionWebPassword);
		
		m_VisionWebSupplierCombo->FromClause = _bstr_t("(SELECT -1 AS PersonID,'' AS Company "
			" UNION "
			" SELECT SupplierT.PersonID, PersonT.Company "
			" FROM SupplierT INNER JOIN PersonT ON SupplierT.PersonID = PersonT.ID "
			" WHERE (SupplierT.VisionWebID IS NULL OR SupplierT.VisionWebID ='') AND PersonT.Archived=0) AS SupplierQ ");
		m_VisionWebSupplierCombo ->Requery(); 
		GetDlgItem(IDC_BTN_ADD_SUPPLIER)->EnableWindow(FALSE);
		
#ifdef _DEBUG
		
		// (s.dhole 2010-12-07 12:44) - PLID 41281 Allow user to see Url Button
		// debug should show this button

		GetDlgItem(IDC_BTN_VISIONWEB_SETUP)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_BTN_VISIONWEB_SETUP)->EnableWindow(TRUE);
#else
		// (s.dhole 2010-12-07 12:44) - PLID 41281 Allow user to see Url Button
		// this allow support login to setup url
		if(GetCurrentUserID() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) 
		{
		
			GetDlgItem(IDC_BTN_VISIONWEB_SETUP)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_BTN_VISIONWEB_SETUP)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_BTN_VISIONWEB_SETUP)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_VISIONWEB_SETUP)->ShowWindow(SW_HIDE);		
		}	
	
#endif
	}NxCatchAll("Error in CVisionWebSetupDlg::OnInitDialog");
	return FALSE;
}

 
// added this event so  user can  select and open poup menu on right click 
void CVisionWebSetupDlg::OnRButtonDownVisionwebSupplierList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow==NULL)
		return;
	try {
		m_VisionWebSupplierList->PutCurSel(pRow);
			// Yes, so show the context menu
			CMenu mnu;
			mnu.LoadMenu(IDR_MENU_VISIONWEB_SUPPLIER);
			CMenu *pmnuSub = mnu.GetSubMenu(0);
			if (pmnuSub) {
				CPoint pt;	
				GetCursorPos(&pt);
				pmnuSub->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, this,NULL);
			}
			pmnuSub->DestroyMenu() ;
	} NxCatchAll("CVisionWebSetupDlg::OnRButtonDownVisionwebSupplierList");

}
//Remove string.Left
void CVisionWebSetupDlg::OnEnKillfocusVisionwebUsername()
{
	try{
		CString strTempUsername;
		GetDlgItemText(IDC_VISIONWEB_USERNAME, strTempUsername);
		strTempUsername.TrimRight();
		strTempUsername.TrimLeft();
		strTempUsername.Replace(" ","") ;
		SetDlgItemText(IDC_VISIONWEB_USERNAME, strTempUsername);
	}NxCatchAll("Error in CVisionWebSetupDlg::OnEnKillfocusVisionwebUsername");
}

//Remove string.Left
void CVisionWebSetupDlg::OnEnKillfocusSupplierCode()
{
	try{
		CString strSupplierCode;
		GetDlgItemText(IDC_SUPPLIER_VISIONWEB_CODE, strSupplierCode);
		strSupplierCode.TrimRight();
		strSupplierCode.TrimLeft();
		strSupplierCode.Replace(" ","") ;
		SetDlgItemText(IDC_SUPPLIER_VISIONWEB_CODE, strSupplierCode);
	}NxCatchAll("Error in CVisionWebSetupDlg::OnEnKillfocusSupplierCode");
}

//Remove string.Left
void CVisionWebSetupDlg::OnEnKillfocusVisionwebPassword()
{
	try{
		CString strTempPassword;
		GetDlgItemText(IDC_VISIONWEB_PASSWORD, strTempPassword);
		strTempPassword.TrimRight();
		strTempPassword.TrimLeft();
		strTempPassword.Replace(" ","") ;
		SetDlgItemText(IDC_VISIONWEB_PASSWORD, strTempPassword);
	}NxCatchAll("Error in CVisionWebSetupDlg::OnEnKillfocusVisionwebPassword");
}

void CVisionWebSetupDlg::OnRequeryFinishedSupplierCombo(short nFlags)
{
	try{
		IRowSettingsPtr pRow= m_VisionWebSupplierCombo->FindByColumn(0,-1L , NULL, VARIANT_FALSE);
		if (pRow !=NULL)
			m_VisionWebSupplierCombo->CurSel = m_VisionWebSupplierCombo->FindByColumn(0,-1L , NULL, VARIANT_FALSE);
		m_nxeSupplierCode.EnableWindow(FALSE);
		m_AddSupplierBtn.EnableWindow(FALSE);
		m_SupplierCatalogBtn.EnableWindow(FALSE);
		SetDlgItemText(IDC_SUPPLIER_VISIONWEB_CODE,CString(""));
		}NxCatchAll("Error in CVisionWebSetupDlg::OnRequeryFinishedSupplierCombo");
}

void CVisionWebSetupDlg::OnSelChosenSupplierCombo(LPDISPATCH lpRow)
{
	try{
		IRowSettingsPtr pRow= m_VisionWebSupplierCombo->CurSel;
		if (pRow !=NULL){
			 if (VarLong(pRow->GetValue(0)) >-1){
				m_nxeSupplierCode.EnableWindow(TRUE);
				m_AddSupplierBtn.EnableWindow(TRUE);
			 }
			 else{
				m_nxeSupplierCode.EnableWindow(FALSE);
				m_AddSupplierBtn.EnableWindow(FALSE);
			 }
		}
		else{
			m_nxeSupplierCode.EnableWindow(FALSE);
			m_AddSupplierBtn.EnableWindow(FALSE);
		}
	}NxCatchAll("Error in CVisionWebSetupDlg::OnSelChosenSupplierCombo");
		
}
// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
void CVisionWebSetupDlg::OnRemoveSupplier()
{
	try{
		IRowSettingsPtr pRow= m_VisionWebSupplierList->GetCurSel();
		long nSupplierID =0;
		if(pRow != NULL) 
			nSupplierID  = VarLong(pRow->GetValue(vwSupplierID)); // replace 0 with enum value
		else
			return;
		// check how many order do we have for this supplier
		_RecordsetPtr rs = CreateParamRecordset("SELECT Count(SupplierID) as recCount   FROM GlassesOrderT  "
			" WHERE SupplierID = {INT} \r\n"		, nSupplierID);
		if(!rs->eof){ 
			// Prompt user 
			 if (VarLong( rs->Fields->Item["recCount"]->Value)>0 ){
				 // (s.dhole 2012-05-25 17:57) - PLID 50617
				AfxMessageBox(FormatString ("There are %d active or deleted orders using this supplier/lab. You cannot Remove a supplier/lab that is in use." ,VarLong ( rs->Fields->Item["recCount"]->Value) )) ;
				return;
			 }
		}

		// Is thi item assig to loction?
		rs = CreateParamRecordset("SELECT Count(SupplierID) as recCount   FROM GlassesSupplierLocationsT  "
			" WHERE SupplierID = {INT} \r\n"		, nSupplierID);
		if(!rs->eof)
		{ 
			 if (VarLong( rs->Fields->Item["recCount"]->Value)>0 ){
				 // (s.dhole 2012-05-25 17:57) - PLID 50617
				 //s.dhole Fix capitalzation
				AfxMessageBox("There are several locations associated with this supplier/lab. You cannot remove a supplier/lab that is in use." ) ;
				return;
			 }
		}
		// Check how many catalog item we have configured for this supplier
		rs = CreateParamRecordset("Select   \r\n"
			" (Select count(*)  from GlassesSupplierCatalogDesignsT WHERE SupplierID ={INT}) + \r\n"
			" (Select count(*)  from GlassesSupplierCatalogFrameTypesT WHERE SupplierID ={INT}) +  \r\n"
			" (Select count(*)  from GlassesSupplierCatalogMaterialsT WHERE SupplierID ={INT}) +  \r\n"
			" (Select count(*)  from GlassesSupplierCatalogTreatmentsT WHERE SupplierID ={INT})  as recCount  \r\n"
		, nSupplierID, nSupplierID, nSupplierID, nSupplierID);
		if(!rs->eof){ 
			 if (VarLong( rs->Fields->Item["recCount"]->Value)>0 ){
				 //s.dhole Fix capitalzation
				AfxMessageBox("There are several Catalog items associated with this supplier/lab. You cannot Remove a supplier/lab that is in use."  ) ;
				return;
			 }
		}
	//Remove supplier from vision web table
	ExecuteParamSql(" UPDATE SupplierT SET VisionWebID =NULL  WHERE PersonID = {INT} ",nSupplierID );
	// refresh supplier combo and list to reflect changes
	m_VisionWebSupplierCombo ->Requery(); 
	m_VisionWebSupplierList ->RemoveRow(pRow) ; 
	}NxCatchAll("Error in CVisionWebSetupDlg::OnRemoveSupplier");
}

void CVisionWebSetupDlg::OnCatalogSetup()
{
	try{
		Save();
		IRowSettingsPtr pRow= m_VisionWebSupplierList->GetCurSel();
		long nSupplierID =0;
		if(pRow != NULL) {
			nSupplierID  = VarLong(pRow->GetValue(vwSupplierID)); //// replace 0 with enum value
			CInvVisionWebLocationDlg dlg(this);// PLID 42835 new Dialog to Maintain Glasses Catalog custom item(this)s
			dlg.m_nSupplierID = nSupplierID;
			dlg.m_strSupplier = VarString(pRow->GetValue(vwSupplierName)); // replace 1 with enum value
			dlg.DoModal();
		}
		else{
			return;
		}
	}NxCatchAll("Error in CVisionWebSetupDlg::OnCatalogSetup");
}

void CVisionWebSetupDlg::SelChangedVisionwebSupplierList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try{
		IRowSettingsPtr pRow= m_VisionWebSupplierList->GetCurSel();
		long nSupplierID =0;
		if(pRow != NULL) 
			m_SupplierCatalogBtn.EnableWindow(TRUE);
		else
			m_SupplierCatalogBtn.EnableWindow(FALSE);

		}NxCatchAll("Error in CVisionWebSetupDlg::SelChangedVisionwebSupplierList");
}

void CVisionWebSetupDlg::SelChangingVisionwebSupplierList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	
}

void CVisionWebSetupDlg::OnBtnClose()
{
	Save();
	CNxDialog::OnCancel();	// change to  CNxDialog
}

void CVisionWebSetupDlg::OnBnClickedBtnSupplierCatalog()
{
	CVisionWebSetupDlg::OnCatalogSetup();
}


void CVisionWebSetupDlg::OnBtnClickedVisionWebUrlSetup()
{
	try{
		CVisionWebServiceSetupDlg  dlg(this);
		dlg.DoModal();
		}NxCatchAll("Error in CVisionWebSetupDlg::OnBtnClickedVisionWebUrlSetup");
}

void CVisionWebSetupDlg::OnBtnClickedAddSupplier()
{
	try{
		IRowSettingsPtr pRow= m_VisionWebSupplierCombo->GetCurSel() ;
		long nSupplierID =0;
		CString strSupplierName;
		if(pRow != NULL){
			nSupplierID  = VarLong(pRow->GetValue(0));
			strSupplierName  = VarString(pRow->GetValue(1),"");
		}
		else{
			return;
		}
		
		if (nSupplierID==-1)
			return;
		CString  strSupplierCode;
		GetDlgItemText(IDC_SUPPLIER_VISIONWEB_CODE, strSupplierCode);	 
		strSupplierCode.TrimRight();
		strSupplierCode.TrimLeft();
		strSupplierCode.Replace(" ","") ;
		if (strSupplierCode.GetLength() !=4) {
			// (s.dhole 2012-05-25 17:57) - PLID 50617
			AfxMessageBox("Invalid VisionWeb supplier/lab code.\nPlease ensure that the VisionWeb supplier/lab code you entered is the same code you received from VisionWeb during registration.");
			return;
		}
		
		CVisionWebSetupDlg::SaveSupplier(nSupplierID,strSupplierCode);
		if (pRow != NULL ){
			m_VisionWebSupplierCombo->RemoveRow(pRow); 
		}
		pRow = m_VisionWebSupplierList ->GetNewRow();
		pRow->PutValue(vwSupplierID, nSupplierID); // replace 0 with enum value
		pRow->PutValue(vwSupplierName,_variant_t( strSupplierName) ); // replace 1 with enum value
		pRow->PutValue(vwSupplierCode ,_variant_t(  strSupplierCode)); // replace 2 with enum value
		m_VisionWebSupplierList->AddRowSorted(pRow ,NULL);
		m_VisionWebSupplierCombo->CurSel = m_VisionWebSupplierCombo->FindByColumn(vwSupplierID,-1L , NULL, VARIANT_FALSE); //converting -1 to -1L
		SetDlgItemText(IDC_SUPPLIER_VISIONWEB_CODE, CString(""));	 
		m_AddSupplierBtn.EnableWindow(FALSE);
		m_nxeSupplierCode.EnableWindow(FALSE);
	}NxCatchAll("Error in CVisionWebSetupDlg::OnBtnClickedAddSupplier");
}

void CVisionWebSetupDlg::SaveSupplier(long nSupplierID, LPCTSTR VisionWebCode)
{
try{
		ExecuteParamSql(" UPDATE SupplierT SET VisionWebID ={STRING}   \r\n"
						" WHERE PersonID = {INT}  ",VisionWebCode ,nSupplierID);
			}NxCatchAll("Error in CVisionWebSetupDlg::SaveSupplier");
}

void CVisionWebSetupDlg::EditingFinishingVisionwebSupplierList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try{
		IRowSettingsPtr pRow(lpRow);
		long nSupplierID =0;
		if(pRow != NULL) 
			nSupplierID  = VarLong(pRow->GetValue(0));
		else
			*pbCommit = FALSE;
		// (b.spivey, January 10, 2013) - PLID 54432 - remove number restriction on vision web codes. 
		CString  strSupplierCode = FormatString("%s", VarString(*pvarNewValue));
		strSupplierCode.TrimRight();
		strSupplierCode.TrimLeft();
		strSupplierCode.Replace(" ","") ;
		if (strSupplierCode.GetLength() !=4) {
			// (s.dhole 2012-05-25 17:57) - PLID 50617
			//s.dhole Fix capitalzation
			AfxMessageBox("Invalid VisionWeb supplier/lab code.\nPlease ensure that the VisionWeb supplier/lab code you entered is the same code you received from VisionWeb during registration.");
			*pbCommit = FALSE;
			return;
		}
		
		CVisionWebSetupDlg::SaveSupplier(nSupplierID,strSupplierCode);
	}NxCatchAll("Error in CVisionWebSetupDlg::OnEditingFinishingSupplierList");
}


void CVisionWebSetupDlg::OnCancel()
{
	try{
		Save();
		//Change as CDialog to CNxDialog
		CNxDialog::OnCancel();  
	}NxCatchAll("Error in CVisionWebSetupDlg::OnCancel");
}

void CVisionWebSetupDlg::Save()
{
	try{
		CString strTemp;
		GetDlgItemText(IDC_VISIONWEB_USERNAME, strTemp);
		SetRemotePropertyText(VISIONWEBSERVICE_USER_ID,strTemp,0,"<None>" );
		GetDlgItemText(IDC_VISIONWEB_PASSWORD, strTemp);
		_variant_t vPass = EncryptStringToVariant(strTemp);
		SetRemotePropertyImage(VISIONWEBSERVICE_USER_PASSWORD,vPass,0,"<None>");
	}NxCatchAll("Error in CVisionWebSetupDlg::Save");
}

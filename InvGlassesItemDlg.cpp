// InvGlassesItemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "InvGlassesItemDlg.h"
#include "InventoryRc.h"
#include "InternationalUtils.h"
#include "InvGlassesItemEditDlg.h"
// (s.dhole 2011-03-15 12:11) - PLID 42835 new Dialog to Maintain Glasses Catalog custom items
// CInvGlassesItemDlg dialog

IMPLEMENT_DYNAMIC(CInvGlassesItemDlg, CNxDialog)

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum gListColumns {
	gIsChecked = 0,
	gID ,
	gName,
	gCode,
	gProcessName,
	gProcessID,
	
};

enum ItemMenuType	{
	menuNone=0,
	menuEdit,
	menuDelete,
};

CInvGlassesItemDlg::CInvGlassesItemDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvGlassesItemDlg::IDD, pParent)
{

}

CInvGlassesItemDlg::~CInvGlassesItemDlg()
{
}

void CInvGlassesItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_GLASSES_ITEM_ADD, m_btnAdd);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	
}


BEGIN_MESSAGE_MAP(CInvGlassesItemDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_GLASSES_ITEM_ADD, &CInvGlassesItemDlg::OnBnClickedBtnAdd)
	
	ON_BN_CLICKED(IDOK, &CInvGlassesItemDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CInvGlassesItemDlg::OnBnClickedCancel)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CInvGlassesItemDlg, CNxDialog)
	ON_EVENT(CInvGlassesItemDlg, IDC_GLASSES_DESIGN_LIST, 6, CInvGlassesItemDlg::RButtonDownGlassesDesignList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()


BOOL CInvGlassesItemDlg::OnInitDialog() 
{
try {
	CNxDialog::OnInitDialog();
	m_btnOk.AutoSet(NXB_OK)  ;
	m_btnCancel.AutoSet(NXB_CANCEL );
	m_btnAdd.AutoSet(NXB_NEW);
	// (s.dhole 2011-04-11 12:58) - PLID 42835 Change display string 
	SetWindowText(FormatString("Select one or more %ss:", m_strDescription));
	m_SelectedItemList = BindNxDataList2Ctrl(IDC_GLASSES_DESIGN_LIST,GetRemoteData(),FALSE );
	m_SelectedItemList->FromClause = _bstr_t(m_strSQL);
	m_SelectedItemList ->Requery(); 
}NxCatchAll(__FUNCTION__);
	return TRUE;
}

HRESULT CInvGlassesItemDlg::Open(CString strTable,CString strName, CString strCode,CString strSQL, CString strDescription)
{
	m_strTable= strTable;
	m_strName = strName;
	m_strCode = strCode;
	m_strDescription=strDescription;
	m_strSQL=strSQL;
	return DoModal();
}

// CInvGlassesItemDlg message handlers

void CInvGlassesItemDlg::OnBnClickedBtnAdd()
{
try {
	CInvGlassesItemEditDlg dlg(this);

	if(IDOK == dlg.Open(m_strTable , m_strName , m_strCode,"","" , 1,-1,m_strDescription )) {
			if (dlg.m_nNewID>0){
			IRowSettingsPtr pRow = m_SelectedItemList->GetNewRow();
			pRow->PutValue(gIsChecked, g_cvarFalse);  // Assign explicate false bool value
			pRow->PutValue(gID, dlg.m_nNewID);
			pRow->PutValue(gName, _variant_t( dlg.m_strNewName) );
			pRow->PutValue(gCode ,_variant_t(  dlg.m_strNewCode) );
			pRow->PutValue(gProcessName , _variant_t( dlg.m_strNewGlassOrderProcessType));
			pRow->PutValue(gProcessID , dlg.m_nNewGlassOrderProcessTypeID );
			m_SelectedItemList->AddRowSorted(pRow ,NULL);
			m_SelectedItemList->CurSel =pRow;
			}
	}
		
}NxCatchAll(__FUNCTION__);
}


void CInvGlassesItemDlg::OnBnClickedOk()
{
try {
	if (ValidateAndStore()) {
		CNxDialog::OnOK();
	}
}NxCatchAll(__FUNCTION__);

}

CString CInvGlassesItemDlg::GetMultiSelectIDString()
{
	if (m_strMultiID.GetLength() >= 1)
		m_strMultiID = m_strMultiID.Left( m_strMultiID.GetLength() - 1 );
	return m_strMultiID;
}
//Casting Bool value befor commpare with false
BOOL CInvGlassesItemDlg::ValidateAndStore()
{
	long nSelection=0;
	m_strMultiID="";
	IRowSettingsPtr pRow = m_SelectedItemList->GetFirstRow();
	while(pRow) {
		if(VarBool( pRow->GetValue(gIsChecked)) != FALSE) {
			m_strMultiID += AsString(pRow->GetValue(gID)) + " ";
			nSelection++; // increase count so we can check that user must have selected a record
		}
		pRow = pRow->GetNextRow();
	}
	if (nSelection < 1)
	{
		MsgBox("You must select at least 1 item from the list" );
		return FALSE;
	}
	return TRUE;
}

void CInvGlassesItemDlg::OnBnClickedCancel()
{
	CNxDialog::OnCancel();
}

void CInvGlassesItemDlg::RButtonDownGlassesDesignList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL)
			return;
		// need to set current selection
		m_SelectedItemList->PutCurSel(pRow); 
		CPoint pt;
		GetCursorPos(&pt);
		GenerateMenu(pt);
		// (j.jones 2016-03-02 15:10) - PLID 47279 - replaced an incorrect function name with __FUNCTION__
	}NxCatchAll(__FUNCTION__);	
}


void CInvGlassesItemDlg::GenerateMenu(CPoint &pt) 
{
try
{
	CMenu mnu;
	mnu.CreatePopupMenu();
	mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, menuEdit, "Edit");
	mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, menuDelete, "Remove");

	int nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN| TPM_RIGHTBUTTON |TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
	switch(nCmdId) {
		case menuEdit:
			EditData();
			break;
		case menuDelete:
			DeleteData();
			break;
	}
	mnu.DestroyMenu();
}NxCatchAll(__FUNCTION__);
}
	
void CInvGlassesItemDlg::EditData()
{
try{
	IRowSettingsPtr pRow= m_SelectedItemList->GetCurSel() ;
	long nID =0;
	CString strNameValue,strCodeValue;
	if(pRow == NULL) 
		return ;
	// chek if this item is VisionWeb Item
	if (VarLong( pRow->GetValue(gProcessID)) ==2)
	{
		AfxMessageBox("You cannot Edit or Remove a VisionWeb item." ) ;
		return;
	}	
	nID=VarLong(pRow->GetValue(gID));
	strNameValue= VarString (pRow->GetValue(gName));
	strCodeValue= VarString(pRow->GetValue(gCode));
	// make sure that we do not allow user to edit any code which is used in order.
	_RecordsetPtr rs; 
	if (m_strTable.CompareNoCase("GlassesCatalogDesignsT" )==0){
		rs = CreateParamRecordset("SELECT Count(ID) as recCount   FROM GlassesOrderT  \r\n"
		" WHERE  ( RightGlassesOrderOtherInfoID IN  (Select ID From GlassesOrderOtherInfoT \r\n"
		" WHERE GlassesCatalogDesignsID = {INT}) \r\n"
		" OR \r\n"
		" leftGlassesOrderOtherInfoID IN (Select ID From GlassesOrderOtherInfoT \r\n"
		" WHERE GlassesCatalogDesignsID = {INT})) \r\n",nID , nID );
		if(!rs->eof){ 
			if (AdoFldLong(rs, "recCount",0)>0 ){
			AfxMessageBox(FormatString ("There are %d active or deleted orders using this Design code. You cannot edit a Design that is in use." ,AdoFldLong(rs, "recCount",0) )) ;
			return;
			}
		}

	}
	else if (m_strTable.CompareNoCase("GlassesCatalogFrameTypesT" ) ==0){
		rs = CreateParamRecordset("SELECT Count(GlassesCatalogFrameTypeID) as recCount   FROM GlassesOrderT  "
		" WHERE  GlassesCatalogFrameTypeID = {INT} \r\n"	, nID );
		if(!rs->eof){ 
			if (AdoFldLong(rs, "recCount",0)>0 ){
			AfxMessageBox(FormatString ("There are %d active or deleted orders using this Frame Type code. You cannot edit a Frame Type that is in use." ,AdoFldLong(rs, "recCount",0) )) ;
			return;
			}
		}
	}
	else if (m_strTable.CompareNoCase("GlassesCatalogMaterialsT" ) ==0)
	{
		rs = CreateParamRecordset("SELECT Count(ID) as recCount   FROM GlassesOrderT \r\n"
		" WHERE (RightGlassesOrderOtherInfoID IN  (Select ID From GlassesOrderOtherInfoT \r\n"
		" WHERE GlassesCatalogMaterialsID  = {INT}) \r\n"
		" OR \r\n"
		" leftGlassesOrderOtherInfoID IN (Select ID From GlassesOrderOtherInfoT \r\n"
		" WHERE GlassesCatalogMaterialsID  = {INT})) \r\n",nID , nID );
		if(!rs->eof){ 
			if (AdoFldLong(rs, "recCount",0)>0 ){
			AfxMessageBox(FormatString ("There are %d active or deleted orders using this Material code. You cannot edit a Material that is in use.",AdoFldLong(rs, "recCount",0) )) ;
			return;
			}
		}
	}
	else if (m_strTable.CompareNoCase("GlassesCatalogTreatmentsT" ) ==0)
	{
		rs = CreateParamRecordset("SELECT Count(ID) as recCount   FROM GlassesOrderT   \r\n"
		" WHERE  (RightGlassesOrderOtherInfoID IN (SELECT     GlassesOrderOtherInfoT.ID \r\n"
		" FROM  GlassesOrderTreatmentsT INNER JOIN  \r\n"
		" GlassesOrderOtherInfoT ON GlassesOrderTreatmentsT.GlassesOrderOtherInfoID = GlassesOrderOtherInfoT.ID \r\n"
		" WHERE GlassesOrderTreatmentsT.GlassesCatalogTreatmentID = {INT}) \r\n"
		" OR \r\n"
		" leftGlassesOrderOtherInfoID IN (SELECT     GlassesOrderOtherInfoT.ID \r\n"
		" FROM GlassesOrderTreatmentsT INNER JOIN  \r\n"
		" GlassesOrderOtherInfoT ON GlassesOrderTreatmentsT.GlassesOrderOtherInfoID = GlassesOrderOtherInfoT.ID \r\n"
		" WHERE GlassesOrderTreatmentsT.GlassesCatalogTreatmentID = {INT})) \r\n", nID , nID );
		if(!rs->eof){ 
			if (AdoFldLong(rs, "recCount",0)>0 ){
			AfxMessageBox(FormatString("There are %d active or deleted orders using this Treatment code. You cannot edit a Treatment that is in use.",AdoFldLong(rs, "recCount",0) )) ;
			return;
			}
		 }
	}
	
	CInvGlassesItemEditDlg dlg(this);
	if(IDOK == dlg.Open(m_strTable , m_strName , m_strCode ,strNameValue,strCodeValue, 1,nID,m_strDescription )) {
		pRow->PutValue(gName, _variant_t( dlg.m_strNewName) );
		pRow->PutValue(gCode ,_variant_t(  dlg.m_strNewCode) );
		m_SelectedItemList->CurSel =pRow;
	}
}NxCatchAll(__FUNCTION__);
}
void CInvGlassesItemDlg::DeleteData()
{
try{
	IRowSettingsPtr pRow= m_SelectedItemList->GetCurSel() ;
		long nID =0;
		if(pRow != NULL) 
			nID= VarLong(pRow->GetValue(gID));
		else
			return;

	if (VarLong( pRow->GetValue(gProcessID)) ==2)
		{
		AfxMessageBox("You cannot Edit or Remove a VisionWeb item." ) ;
		return;
		}	
		_RecordsetPtr rs; 
		if (m_strTable.CompareNoCase("GlassesCatalogDesignsT" )==0){
			 rs = CreateParamRecordset("SELECT Count(ID) as recCount   FROM GlassesSupplierCatalogDesignsT   \r\n"
			" WHERE GlassesCatalogDesignsID={INT}  \r\n",  nID );
			 if (!rs->eof){
				  if (AdoFldLong(rs, "recCount",0)>0 )
						AfxMessageBox("This Design is assign to the supplier. Please remove the design from this supplier catalog before "
						"removing it from the design list");
				  else{
					  // (s.dhole 2012-05-01 12:30) - PLID 43849 Make sure user Cannot delete link items
						rs = CreateParamRecordset("SELECT Count(GlassesCatalogDesignsID) as recCount   FROM GlassesCatalogDesignsCptT  \r\n"
						" WHERE  GlassesCatalogDesignsID = {INT} \r\n",nID );
						if(!rs->eof && AdoFldLong(rs, "recCount",0)>0 ){
							AfxMessageBox(FormatString ("There are %d Service Code(s) linked to this Design code. Please remove the Service Code(s) link from this Design before removing it from the Design list" ,AdoFldLong(rs, "recCount",0) )) ;
							return;
							
						}
						else{
						  ExecuteParamSql("DELETE  FROM GlassesCatalogDesignsT  WHERE ID={INT} AND GlassesOrderProcessTypeID=1  ",nID );
						  m_SelectedItemList->RemoveRow(pRow); 
						}
				  }
			 }

		}
		else if (m_strTable.CompareNoCase("GlassesCatalogFrameTypesT" ) ==0){
			rs = CreateParamRecordset("SELECT Count(ID) as recCount   FROM GlassesSupplierCatalogFrameTypesT   \r\n"
			" WHERE GlassesCatalogFrameTypesID={INT}  \r\n",  nID );
			 if (!rs->eof){
				  if (AdoFldLong(rs, "recCount",0)>0 )
					AfxMessageBox("This Frame Type is assign to the supplier. Please remove the frame type from this supplier catalog before "
					"removing it from the frame type list");
				  else{
					  ExecuteParamSql("DELETE  FROM GlassesCatalogFrameTypesT  WHERE ID={INT} AND GlassesOrderProcessTypeID=1   ",nID );
					  m_SelectedItemList->RemoveRow(pRow);
				  }

			 }
		}
		else if (m_strTable.CompareNoCase("GlassesCatalogMaterialsT" ) ==0)
		{
			rs = CreateParamRecordset("SELECT Count(ID) as recCount   FROM GlassesSupplierCatalogMaterialsT   \r\n"
			" WHERE GlassesCatalogMaterialID={INT}  \r\n",  nID );
			 if (!rs->eof){
				 if (AdoFldLong(rs, "recCount",0)>0 ){
					AfxMessageBox("This Material is assign to the supplier. Please remove the material from this supplier catalog before "
					"removing it from the material list");
					}
				  else{
					  	  // (s.dhole 2012-05-01 12:30) - PLID 43849 Make sure user Cannot delete link items
						rs = CreateParamRecordset("SELECT Count(GlassesCatalogMaterialsID) as recCount   FROM GlassesCatalogMaterialsCptT  \r\n"
						" WHERE  GlassesCatalogMaterialsID = {INT} \r\n",nID );
						if(!rs->eof && AdoFldLong(rs, "recCount",0)>0 ){ 
							AfxMessageBox(FormatString ("There are %d Service Code(s) linked to this Material code. Please remove the Service Code(s) link from this Material before removing it from the Material list" ,AdoFldLong(rs, "recCount",0) )) ;
							return;
						}
						else{
							  ExecuteParamSql("DELETE  FROM GlassesCatalogMaterialsT  WHERE ID={INT} AND GlassesOrderProcessTypeID=1   ",nID );
							  m_SelectedItemList->RemoveRow(pRow);
						}
				  }
			 }
		}
		else if (m_strTable.CompareNoCase("GlassesCatalogTreatmentsT" ) ==0)
		{
			rs = CreateParamRecordset("SELECT Count(ID) as recCount   FROM GlassesSupplierCatalogTreatmentsT   \r\n"
			" WHERE GlassesCatalogTreatmentsID={INT}  \r\n",  nID );
			 if (!rs->eof){
				  if (AdoFldLong(rs, "recCount",0)>0 )
					AfxMessageBox("This Treatment is assign to the supplier.  Please remove the treatment from this supplier catalog before "
					"removing it from the treatment list");
				  else{
					  // (s.dhole 2012-05-01 12:30) - PLID 43849 Make sure user Cannot delete link items
						rs = CreateParamRecordset("SELECT Count(GlassesCatalogTreatmentsID) as recCount   FROM GlassesCatalogTreatmentsCptT  \r\n"
						" WHERE  GlassesCatalogTreatmentsID = {INT} \r\n",nID );
						if(!rs->eof && AdoFldLong(rs, "recCount",0)>0 ){
								AfxMessageBox(FormatString ("There are %d Service Code(s) linked to this Treatment code. Please remove the Service Code(s) link from this Treatment before removing it from the Treatment list" ,AdoFldLong(rs, "recCount",0) )) ;
								return;
						}
							else{
								ExecuteParamSql("DELETE  FROM GlassesCatalogTreatmentsT  WHERE ID={INT} AND GlassesOrderProcessTypeID=1   ",nID );
								m_SelectedItemList->RemoveRow(pRow);
							}
				  
						}
			 }
		}
	}NxCatchAll(__FUNCTION__);
}

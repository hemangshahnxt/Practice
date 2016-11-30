// InvContactLensDataDlg.cpp : implementation file
//
// (s.dhole 2012-03-13 15:31) - PLID 48856 New Dialog
#include "stdafx.h"
#include "Practice.h"
#include "InvContactLensDataDlg.h"
#include "CategorySelectDlg.h"
#include "EditComboBox.h"
#include "InvUtils.h"
#include "AuditTrail.h"
#include "GlobalFinancialUtils.h"

// CInvContactLensDataDlg dialog
using namespace ADODB;
using namespace NXDATALIST2Lib;
IMPLEMENT_DYNAMIC(CInvContactLensDataDlg, CNxDialog)
enum ContactLensTypeColumns
{
	cltID = 0,
	cltContactLensType = 1,
};

// (s.dhole 2012-03-19 15:52) - PLID 48973 
enum ContactLensManufacturerColumns
{
	clmID = 0,
	clmName = 1,
};
CInvContactLensDataDlg::CInvContactLensDataDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvContactLensDataDlg::IDD, pParent)
{
	// (s.dhole 2012-03-19 15:52) - PLID 48973
	m_nContactLensType = -1 ;
	m_nContactLensManufacturerID = -1 ;
	m_nDefaultCategoryID = -1;
}

CInvContactLensDataDlg::~CInvContactLensDataDlg()
{
}

void CInvContactLensDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);	
	DDX_Control(pDX, IDCANCEL, m_btnCancel);	
	DDX_Control(pDX, IDC_CONTACT_LENS_PRODUCT_NAME, m_nxeditName);
	DDX_Control(pDX, IDC_NEW_CONTACT_LENS_CATEGORY, m_nxeditCategory);
	DDX_Control(pDX, IDC_CONTACT_LENS_PRODUCT, m_nxeditProduct);
	DDX_Control(pDX, IDC_CONTACT_LENS_QTY_PER_PACK, m_nxeditQtyPerBox);
	DDX_Control(pDX, IDC_CONTACT_LENS_TINT, m_nxeditTint);
	DDX_Control(pDX, IDC_BTN_CONTACT_LENS_CATEGORY_PICKER, m_btnPickCategory);
	DDX_Control(pDX, IDC_BTN_CONTACT_LENS_CATEGORY_REMOVE, m_btnRemoveCategory);
	DDX_Control(pDX, IDC_BTN_CONTACT_LENS_TYPE, m_btnEditType);
	DDX_Control(pDX, IDC_BTN_CONTACT_LENS_MANUFACTURER, m_nxbtneditManufacturer);
}


BEGIN_MESSAGE_MAP(CInvContactLensDataDlg, CNxDialog)
	
	ON_BN_CLICKED(IDOK, &CInvContactLensDataDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CInvContactLensDataDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_CONTACT_LENS_CATEGORY_PICKER, &CInvContactLensDataDlg::OnCategoryPicker)
	ON_BN_CLICKED(IDC_BTN_CONTACT_LENS_CATEGORY_REMOVE, &CInvContactLensDataDlg::OnCategoryRemove)
	ON_EN_KILLFOCUS(IDC_CONTACT_LENS_PRODUCT, &CInvContactLensDataDlg::OnEnKillfocusContactLensProduct)
	ON_EN_KILLFOCUS(IDC_CONTACT_LENS_QTY_PER_PACK, &CInvContactLensDataDlg::OnEnKillfocusContactLensQtyPerPack)
	ON_BN_CLICKED(IDC_BTN_CONTACT_LENS_TYPE, &CInvContactLensDataDlg::OnBnClickedBtnContactLensType)
	ON_EN_KILLFOCUS(IDC_CONTACT_LENS_TINT, &CInvContactLensDataDlg::OnEnKillfocusContactLensTint)
	ON_BN_CLICKED(IDC_BTN_CONTACT_LENS_MANUFACTURER, &CInvContactLensDataDlg::OnBnClickedBtnContactLensManufacturer)
END_MESSAGE_MAP()


BOOL CInvContactLensDataDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnPickCategory.AutoSet(NXB_MODIFY);
		m_btnRemoveCategory.AutoSet(NXB_DELETE);
		// (s.dhole 2012-03-19 15:52) - PLID 48973
		m_nxbtneditManufacturer.AutoSet(NXB_MODIFY);
		m_btnRemoveCategory.EnableWindow(FALSE);
		m_nxeditProduct.SetLimitText(50);
		m_nxeditQtyPerBox.SetLimitText(50);
		m_nxeditTint.SetLimitText(50);
		
		if (m_nProductID != -1)
		{
			GetDlgItem(IDC_STATIC_NEW_CONTACT_LENS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_NEW_CONTACT_LENS_CATEGORY)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_CONTACT_LENS_CATEGORY_PICKER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_CONTACT_LENS_CATEGORY_REMOVE)->ShowWindow(SW_HIDE);
		}
		
		if (m_nProductID != -1 ) {
			LoadData();
		}
		else {
			m_nContactLensType =-1 ;
			// (s.dhole 2012-03-19 15:52) - PLID 48973
			m_nContactLensManufacturerID=-1 ;
		}
		m_pLensType = BindNxDataList2Ctrl(IDC_DD_CONTACT_LENS_TYPE,false);
		m_pLensType->Requery(); 
		// (s.dhole 2012-03-19 15:52) - PLID 48973
		m_pLensManufacturer = BindNxDataList2Ctrl(IDC_DD_CONTACT_LENS_MANUFACTURER,false);
		m_pLensManufacturer->Requery(); 
		 
		 
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}
// CInvContactLensDataDlg message handlers

void CInvContactLensDataDlg::LoadData()
{
	try
	{
		SetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME,m_strFinalName  );
		_RecordsetPtr rsContactLensData = CreateParamRecordset("SELECT * FROM GlassesContactLensDataT  INNER JOIN  ProductT "
			" ON GlassesContactLensDataT.ID =ProductT.GlassesContactLensDataID "
			"WHERE ProductT.ID = {INT}", m_nProductID );
		if(!rsContactLensData->eof) {
			
			SetDlgItemText(IDC_CONTACT_LENS_PRODUCT ,AdoFldString(rsContactLensData,"ProductName",""));
			SetDlgItemText(IDC_CONTACT_LENS_TINT ,AdoFldString(rsContactLensData,"Tint",""));
			SetDlgItemText(IDC_CONTACT_LENS_QTY_PER_PACK ,AdoFldString(rsContactLensData,"LensesPerBox",""));
			m_nContactLensType =AdoFldLong(rsContactLensData,"GlassesContactLensTypeID",-1);
			// (s.dhole 2012-03-19 15:52) - PLID 48973
			m_nContactLensManufacturerID =AdoFldLong(rsContactLensData,"ContactLensManufacturerID",-1);
		}
		else {
			m_nContactLensType =-1;
			m_nContactLensManufacturerID = -1;

		}

	}NxCatchAll(__FUNCTION__);
}
CString  CInvContactLensDataDlg::GetContactLensName()
{
	CString strProductName, str;
	// (s.dhole 2012-03-19 15:52) - PLID 48973	
	IRowSettingsPtr pRow = m_pLensManufacturer->CurSel;  
	if (pRow  ){
		strProductName += VarString(pRow->GetValue(clmName),"") + ", ";
	}
	else
	{ //nothing
	}

	GetDlgItemText(IDC_CONTACT_LENS_PRODUCT ,str);
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}
	else
	{ //nothing
	}

	pRow = m_pLensType->CurSel;  
	if (pRow  ){
		strProductName += VarString(pRow->GetValue(cltContactLensType),"") + ", ";
	}
	else
	{ //nothing
	}

	GetDlgItemText(IDC_CONTACT_LENS_TINT ,str);
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}
	else
	{ //nothing
	}
	
	GetDlgItemText(IDC_CONTACT_LENS_QTY_PER_PACK ,str);
	if(!str.IsEmpty()) {
		strProductName += str + ", ";
	}
	else
	{ //nothing
	}

	strProductName.TrimRight(", ");
	return strProductName;
}

void CInvContactLensDataDlg::OnBnClickedOk()
{
	try {
		CString strProductName ;
		GetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
		if (strProductName== "")
		{	AfxMessageBox ("You must enter a name.");
			return;
		}
		if (m_nProductID != -1) {
			SaveContactLensData();
		}
		else {
			CString strProductName ;
			GetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
			if  (strProductName.IsEmpty() || m_nProductID<0) 
			{
				strProductName  = GetContactLensName();
				SetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
			}
			m_nProductID = SaveNewContactLensData();
		}

		if (m_nProductID >-1)
			CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CInvContactLensDataDlg::SaveContactLensData()
{
	long nNewProductID = -1;
	try {
		CString strProduct,strProductName,strTint
			,strLensesPerBox,strCategoryText;
		_variant_t varContactLensManufacturerID= g_cvarNull;
		_variant_t varContactLensType = g_cvarNull;
		
		CNxParamSqlArray params;
		CString strSqlBatch = BeginSqlBatch();
		
		GetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME   ,strProduct);
		GetDlgItemText(IDC_NEW_CONTACT_LENS_CATEGORY  ,strCategoryText);
		long nUserID =  GetCurrentUserID();
		GetDlgItemText(IDC_CONTACT_LENS_PRODUCT  ,strProductName );
		GetDlgItemText(IDC_CONTACT_LENS_TINT  ,strTint );
		GetDlgItemText(IDC_CONTACT_LENS_QTY_PER_PACK  ,strLensesPerBox );
		IRowSettingsPtr pRow = m_pLensType->CurSel;  
		if (pRow  ){
			varContactLensType  = VarLong(pRow->GetValue(cltID)) ;
		}
		else{ 
			//nothing
		}
		// (s.dhole 2012-03-19 15:52) - PLID 48973
		pRow = m_pLensManufacturer->CurSel;  
		if (pRow  ){
			varContactLensManufacturerID  = VarLong(pRow->GetValue(clmID)) ;
		}
		else
		{ //nothing
		}
		// s.dhole 06-06-2012 fix tint saveing 
		AddParamStatementToSqlBatch(strSqlBatch,params, "UPDATE  GlassesContactLensDataT SET ContactLensManufacturerID ={VT_I4} "
			" ,ProductName = {STRING},GlassesContactLensTypeID={VT_I4} ,Tint ={STRING} ,LensesPerBox = {STRING} " 
			" WHERE ID =  (Select  GlassesContactLensDataID FROM productT WHERE ID = {INT} ) "
			,varContactLensManufacturerID, strProductName, varContactLensType, strTint, strLensesPerBox, m_nProductID);
		AddParamStatementToSqlBatch(strSqlBatch,params, "UPDATE ServiceT SET name = {STRING} WHERE  ServiceT.ID = (SELECT ID FROM productT WHERE ID = {INT}  ) ", strProduct, m_nProductID);
		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, params);
		if (m_strFinalName!=strProduct)
			{
				long AuditID = -1;
				AuditID = BeginNewAuditEvent();
				if(AuditID != -1) {
					AuditEvent(-1, m_strFinalName,AuditID,aeiProductServiceName,m_nProductID ,m_strFinalName ,strProduct,aepHigh);
				}
			}
		
	}NxCatchAll(__FUNCTION__);
	
}


long CInvContactLensDataDlg::SaveNewContactLensData()
{
	long nNewProductID = -1;
	try {
		CString strContactLensManufacturerID	,strProductName,strTint
			,strLensesPerBox,strContactLensType,strProduct;
		
		CString strSqlBatch = BeginSqlBatch();
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewProductID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewContactLensDatasID INT");
		
		CString strBaseSql = strSqlBatch;
		GetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProduct);
		AddStatementToSqlBatch(strSqlBatch, "SET @nNewProductID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM ServiceT)");
		long nUserID = GetRemotePropertyInt("DefaultProductOrderingUser", -1, 0, "<None>");
		// (j.jones 2005-01-31 17:22) - the user could have been deleted or inactivated
		if(nUserID == -1 || 
			!ReturnsRecordsParam("SELECT ID FROM PersonT INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID "
			"WHERE Archived = 0 AND ID = {INT}", nUserID)) {
			nUserID = GetCurrentUserID();
		}
		InvUtils::AddCreateProductSqlToBatch(strSqlBatch, "@nNewProductID", strProduct, TRUE, FALSE, FALSE, tsTrackQuantity , nUserID);
		GetDlgItemText(IDC_CONTACT_LENS_PRODUCT  ,strProductName );
		GetDlgItemText(IDC_CONTACT_LENS_TINT  ,strTint );
		GetDlgItemText(IDC_CONTACT_LENS_QTY_PER_PACK  ,strLensesPerBox );
		strContactLensType = "NULL";
		IRowSettingsPtr pRow = m_pLensType->CurSel;  
		if (pRow  ){
			if (VarLong(pRow->GetValue(cltID),-1) !=-1){
				strContactLensType = AsString( VarLong(pRow->GetValue(cltID)) );
			}
			else
			{ //nothing
			}
		}
		else
		{ //nothing
		}

		strContactLensManufacturerID = "NULL";
		// (s.dhole 2012-03-19 15:52) - PLID 48973
		pRow = m_pLensManufacturer->CurSel;  
		if (pRow  ){
			strContactLensManufacturerID  = AsString( VarLong(pRow->GetValue(clmID))) ;
		}
		else
		{ //nothing
		}
		// (s.dhole 2012-03-19 15:52) - PLID 48973 Fix SQL 
		AddStatementToSqlBatch(strSqlBatch, FormatString("INSERT INTO GlassesContactLensDataT (ContactLensManufacturerID "
			" ,ProductName,GlassesContactLensTypeID,Tint ,LensesPerBox)"
			" VALUES (%s, '%s',%s, '%s', '%s') ",strContactLensManufacturerID ,_Q(strProductName),strContactLensType,	_Q(strTint),_Q(strLensesPerBox)));
		AddStatementToSqlBatch(strSqlBatch, "SET @nNewContactLensDatasID = (SELECT CONVERT(int, SCOPE_IDENTITY()))");
		
		AddStatementToSqlBatch(strSqlBatch, 
			"UPDATE ProductT \r\n"
			"SET IsContactLens =1 , GlassesContactLensDataID = @nNewContactLensDatasID \r\n"
			"WHERE ProductT.ID = @nNewProductID "
			);
		
		ADODB::_RecordsetPtr prsCreateProduct = CreateRecordsetStd(
		"SET XACT_ABORT ON \r\n"
		"SET NOCOUNT ON \r\n"
		"BEGIN TRAN \r\n" + strSqlBatch + "\r\nCOMMIT TRAN \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT ID AS NewProductID \r\n"
		"FROM ServiceT \r\n"
		"WHERE ServiceT.ID = @nNewProductID \r\n");
		
		if(!prsCreateProduct->eof) {
			nNewProductID = AdoFldLong(prsCreateProduct->GetFields(), "NewProductID");
			m_strFinalName = strProduct;
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1){
				AuditEvent(-1, "", nAuditID, aeiProductCreate, nNewProductID, "", strProduct, aepMedium, aetChanged );
			}

			// (j.jones 2015-03-03 14:44) - PLID 65110 - save the product categories via the API, if they picked any
			if (m_aryCategoryIDs.size() > 0) {
				std::vector<long> aryServiceIDs;
				aryServiceIDs.push_back(nNewProductID);
				UpdateServiceCategories(aryServiceIDs, m_aryCategoryIDs, m_nDefaultCategoryID, true);
			}

			// (b.eyers 2015-07-10) - PLID 24060 - newly created contact lens needs to send a tablechecker
			CClient::RefreshTable(NetUtils::Products, nNewProductID);

			return nNewProductID;	
		}
		else {
			ThrowNxException("CInvContactLensDataDlg::SaveNewContactLensData - eof error when trying to get new product ID");
		}
		return nNewProductID;	
	}NxCatchAll(__FUNCTION__);
	return nNewProductID;	
}

void CInvContactLensDataDlg::OnBnClickedCancel()
{
	try {
		CNxDialog::OnCancel();		
	}NxCatchAll(__FUNCTION__);
}


void CInvContactLensDataDlg::OnEnKillfocusContactLensProduct()
{
	try {
		CString  strProductName  ;
		GetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
		if  (strProductName.IsEmpty() || m_nProductID<0) 
		{
			strProductName  = GetContactLensName();
			SetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
		}
	}NxCatchAll(__FUNCTION__);
}

void CInvContactLensDataDlg::OnEnKillfocusContactLensQtyPerPack()
{
	try {
		CString  strProductName  ;
		GetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
		if  (strProductName.IsEmpty() || m_nProductID<0) 
		{
			strProductName  = GetContactLensName();
			SetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
		}
	}NxCatchAll(__FUNCTION__);
}



void CInvContactLensDataDlg::OnEnKillfocusContactLensTint()
{
	try {
		CString  strProductName  ;
		GetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
		if  (strProductName.IsEmpty() || m_nProductID<0) 
		{
			strProductName  = GetContactLensName();
			SetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
		}
	}NxCatchAll(__FUNCTION__);
}

void CInvContactLensDataDlg::OnCategoryPicker()
{
	//
	// You may now choose a category for the new
	// inventory item here
	//
	try {

		// (j.jones 2015-03-03 16:22) - PLID 65110 - products can now have multiple categories

		// (j.jones 2015-02-27 16:22) - PLID 64962 - added bAllowMultiSelect, true for contact lens info.
		// (j.jones 2015-03-02 15:36) - PLID 64970 - added strItemType
		CCategorySelectDlg dlg(this, true, "contact lens");
		
		// (j.jones 2015-03-02 08:55) - PLID 64962 - this dialog supports multiple categories
		if (m_aryCategoryIDs.size() > 0) {
			dlg.m_aryInitialCategoryIDs.insert(dlg.m_aryInitialCategoryIDs.end(), m_aryCategoryIDs.begin(), m_aryCategoryIDs.end());
			dlg.m_nInitialDefaultCategoryID = m_nDefaultCategoryID;
		}

		// (j.jones 2015-03-02 10:18) - PLID 64962 - this now is just an OK/Cancel dialog
		if (IDOK == dlg.DoModal()) {	// Greater than zero means the user picked a valid category

			// (j.jones 2015-03-03 16:22) - PLID 65110 - products can now have multiple categories
			m_aryCategoryIDs.clear();
			m_aryCategoryIDs.insert(m_aryCategoryIDs.end(), dlg.m_arySelectedCategoryIDs.begin(), dlg.m_arySelectedCategoryIDs.end());
			m_nDefaultCategoryID = dlg.m_nSelectedDefaultCategoryID;

			CString strCategoryNames;
			LoadServiceCategories(m_aryCategoryIDs, m_nDefaultCategoryID, strCategoryNames);

			SetDlgItemText(IDC_NEW_CONTACT_LENS_CATEGORY, strCategoryNames);
			m_btnRemoveCategory.EnableWindow(m_aryCategoryIDs.size() > 0);
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CInvContactLensDataDlg::OnCategoryRemove()
{
	try {
		
		// (j.jones 2015-03-03 16:27) - PLID 65110 - products can now have multiple categories
		m_aryCategoryIDs.clear();
		m_nDefaultCategoryID = -1;
		SetDlgItemText(IDC_NEW_CONTACT_LENS_CATEGORY, "");
		m_btnRemoveCategory.EnableWindow(FALSE);
	}
	NxCatchAll(__FUNCTION__);
}

void CInvContactLensDataDlg::OnBnClickedBtnContactLensType()
{
	try {
		//Call CEditComboBox
		/*CONTACT_LENS_TYPE*/
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 77, m_pLensType, "Edit Contact Lens Type").DoModal();
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2012-03-19 15:52) - PLID 48973
void CInvContactLensDataDlg::OnBnClickedBtnContactLensManufacturer()
{
	try {
		//Call CEditComboBox
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		/*CONTACT_LENS_TYPE*/;
		CEditComboBox(this, 78, m_pLensManufacturer, "Edit Contact Lens Manufacturer").DoModal();
	}NxCatchAll(__FUNCTION__);
}


BEGIN_EVENTSINK_MAP(CInvContactLensDataDlg, CNxDialog)
	ON_EVENT(CInvContactLensDataDlg, IDC_DD_CONTACT_LENS_TYPE, 16, CInvContactLensDataDlg::SelChosenDdContactLensType, VTS_DISPATCH)
	ON_EVENT(CInvContactLensDataDlg, IDC_DD_CONTACT_LENS_TYPE, 1, CInvContactLensDataDlg::SelChangingDdContactLensType, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvContactLensDataDlg, IDC_DD_CONTACT_LENS_TYPE, 18, CInvContactLensDataDlg::RequeryFinishedDdContactLensType, VTS_I2)
	ON_EVENT(CInvContactLensDataDlg, IDC_DD_CONTACT_LENS_MANUFACTURER, 16, CInvContactLensDataDlg::SelChosenDdContactLensManufacturer, VTS_DISPATCH)
	ON_EVENT(CInvContactLensDataDlg, IDC_DD_CONTACT_LENS_MANUFACTURER, 1, CInvContactLensDataDlg::SelChangingDdContactLensManufacturer, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInvContactLensDataDlg, IDC_DD_CONTACT_LENS_MANUFACTURER, 18, CInvContactLensDataDlg::RequeryFinishedDdContactLensManufacturer, VTS_I2)
END_EVENTSINK_MAP()

void CInvContactLensDataDlg::SelChosenDdContactLensType(LPDISPATCH lpRow)
{
	try {
		//TES 5/25/2011 - PLID 43842 - If they selected a note, replace the current note with what they selected
		IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			//(s.dhole 06/11/2012) PLID 48856 Load selection
			m_nContactLensType  = VarLong(pRow->GetValue(cltID),-1); 
			CString  strProductName  ;
			GetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
			if  (strProductName.IsEmpty() || m_nProductID<0) 
			{
				strProductName  = GetContactLensName();
				SetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
			}
			
		}
	}NxCatchAll(__FUNCTION__);
}

void CInvContactLensDataDlg::SelChangingDdContactLensType(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

void CInvContactLensDataDlg::RequeryFinishedDdContactLensType(short nFlags)
{
	try {
			m_pLensType->FindByColumn(cltID , m_nContactLensType , NULL, g_cvarTrue);
			CString strProductName ;
			GetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
			if  (strProductName.IsEmpty() || m_nProductID<0) 
			{
				strProductName  = GetContactLensName();
				SetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
			}
		
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2012-03-19 15:52) - PLID 48973
void CInvContactLensDataDlg::SelChosenDdContactLensManufacturer(LPDISPATCH lpRow)
{
	try {
		//TES 5/25/2011 - PLID 43842 - If they selected a note, replace the current note with what they selected
		IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			m_nContactLensManufacturerID  = VarLong(pRow->GetValue(clmID),-1); 
			CString  strProductName  ;
			GetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
			if  (strProductName.IsEmpty() || m_nProductID<0) 
			{
				strProductName  = GetContactLensName();
				SetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2012-03-19 15:52) - PLID 48973
void CInvContactLensDataDlg::SelChangingDdContactLensManufacturer(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (s.dhole 2012-03-19 15:52) - PLID 48973
void CInvContactLensDataDlg::RequeryFinishedDdContactLensManufacturer(short nFlags)
{
	try {
		m_pLensManufacturer ->FindByColumn(clmID , m_nContactLensManufacturerID , NULL, g_cvarTrue);
		CString strProductName ;
		GetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
		if  (strProductName.IsEmpty() || m_nProductID<0) 
		{
			strProductName  = GetContactLensName();
			SetDlgItemText(IDC_CONTACT_LENS_PRODUCT_NAME  ,strProductName );
		}
	}NxCatchAll(__FUNCTION__);
}

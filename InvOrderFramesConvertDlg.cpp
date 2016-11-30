// InvOrderFramesConvertDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "InvOrderFramesConvertDlg.h"
#include "InvUtils.h"
#include "FramesData.h"
#include "AuditTrail.h"
#include "InvOrderSelectFrameDlg.h"


// CInvOrderFramesConvertDlg dialog
// (b.spivey, November 22, 2011) - PLID 45265 - Created. 

IMPLEMENT_DYNAMIC(CInvOrderFramesConvertDlg, CNxDialog)

CInvOrderFramesConvertDlg::CInvOrderFramesConvertDlg(const long nSupplierID, const CString strSupplierName, const CString strBarcodeNum, CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvOrderFramesConvertDlg::IDD, pParent)
{	
	m_nSupplierID = nSupplierID; 
	m_strSupplierName = strSupplierName; 
	m_strBarcodeNum = strBarcodeNum; 
	//m_strNoMarkUp is a static option to account for the lack of a mark up. It should never change. 
	m_strNoMarkUp = " {No Markup} "; 
	m_nProductID = -1;
	// (b.spivey, November 28, 2011) - PLID 45265 - Set to -1 as a default so we don't needlessly query the database. 
	m_nFramesID = -1; 
}

CInvOrderFramesConvertDlg::~CInvOrderFramesConvertDlg()
{
}

void CInvOrderFramesConvertDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CInvOrderFramesConvertDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CInvOrderFramesConvertDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CInvOrderFramesConvertDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_SELECT_FRAME, &CInvOrderFramesConvertDlg::OnBnClickedSelectFrame)
END_MESSAGE_MAP()


// CInvOrderFramesConvertDlg message handlers

BOOL CInvOrderFramesConvertDlg::OnInitDialog() 
{
	// (b.spivey, November 28, 2011) - PLID 45265 - Added try/catch
	try {
		// (b.spivey, September 21, 2011) - PLID 45265 - If they're unlicensed or don't have permissions, they shouldn't even see this dialogue. 
		if(!CheckCurrentUserPermissions(bioInvItem, sptCreate, 0, 0, 1) || 
			!g_pLicense->CheckForLicense(CLicense::lcFrames, CLicense::cflrUse)){
			OnBnClickedCancel(); 
		}

		CNxDialog::OnInitDialog(); 
		CNxDialog::CenterWindow(); 

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pMarkUpSelect = BindNxDataList2Ctrl(IDC_MARKUP_SELECT, true);
		m_pMarkUpSelect->SetSelByColumn(emuName, _variant_t(m_strNoMarkUp));

		//Need to get the most likely FramesDataT.ID
		// (b.spivey, September 20, 2011) - PLID 45265 - If we don't have a barcode, m_strBarcodeNum should be empty, and the 
		//	  recordset will be empty too. 
		// (b.spivey, November 28, 2011) - PLID 45265 - The above comment is partly wrong. If we m_strBarcodeNum is empty, we shouldn't query the 
		//	  database at all. 
		if(!m_strBarcodeNum.IsEmpty()){
			ADODB::_RecordsetPtr prs = CreateParamRecordset(
				"SELECT ID "
				"FROM FramesDataT "
				"WHERE UPC = {STRING} AND UPC <> '' "
				"ORDER BY YearIntroduced ", m_strBarcodeNum);
			
			if(!prs->eof){
				m_nFramesID = AdoFldLong(prs->Fields, "ID", -1); 
			}
		}
		else{
			//Already set to -1. 
		}
		
		//If there is a frame with this FrameID, we show them the intended product. 
		CString strFrameProduct; 
		// (b.spivey, November 28, 2011) - PLID 45265 - If it's a negative one, don't bother the database. 
		if(m_nFramesID != -1 && (m_fdFramesData.LoadFromFrameID(m_nFramesID))){
			strFrameProduct.Format("%s", m_fdFramesData.GetProductName()); 
		}
		else{
			strFrameProduct.Format("%s", "No frame selected.");
		}
		SetDlgItemText(IDC_SCANNED_FRAME, strFrameProduct); 

		// (b.spivey, September 20, 2011) - PLID 45265 - This is a strongly worded warning detailing the options and that the supplier 
		//   is *automatically* chosen at this stage, meaning that if they want to enter a different supplier they have to fix it later. 
		// (b.spivey, September 22, 2011) - PLID 45265 - Added extra white space. 
		CString strWarning; 
		strWarning.Format("The following frame was found in the frames database. You can create a product for this frame by "
			"pressing OK at this screen. The supplier will automatically be set to the supplier from the Order Form, and you "
			"can set the markup from the drop down here. You can change the selected frame to convert by clicking "
			"the ellipsis. Pressing OK at this screen will create this product even if you cancel the order. \r\n\r\n "
			"Do you want to create the following product with the supplier %s? ", m_strSupplierName);
		GetDlgItem(IDC_STATICWARNING)->SetWindowTextA(strWarning); 	

		return TRUE;
	}NxCatchAll(__FUNCTION__); 
	return FALSE; 
}

BOOL CInvOrderFramesConvertDlg::CreateProductFromFrame()
{
	// (b.spivey, September 20, 2011) - PLID 45265 - Several lines of code was borrowed from the way the FramesTab does this, 
	//   I left most of the comments in as this is the core functionality of the item. I also added comments where I felt necessary
	try{
		// (b.spivey, September 15, 2011) - PLID 45265 - We need to silently pull these billing fields. 
		//	 We have these enums, might as well use them. And cyZero is useful too for a default value. 
		COleCurrency cyZero(0, 0);
		BOOL bBillable = (GetRemotePropertyInt("InvFrameOpBillable", 1, 0, "<None>") == 1);
		BOOL bTaxable1 = (GetRemotePropertyInt("InvFrameOpTaxable1", 0, 0, "<None>") == 1);
		BOOL bTaxable2 = (GetRemotePropertyInt("InvFrameOpTaxable2", 0, 0, "<None>") == 1);
		TrackStatus eTrackStatus = (TrackStatus)GetRemotePropertyInt("InvFrameOpTracking", tsTrackQuantity, 0, "<None>");
		long nCategoryID = GetRemotePropertyInt("InvFrameOpCategory", fpcvNone, 0, "<None>");

		// (b.spivey, September 20, 2011) - PLID 45265 - Set the category. If it's set from frames data, it's a framesData 
		//   category which requires extra query work. Else, if uses the set category. 
		CString strCategory;
		BOOL bCategoryFromFramesData = FALSE;
		switch(nCategoryID)
		{
			case fpcvNone:
				strCategory = "NULL";
				break;

			case fpcvCollection:
			case fpcvManufacturer:
			case fpcvBrand:
			case fpcvGroup:
				bCategoryFromFramesData = TRUE;
				break;

			default:
				strCategory = AsString(nCategoryID);
				break;
		}

		//Default ordering user if we have one. 
		long nUserID = GetRemotePropertyInt("DefaultProductOrderingUser", -1, 0, "<None>");
		// (j.jones 2005-01-31 17:22) - the user could have been deleted or inactivated
		if(nUserID == -1 || 
			!ReturnsRecordsParam("SELECT ID FROM PersonT INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID "
			"WHERE Archived = 0 AND ID = {INT}", nUserID)) {
			nUserID = GetCurrentUserID();
		}


		// (b.spivey, September 15, 2011) - PLID 45265 - Batch starts here. 
		CString strSqlBatch = BeginSqlBatch();
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewProductID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewFramesDataID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nCategoryID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nSupplierID INT"); 
		CString strBaseSql = strSqlBatch;
		long nNewProductID = -1;

		if(bCategoryFromFramesData) {
			CString strCategoryText = m_fdFramesData.GetCategoryText((EFramesProductCategoryOption)nCategoryID);
			// (z.manning 2011-01-24 15:33) - PLID 42193 - As a failsafe, make sure the category name will
			// fit in the database field.
			if(strCategoryText.GetLength() > 100) {
				strCategoryText = strCategoryText.Left(100);
			}
			AddStatementToSqlBatch(strSqlBatch,
				"SET @nCategoryID = (SELECT ID FROM CategoriesT WHERE Name = '%s') \r\n"
				"IF @nCategoryID IS NULL BEGIN \r\n"
				"	SET @nCategoryID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM CategoriesT) \r\n"
				"	INSERT INTO CategoriesT (ID, Name, Parent) VALUES (@nCategoryID, '%s', 0) \r\n"
				"END \r\n"
				, _Q(strCategoryText), _Q(strCategoryText));
			strCategory = "@nCategoryID";
		}

		// (b.spivey, September 20, 2011) - PLID 45265 - Automatically create a product name and set the supplier ID based on 
		//	 the user's selection in the new order dialog. The settings are pulled from the Frames Option dialog from the frames tab.
		CString strProductName = m_fdFramesData.GetProductName();
		AddStatementToSqlBatch(strSqlBatch, "SET @nNewProductID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM ServiceT)");
		InvUtils::AddCreateProductSqlToBatch(strSqlBatch, "@nNewProductID", strProductName, bBillable, bTaxable1, bTaxable2, eTrackStatus, nUserID);
		AddStatementToSqlBatch(strSqlBatch, m_fdFramesData.GetInsertSql(FALSE));
		AddStatementToSqlBatch(strSqlBatch, "SET @nNewFramesDataID = (SELECT CONVERT(int, SCOPE_IDENTITY()))");
		AddStatementToSqlBatch(strSqlBatch, "SET @nSupplierID = %li ", m_nSupplierID);
		AddStatementToSqlBatch(strSqlBatch, 
			"UPDATE ProductT \r\n"
			"SET FramesDataID = @nNewFramesDataID \r\n"
			"	, LastCost = COALESCE(FramesDataT.CompletePrice, 0) \r\n"
			"FROM ProductT \r\n"
			"INNER JOIN FramesDataT ON FramesDataT.ID = @nNewFramesDataID \r\n"
			"WHERE ProductT.ID = @nNewProductID "
			);

		// (j.jones 2015-03-04 12:58) - PLID 65113 - update ServiceT.Category and ServiceMultiCategoriesT
		if (strCategory.GetLength() > 0 && strCategory != "NULL") {
			AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET Category = %s WHERE ID = @nNewProductID", strCategory);
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ServiceMultiCategoryT (ServiceID, CategoryID) VALUES (@nNewProductID, %s)", strCategory);
		}

		CString strPriceUpdateSql;

		// (b.spivey, September 15, 2011) - PLID 45265 - If we have a selection, we apply the mark up. Else assume zero. 
		if(VarLong(m_pMarkUpSelect->CurSel->GetValue(emuID)) != -1) {
			// (b.spivey, September 15, 2011) - PLID 45265 - Grab the formula from the database (2 is the formula string)
			CString strFormula = VarString(m_pMarkUpSelect->CurSel->GetValue(emuFormula), ""); 
			_variant_t varCost = m_fdFramesData.GetValueByDataField("CompletePrice");

			// (b.eyers 2016-03-14) - PLID 68590 - round up is now a long instead of a bit
			long nRoundUp = VarLong(m_pMarkUpSelect->CurSel->GetValue(emuRoundUp));


			// (b.spivey, September 15, 2011) - PLID 45265 - Assume zero as a default. 
			COleCurrency cyCost = VarCurrency(varCost, cyZero);

			// (b.spivey, September 15, 2011) - PLID 45265 - Greater than zero means we got a cost back, so we can 
			//	 get the price using the formula we got from the datalist. 
			if(cyCost > cyZero) {
				// (j.jones 2016-05-17 11:10) - PLID-68615 - markups now return a currency
				COleCurrency cyNewPrice = COleCurrency(0, 0);
				if(InvUtils::CalculateMarkupPrice(cyCost, strFormula, cyNewPrice, nRoundUp)) {
					// (j.jones 2016-05-17 11:10) - PLID-68615 - make absolutely sure the price is rounded
					strPriceUpdateSql = FormatString(", Price = Round(Convert(money, '%s'), 2)", FormatCurrencyForSql(cyNewPrice));
				}
			}
		}

		// (b.spivey, September 20, 2011) - PLID 45265 - Set the UPC code based on the FramesDataT.ID 
		AddStatementToSqlBatch(strSqlBatch, 
			"UPDATE ServiceT \r\n"
			"SET Barcode = (SELECT FramesDataT.UPC FROM FramesDataT WHERE FramesDataT.ID = @nNewFramesDataID) \r\n"
			"	%s \r\n"
			"WHERE ID = @nNewProductID ", strPriceUpdateSql);

		
		// (b.spivey, September 15, 2011) - PLID 45265 - We need to set the supplier automatically so that it may be added to the order. 
		// (b.spivey, September 27, 2011) - PLID 45265 - Forgot to add a where clause on this, so it was updating *all* products in ProductT
		AddStatementToSqlBatch(strSqlBatch, 
			"INSERT INTO MultiSupplierT (SupplierID, ProductID) VALUES (@nSupplierID, @nNewProductID) "
			"UPDATE ProductT SET DefaultMultiSupplierID = "
			"	(SELECT ID FROM MultiSupplierT WHERE SupplierID = @nSupplierID and ProductID = @nNewProductID) "
			"		WHERE ProductT.ID = @nNewProductID ");



		// (b.spivey, September 15, 2011) - PLID 45265 - Auditing
		// (b.eyers 2015-07-10) - PLID 24060 - get category
		ADODB::_RecordsetPtr prs = CreateRecordsetStd(
		"SET NOCOUNT ON \r\n"
		"BEGIN TRAN \r\n" + strSqlBatch + "\r\nCOMMIT TRAN \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT ID, Name, Price, Category \r\n"
		"FROM ServiceT \r\n"
		"WHERE ServiceT.ID = @nNewProductID \r\n"
		);

		CAuditTransaction auditTran;

		if(!prs->eof) {

			nCategoryID = AdoFldLong(prs->GetFields(), "Category", -1); // (b.eyers 2015-07-10) - PLID 24060 - get category
			nNewProductID = AdoFldLong(prs->GetFields(), "ID");	
			CString strProdName = AdoFldString(prs->GetFields(), "Name", "");
			
			AuditEvent(-1, strProdName, auditTran, aeiProductCreate, nNewProductID, "", strProdName, aepMedium, aetCreated);

			// (z.manning 2010-09-22 13:08) - PLID 40619 - Also audit the price if we calculated one
			COleCurrency cyPrice = AdoFldCurrency(prs->GetFields(), "Price", cyZero);
			
			if(cyPrice != cyZero) {
				AuditEvent(-1, strProdName, auditTran, aeiProductServicePrice, nNewProductID, "", FormatCurrencyForInterface(cyPrice), aepMedium, aetChanged);
			}

		}
		auditTran.Commit();

		CClient::RefreshTable(NetUtils::Products, nNewProductID);
		// (b.eyers 2015-07-10) - PLID 24060 - send category table checker
		if (nCategoryID != -1)
			CClient::RefreshTable(NetUtils::CPTCategories, nCategoryID); 
		// (b.spivey, September 21, 2011) - PLID 45265 - we need this to add the item. 
		m_nProductID = nNewProductID; 
		return TRUE; 

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}
void CInvOrderFramesConvertDlg::OnBnClickedOk()
{
	try{
		// (b.spivey, September 20, 2011) - PLID 45265 - Negative one is a sentinel value for "no selection." If they somehow 
		//   managed to reach that state, don't save. 
		if(m_nFramesID != -1){
			CreateProductFromFrame();
			CNxDialog::OnOK(); 
		}
		else{
			MessageBox("Please select a frame.", "Warning", MB_ICONWARNING); 
		}
	}NxCatchAll(__FUNCTION__); 
}

void CInvOrderFramesConvertDlg::OnBnClickedCancel()
{
	try{
		CNxDialog::OnCancel();
	}NxCatchAll(__FUNCTION__); 
}

// (b.spivey, September 20, 2011) - PLID 45266 - If they want to make a selection themselves, the option is available to them. 
void CInvOrderFramesConvertDlg::OnBnClickedSelectFrame()
{
	try{
		CInvOrderSelectFrameDlg dlg(this); 
		//If they pressed OK while they had a selection, then we'll go ahead and update this dialog, else nothing happens. 
		if(dlg.DoModal() == IDOK && dlg.m_nFramesID != -1){
			m_nFramesID = dlg.m_nFramesID;
			m_fdFramesData.LoadFromFrameID(m_nFramesID); 
			SetDlgItemText(IDC_SCANNED_FRAME, m_fdFramesData.GetProductName()); 
		}
	}NxCatchAll(__FUNCTION__); 
}
BEGIN_EVENTSINK_MAP(CInvOrderFramesConvertDlg, CNxDialog)
	ON_EVENT(CInvOrderFramesConvertDlg, IDC_MARKUP_SELECT, 1, CInvOrderFramesConvertDlg::SelChangingMarkupSelect, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

void CInvOrderFramesConvertDlg::SelChangingMarkupSelect(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try{
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__)
}

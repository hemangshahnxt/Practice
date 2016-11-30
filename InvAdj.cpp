// InvAdj.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "InvAdj.h"
#include "InvUtils.h"
#include "GlobalFinancialUtils.h"
#include "AuditTrail.h"
#include "ProductItemsDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "InventoryRc.h"
#include "EditProductAdjustmentCategoriesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



//DRT 5/8/2006 - PLID 20484 - Only use this from OnInitDialog
#define IDT_INIT_LOAD_QTY WM_USER + 10000

/////////////////////////////////////////////////////////////////////////////
// CInvAdj dialog


CInvAdj::CInvAdj(CWnd* pParent)
	: CNxDialog(CInvAdj::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInvAdj)
	m_bUseUU = FALSE;
	m_UUConversion = 1;
	m_bEnterSingleSerialPerUO = FALSE;
	//}}AFX_DATA_INIT
}


void CInvAdj::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvAdj)
	DDX_Control(pDX, IDC_EDIT_ADJ_CATEGORIES, m_nxbEditAdjCategories);
	DDX_Control(pDX, IDC_BTN_AUTO_CALC, m_btnAutoCalc);
	DDX_Control(pDX, IDOK, m_okBtn);
	DDX_Control(pDX, IDCANCEL, m_cancelBtn);
	DDX_Control(pDX, IDC_NAME, m_nxeditName);
	DDX_Control(pDX, IDC_STOCK, m_nxeditStock);
	DDX_Control(pDX, IDC_STOCK_UO, m_nxeditStockUo);
	DDX_Control(pDX, IDC_QUANTITY, m_nxeditQuantity);
	DDX_Control(pDX, IDC_QUANTITY_UO, m_nxeditQuantityUo);
	DDX_Control(pDX, IDC_COST, m_nxeditCost);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_STATIC_INVADJ_QUANTITY, m_nxstaticInvadjQuantity);
	DDX_Control(pDX, IDC_UU_ADJ_TEXT, m_nxstaticUuAdjText);
	DDX_Control(pDX, IDC_UO_ADJ_TEXT, m_nxstaticUoAdjText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvAdj, CNxDialog)
	//{{AFX_MSG_MAP(CInvAdj)
	ON_EN_KILLFOCUS(IDC_COST, OnKillfocusCost)
	ON_EN_KILLFOCUS(IDC_QUANTITY, OnKillfocusQuantity)
	ON_EN_KILLFOCUS(IDC_QUANTITY_UO, OnKillfocusQuantityUo)
	ON_BN_CLICKED(IDC_BTN_AUTO_CALC, OnBtnAutoCalc)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_EDIT_ADJ_CATEGORIES, OnEditAdjCategories)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvAdj message handlers

BOOL CInvAdj::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_okBtn.AutoSet(NXB_OK);
		m_cancelBtn.AutoSet(NXB_CANCEL);
		// (c.haag 2008-05-20 12:23) - PLID 29820 - NxIconify the auto-calculate button
		m_btnAutoCalc.AutoSet(NXB_MODIFY);

		SetDlgItemText(IDC_COST,FormatCurrencyForInterface(COleCurrency(0,0)));
		SetDlgItemInt(IDC_QUANTITY,0);
		try {
			m_location = BindNxDataListCtrl(IDC_LOCATION,false);
			CString str;
			str.Format("Managed = 1 AND Active = 1 AND TypeID = 1 AND ProductLocationInfoT.TrackableStatus = 2 AND ProductLocationInfoT.ProductID = %li",m_product);
			m_location->PutWhereClause(_bstr_t(str));
			m_location->Requery();
		} NxCatchAll("Could Not Bind DataList");

		try {
			_RecordsetPtr rs = CreateRecordset("SELECT Name, UseUU, Conversion, SerialPerUO FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID WHERE ServiceT.ID = %i", m_product);
			if(!rs->eof) {
				SetDlgItemVar(IDC_NAME, rs->Fields->Item["Name"]->Value, true, true);
				m_bUseUU = AdoFldBool(rs, "UseUU",FALSE);

				//(e.lally 2008-07-01) PLID 24534 - Even though data is saved for these options, only set them if the product uses UU.
				if(m_bUseUU != FALSE){
					m_UUConversion = AdoFldLong(rs, "Conversion",1);
					m_bEnterSingleSerialPerUO = AdoFldBool(rs, "SerialPerUO",FALSE);
				}
				else{
					m_UUConversion = 1;
					m_bEnterSingleSerialPerUO = FALSE;
				}
			}
			rs->Close();
		} NxCatchAll("Could not load item name");

		//DRT 5/8/2006 - PLID 20484 - I also made the UO boxes (right side), and the labels off by default, 
		//	and shown if needed here.  About 99% of all items across all clients are NOT using the UU/UO classification, 
		//	so it makes the most sense to avoid this most of the time.
		if(m_bUseUU) {
			GetDlgItem(IDC_UU_ADJ_TEXT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_UO_ADJ_TEXT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STOCK_UO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_QUANTITY_UO)->ShowWindow(SW_SHOW);		
		}

		// (a.walling 2008-02-20 15:45) - PLID 28879 - Put some more info on the label
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if (g_pLicense->HasCandAModule(CLicense::cflrSilent)) {
			GetDlgItem(IDC_STATIC_INVADJ_QUANTITY)->SetWindowText("Quantity (Purchased)");
		}

		//DRT 5/8/2006 - PLID 20484 - I set this down here just so the thread might have a little time to get to this row.  There is no functional difference
		//	to the user where this line goes after the requery.
		m_location->SetSelByColumn(0, m_nLocationID);

		//DRT 5/8/2006 - PLID 20484 - Set a timer to load the quantity values.  This is a very slow operation, and doing it this way gives us the opportunity
		//	to display the dialog while the slowest part of the loading is going on.  I cannot find any way to increase the speed
		//	of this loading.
		SetTimer(IDT_INIT_LOAD_QTY, 25, NULL);

		m_pAdjCategories = BindNxDataList2Ctrl(IDC_ADJ_CATEGORIES, false);
		//TES 6/27/2008 - PLID 30523 - They may not have permission for all categories, so load just the ones they
		// have permission for.
		LoadCategories();

		if(m_pAdjCategories->GetRowCount() == 0) {
			//TES 6/27/2008 - PLID 30523 - They don't have permission for any categories (not even "<None>"), so there's
			// no point in continuing.
			MsgBox("You do not have permission to create adjustments for any active category.  Please contact your office manager for assistance.");
			OnCancel();
		}
		else {
			//TES 6/24/2008 - PLID 26142 - Go ahead and select the "<None>" row, if we've got it, otherwise leave it blank.
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_pAdjCategories->TrySetSelByColumn_Deprecated(0, (long)-1);
		}

	} NxCatchAll("Error in OnInitDialog()");

	return TRUE;
}

void CInvAdj::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent) {
	case IDT_INIT_LOAD_QTY:
		//DRT 5/8/2006 - PLID 20484 - Load this in a timer a few milliseconds after the InitDialog.  This allows the dialog
		//	to properly display to the user, because this bit of code can take 500 odd milliseconds to run, looking
		//	like quite a delay.  This should only be called from the OnInitDialog.
		KillTimer(IDT_INIT_LOAD_QTY);


		//This will calculate the quantity and put it into the InStock box.
		RefreshOnHand();

		CString strInStock;
		GetDlgItemText(IDC_STOCK, strInStock);
		double dblStock = atof(strInStock);
		//JMJ - 6/23/2003 - Negative adjustments are not allowed, and having a negative amount in stock
		//is absurd, so if you have a negative amount then the default amount to adjust to should be zero.
		if(dblStock < 0.0)
			dblStock = 0.0;
		strInStock.Format("%g",dblStock);
		SetDlgItemText(IDC_QUANTITY, strInStock);

		//if we want to use UU/UO
		if(m_bUseUU) {
			//calculate the UO values

			double dblStockUO = dblStock / m_UUConversion;
			CString strStockUO;
			strStockUO.Format("%g",dblStockUO);

			SetDlgItemText(IDC_STOCK_UO, strStockUO);
			SetDlgItemText(IDC_QUANTITY_UO, strStockUO);
		}
		break;
	}

	CNxDialog::OnTimer(nIDEvent);
}

int CInvAdj::DoModal(long product, long nLocationID)
{
	m_product = product;
	m_nLocationID = nLocationID;
	return CNxDialog::DoModal();
}

void CInvAdj::OnOK() 
{
	if(m_location->CurSel == -1) {
		AfxMessageBox("You must have a location selected to make an adjustment.");
		return;
	}

	int loc = VarLong(m_location->GetValue(m_location->CurSel, 0));
	double quantity;
	CString note, cost;

	CString strNewAmtInStock;
	GetDlgItemText(IDC_QUANTITY, strNewAmtInStock);
	double dblNewAmtInStock = atof(strNewAmtInStock);

	CString strOldAmtInStock;
	GetDlgItemText(IDC_STOCK, strOldAmtInStock);
	double dblOldAmtInStock = atof(strOldAmtInStock);

	//JMJ - 06/23/2003 - Keep in mind that an adjustment of zero is perfectly legal and not uncommon,
	//but anything less than zero is absurd and as of right now we have no evidence of why we should support it.
	if(dblNewAmtInStock < 0.0) {
		AfxMessageBox("You cannot adjust an item to be less than 0 in stock.");
		return;
	}

	quantity = dblNewAmtInStock - dblOldAmtInStock;

	double dblProdItemsQty = quantity;

	//if the amt. on hand was negative, then we only want to enter in serial numbers
	//for what is physicaly in stock, not the difference of the adjustment
	if(dblOldAmtInStock < 0.0)
		dblProdItemsQty = dblNewAmtInStock;

	GetDlgItemText(IDC_COST, cost);
	GetDlgItemText(IDC_NOTES, note);

	try {

		//TES 6/24/2008 - PLID 26142 - Get the category they've selected.
		CString strCategoryID = "NULL";
		long nCategoryID = -1;
		if(m_pAdjCategories->CurSel != NULL) {
			nCategoryID = VarLong(m_pAdjCategories->CurSel->GetValue(0));
		}
		if(nCategoryID != -1) {
			strCategoryID.Format("%li", nCategoryID);
		}

		//TES 6/27/2008 - PLID 30523 - If they don't have "<None>" in the list, that means they don't have permission to
		// save an adjustment without a category.
		if(m_pAdjCategories->FindByColumn(0, (long)-1, NULL, VARIANT_FALSE) == NULL) {
			if(nCategoryID == -1) {
				MsgBox("You do not have permission to save an Inventory Adjustment with no category.  Please select a category, "
					"or contact your office manager for assistance.");
				return;
			}
		}
		//TES 6/27/2008 - PLID 30523 - Now, check their permissions normally (this is just to prompt for their password, if
		// it's required).
		if(!CheckCurrentUserPermissions(bioIndivAdjustmentCategories, sptCreate, TRUE, nCategoryID)) {
			return;
		}

		// (j.jones 2008-06-02 15:46) - PLID 28076 - AdjustProductItems will now fill an array with IDs
		// of products that need adjusted, and actually adjust them off later
		CArray<long, long> aryProductItemIDsToRemove;

		long nLocationID = -1;

		if(m_location->GetCurSel()==-1) {
			nLocationID = GetCurrentLocationID();
		}
		else {
			nLocationID = m_location->GetValue(m_location->GetCurSel(),0).lVal;
		}
		
		EnsureRemoteData();

		CString strSqlBatch = BeginSqlBatch();
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nProductAdjustmentID INT");

		// (j.jones 2009-03-09 12:24) - PLID 33096 - move the adjustment saving statements prior to calling AdjustProductItems,
		// so if we do actually execute this batch, we will create the adjustment in data first
		AddStatementToSqlBatch(strSqlBatch, "SET @nProductAdjustmentID = (SELECT COALESCE(MAX(ID),0) + 1 FROM ProductAdjustmentsT)");
		//TES 6/24/2008 - PLID 26142 - Save the category they've selected.
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ProductAdjustmentsT "
			"(ID, ProductID, Date, Login, Quantity, Amount, LocationID, Notes, ProductAdjustmentCategoryID) "
			"SELECT @nProductAdjustmentID, %li, '%s', %li, %g, Convert(money,'%s'), %li, '%s', %s",
			m_product, FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), 
			GetCurrentUserID(), quantity, _Q(FormatCurrencyForSql(ParseCurrencyFromInterface(cost))), loc, _Q(note),
			strCategoryID);

		//check to see if this item has serialized items
		// (j.jones 2009-01-15 15:42) - PLID 32749 - moved AdjustProductItems to InvUtils,
		// and changed this logic to add the results into our sql batch
		// (j.jones 2009-03-09 12:21) - PLID 33096 - added strCreatingProductAdjustmentID as a parameter
		if(!InvUtils::AdjustProductItems(m_product, nLocationID, dblProdItemsQty, aryProductItemIDsToRemove, TRUE, strSqlBatch, "@nProductAdjustmentID")) {
			return;
		}

		// (j.jones 2008-06-02 15:50) - PLID 28076 - mark product items, and track the adjustment ID, as needed
		if(aryProductItemIDsToRemove.GetSize() > 0) {
			CString strIDs;
			for(int i=0;i<aryProductItemIDsToRemove.GetSize();i++) {
				if(!strIDs.IsEmpty()) {
					strIDs += ",";
				}
				strIDs += AsString((long)(aryProductItemIDsToRemove.GetAt(i)));
			}

			//TES 6/23/2008 - PLID 26152 - Now that this item has been adjusted out of existence, it shouldn't be flagged
			// To Be Returned any more.
			AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductItemsT SET Deleted = 1, AdjustmentID = @nProductAdjustmentID, "
				"ToBeReturned = 0 WHERE ID IN (%s)", strIDs);
		}
		
		ExecuteSqlBatch(strSqlBatch);

		// (c.haag 2008-02-07 13:17) - PLID 28853 - Renamed from ChargeInventoryQuantity to EnsureInventoryTodoAlarms
		// because that's a closer description to what it actually does. Also removed unused quantity parameter.
		// (j.jones 2008-09-16 09:28) - PLID 31380 - EnsureInventoryTodoAlarms now supports multiple products,
		// though in this particular case, it really is only one product
		CArray<long, long> aryProductIDs;
		aryProductIDs.Add(m_product);
		//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
		InvUtils::EnsureInventoryTodoAlarms(aryProductIDs, loc, false);

		//auditing
		CString strProduct;
		GetDlgItemText(IDC_NAME, strProduct);
		CString strNew;
		strNew.Format("%g", quantity);
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, strProduct, nAuditID, aeiProductAdjustment, m_product, "", strNew, aepMedium, aetChanged);
		CDialog::OnOK();
	}NxCatchAll("Could not save item adjustment");
}

void CInvAdj::OnKillfocusCost() 
{
	COleCurrency cost;
	CString val;

	GetDlgItemText(IDC_COST, val);
	cost = ParseCurrencyFromInterface(val);
	//TES 11/7/2007 - PLID 27979 - VS2008 - VS 2008 doesn't like this syntax, and there's no need for it.
	//if (cost.GetStatus() != COleCurrency::CurrencyStatus::valid)
	if (cost.GetStatus() != COleCurrency::valid)
		cost = COleCurrency(0,0);
	
	val = FormatCurrencyForInterface(cost);
	SetDlgItemText(IDC_COST, val);
}

void CInvAdj::OnKillfocusQuantity() 
{
	CString strQty;
	GetDlgItemText(IDC_QUANTITY, strQty);
	double dblQty = atof(strQty);
	strQty.Format("%g",dblQty);
	SetDlgItemText(IDC_QUANTITY, strQty);

	if(m_bUseUU) {
		double dblQtyUO = dblQty / m_UUConversion;
		CString strQtyUO;
		strQtyUO.Format("%g",dblQtyUO);

		SetDlgItemText(IDC_QUANTITY_UO, strQtyUO);
	}
}

void CInvAdj::OnKillfocusQuantityUo() 
{
	//note, it is unlikely if not logically possible
	//that a user will type in a decimal value here, but we must allow it

	//first retrieve and format the UO Qty
	CString strQtyUO;
	GetDlgItemText(IDC_QUANTITY_UO,strQtyUO);	
	double dblQtyUO = atof(strQtyUO);
	strQtyUO.Format("%g",dblQtyUO);	
	SetDlgItemText(IDC_QUANTITY_UO, strQtyUO);

	//now set the UU Qty
	/*
	JMJ 3/12/2004 - I allowed decimal amounts in the UU field
	long nQtyUU = (long)(m_UUConversion * dblQtyUO);
	SetDlgItemInt(IDC_QUANTITY,nQtyUU);
	*/

	double dblQtyUU = m_UUConversion * dblQtyUO;
	CString strQtyUU;
	strQtyUU.Format("%g",dblQtyUU);	
	SetDlgItemText(IDC_QUANTITY, strQtyUU);


	//NOW we have to recalculate the UO,
	//because one could have typed in a number that did not come to a
	//whole UU, etc.
	OnKillfocusQuantity();
}

void CInvAdj::OnCancel() 
{
	CDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CInvAdj, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvAdj)
	ON_EVENT(CInvAdj, IDC_LOCATION, 16 /* SelChosen */, OnSelChosenLocation, VTS_I4)
	ON_EVENT(CInvAdj, IDC_ADJ_CATEGORIES, 16 /* SelChosen */, OnSelChosenAdjCategories, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInvAdj::OnSelChosenLocation(long nRow) 
{
	try {
		if(nRow != -1) {
			//Only do anything if the location has changed.
			int nSelLoc = VarLong(m_location->GetValue(nRow, 0));
			if(nSelLoc != m_nLocationID) {
				m_nLocationID = nSelLoc;
				//Recalculate the "On Hand" amount
				RefreshOnHand();
				CString str;
				GetDlgItemText(IDC_STOCK,str);
				SetDlgItemText(IDC_QUANTITY,str);
				((CNxEdit*)GetDlgItem(IDC_QUANTITY))->GetFocus();
				((CNxEdit*)GetDlgItem(IDC_QUANTITY))->SetSel(0, -1);
			}
		}
	}NxCatchAll("Error in CInvAdj::OnSelChosenLocation()");
}


void CInvAdj::RefreshOnHand()
{
	try {
		
		// (j.jones 2008-02-28 10:40) - PLID 28080 - converted item_sql into GetInventoryItemSql()
		// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
		_RecordsetPtr rsNumInStock = CreateParamRecordset(InvUtils::GetInventoryItemSql(m_product, m_nLocationID));
		// (a.walling 2008-02-20 14:56) - PLID 28879 - Do not include consignment items
		double dblNumInStock = AdoFldDouble(rsNumInStock, "ActualPurchasedInv");
		CString strNumInStock;
		strNumInStock.Format("%g",dblNumInStock);

		SetDlgItemText(IDC_STOCK, strNumInStock);
	}NxCatchAll("Error in CInvAdj::RefreshOnHand()");
}

// (j.jones 2008-06-02 15:46) - PLID 28076 - AdjustProductItems will now fill an array with IDs
// of products that need adjusted, and actually adjust them off later
// (j.jones 2009-01-15 15:40) - PLID 32749 - moved to InvUtils
/*
BOOL CInvAdj::AdjustProductItems(long ProductID, double dblQuantity, CArray<long, long> &aryProductItemIDsToRemove, BOOL bCanUseUOAdjustment) {

	try {

		// (j.jones 2006-05-11 10:25) - this only adjusts by Unit Of Usage,
		// only Orders can enter information by UO
		//(e.lally 2008-07-01) PLID 24534 - Add support for adjusting by Unit of Order.

		long LocationID = -1;

		if(m_location->GetCurSel()==-1)
			LocationID = GetCurrentLocationID();
		else 
			LocationID = m_location->GetValue(m_location->GetCurSel(),0).lVal;

		// (j.jones 2007-11-21 16:40) - PLID 28037 - ensure we account for allocated items
		BOOL bHasItemsWSerial = ReturnsRecords("SELECT ID FROM ProductItemsT WHERE SerialNum Is Not Null "
			"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"			    WHERE (Status = %li OR Status = %li) "
			"				AND ProductItemID Is Not Null) "
			"AND Deleted = 0 AND ProductID = %li "
			"AND (ProductItemsT.LocationID = %li OR ProductItemsT.LocationID Is Null)",
			InvUtils::iadsActive, InvUtils::iadsUsed, ProductID, LocationID);

		BOOL bHasItemsWExpDate = ReturnsRecords("SELECT ID FROM ProductItemsT WHERE ExpDate Is Not Null "
			"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"			    WHERE (Status = %li OR Status = %li) "
			"				AND ProductItemID Is Not Null) "
			"AND Deleted = 0 AND ProductID = %li AND (ProductItemsT.LocationID = %li OR ProductItemsT.LocationID Is Null)",
			InvUtils::iadsActive, InvUtils::iadsUsed, ProductID, LocationID);
		
		BOOL bHasSerial = FALSE, bHasExpDate = FALSE;

		_RecordsetPtr rs = CreateRecordset("SELECT HasSerialNum, HasExpDate FROM ProductT WHERE ID = %li AND (HasSerialNum = 1 OR HasExpDate = 1)",ProductID);
		if(!rs->eof) {		

			bHasSerial = AdoFldBool(rs, "HasSerialNum",FALSE);
			bHasExpDate = AdoFldBool(rs, "HasExpDate",FALSE);
		}
		rs->Close();

		if(!bHasSerial && !bHasExpDate && !bHasItemsWSerial && !bHasItemsWExpDate)
			//if no applicable products, return true and move on
			return TRUE;

		if((long)dblQuantity != dblQuantity) {
			AfxMessageBox("This product is being tracked by either a serial number or expiration date,\n"
						"and requires that it is adjusted in increments of 1.\n"
						"Please enter a whole number for the quantity to adjust.");
			return FALSE;
		}

		CProductItemsDlg& dlg = GetMainFrame()->GetProductItemsDlg();

		if(dblQuantity > 0.0) {
			//if the product does not currently require this info, return - we don't need to add
			if(!bHasSerial && !bHasExpDate)
				return TRUE;
			dlg.m_EntryType = PI_ENTER_DATA;
			//DRT 11/15/2007 - PLID 28008 - All ENTER_DATA types must not allow change of qty
			dlg.m_bDisallowQtyChange = TRUE;
			dlg.m_NewItemCount = (long)dblQuantity;
		}
		else if(dblQuantity < 0.0) {
			dlg.m_EntryType = PI_SELECT_DATA;
			dlg.m_CountOfItemsNeeded = -(long)dblQuantity;
			bCanUseUOAdjustment = FALSE;
		}
		else{
			return TRUE; //if they are making a adjustment of 0, get out!
		}

		//(e.lally 2008-07-01) PLID 24534 - check if we can use UO as an option, then prompt the user to see if they want to use that option
		if(dblQuantity > 0.0){
			if(bCanUseUOAdjustment != FALSE){
				//we are able to adjust based on Unit of Order, ask the user.
				UINT response = AfxMessageBox("Do you want to enter items based on Unit of Order (UO)?\n"
					"If 'No', the items in the list will be by Unit of Usage (UU).", MB_YESNO);
				if(response == IDNO){
					bCanUseUOAdjustment = FALSE;
				}
			}
			//Check if we're using UU and UO, the SerialPerUO option is on, but our add adjusted amount is not a whole UO
			else if(m_bUseUU != FALSE && m_bEnterSingleSerialPerUO != FALSE && (long)dblQuantity % (long)m_UUConversion != 0){
				//Prompt to user to see if they want to cancel to update the value to be a full UO.
				UINT response = AfxMessageBox("The adjusted difference in Unit of Usage (UU) does not have a whole number for Unit of Order (UO). If you continue, "
					"the items in the list will be by Unit of Usage.", MB_OKCANCEL);
				if(response == IDCANCEL){
					return FALSE;
				}
			}
		}

		dlg.m_bUseSerial = bHasSerial;
		dlg.m_bUseExpDate = bHasExpDate;
		dlg.m_ProductID = ProductID;
		dlg.m_nLocationID = LocationID;
		//TES 6/18/2008 - PLID 29578 - Changed OrderID to OrderDetailID.
		dlg.m_nOrderDetailID = -1;
		dlg.m_bDisallowQtyChange = TRUE;
		dlg.m_bAllowQtyGrow = FALSE;
		dlg.m_bIsAdjustment = TRUE;		
		dlg.m_bDisallowLocationChange = TRUE; // (c.haag 2008-06-25 12:12) - PLID 28438 - We only allow adjusting at LocationID
		dlg.m_bUseUU = m_bUseUU;
		if(m_bEnterSingleSerialPerUO != FALSE && bCanUseUOAdjustment !=FALSE){
			dlg.m_bSerialPerUO = TRUE;
		}
		else{
			dlg.m_bSerialPerUO = FALSE;
		}
		dlg.m_nConversion = m_UUConversion;

		if(IDCANCEL == dlg.DoModal()) {
			//JMJ - 6/24/2003 - Even if a product does not require this information, WE require them to use these items up first.
			//if they have a problem with that, they will have to adjust off those items and re-add them.
			//That sounds cocky, but Meikin and I discussed that this is the best way. The bill follows the same logic.
			AfxMessageBox("The quantity of this product cannot be changed without appropriately modifying this required information.\n"
				"You will not be permitted to save this adjustment without updating this information.");
			return FALSE;
		}
		else {
			//we only need to do this if it is PI_SELECT_DATA
			if(dlg.m_EntryType == PI_SELECT_DATA) {
				for(int i=0;i<dlg.m_adwProductItemIDs.GetSize();i++) {								
					long ProductItemID = (long)dlg.m_adwProductItemIDs.GetAt(i);
					//DRT 10/2/03 - PLID 9467 - Don't remove the row, just mark it deleted now.
					// (j.jones 2008-06-02 15:47) - PLID 28076 - we no longer delete in this function,
					// instead fill our array with the IDs that need removed
					aryProductItemIDsToRemove.Add(ProductItemID);					
				}
			}
		}

		return TRUE;

	}NxCatchAll("Error in AdjustProductItems");

	AfxMessageBox("The quantity of this product cannot be changed without appropriately modifying this required information.\n"
				"You will not be permitted to save this adjustment without updating this information.");
	return FALSE;
}
*/

void CInvAdj::OnBtnAutoCalc() 
{
	try {

		CString strNewAmtInStock;
		GetDlgItemText(IDC_QUANTITY, strNewAmtInStock);
		double dblNewAmtInStock = atof(strNewAmtInStock);

		CString strOldAmtInStock;
		GetDlgItemText(IDC_STOCK, strOldAmtInStock);
		double dblOldAmtInStock = atof(strOldAmtInStock);

		COleCurrency cyLastCost = COleCurrency(0,0);

		_RecordsetPtr rs = CreateRecordset("SELECT LastCostPerUU AS LastCost "
			"FROM ProductT WHERE ID = %li",m_product);
		if(!rs->eof) {
			cyLastCost = AdoFldCurrency(rs, "LastCost",COleCurrency(0,0));
		}
		rs->Close();

		double dblAdjAmount = dblNewAmtInStock - dblOldAmtInStock;

		COleCurrency cyAdjCost = -CalculateAmtQuantity(cyLastCost,dblAdjAmount);

		SetDlgItemText(IDC_COST,FormatCurrencyForInterface(cyAdjCost));

	}NxCatchAll("Error calculating adjustment cost.");
}

void CInvAdj::OnEditAdjCategories() 
{
	try {
		//TES 6/24/2008 - PLID 26142 - Check which ID they have selected.
		long nCurID = -1;
		if(m_pAdjCategories->CurSel != NULL) {
			nCurID = VarLong(m_pAdjCategories->CurSel->GetValue(0));
		}

		//TES 6/24/2008 - PLID 26142 - Popup the configuration dialog.
		CEditProductAdjustmentCategoriesDlg dlg(this);
		dlg.DoModal();

		//TES 6/24/2008 - PLID 26142 - Refresh the list, and try to restore their previous selection.
		LoadCategories();
		if(m_pAdjCategories->GetRowCount() == 0) {
			//TES 6/27/2008 - PLID 30523 - They no longer have any permission to any categories (including "<None>").
			// However, unlike in OnInitDialog(), we will not immediately close the dialog, because a.) they may have entered
			// some notes or something, and we want to at least let them copy those to Notepad or something, and b.) they
			// may have just accidentally inactivated the wrong category, so they could still go in and re-activate it.
			MsgBox("There are no longer any adjustment categories which you have permission to edit.  You will not be able to save this adjustment.");
		}
		else {
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_pAdjCategories->TrySetSelByColumn_Deprecated(0, nCurID);
		}
	}NxCatchAll("Error in CInvAdj::OnEditAdjCategories()");
}

void CInvAdj::OnSelChosenAdjCategories(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//TES 6/24/2008 - PLID 26142 - Go ahead and select the "<None>" row, if we've got it.
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_pAdjCategories->TrySetSelByColumn_Deprecated(0, (long)-1);
		}
	}NxCatchAll("Error in CInvAdj::OnSelChosenAdjCategories()");
}

void CInvAdj::LoadCategories()
{
	//TES 6/27/2008 - PLID 30523 - Clear out the list.
	m_pAdjCategories->Clear();

	//TES 6/27/2008 - PLID 30523 - Now go through each active category (plus "<None>"), and add it if they have permission
	// to create adjustments with that category.
	_RecordsetPtr rsCategories = CreateRecordset("SELECT -1 AS ID, '<None>' AS Name "
		"UNION SELECT ID, Name FROM ProductAdjustmentCategoriesT WHERE ID > 0 AND Inactive = 0 ORDER BY Name");
	while(!rsCategories->eof) {
		long nID = AdoFldLong(rsCategories, "ID");
		if(GetCurrentUserPermissions(bioIndivAdjustmentCategories, TRUE, nID) & SPT____C_______ANDPASS) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAdjCategories->GetNewRow();
			pRow->PutValue(0, nID);
			pRow->PutValue(1, rsCategories->Fields->Item["Name"]->Value);
			m_pAdjCategories->AddRowAtEnd(pRow, NULL);
		}
		rsCategories->MoveNext();
	}
}

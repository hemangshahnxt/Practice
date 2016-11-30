// SelectLinkedProductsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "inventoryrc.h"
#include "SelectLinkedProductsDlg.h"
#include "barcode.h"
#include "InvUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TES 7/16/2008 - PLID 27983 - Created
/////////////////////////////////////////////////////////////////////////////
// CSelectLinkedProductsDlg dialog


CSelectLinkedProductsDlg::CSelectLinkedProductsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectLinkedProductsDlg::IDD, pParent)
{
	m_nLocationID = -1;
	m_bIsCaseHistory = false;
	m_bIsPackage = FALSE;
	//{{AFX_DATA_INIT(CSelectLinkedProductsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectLinkedProductsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectLinkedProductsDlg)
	DDX_Control(pDX, IDC_SELECT_LINKED_PRODUCTS_TITLE, m_nxsTitle);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	DDX_Control(pDX, IDC_UNBILL_PRODUCT, m_nxbUnbillProduct);
	DDX_Control(pDX, IDC_SELECT_LINKED_PRODUCTS_CAPTION, m_nxsSelectLinkedProductsCaption);
	DDX_Control(pDX, IDC_BILL_PRODUCT, m_nxbBillProduct);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectLinkedProductsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectLinkedProductsDlg)
	ON_BN_CLICKED(IDC_BILL_PRODUCT, OnBillProduct)
	ON_BN_CLICKED(IDC_UNBILL_PRODUCT, OnUnbillProduct)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectLinkedProductsDlg message handlers

BEGIN_EVENTSINK_MAP(CSelectLinkedProductsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSelectLinkedProductsDlg)
	ON_EVENT(CSelectLinkedProductsDlg, IDC_LINKABLE_PRODUCTS, 2 /* SelChanged */, OnSelChangedLinkableProducts, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CSelectLinkedProductsDlg, IDC_LINKABLE_PRODUCTS, 3 /* DblClickCell */, OnDblClickCellLinkableProducts, VTS_DISPATCH VTS_I2)
	ON_EVENT(CSelectLinkedProductsDlg, IDC_PRODUCTS_TO_BILL, 2 /* SelChanged */, OnSelChangedProductsToBill, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CSelectLinkedProductsDlg, IDC_PRODUCTS_TO_BILL, 3 /* DblClickCell */, OnDblClickCellProductsToBill, VTS_DISPATCH VTS_I2)
	ON_EVENT(CSelectLinkedProductsDlg, IDC_PRODUCTS_TO_BILL, 8 /* EditingStarting */, OnEditingStartingProductsToBill, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CSelectLinkedProductsDlg, IDC_PRODUCTS_TO_BILL, 9 /* EditingFinishing */, OnEditingFinishingProductsToBill, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CSelectLinkedProductsDlg, IDC_PRODUCTS_TO_BILL, 10 /* EditingFinished */, OnEditingFinishedProductsToBill, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CSelectLinkedProductsDlg, IDC_LINKABLE_PRODUCTS, 18 /* RequeryFinished */, OnRequeryFinishedLinkableProducts, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

extern CPracticeApp theApp;
BOOL CSelectLinkedProductsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		//TES 7/16/2008 - PLID 27983 - Initialize our controls.
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);
		m_nxbBillProduct.AutoSet(NXB_DOWN);
		m_nxbUnbillProduct.AutoSet(NXB_UP);
		
		//TES 7/16/2008 - PLID 27983 - If the name of our CPT Code wasn't passed in, pull it from data.
		if(m_strCptName.IsEmpty()) m_strCptName = VarString(GetTableField("ServiceT", "Name", "ID", m_nCptID));
		m_nxsTitle.SetWindowText(ConvertToControlText(m_strCptName));
		m_nxsTitle.SetFont(&theApp.m_subtitleFont);

		//TES 7/16/2008 - PLID 27983 - Now fill in the explanatory caption at the bottom of the screen.
		CString strCaption;
		//TES 7/16/2008 - PLID 27983 - Change the caption if we're on a case history.
		if(m_bIsCaseHistory) {
			strCaption.Format("Your case history has the Service Code '%s', which is linked to products.  Please select which "
				"products you would like to use in place of the '%s' Service Code.", m_strCptName, m_strCptName);
		}
		else {
			strCaption.Format("You are billing the Service Code '%s', which is linked to products.  Please select which products "
				"you would like to bill in place of the '%s' Service Code.", m_strCptName, m_strCptName);
		}
		m_nxsSelectLinkedProductsCaption.SetWindowText(ConvertToControlText(strCaption));

		//TES 7/16/2008 - PLID 27983 - We want to get barcode scans.
		if(GetMainFrame()) {
			if(!GetMainFrame()->RegisterForBarcodeScan(this))
				MsgBox("Error registering for barcode scans.  You may not be able to scan.");
		}
		m_pAvailableProducts = BindNxDataList2Ctrl(IDC_LINKABLE_PRODUCTS, false);
		m_pProductsToBill = BindNxDataList2Ctrl(IDC_PRODUCTS_TO_BILL, false);

		CString strLocationFilter;
		if(m_bIsCaseHistory) {
			//TES 7/16/2008 - PLID 27983 - If we're on a case history, then products from all locations are acceptable.
			strLocationFilter = "(1=1)";
		}
		else {
			//TES 7/16/2008 - PLID 27983 - Only show products that are billable at the location we were given.
			strLocationFilter.Format("ProductT.ID IN (SELECT ProductID FROM ProductLocationInfoT WHERE LocationID = %li AND Billable = 1)",
				m_nLocationID);
		}
		//TES 7/16/2008 - PLID 27983 - Now, requery our list of products.
		CString strWhere;
		strWhere.Format("ProductT.ID IN (SELECT ProductID FROM ServiceToProductLinkT WHERE CptID = %li AND ProductID Is Not Null) "
			"AND %s",
			m_nCptID, strLocationFilter);
		m_pAvailableProducts->WhereClause = _bstr_t(strWhere);
		m_pAvailableProducts->Requery();

		//TES 7/16/2008 - PLID 27983 - Set the status of the up and down buttons.
		EnableButtons();

	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectLinkedProductsDlg::OnBillProduct() 
{
	try {
		BillProduct();
	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnBillProduct()");
}

void CSelectLinkedProductsDlg::OnSelChangedLinkableProducts(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		EnableButtons();
	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnSelChangedLinkableProducts()");
}

void CSelectLinkedProductsDlg::OnDblClickCellLinkableProducts(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		BillProduct();
	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnDblClickCellLinkableProducts()");
}

void CSelectLinkedProductsDlg::OnSelChangedProductsToBill(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		EnableButtons();
	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnSelChangedProductsToBill()");
}

void CSelectLinkedProductsDlg::OnDblClickCellProductsToBill(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		UnbillProduct();
	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnDblClickCellProductsToBill()");
}

void CSelectLinkedProductsDlg::OnUnbillProduct() 
{
	try {
		UnbillProduct();
	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnUnbillProduct()");
}

void CSelectLinkedProductsDlg::OnCancel() 
{
	try {
		//TES 7/16/2008 - PLID 27983 - Warn them.
		//TES 7/16/2008 - PLID 27983 - Change the warning if we're on a case history.
		CString strAction = m_bIsCaseHistory ? "removed" : "ignored";
		CString strRecord = m_bIsCaseHistory ? "case history" : "bill";
		if(IDYES != MsgBox(MB_YESNO, "Are you wish to cancel?  The service code will be %s, and no products will be "
			"added to the current %s in its place.", strAction, strRecord)) {
			return;
		}
		
		//TES 7/16/2008 - PLID 27983 - Clear out our list so that our caller doesn't mistakenly try to add them.
		m_arProductsToBill.RemoveAll();

		//TES 7/16/2008 - PLID 27983 - We don't want barcode scans any more.
		if(GetMainFrame()) {
			if(!GetMainFrame()->UnregisterForBarcodeScan(this))
				MsgBox("Error unregistering for barcode scans.");
		}
	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnCancel()");

	CNxDialog::OnCancel();
}

void CSelectLinkedProductsDlg::OnOK() 
{
	try {
		//TES 7/16/2008 - PLID 27983 - Figure out the total quantity that they've selected, either by scanning serial numbers
		// or adding products/adjusting quantities manually.
		long nQtySelected = 0;
		for(int i = 0; i < m_arProductsToBill.GetSize(); i++) {
			ProductToBill ptb = m_arProductsToBill[i];
			int nProductItems = ptb.arProductItemIDs.GetSize();
			if(nProductItems == 0) {
				nQtySelected += ptb.nQty;
			}
			else {
				nQtySelected += nProductItems;
			}
		}

		//TES 7/16/2008 - PLID 27983 - If they didn't select any, that's weird, and we should warn them.
		if(nQtySelected == 0) {
			//TES 7/16/2008 - PLID 27983 - Change the warning if we're on a case history.
			CString strAction = m_bIsCaseHistory ? "removed" : "ignored";
			CString strRecord = m_bIsCaseHistory ? "case history" : "bill";
			if(IDYES != MsgBox(MB_YESNO, "You have not selected any products, are you sure you wish to do this?  The service code will be %s, and no products will be "
				"added to the current %s in its place.", strAction, strRecord)) {
				return;
			}
		}
		//TES 7/16/2008 - PLID 27983 - If they selected a different number than we expected, that's a little weird (though
		// totally acceptable), and we should warn them.
		else if(nQtySelected != m_dDefaultQty) {

			// (j.jones 2010-11-24 08:57) - PLID 41549 - if a package, they can only select the exact quantity
			if(m_bIsPackage) {
				CString str;
				str.Format("The package charge you are billing requires a Quantity of %g, but you have selected a total of "
					"%li product%s to bill. Please select %g product%s to continue.", m_dDefaultQty, nQtySelected, nQtySelected > 1 ? "s" : "", m_dDefaultQty, m_dDefaultQty > 1 ? "s" : "");
				AfxMessageBox(str);
				return;
			}

			//TES 7/16/2008 - PLID 27983 - Change the warning if we're on a case history.
			if(m_bIsCaseHistory) {
				if(IDYES != MsgBox(MB_YESNO, "The line you are replacing on this Case History has a Quantity of %g, but you have selected a total of "
					"%li products to use.  Are you sure you wish to continue?", m_dDefaultQty, nQtySelected)) {
					return;
				}
			}
			else {
				if(IDYES != MsgBox(MB_YESNO, "The charge you are billing has a Quantity of %g, but you have selected a total of "
					"%li products to bill.  Are you sure you wish to continue?", m_dDefaultQty, nQtySelected)) {
					return;
				}
			}
		}

		//TES 7/16/2008 - PLID 27983 - We don't want barcode scans any more
		if(GetMainFrame()) {
			if(!GetMainFrame()->UnregisterForBarcodeScan(this))
				MsgBox("Error unregistering for barcode scans.");
		}

		//TES 7/16/2008 - PLID 27983 - That's it (we've been keeping m_arProductsToBill up-to-date all along, so we don't
		// need to do anything to it now.

	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnOK()");

	CNxDialog::OnOK();
}

void CSelectLinkedProductsDlg::EnableButtons()
{
	//TES 7/16/2008 - PLID 27983 - Enable the buttons if they can do anything.
	m_nxbBillProduct.EnableWindow(m_pAvailableProducts->CurSel != NULL);
	m_nxbUnbillProduct.EnableWindow(m_pProductsToBill->CurSel != NULL);
}

using namespace NXDATALIST2Lib;

void CSelectLinkedProductsDlg::BillProduct(long nProductID /*= -1*/, long nProductItemID /*= -1*/)
{
	//TES 7/16/2008 - PLID 27983 - We may have been given a specific product, if not, just pull the currently selected product.
	if(nProductID == -1) {
		IRowSettingsPtr pCurSel = m_pAvailableProducts->CurSel;
		if(pCurSel == NULL) {
			return;
		}
		nProductID = VarLong(pCurSel->GetValue(0));
	}

	//TES 7/16/2008 - PLID 27983 - Do we already have this in our list?  We need to match on both the product ID, and whether
	// it has ProductItemIDs associated (we don't mix ProductItemIDs with generic quantities).
	bool bFound = false;
	for(int i = 0; i < m_arProductsToBill.GetSize() && !bFound; i++) {
		ProductToBill ptb = m_arProductsToBill[i];
		if(ptb.nProductID == nProductID && ( (nProductItemID == -1 && ptb.arProductItemIDs.GetSize() == 0) || 
			(nProductItemID != -1 && ptb.arProductItemIDs.GetSize() != 0) )) {
			//TES 7/16/2008 - PLID 27983 - Got it!
			bFound = true;
			if(nProductItemID == -1) {
				ptb.nQty += 1;
			}
			else {
				ptb.arProductItemIDs.Add(nProductItemID);
			}
			m_arProductsToBill.SetAt(i, ptb);
		}
	}
	if(!bFound) {
		//TES 7/16/2008 - PLID 27983 - OK, we need to add it.
		ProductToBill ptbNew;
		ptbNew.nProductID = nProductID;
		if(nProductItemID == -1) {
			ptbNew.nQty = 1;
		}
		else {
			ptbNew.arProductItemIDs.Add(nProductItemID);
		}
		m_arProductsToBill.Add(ptbNew);
	}

	//TES 7/16/2008 - PLID 27983 - Now refresh the screen.
	RefreshProductsToBill();
}

enum AvailableProductsColumns {
	apcID = 0,
	apcCategory = 1,
	apcSupplier = 2,
	apcName = 3,
	apcPrice = 4,
	apcBarcode = 5,
};

enum ProductsToBillColumns {
	ptbcID = 0,
	ptbcCategory = 1,
	ptbcSupplier = 2,
	ptbcName = 3,
	ptbcPrice = 4,
	ptbcQty = 5,
	ptbcHasProductItems = 6,
};

void CSelectLinkedProductsDlg::UnbillProduct()
{
	//TES 7/16/2008 - PLID 27983 - Use the currently selected product.
	IRowSettingsPtr pCurSel = m_pProductsToBill->CurSel;
	if(pCurSel == NULL) {
		return;
	}

	//TES 7/16/2008 - PLID 27983 - Find it in our list.
	long nProductID = VarLong(pCurSel->GetValue(ptbcID));
	long nHasProductItems = VarLong(pCurSel->GetValue(ptbcHasProductItems));
	for(int i = 0; i < m_arProductsToBill.GetSize(); i++) {
		ProductToBill ptb = m_arProductsToBill[i];
		if(ptb.nProductID == nProductID && ( (nHasProductItems == 0 && ptb.arProductItemIDs.GetSize() == 0) ||
			(nHasProductItems != 0 && ptb.arProductItemIDs.GetSize() != 0) )) {
			//TES 7/16/2008 - PLID 27983 - Found it!  Remove it from our list.
			m_arProductsToBill.RemoveAt(i);
			//TES 7/16/2008 - PLID 27983 - Now refresh the screen.
			RefreshProductsToBill();
			return;
		}
	}
}

using namespace ADODB;

void CSelectLinkedProductsDlg::RefreshProductsToBill()
{
	//TES 7/16/2008 - PLID 27983 - Don't show the progress of all this.
	m_pProductsToBill->SetRedraw(FALSE);

	//TES 7/16/2008 - PLID 27983 - Remove everything from the list.
	m_pProductsToBill->Clear();

	//TES 7/16/2008 - PLID 27983 - Now go through and re-add everything.
	for(int i = 0; i < m_arProductsToBill.GetSize(); i++) {
		ProductToBill ptb = m_arProductsToBill[i];
		IRowSettingsPtr pNewRow = m_pProductsToBill->GetNewRow();

		long nProductID = ptb.nProductID;
		pNewRow->PutValue(ptbcID, nProductID);

		//TES 7/16/2008 - PLID 27983 - Find the product in our "available products" list, we'll pull some of our information
		// from there.
		IRowSettingsPtr pAvailRow = m_pAvailableProducts->FindByColumn(apcID, nProductID, NULL, VARIANT_FALSE);
		if(pAvailRow == NULL) {
			//TES 7/16/2008 - PLID 27983 - TODO - Any legitimate reason for this?
			ASSERT(FALSE);
			/*_RecordsetPtr rsExistingProductInfo = CreateRecordset("SELECT CategoriesT.Name AS Category, PersonT.Name AS Supplier, "
				"ServiceT.Name, ServiceT.Price "
				"FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID LEFT JOIN CategoriesT ON ServiceT.Category = "
				"CategoriesT.ID LEFT JOIN MultiSupplierT ON ProductT.DefaultMultiSupplierID = MultiSupplierT.ID LEFT JOIN "
				"PersonT ON MultiSupplierT.SupplierID = PersonT.ID "
				"WHERE ProductT.ID = %li", nProductID);
			if(rsExistingProductInfo->eof) {
				AfxThrowNxException("Attempted to load non-existent product ID %li.  The product may have been deleted by another user.", nProductID);
				return;
			}
			pNewRow->PutValue(ptbcCategory, rsExistingProductInfo->Fields->GetItem("Category")->GetValue());
			pNewRow->PutValue(ptbcSupplier, rsExistingProductInfo->Fields->GetItem("Supplier")->GetValue());
			pNewRow->PutValue(ptbcName, rsExistingProductInfo->Fields->GetItem("Name")->GetValue());
			pNewRow->PutValue(ptbcPrice, rsExistingProductInfo->Fields->GetItem("Price")->GetValue());
			rsExistingProductInfo->Close();*/
		}
		else {
			pNewRow->PutValue(ptbcCategory, pAvailRow->GetValue(apcCategory));
			pNewRow->PutValue(ptbcSupplier, pAvailRow->GetValue(apcSupplier));
			pNewRow->PutValue(ptbcName, pAvailRow->GetValue(apcName));
			pNewRow->PutValue(ptbcPrice, pAvailRow->GetValue(apcPrice));
		}
		//TES 7/16/2008 - PLID 27983 - We handle rows that have associated product items and ones that don't differently.
		if(ptb.arProductItemIDs.GetSize()) {
			pNewRow->PutValue(ptbcQty, (long)ptb.arProductItemIDs.GetSize());
			pNewRow->PutValue(ptbcHasProductItems, (long)1);
		}
		else {
			pNewRow->PutValue(ptbcQty, ptb.nQty);
			pNewRow->PutValue(ptbcHasProductItems, (long)0);
		}

		//TES 7/16/2008 - PLID 27983 - Now add the row.
		m_pProductsToBill->AddRowSorted(pNewRow, NULL);
	}
	//TES 7/16/2008 - PLID 27983 - Now show all these changes to the user.
	m_pProductsToBill->SetRedraw(TRUE);
	InvalidateDlgItem(IDC_PRODUCTS_TO_BILL);
}

void CSelectLinkedProductsDlg::OnEditingStartingProductsToBill(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol) {
		case ptbcQty:
		{
			//TES 7/16/2008 - PLID 27983 - If they're using product items, then the quantity is simply the number of product
			// items, so they can't manually edit it.
			long nHasProductItems = VarLong(pRow->GetValue(ptbcHasProductItems));
			if(nHasProductItems == 1) {
				*pbContinue = FALSE;
			}
		}
		break;

		default:
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnEditingStartingProductsToBill()");
}

void CSelectLinkedProductsDlg::OnEditingFinishingProductsToBill(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol) {
		case ptbcQty:
		{
			//TES 7/16/2008 - PLID 27983 - Validate the quantity
			if(pvarNewValue->vt != VT_I4 || VarLong(*pvarNewValue) < 1){
				MsgBox("Please enter a positive quantity");
				*pbContinue = FALSE;
				*pbCommit = FALSE;
			}
		}
		break;

		default:
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnEditingFinishingProductsToBill()");
}

void CSelectLinkedProductsDlg::OnEditingFinishedProductsToBill(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol) {
		case ptbcQty:
		{
			if(bCommit) {
				//TES 7/16/2008 - PLID 27983 - Update the quantity in our list.  We don't need to update the screen, because
				// the only thing that changed is this cell and it's already showing the correct value.
				long nProductID = VarLong(pRow->GetValue(ptbcID));
				for(int i = 0; i < m_arProductsToBill.GetSize(); i++) {
					ProductToBill ptb = m_arProductsToBill[i];
					if(ptb.nProductID == nProductID && ptb.arProductItemIDs.GetSize() == 0) {
						ptb.nQty = VarLong(varNewValue);
						m_arProductsToBill.SetAt(i, ptb);
						return;
					}
				}
			}
		}
		break;

		default:
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnEditingFinishedProductsToBill()");
}

LRESULT CSelectLinkedProductsDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	try {
		//TES 7/16/2008 - PLID 27983 - Get the barcode from teh message.
		_bstr_t bstr = (BSTR)lParam;
		_variant_t vBarcode(bstr);
		CString strBarcode(VarString(vBarcode));
		CWaitCursor wc;

		//TES 7/16/2008 - PLID 27983 - Don't think this should ever happen, but check anyway
		if (strBarcode.IsEmpty()) {
			return 0;
		}

		//TES 7/16/2008 - PLID 27983 - Is this barcode one of our available products?
		//(c.copits 2010-09-30) PLID 40317 - Allow duplicate UPC codes for FramesData certification.
		//IRowSettingsPtr pRow = m_pAvailableProducts->FindByColumn(apcBarcode, vBarcode, NULL, VARIANT_TRUE);
		IRowSettingsPtr pRow = GetBestUPCProduct(vBarcode);
		if (NULL != pRow) {
			EnableButtons();
			//TES 7/16/2008 - PLID 27983 - We found the barcode in the product datalist and auto-selected it, so just simulate clicking the button
			BillProduct();
			return 0;
		}
		else {
			//TES 7/16/2008 - PLID 27983 - OK, we're going to need to call Barcode_CheckExistenceOfSerialNumber().
			// Compile a list of all the ProductItemIDs we've already used, so it doesn't return any of them.
			long nProductID = -1, nProductItemID = -1;
			CString strProductName;
			_variant_t varExpDate;
			CArray<long,long> arSelectedProductItemIDs;
			for(int i = 0; i < m_arProductsToBill.GetSize(); i++) {
				ProductToBill ptb = m_arProductsToBill[i];
				for(int j = 0; j < ptb.arProductItemIDs.GetSize(); j++) {
					arSelectedProductItemIDs.Add(ptb.arProductItemIDs[j]);
				}
			}
			if(InvUtils::Barcode_CheckExistenceOfSerialNumber(strBarcode, m_nLocationID, FALSE, nProductItemID, nProductID, strProductName,
				varExpDate, NULL, TRUE, FALSE, TRUE, TRUE, &arSelectedProductItemIDs)) {
				//TES 7/16/2008 - PLID 27983 - Found it!  Double-check that this is one of our products.
				IRowSettingsPtr pAvailRow = m_pAvailableProducts->FindByColumn(apcID, nProductID, NULL, VARIANT_FALSE);
				if(pAvailRow == NULL) {
					//TES 7/16/2008 - PLID 27983 - Whoops!  That won't work.  Explain to the user.
					MsgBox("The serial number you scanned was valid, but was for a product which is not associated with the "
						"'%s' Service Code.  Please scan a serial number for one of the products in the 'Available Products' list.",
						m_strCptName);
					return 0;
				}
				else {
					//TES 7/16/2008 - PLID 27983 - Got it!  Add it to our list of billed products.
					BillProduct(nProductID, nProductItemID);
				}
			}
			else {
				//They've been told what went wrong, if necessary, so do nothing here.
			}
		}
	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnBarcodeScan()");
	return 0;
}


void CSelectLinkedProductsDlg::OnRequeryFinishedLinkableProducts(short nFlags) 
{
	try {
		if(m_pAvailableProducts->GetRowCount() == 0) {
			//TES 7/16/2008 - PLID 27983 - This should never be possible on a case history, this is only called for CPT Codes
			// with linked products, and case histories don't filter the list at all.
			ASSERT(!m_bIsCaseHistory);
			//TES 7/16/2008 - PLID 27983 - They can't do anything!  Warn the user.
			MsgBox("You attempted to bill the service code '%s', which has linked products.  However, none of the products "
				"linked to '%s' are billable at this bill's location.  The code will not be added to the bill.",
				m_strCptName, m_strCptName);
			//TES 7/16/2008 - PLID 27983 - Clear out our list so that our caller doesn't mistakenly try to add them.
			m_arProductsToBill.RemoveAll();

			//TES 7/16/2008 - PLID 27983 - We don't want barcode scans any more.
			if(GetMainFrame()) {
				if(!GetMainFrame()->UnregisterForBarcodeScan(this))
					MsgBox("Error unregistering for barcode scans.");
			}
			CNxDialog::OnCancel();
		}
	}NxCatchAll("Error in CSelectLinkedProductsDlg::OnRequeryFinishedLinkableProducts()");
}

//(c.copits 2010-09-30) PLID 40317 - Allow duplicate UPC codes for FramesData certification.
// This function will likely be updated to pick the most suitable
// UPC code in response to a barcode scan. Practice now allows multiple
// products to have the same UPC codes. Further, products can share UPC codes
// with service codes (however, service codes cannot share UPC codes).

// Current behavior: returns the first matching UPC code from the linkable products list (IDC_LINKABLE_PRODUCTS)

NXDATALIST2Lib::IRowSettingsPtr CSelectLinkedProductsDlg::GetBestUPCProduct(_variant_t vBarcode)
{
	IRowSettingsPtr pRow;
	
	try {
		pRow = m_pAvailableProducts->FindByColumn(apcBarcode, vBarcode, NULL, VARIANT_TRUE);
	} NxCatchAll(__FUNCTION__);
	
	return pRow;
}
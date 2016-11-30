// ProductsToBeReturnedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "inventoryrc.h"
#include "ProductsToBeReturnedDlg.h"
#include "InvUtils.h"
#include "InvEditReturnDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TES 6/23/2008 - PLID 26152 - Created
/////////////////////////////////////////////////////////////////////////////
// CProductsToBeReturnedDlg dialog


CProductsToBeReturnedDlg::CProductsToBeReturnedDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CProductsToBeReturnedDlg::IDD, pParent)
{
	m_nFirstSupplierID = -1;
	m_nFirstLocationID = -1;
	//{{AFX_DATA_INIT(CProductsToBeReturnedDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CProductsToBeReturnedDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProductsToBeReturnedDlg)
	DDX_Control(pDX, IDC_NEW_RETURN_SELECTED, m_nxbReturnSelected);
	DDX_Control(pDX, IDC_NEW_RETURN_ALL, m_nxbReturnAll);
	DDX_Control(pDX, IDC_CLOSE_PRODUCTS_TO_RETURN, m_nxbClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProductsToBeReturnedDlg, CNxDialog)
	//{{AFX_MSG_MAP(CProductsToBeReturnedDlg)
	ON_BN_CLICKED(IDC_CLOSE_PRODUCTS_TO_RETURN, OnCloseProductsToReturn)
	ON_BN_CLICKED(IDC_NEW_RETURN_ALL, OnNewReturnAll)
	ON_BN_CLICKED(IDC_NEW_RETURN_SELECTED, OnNewReturnSelected)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProductsToBeReturnedDlg message handlers

enum SupplierListColumns {
	slcID = 0,
	slcSupplier = 1,
};

enum LocationListColumns {
	llcID = 0,
	llcLocation = 1,
};

enum ProductListColumns {
	plcID = 0,
	plcSelected = 1,
	plcProductID = 2,
	plcProduct = 3,
	plcSerialNum = 4,
	plcExpDate = 5,
	//DRT 7/23/2008 - PLID 30815 - To default the amount due in the return, we need to track it here
	plcLastCost = 6,
};

using namespace ADODB;
using namespace NXDATALIST2Lib;
BOOL CProductsToBeReturnedDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		m_nxbReturnSelected.AutoSet(NXB_NEW);
		m_nxbReturnAll.AutoSet(NXB_NEW);
		m_nxbClose.AutoSet(NXB_CLOSE);

		//TES 6/23/2008 - PLID 26152 - First, determine whether there are any products that are flagged to be returned.
		_RecordsetPtr rsSuppliers = CreateRecordset("SELECT MultiSupplierT.SupplierID, ProductItemsT.LocationID "
			"FROM MultiSupplierT INNER JOIN ProductItemsT ON MultiSupplierT.ProductID = ProductItemsT.ProductID "
			"WHERE ProductItemsT.ToBeReturned = 1 AND "
			"ProductItemsT.ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"   WHERE (Status = %li OR Status = %li) "
			"	AND ProductItemID Is Not Null) "
			"AND ProductItemsT.Deleted = 0 "
			"GROUP BY MultiSupplierT.SupplierID, ProductItemsT.LocationID", InvUtils::iadsActive, InvUtils::iadsUsed);

		if(rsSuppliers->eof) {
			//TES 6/23/2008 - PLID 26152 - There's nothing for us to do.
			MsgBox("You do not have any products flagged as needing to be returned.");
			OnOK();
			return TRUE;
		}

		//TES 6/23/2008 - PLID 26152 - Now, fill our supplier and location dropdowns with valid options.
		CString strSupplierIDs, strLocationIDs;
		while(!rsSuppliers->eof) {
			long nSupplierID = AdoFldLong(rsSuppliers, "SupplierID");
			long nLocationID = AdoFldLong(rsSuppliers, "LocationID");

			//TES 6/23/2008 - PLID 26152 - While we're at it, grab the first supplier/location combination, we will use
			// that as our default selection.
			if(m_nFirstSupplierID == -1) {
				m_nFirstSupplierID = nSupplierID;
			}
			if(m_nFirstLocationID == -1) {
				m_nFirstLocationID = nLocationID;
			}
			strSupplierIDs += FormatString("%li,", nSupplierID);
			strLocationIDs += FormatString("%li,", nLocationID);
			rsSuppliers->MoveNext();
		}
		strSupplierIDs.TrimRight(",");
		m_pSupplierList = BindNxDataList2Ctrl(IDC_SUPPLIER_LIST, false);
		m_pSupplierList->WhereClause = _bstr_t("PersonT.ID IN (" + strSupplierIDs + ")");
		m_pSupplierList->Requery();

		strLocationIDs.TrimRight(",");
		m_pLocationList = BindNxDataList2Ctrl(IDC_LOCATION_LIST, false);
		m_pLocationList->WhereClause = _bstr_t("LocationsT.ID IN (" + strLocationIDs + ")");
		m_pLocationList->Requery();
		
		//TES 6/23/2008 - PLID 26152 - We will requery this once our supplier and locations lists are ready.
		m_pProductList = BindNxDataList2Ctrl(IDC_PRODUCTS_TO_BE_RETURNED, false);

	}NxCatchAll("Error in CProductsToBeReturnedDlg::OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CProductsToBeReturnedDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CProductsToBeReturnedDlg)
	ON_EVENT(CProductsToBeReturnedDlg, IDC_SUPPLIER_LIST, 18 /* RequeryFinished */, OnRequeryFinishedSupplierList, VTS_I2)
	ON_EVENT(CProductsToBeReturnedDlg, IDC_SUPPLIER_LIST, 1 /* SelChanging */, OnSelChangingSupplierList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CProductsToBeReturnedDlg, IDC_SUPPLIER_LIST, 16 /* SelChosen */, OnSelChosenSupplierList, VTS_DISPATCH)
	ON_EVENT(CProductsToBeReturnedDlg, IDC_PRODUCTS_TO_BE_RETURNED, 18 /* RequeryFinished */, OnRequeryFinishedProductsToBeReturned, VTS_I2)
	ON_EVENT(CProductsToBeReturnedDlg, IDC_PRODUCTS_TO_BE_RETURNED, 10 /* EditingFinished */, OnEditingFinishedProductsToBeReturned, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CProductsToBeReturnedDlg, IDC_LOCATION_LIST, 1 /* SelChanging */, OnSelChangingLocationList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CProductsToBeReturnedDlg, IDC_LOCATION_LIST, 16 /* SelChosen */, OnSelChosenLocationList, VTS_DISPATCH)
	ON_EVENT(CProductsToBeReturnedDlg, IDC_LOCATION_LIST, 18 /* RequeryFinished */, OnRequeryFinishedLocationList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CProductsToBeReturnedDlg::OnRequeryFinishedSupplierList(short nFlags) 
{
	try {
		//TES 6/23/2008 - PLID 26152 - Set our default supplier
		m_pSupplierList->SetSelByColumn(slcID, m_nFirstSupplierID);
		OnSelChosenSupplierList(m_pSupplierList->CurSel);
	}NxCatchAll("Error in CProductsToBeReturnedDlg::OnRequeryFinishedSupplierList()");
}

void CProductsToBeReturnedDlg::OnSelChangingSupplierList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		//TES 6/23/2008 - PLID 26152 - Force a valid selection.
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CProductsToBeReturnedDlg::OnSelChangingSupplierList()");
}

void CProductsToBeReturnedDlg::OnSelChosenSupplierList(LPDISPATCH lpRow) 
{
	try {
		//TES 6/23/2008 - PLID 26152 - Reload the list of products based on our new supplier.
		RequeryProductList();

	}NxCatchAll("Error in CProductsToBeReturnedDlg::OnSelChosenSupplierList()");
}

void CProductsToBeReturnedDlg::OnRequeryFinishedProductsToBeReturned(short nFlags) 
{
	try {
		if(m_pProductList->GetRowCount()) {
			//TES 6/23/2008 - PLID 26152 - We can return all now (this button was disabled before requerying).
			m_nxbReturnAll.EnableWindow(TRUE);
		}
		else {
			//TES 6/23/2008 - PLID 26152 - Let them know that there's nothing they can do with these.
			MsgBox("The currently selected supplier and location do not have any products which are flagged to be returned");
		}
	}NxCatchAll("Error in CProductsToBeReturnedDlg::OnRequeryFinishedProductsToBeReturned()");
}

void CProductsToBeReturnedDlg::OnEditingFinishedProductsToBeReturned(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		if(!bCommit) {
			return;
		}

		switch(nCol) {
		case plcSelected:
			{
				//TES 6/23/2008 - PLID 26152 - If there's at least one row checked off now, we can "Return Selected",
				// otherwise we can't.
				if(VarBool(varNewValue)) {
					m_nxbReturnSelected.EnableWindow(TRUE);
					return;
				}
				else {
					pRow = m_pProductList->GetFirstRow();
					while(pRow) {
						if(VarBool(pRow->GetValue(plcSelected))) {
							m_nxbReturnSelected.EnableWindow(TRUE);
							return;
						}
						pRow = pRow->GetNextRow();
					}
					m_nxbReturnSelected.EnableWindow(FALSE);
				}
			}
			break;
		}
	}NxCatchAll("Error in CProductsToBeReturnedDlg::OnEditingFinishedProductsToBeReturned()");
}

void CProductsToBeReturnedDlg::OnCloseProductsToReturn() 
{
	CNxDialog::OnOK();	
}

void CProductsToBeReturnedDlg::OnSelChangingLocationList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		//TES 6/23/2008 - PLID 26152 - Enforce a valid selection.
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CProductsToBeReturnedDlg::OnSelChangingLocationList()");
}

void CProductsToBeReturnedDlg::OnSelChosenLocationList(LPDISPATCH lpRow) 
{
	try {
		//TES 6/23/2008 - PLID 26152 - Refilter the list of product items based on this new location.
		RequeryProductList();
	}NxCatchAll("Error in CProductsToBeReturnedDlg::OnSelChosenLocationList()");
}

void CProductsToBeReturnedDlg::OnRequeryFinishedLocationList(short nFlags) 
{
	try {
		//TES 6/23/2008 - PLID 26152 - Select our default location.
		m_pLocationList->SetSelByColumn(slcID, m_nFirstLocationID);
		OnSelChosenLocationList(m_pLocationList->CurSel);
	}NxCatchAll("Error in CProductsToBeReturnedDlg::OnRequeryFinishedSupplierList()");
}

void CProductsToBeReturnedDlg::RequeryProductList()
{
	//TES 6/23/2008 - PLID 26152 - Since we're about to clear out our list, they can't return anything.
	m_nxbReturnAll.EnableWindow(FALSE);
	m_nxbReturnSelected.EnableWindow(FALSE);
	
	//TES 6/23/2008 - PLID 26152 - Make sure they have a valid supplier and location selected.
	if(m_pSupplierList->CurSel == NULL) {
		m_pProductList->Clear();
		return;
	}
	if(m_pLocationList->CurSel == NULL) {
		m_pProductList->Clear();
		return;
	}

	long nSupplierID = VarLong(m_pSupplierList->CurSel->GetValue(slcID));
	long nLocationID = VarLong(m_pLocationList->CurSel->GetValue(llcID));

	//TES 6/23/2008 - PLID 26152 - OK, now filter on just the product items for this supplier and location, which are 
	// available to be returned, and flagged as To Be Returned.
	m_pProductList->WhereClause = _bstr_t(FormatString("ServiceT.ID IN (SELECT ProductID FROM MultiSupplierT "
		"WHERE SupplierID = %li) AND "
		"ProductItemsT.ToBeReturned = 1 AND "
		"ProductItemsT.ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
		"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
		"   WHERE (Status = %li OR Status = %li) "
		"	AND ProductItemID Is Not Null) "
		"AND ProductItemsT.Deleted = 0 AND ProductItemsT.LocationID = %li", 
		nSupplierID, InvUtils::iadsActive, InvUtils::iadsUsed, nLocationID));

	m_pProductList->Requery();
}

void CProductsToBeReturnedDlg::OnNewReturnAll() 
{
	try {
		//TES 6/23/2008 - PLID 26152 - Pull our location and supplier.
		if(m_pLocationList->CurSel == NULL) {
			//We should never have gotten here!
			ASSERT(FALSE);
			return;
		}
		long nLocationID = VarLong(m_pLocationList->CurSel->GetValue(llcID));

		if(m_pSupplierList->CurSel == NULL) {
			//We should never have gotten here!
			ASSERT(FALSE);
			return;
		}
		long nSupplierID = VarLong(m_pSupplierList->CurSel->GetValue(slcID));

		//TES 6/23/2008 - PLID 26152 - Now load all the information the return dialog will need about each item into
		// an array.
		CArray<ProductItemReturnInfo,ProductItemReturnInfo&> arProductItems;
		IRowSettingsPtr pRow = m_pProductList->GetFirstRow();
		while(pRow) {
			ProductItemReturnInfo piri;
			piri.nProductItemID = VarLong(pRow->GetValue(plcID));
			piri.nProductID = VarLong(pRow->GetValue(plcProductID));
			piri.strProductName = VarString(pRow->GetValue(plcProduct));
			CString strSerialNum = VarString(pRow->GetValue(plcSerialNum),"");
			if(strSerialNum != "<No Data>") {
				piri.strSerialNum = strSerialNum;
			}
			_variant_t varExpDate = pRow->GetValue(plcExpDate);
			if(varExpDate.vt == VT_DATE) {
				piri.varExpDate = varExpDate;
			}
			else {
				piri.varExpDate = g_cvarNull;
			}
			//DRT 7/23/2008 - PLID 30815
			piri.varProductLastCost = pRow->GetValue(plcLastCost);
			arProductItems.Add(piri);
			pRow = pRow->GetNextRow();
		}

		if(arProductItems.GetSize() == 0) {
			//We should never have gotten here!
			ASSERT(FALSE);
			return;
		}

		//TES 6/23/2008 - PLID 26152 - Now create the return dialog, and initialize it with our list.
		CInvEditReturnDlg dlg(this);
		dlg.SetLoadItems(nLocationID, nSupplierID, arProductItems);

		//TES 6/23/2008 - PLID 26152 - We're done ourselves, the return dialog will take it from here.
		OnOK();

		//TES 6/23/2008 - PLID 26152 - Finally, show the return dialog.  We didn't check permissions, this dialog
		// shouldn't have been launched if the user didn't have permission to create returns.
		dlg.DoModal();

	}NxCatchAll("Error in CProductsToBeReturnedDlg::OnNewReturnAll()");
}

void CProductsToBeReturnedDlg::OnNewReturnSelected() 
{
	try {
		//TES 6/23/2008 - PLID 26152 - Pull our location and supplier.
		if(m_pLocationList->CurSel == NULL) {
			//We should never have gotten here!
			ASSERT(FALSE);
			return;
		}
		long nLocationID = VarLong(m_pLocationList->CurSel->GetValue(llcID));

		if(m_pSupplierList->CurSel == NULL) {
			//We should never have gotten here!
			ASSERT(FALSE);
			return;
		}
		long nSupplierID = VarLong(m_pSupplierList->CurSel->GetValue(slcID));

		//TES 6/23/2008 - PLID 26152 - Now load all the information the return dialog will need about each item into
		// an array.
		CArray<ProductItemReturnInfo,ProductItemReturnInfo&> arProductItems;
		IRowSettingsPtr pRow = m_pProductList->GetFirstRow();
		while(pRow) {
			//TES 6/23/2008 - PLID 26152 - Is this row selected?
			if(VarBool(pRow->GetValue(plcSelected))) {
				ProductItemReturnInfo piri;
				piri.nProductItemID = VarLong(pRow->GetValue(plcID));
				piri.nProductID = VarLong(pRow->GetValue(plcProductID));
				piri.strProductName = VarString(pRow->GetValue(plcProduct));
				CString strSerialNum = VarString(pRow->GetValue(plcSerialNum),"");
				if(strSerialNum != "<No Data>") {
					piri.strSerialNum = strSerialNum;
				}
				_variant_t varExpDate = pRow->GetValue(plcExpDate);
				if(varExpDate.vt == VT_DATE) {
					piri.varExpDate = varExpDate;
				}
				else {
					piri.varExpDate = g_cvarNull;
				}
				//DRT 7/23/2008 - PLID 30815
				piri.varProductLastCost = pRow->GetValue(plcLastCost);
				arProductItems.Add(piri);
			}
			pRow = pRow->GetNextRow();
		}

		if(arProductItems.GetSize() == 0) {
			//We should never have gotten here!
			ASSERT(FALSE);
			return;
		}

		//TES 6/23/2008 - PLID 26152 - Now create the return dialog, and initialize it with our list.
		CInvEditReturnDlg dlg(this);
		dlg.SetLoadItems(nLocationID, nSupplierID, arProductItems);

		//TES 6/23/2008 - PLID 26152 - We're done ourselves, the return dialog will take it from here.
		OnOK();

		//TES 6/23/2008 - PLID 26152 - Finally, show the return dialog.  We didn't check permissions, this dialog
		// shouldn't have been launched if the user didn't have permission to create returns.
		dlg.DoModal();
	}NxCatchAll("Error in CProductsToBeReturnedDlg::OnNewReturnAll()");
}

// ProductsToBeOrderedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "inventoryrc.h"
#include "ProductsToBeOrderedDlg.h"
#include "InvUtils.h"
#include "InvEditOrderDlg.h"
#include "InvOrderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TES 7/22/2008 - PLID 30802 - Created, largely copied from CProductsToBeReturnedDlg
/////////////////////////////////////////////////////////////////////////////
// CProductsToBeOrderedDlg dialog

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
	plcProductName = 3,
	plcPatientName = 4,
	plcApptDate = 5,
	plcQuantity = 6,
};

CProductsToBeOrderedDlg::CProductsToBeOrderedDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CProductsToBeOrderedDlg::IDD, pParent)
{
	m_nFirstLocationID = -1;
	m_nFirstSupplierID = -1;
	m_pInvOrderDlg = NULL;
	//{{AFX_DATA_INIT(CProductsToBeOrderedDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CProductsToBeOrderedDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProductsToBeOrderedDlg)
	DDX_Control(pDX, IDC_ORDER_SELECTED, m_nxbOrderSelected);
	DDX_Control(pDX, IDC_ORDER_ALL, m_nxbOrderAll);
	DDX_Control(pDX, IDC_CLOSE_TO_BE_ORDERED, m_nxbClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProductsToBeOrderedDlg, CNxDialog)
	//{{AFX_MSG_MAP(CProductsToBeOrderedDlg)
	ON_BN_CLICKED(IDC_CLOSE_TO_BE_ORDERED, OnCloseToBeOrdered)
	ON_BN_CLICKED(IDC_ORDER_ALL, OnOrderAll)
	ON_BN_CLICKED(IDC_ORDER_SELECTED, OnOrderSelected)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProductsToBeOrderedDlg message handlers

void CProductsToBeOrderedDlg::OnCloseToBeOrdered() 
{
	CNxDialog::OnOK();		
}

using namespace ADODB;
using namespace NXDATALIST2Lib;

BOOL CProductsToBeOrderedDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		m_nxbClose.AutoSet(NXB_CLOSE);
		m_nxbOrderSelected.AutoSet(NXB_NEW);
		m_nxbOrderAll.AutoSet(NXB_NEW);

		//TES 7/22/2008 - PLID 30802 - First, determine whether there are any products that are flagged to be ordered.
		// (j.jones 2008-09-29 15:39) - PLID 31520 - this shouldn't include products that are not trackable
		//TES 4/20/2011 - PLID 43337 - Filter out inactive Products
		_RecordsetPtr rsSuppliers = CreateParamRecordset("SELECT MultiSupplierT.SupplierID, PatientInvAllocationsT.LocationID "
			"FROM MultiSupplierT "
			"INNER JOIN PatientInvAllocationDetailsT ON MultiSupplierT.ProductID = PatientInvAllocationDetailsT.ProductID "
			"INNER JOIN ServiceT ON MultiSupplierT.ProductID = ServiceT.ID "
			"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"INNER JOIN PersonT ON MultiSupplierT.SupplierID = PersonT.ID "
			"INNER JOIN ProductLocationInfoT ON PatientInvAllocationsT.LocationID = ProductLocationInfoT.LocationID "
			"	AND PatientInvAllocationDetailsT.ProductID = ProductLocationInfoT.ProductID "
			"WHERE PatientInvAllocationDetailsT.Status = {INT} "
			"AND PatientInvAllocationDetailsT.OrderDetailID Is Null "
			"AND PatientInvAllocationsT.Status <> {INT} "
			"AND PersonT.Archived = 0 "
			"AND ProductLocationInfoT.TrackableStatus <> 0 "
			"AND ServiceT.Active = 1 "
			"GROUP BY MultiSupplierT.SupplierID, PatientInvAllocationsT.LocationID", InvUtils::iadsOrder, InvUtils::iasDeleted);

		if(rsSuppliers->eof) {
			//TES 7/22/2008 - PLID 30802 - There's nothing for us to do.
			MsgBox("You do not have any products flagged as needing to be ordered on an allocation.");
			OnOK();
			return TRUE;
		}

		//TES 7/22/2008 - PLID 30802 - Now, fill our supplier and location dropdowns with valid options.
		CString strSupplierIDs, strLocationIDs;
		while(!rsSuppliers->eof) {
			long nSupplierID = AdoFldLong(rsSuppliers, "SupplierID");
			long nLocationID = AdoFldLong(rsSuppliers, "LocationID");

			//TES 7/22/2008 - PLID 30802 - While we're at it, grab the first supplier/location combination, we will use
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
		
		//TES 7/22/2008 - PLID 30802 - We will requery this once our supplier and locations lists are ready.
		m_pProductList = BindNxDataList2Ctrl(IDC_PRODUCTS_TO_BE_ORDERED, false);

	}NxCatchAll("Error in CProductsToBeOrderedDlg::OnInitDialog()");


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CProductsToBeOrderedDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CProductsToBeOrderedDlg)
	ON_EVENT(CProductsToBeOrderedDlg, IDC_LOCATION_LIST, 1 /* SelChanging */, OnSelChangingLocationList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CProductsToBeOrderedDlg, IDC_LOCATION_LIST, 16 /* SelChosen */, OnSelChosenLocationList, VTS_DISPATCH)
	ON_EVENT(CProductsToBeOrderedDlg, IDC_LOCATION_LIST, 18 /* RequeryFinished */, OnRequeryFinishedLocationList, VTS_I2)
	ON_EVENT(CProductsToBeOrderedDlg, IDC_SUPPLIER_LIST, 1 /* SelChanging */, OnSelChangingSupplierList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CProductsToBeOrderedDlg, IDC_SUPPLIER_LIST, 16 /* SelChosen */, OnSelChosenSupplierList, VTS_DISPATCH)
	ON_EVENT(CProductsToBeOrderedDlg, IDC_SUPPLIER_LIST, 18 /* RequeryFinished */, OnRequeryFinishedSupplierList, VTS_I2)
	ON_EVENT(CProductsToBeOrderedDlg, IDC_PRODUCTS_TO_BE_ORDERED, 18 /* RequeryFinished */, OnRequeryFinishedProductsToBeOrdered, VTS_I2)
	ON_EVENT(CProductsToBeOrderedDlg, IDC_PRODUCTS_TO_BE_ORDERED, 10 /* EditingFinished */, OnEditingFinishedProductsToBeOrdered, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CProductsToBeOrderedDlg::OnSelChangingLocationList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		//TES 7/22/2008 - PLID 30802 - Enforce a valid selection.
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}		
	}NxCatchAll("Error in CProductsToBeOrderedDlg::OnSelChangingLocationList()");
}

void CProductsToBeOrderedDlg::OnSelChosenLocationList(LPDISPATCH lpRow) 
{
	try {
		//TES 7/22/2008 - PLID 30802 - Refilter the list of product items based on this new location.
		RequeryProductList();

	}NxCatchAll("Error in CProductsToBeOrderedDlg::OnSelChosenLocationList()");
}

void CProductsToBeOrderedDlg::OnRequeryFinishedLocationList(short nFlags) 
{
	try {
		//TES 7/22/2008 - PLID 30802 - Set our default location
		m_pLocationList->SetSelByColumn(slcID, m_nFirstLocationID);
		OnSelChosenLocationList(m_pLocationList->CurSel);

	}NxCatchAll("Error in CProductsToBeOrderedDlg::OnRequeryFinishedLocationList()");
}

void CProductsToBeOrderedDlg::OnSelChangingSupplierList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		//TES 7/22/2008 - PLID 30802 - Enforce a valid selection.
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CProductsToBeOrderedDlg::OnSelChangingSupplierList()");
}

void CProductsToBeOrderedDlg::OnSelChosenSupplierList(LPDISPATCH lpRow) 
{
	try {
		//TES 7/22/2008 - PLID 30802 - Reload the list of products based on our new supplier.
		RequeryProductList();

	}NxCatchAll("Error in CProductsToBeOrderedDlg::OnSelChosenSupplierList()");
}

void CProductsToBeOrderedDlg::OnRequeryFinishedSupplierList(short nFlags) 
{
	try {
		//TES 7/22/2008 - PLID 30802 - Set our default supplier
		m_pSupplierList->SetSelByColumn(slcID, m_nFirstSupplierID);
		OnSelChosenSupplierList(m_pSupplierList->CurSel);

	}NxCatchAll("Error in CProductsToBeOrderedDlg::OnRequeryFinishedSupplierList()");
}

void CProductsToBeOrderedDlg::RequeryProductList()
{
	//TES 7/22/2008 - PLID 30802 - Since we're about to clear out our list, they can't order anything.
	m_nxbOrderAll.EnableWindow(FALSE);
	m_nxbOrderSelected.EnableWindow(FALSE);
	
	//TES 7/22/2008 - PLID 30802 - Make sure they have a valid supplier and location selected.
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

	//TES 7/22/2008 - PLID 30802 - OK, now filter on just the allocation details for this supplier and location, which are 
	// flagged to be ordered, not already on an order, and not on a deleted allocation
	// (j.jones 2008-09-29 15:39) - PLID 31520 - this shouldn't include products that are not trackable
	// for the selected location
	//TES 4/20/2011 - PLID 43337 - Filter out inactive Products
	m_pProductList->WhereClause = _bstr_t(FormatString(
		"ServiceT.ID IN (SELECT ProductID FROM MultiSupplierT WHERE SupplierID = %li) "
		"AND ServiceT.ID IN (SELECT ProductID FROM ProductLocationInfoT WHERE LocationID = %li AND TrackableStatus <> 0) "
		"AND PatientInvAllocationDetailsT.Status = %li "
		"AND PatientInvAllocationDetailsT.OrderDetailID Is Null "
		"AND PatientInvAllocationsT.Status <> %li "
		"AND PatientInvAllocationsT.LocationID = %li "
		"AND ServiceT.Active = 1 ", 
		nSupplierID, nLocationID, InvUtils::iadsOrder, InvUtils::iasDeleted, nLocationID));

	m_pProductList->Requery();
}

void CProductsToBeOrderedDlg::OnRequeryFinishedProductsToBeOrdered(short nFlags) 
{
	try {
		if(m_pProductList->GetRowCount()) {
			//TES 7/22/2008 - PLID 30802 - We can order all now (this button was disabled before requerying).
			m_nxbOrderAll.EnableWindow(TRUE);
		}
		else {
			//TES 7/22/2008 - PLID 30802 - Let them know that there's nothing they can do with these.
			MsgBox("The currently selected supplier and location do not have any products which are flagged to be ordered on an allocation.");
		}

	}NxCatchAll("CProductsToBeOrderedDlg::OnRequeryFinishedProductsToBeOrdered()");
}

void CProductsToBeOrderedDlg::OnEditingFinishedProductsToBeOrdered(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
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
				//TES 7/22/2008 - PLID 30802 - If there's at least one row checked off now, we can "Order Selected",
				// otherwise we can't.
				if(VarBool(varNewValue)) {
					m_nxbOrderSelected.EnableWindow(TRUE);
					return;
				}
				else {
					pRow = m_pProductList->GetFirstRow();
					while(pRow) {
						if(VarBool(pRow->GetValue(plcSelected))) {
							m_nxbOrderSelected.EnableWindow(TRUE);
							return;
						}
						pRow = pRow->GetNextRow();
					}
					m_nxbOrderSelected.EnableWindow(FALSE);
				}
			}
			break;
		}
	}NxCatchAll("Error in CProductsToBeOrderedDlg::OnEditingFinishedProductsToBeReturned()");	
}

void CProductsToBeOrderedDlg::OnOrderAll() 
{
	try {
		if(m_pSupplierList->CurSel == NULL) {
			ASSERT(FALSE);
			return;
		}
		if(m_pLocationList->CurSel == NULL) {
			ASSERT(FALSE);
			return;
		}

		long nSupplierID = VarLong(m_pSupplierList->CurSel->GetValue(slcID));
		long nLocationID = VarLong(m_pLocationList->CurSel->GetValue(llcID));

		//TES 7/22/2008 - PLID 30802 - Go through and load all the information about these allocation details.
		CArray<AllocationDetail,AllocationDetail&> arDetails;
		IRowSettingsPtr pRow = m_pProductList->GetFirstRow();
		while(pRow) {
			AllocationDetail ad;
			ad.nAllocationDetailID = VarLong(pRow->GetValue(plcID));
			ad.nProductID = VarLong(pRow->GetValue(plcProductID));
			ad.dQty = VarDouble(pRow->GetValue(plcQuantity));
			arDetails.Add(ad);
			pRow = pRow->GetNextRow();
		}

		//TES 7/22/2008 - PLID 30802 - Pre-fill the order with that information.
		CInvEditOrderDlg *pEditOrderDlg = m_pInvOrderDlg->GetEditOrderDlg();
		pEditOrderDlg->SetAllocationDetails(arDetails);

		//TES 7/22/2008 - PLID 30802 - We're done, so dismiss this dialog.
		OnOK();

		//TES 7/22/2008 - PLID 30802 - Show the order.
		pEditOrderDlg->DoFakeModal(-1, FALSE, -1, nLocationID, nSupplierID);

	}NxCatchAll("Error in CProductsToBeOrderedDlg::OnOrderAll()");
}

void CProductsToBeOrderedDlg::OnOrderSelected() 
{
	try {
		if(m_pSupplierList->CurSel == NULL) {
			ASSERT(FALSE);
			return;
		}
		if(m_pLocationList->CurSel == NULL) {
			ASSERT(FALSE);
			return;
		}

		long nSupplierID = VarLong(m_pSupplierList->CurSel->GetValue(slcID));
		long nLocationID = VarLong(m_pLocationList->CurSel->GetValue(llcID));

		//TES 7/22/2008 - PLID 30802 - Go through and load all the information about the selected allocation details.
		CArray<AllocationDetail,AllocationDetail&> arDetails;
		IRowSettingsPtr pRow = m_pProductList->GetFirstRow();
		while(pRow) {
			if(VarBool(pRow->GetValue(plcSelected))) {
				AllocationDetail ad;
				ad.nAllocationDetailID = VarLong(pRow->GetValue(plcID));
				ad.nProductID = VarLong(pRow->GetValue(plcProductID));
				ad.dQty = VarDouble(pRow->GetValue(plcQuantity));
				arDetails.Add(ad);
			}
			pRow = pRow->GetNextRow();
		}

		//TES 7/22/2008 - PLID 30802 - Pre-fill the order with that information.
		CInvEditOrderDlg *pEditOrderDlg = m_pInvOrderDlg->GetEditOrderDlg();
		pEditOrderDlg->SetAllocationDetails(arDetails);

		//TES 7/22/2008 - PLID 30802 - We're done, so dismiss this dialog.
		OnOK();

		//TES 7/22/2008 - PLID 30802 - Show the order.
		pEditOrderDlg->DoFakeModal(-1, FALSE, -1, nLocationID, nSupplierID);

		// (j.jones 2008-09-29 13:55) - PLID 31520 - corrected the text in this exception
		// to properly reference this function name
	}NxCatchAll("Error in CProductsToBeOrderedDlg::OnOrderSelected()");
}

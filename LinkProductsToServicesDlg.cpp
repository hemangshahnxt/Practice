// LinkProductsToServicesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "inventoryrc.h"
#include "LinkProductsToServicesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TES 7/16/2008 - PLID 27983 - Created
/////////////////////////////////////////////////////////////////////////////
// CLinkProductsToServicesDlg dialog


CLinkProductsToServicesDlg::CLinkProductsToServicesDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLinkProductsToServicesDlg::IDD, pParent)
{
	m_nCurrentCptID = -1;

	//{{AFX_DATA_INIT(CLinkProductsToServicesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLinkProductsToServicesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLinkProductsToServicesDlg)
	DDX_Control(pDX, IDC_LINK_PRODUCTS_DESCRIPTION, m_nxsDescription);
	DDX_Control(pDX, IDC_UNLINK_PRODUCT, m_nxbUnlinkProduct);
	DDX_Control(pDX, IDC_LINK_PRODUCT, m_nxbLinkProduct);
	DDX_Control(pDX, IDC_CLOSE_LINK_PRODUCTS, m_nxbClose);
	DDX_Control(pDX, IDC_INCLUDE_UNLINKED, m_nxbIncludeUnlinked);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLinkProductsToServicesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CLinkProductsToServicesDlg)
	ON_BN_CLICKED(IDC_INCLUDE_UNLINKED, OnIncludeUnlinked)
	ON_BN_CLICKED(IDC_CLOSE_LINK_PRODUCTS, OnClose)
	ON_BN_CLICKED(IDC_LINK_PRODUCT, OnLinkProduct)
	ON_BN_CLICKED(IDC_UNLINK_PRODUCT, OnUnlinkProduct)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLinkProductsToServicesDlg message handlers

BOOL CLinkProductsToServicesDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		//TES 7/16/2008 - PLID 27983 - Initialize our controls.
		m_nxbClose.AutoSet(NXB_CLOSE);
		m_nxbLinkProduct.AutoSet(NXB_RIGHT);
		m_nxbUnlinkProduct.AutoSet(NXB_LEFT);

		m_pCptList = BindNxDataList2Ctrl(IDC_AVAILABLE_CPT_LIST, false);
		m_pAvailableProducts = BindNxDataList2Ctrl(IDC_AVAILABLE_PRODUCTS, false);
		m_pLinkedProducts = BindNxDataList2Ctrl(IDC_LINKED_PRODUCTS, false);

		//TES 7/16/2008 - PLID 27983 - Set our caption (to long to store in resources).
		m_nxsDescription.SetWindowText("Use this dialog to assign products to \"placeholder\" Service Codes.  When a Service Code has "
			"products linked to it through this dialog, it will only be available on surgeries, quotes, and uncompleted case "
			"histories.  When the code is billed (or its case history is completed), you will be prompted to select which of "
			"the products was actually used.");

		//TES 7/16/2008 - PLID 27983 - Are there any unlinked products?
		if(ReturnsRecords("SELECT CptID FROM ServiceToProductLinkT")) {
			m_nxbIncludeUnlinked.SetCheck(BST_UNCHECKED);
		}
		else {
			m_nxbIncludeUnlinked.SetCheck(BST_CHECKED);
		}

		//TES 7/16/2008 - PLID 27983 - This will cause the screen to refresh.
		OnIncludeUnlinked();


	}NxCatchAll("Error in CLinkProductsToServicesDlg::OnInitDialog()");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLinkProductsToServicesDlg::OnIncludeUnlinked() 
{
	try {
		//TES 7/16/2008 - PLID 27983 - Disable while we requery.
		GetDlgItem(IDC_AVAILABLE_PRODUCTS)->EnableWindow(FALSE);
		GetDlgItem(IDC_LINKED_PRODUCTS)->EnableWindow(FALSE);
		GetDlgItem(IDC_LINK_PRODUCT)->EnableWindow(FALSE);
		GetDlgItem(IDC_UNLINK_PRODUCT)->EnableWindow(FALSE);

		//TES 7/16/2008 - PLID 27983 - Set the where clause.
		// (j.jones 2011-03-28 16:58) - PLID 42575 - ignore non-billable CPT codes
		if(IsDlgButtonChecked(IDC_INCLUDE_UNLINKED)) {
			m_pCptList->WhereClause = _bstr_t("ServiceT.Active = 1 AND CPTCodeT.Billable = 1");
		}
		else {
			m_pCptList->WhereClause = _bstr_t("ServiceT.Active = 1 AND CPTCodeT.Billable = 1 AND ServiceT.ID IN (SELECT CptID FROM ServiceToProductLinkT "
				"WHERE CptID Is Not Null)");
		}

		//TES 7/16/2008 - PLID 27983 - Remember which row we have selected
		if(m_pCptList->CurSel == NULL) {
			m_nCurrentCptID = -1;
		}
		else {
			m_nCurrentCptID = VarLong(m_pCptList->CurSel->GetValue(0));
		}

		//TES 7/16/2008 - PLID 27983 - Finally, requery, OnRequeryFinished will take care of the reset.
		m_pCptList->Requery();

	}NxCatchAll("Error in CLinkProductsToServicesDlg::OnIncludeUnlinked()");
}

BEGIN_EVENTSINK_MAP(CLinkProductsToServicesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CLinkProductsToServicesDlg)
	ON_EVENT(CLinkProductsToServicesDlg, IDC_AVAILABLE_CPT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedCptList, VTS_I2)
	ON_EVENT(CLinkProductsToServicesDlg, IDC_AVAILABLE_CPT_LIST, 16 /* SelChosen */, OnSelChosenCptList, VTS_DISPATCH)
	ON_EVENT(CLinkProductsToServicesDlg, IDC_AVAILABLE_PRODUCTS, 2 /* SelChanged */, OnSelChangedAvailableProducts, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CLinkProductsToServicesDlg, IDC_AVAILABLE_PRODUCTS, 3 /* DblClickCell */, OnDblClickCellAvailableProducts, VTS_DISPATCH VTS_I2)
	ON_EVENT(CLinkProductsToServicesDlg, IDC_LINKED_PRODUCTS, 2 /* SelChanged */, OnSelChangedLinkedProducts, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CLinkProductsToServicesDlg, IDC_LINKED_PRODUCTS, 3 /* DblClickCell */, OnDblClickCellLinkedProducts, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

using namespace NXDATALIST2Lib;
void CLinkProductsToServicesDlg::OnRequeryFinishedCptList(short nFlags) 
{
	try {
		//TES 7/16/2008 - PLID 27983 - Try to restore our previous selection.
		IRowSettingsPtr pSel = m_pCptList->FindByColumn(0, (long)m_nCurrentCptID, NULL, VARIANT_TRUE);
		if(pSel) {
			m_pCptList->CurSel = pSel;

			//TES 7/16/2008 - PLID 27983 - Just re-enable everything, we don't need to requery it.
			GetDlgItem(IDC_AVAILABLE_PRODUCTS)->EnableWindow(TRUE);
			GetDlgItem(IDC_LINKED_PRODUCTS)->EnableWindow(TRUE);
			EnableButtons();
		}
		else {
			if(m_nCurrentCptID == -1) {
				//TES 7/16/2008 - PLID 27983 - Try to select the first row.
				m_pCptList->CurSel = m_pCptList->GetFirstRow();
			}
			ReflectCurrentCpt();
		}

	}NxCatchAll("Error in CLinkProductsToServicesDlg::OnRequeryFinishedCptList()");
}

void CLinkProductsToServicesDlg::OnSelChosenCptList(LPDISPATCH lpRow) 
{
	try {

		ReflectCurrentCpt();

	}NxCatchAll("Error in CLinkProductsToServicesDlg::OnSelChosenCptList()");
}

void CLinkProductsToServicesDlg::EnableButtons()
{
	m_nxbLinkProduct.EnableWindow(m_pAvailableProducts->CurSel != NULL);
	m_nxbUnlinkProduct.EnableWindow(m_pLinkedProducts->CurSel != NULL);
}

void CLinkProductsToServicesDlg::ReflectCurrentCpt()
{
	IRowSettingsPtr pCptRow = m_pCptList->CurSel;
	if(pCptRow == NULL) {
		//TES 7/16/2008 - PLID 27983 - Just disable everything.
		GetDlgItem(IDC_AVAILABLE_PRODUCTS)->EnableWindow(FALSE);
		GetDlgItem(IDC_LINKED_PRODUCTS)->EnableWindow(FALSE);
		GetDlgItem(IDC_LINK_PRODUCT)->EnableWindow(FALSE);
		GetDlgItem(IDC_UNLINK_PRODUCT)->EnableWindow(FALSE);
	}
	else {
		//TES 7/16/2008 - PLID 27983 - Enable the datalists.
		GetDlgItem(IDC_AVAILABLE_PRODUCTS)->EnableWindow(TRUE);
		GetDlgItem(IDC_LINKED_PRODUCTS)->EnableWindow(TRUE);
		//TES 7/16/2008 - PLID 27983 - Refilter and requery the datalists.
		m_pAvailableProducts->WhereClause = _bstr_t(FormatString("ServiceT.Active = 1 AND ProductT.ID NOT IN (SELECT "
			"ProductID FROM ServiceToProductLinkT WHERE CptID = %li)", VarLong(pCptRow->GetValue(0))));
		m_pAvailableProducts->Requery();
		m_pLinkedProducts->WhereClause = _bstr_t(FormatString("ProductT.ID IN (SELECT "
			"ProductID FROM ServiceToProductLinkT WHERE CptID = %li)", VarLong(pCptRow->GetValue(0))));
		m_pLinkedProducts->Requery();

		EnableButtons();
	}
}

void CLinkProductsToServicesDlg::OnSelChangedAvailableProducts(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		EnableButtons();
	}NxCatchAll("Error in CLinkProductsToServicesDlg::OnSelChangedAvailableProducts()");
}

void CLinkProductsToServicesDlg::OnDblClickCellAvailableProducts(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		LinkProduct();
	}NxCatchAll("Error in CLinkProductsToServicesDlg::OnDblClickCellAvailableProducts()");
}

void CLinkProductsToServicesDlg::OnClose() 
{
	CNxDialog::OnOK();
}

void CLinkProductsToServicesDlg::OnLinkProduct() 
{
	try {
		LinkProduct();
	}NxCatchAll("Error in CLinkProductsToServicesDlg::OnLinkProduct()");
}

void CLinkProductsToServicesDlg::OnSelChangedLinkedProducts(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		EnableButtons();
	}NxCatchAll("Error in CLinkProductsToServicesDlg::OnSelChangedLinkedProducts()");
}

void CLinkProductsToServicesDlg::OnDblClickCellLinkedProducts(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		UnlinkProduct();
	}NxCatchAll("Error in CLinkProductsToServicesDlg::OnDblClickCellLinkedProducts()");
}

void CLinkProductsToServicesDlg::OnUnlinkProduct() 
{
	try {
		UnlinkProduct();
	}NxCatchAll("Error in CLinkProductsToServicesDlg::OnUnlinkProduct()");
}

void CLinkProductsToServicesDlg::LinkProduct()
{
	// (j.jones 2009-02-09 14:43) - PLID 32776 - converted this function to
	// support selecting/unselecting multiple at a time

	BOOL bWasEmpty = (m_pLinkedProducts->GetRowCount() == 0);

	//TES 7/16/2008 - PLID 27983 - Make sure we have a valid product and CPT code
	IRowSettingsPtr pProductSelRow = m_pAvailableProducts->GetFirstSelRow();
	if(pProductSelRow == NULL) {
		return;
	}
	IRowSettingsPtr pCptRow = m_pCptList->CurSel;
	if(pCptRow == NULL) {
		return;
	}

	long nCptID = VarLong(pCptRow->GetValue(0));

	CString strSqlBatch = BeginSqlBatch();

	while (pProductSelRow != NULL) {
		IRowSettingsPtr pRow = pProductSelRow;
		pProductSelRow = pProductSelRow->GetNextSelRow();

		//TES 7/16/2008 - PLID 27983 - Add it in data.
		// (j.jones 2009-02-09 14:44) - PLID 32776 - converted into a batch	
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ServiceToProductLinkT (CptID, ProductID) VALUES (%li, %li)", nCptID,
			VarLong(pRow->GetValue(0)));

		//TES 7/16/2008 - PLID 27983 - Update the screen.
		m_pLinkedProducts->AddRowSorted(pRow, NULL);
		m_pAvailableProducts->RemoveRow(pRow);
	}

	if(!strSqlBatch.IsEmpty()) {
		ExecuteSqlBatch(strSqlBatch);
	}

	if(bWasEmpty && m_pLinkedProducts->GetRowCount() > 0) {
		//TES 7/21/2008 - PLID 27983 - This code was billable, but shouldn't be any more, so make sure the bill screen gets refreshed.
		CClient::RefreshTable(NetUtils::CPTCodeT, nCptID);
	}
}

void CLinkProductsToServicesDlg::UnlinkProduct()
{
	// (j.jones 2009-02-09 14:43) - PLID 32776 - converted this function to
	// support selecting/unselecting multiple at a time

	//TES 7/16/2008 - PLID 27983 - Make sure we have a valid product and CPT Code
	IRowSettingsPtr pProductSelRow = m_pLinkedProducts->GetFirstSelRow();
	if(pProductSelRow == NULL) {
		return;
	}
	IRowSettingsPtr pCptRow = m_pCptList->CurSel;
	if(pCptRow == NULL) {
		return;
	}

	long nCptID = VarLong(pCptRow->GetValue(0));

	CString strSqlBatch = BeginSqlBatch();

	while (pProductSelRow != NULL) {
		IRowSettingsPtr pRow = pProductSelRow;
		pProductSelRow = pProductSelRow->GetNextSelRow();

		//ensure the request is batched, and unselected, but don't change the selection
		//if it is already in the batch

		//TES 7/16/2008 - PLID 27983 - Update the data.
		// (j.jones 2009-02-09 14:44) - PLID 32776 - converted into a batch	
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ServiceToProductLinkT WHERE CptID = %li AND ProductID = %li", nCptID,
			VarLong(pRow->GetValue(0)));

		//TES 7/16/2008 - PLID 27983 - Update the screen.
		m_pAvailableProducts->AddRowSorted(pRow, NULL);
		m_pLinkedProducts->RemoveRow(pRow);
	}

	if(!strSqlBatch.IsEmpty()) {
		ExecuteSqlBatch(strSqlBatch);
	}

	if(m_pLinkedProducts->GetRowCount() == 0) {
		//TES 7/21/2008 - PLID 27983 - This code wasn't billable, but should be now, so make sure the bill screen gets refreshed.
		CClient::RefreshTable(NetUtils::CPTCodeT, nCptID);
	}
}

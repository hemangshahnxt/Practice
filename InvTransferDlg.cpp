// InvTransferDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "InvTransferDlg.h"
#include "InvUtils.h"
#include "GlobalFinancialUtils.h"
#include "AuditTrail.h"
#include "ProductItemsDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;


//DRT 5/8/2006 - PLID 20486 - Only use in OnInitDialog
#define IDT_INIT_LOAD	WM_USER + 10000


/////////////////////////////////////////////////////////////////////////////
// CInvTransferDlg dialog


CInvTransferDlg::CInvTransferDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInvTransferDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInvTransferDlg)
		m_bUseUU = FALSE;
		m_UUConversion = 1;
		m_nDestLocationID = -1;
	//}}AFX_DATA_INIT
}


void CInvTransferDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvTransferDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_NAME, m_nxeditName);
	DDX_Control(pDX, IDC_STOCK_FROM, m_nxeditStockFrom);
	DDX_Control(pDX, IDC_STOCK_UO_FROM, m_nxeditStockUoFrom);
	DDX_Control(pDX, IDC_STOCK_TO, m_nxeditStockTo);
	DDX_Control(pDX, IDC_STOCK_UO_TO, m_nxeditStockUoTo);
	DDX_Control(pDX, IDC_TRANSFER_AMT, m_nxeditTransferAmt);
	DDX_Control(pDX, IDC_TRANSFER_AMT_UO, m_nxeditTransferAmtUo);
	DDX_Control(pDX, IDC_QUANTITY_FROM, m_nxeditQuantityFrom);
	DDX_Control(pDX, IDC_QUANTITY_UO_FROM, m_nxeditQuantityUoFrom);
	DDX_Control(pDX, IDC_QUANTITY_TO, m_nxeditQuantityTo);
	DDX_Control(pDX, IDC_QUANTITY_UO_TO, m_nxeditQuantityUoTo);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_UU_LABEL_FROM, m_nxstaticUuLabelFrom);
	DDX_Control(pDX, IDC_UO_LABEL_FROM, m_nxstaticUoLabelFrom);
	DDX_Control(pDX, IDC_UU_LABEL_TO, m_nxstaticUuLabelTo);
	DDX_Control(pDX, IDC_UO_LABEL_TO, m_nxstaticUoLabelTo);
	DDX_Control(pDX, IDC_UU_TRANSFER_LABEL, m_nxstaticUuTransferLabel);
	DDX_Control(pDX, IDC_UO_TRANSFER_LABEL, m_nxstaticUoTransferLabel);
	DDX_Control(pDX, IDC_UU_LABEL_NEW, m_nxstaticUuLabelNew);
	DDX_Control(pDX, IDC_UO_LABEL_NEW, m_nxstaticUoLabelNew);
	DDX_Control(pDX, IDC_LOC_FROM_NAME_LABEL, m_nxstaticLocFromNameLabel);
	DDX_Control(pDX, IDC_LOC_TO_NAME_LABEL, m_nxstaticLocToNameLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvTransferDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInvTransferDlg)
	ON_EN_KILLFOCUS(IDC_TRANSFER_AMT, OnKillfocusTransferAmt)
	ON_EN_KILLFOCUS(IDC_TRANSFER_AMT_UO, OnKillfocusTransferAmtUo)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvTransferDlg message handlers

BOOL CInvTransferDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (c.haag 2008-04-29 11:00) - PLID 29820 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_brush.CreateSolidBrush(PaletteColor(0x00FFDBDB));

		m_LocationFrom = BindNxDataListCtrl(this,IDC_LOCATION_FROM,GetRemoteData(),false);
		m_LocationTo = BindNxDataListCtrl(this,IDC_LOCATION_TO,GetRemoteData(),false);
		CString str;
		str.Format("Managed = 1 AND Active = 1 AND TypeID = 1 AND ProductLocationInfoT.TrackableStatus = 2 AND ProductLocationInfoT.ProductID = %li",m_nProductID);
		m_LocationFrom->PutWhereClause(_bstr_t(str));
		m_LocationFrom->Requery();
		m_LocationTo->PutWhereClause(_bstr_t(str));
		m_LocationTo->Requery();

		SetDlgItemInt(IDC_QUANTITY,0);

		_RecordsetPtr rs = CreateRecordset("SELECT Name, UseUU, Conversion FROM ServiceT INNER JOIN ProductT ON ServiceT.ID = ProductT.ID WHERE ServiceT.ID = %li", m_nProductID);
		if(!rs->eof) {
			SetDlgItemText(IDC_NAME, AdoFldString(rs, "Name",""));
			m_bUseUU = AdoFldBool(rs, "UseUU",FALSE);
			m_UUConversion = AdoFldLong(rs, "Conversion",1);
		}
		rs->Close();

		//DRT 5/8/2006 - PLID 20486 - 98% of all items are not using the UU distinction -- disable by default, enable only if needed.
		//if we want to use UU/UO
		if(m_bUseUU) {
			//hide the UO fields
			GetDlgItem(IDC_UU_LABEL_FROM)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_UO_LABEL_FROM)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STOCK_UO_FROM)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_UU_LABEL_TO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_UO_LABEL_TO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STOCK_UO_TO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_UU_TRANSFER_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_UO_TRANSFER_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_TRANSFER_AMT_UO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_UU_LABEL_NEW)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_UO_LABEL_NEW)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_QUANTITY_UO_FROM)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_QUANTITY_UO_TO)->ShowWindow(SW_SHOW);
		}

		//DRT 5/8/2006 - PLID 20486 - Moved this below the above query to give the asynchronous nature a little time to finish.  There is no difference
		//	to the user as long as this is selected before the end of this function.
		long nLocFromRow = m_LocationFrom->SetSelByColumn(0, m_nSourceLocationID);
		long nLocToRow = -1;
		if(m_nDestLocationID == -1) {
			_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 ID FROM LocationsT INNER JOIN ProductLocationInfoT ON LocationsT.ID = ProductLocationInfoT.LocationID "
				"WHERE Managed = 1 AND Active = 1 AND TypeID = 1 AND ProductLocationInfoT.TrackableStatus = 2 AND LocationID <> %li AND ProductID = %li ORDER BY Name",m_nSourceLocationID,m_nProductID);
			if(!rs->eof) {
				m_nDestLocationID = AdoFldLong(rs, "ID");			
			}
			else {
				MessageBox("You cannot use this feature without two managed locations that track inventory for this product.");
				CDialog::OnCancel();
			}
			rs->Close();
		}
		nLocToRow = m_LocationTo->SetSelByColumn(0, m_nDestLocationID);

	} NxCatchAll("Could not load inventory transfer dialog.");

	//DRT 5/8/2006 - PLID 20486 - Apparent speed, by allowing the dialog to display before we do the slowest part of loading the inventory quantities.
	SetTimer(IDT_INIT_LOAD, 25, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInvTransferDlg::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent) {
	//DRT 5/8/2006 - PLID 20486 - Apparent speed, by allowing the dialog to display before we do the slowest part of loading the inventory quantities.
	case IDT_INIT_LOAD:
		KillTimer(IDT_INIT_LOAD);
		OnSelChosenLocationFrom(m_LocationFrom->GetCurSel());
		OnSelChosenLocationTo(m_LocationTo->GetCurSel());
		break;
	}

	CDialog::OnTimer(nIDEvent);
}

void CInvTransferDlg::OnOK() 
{
	try {

		if(m_LocationFrom->CurSel == -1) {
			AfxMessageBox("You must have a source location selected to transfer an item.");
			return;
		}

		if(m_LocationTo->CurSel == -1) {
			AfxMessageBox("You must have a destination location selected to transfer an item.");
			return;
		}

		long nSourceLocationID = m_LocationFrom->GetValue(m_LocationFrom->CurSel, 0).lVal;
		long nDestLocationID = m_LocationTo->GetValue(m_LocationTo->CurSel, 0).lVal;

		if(nSourceLocationID == nDestLocationID) {
			AfxMessageBox("Your source and destination locations are the same. Please correct this before continuing.");
			return;
		}

		//get the text from the dialog
		CString value;
		GetDlgItemText (IDC_TRANSFER_AMT, value);
		//only allow it to include these numbers or a "."
		CString strQty = Include(value, ".1234567890");
		double dblQty = atof(strQty.GetBuffer(strQty.GetLength()));

		if(dblQty <= 0) {
			AfxMessageBox("You must transfer a quantity greater than zero.");
			return;
		}

		GetDlgItemText (IDC_STOCK_FROM, value);
		//only allow it to include these numbers or a "."
		CString strStock = Include(value, ".1234567890");
		double dblStock = atof(strStock.GetBuffer(strStock.GetLength()));

		if(dblQty > dblStock) {
			AfxMessageBox("You cannot transfer more items than your source location has in stock.");
			return;
		}

		CString strNotes;
		GetDlgItemText(IDC_NOTES, strNotes);

		//now see if it is a serialized product, and if so, prompt the user to move
		//the proper number of items to the new location
		if(!TransferProductItems(dblQty, nSourceLocationID, nDestLocationID)) {
			return;
		}

		InvUtils::TransferItems(m_nProductID, dblQty, nSourceLocationID, nDestLocationID, strNotes);

		CDialog::OnOK();
	}NxCatchAll("Could not save product transfer");
}

void CInvTransferDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CInvTransferDlg::OnKillfocusTransferAmt() 
{
	//first retrieve and format the Qty
	CString strQty;
	GetDlgItemText(IDC_TRANSFER_AMT,strQty);
	double dblQty = atof(strQty);

	if(dblQty < 0.0) {
		AfxMessageBox("You may not transfer a negative amount.");
		dblQty = 0.0;
	}

	strQty.Format("%g",dblQty);	
	SetDlgItemText(IDC_TRANSFER_AMT,strQty);

	CString strNewQuantityFrom;
	GetDlgItemText(IDC_STOCK_FROM, strNewQuantityFrom);
	double dblNewQuantityFrom = atof(strNewQuantityFrom) - dblQty;
	strNewQuantityFrom.Format("%g",dblNewQuantityFrom);
	SetDlgItemText(IDC_QUANTITY_FROM,strNewQuantityFrom);

	CString strNewQuantityTo;
	GetDlgItemText(IDC_STOCK_TO, strNewQuantityTo);
	double dblNewQuantityTo = atof(strNewQuantityTo) + dblQty;
	strNewQuantityTo.Format("%g",dblNewQuantityTo);
	SetDlgItemText(IDC_QUANTITY_TO,strNewQuantityTo);

	if(m_bUseUU) {
		double dblQtyUO = dblQty / m_UUConversion;
		CString strQtyUO;
		strQtyUO.Format("%g",dblQtyUO);
		SetDlgItemText(IDC_TRANSFER_AMT_UO, strQtyUO);

		dblQtyUO = dblNewQuantityFrom / m_UUConversion;
		strQtyUO.Format("%g",dblQtyUO);
		SetDlgItemText(IDC_QUANTITY_UO_FROM, strQtyUO);

		dblQtyUO = dblNewQuantityTo / m_UUConversion;
		strQtyUO.Format("%g",dblQtyUO);
		SetDlgItemText(IDC_QUANTITY_UO_TO, strQtyUO);
	}
}

void CInvTransferDlg::OnKillfocusTransferAmtUo() 
{
	//note, it is unlikely if not logically possible
	//that a user will type in a decimal value here, but we must allow it

	//first retrieve and format the UO Qty
	CString strQtyUO;
	GetDlgItemText(IDC_TRANSFER_AMT_UO,strQtyUO);
	double dblQtyUO = atof(strQtyUO);
	strQtyUO.Format("%g",dblQtyUO);	
	SetDlgItemText(IDC_TRANSFER_AMT_UO, strQtyUO);

	//now set the UU Qty
	long nQtyUU = (long)(m_UUConversion * dblQtyUO);
	SetDlgItemInt(IDC_TRANSFER_AMT,nQtyUU);

	//NOW we have to recalculate the UO,
	//because one could have typed in a number that did not come to a
	//whole UU, etc.
	//Plus, it will update the "new totals" at the bottom.
	OnKillfocusTransferAmt();
}

int CInvTransferDlg::DoModal(long nProductID, long nSourceLocationID, long nDestLocationID /* = -1*/)
{
	m_nProductID = nProductID;
	m_nSourceLocationID = nSourceLocationID;
	m_nDestLocationID = nDestLocationID;
	return CDialog::DoModal();
}

BEGIN_EVENTSINK_MAP(CInvTransferDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvTransferDlg)
	ON_EVENT(CInvTransferDlg, IDC_LOCATION_FROM, 16 /* SelChosen */, OnSelChosenLocationFrom, VTS_I4)
	ON_EVENT(CInvTransferDlg, IDC_LOCATION_TO, 16 /* SelChosen */, OnSelChosenLocationTo, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CInvTransferDlg::OnSelChosenLocationFrom(long nRow) 
{
	try {

		if(nRow == -1) {
			m_LocationFrom->CurSel = 0;
		}

		// (j.jones 2008-02-28 10:40) - PLID 28080 - converted item_sql into GetInventoryItemSql()
		// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
		_RecordsetPtr rsNumInStock = CreateParamRecordset(InvUtils::GetInventoryItemSql(m_nProductID, VarLong(m_LocationFrom->GetValue(m_LocationFrom->CurSel, 0))));
		double dblNumInStock = AdoFldDouble(rsNumInStock, "Actual");
		if(dblNumInStock < 0.0)
			dblNumInStock = 0.0;
		CString str;
		str.Format("%g",dblNumInStock);
		SetDlgItemText(IDC_STOCK_FROM,str);

		if(m_bUseUU) {
			double ActualUO = AdoFldDouble(rsNumInStock, "ActualUO",0.0);
			CString strActualUO;
			strActualUO.Format("%g",ActualUO);
			SetDlgItemText(IDC_STOCK_UO_FROM,strActualUO);
		}

		//set the transfer amount to zero
		SetDlgItemInt(IDC_TRANSFER_AMT,0);
		OnKillfocusTransferAmt();
	}NxCatchAll("Error selecting the source location.");
}

void CInvTransferDlg::OnSelChosenLocationTo(long nRow) 
{
	try {

		if(nRow == -1) {
			m_LocationTo->CurSel = 0;
		}

		// (j.jones 2008-02-28 10:40) - PLID 28080 - converted item_sql into GetInventoryItemSql()
		// (j.armen 2012-01-04 10:33) - PLID 29253 - Parameratized GetInventoryItemSql()
		_RecordsetPtr rsNumInStock = CreateParamRecordset(InvUtils::GetInventoryItemSql(m_nProductID, VarLong(m_LocationTo->GetValue(m_LocationTo->CurSel, 0))));
		double dblNumInStock = AdoFldDouble(rsNumInStock, "Actual");
		if(dblNumInStock < 0.0)
			dblNumInStock = 0.0;
		CString str;
		str.Format("%g",dblNumInStock);
		SetDlgItemText(IDC_STOCK_TO,str);

		if(m_bUseUU) {
			double ActualUO = AdoFldDouble(rsNumInStock, "ActualUO",0.0);
			CString strActualUO;
			strActualUO.Format("%g",ActualUO);
			SetDlgItemText(IDC_STOCK_UO_TO,strActualUO);
		}

		//do not set the transfer amount to zero
		OnKillfocusTransferAmt();
	}NxCatchAll("Error selecting the destination location.");
}

BOOL CInvTransferDlg::TransferProductItems(double dblQty, long nSourceLocationID, long nDestLocationID)
{
	try {

		// (j.jones 2007-11-26 09:01) - PLID 28037 - ensure we account for allocated items
		BOOL bHasItemsWSerial = ReturnsRecords("SELECT ID FROM ProductItemsT WHERE SerialNum Is Not Null "
			"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"			    WHERE (Status = %li OR Status = %li) "
			"				AND ProductItemID Is Not Null) "
			"AND Deleted = 0 AND ProductID = %li ",
			InvUtils::iadsActive, InvUtils::iadsUsed, m_nProductID);

		BOOL bHasItemsWExpDate = ReturnsRecords("SELECT ID FROM ProductItemsT WHERE ExpDate Is Not Null "
			"AND ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"			    WHERE (Status = %li OR Status = %li) "
			"				AND ProductItemID Is Not Null) "
			"AND Deleted = 0 AND ProductID = %li ",
			InvUtils::iadsActive, InvUtils::iadsUsed, m_nProductID);
		
		BOOL bHasSerial = FALSE, bHasExpDate = FALSE;

		_RecordsetPtr rs = CreateRecordset("SELECT HasSerialNum, HasExpDate FROM ProductT WHERE ID = %li AND (HasSerialNum = 1 OR HasExpDate = 1)",m_nProductID);
		if(!rs->eof) {

			bHasSerial = AdoFldBool(rs, "HasSerialNum",FALSE);
			bHasExpDate = AdoFldBool(rs, "HasExpDate",FALSE);
		}
		rs->Close();

		if(!bHasSerial && !bHasExpDate && !bHasItemsWSerial && !bHasItemsWExpDate)
			//if no applicable products, return true and move on
			return TRUE;

		if((long)dblQty != dblQty) {
			AfxMessageBox("This product is being tracked by either a serial number or expiration date,\n"
						"and requires that it is adjusted in increments of 1.\n"
						"Please enter a whole number for the quantity to adjust.");
			return FALSE;
		}

		// (d.thompson 2009-10-21) - PLID 36015 - Create your own dialog, don't
		//	use the shared one anymore.
		//CProductItemsDlg& dlg = GetMainFrame()->GetProductItemsDlg();
		CProductItemsDlg dlg(this);

		dlg.m_EntryType = PI_SELECT_DATA;
		dlg.m_CountOfItemsNeeded = (long)dblQty;

		dlg.m_bUseSerial = bHasSerial;
		dlg.m_bUseExpDate = bHasExpDate;
		dlg.m_ProductID = m_nProductID;
		dlg.m_nLocationID = nSourceLocationID;
		//TES 6/18/2008 - PLID 29578 - This now takes an OrderDetailID rather than an OrderID
		dlg.m_nOrderDetailID = -1;
		dlg.m_bDisallowQtyChange = TRUE;
		dlg.m_bAllowQtyGrow = FALSE;
		dlg.m_bIsTransfer = TRUE;
		dlg.m_bDisallowLocationChange = FALSE; // (c.haag 2008-06-25 12:12) - PLID 28438 - Ensure the location column is not read-only
		dlg.m_bSaveDataEntryQuery = false;	// (j.jones 2009-04-01 09:39) - PLID 33559 - make sure this is set to false, since we are borrowing an existing dialog from mainframe
		dlg.m_strSavedDataEntryQuery = ""; // (j.jones 2009-07-09 17:59) - PLID 34842 - make sure this gets cleared!

		if(IDCANCEL == dlg.DoModal()) {
			//JMJ - 6/24/2003 - Even if a product does not require this information, WE require them to use these items up first.
			//if they have a problem with that, they will have to adjust off those items and re-add them.
			//That sounds cocky, but Meikin and I discussed that this is the best way. The bill follows the same logic.
			AfxMessageBox("This product cannot be transferred without identifying which items are being moved to the new location.\n"
				"You will not be permitted to complete this transfer without updating this information.");
			return FALSE;
		}
		else {
			//put the selected items into the new location
			for(int i=0;i<dlg.m_adwProductItemIDs.GetSize();i++) {								
				long ProductItemID = (long)dlg.m_adwProductItemIDs.GetAt(i);
				ExecuteSql("UPDATE ProductItemsT SET LocationID = %li WHERE ID = %li", nDestLocationID, ProductItemID);
			}
		}

		return TRUE;

	}NxCatchAll("Error in TransferProductItems"); 

	return FALSE;
}

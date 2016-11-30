// ProductItemTransferDlg.cpp : implementation file
//

#include "stdafx.h"
#include "invutils.h"
#include "GlobalDataUtils.h"
#include "GlobalDrawingUtils.h"
#include "ProductItemTransferDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define COLUMN_INDEX			0
#define COLUMN_SERIAL_NUMBER	1
#define COLUMN_EXP_DATE			2
#define COLUMN_LOCATION_ID		3

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CProductItemTransferDlg dialog


CProductItemTransferDlg::CProductItemTransferDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CProductItemTransferDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProductItemTransferDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CProductItemTransferDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProductItemTransferDlg)
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE, m_btnUnselectOne);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL, m_btnUnselectAll);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE, m_btnSelectOne);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL, m_btnSelectAll);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_APPLY_LOC, m_btnApplyLoc);
	DDX_Control(pDX, IDC_TRANSFER_WARNING, m_nxsTransferWarning);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProductItemTransferDlg, CNxDialog)
	//{{AFX_MSG_MAP(CProductItemTransferDlg)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE, OnBtnSelectOne)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL, OnBtnSelectAll)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE, OnBtnUnselectOne)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL, OnBtnUnselectAll)
	ON_BN_CLICKED(IDC_BTN_APPLY_LOC, OnBtnApplyLoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProductItemTransferDlg message handlers

BOOL CProductItemTransferDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-04-29 11:31) - PLID 29820 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnApplyLoc.AutoSet(NXB_MODIFY);

		m_brush.CreateSolidBrush(PaletteColor(0x00FFDBDB));

		m_btnSelectOne.SetIcon(IDI_DARROW);
		m_btnSelectAll.SetIcon(IDI_DDARROW);
		m_btnUnselectOne.SetIcon(IDI_UARROW);
		m_btnUnselectAll.SetIcon(IDI_UUARROW);
		
		m_UnselectedList = BindNxDataListCtrl(this, IDC_UNSELECTED_ITEM_LIST, GetRemoteData(), false);
		m_SelectedList = BindNxDataListCtrl(this, IDC_SELECTED_ITEM_LIST, GetRemoteData(), true);
		m_LocationCombo = BindNxDataListCtrl(this, IDC_NEW_LOCATION_COMBO, GetRemoteData(), true);

		SetColumnWidths();

		CString str;
		// (j.jones 2007-11-26 09:01) - PLID 28037 - ensure we account for allocated items
		str.Format("ID NOT IN (SELECT ProductItemID FROM ChargedProductItemsT) "
			"AND ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"			    WHERE (Status = %li OR Status = %li) "
			"				AND ProductItemID Is Not Null) "
			"AND Deleted = 0 AND ProductID = %li", InvUtils::iadsActive, InvUtils::iadsUsed, m_ProductID);
		m_UnselectedList->PutWhereClause(_bstr_t(str));
		m_UnselectedList->Requery();

		IRowSettingsPtr pRow = m_LocationCombo->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t(" <No Location>"));
		m_LocationCombo->InsertRow(pRow,0);

		m_LocationCombo->SetSelByColumn(0, GetCurrentLocationID());

		_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ServiceT WHERE ID = %li",m_ProductID);
		if(!rs->eof) {
			CString strName = AdoFldString(rs, "Name","");
			if(!strName.IsEmpty()) {
				str.Format("Transfer Serial Numbered / Expirable Items for '%s'",strName);
				SetWindowText(str);
			}
		}
		rs->Close();

		//TES 6/16/2008 - PLID 27973 - There's not really any reason for them ever to use this dialog, make sure the caption
		// reflects that.
		m_nxsTransferWarning.SetWindowText("WARNING: This process should only be used to reconcile incorrect item locations, "
			"under the direct guidance of NexTech Technical Support.  If you have not been specifically instructed to use "
			"this process, cancel this screen and use the 'Transfer' option instead.");
	}
	NxCatchAll("Error in CProductItemTransferDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProductItemTransferDlg::OnOK() 
{
	if(!Save())
		return;
	
	CNxDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CProductItemTransferDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CProductItemTransferDlg)
	ON_EVENT(CProductItemTransferDlg, IDC_UNSELECTED_ITEM_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedItemList, VTS_I4 VTS_I2)
	ON_EVENT(CProductItemTransferDlg, IDC_SELECTED_ITEM_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedItemList, VTS_I4 VTS_I2)
	ON_EVENT(CProductItemTransferDlg, IDC_UNSELECTED_ITEM_LIST, 18 /* RequeryFinished */, OnRequeryFinishedUnselectedItemList, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CProductItemTransferDlg::OnDblClickCellUnselectedItemList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex != -1) {
		m_UnselectedList->CurSel = nRowIndex;
		OnBtnSelectOne();
	}
}

void CProductItemTransferDlg::OnDblClickCellSelectedItemList(long nRowIndex, short nColIndex) 
{
	if(nRowIndex != -1) {
		m_SelectedList->CurSel = nRowIndex;
		OnBtnUnselectOne();
	}
}

void CProductItemTransferDlg::OnBtnSelectOne()
{
	m_SelectedList->TakeCurrentRow(m_UnselectedList);
	m_SelectedList->Sort();
}

void CProductItemTransferDlg::OnBtnSelectAll() 
{
	m_SelectedList->TakeAllRows(m_UnselectedList);	
	m_SelectedList->Sort();
}

void CProductItemTransferDlg::OnBtnUnselectOne() 
{
	m_UnselectedList->TakeCurrentRow(m_SelectedList);
	m_UnselectedList->Sort();
}

void CProductItemTransferDlg::OnBtnUnselectAll() 
{
	m_UnselectedList->TakeAllRows(m_SelectedList);
	m_UnselectedList->Sort();
}

void CProductItemTransferDlg::OnRequeryFinishedUnselectedItemList(short nFlags) 
{
	if(GetRemotePropertyInt("GrayOutLocationlessItems",0,0,"<None>",TRUE) == 1) {

		for(int i=0;i<m_UnselectedList->GetRowCount();i++) {
			if(VarLong(m_UnselectedList->GetValue(i,COLUMN_LOCATION_ID),-1) == -1) {
				IRowSettingsPtr(m_UnselectedList->GetRow(i))->PutForeColor(RGB(96,96,96));
			}
		}
	}
}

void CProductItemTransferDlg::SetColumnWidths() {

	//only show the columns that we are using
	
	//use serial number and exp. date
	if(m_bUseSerial && m_bUseExpDate) {
		m_UnselectedList->GetColumn(COLUMN_INDEX)->PutStoredWidth(0);
		m_UnselectedList->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(40);
		m_UnselectedList->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(25);
		m_UnselectedList->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(35);
		m_SelectedList->GetColumn(COLUMN_INDEX)->PutStoredWidth(0);
		m_SelectedList->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(40);
		m_SelectedList->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(25);
		m_SelectedList->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(35);
	}
	//use only exp. date.
	else if(!m_bUseSerial && m_bUseExpDate) {
		m_UnselectedList->GetColumn(COLUMN_INDEX)->PutStoredWidth(0);
		m_UnselectedList->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(0);
		m_UnselectedList->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(40);
		m_UnselectedList->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(60);
		m_SelectedList->GetColumn(COLUMN_INDEX)->PutStoredWidth(0);
		m_SelectedList->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(0);
		m_SelectedList->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(40);
		m_SelectedList->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(60);
	}
	//use only serial number
	else if(m_bUseSerial && !m_bUseExpDate) {
		m_UnselectedList->GetColumn(COLUMN_INDEX)->PutStoredWidth(0);
		m_UnselectedList->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(60);
		m_UnselectedList->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(0);
		m_UnselectedList->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(40);
		m_SelectedList->GetColumn(COLUMN_INDEX)->PutStoredWidth(0);
		m_SelectedList->GetColumn(COLUMN_SERIAL_NUMBER)->PutStoredWidth(60);
		m_SelectedList->GetColumn(COLUMN_EXP_DATE)->PutStoredWidth(0);
		m_SelectedList->GetColumn(COLUMN_LOCATION_ID)->PutStoredWidth(40);
	}
}

void CProductItemTransferDlg::OnBtnApplyLoc() 
{
	try {

		if(m_SelectedList->GetRowCount() == 0) {
			AfxMessageBox("Please place items in the 'Selected List' before clicking 'Apply'.");
			return;
		}

		if(m_LocationCombo->CurSel == -1) {
			AfxMessageBox("Please select a location from the list before clicking 'Apply'.");
			return;
		}

		long nLocID = VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0),-1);

		if(nLocID == -1) {
			if(IDNO == MessageBox("If you mark items as having 'No Location', you will only be able to bill these items\n"
								  "in relation to what the On Hand amount is for your bill location, however you will see\n"
								  "both that location's items and those with 'No Location'.\n\n"
								  "Are you sure you wish to mark these items as having 'No Location'?","Practice",MB_YESNO|MB_ICONQUESTION)) {
				return;
			}
		}

		//if we're here, we can safely update the locations of the items in the selected list
		for(int i=0;i<m_SelectedList->GetRowCount();i++) {
			m_SelectedList->PutValue(i,COLUMN_LOCATION_ID,(long)nLocID);

			if(nLocID == -1 && GetRemotePropertyInt("GrayOutLocationlessItems",0,0,"<None>",TRUE) == 1) {
				IRowSettingsPtr(m_SelectedList->GetRow(i))->PutForeColor(RGB(96,96,96));
			}
			else {
				IRowSettingsPtr(m_SelectedList->GetRow(i))->PutForeColor(RGB(0,0,0));
			}
		}

	}NxCatchAll("Error applying locations.");
}

BOOL CProductItemTransferDlg::Save()
{
	try {

		//first compare the numbers for each location

		CString str, strWarning;
		_RecordsetPtr rs = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1");
		while(!rs->eof) {

			long nLocID = AdoFldLong(rs, "ID");
			CString strLocName = AdoFldString(rs, "Name","");

			long nQtySetToLoc = 0;

			double dblAmtOnHand = 0.0;
			double dblAllocated = 0.0;

			// (j.jones 2007-12-18 11:44) - PLID 28037 - CalcAmtOnHand changed to return allocation information,
			// which for now is unused in this code
			InvUtils::CalcAmtOnHand(m_ProductID, nLocID, dblAmtOnHand, dblAllocated);

			for(int i=0;i<m_UnselectedList->GetRowCount();i++) {
				long nLocIDToCompare = VarLong(m_UnselectedList->GetValue(i,COLUMN_LOCATION_ID),-1);
				if(nLocID == nLocIDToCompare)
					nQtySetToLoc++;
			}

			for(i=0;i<m_SelectedList->GetRowCount();i++) {
				long nLocIDToCompare = VarLong(m_SelectedList->GetValue(i,COLUMN_LOCATION_ID),-1);
				if(nLocID == nLocIDToCompare)
					nQtySetToLoc++;
			}

			//Warn if the amount of items configured to go into this location match the amount needed,
			//unless we default items to no location. If we do have that default, and "some" of the items are set
			//to go to our location, well, warn that too, because that's plain weird.
			// (c.haag 2008-07-01 17:32) - PLID 30594 - "Default Serialized Items To No Location" preference has been deprecated
			// We want to treat the preference as always returning 0, so the statement consolidates to
			// "...&& (0 != 1 || nQtySetToLoc > 0)" = "1 || nQtySetToLoc > 0" = 1 = always true.
			if(nQtySetToLoc != (long)dblAmtOnHand /*&& (GetRemotePropertyInt("DefaultSerializedItemsToNoLocation",0,0,"<None>",TRUE) != 1
				|| nQtySetToLoc > 0)*/) {

				CString strAllocated = "";
				if(dblAllocated > 0.0) {
					strAllocated.Format(" (%g allocated to patients)", dblAllocated);
				}

				CString str;
				str.Format("- There are %li items%s at %s but you have configured %li to be assigned to this location.\n\n",
					(long)dblAmtOnHand, strAllocated, strLocName, nQtySetToLoc);

				strWarning += str;
			}

			rs->MoveNext();
		}
		rs->Close();		

		if(!strWarning.IsEmpty()) {

			strWarning = "The location assignments for this product have triggered the following warnings:\n\n" + strWarning + 
						 "When billing this product, you will only be able to choose serial numbered / expiration date items\n"
						 "that are associated with the location of the bill, and items unassociated with any location.\n\n"
						 "Are you sure you wish to save these items to these locations?";

			if(IDNO == MessageBox(strWarning,"Practice",MB_YESNO|MB_ICONQUESTION)) {
				return FALSE;
			}
		}

		//if we got here, we're ready to save, so update the location on each item in each list
		for(int i=0;i<m_UnselectedList->GetRowCount();i++) {
			long nID = VarLong(m_UnselectedList->GetValue(i,COLUMN_INDEX),-1);
			long nLocID = VarLong(m_UnselectedList->GetValue(i,COLUMN_LOCATION_ID),-1);

			CString strLocation = "NULL";
			if(nLocID != -1)
				strLocation.Format("%li",nLocID);

			ExecuteSql("UPDATE ProductItemsT SET LocationID = %s WHERE ID = %li",strLocation,nID);
		}

		for(i=0;i<m_SelectedList->GetRowCount();i++) {
			long nID = VarLong(m_SelectedList->GetValue(i,COLUMN_INDEX),-1);
			long nLocID = VarLong(m_SelectedList->GetValue(i,COLUMN_LOCATION_ID),-1);

			CString strLocation = "NULL";
			if(nLocID != -1)
				strLocation.Format("%li",nLocID);

			ExecuteSql("UPDATE ProductItemsT SET LocationID = %s WHERE ID = %li",strLocation,nID);
		}

		return TRUE;

	}NxCatchAll("Error saving location assignments.");

	return FALSE;
}
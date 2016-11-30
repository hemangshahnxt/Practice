// InvNew.cpp : implementation file
//
#include "stdafx.h"
#include "InvNew.h"
#include "AuditTrail.h"
#include "DateTimeUtils.h"
#include "CategorySelectDlg.h"
#include "InventoryRc.h"
#include "InvUtils.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CInvNew dialog

CInvNew::CInvNew()
	: CNxDialog(CInvNew::IDD, NULL)
{
	//{{AFX_DATA_INIT(CInvNew)
	m_nDefaultCategoryID = -1;
	//}}AFX_DATA_INIT
}


void CInvNew::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInvNew)
	DDX_Control(pDX, IDC_PREV_LOC, m_prevLocationBtn);
	DDX_Control(pDX, IDC_NEXT_LOC, m_nextLocationBtn);
	DDX_Control(pDX, IDC_RADIO_NOT_TRACKABLE_ITEM, m_radioNotTrackable);
	DDX_Control(pDX, IDC_RADIO_TRACK_ORDERS_ITEM, m_radioTrackOrders);
	DDX_Control(pDX, IDC_RADIO_TRACK_QUANTITY_ITEM, m_radioTrackQuantity);
	DDX_Control(pDX, IDC_TAXABLE2, m_taxable2);
	DDX_Control(pDX, IDOK, m_okBtn);
	DDX_Control(pDX, IDCANCEL, m_cancelBtn);
	DDX_Control(pDX, IDC_TAXABLE, m_taxable);
	DDX_Control(pDX, IDC_BILLABLE, m_billable);
	DDX_Control(pDX, IDC_NEW_ITEM_LOC_LABEL, m_location);
	DDX_Control(pDX, IDC_BTN_CATEGORY_PICKER, m_btnPickCategory);
	DDX_Control(pDX, IDC_BTN_CATEGORY_REMOVE, m_btnRemoveCategory);
	DDX_Control(pDX, IDC_NAME, m_nxeditName);
	DDX_Control(pDX, IDC_ACTUAL, m_nxeditActual);
	DDX_Control(pDX, IDC_CATEGORY, m_nxeditCategory);
	DDX_Control(pDX, IDC_ACTUAL_TEXT, m_nxstaticActualText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvNew, CNxDialog)
	//{{AFX_MSG_MAP(CInvNew)
	ON_BN_CLICKED(IDC_BILLABLE, OnChangeBillable)
	ON_BN_CLICKED(IDC_TRACKABLE, OnChangeTrackable)
	ON_BN_CLICKED(IDC_RADIO_NOT_TRACKABLE_ITEM, OnRadioNotTrackableItem)
	ON_BN_CLICKED(IDC_RADIO_TRACK_ORDERS_ITEM, OnRadioTrackOrdersItem)
	ON_BN_CLICKED(IDC_RADIO_TRACK_QUANTITY_ITEM, OnRadioTrackQuantityItem)
	ON_BN_CLICKED(IDC_PREV_LOC, OnPreviousLocation)
	ON_BN_CLICKED(IDC_NEXT_LOC, OnNextLocation)
	ON_BN_CLICKED(IDC_BTN_CATEGORY_PICKER, OnCategoryPicker)
	ON_BN_CLICKED(IDC_BTN_CATEGORY_REMOVE, OnCategoryRemove)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CInvNew, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInvNew)
	ON_EVENT(CInvNew, IDC_NEW_ITEM_LOCATION, 20 /* TrySetSelFinished */, OnTrySetSelFinishedLocation, VTS_I4 VTS_I4)
	ON_EVENT(CInvNew, IDC_NEW_ITEM_LOCATION, 2 /* SelChanged */, OnSelChangedLocation, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CInvNew, IDC_NEW_ITEM_LOCATION, 18 /* RequeryFinished */, OnRequeryFinishedLocation, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvNew message handlers

BOOL CInvNew::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_okBtn.AutoSet(NXB_OK);
		m_cancelBtn.AutoSet(NXB_CANCEL);
		m_prevLocationBtn.AutoSet(NXB_LEFT);
		m_nextLocationBtn.AutoSet(NXB_RIGHT);
		// (c.haag 2008-05-20 12:25) - PLID 29820 - NxIconify more buttons
		m_btnPickCategory.AutoSet(NXB_MODIFY);
		m_btnRemoveCategory.AutoSet(NXB_DELETE);
		
		m_locationMap.RemoveAll();
		m_pLocationCombo = BindNxDataList2Ctrl(IDC_NEW_ITEM_LOCATION);

		//(e.lally 2007-02-26) PLID 24912 - Using bulk caching to optimize our DB accesses.
		// (j.armen 2011-10-25 10:59) - PLID 46137 - GetPracPath is referencing ConfigRT
		g_propManager.CachePropertiesInBulk("NewInventoryItem", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'NewItemTaxable' OR "
			"Name = 'NewItemTaxable2' OR "
			"Name = 'NewItemBillable' OR "
			"Name = 'NewItemTrackable' OR "
			"Name = 'DefaultProductOrderingUser' "
			")",
			_Q(g_propManager.GetSystemName() + '.' + GetPracPath(PracPath::ConfigRT)));

		m_taxable.SetCheck(GetPropertyInt("NewItemTaxable"));
		m_taxable2.SetCheck(GetPropertyInt("NewItemTaxable2"));
		m_billable.SetCheck(GetPropertyInt("NewItemBillable"));
		SetTrackableStatus(GetPropertyInt("NewItemTrackable"));

		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_pLocationCombo->TrySetSelByColumn_Deprecated(0, GetCurrentLocation());
		m_bVisitedMultiple = FALSE;

		OnChangeBillable();
		OnChangeTrackable();

		// (j.jones 2015-03-03 14:17) - PLID 64965 - we may have been given a default category
		CString strCategoryNames;
		if (m_aryCategoryIDs.size() > 0) {
			LoadServiceCategories(m_aryCategoryIDs, m_nDefaultCategoryID, strCategoryNames);
		}

		SetDlgItemText(IDC_CATEGORY, strCategoryNames);
		m_btnRemoveCategory.EnableWindow(m_aryCategoryIDs.size() > 0);

		GetDlgItem(IDC_NAME)->SetFocus();
	}
	NxCatchAll("Error in CInvNew::OnInitDialog");
	return FALSE;  
}

void CInvNew::OnOK()
{
	CString			name;
	_RecordsetPtr	rs;
	
	try
	{
		
		{ //Be sure to save our current information into the map for the selected location
			long nCurSelLoc = VarLong(m_pLocationCombo->GetCurSel()->GetValue(0), -1);
			LocationRecord lrCurLoc;
			double dblStock;
			CString strStock;
			GetDlgItemText(IDC_ACTUAL,strStock);
			dblStock = atof(strStock);
			lrCurLoc.dblOnHandQty = dblStock;

			m_locationMap.SetAt(nCurSelLoc, lrCurLoc);
		}

		// (b.cardillo 2005-06-30 13:14) - PLID 16691 - We allow the caller to give the text of a warning 
		// message to prompt the user with if the user didn't make this item billable.  Right now this is 
		// only used for this pl item, but it's a general feature of CInvNew that could be used anywhere.
		if (m_billable.GetCheck() == 0 && !m_strWarnIfNotBillable.IsEmpty()) {
			// Warn if not billable, giving the user an opportunity to cancel
			if (MessageBox(m_strWarnIfNotBillable, NULL, MB_OKCANCEL|MB_ICONWARNING) != IDOK) {
				return;
			}
		}

		EnsureRemoteData();

		GetDlgItemText (IDC_NAME, name);
		if (name == "")
		{	AfxMessageBox ("You must enter a name.");
			return;
		}
		// (a.walling 2007-09-27 08:53) - PLID 27530 - We are passing a user string directly to CreateRecordset;
		// if it contains % signs then they will try to be formatted. We need to do ("%s", "string") in this case,
		// or even better just call the non-formatting version, CreateRecordsetStd.
		rs = CreateRecordsetStd("SELECT COUNT (Name) AS Num "
			"FROM ServiceT WHERE Name = \'" + _Q(name) + "\';");
		if (rs->Fields->Item["num"]->Value.lVal)
		{	rs->Close();
			// (j.politis 2015-07-08 14:48) - PLID 66481 - You can't enter a product name that is also a service code name.
			AfxMessageBox ("A service code or product already exists with this name. (It may be marked inactive.)");
			return;
		}
		rs->Close();
		TrackStatus tsTrackStatus = GetEnumTrackableStatus();
		if(m_bVisitedMultiple == FALSE && m_pLocationCombo->GetRowCount() >1 && tsTrackStatus == tsTrackQuantity){
			//They have track quantity selected but didn't configure any of the other locations. 
				//We should prompt the user to make sure this is ok.
			int result = MessageBox("You have only configured the On Hand amount for the current location. "
				"All other locations will have a default amount of 0."
				, NULL, MB_OKCANCEL);
			if(result == IDCANCEL)
				return;
		}
		POSITION pos = m_locationMap.GetStartPosition();
		long nLocationID =-1;
		LocationRecord lrLocation;
		BOOL bLocBillable = m_billable.GetCheck();
		double dblInStock = 0;
		
		long nUserID = GetRemotePropertyInt("DefaultProductOrderingUser", -1, 0, "<None>");
		// (j.jones 2005-01-31 17:22) - the user could have been deleted or inactivated
		if(nUserID == -1 || IsRecordsetEmpty("SELECT ID FROM PersonT INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID WHERE Archived = 0 AND ID = %li",nUserID)) {
			nUserID = GetCurrentUserID();
		}

		//(e.lally 2007-02-26) PLID 24912 - Put our insert statements in a batch as an optimization
		CString strSql = BeginSqlBatch();
		AddStatementToSqlBatch(strSql, "SET NOCOUNT ON");
		AddDeclarationToSqlBatch(strSql, "DECLARE @nNewProductID int");
		AddStatementToSqlBatch(strSql, "SET @nNewProductID = (SELECT COALESCE(MAX(ID), 0) + 1 FROM ServiceT)");
		//(e.lally 2007-02-26) PLID 23258 - These queries are only run once so they can be pulled out of the loop.
		// (z.manning 2010-06-23 12:36) - PLID 39311 - Moved the code to create a product to InvUtils
		InvUtils::AddCreateProductSqlToBatch(strSql, "@nNewProductID", name, bLocBillable, m_taxable.GetCheck() == BST_CHECKED, m_taxable2.GetCheck() == BST_CHECKED, GetEnumTrackableStatus(), nUserID);

		//If Track Quantity is selected then for each location in our map, enter the On Hand qty
		if(tsTrackStatus == tsTrackQuantity){
			while(pos){
				m_locationMap.GetNextAssoc(pos, nLocationID, lrLocation);
				dblInStock = lrLocation.dblOnHandQty;
				
				if (dblInStock != 0.0)
				{
					//(e.lally 2007-02-26) PLID 24912 - Calculate the adjustmentID as we execute for an optimization
					//TES 6/26/2008 - PLID 30522 - Assign this to the system "Initial Adjustment" category
					AddStatementToSqlBatch(strSql, "INSERT INTO ProductAdjustmentsT "
					"(ID, ProductID, Date, Login, Quantity, LocationID, ProductAdjustmentCategoryID) "
					"SELECT (SELECT COALESCE(MAX(ID),0)+1 FROM ProductAdjustmentsT), "
					"@nNewProductID, \'%s\', %i, %g, %i, %li", 
					FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), 
					GetCurrentUserID(), dblInStock, nLocationID, InvUtils::g_nInitialAdjustmentID);
				}

			}//end while loop
		}

		AddStatementToSqlBatch(strSql, "SET NOCOUNT OFF");
		AddStatementToSqlBatch(strSql, "SELECT @nNewProductID AS NewProductID");
		_RecordsetPtr prsCreateProduct = CreateRecordset(
			"BEGIN TRAN \r\n"
			"%s \r\n"
			"COMMIT TRAN \r\n"
			, strSql);

		long nNewProductID = -1;
		if(!prsCreateProduct->eof) {
			nNewProductID = AdoFldLong(prsCreateProduct->GetFields(), "NewProductID");
		}
		else {
			ThrowNxException("CInvNew::OnOK - eof error when trying to get new product ID");
		}

		//(e.lally 2007-02-26) PLID 24912 - Moved auditing to outside of batch.
		//auditing - only do once
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiProductCreate, nNewProductID, "", name, aepMedium, aetCreated);
		
		// (j.jones 2015-03-03 13:35) - PLID 64965 - save the product categories via the API, if they picked any
		if (m_aryCategoryIDs.size() > 0) {
			std::vector<long> aryServiceIDs;
			aryServiceIDs.push_back(nNewProductID);
			UpdateServiceCategories(aryServiceIDs, m_aryCategoryIDs, m_nDefaultCategoryID, true);
		}

		// (b.eyers 2015-07-10) - PLID 24060 - newly created product needs to send a tablechecker
		CClient::RefreshTable(NetUtils::Products, nNewProductID);

		SetPropertyInt("NewItemTaxable", m_taxable.GetCheck());
		SetPropertyInt("NewItemTaxable2", m_taxable2.GetCheck());
		SetPropertyInt("NewItemTrackable", GetIntTrackableStatus());
		SetPropertyInt("NewItemBillable", m_billable.GetCheck());
		m_strFinalName = name;
		m_bFinalBillable = m_billable.GetCheck() == 0 ? FALSE : TRUE;
		EndDialog(nNewProductID);
		return;
	}
	NxCatchAll("Could not save item");

	// (a.walling 2010-09-08 13:26) - PLID 40377 - There is no transaction active for this to rollback!!
	//RollbackTrans("NewInvItem");
}

void CInvNew::OnCancel()
{
	SetPropertyInt("NewItemTaxable", m_taxable.GetCheck());
	SetPropertyInt("NewItemTaxable2", m_taxable2.GetCheck());
	SetPropertyInt("NewItemTrackable", GetIntTrackableStatus());
	SetPropertyInt("NewItemBillable", m_billable.GetCheck());

	EndDialog(0);
}

void CInvNew::OnChangeBillable() 
{
	BOOL bShow;
	if (m_billable.GetCheck())
		bShow = TRUE;
	else bShow = FALSE;
	m_taxable.EnableWindow(bShow);
	m_taxable2.EnableWindow(bShow);	
}

void CInvNew::OnChangeTrackable() 
{
	DWORD show;
	if (m_radioTrackQuantity.GetCheck())
		show = SW_SHOW;
	else show = SW_HIDE;
	GetDlgItem(IDC_ACTUAL)->ShowWindow(show);
	GetDlgItem(IDC_ACTUAL_TEXT)->ShowWindow(show);
	m_prevLocationBtn.ShowWindow(show);
	m_nextLocationBtn.ShowWindow(show);
	m_location.ShowWindow(show);
	GetDlgItem(IDC_NEW_ITEM_LOCATION)->ShowWindow(show);
}

int CInvNew::DoModal(long category)
{
	// (j.jones 2015-03-03 14:16) - PLID 64965 - if a default category was given, track it
	if (category != -1) {
		m_aryCategoryIDs.clear();
		m_aryCategoryIDs.push_back(category);
		m_nDefaultCategoryID = category;
	}
	return CNxDialog::DoModal();
}

void CInvNew::OnRadioNotTrackableItem() 
{
	OnChangeTrackable();
}

void CInvNew::OnRadioTrackOrdersItem() 
{
	OnChangeTrackable();
}

void CInvNew::OnRadioTrackQuantityItem() 
{
	OnChangeTrackable();
}

int CInvNew::GetIntTrackableStatus()
{
	int TrackableStatus = 0;

	if(m_radioNotTrackable.GetCheck()) {
		TrackableStatus = 0;
	}
	if(m_radioTrackOrders.GetCheck()) {
		TrackableStatus = 1;
	}
	if(m_radioTrackQuantity.GetCheck()) {
		TrackableStatus = 2;
	}

	return TrackableStatus;
}

TrackStatus CInvNew::GetEnumTrackableStatus()
{
	TrackStatus TrackableStatus = tsNotTrackable;

	if(m_radioNotTrackable.GetCheck()) {
		TrackableStatus = tsNotTrackable;
	}
	if(m_radioTrackOrders.GetCheck()) {
		TrackableStatus = tsTrackOrders;
	}
	if(m_radioTrackQuantity.GetCheck()) {
		TrackableStatus = tsTrackQuantity;
	}

	return TrackableStatus;
}

void CInvNew::UpdateArrows()
{
	//(e.lally 2007-02-26) PLID 23258 - Reflect arrow statuses for selected location
	try{
		//Get the currently selected row
		IRowSettingsPtr pCurRow = m_pLocationCombo->GetCurSel();
		if(m_pLocationCombo->GetRowCount() == 0){
			//This should not be possible. We have no locations in the list.
			ASSERT(FALSE);
			m_nextLocationBtn.EnableWindow(FALSE);
			m_prevLocationBtn.EnableWindow(FALSE);
			return;
		}
		if(pCurRow == NULL) {
			//Yikes, we have no current selection!
			ASSERT(FALSE);
			//Let's just select the first row since we know there is one.
			m_pLocationCombo->PutCurSel(m_pLocationCombo->GetFirstRow());
			HandleSelChangedLocation(-1);
			return;
		}
		if(pCurRow->IsSameRow(m_pLocationCombo->GetLastRow())){
			//We're on the last row, disable the next arrow
			m_nextLocationBtn.EnableWindow(FALSE);
		}
		else{
			//We're not on the last row, enable the next arrow
			m_nextLocationBtn.EnableWindow(TRUE);
		}
		if(pCurRow->IsSameRow(m_pLocationCombo->GetFirstRow())){
			//We're on the first row, disable the previous arrow
			m_prevLocationBtn.EnableWindow(FALSE);
		}
		else{
			//We're not on the first row, enable the previous arrow
			m_prevLocationBtn.EnableWindow(TRUE);
		}

	}NxCatchAll("CInvNew::Error in UpdateArrows");
}

void CInvNew::OnPreviousLocation() 
{
	//(e.lally 2007-02-26) PLID 23258 - Switch to previous location in the list
	try{
		//Get our current row
		IRowSettingsPtr pCurRow = m_pLocationCombo->GetCurSel();

		if(pCurRow ==NULL){
			//No current row!
			ASSERT(FALSE);
			//Silently return
			return;
		}
		else if(pCurRow->IsSameRow(m_pLocationCombo->GetFirstRow())){
			//We're on the first row! This button should not have been enabled!!
			ASSERT(FALSE);
			//We'll leave the button enabled so it does not appear like we changed locations
			//Silently return
			return;
		}

		//Put our selection on the previous row
		long nCurSelLocationID = VarLong(pCurRow->GetValue(0), -1);
		IRowSettingsPtr pPreviousRow = pCurRow->GetPreviousRow();
		m_pLocationCombo->PutCurSel(pPreviousRow);

		//Since the PutCurSel won't fire the OnSelChanged event, we need to call the handling of it manually
		HandleSelChangedLocation(nCurSelLocationID);

	}NxCatchAll("Error in OnPreviousLocation");
	
}

void CInvNew::OnNextLocation() 
{
	//(e.lally 2007-02-26) PLID 23258 - Switch to next location in the list
	try{
		//Get our current row
		IRowSettingsPtr pCurRow = m_pLocationCombo->GetCurSel();

		if(pCurRow ==NULL){
			//No current row!
			ASSERT(FALSE);
			//Silently return
			return;
		}
		else if(pCurRow->IsSameRow(m_pLocationCombo->GetLastRow())){
			//We're on the last row! This button should not have been enabled!!
			ASSERT(FALSE);
			//We'll leave the button enabled so it does not appear like we changed locations
			//Silently return
			return;
		}
		//Put our selection on the previous row
		long nCurSelLocationID = VarLong(pCurRow->GetValue(0), -1);
		IRowSettingsPtr pNextRow = pCurRow->GetNextRow();
		m_pLocationCombo->PutCurSel(pNextRow);

		//Since the PutCurSel won't fire the OnSelChanged event, we need to call the handling of it manually
		HandleSelChangedLocation(nCurSelLocationID);

	}NxCatchAll("Error in OnNextLocation");
	
}

void CInvNew::OnTrySetSelFinishedLocation(long nRowEnum, long nFlags) 
{
	//(e.lally 2007-02-26) PLID 23258 - Add ability to configure new items for each location
	try{
		UpdateArrows();
	}NxCatchAll("Error in CInvNew::OnTrySetSelFinishedLocation");
}

void CInvNew::OnSelChangedLocation(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	//(e.lally 2007-02-26) PLID 23258 - Add ability to configure new items for each location
	try{
		IRowSettingsPtr pOldSel(lpOldSel);
		long nOldLocationID = VarLong(pOldSel->GetValue(0), -1);
		if(lpNewSel == NULL){
			m_pLocationCombo->PutCurSel(m_pLocationCombo->GetFirstRow());
		}
		HandleSelChangedLocation(nOldLocationID);
	}NxCatchAll("Error in CInvNew::OnSelChangedLocation");
	
}

void CInvNew::HandleSelChangedLocation(long nOldSelLocationID)
{
	//(e.lally 2007-02-26) PLID 23258 - save and load location specific info in the cache
	try{
		m_bVisitedMultiple = TRUE;
		//Save the current info from the last location
		if(nOldSelLocationID >=0){
			LocationRecord lrOldLoc;
			double dblStock;
			CString strStock;
			GetDlgItemText(IDC_ACTUAL,strStock);
			dblStock = atof(strStock);
			lrOldLoc.dblOnHandQty = dblStock;

			m_locationMap.SetAt(nOldSelLocationID, lrOldLoc);
		}

		//Load this location's new data that has been entered into our map.
		long nLocationID = VarLong(m_pLocationCombo->GetCurSel()->GetValue(0), -1);
		LocationRecord lrData = m_locationMap[nLocationID];
		CString strStock;
		strStock.Format("%g", lrData.dblOnHandQty);
		SetDlgItemText(IDC_ACTUAL, strStock);

		//Update our arrows to reflect the change in location
		UpdateArrows();

	}NxCatchAll("Error in HandleSelChangedLocation");
}

void CInvNew::OnRequeryFinishedLocation(short nFlags) 
{
	//(e.lally 2007-02-26) PLID 23258 - Add location specific info for each location to the cache, setting defaults
	try{
		//If our map is empty, load all the locations into our map.
		if(m_locationMap.GetCount()==0){
			LocationRecord lrDefault;
			lrDefault.dblOnHandQty=0.00;

			IRowSettingsPtr pCurRow = m_pLocationCombo->GetFirstRow();
			while(pCurRow){
				m_locationMap.SetAt(VarLong(pCurRow->GetValue(0), -1), lrDefault);
				pCurRow = pCurRow->GetNextRow();
			}
		}
	}NxCatchAll("Error in OnRequeryFinishedLocation");
	
}

void CInvNew::SetTrackableStatus(int nTrackableStatus)
{
	try{
		switch(nTrackableStatus) {
		case tsNotTrackable:
			m_radioNotTrackable.SetCheck(TRUE);
			m_radioTrackOrders.SetCheck(FALSE);
			m_radioTrackQuantity.SetCheck(FALSE);
			break;
		case tsTrackOrders:
			m_radioNotTrackable.SetCheck(FALSE);
			m_radioTrackOrders.SetCheck(TRUE);
			m_radioTrackQuantity.SetCheck(FALSE);
			break;
		case tsTrackQuantity:
			m_radioNotTrackable.SetCheck(FALSE);
			m_radioTrackOrders.SetCheck(FALSE);
			m_radioTrackQuantity.SetCheck(TRUE);
			break;
		}
	}NxCatchAll("Error in CInvNew::SetTrackableStatus");
}

void CInvNew::OnCategoryPicker()
{
	//
	// (c.haag 2007-03-26 08:52) - PLID 24824 - You may now choose a category for the new
	// inventory item here
	//
	try {

		// (j.jones 2015-03-03 13:35) - PLID 64965 - products can now have multiple categories
		
		// (j.jones 2015-02-27 16:22) - PLID 64962 - added bAllowMultiSelect, true for inventory items
		// (j.jones 2015-03-02 15:36) - PLID 64970 - added strItemType
		CCategorySelectDlg dlg(this, true, "inventory item");

		// (j.jones 2015-03-02 08:55) - PLID 64962 - this dialog supports multiple categories
		if (m_aryCategoryIDs.size() > 0) {
			dlg.m_aryInitialCategoryIDs.insert(dlg.m_aryInitialCategoryIDs.end(), m_aryCategoryIDs.begin(), m_aryCategoryIDs.end());
			dlg.m_nInitialDefaultCategoryID = m_nDefaultCategoryID;
		}

		// (j.jones 2015-03-02 10:18) - PLID 64962 - this now is just an OK/Cancel dialog
		if (IDOK == dlg.DoModal()) {	// Greater than zero means the user picked a valid category

			// (j.jones 2015-03-03 13:35) - PLID 64965 - products can now have multiple categories
			m_aryCategoryIDs.clear();
			m_aryCategoryIDs.insert(m_aryCategoryIDs.end(), dlg.m_arySelectedCategoryIDs.begin(), dlg.m_arySelectedCategoryIDs.end());
			m_nDefaultCategoryID = dlg.m_nSelectedDefaultCategoryID;

			CString strCategoryNames;
			LoadServiceCategories(m_aryCategoryIDs, m_nDefaultCategoryID, strCategoryNames);

			SetDlgItemText(IDC_CATEGORY, strCategoryNames);
			m_btnRemoveCategory.EnableWindow(m_aryCategoryIDs.size() > 0);
		}
	}
	NxCatchAll("Error in CInvNew::OnCategoryPicker()");
}

void CInvNew::OnCategoryRemove()
{
	try {

		// (j.jones 2015-03-03 14:09) - PLID 64965 - products can now have multiple categories
		m_aryCategoryIDs.clear();
		m_nDefaultCategoryID = -1;
		SetDlgItemText(IDC_CATEGORY, "");
		m_btnRemoveCategory.EnableWindow(FALSE);
	}
	NxCatchAll("Error in CInvNew::OnCategoryRemove()");
}


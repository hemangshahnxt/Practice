// EditDrawersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EditDrawersDlg.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "UserDrawerDlg.h"
#include "AuditTrail.h"
#include "OPOSCashDrawerDevice.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CEditDrawersDlg dialog

enum DrawerColumns {
	dcID = 0,
	dcName,
	dcOpenDate,
	dcOpenAmt,
	dcLocation,
	dcCloseDate,
	dcCloseAmt,
};

//also defined in UserDrawerDlg.cpp
#define IDPREVIEW  17895


CEditDrawersDlg::CEditDrawersDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditDrawersDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditDrawersDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditDrawersDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditDrawersDlg)
	DDX_Control(pDX, IDC_CASH_DRAWER_CURRENT_LOCATION, m_btnCurrentLoc);
	DDX_Control(pDX, IDC_CASH_DRAWER_ALL_LOCATIONS, m_btnAllLoc);
	DDX_Control(pDX, IDC_INCLUDE_CLOSED, m_btnIncludeClosed);
	DDX_Control(pDX, IDC_NEW_DRAWER, m_btnNewDrawer);
	DDX_Control(pDX, IDC_CLOSE_DRAWER, m_btnCloseDrawer);
	DDX_Control(pDX, IDC_EDIT_DRAWER, m_btnEditDrawer);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditDrawersDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditDrawersDlg)
	ON_BN_CLICKED(IDC_NEW_DRAWER, OnNewDrawer)
	ON_BN_CLICKED(IDC_CLOSE_DRAWER, OnCloseDrawer)
	ON_BN_CLICKED(IDC_INCLUDE_CLOSED, OnIncludeClosed)
	ON_BN_CLICKED(IDC_EDIT_DRAWER, OnEditDrawer)
	ON_BN_CLICKED(IDC_CASH_DRAWER_CURRENT_LOCATION, OnCurrentLocation)
	ON_BN_CLICKED(IDC_CASH_DRAWER_ALL_LOCATIONS, OnAllLocations)
	ON_BN_CLICKED(IDC_OPEN_PHYSICAL_CASH_DRAWER, OnOpenPhysicalCashDrawer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditDrawersDlg message handlers

BOOL CEditDrawersDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (c.haag 2008-04-23 12:35) - PLID 29761 - NxIconify the buttons
		m_btnNewDrawer.AutoSet(NXB_NEW);
		m_btnCloseDrawer.AutoSet(NXB_MODIFY);
		m_btnEditDrawer.AutoSet(NXB_MODIFY);
		m_btnClose.AutoSet(NXB_CLOSE);

		m_pList = BindNxDataListCtrl(this, IDC_DRAWER_LIST, GetRemoteData(), false);

		//handles requery
		OnIncludeClosed();

		// (j.gruber 2007-08-09 11:13) - PLID 25119 - get their preference
		if (GetRemotePropertyInt("CashDrawerDefaultAllLocations", 1, 0, GetCurrentUserName(), TRUE)) {
			CheckDlgButton(IDC_CASH_DRAWER_ALL_LOCATIONS, 1);
			CheckDlgButton(IDC_CASH_DRAWER_CURRENT_LOCATION, 0);
		}
		else {
			CheckDlgButton(IDC_CASH_DRAWER_ALL_LOCATIONS, 0);
			CheckDlgButton(IDC_CASH_DRAWER_CURRENT_LOCATION, 1);
		}

		// (a.walling 2007-09-28 12:05) - PLID 27468 - Button to open the cash drawer
		if ( (GetMainFrame()->GetCashDrawerDevice() != NULL) &&
			(CheckCurrentUserPermissions(bioCashDrawers, sptDynamic0) || IsCurrentUserAdministrator()) ) {
			GetDlgItem(IDC_OPEN_PHYSICAL_CASH_DRAWER)->EnableWindow(TRUE);
		} else {
			GetDlgItem(IDC_OPEN_PHYSICAL_CASH_DRAWER)->EnableWindow(FALSE);
		}

	} NxCatchAll("Error loading dialog.");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditDrawersDlg::OnOK() 
{

	CDialog::OnOK();
}

void CEditDrawersDlg::OnNewDrawer() 
{
	if(!CheckCurrentUserPermissions(bioCashDrawers, sptCreate))
		return;

	try {
		IRowSettingsPtr pRow = m_pList->GetRow(-1);

		COleCurrency cy(0, 0);
		COleDateTime dt = COleDateTime::GetCurrentTime();
		CString strName;
		strName.Format("Drawer @ %s", FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoTime, false));

		int nRes;
		do {
			nRes = InputBoxLimited(this, "Enter a name for this drawer:", strName, "",255,false,false,NULL);

			//quit if they cancel
			if(nRes == IDCANCEL)
				return;

			strName.TrimLeft();
			strName.TrimRight();
			if(strName.IsEmpty())
				MsgBox("You cannot enter a blank name.");

		} while(nRes == IDOK && strName.IsEmpty());

		long nID = NewNumber("CashDrawersT", "ID");

		// (j.gruber 2007-08-09 10:55) - PLID 25119 - location
		long nLocationID = -1;
		if (IsDlgButtonChecked(IDC_CASH_DRAWER_CURRENT_LOCATION)) {
			//they want to use the current location
			nLocationID = GetCurrentLocationID();
		}

		CString strLocationID;
		if (nLocationID == -1) {
			strLocationID = "NULL";
		}
		else {
			strLocationID.Format("%li", nLocationID);
		}

		ExecuteSql("INSERT INTO CashDrawersT (ID, Name, DateOpen, OpenAmt, LocationID) values (%li, '%s', '%s', Convert(money, '%s'), %s)", 
			nID, _Q(strName), FormatDateTimeForSql(dt), FormatCurrencyForSql(cy), strLocationID);

		_variant_t var;
		var.vt = VT_DATE;
		var.date = dt;

		pRow->PutValue(dcID, (long)nID);
		pRow->PutValue(dcName, _bstr_t(strName));
		pRow->PutValue(dcOpenDate, var);
		pRow->PutValue(dcOpenAmt, _variant_t(cy));
		pRow->PutValue(dcLocation, nLocationID);

		m_pList->AddRow(pRow);

		// (j.anspach 05-24-2005 14:04 PLID 15685) - Audit a new drawer
		int AuditID;
		AuditID = BeginNewAuditEvent();
		if (AuditID != -1)
			AuditEvent(-1, strName, AuditID, aeiCashDrawerOpen, -1, "", strName, aepMedium);

	} NxCatchAll("Error adding new drawer.");
}

//closing a drawer which is still open
void CEditDrawersDlg::OnCloseDrawer() 
{
	if(!CheckCurrentUserPermissions(bioCashDrawers, sptWrite))
		return;

	try {
		long nCurSel = m_pList->GetCurSel();
		if(nCurSel == sriNoRow)
			return;

		//get the id
		long nID = VarLong(m_pList->GetValue(nCurSel, dcID));
		CString strName = VarString(m_pList->GetValue(nCurSel, dcName));

		//now prompt the user and let them assign values & save
		CUserDrawerDlg dlg(this);
		dlg.m_nID = nID;
		int nReturn = dlg.DoModal();
		if(nReturn == IDCANCEL) {
			//TODO
			return;
		}
		else if (nReturn == IDPREVIEW) {
			//close this dialog also
			OnOK();
			return;
		}

		//remove the row if we're not showing all, requery if we are
		if(IsDlgButtonChecked(IDC_INCLUDE_CLOSED))
			OnIncludeClosed();	//requery
		else
			m_pList->RemoveRow(nCurSel);

		// (j.anspach 05-24-2005 14:06 PLID 15685) - Audit a drawer closing
		//  auditing will be done in userdrawerdlg.cpp
		/*int AuditID;
		AuditID = BeginNewAuditEvent();
		if (AuditID != -1)
			AuditEvent(strName, AuditID, aeiCashDrawerClose, -1, strName, "", aepMedium);*/

	} NxCatchAll("Error closing drawer.");
}

//editing an already-closed drawer
void CEditDrawersDlg::OnEditDrawer() 
{
	//after examining this, it really does the same thing as close
	OnCloseDrawer();
}

BEGIN_EVENTSINK_MAP(CEditDrawersDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditDrawersDlg)
	ON_EVENT(CEditDrawersDlg, IDC_DRAWER_LIST, 9 /* EditingFinishing */, OnEditingFinishingDrawerList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditDrawersDlg, IDC_DRAWER_LIST, 10 /* EditingFinished */, OnEditingFinishedDrawerList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEditDrawersDlg, IDC_DRAWER_LIST, 16 /* SelChosen */, OnSelChosenDrawerList, VTS_I4)
	ON_EVENT(CEditDrawersDlg, IDC_DRAWER_LIST, 2 /* SelChanged */, OnSelChangedDrawerList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditDrawersDlg::OnEditingFinishingDrawerList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	// (a.walling 2006-09-26 11:08) - PLID 22683 - Users should not be able to write to the opening value without permissions
	if(!CheckCurrentUserPermissions(bioCashDrawers, sptWrite))
	{
		*pbCommit = FALSE;
		return;
	}
	if (COleVariant(varOldValue) == COleVariant(*pvarNewValue)) {
		// a.walling Nothing changed, don't refresh, don't write to database, don't audit!
		*pbCommit = FALSE;
		return;
	}

	switch(nCol) {
	case dcName:
		{
			//make sure it's valid
			CString str = VarString(pvarNewValue, "");
			str.TrimLeft();	str.TrimRight();

			if(str.IsEmpty()) {
				MsgBox("You may not have an empty name for a drawer.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
			// (j.anspach 05-24-2005 14:10 PLID 15685) - Audit a change to a cash drawer's name
			try {
				int AuditID;
				AuditID = BeginNewAuditEvent();
				if (AuditID != -1)
					AuditEvent(-1, VarString(m_pList->GetValue(nRow, dcName)), AuditID, aeiCashDrawerName, -1, VarString(varOldValue, ""), VarString(pvarNewValue, ""), aepMedium);
			} NxCatchAll("Error auditing change in cash drawer name.");
		}
		break;

	case dcOpenAmt:
		{
			//make sure it's a valid currency
			COleCurrency cyInvalid;
			cyInvalid.SetStatus(COleCurrency::invalid);
			COleCurrency cy = VarCurrency(pvarNewValue, cyInvalid);

			if(cy.GetStatus() == COleCurrency::invalid) {
				MsgBox("You have entered an invalid currency.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
			// (j.anspach 05-24-2005 14:10 PLID 15685) - Audit a change to the starting amount
			try {
				CString strNew, strOld;
				strOld = FormatCurrencyForInterface(VarCurrency(varOldValue, cyInvalid));
				strNew = FormatCurrencyForInterface(VarCurrency(pvarNewValue, cyInvalid));
				int AuditID;
				AuditID = BeginNewAuditEvent();
				if (AuditID != -1)
					AuditEvent(-1, VarString(m_pList->GetValue(nRow, dcName)), AuditID, aeiCashDrawerStartAmt, -1, strOld, strNew, aepMedium);
			} NxCatchAll("Error auditing change in cash drawer starting amount.");
		}
		break;

	default:
		break;
	}
}

void CEditDrawersDlg::OnEditingFinishedDrawerList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(!bCommit)
		return;

	switch(nCol) {
	case dcName:
		{
			try {
				//save it
				ExecuteSql("UPDATE CashDrawersT SET Name = '%s' WHERE ID = %li", 
					_Q(VarString(varNewValue)), VarLong(m_pList->GetValue(nRow, dcID)));
			} NxCatchAll("Error saving drawer name.");
		}
		break;

	case dcOpenAmt:
		{
			try {
				//save it
				ExecuteSql("UPDATE CashDrawersT SET OpenAmt = convert(money, '%s') WHERE ID = %li", 
					FormatCurrencyForSql(VarCurrency(varNewValue)), VarLong(m_pList->GetValue(nRow, dcID)));
			} NxCatchAll("Error saving drawer amount.");
		}
		break;

	// (j.gruber 2007-08-09 10:31) - PLID 25119  - adding location
	case dcLocation:
		try {
			long nLocationID = VarLong(varNewValue);
			long nDrawerID = VarLong(m_pList->GetValue(nRow, dcID));
			if (nLocationID == -1) {
				ExecuteSql("UPDATE CashDrawersT SET LocationID = NULL WHERE ID = %li", 
					nDrawerID);
			}
			else {
				ExecuteSql("UPDATE CashDrawersT SET LocationID = %li WHERE ID = %li", 
					nLocationID, nDrawerID);
			}

			

		}NxCatchAll("Error saving drawer location");
	break;


	default:
		break;
	}
}

void CEditDrawersDlg::OnIncludeClosed()
{
	try {
		CString strWhere;

		if(IsDlgButtonChecked(IDC_INCLUDE_CLOSED)) {
			//show the columns for close date & close amt
			IColumnSettingsPtr pCol = m_pList->GetColumn(dcCloseDate);
			pCol->PutColumnStyle(csVisible|csWidthAuto);

			pCol = m_pList->GetColumn(dcCloseAmt);
			pCol->PutColumnStyle(csVisible|csWidthAuto);

			strWhere.Empty();
		}
		else {
			//hide the columns for close date & close amt
			IColumnSettingsPtr pCol = m_pList->GetColumn(dcCloseDate);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
			pCol->PutStoredWidth(0);

			pCol = m_pList->GetColumn(dcCloseAmt);
			pCol->PutColumnStyle(csVisible|csFixedWidth);
			pCol->PutStoredWidth(0);

			strWhere.Format("DateClosed IS NULL");
		}

		m_pList->PutWhereClause(_bstr_t(strWhere));
		m_pList->Requery();

		OnSelChangedDrawerList(-1);

	} NxCatchAll("Error in OnIncludeClosed()");
}

void CEditDrawersDlg::OnSelChosenDrawerList(long nRow) 
{
}

void CEditDrawersDlg::OnSelChangedDrawerList(long nNewSel) 
{
	try {
		//disable both if no selection
		if(nNewSel == sriNoRow) {
			GetDlgItem(IDC_CLOSE_DRAWER)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_DRAWER)->EnableWindow(FALSE);
			return;
		}

		//if they're on a closed row, disable the close button and enable the edit button
		_variant_t var = m_pList->GetValue(nNewSel, dcCloseDate);

		BOOL bEnable = TRUE;
		if(var.vt == VT_DATE) {
			//it is a closed drawer
			bEnable = FALSE;
		}

		GetDlgItem(IDC_CLOSE_DRAWER)->EnableWindow(bEnable);
		GetDlgItem(IDC_EDIT_DRAWER)->EnableWindow(!bEnable);

	} NxCatchAll("Error in OnSelChosenDrawerList");
}

void CEditDrawersDlg::OnCurrentLocation() 
{
	if (IsDlgButtonChecked(IDC_CASH_DRAWER_CURRENT_LOCATION)) {
		SetRemotePropertyInt("CashDrawerDefaultAllLocations", 0, 0, GetCurrentUserName());
	}
	else {
		SetRemotePropertyInt("CashDrawerDefaultAllLocations", 1, 0, GetCurrentUserName());
	}
	
}

void CEditDrawersDlg::OnAllLocations() 
{
	if (IsDlgButtonChecked(IDC_CASH_DRAWER_ALL_LOCATIONS)) {
		SetRemotePropertyInt("CashDrawerDefaultAllLocations", 1, 0, GetCurrentUserName());
	}
	else {
		SetRemotePropertyInt("CashDrawerDefaultAllLocations", 0, 0, GetCurrentUserName());
	}
		


	
}

// (a.walling 2007-09-28 12:01) - PLID 27468 - Open physical cash drawer
void CEditDrawersDlg::OnOpenPhysicalCashDrawer() 
{
	try {
		if (GetMainFrame()->GetCashDrawerDevice()) { // ensure we have a valid device
			if (CheckCurrentUserPermissions(bioCashDrawers, sptDynamic0)) { // and that we have permission
				CString strLastState = GetMainFrame()->GetCashDrawerDevice()->GetDrawerLastStateString();

				long nResult = GetMainFrame()->GetCashDrawerDevice()->OpenDrawer();
				if (nResult == OPOS_SUCCESS) {
					AuditEvent(-1, "", BeginNewAuditEvent(), aeiPOSCashDrawerOpen, -1, strLastState, "Drawer Opened (Manually Opened from Edit Cash Drawer Sessions)", aepHigh, aetChanged);
				}
			}
		}
	} NxCatchAll("Error in CEditDrawersDlg::OnOpenPhysicalCashDrawer()");
}

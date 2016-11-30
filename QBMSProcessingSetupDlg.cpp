// QBMSProcessingSetupDlg.cpp : implementation file
// (d.thompson 2009-07-06) - PLID 34690 - Created
//

#include "stdafx.h"
#include "Practice.h"
#include "QBMSProcessingSetupDlg.h"
#include "QBMSProcessingUtils.h"

using namespace ADODB;

enum eAccountListColumns {
	ealcID = 0,
	ealcDescription,
};

// CQBMSProcessingSetupDlg dialog

IMPLEMENT_DYNAMIC(CQBMSProcessingSetupDlg, CNxDialog)

CQBMSProcessingSetupDlg::CQBMSProcessingSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CQBMSProcessingSetupDlg::IDD, pParent)
{
	// (d.thompson 2010-11-05) - PLID 40628 - This tracks the next ID to be used for a 'new' account.  We'll track them negatively, 
	//	so -1, -2, -3, etc.  That will be the signifier that it's new, while still giving us a unique ID to access the account by.
	m_nNextNewID = -1;
	// (d.thompson 2010-11-05) - PLID 40628 - See workaround description in header.  Default to 0 because positive values are real and
	//	negative values are new records.
	m_nPreviouslySelectedAcctID = 0;
}

CQBMSProcessingSetupDlg::~CQBMSProcessingSetupDlg()
{
}

void CQBMSProcessingSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CONNECTION_TICKET, m_nxeditTicket);
	DDX_Control(pDX, IDC_QBMS_ACCOUNT_ADD, m_btnAdd);
	DDX_Control(pDX, IDC_QBMS_ACCOUNT_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_QBMS_ACCOUNT_ACTIVE, m_btnActive);
}

BEGIN_MESSAGE_MAP(CQBMSProcessingSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SWITCH_MODE, &CQBMSProcessingSetupDlg::OnBnClickedSwitchMode)
	ON_BN_CLICKED(IDC_GET_CONNECTION_TICKET, &CQBMSProcessingSetupDlg::OnBnClickedGetConnectionTicket)
	ON_BN_CLICKED(IDC_QBMS_ACCOUNT_ADD, &CQBMSProcessingSetupDlg::OnBnClickedQbmsAccountAdd)
	ON_BN_CLICKED(IDC_QBMS_ACCOUNT_DELETE, &CQBMSProcessingSetupDlg::OnBnClickedQbmsAccountDelete)
	ON_BN_CLICKED(IDC_QBMS_ACCOUNT_ACTIVE, &CQBMSProcessingSetupDlg::OnBnClickedQbmsAccountActive)
	ON_EN_KILLFOCUS(IDC_QBMS_ACCOUNT_NAME, &CQBMSProcessingSetupDlg::OnEnKillfocusQbmsAccountName)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CQBMSProcessingSetupDlg, CNxDialog)
	ON_EVENT(CQBMSProcessingSetupDlg, IDC_QBMS_ACCOUNT_LIST, 16, CQBMSProcessingSetupDlg::SelChosenQbmsAccountList, VTS_DISPATCH)
	ON_EVENT(CQBMSProcessingSetupDlg, IDC_QBMS_ACCOUNT_LIST, 1, CQBMSProcessingSetupDlg::SelChangingQbmsAccountList, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

BOOL CQBMSProcessingSetupDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//Setup controls
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (d.thompson 2010-11-04) - PLID 40628
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		// (d.thompson 2010-11-04) - PLID 40628
		m_pAccountList = BindNxDataList2Ctrl(IDC_QBMS_ACCOUNT_LIST, false);

		SetDlgItemText(IDC_QBMS_CONNECTION_TICKET_LABEL, "For security purposes, Quickbooks requires that you login to your Quickbooks "
			"Merchant Services account and request a connection ticket for this application.  This action will authorize NexTech Practice "
			"to send Credit Card transactions through your Quickbooks Merchant Services account.  This ticket will remain valid indefinitely "
			"until you remove its access through your Merchant Services account.\r\n\r\n"
			"1)  Please press 'Get Connection Ticket' below, then login with your Merchant Services account information.\r\n"
			"2)  Click the 'Create a Connection' button.  If prompted, ensure that 'login security' is set to 'No'.\r\n"
			"3)  You will be provided a connection ticket.  Please copy it and paste it into the box below.");

		// (d.thompson 2010-11-04) - PLID 40628 - We now have multiple accounts.  Load them all into the member maps.
		LoadAllAccountsFromData();

		// (d.thompson 2010-11-04) - PLID 40628 - Reflect them in the datalist as well
		ReflectAccountsInDatalist();

		// (d.thompson 2010-11-04) - PLID 40628 - Display the "first" account in the datalist
		m_pAccountList->PutCurSel( m_pAccountList->GetFirstRow() );

		//Now reflect that one on screen
		if(m_pAccountList->GetCurSel()) {
			ReflectCurrentAccount();
		}
		else {
			//Can happen if there are no accounts in the system.
			EnableInterface();
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (d.thompson 2010-11-05) - PLID 40628
//Loads all QBMS account information from data into member variable maps for this dialog.  This function
//	should only be called once.  Note that previous to the multi-account implementation, we loaded the
//	QBMS data from the cache.  We will no longer trust the cache, and always go back to data.
//This function will ensure the maps are empty before it starts, and that they match data when it finishes.
void CQBMSProcessingSetupDlg::LoadAllAccountsFromData()
{
	//Ensure all lists are clear before we start
	m_mapLiveAccounts.RemoveAll();
	m_mapDeletedAccounts.RemoveAll();

	//Now load all from database
	_RecordsetPtr prs = CreateRecordset("SELECT ID, Description, Inactive, IsProduction, ConnectionTicket FROM QBMS_SetupData");
	while(!prs->eof) {
		long nID = AdoFldLong(prs, "ID");
		CQBMSProcessingAccount acct;
		acct.LoadFromData(nID, 
			AdoFldString(prs, "Description"), 
			AdoFldBool(prs, "Inactive") == FALSE ? false : true, 
			AdoFldBool(prs, "IsProduction") == FALSE ? false : true, 
			prs->Fields->Item["ConnectionTicket"]->Value);
		m_mapLiveAccounts.SetAt(nID, acct);

		prs->MoveNext();
	}
}

// (d.thompson 2010-11-05) - PLID 40628
//Takes the member maps and loads the account datalist with the information from them.  This function should
//	only be called once, as it will erase the datalist prior to loading.
void CQBMSProcessingSetupDlg::ReflectAccountsInDatalist()
{
	//Clear the datalist to start
	m_pAccountList->Clear();

	//Now create rows based on everything in the map
	POSITION pos = m_mapLiveAccounts.GetStartPosition();
	while(pos != NULL) {
		long nID;
		CQBMSProcessingAccount acct;
		m_mapLiveAccounts.GetNextAssoc(pos, nID, acct);

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->GetNewRow();
		pRow->PutValue(ealcID, (long)acct.GetID());
		pRow->PutValue(ealcDescription, _bstr_t(acct.GetDescription()));
		m_pAccountList->AddRowSorted(pRow, NULL);
	}
}

// (d.thompson 2010-11-05) - PLID 40628
//Returns a copy of the account information based on the given ID.  Throws exceptions if the ID does not exist.  This will
//	not search the deleted list.  (Open to changing that if necessary)
CQBMSProcessingAccount CQBMSProcessingSetupDlg::GetAccountByID(long nIDToFind)
{
	//Search the 'live' list first
	POSITION pos = m_mapLiveAccounts.GetStartPosition();
	while(pos != NULL) {
		long nID;
		CQBMSProcessingAccount acct;
		m_mapLiveAccounts.GetNextAssoc(pos, nID, acct);

		if(nID == nIDToFind) {
			return acct;
		}
	}

	//Currently, do not search the deleted list.  If it wasn't found above, blow up.  We require a valid ID.  This should
	//	be impossible unless the UI and data have become disconnected.
	AfxThrowNxException("Could not find QBMS Account ID %li!", nIDToFind);
	throw;	//Required to avoid compiler warning, will not actually do anything
}

// (d.thompson 2010-11-05) - PLID 40628
//Looks up the current account in the account datalist.  Loads the information for that account into the fields in the UI.  Does
//	not save any current data that may be overwritten.
void CQBMSProcessingSetupDlg::ReflectCurrentAccount()
{
	//Data to load
	CString strDescription;
	bool bActive = true;
	bool bProduction = false;;
	CString strTicket;
	long nAcctID = 0;

	//find the current account
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->CurSel;
	if(pRow == NULL) {
		//No row, nothing to reflect.  Leave the values empty.
	}
	else {
		CQBMSProcessingAccount acct = GetAccountByID(VarLong(pRow->GetValue(ealcID)));
		strDescription = acct.GetDescription();
		bActive = !acct.GetIsInactive();
		bProduction = acct.GetIsProduction();
		strTicket = acct.GetDecryptedTicket();
		nAcctID = acct.GetID();
	}

	// (d.thompson 2010-11-05) - PLID 40628 - Workaround:  If the previously selected acct is the same as the current account...
	//	don't do this.  See workaround description in the header.
	if(m_nPreviouslySelectedAcctID == nAcctID) {
		return;
	}

	//Load the values into the controls
	SetDlgItemText(IDC_QBMS_ACCOUNT_NAME, strDescription);
	CheckDlgButton(IDC_QBMS_ACCOUNT_ACTIVE, bActive ? TRUE : FALSE);
	SetDlgItemText(IDC_CONNECTION_TICKET, strTicket);
	EnsureModeButtonText(bProduction);

	// (d.thompson 2010-11-05) - PLID 40628 - Maintain the currently selected account
	m_nPreviouslySelectedAcctID = nAcctID;
}

// (d.thompson 2010-11-05) - PLID 40628 - Enables / disables the interface as appropriate depending on interface selections.
void CQBMSProcessingSetupDlg::EnableInterface()
{
	//If there is no selection (typically only possible if there are no possible selections), we need to disable the interface, 
	//	then possible enable again if a new one is added.
	BOOL bEnabled = FALSE;
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->CurSel;
	if(pRow == NULL) {
		bEnabled = FALSE;
	}
	else {
		bEnabled = TRUE;
	}

	GetDlgItem(IDC_SWITCH_MODE)->EnableWindow(bEnabled);
	GetDlgItem(IDC_QBMS_ACCOUNT_NAME)->EnableWindow(bEnabled);
	GetDlgItem(IDC_QBMS_ACCOUNT_ACTIVE)->EnableWindow(bEnabled);
	GetDlgItem(IDC_GET_CONNECTION_TICKET)->EnableWindow(bEnabled);
	GetDlgItem(IDC_CONNECTION_TICKET)->EnableWindow(bEnabled);
	GetDlgItem(IDC_QBMS_ACCOUNT_DELETE)->EnableWindow(bEnabled);
	//IDC_QBMS_ACCOUNT_ADD is always enabled
	//IDOK and IDCANCEL are always enabled
}

void CQBMSProcessingSetupDlg::OnBnClickedSwitchMode()
{
	try {
		// (d.thompson 2010-11-05) - PLID 40628 - Determine the action based off the currently selected account.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->CurSel;
		if(pRow == NULL) {
			//Should be blocked by the UI and impossible.  Just warn them.
			AfxMessageBox("Please select a Merchant Account from the list before attempting to change the production status.");
		}
		else {
			CQBMSProcessingAccount acct = GetAccountByID(VarLong(pRow->GetValue(ealcID)));

			//Give a thorough warning.
			CString strWarningMessage;
			if(acct.GetIsProduction() == FALSE) {
				//They are in pre-prod and want to go to prod.
				strWarningMessage = "You are currently in a pre-production mode.  If you continue to switch modes, all "
					"credit card transactions will be processed live against your account.  Please ensure you do not need "
					"to do any more testing before running live credit cards through the system.";
			}
			else {
				//They are in prod already and want to turn it off.
				strWarningMessage = "You are currently in a production mode.  If you enable this, your credit card transactions will no "
					"longer be processed through Merchant Services.  All attempts to process will go through a test emulation server, no "
					"actual charges will be processed.";
			}

			if(AfxMessageBox(strWarningMessage + "\r\n\r\nAre you SURE you wish to make this change?", MB_YESNO) != IDYES) {
				return;
			}

			//Nothing saves until they press OK.  Flag the change in memory.  This will automatically
			//	wipe the connection ticket in memory.
			acct.SetProduction( !acct.GetIsProduction() );

			//Wipe the connection ticket in the interface -- they need to get a new one for this new mode.  
			SetDlgItemText(IDC_CONNECTION_TICKET, "");

			//Make sure the button says the right thing
			EnsureModeButtonText(acct.GetIsProduction());

			//Put data back in the map
			m_mapLiveAccounts.SetAt(acct.GetID(), acct);
		}

	} NxCatchAll(__FUNCTION__);
}

//Will update the text of the "switch to ..." button appropriately based on the parameter given.
void CQBMSProcessingSetupDlg::EnsureModeButtonText(bool bProduction)
{
	//Based on the production status given, update the label of our 'Swap' button
	CString strMode = "Production";
	if(bProduction) {
		//The button will swap to pre-prod
		strMode = "Pre-Production";
	}

	GetDlgItem(IDC_SWITCH_MODE)->SetWindowText("S&witch to " + strMode + " Mode");
}

void CQBMSProcessingSetupDlg::OnBnClickedGetConnectionTicket()
{
	try {
		// (d.thompson 2010-11-05) - PLID 40628 - Get ticket based on current account selection
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->CurSel;
		if(pRow == NULL) {
			//Should be blocked by the UI and impossible.  Just warn them.
			AfxMessageBox("Please select a Merchant Account from the list before attempting to change the retrieve a ticket.");
		}
		else {
			CQBMSProcessingAccount acct = GetAccountByID(VarLong(pRow->GetValue(ealcID)));

			//This function just launches a browser to the given URL
			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			ShellExecute(GetDesktopWindow()->GetSafeHwnd(), NULL, QBMSProcessingUtils::GetConnectionTicketURL(acct.GetIsProduction() == false ? FALSE : TRUE), "", "", SW_SHOW);
		}
	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-05) - PLID 40628 - Handler for clicking the 'Active' button.
void CQBMSProcessingSetupDlg::OnBnClickedQbmsAccountActive()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->CurSel;
		if(pRow == NULL) {
			//Should be blocked by the UI and impossible.  Do nothing.
		}
		else {
			CQBMSProcessingAccount acct = GetAccountByID(VarLong(pRow->GetValue(ealcID)));

			//Just update the account status based on what the user just checked.  Flag is opposite the display.
			acct.SetInactive( IsDlgButtonChecked(IDC_QBMS_ACCOUNT_ACTIVE) == FALSE ? true : false );

			//Put data back in the map
			m_mapLiveAccounts.SetAt(acct.GetID(), acct);
		}
	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-05) - PLID 40628 - User may have changed the account name.
void CQBMSProcessingSetupDlg::OnEnKillfocusQbmsAccountName()
{
	try {
		//This is purely for display purposes.  We won't actually "commit" the name until they press OK or change selections.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->CurSel;
		if(pRow == NULL) {
			//Should be blocked by the UI and impossible.  Do nothing.
		}
		else {
			CString strNewDesc;
			GetDlgItemText(IDC_QBMS_ACCOUNT_NAME, strNewDesc);

			//Set new description in the datalist
			pRow->PutValue(ealcDescription, _bstr_t(strNewDesc));
		}
	} NxCatchAll(__FUNCTION__);
}

//Look through all live accounts (does not search deleted) for a duplicated name.  Will skip the 
//	account that matches nIDToSkip.  Returns true if this name exists already, false if it's unique.
bool CQBMSProcessingSetupDlg::DoesNameExist(CString strName, long nIDToSkip)
{
	POSITION pos = m_mapLiveAccounts.GetStartPosition();
	while(pos != NULL) {
		long nID;
		CQBMSProcessingAccount acct;
		m_mapLiveAccounts.GetNextAssoc(pos, nID, acct);

		if(nID == nIDToSkip) {
			//Skip this one
		}
		else {
			//See if the name matches.  Compare case insensitive.
			if(acct.GetDescription().CompareNoCase(strName) == 0) {
				//We have a match!
				return true;
			}
		}
	}

	//no matches
	return false;
}

// (d.thompson 2010-11-05) - PLID 40628 - Takes the strings (currently Description and Connection Ticket) on the UI and
//	saves them in the map of account details.
bool CQBMSProcessingSetupDlg::SaveCurrentStringsToMap()
{
	//m_nPreviouslySelectedAcctID is the currently selected account.  We can't trust the datalist selection because
	//	OnSelChanging is fired by using the keyboard w/o making a selection.  
	CQBMSProcessingAccount acct = GetAccountByID(m_nPreviouslySelectedAcctID);

	CString strDescription;
	CString strTicket;
	GetDlgItemText(IDC_QBMS_ACCOUNT_NAME, strDescription);
	GetDlgItemText(IDC_CONNECTION_TICKET, strTicket);

	//No blank names, and get rid of any spaces at the end.
	strDescription.TrimRight();
	if(strDescription.IsEmpty()) {
		AfxMessageBox("The account description may not be blank.  Please enter some text to describe this account.");
		return false;
	}

	//Description is capped at 100 chars
	if(strDescription.GetLength() > 100) {
		AfxMessageBox("The account description may not exceed 100 characters.  Please shorten your description.");
		return false;
	}

	//We cannot allow duplicate names.  Don't bother checking against data, we just need to check against our arrays, any of them
	//	may have changed.
	if(DoesNameExist(strDescription, acct.GetID())) {
		AfxMessageBox("The name you have entered already exists.  You cannot use the same name more than once.  Please try another.");
		return false;
	}

	// (d.thompson 2010-11-08) - PLID 40628 - The ticket is provided by QBMS and we know will never be more than 20 or so characters.  We'll do
	//	a silent truncate here, just to make sure that a user doesn't put anything crazy in the field.  There will be no warning about
	//	this, we just toss any extra data out the window.
	if(strTicket.GetLength() > 100) {
		//Completely arbitrary length chosen.  Tickets are approximately 30 - 40 characters from what I've seen.
		strTicket = strTicket.Left(100);
	}

	//Update the map object
	acct.SetDescription(strDescription);
	acct.SetAndEncryptTicket(strTicket);

	//Put data back in the map
	m_mapLiveAccounts.SetAt(acct.GetID(), acct);
	return true;
}

// (d.thompson 2010-11-05) - PLID 40628 - Attempting to change the currently active account
void CQBMSProcessingSetupDlg::SelChangingQbmsAccountList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Don't let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-05) - PLID 40628 - Committing the selection to a new account.
void CQBMSProcessingSetupDlg::SelChosenQbmsAccountList(LPDISPATCH lpRow)
{
	try {
		//When updating selections, we need to save the string values into the current map object.  This is the safest method, as KillFocus
		//	is not reliable when making changes then immediately hitting "Enter" or pressing "OK".
		if(!SaveCurrentStringsToMap()) {
			//Unable to save the strings, so do not continue
			m_pAccountList->SetSelByColumn(ealcID, (long)m_nPreviouslySelectedAcctID);
			return;
		}

		//OnSelChanging will handle saving everything on the interface to memory, so all we need to do here is load up the new selection and 
		//	reflect it on-screen.
		ReflectCurrentAccount();

		//Ensure the interface is properly enabled/disabled
		EnableInterface();

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-05) - PLID 40628 - Allow users to add a new account.
void CQBMSProcessingSetupDlg::OnBnClickedQbmsAccountAdd()
{
	try {
		//Ensure whatever is currently on screen gets saved
		//When updating selections, we need to save the string values into the current map object.  This is the safest method, as KillFocus
		//	is not reliable when making changes then immediately hitting "Enter" or pressing "OK".
		//Only save if there's an account selected.
		if(m_pAccountList->GetCurSel() != NULL) {
			if(!SaveCurrentStringsToMap()) {
				//Failed to save, so we cannot continue.  
				return;
			}
		}

		//Create a new account
		CQBMSProcessingAccount acct;
		acct.LoadAsNewAccount(m_nNextNewID);
		//Decrement the new ID so it is not reused.  We go negative for new ids (-1, -2, -3, etc)
		m_nNextNewID--;

		//Add this new account to our map
		m_mapLiveAccounts.SetAt(acct.GetID(), acct);

		//Create a row for the datalist
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->GetNewRow();
		pRow->PutValue(ealcID, (long)acct.GetID());
		pRow->PutValue(ealcDescription, _bstr_t(acct.GetDescription()));
		m_pAccountList->AddRowSorted(pRow, NULL);

		//Set our new row active
		m_pAccountList->PutCurSel(pRow);

		//And ensure that we load it properly on screen
		ReflectCurrentAccount();

		//Ensure that the interface is properly enabled
		EnableInterface();

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-05) - PLID 40628 - Provide the ability to delete an account.
void CQBMSProcessingSetupDlg::OnBnClickedQbmsAccountDelete()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->CurSel;
		if(pRow == NULL) {
			//Should be blocked by the UI and impossible.  Do nothing
		}
		else {
			CQBMSProcessingAccount acct = GetAccountByID(VarLong(pRow->GetValue(ealcID)));

			//Don't allow deletion of in use accounts (if already saved)
			if(acct.GetID() > 0) {
				_RecordsetPtr prsTest = CreateParamRecordset("SELECT TOP 1 * FROM QBMS_CreditTransactionsT WHERE AccountID = {INT};", acct.GetID());
				if(!prsTest->eof) {
					AfxMessageBox("This account has been used for existing payments and cannot be deleted.  Please use the inactivate option instead.");
					return;
				}
			}

			//Warn first
			if(AfxMessageBox("Are you sure you wish to delete this account?", MB_YESNO) == IDNO) {
				return;
			}

			//If it's a negative account ID, that means it's new.  We don't need to bother doing anything, just drop it from the list.
			long nID = acct.GetID();
			if(nID < 0) {
				//Let it vanish from here, it's never been saved.
			}
			else {
				//if it had a positive ID, it has been saved to data.  We need to ensure it's removed from data, by adding it to the "deleted" map.
				m_mapDeletedAccounts.SetAt(nID, acct);
			}

			//In both cases, we remove from the 'live' map
			m_mapLiveAccounts.RemoveKey(nID);

			//Change the datalist to be the 'next' row, if there is one.
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pRow->GetNextRow();

			//Now remove the deleted row from the datalist
			m_pAccountList->RemoveRow(pRow);

			//If we did not find a 'next' row, pick the now-first row.
			if(pNextRow == NULL) {
				pNextRow = m_pAccountList->GetFirstRow();
			}

			//Set the current selection to the row we picked above
			m_pAccountList->PutCurSel(pNextRow);

			//Ensure we load that row.  I'd prefer to just call SelChosen, but we have already deleted
			//	the current account, and we don't want to do any of the "save current value" stuff, as
			//	it may fail for various reasons (invalid data, etc) that we don't care about while deleting.
			ReflectCurrentAccount();

			//Ensure the interface is properly enabled/disabled
			EnableInterface();
		}
	} NxCatchAll(__FUNCTION__);
}

void CQBMSProcessingSetupDlg::OnOK()
{
	try {
		// (d.thompson 2010-11-05) - PLID 40628 - When updating selections, we need to save the string values into the current 
		//	map object.  This is the safest method, as KillFocus is not reliable when making changes then immediately 
		//	hitting "Enter" or pressing "OK".
		//Special case:  Allow them to press OK even if there's nothing in the list or nothing selected
		if(m_pAccountList->GetCurSel() != NULL) {
			if(!SaveCurrentStringsToMap()) {
				//Failed to save, we cannot continue
				return;
			}
		}


		// (d.thompson 2010-11-05) - PLID 40628
		//At this point, all data is saved into the maps.  We need to commit that to the database.  Then we need to update the cached maps that
		//	are used by the actual processing.  Perform all work in a batch transaction.
		CParamSqlBatch batch;
		long nCountUpdates = 0;

		//0)  Setup. Any new records will need to have their IDs tracked, so maintain a table for that
		batch.Declare("SET NOCOUNT ON;");
		batch.Declare("DECLARE @tblIDs TABLE (OldID int NOT NULL, NewID int NOT NULL);");

		//1)  Anything that has been deleted should be removed from the database
		{
			POSITION pos = m_mapDeletedAccounts.GetStartPosition();
			while(pos != NULL) {
				long nID;
				CQBMSProcessingAccount acct;
				m_mapDeletedAccounts.GetNextAssoc(pos, nID, acct);

				//Queue up a SQL statement to delete this account
				batch.Add("DELETE FROM QBMS_SetupData WHERE ID = {INT};", nID);
				nCountUpdates++;
			}
		}

		//2)  Save any modified 'live' accounts.  We'll just overwrite the whole record.
		{
			POSITION pos = m_mapLiveAccounts.GetStartPosition();
			while(pos != NULL) {
				long nID;
				CQBMSProcessingAccount acct;
				m_mapLiveAccounts.GetNextAssoc(pos, nID, acct);

				//Only save if the account was modified
				if(acct.GetIsModified()) {
					if(nID > 0) {
						//Queue up a SQL statement to update this account
						batch.Add("UPDATE QBMS_SetupData "
							"SET Description = {STRING}, Inactive = {INT}, IsProduction = {INT}, ConnectionTicket = {VARBINARY} "
							"WHERE ID = {INT};", acct.GetDescription(), acct.GetIsInactive() == false ? 0 : 1, 
							acct.GetIsProduction() == false ? 0 : 1, acct.GetEncryptedTicketForDatabase(), nID);
						nCountUpdates++;
					}
					else {
						//Negative IDs mean this is a new account.  We'll want to write a new record.
						batch.Add("INSERT INTO QBMS_SetupData (Description, Inactive, IsProduction, ConnectionTicket) values "
							"({STRING}, {INT}, {INT}, {VARBINARY});"
							"INSERT INTO @tblIDs (OldID, NewID) SELECT {INT} AS OldID, @@IDENTITY AS NewID;", 
							acct.GetDescription(), acct.GetIsInactive() == false ? 0 : 1, 
							acct.GetIsProduction() == false ? 0 : 1, acct.GetEncryptedTicketForDatabase(), nID);
						nCountUpdates++;
					}
				}
			}
		}

		//3)  Batch cleanup, select any IDs that were updated
		batch.Declare("SET NOCOUNT OFF;");
		batch.Add("SELECT OldID, NewID FROM @tblIDs;");

		//Go ahead and execute our changes, if there were any
		if(nCountUpdates > 0) {
			_RecordsetPtr prsExec = batch.CreateRecordset(GetRemoteData());

			//Any new records will have IDs we need to update.  Modified records will have none.
			while(!prsExec->eof) {
				long nOldID = AdoFldLong(prsExec, "OldID");
				long nNewID = AdoFldLong(prsExec, "NewID");

				//1)  Lookup the account
				CQBMSProcessingAccount acct = GetAccountByID(nOldID);

				//2)  Update the ID to the new value
				acct.ChangeIDAfterSave(nNewID);

				//3)  Remove it from our map entirely
				m_mapLiveAccounts.RemoveKey(nOldID);

				//4)  Add it again under the new ID
				m_mapLiveAccounts.SetAt(nNewID, acct);

				prsExec->MoveNext();
			}
		}

		//At this point:
		//	- All deleted records are deleted from data
		//	- All modified records are updated in data
		//	- All new records are inserted to data
		//	- The m_mapLiveAccounts map is updated to correspond with what is in data

		//We now need to reflect that map into what is cached system-wide for the actual processing functions to use

		// (d.thompson 2010-11-08) - PLID 41367 - I was going to do a fancy conversion where I take the map of live data in
		//	this dialog, then pass it over to become the map for the system.  But it's a lot more simple just to clear the
		//	global map and let it re-cache upon next use.  It's 1 data access, and this OnOK is processed very rarely
		//	(in most live setups to this point, it's been used once ever).
		//So just clear the global cache
		QBMSProcessingUtils::ClearCachedAccounts();

		CDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

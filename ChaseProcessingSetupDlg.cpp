// ChaseProcessingSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "ChaseProcessingSetupDlg.h"
#include "ChaseProcessingUtils.h"

// (d.lange 2010-08-31 11:19) - PLID 40309 - created new dialog for Chase CC setup
// CChaseProcessingSetupDlg dialog
using namespace ChaseProcessingUtils;
using namespace ADODB;

enum eListColumns {
	elcID = 0,
	elcDescription,
	elcInactive,
	elcIsProduction,
	elcUsername,
	elcLoadedPassword,		//This is only valid once, upon loading.  It is never updated as the dialog is used
	elcMerchantID,
	elcTerminalID,
	elcTextPassword,		//This version will be updated as the password is changed through program use.
	elcModified,
};

IMPLEMENT_DYNAMIC(CChaseProcessingSetupDlg, CNxDialog)

CChaseProcessingSetupDlg::CChaseProcessingSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CChaseProcessingSetupDlg::IDD, pParent)
{
	// (d.thompson 2010-11-15) - PLID 40614 - Use 0 because positive are known saved values and negatives will be reserved for new values.
	m_nPreviouslySelectedAcctID = 0;
	// (d.thompson 2010-11-15) - PLID 40614 - Decrement as new accounts are created
	m_nNextNewID = -1;
}

CChaseProcessingSetupDlg::~CChaseProcessingSetupDlg()
{
}

void CChaseProcessingSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_SET_PASSWORD_BTN, m_btnSetPassword);
	DDX_Control(pDX, IDC_CHASE_USERNAME, m_nxeditUsername);
	DDX_Control(pDX, IDC_MERCHANT_ID, m_nxeditMerchantID);
	DDX_Control(pDX, IDC_TERMINAL_ID, m_nxeditTerminalID);
	DDX_Control(pDX, IDC_ACCOUNT_DESCRIPTION, m_nxeditDescription);
	DDX_Control(pDX, IDC_ADD_ACCOUNT, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE_ACCOUNT, m_btnDelete);
}

BEGIN_MESSAGE_MAP(CChaseProcessingSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_SET_PASSWORD_BTN, &CChaseProcessingSetupDlg::OnBnClickedSetPasswordBtn)
	ON_BN_CLICKED(IDC_SWITCH_PROD_MODE_BTN, &CChaseProcessingSetupDlg::OnBnClickedSwitchProdModeBtn)
	ON_BN_CLICKED(IDC_ADD_ACCOUNT, &CChaseProcessingSetupDlg::OnBnClickedAddAccount)
	ON_BN_CLICKED(IDC_DELETE_ACCOUNT, &CChaseProcessingSetupDlg::OnBnClickedDeleteAccount)
	ON_EN_KILLFOCUS(IDC_ACCOUNT_DESCRIPTION, &CChaseProcessingSetupDlg::OnEnKillfocusAccountDescription)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CChaseProcessingSetupDlg, CNxDialog)
	ON_EVENT(CChaseProcessingSetupDlg, IDC_ACCOUNT_LIST, 1, CChaseProcessingSetupDlg::SelChangingAccountList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CChaseProcessingSetupDlg, IDC_ACCOUNT_LIST, 16, CChaseProcessingSetupDlg::SelChosenAccountList, VTS_DISPATCH)
END_EVENTSINK_MAP()

//Call anytime the password may change or current account is updated.  Will appropriately update
//	the display of the "Set password" button.
void CChaseProcessingSetupDlg::UpdatePasswordBtnAppearance()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->GetCurSel();
	if(pRow == NULL) {
		//Should not be possible, interface is disabled, but be safe just in case and quit.
		return;
	}

	CString strTextPassword = VarString(pRow->GetValue(elcTextPassword));

	if(strTextPassword.IsEmpty()) {
		//Update the button text to 'Set Password'
		m_btnSetPassword.SetWindowText("Set Password");
		m_btnSetPassword.SetTextColor(RGB(255, 0, 0));
	}
	else {
		//Update the button text to 'Change Password'
		m_btnSetPassword.SetWindowTextA("Change Password");
		m_btnSetPassword.SetTextColor(0x008000);
	}
}

//Call anytime the production mode may change or current account is updated.  Will appropriately
//update the text of the "Switch..." button.
void CChaseProcessingSetupDlg::UpdateModeBtnText(BOOL bIsProduction)
{
	CString strMode = "Production";
	if(bIsProduction) {
		strMode = "Pre-Production";
	}

	GetDlgItem(IDC_SWITCH_PROD_MODE_BTN)->SetWindowText("S&witch to " + strMode + " Mode");
}

// (d.thompson 2010-11-15) - PLID 40614
//Look through all live accounts (does not search deleted) for a duplicated name.  Will skip the 
//	account that matches nIDToSkip.  Returns true if this name exists already, false if it's unique.
bool CChaseProcessingSetupDlg::DoesNameExist(CString strName, long nIDToSkip)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->GetFirstRow();
	while(pRow != NULL) {
		long nID = VarLong(pRow->GetValue(elcID));

		if(nID == nIDToSkip) {
			//Skip this one
		}
		else {
			//See if the name matches.  Compare case insensitive.
			if(VarString(pRow->GetValue(elcDescription)).CompareNoCase(strName) == 0) {
				//We have a match!
				return true;
			}
		}

		pRow = pRow->GetNextRow();
	}

	//no matches
	return false;
}

// CChaseProcessingSetupDlg message handlers
BOOL CChaseProcessingSetupDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnSetPassword.AutoSet(NXB_MODIFY);
		// (d.thompson 2010-11-15) - PLID 40614
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);

		// (d.thompson 2010-11-15) - PLID 40614 
		m_pAccountList = BindNxDataList2Ctrl(IDC_ACCOUNT_LIST, true);

		//Select the first account
		m_pAccountList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		//The requery() will load the encrypted passwords into the list.  We then need to do a 1-time
		//	decrypt so we can work with the text passwords.
		OneTimeDecryptPasswords();
		m_pAccountList->PutCurSel( m_pAccountList->GetFirstRow() );

		//Attempt to load that account
		if(m_pAccountList->GetCurSel()) {
			ReflectCurrentAccount();
		}
		else {
			//Can happen if there are no accounts in the system.
			EnableInterface();

			//If these don't get called at least once, your buttons may say "Button1"
			UpdateModeBtnText(FALSE);
			m_btnSetPassword.SetWindowText("Set Password");
		}

		m_nxeditUsername.SetLimitText(255);
		m_nxeditMerchantID.SetLimitText(255);
		// (d.thompson 2010-11-15) - PLID 40614 - Added description
		m_nxeditDescription.SetLimitText(100);

		SetDlgItemText(IDC_CHASE_DESC_LABEL, "Chase Paymentech requires that you log into your Chase Paymentech account, which will "
								"authorize NexTech Practice to send Credit Card transactions through your Chase Paymentech account.  For any "
								"questions regarding your Chase Paymentech account information, contact Chase Paymentech Advanced Product Support "
								"1-800-254-9556.");

	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

//Will take the currently selected row in the datalist and load the values onto the screen.  Will not reload
//	if the selection is the same as the previously loaded selection.
void CChaseProcessingSetupDlg::ReflectCurrentAccount()
{
	//Data to load
	CString strDescription, strUser, strMerchantID, strTerminalID;
	bool bActive = true;
	BOOL bProduction = FALSE;
	long nAcctID = 0;

	//find the current account
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->GetCurSel();
	if(pRow == NULL) {
		//No row selected, leave all the values empty
	}
	else {
		strDescription = VarString(pRow->GetValue(elcDescription));
		strUser = VarString(pRow->GetValue(elcUsername));
		strMerchantID = VarString(pRow->GetValue(elcMerchantID));
		strTerminalID = VarString(pRow->GetValue(elcTerminalID));
		bActive = VarBool(pRow->GetValue(elcInactive)) == FALSE ? true : false;		//Reverse sign
		bProduction = VarBool(pRow->GetValue(elcIsProduction));
		nAcctID = VarLong(pRow->GetValue(elcID));
	}

	// (d.thompson 2010-11-05) - PLID 40614 - Workaround:  If the previously selected acct is the same as the current account...
	//	don't do this.  See workaround description in the header.
	if(m_nPreviouslySelectedAcctID == nAcctID) {
		return;
	}

	//Load the values into the controls
	SetDlgItemText(IDC_ACCOUNT_DESCRIPTION, strDescription);
	SetDlgItemText(IDC_CHASE_USERNAME, strUser);
	SetDlgItemText(IDC_MERCHANT_ID, strMerchantID);
	SetDlgItemText(IDC_TERMINAL_ID, strTerminalID);
	CheckDlgButton(IDC_CHASE_ACTIVE, bActive ? TRUE : FALSE);
	UpdateModeBtnText(bProduction);
	UpdatePasswordBtnAppearance();

	// (d.thompson 2010-11-05) - PLID 40614 - Maintain the currently selected account
	m_nPreviouslySelectedAcctID = nAcctID;
}

// (d.thompson 2010-11-05) - PLID 40614 - Enables / disables the interface as appropriate depending on interface selections.
void CChaseProcessingSetupDlg::EnableInterface()
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

	GetDlgItem(IDC_SWITCH_PROD_MODE_BTN)->EnableWindow(bEnabled);
	GetDlgItem(IDC_CHASE_USERNAME)->EnableWindow(bEnabled);
	GetDlgItem(IDC_CHASE_ACTIVE)->EnableWindow(bEnabled);
	GetDlgItem(IDC_SET_PASSWORD_BTN)->EnableWindow(bEnabled);
	GetDlgItem(IDC_MERCHANT_ID)->EnableWindow(bEnabled);
	GetDlgItem(IDC_DELETE_ACCOUNT)->EnableWindow(bEnabled);
	GetDlgItem(IDC_TERMINAL_ID)->EnableWindow(bEnabled);
	GetDlgItem(IDC_ACCOUNT_DESCRIPTION)->EnableWindow(bEnabled);
	//IDC_ACCOUNT_ADD is always enabled
	//IDOK and IDCANCEL are always enabled
}

void CChaseProcessingSetupDlg::OnOK()
{
	try {
		//Special case:  If there is nothing selected, don't try to save the currently displayed account.  The rest of the work can continue.  This
		//	is mostly the case where nothing exists in the list (starting new or you just deleted all).
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->GetCurSel();
		if(pRow != NULL) {
			// (d.thompson 2010-11-05) - PLID 40614 - When updating selections, we need to save the values into the current 
			//	list.  This is the safest method, as KillFocus is not reliable when making changes then immediately 
			//	hitting "Enter" or pressing "OK".
			if(!SaveCurrentlyDisplayedAccount()) {
				//Failed to save, we cannot continue
				return;
			}
		}

		// (d.thompson 2010-11-05) - PLID 40614
		//At this point, all data is saved into the datalist.  We need to commit that to the database.  Perform all work in a batch transaction.
		CParamSqlBatch batch;
		long nCountUpdates = 0;

		//1)  Anything that has been deleted should be removed from the database
		{
			for(int i = 0; i < m_dwaryDeletedAccounts.GetSize(); i++) {
				long nID = m_dwaryDeletedAccounts.GetAt(i);

				//Queue up a SQL statement to delete this account.
				batch.Add("DELETE FROM Chase_SetupDataT WHERE ID = {INT};", nID);
				nCountUpdates++;
			}
		}

		//2)  Save any modified accounts in the list.  We'll just overwrite the whole record.
		{
			NXDATALIST2Lib::IRowSettingsPtr pCurrentRow = m_pAccountList->GetFirstRow();
			while(pCurrentRow != NULL) {
				long nID = VarLong(pCurrentRow->GetValue(elcID));
				CString strDescription = VarString(pCurrentRow->GetValue(elcDescription));
				CString strUsername = VarString(pCurrentRow->GetValue(elcUsername));
				CString strMerchantID = VarString(pCurrentRow->GetValue(elcMerchantID));
				CString strTerminalID = VarString(pCurrentRow->GetValue(elcTerminalID));
				BOOL bInactive = VarBool(pCurrentRow->GetValue(elcInactive));
				BOOL bIsProduction = VarBool(pCurrentRow->GetValue(elcIsProduction));

				_variant_t varEncryptedPassword = EncryptStringToVariant(VarString(pCurrentRow->GetValue(elcTextPassword)));
				VARIANT varPassword = varEncryptedPassword.Detach();

				//Only save if the account was modified
				if(VarBool(pCurrentRow->GetValue(elcModified))) {
					if(nID > 0) {
						//Queue up a SQL statement to update this account
						batch.Add("UPDATE Chase_SetupDataT "
							"SET Description = {STRING}, Inactive = {INT}, IsProduction = {INT}, ChaseUsername = {STRING}, Password = {VARBINARY}, "
							"MerchantID = {STRING}, TerminalID = {STRING} "
							"WHERE ID = {INT};", strDescription, bInactive == FALSE ? 0 : 1, 
							bIsProduction == FALSE ? 0 : 1, strUsername, varPassword, strMerchantID, strTerminalID, nID);
						nCountUpdates++;
					}
					else {
						//Negative IDs mean this is a new account.  We'll want to write a new record.
						batch.Add("INSERT INTO Chase_SetupDataT (Description, Inactive, IsProduction, ChaseUsername, Password, MerchantID, TerminalID) values "
							"({STRING}, {INT}, {INT}, {STRING}, {VARBINARY}, {STRING}, {STRING});",
							strDescription, bInactive == FALSE ? 0 : 1, 
							bIsProduction == FALSE ? 0 : 1, strUsername, varPassword, strMerchantID, strTerminalID);
						nCountUpdates++;
					}
				}

				//Cleanup password
				VariantClear(&varEncryptedPassword);

				pCurrentRow = pCurrentRow->GetNextRow();
			}
		}

		//Go ahead and execute our changes, if there were any
		if(nCountUpdates > 0) {
			batch.Execute(GetRemoteData());
		}

		// (d.thompson 2011-01-07) - PLID 41760 - Due to rules regarding process of cards, merchants will receive a better rate
		//	from Visa/etc if they pass the "track2" data from swiping the credit card.  The default auto-remember functionality
		//	of Practice is directly in conflict with that, and encourages people to not swipe the card, to their detriment.
		//So, we'll add a prompt here to warn them if necessary
		if(GetRemotePropertyInt("GetChargeInfo", -1, 0, "<None>", true) != 0) {
			//The feature is enabled, we want to warn them.
			if(AfxMessageBox("Your system settings are currently set to auto-recall credit card information in NexTech.  Credit card issuers recommend that "
				"you swipe the card for each transaction that is processed.\r\n"
				"NexTech highly recommends that you disable this setting, requiring credit cards to be swiped every time.  Would you like to make this change now?\r\n\r\n"
				" - Clicking Yes will change the preference to not auto-recall credit card info.\r\n"
				" - Clicking No will leave the setting to auto-recall credit card info.\r\n"
				"This can always be changed later under Preferences -> Billing -> Payments.", MB_YESNO) == IDYES) {
				//They've agreed, go ahead and turn it off
				SetRemotePropertyInt("GetChargeInfo", 0, 0, "<None>");
			}
		}

		CDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

void CChaseProcessingSetupDlg::OnBnClickedSetPasswordBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->GetCurSel();
		if(pRow == NULL) {
			//Should not be possible, interface is disabled, but be safe just in case and quit.
			AfxMessageBox("Please select an account before attempting to change the password.");
			return;
		}

		CString strResult;
		if (IDOK == InputBoxLimited(this, "Please enter your password", strResult, "", 255, true, false, NULL))
		{
			//Update the text password in the datalist
			pRow->PutValue(elcTextPassword, _bstr_t(strResult));
			//Flag as modified
			pRow->PutValue(elcModified, g_cvarTrue);
			UpdatePasswordBtnAppearance();
		}

	} NxCatchAll(__FUNCTION__);
}

void CChaseProcessingSetupDlg::OnBnClickedSwitchProdModeBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->GetCurSel();
		if(pRow == NULL) {
			//Should not be possible, interface is disabled, but be safe just in case.
			AfxMessageBox("Please select an account from the list before attempting to change the production status.");
			return;
		}

		BOOL bExistingMode = VarBool(pRow->GetValue(elcIsProduction));

		CString strWarningMessage;
		if(bExistingMode == FALSE) {
			//They are in pre-prod and want to go to prod.
			strWarningMessage = "You are currently in a pre-production mode.  If you continue to switch modes, all "
				"credit card transactions will be processed live against your account.  Please ensure you do not need "
				"to do any more testing before running live credit cards through the system.";
		}
		else {
			//They are in prod already and want to turn it off.
			strWarningMessage = "You are currently in a production mode.  If you enable this, your credit card transactions will no "
				"longer be processed through Chase.  All attempts to process will go through a test emulation server, no "
				"actual charges will be processed.";
		}
		if(AfxMessageBox(strWarningMessage + "\r\n\r\nAre you SURE you wish to make this change?", MB_YESNO) != IDYES) {
			return;
		}

		//Update the datalist to reflect the new selection (revert sign)
		pRow->PutValue(elcIsProduction, bExistingMode == FALSE ? g_cvarTrue : g_cvarFalse);
		//flag as modified
		pRow->PutValue(elcModified, g_cvarTrue);

		//Update the button text to be the opposite
		UpdateModeBtnText(!bExistingMode);

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-15) - PLID 40614
void CChaseProcessingSetupDlg::SelChangingAccountList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Don't let them select nothing
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-15) - PLID 40614 - Control the reloading of the account on screen
void CChaseProcessingSetupDlg::SelChosenAccountList(LPDISPATCH lpRow)
{
	try {
		//When updating selections, we need to save the string values into the current map object.  This is the safest method, as KillFocus
		//	is not reliable when making changes then immediately hitting "Enter" or pressing "OK".
		if(!SaveCurrentlyDisplayedAccount()) {
			//Unable to save the strings, so do not continue - revert to prev selection
			m_pAccountList->SetSelByColumn(elcID, (long)m_nPreviouslySelectedAcctID);
			return;
		}

		//OnSelChanging will handle saving everything on the interface to memory, so all we need to do here is load up the new selection and 
		//	reflect it on-screen.
		ReflectCurrentAccount();

		//Ensure the interface is properly enabled/disabled
		EnableInterface();

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-05) - PLID 40614 - Takes the interface on the UI and saves it all in the datalist for retrieval.
bool CChaseProcessingSetupDlg::SaveCurrentlyDisplayedAccount()
{
	//m_nPreviouslySelectedAcctID is the currently selected account.  We can't trust the datalist selection because
	//	OnSelChanging is fired by using the keyboard w/o making a selection.  
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->FindByColumn(elcID, (long)m_nPreviouslySelectedAcctID, NULL, VARIANT_FALSE);
	if(pRow == NULL) {
		//We've been asked to save to a row that doesn't exist.  Something is horribly wrong!
		AfxThrowNxException("Chase processing setup:  Cannot save to account %li, the account does not exist.", m_nPreviouslySelectedAcctID);
	}

	//Save all the current values
	CString strDescription, strUsername, strMerchantID, strTerminalID;
	BOOL bActive;

	GetDlgItemText(IDC_ACCOUNT_DESCRIPTION, strDescription);
	GetDlgItemText(IDC_CHASE_USERNAME, strUsername);
	GetDlgItemText(IDC_MERCHANT_ID, strMerchantID);
	GetDlgItemText(IDC_TERMINAL_ID, strTerminalID);
	bActive = IsDlgButtonChecked(IDC_CHASE_ACTIVE) == FALSE ? false : true;

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

	//We cannot allow duplicate names.  Don't bother checking against data, we just need to check against the list, any of them
	//	may have changed.
	if(DoesNameExist(strDescription, VarLong(pRow->GetValue(elcID)))) {
		AfxMessageBox("The name you have entered already exists.  You cannot use the same name more than once.  Please try another.");
		return false;
	}

	//Hard-caps on the other text fields.  I removed any other validation (blank, etc) for simplicity.  It's obvious that these
	//	things are required to process.
	if(strUsername.GetLength() > 255) {
		AfxMessageBox("The username may not exceed 255 characters.  Please check your username and try again.");
		return false;
	}
	if(strMerchantID.GetLength() > 255) {
		AfxMessageBox("The Merchant ID may not exceed 255 characters.  Please check your value and try again.");
		return false;
	}
	if(strTerminalID.GetLength() > 255) {
		AfxMessageBox("The Terminal ID may not exceed 255 characters.  Please check your value and try again.");
		return false;
	}

	//Password cannot be blank
	CString strPassword = VarString(pRow->GetValue(elcTextPassword));
	if(strPassword.IsEmpty()) {
		AfxMessageBox("The password may not be blank.  Please set a password and try again.");
		return false;
	}

	//Need to determine if anything changed, for the 'modified' flag.
	CString strOldDesc, strOldUser, strOldMerchant, strOldTerminal;
	BOOL bOldActive;
	{
		strOldDesc = VarString(pRow->GetValue(elcDescription));
		strOldUser = VarString(pRow->GetValue(elcUsername));
		strOldMerchant = VarString(pRow->GetValue(elcMerchantID));
		strOldTerminal = VarString(pRow->GetValue(elcTerminalID));
		bOldActive = VarBool(pRow->GetValue(elcInactive)) == FALSE ? TRUE : FALSE;	//reverse signs
	}

	if(strOldDesc != strDescription || strOldUser != strUsername || strOldMerchant != strMerchantID || 
		strOldTerminal != strTerminalID || bOldActive != bActive)
	{
		//flag as modified
		pRow->PutValue(elcModified, g_cvarTrue);
	}
	else {
		//nothing was modified, so just quit here
		return true;
	}

	//We are storing all of the data in the datalist, so put these values back there
	pRow->PutValue(elcDescription, _bstr_t(strDescription));
	pRow->PutValue(elcUsername, _bstr_t(strUsername));
	pRow->PutValue(elcMerchantID, _bstr_t(strMerchantID));
	pRow->PutValue(elcTerminalID, _bstr_t(strTerminalID));
	pRow->PutValue(elcInactive, bActive == false ? g_cvarTrue : g_cvarFalse);		//reverse signs
	//Password and Production are both set immediately to the list when changes happen
	return true;
}

// (d.thompson 2010-11-15) - PLID 40614
void CChaseProcessingSetupDlg::OnBnClickedAddAccount()
{
	try {
		//Special case:  If there is nothing currently selected, don't try to save it.  This can happen if there is nothing
		//	in the list.
		if(m_pAccountList->GetCurSel() != NULL) {
			//Ensure whatever is currently on screen gets saved
			//When updating selections, we need to save the string values into the current map object.  This is the safest method, as KillFocus
			//	is not reliable when making changes then immediately hitting "Enter" or pressing "OK".
			if(!SaveCurrentlyDisplayedAccount()) {
				//Failed to save, so we cannot continue.  
				return;
			}
		}

		//Create a row for the datalist
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->GetNewRow();
		pRow->PutValue(elcID , (long)m_nNextNewID);
		pRow->PutValue(elcDescription, _bstr_t("Chase Paymentech"));
		pRow->PutValue(elcInactive, g_cvarFalse);
		pRow->PutValue(elcIsProduction, g_cvarFalse);
		pRow->PutValue(elcUsername, "");
		pRow->PutValue(elcLoadedPassword, g_cvarNull);
		pRow->PutValue(elcMerchantID, "");
		//Terminal ID is almost always 001 except in rare cases, so go ahead and default it
		pRow->PutValue(elcTerminalID, "001");
		pRow->PutValue(elcTextPassword, "");
		pRow->PutValue(elcModified, g_cvarTrue);		//New accounts always 'modified'
		m_pAccountList->AddRowSorted(pRow, NULL);

		//Decrement the new ID so it is not reused.  We go negative for new ids (-1, -2, -3, etc)
		m_nNextNewID--;

		//Set our new row active
		m_pAccountList->PutCurSel(pRow);

		//And ensure that we load it properly on screen
		ReflectCurrentAccount();

		//Ensure that the interface is properly enabled
		EnableInterface();

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2010-11-15) - PLID 40614
void CChaseProcessingSetupDlg::OnBnClickedDeleteAccount()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->CurSel;
		if(pRow == NULL) {
			//Should be blocked by the UI and impossible.  Do nothing
		}
		else {
			long nID = VarLong(pRow->GetValue(elcID));

			//Don't allow deletion of in use accounts (if already saved)
			if(nID > 0) {
				_RecordsetPtr prsTest = CreateParamRecordset("SELECT TOP 1 * FROM Chase_CreditTransactionsT WHERE AccountID = {INT};", nID);
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
			if(nID < 0) {
				//Let it vanish from here, it's never been saved.
			}
			else {
				//if it had a positive ID, it has been saved to data.  We need to ensure it's removed from data, by adding it to the "deleted" map.
				m_dwaryDeletedAccounts.Add(nID);
			}

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

// (d.thompson 2010-11-15) - PLID 40614 - Perform a 1 time decryption of all passwords
void CChaseProcessingSetupDlg::OneTimeDecryptPasswords()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAccountList->GetFirstRow();
		while(pRow != NULL) {
			//Pull it out as loaded
			_variant_t varPassword = pRow->GetValue(elcLoadedPassword);
			CString strDecryptedPass;
			if(varPassword.vt != VT_NULL) {
				//decrypt the password
				strDecryptedPass = DecryptStringFromVariant(varPassword);
			}
			//Put it back in the "decrypted" spot
			pRow->PutValue(elcTextPassword, _bstr_t(strDecryptedPass));
			//Do not set the 'modified' status

			pRow = pRow->GetNextRow();
		}

	} NxCatchAll(__FUNCTION__);
}

void CChaseProcessingSetupDlg::OnEnKillfocusAccountDescription()
{
	try {
		//I go back and forth on whether I think we should update the description immediately or not.  For now, I'm
		//	not going to update until all data is committed.
	} NxCatchAll(__FUNCTION__);
}

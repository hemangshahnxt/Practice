// NexWebLoginInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "NexWebLoginInfoDlg.h"
#include "NexWebChangePasswordDlg.h" 	// (j.armen 2014-07-08 14:07) - PLID 57914
#include "InternationalUtils.h"
#include "GlobalNexWebUtils.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CNexWebLoginInfoDlg dialog

//(e.lally 2009-01-22) PLID 32481 - Added enum column references
enum LoginListColumn {
	llcPersonID = 0,
	llcUserName,
	llcPassword,
	llcEnabled,
	llcMustChangePass,
	// (b.savon 2012-08-16 17:31) - PLID 50584 
	llcLocked,
};

CNexWebLoginInfoDlg::CNexWebLoginInfoDlg(long nPersonID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexWebLoginInfoDlg::IDD, pParent)
{
	m_nPersonID = nPersonID;
	m_bEnableSecurityCode = FALSE;
}


void CNexWebLoginInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNexWebLoginInfoDlg)
	DDX_Control(pDX, IDC_ADD_LOGIN, m_btnAddLogin);
	DDX_Control(pDX, IDC_REMOVE_LOGIN, m_btnRemoveLogin);
	DDX_Control(pDX, IDC_CHANGE_PASSWORD, m_btnChangePassword);
	DDX_Control(pDX, IDC_EXIT_DIALOG, m_btnClose);
	DDX_Control(pDX, IDC_BTN_CREATE_NEXWEB_EMAIL, m_btnCreateEmail);
	DDX_Control(pDX, IDC_CHK_ENABLE_SECURITY_CODE, m_checkEnableSecurityCode);
	DDX_Check(pDX, IDC_CHK_ENABLE_SECURITY_CODE, m_bEnableSecurityCode);
	DDX_Control(pDX, IDC_EDIT_SECURITY_CODE, m_nxeditSecurityCode);
	DDX_Control(pDX, IDC_EDIT_SECURITY_CODE_EXPIRATION, m_nxeditSecurityCodeExpiration);
	DDX_Control(pDX, IDC_RADIO_INCLUDE_SECURITY_CODE, m_radioIncludeSecurityCode);
	DDX_Control(pDX, IDC_RADIO_INCLUDE_LOGIN, m_radioIncludeLogin);
	DDX_Control(pDX, IDC_NXC_NEXWEB_LOGIN1, m_nxcTop);
	DDX_Control(pDX, IDC_NXC_NEXWEB_LOGIN2, m_nxcBottom);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNexWebLoginInfoDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNexWebLoginInfoDlg)
	ON_BN_CLICKED(IDC_ADD_LOGIN, OnAddLogin)
	ON_BN_CLICKED(IDC_EXIT_DIALOG, OnExitDialog)
	ON_BN_CLICKED(IDC_REMOVE_LOGIN, OnRemoveLogin)
	ON_BN_CLICKED(IDC_CHANGE_PASSWORD, OnChangePassword)
	ON_BN_CLICKED(IDC_BTN_CREATE_NEXWEB_EMAIL, OnCreateNexWebEmail)
	ON_BN_CLICKED(IDC_CHK_ENABLE_SECURITY_CODE, OnCheckUseSecurityCode)
	ON_BN_CLICKED(IDC_RADIO_INCLUDE_SECURITY_CODE, OnIncludeSecurityCode)
	ON_BN_CLICKED(IDC_RADIO_INCLUDE_LOGIN, OnIncludeLogin)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNexWebLoginInfoDlg message handlers

BOOL CNexWebLoginInfoDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	//(e.lally 2009-01-22) PLID 32481 - added try/catch
	try{
	
		m_btnAddLogin.AutoSet(NXB_NEW);
		m_btnRemoveLogin.AutoSet(NXB_DELETE);
		m_btnChangePassword.AutoSet(NXB_MODIFY);
		m_btnClose.AutoSet(NXB_CLOSE);

		//(e.lally 2009-01-22) PLID 32481 - Assign the dialog color
		m_nxcTop.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_nxcBottom.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		//(e.lally 2009-01-26) PLID 32811 - Load the security code
		CString strSecurityCode;
		// (b.savon 2012-08-21 11:17) - PLID 50589 - Added SecurityCodeCreation date and SecurityCodeExpiration
		// (b.savon 2012-09-13 14:56) - PLID 50584 - Clear the lockout flag if its past its duration
		CString strSecurityCodeExpiration;
		_RecordsetPtr rs = CreateParamRecordset(
			"SET NOCOUNT ON \r\n"
			/* Reset the Lockout flag if the timing and cycles are correct */
			"DECLARE @Reset INT; \r\n"
			"SELECT @Reset = CASE WHEN \r\n" 
			"			(NexWebLoginInfoT.LockedOut = 1 AND \r\n" 
			"						(NexWebLoginInfoT.LockedOutCycle < (SELECT COALESCE((SELECT IntParam FROM ConfigRT WHERE Name = 'NexWebFailedLoginCycles'), -1)) OR  (SELECT COALESCE((SELECT IntParam FROM ConfigRT WHERE Name = 'NexWebFailedLoginCycles'), -1)) <= 0) AND  \r\n"
			"			DATEDIFF(MI,NexWebLoginInfoT.LockedOutTime, GETDATE()) >= (SELECT COALESCE((SELECT IntParam FROM ConfigRT WHERE Name = 'NexWebFailedLoginTimeout'), -1)) AND \r\n"
			"			(SELECT COALESCE((SELECT IntParam FROM ConfigRT WHERE Name = 'NexWebFailedLoginTimeout'), -1)) > -1) \r\n"
			"			THEN 1 ELSE 0 END \r\n"
			"FROM	PatientsT INNER JOIN NexWebLoginInfoT ON PatientsT.PersonID = NexWebLoginInfoT.PersonID  \r\n"
			"WHERE	PatientsT.PersonID = {INT}		 \r\n"
			"IF @Reset = 1 \r\n"
			"	UPDATE NexWebLoginInfoT SET LockedOut = 0, LockedOutTime = NULL, LoginAttempt = 0 WHERE PersonID = {INT}	"
			"SET NOCOUNT OFF \r\n"
			/* Get the security code as usual */
			"SELECT SecurityCode, SecurityCodeCreationDate, "  
			"(SELECT COALESCE((SELECT IntParam FROM ConfigRT WHERE Name = 'NexWebSecurityCodeExpire'), -1)) AS SecurityCodeExpiration "
			"FROM PatientsT WHERE PersonID = {INT}", m_nPersonID, m_nPersonID, m_nPersonID);
		if(!rs->eof){
			strSecurityCode = AdoFldString(rs, "SecurityCode", "");
			if(!strSecurityCode.IsEmpty()){
				m_bEnableSecurityCode = TRUE;
				SetDlgItemText(IDC_EDIT_SECURITY_CODE, strSecurityCode);
				UpdateData(FALSE);
			}

			// (b.savon 2012-08-21 11:18) - PLID 50589 - Set the expiration date (if any)
			SetSecurityCodeLabel(rs);
		}

		//bind the datalist
		m_pLoginList = BindNxDataListCtrl(this, IDC_LOGIN_LIST, GetRemoteData(), FALSE);

		//add the WHERE Clause
		CString strWhere;
		strWhere.Format("PersonID = %li", m_nPersonID);
		m_pLoginList->WhereClause = _bstr_t(strWhere);

		//now requery the list
		m_pLoginList->Requery();

		// (d.singleton 2011-10-11 12:30) - PLID - 42102 assign security code to member variable for auditing
		m_strSecurityCode = strSecurityCode;

		// (d.singleton 2011-10-11 11:44) - PLID load the patient name for auditing
		_RecordsetPtr rsSecurityCode = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", m_nPersonID);
		if(!rsSecurityCode->eof) {
			m_strPersonName = AdoFldString(rsSecurityCode, "Name", "");
		}
		rsSecurityCode->Close();

		//(e.lally 2009-01-28) PLID 32814 - Bulk cache preferences
		g_propManager.CachePropertiesInBulk("NexWebLoginInfo-num", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'NexWebEmailUseSecurityCode' OR "
			"Name = 'Gen1SaveEmails' "
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("NexWebLoginInfo-text", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'NexWebEmailSubject' "
			")",
			_Q(GetCurrentUserName()));

		//(e.lally 2009-01-28) PLID 32814 - Set our email option for whether to use the Security Code or Username and Password
		//in the body of the email.
		int nIncludeSecurityCode = GetRemotePropertyInt("NexWebEmailUseSecurityCode", 1, 0, GetCurrentUserName(), true);
		if(nIncludeSecurityCode == 1){
			m_radioIncludeSecurityCode.SetCheck(TRUE);
		}
		else{
			m_radioIncludeLogin.SetCheck(TRUE);
		}

		//Permissions
		//(e.lally 2011-08-09) PLID 37287 - Disable controls accordingly
		CPermissions nexwebPerms = GetCurrentUserPermissions(bioPatientNexWebLogin);

		if(!(nexwebPerms & (sptCreate | sptCreateWithPass))){
			m_btnAddLogin.EnableWindow(FALSE);
		}
		//if(!(nexwebPerms & (sptWrite | sptWriteWithPass)) ){
			//Users need to be able to select records for other permissions
			//m_pLoginList->ReadOnly = VARIANT_TRUE;
		//}
		if(!(nexwebPerms & (sptDelete | sptDeleteWithPass)) ){
			m_btnRemoveLogin.EnableWindow(FALSE);
		}
		if(!(nexwebPerms & (sptDynamic0 | sptDynamic0WithPass)) ){
			m_btnChangePassword.EnableWindow(FALSE);
		}
		if(!(nexwebPerms & (sptDynamic1 | sptDynamic1WithPass)) ){
			GetDlgItem(IDC_CHK_ENABLE_SECURITY_CODE)->EnableWindow(FALSE);
		}
		if(!(nexwebPerms & (sptDynamic2 | sptDynamic2WithPass)) ){
			m_btnCreateEmail.EnableWindow(FALSE);
			m_radioIncludeSecurityCode.EnableWindow(FALSE);
			m_radioIncludeLogin.EnableWindow(FALSE);
		}

	}NxCatchAll("Error initializing the Edit NexWeb Login screen")
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//(e.lally 2009-01-28) PLID 32814 - Put the Add Login logic in its own function.
//	Allows the caller to specify a default username to populate in the input box.
BOOL CNexWebLoginInfoDlg::AddLogin(CString strDefaultUserName /* = "" */)
{
	// Popup an input dialog for the username
	CString strUserName = strDefaultUserName;
	//(e.lally 2009-01-28) PLID 32814 - Expanded the username to 50 characters.
	if (IDOK == InputBoxLimited(this, "Enter a new username:", strUserName, "",50,false,false,NULL)) {

		//they clicked ok, so check the name
		if (ReturnsRecords("SELECT PersonID FROM NexWebLoginInfoT WHERE UserName = '%s'", _Q(strUserName))) {

			MsgBox("That username is already taken, please select another name");
			strUserName = "";
			AddLogin(strUserName);
		}
		else {

			if (strUserName.IsEmpty() ) {
				MsgBox("You cannot have a blank username");
				AddLogin(strUserName);
			}
			//(e.lally 2009-01-28) PLID 32814 - Expanded the username to 50 characters.
			else if (strUserName.GetLength() > 50) {
				MsgBox("The username must be less than 50 characters");
				AddLogin(strUserName);
			}
			else {

			/*	long nLevelID;
				//see if this is their first username
				if (ReturnsRecords("SELECT Username FROM NexwebLoginInfoT WHERE PersonID = %li", m_nPersonID)) {

					//go ahead and add the name
					//default the level to be limited
					/*nLevelID = 1;
					ExecuteSql("INSERT INTO NexWebLoginInfoT (PersonID, UserName, Password, Enabled, UserLevel) VALUES "
						" (%li, '%s', '', 1, 1) ", m_nPersonID, strUserName);

					
				}
				else {
					//this is their first login, so default the level to be administrative
					nLevelID = 3;
					ExecuteSql("INSERT INTO NexWebLoginInfoT (PersonID, UserName, Password, Enabled, UserLevel) VALUES "
						" (%li, '%s', '', 1, 3) ", m_nPersonID, strUserName);

				
				}*/

				//PLID 21861 - taking out userlevel for now since its not implemented
				//(e.lally 2009-01-22) PLID 32812 - Set the Must Change Pasword flag
				// (j.jones 2009-05-01 10:27) - PLID 33853 - the password has to be NULL if blank
					ExecuteSql("INSERT INTO NexWebLoginInfoT (PersonID, UserName, Password, Enabled, PWExpireNextLogin) VALUES "
						" (%li, '%s', NULL, 1, 1) ", m_nPersonID, _Q(strUserName));


				//now add it into the datalist
				//(e.lally 2009-01-22) PLID 32812 - Updated to use column enums, added MustChangePass value
				IRowSettingsPtr pRow = m_pLoginList->GetRow(-1);
				pRow->PutValue(llcPersonID, m_nPersonID);
				pRow->PutValue(llcUserName, _variant_t(strUserName));
				pRow->PutValue(llcPassword, _variant_t(""));
				//pRow->PutValue(3, nLevelID);
				pRow->PutValue(llcEnabled, (long)1);
				pRow->PutValue(llcMustChangePass, (long)1);
				// (b.savon 2012-08-27 15:27) - PLID 50584 - add a default
				pRow->PutValue(llcLocked, g_cvarFalse);
				m_pLoginList->InsertRow(pRow, 0);
				

				//now open up the password field so that they know to fill it in
				CNexWebChangePasswordDlg dlg(m_nPersonID, strUserName, this);
				BOOL bContinue = TRUE;
				long nResult = dlg.DoModal();
				while (bContinue && nResult != IDOK) {
					if (nResult == IDCANCEL) {
						if (IDYES == MsgBox(MB_YESNO, "You must enter a password for this login, do you want to cancel adding this login?")) {
							// (r.farnworth 2013-11-26 14:14) - PLID 59520 - Need to delete referenced row in NexWebCCDAAccessHistoryT before deleting LoginInfo
							// (r.gonet 2015-02-18 10:34) - PLID 64437 - Don't remove the access history. That could change MU measure numbers. Parameterized.
							ExecuteParamSql("DELETE FROM NexwebLoginInfoT WHERE PersonID = {INT} and UserName = {STRING}", m_nPersonID, strUserName);
							m_pLoginList->RemoveRow(0);								
							return FALSE;
						}
						else {
							//pop up the dialog again
							nResult = dlg.DoModal();
						}
					}
				}
				
				if (nResult == IDOK) {
					//just put *s in the password field
					//(e.lally 2009-01-22) PLID 32481 - Updated to use column enum
					pRow->PutValue(llcPassword, "*****");
					return TRUE;
				}
				else{
					return FALSE;
				}
			}

		}
	}
	return FALSE;
}
void CNexWebLoginInfoDlg::OnAddLogin() 
{
	try {
		//(e.lally 2011-08-09) PLID 37287 - Check create permissions
		if(!CheckCurrentUserPermissions(bioPatientNexWebLogin, sptCreate)){
			return;
		}
		AddLogin();
	}NxCatchAll("Error adding Login");
}

void CNexWebLoginInfoDlg::OnExitDialog() 
{
	OnOK();
	
}

void CNexWebLoginInfoDlg::OnRemoveLogin() 
{
	try {
		//(e.lally 2011-08-09) PLID 37287 - Check delete permissions
		if(!CheckCurrentUserPermissions(bioPatientNexWebLogin, sptDelete)){
			return;
		}

		long nCurSel = m_pLoginList->CurSel;
	
		if (nCurSel == -1){
			MsgBox("Please select a Login to delete");
			return;
		}

		if (IDYES == MsgBox(MB_YESNO, "This will permanently delete this login, are you sure you wish to continue?")) {

			//delete the delete
			CString strUserName = VarString(m_pLoginList->GetValue(nCurSel, 1));
		
			// (r.farnworth 2013-11-26 14:14) - PLID 59520 - Need to delete referenced row in NexWebCCDAAccessHistoryT before deleting LoginInfo
			// (r.gonet 2015-02-18 10:34) - PLID 64437 - Don't remove the access history. That could change MU measure numbers. Parameterized.
			ExecuteParamSql("DELETE FROM NexWebLoginInfoT WHERE PersonID = {INT} AND UserName = {STRING}", m_nPersonID, strUserName);

			//remove the row
			m_pLoginList->RemoveRow(nCurSel);


		}
	}NxCatchAll("Error deleting Login");


	
}

BEGIN_EVENTSINK_MAP(CNexWebLoginInfoDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNexWebLoginInfoDlg)
	ON_EVENT(CNexWebLoginInfoDlg, IDC_LOGIN_LIST, 10 /* EditingFinished */, OnEditingFinishedLoginList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNexWebLoginInfoDlg, IDC_LOGIN_LIST, 9 /* EditingFinishing */, OnEditingFinishingLoginList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CNexWebLoginInfoDlg, IDC_LOGIN_LIST, 9 /* EditingStarting */, OnEditingFinishingLoginList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CNexWebLoginInfoDlg, IDC_LOGIN_LIST, 8 /* EditingStarting */, OnEditingStartingLoginList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNexWebLoginInfoDlg::OnEditingFinishedLoginList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		//(e.lally 2011-08-09) PLID 37287 - Don't go any further if they aren't commiting
		if(!bCommit){
			return;
		}

		//(e.lally 2009-01-22) PLID 32481 - Changed to enum column references
		switch (nCol) {

			case llcUserName: {

				CString strOldValue = VarString(varOldValue);
				CString strNewValue = VarString(varNewValue);

				//we already checked for our bad cases, so lets continue
				ExecuteSql("Update NexWebLoginInfoT SET UserName = '%s' WHERE PersonID = %li AND UserName = '%s'", _Q(strNewValue), m_nPersonID, _Q(strOldValue)); 
				}
			break;

			case llcPassword: {
				CString strUserName = VarString(m_pLoginList->GetValue(nRow, llcUserName));
				CString strNewValue = VarString(varNewValue);

				//we already checked for our bad cases, so lets continue
				// (j.jones 2009-04-30 14:19) - PLID 33853 - used AES encryption on the password
				ExecuteSql("Update NexWebLoginInfoT SET Password = %s WHERE PersonID = %li AND UserName = '%s'", EncryptStringForSql(strNewValue), m_nPersonID, _Q(strUserName)); 

				//now set it to be *'s
				m_pLoginList->PutValue(nRow, nCol, "*****");

					}
				
			break;

			/*case 3: {
				
				CString strUserName = VarString(m_pLoginList->GetValue(nRow, llcUserName));

				ExecuteSql("Update NexWebLoginInfoT SET UserLevel = %li WHERE PersonID = %li AND UserName = '%s'", VarLong(varNewValue), m_nPersonID, strUserName);
				}

			break;*/

			case llcEnabled: 
				{
				CString strUserName = VarString(m_pLoginList->GetValue(nRow, llcUserName));

				ExecuteSql("Update NexWebLoginInfoT SET Enabled = %li WHERE PersonID = %li AND UserName = '%s'", VarBool(varNewValue), m_nPersonID, _Q(strUserName));
				}
			break;

			//(e.lally 2009-01-22) PLID 32812 - Set/Clear the flag for making the patient change their password at the next login.
			case llcMustChangePass:
			{
				CString strUserName = VarString(m_pLoginList->GetValue(nRow, llcUserName));
				ExecuteSql("Update NexWebLoginInfoT SET PWExpireNextLogin = %li WHERE PersonID = %li AND UserName = '%s'", VarBool(varNewValue), m_nPersonID, _Q(strUserName));
			}
			break;

			// (b.savon 2012-08-16 17:44) - PLID 50584 - Set/Clear the flag for making the patient's account lock
			case llcLocked:
			{
				BOOL bLocked = VarBool(varOldValue, FALSE);
				CString strUserName = VarString(m_pLoginList->GetValue(nRow, llcUserName), "");
				CSqlFragment sqlUpdate;
				if( bLocked ){
					if( IDYES == AfxMessageBox("It is recommended that the patient changes their password.\r\n\r\nWould you like to change the patient password now?",
						MB_YESNO | MB_ICONQUESTION ) ){

						OnChangePassword();
					}
					sqlUpdate = CSqlFragment(
					"Update NexWebLoginInfoT SET LockedOutTime = (CASE WHEN {BOOL} = 0 THEN NULL ELSE GETDATE() END), \r\n"
					"LockedOut = {BOOL}, LockedOutCycle = 0, LoginAttempt = 0 WHERE PersonID = {INT} AND UserName = {STRING} \r\n",
					VarBool(varNewValue), VarBool(varNewValue), m_nPersonID, strUserName);
				}else{ // (b.savon 2012-09-13 15:01) - PLID 50584 - Perma-lock
					sqlUpdate = CSqlFragment(
					"Update NexWebLoginInfoT SET LockedOutTime = NULL, \r\n"
					"LockedOut = 1, LockedOutCycle = -1, LoginAttempt = 0 WHERE PersonID = {INT} AND UserName = {STRING} \r\n",
					m_nPersonID, strUserName);
				}

				// (b.savon 2012-08-27 15:34) - PLID 50584 - Use param
				 
				ExecuteParamSql("{SQL}",sqlUpdate); 
							
			}
			break;
		}	
	}NxCatchAll("Error Updating");
}

void CNexWebLoginInfoDlg::OnEditingFinishingLoginList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

		//(e.lally 2011-08-09) PLID 37287 - Don't go any further if they aren't commiting
		if(*pbCommit == FALSE){
			return;
		}
		//(e.lally 2011-08-09) PLID 37287 - Check write permissions
		else if(!CheckCurrentUserPermissions(bioPatientNexWebLogin, sptWrite)){
			*pbContinue = TRUE;
			*pbCommit = FALSE;
			return;
		}
	
		//(e.lally 2009-01-22) PLID 32481 - Changed to enum column references
		switch (nCol) {

		case llcUserName:{
				//check to make sure that the username hasn't been take
				CString strNewValue, strOldValue;
				strOldValue = VarString(varOldValue);
				strNewValue = VarString(pvarNewValue);

				if (strOldValue.CompareNoCase(strNewValue) != 0) {

					//now make sure that its not blank
					if (strNewValue.IsEmpty()) {
						MsgBox("You can't have a blank username");
						*pbCommit = FALSE;
						*pbContinue = FALSE;
						
					}
					else {
						//now make sure its not too long
						//(e.lally 2009-01-28) PLID 32814 - Expanded the username to 50 characters.
						if (strNewValue.GetLength() > 50) {
							MsgBox("Usernames can only be 50 characters.  Please shorten the username");
							*pbCommit = FALSE;
							*pbContinue = FALSE;
						}
						else {
							if (ReturnsRecords("SELECT PersonID FROM NexWebLoginInfoT WHERE UserName = '%s'", _Q(strNewValue))) {
								MsgBox("That Username has already been taken, please pick another");
								*pbCommit = FALSE;
								*pbContinue = FALSE;
							}
						}
						
					}
				}

			   }
			break;

		case llcPassword: {
				CString strNewValue;
				strNewValue = VarString(pvarNewValue);
				//check to make sure it isn't blank
				if (strNewValue.IsEmpty()) {
						MsgBox("You can't have a blank password");
						*pbCommit = FALSE;
						*pbContinue = FALSE;
				}
				//(e.lally 2009-01-28) PLID 32814 - Expanded the Password to 50 characters.
				else if (strNewValue.GetLength() > 50) {
					MsgBox("Passwords can only be 50 characters.  Please shorten the passowrd");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
				}
				}
				
			break;

	/*	case 3:
			//level
		break;*/

		case llcEnabled:
				//enabled
			break;

		case llcMustChangePass:
				//Must change password
			break;
		}
		
	}NxCatchAllCall("Error in OnEditingFinishingLoginList", *pbCommit = FALSE; *pbContinue = FALSE;);
}

void CNexWebLoginInfoDlg::OnRequeryFinishedLoginList(short nFlags) 
{
	//(e.lally 2009-01-22) PLID 32481 - Added try/catch
	try {
		//blank out the password column
		for (int i = 0; i < m_pLoginList->GetRowCount(); i++) {
			//(e.lally 2009-01-22) PLID 32481 - Updated to use column enum
			m_pLoginList->PutValue(i, llcPassword, "*****");
		}
	}NxCatchAll("Error in OnRequeryFinishedLoginList");
}

void CNexWebLoginInfoDlg::OnEditingStartingLoginList(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		//(e.lally 2011-08-09) PLID 37287 - Don't even let them start editing if they don't have permission.
		if(!(GetCurrentUserPermissions(bioPatientNexWebLogin) & (sptWrite|sptWriteWithPass)) ){
			if(!CheckCurrentUserPermissions(bioPatientNexWebLogin, sptWrite)){
				*pbContinue = FALSE;
				return;
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CNexWebLoginInfoDlg::OnChangePassword() 
{
	try {
		//(e.lally 2011-08-09) PLID 37287 - Check the change password permissions
		if(!CheckCurrentUserPermissions(bioPatientNexWebLogin, sptDynamic0)){
			return;
		}

		long nCurSel = m_pLoginList->CurSel;
		if (nCurSel == -1) {
			MessageBox("Please select a username.");
			return;
		}

		CString strUserName;
		//(e.lally 2009-01-22) PLID 32481 - Updated to use column enum
		strUserName = VarString(m_pLoginList->GetValue(nCurSel, llcUserName));

		CNexWebChangePasswordDlg dlg(m_nPersonID, strUserName, this);
		//(e.lally 2009-01-22)PLID 32812 - If the password was changed, we should update the Must Change Password flag
		if(IDOK == dlg.DoModal()){
			//For simplicity, requery the login list.
			m_pLoginList->Requery();
		}
	}NxCatchAll("Error changing password");
	
	
}

//(e.lally 2009-01-28) PLID 32814 - Opens the default email program with a new email, prompting for
//	a valid email address if one does not exist, and pre-populates the email with an address, subject and
//	selected Security Code or Username and Password information (forcing creation of info if none exists)
//	in the body of the email.
void CNexWebLoginInfoDlg::OnCreateNexWebEmail()
{
	try {
		//(e.lally 2011-08-09) PLID 37287 - Check the Email credentials permissions
		if(!CheckCurrentUserPermissions(bioPatientNexWebLogin, sptDynamic2)){
			return;
		}
		
		UpdateData(TRUE);
		if(m_radioIncludeSecurityCode.GetCheck() != 0 && m_bEnableSecurityCode == FALSE){
			//User wants to send the security code, but one has not been created yet. Let's prompt them.
			if(MsgBox(MB_OKCANCEL, "You have selected to send the patient's Security Code, but one has not been created.  Would you like to have one generated now?") == IDCANCEL){
				return;
			}
			else{
				SetUseSecurityCode(TRUE);
			}
		}


		//Check if their email address is private, then warn.
		BOOL bEmailPrivate = TRUE;
		CString strEmailAddress, strOldEmailAddress;
		_RecordsetPtr rsEmail = CreateParamRecordset("SELECT Email, PrivEmail FROM PersonT WHERE ID = {INT} ", m_nPersonID);
		if(!rsEmail->eof){
			//Check if the patient has an email address
			strEmailAddress = AdoFldString(rsEmail, "Email", "");
			strOldEmailAddress = strEmailAddress;
			if(AdoFldBool(rsEmail, "PrivEmail") == 0){
				bEmailPrivate = FALSE;
			}
			else{
				bEmailPrivate = TRUE;
			}
		}
		else {
			//this shouldn't be possible
			ASSERT(FALSE);
		}
		rsEmail->Close();

		//Prompt if the privacy for email is checked.
		if(bEmailPrivate) {
			if(MsgBox(MB_YESNO, "This patient is set for privacy on their email.  Are you sure you wish to send this message?") == IDNO){
				return;
			}
		}
		
		BOOL bIsValid = IsValidEmail(strEmailAddress);
		BOOL bContinueLoop = TRUE;
		CString strPrompt;
		while(bIsValid == FALSE && bContinueLoop != FALSE){
			if(strEmailAddress.IsEmpty()){
				strPrompt = "Enter the email address for this patient:";
			}
			else{
				strPrompt = "Enter a valid email address for this patient:";
			}

			//Prompt user to enter an email address
			if(IDCANCEL == InputBoxLimited(this, strPrompt, strEmailAddress, "", 50, false, false, "Cancel")){
				//The user cancelled the email address entry. Return.
				return;
			}

			//Is the email valid now?
			bIsValid = IsValidEmail(strEmailAddress);
			if(bIsValid == FALSE){
				//Still not valid, check if they want to use this email address anyways
				if(IDYES == AfxMessageBox("The email address entered is not valid. Do you still want to continue?", MB_YESNO)){
					bContinueLoop = FALSE;
				}
			}
			if(bIsValid != FALSE || bContinueLoop == FALSE){
				if(strOldEmailAddress != strEmailAddress){
					//(e.lally 2009-01-29) PLID 32814 - We have a valid email or the user wants to keep this entry. 
					//	We should save it.
					ExecuteSql("UPDATE PersonT SET Email = '%s' WHERE ID = %li", _Q(strEmailAddress), m_nPersonID);
					//auditing
					CString strPatientName = GetExistingPatientName(m_nPersonID);
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(m_nPersonID, strPatientName, nAuditID, aeiPatientEmail, m_nPersonID, strOldEmailAddress, strEmailAddress, aepMedium, aetChanged);
				}
			}
		}

		//(e.lally 2009-01-28) PLID 32814 - Now that we have an email address, check if we are 
		//	sending a username and password and if one exists. Otherwise, we'll need one created, 
		//	utilizing the email address as the default for the new username.
		CString strUsername, strPassword;
		if(m_radioIncludeLogin.GetCheck()){
			//(e.lally 2009-01-28) PLID 32814 - check if they have a valid username and password already.
			//Get the most recent enabled entry.
			_RecordsetPtr rsLogin = CreateParamRecordset("SELECT top 1 Username, Password FROM NexWebLoginInfoT "
				"WHERE PersonID = {INT} AND Enabled = 1 "
				"ORDER BY CreatedDate DESC ", m_nPersonID);
			if(!rsLogin->eof){
				strUsername = AdoFldString(rsLogin, "Username", "");
				// (j.jones 2009-04-30 14:19) - PLID 33853 - used AES encryption on the password
				strPassword = DecryptStringFromVariant(rsLogin->Fields->Item["Password"]->Value);

				//Close the recordset once we are done with it.
				rsLogin->Close();
			}
			else{
				//Close the recordset, we might be reusing it
				rsLogin->Close();
				//Create a username and password
				if(AddLogin(strEmailAddress) != FALSE){
					rsLogin = CreateParamRecordset("SELECT top 1 Username, Password FROM NexWebLoginInfoT "
						"WHERE PersonID = {INT} AND Enabled = 1 "
						"ORDER BY CreatedDate DESC ", m_nPersonID);
					if(!rsLogin->eof){
						strUsername = AdoFldString(rsLogin, "Username", "");
						// (j.jones 2009-04-30 14:19) - PLID 33853 - used AES encryption on the password
						strPassword = DecryptStringFromVariant(rsLogin->Fields->Item["Password"]->Value);

						//Close the recordset once we are done with it.
						rsLogin->Close();
					}
					else{
						//Close the recordset once we are done with it.
						rsLogin->Close();

						//Still no username and password, give up
						return;
					}
				}
				else{
					//Still no username and password, give up
					return;
				}
			}
		}
		

		
		//(e.lally 2009-01-28) PLID 32814 - Now for the email creation. 
		//		Copied most of this from the General1Dlg code for OnEmail.
		//DRT 7/14/03 - Added the prompt for a subject.  Once that is entered, we can put a record
		//		in the history that an email was sent. Note that we have no control over any of this, 
		//		so we're only opening the default email client with the given subject.  If they choose
		//		to cancel later, we don't know.
		CString strSubject, strHistory, strBody;
		//(e.lally 2009-01-28) PLID 32814 - Remember the last email subject - global
		strSubject = GetRemotePropertyText("NexWebEmailSubject", "", 0, "<None>", true);
		// (j.jones 2006-09-18 12:47) - PLID 22545 - limit the subject text to 255 characters, which is Outlook's maximum
		// and thus, by the commonality of said number, we can presume is the maximum in other email clients
		if(InputBoxLimited(this, "Enter your email subject:", strSubject, "", 255, false, false, "Cancel") == IDOK) {
			//(j.anspach 05-31-2006 13:19 PLID 20272) - Entering an ampersand into the subject line causes
			//  everything beyond it to get cut off.  We need to change any ampersands in the subject into
			//  their hex code %26.... while we're at it, let's make it so they can't accidentally (or
			//  otherwise) put in their own hex codes ...
			//  I also saved strSubject to strHistory before making the changes so that we save it in the history
			//  tab correctly.
			// (a.walling 2006-10-05 13:03) - PLID 22869 - Replaced the ampersand replace with EncodeURL function.
			strHistory = strSubject;
			//(e.lally 2009-01-28) PLID 32814 - Save the last email subject - global
			SetRemotePropertyText("NexWebEmailSubject", strSubject, 0, "<None>");
			strSubject = EncodeURL(strSubject);

			//(e.lally 2009-01-28) PLID 32814 - Are we sending the security code or username and password?
			if(m_radioIncludeSecurityCode.GetCheck()){
				
				CString strSecurityCode;
				GetDlgItemText(IDC_EDIT_SECURITY_CODE, strSecurityCode);
				strBody = "Security Code: " + strSecurityCode;
			}
			else if(m_radioIncludeLogin.GetCheck()){
				
				strBody = "Username: " + strUsername + "\r\n" +
					"Password: " + strPassword;

			}

			strBody = EncodeURL(strBody);

			//DRT 7/16/03 - More improvements!  There is a preference for what they want to do now.
			//1 = always save in history, 2 = save if strSubject is not blank, 0 = never save
			long nSave = GetRemotePropertyInt("Gen1SaveEmails", 2, 0, "<None>", false);

			//(e.lally 2009-01-29) PLID 32814 - Build valid command string for the "mail to".
			CString strShellCmd = "mailto:" + EncodeURL(strEmailAddress);
			if(!strSubject.IsEmpty() || !strBody.IsEmpty()){
				//We have more than just an email address, add the '?'
				strShellCmd +="?";
				if(!strSubject.IsEmpty()){
					strShellCmd += "subject=" + strSubject;
				}
				if(!strBody.IsEmpty()){
					if(!strSubject.IsEmpty()){
						//We have a subject and body so add the '&'
						strShellCmd += "&";
					}
					strShellCmd += "body=" + strBody;
				}
			}
			//Run the command.
			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			if ((int)ShellExecute(GetSafeHwnd(), NULL, strShellCmd, NULL, "", SW_MAXIMIZE) < 32) {
				AfxMessageBox("Could not open default e-mail program");
			}
			else {
				if(nSave == 1 || (nSave == 2 && !strSubject.IsEmpty())) {
					try {
						//email started successfully
						//enter a note in the history tab
						CString strNote;
						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						strNote.Format("Email created for '%s' on %s.  Subject:  '%s'", strEmailAddress, FormatDateTimeForInterface(dtNow, DTF_STRIP_SECONDS, dtoDateTime, false), strHistory);

						// (j.jones 2008-09-04 15:28) - PLID 30288 - converted to use CreateNewMailSentEntry,
						// which creates the data in one batch and sends a tablechecker
						// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
						// Obviously this will contradict strNote if the server/workstation times are different, but if a new MailSent date is to be "now", then it
						// needs to be the server's "now". If the note becomes an issue, we can address it in a future item.
						CreateNewMailSentEntry(GetActivePatientID(), strNote, "", "", GetCurrentUserName(), "", GetCurrentLocationID());

					} NxCatchAll("Error saving email to history");
				}
				else {
					//nSave is 0, we do not want to save this in the history
				}
			}
		}

	}NxCatchAll("Error creating new email");
}

//(e.lally 2009-01-28) PLID 32814 - This allows us to turn the Security Code use on and off.
void CNexWebLoginInfoDlg::SetUseSecurityCode(BOOL bUseSecurityCode)
{
	if(bUseSecurityCode == FALSE){
		m_checkEnableSecurityCode.SetCheck(FALSE);
		//Clear out the security code
		// (b.savon 2012-08-21 10:49) - PLID 50589 - Clear the creation date
		// (r.gonet 2014-12-31 10:27) - PLID 64498 - Don't clear the InitialSecurityCodeCreationDate. That must stay since it means the first time the patient had access to the portal,
		// which is useful in certain MU measures.
		ExecuteParamSql("UPDATE PatientsT SET SecurityCode = NULL, SecurityCodeCreationDate = NULL WHERE PersonID = {INT}", m_nPersonID);
		SetDlgItemText(IDC_EDIT_SECURITY_CODE, "");
		SetDlgItemText(IDC_EDIT_SECURITY_CODE_EXPIRATION, "<None>");

		// (d.singleton 2011-10-11 12:32) - PLID 42102 audit security code
		AuditEvent(m_nPersonID, m_strPersonName, BeginNewAuditEvent(), aeiPatientSecurityCode, m_nPersonID, m_strSecurityCode, "", aepHigh, aetChanged);
	}
	else {
		m_checkEnableSecurityCode.SetCheck(TRUE);
		//(e.lally 2009-01-26) PLID 32811 - This gives us a random, unique security code, but does not
		// *guarantee* that it will remain unique before we can store it. The probably of there being 
		//a conflict is super low though.
		// (b.savon 2012-08-21 10:50) - PLID 50589 - Even though this is set within the GenerateUniquePatientSecurityCode code branch, let's
		// follow eric's reasoning to be safe. Added to the query so we can update the expiration date.
		// (r.gonet 2014-12-31 10:39) - PLID 64498 - We now store the date a security code was first given to the patient. It then remains constant through any changes to the current security code.
		CString strSecurityCode = GenerateUniquePatientSecurityCode(GetRemoteData());
		_RecordsetPtr rs = CreateParamRecordset("SET NOCOUNT ON; "
												"DECLARE @PersonID INT SET @PersonID = {INT}; "
												"UPDATE PatientsT SET SecurityCode = {STRING}, SecurityCodeCreationDate = GETDATE() WHERE PersonID = @PersonID "
												"IF (SELECT InitialSecurityCodeCreationDate FROM PatientsT WHERE PersonID = @PersonID) IS NULL "
												"BEGIN "
												"	UPDATE PatientsT SET InitialSecurityCodeCreationDate = SecurityCodeCreationDate WHERE PersonID = @PersonID "
												"END "
												"SET NOCOUNT OFF; "
												"SELECT SecurityCodeCreationDate, "  
												"(SELECT COALESCE((SELECT IntParam FROM ConfigRT WHERE Name = 'NexWebSecurityCodeExpire'), -1)) AS SecurityCodeExpiration "
												"FROM PatientsT WHERE PersonID = @PersonID", m_nPersonID, strSecurityCode);
		// (b.savon 2012-08-21 11:19) - PLID 50589 - Set the secuirtycode expiration label
		SetSecurityCodeLabel(rs);

		SetDlgItemText(IDC_EDIT_SECURITY_CODE, strSecurityCode);
		
		// (d.singleton 2011-11-08 14:42) - PLID 42102 - assign security code to member variable
		m_strSecurityCode = strSecurityCode;

		// (d.singleton 2011-10-11 11:31) - PLID 42102 - add auditing for security codes.
		AuditEvent(m_nPersonID, m_strPersonName, BeginNewAuditEvent(), aeiPatientSecurityCode, m_nPersonID, "", strSecurityCode, aepHigh, aetChanged);
	}
}

void CNexWebLoginInfoDlg::OnCheckUseSecurityCode()
{
	try {
		//(e.lally 2011-08-09) PLID 37287 - Check the security code permissions
		if(!CheckCurrentUserPermissions(bioPatientNexWebLogin, sptDynamic1)){
			UpdateData(FALSE);
			/*BOOL bUseSecurityCode = TRUE;
			if(m_bEnableSecurityCode == FALSE){
				bUseSecurityCode = FALSE;
			}*/
			return;
		}

		//(e.lally 2009-01-26) PLID 32811 - Load the security code
		UpdateData(TRUE);
		BOOL bUseSecurityCode = TRUE;
		if(m_bEnableSecurityCode == FALSE){
			bUseSecurityCode = FALSE;
		}
		//(e.lally 2009-01-28) PLID 32814 - Actually turn the security code on or off.
		SetUseSecurityCode(bUseSecurityCode);

	}NxCatchAll("Error updating use of Security Code");
}

void CNexWebLoginInfoDlg::HandleEmailUseRadio()
{
	int nIncludeSecurityCode = 1;
	if(m_radioIncludeLogin.GetCheck()){
		nIncludeSecurityCode = 0;
	}
	SetRemotePropertyInt("NexWebEmailUseSecurityCode", nIncludeSecurityCode, 0, GetCurrentUserName());
}

void CNexWebLoginInfoDlg::OnIncludeSecurityCode()
{
	try {
		HandleEmailUseRadio();
	}NxCatchAll("Error selecting to include Security Code");
}

void CNexWebLoginInfoDlg::OnIncludeLogin()
{
	try {
		HandleEmailUseRadio();
	}NxCatchAll("Error selecting to include Username and Password");
}

// (b.savon 2012-08-21 11:19) - PLID 50589 - Hand in a recordsetptr and set the code expiration accordingly
void CNexWebLoginInfoDlg::SetSecurityCodeLabel(_RecordsetPtr rs)
{
	try{
		CString strSecurityCodeExpiration;
		if( !rs->eof ){
			COleDateTime dtCreation = AdoFldDateTime(rs, "SecurityCodeCreationDate", COleDateTime());
			int nExpirationDays = AdoFldLong(rs, "SecurityCodeExpiration", -1);
			if( dtCreation != COleDateTime() ){
				if( nExpirationDays > 0 ){
					COleDateTimeSpan dsSpan = COleDateTimeSpan(nExpirationDays, 0, 0, 0);
					strSecurityCodeExpiration = FormatDateTimeForInterface(dtCreation+dsSpan, DTF_STRIP_SECONDS, dtoDate);
				}else{
					strSecurityCodeExpiration = "<None>";
				}
			}else{
				strSecurityCodeExpiration = "<None>";
			}
		}else{
			strSecurityCodeExpiration = "<None>";
		}
		
		if(!strSecurityCodeExpiration.IsEmpty()){
			SetDlgItemText(IDC_EDIT_SECURITY_CODE_EXPIRATION, strSecurityCodeExpiration);
		}
	}NxCatchAll(__FUNCTION__);
}
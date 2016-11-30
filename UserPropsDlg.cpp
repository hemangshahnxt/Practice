// UserPropsDlg.cpp : implementation file
//
#include "stdafx.h"
#include "AdministratorRc.h"
#include "UserPropsDlg.h"
#include "contactsrc.h"
#include "copypermissionsdlg.h"
#include "mainfrm.h"
#include "PasswordConfigDlg.h"
#include "AuditTrail.h"
#include "ConfigurePermissionGroupsDlg.h"
#include "NxAPI.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CUserPropsDlg dialog


CUserPropsDlg::CUserPropsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CUserPropsDlg::IDD, pParent)
{
	m_bMemory = false;
	m_bInactivity = false;
	m_bAdministrator = false;
	m_bExpires = false;
	// (j.politis 2015-07-08 14:16) - PLID 64164 - It would be convenient to have a preference that allows users to set newly added contacts to be added to all practice locatoins, rather than just one.
	m_bAllLocations = false;
	m_nInactivityMinutes = 30;
	m_nPwExpireDays = 30;
	m_nUserID = -1;
	m_bPasswordExpiresNextLogin = FALSE;
	m_nFailedLogins = 0;
	m_bShowConfigureGroups = TRUE;
	// (b.savon 2012-02-21 14:59) - PLID 48274 - Respect password rules
	m_bNewUser = FALSE;
	//{{AFX_DATA_INIT(CUserPropsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CUserPropsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUserPropsDlg)
	DDX_Control(pDX, IDC_CHECK_PASSWORD_EXPIRES_NEXT_LOGIN, m_checkPasswordExpiresNextLogin);
	DDX_Control(pDX, IDC_BTN_CONFIGURE_PASSWORD_STRENGTH, m_btnConfigPasswordStrength);
	DDX_Control(pDX, IDC_ADMINISTRATOR_CHECK, m_checkAdministrator);
	DDX_Control(pDX, IDC_INACTIVITY_CHK, m_inactivity);
	DDX_Control(pDX, IDC_USER_NAME_BOX, m_userName);
	DDX_Control(pDX, IDC_SAVE_PASSWORD_CHK, m_memory);
	DDX_Control(pDX, IDC_EXPIRE_CHK, m_expires);
	DDX_Control(pDX, IDC_USER_PASS_BOX, m_nxeditUserPassBox);
	DDX_Control(pDX, IDC_USER_PASS_VER_BOX, m_nxeditUserPassVerBox);
	DDX_Control(pDX, IDC_EDIT_PW_EXPIRES, m_nxeditEditPwExpires);
	DDX_Control(pDX, IDC_INACTIVITY_BOX, m_nxeditInactivityBox);
	DDX_Control(pDX, IDC_BTN_CLEAR_PERMISSIONS, m_btnClearPermissions);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_IMPORT_PERMS_FROM, m_btnImportPermsFrom);
	DDX_Control(pDX, IDC_UNLOCK_LABEL, m_nxsUnlockLabel);
	DDX_Control(pDX, IDC_UNLOCK_USER, m_btnUnlockUser);
	DDX_Control(pDX, IDC_CONGIFURE_GROUPS, m_btnConfigureGroups);
	
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CUserPropsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CUserPropsDlg)
	ON_BN_CLICKED(IDC_INACTIVITY_CHK, OnInactivityChk)
	ON_BN_CLICKED(IDC_BTN_IMPORT_PERMS_FROM, OnBtnImportPermsFrom)
	ON_BN_CLICKED(IDC_ADMINISTRATOR_CHECK, OnAdministratorCheck)
	ON_BN_CLICKED(IDC_EXPIRE_CHK, OnExpireCheck)
	ON_BN_CLICKED(IDC_BTN_CLEAR_PERMISSIONS, OnBtnClearPermissions)
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_PASSWORD_STRENGTH, OnBtnConfigurePasswordStrength)
	ON_BN_CLICKED(IDC_UNLOCK_USER, &CUserPropsDlg::OnUnlockUser)
	ON_BN_CLICKED(IDC_CONGIFURE_GROUPS, &CUserPropsDlg::OnBnClickedConfigureGroups)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CUserPropsDlg, CNxDialog)
	//{{AFX_EVENTSINK_MAP(CUserPropsDlg)
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CUserPropsDlg message handlers

BOOL CUserPropsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	m_btnClearPermissions.AutoSet(NXB_DELETE);
	m_btnImportPermsFrom.AutoSet(NXB_MODIFY);
	// (j.jones 2008-11-18 15:19) - PLID 28572 - added m_btnConfigPasswordStrength
	m_btnConfigPasswordStrength.AutoSet(NXB_MODIFY);
	//TES 4/30/2009 - PLID 28573 - Added m_btnUnlockUser
	m_btnUnlockUser.AutoSet(NXB_MODIFY);
	m_btnConfigureGroups.AutoSet(NXB_MODIFY);

	// (j.jones 2008-11-18 17:07) - PLID 32077 - disable the config. password strength button if no permissions
	if(!(GetCurrentUserPermissions(bioUserPasswordStrength) & SPT___W________ANDPASS)) {
		m_btnConfigPasswordStrength.EnableWindow(FALSE);
	}

	m_oldName = m_name;
	SetDlgItemText(IDC_USER_NAME_BOX, m_name);
	if (m_name.CollateNoCase("Administrator") == 0) {
		m_userName.SetReadOnly();
	}

	// (j.luckoski 2013-03-28 10:32) - PLID 55913 - Created providerID dropdown for linking users to providers
	m_ProvCombo = BindNxDataList2Ctrl(IDC_COMBO_PROVID,false);
	_RecordsetPtr prs = CreateParamRecordset("SELECT ProviderID, FullName from UsersT LEFT JOIN PersonT ON UsersT.ProviderID = PersonT.ID where UsersT.PersonID = {INT}", m_nUserID);

	long nProvID = 0;
	CString strProv = "<No Provider>";

	m_ProvCombo->Requery();
	m_ProvCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_ProvCombo->GetNewRow();
	pRow->PutValue(0, 0);
	pRow->PutValue(1, "<No Provider>");
	m_ProvCombo->AddRowSorted(pRow, NULL);

	if(!prs->eof) {
		nProvID = AdoFldLong(prs->Fields, "ProviderID",0);
		strProv = AdoFldString(prs->Fields, "FullName", "<No Provider>");
		// (j.luckoski 2013-04-01 11:15) - PLID 55913 - If provider is inactive, show the name but don't allow it in the dropdown
		if(!m_ProvCombo->SetSelByColumn(0,nProvID)) {
			//they may have an inactive provider
			_RecordsetPtr rsProv = CreateParamRecordset("SELECT FullName FROM PersonT WHERE ID = {INT}", nProvID);
			if(!rsProv->eof) {
				m_ProvCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "FullName", "<No Provider>")));
			}
			else 
				m_ProvCombo->PutCurSel(0);
		}
	}

	m_nOldProviderID = nProvID;
	m_strOldProvider = strProv;
	
	SetDlgItemText(IDC_USER_PASS_BOX, m_pass);
	SetDlgItemText(IDC_USER_PASS_VER_BOX, m_pass);
	SetDlgItemText(IDC_USER_FULLNAME, m_strFullName);
	SetDlgItemInt(IDC_INACTIVITY_BOX, m_nInactivityMinutes);
	SetDlgItemInt(IDC_EDIT_PW_EXPIRES, m_nPwExpireDays);	
	// (j.jones 2008-11-20 08:49) - PLID 23227 - added m_bPasswordExpiresNextLogin
	m_checkPasswordExpiresNextLogin.SetCheck(m_bPasswordExpiresNextLogin);
	m_memory.SetCheck(m_bMemory);
	m_checkAdministrator.SetCheck(m_bAdministrator);
	// (j.politis 2015-07-08 14:16) - PLID 64164 - It would be convenient to have a preference that allows users to set newly added contacts to be added to all practice locatoins, rather than just one.
	CheckDlgButton(IDC_ADDTO_ALL_LOCS_CHECK, m_bAllLocations ? BST_CHECKED : BST_UNCHECKED);
	GetDlgItem(IDC_BTN_IMPORT_PERMS_FROM)->EnableWindow(!m_bAdministrator);
	GetDlgItem(IDC_BTN_CLEAR_PERMISSIONS)->EnableWindow(!m_bAdministrator);
	m_inactivity.SetCheck(m_bInactivity);
	m_expires.SetCheck(m_bExpires);
	GetDlgItem(IDC_EDIT_PW_EXPIRES)->EnableWindow(m_bExpires);
	GetDlgItem(IDC_INACTIVITY_BOX)->EnableWindow(m_bInactivity);
	GetDlgItem(IDC_USER_NAME_BOX)->SetFocus();
	m_adwImportPermissionsFrom.RemoveAll();

	//The administrator checkbox and the import permission button should not even be enabled
	//if you do not have the permission to access it

	BOOL bCanEditPermissions = TRUE;
	// (j.jones 2009-05-26 15:47) - PLID 34315 - split into bioEditOwnPermissions and bioEditOtherUserPermissions
	if(m_nUserID == GetCurrentUserID()) {
		if(!(GetCurrentUserPermissions(bioEditOwnPermissions) & SPT___W_______) &&
			!(GetCurrentUserPermissions(bioEditOwnPermissions) & SPT___W________ANDPASS)) {
				
			bCanEditPermissions = FALSE;
		}
	}
	else {
		if(!(GetCurrentUserPermissions(bioEditOtherUserPermissions) & SPT___W_______) &&
			!(GetCurrentUserPermissions(bioEditOtherUserPermissions) & SPT___W________ANDPASS)) {
				
			bCanEditPermissions = FALSE;
		}
	}

	if(!bCanEditPermissions) {

		m_checkAdministrator.EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_IMPORT_PERMS_FROM)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_CLEAR_PERMISSIONS)->EnableWindow(FALSE);
	}

	//TES 4/30/2009 - PLID 28573 - Check whether this account is locked.
	// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Enabled
	if(GetRemotePropertyInt("LockAccountsOnFailedLogin", 1, 0, "<None>")) {
		long nMaxFailedLogins = GetRemotePropertyInt("FailedLoginsAllowed", 5, 0, "<None>");
		if(m_nFailedLogins >= nMaxFailedLogins) {
			//TES 4/30/2009 - PLID 28573 - Yes, it's locked, so enable the unlock options.
			m_nxsUnlockLabel.SetWindowText("This account is locked due to " + AsString(m_nFailedLogins) + " failed login attempts.");
			m_nxsUnlockLabel.ShowWindow(SW_SHOWNA);
			m_btnUnlockUser.EnableWindow(TRUE);
			m_btnUnlockUser.ShowWindow(SW_SHOWNA);
		}
	}

	// (j.gruber 2010-04-14 13:37) - PLID 38186 - only show the configure groups button if this is an existing user
	if (!m_bShowConfigureGroups){
		m_btnConfigureGroups.ShowWindow(SW_HIDE);
	}

	// (j.luckoski 2013-03-28 13:56) - PLID 55929 - Check permissions to whether we can change this dropdown.
	bool bCanEditProviderLink = true;
	if(!(GetCurrentUserPermissions(bioLinkUsersToProvider) & SPT___W_______)) {

			bCanEditProviderLink = false;
	}

	if(!bCanEditProviderLink) {
		m_ProvCombo->Enabled = FALSE;
	}

	return false;
}

void CUserPropsDlg::OnOK() 
{
	try {

		CString confirm;

		CString strPassword;
		GetDlgItemText(IDC_USER_PASS_BOX, strPassword);
		GetDlgItemText(IDC_USER_PASS_VER_BOX, confirm);
		GetDlgItemText(IDC_USER_NAME_BOX, m_name);
		m_nInactivityMinutes = GetDlgItemInt(IDC_INACTIVITY_BOX);
		m_nPwExpireDays = GetDlgItemInt(IDC_EDIT_PW_EXPIRES);

		m_name.TrimLeft();
		m_name.TrimRight();
		if (m_name == "") {
			MsgBox("The Username cannot be blank");
			return;
		}

		if (strPassword != confirm) {
			MsgBox("The Password doesn't match the Password Confirmation.\n\n"
				"Be sure to use the correct uppercase and lowercase characters in your passwords.");
			return;
		}
		
		// (j.jones 2009-04-30 14:09) - PLID 33745 - required a 1 minute minimum, not 2
		if (m_nInactivityMinutes < 1)
		{
			MsgBox("You must set the inactivity timer to at least one minute.");
			return;
		}

		if (m_nPwExpireDays < 1)
		{
			MsgBox("You must set the password expiration timer to at least one day.");
			return;
		}

		// (b.savon 2012-02-21 12:25) - PLID 48274 - Or when this is a new user
		// (j.jones 2008-11-20 11:18) - PLID 28572 - load up the password strength rules
		// and enforce them *only* of the password has changed
		if(strPassword != m_pass || m_bNewUser ){

			// (r.farnworth 2016-01-07 14:41) - PLID 67719 - Use the API to validate password strength
			NexTech_Accessor::_ChangePasswordPtr pChangePassword(__uuidof(NexTech_Accessor::ChangePassword));
				NexTech_Accessor::_UserPtr pUser(__uuidof(NexTech_Accessor::User));
				CString strUserID;
				strUserID.Format("%li", m_nUserID);
				pUser->Putusername(AsBstr(m_name));
				pUser->PutID(AsBstr(strUserID));
			pChangePassword->User = pUser;
			pChangePassword->PutCurrentPassword(AsBstr(m_pass));
			pChangePassword->PutNewPassword(AsBstr(strPassword));

			NexTech_Accessor::_ChangePasswordResultPtr pResult = GetAPI()->ValidatePasswordStrengthRules(GetAPISubkey(), GetAPILoginToken(), pChangePassword);

			if (pResult->Getstatus() != NexTech_Accessor::ChangePasswordStatus_Success) {
				MessageBox(pResult->GetMessageA(), "Nextech", MB_ICONERROR | MB_OK);
				return;
			}

			// (j.jones 2008-11-20 08:49) - PLID 23227 - if they changed their password and
			// they want it to expire on the next login, ask them if that's what they really want
			// (d.thompson 2009-03-09) - PLID 33147 - Do NOT give this warning if they are making a
			//		new user.
			if(m_nUserID != -1 && m_checkPasswordExpiresNextLogin.GetCheck()
				&& MessageBox("You are changing this user's password, but also have the 'password expires at next login' setting enabled.\n"
				"Do you wish to uncheck this setting since you are changing the password now?", "Practice", MB_ICONQUESTION|MB_YESNO) == IDYES) {
				m_checkPasswordExpiresNextLogin.SetCheck(FALSE);
			}
		}

		// (j.gruber 2010-04-14 13:23) - PLID 38186 - take this out here because we are asking down the line more
		/*if(!m_checkAdministrator.GetCheck() && IsRecordsetEmpty("SELECT ObjectID FROM PermissionT "
			"LEFT JOIN SecurityObjectT ON PermissionT.ObjectID = SecurityObjectT.ID "
			"INNER JOIN UsersT ON PermissionT.UserGroupID = UsersT.PersonID "
			"WHERE UserGroupID IN (%li)", m_nUserID) &&
			m_adwImportPermissionsFrom.GetSize() == 0) {

			if(IDNO == MessageBox("There are no permissions configured for this user. Are you sure you wish to save?",
				"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
		}*/

		{	
			_RecordsetPtr rs;
			CString sql;

			EnsureRemoteData();
			if (m_oldName.CollateNoCase(m_name) != 0)
			{
				// The username has changed
				rs = CreateParamRecordset("SELECT UserName FROM UsersT WHERE UserName = {STRING};", m_name);
				if (!rs->eof)
				{	rs->Close();
					MsgBox("Logins must be unique");
					return;
				}
				rs->Close();

				//DRT 10/15/2003 - PLID 9514 - Only update the name if the person we are editing is the current user!
				if(m_nUserID == GetCurrentUserID()) {
					SetCurrentUserName(m_name);
					GetMainFrame()->LoadTitleBarText();
				}
			}

			// (r.farnworth 2015-12-30 11:19) - PLID 67719 - Change Practice to call our new ChangeContactInformation API method instead of calling SQL from C++ code.
			if (m_ProvCombo->GetCurSel())
			{
				m_nNewProviderID = m_ProvCombo->GetCurSel()->GetValue(0);
			}
			else
			{
				m_nNewProviderID = 0;
			}

			m_bMemory = m_memory.GetCheck() ? true : false;
			m_bAdministrator = m_checkAdministrator.GetCheck() ? true : false;
			m_bInactivity = m_inactivity.GetCheck() ? true : false;
			m_bExpires = m_expires.GetCheck() ? true : false;
			// (j.politis 2015-07-08 14:16) - PLID 64164 - It would be convenient to have a preference that allows users to set newly added contacts to be added to all practice locatoins, rather than just one.
			m_bAllLocations = IsDlgButtonChecked(IDC_ADDTO_ALL_LOCS_CHECK) == BST_CHECKED ? true : false;
			// (j.jones 2008-11-20 08:49) - PLID 23227 - added m_bPasswordExpiresNextLogin
			m_bPasswordExpiresNextLogin = m_checkPasswordExpiresNextLogin.GetCheck() ? TRUE : FALSE;
			// (j.jones 2008-11-20 11:18) - PLID 28572 - don't assign m_pass until now
			m_pass = strPassword;
			
			CDialog::OnOK();
		}

	}NxCatchAll("Error in UserPropsDlg::OnOK()");
}

void CUserPropsDlg::OnInactivityChk() 
{
	m_bInactivity = m_inactivity.GetCheck()?true:false;
	GetDlgItem(IDC_INACTIVITY_BOX)->EnableWindow(m_bInactivity);
	
}

void CUserPropsDlg::OnBtnImportPermsFrom() 
{

	//DRT 4/3/03 - Only let them do this if they have bioEditPermissions
	// (j.jones 2009-05-26 15:47) - PLID 34315 - split into bioEditOwnPermissions and bioEditOtherUserPermissions
	if(m_nUserID == GetCurrentUserID()) {
		if(!CheckCurrentUserPermissions(bioEditOwnPermissions, sptWrite)) {
			return;
		}
	}
	else {
		if(!CheckCurrentUserPermissions(bioEditOtherUserPermissions, sptWrite)) {
			return;
		}
	}

	CCopyPermissionsDlg dlg(this);
	dlg.m_padwUserGroups = &m_adwImportPermissionsFrom;

	if (IDOK == dlg.DoModal())
	{
		if(dlg.m_bAdministratorSet) {
			m_checkAdministrator.SetCheck(TRUE);
			m_bAdministrator = TRUE;
			GetDlgItem(IDC_BTN_IMPORT_PERMS_FROM)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_CLEAR_PERMISSIONS)->EnableWindow(FALSE);
		}
	}
}

void CUserPropsDlg::OnAdministratorCheck() 
{
	BOOL bIsChecked = m_checkAdministrator.GetCheck();

	//JMJ 5/27/03 - The ability to edit the Administrator status should only be allowed if you can edit permissions

	// (j.jones 2009-05-26 15:47) - PLID 34315 - split into bioEditOwnPermissions and bioEditOtherUserPermissions
	if(m_nUserID == GetCurrentUserID()) {
		if(!CheckCurrentUserPermissions(bioEditOwnPermissions, sptWrite)) {
			m_checkAdministrator.SetCheck(!bIsChecked);
			return;
		}
	}
	else {
		if(!CheckCurrentUserPermissions(bioEditOtherUserPermissions, sptWrite)) {
			m_checkAdministrator.SetCheck(!bIsChecked);
			return;
		}
	}

	//we are unchecking the Administrator box
	if(!bIsChecked) {
		//check to ensure another Administrator exists, else warn
		if(IsRecordsetEmpty("SELECT Administrator FROM UsersT WHERE Administrator = 1 AND UsersT.PersonID > 0 AND PersonID <> %li",m_nUserID)) {
			MsgBox("At least one user must be an Administrator. If you need to remove the Administrator\n"
				"status from this user, you must first designate another user to be an Administrator.");
			m_checkAdministrator.SetCheck(TRUE);
			return;
		}
	}

	m_bAdministrator = bIsChecked?true:false;
	// (j.politis 2015-07-08 14:16) - PLID 64164 - It would be convenient to have a preference that allows users to set newly added contacts to be added to all practice locatoins, rather than just one.
	m_bAllLocations = m_bAdministrator;
	CheckDlgButton(IDC_ADDTO_ALL_LOCS_CHECK, m_bAllLocations ? BST_CHECKED : BST_UNCHECKED);
	GetDlgItem(IDC_ADDTO_ALL_LOCS_CHECK)->EnableWindow(!bIsChecked);
	GetDlgItem(IDC_BTN_IMPORT_PERMS_FROM)->EnableWindow(!bIsChecked);
	GetDlgItem(IDC_BTN_CLEAR_PERMISSIONS)->EnableWindow(!bIsChecked);
}

void CUserPropsDlg::OnExpireCheck()
{
	GetDlgItem(IDC_EDIT_PW_EXPIRES)->EnableWindow(m_expires.GetCheck());
}

void CUserPropsDlg::OnBtnClearPermissions() 
{
	try {

		if(m_nUserID == GetCurrentUserID()) {
			MsgBox("You cannot clear the permissions from the current user.");
			return;
		}

		if(IDNO == MessageBox("This action will remove all permissions from this user.\n"
			"Are you sure you wish to do this?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		ExecuteSql("UPDATE PermissionT SET Permissions = 0 WHERE UserGroupID = %li", m_nUserID);

		MsgBox("This user has now had all their permissions removed.");

	}NxCatchAll("Error clearing permissions.");
}

// (j.jones 2008-11-18 15:17) - PLID 28572 - added ability to configure password strength
void CUserPropsDlg::OnBtnConfigurePasswordStrength()
{
	try {

		// (j.jones 2008-11-18 17:07) - PLID 32077 - check permissions
		if(!CheckCurrentUserPermissions(bioUserPasswordStrength, sptWrite)) {
			return;
		}		

		CPasswordConfigDlg dlg(this);
		if(dlg.DoModal() == IDOK && dlg.m_bMadeAllUserPasswordsExpire) {
			//If the password strength changed, the user would have been prompted to
			//make all user passwords expire. If they did, m_bMadeAllUserPasswordsExpire
			//would be set to TRUE. Which means that we would have changed the current
			//user's setting. So reflect that change.
			m_checkPasswordExpiresNextLogin.SetCheck(TRUE);
		}

	}NxCatchAll("Error in CUserPropsDlg::OnBtnConfigurePasswordStrength");
}

void CUserPropsDlg::OnUnlockUser()
{
	try {
		//TES 4/30/2009 - PLID 28573 - Confirm
		if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to restore this user's access to NexTech Practice?")) {
			return;
		}

		//TES 4/30/2009 - PLID 28573 - Just reset the FailedLogins count.
		ASSERT(m_nUserID != -1);
		ExecuteParamSql("UPDATE UsersT SET FailedLogins = 0 WHERE PersonID = {INT}", m_nUserID);

		//TES 4/30/2009 - PLID 28573 - Audit
		AuditEvent(-1, m_name, BeginNewAuditEvent(), aeiUserUnlocked, m_nUserID, "", "Account '" + m_name + "' unlocked", aepHigh);

		//TES 4/30/2009 - PLID 28573 - Let them know.
		MsgBox("The '%s' account has been unlocked, they may now log into NexTech Practice.", m_name);

		//TES 4/30/2009 - PLID 28573 - Hide the (now irrelevant) controls.
		m_btnUnlockUser.ShowWindow(SW_HIDE);
		m_btnUnlockUser.EnableWindow(FALSE);
		m_nxsUnlockLabel.ShowWindow(SW_HIDE);
	}NxCatchAll("Error in CUserPropsDlg::OnUnlockUser()");
}

// (j.gruber 2010-04-13 16:31) - PLID 38186 - took out import and clear permissions buttons and added configure groups
void CUserPropsDlg::OnBnClickedConfigureGroups()
{
	try {

		if(!CheckCurrentUserPermissions(bioEditOtherUserPermissions, sptWrite)) {		
			return;
		}

		CConfigurePermissionGroupsDlg dlg(TRUE, m_nUserID, this);

		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}
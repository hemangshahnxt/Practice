// ConfigureNexWebPasswordComplexityRequirementsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "ConfigureNexWebPasswordComplexityRequirementsDlg.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"

// CConfigureNexWebPasswordComplexityRequirementsDlg dialog
// (b.savon 2012-07-25 22:32) - PLID 50585 - Created

IMPLEMENT_DYNAMIC(CConfigureNexWebPasswordComplexityRequirementsDlg, CNxDialog)

CConfigureNexWebPasswordComplexityRequirementsDlg::CConfigureNexWebPasswordComplexityRequirementsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigureNexWebPasswordComplexityRequirementsDlg::IDD, pParent)
{
	m_nMinPasswordLength = 0;
	m_nMinLetters = 0;
	m_nMinNumbers = 0;
	m_nMinUppercaseLetters = 0;
	m_nMinLowercaseLetters = 0;
	m_nPasswordRequirementsEnforced = 0;
	m_nLockoutAttempts = 0;
	m_nLockoutDuration = 0;
	m_nLockoutReset = 0;
	m_nSecurityCodeExpire = 0;
}

CConfigureNexWebPasswordComplexityRequirementsDlg::~CConfigureNexWebPasswordComplexityRequirementsDlg()
{
}


void CConfigureNexWebPasswordComplexityRequirementsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_NXC_BACKGROUND, m_nxcBackground);
	DDX_Control(pDX, IDC_NXC_LOCKOUT, m_nxcLockoutBackground);
	DDX_Control(pDX, IDC_NXC_SECURITYCODE, m_nxcSecurityCode);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_MORE_INFO, m_btnQuestion);
	DDX_Control(pDX, IDC_MIN_PASSWORD_LENGTH, m_nxeMinPasswordLength);
	DDX_Control(pDX, IDC_MIN_LETTERS, m_nxeMinLetters);
	DDX_Control(pDX, IDC_MIN_NUMBERS, m_nxeMinNumbers);
	DDX_Control(pDX, IDC_MIN_UPPERCASE_LETTERS, m_nxeMinUppercaseLetters);
	DDX_Control(pDX, IDC_MIN_LOWERCASE_LETTERS, m_nxeMinLowercaseLetters);
	DDX_Control(pDX, IDC_LOCKOUT_ATTEMPTS, m_nxeLockoutAttempts);
	DDX_Control(pDX, IDC_LOCKOUT_MINUTES, m_nxeLockoutDuration);
	DDX_Control(pDX, IDC_LOCKOUT_CYCLES, m_nxeLockoutReset);
	DDX_Control(pDX, IDC_SECURITY_EXPIRE, m_nxeSecurityCodeExpire);
	DDX_Control(pDX, IDC_ENABLE_ATTEMPTS, m_chkLoginAttempts);
	DDX_Control(pDX, IDC_ENABLE_DURATION, m_chkLoginDuration);
	DDX_Control(pDX, IDC_ENABLE_ADMIN, m_chkAdminReset);
}

BOOL CConfigureNexWebPasswordComplexityRequirementsDlg::OnInitDialog()
{
	try{
		CNxDialog::OnInitDialog();

		//bulk load our settings
		g_propManager.CachePropertiesInBulk("NexWebPasswordConfigDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'NexWebPasswordMinLength' OR "
			"Name = 'NexWebPasswordMinLetters' OR "
			"Name = 'NexWebPasswordMinNumbers' OR "
			"Name = 'NexWebPasswordMinUpper' OR "
			"Name = 'NexWebPasswordMinLower' OR "
			"Name = 'NexWebPasswordRequirementsEnforced' OR "
			"Name = 'NexWebFailedLoginsAllowed' OR "
			"Name = 'NexWebFailedLoginCycles' OR "
			"Name = 'NexWebFailedLoginTimeout' OR "
			"Name = 'NexWebSecurityCodeExpire' "			
			")",
			_Q(GetCurrentUserName()));

		// Set our title bar icon
		SetTitleBarIcon(IDI_NEW_LOCK);

		// Set our NxColor and Button
		m_nxcBackground.SetColor(GetNxColor(GNC_ADMIN,1));
		m_nxcLockoutBackground.SetColor(GetNxColor(GNC_ADMIN,1));
		m_nxcSecurityCode.SetColor(GetNxColor(GNC_ADMIN,1));		

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnQuestion.AutoSet(NXB_QUESTION);

		// Set our group text bold
		SetBoldText(IDC_STATIC_PSS_COMP);
		SetBoldText(IDC_STATIC_LOCKOUT_CONFIG);
		SetBoldText(IDC_STATIC_SECURITY_EXPIRE);

		// Set the text limits
		// I can't see anyone requiring a password greater than 10 characters, but let's
		// let them do it anyway.
		m_nxeMinPasswordLength.SetLimitText(2);
		m_nxeMinLetters.SetLimitText(2);
		m_nxeMinNumbers.SetLimitText(2);
		m_nxeMinUppercaseLetters.SetLimitText(2);
		m_nxeMinLowercaseLetters.SetLimitText(2);
		// (b.savon 2012-08-16 16:33) - PLID 50584
		m_nxeLockoutAttempts.SetLimitText(2);
		m_nxeLockoutDuration.SetLimitText(3); // Maybe they want say 120 minutes
		m_nxeLockoutReset.SetLimitText(2);
		// (b.savon 2012-08-21 09:23) - PLID 50589
		m_nxeSecurityCodeExpire.SetLimitText(2); //I'm thinking they'll make them expire in a month or less

		// Load the settings
		m_nMinPasswordLength = GetRemotePropertyInt("NexWebPasswordMinLength", 0, 0, "<None>", true);
		m_nMinLetters = GetRemotePropertyInt("NexWebPasswordMinLetters", 0, 0, "<None>", true);
		m_nMinNumbers = GetRemotePropertyInt("NexWebPasswordMinNumbers", 0, 0, "<None>", true);
		m_nMinUppercaseLetters = GetRemotePropertyInt("NexWebPasswordMinUpper", 0, 0, "<None>", true);
		m_nMinLowercaseLetters = GetRemotePropertyInt("NexWebPasswordMinLower", 0, 0, "<None>", true);
		m_nPasswordRequirementsEnforced = GetRemotePropertyInt("NexWebPasswordRequirementsEnforced", 0, 0, "<None>", true);
		// (b.savon 2012-08-16 16:33) - PLID 50584
		m_nLockoutAttempts = GetRemotePropertyInt("NexWebFailedLoginsAllowed", 0, 0, "<None>", false);
		m_nLockoutDuration = GetRemotePropertyInt("NexWebFailedLoginTimeout", 0, 0, "<None>", false);
		m_nLockoutReset = GetRemotePropertyInt("NexWebFailedLoginCycles", 0, 0, "<None>", false);
		// (b.savon 2012-08-21 09:24) - PLID 50589
		m_nSecurityCodeExpire = GetRemotePropertyInt("NexWebSecurityCodeExpire", 0, 0, "<None>", true);		

		SetDlgItemInt(IDC_MIN_PASSWORD_LENGTH, m_nMinPasswordLength);
		SetDlgItemInt(IDC_MIN_LETTERS, m_nMinLetters);
		SetDlgItemInt(IDC_MIN_NUMBERS, m_nMinNumbers);
		SetDlgItemInt(IDC_MIN_UPPERCASE_LETTERS, m_nMinUppercaseLetters);
		SetDlgItemInt(IDC_MIN_LOWERCASE_LETTERS, m_nMinLowercaseLetters);

		// (b.savon 2012-08-16 16:33) - PLID 50584
		if( m_nLockoutAttempts != 0 ){
			SetDlgItemInt(IDC_LOCKOUT_ATTEMPTS, m_nLockoutAttempts);
			m_chkLoginAttempts.SetCheck(BST_CHECKED);
			m_nxeLockoutAttempts.EnableWindow();
		}
		if( m_nLockoutDuration != 0 ){
			SetDlgItemInt(IDC_LOCKOUT_MINUTES, m_nLockoutDuration);
			m_chkLoginDuration.SetCheck(BST_CHECKED);
			m_nxeLockoutDuration.EnableWindow();
		}
		if( m_nLockoutReset != 0 ){
			SetDlgItemInt(IDC_LOCKOUT_CYCLES, m_nLockoutReset);
			m_chkAdminReset.SetCheck(BST_CHECKED);
			if( m_chkLoginAttempts.GetCheck() == BST_CHECKED && m_chkLoginDuration.GetCheck() == BST_UNCHECKED ){
				m_nxeLockoutReset.EnableWindow(FALSE);
				m_chkAdminReset.EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_LOGON_CYCLE)->EnableWindow(FALSE);
			}else{
				m_nxeLockoutReset.EnableWindow();
			}
		}

		// (b.savon 2012-08-21 09:25) - PLID 50589
		SetDlgItemInt(IDC_SECURITY_EXPIRE, m_nSecurityCodeExpire);
		OnEnKillfocusSecurityExpire();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CConfigureNexWebPasswordComplexityRequirementsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnBnClickedOk)
	ON_EN_KILLFOCUS(IDC_MIN_LOWERCASE_LETTERS, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusMinLowercaseLetters)
	ON_EN_KILLFOCUS(IDC_MIN_UPPERCASE_LETTERS, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusMinUppercaseLetters)
	ON_EN_KILLFOCUS(IDC_MIN_NUMBERS, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusMinNumbers)
	ON_EN_KILLFOCUS(IDC_MIN_LETTERS, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusMinLetters)
	ON_EN_KILLFOCUS(IDC_MIN_PASSWORD_LENGTH, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusMinPasswordLength)
	ON_BN_CLICKED(IDC_BTN_MORE_INFO, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnBnClickedBtnMoreInfo)
	ON_EN_KILLFOCUS(IDC_LOCKOUT_ATTEMPTS, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusLockoutAttempts)
	ON_EN_KILLFOCUS(IDC_LOCKOUT_MINUTES, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusLockoutMinutes)
	ON_EN_KILLFOCUS(IDC_LOCKOUT_CYCLES, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusLockoutCycles)
	ON_EN_KILLFOCUS(IDC_SECURITY_EXPIRE, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusSecurityExpire)
	ON_BN_CLICKED(IDC_ENABLE_ATTEMPTS, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnBnClickedEnableAttempts)
	ON_BN_CLICKED(IDC_ENABLE_DURATION, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnBnClickedEnableDuration)
	ON_BN_CLICKED(IDC_ENABLE_ADMIN, &CConfigureNexWebPasswordComplexityRequirementsDlg::OnBnClickedEnableAdmin)
END_MESSAGE_MAP()


// CConfigureNexWebPasswordComplexityRequirementsDlg message handlers

void CConfigureNexWebPasswordComplexityRequirementsDlg::OnBnClickedOk()
{
	try{

		if(!ValidateAndSavePasswordComplexity()){
			return;
		}
		// (b.savon 2012-08-16 16:33) - PLID 50584
		if(!ValidateAndSaveLockoutSettings()){
			return;
		}
		// (b.savon 2012-08-21 09:49) - PLID 50589
		ValidateAndSaveSecurityCodeExpiration();
	
	}NxCatchAll(__FUNCTION__);
	
	CNxDialog::OnOK();
}

BOOL CConfigureNexWebPasswordComplexityRequirementsDlg::ValidateAndSavePasswordComplexity()
{
	long nAuditTransactionID  = -1;

	try{
		//Get current values
		long nMinimumLength = GetDlgItemInt(IDC_MIN_PASSWORD_LENGTH);
		long nMinimumLetters = GetDlgItemInt(IDC_MIN_LETTERS);
		long nMinimumNumbers = GetDlgItemInt(IDC_MIN_NUMBERS);
		long nMinimumUpper = GetDlgItemInt(IDC_MIN_UPPERCASE_LETTERS);
		long nMinimumLower = GetDlgItemInt(IDC_MIN_LOWERCASE_LETTERS);
		long nPasswordRequirementEnforced = (long)IsPasswordRequirementSet();

		if(nMinimumLength > 50 || (nMinimumLetters+nMinimumNumbers) > 50 || 
		  (nMinimumUpper+nMinimumLower) > 50 || (nMinimumUpper+nMinimumLower+nMinimumNumbers) > 50) {
			//passwords are limited to 50 characters
			AfxMessageBox("The values you entered for the password complexity are greater than 50. User passwords are limited to 50 characters.\n"
				"You cannot require a configuration greater than this limit.");
			return FALSE;
		}

		// Track whether anything changed so that we can audit and only save
		// those fields that changed.
		BOOL bDataChanged = FALSE;
		BOOL bStrengthChanged = FALSE;
		if(m_nMinPasswordLength != nMinimumLength) {
			bDataChanged = TRUE;
			bStrengthChanged = TRUE;
			SetRemotePropertyInt("NexWebPasswordMinLength", nMinimumLength, 0, "<None>");
		}
		if(m_nMinLetters != nMinimumLetters) {
			bDataChanged = TRUE;
			bStrengthChanged = TRUE;
			SetRemotePropertyInt("NexWebPasswordMinLetters", nMinimumLetters, 0, "<None>");
		}
		if(m_nMinNumbers != nMinimumNumbers) {
			bDataChanged = TRUE;
			bStrengthChanged = TRUE;
			SetRemotePropertyInt("NexWebPasswordMinNumbers", nMinimumNumbers, 0, "<None>");
		}
		if(m_nMinUppercaseLetters != nMinimumUpper) {
			bDataChanged = TRUE;
			bStrengthChanged = TRUE;
			SetRemotePropertyInt("NexWebPasswordMinUpper", nMinimumUpper, 0, "<None>");
		}
		if(m_nMinLowercaseLetters != nMinimumLower) {
			bDataChanged = TRUE;
			bStrengthChanged = TRUE;
			SetRemotePropertyInt("NexWebPasswordMinLower", nMinimumLower, 0, "<None>");
		}
		if(m_nPasswordRequirementsEnforced != nPasswordRequirementEnforced)
		{
			bDataChanged = TRUE;
			bStrengthChanged = TRUE;
			SetRemotePropertyInt("NexWebPasswordRequirementsEnforced", nPasswordRequirementEnforced, 0, "<None>");
		}

		if(bDataChanged) {
			//audit this change
			nAuditTransactionID	= BeginAuditTransaction();

			AuditEvent(-1, "", nAuditTransactionID, aeiNexWebPasswordComplexity, -1,
				CalcAuditDesc(m_nMinPasswordLength, m_nMinLetters, m_nMinNumbers, m_nMinUppercaseLetters, m_nMinLowercaseLetters),
				CalcAuditDesc(nMinimumLength, nMinimumLetters, nMinimumNumbers, nMinimumUpper, nMinimumLower),
				aepMedium, aetChanged);

			if(bStrengthChanged) {

				//Give them the option to change all the passwords (or not)
				if(MessageBox("The password strength rules will be enforced for all new NexWeb patients.\n\n"
					"Would you like to make all NexWeb patient passwords expire upon the next login, forcing all your NexWeb patients to change their password?"
					"\n\nAnswering 'No' to this question will preserve all current NexWeb passwords and require new NexWeb passwords to conform to this policy.",
					"Practice", MB_ICONQUESTION|MB_YESNO) == IDYES) {

					//update each user that will be changed, skip the nextech login
						ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle + ' ' "
						"+ PersonT.Title + ' - < ' + NexWebLoginInfoT.UserName + ' >' AS UserFullName "
						"FROM PersonT "
						"INNER JOIN NexWebLoginInfoT ON PersonT.ID = NexWebLoginInfoT.PersonID "
						"WHERE NexWebLoginInfoT.PersonID <> -26 AND NexWebLoginInfoT.PWExpireNextLogin = 0");
					BOOL bUsersNeedUpdating = FALSE;
					while(!rs->eof) {
						bUsersNeedUpdating = TRUE;
						long nUserID = AdoFldLong(rs, "PersonID");
						CString strUsernameToAudit = AdoFldString(rs, "UserFullName", "");

						AuditEvent(-1, strUsernameToAudit, nAuditTransactionID, aeiNexWebPWExpireNextLogin, nUserID, "Disabled", "Enabled", aepMedium, aetChanged);

						rs->MoveNext();
					}
					rs->Close();

					//now update these users, no need to get the individual IDs
					if(bUsersNeedUpdating ) {
						ExecuteSql("UPDATE NexWebLoginInfoT SET PWExpireNextLogin = 1 WHERE PWExpireNextLogin = 0 AND PersonID <> -26");
					}

					//let the user know that this process is complete
					AfxMessageBox("All NexWeb patients will now have to change their passwords the next time they login to NexWeb.\n\n"
						"All new passwords will be forced to follow the new password requirements.");
				}else{
					MessageBox("The passwords for existing NexWeb patients have not changed.  All new NexWeb patient passwords must conform to the configured policy.",
					"Practice", MB_ICONINFORMATION|MB_OK);
				}
			}

			//we'll always have audited the strength change,
			//we may have also audited user changes too
			//as well as the login banner
			CommitAuditTransaction(nAuditTransactionID);
			return TRUE;
		}else{
			return TRUE;
		}
	}NxCatchAllCall(__FUNCTION__,
		
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
		return FALSE;
	);
	
}

// (b.savon 2012-08-16 16:32) - PLID 50584 - Validate and save lockout settings
BOOL CConfigureNexWebPasswordComplexityRequirementsDlg::ValidateAndSaveLockoutSettings()
{
	try{

		long nLockoutAttempts = GetDlgItemInt(IDC_LOCKOUT_ATTEMPTS);
		long nLockoutDuration = GetDlgItemInt(IDC_LOCKOUT_MINUTES);
		long nLockoutReset = GetDlgItemInt(IDC_LOCKOUT_CYCLES);

		if( !ValidateLockoutSettings() ){
			return FALSE;
		}

		// Track whether anything changed so that we can audit and only save
		// those fields that changed.
		BOOL bDataChanged = FALSE;
		if(m_nLockoutAttempts != nLockoutAttempts) {
			bDataChanged = TRUE;
			SetRemotePropertyInt("NexWebFailedLoginsAllowed", nLockoutAttempts, 0, "<None>");
		}
		if(m_nLockoutDuration != nLockoutDuration) {
			bDataChanged = TRUE;
			SetRemotePropertyInt("NexWebFailedLoginTimeout", nLockoutDuration, 0, "<None>");	
		}
		if(m_nLockoutReset != nLockoutReset) {
			bDataChanged = TRUE;
			SetRemotePropertyInt("NexWebFailedLoginCycles", nLockoutReset, 0, "<None>");
			if( nLockoutReset == 0 ){
				ExecuteSql(
				"UPDATE NexWebLoginInfoT SET LockedOutCycle = -1 \r\n"
				"WHERE NexWebLoginInfoT.PersonID <> -26 AND NexWebLoginInfoT.LockedOut = 1 AND LockedOutTime IS NULL \r\n");
			}
		}

		if(bDataChanged) {
			//audit this change
			long nAuditTransactionID = BeginAuditTransaction();

			AuditEvent(-1, "", nAuditTransactionID, aeiNexWebLockoutRequirements, -1,
				CalcLockoutAuditDesc(m_nLockoutAttempts, m_nLockoutDuration, m_nLockoutReset),
				CalcLockoutAuditDesc(nLockoutAttempts, nLockoutDuration, nLockoutReset),
				aepMedium, aetChanged);		
			
			CommitAuditTransaction(nAuditTransactionID); 
		}

		return TRUE;

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

BOOL CConfigureNexWebPasswordComplexityRequirementsDlg::ValidateLockoutSettings()
{
	long nLockoutAttempts = GetDlgItemInt(IDC_LOCKOUT_ATTEMPTS);
	long nLockoutDuration = GetDlgItemInt(IDC_LOCKOUT_MINUTES);
	long nLockoutReset = GetDlgItemInt(IDC_LOCKOUT_CYCLES);

	if(nLockoutAttempts != 0 && nLockoutDuration == 0 && nLockoutReset == 0){
		//if the count is non zero the others both cannot be 0
		AfxMessageBox("The values you entered are invalid.  You must either define a lockout duration or administrative reset count.");
		return FALSE;
	}

	if(nLockoutAttempts == 0 && (nLockoutDuration != 0 || nLockoutReset != 0)){
		//if the count is zero the others both cannot be non 0
		AfxMessageBox("The values you entered are invalid.  You must define a lockout count.");
		return FALSE;
	}

	return TRUE;
}

// (b.savon 2012-08-21 09:27) - PLID 50589
void CConfigureNexWebPasswordComplexityRequirementsDlg::ValidateAndSaveSecurityCodeExpiration()
{
	try{

		long nSecurityCodeExpire = GetDlgItemInt(IDC_SECURITY_EXPIRE);

		// Track whether anything changed so that we can audit and only save
		// those fields that changed.
		BOOL bDataChanged = FALSE;
		if(m_nSecurityCodeExpire != nSecurityCodeExpire) {
			bDataChanged = TRUE;
			SetRemotePropertyInt("NexWebSecurityCodeExpire", nSecurityCodeExpire, 0, "<None>");
		}

		if(bDataChanged) {
			//audit this change
			long nAuditTransactionID = BeginAuditTransaction();

			AuditEvent(-1, "", nAuditTransactionID, aeiNexWebSecurityCodeExpiration, -1,
				CalcSecuirtyCodeExpirationAuditDesc(m_nSecurityCodeExpire),
				CalcSecuirtyCodeExpirationAuditDesc(nSecurityCodeExpire),
				aepMedium, aetChanged);		
			
			CommitAuditTransaction(nAuditTransactionID); 
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-08-21 09:30) - PLID 50589
CString CConfigureNexWebPasswordComplexityRequirementsDlg::CalcSecuirtyCodeExpirationAuditDesc(long nSecurityCodeExpiration)
{
	try {
		//catch our own exceptions, which (while unlikely) would still allow the audit to complete

		if(nSecurityCodeExpiration == 0) {
			return "There is no NexWeb security code expiration.";
		}

		CString strDesc;
		if(nSecurityCodeExpiration != 0) {

			strDesc = "NexWeb security code expiration requirements:";

			CString str;
			if(nSecurityCodeExpiration > 0) {
				str.Format(" %li day security code expiration", nSecurityCodeExpiration);
				strDesc += str;
			}
		}

		strDesc.TrimRight(",");
		if(!strDesc.IsEmpty()) {
			strDesc += ". ";
		}

		return strDesc;

	}NxCatchAll(__FUNCTION__);

	return "";
}

CString CConfigureNexWebPasswordComplexityRequirementsDlg::CalcAuditDesc(long nLength, long nLetters, long nNumbers, long nUpper, long nLower)
{
	try {
		//catch our own exceptions, which (while unlikely) would still allow the audit to complete

		if(nLength == 0 && nLetters == 0 && nNumbers == 0 && nUpper == 0 && nLower == 0) {
			return "Passwords have no requirements.";
		}

		CString strDesc;
		if(nLength != 0 || nLetters != 0 || nNumbers != 0 || nUpper != 0 || nLower != 0) {

			strDesc = "Passwords require:";

			CString str;
			if(nLength > 0) {
				//keep the plural here, even if 1
				str.Format(" %li total characters,", nLength);
				strDesc += str;
			}
			if(nLetters > 0) {
				if(nLetters == 1) {
					strDesc += " 1 letter,";
				}
				else {
					str.Format(" %li letters,", nLetters);
					strDesc += str;
				}
			}
			if(nNumbers > 0) {
				if(nNumbers == 1) {
					strDesc += " 1 number,";
				}
				else {
					str.Format(" %li numbers,", nNumbers);
					strDesc += str;
				}
			}
			if(nUpper > 0) {
				if(nUpper == 1) {
					strDesc += " 1 uppercase letter,";
				}
				else {
					str.Format(" %li uppercase letters,", nUpper);
					strDesc += str;
				}
			}
			if(nLower > 0) {
				if(nLower == 1) {
					strDesc += " 1 lowercase letter,";
				}
				else {
					str.Format(" %li lowercase letters,", nLower);
					strDesc += str;
				}
			}
		}

		strDesc.TrimRight(",");
		if(!strDesc.IsEmpty()) {
			strDesc += ". ";
		}

		return strDesc;

	}NxCatchAll("Error in CConfigureNexWebPasswordComplexityRequirementsDlg::CalcAuditDesc");

	return "";
}

// (b.savon 2012-08-16 16:20) - PLID 50584 - Audit
CString CConfigureNexWebPasswordComplexityRequirementsDlg::CalcLockoutAuditDesc(long nAttempts, long nDuration, long nReset)
{
	try {
		//catch our own exceptions, which (while unlikely) would still allow the audit to complete

		if(nAttempts == 0 && nDuration == 0 && nReset == 0) {
			return "There are no NexWeb account lock-out requirements.";
		}

		CString strDesc;
		if(nAttempts != 0 || nDuration != 0 || nReset != 0) {

			strDesc = "NexWeb account lock-out requirements:";

			CString str;
			if(nAttempts > 0) {
				if(nAttempts == 1) {
					strDesc += " 1 unsuccessful attempt,";
				}
				else {
					str.Format(" %li unsuccessful attempts,", nAttempts);
					strDesc += str;
				}
			}
			if(nDuration > 0) {
				str.Format(" %li minute lock-out duration,", nDuration);
				strDesc += str;
			}
			if(nReset > 0) {
				if(nReset == 1) {
					strDesc += " administrative reset after 1 lock-out,";
				}
				else {
					str.Format(" administrative reset after %li lock-outs,", nReset);
					strDesc += str;
				}
			}
		}

		strDesc.TrimRight(",");
		if(!strDesc.IsEmpty()) {
			strDesc += ". ";
		}

		return strDesc;

	}NxCatchAll(__FUNCTION__);

	return "";
}

BOOL CConfigureNexWebPasswordComplexityRequirementsDlg::IsPasswordRequirementSet()
{
	//Setup
	CString strMinPasswordLength;
	CString strMinLowercaseLetters;
	CString strMinUppercaseLetters;
	CString strMinNumbers;
	CString strMinLetters;

	//Get Edit Text
	m_nxeMinPasswordLength.GetWindowText(strMinPasswordLength);
	m_nxeMinLowercaseLetters.GetWindowText(strMinLowercaseLetters);
	m_nxeMinUppercaseLetters.GetWindowText(strMinUppercaseLetters);
	m_nxeMinNumbers.GetWindowText(strMinNumbers);
	m_nxeMinLetters.GetWindowText(strMinLetters);
	m_nxeMinPasswordLength.GetWindowText(strMinPasswordLength);

	//Check
	if( atoi(strMinPasswordLength) != 0 ||
		atoi(strMinLowercaseLetters) != 0 ||
		atoi(strMinUppercaseLetters) != 0 ||
		atoi(strMinNumbers) != 0 ||
		atoi(strMinLetters) != 0  )
	{
		return TRUE;
	}
	
	// We have no password requirements
	return FALSE;
}
void CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusMinLowercaseLetters()
{
	try{

		//Setup
		CString strMinLowercaseLetters;

		//Get Edit Text
		m_nxeMinLowercaseLetters.GetWindowText(strMinLowercaseLetters);

		if( strMinLowercaseLetters.IsEmpty() )
		{
			m_nxeMinLowercaseLetters.SetWindowText("0");
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusMinUppercaseLetters()
{
	try{

		//Setup
		CString strMinUppercaseLetters;

		//Get Edit Text
		m_nxeMinUppercaseLetters.GetWindowText(strMinUppercaseLetters);

		if( strMinUppercaseLetters.IsEmpty() )
		{
			m_nxeMinUppercaseLetters.SetWindowText("0");
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusMinNumbers()
{
	try{

		//Setup
		CString strMinNumbers;

		//Get Edit Text
		m_nxeMinNumbers.GetWindowText(strMinNumbers);

		if( strMinNumbers.IsEmpty() )
		{
			m_nxeMinNumbers.SetWindowText("0");
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusMinLetters()
{
	try{

		//Setup
		CString strMinLetters;

		//Get Edit Text
		m_nxeMinLetters.GetWindowText(strMinLetters);

		if( strMinLetters.IsEmpty() )
		{
			m_nxeMinLetters.SetWindowText("0");
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusMinPasswordLength()
{
	try{

		//Setup
		CString strMinPasswordLength;

		//Get Edit Text
		m_nxeMinPasswordLength.GetWindowText(strMinPasswordLength);

		if( strMinPasswordLength.IsEmpty() )
		{
			m_nxeMinPasswordLength.SetWindowText("0");
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigureNexWebPasswordComplexityRequirementsDlg::SetBoldText(long nControlID)
{
	try{
		// Get current font.
		CFont* pFont = GetDlgItem( nControlID )->GetFont();
		LOGFONT LogFont = { 0 };
		pFont->GetLogFont( &LogFont );

		// Create new font with *bold* style.
		LogFont.lfWeight = 300;
		CFont pNewFont;
		pNewFont.CreateFontIndirect( &LogFont );

		// Sets the new font back to static text.
		GetDlgItem( nControlID )->SetFont( &pNewFont );

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-08-16 16:35) - PLID 50584 - Compliance description message
void CConfigureNexWebPasswordComplexityRequirementsDlg::OnBnClickedBtnMoreInfo()
{
	try{
		CString strDuration;
		m_nxeLockoutDuration.GetWindowText(strDuration);
		CString strAttempts;
		m_nxeLockoutAttempts.GetWindowText(strAttempts);
		CString strReset;
		m_nxeLockoutReset.GetWindowText(strReset);
		
		bool bNotConfigured = (strDuration.CompareNoCase("0")==0&&strAttempts.CompareNoCase("0")==0&&strReset.CompareNoCase("0")==0) ||
							  (strDuration.IsEmpty()&&strAttempts.IsEmpty()&&strReset.IsEmpty());

		if( !ValidateLockoutSettings() ){
			return;
		}

		// (b.savon 2012-08-27 15:40) - PLID 50584 - Clean up the message box for singularity
		AfxMessageBox( bNotConfigured ? "There are no compliance settings configured." : 
					  (m_chkAdminReset.GetCheck()==BST_CHECKED&&m_nxeLockoutReset.IsWindowEnabled()==FALSE ? 
					  "Account lock-out compliance description:\r\n\r\nAfter " + strAttempts + " failed login " + (atoi(strAttempts)==1?"attempt ":"attempts ") + "the user account must require an office administrator to restore access." :
					  ("Account lock-out compliance description:\r\n\r\nDisable access for " + strDuration + (strDuration.CompareNoCase("1")==0 ? " minute" : " minutes") +   
					  (strAttempts.CompareNoCase("0")!= 0 ? " after " + strAttempts + (strAttempts.CompareNoCase("1")==0 ? " failed logon attempt." : " failed logon attempts.") : ".") + 
					  (!strReset.IsEmpty() ? (strReset.CompareNoCase("0")!= 0 ? ("  After " + strReset + (strReset.CompareNoCase("1")==0 ? " lock-out, " : " consecutive lock-outs, ")) +
					  "the user account must require an office administrator to reset the account and restore access." : "") : ""))),
					  MB_OK | MB_ICONINFORMATION );
	}NxCatchAll(__FUNCTION__);
}

void CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusLockoutAttempts()
{
	try{

		//Setup
		CString strLockoutAttempts;

		//Get Edit Text
		m_nxeLockoutAttempts.GetWindowText(strLockoutAttempts);

		if( atoi(strLockoutAttempts) == 0 || strLockoutAttempts.IsEmpty() )
		{
			m_nxeLockoutAttempts.SetWindowText("1");
		}

		if( m_chkAdminReset.GetCheck() == BST_UNCHECKED && m_chkLoginDuration.GetCheck() == BST_UNCHECKED ){
			m_chkAdminReset.SetCheck(BST_CHECKED);
			m_chkAdminReset.EnableWindow(FALSE);
			GetDlgItem(IDC_STATIC_LOGON_CYCLE)->EnableWindow(FALSE);
			m_nxeLockoutReset.EnableWindow(FALSE);
			OnEnKillfocusLockoutCycles();
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusLockoutMinutes()
{
	try{

		//Setup
		CString strLockoutDuration;

		//Get Edit Text
		m_nxeLockoutDuration.GetWindowText(strLockoutDuration);

		// (b.savon 2012-08-27 15:50) - PLID 50584 - Handle singularity
		if( atoi(strLockoutDuration) == 0 || strLockoutDuration.IsEmpty() )
		{
			m_nxeLockoutDuration.SetWindowText("1");
			GetDlgItem(IDC_STATIC_LOCKOUT_MINUTES)->SetWindowText("minute");
		}else if( atoi(strLockoutDuration) == 1 ){
			GetDlgItem(IDC_STATIC_LOCKOUT_MINUTES)->SetWindowText("minute");
		}else{
			GetDlgItem(IDC_STATIC_LOCKOUT_MINUTES)->SetWindowText("minutes");
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusLockoutCycles()
{
	try{

		//Setup
		CString strReset;

		//Get Edit Text
		m_nxeLockoutReset.GetWindowText(strReset);

		// (b.savon 2012-08-27 15:50) - PLID 50584 - Handle singularity
		if ( atoi(strReset) == 0 || strReset.IsEmpty() ){
			m_nxeLockoutReset.SetWindowText("1");
			GetDlgItem(IDC_STATIC_LOGON_CYCLE)->SetWindowText("lock-out");
		}else if ( atoi(strReset) == 1 ){
			GetDlgItem(IDC_STATIC_LOGON_CYCLE)->SetWindowText("lock-out");
		}else{
			GetDlgItem(IDC_STATIC_LOGON_CYCLE)->SetWindowText("consecutive lock-outs");
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2012-08-21 09:22) - PLID 50589
void CConfigureNexWebPasswordComplexityRequirementsDlg::OnEnKillfocusSecurityExpire()
{
	try{

		//Setup
		CString strExpire;

		//Get Edit Text
		m_nxeSecurityCodeExpire.GetWindowText(strExpire);

		// (b.savon 2012-08-27 16:22) - PLID 50589 - Handle singularity
		if( atoi(strExpire) == 1 ){
			GetDlgItem(IDC_STATIC_SECURITY_CODE)->SetWindowText("day");
		}else{
			GetDlgItem(IDC_STATIC_SECURITY_CODE)->SetWindowText("days");
		}

	}NxCatchAll(__FUNCTION__);
}

void CConfigureNexWebPasswordComplexityRequirementsDlg::OnBnClickedEnableAttempts()
{
	try{
		m_nxeLockoutAttempts.EnableWindow(m_chkLoginAttempts.GetCheck() == BST_CHECKED ? TRUE : FALSE);
		if( m_chkLoginAttempts.GetCheck() == BST_CHECKED ){
			m_nxeLockoutAttempts.SetFocus();
		}else{
			m_chkLoginDuration.SetCheck(BST_UNCHECKED);
			m_nxeLockoutDuration.EnableWindow(FALSE);
			m_nxeLockoutDuration.SetWindowText("");
			m_chkAdminReset.SetCheck(BST_UNCHECKED);
			m_chkAdminReset.EnableWindow(TRUE);
			GetDlgItem(IDC_STATIC_LOGON_CYCLE)->EnableWindow(TRUE);
			m_nxeLockoutReset.EnableWindow(FALSE);
			m_nxeLockoutReset.SetWindowText("");
		}
		m_nxeLockoutAttempts.SetWindowText("");
	}NxCatchAll(__FUNCTION__);
}

void CConfigureNexWebPasswordComplexityRequirementsDlg::OnBnClickedEnableDuration()
{
	try{
		m_nxeLockoutDuration.EnableWindow(m_chkLoginDuration.GetCheck() == BST_CHECKED ? TRUE : FALSE);
		if( m_chkLoginDuration.GetCheck() == BST_CHECKED ){
			m_nxeLockoutDuration.SetFocus();
			if( m_chkAdminReset.GetCheck() == BST_CHECKED && m_nxeLockoutReset.IsWindowEnabled() == FALSE ){
				m_nxeLockoutReset.EnableWindow(TRUE);
				m_chkAdminReset.EnableWindow(TRUE);
				GetDlgItem(IDC_STATIC_LOGON_CYCLE)->EnableWindow(TRUE);
			}
			if( m_chkLoginAttempts.GetCheck() == BST_UNCHECKED ){
				m_nxeLockoutAttempts.EnableWindow(TRUE);
				m_nxeLockoutAttempts.SetWindowText("1");
				m_chkLoginAttempts.SetCheck(BST_CHECKED);
			}
		}else{
			if( m_chkAdminReset.GetCheck() == BST_CHECKED && m_nxeLockoutReset.IsWindowEnabled() == TRUE ){
				m_nxeLockoutReset.EnableWindow(FALSE);
				m_chkAdminReset.EnableWindow(FALSE);
				m_nxeLockoutReset.SetWindowText("1");
				OnEnKillfocusLockoutCycles();
				GetDlgItem(IDC_STATIC_LOGON_CYCLE)->EnableWindow(FALSE);
			}
			if( m_chkLoginAttempts.GetCheck() == BST_CHECKED && m_chkAdminReset.GetCheck() == BST_UNCHECKED ){
				if( m_chkAdminReset.GetCheck() == BST_UNCHECKED ){
					m_chkAdminReset.SetCheck(BST_CHECKED);
					m_chkAdminReset.EnableWindow(FALSE);
					GetDlgItem(IDC_STATIC_LOGON_CYCLE)->EnableWindow(FALSE);
					m_nxeLockoutReset.EnableWindow(FALSE);
					OnEnKillfocusLockoutCycles();
				}
			}
		}
		m_nxeLockoutDuration.SetWindowText("");
	}NxCatchAll(__FUNCTION__);
}

void CConfigureNexWebPasswordComplexityRequirementsDlg::OnBnClickedEnableAdmin()
{
	try{
		m_nxeLockoutReset.EnableWindow(m_chkAdminReset.GetCheck() == BST_CHECKED ? TRUE : FALSE);
		if( m_chkAdminReset.GetCheck() == BST_CHECKED ){
			m_nxeLockoutReset.SetFocus();
			if( m_chkLoginAttempts.GetCheck() == BST_UNCHECKED ){
				m_chkLoginAttempts.SetCheck(BST_CHECKED);
				m_nxeLockoutAttempts.EnableWindow(TRUE);
				m_nxeLockoutAttempts.SetFocus();
				m_nxeLockoutReset.EnableWindow(FALSE);
				m_chkAdminReset.EnableWindow(FALSE);
				GetDlgItem(IDC_STATIC_LOGON_CYCLE)->EnableWindow(FALSE);
				m_nxeLockoutReset.SetWindowText("1");
			}
		}else if( m_chkLoginAttempts.GetCheck() == BST_CHECKED && m_chkLoginDuration.GetCheck() == BST_UNCHECKED ){
			m_chkLoginAttempts.SetCheck(BST_UNCHECKED);
			m_nxeLockoutAttempts.SetWindowText("");
			m_nxeLockoutAttempts.EnableWindow(FALSE);
		}

		if( m_chkAdminReset.GetCheck() == BST_UNCHECKED ){ 
			m_nxeLockoutReset.SetWindowText("");
		}
	}NxCatchAll(__FUNCTION__);
}

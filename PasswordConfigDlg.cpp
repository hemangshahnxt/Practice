// PasswordConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PasswordConfigDlg.h"
#include "AuditTrail.h"

// (j.jones 2008-11-18 11:55) - PLID 28572 - created

// CPasswordConfigDlg dialog

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



CPasswordConfigDlg::CPasswordConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPasswordConfigDlg::IDD, pParent)
{
	m_nOrig_MinLength = 0;
	m_nOrig_MinLetters = 0;
	m_nOrig_MinNumbers = 0;
	m_nOrig_MinUpper = 0;
	m_nOrig_MinLower = 0;
	m_bMadeAllUserPasswordsExpire = FALSE;

	// (j.jones 2009-05-19 16:02) - PLID 33801 - supported preventing reuse of last X passwords
	m_bOrig_PreventReuseOfLastPasswords = FALSE;
	m_nOrig_PreventLastPasswords = 5;
}

void CPasswordConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PASSWORD_MIN_CHARACTERS, m_editMinimumLength);
	DDX_Control(pDX, IDC_EDIT_PASSWORD_MIN_LETTERS, m_editMinimumLetters);
	DDX_Control(pDX, IDC_EDIT_PASSWORD_MIN_NUMBERS, m_editMinimumNumbers);
	DDX_Control(pDX, IDC_EDIT_PASSWORD_MIN_UPPERCASE, m_editMinimumUpper);
	DDX_Control(pDX, IDC_EDIT_PASSWORD_MIN_LOWERCASE, m_editMinimumLower);
	DDX_Control(pDX, IDC_SAMPLE_PASSWORD_LABEL, m_nxstaticSamplePassword);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_PREVENT_REUSE_PASSWORDS, m_checkPreventReuseOfLastPasswords);
	DDX_Control(pDX, IDC_EDIT_LAST_PASSWORD_COUNT, m_editPreventLastPasswords);
	// (a.walling 2009-12-16 14:58) - PLID 35784 - Supported a login banner
	DDX_Control(pDX, IDC_EDIT_LOGIN_BANNER, m_editLoginBanner);
}


BEGIN_MESSAGE_MAP(CPasswordConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_EN_KILLFOCUS(IDC_EDIT_PASSWORD_MIN_CHARACTERS, OnEnKillfocusEditPasswordMinCharacters)
	ON_EN_KILLFOCUS(IDC_EDIT_PASSWORD_MIN_LETTERS, OnEnKillfocusEditPasswordMinLetters)
	ON_EN_KILLFOCUS(IDC_EDIT_PASSWORD_MIN_NUMBERS, OnEnKillfocusEditPasswordMinNumbers)
	ON_EN_KILLFOCUS(IDC_EDIT_PASSWORD_MIN_UPPERCASE, OnEnKillfocusEditPasswordMinUppercase)
	ON_EN_KILLFOCUS(IDC_EDIT_PASSWORD_MIN_LOWERCASE, OnEnKillfocusEditPasswordMinLowercase)
	ON_BN_CLICKED(IDC_CHECK_PREVENT_REUSE_PASSWORDS, &CPasswordConfigDlg::OnCheckPreventReusePasswords)
END_MESSAGE_MAP()

// CPasswordConfigDlg message handlers

BOOL CPasswordConfigDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		//bulk load our settings
		g_propManager.CachePropertiesInBulk("PasswordConfigDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'UserPasswordMinLength' OR "
			"Name = 'UserPasswordMinLetters' OR "
			"Name = 'UserPasswordMinNumbers' OR "
			"Name = 'UserPasswordMinUpper' OR "
			"Name = 'UserPasswordMinLower' OR "
			// (j.jones 2009-05-19 14:32) - PLID 33801 - supported preventing reuse of last X passwords
			"Name = 'UserPasswordPreventReuseOfPasswords' OR "
			"Name = 'UserPasswordPreventReuseOfPasswordCount' "
			")",
			_Q(GetCurrentUserName()));
		
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_editMinimumLength.SetLimitText(2);
		m_editMinimumLetters.SetLimitText(2);
		m_editMinimumNumbers.SetLimitText(2);
		m_editMinimumUpper.SetLimitText(2);
		m_editMinimumLower.SetLimitText(2);
		// (j.jones 2009-05-19 14:32) - PLID 33801 - limit to no more than 2 characters
		m_editPreventLastPasswords.SetLimitText(2);

		// (a.walling 2009-12-16 15:56) - PLID 35784 - Limit the text
		m_editLoginBanner.SetLimitText(64);

		//load the settings
		m_nOrig_MinLength = GetRemotePropertyInt("UserPasswordMinLength", 0, 0, "<None>", true);
		m_nOrig_MinLetters = GetRemotePropertyInt("UserPasswordMinLetters", 0, 0, "<None>", true);
		m_nOrig_MinNumbers = GetRemotePropertyInt("UserPasswordMinNumbers", 0, 0, "<None>", true);
		m_nOrig_MinUpper = GetRemotePropertyInt("UserPasswordMinUpper", 0, 0, "<None>", true);
		m_nOrig_MinLower = GetRemotePropertyInt("UserPasswordMinLower", 0, 0, "<None>", true);

		SetDlgItemInt(IDC_EDIT_PASSWORD_MIN_CHARACTERS, m_nOrig_MinLength);
		SetDlgItemInt(IDC_EDIT_PASSWORD_MIN_LETTERS, m_nOrig_MinLetters);
		SetDlgItemInt(IDC_EDIT_PASSWORD_MIN_NUMBERS, m_nOrig_MinNumbers);
		SetDlgItemInt(IDC_EDIT_PASSWORD_MIN_UPPERCASE, m_nOrig_MinUpper);
		SetDlgItemInt(IDC_EDIT_PASSWORD_MIN_LOWERCASE, m_nOrig_MinLower);

		CalcSamplePassword();

		// (j.jones 2009-05-19 14:32) - PLID 33801 - supported preventing reuse of last X passwords
		m_bOrig_PreventReuseOfLastPasswords = GetRemotePropertyInt("UserPasswordPreventReuseOfPasswords", 0, 0, "<None>", true) == 1;
		m_checkPreventReuseOfLastPasswords.SetCheck(m_bOrig_PreventReuseOfLastPasswords);
		m_nOrig_PreventLastPasswords = GetRemotePropertyInt("UserPasswordPreventReuseOfPasswordCount", 5, 0, "<None>", true);
		SetDlgItemInt(IDC_EDIT_LAST_PASSWORD_COUNT, m_nOrig_PreventLastPasswords);
		m_editPreventLastPasswords.EnableWindow(m_checkPreventReuseOfLastPasswords.GetCheck());

		// (a.walling 2009-12-16 14:59) - PLID 35784 - Load the login banner
		// (a.walling 2010-01-22 16:20) - PLID 35784 - No default anymore
		m_editLoginBanner.SetWindowText(GetRemotePropertyText("LoginBanner", "", 0, "<None>", true));

	}NxCatchAll("CPasswordConfigDlg::OnInitDialog");

	return TRUE;
}

//CalcSamplePassword will display a sample valid password on the screen
void CPasswordConfigDlg::CalcSamplePassword()
{
	try {

		//generate a sample valid password from our settings

		long nMinimumLength = GetDlgItemInt(IDC_EDIT_PASSWORD_MIN_CHARACTERS);
		long nMinimumLetters = GetDlgItemInt(IDC_EDIT_PASSWORD_MIN_LETTERS);
		long nMinimumNumbers = GetDlgItemInt(IDC_EDIT_PASSWORD_MIN_NUMBERS);
		long nMinimumUpper = GetDlgItemInt(IDC_EDIT_PASSWORD_MIN_UPPERCASE);
		long nMinimumLower = GetDlgItemInt(IDC_EDIT_PASSWORD_MIN_LOWERCASE);

		CString strSamplePassword = "<All Passwords Valid>";

		if(nMinimumLength == 0 && nMinimumLetters == 0 && nMinimumNumbers == 0 && nMinimumUpper == 0 && nMinimumLower == 0) {
			strSamplePassword = "<All Passwords Valid>";
		}
		else {
			CString strLetters = "";
			CString strNumbers = "";
			CString strUpper = "";
			CString strLower = "";
			int i = 0;
			char chNextChar = char('a');
			for(i=0; i<nMinimumUpper; i++) {

				CString strCharToAdd = chNextChar;
				strCharToAdd.MakeUpper();
				strUpper += strCharToAdd;

				chNextChar++;
				if(chNextChar > char('z')) {
					chNextChar = char('a');
				}
			}
			for(i=0; i<nMinimumLower; i++) {

				CString strCharToAdd = chNextChar;
				strLower += strCharToAdd;

				chNextChar++;
				if(chNextChar > char('z')) {
					chNextChar = char('a');
				}
			}

			strSamplePassword = strUpper + strLower;

			for(i=nMinimumUpper + nMinimumLower; i<nMinimumLetters; i++) {

				CString strCharToAdd = chNextChar;
				strSamplePassword += strCharToAdd;

				chNextChar++;
				if(chNextChar > char('z')) {
					chNextChar = char('a');
				}
			}
			for(i=0; i<nMinimumNumbers; i++) {
				long nNum = i;
				nNum++;
				while(nNum >= 10) {
					nNum -= 10;
				}
				CString strCur;
				strCur.Format("%li", i);
				strNumbers += strCur;
			}

			if(strSamplePassword.GetLength() + strNumbers.GetLength() >= nMinimumLength) {
				strSamplePassword += strNumbers;
			}
			else {

				long nCurLength = strSamplePassword.GetLength() + strNumbers.GetLength();

				for(i=nCurLength; i<nMinimumLength; i++) {

					CString strCharToAdd = chNextChar;
					strSamplePassword += strCharToAdd;

					chNextChar++;
					if(chNextChar > char('z')) {
						chNextChar = char('a');
					}
				}

				strSamplePassword += strNumbers;
			}

			if(strSamplePassword.IsEmpty()) {
				strSamplePassword = "<All Passwords Valid>";
			}
		}

		//we can only display so much, so cut it off at 30 characters and add an ellipsis
		if(strSamplePassword.GetLength() > 30) {
			strSamplePassword = strSamplePassword.Left(30);
			strSamplePassword += "...";
		}

		m_nxstaticSamplePassword.SetWindowText(strSamplePassword);

	}NxCatchAll("CPasswordConfigDlg::CalcSamplePassword");
}

//CalcAuditDesc will generate an audit description including each non-zero field
// (j.jones 2009-05-19 16:02) - PLID 33801 - supported preventing reuse of last X passwords
CString CPasswordConfigDlg::CalcAuditDesc(long nLength, long nLetters, long nNumbers, long nUpper, long nLower,
										  BOOL bPreventReuseOfLastPasswords, long nPreventLastPasswords)
{
	try {
		//catch our own exceptions, which (while unlikely) would still allow the audit to complete

		if(nLength == 0 && nLetters == 0 && nNumbers == 0 && nUpper == 0 && nLower == 0 && !bPreventReuseOfLastPasswords) {
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

		// (j.jones 2009-05-19 16:02) - PLID 33801 - supported preventing reuse of last X passwords
		if(bPreventReuseOfLastPasswords) {			
			if(nPreventLastPasswords == 1) {				
				strDesc += "Passwords cannot be the last password used.";
			}
			else {
				CString str;
				str.Format("Passwords cannot be one of the last %li passwords used.", nPreventLastPasswords);
				strDesc += str;
			}
		}

		return strDesc;

	}NxCatchAll("Error in CPasswordConfigDlg::CalcAuditDesc");

	return "";
}

void CPasswordConfigDlg::OnOk()
{
	long nAuditTransactionID = -1;

	try {

		long nMinimumLength = GetDlgItemInt(IDC_EDIT_PASSWORD_MIN_CHARACTERS);
		long nMinimumLetters = GetDlgItemInt(IDC_EDIT_PASSWORD_MIN_LETTERS);
		long nMinimumNumbers = GetDlgItemInt(IDC_EDIT_PASSWORD_MIN_NUMBERS);
		long nMinimumUpper = GetDlgItemInt(IDC_EDIT_PASSWORD_MIN_UPPERCASE);
		long nMinimumLower = GetDlgItemInt(IDC_EDIT_PASSWORD_MIN_LOWERCASE);

		BOOL bPreventReuseOfLastPasswords = m_checkPreventReuseOfLastPasswords.GetCheck();
		long nPreventLastPasswords = GetDlgItemInt(IDC_EDIT_LAST_PASSWORD_COUNT);
		
		if(bPreventReuseOfLastPasswords) {
			// (j.jones 2009-05-19 16:09) - PLID 33801 - limit to greater than zero and not greater than 24
			if(nPreventLastPasswords <= 0) {
				AfxMessageBox("You must enter a value greater than 0 for preventing previous passwords to be reused.");
				return;
			}			
			else if(nPreventLastPasswords > 24) {		
				AfxMessageBox("The value for preventing passwords to be reused cannot exceed 24.");
				return;
			}			
		}

		if(nMinimumLength > 50) {
			//passwords are limited to 50 characters, just check the min length, our other checks
			//will eventually disallow saving other combinations that would be greater than the min length
			// (b.savon 2012-08-14 18:12) - PLID 52139 - Added 'is' greater than 50
			AfxMessageBox("The value you entered for Minimum Letters is greater than 50. User passwords are limited to 50 characters.\n"
				"You cannot require a length greater than this limit.");
			return;
		}

		//enforce that the "letter" rule can't be less than the upper + lower count,
		//and enforce that the total length can't be less than the total letters + total numbers
		
		if(nMinimumLetters < nMinimumUpper + nMinimumLower) {
			AfxMessageBox("The value you entered for Minimum Letters is less than the combined value of Minimum Uppercase Letters "
				"and Minimum Lowercase Letters. Please correct this before continuing.");
			return;
		}

		if(nMinimumLength < nMinimumLetters + nMinimumNumbers) {
			AfxMessageBox("The value you entered for Minimum Password Length is less than the combined value of Minimum Letters "
				"and Minimum Numbers. Please correct this before continuing.");
			return;
		}

		// (j.jones 2009-05-19 16:04) - PLID 33801 - track two booleans here,
		// one is whether anything changed and we need to audit, the other is
		// explicitly whether the strength level changed, which prompts to make
		// all passwords expire
		BOOL bLoginBannerChanged = FALSE;
		BOOL bDataChanged = FALSE;
		BOOL bStrengthChanged = FALSE;
		if(m_nOrig_MinLength != nMinimumLength) {
			bDataChanged = TRUE;
			bStrengthChanged = TRUE;
			SetRemotePropertyInt("UserPasswordMinLength", nMinimumLength, 0, "<None>");
		}
		if(m_nOrig_MinLetters != nMinimumLetters) {
			bDataChanged = TRUE;
			bStrengthChanged = TRUE;
			SetRemotePropertyInt("UserPasswordMinLetters", nMinimumLetters, 0, "<None>");
		}
		if(m_nOrig_MinNumbers != nMinimumNumbers) {
			bDataChanged = TRUE;
			bStrengthChanged = TRUE;
			SetRemotePropertyInt("UserPasswordMinNumbers", nMinimumNumbers, 0, "<None>");
		}
		if(m_nOrig_MinUpper != nMinimumUpper) {
			bDataChanged = TRUE;
			bStrengthChanged = TRUE;
			SetRemotePropertyInt("UserPasswordMinUpper", nMinimumUpper, 0, "<None>");
		}
		if(m_nOrig_MinLower != nMinimumLower) {
			bDataChanged = TRUE;
			bStrengthChanged = TRUE;
			SetRemotePropertyInt("UserPasswordMinLower", nMinimumLower, 0, "<None>");
		}
		// (j.jones 2009-05-19 16:02) - PLID 33801 - supported preventing reuse of last X passwords
		if(m_bOrig_PreventReuseOfLastPasswords != bPreventReuseOfLastPasswords) {
			bDataChanged = TRUE;
			//we do not consider the strength level to have changed here
			SetRemotePropertyInt("UserPasswordPreventReuseOfPasswords", bPreventReuseOfLastPasswords ? 1 : 0, 0, "<None>");
		}
		if(m_nOrig_PreventLastPasswords != nPreventLastPasswords) {
			bDataChanged = TRUE;
			//we do not consider the strength level to have changed here
			SetRemotePropertyInt("UserPasswordPreventReuseOfPasswordCount", nPreventLastPasswords, 0, "<None>");
		}
		

		// (a.walling 2009-12-16 14:59) - PLID 35784 - Save the login banner
		CString strLoginBanner;
		m_editLoginBanner.GetWindowText(strLoginBanner);
		CString strOldLoginBanner = GetRemotePropertyText("LoginBanner", "", 0, "<None>", true);

		if (strLoginBanner != strOldLoginBanner) {
			bLoginBannerChanged = TRUE;
			SetRemotePropertyText("LoginBanner", strLoginBanner, 0, "<None>");
		}

		if(bDataChanged || bLoginBannerChanged) {
			//audit this change
			nAuditTransactionID = BeginAuditTransaction();

			
			// (a.walling 2009-12-16 14:59) - PLID 35784 - Audit the login banner
			if (bLoginBannerChanged) {
				AuditEvent(-1, "", nAuditTransactionID, aeiLoginBanner, -1, strOldLoginBanner, strLoginBanner, aepLow, aetChanged);
			}

			if (bDataChanged) {
				AuditEvent(-1, "", nAuditTransactionID, aeiUserPasswordStrengthRules, -1,
					CalcAuditDesc(m_nOrig_MinLength, m_nOrig_MinLetters, m_nOrig_MinNumbers, m_nOrig_MinUpper, m_nOrig_MinLower,
						m_bOrig_PreventReuseOfLastPasswords, m_nOrig_PreventLastPasswords),
					CalcAuditDesc(nMinimumLength, nMinimumLetters, nMinimumNumbers, nMinimumUpper, nMinimumLower,
						bPreventReuseOfLastPasswords, nPreventLastPasswords),
					aepMedium, aetChanged);

				if(bStrengthChanged) {

					//now ask the user if they want to force all user passwords to expire, with confirmation?
					if(MessageBox("The password strength rules will take effect the next time any user changes their password.\n"
						"Would you like to make all user passwords expire upon the next login, forcing all your users to change their password?",
						"Practice", MB_ICONQUESTION|MB_YESNO) == IDYES
						&& MessageBox("Are you sure? This will cause every user's password to expire.",
						"Practice", MB_ICONQUESTION|MB_YESNO) == IDYES) {

						//update each user that will be changed, skip the nextech login
						_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle + ' ' "
							"+ PersonT.Title + ' - < ' + UsersT.UserName + ' >' AS UserFullName "
							"FROM PersonT "
							"INNER JOIN UsersT ON PersonT.ID = UsersT.PersonID "
							"WHERE UsersT.PersonID <> -26 AND PWExpireNextLogin = 0");
						BOOL bUsersNeedUpdating = FALSE;
						while(!rs->eof) {
							bUsersNeedUpdating = TRUE;
							long nUserID = AdoFldLong(rs, "PersonID");
							CString strUsernameToAudit = AdoFldString(rs, "UserFullName", "");

							AuditEvent(-1, strUsernameToAudit, nAuditTransactionID, aeiUserPWExpireNextLogin, nUserID, "Disabled", "Enabled", aepMedium, aetChanged);

							rs->MoveNext();
						}
						rs->Close();

						//now update these users, no need to get the individual IDs
						if(bUsersNeedUpdating) {
							ExecuteSql("UPDATE UsersT SET PWExpireNextLogin = 1 WHERE PWExpireNextLogin = 0 AND PersonID <> -26");
						}

						//this will tell the caller that we updated UsersT.PWExpireNextLogin for all users
						m_bMadeAllUserPasswordsExpire = TRUE;

						//let the user know that this process is complete
						AfxMessageBox("All users will now have to change their passwords the next time they login to Practice.\n"
							"All new passwords will be forced to follow your new password strengths.");
					}
					else {
						//they aren't making user passwords expire, so warn that these changes won't be enforced until
						//users change their password on their own
						AfxMessageBox("No users were updated. Your new password strengths will be required for all future user password changes.\n"
							"Existing user passwords are still valid until they are next changed.");
					}
				}
			}

			//we'll always have audited the strength change,
			//we may have also audited user changes too
			//as well as the login banner
			CommitAuditTransaction(nAuditTransactionID);
		}

		CNxDialog::OnOK();

	}NxCatchAllCall("CPasswordConfigDlg::OnOk",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

void CPasswordConfigDlg::OnCancel()
{
	try {

		CNxDialog::OnCancel();

	}NxCatchAll("CPasswordConfigDlg::OnCancel");
}

void CPasswordConfigDlg::OnEnKillfocusEditPasswordMinCharacters()
{
	try {

		CalcSamplePassword();

	}NxCatchAll("CPasswordConfigDlg::OnEnKillfocusEditPasswordMinCharacters");
}

void CPasswordConfigDlg::OnEnKillfocusEditPasswordMinLetters()
{
	try {

		CalcSamplePassword();

	}NxCatchAll("CPasswordConfigDlg::OnEnKillfocusEditPasswordMinLetters");
}

void CPasswordConfigDlg::OnEnKillfocusEditPasswordMinNumbers()
{
	try {

		CalcSamplePassword();

	}NxCatchAll("CPasswordConfigDlg::OnEnKillfocusEditPasswordMinNumbers");
}

void CPasswordConfigDlg::OnEnKillfocusEditPasswordMinUppercase()
{
	try {

		CalcSamplePassword();

	}NxCatchAll("CPasswordConfigDlg::OnEnKillfocusEditPasswordMinUppercase");
}

void CPasswordConfigDlg::OnEnKillfocusEditPasswordMinLowercase()
{
	try {

		CalcSamplePassword();

	}NxCatchAll("CPasswordConfigDlg::OnEnKillfocusEditPasswordMinLowercase");
}

// (j.jones 2009-05-19 14:32) - PLID 33801 - supported preventing reuse of last X passwords
void CPasswordConfigDlg::OnCheckPreventReusePasswords()
{
	try {

		m_editPreventLastPasswords.EnableWindow(m_checkPreventReuseOfLastPasswords.GetCheck());

	}NxCatchAll("CPasswordConfigDlg::OnCheckPreventReusePasswords");
}

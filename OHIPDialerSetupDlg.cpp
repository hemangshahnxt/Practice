// OHIPDialerSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OHIPDialerSetupDlg.h"
#include "OHIPDialerUtils.h"
#include "RegUtils.h"
#include "FinancialRc.h"

// COHIPDialerSetupDlg dialog

// (j.jones 2009-03-09 13:28) - PLID 32405 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum ProviderFilterColumns {

	pfcID = 0,
	pfcName,
	pfcOHIPUserName,
	pfcOHIPPassword,
	pfcOHIPChangePassword,
	pfcOHIPNewPassword,
	pfcOHIPPasswordLastChangedDate,
};
// (s.dhole 2011-01-17 14:06) - PLID 42145
CString m_strTTLocation,m_strNXOhipModemLocation;

COHIPDialerSetupDlg::COHIPDialerSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COHIPDialerSetupDlg::IDD, pParent)
{
	m_nCurProviderID = -1;
	m_bNeedSaveProviderData = FALSE;
}

COHIPDialerSetupDlg::~COHIPDialerSetupDlg()
{
}

void COHIPDialerSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_SEND, m_btnSend);
	DDX_Control(pDX, IDC_BTN_BROWSE_DOWNLOAD, m_btnBrowseDownload);
	DDX_Control(pDX, IDC_BTN_BROWSE_TT, m_btnBrowseTT);
	DDX_Control(pDX, IDC_CHECK_CHANGE_PASSWORD, m_checkChangePassword);
	DDX_Control(pDX, IDC_CHECK_DELETE_REPORTS, m_checkDeleteReports);
	DDX_Control(pDX, IDC_CHECK_ENABLE_AUTO_DIALER, m_checkEnableAutoDialer);
	DDX_Control(pDX, IDC_CHECK_SHOW_TT, m_checkShowTerminal);
	DDX_Control(pDX, IDC_EDIT_DOWNLOAD_PATH, m_editDownloadPath);
	DDX_Control(pDX, IDC_EDIT_PHONE_NUMBER, m_editPhoneNumber);
	DDX_Control(pDX, IDC_EDIT_COMMPORT, m_editCommPort);
	DDX_Control(pDX, IDC_EDIT_USERNAME, m_editUsername);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_editPassword);
	DDX_Control(pDX, IDC_EDIT_NEW_PASSWORD, m_editNewPassword);
	DDX_Control(pDX, IDC_EDIT_TERATERM_LOC, m_editTTLocation);
	DDX_Control(pDX, IDC_CHECK_SUBMIT_PER_PROVIDER, m_checkSubmitPerProvider);	
	DDX_Control(pDX, IDC_OHIP_PROVIDER_LABEL, m_nxstaticProviderLabel);	
	DDX_Control(pDX, IDC_CHECK_TERATERM, m_checkUseTeraTerm);	
	
}


BEGIN_MESSAGE_MAP(COHIPDialerSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDC_BTN_SEND, OnBtnSend)
	ON_BN_CLICKED(IDC_BTN_BROWSE_DOWNLOAD, OnBtnBrowseDownload)
	ON_BN_CLICKED(IDC_BTN_BROWSE_TT, OnBtnBrowseTt)
	ON_BN_CLICKED(IDC_CHECK_CHANGE_PASSWORD, OnCheckChangePassword)
	ON_BN_CLICKED(IDC_CHECK_SUBMIT_PER_PROVIDER, OnCheckSubmitPerProvider)
	ON_BN_CLICKED(IDC_CHECK_TERATERM, &COHIPDialerSetupDlg::OnBnClickedCheckTeraterm)
	ON_EN_KILLFOCUS(IDC_EDIT_TERATERM_LOC, &COHIPDialerSetupDlg::OnEnKillfocusEditTeratermLoc)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_AUTO_DIALER, &COHIPDialerSetupDlg::OnBnClickedCheckEnableAutoDialer)
END_MESSAGE_MAP()


// COHIPDialerSetupDlg message handlers

BOOL COHIPDialerSetupDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnSend.AutoSet(NXB_EXPORT);
	
		m_editDownloadPath.SetLimitText(1000);

		// (j.jones 2009-04-03 13:30) - PLID 33846 - added limit of 15 characters
		m_editPassword.SetLimitText(15);
		m_editNewPassword.SetLimitText(15);

		// (j.jones 2009-08-03 12:22) - PLID 34914 - enforced a username limit of 50 characters
		m_editUsername.SetLimitText(50);

		// (j.jones 2009-08-14 10:54) - PLID 34914 - added per-provider options
		m_ProviderCombo = BindNxDataList2Ctrl(IDC_OHIP_DIALER_PROVIDER_COMBO);

		//load the settings from the registry

		CString strPhoneNumber, strUserName, strPassword, strNewPassword;
		CString strDownloadPath;//, strTTLocation,strNXOhipModemLocation;
		long nCommPort = 3;
		BOOL bDeleteReports = FALSE, bChangePassword = FALSE, bShowTerminalWindow = FALSE;
		BOOL bSeparateAccountsPerProviders = FALSE;
			// (s.dhole 2011-01-17 14:06) - PLID 42145
		BOOL bTeraTerm = TRUE;

		bool bEnableAutoDial = true; 
		
		// (j.jones 2009-08-14 10:38) - PLID 34914 - added OHIP per-provider setting		
		// (s.dhole 2011-01-17 14:06) - PLID 42145 Added m_strNXOhipModemLocation , bTeraTerm 
		LoadDialerSettings(strPhoneNumber, strUserName, strPassword, nCommPort, strDownloadPath, m_strTTLocation,
			bDeleteReports, bChangePassword, strNewPassword, bShowTerminalWindow, bSeparateAccountsPerProviders,m_strNXOhipModemLocation , bTeraTerm, bEnableAutoDial);
		
		SetDlgItemText(IDC_EDIT_PHONE_NUMBER, strPhoneNumber);		
		SetDlgItemInt(IDC_EDIT_COMMPORT, nCommPort);
		// (s.dhole 2011-01-17 14:06) - PLID 42145 Set Location text and value
		if (bTeraTerm==TRUE) 
		{
			SetDlgItemText(IDC_STATIC_LOCATION, "TeraTerm Location");
			SetDlgItemText(IDC_EDIT_TERATERM_LOC, m_strTTLocation);
		}
		else
		{
		SetDlgItemText(IDC_STATIC_LOCATION, "NxModem Location");
		SetDlgItemText(IDC_EDIT_TERATERM_LOC, m_strNXOhipModemLocation);
		}

		SetDlgItemText(IDC_EDIT_DOWNLOAD_PATH, strDownloadPath);
		m_checkDeleteReports.SetCheck(bDeleteReports);		
		m_checkShowTerminal.SetCheck(bShowTerminalWindow);
		// (s.dhole 2011-01-17 11:47) - PLID By    default use Teraterm  
		m_checkUseTeraTerm.SetCheck(bTeraTerm);
		// (j.jones 2009-08-14 10:54) - PLID 34914 - added per-provider options
		m_checkSubmitPerProvider.SetCheck(bSeparateAccountsPerProviders);


		EnableAllControls(bEnableAutoDial);
		m_checkEnableAutoDialer.SetCheck(bEnableAutoDial);


		if(bSeparateAccountsPerProviders) {
			//show the filter, change the label
			GetDlgItem(IDC_OHIP_DIALER_PROVIDER_COMBO)->ShowWindow(SW_SHOW);
			m_nxstaticProviderLabel.SetWindowText("Provider:");

			//select the first provider in the list
			m_ProviderCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			if(m_ProviderCombo->GetRowCount() > 0) {				
				IRowSettingsPtr pRow = m_ProviderCombo->GetFirstRow();
				if(pRow) {
					m_ProviderCombo->PutCurSel(pRow);
					OnSelChosenOhipDialerProviderCombo(pRow);
				}
			}
		}
		else {
			//hide the filter, change the label
			GetDlgItem(IDC_OHIP_DIALER_PROVIDER_COMBO)->ShowWindow(SW_HIDE);
			m_nxstaticProviderLabel.SetWindowText("Login Information:");

			//show the workstation login info
			SetDlgItemText(IDC_EDIT_USERNAME, strUserName);
			SetDlgItemText(IDC_EDIT_PASSWORD, strPassword);
			m_checkChangePassword.SetCheck(bChangePassword);
			m_editNewPassword.SetReadOnly(!bChangePassword);
			if(bChangePassword) {
				SetDlgItemText(IDC_EDIT_NEW_PASSWORD, strNewPassword);
			}
			else {
				SetDlgItemText(IDC_EDIT_NEW_PASSWORD, "");
			}
		}

	} NxCatchAll("Error in COHIPDialerSetupDlg::OnInitDialog");

	return FALSE;
}

void COHIPDialerSetupDlg::OnOk()
{
	try {

		if(!Save()) {
			return;
		}

		CNxDialog::OnOK();

	} NxCatchAll("Error in COHIPDialerSetupDlg::OnOk");
}

void COHIPDialerSetupDlg::OnBtnSend()
{
	try {

		if(!Save()) {
			return;
		}

		// (j.jones 2009-03-10 14:17) - PLID 33418 - browse for a file to send, then call SendClaimFileToOHIP to submit it
		// (j.armen 2011-10-25 15:16) - PLID 46134 - Ebilling is in the practice path
		CFileDialog dlgBrowse(TRUE, "*.*", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOREADONLYRETURN, "Claim Files|*.*|");
		// (v.maida 2016-05-19 17:01) - NX-100684 - Updated to used the shared path for Azure.
		CString strInitPath = GetEnvironmentDirectory() ^ "EBilling\\";
		dlgBrowse.m_ofn.lpstrInitialDir = strInitPath;
		if (dlgBrowse.DoModal() == IDOK) 
		{
			CString strPath = dlgBrowse.GetPathName();

			if(strPath.IsEmpty() || !DoesExist(strPath)) {
				AfxMessageBox("The path you have specified does not point to a valid file.");
				return;
			}

			// (j.jones 2009-08-14 11:53) - PLID 34914 - are they sending per-provider?
			long nProviderID = -1;
			if(m_checkSubmitPerProvider.GetCheck()) {
				IRowSettingsPtr pRow = m_ProviderCombo->GetCurSel();
				if(pRow == NULL) {
					AfxMessageBox("You must select a provider to send a claim for.");
					return;
				}

				nProviderID = VarLong(pRow->GetValue(pfcID));
			}

			SendClaimFileToOHIP(strPath, nProviderID);

			//regardless of the return value, a message will have been given to the user
			//indicating success or failure

			// (j.jones 2009-08-14 14:32) - PLID 34914 - this function could have updated passwords,
			// so we need to reload the list of providers (which has cached information
			if(nProviderID != -1) {

				m_ProviderCombo->Requery();

				IRowSettingsPtr pRow = m_ProviderCombo->SetSelByColumn(pfcID, nProviderID);
				if(pRow == NULL) {
					//set the first selection
					pRow = m_ProviderCombo->GetFirstRow();
					if(pRow) {
						m_ProviderCombo->PutCurSel(pRow);					
					}
				}
				
				//load the data
				OnSelChosenOhipDialerProviderCombo(pRow);
			}
		}

	} NxCatchAll("Error in COHIPDialerSetupDlg::OnBtnSend");
}

void COHIPDialerSetupDlg::OnBtnBrowseDownload()
{
	try {

		CString strCurPath;
		GetDlgItemText(IDC_EDIT_DOWNLOAD_PATH, strCurPath);
		if(Browse(GetSafeHwnd(), strCurPath, strCurPath, false)) {

			//a path this long would be crazy, but let's cleanly handle this now because
			//we can't store paths this long in the ConfigRT data
			if(strCurPath.GetLength() > 1000) {
				AfxMessageBox("Practice does not support download folder paths of this length.\n"
					"You must select a download path of a shorter length to use this feature.");
				return;
			}

			SetDlgItemText(IDC_EDIT_DOWNLOAD_PATH, strCurPath);
		}

	} NxCatchAll("Error in COHIPDialerSetupDlg::OnBtnBrowseDownload");
}

void COHIPDialerSetupDlg::OnBtnBrowseTt()
{
	try {

		CString strInitPath;

		CFileDialog dlgBrowse(TRUE, "*.exe", NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOREADONLYRETURN, "Executable Files|*.exe|");

		GetDlgItemText(IDC_EDIT_TERATERM_LOC, strInitPath);
		if (strInitPath == "") {
			strInitPath = "c:\\";
		}

		dlgBrowse.m_ofn.lpstrInitialDir = strInitPath;
		if (dlgBrowse.DoModal() == IDOK) 
		{
			CString strPath = dlgBrowse.GetPathName();
			SetDlgItemText(IDC_EDIT_TERATERM_LOC, strPath);
		}

	} NxCatchAll("Error in COHIPDialerSetupDlg::OnBtnBrowseTt");
}

void COHIPDialerSetupDlg::OnCheckChangePassword()
{
	try {

		BOOL bEnable = m_checkChangePassword.GetCheck();
		m_editNewPassword.SetReadOnly(!bEnable);

		if(bEnable) {

			// (j.jones 2009-04-03 15:03) - PLID 33846 - generate a password if the preference is enabled
			// (would already be cached upon loading this screen)
			if(GetRemotePropertyInt("OHIPAutoChangePasswords", 0, 0, "<None>", TRUE) != 0) {
				CString strPassword;
				GetDlgItemText(IDC_EDIT_PASSWORD, strPassword);
				CString strNewPassword = "";

				// (j.jones 2009-08-14 11:30) - PLID 34914 - support per provider options
				long nProviderID = -1;
				if(m_checkSubmitPerProvider.GetCheck()) {
					nProviderID = m_nCurProviderID;
				}
				
				strNewPassword = CalcNewOHIPPassword(strPassword, nProviderID);

				SetDlgItemText(IDC_EDIT_NEW_PASSWORD, strNewPassword);
			}

			AfxMessageBox("If you choose to change your password, the password will be changed upon the next login to OHIP.");
		}
		else {
			SetDlgItemText(IDC_EDIT_NEW_PASSWORD, "");
		}

	} NxCatchAll("Error in COHIPDialerSetupDlg::OnCheckChangePassword");
}

BOOL COHIPDialerSetupDlg::Save()
{
	try {

		//first silently cache the current provider, it won't save yet
		if(!ValidateAndCacheCurrentProviderInfo()) {
			return FALSE;
		}

		CString strPhoneNumber, strUserName, strPassword, strNewPassword, strFilePath;
		GetDlgItemText(IDC_EDIT_PHONE_NUMBER, strPhoneNumber);
		GetDlgItemText(IDC_EDIT_USERNAME, strUserName);
		GetDlgItemText(IDC_EDIT_PASSWORD, strPassword);
		GetDlgItemText(IDC_EDIT_NEW_PASSWORD, strNewPassword);
		BOOL bChangePassword = m_checkChangePassword.GetCheck();
		CString strTTLocation;
		
		CString strDownloadPath;
		GetDlgItemText(IDC_EDIT_DOWNLOAD_PATH, strDownloadPath);
		BOOL bDeleteReports = m_checkDeleteReports.GetCheck();
		BOOL bShowTerminalWindow = m_checkShowTerminal.GetCheck();

		strPhoneNumber.TrimLeft(); strPhoneNumber.TrimRight();
		strUserName.TrimLeft(); strUserName.TrimRight();
		strPassword.TrimLeft(); strPassword.TrimRight();
		strNewPassword.TrimLeft(); strNewPassword.TrimRight();
		strDownloadPath.TrimLeft(); strDownloadPath.TrimRight();
		
		// (s.dhole 2011-01-17 14:06) - PLID 42145 
		BOOL bTeraTerm = m_checkUseTeraTerm.GetCheck();
		CString strModemLocation;

		// (b.spivey, June 26th, 2014) - PLID 62603 - So we get the auto dialer settings first. If it's 
		//		Enabled, proceed as usual. If it's not, we do not care what values are in there, don't save. 
		bool bEnableAutoDialer = !!m_checkEnableAutoDialer.GetCheck();
				
			
		if (bEnableAutoDialer) {
			short nPort = 3;
			nPort = GetDlgItemInt(IDC_EDIT_COMMPORT);

			// (s.dhole 2011-01-17 14:06) - PLID 42145 
			if (bTeraTerm == TRUE)
			{
				GetDlgItemText(IDC_EDIT_TERATERM_LOC, strTTLocation);
				strTTLocation.TrimLeft(); strTTLocation.TrimRight();
				strModemLocation = m_strNXOhipModemLocation;
				if (strTTLocation.IsEmpty() || !DoesExist(strTTLocation)) {
					AfxMessageBox("You must enter a valid location for the TeraTerm product.");
					return FALSE;
				}
			}
			else
			{
				GetDlgItemText(IDC_EDIT_TERATERM_LOC, strModemLocation);
				strModemLocation.TrimLeft(); strModemLocation.TrimRight();
				strTTLocation = m_strTTLocation;
				if (strModemLocation.IsEmpty() || !DoesExist(strModemLocation)) {

					AfxMessageBox("You must enter a valid location for the NxModem product.");
					return FALSE;
				}
			}

			if (strDownloadPath.IsEmpty() || !DoesExist(strDownloadPath)) {
				AfxMessageBox("You must enter a valid location for the download path.");
				return FALSE;
			}

			//a path this long would be crazy, but let's cleanly handle this now because
			//we can't store paths this long in the ConfigRT data
			if (strDownloadPath.GetLength() > 1000) {
				AfxMessageBox("Practice does not support download folder paths of this length.\n"
					"You must select a download path of a shorter length to use this feature.");
				return FALSE;
			}

			if (nPort <= 0 || nPort > 256) {
				AfxMessageBox("You must enter valid comm port number (1 - 256).");
				return FALSE;
			}

			if (strPhoneNumber.IsEmpty()) {
				AfxMessageBox("You must enter a phone number to continue.");
				return FALSE;
			}

			if (!m_checkSubmitPerProvider.GetCheck()) {
				//they are saving to the workstation, so validate the login data

				if (strUserName.IsEmpty() || strPassword.IsEmpty()) {
					AfxMessageBox("You must enter a username and password to continue.");
					return FALSE;
				}

				if (!bChangePassword) {
					strNewPassword = "";
				}
				else {
					//they want to change the password, let's make sure they enter in a valid one

					if (strNewPassword.GetLength() < 8 || strNewPassword.GetLength() > 15) {
						AfxMessageBox("You have indicated that you wish to change your password, but the new password is an invalid length.\n"
							"You must enter a password between 8 and 15 characters to continue.");
						return FALSE;
					}

					// (j.jones 2009-04-03 13:34) - PLID 33846 - this function will check the last 5 passwords used
					// (j.jones 2009-08-14 11:27) - PLID 34914 - renamed to be different from the per-provider ability
					if (HasOHIPPasswordBeenUsed_Workstation(strNewPassword, strPassword)) {
						AfxMessageBox("You have indicated that you wish to change your password, but the new password you entered has been used in the past.\n"
							"You must enter a password that has not been used as one of your last 5 passwords.");
						return FALSE;
					}

					// (j.jones 2009-03-10 15:38) - PLID 33432 - OHIP has a very unusual rule of not allowing
					// passwords to have repeating characters next to each other, so we should validate that
					BOOL bValid = TRUE;
					CString strNewPasswordCaps = strNewPassword;
					strNewPasswordCaps.MakeUpper();
					for (int i = 0; i < strNewPasswordCaps.GetLength() - 1 && bValid; i++) {
						char chThis = strNewPasswordCaps.GetAt(i);
						char chNext = strNewPasswordCaps.GetAt(i + 1);
						if (chThis == chNext) {
							bValid = FALSE;
						}
					}

					if (!bValid) {
						AfxMessageBox("You have indicated that you wish to change your password, but the new password is invalid.\n"
							"OHIP does not allow passwords with two identical characters next to each other.\n\n"
							"You must enter a new password to continue.");
						return FALSE;
					}
				}

				//save now
				// (j.jones 2009-09-15 14:48) - PLID 33955 - these are all saved in ConfigRT now
				SetRemotePropertyText("OHIP_UserName", strUserName, 0, "<None>");
				SetRemotePropertyText("OHIP_Password", strPassword, 0, "<None>");
				SetRemotePropertyInt("OHIP_ChangePassword", bChangePassword ? 1 : 0, 0, "<None>");
				SetRemotePropertyText("OHIP_NewPassword", strNewPassword, 0, "<None>");
			}
			else {

				CString strSqlBatch;

				//warn if any providers are empty
				BOOL bWarnBlankProvider = FALSE;

				IRowSettingsPtr pRow = m_ProviderCombo->GetFirstRow();
				while (pRow) {

					long nProviderID = VarLong(pRow->GetValue(pfcID));
					CString strUserName = VarString(pRow->GetValue(pfcOHIPUserName));
					CString strPassword = VarString(pRow->GetValue(pfcOHIPPassword));
					BOOL bChangePassword = VarBool(pRow->GetValue(pfcOHIPChangePassword));
					CString strNewPassword = VarString(pRow->GetValue(pfcOHIPNewPassword));

					if (strUserName.IsEmpty() || strPassword.IsEmpty()) {
						bWarnBlankProvider = TRUE;
					}

					if (m_bNeedSaveProviderData) {

						//save for each provider, it should have already been validated
						AddStatementToSqlBatch(strSqlBatch, "UPDATE ProvidersT SET OHIPUserName = '%s', "
							"OHIPPassword = '%s', OHIPChangePassword= %li, OHIPNewPassword = '%s' "
							"WHERE PersonID = %li", _Q(strUserName), _Q(strPassword), bChangePassword ? 1 : 0,
							_Q(strNewPassword), nProviderID);
					}

					pRow = pRow->GetNextRow();
				}

				if (bWarnBlankProvider
					&& IDNO == MessageBox("At least one provider does not have a username or password entered. Are you sure you wish to continue?", "Practice", MB_YESNO | MB_ICONEXCLAMATION)) {
					return FALSE;
				}

				if (m_bNeedSaveProviderData && !strSqlBatch.IsEmpty()) {
					ExecuteSqlBatch(strSqlBatch);
				}

				m_bNeedSaveProviderData = FALSE;
			}

			// (j.jones 2009-09-15 14:48) - PLID 33955 - these are all saved in ConfigRT now
			SetRemotePropertyText("OHIP_DialupNumber", strPhoneNumber, 0, "<None>");
			SetPropertyInt("OHIP_CommPort", nPort, 0);
			SetPropertyText("OHIP_TTLocation", strTTLocation, 0);
			// (s.dhole 2011-01-17 14:06) - PLID 42145 
			SetPropertyInt("OHIP_UseTeraTerm", bTeraTerm, 0);
			// (s.dhole 2011-01-17 14:06) - PLID 42145 
			SetPropertyText("OHIP_NXModemLocation", strModemLocation, 0);
			SetRemotePropertyInt("OHIP_DeleteReports", bDeleteReports ? 1 : 0, 0, "<None>");
			SetRemotePropertyInt("OHIP_ShowTerminalWindow", bShowTerminalWindow ? 1 : 0, 0, "<None>");


			//this is a change from the OHIP dialer - we want our download path to be the OHIP report manager path,
			//which defaults to the shared path, and is stored per machine (like these registry keys)
			SetPropertyText("OHIPReportManager_ScanReportPath", strDownloadPath, 0);

			// (j.jones 2009-08-14 12:26) - PLID 34914 - save the per-provider setting
			SetRemotePropertyInt("OHIP_SeparateAccountsPerProviders", m_checkSubmitPerProvider.GetCheck() ? 1 : 0, 0, "<None>");

		}

		// (b.spivey, June 26th, 2014) - PLID 62603 - Save this value every time. 
		SetPropertyInt("OHIP_AutoDialerEnabled", (bEnableAutoDialer ? 1 : 0));

		return TRUE;

	} NxCatchAll("Error in COHIPDialerSetupDlg::Save");

	return FALSE;
}

// (j.jones 2009-08-14 10:35) - PLID 34914 - added per-provider login information
void COHIPDialerSetupDlg::OnCheckSubmitPerProvider()
{
	try {

		//don't bother saving what was previously in this field,
		//because they are changing the setting completely
		
		if(m_checkSubmitPerProvider.GetCheck()) {
			//show the filter, change the label
			GetDlgItem(IDC_OHIP_DIALER_PROVIDER_COMBO)->ShowWindow(SW_SHOW);
			m_nxstaticProviderLabel.SetWindowText("Provider:");

			//select the first provider in the list
			if(m_ProviderCombo->GetRowCount() > 0) {
				IRowSettingsPtr pRow = m_ProviderCombo->GetFirstRow();
				if(pRow) {
					m_ProviderCombo->PutCurSel(pRow);
					OnSelChosenOhipDialerProviderCombo(pRow);
				}
			}
		}
		else {
			//hide the filter, change the label
			GetDlgItem(IDC_OHIP_DIALER_PROVIDER_COMBO)->ShowWindow(SW_HIDE);
			m_nxstaticProviderLabel.SetWindowText("Login Information:");

			//clear the last selected provider
			m_nCurProviderID = -1;

			//reload the per-workstation info

			CString strPhoneNumber, strUserName, strPassword, strNewPassword;
			CString strDownloadPath, strTTLocation,strNXOhipModemLocation;
			long nCommPort = 3;
			BOOL bDeleteReports = FALSE, bChangePassword = FALSE, bShowTerminalWindow = FALSE;
			BOOL bSeparateAccountsPerProviders = FALSE,bTeraTerm=TRUE;

			bool bEnableAutoDialer = true;

			// (s.dhole 2011-01-17 14:06) - PLID 42145 Added strNXOhipModemLocation, bTeraTerm
			LoadDialerSettings(strPhoneNumber, strUserName, strPassword, nCommPort, strDownloadPath, strTTLocation,
				bDeleteReports, bChangePassword, strNewPassword, bShowTerminalWindow, bSeparateAccountsPerProviders,strNXOhipModemLocation, bTeraTerm, bEnableAutoDialer);

			//update just the login info
			SetDlgItemText(IDC_EDIT_USERNAME, strUserName);
			SetDlgItemText(IDC_EDIT_PASSWORD, strPassword);
			m_checkChangePassword.SetCheck(bChangePassword);
			m_editNewPassword.SetReadOnly(!bChangePassword);
			if(bChangePassword) {
				SetDlgItemText(IDC_EDIT_NEW_PASSWORD, strNewPassword);
			}
			else {
				SetDlgItemText(IDC_EDIT_NEW_PASSWORD, "");
			}
		}

	} NxCatchAll("Error in COHIPDialerSetupDlg::OnCheckSubmitPerProvider");
}
BEGIN_EVENTSINK_MAP(COHIPDialerSetupDlg, CNxDialog)
	ON_EVENT(COHIPDialerSetupDlg, IDC_OHIP_DIALER_PROVIDER_COMBO, 16, OnSelChosenOhipDialerProviderCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

// (j.jones 2009-08-14 10:35) - PLID 34914 - added per-provider login information
void COHIPDialerSetupDlg::OnSelChosenOhipDialerProviderCombo(LPDISPATCH lpRow)
{
	try {

		//first silently cache the current provider, it won't save yet
		if(!ValidateAndCacheCurrentProviderInfo()) {
			//the old row would have been re-selected
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//select the first row
			pRow = m_ProviderCombo->GetFirstRow();
			m_ProviderCombo->PutCurSel(pRow);

			if(pRow == NULL) {
				ThrowNxException("No providers available!");
			}
		}

		//load the cached info
		long nProviderID = VarLong(pRow->GetValue(pfcID));
		CString strUserName = VarString(pRow->GetValue(pfcOHIPUserName));
		CString strPassword = VarString(pRow->GetValue(pfcOHIPPassword));
		BOOL bChangePassword = VarBool(pRow->GetValue(pfcOHIPChangePassword));
		CString strNewPassword = VarString(pRow->GetValue(pfcOHIPNewPassword));

		SetDlgItemText(IDC_EDIT_USERNAME, strUserName);
		SetDlgItemText(IDC_EDIT_PASSWORD, strPassword);

		// (j.jones 2009-08-14 14:30) - PLID 34914 - enforce changing the password routinely,
		// but don't bother if we're already changing the password
		// (the preference to change this routinely is global, but the tracking information is per-provider)
		if(!bChangePassword && GetRemotePropertyInt("OHIPAutoChangePasswords", 0, 0, "<None>", TRUE) != 0) {
			
			//how long ago did we change the password?
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);

			//defaults to invalid date time
			COleDateTime dtOld = COleDateTime(1901, 1, 1, 1, 1, 1);
			COleDateTime dtLast = VarDateTime(pRow->GetValue(pfcOHIPPasswordLastChangedDate), dtOld);
			if(dtLast.GetStatus() == COleDateTime::invalid) {
				//invalid date time, definitely change the password
				bChangePassword = TRUE;
			}
			else {
				COleDateTime dtNow = COleDateTime::GetCurrentTime();
				COleDateTimeSpan dtSpan = dtNow - dtLast;
				if(dtSpan.GetDays() >= 25) {
					//it's been 25 days or more, so change it now
					bChangePassword = TRUE;
				}
			}

			if(bChangePassword) {
				//if we're changing the password, we must auto-generate a new one

				//this function will return a valid password that hasn't been used in the past 5 passwords
				strNewPassword = CalcNewOHIPPassword(strPassword, nProviderID);
			}
		}

		m_checkChangePassword.SetCheck(bChangePassword);
		m_editNewPassword.SetReadOnly(!bChangePassword);
		if(bChangePassword) {
			SetDlgItemText(IDC_EDIT_NEW_PASSWORD, strNewPassword);
		}
		else {
			SetDlgItemText(IDC_EDIT_NEW_PASSWORD, "");
		}

		//track this provider ID
		m_nCurProviderID = VarLong(pRow->GetValue(pfcID));

	} NxCatchAll("Error in COHIPDialerSetupDlg::OnSelChosenOhipDialerProviderCombo");
}

// (j.jones 2009-08-14 11:12) - PLID 34914 - used for provider info. saving
BOOL COHIPDialerSetupDlg::ValidateAndCacheCurrentProviderInfo()
{
	try {

		if(m_nCurProviderID != -1 && m_checkSubmitPerProvider.GetCheck()) {

			CString strUserName, strPassword, strNewPassword;
			GetDlgItemText(IDC_EDIT_USERNAME, strUserName);
			GetDlgItemText(IDC_EDIT_PASSWORD, strPassword);
			GetDlgItemText(IDC_EDIT_NEW_PASSWORD, strNewPassword);
			BOOL bChangePassword = m_checkChangePassword.GetCheck();

			IRowSettingsPtr pOldRow = m_ProviderCombo->FindByColumn(pfcID, m_nCurProviderID, m_ProviderCombo->GetFirstRow(), FALSE);
			if(pOldRow) {

				CString strName = VarString(pOldRow->GetValue(pfcName));
				CString strOldUserName = VarString(pOldRow->GetValue(pfcOHIPUserName));
				CString strOldPassword = VarString(pOldRow->GetValue(pfcOHIPPassword));
				BOOL bOldChangePassword = VarBool(pOldRow->GetValue(pfcOHIPChangePassword));
				CString strOldNewPassword = VarString(pOldRow->GetValue(pfcOHIPNewPassword));

				//validate the information we entered

				if(strUserName.IsEmpty() || strPassword.IsEmpty()) {
					CString strWarn;
					strWarn.Format("You have not entered a username and password for the provider '%s'. Are you sure you wish to continue?", strName);
					if(IDNO == MessageBox(strWarn, "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {

						//select the old row
						m_ProviderCombo->PutCurSel(pOldRow);
						return FALSE;
					}
				}

				if(!bChangePassword) {
					strNewPassword = "";
				}
				else {
					//they want to change the password, let's make sure they enter in a valid one

					if(strNewPassword.GetLength() < 8 || strNewPassword.GetLength() > 15) {
						AfxMessageBox("You have indicated that you wish to change this provider's password, but the new password is an invalid length.\n"
							"You must enter a password between 8 and 15 characters to continue.");

						//select the old row
						m_ProviderCombo->PutCurSel(pOldRow);
						return FALSE;
					}

					// (j.jones 2009-04-03 13:34) - PLID 33846 - this function will check the last 5 passwords used
					if(HasOHIPPasswordBeenUsed_Provider(m_nCurProviderID, strNewPassword, strPassword)) {
						AfxMessageBox("You have indicated that you wish to change this provider's password, but the new password you entered has been used in the past.\n"
							"You must enter a password that has not been used as one of your last 5 passwords for this provider.");

						//select the old row
						m_ProviderCombo->PutCurSel(pOldRow);
						return FALSE;
					}

					// (j.jones 2009-03-10 15:38) - PLID 33432 - OHIP has a very unusual rule of not allowing
					// passwords to have repeating characters next to each other, so we should validate that
					BOOL bValid = TRUE;
					CString strNewPasswordCaps = strNewPassword;
					strNewPasswordCaps.MakeUpper();
					for(int i=0; i<strNewPasswordCaps.GetLength() - 1 && bValid; i++) {
						char chThis = strNewPasswordCaps.GetAt(i);
						char chNext = strNewPasswordCaps.GetAt(i+1);
						if(chThis == chNext) {
							bValid = FALSE;
						}
					}

					if(!bValid) {
						AfxMessageBox("You have indicated that you wish to change this provider's password, but the new password is invalid.\n"
							"OHIP does not allow passwords with two identical characters next to each other.\n\n"
							"You must enter a new password to continue.");
						
						//select the old row
						m_ProviderCombo->PutCurSel(pOldRow);
						return FALSE;
					}
				}

				//now cache our new information

				if(strOldUserName != strUserName) {
					pOldRow->PutValue(pfcOHIPUserName, _bstr_t(strUserName));
					m_bNeedSaveProviderData = TRUE;
				}

				if(strOldPassword != strPassword) {
					pOldRow->PutValue(pfcOHIPPassword, _bstr_t(strPassword));
					m_bNeedSaveProviderData = TRUE;
				}

				if(bOldChangePassword != bChangePassword) {
					if(bChangePassword) {
						pOldRow->PutValue(pfcOHIPChangePassword, g_cvarTrue);
					}
					else {
						pOldRow->PutValue(pfcOHIPChangePassword, g_cvarFalse);
					}
					m_bNeedSaveProviderData = TRUE;
				}

				if(strOldNewPassword != strNewPassword) {
					pOldRow->PutValue(pfcOHIPNewPassword, _bstr_t(strNewPassword));
					m_bNeedSaveProviderData = TRUE;
				}
			}
			else {
				//shouldn't even be possible, but if so, return silently
				ASSERT(FALSE);
				return TRUE;
			}
		}

		return TRUE;

	}NxCatchAll("Error in COHIPDialerSetupDlg::CacheCurrentProviderInfo");

	return FALSE;
}
// (s.dhole 2011-01-17 14:06) - PLID 42145 
void COHIPDialerSetupDlg::OnBnClickedCheckTeraterm()
{
	try{
	BOOL bTeraTerm = m_checkUseTeraTerm.GetCheck();
	if (bTeraTerm==TRUE){
		SetDlgItemText(IDC_STATIC_LOCATION, "TeraTerm Location");
		SetDlgItemText(IDC_EDIT_TERATERM_LOC, m_strTTLocation);
	}
	else{
		SetDlgItemText(IDC_STATIC_LOCATION, "NxModem Location");
		SetDlgItemText(IDC_EDIT_TERATERM_LOC, m_strNXOhipModemLocation);
	}
	
	}NxCatchAll("Error in COHIPDialerSetupDlg::OnBnClickedCheckTeraterm");
	
}
// (s.dhole 2011-01-17 14:06) - PLID 42145 

void COHIPDialerSetupDlg::OnEnKillfocusEditTeratermLoc()
{
	try{
	BOOL bTeraTerm = m_checkUseTeraTerm.GetCheck();
	if (bTeraTerm==TRUE){
		GetDlgItemText(IDC_EDIT_TERATERM_LOC, m_strTTLocation);
	}
	else{
		GetDlgItemText(IDC_EDIT_TERATERM_LOC, m_strNXOhipModemLocation);
	}
	}NxCatchAll("Error in COHIPDialerSetupDlg::OnEnKillfocusEditTeratermLoc");

	// TODO: Add your control notification handler code here
}

// (b.spivey, June 26th, 2014) - PLID 62603 - Enable all controls based on this checkbox. 
void COHIPDialerSetupDlg::EnableAllControls(bool bEnableControls)
{
	BOOL bEnableAllControls = bEnableControls ? 1 : 0; 

	m_checkChangePassword.EnableWindow(bEnableAllControls);
	m_btnBrowseTT.EnableWindow(bEnableAllControls);
	m_btnSend.EnableWindow(bEnableAllControls);
	m_btnBrowseDownload.EnableWindow(bEnableAllControls);
	m_checkChangePassword.EnableWindow(bEnableAllControls);
	m_checkDeleteReports.EnableWindow(bEnableAllControls);
	m_checkShowTerminal.EnableWindow(bEnableAllControls);
	m_editDownloadPath.EnableWindow(bEnableAllControls);
	m_editPhoneNumber.EnableWindow(bEnableAllControls);
	m_editCommPort.EnableWindow(bEnableAllControls);
	m_editUsername.EnableWindow(bEnableAllControls);
	m_editPassword.EnableWindow(bEnableAllControls);
	m_editNewPassword.EnableWindow(bEnableAllControls);
	m_editTTLocation.EnableWindow(bEnableAllControls);
	m_checkSubmitPerProvider.EnableWindow(bEnableAllControls);
	m_checkUseTeraTerm.EnableWindow(bEnableAllControls);
}

// (b.spivey, June 26th, 2014) - PLID 62603 - Message handler for the auto dialer checkbox. 
void COHIPDialerSetupDlg::OnBnClickedCheckEnableAutoDialer()
{
	try {
		if (!m_checkEnableAutoDialer.GetCheck()) {
			DontShowMeAgain(this, "By disabling the Dialer you will no longer be able to use the dial-up method to send and receive claims with OHIP ministries. You should only disable this "
				"feature if your office has switched completely off the dial-up method. OHIP will no longer be supporting this feature after January 1st, 2015.", "OHIPDialerSettingsAutoDialWarning");
		}
		EnableAllControls(!!m_checkEnableAutoDialer.GetCheck());
	} NxCatchAll(__FUNCTION__);
}
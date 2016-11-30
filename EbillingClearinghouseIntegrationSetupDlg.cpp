// EbillingAutomaticTransferSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EbillingClearinghouseIntegrationSetupDlg.h"
#include "AuditTrail.h"
#include "NxAPI.h"

// CEbillingAutomaticTransferSetupDlg dialog
using namespace ADODB;
using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CEbillingClearinghouseIntegrationSetupDlg, CNxDialog)

CEbillingClearinghouseIntegrationSetupDlg::CEbillingClearinghouseIntegrationSetupDlg(CWnd* pParent)
	: CNxDialog(CEbillingClearinghouseIntegrationSetupDlg::IDD, pParent)
{

}

CEbillingClearinghouseIntegrationSetupDlg::~CEbillingClearinghouseIntegrationSetupDlg()
{
}

void CEbillingClearinghouseIntegrationSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ENABLE_EBILLING_CLEARINGHOUSE_INTEGRATION_CHECK, m_checkEnableEbillingClearinghouseIntegration);
	DDX_Control(pDX, IDC_GLOBAL_CLEARINGHOUSE_LOGIN_RADIO, m_radioUseOneLoginForAllProvidersAndLocations);
	DDX_Control(pDX, IDC_PROVIDER_LOCATION_SPECIFIC_CLEARINGHOUSE_LOGINS_RADIO, m_radioUseADifferentLoginPerProviderPerLocation);
	DDX_Control(pDX, IDC_CLEARINGHOUSE_LOGIN_USERNAME_EDIT, m_nxeditSiteID);
	DDX_Control(pDX, IDC_CLEARINGHOUSE_PORTAL_PASSWORD_EDIT, m_nxeditPortalPassword);
	DDX_Control(pDX, IDC_CLEARINGHOUSE_FTP_PASSWORD_EDIT, m_nxeditFtpPassword);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CEbillingClearinghouseIntegrationSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_GLOBAL_CLEARINGHOUSE_LOGIN_RADIO, &CEbillingClearinghouseIntegrationSetupDlg::OnBnClickedClearinghouseLoginTypeRadio)
	ON_BN_CLICKED(IDC_PROVIDER_LOCATION_SPECIFIC_CLEARINGHOUSE_LOGINS_RADIO, &CEbillingClearinghouseIntegrationSetupDlg::OnBnClickedClearinghouseLoginTypeRadio)
	ON_EN_CHANGE(IDC_CLEARINGHOUSE_LOGIN_USERNAME_EDIT, &CEbillingClearinghouseIntegrationSetupDlg::OnEnChangeClearinghouseLoginUsernameEdit)
	ON_EN_CHANGE(IDC_CLEARINGHOUSE_PORTAL_PASSWORD_EDIT, &CEbillingClearinghouseIntegrationSetupDlg::OnEnChangeClearinghousePortalPasswordEdit)
	ON_EN_CHANGE(IDC_CLEARINGHOUSE_FTP_PASSWORD_EDIT, &CEbillingClearinghouseIntegrationSetupDlg::OnEnChangeClearinghouseFtpPasswordEdit)
	ON_BN_CLICKED(IDOK, &CEbillingClearinghouseIntegrationSetupDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// CEbillingClearinghouseIntegrationSetupDlg message handlers

BOOL CEbillingClearinghouseIntegrationSetupDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		BulkCacheSettings();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_nxeditSiteID.SetLimitText(50);
		m_nxeditPortalPassword.SetLimitText(50);
		m_nxeditFtpPassword.SetLimitText(50);

		m_ProvCombo = BindNxDataList2Ctrl(IDC_CLEARINGHOUSE_LOGIN_PROVIDER_COMBO, true);
		m_LocCombo = BindNxDataList2Ctrl(IDC_CLEARINGHOUSE_LOGIN_LOCATION_COMBO, true);
		
		// Before the dialog can load, we must have these combos filled, because we need to have a default provider and location
		// selected in order to load a login for. So wait on these synchronously.
		m_ProvCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		m_LocCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		Load();
	}
	NxCatchAll(__FUNCTION__);

	return FALSE;
}

void CEbillingClearinghouseIntegrationSetupDlg::BulkCacheSettings()
{
	g_propManager.CachePropertiesInBulk("CEbillingClearinghouseIntegrationSetupDlg-Number", propNumber,
		"(Username = '<None>') AND ("
		"Name = 'EbillingClearinghouseIntegration_Enabled' OR "
		"Name = 'Clearinghouse_LoginType' "
		")");
}

void CEbillingClearinghouseIntegrationSetupDlg::OnBnClickedClearinghouseLoginTypeRadio()
{
	try
	{
		LoadLogin();
		EnsureControls();
	}
	NxCatchAll(__FUNCTION__);
}

void CEbillingClearinghouseIntegrationSetupDlg::OnEnChangeClearinghouseLoginUsernameEdit()
{
	try
	{
		m_bLoginInfoChanged = TRUE;
	}
	NxCatchAll(__FUNCTION__);
}


void CEbillingClearinghouseIntegrationSetupDlg::OnEnChangeClearinghousePortalPasswordEdit()
{
	try
	{
		m_bLoginInfoChanged = TRUE;
	}
	NxCatchAll(__FUNCTION__);
}


void CEbillingClearinghouseIntegrationSetupDlg::OnEnChangeClearinghouseFtpPasswordEdit()
{
	try
	{
		m_bLoginInfoChanged = TRUE;
	}
	NxCatchAll(__FUNCTION__);
}


void CEbillingClearinghouseIntegrationSetupDlg::OnBnClickedOk()
{
	try
	{
		//save the settings
		if (!Save())
		{
			return;
		}
		CNxDialog::OnOK();
	}
	NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CEbillingClearinghouseIntegrationSetupDlg, CNxDialog)
	ON_EVENT(CEbillingClearinghouseIntegrationSetupDlg, IDC_CLEARINGHOUSE_LOGIN_PROVIDER_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEbillingClearinghouseIntegrationSetupDlg, IDC_CLEARINGHOUSE_LOGIN_PROVIDER_COMBO, 16, CEbillingClearinghouseIntegrationSetupDlg::SelChosenClearinghouseLoginProviderCombo, VTS_DISPATCH)
	ON_EVENT(CEbillingClearinghouseIntegrationSetupDlg, IDC_CLEARINGHOUSE_LOGIN_LOCATION_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEbillingClearinghouseIntegrationSetupDlg, IDC_CLEARINGHOUSE_LOGIN_LOCATION_COMBO, 16, CEbillingClearinghouseIntegrationSetupDlg::SelChosenClearinghouseLoginLocationCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CEbillingClearinghouseIntegrationSetupDlg::SelChosenClearinghouseLoginProviderCombo(LPDISPATCH lpRow)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL)
		{
			m_ProvCombo->SetSelByColumn((short)EProviderComboColumn::ID, m_nCurProviderID);
			return;
		}

		long nProviderID = VarLong(pRow->GetValue((short)EProviderComboColumn::ID));
		if (nProviderID != m_nCurProviderID && m_bLoginInfoChanged)
		{
			if (IDNO == MessageBox("Do you wish to save the current login information for the previous provider?", "Practice", MB_ICONQUESTION | MB_YESNO))
			{
				m_ProvCombo->SetSelByColumn((short)EProviderComboColumn::ID, m_nCurProviderID);
				return;
			}

			//just save everything
			if (!Save())
			{
				//something failed
				m_ProvCombo->SetSelByColumn((short)EProviderComboColumn::ID, m_nCurProviderID);
				return;
			}
		}

		m_nCurProviderID = nProviderID;
		LoadLogin();
	}
	NxCatchAll(__FUNCTION__);
}

void CEbillingClearinghouseIntegrationSetupDlg::SelChosenClearinghouseLoginLocationCombo(LPDISPATCH lpRow)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL)
		{
			m_LocCombo->SetSelByColumn((short)ELocationComboColumn::ID, m_nCurLocationID);
			return;
		}

		long nLocationID = VarLong(pRow->GetValue((short)ELocationComboColumn::ID));
		if (nLocationID != m_nCurLocationID && m_bLoginInfoChanged)
		{
			if (IDNO == MessageBox("Do you wish to save the current login information for the previous location?", "Practice", MB_ICONQUESTION | MB_YESNO))
			{
				m_LocCombo->SetSelByColumn((short)ELocationComboColumn::ID, m_nCurLocationID);
				return;
			}

			//just save everything
			if (!Save())
			{
				//something failed
				m_LocCombo->SetSelByColumn((short)ELocationComboColumn::ID, m_nCurLocationID);
				return;
			}
		}

		m_nCurLocationID = nLocationID;
		LoadLogin();
	}
	NxCatchAll(__FUNCTION__);
}

bool CEbillingClearinghouseIntegrationSetupDlg::UseGlobalLogin()
{
	return m_radioUseOneLoginForAllProvidersAndLocations.GetCheck() == BST_CHECKED;
}

void CEbillingClearinghouseIntegrationSetupDlg::EnsureControls()
{
	// Everything is accessible whether the integration is on or off because opening TriZetto's portal
	// requires we have a site ID and password. And it seems stupid to just disable the FTP password
	// and not clear it out if they disable the integration; it doesn't harm anything by being there.

	// The combo boxes should only be available if they want to use individual logins.
	GetDlgItem(IDC_CLEARINGHOUSE_LOGIN_PROVIDER_COMBO)->EnableWindow(!UseGlobalLogin() ? TRUE : FALSE);
	GetDlgItem(IDC_CLEARINGHOUSE_LOGIN_LOCATION_COMBO)->EnableWindow(!UseGlobalLogin() ? TRUE : FALSE);
}

NexTech_Accessor::_ClearinghouseLoginPtr CEbillingClearinghouseIntegrationSetupDlg::CreateLoginFromControls()
{
	NexTech_Accessor::_ClearinghouseLoginPtr pLogin(__uuidof(NexTech_Accessor::ClearinghouseLogin));

	pLogin->_ClearinghouseType = NexTech_Accessor::ClearinghouseType_TriZetto;

	if (UseGlobalLogin())
	{
		pLogin->LoginType = NexTech_Accessor::ClearinghouseLoginType::ClearinghouseLoginType_Global;
	}
	else
	{
		pLogin->LoginType = NexTech_Accessor::ClearinghouseLoginType::ClearinghouseLoginType_Individual;
	}

	if (m_nCurProviderID != -1 && !UseGlobalLogin())
	{
		pLogin->providerID = _bstr_t(FormatString("%li", m_nCurProviderID));
	}

	if (m_nCurLocationID != -1 && !UseGlobalLogin())
	{
		pLogin->locationID = _bstr_t(FormatString("%li", m_nCurLocationID));
	}

	CString strSiteID;
	m_nxeditSiteID.GetWindowText(strSiteID);
	pLogin->SiteID = _bstr_t(strSiteID);

	CString strFtpPassword;
	m_nxeditFtpPassword.GetWindowText(strFtpPassword);
	pLogin->password = _bstr_t(strFtpPassword);

	CString strPortalPassword;
	m_nxeditPortalPassword.GetWindowText(strPortalPassword);
	pLogin->PortalPassword = _bstr_t(strPortalPassword);

	return pLogin;
}

BOOL CEbillingClearinghouseIntegrationSetupDlg::Save()
{
	try
	{
		CWaitCursor wc;

		// Validate the current settings before attempting to save to the database.
		if (!ValidateBeforeSaving())
		{
			return FALSE;
		}

		if (!SaveLogin()) {
			return FALSE;
		}
		SaveEnabledState();
		SaveLoginType();
	
		// There are some things that can only be checked post saving.
		ValidateAfterSaving();
		return TRUE;
	}
	NxCatchAll(__FUNCTION__);

	return FALSE;
}

bool CEbillingClearinghouseIntegrationSetupDlg::ValidateBeforeSaving()
{
	NexTech_Accessor::_ClearinghouseLoginPtr pLogin = CreateLoginFromControls();
	if (pLogin->LoginType == NexTech_Accessor::ClearinghouseLoginType_Individual)
	{
		if (pLogin->providerID.length() == 0)
		{
			MessageBox("The Provider may not be empty. Please correct this and try saving again.", "Error", MB_ICONERROR | MB_OK);
			return false;
		}
		if (pLogin->locationID.length() == 0)
		{
			MessageBox("The Location may not be empty. Please correct this and try saving again.", "Error", MB_ICONERROR | MB_OK);
			return false;
		}
	}

	if (pLogin->SiteID.length() == 0 && pLogin->password.length() > 0)
	{
		MessageBox("The Login / Site ID may not be empty. Please correct this and try saving again.", "Error", MB_ICONERROR | MB_OK);
		return false;
	}
	else if (pLogin->SiteID.length() > 0 && pLogin->password.length() == 0)
	{
		MessageBox("The FTP Password may not be empty. Please correct this and try saving again.", "Error", MB_ICONERROR | MB_OK);
		return false;
	}

	return true;
}

void CEbillingClearinghouseIntegrationSetupDlg::ValidateAfterSaving()
{
	NexTech_Accessor::_ClearinghouseLoginPtr pLogin = CreateLoginFromControls();
	if (pLogin->SiteID.length() == 0)
	{
		// Logins with empty site IDs won't be sent anyway. Early out.
		return;
	}

	if (pLogin->LoginType == NexTech_Accessor::ClearinghouseLoginType_Individual)
	{
		NexTech_Accessor::_ClearinghouseLoginListPtr pMismatchingLogins =
			GetAPI()->GetMismatchingClearinghouseLogins(GetAPISubkey(), GetAPILoginToken(), pLogin);
		Nx::SafeArray<IUnknown *> saryMismatchingLogins(pMismatchingLogins->Logins);

		if (saryMismatchingLogins.GetSize() > 0)
		{
			CString strMismatchingLoginsMessage;
			if (saryMismatchingLogins.GetSize() == 1)
			{
				strMismatchingLoginsMessage = "There is one other clearinghouse login with the same "
					"Site ID as this one but a different FTP or Portal password. "
					"Nextech will now update the passwords of this other login.";
			}
			else
			{
				strMismatchingLoginsMessage = FormatString("There are %li other clearinghouse logins with the same "
					"Site ID as this one but different FTP or Portal passwords. "
					"Nextech will now update the passwords of these other logins.",
					saryMismatchingLogins.GetSize());
			}

			MessageBox(strMismatchingLoginsMessage, "Mismatched Logins", MB_ICONINFORMATION | MB_OK);
			// Fix the logins we know now are invalid. Do this unconditionally to keep data consistent
			// and to avoid a support case later due to mismatching logins.
			for (NexTech_Accessor::_ClearinghouseLoginPtr pOtherLogin : saryMismatchingLogins)
			{
				pOtherLogin->password = pLogin->password;
				pOtherLogin->PortalPassword = pLogin->PortalPassword;
			}
			GetAPI()->SaveClearinghouseLogins(GetAPISubkey(), GetAPILoginToken(), pMismatchingLogins, VARIANT_FALSE);
		}
		else
		{
			// No other logins use the same Site ID but different passwords.
		}
	}
	else
	{
		// No post-validation for global login type.
	}
}

void CEbillingClearinghouseIntegrationSetupDlg::SaveEnabledState()
{
	bool bOldEnabled = GetRemotePropertyInt("EbillingClearinghouseIntegration_Enabled", FALSE, 0, "<None>", false) != FALSE;

	bool bEnabled = m_checkEnableEbillingClearinghouseIntegration.GetCheck() == BST_CHECKED;
	SetRemotePropertyInt("EbillingClearinghouseIntegration_Enabled", bEnabled ? 1 : 0, 0, "<None>");

	if (bOldEnabled != bEnabled)
	{
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiEbillingClearinghouseIntegrationEnabled, -1,
			bOldEnabled ? "Enabled" : "Disabled",
			bEnabled ? "Enabled" : "Disabled",
			aepMedium, aetChanged);
	}
}

void CEbillingClearinghouseIntegrationSetupDlg::SaveLoginType()
{
	NexTech_Accessor::ClearinghouseLoginType apiClearinghouseLoginType;
	if (UseGlobalLogin())
	{
		apiClearinghouseLoginType = NexTech_Accessor::ClearinghouseLoginType::ClearinghouseLoginType_Global;
	}
	else
	{
		apiClearinghouseLoginType = NexTech_Accessor::ClearinghouseLoginType::ClearinghouseLoginType_Individual;
	}

	GetAPI()->SetClearinghouseLoginType(GetAPISubkey(), GetAPILoginToken(), NexTech_Accessor::ClearinghouseType_TriZetto, apiClearinghouseLoginType);
}

BOOL CEbillingClearinghouseIntegrationSetupDlg::SaveLogin()
{
	// Only validate the credentials if they have enabled the integration.
	VARIANT_BOOL bCheckCredentials = (m_checkEnableEbillingClearinghouseIntegration.GetCheck() == BST_CHECKED ? VARIANT_TRUE : VARIANT_FALSE);
	NexTech_Accessor::_SaveClearinghouseLoginResultPtr pResult =
		GetAPI()->SaveClearinghouseLogin(GetAPISubkey(), GetAPILoginToken(), CreateLoginFromControls(), bCheckCredentials);

	if (pResult->Success)
	{
		m_bLoginInfoChanged = FALSE;
		return TRUE;
	}
	else
	{
		CString strMessage = FormatString("An error occurred communicating with "
			"the clearinghouse server.\r\n\r\nDetails:\r\n%s", (LPCTSTR)pResult->RemoteErrorMessage);
		MessageBox(strMessage, "Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}
}

void CEbillingClearinghouseIntegrationSetupDlg::Load()
{
	try
	{
		LoadEnabledState();
		LoadLoginType();
		LoadDefaultProviderCombo();
		LoadDefaultLocationCombo();
		LoadLogin();
		EnsureControls();
	}
	NxCatchAll(__FUNCTION__);
}

void CEbillingClearinghouseIntegrationSetupDlg::LoadEnabledState()
{
	bool bIsIntegrationEnabled = GetRemotePropertyInt("EbillingClearinghouseIntegration_Enabled", FALSE, 0, "<None>", true) ? true : false;
	m_checkEnableEbillingClearinghouseIntegration.SetCheck(bIsIntegrationEnabled ? BST_CHECKED : BST_UNCHECKED);
}

void CEbillingClearinghouseIntegrationSetupDlg::LoadLoginType()
{
	NexTech_Accessor::ClearinghouseLoginType eClearinghouseLoginType = (NexTech_Accessor::ClearinghouseLoginType)GetRemotePropertyInt(
		"Clearinghouse_LoginType", (long)NexTech_Accessor::ClearinghouseLoginType_Global, (long)NexTech_Accessor::ClearinghouseType_TriZetto, "<None>", true);
	if (eClearinghouseLoginType != NexTech_Accessor::ClearinghouseLoginType_Global)
	{
		m_radioUseOneLoginForAllProvidersAndLocations.SetCheck(BST_UNCHECKED);
		m_radioUseADifferentLoginPerProviderPerLocation.SetCheck(BST_CHECKED);
	}
	else
	{
		m_radioUseOneLoginForAllProvidersAndLocations.SetCheck(BST_CHECKED);
		m_radioUseADifferentLoginPerProviderPerLocation.SetCheck(BST_UNCHECKED);
	}
}

void CEbillingClearinghouseIntegrationSetupDlg::LoadDefaultProviderCombo()
{
	IRowSettingsPtr pProvRow = m_ProvCombo->GetFirstRow();
	if (pProvRow)
	{
		m_nCurProviderID = VarLong(pProvRow->GetValue((short)EProviderComboColumn::ID));
		m_ProvCombo->PutCurSel(pProvRow);
	}
	else
	{
		ResetProviderCombo();
	}
}

void CEbillingClearinghouseIntegrationSetupDlg::LoadDefaultLocationCombo()
{
	m_nCurLocationID = GetCurrentLocationID();
	if (!m_LocCombo->SetSelByColumn((short)ELocationComboColumn::ID, m_nCurLocationID))
	{
		ResetLocationCombo();
	}
}

void CEbillingClearinghouseIntegrationSetupDlg::ResetProviderCombo()
{
	m_nCurProviderID = -1;
	m_ProvCombo->PutCurSel(nullptr);
	m_ProvCombo->ComboBoxText = _bstr_t("<No Provider>");
}

void CEbillingClearinghouseIntegrationSetupDlg::ResetLocationCombo()
{
	m_nCurLocationID = -1;
	m_LocCombo->PutCurSel(nullptr);
	m_LocCombo->ComboBoxText = _bstr_t("<No Location>");
}

NexTech_Accessor::_ClearinghouseLoginFilterPtr CEbillingClearinghouseIntegrationSetupDlg::CreateLoginFilterFromControls()
{
	NexTech_Accessor::_ClearinghouseLoginFilterPtr pLoginFilter(__uuidof(NexTech_Accessor::ClearinghouseLoginFilter));

	pLoginFilter->_ClearinghouseType = NexTech_Accessor::ClearinghouseType_TriZetto;

	if (UseGlobalLogin())
	{
		pLoginFilter->LoginType = NexTech_Accessor::ClearinghouseLoginType::ClearinghouseLoginType_Global;
	}
	else
	{
		pLoginFilter->LoginType = NexTech_Accessor::ClearinghouseLoginType::ClearinghouseLoginType_Individual;
	}

	if (m_nCurProviderID != -1 && !UseGlobalLogin())
	{
		pLoginFilter->providerID = _bstr_t(FormatString("%li", m_nCurProviderID));
	}

	if (m_nCurLocationID != -1 && !UseGlobalLogin())
	{
		pLoginFilter->locationID = _bstr_t(FormatString("%li", m_nCurLocationID));
	}

	return pLoginFilter;
}

void CEbillingClearinghouseIntegrationSetupDlg::LoadLogin()
{
	NexTech_Accessor::_ClearinghouseLoginPtr pLogin =
		GetAPI()->GetClearinghouseLogin(GetAPISubkey(), GetAPILoginToken(), CreateLoginFilterFromControls());

	m_nxeditSiteID.SetWindowText((LPCTSTR)pLogin->SiteID);
	m_nxeditFtpPassword.SetWindowText((LPCTSTR)pLogin->password);
	m_nxeditPortalPassword.SetWindowText((LPCTSTR)pLogin->PortalPassword);
	m_bLoginInfoChanged = FALSE;
}
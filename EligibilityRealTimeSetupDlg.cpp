// EligibilityRealTimeSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EligibilityRealTimeSetupDlg.h"
#include "EEligibility.h"
#include "NxAPI.h"
#include "AuditTrail.h"

// CEligibilityRealTimeSetupDlg dialog

// (j.jones 2010-07-02 14:22) - PLID 39506 - created

using namespace ADODB;
using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CEligibilityRealTimeSetupDlg, CNxDialog)

CEligibilityRealTimeSetupDlg::CEligibilityRealTimeSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEligibilityRealTimeSetupDlg::IDD, pParent), m_pCurrentLogin(__uuidof(NexTech_Accessor::ClearinghouseLogin))
{
	m_bLoginInfoChanged = FALSE;
}

CEligibilityRealTimeSetupDlg::~CEligibilityRealTimeSetupDlg()
{
}

void CEligibilityRealTimeSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_ENABLE_REALTIME_SENDS, m_checkEnableRealTime);
	DDX_Control(pDX, IDC_CHECK_OPEN_NOTEPAD, m_checkOpenNotepad);
	DDX_Control(pDX, IDC_RADIO_TRIZETTO_UNBATCH, m_radioUnbatch);
	DDX_Control(pDX, IDC_RADIO_TRIZETTO_UNSELECT, m_radioUnselect);
	DDX_Control(pDX, IDC_EDIT_TRIZETTO_SITE_ID, m_nxeditSiteID);
	DDX_Control(pDX, IDC_EDIT_TRIZETTO_PASSWORD, m_nxeditPassword);
	DDX_Control(pDX, IDC_RADIO_TRIZETTO_USE_GLOBAL_LOGIN, m_radioGlobalLogin);
	DDX_Control(pDX, IDC_RADIO_TRIZETTO_USE_INDIV_LOGIN, m_radioIndivLogin);
	DDX_Control(pDX, IDC_RADIO_TRIZETTO, m_radioTrizetto);
	DDX_Control(pDX, IDC_RADIO_ECP, m_radioECP);
}


BEGIN_MESSAGE_MAP(CEligibilityRealTimeSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_REALTIME_SENDS, OnCheckEnableRealtimeSends)
	ON_BN_CLICKED(IDC_RADIO_TRIZETTO_USE_GLOBAL_LOGIN, OnBnClickedRadioTrizettoLogin)
	ON_BN_CLICKED(IDC_RADIO_TRIZETTO_USE_INDIV_LOGIN, OnBnClickedRadioTrizettoLogin)
	ON_EN_CHANGE(IDC_EDIT_TRIZETTO_SITE_ID, OnEnChangeEditTrizettoSiteId)
	ON_EN_CHANGE(IDC_EDIT_TRIZETTO_PASSWORD, OnEnChangeEditTrizettoPassword)
	ON_BN_CLICKED(IDC_RADIO_TRIZETTO, &CEligibilityRealTimeSetupDlg::OnBnClickedRadioTrizetto)
	ON_BN_CLICKED(IDC_RADIO_ECP, &CEligibilityRealTimeSetupDlg::OnBnClickedRadioEcp)
END_MESSAGE_MAP()


// CEligibilityRealTimeSetupDlg message handlers

BOOL CEligibilityRealTimeSetupDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_ProvCombo = BindNxDataList2Ctrl(IDC_ELIG_REALTIME_PROV_COMBO, true);
		m_LocCombo = BindNxDataList2Ctrl(IDC_ELIG_REALTIME_LOC_COMBO, true);

		m_nxeditSiteID.SetLimitText(50);
		m_nxeditPassword.SetLimitText(50);

		g_propManager.CachePropertiesInBulk("CEligibilityRealTimeSetupDlg-Number", propNumber,
			"(Username = '<None>') AND ("
			"Name = 'GEDIEligibilityRealTime_Enabled' OR "
			"Name = 'GEDIEligibilityRealTime_OpenNotepad' OR "
			"Name = 'GEDIEligibilityRealTime_UnbatchOnReceive' OR "
			"Name = 'Clearinghouse_LoginType' OR "
			// (j.jones 2010-10-21 12:45) - PLID 40914 - added type of clearinghouse, 0 - Trizetto, 1 - ECP
			"Name = 'RealTimeEligibility_Clearinghouse' OR "
			"Name = 'EbillingClearinghouseIntegration_Enabled' "
			")");

		m_ProvCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		IRowSettingsPtr pProvRow = m_ProvCombo->GetFirstRow();
		if(pProvRow) {
			m_ProvCombo->PutCurSel(pProvRow);
		}

		m_LocCombo->SetSelByColumn(0, GetCurrentLocationID());
		m_bLoginInfoChanged = FALSE;

		Load();

	}NxCatchAll("Error in CEligibilityRealTimeSetupDlg::OnInitDialog()");

	return FALSE;
}

void CEligibilityRealTimeSetupDlg::OnOk()
{
	try {

		//save the settings
		if(!Save()) {
			return;
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRealTimeSetupDlg::OnCheckEnableRealtimeSends()
{
	try {

		BOOL bIsEnabled = m_checkEnableRealTime.GetCheck();

		GetDlgItem(IDC_CHECK_OPEN_NOTEPAD)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_RADIO_TRIZETTO_UNBATCH)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_RADIO_TRIZETTO_UNSELECT)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_EDIT_TRIZETTO_SITE_ID)->EnableWindow(bIsEnabled);
		GetDlgItem(IDC_EDIT_TRIZETTO_PASSWORD)->EnableWindow(bIsEnabled);
		// (j.jones 2010-10-21 13:08) - PLID 40914 - added clearinghouse selection
		m_radioTrizetto.EnableWindow(bIsEnabled);
		m_radioECP.EnableWindow(bIsEnabled);
		
	}NxCatchAll(__FUNCTION__);
}

NexTech_Accessor::_ClearinghouseLoginFilterPtr CEligibilityRealTimeSetupDlg::CreateLoginFilterFromControls()
{
	NexTech_Accessor::_ClearinghouseLoginFilterPtr pLoginFilter(__uuidof(NexTech_Accessor::ClearinghouseLoginFilter));

	if (m_radioECP.GetCheck() == BST_CHECKED)
	{
		pLoginFilter->_ClearinghouseType = NexTech_Accessor::ClearinghouseType_ECP;
	}
	else
	{
		pLoginFilter->_ClearinghouseType = NexTech_Accessor::ClearinghouseType_TriZetto;
	}

	if (UseGlobalLogin())
	{
		pLoginFilter->LoginType = NexTech_Accessor::ClearinghouseLoginType::ClearinghouseLoginType_Global;
	}
	else
	{
		pLoginFilter->LoginType = NexTech_Accessor::ClearinghouseLoginType::ClearinghouseLoginType_Individual;
	}
	
	NXDATALIST2Lib::IRowSettingsPtr pProviderRow = m_ProvCombo->CurSel;
	if (pProviderRow != nullptr && !UseGlobalLogin())
	{
		long nProviderID = VarLong(pProviderRow->GetValue(0));
		pLoginFilter->providerID = _bstr_t(FormatString("%li", nProviderID));
	}

	NXDATALIST2Lib::IRowSettingsPtr pLocationRow = m_LocCombo->CurSel;
	if (pLocationRow != nullptr && !UseGlobalLogin())
	{
		long nLocationID = VarLong(pLocationRow->GetValue(0));
		pLoginFilter->locationID = _bstr_t(FormatString("%li", nLocationID));
	}

	return pLoginFilter;
}

void CEligibilityRealTimeSetupDlg::OnBnClickedRadioTrizettoLogin()
{
	try {
		LoadLogin();
	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRealTimeSetupDlg::LoadLogin()
{
	m_pCurrentLogin = GetAPI()->GetClearinghouseLogin(GetAPISubkey(), GetAPILoginToken(), CreateLoginFilterFromControls());

	if (m_radioGlobalLogin.GetCheck())
	{
		//disable the provider/location
		m_ProvCombo->PutEnabled(g_cvarFalse);
		m_LocCombo->PutEnabled(g_cvarFalse);
	}
	else
	{
		//enable the provider/location
		m_ProvCombo->PutEnabled(g_cvarTrue);
		m_LocCombo->PutEnabled(g_cvarTrue);
	}

	m_nxeditSiteID.SetWindowText((LPCTSTR)m_pCurrentLogin->SiteID);
	m_nxeditPassword.SetWindowText((LPCTSTR)m_pCurrentLogin->password);
	m_bLoginInfoChanged = FALSE;
}

void CEligibilityRealTimeSetupDlg::Load()
{
	try {

		//load the settings
		m_checkEnableRealTime.SetCheck(GetRemotePropertyInt("GEDIEligibilityRealTime_Enabled", 0, 0, "<None>", true) == 1);
		m_checkOpenNotepad.SetCheck(GetRemotePropertyInt("GEDIEligibilityRealTime_OpenNotepad", 1, 0, "<None>", true) == 1);

		// (j.jones 2010-10-21 12:55) - PLID 40914 - load the clearinghouse type
		EligibilityRealTime_ClearingHouse ertcClearinghouse = (EligibilityRealTime_ClearingHouse)GetRemotePropertyInt("RealTimeEligibility_Clearinghouse", (long)ertcTrizetto, 0, "<None>", true);
		if(ertcClearinghouse == ertcECP) {
			m_radioECP.SetCheck(TRUE);
			m_radioTrizetto.SetCheck(FALSE);
		}
		else {
			m_radioTrizetto.SetCheck(TRUE);
			m_radioECP.SetCheck(FALSE);
		}

		long nRealTimeUnbatchOnReceive = GetRemotePropertyInt("GEDIEligibilityRealTime_UnbatchOnReceive", 1, 0, "<None>", true);
		//1 - unbatch, 0 - unselect
		if(nRealTimeUnbatchOnReceive == 1) {
			//unbatch
			m_radioUnbatch.SetCheck(TRUE);
			m_radioUnselect.SetCheck(FALSE);
		}
		else {
			//unselect
			m_radioUnbatch.SetCheck(FALSE);
			m_radioUnselect.SetCheck(TRUE);
		}

		NexTech_Accessor::ClearinghouseLoginType eLoginType = (NexTech_Accessor::ClearinghouseLoginType)GetRemotePropertyInt(
			"Clearinghouse_LoginType", (long)NexTech_Accessor::ClearinghouseLoginType_Global, (long)ertcClearinghouse, "<None>", true);
		if(eLoginType == NexTech_Accessor::ClearinghouseLoginType_Individual) {
			//indiv.
			m_radioGlobalLogin.SetCheck(FALSE);
			m_radioIndivLogin.SetCheck(TRUE);
		}
		else {
			//global
			m_radioGlobalLogin.SetCheck(TRUE);
			m_radioIndivLogin.SetCheck(FALSE);
		}

		LoadLogin();

		//enable/disable the screen
		OnCheckEnableRealtimeSends();

	}NxCatchAll(__FUNCTION__);
}

bool CEligibilityRealTimeSetupDlg::UseGlobalLogin()
{
	return m_radioGlobalLogin.GetCheck() == BST_CHECKED;
}

bool CEligibilityRealTimeSetupDlg::ValidateBeforeSaving()
{
	if (m_pCurrentLogin->LoginType == NexTech_Accessor::ClearinghouseLoginType_Individual)
	{
		if (m_pCurrentLogin->providerID.length() == 0)
		{
			MessageBox("The Provider may not be empty. Please correct this and try saving again.", "Error", MB_ICONERROR | MB_OK);
			return false;
		}
		if (m_pCurrentLogin->locationID.length() == 0)
		{
			MessageBox("The Location may not be empty. Please correct this and try saving again.", "Error", MB_ICONERROR | MB_OK);
			return false;
		}
	}

	if (m_pCurrentLogin->SiteID.length() == 0 && m_pCurrentLogin->password.length() > 0)
	{
		MessageBox("The Login / Site ID may not be empty. Please correct this and try saving again.", "Error", MB_ICONERROR | MB_OK);
		return false;
	}
	else if (m_pCurrentLogin->SiteID.length() > 0 && m_pCurrentLogin->password.length() == 0)
	{
		MessageBox("The Password may not be empty. Please correct this and try saving again.", "Error", MB_ICONERROR | MB_OK);
		return false;
	}

	return true;
}

BOOL CEligibilityRealTimeSetupDlg::Save()
{
	try {

		CWaitCursor wc;

		// Validate the current settings before attempting to save to the database.
		if (!ValidateBeforeSaving())
		{
			return FALSE;
		}

		BOOL bEnabled = m_checkEnableRealTime.GetCheck();
		SetRemotePropertyInt("GEDIEligibilityRealTime_Enabled", bEnabled ? 1 : 0, 0, "<None>");
		if (!bEnabled)
		{
			// If the real time integration is disabled, then these settings will be disabled anyway. Discard any modifications to them.
			// The enabled state takes prescedence.
			return TRUE;
		}

		if (!SaveLogin()) {
			return FALSE;
		}
		SaveLoginType();

		BOOL bOpenNotepad = m_checkOpenNotepad.GetCheck();
		SetRemotePropertyInt("GEDIEligibilityRealTime_OpenNotepad", bOpenNotepad ? 1 : 0, 0, "<None>");

		// (j.jones 2010-10-21 12:55) - PLID 40914 - save the clearinghouse type
		EligibilityRealTime_ClearingHouse ertcClearinghouse = ertcTrizetto;
		if(m_radioECP.GetCheck()) {
			ertcClearinghouse = ertcECP;
		}
		SetRemotePropertyInt("RealTimeEligibility_Clearinghouse", (long)ertcClearinghouse, 0, "<None>");

		long nRealTimeUnbatchOnReceive = 0;		
		if(m_radioUnbatch.GetCheck()) {
			nRealTimeUnbatchOnReceive = 1;
		}

		SetRemotePropertyInt("GEDIEligibilityRealTime_UnbatchOnReceive", nRealTimeUnbatchOnReceive, 0, "<None>");

		ValidateAfterSaving();

		return TRUE;

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

void CEligibilityRealTimeSetupDlg::ValidateAfterSaving()
{
	if (m_pCurrentLogin->SiteID.length() == 0)
	{
		// Logins with empty site IDs won't be sent anyway. Early out.
		return;
	}

	if (m_pCurrentLogin->LoginType == NexTech_Accessor::ClearinghouseLoginType_Individual)
	{
		NexTech_Accessor::_ClearinghouseLoginListPtr pMismatchingLogins =
			GetAPI()->GetMismatchingClearinghouseLogins(GetAPISubkey(), GetAPILoginToken(), m_pCurrentLogin);
		Nx::SafeArray<IUnknown *> saryMismatchingLogins(pMismatchingLogins->Logins);

		if (saryMismatchingLogins.GetSize() > 0)
		{
			CString strMismatchingLoginsMessage;
			if (saryMismatchingLogins.GetSize() == 1)
			{
				strMismatchingLoginsMessage = "There is one other clearinghouse login with the same "
					"Site ID as this one but a different password. "
					"Nextech will now update the password of this other login.";
			}
			else
			{
				strMismatchingLoginsMessage = FormatString("There are %li other clearinghouse logins with the same "
					"Site ID as this one but different passwords. "
					"Nextech will now update the password of these other logins.",
					saryMismatchingLogins.GetSize());
			}

			MessageBox(strMismatchingLoginsMessage, "Mismatched Logins", MB_ICONINFORMATION | MB_OK);
			// Fix the logins we know now are invalid. Do this unconditionally to keep data consistent
			// and to avoid a support case later due to mismatching logins.
			for (NexTech_Accessor::_ClearinghouseLoginPtr pOtherLogin : saryMismatchingLogins)
			{
				pOtherLogin->password = m_pCurrentLogin->password;
				pOtherLogin->PortalPassword = m_pCurrentLogin->PortalPassword;
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

void CEligibilityRealTimeSetupDlg::SaveLoginType()
{
	NexTech_Accessor::ClearinghouseType eClearinghouseType;
	if (m_radioECP.GetCheck() == BST_CHECKED)
	{
		eClearinghouseType = NexTech_Accessor::ClearinghouseType_ECP;
	}
	else
	{
		eClearinghouseType = NexTech_Accessor::ClearinghouseType_TriZetto;
	}

	NexTech_Accessor::ClearinghouseLoginType apiClearinghouseLoginType;
	if (UseGlobalLogin())
	{
		apiClearinghouseLoginType = NexTech_Accessor::ClearinghouseLoginType::ClearinghouseLoginType_Global;
	}
	else
	{
		apiClearinghouseLoginType = NexTech_Accessor::ClearinghouseLoginType::ClearinghouseLoginType_Individual;
	}

	GetAPI()->SetClearinghouseLoginType(GetAPISubkey(), GetAPILoginToken(), eClearinghouseType, apiClearinghouseLoginType);
}

BOOL CEligibilityRealTimeSetupDlg::SaveLogin()
{
	// Validate the login only with TriZetto if 1) TriZetto is the clearinghouse, 2) the direct export of CLAIM FILES
	// to TriZetto is enabled. Even though this is the eligibility dialog, the logins are shared and we perform login validation
	// over in EBilling's Clearinghouse Integration when its on. Why don't we validate eligibility logins though? Because
	// of existing behavior.
	BOOL bEbillingClearinghouseIntegrationEnabled = (BOOL)GetRemotePropertyInt("EbillingClearinghouseIntegration_Enabled", FALSE, 0, "<None>", false);
	VARIANT_BOOL vbSendToClaimService = m_radioTrizetto.GetCheck() == BST_CHECKED && bEbillingClearinghouseIntegrationEnabled ? VARIANT_TRUE : VARIANT_FALSE;

	NexTech_Accessor::_SaveClearinghouseLoginResultPtr pResult =
		GetAPI()->SaveClearinghouseLogin(GetAPISubkey(), GetAPILoginToken(), m_pCurrentLogin, vbSendToClaimService);

	if (pResult->Success)
	{
		m_bLoginInfoChanged = FALSE;
		return TRUE;
	}
	{
		CString strMessage = FormatString("An error occurred communicating with "
			"the clearinghouse server.\r\n\r\nDetails:\r\n%s", (LPCTSTR)pResult->RemoteErrorMessage);
		MessageBox(strMessage, "Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}
}

BEGIN_EVENTSINK_MAP(CEligibilityRealTimeSetupDlg, CNxDialog)
ON_EVENT(CEligibilityRealTimeSetupDlg, IDC_ELIG_REALTIME_PROV_COMBO, 1, OnSelChangingEligRealtimeProvCombo, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CEligibilityRealTimeSetupDlg, IDC_ELIG_REALTIME_LOC_COMBO, 1, OnSelChangingEligRealtimeLocCombo, VTS_DISPATCH VTS_PDISPATCH)
ON_EVENT(CEligibilityRealTimeSetupDlg, IDC_ELIG_REALTIME_PROV_COMBO, 16, CEligibilityRealTimeSetupDlg::OnSelChosenEligRealtimeProvCombo, VTS_DISPATCH)
ON_EVENT(CEligibilityRealTimeSetupDlg, IDC_ELIG_REALTIME_LOC_COMBO, 16, CEligibilityRealTimeSetupDlg::OnSelChosenEligRealtimeLocCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CEligibilityRealTimeSetupDlg::OnSelChangingEligRealtimeProvCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRealTimeSetupDlg::OnSelChangingEligRealtimeLocCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRealTimeSetupDlg::OnSelChosenEligRealtimeProvCombo(LPDISPATCH lpRow)
{
	try {
		long nCurProviderID = m_pCurrentLogin->providerID.length() != 0 ? atol(m_pCurrentLogin->providerID) : -1;
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_ProvCombo->SetSelByColumn(0, nCurProviderID);
			return;
		}

		long nProviderID = VarLong(pRow->GetValue(0));

		if(nProviderID != nCurProviderID && m_bLoginInfoChanged) {

			if(IDNO == MessageBox("Do you wish to save the current login information for the previous provider?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				m_ProvCombo->SetSelByColumn(0, nCurProviderID);
				return;
			}

			//just save everything
			if(!Save()) {
				//something failed
				m_ProvCombo->SetSelByColumn(0, nCurProviderID);
				return;
			}
		}

		LoadLogin();

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRealTimeSetupDlg::OnSelChosenEligRealtimeLocCombo(LPDISPATCH lpRow)
{
	try {
		long nCurLocationID = m_pCurrentLogin->locationID.length() != 0 ? atol(m_pCurrentLogin->locationID) : -1;
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_LocCombo->SetSelByColumn(0, nCurLocationID);
			return;
		}

		long nLocationID = VarLong(pRow->GetValue(0));

		if(nLocationID != nCurLocationID && m_bLoginInfoChanged) {

			if(IDNO == MessageBox("Do you wish to save the current login information for the previous location?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				m_LocCombo->SetSelByColumn(0, nCurLocationID);
				return;
			}

			//just save everything
			if(!Save()) {
				//something failed
				m_LocCombo->SetSelByColumn(0, nCurLocationID);
				return;
			}
		}

		LoadLogin();

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRealTimeSetupDlg::OnEnChangeEditTrizettoSiteId()
{
	try
	{
		CString strSiteID;
		m_nxeditSiteID.GetWindowText(strSiteID);
		m_pCurrentLogin->SiteID = _bstr_t(strSiteID);
		m_bLoginInfoChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CEligibilityRealTimeSetupDlg::OnEnChangeEditTrizettoPassword()
{
	try {
		CString strPassword;
		m_nxeditPassword.GetWindowText(strPassword);
		m_pCurrentLogin->password = _bstr_t(strPassword);
		m_bLoginInfoChanged = TRUE;

	}NxCatchAll(__FUNCTION__);
}


void CEligibilityRealTimeSetupDlg::OnBnClickedRadioTrizetto()
{
	try
	{
		LoadLogin();
	}
	NxCatchAll(__FUNCTION__);
}


void CEligibilityRealTimeSetupDlg::OnBnClickedRadioEcp()
{
	try
	{
		LoadLogin();
	}
	NxCatchAll(__FUNCTION__);
}

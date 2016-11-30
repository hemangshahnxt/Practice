// SureScriptsSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "SureScriptsSettingsDlg.h"
#include "MultiSelectDlg.h"

//TES 4/2/2009 - PLID 33376 - Created
// CSureScriptsSettingsDlg dialog

CSureScriptsSettingsDlg::CSureScriptsSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSureScriptsSettingsDlg::IDD, pParent)
{
	m_bNotifyProvidersChanged = false;
}

CSureScriptsSettingsDlg::~CSureScriptsSettingsDlg()
{
}

void CSureScriptsSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ENABLE_SURESCRIPTS, m_checkEnableSureScripts);
	DDX_Control(pDX, IDC_SURESCRIPTS_CLIENTID, m_nxeClientID);
	DDX_Control(pDX, IDC_SURESCRIPTS_AUTHKEY, m_nxeAuthKey);
	DDX_Control(pDX, IDC_SURESCRIPTS_URL, m_nxeURL);
	DDX_Control(pDX, IDC_SURESCRIPTS_NOTIFY, m_checkNotify);
	DDX_Control(pDX, IDC_SELECT_PROVIDERS, m_btnSelectProviders);
	DDX_Control(pDX, IDC_COMMUNICATION_SETTINGS_GROUP, m_grpCommSettings);
}


BEGIN_MESSAGE_MAP(CSureScriptsSettingsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ENABLE_SURESCRIPTS, OnEnableSureScripts)
	ON_BN_CLICKED(IDC_SURESCRIPTS_NOTIFY, OnSureScriptsNotify)
	ON_BN_CLICKED(IDC_SELECT_PROVIDERS, OnSelectProviders)
END_MESSAGE_MAP()


// CSureScriptsSettingsDlg message handlers
BOOL CSureScriptsSettingsDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//TES 4/2/2009 - PLID 33376 - Load our settings.
		//TES 4/3/2009 - PLID 33376 - Track all the settings so we can tell at the end whether anything changed.
		m_bOldSureScriptsEnabled = GetRemotePropertyInt("SureScriptsEnabled", 0, 0, "<None>") == 0 ? FALSE : TRUE;
		CheckDlgButton(IDC_ENABLE_SURESCRIPTS, m_bOldSureScriptsEnabled?BST_CHECKED:BST_UNCHECKED);
		OnEnableSureScripts();
		m_strOldURL = GetRemotePropertyText("SureScriptsURL", "https://hub.nextech.com/ss/ClientService.asmx", 0, "<None>");
		SetDlgItemText(IDC_SURESCRIPTS_URL, m_strOldURL);
		m_strOldClientID = GetRemotePropertyText("SureScriptsClientID", "", 0, "<None>");
		SetDlgItemText(IDC_SURESCRIPTS_CLIENTID, m_strOldClientID);
		m_strOldAuthKey = GetRemotePropertyText("SureScriptsAuthKey", "", 0, "<None>");
		SetDlgItemText(IDC_SURESCRIPTS_AUTHKEY, m_strOldAuthKey);

		//TES 4/10/2009 - PLID 33889 - Load our notification settings
		m_bOldNotify = GetRemotePropertyInt("SureScriptsNotify", 1, 0, GetCurrentUserName()) == 0 ? FALSE : TRUE;
		CheckDlgButton(IDC_SURESCRIPTS_NOTIFY, m_bOldNotify?BST_CHECKED:BST_UNCHECKED);
		OnSureScriptsNotify();

		GetRemotePropertyArray("SureScriptsNotifyProviders", m_arNotifyProviders, 0, GetCurrentUserName());

		//TES 7/6/2009 - PLID 34087 - If the current user is not an Admin, only let them change the user-specific settings.
		if(!IsCurrentUserAdministrator()) {
			m_checkEnableSureScripts.EnableWindow(FALSE);
			m_nxeClientID.EnableWindow(FALSE);
			m_nxeAuthKey.EnableWindow(FALSE);
			m_nxeURL.EnableWindow(FALSE);
		}

	}NxCatchAll("Error in CSureScriptsSettingsDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CSureScriptsSettingsDlg, CNxDialog)
END_EVENTSINK_MAP()

void CSureScriptsSettingsDlg::OnEnableSureScripts()
{
	try {
		//TES 4/2/2009 - PLID 33376 - Disable all the settings if integration isn't enabled.
		BOOL bEnable = IsDlgButtonChecked(IDC_ENABLE_SURESCRIPTS);
		m_nxeClientID.EnableWindow(bEnable);
		m_nxeAuthKey.EnableWindow(bEnable);
		m_nxeURL.EnableWindow(bEnable);
		m_checkNotify.EnableWindow(bEnable);
		//TES 4/10/2009 - PLID 33889 - Don't enable the Select Providers button if the Notify button isn't checked.
		if(bEnable && IsDlgButtonChecked(IDC_SURESCRIPTS_NOTIFY)) {
			m_btnSelectProviders.EnableWindow(TRUE);
		}
		else {
			m_btnSelectProviders.EnableWindow(FALSE);
		}
	}NxCatchAll("Error in CSureScriptsSettingsDlg::OnEnableSureScripts()");
}

void CSureScriptsSettingsDlg::OnOK()
{
	try {
		//TES 4/2/2009 - PLID 33376 - Save our settings
		BOOL bSureScriptsEnabled = IsDlgButtonChecked(IDC_ENABLE_SURESCRIPTS);
		SetRemotePropertyInt("SureScriptsEnabled", bSureScriptsEnabled?1:0, 0, "<None>");
		CString strURL;
		GetDlgItemText(IDC_SURESCRIPTS_URL, strURL);
		SetRemotePropertyText("SureScriptsURL", strURL, 0, "<None>");
		CString strClientID;
		GetDlgItemText(IDC_SURESCRIPTS_CLIENTID, strClientID);
		SetRemotePropertyText("SureScriptsClientID", strClientID, 0, "<None>");
		CString strAuthKey;
		GetDlgItemText(IDC_SURESCRIPTS_AUTHKEY, strAuthKey);
		SetRemotePropertyText("SureScriptsAuthKey", strAuthKey, 0, "<None>");

		//TES 4/10/2009 - PLID 33889 - Do we need to update NxServer?
		if(bSureScriptsEnabled != m_bOldSureScriptsEnabled || 
			strURL != m_strOldURL || 
			strClientID != m_strOldClientID ||
			strAuthKey != m_strOldAuthKey) {
				//TES 4/3/2009 - PLID 33376 - A setting changed, notify NxServer.
				CClient::Send(PACKET_TYPE_SURESCRIPTS_SETTINGS_CHANGED, NULL, 0);
		}

		//TES 4/10/2009 - PLID 33889 - Save our notification settings.
		BOOL bNotify = IsDlgButtonChecked(IDC_SURESCRIPTS_NOTIFY);
		SetRemotePropertyInt("SureScriptsNotify", bNotify?1:0, 0, GetCurrentUserName());
		SetRemotePropertyArray("SureScriptsNotifyProviders", m_arNotifyProviders, 0, GetCurrentUserName());

		//TES 4/10/2009 - PLID 33889 - Do we need to refresh our notifications?
		if(bSureScriptsEnabled != m_bOldSureScriptsEnabled ||
			bNotify != m_bOldNotify || 
			m_bNotifyProvidersChanged) {
			//GetMainFrame()->RefreshSureScriptsMessagesNeedingAttention();
		}

		CNxDialog::OnOK();
	}NxCatchAll("Error in CSureScriptsSettingsDlg::OnOK()");
}

void CSureScriptsSettingsDlg::OnSureScriptsNotify()
{
	try {
		//TES 4/10/2009 - PLID 33889 - Update the Select Providers button appropriately.
		m_btnSelectProviders.EnableWindow(IsDlgButtonChecked(IDC_SURESCRIPTS_NOTIFY) && IsDlgButtonChecked(IDC_ENABLE_SURESCRIPTS));
	} NxCatchAll("Error in CSureScriptsSettingsDlg::OnSureScriptsNotify()");
}

void CSureScriptsSettingsDlg::OnSelectProviders()
{
	try {
		//TES 4/10/2009 - PLID 33889 - Give them a dialog to choose.
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ProvidersT");
		dlg.PreSelect(m_arNotifyProviders);
		//TES 5/1/2009 - PLID 34141 - Only let them choose providers that they've already selected, or that have SPIs.
		CString strWhere;
		strWhere.Format("COALESCE(SPI,'') <> '' OR ID IN (%s)", ArrayAsString(m_arNotifyProviders, false));
		if(IDOK == dlg.Open("PersonT INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID", strWhere,
			"PersonT.ID", "Last + ', ' + First + ' ' + Middle", 
			"Please select the providers for which you would like to be notified, when they have "
			"SureScripts messages requiring attention (you may leave all boxes unchecked to be "
			"notified for all providers).")) {
				//TES 4/10/2009 - PLID 33889 - Now we need to determine whether or not the list changed, so that, 
				// if and when we save the change, we know whether to refresh our notifications.
				CArray<long,long> arNewProviders;
				dlg.FillArrayWithIDs(arNewProviders);
				if(arNewProviders.GetSize() != m_arNotifyProviders.GetSize()) {
					m_bNotifyProvidersChanged = true;
				}
				else {
					for(int i = 0; i < arNewProviders.GetSize() && !m_bNotifyProvidersChanged; i++) {
						bool bFound = false;
						for(int j = 0; j < m_arNotifyProviders.GetSize() && !bFound; j++) {
							if(arNewProviders[i] == m_arNotifyProviders[j]) {
								bFound = true;
							}
						}
						if(!bFound) m_bNotifyProvidersChanged = true;
					}
				}

				//TES 4/10/2009 - PLID 33889 - Now store the list.
				m_arNotifyProviders.RemoveAll();
				for(int i = 0; i < arNewProviders.GetSize(); i++) {
					m_arNotifyProviders.Add(arNewProviders[i]);
				}
		}

	}NxCatchAll("Error in CSureScriptsSettingsDlg::OnSelectProviders()");
}

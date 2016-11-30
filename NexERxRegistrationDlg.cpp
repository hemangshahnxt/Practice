// NexERxRegistrationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NexERxRegistrationDlg.h"

// CNexERxRegistrationDlg dialog
// (b.savon 2012-12-03 15:51) - PLID 53559 - Created

IMPLEMENT_DYNAMIC(CNexERxRegistrationDlg, CNxDialog)

CNexERxRegistrationDlg::CNexERxRegistrationDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexERxRegistrationDlg::IDD, pParent)
{

}

BOOL CNexERxRegistrationDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{

		SetTitleBarIcon(IDI_ERX);

		m_nxcBackground.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_btnSync.AutoSet(NXB_REFRESH);
		m_btnClose.AutoSet(NXB_CLOSE);

		// (b.savon 2013-05-21 09:06) - PLID 56678
		CacheProperties();
		CString strRegistrationURL = GetRegistrationWebServiceURL();

		CWnd* pBrowserWnd = GetDlgItem(IDC_NEXERX_REGISTRATION_BROWSER);
		if(pBrowserWnd)
		{
			m_pBrowser = pBrowserWnd->GetControlUnknown();
			if(!m_pBrowser)
			{
				return FALSE;
			}

			m_pBrowser->Navigate(_bstr_t(strRegistrationURL), NULL, NULL, NULL, NULL);
		}

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;
}

// (b.savon 2013-05-14 10:03) - PLID 56678
void CNexERxRegistrationDlg::CacheProperties()
{
	try{
		g_propManager.CachePropertiesInBulk("NexERxRegistrationDlgText", propText,
		"(Username = '<None>' OR Username = '%s') AND ("
		" Name = 'NexERxProductionWebServiceURL' OR "
		" Name = 'NexERxPreProductionWebServiceURL' "
		")",
		_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("NexERxRegistrationDlgNumber", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		" Name = 'NexERxStatus' "
		")",
		_Q(GetCurrentUserName()));
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-05-14 10:03) - PLID 56678
CString CNexERxRegistrationDlg::GetRegistrationWebServiceURL()
{
	//Pass in the license key and go
	CString strLicenseKey;
	strLicenseKey.Format("%li", g_pLicense->GetLicenseKey());

	if( GetRemotePropertyInt("NexERxStatus", 0, 0, "<None>", true) == 1 )
		return GetProductionURL() + strLicenseKey;
	else
		return GetPreProductionURL() + strLicenseKey;
}

// (b.savon 2013-05-14 10:03) - PLID 56678
CString CNexERxRegistrationDlg::GetProductionURL()
{
	return GetPrescriberRegistrationURL(GetRemotePropertyText("NexERxProductionWebServiceURL", "about:blank", 0, "<None>", false)) + "?lk=";
}

// (b.savon 2013-05-14 10:03) - PLID 56678
CString CNexERxRegistrationDlg::GetPreProductionURL()
{
	return GetPrescriberRegistrationURL(GetRemotePropertyText("NexERxPreProductionWebServiceURL", "about:blank", 0, "<None>", false)) + "?lk=";
}

// (b.savon 2013-05-14 10:03) - PLID 56678
CString CNexERxRegistrationDlg::GetPrescriberRegistrationURL(CString strWebServiceURL)
{
	strWebServiceURL.Replace("client/nexerx.asmx", "AdminSite/Prescribers.aspx");
	return strWebServiceURL;
}

CNexERxRegistrationDlg::~CNexERxRegistrationDlg()
{
}

void CNexERxRegistrationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);	
	DDX_Control(pDX, IDC_BTN_SYNC_NEXERX_PRESCRIBERS, m_btnSync);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_NXC__NEXERX_REG_BACKGROUND, m_nxcBackground);
}

BEGIN_MESSAGE_MAP(CNexERxRegistrationDlg, CNxDialog)

	ON_BN_CLICKED(IDC_BTN_SYNC_NEXERX_PRESCRIBERS, &CNexERxRegistrationDlg::OnBnClickedBtnSyncNexerxPrescribers)
END_MESSAGE_MAP()


// CNexERxRegistrationDlg message handlers

void CNexERxRegistrationDlg::OnBnClickedBtnSyncNexerxPrescribers()
{
	try{
		CWaitCursor cwait;

		CString strLicenseKey;
		strLicenseKey.Format("%li", g_pLicense->GetLicenseKey());
		NexTech_Accessor::_SyncClientSettingsAndPrescribersResultPtr syncResults = GetAPI()->SyncRegisteredPrescribers(GetAPISubkey(), GetAPILoginToken(), _bstr_t(strLicenseKey));

		if( syncResults && syncResults->SyncStatus ){
			MessageBox(GetSyncResultMessage(syncResults->SyncStatus), "NexERx Registration Results", MB_ICONINFORMATION);
		}else{
			MessageBox("Unknown Error!", "NexERx Registration Results", MB_ICONERROR);
		}

	}NxCatchAll(__FUNCTION__);
}

LPCTSTR CNexERxRegistrationDlg::GetSyncResultMessage(NexTech_Accessor::SyncClientSettingsAndPrescriberStatus syncStatus)
{
	switch(syncStatus)
	{
		case NexTech_Accessor::SyncClientSettingsAndPrescriberStatus_UnknownFailure:
			return (LPCTSTR)"Unknown Sync Failure!";
		case NexTech_Accessor::SyncClientSettingsAndPrescriberStatus_UndeterminedAuthenticationKey:
			return (LPCTSTR)"Unable to determine authentication key for client!";
		case NexTech_Accessor::SyncClientSettingsAndPrescriberStatus_UnableToSyncClientSettings:
			return (LPCTSTR)"Unable to sync client settings!";
		case NexTech_Accessor::SyncClientSettingsAndPrescriberStatus_Success:
			return (LPCTSTR)"Synced registered prescribers successfully!";
		case NexTech_Accessor::SyncClientSettingsAndPrescriberStatus_NoPrescribersSynced:
			return (LPCTSTR)"No prescribers synced.";
		case NexTech_Accessor::SyncClientSettingsAndPrescriberStatus_NotLicensedForNexERx:
			return (LPCTSTR)"Not licensed for NexERx";
		default:
			//This shouldn't happen.  If it does, handle message for newly added status added in API.
			ASSERT(FALSE);
			return (LPCTSTR)"Unknown result received";
	}
}

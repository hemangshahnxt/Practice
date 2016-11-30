// SupportDlg.cpp : implementation file
//
// (r.farnworth 2013-07-25 13:08) - PLID 57677 - Removed all code related to the dialogs IDC_RATING and IDC_RATED_DATE

#include "stdafx.h"
#include "SupportDlg.h"
#include "GlobalDataUtils.h"
#include "AuditTrail.h"
#include "AuditSupportDlg.h"
#include "ViewSupportHistoryDlg.h"
#include "InternationalUtils.h"
#include "NexWebLoginInfoDlg.h"
#include "NewVersionDlg.h"
#include "SupportThirdPartyInfoDlg.h"
#include "SupportConnectionDlg.h"
#include "PatientsRc.h"
#include "SupportUpgradeHistoryDlg.h"
#include "IntegrationDlg.h"
// (f.dinatale 2010-10-25) - PLID 40827
#include "NxReminderSettingsDlg.h"
#include "NxReminderSOAPUtils.h"
#include "ConcTsMachinePoolDlg.h"
// (d.singleton 2012-01-10 08:56) - PLID 28219 - edit licenses now has a user friendly dialog
#include "SupportEditLicenseDlg.h"
#include "NxAPI.h"
#include <NxPracticeSharedLib\NexWebAdminServiceAccessor.h>
#include "AnchorChangePasswordDlg.h"
#include "NexwebLoginGeneratePasswordDlg.h"
#include "AnalyticsLicensingDlg.h"
#include "DictationEditInfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NXTIMELib;
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

/////////////////////////////////////////////////////////////////////////////
// CSupportDlg dialog
#define ASPS_FROM_ASPS		1
#define ASPS_FROM_NEXTECH	2
#define AAO_FROM_AAO		1
#define AAO_FROM_NEXTECH	2

// (b.eyers 2015-03-05) - PLID 65097
#define LOCKOUT_DAYS	7

//DRT 5/11/2004 - Change of structure -- instead of a giant list of checkboxes that is hard to maintain
//	and quite restricted by spatial concerns, we're going to put it all in a datalist so it can be scrolled
//	and expanded much easier.
enum SupportItemList {
	slcField = 0,
	slcModule,
	slcStatus,
	slcUsageCount,
	slcExpiration,
};

CSupportDlg::CSupportDlg(CWnd* pParent)
	: CPatientDialog(CSupportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSupportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_bItemsEnabled = FALSE;
	m_bHideForNewClient = FALSE;
}

void CSupportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMRPRO_DR_NAMES, m_nxeditEmrProDoctorNames);
	DDX_Control(pDX, IDC_EMRPRO, m_nxeditEmrPro);
	DDX_Control(pDX, IDC_ALLOW_SUPPORT_CHECK, m_btnAllowSupport);
	DDX_Control(pDX, IDC_IS_LICENSE_ACTIVE, m_btnLicenseActive);
	DDX_Control(pDX, IDC_B_LIC, m_nxeditBLic);
	DDX_Control(pDX, IDC_U_LIC, m_nxeditULic);
	DDX_Control(pDX, IDC_P_LIC, m_nxeditPLic);
	DDX_Control(pDX, IDC_B_CONC_TS_LIC, m_nxeditBConcTSLic);
	DDX_Control(pDX, IDC_U_CONC_TS_LIC, m_nxeditUConcTSLic);
	DDX_Control(pDX, IDC_P_CONC_TS_LIC, m_nxeditPConcTSLic);
	DDX_Control(pDX, IDC_B_DOC, m_nxeditBDoc);
	DDX_Control(pDX, IDC_U_DOC, m_nxeditUDoc);
	DDX_Control(pDX, IDC_P_DOC, m_nxeditPDoc);
	DDX_Control(pDX, IDC_B_PALM, m_nxeditBPalm);
	DDX_Control(pDX, IDC_U_PALM, m_nxeditUPalm);
	DDX_Control(pDX, IDC_P_PALM, m_nxeditPPalm);
	DDX_Control(pDX, IDC_B_PPC, m_nxeditBPpc);
	DDX_Control(pDX, IDC_U_PPC, m_nxeditUPpc);
	DDX_Control(pDX, IDC_P_PPC, m_nxeditPPpc);
	DDX_Control(pDX, IDC_B_NEXSYNC, m_nxeditBNexSync);
	DDX_Control(pDX, IDC_U_NEXSYNC, m_nxeditUNexSync);
	DDX_Control(pDX, IDC_P_NEXSYNC, m_nxeditPNexSync);
	DDX_Control(pDX, IDC_B_DATABASES, m_nxeditBDatabases);
	DDX_Control(pDX, IDC_U_DATABASES, m_nxeditUDatabases);
	DDX_Control(pDX, IDC_P_DATABASES, m_nxeditPDatabases);
	DDX_Control(pDX, IDC_B_LICPRES, m_nxeditBLicPres); 	//(r.farnworth 2013-07-19) PLID 57522
	DDX_Control(pDX, IDC_U_LICPRES, m_nxeditULicPres); 	//(r.farnworth 2013-07-19) PLID 57522
	DDX_Control(pDX, IDC_P_LICPRES, m_nxeditPLicPres); 	//(r.farnworth 2013-07-19) PLID 57522
	DDX_Control(pDX, IDC_B_MIDPRES, m_nxeditBMidPres); 	//(r.farnworth 2013-07-19) PLID 57522
	DDX_Control(pDX, IDC_U_MIDPRES, m_nxeditUMidPres); 	//(r.farnworth 2013-07-19) PLID 57522
	DDX_Control(pDX, IDC_P_MIDPRES, m_nxeditPMidPres); 	//(r.farnworth 2013-07-19) PLID 57522
	DDX_Control(pDX, IDC_B_STAFFPRES, m_nxeditBStaffPres); 	//(r.farnworth 2013-07-19) PLID 57522
	DDX_Control(pDX, IDC_U_STAFFPRES, m_nxeditUStaffPres); 	//(r.farnworth 2013-07-19) PLID 57522
	DDX_Control(pDX, IDC_P_STAFFPRES, m_nxeditPStaffPres); 	//(r.farnworth 2013-07-19) PLID 57522
	DDX_Control(pDX, IDC_B_EMRPROV, m_nxeditBEmrprov);
	DDX_Control(pDX, IDC_B_NEXWEB_DOMAINS, m_nxeditBNexWebDomains);
	DDX_Control(pDX, IDC_U_EMRPROV, m_nxeditUEmrprov);
	DDX_Control(pDX, IDC_U_NEXWEB_DOMAINS, m_nxeditUNexWebDomains);
	DDX_Control(pDX, IDC_P_EMRPROV, m_nxeditPEmrprov);
	DDX_Control(pDX, IDC_B_DICTATION, m_nxeditBDictation);
	DDX_Control(pDX, IDC_U_DICTATION, m_nxeditUDictation);
	DDX_Control(pDX, IDC_P_DICTATION, m_nxeditPDictation);
	DDX_Control(pDX, IDC_P_NEXWEB_DOMAINS, m_nxeditPNexWebDomains);
	DDX_Control(pDX, IDC_MAX_PATIENT_COUNT, m_nxeditMaxPatientCount);
	DDX_Control(pDX, IDC_MAX_PRACTICE_COUNT, m_nxeditMaxPracticeCount);
	DDX_Control(pDX, IDC_AMA_VERSION, m_nxeditAmaVersion);
	DDX_Control(pDX, IDC_PURCH_AGR, m_nxeditPurchAgr);
	DDX_Control(pDX, IDC_LIC_AGR, m_nxeditLicAgr);
	DDX_Control(pDX, IDC_INSTALLED, m_nxeditInstalled);
	DDX_Control(pDX, IDC_TRAINER, m_nxeditTrainer);
	DDX_Control(pDX, IDC_SUPP_EXP, m_nxeditSuppExp);
	DDX_Control(pDX, IDC_PRAC_EMAIL, m_nxeditPracEmail);
	DDX_Control(pDX, IDC_LICENSE_KEY, m_nxeditLicenseKey);
	DDX_Control(pDX, IDC_LICENSE_ACTIVATED_BY, m_nxeditLicenseActivatedBy);
	DDX_Control(pDX, IDC_SUPPORT_NOTES, m_nxeditSupportNotes);
	DDX_Control(pDX, IDC_SERV_NAME, m_nxeditServName);
	DDX_Control(pDX, IDC_SERV_OS, m_nxeditServOs);
	DDX_Control(pDX, IDC_WS_OS, m_nxeditWsOs);
	DDX_Control(pDX, IDC_PATH_SERVER, m_nxeditPathServer);
	DDX_Control(pDX, IDC_NXSERVER_IP, m_nxeditNxserverIp);
	DDX_Control(pDX, IDC_CONNECTION_INFO, m_btnConnectionInfo);
	DDX_Control(pDX, IDC_THIRD_PARTY_INFO, m_btnThirdPartInfo);
	DDX_Control(pDX, IDC_EDIT_WEB_LOGIN, m_btnEditWebLogin);
	// (f.dinatale 2010-05-24) - PLID 38842 - Added version history.
	DDX_Control(pDX, IDC_UPDATED_BY, m_nxeditUpdatedby);
	DDX_Control(pDX, IDC_UPDATED_ON, m_nxeditUpdatedon);
	DDX_Control(pDX, IDC_VER_HISTORY, m_btnUpgradeHistory);
	// (f.dinatale 2010-07-07) - PLID 39527 - Added Lab Integration information.
	DDX_Control(pDX, IDC_BTNINTEGRATION, m_btnLabIntegrationInfo);
	// (f.dinatale 2010-10-25) - PLID 40827 - Added NxReminder settings.
	DDX_Control(pDX, IDC_CELLTRUST_SETTINGS, m_btnNxReminderSettings);
	DDX_Control(pDX, IDC_EDIT_ANCHOR_CALLERS, m_nxeditAnchorCallerCount);
	DDX_Text(pDX, IDC_EDIT_ANCHOR_CALLERS, m_nAnchorCallerCount);
	DDV_MinMaxLong(pDX, m_nAnchorCallerCount, 0, 255);
	// Please stop adding these pointless m_nxeditXYZ member variables. The controls are subclassed automatically by CNxDialog.
}


BEGIN_MESSAGE_MAP(CSupportDlg, CPatientDialog)
	//{{AFX_MSG_MAP(CSupportDlg)
	// (f.dinatale 2010-05-21) - PLID 38842 - Removed Shadow support
	//ON_BN_CLICKED(IDC_SHADOW_SETUP, OnShadowSetup)
	ON_BN_CLICKED(IDC_ALLOW_SUPPORT_CHECK, OnAllowSupport)
	ON_BN_CLICKED(IDC_SUPPORT_FILE, OnSupportFile)
	ON_BN_CLICKED(IDC_ENABLE_SUPPORT_ITEMS, OnEnableSupportItems)
	ON_BN_CLICKED(IDC_VIEW_SUPPORT_HISTORY, OnViewSupportHistory)
	ON_BN_CLICKED(IDC_EDIT_WEB_LOGIN, OnEditWebLogin)
	ON_BN_CLICKED(IDC_NEW_VERSION, OnNewVersion)
	ON_BN_CLICKED(IDC_THIRD_PARTY_INFO, OnThirdPartyInfo)
	ON_BN_CLICKED(IDC_IS_LICENSE_ACTIVE, OnIsLicenseActive)
	// (f.dinatale 2010-05-21) - PLID 38842 - Removed Shadow support
	//ON_BN_CLICKED(IDC_ALLOW_SHADOW, OnAllowShadow)
	ON_BN_CLICKED(IDC_CONNECTION_INFO, OnConnectionInfo)
	// (f.dinatale 2010-05-21) - PLID 38842 - Added a Version History Dialog
	ON_BN_CLICKED(IDC_VER_HISTORY, OnVerHistory)
	// (f.dinatale 2010-07-07) - PLID 39527 - Add an Integration Info Dialog
	ON_BN_CLICKED(IDC_BTNINTEGRATION, OnIntegration)
	// (f.dinatale 2010-10-25) - PLID 40827 - Added a NxReminder Settings Dialog
	ON_BN_CLICKED(IDC_CELLTRUST_SETTINGS, OnNxReminderSettings)
	//(c.copits 2011-10-04) PLID 28219 - Use guided dialog to edit licensing options
	ON_BN_CLICKED(IDC_CHANGE_LICENSES, OnBnClickedChangeLicenses)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CONC_TS_LIST_BTN, &CSupportDlg::OnBnClickedConcTsListBtn)
	ON_BN_CLICKED(IDC_ENABLE_IPADS_CHECK, &CSupportDlg::OnEnableIpadsCheck)
	ON_BN_CLICKED(IDC_NexCloud, &CSupportDlg::OnBnClickedNexcloud)//l.banegas PLID 57275 
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_ANCHOR_PASS, &CSupportDlg::OnBnClickedChangeAnchorPassword)
	ON_EN_KILLFOCUS(IDC_EDIT_ANCHOR_CALLERS, &CSupportDlg::OnAnchorCallerCountFocusLost)
	ON_BN_CLICKED(IDC_ANALYTICSLICENSING_BTN, &CSupportDlg::OnBnClickedAnalyticslicensingBtn)
	ON_BN_CLICKED(IDC_EDIT_DICTATION_INFO, &CSupportDlg::OnBnClickedEditDictationInfo)
	ON_BN_CLICKED(IDC_SUBSCRIPTION_LOCAL_HOSTED, &CSupportDlg::OnIsSubscriptionLocalHosted) // (b.eyers - 2016-03-03) - PLID 68480 - Subscription Local Hosted
	ON_BN_CLICKED(IDC_AZURE_REMOTE_APP_CHECK, &CSupportDlg::OnAzureRemoteAppCheck)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSupportDlg message handlers

void CSupportDlg::SetColor(OLE_COLOR nNewColor)
{
	((CNxColor*)GetDlgItem(IDC_COLOR_LICENSE))->SetColor(nNewColor);
	((CNxColor*)GetDlgItem(IDC_COLOR_DATES))->SetColor(nNewColor);
	((CNxColor*)GetDlgItem(IDC_COLOR_INFO))->SetColor(nNewColor);

	CPatientDialog::SetColor(nNewColor);
}

// (f.dinatale 2010-05-21) - PLID 38842 - Removed Shadow support
//void CSupportDlg::OnShadowSetup() 
//{
//	long set = 0;
//	if(((CButton*)GetDlgItem(IDC_SHADOW_SETUP))->GetCheck())
//		set = 1;
//
//	try {
//		ExecuteSql("UPDATE NxClientsT SET ShadowSetup = %li WHERE PersonID = %li",set, m_id);
//	} NxCatchAll("Error in Updating ShadowSetup");	
//	
//}

void CSupportDlg::OnAllowSupport()
{
	try {
		CAuditSupportDlg dlg(this);
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				CheckDlgButton(IDC_ALLOW_SUPPORT_CHECK, IsDlgButtonChecked(IDC_ALLOW_SUPPORT_CHECK)?BST_UNCHECKED:BST_CHECKED);
			}

			CString strOld = IsDlgButtonChecked(IDC_ALLOW_SUPPORT_CHECK)?"No":"Yes";
			CString strNew = IsDlgButtonChecked(IDC_ALLOW_SUPPORT_CHECK)?"Yes":"No";

			SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "AllowExpiredSupport", strOld, strNew,  _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils
		}
		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		ExecuteParamSql(
			"UPDATE NxClientsT SET AllowExpiredSupport = {INT} WHERE PersonID = {INT}", 
			IsDlgButtonChecked(IDC_ALLOW_SUPPORT_CHECK) ? 1 : 0, 
			m_id);
	} NxCatchAll("CSupportDlg::OnAllowSupport");	
}

//These values are written to data.
CString GetNameForPracticeType(int nType)
{
	switch(nType) {
	case 0:
		return "Standard";
		break;
	case 1:
		return "Reproductive";
		break;
	case 2:
		return "Refractive";
		break;
	case 3:
		return "Internal";
		break;
	default:
		ASSERT(FALSE);
		return "";
	}
}

BOOL CSupportDlg::OnInitDialog() 
{
	CPatientDialog::OnInitDialog();

	try {
		// (j.armen 2014-09-03 09:22) - PLID 57853 - Bulk Caching
		// (r.gonet 09-08-2014) - PLID 63583 - Bulk cache NexReminder properties.
		g_propManager.BulkCache("SupportDlg", propbitText | propbitImage | propbitNumber, 
			"Username = '<None>' AND Name IN ("
			"	'NexWebAdminURL' "
			"	, 'NexWebAdminUsername' "
			"	, 'NexWebAdminPassword' "
			"	, 'NxReminderProductionURL' "
			"	, 'NxReminderTestURL' "
			"	, 'NxReminderURI' "
			"	, 'NxReminderUsername' "
			"	, 'NxReminderPassword' "
			"	, 'NxReminderProductionMode' "
			") ");

		m_btnConnectionInfo.AutoSet(NXB_MODIFY);
		m_btnThirdPartInfo.AutoSet(NXB_MODIFY);
		m_btnEditWebLogin.AutoSet(NXB_MODIFY);
		m_btnLabIntegrationInfo.AutoSet(NXB_MODIFY);
		// (f.dinatale 2010-10-25) - PLID 40827
		m_btnNxReminderSettings.AutoSet(NXB_MODIFY);

		m_id = GetActivePatientID();

		//fill in the ratings combo
		m_pEbillingType = BindNxDataListCtrl(IDC_EBILLING_TYPE, true);
		m_pItemList = BindNxDataListCtrl(IDC_SUPPORT_ITEMS_LIST, false);
		m_pASPSForms = BindNxDataListCtrl(IDC_ASPS_FORMS, false);
		m_pCurVersion = BindNxDataListCtrl(IDC_VERSION_CUR, true);
		m_pPracticeType = BindNxDataListCtrl(IDC_PRACTICE_TYPE, false);
		// (f.dinatale 2010-05-21) - PLID 38842 - Removed Shadow support
		//m_pShadowedVersion = BindNxDataListCtrl(IDC_VERSION_DOWNLOADED, true);
		//r.wilson Oct 5 2011 PLID 45836
		m_pAAOForms = BindNxDataListCtrl(IDC_AAO_FORMS, false);

		m_pAMAInstalled = BindNxTimeCtrl(this, IDC_AMA_INSTALLED_ON_DATE);
		m_pASPSPurchDate = BindNxTimeCtrl(this, IDC_ASPS_PURCH_DATE);
		m_pEStatementType = BindNxDataListCtrl(IDC_ESTATEMENT_TYPE, true);
		m_pLicenseActivatedDate = BindNxTimeCtrl(this, IDC_LICENSE_ACTIVATED_ON);
		//r.wilson Oct 5 2011 PLID 45836
		m_pAAOPurchDate = BindNxTimeCtrl(this, IDC_AAO_PURCH_DATE);

		RequeryItemList();
		OnEnableSupportItems();

		((CEdit*)GetDlgItem(IDC_IPAD_BOUGHT))->SetLimitText(9);
		((CEdit*)GetDlgItem(IDC_IPAD_INUSE))->SetLimitText(9);
		((CEdit*)GetDlgItem(IDC_IPAD_PENDING))->SetLimitText(9);

		// (b.savon 2016-04-08) - NX-100086 - Set max text limit
		((CEdit*)GetDlgItem(IDC_NEXERX_IDP_ID_EDIT))->SetLimitText(1000);
		((CEdit*)GetDlgItem(IDC_NEWCROP_IDP_ID_EDIT))->SetLimitText(1000);

		IRowSettingsPtr pRow;
		_variant_t var;

		//add an empty row to the ebilling
		pRow = m_pEbillingType->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, "{No Type}");
		m_pEbillingType->InsertRow(pRow,0);

		//add all data for the asps forms list
		pRow = m_pASPSForms->GetRow(sriNoRow);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("{No Selection}"));
		m_pASPSForms->AddRow(pRow);

		pRow = m_pASPSForms->GetRow(sriNoRow);
		pRow->PutValue(0, (long)ASPS_FROM_ASPS);
		pRow->PutValue(1, _bstr_t("From ASPS"));
		m_pASPSForms->AddRow(pRow);

		pRow = m_pASPSForms->GetRow(sriNoRow);
		pRow->PutValue(0, (long)ASPS_FROM_NEXTECH);
		pRow->PutValue(1, _bstr_t("From NexTech"));
		m_pASPSForms->AddRow(pRow);

		//r.wilson Oct 5 2011 PLID 45836
		//add all data for the AAO forms list
		pRow = m_pAAOForms->GetRow(sriNoRow);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, _bstr_t("{No Selection}"));
		m_pAAOForms->AddRow(pRow);

		pRow = m_pAAOForms->GetRow(sriNoRow);
		pRow->PutValue(0, (long)AAO_FROM_AAO);
		pRow->PutValue(1, _bstr_t("From AAO"));
		m_pAAOForms->AddRow(pRow);

		pRow = m_pAAOForms->GetRow(sriNoRow);
		pRow->PutValue(0, (long)AAO_FROM_NEXTECH);
		pRow->PutValue(1, _bstr_t("From NexTech"));
		m_pAAOForms->AddRow(pRow);

		//add an empty row to the estatement
		pRow = m_pEStatementType->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, "{No Type}");
		m_pEStatementType->InsertRow(pRow,0);

		pRow = m_pPracticeType->GetRow(sriNoRow);
		pRow->PutValue(0, (long)0);
		pRow->PutValue(1, _bstr_t(GetNameForPracticeType(0)));
		m_pPracticeType->AddRow(pRow);
		pRow = m_pPracticeType->GetRow(sriNoRow);
		pRow->PutValue(0, (long)1);
		pRow->PutValue(1, _bstr_t(GetNameForPracticeType(1)));
		m_pPracticeType->AddRow(pRow);
		pRow = m_pPracticeType->GetRow(sriNoRow);
		pRow->PutValue(0, (long)2);
		pRow->PutValue(1, _bstr_t(GetNameForPracticeType(2)));
		m_pPracticeType->AddRow(pRow);
		pRow = m_pPracticeType->GetRow(sriNoRow);
		pRow->PutValue(0, (long)3);
		pRow->PutValue(1, _bstr_t(GetNameForPracticeType(3)));
		m_pPracticeType->AddRow(pRow);

		m_changed = FALSE;
		m_bBeginNewLockoutPeriod = FALSE; // (b.eyers 2015-03-05) - PLID 65097

	} NxCatchAll("Error loading support dialog.");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSupportDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	if(m_id != GetActivePatientID()) {
		//The user has moved to a new patient.  We need to correct the m_bHideForNewClient flag, if it has been enabled.
		if(m_bHideForNewClient) {
			m_bHideForNewClient = FALSE;
		}
	}

	m_id = GetActivePatientID();

	//just for this tab we're going to do things a little differently
	//when this tab is loaded it checks to see if an entry exists in NxClientsT (which contains a PersonID to 
	//link with PersonT).  If nothing exists, this function will add an entry with the default values to it
	//This saves us changing a bunch of code elsewhere in the program and conflicts with actual Clients who 
	//don't have a Support tab.
	//If an entry does exist, nothing will happen and the dialog will act as it normally would
	_RecordsetPtr rs(__uuidof(Recordset));
	CString sql;
	sql.Format("SELECT PersonID FROM NxClientsT WHERE PersonID = %li", m_id);
	try{

		rs = CreateRecordsetStd(sql);

		if(rs->eof)
		{
			// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
			ExecuteParamSql("INSERT INTO NxClientsT (PersonID, LicenseKey) (SELECT {INT}, {INT})", m_id, NewNumber("NxClientsT", "LicenseKey"));
		}

		rs->Close();
	} NxCatchAll("Error in CSupportDlg::UpdateView()");

	// Now load the data
	try {
		Load();
		//when load does a SetDlgItemText(), it causes m_changed to get set to true.  we need to fix that.
		m_changed = false;
	} NxCatchAll("CSupportDlg::UpdateView:Load");

	//(c.copits 2010-06-30) PLID 39427 - Reset support permissions when switching patients
	// (j.dinatale 2010-09-03) - PLID 39947 - Always set hide for new client to false before assessing permissions.
	//		We would like to have the user audit happen if the user does change clients. After we reset the hide on new cleint flag,
	//		we want to check if the user now has permissions to have edit every client, if they dont, then go ahead and reset our 
	//		enabled flags and refresh our view.
	try{
		m_bHideForNewClient = FALSE;
		if(!(GetCurrentUserPermissions(bioSupportBought) & sptWrite)){
			m_bItemsEnabled = FALSE;
			OnEnableSupportItems();
		}
		else if(!m_bItemsEnabled) {
			// (z.manning 2012-10-17 13:03) - PLID 53229 - Update the fields no matter what if we're in enabled mode
			EnableBoughtUsedLicenseFields(TRUE);
		}
	}NxCatchAll("CSupportDlg::CheckUserPermissions");
}

// (r.ries 2013-1-4 PLID) - 50881 strPreparedAuditText accepts a string that is used for the description rather than using the Audit popup
CString CSupportDlg::Save(int nID, const CString strPreparedAuditText)
{
	try {

		CString field, value, strAuditText;
		// (j.luckoski 2012-06-06 12:26) - PLID 50831 - Added dt to validate fields
		COleDateTime dt;

		if(!m_changed && strPreparedAuditText.IsEmpty())
			return "";
		
		if(!nID) {
			m_changed = false;
			return "";
		}

		GetDlgItemText(nID, value);
		bool bQuoted = true;
		// (j.luckoski 2012-06-06 12:27) - PLID 50114 - Distinquish date fields in the saving process
		bool bDate = false;

		// (j.armen 2014-02-04 12:44) - PLID 60632 - To make this as simple as possible, if any of our dication fields
		//	changed, and the value is greater than 0, call our API function.  The API will only attempt to create
		//	a Nuance Organization if we do not have one logged for the client already
		//
		// (z.manning 2015-09-08 11:13) - PLID 63512 - Moved this up because we need to make sure this works
		// before saving the Nuance license fields.
		if ((nID == IDC_B_DICTATION || nID == IDC_U_DICTATION || nID == IDC_P_DICTATION)
			&& AsLong(_bstr_t(value)) > 0)
		{
			if (ReturnsRecordsParam("SELECT C.LicenseKey FROM NxClientsT C WHERE C.PersonID = {INT} AND (C.NuanceOrganizationGuid IS NULL OR C.NuanceLicenseGuid IS NULL)", m_id))
			{
				// (z.manning 2015-12-18 10:51) - PLID 67738 - We now have a dialog for the required dictation fields
				if (PromptForDictationInfo() != IDOK)
				{
					UpdateView();
					MessageBox("You cannot assign dictation licenses without filling out the required fields.", "Nuance", MB_ICONERROR);
					return "";
				}
			}
		}

		//find the correct field
		switch(nID)
		{
		case IDC_B_LIC:
			field = "LicenseBought";
			break;
		case IDC_U_LIC:
			field = "LicenseUsed";
			break;
		case IDC_P_LIC:
			field = "LicensePending";
			bQuoted = false;
			break;
		// (z.manning 2012-06-12 15:05) - PLID 50878 - Added iPad fields
		case IDC_IPAD_BOUGHT:
			field = "IpadBought";
			break;
		case IDC_IPAD_INUSE:
			field = "IpadUsed";
			break;
		case IDC_IPAD_PENDING:
			field = "IpadPending";
			break;
		// (d.thompson 2011-10-03) - PLID 45791
		case IDC_B_CONC_TS_LIC:
			field = "ConcurrentTSLicensesBought";
			break;
		case IDC_U_CONC_TS_LIC:
			field = "ConcurrentTSLicensesUsed";
			break;
		case IDC_P_CONC_TS_LIC:
			field = "ConcurrentTSLicensesPending";
			bQuoted = false;
			break;
		case IDC_B_DOC:
			field = "DoctorsBought";
			break;
		case IDC_U_DOC:
			field = "DoctorsUsed";
			break;
		case IDC_P_DOC:
			field = "DoctorsPending";
			bQuoted = false;
			break;
		case IDC_B_PALM:
			field = "PalmPilotBought";
			break;
		case IDC_U_PALM:
			field = "PalmPilotUsed";
			break;
		case IDC_P_PALM:
			field = "PalmPilotPending";
			bQuoted = false;
			break;
		case IDC_B_PPC:
			field = "PPCBought";
			break;
		case IDC_U_PPC:
			field = "PPCUsed";
			break;
		case IDC_P_PPC:
			field = "PPCPending";
			bQuoted = false;
			break;

		// (z.manning 2009-10-16 15:45) - PLID 35749 - NexSync licensing
		case IDC_B_NEXSYNC:
			field = "NexSyncBought";
			break;
		case IDC_U_NEXSYNC:
			field = "NexSyncUsed";
			break;
		case IDC_P_NEXSYNC:
			field = "NexSyncPending";
			bQuoted = false;
			break;

		// (z.manning 2014-01-31 14:04) - PLID 55147
		case IDC_B_DICTATION:
			field = "NuanceBought";
			break;
		case IDC_U_DICTATION:
			field = "NuanceUsed";
			break;
		case IDC_P_DICTATION:
			field = "NuancePending";
			bQuoted = false;
			break;

		case IDC_B_DATABASES:
			field = "DatabasesBought";
			break;
		case IDC_U_DATABASES:
			field = "Databasesused";
			break;
		case IDC_P_DATABASES:
			field = "DatabasesPending";
			bQuoted = false;
			break;
		case IDC_B_EMRPROV:					// (a.walling 2006-12-19 16:45) - PLID 23929 - Handle our EMR Provider licenses
			field = "EMRProvidersBought";
			break;
		case IDC_VPN://l.banegas 7/9/13
			field = "NexCloudVPN";
			break;
		case IDC_U_EMRPROV:
			field = "EMRProvidersUsed";
			break;
		case IDC_P_EMRPROV:
			field = "EMRProvidersPending";
			bQuoted = false;
			break;

		case IDC_B_LICPRES:							// (r.farnworth 2013-07-19) PLID 57522
			field = "ERxLicPrescribersBought";
			break;
		case IDC_U_LICPRES:							// (r.farnworth 2013-07-19) PLID 57522
			field = "ERxLicPrescribersUsed";
			break;
		case IDC_P_LICPRES:							// (r.farnworth 2013-07-19) PLID 57522
			field = "ERxLicPrescribersPending";
			bQuoted = false;
			break;
		case IDC_B_MIDPRES:							// (r.farnworth 2013-07-19) PLID 57522
			field = "ERxMidPrescribersBought";
			break;
		case IDC_U_MIDPRES:							// (r.farnworth 2013-07-19) PLID 57522
			field = "ERxMidPrescribersUsed";
			break;
		case IDC_P_MIDPRES:							// (r.farnworth 2013-07-19) PLID 57522
			field = "ERxMidPrescribersPending";
			bQuoted = false;
			break;
		case IDC_B_STAFFPRES:							// (r.farnworth 2013-07-19) PLID 57522
			field = "ERxStaffPrescribersBought";
			break;
		case IDC_U_STAFFPRES:							// (r.farnworth 2013-07-19) PLID 57522
			field = "ERxStaffPrescribersUsed";
			break;
		case IDC_P_STAFFPRES:							// (r.farnworth 2013-07-19) PLID 57522
			field = "ERxStaffPrescribersPending";
			bQuoted = false;
			break;

			// (z.manning 2015-06-17 14:25) - PLID 66278 - Added portal provider fields
		case IDC_B_PORTAL_PROVIDERS:							// (r.farnworth 2013-07-19) PLID 57522
			field = "PortalProvidersBought";
			break;
		case IDC_U_PORTAL_PROVIDERS:							// (r.farnworth 2013-07-19) PLID 57522
			field = "PortalProvidersUsed";
			break;
		case IDC_P_PORTAL_PROVIDERS:							// (r.farnworth 2013-07-19) PLID 57522
			field = "PortalProvidersPending";
			bQuoted = false;
			break;

		case IDC_B_NEXWEB_DOMAINS:					// (d.thompson 2009-08-11) - PLID 35170
			field = "NexWebAddtlDomainsBought";
			break;
		case IDC_U_NEXWEB_DOMAINS:					// (d.thompson 2009-08-11) - PLID 35170
			field = "NexWebAddtlDomainsUsed";
			break;
		case IDC_P_NEXWEB_DOMAINS:
			field = "NexWebAddtlDomainsPending";
			bQuoted = false;
			break;
		case IDC_Users:
			field ="NexCloudUsers";
			break;
		case IDC_EMRPRO:	// (e.lally 2008-07-09) PLID 30493
		{
			field = "EMRPro";
			CString strEmrProvLicenses;
			GetDlgItemText(IDC_B_EMRPROV, strEmrProvLicenses);
			long nEmrProValueEntered = atoi(value);
			if(atoi(strEmrProvLicenses) < nEmrProValueEntered){
				m_changed = false;
				AfxMessageBox(FormatString(
					"The number of EMR Pro providers may not be greater than the total number of licensed EMR Providers (%s).", strEmrProvLicenses));
				GetDlgItem(IDC_EMRPRO)->SetFocus();
				m_changed = true;
				return "";
			}
			CString strConvertedValue;
			strConvertedValue.Format("%li", nEmrProValueEntered);
			if(value != strConvertedValue){
				m_changed = false;
				AfxMessageBox("The EMR Professional providers must be an integer.");
				value = strConvertedValue;
				SetDlgItemText(IDC_EMRPRO, value);
				GetDlgItem(IDC_EMRPRO)->SetFocus();
				m_changed = true;
				return "";
			}

			break;
		}
		case IDC_EMRPRO_DR_NAMES:	// (e.lally 2008-07-09) PLID 30493
			field = "EMRProDoctorNames";
			break;
		case IDC_PURCH_AGR:
			field = "PurchAgrSign";
			// (j.luckoski 2012-06-06 12:27) - PLID 50831 - Validate teh purchase agreement date.
			bDate = true;
				if (value.GetLength() > 0) {
					dt.ParseDateTime(value);

					if ( (dt.GetYear() < 1990) || (dt.GetStatus() != COleDateTime::valid) )  {
						// invalid or corrupt date time! Prompt the user, and refill their changes with current data.
						// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
						_RecordsetPtr prs = CreateParamRecordset("SELECT PurchAgrSign FROM NxClientsT WHERE PersonID = {INT}", m_id);
						CString strOriginalValue = FormatDateTimeForInterface(AdoFldDateTime(prs, "PurchAgrSign", g_cdtInvalid), DTF_STRIP_SECONDS);

						// (j.luckoski 2012-06-06 16:10) - PLID 50831 - Don't show invalid datetime so null it
						if(strOriginalValue != "Invalid DateTime") {
							SetDlgItemText(IDC_PURCH_AGR, strOriginalValue);
						} else {
							SetDlgItemText(IDC_PURCH_AGR, "");
						}

						MessageBox(FormatString("The purchase agr. signed date of '%s' is invalid and has been reset.", value), "Practice", MB_OK | MB_ICONEXCLAMATION);
						m_changed = false;

						return "";
					}
				}
			break;
		case IDC_LIC_AGR:
			field = "LicAgrSign";
			// (j.luckoski 2012-06-06 12:28) - PLID 50831 - Validate the license agreement date
			bDate = true;
				if (value.GetLength() > 0) {
					dt.ParseDateTime(value);

					if ( (dt.GetYear() < 1990) || (dt.GetStatus() != COleDateTime::valid) )  {
						// invalid or corrupt date time! Prompt the user, and refill their changes with current data.
						// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
						_RecordsetPtr prs = CreateParamRecordset("SELECT LicAgrSign FROM NxClientsT WHERE PersonID = {INT}", m_id);
						CString strOriginalValue = FormatDateTimeForInterface(AdoFldDateTime(prs, "LicAgrSign", g_cdtInvalid), DTF_STRIP_SECONDS);

						// (j.luckoski 2012-06-06 16:10) - PLID 50831 - Don't show invalid datetime so null it
						if(strOriginalValue != "Invalid DateTime") {
							SetDlgItemText(IDC_LIC_AGR, strOriginalValue);
						} else {
							SetDlgItemText(IDC_LIC_AGR, "");
						}

						MessageBox(FormatString("The license agr. signed date of '%s' is invalid and has been reset.", value), "Practice", MB_OK | MB_ICONEXCLAMATION);
						m_changed = false;

						return "";
					}
				}
			break;
		case IDC_INSTALLED:
			field = "InstalledOn";
			// (j.luckoski 2012-06-06 14:18) - PLID 50831 - Validate these date fields
			bDate = true; 
				if (value.GetLength() > 0) {
					dt.ParseDateTime(value);

					if ( (dt.GetYear() < 1990) || (dt.GetStatus() != COleDateTime::valid) )  {
						// invalid or corrupt date time! Prompt the user, and refill their changes with current data.
						// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
						_RecordsetPtr prs = CreateParamRecordset("SELECT InstalledOn FROM NxClientsT WHERE PersonID = {INT}", m_id);
						CString strOriginalValue = FormatDateTimeForInterface(AdoFldDateTime(prs, "InstalledOn", g_cdtInvalid), DTF_STRIP_SECONDS);

						// (j.luckoski 2012-06-06 16:10) - PLID 50831 - Don't show invalid datetime so null it
						if(strOriginalValue != "Invalid DateTime") {
							SetDlgItemText(IDC_INSTALLED, strOriginalValue);
						} else {
							SetDlgItemText(IDC_INSTALLED, "");
						}

						MessageBox(FormatString("The installed on date of '%s' is invalid and has been reset.", value), "Practice", MB_OK | MB_ICONEXCLAMATION);
						m_changed = false;

						return "";
					}
				}
			break;
		case IDC_TRAINER:
			field = "Trainer";
			break;
		case IDC_SUPP_EXP:
			field = "SupportExpires";
			{ // (a.walling 2007-03-22 09:47) - PLID 24434 - Validate the date!
				if (value.GetLength() > 0) {
					dt.ParseDateTime(value, VAR_DATEVALUEONLY);

					// (a.walling 2007-03-23 08:55) - PLID 24434 - Limit lowest date to 1990
					if ( (dt.GetYear() < 1990) || (dt.GetStatus() != COleDateTime::valid) )  {
						// invalid or corrupt date time! Prompt the user, and refill their changes with current data.
						// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
						_RecordsetPtr prs = CreateParamRecordset("SELECT SupportExpires FROM NxClientsT WHERE PersonID = {INT}", m_id);
						CString strOriginalValue = AdoFldString(prs, "SupportExpires", "");

						// (j.luckoski 2012-06-06 16:10) - PLID 50831 - Don't show invalid datetime so null it
						if(strOriginalValue != "Invalid DateTime") {
							SetDlgItemText(IDC_SUPP_EXP, strOriginalValue);
						} else {
							SetDlgItemText(IDC_SUPP_EXP, "");
						}

						MessageBox(FormatString("The support expiration date of '%s' is invalid and has been reset.", value), "Practice", MB_OK | MB_ICONEXCLAMATION);
						m_changed = false;

						return "";
					}
				}
			}
			break;
		case IDC_SUPPORT_NOTES:
			field = "Notes";
			break;
		case IDC_PCA:
			field = "PCA";
			break;
		case IDC_PCA_IP:
			field = "PCAIP";
			break;
		case IDC_LOGIN:
			field = "Login";
			break;
		case IDC_PASSWORD:
			field = "Password";
			break;
		case IDC_SERV_NAME:
			field = "ServerName";
			break;
		case IDC_SERV_OS:
			field = "ServerOS";
			break;
		case IDC_WS_OS:
			field = "WorkOS";
			break;
		case IDC_PATH_SERVER:
			field = "ServerPath";
			break;
		case IDC_NXSERVER_IP:
			field = "NxServerIP";
			break;
		case IDC_PRAC_EMAIL:
			field = "PracEmail";
			break;
		case IDC_LICENSE_KEY:
			field = "LicenseKey";
			if(ReturnsRecords("SELECT PersonID FROM NxClientsT WHERE LicenseKey = %s AND PersonID <> %li", value, m_id)) {
				MsgBox("The license key you have entered is already in use.  Please select a different license key.");
				GetDlgItem(IDC_LICENSE_KEY)->SetFocus();
				return "";
			}
			bQuoted = false;
			break;
		case IDC_AMA_VERSION:
			field = "AMAVersion";
			break;
		case IDC_LICENSE_ACTIVATED_BY:
			field = "LicenseActivatedBy";
			break;
		case IDC_MAX_PATIENT_COUNT:
			field = "MaxPatientCount";
			bQuoted = false;
			break;
		case IDC_MAX_PRACTICE_COUNT:
			field = "PracticeUsageCount";
			bQuoted = false;
			break;
		case IDC_IAGNOSIS_COUNT:
			// (b.cardillo 2015-05-21 09:11) - PLID 66127 - Keep track of the number of Iagnosis Providers
			field = "IagnosisProviderCount";
			bQuoted = false;
			break;
		// (b.savon 2016-04-08) - NX-100086 - Save IDP Transaction IDs
		case IDC_NEXERX_IDP_ID_EDIT:
			field = "NexERxIDPTransactionIDs";
			break;
		case IDC_NEWCROP_IDP_ID_EDIT:
			field = "NewCropIDPTransactionIDs";
			break;
		default:
			break;
		}

		if(field == "") {
			m_changed = false;
			return "";
		}

		//In case data has changed since last update
		bool bNeedsRefresh = false;

		//Auto-increment pending if appropriate.
		bool bUpdatePending = false;
		long nPendingAmount = 0;
		long nNewPendingAmount = 0;
		int nPendingID = -1;
		bool bBought = false;
		CString strPendingField;

		// (j.armen 2014-02-04 12:44) - PLID 60632 - Cleanup, alot of duplicated code.
		// By using a switch statement, we can see more quickly where we are headed
		switch(nID)
		{
			case IDC_B_LIC:
				bBought = true;
			case IDC_U_LIC:
				nPendingID = IDC_P_LIC;
				strPendingField = "LicensePending";
				break;

			// (z.manning 2012-06-12 15:06) - PLID 50878 - iPad fields
			case IDC_IPAD_BOUGHT:
				bBought = true;
			case IDC_IPAD_INUSE:
				nPendingID = IDC_IPAD_PENDING;
				strPendingField = "IpadPending";
				break;

			// (d.thompson 2011-10-03) - PLID 45791
			case IDC_B_CONC_TS_LIC:
				bBought = true;
			case IDC_U_CONC_TS_LIC:
				nPendingID = IDC_P_CONC_TS_LIC;
				strPendingField = "ConcurrentTSLicensesPending";
				break;

			case IDC_B_DOC:
				bBought = true;
			case IDC_U_DOC:
				nPendingID = IDC_P_DOC;
				strPendingField = "DoctorsPending";
				break;

			case IDC_B_PALM:
				bBought = true;
			case IDC_U_PALM:
				nPendingID = IDC_P_PALM;
				strPendingField = "PalmPilotPending";
				break;

			case IDC_B_PPC:
				bBought = true;
			case IDC_U_PPC:
				nPendingID = IDC_P_PPC;
				strPendingField = "PPCPending";
				break;

			// (z.manning 2009-10-16 15:47) - PLID 35749 - NexSync licensing
			case IDC_B_NEXSYNC:
				bBought = true;
			case IDC_U_NEXSYNC:
				nPendingID = IDC_P_NEXSYNC;
				strPendingField = "NexSyncPending";
				break;

			// (z.manning 2014-01-31 14:06) - PLID 55147
			case IDC_B_DICTATION:
				bBought = true;
			case IDC_U_DICTATION:
				nPendingID = IDC_P_DICTATION;
				strPendingField = "NuancePending";
				break;

			case IDC_B_DATABASES:
				bBought = true;
			case IDC_U_DATABASES:
				nPendingID = IDC_P_DATABASES;
				strPendingField = "DatabasesPending";
				break;

			case IDC_B_EMRPROV:			// (a.walling 2006-12-19 16:51) - PLID 23929
				bBought = true;
			case IDC_U_EMRPROV:
				nPendingID = IDC_P_EMRPROV;
				strPendingField = "EMRProvidersPending";
				break;

			case IDC_B_LICPRES:			// (r.farnworth 2013-07-19) - PLID 57522
				bBought = true;
			case IDC_U_LICPRES:
				nPendingID = IDC_P_LICPRES;
				strPendingField = "ERxLicPrescribersPending";
				break;

			case IDC_B_MIDPRES:			// (r.farnworth 2013-07-19) - PLID 57522
				bBought = true;
			case IDC_U_MIDPRES:
				nPendingID = IDC_P_MIDPRES;
				strPendingField = "ERxMidPrescribersPending";
				break;

			case IDC_B_STAFFPRES:			// (r.farnworth 2013-07-19) - PLID 57522
				bBought = true;
			case IDC_U_STAFFPRES:
				nPendingID = IDC_P_STAFFPRES;
				strPendingField = "ERxStaffPrescribersPending";
				break;

			case IDC_B_PORTAL_PROVIDERS:		// (z.manning 2015-06-17 14:26) - PLID 66278
				bBought = true;
			case IDC_U_PORTAL_PROVIDERS:
				nPendingID = IDC_P_PORTAL_PROVIDERS;
				strPendingField = "PortalProvidersPending";
				break;

			case IDC_B_NEXWEB_DOMAINS:			// (d.thompson 2009-08-11) - PLID 35170
				bBought = true;
			case IDC_U_NEXWEB_DOMAINS:
				nPendingID = IDC_P_NEXWEB_DOMAINS;
				strPendingField = "NexWebAddtlDomainsPending";
				break;
		}
		
		if(nPendingID != -1) {
			//DRT 10/5/2005 - PLID 17810 - This value needs to calculate off the data, not the interface.  Someone may have updated
			//	the other value since the time we loaded this data.
			//nPendingAmount = GetDlgItemInt(nPendingID);
			_RecordsetPtr prsPending = CreateParamRecordset(FormatString("SELECT %s AS Data FROM NxClientsT WHERE PersonID = {INT}", strPendingField), m_id);
			if(prsPending->eof) {
				AfxMessageBox("Failed to retrieve pending information.  Please refresh and try again.");
				m_changed = false;
				return "";
			}
			nPendingAmount = AdoFldLong(prsPending, "Data", 0);
			prsPending->Close();
			//

			//If the interface and data do not match, we'll refresh at the end
			if(nPendingAmount != (long)GetDlgItemInt(nPendingID))
				bNeedsRefresh = true;

			CString strCurrentAmount = VarString(GetTableField("NxClientsT", field, "PersonID", m_id));
			long nCurrentAmount = strCurrentAmount.IsEmpty() ? 0 : atol(strCurrentAmount);
			long nNewAmount = atol(value);
			long nOffset = nNewAmount-nCurrentAmount;
			if(nOffset != 0) {
				bUpdatePending = true;
				if(bBought) {
					//Just go ahead and do it.
					nNewPendingAmount = nPendingAmount + nOffset;
				}
				else {
					//Validate.
					nNewPendingAmount = nPendingAmount-nOffset;
					if((nPendingAmount < 0 && nNewPendingAmount>0) ||
						(nPendingAmount > 0 && nNewPendingAmount<0)) {
						nNewPendingAmount = 0;
						m_changed = false;
						if(IDYES != MsgBox(MB_YESNO, "The amount you are changing the In Use column is greater than the amount Pending.  Are you SURE you wish to do this? (If Yes, the Pending column will be set to 0.)")) {
							SetDlgItemText(nID, strCurrentAmount);
							return "";
						}
					}
				}
			}
		}

		//auditing
		switch(nID) {
		case IDC_B_LIC:
		case IDC_U_LIC:
		case IDC_P_LIC:
		// (z.manning 2012-06-12 15:10) - PLID 50878 - iPad fields
		case IDC_IPAD_BOUGHT:
		case IDC_IPAD_INUSE:
		case IDC_IPAD_PENDING:
		case IDC_B_CONC_TS_LIC:	// (d.thompson 2011-10-03) - PLID 45791
		case IDC_U_CONC_TS_LIC:	// (d.thompson 2011-10-03) - PLID 45791
		case IDC_P_CONC_TS_LIC:	// (d.thompson 2011-10-03) - PLID 45791
		case IDC_B_DOC:
		case IDC_U_DOC:
		case IDC_P_DOC:
		case IDC_B_PALM:
		case IDC_U_PALM:
		case IDC_P_PALM:
		case IDC_B_PPC:
		case IDC_U_PPC:
		case IDC_P_PPC:
		case IDC_B_NEXSYNC: // (z.manning 2009-10-16 15:48) - PLID 35749
		case IDC_U_NEXSYNC:
		case IDC_P_NEXSYNC:
		case IDC_B_DICTATION: // (z.manning 2014-01-31 14:06) - PLID 55147
		case IDC_U_DICTATION:
		case IDC_P_DICTATION:
		case IDC_B_DATABASES:
		case IDC_U_DATABASES:
		case IDC_P_DATABASES:
		case IDC_B_LICPRES:		//	(r.farnworth 2013-07-19) PLID 57522
		case IDC_U_LICPRES:		//	(r.farnworth 2013-07-19) PLID 57522
		case IDC_P_LICPRES:		//	(r.farnworth 2013-07-19) PLID 57522
		case IDC_B_MIDPRES:		//	(r.farnworth 2013-07-19) PLID 57522
		case IDC_U_MIDPRES:		//	(r.farnworth 2013-07-19) PLID 57522
		case IDC_P_MIDPRES:		//	(r.farnworth 2013-07-19) PLID 57522
		case IDC_B_STAFFPRES:		//	(r.farnworth 2013-07-19) PLID 57522
		case IDC_U_STAFFPRES:		//	(r.farnworth 2013-07-19) PLID 57522
		case IDC_P_STAFFPRES:		//	(r.farnworth 2013-07-19) PLID 57522
		case IDC_B_PORTAL_PROVIDERS:
		case IDC_U_PORTAL_PROVIDERS:
		case IDC_P_PORTAL_PROVIDERS:
		case IDC_B_EMRPROV:		// (a.walling 2006-12-19 16:50) - PLID 23929
		case IDC_U_EMRPROV:
		case IDC_B_NEXWEB_DOMAINS:	// (d.thompson 2009-08-11) - PLID 35170
		case IDC_U_NEXWEB_DOMAINS:	// (d.thompson 2009-08-11) - PLID 35170
		case IDC_P_EMRPROV:
		case IDC_P_NEXWEB_DOMAINS:	// (d.thompson 2009-08-11) - PLID 35170
		case IDC_EMRPRO:			// (e.lally 2008-07-09) PLID 30493
		case IDC_EMRPRO_DR_NAMES:	// (e.lally 2008-07-09) PLID 30493
		case IDC_AMA_VERSION:
		case IDC_LICENSE_KEY:
		case IDC_IS_LICENSE_ACTIVE:
		case IDC_LICENSE_ACTIVATED_BY:
		case IDC_MAX_PATIENT_COUNT:
		case IDC_MAX_PRACTICE_COUNT:
		case IDC_IAGNOSIS_COUNT: // (b.cardillo 2015-05-21 09:11) - PLID 66127 - Keep track of the number of Iagnosis Providers
		case IDC_LIC_AGR: //(a.wilson 2011-9-2) PLID 45257
		//case IDC_ALLOW_SHADOW:	// (f.dinatale 2010-05-21) - PLID 38842 - Removed Shadow support
		case IDC_SUPP_EXP:
		case IDC_VPN://l.banegas 7/09/13
		case IDC_Users: //l.banegas 7/09/13
		case IDC_NexCloud: //l.banegas 
		case IDC_SUBSCRIPTION_LOCAL_HOSTED: // (b.eyers - 2016-03-03) - PLID 68480 - Subscription Local Hosted
		// (b.savon 2016-04-08) - NX-100086 - Audit IDP Transaction IDs
		case IDC_NEXERX_IDP_ID_EDIT:
		case IDC_NEWCROP_IDP_ID_EDIT:
		{
			//for auditing
			// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
			_RecordsetPtr prs = CreateParamRecordset(FormatString("SELECT %s FROM NxClientsT WHERE PersonID = {INT}", field), m_id);

			//we have to set the changed to false first, otherwise the DoModal() will force another killfocus message
			// and we'll get 2 boxes to popup asking why.
			m_changed = false;

			//(c.copits 2011-11-10) PLID 28219 - Use guided dialog to edit licensing options
			// (d.singleton 2012-01-10 12:14) - PLID 28219 - took out the check to see if wizard was used, the new dialog no longer needs special treatment for the audit text
			// (r.ries 2013-1-4 PLID) - 50881 - Support predefined audit text
			if(strPreparedAuditText.IsEmpty())
			{
				CAuditSupportDlg dlg(this);

				if(!m_bHideForNewClient) {
					if(dlg.DoModal() == IDCANCEL) {
						//reset the selection
						if(!prs->eof) {
							SetDlgItemText(nID, AsString(prs->Fields->Item[_bstr_t(field)]->Value));
						}
						else
							SetDlgItemText(nID, "");
						return "";
					}
					strAuditText = dlg.m_strText;
				}
			}
			else
			{
				strAuditText = strPreparedAuditText;
			}

			CString strOld;
			if(!prs->eof) {
				// (a.walling 2006-12-19 16:50) - PLID 23929
				// (e.lally 2008-07-09) PLID 30493
				// (z.manning 2009-10-16 15:48) - PLID 35749 - Added IDC_P_NEXSYNC
				// (d.thompson 2011-10-03) - PLID 45791 - Added IDC_P_CONC_TS_LIC
				// (r.ries 2013-01-28) - PLID 54836 - Reorganized these a bit to make them make sense, added a couple fields
				// (r.farnworth 2013-07-19) PLID 57522 - Added checking for the Licesed/Mid/Staff Providers
				// (b.cardillo 2015-05-21 09:11) - PLID 66127 - Keep track of the number of Iagnosis Providers
				if(nID == IDC_P_LIC || nID == IDC_IPAD_PENDING || nID == IDC_P_CONC_TS_LIC || nID == IDC_P_DOC || nID == IDC_P_NEXSYNC || nID == IDC_P_PALM || nID == IDC_P_PPC || nID == IDC_P_DATABASES || 
					nID == IDC_LICENSE_KEY || nID == IDC_MAX_PATIENT_COUNT || nID == IDC_MAX_PRACTICE_COUNT ||
					nID == IDC_P_EMRPROV || nID == IDC_P_NEXWEB_DOMAINS || nID == IDC_EMRPRO || nID == IDC_MAX_PATIENT_COUNT || nID == IDC_MAX_PRACTICE_COUNT ||
					nID == IDC_P_LICPRES || nID == IDC_P_MIDPRES || nID == IDC_P_STAFFPRES || nID == IDC_P_DICTATION ||
					nID == IDC_IAGNOSIS_COUNT || nID == IDC_P_PORTAL_PROVIDERS
					)
				{
					strOld.Format("%li", AdoFldLong(prs, field, 0));
				} // (r.ries 2013-01-28) - PLID 54836 - Need to audit this stuff too
				else if (nID == IDC_B_LIC || nID == IDC_IPAD_BOUGHT || nID == IDC_B_CONC_TS_LIC || nID == IDC_B_DOC || nID == IDC_B_NEXSYNC || nID == IDC_B_PALM || nID == IDC_B_PPC || nID == IDC_B_DATABASES ||
					nID == IDC_B_EMRPROV || nID == IDC_B_NEXWEB_DOMAINS ||
					nID == IDC_U_LIC || nID == IDC_IPAD_INUSE || nID == IDC_U_CONC_TS_LIC || nID == IDC_U_DOC || nID == IDC_U_NEXSYNC || nID == IDC_U_PALM || nID == IDC_U_PPC || nID == IDC_U_DATABASES ||
					nID == IDC_U_EMRPROV || nID == IDC_U_NEXWEB_DOMAINS || nID == IDC_EMRPRO_DR_NAMES || nID==IDC_VPN || nID==IDC_Users/*l.banegas*/ ||
					nID == IDC_B_LICPRES || nID == IDC_B_MIDPRES || nID == IDC_B_STAFFPRES || nID == IDC_B_PORTAL_PROVIDERS ||
					nID == IDC_U_LICPRES || nID == IDC_U_MIDPRES || nID == IDC_U_STAFFPRES || nID == IDC_U_PORTAL_PROVIDERS ||
					nID == IDC_U_DICTATION)
				{
					strOld = AdoFldString(prs, field, 0);
				}
				else if (nID == IDC_NEXERX_IDP_ID_EDIT || nID == IDC_NEWCROP_IDP_ID_EDIT) {
					// (b.savon 2016-04-08) - NX-100086 - Audit IDP Transaction IDs
					strOld = AdoFldString(prs, field, "");
				}
				else if (nID == IDC_LIC_AGR || nID == IDC_SUPP_EXP) // (r.ries 2013-01-29) - PLID 54836 - Need to audit IDC_SUPP_EXP
				{
					//(a.wilson 2011-9-2) PLID 45257 - safely handle what variant we get from recordset.
					_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
					if (var.vt == VT_DATE)
					{
						COleDateTime dtOld = VarDateTime(var);
						// (j.luckoski 2012-06-06 16:26) - PLID 50831 - Format oldstr to show time.
						strOld = dtOld.Format(_T("%m/%d/%Y %I:%M %p"));
					}
					else
					{
						strOld = VarString(var, "");
					}
					// (d.thompson 2013-07-23) - PLID 57674 - Removed some old code that was re-validating the date (incorrectly), 
					//	even though we already validated the date above.
				}
			}
			SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), field, strOld, value,  _Q(strAuditText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils
		}	break;
		default:
			//no auditing
			break;
		}

		// (j.luckoski 2012-06-06 14:23) - PLID 50114 - Added in bDate to distinquish date fields from STRING fields
		if(bDate) {
			if (value.GetLength() > 0) {
				ExecuteParamSql("UPDATE NxClientsT SET {CONST_STRING} = {OLEDATETIME} WHERE PersonID = {INT}", field, dt, m_id);
			} else {
				ExecuteParamSql("UPDATE NxClientsT SET {CONST_STRING} = NULL WHERE PersonID = {INT}", field, m_id);
			}
		} else if(bQuoted) {
			// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
			ExecuteParamSql("UPDATE NxClientsT SET {CONST_STRING} = {STRING} WHERE PersonID = {INT}", field, value, m_id);
		}
		else {
			ExecuteParamSql("UPDATE NxClientsT SET {CONST_STRING} = {INT} WHERE PersonID = {INT}", field, AsLong(_bstr_t(value)), m_id);
		}
		
		if(bUpdatePending) {
			SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), strPendingField, AsString(nPendingAmount), AsString(nNewPendingAmount), "Auto-incremented by system"); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils
			ExecuteParamSql("UPDATE NxClientsT SET {CONST_STRING} = {INT} WHERE PersonID = {INT}", strPendingField, nNewPendingAmount, m_id);
			SetDlgItemInt(nPendingID, nNewPendingAmount);
		}

		// (b.eyers 2015-03-05) - PLID 65098 - Save the swap lockout timestamp, reset to false so it doesn't insert twice
		if (m_bBeginNewLockoutPeriod) {
			ExecuteParamSql("INSERT INTO NxClientsLicenseSwapHistoryT (ClientID, SwappedByUserID, SwapDate, ExpirationDate) "
				"VALUES ({INT}, {INT}, GetDate(), DATEADD(d, {CONST_INT}, GetDate()))", m_id, GetCurrentUserID(), LOCKOUT_DAYS);
			m_bBeginNewLockoutPeriod = false;
		}

		m_changed = false;

		if(bNeedsRefresh) {
			AfxMessageBox("This client's data has changed since the tab was last loaded.  The support tab will now be refreshed.");
			UpdateView();
		}
		return strAuditText;
	} NxCatchAll("Error Saving Client Info");
	return "";
}

////////////////////////
// These three macros are specifically for use in the CSupportDlg::Load() function (CSDL 
// stands for C Support Dlg Load) because they make reference to local variables as well 
// as dialog objects
#define CSDL__LOAD_FIELD_BOOL(field_name, control_idc) \
	if (AdoFldBool(pflds, field_name)) CheckDlgButton(control_idc, 1); else CheckDlgButton(control_idc, 0);

#define CSDL__LOAD_FIELD_STRING(field_name, control_idc) \
	SetDlgItemText(control_idc, AdoFldString(pflds, field_name, ""));

#define CSDL__LOAD_FIELD_VAR(field_name, control_idc) \
	SetDlgItemVar(control_idc, pflds->Item[field_name]->Value);
///////////////////////////////////////////////////////////////////////////////////////

// Throws _com_error
void CSupportDlg::Load()
{
	_RecordsetPtr rs;
	//(e.lally 2008-07-09) PLID 30493 - Added entries for EMR Professional (vs EMR Enterprise) and a text field for the doctor names
	// (d.thompson 2009-08-11) - PLID 35170 - Added NexWeb domains
	// (z.manning 2009-10-16 15:49) - PLID 35749 - Added NexSync fields
	// (f.dinatale 2010-05-21) - PLID 38842 - Added a query to retrieve version history
	// (f.dinatale 2010-09-29) - PLID 40739 - Fixed invalid column reference
	// (d.thompson 2011-10-03) - PLID 45791 - Added Concurrent TS Licensing
	// (z.manning 2012-06-12 15:11) - PLID 50878 - Added iPad fields
	// (r.farnworth 2013-07-19) PLID 57522 - Added Licenced/Mid/Staff Provider Info
	// (z.manning 2014-01-31 14:25) - PLID 55147 - Added nuance dictation fields
	// (j.armen 2014-07-08 14:32) - PLID 57853 - Added Anchor Password / Anchor Caller Count
	// (b.cardillo 2015-05-21 09:11) - PLID 66127 - Keep track of the number of Iagnosis Providers
	// (z.manning 2015-06-17 14:48) - PLID 66278
	// (b.eyers - 2016-03-03) - PLID 68480 - Subscription Local Hosted
	// (r.gonet 2016-05-10) - NX-100478 - Added AzureRemoteApp
	rs = CreateParamRecordset(GetRemoteData(), R"(
SELECT
	NexCloud, NexCloudVPN, NexCloudUsers, PersonID, LicenseBought, LicenseUsed, LicensePending,
	ConcurrentTSLicensesBought, ConcurrentTSLicensesUsed, ConcurrentTSLicensesPending, DoctorsBought, DoctorsUsed,
	DoctorsPending, PalmPilotBought, PalmPilotUsed, PalmPilotPending, Patients, Scheduler, Billing, HCFA, Letters,
	Quotes, Marketing, Inventory, EBilling, MirrorLink, InformLink, UnitedLink, QuickBooksLink, UnicornLink, PurchAgrSign,
	LicAgrSign, InstalledOn, Trainer, InvoiceDate, SupportExpires, Notes, PCA, PCAIP, Login,
	Password, ServerName, ServerOS, WorkOS, ServerPath, MirrorPath, InformPath,
	UnitedPath, NxServerIP, PalmPath, PracEmail, AllowExpiredSupport, Rating, RateDate,
	PPCBought, PPCUsed, PPCPending, LicenseKey, NexTrack, NexForms, NexWeb, ASCModule, NexSpa, NexVoice, EBillingType,
	ASPSForms, ASPSPurchDate, Retention, AMAVersion, AMAInstalled, EStatementType, DatabasesBought, DatabasesUsed,
	DatabasesPending, VersionCurrent, LicenseActivated, LicenseActivatedBy, LicenseActivatedOn, PracticeType,
	MaxPatientCount, PracticeUsageCount, LastShadowedVersion, AllowShadow, EMRProvidersBought, EMRProvidersUsed, EMRProvidersPending,
	EMRPro, EMRProDoctorNames, NexWebAddtlDomainsBought, NexWebAddtlDomainsUsed, NexWebAddtlDomainsPending,
	NexSyncBought, NexSyncUsed, NexSyncPending, VersionHistoryQ.Username, VersionHistoryQ.UpgradeDate,
	IpadBought, IpadUsed, IpadPending, IpadEnabled,
	ERxLicPrescribersPending, ERxLicPrescribersBought, ERxLicPrescribersUsed,
	ERxMidPrescribersPending, ERxMidPrescribersBought, ERxMidPrescribersUsed,
	ERxStaffPrescribersPending, ERxStaffPrescribersBought, ERxStaffPrescribersUsed,
	NuanceBought, NuanceUsed, NuancePending,
	AnchorPassword, AnchorCallerCount, SubscriptionLocalHosted 
	, IagnosisProviderCount
	, PortalProvidersBought, PortalProvidersUsed, PortalProvidersPending, NexERxIDPTransactionIDs, NewCropIDPTransactionIDs, 
	AzureRemoteApp
FROM NxClientsT
INNER JOIN PersonT ON NxClientsT.PersonID = PersonT.ID
LEFT JOIN (
	SELECT TOP 1
		VersionHistoryT.ClientID AS ClientID, 
		UsersT.Username AS Username, 
		VersionHistoryT.Date AS UpgradeDate
	FROM VersionHistoryT
	INNER JOIN UsersT ON VersionHistoryT.UserID = UsersT.PersonID
	INNER JOIN ReleasedVersionsT ON VersionHistoryT.VerID = ReleasedVersionsT.ID
	WHERE VersionHistoryT.ClientID = {INT}
	ORDER BY VersionHistoryT.Date DESC
) AS VersionHistoryQ ON VersionHistoryQ.ClientID = PersonT.ID
WHERE PersonID = {INT})", m_id, m_id);

	FieldsPtr pflds = rs->Fields;
	LoadNexCloudStatus(AdoFldBool(rs,"NexCloud",FALSE));
	//start the fun
	CSDL__LOAD_FIELD_STRING(	"LicenseBought",		IDC_B_LIC);
	CSDL__LOAD_FIELD_STRING(	"LicenseUsed",			IDC_U_LIC);
	CSDL__LOAD_FIELD_VAR(	"LicensePending",	IDC_P_LIC);
	// (z.manning 2012-06-12 15:12) - PLID 50878 - iPad fields
	CSDL__LOAD_FIELD_STRING(	"IpadBought",		IDC_IPAD_BOUGHT);
	CSDL__LOAD_FIELD_STRING(	"IpadUsed",			IDC_IPAD_INUSE);
	CSDL__LOAD_FIELD_VAR(	"IpadPending",	IDC_IPAD_PENDING);
	// (d.thompson 2011-10-03) - PLID 45791
	CSDL__LOAD_FIELD_STRING(	"ConcurrentTSLicensesBought",		IDC_B_CONC_TS_LIC);
	CSDL__LOAD_FIELD_STRING(	"ConcurrentTSLicensesUsed",			IDC_U_CONC_TS_LIC);
	CSDL__LOAD_FIELD_VAR(	"ConcurrentTSLicensesPending",	IDC_P_CONC_TS_LIC);

	CSDL__LOAD_FIELD_STRING(	"DoctorsBought",		IDC_B_DOC);
	CSDL__LOAD_FIELD_STRING(	"DoctorsUsed",			IDC_U_DOC);
	CSDL__LOAD_FIELD_VAR(	"DoctorsPending",	IDC_P_DOC);

	// (z.manning 2009-10-16 15:50) - PLID 35749 - NexSync
	CSDL__LOAD_FIELD_STRING(	"NexSyncBought",	IDC_B_NEXSYNC);
	CSDL__LOAD_FIELD_STRING(	"NexSyncUsed",		IDC_U_NEXSYNC);
	CSDL__LOAD_FIELD_VAR(	"NexSyncPending",		IDC_P_NEXSYNC);

	// (z.manning 2014-01-31 14:07) - PLID 55147
	CSDL__LOAD_FIELD_STRING(	"NuanceBought",		IDC_B_DICTATION);
	CSDL__LOAD_FIELD_STRING(	"NuanceUsed",		IDC_U_DICTATION);
	CSDL__LOAD_FIELD_VAR(		"NuancePending",	IDC_P_DICTATION);

	CSDL__LOAD_FIELD_STRING(	"PalmPilotBought",		IDC_B_PALM);
	CSDL__LOAD_FIELD_STRING(	"PalmPilotUsed",		IDC_U_PALM);
	CSDL__LOAD_FIELD_VAR(	"PalmPilotPending",	IDC_P_PALM);
	CSDL__LOAD_FIELD_STRING(	"PPCBought",			IDC_B_PPC);
	CSDL__LOAD_FIELD_STRING(	"PPCUsed",				IDC_U_PPC );
	CSDL__LOAD_FIELD_VAR(	"PPCPending",	IDC_P_PPC);
	CSDL__LOAD_FIELD_STRING(	"DatabasesBought",		IDC_B_DATABASES );
	CSDL__LOAD_FIELD_STRING(	"DatabasesUsed",		IDC_U_DATABASES );
	CSDL__LOAD_FIELD_VAR(	"DatabasesPending",	IDC_P_DATABASES);
	CSDL__LOAD_FIELD_STRING(	"EMRProvidersBought",		IDC_B_EMRPROV);	// (a.walling 2006-12-19 16:50) - PLID 23929
	CSDL__LOAD_FIELD_STRING(	"EMRProvidersUsed",			IDC_U_EMRPROV); // load EMR Provider licensing info into the dialog here
	CSDL__LOAD_FIELD_VAR(	"EMRProvidersPending",	IDC_P_EMRPROV);
	CSDL__LOAD_FIELD_VAR(	"EMRPro", IDC_EMRPRO);
	CSDL__LOAD_FIELD_STRING(	"EMRProDoctorNames", IDC_EMRPRO_DR_NAMES);		// (e.lally 2008-07-09) PLID 30493
	// (d.thompson 2009-08-11) - PLID 35170 - NexWeb domains
	CSDL__LOAD_FIELD_STRING(	"NexWebAddtlDomainsBought", IDC_B_NEXWEB_DOMAINS);
	CSDL__LOAD_FIELD_STRING(	"NexWebAddtlDomainsUsed", IDC_U_NEXWEB_DOMAINS);
	CSDL__LOAD_FIELD_VAR(	"NexWebAddtlDomainsPending", IDC_P_NEXWEB_DOMAINS);
	CSDL__LOAD_FIELD_STRING(	"NexCloudVPN", IDC_VPN);
	CSDL__LOAD_FIELD_STRING(	"NexCloudUsers", IDC_Users);
	// (r.farnworth 2013-07-2013) PLID 57522
	CSDL__LOAD_FIELD_STRING(	"ERxLicPrescribersBought",		IDC_B_LICPRES);
	CSDL__LOAD_FIELD_STRING(	"ERxLicPrescribersUsed",			IDC_U_LICPRES);
	CSDL__LOAD_FIELD_VAR(	"ERxLicPrescribersPending",	IDC_P_LICPRES);
	CSDL__LOAD_FIELD_STRING(	"ERxMidPrescribersBought",		IDC_B_MIDPRES);
	CSDL__LOAD_FIELD_STRING(	"ERxMidPrescribersUsed",			IDC_U_MIDPRES);
	CSDL__LOAD_FIELD_VAR(	"ERxMidPrescribersPending",	IDC_P_MIDPRES);
	CSDL__LOAD_FIELD_STRING(	"ERxStaffPrescribersBought",		IDC_B_STAFFPRES);
	CSDL__LOAD_FIELD_STRING(	"ERxStaffPrescribersUsed",			IDC_U_STAFFPRES);
	CSDL__LOAD_FIELD_VAR(	"ERxStaffPrescribersPending",	IDC_P_STAFFPRES);
	// (z.manning 2015-06-17 14:28) - PLID 66278 - Portal providers
	CSDL__LOAD_FIELD_STRING("PortalProvidersBought", IDC_B_PORTAL_PROVIDERS);
	CSDL__LOAD_FIELD_STRING("PortalProvidersUsed", IDC_U_PORTAL_PROVIDERS);
	CSDL__LOAD_FIELD_VAR("PortalProvidersPending", IDC_P_PORTAL_PROVIDERS);


	// (f.dinatale 2010-05-24) - PLID 38842 - Version history
	CSDL__LOAD_FIELD_STRING(	"Username", IDC_UPDATED_BY);
	COleDateTime dtLastUpdate = AdoFldDateTime(pflds, "UpgradeDate", COleDateTime(g_cvarNull));
	if(dtLastUpdate.GetStatus() == COleDateTime::valid){
		SetDlgItemText(IDC_UPDATED_ON, FormatDateTimeForInterface(dtLastUpdate, NULL, dtoDate));
	}else{
		SetDlgItemText(IDC_UPDATED_ON, "");
	}


	//DRT 5/11/2004 - Requery the new datalist instead of checking all those boxes
	RequeryItemList();


	// (f.dinatale 2010-05-21) - PLID 38842 - Removed Shadow support
	//CSDL__LOAD_FIELD_BOOL(		"ShadowSetup",			IDC_SHADOW_SETUP);
	CSDL__LOAD_FIELD_BOOL(		"AllowExpiredSupport",	IDC_ALLOW_SUPPORT_CHECK);

	CSDL__LOAD_FIELD_VAR(		"PurchAgrSign",			IDC_PURCH_AGR);
	CSDL__LOAD_FIELD_VAR(		"LicAgrSign",			IDC_LIC_AGR);
	CSDL__LOAD_FIELD_VAR(		"InstalledOn",			IDC_INSTALLED);
	CSDL__LOAD_FIELD_STRING(	"Trainer",				IDC_TRAINER);
	CSDL__LOAD_FIELD_STRING(	"SupportExpires",		IDC_SUPP_EXP);
	CSDL__LOAD_FIELD_STRING(	"Notes",				IDC_SUPPORT_NOTES);
	CSDL__LOAD_FIELD_STRING(	"PCA",					IDC_PCA);
	CSDL__LOAD_FIELD_STRING(	"PCAIP",				IDC_PCA_IP);
	CSDL__LOAD_FIELD_STRING(	"Login",				IDC_LOGIN);
	CSDL__LOAD_FIELD_STRING(	"Password",				IDC_PASSWORD);
	CSDL__LOAD_FIELD_STRING(	"ServerName",			IDC_SERV_NAME);
	CSDL__LOAD_FIELD_STRING(	"ServerOS",				IDC_SERV_OS);
	CSDL__LOAD_FIELD_STRING(	"WorkOS",				IDC_WS_OS);
	// (f.dinatale 2010-05-21) - PLID 38842 - Removed and added new fields
	//CSDL__LOAD_FIELD_STRING(	"VerNeeded",			IDC_VER_NEEDED);
	//CSDL__LOAD_FIELD_STRING(	"VerSent",				IDC_VER_SENT);
	//CSDL__LOAD_FIELD_STRING(	"VerPrev",				IDC_VER_PREV);
	CSDL__LOAD_FIELD_STRING(	"ServerPath",			IDC_PATH_SERVER);
	CSDL__LOAD_FIELD_STRING(	"NxServerIP",			IDC_NXSERVER_IP);
	CSDL__LOAD_FIELD_STRING(	"PracEmail",			IDC_PRAC_EMAIL);
	CSDL__LOAD_FIELD_VAR(	"LicenseKey",			IDC_LICENSE_KEY);
	CSDL__LOAD_FIELD_STRING(	"AMAVersion",			IDC_AMA_VERSION);

	CSDL__LOAD_FIELD_BOOL(	"LicenseActivated",	IDC_IS_LICENSE_ACTIVE);
	CSDL__LOAD_FIELD_STRING(	"LicenseActivatedBy", IDC_LICENSE_ACTIVATED_BY);

	CSDL__LOAD_FIELD_VAR(	"MaxPatientCount", IDC_MAX_PATIENT_COUNT);
	CSDL__LOAD_FIELD_VAR(	"PracticeUsageCount", IDC_MAX_PRACTICE_COUNT);
	// (b.cardillo 2015-05-21 09:11) - PLID 66127 - Keep track of the number of Iagnosis Providers
	CSDL__LOAD_FIELD_VAR(	"IagnosisProviderCount", IDC_IAGNOSIS_COUNT);
	// (b.savon 2016-04-08) - NX-100086 - Load IDP Transaction IDs
	CSDL__LOAD_FIELD_STRING("NewCropIDPTransactionIDs", IDC_NEWCROP_IDP_ID_EDIT);
	CSDL__LOAD_FIELD_STRING("NexERxIDPTransactionIDs", IDC_NEXERX_IDP_ID_EDIT);

	// (f.dinatale 2010-05-21) - PLID 38842 - Removed Shadow support
	//CSDL__LOAD_FIELD_BOOL(	"AllowShadow", IDC_ALLOW_SHADOW);

	// (z.manning 2012-06-13 08:37) - PLID 50878 - Load the iPad enabled check
	SupportItemStatus eIpadStatus = (SupportItemStatus)AdoFldLong(pflds, "IpadEnabled");
	if(eIpadStatus == sisBought || eIpadStatus == sisActive || eIpadStatus == sisActiveNotBought) {
		CheckDlgButton(IDC_ENABLE_IPADS_CHECK, BST_CHECKED);
	}
	else {
		CheckDlgButton(IDC_ENABLE_IPADS_CHECK, BST_UNCHECKED);
	}

	// (r.gonet 2016-05-10) - NX-100478 - Load the Azure RemoteApp checkbox value.
	SupportItemStatus eAzureRemoteAppStatus = (SupportItemStatus)AdoFldBool(pflds, "AzureRemoteApp");
	if (eAzureRemoteAppStatus == sisBought || eAzureRemoteAppStatus == sisActive || eAzureRemoteAppStatus == sisActiveNotBought) {
		CheckDlgButton(IDC_AZURE_REMOTE_APP_CHECK, BST_CHECKED);
	} else {
		CheckDlgButton(IDC_AZURE_REMOTE_APP_CHECK, BST_UNCHECKED);
	}

	// (b.eyers - 2016-03-03) - PLID 68480 - Subscription Local Hosted
	SupportItemStatus nSubLocalHosted = (SupportItemStatus)AdoFldLong(pflds, "SubscriptionLocalHosted");
	if (nSubLocalHosted == sisBought) {
		CheckDlgButton(IDC_SUBSCRIPTION_LOCAL_HOSTED, BST_CHECKED);
	}
	else {
		CheckDlgButton(IDC_SUBSCRIPTION_LOCAL_HOSTED, BST_UNCHECKED);
	}

	//load the ebilling type dropdown
	if(pflds->Item["EBillingType"]->Value.vt == VT_I4)
		m_pEbillingType->SetSelByColumn(0, pflds->Item["EBillingType"]->Value);
	else
		m_pEbillingType->CurSel = -1;

	//load the asps forms datalist
	if(pflds->Item["ASPSForms"]->Value.vt == VT_I4)
		m_pASPSForms->SetSelByColumn(0, pflds->Item["ASPSForms"]->Value);
	else
		m_pASPSForms->PutCurSel(sriNoRow);

	//Load the practice type
	m_pPracticeType->SetSelByColumn(0, pflds->Item["PracticeType"]->Value);

	_variant_t var = pflds->Item["LicenseActivatedOn"]->Value;
	if(var.vt == VT_DATE) {
		m_pLicenseActivatedDate->SetDateTime(VarDateTime(var));
	}
	else
		m_pLicenseActivatedDate->Clear();

	var = pflds->Item["AMAInstalled"]->Value;
	if(var.vt == VT_DATE) {
		m_pAMAInstalled->SetDateTime(VarDateTime(var));
	}
	else
		m_pAMAInstalled->Clear();

	var = pflds->Item["ASPSPurchDate"]->Value;
	if(var.vt == VT_DATE) {
		m_pASPSPurchDate->SetDateTime(VarDateTime(var));
	}
	else
		m_pASPSPurchDate->Clear();

	//load the estatement type dropdown
	if(pflds->Item["EStatementType"]->Value.vt == VT_I4)
		m_pEStatementType->SetSelByColumn(0, pflds->Item["EStatementType"]->Value);
	else
		m_pEStatementType->CurSel = -1;

	//DRT 12/31/2004 - PLID 15160 - Load the current version
	var = pflds->Item["VersionCurrent"]->Value;
	if(var.vt == VT_I4) 
		m_pCurVersion->TrySetSelByColumn(0, VarLong(var));
	else
		m_pCurVersion->PutCurSel(-1);

	// (j.armen 2013-10-03 07:20) - PLID 57853 - Save password and caller count in members
	m_nAnchorCallerCount = AdoFldByte(pflds, "AnchorCallerCount");
	m_strAnchorPassword = DecryptStringFromVariant(AdoFldVar(pflds, "AnchorPassword", g_cvarNull));
	
	// Push bound data members to dialog
	UpdateData(FALSE);
}

BOOL CSupportDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try
	{
		switch (HIWORD(wParam))
		{
		case EN_CHANGE:
			m_changed = true;
			break;

		case EN_KILLFOCUS:
			Save(LOWORD(wParam));
			break;

		default:
			break;
		}
	}
	NxCatchAll(__FUNCTION__);
	
	return CPatientDialog::OnCommand(wParam, lParam);
}

void CSupportDlg::StoreDetails()
{
	try
	{
		if (m_changed)
		{	Save (GetFocus()->GetDlgCtrlID());
			m_changed = false;
		}
	}NxCatchAll("Error in StoreDetails");
}

BEGIN_EVENTSINK_MAP(CSupportDlg, CPatientDialog)
    //{{AFX_EVENTSINK_MAP(CSupportDlg)
	ON_EVENT(CSupportDlg, IDC_EBILLING_TYPE, 16 /* SelChosen */, OnSelChosenEbillingType, VTS_I4)
	ON_EVENT(CSupportDlg, IDC_EBILLING_TYPE, 1 /* SelChanging */, OnSelChangingEbillingType, VTS_PI4)
	ON_EVENT(CSupportDlg, IDC_SUPPORT_ITEMS_LIST, 10 /* EditingFinished */, OnEditingFinishedSupportItemsList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CSupportDlg, IDC_SUPPORT_ITEMS_LIST, 9 /* EditingFinishing */, OnEditingFinishingSupportItemsList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CSupportDlg, IDC_ASPS_FORMS, 16 /* SelChosen */, OnSelChosenAspsForms, VTS_I4)
	ON_EVENT(CSupportDlg, IDC_AAO_FORMS, 16 /* SelChosen */, OnSelChosenAaoForms, VTS_I4)
	ON_EVENT(CSupportDlg, IDC_ASPS_PURCH_DATE, 1 /* KillFocus */, OnKillFocusAspsPurchDate, VTS_NONE)
	//r.wilson Oct 5 2011 PLID 45836
	ON_EVENT(CSupportDlg, IDC_AAO_PURCH_DATE, 1 /* KillFocus */, OnKillFocusAaoPurchDate, VTS_NONE)
	ON_EVENT(CSupportDlg, IDC_AMA_INSTALLED_ON_DATE, 1 /* KillFocus */, OnKillFocusAmaInstalledOnDate, VTS_NONE)
	ON_EVENT(CSupportDlg, IDC_ESTATEMENT_TYPE, 16 /* SelChosen */, OnSelChosenEstatementType, VTS_I4)
	ON_EVENT(CSupportDlg, IDC_ESTATEMENT_TYPE, 1 /* SelChanging */, OnSelChangingEstatementType, VTS_PI4)
	ON_EVENT(CSupportDlg, IDC_VERSION_CUR, 16 /* SelChosen */, OnSelChosenVersionCur, VTS_I4)
	ON_EVENT(CSupportDlg, IDC_PRACTICE_TYPE, 1 /* SelChanging */, OnSelChangingPracticeType, VTS_PI4)
	ON_EVENT(CSupportDlg, IDC_PRACTICE_TYPE, 16 /* SelChosen */, OnSelChosenPracticeType, VTS_I4)
	ON_EVENT(CSupportDlg, IDC_LICENSE_ACTIVATED_ON, 1 /* KillFocus */, OnKillFocusLicenseActivatedOn, VTS_NONE)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


void CSupportDlg::OnSupportFile() 
{
	//run the report (it's in the main frame)
	GetMainFrame()->NxClientReport();
}

void CSupportDlg::OnSelChosenEbillingType(long nRow) 
{
	if(nRow == -1)
		return;

	try {
		long nVal = VarLong(m_pEbillingType->GetValue(nRow, 0));

		CString str;
		if(nVal == -1)
			// (j.luckoski 2012-06-06 14:24) - PLID 50114 - Removed the NULL and relaced it with "" because of params
			str.Format("");
		else
			str.Format("%li", nVal);

		//for auditing
		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		_RecordsetPtr prs = CreateParamRecordset("SELECT EBillingType, Type FROM NxClientsT LEFT JOIN InternalEbillingTypes "
			"ON NxClientsT.EBillingType = InternalEbillingTypes.ID WHERE PersonID = {INT}", m_id);

		CAuditSupportDlg dlg(this);
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				//reset the selection
				if(!prs->eof) {
					m_pEbillingType->SetSelByColumn(0, AdoFldLong(prs, "EBillingType", -1));
				}
				else
					m_pEbillingType->PutCurSel(sriNoRow);
				return;
			}
		}

		CString strOld, strNew;
		if(!prs->eof)
			strOld = AdoFldString(prs, "Type", "");
		if(nVal != -1)
			strNew = VarString(m_pEbillingType->GetValue(nRow, 1), "");

		SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "EBilling Type", strOld, strNew,  _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils

		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		ExecuteParamSql("UPDATE NxClientsT SET EBillingType = {STRING} WHERE PersonID = {INT}", str, m_id);
	} NxCatchAll("Error selecting ebilling type.");
}

void CSupportDlg::OnSelChangingEbillingType(long FAR* nNewSel) 
{
	if(*nNewSel == -1)
		*nNewSel = 0;
}

void CSupportDlg::AddRowToList(CString strField, CString strName, long nStatus, long nUsageCount, COleDateTime dtExpiration)
{
	IRowSettingsPtr pRow = m_pItemList->GetRow(sriNoRow);
	pRow->PutValue(slcField, _bstr_t(strField));
	pRow->PutValue(slcModule, _bstr_t(strName));
	pRow->PutValue(slcStatus, nStatus);
	pRow->PutValue(slcUsageCount, nUsageCount);
	if(dtExpiration == COleDateTime(1899,12,30,0,0,0)) {
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pRow->PutValue(slcExpiration, varNull);
	}
	else {
		pRow->PutValue(slcExpiration, _variant_t(dtExpiration, VT_DATE));
	}

	m_pItemList->AddRow(pRow);
}

#define ADD_ROW(status, name)	AddRowToList(status, name, VarLong(fields->Item[_bstr_t(status)]->Value, 0), \
VarLong(fields->Item[_bstr_t(CString(status)+CString("Usage"))]->Value, 0), \
AdoFldDateTime(fields, CString(status)+CString("Expiration"), COleDateTime(1899,12,30,0,0,0)))

//throws _com_error
void CSupportDlg::RequeryItemList()
{
	m_pItemList->Clear();
		
	// (j.jones 2007-06-28 15:19) - PLID 23951 - added licensing for E-Remittance
	// (j.jones 2007-06-28 15:19) - PLID 23952 - added licensing for E-Eligibility
	// (a.walling 2007-07-30 12:55) - PLID 26758 - added licensing for CC Processing
	// (a.walling 2008-02-14 13:04) - PLID 28388 - added licensing for Advanced Inventory
	// (a.walling 2008-07-16 15:44) - PLID 30734 - added licensing for Transcriptions
	//DRT 7/31/2008 - PLID 30890 - Added licensing for Faxing and Barcode Reading
	//DRT 10/9/2008 - PLID 31491 - Added licensing for Standard modules
	//DRT 10/22/2008 - PLID 31788 - Added licensing for Cycle of Care
	//DRT 11/20/2008 - PLID 32085 - Added licensing for ePrescribe
	// (d.thompson 2009-03-24) - PLID 33583 - Added licensing for NewCrop
	// (d.thompson 2009-08-11) - PLID 35170 - Added licensing for NexWeb Leads, Portal, Domains
	//TES 10/7/2009 - PLID 35802 - Added FirstDataBank licensing
	// (c.haag 2009-10-28 10:34) - PLID 36067 - Added NexPhoto licensing
	// (j.jones 2009-11-04 08:56) - PLID 36135 - added TOPS license
	// (c.haag 2010-06-01 15:40) - PLID 38965 - Added CellTrust licensing
	// (j.gruber 2010-06-02 14:39) - PLID 38935 - Added Bold licensing
	// (c.haag 2010-06-29 17:08) - PLID 39402 - Frames licensing
	// (c.haag 2010-06-30 09:00) - PLID 37986 - Device Import
	// (d.lange 2010-09-08 10:00) - PLID 40370 - Chase CC Processing
	//TES 12/9/2010 - PLID 41701 - Glasses Orders
	// (f.dinatale 2010-09-29) - PLID 40739 - Fixed invalid column reference
	// (z.manning 2011-01-11 10:55) - PLID 42057 - Patient check-in
	//(a.wilson 2011-8-30) PLID 44686 - PQRS Licensing
	// (a.wilson 2011-11-8) PLID 46335 - Eyemaginations license
	// (z.manning 2011-12-05 10:10) - PLID 44162 - NexWeb EMR license
	// (d.singleton 2012-03-23 11:18) - PLID 49086 - added codecorrect license
	// (j.armen 2012-03-27 17:00) - PLID 49244 - Recall
	// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
	// (j.armen 2012-05-30 15:40) - PLID 50688 - NexEMRCosmetic
	// (b.spivey - November 25th, 2013) - PLID 59590 - Direct Message. 
	// (a.wilson 2014-07-11 14:46) - PLID 62516 - VisionPayments
	// (b.spivey - August 5th, 2014) - PLID 62926 
	// (z.manning 2015-05-12 15:07) - PLID 65961 - StateASCReports
	// (z.manning 2015-06-17 09:11) - PLID 66407 - MPVPatients
	// (z.manning 2015-07-21 13:11) - PLID 66599 - Added Integrated Credit Card Processing
	// (r.farnworth 2015-11-11 13:55) - PLID 67567 - MPVMessaging
	_RecordsetPtr prs = CreateParamRecordset("SELECT NexCloudVPN,NexCloudUsers, NexCloud,PersonID, LicenseBought, LicenseUsed, LicensePending, DoctorsBought, DoctorsUsed, "
		"DoctorsPending, PalmPilotBought, PalmPilotUsed, PalmPilotPending, Patients, Scheduler, Billing, CCProcessing, HCFA, Letters, "
		"Quotes, Marketing, Inventory, AdvInventory, EBilling, ERemittance, EEligibility, CareCreditLink, MirrorLink, InformLink, UnitedLink, QuickBooksLink, UnicornLink, PurchAgrSign, "
		"LicAgrSign, InstalledOn, Trainer, InvoiceDate, SupportExpires, Notes, PCA, PCAIP, Login, "
		"Password, ServerName, ServerOS, WorkOS, ServerPath, MirrorPath, InformPath, "
		"UnitedPath, NxServerIP, PalmPath, PracEmail, AllowExpiredSupport, Rating, RateDate, "
		"PPCBought, PPCUsed, PPCPending, LicenseKey, NexTrack, NexForms, NexWeb, ASCModule, NexSpa, NexVoice, EBillingType, "
		"ASPSForms, ASPSPurchDate, Retention, Labs, AMAVersion, AMAInstalled, EStatementType, DatabasesBought, DatabasesUsed, "
		"DatabasesPending, VersionCurrent, CustomRecords, NexEMR, Transcriptions, Faxing, BarcodeRead, HL7, PatientsUsage, SchedulerUsage, BillingUsage, CCProcessingUsage, HCFAUsage, "
		"LettersUsage, QuotesUsage, MarketingUsage, NexTrackUsage, InventoryUsage, AdvInventoryUsage, NexFormsUsage, EBillingUsage, "
		"ERemittanceUsage, EEligibilityUsage, CareCreditLinkUsage, MirrorLinkUsage, InformLinkUsage, UnitedLinkUsage, "
		"QuickBooksLinkUsage, UnicornLinkUsage, HL7Usage, NexWebUsage, ASCModuleUsage, NexSpaUsage, NexEMRUsage, TranscriptionsUsage, FaxingUsage, BarcodeReadUsage, NexVoiceUsage, CustomRecordsUsage, RetentionUsage, LabsUsage, "
		"PatientsExpiration, SchedulerExpiration, BillingExpiration, CCProcessingExpiration, HCFAExpiration, LettersExpiration, QuotesExpiration, MarketingExpiration, "
		"NexTrackExpiration, InventoryExpiration, AdvInventoryExpiration, NexFormsExpiration, EBillingExpiration, ERemittanceExpiration, EEligibilityExpiration, CareCreditLinkExpiration, "
		"MirrorLinkExpiration, InformLinkExpiration, UnitedLinkExpiration, QuickBooksLinkExpiration, UnicornLinkExpiration, HL7Expiration, "
		"NexWebExpiration, ASCModuleExpiration, NexSpaExpiration, NexEMRExpiration, TranscriptionsExpiration, FaxingExpiration, BarcodeReadExpiration, NexVoiceExpiration, "
		"CustomRecordsExpiration, RetentionExpiration, LabsExpiration, SchedStandard, LWStandard, EMRStandard,  "
		"SchedStandardUsage, LWStandardUsage, EMRStandardUsage, SchedStandardExpiration, LWStandardExpiration, EMRStandardExpiration, "
		"CycleOfCare, CycleOfCareUsage, CycleOfCareExpiration, ePrescribe, ePrescribeUsage, ePrescribeExpiration, "
		"NewCrop, NewCropUsage, NewCropExpiration, NexWebLeads, NexWebLeadsUsage, NexWebLeadsExpiration, " 
		"NexWebPortal, NexWebPortalUsage, NexWebPortalExpiration, FirstDataBank, FirstDataBankUsage, FirstDataBankExpiration, "
		"NexPhoto, NexPhotoUsage, NexPhotoExpiration, TOPSLink, TOPSLinkUsage, TOPSLinkExpiration, "
		"CellTrust, CellTrustUsage, CellTrustExpiration, Bold, BoldUsage, BoldExpiration,  "
		"Frames, FramesUsage, FramesExpiration, DeviceImport, DeviceImportUsage, DeviceImportExpiration, "
		"ChaseCCProcessing, ChaseCCProcessingUsage, ChaseCCProcessingExpiration, GlassesOrders, GlassesOrdersUsage, GlassesOrdersExpiration, "
		"PatientCheckIn, PatientCheckInUsage, PatientCheckInExpiration, Eyemaginations, EyemaginationsUsage, EyemaginationsExpiration, "
		"NexWebEmr, NexWebEmrUsage, NexWebEmrExpiration, CodeCorrect, CodeCorrectUsage, CodeCorrectExpiration, Recall, RecallUsage, RecallExpiration, \r\n"
		"NexEMRCosmetic, NexEMRCosmeticUsage, NexEMRCosmeticExpiration, \r\n"
		"DirectMessage, DirectMessageUsage, DirectMessageExpiration, \r\n" 
		"VisionPayments, VisionPaymentsUsage, VisionPaymentsExpiration, \r\n"
		"LockboxPayments, LockboxPaymentsUsage, LockboxPaymentsExpiration, \r\n"
		"StateASCReports, StateASCReportsUsage, StateASCReportsExpiration, \r\n"
		"MPVPatients, MPVPatientsUsage, MPVPatientsExpiration, \r\n"
		"ICCP, ICCPUsage, ICCPExpiration, \r\n"
		"MPVMessaging, MPVMessagingUsage, MPVMessagingExpiration, \r\n"
		"ActiveDirectoryLogin, ActiveDirectoryLoginUsage, ActiveDirectoryLoginExpiration, \r\n"
		"EnterpriseReporting, EnterpriseReportingUsage, EnterpriseReportingExpiration \r\n"
		"FROM NxClientsT INNER JOIN PersonT ON NxClientsT.PersonID = PersonT.ID WHERE PersonID = {INT}", m_id);

	FieldsPtr fields = prs->Fields;
	

	//manually add everything
	//ADD_ROW(DATABASEFIELD, DISPLAYFIELD)
	ADD_ROW("Patients", "Patients");
	ADD_ROW("Scheduler", "Scheduler");
	ADD_ROW("Billing", "Cosmetic Billing");
	// (d.lange 2010-12-16 14:59) - PLID 40312 - Renamed to 'Intuit CC Processing' from 'Credit Card Processing'
	ADD_ROW("CCProcessing", "Intuit CC Processing");
	ADD_ROW("HCFA", "HCFA");
	ADD_ROW("Letters", "LetterWriting");
	ADD_ROW("Quotes", "Quotes");
	ADD_ROW("Marketing", "Marketing");
	ADD_ROW("NexTrack", "NexTrak");
	ADD_ROW("Inventory", "Inventory");
	// (d.thompson 2009-01-26) - PLID 32851 - Renamed to 'Consignment and Allocation' from 'Advanced Inventory'
	ADD_ROW("AdvInventory", "Consignment and Allocation");
	ADD_ROW("NexForms", "NexForms");
	ADD_ROW("EBilling", "EBilling");
	ADD_ROW("ERemittance", "E-Remittance");
	ADD_ROW("EEligibility", "E-Eligibility");
	ADD_ROW("CareCreditLink", "CareCredit Link");
	ADD_ROW("MirrorLink", "Mirror Link");
	ADD_ROW("InformLink", "Inform Link");
	ADD_ROW("UnitedLink", "United Link");
	ADD_ROW("QuickBooksLink", "QuickBooks Link");
	ADD_ROW("UnicornLink", "ChaseHealthAdvance Link");
	ADD_ROW("HL7", "HL7");
//	ADD_ROW("NexWeb", "NxWeb");
	ADD_ROW("ASCModule", "ASC");
	ADD_ROW("NexSpa", "NexSpa");
	ADD_ROW("NexEMR", "NexEMR");
	ADD_ROW("Transcriptions", "Transcription Parsing");
	//ADD_ROW("NexVoice", "NexVoice"); // (c.haag 2016-03-09) - PLID 68175 - Deprecated
	ADD_ROW("CustomRecords", "Custom Records");
	ADD_ROW("Retention", "Retention");
	ADD_ROW("Labs", "Labs");
	//DRT 7/31/2008 - PLID 30890 - Added Faxing and Barcode Reading
	ADD_ROW("Faxing", "Faxing");
	ADD_ROW("BarcodeRead", "Barcode Reading");
	//DRT 10/9/2008 - PLID 31491 - Added licensing for Standard modules
	ADD_ROW("SchedStandard", "Scheduler (Std)");
	// (d.thompson 2008-12-03) - PLID 32144 - Removed this module
	//ADD_ROW("LWStandard", "Letter Writing (Std)");
	ADD_ROW("EMRStandard", "EMR (Std)");
	//DRT 10/22/2008 - PLID 31788 - Added licensing for Cycle of Care
	ADD_ROW("CycleOfCare", "Cycle Of Care");
	//DRT 11/20/2008 - PLID 32085 - Added licensing for ePrescribe
	// (d.thompson 2009-06-10) - PLID 34591 - Renamed to eRx (SureScripts) for clarity since we have options.
	ADD_ROW("ePrescribe", "eRx (SureScripts)");
	// (d.thompson 2009-03-24) - PLID 33583 - Added licensing for NewCrop
	// (d.thompson 2009-06-10) - PLID 34591 - Renamed to eRx (NewCrop) for clarity since we have options.
	ADD_ROW("NewCrop", "eRx (NewCrop)");
	// (d.thompson 2009-08-11) - PLID 35170 - Added NexWeb objects
	ADD_ROW("NexWebLeads", "NexWeb Lead Generation");
	ADD_ROW("NexWebPortal", "NexWeb Patient Portal");
	ADD_ROW("NexWebEmr", "NexWeb EMR"); // (z.manning 2011-12-05 10:11) - PLID 44162
	//TES 10/7/2009 - PLID 35802 - Added FirstDataBank licensing
	ADD_ROW("FirstDataBank", "FirstDataBank Integration");
	// (c.haag 2009-10-28 10:34) - PLID 36067 - Added NexPhoto licensing
	ADD_ROW("NexPhoto", "NexPhoto");
	// (j.jones 2009-11-04 08:56) - PLID 36135 - added TOPS license
	ADD_ROW("TOPSLink", "TOPS Link");
	// (c.haag 2010-06-01 15:40) - PLID 38965 - Added CellTrust licensing
	// (z.manning 2010-12-02 09:19) - PLID 40643 - Renamed to NexReminder
	ADD_ROW("CellTrust", "NexReminder");
	// (j.gruber 2010-06-02 14:40) - PLID 38935 - Added Bold Licensing
	ADD_ROW("Bold", "Bold Link");
	// (c.haag 2010-06-29 17:08) - PLID 39402 - Frames
	ADD_ROW("Frames", "Frames");
	// (c.haag 2010-06-30 09:00) - PLID 37986 - Device Import
	ADD_ROW("DeviceImport", "Device Import");
	// (d.lange 2010-09-08 09:56) - PLID 40370 - Chase CC Processing
	ADD_ROW("ChaseCCProcessing", "Chase CC Processing");
	//TES 12/9/2010 - PLID 41701 - Glasses Orders
	// (j.dinatale 2012-05-11 10:14) - PLID 49078 - now called optical orders
	ADD_ROW("GlassesOrders", "Optical Orders");
	// (z.manning 2011-01-11 10:56) - PLID 42057 - Patient check-in
	ADD_ROW("PatientCheckIn", "Patient Check-in");
	// (a.wilson 2011-11-8) PLID 46335 - Eyemaginations license
	ADD_ROW("Eyemaginations", "Eyemaginations");
	// (d.singleton 2012-03-21 11:49) - PLID 49086 license for code correct
	ADD_ROW("CodeCorrect", "CodeCorrect");
	// (j.armen 2012-03-27 17:00) - PLID 49244 - Recall
	ADD_ROW("Recall", "Recall");
	// (j.armen 2012-05-30 14:08) - PLID 50688 - NexEMRCosmetic
	ADD_ROW("NexEMRCosmetic", "NexEMR Cosmetic");
	// (b.spivey - November 25th, 2013) - PLID 59590
	ADD_ROW("DirectMessage", "Direct Message");
	// (a.wilson 2014-07-11 14:47) - PLID 62516 - VisionPayments
	ADD_ROW("VisionPayments", "Vision Payments");
	// (b.spivey - August 5th, 2014) - PLID 62926 
	ADD_ROW("LockboxPayments", "Lockbox Payments"); 
	// (z.manning 2015-05-12 15:07) - PLID 65961
	ADD_ROW("StateASCReports", "State ASC Reports");
	// (z.manning 2015-06-17 09:12) - PLID 66407
	ADD_ROW("MPVPatients", "MyPatientVisit - Patients");
	// (z.manning 2015-07-21 13:13) - PLID 66599
	ADD_ROW("ICCP", "Integrated Credit Card Processing");
	// (r.farnworth 2015-11-11 15:45) - PLID 67567
	ADD_ROW("MPVMessaging", "MyPatientVisit - Enhanced Messaging");
	ADD_ROW("ActiveDirectoryLogin", "Active Directory Login");
	ADD_ROW("EnterpriseReporting", "Enterprise Reporting");
}

void CSupportDlg::OnEnableSupportItems() 
{
	
	try {
		BOOL bEnable = FALSE;
		if(m_bItemsEnabled == FALSE) {
			SetDlgItemText(IDC_ENABLE_SUPPORT_ITEMS, "En&able");
			m_bItemsEnabled = TRUE;
			m_bHideForNewClient = FALSE;
		}
		else { 
			//DRT 5/18/2004 - PLID 12413 - If this is a new client (defined as having no "New Client" audit, 
			//	no data in w/s bought or in use, no data in doctor bought or in use) then we give them an
			//	opportunity to not be prompted with the box each time.  Once disable is pressed or they
			//	close this module, they can never get that back for this client.
			//DRT 6/3/2005 - PLID 16636 - Changed the definition -- we no longer care about doctors, we just
			//	check to see if the workstation field is EITHER blank OR zero.
			// (j.luckoski 2012-03-26) - PLID 49127 - Made it so that if terminal server licenses were not 0 then
			// normal users could still enable the page and use features like active license.
			// (z.manning 2012-06-12 15:20) - PLID 50878 - Also added check for iPad licenses
			CString strWsB, strWsU, strCsB, strCuB, strIpadBought, strIpadUsed;
			GetDlgItemText(IDC_B_LIC, strWsB);
			GetDlgItemText(IDC_U_LIC, strWsU);
			GetDlgItemText(IDC_IPAD_BOUGHT, strIpadBought);
			GetDlgItemText(IDC_IPAD_INUSE, strIpadUsed);
			GetDlgItemText(IDC_B_CONC_TS_LIC, strCsB);
			GetDlgItemText(IDC_U_CONC_TS_LIC, strCuB);
			//if( ( (strWsB.IsEmpty() || strWsB == "0") && (strWsU.IsEmpty() || strWsU == "0") ) && ( (strCsB.IsEmpty() || strCsB == "0") && (strCsU.IsEmpty() || strCsU == "0") ) ) {
			if((strWsB.IsEmpty() || strWsB == "0") && (strWsU.IsEmpty() || strWsU == "0") && (strCsB.IsEmpty() || strCsB == "0") && (strCuB.IsEmpty() || strCuB == "0")
				&& (strIpadBought.IsEmpty() || strIpadBought == "0") && (strIpadUsed.IsEmpty() || strIpadUsed == "0")
				)
			{
				//there is no data, now check to see if we've audited it before
				if(!ReturnsRecords("SELECT * FROM NxSupportAuditT WHERE FieldChanged = 'New Client' AND PersonID = %li", m_id)) {
					//they must have permission for Bought to be able to do this.  If they don't, they're not 
					//	allowed to convert this client.
					//(c.copits 2010-06-25) PLID 33989 - Add the ability to flag certain accounts as "non-permissioned"
					if((GetCurrentUserPermissions(bioSupportBought) & sptWrite) || IsTestAccount()) {
						//this is indeed a new client!
						if(MsgBox(MB_YESNO, "This is a new client.  Would you like to hide all auditing messages for this time, and this time only?\r\n"
							"Once you close this module or press the disable button, the client will no longer be specified as a new client.") == IDYES) {
							//set our flag and audit the new client status
							m_bHideForNewClient = TRUE;
							SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "New Client", "", "New Client", ""); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils
						}
					}
					else {
						MsgBox("Since you do not have permission to change bought fields, and this is a new client, you will not be allowed to enable the fields to change.");
						return;
					}
				}
			}

			SetDlgItemText(IDC_ENABLE_SUPPORT_ITEMS, "Dis&able");
			bEnable = TRUE;
			m_bItemsEnabled = FALSE;
		}

		//(c.copits 2010-06-25) PLID 33989 - Add the ability to flag certain accounts as "non-permissioned"
		// Are we dealing with one of those "test" accounts?
		if (IsTestAccount()) {
			m_pItemList->ReadOnly = !bEnable;
		}
		// No, handle all other accounts.
		else {
			if(!GetCurrentUserPermissions(bioSupportBought, sptWrite) && !GetCurrentUserPermissions(bioSupportUsed, sptWrite)) {
				m_pItemList->ReadOnly = TRUE;
			}
			else {
				m_pItemList->ReadOnly = !bEnable;	//read only allows scrolling, disabled does not
			}
		}

		GetDlgItem(IDC_EBILLING_TYPE)->EnableWindow(bEnable);
		GetDlgItem(IDC_ASPS_FORMS)->EnableWindow(bEnable);
		//r.wilson Oct 5 2011 PLID 45836
		GetDlgItem(IDC_AAO_FORMS)->EnableWindow(bEnable);
		GetDlgItem(IDC_ASPS_PURCH_DATE)->EnableWindow(bEnable);
		//r.wilson Oct 5 2011 PLID 45836
		GetDlgItem(IDC_AAO_PURCH_DATE)->EnableWindow(bEnable);
		GetDlgItem(IDC_AMA_VERSION)->EnableWindow(bEnable);
		GetDlgItem(IDC_AMA_INSTALLED_ON_DATE)->EnableWindow(bEnable);
		GetDlgItem(IDC_ESTATEMENT_TYPE)->EnableWindow(bEnable);

		if(!IsTestAccount())
		{
			if(!GetCurrentUserPermissions(bioPurchaseLicenseAgreements, sptWrite)) //(d.singleton 2011-07-01 13:57) - PLID 43832 - add permissions to write to the purchase and license agreement fields in support tab, (split out from supportbought)
			{
				GetDlgItem(IDC_PURCH_AGR)->EnableWindow(FALSE);
				GetDlgItem(IDC_LIC_AGR)->EnableWindow(FALSE);
			}
			else
			{
				GetDlgItem(IDC_PURCH_AGR)->EnableWindow(bEnable);
				GetDlgItem(IDC_LIC_AGR)->EnableWindow(bEnable);
			}
		}
		else
		{
			GetDlgItem(IDC_PURCH_AGR)->EnableWindow(bEnable);
			GetDlgItem(IDC_LIC_AGR)->EnableWindow(bEnable);
		}


		//(c.copits 2011-10-04) PLID 28219 - Use guided dialog to edit licensing options
		//(d.singleton 2012-01-10 13:52) - PLID 28219 changed back to way it used to work, so when hitting enable button
		//you can still manually edit the bought, in use, and pending values ( if you have permission ) instead of forcing the new dialog
		EnableBoughtUsedLicenseFields(bEnable);

		//(c.copits 2010-06-25) PLID 33989 - Add the ability to flag certain accounts as "non-permissioned"
		if (IsTestAccount()) {	
			m_pItemList->GetColumn(slcUsageCount)->Editable = bEnable;
			m_pItemList->GetColumn(slcExpiration)->Editable = bEnable;
		}
	
		GetDlgItem(IDC_LICENSE_KEY)->EnableWindow(bEnable);
		GetDlgItem(IDC_IS_LICENSE_ACTIVE)->EnableWindow(bEnable);
		GetDlgItem(IDC_LICENSE_ACTIVATED_BY)->EnableWindow(bEnable);
		GetDlgItem(IDC_LICENSE_ACTIVATED_ON)->EnableWindow(bEnable);
		m_pPracticeType->ReadOnly = !bEnable;
		GetDlgItem(IDC_TRAINER)->EnableWindow(bEnable);
		// (f.dinatale 2010-05-21) - PLID 38842 - Removed Shadow support
		//GetDlgItem(IDC_ALLOW_SHADOW)->EnableWindow(bEnable);
		GetDlgItem(IDC_EMRPRO_DR_NAMES)->EnableWindow(bEnable);	// (e.lally 2008-07-09) PLID 30493 - Enable text field based on button

		// (b.savon 2016-04-08) - NX-100086 - Enable/Disable IDP Transaction Controls
		GetDlgItem(IDC_NEXERX_IDP_ID_EDIT)->EnableWindow(bEnable);
		GetDlgItem(IDC_NEWCROP_IDP_ID_EDIT)->EnableWindow(bEnable);
		
		//(c.copits 2011-10-04) PLID 28219 - Use guided dialog to edit licensing options
		// (d.singleton 2012-01-10 13:58) - PLID 28219 - changed to only check bioSupportBought as the dialog only lets you change bought fields
		// ie bought and pending,  in use will automatically adjust once their license is updated.
		// (b.eyers 2015-02-24) - PLID 64989 - Users with the Support Swap Licenses permission can also access this dialog
		if(GetCurrentUserPermissions(bioSupportBought, sptWrite) || GetCurrentUserPermissions(bioSupportSwapLicenses, sptWrite)) {

			GetDlgItem(IDC_CHANGE_LICENSES)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_CHANGE_LICENSES)->EnableWindow(FALSE);
		}
		// Also enable button if we're running on one of the permissioned test accounts.
		if (IsTestAccount()) {
			GetDlgItem(IDC_CHANGE_LICENSES)->EnableWindow(TRUE);
		}

	} NxCatchAll("Error enabling support items");
}

//(c.copits 2011-10-04) PLID 28219 - Use guided dialog to edit licensing options
void CSupportDlg::EnableBoughtUsedLicenseFields(BOOL bEnable)
{
	try
	{
		// (j.armen 2014-07-08 14:53) - PLID 57853 - Handle enabling of the anchor controls
		// (b.cardillo 2015-05-21 09:11) - PLID 66127 - Keep track of the number of Iagnosis Providers
		// (b.eyers - 2016-03-03) - PLID 68480 - Subscription Local Hosted
		// (r.gonet 2016-05-10) - NX-100478 - Added AzureRemoteApp
		UINT aryBoughtIDCs[] =
		{
			IDC_B_LIC, IDC_IPAD_BOUGHT, IDC_B_CONC_TS_LIC, IDC_B_DOC, IDC_B_PALM,
			IDC_B_PPC, IDC_B_DATABASES, IDC_B_EMRPROV, IDC_EMRPRO, IDC_B_NEXWEB_DOMAINS,
			IDC_B_LICPRES, IDC_B_MIDPRES, IDC_B_STAFFPRES, IDC_INSTALLED, IDC_SUPP_EXP,
			IDC_P_LIC, IDC_IPAD_PENDING, IDC_P_CONC_TS_LIC, IDC_P_DOC, IDC_P_PALM,
			IDC_P_PPC, IDC_P_DATABASES, IDC_P_EMRPROV, IDC_P_NEXWEB_DOMAINS, IDC_P_LICPRES,
			IDC_P_MIDPRES, IDC_P_STAFFPRES, IDC_ALLOW_SUPPORT_CHECK, IDC_MAX_PATIENT_COUNT,
			IDC_MAX_PRACTICE_COUNT, IDC_B_NEXSYNC, IDC_P_NEXSYNC, IDC_B_DICTATION,
			IDC_P_DICTATION, IDC_ENABLE_IPADS_CHECK, IDC_NexCloud, IDC_VPN, IDC_Users,
			IDC_EDIT_ANCHOR_CALLERS, IDC_IAGNOSIS_COUNT, IDC_B_PORTAL_PROVIDERS,
			IDC_P_PORTAL_PROVIDERS, IDC_SUBSCRIPTION_LOCAL_HOSTED, IDC_AZURE_REMOTE_APP_CHECK
		};

		if (!GetCurrentUserPermissions(bioSupportBought, sptWrite) && !IsTestAccount())
		{
			//not allowed to change bought fields, so disable them
			m_pItemList->GetColumn(slcUsageCount)->Editable = FALSE;
			m_pItemList->GetColumn(slcExpiration)->Editable = FALSE;

			GetDlgItem(IDC_BUTTON_CHANGE_ANCHOR_PASS)->EnableWindow(FALSE);

			for each(UINT idc in aryBoughtIDCs)
				GetDlgItem(idc)->EnableWindow(FALSE);
		}
		else
		{
			m_pItemList->GetColumn(slcUsageCount)->Editable = bEnable;
			m_pItemList->GetColumn(slcExpiration)->Editable = bEnable;

			GetDlgItem(IDC_BUTTON_CHANGE_ANCHOR_PASS)->EnableWindow(bEnable && (!m_strAnchorPassword.IsEmpty() || m_nAnchorCallerCount));

			for each(UINT idc in aryBoughtIDCs)
				GetDlgItem(idc)->EnableWindow(bEnable);
		}

		UINT aryUsedIDCs[] =
		{
			IDC_U_LIC, IDC_U_CONC_TS_LIC, IDC_U_DOC, IDC_U_PALM, IDC_U_PPC,
			IDC_U_DATABASES, IDC_U_EMRPROV, IDC_U_NEXWEB_DOMAINS, IDC_U_LICPRES,
			IDC_U_MIDPRES, IDC_U_STAFFPRES, IDC_U_NEXSYNC, IDC_U_DICTATION,
			IDC_U_PORTAL_PROVIDERS,
		};
	
		if(!GetCurrentUserPermissions(bioSupportUsed, sptWrite) && !IsTestAccount())
		{
			GetDlgItem(IDC_IPAD_INUSE)->EnableWindow(FALSE); // (z.manning 2012-06-12 15:26) - PLID 50878

			//not allowed to change used fields, disable them
			for each(UINT idc in aryUsedIDCs)
				GetDlgItem(idc)->EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_IPAD_INUSE)->EnableWindow(bEnable && AreIpadsEnabled()); // (z.manning 2012-06-12 15:26) - PLID 50878

			for each(UINT idc in aryUsedIDCs)
				GetDlgItem(idc)->EnableWindow(bEnable);
		}

	} NxCatchAll(__FUNCTION__);
}

CString GetNameFromStatus(int nStatus)
{
	switch(nStatus) {
	case 0:
		return "Not Bought";
		break;
	case 1:
		return "Bought";
		break;
	case 2:
		return "In Use";
		break;
	case 3:
		return "De-activated";
		break;
	case 4:
		return "In Use, not bought";
		break;
	default:
		ASSERT(FALSE);
		return "";
		break;
	}
}

void CSupportDlg::OnEditingFinishedSupportItemsList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	if(!bCommit)
		return;

	if(nRow == sriNoRow)
		return;

	CString strField;
	CString strAudit;

	try {
		CString strOld, strNew;
		CString strNewValue;
		// (j.luckoski 2012-06-05 17:44) - PLID 50114 - Added in var to handle the dates
		_variant_t varDate;
		switch(nCol) {
		case slcStatus:
			strField = VarString(m_pItemList->GetValue(nRow, slcField));
			strOld = GetNameFromStatus(VarLong(varOldValue));
			strNew = GetNameFromStatus(VarLong(varNewValue));
			strNewValue.Format("%li", VarLong(varNewValue));
			break;		
		case slcUsageCount:
			strField = VarString(m_pItemList->GetValue(nRow, slcField)) + "Usage";
			strOld.Format("%li", VarLong(varOldValue));
			strNew.Format("%li", VarLong(varNewValue));
			strNewValue = strNew;
			break;
		case slcExpiration:
			strField = VarString(m_pItemList->GetValue(nRow, slcField)) + "Expiration";
			if(varOldValue.vt == VT_DATE) strOld = FormatDateTimeForInterface(VarDateTime(varOldValue), 0, dtoDate, true);
			else strOld = "None";
			if(varNewValue.vt == VT_DATE) {
				strNew = FormatDateTimeForInterface(VarDateTime(varNewValue), 0, dtoDate, true);
				// (j.luckoski 2012-06-05 17:44) - PLID 50114 - Grab the new value
				varDate = varNewValue;
			}
			else {
				strNew = "None";
				// (j.luckoski 2012-06-05 17:44) - PLID 50114 - If not date then null
				varDate.vt = VT_NULL;
			}
			break;
		default:
			return;
		}

		if(strOld == strNew) return;

		CAuditSupportDlg dlg(this);
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				//reset the selection
				m_pItemList->PutValue(nRow, nCol, varOldValue);
				return;
			}
		}

		//write the auditing info to our special support audit table
		//		ExecuteSql("CREATE TABLE NxSupportAuditT (ID int NOT NULL PRIMARY KEY, PersonID int NOT NULL REFERENCES PatientsT(PersonID), Username nvarchar(255) NOT NULL DEFAULT(''), FieldChanged nvarchar(255) NOT NULL DEFAULT(''), "
		//			"OldValue nvarchar(255) NOT NULL DEFAULT(''), NewValue nvarchar(255) NOT NULL DEFAULT(''), AuditNotes ntext NOT NULL DEFAULT(''), Date datetime NOT NULL DEFAULT(GetDate()))");

		if(strField.CompareNoCase("CellTrust") != 0) {
			SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), strField, strOld, strNew,  _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils

			//update this field in the data
			// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
			// (j.luckoski 2012-06-05 16:06) - PLID 50114 - Insert the date if that is the desired field.
			if(nCol == slcExpiration) {
					ExecuteParamSql(FormatString("UPDATE NxClientsT SET %s = {VT_DATE} WHERE PersonID = {INT}", 
					strField), varDate, GetActivePatientID());
			} else {
				ExecuteParamSql(FormatString("UPDATE NxClientsT SET %s = {STRING} WHERE PersonID = {INT}", 
				strField), strNewValue, GetActivePatientID());
			}

		}

			
		// (j.luckoski 03/27/12) - PLID 49168 - Added in section to mark occupanying modules as bought when these
		// modules are marked as bought.

		if(nCol == slcStatus && strNewValue == "1" && strOld == "Not Bought") {
			if(strField == "Marketing") {
				_RecordsetPtr prsOldValue = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Retention FROM NxClientsT WHERE PersonID = {INT} AND Retention <> 2", GetActivePatientID());
				if(!prsOldValue->eof) {
					FieldsPtr pflds = prsOldValue->GetFields();
					long nOldValue = AdoFldLong(pflds, "Retention", 0);
					CString strOld = GetNameFromStatus(nOldValue);
					SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "Retention", strOld, strNew, _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils
				}
				ExecuteParamSql("UPDATE NxClientsT SET Retention = {STRING} WHERE PersonID = {INT} AND Retention <> 2", strNewValue, GetActivePatientID());
				

			} else if(strField == "NexEMR") {
				_RecordsetPtr prsOldValue = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Labs, Transcriptions, DeviceImport, Recall, Patients FROM NxClientsT WHERE PersonID = {INT} \r\n", GetActivePatientID());
				CSqlFragment sqlFrag;
				// (d.singleton 2012-06-13 16:07) - PLID 50301 create sql frag
				sqlFrag = CSqlFragment("");

				// Labs
				if(!prsOldValue->eof) {
					FieldsPtr pflds = prsOldValue->GetFields();
					long nLabOld = AdoFldLong(pflds, "Labs", 0);
					CString strOld = GetNameFromStatus(nLabOld);
					if(nLabOld != 2) {
						SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "Labs", strOld, strNew, _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils
					}
					
					long nTranOld = AdoFldLong(pflds, "Transcriptions", 0);
					strOld = GetNameFromStatus(nTranOld);
					if(nTranOld != 2 ) {
						SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "Transcriptions", strOld, strNew, _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils
					}
						
					long nDevOld = AdoFldLong(pflds, "DeviceImport", 0);
					strOld = GetNameFromStatus(nDevOld);
					if(nDevOld != 2) {
						SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "DeviceImport", strOld, strNew, _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils
					}
		
					long nRecOld = AdoFldLong(pflds, "Recall", 0);
					strOld = GetNameFromStatus(nRecOld);
					if(nRecOld != 2) {
						SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "Recall", strOld, strNew, _Q(dlg.m_strText));
					}

					// (d.singleton 2012-06-13 16:08) - PLID 50301 if patients is not bought and nexemr is bought prompt to add patients
					long nPatOld = AdoFldLong(pflds, "Patients", 0);
					if(nPatOld == 0 && strNewValue == "1") {
						if(MessageBox("Would you also like to change the Patients module to \"Bought\" status?", "Nextech Practice", MB_YESNO) == IDYES) {
							sqlFrag = CSqlFragment("UPDATE NxClientsT SET Patients = 2 WHERE PersonID = {INT} AND Patients <> 2; ", GetActivePatientID());
						}
					}
				}

				// (d.singleton 2012-06-13 16:08) - PLID 50301 add the sql frag for patients module
				ExecuteParamSql("UPDATE NxClientsT SET Labs = {STRING} WHERE PersonID = {INT} AND Labs <> 2; "
					"UPDATE NxClientsT SET Transcriptions = {STRING} WHERE PersonID = {INT} AND Transcriptions <> 2; "
					"UPDATE NxClientsT SET DeviceImport = {STRING} WHERE PersonID = {INT} AND DeviceImport <> 2; "
					"UPDATE NxClientsT SET Recall = {STRING} WHERE PersonID = {INT} AND Recall <> 2; "
					"{SQL}", strNewValue, GetActivePatientID(), strNewValue, GetActivePatientID(), strNewValue, GetActivePatientID(), strNewValue, GetActivePatientID(), sqlFrag);

			} else if(strField == "NexForms") {
				_RecordsetPtr prsOldValue = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT CycleOfCare FROM NxClientsT WHERE PersonID = {INT} AND CycleOfCare <> 2", GetActivePatientID());
				if(!prsOldValue->eof) {
					FieldsPtr pflds = prsOldValue->GetFields();
					long nOldValue = AdoFldLong(pflds, "CycleOfCare", 0);
					CString strOld = GetNameFromStatus(nOldValue);
					SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "CycleOfCare", strOld, strNew, _Q(dlg.m_strText));
				}
				// (j.luckoski 2012-06-04 11:44) - PLID 49168 - Corrected {STRNG} to {STRING}
				ExecuteParamSql("UPDATE NxClientsT SET CycleOfCare = {STRING} WHERE PersonID = {INT} AND CycleOfCare <> 2", strNewValue, GetActivePatientID());
			} else if(strField == "Patients") {
				_RecordsetPtr prsOldValue = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT BarcodeRead FROM NxClientsT WHERE PersonID = {INT} AND BarcodeRead <> 2", GetActivePatientID());
				if(!prsOldValue->eof) {
					FieldsPtr pflds = prsOldValue->GetFields();
					long nOldValue = AdoFldLong(pflds, "BarcodeRead", 0);
					CString strOld = GetNameFromStatus(nOldValue);
					SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "BarcodeRead", strOld, strNew, _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils
				}
				ExecuteParamSql("UPDATE NxClientsT SET BarcodeRead = {STRING} WHERE PersonID = {INT} AND BarcodeRead <> 2", strNewValue, GetActivePatientID());
			}
		 RequeryItemList();
		}

		//DRT 3/3/2005 - PLID 15832 - Warn them to fill in the ASPS forms fields.  This makes no actual requirements, just a reminder.
		//Only do this if they're adding the value -- if they are removing, the data remains.
		if(nCol == slcStatus && VarLong(varOldValue) != 2 && VarLong(varNewValue) == 2) {
			CString strRowValue = VarString(m_pItemList->GetValue(nRow, slcField), "");
			if(strRowValue.CompareNoCase("NexForms") == 0) {
				//this is the nexforms in use field
				AfxMessageBox("You have enabled NexForms for this client.  Please make sure that you ask them about the ASPS forms and enter the "
					"appropriate data in the ASPS Forms fields.");
			}
		}

		// (a.walling 2007-11-30 11:18) - PLID 28044 - If marking NexEMR as deactivated, prompt to remind that an expiration date
		// needs to be set!
		try {
			if ( (strField.CompareNoCase("NexEMR") == 0) && nCol == slcStatus && VarLong(varOldValue) != sisDeactivated && VarLong(varNewValue) == sisDeactivated) {
				// NexEMR marked as deactivated, is there an expiration date?
				_variant_t varExp = m_pItemList->GetValue(nRow, slcExpiration);
				COleDateTime dtExp;
				dtExp.SetStatus(COleDateTime::invalid);

				if (varExp.vt == VT_DATE) {
					dtExp = VarDateTime(varExp);
				}

				// if the expiration date is null or invalid, prompt
				if (dtExp.GetStatus() != COleDateTime::valid) {
					AfxMessageBox("You have deactivated NexEMR; please ensure that an expiration date is set so the client can still access existing patient health records!", MB_ICONEXCLAMATION);
				}
			}
		} NxCatchAll("Error checking NexEMR expiration status");

		// (f.dinatale 2010-11-09) - PLID 40829 - If we're enabling Celltrust Integration, make sure to send a message to the NxReminder Server to make sure
		// the client is added to its records.
		try
		{
			if ((strField.CompareNoCase("CellTrust") == 0) && nCol == slcStatus)
			{
				// If the row changed was the NxReminder service, get ready to send a message to the NxReminder server.
				CString strLicense;
				int nStatus;
				GetDlgItemText(IDC_LICENSE_KEY, strLicense);
				CNxRetrieveNxReminderClient nxRem(strLicense);

				// We know that the new value is going to be commited since the audit wasn't cancelled.  Take the appropriate action based on what it 
				// was set to.  In every case, setting a client to active is the last step in case something goes wrong so that in worst case they have
				// corrupted settings, but are unable to use those settings because they are not set active.
				switch(VarLong(varNewValue))
				{
					// (f.dinatale 2011-03-02) - PLID 40829 - Made it so that "Bought" is the same as "In-Use".
					// They have now bought NxReminder.
					// Assuming that we want to maintain any previous settings that the client may have had (though this is probably a bad idea because of
					// the number of messages that need to be sent), we retrieve the clients settings and 
				case sisBought:
				case sisActive:
				case sisActiveNotBought:
					// If the client is jumping to active without having bought the product, the likelihood is that they need to be added to the database.
					if(VarLong(varOldValue) == sisActiveNotBought || VarLong(varOldValue) == sisNotBought || VarLong(varOldValue) == sisDeactivated)
					{
						// (z.manning 2011-09-12 10:13) - PLID 45422 - We do not actually use any of this tier stuff.
						// Rather than defaulting the number of credits to nothing let's give them whatever
						// the highest amount of credits as anyone who has NexReminder has unlimited credits.
						// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
						_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID, Messages FROM CellTrustTiersT ORDER BY Messages DESC");
						long nTierID = -1;
						long nMessages = 0;
						if(!prs->eof) {
							nTierID = AdoFldLong(prs, "ID");
							nMessages = AdoFldLong(prs, "Messages");
						}
						nStatus = nxRem.ModifyClientLicense(nTierID, nMessages);

						// If we do not get a proper response back, rollback any changes on the Internal side to keep NxReminder server and Internal in sync.
						// Notify the user also.
						if(nStatus < 0) {
							m_pItemList->PutValue(nRow, nCol, varOldValue);
							MessageBox("Error modifying client information on NexReminder Server, changes were not saved.");
							return;
						}
					}

					nStatus = nxRem.ModifyClientActiveStatus(true);

					// If we do not get a proper response back, rollback any changes on the Internal side to keep NxReminder server and Internal in sync.
					// Notify the user also.
					if(nStatus < 0) {
						m_pItemList->PutValue(nRow, nCol, varOldValue);
						MessageBox("Error modifying client information on NexReminder Server, changes were not saved.");
						return;
					}
					break;
					// The client has decided to deactivate their use of NxReminder.  Set them to be inactive.  Setting the license to Not Bought will have the
					// same effect, except the clients settings will not be retained should they reactivate the service.
				case sisNotBought:
				case sisDeactivated:
					nStatus = nxRem.ModifyClientActiveStatus(false);

					// If we do not get a proper response back, rollback any changes on the Internal side to keep NxReminder server and Internal in sync.
					// Notify the user also.
					if(nStatus < 0) {
						m_pItemList->PutValue(nRow, nCol, varOldValue);
						MessageBox("Error modifying client information on NexReminder Server, changes were not saved.");
						return;
					}
					break;
				}

				//(f.dinatale 2011-01-24) - PLID 40829
				// Placed the audit after trying to contact the middle man server in order to prevent having to revert back settings in Internal should the SOAP
				// message fail.
				SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), strField, strOld, strNew,  _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils

				//update this field in the data
				// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
				ExecuteParamSql(FormatString("UPDATE NxClientsT SET %s = {STRING} WHERE PersonID = {INT}", 
					strField), strNewValue, GetActivePatientID());
			}
		} NxCatchAll("Error sending a message to NexReminder Server.");


	} NxCatchAll("Error editing support item.");
}

// (j.armen 2012-06-20 18:07) - PLID 50688 - Pulled this out as we use the same logic several times
bool HasEMRType(_DNxDataListPtr pdl, const CString& strLicense)
{
	//See if records are in use
	long nRow = pdl->FindByColumn(0, _bstr_t(strLicense), 0, VARIANT_FALSE);
	long nStatus = VarLong(pdl->GetValue(nRow, slcStatus));

	//We also need to get the usage count & expiration date data
	long nUsage = VarLong(pdl->GetValue(nRow, slcUsageCount));
	_variant_t varDate = pdl->GetValue(nRow, slcExpiration);
	CString strExpiration;
	if(varDate.vt != VT_NULL && varDate.vt != VT_EMPTY)
		strExpiration = VarDateTime(varDate).Format("%m/%d/%Y");

	if( (nStatus == sisNotBought || nStatus == sisDeactivated) && nUsage < 0 && strExpiration.IsEmpty() ) {
		return false;
	}
	else {
		return true;
	}
}


// (j.armen 2012-06-20 18:07) - PLID 50688 - Check for NexEMR, NexEMRCosmetic, and Custom Records
BOOL CSupportDlg::AllowEMRType(long nRow, VARIANT FAR* pvarNewValue)
{
	CString strField = VarString(m_pItemList->GetValue(nRow, slcField));
	//DRT PLID 17779 - If they're changing the status of NexEMR or CustomRecords, we need to make sure the other one is
	//	not already non-bought or non-deactivated.  A user may not have both custom records & nexemr in use 
	//	at the same time.
	//The pvarNewValue parameter is only if we're changing the status field, otherwise we assume 'bought'
	SupportItemStatus sisCurrent;
	if(pvarNewValue->vt == VT_NULL) {
		//Due to the way expiration/usages work, we "pretend" to be selecting 'Bought'.
		sisCurrent = sisBought;
	}
	else {
		sisCurrent = (SupportItemStatus)VarLong(*pvarNewValue);
	}

	if(sisCurrent != sisNotBought && sisCurrent != sisDeactivated) {
		if(strField.CompareNoCase("NexEMR") == 0) {
			if(HasEMRType(m_pItemList, "CustomRecords")) {
				MsgBox("You may not enable NexEMR for a client who already has Custom Records.  Please deactivate the Custom Records module first.");
				return FALSE;
			} else if(HasEMRType(m_pItemList, "NexEMRCosmetic")) {
				MsgBox("You may not enable NexEMR for a client who already has Custom Records.  Please deactivate the NexEMRCosmetic module first.");
				return FALSE;
			} else {
				return TRUE;
			}
		} else if(strField.CompareNoCase("CustomRecords") == 0) {
			if(HasEMRType(m_pItemList, "NexEMR")) {
				MsgBox("You may not enable Custom Records for a client who already has NexEMR.  Please deactivate the Custom Records module first.");
				return FALSE;
			} else if(HasEMRType(m_pItemList, "NexEMRCosmetic")) {
				MsgBox("You may not enable Custom Records for a client who already has NexEMR.  Please deactivate the NexEMRCosmetic module first.");
				return FALSE;
			} else {
				return TRUE;
			}
		} else if(strField.CompareNoCase("NexEMRCosmetic") == 0) {
			if(HasEMRType(m_pItemList, "CustomRecords")) {
				MsgBox("You may not enable NexEMR Cosmetic for a client who already has Custom Records.  Please deactivate the Custom Records module first.");
				return FALSE;
			} else if(HasEMRType(m_pItemList, "NexEMR")) {
				MsgBox("You may not enable NexEMR Cosmetic for a client who already has NexEMR.  Please deactivate the NexEMRCosmetic module first.");
				return FALSE;
			} else {
				return TRUE;
			}
		}
	}

	return TRUE;
}

void CSupportDlg::OnEditingFinishingSupportItemsList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if(!*pbCommit) return;
		//check permissions
		switch(nCol) {
		case slcStatus:
			{
				if(!AllowEMRType(nRow, pvarNewValue)) {
					*pbCommit = FALSE;
					*pbContinue = TRUE;
					return;
				}

				bool bNeedBought = false;
				bool bNeedUsed = false;
				bool bIllegal = false;
				switch(VarLong(varOldValue)) {
				case sisNotBought:
					switch(VarLong(*pvarNewValue)) {
					case sisBought:
						bNeedBought = true;
						break;
					case sisActiveNotBought:
						bNeedUsed = true;
						break;
					case sisActive:
					case sisDeactivated:
						bIllegal = true;
						break;
					}
					break;
				case sisBought:
					switch(VarLong(*pvarNewValue)) {
					case sisNotBought:
					case sisDeactivated:
						bNeedBought = true;
						break;
					case sisActive:
						bNeedUsed = true;
						break;
					case sisActiveNotBought:
						bIllegal = true;
						break;
					}
					break;
				case sisActive:
					switch(VarLong(*pvarNewValue)) {
					case sisNotBought:
					case sisDeactivated:
					case sisActiveNotBought:
						bNeedBought = true;
						break;
					case sisBought:
						bNeedUsed = true;
						break;
					}
					break;
				case sisDeactivated:
					switch(VarLong(*pvarNewValue)) {
					case sisNotBought:
					case sisBought:
						bNeedBought = true;
						break;
					case sisActive:
					case sisActiveNotBought:
						bIllegal = true;
						break;
					}
					break;
				case sisActiveNotBought:
					switch(VarLong(*pvarNewValue)) {
					case sisNotBought:
						bNeedUsed = true;
						break;
					case sisBought:
					case sisDeactivated:
						bIllegal = true;
						break;
					case sisActive:
						bNeedBought = true;
						break;
					}
					break;
				}

				if(bIllegal) {
					MsgBox("It is not valid to change a status from %s to %s", GetNameFromStatus(VarLong(varOldValue)), GetNameFromStatus(VarLong(*pvarNewValue)));
					*pbCommit = FALSE;
					*pbContinue = TRUE;
					return;
				}
				if(bNeedBought) {
					//(c.copits 2010-06-25) PLID 33989 - Add the ability to flag certain accounts as "non-permissioned"
					if (!IsTestAccount()) {
						//make sure this user has permission to bought
						if(!CheckCurrentUserPermissions(bioSupportBought, sptWrite)) {
							*pbCommit = FALSE;
							*pbContinue = TRUE;
						}
					}
				}
				if(bNeedUsed) {
					//(c.copits 2010-06-25) PLID 33989 - Add the ability to flag certain accounts as "non-permissioned"
					if (!IsTestAccount()) {
						//make sure this user has permission to bought
						if(!CheckCurrentUserPermissions(bioSupportUsed, sptWrite)) {
							*pbCommit = FALSE;
							*pbContinue = TRUE;
						}
					}
				}

				// (b.savon 2016-04-11) - NX-100081 - Protect the license change from invalid IDP transaction ID entries
				if (DisallowERxLicenseChange(VarString(m_pItemList->GetValue(nRow, slcField)), (SupportItemStatus)VarLong(*pvarNewValue))) {
					MessageBox(
						"You must have at least one IDP transaction ID for this ePrescribing license before you can mark it 'Bought' or 'In Use'. Please fill in the IDP transaction ID and try again.",
						"Nextech", 
						MB_ICONINFORMATION
					);
					*pbCommit = FALSE;
					*pbContinue = TRUE;
					return;
				}
				else {
					// They are allowed or the license being changed isn't an ePrescribing license.
				}
				
			}
			break;
		case slcExpiration:
			{
				COleDateTime dt;
				if(CString(strUserEntered).IsEmpty()) {
					(*pvarNewValue).vt = VT_NULL;
				}
				else if(!dt.ParseDateTime(strUserEntered)) {
					MsgBox("Please enter a valid expiration date.");
					*pbCommit = FALSE;
					*pbContinue = TRUE;
					return;
				}

				//Check the emr / custom records conflict
				_variant_t varTest;
				varTest.vt = VT_NULL;
				if(!AllowEMRType(nRow, &varTest)) {
					*pbCommit = FALSE;
					*pbContinue = TRUE;
					return;
				}
			}
			break;

		case slcUsageCount:
			_variant_t varTest;
			varTest.vt = VT_NULL;
			if(!AllowEMRType(nRow, &varTest)) {
				*pbCommit = FALSE;
				*pbContinue = TRUE;
				return;
			}
			break;
		}
	} NxCatchAll("Error in OnEditingFinishingSupportItemsList");
}

// (b.savon 2016-04-11) - NX-100081
/// <summary>
/// A utility that checks if there is at least one IDP (Identify Proofing) transaction id specified
/// for the NewCrop or NexERx license that is being changed to 'Bought' or 'In Use'
/// </summary>
/// <param name="strField">The field name for the license</param>
/// <param name="sisNewStatus">The (new) status of the license</param>
/// <returns>true to disallow the license change; false if not</returns>
bool CSupportDlg::DisallowERxLicenseChange(const CString &strField, const SupportItemStatus &sisNewStatus)
{
	if (sisNewStatus != sisActive && sisNewStatus != sisBought) {
		return false;
	}
	else {
		bool bDisallowERxChange = false;
		if (strField.CompareNoCase("NewCrop") == 0) {
			CString strNewCropIDPTransactionIDs;
			GetDlgItemText(IDC_NEWCROP_IDP_ID_EDIT, strNewCropIDPTransactionIDs);
			if (strNewCropIDPTransactionIDs.Trim().IsEmpty()) {
				bDisallowERxChange = true;
			}
		}
		else if (strField.CompareNoCase("ePrescribe") == 0) {
			CString strNexERxIDPTransactionIDs;
			GetDlgItemText(IDC_NEXERX_IDP_ID_EDIT, strNexERxIDPTransactionIDs);
			if (strNexERxIDPTransactionIDs.Trim().IsEmpty()) {
				bDisallowERxChange = true;
			}
		}
		else {
			// Not a field we care about
		}

		return bDisallowERxChange;
	}
}

void CSupportDlg::OnViewSupportHistory() 
{
	CViewSupportHistoryDlg dlg(this);
	dlg.DoModal();
}

/*
ALTER TABLE NxClientsT ADD ASPSForms int NULL, ASPSPurchDate datetime NULL, 
AMAVersion nvarchar(100) NOT NULL DEFAULT(''), AMAInstalled datetime NULL
*/

void CSupportDlg::OnSelChosenAspsForms(long nRow) 
{
	try {
		// (j.luckoski 2012-06-06 14:24) - PLID 50114 - Removed the NULL and relaced it with "" because of params
		CString strType = "";
		CString strNew;
		if(nRow != sriNoRow) {
			long nID = VarLong(m_pASPSForms->GetValue(nRow, 0));
			if(nID > 0) {
				strType.Format("%li", nID);
				strNew = VarString(m_pASPSForms->GetValue(nRow, 1), "");
			}
		}

		//for auditing
		CString field = "ASPSForms";
		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		_RecordsetPtr prs = CreateParamRecordset(FormatString("SELECT %s FROM NxClientsT WHERE PersonID = {INT}", field), m_id);

		CString strOld;
		if(!prs->eof) {
			_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
			if(var.vt == VT_I4) {
				if(VarLong(var) == 1)
					strOld = "From ASPS";
				else if(VarLong(var) == 2)
					strOld = "From NexTech";
			}
		}

		if(strOld == strNew)
			return;	//didnt change anything

		CAuditSupportDlg dlg(this);
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				//reset the selection
				if(!prs->eof) {
					_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
					if(var.vt == VT_I4)
						m_pASPSForms->SetSelByColumn(0, var);
					else
						m_pASPSForms->PutCurSel(sriNoRow);
				}
				else
					m_pASPSForms->PutCurSel(sriNoRow);
				return;
			}
		}

		SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), field, strOld, strNew, _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils

		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		ExecuteParamSql("UPDATE NxClientsT SET ASPSForms = {STRING} WHERE PersonID = {INT}", strType, m_id);

	} NxCatchAll("Error in OnSelChosenASPSForms");
}

void CSupportDlg::OnKillFocusAspsPurchDate() 
{
	try {
		// (j.luckoski 2012-06-05 17:45) - PLID 50114 - Use var and {VT_DATE } instead of {STRING} for the new params
		COleDateTime dt;
		dt = m_pASPSPurchDate->GetDateTime();
		_variant_t varDate;

		CString strNew;
		if(m_pASPSPurchDate->GetStatus() != 1)
			varDate.vt = VT_NULL;
		else {
			COleVariant varDateExtract(dt);
			varDate = varDateExtract;
			strNew = FormatDateTimeForInterface(dt, dtoDate);
		}

		//for auditing
		CString field = "ASPSPurchDate";
		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		_RecordsetPtr prs = CreateParamRecordset(FormatString("SELECT %s FROM NxClientsT WHERE PersonID = {INT}", field), m_id);

		CString strOld;
		if(!prs->eof) {
			_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
			if(var.vt == VT_DATE)
				strOld = FormatDateTimeForInterface(VarDateTime(var), dtoDate);
		}

		if(strOld == strNew)
			return;	//didnt change anything

		CAuditSupportDlg dlg(this);
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				//reset the selection
				if(!prs->eof) {
					_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
					if(var.vt == VT_DATE)
						m_pASPSPurchDate->SetDateTime(VarDateTime(var));
					else
						m_pASPSPurchDate->Clear();
				}
				else
					m_pASPSPurchDate->Clear();
				return;
			}
		}

		SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), field, strOld, strNew == "NULL" ? "" : strNew,  _Q(dlg.m_strText));

		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		ExecuteParamSql("UPDATE NxClientsT SET ASPSPurchDate = {VT_DATE} WHERE PersonID = {INT}", varDate, m_id);
	} NxCatchAll("Error saving ASPS Purchase date.");
}

void CSupportDlg::OnKillFocusAmaInstalledOnDate() 
{
	try {
		// (j.luckoski 2012-06-05 17:45) - PLID 50114 - Use var and {VT_DATE } instead of {STRING} for the new params
		COleDateTime dt;
		dt = m_pAMAInstalled->GetDateTime();
		_variant_t varDate;

		
		CString strNew;
		if(m_pAMAInstalled->GetStatus() != 1)
			varDate.vt = VT_NULL;
		else {
			COleVariant varDateExtract(dt);
			varDate = varDateExtract;
			strNew = FormatDateTimeForInterface(dt, dtoDate);
		}

		//for auditing
		CString field = "AMAInstalled";
		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		_RecordsetPtr prs = CreateParamRecordset(FormatString("SELECT %s FROM NxClientsT WHERE PersonID = {INT}", field), m_id);

		CString strOld;
		if(!prs->eof) {
			_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
			if(var.vt == VT_DATE)
				strOld = FormatDateTimeForInterface(VarDateTime(var), dtoDate);
		}

		if(strOld == strNew)
			return;	//they didnt change anything

		CAuditSupportDlg dlg(this);
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				//reset the selection
				if(!prs->eof) {
					_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
					if(var.vt == VT_DATE)
						m_pASPSPurchDate->SetDateTime(VarDateTime(var));
					else
						m_pASPSPurchDate->Clear();
				}
				else
					m_pASPSPurchDate->Clear();
				return;
			}
		}

		SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), field, strOld, strNew == "NULL" ? "" : strNew,  _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils

		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		ExecuteParamSql("UPDATE NxClientsT SET AMAInstalled = {VT_DATE} WHERE PersonID = {INT}", varDate, m_id);
	} NxCatchAll("Error saving AMA installation date.");
}

void CSupportDlg::OnSelChosenEstatementType(long nRow) 
{
	if(nRow == -1)
		return;

	try {
		long nVal = VarLong(m_pEStatementType->GetValue(nRow, 0));

		CString str;
		if(nVal == -1)
			// (j.luckoski 2012-06-06 14:24) - PLID 50114 - Removed the NULL and relaced it with "" because of params
			str.Format("");
		else
			str.Format("%li", nVal);

		//for auditing
		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		_RecordsetPtr prs = CreateParamRecordset("SELECT EStatementType, Type FROM NxClientsT LEFT JOIN InternalEStatementTypes "
			"ON NxClientsT.EStatementType = InternalEStatementTypes.ID WHERE PersonID = {INT}", m_id);

		CAuditSupportDlg dlg(this);
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				//reset the selection
				if(!prs->eof) {
					m_pEStatementType->SetSelByColumn(0, AdoFldLong(prs, "EStatementType", -1));
				}
				else
					m_pEStatementType->PutCurSel(sriNoRow);
				return;
			}
		}

		CString strOld, strNew;
		if(!prs->eof)
			strOld = AdoFldString(prs, "Type", "");
		if(nVal != -1)
			strNew = VarString(m_pEStatementType->GetValue(nRow, 1), "");

		SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "EStatement Type", strOld, strNew,  _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils

		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		ExecuteParamSql("UPDATE NxClientsT SET EStatementType = {STRING} WHERE PersonID = {INT}", str, m_id);
	} NxCatchAll("Error selecting estatement type.");
	
}

void CSupportDlg::OnSelChangingEstatementType(long FAR* nNewSel) 
{
	if(*nNewSel == -1)
		*nNewSel = 0;
	
}

void CSupportDlg::OnEditWebLogin() 
{
	//pop up the editing dialog
	CNexWebLoginInfoDlg dlg(m_id, this);
	dlg.DoModal();
	
}

//DRT 12/31/2004 - PLID 15160 - Changed the version current from a text box to a dropdown list.  Here are the relevant
//	table creation queries, since we don't use mods for internal:
//	
//	Create the new table to hold the possible versions:
//	CREATE TABLE ReleasedVersionsT (ID int NOT NULL PRIMARY KEY, Version nvarchar(50) NOT NULL, AMAVersion nvarchar(50) NOT NULL, 
//	ReleaseDate datetime NOT NULL, Notes nvarchar(2000) NOT NULL DEFAULT(''))
//
//	Populate the ReleasedVersionsT table:
//	INSERT INTO ReleasedVersionsT (Version, AMAVersion, ReleaseDate) 
//	SELECT VerCur, '' AS AmaVer, convert(datetime, left(VerCur, 10)) AS ReleaseDate
//	FROM NxClientsT
//	WHERE VerCur <> ''
//	GROUP BY VerCur
//
//	Add a new column to NxClientsT to reference ReleasedVersionsT
//	ALTER TABLE NxClientsT ADD VersionCurrent int NULL CONSTRAINT FK_NxClientsT_ReleasedVersionsT REFERENCES ReleasedVersionsT(ID)
//
//	Move the data to the new column:
//	UPDATE NxClientsT SET VersionCurrent = SubQ.ID
//	FROM (SELECT ReleasedVersionsT.ID, NxC.PersonID
//	FROM NxClientsT NxC LEFT JOIN ReleasedVersionsT ON NxC.VerCur = ReleasedVersionsT.Version
//	WHERE NxC.VerCur <> '') SubQ
//	WHERE SubQ.PersonID = NxClientsT.PersonID
//	
//
void CSupportDlg::OnSelChosenVersionCur(long nRow) 
{
	try {
		// (f.dinatale 2010-08-06) - PLID 38842 - Updated to use an {INT} for the version ID in queries rather than a {STRING}
		// Get the selected item and pull its ID if the selection isn't NULL
		long nCurSel = m_pCurVersion->GetCurSel();
		if(nCurSel != sriNoRow) {
			long nVerID = VarLong(m_pCurVersion->GetValue(nCurSel, 0));


			//Now insert this value into the data
			// (f.dinatale 2010-05-24) - PLID 38842 - Added the update for the version history.
			CParamSqlBatch sqlBatch;

			sqlBatch.Add("SET NOCOUNT ON\r\n");
			sqlBatch.Add("UPDATE NxClientsT SET VersionCurrent = {INT} WHERE PersonID = {INT}", nVerID, m_id);
			sqlBatch.Add("INSERT INTO VersionHistoryT (VerID, UserID, Date, ClientID) VALUES ({INT}, {INT}, GETDATE(), {INT})",
				nVerID, GetCurrentUserID(), m_id);
			sqlBatch.Add("SELECT TOP 1 UsersT.UserName AS Username, VersionHistoryT.Date AS UpgradeDate FROM VersionHistoryT "
				"INNER JOIN UsersT ON VersionHistoryT.UserID = UsersT.PersonID "
				"WHERE VersionHistoryT.ClientID = {INT} ORDER BY UpgradeDate DESC", m_id);
			sqlBatch.Add("SET NOCOUNT OFF\r\n");

			_RecordsetPtr prs = sqlBatch.CreateRecordset(GetRemoteData());
			FieldsPtr pfields = prs->GetFields();

			SetDlgItemText(IDC_UPDATED_BY, AdoFldString(pfields, "Username"));
			COleDateTime dtLastUpdate = AdoFldDateTime(pfields, "UpgradeDate", COleDateTime(g_cvarNull));
			if(dtLastUpdate.GetStatus() == COleDateTime::valid){
				SetDlgItemText(IDC_UPDATED_ON, FormatDateTimeForInterface(dtLastUpdate, NULL, dtoDate));
			}else{
				SetDlgItemText(IDC_UPDATED_ON, "");
			}
		}

	} NxCatchAll("Error in OnSelChosenVersionCur");
}

void CSupportDlg::OnNewVersion() 
{
	try {
		CNewVersionDlg dlg(this);
		dlg.DoModal();

	} NxCatchAll("Error in OnNewVersion()");
}

void CSupportDlg::OnThirdPartyInfo() 
{
	CSupportThirdPartyInfoDlg dlg(this);
	dlg.m_nPatientID = m_id;
	dlg.m_Color = GetCurrentColor();
	dlg.DoModal();
}

void CSupportDlg::OnIsLicenseActive() 
{
	try {
		CAuditSupportDlg dlg(this);
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				CheckDlgButton(IDC_IS_LICENSE_ACTIVE, IsDlgButtonChecked(IDC_IS_LICENSE_ACTIVE)?BST_UNCHECKED:BST_CHECKED);
			}

			CString strOld = IsDlgButtonChecked(IDC_IS_LICENSE_ACTIVE)?"Inactive":"Active";
			CString strNew = IsDlgButtonChecked(IDC_IS_LICENSE_ACTIVE)?"Active":"Inactive";

			SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "LicenseActivated", strOld, strNew,  _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils
		}

		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		ExecuteParamSql(
			"UPDATE NxClientsT SET LicenseActivated = {INT} WHERE PersonID = {INT}", 
			IsDlgButtonChecked(IDC_IS_LICENSE_ACTIVE) ? 1 : 0, 
			m_id);
	} NxCatchAll("CSupportDlg::OnIsLicenseActive");	
}

// (z.manning 2012-06-12 16:58) - PLID 50878
void CSupportDlg::OnEnableIpadsCheck()
{
	EnableIpads(); // (r.ries 2013-2-20) - PLID 55244 - Moved this to a separate function
}

// (r.gonet 2016-05-10) - NX-100478 - When the user checks the Azure RemoteApp checkbox, toggle the license for it.
void CSupportDlg::OnAzureRemoteAppCheck()
{
	EnableAzureRemoteApp();
}

void CSupportDlg::OnSelChangingPracticeType(long FAR* nNewSel) 
{
	if(*nNewSel == -1)
		*nNewSel = 0;
	
}

void CSupportDlg::OnSelChosenPracticeType(long nRow) 
{
	if(nRow == -1)
		return;

	try {
		long nVal = VarLong(m_pPracticeType->GetValue(nRow, 0));

		CString str;
		if(nVal == -1)
			str.Format("NULL");
		else
			str.Format("%li", nVal);

		//for auditing
		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		_RecordsetPtr prs = CreateParamRecordset("SELECT PracticeType FROM NxClientsT WHERE PersonID = {INT}", m_id);
		long nOldType = AdoFldLong(prs, "PracticeType");
		
		CAuditSupportDlg dlg(this);
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				//reset the selection
				m_pPracticeType->SetSelByColumn(0, nOldType);
				return;
			}
		}

		CString strOld, strNew;
		if(!prs->eof)
			strOld = GetNameForPracticeType(nOldType);
		if(nVal != -1)
			strNew = GetNameForPracticeType(nVal);;

		SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "Practice Type", strOld, strNew,  _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils

		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		ExecuteParamSql("UPDATE NxClientsT SET PracticeType = {STRING} WHERE PersonID = {INT}", str, m_id);
	} NxCatchAll("Error selecting practice type.");
}

void CSupportDlg::OnKillFocusLicenseActivatedOn() 
{
	try {
		// (j.luckoski 2012-06-05 17:45) - PLID 50114 - Use var and {VT_DATE } instead of {STRING} for the new params
		COleDateTime dt;
		dt = m_pLicenseActivatedDate->GetDateTime();
		_variant_t varDate;
		

		CString strNew;
		if(m_pLicenseActivatedDate->GetStatus() != 1)
			varDate.vt = VT_NULL;
		else {
			COleVariant varDateExtract(dt);
			varDate = varDateExtract;
			strNew = FormatDateTimeForInterface(dt, dtoDate);
		}

		//for auditing
		CString field = "LicenseActivatedOn";
		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		_RecordsetPtr prs = CreateParamRecordset(FormatString("SELECT %s FROM NxClientsT WHERE PersonID = {INT}", field), m_id);

		CString strOld;
		if(!prs->eof) {
			_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
			if(var.vt == VT_DATE)
				strOld = FormatDateTimeForInterface(VarDateTime(var), dtoDate);
		}

		if(strOld == strNew)
			return;	//didnt change anything

		CAuditSupportDlg dlg(this);
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				//reset the selection
				if(!prs->eof) {
					_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
					if(var.vt == VT_DATE)
						m_pLicenseActivatedDate->SetDateTime(VarDateTime(var));
					else
						m_pLicenseActivatedDate->Clear();
				}
				else
					m_pLicenseActivatedDate->Clear();
				return;
			}
		}

		SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), field, strOld, strNew == "NULL" ? "" : strNew,  _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils

		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		ExecuteParamSql("UPDATE NxClientsT SET LicenseActivatedOn = {VT_DATE} WHERE PersonID = {INT}", varDate, m_id);
	} NxCatchAll("Error saving License Activation date.");
}

// (f.dinatale 2010-05-21) - PLID 38842 - Removed Shadow support
/*void CSupportDlg::OnAllowShadow() 
{
	try {
		CAuditSupportDlg dlg;
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				CheckDlgButton(IDC_ALLOW_SHADOW, IsDlgButtonChecked(IDC_ALLOW_SHADOW)?BST_UNCHECKED:BST_CHECKED);
			}

			CString strOld = IsDlgButtonChecked(IDC_ALLOW_SHADOW)?"Not Allowed":"Allowed";
			CString strNew = IsDlgButtonChecked(IDC_ALLOW_SHADOW)?"Allowed":"Not Allowed";

			AuditSupportItem("AllowShadow", strOld, strNew,  _Q(dlg.m_strText));
		}

		ExecuteSql(
			"UPDATE NxClientsT SET AllowShadow = %li WHERE PersonID = %li", 
			IsDlgButtonChecked(IDC_ALLOW_SHADOW) ? 1 : 0, 
			m_id);
	} NxCatchAll("CSupportDlg::OnAllowShadow");
}*/	

void CSupportDlg::OnConnectionInfo() 
{
	CSupportConnectionDlg dlg(this);
	dlg.m_nPatientID = m_id;
	dlg.m_Color = GetCurrentColor();
	dlg.DoModal();
}

void CSupportDlg::OnVerHistory()
{
	try{
		CSupportUpgradeHistoryDlg dlg(this, m_id);
		dlg.DoModal();
	}NxCatchAll("Error in retrieving version history.");
}

//(c.copits 2010-06-25) PLID 33989 - Add the ability to flag certain accounts as "non-permissioned"
//IsTestAccount() -- This function will return true if the current patient is one of the allowable test accounts and
//false otherwise.
bool CSupportDlg::IsTestAccount()
{
	switch (m_id) {
				
		// The following patients allow the three module fields
		// "Status," "Uses," and "Exp Date" to be edited in patients > support by
		// anyone.

		// Can find the number to add here by SELECT ID FROM PersonT WHERE ID = (SELECT PersonID from PatientsT WHERE UserDefinedID = X)
		// where X = the Practice Patient ID

		case 2010:		// Thompson, Don
		case 124:		// Gruber, Jennie
		case 12310:		// Lally, Eric
		case 113:		// Cardillo, Bob
		case 125:		// Schneider, Tom
		case 13253:		// Walling, Adam
		case 12463:		// Manning, Zack
		case 45206:		// Luckoski, Jason
		case 12266:		// Main, test
		case 19430:		// Main, ZCustom Records
		case 25252:		// Main, ZRefractive
		case 21076:		// Main, ZStandardMods
		case 50888:		// Main, UnitTest
			return true;
			break;
		default:
			return false;
	}		

	return false;
}

void CSupportDlg::OnIntegration()
{
	try {
		CIntegrationDlg dlg(this, m_id);
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

// (f.dinatale 2010-10-25) - PLID 40827 - Added a button to edit Cell Trust tiers.
void CSupportDlg::OnNxReminderSettings()
{
	try {
		CString strLicense;
		GetDlgItemText(IDC_LICENSE_KEY, strLicense);

		CNxReminderSettingsDlg dlg(strLicense, this);
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2011-10-03) - PLID 45791 - Concurrent TS Licensing
void CSupportDlg::OnBnClickedConcTsListBtn()
{
	try {
		CString strTemp; 
		
		if(IsAzureRemoteAppEnabled()) {
			// (r.gonet 2016-05-10) - NX-100478 - When the client has Azure RemoteApp on, it is useless to configure the machine names
			// allowed for terminal servers. Those names will be ignored. Don't lock them out though.
			MessageBox("This client is licensed for Azure RemoteApp. It is not necessary to configure allowed terminal server names for Azure RemoteApp clients. "
				"Any configured allowed machine names in this window will be ignored.", "Warning", MB_ICONWARNING|MB_OK);
		}

		CConcTsMachinePoolDlg dlg(this);
		dlg.m_nClientID = m_id;
		dlg.m_bIsTestAccount = IsTestAccount();
		//dlg.DoModal();

		// (b.eyers 2015-03-10) - PLID 65208 - Machine Pool dlg needs to know the bought and in use for Con. TS
		GetDlgItemText(IDC_B_CONC_TS_LIC, strTemp);
		dlg.SetBConc(atoi(strTemp));
		GetDlgItemText(IDC_U_CONC_TS_LIC, strTemp);
		dlg.SetUConc(atoi(strTemp));

		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-10-04) PLID 28219 - Use guided dialog to edit licensing options
// (d.singleton 2012-01-06 16:27) - PLID 28219 i totally changed this item after c.copits left,  new dialog and everything.
void CSupportDlg::OnBnClickedChangeLicenses()
{
	try {
		CString strTemp;

		CSupportEditLicenseDlg dlg;
		dlg.m_Color = GetCurrentColor();
		dlg.m_nPatientID = m_id;
		dlg.m_bIsTestAccount = IsTestAccount();

		// Pass in bought values
		GetDlgItemText(IDC_B_LIC, strTemp);
		dlg.SetBLicenses(atoi(strTemp));

		GetDlgItemText(IDC_IPAD_BOUGHT, strTemp);
		dlg.SetBIpads(atoi(strTemp));

		GetDlgItemText(IDC_B_CONC_TS_LIC, strTemp);
		dlg.SetBConc(atoi(strTemp));

		GetDlgItemText(IDC_B_DOC, strTemp);
		dlg.SetBDoctors(atoi(strTemp));

		GetDlgItemText(IDC_B_NEXSYNC, strTemp);
		dlg.SetBNexSync(atoi(strTemp));

		GetDlgItemText(IDC_B_DICTATION, strTemp);
		dlg.SetBDictation(atoi(strTemp));

		GetDlgItemText(IDC_B_PALM, strTemp);
		dlg.SetBPalm(atoi(strTemp));

		GetDlgItemText(IDC_B_PPC, strTemp);
		dlg.SetBNexPDA(atoi(strTemp));

		GetDlgItemText(IDC_B_DATABASES, strTemp);
		dlg.SetBDatabases(atoi(strTemp));

		GetDlgItemText(IDC_B_EMRPROV, strTemp);
		dlg.SetBEMRProv(atoi(strTemp));

		GetDlgItemText(IDC_B_NEXWEB_DOMAINS, strTemp); // (r.ries 2012-01-22) - PLID 54767
		dlg.SetBNxWeb(atoi(strTemp));

		//(r.farnworth 2013-07-19) PLID 57522
		GetDlgItemText(IDC_B_LICPRES, strTemp);
		dlg.SetBLicensedPres(atoi(strTemp));

		//(r.farnworth 2013-07-19) PLID 57522
		GetDlgItemText(IDC_B_MIDPRES, strTemp);
		dlg.SetBMidPres(atoi(strTemp));

		//(r.farnworth 2013-07-19) PLID 57522
		GetDlgItemText(IDC_B_STAFFPRES, strTemp);
		dlg.SetBStaffPres(atoi(strTemp));

		// (z.manning 2015-06-17 14:31) - PLID 66278 - Portal provider fields
		GetDlgItemText(IDC_B_PORTAL_PROVIDERS, strTemp);
		dlg.SetBPortalProviders(atoi(strTemp));

		// Pass in pending values
		GetDlgItemText(IDC_P_LIC, strTemp);
		dlg.SetPLicenses(atoi(strTemp));

		GetDlgItemText(IDC_IPAD_PENDING, strTemp);
		dlg.SetPIpads(atoi(strTemp));

		GetDlgItemText(IDC_P_CONC_TS_LIC, strTemp);
		dlg.SetPConc(atoi(strTemp));

		GetDlgItemText(IDC_P_DOC, strTemp);
		dlg.SetPDoctors(atoi(strTemp));

		GetDlgItemText(IDC_P_NEXSYNC, strTemp);
		dlg.SetPNexSync(atoi(strTemp));

		GetDlgItemText(IDC_P_DICTATION, strTemp);
		dlg.SetPDictation(atoi(strTemp));

		GetDlgItemText(IDC_P_PALM, strTemp);
		dlg.SetPPalm(atoi(strTemp));

		GetDlgItemText(IDC_P_PPC, strTemp);
		dlg.SetPNexPDA(atoi(strTemp));

		GetDlgItemText(IDC_P_DATABASES, strTemp);
		dlg.SetPDatabases(atoi(strTemp));

		GetDlgItemText(IDC_P_EMRPROV, strTemp);
		dlg.SetPEMRProv(atoi(strTemp));

		GetDlgItemText(IDC_P_NEXWEB_DOMAINS, strTemp); // (r.ries 2012-01-22) - PLID 54767
		dlg.SetPNxWeb(atoi(strTemp));

		//(r.farnworth 2013-07-19) PLID 57522
		GetDlgItemText(IDC_P_LICPRES, strTemp);
		dlg.SetPLicensedPres(atoi(strTemp));

		//(r.farnworth 2013-07-19) PLID 57522
		GetDlgItemText(IDC_P_MIDPRES, strTemp);
		dlg.SetPMidPres(atoi(strTemp));

		//(r.farnworth 2013-07-19) PLID 57522
		GetDlgItemText(IDC_P_STAFFPRES, strTemp);
		dlg.SetPStaffPres(atoi(strTemp));

		// (z.manning 2015-06-17 14:33) - PLID 66278
		GetDlgItemText(IDC_P_PORTAL_PROVIDERS, strTemp);
		dlg.SetPPortalProviders(atoi(strTemp));

		// Pass in in-use values
		GetDlgItemText(IDC_U_LIC, strTemp);
		dlg.SetIULicenses(atoi(strTemp));

		GetDlgItemText(IDC_IPAD_INUSE, strTemp);
		dlg.SetIUIpads(atoi(strTemp));

		GetDlgItemText(IDC_U_CONC_TS_LIC, strTemp);
		dlg.SetIUConc(atoi(strTemp));

		GetDlgItemText(IDC_U_DOC, strTemp);
		dlg.SetIUDoctors(atoi(strTemp));

		GetDlgItemText(IDC_U_NEXSYNC, strTemp);
		dlg.SetIUNexSync(atoi(strTemp));

		GetDlgItemText(IDC_U_DICTATION, strTemp);
		dlg.SetIUDictation(atoi(strTemp));

		GetDlgItemText(IDC_U_PALM, strTemp);
		dlg.SetIUPalm(atoi(strTemp));

		GetDlgItemText(IDC_U_PPC, strTemp);
		dlg.SetIUNexPDA(atoi(strTemp));

		GetDlgItemText(IDC_U_DATABASES, strTemp);
		dlg.SetIUDatabases(atoi(strTemp));

		GetDlgItemText(IDC_U_EMRPROV, strTemp);  // (r.ries 2012-01-24) - PLID 54767 Accidentally removed this, adding it back. Don't mind me
		dlg.SetIUEMRProv(atoi(strTemp));

		GetDlgItemText(IDC_U_NEXWEB_DOMAINS, strTemp); // (r.ries 2012-01-22) - PLID 54767
		dlg.SetIUNxWeb(atoi(strTemp));

		GetDlgItemText(IDC_MAX_PATIENT_COUNT, strTemp); // (r.ries 2013-01-24) - PLID 54820
		dlg.SetIUNumPatients(atoi(strTemp));

		GetDlgItemText(IDC_MAX_PRACTICE_COUNT, strTemp); // (r.ries 2013-01-24) - PLID 54820
		dlg.SetIULoginAttempts(atoi(strTemp));
		
		 
		 
			
		GetDlgItemText(IDC_VPN, strTemp); // (s.tullis 2014-01-30 15:20) - PLID 57553 - Nexcloud in Practice support tab change number of licences
		dlg.SetIUNxCloudVPN(atoi(strTemp));

		GetDlgItemText(IDC_Users, strTemp); // (s.tullis 2014-01-30 15:20) - PLID 57553 - Nexcloud in Practice support tab change number of licences
		dlg.SetIUNxCloudUsers(atoi(strTemp));

		

		//(r.farnworth 2013-07-19) PLID 57522
		GetDlgItemText(IDC_U_LICPRES, strTemp);
		dlg.SetIULicensedPres(atoi(strTemp));

		//(r.farnworth 2013-07-19) PLID 57522
		GetDlgItemText(IDC_U_MIDPRES, strTemp);
		dlg.SetIUMidPres(atoi(strTemp));

		//(r.farnworth 2013-07-19) PLID 57522
		GetDlgItemText(IDC_U_STAFFPRES, strTemp);
		dlg.SetIUStaffPres(atoi(strTemp));

		// (z.manning 2015-06-17 14:34) - PLID 66278
		GetDlgItemText(IDC_U_PORTAL_PROVIDERS, strTemp);
		dlg.SetIUPortalProviders(atoi(strTemp));


		CString strSwapDescription;

		if ( dlg.DoModal() == IDOK) {
			//find out the license and type of action to take with license and assigned values to bought or pending fields
			switch(dlg.m_ltidLicenseType)
			{
			case ltidLicenses:
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBLicenses);
							SetDlgItemText(IDC_B_LIC, strTemp);
							Save(IDC_B_LIC);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPLicenses);
							SetDlgItemText(IDC_P_LIC, strTemp);
							Save(IDC_P_LIC);
						}
						break;
					case atSwap:
						{
							// (r.ries 2013-1-4 PLID) - 50881 - Support swap case
							strTemp.Format("%li", dlg.m_nBLicenses);
							SetDlgItemText(IDC_B_LIC, strTemp);
							strTemp.Format("%li", dlg.m_nLicenseEditAmt);
							m_bBeginNewLockoutPeriod = dlg.bBeginNewLockoutPeriod; // (b.eyers 2015-03-05) - PLID 65098 - get lockout bool from dlg
							strSwapDescription = SwapLicenseWith(dlg.m_ltidLicenseTypeSwap, abs(atol(strTemp)));
							if (!strSwapDescription.IsEmpty())
							{
								strSwapDescription = Save(IDC_B_LIC, strSwapDescription);
								if (strSwapDescription.IsEmpty())
								{
									AfxMessageBox("WARNING: There was a problem with the swap. The Workstation License change did NOT save. Please check the audit history.");
								}
							}
							else
							{
								strTemp.Format("%li", dlg.m_nBLicenses + dlg.m_nLicenseEditAmt);
								SetDlgItemText(IDC_B_LIC, strTemp);
							}
						}
						break;
					default:;
					}
				}
				break;
			case ltidIpads: // (z.manning 2012-06-12 15:57) - PLID 50878
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBIpads);
							SetDlgItemText(IDC_IPAD_BOUGHT, strTemp);
							Save(IDC_IPAD_BOUGHT);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPIpads);
							SetDlgItemText(IDC_IPAD_PENDING, strTemp);
							Save(IDC_IPAD_PENDING);
						}
						break;
					case atSwap:
						{
							// (r.ries 2013-1-4 PLID) - 50881 - Support swap case
							strTemp.Format("%li", dlg.m_nBIpads);
							SetDlgItemText(IDC_IPAD_BOUGHT, strTemp);
							strTemp.Format("%li", dlg.m_nLicenseEditAmt);
							m_bBeginNewLockoutPeriod = dlg.bBeginNewLockoutPeriod; // (b.eyers 2015-03-05) - PLID 65098 - get lockout bool from dlg
							strSwapDescription = SwapLicenseWith(dlg.m_ltidLicenseTypeSwap, abs(atol(strTemp)));
							if (!strSwapDescription.IsEmpty())
							{
								strSwapDescription = Save(IDC_IPAD_BOUGHT, strSwapDescription);
								if (strSwapDescription.IsEmpty())
								{
									AfxMessageBox("WARNING: There was a problem with the swap. The iPad License change did NOT save. Please check the audit history.");
								}
							}
							else
							{
								strTemp.Format("%li", dlg.m_nBIpads + dlg.m_nLicenseEditAmt);
								SetDlgItemText(IDC_IPAD_BOUGHT, strTemp);
							}
						}
						break;
					default:;
					}
				}
				break;
			case ltidConcLicenses:
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBConc);
							SetDlgItemText(IDC_B_CONC_TS_LIC, strTemp);
							Save(IDC_B_CONC_TS_LIC);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPConc);
							SetDlgItemText(IDC_P_CONC_TS_LIC, strTemp);
							Save(IDC_P_CONC_TS_LIC);
						}
						break;
					case atSwap:
						{
							// (r.ries 2013-1-4 PLID) - 50881 - Support swap case
							strTemp.Format("%li", dlg.m_nBConc);
							SetDlgItemText(IDC_B_CONC_TS_LIC, strTemp);
							strTemp.Format("%li", dlg.m_nLicenseEditAmt);
							m_bBeginNewLockoutPeriod = dlg.bBeginNewLockoutPeriod; // (b.eyers 2015-03-05) - PLID 65098 - get lockout bool from dlg
							strSwapDescription = SwapLicenseWith(dlg.m_ltidLicenseTypeSwap, abs(atol(strTemp)));
							if (!strSwapDescription.IsEmpty())
							{
								strSwapDescription = Save(IDC_B_CONC_TS_LIC, strSwapDescription);
								if (strSwapDescription.IsEmpty())
								{
									AfxMessageBox("WARNING: There was a problem with the swap. The Conc. TS License change did NOT save. Please check the audit history.");
								}
							}
							else
							{
								strTemp.Format("%li", dlg.m_nBConc + dlg.m_nLicenseEditAmt);
								SetDlgItemText(IDC_B_CONC_TS_LIC, strTemp);
							}
						}
						break;
					default:;
					}
				}
				break;
			case ltidNumDoctors:
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBDoctors);
							SetDlgItemText(IDC_B_DOC, strTemp);
							Save(IDC_B_DOC);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPDoctors);
							SetDlgItemText(IDC_P_DOC, strTemp);
							Save(IDC_P_DOC);
						}
						break;
					default:;
					}
				}
				break;

			case ltidNexsync:
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBNexSync);
							SetDlgItemText(IDC_B_NEXSYNC, strTemp);
							Save(IDC_B_NEXSYNC);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPNexSync);
							SetDlgItemText(IDC_P_NEXSYNC, strTemp);
							Save(IDC_P_NEXSYNC);
						}
						break;
					case atSwap:
						{
						// (b.eyers 2015-02-24) - PLID 64987 - Ability to swap licenses between Palm Pilot/NexPDA/NexSync
							strTemp.Format("%li", dlg.m_nBNexSync);
							SetDlgItemText(IDC_B_NEXSYNC, strTemp);
							strTemp.Format("%li", dlg.m_nLicenseEditAmt);
							m_bBeginNewLockoutPeriod = dlg.bBeginNewLockoutPeriod; // (b.eyers 2015-03-05) - PLID 65098 - get lockout bool from dlg
							strSwapDescription = SwapLicenseWith(dlg.m_ltidLicenseTypeSwap, abs(atol(strTemp)));
							if (!strSwapDescription.IsEmpty())
							{
								strSwapDescription = Save(IDC_B_NEXSYNC, strSwapDescription);
								if (strSwapDescription.IsEmpty())
								{
									AfxMessageBox("WARNING: There was a problem with the swap. The NexSync change did NOT save. Please check the audit history.");
								}
							}
							else
							{
								strTemp.Format("%li", dlg.m_nBNexSync + dlg.m_nLicenseEditAmt);
								SetDlgItemText(IDC_B_NEXSYNC, strTemp);
							}
						}
						break;
					}
				}
				break;

			case ltidDictation: // (z.manning 2014-01-31 14:13) - PLID 55147
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBDictation);
							SetDlgItemText(IDC_B_DICTATION, strTemp);
							Save(IDC_B_DICTATION);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPDictation);
							SetDlgItemText(IDC_P_DICTATION, strTemp);
							Save(IDC_P_DICTATION);
						}
						break;
					}
				}
				break;

			case ltidPalmPilot:
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBPalm);
							SetDlgItemText(IDC_B_PALM, strTemp);
							Save(IDC_B_PALM);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPPalm);
							SetDlgItemText(IDC_P_PALM, strTemp);
							Save(IDC_P_PALM);
						}
						break;
					case atSwap:
						{
						// (b.eyers 2015-02-24) - PLID 64987 - Ability to swap licenses between Palm Pilot/NexPDA/NexSync
							strTemp.Format("%li", dlg.m_nBPalm);
							SetDlgItemText(IDC_B_PALM, strTemp);
							strTemp.Format("%li", dlg.m_nLicenseEditAmt);
							m_bBeginNewLockoutPeriod = dlg.bBeginNewLockoutPeriod; // (b.eyers 2015-03-05) - PLID 65098 - get lockout bool from dlg
							strSwapDescription = SwapLicenseWith(dlg.m_ltidLicenseTypeSwap, abs(atol(strTemp)));
							if (!strSwapDescription.IsEmpty())
							{
								strSwapDescription = Save(IDC_B_PALM, strSwapDescription);
								if (strSwapDescription.IsEmpty())
								{
									AfxMessageBox("WARNING: There was a problem with the swap. The Palm Pilot change did NOT save. Please check the audit history.");
								}
							}
							else
							{
								strTemp.Format("%li", dlg.m_nBPalm + dlg.m_nLicenseEditAmt);
								SetDlgItemText(IDC_B_PALM, strTemp);
							}
						}
						break;
					default:;
					}
				}
				break;
			case ltidNexPDA:
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBNexPDA);
							SetDlgItemText(IDC_B_PPC, strTemp);
							Save(IDC_B_PPC);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPNexPDA);
							SetDlgItemText(IDC_P_PPC, strTemp);
							Save(IDC_P_PPC);
						}
						break;
					case atSwap:
						{
						// (b.eyers 2015-02-24) - PLID 64987 - Ability to swap licenses between Palm Pilot/NexPDA/NexSync
							strTemp.Format("%li", dlg.m_nBNexPDA);
							SetDlgItemText(IDC_B_PPC, strTemp);
							strTemp.Format("%li", dlg.m_nLicenseEditAmt);
							m_bBeginNewLockoutPeriod = dlg.bBeginNewLockoutPeriod; // (b.eyers 2015-03-05) - PLID 65098 - get lockout bool from dlg
							strSwapDescription = SwapLicenseWith(dlg.m_ltidLicenseTypeSwap, abs(atol(strTemp)));
							if (!strSwapDescription.IsEmpty())
							{
								strSwapDescription = Save(IDC_B_PPC, strSwapDescription);
								if (strSwapDescription.IsEmpty())
								{
									AfxMessageBox("WARNING: There was a problem with the swap. The NexPDA change did NOT save. Please check the audit history.");
								}
							}
							else
							{
								strTemp.Format("%li", dlg.m_nBNexPDA + dlg.m_nLicenseEditAmt);
								SetDlgItemText(IDC_B_PPC, strTemp);
							}
						}
						break;
					default:;
					}
				}
				break;
			case ltidDatabases:
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBDatabases);
							SetDlgItemText(IDC_B_DATABASES, strTemp);
							Save(IDC_B_DATABASES);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPDatabases);
							SetDlgItemText(IDC_P_DATABASES, strTemp);
							Save(IDC_P_DATABASES);
						}
						break;
					default:;
					}
				}
				break;
			case ltidEMRProv:
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBEMRProv);
							SetDlgItemText(IDC_B_EMRPROV, strTemp);
							Save(IDC_B_EMRPROV);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPEMRProv);
							SetDlgItemText(IDC_P_EMRPROV, strTemp);
							Save(IDC_P_EMRPROV);
						}
						break;
					default:;
					}
				}
				break;
			case ltidNxWeb: // (r.ries 2012-01-22) - PLID 54767
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBNxWeb);
							SetDlgItemText(IDC_B_NEXWEB_DOMAINS, strTemp);
							Save(IDC_B_NEXWEB_DOMAINS);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPNxWeb);
							SetDlgItemText(IDC_P_NEXWEB_DOMAINS, strTemp);
							Save(IDC_P_NEXWEB_DOMAINS);
						}
						break;
					default:;
					}
				}
				break;

		
			case ltidNxCloudUser: // (s.tullis 2014-01-30 17:02) - PLID 57553 - Nexcloud in Practice support tab change number of licences
				{
					if (dlg.m_atType == atAddOn || dlg.m_atType == atDeactivate)
					{	
						if(IsDlgButtonChecked(IDC_NexCloud)== BST_CHECKED)
						{
							strTemp.Format("%li", dlg.m_nIUNxCloudUser);
							SetDlgItemText(IDC_Users, strTemp);
							Save(IDC_Users);
						}
						else 
							MessageBox("NexCloud needs to be enabled before you can change #Users");
					}
				}
				break;
			
			case ltidNxCloudVPN: // (s.tullis 2014-01-30 17:04) - PLID 57553 - Nexcloud in Practice support tab change number of licences
				{
					if (dlg.m_atType == atAddOn || dlg.m_atType == atDeactivate)
					{
						if(IsDlgButtonChecked(IDC_NexCloud)== BST_CHECKED)
						{
							strTemp.Format("%li", dlg.m_nIUNxCloudVPN);
							SetDlgItemText(IDC_VPN, strTemp);
							Save(IDC_VPN);
						}
						else 
							MessageBox("NexCloud needs to be enabled before you can change #VPN");
					}
				}
				break;
			case ltidMaxLogins: // (r.ries 2013-01-24) - PLID 54820
				{
					if (dlg.m_atType == atAddOn || dlg.m_atType == atDeactivate)
					{
						strTemp.Format("%li", dlg.m_nIULoginAttempts);
						SetDlgItemText(IDC_MAX_PRACTICE_COUNT, strTemp);
						Save(IDC_MAX_PRACTICE_COUNT);
					}
				}
				break;
			//(r.farnworth 2013-07-19) PLID 57522
			case ltidLicensedPres:
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBLicPres);
							SetDlgItemText(IDC_B_LICPRES, strTemp);
							Save(IDC_B_LICPRES);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPLicPres);
							SetDlgItemText(IDC_P_LICPRES, strTemp);
							Save(IDC_P_LICPRES);
						}
						break;
					default:;
					}
				}
				break;
			//(r.farnworth 2013-07-19) PLID 57522
			case ltidMidPres:
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBMidPres);
							SetDlgItemText(IDC_B_MIDPRES, strTemp);
							Save(IDC_B_MIDPRES);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPMidPres);
							SetDlgItemText(IDC_P_MIDPRES, strTemp);
							Save(IDC_P_MIDPRES);
						}
						break;
					default:;
					}
				}
				break;
			//(r.farnworth 2013-07-19) PLID 57522
			case ltidStaffPres:
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBStaffPres);
							SetDlgItemText(IDC_B_STAFFPRES, strTemp);
							Save(IDC_B_STAFFPRES);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPStaffPres);
							SetDlgItemText(IDC_P_STAFFPRES, strTemp);
							Save(IDC_P_STAFFPRES);
						}
						break;
					default:;
					}
				}
				break;

			// (z.manning 2015-06-17 14:36) - PLID 66278
			case ltidPortalProviders:
				{
					switch(dlg.m_atType)
					{
					case atAddOn:
						{
							strTemp.Format("%li", dlg.m_nBPortalProviders);
							SetDlgItemText(IDC_B_PORTAL_PROVIDERS, strTemp);
							Save(IDC_B_PORTAL_PROVIDERS);
						}
						break;
					case atReactivate:
					case atDeactivate:
						{
							strTemp.Format("%li", dlg.m_nPPortalProviders);
							SetDlgItemText(IDC_P_PORTAL_PROVIDERS, strTemp);
							Save(IDC_P_PORTAL_PROVIDERS);
						}
						break;
					default:;
					}
				}
				break;

			default:;
			}
		}
		UpdateView(); // (r.ries 2013-17-3) - PLID 54643
	} NxCatchAll(__FUNCTION__);
}

// (r.ries 2013-1-3) - PLID 50881 - Adds license to "Bought" for license type and saves, returns what the user input in the description field of the Audit dialog
CString CSupportDlg::SwapLicenseWith(eLicenseTypeID ltidLicenseTypeSwap, long lamtToSwap)
{
	CSupportEditLicenseDlg dlg;
	long longTemp;
	CString strTemp;

	switch (ltidLicenseTypeSwap)
	{
	case ltidLicenses:
		{
			GetDlgItemText(IDC_B_LIC,strTemp);
			longTemp = atoi(strTemp) + lamtToSwap;
			strTemp.Format("%li", longTemp);
			SetDlgItemText(IDC_B_LIC, strTemp);
			return Save(IDC_B_LIC);
		}
		break;
	case ltidIpads:
		{
			if (!AreIpadsEnabled()) // (r.ries 2013-2-20) - PLID 55244 - Auto check "Enable iPad" checkbox if licenses are swapped to iPad
			{
				CheckDlgButton(IDC_ENABLE_IPADS_CHECK, BST_CHECKED);
				EnableIpads("Auto-enabled by system");
			}
			GetDlgItemText(IDC_IPAD_BOUGHT,strTemp);
			longTemp = atoi(strTemp) + lamtToSwap;
			strTemp.Format("%li", longTemp);
			SetDlgItemText(IDC_IPAD_BOUGHT, strTemp);
			return Save(IDC_IPAD_BOUGHT);
		}
		break;
	case ltidConcLicenses:
		{
			GetDlgItemText(IDC_B_CONC_TS_LIC,strTemp);
			longTemp = atoi(strTemp) + lamtToSwap;
			strTemp.Format("%li", longTemp);
			SetDlgItemText(IDC_B_CONC_TS_LIC, strTemp);
			return Save(IDC_B_CONC_TS_LIC);
		}
		break;
	case ltidNexsync:
		{
		// (b.eyers 2015-02-24) - PLID 64987 - Ability to swap licenses between Palm Pilot/NexPDA/NexSync
			GetDlgItemText(IDC_B_NEXSYNC, strTemp);
			longTemp = atoi(strTemp) + lamtToSwap;
			strTemp.Format("%li", longTemp);
			SetDlgItemText(IDC_B_NEXSYNC, strTemp);
			return Save(IDC_B_NEXSYNC);
		}
		break;
	case ltidPalmPilot:
		{
		// (b.eyers 2015-02-24) - PLID 64987 - Ability to swap licenses between Palm Pilot/NexPDA/NexSync
			GetDlgItemText(IDC_B_PALM, strTemp);
			longTemp = atoi(strTemp) + lamtToSwap;
			strTemp.Format("%li", longTemp);
			SetDlgItemText(IDC_B_PALM, strTemp);
			return Save(IDC_B_PALM);
		}
		break;
	case ltidNexPDA:
		{
		// (b.eyers 2015-02-24) - PLID 64987 - Ability to swap licenses between Palm Pilot/NexPDA/NexSync
			GetDlgItemText(IDC_B_PPC, strTemp);
			longTemp = atoi(strTemp) + lamtToSwap;
			strTemp.Format("%li", longTemp);
			SetDlgItemText(IDC_B_PPC, strTemp);
			return Save(IDC_B_PPC);
		}
		break;
	default:
		return "";
	}
}


/* r.wilson Oct 5 2011 PLID 45836 */
void CSupportDlg::OnSelChosenAaoForms(long nRow) 
{
	try {
		// (j.luckoski 2012-06-06 14:24) - PLID 50114 - Removed the NULL and relaced it with "" because of params
		CString strType = "";
		CString strNew;
		if(nRow != sriNoRow) {
			long nID = VarLong(m_pASPSForms->GetValue(nRow, 0));
			if(nID > 0) {
				strType.Format("%li", nID);
				strNew = VarString(m_pAAOForms->GetValue(nRow, 1), "");
			}
		}

		//for auditing
		CString field = "AAOForms";
		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		_RecordsetPtr prs = CreateParamRecordset(FormatString("SELECT %s FROM NxClientsT WHERE PersonID = {INT}", field), m_id);

		CString strOld;
		if(!prs->eof) {
			_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
			if(var.vt == VT_I4) {
				if(VarLong(var) == 1)
					strOld = "From AAO";
				else if(VarLong(var) == 2)
					strOld = "From NexTech";
			}
		}

		if(strOld == strNew)
			return;	//didnt change anything

		CAuditSupportDlg dlg(this);
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				//reset the selection
				if(!prs->eof) {
					_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
					if(var.vt == VT_I4)
						m_pAAOForms->SetSelByColumn(0, var);
					else
						m_pAAOForms->PutCurSel(sriNoRow);
				}
				else
					m_pAAOForms->PutCurSel(sriNoRow);
				return;
			}
		}

		SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), field, strOld, strNew, _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils

		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		ExecuteParamSql("UPDATE NxClientsT SET AAOForms = {STRING} WHERE PersonID = {INT}", strType, m_id);

	} NxCatchAll("Error in OnSelChosenAAOForms");
}

void CSupportDlg::OnKillFocusAaoPurchDate() 
{
	try {
		// (j.luckoski 2012-06-05 17:45) - PLID 50114 - Use var and {VT_DATE } instead of {STRING} for the new params
		COleDateTime dt;
		dt = m_pAAOPurchDate->GetDateTime();
		_variant_t varDate;

		CString strNew;
		if(m_pAAOPurchDate->GetStatus() != 1)
			varDate.vt = VT_NULL;
		else {
			COleVariant varDateExtract(dt);
			varDate = varDateExtract;
			strNew = FormatDateTimeForInterface(dt, dtoDate);
		}

		//for auditing
		CString field = "AAOPurchaseDate";
		
		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Parametrized sql statements
		_RecordsetPtr prs = CreateParamRecordset(FormatString("SELECT %s FROM NxClientsT WHERE PersonID = {INT}", field), m_id);

		CString strOld;
		if(!prs->eof) {
			_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
			if(var.vt == VT_DATE)
				strOld = FormatDateTimeForInterface(VarDateTime(var), dtoDate);
		}

		if(strOld == strNew)
			return;	//didnt change anything

		CAuditSupportDlg dlg(this);
		if(!m_bHideForNewClient) {
			if(dlg.DoModal() == IDCANCEL) {
				//reset the selection
				if(!prs->eof) {
					_variant_t var = prs->Fields->Item[_bstr_t(field)]->Value;
					if(var.vt == VT_DATE)
						m_pAAOPurchDate->SetDateTime(VarDateTime(var));
					else
						m_pAAOPurchDate->Clear();
				}
				else
					m_pAAOPurchDate->Clear();
				return;
			}
		}

		SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), field, strOld, strNew == "NULL" ? "" : strNew,  _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils

		// (j.luckoski 2012-05-01 17:23) - PLID 50114 - Paramterized sql statements
		ExecuteParamSql("UPDATE NxClientsT SET AAOPurchaseDate = {VT_DATE} WHERE PersonID = {INT}", varDate, m_id);
	} NxCatchAll("Error saving AAO Purchase date.");
}

// (z.manning 2012-06-12 17:28) - PLID 50878
BOOL CSupportDlg::AreIpadsEnabled()
{
	return (IsDlgButtonChecked(IDC_ENABLE_IPADS_CHECK) == BST_CHECKED);
}

void CSupportDlg::EnableIpads(const CString& strPreparedAuditText) // (r.ries 2013-2-20) - PLID 55244 - Need to call this if systen automatically checks "Enable iPads" if a swap is performed
{
	try
	{	
		CString strOld = IsDlgButtonChecked(IDC_ENABLE_IPADS_CHECK) == BST_CHECKED ? "Disabled" : "Enabled";
		CString strNew = IsDlgButtonChecked(IDC_ENABLE_IPADS_CHECK) == BST_CHECKED ? "Enabled" : "Disabled";
		if(strPreparedAuditText.IsEmpty())
		{
			CAuditSupportDlg dlg(this);
			if(!m_bHideForNewClient)
			{
				if(dlg.DoModal() == IDCANCEL) 
				{
					CheckDlgButton(IDC_ENABLE_IPADS_CHECK, IsDlgButtonChecked(IDC_ENABLE_IPADS_CHECK) ? BST_UNCHECKED : BST_CHECKED);
					return;
				}

				SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "IpadsEnabled", strOld, strNew,  _Q(dlg.m_strText)); // (r.ries 2013-01-09) - PLID 54643 - Moved AuditSupportItem to SupportUtils
			}
		}
		else
		{
			SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "IpadsEnabled", strOld, strNew, strPreparedAuditText);
		}

		// (z.manning 2012-06-12 17:04) - PLID 50878 - There is actually a "hidden" license object to control enabling iPads
		// in addition to the iPad bought field. So when this box is checked set that field to in use and then to not bought
		// when unchecked.
		SupportItemStatus eEnabledStatus = IsDlgButtonChecked(IDC_ENABLE_IPADS_CHECK) == BST_CHECKED ? sisActive : sisNotBought;
		ExecuteParamSql(
			"UPDATE NxClientsT SET IpadEnabled = {INT} WHERE PersonID = {INT}"
			, eEnabledStatus, m_id);

		if(!m_bItemsEnabled) {
			EnableBoughtUsedLicenseFields(TRUE);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-05-10) - NX-100478 - Returns whether the client has Azure RemoteApp licensed or not.
bool CSupportDlg::IsAzureRemoteAppEnabled()
{
	return (IsDlgButtonChecked(IDC_AZURE_REMOTE_APP_CHECK) == BST_CHECKED);
}

void CSupportDlg::EnableAzureRemoteApp()
{
	try {
		CString strOld = IsDlgButtonChecked(IDC_AZURE_REMOTE_APP_CHECK) == BST_CHECKED ? "Disabled" : "Enabled";
		CString strNew = IsDlgButtonChecked(IDC_AZURE_REMOTE_APP_CHECK) == BST_CHECKED ? "Enabled" : "Disabled";
		CAuditSupportDlg dlg(this);
		if (!m_bHideForNewClient) {
			if (dlg.DoModal() == IDCANCEL) {
				CheckDlgButton(IDC_AZURE_REMOTE_APP_CHECK, IsDlgButtonChecked(IDC_AZURE_REMOTE_APP_CHECK) ? BST_UNCHECKED : BST_CHECKED);
				return;
			}
			
			SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "AzureRemoteApp", strOld, strNew, _Q(dlg.m_strText));
		}

		// There is actually a "hidden" license object to control enabling Azure Remote App. So when this box is checked, set that field to in use
		// and then to not bought when unchecked.
		SupportItemStatus eEnabledStatus = IsDlgButtonChecked(IDC_AZURE_REMOTE_APP_CHECK) == BST_CHECKED ? sisActive : sisNotBought;
		ExecuteParamSql(
			"UPDATE NxClientsT SET AzureRemoteApp = {INT} WHERE PersonID = {INT}"
			, eEnabledStatus, m_id);

		if (!m_bItemsEnabled) {
			EnableBoughtUsedLicenseFields(TRUE);
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CSupportDlg::LoadNexCloudStatus(BOOL bValue)//l.banegas PLID 57275 this methad is called in OnEnableSupportItems()
{
	if(bValue == FALSE){
	     CheckDlgButton(IDC_NexCloud, BST_UNCHECKED);
	}
	else{
		CheckDlgButton(IDC_NexCloud, BST_CHECKED);
	} 

}
void CSupportDlg::OnBnClickedNexcloud()//l.banegas PLID 57275 adds sets the cliant as NexCloud User or Deactivates User as NexCloud user
{
	try{
		CString strOld = IsDlgButtonChecked(IDC_NexCloud) == BST_CHECKED ? "Disabled" : "Enabled";
		CString strNew = IsDlgButtonChecked(IDC_NexCloud) == BST_CHECKED ? "Enabled" : "Disabled";
		CAuditSupportDlg dlg(this);
		if(dlg.DoModal() == IDCANCEL) 
		{
			CheckDlgButton(IDC_NexCloud, IsDlgButtonChecked(IDC_NexCloud) ? BST_UNCHECKED : BST_CHECKED);
			return;
		}
		else{
			SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "NexCloud Enabled User", strOld, strNew,  _Q(dlg.m_strText)); 
			int nValue = ((CButton*)GetDlgItem(IDC_NexCloud))->GetCheck();
			ExecuteParamSql("UPDATE NxClientsT SET NexCloud ={INT} WHERE PersonID={INT}",nValue,m_id);
		}
			
	}NxCatchAll(__FUNCTION__);
	
}

void CSupportDlg::OnBnClickedChangeAnchorPassword()
{
	try
	{
		CAnchorChangePasswordDlg dlg(m_id, GetCurrentUserName(), this);
		if (dlg.DoModal() == IDOK)
		{
			// (j.armen 2014-09-02 13:57) - PLID 57853 - Store the old password just in case we need to revert
			CString strOldPassword = m_strAnchorPassword;

			try
			{
				// (j.armen 2013-10-03 07:23) - PLID 57853 - Don't reload the entire dialog, just get what we need
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT AnchorPassword FROM NxClientsT WHERE PersonID = {INT}", m_id);

				_variant_t ciphertext = AdoFldVar(prs, "AnchorPassword", g_cvarNull);
				m_strAnchorPassword = DecryptStringFromVariant(ciphertext);

				// Push our members to dialog
				UpdateData(FALSE);

				PushAnchorData();
			} NxCatchAllSilentCallThrow(
			{
				// (j.armen 2014-09-02 13:57) - PLID 57853 - If we failed for any reason (most likly the admin service rejected us)
				// revert the anchor password back to what it was previously
				SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "Anchor Password", "<Encrypted>", "<Encrypted>", _Q("Failed to update anchor password"));

				m_strAnchorPassword = strOldPassword;
				_variant_t vtAnchorPassword = EncryptStringToVariant(m_strAnchorPassword);

				// Push our members to dialog
				UpdateData(FALSE);

				ExecuteParamSql(R"(
UPDATE NxClientsT
	SET AnchorPassword = {VARBINARY}
WHERE PersonID = {INT})",
vtAnchorPassword,
m_id);
			});
		}
	}NxCatchAll(__FUNCTION__);
}

void CSupportDlg::OnAnchorCallerCountFocusLost()
{
	try
	{
		// First store the old value
		long nOldAnchorCallerCount = m_nAnchorCallerCount;
		CString strOldAnchorPassword = m_strAnchorPassword;

		// Update our members
		if (!UpdateData(TRUE))
		{
			m_nAnchorCallerCount = nOldAnchorCallerCount;
			m_strAnchorPassword = strOldAnchorPassword;
			return;
		}
			

		if (AsString(m_nAnchorCallerCount) != m_nxeditAnchorCallerCount.GetText())
		{
			AfxMessageBox("Enter an integer.", MB_ICONEXCLAMATION);
			m_nxeditAnchorCallerCount.SetFocus();
			return;
		}

		// Process any changes
		if (nOldAnchorCallerCount != m_nAnchorCallerCount)
		{
			// (j.armen 2014-09-02 14:01) - PLID 57853 - Audit
			CAuditSupportDlg dlg(this);
			if (dlg.DoModal() == IDCANCEL)
			{
				m_nAnchorCallerCount = nOldAnchorCallerCount;
				m_strAnchorPassword = strOldAnchorPassword;
				UpdateData(FALSE); // Push members to interface
			}
			else
			{
				try
				{
					// (j.armen 2014-09-02 14:01) - PLID 57853 - Audit caller count first
					SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "Anchor Caller Count", AsString(nOldAnchorCallerCount), AsString(m_nAnchorCallerCount), _Q(dlg.m_strText));

					// (j.armen 2013-10-03 07:23) - PLID 57853 - If we are moving from 0 and the password has not been set,
					//	we generate a new password.
					if (nOldAnchorCallerCount == 0 && m_strAnchorPassword.IsEmpty())
					{
						CAnchorChangePasswordDlg pwdChangeDlg(m_id, GetCurrentUserName(), this);
						CNexwebLoginGeneratePasswordDlg pwdGenerate(this, boost::bind(&CAnchorChangePasswordDlg::GenerateComplexity, &pwdChangeDlg));
						m_strAnchorPassword = pwdGenerate.GeneratePasswordText();
						// Push our members to dialog
						UpdateData(FALSE);

						// Audit that we are setting the password for the first time
						SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "Anchor Password", "", "<Encrypted>", _Q(dlg.m_strText));
					}
					else if (m_nAnchorCallerCount == 0 && !m_strAnchorPassword.IsEmpty())
					{
						m_strAnchorPassword = "";
						SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "Anchor Password", "<Encrypted>", "", _Q(dlg.m_strText));
					}

					_variant_t vtAnchorPassword = EncryptStringToVariant(m_strAnchorPassword);

					ExecuteParamSql("UPDATE NxClientsT SET AnchorPassword = {VARBINARY}, AnchorCallerCount = {INT} WHERE PersonID = {INT}",
						vtAnchorPassword,
						m_nAnchorCallerCount,
						m_id);

					PushAnchorData();

					GetDlgItem(IDC_BUTTON_CHANGE_ANCHOR_PASS)->EnableWindow(!!m_nAnchorCallerCount);
				} NxCatchAllSilentCallThrow(
				{
					// (j.armen 2014-09-02 14:01) - PLID 57853 - Audit failure, and revert data
					SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "Anchor Caller Count", AsString(m_nAnchorCallerCount), AsString(nOldAnchorCallerCount), _Q("Failed to update anchor caller count"));

					m_nAnchorCallerCount = nOldAnchorCallerCount;
					m_strAnchorPassword = strOldAnchorPassword;
					UpdateData(FALSE); // Push members to interface

					_variant_t vtAnchorPassword = EncryptStringToVariant(m_strAnchorPassword);

					// (j.armen 2013-10-03 07:32) - PLID 57853 - Execute Sql
					ExecuteParamSql("UPDATE NxClientsT SET AnchorPassword = {VARBINARY}, AnchorCallerCount = {INT} WHERE PersonID = {INT}",
						vtAnchorPassword,
						m_nAnchorCallerCount,
						m_id);
				})
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CSupportDlg::PushAnchorData()
{
	// (j.armen 2014-09-02 14:01) - PLID 57853 - For sanity, if/when Internal is coppied to Internal Copy, the config values would also be coppied.
	// To prevent headaches later, use a code comparison for the current admin service site, and make sure that if anyone other than internal calls it,
	// we will fail.
	if (VarString(GetAPISubkey()).CompareNoCase("Internal") != 0 &&
		GetRemotePropertyText("NexWebAdminURL", "", 0, "<None>", false).CompareNoCase("https://nexweb.nextech.com/NexWebAdmin/NexWebAdminService.asmx") == 0)
	{
		AfxThrowNxException("Cannot connect to the live NexWeb Admin Service from a database other than Internal");
	}

	// (j.armen 2013-10-03 08:09) - PLID 57853 - Credentials are stored in configrt
	CNexWebAdminServiceAccessor s(
		"https://nexweb.nextech.com/NexWebAdmin",
		GetRemotePropertyText("NexWebAdminURL", "", 0, "<None>", false),
		GetRemotePropertyText("NexWebAdminUsername", "", 0, "<None>", false),
		DecryptStringFromVariant(GetRemotePropertyImage("NexWebAdminPassword", 0, "<None>", false)));

	CWaitCursor c;

	// (j.armen 2013-10-03 07:32) - PLID 57853 - Update Web Service
	s.UpdateAnchorLicense(
		atol(m_nxeditLicenseKey.GetText()),
		GetActivePatientName());
}

// (r.goldschmidt 2015-04-06 17:31) - PLID 65479 - Manage Analytics Licenses dialog
void CSupportDlg::OnBnClickedAnalyticslicensingBtn()
{
	try {
		CAnalyticsLicensingDlg dlg(this, m_id);
		dlg.m_bIsTestAccount = IsTestAccount();
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-12-18 09:47) - PLID 67738
void CSupportDlg::OnBnClickedEditDictationInfo()
{
	try
	{
		PromptForDictationInfo();
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2015-12-18 10:47) - PLID 67738
int CSupportDlg::PromptForDictationInfo()
{
	CDictationEditInfoDlg dlg(GetDlgItemInt(IDC_LICENSE_KEY), this);
	return dlg.DoModal();
}

// (b.eyers - 2016-03-03) - PLID 68480 - Subscription Local Hosted
void CSupportDlg::OnIsSubscriptionLocalHosted()
{
	try {
		CAuditSupportDlg dlg(this);
		if (!m_bHideForNewClient) {
			if (dlg.DoModal() == IDCANCEL) {
				CheckDlgButton(IDC_SUBSCRIPTION_LOCAL_HOSTED, IsDlgButtonChecked(IDC_SUBSCRIPTION_LOCAL_HOSTED) ? BST_UNCHECKED : BST_CHECKED);
			}

			CString strOld = IsDlgButtonChecked(IDC_SUBSCRIPTION_LOCAL_HOSTED) ? "Inactive" : "Active";
			CString strNew = IsDlgButtonChecked(IDC_SUBSCRIPTION_LOCAL_HOSTED) ? "Active" : "Inactive";

			SupportUtils::AuditSupportItem(GetActivePatientID(), GetCurrentUserName(), "Subscription Local Hosted", strOld, strNew, _Q(dlg.m_strText));
		}

		ExecuteParamSql(
			"UPDATE NxClientsT SET SubscriptionLocalHosted = {INT} WHERE PersonID = {INT}",
			IsDlgButtonChecked(IDC_SUBSCRIPTION_LOCAL_HOSTED) ? 1 : 0,
			m_id);
	} NxCatchAll(__FUNCTION__);
}
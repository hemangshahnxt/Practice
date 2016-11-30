// BoldSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "BoldSettingsDlg.h"
#include "FinancialRc.h"


// CBoldSettingsDlg dialog
// (j.gruber 2010-06-01 12:25) - PLID 38817 - created for
IMPLEMENT_DYNAMIC(CBoldSettingsDlg, CNxDialog)

CBoldSettingsDlg::CBoldSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CBoldSettingsDlg::IDD, pParent)
{

}

CBoldSettingsDlg::~CBoldSettingsDlg()
{
}

void CBoldSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BOLD_PREPRODUCTION, m_rdPreProduction);
	DDX_Control(pDX, IDC_BOLD_PRODUCTION, m_rdProduction);
	DDX_Control(pDX, IDC_BOLD_MESSAGE, m_stMessage);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BOLD_SETTING_PRACTICEID, m_edtPracCOEID);
}


BEGIN_MESSAGE_MAP(CBoldSettingsDlg, CNxDialog)	
	ON_BN_CLICKED(IDOK, &CBoldSettingsDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CBoldSettingsDlg message handlers
BOOL CBoldSettingsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		//Set the icons on the NxIconButtons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
	
		//Preferences
		g_propManager.CachePropertiesInBulk("BOLDSettings", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("			
			" Name = 'BOLDProductionMode' "	
			")",
			_Q(GetCurrentUserName()));

		// (j.gruber 2010-05-28 14:20) - PLID 38932 - Practice COEID
		// (j.gruber 2011-05-17 14:03) - PLID 43744 - add production services
		g_propManager.CachePropertiesInBulk("BOLDSettings", propText,
			"(Username = '<None>' OR Username = '%s') AND ("			
			" Name = 'BOLDPracticeCOEID' OR "						
			" Name = 'BOLDProductionServices' OR "
			" Name = 'BOLDPreProductionServices' "
			")",
			_Q(GetCurrentUserName()));

		long nProduction = GetRemotePropertyInt("BOLDProductionMode", 0, 0, "<None>", true);

		if (nProduction == 1) {
			CheckDlgButton(IDC_BOLD_PRODUCTION, 1);
			CheckDlgButton(IDC_BOLD_PREPRODUCTION, 0);
			//m_rdProduction.SetCheck(1);
			//m_rdPreProduction.SetCheck(0);
		}
		else {
			CheckDlgButton(IDC_BOLD_PRODUCTION, 0);
			CheckDlgButton(IDC_BOLD_PREPRODUCTION, 1);
			//m_rdProduction.SetCheck(0);
			//m_rdPreProduction.SetCheck(1);
		}

		//you are the NexTech tech support user, OR in debug mode
		BOOL bEnableProduction = FALSE;
		if(GetCurrentUserID() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID) {
			bEnableProduction = TRUE;
		}

#ifdef DEBUG
		bEnableProduction = TRUE;
#endif

		//m_rdPreProduction.EnableWindow(bEnableProduction);
		//m_rdProduction.EnableWindow(bEnableProduction);			
		GetDlgItem(IDC_BOLD_PRODUCTION)->EnableWindow(bEnableProduction);
		GetDlgItem(IDC_BOLD_PREPRODUCTION)->EnableWindow(bEnableProduction);
		// (j.gruber 2011-05-17 14:04) - PLID 43744 - do the same for the production services
		GetDlgItem(IDC_BOLD_SETTING_PRODUCTION_SERVICES)->EnableWindow(bEnableProduction);
		GetDlgItem(IDC_BOLD_SETTING_PREPRODUCTION_SERVICES)->EnableWindow(bEnableProduction);

		
		// (j.gruber 2010-05-28 14:21) - PLID 38932 - Practice COEID
		CString strPracCOEID = GetRemotePropertyText("BOLDPracticeCOEID", "", 0, "<None>", true);
		SetDlgItemText(IDC_BOLD_SETTING_PRACTICEID, strPracCOEID);

		// (j.gruber 2011-05-17 14:04) - PLID 43744 - set the dialog box
		CString strProductionServices = GetRemotePropertyText("BOLDProductionServices", "https://service.surgicalreview.org/bold/BoldService.asmx", 0, "<None>");
		SetDlgItemText(IDC_BOLD_SETTING_PRODUCTION_SERVICES, strProductionServices);

		CString strPreProductionServices = GetRemotePropertyText("BOLDPreProductionServices", "https://service.surgicalreview.org/boldstaging/BoldService.asmx", 0, "<None>");
		SetDlgItemText(IDC_BOLD_SETTING_PREPRODUCTION_SERVICES, strPreProductionServices);

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



void CBoldSettingsDlg::OnBnClickedOk()
{
	try {

		if (IsDlgButtonChecked(IDC_BOLD_PREPRODUCTION)) {
			SetRemotePropertyInt("BOLDProductionMode", 0, 0, "<None>");
		}else {
			SetRemotePropertyInt("BOLDProductionMode", 1, 0, "<None>");
		}
		// (j.gruber 2010-05-28 14:18) - PLID 38932 - Practice COE Username
		CString strPracCOEID;
		GetDlgItemText(IDC_BOLD_SETTING_PRACTICEID, strPracCOEID);
		SetRemotePropertyText("BOLDPracticeCOEID", strPracCOEID, 0, "<None>");

		// (j.gruber 2011-05-17 14:08) - PLID 43744
		CString strProductionServices;
		GetDlgItemText(IDC_BOLD_SETTING_PRODUCTION_SERVICES, strProductionServices);
		SetRemotePropertyText("BOLDProductionServices", strProductionServices, 0 , "<None>");

		CString strPreProductionServices;
		GetDlgItemText(IDC_BOLD_SETTING_PREPRODUCTION_SERVICES, strPreProductionServices);
		SetRemotePropertyText("BOLDPreProductionServices", strPreProductionServices, 0 , "<None>");

		OnOK();
	}NxCatchAll(__FUNCTION__);
}

// HL7SchedSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialRc.h"
#include "HL7SchedSettingsDlg.h"
#include "HL7Utils.h"

using namespace NXDATALIST2Lib;

// CHL7SchedSettingsDlg dialog
// (z.manning 2011-04-21 10:51) - PLID 43361 - Created

IMPLEMENT_DYNAMIC(CHL7SchedSettingsDlg, CNxDialog)

CHL7SchedSettingsDlg::CHL7SchedSettingsDlg(const long nHL7GroupID, CWnd* pParent)
	: CNxDialog(CHL7SchedSettingsDlg::IDD, pParent)
	, m_nHL7GroupID(nHL7GroupID)
{

}

CHL7SchedSettingsDlg::~CHL7SchedSettingsDlg()
{
}

void CHL7SchedSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CHL7SchedSettingsDlg, CNxDialog)
END_MESSAGE_MAP()


// CHL7SchedSettingsDlg message handlers

BOOL CHL7SchedSettingsDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		// (b.savon 2014-12-09 13:35) - PLID 64320 - Remove the Schedule Export HL7 Version dropdown from Advanced Settings for Appts. and make this into a hidden setting.

		// (r.farnworth 2014-12-08 14:54) - PLID 64333 - Create an option in the HL7 configuration for SIU messages only that will determine if the PV1-7 segment should be filled with the General 1 provider or the linked provider to the resource of the appointment.
		int  nSchedProvider = GetHL7SettingInt(m_nHL7GroupID, "SchedulerProvider");
		if (nSchedProvider == 0) {
			CheckRadioButton(IDC_PROV_LINKED, IDC_PROV_GEN1, IDC_PROV_LINKED);
		}
		else if (nSchedProvider == 1) {
			CheckRadioButton(IDC_PROV_LINKED, IDC_PROV_GEN1, IDC_PROV_GEN1);
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CHL7SchedSettingsDlg::OnOK()
{
	try
	{
		if(ValidateAndSave()) {
			CNxDialog::OnOK();
		}

	}NxCatchAll(__FUNCTION__);
}

BOOL CHL7SchedSettingsDlg::ValidateAndSave()
{
	//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
	// (b.savon 2014-12-09 13:35) - PLID 64320 - Remove the Schedule Export HL7 Version dropdown from Advanced Settings for Appts. and make this into a hidden setting.

	// (r.farnworth 2014-12-08 15:22) - PLID 64333 - Create an option in the HL7 configuration for SIU messages only that will determine if the PV1-7 segment should be filled with the General 1 provider or the linked provider to the resource of the appointment.
	if (IsDlgButtonChecked(IDC_PROV_LINKED)) {
		SetHL7SettingInt(m_nHL7GroupID, "SchedulerProvider", 0);
	}
	else if (IsDlgButtonChecked(IDC_PROV_GEN1)) {
		SetHL7SettingInt(m_nHL7GroupID, "SchedulerProvider", 1);
	}
	
	CClient::RefreshTable(NetUtils::HL7SettingsT, m_nHL7GroupID);
	RefreshHL7Group(m_nHL7GroupID);

	return TRUE;
}
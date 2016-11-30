// HL7PatientSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialRc.h"
#include "HL7PatientSettingsDlg.h"
#include "HL7Utils.h"
#include "HL7ConfigCodeLinksDlg.h"
#include "HL7ParseUtils.h"

// CHL7PatientSettingsDlg dialog
// (z.manning 2010-10-04 17:26) - PLID 40795 - Created

IMPLEMENT_DYNAMIC(CHL7PatientSettingsDlg, CNxDialog)

CHL7PatientSettingsDlg::CHL7PatientSettingsDlg(long nHL7GroupID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7PatientSettingsDlg::IDD, pParent)
{
	m_nHL7GroupID = nHL7GroupID;
}

CHL7PatientSettingsDlg::~CHL7PatientSettingsDlg()
{
}

void CHL7PatientSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PID_18_USE_OVERRIDE, m_nxbPID18UseOverride);
	DDX_Control(pDX, IDC_PID_18_OVERRIDE, m_nxePID18Override);
	DDX_Control(pDX, IDC_USE_IN1_3_2, m_nxbUseIn1_3_2);
	DDX_Control(pDX, IDC_WORKERS_COMP_IN1_31, m_nxbWorkersCompIn1_31);
	DDX_Control(pDX, IDC_REQUIRE_PATIENT_IDS_MAP, m_nxbRequirePatientIDMap);
}


BEGIN_MESSAGE_MAP(CHL7PatientSettingsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_PID_18_USE_OVERRIDE, &CHL7PatientSettingsDlg::OnPid18UseOverride)
	ON_BN_CLICKED(IDC_CONFIG_ETHNICITY, &CHL7PatientSettingsDlg::OnBnClickedConfigEthnicity)
	ON_BN_CLICKED(IDC_CONFIG_RACE, &CHL7PatientSettingsDlg::OnBnClickedConfigRace)
	ON_BN_CLICKED(IDC_CONFIG_LANGUAGE, &CHL7PatientSettingsDlg::OnBnClickedConfigLanguage)
END_MESSAGE_MAP()


// CHL7PatientSettingsDlg message handlers

BOOL CHL7PatientSettingsDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
		CheckDlgButton(IDC_A28_ADD_NEW_ONLY, GetHL7SettingBit(m_nHL7GroupID, "A28AddNewOnly") ? BST_CHECKED : BST_UNCHECKED);

		//TES 9/16/2011 - PLID 45537 - Added an option to override the PID-18 field
		CheckDlgButton(IDC_PID_18_USE_OVERRIDE, GetHL7SettingBit(m_nHL7GroupID, "PID18_UseOverride") ? BST_CHECKED : BST_UNCHECKED);
		ReflectPid18UseOverride();
		SetDlgItemText(IDC_PID_18_OVERRIDE, GetHL7SettingText(m_nHL7GroupID, "PID18_Override"));

		//TES 9/16/2011 - PLID 45538 - Added an option to send the insurance code in IN1-3.2
		CheckDlgButton(IDC_USE_IN1_3_2, GetHL7SettingBit(m_nHL7GroupID, "Use_IN1_3_2"));

		//TES 2/27/2012 - PLID 48331 - Send a Worker's Comp flag in IN1-31
		CheckDlgButton(IDC_WORKERS_COMP_IN1_31, GetHL7SettingBit(m_nHL7GroupID, "WorkersComp_IN1_31"));

		// (d.thompson 2012-06-05) - PLID 50551 - Require Patient ID Map
		CheckDlgButton(IDC_REQUIRE_PATIENT_IDS_MAP, GetHL7SettingBit(m_nHL7GroupID, "RequirePatientIDMap"));

		// (d.thompson 2012-08-28) - PLID 52129 - Options for IN1-15.
		{
			long nValue = GetHL7SettingInt(m_nHL7GroupID, "IN1_15");
			if(nValue == 1) {
				//Use ins plan name
				CheckDlgButton(IDC_IN1_15_PLAN_NAME, 1);
				CheckDlgButton(IDC_IN1_15_INS_TYPE, 0);
			}
			else {
				//Default option - use type
				CheckDlgButton(IDC_IN1_15_PLAN_NAME, 0);
				CheckDlgButton(IDC_IN1_15_INS_TYPE, 1);
			}
		}

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CHL7PatientSettingsDlg::OnOK()
{
	try
	{
		BOOL bA28AddNewOnly = (IsDlgButtonChecked(IDC_A28_ADD_NEW_ONLY) == BST_CHECKED);

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 settings
		SetHL7SettingBit(m_nHL7GroupID, "A28AddNewOnly", bA28AddNewOnly);
		
		//TES 9/16/2011 - PLID 45537 - Added an option to override the PID-18 field
		SetHL7SettingBit(m_nHL7GroupID, "PID18_UseOverride", IsDlgButtonChecked(IDC_PID_18_USE_OVERRIDE));
		CString strPID18Override;
		GetDlgItemText(IDC_PID_18_OVERRIDE, strPID18Override);
		SetHL7SettingText(m_nHL7GroupID, "PID18_Override", strPID18Override);

		//TES 9/16/2011 - PLID 45538 - Added an option to send the insurance code in IN1-3.2
		SetHL7SettingBit(m_nHL7GroupID, "Use_IN1_3_2", IsDlgButtonChecked(IDC_USE_IN1_3_2));

		//TES 2/27/2012 - PLID 48331 - Send a Worker's Comp flag in IN1-31
		SetHL7SettingBit(m_nHL7GroupID, "WorkersComp_IN1_31", IsDlgButtonChecked(IDC_WORKERS_COMP_IN1_31));

		// (d.thompson 2012-06-05) - PLID 50551 - Require Patient ID Map
		SetHL7SettingBit(m_nHL7GroupID, "RequirePatientIDMap", IsDlgButtonChecked(IDC_REQUIRE_PATIENT_IDS_MAP));

		// (d.thompson 2012-08-28) - PLID 52129 - Options for IN1-15.  We'll set this as integer values (in case a
		//	third option is defined later - it's a rather vague field).
		//	0 - Default option - Use Insurance Type (this was the only option available before 10.2)
		//	1 - Use Insurance Plan Name
		{
			long nValue = 0;
			if(IsDlgButtonChecked(IDC_IN1_15_PLAN_NAME)) {
				nValue = 1;
			}
			else {
				//Default or IDC_IN1_15_INS_TYPE
				nValue = 0;
			}
			SetHL7SettingInt(m_nHL7GroupID, "IN1_15", nValue);
		}

		RefreshHL7Group(m_nHL7GroupID);
		CClient::RefreshTable(NetUtils::HL7SettingsT, m_nHL7GroupID);

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}
void CHL7PatientSettingsDlg::OnPid18UseOverride()
{
	try {
		ReflectPid18UseOverride();
	}NxCatchAll(__FUNCTION__);
}


void CHL7PatientSettingsDlg::ReflectPid18UseOverride()
{
	//TES 9/16/2011 - PLID 45537 - Disable the override field if the box to use it is unchecked
	GetDlgItem(IDC_PID_18_OVERRIDE)->EnableWindow(IsDlgButtonChecked(IDC_PID_18_USE_OVERRIDE));
}

// (d.thompson 2012-08-21) - PLID 52048
void CHL7PatientSettingsDlg::OnBnClickedConfigEthnicity()
{
	try {
		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = m_nHL7GroupID;
		dlg.m_hclrtType = hclrtEthnicity;
		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2012-08-22) - PLID 52047
void CHL7PatientSettingsDlg::OnBnClickedConfigRace()
{
	try {
		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = m_nHL7GroupID;
		dlg.m_hclrtType = hclrtRace;
		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2012-08-23) - PLID 52049
void CHL7PatientSettingsDlg::OnBnClickedConfigLanguage()
{
	try {
		CHL7ConfigCodeLinksDlg dlg(this);
		dlg.m_nHL7GroupID = m_nHL7GroupID;
		dlg.m_hclrtType = hclrtLanguage;
		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);
}

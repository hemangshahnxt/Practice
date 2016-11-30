// OHIPValidationConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "OHIPValidationConfigDlg.h"

// COHIPValidationConfigDlg dialog

// (j.jones 2008-12-03 11:15) - PLID 32258 - created

COHIPValidationConfigDlg::COHIPValidationConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(COHIPValidationConfigDlg::IDD, pParent)
{

}

void COHIPValidationConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COHIPValidationConfigDlg)
	DDX_Control(pDX, IDC_CHECK_PATIENT_HEALTH_NUMBER_VERIFICATION, m_checkPatHealthNumVerify);
	DDX_Control(pDX, IDC_CHECK_MOH_OFFICE_CODE, m_checkMOHOfficeCode);
	DDX_Control(pDX, IDC_CHECK_GROUP_NUMBER, m_checkGroupNumber);
	DDX_Control(pDX, IDC_CHECK_PROVIDER_SPECIALTY_CODE, m_checkProvSpecialty);
	DDX_Control(pDX, IDC_CHECK_PROVIDER_NPI, m_checkProvNPI);
	DDX_Control(pDX, IDC_CHECK_LOCATION_NPI, m_checkLocNPI);
	DDX_Control(pDX, IDC_CHECK_POS_NPI, m_checkPOSNPI);
	DDX_Control(pDX, IDC_CHECK_REF_PHY_SELECTED, m_checkRefPhySelected);
	DDX_Control(pDX, IDC_CHECK_REF_PHY_NPI, m_checkRefPhyNPI);
	DDX_Control(pDX, IDC_CHECK_SERVICE_CODE, m_checkServiceCode);
	DDX_Control(pDX, IDC_CHECK_PATIENT_HEALTH_NUMBER, m_checkPatHealthNum);
	DDX_Control(pDX, IDC_CHECK_PATIENT_VERSION_CODE, m_checkPatVersionCode);
	DDX_Control(pDX, IDC_CHECK_REGISTRATION_NUMBER, m_checkRegistrationNum);
	DDX_Control(pDX, IDC_CHECK_PATIENT_NAME, m_checkPatName);
	DDX_Control(pDX, IDC_CHECK_PATIENT_BIRTHDATE, m_checkPatBirthdate);
	DDX_Control(pDX, IDC_CHECK_PATIENT_GENDER, m_checkPatGender);
	DDX_Control(pDX, IDC_CHECK_PATIENT_PROVINCE, m_checkPatProvince);
	DDX_Control(pDX, IDC_CHECK_DIAGNOSIS_CODE, m_checkDiagCode);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_OHIP_CHECK_VALIDATE_DUPLICATE_ECLAIM, m_btnExportedTwiceInSameDay);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COHIPValidationConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(COHIPValidationConfigDlg)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_PATIENT_HEALTH_NUMBER, &COHIPValidationConfigDlg::OnCheckPatientHealthNumber)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COHIPValidationConfigDlg message handlers

BOOL COHIPValidationConfigDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		g_propManager.CachePropertiesInBulk("OHIPValidationConfigDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'OHIPValidateMOHOfficeCode' OR "
			"Name = 'OHIPValidateGroupNumber' OR "
			"Name = 'OHIPValidateSpecialty' OR "
			"Name = 'OHIPValidateProviderNPI' OR "
			"Name = 'OHIPValidateLocationNPI' OR "
			"Name = 'OHIPValidatePOSNPI' OR "
			"Name = 'OHIPValidateRefPhySelected' OR "
			"Name = 'OHIPValidateRefPhyNPI' OR "
			"Name = 'OHIPValidateServiceCode' OR "
			"Name = 'OHIPValidatePatientHealthNumber' OR "
			"Name = 'OHIPValidatePatientVersionCode' OR "
			"Name = 'OHIPValidateRegistrationNumber' OR "
			"Name = 'OHIPValidatePatientName' OR "
			"Name = 'OHIPValidatePatientBirthdate' OR "
			"Name = 'OHIPValidatePatientGender' OR "
			"Name = 'OHIPValidatePatientProvince' OR "
			"Name = 'OHIPValidateDiagCode'  OR "
			// (j.jones 2008-12-10 11:22) - PLID 32312 - added patient health number verification
			"Name = 'OHIPValidatePatientHealthNumberVerify' OR "
			// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
			"Name = 'OHIPValidateExportedTwiceInSameDay' "
			")",
			_Q(GetCurrentUserName()));
		
		m_checkMOHOfficeCode.SetCheck(GetRemotePropertyInt("OHIPValidateMOHOfficeCode",1,0,"<None>",TRUE) == 1);
		m_checkGroupNumber.SetCheck(GetRemotePropertyInt("OHIPValidateGroupNumber",1,0,"<None>",TRUE) == 1);
		m_checkProvSpecialty.SetCheck(GetRemotePropertyInt("OHIPValidateSpecialty",1,0,"<None>",TRUE) == 1);
		m_checkProvNPI.SetCheck(GetRemotePropertyInt("OHIPValidateProviderNPI",1,0,"<None>",TRUE) == 1);
		m_checkLocNPI.SetCheck(GetRemotePropertyInt("OHIPValidateLocationNPI",1,0,"<None>",TRUE) == 1);
		m_checkPOSNPI.SetCheck(GetRemotePropertyInt("OHIPValidatePOSNPI",1,0,"<None>",TRUE) == 1);
		m_checkRefPhySelected.SetCheck(GetRemotePropertyInt("OHIPValidateRefPhySelected",1,0,"<None>",TRUE) == 1);
		m_checkRefPhyNPI.SetCheck(GetRemotePropertyInt("OHIPValidateRefPhyNPI",1,0,"<None>",TRUE) == 1);
		m_checkServiceCode.SetCheck(GetRemotePropertyInt("OHIPValidateServiceCode",1,0,"<None>",TRUE) == 1);
		m_checkPatHealthNum.SetCheck(GetRemotePropertyInt("OHIPValidatePatientHealthNumber",1,0,"<None>",TRUE) == 1);
		//the version code validation defaults to off
		m_checkPatVersionCode.SetCheck(GetRemotePropertyInt("OHIPValidatePatientVersionCode",0,0,"<None>",TRUE) == 1);
		m_checkRegistrationNum.SetCheck(GetRemotePropertyInt("OHIPValidateRegistrationNumber",1,0,"<None>",TRUE) == 1);
		m_checkPatName.SetCheck(GetRemotePropertyInt("OHIPValidatePatientName",1,0,"<None>",TRUE) == 1);
		m_checkPatBirthdate.SetCheck(GetRemotePropertyInt("OHIPValidatePatientBirthdate",1,0,"<None>",TRUE) == 1);
		m_checkPatGender.SetCheck(GetRemotePropertyInt("OHIPValidatePatientGender",1,0,"<None>",TRUE) == 1);
		m_checkPatProvince.SetCheck(GetRemotePropertyInt("OHIPValidatePatientProvince",1,0,"<None>",TRUE) == 1);
		m_checkDiagCode.SetCheck(GetRemotePropertyInt("OHIPValidateDiagCode",1,0,"<None>",TRUE) == 1);
		// (j.jones 2008-12-10 11:33) - PLID 32312 - added patient health number verification
		m_checkPatHealthNumVerify.SetCheck(GetRemotePropertyInt("OHIPValidatePatientHealthNumberVerify",1,0,"<None>",TRUE) == 1);
		OnCheckPatientHealthNumber();
		// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
		m_btnExportedTwiceInSameDay.SetCheck(GetRemotePropertyInt("OHIPValidateExportedTwiceInSameDay",1,0,"<None>",TRUE) == 1);

	}NxCatchAll("Error in COHIPValidationConfigDlg::OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void COHIPValidationConfigDlg::OnOK() 
{
	try {

		SetRemotePropertyInt("OHIPValidateMOHOfficeCode", m_checkMOHOfficeCode.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidateGroupNumber", m_checkGroupNumber.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidateSpecialty", m_checkProvSpecialty.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidateProviderNPI", m_checkProvNPI.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidateLocationNPI", m_checkLocNPI.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidatePOSNPI", m_checkPOSNPI.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidateRefPhySelected", m_checkRefPhySelected.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidateRefPhyNPI", m_checkRefPhyNPI.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidateServiceCode", m_checkServiceCode.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidatePatientHealthNumber", m_checkPatHealthNum.GetCheck() ? 1 : 0, 0, "<None>");		
		SetRemotePropertyInt("OHIPValidatePatientVersionCode", m_checkPatVersionCode.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidateRegistrationNumber", m_checkRegistrationNum.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidatePatientName", m_checkPatName.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidatePatientBirthdate", m_checkPatBirthdate.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidatePatientGender", m_checkPatGender.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidatePatientProvince", m_checkPatProvince.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("OHIPValidateDiagCode", m_checkDiagCode.GetCheck() ? 1 : 0, 0, "<None>");
		// (j.jones 2008-12-10 11:33) - PLID 32312 - added patient health number verification
		SetRemotePropertyInt("OHIPValidatePatientHealthNumberVerify", m_checkPatHealthNumVerify.GetCheck() ? 1 : 0, 0, "<None>");
		// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
		SetRemotePropertyInt("OHIPValidateExportedTwiceInSameDay", m_btnExportedTwiceInSameDay.GetCheck() ? 1 : 0, 0, "<None>");
		
		CNxDialog::OnOK();

	}NxCatchAll("Error in COHIPValidationConfigDlg::OnOK");
}

// (j.jones 2008-12-10 11:22) - PLID 32312 - added OnCheckPatientHealthNumber
void COHIPValidationConfigDlg::OnCheckPatientHealthNumber()
{
	try {

		//disable the health number validation if the regular check is disabled
		m_checkPatHealthNumVerify.EnableWindow(m_checkPatHealthNum.GetCheck());

	}NxCatchAll("Error in COHIPValidationConfigDlg::OnCheckPatientHealthNumber");
}

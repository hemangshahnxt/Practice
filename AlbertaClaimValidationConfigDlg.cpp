// AlbertaClaimValidationConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AlbertaClaimValidationConfigDlg.h"

// (j.jones 2010-11-03 09:56) - PLID 41288 - created

// CAlbertaClaimValidationConfigDlg dialog

IMPLEMENT_DYNAMIC(CAlbertaClaimValidationConfigDlg, CNxDialog)

CAlbertaClaimValidationConfigDlg::CAlbertaClaimValidationConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAlbertaClaimValidationConfigDlg::IDD, pParent)
{

}

void CAlbertaClaimValidationConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAlbertaClaimValidationConfigDlg)
	DDX_Control(pDX, IDC_CHECK_PROVIDER_BAID, m_checkProviderBAID);
	DDX_Control(pDX, IDC_CHECK_SUBMITTER_PREFIX, m_checkSubmitterPrefix);
	DDX_Control(pDX, IDC_CHECK_PROVIDER_NPI, m_checkProvNPI);
	DDX_Control(pDX, IDC_CHECK_PROVIDER_TAXONOMY, m_checkProvTaxonomy);
	DDX_Control(pDX, IDC_CHECK_POS_NPI, m_checkPOSNPI);
	DDX_Control(pDX, IDC_CHECK_REF_PHY_SELECTED, m_checkRefPhySelected);
	DDX_Control(pDX, IDC_CHECK_REF_PHY_NPI, m_checkRefPhyNPI);
	DDX_Control(pDX, IDC_CHECK_PATIENT_HEALTH_NUMBER, m_checkPatHealthNum);
	DDX_Control(pDX, IDC_CHECK_REGISTRATION_NUMBER, m_checkRegistrationNum);
	DDX_Control(pDX, IDC_CHECK_PATIENT_NAME, m_checkPatName);
	DDX_Control(pDX, IDC_CHECK_PATIENT_BIRTHDATE, m_checkPatBirthdate);
	DDX_Control(pDX, IDC_CHECK_PATIENT_GENDER, m_checkPatGender);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ALBERTA_CHECK_VALIDATE_DUPLICATE_ECLAIM, m_btnExportedTwiceInSameDay);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAlbertaClaimValidationConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAlbertaClaimValidationConfigDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAlbertaClaimValidationConfigDlg message handlers

BOOL CAlbertaClaimValidationConfigDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		g_propManager.CachePropertiesInBulk("CAlbertaClaimValidationConfigDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'AlbertaValidatePOSNPI' OR "
			"Name = 'AlbertaValidateRefPhySelected' OR "
			"Name = 'AlbertaValidateRefPhyNPI' OR "
			"Name = 'AlbertaValidatePatientBirthdate' OR "
			"Name = 'AlbertaValidatePatientName' OR "
			"Name = 'AlbertaValidatePatientGender' OR "
			"Name = 'AlbertaValidateProviderNPI' OR "
			"Name = 'AlbertaValidateProviderTaxonomyCode' OR "
			"Name = 'AlbertaValidateProviderBAID' OR "
			"Name = 'AlbertaValidateSubmitterPrefix' OR "
			"Name = 'AlbertaValidatePatientHealthNumber' OR "
			"Name = 'AlbertaValidatePatientRegNumber' OR "
			// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
			"Name = 'AlbertaValidateExportedTwiceInSameDay' "
			")",
			_Q(GetCurrentUserName()));
		
		m_checkProviderBAID.SetCheck(GetRemotePropertyInt("AlbertaValidateProviderBAID",1,0,"<None>",TRUE) == 1);
		m_checkSubmitterPrefix.SetCheck(GetRemotePropertyInt("AlbertaValidateSubmitterPrefix",1,0,"<None>",TRUE) == 1);
		m_checkProvNPI.SetCheck(GetRemotePropertyInt("AlbertaValidateProviderNPI",1,0,"<None>",TRUE) == 1);
		m_checkProvTaxonomy.SetCheck(GetRemotePropertyInt("AlbertaValidateProviderTaxonomyCode",1,0,"<None>",TRUE) == 1);
		m_checkPOSNPI.SetCheck(GetRemotePropertyInt("AlbertaValidatePOSNPI",1,0,"<None>",TRUE) == 1);
		m_checkRefPhySelected.SetCheck(GetRemotePropertyInt("AlbertaValidateRefPhySelected",1,0,"<None>",TRUE) == 1);
		m_checkRefPhyNPI.SetCheck(GetRemotePropertyInt("AlbertaValidateRefPhyNPI",1,0,"<None>",TRUE) == 1);
		m_checkPatHealthNum.SetCheck(GetRemotePropertyInt("AlbertaValidatePatientHealthNumber",1,0,"<None>",TRUE) == 1);
		m_checkRegistrationNum.SetCheck(GetRemotePropertyInt("AlbertaValidatePatientRegNumber",1,0,"<None>",TRUE) == 1);
		m_checkPatName.SetCheck(GetRemotePropertyInt("AlbertaValidatePatientName",1,0,"<None>",TRUE) == 1);
		m_checkPatBirthdate.SetCheck(GetRemotePropertyInt("AlbertaValidatePatientBirthdate",1,0,"<None>",TRUE) == 1);
		m_checkPatGender.SetCheck(GetRemotePropertyInt("AlbertaValidatePatientGender",1,0,"<None>",TRUE) == 1);
		// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
		m_btnExportedTwiceInSameDay.SetCheck(GetRemotePropertyInt("AlbertaValidateExportedTwiceInSameDay",1,0,"<None>",TRUE) == 1);

	}NxCatchAll("Error in CAlbertaClaimValidationConfigDlg::OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CAlbertaClaimValidationConfigDlg::OnOK() 
{
	try {

		SetRemotePropertyInt("AlbertaValidateProviderBAID", m_checkProviderBAID.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("AlbertaValidateSubmitterPrefix", m_checkSubmitterPrefix.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("AlbertaValidateProviderNPI", m_checkProvNPI.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("AlbertaValidateProviderTaxonomyCode", m_checkProvTaxonomy.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("AlbertaValidatePOSNPI", m_checkPOSNPI.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("AlbertaValidateRefPhySelected", m_checkRefPhySelected.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("AlbertaValidateRefPhyNPI", m_checkRefPhyNPI.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("AlbertaValidatePatientHealthNumber", m_checkPatHealthNum.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("AlbertaValidatePatientRegNumber", m_checkRegistrationNum.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("AlbertaValidatePatientName", m_checkPatName.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("AlbertaValidatePatientBirthdate", m_checkPatBirthdate.GetCheck() ? 1 : 0, 0, "<None>");
		SetRemotePropertyInt("AlbertaValidatePatientGender", m_checkPatGender.GetCheck() ? 1 : 0, 0, "<None>");
		// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
		SetRemotePropertyInt("AlbertaValidateExportedTwiceInSameDay", m_btnExportedTwiceInSameDay.GetCheck() ? 1 : 0, 0, "<None>");
		
		CNxDialog::OnOK();

	}NxCatchAll("Error in CAlbertaClaimValidationConfigDlg::OnOK");
}
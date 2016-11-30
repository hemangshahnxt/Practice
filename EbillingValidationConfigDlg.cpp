// EbillingValidationConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EbillingValidationConfigDlg.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEbillingValidationConfigDlg dialog


CEbillingValidationConfigDlg::CEbillingValidationConfigDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEbillingValidationConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEbillingValidationConfigDlg)
		m_bIsPaper = FALSE;
		m_bIsANSI = FALSE;
		// (j.jones 2012-10-22 13:36) - PLID 53297 - added ANSI Version
		m_avANSIVersion = av5010;
	//}}AFX_DATA_INIT
}


void CEbillingValidationConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEbillingValidationConfigDlg)
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PROVIDER_24J, m_checkProvider24J);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_REF_PHY_NPI, m_checkRefPhyNPI);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PROVIDER_NPI, m_checkProviderNPI);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_SECONDARY_POLICY_NUM, m_btnSecPolicy);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_SECONDARY_PHONE_NUM, m_btnSecPhone);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_SECONDARY_PAYER_ID, m_btnSecPayer);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_SECONDARY_NSF_CODE, m_btnSecNSF);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_SECONDARY_NAME, m_btnSecName);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_SECONDARY_INSURANCE_ID, m_btnSecInsID);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_SECONDARY_CONTACT_NAME, m_btnSecContactName);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_SECONDARY_BIRTHDATE, m_btnSecDOB);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_SECONDARY_ADDRESS, m_btnSecAddr);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_REF_PHY_ID, m_btnRefPhy);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PROVIDER_TAXONOMY_CODE, m_btnProvTaxonomy);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PROVIDER_OTHER_IDS, m_btnProvANSIREFIDs);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PROVIDER, m_btnProv);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PRIMARY_POLICY_NUM, m_btnPriPolicy);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PRIMARY_PAYER_ID, m_btnPriPayerID);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PRIMARY_NSF_CODE, m_btnPriNSF);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PRIMARY_NAME, m_btnPriName);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PRIMARY_INSURANCE_ID, m_btnPriInsID);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PRIMARY_BIRTHDATE, m_btnPriDOB);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PRIMARY_ADDRESS, m_btnPriAddr);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_POS_NPI, m_btnPOSNPI);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_POS_CODE, m_btnPOSCode);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PATIENT_BIRTHDATE, m_btnPtDOB);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PATIENT_ADDRESS, m_btnPtAddress);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_INVALID_MODIFIERS, m_btnInvalidModifiers);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_HOSP_TO, m_btnHospTo);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_HOSP_FROM, m_btnHospFrom);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_DIAG_POINTERS, m_btnDiagPtrs);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_DIAG_CODE_1, m_btnDiag1);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_NPI_CHECKSUM, m_btnNPIChecksum);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PRIMARY_GENDER, m_btnPriGender);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_SECONDARY_GENDER, m_btnSecGender);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PATIENT_GENDER, m_btnPatGender);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PATIENT_NAME_SELF, m_btnPatInsNamesMatchWhenSelf);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_ASSIGN_BENEFITS, m_btnAssignmentOfBenefits);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_DUPLICATE_ECLAIM, m_btnExportedTwiceInSameDay);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_PRIOR_AUTH_NUMBER, m_btnPriorAuthNum);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_OFFICE_VISITS_IN_GLOBAL_PERIODS, m_btnOfficeVisitGlobalPeriods);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_ACCIDENT_TYPE_FOR_CURRENT_ACCIDENT_DATE, m_btnAccTypeForAccDate);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_REQUIRES_ICD9, m_btnRequiresICD9);
	DDX_Control(pDX, IDC_CHECK_VALIDATE_REQUIRES_ICD10, m_btnRequiresICD10);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEbillingValidationConfigDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEbillingValidationConfigDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEbillingValidationConfigDlg message handlers

BOOL CEbillingValidationConfigDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (j.jones 2008-05-07 15:09) - PLID 29854 - added nxiconbuttons for modernization
	m_btnOK.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);

	// (j.jones 2006-04-25 11:30) - load all the properties at once into the NxPropManager cache
	// Note: this must remain identical to the cache in EbillingValidationDlg.cpp, so ensure
	// that both files have the exact same cache list, whenever this list changes.
	g_propManager.CachePropertiesInBulk("EbillingValidationPrefs", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'EbillingValidatePOSCode' OR "
		"Name = 'EbillingValidatePrimaryPayerID' OR "
		"Name = 'EbillingValidatePrimaryNSFCode' OR "
		"Name = 'EbillingValidatePrimaryName' OR "
		"Name = 'EbillingValidatePrimaryAddress' OR "
		"Name = 'EbillingValidatePrimaryBirthdate' OR "
		"Name = 'EbillingValidatePrimaryInsuranceID' OR "
		"Name = 'EbillingValidatePrimaryPolicyGroupNum' OR "
		"Name = 'EbillingValidateProvider' OR "
		// (j.jones 2011-03-15 10:55) - PLID 42788 - EbillingValidateProviderIDs is now obsolete,
		// but is used to determine the default for EbillingValidateProviderOtherIDs and
		// EbillingValidateProviderBox24JID
		"Name = 'EbillingValidateProviderIDs' OR "
		"Name = 'EbillingValidateProviderOtherIDs' OR "
		"Name = 'EbillingValidateProviderBox24JID' OR "
		"Name = 'EbillingValidateProviderTaxonomyCode' OR "
		"Name = 'EbillingValidateRefPhyID' OR "
		"Name = 'EbillingValidateHospFrom' OR "
		"Name = 'EbillingValidateHospTo' OR "
		"Name = 'EbillingValidateDiagCode1' OR "
		"Name = 'EbillingValidateDiagPointers' OR "
		"Name = 'EbillingValidateInvalidModifiers' OR "
		"Name = 'EbillingValidateSecondaryPayerID' OR "
		"Name = 'EbillingValidateSecondaryNSFCode' OR "
		"Name = 'EbillingValidateSecondaryPhoneNumber' OR "
		"Name = 'EbillingValidateSecondaryContactName' OR "
		"Name = 'EbillingValidateSecondaryName' OR "
		"Name = 'EbillingValidateSecondaryAddress' OR "
		"Name = 'EbillingValidateSecondaryBirthdate' OR "
		"Name = 'EbillingValidateSecondaryInsuranceID' OR "
		"Name = 'EbillingValidateSecondaryPolicyGroupNum' OR "
		"Name = 'EbillingValidatePatientAddress' OR "
		"Name = 'EbillingValidatePatientBirthdate' OR "
		"Name = 'EbillingValidatePOSNPI' OR "
		"Name = 'EbillingValidateNPIChecksum' OR "	// (a.walling 2008-05-20 09:30) - PLID 27812
		"Name = 'EbillingValidateProviderNPI' OR "	// (j.jones 2008-05-21 13:47) - PLID 29280 - added provider and ref. phy. NPI options
		"Name = 'EbillingValidateRefPhyNPI' OR "
		"Name = 'EbillingValidatePrimaryGender' OR "	// (j.jones 2009-08-10 10:55) - PLID 32886 - added gender validations
		"Name = 'EbillingValidateSecondaryGender' OR "
		"Name = 'EbillingValidatePatientGender' OR "
		// (j.jones 2010-01-07 09:18) - PLID 36786 - added ability to disable the Self name checking
		"Name = 'EbillingValidatePatientNameWhenSelf' OR "
		// (j.jones 2010-07-23 15:59) - PLID 39797 - added assignment of benefits validation
		"Name = 'EbillingValidateAssignmentOfBenefits' OR "
		// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
		"Name = 'EbillingValidateExportedTwiceInSameDay' "
		// (j.jones 2012-05-01 09:59) - PLID 48530 - added prior auth. number validation
		"OR Name = 'EbillingValidatePriorAuthNumber' "
		// (j.jones 2012-07-24 17:54) - PLID 51764 - added office visit global period validation
		"OR Name = 'EbillingValidateOfficeVisitGlobalPeriods' "
		// (a.wilson 2013-03-26 16:32) - PLID 51773 - added accident type for current accident date validation.
		"OR Name = 'EbillingValidateAccidentTypeForCurrentAccidentDate' "
		// (d.singleton 2014-03-06 15:23) - PLID 61235 - if ins company requires icd9 codes and none are present on bill need to have validation error in ebilling
		"OR Name = 'EbillingValidateRequiresICD9' "
		// (d.singleton 2014-03-06 15:23) - PLID 61236 - if ins company requires icd10 codes and none are present on bill need to have validation error in ebilling
		"OR Name = 'EbillingValidateRequiresICD10' "
		")",
		_Q(GetCurrentUserName()));

	// (j.jones 2012-10-22 13:36) - PLID 53297 - disable options that have no bearing on 5010 validations
	if(m_bIsANSI && m_avANSIVersion == av5010) {
		GetDlgItem(IDC_CHECK_VALIDATE_SECONDARY_PHONE_NUM)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_VALIDATE_SECONDARY_CONTACT_NAME)->EnableWindow(FALSE);
	}
	
	CheckDlgButton(IDC_CHECK_VALIDATE_POS_CODE, GetRemotePropertyInt("EbillingValidatePOSCode",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_PRIMARY_PAYER_ID, GetRemotePropertyInt("EbillingValidatePrimaryPayerID",1,0,"<None>",TRUE));
	// (j.jones 2009-08-04 12:34) - PLID 14573 - removed THIN ID
	//CheckDlgButton(IDC_CHECK_VALIDATE_PRIMARY_THIN_NUM, GetRemotePropertyInt("EbillingValidatePrimaryTHINNum",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_PRIMARY_NSF_CODE, GetRemotePropertyInt("EbillingValidatePrimaryNSFCode",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_PRIMARY_NAME, GetRemotePropertyInt("EbillingValidatePrimaryName",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_PRIMARY_ADDRESS, GetRemotePropertyInt("EbillingValidatePrimaryAddress",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_PRIMARY_BIRTHDATE, GetRemotePropertyInt("EbillingValidatePrimaryBirthdate",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_PRIMARY_INSURANCE_ID, GetRemotePropertyInt("EbillingValidatePrimaryInsuranceID",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_PRIMARY_POLICY_NUM, GetRemotePropertyInt("EbillingValidatePrimaryPolicyGroupNum", GetRemotePropertyInt("EbillingValidatePolicyGroupNum",1,0,"<None>",TRUE),0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_PROVIDER, GetRemotePropertyInt("EbillingValidateProvider",1,0,"<None>",TRUE));
	// (j.jones 2011-03-15 10:52) - PLID 42788 - renamed this checkbox
	CheckDlgButton(IDC_CHECK_VALIDATE_PROVIDER_OTHER_IDS, GetRemotePropertyInt("EbillingValidateProviderOtherIDs",GetRemotePropertyInt("EbillingValidateProviderIDs",1,0,"<None>",FALSE),0,"<None>",TRUE));
	// (j.jones 2011-03-15 11:11) - PLID 42788 - added Box 24J 
	CheckDlgButton(IDC_CHECK_VALIDATE_PROVIDER_24J, GetRemotePropertyInt("EbillingValidateProviderBox24JID",GetRemotePropertyInt("EbillingValidateProviderIDs",1,0,"<None>",FALSE),0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_PROVIDER_TAXONOMY_CODE, GetRemotePropertyInt("EbillingValidateProviderTaxonomyCode",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_REF_PHY_ID, GetRemotePropertyInt("EbillingValidateRefPhyID",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_HOSP_FROM, GetRemotePropertyInt("EbillingValidateHospFrom",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_HOSP_TO, GetRemotePropertyInt("EbillingValidateHospTo",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_DIAG_CODE_1, GetRemotePropertyInt("EbillingValidateDiagCode1",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_DIAG_POINTERS, GetRemotePropertyInt("EbillingValidateDiagPointers",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_INVALID_MODIFIERS, GetRemotePropertyInt("EbillingValidateInvalidModifiers",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_SECONDARY_PAYER_ID, GetRemotePropertyInt("EbillingValidateSecondaryPayerID",1,0,"<None>",TRUE));
	// (j.jones 2009-08-04 12:34) - PLID 14573 - removed THIN ID
	//CheckDlgButton(IDC_CHECK_VALIDATE_SECONDARY_THIN_NUM, GetRemotePropertyInt("EbillingValidateSecondaryTHINNum",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_SECONDARY_NSF_CODE, GetRemotePropertyInt("EbillingValidateSecondaryNSFCode",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_SECONDARY_PHONE_NUM, GetRemotePropertyInt("EbillingValidateSecondaryPhoneNumber",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_SECONDARY_CONTACT_NAME, GetRemotePropertyInt("EbillingValidateSecondaryContactName",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_SECONDARY_NAME, GetRemotePropertyInt("EbillingValidateSecondaryName",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_SECONDARY_ADDRESS, GetRemotePropertyInt("EbillingValidateSecondaryAddress",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_SECONDARY_BIRTHDATE, GetRemotePropertyInt("EbillingValidateSecondaryBirthdate",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_SECONDARY_INSURANCE_ID, GetRemotePropertyInt("EbillingValidateSecondaryInsuranceID",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_SECONDARY_POLICY_NUM, GetRemotePropertyInt("EbillingValidateSecondaryPolicyGroupNum", GetRemotePropertyInt("EbillingValidatePolicyGroupNum",1,0,"<None>",TRUE),0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_PATIENT_ADDRESS, GetRemotePropertyInt("EbillingValidatePatientAddress",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_PATIENT_BIRTHDATE, GetRemotePropertyInt("EbillingValidatePatientBirthdate",1,0,"<None>",TRUE));
	// (j.jones 2007-03-27 11:31) - PLID 25364 - added option for Place Of Service NPI
	CheckDlgButton(IDC_CHECK_VALIDATE_POS_NPI, GetRemotePropertyInt("EbillingValidatePOSNPI",1,0,"<None>",TRUE));
	// (a.walling 2008-05-20 09:31) - PLID 27812 - added option to verify NPI checksums
	CheckDlgButton(IDC_CHECK_VALIDATE_NPI_CHECKSUM, GetRemotePropertyInt("EbillingValidateNPIChecksum",1,0,"<None>",TRUE));
	// (j.jones 2008-05-21 13:47) - PLID 29280 - added provider and ref. phy. NPI options
	CheckDlgButton(IDC_CHECK_VALIDATE_PROVIDER_NPI, GetRemotePropertyInt("EbillingValidateProviderNPI",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_REF_PHY_NPI, GetRemotePropertyInt("EbillingValidateRefPhyNPI",1,0,"<None>",TRUE));
	// (j.jones 2009-08-10 10:55) - PLID 32886 - added gender validations
	CheckDlgButton(IDC_CHECK_VALIDATE_PRIMARY_GENDER, GetRemotePropertyInt("EbillingValidatePrimaryGender",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_SECONDARY_GENDER, GetRemotePropertyInt("EbillingValidateSecondaryGender",1,0,"<None>",TRUE));
	CheckDlgButton(IDC_CHECK_VALIDATE_PATIENT_GENDER, GetRemotePropertyInt("EbillingValidatePatientGender",1,0,"<None>",TRUE));
	// (j.jones 2010-01-07 09:25) - PLID 36786 - added ability to disable the Self name checking
	CheckDlgButton(IDC_CHECK_VALIDATE_PATIENT_NAME_SELF, GetRemotePropertyInt("EbillingValidatePatientNameWhenSelf",1,0,"<None>",TRUE));
	// (j.jones 2010-07-23 15:58) - PLID 39797 - added assignment of benefits validation
	CheckDlgButton(IDC_CHECK_VALIDATE_ASSIGN_BENEFITS, GetRemotePropertyInt("EbillingValidateAssignmentOfBenefits",1,0,"<None>",TRUE));
	// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
	CheckDlgButton(IDC_CHECK_VALIDATE_DUPLICATE_ECLAIM, GetRemotePropertyInt("EbillingValidateExportedTwiceInSameDay",1,0,"<None>",TRUE));
	// (j.jones 2012-05-01 09:59) - PLID 48530 - added prior auth. number validation
	CheckDlgButton(IDC_CHECK_VALIDATE_PRIOR_AUTH_NUMBER, GetRemotePropertyInt("EbillingValidatePriorAuthNumber",1,0,"<None>",TRUE));
	// (j.jones 2012-07-24 17:54) - PLID 51764 - added office visit global period validation
	CheckDlgButton(IDC_CHECK_VALIDATE_OFFICE_VISITS_IN_GLOBAL_PERIODS, GetRemotePropertyInt("EbillingValidateOfficeVisitGlobalPeriods",1,0,"<None>",TRUE));
	// (a.wilson 2013-03-26 16:33) - PLID 51773 - added accident type for current accident date validation
	CheckDlgButton(IDC_CHECK_VALIDATE_ACCIDENT_TYPE_FOR_CURRENT_ACCIDENT_DATE, GetRemotePropertyInt("EbillingValidateAccidentTypeForCurrentAccidentDate", 1, 0, "<None>", true));
	// (d.singleton 2014-03-06 15:29) - PLID 61235 - if ins company requires icd9 codes and none are present on bill need to have validation error in ebilling
	CheckDlgButton(IDC_CHECK_VALIDATE_REQUIRES_ICD9, GetRemotePropertyInt("EbillingValidateRequiresICD9", 1, 0, "<None>", true));
	// (d.singleton 2014-03-06 15:34) - PLID 61236 - if ins company requires icd10 codes and none are present on bill need to have validation error in ebilling
	CheckDlgButton(IDC_CHECK_VALIDATE_REQUIRES_ICD10, GetRemotePropertyInt("EbillingValidateRequiresICD10", 1, 0, "<None>", true));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEbillingValidationConfigDlg::OnOK() 
{
	SetRemotePropertyInt("EbillingValidatePOSCode", IsDlgButtonChecked(IDC_CHECK_VALIDATE_POS_CODE) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidatePrimaryPayerID", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PRIMARY_PAYER_ID) ? 1 : 0, 0, "<None>");
	// (j.jones 2009-08-04 12:34) - PLID 14573 - removed THIN ID
	//SetRemotePropertyInt("EbillingValidatePrimaryTHINNum", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PRIMARY_THIN_NUM) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidatePrimaryNSFCode", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PRIMARY_NSF_CODE) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidatePrimaryName", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PRIMARY_NAME) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidatePrimaryAddress", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PRIMARY_ADDRESS) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidatePrimaryBirthdate", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PRIMARY_BIRTHDATE) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidatePrimaryInsuranceID", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PRIMARY_INSURANCE_ID) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidatePrimaryPolicyGroupNum", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PRIMARY_POLICY_NUM) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateProvider", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PROVIDER) ? 1 : 0, 0, "<None>");
	// (j.jones 2011-03-15 10:52) - PLID 42788 - renamed this checkbox
	SetRemotePropertyInt("EbillingValidateProviderOtherIDs", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PROVIDER_OTHER_IDS) ? 1 : 0, 0, "<None>");
	// (j.jones 2011-03-15 11:11) - PLID 42788 - added Box 24J 
	SetRemotePropertyInt("EbillingValidateProviderBox24JID", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PROVIDER_24J) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateProviderTaxonomyCode", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PROVIDER_TAXONOMY_CODE) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateRefPhyID", IsDlgButtonChecked(IDC_CHECK_VALIDATE_REF_PHY_ID) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateHospFrom", IsDlgButtonChecked(IDC_CHECK_VALIDATE_HOSP_FROM) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateHospTo", IsDlgButtonChecked(IDC_CHECK_VALIDATE_HOSP_TO) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateDiagCode1", IsDlgButtonChecked(IDC_CHECK_VALIDATE_DIAG_CODE_1) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateDiagPointers", IsDlgButtonChecked(IDC_CHECK_VALIDATE_DIAG_POINTERS) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateInvalidModifiers", IsDlgButtonChecked(IDC_CHECK_VALIDATE_INVALID_MODIFIERS) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateSecondaryPayerID", IsDlgButtonChecked(IDC_CHECK_VALIDATE_SECONDARY_PAYER_ID) ? 1 : 0, 0, "<None>");
	// (j.jones 2009-08-04 12:34) - PLID 14573 - removed THIN ID
	//SetRemotePropertyInt("EbillingValidateSecondaryTHINNum", IsDlgButtonChecked(IDC_CHECK_VALIDATE_SECONDARY_THIN_NUM) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateSecondaryNSFCode", IsDlgButtonChecked(IDC_CHECK_VALIDATE_SECONDARY_NSF_CODE) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateSecondaryPhoneNumber", IsDlgButtonChecked(IDC_CHECK_VALIDATE_SECONDARY_PHONE_NUM) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateSecondaryContactName", IsDlgButtonChecked(IDC_CHECK_VALIDATE_SECONDARY_CONTACT_NAME) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateSecondaryName", IsDlgButtonChecked(IDC_CHECK_VALIDATE_SECONDARY_NAME) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateSecondaryAddress", IsDlgButtonChecked(IDC_CHECK_VALIDATE_SECONDARY_ADDRESS) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateSecondaryBirthdate", IsDlgButtonChecked(IDC_CHECK_VALIDATE_SECONDARY_BIRTHDATE) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateSecondaryInsuranceID", IsDlgButtonChecked(IDC_CHECK_VALIDATE_SECONDARY_INSURANCE_ID) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateSecondaryPolicyGroupNum", IsDlgButtonChecked(IDC_CHECK_VALIDATE_SECONDARY_POLICY_NUM) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidatePatientAddress", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PATIENT_ADDRESS) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidatePatientBirthdate", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PATIENT_BIRTHDATE) ? 1 : 0, 0, "<None>");
	// (j.jones 2007-03-27 11:31) - PLID 25364 - added option for Place Of Service NPI
	SetRemotePropertyInt("EbillingValidatePOSNPI", IsDlgButtonChecked(IDC_CHECK_VALIDATE_POS_NPI) ? 1 : 0, 0, "<None>");
	// (a.walling 2008-05-20 09:31) - PLID 27812 - added option to verify NPI checksums
	SetRemotePropertyInt("EbillingValidateNPIChecksum", IsDlgButtonChecked(IDC_CHECK_VALIDATE_NPI_CHECKSUM) ? 1 : 0, 0, "<None>");
	// (j.jones 2008-05-21 13:47) - PLID 29280 - added provider and ref. phy. NPI options
	SetRemotePropertyInt("EbillingValidateProviderNPI", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PROVIDER_NPI) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateRefPhyNPI", IsDlgButtonChecked(IDC_CHECK_VALIDATE_REF_PHY_NPI) ? 1 : 0, 0, "<None>");
	// (j.jones 2009-08-10 10:55) - PLID 32886 - added gender validations
	SetRemotePropertyInt("EbillingValidatePrimaryGender", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PRIMARY_GENDER) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidateSecondaryGender", IsDlgButtonChecked(IDC_CHECK_VALIDATE_SECONDARY_GENDER) ? 1 : 0, 0, "<None>");
	SetRemotePropertyInt("EbillingValidatePatientGender", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PATIENT_GENDER) ? 1 : 0, 0, "<None>");
	// (j.jones 2010-01-07 09:25) - PLID 36786 - added ability to disable the Self name checking
	SetRemotePropertyInt("EbillingValidatePatientNameWhenSelf", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PATIENT_NAME_SELF) ? 1 : 0, 0, "<None>");
	// (j.jones 2010-07-23 15:58) - PLID 39797 - added assignment of benefits validation
	SetRemotePropertyInt("EbillingValidateAssignmentOfBenefits", IsDlgButtonChecked(IDC_CHECK_VALIDATE_ASSIGN_BENEFITS) ? 1 : 0, 0, "<None>");
	// (j.jones 2011-09-23 16:07) - PLID 39377 - added validation for duplicate same-day ebilling claims
	SetRemotePropertyInt("EbillingValidateExportedTwiceInSameDay", IsDlgButtonChecked(IDC_CHECK_VALIDATE_DUPLICATE_ECLAIM) ? 1 : 0, 0, "<None>");
	// (j.jones 2012-05-01 09:59) - PLID 48530 - added prior auth. number validation
	SetRemotePropertyInt("EbillingValidatePriorAuthNumber", IsDlgButtonChecked(IDC_CHECK_VALIDATE_PRIOR_AUTH_NUMBER) ? 1 : 0, 0, "<None>");
	// (j.jones 2012-07-24 17:54) - PLID 51764 - added office visit global period validation
	SetRemotePropertyInt("EbillingValidateOfficeVisitGlobalPeriods", IsDlgButtonChecked(IDC_CHECK_VALIDATE_OFFICE_VISITS_IN_GLOBAL_PERIODS) ? 1 : 0, 0, "<None>");
	// (a.wilson 2013-03-26 16:33) - PLID 51773 - added accident type for current accident date validation
	SetRemotePropertyInt("EbillingValidateAccidentTypeForCurrentAccidentDate", IsDlgButtonChecked(IDC_CHECK_VALIDATE_ACCIDENT_TYPE_FOR_CURRENT_ACCIDENT_DATE) ? 1 : 0, 0, "<None>");
	// (d.singleton 2014-03-06 15:35) - PLID 61235 - if ins company requires icd9 codes and none are present on bill need to have validation error in ebilling
	SetRemotePropertyInt("EbillingValidateRequiresICD9", IsDlgButtonChecked(IDC_CHECK_VALIDATE_REQUIRES_ICD9) ? 1 : 0, 0, "<None>");
	// (d.singleton 2014-03-06 15:37) - PLID 61236 - if ins company requires icd10 codes and none are present on bill need to have validation error in ebilling
	SetRemotePropertyInt("EbillingValidateRequiresICD10", IsDlgButtonChecked(IDC_CHECK_VALIDATE_REQUIRES_ICD10) ? 1 : 0, 0, "<None>");

	CDialog::OnOK();
}

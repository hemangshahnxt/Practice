// ContactsGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ContactsGeneral.h"
#include "MainFrm.h"
#include "UserPermissionsDlg.h"
#include "UserPropsDlg.h"
#include "Client.h"
#include "GlobalDataUtils.h"
#include "contactsrc.h"
#include "AuditTrail.h"
#include "UserGroupSecurityDlg.h"
#include "GroupSecurityDlg.h"
#include "CopyPermissionsDlg.h"
#include "EditPrefixesDlg.h"
#include "SelectUserDlg.h"
#include "NxSecurity.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "CommissionSetupDlg.h"
#include "RefPhysProcsDlg.h"
#include "GlobalDrawingUtils.h"
#include "PPCLink.h"
#include "ReferredPatients.h"
#include "DontShowDlg.h"
#include "ContactBiographyDlg.h"
#include "ContactSelectImageDlg.h"
#include "AttendanceUserSetupDlg.h"
#include "AttendanceUtils.h"
#include "TodoUtils.h"
#include "OHIPUtils.h"
#include "AlbertaHLINKUtils.h"
#include "NexERxRegisterPrescriberDlg.h"
#include "MultiSelectDlg.h"
#include "ConfigureProviderTypesDlg.h"
#include "HL7Utils.h" // (v.maida 2014-12-23 12:19) - PLID 64472 - Added option to automatically export updated referring physicians.

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CContactsGeneral dialog

// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker
CContactsGeneral::CContactsGeneral(CWnd* pParent)
	: CNxDialog(CContactsGeneral::IDD, pParent),
	m_labelChecker(NetUtils::CustomLabels),
	m_locationsChecker(NetUtils::LocationsT),
	m_providerChecker(NetUtils::Providers)
	//m_rs("ContactGeneral", &g_dbPractice)//ContactGeneral
{
	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Contacts_Module/enter_a_new_contact.htm";

	//{{AFX_DATA_INIT(CContactsGeneral)
	m_strContactType = _T("");
	m_PendingClaimProvID = -1;
	//}}AFX_DATA_INIT
}

// (a.walling 2007-11-21 15:06) - PLID 28157
// (a.walling 2008-10-10 12:53) - PLID 29688 - Changed to say IDC_PREFIX_LIST rather than IDC_PREFIX_COMBO
BEGIN_EVENTSINK_MAP(CContactsGeneral, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CContactsGeneral)
	ON_EVENT(CContactsGeneral, IDC_PREFIX_LIST, 2 /* SelectionChange */, OnChangePrefix, VTS_I4)
	ON_EVENT(CContactsGeneral, IDC_COMBO_PALMSYNC, 2 /* SelectionChange */, OnChangePalm, VTS_I4)
	ON_EVENT(CContactsGeneral, IDC_BDATE_BOX, 1 /* KillFocus */, OnKillFocusBdateBox, VTS_NONE)
	ON_EVENT(CContactsGeneral, IDC_PREFIX_LIST, 16 /* SelChosen */, OnSelChosenPrefixList, VTS_I4)
	ON_EVENT(CContactsGeneral, IDC_CONTACTLOCATION_COMBO, 16 /* SelChosen */, OnSelChosenContactLocation, VTS_I4)
	ON_EVENT(CContactsGeneral, IDC_DATE_OF_HIRE, 1 /* KillFocus */, OnKillFocusDateOfHire, VTS_NONE)
	ON_EVENT(CContactsGeneral, IDC_DATE_OF_TERM, 1 /* KillFocus */, OnKillFocusDateOfTerm, VTS_NONE)
	ON_EVENT(CContactsGeneral, IDC_CLAIM_PROVIDER, 16 /* SelChosen */, OnSelChosenClaimProvider, VTS_DISPATCH)
	ON_EVENT(CContactsGeneral, IDC_CLAIM_PROVIDER, 18 /* RequeryFinished */, OnRequeryFinishedClaimProvider, VTS_I2)
	ON_EVENT(CContactsGeneral, IDC_CLAIM_PROVIDER, 20 /* TrySetSelFinished */, OnTrySetSelFinishedClaimProvider, VTS_I4 VTS_I4)
	ON_EVENT(CContactsGeneral, IDC_PROVIDERLOCATION_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedProviderlocationCombo, VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CContactsGeneral, IDC_AMA_SPECIALTY_LIST, 16, CContactsGeneral::OnSelChosenAMASpecialtyList, VTS_DISPATCH)
	ON_EVENT(CContactsGeneral, IDC_CLAIM_PROVIDER, 1, CContactsGeneral::SelChangingClaimProvider, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

void CContactsGeneral::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CContactsGeneral)
	DDX_Control(pDX, IDC_NURSE_CHECK, m_btnNurse);
	DDX_Control(pDX, IDC_ANESTHESIOLOGIST, m_btnAnesthesiologist);
	DDX_Control(pDX, IDC_CHECK_CONTACT_INACTIVE, m_checkInactive);
	DDX_Control(pDX, IDC_MALE, m_male);
	DDX_Control(pDX, IDC_FEMALE, m_female);
	DDX_Control(pDX, IDC_PATCOORD_CHECK, m_btnPatCoord);
	DDX_Control(pDX, IDC_TECHNICIAN_CHECK, m_btnTechnician);
	DDX_Text(pDX, IDC_CONTACT_TYPE, m_strContactType);
	DDX_Control(pDX, IDC_EMAIL_BTN, m_btnEmail);
	DDX_Control(pDX, IDC_EDIT_PREFIXES, m_btnEditPrefixes);
	DDX_Control(pDX, IDC_PROPERTIES_BTN, m_btnProperties);
	DDX_Control(pDX, IDC_SHOW_COMMISSION, m_btnShowCommision);
	DDX_Control(pDX, IDC_SHOW_PROCS, m_btnShowProcs);
	DDX_Control(pDX, IDC_PERMISSIONS_BTN, m_btnPermissions);
	DDX_Control(pDX, IDC_REFERRED_PATS, m_btnReferredPats);
	DDX_Control(pDX, IDC_REFERRED_PROSPECTS, m_btnReferredProspects);
	DDX_Control(pDX, IDC_EDITSECURITYGROUPS_BTN, m_btnEditSecurityGroups);
	DDX_Control(pDX, IDC_EDIT_BIOGRAPHY, m_btnEditBiography);
	DDX_Control(pDX, IDC_SELECT_IMAGE, m_btnSelectImage);
	DDX_Control(pDX, IDC_ATTENDANCE_SETUP, m_btnAttendanceSetup);
	DDX_Control(pDX, IDC_EMPLOYER_BOX, m_nxeditEmployerBox);
	DDX_Control(pDX, IDC_ACCOUNT_BOX, m_nxeditAccountBox);
	DDX_Control(pDX, IDC_TITLE_BOX, m_nxeditTitleBox);
	DDX_Control(pDX, IDC_FIRST_NAME_BOX, m_nxeditFirstNameBox);
	DDX_Control(pDX, IDC_MIDDLE_NAME_BOX, m_nxeditMiddleNameBox);
	DDX_Control(pDX, IDC_LAST_NAME_BOX, m_nxeditLastNameBox);
	DDX_Control(pDX, IDC_ADDRESS1_BOX, m_nxeditAddress1Box);
	DDX_Control(pDX, IDC_ADDRESS2_BOX, m_nxeditAddress2Box);
	DDX_Control(pDX, IDC_ZIP_BOX, m_nxeditZipBox);
	DDX_Control(pDX, IDC_CITY_BOX, m_nxeditCityBox);
	DDX_Control(pDX, IDC_STATE_BOX, m_nxeditStateBox);
	DDX_Control(pDX, IDC_MARRIAGE_OTHER_BOX, m_nxeditMarriageOtherBox);
	DDX_Control(pDX, IDC_SSN_BOX, m_nxeditSsnBox);
	DDX_Control(pDX, IDC_HOME_PHONE_BOX, m_nxeditHomePhoneBox);
	DDX_Control(pDX, IDC_WORK_PHONE_BOX, m_nxeditWorkPhoneBox);
	DDX_Control(pDX, IDC_EXT_PHONE_BOX, m_nxeditExtPhoneBox);
	DDX_Control(pDX, IDC_EMAIL_BOX, m_nxeditEmailBox);
	DDX_Control(pDX, IDC_CELL_PHONE_BOX, m_nxeditCellPhoneBox);
	DDX_Control(pDX, IDC_PAGER_PHONE_BOX, m_nxeditPagerPhoneBox);
	DDX_Control(pDX, IDC_OTHER_PHONE_BOX, m_nxeditOtherPhoneBox);
	DDX_Control(pDX, IDC_FAX_BOX, m_nxeditFaxBox);
	DDX_Control(pDX, IDC_EMERGENCY_FIRST_NAME, m_nxeditEmergencyFirstName);
	DDX_Control(pDX, IDC_EMERGENCY_LAST_NAME, m_nxeditEmergencyLastName);
	DDX_Control(pDX, IDC_RELATION_BOX, m_nxeditRelationBox);
	DDX_Control(pDX, IDC_OTHERWORK_BOX, m_nxeditOtherworkBox);
	DDX_Control(pDX, IDC_OTHERHOME_BOX, m_nxeditOtherhomeBox);
	DDX_Control(pDX, IDC_NPI_BOX, m_nxeditNpiBox);
	DDX_Control(pDX, IDC_ACCOUNT_NAME, m_nxeditAccountName);
	DDX_Control(pDX, IDC_NATIONALNUM_BOX, m_nxeditNationalnumBox);
	DDX_Control(pDX, IDC_FEDID_BOX, m_nxeditFedidBox);
	DDX_Control(pDX, IDC_METHOD_BOX, m_nxeditMethodBox);
	DDX_Control(pDX, IDC_WORKCOMP_BOX, m_nxeditWorkcompBox);
	DDX_Control(pDX, IDC_MEDICAID_BOX, m_nxeditMedicaidBox);
	DDX_Control(pDX, IDC_LICENSE_BOX, m_nxeditLicenseBox);
	DDX_Control(pDX, IDC_BCBS_BOX, m_nxeditBcbsBox);
	DDX_Control(pDX, IDC_UPIN_BOX, m_nxeditUpinBox);
	DDX_Control(pDX, IDC_MEDICARE_BOX, m_nxeditMedicareBox);
	DDX_Control(pDX, IDC_OTHERID_BOX, m_nxeditOtheridBox);
	DDX_Control(pDX, IDC_DEA_BOX, m_nxeditDeaBox);
	DDX_Control(pDX, IDC_REFPHYSID_BOX, m_nxeditRefphysidBox);
	DDX_Control(pDX, IDC_TAXONOMY_CODE, m_nxeditTaxonomyCode);
	DDX_Control(pDX, IDC_CON_CUSTOM1_BOX, m_nxeditConCustom1Box);
	DDX_Control(pDX, IDC_CON_CUSTOM2_BOX, m_nxeditConCustom2Box);
	DDX_Control(pDX, IDC_CON_CUSTOM3_BOX, m_nxeditConCustom3Box);
	DDX_Control(pDX, IDC_CON_CUSTOM4_BOX, m_nxeditConCustom4Box);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_NUM_PATIENTS_REF_BOX, m_nxeditNumPatientsRefBox);
	DDX_Control(pDX, IDC_NUM_PROSPECTS_REF_BOX, m_nxeditNumProspectsRefBox);
	DDX_Control(pDX, IDC_DEFAULT_COST_EDIT, m_nxeditDefaultCostEdit);
	DDX_Control(pDX, IDC_CONTACT_TYPE, m_nxstaticContactType);
	DDX_Control(pDX, IDC_LABEL_NPI, m_nxstaticLabelNpi);
	DDX_Control(pDX, IDC_LABEL26, m_nxstaticLabel26);
	DDX_Control(pDX, IDC_DATE_OF_HIRE_LABEL, m_nxstaticDateOfHireLabel);
	DDX_Control(pDX, IDC_DATE_OF_TERM_LABEL, m_nxstaticDateOfTermLabel);
	DDX_Control(pDX, IDC_NUM_PATIENTS_REF_LABEL, m_nxstaticNumPatientsRefLabel);
	DDX_Control(pDX, IDC_NUM_PROSPECTS_REF_LABEL, m_nxstaticNumProspectsRefLabel);
	DDX_Control(pDX, IDC_DEFAULT_COST_LABEL, m_nxstaticDefaultCostLabel);
	DDX_Control(pDX, IDC_CUSTOM1_LABEL, m_nxstaticCustom1Label);
	DDX_Control(pDX, IDC_CUSTOM2_LABEL, m_nxstaticCustom2Label);
	DDX_Control(pDX, IDC_CUSTOM3_LABEL, m_nxstaticCustom3Label);
	DDX_Control(pDX, IDC_CUSTOM4_LABEL, m_nxstaticCustom4Label);
	DDX_Control(pDX, IDC_ID_LABEL12, m_nxstaticIdLabel12);
	DDX_Control(pDX, IDC_LABEL22, m_nxstaticLabel22);
	DDX_Control(pDX, IDC_LABEL5, m_nxstaticLabel5);
	DDX_Control(pDX, IDC_LABEL34, m_nxstaticLabel34);
	DDX_Control(pDX, IDC_LABEL35, m_nxstaticLabel35);
	DDX_Control(pDX, IDC_FAX_LABEL, m_nxstaticFaxLabel);
	DDX_Control(pDX, IDC_LABEL6, m_nxstaticLabel6);
	DDX_Control(pDX, IDC_LABEL10, m_nxstaticLabel10);
	DDX_Control(pDX, IDC_LABEL8, m_nxstaticLabel8);
	DDX_Control(pDX, IDC_LABEL9, m_nxstaticLabel9);
	DDX_Control(pDX, IDC_LABEL23, m_nxstaticLabel23);
	DDX_Control(pDX, IDC_CLAIM_PROV_LABEL, m_nxstaticClaimProvLabel);
	DDX_Control(pDX, IDC_LABEL30, m_nxstaticLabel30);
	DDX_Control(pDX, IDC_LABEL29, m_nxstaticLabel29);
	DDX_Control(pDX, IDC_LABEL27, m_nxstaticLabel27);
	DDX_Control(pDX, IDC_LABEL32, m_nxstaticLabel32);
	DDX_Control(pDX, IDC_LABEL33, m_nxstaticLabel33);
	DDX_Control(pDX, IDC_LABEL28, m_nxstaticLabel28);
	DDX_Control(pDX, IDC_LABEL_TAXONOMY_CODE, m_nxstaticLabelTaxonomyCode);
	DDX_Control(pDX, IDC_LABEL31, m_nxstaticLabel31);
	DDX_Control(pDX, IDC_LABEL24, m_nxstaticLabel24);
	DDX_Control(pDX, IDC_METHOD_LABEL, m_nxstaticMethodLabel);
	DDX_Control(pDX, IDC_ACCOUNT_NAME_LABEL, m_nxstaticAccountNameLabel);
	DDX_Control(pDX, IDC_LABEL19, m_nxstaticLabel19);
	DDX_Control(pDX, IDC_LABEL_DEFLOCATION, m_nxstaticLabelDeflocation);
	DDX_Control(pDX, IDC_PROV_SPI, m_editSPI);
	DDX_Control(pDX, IDC_OHIP_SPECIALTY, m_nxeditOHIPSpecialty);
	DDX_Control(pDX, IDC_LABEL_OHIP_SPECIALTY, m_nxstaticLabelOHIPSpecialty);
	DDX_Control(pDX, IDC_CHECK_SEND_COMPANY_ON_CLAIM, m_checkSendCompanyOnClaim);
	DDX_Control(pDX, IDC_OPTICIAN_CHECK, m_btnOptician);
	DDX_Control(pDX, IDC_GROUP_NPI_BOX, m_nxeditGroupNPIBox);
	DDX_Control(pDX, IDC_LABEL_GROUP_NPI, m_nxstaticLabelGroupNPI);
	DDX_Control(pDX, IDC_BTN_REGISTER_PRESCRIBER, m_btnRegisterPrescriber);
	DDX_Control(pDX, IDC_DIRECT_ADDRESS_EDIT, m_nxeditDirectAddress); 
	DDX_Control(pDX, IDC_CONTACTS_PROVIDER_TYPE_INFO, m_icoProviderTypeInfo);
	// (s.tullis 2015-10-29 17:13) - PLID 67483
	DDX_Control(pDX, IDC_CAPITATION_DISTRIBUTION_EDIT, m_editCaptitationDistribution);
	DDX_Control(pDX, IDC_CAPITATION_DISTRIBUTION_TEXT, m_nxstaticCapitationDistribution);
	DDX_Control(pDX, IDC_CAPITATION_TEXT, m_nxstaticCapitationDistributionPercentsign);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CContactsGeneral, CNxDialog)
	//{{AFX_MSG_MAP(CContactsGeneral)
	ON_BN_CLICKED(IDC_PATCOORD_CHECK, OnClickPatcoordCheck)
	ON_BN_CLICKED(IDC_TECHNICIAN_CHECK, OnClickTechnicianCheck)
	ON_BN_CLICKED(IDC_PERMISSIONS_BTN, OnPermissionsBtn)
	ON_BN_CLICKED(IDC_PROPERTIES_BTN, OnPropertiesBtn)
	ON_BN_CLICKED(IDC_EMAIL_BTN, OnClickEmail)
	ON_BN_CLICKED(IDC_FEMALE, OnFemale)
	ON_BN_CLICKED(IDC_MALE, OnMale)
	ON_BN_CLICKED(IDC_EDITSECURITYGROUPS_BTN, OnEditsecuritygroupsBtn)
	ON_BN_CLICKED(IDC_NURSE_CHECK, OnNurseCheck)
	ON_BN_CLICKED(IDC_ANESTHESIOLOGIST, OnAnesthesiologist)
	ON_BN_CLICKED(IDC_EDIT_PREFIXES, OnEditPrefixes)
	ON_BN_CLICKED(IDC_CHECK_CONTACT_INACTIVE, OnCheckContactInactive)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_SHOW_COMMISSION, OnShowCommission)
	ON_BN_CLICKED(IDC_SHOW_PROCS, OnShowProcs)
	ON_BN_CLICKED(IDC_REFERRED_PATS, OnReferredPats)
	ON_BN_CLICKED(IDC_REFERRED_PROSPECTS, OnReferredProspects)
	ON_BN_CLICKED(IDC_EDIT_BIOGRAPHY, OnEditBiography)
	ON_BN_CLICKED(IDC_SELECT_IMAGE, OnSelectImage)
	ON_BN_CLICKED(IDC_ATTENDANCE_SETUP, OnAttendanceSetup)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_AFFILIATE_PHYS, &CContactsGeneral::OnBnClickedAffiliatePhys)
	ON_BN_CLICKED(IDC_CHECK_SEND_COMPANY_ON_CLAIM, OnSendCompanyOnClaim)
	ON_BN_CLICKED(IDC_OPTICIAN_CHECK, &CContactsGeneral::OnBnClickedOpticianCheck)
	ON_BN_CLICKED(IDC_BTN_REGISTER_PRESCRIBER, &CContactsGeneral::OnBnClickedBtnRegisterPrescriber)
	ON_BN_CLICKED(IDC_DIRECT_ADDRESS_USERS_BUTTON, &CContactsGeneral::OnBnClickedDirectAddressUsersButton)
	ON_BN_CLICKED(IDC_CONTACT_REFERRING_PROVIDER_CHECK, &CContactsGeneral::OnBnClickedContactReferringProviderCheck)
	ON_BN_CLICKED(IDC_CONTACT_ORDERING_PROVIDER_CHECK, &CContactsGeneral::OnBnClickedContactOrderingProviderCheck)
	ON_BN_CLICKED(IDC_CONTACT_SUPERVISING_PROVIDER_CHECK, &CContactsGeneral::OnBnClickedContactSupervisingProviderCheck)
	ON_BN_CLICKED(IDC_CONTACT_CONFIGURE_PROVIDER_TYPES_BUTTON, &CContactsGeneral::OnBnClickedContactConfigureProviderTypesButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContactsGeneral message handlers

BOOL CContactsGeneral::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try {

		// (d.singleton 2012-06-18 13:21) - PLID 51029 set limit on address 1 and 2
		m_nxeditAddress1Box.SetLimitText(150);
		m_nxeditAddress2Box.SetLimitText(150);
		m_nxeditEmployerBox.SetLimitText(150);
		// (j.jones 2013-06-10 13:26) - PLID 57089 - Added Group NPI, limited to 50 characters,
		// though technically a real NPI would not be that large. I noticed the NPI isn't limited
		// either, so I handled that as well.
		m_nxeditGroupNPIBox.SetLimitText(50);
		m_nxeditNpiBox.SetLimitText(50);
		
		m_btnEditBiography.AutoSet(NXB_MODIFY);
		m_btnSelectImage.AutoSet(NXB_MODIFY);
		m_btnProperties.AutoSet(NXB_MODIFY);
		m_btnShowCommision.AutoSet(NXB_MODIFY);
		m_btnShowProcs.AutoSet(NXB_MODIFY);
		m_btnPermissions.AutoSet(NXB_MODIFY);
		m_btnAttendanceSetup.AutoSet(NXB_MODIFY);

		// (j.jones 2009-06-26 09:13) - PLID 34292 - added OHIP Specialty
		m_nxeditOHIPSpecialty.SetLimitText(10);
		// (s.tullis 2015-10-29 17:13) - PLID 67483 - Set max % to 3 digits
		m_editCaptitationDistribution.SetLimitText(3);
		//set up the prefix list
		m_PrefixCombo = BindNxDataListCtrl(IDC_PREFIX_LIST);
		//set up the location list
		m_LocationCombo = BindNxDataListCtrl(IDC_CONTACTLOCATION_COMBO);
		// (j.jones 2006-12-01 09:42) - PLID 22110 - added claim provider list
		// (a.walling 2007-11-09 17:09) - PLID 28059 - Bad bind; use BindNxDataList2Ctrl
		m_ClaimProviderCombo = BindNxDataList2Ctrl(IDC_CLAIM_PROVIDER, false);
		//DRT 11/20/2008 - PLID 32082
		m_pAMASpecialtyList = BindNxDataList2Ctrl(IDC_AMA_SPECIALTY_LIST);
		NXDATALIST2Lib::IRowSettingsPtr pRowSpecialty = m_pAMASpecialtyList->GetNewRow();
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pRowSpecialty->PutValue(0, varNull);
		pRowSpecialty->PutValue(1, _bstr_t(""));	//code of nothing
		pRowSpecialty->PutValue(2, _bstr_t("<None Selected>"));
		m_pAMASpecialtyList->AddRowBefore(pRowSpecialty, m_pAMASpecialtyList->GetFirstRow());

		//add the blank row to the prefix list
		IRowSettingsPtr pRow;
		pRow = m_PrefixCombo->GetRow(-1);
		pRow->PutValue(0, (long)0);
		pRow->PutValue(1, _variant_t(""));
		pRow->PutValue(2, (long)0);
		m_PrefixCombo->InsertRow(pRow, 0);

		m_PalmCombo = BindNxDataListCtrl(IDC_COMBO_PALMSYNC,false);

		IColumnSettingsPtr(m_PalmCombo->GetColumn(m_PalmCombo->InsertColumn(0, _T("PalmSetting"), _T("PalmSetting"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
		m_PalmCombo->IsComboBox = TRUE;

		pRow = m_PalmCombo->GetRow(-1);
		_variant_t var = _bstr_t("Never sync");
		pRow->PutValue(0,var);
		m_PalmCombo->AddRow(pRow);
		pRow = m_PalmCombo->GetRow(-1);
		var = _bstr_t("Always sync");
		pRow->PutValue(0,var);
		m_PalmCombo->AddRow(pRow);
		pRow = m_PalmCombo->GetRow(-1);
		var = _bstr_t("Follow adv. settings");
		pRow->PutValue(0,var);
		m_PalmCombo->AddRow(pRow);	

		m_nxtBirthDate = GetDlgItemUnknown(IDC_BDATE_BOX);
		m_dtBirthDate.SetDateTime(1899,12,30,0,0,0);
		m_dtBirthDate.SetStatus(COleDateTime::invalid);

		m_nxtDateOfHire = GetDlgItemUnknown(IDC_DATE_OF_HIRE);
		m_dtDateOfHire.SetDateTime(1899,12,30,0,0,0);
		m_dtDateOfHire.SetStatus(COleDateTime::invalid);

		// (a.walling 2007-11-21 15:06) - PLID 28157
		m_nxtDateOfTerm = GetDlgItemUnknown(IDC_DATE_OF_TERM);
		m_dtDateOfTerm.SetDateTime(1899,12,30,0,0,0);
		m_dtDateOfTerm.SetStatus(COleDateTime::invalid);

		// (b.cardillo 2006-05-19 17:20) - PLID 20735 - Use the global functions so that we benefit 
		// from the cached custom field names.
		SetDlgItemText(IDC_CUSTOM1_LABEL, ConvertToControlText(GetCustomFieldName(6)));
		SetDlgItemText(IDC_CUSTOM2_LABEL, ConvertToControlText(GetCustomFieldName(7)));
		SetDlgItemText(IDC_CUSTOM3_LABEL, ConvertToControlText(GetCustomFieldName(8)));
		SetDlgItemText(IDC_CUSTOM4_LABEL, ConvertToControlText(GetCustomFieldName(9)));

		m_id = GetActiveContactID();

		//(e.lally 2010-08-03) PLID 33962 - Added bulk cache
		g_propManager.CachePropertiesInBulk("ContactsGeneral-Number", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( "
			"	'FormatPhoneNums' "
			"	, 'AutoCapitalizeMiddleInitials' "
			"	, 'GenderPrefixLink' "
			"	, 'DefaultMalePrefix' "
			"	, 'DefaultFemalePrefix' "
			"	, 'Gen1SaveEmails' "
			"	, 'DefaultProductOrderingUser' "
			"	, 'LookupZipStateByCity' "
			"	, 'NexWebTrackingLadderAssignToUser' "
			// (j.jones 2010-11-08 14:54) - PLID 39620 - added Alberta option
			"	, 'UseAlbertaHLINK' "
			"	, 'Alberta_PatientULICustomField' "
			// (s.tullis 2015-10-29 17:13) - PLID 67483 - Cache Prop
			"	, 'BatchPayments_EnableCapitation' "									
			")", _Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("ContactsGeneral-Text", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name IN ('PhoneFormatString'))", _Q(GetCurrentUserName()));

		m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true); 
		m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

		((CNxEdit*)GetDlgItem(IDC_EXT_PHONE_BOX))->SetLimitText(10);
		// (b.savon 2013-04-23 13:44) - PLID 56409 - Change this to 10
		((CNxEdit*)GetDlgItem(IDC_PROV_SPI))->SetLimitText(10);
		
		//TES 2/9/2004: The Note is now ntext, therefore it supports an unlimited length.  CEdits, however, do not support an 
		//unlimited length, so we will set it to the maximum possible.
		((CNxEdit*)GetDlgItem(IDC_NOTES))->SetLimitText(0x7FFFFFFE);

		// (b.spivey -- October 16th, 2013) - PLID 59022 - set the text limit to 100.
		m_nxeditDirectAddress.SetLimitText(100); 

		// (z.manning, 12/12/2007) - PLID 28218 - For now at least, the attendance stuff is Internal only.
		if(IsNexTechInternal()) {
			GetDlgItem(IDC_ATTENDANCE_SETUP)->ShowWindow(SW_SHOW);
		}

		//(a.wilson 2014-04-23) PLID 61825 - load icon with tooltip.
		m_icoProviderTypeInfo.LoadToolTipIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK), CString(""), false, false, false);

		CString str;
		//TODO: FILL IN FIELDS: Status, RecordID
		Load();
		m_changed = false;
		
		m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_CONTACT, 0)));

	} NxCatchAll ("Error in ContactsGeneral::OnInitDialog");

	return TRUE;
}

void CContactsGeneral::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// (j.gruber 2009-10-06 17:02) - PLID 35825 - reset here in case they changed the preference
	m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");

	if (m_bLookupByCity) {
		ChangeZOrder(IDC_ZIP_BOX, IDC_STATE_BOX);
	} else {
		ChangeZOrder(IDC_ZIP_BOX, IDC_ADDRESS2_BOX);
	}

	StoreDetails();
	m_id = GetActiveContactID();
	RecallDetails();
}

void CContactsGeneral::Load()
{
	CWaitCursor cur;

	int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();


	try {

		// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker,
		// these properties are cached, so just update the member variables from the cache
		m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true); 
		m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

		if(m_locationsChecker.Changed()){
			m_LocationCombo->Requery();
		}

		// (j.jones 2006-12-01 11:10) - PLID 22110 - always show the current provider,
		// even if inactive
		CString strExistingWhere = AsString(m_ClaimProviderCombo->GetWhereClause());
		CString strNewWhere;
		strNewWhere.Format("Archived = 0 OR ProvidersT.PersonID = %li", m_id);
		if(m_providerChecker.Changed() || strExistingWhere != strNewWhere) {
			m_ClaimProviderCombo->PutWhereClause(_bstr_t(strNewWhere));
			m_ClaimProviderCombo->Requery();
		}

	/*	m_rs = PersonT recordset
		m_rsProv = ProvidersT recordset
		m_rsEmp = UsersT recordset
		m_rsSup	= SupplierT recordset
		m_rsRef = RefPhyST recordset
		m_rsCon = ContactsT recordset
	*/

	// (a.walling 2011-01-19 16:16) - PLID 40965 - Parameterized
	_RecordsetPtr m_rs = CreateParamRecordset("SELECT PersonT.ID, PersonT.Archived, PersonT.First AS [First Name], PersonT.Middle AS [Middle Name], PersonT.Last AS [Last Name],  "
		"PersonT.Title, PersonT.Address1 AS [Address 1], PersonT.Address2 AS [Address 2], PersonT.City,  "
		"PersonT.State AS StateProv, PersonT.Zip AS PostalCode, PersonT.HomePhone AS [Home Phone],  "
		"PersonT.WorkPhone AS [Work Phone], PersonT.Extension, PersonT.Pager, PersonT.OtherPhone AS [Other Phone],   "
		"PersonT.Email AS [Email Address], PersonT.Gender, PersonT.PrefixID, PersonT.CellPhone, PersonT.Note AS [Memo], PersonT.BirthDate, PersonT.SocialSecurity, "
		"PersonT.Company, PersonT.CompanyID, PersonT.EmergFirst, PersonT.EmergLast, PersonT.EmergHPhone, PersonT.EmergWPhone, PersonT.EmergRelation, PersonT.Spouse, PersonT.Fax, PersonT.PalmFlag, "
		"PersonT.Location, LocationsT.Name AS LocationName, DAFT.DirectAddress "
		"FROM "
		"(PersonT LEFT OUTER JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID) LEFT OUTER JOIN ContactsT ON PersonT.ID = ContactsT.PersonID "
		"LEFT OUTER JOIN UsersT ON PersonT.ID = UsersT.PersonID "
		"LEFT OUTER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
		"LEFT OUTER JOIN ReferringPhyST ON PersonT.ID = ReferringPhyST.PersonID "
		"LEFT OUTER JOIN SupplierT ON PersonT.ID = SupplierT.PersonID "
		"LEFT OUTER JOIN LocationsT ON PersonT.Location = LocationsT.ID "
		"LEFT JOIN DirectAddressFromT DAFT ON PersonT.ID = DAFT.PersonID " 
		"WHERE (ContactsT.PersonID Is Not NULL OR  "
		"UsersT.PersonID IS NOT NULL OR  "
		"ProvidersT.PersonID IS NOT NULL OR  "
		"ReferringPhyST.PersonID IS NOT NULL OR "
		"SupplierT.PersonID IS NOT NULL) AND "
		"PersonT.ID = {INT}", m_id);

	_variant_t var;
	
	//load data from PersonT
	if (!m_rs->eof) {
		EnableWindow(TRUE);
		SetDlgItemText (IDC_TITLE_BOX,			CString(m_rs->Fields->Item["Title"]->Value.bstrVal));
		SetDlgItemText (IDC_FIRST_NAME_BOX,		CString(m_rs->Fields->Item["First Name"]->Value.bstrVal));
		SetDlgItemText (IDC_MIDDLE_NAME_BOX,		CString(m_rs->Fields->Item["Middle Name"]->Value.bstrVal));
		SetDlgItemText (IDC_LAST_NAME_BOX,		CString(m_rs->Fields->Item["Last Name"]->Value.bstrVal));
		SetDlgItemText (IDC_ADDRESS1_BOX,		CString(m_rs->Fields->Item["Address 1"]->Value.bstrVal));
		SetDlgItemText (IDC_ADDRESS2_BOX,		CString(m_rs->Fields->Item["Address 2"]->Value.bstrVal));
		SetDlgItemText (IDC_CITY_BOX,			CString(m_rs->Fields->Item["City"]->Value.bstrVal));
		SetDlgItemText (IDC_STATE_BOX,			CString(m_rs->Fields->Item["StateProv"]->Value.bstrVal));
		SetDlgItemText (IDC_ZIP_BOX,				CString(m_rs->Fields->Item["PostalCode"]->Value.bstrVal));

		BOOL bArchived = AdoFldBool(m_rs, "Archived",FALSE);

		m_checkInactive.SetCheck(bArchived);

		switch (m_rs->Fields->Item["PalmFlag"]->Value.iVal)
		{
		case 0: m_PalmCombo->SetSelByColumn(0,_variant_t("Never sync")); break;
		case 1: m_PalmCombo->SetSelByColumn(0,_variant_t("Always sync")); break;
		case 2: m_PalmCombo->SetSelByColumn(0,_variant_t("Follow adv. settings")); break;
		}

		// (j.jones 2006-05-02 10:03) - PLID 20389 - changed 4 recordsets into one loop

		//important to clear out existing data, because CustomFieldDataT won't have an entry if the field is blank
		SetDlgItemText(IDC_CON_CUSTOM1_BOX,CString(""));
		SetDlgItemText(IDC_CON_CUSTOM2_BOX,CString(""));
		SetDlgItemText(IDC_CON_CUSTOM3_BOX,CString(""));
		SetDlgItemText(IDC_CON_CUSTOM4_BOX,CString(""));

		_RecordsetPtr rsCustom = CreateRecordset("SELECT TextParam, FieldID FROM CustomFieldDataT WHERE PersonID = %li AND FieldID >=6 AND FieldID <=9",m_id);
		while(!rsCustom->eof) {

			long i = AdoFldLong(rsCustom, "FieldID", -1);

			switch (i) {
			case 6:
				SetDlgItemText (IDC_CON_CUSTOM1_BOX, AdoFldString(rsCustom, "TextParam", ""));//,				overwrite);
				break;
			case 7:
				SetDlgItemText (IDC_CON_CUSTOM2_BOX, AdoFldString(rsCustom, "TextParam", ""));//,				overwrite);
				break;
			case 8:
				SetDlgItemText (IDC_CON_CUSTOM3_BOX, AdoFldString(rsCustom, "TextParam", ""));//,				overwrite);
				break;
			case 9:
				SetDlgItemText (IDC_CON_CUSTOM4_BOX, AdoFldString(rsCustom, "TextParam", ""));//,				overwrite);
				break;
			}
			rsCustom->MoveNext();
		}

		SetDlgItemText (IDC_HOME_PHONE_BOX,		AdoFldString(m_rs, "Home Phone",""));
		SetDlgItemText (IDC_WORK_PHONE_BOX,		AdoFldString(m_rs, "Work Phone",""));
		SetDlgItemText (IDC_EXT_PHONE_BOX,		AdoFldString(m_rs, "Extension",""));
		SetDlgItemText (IDC_CELL_PHONE_BOX,		AdoFldString(m_rs, "CellPhone",""));
		SetDlgItemText (IDC_PAGER_PHONE_BOX,	AdoFldString(m_rs, "Pager",""));
		SetDlgItemText (IDC_OTHER_PHONE_BOX,	AdoFldString(m_rs, "Other Phone",""));
		SetDlgItemText (IDC_FAX_BOX,			AdoFldString(m_rs, "Fax",""));
		SetDlgItemText (IDC_EMAIL_BOX,			AdoFldString(m_rs, "Email Address",""));
		SetDlgItemText (IDC_NOTES,				AdoFldString(m_rs, "Memo",""));
		SetDlgItemText (IDC_SSN_BOX,			AdoFldString(m_rs, "SocialSecurity",""));
		_variant_t varDate = m_rs->Fields->Item["BirthDate"]->Value;
		if(varDate.vt == VT_DATE) {
			m_dtBirthDate = VarDateTime(varDate);
			m_nxtBirthDate->SetDateTime(m_dtBirthDate);
		}
		else {
			m_dtBirthDate.SetDateTime(1899,12,30,0,0,0);
			m_dtBirthDate.SetStatus(COleDateTime::invalid);
			m_nxtBirthDate->Clear();
		}

		//Enable or disable the "E-mail" button 
		UpdateEmailButton();

		var = m_rs->Fields->Item["PrefixID"]->Value;
		if (var.vt == VT_I4)
			m_PrefixCombo->SetSelByColumn(0,var);
		else m_PrefixCombo->SetSelByColumn(0,long(0));

		var = m_rs->Fields->Item["Gender"]->Value;
		if (var.vt != VT_NULL)
		{	if (VarByte(var) == 1) {
				m_male.SetCheck(true);
				m_female.SetCheck(false);
			}
			else if (VarByte(var) == 2) {
				m_female.SetCheck(true);
				m_male.SetCheck(false);
			}
			else
			{	m_male.SetCheck(false);
				m_female.SetCheck(false);
			}
		}
		else
		{	m_male.SetCheck(false);
			m_female.SetCheck(false);
		}

		SetDlgItemText (IDC_EMPLOYER_BOX,			CString(m_rs->Fields->Item["Company"]->Value.bstrVal));
		SetDlgItemText (IDC_ACCOUNT_BOX,	CString(m_rs->Fields->Item["CompanyID"]->Value.bstrVal));
		SetDlgItemText (IDC_MARRIAGE_OTHER_BOX,			CString(m_rs->Fields->Item["Spouse"]->Value.bstrVal));
		SetDlgItemText (IDC_EMERGENCY_FIRST_NAME,		CString(m_rs->Fields->Item["EmergFirst"]->Value.bstrVal));
		SetDlgItemText (IDC_EMERGENCY_LAST_NAME,			CString(m_rs->Fields->Item["EmergLast"]->Value.bstrVal));
		SetDlgItemText (IDC_RELATION_BOX,		CString(m_rs->Fields->Item["EmergRelation"]->Value.bstrVal));
		SetDlgItemText (IDC_OTHERHOME_BOX,		CString(m_rs->Fields->Item["EmergHPhone"]->Value.bstrVal));
		SetDlgItemText (IDC_OTHERWORK_BOX,			CString(m_rs->Fields->Item["EmergWPhone"]->Value.bstrVal));

		//TES 9/8/2008 - PLID 27727 - We now load the location for all contacts, using PersonT.Location.
		// (s.tullis 2016-05-25 15:41) - NX-100760 - Removed tryset here
		long nLocation = AdoFldLong(m_rs, "Location", 0);
		if(nLocation <= 0) {
			m_LocationCombo->CurSel = -1;
		}
		else {
			if(m_LocationCombo->TrySetSelByColumn(0,nLocation) == -1) {
				//TES 9/8/2008 - PLID 27727 - Must be an inactive location, we include the name in the query for just such
				// an occasion.
				m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(m_rs, "LocationName", "")));
			}
		}

		// (b.spivey -- October 16th, 2013) - PLID 59022 - fill the direct address field. 
		m_nxeditDirectAddress.SetWindowTextA(AdoFldString(m_rs->Fields, "DirectAddress", "")); 

		m_rs->Close();

		//Enable or disable the Direct Address users button. 
		UpdateDirectAddressButton();

		if(nStatus & 0x2)
		{

			// (j.jones 2006-12-01 10:30) - PLID 22110 - supported ClaimProviderID
			//DRT 11/20/2008 - PLID 32082 - Added SPI and AMA Specialty ID
			// (j.jones 2009-06-26 09:13) - PLID 34292 - added OHIP Specialty
			// (a.walling 2011-01-19 16:16) - PLID 40965 - Parameterized, removed dynamic cursor
			// (j.jones 2011-11-07 14:25) - PLID 46299 - added ProvidersT.UseCompanyOnClaims
			// (j.dinatale 2012-04-30 18:01) - PLID 49332
			// (b.savon 2013-06-12 12:26) - PLID 56867 - Added RegisteredPrescriber
			// (a.wilson 2014-04-21) PLID 61816 - provider types.
			// (s.tullis 2015-10-29 17:13) - PLID 67483 - Add Capitation Distrib
			_RecordsetPtr m_rsProv = CreateParamRecordset(
				R"(
SELECT PersonID, [Fed Employer ID] AS FedID, [DEA Number] AS DEA, [BCBS Number] AS BCBS, 
[Medicare Number] AS Medicare, [Medicaid Number] AS Medicaid, [Workers Comp Number] AS WorkComp, 
[Other ID Number] AS OtherID, UPIN, License, TaxonomyCode, DefaultCost, NPI, ClaimProviderID, 
ReferringProvider, OrderingProvider, SupervisingProvider, 
ProvidersT.SPI, ProvidersT.AMASpecialtyID, OHIPSpecialty, UseCompanyOnClaims, Optician, COALESCE(ProvidersT.NexERxProviderTypeID, -1) AS NexERxProviderTypeID, 
COALESCE(ProvidersT.CapitationDistribution, 0) as Capitation, 
COALESCE((SELECT TOP 1 SPI FROM NexERxPrescriberRegistrationT WHERE ProviderID = {INT}), '-1') AS RegisteredPrescriber 
FROM ProvidersT WHERE ProvidersT.PersonID = {INT}
				)", m_id, m_id);

			//load provider info
			if(!m_rsProv->eof)
			{
				SetDlgItemText (IDC_NPI_BOX,			CString(m_rsProv->Fields->Item["NPI"]->Value.bstrVal));
				SetDlgItemText (IDC_FEDID_BOX,			CString(m_rsProv->Fields->Item["FedID"]->Value.bstrVal));
				SetDlgItemText (IDC_UPIN_BOX,			CString(m_rsProv->Fields->Item["UPIN"]->Value.bstrVal));
				SetDlgItemText (IDC_MEDICARE_BOX,		CString(m_rsProv->Fields->Item["Medicare"]->Value.bstrVal));
				SetDlgItemText (IDC_BCBS_BOX,			CString(m_rsProv->Fields->Item["BCBS"]->Value.bstrVal));
				SetDlgItemText (IDC_MEDICAID_BOX,		CString(m_rsProv->Fields->Item["Medicaid"]->Value.bstrVal));
				SetDlgItemText (IDC_WORKCOMP_BOX,		CString(m_rsProv->Fields->Item["WorkComp"]->Value.bstrVal));
				SetDlgItemText (IDC_DEA_BOX,			CString(m_rsProv->Fields->Item["DEA"]->Value.bstrVal));
				SetDlgItemText (IDC_LICENSE_BOX,		CString(m_rsProv->Fields->Item["License"]->Value.bstrVal));
				SetDlgItemText (IDC_OTHERID_BOX,		CString(m_rsProv->Fields->Item["OtherID"]->Value.bstrVal));
				SetDlgItemText (IDC_TAXONOMY_CODE,		CString(m_rsProv->Fields->Item["TaxonomyCode"]->Value.bstrVal));
				SetDlgItemText (IDC_DEFAULT_COST_EDIT, FormatCurrencyForInterface(AdoFldCurrency(m_rsProv, "DefaultCost",COleCurrency(0,0)),TRUE,TRUE));
				SetDlgItemText (IDC_PROV_SPI,			AdoFldString(m_rsProv, "SPI"));
				// (j.jones 2009-06-26 09:13) - PLID 34292 - added OHIP Specialty
				SetDlgItemText (IDC_OHIP_SPECIALTY,		AdoFldString(m_rsProv, "OHIPSpecialty"));
				// (j.jones 2011-11-07 14:10) - PLID 46299 - added ProvidersT.UseCompanyOnClaims
				m_checkSendCompanyOnClaim.SetCheck(VarBool(m_rsProv->Fields->Item["UseCompanyOnClaims"]->Value, FALSE));
				// (s.tullis 2015-10-29 17:13) - PLID 67483 - Set the value
				SetDlgItemText(IDC_CAPITATION_DISTRIBUTION_EDIT, FormatString("%li",AdoFldLong(m_rsProv, "Capitation",0)));
				m_PendingClaimProvID = AdoFldLong(m_rsProv, "ClaimProviderID", -1);
				if(m_PendingClaimProvID == -1)
					m_ClaimProviderCombo->PutCurSel(NULL);
				else {
					// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
					m_ClaimProviderCombo->TrySetSelByColumn_Deprecated(0, m_PendingClaimProvID);
				}

				long nAMASpecialtyID = AdoFldLong(m_rsProv, "AMASpecialtyID", -1);
				if(nAMASpecialtyID != -1) {
					m_pAMASpecialtyList->SetSelByColumn(0, (long)nAMASpecialtyID);
				}
				else {
					m_pAMASpecialtyList->CurSel = NULL;
				}

				// (j.dinatale 2012-04-05 16:04) - PLID 49332 - optician field
				BOOL bOptician = AdoFldBool(m_rsProv, "Optician", FALSE);
				if(bOptician)
					m_btnOptician.SetCheck(true);
				else
					m_btnOptician.SetCheck(false);

				// (a.wilson 2014-04-21) PLID 61816 - provider types.
				CheckDlgButton(IDC_CONTACT_REFERRING_PROVIDER_CHECK, AdoFldBool(m_rsProv, "ReferringProvider"));
				CheckDlgButton(IDC_CONTACT_ORDERING_PROVIDER_CHECK, AdoFldBool(m_rsProv, "OrderingProvider"));
				CheckDlgButton(IDC_CONTACT_SUPERVISING_PROVIDER_CHECK, AdoFldBool(m_rsProv, "SupervisingProvider"));

				// (b.savon 2013-04-23 13:54) - PLID 56409 - NexERxProviderType
				long nNexERxProviderType = AdoFldLong(m_rsProv, "NexERxProviderTypeID", -1);
				// (b.savon 2013-06-12 12:25) - PLID 56867
				CString strRegisteredPrescriber = AdoFldString(m_rsProv, "RegisteredPrescriber");
				if(nNexERxProviderType > -1 || strRegisteredPrescriber.CompareNoCase("-1") != 0){
					m_bIsConfiguredNexERxPrescriber = TRUE;
				}else{
					m_bIsConfiguredNexERxPrescriber = FALSE;
				}
			}
			m_rsProv->Close();

		}


		
		else if (nStatus & 0x4)
		{
		/*	ContactsT is empty except for the PersonID
			_RecordsetPtr m_rsCon;
			str.Format("SELECT PersonID, Company, Spouse, OtherFirst, OtherLast, OtherRelation, OtherHomePhone, OtherWorkPhone FROM ContactsT WHERE ContactsT.PersonID = %li", m_id);
			m_rsCon = CreateRecordset(adOpenDynamic, adLockOptimistic, str);
		*/

			// (a.walling 2007-11-21 15:12) - PLID 28157 - Added DateOfTerm
			// (a.walling 2011-01-19 16:16) - PLID 40965 - Parameterized, removed dynamic cursor
			// (d.lange 2011-03-22 12:52) - PLID 42943 - Added Technician field
			// (j.dinatale 2012-04-05 16:03) - PLID 49332 - Optician field
			// (j.dinatale 2012-04-30 17:56) - PLID 49332 - removed the Optician field from UsersT
			_RecordsetPtr m_rsEmp = CreateParamRecordset(
				"SELECT PersonID, NationalEmplNumber, PatientCoordinator, Administrator, DateOfHire, \r\n"
				"	DateOfTerm, DefaultCost, Technician \r\n"
				"FROM UsersT \r\n"
				"WHERE UsersT.PersonID = {INT} \r\n", m_id);

			//load employee info
			if(!m_rsEmp->eof)
			{
				SetDlgItemText (IDC_NATIONALNUM_BOX,		CString(m_rsEmp->Fields->Item["NationalEmplNumber"]->Value.bstrVal));

				_variant_t varDate = m_rsEmp->Fields->Item["DateOfHire"]->Value;
				if(varDate.vt == VT_DATE) {
					m_dtDateOfHire = VarDateTime(varDate);
					m_nxtDateOfHire->SetDateTime(m_dtDateOfHire);
				}
				else {
					m_dtDateOfHire.SetDateTime(1899,12,30,0,0,0);
					m_dtDateOfHire.SetStatus(COleDateTime::invalid);
					m_nxtDateOfHire->Clear();
				}

				// (a.walling 2007-11-21 15:12) - PLID 28157
				varDate = m_rsEmp->Fields->Item["DateOfTerm"]->Value;
				if(varDate.vt == VT_DATE) {
					m_dtDateOfTerm = VarDateTime(varDate);
					m_nxtDateOfTerm->SetDateTime(m_dtDateOfTerm);
				}
				else {
					m_dtDateOfTerm.SetDateTime(1899,12,30,0,0,0);
					m_dtDateOfTerm.SetStatus(COleDateTime::invalid);
					m_nxtDateOfTerm->Clear();
				}
				
				if(m_rsEmp->Fields->Item["PatientCoordinator"]->Value.boolVal)
					m_btnPatCoord.SetCheck(true);
				else
					m_btnPatCoord.SetCheck(false);

				// (d.lange 2011-03-22 12:53) - PLID 42943 - Added Technician field
				if(m_rsEmp->Fields->Item["Technician"]->Value.boolVal)
					m_btnTechnician.SetCheck(true);
				else
					m_btnTechnician.SetCheck(false);

				if(m_rsEmp->Fields->Item["Administrator"]->Value.boolVal) {
					GetDlgItem(IDC_PERMISSIONS_BTN)->EnableWindow(FALSE);
				}
				else {
					GetDlgItem(IDC_PERMISSIONS_BTN)->EnableWindow(TRUE);
				}

				SetDlgItemText(IDC_DEFAULT_COST_EDIT, FormatCurrencyForInterface(AdoFldCurrency(m_rsEmp, "DefaultCost",COleCurrency(0,0)),TRUE,TRUE));
			}
			m_rsEmp->Close();
		}

		else if (nStatus & 0x8)
		{

			// (a.walling 2011-01-19 16:16) - PLID 40965 - Parameterized, removed dynamic cursor
			_RecordsetPtr m_rsSup = CreateParamRecordset("SELECT PersonID, CCNumber, AccountName FROM SupplierT WHERE SupplierT.PersonID = {INT}", m_id);

			//load supplier info
			if(!m_rsSup->eof)
			{
				// (a.walling 2007-10-31 09:41) - PLID 27891 - FYI, the SupplierT.CCNumber is really a misnomer; it can (and should) be 'Method of Payment'
				SetDlgItemText (IDC_METHOD_BOX,	CString(m_rsSup->Fields->Item["CCNumber"]->Value.bstrVal));

				//TES 2/18/2008 - PLID 28954 - We also need a field for the name of our account with this supplier.
				SetDlgItemText(IDC_ACCOUNT_NAME, AdoFldString(m_rsSup, "AccountName"));
			}
			m_rsSup->Close();
		
		}

		else if (nStatus & 0x1)
		{

			// (m.hancock 2006-11-13 10:06) - PLID 20917 - Moved the query for finding the counts into the existing query for
			// loading, which is where it should have been in the first place.
			// (z.manning 2009-05-05 08:41) - PLID 26074 - Added AMASpecialtyID
			// (a.walling 2011-01-19 16:16) - PLID 40965 - Parameterized, removed dynamic cursor
			// (j.gruber 2011-09-22 11:08) - PLID 45354 - added Affiliate Physician
			// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore.
			// (j.jones 2013-06-10 13:26) - PLID 57089 - added Group NPI
			// (a.wilson 2014-04-21) PLID 61816 - added provider types.
			_RecordsetPtr m_rsRef = CreateParamRecordset(
				"SELECT PersonID, ReferringPhyID, UPIN, BlueShieldID, NPI, AMASpecialtyID, \r\n"
				"	FedEmployerID, DEANumber, MedicareNumber, MedicaidNumber, \r\n"
				"	WorkersCompNumber, OtherIDNumber, License, AffiliatePhysician, \r\n"
				"	ReferringProvider, OrderingProvider, SupervisingProvider, \r\n"
				// (d.moore 2007-05-02 13:43) - PLID 23602 - 'CurrentStatus = 3' was used to determine both the number of Patients and the number of Prospects. It is now only counted as a Patient.
				"	(SELECT CONVERT(INT, COUNT(*)) FROM PatientsT WHERE DefaultReferringPhyID = {INT} AND CurrentStatus IN (1, 3)) AS CountOfReferredPatients, \r\n"
				"	(SELECT CONVERT(INT, COUNT(*)) FROM PatientsT WHERE DefaultReferringPhyID = {INT} AND CurrentStatus = 2) AS CountOfReferredProspects, \r\n"
				"	ReferringPhysT.GroupNPI "
				"FROM ReferringPhysT \r\n"
				"WHERE ReferringPhyST.PersonID = {INT} \r\n"
				, m_id, m_id, m_id);

			//load ref phys info
			if(!m_rsRef->eof)
			{
				SetDlgItemText (IDC_NPI_BOX,			CString(m_rsRef->Fields->Item["NPI"]->Value.bstrVal));
				SetDlgItemText (IDC_REFPHYSID_BOX,	CString(m_rsRef->Fields->Item["ReferringPhyID"]->Value.bstrVal));
				SetDlgItemText (IDC_UPIN_BOX,	CString(m_rsRef->Fields->Item["UPIN"]->Value.bstrVal));
				SetDlgItemText (IDC_FEDID_BOX,			CString(m_rsRef->Fields->Item["FedEmployerID"]->Value.bstrVal));
				SetDlgItemText (IDC_MEDICARE_BOX,		CString(m_rsRef->Fields->Item["MedicareNumber"]->Value.bstrVal));
				SetDlgItemText (IDC_BCBS_BOX,			CString(m_rsRef->Fields->Item["BlueShieldID"]->Value.bstrVal));
				SetDlgItemText (IDC_MEDICAID_BOX,		CString(m_rsRef->Fields->Item["MedicaidNumber"]->Value.bstrVal));
				SetDlgItemText (IDC_WORKCOMP_BOX,		CString(m_rsRef->Fields->Item["WorkersCompNumber"]->Value.bstrVal));
				SetDlgItemText (IDC_DEA_BOX,			CString(m_rsRef->Fields->Item["DEANumber"]->Value.bstrVal));
				SetDlgItemText (IDC_LICENSE_BOX,		CString(m_rsRef->Fields->Item["License"]->Value.bstrVal));
				SetDlgItemText (IDC_OTHERID_BOX,		CString(m_rsRef->Fields->Item["OtherIDNumber"]->Value.bstrVal));
				// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore.
				//SetDlgItemText (IDC_TAXONOMY_CODE,		CString(m_rsRef->Fields->Item["TaxonomyCode"]->Value.bstrVal));
				// (j.jones 2013-06-10 13:26) - PLID 57089 - added Group NPI
				SetDlgItemText (IDC_GROUP_NPI_BOX,		AdoFldString(m_rsRef, "GroupNPI"));

				// (z.manning 2009-05-05 08:39) - PLID 26074 - Ref phys now use the specialty dropdown too.
				long nAMASpecialtyID = AdoFldLong(m_rsRef, "AMASpecialtyID", -1);
				if(nAMASpecialtyID != -1) {
					m_pAMASpecialtyList->SetSelByColumn(0, nAMASpecialtyID);
				}
				else {
					m_pAMASpecialtyList->PutCurSel(NULL);
				}

				// (j.gruber 2011-09-22 11:09) - PLID 45354 - affiliate phys
				CheckDlgButton(IDC_AFFILIATE_PHYS, AdoFldBool(m_rsRef, "AffiliatePhysician"));

				// (a.wilson 2014-04-21) PLID 61816 - provider types.
				CheckDlgButton(IDC_CONTACT_REFERRING_PROVIDER_CHECK, AdoFldBool(m_rsRef, "ReferringProvider"));
				CheckDlgButton(IDC_CONTACT_ORDERING_PROVIDER_CHECK, AdoFldBool(m_rsRef, "OrderingProvider"));
				CheckDlgButton(IDC_CONTACT_SUPERVISING_PROVIDER_CHECK, AdoFldBool(m_rsRef, "SupervisingProvider"));

				// (m.hancock 2006-08-02 13:01) - PLID 20917 - Show the number of referred patients and prospects for referring physicians
				// Calc and set the "referred person" counts for this referring physician
				{
					// Put the counts on screen
					long nNumReferredPatients = AdoFldLong(m_rsRef, "CountOfReferredPatients");
					SetDlgItemInt(IDC_NUM_PATIENTS_REF_BOX, nNumReferredPatients);
					if(nNumReferredPatients <= 0){
						// grey out the elipsis button
						GetDlgItem(IDC_REFERRED_PATS)->EnableWindow(FALSE);
					}
					else{
						GetDlgItem(IDC_REFERRED_PATS)->EnableWindow(TRUE);
					}
						
					long nNumReferredProspects = AdoFldLong(m_rsRef, "CountOfReferredProspects");
					SetDlgItemInt(IDC_NUM_PROSPECTS_REF_BOX, nNumReferredProspects);
					if(nNumReferredProspects <= 0){
						// grey out the elipsis button
						GetDlgItem(IDC_REFERRED_PROSPECTS)->EnableWindow(FALSE);
					}
					else{
						GetDlgItem(IDC_REFERRED_PROSPECTS)->EnableWindow(TRUE);
					}
				}
			}
			m_rsRef->Close();

		}

		else
		{
			// (a.walling 2011-01-19 16:16) - PLID 40965 - Parameterized
			_RecordsetPtr rsCon = CreateParamRecordset("SELECT PersonID, Nurse, Anesthesiologist, DefaultCost FROM ContactsT WHERE PersonID = {INT}", m_id);
			if(!rsCon->eof)
			{
				CheckDlgButton(IDC_NURSE_CHECK, AdoFldBool(rsCon, "Nurse"));
				CheckDlgButton(IDC_ANESTHESIOLOGIST, AdoFldBool(rsCon, "Anesthesiologist"));

				SetDlgItemText(IDC_DEFAULT_COST_EDIT, FormatCurrencyForInterface(AdoFldCurrency(rsCon, "DefaultCost",COleCurrency(0,0)),TRUE,TRUE));
			}
			rsCon->Close();
		}

	//Refer|Main|Employee|Supplier|Other

		//for some reason, it won't redraw this item correctly, so we had to do it manually
		//keeping the code this way keeps flickering from occurring when we switch contacts
		//TODO:  Try to rewrite parts of this with Child dialogs inside the tab
		UpdateData(false);	//this is needed if the filter is reset from code (clicking a group that has no persons)
		if (nStatus & 0x1)
		{
			//(e.lally 2009-03-25) PLID 33634 - Dynamic label for standard/OHIP.
			// (j.jones 2010-11-03 15:11) - PLID 39620 - supported Alberta
			if(!UseOHIP() && !UseAlbertaHLINK()){
				m_nxstaticLabelNpi.SetWindowText("NPI");
			}
			else{
				m_nxstaticLabelNpi.SetWindowText("Ref Phys Number");
			}

			if(m_strContactType != "Referring Physician")
			{
				m_strContactType = "Referring Physician";
				UpdateData(false);
				CRect rect;
				GetDlgItem(IDC_CONTACT_TYPE)->GetWindowRect(rect);
				ScreenToClient(rect);
				InvalidateRect(rect);
			
				//referring physician info
				//show boxes if applicable
				CWnd* pWnd;

				pWnd = GetDlgItem(IDC_DEFAULT_COST_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DEFAULT_COST_EDIT);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_CHECK_CONTACT_INACTIVE);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_PERMISSIONS_BTN);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_PROPERTIES_BTN);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_EDITSECURITYGROUPS_BTN);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL19);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_NATIONALNUM_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_PATCOORD_CHECK);
				pWnd->ShowWindow(SW_HIDE);

				// (d.lange 2011-03-22 13:11) - PLID 42943 - Hide the Assistant/Technician checkbox
				pWnd = GetDlgItem(IDC_TECHNICIAN_CHECK);
				pWnd->ShowWindow(SW_HIDE);

				// (j.dinatale 2012-04-05 16:04) - PLID 49332 - Hide the Optician checkbox
				pWnd = GetDlgItem(IDC_OPTICIAN_CHECK);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DATE_OF_HIRE_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DATE_OF_HIRE);
				pWnd->ShowWindow(SW_HIDE);

				// (a.walling 2007-11-21 15:07) - PLID 28157
				pWnd = GetDlgItem(IDC_DATE_OF_TERM_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DATE_OF_TERM);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL23);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_REFPHYSID_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL24);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_UPIN_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL26);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_FEDID_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL27);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_MEDICARE_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL28);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_BCBS_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_NPI_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL_NPI);
				pWnd->ShowWindow(SW_SHOW);

				// (j.jones 2013-06-10 13:26) - PLID 57089 - added Group NPI, shown for referring physicians
				pWnd = GetDlgItem(IDC_GROUP_NPI_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL_GROUP_NPI);
				pWnd->ShowWindow(SW_SHOW);

				// (j.jones 2009-06-26 09:13) - PLID 34292 - added OHIP Specialty
				pWnd = GetDlgItem(IDC_OHIP_SPECIALTY);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_OHIP_SPECIALTY);
				pWnd->ShowWindow(SW_HIDE);

				// (j.jones 2011-11-07 14:10) - PLID 46299 - added ProvidersT.UseCompanyOnClaims				
				pWnd = GetDlgItem(IDC_CHECK_SEND_COMPANY_ON_CLAIM);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL29);
				pWnd->ShowWindow(SW_SHOW);

				// (j.jones 2012-03-06 17:47) - PLID 48675 - we no longer display the taxonomy code
				// for Referring Physicians, but we haven't permanently removed the field yet
				pWnd = GetDlgItem(IDC_TAXONOMY_CODE);
				pWnd->ShowWindow(SW_HIDE);

				//m_nxstaticLabelTaxonomyCode.SetWindowText("Taxonomy Code");
				pWnd = GetDlgItem(IDC_LABEL_TAXONOMY_CODE);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_MEDICAID_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL30);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_WORKCOMP_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL31);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_DEA_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL32);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LICENSE_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL33);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_OTHERID_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_METHOD_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_METHOD_BOX);
				pWnd->ShowWindow(SW_HIDE);

				//TES 2/18/2008 - PLID 28954 - New field
				pWnd = GetDlgItem(IDC_ACCOUNT_NAME_LABEL);
				pWnd->ShowWindow(SW_HIDE);
				pWnd = GetDlgItem(IDC_ACCOUNT_NAME);
				pWnd->ShowWindow(SW_HIDE);

				//(a.wilson 2014-04-21) PLID 61816 - show ref, ord, super provider checks and configure box.
				pWnd = GetDlgItem(IDC_CONTACT_REFERRING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_SHOW);
				pWnd->EnableWindow(TRUE);
				pWnd = GetDlgItem(IDC_CONTACT_ORDERING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_SHOW);
				pWnd->EnableWindow(TRUE);
				pWnd = GetDlgItem(IDC_CONTACT_SUPERVISING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_SHOW);
				pWnd->EnableWindow(TRUE);
				pWnd = GetDlgItem(IDC_CONTACT_CONFIGURE_PROVIDER_TYPES_BUTTON);
				pWnd->ShowWindow(SW_SHOW);
				pWnd->EnableWindow(TRUE);
				pWnd = GetDlgItem(IDC_CONTACT_PROVIDER_TYPES_LABEL);
				pWnd->ShowWindow(SW_SHOW);
				pWnd->EnableWindow(TRUE);
				pWnd = GetDlgItem(IDC_CONTACTS_BKG7);
				pWnd->ShowWindow(SW_SHOW);
				pWnd->EnableWindow(TRUE);
				//(a.wilson 2014-04-23) PLID 61825 - show icon
				m_icoProviderTypeInfo.EnableWindow(TRUE);
				m_icoProviderTypeInfo.ShowWindow(SW_SHOW);
				m_icoProviderTypeInfo.SetToolTip("Provider Types:\r\nAffiliate Physician – Alternate Physician responsible for providing pre-op and post-op services.\r\n\r\n"
					"Referring Provider – Referring provider at a charge level, not claim level. This provider is not the same as a Referring Physician. "
					"This provider will print in Box 17 of the HCFA form with a DN qualifier, and send in ANSI Loop 2420F.\r\n\r\n"
					"Ordering Provider – Ordering provider at a charge level, not claim level. "
					"This provider will print in Box 17 of the HCFA form with a DK qualifier, and send in ANSI Loop 2420E.\r\n\r\n"
					"Supervising Provider – Supervising provider at a charge level, not claim level. "
					"This provider will print in Box 17 of the HCFA form with a DQ qualifier, and send in ANSI Loop 2420D.");

				//TES 9/8/2008 - PLID 27727 - The location is now visible for all contacts.
				//GetDlgItem(IDC_LABEL_DEFLOCATION)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SHOW_COMMISSION)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NURSE_CHECK)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ANESTHESIOLOGIST)->ShowWindow(SW_HIDE);
				//GetDlgItem(IDC_PROVIDERLOCATION_COMBO)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SHOW_PROCS)->ShowWindow(SW_SHOW);

				// (j.jones 2006-12-01 09:47) - PLID 22110 - hide claim provider combo
				GetDlgItem(IDC_CLAIM_PROV_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CLAIM_PROVIDER)->ShowWindow(SW_HIDE);

				// (m.hancock 2006-08-02 11:28) - PLID 20917 - Show number of referred patients to referring physician display
				GetDlgItem(IDC_NUM_PATIENTS_REF_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_NUM_PATIENTS_REF_BOX)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_REFERRED_PATS)->ShowWindow(SW_SHOW);

				// (m.hancock 2006-08-02 11:28) - PLID 20917 - Show number of referred prospects to referring physician display
				GetDlgItem(IDC_NUM_PROSPECTS_REF_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_NUM_PROSPECTS_REF_BOX)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_REFERRED_PROSPECTS)->ShowWindow(SW_SHOW);

				// (z.manning, 06/06/2007) - PLID 23862 - These buttons are for providers only.
				GetDlgItem(IDC_EDIT_BIOGRAPHY)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SELECT_IMAGE)->ShowWindow(SW_HIDE);

				// (z.manning, 12/12/2007) - PLID 28216 - Attendance setup button is for users only.
				GetDlgItem(IDC_ATTENDANCE_SETUP)->ShowWindow(SW_HIDE);

				//DRT 11/20/2008 - PLID 32082
				GetDlgItem(IDC_PROV_SPI)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SPI_LABEL)->ShowWindow(SW_HIDE);
				// (s.tullis 2015-10-29 17:13) - PLID 67483
				ShowCapitation(FALSE);

				// (b.savon 2013-06-03 11:24) - PLID 56867 - Hide if not a provider
				m_btnRegisterPrescriber.EnableWindow(FALSE);
				m_btnRegisterPrescriber.ShowWindow(SW_HIDE);

				// (z.manning 2009-05-04 17:54) - PLID 26074 - We now show the specialty list for
				// referring physicians.
				GetDlgItem(IDC_AMA_SPECIALTY_LIST)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_AMA_SPECIALTY_LABEL)->ShowWindow(SW_SHOW);

				// (j.gruber 2011-09-22 11:12) - PLID 45354 - affiliate phys
				GetDlgItem(IDC_AFFILIATE_PHYS)->ShowWindow(SW_SHOW);	

				// (b.spivey -- October 16th, 2013) - PLID 59022 - hide
				// (b.spivey -- November 12th, 2013) - PLID 59022 - Show the text box, we want that value. This is probably temporary. 
				GetDlgItem(IDC_DIRECT_ADDRESS_USERS_BUTTON)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_DIRECT_ADDRESS_EDIT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_DIRECT_ADDRESS_LABEL)->ShowWindow(SW_SHOW);

				GetDlgItem(IDC_DIRECT_ADDRESS_USERS_BUTTON)->EnableWindow(FALSE);
				GetDlgItem(IDC_DIRECT_ADDRESS_EDIT)->EnableWindow(TRUE);
				GetDlgItem(IDC_DIRECT_ADDRESS_LABEL)->EnableWindow(TRUE);
			}
		}
		else if (nStatus & 0x2)
		{
			CWnd* pWnd;

			//(e.lally 2009-03-25) PLID 33634 - Dynamic label for standard/OHIP.
			// (j.jones 2009-06-26 09:13) - PLID 34292 - added OHIP Specialty,
			// which replaces the taxonomy code
			// (j.jones 2010-11-03 15:11) - PLID 39620 - supported Alberta
			if(!UseOHIP() && !UseAlbertaHLINK()) {
				m_nxstaticLabelNpi.SetWindowText("NPI");
			}
			else {
				m_nxstaticLabelNpi.SetWindowText("Provider Number");
			}

			if(!UseAlbertaHLINK()) {
				m_nxstaticLabelTaxonomyCode.SetWindowText("Taxonomy Code");
			}
			else {
				m_nxstaticLabelTaxonomyCode.SetWindowText("Skill Code");
			}

			if(!UseOHIP()) {
				pWnd = GetDlgItem(IDC_LABEL_TAXONOMY_CODE);
				pWnd->ShowWindow(SW_SHOW);		
				pWnd = GetDlgItem(IDC_TAXONOMY_CODE);
				pWnd->ShowWindow(SW_SHOW);
				pWnd = GetDlgItem(IDC_LABEL_OHIP_SPECIALTY);
				pWnd->ShowWindow(SW_HIDE);				
				pWnd = GetDlgItem(IDC_OHIP_SPECIALTY);
				pWnd->ShowWindow(SW_HIDE);
				// (j.jones 2011-11-07 14:10) - PLID 46299 - added ProvidersT.UseCompanyOnClaims
				pWnd = GetDlgItem(IDC_CHECK_SEND_COMPANY_ON_CLAIM);
				pWnd->ShowWindow(SW_SHOW);
			}
			else {
				pWnd = GetDlgItem(IDC_LABEL_TAXONOMY_CODE);
				pWnd->ShowWindow(SW_HIDE);
				pWnd = GetDlgItem(IDC_TAXONOMY_CODE);
				pWnd->ShowWindow(SW_HIDE);
				pWnd = GetDlgItem(IDC_LABEL_OHIP_SPECIALTY);
				pWnd->ShowWindow(SW_SHOW);				
				pWnd = GetDlgItem(IDC_OHIP_SPECIALTY);
				pWnd->ShowWindow(SW_SHOW);
				// (j.jones 2011-11-07 14:10) - PLID 46299 - added ProvidersT.UseCompanyOnClaims, not used in OHIP
				pWnd = GetDlgItem(IDC_CHECK_SEND_COMPANY_ON_CLAIM);
				pWnd->ShowWindow(SW_HIDE);
			}

			if(m_strContactType != "Provider")
			{
				m_strContactType = "Provider";
				UpdateData(false);
				CRect rect;
				GetDlgItem(IDC_CONTACT_TYPE)->GetWindowRect(rect);
				ScreenToClient(rect);
				InvalidateRect(rect);
			
				//provider info
				//show boxes if applicable				

				if(IsSurgeryCenter(FALSE) && (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead)) {
					// (j.jones 2005-04-15 17:09) - the default cost field is only available in ASC
					pWnd = GetDlgItem(IDC_DEFAULT_COST_LABEL);
					pWnd->ShowWindow(SW_SHOW);

					pWnd = GetDlgItem(IDC_DEFAULT_COST_EDIT);
					pWnd->ShowWindow(SW_SHOW);
				}
				else {
					pWnd = GetDlgItem(IDC_DEFAULT_COST_LABEL);
					pWnd->ShowWindow(SW_HIDE);

					pWnd = GetDlgItem(IDC_DEFAULT_COST_EDIT);
					pWnd->ShowWindow(SW_HIDE);
				}

				pWnd = GetDlgItem(IDC_CHECK_CONTACT_INACTIVE);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_PERMISSIONS_BTN);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_PROPERTIES_BTN);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_EDITSECURITYGROUPS_BTN);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL19);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_NATIONALNUM_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_PATCOORD_CHECK);
				pWnd->ShowWindow(SW_HIDE);

				// (d.lange 2011-03-22 13:11) - PLID 42943 - Hide the Assistant/Technician checkbox
				pWnd = GetDlgItem(IDC_TECHNICIAN_CHECK);
				pWnd->ShowWindow(SW_HIDE);

				// (j.dinatale 2012-04-05 16:04) - PLID 49332 - show/hide the Optician checkbox
				if(g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)){
					pWnd = GetDlgItem(IDC_OPTICIAN_CHECK);
					pWnd->ShowWindow(SW_SHOW);
				}else{
					pWnd = GetDlgItem(IDC_OPTICIAN_CHECK);
					pWnd->ShowWindow(SW_HIDE);
				}

				pWnd = GetDlgItem(IDC_DATE_OF_HIRE_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DATE_OF_HIRE);
				pWnd->ShowWindow(SW_HIDE);

				// (a.walling 2007-11-21 15:07) - PLID 28157
				pWnd = GetDlgItem(IDC_DATE_OF_TERM_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DATE_OF_TERM);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL23);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_REFPHYSID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL24);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_UPIN_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL26);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_FEDID_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL27);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_MEDICARE_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL28);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_BCBS_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_NPI_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL_NPI);
				pWnd->ShowWindow(SW_SHOW);

				// (j.jones 2013-06-10 13:26) - PLID 57089 - added Group NPI, hidden for providers
				pWnd = GetDlgItem(IDC_GROUP_NPI_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_GROUP_NPI);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL29);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_MEDICAID_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL30);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_WORKCOMP_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL31);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_DEA_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL32);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LICENSE_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL33);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_OTHERID_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_METHOD_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_METHOD_BOX);
				pWnd->ShowWindow(SW_HIDE);

				//(a.wilson 2014-04-21) PLID 61816 - show ref, ord, super provider checks and configure box.
				pWnd = GetDlgItem(IDC_CONTACT_REFERRING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_SHOW);
				pWnd->EnableWindow(TRUE);
				pWnd = GetDlgItem(IDC_CONTACT_ORDERING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_SHOW);
				pWnd->EnableWindow(TRUE);
				pWnd = GetDlgItem(IDC_CONTACT_SUPERVISING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_SHOW);
				pWnd->EnableWindow(TRUE);
				pWnd = GetDlgItem(IDC_CONTACT_CONFIGURE_PROVIDER_TYPES_BUTTON);
				pWnd->ShowWindow(SW_SHOW);
				pWnd->EnableWindow(TRUE);
				pWnd = GetDlgItem(IDC_CONTACT_PROVIDER_TYPES_LABEL);
				pWnd->ShowWindow(SW_SHOW);
				pWnd->EnableWindow(TRUE);
				pWnd = GetDlgItem(IDC_CONTACTS_BKG7);
				pWnd->ShowWindow(SW_SHOW);
				pWnd->EnableWindow(TRUE);
				//(a.wilson 2014-04-23) PLID 61825 - show icon
				m_icoProviderTypeInfo.EnableWindow(TRUE);
				m_icoProviderTypeInfo.ShowWindow(SW_SHOW);
				CString strToolTipText = ("Provider Types\r\n");
				if (g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)){
					strToolTipText += ("Optician – Responsible for placing glasses orders.\r\n\r\n");
				}
				strToolTipText += ("Referring Provider – Referring provider at a charge level, not claim level. "
					"This provider is not the same as a Referring Physician."
					"This provider will print in Box 17 of the HCFA form with a DN qualifier, and send in ANSI Loop 2420F.\r\n\r\n"
					"Ordering Provider – Ordering provider at a charge level, not claim level. "
					"This provider will print in Box 17 of the HCFA form with a DK qualifier, and send in ANSI Loop 2420E.\r\n\r\n"
					"Supervising Provider – Supervising provider at a charge level, not claim level. "
					"This provider will print in Box 17 of the HCFA form with a DQ qualifier, and send in ANSI Loop 2420D.");
				m_icoProviderTypeInfo.SetToolTip(strToolTipText);

				//TES 2/18/2008 - PLID 28954 - New field
				pWnd = GetDlgItem(IDC_ACCOUNT_NAME_LABEL);
				pWnd->ShowWindow(SW_HIDE);
				pWnd = GetDlgItem(IDC_ACCOUNT_NAME);
				pWnd->ShowWindow(SW_HIDE);

				//Make sure they have access to NexSpa before we let them use this.
				if(IsSpa(FALSE))
					GetDlgItem(IDC_SHOW_COMMISSION)->ShowWindow(SW_SHOW);
				else
					GetDlgItem(IDC_SHOW_COMMISSION)->ShowWindow(SW_HIDE);

				GetDlgItem(IDC_NURSE_CHECK)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ANESTHESIOLOGIST)->ShowWindow(SW_HIDE);
				//TES 9/8/2008 - PLID 27727 - The location is now visible for all contacts.
				//GetDlgItem(IDC_LABEL_DEFLOCATION)->ShowWindow(SW_SHOW);
				//GetDlgItem(IDC_PROVIDERLOCATION_COMBO)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SHOW_PROCS)->ShowWindow(SW_HIDE);

				// (j.jones 2006-12-01 09:47) - PLID 22110 - show claim provider combo
				GetDlgItem(IDC_CLAIM_PROV_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CLAIM_PROVIDER)->ShowWindow(SW_SHOW);

				// (m.hancock 2006-08-02 11:38) - PLID 20917 - Hide number of referred patients
				GetDlgItem(IDC_NUM_PATIENTS_REF_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NUM_PATIENTS_REF_BOX)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_REFERRED_PATS)->ShowWindow(SW_HIDE);

				// (m.hancock 2006-08-02 11:38) - PLID 20917 - Hide number of referred prospects
				GetDlgItem(IDC_NUM_PROSPECTS_REF_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NUM_PROSPECTS_REF_BOX)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_REFERRED_PROSPECTS)->ShowWindow(SW_HIDE);

				// (z.manning, 06/06/2007) - PLID 23862 - Ok, we're on a provider. If they have NexForms
				// then show the buttons to edit biographical info and select an image.
				if(g_pLicense->CheckForLicense(CLicense::lcNexForms, CLicense::cflrSilent)) {
					GetDlgItem(IDC_EDIT_BIOGRAPHY)->ShowWindow(SW_SHOW);
					GetDlgItem(IDC_SELECT_IMAGE)->ShowWindow(SW_SHOW);
				}

				// (z.manning, 12/12/2007) - PLID 28216 - Attendance setup button is for users only.
				GetDlgItem(IDC_ATTENDANCE_SETUP)->ShowWindow(SW_HIDE);

				//DRT 11/20/2008 - PLID 32082
				GetDlgItem(IDC_PROV_SPI)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_AMA_SPECIALTY_LIST)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SPI_LABEL)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_AMA_SPECIALTY_LABEL)->ShowWindow(SW_SHOW);
				// (s.tullis 2015-10-29 17:13) - PLID 67483
				ShowCapitation();

				// (b.savon 2013-06-03 11:24) - PLID 56867 - Only show if the user is an administrator
				// and do they have the license
				// (b.savon 2013-09-18 09:23) - PLID 58551 - Only allow NexTech Technical support to Register NexERx Prescribers
				if( GetCurrentUserID() == BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID &&
					g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts){
					m_btnRegisterPrescriber.AutoSet(NXB_PILLBOTTLE);
					m_btnRegisterPrescriber.EnableWindow(TRUE);
					m_btnRegisterPrescriber.ShowWindow(SW_SHOW);
				}

				// (j.gruber 2011-09-22 11:12) - PLID 45354 - affiliate phys
				GetDlgItem(IDC_AFFILIATE_PHYS)->ShowWindow(SW_HIDE);		

				// (b.spivey -- October 16th, 2013) - PLID 59022 - show
				GetDlgItem(IDC_DIRECT_ADDRESS_USERS_BUTTON)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_DIRECT_ADDRESS_EDIT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_DIRECT_ADDRESS_LABEL)->ShowWindow(SW_SHOW);

				// (d.singleton 2014-04-22 10:38) - PLID 61810 - exception when adding users to a providers direct address. if address is empty
				CString strAddress;
				GetDlgItemText(IDC_DIRECT_ADDRESS_EDIT, strAddress);
				GetDlgItem(IDC_DIRECT_ADDRESS_USERS_BUTTON)->EnableWindow(!strAddress.IsEmpty());
				GetDlgItem(IDC_DIRECT_ADDRESS_EDIT)->EnableWindow(TRUE);
				GetDlgItem(IDC_DIRECT_ADDRESS_LABEL)->EnableWindow(TRUE);
			}
		}
		else if (nStatus & 0x4)
		{
			if(m_strContactType != "User")
			{
				m_strContactType = "User";
				UpdateData(false);
				CRect rect;
				GetDlgItem(IDC_CONTACT_TYPE)->GetWindowRect(rect);
				ScreenToClient(rect);
				InvalidateRect(rect);
					
				//user info
				CWnd* pWnd;

				if(IsSurgeryCenter(FALSE) && (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead)) {
					// (j.jones 2005-04-15 17:09) - the default cost field is only available in ASC
					pWnd = GetDlgItem(IDC_DEFAULT_COST_LABEL);
					pWnd->ShowWindow(SW_SHOW);

					pWnd = GetDlgItem(IDC_DEFAULT_COST_EDIT);
					pWnd->ShowWindow(SW_SHOW);
				}
				else {
					pWnd = GetDlgItem(IDC_DEFAULT_COST_LABEL);
					pWnd->ShowWindow(SW_HIDE);

					pWnd = GetDlgItem(IDC_DEFAULT_COST_EDIT);
					pWnd->ShowWindow(SW_HIDE);
				}

				pWnd = GetDlgItem(IDC_CHECK_CONTACT_INACTIVE);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_PERMISSIONS_BTN);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_PROPERTIES_BTN);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_EDITSECURITYGROUPS_BTN);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL19);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_NATIONALNUM_BOX);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_PATCOORD_CHECK);
				pWnd->ShowWindow(SW_SHOW);

				// (d.lange 2011-03-22 13:11) - PLID 42943 - Hide the Assistant/Technician checkbox
				pWnd = GetDlgItem(IDC_TECHNICIAN_CHECK);
				pWnd->ShowWindow(SW_SHOW);

				// (j.dinatale 2012-04-05 16:04) - PLID 49332 - hide the Optician checkbox based on license
				pWnd = GetDlgItem(IDC_OPTICIAN_CHECK);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DATE_OF_HIRE_LABEL);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_DATE_OF_HIRE);
				pWnd->ShowWindow(SW_SHOW);

				// (a.walling 2007-11-21 15:07) - PLID 28157
				pWnd = GetDlgItem(IDC_DATE_OF_TERM_LABEL);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_DATE_OF_TERM);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_LABEL23);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_REFPHYSID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL24);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_UPIN_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL26);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_FEDID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL27);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_MEDICARE_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL28);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_BCBS_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_NPI_BOX);
				pWnd->ShowWindow(SW_HIDE);			

				pWnd = GetDlgItem(IDC_LABEL_NPI);
				pWnd->ShowWindow(SW_HIDE);

				// (j.jones 2013-06-10 13:26) - PLID 57089 - added Group NPI
				pWnd = GetDlgItem(IDC_GROUP_NPI_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_GROUP_NPI);
				pWnd->ShowWindow(SW_HIDE);

				// (j.jones 2009-06-26 09:13) - PLID 34292 - added OHIP Specialty
				pWnd = GetDlgItem(IDC_OHIP_SPECIALTY);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_OHIP_SPECIALTY);
				pWnd->ShowWindow(SW_HIDE);

				// (j.jones 2011-11-07 14:10) - PLID 46299 - added ProvidersT.UseCompanyOnClaims, not used in OHIP
				pWnd = GetDlgItem(IDC_CHECK_SEND_COMPANY_ON_CLAIM);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL29);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_TAXONOMY_CODE);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_TAXONOMY_CODE);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_MEDICAID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL30);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_WORKCOMP_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL31);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DEA_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL32);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LICENSE_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL33);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_OTHERID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_METHOD_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_METHOD_BOX);
				pWnd->ShowWindow(SW_HIDE);

				//TES 2/18/2008 - PLID 28954 - New field
				pWnd = GetDlgItem(IDC_ACCOUNT_NAME_LABEL);
				pWnd->ShowWindow(SW_HIDE);
				pWnd = GetDlgItem(IDC_ACCOUNT_NAME);
				pWnd->ShowWindow(SW_HIDE);

				//(a.wilson 2014-04-21) PLID 61816 - show ref, ord, super provider checks and configure box.
				pWnd = GetDlgItem(IDC_CONTACT_REFERRING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACT_ORDERING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACT_SUPERVISING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACT_CONFIGURE_PROVIDER_TYPES_BUTTON);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACT_PROVIDER_TYPES_LABEL);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACTS_BKG7);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				//(a.wilson 2014-04-23) PLID 61825 - hide icon
				pWnd = GetDlgItem(IDC_CONTACTS_PROVIDER_TYPE_INFO);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);

				//TES 9/8/2008 - PLID 27727 - The location is now visible for all contacts.
				GetDlgItem(IDC_SHOW_COMMISSION)->ShowWindow(SW_HIDE);
				//GetDlgItem(IDC_LABEL_DEFLOCATION)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NURSE_CHECK)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ANESTHESIOLOGIST)->ShowWindow(SW_HIDE);
				//GetDlgItem(IDC_PROVIDERLOCATION_COMBO)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SHOW_PROCS)->ShowWindow(SW_HIDE);

				// (j.jones 2006-12-01 09:47) - PLID 22110 - hide claim provider combo
				GetDlgItem(IDC_CLAIM_PROV_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CLAIM_PROVIDER)->ShowWindow(SW_HIDE);

				// (m.hancock 2006-08-02 11:38) - PLID 20917 - Hide number of referred patients
				GetDlgItem(IDC_NUM_PATIENTS_REF_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NUM_PATIENTS_REF_BOX)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_REFERRED_PATS)->ShowWindow(SW_HIDE);

				// (m.hancock 2006-08-02 11:38) - PLID 20917 - Hide number of referred prospects
				GetDlgItem(IDC_NUM_PROSPECTS_REF_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NUM_PROSPECTS_REF_BOX)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_REFERRED_PROSPECTS)->ShowWindow(SW_HIDE);

				// (z.manning, 06/06/2007) - PLID 23862 - These buttons are for providers only.
				GetDlgItem(IDC_EDIT_BIOGRAPHY)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SELECT_IMAGE)->ShowWindow(SW_HIDE);

				// (z.manning, 12/12/2007) - PLID 28216 - Attendance setup button is for users only.
				// (z.manning, 12/12/2007) - PLID 28218 - Internal only for now
				if(IsNexTechInternal()) {
					GetDlgItem(IDC_ATTENDANCE_SETUP)->ShowWindow(SW_SHOW);
				}

				//DRT 11/20/2008 - PLID 32082
				GetDlgItem(IDC_PROV_SPI)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_AMA_SPECIALTY_LIST)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SPI_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_AMA_SPECIALTY_LABEL)->ShowWindow(SW_HIDE);
				// (s.tullis 2015-10-29 17:13) - PLID 67483
				ShowCapitation(FALSE);

				// (b.savon 2013-06-03 11:24) - PLID 56867 - Hide if not a provider
				m_btnRegisterPrescriber.EnableWindow(FALSE);
				m_btnRegisterPrescriber.ShowWindow(SW_HIDE);

				// (j.gruber 2011-09-22 11:12) - PLID 45354 - affiliate phys
				GetDlgItem(IDC_AFFILIATE_PHYS)->ShowWindow(SW_HIDE);		

				// (b.spivey -- October 16th, 2013) - PLID 59022 - hide
				// (b.spivey -- November 13th, 2013) - PLID 59022 - Show the text box, we want that value. 
				GetDlgItem(IDC_DIRECT_ADDRESS_USERS_BUTTON)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_DIRECT_ADDRESS_EDIT)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_DIRECT_ADDRESS_LABEL)->ShowWindow(SW_SHOW);

				GetDlgItem(IDC_DIRECT_ADDRESS_USERS_BUTTON)->EnableWindow(FALSE);
				GetDlgItem(IDC_DIRECT_ADDRESS_EDIT)->EnableWindow(TRUE);
				GetDlgItem(IDC_DIRECT_ADDRESS_LABEL)->EnableWindow(TRUE);
			}

		}
		else if (nStatus & 0x8)
		{
			if(m_strContactType != "Supplier")
			{
				m_strContactType = "Supplier";
				UpdateData(false);
				CRect rect;
				GetDlgItem(IDC_CONTACT_TYPE)->GetWindowRect(rect);
				ScreenToClient(rect);
				InvalidateRect(rect);
					
				//supplier info
				CWnd* pWnd;
				pWnd = GetDlgItem(IDC_DEFAULT_COST_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DEFAULT_COST_EDIT);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_CHECK_CONTACT_INACTIVE);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_PERMISSIONS_BTN);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_PROPERTIES_BTN);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_EDITSECURITYGROUPS_BTN);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL19);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_NATIONALNUM_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_PATCOORD_CHECK);
				pWnd->ShowWindow(SW_HIDE);

				// (d.lange 2011-03-22 13:11) - PLID 42943 - Hide the Assistant/Technician checkbox
				pWnd = GetDlgItem(IDC_TECHNICIAN_CHECK);
				pWnd->ShowWindow(SW_HIDE);

				// (j.dinatale 2012-04-05 16:04) - PLID 49332 - Hide the Optician checkbox
				pWnd = GetDlgItem(IDC_OPTICIAN_CHECK);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DATE_OF_HIRE_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DATE_OF_HIRE);
				pWnd->ShowWindow(SW_HIDE);

				// (a.walling 2007-11-21 15:07) - PLID 28157
				pWnd = GetDlgItem(IDC_DATE_OF_TERM_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DATE_OF_TERM);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL23);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_REFPHYSID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL24);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_UPIN_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL26);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_FEDID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL27);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_MEDICARE_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL28);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_BCBS_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_NPI_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_NPI);
				pWnd->ShowWindow(SW_HIDE);

				// (j.jones 2013-06-10 13:26) - PLID 57089 - added Group NPI
				pWnd = GetDlgItem(IDC_GROUP_NPI_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_GROUP_NPI);
				pWnd->ShowWindow(SW_HIDE);

				// (j.jones 2009-06-26 09:13) - PLID 34292 - added OHIP Specialty
				pWnd = GetDlgItem(IDC_OHIP_SPECIALTY);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_OHIP_SPECIALTY);
				pWnd->ShowWindow(SW_HIDE);

				// (j.jones 2011-11-07 14:10) - PLID 46299 - added ProvidersT.UseCompanyOnClaims, not used in OHIP
				pWnd = GetDlgItem(IDC_CHECK_SEND_COMPANY_ON_CLAIM);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL29);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_TAXONOMY_CODE);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_TAXONOMY_CODE);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_MEDICAID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL30);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_WORKCOMP_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL31);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DEA_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL32);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LICENSE_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL33);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_OTHERID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_METHOD_LABEL);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_METHOD_BOX);
				pWnd->ShowWindow(SW_SHOW);

				//TES 2/18/2008 - PLID 28954 - New field
				pWnd = GetDlgItem(IDC_ACCOUNT_NAME_LABEL);
				pWnd->ShowWindow(SW_SHOW);
				pWnd = GetDlgItem(IDC_ACCOUNT_NAME);
				pWnd->ShowWindow(SW_SHOW);

				//(a.wilson 2014-04-21) PLID 61816 - show ref, ord, super provider checks and configure box.
				pWnd = GetDlgItem(IDC_CONTACT_REFERRING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACT_ORDERING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACT_SUPERVISING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACT_CONFIGURE_PROVIDER_TYPES_BUTTON);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACT_PROVIDER_TYPES_LABEL);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACTS_BKG7);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				//(a.wilson 2014-04-23) PLID 61825 - hide icon
				pWnd = GetDlgItem(IDC_CONTACTS_PROVIDER_TYPE_INFO);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);

				//TES 9/8/2008 - PLID 27727 - The location is now visible for all contacts.
				GetDlgItem(IDC_SHOW_COMMISSION)->ShowWindow(SW_HIDE);
				//GetDlgItem(IDC_LABEL_DEFLOCATION)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NURSE_CHECK)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_ANESTHESIOLOGIST)->ShowWindow(SW_HIDE);
				//GetDlgItem(IDC_PROVIDERLOCATION_COMBO)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SHOW_PROCS)->ShowWindow(SW_HIDE);

				// (j.jones 2006-12-01 09:47) - PLID 22110 - hide claim provider combo
				GetDlgItem(IDC_CLAIM_PROV_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CLAIM_PROVIDER)->ShowWindow(SW_HIDE);

				// (m.hancock 2006-08-02 11:38) - PLID 20917 - Hide number of referred patients
				GetDlgItem(IDC_NUM_PATIENTS_REF_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NUM_PATIENTS_REF_BOX)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_REFERRED_PATS)->ShowWindow(SW_HIDE);

				// (m.hancock 2006-08-02 11:38) - PLID 20917 - Hide number of referred prospects
				GetDlgItem(IDC_NUM_PROSPECTS_REF_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NUM_PROSPECTS_REF_BOX)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_REFERRED_PROSPECTS)->ShowWindow(SW_HIDE);

				// (z.manning, 06/06/2007) - PLID 23862 - These buttons are for providers only.
				GetDlgItem(IDC_EDIT_BIOGRAPHY)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SELECT_IMAGE)->ShowWindow(SW_HIDE);

				// (z.manning, 12/12/2007) - PLID 28216 - Attendance setup button is for users only.
				GetDlgItem(IDC_ATTENDANCE_SETUP)->ShowWindow(SW_HIDE);

				//DRT 11/20/2008 - PLID 32082
				GetDlgItem(IDC_PROV_SPI)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_AMA_SPECIALTY_LIST)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SPI_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_AMA_SPECIALTY_LABEL)->ShowWindow(SW_HIDE);
				// (s.tullis 2015-10-29 17:13) - PLID 67483
				ShowCapitation(FALSE);

				// (b.savon 2013-06-03 11:24) - PLID 56867 - Hide if not a provider
				m_btnRegisterPrescriber.EnableWindow(FALSE);
				m_btnRegisterPrescriber.ShowWindow(SW_HIDE);

				// (j.gruber 2011-09-22 11:12) - PLID 45354 - affiliate phys
				GetDlgItem(IDC_AFFILIATE_PHYS)->ShowWindow(SW_HIDE);	

				// (b.spivey -- October 16th, 2013) - PLID 59022 - hide
				GetDlgItem(IDC_DIRECT_ADDRESS_USERS_BUTTON)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_DIRECT_ADDRESS_EDIT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_DIRECT_ADDRESS_LABEL)->ShowWindow(SW_HIDE);

				GetDlgItem(IDC_DIRECT_ADDRESS_USERS_BUTTON)->EnableWindow(FALSE);
				GetDlgItem(IDC_DIRECT_ADDRESS_EDIT)->EnableWindow(FALSE);
				GetDlgItem(IDC_DIRECT_ADDRESS_LABEL)->EnableWindow(FALSE);
			}
		}
		else 
		{
			CWnd* pWnd;

			// (j.jones 2005-04-15 17:11) - this will be shown if ASC and if
			// the contact is a Nurse or Anesthesiologist, which is loaded above
			//
			//(e.lally 2009-09-09) PLID 32981 - We need to check the Nurse/Anesth. setting for each load, before it would only check 
			//if switching to an Other contact from some other contact type. This is person specific.
			if(IsSurgeryCenter(FALSE) && (IsDlgButtonChecked(IDC_NURSE_CHECK) || IsDlgButtonChecked(IDC_ANESTHESIOLOGIST))
				&& (GetCurrentUserPermissions(bioContactsDefaultCost) & sptRead)) {
				// (j.jones 2005-04-15 17:09) - the default cost field is only available in ASC
				pWnd = GetDlgItem(IDC_DEFAULT_COST_LABEL);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_DEFAULT_COST_EDIT);
				pWnd->ShowWindow(SW_SHOW);
			}
			else {
				pWnd = GetDlgItem(IDC_DEFAULT_COST_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DEFAULT_COST_EDIT);
				pWnd->ShowWindow(SW_HIDE);
			}

			if(m_strContactType != "Other")
			{
				m_strContactType = "Other";
				UpdateData(false);
				CRect rect;
				GetDlgItem(IDC_CONTACT_TYPE)->GetWindowRect(rect);
				ScreenToClient(rect);
				InvalidateRect(rect);
			
				//other contact info

				pWnd = GetDlgItem(IDC_CHECK_CONTACT_INACTIVE);
				pWnd->ShowWindow(SW_SHOW);

				pWnd = GetDlgItem(IDC_PERMISSIONS_BTN);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_PROPERTIES_BTN);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_EDITSECURITYGROUPS_BTN);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL19);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_NATIONALNUM_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_PATCOORD_CHECK);
				pWnd->ShowWindow(SW_HIDE);

				// (d.lange 2011-03-22 13:11) - PLID 42943 - Hide the Assistant/Technician checkbox
				pWnd = GetDlgItem(IDC_TECHNICIAN_CHECK);
				pWnd->ShowWindow(SW_HIDE);

				// (j.dinatale 2012-04-05 16:04) - PLID 49332 - Hide the Optician checkbox
				pWnd = GetDlgItem(IDC_OPTICIAN_CHECK);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DATE_OF_HIRE_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DATE_OF_HIRE);
				pWnd->ShowWindow(SW_HIDE);

				// (a.walling 2007-11-21 15:07) - PLID 28157
				pWnd = GetDlgItem(IDC_DATE_OF_TERM_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DATE_OF_TERM);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL23);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_REFPHYSID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL24);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_UPIN_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL26);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_FEDID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL27);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_MEDICARE_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL28);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_BCBS_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_NPI_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_NPI);
				pWnd->ShowWindow(SW_HIDE);

				// (j.jones 2013-06-10 13:26) - PLID 57089 - added Group NPI
				pWnd = GetDlgItem(IDC_GROUP_NPI_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_GROUP_NPI);
				pWnd->ShowWindow(SW_HIDE);

				// (j.jones 2009-06-26 09:13) - PLID 34292 - added OHIP Specialty
				pWnd = GetDlgItem(IDC_OHIP_SPECIALTY);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_OHIP_SPECIALTY);
				pWnd->ShowWindow(SW_HIDE);

				// (j.jones 2011-11-07 14:10) - PLID 46299 - added ProvidersT.UseCompanyOnClaims, not used in OHIP
				pWnd = GetDlgItem(IDC_CHECK_SEND_COMPANY_ON_CLAIM);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL29);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_TAXONOMY_CODE);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL_TAXONOMY_CODE);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_MEDICAID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL30);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_WORKCOMP_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL31);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_DEA_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL32);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LICENSE_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_LABEL33);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_OTHERID_BOX);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_METHOD_LABEL);
				pWnd->ShowWindow(SW_HIDE);

				pWnd = GetDlgItem(IDC_METHOD_BOX);
				pWnd->ShowWindow(SW_HIDE);

				//TES 2/18/2008 - PLID 28954 - New field
				pWnd = GetDlgItem(IDC_ACCOUNT_NAME_LABEL);
				pWnd->ShowWindow(SW_HIDE);
				pWnd = GetDlgItem(IDC_ACCOUNT_NAME);
				pWnd->ShowWindow(SW_HIDE);

				//(a.wilson 2014-04-21) PLID 61816 - show ref, ord, super provider checks and configure box.
				pWnd = GetDlgItem(IDC_CONTACT_REFERRING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACT_ORDERING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACT_SUPERVISING_PROVIDER_CHECK);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACT_CONFIGURE_PROVIDER_TYPES_BUTTON);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACT_PROVIDER_TYPES_LABEL);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				pWnd = GetDlgItem(IDC_CONTACTS_BKG7);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);
				//(a.wilson 2014-04-23) PLID 61825 - hide icon
				pWnd = GetDlgItem(IDC_CONTACTS_PROVIDER_TYPE_INFO);
				pWnd->ShowWindow(SW_HIDE);
				pWnd->EnableWindow(FALSE);

				//TES 9/8/2008 - PLID 27727 - The location is now visible for all contacts.
				GetDlgItem(IDC_SHOW_COMMISSION)->ShowWindow(SW_HIDE);
				//GetDlgItem(IDC_LABEL_DEFLOCATION)->ShowWindow(SW_HIDE);
				//GetDlgItem(IDC_PROVIDERLOCATION_COMBO)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NURSE_CHECK)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_ANESTHESIOLOGIST)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_SHOW_PROCS)->ShowWindow(SW_HIDE);

				// (j.jones 2006-12-01 09:47) - PLID 22110 - hide claim provider combo
				GetDlgItem(IDC_CLAIM_PROV_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_CLAIM_PROVIDER)->ShowWindow(SW_HIDE);

				// (m.hancock 2006-08-02 11:38) - PLID 20917 - Hide number of referred patients
				GetDlgItem(IDC_NUM_PATIENTS_REF_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NUM_PATIENTS_REF_BOX)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_REFERRED_PATS)->ShowWindow(SW_HIDE);

				// (m.hancock 2006-08-02 11:38) - PLID 20917 - Hide number of referred prospects
				GetDlgItem(IDC_NUM_PROSPECTS_REF_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NUM_PROSPECTS_REF_BOX)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_REFERRED_PROSPECTS)->ShowWindow(SW_HIDE);

				// (z.manning, 06/06/2007) - PLID 23862 - These buttons are for providers only.
				GetDlgItem(IDC_EDIT_BIOGRAPHY)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SELECT_IMAGE)->ShowWindow(SW_HIDE);

				// (z.manning, 12/12/2007) - PLID 28216 - Attendance setup button is for users only.
				GetDlgItem(IDC_ATTENDANCE_SETUP)->ShowWindow(SW_HIDE);

				//DRT 11/20/2008 - PLID 32082
				GetDlgItem(IDC_PROV_SPI)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_AMA_SPECIALTY_LIST)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_SPI_LABEL)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_AMA_SPECIALTY_LABEL)->ShowWindow(SW_HIDE);
				// (s.tullis 2015-10-29 17:13) - PLID 67483
				ShowCapitation(FALSE);

				// (b.savon 2013-06-03 11:24) - PLID 56867 - Hide if not a provider
				m_btnRegisterPrescriber.EnableWindow(FALSE);
				m_btnRegisterPrescriber.ShowWindow(SW_HIDE);

				// (j.gruber 2011-09-22 11:12) - PLID 45354 - affiliate phys
				GetDlgItem(IDC_AFFILIATE_PHYS)->ShowWindow(SW_HIDE);	

				// (b.spivey -- October 16th, 2013) - PLID 59022 - hide
				GetDlgItem(IDC_DIRECT_ADDRESS_USERS_BUTTON)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_DIRECT_ADDRESS_EDIT)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_DIRECT_ADDRESS_LABEL)->ShowWindow(SW_HIDE);

				GetDlgItem(IDC_DIRECT_ADDRESS_USERS_BUTTON)->EnableWindow(FALSE);
				GetDlgItem(IDC_DIRECT_ADDRESS_EDIT)->EnableWindow(FALSE);
				GetDlgItem(IDC_DIRECT_ADDRESS_LABEL)->EnableWindow(FALSE);
			}
		}



/*	For Contact Type Checkboxes - removed 1/16/00 - dt
		if (status & 0x1)
			m_refer.SetCheck(true);
		else m_refer.SetCheck(false);
		if (status & 0x2)
			m_main.SetCheck(true);
		else m_main.SetCheck(false);
		if (status & 0x4)
			m_employee.SetCheck(true);
		else m_employee.SetCheck(false);
*/


	/*	if (m_rs["Refer"].boolVal)
			m_refer.SetCheck(true);
		else m_refer.SetCheck(false);
		if (m_rs["Main"].boolVal)
			m_main.SetCheck(true);
		else m_main.SetCheck(false);
		if (m_rs["Employee"].boolVal)
			m_employee.SetCheck(true);
		else m_employee.SetCheck(false);*/
	} else {
		// There is not current record, so clear everything
		EnableWindow(FALSE);
		SetDlgItemText(IDC_EMPLOYER_BOX, "");
		SetDlgItemText(IDC_TITLE_BOX, "");
		SetDlgItemText(IDC_FIRST_NAME_BOX, "");
		SetDlgItemText(IDC_MIDDLE_NAME_BOX, "");
		SetDlgItemText(IDC_LAST_NAME_BOX, "");
		SetDlgItemText(IDC_ADDRESS1_BOX, "");
		SetDlgItemText(IDC_ADDRESS2_BOX, "");
		SetDlgItemText(IDC_CITY_BOX, "");
		SetDlgItemText(IDC_STATE_BOX, "");
		SetDlgItemText(IDC_SSN_BOX, "");
		SetDlgItemText(IDC_ZIP_BOX, "");
		SetDlgItemText(IDC_MARRIAGE_OTHER_BOX, "");
		SetDlgItemText(IDC_EMERGENCY_FIRST_NAME, "");
		SetDlgItemText(IDC_EMERGENCY_LAST_NAME, "");
		SetDlgItemText(IDC_CON_CUSTOM1_BOX, "");
		SetDlgItemText(IDC_CON_CUSTOM2_BOX, "");
		SetDlgItemText(IDC_CON_CUSTOM3_BOX, "");
		SetDlgItemText(IDC_CON_CUSTOM4_BOX, "");
		SetDlgItemText(IDC_HOME_PHONE_BOX, "");
		SetDlgItemText(IDC_WORK_PHONE_BOX, "");
		SetDlgItemText(IDC_EXT_PHONE_BOX, "");
		SetDlgItemText(IDC_CELL_PHONE_BOX, "");
		SetDlgItemText(IDC_PAGER_PHONE_BOX, "");
		SetDlgItemText(IDC_OTHER_PHONE_BOX, "");
		SetDlgItemText(IDC_FAX_BOX, "");
		SetDlgItemText(IDC_EMAIL_BOX, "");
		SetDlgItemText(IDC_NOTES, "");
		SetDlgItemText(IDC_CONTACT_TYPE, "");
		SetDlgItemText(IDC_RELATION_BOX, "");
		SetDlgItemText(IDC_OTHERHOME_BOX, "");
		SetDlgItemText(IDC_OTHERWORK_BOX, "");
		SetDlgItemText(IDC_NATIONALNUM_BOX, "");
		SetDlgItemText(IDC_ACCOUNT_BOX, "");
		SetDlgItemText(IDC_REFPHYSID_BOX, "");
		SetDlgItemText(IDC_UPIN_BOX, "");
		SetDlgItemText(IDC_FEDID_BOX, "");
		SetDlgItemText(IDC_MEDICARE_BOX, "");
		SetDlgItemText(IDC_BCBS_BOX, "");
		SetDlgItemText(IDC_NPI_BOX, "");
		// (j.jones 2013-06-10 13:26) - PLID 57089 - added Group NPI
		SetDlgItemText(IDC_GROUP_NPI_BOX, "");
		// (j.jones 2009-06-26 09:13) - PLID 34292 - added OHIP Specialty
		SetDlgItemText(IDC_OHIP_SPECIALTY, "");
		// (j.jones 2011-11-07 14:10) - PLID 46299 - added ProvidersT.UseCompanyOnClaims
		m_checkSendCompanyOnClaim.SetCheck(FALSE);
		SetDlgItemText(IDC_TAXONOMY_CODE, "");
		SetDlgItemText(IDC_MEDICAID_BOX, "");
		SetDlgItemText(IDC_WORKCOMP_BOX, "");
		SetDlgItemText(IDC_DEA_BOX, "");
		SetDlgItemText(IDC_LICENSE_BOX, "");
		SetDlgItemText(IDC_OTHERID_BOX, "");
		SetDlgItemText(IDC_METHOD_BOX, "");
		//TES 2/18/2008 - PLID 28954 - New field
		SetDlgItemText(IDC_ACCOUNT_NAME, "");
		SetDlgItemText(IDC_DEFAULT_COST_EDIT, "");
		// (m.hancock 2006-08-02 12:56) - PLID 20917 - Set number of referred patients and prospects to 0
		SetDlgItemText(IDC_NUM_PATIENTS_REF_BOX, "0");
		SetDlgItemText(IDC_NUM_PROSPECTS_REF_BOX, "0");
		//DRT 11/20/2008 - PLID 32082
		SetDlgItemText(IDC_PROV_SPI, "");

		SetDlgItemText(IDC_CAPITATION_DISTRIBUTION_EDIT, "");
		m_nxstaticCapitationDistribution.ShowWindow(SW_HIDE);
		m_pAMASpecialtyList->CurSel = NULL;

		m_PrefixCombo->SetSelByColumn(0,_variant_t());

		m_male.SetCheck(false);
		m_female.SetCheck(false);
		m_btnPatCoord.SetCheck(false);
		m_btnTechnician.SetCheck(false);

		// (j.gruber 2011-09-22 11:12) - PLID 45354 - affiliate phys
		SetDlgItemCheck(IDC_AFFILIATE_PHYS, 0);

		// (b.spivey -- October 16th, 2013) - PLID 59022 - set this to blank. 
		GetDlgItem(IDC_DIRECT_ADDRESS_EDIT)->SetWindowTextA(""); 

	}

	//DRT 5/2/03 - If they do not have permissions to sptWrite for users, ref phys, or phys, these
	//		boxes should all be disabled!
	SecureControls();

	}NxCatchAll("Error in ContactsGeneral::Load");

	m_changed = false;
}

void CContactsGeneral::SendContactsTablecheckerMsg() 
{
	if(m_strContactType == "Provider")
	{
		CClient::RefreshTable(NetUtils::Providers, m_id);
	}
	else if (m_strContactType == "User")
	{
		CClient::RefreshTable(NetUtils::Coordinators, m_id);
	}
	else if (m_strContactType == "Referring Physician")
	{
		CClient::RefreshTable(NetUtils::RefPhys, m_id);
	}
	else if (m_strContactType == "Supplier")
	{
		CClient::RefreshTable(NetUtils::Suppliers, m_id);
	}
	else if (m_strContactType == "Other")
	{
		//For example, a nurse or anesthesiologist.
		CClient::RefreshTable(NetUtils::ContactsT, m_id);
	}
	CClient::RefreshTable(NetUtils::CustomContacts, m_id);

}

void CContactsGeneral::Save(int nID)
{
	//(e.lally 2005-09-02) PLID 17387 - Added auditing for: CompanyID, Prefix, Title, 
		//Spouse, SSN, HomePhone, WorkPhone, Extension, EMail, CellPhone, Pager, OtherPhone, Fax, 
		//Note, Gender, and BirthDate.
	try {
		static CString field,sql,str="";
		static COleVariant var;
		_RecordsetPtr rs;
		CString strOld="";
		BOOL bRefPhysHL7FieldChosen = FALSE; // (v.maida 2014-12-23 12:19) - PLID 64472 - Flag that indicates whether a changed field is a field that is included in ref phys HL7 messages.
		
		int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();

		//If the field is synced with Outlook, we need to update it in Outlook.
		bool bUpdateOutlook = false;

		switch (nID)
		{	//fields in PersonT
			case IDC_TITLE_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT Title FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "Title","");
				field = "Title";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				// (c.haag 2003-11-24 11:12) - The toolbar is now updated because of the table
				// checker message
				//UpdateToolbar();
				}
				break;
			case IDC_FIRST_NAME_BOX:
			{
				strOld = VarString(GetMainFrame()->m_contactToolBar.m_toolBarCombo->GetValue(GetMainFrame()->m_contactToolBar.m_toolBarCombo->CurSel, 2));
				// (j.gruber 2007-08-07 11:41) - PLID 26976 - warn if they change a provider name
				if (nStatus & 0x2 /*provider*/) {
					CString strValue;
					GetDlgItemText(nID, strValue);
					if (strOld.CompareNoCase(strValue) != 0) {
						if (IDNO == MsgBox(MB_YESNO, "Changing the name of a provider can affect many places in Practice.\n"
							" Are you sure you want to do this?")) {
							//they want to cancel
							SetDlgItemText(nID, strOld);
							return;
						}
					}
				}
				// (j.gruber 2007-08-07 12:58) - PLID 26972 - warn if user's name changes, check if they are a lab assistant
				else if (nStatus & 0x04) {
					CString strValue;
					GetDlgItemText(nID, strValue);
					if (strOld.CompareNoCase(strValue) != 0) {
						//check if they are a lab assistant
						_RecordsetPtr rsCount = CreateParamRecordset("SELECT Count(*) as Count FROM LabsT where Deleted = 0 AND MedAssistant = {INT}", m_id);
						CString strMessage;
						CString strLabAsstMessage;
						if (!rsCount->eof) {

							long nLabsAsstCount = AdoFldLong(rsCount, "Count", -1);
							if (nLabsAsstCount > 0) {
								strLabAsstMessage.Format("\nincluding %li Lab(s) where this user is the lab assistant.", nLabsAsstCount);
							}
						}
						strMessage.Format("Changing the name of a user can affect many places in Practice%s\n"
							"Are you sure you want to do this?",
								strLabAsstMessage.IsEmpty() ? "." : strLabAsstMessage);

						if (IDNO == MsgBox(MB_YESNO, strMessage)) {
							//they want to cancel
							SetDlgItemText(nID, strOld);
							return;
						}
					}
				}
				field = "First";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.

				
				//UpdateToolbar();
			}
				break;
			case IDC_MIDDLE_NAME_BOX:
			{
				strOld = VarString(GetMainFrame()->m_contactToolBar.m_toolBarCombo->GetValue(GetMainFrame()->m_contactToolBar.m_toolBarCombo->CurSel, 3));
				field = "Middle";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				//UpdateToolbar();
			}
				break;
			case IDC_LAST_NAME_BOX:
			{
				strOld = VarString(GetMainFrame()->m_contactToolBar.m_toolBarCombo->GetValue(GetMainFrame()->m_contactToolBar.m_toolBarCombo->CurSel, 1));
				// (j.gruber 2007-08-07 11:41) - PLID 26976 - warn if they change a provider name
				if (nStatus & 0x2 /*provider*/) {
					CString strValue;
					GetDlgItemText(nID, strValue);
					if (strOld.CompareNoCase(strValue) != 0) {
						if (IDNO == MsgBox(MB_YESNO, "Changing the name of a provider can affect many places in Practice.\n"
							" Are you sure you want to do this?")) {
							//they want to cancel
							SetDlgItemText(nID, strOld);
							return;
						}
					}
				}
				// (j.gruber 2007-08-07 12:58) - PLID 26972 - warn if user's name changes, check if they are a lab assistant
				else if (nStatus & 0x04) {
					CString strValue;
					GetDlgItemText(nID, strValue);
					if (strOld.CompareNoCase(strValue) != 0) {
						//check if they are a lab assistant
						_RecordsetPtr rsCount = CreateParamRecordset("SELECT Count(*) as Count FROM LabsT where Deleted = 0 AND MedAssistant = {INT}", m_id);
						CString strMessage;
						CString strLabAsstMessage;
						if (!rsCount->eof) {

							long nLabsAsstCount = AdoFldLong(rsCount, "Count", -1);
							if (nLabsAsstCount > 0) {
								strLabAsstMessage.Format("\nincluding %li Lab(s) where this user is the lab assistant.", nLabsAsstCount);
							}
						}
						strMessage.Format("Changing the name of a user can affect many places in Practice%s\n"
							"Are you sure you want to do this?",
								strLabAsstMessage.IsEmpty() ? "." : strLabAsstMessage);

						if (IDNO == MsgBox(MB_YESNO, strMessage)) {
							//they want to cancel
							SetDlgItemText(nID, strOld);
							return;
						}
					}
				}

				field = "Last";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				//UpdateToolbar();
			}
				break;
			case IDC_EMPLOYER_BOX:
			{
				_RecordsetPtr rs = CreateRecordset("SELECT Company FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "Company","");
				rs->Close();
				field = "Company";
				bUpdateOutlook = true;
				//UpdateToolbar();
			}
				break;
			case IDC_ADDRESS1_BOX: {
				_RecordsetPtr rs = CreateRecordset("SELECT Address1 FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "Address1","");
				rs->Close();
				field = "Address1";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
			   }
				break;
			case IDC_ADDRESS2_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT Address2 FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "Address2","");
				rs->Close();
				field = "Address2";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				}
				break;
			case IDC_CITY_BOX:{				
					_RecordsetPtr rs = CreateRecordset("SELECT City FROM PersonT WHERE ID = %li",m_id);
					if(!rs->eof)
						strOld = AdoFldString(rs, "City","");
					rs->Close();
					field = "City";
					bUpdateOutlook = true;
					if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
					}				
				break;
			case IDC_STATE_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT State FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "State","");
				rs->Close();
				field = "State";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				}
				break;
			case IDC_ZIP_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT Zip FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "Zip","");
				rs->Close();
				field = "Zip";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				}
				break;

			case IDC_HOME_PHONE_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT HomePhone FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "HomePhone","");
				rs->Close();
				field = "HomePhone";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				}
				break;
			case IDC_WORK_PHONE_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT WorkPhone FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "WorkPhone","");
				rs->Close();
				field = "WorkPhone";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				}
				break;
			case IDC_EXT_PHONE_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT Extension FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "Extension","");
				rs->Close();
				field = "Extension";
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				}
				break;
			case IDC_CELL_PHONE_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT CellPhone FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "CellPhone","");
				rs->Close();
				field = "CellPhone";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				}
				break;
			case IDC_PAGER_PHONE_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT Pager FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "Pager","");
				rs->Close();
				field = "Pager";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				}
				break;
			case IDC_OTHER_PHONE_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT OtherPhone FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "OtherPhone","");
				rs->Close();
				field = "OtherPhone";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				}
				break;
			case IDC_FAX_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT Fax FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "Fax","");
				rs->Close();
				field = "Fax";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				}
				break;
			case IDC_EMAIL_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT Email FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "Email","");
				rs->Close();
				field = "Email";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				}
				break;
			case IDC_NOTES:{
				_RecordsetPtr rs = CreateRecordset("SELECT Note FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "Note","");
				rs->Close();
				field = "Note";
				bUpdateOutlook = true;
				}
				break;
			case IDC_BDATE_BOX:	
				{
					//Get the old birth date for auditing
					COleDateTime dtBirthDate;
					_RecordsetPtr rs = CreateRecordset("SELECT BirthDate FROM PersonT "
						"WHERE BirthDate IS NOT NULL AND ID = %li",m_id);
					if(!rs->eof){
						dtBirthDate = AdoFldDateTime(rs, "BirthDate");
						strOld = FormatDateTimeForInterface(dtBirthDate, dtoDate);
					}
					else{
						//There was no birth date before
						strOld ="";
					}
					rs->Close();
					if(!m_bSavingBirthDate) {
						CGuardLock guardBirthDate(m_bSavingBirthDate); // (a.walling 2011-08-29 12:20) - PLID 45232 - Use safe guard locks
						if (m_nxtBirthDate->GetStatus() == 3 ) {//Empty

							m_dtBirthDate.SetDateTime(1899,12,30,0,0,0);
							m_dtBirthDate.SetStatus(COleDateTime::invalid);
							m_nxtBirthDate->Clear();

							//Check to see if anything really changed. If not, we want to return
							if(strOld == ""){
								return;
							}
							ExecuteSql("UPDATE PersonT SET BirthDate = NULL WHERE ID = %li",m_id);

							//audit the change
							AuditContactChange(nID, nStatus, "", strOld);
						}
						else if(m_nxtBirthDate->GetStatus() == 2 || m_nxtBirthDate->GetDateTime() < COleDateTime(1800,1,1,0,0,0)) {//Invalid
							AfxMessageBox("You have entered an invalid date. The date will be reset.");
							_RecordsetPtr rs = CreateRecordset("SELECT BirthDate FROM PersonT WHERE ID = %li",m_id);
							if(rs->eof) {
								m_dtBirthDate.SetDateTime(1899,12,30,0,0,0);
								m_dtBirthDate.SetStatus(COleDateTime::invalid);
								m_nxtBirthDate->Clear();
							}
							else {
								Load();
							}	
							rs->Close();
						}
						else { //Valid
							COleDateTime dt, dtNow;
							dt = m_nxtBirthDate->GetDateTime();
							dtNow = COleDateTime::GetCurrentTime();
							if(dt > dtNow) {
								AfxMessageBox("You have entered a birthdate in the future. This will be adjusted to a valid date.");

								while(dt > dtNow)
									dt.SetDate(dt.GetYear() - 100, dt.GetMonth(), dt.GetDay());
							}
							CString bdate;
							bdate = FormatDateTimeForSql(dt, dtoDate);
							ExecuteSql("UPDATE PersonT SET BirthDate = '%s' WHERE ID = %li",bdate,m_id);
							m_dtBirthDate = dt;
							m_nxtBirthDate->SetDateTime(dt);

							str = FormatDateTimeForInterface(dt, dtoDate);
							//Check to see if anything really changed. If not, we want to return
							if(strOld == str){
								return;
							}

							//audit the change
							AuditContactChange(nID, nStatus, str, strOld);
						}

						// (v.maida 2014-12-23 12:19) - PLID 64472 - Try to add or update the referring physician in HL7, if the appropriate HL7 settings have been chosen.
						AddOrUpdateRefPhysInHL7(m_id, false);

						UpdatePalm();
						bUpdateOutlook = true;
					}
					
				}
				return;
			case IDC_SSN_BOX:{
				//(e.lally 2005-06-06) PLID 16365 - Make sure ssn's with value '###-##-####' get reset.
					//The OnKillFocus event was getting called after the save so the code to handle this
					//needs to be here.
				GetDlgItemText(IDC_SSN_BOX, str);
				if(str == "###-##-####") {
					str="";
					FormatItemText(GetDlgItem(IDC_SSN_BOX),str,"");
				}
				//Get the old value for auditing
				_RecordsetPtr rs = CreateRecordset("SELECT SocialSecurity FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof){
					strOld = AdoFldString(rs, "SocialSecurity","");
					//Currently blank SSN's are getting stored as 11 spaces
					//so we have to do a trim for our comparison to work properly later.
					strOld.TrimRight();
				}
				rs->Close();

				field = "SocialSecurity";
				bUpdateOutlook = true;
				if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; } // (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
				}
				break;
				

			case IDC_EMERGENCY_FIRST_NAME:
				field = "EmergFirst";
				break;
			case IDC_EMERGENCY_LAST_NAME:
				field = "EmergLast";
				break;
			case IDC_MARRIAGE_OTHER_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT Spouse FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "Spouse","");
				field = "Spouse";
				bUpdateOutlook = true;
				}
				break;
			case IDC_OTHERHOME_BOX:
				field = "EmergHPhone";
				break;
			case IDC_OTHERWORK_BOX:
				field = "EmergWPhone";
				break;
			case IDC_RELATION_BOX:
				field = "EmergRelation";
				break;
			case IDC_ACCOUNT_BOX:{
				_RecordsetPtr rs = CreateRecordset("SELECT CompanyID FROM PersonT WHERE ID = %li",m_id);
				if(!rs->eof)
					strOld = AdoFldString(rs, "CompanyID","");
				field = "CompanyID";
				bUpdateOutlook = true;
				}
				break;

			//TODO:  this needs fixed up and made better code, but it works for now
			//fields that are not in PersonT, we have to set these manually based on the selection
			case IDC_NATIONALNUM_BOX:
				GetDlgItemText(nID, str);
				ExecuteSql("UPDATE UsersT SET NationalEmplNumber = '%s' WHERE UsersT.PersonID = %li", _Q(str), m_id);
				return;

			case IDC_DATE_OF_HIRE:
				{
					if(!m_bSavingDateOfHire) {
						CGuardLock guardDateOfHire(m_bSavingDateOfHire); // (a.walling 2011-08-29 12:20) - PLID 45232 - Use safe guard locks
						if (m_nxtDateOfHire->GetStatus() == 3 ) {//Empty
							ExecuteSql("UPDATE UsersT SET DateOfHire = NULL WHERE PersonID = %li",m_id);
							m_dtDateOfHire.SetDateTime(1899,12,30,0,0,0);
							m_dtDateOfHire.SetStatus(COleDateTime::invalid);
							m_nxtDateOfHire->Clear();
						}
						else if(m_nxtDateOfHire->GetStatus() == 2 || m_nxtDateOfHire->GetDateTime() < COleDateTime(1800,1,1,0,0,0)) {//Invalid
							AfxMessageBox("You have entered an invalid date. The date will be reset.");
							_RecordsetPtr rs = CreateRecordset("SELECT DateOfHire FROM UsersT WHERE PersonID = %li",m_id);
							if(rs->eof) {
								m_dtDateOfHire.SetDateTime(1899,12,30,0,0,0);
								m_dtDateOfHire.SetStatus(COleDateTime::invalid);
								m_nxtDateOfHire->Clear();
							}
							else {
								Load();
							}	
							rs->Close();
						}
						else { //Valid
							COleDateTime dt, dtNow;
							dt = m_nxtDateOfHire->GetDateTime();
							dtNow = COleDateTime::GetCurrentTime();
							if(dt > dtNow) {
								AfxMessageBox("You have entered a 'Date Of Hire' in the future. Please ensure that this is correct.");
							}
							// (a.walling 2007-11-26 11:45) - PLID 28157
							if ( (dt != m_dtDateOfHire) && (m_dtDateOfTerm.GetStatus() == COleDateTime::valid) && (dt > m_dtDateOfTerm) ) {
								AfxMessageBox("You have entered a 'Date Of Hire' which is after the 'Date Of Termination.' Please ensure that this is correct.");
							}
							CString date;
							date = FormatDateTimeForSql(dt, dtoDate);
							ExecuteSql("UPDATE UsersT SET DateOfHire = '%s' WHERE PersonID = %li",date,m_id);
							m_dtDateOfHire = dt;
							m_nxtDateOfHire->SetDateTime(dt);
						}

						UpdatePalm();
					}
					
				}
				return;

			// (a.walling 2007-11-21 15:08) - PLID 28157
			case IDC_DATE_OF_TERM:
				{
					if(!m_bSavingDateOfTerm) {
						CGuardLock guardDateOfTerm(m_bSavingDateOfTerm); // (a.walling 2011-08-29 12:20) - PLID 45232 - Use safe guard locks
						if (m_nxtDateOfTerm->GetStatus() == 3 ) {//Empty
							ExecuteSql("UPDATE UsersT SET DateOfTerm = NULL WHERE PersonID = %li",m_id);
							m_dtDateOfTerm.SetDateTime(1899,12,30,0,0,0);
							m_dtDateOfTerm.SetStatus(COleDateTime::invalid);
							m_nxtDateOfTerm->Clear();
						}
						else if(m_nxtDateOfTerm->GetStatus() == 2 || m_nxtDateOfTerm->GetDateTime() < COleDateTime(1800,1,1,0,0,0)) {//Invalid
							AfxMessageBox("You have entered an invalid date. The date will be reset.");
							_RecordsetPtr rs = CreateRecordset("SELECT DateOfTerm FROM UsersT WHERE PersonID = %li",m_id);
							if(rs->eof) {
								m_dtDateOfTerm.SetDateTime(1899,12,30,0,0,0);
								m_dtDateOfTerm.SetStatus(COleDateTime::invalid);
								m_nxtDateOfTerm->Clear();
							}
							else {
								Load();
							}	
							rs->Close();
						}
						else { //Valid
							COleDateTime dt, dtNow;
							dt = m_nxtDateOfTerm->GetDateTime();
							if ( (dt != m_dtDateOfTerm) && (m_dtDateOfHire.GetStatus() == COleDateTime::valid) && (m_dtDateOfHire > dt) ) {
								AfxMessageBox("You have entered a 'Date Of Termination' which precedes the 'Date Of Hire.' Please ensure that this is correct.");
							}
							CString date;
							date = FormatDateTimeForSql(dt, dtoDate);
							ExecuteSql("UPDATE UsersT SET DateOfTerm = '%s' WHERE PersonID = %li",date,m_id);
							m_dtDateOfTerm = dt;
							m_nxtDateOfTerm->SetDateTime(dt);
						}

						UpdatePalm();
					}
					
				}
				return;

			case IDC_DEFAULT_COST_EDIT:
				strOld = "";
				GetDlgItemText(nID, str);
				if(nStatus == 0x0) { //other contact
					_RecordsetPtr rs = CreateRecordset("SELECT DefaultCost FROM ContactsT WHERE PersonID = %li", m_id);

					//get the old value
					COleCurrency cy = AdoFldCurrency(rs, "DefaultCost",COleCurrency(0,0));
					strOld = FormatCurrencyForInterface(cy,TRUE,TRUE);

					//now properly format the new value, though OnKillFocus should have already done this
					cy = ParseCurrencyFromInterface(str);
					str = FormatCurrencyForInterface(cy,TRUE,TRUE);

					ExecuteSql("UPDATE ContactsT SET DefaultCost = Convert(money,'%s') WHERE PersonID = %li", _Q(FormatCurrencyForSql(cy)), m_id);
				}
				else if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateRecordset("SELECT DefaultCost FROM ProvidersT WHERE PersonID = %li", m_id);

					//get the old value
					COleCurrency cy = AdoFldCurrency(rs, "DefaultCost",COleCurrency(0,0));
					strOld = FormatCurrencyForInterface(cy,TRUE,TRUE);

					//now properly format the new value, though OnKillFocus should have already done this
					cy = ParseCurrencyFromInterface(str);
					str = FormatCurrencyForInterface(cy,TRUE,TRUE);

					ExecuteSql("UPDATE ProvidersT SET DefaultCost = Convert(money,'%s') WHERE PersonID = %li", _Q(FormatCurrencyForSql(cy)), m_id);
				}
				else if (nStatus & 0x4) {	//user
					_RecordsetPtr rs = CreateRecordset("SELECT DefaultCost FROM UsersT WHERE PersonID = %li", m_id);

					//get the old value
					COleCurrency cy = AdoFldCurrency(rs, "DefaultCost",COleCurrency(0,0));
					strOld = FormatCurrencyForInterface(cy,TRUE,TRUE);

					//now properly format the new value, though OnKillFocus should have already done this
					cy = ParseCurrencyFromInterface(str);
					str = FormatCurrencyForInterface(cy,TRUE,TRUE);

					ExecuteSql("UPDATE UsersT SET DefaultCost = Convert(money,'%s') WHERE PersonID = %li", _Q(FormatCurrencyForSql(cy)), m_id);
				}

				AuditField(aeiDefaultCost, strOld, str);

				return;

			case IDC_REFPHYSID_BOX:
				GetDlgItemText(nID, str);
				ExecuteSql("UPDATE ReferringPhyST SET ReferringPhyID = '%s' WHERE ReferringPhyST.PersonID = %li", _Q(str), m_id);
				return;
			case IDC_UPIN_BOX:
				{
				strOld = "";
				GetDlgItemText(nID, str);
				if (nStatus & 0x1) {	//ref phys
					_RecordsetPtr rs = CreateRecordset("SELECT UPIN FROM ReferringPhysT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["UPIN"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["UPIN"]->Value.bstrVal);
					ExecuteSql("UPDATE ReferringPhyST SET UPIN = '%s' WHERE ReferringPhyST.PersonID = %li", _Q(str), m_id);
					// (v.maida 2014-12-23 12:19) - PLID 64472 - Try to add or update the referring physician in HL7, if the appropriate HL7 settings have been chosen.
					if (strOld != str) {
						AddOrUpdateRefPhysInHL7(m_id, false);
					}
				}
				else if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateRecordset("SELECT UPIN FROM ProvidersT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["UPIN"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["UPIN"]->Value.bstrVal);
					ExecuteSql("UPDATE ProvidersT SET UPIN = '%s' WHERE ProvidersT.PersonID = %li", _Q(str), m_id);
				}

				AuditField(aeiUPIN, strOld, str);
				}
				return;
			case IDC_FEDID_BOX:
				strOld = "";
				GetDlgItemText(nID, str);
				if (nStatus & 0x1) {	//ref phys
					_RecordsetPtr rs = CreateRecordset("SELECT FedEmployerID FROM ReferringPhysT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["FedEmployerID"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["FedEmployerID"]->Value.bstrVal);
					ExecuteSql("UPDATE ReferringPhyST SET FedEmployerID = '%s' WHERE ReferringPhyST.PersonID = %li", _Q(str), m_id);
				}
				else if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateRecordset("SELECT [Fed Employer ID] FROM ProvidersT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["Fed Employer ID"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["Fed Employer ID"]->Value.bstrVal);
					ExecuteSql("UPDATE ProvidersT SET [Fed Employer ID] = '%s' WHERE ProvidersT.PersonID = %li", _Q(str), m_id);
				}

				AuditField(aeiFedID, strOld, str);

				return;
			case IDC_MEDICARE_BOX:
				strOld = "";
				GetDlgItemText(nID, str);
				if (nStatus & 0x1) {	//ref phys
					_RecordsetPtr rs = CreateRecordset("SELECT MedicareNumber FROM ReferringPhysT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["MedicareNumber"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["MedicareNumber"]->Value.bstrVal);
					ExecuteSql("UPDATE ReferringPhyST SET MedicareNumber = '%s' WHERE ReferringPhyST.PersonID = %li", _Q(str), m_id);
					// (v.maida 2014-12-23 12:19) - PLID 64472 - Try to add or update the referring physician in HL7, if the appropriate HL7 settings have been chosen.
					if (strOld != str) {
						AddOrUpdateRefPhysInHL7(m_id, false);
					}
				}
				else if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateRecordset("SELECT [Medicare Number] FROM ProvidersT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["Medicare Number"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["Medicare Number"]->Value.bstrVal);
					ExecuteSql("UPDATE ProvidersT SET [Medicare Number] = '%s' WHERE ProvidersT.PersonID = %li", _Q(str), m_id);
				}

				AuditField(aeiMedicare, strOld, str);

				return;
			case IDC_BCBS_BOX:
				strOld = "";
				GetDlgItemText(nID, str);
				if (nStatus & 0x1) {	//ref phys
					_RecordsetPtr rs = CreateRecordset("SELECT BlueShieldID FROM ReferringPhysT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["BlueShieldID"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["BlueShieldID"]->Value.bstrVal);
					ExecuteSql("UPDATE ReferringPhyST SET BlueShieldID = '%s' WHERE ReferringPhyST.PersonID = %li", _Q(str), m_id);
				}
				else if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateRecordset("SELECT [BCBS Number] FROM ProvidersT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["BCBS Number"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["BCBS Number"]->Value.bstrVal);
					ExecuteSql("UPDATE ProvidersT SET [BCBS Number] = '%s' WHERE ProvidersT.PersonID = %li", _Q(str), m_id);
				}

				AuditField(aeiBCBS, strOld, str);

				return;
			case IDC_NPI_BOX:
				strOld = "";
				GetDlgItemText(nID, str);
				// (a.walling 2008-05-19 15:21) - PLID 27810 - Check for valid NPI
				CheckNPI(str, this);
				if (nStatus & 0x1) {	//ref phys
					_RecordsetPtr rs = CreateRecordset("SELECT NPI FROM ReferringPhysT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["NPI"]->Value.vt == VT_BSTR)
						strOld = CString(rs->Fields->Item["NPI"]->Value.bstrVal);
					ExecuteSql("UPDATE ReferringPhyST SET NPI = '%s' WHERE ReferringPhyST.PersonID = %li", _Q(str), m_id);
					// (v.maida 2014-12-23 12:19) - PLID 64472 - Try to add or update the referring physician in HL7, if the appropriate HL7 settings have been chosen.
					if (strOld != str) {
						AddOrUpdateRefPhysInHL7(m_id, false);
					}
				}
				else if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateRecordset("SELECT NPI FROM ProvidersT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["NPI"]->Value.vt == VT_BSTR)
						strOld = CString(rs->Fields->Item["NPI"]->Value.bstrVal);
					ExecuteSql("UPDATE ProvidersT SET NPI = '%s' WHERE ProvidersT.PersonID = %li", _Q(str), m_id);
				}

				AuditField(aeiNPI, strOld, str);
				return;

			// (j.jones 2013-06-10 13:26) - PLID 57089 - added Group NPI
			case IDC_GROUP_NPI_BOX:
				strOld = "";
				GetDlgItemText(nID, str);
				//validate the NPI
				CheckNPI(str, this);
				if (nStatus & 0x1) {	//ref phys
					_RecordsetPtr rs = CreateParamRecordset("SELECT GroupNPI FROM ReferringPhysT WHERE PersonID = {INT}", m_id);
					if(!rs->eof) {
						strOld = VarString(rs->Fields->Item["GroupNPI"]->Value);
					}
					rs->Close();
					ExecuteParamSql("UPDATE ReferringPhyST SET GroupNPI = {STRING} WHERE ReferringPhyST.PersonID = {INT}", str, m_id);
					
					AuditField(aeiGroupNPI, strOld, str);
				}
				return;
			
			case IDC_TAXONOMY_CODE:
				strOld = "";
				GetDlgItemText(nID, str);
				// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore.
				/*
				if (nStatus & 0x1) {	//ref phys
					_RecordsetPtr rs = CreateRecordset("SELECT TaxonomyCode FROM ReferringPhysT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["TaxonomyCode"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["TaxonomyCode"]->Value.bstrVal);
					ExecuteSql("UPDATE ReferringPhyST SET TaxonomyCode = '%s' WHERE ReferringPhyST.PersonID = %li", _Q(str), m_id);
				}
				else*/
				if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateRecordset("SELECT TaxonomyCode FROM ProvidersT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["TaxonomyCode"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["TaxonomyCode"]->Value.bstrVal);
					ExecuteSql("UPDATE ProvidersT SET TaxonomyCode = '%s' WHERE ProvidersT.PersonID = %li", _Q(str), m_id);
				}

				AuditField(aeiTaxonomy, strOld, str);

				return;
			// (j.jones 2009-06-26 09:13) - PLID 34292 - added OHIP Specialty
			case IDC_OHIP_SPECIALTY:
				strOld = "";
				GetDlgItemText(nID, str);
				if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateParamRecordset("SELECT OHIPSpecialty FROM ProvidersT WHERE PersonID = {INT}", m_id);
					if(rs->Fields->Item["OHIPSpecialty"]->Value.vt == VT_BSTR) {
						strOld = AdoFldString(rs, "OHIPSpecialty", "");
					}
					ExecuteParamSql("UPDATE ProvidersT SET OHIPSpecialty = {STRING} WHERE ProvidersT.PersonID = {INT}", str, m_id);
				}

				AuditField(aeiOHIPSpecialty, strOld, str);

				return;
			case IDC_WORKCOMP_BOX:
				strOld = "";
				GetDlgItemText(nID, str);
				if (nStatus & 0x1) {	//ref phys
					_RecordsetPtr rs = CreateRecordset("SELECT WorkersCompNumber FROM ReferringPhysT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["WorkersCompNumber"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["WorkersCompNumber"]->Value.bstrVal);
					ExecuteSql("UPDATE ReferringPhyST SET WorkersCompNumber = '%s' WHERE ReferringPhyST.PersonID = %li", _Q(str), m_id);
					// (v.maida 2014-12-23 12:19) - PLID 64472 - Try to add or update the referring physician in HL7, if the appropriate HL7 settings have been chosen.
					if (strOld != str) {
						AddOrUpdateRefPhysInHL7(m_id, false);
					}
				}
				else if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateRecordset("SELECT [Workers Comp Number] FROM ProvidersT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["Workers Comp Number"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["Workers Comp Number"]->Value.bstrVal);
					ExecuteSql("UPDATE ProvidersT SET [Workers Comp Number] = '%s' WHERE ProvidersT.PersonID = %li", _Q(str), m_id);
				}

				AuditField(aeiWorkComp, strOld, str);

				return;
			case IDC_MEDICAID_BOX:
				strOld = "";
				GetDlgItemText(nID, str);
				if (nStatus & 0x1) {	//ref phys
					_RecordsetPtr rs = CreateRecordset("SELECT MedicaidNumber FROM ReferringPhysT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["MedicaidNumber"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["MedicaidNumber"]->Value.bstrVal);
					ExecuteSql("UPDATE ReferringPhyST SET MedicaidNumber = '%s' WHERE ReferringPhyST.PersonID = %li", _Q(str), m_id);
					// (v.maida 2014-12-23 12:19) - PLID 64472 - Try to add or update the referring physician in HL7, if the appropriate HL7 settings have been chosen.
					if (strOld != str) {
						AddOrUpdateRefPhysInHL7(m_id, false);
					}
				}
				else if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateRecordset("SELECT [Medicaid Number] FROM ProvidersT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["Medicaid Number"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["Medicaid Number"]->Value.bstrVal);
					ExecuteSql("UPDATE ProvidersT SET [Medicaid Number] = '%s' WHERE ProvidersT.PersonID = %li", _Q(str), m_id);
				}

				AuditField(aeiMedicaid, strOld, str);

				return;
			case IDC_DEA_BOX:
				strOld = "";
				GetDlgItemText(nID, str);
				if (nStatus & 0x1) {	//ref phys
					_RecordsetPtr rs = CreateRecordset("SELECT DEANumber FROM ReferringPhysT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["DEANumber"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["DEANumber"]->Value.bstrVal);
					ExecuteSql("UPDATE ReferringPhyST SET DEANumber = '%s' WHERE ReferringPhyST.PersonID = %li", _Q(str), m_id);
					// (v.maida 2014-12-23 12:19) - PLID 64472 - Try to add or update the referring physician in HL7, if the appropriate HL7 settings have been chosen.
					if (strOld != str) {
						AddOrUpdateRefPhysInHL7(m_id, false);
					}
				}
				else if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateRecordset("SELECT [DEA Number] FROM ProvidersT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["DEA Number"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["DEA Number"]->Value.bstrVal);
					ExecuteSql("UPDATE ProvidersT SET [DEA Number] = '%s' WHERE ProvidersT.PersonID = %li", _Q(str), m_id);
				}

				AuditField(aeiDEA, strOld, str);

				return;
			case IDC_LICENSE_BOX:
				strOld = "";
				GetDlgItemText(nID, str);
				if (nStatus & 0x1) {	//ref phys
					_RecordsetPtr rs = CreateRecordset("SELECT License FROM ReferringPhysT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["License"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["License"]->Value.bstrVal);
					ExecuteSql("UPDATE ReferringPhyST SET License = '%s' WHERE ReferringPhyST.PersonID = %li", _Q(str), m_id);
					// (v.maida 2014-12-23 12:19) - PLID 64472 - Try to add or update the referring physician in HL7, if the appropriate HL7 settings have been chosen.
					if (strOld != str) {
						AddOrUpdateRefPhysInHL7(m_id, false);
					}
				}
				else if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateRecordset("SELECT License FROM ProvidersT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["License"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["License"]->Value.bstrVal);
					ExecuteSql("UPDATE ProvidersT SET License = '%s' WHERE ProvidersT.PersonID = %li", _Q(str), m_id);
				}

				AuditField(aeiLicense, strOld, str);

				return;
			case IDC_OTHERID_BOX:
				strOld = "";
				GetDlgItemText(nID, str);
				if (nStatus & 0x1) {	//ref phys
					_RecordsetPtr rs = CreateRecordset("SELECT OtherIDNUmber FROM ReferringPhysT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["OtherIDNumber"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["OtherIDNumber"]->Value.bstrVal);
					ExecuteSql("UPDATE ReferringPhyST SET OtherIDNumber = '%s' WHERE ReferringPhyST.PersonID = %li", _Q(str), m_id);
				}
				else if (nStatus & 0x2) {	//main phys
					_RecordsetPtr rs = CreateRecordset("SELECT [Other ID Number] FROM ProvidersT WHERE PersonID = %li", m_id);
					if(rs->Fields->Item["Other ID Number"]->Value.vt == VT_BSTR)	strOld = CString(rs->Fields->Item["Other ID Number"]->Value.bstrVal);
					ExecuteSql("UPDATE ProvidersT SET [Other ID Number] = '%s' WHERE ProvidersT.PersonID = %li", _Q(str), m_id);
				}

				AuditField(aeiOtherID, strOld, str);

				return;

			case IDC_PROV_SPI:
				{
					// (b.savon 2013-04-23 16:50) - PLID 56419 - only do this if the contact is a provider
					if (nStatus & 0x2) {	//main phys
						//DRT 11/20/2008 - PLID 32082
						strOld = "";
						GetDlgItemText(nID, str);
						//Get audit data & insert new and a single bound
						_RecordsetPtr prsAudit = CreateParamRecordset(
							"SELECT SPI FROM ProvidersT WHERE PersonID = {INT};\r\n"
							"SET NOCOUNT ON;\r\n"
							"UPDATE ProvidersT SET SPI = {STRING} WHERE PersonID = {INT};", m_id, str, m_id);
						if(prsAudit->eof) {
							//not possible
							AfxThrowNxException("Unable to audit previous SPI string after saving.");
						}
						strOld = AdoFldString(prsAudit, "SPI");
						AuditField(aeiSPI, strOld, str);
					}
				}
				return;
			// (s.tullis 2015-10-29 17:27) - PLID 67483 - Save and Audit
			case IDC_CAPITATION_DISTRIBUTION_EDIT:
			{
				if (nStatus & 0x2) {	
					strOld = "";
					GetDlgItemText(nID, str);
					//Get audit data & insert new and a single bound
					long nCapitDistrib = atoi(str);
					_RecordsetPtr prsAudit = CreateParamRecordset(R"(
						SELECT CapitationDistribution FROM ProvidersT WHERE PersonID = {INT};
						SET NOCOUNT ON;
						UPDATE ProvidersT SET CapitationDistribution = {INT} WHERE PersonID = {INT};
						)", m_id, nCapitDistrib , m_id);
					if (prsAudit->eof) {
						//not possible
						AfxThrowNxException("Unable to audit previous CapitationDistribution string after saving.");
					}
					strOld = FormatString("%li %%",AdoFldLong(prsAudit, "CapitationDistribution", 0));
					AuditField(aeiProviderCapitationDistribution, strOld, str + "  %");
				}
			}
			case IDC_METHOD_BOX:
				GetDlgItemText(nID, str);
				// (a.walling 2007-10-31 09:41) - PLID 27891 - FYI, the SupplierT.CCNumber is really a misnomer; it can (and should) be 'Method of Payment'
				ExecuteSql("UPDATE SupplierT SET CCNumber = '%s' WHERE SupplierT.PersonID = %li", _Q(str), m_id);
				return;

			case IDC_ACCOUNT_NAME:
				GetDlgItemText(nID, str);
				//TES 2/18/2008 - PLID 28954 - New field
				ExecuteSql("UPDATE SupplierT SET AccountName = '%s' WHERE SupplierT.PersonID = %li", _Q(str), m_id);
				return;
				break;
			
			//custom fields
			case IDC_CON_CUSTOM1_BOX:
				sql.Format("SELECT TextParam AS Val FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = %li",m_id,6);
				GetDlgItemText (nID, str);
				rs = CreateRecordset(adOpenDynamic, adLockOptimistic, sql);
				if(!rs->eof) {
					ExecuteSql("UPDATE CustomFieldDataT SET TextParam = '%s' WHERE (CustomFieldDataT.PersonID = %li) AND (CustomFieldDataT.FieldID = %li)", _Q(str), m_id, 6);
					rs->Close();
				}
				else {
					rs->Close();
					//check CustomFieldsT
					if(IsRecordsetEmpty("SELECT ID FROM CustomFieldsT WHERE ID = 6"))
						ExecuteSql("INSERT INTO CustomFieldsT (ID, Name, Type) VALUES (6, 'Custom 1', 1)");

					ExecuteSql("INSERT INTO CustomFieldDataT (PersonID, FieldID, TextParam) VALUES (%li,%li,'%s')",m_id,6,_Q(str));
				}
				return;
				break;
			case IDC_CON_CUSTOM2_BOX:
				sql.Format("SELECT TextParam AS Val FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = %li",m_id,7);
				GetDlgItemText (nID, str);
				rs = CreateRecordset(adOpenDynamic, adLockOptimistic, sql);
				if(!rs->eof) {
					ExecuteSql("UPDATE CustomFieldDataT SET TextParam = '%s' WHERE (CustomFieldDataT.PersonID = %li) AND (CustomFieldDataT.FieldID = %li)", _Q(str), m_id, 7);
					rs->Close();
				}
				else {
					rs->Close();
					//check CustomFieldsT
					if(IsRecordsetEmpty("SELECT ID FROM CustomFieldsT WHERE ID = 7"))
						ExecuteSql("INSERT INTO CustomFieldsT (ID, Name, Type) VALUES (7, 'Custom 2', 1)");

					ExecuteSql("INSERT INTO CustomFieldDataT (PersonID, FieldID, TextParam) VALUES (%li,%li,'%s')",m_id,7,_Q(str));
				}
				return;
				break;
			case IDC_CON_CUSTOM3_BOX:
				sql.Format("SELECT TextParam AS Val FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = %li",m_id,8);
				GetDlgItemText (nID, str);
				rs = CreateRecordset(adOpenDynamic, adLockOptimistic, sql);
				if(!rs->eof) {
					ExecuteSql("UPDATE CustomFieldDataT SET TextParam = '%s' WHERE (CustomFieldDataT.PersonID = %li) AND (CustomFieldDataT.FieldID = %li)", _Q(str), m_id, 8);
					rs->Close();
				}
				else {
					rs->Close();
					//check CustomFieldsT
					if(IsRecordsetEmpty("SELECT ID FROM CustomFieldsT WHERE ID = 8"))
						ExecuteSql("INSERT INTO CustomFieldsT (ID, Name, Type) VALUES (8, 'Custom 3', 1)");

					ExecuteSql("INSERT INTO CustomFieldDataT (PersonID, FieldID, TextParam) VALUES (%li,%li,'%s')",m_id,8,_Q(str));
				}
				return;
				break;
			case IDC_CON_CUSTOM4_BOX:
				sql.Format("SELECT TextParam AS Val FROM CustomFieldDataT WHERE PersonID = %li AND FieldID = %li",m_id,9);
				GetDlgItemText (nID, str);
				rs = CreateRecordset(adOpenDynamic, adLockOptimistic, sql);
				if(!rs->eof) {
					ExecuteSql("UPDATE CustomFieldDataT SET TextParam = '%s' WHERE (CustomFieldDataT.PersonID = %li) AND (CustomFieldDataT.FieldID = %li)", _Q(str), m_id, 9);
					rs->Close();
				}
				else {
					rs->Close();
					//check CustomFieldsT
					if(IsRecordsetEmpty("SELECT ID FROM CustomFieldsT WHERE ID = 9"))
						ExecuteSql("INSERT INTO CustomFieldsT (ID, Name, Type) VALUES (9, 'Custom 4', 1)");

					ExecuteSql("INSERT INTO CustomFieldDataT (PersonID, FieldID, TextParam) VALUES (%li,%li,'%s')",m_id,9,_Q(str));
				}
				return;
				break;

			// (b.spivey -- October 16th, 2013) - PLID 59022 - save this on killfocus. 
			case IDC_DIRECT_ADDRESS_EDIT:
				str = "";
				GetDlgItemText(nID, str);
				if(ReturnsRecordsParam("SELECT TOP 1 * FROM DirectAddressFromT WHERE PersonID = {INT} ", m_id)) {
					if(str.Trim().GetLength() > 0) {
						ExecuteParamSql("UPDATE DirectAddressFromT SET DirectAddress = {STRING} WHERE PersonID = {INT} ", str, m_id);
					}
					else
					{
						// (b.spivey - December 16th, 2013) - PLID 59022 - Make sure we're cleaning up after ourselves. 
						int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();
						bool bWarn = false;
						// (b.spivey - December 16th, 2013) - PLID 59022 - If we're a provider and we have users linked, 
						//	 then we warn about this. 
						if(nStatus & 0x2 && ReturnsRecordsParam("SELECT TOP 1 * FROM DirectAddressUserT DAUT "
												"INNER JOIN DirectAddressFromT DAFT ON DAUT.DirectAddressFromID = DAFT.ID "
												"WHERE DAFT.PersonID = {INT} ", m_id)) {
							bWarn = true; 
						}

						// (b.spivey - December 16th, 2013) - PLID 59022 - The warning is only for providers with 
						//	 linked users. If we're not to warn, it'll short circuit the message box. 
						if(bWarn && MessageBox("This provider has users who have permission to use their direct address. "
							"If you remove this provider's direct address, those users will become unlinked. "
							"Are you sure you wish to continue?", "Warning", MB_YESNO) == IDYES) {
							
							ExecuteParamSql("DECLARE @PersonID INT " 
							"DECLARE @DirectAddressID INT " 
							"	"
							"SET @PersonID = {INT} "
							"SET @DirectAddressID = (SELECT ID FROM DirectAddressFromT WHERE PersonID = @PersonID) "
							"	"
							"DELETE FROM DirectAddressUserT WHERE DirectAddressFromID = @DirectAddressID " 
							"DELETE FROM DirectAddressFromT WHERE PersonID = @PersonID ", m_id);
						}
						//unorthodox to be sure, if they did not hit yes then the hit no, and that's where this case comes
						//	in, because we need to put a value back into the edit control. 
						else if(bWarn) {
							_RecordsetPtr prs = CreateParamRecordset("SELECT DirectAddress "
								"FROM DirectAddressFromT WHERE PersonID = {INT} ", m_id);
							if (!prs->eof) {
								GetDlgItem(IDC_DIRECT_ADDRESS_EDIT)->SetWindowTextA(
									AdoFldString(prs->Fields, "DirectAddress", "")); 
							}
							else {
								//This should be impossible. We wouldn't be here if there wasn't something to select 
								//from the database. So somehow this got deleted before we were attempting to delete it
								//or it got deleted regardless of our cancelling of the delete. 
								ASSERT(FALSE);
							}
						}
						//If we shouldn't warn, then we should be free to delete from the main list willy-nilly. 
						else if(!bWarn) {
							ExecuteParamSql("DELETE FROM DirectAddressFromT WHERE PersonID = {INT} ", m_id);
						}
					}
				}
				//If there was no entry in DirectAddressFromT, we shall make one. 
				else {
					ExecuteParamSql("INSERT DirectAddressFromT (DirectAddress, PersonID) VALUES ({STRING}, {INT}) ", str, m_id); 
				}
				return;
				break;

			default:
				return;
		}

		//GetDlgItemVar(nID, var, vt);
		GetDlgItemText(nID, str);

		//Check to see if anything really changed. If not, we want to return
		if(strOld == str) return;


		//DRT 8/6/2007 - PLID 26970 - If this person is used as a nurse or anesthesiologist, warn the user first.  These can only come
		//	when the person is an "other" type.
		if(nStatus == 0) {
			switch(nID) {
				case IDC_FIRST_NAME_BOX:
				case IDC_MIDDLE_NAME_BOX:
				case IDC_LAST_NAME_BOX:
					{
						_RecordsetPtr prsCount = CreateParamRecordset("SELECT SUM(CASE WHEN NurseID = {INT} THEN 1 ELSE 0 END) AS NurseCnt, "
							"SUM(CASE WHEN AnesthesiologistID = {INT} THEN 1 ELSE 0 END) AS AnesCnt FROM ProcInfoT", m_id, m_id);
						if(!prsCount->eof) {
							long nNurse = AdoFldLong(prsCount, "NurseCnt", 0);
							long nAnes = AdoFldLong(prsCount, "AnesCnt", 0);

							if(nNurse != 0 || nAnes != 0) {
								CString strMsg;
								strMsg.Format("Renaming this person will affect the display on the PIC for:\r\n"
									" - %li nurse records.\r\n"
									" - %li anesthesiologist records.\r\n"
									"Are you sure you wish to rename this person?", nNurse, nAnes);
								if(MsgBox(MB_YESNO, strMsg) != IDYES) {
									//Undo by setting back to the "old" text
									SetDlgItemText(nID, strOld);
									return;
								}
							}
						}

					}
					break;
			}
		}

		// (a.walling 2010-05-17 13:08) - PLID 34056 - Check the history folder first
		bool bHistoryOK = true;
		if(nID == IDC_FIRST_NAME_BOX)
			bHistoryOK = EnsureCorrectHistoryFolder(this, eChangedHFFirst, str, m_id, true);
		else if(nID == IDC_LAST_NAME_BOX)
			bHistoryOK = EnsureCorrectHistoryFolder(this, eChangedHFLast, str, m_id, true);

		if (!bHistoryOK) {
			//Undo by setting back to the "old" text
			SetDlgItemText(nID, strOld);
			return;
		}


		ExecuteSql("UPDATE PersonT SET %s = '%s' WHERE PersonT.ID = %li", field, _Q(str), m_id);

		//audit the change
		AuditContactChange(nID, nStatus, str, strOld);

		// update the palm
		UpdatePalm();

		//Update Outlook, if needed.
		if(bUpdateOutlook) {
			PPCModifyContact(m_id);
		}

		// (v.maida 2014-12-23 12:19) - PLID 64472 - Add ref phys HL7 update, if applicable.
		if ( (nStatus & 0x1) && bRefPhysHL7FieldChosen) {
			AddOrUpdateRefPhysInHL7(m_id, false);
		}

	}NxCatchAll("Error saving field: ");
}

void CContactsGeneral::RecallDetails() 
{
	Load();
}

void CContactsGeneral::StoreDetails() 
{
	//We can't trust m_changed for NxTimes.
	// (a.walling 2008-10-06 13:16) - PLID 31595 - ASSERTions when casting an invalid COleDateTime to a DATE
	if(m_nxtBirthDate->GetDateTime() != m_dtBirthDate.m_dt) {
		Save(IDC_BDATE_BOX);
	}
	if(m_nxtDateOfHire->GetDateTime() != m_dtDateOfHire.m_dt) {
		Save(IDC_DATE_OF_HIRE);
	}
	// (a.walling 2007-11-21 15:10) - PLID 28157
	if(m_nxtDateOfTerm->GetDateTime() != m_dtDateOfTerm.m_dt) {
		Save(IDC_DATE_OF_TERM);
	}
	if (m_changed) {
		int nID = GetFocus()->GetDlgCtrlID();
		if (nID)
			Save(nID);
		m_changed = false;
	}
}

BOOL CContactsGeneral::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID;
	CString str;
	// (v.maida 2014-12-23 12:19) - PLID 64472 - Add ref phys HL7 update, if applicable.
	BOOL bRefPhysHL7FieldChosen = FALSE;

	try {

	switch(HIWORD(wParam))
	{	case EN_CHANGE:
			switch (nID = LOWORD(wParam))
			{
				case IDC_MIDDLE_NAME_BOX:
					// (c.haag 2006-08-02 11:40) - PLID 21740 - We now check for auto-capitalization
					// for middle name boxes
					if (GetRemotePropertyInt("AutoCapitalizeMiddleInitials", 1, 0, "<None>", true)) {
						Capitalize(nID);
					}
					break;
				case IDC_TITLE_BOX: 
				case IDC_FIRST_NAME_BOX: 
				case IDC_LAST_NAME_BOX: 
				case IDC_ADDRESS1_BOX: 
				case IDC_ADDRESS2_BOX: 
				case IDC_CITY_BOX: 
				case IDC_STATE_BOX: 
				case IDC_MARRIAGE_OTHER_BOX: 
				case IDC_EMERGENCY_FIRST_NAME: 
				case IDC_EMERGENCY_LAST_NAME:
				case IDC_EMPLOYER_BOX: 
					Capitalize(nID);
					break;
				case IDC_ZIP_BOX:
					// (d.moore 2007-04-23 12:11) - PLID 23118 - 
					//  Capitalize letters in the zip code as they are typed in. Canadian postal
					//    codes need to be formatted this way.
					CapitalizeAll(IDC_ZIP_BOX);
					GetDlgItemText(nID, str);
					str.TrimRight();
					//if (str != "")
					//	FormatItem (nID, "#####-nnnn");
					break;
				case IDC_HOME_PHONE_BOX: 
				case IDC_WORK_PHONE_BOX: 
				case IDC_CELL_PHONE_BOX: 
				case IDC_PAGER_PHONE_BOX: 
				case IDC_OTHER_PHONE_BOX: 
				case IDC_FAX_BOX:
				case IDC_OTHERHOME_BOX:
				case IDC_OTHERWORK_BOX:
					GetDlgItemText(nID, str);
					str.TrimRight();
					if (str != "") {
						if(m_bFormatPhoneNums) {
							FormatItem (nID, m_strPhoneFormat);
						}
					}
					break;	
				case IDC_EXT_PHONE_BOX: 
					//I'm taking this out as part of a broader allow-letters-in-extensions campaign.
					//Also, why didn't you just mark this as "Number" in the resources?  Sigh.
					/*
					GetDlgItemText(nID, str);
					str.TrimRight();
					if (str != "")
						FormatItem (nID, "nnnnnnn");*/
					break;
				case IDC_SSN_BOX:
					GetDlgItemText(nID, str);
					str.TrimRight();
					if (str != "")
						FormatItem (nID, "###-##-####");
					else
						//clear out empty strings
						FormatItem (nID, "");
					break;
				case IDC_EMAIL_BOX:
					UpdateEmailButton();
					break;
				case IDC_DIRECT_ADDRESS_EDIT:
					UpdateDirectAddressButton(); 
					break;
				default:
					break;
			}
			m_changed = true;
			break;
		case EN_KILLFOCUS:
			{
			//DRT 8/6/2007 - Needed for PLID 26970
			if(m_bIsSaving)
				break;
			
			CGuardLock guardSaving(m_bIsSaving); // (a.walling 2011-08-29 12:20) - PLID 45232 - Use safe guard locks

			int nID = LOWORD(wParam);

			switch (nID) {
				case IDC_ZIP_BOX:
					{
						// (j.gruber 2009-10-07 16:12) - PLID 35825 - save normally if not using the preference
						if (!m_bLookupByCity) {
							CString city, 
							state,
							tempCity,
							tempState,
							strTempZip,
							value;

							GetDlgItemText(IDC_ZIP_BOX, value);
							GetDlgItemText(IDC_CITY_BOX, tempCity);
							GetDlgItemText(IDC_STATE_BOX, tempState);
							tempCity.TrimRight();
							tempState.TrimRight();
							// (d.thompson 2009-08-24) - PLID 31136 - Prompt to see if they wish to overwrite city/state
							if(!tempCity.IsEmpty() || !tempState.IsEmpty()) {
								MAINTAIN_FOCUS(); // (a.walling 2011-08-26 09:56) - PLID 45199 - Safely maintain focus
								if(AfxMessageBox("You have changed the postal code but the city or state already have data in them.  Would you like to overwrite "
									"this data with that of the new postal code?", MB_YESNO) == IDYES)
								{
									//Just treat them as empty and the code below will fill them.
									tempCity.Empty();
									tempState.Empty();
								}
							}
							if(tempCity == "" || tempState == "") {
								GetZipInfo(value, &city, &state);
								// (s.tullis 2013-10-21 10:14) - PLID 45031 - If 9-digit zipcode match fails compair it with the 5-digit zipcode.
								if(city == "" && state == ""){
									strTempZip = value.Left(5);// Get the 5 digit zip code
									GetZipInfo(strTempZip, &city, &state);
									// (b.savon 2014-04-03 13:02) - PLID 61644 - If you enter a 9
									//digit zipcode in the locations tab of Administrator, it looks
									//up the city and state based off the 5 digit code, and then 
									//changes the zip code to 5 digits. It should not change the zip code.
								}
								if(tempCity == "") 
									SetDlgItemText(IDC_CITY_BOX, city);
								else city = tempCity;
								if(tempState == "")
									SetDlgItemText(IDC_STATE_BOX, state);
								else state = tempState;

								int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();

								CString strOldCity, strOldState, strOldZip;

								_RecordsetPtr rs = CreateRecordset("SELECT City, State, Zip FROM PersonT WHERE ID = %li",m_id);
								if(!rs->eof) {
									strOldCity = AdoFldString(rs, "City","");
									strOldState = AdoFldString(rs, "State","");
									strOldZip = AdoFldString(rs, "Zip","");
								}
								rs->Close();

								if(city != strOldCity) 
									AuditContactChange(IDC_CITY_BOX,nStatus,city,strOldCity);
								if(state != strOldState) 
									AuditContactChange(IDC_STATE_BOX,nStatus,state,strOldState);
								if(value != strOldZip){ 
									AuditContactChange(IDC_ZIP_BOX,nStatus,value,strOldZip);
									ExecuteSql("UPDATE PersonT SET Zip = '%s', City = '%s', State = '%s' WHERE ID = %li",_Q(value),_Q(city),_Q(state),m_id);
									if (((city != strOldCity) || (state != strOldState) || (value != strOldZip)) && (nStatus & 0x1)) { // (v.maida 2014-12-23 12:19) - PLID 64472 - If this is a referring physician and at least one of the values has changed, then set update HL7 flag.
										bRefPhysHL7FieldChosen = TRUE;
									}

								}
							}
							else {

								int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();

								CString strOld;

								_RecordsetPtr rs = CreateRecordset("SELECT Zip FROM PersonT WHERE ID = %li",m_id);
								if(!rs->eof)
									strOld = AdoFldString(rs, "Zip","");
								rs->Close();

								if (value != strOld) {
									AuditContactChange(IDC_ZIP_BOX, nStatus, value, strOld);
									// (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
									if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; }
								}

								ExecuteSql("UPDATE PersonT SET Zip = '%s' WHERE ID = %li",_Q(value),m_id);
							}
						}
						else {
							Save(LOWORD(wParam));
						}				

				}
				break;

				case IDC_CITY_BOX:
					{
						if (m_bLookupByCity) {
							CString zip, 
							state,
							tempZip,
							tempState,
							value;
							GetDlgItemText(IDC_CITY_BOX, value);
							GetDlgItemText(IDC_ZIP_BOX, tempZip);
							GetDlgItemText(IDC_STATE_BOX, tempState);
							tempZip.TrimRight();
							tempState.TrimRight();
							// (d.thompson 2009-08-24) - PLID 31136 - Prompt to see if they wish to overwrite city/state
							if(!tempZip.IsEmpty() || !tempState.IsEmpty()) {
								MAINTAIN_FOCUS(); // (a.walling 2011-08-26 09:56) - PLID 45199 - Safely maintain focus
								if(AfxMessageBox("You have changed the city but the post code or state already have data in them.  Would you like to overwrite "
									"this data with that of the new city?", MB_YESNO) == IDYES)
								{
									//Just treat them as empty and the code below will fill them.
									tempZip.Empty();
									tempState.Empty();
								}
							}
							if(tempZip == "" || tempState == "") {
								GetCityInfo(value, &zip, &state);
								if(tempZip == "") 
									SetDlgItemText(IDC_ZIP_BOX, zip);
								else zip = tempZip;
								if(tempState == "")
									SetDlgItemText(IDC_STATE_BOX, state);
								else state = tempState;

								int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();

								CString strOldCity, strOldState, strOldZip;

								_RecordsetPtr rs = CreateRecordset("SELECT City, State, Zip FROM PersonT WHERE ID = %li",m_id);
								if(!rs->eof) {
									strOldCity = AdoFldString(rs, "City","");
									strOldState = AdoFldString(rs, "State","");
									strOldZip = AdoFldString(rs, "Zip","");
								}
								rs->Close();

								if(value != strOldCity) 
									AuditContactChange(IDC_CITY_BOX,nStatus,value,strOldCity);
								if(state != strOldState) 
									AuditContactChange(IDC_STATE_BOX,nStatus,state,strOldState);
								if(zip != strOldZip) 
									AuditContactChange(IDC_ZIP_BOX,nStatus,zip,strOldZip);


								ExecuteSql("UPDATE PersonT SET Zip = '%s', City = '%s', State = '%s' WHERE ID = %li",_Q(zip),_Q(value),_Q(state),m_id);
								if ( ( (value != strOldCity) || (state != strOldState) || (zip != strOldZip)) && (nStatus & 0x1)) { // (v.maida 2014-12-23 12:19) - PLID 64472 - If this is a referring physician and at least one of the values has changed, then set update HL7 flag.
									bRefPhysHL7FieldChosen = TRUE; 
								}
							}
							else {

								int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();

								CString strOld;

								_RecordsetPtr rs = CreateRecordset("SELECT City FROM PersonT WHERE ID = %li",m_id);
								if(!rs->eof)
									strOld = AdoFldString(rs, "City","");
								rs->Close();

								if (value != strOld)  {
									AuditContactChange(IDC_CITY_BOX, nStatus, value, strOld);
									// (v.maida 2014-12-23 12:19) - PLID 64472 - "nStatus & 0x1" indicates that a referring physician is being edited.
									if (nStatus & 0x1) { bRefPhysHL7FieldChosen = TRUE; }
								}

								ExecuteSql("UPDATE PersonT SET City = '%s' WHERE ID = %li",_Q(value),m_id);
							}
						}
						else {
							Save(LOWORD(wParam));
						}				
				}
				break;

				case IDC_NOTES:
					if(m_changed){
					//TES 2/9/04: This is now ntext.
					//if(ForceFieldLength((CNxEdit*)GetDlgItem(IDC_NOTES))){
						Save(LOWORD(wParam));
					//}
					}
				break;

				case IDC_DEFAULT_COST_EDIT: {
					if (m_changed) {

						COleCurrency cy;
						CString str;
						GetDlgItemText(nID, str);

						cy = ParseCurrencyFromInterface(str);

						BOOL bFailed = FALSE;
						if(cy.GetStatus()==COleCurrency::invalid) {
							// (r.farnworth 2015-03-16 09:33) - PLID 63291 - No longer refered to as Default Cost Per Hour
							AfxMessageBox("An invalid currency was entered as the Cost Per Hour.\n"
								"Please correct this.");
							bFailed = TRUE;
						}

						if(!bFailed && cy < COleCurrency(0,0)) {
							// (r.farnworth 2015-03-16 09:33) - PLID 63291 - No longer refered to as Default Cost Per Hour
							AfxMessageBox("Practice does not allow a negative amount for a Cost Per Hour.\n"
								"Please correct this.");
							bFailed = TRUE;
						}

						if(!bFailed && cy > COleCurrency(100000000,0)) {
							CString str;
							str.Format("Practice does not allow an amount greater than %s.",FormatCurrencyForInterface(COleCurrency(100000000,0),TRUE,TRUE));
							AfxMessageBox(str);
							bFailed = TRUE;
						}
						
						if(!bFailed) {
							SetDlgItemText(IDC_DEFAULT_COST_EDIT, FormatCurrencyForInterface(cy,TRUE,TRUE));
							Save (LOWORD(wParam));
						}
						else {
							//reload the existing data

							int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();

							COleCurrency cy = COleCurrency(0,0);

							if(nStatus == 0x0) { //other contact
								_RecordsetPtr rs = CreateRecordset("SELECT DefaultCost FROM ContactsT WHERE PersonID = %li",m_id);
								if(!rs->eof) {
									cy = AdoFldCurrency(rs, "DefaultCost",COleCurrency(0,0));
								}
								rs->Close();
							}
							else if (nStatus & 0x2) {	//main phys
								_RecordsetPtr rs = CreateRecordset("SELECT DefaultCost FROM ProvidersT WHERE PersonID = %li",m_id);
								if(!rs->eof) {
									cy = AdoFldCurrency(rs, "DefaultCost",COleCurrency(0,0));
								}
								rs->Close();
							}
							else if (nStatus & 0x4) {	//user
								_RecordsetPtr rs = CreateRecordset("SELECT DefaultCost FROM UsersT WHERE PersonID = %li",m_id);
								if(!rs->eof) {
									cy = AdoFldCurrency(rs, "DefaultCost",COleCurrency(0,0));
								}
								rs->Close();
							}

							SetDlgItemText(IDC_DEFAULT_COST_EDIT, FormatCurrencyForInterface(cy,TRUE,TRUE));
						}
					}
				}

				case IDC_HOME_PHONE_BOX: 
				case IDC_WORK_PHONE_BOX: 
				case IDC_CELL_PHONE_BOX: 
				case IDC_PAGER_PHONE_BOX: 
				case IDC_OTHER_PHONE_BOX: 
				case IDC_FAX_BOX:
				case IDC_OTHERHOME_BOX:
				case IDC_OTHERWORK_BOX:
					if (SaveAreaCode(nID) && m_changed) {
						Save(LOWORD(wParam));
					}
				break;

				default:
					if (m_changed) {
						Save (LOWORD(wParam));
					}
				break;
			}
			m_changed = false;
			}
		break;

		case EN_SETFOCUS:
			{

			int nID = LOWORD(wParam);
			switch (nID) {
				case IDC_HOME_PHONE_BOX: 
				case IDC_WORK_PHONE_BOX: 
				case IDC_CELL_PHONE_BOX: 
				case IDC_PAGER_PHONE_BOX: 
				case IDC_OTHER_PHONE_BOX: 
				case IDC_FAX_BOX:
				case IDC_OTHERHOME_BOX:
				case IDC_OTHERWORK_BOX:
					if (ShowAreaCode()) {
						FillAreaCode(nID);
					}
				break;

				default:
				break;
			}
			}
		break;
		
		default:
		break;
	}

	// (v.maida 2014-12-23 12:19) - PLID 64472 - Add ref phys HL7 update, if applicable.
	int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();
	if ((nStatus & 0x1) && bRefPhysHL7FieldChosen) {
		AddOrUpdateRefPhysInHL7(m_id, false);
	}

	}NxCatchAll("Error in OnCommand");
	return CNxDialog::OnCommand(wParam, lParam);
}

void CContactsGeneral::OnChangePrefix(long iNewRow) 
{
	// (a.walling 2008-10-10 12:54) - PLID 29688 - Moved to OnSelChosen
}

void CContactsGeneral::OnChangePalm(long iNewRow) 
{
	if (iNewRow < 0 || iNewRow > 2)
	{
		iNewRow = 2;
		m_PalmCombo->CurSel = 2;
		return;
	}
	try{
		ExecuteSql("UPDATE PersonT SET PalmFlag = %d WHERE PersonT.ID = %d", iNewRow, m_id);
		SendContactsTablecheckerMsg();
	}NxCatchAll("Error in changing palm status");
}


void CContactsGeneral::OnMale() 
{
	//(e.lally 2005-09-02) PLID 17387 - Add auditing for gender changes (amongst others)
	try {
		CString strOld ="", str ="";
		int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();
		_RecordsetPtr rs = CreateRecordset("SELECT Gender FROM PersonT WHERE ID = %li",m_id);
		if(!rs->eof){
			BYTE usGender = AdoFldByte(rs, "Gender");
			if(usGender == 0) strOld="";
			else if(usGender == 2) strOld="Female";
			//If for some reason they click on the same gender again, we can just return;
			else return;
		}

		ExecuteSql("UPDATE PersonT SET Gender = 1 WHERE PersonT.ID = %li", m_id);

		AuditContactChange(IDC_MALE, nStatus, "Male", strOld);

		PPCModifyContact(m_id);

		//check the preference first
		if(GetRemotePropertyInt("GenderPrefixLink", 1, 0, "<None>", true) == 1) {
			long nCurrentPrefixGender = m_PrefixCombo->CurSel == -1 ? -1 : VarLong(m_PrefixCombo->GetValue(m_PrefixCombo->CurSel, 2), -1);
			if(nCurrentPrefixGender != 0) {
				//if prefix is not male
				if(nCurrentPrefixGender != 1) {					
					long nNewPrefix = GetRemotePropertyInt("DefaultMalePrefix", 1, 0, "<None>", true);
					rs = CreateRecordset("SELECT PrefixT.Prefix FROM PrefixT, PersonT "
						"WHERE PersonT.PrefixID = PrefixT.ID AND PersonT.ID = %li", m_id);
					if(!rs->eof) {
						strOld = AdoFldString(rs, "Prefix");
						m_PrefixCombo->SetSelByColumn(0, nNewPrefix);
						//(e.lally 2011-07-15) PLID 42529 - Check for "none" as the new prefix
						if(nNewPrefix != -1){
							ExecuteSql("UPDATE PersonT SET PrefixID = %li WHERE ID = %li", nNewPrefix, m_id);
							//get the string value of the new prefix
							str = VarString(m_PrefixCombo->GetValue(m_PrefixCombo->GetCurSel(), 1));
						}
						else {
							ExecuteParamSql("UPDATE PersonT SET PrefixID = NULL WHERE ID = {INT} ", m_id);
							str = "";
						}
						//Audit the change
						if(str != strOld){
							AuditContactChange(IDC_PREFIX_LIST, nStatus, str, strOld);
						}
					}
					rs->Close();
				}
			}
		}
		
		// (v.maida 2014-12-23 12:19) - PLID 64472 - Add ref phys HL7 update, if applicable. If we've made it to this point, then the gender has at least changed, possibly the prefix as well.
		if (nStatus & 0x1) {
			AddOrUpdateRefPhysInHL7(m_id, false);
		}

	}NxCatchAll("Error in changing gender");
}

void CContactsGeneral::OnFemale() 
{
	//(e.lally 2005-09-02) PLID 17387 - Add auditing for gender changes (amongst others)
	try {
		CString strOld ="", str ="";
		int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();
		_RecordsetPtr rs = CreateRecordset("SELECT Gender FROM PersonT WHERE ID = %li",m_id);
		if(!rs->eof){
			BYTE usGender = AdoFldByte(rs, "Gender");
			if(usGender == 0) strOld="";
			else if(usGender == 1) strOld="Male";
			//If for some reason they click on the same gender again, we can just return;
			else return;
		}
		ExecuteSql("UPDATE PersonT SET Gender = 2 WHERE PersonT.ID = %li", m_id);

		AuditContactChange(IDC_FEMALE, nStatus, "Female", strOld);

		PPCModifyContact(m_id);

		//check the preference first
		if(GetRemotePropertyInt("GenderPrefixLink", 1, 0, "<None>", true) == 1) {
			long nCurrentPrefixGender = m_PrefixCombo->CurSel == -1 ? -1 : VarLong(m_PrefixCombo->GetValue(m_PrefixCombo->CurSel, 2), -1);
			if(nCurrentPrefixGender != 0) {
				//if prefix is not female
				if(nCurrentPrefixGender != 2) {					
					//female
					long nNewPrefix = GetRemotePropertyInt("DefaultFemalePrefix", 1, 0, "<None>", true);
					rs = CreateRecordset("SELECT PrefixT.Prefix FROM PrefixT, PersonT "
						"WHERE PersonT.PrefixID = PrefixT.ID AND PersonT.ID = %li", m_id);
					if(!rs->eof) {
						strOld = AdoFldString(rs, "Prefix");
						m_PrefixCombo->SetSelByColumn(0, nNewPrefix);
						//(e.lally 2011-07-15) PLID 42529 - Check for "none" as the new prefix
						if(nNewPrefix != -1){
							ExecuteSql("UPDATE PersonT SET PrefixID = %li WHERE ID = %li", nNewPrefix, m_id);
							//get the string value of the new prefix
							str = VarString(m_PrefixCombo->GetValue(m_PrefixCombo->GetCurSel(), 1));
						}
						else {
							ExecuteParamSql("UPDATE PersonT SET PrefixID = NULL WHERE ID = {INT} ", m_id);
							str = "";
						}
						
						//Audit the change
						if(str != strOld){
							AuditContactChange(IDC_PREFIX_LIST, nStatus, str, strOld);
						}
					}
				}
			}
		}

		// (v.maida 2014-12-23 12:19) - PLID 64472 - Add ref phys HL7 update, if applicable. If we've made it to this point, then the gender has at least changed, possibly the prefix as well.
		if (nStatus & 0x1) {
			AddOrUpdateRefPhysInHL7(m_id, false);
		}

	}NxCatchAll("Error in changing gender");
}

void CContactsGeneral::UpdatePalm()
{
	long nPalmID = m_id;
	SetPalmRecordStatus("PalmContactsInfoT", nPalmID, 1);
}

void CContactsGeneral::OnClickPatcoordCheck() 
{
	int type;

	if(m_btnPatCoord.GetCheck())
		type = 1;
	else
		type = 0;

	try
	{

		if(type == 0) {
			//removing patient coord status.  if they have any data, it can be lost... we need to warn them
			if(ReturnsRecords("SELECT PersonID FROM PatientsT WHERE EmployeeID = %li", m_id)) {
				CSelectUserDlg dlg(this);
				dlg.m_nExcludeUser = m_id;
				dlg.m_strCaption = "This user (as a patient coordinator) has patients assigned to them.  Please select a new patient coordinator for these "
					"patients to be re-assigned to.  If {None} is selected, then the patient coordinator will be deleted for these patients.";
				dlg.m_strUserWhereClause = "UsersT.PatientCoordinator = 1";

				if(IDOK == dlg.DoModal()) {
					if(dlg.m_nSelectedUser != -1) {
						// (m.cable 6/21/2004 10:02) - they chose a different coordinator, change the existing
						// patient info
						ExecuteSql("UPDATE PatientsT SET EmployeeID = %li WHERE EmployeeID = %li", dlg.m_nSelectedUser, m_id);
					}
					else {
						// (m.cable 6/21/2004 10:13) - the chose none, delete the pat coord field for patients with this coordinator
						ExecuteSql("UPDATE PatientsT SET EmployeeID = NULL WHERE EmployeeID = %li", m_id);
					}
				}
				else {
					// (m.cable 6/21/2004 10:14) - they canceled the dlg, they must not want to change the status
					m_btnPatCoord.SetCheck(true);
					return;
				}
			}
		}

		ExecuteSql("UPDATE UsersT SET PatientCoordinator = %i WHERE UsersT.PersonID = %li", type, m_id);
		CClient::RefreshTable(NetUtils::Coordinators, m_id);

		//audit this, it's pretty important
		long nID = BeginNewAuditEvent();
		CString strNew, strName, str;
		strNew.Format("%li", type);
		
		//format the name
		GetDlgItemText(IDC_LAST_NAME_BOX, str);
		strName += str;
		strName += ", ";
		GetDlgItemText(IDC_FIRST_NAME_BOX, str);
		strName += str;
		strName += " ";
		GetDlgItemText(IDC_MIDDLE_NAME_BOX, str);
		strName += str;

		AuditEvent(-1, strName, nID, aeiPatCoordStatus, m_id, (type == 0 ? "1" : "0"), strNew, aepMedium, aetChanged);
		SendContactsTablecheckerMsg();
	} NxCatchAll("Error in CContactsGeneral::OnClickPatcoordCheck()");

}

// (d.lange 2011-03-22 12:58) - PLID 42943 - Save and audit the change in status of the 'Assistant/Technician' checkbox
void CContactsGeneral::OnClickTechnicianCheck()
{
	long nType;
	if(m_btnTechnician.GetCheck()) {
		nType = 1;
	}else {
		nType = 0;
	}

	try {
		ExecuteParamSql("UPDATE UsersT SET Technician = {INT} WHERE PersonID = {INT}", nType, m_id);

		if(nType == 0) {
			if(ReturnsRecords("SELECT TOP 1 PersonID FROM EmrTechniciansT INNER JOIN EMRMasterT "
							"ON EmrTechniciansT.EmrID = EMRMasterT.ID WHERE EmrTechniciansT.Deleted = 0 AND EMRMasterT.Deleted = 0 AND "
							"EmrTechniciansT.PersonID = %li", m_id)) {
				AfxMessageBox("This user is associated with at least one EMN as an Assistant/Technician.\r\n\r\nBy unselecting this "
						"user as an Assistant/Technician, they will no longer be available for selection on future EMNs.");
			}
		}

		// (d.lange 2011-04-01 09:30) - PLID 42987 - Make sure we update g_bIsCurrentUserTechnician if the current logged in user
		// has been modified
		if(GetCurrentUserID() == m_id) {
			SetCurrentUserTechnicianStatus(m_btnTechnician.GetCheck());
		}

		//We need to follow suit and audit this as well
		long nID = BeginNewAuditEvent();
		CString strNew, strName, str;
		strNew.Format("%li", nType);

		GetDlgItemText(IDC_LAST_NAME_BOX, str);
		strName += str;
		strName += ", ";
		GetDlgItemText(IDC_FIRST_NAME_BOX, str);
		strName += str;
		strName += " ";
		GetDlgItemText(IDC_MIDDLE_NAME_BOX, str);
		strName += str;

		AuditEvent(-1, strName, nID, aeiTechnicianStatus, m_id, (nType == 0 ? "1" : "0"), strNew, aepMedium, aetChanged);
		SendContactsTablecheckerMsg();
	} NxCatchAll("Error in CContactsGeneral::OnClickTechnicianCheck()");
}

void CContactsGeneral::OnPermissionsBtn() 
{
	CUserPermissionsDlg dlg(this);
	CUserGroupSecurityDlg dlgNew(this);

	//DRT - 4/3/03 - This is now determined by the 'Edit Permissions' permission
	// (j.jones 2009-05-26 15:47) - PLID 34315 - split into bioEditOwnPermissions and bioEditOtherUserPermissions
	if(m_id == GetCurrentUserID()) {
		if(!CheckCurrentUserPermissions(bioEditOwnPermissions, sptWrite)) {
			return;
		}
	}
	else {
		if(!CheckCurrentUserPermissions(bioEditOtherUserPermissions, sptWrite)) {
			return;
		}
	}

	try{
		BOOL bShowNewPerms = TRUE;

#ifdef _DEBUG
		// If we're in debug mode, and we want to see the old permissions,
		// do it if the control key is down.
		if (GetAsyncKeyState(VK_CONTROL) & 0xE000)
		{
			bShowNewPerms = FALSE;
		}
#endif

		if (bShowNewPerms)
		{
			int result = dlgNew.Open(m_id, FALSE);
			if(result == IDOK) {
				//If a user is changing their own permissions, some really wierd things can happen sometimes
				//ask them to restart to make sure everything is reloaded correctly
				if(m_id == GetCurrentUserID()) {
					AfxMessageBox("You have modified your own permissions.  Please restart Practice to ensure these all take full effect.");
				}
				UpdateView();
			}
		}
		else
		{
			//set the user id to the PersonID of this contact
			dlg.m_id = m_id;

			if(dlg.DoModal() == IDOK)
			{
				HANDLE hNewUser = LogInUser(GetCurrentUserName(), GetCurrentUserPassword(), GetCurrentLocationID(), GetInactivityTimeout(), GetCurrentLocationName());
				if (hNewUser == NULL) {
					// Failure to open the new user
					AfxThrowNxException("Could not load new permissions for this user.  Please restart Practice.");
				}
			}			
		}
		GetDlgItem(IDC_EMPLOYER_BOX)->SetFocus();
	} NxCatchAll("CContactsGeneral::OnPermissionsBtn");
}

void CContactsGeneral::OnPropertiesBtn() 
{
	//DRT 4/3/03 - This no longers uses the bioContactsUsers permission, it now uses
	//		one of 2 things:  bioEditSelfPassword or bioEditAllPassword.  AllPassword has
	//		higher precedence.

	CPermissions perms;
	GetPermissions(GetCurrentUserHandle(), bioEditAllPassword, FALSE, 0, perms);

	if(perms.nPermissions & sptWrite || perms.nPermissions & sptWriteWithPass) {
		//we have permission - call this function to prompt for password if it's needed
		if(!CheckCurrentUserPermissions(bioEditAllPassword, sptWrite))
			return;
	}
	else {
		//they do not have permissions to edit all passwords
		if(m_id == GetCurrentUserID()) {
			//we are looking at our own username - this will either give the pword box
			//or it will give the "you don't have permission box)
			if(!CheckCurrentUserPermissions(bioEditSelfPassword, sptWrite))
				return;
		}
		else {
			//we are looking at someone else, but we already determined we do not have permission to
			//all users
			// (a.walling 2010-08-02 11:01) - PLID 39182 - Consolidating all these copies of "You do not have permission to access this function"
			// messageboxes with PermissionsFailedMessageBox
			PermissionsFailedMessageBox();
			return;
		}
	}

	// Also, you're not allowed to edit the built-in users
	if (!(m_id > 0)) {
		AfxMessageBox("You may not edit the properties of this user because it is a built-in user.  Please select a different user.");
	}

	CUserPropsDlg dlg(this);
	_RecordsetPtr rs;
	FieldsPtr	rsFields;

	CString sql,name;
	try {
		EnsureRemoteData();

		// (j.jones 2008-11-20 08:56) - PLID 23227 - added PWExpireNextLogin
		//TES 4/30/2009 - PLID 28573 - Added FailedLogins
		// (r.farnworth 2015-12-30 15:31) - PLID 67719 - Field is now Passwordhash
		rs = CreateParamRecordset("SELECT PersonID, UserName, PasswordHash, SavePassword, AutoLogoff, AutoLogoffDuration, "
			"Administrator, PasswordExpires, PasswordExpireDays, PWExpireNextLogin, FailedLogins FROM UsersT WHERE PersonID = {INT}", m_id);

		if (!rs->eof)
		{
			rsFields = rs->Fields;
			bool bOriginalPwExpires = AdoFldBool(rsFields->Item["PasswordExpires"]) ? true : false;
			long nOriginalPwDays = AdoFldLong(rsFields->Item["PasswordExpireDays"]);			
			dlg.m_name = AdoFldString(rsFields->Item["UserName"]);
			// (j.jones 2009-04-30 14:19) - PLID 33853 - used AES encryption on the password
			// (r.farnworth 2015-12-30 15:31) - PLID 67719 - Field is now Passwordhash
			dlg.m_pass = AdoFldString(rsFields->Item["PasswordHash"]);
			dlg.m_bMemory = AdoFldBool(rsFields->Item["SavePassword"]) ? true : false;
			dlg.m_bInactivity = AdoFldBool(rsFields->Item["AutoLogoff"]) ? true : false;
			dlg.m_nInactivityMinutes = AdoFldLong(rsFields->Item["AutoLogoffDuration"]);
			dlg.m_bAdministrator = AdoFldBool(rsFields->Item["Administrator"]) ? true : false;
			dlg.m_bExpires = bOriginalPwExpires;
			dlg.m_nPwExpireDays = nOriginalPwDays;
			// (j.jones 2008-11-20 08:56) - PLID 23227 - added PWExpireNextLogin
			dlg.m_bPasswordExpiresNextLogin = VarBool(rsFields->Item["PWExpireNextLogin"]->Value, FALSE);
			//TES 4/30/2009 - PLID 28573 - Added FailedLogins
			dlg.m_nFailedLogins = AdoFldLong(rsFields, "FailedLogins");
			dlg.m_nUserID = m_id;
			rs->Close();
			//for auditing
			CString strOldname, strOldpass;
			strOldname = dlg.m_name;
			strOldpass = dlg.m_pass;
			BOOL bOldPasswordExpiresNextLogin = dlg.m_bPasswordExpiresNextLogin;
			long nOldAdminStatus = dlg.m_bAdministrator;
			//
			if (IDOK == dlg.DoModal())
			{
				try{

					// (j.jones 2008-11-19 10:16) - PLID 28578 - track if the password changed
					BOOL bPasswordChanged = FALSE;
					if(dlg.m_pass != strOldpass) {
						bPasswordChanged = TRUE;
					}

					// (r.farnworth 2015-12-29 11:12) - PLID 67719 - Change Practice to call our new ChangeContactInformation API method instead of calling SQL from C++ code.
					NexTech_Accessor::_UserPropertiesPtr pUserProperties(__uuidof(NexTech_Accessor::UserProperties));
					NexTech_Accessor::_ChangePasswordPtr pChangePassword(__uuidof(NexTech_Accessor::ChangePassword));
					NexTech_Accessor::_UserPtr pUser(__uuidof(NexTech_Accessor::User));

					CString strUserID, strLocID;
					strUserID.Format("%li",m_id);
					strLocID.Format("%li", GetCurrentLocationID());
					pUser->PutID(AsBstr(strUserID));
					pUser->Putusername(AsBstr(dlg.m_name));
					pUser->PutlocationID(AsBstr(strLocID));

					pChangePassword->User = pUser;
					pChangePassword->PutCurrentPassword(AsBstr(strOldpass));
					pChangePassword->PutNewPassword(AsBstr(dlg.m_pass));
					
					pUserProperties->ChangePassword = pChangePassword;
					pUserProperties->PutReason(NexTech_Accessor::UserPropertyChangeReason::UserPropertyChangeReason_UpdateUser);
					pUserProperties->PutSavePassword((VARIANT_BOOL)dlg.m_bMemory);
					pUserProperties->PutAutoLogoff((VARIANT_BOOL)dlg.m_bInactivity);
					pUserProperties->PutAutoLogoffDuration(dlg.m_nInactivityMinutes);
					pUserProperties->PutAdministrator((VARIANT_BOOL)dlg.m_bAdministrator);
					pUserProperties->PutPasswordExpireNextLogin((VARIANT_BOOL)(dlg.m_bPasswordExpiresNextLogin ? 1 : 0));

					pUserProperties->PutResetPivot((VARIANT_BOOL)(bOriginalPwExpires != dlg.m_bExpires || dlg.m_nPwExpireDays != nOriginalPwDays));
					pUserProperties->PutPasswordExpires((VARIANT_BOOL)dlg.m_bExpires);
					pUserProperties->PutPasswordExpireDays(dlg.m_nPwExpireDays);
					COleDateTime dtNow = COleDateTime::GetCurrentTime();
					dtNow.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());
					pUserProperties->PutPasswordPivotDate(dtNow);
					pUserProperties->PutAllLocations((VARIANT_BOOL)dlg.m_bAllLocations);
					pUserProperties->PutproviderID(dlg.m_nNewProviderID);

					GetAPI()->UpdateUserProperties(GetAPISubkey(), GetAPILoginToken(), pUserProperties);

					if (GetCurrentUserID() == m_id)
					{
						//DRT 5/19/2006 - PLID 20721 - This should ONLY be called if the user changes their own password, 
						//	not anyone's password!
						//DRT 7/23/03 - Set the global password for the program - so it knows that it changed
						SetCurrentUserPassword(dlg.m_pass);

						// (j.jones 2008-11-19 10:16) - PLID 28578 - added SetCurrentUserPasswordVerified(),
						//which is always set to TRUE if the password changed
						if(bPasswordChanged) {
							SetCurrentUserPasswordVerified(TRUE);
						}

						if (dlg.m_bInactivity)
							SetInactivityTimeout(dlg.m_nInactivityMinutes);
						else
							SetInactivityTimeout(-1);

						if((dlg.m_bAdministrator?1:0) != nOldAdminStatus) {
							AfxMessageBox("By changing your Administrator status, you have modified your own permissions.  Please restart Practice to ensure these all take full effect.");

							//DRT 5/19/2006 - PLID 20693 - We now cache the administrator status, if this is the current user.
							if(nOldAdminStatus == 0 && dlg.m_bAdministrator) {
								//The user was not an administrator, and now they are, update our global cache
								SetCurrentUserAdministratorStatus(TRUE);
							}
							else if(nOldAdminStatus != 0 && !dlg.m_bAdministrator) {
								//The user was an administrator, and now they are not, update our cache
								SetCurrentUserAdministratorStatus(FALSE);
							}
						}
					}
					Load();

					//set the username in the toolbar
					if(strOldname != dlg.m_name) {
						GetMainFrame()->m_contactToolBar.m_toolBarCombo->PutValue(GetMainFrame()->m_contactToolBar.m_toolBarCombo->CurSel, 10, _bstr_t("< " + dlg.m_name + " >"));
					}

					//TES 2003-12-29: Tell everyone that a user has changed (using the deceptively misnamed Coordinators enum).
					CClient::RefreshTable(NetUtils::Coordinators, m_id);

					// (r.farnworth 2015-12-30 15:17) - PLID 67719 - Auditing moved to API.

				// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
				} NxCatchAll("Error updating user properties");
			}
			GetDlgItem(IDC_EMPLOYER_BOX)->SetFocus();
			return;
		}
		rs->Close();

	}NxCatchAll("Could not load user properties");
	
}

BOOL CContactsGeneral::PreTranslateMessage(MSG* pMsg) 
{
	switch(pMsg->message)
	{
	case WM_LBUTTONDBLCLK:
		ChangeLabel(::GetDlgCtrlID(pMsg->hwnd));
		return TRUE;
		break;
	default:
		break;
	}

	return CNxDialog::PreTranslateMessage(pMsg);
}

//returns true if label is changed, false otherwise
BOOL CContactsGeneral::ChangeLabel(const int nID)
{
	int field = GetFieldID(nID);

	if(field == 0)	//didn't click on a changeable label
		return false;
	
	if (!UserPermission(CustomLabel))
		return false;

	CString strResult, strPrompt;
	GetDlgItemText(nID, strPrompt);
	strResult = ConvertFromControlText(strPrompt);

	_variant_t var;
	int nResult;
	
	do {
		nResult = InputBoxLimited(this, "Enter new name for " + strPrompt, strResult, "",50,false,false,NULL);
		
		if (nResult == IDCANCEL) // if they hit cancel, immediately return
			return false;

		strResult.TrimRight();
		strResult.TrimLeft();

		if (strResult == "")
			AfxMessageBox("Please type in a new name for this custom label or press cancel");

		else if(strResult.GetLength() > 50)
			AfxMessageBox("The label name you entered is too long.");
	
	} while ((strResult.GetLength() > 50) | (strResult == ""));

	try {
		// m.carlson:Feb. 2, 2004	PLID #10737
		if(!IsRecordsetEmpty("SELECT * FROM CustomFieldsT WHERE ID = %d", field)) {
			// Make the data change
			ExecuteSql("UPDATE CustomFieldsT SET Name = '%s' WHERE ID = %li", _Q(strResult), field);
		} else {
			// The record wasn't found so insert it into the database
			ExecuteSql("INSERT INTO CustomFieldsT (ID,Name,Type) VALUES (%li,'%s',1)",field,_Q(strResult));
		}
		// (b.cardillo 2006-05-19 17:28) - PLID 20735 - We know the new name for this label, so 
		// update the global cache so it won't have to be reloaded in its entirety.
		SetCustomFieldNameCachedValue(field, strResult);
		// (b.cardillo 2006-07-06 16:27) - PLID 20737 - Notify all the other users as well.
		CClient::RefreshTable_CustomFieldName(field, strResult);

		// (z.manning, 08/13/2007) - PLID 26256 - The clip children propery of this dialog is on to prevent
		// drawing issues when chaning between contact types. However, it causes problems when renaming custom
		// field labels. So, temporarily turn it off while we rename that static control.
		ModifyStyle(WS_CLIPCHILDREN, 0);
		SetDlgItemText(nID, ConvertToControlText(strResult));
		Invalidate();
		ModifyStyle(0, WS_CLIPCHILDREN);

		// (j.jones 2007-08-02 08:38) - PLID 26866 - removed this functionality, deemed too dangerous
		/*
		if (!IsRecordsetEmpty("SELECT FieldID FROM CustomFieldDataT WHERE FieldID = %li",field) &&
			IDYES == AfxMessageBox("Do you want to clear the existing data for ALL contacts? Selecting \"No\" will only rename the label and continue to use the existing data.", MB_YESNO))
		{
			CString str, strSQL;
			strSQL.Format("SELECT PersonID FROM CustomFieldDataT WHERE FieldID = '%d'", field);
			long nRecordCount = GetRecordCount(strSQL);
			if (nRecordCount > 1)
			{
				str.Format("There are %d records of legacy data you will be removing information from by doing this!!! ARE YOU ABSOLUTELY SURE YOU WISH TO ERASE ALL OF THE EXISTING DATA FOR THIS FIELD?",
					nRecordCount);
			}
			else
			{
				str = "There is 1 record of legacy data you will be removing information from by doing this!!! ARE YOU ABSOLUTELY SURE YOU WISH TO ERASE ALL OF THE EXISTING DATA FOR THIS FIELD?";
			}
			if (IDYES == AfxMessageBox(str, MB_YESNO))
			{	
				ExecuteSql("DELETE FROM CustomFieldDataT WHERE FieldID = %li",field);
				UpdateView();
				// update the palm record
				UpdatePalm();
			}
		}
		*/

	}NxCatchAll("Error in changing custom field. CContactsGeneral::ChangeLabel");

	m_labelChecker.Refresh();

	//success!
	return true;
}

int CContactsGeneral::GetFieldID(int nID)
{
	int field = 0;

	switch(nID)
	{
	case IDC_CUSTOM1_LABEL:
		field = 6;
		break;
	case IDC_CUSTOM2_LABEL:
		field = 7;
		break;
	case IDC_CUSTOM3_LABEL:
		field = 8;
		break;
	case IDC_CUSTOM4_LABEL:
		field = 9;
		break;
	default:
		break;
	}

	return field;
}

void CContactsGeneral::OnClickEmail()
{
	CString str;
	GetDlgItemText(IDC_EMAIL_BOX, str);
	str.TrimRight();
	str.TrimLeft();
	if (str != "") {

		//DRT 9/22/03 - Added the prompt for a subject.  Once that is entered, we can put a record
		//		in the history that an email was sent. Note that we have no control over any of this, 
		//		so we're only opening the default email client with the given subject.  If they choose
		//		to cancel later, we don't know.
		CString strSubject;
		// (j.jones 2006-09-18 12:47) - PLID 22545 - limit the subject text to 255 characters, which is Outlook's maximum
		// and thus, by the commonality of said number, we can presume is the maximum in other email clients
		if(InputBoxLimited(this, "Enter your email subject:", strSubject, "", 255, false, false, "Cancel") == IDOK) {
			//DRT 9/22/03 - Follow the preference from general1.  We could save a different preference, but I don't
			//see any value in separating them, I don't see where someone would want to see one and not the other.
			//1 = always save in history, 2 = save if strSubject is not blank, 0 = never save
			long nSave = GetRemotePropertyInt("Gen1SaveEmails", 2, 0, "<None>", false);

			// (a.walling 2009-08-10 16:17) - PLID 35153 - Use NULL instead of "open" when calling ShellExecute(Ex). Usually equivalent; see PL notes.
			if ((int)ShellExecute(GetSafeHwnd(), NULL, "mailto:" + str + "?Subject=" + strSubject, 
				NULL, "", SW_MAXIMIZE) < 32) {
				AfxMessageBox("Could not e-mail patient");
			}
			else {
				if(nSave == 1 || (nSave == 2 && !strSubject.IsEmpty())) {
					try {
						//email started successfully
						//enter a note in the history tab
						CString strNote;
						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						strNote.Format("Email sent to '%s' on %s.  Subject:  '%s'", str, FormatDateTimeForInterface(dtNow, DTF_STRIP_SECONDS, dtoDateTime, false), strSubject);
	
						// (j.jones 2008-09-04 14:31) - PLID 30288 - converted to use CreateNewMailSentEntry,
						// which creates the data in one batch and sends a tablechecker
						// (c.haag 2010-01-28 11:00) - PLID 37086 - Removed COleDateTime::GetCurrentTime as the service date; should be the server's time.
						// Obviously this will contradict strNote if the server/workstation times are different, but if a new MailSent date is to be "now", then it
						// needs to be the server's "now". If the note becomes an issue, we can address it in a future item.
						CreateNewMailSentEntry(GetActiveContactID(), strNote, "", "", GetCurrentUserName(), "", GetCurrentLocationID());
						
					} NxCatchAll("Error saving email to history");
				}
				else {
					//nSave is 0, we do not want to save this in the history
				}
			}
		}
	}
	else
		AfxMessageBox("Please enter an e-mail address.");
	GetDlgItem(IDC_EMAIL_BOX)->SetFocus();
}


/*void CContactsGeneral::OnPalmCheck() 
{
	try{
		int check = ((CComboBox*)GetDlgItem(IDC_COMBO_PALMSYNC))->GetCurSel();			
		ExecuteSql("UPDATE PersonT SET PalmFlag = %li WHERE PersonT.ID = %li", check, m_id);
	} NxCatchAll("Error in CContactsGeneral::OnPalmCheck()");

	UpdatePalm();
	
}*/

void CContactsGeneral::UpdateToolbar()
{
	int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();

	CString last, middle, first, company, title;
	GetDlgItemText(IDC_MIDDLE_NAME_BOX, middle);
	GetDlgItemText(IDC_LAST_NAME_BOX, last);
	GetDlgItemText(IDC_FIRST_NAME_BOX, first);
	GetDlgItemText(IDC_EMPLOYER_BOX, company);
	GetDlgItemText(IDC_TITLE_BOX, title);

	CString name = last + ", " + first + ' ' + middle + " " + title + "  - ";
	//DRT 3/11/03 - Toolbar now shows the Username, so don't update with the company if they are a user
	if (nStatus & 0x4)
		name += VarString(GetMainFrame()->m_contactToolBar.m_toolBarCombo->GetValue(GetMainFrame()->m_contactToolBar.m_toolBarCombo->CurSel, 10));	//pull the username out of the toolbar
	else
		name += company;

	IRowSettingsPtr pRow = GetMainFrame()->m_contactToolBar.m_toolBarCombo->GetRow(GetMainFrame()->m_contactToolBar.m_toolBarCombo->GetCurSel());
	pRow->PutValue(5,_bstr_t(name));
	pRow->PutValue(1,_bstr_t(last));
	pRow->PutValue(2,_bstr_t(first));
	pRow->PutValue(3,_bstr_t(middle));
	pRow->PutValue(4,_bstr_t(title));
	if (nStatus & 0x4)
		;	//nothing
	else
		pRow->PutValue(10,_bstr_t(company));

	GetMainFrame()->m_contactToolBar.m_toolBarCombo->Sort();
}


bool CContactsGeneral::SaveAreaCode(long nID) {

	//is the member variable empty
	if (m_strAreaCode.IsEmpty() ) {
		//default to returning true becuase just becauase we didn't do anything with the areacode, doesn't mean they didn't change the number
		return true;
	}
	else {
		//check to see if that is the only thing that is in the box
		CString strPhone;
		GetDlgItemText(nID, strPhone);
		if (strPhone == m_strAreaCode) {
			//if they are equal then erase the area code
			SetDlgItemText(nID, "");
			return false;
		}
		else {
			return true;
		}

	}
	//set out member variable to blank
	m_strAreaCode = "";

}

void CContactsGeneral::FillAreaCode(long nID)  {

	
		//first check to see if anything is in this box
		CString strPhone;
		GetDlgItemText(nID, strPhone);
		if (! ContainsDigit(strPhone)) {
			// (j.gruber 2009-10-07 16:05) - PLID 35825 - updated for city lookup
			CString strAreaCode, strZip, strCity;
			GetDlgItemText(IDC_ZIP_BOX, strZip);
			GetDlgItemText(IDC_CITY_BOX, strCity);
			BOOL bResult = FALSE;
			if (!m_bLookupByCity) {
				bResult = GetZipInfo(strZip, NULL, NULL, &strAreaCode);
			}
			else {
				bResult = GetCityInfo(strCity, NULL, NULL, &strAreaCode);
			}
			if (bResult) {
				SetDlgItemText(nID, strAreaCode);
				
				//set the member variable 
				m_strAreaCode.Format("(%s) ###-####", strAreaCode);

				//set the cursor
				::PostMessage(GetDlgItem(nID)->GetSafeHwnd(), EM_SETSEL, 5, 5);
			}
			else {
				//set the member variable to be blank
				m_strAreaCode = "";
			}		

	  	}
		else {

			//set the member variable to be blank
			m_strAreaCode = "";
		}

}

void CContactsGeneral::UpdateEmailButton(){
	CString strEmail;
	GetDlgItemText(IDC_EMAIL_BOX, strEmail);
	if(strEmail != "")
		GetDlgItem(IDC_EMAIL_BTN)->EnableWindow(TRUE);
	else
		GetDlgItem(IDC_EMAIL_BTN)->EnableWindow(FALSE);
}

#define AUDIT_CONTACT_FIELD(strField)	switch (status) {\
		case 0:\
			item = aeiContactPerson##strField##;\
			break;\
		case 1:\
			item = aeiRefPhysPerson##strField##;\
			break;\
		case 2:\
			item = aeiProviderPerson##strField##;\
			break;\
		case 4:\
			item = aeiUserPerson##strField##;\
			break;\
		case 8:\
			item = aeiSupplierPerson##strField##;\
			break;\
		default:\
			return;	\
		}\
		break;

void CContactsGeneral::AuditContactChange(long nID, long status, CString strNew, CString strOld) {
	//(e.lally 2005-09-02) PLID 17387 - Added auditing for: CompanyID, Prefix, Title, 
		//Spouse, SSN, HomePhone, WorkPhone, Extension, EMail, CellPhone, Pager, OtherPhone, Fax, 
		//Note, Gender, and BirthDate.
	long item = -1;
	int nCol = -1;	//to get old name

	switch (nID) {		//resource id
	case IDC_TITLE_BOX:
		AUDIT_CONTACT_FIELD(Title)

	case IDC_FIRST_NAME_BOX:
		nCol = 2;
		AUDIT_CONTACT_FIELD(First)
		
	case IDC_LAST_NAME_BOX:
		nCol = 1;
		AUDIT_CONTACT_FIELD(Last)
		
	case IDC_MIDDLE_NAME_BOX:
		nCol = 3;
		AUDIT_CONTACT_FIELD(Middle)
		
	case IDC_EMPLOYER_BOX:
		nCol = 10;
		AUDIT_CONTACT_FIELD(Company)
		
	case IDC_ADDRESS1_BOX:
		AUDIT_CONTACT_FIELD(Address1)
		
	case IDC_ADDRESS2_BOX:
		AUDIT_CONTACT_FIELD(Address2)
		
	case IDC_CITY_BOX:
		AUDIT_CONTACT_FIELD(City)
		
	case IDC_STATE_BOX:
		AUDIT_CONTACT_FIELD(State)
		
	case IDC_ZIP_BOX:
		AUDIT_CONTACT_FIELD(Zip)
		
	case IDC_HOME_PHONE_BOX:
		AUDIT_CONTACT_FIELD(HomePhone)
		
	case IDC_WORK_PHONE_BOX:
		AUDIT_CONTACT_FIELD(WorkPhone)
		
	case IDC_EXT_PHONE_BOX:
		AUDIT_CONTACT_FIELD(Extension)
		
	case IDC_CELL_PHONE_BOX:
		AUDIT_CONTACT_FIELD(CellPhone)
		
	case IDC_PAGER_PHONE_BOX:
		AUDIT_CONTACT_FIELD(Pager)
		
	case IDC_OTHER_PHONE_BOX:
		AUDIT_CONTACT_FIELD(OtherPhone)
		
	case IDC_FAX_BOX:
		AUDIT_CONTACT_FIELD(Fax)
		
	case IDC_EMAIL_BOX:
		AUDIT_CONTACT_FIELD(EMail)
		
	case IDC_NOTES:
		AUDIT_CONTACT_FIELD(Note)
		
	case IDC_BDATE_BOX:
		AUDIT_CONTACT_FIELD(BirthDate)
		
	case IDC_SSN_BOX:
		AUDIT_CONTACT_FIELD(SocialSecurity)
		
	case IDC_MARRIAGE_OTHER_BOX:
		AUDIT_CONTACT_FIELD(Spouse)
		
	case IDC_ACCOUNT_BOX:
		AUDIT_CONTACT_FIELD(CompanyID)
		
	case IDC_PREFIX_LIST:
		AUDIT_CONTACT_FIELD(Prefix)
		
	case IDC_MALE:
	case IDC_FEMALE:
		AUDIT_CONTACT_FIELD(Gender)


	default:
		return;		//not one of these fields, let's quit
	}

	//now do the auditing
	long nAuditID = BeginNewAuditEvent();
	if(nAuditID > 0) {
		CString strFullName = GetActiveContactName();
		
		AuditEvent(-1, strFullName, nAuditID, item, GetActiveContactID(), strOld, strNew, aepMedium, aetChanged);
	}

	SendContactsTablecheckerMsg();
}

void CContactsGeneral::AuditField(AuditEventItems aeiItem, CString strOld, CString strNew, AuditEventPriorities aepPriority /* = aepMedium */, AuditEventTypes aetType /* = aetChanged */) {

	long nAuditID;
	nAuditID = BeginNewAuditEvent();
	AuditEvent(-1, GetActiveContactName(), nAuditID, aeiItem, GetActiveContactID(), strOld, strNew, aepPriority, aetType);

	SendContactsTablecheckerMsg();

}

void CContactsGeneral::OnEditsecuritygroupsBtn() 
{
	//DRT 4/3/03 - Changed from bioContactsUsers to bioEditPermissions
	// (j.jones 2009-05-26 15:47) - PLID 34315 - split into bioEditOwnPermissions and bioEditOtherUserPermissions
	if(m_id == GetCurrentUserID()) {
		if(!CheckCurrentUserPermissions(bioEditOwnPermissions, sptWrite)) {
			return;
		}
	}
	else {
		if(!CheckCurrentUserPermissions(bioEditOtherUserPermissions, sptWrite)) {
			return;
		}
	}

	CGroupSecurityDlg dlg(this);
	dlg.Open(m_id);
	GetDlgItem(IDC_EMPLOYER_BOX)->SetFocus();
}

void CContactsGeneral::OnNurseCheck() 
{
	try {
		int nNurse = IsDlgButtonChecked(IDC_NURSE_CHECK) ? 1 : 0;
		ExecuteSql("UPDATE ContactsT SET Nurse = %li WHERE PersonID = %li", nNurse, m_id);

		SendContactsTablecheckerMsg();

		if(IsSurgeryCenter(FALSE) && nNurse == 0 && !IsDlgButtonChecked(IDC_ANESTHESIOLOGIST)) {
			//update licensing information
			ExecuteSql("DELETE FROM PersonCertificationsT WHERE PersonID = %li", m_id);		
			UpdateASCLicenseToDos();
		}

	}NxCatchAll("Error in CContactsGeneral::OnNurseCheck()");
	return;
}

void CContactsGeneral::OnAnesthesiologist() 
{
	try {
		int nAnesthesiologist = IsDlgButtonChecked(IDC_ANESTHESIOLOGIST) ? 1 : 0;
		ExecuteSql("UPDATE ContactsT SET Anesthesiologist = %li WHERE PersonID = %li", nAnesthesiologist, m_id);

		SendContactsTablecheckerMsg();

		if(IsSurgeryCenter(FALSE) && nAnesthesiologist == 0 && IsDlgButtonChecked(IDC_NURSE_CHECK)) {
			//update licensing information
			ExecuteSql("DELETE FROM PersonCertificationsT WHERE PersonID = %li", m_id);		
			UpdateASCLicenseToDos();
		}

	}NxCatchAll("Error in CContactsGeneral::OnAnesthesiologist()");
	return;
}

//Macro for enabling/disabling controls
#define SC_UPDATECTRL(bEnabled, idc) {  CWnd* pWnd;  pWnd = GetDlgItem(idc);  pWnd->EnableWindow(bEnabled);  }

// (d.thompson 2009-10-08) - PLID 35888 - Ideally text controls will be flagged read only, not disabled
#define SC_READCTRL(bReadable, idc) { CEdit *pEdit; pEdit = ((CEdit*)GetDlgItem(idc));  pEdit->SetReadOnly(!bReadable); }

void CContactsGeneral::SecureControls() {
	//DRT 5/2/03 - Enables/disables all controls as necessary

	try {

		//do we want to disable these?
		int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();
		bool bEnabled = false;	//all controls follow the same rules, they either
								//are allowed or they aren't.

		switch(nStatus) {
		case 0x0:
			//other contact
			if(!(GetCurrentUserPermissions(bioContactsOther) & sptWrite))
				bEnabled = false;
			else
				bEnabled = true;
			break;
		case 0x1:
			//referring physician
			if(!(GetCurrentUserPermissions(bioContactsRefPhys) & sptWrite))
				bEnabled = false;
			else
				bEnabled = true;
			break;
		case 0x2:
			//provider
			if(!(GetCurrentUserPermissions(bioContactsProviders) & sptWrite))
				bEnabled = false;
			else
				bEnabled = true;
			break;
		case 0x4:
			//user
			if(!(GetCurrentUserPermissions(bioContactsUsers) & sptWrite))
				bEnabled = false;
			else
				bEnabled = true;
			break;
		case 0x8:
			//supplier
			if(!(GetCurrentUserPermissions(bioContactsSuppliers) & sptWrite))
				bEnabled = false;
			else
				bEnabled = true;
			break;
		default:
			//unknown condition, disable them
			ASSERT(false);
			bEnabled = false;
		}

		//now go through all the controls and update them appropriately
		SC_READCTRL(bEnabled, IDC_EMPLOYER_BOX)
		SC_READCTRL(bEnabled, IDC_ACCOUNT_BOX)
		SC_UPDATECTRL(bEnabled, IDC_PREFIX_LIST)	//??
		SC_READCTRL(bEnabled, IDC_TITLE_BOX)
		SC_UPDATECTRL(bEnabled, IDC_COMBO_PALMSYNC)	//??
		SC_READCTRL(bEnabled, IDC_FIRST_NAME_BOX)
		SC_READCTRL(bEnabled, IDC_MIDDLE_NAME_BOX)
		SC_READCTRL(bEnabled, IDC_LAST_NAME_BOX)
		SC_READCTRL(bEnabled, IDC_ADDRESS1_BOX)
		SC_READCTRL(bEnabled, IDC_ADDRESS2_BOX)
		SC_UPDATECTRL(bEnabled, IDC_CITY_BOX)
		SC_UPDATECTRL(bEnabled, IDC_STATE_BOX)
		SC_UPDATECTRL(bEnabled, IDC_ZIP_BOX)
		SC_READCTRL(bEnabled, IDC_MARRIAGE_OTHER_BOX)
		SC_UPDATECTRL(bEnabled, IDC_BDATE_BOX)
		SC_READCTRL(bEnabled, IDC_SSN_BOX)
		SC_UPDATECTRL(bEnabled, IDC_MALE)
		SC_UPDATECTRL(bEnabled, IDC_FEMALE)
		SC_READCTRL(bEnabled, IDC_HOME_PHONE_BOX)
		SC_READCTRL(bEnabled, IDC_WORK_PHONE_BOX)
		SC_READCTRL(bEnabled, IDC_EXT_PHONE_BOX)
		SC_READCTRL(bEnabled, IDC_EMAIL_BOX)
		SC_READCTRL(bEnabled, IDC_CELL_PHONE_BOX)
		SC_READCTRL(bEnabled, IDC_PAGER_PHONE_BOX)
		SC_READCTRL(bEnabled, IDC_OTHER_PHONE_BOX)
		SC_READCTRL(bEnabled, IDC_FAX_BOX)
		SC_READCTRL(bEnabled, IDC_EMERGENCY_FIRST_NAME)
		SC_READCTRL(bEnabled, IDC_EMERGENCY_LAST_NAME)
		SC_READCTRL(bEnabled, IDC_RELATION_BOX)
		SC_READCTRL(bEnabled, IDC_OTHERWORK_BOX)
		SC_READCTRL(bEnabled, IDC_OTHERHOME_BOX)
		SC_READCTRL(bEnabled, IDC_NOTES)
		SC_READCTRL(bEnabled, IDC_CON_CUSTOM1_BOX)
		SC_READCTRL(bEnabled, IDC_CON_CUSTOM2_BOX)
		SC_READCTRL(bEnabled, IDC_CON_CUSTOM3_BOX)
		SC_READCTRL(bEnabled, IDC_CON_CUSTOM4_BOX)
		SC_READCTRL(bEnabled, IDC_CONTACT_CONFIGURE_PROVIDER_TYPES_BUTTON)// (s.tullis 2014-05-28 12:40) - PLID 61826 - disable control if user has no read permissions
		SC_READCTRL(bEnabled, IDC_NATIONALNUM_BOX)
		SC_UPDATECTRL(bEnabled, IDC_PATCOORD_CHECK)
		SC_UPDATECTRL(bEnabled, IDC_TECHNICIAN_CHECK)
		SC_UPDATECTRL(bEnabled, IDC_DATE_OF_HIRE)
		SC_UPDATECTRL(bEnabled, IDC_DATE_OF_TERM) // (a.walling 2007-11-21 15:11) - PLID 28157		
		SC_READCTRL(bEnabled, IDC_REFPHYSID_BOX)
		SC_READCTRL(bEnabled, IDC_UPIN_BOX)
		SC_READCTRL(bEnabled, IDC_FEDID_BOX)
		SC_READCTRL(bEnabled, IDC_MEDICARE_BOX)
		SC_READCTRL(bEnabled, IDC_BCBS_BOX)
		SC_READCTRL(bEnabled, IDC_NPI_BOX)
		// (j.jones 2013-06-10 13:26) - PLID 57089 - added Group NPI
		SC_READCTRL(bEnabled, IDC_GROUP_NPI_BOX)
		// (j.jones 2009-06-26 09:13) - PLID 34292 - added OHIP Specialty
		SC_READCTRL(bEnabled, IDC_OHIP_SPECIALTY)
		// (j.jones 2011-11-07 14:10) - PLID 46299 - added ProvidersT.UseCompanyOnClaims
		SC_READCTRL(bEnabled, IDC_CHECK_SEND_COMPANY_ON_CLAIM)
		SC_READCTRL(bEnabled, IDC_TAXONOMY_CODE)
		SC_READCTRL(bEnabled, IDC_MEDICAID_BOX)
		SC_READCTRL(bEnabled, IDC_WORKCOMP_BOX)
		SC_READCTRL(bEnabled, IDC_DEA_BOX)
		SC_READCTRL(bEnabled, IDC_LICENSE_BOX)
		SC_READCTRL(bEnabled, IDC_OTHERID_BOX)
		
		//SC_UPDATECTRL(bEnabled, IDC_METHOD_LABEL)
		SC_READCTRL(bEnabled, IDC_METHOD_BOX)
		//TES 2/18/2008 - PLID 28954 - New field
		SC_READCTRL(bEnabled, IDC_ACCOUNT_NAME)
		SC_UPDATECTRL(bEnabled, IDC_SHOW_COMMISSION)
		SC_UPDATECTRL(bEnabled, IDC_NURSE_CHECK)
		SC_UPDATECTRL(bEnabled, IDC_ANESTHESIOLOGIST)
		SC_UPDATECTRL(bEnabled, IDC_CHECK_CONTACT_INACTIVE)
		SC_UPDATECTRL(bEnabled, IDC_CONTACTLOCATION_COMBO)
		SC_UPDATECTRL(bEnabled, IDC_CLAIM_PROVIDER)
		//DRT 11/20/2008 - PLID 32082
		// (b.savon 2013-04-23 14:09) - PLID 56409 - Readonly if they are configured for nexerx
		SC_READCTRL(IsProviderConfiguredForNexERx() ? FALSE : bEnabled, IDC_PROV_SPI)
		SC_UPDATECTRL(bEnabled, IDC_AMA_SPECIALTY_LIST)
		SC_UPDATECTRL(bEnabled, IDC_AFFILIATE_PHYS); // (j.gruber 2011-09-22 11:22) - PLID 45354
		SC_UPDATECTRL(bEnabled, IDC_CONTACT_CONFIGURE_PROVIDER_TYPES_BUTTON); //(r.wilson 4/22/2014) PLID 61826
		// (s.tullis 2015-10-29 17:27) - PLID 67483 - Follow Permissions
		SC_UPDATECTRL(bEnabled, IDC_CAPITATION_DISTRIBUTION_EDIT);

		if(bEnabled && (GetCurrentUserPermissions(bioContactsDefaultCost) & sptWrite)) {
			((CNxEdit*)GetDlgItem(IDC_DEFAULT_COST_EDIT))->SetReadOnly(FALSE);
		}
		else {
			((CNxEdit*)GetDlgItem(IDC_DEFAULT_COST_EDIT))->SetReadOnly(TRUE);
		}
			
		// (d.singleton 2014-05-20 10:42) - PLID 61890 - need the direct message controls to follow provider permissions
		CString strDirAddress;
		GetDlgItemText(IDC_DIRECT_ADDRESS_EDIT, strDirAddress);
		if(!strDirAddress.IsEmpty()) {
			SC_UPDATECTRL(bEnabled, IDC_DIRECT_ADDRESS_USERS_BUTTON);
			SC_READCTRL(bEnabled, IDC_DIRECT_ADDRESS_EDIT);
		}
		else {
			SC_UPDATECTRL(false, IDC_DIRECT_ADDRESS_USERS_BUTTON);
			SC_READCTRL(bEnabled, IDC_DIRECT_ADDRESS_EDIT);
		}

	} NxCatchAll("Error in SecureControls()");

}

void CContactsGeneral::OnKillFocusBdateBox() 
{
	Save(IDC_BDATE_BOX);
}

void CContactsGeneral::OnEditPrefixes() 
{
	
	CEditPrefixesDlg dlg(this);
	dlg.m_bChangeInformIds = false;
	if(IDOK == dlg.DoModal()) {
		m_PrefixCombo->Requery();
		
		//add the blank row to it
		IRowSettingsPtr pRow;
		pRow = m_PrefixCombo->GetRow(-1);
		pRow->PutValue(0, (long)0);
		pRow->PutValue(1, _variant_t(""));
		// (j.jones 2009-08-10 17:21) - PLID 35160 - ensured this row had a gender of 0 assigned
		pRow->PutValue(2, (long)0);
		m_PrefixCombo->InsertRow(pRow, 0);
		
		UpdateView();
	}	
	GetDlgItem(IDC_PREFIX_LIST)->SetFocus();
}

void CContactsGeneral::OnSelChosenPrefixList(long nRow) 
{
	// (a.walling 2008-10-10 13:03) - PLID 29688 - Consolidated code into the SelChosen function.
	// Previously it would update data (and audit) as you scrolled through the list!
	// There is a bug where the selection you choose does not stick. I am not exactly sure why,
	// not can I reproduce yet, but this is at least a good start since it is the only difference
	// between contacts and patient general 1.

	//(e.lally 2005-09-02) PLID 17387 - Add auditing for Prefix changes (amongst others)
	try {
		CString str, strOld, strPrefix;
		int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();

		_RecordsetPtr rs = CreateRecordset("SELECT PrefixT.Prefix FROM PersonT INNER JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID WHERE PersonT.ID = %li", m_id);
		if(!rs->eof) {
			strOld = AdoFldString(rs, "Prefix","");
		}
		rs->Close();

		if(nRow == -1) {
			strPrefix = "NULL";
			str = "";
		} else {
			strPrefix.Format("%li",VarLong(m_PrefixCombo->GetValue(nRow, 0)));
			//Get the string value of the new prefix
			str.Format("%s", VarString(m_PrefixCombo->GetValue(nRow, 1)));
			if(!ReturnsRecords("SELECT ID FROM PrefixT WHERE ID = %s", strPrefix)) {
				if(strPrefix != "0") {
					MessageBox("This prefix has been deleted, possibly by another user.");
					m_PrefixCombo->RemoveRow(nRow);
					m_PrefixCombo->CurSel = -1;
				}
				strPrefix = "NULL";
				str="";
			}
		}

		ExecuteSql("UPDATE PersonT SET PrefixID = %s WHERE PersonT.ID = %li", strPrefix, m_id);

		//audit the change
		AuditContactChange(IDC_PREFIX_LIST, nStatus, str, strOld);

		//check the preference first
		if(GetRemotePropertyInt("GenderPrefixLink", 1, 0, "<None>", true) == 1) {
			//we need to update the gender based on the selection
			if (m_PrefixCombo->GetValue(nRow,2).vt != VT_EMPTY)
			{
				long nGender = VarLong(m_PrefixCombo->GetValue(nRow,2));
				if(nGender == 1) { // Male
					m_male.SetCheck(true);
					m_female.SetCheck(false);
					OnMale();
				}
				else if (nGender == 2) { // Female
					m_male.SetCheck(false);
					m_female.SetCheck(true);
					OnFemale();
				}
			}
		}

		PPCModifyContact(m_id);

		// (v.maida 2014-12-23 12:19) - PLID 64472 - Add ref phys HL7 update, if applicable.
		if ( (str != strOld) && (nStatus & 0x1)) {
			AddOrUpdateRefPhysInHL7(m_id, false);
		}

	} NxCatchAll("Error in changing prefix");
}

void CContactsGeneral::OnSelChosenContactLocation(long nRow)
{
	try {
		if(nRow != -1) {
			int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();
			//TES 9/8/2008 - PLID 27727 - This combo now represents PersonT.Location, not ProvidersT.DefLocationID
			long nLocationID = VarLong(m_LocationCombo->GetValue(nRow, 0));
			ExecuteSql("UPDATE PersonT SET Location = %d WHERE ID = %d", nLocationID, m_id);

			SendContactsTablecheckerMsg();
		}
	} NxCatchAll("Error in changing default location");
}

void CContactsGeneral::OnCheckContactInactive() 
{
	//(e.lally 2005-11-02) PLID 17444 - added the ability to make referring physicians inactive
	//(e.lally 2005-11-04) PLID 18152 - added the ability to make suppliers inactive
	//(e.lally 2005-11-28) PLID 18153 - added the ability to make other contacts inactive
	try {

		int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();
		BOOL bInactive = m_checkInactive.GetCheck();
		//(r.wilson 7/29/2013) PLID 48684
		BOOL bUnlinkEMRDefaultProviderID = FALSE;

		CString strMsg;
		if(bInactive) {
			//we're inactivating this person
			if (m_strContactType == "User") {
				//first check to see if we can inactivate them

				//if the user is yourself, you can't inactivate yourself
				if(m_id == GetCurrentUserID()) {
					MsgBox("You cannot inactivate your own username while you are logged in.");
					m_checkInactive.SetCheck(!bInactive);
					return;
				}

				//If the user is an Administrator, you CANNOT DELETE it
				if(ReturnsRecords("SELECT Administrator FROM UsersT WHERE Administrator = 1 AND PersonID = %li",m_id)) {
					MsgBox("This user is an Administrator and cannot be deactivated.\n\n"
						"You may, however, remove the user's administrator status and then inactivate them,\n"
						"if another administrator user exists.");
					m_checkInactive.SetCheck(!bInactive);
					return;
				}

				_RecordsetPtr prs = CreateRecordset("SELECT PersonID FROM UserLocationT INNER JOIN LocationsT ON UserLocationT.LocationID = LocationsT.ID WHERE PersonID != %li AND Managed = 1", m_id);
				if(prs->eof) {
					//if we take away this user, there is noone else allowed to login
					MsgBox("You must have at least one user allowed to login to Practice. Inactivating this user would mean nobody can log in.");
					m_checkInactive.SetCheck(!bInactive);
					return;
				}

				// (j.jones 2011-10-06 16:43) - PLID 45828 - if this user is logged in from a device, you cannot inactivate the user
				if(ReturnsRecordsParam("SELECT TOP 1 UserID FROM UserLoginTokensT WHERE UserID = {INT}", m_id)) {
					MsgBox("This user is currently logged in from a device, and cannot be inactivated while they are still logged in to the system.");
					m_checkInactive.SetCheck(!bInactive);
					return;
				}

				// (b.savon 2013-03-01 14:26) - PLID 54578 - if this user has nexerx role
				if(ReturnsRecordsParam("SELECT TOP 1 PersonID FROM UsersT WHERE PersonID = {INT} AND NexERxUserTypeID IS NOT NULL AND NexERxUserTypeID > -1", m_id)){
					MsgBox("This user has a NexERx role and cannot be changed. Please deactivate their NexERx license before inactivating the user.");
					m_checkInactive.SetCheck(!bInactive);
					return;
				}

				// (v.maida 2014-08-14 15:50) - PLID 62753 - Don't allow the contact to be marked inactive if it is a user that has been selected to be assigned a Todo alarm for diagnosis codes spawned in different diagnosis modes.
				if (ReturnsRecordsParam("SELECT TOP 1 1 FROM dbo.StringToIntTable((SELECT MemoParam FROM ConfigRT WHERE Name = 'TodoUsersWhenDiagCodeSpawnedInDiffDiagMode'), ',') AS PrefUsersQ WHERE PrefUsersQ.Data = {INT}", m_id)) {
					MsgBox("This user has been selected to be assigned Todo alarms when diagnosis codes are spawned while in a different global diagnosis mode, and thus cannot be inactivated."
						"\r\nNavigate to Tools->Preferences...->Patients Module->NexEMR->EMR 3 tab->'Create a Todo when a diagnosis code is spawned while in a different global diagnosis mode' preference and uncheck this user's name from that preference's datalist to lift this restriction.");
					m_checkInactive.SetCheck(!bInactive);
					return;
				}

				strMsg.Format("You are about to mark this user inactive, which will forbid them from logging in to Practice.\n"
					"Are you sure you wish to do this?");
			}
			else if(m_strContactType == "Provider" || m_strContactType == "Referring Physician" ||
				m_strContactType == "Supplier"){

				// (b.savon 2013-03-01 14:26) - PLID 54578 - if this user has nexerx role
				if(ReturnsRecordsParam("SELECT TOP 1 PersonID FROM ProvidersT WHERE PersonID = {INT} AND NexERxProviderTypeID IS NOT NULL AND NexERxProviderTypeID > -1", m_id)){
					MsgBox("This user has a NexERx role and cannot be changed. Please deactivate their NexERx license before inactivating the provider.");
					m_checkInactive.SetCheck(!bInactive);
					return;
				}

				
				//(r.wilson 7/29/2013) PLID 48684 - We are about to inactivate this provider
				long nLinkedCount = 0;
				bUnlinkEMRDefaultProviderID = TRUE;
				//(r.wilson 7/29/2013) PLID 48684 - First make sure no other privder has this provider as their default emr primary provider
				_RecordsetPtr prs = CreateParamRecordset("SELECT Count(PersonID) AS LinkedCount FROM ProvidersT WHERE EMRDefaultProviderID = {INT}", m_id);
				if(!prs->eof){
					nLinkedCount = AdoFldLong(prs, "LinkedCount");
				}
				if(nLinkedCount > 0){
					CString strTmpMessage;
					strTmpMessage.Format("This provider is the default licensed EMR primary provider for %li provider%s. Do you still want to inactivate this provider?", nLinkedCount, nLinkedCount < 2?"":"s" );					
					if(MsgBox(strTmpMessage,"Practice",MB_ICONEXCLAMATION|MB_YESNO)== IDYES ){					
						bUnlinkEMRDefaultProviderID = TRUE;
					}
				}
				

				strMsg.Format("You are about to mark this %s inactive.  This will remove them from all %s lists.\n"
					"Are you sure you wish to do this?", m_strContactType, m_strContactType);
			}else if(m_strContactType == "Other"){
				//Find out if they are a Nurse or Anesthesiologist or regular contact
				//Format message accordingly
				_RecordsetPtr prs = CreateRecordset("SELECT PersonID, Nurse, Anesthesiologist FROM PersonT INNER JOIN ContactsT ON PersonT.ID = ContactsT.PersonID WHERE PersonT.ID = %li", m_id);
				if(!prs->eof) {
					BOOL bNurse = AdoFldBool(prs, "Nurse");
					BOOL bAnesthesiologist = AdoFldBool(prs, "Anesthesiologist");
					CString strContact;
					if(bNurse && bAnesthesiologist)
						strContact = "Nurse/Anesthesiologist";
					else if(bNurse)
						strContact = "Nurse";
					else if(bAnesthesiologist)
						strContact = "Anesthesiologist";
					else
						strContact = "contact";
					strMsg.Format("You are about to mark this %s inactive.  This will remove them from all %s lists.\n"
						"Are you sure you wish to do this?", strContact, strContact);
				}
				else
					strMsg = "You are about to mark this contact inactive.  This will remove them from all contact lists.\n"
						"Are you sure you wish to do this?";
			}
			else
				return;
		}
		else {
			//we're re-activating this person
			if (m_strContactType == "User")
				strMsg.Format("You are about to reactivate this user, which will re-allow them to log in to Practice.\n"
					"Are you sure you wish to do this?");
			else if(m_strContactType == "Provider" || m_strContactType == "Referring Physician" ||
					m_strContactType == "Supplier")
				strMsg.Format("You are about to reactivate this %s, are you sure you wish to do this?", m_strContactType);
			else if(m_strContactType == "Other"){
				//Find out if they are a Nurse or Anesthesiologist or regular contact
				//Format message accordingly
				_RecordsetPtr prs = CreateRecordset("SELECT PersonID, Nurse, Anesthesiologist FROM PersonT INNER JOIN ContactsT ON PersonT.ID = ContactsT.PersonID WHERE PersonT.ID = %li", m_id);
				if(!prs->eof) {
					BOOL bNurse = AdoFldBool(prs, "Nurse");
					BOOL bAnesthesiologist = AdoFldBool(prs, "Anesthesiologist");
					CString strContact;
					if(bNurse && bAnesthesiologist)
						strContact = "Nurse/Anesthesiologist";
					else if(bNurse)
						strContact = "Nurse";
					else if(bAnesthesiologist)
						strContact = "Anesthesiologist";
					else
						strContact = "contact";
					strMsg.Format("You are about to reactivate this %s, are you sure you wish to do this?", strContact);
				}
				else
					strMsg = "You are about to reactivate this contact, are you sure you wish to do this?";
			}
			else
				return;
		}

		if(IDNO == MessageBox(strMsg,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			m_checkInactive.SetCheck(!bInactive);
			return;
		}

		if(bInactive) {

			if (m_strContactType == "User") {
				//see if they have any assigned steps
				long nAssignStepsTo = -1;
				// (z.manning 2008-10-13 09:52) - PLID 21108 - Need to handle lab procedure steps here
				// (j.jones 2008-11-26 13:30) - PLID 30830 - changed StepsT/StepTemplatesT references to support the multi-user tables
				if(ReturnsRecords("SELECT UserID FROM StepsAssignToT WHERE UserID = %li UNION SELECT UserID FROM StepTemplatesAssignToT WHERE UserID = %li UNION SELECT UserID FROM LaddersT WHERE UserID = %li UNION SELECT UserID FROM LabProcedureStepTodoAssignToT WHERE UserID = %li", m_id, m_id, m_id, m_id) ) {
					CSelectUserDlg dlg(this);
					dlg.m_nExcludeUser = m_id;
					dlg.m_strCaption = "This user has Tracking steps assigned to them. Please select a user for these steps to be re-assigned to.";
					if(IDOK == dlg.DoModal()) {

						nAssignStepsTo = dlg.m_nSelectedUser;

						//DRT 9/25/03 - PLID 9618 - Must do the updating code after we've done all prompting - don't want to update half the stuff then
						//	let them hit cancel.
					}
					else {
						CheckDlgButton(IDC_CHECK_CONTACT_INACTIVE, FALSE);
						return;
					}
				}

				//(e.lally 2010-08-03) PLID 33962 - We may need to reassign the preference for Nexweb ladders
				long nAssignToNexWebLaddersPref =-1;
				if(GetRemotePropertyInt("NexWebTrackingLadderAssignToUser", -1, 0, "<None>", false) == m_id){
					CSelectUserDlg dlg(this);
					dlg.m_nExcludeUser = m_id;
					dlg.m_strCaption = "This user is the default owner of new Tracking Ladders created through NexWeb. Please select a user for the new Tracking Ladders to be assigned to by default.\r\n";
					if(IDOK == dlg.DoModal()) {
						nAssignToNexWebLaddersPref = dlg.m_nSelectedUser;
					}
					else {
						CheckDlgButton(IDC_CHECK_CONTACT_INACTIVE, FALSE);
						return;
					}
				}

				//(e.lally 2010-08-18) PLID 37982 - We may need to reassign the preference for Nexweb ToDos
				//(e.lally 2010-11-19) PLID 35819 - Migrated from NexWeb Leads AssignTo Users preference to its own table structure
				//(e.lally 2012-01-03) PLID 47136 - NexWeb_ToDo_Template_Assign_To_T is now NexWeb_Event_Assign_To_T
				long nAssignToNexWebEvent = -1;
				{
					//Check if this user is the only one assigned to a NexWeb Todo template.
					_RecordsetPtr rs = CreateParamRecordset("SELECT COUNT(*) AS UserCount FROM NexWebEventToDoAssignToT "
						"WHERE NexWebEventID IN(SELECT NexWebEventID FROM NexWebEventToDoAssignToT WHERE UserID = {INT}) "
						"GROUP BY NexWebEventID "
						"HAVING COUNT(*) = 1 ", m_id);
					if(!rs->eof){
						//this is the only user assigned
						CSelectUserDlg dlg(this);
						dlg.m_nExcludeUser = m_id;
						dlg.m_bAllowNone = false;
						dlg.m_strCaption = "This user is the default owner of new ToDo Alarms for events in NexWeb. Please select a user for the new ToDo Alarms to be assigned to by default.\r\n";
						if(IDOK == dlg.DoModal()) {
							nAssignToNexWebEvent = dlg.m_nSelectedUser;
						}
						else {
							CheckDlgButton(IDC_CHECK_CONTACT_INACTIVE, FALSE);
							return;
						}
					}
				}

				//see if they have any assigned ToDos
				//see if they have any assigned steps
				// (c.haag 2008-06-11 12:09) - PLID 11599 - Use the new todo structure
				// (j.jones 2009-09-21 13:52) - PLID 35595 - we're only going to transfer
				// incompleted todos, so filter on whether any exist
				long nAssignAlarmsTo = -1;
				if(ReturnsRecords("SELECT AssignTo "
					"FROM ToDoList "
					"INNER JOIN TodoAssignToT ON TodoAssignToT.TaskID = ToDoList.TaskID "
					"WHERE AssignTo = %li AND Done Is Null", m_id) ) {
					CSelectUserDlg dlg(this);
					dlg.m_nExcludeUser = m_id;
					dlg.m_strCaption = "This user has unfinished ToDo Alarms assigned to them. Please select a user for these alarms to be re-assigned to.\n"
						"(If you choose {None}, they will be unchanged.)";
					if(IDOK == dlg.DoModal()) {

						nAssignAlarmsTo = dlg.m_nSelectedUser;

						//DRT 9/25/03 - PLID 9618 - Must do the updating code after we've done all prompting - don't want to update half the stuff then
						//	let them hit cancel.
					}
					else {
						CheckDlgButton(IDC_CHECK_CONTACT_INACTIVE, FALSE);
						return;
					}
				}

				//see if they have any patients set as a patient coordinator
				long nAssignToUser = -1;
				if(ReturnsRecords("SELECT PersonID FROM PatientsT WHERE EmployeeID = %li", m_id)) {
					CSelectUserDlg dlg(this);
					dlg.m_nExcludeUser = m_id;
					dlg.m_strCaption = "This user (as a patient coordinator) has patients assigned to them.  Please select a new patient coordinator for these "
						"patients to be re-assigned to.";
					dlg.m_strUserWhereClause = "UsersT.PatientCoordinator = 1";

					if(IDOK == dlg.DoModal()) {
						nAssignToUser = dlg.m_nSelectedUser;

						//DRT 9/25/03 - PLID 9618 - Must do the updating code after we've done all prompting - don't want to update half the stuff then
						//	let them hit cancel.
					}
					else {
						CheckDlgButton(IDC_CHECK_CONTACT_INACTIVE, FALSE);
						return;
					}
				}

				//see if they exist in a surgery
				// (j.jones 2009-08-24 12:13) - PLID 35124 - personnel are only in Preference Cards now
				long nAssignPreferenceCardTo = -1;
				if(ReturnsRecords("SELECT Name FROM PreferenceCardsT "
					"INNER JOIN PreferenceCardDetailsT ON PreferenceCardsT.ID = PreferenceCardDetailsT.PreferenceCardID "
					"WHERE PersonID = %li", m_id) ) {
					CSelectUserDlg dlg(this);
					dlg.m_nExcludeUser = m_id;
					dlg.m_strCaption = "This user exists in a preference card. Please select a user to replace these preference card details with.\n"
						"(If you choose {None}, the preference card details will be deleted.)";
					if(IDOK == dlg.DoModal()) {

						nAssignPreferenceCardTo = dlg.m_nSelectedUser;

						//DRT 9/25/03 - PLID 9618 - Must do the updating code after we've done all prompting - don't want to update half the stuff then
						//	let them hit cancel.
					}
					else {
						CheckDlgButton(IDC_CHECK_CONTACT_INACTIVE, FALSE);
						return;
					}
				}

				// (j.jones 2005-01-31 17:20) - if this user was the default ordering user, reset the value
				long nDefProductUserID = GetRemotePropertyInt("DefaultProductOrderingUser",-1,0,"<None>",TRUE);
				BOOL bResetDefaultProduceOrderingUser = FALSE;
				if(nDefProductUserID == m_id) {
					bResetDefaultProduceOrderingUser = TRUE;					
				}

				// (c.haag 2008-07-16 10:52) - PLID 17244 - Prompt if any EMR todo actions involve this user
				long nAssignEmrTodoActionsTo = -1;
				if(ReturnsRecords("SELECT ActionID FROM EMRActionsTodoAssignToT WHERE AssignTo = %li", m_id)) {
					CSelectUserDlg dlg(this);
					dlg.m_nExcludeUser = m_id;
					dlg.m_bAllowNone = false;
					dlg.m_strCaption = "This user has EMR Todo spawning actions assigned to them. Please select a user for these actions to be re-assigned to.";
					if (IDOK == dlg.DoModal()) {
						nAssignEmrTodoActionsTo = dlg.m_nSelectedUser;
					}
					else {
						CheckDlgButton(IDC_CHECK_CONTACT_INACTIVE, FALSE);
						return;
					}
				}

				//Actually do the writing
				BEGIN_TRANS("InactiveUser") 
					if(nAssignStepsTo != -1) {
						CString strSqlBatch;
						AddStatementToSqlBatch(strSqlBatch, "UPDATE LaddersT SET UserID = %li WHERE UserID = %li", nAssignStepsTo, m_id);
						// (j.jones 2008-11-26 15:30) - PLID 30830 - changed to support multiple users per step
						AddStatementToSqlBatch(strSqlBatch, "UPDATE StepTemplatesAssignToT SET UserID = %li WHERE UserID = %li", nAssignStepsTo, m_id);
						AddStatementToSqlBatch(strSqlBatch, "UPDATE StepsAssignToT SET UserID = %li WHERE UserID = %li", nAssignStepsTo, m_id);
						// (z.manning 2008-10-13 08:40) - PLID 21108 - Lab step user ID
						AddStatementToSqlBatch(strSqlBatch, "UPDATE LabProcedureStepTodoAssignToT SET UserID = %li WHERE UserID = %li", nAssignStepsTo, m_id);
						ExecuteSqlBatch(strSqlBatch);
					}
					else {
						CString strSqlBatch;
						AddStatementToSqlBatch(strSqlBatch, "UPDATE LaddersT SET UserID = NULL WHERE UserID = %li", m_id);
						// (j.jones 2008-11-26 15:30) - PLID 30830 - changed to support multiple users per step
						AddStatementToSqlBatch(strSqlBatch, "DELETE FROM StepTemplatesAssignToT WHERE UserID = %li", m_id);
						AddStatementToSqlBatch(strSqlBatch, "DELETE FROM StepsAssignToT WHERE UserID = %li", m_id);
						// (z.manning 2008-10-13 08:40) - PLID 21108 - Lab step user ID
						AddStatementToSqlBatch(strSqlBatch, "DELETE FROM LabProcedureStepTodoAssignToT WHERE UserID = %li", m_id);
						ExecuteSqlBatch(strSqlBatch);
					}

					if(nAssignAlarmsTo != -1) {
						// (c.haag 2008-06-09 12:12) - PLID 30321 - We now use a global function to replace assignees
						// (j.jones 2009-09-21 13:52) - PLID 35595 - transferring todos when inactivating should
						// only transfer incomplete todos
						TodoTransferAssignTo(m_id,nAssignAlarmsTo, "Done IS NULL");
					}

					if(nAssignToUser != -1) {
						ExecuteSql("UPDATE PatientsT SET EmployeeID = %li WHERE EmployeeID = %li", nAssignToUser, m_id);
					}

					//(e.lally 2010-08-03) PLID 33962 - We may need to reassign the preference for Nexweb ladders
					if(nAssignToNexWebLaddersPref != -1) {
						SetRemotePropertyInt("NexWebTrackingLadderAssignToUser", nAssignToNexWebLaddersPref, 0, "<None>");
					}

					//(e.lally 2010-11-19) PLID 35819 - Migrated from NexWeb Leads AssignTo Users preference to its own table structure
					//(e.lally 2012-01-03) PLID 47136 - NexWeb_ToDo_Template_Assign_To_T is now NexWeb_Event_Assign_To_T
					//reassign NexWeb ToDo template assignment
					if(nAssignToNexWebEvent != -1){
						//Add the replacement user then remove the inactive user to ensure we don't double enter the new user anywhere.
						ExecuteParamSql("INSERT INTO NexWebEventToDoAssignToT (NexWebEventID, UserID) "
							"SELECT DISTINCT NexWebEventID, {INT} "
							"FROM NexWebEventToDoAssignToT "
							"WHERE UserID = {INT} AND NexWebEventID NOT IN( "
								"SELECT NexWebEventID FROM NexWebEventToDoAssignToT "
								"WHERE UserID = {INT} "
							")\r\n"
							"DELETE FROM NexWebEventToDoAssignToT WHERE UserID = {INT}"
							, nAssignToNexWebEvent, m_id, nAssignToNexWebEvent, m_id);
					}
					else{
						//This user either does not appear on a NexWeb ToDo template or is one of multiple users where
						//we can silently remove them
						ExecuteParamSql("DELETE FROM NexWebEventToDoAssignToT WHERE UserID = {INT}", m_id);
					}

					// (j.jones 2009-08-24 12:13) - PLID 35124 - personnel are only in Preference Cards now
					if(nAssignPreferenceCardTo != -1) {
						ExecuteParamSql("UPDATE PreferenceCardDetailsT SET PersonID = {INT} WHERE PersonID = {INT}", nAssignPreferenceCardTo, m_id);
					}
					else {
						ExecuteParamSql("DELETE FROM PreferenceCardDetailsT WHERE PersonID = {INT}", m_id);
					}
					
					// (a.wetta 2007-01-11 14:23) - PLID 16065 - If a user is inactivated we don't want them to be linked to a resource anymore
					ExecuteSql("DELETE FROM ResourceUserLinkT WHERE UserID = %li", m_id);

					SetRemotePropertyInt("DefaultProductOrderingUser",-1,0,"<None>");

					// (c.haag 2008-07-16 10:52) - PLID 17244 - Reassign EMR todo action users
					if (nAssignEmrTodoActionsTo > -1) {
						TodoTransferEmrActionAssignTo(m_id, nAssignEmrTodoActionsTo);
					}

					//(r.wilson 7/29/2013) PLID 48684 - Since this provider is being inactivated we set EMRDefaultProviderID to NULL for anyone with his/her id there
					if(bUnlinkEMRDefaultProviderID == TRUE)
					{
						ExecuteParamSql("UPDATE ProvidersT SET EMRDefaultProviderID = NULL WHERE EMRDefaultProviderID = {INT}", m_id);
					}

				END_TRANS_CATCH_ALL("InactiveUser")
			}

			else if(m_strContactType == "Provider") {
				CString strMsg = "";

				//check the data to let them know what we'll be deleting - slow, but better safe than sorry
				if(ReturnsRecords("SELECT DefaultProviderID FROM LocationsT WHERE DefaultProviderID = %li", m_id)) {
					strMsg += "  - Default Provider\r\n";
				}

				if(ReturnsRecords("SELECT ProviderID FROM MultiFeeProvidersT WHERE ProviderID = %li", m_id)) {
					strMsg += "  - Fee Schedules\r\n";
				}

				// (j.jones 2009-08-24 11:36) - PLID 35124 - changed to PreferenceCardDetailsT
				if(ReturnsRecords("SELECT PersonID FROM PreferenceCardDetailsT WHERE PersonID = %li", m_id)) {
					strMsg += "  - Preference Cards (as Personnel)\r\n";
				}

				// (j.jones 2009-08-24 11:36) - PLID 35124 - changed to PreferenceCardProvidersT
				if(ReturnsRecords("SELECT ProviderID FROM PreferenceCardProvidersT WHERE ProviderID = %li", m_id)) {
					strMsg += "  - Preference Cards (as Provider)\r\n";
				}

				if(ReturnsRecords("SELECT * FROM ConfigRT WHERE Name = 'MirrorImportProvider' AND IntParam = %li", m_id)) {
					strMsg += "  - Mirror Default Provider\r\n";
				}

				if(ReturnsRecords("SELECT ProviderID FROM ResourceProviderLinkT WHERE ProviderID = %li", m_id)) {
					strMsg += "  - Resource / Provider Linking\r\n";
				}

				if(ReturnsRecords("SELECT ProviderID FROM SurgeryDetailsT WHERE ProviderID = %li", m_id)) {
					strMsg += "  - Surgery Item Providers\r\n";
				}

				//now warn them based on that
				if(strMsg.IsEmpty()) {
					//no warnings, so don't bother telling them anything, nothing will be deleted
				}
				else {
					//format a message nicely
					CString str;
					str.Format("This provider will be removed from:\r\n"
						"%s"
						"Are you sure you wish to continue?", strMsg);

					if(MsgBox(MB_YESNO, str) == IDNO) {
						CheckDlgButton(IDC_CHECK_CONTACT_INACTIVE, FALSE);
						return;
					}
				}


				//default provider
				ExecuteSql("UPDATE LocationsT SET DefaultProviderID = NULL WHERE DefaultProviderID = %li", m_id);
				//remove from any Multi Fee schedules
				ExecuteSql("DELETE FROM MultiFeeProvidersT WHERE ProviderID = %li", m_id);
				//remove from any Preference Cards (as personnel)
				// (j.jones 2009-08-24 11:36) - PLID 35124 - changed to PreferenceCardDetailsT
				ExecuteParamSql("DELETE FROM PreferenceCardDetailsT WHERE PersonID = {INT}", m_id);
				//remove from any Preference Cards (as a linked provider)
				// (j.jones 2009-08-24 11:36) - PLID 35124 - changed to PreferenceCardProvidersT
				ExecuteParamSql("DELETE FROM PreferenceCardProvidersT WHERE ProviderID = {INT}", m_id);
				//remove from ResourceProviderLinkT (linked to resources)
				ExecuteSql("DELETE FROM ResourceProviderLinkT WHERE ProviderID = %li", m_id);
				//remove from SurgeryDetailsT.ProviderID
				ExecuteSql("UPDATE SurgeryDetailsT SET ProviderID = NULL WHERE ProviderID = %li", m_id);

				//default durations are left in the data in case he is re-activated

				//delete if the mirror provider default
				if(ReturnsRecords("SELECT * FROM ConfigRT WHERE Name = 'MirrorImportProvider' AND IntParam = %li", m_id)) {
					SetRemotePropertyInt("MirrorImportAssignProvider", 0, 0, "<None>");
					SetRemotePropertyInt("MirrorImportProvider", 0, 0, "<None>");
				}
			}
			else if(m_strContactType == "Referring Physician"){
				//if future table references are added for referring physicians, they should be handled here.
			}
			else if(m_strContactType == "Supplier"){
				//if future table references are added for suppliers, they should be handled here.
			}
			else if(m_strContactType == "Other"){
				//if future table references are added for other contacts, they should be handled here.
			}
		}

		ExecuteSql("UPDATE PersonT SET Archived = %li WHERE ID = %li",bInactive ? 1 : 0, m_id);

		if(m_strContactType == "User")
			CClient::RefreshTable(NetUtils::Coordinators, m_id);
		else if(m_strContactType == "Provider")
			CClient::RefreshTable(NetUtils::Providers, m_id);
		else if (m_strContactType == "Referring Physician")
			CClient::RefreshTable(NetUtils::RefPhys, m_id);
		else if(m_strContactType == "Supplier")
			CClient::RefreshTable(NetUtils::Suppliers, m_id);
		else if(m_strContactType == "Other")
			CClient::RefreshTable(NetUtils::ContactsT, m_id);

		// (j.jones 2009-09-10 11:01) - PLID 28872 - update ASC todos if a provider or nurse/anesthesiologist
		if(IsSurgeryCenter(FALSE) &&
			(m_strContactType == "Provider" ||
			(m_strContactType == "Other" &&
				(IsDlgButtonChecked(IDC_NURSE_CHECK) || IsDlgButtonChecked(IDC_ANESTHESIOLOGIST))))) {
			//update licensing information
			UpdateASCLicenseToDos();
		}

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1) {
			CString strNew;
			if(bInactive)
				strNew = "<Marked Inactive>";
			else
				strNew = "<Marked Active>";
			int nAuditItem = -1;
			if(m_strContactType == "User")
				nAuditItem = aeiUserInactive;
			else if(m_strContactType == "Provider")
				nAuditItem = aeiProviderInactive;
			else if(m_strContactType == "Referring Physician")
				nAuditItem = aeiRefPhysInactive;
			else if(m_strContactType == "Supplier")
				nAuditItem = aeiSupplierInactive;
			else if(m_strContactType == "Other")
				nAuditItem = aeiContactInactive;
			else
				//this should never happen
				return;
			AuditEvent(-1, GetExistingContactName(m_id), nAuditID, nAuditItem, m_id, "", strNew, aepHigh, aetChanged);
		}		

		SendContactsTablecheckerMsg();

		// (v.maida 2014-12-23 12:19) - PLID 64472 - Add ref phys HL7 update, if applicable.
		if (nStatus & 0x1) {
			AddOrUpdateRefPhysInHL7(m_id, false);
		}

		GetMainFrame()->m_contactToolBar.Requery();

	}NxCatchAll("Error changing the 'active' status.");	
}

HBRUSH CContactsGeneral::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	//to avoid drawing problems with transparent text and disabled ites
	//override the NxDialog way of doing text with a non-grey background
	//NxDialog relies on the NxColor to draw the background, then draws text transparently
	//instead, we actually color the background of the STATIC text
	// (m.hancock 2006-08-02 13:58) - PLID 20917 - Added controls for displaying number of referred patients and prospects
	if (nCtlColor == CTLCOLOR_STATIC 
		&& (pWnd->GetDlgCtrlID() == IDC_CHECK_CONTACT_INACTIVE
		|| pWnd->GetDlgCtrlID() == IDC_DEFAULT_COST_LABEL
		|| pWnd->GetDlgCtrlID() == IDC_DEFAULT_COST_EDIT
		|| pWnd->GetDlgCtrlID() == IDC_NUM_PATIENTS_REF_LABEL
		|| pWnd->GetDlgCtrlID() == IDC_NUM_PATIENTS_REF_BOX
		|| pWnd->GetDlgCtrlID() == IDC_NUM_PROSPECTS_REF_LABEL
		|| pWnd->GetDlgCtrlID() == IDC_NUM_PROSPECTS_REF_BOX)) {
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	else {
		return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);		
	}
	*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CContactsGeneral::OnShowCommission() 
{
	//Make sure they have access to NexSpa before we let them use this.
	if(!IsSpa(TRUE)) {
		MsgBox("You must purchase a NexSpa license before using this feature.");
		return;
	}

	// (a.wetta 2007-05-17 15:19) - PLID 25394 - Make sure they have permission to edit commissions
	if (CheckCurrentUserPermissions(bioProviderCommission, sptRead)) {
		CCommissionSetupDlg dlg(this);
		dlg.m_nProviderID = m_id;
		dlg.DoModal();
	}
}

void CContactsGeneral::OnShowProcs() 
{
	CRefPhysProcsDlg dlg(this);
	dlg.m_nRefPhysID = m_id;
	dlg.DoModal();
}

void CContactsGeneral::OnKillFocusDateOfHire() 
{
	Save(IDC_DATE_OF_HIRE);
}

// (a.walling 2007-11-21 15:11) - PLID 28157
void CContactsGeneral::OnKillFocusDateOfTerm() 
{
	Save(IDC_DATE_OF_TERM);
}

LRESULT CContactsGeneral::OnTableChanged(WPARAM wParam, LPARAM lParam) 
{
	try {

	}NxCatchAll("Error in CContactsGeneral::OnTableChanged");

	return 0;
}

void CContactsGeneral::OnReferredPats() 
{
	try {
		// (m.hancock 2006-08-02 14:20) - PLID 20917 - Display the referred patients
		CReferredPatients dlg(this);
		dlg.m_nPersonID = m_id;
		dlg.m_nStatus = 1;
		dlg.m_bRefPhys = true;
		dlg.DoModal();
	}NxCatchAll("Error in CContactsGeneral::OnReferredPats");
}

void CContactsGeneral::OnReferredProspects() 
{
	try {
		// (m.hancock 2006-08-02 14:20) - PLID 20917 - Display the referred prospects
		CReferredPatients dlg(this);
		dlg.m_nPersonID = m_id;
		dlg.m_nStatus = 2;
		dlg.m_bRefPhys = true;
		dlg.DoModal();
	}NxCatchAll("Error in CContactsGeneral::OnReferredProspects");
}

void CContactsGeneral::OnSelChosenClaimProvider(LPDISPATCH lpRow) 
{
	try {
	
		// (j.jones 2006-12-01 09:49) - PLID 22110 - added this functionality

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		// (a.wilson 2013-04-01 13:48) - PLID 55687 - no longer necessary. this should never reach a null row but assert just in case.
		if(pRow == NULL) {
			ASSERT(FALSE);
			return;
			//not allowed, reload the old value
			//Load();
		}

		long nClaimProviderID = VarLong(pRow->GetValue(0));

		//if the IDs differ, warn them of the implications
		if(nClaimProviderID != m_id) {

			if(IDNO == DontShowMeAgain(this, "If you change the Claim Provider, any insurance claims generated for this.\n"
				"provider will report the Claim Provider as the billing/rendering provider,\n"
				"using the Claim Provider's IDs, name, address, etc.\n\n"
				"Are you sure you wish to make this change?", "ChangingClaimProvider", "Practice", FALSE, TRUE)) {
				//load the old value
				Load();
				return;
			}
		}

		//for auditing
		CString strOldProvider, strNewProvider;
		long nOldID = -1;
		_RecordsetPtr rsProv = CreateRecordset("SELECT ID, Last + ', ' + First + ' ' + Middle + ' ' + Title AS Name FROM PersonT WHERE ID IN (SELECT ClaimProviderID FROM ProvidersT WHERE PersonID = %li)", m_id);
		if(!rsProv->eof) {
			nOldID = AdoFldLong(rsProv, "ID",-1);
			strOldProvider = AdoFldString(rsProv, "Name", "");
		}
		rsProv->Close();
		strNewProvider = VarString(pRow->GetValue(1),"");

		ExecuteSql("UPDATE ProvidersT SET ClaimProviderID = %li WHERE PersonID = %li", nClaimProviderID, m_id);
		
		//now audit
		if(nOldID != nClaimProviderID) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, GetActiveContactName(), nAuditID, aeiClaimProvider, m_id, strOldProvider, strNewProvider, aepHigh, aetChanged);
		}
	
	}NxCatchAll("CContactsGeneral::OnSelChosenClaimProvider");
}

void CContactsGeneral::OnRequeryFinishedClaimProvider(short nFlags) 
{
	try {


	
	}NxCatchAll("CContactsGeneral::OnRequeryFinishedClaimProvider");
}

void CContactsGeneral::OnTrySetSelFinishedClaimProvider(long nRowEnum, long nFlags) 
{
	try {

		if(nFlags == dlTrySetSelFinishedFailure && m_PendingClaimProvID != -1) {
			// (j.jones 2006-12-01 10:33) - PLID 22110 - must be an inactive provider
			_RecordsetPtr rsProv = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle + ' ' + Title AS Name FROM PersonT WHERE ID = %li", m_PendingClaimProvID);
			if(!rsProv->eof) {
				m_ClaimProviderCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
			}
		}
	
	}NxCatchAll("CContactsGeneral::OnTrySetSelFinishedClaimProvider");
}

// (z.manning, 06/07/2007) - PLID 23862 - Open the dialog with the rich text control to edit biography text.
void CContactsGeneral::OnEditBiography()
{
	try 
	{
		CContactBiographyDlg dlg(m_id, this);
		dlg.DoModal();
		
	}NxCatchAll("CContactsGeneral::OnEditBiography");
}

// (z.manning, 06/07/2007) - PLID 23862 - Opens the image select dialog so the user can select an image for the current provider.
void CContactsGeneral::OnSelectImage()
{
	try
	{
		CContactSelectImageDlg dlg(m_id, this);
		dlg.DoModal();

	}NxCatchAll("CContactsGeneral::OnSelectImage");
}

// (z.manning, 12/12/2007) - PLID 28216 - Added an attendance setup button to contacts' general tab.
void CContactsGeneral::OnAttendanceSetup()
{
	CWaitCursor wc;
	try
	{
		// (z.manning, 12/06/2007) - PLID 28295 - Check permission
		if(!ReturnsRecords("SELECT UserID FROM DepartmentManagersT WHERE UserID = %li", GetCurrentUserID())) {
			if(!CheckCurrentUserPermissions(bioAttendance, sptDynamic0)) {
				return;
			}
		}

		AttendanceInfo info;
		info.LoadAllByYear(COleDateTime::GetCurrentTime().GetYear());

		CAttendanceUserSetupDlg dlg(&info, this);
		dlg.SetDefaultUserID(m_id);
		dlg.DoModal();

	}NxCatchAll("CContactsGeneral::OnAttendanceSetup");
}

void CContactsGeneral::OnTrySetSelFinishedProviderlocationCombo(long nRowEnum, long nFlags) 
{
	try {
		if(nFlags == dlTrySetSelFinishedFailure) {
			//TES 9/8/2008 - PLID 27727 - OK, they must have an inactive location selected, pull the name from data.
			_RecordsetPtr rsLocation = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = (SELECT Location FROM PersonT WHERE ID = %li)", m_id);
			if(!rsLocation->eof) {
				m_LocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLocation, "Name", "")));
			}
		}
	}NxCatchAll("Error in CContactsGeneral::OnTrySetSelFinishedProviderlocationCombo()");
}

void CContactsGeneral::OnSelChosenAMASpecialtyList(LPDISPATCH lpRow)
{
	try {
		_variant_t varNewValue;
		varNewValue.vt = VT_NULL;

		int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();

		//for audit
		CString strOld, strNew;

		if(lpRow != NULL) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			//get the value of the ID column into our variant
			varNewValue = pRow->GetValue(0);
			if(varNewValue.vt == VT_I4) {
				//for auditing, get the name if we have a valid row (not the {None Selected} row)
				strNew = VarString(pRow->GetValue(2), "");
			}
		}

		// (z.manning 2009-05-05 09:06) - PLID 26074 - This may now be a referring physician or a provider.
		CString strTableToUpdate;
		if(nStatus == 1) {
			strTableToUpdate = "ReferringPhysT";
		}
		else {
			strTableToUpdate = "ProvidersT";
		}

		//save the new selection, and get auditing from old at the same time
		_RecordsetPtr prsAudit = CreateParamRecordset(FormatString(
			"SELECT AMASpecialtyListT.Description FROM %s "
			"	LEFT JOIN AMASpecialtyListT ON %s.AMASpecialtyID = AMASpecialtyListT.ID "
			"	WHERE %s.PersonID = {INT};\r\n"
			"SET NOCOUNT ON;\r\n"
			"UPDATE %s SET AMASpecialtyID = {VT_I4} WHERE PersonID = {INT};"
			, strTableToUpdate, strTableToUpdate, strTableToUpdate, strTableToUpdate)
			, m_id, varNewValue, m_id);

		if(prsAudit->eof) {
			//not possible
			AfxThrowNxException("Unable to retrieve specialty description for auditing after save.");
		}

		strOld = AdoFldString(prsAudit, "Description", "");

		// (z.manning 2009-05-05 09:11) - PLID 26074 - Don't audit if nothing changed.
		if(strOld != strNew) {
			//Audit
			AuditField(aeiAMASpecialty, strOld, strNew);
			// (v.maida 2014-12-23 12:19) - PLID 64472 - Add ref phys HL7 update, if applicable.
			if (nStatus & 0x1) {
				AddOrUpdateRefPhysInHL7(m_id, false);
			}
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.gruber 2011-09-22 10:54) - PLID 45354
void CContactsGeneral::OnBnClickedAffiliatePhys()
{	
	try {

		int nCheck = IsDlgButtonChecked(IDC_AFFILIATE_PHYS)  ? 1 : 0;		
		CString strChecked = IsDlgButtonChecked(IDC_AFFILIATE_PHYS)  ? "Checked" : "Not Checked";		

		CString strOldChecked = "Not Checked";
		_RecordsetPtr rsChecked = CreateParamRecordset("SELECT AffiliatePhysician FROM ReferringPhysT WHERE PersonID = {INT}", m_id);
		if (! rsChecked->eof) {
			if (AdoFldBool(rsChecked, "AffiliatePhysician")) {
				strOldChecked = "Checked";
			}
			else {
				strOldChecked = "Not Checked";
			}
		}
		else {
			//we should never get here
			ASSERT(FALSE);
		}
		
		ExecuteParamSql("UPDATE ReferringPhysT SET AffiliatePhysician = {INT} WHERE PersonID = {INT}", nCheck, m_id);

		if (strChecked != strOldChecked) {
			AuditField(aeiAffiliatePhysician, strOldChecked, strChecked);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-11-07 14:23) - PLID 46299 - added ProvidersT.UseCompanyOnClaims
void CContactsGeneral::OnSendCompanyOnClaim()
{	
	try {

		if(m_checkSendCompanyOnClaim.GetCheck()) {
			CString strCompany;
			GetDlgItemText(IDC_EMPLOYER_BOX, strCompany);
			strCompany.TrimLeft();
			strCompany.TrimRight();
			if(strCompany.IsEmpty()) {
				//warn that this will do nothing
				AfxMessageBox("The current provider does not have a Company entered. This setting will be ignored if the Company field is blank.");
			}
		}

		int nCheck = m_checkSendCompanyOnClaim.GetCheck() ? 1 : 0;
		
		ExecuteParamSql("UPDATE ProvidersT SET UseCompanyOnClaims = {INT} WHERE PersonID = {INT}", nCheck, m_id);

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, GetActiveContactName(), nAuditID, aeiProviderUseCompanyOnClaims, GetActiveContactID(), nCheck ? "Disabled" : "Enabled", nCheck ? "Enabled" : "Disabled", aepMedium, aetChanged);

		SendContactsTablecheckerMsg();

	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2012-04-05 16:17) - PLID 49332 - optician field!
void CContactsGeneral::OnBnClickedOpticianCheck()
{
	try {
		long nType;
		if(m_btnOptician.GetCheck()) {
			nType = 1;
		}else {
			nType = 0;
		}

		ExecuteParamSql("UPDATE ProvidersT SET Optician = {INT} WHERE PersonID = {INT}", nType, m_id);

		if(nType == 0) {
			// (j.dinatale 2012-04-09 18:34) - PLID 49332 - prompt to let them know that the user is associated with glasses orders
			if(ReturnsRecordsParam("SELECT TOP 1 1 FROM GlassesOrderT WHERE OpticianID = {INT}", m_id)) {
				AfxMessageBox("This provider is associated with at least one Optical Order as an Optician.\r\n\r\nBy unselecting this "
						"provider as an Optician, they will no longer be available for selection on future Optical Orders.");
			}
		}

		//We need to follow suit and audit this as well
		long nID = BeginNewAuditEvent();
		CString strNew, strName, str;
		strNew.Format("%li", nType);

		GetDlgItemText(IDC_LAST_NAME_BOX, str);
		strName += str;
		strName += ", ";
		GetDlgItemText(IDC_FIRST_NAME_BOX, str);
		strName += str;
		strName += " ";
		GetDlgItemText(IDC_MIDDLE_NAME_BOX, str);
		strName += str;

		AuditEvent(-1, strName, nID, aeiOpticianStatus, m_id, (nType == 0 ? "1" : "0"), strNew, aepMedium, aetChanged);

		if(nID != -1){
			CommitAuditTransaction(nID);
		}

		SendContactsTablecheckerMsg();
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-03-26 14:16) - PLID 55687 - ensure you cannot select a null row for claim provider.
void CContactsGeneral::SelChangingClaimProvider(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-04-23 13:56) - PLID 56409 - Utility
BOOL CContactsGeneral::IsProviderConfiguredForNexERx()
{
	return m_bIsConfiguredNexERxPrescriber;
}

// (b.savon 2013-06-06 15:50) - PLID 56867 - Aggregate the data and pass it to the registration
// dialog
void CContactsGeneral::OnBnClickedBtnRegisterPrescriber()
{
	try{
		/* Fill Prescriber Info */
		NexERxPrescriber prescriber;
		CString strID;
		strID.Format("%li", m_id);
		prescriber.strID = strID;
		m_nxeditFirstNameBox.GetWindowTextA(prescriber.strFirst);
		m_nxeditLastNameBox.GetWindowTextA(prescriber.strLast);
		m_nxeditMiddleNameBox.GetWindowTextA(prescriber.strMiddle);
		m_nxeditNpiBox.GetWindowTextA(prescriber.strNPI);
		m_nxeditLicenseBox.GetWindowTextA(prescriber.strStateLicense);
		// (b.savon 2013-08-02 14:43) - PLID 57747 - DEA
		m_nxeditDeaBox.GetWindowTextA(prescriber.strDEA);
		m_nxeditEmailBox.GetWindowTextA(prescriber.strEmail);
		m_nxeditWorkPhoneBox.GetWindowTextA(prescriber.strWork);
		m_nxeditExtPhoneBox.GetWindowTextA(prescriber.strExt);
		m_nxeditFaxBox.GetWindowTextA(prescriber.strFax);

		/* Show the dialog */
		CNexERxRegisterPrescriberDlg dlg(prescriber);
		dlg.DoModal();

		if( dlg.IsRegistered() ){
			UpdateView();
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.spivey -- October 16th, 2013) - PLID 59022 - If they opened this dialog we need to update the user 
//		list of who has access to use this Direct Address.
void CContactsGeneral::OnBnClickedDirectAddressUsersButton()
{
	try {

		//Active users only. 
		CMultiSelectDlg dlg(this, "UsersT");
		CArray<long, long> aryUserIDs; 
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON " 
			"	"
			"DECLARE @PersonID INT "
			"SET @PersonID = {INT} "
			"	"
			"SELECT ID AS DAFTID "
			"FROM DirectAddressFromT "
			"WHERE PersonID = @PersonID "
			"	"
			"SELECT DAUT.UserID, DAFT.ID AS DAFTID "
			"FROM DirectAddressUserT DAUT "
			"INNER JOIN PersonT PT ON DAUT.UserID = PT.ID "
			"INNER JOIN DirectAddressFromT DAFT ON DAUT.DirectAddressFromID = DAFT.ID "
			"WHERE DAFT.PersonID = @PersonID AND PT.Archived = 0 "
			"SET NOCOUNT OFF " , m_id);

		long nDirectAddressFromID = -1;
		if(!prs->eof) {
			nDirectAddressFromID = AdoFldLong(prs->Fields, "DAFTID", -1); 
			prs = prs->NextRecordset(NULL); 
			while (!prs->eof) {
				aryUserIDs.Add(AdoFldLong(prs->Fields, "UserID"));
				prs->MoveNext(); 
			} 

			//Select all the IDs in data. 
			dlg.PreSelect(aryUserIDs);
		}

		//Open the multi select dialog. 
		if (dlg.Open("UsersT UT INNER JOIN PersonT PT ON UT.PersonID = PT.ID ", "PT.Archived = 0 AND PT.ID > 0 ", "UT.PersonID", "UT.UserName", 
			"Please select the users you want to have access to this Direct Address.") == IDOK) {
				aryUserIDs.RemoveAll();
				dlg.FillArrayWithIDs(aryUserIDs);
				CSqlFragment sql("");
				
				for (int i = 0; i < aryUserIDs.GetCount(); i++) {
					sql += CSqlFragment("INSERT INTO DirectAddressUserT (UserID, DirectAddressFromID) \r\n"
						"VALUES ({INT}, {INT}) \r\n ", aryUserIDs.GetAt(i), nDirectAddressFromID);
				}

				//Delete all the entries for this provider and then reinsert them. 
				ExecuteParamSql("DELETE FROM DirectAddressUserT WHERE DirectAddressFromID = {INT} \r\n"
					"{SQL} \r\n", nDirectAddressFromID, sql); 
		}

	}NxCatchAll(__FUNCTION__);
}


// (b.spivey -- October 24th, 2013) - PLID 59022 - Add a function for the direct address button. 
void CContactsGeneral::UpdateDirectAddressButton()
{
	CString strDirectAddress;
	GetDlgItemText(IDC_DIRECT_ADDRESS_EDIT, strDirectAddress);

	if(!strDirectAddress.IsEmpty()) {
		GetDlgItem(IDC_DIRECT_ADDRESS_USERS_BUTTON)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_DIRECT_ADDRESS_USERS_BUTTON)->EnableWindow(FALSE);
	}

}

// (a.wilson 2014-04-21) PLID 61816 - update referring provider if checked.
void CContactsGeneral::OnBnClickedContactReferringProviderCheck()
{
	try {
		UpdateProviderType("ReferringProvider", (IsDlgButtonChecked(IDC_CONTACT_REFERRING_PROVIDER_CHECK) ? 1 : 0));
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-04-21) PLID 61816 - update ordering provider if checked.
void CContactsGeneral::OnBnClickedContactOrderingProviderCheck()
{
	try {
		UpdateProviderType("OrderingProvider", (IsDlgButtonChecked(IDC_CONTACT_ORDERING_PROVIDER_CHECK) ? 1 : 0));
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-04-21) PLID 61816 - update supervising provider if checked.
void CContactsGeneral::OnBnClickedContactSupervisingProviderCheck()
{
	try {
		UpdateProviderType("SupervisingProvider", (IsDlgButtonChecked(IDC_CONTACT_SUPERVISING_PROVIDER_CHECK) ? 1 : 0));
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-04-22) PLID 61816 - function to update based on which contact type and which provider type was changed.
void CContactsGeneral::UpdateProviderType(const CString& strField, const long& nNewValue)
{
	//New Value
	CString strNewValue = (nNewValue == 1 ? "Checked" : "Not Checked");

	//Contact Type
	CString strTable = "";
	int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();
	switch (nStatus) {
		case 0x1:
			strTable = "ReferringPhysT";
			break;
		case 0x2:
			strTable = "ProvidersT";
			break;
		default:
			ASSERT(FALSE);	//should not be possible.
			return;
	}

	//Old Value
	CString strOldValue = "Not Checked";
	_RecordsetPtr rsOldValue = CreateParamRecordset("SELECT {CONST_STRING} FROM {CONST_STRING} WHERE PersonID = {INT}", 
		strField, strTable, m_id);
	if (!rsOldValue->eof) {
		if (AdoFldBool(rsOldValue, strField)) {
			strOldValue = "Checked";
		}
		else {
			strOldValue = "Not Checked";
		}
	}
	else {
		//we should never get here
		ASSERT(FALSE);
	}

	//Update
	ExecuteParamSql("UPDATE {CONST_STRING} SET {CONST_STRING} = {INT} WHERE PersonID = {INT}", 
		strTable, strField, nNewValue, m_id);

	//Audit
	if (strOldValue != strNewValue) {
		
		if (strField == "ReferringProvider") {
			AuditField((nStatus == 0x1 ? aeiRefPhysReferringProvider : aeiProviderReferringProvider), strOldValue, strNewValue);
		} else if (strField == "OrderingProvider") {
			AuditField((nStatus == 0x1 ? aeiRefPhysOrderingProvider : aeiProviderOrderingProvider), strOldValue, strNewValue);
		} else if (strField == "SupervisingProvider") {
			AuditField((nStatus == 0x1 ? aeiRefPhysSupervisingProvider : aeiProviderSupervisingProvider), strOldValue, strNewValue);
		} else {
			//should never reach here.
			ASSERT(FALSE);
		}
	}
}
void CContactsGeneral::OnBnClickedContactConfigureProviderTypesButton()
{
	// TODO: Add your control notification handler code here
	//(r.wilson 4/22/2014) PLID 61826 -
	try{

		 // (s.tullis 2014-05-28 12:41) - PLID 61827 - Check if contact type is correct and pass it to dialog
		// (s.tullis 2014-05-28 12:46) - PLID 61826 - Check if contact type is correct and pass it to dialog
			CConfigureProviderTypesDlg dlg;	
			if (m_strContactType == "Referring Physician" || m_strContactType == "Provider"){
				dlg.strCurSelContactType = m_strContactType;
			}
			else{
				//No other contact type diplays this button in general
			}
			dlg.DoModal();
			UpdateView(TRUE);
			CClient::RefreshTable(NetUtils::ContactsT);

	}NxCatchAll(__FUNCTION__);
}


// (s.tullis 2015-10-29 17:27) - PLID 67483 - Show/Hide Capitation
void CContactsGeneral::ShowCapitation(BOOL bEnable /*=TRUE*/)
{
	try {

		bEnable = bEnable && (GetRemotePropertyInt("BatchPayments_EnableCapitation", 0, 0, "<None>", true) == 1 ? TRUE : FALSE);
		m_editCaptitationDistribution.ShowWindow(bEnable ? SW_SHOW : SW_HIDE);
		m_nxstaticCapitationDistribution.ShowWindow(bEnable ? SW_SHOW : SW_HIDE);
		m_nxstaticCapitationDistributionPercentsign.ShowWindow(bEnable ? SW_SHOW : SW_HIDE);

	}NxCatchAll(__FUNCTION__)
}
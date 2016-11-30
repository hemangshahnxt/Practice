
// NewPatient.cpp : implementation file
//
#include "stdafx.h"
#include "NewPatient.h"
#include "duplicate.h"
#include "mirrorLink.h"
#include "globalutils.h"
#include "MsgBox.h"
#include "Mirror.h"
#include "AddProcedureDlg.h"
#include "PhaseTracking.h"
#include "contacts.h"
#include "PatientsRc.h"
#include "AuditTrail.h"
#include "PreferenceUtils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
//#include "EditComboBox.h" // (b.spivey, May 14, 2012) - PLID 50224 - deprecated
#include "Mirror.h"
#include "InquiryDlg.h"
#include "HL7Utils.h"
#include "referraltreedlg.h"
#include "EditFamilyDlg.h" // for FamilyUtils;: namespace
#include "OPOSMSRDevice.h"
#include "dontshowdlg.h"
#include "Rewards.h"
#include "NewPatientMatchInquiry.h"
#include "GlobalNexWebUtils.h"
#include "OHIPUtils.h"
#include "GlobalInsuredPartyUtils.h"
#include "WellnessDataUtils.h"
#include "EditInsInfoDlg.h"
#include "AlbertaHLINKUtils.h"
#include "OPOSBarcodeScanner.h"	//(a.wilson 2012-1-11) PLID 47485
#include "EditPatientTypeDlg.h" // (b.spivey, May 14, 2012) - PLID 50224 - New dialog to replace old combo dialog.
#include <NxHL7Lib/HL7DataUtils.h>
#include "HL7Client_Practice.h" // (z.manning 2013-05-20 11:07) - PLID 56777 - Renamed

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NXTIMELib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CNewPatient dialog

//(e.lally 2008-08-29) - PLID 16395 - Copied from General1 - Preference types are hardcoded.
enum PreferredContactType
{
	pctNone=0,
	pctHomePhone,
	pctWorkPhone,
	pctCellPhone,
	pctPager,
	pctOtherPhone,
	pctEmail,
	pctTextMessaging,// (a.vengrofski 2010-02-01 13:08) - PLID <34208> - Added for the text messaging
};

// (s.dhole 2013-06-05 10:48) - PLID 12018 Location dropdown
enum LocationColumns
{
	pctcID = 0,
	pctcName = 1,
};

// (c.haag 2010-10-04 10:16) - PLID 39447 - Insurance-related list column enumerations
enum InsuranceListColumn {
	ilcName = 0,			// Company name
	ilcPlacement,	// Placement name
};

// (j.jones 2012-11-12 13:38) - PLID 53622 - added an enum for the country dropdown
enum CountryComboColumn {
	cccID = 0,
	cccName,
	cccISOCode,
};

// (j.jones 2016-02-25 15:52) - PLID 67549 - added enums for the Gender combo
enum GenderComboColumn {
	gccID = 0,
	gccGenderName,
};

CString CNewPatient::GetPreferredContactTypeStringFromID(long nID)
{
	switch(nID) {
		//These are all hardcoded
	case -1:
	case pctNone:
		return "<No Preference>";
		break;
	case pctHomePhone:
		return "Home Phone";
		break;
	case pctWorkPhone:
		return "Work Phone";
		break;
	case pctCellPhone:
		return "Cell Phone";
		break;
	case pctPager:
		return "Pager";
		break;
	case pctOtherPhone:
		return "Other Phone";
		break;
	case pctEmail:
		return "Email";
		break;
	case pctTextMessaging:// (a.vengrofski 2010-02-01 13:10) - PLID <34208> - Added for the "text messaging"
		return "Text Messaging";// (a.vengrofski 2010-02-01 13:10) - PLID <34208> 
		break;// (a.vengrofski 2010-02-01 13:10) - PLID <34208> 
	default:
		return "";
		break;
	}
}

// (j.gruber 2010-01-11 13:14) - PLID 36140 - added referralID to Select
CNewPatient::CNewPatient(CWnd* pParent,
						 UINT nIDTemplate, long nReferralIDToSelect /*=-1*/)
	: CNxDialog(nIDTemplate, pParent),
	m_groupChecker(NetUtils::Groups)
	, m_dlgGroups(this)
{
	//{{AFX_DATA_INIT(CNewPatient)
	//}}AFX_DATA_INIT
	
	m_id = NULL;
	m_nOriginalUserDefinedID = 0;
	m_pAddProDlg = NULL;
	m_nIDTemplate = nIDTemplate;
	m_bForGC = false;
	m_bFromReassign = false;
	m_bFromResEntry = false;
	m_pdlgReferralSubDlg = NULL;
	m_pMultiReferralList = NULL;
	m_nPrimaryMultiReferralID = -1;
	m_bEmailDeclined = FALSE;
	// (j.gruber 2010-01-11 13:23) - PLID 36140 - default referral ID
	m_nDefaultReferralID = nReferralIDToSelect;
	m_nHealthNumberCustomField = 1;
	m_nVersionCodeCustomField = 2;
	//TES 8/13/2014 - PLID 63194 - Added tracking for the prospect status, in case our caller is curious
	m_bIsProspect = true;
	// (r.farnworth 2016-03-03 10:47) - PLID 68454 - Edit the New Patient dialog so that it can be preloaded with patient information when it's launched.
	m_strPreselectFirst = "";
	m_strPreselectLast = "";
	m_strPreselectGender = "";
	m_strPreselectEmail = "";
	m_strPreselectAddress1 = "";
	m_strPreselectAddress2 = "";
	m_strPreselectCity = "";
	m_strPreselectState = "";
	m_strPreselectZip = "";
	m_strPreselectHomePhone = "";
	m_strPreselectCellPhone = "";

	COleDateTime dtInvalid;
	dtInvalid.SetStatus(COleDateTime::invalid);
	m_dtPreselectBirthdate = dtInvalid;
}

BOOL CNewPatient::DestroyWindow() 
{
	if(m_pdlgReferralSubDlg != NULL) {
		m_pdlgReferralSubDlg->DestroyWindow();
		delete m_pdlgReferralSubDlg;
		m_pdlgReferralSubDlg = NULL;
	}

	return CNxDialog::DestroyWindow();
}

CNewPatient::~CNewPatient()
{
	if (m_pAddProDlg)
		delete m_pAddProDlg;
}

void CNewPatient::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewPatient)
	DDX_Control(pDX, IDC_CHECK_MULTIPLE_REFERRAL_SOURCES, m_btnMultipleReferrals);
	DDX_Control(pDX, IDC_SAVE_AND_EDIT, m_editButton);
	DDX_Control(pDX, IDC_SAVE_AND_RESUME, m_resumeButton);
	DDX_Control(pDX, IDCANCEL, m_cancelButton);
	DDX_Control(pDX, IDC_SAVE_AND_SCHEDULE, m_scheduleButton);
	DDX_Control(pDX, IDC_SAVE_AND_ADD_ANOTHER, m_addanotherBtn);
	DDX_Control(pDX, IDC_SAVE_AND_FFA, m_ffaBtn);
	DDX_Control(pDX, IDC_HL7, m_hl7);
	DDX_Control(pDX, IDC_INFORM, m_inform);
	DDX_Control(pDX, IDC_MIRROR, m_mirror);
	DDX_Control(pDX, IDC_UNITED, m_united);
	DDX_Control(pDX, IDC_PATIENT_RAD,	m_patient);
	DDX_Control(pDX, IDC_PROSPECT_RAD,	m_prospect);
	DDX_Control(pDX, IDC_INQUIRY, m_inquiry);
	DDX_Control(pDX, IDC_GROUPS, m_group);
	DDX_Control(pDX, IDC_ID_BOX, m_nxeditIdBox);
	DDX_Control(pDX, IDC_FIRST_NAME_BOX, m_nxeditFirstNameBox);
	DDX_Control(pDX, IDC_MIDDLE_NAME_BOX, m_nxeditMiddleNameBox);
	DDX_Control(pDX, IDC_LAST_NAME_BOX, m_nxeditLastNameBox);
	DDX_Control(pDX, IDC_ADDRESS1_BOX, m_nxeditAddress1Box);
	DDX_Control(pDX, IDC_ADDRESS2_BOX, m_nxeditAddress2Box);
	DDX_Control(pDX, IDC_ZIP_BOX, m_nxeditZipBox);
	DDX_Control(pDX, IDC_CITY_BOX, m_nxeditCityBox);
	DDX_Control(pDX, IDC_STATE_BOX, m_nxeditStateBox);
	DDX_Control(pDX, IDC_SSN_BOX, m_nxeditSsnBox);
	DDX_Control(pDX, IDC_HOME_PHONE_BOX, m_nxeditHomePhoneBox);
	DDX_Control(pDX, IDC_WORK_PHONE_BOX, m_nxeditWorkPhoneBox);
	DDX_Control(pDX, IDC_EXT_PHONE_BOX, m_nxeditExtPhoneBox);
	DDX_Control(pDX, IDC_CELL_PHONE_BOX, m_nxeditCellPhoneBox);
	DDX_Control(pDX, IDC_EMAIL_BOX, m_nxeditEmailBox);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_EMAIL_REQ, m_nxlabelEmailReq);
	DDX_Control(pDX, IDC_PREFIX_REQ, m_nxstaticPrefixReq);
	DDX_Control(pDX, IDC_PATIENT_TYPE_REQ, m_nxstaticPatientTypeReq);
	DDX_Control(pDX, IDC_FIRST_NAME_REQ, m_nxstaticFirstNameReq);
	DDX_Control(pDX, IDC_LAST_NAME_REQ, m_nxstaticLastNameReq);
	DDX_Control(pDX, IDC_ADDRESS1_REQ, m_nxstaticAddress1Req);
	DDX_Control(pDX, IDC_CITY_REQ, m_nxstaticCityReq);
	DDX_Control(pDX, IDC_STATE_REQ, m_nxstaticStateReq);
	DDX_Control(pDX, IDC_ZIP_REQ, m_nxstaticZipReq);
	DDX_Control(pDX, IDC_SSN_REQ, m_nxstaticSsnReq);
	DDX_Control(pDX, IDC_DOB_REQ, m_nxstaticDobReq);
	DDX_Control(pDX, IDC_DOCTOR_REQ, m_nxstaticDoctorReq);
	DDX_Control(pDX, IDC_COORDINATOR_REQ, m_nxstaticCoordinatorReq);
	DDX_Control(pDX, IDC_REF_PHYS_REQ, m_nxstaticRefPhysReq);
	DDX_Control(pDX, IDC_NP_AFFILIATE_PHYS_REQUIRE_LBL, m_nxStaticAffiliatePhysReq);
	DDX_Control(pDX, IDC_REF_PAT_REQ, m_nxstaticRefPatReq);
	DDX_Control(pDX, IDC_GENDER_REQ, m_nxstaticGenderReq);
	DDX_Control(pDX, IDC_HOME_PHONE_REQ, m_nxstaticHomePhoneReq);
	DDX_Control(pDX, IDC_WORK_PHONE_REQ, m_nxstaticWorkPhoneReq);
	DDX_Control(pDX, IDC_CELL_PHONE_REQ, m_nxstaticCellPhoneReq);
	DDX_Control(pDX, IDC_NOTE_REQ, m_nxstaticNoteReq);
	DDX_Control(pDX, IDC_MIDDLE_NAME_REQ, m_nxstaticMiddleNameReq);
	DDX_Control(pDX, IDC_REFERRAL_REQ, m_nxstaticReferralReq);
	DDX_Control(pDX, IDC_PROCEDURE_REQ, m_nxstaticProcedureReq);
	DDX_Control(pDX, IDC_BTN_EDIT_PATIENT_TYPES, m_EditPtTypesBtn);
	DDX_Control(pDX, IDC_NEW_REF_PHYS, m_NewRefPhysBtn);
	DDX_Control(pDX, IDC_NEWPT_REFERRAL_AREA, m_btnNewptReferralArea);
	DDX_Control(pDX, IDC_NEWPAT_INS_ID_BOX, m_edtIDForInsurance);
	DDX_Control(pDX, IDC_NEWPAT_INS_FECA_BOX, m_edtInsPolGrouNum);
	DDX_Control(pDX, IDC_INS_INFO_LABEL, m_nxstaticInsInfoLabel);
	DDX_Control(pDX, IDC_INS_ID_LABEL, m_nxstaticIDForInsurance);
	DDX_Control(pDX, IDC_GROUP_NO_LABEL, m_nxstaticPolicyGroupNum);
	DDX_Control(pDX, IDC_LIST_INS_ADD, m_NewInsPartyBtn);
	DDX_Control(pDX, IDC_LIST_INS_EDIT, m_EditInsPartyBtn);
	DDX_Control(pDX, IDC_LIST_INS_DELETE, m_DeleteInsPartyBtn);
	DDX_Control(pDX, IDC_NEW_PATIENT_LOCATION, m_nxStaticLocationReq);
	
	
	//}}AFX_DATA_MAP
	if (!Is640x480Resource())
	{
		DDX_Control(pDX, IDC_PROCEDURES, m_procedureBtn);
		DDX_Control(pDX, IDC_HOME_PRIV_CHECK, m_homePriv);
		DDX_Control(pDX, IDC_WORK_PRIV_CHECK, m_workPriv);
	}
}



BEGIN_MESSAGE_MAP(CNewPatient, CNxDialog)
	//{{AFX_MSG_MAP(CNewPatient)
	ON_EN_CHANGE(IDC_EXT_PHONE_BOX, OnChangeExtPhoneBox)
	ON_EN_CHANGE(IDC_HOME_PHONE_BOX, OnChangeHomePhoneBox)
	ON_EN_CHANGE(IDC_WORK_PHONE_BOX, OnChangeWorkPhoneBox)
	ON_EN_CHANGE(IDC_ZIP_BOX, OnChangeZipBox)
	ON_EN_KILLFOCUS(IDC_ZIP_BOX, OnKillfocusZipBox)
	ON_EN_KILLFOCUS(IDC_CITY_BOX, OnKillfocusCityBox)
	ON_EN_KILLFOCUS(IDC_SSN_BOX, OnKillfocusSsnBox)
	ON_EN_CHANGE(IDC_FIRST_NAME_BOX, OnChangeFirstNameBox)
	ON_EN_CHANGE(IDC_MIDDLE_NAME_BOX, OnChangeMiddleNameBox)
	ON_EN_CHANGE(IDC_LAST_NAME_BOX, OnChangeLastNameBox)
	ON_EN_CHANGE(IDC_ADDRESS2_BOX, OnChangeAddress2Box)
	ON_EN_CHANGE(IDC_ADDRESS1_BOX, OnChangeAddress1Box)
	ON_EN_CHANGE(IDC_SSN_BOX, OnChangeSSN)
	ON_EN_CHANGE(IDC_CELL_PHONE_BOX, OnChangeCellPhoneBox)
	ON_BN_CLICKED(IDC_PROCEDURES, OnProcedures)
	ON_BN_CLICKED(IDC_SAVE_AND_SCHEDULE, OnSaveAndSchedule)
	ON_BN_CLICKED(IDC_SAVE_AND_EDIT, OnSaveAndEdit)
	ON_BN_CLICKED(IDC_SAVE_AND_RESUME, OnSaveAndResume)
	ON_BN_CLICKED(IDC_GROUPS, OnGroups)
	ON_BN_CLICKED(IDC_BTN_EDIT_PATIENT_TYPES, OnBtnEditPatientTypes)
	ON_BN_CLICKED(IDC_SAVE_AND_ADD_ANOTHER, OnSaveAndAddAnother)
	ON_BN_CLICKED(IDC_SAVE_AND_FFA, OnSaveAndFFA)
	ON_BN_CLICKED(IDC_INQUIRY, OnClickInquiry)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_NEW_REF_PHYS, OnNewRefPhys)
	ON_BN_CLICKED(IDC_CHECK_MULTIPLE_REFERRAL_SOURCES, OnCheckMultipleReferralSources)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_NEWPT_ADD_REFERRAL, OnNewptAddReferral)
	ON_BN_CLICKED(IDC_NEWPT_MAKE_PRIMARY_REFERRAL, OnNewptMakePrimaryReferral)
	ON_BN_CLICKED(IDC_NEWPT_REMOVE_REFERRAL, OnNewptRemoveReferral)
	ON_MESSAGE(WM_MSR_DATA_EVENT, OnMSRDataEvent)
	ON_MESSAGE(NXM_BARSCAN_DATA_EVENT, OnBarcodeScannerDataEvent)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_LIST_INS_ADD, OnBnClickedListInsAdd)
	ON_BN_CLICKED(IDC_LIST_INS_EDIT, OnBnClickedListInsEdit)
	ON_BN_CLICKED(IDC_LIST_INS_DELETE, OnBnClickedListInsDelete)
	ON_EN_KILLFOCUS(IDC_NEWPAT_INS_ID_BOX, OnEnKillfocusNewpatInsIdBox)
	ON_EN_KILLFOCUS(IDC_NEWPAT_INS_FECA_BOX, OnEnKillfocusNewpatInsFecaBox)
	ON_EN_KILLFOCUS(IDC_ID_BOX, OnKillFocusID)
	ON_BN_CLICKED(IDC_NEW_PROVIDER, &CNewPatient::OnNewProvider)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewPatient message handlers

BEGIN_EVENTSINK_MAP(CNewPatient, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNewPatient)
	ON_EVENT(CNewPatient, IDC_PT_PREFIX_LIST, 16 /* SelChosen */, OnSelChosenPrefix, VTS_I4)
	ON_EVENT(CNewPatient, IDC_GENDERLIST, 16 /* SelChosen */, OnSelChosenGender, VTS_I4)
	ON_EVENT(CNewPatient, IDC_REF_PATIENT_LIST, 16 /* SelChosen */, OnSelChosenRefPatientList, VTS_I4)
	ON_EVENT(CNewPatient, IDC_REF_PHYS_LIST, 16 /* SelChosen */, OnSelChosenRefPhysList, VTS_I4)
	ON_EVENT(CNewPatient, IDC_REF_PHYS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedRefPhysList, VTS_I2)
	ON_EVENT(CNewPatient, IDC_PATIENT_TYPE_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedPatientTypeCombo, VTS_I2)
	ON_EVENT(CNewPatient, IDC_DOCLIST, 18 /* RequeryFinished */, OnRequeryFinishedDoclist, VTS_I2)
	ON_EVENT(CNewPatient, IDC_COORDLIST, 18 /* RequeryFinished */, OnRequeryFinishedCoordlist, VTS_I2)
	ON_EVENT(CNewPatient, IDC_PT_PREFIX_LIST, 18 /* RequeryFinished */, OnRequeryFinishedPtPrefixList, VTS_I2)
	ON_EVENT(CNewPatient, IDC_REF_PATIENT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedRefPatientList, VTS_I2)
	ON_EVENT(CNewPatient, IDC_NEWPT_REFERRAL_LIST, 2 /* SelChanged */, OnSelChangedNewptReferralList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CNewPatient, IDC_RELATEDPAT, 1 /* SelChanging */, OnSelChangingRelatedPat, VTS_PI4)
	ON_EVENT(CNewPatient, IDC_LIST_INS_PARTIES, 3 /* DblClickCell */, OnDblClickCellInsList, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CNewPatient, IDC_NP_AFFILIATE_PHYS_LIST, 18, CNewPatient::RequeryFinishedNpAffiliatePhysList, VTS_I2)
	ON_EVENT(CNewPatient, IDC_NEW_PATIENT_LOCATION, 1, CNewPatient::SelChangingNewPatientLocation, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CNewPatient, IDC_DOB_BOX, 1, CNewPatient::KillFocusDobBox, VTS_NONE)
END_EVENTSINK_MAP()

BOOL CNewPatient::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		// (j.jones 2006-04-27 14:09) - PLID 20324 - Load all common New Patient properties into the
		// NxPropManager cache
		//(e.lally 2007-06-21) PLID 24334 - Add the NewPatientAdditionalFields property
		// (z.manning 2008-12-08 12:59) - PLID 32320 - Added RefPhysComboShow properties
		// (e.lally 2009-01-26) PLID 32813 - Added AssignNewPatientSecurityCode preference
		// (z.manning 2009-08-24 12:30) - PLID 31135 - Added NewPatientsCheckBirthDate
		// (j.gruber 2009-10-08 12:03) - PLID 35826 - added LookupZipStateByCity
		// (j.jones 2010-01-12 10:05) - PLID 31927 - added NewPatientsDefaultTextMessagePrivacy
		// (r.goldschmidt 2014-08-12 18:53) - PLID 20992 - added DefaultNewPatientsPatCoord
		g_propManager.CachePropertiesInBulk("NewPatient-1", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'NewPatientDefault' OR "
			"Name = 'DefaultProcedureType' OR "
			"Name = 'NewPatientRequiredFields' OR "
			"Name = 'FormatPhoneNums' OR "
			"Name = 'AllowSaveAndEditFurtherInScheduler' OR "
			"Name = 'DefaultICD9Code' OR "
			"Name = 'GenderPrefixLink' OR "
			"Name = 'DefaultMalePrefix' OR "
			"Name = 'DefaultFemalePrefix' OR "
			"Name = 'DefaultNeuterPrefix' OR "
			"Name = 'DefaultPatientReferral' OR "
			"Name = 'DefaultPhysicianReferral' OR "
			"Name = 'DefaultPatType' OR "
			"Name = 'DisableNewPatRefPat' OR "
			"Name = 'DefaultPatType' OR "
			"Name = 'DefaultGender' OR "
			"Name = 'DefaultPatientPrefix' OR "
			"Name = 'NewPatientAdditionalFields' OR "
			"Name = 'RefPhysComboShowNPI' OR "
			"Name = 'RefPhysComboShowID' OR "
			"Name = 'RefPhysComboShowUPIN' OR "
			"Name = 'AssignNewPatientSecurityCode' "
			// (d.thompson 2009-03-19) - PLID 33061
			"OR Name = 'DefaultInsuranceCoNewPatient' "
			"OR Name = 'DefaultInsurancePlanNewPatient' "
			"OR Name = 'SetDefaultInsCoOnNewPatients' "
			"OR Name = 'NewPatientsCheckBirthDate' "
			"OR Name = 'LookupZipStateByCity' "
			"OR Name = 'NewPatientsDefaultTextMessagePrivacy' "
			// (j.jones 2010-05-06 14:40) - PLID 38482 - cached OHIP data
			"OR Name = 'UseOHIP' "
			"OR Name = 'OHIP_HealthNumberCustomField' "
			"OR Name = 'OHIP_VersionCodeCustomField' "
			// (j.jones 2010-11-03 15:08) - PLID 39620 - added Alberta option
			"OR Name = 'UseAlbertaHLINK' "
			"OR Name = 'Alberta_PatientULICustomField' "
			//(c.copits 2011-01-18) PLID 27344 - Truncate trailing space on names and addresses
			"OR Name = 'TruncateTrailingNameAddressSpace' "
			// (j.jones 2011-06-24 16:45) - PLID 31005 - added AutoCapitalizeInsuranceIDs
			"OR Name = 'AutoCapitalizeInsuranceIDs' "
			"OR Name = 'DefaultNewPatientsPatCoord' "
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("NewPatient-2", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'InformDataPath' OR "
			"Name = 'MirrorDataPath' OR "
			"Name = 'UnitedDataPath' OR "
			"Name = 'PhoneFormatString' OR "
			// (j.jones 2011-06-24 12:24) - PLID 29885 - added DefaultMedicareSecondaryReasonCode
			"Name = 'DefaultMedicareSecondaryReasonCode' "
			")", _Q(GetCurrentUserName()));

		m_informPath = GetRemotePropertyText("InformDataPath", "", 0, "<None>");
		m_mirrorPath = GetRemotePropertyText("MirrorDataPath", "", 0, "<None>");
		m_unitedPath = GetRemotePropertyText("UnitedDataPath", "", 0, "<None>");

		// (d.singleton 2012-06-18 17:11) - PLID 51029 set text limit on address 1 and 2
		m_nxeditAddress1Box.SetLimitText(150);
		m_nxeditAddress2Box.SetLimitText(150);

		// (b.eyers 2015-04-14) - PLID 25917 - set text limit on first, middle, last, city, state, and zip
		m_nxeditFirstNameBox.SetLimitText(50);
		m_nxeditMiddleNameBox.SetLimitText(50);
		m_nxeditLastNameBox.SetLimitText(50);
		m_nxeditCityBox.SetLimitText(50);
		m_nxeditStateBox.SetLimitText(20);
		m_nxeditZipBox.SetLimitText(20);

		// (b.savon 2013-03-28 19:37) - PLID 55947 - Limit the email text to the database size
		m_nxeditEmailBox.SetLimitText(50);

		m_deInform.SetPath(m_informPath);
		m_deInform.TryAccess();
		if (Mirror::IsMirrorEnabled()) {
			// (c.haag 2009-03-31 13:33) - PLID 33630 - This variable is TRUE if we are using SDK functionality
			const BOOL bUsingSDKFunctionality = Mirror::IsUsingCanfieldSDK();
			if (!bUsingSDKFunctionality) {
				m_deMirror.SetPath(m_mirrorPath);
				m_deMirror.TryAccess();
			}
		}
		m_deUnited.SetPath(m_unitedPath);
		m_deUnited.TryAccess();

		// Make the font smaller if the screen is 640x480. TODO: Get
		// rid of this code and do it the RIGHT way!
		if (Is640x480Resource())
		{
			extern CPracticeApp theApp;
			CWnd* pWnd = GetNextDlgTabItem(NULL);
			CWnd* pFirstWnd = pWnd;

			while (pWnd)
			{
				if (IsWindow(pWnd->GetSafeHwnd()))
				{
					pWnd->SetFont(&theApp.m_smallFont);
				}
				pWnd = GetNextDlgTabItem(pWnd);

				if (pWnd == pFirstWnd)
					break;
			}

			pWnd = GetDlgItem(IDC_STATIC_NEWPT_FIRST);
			pFirstWnd = pWnd;
			while (pWnd)
			{
				if (IsWindow(pWnd->GetSafeHwnd()))
				{
					pWnd->SetFont(&theApp.m_smallFont);
				}
				pWnd = GetNextDlgGroupItem(pWnd);

				if (pWnd == pFirstWnd)
					break;
			}
		}
		
		// (c.haag 2008-04-25 12:28) - PLID 29790 - NxIconified buttons
		/*
		m_resumeButton.SetTextColor(0xFF0000);
		m_editButton.SetTextColor(0x008000);
		m_cancelButton.SetTextColor(0x0000FF);
		m_scheduleButton.SetTextColor(0xC08000);
		m_addanotherBtn.SetTextColor(0x333333);
		m_ffaBtn.SetTextColor(0xA0A000);*/
		m_resumeButton.AutoSet(NXB_OK);
		m_editButton.AutoSet(NXB_OK);
		m_cancelButton.AutoSet(NXB_CANCEL);
		m_scheduleButton.AutoSet(NXB_OK);
		m_addanotherBtn.AutoSet(NXB_OK);
		m_ffaBtn.AutoSet(NXB_OK);
		m_NewInsPartyBtn.AutoSet(NXB_NEW);
		m_EditInsPartyBtn.AutoSet(NXB_MODIFY);
		m_DeleteInsPartyBtn.AutoSet(NXB_DELETE);

		// (j.gruber 2009-10-13 09:27) - PLID 10723 - set the limit for ID for insurance and policy
		m_edtIDForInsurance.SetLimitText(50);
		m_edtInsPolGrouNum.SetLimitText(50);

		// (s.dhole 2013-06-04 13:00) - PLID 12018 loda location dropdown
		m_pDefaultLocation = BindNxDataList2Ctrl(IDC_NEW_PATIENT_LOCATION ,true);
		NXDATALIST2Lib::IRowSettingsPtr pNewRow;
		pNewRow = m_pDefaultLocation->GetNewRow();
		pNewRow->PutValue(pctcID, (long)-1);
		pNewRow->PutValue(pctcName, _bstr_t("<No Location Selected>"));
		m_pDefaultLocation->AddRowBefore(pNewRow, m_pDefaultLocation->GetFirstRow());

		

		// (j.gruber 2009-11-12 09:29) - PLID 10723 - disable the function if they have ohip
		// (c.haag 2010-10-04 16:30) - PLID 39447 - Deprecated insurance form fields
		// (j.jones 2010-11-04 16:27) - PLID 39620 - supported Alberta HLINK
		if(UseAlbertaHLINK()) {
			m_nHealthNumberCustomField = GetRemotePropertyInt("Alberta_PatientULICustomField", 1, 0, "<None>", true);
			m_nVersionCodeCustomField = -1;
			CString strHealthNumberName = "Health Number";
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Name FROM CustomFieldsT "
				"WHERE ID = {INT}", m_nHealthNumberCustomField);
			while(!rs->eof) {
				long nID = AdoFldLong(rs, "ID");
				if(nID == m_nHealthNumberCustomField) {
					strHealthNumberName = AdoFldString(rs, "Name");
				}
				rs->MoveNext();
			}
			rs->Close();

			m_nxstaticInsInfoLabel.SetWindowText("Alberta HLINK Information");
			GetDlgItem(IDC_LIST_INS_PARTIES)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_INS_ADD)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_INS_EDIT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_INS_DELETE)->ShowWindow(SW_HIDE);
			m_nxstaticIDForInsurance.SetWindowText(strHealthNumberName);
			m_nxstaticIDForInsurance.ShowWindow(SW_SHOW);
			m_nxstaticPolicyGroupNum.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_NEWPAT_INS_ID_BOX)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_NEWPAT_INS_FECA_BOX)->ShowWindow(SW_HIDE);
		}
		else if(UseOHIP()) 
		{
			// (j.jones 2010-05-06 14:39) - PLID 38482 - the ID For Ins & Group Number
			// remain enabled, but their labels change
			m_nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
			m_nVersionCodeCustomField = GetRemotePropertyInt("OHIP_VersionCodeCustomField", 2, 0, "<None>", true);
			CString strHealthNumberName = "Health Card Num.";
			CString strVersionCodeName = "Version Code";
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Name FROM CustomFieldsT "
				"WHERE ID = {INT} OR ID = {INT}", m_nHealthNumberCustomField, m_nVersionCodeCustomField);
			while(!rs->eof) {
				long nID = AdoFldLong(rs, "ID");
				if(nID == m_nHealthNumberCustomField) {
					strHealthNumberName = AdoFldString(rs, "Name");
				}
				else if(nID == m_nVersionCodeCustomField) {
					strVersionCodeName = AdoFldString(rs, "Name");
				}
				rs->MoveNext();
			}
			rs->Close();

			m_nxstaticInsInfoLabel.SetWindowText("OHIP Information");
			GetDlgItem(IDC_LIST_INS_PARTIES)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_INS_ADD)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_INS_EDIT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_INS_DELETE)->ShowWindow(SW_HIDE);
			m_nxstaticIDForInsurance.SetWindowText(strHealthNumberName);
			m_nxstaticIDForInsurance.ShowWindow(SW_SHOW);
			m_nxstaticPolicyGroupNum.SetWindowText(strVersionCodeName);
			m_nxstaticPolicyGroupNum.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_NEWPAT_INS_ID_BOX)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_NEWPAT_INS_FECA_BOX)->ShowWindow(SW_SHOW);
		}
		else {
			m_nxstaticInsInfoLabel.SetWindowText("Insurance Information");
			m_nxstaticIDForInsurance.ShowWindow(SW_HIDE);
			m_nxstaticPolicyGroupNum.ShowWindow(SW_HIDE);	
			GetDlgItem(IDC_NEWPAT_INS_ID_BOX)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_NEWPAT_INS_FECA_BOX)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_LIST_INS_PARTIES)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LIST_INS_ADD)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LIST_INS_EDIT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_LIST_INS_DELETE)->ShowWindow(SW_SHOW);
		}


		//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
		if(!g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent)) {
			m_scheduleButton.EnableWindow(FALSE);
			m_ffaBtn.EnableWindow(FALSE);
		}

		// Kill application hot-keys so that the local hot-keys work, and global keys don't
		GetMainFrame()->DisableHotKeys();

		// Set the default check boxes
		// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Prospect (2)
		if(GetRemotePropertyInt("NewPatientDefault",2,0,"<None>",true)==1)
			m_patient.SetCheck(true);
		else
			m_prospect.SetCheck(true);

	if (!Is640x480Resource())
	{
		m_pRefPhysList = BindNxDataListCtrl(IDC_REF_PHYS_LIST);
		// (z.manning 2008-12-08 12:39) - PLID 32320 - Show/hide columns in the ref phys list based on
		// the preferences to show them.
		if(GetRemotePropertyInt("RefPhysComboShowNPI", 1, 0, "<None>") == 0) {
			m_pRefPhysList->GetColumn(rpcNpi)->PutStoredWidth(0);
		}
		else {
			m_pRefPhysList->GetColumn(rpcNpi)->PutStoredWidth(80);
		}
		if(GetRemotePropertyInt("RefPhysComboShowID", 0, 0, "<None>") == 0) {
			m_pRefPhysList->GetColumn(rpcRefPhysID)->PutStoredWidth(0);
		}
		else {
			m_pRefPhysList->GetColumn(rpcRefPhysID)->PutStoredWidth(70);
		}
		if(GetRemotePropertyInt("RefPhysComboShowUPIN", 0, 0, "<None>") == 0) {
			m_pRefPhysList->GetColumn(rpcUpin)->PutStoredWidth(0);
		}
		else {
			m_pRefPhysList->GetColumn(rpcUpin)->PutStoredWidth(70);
		}

		// (r.gonet 05/16/2012) - PLID 48561 - Initialize the affilaite physician drop down
		m_pAffiliatePhysList = BindNxDataList2Ctrl(IDC_NP_AFFILIATE_PHYS_LIST);
		// (r.gonet 05/16/2012) - PLID 48561 - Follow the referring physician preferences, since this draws from the same list.
		if(GetRemotePropertyInt("RefPhysComboShowNPI", 1, 0, "<None>") == 0) {
			m_pAffiliatePhysList->GetColumn(afphNpi)->PutStoredWidth(0);
		}
		else {
			m_pAffiliatePhysList->GetColumn(afphNpi)->PutStoredWidth(80);
		}
		if(GetRemotePropertyInt("RefPhysComboShowID", 0, 0, "<None>") == 0) {
			m_pAffiliatePhysList->GetColumn(afphAffiliatePhysID)->PutStoredWidth(0);
		}
		else {
			m_pAffiliatePhysList->GetColumn(afphAffiliatePhysID)->PutStoredWidth(70);
		}
		if(GetRemotePropertyInt("RefPhysComboShowUPIN", 0, 0, "<None>") == 0) {
			m_pAffiliatePhysList->GetColumn(afphUpin)->PutStoredWidth(0);
		}
		else {
			m_pAffiliatePhysList->GetColumn(afphUpin)->PutStoredWidth(70);
		}

		m_PatientTypeCombo = BindNxDataListCtrl(IDC_PATIENT_TYPE_COMBO);
		m_pRelatedPatientList = BindNxDataListCtrl(IDC_RELATEDPAT, false);

		// (a.walling 2006-11-14 12:34) - PLID 22715
		m_pRelatedPatientList->Requery();
		NXDATALISTLib::IRowSettingsPtr pRow = m_pRelatedPatientList->GetRow(NXDATALISTLib::sriGetNewRow);
		pRow->PutValue(0, long(-1));
		pRow->PutValue(1, "<No Relation>");
		m_pRelatedPatientList->AddRow(pRow);
		m_pRelatedPatientList->TrySetSelByColumn(0, long(-1));
	}

		// Get a patient ID that will always work
		m_nOriginalUserDefinedID = NewNumber("PatientsT", "UserDefinedID");
		SetDlgItemInt(IDC_ID_BOX, m_nOriginalUserDefinedID);
		
		// Fill and default the Provider box
		// This can ask for the prac path without the filename on the end
		// because it will return the path with a backslash on the end
		m_pDocList = BindNxDataListCtrl(IDC_DOCLIST);

	    m_pCoordList = BindNxDataListCtrl(IDC_COORDLIST);

		//setup gender combo
		m_GenderCombo = BindNxDataListCtrl(IDC_GENDERLIST, false);
		// (j.jones 2016-02-25 15:52) - PLID 67549 - added enums for the Gender combo
		NXDATALISTLib::IRowSettingsPtr pRow = m_GenderCombo->GetRow(NXDATALISTLib::sriGetNewRow);
		pRow->PutValue(gccID, (long)0);
		pRow->PutValue(gccGenderName, _bstr_t(""));
		m_GenderCombo->AddRow(pRow);
		pRow = m_GenderCombo->GetRow(NXDATALISTLib::sriGetNewRow);
		pRow->PutValue(gccID, (long)1);
		pRow->PutValue(gccGenderName, _bstr_t("Male"));
		m_GenderCombo->AddRow(pRow);
		pRow = m_GenderCombo->GetRow(NXDATALISTLib::sriGetNewRow);
		pRow->PutValue(gccID, (long)2);
		pRow->PutValue(gccGenderName, _bstr_t("Female"));
		m_GenderCombo->AddRow(pRow);

		//DRT 4/3/03 - setup prefix combo
		m_PrefixCombo = BindNxDataListCtrl(IDC_PT_PREFIX_LIST);

		// (c.haag 2010-10-04 11:21) - PLID 39447 - Bind the insured party list
		m_dlInsuredParties = BindNxDataList2Ctrl(IDC_LIST_INS_PARTIES, false);

		//(e.lally 2008-08-29) PLID 16395 - setup preferred contact
		m_PreferredContactCombo = BindNxDataListCtrl(IDC_PRIMARY_CONTACT, false);

		//populate the Preferred Contact combo
		pRow = m_PreferredContactCombo->GetRow(NXDATALISTLib::sriGetNewRow);
		_variant_t var = (long)pctNone;
		pRow->PutValue(0,var);
		pRow->PutValue(1, _bstr_t(GetPreferredContactTypeStringFromID(pctNone)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(NXDATALISTLib::sriGetNewRow);
		var = (long)pctHomePhone;
		pRow->PutValue(0,var);
		pRow->PutValue(1, _bstr_t(GetPreferredContactTypeStringFromID(pctHomePhone)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(NXDATALISTLib::sriGetNewRow);
		var = (long)pctWorkPhone;
		pRow->PutValue(0,var);
		pRow->PutValue(1,_bstr_t(GetPreferredContactTypeStringFromID(pctWorkPhone)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(NXDATALISTLib::sriGetNewRow);
		var = (long)pctCellPhone;
		pRow->PutValue(0,var);
		pRow->PutValue(1,_bstr_t(GetPreferredContactTypeStringFromID(pctCellPhone)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(NXDATALISTLib::sriGetNewRow);
		var = (long)pctPager;
		pRow->PutValue(0,var);
		pRow->PutValue(1,_bstr_t(GetPreferredContactTypeStringFromID(pctPager)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(NXDATALISTLib::sriGetNewRow);
		var = (long)pctOtherPhone;
		pRow->PutValue(0,var);
		pRow->PutValue(1,_bstr_t(GetPreferredContactTypeStringFromID(pctOtherPhone)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(NXDATALISTLib::sriGetNewRow);
		var = (long)pctEmail;
		pRow->PutValue(0,var);
		pRow->PutValue(1,_bstr_t(GetPreferredContactTypeStringFromID(pctEmail)));
		m_PreferredContactCombo->AddRow(pRow);
		pRow = m_PreferredContactCombo->GetRow(NXDATALISTLib::sriGetNewRow);// (a.vengrofski 2010-02-01 13:10) - PLID <34208> - Added for the "text messaging"
		var = (long)pctTextMessaging;// (a.vengrofski 2010-02-01 13:12) - PLID <34208> 
		pRow->PutValue(0,var);// (a.vengrofski 2010-02-01 13:12) - PLID <34208> 
		pRow->PutValue(1, _bstr_t(GetPreferredContactTypeStringFromID(pctTextMessaging)));// (a.vengrofski 2010-02-01 13:12) - PLID <34208> 
		m_PreferredContactCombo->AddRow(pRow);// (a.vengrofski 2010-02-01 13:12) - PLID <34208> 

		//DRT 9/23/03 - PLID 9351 - Mirror link is still trying to send patients when it is disabled.  Also, 
		//		short circuiting!  We should check the license first, because maybe they removed the link, but
		//		it still has a path (which may be invalid).  That will slow down the new patient dialog, b/c
		//		it goes off to read that data path.  So check them in the order of fastest to slowest.
		//	I have also added checks for the link paths being disabled.  At this time, there is nowhere to set
		//	them, but there is a new PL item being added to handle that (9590).
		int nMirrorDisable = GetPropertyInt("MirrorDisable", 0, 0, false);
		int nInformDisable = GetPropertyInt("InformDisable", 0, 0, false);
		int nUnitedDisable = GetPropertyInt("UnitedDisable", 0, 0, false);

		//apparently an empty string DOES exist
		if (!g_pLicense->CheckForLicense(CLicense::lcInform, CLicense::cflrSilent) || nInformDisable == 1 || m_informPath == "" || m_deInform.GetStatus() == CDoesExist::eFailed)
			m_inform.EnableWindow(FALSE);
		else
			m_inform.SetCheck(GetPropertyInt("NewPatExportToInform",1));			

		if (!Mirror::IsMirrorEnabled() || !Mirror::HasMirrorLinkLicense())
			m_mirror.EnableWindow(FALSE);
		else {
			m_mirror.SetCheck(GetPropertyInt("NewPatExportToMirror",1));
		}

		if (!g_pLicense->CheckForLicense(CLicense::lcUnited, CLicense::cflrSilent) || nUnitedDisable == 1 || m_unitedPath == "" || m_deUnited.GetStatus() == CDoesExist::eFailed)
			m_united.EnableWindow(FALSE);
		else
			m_united.SetCheck(GetPropertyInt("NewPatExportToUnited",1));

		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 Settings
		CArray<long,long> arHL7GroupIDs;
		GetHL7SettingsGroupsBySetting("ExportNewPatients", TRUE, arHL7GroupIDs);
		if(!g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent) || arHL7GroupIDs.GetSize() == 0) {
			GetDlgItem(IDC_HL7)->EnableWindow(FALSE);
		}
		else {
			CheckDlgButton(IDC_HL7, BST_CHECKED);
		}

	if (!Is640x480Resource())
	{
		if(	m_pdlgReferralSubDlg == NULL) {
			m_pdlgReferralSubDlg = new CReferralSubDlg(this);
			m_pdlgReferralSubDlg->Create(IDD_REFERRAL_SUBDIALOG, this);

			CRect rc;
			GetDlgItem(IDC_NEWPT_REFERRAL_AREA)->GetWindowRect(&rc);
			ScreenToClient(&rc);
			m_pdlgReferralSubDlg->MoveWindow(rc);
		}
		if (NULL == m_pMultiReferralList) {
			// (c.haag 2006-08-04 14:50) - PLID 21776 - Move the multi-referral list to
			// the area, but reduce the window rectangle size so that it doesn't overlap
			// the buttons
			const int nBtnSpace = 5;
			CRect rcBtn;
			CRect rc;
			GetDlgItem(IDC_NEWPT_REFERRAL_AREA)->GetWindowRect(&rc);
			ScreenToClient(&rc);
			GetDlgItem(IDC_NEWPT_ADD_REFERRAL)->GetWindowRect(rcBtn);
			rc.bottom -= rcBtn.Height() + nBtnSpace;
			m_pMultiReferralList = BindNxDataList2Ctrl(IDC_NEWPT_REFERRAL_LIST, false);
			GetDlgItem(IDC_NEWPT_REFERRAL_LIST)->MoveWindow(rc);
		}

		// (j.gruber 2010-01-11 13:22) - PLID 36140 - add a default referral from the caller ID
		if (m_nDefaultReferralID != -1) {
			m_pdlgReferralSubDlg->SelectReferralID(m_nDefaultReferralID);
		}


		// (m.cable 2004-06-07 16:13) - If the user doesn't have a license for NexTrak, then don't worry about checking
		// to see if they have a default procedure chosen or not
		if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)){
			if (!m_pAddProDlg) {
				m_pAddProDlg = new CAddProcedureDlg(this);
			}
			m_pAddProDlg->m_bProcedures = GetRemotePropertyInt("DefaultProcedureType", 1) == 0 ? false : true;
			if(m_pAddProDlg->m_bProcedures) {
				GetRemotePropertyArray("DefaultProcedureList", m_pAddProDlg->m_arProcIDs);
			}
			else {
				GetRemotePropertyArray("DefaultProcedureList", m_pAddProDlg->m_arProcGroupIDs);
			}


			if (m_pAddProDlg) {
				//PLID 21774 - if there is a default procedure or group then color the button
				if ((m_pAddProDlg->m_arProcIDs.GetSize() == 0) && m_pAddProDlg->m_arProcGroupIDs.GetSize() == 0) {
					//turn it black
					m_procedureBtn.SetTextColor(RGB(0,0,0));
				}
				else {
					//turn the button red
					m_procedureBtn.SetTextColor(RGB(255,0,0));
			
				}
			}
		}

		//CAH 5/7/03 - Get our required property fields now
		m_lRequiredFields = GetRemotePropertyInt("NewPatientRequiredFields",0,0,"<None>",true);

		//(e.lally 2007-06-21) PLID 24334 - test optional contact number fields
		// (d.thompson 2012-06-27) - PLID 51220 - Changed default values
		m_lAdditionalFields = GetRemotePropertyInt("NewPatientAdditionalFields",NEWPT_ADDITIONAL_HOMENO|NEWPT_ADDITIONAL_WORKNO|NEWPT_ADDITIONAL_CELL|NEWPT_ADDITIONAL_EMAIL,0,"<None>",true);

		//TES 2003-12-30: I'm changing the implementation of this so it will be easier to find, there was a problem with
		// the patient type asterisk.
		/*// The required referral and procedure * boxes are the only ones that are not within a color control, so
		// we made to make them disappear altogether if the fields are not required.
		if (m_lRequiredFields & NEWPT_REQUIRED_REFERRAL)
			GetDlgItem(IDC_REFERRAL_REQ)->ShowWindow(SW_SHOW);
		else
			GetDlgItem(IDC_REFERRAL_REQ)->ShowWindow(SW_HIDE);
		
		if (m_lRequiredFields & NEWPT_REQUIRED_PROCEDURE)
			GetDlgItem(IDC_PROCEDURE_REQ)->ShowWindow(SW_SHOW);
		else
			GetDlgItem(IDC_PROCEDURE_REQ)->ShowWindow(SW_HIDE);*/
	}

	if (NULL == m_pMultiReferralList) {
		// (c.haag 2006-08-04 15:56) - PLID 21776 - Hide the multi-referral source checkbox
		// if the window doesn't exist (just an extra safeguard)
		GetDlgItem(IDC_CHECK_MULTIPLE_REFERRAL_SOURCES)->ShowWindow(SW_HIDE);
	}


	// (s.dhole 2013-06-05 10:17) - PLID 12018
	//1)	If location is required  then location dropdown won’t have any default selection
	//2)	If location is not required then it will try to select location which is mark form  new patient (Admin->Location-> Default New patient location checkbox), else current location.
	if (m_lRequiredFields & NEWPT_REQUIRED_LOCATION){
		m_pDefaultLocation->SetSelByColumn(pctcID, -1L);
	}
	else{
		long nLocationID = GetCurrentLocationID();
		_RecordsetPtr rs = CreateRecordset("SELECT ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND IsDefault = 1 AND TypeID = 1");
		if(!rs->eof) {
			nLocationID  = AdoFldLong(rs, "ID",GetCurrentLocationID());
		}
		rs->Close();
		m_pDefaultLocation->SetSelByColumn(pctcID, nLocationID );
	}
	m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true); 
	m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

	m_nxtBirthDate = GetDlgItemUnknown(IDC_DOB_BOX);

	m_strCurrentFirstName = "";
	m_strCurrentMiddleName = "";
	m_strCurrentLastName = "";
	m_dtCurrentBirthDate.SetStatus(COleDateTime::invalid);
	m_bTestForDuplicates = false;

	//Tell the mainframe that we want to know when the database changes.
	GetMainFrame()->RequestTableCheckerMessages(GetSafeHwnd());

	//DRT 3/29/2004 - If we're coming in from a GC, we don't want to confuse users with all sorts of available
	//	buttons, when all they can do is add the patient and go back to the GC window or cancel.
	// (r.farnworth 2016-03-08 11:40) - PLID 68454 - Same rules apply to Online Visits
	if(m_bForGC || m_bFromReassign) {
		m_editButton.EnableWindow(FALSE);
		m_resumeButton.EnableWindow(TRUE);
		m_cancelButton.EnableWindow(TRUE);
		m_scheduleButton.EnableWindow(FALSE);
		m_addanotherBtn.EnableWindow(FALSE);
		m_ffaBtn.EnableWindow(FALSE);
	}

	// (c.haag 2005-02-08 14:13) - PLID 15298 - Some clients need this feature, so we made it into a preference

	// (m.cable 6/21/2004 12:25) - PLID 13126 - If they click on save and edit further, then they are going to lose
	// any info they entered.  The new patient dlg should have this option disabled if m_bFromResEntry is set to true
	if(m_bFromResEntry){
		if(GetRemotePropertyInt("AllowSaveAndEditFurtherInScheduler", 0, 0, GetCurrentUserName(), true)) {
			m_editButton.EnableWindow(TRUE);
		}
		else {
			m_editButton.EnableWindow(FALSE);
		}
		m_ffaBtn.EnableWindow(FALSE);
	}

	if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent))
		m_procedureBtn.ShowWindow(SW_HIDE);
	else m_procedureBtn.ShowWindow(SW_SHOW);

	// (a.wetta 2007-03-19 13:30) - PLID 24983 - Get a pointer to the MSR device
	//(a.wilson 2012-1-11) PLID 47485 - added text to include the barcode scanner.
	if (GetMainFrame()->DoesOPOSMSRDeviceExist() || GetMainFrame()->DoesOPOSBarcodeScannerExist()) {
		// A MSR device is on and working, let's let the user know they can use it to input patient information
		CString strMsg = "Practice has detected a magnetic strip reader (card swiper) and/or a 2D barcode scanner attached to this computer. You can swipe or scan a driver's license at "
						"anytime and the information from that license will fill the new patient demographic fields.\n\n"
						"NOTE: The information is read off of a driver's license using the AAMVA DL/ID Card Design Specifications Ver 2.0, the national "
						"standard for storing information on a driver's license in the USA.  Some states and other countries may not follow these "
						"specifications, so not all driver's licenses will work.  Additionally, older driver's licenses may use an older standard which "
						"will also not work.";
		DontShowMeAgain(this, strMsg, "MSR_NewPatient", "Driver's License Reader Detected");
	}


	//
	// v.arth 2009-06-01 PLID 34386 - Add a country list to the new patient dialog
	//
	// Bind the countries drop down list to the database
	m_pCountryList = BindNxDataList2Ctrl(IDC_COUNTRY_LIST, true);

	// Get the ID of the default country stored in the system preferences
	int defaultCountryId = GetRemotePropertyInt("DefaultCountry", -1 , 0, "<None>", false);
	// If the defaultCountryId is -1 then no value was found from the system preferences; therefore,
	// manually setup a default country
	/*if (defaultCountryId == -1)
	{
		CString defaultCountryName = "United States";
		_RecordsetPtr countryRecordset = CreateParamRecordset("SELECT ID FROM CountriesT WHERE CountryName = {STRING}", defaultCountryName);
		if (!countryRecordset->eof)
		{
			defaultCountryId = AdoFldLong(countryRecordset->Fields->Item["ID"]); 
		}
		countryRecordset->Close();

		SetRemotePropertyInt("DefaultCountry", defaultCountryId, 0, "<None>");
	}*/

	// Get the stored value from the database
	_RecordsetPtr recordsetPtr = CreateParamRecordset("SELECT CountryName FROM CountriesT WHERE ID = {INT}", defaultCountryId);
	// If there was an error
	if (recordsetPtr->eof)
	{
		m_pCountryList->SetSelByColumn(cccID, -1);
	}
	else
	{
		// Get the value from the field
		_variant_t countryName = recordsetPtr->Fields->Item["CountryName"]->Value;
		if (countryName.vt == VT_NULL)
		{
			// Default to an empty selection
			m_pCountryList->SetSelByColumn(cccID, -1);
		}
		else
		{
			// Show the country name if there was one
			m_pCountryList->SetSelByColumn(cccName, countryName);
		}
	}
	
	// (z.manning 2009-07-08 15:31) - PLID 27251 - The email required asterisk needs to be 
	// handled here as NxLabels don't seem to get hit in OnCtlColor.
	if (m_lRequiredFields & NEWPT_REQUIRED_EMAIL) {
		m_nxlabelEmailReq.SetType(dtsHyperlink);
		m_nxlabelEmailReq.SetText("**");
	}
	//(e.lally 2007-06-21) PLID 24334 - Color additional required fields
	else if(m_lAdditionalFields & NEWPT_ADDITIONAL_EMAIL) {
		m_nxlabelEmailReq.SetText("*");
	}
	else {
		m_nxlabelEmailReq.SetText("");
	}

	// (j.gruber 2009-10-06 17:02) - PLID 35826 - reset here in case they changed the preference
	m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");

	if (m_bLookupByCity) {
		ChangeZOrder(IDC_ZIP_BOX, IDC_STATE_BOX);
	} else {
		// (r.goldschmidt 2014-11-18 14:55) - PLID 64032 - Zip comes after country list in the new patient dialog
		ChangeZOrder(IDC_ZIP_BOX, IDC_COUNTRY_LIST); 
	}

	//r.wilson PLID 39357
	if (!CheckCurrentUserPermissions(bioPatientID, SPT___W_______, FALSE, 0, TRUE) && !CheckCurrentUserPermissions(bioPatientID, SPT___W________ONLYWITHPASS, FALSE, 0, TRUE))
	{
		m_nxeditIdBox.SetReadOnly(TRUE);
	}

	// (r.farnworth 2016-03-03 12:16) - PLID 68454 - Edit the New Patient dialog so that it can be preloaded with patient information when it's launched.
	 PopulateWithPreselects();
	
	//r.wilson PLID 39357
	//bUserCanWrite = CheckCurrentUserPermissions(bioPatientID, SPT___W_______, FALSE, 0, TRUE);
	//bUserCanWriteOnlyWithPass = CheckCurrentUserPermissions(bioPatientID, SPT___W________ONLYWITHPASS, FALSE, 0, TRUE);

	}NxCatchAll("Error in OnInitDialog()");

	
	// Give focus to the first name box
	GetDlgItem(IDC_FIRST_NAME_BOX)->SetFocus();
	
	return false;
}

int CNewPatient::DoModal(long *id) 
{
	m_id = id;
	GetMainFrame()->DisallowPopup();
	int nReturn = CNxDialog::DoModal();
	GetMainFrame()->AllowPopup();
	return nReturn;
}

// Returns false if the field is required but blank
bool CNewPatient::TestRequiredField(UINT id, long nFlag, const CString& strFieldName)
{
	CString str;
	if (!(m_lRequiredFields & nFlag))
		return true;
	
	GetDlgItemText(id, str);
	if (id == IDC_DOB_BOX) {
		if(m_nxtBirthDate->GetStatus() == 1){
			// valid 
			return true;
		}
		else if(m_nxtBirthDate->GetStatus() == 2){
			// invalid
			MessageBox("Please enter a valid date of birth.");
			return false;
		}
		else{/*empty*/}
	}
	else if (id == IDC_SSN_BOX || id == IDC_HOME_PHONE_BOX ||
		id == IDC_WORK_PHONE_BOX)
	{
		str.Remove('#');
		str.Remove('-');
		str.Remove('/');
	}

	if (!str.IsEmpty())
		return true;

	str = "The new patient could not be saved because the " + strFieldName + " field must be filled in.";
	MessageBox(str);
	return false;
}

bool CNewPatient::MissingRequiredFields()
{
	CString first, last, str, error;
	int id;

	id = GetDlgItemInt(IDC_ID_BOX);
	GetDlgItemText(IDC_FIRST_NAME_BOX, first);
	GetDlgItemText(IDC_LAST_NAME_BOX, last);

	if (first == "" || last == "" || !id)//required field missing
	{	if (first == "")
			error += GetStringOfResource(IDS_FIRST_NAME) + " ";
		if (last == "")
			error += GetStringOfResource(IDS_LAST_NAME) + " ";
		if (!id)
			error += GetStringOfResource(IDS_ID) + " ";
		error.TrimRight(',');
		error += GetStringOfResource(IDS_NEW_PAT_BLANK);
		MessageBox (error);
		return true;
	}

	if (!TestRequiredField(IDC_MIDDLE_NAME_BOX, NEWPT_REQUIRED_MIDDLE, "Middle Name")) return true;
	if (!TestRequiredField(IDC_ADDRESS1_BOX, NEWPT_REQUIRED_ADDRESS1, "Address 1")) return true;
	if (!TestRequiredField(IDC_CITY_BOX, NEWPT_REQUIRED_CITY, "City")) return true;
	if (!TestRequiredField(IDC_STATE_BOX, NEWPT_REQUIRED_STATE, "State")) return true;
	if (!TestRequiredField(IDC_ZIP_BOX, NEWPT_REQUIRED_ZIP, "Zip")) return true;
	if (!TestRequiredField(IDC_SSN_BOX, NEWPT_REQUIRED_SSN, "Social Security Number")) return true;
	if (!TestRequiredField(IDC_DOB_BOX, NEWPT_REQUIRED_DOB, "Birth Date")) return true;
	if (!TestRequiredField(IDC_HOME_PHONE_BOX, NEWPT_REQUIRED_HOMENO, "Home Phone")) return true;
	if (!TestRequiredField(IDC_WORK_PHONE_BOX, NEWPT_REQUIRED_WORKNO, "Work Phone")) return true;
	// (z.manning 2009-07-08 16:45) - PLID 27251 - If they declined email, don't bother checking 
	// it's required status.
	if(!m_bEmailDeclined) {
		if (!TestRequiredField(IDC_EMAIL_BOX, NEWPT_REQUIRED_EMAIL, "Email")) return true;
	}
	if (!TestRequiredField(IDC_CELL_PHONE_BOX, NEWPT_REQUIRED_CELL, "Cell Phone")) return true;
	if (!TestRequiredField(IDC_NOTES, NEWPT_REQUIRED_NOTE, "Patient Note")) return true;

	// (s.dhole 2013-06-04 13:29) - PLID 12018
	//if (!TestRequiredField(IDC_NEW_PATIENT_LOCATION, NEWPT_REQUIRED_LOCATION, "Patient Location")) return true;
	// v.arth 2009-06-01 PLID 34384 - Account for country field being required
	if (m_lRequiredFields & NEWPT_REQUIRED_COUNTRY)
	{
		// Get the value from the drop down
		NXDATALIST2Lib::IRowSettingsPtr currentRowPtr = m_pCountryList->GetCurSel();
		CString countryName = "";
		if (currentRowPtr != NULL)
		{
			countryName = currentRowPtr->GetValue(cccName);
		}

		// Check if it is empty
		if (countryName == "")
		{
			str = "The new patient could not be saved because the country field must be filled in.";
			MessageBox(str);
			return true;
		}
	}
	

	if (m_lRequiredFields & NEWPT_REQUIRED_DOCTOR)
	{
		_variant_t varValue;
		if(m_pDocList->CurSel == -1)
			varValue = (long) -1;
		else
			varValue = m_pDocList->GetValue(m_pDocList->CurSel, 0);
		if (VarLong(varValue, -1) == -1)
		{
			// (j.jones 2010-04-30 09:20) - PLID 38151 - renamed to Provider
			MessageBox("The new patient could not be saved because a Provider must be selected.");
			return true;
		}
	}
	if (m_lRequiredFields & NEWPT_REQUIRED_COORDINATOR)
	{
		_variant_t varValue;
		if(m_pCoordList->CurSel == -1)
			varValue = (long) -1;
		else
			varValue = m_pCoordList->GetValue(m_pCoordList->CurSel, 0);
		if (VarLong(varValue, -1) == -1)
		{
			MessageBox("The new patient could not be saved because a Patient Coordinator must be selected.");
			return true;
		}
	}
	if (m_lRequiredFields & NEWPT_REQUIRED_GENDER)
	{
		// (j.jones 2016-02-25 15:56) - PLID 67549 - get the value, assume 0 if unselected
		long nGender = 0;
		if (m_GenderCombo->GetCurSel() != -1) {
			nGender = VarLong(m_GenderCombo->GetValue(m_GenderCombo->GetCurSel(), gccID), 0);
		}
		if (nGender == 0)
		{
			MessageBox("The new patient could not be saved because a Gender must be selected.");
			return true;
		}
	}
	if (m_lRequiredFields & NEWPT_REQUIRED_REFERRAL)
	{
		if (!m_btnMultipleReferrals.GetCheck() && m_pdlgReferralSubDlg != NULL) {
			long nReferralID = m_pdlgReferralSubDlg->GetSelectedReferralID();
			if (nReferralID <= 0)
			{
				MessageBox("The new patient could not be saved because a Referral must be selected.");
				return true;
			}
		}
		// (c.haag 2006-08-04 15:53) - PLID 21776 - Support for multi-referrals
		else if (m_btnMultipleReferrals.GetCheck() && m_pMultiReferralList != NULL) {
			if (0 == m_pMultiReferralList->GetRowCount())
			{
				MessageBox("The new patient could not be saved because a Referral must be selected.");
				return true;
			}
		}
	}
	if (m_lRequiredFields & NEWPT_REQUIRED_REF_PHYS)
	{
		_variant_t varValue;
		if(m_pRefPhysList->CurSel == -1)
			varValue = (long) -1;
		else
			varValue = m_pRefPhysList->GetValue(m_pRefPhysList->CurSel, rpcID);
		if (VarLong(varValue, -1) == -1)
		{
			MessageBox("The new patient could not be saved because a Referring Physician must be selected.");
			return true;
		}
	}
	// (r.gonet 05/16/2012) - PLID 48561 - Made the affiliate physician required.
	if (m_lRequiredFields & NEWPT_REQUIRED_AFFILIATE_PHYS)
	{
		_variant_t varValue;
		if(m_pAffiliatePhysList->CurSel == NULL) {
			varValue = (long)-1;
		} else {
			varValue = m_pAffiliatePhysList->CurSel->GetValue(afphID);
		}
		if(VarLong(varValue, -1) == -1) {
			MessageBox("The new patient could not be saved because an Affiliate Physician must be selected.");
			return true;
		}
	}
	if (m_lRequiredFields & NEWPT_REQUIRED_REF_PAT)
	{
		//DRT 6/28/2005 - PLID 16745 - If we have disabled this combo, the required property no longer applies.
		if(m_pRefPatientList != NULL) {
			_variant_t varValue;
			if(m_pRefPatientList->CurSel == -1)
				varValue = (long) -1;
			else
				varValue = m_pRefPatientList->GetValue(m_pRefPatientList->CurSel, 0);
			if (VarLong(varValue, -1) == -1)
			{
				MessageBox("The new patient could not be saved because a Referring Patient must be selected.");
				return true;
			}
		}
	}
	if (m_lRequiredFields & NEWPT_REQUIRED_PAT_TYPE)
	{
		_variant_t varValue;
		if(m_PatientTypeCombo->CurSel == -1)
			varValue = (long) -1;
		else
			varValue = m_PatientTypeCombo->GetValue(m_PatientTypeCombo->CurSel, 0);
		if (VarLong(varValue, -1) == -1)
		{
			MessageBox("The new patient could not be saved because a patient type must be selected.");
			return true;
		}
	}

	if (m_lRequiredFields & NEWPT_REQUIRED_PROCEDURE)
	{
		BOOL bProcedure = FALSE;
		if (!m_pAddProDlg)
			m_pAddProDlg = new CAddProcedureDlg(this);

		if(!m_pAddProDlg)
			return false;	//There's a slight possibility we might not have this dialog, fail if so.

		if(m_pAddProDlg->m_bProcedures) {
			if(m_pAddProDlg->m_arProcIDs.GetSize() > 0)
				bProcedure = TRUE;
		}
		else {
			if(m_pAddProDlg->m_arProcGroupIDs.GetSize() > 0)
				bProcedure = TRUE;
		}
		if (!bProcedure)
		{			
			MessageBox("The new patient could not be saved because a procedure must be selected.");
			return true;
		}
	}

	// (c.haag 2009-03-02 15:46) - PLID 17142 - Required prefixes
	if (m_lRequiredFields & NEWPT_REQUIRED_PREFIX) {
		CString strPrefix;
		if(m_PrefixCombo->GetCurSel() > -1) {
			// Test for a valid prefix by taking the string value and trimming any whitespace characters.
			// If the result is an empty string, the test will fail.
			strPrefix = VarString(m_PrefixCombo->GetValue(m_PrefixCombo->GetCurSel(), 1));
			strPrefix.Trim();
		}
		else {
			// No valid prefix selected
		}

		if (strPrefix.IsEmpty()) {
			MessageBox("The new patient could not be saved because a prefix must be selected.", NULL, MB_OK | MB_ICONERROR);
			return true;
		}
	}
	// (s.dhole 2013-06-05 10:48) - PLID 12018 check loacation if it is required
	if (m_lRequiredFields & NEWPT_REQUIRED_LOCATION)
	{
		long nValue =-1;
		if(m_pDefaultLocation && m_pDefaultLocation->CurSel ){
			nValue = VarLong(m_pDefaultLocation->CurSel->GetValue(pctcID),-1); 
		}
		if (nValue  == -1)
		{
			MessageBox("The new patient could not be saved because a location must be selected.");
			return true;
		}
	}

	return false;
}

bool CNewPatient::MissingAdditionalField()
{
	//(e.lally 2007-06-21) PLID 24334 - test optional contact number fields
	//if nothing else is optional, we can return
	if(m_lAdditionalFields == 0)
		return false;
	
	bool bNeedsAdditionalField = false;
	CString strAdditionalFields;

	if(m_lAdditionalFields & NEWPT_ADDITIONAL_HOMENO){
		//Double check that this isn't a required field already
		if((m_lRequiredFields & NEWPT_REQUIRED_HOMENO)==0){
			CString strValue;
			GetDlgItemText(IDC_HOME_PHONE_BOX, strValue);
			strValue.Remove('#');
			strValue.Remove('-');
			strValue.Remove('/');
			strValue.Remove('(');
			strValue.Remove(')');
			strValue.TrimLeft();
			strValue.TrimRight();
			if(!strValue.IsEmpty())
				return false; //They meet the any requirement
			//They did not meet the any requirement yet
			bNeedsAdditionalField = true;
			strAdditionalFields += "Home Phone, ";
		}
	}
	if(m_lAdditionalFields & NEWPT_ADDITIONAL_WORKNO){
		if((m_lRequiredFields & NEWPT_REQUIRED_WORKNO)==0){
			CString strValue;
			GetDlgItemText(IDC_WORK_PHONE_BOX, strValue);
			strValue.Remove('#');
			strValue.Remove('-');
			strValue.Remove('/');
			strValue.Remove('(');
			strValue.Remove(')');
			strValue.TrimLeft();
			strValue.TrimRight();
			if(!strValue.IsEmpty())
				return false; //They meet the any requirement
			//They did not meet the any requirement yet
			bNeedsAdditionalField = true;
			strAdditionalFields += "Work Phone, ";
		}
	}
	if(m_lAdditionalFields & NEWPT_ADDITIONAL_CELL){
		if((m_lRequiredFields & NEWPT_REQUIRED_CELL)==0){
			CString strValue;
			GetDlgItemText(IDC_CELL_PHONE_BOX, strValue);
			strValue.Remove('#');
			strValue.Remove('-');
			strValue.Remove('/');
			strValue.Remove('(');
			strValue.Remove(')');
			strValue.TrimLeft();
			strValue.TrimRight();
			if(!strValue.IsEmpty())
				return false; //They meet the any requirement
			//They did not meet the any requirement yet
			bNeedsAdditionalField = true;
			strAdditionalFields += "Cell Phone, ";
		}
	}
	if(m_lAdditionalFields & NEWPT_ADDITIONAL_EMAIL){
		if((m_lRequiredFields & NEWPT_REQUIRED_EMAIL)==0){
			CString strValue;
			GetDlgItemText(IDC_EMAIL_BOX, strValue);
			strValue.TrimLeft();
			strValue.TrimRight();
			if(!strValue.IsEmpty())
				return false; //They meet the any requirement
			//They did not meet the any requirement yet
			bNeedsAdditionalField = true;
			strAdditionalFields += "E-mail, ";
		}
	}
	if(bNeedsAdditionalField == true){
		//Something additional is required, but nothing extra is filled in
		strAdditionalFields.TrimRight(", ");
		long nLastSeparator = strAdditionalFields.ReverseFind(',');
		if(nLastSeparator != -1){
			strAdditionalFields.Delete(nLastSeparator, 1);
			strAdditionalFields.Insert(nLastSeparator, " or");
		}
		CString strMessage;
		strMessage.Format("You must fill in the %s field before this new patient record can be saved.", strAdditionalFields);
		MessageBox(strMessage);
	}
	return bNeedsAdditionalField;

}

// (c.haag 2010-10-04 15:20) - PLID 39447 - Moved into its own function. This is called early in the Save event
// because there's no need to be running all those queries in Save when we find a problem in memory.
BOOL CNewPatient::InsuranceInfoIsInvalid()
{
	// (j.gruber 2009-10-12 11:41) - PLID 10723 - validate the insurance
	// (c.haag 2010-10-04 15:20) - PLID 39447 - We don't require the old style validation any longer.
	// We require them to select an insurance company before leaving the modal insurance window,
	// and we force self guarantors to have the patient information. The only thing we have to ensure
	// is that no two parties have the same placement.
	// (j.jones 2010-11-08 13:53) - PLID 39620 - skip this for Alberta as well
	if (!UseOHIP() && !UseAlbertaHLINK()) {
		CMap<long,long,BOOL,BOOL> mapPlacement;
		for (int i=0; i < m_aInsuredParties.GetSize(); i++) 
		{
			BOOL bDummy;
			long nRespID = m_aInsuredParties[i].GetRespTypeID();
			if (nRespID > 0) {
				if (mapPlacement.Lookup(nRespID, bDummy)) {
					AfxMessageBox("At least two insured parties have matching placements. Please fix this before saving.",
							MB_OK | MB_ICONSTOP);
					return TRUE;
				}
				else {
					mapPlacement.SetAt(m_aInsuredParties[i].GetRespTypeID(), TRUE);
				}
			}
			else {
				// Invalid or inactive placement. We do allow for multiple inactives though I don't know
				// why anyone would enter those now.
			}
		}
	}
	return FALSE;

/*	BOOL bCreateInsurance = FALSE;
	long nInsuranceID = -1, nInsPartyID = -1;
	CString strInsIDForIns, strInsPolGroupNum, strInsRelationToPatient, strInsuranceCoName;
	if(!UseOHIP()) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInsCoList->CurSel;
		if (pRow) {

			long nValue = VarLong(pRow->GetValue(ilcID));

			GetDlgItemText(IDC_NEWPAT_INS_ID_BOX, strInsIDForIns);
			GetDlgItemText(IDC_NEWPAT_INS_FECA_BOX, strInsPolGroupNum);

			if (nValue == -1) {		

				//they have none selected, so make sure they don't have any information entered
				//check everything but the relation, since we set that to self
				if (m_bInsuredPartySet ||
					!strInsIDForIns.IsEmpty() ||
					!strInsPolGroupNum.IsEmpty()) {

						//Warn them
						if (IDNO == MsgBox(MB_YESNO, "You have filled out insured party information, but have no insurance company selected.\n"
							"If you continue an Insured Party will NOT be created for this patient.\n"
							"Are you sure you want to continue?")) {
								return FALSE;
						}
						else {
							//they said yes, so we aren't creating one
							bCreateInsurance = FALSE;
						}
				}
			}
			else {

				ASSERT(nValue != 2);

				bCreateInsurance = TRUE;

				nInsuranceID = nValue;			

				strInsuranceCoName = VarString(pRow->GetValue(ilcName));

				pRow = m_pRelationToPatientList->CurSel;
				if (pRow) {
					strInsRelationToPatient = VarString(pRow->GetValue(0));
				}
				else {
					//this shouldn't be possible
					ASSERT(FALSE);

					//assign it to self
					strInsRelationToPatient = "Self";
				}
			}

		}	
	}*/
}

//TES 7/16/2009 - PLID 25154 - HL7SettingsGroup declaration moved to HL7Utils.h

bool CNewPatient::Save(void) 
{
	CString			first, 
					middle, 
					last,
					add1,
					add2,
					city,
					state,
					zip,
					ssn,
					dob,
					home,
					work,
					ext,
					notes,
					error,
					sql,
					referral,
					cell,
					email,
					strPrefix;


	int				id,
					sex = 0,
					status,
					homepriv,
					workpriv,
					referralID;

	_RecordsetPtr	rs;
	_variant_t		tmpVar;

	try {

	CWaitCursor pWait;

	//CAH 5/7/03 - Abort from saving if we are missing any required information.
	if (MissingRequiredFields()) // The function will throw a message box so we don't need to.
		return false;

	//(e.lally 2007-06-21) PLID 24334 - test optional contact number fields in addition to required fields.
	//We know the required fields are filled in, so now we can test if they need any optional fields in addition to that
	if(MissingAdditionalField())
		return false;

	// (c.haag 2010-10-04 15:20) - PLID 39447 - Insurance info validation (moved from further down to here)
	if(InsuranceInfoIsInvalid())
		return false;

//CurrentStatus
	id = GetDlgItemInt(IDC_ID_BOX);
	GetDlgItemText(IDC_FIRST_NAME_BOX, first);
	GetDlgItemText(IDC_LAST_NAME_BOX, last);
	GetDlgItemText(IDC_MIDDLE_NAME_BOX, middle);
	
	if (id <= 0)//negative id
	{	MsgBox (RCS(IDS_NEGATIVE_PAT_ID));
		return false;
	}
	if(id >= 2147483646)
	{
		AfxMessageBox("Practice cannot store Patient IDs greater than 2,147,483,645.\nPlease enter a smaller Patient ID number.");
		return false;
	}

	// (r.goldschmidt 2014-07-17 19:05) - PLID 62774 - Must validate birth date
	if (!ValidateDateOfBirth(dob)){ // the function will either throw message box or save dob as correct string for queries
		return false;
	}

	if (!m_bTestForDuplicates){
		if (CheckForDuplicatePatients()){
			return false;
		}
	}

	// (c.haag 2003-10-01 11:42) - Referral tree is too big for 640x480 view
	if (!m_btnMultipleReferrals.GetCheck() && m_pdlgReferralSubDlg != NULL)
	{
		referralID = m_pdlgReferralSubDlg->GetSelectedReferralID();
		referral = m_pdlgReferralSubDlg->GetSelectedReferralName();

		// (r.goldschmidt 2014-08-29 12:18) - PLID 31191 - Check if selection is restricted.
		if (referralID > 0){
			CString strWarning;
			long nPreferenceViolation = m_pdlgReferralSubDlg->ReferralRestrictedByPreference(referralID, strWarning);
			if (nPreferenceViolation != 0){
				AfxMessageBox(strWarning);
				return false;
			}
			else {
				// nPreferenceViolation == 0, which means there is no violation
			}
		}
	}
	else if (m_btnMultipleReferrals.GetCheck() && m_pMultiReferralList != NULL) {
		// (c.haag 2006-08-04 15:26) - PLID 21776 - Assign primary referral
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMultiReferralList->FindByColumn(0, m_nPrimaryMultiReferralID, m_pMultiReferralList->GetFirstRow(), false);
		referralID = m_nPrimaryMultiReferralID;
		if (NULL != pRow) {
			referral = VarString(pRow->Value[1]);
		}
		else {
			referral = "";
		}
	}
	else
	{
		referralID = -1;
		referral = "";
	}

		//duplicate id
	BOOL bUseNextAvailableID = FALSE;
		try {
			if (!IsRecordsetEmpty("SELECT UserDefinedID FROM PatientsT WHERE UserDefinedID = %li", id)) {
				if (id == m_nOriginalUserDefinedID) {
					// The ID we're trying to save is exactly the same as the id we internally generated
					UINT nResult = MsgBox(MB_ICONINFORMATION|MB_OKCANCEL, 
						"The automatically generated patient ID has just been used by someone else "
						"to create a new patient.  Click OK to use the next available ID or "
						"click Cancel to go back and change to a different patient ID.");
					if (nResult == IDOK) {
						bUseNextAvailableID = TRUE;
					} else {
						return false;
					}
				} else {
					// The ID we're trying to save was edited by the user so tell them to change it
					UINT nResult = MsgBox(MB_ICONINFORMATION|MB_OK, 
						"The patient ID you entered is assigned to an existing "
						"patient.  Click OK to go back and change to a different ID.");
					return false;
				}
			}
		}NxCatchAllCall("Error saving new patient - duplicate ID check.",{return false;});

	// (d.moore 2007-08-15) - PLID 25455
	// Check to see if there are any inquiries that are a close match to the new
	//  patients name or email address.
	// (z.manning 2009-07-08 16:48) - PLID 27251 - If email was declined, just leave it as a blank string
	if(!m_bEmailDeclined) {
		GetDlgItemText(IDC_EMAIL_BOX, email);
	}
	long nInquiryID = CheckForInquiryConversion(first, last, email);
	
	//we are good to go!
	_variant_t vtDoctor = g_cvarNull;
	if(m_pDocList->CurSel != -1) {
		tmpVar = m_pDocList->GetValue(m_pDocList->CurSel,0);
		if(tmpVar.vt == VT_I4)
			vtDoctor = tmpVar;
	}

	GetDlgItemText(IDC_ADDRESS1_BOX,	add1);
	GetDlgItemText(IDC_ADDRESS2_BOX,	add2);
	GetDlgItemText(IDC_CITY_BOX,		city);
	GetDlgItemText(IDC_STATE_BOX,		state);
	GetDlgItemText(IDC_ZIP_BOX,			zip);
	GetDlgItemText(IDC_HOME_PHONE_BOX,	home);
	GetDlgItemText(IDC_WORK_PHONE_BOX,	work);
	GetDlgItemText(IDC_EXT_PHONE_BOX,	ext);
	GetDlgItemText(IDC_NOTES,			notes);
	GetDlgItemText(IDC_CELL_PHONE_BOX,	cell);
	
	if (notes == "")//bandaid for zero length not allowed in notes
		notes = " ";
	ext.TrimRight('#');//get rid of those annoying pound signs!
	zip.TrimRight('#');
	home.TrimRight('#');
	work.TrimRight('#');

	GetDlgItemText(IDC_SSN_BOX, ssn);
	//DRT 7/14/03 - We want to save the -'s in the data, so that LW and reports can accurately show the ssn in the right format
	//ssn.Replace("-","");
	ssn.Replace("#", "");
	ssn.TrimLeft();
	ssn.TrimRight();

	//now that we cleaned it up, check to see if its empty
	if (! ssn.IsEmpty()) {
		//check to see that this is the only SSN in the database
		//DRT 7/14/03 - This handles checking 2 ways of SSN's.  For a while we've been saving with no -'s here, but with -'s in Gen1
		//		Now we save with -'s in both places, but we want this check to look for both of them.
		try{
			_RecordsetPtr rsDuplicate = CreateRecordset("SELECT ID, First, Last FROM PersonT WHERE REPLACE(SocialSecurity, '-', '') = Replace('%s', '-', '') AND (ID IN (SELECT PersonID FROM PatientsT)) ", ssn, ssn);
			if (!rsDuplicate->eof) {
				//there is a duplicate!! Check to see if they want to do this
				CString strMessage;
					if(rsDuplicate->RecordCount < 5){
					// there's only a few with the same SSN so tell the user who they are
					CString strFirst, strLast, strNameToAdd;
					strMessage.Format("The following patients have the same Social Security Number:  \r\n");
					while(!rsDuplicate->eof){
						strFirst = AdoFldString(rsDuplicate->GetFields(), "First", "");
						strLast	 = AdoFldString(rsDuplicate->GetFields(), "Last", "");
						strNameToAdd.Format("%s %s \r\n", strFirst, strLast);
						strMessage += strNameToAdd;
						rsDuplicate->MoveNext();
					}
				}
				else{
					// there are potentially a lot of duplicates so just let the user know how many there are
					strMessage.Format("There are %li other patients that have this Social Security Number.  ", rsDuplicate->RecordCount);
				}
				
				if (IDNO == MsgBox(MB_YESNO, strMessage + "\r\nAre you sure you wish to save this?", "NexTech")) {
					//set it back
					SetDlgItemText(IDC_SSN_BOX, "");
					return false;
				}	
			}
		}NxCatchAll("Error checking for duplicate SSN");
	}

	// (j.jones 2016-02-25 15:56) - PLID 67549 - get the value, assume 0 if unselected
	sex = 0;
	if (m_GenderCombo->GetCurSel() != -1) {
		sex = VarLong(m_GenderCombo->GetValue(m_GenderCombo->GetCurSel(), gccID), 0);
	}

	if(m_PrefixCombo->GetCurSel() > -1) {
		long nPrefix = VarLong(m_PrefixCombo->GetValue(m_PrefixCombo->GetCurSel(), 0));
		//DRT 12/12/2003 - PLID 8817 - Code below checks to make sure this is NULL, so
		//	if nothing is selected (having the empty row counts as nothing) we need to 
		//	get the value of NULL here.
		if(nPrefix > -1) 
			strPrefix.Format("%li", nPrefix);
		else
			strPrefix = "NULL";
	}
	else
		strPrefix = "NULL";

	
	// v.arth 2009-06-01 PLID 34386 - Add country list tohe New Patient dialog
	// Get the value of the country
	NXDATALIST2Lib::IRowSettingsPtr currentRowPtr = m_pCountryList->GetCurSel();
	CString countryName = "";
	if (currentRowPtr != NULL)
	{
		countryName = currentRowPtr->GetValue(cccName);
	}

	// (a.walling 2009-06-05 11:06) - PLID 34464 - Save NULL rather than a blank string for country
	CString strSaveCountry = "NULL";
	if (!countryName.IsEmpty()) {
		strSaveCountry.Format("'%s'", _Q(countryName));
	}

/*	DRT 10/23/01 - New Gender Combo
	if (m_male.GetCheck()) sex = 1;
		else if (m_female.GetCheck()) sex = 2;
		else sex = 0;
*/
	//TES 8/13/2014 - PLID 63194 - Store prospect status in our member variable
	if (m_patient.GetCheck()) {
		m_bIsProspect = false;
		status = 1;
	}
	else if (m_prospect.GetCheck()) {
		m_bIsProspect = true;
		status = 2;
	}
	else {
		m_bIsProspect = false;
		status = 0;
	}

	if (m_homePriv.GetSafeHwnd() && m_homePriv.GetCheck()) 
		homepriv = -1;
	else homepriv = 0;

	if (m_workPriv.GetSafeHwnd() && m_workPriv.GetCheck()) 
		workpriv = -1;
	else workpriv = 0;

	_variant_t vtCoord = g_cvarNull;;
	if(m_pCoordList->CurSel != -1) {
		tmpVar = m_pCoordList->GetValue(m_pCoordList->CurSel,0);
		if (tmpVar.vt == VT_I4 && tmpVar.lVal > 0)
			vtCoord = tmpVar;
	}

	_variant_t vtRefPhys = g_cvarNull;
	if(m_pRefPhysList && m_pRefPhysList->CurSel != -1) {
		tmpVar = m_pRefPhysList->GetValue(m_pRefPhysList->CurSel, rpcID);
		if(tmpVar.vt == VT_I4 && VarLong(tmpVar) > 0)
			vtRefPhys = tmpVar;
	}

	// (r.gonet 05/16/2012) - PLID 48561 - Affiliate Physician
	_variant_t vtAffiliatePhys = g_cvarNull;
	if(m_pAffiliatePhysList && m_pAffiliatePhysList->CurSel != NULL) {
		tmpVar = m_pAffiliatePhysList->CurSel->GetValue(afphID);
		if(tmpVar.vt == VT_I4 && VarLong(tmpVar) > 0) {
			vtAffiliatePhys = tmpVar;
		}
	}

	_variant_t vtRefPatient = g_cvarNull;
	if(m_pRefPatientList && m_pRefPatientList->CurSel != -1) {
		tmpVar = m_pRefPatientList->GetValue(m_pRefPatientList->CurSel,0);
		if(tmpVar.vt == VT_I4 && VarLong(tmpVar) > 0) {
			vtRefPatient = tmpVar;
		}
	}

	//ensure the prefix is still valid
	if(strPrefix != "NULL" && IsRecordsetEmpty("SELECT ID FROM PrefixT WHERE ID = %s",strPrefix)) {
		MsgBox("The prefix you selected for this patient has been deleted by another user.  This patient will be saved without a prefix.");
		strPrefix = "NULL";
	}

	//ensure the referral source is still valid
	if(referralID != -1 && IsRecordsetEmpty("SELECT PersonID FROM ReferralSourceT WHERE PersonID = %li",referralID)) {
		referralID = -1;
	}

	//get the patient type ID
	_variant_t vtPatTypeID = g_cvarNull;
	if(m_PatientTypeCombo != NULL && m_PatientTypeCombo->GetCurSel() != -1) {
		long pattype = VarLong(m_PatientTypeCombo->GetValue(m_PatientTypeCombo->GetCurSel(),0),-1);
		if(pattype != -1 && ReturnsRecordsParam("SELECT TypeIndex FROM GroupTypes WHERE TypeIndex = {INT}",pattype)) {
			vtPatTypeID = pattype;
		}
	}

	


	//(e.lally 2008-08-29) PLID 16395 - Preferred method of contact
	long nPreferredContact = 0;
	if(m_PreferredContactCombo != NULL && m_PreferredContactCombo->GetCurSel() != -1){
		nPreferredContact = VarLong(m_PreferredContactCombo->GetValue(m_PreferredContactCombo->GetCurSel(), 0), 0);
	}

	//get the default ICD9 code
	// (j.armen 2014-03-14 09:13) - PLID 61375 - The default could be 9 or 10.  Query to find out what type of code it is
	_variant_t vtDefaultDiag9ID1 = g_cvarNull;
	_variant_t vtDefaultDiag10ID1 = g_cvarNull;
	long nDefaultICD = GetRemotePropertyInt("DefaultICD9Code",-1,0,"<None>",TRUE);
	_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(),
		"SELECT ID, ICD10 FROM DiagCodes WHERE ID = {INT}", nDefaultICD);
	if(!prs->eof)
	{
		if(AdoFldBool(prs, "ICD10"))
			vtDefaultDiag10ID1 = nDefaultICD;
		else
			vtDefaultDiag9ID1 = nDefaultICD;
	}
	

	// (s.dhole 2013-06-05 10:18) - PLID 12018 since user can select location , we will use only user selected location
	// It is odd we do not validate location and worn user , rather than simply eliminate data, 
	// will follow same till we revisit this logic 
	// We wont allow to delete location if there patient assigned to it
	// (s.tullis 2016-05-25 15:41) - NX-100760  
	long LocationID = -1 ;
	if(m_pDefaultLocation != NULL && VarLong( m_pDefaultLocation->CurSel->GetValue(pctcID),-1) !=-1) {
		LocationID=  VarLong(m_pDefaultLocation->CurSel->GetValue(pctcID),-1);
		if (LocationID != -1) {
			if (!ReturnsRecordsParam("SELECT ID  FROM LocationsT WHERE Managed = 1 AND Active = 1 AND TypeID = 1  AND ID = {INT} ", LocationID))
			{// no recodr found reset it
				LocationID = -1;
			}
		}
	}
	//find the default location, otherwise use the current location
	/*long LocationID = GetCurrentLocationID();
	rs = CreateRecordset("SELECT ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND IsDefault = 1 AND TypeID = 1");
	if(!rs->eof) {
		LocationID = AdoFldLong(rs, "ID",GetCurrentLocationID());
	}
	rs->Close();*/

	//(e.lally 2009-01-26) PLID 32813 - Check our NexWeb preference for creating a security code.
	BOOL bCreateSecurityCode = FALSE;

	// (j.gruber 2009-12-01 09:06) - PLID 36455 - update to new license structure
	if(g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrSilent)) {
		// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Always (1)
		if(GetRemotePropertyInt("AssignNewPatientSecurityCode", 1, 0, "<None>", true) != 0){
			bCreateSecurityCode = TRUE;
		}
	}

	try {
		// (d.moore 2007-08-15) - PLID 25455
		// Check to see if an inquiry person was selected. If so, then we want to 
		//  convert the inquiry over to a patient and update existing records. If
		//  an inquiry wasn't selected, then we need to create a new patient.
		if (nInquiryID <= 0) {
			// (d.moore 2007-08-15) - PLID 25455
			// No inquiry selected. Create a new patient.
			CString strSqlBatch = BeginSqlBatch();

			// (j.jones 2010-01-12 10:07) - PLID 31927 - check the default text message privacy field
			long nTextMessagePrivacy = GetRemotePropertyInt("NewPatientsDefaultTextMessagePrivacy", 0, 0, "<None>", true);

			int nPersonID = NewNumber("PersonT", "ID");
			*m_id = nPersonID;

			if (bUseNextAvailableID) {
				id = NewNumber("PatientsT", "UserDefinedID");
			}

			// (a.walling 2006-11-14 13:55) - PLID 22715 - Put all these into a sql batch
			// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
			CSqlTransaction trans("CNewPatient::Save");
			trans.Begin();
			// (z.manning 2008-07-11 14:44) - PLID 30678 - Added text message.
			// v.arth 2009-06-01 PLID 34386 - Add country list to New Patient dialog
			// (j.jones 2010-01-12 10:07) - PLID 31927 - removed a recordset that loaded the UserID, as it is already in memory
			// (j.jones 2010-01-12 10:07) - PLID 31927 - supported defaulting the text message privacy field
			ExecuteSql("INSERT INTO PersonT (ID, Location, First, Middle, Last, Address1, Address2, City, State, Zip, "
				"HomePhone, WorkPhone, Extension, Note, Gender, UserID, FirstContactDate, SocialSecurity, "
				"BirthDate, PrivHome, PrivWork, InputDate, Email, CellPhone, PrefixID, Country, TextMessage) "
				"VALUES (%li, %s, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %li, "
				"%li, GetDate(), '%s', %s, %li, %li, GetDate(), '%s', '%s', %s, %s, %li)", nPersonID, LocationID == -1 ? "NULL"  : FormatString("%li",LocationID) , _Q(first),
				_Q(middle), _Q(last), _Q(add1), _Q(add2), _Q(city), _Q(state), _Q(zip), home, work, ext, _Q(notes), sex, GetCurrentUserID(), 
				ssn, dob, homepriv, workpriv,  _Q(email), cell, strPrefix, strSaveCountry, nTextMessagePrivacy); 

			//(e.lally 2008-08-29) PLID 16395 - Added preferred contact
			// (z.manning 2009-07-09 09:53) - PLID 27251 - Added DeclinedEmail
			// (r.gonet 05/16/2012) - PLID 48561 - Added Affiliate Physician
			// (j.armen 2014-03-14 09:13) - PLID 61375 - Parameterize, added DefaultICD10DiagID1
			ExecuteParamSql(
				"INSERT INTO PatientsT (\r\n"
				"	PersonID, UserDefinedID, MainPhysician, CurrentStatus, ReferralID,\r\n"
				"	EmployeeID, TypeOfPatient, DefaultReferringPhyID, AffiliatePhysID, ImageIndex,\r\n"
				"	InformID, ReferringPatientID, DefaultDiagID1, DefaultICD10DiagID1, PreferredContact,\r\n"
				"	DeclinedEmail\r\n"
				"	) VALUES (\r\n"
				"	{INT}, {INT}, {VT_I4}, {INT}, {INT},\r\n"
				"	{VT_I4}, {VT_I4}, {VT_I4}, {VT_I4}, 0,\r\n"
				"	0, {VT_I4}, {VT_I4}, {VT_I4}, {INT}, {BIT})", 
				nPersonID, id, vtDoctor, status, referralID,
				vtCoord, vtPatTypeID, vtRefPhys, vtAffiliatePhys,
				vtRefPatient, vtDefaultDiag9ID1, vtDefaultDiag10ID1, nPreferredContact,
				!!m_bEmailDeclined);

			// (j.armen 2014-01-28 16:47) - PLID 60146 - Set Default Security Group
			ExecuteParamSql(
				"DECLARE @SecurityGroupID INT\r\n"
				"SET @SecurityGroupID = (SELECT IntParam FROM ConfigRT WHERE Username = '<None>' AND Name = 'DefaultSecurityGroup')\r\n"
				"IF (ISNULL(@SecurityGroupID, -1) <> -1) AND EXISTS(SELECT ID FROM SecurityGroupsT WHERE ID = @SecurityGroupID)\r\n"
				"BEGIN\r\n"
				"	INSERT INTO SecurityGroupDetailsT (SecurityGroupID, PatientID) VALUES (@SecurityGroupID, {INT})\r\n"
				"END\r\n", nPersonID);

			// (b.cardillo 2009-05-28 16:46) - PLIDs 34368 and 34369 - We just created a patient, so all wellness qualification needs to be updated
			UpdatePatientWellnessQualification_NewPatient(GetRemoteData(), nPersonID);

			//(e.lally 2009-01-26) PLID 32813 - Create a security code for this patient if the preference to do so is on.
			if(bCreateSecurityCode != FALSE){
				//(e.lally 2009-01-26) PLID 32813 - Generate and assign a security code. 
				_ConnectionPtr pCon = GetRemoteData();
				//Potential performance hit here depending on how many trips to the sql server this has to make.
				CString strSecurityCode = EnsurePatientSecurityCode(nPersonID, pCon);
				// (d.singleton 2011-10-11 11:31) - PLID 42102 - add auditing for security codes.
				AuditEvent(nPersonID, GetExistingPatientName(nPersonID), BeginNewAuditEvent(), aeiPatientSecurityCode, nPersonID, "", strSecurityCode, aepHigh, aetChanged);
			}
			
			//
			// (c.haag 2006-08-04 15:26) - PLID 21776 - Add multiple referrals if necessary
			//
			if (m_btnMultipleReferrals.GetCheck() && m_pMultiReferralList != NULL) {
				NXDATALIST2Lib::IRowSettingsPtr p = m_pMultiReferralList->GetFirstRow();
				CString strSql;
				while (NULL != p) {
					CString strPart;
					strPart.Format("INSERT INTO MultiReferralsT (ReferralID, PatientID, Date) VALUES (%d, %d, GetDate());",
						VarLong(p->Value[0]), nPersonID);
					strSql += strPart;
					p = p->GetNextRow();
				}
				if (strSql.GetLength() > 0) {
					ExecuteSql(strSql);
				}
			}

			//also need to insert into MultiReferralsT for the new referral stuff
			COleDateTime dt;
			dt = COleDateTime::GetCurrentTime();
			CString date = FormatDateTimeForSql(dt, dtoDate);
			if (referralID != -1 && !m_btnMultipleReferrals.GetCheck())
				ExecuteSql("INSERT INTO MultiReferralsT (ReferralID, PatientID, Date) (SELECT %li, %li, '%s')", referralID, nPersonID, date);


			// (a.walling 2006-11-14 13:25) - PLID 22715 - Also store the related patient if necessary
			long nRelPatSel = m_pRelatedPatientList->GetCurSel();
			if (nRelPatSel != NXDATALISTLib::sriNoRow) {
				long nRelPatID = VarLong(m_pRelatedPatientList->GetValue(nRelPatSel, 0), -1);
				if (nRelPatID > 0) {
					long nFamilyID = FamilyUtils::GetFamilyID(nRelPatID);
					if (nFamilyID == -1) { // relative not in family
						// create a new family record and insert the patient
						nFamilyID = FamilyUtils::CreateNewFamilyAndRelationship(nPersonID, nRelPatID);
					} else {
						// save the data into PersonFamilyT
						FamilyUtils::InsertRelationship(nPersonID, nRelPatID, nFamilyID);
						// since this is a new patient we can be guaranteed that they do not already belong to a different family.
					}
				}
			}

			// (a.walling 2007-05-21 11:00) - PLID 26079 - Apply any referred patient points
			try {
				if (vtRefPatient != g_cvarNull) {
					Rewards::ApplyRefPatient(nPersonID, VarLong(vtRefPatient)); // points go to nRefPatient
				}
			} NxCatchAll("Error applying referred patient rewards.");

			trans.Commit();

			//auditing
			CString strName;
			strName.Format("%s, %s %s", last, first, middle);
			CAuditTransaction audit;
			AuditEvent(nPersonID, strName, audit, aeiPatientCreated, nPersonID, "", strName, aepMedium, aetCreated);
			if(m_bEmailDeclined) {
				AuditEvent(nPersonID, strName, audit, aeiPatientDeclinedEmail, nPersonID, "", "Declined Email Address", aepMedium, aetChanged);
			}			
			audit.Commit();
		}
		else { 
			// (d.moore 2007-08-15) - PLID 25455
			// An inquiry was selected, so we need to convert the person instead of
			//  creating a new patient.
			CString strSqlBatch = BeginSqlBatch();
						
			int nPersonID = nInquiryID;
			*m_id = nPersonID;
			
			// PatientsT.CurrentStatus
			// 1 = patient, 2 = prospect, 3 = patient/prospect, 4 = Inquiry

			// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
			CSqlTransaction trans("CNewPatient::Save from Inquiry");
			trans.Begin();
			CString strUpdateQuery, strPersonTQuery, strPatientStatusHistoryTQuery;

			CSqlFragment sqlPatientsTQuery;

			// v.arth 2009-06-01 PLID 34386 - Add country list to New Patient dialog
			// (a.walling 2010-06-11 10:19) - PLID 39008 - Don't overwrite the inquiry's email or middle if we did not put one in ourselves.
			// (a.walling 2010-06-11 10:19) - PLID 39008 - Or notes... also notes are set to single space if empty!@*)(
			// also fix some sql injection surfaces
			// (s.tullis 2016-05-25 15:41) - NX-100760  
			middle.TrimRight();
			email.TrimRight();
			CString trimNotes = notes;
			trimNotes.TrimRight();
			strPersonTQuery.Format(
				"UPDATE PersonT SET "
					"Location = %s, First = '%s', %s Last = '%s', "
					"Address1 = '%s', Address2 = '%s', City = '%s', State = '%s', "
					"Zip = '%s', HomePhone = '%s', WorkPhone = '%s', Extension = '%s', "
					"%s Gender = %li, UserID = %li, SocialSecurity = '%s', "
					"BirthDate = %s, PrivHome = %li, PrivWork = %li, %s "
					"CellPhone = '%s', PrefixID = %s, ModifiedDate = GetDate(), "
					"Country = %s "
				"WHERE ID = %li", 
				LocationID == -1 ? " NULL " : FormatString("%li",LocationID), _Q(first), middle.IsEmpty() ? "" : FormatString("Middle = '%s', ", _Q(middle)), _Q(last), _Q(add1), _Q(add2), 
				_Q(city), _Q(state), _Q(zip), _Q(home), _Q(work), _Q(ext), trimNotes.IsEmpty() ? "" : FormatString("Note = '%s', ", _Q(notes)), sex, 
				GetCurrentUserID(), _Q(ssn), dob, homepriv, workpriv, email.IsEmpty() ? "" : FormatString("Email = '%s', ", _Q(email)), _Q(cell), strPrefix, 
				strSaveCountry, nPersonID);

			//(e.lally 2008-08-29) PLID 16395 - Added preferred contact
			// (r.gonet 05/16/2012) - PLID 48561 - Added Affiliate Physician
			// (j.armen 2014-03-14 09:13) - PLID 61375 - Parameterize, added DefaultICD10DiagID1
			sqlPatientsTQuery.Create(
				"\r\n\r\n"
				"UPDATE PatientsT SET\r\n"
				"	MainPhysician = {VT_I4}, CurrentStatus = 1, ReferralID = {INT}, EmployeeID = {VT_I4}, TypeOfPatient = {VT_I4},\r\n"
				"	DefaultReferringPhyID = {VT_I4}, AffiliatePhysID = {VT_I4}, ReferringPatientID = {VT_I4}, DefaultDiagID1 = {VT_I4}, DefaultICD10DiagID1 = {VT_I4},\r\n"
				"	PreferredContact = {INT}\r\n"
				"WHERE PersonID = {INT}\r\n\r\n", 
				vtDoctor, referralID, vtCoord, vtPatTypeID,
				vtRefPhys, vtAffiliatePhys, vtRefPatient, vtDefaultDiag9ID1, vtDefaultDiag10ID1,
				nPreferredContact,
				nPersonID);
			
			long nPatientStatusHistoryID = NewNumber("PatientStatusHistoryT", "ID");
			strPatientStatusHistoryTQuery.Format(
				"INSERT INTO PatientStatusHistoryT "
					"(ID, PersonID, OldStatus, NewStatus, "
						"DateConverted, ConvertedByUserName) "
					"VALUES (%li, %li, 4, 1, GetDate(), '%s')", 
					nPatientStatusHistoryID, nPersonID, 
					_Q(GetCurrentUserName()));

			strUpdateQuery = strPersonTQuery + sqlPatientsTQuery.Flatten() + strPatientStatusHistoryTQuery;

			//
			// (c.haag 2006-08-04 15:26) - PLID 21776 - Add multiple referrals if necessary
			//
			if (m_btnMultipleReferrals.GetCheck() && m_pMultiReferralList != NULL) {
				NXDATALIST2Lib::IRowSettingsPtr p = m_pMultiReferralList->GetFirstRow();
				CString strSql;
				while (NULL != p) {
					CString strPart;
					strPart.Format(
						"IF ((SELECT COUNT(*) FROM MultiReferralsT "
							"WHERE ReferralID = %li AND PatientID = %li) = 0)\r\n"
						"INSERT INTO MultiReferralsT (ReferralID, PatientID, Date) "
							"VALUES (%li, %li, GetDate());\r\n",
						VarLong(p->Value[0]), nPersonID, 
						VarLong(p->Value[0]), nPersonID);
					strUpdateQuery += strPart;
					p = p->GetNextRow();
				}
			}

			// Also need to insert into MultiReferralsT for the new referral stuff
			if (referralID != -1 && !m_btnMultipleReferrals.GetCheck()) {
				CString strReferralQuery;
				strReferralQuery.Format(
					"IF ((SELECT COUNT(*) FROM MultiReferralsT "
							"WHERE ReferralID = %li AND PatientID = %li) = 0)\r\n"
					"INSERT INTO MultiReferralsT (ReferralID, PatientID, Date) "
						"VALUES (%li, %li, GetDate())", 
					referralID, nPersonID,
					referralID, nPersonID);
				strUpdateQuery += strReferralQuery;
			}

			// Execute the batch query.
			ExecuteSqlStd(strUpdateQuery);

			// (b.cardillo 2009-05-27 23:11) - PLID 34368 - This patient's gender just changed, so update the qualification records.
			UpdatePatientWellnessQualification_Gender(GetRemoteData(), nPersonID);
			// (b.cardillo 2009-05-28 14:55) - PLID 34369 - This patient's birthdate just changed, so update the qualification records.
			UpdatePatientWellnessQualification_Age(GetRemoteData(), nPersonID);

			// (a.walling 2006-11-14 13:25) - PLID 22715 - Also store the related patient if necessary
			long nRelPatSel = m_pRelatedPatientList->GetCurSel();
			if (nRelPatSel != NXDATALISTLib::sriNoRow) {
				long nRelPatID = VarLong(m_pRelatedPatientList->GetValue(nRelPatSel, 0), -1);
				if (nRelPatID > 0) {
					long nFamilyID = FamilyUtils::GetFamilyID(nRelPatID);
					if (nFamilyID == -1) { // relative not in family
						// create a new family record and insert the patient
						nFamilyID = FamilyUtils::CreateNewFamilyAndRelationship(nPersonID, nRelPatID);
					} else {
						// save the data into PersonFamilyT
						FamilyUtils::InsertRelationship(nPersonID, nRelPatID, nFamilyID);
						// since this is a new patient we can be guaranteed that they do not already belong to a different family.
					}
				}
			}

			// (a.walling 2007-05-21 11:00) - PLID 26079 - Apply any referred patient points
			try {
				if (vtRefPatient != g_cvarNull) {
					Rewards::ApplyRefPatient(nPersonID, VarLong(vtRefPatient)); // points go to nRefPatient
				}
			} NxCatchAll("Error applying referred patient rewards.");

			trans.Commit();

			//auditing
			CString strName;
			strName.Format("%s, %s %s", last, first, middle);
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(nPersonID, strName, nAuditID, aeiPatientCreated, nPersonID, "", strName, aepMedium, aetCreated);
		}
	}NxCatchAllCall("Error saving new patient.",{return false;});

	// (j.gruber 2009-10-12 13:00) - PLID 10723 - Add insurance
	// (c.haag 2010-10-04 15:20) - PLID 39447 - Write multiple parties
	if(!UseOHIP() && m_aInsuredParties.GetSize() > 0)
	{
		CString strPatientName = last + ", " + first + " " + middle;
		BOOL bHadErrors = FALSE;
		for (int iParty=0; iParty < m_aInsuredParties.GetSize(); iParty++) 
		{
			CNewPatientInsuredParty& party = m_aInsuredParties[iParty];
			long nInsPartyID, nInsRespTypeID, nInsCoPersonID;
			_variant_t varInsPlanID;

			if (party.m_strRelationToPt != "Self")
			{
				// (j.jones 2012-10-25 09:39) - PLID 36305 - added Title
				// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
				// (r.goldschmidt 2014-07-24 16:33) - PLID 63111 - added insurance pay group information
				// (r.gonet 2015-11-12 11:14) - PLID 66907 - We don't want an HL7 message to be generated before
				// the patient has been fully saved. Pass false to prevent the function from sending a message.
				if (!CreateNewInsuredPartyRecord(*m_id, strPatientName, nInsPartyID, varInsPlanID, nInsRespTypeID,
					nInsCoPersonID, party.GetInsuranceCompanyID(), -1, party.GetRespTypeID(), party.m_strPatientIDNumber, party.m_strGroupNumber,
					party.m_bPerPayGroup, party.m_cyTotalDeductible, party.m_cyTotalOOP, party.m_mapPayGroupVals, party.m_strRelationToPt,
					party.m_strInsFirst, party.m_strInsMiddle, party.m_strInsLast, party.m_strInsTitle,
					party.m_strInsAddress1, party.m_strInsAddress2, party.m_strInsCity, party.m_strInsState, party.m_strInsZip, party.m_strInsCountry, party.m_strInsPhone,
					party.m_strInsEmployer, party.m_nInsGender, party.m_dtInsBirthDate, party.m_strInsSSN, false)) 
				{
					bHadErrors = TRUE;					
				}
			}
			else {

				//first create the record
				// (r.goldschmidt 2014-07-24 16:33) - PLID 63111 - added insurance pay group information
				// (r.gonet 2015-11-12 11:14) - PLID 66907 - We don't want an HL7 message to be generated before
				// the patient has been fully saved. Pass false to prevent the function from sending a message.
				if (!CreateNewInsuredPartyRecord(*m_id, strPatientName, nInsPartyID, varInsPlanID, nInsRespTypeID,
					nInsCoPersonID, party.GetInsuranceCompanyID(), -1, party.GetRespTypeID(), party.m_strPatientIDNumber, party.m_strGroupNumber,
					party.m_bPerPayGroup, party.m_cyTotalDeductible, party.m_cyTotalOOP, party.m_mapPayGroupVals, "Self", "", "", "", "", "", "", "", "", "", "", "", "", 0, COleDateTime(0, 0, 0, 0, 0, 0), "", false))
				{
					bHadErrors = TRUE;
				}
				else {
					//now copy the patient information
					// (r.gonet 2015-11-12 11:14) - PLID 66907 - We don't want an HL7 message to be generated before
					// the patient has been fully saved. Pass false to prevent the function from sending a message.
					if (!CopyPatientInfoToInsuredParty(*m_id, nInsPartyID, strPatientName, false)) 
					{
						bHadErrors = TRUE;
					}
				}
			}
		} // for (int iParty=0; i < m_aInsuredParties.GetSize(); iParty++) 

		if (bHadErrors) {
			AfxMessageBox("There was an error creating one or more insured parties. Please go to the insurance tab and try entering the insured party information again.", MB_OK | MB_ICONERROR);
		}
	}

	if (bUseNextAvailableID) {
		MsgBox(MB_ICONINFORMATION|MB_OK, "This patient was saved with an ID of %li.", id);
	}

	if (m_inform.GetCheck()) {
		try {
			UpdateInform((ULONG)(*m_id));
		} NxCatchAll("Error sending patient to Inform.");
	}
	if (m_mirror.GetCheck())
	{
		try
		{
			Mirror::Export(id, CString("")); 
		}NxCatchAll("Could not add patient to mirror");
	}	
	if (m_united.GetCheck())
	{
		UpdateUnited((ULONG)(*m_id));
	}

	// (r.gonet 2015-11-12 11:14) - PLID 66907 - Moved HL7 message sending to the end of the function
	// so all saving will be done first before sending.

	m_groupChecker.Refresh();

	if (m_pAddProDlg) {
		m_pAddProDlg->Refresh();
		if(m_pAddProDlg->m_bProcedures) {
			PhaseTracking::AddPtntProcedures(m_pAddProDlg->m_arProcIDs, *m_id);
		}
		else {
			PhaseTracking::AddProcedureGroups(m_pAddProDlg->m_arProcGroupIDs, *m_id);
		}
	}

	SaveGroups();

	// (d.thompson 2009-03-19) - PLID 33061 - Added a preference to set a default insured party for OHIP users.  We will
	//	do this after all the normal patient saving has been done.
	// (j.jones 2010-11-08 13:50) - PLID 39620 - supported this for Alberta as well, which only fills the health card
	if(UseOHIP() || UseAlbertaHLINK()) {
		// (j.jones 2010-05-06 14:39) - PLID 38482 - the ID For Ins & Group Number fields
		// are the Health Card and Version Code in OHIP, always save regardless of creating an insurance company
		CString strHealthCardNumber, strVersionCode;
		GetDlgItemText(IDC_NEWPAT_INS_ID_BOX, strHealthCardNumber);
		GetDlgItemText(IDC_NEWPAT_INS_FECA_BOX, strVersionCode);

		SaveCustomInfo(*m_id, m_nHealthNumberCustomField, strHealthCardNumber);
		// (j.jones 2010-11-08 13:50) - PLID 39620 - the version code is OHIP only
		if(UseOHIP() && m_nVersionCodeCustomField != -1) {
			SaveCustomInfo(*m_id, m_nVersionCodeCustomField, strVersionCode);
		}
		
		if(GetRemotePropertyInt("SetDefaultInsCoOnNewPatients", 0, 0, "<None>", true)) {
			long nDefaultInsCoID = GetRemotePropertyInt("DefaultInsuranceCoNewPatient", -1, 0, "<None>", true);
			if(nDefaultInsCoID != -1) {
				//There may be a plan too
				long nDefaultPlanID = GetRemotePropertyInt("DefaultInsurancePlanNewPatient", -1, 0, "<None>", true);

				//Add an insured party record
				long nNewInsPartyID, nNewRespTypeID, nNewInsCoID;
				_variant_t varNewPlan;
				CString strPatientName = last + ", " + first + " " + middle;
				// (r.gonet 2015-11-12 11:14) - PLID 66907 - We don't want an HL7 message to be generated before
				// the patient has been fully saved. Pass false to prevent the function from sending a message.
				if(!CreateNewInsuredPartyRecord(*m_id, strPatientName, nNewInsPartyID, varNewPlan, nNewRespTypeID, nNewInsCoID,
					//Override with our preferences.  And since this is a new patient who cannot possibly have insurance, we
					//	want this to always be primary.
					nDefaultInsCoID, nDefaultPlanID, 1, "", "", false, 
					InsuredPartyPayGroupValues().m_cyTotalDeductible, InsuredPartyPayGroupValues().m_cyTotalOOP, CMap<long, long, InsuredPartyPayGroupValues, InsuredPartyPayGroupValues>(10),
					"", "", "", "", "", "", "", "", "", "", "", "", "", 0, COleDateTime(0, 0, 0, 0, 0, 0), "", false))
				{
					//failure!  There's not really anything we can do, so just alert the user.  The most likely cause is that
					//	an invalid insurance company is set in the preference data.
					AfxMessageBox("Your preference settings dictate that an insured party record should be created for this patient.  However, "
						"there was a failure while attempting to create this data.  Please check that you have the preference properly setup.  "
						"You will need to create the record for this patient manually.");
				}
				else {
					//Success!
					// (r.goldschmidt 2014-07-31 17:45) - PLID 63111 - Rework saving of Default Insured Party Pay Groups and Insurance Company Deductibles when creating New Patient
					InsertDefaultDeductibles(nNewInsCoID, nNewInsPartyID);
					//Now we need to copy the patient information into the insured party
					if(!CopyPatientInfoToInsuredParty(*m_id, nNewInsPartyID, strPatientName, false)) {
						//Failed to copy the patient info.  There's nothing we can do about this, so just alert the user.
						AfxMessageBox("A default insured party was created for this new patient.  However, the patient information "
							"could not be copied to the insured party record.  Please review the insured party record and manually "
							"copy the patient information.");
					}
				}
			}
		}
	}

	// (r.gonet 2015-11-12 11:14) - PLID 66907 - Moved HL7 message sending to the end of the function
	// so all saving will be done first before sending.
	if (IsDlgButtonChecked(IDC_HL7)) {
		CArray<HL7SettingsGroup, HL7SettingsGroup&> arDefaultGroups;
		//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 Settings
		CArray<long, long> arDefaultGroupIDs;
		GetHL7SettingsGroupsBySetting("ExportNewPatients", TRUE, arDefaultGroupIDs);
		for (int i = 0; i < arDefaultGroupIDs.GetSize(); i++) {
			long nGroupID = arDefaultGroupIDs[i];
			HL7SettingsGroup hsg;
			hsg.nID = nGroupID;
			hsg.nExportType = GetHL7SettingInt(nGroupID, "ExportType");
			hsg.bExpectAck = GetHL7SettingBit(nGroupID, "ExpectAck");
			hsg.strName = GetHL7GroupName(nGroupID);
			arDefaultGroups.Add(hsg);
		}
		// v.arth 2009-06-01 PLID 34386 - Add country list to New Patient dialog
		if (arDefaultGroups.GetSize()) {
			// (r.gonet 12/03/2012) - PLID 54104 - We used to get all the patient demographics here
			//  in a query and passed that to an HL7 message generation function, but NxServer does that
			//  now.
			for (int i = 0; i < arDefaultGroups.GetSize(); i++) {
				HL7SettingsGroup hsg = arDefaultGroups[i];
				BOOL bExpectAck = (hsg.nExportType == 1 && hsg.bExpectAck);
				//TES 9/21/2010 - PLID 40595 - If we're excluding prospects, and this patient is a prospect, then silently skip the export.
				// It might be slightly better to enable/disable the HL7 box as they change the status, but this is a single-client feature
				// and likely to remain so, so I'm not going to the trouble.  I'm also not changing HL7Settings group, as we've already
				// got the global CHL7Settings cache so it's really kind of obsolete at this point.
				//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
				if (!GetHL7SettingBit(hsg.nID, "ExcludeProspects") || status != 2) {
					// (j.jones 2008-05-19 16:26) - PLID 30110 - ensure the function sends a tablechecker
					// (r.gonet 12/03/2012) - PLID 54104 - Updated to use the refactored send function.
					// (r.gonet 12/11/2012) - PLID 54104 - Have CMainFrame handle any failures.
					GetMainFrame()->GetHL7Client()->SendNewPatientHL7Message(*m_id, hsg.nID, true);
				}
			}
		}

	}

	}NxCatchAllCall("Error in NewPatient::Save()",{return false;});

	return true;
}

void CNewPatient::OnCancel()
{
	//check to see if they entered anything in the first or last name fields
	CString strFirst, strLast;
	GetDlgItemText(IDC_FIRST_NAME_BOX, strFirst);
	GetDlgItemText(IDC_LAST_NAME_BOX, strLast);

	if (!strFirst.IsEmpty() || !strLast.IsEmpty()) {
	
		//check to make sure they want to cancel
		if (MsgBox(MB_YESNO, "Are you sure you want to cancel and lose the information entered?") == IDYES) {
			GetMainFrame()->EnableHotKeys();
			//Tell main frame not to send us any more table checker messages.
			GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
			EndDialog(0);	
		}
		
	}
	else {
		//just end it
		//Tell main frame not to send us any more table checker messages.
		GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
		GetMainFrame()->EnableHotKeys();
		EndDialog(0);	
	}

}

void CNewPatient::OnChangeExtPhoneBox() 
{
	FormatItem (IDC_EXT_PHONE_BOX, "nnnnnnn");
}

void CNewPatient::OnChangeHomePhoneBox() 
{
	GetDlgItemText(IDC_HOME_PHONE_BOX, str);
	str.TrimRight();
	if (str != "") {
		if(m_bFormatPhoneNums) {
			FormatItem (IDC_HOME_PHONE_BOX, m_strPhoneFormat);
		}
	}
}

void CNewPatient::OnChangeWorkPhoneBox() 
{
	GetDlgItemText(IDC_WORK_PHONE_BOX, str);
	str.TrimRight();
	if (str != "") {
		if(m_bFormatPhoneNums) {
			FormatItem (IDC_WORK_PHONE_BOX, m_strPhoneFormat);
		}
	}
}

void CNewPatient::OnChangeCellPhoneBox() 
{
	GetDlgItemText(IDC_CELL_PHONE_BOX, str);
	str.TrimRight();
	if (str != "") {
		if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
			FormatItem (IDC_CELL_PHONE_BOX, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
		}
	}
}

void CNewPatient::OnChangeZipBox() 
{
	// (d.moore 2007-04-23 12:11) - PLID 23118 - 
	//  Capitalize letters in the zip code as they are typed in. Canadian postal
	//    codes need to be formatted this way.
	CapitalizeAll(IDC_ZIP_BOX);
	//FormatItem (IDC_ZIP_BOX, "#####-nnnn");
}

void CNewPatient::OnKillfocusZipBox() 
{
	// (j.gruber 2009-10-07 17:14) - PLID 35826 - updated for city lookup
	if (!m_bLookupByCity) {
		CString city, 
				state,
				tempCity,
				tempState,
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

			// (s.tullis 2013-10-21 10:10) - PLID 45031 - If 9-digit zipcode match fails compair it with the 5-digit zipcode.
			if(city == "" && state == ""){				
				CString str;
				str = value.Left(5);// Get the 5 digit zip code
				GetZipInfo(str, &city, &state);
				// (b.savon 2014-04-03 13:02) - PLID 61644 - If you enter a 9
				//digit zipcode in the locations tab of Administrator, it looks
				//up the city and state based off the 5 digit code, and then 
				//changes the zip code to 5 digits. It should not change the zip code.
			}
			if(tempCity == "") 
				SetDlgItemText(IDC_CITY_BOX, city);
			if(tempState == "")
				SetDlgItemText(IDC_STATE_BOX, state);
		}
	}
}

void CNewPatient::OnKillfocusCityBox() 
{
	try {
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
				if(AfxMessageBox("You have changed the city but the postal code or state already have data in them.  Would you like to overwrite "
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
				if(tempState == "")
					SetDlgItemText(IDC_STATE_BOX, state);
			}
		}
	}NxCatchAll(__FUNCTION__);
}


//(e.lally 2005-06-06) - PLID 16365 - If ssn has a value of '###-##-####' make it empty
void CNewPatient::OnKillfocusSsnBox() 
{
	CString strSsn;
	GetDlgItemText(IDC_SSN_BOX, strSsn);
	if(strSsn == "###-##-####") {
		strSsn="";
		FormatItemText(GetDlgItem(IDC_SSN_BOX),strSsn,"");
	}
}

void CNewPatient::OnChangeFirstNameBox() 
{
	Capitalize(IDC_FIRST_NAME_BOX);
}

void CNewPatient::OnChangeMiddleNameBox() 
{
	// (c.haag 2006-08-02 11:40) - PLID 21740 - We now check for auto-capitalization
	// for middle name boxes
	if (GetRemotePropertyInt("AutoCapitalizeMiddleInitials", 1, 0, "<None>", true)) {
		Capitalize(IDC_MIDDLE_NAME_BOX);
	}
}

void CNewPatient::OnChangeLastNameBox() 
{
	Capitalize(IDC_LAST_NAME_BOX);
}

void CNewPatient::OnChangeAddress2Box() 
{
	Capitalize(IDC_ADDRESS2_BOX);
}

void CNewPatient::OnChangeAddress1Box() 
{
	Capitalize(IDC_ADDRESS1_BOX);	
}

void CNewPatient::OnChangeSSN() 
{
	GetDlgItemText (IDC_SSN_BOX, str);
	FormatItemText (GetDlgItem(IDC_SSN_BOX), str, "###-##-####");
}

// (v.maida - 2014-02-06 15:15) - PLID 60120 - PreTranslateMessage() removed because it's no longer needed to handle Alt-key combinations.
// Keeping it in results in it attempting to save patients twice.
/*BOOL CNewPatient::PreTranslateMessage(MSG* pMsg) 
{
	
	if (pMsg->message == WM_SYSKEYDOWN)
		switch (pMsg->wParam)
		{	case 'S':
				OnSaveAndSchedule();
				break;
			case 'E':
				OnSaveAndEdit();
				break;
			case 'R':
				OnSaveAndResume();
				break;
			case 'C':
				OnCancel();
				break;
			default:
				break;
		}
	
	return CNxDialog::PreTranslateMessage(pMsg);
}*/

void CNewPatient::OnProcedures() 
{
	if (!m_pAddProDlg)
		m_pAddProDlg = new CAddProcedureDlg(this);
	m_pAddProDlg->DoModal();

	extern CPracticeApp theApp;

	//PLID 21774 - check to see if they picked any procedures
	if ((m_pAddProDlg->m_arProcIDs.GetSize() == 0) && m_pAddProDlg->m_arProcGroupIDs.GetSize() == 0) {
		//turn it black
		m_procedureBtn.SetTextColor(RGB(0,0,0));
	}
	else {
		//turn the button red
		m_procedureBtn.SetTextColor(RGB(255,0,0));
		
	}
}

void CNewPatient::OnSaveAndSchedule() 
{
	if (Save()) {
		GetMainFrame()->EnableHotKeys();
		//Tell main frame not to send us any more table checker messages.
		GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
		EndDialog(3);
	}	
}

void CNewPatient::OnSaveAndEdit() 
{
	// (c.haag 2005-03-22 10:26) - PLID 15298 - If we came from the resentry dialog and
	// we are not allowed to save and edit further, don't let the user do it by pressing
	// Alt-E.
	if(m_bFromResEntry) {
		if (!GetRemotePropertyInt("AllowSaveAndEditFurtherInScheduler", 0, 0, GetCurrentUserName(), true))
			return;

		if (IDNO == MsgBox(MB_YESNO, "The appointment you came from will be saved with this new patient assigned to it. Do you wish to continue?"))
			return;
	}

	if (Save()) {
		GetMainFrame()->EnableHotKeys();
		//Tell main frame not to send us any more table checker messages.
		GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
		EndDialog(2);
	}	
}

void CNewPatient::OnSaveAndResume() 
{
	if (Save()) {
		GetMainFrame()->EnableHotKeys();
		//Tell main frame not to send us any more table checker messages.
		GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
		EndDialog(1);
	}	
}
void CNewPatient::OnGroups() 
{
	m_dlgGroups.m_bAutoWriteToData = FALSE;
	m_dlgGroups.DoModal();
}

void CNewPatient::SaveGroups()
{
	BEGIN_TRANS("Save Groups") {
		//Just tell the dialog to save them.
		if(m_groupChecker.Changed()) 
			m_dlgGroups.Refresh();
		m_dlgGroups.m_nPatID = *m_id;
		m_dlgGroups.StoreData_Flat();
	}END_TRANS_CATCH_ALL("Save Groups");
}

BOOL CNewPatient::OnCommand(WPARAM wParam, LPARAM lParam) 
{

	switch (HIWORD(wParam)) {

		

		case EN_KILLFOCUS:
			{
			int nID = LOWORD(wParam);

			switch(nID) {

				//(c.copits 2010-10-12) PLID 27344 - Truncate trailing space on names and addresses
				case IDC_FIRST_NAME_BOX:
					if (GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true)) {
						CString strFirstName;
						GetDlgItemText(IDC_FIRST_NAME_BOX, strFirstName);
						strFirstName.TrimRight();
						SetDlgItemText(IDC_FIRST_NAME_BOX, strFirstName);
					}
					// (r.goldschmidt 2014-07-17 16:44) - PLID 62774
					if (ReadyToCheckForDuplicatePatients()){
						CheckForDuplicatePatients(); 
					}
					break;
				case IDC_MIDDLE_NAME_BOX:
					if (GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true)) {
						CString strMiddleName;
						GetDlgItemText(IDC_MIDDLE_NAME_BOX, strMiddleName);
						strMiddleName.TrimRight();
						SetDlgItemText(IDC_MIDDLE_NAME_BOX, strMiddleName);
					}
					// (r.goldschmidt 2014-07-17 16:44) - PLID 62774 - Don't check on middle name change
					break;
				case IDC_LAST_NAME_BOX:
					if (GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true)) {
						CString strLastName;
						GetDlgItemText(IDC_LAST_NAME_BOX, strLastName);
						strLastName.TrimRight();
						SetDlgItemText(IDC_LAST_NAME_BOX, strLastName);
					}
					// (r.goldschmidt 2014-07-17 16:44) - PLID 62774
					if (ReadyToCheckForDuplicatePatients()){
						CheckForDuplicatePatients();
					}
					break;
				case IDC_ADDRESS1_BOX:
					if (GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true)) {
						CString strAddress;
						GetDlgItemText(IDC_ADDRESS1_BOX, strAddress);
						strAddress.TrimRight();
						SetDlgItemText(IDC_ADDRESS1_BOX, strAddress);
					}
					break;
				case IDC_ADDRESS2_BOX:
					if (GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true)) {
						CString strAddress;
						GetDlgItemText(IDC_ADDRESS2_BOX, strAddress);
						strAddress.TrimRight();
						SetDlgItemText(IDC_ADDRESS2_BOX, strAddress);
					}
					
				case IDC_CITY_BOX:
					if (GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true)) {
						CString strCity;
						GetDlgItemText(IDC_CITY_BOX, strCity);
						strCity.TrimRight();
						SetDlgItemText(IDC_CITY_BOX, strCity);
					}
					break;
				case IDC_STATE_BOX:
					if (GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true)) {
						CString strState;
						GetDlgItemText(IDC_STATE_BOX, strState);
						strState.TrimRight();
						SetDlgItemText(IDC_STATE_BOX, strState);
					}
					break;
				case IDC_ZIP_BOX:
					if (GetRemotePropertyInt("TruncateTrailingNameAddressSpace", 1, 0, "<None>", true)) {
						CString strZip;
						GetDlgItemText(IDC_ZIP_BOX, strZip);
						strZip.TrimRight();
						SetDlgItemText(IDC_ZIP_BOX, strZip);
					}
					break;

				case IDC_HOME_PHONE_BOX:
				case IDC_WORK_PHONE_BOX:
				case IDC_CELL_PHONE_BOX:
					if (SaveAreaCode(nID)) {
					//	Save(nID);
					}
				break;
				default:
				break;
			}
			}

		break;

		case EN_SETFOCUS:
			{
			int nID = LOWORD(wParam);

			switch(nID) {
				case IDC_HOME_PHONE_BOX:
				case IDC_WORK_PHONE_BOX:
				case IDC_CELL_PHONE_BOX:
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

	return CNxDialog::OnCommand(wParam, lParam);
}



bool CNewPatient::SaveAreaCode(long nID) {

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

void CNewPatient::FillAreaCode(long nID)  {

		//first check to see if anything is in this box
		CString strPhone;
		GetDlgItemText(nID, strPhone);
		if (! ContainsDigit(strPhone)) {
			// (j.gruber 2009-10-07 17:17) - PLID 35826 - updated for city lookup
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

BOOL CNewPatient::Is640x480Resource()
{
	// (z.manning, 04/16/2008) - PLID 29680 - Removed all references to obsolete 640x480 dialogs
	return FALSE;
}

HBRUSH CNewPatient::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr;
	COLORREF clr, clrDefault = 0x009CE3F5, clrRed = RGB(255,0,0), clrBlue = RGB(0,0,256), clrDlgBkg = GetSysColor(COLOR_BTNFACE);
	BOOL bSetTextColor = FALSE;

	// If this is an edit control, we need to color it with a orange tint
	// if it's the base for a required field.
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		bSetTextColor = TRUE;
		clr = clrDefault;
		switch (pWnd->GetDlgCtrlID())
		{
		case IDC_FIRST_NAME_REQ:			
		case IDC_LAST_NAME_REQ:
			clr = clrRed;
			break;
		case IDC_MIDDLE_NAME_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_MIDDLE) clr = clrRed;
			break;
		case IDC_ADDRESS1_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_ADDRESS1) clr = clrRed;
			break;
		case IDC_CITY_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_CITY) clr = clrRed;
			break;
		case IDC_STATE_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_STATE) clr = clrRed;
			break;
		case IDC_ZIP_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_ZIP) clr = clrRed;
			break;
		case IDC_SSN_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_SSN) clr = clrRed;
			break;
		case IDC_DOB_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_DOB) clr = clrRed;
			break;
		case IDC_DOCTOR_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_DOCTOR) clr = clrRed;
			break;
		case IDC_COORDINATOR_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_COORDINATOR) clr = clrRed;
			break;
		case IDC_GENDER_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_GENDER) clr = clrRed;
			break;
		case IDC_HOME_PHONE_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_HOMENO) clr = clrRed;
			//(e.lally 2007-06-21) PLID 24334 - Color additional required fields
			else if(m_lAdditionalFields & NEWPT_ADDITIONAL_HOMENO) clr = clrBlue;
			break;
		case IDC_WORK_PHONE_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_WORKNO) clr = clrRed;
			//(e.lally 2007-06-21) PLID 24334 - Color additional required fields
			else if(m_lAdditionalFields & NEWPT_ADDITIONAL_WORKNO) clr = clrBlue;
			break;
		case IDC_REFERRAL_REQ://Not on NxColor
			if (m_lRequiredFields & NEWPT_REQUIRED_REFERRAL) clr = clrRed;
			else clr = clrDlgBkg;
			break;
		case IDC_REF_PHYS_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_REF_PHYS) clr = clrRed;
			break;
		case IDC_REF_PAT_REQ:
			//DRT 6/28/2005 - PLID 16745 - If the field is disabled, even if they've tried to require it
			//	we're not going to enable it, so hide this flag, so it does not confuse the user.
			if ( (m_lRequiredFields & NEWPT_REQUIRED_REF_PAT) && m_pRefPatientList != NULL) clr = clrRed;
			break;
		case IDC_NP_AFFILIATE_PHYS_REQUIRE_LBL:
			// (r.gonet 05/16/2012) - PLID 48561 - Conditionally require the affiliate physician
			if( (m_lRequiredFields & NEWPT_REQUIRED_AFFILIATE_PHYS) && m_pAffiliatePhysList != NULL) clr = clrRed;
			break;
		case IDC_PATIENT_TYPE_REQ://Not on NxColor
			if (m_lRequiredFields & NEWPT_REQUIRED_PAT_TYPE) clr = clrRed;
			else clr = clrDlgBkg;
			break;
		case IDC_PROCEDURE_REQ://Not on NxColor
			if (m_lRequiredFields & NEWPT_REQUIRED_PROCEDURE) clr = clrRed;
			else clr = clrDlgBkg;
			break;
		case IDC_EMAIL_REQ:
			// (z.manning 2009-07-08 15:36) - PLID 27251 - This is now handled in OnInitDialog
			/*if (m_lRequiredFields & NEWPT_REQUIRED_EMAIL) clr = clrRed;
			//(e.lally 2007-06-21) PLID 24334 - Color additional required fields
			else if(m_lAdditionalFields & NEWPT_ADDITIONAL_EMAIL) clr = clrBlue;*/
			break;
		case IDC_CELL_PHONE_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_CELL) clr = clrRed;
			//(e.lally 2007-06-21) PLID 24334 - Color additional required fields
			else if(m_lAdditionalFields & NEWPT_ADDITIONAL_CELL) clr = clrBlue;
			break;
		case IDC_NOTE_REQ:
			if (m_lRequiredFields & NEWPT_REQUIRED_NOTE) clr = clrRed;
			break;
		case IDC_PREFIX_REQ:
			// (c.haag 2009-03-02 16:13) - PLID 17142 - Prefix
			if (m_lRequiredFields & NEWPT_REQUIRED_PREFIX) clr = clrRed;
			break;
		case IDC_LOCATION_PAT_REQ:
			// (s.dhole 2013-06-04 13:26) - PLID 12018 Location
			if ( (m_lRequiredFields & NEWPT_REQUIRED_LOCATION) && m_pDefaultLocation != NULL) clr = clrRed;
			break;
		case IDC_COUNTRY_REQ:
			// (r.goldschmidt 2014-11-18 12:11) - PLID 64032 - country req missing asterisk
			if (m_lRequiredFields & NEWPT_REQUIRED_COUNTRY) clr = clrRed;
			break;
		default:
			bSetTextColor = FALSE;
			break;
		}
	}

	hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	if (bSetTextColor)
	{
		pDC->SetTextColor(clr);
	}

	return hbr;
}

void CNewPatient::OnSelChosenPrefix(long nRow) 
{
	try {

		// (j.jones 2016-02-25 15:59) - PLID 67549 - moved the -1 check to be the first line of code
		if (nRow == -1)
			return;

		//check the preference first
		if(GetRemotePropertyInt("GenderPrefixLink", 1, 0, "<None>", true) == 0)
			return;

		if(nRow == 0) {
			//empty row
			m_PrefixCombo->PutCurSel(-1);
			return;
		}

		//we default to Ms., so change the gender if they change this
		long nGender = VarLong(m_PrefixCombo->GetValue(nRow, 2), 0);
		
		if(nGender == 1 || nGender == 2) {
			m_GenderCombo->CurSel = nGender;
		}

	}NxCatchAll("Error setting default gender.");
}

void CNewPatient::OnSelChosenGender(long nRow) 
{
	try {

		// (j.jones 2016-02-25 15:59) - PLID 67549 - moved the -1 check to be the first line of code
		if (nRow == -1) {
			m_GenderCombo->SetSelByColumn(gccID, (long)0);
			return;
		}

		//check the preference first
		if(GetRemotePropertyInt("GenderPrefixLink", 1, 0, "<None>", true) == 0)
			return;

		//we default to female, so we always want to change their prefix
		long nDefaultPrefix = -1;
		switch(nRow) {
		case 1:
			//male
			if(m_PrefixCombo->CurSel == -1 || VarLong(m_PrefixCombo->GetValue(m_PrefixCombo->CurSel, 2)) != 1) {
				nDefaultPrefix = GetRemotePropertyInt("DefaultMalePrefix", 1, 0, "<None>", true);
			}			
			break;
		case 2:
			//female
			if(m_PrefixCombo->CurSel == -1 || VarLong(m_PrefixCombo->GetValue(m_PrefixCombo->CurSel, 2)) != 2) {
				nDefaultPrefix = GetRemotePropertyInt("DefaultFemalePrefix", 4, 0, "<None>", true);
			}
			break;
		default:
			//nothing
			if(m_PrefixCombo->CurSel == -1 || VarLong(m_PrefixCombo->GetValue(m_PrefixCombo->CurSel, 2)) != 0) {
				nDefaultPrefix = GetRemotePropertyInt("DefaultNeuterPrefix", -1, 0, "<None>", true);
			}
			break;
		}
		if(nDefaultPrefix != -1) {
			m_PrefixCombo->SetSelByColumn(0,nDefaultPrefix);
		}

	}NxCatchAll("Error setting default prefix.");
}

LRESULT CNewPatient::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		switch(wParam) {
		case NetUtils::Groups:
			try {
				if(m_dlgGroups)
					m_dlgGroups.Refresh();
			} NxCatchAll("Error in CNewPatient::OnTableChanged:Groups");
			break;
		}
	} NxCatchAll("Error in CNewPatient::OnTableChanged");

	return 0;
}

void CNewPatient::OnBtnEditPatientTypes()
{
	try {
		// (b.spivey, May 14, 2012) - PLID 50224 - Deprecated old combobox code. 
		_variant_t value;
		long curSel = m_PatientTypeCombo->CurSel;
		if (curSel != -1)
		{
			value = m_PatientTypeCombo->Value[curSel][0];
		}

		//New Dialog at work here. 
		CEditPatientTypeDlg dlg; 
		dlg.DoModal(); 

		//Requery after the previous dialog. The previous dialog used to pass the list by pointer, but the new dialog 
		//   is completely independent. 
		// (b.spivey, June 07, 2012) - PLID 50224 - Forgot to call requery here. 
		m_PatientTypeCombo->Requery(); 
		m_PatientTypeCombo->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);

		//The old logic put in a "No Type selected" entry here, which caused a non-vital bug that would keep 
		//	resetting your selection once you left the editcombodlg. The new dialog implementation fixes this.
		if (curSel != -1) {
			m_PatientTypeCombo->SetSelByColumn(0, value);
		}

		GetDlgItem(IDC_PATIENT_TYPE_COMBO)->SetFocus();
	}NxCatchAll(__FUNCTION__); 
}

void CNewPatient::OnSaveAndAddAnother()
{
	if (Save()) {
		GetMainFrame()->EnableHotKeys();
		//Tell main frame not to send us any more table checker messages.
		GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
		EndDialog(4);
	}
}

void CNewPatient::OnSaveAndFFA()
{
	if (Save()) {
		GetMainFrame()->EnableHotKeys();
		//Tell main frame not to send us any more table checker messages.
		GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
		EndDialog(5);
	}
}

void CNewPatient::OnSelChosenRefPatientList(long nRow) 
{
	try {
		//DRT 11/24/2003 - PLID 9574 - Added a preference to auto-add a referral source when you choose this.
		//	We only want to do this here if no referral source is selected currently.
		// (b.eyers 8/18/2014) - PLID 63050 - don't add the default referral source if 'no patient selected' was choosen for referred by a patient
		CString strRefPat = VarString(m_pRefPatientList->GetValue(nRow, 1));
		long nDefReferral = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
		if (nDefReferral != -1 && strRefPat != "<No Referring Patient>") {
			long nReferralID = m_pdlgReferralSubDlg->GetSelectedReferralID();
			// (r.goldschmidt 2014-09-22 17:07) - PLID 31191 - Check for possible conflict with parent referral preference
			//   If selection not allowed, throw a message box, no referral source gets added, DefaultPatientReferral preference remains intact.
			CString strWarning;
			long nIsRestricted = m_pdlgReferralSubDlg->ReferralRestrictedByPreference(nDefReferral, strWarning);
			if (nIsRestricted == 0) { // selection allowed
				if (nReferralID <= 0) {
					//set the selection to our default
					m_pdlgReferralSubDlg->SelectReferralID(nDefReferral);
				}
				// (c.haag 2006-08-11 16:04) - PLID 21776 - Add it to the multi-referral list if necessary
				if (NULL == m_pMultiReferralList->FindByColumn(0, nDefReferral, NULL, false)) {
					AddMultiReferral(nDefReferral, TRUE);
				}
			}
			else { // selection not allowed
				strWarning += " The default referring patient referral source conflicts with these preferences.\n\nPlease manually add the desired referral source and review preference settings.";
				AfxMessageBox(strWarning);
			}
		}

	} NxCatchAll("Error in OnSelChosenRefPatientList()");
}

void CNewPatient::OnClickInquiry() {

	try {

		// (j.gruber 2008-09-08 13:29) - PLID 30899 - see if they started adding information
		CString strFirst, strLast, strMiddle, strEmail, strNote;
		GetDlgItemText(IDC_FIRST_NAME_BOX, strFirst);
		GetDlgItemText(IDC_MIDDLE_NAME_BOX, strMiddle);
		GetDlgItemText(IDC_LAST_NAME_BOX, strLast);
		// (z.manning 2009-07-09 17:33) - PLID 33644 - ignore email field if it's read-only as that
		// means they declined it. Inquiries do not support the declined email option at this point.
		if((m_nxeditEmailBox.GetStyle() & ES_READONLY) == 0) {
			GetDlgItemText(IDC_EMAIL_BOX, strEmail);
		}
		GetDlgItemText(IDC_NOTES, strNote);

		//referrals
		long nReferralID = -1;
		if (!m_btnMultipleReferrals.GetCheck() && m_pdlgReferralSubDlg != NULL)
		{
			nReferralID = m_pdlgReferralSubDlg->GetSelectedReferralID();
			
		}
		else if (m_btnMultipleReferrals.GetCheck() && m_pMultiReferralList != NULL) {
			
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMultiReferralList->FindByColumn(0, m_nPrimaryMultiReferralID, m_pMultiReferralList->GetFirstRow(), false);
			nReferralID = m_nPrimaryMultiReferralID;			
		}
		
		long nResult;
		// (j.gruber 2009-06-01 11:19) - PLID 34424 - fixed error if you don't have tracking
		BOOL bCheck;
		if (m_pAddProDlg) {
			bCheck = strFirst.IsEmpty() && strLast.IsEmpty() && strMiddle.IsEmpty() && strEmail.IsEmpty() && strNote.IsEmpty() && 
			(m_pAddProDlg->m_arProcIDs.GetSize() == 0 && m_pAddProDlg->m_arProcGroupIDs.GetSize() == 0) &&
			nReferralID == -1;
		}
		else {
			bCheck = strFirst.IsEmpty() && strLast.IsEmpty() && strMiddle.IsEmpty() && strEmail.IsEmpty() && strNote.IsEmpty() && nReferralID == -1;
		}			
		if (bCheck) {				

			//they haven't changed anything, don't prompt for anything
			CInquiryDlg dlg(this);
			nResult = dlg.DoModal();
		}
		else {
			//they selected something, ask if they want it to carry over
			if (IDYES == MsgBox(MB_YESNO, "You have begun filling in some fields, would you like to carry these over to the new inquiry?") ) {

				
				/*CArray<int,int> aryProcIDs, aryProcGroupIDs;
				for (int i = 0; i < m_pAddProDlg->m_arProcIDs.GetSize(); i++ ){
					aryProcIDs.Add(m_pAddProDlg->m_arProcIDs.GetAt(i));
				}

				for (i = 0; i < m_pAddProDlg->m_arProcIDs.GetSize(); i++ ){
					aryProcGroupIDs.Add(m_pAddProDlg->m_arProcGroupIDs.GetAt(i));
				}*/

				// (j.gruber 2009-06-01 11:19) - PLID 34424 - fixed error if you don't have tracking
				CInquiryDlg dlg(strFirst, strMiddle, strLast, strEmail, nReferralID, 
					m_pAddProDlg != NULL ? &m_pAddProDlg->m_arProcIDs : NULL, 
					m_pAddProDlg != NULL ? &m_pAddProDlg->m_arProcGroupIDs : NULL, 
					m_pAddProDlg != NULL ? m_pAddProDlg->m_bProcedures: TRUE, strNote, this);
				nResult = dlg.DoModal();
			}
			else {
				//they said no, just do the regular
				CInquiryDlg dlg(this);
				nResult = dlg.DoModal();
			}

		}
		
		if (nResult== IDOK) {
			CDialog::OnOK();
		}
	}NxCatchAll("Error in OnClickInquiry()");

}

void CNewPatient::OnSelChosenRefPhysList(long nRow)
{
	try {
		if(nRow != -1 && VarBool(m_pRefPhysList->GetValue(nRow, rpcShowProcWarning),FALSE)) {
			//OK, we need to list the procedures performed by this patient.
			CString strMessage = "This referring physician performs the following procedures:\r\n";
			_RecordsetPtr rsProcs = CreateRecordset("SELECT Name FROM ProcedureT INNER JOIN RefPhysProcLinkT ON ProcedureT.ID = RefPhysProcLinkT.ProcedureID "
				"WHERE RefPhysProcLinkT.RefPhysID = %li", VarLong(m_pRefPhysList->GetValue(nRow, rpcID)));
			while(!rsProcs->eof) {
				strMessage += AdoFldString(rsProcs, "Name") + "; ";
				rsProcs->MoveNext();
			}
			rsProcs->Close();
			if(strMessage.Right(2) == "; ") strMessage = strMessage.Left(strMessage.GetLength()-2);
			CString strExtraText = VarString(m_pRefPhysList->GetValue(nRow, rpcProcWarning), "");
			if(!strExtraText.IsEmpty()) {
				strMessage += "\r\n" + strExtraText;
			}
			MsgBox(strMessage);
		}

		//PLID 20163: auto-add the preferenced referral source if you choose a referring physician
		long nDefReferral = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);
		// (b.eyers 8/18/2014) - PLID 63050 - don't add the default referral source if 'no provider selected' was choosen for referring physician
		CString strRefPhys = VarString(m_pRefPhysList->GetValue(nRow, rpcName)); 
		if (nDefReferral != -1 && strRefPhys != "<No Referring Physician>") {
			long nReferralID = m_pdlgReferralSubDlg->GetSelectedReferralID();
			// (r.goldschmidt 2014-09-22 17:07) - PLID 31191 - Check for possible conflict with parent referral preference
			//   If selection not allowed, throw a message box, no referral source gets added, DefaultPatientReferral preference remains intact.
			CString strWarning;
			long nIsRestricted = m_pdlgReferralSubDlg->ReferralRestrictedByPreference(nDefReferral, strWarning);
			if (nIsRestricted == 0) { // selection is allowed
				if (nReferralID <= 0) {
					//set the selection to our default
					m_pdlgReferralSubDlg->SelectReferralID(nDefReferral);
				}
				// (c.haag 2006-08-11 16:04) - PLID 21776 - Add it to the multi-referral list if necessary
				if (NULL == m_pMultiReferralList->FindByColumn(0, nDefReferral, NULL, false)) {
					AddMultiReferral(nDefReferral, TRUE);
				}
			}
			else { // selection not allowed
				strWarning += " The default referring physician referral source conflicts with these preferences.\n\nPlease manually add the desired referral source and review preference settings.";
				AfxMessageBox(strWarning);
			}
		}

	}NxCatchAll("Error in CNewPatient::OnSelChosenRefPhysList");
}

// (e.lally 2005-5-23)
//Code modeled after Patients Module, General2 tab
// (v.maida - 2014-08-18 16:30) - PLID 30758 - If the referring physician ellipse button has been clicked, pass a referring phys enum to the function that handles new contact choices.
void CNewPatient::OnNewRefPhys()
{
	try {
		HandleContactChoice(EContactChoices::refPhysChoice);
	}NxCatchAll(__FUNCTION__);
}

// (v.maida 2014-08-18 16:14) - PLID 30758 - If the provider ellipse button has been clicked, pass a provider enum to the function that handles new contact choices.
void CNewPatient::OnNewProvider()
{
	try {
		HandleContactChoice(EContactChoices::providerChoice);
	}NxCatchAll(__FUNCTION__);
}

void CNewPatient::OnRequeryFinishedRefPhysList(short nFlags) 
{
	NXDATALISTLib::IRowSettingsPtr pRow = m_pRefPhysList->GetRow(NXDATALISTLib::sriGetNewRow);
	// (j.jones 2011-12-01 14:11) - PLID 46771 - this now uses the enums
	pRow->PutValue(rpcID, g_cvarNull);
	pRow->PutValue(rpcName, "<No Referring Physician>");
	pRow->PutValue(rpcNpi, "");
	pRow->PutValue(rpcRefPhysID, "");
	pRow->PutValue(rpcUpin, "");
	pRow->PutValue(rpcShowProcWarning, g_cvarFalse);
	pRow->PutValue(rpcProcWarning, "");
	pRow->PutValue(rpcAddress1, "");
	pRow->PutValue(rpcAddress2, "");
	pRow->PutValue(rpcCity, "");
	pRow->PutValue(rpcState, "");
	pRow->PutValue(rpcZip, "");
	m_pRefPhysList->InsertRow(pRow,0);
	m_pRefPhysList->PutCurSel(0);	

	//DRT 6/28/2005 - PLID 16745 - Add a hidden ConfigRT property to disable this list (save memory)
	if(GetRemotePropertyInt("DisableNewPatRefPat", 0, 0, "<None>", true) == 0) {
		m_pRefPatientList = BindNxDataListCtrl(IDC_REF_PATIENT_LIST);
	}
	else {
		GetDlgItem(IDC_REF_PATIENT_LIST)->EnableWindow(FALSE);
		m_pRefPatientList = NULL;
	}
}

void CNewPatient::OnRequeryFinishedPatientTypeCombo(short nFlags) 
{
	NXDATALISTLib::IRowSettingsPtr pRow = m_PatientTypeCombo->GetRow(NXDATALISTLib::sriGetNewRow);
	_variant_t var;
	var.vt = VT_NULL;
	pRow->PutValue(0,var);
	pRow->PutValue(1,"<No Patient Type>");
	m_PatientTypeCombo->InsertRow(pRow,0);
	m_PatientTypeCombo->PutCurSel(0);

	int pattype = GetRemotePropertyInt("DefaultPatType",-1,0,"<None>",TRUE);
	if(pattype != -1)
		m_PatientTypeCombo->SetSelByColumn(0,(long)pattype);	
}

void CNewPatient::OnRequeryFinishedDoclist(short nFlags) 
{
	NXDATALISTLib::IRowSettingsPtr pRow = m_pDocList->GetRow(NXDATALISTLib::sriGetNewRow);
	_variant_t var;
	var.vt = VT_NULL;
	pRow->PutValue(0,var);
	// (j.jones 2010-04-30 09:20) - PLID 38151 - renamed to Provider
	pRow->PutValue(1,"<No Provider>");
	m_pDocList->InsertRow(pRow,0);
	long nDefaultProvider = GetDefaultProviderID();
	if(nDefaultProvider > 0)
		m_pDocList->SetSelByColumn(0,(_variant_t) nDefaultProvider);
	else
		m_pDocList->PutCurSel(0);
}

void CNewPatient::OnRequeryFinishedCoordlist(short nFlags) 
{
	NXDATALISTLib::IRowSettingsPtr pRow = m_pCoordList->GetRow(NXDATALISTLib::sriGetNewRow);
	_variant_t var;
	var.vt = VT_NULL;
	pRow->PutValue(0,var);
	pRow->PutValue(1,"<No Patient Coordinator>");
	m_pCoordList->InsertRow(pRow,0);
	long nDefaultCoord = GetDefaultCoordinatorID();
	if(nDefaultCoord > 0)
		m_pCoordList->SetSelByColumn(0, nDefaultCoord);
	else
		m_pCoordList->PutCurSel(0);

	// (r.goldschmidt 2014-08-12 17:31) - PLID 20992 - check preference to override the patient coordinator into current user
	if (GetRemotePropertyInt("DefaultNewPatientsPatCoord", 1, 0, GetCurrentUserName(), true) == 1){
		_RecordsetPtr rs = CreateRecordset("SELECT PatientCoordinator FROM UsersT WHERE PersonID = %li", GetCurrentUserID());
		if (!rs->eof){
			var = rs->Fields->Item["PatientCoordinator"]->Value;
			if (var.vt == VT_BOOL && var.boolVal) {
				m_pCoordList->SetSelByColumn(0, GetCurrentUserID());
			}
		}
		rs->Close();
	}
}

void CNewPatient::OnRequeryFinishedPtPrefixList(short nFlags) 
{
	NXDATALISTLib::IRowSettingsPtr pRow = m_PrefixCombo->GetRow(NXDATALISTLib::sriGetNewRow);
	pRow->PutValue(0, long(-1));
	pRow->PutValue(1, _bstr_t(""));
	pRow->PutValue(2, long(0));
	m_PrefixCombo->AddRow(pRow);

	//DRT 5/23/03 - Can no choose the default gender and default prefix in the preferences dialog
	{
		// (j.jones 2016-02-25 15:52) - PLID 67549 - added enums for the Gender combo
		// (r.farnworth 2016-03-03 12:55) - PLID 68454 - Check if we're already pre-selecting here.
		if (m_strPreselectGender != "") {
			m_GenderCombo->SetSelByColumn(gccGenderName, _variant_t(m_strPreselectGender));
		}
		else {
			m_GenderCombo->SetSelByColumn(gccID, GetRemotePropertyInt("DefaultGender", 2, 0, "<None>", true));
		}

		long nPrefix = GetRemotePropertyInt("DefaultPatientPrefix", -1, 0, "<None>", true);
		m_PrefixCombo->SetSelByColumn(0, nPrefix);
	}	
}

void CNewPatient::OnRequeryFinishedRefPatientList(short nFlags) 
{
	NXDATALISTLib::IRowSettingsPtr pRow = m_pRefPatientList->GetRow(-1);
	_variant_t var;
	var.vt = VT_NULL;
	pRow->PutValue(0,var);
	pRow->PutValue(1,"<No Referring Patient>");
	m_pRefPatientList->InsertRow(pRow,0);
	m_pRefPatientList->PutCurSel(0);
}

void CNewPatient::OnCheckMultipleReferralSources() 
{
	if (m_btnMultipleReferrals.GetCheck()) {
		GetDlgItem(IDC_NEWPT_ADD_REFERRAL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NEWPT_REFERRAL_LIST)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NEWPT_REMOVE_REFERRAL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NEWPT_MAKE_PRIMARY_REFERRAL)->ShowWindow(SW_SHOW);
		m_pdlgReferralSubDlg->ShowWindow(SW_HIDE);
	} else {
		m_pdlgReferralSubDlg->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NEWPT_ADD_REFERRAL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NEWPT_REFERRAL_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NEWPT_REMOVE_REFERRAL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_NEWPT_MAKE_PRIMARY_REFERRAL)->ShowWindow(SW_HIDE);
	}
}

void CNewPatient::AddMultiReferral(long nReferralID, BOOL bSilent)
{
	_RecordsetPtr prs = CreateRecordset("SELECT Name FROM ReferralSourceT WHERE PersonID = %d", nReferralID);
	if (prs->eof) {
		if (!bSilent) {
			MsgBox("This referral has been removed from your system. Please review your referral list and try again.");
		}
	} else if (NULL != m_pMultiReferralList->FindByColumn(0, nReferralID, 0, FALSE)) {
		if (!bSilent) {
			MsgBox("This referral already exists in your list.");
		}
	} else {
		//this preference lets them leave existing referrals as primary
		//1 - make new referral primary (default), 2 - leave existing as primary
		long nPrimaryReferralPref = GetRemotePropertyInt("PrimaryReferralPref",1,0,"<None>",TRUE);
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMultiReferralList->GetNewRow();
		BOOL bMakePrimary = (nPrimaryReferralPref == 1 || m_pMultiReferralList->GetRowCount() == 0) ? TRUE : FALSE;
		pRow->Value[0] = nReferralID;
		pRow->Value[1] = prs->Fields->Item["Name"]->Value;
		if (bMakePrimary) {
			if (m_nPrimaryMultiReferralID != -1) {
				NXDATALIST2Lib::IRowSettingsPtr pOldPrimary = m_pMultiReferralList->FindByColumn(0, m_nPrimaryMultiReferralID, NULL, false);
				if (NULL != pOldPrimary) {
					pOldPrimary->ForeColor = RGB(0,0,0);
					pOldPrimary->ForeColorSel = RGB(0,0,0);
				}
			}
			pRow->ForeColor = RGB(255,0,0);
			pRow->ForeColorSel = RGB(255,0,0);
			m_nPrimaryMultiReferralID = nReferralID;
			// Make sure the multi-select list matches the primary
			m_pdlgReferralSubDlg->SelectReferralID(nReferralID);
		} else {
			pRow->ForeColor = RGB(0,0,0);
			pRow->ForeColorSel = RGB(0,0,0);
		}
		m_pMultiReferralList->AddRowAtEnd(pRow, NULL);
		m_pMultiReferralList->Sort();
	}
}

void CNewPatient::OnNewptAddReferral() 
{
	try {
		// (c.haag 2006-08-04 14:59) - PLID 21776 - Add a referral to the multi-referral list
		//
		CReferralTreeDlg dlg(this);
		long nReferralID = dlg.DoModal();	//returns the id selected
		if (nReferralID > 0) {
			AddMultiReferral(nReferralID, FALSE);
		}
	} NxCatchAll("Error adding new referral");
}

void CNewPatient::OnNewptMakePrimaryReferral() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMultiReferralList->GetCurSel();
		if (NULL != pRow) {
			NXDATALIST2Lib::IRowSettingsPtr p = m_pMultiReferralList->GetFirstRow();
			while (NULL != p) {
				if (p == pRow) {
					p->ForeColor = RGB(255,0,0);
					p->ForeColorSel = RGB(255,0,0);
				} else {
					p->ForeColor = RGB(0,0,0);
					p->ForeColorSel = RGB(0,0,0);
				}
				p = p->GetNextRow();
			}
			m_nPrimaryMultiReferralID = VarLong(pRow->Value[0]);
		}
	}
	NxCatchAll("Error assigning primary referral");
}

void CNewPatient::OnNewptRemoveReferral() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMultiReferralList->GetCurSel();
		if (NULL != pRow) {
			if (IDNO == MsgBox(MB_YESNO|MB_ICONQUESTION,"Are you sure you wish to remove this referral?")) {
				return;
			}
			if (m_nPrimaryMultiReferralID == VarLong(pRow->Value[0])) {
				m_nPrimaryMultiReferralID = -1;
			}
			m_pMultiReferralList->RemoveRow(pRow);
		}
	}
	NxCatchAll("Error removing referral");
}

void CNewPatient::OnSelChangedNewptReferralList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	BOOL bEnable = (NULL == lpNewSel) ? FALSE : TRUE;
	GetDlgItem(IDC_NEWPT_REMOVE_REFERRAL)->EnableWindow(bEnable);
	GetDlgItem(IDC_NEWPT_MAKE_PRIMARY_REFERRAL)->EnableWindow(bEnable);
}

void CNewPatient::OnSelChangingRelatedPat(long FAR* nNewSel) 
{
	// (a.walling 2006-11-24 16:21) - PLID 22715 - Prevent invalid selections
	if (*nNewSel == NXDATALISTLib::sriNoRow) {
		*nNewSel = m_pRelatedPatientList->CurSel;	
	}
}

LRESULT CNewPatient::OnMSRDataEvent(WPARAM wParam, LPARAM lParam)
{
	// (a.wetta 2007-03-19 14:25) - PLID 24983 - Process the driver's license information

	CWaitCursor wait;

	// (a.wetta 2007-07-05 08:56) - PLID 26547 - Get the track information from the card swipe and then parse the tracks
	MSRTrackInfo *mtiInfo = (MSRTrackInfo*)wParam;
	DriversLicenseInfo dliLicenseInfo = COPOSMSRDevice::ParseDriversLicenseInfoFromMSRTracks(mtiInfo->strTrack1, mtiInfo->strTrack2, mtiInfo->strTrack3);

	// (j.jones 2009-06-19 10:56) - PLID 33650 - changed boolean to be an enum
	if(mtiInfo->msrCardType == msrDriversLicense) {
		// (b.cardillo 2007-09-17 11:43) - PLID 24983 - Fixed up the order things happen here.  We used to prompt the user 
		// first, telling her that she had swiped a driver's license, even before we knew it wasn't a credit card.  Now we 
		// check first, then if it's a not a credit card prompt her, or if it is just tell her that credit cards can't be 
		// swiped for demographic info.

		// Warn the user that the information from the license will overwrite the information already in the fields
		CString strMsg;
		strMsg = "A driver's license has just been swiped.  The information from the license will fill the demographic fields for this "
				"new patient and any information already in those fields will be overwritten. \r\n\r\n"
				"Are you sure you want to import the information from the driver's license?";
		int nResult = DontShowMeAgain(this, strMsg, "MSR_GetInfoForNewPatient", "Get New Patient Demographics from Driver's License", FALSE, TRUE);
		if (nResult == IDYES || nResult == IDOK) {
			// Fill the information from the driver's license to the screen
			SetDlgItemText(IDC_FIRST_NAME_BOX, FixCapitalization(dliLicenseInfo.m_strFirstName));
			SetDlgItemText(IDC_MIDDLE_NAME_BOX, FixCapitalization(dliLicenseInfo.m_strSuffix));
			SetDlgItemText(IDC_LAST_NAME_BOX, FixCapitalization(dliLicenseInfo.m_strLastName));
			SetDlgItemText(IDC_ADDRESS1_BOX, FixCapitalization(dliLicenseInfo.m_strAddress1));
			SetDlgItemText(IDC_ADDRESS2_BOX, FixCapitalization(dliLicenseInfo.m_strAddress2));
			SetDlgItemText(IDC_CITY_BOX, FixCapitalization(dliLicenseInfo.m_strCity));
			SetDlgItemText(IDC_STATE_BOX, dliLicenseInfo.m_strState);
			SetDlgItemText(IDC_ZIP_BOX, dliLicenseInfo.m_strPostalCode);

			if (dliLicenseInfo.m_dtBirthdate != 0)
				m_nxtBirthDate->SetDateTime(dliLicenseInfo.m_dtBirthdate);
			else 
				m_nxtBirthDate->Clear();

			// (j.jones 2016-02-25 15:52) - PLID 67549 - added enums for the Gender combo
			if (dliLicenseInfo.m_strSex == "male") {
				// Make sure to select the correct prefix
				OnSelChosenGender(m_GenderCombo->SetSelByColumn(gccID, (long)1));
			}
			else if (dliLicenseInfo.m_strSex == "female") {
				// Make sure to select the correct prefix
				OnSelChosenGender(m_GenderCombo->SetSelByColumn(gccID, (long)2));
			}
			else {
				// Make sure to select the correct prefix
				OnSelChosenGender(m_GenderCombo->SetSelByColumn(gccID, (long)0));
			}
		}
	}
	// (j.jones 2010-05-18 12:43) - PLID 38483 - supported swiping an OHIP card
	else if(UseOHIP() && mtiInfo->msrCardType == msrOHIPHealthCard) {

		OHIPHealthCardInfo ohciHealthCardInfo = COPOSMSRDevice::ParseOHIPHealthCardInfoFromMSRTracks(mtiInfo->strTrack1, mtiInfo->strTrack2, mtiInfo->strTrack3);

		// Warn the user that the information from the license will overwrite the information already in the fields
		CString strMsg;
		strMsg = "A health card has just been swiped.  The information from the health card will fill the demographic fields for this "
				"new patient and any information already in those fields will be overwritten. \r\n\r\n"
				"Are you sure you want to import the information from the health card?";
		int nResult = DontShowMeAgain(this, strMsg, "MSR_GetInfoForNewPatient_OHIP", "Get New Patient Demographics from Health Card", FALSE, TRUE);
		if (nResult == IDYES || nResult == IDOK) {

			SetDlgItemText(IDC_FIRST_NAME_BOX, FixCapitalization(ohciHealthCardInfo.m_strFirstName));
			SetDlgItemText(IDC_MIDDLE_NAME_BOX, FixCapitalization(ohciHealthCardInfo.m_strMiddleName));
			SetDlgItemText(IDC_LAST_NAME_BOX, FixCapitalization(ohciHealthCardInfo.m_strLastName));

			if (ohciHealthCardInfo.m_dtBirthDate != 0) {
				m_nxtBirthDate->SetDateTime(ohciHealthCardInfo.m_dtBirthDate);	
			}
			else {
				m_nxtBirthDate->Clear();
			}


			// (j.jones 2016-02-25 15:52) - PLID 67549 - added enums for the Gender combo
			if (ohciHealthCardInfo.m_strSex == "1") {
				// Make sure to select the correct prefix
				OnSelChosenGender(m_GenderCombo->SetSelByColumn(gccID, (long)1));
			}
			else if (ohciHealthCardInfo.m_strSex == "2") {
				// Make sure to select the correct prefix
				OnSelChosenGender(m_GenderCombo->SetSelByColumn(gccID, (long)2));
			}
			else {
				// Make sure to select the correct prefix
				OnSelChosenGender(m_GenderCombo->SetSelByColumn(gccID, (long)0));
			}

			//when OHIP is in use, these fields are the health card & version code
			GetDlgItemText(IDC_NEWPAT_INS_ID_BOX, ohciHealthCardInfo.m_strHealthCardNum);
			GetDlgItemText(IDC_NEWPAT_INS_FECA_BOX, ohciHealthCardInfo.m_strVersionCode);
		}
	} else {
		// (j.jones 2009-06-19 10:58) - PLID 33650 - made this message more generic as we support more than two card types
		if(UseOHIP()) {
			// (j.jones 2010-05-18 12:41) - PLID 38483 - give a message mentioning OHIP support
			MessageBox(
				"A card has just been swiped, but patient demographic information can only be retrieved from a driver's license or a health card.\r\n\r\n"
				"Please swipe a driver's license or health card to import patient information.", 
				"Magnetic Strip Reader", MB_OK|MB_ICONEXCLAMATION);
		}
		else {
			//normal message
			MessageBox(
				"A card has just been swiped, but patient demographic information can only be retrieved from a driver's license.\r\n\r\n"
				"Please swipe a driver's license to import patient information.", 
				"Magnetic Strip Reader", MB_OK|MB_ICONEXCLAMATION);
		}
	}

	return 0;
}


// (d.moore 2007-08-15) - PLID 25455 - Check to see if there is an inquiry entry that
//  the user would like to use when creating a new patient. The function returns the
//  ID value of the selected inquiry person. Returns -1 if an inquiry was not selected,
//  or if there were no matches in the inquiry list to choose from.
long CNewPatient::CheckForInquiryConversion(const CString &strFirstName, const CString &strLastName, const CString &strEmail)
{
	if (strFirstName.GetLength() == 0 
		&& strLastName.GetLength() == 0
		&& strEmail.GetLength() == 0) {
		// There is nothing to search on, so just return.
		return -1;
	}
	
	// Check to see if there are any inquiries that match the name or email.
	CString strQuery = 
		"SELECT COUNT(*) AS MatchCount "
		"FROM PatientsT "
			"INNER JOIN PersonT "
			"ON PatientsT.PersonID = PersonT.ID "
		"WHERE PatientsT.CurrentStatus = 4 ";
	
	CString strWhere, strNameQuery, strTemp;
	bool bValidName = false;
	
	if (strFirstName.GetLength() > 0) {
		bValidName = true;
		// (j.gruber 2007-11-30 09:30) - PLID 28174 - fixed adding a new patient with a ' in its name
		strNameQuery.Format("PersonT.[First] = '%s'", _Q(strFirstName));
	}

	if (strLastName.GetLength() > 0) {
		// (j.gruber 2007-11-30 09:31) - PLID 28174 - fixed adding a new patient with a ' in its name
		strTemp.Format("PersonT.[Last] = '%s'", _Q(strLastName));
		if (bValidName) {
			strNameQuery = "(" + strNameQuery + " AND " + strTemp + ")";
		} else {
			strNameQuery = strTemp;
		}
		bValidName = true;
	}

	strWhere = strNameQuery;

	if (strEmail.GetLength()) {
		if (bValidName) {
			strWhere += " OR ";
		}
		// (j.gruber 2007-11-30 09:32) - PLID 28174 - fixed adding a new patient with a ' in its name
		strTemp.Format("PersonT.Email = '%s'", _Q(strEmail));
		strWhere += strTemp;
	}

	if (strWhere.GetLength()) {
		strWhere = " AND (" + strWhere + ")";
	}
	
	long nCount = 0;
	strQuery += strWhere;
	// (j.jones 2010-04-20 09:28) - PLID 38273 - converted to CreateRecordsetStd
	_RecordsetPtr rs = CreateRecordsetStd(strQuery);
	if (!rs->eof) {
		nCount = AdoFldLong(rs, "MatchCount", 0);
	}

	// No matches, so just return.
	if (nCount <= 0) {
		return -1;
	}
	
	// Open up a dialog to allow the user to select an inquiry to convert.
	CNewPatientMatchInquiry dlg(this);
	dlg.m_strFirstName = strFirstName;
	dlg.m_strLastName = strLastName;
	dlg.m_strEmail = strEmail;

	long nInquiryID = -1;
	if (dlg.DoModal() == IDOK) {
		nInquiryID = dlg.m_nPersonID;
	}

	return nInquiryID;
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CNewPatient::OnOK()
{
	//Eat the message
}

// (z.manning 2009-07-08 15:45) - PLID 27251 - Handle the cursor when hovering over any hyperlinks
BOOL CNewPatient::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
	try
	{
		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		CRect rcEmailReq;
		m_nxlabelEmailReq.GetWindowRect(rcEmailReq);
		ScreenToClient(&rcEmailReq);

		if(rcEmailReq.PtInRect(pt) && m_nxlabelEmailReq.GetType() == dtsHyperlink) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

	}NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (z.manning 2009-07-08 15:54) - PLID 27251 - Handle clicking on labels
LRESULT CNewPatient::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		if(m_nxlabelEmailReq.GetType() == dtsHyperlink) {
			PromptDeclineEmail();
		}

	} NxCatchAll(__FUNCTION__);

	return 0;
}

void CNewPatient::PromptDeclineEmail()
{
	if(m_bEmailDeclined) {
		// (z.manning 2009-07-08 16:11) - PLID 27251 - Email was already declined, so un-decline it.
		m_bEmailDeclined = FALSE;
		m_nxeditEmailBox.SetReadOnly(FALSE);
		m_nxeditEmailBox.SetWindowText("");
	}
	else {
		// (z.manning 2009-07-08 16:11) - PLID 27251 - Prompt them to mark email as "declined" so that
		// they can still have it as a requied field but be able to not fill it out for patients who
		// don't have e-mail or refuse to share it.
		int nResult = MessageBox("E-mail is a required field for new patients. However, if the patient does not have an "
			"e-mail address or is unwilling to give it then you may mark this field as declined.\r\n\r\n"
			"Would you like to mark e-mail as declined for this patient?", "Decline Email", MB_YESNO|MB_ICONQUESTION);
		if(nResult == IDYES) {
			m_bEmailDeclined = TRUE;
			m_nxeditEmailBox.SetReadOnly(TRUE);
			m_nxeditEmailBox.SetWindowText("< Declined >");
		}
	}
}


// (j.jones 2010-05-06 14:49) - PLID 38482 - added custom fields, used by OHIP
void CNewPatient::SaveCustomInfo(long nPatientID, long nFieldID, CString strValue)
{
	try {

		//no need to audit, this is a new patient

		strValue.TrimLeft();
		strValue.TrimRight();

		_RecordsetPtr rsCustom = CreateParamRecordset("SELECT TextParam FROM CustomFieldDataT WHERE PersonID = {INT} AND FieldID = {INT}", nPatientID, nFieldID);
		if(!rsCustom->eof) {
			ExecuteParamSql("UPDATE CustomFieldDataT SET TextParam = {STRING} WHERE PersonID = {INT} AND FieldID = {INT}", strValue, nPatientID, nFieldID);
		}
		else {
			//check CustomFieldsT
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM CustomFieldsT WHERE ID = {INT}", nFieldID);
			if(rs->eof) {
				ExecuteParamSql("INSERT INTO CustomFieldsT (ID, Name, Type) VALUES ({INT}, {STRING}, 1)",nFieldID, FormatString("Custom %li", nFieldID));
			}
			rs->Close();

			ExecuteParamSql("INSERT INTO CustomFieldDataT (PersonID,FieldID,TextParam) VALUES ({INT},{INT},{STRING})",nPatientID,nFieldID,strValue);
		}
		rsCustom->Close();
	}
	NxCatchAll("Error in saving custom data. CNewPatient::SaveCustomInfo");
}

// (c.haag 2010-10-04 10:21) - PLID 39447 - Call this function to fill the demographic
// section of a CNewPatientInsuredParty object with the patient demographics in this
// window.
void CNewPatient::FillPatientInfo(CNewPatientInsuredParty& p)
{
	GetDlgItemText(IDC_FIRST_NAME_BOX, p.m_strInsFirst);
	GetDlgItemText(IDC_MIDDLE_NAME_BOX, p.m_strInsMiddle);
	GetDlgItemText(IDC_LAST_NAME_BOX, p.m_strInsLast);
	// (j.jones 2012-10-25 09:42) - PLID 36305 - insured parties also support Title, but this dialog has no Title field
	GetDlgItemText(IDC_ADDRESS1_BOX, p.m_strInsAddress1);
	GetDlgItemText(IDC_ADDRESS2_BOX, p.m_strInsAddress2);
	GetDlgItemText(IDC_CITY_BOX, p.m_strInsCity);
	GetDlgItemText(IDC_STATE_BOX, p.m_strInsState);
	GetDlgItemText(IDC_ZIP_BOX, p.m_strInsZip);
	
	// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country for the new patient insurance struct
	NXDATALIST2Lib::IRowSettingsPtr pCountryRow = m_pCountryList->GetCurSel();
	p.m_strInsCountry = "";
	if(pCountryRow) {
		p.m_strInsCountry = pCountryRow->GetValue(cccName);
	}

	GetDlgItemText(IDC_HOME_PHONE_BOX, p.m_strInsPhone);
	GetDlgItemText(IDC_SSN_BOX, p.m_strInsSSN);

	// (j.jones 2016-02-25 15:56) - PLID 67549 - get the value, assume 0 if unselected
	long nGender = 0;
	if (m_GenderCombo->GetCurSel() != -1) {
		nGender = VarLong(m_GenderCombo->GetValue(m_GenderCombo->GetCurSel(), gccID), 0);
	}

	p.m_nInsGender = nGender;
	if(m_nxtBirthDate->GetStatus() == 1){
		p.m_dtInsBirthDate = m_nxtBirthDate->GetDateTime();
	}
}

// (c.haag 2010-10-04 10:21) - PLID 39447 - Add a new insured party
void CNewPatient::OnBnClickedListInsAdd()
{
	try {
		CNewPatientInsuredParty party;
		CNewPatientInsuredParty patient;
		FillPatientInfo(patient);

		// By default, we want to assign the party a Primary placement. If one already
		// exists, we do secondary. If one already exists, then leave unselected. It would
		// seem primary and secondary are hard-entered into the database, and I have not
		// found a way to change their names. So, I'll just hard code them; no need to take
		// a trip to the SQL Server.
		BOOL bHasPrimary = FALSE;
		BOOL bHasSecondary = FALSE;
		for (int i=0; i < m_aInsuredParties.GetSize(); i++) {
			switch (m_aInsuredParties[i].GetRespTypeID())
			{
			case 1: bHasPrimary = TRUE; break;
			case 2: bHasSecondary = TRUE; break;
			}
		}
		if (!bHasPrimary) {
			party.SetRespType(1, "Primary");
		}
		else if (!bHasSecondary) {
			party.SetRespType(2, "Secondary");
		}
		// Like the insurance tab, default the new party to the patient's demographics
		FillPatientInfo(party);

		// Now invoke the window
		CNewPatientAddInsuredDlg dlg(party, patient, this);
		if (IDOK == dlg.DoModal())
		{
			m_aInsuredParties.Add(party);
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlInsuredParties->GetNewRow();
			pRow->PutValue(ilcName, _bstr_t(party.GetInsuranceCompanyName()));
			pRow->PutValue(ilcPlacement, _bstr_t(party.GetRespTypeName()));
			m_dlInsuredParties->CurSel = m_dlInsuredParties->AddRowAtEnd(pRow, NULL);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-10-13 10:26) - PLID 39447 - Edit an insured party
void CNewPatient::DoInsuredPartyEdit(NXDATALIST2Lib::IRowSettingsPtr& pRow)
{
	long nIndex = pRow->CalcRowNumber();
	CNewPatientInsuredParty patient;
	FillPatientInfo(patient);
	// If this party is self-responsible, ensure the demographics are consistent with the working patient name.
	if ("Self" == m_aInsuredParties[nIndex].m_strRelationToPt) {
		FillPatientInfo(m_aInsuredParties[nIndex]);
	}
	CNewPatientAddInsuredDlg dlg(m_aInsuredParties[nIndex], patient, this);
	if (IDOK == dlg.DoModal())
	{
		pRow->PutValue(ilcName, _bstr_t(m_aInsuredParties[nIndex].GetInsuranceCompanyName()));
		pRow->PutValue(ilcPlacement, _bstr_t(m_aInsuredParties[nIndex].GetRespTypeName()));
	}
}

// (c.haag 2010-10-04 10:21) - PLID 39447 - Edit an insured party
void CNewPatient::OnBnClickedListInsEdit()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlInsuredParties->CurSel;
		if (0 == m_dlInsuredParties->GetRowCount()) {
			AfxMessageBox("There are no insured parties to edit.\n\nTo create one, please press the Add button in the Insurance Information section.", MB_OK | MB_ICONSTOP);
		}
		else if (NULL == pRow) {
			AfxMessageBox("Please select an insured party to edit.", MB_OK | MB_ICONSTOP);
		}
		else {
			DoInsuredPartyEdit(pRow);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-10-04 10:21) - PLID 39447 - Delete an insured party
void CNewPatient::OnBnClickedListInsDelete()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlInsuredParties->CurSel;
		if (0 == m_dlInsuredParties->GetRowCount()) 
		{
			AfxMessageBox("There are no insured parties to delete.", MB_OK | MB_ICONSTOP);
		}
		else if (NULL == pRow) {
			AfxMessageBox("Please select an insured party to delete.", MB_OK | MB_ICONSTOP);
		}
		else {
			if (IDYES == AfxMessageBox("Are you sure you wish to delete the selected party?", MB_YESNO | MB_ICONQUESTION)) 
			{
				long nIndex = pRow->CalcRowNumber();
				m_aInsuredParties.RemoveAt(nIndex);
				m_dlInsuredParties->RemoveRow(pRow);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-10-12 10:27) - PLID 39447 - Edit an insured party by double-clicking on it
void CNewPatient::OnDblClickCellInsList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow) {
			DoInsuredPartyEdit(pRow);
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-06-24 16:57) - PLID 31005 - supported OnEnKillfocusNewpatInsIdBox
void CNewPatient::OnEnKillfocusNewpatInsIdBox()
{
	try {

		// (j.jones 2011-06-24 16:49) - PLID 31005 - if they want the ID capitalized, do so now
		if(GetRemotePropertyInt("AutoCapitalizeInsuranceIDs", 0, 0, "<None>", true) == 1) {
			CString str;
			GetDlgItemText(IDC_NEWPAT_INS_ID_BOX, str);
			str.MakeUpper();
			SetDlgItemText(IDC_NEWPAT_INS_ID_BOX, str);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-06-24 16:57) - PLID 31005 - supported OnEnKillfocusNewpatInsFecaBox
void CNewPatient::OnEnKillfocusNewpatInsFecaBox()
{
	try {

		// (j.jones 2011-06-24 16:49) - PLID 31005 - if they want the ID capitalized, do so now
		if(GetRemotePropertyInt("AutoCapitalizeInsuranceIDs", 0, 0, "<None>", true) == 1) {
			CString str;
			GetDlgItemText(IDC_NEWPAT_INS_FECA_BOX, str);
			str.MakeUpper();
			SetDlgItemText(IDC_NEWPAT_INS_FECA_BOX, str);
		}

	}NxCatchAll(__FUNCTION__);
}

/*r.wilson  PLID 39357 10/26/2011*/
void CNewPatient::OnKillFocusID()
{
	//These two variables are for keeping track in any errors occurred, and which ones occurred
	BOOL bErrorsFound = FALSE;
	CString strErrors;

	//nNewID holds the id that the user is TRYING to change the Patient to
	long nNewID = -1;

	try
	{
		BOOL bUserCanWrite = CheckCurrentUserPermissions(bioPatientID, SPT___W_______, FALSE, 0, TRUE);
		BOOL bUserCanWriteOnlyWithPass = CheckCurrentUserPermissions(bioPatientID, SPT___W________ONLYWITHPASS, FALSE, 0, TRUE);

		nNewID = GetDlgItemInt(IDC_ID_BOX);
		
		//If the id didn't change then do nothing
		if(nNewID == m_nOriginalUserDefinedID)
		{
			//no change was made
			return;
		}
		
		/* r.wilson  PLID 39357 10/26/2011 - If a user has write priv. then let the id be changed provided that
											  it is in the correct format*/
		if(bUserCanWrite)
		{
			bErrorsFound = IsIdInvalid(nNewID,strErrors);
			if(!bErrorsFound)
			{
				//The user has the appropraite permissions and no errors were found so the data goes untouched
				m_nOriginalUserDefinedID = nNewID;
				return;
			}
			/*
			If a error is found then the last IF in this function will reset the ID 
			in the text box to its original value
			*/
		}

		//r.wilson  PLID 39357 10/26/2011
		else if (bUserCanWriteOnlyWithPass)
		{
			if (!CheckCurrentUserPassword())
			{
				//If we didn't have a passsword of if it failed
				CString strPatientID;
				strPatientID.Format("%d",m_nOriginalUserDefinedID);
				m_nxeditIdBox.SetWindowTextA(strPatientID);
			}
			else 
			{
				//The password was correct
				strErrors = "";
				bErrorsFound = IsIdInvalid(nNewID,strErrors);

				if(!bErrorsFound){
					m_nOriginalUserDefinedID = nNewID;
				}
			}
			
		}
	 /* 
		r.wilson  PLID 39357 10/26/2011
		(IF WE FOUND ERRORS...)
		-Show a message box that tells of any errors that pertain to the ID
		-This also sets the text box back to the original value */
		if(bErrorsFound)
		{
			CString strPatientID = "";
			strPatientID.Format("%d",m_nOriginalUserDefinedID);
			m_nxeditIdBox.SetWindowTextA(strPatientID);	
			AfxMessageBox(strErrors);
		}
	}NxCatchAll(__FUNCTION__);
	
}


/*
r.wilson  PLID 39357 10/26/2011
This function only does "client" side verification to ensure that the id is not invalid
This function will use the referenced string to hold the actual error
*/
BOOL CNewPatient::IsIdInvalid(int id, CString& strErrors )
{
	BOOL bErrorsFound = FALSE;
	
	if (id <= 0)
	{
		//if negative or 0, don't change
		strErrors += "Patient ID must be greater than 0";
		bErrorsFound = TRUE;
	}

	else if (id >= 2147483646)
	{
		strErrors += "Practice cannot store Patient IDs greater than 2,147,483,645.\nPlease enter a smaller Patient ID number.";
		bErrorsFound = TRUE;
	}

	return bErrorsFound;

}

//(a.wilson 2012-1-11) PLID 47485 - function to handle a scan from the barcode scanner
LRESULT CNewPatient::OnBarcodeScannerDataEvent(WPARAM wParam, LPARAM lParam)
{
	try {
		//always success, but still check anyway.
		if ((long)wParam == OPOS_SUCCESS) {
			// (a.walling 2012-01-17 12:55) - PLID 47120 - lParam is a BSTR which we now own
			_bstr_t bstrData((BSTR)lParam, false);
			CString strDLData = (LPCTSTR)bstrData;
			
			//message for user notifying them that they have scanned a 2D barcode and if they wanted 
			//to input the information.
			CString strMsg = "A driver's license has just been scanned.  The information from the license will "
				"fill the demographic fields for this new patient and any information already in those "
				"fields will be overwritten. \r\n\r\nAre you sure you want to import the information from the driver's license?";
			long nResult = DontShowMeAgain(this, strMsg, "OposBarcodeScanner_GetInfoForNewPatient", 
				"Get New Patient Demographics from Driver's License", FALSE, TRUE);
			
			if (nResult == IDYES || nResult == IDOK) {
				//fill them all in.
				BarcodeUtils::DriversLicenseInfo dlInfo(strDLData);

				SetDlgItemText(IDC_FIRST_NAME_BOX, dlInfo.strFirst);
				SetDlgItemText(IDC_MIDDLE_NAME_BOX, dlInfo.strMiddle);
				SetDlgItemText(IDC_LAST_NAME_BOX, dlInfo.strLast);
				SetDlgItemText(IDC_ADDRESS1_BOX, dlInfo.strAddress);
				SetDlgItemText(IDC_ADDRESS2_BOX, "");
				SetDlgItemText(IDC_CITY_BOX, dlInfo.strCity);
				SetDlgItemText(IDC_STATE_BOX, dlInfo.strState);
				SetDlgItemText(IDC_ZIP_BOX, dlInfo.strZip);

				//gender
				// (j.jones 2016-02-25 15:52) - PLID 67549 - added enums for the Gender combo
				if (dlInfo.strGender.CompareNoCase("male") == 0) {
					// Make sure to select the correct prefix
					OnSelChosenGender(m_GenderCombo->SetSelByColumn(gccID, (long)1));
				} else if (dlInfo.strGender.CompareNoCase("female") == 0) {
					// Make sure to select the correct prefix
					OnSelChosenGender(m_GenderCombo->SetSelByColumn(gccID, (long)2));
				} else {
					// Make sure to select the correct prefix
					OnSelChosenGender(m_GenderCombo->SetSelByColumn(gccID, (long)0));
				}
				//birthdate
				COleDateTime cdtBirthdate;
				cdtBirthdate.ParseDateTime(dlInfo.strDOB, VAR_DATEVALUEONLY);

				if (cdtBirthdate.GetStatus() != COleDateTime::invalid && 
					cdtBirthdate.GetStatus() != COleDateTime::error && 
					cdtBirthdate.GetStatus() != COleDateTime::null) {
					
					m_nxtBirthDate->SetDateTime(cdtBirthdate);
				} else {
					m_nxtBirthDate->Clear();
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
	
	return TRUE;
}

// (r.gonet 05/16/2012) - PLID 48561 - Add in the standard <No Selection> option
void CNewPatient::RequeryFinishedNpAffiliatePhysList(short nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAffiliatePhysList->GetNewRow();
		pRow->PutValue(afphID, g_cvarNull);
		pRow->PutValue(afphName, "<No Affiliate Physician>");
		pRow->PutValue(afphNpi, "");
		pRow->PutValue(afphAffiliatePhysID, "");
		pRow->PutValue(afphUpin, "");
		pRow->PutValue(afphAddress1, "");
		pRow->PutValue(afphAddress2, "");
		pRow->PutValue(afphCity, "");
		pRow->PutValue(afphState, "");
		pRow->PutValue(afphZip, "");
		m_pAffiliatePhysList->AddRowBefore(pRow, m_pAffiliatePhysList->GetFirstRow());
		m_pAffiliatePhysList->CurSel = pRow;	

	} NxCatchAll(__FUNCTION__);
}
// (s.dhole 2013-06-04 13:24) - PLID  12018
void CNewPatient::SelChangingNewPatientLocation(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		// (s.dhole 2013-06-04 13:07) - PLID  12018
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-07-17 12:07) - PLID 62774 - Check if birth date valid. Fix if set to future date.
bool CNewPatient::ValidateDateOfBirth()
{
	CString dob;
	return ValidateDateOfBirth(dob);
}

// (r.goldschmidt 2014-07-17 12:07) - PLID 62774 - Check if birth date valid, give dob string for queries. Fix if set to future date.
bool CNewPatient::ValidateDateOfBirth(CString& dob)
{
	//DRT - 10/5/01 - Allow them to still save if the date has been made screwy, but then changed back to nothing
	COleDateTime dtBirthDate;
	dtBirthDate.SetStatus(COleDateTime::invalid);
	if (m_nxtBirthDate->GetStatus() != 3) {
		dtBirthDate = m_nxtBirthDate->GetDateTime();
		COleDateTime dt;
		dt.ParseDateTime("01/01/1800");
		if (m_nxtBirthDate->GetStatus() == 1 && dtBirthDate.m_dt >= dt.m_dt) {
			if (dtBirthDate > COleDateTime::GetCurrentTime()) {
				AfxMessageBox("You have entered a birthdate in the future. This will be adjusted to a valid date.");
				// (r.goldschmidt 2014-07-18 15:07) - PLID 62973 - make sure birth date gets set to valid date
				while (dtBirthDate > COleDateTime::GetCurrentTime()){ 
					dtBirthDate.SetDate(dtBirthDate.GetYear() - 100, dtBirthDate.GetMonth(), dtBirthDate.GetDay());
				}
				m_nxtBirthDate->SetDateTime(dtBirthDate);
			}
			dob = "'" + _Q(FormatDateTimeForSql(dtBirthDate)) + "'";
		}
		else {
			MsgBox(RCS(IDS_BIRTHDAY_INVALID));
			return false;
		}
	}
	else {
		dob = "NULL";
	}

	return true;
}

// (r.goldschmidt 2014-07-17 12:21) - PLID 62774 - tests if we are ready to check for duplicates given parameter set. (contains rule set for performing check)
bool CNewPatient::ReadyToCheckForDuplicatePatients()
{

	CString first, middle, last;
	COleDateTime dtBirthDate;

	GetDlgItemText(IDC_FIRST_NAME_BOX, first);
	GetDlgItemText(IDC_MIDDLE_NAME_BOX, middle);
	GetDlgItemText(IDC_LAST_NAME_BOX, last);

	// no point in checking duplicates if birth date is invalid and hasn't been fixed yet
	// when killed focus from date of birth field, will have already corrected future dates
	dtBirthDate.SetStatus(COleDateTime::invalid);
	if (m_nxtBirthDate->GetStatus() != 3) {
		dtBirthDate = m_nxtBirthDate->GetDateTime();
		COleDateTime dt;
		dt.ParseDateTime("01/01/1800");
		if (m_nxtBirthDate->GetStatus() != 1 || dtBirthDate.m_dt < dt.m_dt) {
			return false;
		}
	}

	// checks if same as last duplicate check
	bool bFirstNameSame = !m_strCurrentFirstName.Compare(first);
	bool bMiddleNameSame = !m_strCurrentMiddleName.Compare(middle);
	bool bLastNameSame = !m_strCurrentLastName.Compare(last);
	bool bBirthDateSame = m_dtCurrentBirthDate.GetStatus() == dtBirthDate.GetStatus() &&
		((dtBirthDate.GetStatus() == COleDateTime::invalid) || (m_dtCurrentBirthDate.m_dt == dtBirthDate.m_dt));
	// checks relevant preferences
	bool bHasPreferenceCheckBirthDate = (GetRemotePropertyInt("NewPatientsCheckBirthDate", 1, 0, "<None>") == 1);
	bool bRequireBirthDate = (m_lRequiredFields & NEWPT_REQUIRED_DOB) != 0;

	// cases when we check for patient duplicates:
	//  1a.) if we have already checked for patient duplicates and either first, last, or birth date is changed (not middle, currently)
	//  1b.) if we have already checked for patient duplicates and either first or last is changed (when preference to check birthdate is off)
	//  2a.) if we've never checked for patient duplicates and first, last, and birth date are filled (not middle, currently)
	//  2b.) if we've never checked for patient duplicates and first and last are filled in (when preference to check birthdate is off)
	{
		if (m_bTestForDuplicates){ // case 1
			if (bFirstNameSame &&
				// bMiddleNameSame && // middle name isn't currently checked by Duplicate Dialog
				bLastNameSame &&
				(bBirthDateSame || !bHasPreferenceCheckBirthDate))  // takes care of difference between 1a and 1b
			{
				// don't need to continue checking, not ready to check for duplicates
				return false;
			}
		}
		else { // case 2
			if (first.IsEmpty() ||
				// middle.IsEmpty() || // middle name isn't currently checked by Duplicate Dialog
				last.IsEmpty() ||
				((dtBirthDate.GetStatus() == COleDateTime::invalid) &&
				bHasPreferenceCheckBirthDate))  // takes care of difference between 2a and 2b
			{
				// don't need to continue checking, not ready to check for duplicates
				return false;
			}
		}
	}

	// if we get to this point, we will be checking for duplicates
	return true;

}

// (r.goldschmidt 2014-07-17 12:21) - PLID 62774 - In New Patient Dialog, check against possible duplicates once first name, last name, and date of birth are entered.
bool CNewPatient::CheckForDuplicatePatients()
{

	// remember values (want to remember values at time of last duplicate check for future comparisons)
	GetDlgItemText(IDC_FIRST_NAME_BOX, m_strCurrentFirstName);
	GetDlgItemText(IDC_MIDDLE_NAME_BOX, m_strCurrentMiddleName);
	GetDlgItemText(IDC_LAST_NAME_BOX, m_strCurrentLastName);
	m_dtCurrentBirthDate.SetStatus(COleDateTime::invalid);
	if (m_nxtBirthDate->GetStatus() != 3) {
		m_dtCurrentBirthDate = m_nxtBirthDate->GetDateTime();
		if (m_nxtBirthDate->GetStatus() != 1) {
			ASSERT(FALSE); // should never get here if dob validation is confirmed properly
			m_dtCurrentBirthDate.SetStatus(COleDateTime::invalid);
		}
	}
	m_bTestForDuplicates = true;

	// (r.goldschmidt 2014-07-22 12:13) - PLID 62774 - moved from CNewPatient::Save(void)
	CDuplicate dlg(this);
	dlg.SetStatusFilter(EStatusFilterTypes(esfPatient | esfProspect | esfPatientProspect));
	dlg.EnableSaveBtn(FALSE); // (r.goldschmidt 2014-07-22 12:13) - PLID 62774 - altered dialog functionality and UI
	// (z.manning 2009-08-24 14:34) - PLID 31135 - Added a parameter for birth date
	if (dlg.FindDuplicates(_Q(m_strCurrentFirstName), _Q(m_strCurrentLastName), _Q(m_strCurrentMiddleName), m_dtCurrentBirthDate)) {

		//(e.lally 2008-02-27) PLID 27379 - Updated Duplicate dlg to return more descriptive ID names
		int choice = dlg.DoModal();
		if (choice == ID_CHANGE_NEW_PATIENT_NAME) {
			return true;
		}
		else if (choice == ID_CANCEL_NEW_PATIENT || choice == ID_CANCEL_AND_GOTO_SELECTED) {
			//DRT 3/10/03 - If we aborted, we either 1)  Are cancelling, or 2)  Are switching to a patient (cancel + switch).
			//		If we are switching, the m_nSelPatientId variable will be set.
			//		Copied this code from ToDoAlarmDlg - really should be made more modular.
			if (dlg.m_nSelPatientId != -1) {
				//cancel this dialog and switch to the patient
				CMainFrame *p = GetMainFrame();
				CNxTabView *pView;

				if (dlg.m_nSelPatientId != GetActivePatientID()) {
					if (!p->m_patToolBar.DoesPatientExistInList(dlg.m_nSelPatientId)) {
						if (IDNO == MessageBox("This patient is not in the current lookup. \n"
							"Do you wish to reset the lookup to include all patients?", "Practice", MB_ICONQUESTION | MB_YESNO)) {
							return true;
						}
					}
					//TES 1/7/2010 - PLID 36761 - This function may fail now
					if (!p->m_patToolBar.TrySetActivePatientID(dlg.m_nSelPatientId)) {
						return true;
					}
				}

				if (p->FlipToModule(PATIENT_MODULE_NAME)) {

					pView = (CNxTabView *)p->GetOpenView(PATIENT_MODULE_NAME);
					if (pView)
					{
						if (pView->GetActiveTab() != 0)
							pView->SetActiveTab(0);
					}

					p->UpdateAllViews();

					//cancel the new patient dlg
					GetMainFrame()->EnableHotKeys();
					//Tell main frame not to send us any more table checker messages.
					GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
					EndDialog(0);
				}
				return true;
			}
			else {
				//cancel the dialog
				GetMainFrame()->EnableHotKeys();
				//Tell main frame not to send us any more table checker messages.
				GetMainFrame()->UnrequestTableCheckerMessages(GetSafeHwnd());
				EndDialog(0);
				return true;
			}
		}
	}
	else {
		return false;
	}

	return true;

}

// (r.goldschmidt 2014-07-17 17:50) - PLID 62774 - In New Patient Dialog, check against possible duplicates once date of birth is entered.
void CNewPatient::KillFocusDobBox()
{
	try{
		if (ValidateDateOfBirth()) {
			if (ReadyToCheckForDuplicatePatients()){
				CheckForDuplicatePatients();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (v.maida - 2014-08-18 16:30) - PLID 30758 - Determine what type of person was added into the New Contact dialog, and return an enumeration indicating 
// the choice.
int CNewPatient::GetContactChoice(long nContactID)
{
	try {
		_RecordsetPtr prsContactChoice = CreateParamRecordset("SELECT CASE WHEN EXISTS(SELECT PersonID FROM ProvidersT WHERE PersonID = {INT}) THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS IsProvider, "
			"CASE WHEN EXISTS(SELECT PersonID FROM ReferringPhysT WHERE PersonID = {INT}) THEN CAST(1 AS BIT) ELSE CAST(0 AS BIT) END AS IsReferringPhys", nContactID, nContactID);
		if (AdoFldBool(prsContactChoice, "IsProvider")){
			return EContactChoices::providerChoice;
		}
		else if (AdoFldBool(prsContactChoice, "IsReferringPhys")){
			return EContactChoices::refPhysChoice;
		}
	}NxCatchAll(__FUNCTION__);
	return -1;
}

// (v.maida 2014-08-18 16:14) - PLID 30758 - Added a single function to handle choosing referring physicians and/or providers. Most of the code copied over and altered from the previous OnNewRefPhys() function.
void CNewPatient::HandleContactChoice(EContactChoices eContactChoice)
{
	try {
		long nNewContactID = -1;
		if (eContactChoice == EContactChoices::providerChoice) {
			// this will create a contact and return the ID of the contact that was added. Make the New Contact dialog's default contact type a provider, 
			// since the ellipse button by the provider dropdown was clicked.
			nNewContactID = GetMainFrame()->AddContact(GetMainFrame()->dctProvider, FALSE);
		}
		else if (eContactChoice == EContactChoices::refPhysChoice) {
			nNewContactID = GetMainFrame()->AddContact(GetMainFrame()->dctRefPhys, FALSE);
		}
		
		if (nNewContactID != -1) // return value of -1 indicates no new contact was added / chosen
		{
			switch (GetContactChoice(nNewContactID)) // get what was actually chosen in the Add Contact dialog, which may not be the same as the field corresponding to the button that was clicked
			{
				case EContactChoices::refPhysChoice:
				{
					//manually add ref phys to the dataList
					_RecordsetPtr prsNewRefPhys = CreateParamRecordset("SELECT PersonT.FullName, "
						"NPI, ReferringPhyID, UPIN, ShowProcWarning, ProcWarning, "
						"Address1, Address2, City, State, Zip "
						"FROM ReferringPhysT "
						"INNER JOIN PersonT ON ReferringPhysT.PersonID = PersonT.ID "
						"WHERE ReferringPhysT.PersonID = {INT}", nNewContactID);
					FieldsPtr pFlds = prsNewRefPhys->Fields;
					CString strFullName = AdoFldString(prsNewRefPhys, "FullName");
					_variant_t varCurSelID = m_pRefPhysList->GetValue(m_pRefPhysList->GetCurSel(), rpcID); // get a variant for the ID column of the current dropdown selection
					CString strCurSelName = VarString(m_pRefPhysList->GetValue(m_pRefPhysList->GetCurSel(), rpcName));
					NXDATALISTLib::IRowSettingsPtr pRow;
					pRow = m_pRefPhysList->GetRow(NXDATALISTLib::sriGetNewRow);
					_variant_t varID, varName;
					varID = nNewContactID;
					varName = strFullName;

					// (j.jones 2011-12-01 14:11) - PLID 46771 - this now uses the enums
					pRow->PutValue(rpcID, varID);
					pRow->PutValue(rpcName, varName);
					pRow->PutValue(rpcNpi, pFlds->Item["NPI"]->Value);
					pRow->PutValue(rpcRefPhysID, pFlds->Item["ReferringPhyID"]->Value);
					pRow->PutValue(rpcUpin, pFlds->Item["UPIN"]->Value);
					pRow->PutValue(rpcShowProcWarning, pFlds->Item["ShowProcWarning"]->Value);
					pRow->PutValue(rpcProcWarning, pFlds->Item["ProcWarning"]->Value);
					pRow->PutValue(rpcAddress1, pFlds->Item["Address1"]->Value);
					pRow->PutValue(rpcAddress2, pFlds->Item["Address2"]->Value);
					pRow->PutValue(rpcCity, pFlds->Item["City"]->Value);
					pRow->PutValue(rpcState, pFlds->Item["State"]->Value);
					pRow->PutValue(rpcZip, pFlds->Item["Zip"]->Value);
					long nNewRow = m_pRefPhysList->AddRow(pRow);

					if (varCurSelID.vt == VT_NULL && strCurSelName == "<No Referring Physician>") // check for <No Referring Physician> row
					{
						// they didn't have a referring physician selected before, so set the dropdown to the new ref phys
						m_pRefPhysList->PutCurSel(nNewRow);
						return;
					}
				}
				break;

				case EContactChoices::providerChoice:
				{
					// manually add provider to datalist
					_RecordsetPtr prsNewDoc = CreateParamRecordset("SELECT ProvidersT.PersonID, "
						"PersonT.FullName "
						"FROM ProvidersT "
						"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
						"WHERE ProvidersT.PersonID = {INT}", nNewContactID);
					CString strFullName = AdoFldString(prsNewDoc, "FullName", "");
					_variant_t varCurSelID = m_pDocList->GetValue(m_pDocList->GetCurSel(), pcID); // get a variant for the ID column of the current dropdown selection
					CString strCurSelName = VarString(m_pDocList->GetValue(m_pDocList->GetCurSel(), pcFullName));
					_variant_t varID, varName;
					varID = nNewContactID;
					varName = strFullName;

					NXDATALISTLib::IRowSettingsPtr pRow = m_pDocList->GetRow(NXDATALISTLib::sriGetNewRow);
					pRow->PutValue(pcID, varID);
					pRow->PutValue(pcFullName, varName); 
					long nNewRow = m_pDocList->AddRow(pRow);

					if (varCurSelID.vt == VT_NULL && strCurSelName == "<No Provider>") // check for <No Provider> row
					{
						// they didn't have a provider selected before, so set the drop down to the new provider
						m_pDocList->PutCurSel(nNewRow);
						return;
					}
				}
				break;
			}
		}
	}NxCatchAll("Error in HandleContactChoice");
}

// (r.farnworth 2016-03-03 12:16) - PLID 68454 - Edit the New Patient dialog so that it can be preloaded with patient information when it's launched.
void CNewPatient::PopulateWithPreselects()
{
	if (m_strPreselectFirst != "")
	{
		SetDlgItemText(IDC_FIRST_NAME_BOX, m_strPreselectFirst);
	}

	if (m_strPreselectLast != "")
	{
		SetDlgItemText(IDC_LAST_NAME_BOX, m_strPreselectLast);
	}

	if (m_dtPreselectBirthdate.GetStatus() != COleDateTime::invalid)
	{
		m_nxtBirthDate->SetDateTime(m_dtPreselectBirthdate);
	}

	if (m_strPreselectAddress1 != "")
	{
		SetDlgItemText(IDC_ADDRESS1_BOX, m_strPreselectAddress1);
	}

	if (m_strPreselectAddress2 != "")
	{
		SetDlgItemText(IDC_ADDRESS2_BOX, m_strPreselectAddress2);
	}

	if (m_strPreselectCity != "")
	{
		SetDlgItemText(IDC_CITY_BOX, m_strPreselectCity);
	}

	if (m_strPreselectState != "")
	{
		SetDlgItemText(IDC_STATE_BOX, m_strPreselectState);
	}

	if (m_strPreselectZip != "")
	{
		SetDlgItemText(IDC_ZIP_BOX, m_strPreselectZip);
	}

	if (m_strPreselectHomePhone != "")
	{
		SetDlgItemText(IDC_HOME_PHONE_BOX, m_strPreselectHomePhone);
	}

	if (m_strPreselectCellPhone != "")
	{
		SetDlgItemText(IDC_CELL_PHONE_BOX, m_strPreselectCellPhone);
	}

	if (m_strPreselectEmail != "")
	{
		SetDlgItemText(IDC_EMAIL_BOX, m_strPreselectEmail);
	}
}
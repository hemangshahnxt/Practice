// General2Dlg.cpp : implementation file
//"God help us all"

#include "stdafx.h"
#include "General2Dlg.h"
#include "MainFrm.h"
#include "GlobalUtils.h"
//#include "LabelControl.h"
//#include "SelectStatusDlg.h"
#include "PracProps.h"
#include "NxStandard.h"
//#include "EditComboBox.h" // (b.spivey, May 14, 2012) - PLID 50224 - deprecated. 
#include "globalDataUtils.h"
#include "ReferralTreeDlg.h"
#include "ReferredPatients.h"
#include "AuditTrail.h"
#include "NxSecurity.h"
#include "CopyEmployerDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "patientsrc.h"
#include "GlobalSchedUtils.h"
#include "ppclink.h"
#include "NexwebLoginInfoDlg.h"
#include "EditFamilyDlg.h"
#include "Rewards.h"
#include "InvUtils.h"
#include "InvPatientAllocationDlg.h"
#include "RaceEditDlg.h"
#include "PatientView.h"
#include "ContactView.h" // (k.messina 2010-04-12 11:15) - PLID 37957
#include "EditWarningCategoriesDlg.h"
#include "WarrantyDlg.h" // (c.copits 2010-10-28) PLID 38598 - Warranty tracking system
#include "EditPatientTypeDlg.h" // (b.spivey, May 08, 2012) - PLID 50224 - New dialog to replace old combo dialog.
#include "HL7Utils.h"	// (j.armen 2012-06-07 13:11) - PLID 50825
#include "MultiSelectDlg.h" // (b.spivey, May 14, 2013) - PLID 56872 - Allows us to select multiple races.
#include <NxHL7Lib\HL7CommonTypes.h> // (b.eyers 2015-06-22) - PLID 66213
#include "DiagSearchUtils.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (d.thompson 2012-08-09) - PLID 52045 - Fixed too much code-copying while I was renaming this
const CString l_strNoEthnicityText = "< No Ethnicity Selected >";
// (d.thompson 2012-08-13) - PLID 52044 - Same for rae
const CString l_strNoRaceText = "< No Race Selected >";
// (d.thompson 2012-08-13) - PLID 52046 - Same change for language
const CString l_strNoLanguageText = "< No Language Selected >";
// (b.spivey, May 13, 2013) - PLID 56872 - static string 
const CString l_strMultipleRaceText = "< Multiple Races >";

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_ADD		42345
#define ID_REMOVE	42346
#define	ID_MAKE_PRIMARY	42347

// (a.walling 2011-08-01 17:34) - PLID 44788
#define IDT_FREEBIGDATALISTS 1042

// (b.spivey, May 22, 2013) - PLID 56872 - Static value for "Multi-select row"
enum RaceComboRows {
	rcrMultiSelectRow = -2, 
};

// (j.jones 2009-10-14 14:32) - PLID 3427 - added enums for the race and ethnicity dropdowns
enum RaceComboColumns {
	raceID = 0,
	raceName,
	raceRaceName,
};

enum EthnicityComboColumns {
	ethnicityID = 0,
	ethnicityName,
};

// (j.gruber 2011-09-27 15:05) - PLID 45357 - affiliate physician
// (r.gonet 03/02/2012) - PLID 48600 - Added phone and fax columns
enum AffiliatePhysicianColumns {
	afclID = 0,
	afclName,
	afclNPI,
	afclPhone,
	afclFax,
	afclAddress1,
	afclAddress2,
	afclCity,
	afclState,
	afclZip,
};

// (j.gruber 2012-10-16 17:48) - PLID 47289 - affiliate phys type list
enum AffliatePhysTypeColumns {
	aptcID = 0,
	aptcName,
};

// (a.wilson 2012-5-23) PLID 48537 - add enum for referral source columns
enum ReferralSourceColumns {
	rscID = 0,
	rscReferralName = 1,
	rscDate = 2,
};

// (j.jones 2014-02-18 17:16) - PLID 60719 - added a diagnosis code list
enum DiagnosisCodeListColumns {
	dclcIndex = 0,
	dclcDiagICD9CodeID,
	dclcDiagICD9Code,
	dclcDiagICD9Desc,
	dclcDiagICD10CodeID,
	dclcDiagICD10Code,
	dclcDiagICD10Desc,
};

#define LOG_IF_TOO_SLOW_BEFORE(obj, alt)				\
	DWORD __dwBefore_##obj##__##alt##__ = GetTickCount();

#define LOG_IF_TOO_SLOW_AFTER(obj, alt, toolong, msg)	\
	DWORD __dwDuration_##obj##__##alt##__ = GetTickCount() - __dwBefore_##obj##__##alt##__; if (__dwDuration_##obj##__##alt##__ > 2000) { \
	Log("PERFORMANCE WARNING: " #obj "(" #alt ") took too long (%lu ms):  %s", __dwDuration_##obj##__##alt##__, msg); }

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXTIMELib;

/////////////////////////////////////////////////////////////////////////////
// CGeneral2Dlg dialog
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker

CGeneral2Dlg::CGeneral2Dlg(CWnd* pParent)
	: CPatientDialog(CGeneral2Dlg::IDD, pParent)
	,
	m_refphyChecker(NetUtils::RefPhys),
	m_diagChecker(NetUtils::DiagCodes),

	m_patienttypeChecker(NetUtils::GroupTypes),
	m_defaultlocChecker(NetUtils::LocationsT),
	m_refPatChecker(NetUtils::PatCombo)
{
	//{{AFX_DATA_INIT(CGeneral2Dlg)
	//}}AFX_DATA_INIT
	// (a.walling 2010-10-12 14:40) - PLID 40908
	m_id = -1;

	
	// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
	//m_bSettingBox = false;
	// (a.walling 2010-10-13 10:44) - PLID 40908 - Dead code
	//m_bAutoRefreshing = false;

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Patient_Information/print_patient_demographics.htm";
	m_nLastEditLength = 0;
	m_bSettingCompany = false;

	m_bHasAdvInventoryLicense = FALSE;
}


void CGeneral2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGeneral2Dlg)
	DDX_Control(pDX, IDC_ADD_ALLOCATION, m_btnAddAllocation);
	DDX_Control(pDX, IDC_EXPIRE_DATE, m_btnExpiresOn);
	DDX_Control(pDX, IDC_MAKE_PRIMARY_REFERRAL, m_btnMakePrimary);
	DDX_Control(pDX, IDC_REMOVE_REFERRAL, m_btnRemoveReferral);
	DDX_Control(pDX, IDC_ADD_REFERRAL, m_btnAddReferral);
	DDX_Control(pDX, IDC_WARNING_CHECK, m_WarningCheck);
	DDX_Control(pDX, IDC_EMPLOYMENT_BKG, m_empBkg);
	DDX_Control(pDX, IDC_EMPLOYMENT3_BKG, m_emp3Bkg);
	DDX_Control(pDX, IDC_REFERRAL_BKG, m_referralBkg);
	DDX_Control(pDX, IDC_LASTMODIFIED_BKG, m_lastupdatedBkg);
	DDX_Control(pDX, IDC_RACE_BKG, m_raceBkg);
	DDX_Control(pDX, IDC_FULLTIME_RADIO, m_fulltime);
	DDX_Control(pDX, IDC_FULLTIMESTUDENT_RADIO, m_fulltimeStudent);
	DDX_Control(pDX, IDC_PARTTIMESTUDENT_RADIO, m_parttimeStudent);
	DDX_Control(pDX, IDC_PARTTIME_RADIO, m_parttime);
	DDX_Control(pDX, IDC_RETIRED_RADIO, m_retired);
	DDX_Control(pDX, IDC_OTHER_RADIO, m_other);
	DDX_Control(pDX, IDC_TYPE_BKG, m_typeBkg);
	DDX_Control(pDX, IDC_EXPIRE_DATE_PICKER, m_expireDate);
	DDX_Control(pDX, IDC_NEXWEB_INFO_BKG, m_NexwebLoginInfoBkg);
	DDX_Control(pDX, IDC_OCCUPATION, m_nxeditOccupation);
	DDX_Control(pDX, IDC_COMPANY, m_nxeditCompany);
	DDX_Control(pDX, IDC_MANAGER_FNAME, m_nxeditManagerFname);
	DDX_Control(pDX, IDC_MANAGER_MNAME, m_nxeditManagerMname);
	DDX_Control(pDX, IDC_MANAGER_LNAME, m_nxeditManagerLname);
	DDX_Control(pDX, IDC_EMP_ADDRESS1, m_nxeditEmpAddress1);
	DDX_Control(pDX, IDC_EMP_ADDRESS2, m_nxeditEmpAddress2);
	DDX_Control(pDX, IDC_EMP_ZIP, m_nxeditEmpZip);
	DDX_Control(pDX, IDC_EMP_CITY, m_nxeditEmpCity);
	DDX_Control(pDX, IDC_EMP_STATE, m_nxeditEmpState);
	DDX_Control(pDX, IDC_WARNING, m_nxeditWarning);
	DDX_Control(pDX, IDC_EDIT_WARNING_USER, m_nxeditEditWarningUser);
	DDX_Control(pDX, IDC_NUM_PATIENTS_REF, m_nxeditNumPatientsRef);
	DDX_Control(pDX, IDC_NUM_PROSPECTS_REF, m_nxeditNumProspectsRef);
	DDX_Control(pDX, IDC_LAST_MODIFIED, m_nxeditLastModified);
	DDX_Control(pDX, IDC_ACCUM_REWARD_PTS, m_nxeditAccumRewardPts);
	DDX_Control(pDX, IDC_G2_LOCATION_LABEL, m_nxstaticG2LocationLabel);
	DDX_Control(pDX, IDC_DONOR_LABEL, m_nxstaticDonorLabel);
	DDX_Control(pDX, IDC_NUM_PATIENTS_REFERRED_LABEL, m_nxstaticNumPatientsReferredLabel);
	DDX_Control(pDX, IDC_NUM_PROSPECTS_REFERRED_LABEL, m_nxstaticNumProspectsReferredLabel);
	DDX_Control(pDX, IDC_SERIALIZED_PRODUCTS_LABEL, m_nxstaticSerializedProductsLabel);
	DDX_Control(pDX, IDC_ADJUST_REWARD_POINTS, m_btnEditRewardPoints);
	//(c.copits 2010-10-28) PLID 38598 - Warranty tracking system
	DDX_Control(pDX, IDC_WARRANTY_BKG, m_WarrantyBkg);
	DDX_Control(pDX, IDC_WARNING_CHECK2, m_RewardsWarningCheck); // (j.luckoski 2013-03-04 11:24) - PLID 33548 - Prompt if patient has rewards
	DDX_Control(pDX, IDC_RACE_LABEL, m_nxstaticMultiRaceLabel);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CGeneral2Dlg, IDC_EXPIRE_DATE_PICKER, 2 /* Change */, OnChangeExpireDatePicker, VTS_NONE)

BEGIN_MESSAGE_MAP(CGeneral2Dlg, CNxDialog)
	//{{AFX_MSG_MAP(CGeneral2Dlg)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_WARNING_CHECK, OnWarningCheck)
	ON_BN_CLICKED(IDC_FULLTIME_RADIO, OnFulltimeRadio)
	ON_BN_CLICKED(IDC_FULLTIMESTUDENT_RADIO, OnFulltimeStudentRadio)
	ON_BN_CLICKED(IDC_RETIRED_RADIO, OnRetiredRadio)
	ON_BN_CLICKED(IDC_PARTTIME_RADIO, OnParttimeRadio)
	ON_BN_CLICKED(IDC_PARTTIMESTUDENT_RADIO, OnParttimeStudentRadio)
	ON_BN_CLICKED(IDC_OTHER_RADIO, OnOtherRadio)
	ON_BN_CLICKED(IDC_EDIT_PATIENT_TYPE_COMBO, OnEditPatientTypeCombo)
	ON_BN_CLICKED(IDC_BTN_EDITRACE, OnEditRaceCombo)
	ON_BN_CLICKED(IDC_ADD_REFERRAL, OnAddReferral)
	ON_BN_CLICKED(IDC_REMOVE_REFERRAL, OnRemoveReferral)
	ON_BN_CLICKED(IDC_MAKE_PRIMARY_REFERRAL, OnMakePrimaryReferral)
	ON_BN_CLICKED(IDC_REFERRED_PATS, OnReferredPats)
	ON_BN_CLICKED(IDC_REFERRED_PROSPECTS, OnReferredProspects)
	ON_EN_KILLFOCUS(IDC_WARNING, OnKillfocusWarning)
	ON_BN_CLICKED(IDC_COPY_EMPLOYER_BTN, OnCopyEmployerBtn)
	ON_BN_CLICKED(IDC_NEW_REF_PHYS, OnNewRefPhys)
	ON_BN_CLICKED(IDC_NEW_PCP, OnNewPcp)
	ON_BN_CLICKED(IDC_EXPIRE_DATE, OnExpireDate)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_EXPIRE_DATE_PICKER, OnChangeExpireDatePicker)
	ON_EN_CHANGE(IDC_COMPANY, OnChangeCompany)
	ON_BN_CLICKED(IDC_EDIT_NEXWEB_INFO, OnEditNexwebInfo)
	ON_BN_CLICKED(IDC_BTN_FAMILY, OnBtnEditFamilies)
	ON_BN_CLICKED(IDC_GOTO_REF_PHYS, OnGotoRefPhys)
	ON_BN_CLICKED(IDC_GOTO_REF_PAT, OnGotoRefPat)
	ON_BN_CLICKED(IDC_GOTO_PCP, OnGotoPcp)
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_ADD, OnAddReferral)
	ON_COMMAND(ID_REMOVE, OnRemoveReferral)
	ON_COMMAND(ID_MAKE_PRIMARY, OnMakePrimaryReferral)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_ADD_ALLOCATION, OnAddAllocation)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ADJUST_REWARD_POINTS, &CGeneral2Dlg::OnBnClickedAdjustRewardPoints)
	//(c.copits 2010-10-28) PLID 38598 - Warranty tracking system
	ON_BN_CLICKED(IDC_WARRANTY_BTN, &CGeneral2Dlg::OnBnClickedWarranty)
	ON_WM_SHOWWINDOW() // (a.walling 2011-08-01 17:34) - PLID 44788
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_GOTO_AFFILIATE, &CGeneral2Dlg::OnBnClickedGotoAffiliate)
	ON_BN_CLICKED(IDC_WARNING_CHECK2, OnBnClickedWarningCheck2) // (j.luckoski 2013-03-04 11:46) - PLID 33548 - Prompt reward point balance
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CGeneral2Dlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CGeneral2Dlg)
	ON_EVENT(CGeneral2Dlg, IDC_REFERRING_PATIENT_COMBO, 16 /* SelChosen */, OnSelChosenReferringPatientCombo, VTS_I4)
	// (a.walling 2011-08-01 17:34) - PLID 44788
	ON_EVENT(CGeneral2Dlg, IDC_REFERRING_PATIENT_COMBO, 26 /* DroppingDown */, OnDroppingDownReferringPatientCombo, VTS_NONE)
	ON_EVENT(CGeneral2Dlg, IDC_CODE_RACE, 16 /* SelChosen */, OnSelChosenRaceCombo, VTS_I4)
	ON_EVENT(CGeneral2Dlg, IDC_DEFAULT_LOCATION_COMBO, 16 /* SelChosen */, OnSelChosenDefaultLocationCombo, VTS_I4)
	ON_EVENT(CGeneral2Dlg, IDC_REFERRING_PHYSICIAN_COMBO, 16 /* SelChosen */, OnSelChosenReferringPhysicianCombo, VTS_I4)
	ON_EVENT(CGeneral2Dlg, IDC_PATIENT_TYPE_COMBO, 16 /* SelChosen */, OnSelChosenPatientTypeCombo, VTS_I4)
	ON_EVENT(CGeneral2Dlg, IDC_PCP_COMBO, 16 /* SelChosen */, OnSelChosenPcpCombo, VTS_I4)
	ON_EVENT(CGeneral2Dlg, IDC_DONOR_PATIENT_COMBO, 16 /* SelChosen */, OnSelChosenDonorPatientCombo, VTS_I4)
	ON_EVENT(CGeneral2Dlg, IDC_REFERRAL_LIST, 9 /* EditingFinishing */, OnEditingFinishingReferralList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CGeneral2Dlg, IDC_REFERRAL_LIST, 10 /* EditingFinished */, OnEditingFinishedReferralList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CGeneral2Dlg, IDC_REFERRAL_LIST, 2 /* SelChanged */, OnSelChangedReferralList, VTS_I4)
	ON_EVENT(CGeneral2Dlg, IDC_DEFAULT_LOCATION_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedDefaultLocationCombo, VTS_I4 VTS_I4)
	ON_EVENT(CGeneral2Dlg, IDC_ACCIDENT_DATE, 1 /* KillFocus */, OnKillFocusAccidentDate, VTS_NONE)
	ON_EVENT(CGeneral2Dlg, IDC_REFERRAL_LIST, 7 /* RButtonUp */, OnRButtonUpReferralList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CGeneral2Dlg, IDC_REFERRAL_LIST, 18 /* RequeryFinished */, OnRequeryFinishedReferralList, VTS_I2)
	ON_EVENT(CGeneral2Dlg, IDC_PATIENT_TYPE_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedPatientTypeCombo, VTS_I2)
	ON_EVENT(CGeneral2Dlg, IDC_RELATION, 1 /* SelChanging */, OnSelChangingRelation, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CGeneral2Dlg, IDC_RELATION, 16 /* SelChosen */, OnSelChosenRelation, VTS_DISPATCH)
	// (a.walling 2011-08-01 17:34) - PLID 44788
	//ON_EVENT(CGeneral2Dlg, IDC_RELATION, 20 /* TrySetSelFinished */, OnTrySetSelFinishedRelation, VTS_I4 VTS_I4)
	ON_EVENT(CGeneral2Dlg, IDC_RELATION, 31 /* DroppingDown */, OnDroppingDownRelation, VTS_NONE)
	ON_EVENT(CGeneral2Dlg, IDC_SERIALIZED_PRODUCT_LIST, 7 /* RButtonUp */, OnRButtonUpSerializedProductList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CGeneral2Dlg, IDC_LANGUAGE_COMBO, 16, OnSelChosenLanguageCombo, VTS_DISPATCH)
	ON_EVENT(CGeneral2Dlg, IDC_ETHNICITY_COMBO, 16, OnSelChosenEthnicityCombo, VTS_I4)
	//}}AFX_EVENTSINK_MAP	
	ON_EVENT(CGeneral2Dlg, IDC_LIST_WARNING_CATEGORIES, 1, CGeneral2Dlg::SelChangingListWarningCategories, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CGeneral2Dlg, IDC_LIST_WARNING_CATEGORIES, 16, CGeneral2Dlg::SelChosenListWarningCategories, VTS_DISPATCH)
	ON_EVENT(CGeneral2Dlg, IDC_LIST_WARNING_CATEGORIES, 18, CGeneral2Dlg::RequeryFinishedListWarningCategories, VTS_I2)
	ON_EVENT(CGeneral2Dlg, IDC_LIST_WARNING_CATEGORIES, 28, CGeneral2Dlg::CurSelWasSetListWarningCategories, VTS_NONE)
	ON_EVENT(CGeneral2Dlg, IDC_AFFILIATE_PHYS_LIST, 16, CGeneral2Dlg::SelChosenAffiliatePhysList, VTS_DISPATCH)
	ON_EVENT(CGeneral2Dlg, IDC_AFFILIATE_PHYS_LIST, 18, CGeneral2Dlg::RequeryFinishedAffiliatePhysList, VTS_I2)
	ON_EVENT(CGeneral2Dlg, IDC_AFFILIATE_PHYS_LIST, 1, CGeneral2Dlg::SelChangingAffiliatePhysList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CGeneral2Dlg, IDC_AFF_PHYS_TYPE, 16, CGeneral2Dlg::SelChosenAffPhysType, VTS_DISPATCH)
	ON_EVENT(CGeneral2Dlg, IDC_AFF_PHYS_TYPE, 1, CGeneral2Dlg::SelChangingAffPhysType, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CGeneral2Dlg, IDC_CODE_RACE, 18 /* RequeryFinished */, CGeneral2Dlg::OnRequeryFinishedRaceList, VTS_I2)
	ON_EVENT(CGeneral2Dlg, IDC_OCCUPATION_CENSUS_CODE, 16, CGeneral2Dlg::SelChosenOccupationCensusCode, VTS_DISPATCH)
	ON_EVENT(CGeneral2Dlg, IDC_INDUSTRY_CENSUS_CODE, 16, CGeneral2Dlg::SelChosenIndustryCensusCode, VTS_DISPATCH)
	ON_EVENT(CGeneral2Dlg, IDC_DEFAULT_DIAG_SEARCH_LIST, 16, OnSelChosenDefaultDiagSearchList, VTS_DISPATCH)
	ON_EVENT(CGeneral2Dlg, IDC_DEFAULT_DIAG_CODE_LIST, 6, OnRButtonDownDefaultDiagCodeList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CGeneral2Dlg, IDC_DEFAULT_DIAG_CODE_LIST, 18, OnRequeryFinishedDefaultDiagCodeList, VTS_I2)
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGeneral2Dlg message handlers

void CGeneral2Dlg::SetColor(OLE_COLOR nNewColor)
{
	m_empBkg.SetColor(nNewColor);
	m_emp3Bkg.SetColor(nNewColor);
	m_referralBkg.SetColor(nNewColor);
	m_typeBkg.SetColor(nNewColor);
	m_raceBkg.SetColor(nNewColor);
	m_lastupdatedBkg.SetColor(nNewColor);
	m_NexwebLoginInfoBkg.SetColor(nNewColor);
	//(c.copits 2010-10-28) PLID 38598 - Warranty tracking system
	m_WarrantyBkg.SetColor(nNewColor);
	
	//DRT 4/11/2008 - PLID 29636 - NxIconify
	m_btnAddReferral.AutoSet(NXB_NEW);
	m_btnRemoveReferral.AutoSet(NXB_DELETE);
	m_btnMakePrimary.AutoSet(NXB_MODIFY);

	m_btnAddAllocation.AutoSet(NXB_NEW);
	m_btnEditRewardPoints.AutoSet(NXB_MODIFY); // (z.manning 2010-07-20 12:45) - PLID 30127

	CPatientDialog::SetColor(nNewColor);
}

BOOL CGeneral2Dlg::OnInitDialog() 
{
	try {
		LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__OnInitDialog, 1);

		// (z.manning 2008-12-08 12:33) - PLID 32320 - Cache frequently accessed general2 preferences
		g_propManager.CachePropertiesInBulk("General2", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( "
			"	'RefPhysComboShowNPI' "
			"	, 'RefPhysComboShowID' "
			"	, 'RefPhysComboShowUPIN' "
			"	, 'DisableG2RefPat' "
			"	, 'LookupZipStateByCity' "
			"	, 'FillGeneral2Company' "				// (d.thompson 2010-08-04) - PLID 39897
			"	, 'FillGeneral2CompanyMultiplePrompt' "	// (d.thompson 2010-08-04) - PLID 39897
			"	, 'AutoCapitalizeMiddleInitials' "		// (d.thompson 2010-08-04) - PLID 39897
			"	, 'DefaultPatientReferral' "			// (d.thompson 2010-08-04) - PLID 39897
			"	, 'DefaultPhysicianReferral' "			// (d.thompson 2010-08-04) - PLID 39897
			"	, 'PrimaryReferralPref' "				// (d.thompson 2010-08-04) - PLID 39897
			"	, 'G2ShowWarningStats' "				// (d.thompson 2010-08-04) - PLID 39897
			"	, 'AllowParentReferrals' "				// (r.goldschmidt 2014-09-22 17:17) - PLID 31191
			")", _Q(GetCurrentUserName()));

		// (j.jones 2008-03-20 16:05) - PLID 29334 - cached the adv. inv. license
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		m_bHasAdvInventoryLicense = g_pLicense->HasCandAModule(CLicense::cflrSilent);
		
		if(!m_bHasAdvInventoryLicense) {
			//if they do not have the license, we have to hide the list and move other controls
			//(do this before CNxDialog::OnInitDialog())
			HideSerializedProductList();
		}

		CNxDialog::OnInitDialog();

		// (d.singleton 2012-06-18 11:27) - PLID 51029 set text limit on PersonT.Company field
		m_nxeditCompany.SetLimitText(150);

		COleDateTime dtMin;
		dtMin.SetDate(1753,1,1);
		m_expireDate.SetMinDate(_variant_t(dtMin));

		m_nxtAccidentDate = GetDlgItemUnknown(IDC_ACCIDENT_DATE);
		m_bSavingAccidentDate = false;
		//(e.lally 2007-01-31) PLID 24518 - Make sure our member variable is initialized correctly.
		m_dtAccident.m_dt = 0;
		m_dtAccident.SetStatus(COleDateTime::invalid);

		// (j.jones 2014-02-10 15:25) - PLID 60719 - added the diagnosis search control
		// and list of diagnosis codes
		m_diagSearch = DiagSearchUtils::BindDiagPreferenceSearchListCtrl(this, IDC_DEFAULT_DIAG_SEARCH_LIST, GetRemoteData());
		m_defaultDiagList = BindNxDataList2Ctrl(IDC_DEFAULT_DIAG_CODE_LIST, false);
		//this query will not care if the diagnosis is inactive
		m_defaultDiagList->PutFromClause("("
			"SELECT PatientsT.PersonID, 1 AS IndexNumber, "
			"DiagCodes9.ID AS DiagICD9CodeID, DiagCodes9.CodeNumber AS DiagICD9Code, DiagCodes9.CodeDesc AS DiagICD9Desc, "
			"DiagCodes10.ID AS DiagICD10CodeID, DiagCodes10.CodeNumber AS DiagICD10Code, DiagCodes10.CodeDesc AS DiagICD10Desc "
			"FROM PatientsT "
			"LEFT JOIN DiagCodes DiagCodes9 ON PatientsT.DefaultDiagID1 = DiagCodes9.ID "
			"LEFT JOIN DiagCodes DiagCodes10 ON PatientsT.DefaultICD10DiagID1 = DiagCodes10.ID "
			""
			"UNION ALL "
			"SELECT PatientsT.PersonID, 2 AS IndexNumber, "
			"DiagCodes9.ID AS DiagICD9CodeID, DiagCodes9.CodeNumber AS DiagICD9Code, DiagCodes9.CodeDesc AS DiagICD9Desc, "
			"DiagCodes10.ID AS DiagICD10CodeID, DiagCodes10.CodeNumber AS DiagICD10Code, DiagCodes10.CodeDesc AS DiagICD10Desc "
			"FROM PatientsT "
			"LEFT JOIN DiagCodes DiagCodes9 ON PatientsT.DefaultDiagID2 = DiagCodes9.ID "
			"LEFT JOIN DiagCodes DiagCodes10 ON PatientsT.DefaultICD10DiagID2 = DiagCodes10.ID "
			""
			"UNION ALL "
			"SELECT PatientsT.PersonID, 3 AS IndexNumber, "
			"DiagCodes9.ID AS DiagICD9CodeID, DiagCodes9.CodeNumber AS DiagICD9Code, DiagCodes9.CodeDesc AS DiagICD9Desc, "
			"DiagCodes10.ID AS DiagICD10CodeID, DiagCodes10.CodeNumber AS DiagICD10Code, DiagCodes10.CodeDesc AS DiagICD10Desc "
			"FROM PatientsT "
			"LEFT JOIN DiagCodes DiagCodes9 ON PatientsT.DefaultDiagID3 = DiagCodes9.ID "
			"LEFT JOIN DiagCodes DiagCodes10 ON PatientsT.DefaultICD10DiagID3 = DiagCodes10.ID "
			""
			"UNION ALL "
			"SELECT PatientsT.PersonID, 4 AS IndexNumber, "
			"DiagCodes9.ID AS DiagICD9CodeID, DiagCodes9.CodeNumber AS DiagICD9Code, DiagCodes9.CodeDesc AS DiagICD9Desc, "
			"DiagCodes10.ID AS DiagICD10CodeID, DiagCodes10.CodeNumber AS DiagICD10Code, DiagCodes10.CodeDesc AS DiagICD10Desc "
			"FROM PatientsT "
			"LEFT JOIN DiagCodes DiagCodes9 ON PatientsT.DefaultDiagID4 = DiagCodes9.ID "
			"LEFT JOIN DiagCodes DiagCodes10 ON PatientsT.DefaultICD10DiagID4 = DiagCodes10.ID "
			") AS DefaultDiagsQ");

		m_PatientTypeCombo = BindNxDataListCtrl(IDC_PATIENT_TYPE_COMBO);
		m_ReferringPhyCombo = BindNxDataListCtrl(IDC_REFERRING_PHYSICIAN_COMBO);
		m_PCPCombo = BindNxDataListCtrl(IDC_PCP_COMBO);
		m_DefaultLocationCombo = BindNxDataListCtrl(IDC_DEFAULT_LOCATION_COMBO);
		// (a.walling 2007-11-09 17:09) - PLID 28059 - Bad bind; use BindNxDataList2Ctrl
		m_pRelatedPatientList = BindNxDataList2Ctrl(IDC_RELATION, false);
		m_pRelatedPatientList->PutSnapToVertically(VARIANT_FALSE);

		// (a.walling 2010-06-30 17:43) - PLID 18081 - Warning categories
		m_pWarningCategoryList = BindNxDataList2Ctrl(IDC_LIST_WARNING_CATEGORIES, false);
		RequeryListWarningCategories();
		
		// (j.jones 2008-03-20 16:05) - PLID 29334 - added serialized product list
		m_SerializedProductList = BindNxDataList2Ctrl(IDC_SERIALIZED_PRODUCT_LIST, false);
		
		if(m_bHasAdvInventoryLicense) {
			//set the from clause in code
			SetSerializedProductListFromClause();
		}

		// (a.walling 2006-11-14 12:34) - PLID 22715
		m_nRelatedPatient = -1;
		//m_pRelatedPatientList->Requery(); // (a.walling 2011-08-01 17:34) - PLID 44788 - Don't load until dropping down
		/*
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pRelatedPatientList->GetNewRow();
		pNewRow->PutValue(0, long(-1));
		pNewRow->PutValue(1, "<No Relation>");
		m_pRelatedPatientList->AddRowSorted(pNewRow, NULL);
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_pRelatedPatientList->TrySetSelByColumn_Deprecated(0, long(-1));
		*/
		
		if(GetRemotePropertyInt("DisableG2RefPat", 0, 0, "<None>", true) == 0) {
			// (a.walling 2011-08-01 17:34) - PLID 44788 - Don't load until dropping down
			m_referringPatientCombo = BindNxDataListCtrl(IDC_REFERRING_PATIENT_COMBO, false);
			m_referringPatientCombo->PutSnapToVertically(VARIANT_FALSE);
		}
		else {
			GetDlgItem(IDC_REFERRING_PATIENT_COMBO)->EnableWindow(FALSE);
			m_referringPatientCombo = NULL;
		}
		m_referralSourceList = BindNxDataListCtrl(IDC_REFERRAL_LIST, false);

		// (j.gruber 2011-09-22 16:24) - PLID 45357 - show affiliate unless we are repro, then hide affiliate
		// (j.gruber 2012-10-16 17:49) - PLID 47289 - hide the type also
		if(IsReproductive()) {
			GetDlgItem(IDC_DONOR_PATIENT_COMBO)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_DONOR_LABEL)->ShowWindow(SW_SHOW);		
			m_donorCombo = BindNxDataListCtrl(IDC_DONOR_PATIENT_COMBO,true);
			
			GetDlgItem(IDC_AFFILIATE_PHYS_LIST)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_AFFILIATE_PHYS_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_AFF_PHYS_TYPE)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_GOTO_AFFILIATE)->ShowWindow(SW_HIDE);
			
			m_pAffiliateList = BindNxDataList2Ctrl(IDC_AFFILIATE_PHYS_LIST, false);
			m_pAffiliateTypeList = BindNxDataList2Ctrl(IDC_AFF_PHYS_TYPE, false);
			
		}
		else {
			GetDlgItem(IDC_DONOR_PATIENT_COMBO)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DONOR_LABEL)->ShowWindow(SW_HIDE);
			m_donorCombo = BindNxDataListCtrl(IDC_DONOR_PATIENT_COMBO,false);			

			GetDlgItem(IDC_AFFILIATE_PHYS_LIST)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_AFFILIATE_PHYS_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_AFF_PHYS_TYPE)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_GOTO_AFFILIATE)->ShowWindow(SW_SHOW);			

			m_pAffiliateList = BindNxDataList2Ctrl(IDC_AFFILIATE_PHYS_LIST, true);

			m_pAffiliateTypeList = BindNxDataList2Ctrl(IDC_AFF_PHYS_TYPE, false);

			NXDATALIST2Lib::IRowSettingsPtr pAffRow = m_pAffiliateTypeList->GetNewRow();
			if (pAffRow) {
				pAffRow->PutValue(aptcID, (long)afptPreOp);
				pAffRow->PutValue(aptcName, _variant_t(GetAffiliateTypeDescription(afptPreOp)));
				m_pAffiliateTypeList->AddRowAtEnd(pAffRow, NULL);
			}

			pAffRow = m_pAffiliateTypeList->GetNewRow();
			if (pAffRow) {
				pAffRow->PutValue(aptcID, (long)afptPostOp);
				pAffRow->PutValue(aptcName, _variant_t(GetAffiliateTypeDescription(afptPostOp)));
				m_pAffiliateTypeList->AddRowAtEnd(pAffRow, NULL);
			}

			pAffRow = m_pAffiliateTypeList->GetNewRow();
			if (pAffRow) {
				pAffRow->PutValue(aptcID, (long)afptPrePostOp);
				pAffRow->PutValue(aptcName, _variant_t(GetAffiliateTypeDescription(afptPrePostOp)));
				m_pAffiliateTypeList->AddRowAtEnd(pAffRow, NULL);
			}

			pAffRow = m_pAffiliateTypeList->GetNewRow();
			if (pAffRow) {
				pAffRow->PutValue(aptcID, (long)-1);
				pAffRow->PutValue(aptcName, _variant_t("<None>"));
				m_pAffiliateTypeList->AddRowBefore(pAffRow, m_pAffiliateTypeList->GetFirstRow());
			}
		}
		m_raceCombo = BindNxDataListCtrl(IDC_CODE_RACE,true);	
		// (j.jones 2009-10-19 10:00) - PLID 34327 - added ethnicity combo
		m_ethnicityCombo = BindNxDataListCtrl(IDC_ETHNICITY_COMBO,true);

		// (a.walling 2009-05-25 10:28) - PLID 27859 - Language combo
		m_languageCombo = BindNxDataList2Ctrl(IDC_LANGUAGE_COMBO,false);
		PopulateLanguageCombo();

		// (d.thompson 2009-03-20) - PLID 28726 - Accumulated reward points should be hidden if not NexSpa
		if(!g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)) {
			GetDlgItem(IDC_ACCUM_REWARD_PTS_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ACCUM_REWARD_PTS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_WARNING_CHECK2)->ShowWindow(SW_HIDE); // (j.luckoski 2013-03-04 11:47) - PLID 33548 - Prompt reward balance
			m_btnEditRewardPoints.ShowWindow(SW_HIDE); // (z.manning 2010-07-20 12:48) - PLID 30127
			m_btnEditRewardPoints.EnableWindow(FALSE);
		} else if(!GetRemotePropertyInt("DisplayRewardsWarning",0,0, GetCurrentUserName(),true) == 1) {
			GetDlgItem(IDC_WARNING_CHECK2)->ShowWindow(SW_HIDE); // (j.luckoski 2013-03-05 11:55) - PLID 33548 - if pref is off hide
		}

		// (j.jones 2014-02-18 17:00) - PLID 60719 - configure the diagnosis list
		// to reflect the user's search style
		UpdateDiagnosisListColumnSizes();

		LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__OnInitDialog, 1, 2000, "");
		LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__OnInitDialog, 2);

		NXDATALISTLib::IRowSettingsPtr pRow;

		// (j.jones 2011-12-01 14:15) - PLID 46771 - these now use the enums
		pRow = m_ReferringPhyCombo->GetRow(-1);
		pRow->PutValue(rpcID, g_cvarNull);
		pRow->PutValue(rpcName, "<No Provider Selected>");
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
		m_ReferringPhyCombo->InsertRow(pRow,0);

		pRow = m_PCPCombo->GetRow(-1);
		pRow->PutValue(pcpcID, g_cvarNull);
		pRow->PutValue(pcpcName, "<No Provider Selected>");
		pRow->PutValue(pcpcNpi, "");
		pRow->PutValue(pcpcRefPhysID, "");
		pRow->PutValue(pcpcUpin, "");
		pRow->PutValue(pcpcAddress1, "");
		pRow->PutValue(pcpcAddress2, "");
		pRow->PutValue(pcpcCity, "");
		pRow->PutValue(pcpcState, "");
		pRow->PutValue(pcpcZip, "");
		m_PCPCombo->InsertRow(pRow, 0);


		// (a.walling 2011-08-01 17:34) - PLID 44788 - Don't load until dropping down
		/*
		if(m_referringPatientCombo != NULL) {
			pRow = m_referringPatientCombo->GetRow(-1);
			var.vt = VT_NULL;
			pRow->PutValue(0,var);
			pRow->PutValue(1,"<No Patient Selected>");
			m_referringPatientCombo->InsertRow(pRow,0);

			pRow = m_referringPatientCombo->GetRow(-1);
			var = (long)-2;
			pRow->PutValue(0,var);
			pRow->PutValue(1,"<Unspecified>");
			m_referringPatientCombo->InsertRow(pRow,1);
		}
		*/

		pRow = m_donorCombo->GetRow(-1);
		pRow->PutValue(0,g_cvarNull);
		pRow->PutValue(1,"<No Donor Selected>");
		m_donorCombo->InsertRow(pRow,0);

		pRow = m_DefaultLocationCombo->GetRow(-1);
		pRow->PutValue(0,g_cvarNull);
		pRow->PutValue(1,"<No Location Selected>");
		m_DefaultLocationCombo->InsertRow(pRow,0);

// (b.spivey, May 14, 2013) - PLID 56872 - changed to a label set up here. 
		m_nxstaticMultiRaceLabel.SetType(dtsHyperlink);
		m_nxstaticMultiRaceLabel.SetSingleLine();

		// (j.jones 2009-10-19 10:08) - PLID 34327 - added ethnicity combo
		pRow = m_ethnicityCombo->GetRow(-1);
		pRow->PutValue(ethnicityID, g_cvarNull);
		pRow->PutValue(ethnicityName, _bstr_t(l_strNoEthnicityText));
		m_ethnicityCombo->InsertRow(pRow,0);

		//TES 11/13/2013 - PLID 59483 - Added Occupation and Industry Census Code lists
		m_pOccupationCodeList = BindNxDataList2Ctrl(IDC_OCCUPATION_CENSUS_CODE);
		m_pIndustryCodeList = BindNxDataList2Ctrl(IDC_INDUSTRY_CENSUS_CODE);
	/* MULTI
		m_referralTree.OnInitialUpdate();
	*/

		LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__OnInitDialog, 2, 2000, "");
		LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__OnInitDialog, 3);

		// (a.walling 2010-10-12 14:54) - PLID 40908 - Don't load the id until UpdateView is called
		/*
		m_id = GetActivePatientID();
		m_strPatientName = GetExistingPatientName(m_id);
		*/

		
		// (a.walling 2010-10-12 14:54) - PLID 40908 - Dead code
		//m_lastID = 0;

		SecureControls();

		LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__OnInitDialog, 3, 2000, "");

		m_bShowWarningMessage = true;

	} NxCatchAll(__FUNCTION__)

	return TRUE;
}

void CGeneral2Dlg::StoreDetails ()
{
	//We can't count on GetFocus() for NxTimes.
	//DRT 10/6/2008 - PLID 31597 - We cannot compare DATE against COleDateTime anymore if one can be invalid
	COleDateTime dtCurrent(m_nxtAccidentDate->GetDateTime());
	if(m_nxtAccidentDate->GetStatus() == 3) {
		//the accident date is empty, so our datetime needs to be set to invalid, not valid and 1/1/1900.
		dtCurrent.SetStatus(COleDateTime::invalid);
	}
	//invalid != invalid, so we need to check specifically
	if(dtCurrent.GetStatus() != COleDateTime::invalid || m_dtAccident.GetStatus() != COleDateTime::invalid) {
		//Now at least 1 of them is valid
		if(dtCurrent != m_dtAccident) {
			m_changed = true;
			Save(IDC_ACCIDENT_DATE);
		}
	}
	if (GetFocus())
		Save(GetFocus()->GetDlgCtrlID());
}

void CGeneral2Dlg::RecallDetails(void)
{
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CGeneral2Dlg::Hotkey(int key)
{
	switch(key)
	{	case VK_ESCAPE:
			UpdateView();
			return 0;
	}

	//unhandled
	return 1;
}

void CGeneral2Dlg::Save(int nID)
{
	CString field;
	CString value;

	int item = -1;
	CString strOld = "";

	// (c.haag 2006-04-12 16:34) - PLID 20096 - Don't save anything if nothing was changed
	if (!m_changed)
		return;

	try {
	switch (nID)
	{	case IDC_OCCUPATION: {
			field = "Occupation";
			//for auditing
			_RecordsetPtr rs = CreateRecordset("SELECT Occupation FROM PatientsT WHERE PersonID = %li", m_id);
			item = aeiPatientOccupation;
			
			if(!rs->eof && rs->Fields->Item["Occupation"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["Occupation"]->Value.bstrVal);
			}
			// (j.camacho 2016-02-11) PLID 68229 - grab value and update so we can send hl7 update
			value= m_nxeditOccupation.GetText();
			if (strOld != value)
			{
				ExecuteSql("UPDATE PatientsT SET Occupation='%s' WHERE Personid = %li", value, m_id);
				value = "";
				// (j.camacho 2016-02-11) PLID 68229 - Update HL7
				UpdateExistingPatientInHL7(m_id);

			}

			break;
		case IDC_MANAGER_FNAME: {
			field = "EmployerFirst";
			//for auditing
			_RecordsetPtr rs = CreateRecordset("SELECT EmployerFirst FROM PatientsT WHERE PersonID = %li", m_id);
			item = aeiPatientEmployerFirst;
			if(!rs->eof && rs->Fields->Item["EmployerFirst"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["EmployerFirst"]->Value.bstrVal);
			}
			break;
		case IDC_MANAGER_MNAME: {
			field = "EmployerMiddle";
			//for auditing
			_RecordsetPtr rs = CreateRecordset("SELECT EmployerMiddle FROM PatientsT WHERE PersonID = %li", m_id);
			item = aeiPatientEmployerMiddle;
			if(!rs->eof && rs->Fields->Item["EmployerMiddle"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["EmployerMiddle"]->Value.bstrVal);
			}
			break;
		case IDC_MANAGER_LNAME: {
			field = "EmployerLast";
			//for auditing
			_RecordsetPtr rs = CreateRecordset("SELECT EmployerLast FROM PatientsT WHERE PersonID = %li", m_id);
			item = aeiPatientEmployerLast;
			if(!rs->eof && rs->Fields->Item["EmployerLast"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["EmployerLast"]->Value.bstrVal);
			}
			break;
		case IDC_EMP_ADDRESS1: {
			field = "EmployerAddress1";
			//for auditing
			_RecordsetPtr rs = CreateRecordset("SELECT EmployerAddress1 FROM PatientsT WHERE PersonID = %li", m_id);
			item = aeiPatientEmployerAddress1;
			if(!rs->eof && rs->Fields->Item["EmployerAddress1"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["EmployerAddress1"]->Value.bstrVal);
			}
			break;
		case IDC_EMP_ADDRESS2: {
			field = "EmployerAddress2";
			//for auditing
			_RecordsetPtr rs = CreateRecordset("SELECT EmployerAddress2 FROM PatientsT WHERE PersonID = %li", m_id);
			item = aeiPatientEmployerAddress2;
			if(!rs->eof && rs->Fields->Item["EmployerAddress2"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["EmployerAddress2"]->Value.bstrVal);
			}
			break;
		case IDC_EMP_CITY: {
			// (j.gruber 2009-10-07 16:41) - PLID 35825 - updated for city lookup
			if (m_bLookupByCity) {					
				CString zip, 
						state,
						tempZip,
						tempState;
				GetDlgItemText(IDC_EMP_CITY, value);
				GetDlgItemText(IDC_EMP_ZIP, tempZip);
				GetDlgItemText(IDC_EMP_STATE, tempState);
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
						SetDlgItemText(IDC_EMP_ZIP, zip);
					else 
						zip = tempZip;
					if(tempState == "")
						SetDlgItemText(IDC_EMP_STATE, state);
					else
						state = tempState;

					//for auditing
					CString strOldZip, strOldCity, strOldState;
					_RecordsetPtr rs = CreateRecordset("SELECT EmployerZip, EmployerCity, EmployerState FROM PatientsT WHERE PersonID = %li", m_id);
					if(!rs->eof) {
						strOldZip = AdoFldString(rs, "EmployerZip","");
						strOldCity = AdoFldString(rs, "EmployerCity","");
						strOldState = AdoFldString(rs, "EmployerState","");
					}

					ExecuteSql("UPDATE PatientsT SET EmployerZip = '%s', EmployerCity = '%s', EmployerState = '%s' WHERE PersonID = %li",_Q(zip),value,state,m_id);

					long nAuditID = BeginNewAuditEvent();
					if(value != strOldCity)
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientEmployerCity, m_id, strOldCity, value, aepMedium, aetChanged);
					if(strOldZip != zip)
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientEmployerZip, m_id, strOldZip, zip, aepMedium, aetChanged);
					if(strOldState != state)
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientEmployerState, m_id, strOldState, state, aepMedium, aetChanged);
					
					UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
				}
				else {
					//for auditing
					_RecordsetPtr rs = CreateRecordset("SELECT EmployerCity FROM PatientsT WHERE PersonID = %li", m_id);
					item = aeiPatientEmployerCity;
					if(!rs->eof && rs->Fields->Item["EmployerCity"]->Value.vt == VT_BSTR)
						strOld = CString(rs->Fields->Item["EmployerCity"]->Value.bstrVal);

					ExecuteSql("UPDATE PatientsT SET EmployerCity = '%s' WHERE PersonID = %li",_Q(value),m_id);

					if(strOld != value) {
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(m_id, m_strPatientName, nAuditID, item, m_id, strOld, value, aepMedium, aetChanged);
						UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
					}
				}
				m_changed = false;
				return;
			}
			else {
				field = "EmployerCity";
				//for auditing
				_RecordsetPtr rs = CreateRecordset("SELECT EmployerCity FROM PatientsT WHERE PersonID = %li", m_id);
				item = aeiPatientEmployerCity;
				if(!rs->eof && rs->Fields->Item["EmployerCity"]->Value.vt == VT_BSTR)
					strOld = CString(rs->Fields->Item["EmployerCity"]->Value.bstrVal);
				}
			}
			break;
		case IDC_EMP_STATE: {
			field = "EmployerState";
			//for auditing
			_RecordsetPtr rs = CreateRecordset("SELECT EmployerState FROM PatientsT WHERE PersonID = %li", m_id);
			item = aeiPatientEmployerState;
			if(!rs->eof && rs->Fields->Item["EmployerState"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["EmployerState"]->Value.bstrVal);
			}
			break;
		case IDC_ACCIDENT_DATE:
			if(!m_bSavingAccidentDate) {
				m_bSavingAccidentDate = true;

				CString strOldDate, strNewDate;
				//(e.lally 2007-01-31) PLID 24518 - Renaming this local variable to be more clear.
				_variant_t varOldDate;

				_RecordsetPtr rs = CreateRecordset("SELECT DefaultInjuryDate FROM PatientsT WHERE PersonID = %li",m_id);
				if(!rs->eof) {
					varOldDate = rs->Fields->Item["DefaultInjuryDate"]->Value;
					if(varOldDate.vt == VT_DATE)
						strOldDate = FormatDateTimeForInterface(varOldDate.date, dtoDate);
					else
						strOldDate = "";
				}
				rs->Close();

				if(m_nxtAccidentDate->GetStatus() != 3) {
					//(e.lally 2007-01-31) PLID 24518 - Renaming this local variable to be more clear.
					COleDateTime dtNewDate, dtNow, dttemp;
					dtNow = COleDateTime::GetCurrentTime();
					COleDateTimeSpan dtSpan;
					dtSpan.SetDateTimeSpan(1,0,0,0);
					dtNow += dtSpan;
					dtNewDate = m_nxtAccidentDate->GetDateTime();
					//TES 11/8/2007 - PLID 28033 - ParseDateTime() returns a bool, not, as whoever wrote
					// this seems to have believed, a date value.  This mostly worked, however, because
					// it was comparing dt to "true", aka 1, and a dt value >= 1 is also >= 1/1/1900.  
					// I fixed the code to do what the original creator seems to have intended,  and also resolve 
					// a compile warning in VS 2008.
					if(m_nxtAccidentDate->GetStatus() == 1 && dtNewDate < dtNow && dtNewDate >= COleDateTime(1800,1,1,0,0,0)/*dttemp.ParseDateTime("01/01/1800")*/) {
						//(e.lally 2007-01-31) PLID 24518 - Switching the update statement to be after we check if the date has changed
						strNewDate = FormatDateTimeForInterface(dtNewDate,dtoDate);
						if(strOldDate != strNewDate) {
							ExecuteSql("UPDATE PatientsT SET DefaultInjuryDate = '%s' WHERE PersonID = %li",_Q(FormatDateTimeForSql(dtNewDate)),m_id);
							//(e.lally 2007-01-31) PLID 24518 - set our member variable
							m_dtAccident = dtNewDate;
							long nAuditID = -1;
							nAuditID = BeginNewAuditEvent();
							if(nAuditID != -1) {
								AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientDefaultInjuryDate, m_id, strOldDate, strNewDate, aepMedium, aetChanged);
								UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
							}
						}
					}
					else
					{
						if (m_nxtAccidentDate->GetStatus() == 1 && dtNewDate > dtNow)  {
							AfxMessageBox("You have entered a Date of Current Illness/Accident in the future.  It will be reset.");
						}
						else{
							AfxMessageBox("You have entered an invalid Date of Current Illness/Accident.  It will be reset.");
						}				
						
						if(varOldDate.vt == VT_DATE) {
							m_dtAccident = VarDateTime(varOldDate);
							m_nxtAccidentDate->SetDateTime(m_dtAccident);								
						}
						else {
							m_nxtAccidentDate->Clear();
							//(e.lally 2007-01-31) PLID 24518 - Cleanup: This SetDateTime does not actually reset the date.
							//m_dtAccident.SetDateTime(0,0,0,0,0,0);
							m_dtAccident.m_dt = 0;
							m_dtAccident.SetStatus(COleDateTime::invalid);
						}
					}
				}
				else {
					ExecuteSql("UPDATE PatientsT SET DefaultInjuryDate = NULL WHERE PersonID = %li",m_id);
					if(strOldDate != "") {
						long nAuditID = -1;
						nAuditID = BeginNewAuditEvent();
						if(nAuditID != -1) {
							AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientDefaultInjuryDate, m_id, strOldDate, "", aepMedium, aetChanged);
							UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
						}
					}
					//(e.lally 2007-01-31) PLID 24518 - Force a clear and reset our member variable.
					m_nxtAccidentDate->Clear();
					m_dtAccident.m_dt = 0;
					m_dtAccident.SetStatus(COleDateTime::invalid);
				}

				m_bSavingAccidentDate = false;
				// (a.walling 2007-09-13 09:48) - PLID 27376 - We forgot to reset our changed flag
				m_changed = false;
			}

			return;
			break;
		case IDC_WARNING:
		{
			if (((CNxEdit*)GetDlgItem(IDC_WARNING))->GetStyle() & ES_READONLY) {
				// (a.walling 2007-09-13 09:48) - PLID 27376 - We forgot to reset our changed flag
				m_changed = false;
				return;
			}

			if (UserPermission(ChangeWarning)) {
				//for auditing
				CString strOld;
				_RecordsetPtr prsAudit = CreateRecordset("SELECT WarningMessage FROM PersonT WHERE ID = %li", m_id);
				if(!prsAudit->eof)
					strOld = AdoFldString(prsAudit, "WarningMessage", "");
				//

				//check to see if the warning is blank
				long nCheck = m_WarningCheck.GetCheck();

				if (nCheck != 0) {

					if (! m_bShowWarningMessage) {			
						// (a.walling 2007-09-13 09:48) - PLID 27376 - We forgot to reset our changed flag
						m_changed = false;
						return;
					}

					m_bShowWarningMessage = false;
					//they are checking the box, so check to see if there is anything in the warning box
					CString strWarningText;
					GetDlgItemText(IDC_WARNING, strWarningText);

					strWarningText.TrimRight();
					strWarningText.TrimLeft();

					//if they are clicking on the Warning box, they are unchecking it, so don't warn
					BOOL bWarn = TRUE;
					if(GetFocus() != NULL && GetFocus()->GetDlgCtrlID() == IDC_WARNING_CHECK && !IsKeyDown(VK_TAB)) {
						//JMJ 2/19/2004 - right now what this will do is check to see if the warning check has focus
						//at this point. By checking the VK_TAB, we'll know if they tabbed to it, which doesn't
						//necessarily mean they are planning to uncheck it so we must warn them.
						//So this will only NOT warn if they are left-clicking on the checkbox, which 99.9% of the
						//time they are unchecking it, unless they are a developer specifically trying to break it
						//by holding down the left mouse button, dragging off the checkbox, and letting go.
						//I'm willing to let that ridiculous situation as is. :)
						bWarn = FALSE;
					}

					if (strWarningText.IsEmpty() && bWarn){

						//make sure that they want to do this
						if (IDYES != AfxMessageBox("The warning message is empty. Are you sure you want to display a blank message?", MB_YESNO)) {
							//They aren't sure, so we'll uncheck the box.
							m_WarningCheck.SetCheck(0);
							SetWarningBox();
							SetExpireBox();
						}
					
					}
					m_bShowWarningMessage = true;

				}
				GetDlgItemText(nID, value);
				if(value.GetLength()>1024) {
					value = value.Left(1024);
					AfxMessageBox("The text you entered was greater than the maximum allowed amount (1024) and has been shortened.\n"
						"Please double-check the new text and edit it as needed.");
					SetDlgItemText(nID, value);
				}
				COleDateTime dtNow = COleDateTime::GetCurrentTime();
				if (GetPropertyInt("G2ShowWarningStats", 1))
				{
					CString strOldWarning, str;
					_RecordsetPtr prs = CreateRecordset("SELECT WarningMessage FROM PersonT WHERE ID = %d", m_id);
					strOldWarning = AdoFldString(prs, "WarningMessage", "");
					if (value != strOldWarning)
					{
						ExecuteSql("UPDATE PersonT SET WarningDate = '%s', WarningUserID = %d WHERE ID = %li",FormatDateTimeForSql(dtNow,dtoDate),GetCurrentUserID(),m_id);
						str.Format("%s (%s)", GetCurrentUserName(), FormatDateTimeForInterface(dtNow, DTF_STRIP_SECONDS, dtoDate));
						SetDlgItemText(IDC_EDIT_WARNING_USER, str);
					}
				}
				ExecuteSql("UPDATE PersonT SET WarningMessage = '%s' WHERE ID = %li",_Q(value),m_id);

				if(strOld != value) {
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientG2WarningText, m_id, strOld, value, aepMedium, aetChanged);
				}
			}
			else {
				_RecordsetPtr rs1 = CreateRecordset("SELECT WarningMessage FROM PersonT WHERE ID = %li",m_id);
				SetDlgItemText(IDC_WARNING, CString(rs1->Fields->Item["WarningMessage"]->Value.bstrVal));
				rs1->Close();
			}

			// (a.walling 2007-09-13 09:48) - PLID 27376 - We forgot to reset our changed flag
			m_changed = false;
			return;
		}
			break;

			case IDC_EXPIRE_DATE:
			case IDC_EXPIRE_DATE_PICKER:
			{
				//for auditing
				CString strOldDate, strNewDate;
				_RecordsetPtr prsAudit = CreateRecordset("SELECT WarningExpireDate FROM PersonT WHERE ID = %li", m_id);
				if(!prsAudit->eof) {
					COleDateTime dtInvalid;		dtInvalid.SetStatus(COleDateTime::invalid);

					COleDateTime dt = AdoFldDateTime(prsAudit, "WarningExpireDate", dt);
					if(dt.GetStatus() == COleDateTime::valid)
						strOldDate = FormatDateTimeForInterface(dt, NULL, dtoDate);
				}

				strNewDate = FormatDateTimeForInterface(VarDateTime(m_expireDate.GetValue()), NULL, dtoDate);
				//


				if (IsDlgButtonChecked(IDC_EXPIRE_DATE)) {
					//get the value of the date picker
					CString strDate = FormatDateTimeForSql(VarDateTime(m_expireDate.GetValue()), dtoDate);
					ExecuteSql("UPDATE PersonT SET WarningUseExpireDate = 1, WarningExpireDate = '%s' WHERE ID = %li", strDate, m_id);
				}
				else {
					GetDlgItem(IDC_EXPIRE_DATE_PICKER)->EnableWindow(FALSE);
					ExecuteSql("UPDATE PersonT SET WarningUseExpireDate = 0, WarningExpireDate = NULL WHERE ID = %li", m_id);
				}

				//DRT 6/30/2005 - PLID 16654 - Auditing
				if(nID == IDC_EXPIRE_DATE) {
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientG2WarnExpire, m_id, IsDlgButtonChecked(IDC_EXPIRE_DATE) ? "No" : "Yes", 
						IsDlgButtonChecked(IDC_EXPIRE_DATE) ? "Yes" : "No", aepMedium, aetChanged);
				}
				if(nID == IDC_EXPIRE_DATE_PICKER && strOldDate != strNewDate) {
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientG2WarnExpireDate, m_id, strOldDate, strNewDate, aepMedium, aetChanged);
				}

				// (a.walling 2007-09-13 09:48) - PLID 27376 - We forgot to reset our changed flag
				m_changed = false;

				return;
			}
			break;
/* MULTI
		case IDC_REFERRAL:
		{	CString sql;
			long id;
			GetDlgItemText (nID, value);
			sql.Format ("SELECT PersonID AS ID FROM ReferralSourceT WHERE Name = '%s'", _Q(value));
			try
			{	_RecordsetPtr rs1 = CreateRecordsetStd(sql);
				if(!rs1->eof)
				{
					id = rs1->Fields->Item["ID"]->Value.lVal;
					ExecuteSql("UPDATE PatientsT SET ReferralID = %li WHERE PersonID = %li",id,m_id);
				}
				else{
					ExecuteSql("UPDATE PatientsT SET ReferralID = NULL WHERE PersonID = %li",m_id);
				}
				rs1->Close();
			}NxCatchAll("Error saving referral source: ");
			m_changed = false;
			return;
		}
*/
		case IDC_COMPANY:
		{
			GetDlgItemText (nID, value);
			value.TrimRight("");

			//make sure that the name has actually changed.  I don't know why this isn't done at the top
			//and used for all items, but I don't want to break anything, so I'll just use it here.
			// (c.haag 2006-04-12 16:34) - PLID 20096 - Now it's official
			//if(!m_changed)
			//	return;

			//DRT 10/28/2003 - PLID 9914 - try to auto-find this company and fill in its details
			try {
				long nFillCompany = GetRemotePropertyInt("FillGeneral2Company", 1, 0, GetCurrentUserName(), true);
				if(!value.IsEmpty() && nFillCompany > 0) {
					//we've got 1 company info.  If any of these fields are already filled, don't touch them
					CString strAddr1, strAddr2, strCity, strState, strZip;	//the ones we read from the data
					CString curAddr1, curAddr2, curCity, curState, curZip;	//the info currently filled into the fields
					bool bSave = false;		//did we reach a criteria that lets us save?

					//run a query to find the data
					_RecordsetPtr prs = CreateRecordset("SELECT Company, EmployerAddress1, EmployerAddress2, EmployerCity, EmployerState, EmployerZip "
						"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
						"WHERE Company = '%s' "
						"GROUP BY Company, EmployerAddress1, EmployerAddress2, EmployerCity, EmployerState, EmployerZip ", _Q(value));

					long nRecords = prs->GetRecordCount();
					if(nRecords == 0) {
					}
					else if(nRecords > 1) {
						//if they have this preference set, prompt them, otherwise we don't fill at all
						long nFillMultiple = GetRemotePropertyInt("FillGeneral2CompanyMultiplePrompt", 1, 0, GetCurrentUserName(), true);
						if(nFillMultiple > 0) {
							CCopyEmployerDlg dlg(this);
							// (e.frazier 2016-04-28 12:22) - PLID 67674 - Editing a patient's company name that contains a single quote can throw an exception without using _Q(value)
							dlg.FilterOnCompany(_Q(value));
							if(dlg.DoModal() == IDOK) {
								//fill in our str... fields based on the selection
								strAddr1 = dlg.m_addr1;
								strAddr2 = dlg.m_addr2;
								strCity = dlg.m_city;
								strState = dlg.m_state;
								strZip = dlg.m_zip;

								bSave = true;
							}
						}
					}
					else {
						//fill in our new fields
						strAddr1 = AdoFldString(prs, "EmployerAddress1", "");
						strAddr2 = AdoFldString(prs, "EmployerAddress2", "");
						strCity = AdoFldString(prs, "EmployerCity", "");
						strState = AdoFldString(prs, "EmployerState", "");
						strZip = AdoFldString(prs, "EmployerZip", "");

						bSave = true;
					}

					if(bSave) {
						//fill in our current fields
						GetDlgItemText(IDC_EMP_ADDRESS1, curAddr1);
						GetDlgItemText(IDC_EMP_ADDRESS2, curAddr2);
						GetDlgItemText(IDC_EMP_CITY, curCity);
						GetDlgItemText(IDC_EMP_STATE, curState);
						GetDlgItemText(IDC_EMP_ZIP, curZip);

						//We follow the same logic as zipcodes here - If any field is filled in, leave it as is, but otherwise 
						//we copy in whatever we pulled out.
						if(curAddr1.IsEmpty())
							SetDlgItemText(IDC_EMP_ADDRESS1, strAddr1);
						if(curAddr2.IsEmpty())
							SetDlgItemText(IDC_EMP_ADDRESS2, strAddr2);
						if(curCity.IsEmpty())
							SetDlgItemText(IDC_EMP_CITY, strCity);
						if(curState.IsEmpty())
							SetDlgItemText(IDC_EMP_STATE, strState);
						if(curZip.IsEmpty())
							SetDlgItemText(IDC_EMP_ZIP, strZip);

						//force those fields to save
						// (a.walling 2007-09-13 09:46) - PLID 27376 - Only the first field actually saves,
						// since m_changed is true now, but is set to false at the end of Save(). So
						// we were effectively only saving and auditing Address1!!
						if(curAddr1.IsEmpty())
							Save(IDC_EMP_ADDRESS1);
						m_changed = true;
						if(curAddr2.IsEmpty())
							Save(IDC_EMP_ADDRESS2);
						m_changed = true;
						if(curCity.IsEmpty())
							Save(IDC_EMP_CITY);
						m_changed = true;
						if(curState.IsEmpty())
							Save(IDC_EMP_STATE);
						m_changed = true;
						if(curZip.IsEmpty())
							Save(IDC_EMP_ZIP);
					}
				}
				//even if we have a failure in the code, we will catch it here and still save the company
			} NxCatchAll("Error determining company address.");

			//for auditing
			_RecordsetPtr rs = CreateRecordset("SELECT Company FROM PersonT WHERE ID = %li", m_id);
			item = aeiPatientCompany;
			if(!rs->eof && rs->Fields->Item["Company"]->Value.vt == VT_BSTR)
				strOld = CString(rs->Fields->Item["Company"]->Value.bstrVal);

			//do the final save for the company
			ExecuteSql("UPDATE PersonT SET Company = '%s' WHERE ID = %li",_Q(value),m_id);

			//m.hancock - 5/8/2006 - PLID 20462 - Update the patient toolbar to reflect the new company
			//TES 1/6/2010 - PLID 36761 - New function for updating the datalist.
			GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(GetMainFrame()->m_patToolBar.GetCompanyColumn(),_bstr_t(value));
			
			if(value != strOld) {
				long nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1) {
					AuditEvent(m_id, m_strPatientName, nAuditID, item, m_id, strOld, value, aepMedium, aetChanged);
					UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
				}

				UpdatePalm(FALSE);
			}

			// (a.walling 2007-09-13 09:48) - PLID 27376 - We forgot to reset the m_changed flag, too.
			m_changed = false;
			
			return;
		}	break;
		case IDC_EMP_ZIP:
		{	
			// (s.tullis 2013-10-28 17:13) - PLID 45031 - If 9-digit zipcode match fails compair it with the 5-digit zipcode.
			// (j.gruber 2009-10-07 16:34) - PLID 35825 - update for city lookup
			if (!m_bLookupByCity) {
				CString city, 
						state,
						strTempZip,
						tempCity,
						tempState;
				GetDlgItemText(IDC_EMP_ZIP, value);
				GetDlgItemText(IDC_EMP_CITY, tempCity);
				GetDlgItemText(IDC_EMP_STATE, tempState);
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
					if( city == "" && state == ""){
						strTempZip = value.Left(5);// Get the 5 digit zip code
						GetZipInfo(strTempZip, &city, &state);
						// (b.savon 2014-04-03 13:02) - PLID 61644 - If you enter a 9
						//digit zipcode in the locations tab of Administrator, it looks
						//up the city and state based off the 5 digit code, and then 
						//changes the zip code to 5 digits. It should not change the zip code.
					}
					if(tempCity == "") 
						SetDlgItemText(IDC_EMP_CITY, city);
					else 
						city = tempCity;
					if(tempState == "")
						SetDlgItemText(IDC_EMP_STATE, state);
					else
						state = tempState;

					//for auditing
					CString strOldZip, strOldCity, strOldState;
					_RecordsetPtr rs = CreateRecordset("SELECT EmployerZip, EmployerCity, EmployerState FROM PatientsT WHERE PersonID = %li", m_id);
					if(!rs->eof) {
						strOldZip = AdoFldString(rs, "EmployerZip","");
						strOldCity = AdoFldString(rs, "EmployerCity","");
						strOldState = AdoFldString(rs, "EmployerState","");
					}

					ExecuteSql("UPDATE PatientsT SET EmployerZip = '%s', EmployerCity = '%s', EmployerState = '%s' WHERE PersonID = %li",_Q(value),city,state,m_id);

					long nAuditID = BeginNewAuditEvent();

					if(value != strOldZip)
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientEmployerZip, m_id, strOldZip, value, aepMedium, aetChanged);
					if(strOldCity != city)
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientEmployerCity, m_id, strOldCity, city, aepMedium, aetChanged);
					if(strOldState != state)
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientEmployerState, m_id, strOldState, state, aepMedium, aetChanged);
					
					UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
				}
				else {
					//for auditing
					_RecordsetPtr rs = CreateRecordset("SELECT EmployerZip FROM PatientsT WHERE PersonID = %li", m_id);
					item = aeiPatientEmployerZip;
					if(!rs->eof && rs->Fields->Item["EmployerZip"]->Value.vt == VT_BSTR)
						strOld = CString(rs->Fields->Item["EmployerZip"]->Value.bstrVal);

					ExecuteSql("UPDATE PatientsT SET EmployerZip = '%s' WHERE PersonID = %li",_Q(value),m_id);

					if(strOld != value) {
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(m_id, m_strPatientName, nAuditID, item, m_id, strOld, value, aepMedium, aetChanged);
						UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
					}
				}
				m_changed = false;
				return;
			}
			else {
				field = "EmployerZip";
				//for auditing
				_RecordsetPtr rs = CreateRecordset("SELECT EmployerZip FROM PatientsT WHERE PersonID = %li", m_id);
				item = aeiPatientEmployerZip;
				if(!rs->eof && rs->Fields->Item["EmployerZip"]->Value.vt == VT_BSTR)
					strOld = CString(rs->Fields->Item["EmployerZip"]->Value.bstrVal);
				}
			break;
			
		}
	
		default: 
		{
			//We were told to save, but it wasn't a field we check for
			//reset our changed flag
			m_changed = false;
			return;
		}
	}
	GetDlgItemText (nID, value);
	value.TrimRight("");
	if (value != "")
		ExecuteSql("UPDATE PatientsT SET %s = '%s' WHERE PersonID = %li",field,_Q(value),m_id);
	else ExecuteSql("UPDATE PatientsT SET %s = '' WHERE PersonID = %li",field,m_id);

	//for auditing
	if(item != -1 && strOld != value) {
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, item, m_id, strOld, value, aepMedium, aetChanged);
		UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
	}

	}NxCatchAll("Error in Save");

	m_changed = false;
}

void CGeneral2Dlg::RefreshSlowStuff()
{
	NXDATALISTLib::IRowSettingsPtr pRow;
//#ifdef NET_AWARE//use the network code for speed
	if (m_diagChecker.Changed())
	{
		// (j.jones 2014-02-18 17:28) - PLID 60719 - do nothing
		// except force a full refresh so we reload the diag code
		// list, which might now have new descriptions

		// (a.walling 2010-10-12 17:58) - PLID 40908
		m_ForceRefresh = true;
	}

	if (m_refphyChecker.Changed()) {
		LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__RefreshSlowStuff, 3);
		m_ReferringPhyCombo->Requery();
		m_PCPCombo->Requery();
		// (j.gruber 2011-09-28 09:03) - PLID 45657
		m_pAffiliateList->Requery();
		
		// (j.jones 2011-12-01 14:15) - PLID 46771 - these now use the enums
		pRow = m_ReferringPhyCombo->GetRow(-1);
		pRow->PutValue(rpcID, g_cvarNull);
		pRow->PutValue(rpcName, "<No Provider Selected>");
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
		m_ReferringPhyCombo->InsertRow(pRow,0);

		pRow = m_PCPCombo->GetRow(-1);
		pRow->PutValue(pcpcID, g_cvarNull);
		pRow->PutValue(pcpcName, "<No Provider Selected>");
		pRow->PutValue(pcpcNpi, "");
		pRow->PutValue(pcpcRefPhysID, "");
		pRow->PutValue(pcpcUpin, "");
		pRow->PutValue(pcpcAddress1, "");
		pRow->PutValue(pcpcAddress2, "");
		pRow->PutValue(pcpcCity, "");
		pRow->PutValue(pcpcState, "");
		pRow->PutValue(pcpcZip, "");
		m_PCPCombo->InsertRow(pRow, 0);		

		LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__RefreshSlowStuff, 3, 2000, "");
		// (a.walling 2010-10-12 17:58) - PLID 40908
		m_ForceRefresh = true;
	}

/* MULTI
	m_referralTree.Update();
*/
	if (m_patienttypeChecker.Changed()) {
		LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__RefreshSlowStuff, 4);
		m_PatientTypeCombo->Requery();
		LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__RefreshSlowStuff, 4, 2000, "");
		// (a.walling 2010-10-12 17:58) - PLID 40908
		m_ForceRefresh = true;
	}

	if (m_defaultlocChecker.Changed()) {
		LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__RefreshSlowStuff, 5);
		m_DefaultLocationCombo->Requery();
		_variant_t var;
		pRow = m_DefaultLocationCombo->GetRow(-1);
		var.vt = VT_NULL;
		pRow->PutValue(0,var);
		pRow->PutValue(1,"<No Location Selected>");
		m_DefaultLocationCombo->InsertRow(pRow,0);
		LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__RefreshSlowStuff, 5, 2000, "");
		// (a.walling 2010-10-12 17:58) - PLID 40908
		m_ForceRefresh = true;
	}

	if (m_refPatChecker.Changed()) {		
		LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__RefreshSlowStuff, 6);

		_variant_t var;

		if(m_referringPatientCombo != NULL) {
			// (a.walling 2011-08-01 17:34) - PLID 44788 - Don't load until dropping down
			/*
			m_referringPatientCombo->Requery();

			pRow = m_referringPatientCombo->GetRow(-1);
			var.vt = VT_NULL;
			pRow->PutValue(0,var);
			pRow->PutValue(1,"<No Patient Selected>");
			m_referringPatientCombo->InsertRow(pRow,0);

			pRow = m_referringPatientCombo->GetRow(-1);
			var = (long)-2;
			pRow->PutValue(0,var);
			pRow->PutValue(1,"<Unspecified>");
			m_referringPatientCombo->InsertRow(pRow,1);
			*/

			if (!m_referringPatientCombo->GetDropDownState() && !m_referringPatientCombo->IsRequerying()) {
				m_referringPatientCombo->Clear();
			}
		}

		// (a.walling 2006-11-14 12:34) - PLID 22715
		// (a.walling 2011-08-01 17:34) - PLID 44788 - Don't load until dropping down
		/*
		m_pRelatedPatientList->Requery();
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pRelatedPatientList->GetNewRow();
		pNewRow->PutValue(0, long(-1));
		pNewRow->PutValue(1, "<No Relation>");
		m_pRelatedPatientList->AddRowSorted(pNewRow, NULL);
		*/

		if (!m_pRelatedPatientList->GetDropDownState() && !m_pRelatedPatientList->IsRequerying()) {
			m_pRelatedPatientList->Clear();
		}

		// (d.thompson 2010-08-04) - PLID 39897 - This should only requery if we're on a repro license.
		if(IsReproductive()) {
			m_donorCombo->Requery();

			pRow = m_donorCombo->GetRow(-1);
			var.vt = VT_NULL;
			pRow->PutValue(0,var);
			pRow->PutValue(1,"<No Donor Selected>");
			m_donorCombo->InsertRow(pRow,0);
		}
		LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__RefreshSlowStuff, 6, 2000, "");
		// (a.walling 2010-10-12 17:58) - PLID 40908
		m_ForceRefresh = true;
	}
}

// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages
void CGeneral2Dlg::UpdateView(bool bForceRefresh)
{
	try {
		// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages
		CheckFocus();

		//(e.lally 2007-01-31) PLID 24518 - We need to store details so that our current field changes are not lost
		StoreDetails();

		// (j.gruber 2009-10-06 17:02) - PLID 35825 - reset here in case they changed the preference
		m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");

		if (m_bLookupByCity) {
			ChangeZOrder(IDC_EMP_ZIP, IDC_EMP_STATE);
		} else {
			ChangeZOrder(IDC_EMP_ZIP, IDC_EMP_ADDRESS2);
		}

		long nCurrentlyLoadedID = m_id;
		m_id = GetActivePatientID();
		m_strPatientName = GetExistingPatientName(m_id);
		
		RefreshSlowStuff();

		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;
		}

		if (bForceRefresh || m_ForceRefresh) {
			Refresh();
		}
		m_ForceRefresh = false;

	} NxCatchAll(__FUNCTION__);
}

//TES 11/13/2013 - PLID 59483
enum OccupationCensusCodeList
{
	occlID = 0,
	occlCode = 1,
	occlDescription = 2,
};
//TES 11/13/2013 - PLID 59483
enum IndustryCensusCodeLIst
{
	icclID = 0,
	icclCode = 1,
	iccsDescription = 3,
};

void CGeneral2Dlg::Refresh()
{
	try {

	// (j.jones 2008-03-20 16:09) - PLID 29334 - if they have the license,
	// requery the serialized product list

	LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__Refresh, 2a);

	m_id = GetActivePatientID();
	m_strPatientName = GetExistingPatientName(m_id);

	if(m_bHasAdvInventoryLicense) {
		CString strWhere;
		strWhere.Format("PatientID = %li", m_id);
		m_SerializedProductList->PutWhereClause(_bstr_t(strWhere));
		m_SerializedProductList->Requery();
	}

	// (j.fouts 2012-06-13 16:38) - PLID 50863 - Rather than forcing a trip to the database here we are using a table checker
	// (j.fouts 2012-06-12 15:34) - PLID 50863 - Requery this combo box before selecting
	//m_PatientTypeCombo->Requery();

	LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__Refresh, 2a, 2000, "");

	_RecordsetPtr rs;

	LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__Refresh, 2b);

	//new multi referral stuff
	CString temp;
	temp.Format("(SELECT ReferralID AS ID, ReferralSourceT.Name AS ReferralName, Date FROM MultiReferralsT INNER JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID WHERE PatientID = %li) AS SubQ", m_id);
	m_referralSourceList->FromClause = _bstr_t(temp);
	
	LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__Refresh, 2b, 2000, "");
	LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__Refresh, 4);

	// (j.jones 2005-04-14 09:36) - PLID 16238 - bypassed checking Auditing at all for the ModifiedDate,
	// now there is PatientsT.ModifiedDate which is updated every time an Audit is made to a patient record.
	// The logic is the same as before, it updates the ModifiedDate if the Audit ItemID is <= 1000.
	// (d.thompson 2009-03-20) - PLID 28726 - Added an accumulated reward points calculation.  Parameterized the query
	//	while I was here.  I just copied the query from the merge engine.
	// (a.walling 2010-06-30 17:43) - PLID 18081 - Warning categories
	// (j.gruber 2011-09-27 13:13) - PLID 45357 - Affiliate Physician
	// (d.thompson 2012-08-09) - PLID 52045 - Reworked ethnicity table structure
	// (d.thompson 2012-08-13) - PLID 52046 - Reworked language table structure
	// (j.gruber 2012-10-17 11:36) - PLID 47289 - affiliate phys type
	// (j.luckoski 2013-03-04 11:47) - PLID 33548 - Added DisplayRewardsWarning
	//TES 11/13/2013 - PLID 59483 - Added OccupationCensusCodeID and IndustryCensusCodeID
	// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading General2
	rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PersonT.Location, RaceIDSubQ.RaceID AS RaceID, PersonT.Ethnicity, PersonT.LanguageID, PatientsT.PersonID, PatientsT.UserDefinedID AS ID, PatientsT.DefaultReferringPhyID, PatientsT.PCP, PatientsT.Employment,   "
		" PatientsT.Occupation, PersonT.Company, PatientsT.EmployerFirst, PatientsT.EmployerMiddle, PatientsT.EmployerLast, PatientsT.EmployerAddress1, PatientsT.EmployerAddress2,  "
		" PatientsT.EmployerCity, PatientsT.EmployerState, PatientsT.EmployerZip AS EmployerPostalCode, ReferralSourceT.Name AS ReferralName, PatientsT.ReferralID, PatientsT.DefaultInjuryDate,  "
		" PatientsT.TypeOfPatient, PersonT.DisplayWarning, PersonT.WarningMessage, "
		" PatientsT.ReferringPatientID, UsersT.UserName, PersonT.WarningDate, PersonT.WarningUseExpireDate, PersonT.WarningExpireDate, PersonT.WarningCategoryID, PatientsT.DonorID, PatientsT.ModifiedDate, "
		" LocationsT.Name AS LocationName, "
		" PersonFamilyT.RelativeID, PersonFamilyT.FamilyID, "
		" (SELECT Sum(RewardHistoryT.Points) FROM RewardHistoryT WHERE DELETED = 0 AND RewardHistoryT.PatientID = PersonT.ID) AS RewardPoints, "
		" PatientsT.AffiliatePhysID, AffiliatePhysT.Archived as AffillArchived, AffiliatePhysT.Name as AffillName, PatientsT.AffiliatePhysType, "
		" PersonT.DisplayRewardsWarning, PatientsT.OccupationCensusCodeID, PatientsT.IndustryCensusCodeID "
		" FROM   "
		" (PersonT WITH(NOLOCK) INNER JOIN PatientsT WITH(NOLOCK) ON PersonT.ID = PatientsT.PersonID)   "
		" LEFT JOIN ReferralSourceT WITH(NOLOCK) ON PatientsT.ReferralID = ReferralSourceT.PersonID  "
		" LEFT JOIN UsersT WITH(NOLOCK) ON UsersT.PersonID = PersonT.WarningUserID  "
		" LEFT JOIN LocationsT WITH(NOLOCK) ON LocationsT.ID = PersonT.Location "
		" LEFT JOIN PersonFamilyT WITH(NOLOCK) ON PersonFamilyT.PersonID = PatientsT.PersonID "
		" LEFT JOIN (SELECT Archived, PersonT.ID, [Last] + ', ' + [First] + ' ' + [Middle] + ' ' + [Title] AS Name FROM PersonT WITH(NOLOCK)) AffiliatePhysT ON PatientsT.AffiliatePhysID = AffiliatePhysT.ID "
		" LEFT JOIN (SELECT CASE WHEN COUNT(*) > 1 THEN -2 ELSE MAX(RaceID) END AS RaceID, PersonID FROM PersonRaceT PRT GROUP BY PersonID) RaceIDSubQ ON RaceIDSubQ.PersonID = PersonT.ID "
		" WHERE PersonT.ID = {INT};\r\n", m_id);

	int employment;
	if (rs->Fields->Item["Employment"]->Value.vt != VT_NULL)
		employment = rs->Fields->Item["Employment"]->Value.intVal;
	else employment = 0;

	LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__Refresh, 4, 2000, "");
	LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__Refresh, 3);

	// (c.haag 2006-04-13 09:42) - PLID 20119 - Grab the referral ID now so that when the referral
	// list is finished requerying, we don't have to look up the database to find out what the primary
	// referral is
	m_nReferralID = AdoFldLong(rs, "ReferralID", -1);

	RefreshReferrals();

	// (a.wilson 2012-5-23) PLID 48537 - check permissions for referral source list access.
	CPermissions permReferrals = GetCurrentUserPermissions(bioPatientReferralSources);
	//check write permissions
	if ((permReferrals & (sptWrite | sptWriteWithPass)))
		m_btnAddReferral.EnableWindow(TRUE);
	else {
		m_btnAddReferral.EnableWindow(FALSE);
	}

	LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__Refresh, 3, 2000, "");
	LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__Refresh, 5);

	switch (employment)
	{	case 1:
			m_fulltime.SetCheck			(true);
			m_fulltimeStudent.SetCheck	(false);
			m_parttimeStudent.SetCheck	(false);
			m_parttime.SetCheck			(false);
			m_retired.SetCheck			(false);
			m_other.SetCheck			(false);
			break;
		case 2:
			m_fulltimeStudent.SetCheck	(true);
			m_fulltime.SetCheck			(false);
			m_parttimeStudent.SetCheck	(false);
			m_parttime.SetCheck			(false);
			m_retired.SetCheck			(false);
			m_other.SetCheck			(false);
			break;
		case 3:
			m_parttimeStudent.SetCheck	(true);
			m_fulltime.SetCheck			(false);
			m_fulltimeStudent.SetCheck	(false);
			m_parttime.SetCheck			(false);
			m_retired.SetCheck			(false);
			m_other.SetCheck			(false);
			break;
		case 4:
			m_parttime.SetCheck			(true);
			m_fulltime.SetCheck			(false);
			m_fulltimeStudent.SetCheck	(false);
			m_parttimeStudent.SetCheck	(false);
			m_retired.SetCheck			(false);
			m_other.SetCheck			(false);
			break;
		case 5:
			m_retired.SetCheck			(true);
			m_fulltime.SetCheck			(false);
			m_fulltimeStudent.SetCheck	(false);
			m_parttimeStudent.SetCheck	(false);
			m_parttime.SetCheck			(false);
			m_other.SetCheck			(false);
			break;
		case 6:
			m_other.SetCheck			(true);
			m_fulltime.SetCheck			(false);
			m_fulltimeStudent.SetCheck	(false);
			m_parttimeStudent.SetCheck	(false);
			m_parttime.SetCheck			(false);
			m_retired.SetCheck			(false);
			break;
		default:
			m_fulltime.SetCheck			(false);
			m_fulltimeStudent.SetCheck	(false);
			m_parttimeStudent.SetCheck	(false);
			m_parttime.SetCheck			(false);
			m_retired.SetCheck			(false);
			m_other.SetCheck			(true);
			break;
	}
	SetDlgItemText (IDC_OCCUPATION, CString(rs->Fields->Item["Occupation"]->Value.bstrVal));
	SetDlgItemText (IDC_MANAGER_FNAME, CString(rs->Fields->Item["EmployerFirst"]->Value.bstrVal));
	SetDlgItemText (IDC_MANAGER_MNAME, CString(rs->Fields->Item["EmployerMiddle"]->Value.bstrVal));
	SetDlgItemText (IDC_MANAGER_LNAME, CString(rs->Fields->Item["EmployerLast"]->Value.bstrVal));
	SetDlgItemText (IDC_EMP_ADDRESS1, CString(rs->Fields->Item["EmployerAddress1"]->Value.bstrVal));
	SetDlgItemText (IDC_EMP_ADDRESS2, CString(rs->Fields->Item["EmployerAddress2"]->Value.bstrVal));
	SetDlgItemText (IDC_EMP_CITY, CString(rs->Fields->Item["EmployerCity"]->Value.bstrVal));
	SetDlgItemText (IDC_EMP_STATE, CString(rs->Fields->Item["EmployerState"]->Value.bstrVal));
	SetDlgItemText (IDC_EMP_ZIP, CString(rs->Fields->Item["EmployerPostalCode"]->Value.bstrVal));
	SetDlgItemText (IDC_WARNING, CString(rs->Fields->Item["WarningMessage"]->Value.bstrVal));

	//TES 10/23/03: I set this to check for VT_DATE instead of VT_BSTR(!)  Not that it matters,
	//because I also hid the fields, because ModifiedDate is never written to, so it's a foolish
	//thing to show on screen.
	if (rs->Fields->Item["ModifiedDate"]->Value.vt != VT_DATE)
		SetDlgItemText(IDC_LAST_MODIFIED, "");
	else
		SetDlgItemText(IDC_LAST_MODIFIED, FormatDateTimeForInterface(AdoFldDateTime(rs, "ModifiedDate")));

	if (GetPropertyInt("G2ShowWarningStats", 1))
	{
		CString strWarning;
		COleDateTime dt, dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		dt = AdoFldDateTime(rs, "WarningDate", dtInvalid);
		strWarning.Format("%s %s", AdoFldString(rs, "UserName", ""), dt.GetStatus() == COleDateTime::invalid ? "" : CString("(") + FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoDate) + ")");
		SetDlgItemText(IDC_EDIT_WARNING_USER, strWarning);
	}
	else
	{
		SetDlgItemText(IDC_EDIT_WARNING_USER, "");
	}
	
	// (j.luckoski 2013-03-05 11:54) - PLID 33548 - If pref is off then hide the checkbox
	if(!GetRemotePropertyInt("DisplayRewardsWarning",0,0, GetCurrentUserName(),true) == 1) {
		GetDlgItem(IDC_WARNING_CHECK2)->ShowWindow(SW_HIDE);
	} else {
		GetDlgItem(IDC_WARNING_CHECK2)->ShowWindow(SW_SHOW);
	}

	m_WarningCheck.SetCheck(rs->Fields->Item["DisplayWarning"]->Value.boolVal);
	m_RewardsWarningCheck.SetCheck(rs->Fields->Item["DisplayRewardsWarning"]->Value.boolVal); // (j.luckoski 2013-03-04 11:48) - PLID 33548
	m_expireDate.SetValue(_variant_t(COleDateTime::GetCurrentTime()));	
	
	//TES 11/13/2013 - PLID 59483 - Added Occupation and Industry Census Code lists
	m_pOccupationCodeList->SetSelByColumn(occlID, rs->Fields->Item["OccupationCensusCodeID"]->Value);
	m_pIndustryCodeList->SetSelByColumn(icclID, rs->Fields->Item["IndustryCensusCodeID"]->Value);
	


	// (a.walling 2010-06-30 17:43) - PLID 18081 - Warning categories
	m_pWarningCategoryList->SetSelByColumn(0, rs->Fields->Item["WarningCategoryID"]->Value);

	if (m_WarningCheck.GetCheck()) {
		if (AdoFldBool(rs, "WarningUseExpireDate")) {

			GetDlgItem(IDC_EXPIRE_DATE)->EnableWindow(TRUE);
			CheckDlgButton(IDC_EXPIRE_DATE, TRUE);

			GetDlgItem(IDC_EXPIRE_DATE_PICKER)->EnableWindow(TRUE);
			m_expireDate.SetValue(rs->Fields->Item["WarningExpireDate"]->Value);
		}	
		else {
			GetDlgItem(IDC_EXPIRE_DATE)->EnableWindow(TRUE);
			CheckDlgButton(IDC_EXPIRE_DATE, FALSE);
			GetDlgItem(IDC_EXPIRE_DATE_PICKER)->EnableWindow(FALSE);
		}
	}
	else {		
		GetDlgItem(IDC_EXPIRE_DATE)->EnableWindow(FALSE);
		// (b.cardillo 2006-07-19 16:05) - PLID 21521 - For correctness and a consistent UI, we 
		// uncheck the box in case the previous patient had it checked.
		CheckDlgButton(IDC_EXPIRE_DATE, FALSE);
		GetDlgItem(IDC_EXPIRE_DATE_PICKER)->EnableWindow(FALSE);
	}

	_variant_t varDate = rs->Fields->Item["DefaultInjuryDate"]->Value;
	if(varDate.vt == VT_DATE) {
		m_dtAccident = VarDateTime(varDate);
		m_nxtAccidentDate->SetDateTime(m_dtAccident);
	}
	else {
		m_nxtAccidentDate->Clear();
		//(e.lally 2007-01-31) PLID 24518 - Cleanup: This SetDateTime does not actually reset the date.
		//m_dtAccident.SetDateTime(0,0,0,0,0,0);
		m_dtAccident.m_dt = 0;
		m_dtAccident.SetStatus(COleDateTime::invalid);
	}

	LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__Refresh, 5, 2000, "");
	LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__Refresh, 6);
/* MULTI	
	if(rs->Fields->Item["ReferralName"]->Value.vt == VT_NULL){
		SetDlgItemText (IDC_REFERRAL, "");
		m_referralTree.SelectItem(NULL);
	}
	else 
	{
		SetDlgItemText (IDC_REFERRAL, CString(rs->Fields->Item["ReferralName"]->Value.bstrVal));

		m_referralTree.Select(AdoFldLong(rs, "ReferralID"));
	}
*/
	//DRT 8/6/2004 - PLID 13805 - This can be -2, and must be set!
	// (a.walling 2007-05-21 11:12) - PLID 26079 - Store the referring patient id
	m_nReferringPatientID = AdoFldLong(rs, "ReferringPatientID", -1);

	// (a.walling 2011-08-01 17:34) - PLID 44788 - Don't load until dropping down
	/*
	if(m_referringPatientCombo != NULL) {
		if(AdoFldLong(rs, "ReferringPatientID", 0) == -1)
			m_referringPatientCombo->CurSel = -1;
		else
			m_referringPatientCombo->TrySetSelByColumn(0, rs->Fields->Item["ReferringPatientID"]->Value);			
	}
	*/
	TrySetReferringPatient();

	// (j.gruber 2011-09-27 13:19) - PLID 45357 - affiliate phys
	if(IsReproductive()) {
		if(AdoFldLong(rs, "DonorID", 0) <= 0)
			m_donorCombo->CurSel = -1;
		else
			m_donorCombo->TrySetSelByColumn(0,rs->Fields->Item["DonorID"]->Value);
	}
	else {
		long nAffilID = AdoFldLong(rs, "AffiliatePhysID", -1);
		if ( nAffilID == -1) {
			m_nAffiliatePhysID = -1;
			m_strAffiliateName = "";			
			m_pAffiliateList->TrySetSelByColumn_Deprecated(afclID, nAffilID);
		}
		else {
			m_nAffiliatePhysID = nAffilID;
			m_strAffiliateName = AdoFldString(rs, "AffillName", "");
			//see if its archived
			if (AdoFldBool(rs, "AffillArchived", 0) == 1) {
				//set the textBox
				m_nAffiliatePhysID = -2;
				m_pAffiliateList->PutComboBoxText(_bstr_t(AdoFldString(rs, "AffillName", "")));				
			}
			else {
				//since we are setting our own stuff, we are ok to not handle this, I just didn't want it waiting for SetSel
				m_pAffiliateList->TrySetSelByColumn_Deprecated(afclID, nAffilID);
			}
		}
		//j.gruber 2012-10-16 - PLID 47289
		AffiliatePhysType nAffilTypeID = (AffiliatePhysType) AdoFldLong(rs, "AffiliatePhysType", afptNone);
		if (nAffilTypeID == afptNone) {
			m_nAffiliatePhysTypeID = afptNone;
			m_pAffiliateTypeList->CurSel = NULL;
		}
		else {
			m_nAffiliatePhysTypeID = nAffilTypeID;
			m_pAffiliateTypeList->TrySetSelByColumn_Deprecated(aptcID, nAffilTypeID);
		}		

	}

	// (b.spivey, May 28, 2013) - PLID 56872 0 if it's a multiselect then we'll just get all the races. 
	//		If it's just a single select then we'll add that to our member array. 
	long nRaceID = AdoFldLong(rs, "RaceID", 0);
	m_naryRaceIDs.RemoveAll(); 
	if(nRaceID == rcrMultiSelectRow) {
		m_raceCombo->TrySetSelByColumn(raceID, nRaceID);
		// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading General2
		_RecordsetPtr prsRace = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT DISTINCT RaceID FROM PersonRaceT WHERE PersonID = {INT}", m_id);
		while (!prsRace->eof) {
			m_naryRaceIDs.Add(AdoFldLong(prsRace->Fields, "RaceID", 0));
			prsRace->MoveNext();
		}
	}
	else if (nRaceID > 0) {
		m_naryRaceIDs.Add(nRaceID); 
	}

	//Make sure we show or hide the label appropriately. 
	ToggleLabels(); 

	// (j.jones 2009-10-19 10:08) - PLID 34327 - added ethnicity combo
	// (d.thompson 2012-08-09) - PLID 52045 - Reworked ethnicity table structure
	if(AdoFldLong(rs, "Ethnicity", 0) <= 0)
		m_ethnicityCombo->CurSel = -1;
	else
		m_ethnicityCombo->TrySetSelByColumn(ethnicityID,rs->Fields->Item["Ethnicity"]->Value);

	// (a.walling 2009-05-25 11:29) - PLID 27859
	// (d.thompson 2012-08-13) - PLID 52046 - Reworked language table structure
	if(AdoFldLong(rs, "LanguageID", -1) == -1)
		m_languageCombo->CurSel = NULL;
	else
		m_languageCombo->TrySetSelByColumn_Deprecated(0, rs->Fields->Item["LanguageID"]->Value);
	
	m_bSettingCompany = true;
	SetDlgItemText (IDC_COMPANY, CString(rs->Fields->Item["Company"]->Value.bstrVal));
	m_bSettingCompany = false;
	
	LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__Refresh, 6, 2000, "");
	LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__Refresh, 7);

	// (z.manning 2008-12-08 12:39) - PLID 32320 - Show/hide columns in the ref phys list based on
	// the preferences to show them.
	// (j.jones 2011-12-01 13:59) - PLID 46771 - applied these changes to the PCP combo too
	if(GetRemotePropertyInt("RefPhysComboShowNPI", 1, 0, "<None>") == 0) {
		m_ReferringPhyCombo->GetColumn(rpcNpi)->PutStoredWidth(0);
		m_PCPCombo->GetColumn(pcpcNpi)->PutStoredWidth(0);
	}
	else {
		m_ReferringPhyCombo->GetColumn(rpcNpi)->PutStoredWidth(80);
		m_PCPCombo->GetColumn(pcpcNpi)->PutStoredWidth(80);
	}
	if(GetRemotePropertyInt("RefPhysComboShowID", 0, 0, "<None>") == 0) {
		m_ReferringPhyCombo->GetColumn(rpcRefPhysID)->PutStoredWidth(0);
		m_PCPCombo->GetColumn(pcpcRefPhysID)->PutStoredWidth(0);
	}
	else {
		m_ReferringPhyCombo->GetColumn(rpcRefPhysID)->PutStoredWidth(70);
		m_PCPCombo->GetColumn(pcpcRefPhysID)->PutStoredWidth(70);
	}
	if(GetRemotePropertyInt("RefPhysComboShowUPIN", 0, 0, "<None>") == 0) {
		m_ReferringPhyCombo->GetColumn(rpcUpin)->PutStoredWidth(0);
		m_PCPCombo->GetColumn(pcpcUpin)->PutStoredWidth(0);
	}
	else {
		m_ReferringPhyCombo->GetColumn(rpcUpin)->PutStoredWidth(70);
		m_PCPCombo->GetColumn(pcpcUpin)->PutStoredWidth(70);
	}

	//(e.lally 2005-11-02) PLID 17444 - add the ability to make referring physicians inactive
	if(AdoFldLong(rs, "DefaultReferringPhyID", 0) <= 0){
		m_ReferringPhyCombo->PutCurSel(-1);
	}
	else{
		long nDefaultRefPhysID = AdoFldLong(rs, "DefaultReferringPhyID");
		if(m_ReferringPhyCombo->SetSelByColumn(rpcID,nDefaultRefPhysID) == -1){
			m_ReferringPhyCombo->PutCurSel(-1);
			//they may have an inactive referring physician
			// (d.thompson 2010-08-04) - PLID 39897 - Parameterized
			// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading General2
			_RecordsetPtr rsProv = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", nDefaultRefPhysID);
			if(!rsProv->eof) {
				m_ReferringPhyCombo->PutComboBoxText(_bstr_t(AdoFldString(rsProv, "Name", "")));
			}
		}
	}
	if(AdoFldLong(rs, "PCP", 0) <= 0){
		m_PCPCombo->PutCurSel(-1);
	}
	else{
		long nPCP = AdoFldLong(rs, "PCP");
		if(m_PCPCombo->SetSelByColumn(0,nPCP) == -1){
			m_PCPCombo->PutCurSel(-1);
			//they may have an inactive primary care physician
			// (d.thompson 2010-08-04) - PLID 39897 - Parameterized
			// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading General2
			_RecordsetPtr rsPCP = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE ID = {INT}", nPCP);
			if(!rsPCP->eof) {
				m_PCPCombo->PutComboBoxText(_bstr_t(AdoFldString(rsPCP, "Name", "")));
			}
		}
	}

	// (j.jones 2014-02-19 09:42) - PLID 60719 - default diagnosis codes are now
	// loaded by a simple requery of m_defaultDiagList
	{
		CString strDiagWhere;
		strDiagWhere.Format("PersonID = %li", m_id);
		m_defaultDiagList->PutWhereClause(_bstr_t(strDiagWhere));
		//OnRequeryFinished will resize columns as needed.
		m_defaultDiagList->Requery();
	}

	long nTypeOfPatient = AdoFldLong(rs, "TypeOfPatient", 0);
	if(nTypeOfPatient <= 0)
		m_PatientTypeCombo->CurSel = -1;
	else
		m_PatientTypeCombo->TrySetSelByColumn(0,nTypeOfPatient);
	// (s.tullis 2016-05-25 15:41) - NX-100760 
	_variant_t nDefaultLocation= rs->Fields->Item["Location"]->Value;
	m_DefaultLocationCombo->TrySetSelByColumn(0,nDefaultLocation);
	

	// (a.walling 2006-11-14 16:54) - PLID 22715 - Attempt to select the related patient
	m_nRelatedPatient = AdoFldLong(rs, "RelativeID", -1);
	TrySetRelatedPatient();

	// (d.thompson 2009-03-20) - PLID 28726 - Added reward points.
	COleCurrency cyRewardPts = AdoFldCurrency(rs, "RewardPoints", COleCurrency(0, 0));
	SetDlgItemText(IDC_ACCUM_REWARD_PTS, FormatCurrencyForInterface(cyRewardPts, FALSE, TRUE));

	LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__Refresh, 7, 2000, "");
	LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__Refresh, 7a);

	rs->Close();

	LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__Refresh, 7a, 2000, "");
	LOG_IF_TOO_SLOW_BEFORE(CGeneral2Dlg__Refresh, 8);

	// Calc and set the "referred person" counts for this patient
	{
		// Get the counts
		// (d.moore 2007-05-02 13:43) - PLID 23602 - 'CurrentStatus = 3' was used to determine both 
		//  the number of Patients and the number of Prospects. It is now only counted as a Patient.
		// (d.thompson 2010-08-04) - PLID 39897 - Rewrote to double the speed of the query.  Param while I'm at it.
		// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading General2
		_RecordsetPtr prsReferredCounts = CreateParamRecordset(GetRemoteDataSnapshot(), 
			"SELECT "
			"   COALESCE(SUM(CASE WHEN CurrentStatus IN (1, 3) THEN 1 ELSE 0 END), 0) AS CountOfReferredPatients, "
			"   COALESCE(SUM(CASE WHEN CurrentStatus IN (2) THEN 1 ELSE 0 END), 0) AS CountOfReferredProspects "
			"FROM PatientsT WHERE ReferringPatientID = {INT} ", m_id);
		FieldsPtr pfldsRefCnts = prsReferredCounts->GetFields();

		// Put the counts on screen
		long nNumReferredPatients = AdoFldLong(pfldsRefCnts, "CountOfReferredPatients");
		SetDlgItemInt(IDC_NUM_PATIENTS_REF, nNumReferredPatients);
		if(nNumReferredPatients <= 0){
			// grey out the elipsis button
			GetDlgItem(IDC_REFERRED_PATS)->EnableWindow(FALSE);
		}
		else{
			GetDlgItem(IDC_REFERRED_PATS)->EnableWindow(TRUE);
		}
			
		long nNumReferredProspects = AdoFldLong(pfldsRefCnts, "CountOfReferredProspects");
		SetDlgItemInt(IDC_NUM_PROSPECTS_REF, nNumReferredProspects);
		if(nNumReferredProspects <= 0){
			// grey out the elipsis button
			GetDlgItem(IDC_REFERRED_PROSPECTS)->EnableWindow(FALSE);
		}
		else{
			GetDlgItem(IDC_REFERRED_PROSPECTS)->EnableWindow(TRUE);
		}
	}

	LOG_IF_TOO_SLOW_AFTER(CGeneral2Dlg__Refresh, 8, 2000, "");

	// (z.manning, 5/19/2006, PLID 20705) - The dialog thinks something has changed now when
	// that's not even possible since we just loaded from data, so let's make sure we don't 
	// unnecessarily save anything.
	m_changed = false;
	m_ForceRefresh = false;

	} NxCatchAll("Error in reloading data: General2Dlg::Refresh()");
}

void CGeneral2Dlg::OnFulltimeRadio()
{
	try {

		long employment;

		CString strOld = "";
		_RecordsetPtr rs = CreateRecordset("SELECT Employment FROM PatientsT WHERE PersonID = %li",m_id);
		if(!rs->eof) {
			int employment = AdoFldLong(rs, "Employment",0);
			if(employment == 1)
				strOld = "Full-Time";
			else if(employment == 2)
				strOld = "Full-Time Student";
			else if(employment == 3)
				strOld = "Part-Time Student";
			else if(employment == 4)
				strOld = "Part-Time";
			else if(employment == 5)
				strOld = "Retired";
			else if(employment == 6)
				strOld = "Other";
		}

		CString strNew = "";

		if (m_fulltime.GetCheck()) {
			employment = 1;
			strNew = "Full-Time";
		}
		else if (m_fulltimeStudent.GetCheck()) {
			employment = 2;
			strNew = "Full-Time Student";
		}
		else if (m_parttimeStudent.GetCheck()) {
			employment = 3;
			strNew = "Part-Time Student";
		}
		else if (m_parttime.GetCheck()) {
			employment = 4;
			strNew = "Part-Time";
		}
		else if (m_retired.GetCheck()) {
			employment = 5;
			strNew = "Retired";
		}
		else if (m_other.GetCheck()) {
			employment = 6;
			strNew = "Other";
		}
		else employment = 0;

		ExecuteSql("UPDATE PatientsT SET Employment = %li WHERE PersonID = %li",employment,m_id);

		if(strOld != strNew) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientEmployment, m_id, strOld, strNew, aepMedium, aetChanged);
			UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
		}

	} NxCatchAll("Error in updating employment status");
}

void CGeneral2Dlg::OnFulltimeStudentRadio() 
{
	OnFulltimeRadio();
}

void CGeneral2Dlg::OnOtherRadio() 
{
	OnFulltimeRadio();
}

void CGeneral2Dlg::OnParttimeRadio() 
{
	OnFulltimeRadio();
}

void CGeneral2Dlg::OnParttimeStudentRadio() 
{
	OnFulltimeRadio();
}

void CGeneral2Dlg::OnRetiredRadio() 
{
	OnFulltimeRadio();
}

void CGeneral2Dlg::OnWarningCheck()
{
	if (UserPermission(ChangeWarning))
	{	
		long nCheck = m_WarningCheck.GetCheck();

		if (nCheck != 0) {
			GetDlgItem(IDC_WARNING)->SetFocus();
		}
		SetExpireBox();
		SetWarningBox();
					
	}
	else {
		//TS: Check this based on the data, NOT the current state of the checkbox, because in certain circumstances
		//the permission stuff can change the state of the checkbox
		_RecordsetPtr rsWarning = CreateRecordset("SELECT DisplayWarning FROM PersonT WHERE ID = %li", m_id);
		BOOL bCheck = AdoFldBool(rsWarning, "DisplayWarning");
		m_WarningCheck.SetCheck(bCheck);
	}
}


void CGeneral2Dlg::SetExpireBox() {

	if (m_WarningCheck.GetCheck()) {

		//its check so enable the box
		GetDlgItem(IDC_EXPIRE_DATE)->EnableWindow(TRUE);
		//set the check to be false
		CheckDlgButton(IDC_EXPIRE_DATE, FALSE);
		GetDlgItem(IDC_EXPIRE_DATE_PICKER)->EnableWindow(FALSE);
	}
	else {
	
		GetDlgItem(IDC_EXPIRE_DATE)->EnableWindow(FALSE);
		//set the check to be false
		CheckDlgButton(IDC_EXPIRE_DATE, FALSE);
		GetDlgItem(IDC_EXPIRE_DATE_PICKER)->EnableWindow(FALSE);

		//Update the data

	}

}


void CGeneral2Dlg::SetWarningBox() {

	
	ExecuteSql("UPDATE PersonT SET DisplayWarning = %li WHERE ID = %li", m_WarningCheck.GetCheck(),m_id);

	//auditing
	long nAuditID = BeginNewAuditEvent();
	AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientWarning, m_id, (m_WarningCheck.GetCheck() == 0 ? "Warn" : "Don't Warn"), (m_WarningCheck.GetCheck() == 0 ? "Don't Warn" : "Warn"), aepMedium, aetChanged);
	UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
}

// (j.luckoski 2013-03-04 11:48) - PLID 33548 - Update warning box to show or not to show
void CGeneral2Dlg::SetRewardsWarningBox() {

	
	ExecuteSql("UPDATE PersonT SET DisplayRewardsWarning = %li WHERE ID = %li", m_RewardsWarningCheck.GetCheck(),m_id);

	//auditing
	long nAuditID = BeginNewAuditEvent();
	AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientWarning, m_id, (m_RewardsWarningCheck.GetCheck() == 0 ? "Warn" : "Don't Warn"), (m_RewardsWarningCheck.GetCheck() == 0 ? "Don't Warn" : "Warn"), aepMedium, aetChanged);
	UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
}

BOOL CGeneral2Dlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID;

	switch (HIWORD(wParam))
	{	case EN_CHANGE:
			switch (nID = LOWORD(wParam))
			{
				case IDC_MANAGER_MNAME:
					// (c.haag 2006-08-02 11:40) - PLID 21740 - We now check for auto-capitalization
					// for middle name boxes
					if (GetRemotePropertyInt("AutoCapitalizeMiddleInitials", 1, 0, "<None>", true)) {
						Capitalize(nID);
					}
					break;

				case IDC_OCCUPATION:
				case IDC_MANAGER_FNAME:
				case IDC_MANAGER_LNAME:
				case IDC_EMP_ADDRESS1:
				case IDC_EMP_ADDRESS2:
				case IDC_EMP_CITY:
				case IDC_EMP_STATE:
					Capitalize(nID);
					break;
				
				case IDC_EMP_ZIP:
				{	
					// (d.moore 2007-04-23 12:11) - PLID 23118 - 
					//  Capitalize letters in the zip code as they are typed in. Canadian postal
					//    codes need to be formatted this way.
					CapitalizeAll(IDC_EMP_ZIP);
					CString str;
					GetDlgItemText(nID, str);
					str.TrimRight();
					//if (str != "")
					//	FormatItem (nID, "#####-nnnn");
					break;
				}
				default:
					m_changed = true;
					return CNxDialog::OnCommand(wParam, lParam);
			}
			m_changed = true;
			break;
		case EN_KILLFOCUS:
			Save(LOWORD(wParam));
		break;
	}	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CGeneral2Dlg::OnSelectReferral(NMHDR* pNMHDR, LRESULT* pResult) 
{
/* MULTI
	if (m_updateable)
	{
		CString tmpText = m_referralTree.GetSelectedItemText();
		SetDlgItemText (IDC_REFERRAL, tmpText);
		Save(IDC_REFERRAL);
	}
*/
}

void CGeneral2Dlg::UpdatePalm(BOOL bUpdateAppointments)
{
	long nPalmID = m_id;
	SetPalmRecordStatus("PersonT", nPalmID, 1);

	if (bUpdateAppointments) {
		UpdatePalmSyncTByPerson(m_id);
		PPCRefreshAppts(m_id);
	}
}

void CGeneral2Dlg::OnEndlabeleditReferralTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnSelectReferral(pNMHDR, pResult);
}

void CGeneral2Dlg::OnEditPatientTypeCombo() 
{
	//DRT 3/17/2004 - PLID 11481 - This was getting the name out, then requerying, then selecting
	//	on the name instead of ID!  Also put in a try/catch 
	try {

		// (b.spivey, May 14, 2012) - PLID 50224 - Deprecated the old combobox in favor of a new interface. 

		_variant_t value;
		long curSel = m_PatientTypeCombo->CurSel;
		if (curSel != -1)
			value = m_PatientTypeCombo->Value[curSel][0];

		// (b.spivey, May 09, 2012) - PLID 50224 - New Dialog at work here. 
		CEditPatientTypeDlg dlg; 
		dlg.DoModal(); 

		//Requery after the previous dialog. The previous dialog used to pass the list by pointer, but the new dialog 
		//   is completely independent. 
		m_PatientTypeCombo->Requery(); 

		if (curSel != -1) {
			m_PatientTypeCombo->SetSelByColumn(0, value);
		}

		// (b.spivey, May 15, 2012) - PLID 20752 - Update the color when they change patient types. 
		if(GetMainFrame()) {
			CPatientView* pView = (CPatientView*)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
			if (pView) {
				pView->SetColor();
			}
		}

		GetDlgItem(IDC_PATIENT_TYPE_COMBO)->SetFocus();
	} NxCatchAll("Error in OnEditPatientTypeCombo()");
}

void CGeneral2Dlg::OnEditRaceCombo()
{
	try {

		_variant_t value;
		long curSel = m_raceCombo->CurSel;
		if(curSel != -1) {
			value = m_raceCombo->Value[curSel][raceID];
		}

		// (j.jones 2009-10-15 10:27) - PLID 34327 - we now have a dedicated
		// race editor dialog
		CRaceEditDlg dlg(this);
		dlg.DoModal();

		m_raceCombo->Requery();		

		if(curSel != -1) {
			m_raceCombo->TrySetSelByColumn(raceID, value);
		}

		GetDlgItem(IDC_CODE_RACE)->SetFocus();

	} NxCatchAll("Error in CGeneral2Dlg::OnEditRaceCombo()");
}

void CGeneral2Dlg::OnSelChosenReferringPatientCombo(long nRow) 
{
	// (a.walling 2006-10-05 14:34) - PLID 22844 - Fix the no selection exception
	if (nRow == NXDATALISTLib::sriNoRow) {
		return;
	}
	try
	{
		//for auditing
		CString strOld;
		_RecordsetPtr rs = CreateRecordset("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name FROM PersonT WHERE ID = (SELECT ReferringPatientID FROM PatientsT WHERE PersonID = %li)", m_id);
		if(!rs->eof && rs->Fields->Item["Name"]->Value.vt == VT_BSTR)
			strOld = CString(rs->Fields->Item["Name"]->Value.bstrVal);
		//

		// (a.walling 2009-11-05 17:42) - PLID 36218 - Fixed foreign key violation
		_variant_t varReferringPatient = m_referringPatientCombo->GetValue(nRow,0);

		long nNewRefPatID = VarLong(varReferringPatient, -1);

		if(nNewRefPatID == -1) {
			ExecuteSql("UPDATE PatientsT SET ReferringPatientID = NULL WHERE PersonID = %li", m_id);
			m_referringPatientCombo->CurSel = -1;

			// (a.walling 2007-05-21 11:14) - PLID 26079 - Update the referring patient points
			try {
				if (m_nReferringPatientID != -1 && m_nReferringPatientID != -2) {
					Rewards::UnapplyRefPatient(m_id, Rewards::erdrRemoved);
				}
			} NxCatchAll("Error updating referred patient reward points!");

			m_nReferringPatientID = -1;
		}
		else{
			//check to see if they selected themselves
			if(m_referringPatientCombo->GetValue(nRow,0).lVal == m_id){
				AfxMessageBox("A patient cannot be self-referred.");
				// (a.walling 2007-08-29 13:00) - Fix pending in PLID 27230 (8500)
				// #pragma TODO("a.walling - The patient set themselves as the referring patient. We set the cursel to -1, but don't change data at all. Shouldn't we revert to the last selection, since nothing has changed?")
				m_referringPatientCombo->CurSel = -1;
				return;
			}
			else {
				ExecuteSql("UPDATE PatientsT SET ReferringPatientID = %li WHERE PersonID = %li", nNewRefPatID, m_id);

				// (a.walling 2007-05-21 11:14) - PLID 26079 - Update the referring patient points
				// (a.walling 2009-11-05 17:42) - PLID 36218 - Fixed foreign key violation
				if (nNewRefPatID != -2) { 
					try {
						if (m_nReferringPatientID != -1 && m_nReferringPatientID != -2) {
							Rewards::UnapplyRefPatient(m_id, Rewards::erdrChanged);
						}
						Rewards::ApplyRefPatient(m_id, nNewRefPatID);
					} NxCatchAll("Error updating referred patient reward points!");
				}

				m_nReferringPatientID = nNewRefPatID;
			}
		}

		// (j.dinatale 2012-12-19 10:52) - PLID 54186 - need to fire a table checker here because the marketing module needs to refresh its horrible caching
		CClient::RefreshTable(NetUtils::ReferralSourceT);

		//auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientRefPat, m_id, strOld, CString(m_referringPatientCombo->GetValue(nRow, 1).bstrVal), aepMedium, aetChanged);
		UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
		//

		//DRT 9/24/03 - PLID 9574 - Added a preference to auto-add a referral source when you choose this.
		//	Only do this if nothing was selected.
		// (b.eyers 8/18/2014) - PLID 63050 - don't add the default referral source if 'no patient selected' was choosen for referred by a patient
		CString strRefPat = VarString(m_referringPatientCombo->GetValue(nRow, 1));
		if (strOld.IsEmpty() && strRefPat != "<No Patient Selected>") {
			long nDefReferral = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
			if (nDefReferral != -1) {
				//Just call the AddReferral function, it handles writing the data and all that good stuff.
				//However, we don't want to call this if we already have a referral source for this patient.  They've already got
				//the one we want selected, so just ignore it.
				if (!ReturnsRecords("SELECT * FROM MultiReferralsT WHERE ReferralID = %li AND PatientID = %li", nDefReferral, GetActivePatientID())) {
					// (r.goldschmidt 2014-09-19 10:28) - PLID 31191 - Check for possible conflict with parent referral preference
					//   If selection not allowed, throw a message box, no referral source gets added, DefaultPatientReferral preference remains intact.
					CString strWarning;
					if (!IsReferralSelectionAllowed(nDefReferral, strWarning)) {
						strWarning += " The default referring patient referral source conflicts with these preferences.\n\nPlease manually add the desired referral source and review preference settings.";
						AfxMessageBox(strWarning);
					}
					else {
						AddReferral(nDefReferral);
					}
				}
			}
		}
	}
	NxCatchAll("Could not save referring patient.");
}

void CGeneral2Dlg::OnKillfocusReferralTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
/* MULTI
	Save(IDC_REFERRAL);
	*pResult = 0;
*/
}

void CGeneral2Dlg::OnSelChosenRaceCombo(long nRow)
{
	try{

		// (b.spivey, May 28, 2013) - PLID 56872 - Race has to be inserted into a tag table now. 
		CSqlFragment sqlInsertRace(" "); 
		// (b.spivey, June 13, 2013) - PLID 56872 - no row handling.  
		if (nRow == NXDATALISTLib::sriNoRow) {
			m_naryRaceIDs.RemoveAll(); 
		}
		else if(VarLong(m_raceCombo->GetValue(nRow, raceID), -1) == rcrMultiSelectRow) {
			MultiSelectRaceDlg();
		}
		else if (nRow > 0) {
			m_naryRaceIDs.RemoveAll();
			m_naryRaceIDs.Add(VarLong(m_raceCombo->GetValue(nRow, raceID))); 
		}
		else {
			m_naryRaceIDs.RemoveAll();
		}

		// (b.spivey, May 28, 2013) - PLID 56872 - This code should work even if they only have one selection. 
		for (int i = 0; i < m_naryRaceIDs.GetCount(); i++) {
			sqlInsertRace += CSqlFragment("INSERT INTO PersonRaceT (RaceID, PersonID) VALUES ({INT}, @PersonID) ", 
												m_naryRaceIDs.GetAt(i));
		}

		//Due to the nature of the data structure now, the only easy way to get the names for auditing is to get the race list
		//	 like we would for putting it in the label. So I do that before and after the inserts for the old and new names respectively.
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON "
			"DECLARE @PersonID INT "
			"SET @PersonID = {INT} "
			" "
			" "
			"SELECT LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) AS OldRaceName "
			"FROM "
			"( "
			"	SELECT ( "
			"		SELECT RT.Name + ', ' "
			"		FROM PersonRaceT PRT "
			"		INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
			"		WHERE PRT.PersonID = @PersonID  "
			"		FOR XML PATH(''), TYPE "
			"	).value('/', 'nvarchar(max)')	"
			") RaceSubQ (RaceName) "
			" "
			" "
			"DELETE FROM PersonRaceT WHERE PersonID = @PersonID "
			" " 
			" "
			" {SQL} "
			" "
			" "
			"SELECT LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) AS NewRaceName "
			"FROM "
			"( "
			"	SELECT ( "
			"		SELECT RT.Name + ', ' "
			"		FROM PersonRaceT PRT "
			"		INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
			"		WHERE PRT.PersonID = @PersonID  "
			"		FOR XML PATH(''), TYPE "
			"	).value('/', 'nvarchar(max)')	"
			") RaceSubQ (RaceName) "
			"SET NOCOUNT OFF\r\n",
			m_id, sqlInsertRace);

		
		if(m_naryRaceIDs.GetCount() > 1) {
			m_raceCombo->SetSelByColumn(raceID, rcrMultiSelectRow); 
		}
		else if (m_naryRaceIDs.GetCount() == 1) {
			m_raceCombo->SetSelByColumn(raceID, m_naryRaceIDs.GetAt(0)); 
		}
		else if (m_naryRaceIDs.GetCount() <= 0) {
			m_raceCombo->CurSel = NXDATALISTLib::sriNoRow; 
		}

		
		//No records? Clearly nothing changed. 
		if(!prs->eof)
		{
			CString strOldValue = AdoFldString(prs, "OldRaceName", l_strNoRaceText);
			prs = prs->NextRecordset(NULL); 
			CString strNewValue = l_strNoRaceText; 
			//This could be empty, doesn't mean we shouldn't audit though.
			if(!prs->eof) {
				strNewValue = AdoFldString(prs, "NewRaceName", l_strNoRaceText);
			}

			//DRT 6/30/2005 - PLID 16654 - Auditing
			if(strOldValue.Compare(strNewValue) != 0) {
				long nAuditID = BeginNewAuditEvent();
				// (j.jones 2009-10-14 14:31) - PLID 34327 - renamed this audit
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientRace, m_id, strOldValue, strNewValue, aepMedium, aetChanged);
				UpdateChangedDate(nAuditID);
				UpdateExistingPatientInHL7(m_id);
			}
		}
		ToggleLabels(); 
	} NxCatchAll("Error changing patient race.");
}

// (a.walling 2009-05-25 10:27) - PLID 27859 - Language combo
void CGeneral2Dlg::OnSelChosenLanguageCombo(LPDISPATCH lpRow)
{
	try{
		// (j.armen 2012-06-07 14:51) - PLID 50825 - Refactored, Update HL7
		// (d.thompson 2012-08-13) - PLID 52046 - Reworked Language tables
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		_variant_t varLanguageID = g_cvarNull;
		if(pRow) {
			varLanguageID = pRow->GetValue(0);
		}
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON\r\n"
			"DECLARE @LanguageCode TABLE (OldID int, NewID int)\r\n"
			"UPDATE PersonT SET LanguageID = {VT_I4} "
			"	OUTPUT deleted.LanguageID, inserted.LanguageID INTO @LanguageCode "
			"	WHERE ID = {INT}\r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT\r\n"
			"	OldID, OldLanguageT.Name AS OldName, NewID, NewLanguageT.Name AS NewName\r\n"
			"FROM @LanguageCode LanguageCode\r\n"
			"LEFT JOIN LanguageT OldLanguageT ON LanguageCode.OldID = OldLanguageT.ID\r\n"
			"LEFT JOIN LanguageT NewLanguageT ON LanguageCode.NewID = NewLanguageT.ID\r\n",
			varLanguageID, m_id);
		
		if(!prs->eof) {
			CString strOldValue = AdoFldString(prs, "OldName", l_strNoLanguageText);
			CString strNewValue = AdoFldString(prs, "NewName", l_strNoLanguageText);

			//DRT 6/30/2005 - PLID 16654 - Auditing
			if(AdoFldLong(prs, "OldID", -1) != AdoFldLong(prs, "NewID", -1)) {
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientLanguage, m_id, strOldValue, strNewValue, aepMedium, aetChanged);
				UpdateChangedDate(nAuditID);
				UpdateExistingPatientInHL7(m_id);
			}
		}
	} NxCatchAll("Error changing patient language");
}


void CGeneral2Dlg::OnSelChosenDefaultLocationCombo(long nRow) 
{
	try{
		// (s.tullis 2016-05-25 15:41) - NX-100760 	
		// (j.armen 2012-06-07 14:51) - PLID 50825 - Refactored, Update HL7
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON\r\n"
			"DECLARE @LocationID TABLE (OldID INT, NewID INT)\r\n"
			"UPDATE PersonT SET Location = {VT_I4} "
			"	OUTPUT deleted.Location, inserted.Location INTO @LocationID "
			"	WHERE ID = {INT}\r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT\r\n"
			"	OldID, OldLocationT.Name AS OldName, NewID, NewLocationT.Name AS NewName\r\n"
			"FROM @LocationID LocationID\r\n"
			"LEFT JOIN LocationsT OldLocationT ON LocationID.OldID = OldLocationT.ID\r\n"
			"LEFT JOIN LocationsT NewLocationT ON LocationID.NewID = NewLocationT.ID\r\n",
			m_DefaultLocationCombo->GetValue(nRow, 0), m_id);

		if(!prs->eof)
		{
			CString strOldValue = AdoFldString(prs, "OldName", "<No Location Selected>");
			CString strNewValue = AdoFldString(prs, "NewName", "<No Location Selected>");

			if(AdoFldLong(prs, "OldID", -1) != AdoFldLong(prs, "NewID", -1))
			{
				//auditing
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientLocation, m_id, strOldValue, strNewValue, aepMedium, aetChanged);
				UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
				UpdateExistingPatientInHL7(m_id);
			}
		}
	}NxCatchAll("Error in changing location.");	
}

void CGeneral2Dlg::OnSelChosenReferringPhysicianCombo(long nRow) 
{
	CString str;
	try{
		//for auditing
		CString strOld;
		_RecordsetPtr rs = CreateRecordset("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name FROM PersonT WHERE ID = (SELECT DefaultReferringPhyID FROM PatientsT WHERE PersonID = %li)", m_id);
		if(!rs->eof && rs->Fields->Item["Name"]->Value.vt == VT_BSTR)
			strOld = CString(rs->Fields->Item["Name"]->Value.bstrVal);
		//
		if(m_ReferringPhyCombo->GetCurSel() == -1 || m_ReferringPhyCombo->GetValue(nRow,rpcID).vt==VT_NULL){
			str = "NULL";
			m_ReferringPhyCombo->CurSel = -1;
		}
		else
			str.Format("%li",m_ReferringPhyCombo->GetValue(nRow,rpcID).lVal);
		ExecuteSql("UPDATE PatientsT SET DefaultReferringPhyID = %s WHERE PersonID = %li",str,m_id);
		if(nRow != -1)
			SetDlgItemVar(IDC_ID_NUMBER, m_ReferringPhyCombo->GetValue(nRow, rpcRefPhysID));
		//auditing
		CString strNew;
		if(str == "NULL")
			strNew = "";
		else
			strNew = CString(m_ReferringPhyCombo->GetValue(nRow, rpcName).bstrVal);
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientRefPhys, m_id, strOld, strNew, aepMedium, aetChanged);
		UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
		// (r.gonet 01/23/2014) - PLID 60101 - The referring physician changed. Send a patient update message.
		UpdateExistingPatientInHL7(m_id);

		// (j.dinatale 2012-12-19 10:52) - PLID 54186 - need to fire a table checker here because the marketing module needs to refresh its horrible caching
		CClient::RefreshTable(NetUtils::ReferralSourceT);

		if(str != "NULL" && VarBool(m_ReferringPhyCombo->GetValue(nRow, rpcShowProcWarning))) {
			//OK, we need to list the procedures performed by this patient.
			CString strMessage = "This referring physician performs the following procedures:\r\n";
			_RecordsetPtr rsProcs = CreateRecordset("SELECT Name FROM ProcedureT INNER JOIN RefPhysProcLinkT ON ProcedureT.ID = RefPhysProcLinkT.ProcedureID "
				"WHERE RefPhysProcLinkT.RefPhysID = %s", str);
			while(!rsProcs->eof) {
				strMessage += AdoFldString(rsProcs, "Name") + "; ";
				rsProcs->MoveNext();
			}
			rsProcs->Close();
			if(strMessage.Right(2) == "; ") strMessage = strMessage.Left(strMessage.GetLength()-2);
			CString strExtraText = VarString(m_ReferringPhyCombo->GetValue(nRow, rpcProcWarning), "");
			if(!strExtraText.IsEmpty()) {
				strMessage += "\r\n" + strExtraText;
			}
			MsgBox(strMessage);
		}

		//PLID 20163 - added preference to select referral source based on referring physician
		// (b.eyers 8/18/2014) - PLID 63050 - don't add the default referral source if 'no provider selected' was choosen for referring physician
		if(strOld.IsEmpty() && strNew != "") {
			long nDefReferral = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);
			if (nDefReferral != -1) {
				//Just call the AddReferral function, it handles writing the data and all that good stuff.
				//However, we don't want to call this if we already have this referral source for this patient.  They've already got
				//the one we want selected, so just ignore it.
				if (!ReturnsRecords("SELECT * FROM MultiReferralsT WHERE ReferralID = %li AND PatientID = %li", nDefReferral, GetActivePatientID())) {
					// (r.goldschmidt 2014-09-19 10:28) - PLID 31191 - Check for possible conflict with parent referral preference
					//   If selection not allowed, throw a message box, no referral source gets added, DefaultPhysicianReferral preference remains intact.
					CString strWarning;
					if (!IsReferralSelectionAllowed(nDefReferral, strWarning)) {
						strWarning += " The default referring physician referral source conflicts with these preferences.\n\nPlease manually add the desired referral source and review preference settings.";
						AfxMessageBox(strWarning);
					}
					else {
						AddReferral(nDefReferral);
					}
				}
			}
		}

	}NxCatchAll("Error in changing referring physician.");
}

void CGeneral2Dlg::OnSelChosenPatientTypeCombo(long nRow) 
{
	CString str;
	try{
		//for auditing
		_RecordsetPtr rs = CreateRecordset("SELECT GroupName FROM GroupTypes WHERE TypeIndex = (SELECT TypeOfPatient FROM PatientsT WHERE PersonID = %li)", m_id);
		CString strOld;
		if(!rs->eof && rs->Fields->Item["GroupName"]->Value.vt == VT_BSTR)
			strOld = CString(rs->Fields->Item["GroupName"]->Value.bstrVal);
		//

		if(m_PatientTypeCombo->GetCurSel() == -1 || m_PatientTypeCombo->GetValue(nRow,0).vt==VT_NULL){
			str = "NULL";
			m_PatientTypeCombo->CurSel = -1;
		}
		else
			str.Format("%li",m_PatientTypeCombo->GetValue(nRow,0).lVal);
		ExecuteSql("UPDATE PatientsT SET TypeOfPatient = %s WHERE PersonID = %li",str,m_id);

		//auditing
		CString strNew;
		if(str == "NULL")
			strNew = "";
		else
			strNew = CString(m_PatientTypeCombo->GetValue(m_PatientTypeCombo->CurSel, 1).bstrVal);
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1) {
			AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientType, m_id, strOld, strNew, aepMedium, aetChanged);
			UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
		}
		
		// (b.spivey, May 15, 2012) - PLID 20752 - Update the color when they change patient types. 
		if(GetMainFrame()) {
			CPatientView* pView = (CPatientView*)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
			if (pView) {
				pView->SetColor();
			}
		}

	}NxCatchAll("Error in changing patient type.");
}

void CGeneral2Dlg::OnSelChosenPcpCombo(long nRow) 
{
	CString str;
	_variant_t var;

	try
	{
		// (b.cardillo 2011-08-19 16:33) - PLID 25886 - Fail gracefully if the selected referring physician no longer exists.
		// First get the ID of the selected row or NULL if no row is selected
		if (m_PCPCombo->GetCurSel() == -1 || m_PCPCombo->GetValue(nRow,0).vt==VT_NULL) {
			str = "NULL";
			m_PCPCombo->CurSel = -1;
		} else {
			// Get the ID
			long nNewID = m_PCPCombo->GetValue(nRow, 0);
			// If it's possible the ref phys doesn't exist anymore, check
			if (m_refphyChecker.PeekChanged() && !ReturnsRecordsParam("SELECT * FROM ReferringPhysT WHERE PersonID = {INT}", nNewID)) {
				// The ref phys no longer exists in data, so short-circuit after letting the user know
				MessageBox("The selected physician no longer exists in the database.  It may have been deleted or changed to a different contact type.  Please select a different physician.", NULL, MB_ICONEXCLAMATION);
				m_PCPCombo->RemoveRow(nRow);
				m_PCPCombo->PutCurSel(NXDATALISTLib::sriNoRow);
				return;
			}
			// Otherwise proceed as normal
			str.Format("%li", VarLong(nNewID));
		}

		//for auditing
		CString strOld, strOldID; // (b.eyers 2015-06-22) - PLID 66213 
		_RecordsetPtr rs = CreateRecordset("SELECT PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS Name, ID FROM PersonT WHERE ID = (SELECT PCP FROM PatientsT WHERE PersonID = %li)", m_id);
		if (!rs->eof && rs->Fields->Item["Name"]->Value.vt == VT_BSTR) {
			strOld = CString(rs->Fields->Item["Name"]->Value.bstrVal);
			// (b.eyers 2015-06-22) - PLID 66213 - what was the old pcp ID
			//nOldID = rs->Fields->Item["ID"]->Value;
			strOldID.Format("%li", VarLong(rs->Fields->Item["ID"]->Value));
		}
		ExecuteSql("UPDATE PatientsT SET PCP = %s WHERE PersonID = %li", str, m_id);
		//auditing
		long nAuditID = BeginNewAuditEvent();
		CString strNew = "";
		if(str != "NULL")
			strNew = CString(m_PCPCombo->GetValue(nRow, 1).bstrVal);
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientPCP, m_id, strOld, strNew, aepMedium, aetChanged);
		UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
		// (b.eyers 2015-06-19) - PLID 66213 - PCP changed, send patient update message
		if (strOldID == "" && str != "") //no past pcp, new pcp - AD
			UpdateExistingPatientInHL7(m_id, false, false, "", racAdded);
		else if (strOldID != "" && str == "NULL") //past pcp, no new pcp - DE
			UpdateExistingPatientInHL7(m_id, false, false, "", racDeleted);
		else if (strOldID.Compare(str) == 0) //pcp was not changed - UN
			UpdateExistingPatientInHL7(m_id, false, false, "", racUnchanged);
		else if (strOldID.Compare(str) != 0) //pcp changed - UP
			UpdateExistingPatientInHL7(m_id, false, false, "", racUpdated);
		else 
			UpdateExistingPatientInHL7(m_id);
		//
	}NxCatchAll("Error in changing primary care physician.");
}

void CGeneral2Dlg::OnSelChosenDonorPatientCombo(long nRow) 
{
  	try
	{
		if(m_donorCombo->GetValue(nRow,0).vt==VT_NULL){
			ExecuteSql("UPDATE PatientsT SET DonorID = NULL WHERE PersonID = %li", m_id);
			m_donorCombo->CurSel = -1;
		}
		else
			ExecuteSql("UPDATE PatientsT SET DonorID = %li WHERE PersonID = %li", m_donorCombo->GetValue(nRow,0).lVal, m_id);
	}
	NxCatchAll("Could not save donor.");
}

void CGeneral2Dlg::OnAddReferral() 
{
	AddReferral();
}

void CGeneral2Dlg::AddReferral(long nReferralID /* = -1 */)
{
	//(a.wilson 2012-5-23) PLID 48537 - check write with pass before continuing. Also encapsulate entire function in try/catch.
	try {
		CPermissions permReferrals = GetCurrentUserPermissions(bioPatientReferralSources);
		BOOL bWritePerm = (permReferrals & (sptWrite));
		BOOL bWritePermWithPass = (permReferrals & (sptWriteWithPass));
		BOOL bPrimaryPermOrWithPass = (permReferrals & (sptDynamic0 | sptDynamic0WithPass));

		if (!bWritePerm && bWritePermWithPass) {
			if (!CheckCurrentUserPassword()) {
				return;
			}
		}

		//prompt them with the referral selection box
		CReferralTreeDlg dlg(this);
		long result = -1;

		//DRT 9/24/03 - If we passed in a referral id, just add it - this is generally used by the preference when you are adding a new
		//		patient referral.  It follows whatever preferences are set for the primary stuff.
		if(nReferralID == -1) {
			result = dlg.DoModal();	//returns the id selected
		}
		else {
			result = nReferralID;
		}

		if(result > 0){	//need to put the new one in MultiReferralsT
			//first thing to check, we don't want the same source multiple times
			_RecordsetPtr rs = CreateRecordset("SELECT ReferralID FROM MultiReferralsT WHERE ReferralID = %li AND PatientID = %li", result, m_id);
			if(rs->eof){
				COleDateTime dt = COleDateTime::GetCurrentTime();
				CString str;
				str = FormatDateTimeForSql(dt, dtoDate);

				BOOL bNewReferral = IsRecordsetEmpty("SELECT ReferralID FROM MultiReferralsT WHERE PatientID = %li",m_id);

				ExecuteSql("INSERT INTO MultiReferralsT (ReferralID, PatientID, Date) (SELECT %li, %li, '%s')", result, m_id, str);

				//this preference lets them leave existing referrals as primary
				//1 - make new referral primary (default), 2 - leave existing as primary
				long nPrimaryReferralPref = GetRemotePropertyInt("PrimaryReferralPref",1,0,"<None>",TRUE);

				//(a.wilson 2012-5-23) PLID 48537 - if they have perm or with pass then honor the preference.
				//now "select" it as the primary referral source
				if((bNewReferral || nPrimaryReferralPref == 1) && bPrimaryPermOrWithPass) {
					ExecuteSql("UPDATE PatientsT SET ReferralID = %li WHERE PersonID = %li", result, m_id);
					m_nReferralID = result;
					//(j.camacho 2016-02-11) PLID 68228 - Only update when added referrals are made primary
					UpdateExistingPatientInHL7(m_id);
				}
			}
			else {
				MsgBox("You cannot add a referral source more than once per patient.");
			}
		}
		RefreshReferrals();

		// Only audit if we the user selected a referral to add
		if (result > 0) {
			//auditing
			CString strNew;
			_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ReferralSourceT WHERE PersonID = %li",result);
			if(!rs->eof) {
				strNew = AdoFldString(rs, "Name","");
			}
			rs->Close();
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1) {
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientReferralAdd, m_id, "", strNew, aepMedium, aetChanged);
				UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate
			}
			//
		}
	} NxCatchAll("Error adding referral source");

	GetDlgItem(IDC_OCCUPATION)->SetFocus();
}

void CGeneral2Dlg::OnRemoveReferral() 
{
	if(m_referralSourceList->CurSel == -1)
		return;

	// (a.wilson 2012-5-23) PLID 48537 - check write permission with pass.
	CPermissions permReferrals = GetCurrentUserPermissions(bioPatientReferralSources);
	BOOL bWritePerm = (permReferrals & (sptWrite));
	BOOL bWritePermWithPass = (permReferrals & (sptWriteWithPass));
	BOOL bPrimaryPerm = (permReferrals & (sptDynamic0));
	BOOL bPrimaryPermWithPass = (permReferrals & (sptDynamic0WithPass));

	if (!bWritePerm && bWritePermWithPass) {
		if (!CheckCurrentUserPassword()) {
			return;
		}
	}

	COleVariant var;
	var = m_referralSourceList->GetValue(m_referralSourceList->CurSel, 0);	//get the referralID
	// (j.luckoski 2013-01-23 16:25) - PLID 54800 - Make a long to hold our referral ID since we use it more than once
	long nRefID = VarLong(var);

	//for auditing
	CString strOldRef = CString(m_referralSourceList->GetValue(m_referralSourceList->CurSel, 1).bstrVal);
	bool bChangedPrimary = false;
	CString strNewPrimary;
	//
	// (a.wilson 2012-5-23) PLID 48537 - check if they have permission to edit primary, 
	//also move the delete down and parameterize the queries.
	try {
		_RecordsetPtr rs = CreateParamRecordset("SELECT ReferralID FROM PatientsT WHERE PersonID = {INT}", m_id);

		if(nRefID == AdoFldLong(rs, "ReferralID", -1)) {	
			//we're deleting the primary source, so, if there are any others, let's set the newest one as primary
			if (!bPrimaryPerm && !bPrimaryPermWithPass) {
				PermissionsFailedMessageBox("", "change the patient's primary referral source");
				return;
			} else if (!bPrimaryPerm && bPrimaryPermWithPass && bWritePerm) {
				if (!CheckCurrentUserPassword())
					return;
			}

			bChangedPrimary = true;
			rs->Close();
			// (j.luckoski 2013-01-23 16:26) - PLID 54800 - Exclude the referralID that we are about to delete
			rs = CreateParamRecordset("SELECT TOP 1 ReferralID, ReferralSourceT.Name, Date FROM MultiReferralsT "
				"LEFT JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourcet.PersonID WHERE PatientID = {INT} AND ReferralID <> {INT} ORDER BY Date DESC", m_id, nRefID);

			if(!rs->eof) {
				m_nReferralID = AdoFldLong(rs, "ReferralID");
				strNewPrimary = AdoFldString(rs, "Name", "");
				ExecuteParamSql("UPDATE PatientsT SET ReferralID = {INT} WHERE PersonID = {INT}", m_nReferralID, m_id);
				
			}
			else {			//there are no other records, we'll just update this to nothing
				ExecuteParamSql("UPDATE PatientsT SET ReferralID = NULL WHERE PersonID = {INT}", m_id);
				m_nReferralID = -1;
				strNewPrimary = "<No Primary Source>";
			}
			//(j.camacho 2016-02-11) PLID 68228 - only update if we removed primary
			UpdateExistingPatientInHL7(m_id);
		}
		//delete the record from MultiReferralsT, still in PatientsT at this time
		ExecuteParamSql("DELETE FROM MultiReferralsT WHERE ReferralID = {INT} AND PatientID = {INT}", nRefID, m_id);

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1) {
			AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientReferralDelete, m_id, strOldRef, "", aepMedium, aetChanged);
			UpdateChangedDate(nAuditID); //required to show the most current ModifiedDate

			//DRT 7/1/2005 - PLID 16654 - Also need to audit that the default referral was changed.
			if(bChangedPrimary) {
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientPriReferral, m_id, strOldRef, strNewPrimary, aepMedium, aetChanged);
			}
		}

		RefreshReferrals();		
	} NxCatchAll("Error removing referral source");
	GetDlgItem(IDC_OCCUPATION)->SetFocus();
}

void CGeneral2Dlg::OnMakePrimaryReferral() 
{
	try{
		if(m_referralSourceList->CurSel == -1)
			return;

		// (a.wilson 2012-5-23) PLID 48537 - check if they have marke primary permission with pass.
		BOOL bPrimaryPerm = (GetCurrentUserPermissions(bioPatientReferralSources) & (sptDynamic0));
		BOOL bPrimaryPemWithPass = (GetCurrentUserPermissions(bioPatientReferralSources) & (sptDynamic0WithPass));

		if (!bPrimaryPerm && bPrimaryPemWithPass) {
			if (!CheckCurrentUserPassword()) {
				return;
			}
		}

		//for auditing
		CString strOld;
		_RecordsetPtr prsAudit = CreateRecordset("SELECT Name FROM PatientsT INNER JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID WHERE PatientsT.PersonID = %li", m_id);
		if(!prsAudit->eof)
			strOld = AdoFldString(prsAudit, "Name", "");
		//

		COleVariant var;
		var = m_referralSourceList->GetValue(m_referralSourceList->CurSel, 0);	//get the referralID

		ExecuteSql("UPDATE PatientsT SET ReferralID = %li WHERE PersonID = %li", VarLong(var), m_id);
		m_nReferralID = VarLong(var);

		//DRT 6/30/2005 - PLID 16654 - Auditing
		CString strNew = VarString(m_referralSourceList->GetValue(m_referralSourceList->GetCurSel(), 1), "");
		if(strOld != strNew) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientPriReferral, m_id, strOld, strNew, aepMedium, aetChanged);
		}
		//(j.camacho 2016-02-11) PLID 68228
		UpdateExistingPatientInHL7(m_id);

		RefreshReferrals();

	} NxCatchAll("Error setting Primary Referral");
	GetDlgItem(IDC_OCCUPATION)->SetFocus();
}

void CGeneral2Dlg::RefreshReferrals()
{	//this takes care of refreshing the referral list,
	//OnRequeryFinished will select the primary source
	if ((GetCurrentUserPermissions(bioPatientReferralSources) & (sptRead))) {
		m_referralSourceList->Requery();
	} else {
		m_referralSourceList->Clear();
		NXDATALISTLib::IRowSettingsPtr pRow = m_referralSourceList->GetRow(NXDATALISTLib::sriGetNewRow);
		pRow->PutValue(rscID, -1);
		pRow->PutValue(rscReferralName, _bstr_t("<Hidden>"));
		pRow->PutValue(rscDate, g_cvarNull);
		m_referralSourceList->AddRow(pRow);
		OnSelChangedReferralList(-1);
	}
}

void CGeneral2Dlg::OnEditingFinishingReferralList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if(*pbCommit == FALSE)
		return;

	switch (nCol) {
	//Date
	case 2:
		//If this isn't a date, is an invalid date, or has been converted to 12/30/1899 and therefore is some crazy thing.
		if(pvarNewValue->vt != VT_DATE || VarDateTime(*pvarNewValue).GetStatus() != COleDateTime::valid || VarDateTime(*pvarNewValue).GetYear() < 1900) {
			MsgBox("The text you entered does not correspond to a valid date. \n Your changes will not be saved");
			*pbCommit = FALSE;
		}
		break;

	}
}

void CGeneral2Dlg::OnEditingFinishedReferralList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	switch(nCol)
	{
		case 2:
			if(bCommit){
				try {
					ExecuteSql("UPDATE MultiReferralsT SET Date = '%s' WHERE ReferralID = %li AND PatientID = %li", _Q(FormatDateTimeForSql(VarDateTime(varNewValue), dtoDate)), VarLong(m_referralSourceList->GetValue(nRow, 0)), m_id);

					//DRT 6/30/2005 - PLID 16654 - Auditing
					if(VarDateTime(varOldValue) != VarDateTime(varNewValue)) {
						CString strReferralName = VarString(m_referralSourceList->GetValue(nRow, 1), "");

						long nAuditID = BeginNewAuditEvent();
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientReferralDate, m_id, "(" + strReferralName + ") " + _Q(FormatDateTimeForInterface(VarDateTime(varOldValue), dtoDate)), 
							 "(" + strReferralName + ") " + _Q(FormatDateTimeForInterface(VarDateTime(varNewValue), dtoDate)), aepMedium, aetChanged);
					}
				} NxCatchAll("Error editing referral source: ");
			}
		break;
	}

}

// (r.goldschmidt 2014-09-19 12:28) - PLID 31191 - Check for possible conflict with parent referral preference, AllowParentReferral
//  If return false, make sure strWarning describes why.
bool CGeneral2Dlg::IsReferralSelectionAllowed(long nReferralID, CString& strWarning)
{
	try{
		long nPreferenceSetting = GetRemotePropertyInt("AllowParentReferrals", 0, 0, "<None>", true);
		long nRestrictionLevel = nPreferenceSetting;
		strWarning = "Referral source selection error."; // just in case

		// only check if there are restrictions
		if (nPreferenceSetting != 0){

			_RecordsetPtr prs = CreateParamRecordset(R"(
SELECT ReferralSourceT.PersonID, ReferralSourceT.Name, ReferralSourceT.Parent,
	COALESCE(ChildCounter.ChildrenCount, 0) AS ActiveChildrenCount
FROM ReferralSourceT
LEFT JOIN (
	SELECT Parent, COUNT(Parent) AS ChildrenCount
	FROM ReferralSourceT
	INNER JOIN PersonT ON PersonT.ID = ReferralSourceT.PersonID
	WHERE PersonT.Archived = 0
	GROUP BY Parent
	) AS ChildCounter ON ChildCounter.Parent = ReferralSourceT.PersonID
WHERE ReferralSourceT.PersonID = {INT}
ORDER BY Parent, PersonID ASC
				)", nReferralID);

			bool HasActiveChildren = (AdoFldLong(prs, "ActiveChildrenCount", 0) > 0);
			bool IsTopLevel = (AdoFldLong(prs, "Parent") == -1);

			prs->Close();

			// if there are no children, selection is allowed
			if (!HasActiveChildren){
				nRestrictionLevel = 0;
			}
			// if it isn't top level and the preference allows parents, selection is allowed
			else if (nPreferenceSetting == 1 && !IsTopLevel){
				nRestrictionLevel = 0;
			}
			// else, the selection is restricted according to preference

		}

		switch (nRestrictionLevel){
		case 0: // selection is allowed
			return true;
		case 1: // illegal selection according to AllowParentReferrals preference (1)
			strWarning = "Current referral source preferences do not allow selecting a top level parent that has active children.";
			return false;
		case 2: // illegal selection according to AllowParentReferrals preference (2)
			strWarning = "Current referral source preferences do not allow selecting a parent that has active children.";
			return false;
		default:
			ASSERT(FALSE); // unknown preference
			return false;
		}
	}NxCatchAll(__FUNCTION__);
	return false;
}

void CGeneral2Dlg::OnReferredPats() 
{
	CReferredPatients dlg(this);
	dlg.m_nPersonID = GetActivePatientID(); // (m.hancock 2006-08-02 16:15) - PLID 21752 - Changed dialog so it uses PersonID instead of PatientID
	dlg.m_nStatus = 1;
	dlg.DoModal();
	GetDlgItem(IDC_OCCUPATION)->SetFocus();
}

void CGeneral2Dlg::OnReferredProspects() 
{
	CReferredPatients dlg(this);
	dlg.m_nPersonID = GetActivePatientID(); // (m.hancock 2006-08-02 16:15) - PLID 21752 - Changed dialog so it uses PersonID instead of PatientID
	dlg.m_nStatus = 2;
	dlg.DoModal();
	GetDlgItem(IDC_OCCUPATION)->SetFocus();
}

void CGeneral2Dlg::OnSelChangedReferralList(long nNewSel) 
{
	//If we currently have a row selected, and it's not the primary row, enable the "Make Primary" button.
	//Otherwise, disable it.

	// (a.wilson 2012-5-23) PLID 48537 - consider referral sources permissions.
	CPermissions permReferrals = GetCurrentUserPermissions(bioPatientReferralSources);
	if(nNewSel != -1) {
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading General2
		if(ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT PersonID FROM PatientsT WHERE PersonID = {INT} AND ReferralID = {INT}", m_id, VarLong(m_referralSourceList->GetValue(nNewSel, 0)))) {
			GetDlgItem(IDC_MAKE_PRIMARY_REFERRAL)->EnableWindow(FALSE);
		}
		else if ((permReferrals & (sptDynamic0 | sptDynamic0WithPass)) && (permReferrals & (sptRead))) {
			GetDlgItem(IDC_MAKE_PRIMARY_REFERRAL)->EnableWindow(TRUE);
		}
		if ((permReferrals & (sptWrite | sptWriteWithPass)))
			GetDlgItem(IDC_REMOVE_REFERRAL)->EnableWindow(TRUE);
		else
			GetDlgItem(IDC_REMOVE_REFERRAL)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_MAKE_PRIMARY_REFERRAL)->EnableWindow(FALSE);
		GetDlgItem(IDC_REMOVE_REFERRAL)->EnableWindow(FALSE);
	}
}

void CGeneral2Dlg::SecureControls()
{
	CWnd* pWnd;
	int i;

	if (!(GetCurrentUserPermissions(bioPatientWarning) & SPT___W________ANDPASS))
	{
		GetDlgItem(IDC_WARNING_CHECK)->EnableWindow(FALSE);
		((CNxEdit*)GetDlgItem(IDC_WARNING))->SetReadOnly(TRUE);
		// (a.walling 2010-06-30 17:43) - PLID 18081 - Warning categories
		GetDlgItem(IDC_LIST_WARNING_CATEGORIES)->EnableWindow(FALSE);
	}

	// (j.gruber 2009-12-01 09:00) - PLID 36455 - changed to new licensing
	//(e.lally 2011-08-09) PLID 37287 - Added permissions for NexWeb logins
	if(!g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrSilent) 
		|| !(GetCurrentUserPermissions(bioPatientNexWebLogin) & (sptRead|sptReadWithPass)) ) {
		GetDlgItem(IDC_EDIT_NEXWEB_INFO)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_EDIT_NEXWEB_INFO)->EnableWindow(TRUE);
	}

	//TES 5/30/2008 - PLID 29999 - Disable the New Allocation button if they don't have access.
	if(!(GetCurrentUserPermissions(bioInventoryAllocation) & SPT____C_______ANDPASS)) {
		GetDlgItem(IDC_ADD_ALLOCATION)->EnableWindow(FALSE);
	}

	//TES 4/16/2012 - PLID 48740 - Added a permission specific to the referring physician field
	if(!(GetCurrentUserPermissions(bioPatientReferringPhysician) & SPT___W________ANDPASS)) {
		GetDlgItem(IDC_REFERRING_PHYSICIAN_COMBO)->EnableWindow(FALSE);
	}

	// Return if we have general write access (TS: There's no "with pass" at the moment)
	if (GetCurrentUserPermissions(bioPatient) & SPT___W________ANDPASS)
		return;

	// No write access. Traverse the controls to disable all edit boxes
	for (i=0, pWnd = GetWindow(GW_CHILD); pWnd; i++, pWnd = pWnd->GetWindow(GW_HWNDNEXT)) 
	{
		if (IsEditBox(pWnd))
		{
			((CNxEdit*)pWnd)->SetReadOnly(TRUE);
		}
	}

	// Disable date
	GetDlgItem(IDC_ACCIDENT_DATE)->EnableWindow(FALSE);

	// (j.jones 2014-02-19 16:02) - PLID 60719 - disable the diag search
	m_diagSearch->Enabled = g_cvarFalse;

	// Disable dropdowns
	GetDlgItem(IDC_REFERRING_PATIENT_COMBO)->EnableWindow(FALSE);
	GetDlgItem(IDC_DEFAULT_LOCATION_COMBO)->EnableWindow(FALSE);
	GetDlgItem(IDC_PATIENT_TYPE_COMBO)->EnableWindow(FALSE);
	GetDlgItem(IDC_PCP_COMBO)->EnableWindow(FALSE);
	GetDlgItem(IDC_DONOR_PATIENT_COMBO)->EnableWindow(FALSE);
	GetDlgItem(IDC_REFERRING_PHYSICIAN_COMBO)->EnableWindow(FALSE);
	// (c.haag 2006-03-28 15:30) - PLID 19904 - Don't forget ethnicity!
	GetDlgItem(IDC_CODE_RACE)->EnableWindow(FALSE);
	// (a.walling 2009-05-25 10:26) - PLID 27859 - Language combo
	GetDlgItem(IDC_LANGUAGE_COMBO)->EnableWindow(FALSE);
	// (b.eyers 2013-03-26) - PLID 53984 - Ethnicity, affiliate physician, related to read only permission
	GetDlgItem(IDC_ETHNICITY_COMBO)->EnableWindow(FALSE);
	GetDlgItem(IDC_AFFILIATE_PHYS_LIST)->EnableWindow(FALSE);
	GetDlgItem(IDC_RELATION)->EnableWindow(FALSE);
	GetDlgItem(IDC_AFF_PHYS_TYPE)->EnableWindow(FALSE);

	// Disable lists
	GetDlgItem(IDC_REFERRAL_LIST)->EnableWindow(FALSE);

	// Disable radio buttons and checkboxes
	GetDlgItem(IDC_FULLTIME_RADIO)->EnableWindow(FALSE);
	GetDlgItem(IDC_FULLTIMESTUDENT_RADIO)->EnableWindow(FALSE);
	GetDlgItem(IDC_PARTTIMESTUDENT_RADIO)->EnableWindow(FALSE);
	GetDlgItem(IDC_PARTTIME_RADIO)->EnableWindow(FALSE);
	GetDlgItem(IDC_RETIRED_RADIO)->EnableWindow(FALSE);
	GetDlgItem(IDC_OTHER_RADIO)->EnableWindow(FALSE);
	GetDlgItem(IDC_WARNING_CHECK)->EnableWindow(FALSE);

	// Disable buttons
	GetDlgItem(IDC_ADD_REFERRAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_REMOVE_REFERRAL)->EnableWindow(FALSE);
	GetDlgItem(IDC_COPY_EMPLOYER_BTN)->EnableWindow(FALSE);
}

BOOL CGeneral2Dlg::IsEditBox(CWnd* pWnd)
{
	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_OCCUPATION:
		case IDC_MANAGER_FNAME:
		case IDC_MANAGER_MNAME:
		case IDC_MANAGER_LNAME:
		case IDC_EMP_ADDRESS1:
		case IDC_EMP_ADDRESS2:
		case IDC_EMP_CITY:
		case IDC_EMP_STATE:
		case IDC_COMPANY:
		case IDC_ACCIDENT_DATE:
		case IDC_EMP_ZIP:
		case IDC_WARNING:
			return TRUE;
	}
	return FALSE;
}

void CGeneral2Dlg::OnKillfocusWarning() 
{

}

//DRT 4/28/02 - Copy another employer's information to this patient
void CGeneral2Dlg::OnCopyEmployerBtn() 
{
	CCopyEmployerDlg dlg(this);
	if(dlg.DoModal() == IDOK) {
		//if they have something there already, warn before overwriting
		CString str;
		GetDlgItemText(IDC_COMPANY, str);
		if(!str.IsEmpty()) {
			if(MsgBox(MB_YESNO, "You already have company information for this patient.  Are you sure you wish to overwrite it?") == IDNO)
				return;
		}

		SetDlgItemText(IDC_COMPANY, dlg.m_company);
		SetDlgItemText(IDC_EMP_ADDRESS1, dlg.m_addr1);
		SetDlgItemText(IDC_EMP_ADDRESS2, dlg.m_addr2);
		SetDlgItemText(IDC_EMP_CITY, dlg.m_city);
		SetDlgItemText(IDC_EMP_STATE, dlg.m_state);
		SetDlgItemText(IDC_EMP_ZIP, dlg.m_zip);

		//rather than calling Save() 6 times, just write 2 update statements (Company is in PersonT)

		long nAuditTransactionID = -1;

		try {

			long nPatientID = GetActivePatientID();
			CString strPatientName = GetExistingPatientName(nPatientID);

			// (j.jones 2008-05-27 11:12) - PLID 27375 - added auditing, which will check against data,
			// not against what was previously on the screen
			CString strOldAddress1, strOldAddress2, strOldCity, strOldState, strOldZip, strOldCompany;

			_RecordsetPtr rs = CreateParamRecordset("SELECT EmployerAddress1, EmployerAddress2, EmployerCity, "
				"EmployerState, EmployerZip, Company "
				"FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"WHERE PersonID = {INT}", nPatientID);
			if(!rs->eof) {
				strOldAddress1 = AdoFldString(rs, "EmployerAddress1", "");
				strOldAddress2 = AdoFldString(rs, "EmployerAddress2", "");
				strOldCity = AdoFldString(rs, "EmployerCity", "");
				strOldState = AdoFldString(rs, "EmployerState", "");
				strOldZip = AdoFldString(rs, "EmployerZip", "");
				strOldCompany = AdoFldString(rs, "Company", "");
			}
			rs->Close();
			
			// (j.jones 2008-05-27 11:21) - PLID 27375 - converted into a batch
			CString strSqlBatch;
			AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientsT SET EmployerAddress1 = '%s', EmployerAddress2 = '%s', "
				"EmployerCity = '%s', EmployerState = '%s', EmployerZip = '%s' "
				"WHERE PersonID = %li", _Q(dlg.m_addr1), _Q(dlg.m_addr2), _Q(dlg.m_city), _Q(dlg.m_state), _Q(dlg.m_zip), nPatientID);
			AddStatementToSqlBatch(strSqlBatch, "UPDATE PersonT SET Company = '%s' WHERE ID = %li", _Q(dlg.m_company), nPatientID);
			ExecuteSqlBatch(strSqlBatch);

			if(strOldAddress1 != dlg.m_addr1) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiPatientEmployerAddress1, nPatientID, strOldAddress1, dlg.m_addr1, aepMedium, aetChanged);
			}
			if(strOldAddress2 != dlg.m_addr2) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiPatientEmployerAddress2, nPatientID, strOldAddress2, dlg.m_addr2, aepMedium, aetChanged);
			}
			if(strOldCity != dlg.m_city) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiPatientEmployerCity, nPatientID, strOldCity, dlg.m_city, aepMedium, aetChanged);
			}
			if(strOldState != dlg.m_state) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiPatientEmployerState, nPatientID, strOldState, dlg.m_state, aepMedium, aetChanged);
			}
			if(strOldZip != dlg.m_zip) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiPatientEmployerZip, nPatientID, strOldZip, dlg.m_zip, aepMedium, aetChanged);
			}
			if(strOldCompany != dlg.m_company) {
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				AuditEvent(nPatientID, strPatientName, nAuditTransactionID, aeiPatientCompany, nPatientID, strOldCompany, dlg.m_company, aepMedium, aetChanged);
			}

			if(nAuditTransactionID != -1) {
				CommitAuditTransaction(nAuditTransactionID);
			}

			//m.hancock - 5/8/2006 - PLID 20462 - Update the patient toolbar to reflect the new company
			//TES 1/6/2010 - PLID 36761 - New function for updating the datalist.
			GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(GetMainFrame()->m_patToolBar.GetCompanyColumn(),_bstr_t(dlg.m_company));
			
		} NxCatchAllCall("Error updating employer information.",
			if(nAuditTransactionID != -1) {
				RollbackAuditTransaction(nAuditTransactionID);
			}
		);
	}
	GetDlgItem(IDC_OCCUPATION)->SetFocus();
}

void CGeneral2Dlg::OnTrySetSelFinishedDefaultLocationCombo(long nRowEnum, long nFlags) 
{
	if(nFlags == NXDATALISTLib::dlTrySetSelFinishedFailure) {
		//OK, they must have an inactive location selected.
		_RecordsetPtr rsLocation = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = (SELECT Location FROM PersonT WHERE ID = %li)", GetActivePatientID());
		if(!rsLocation->eof) {
			m_DefaultLocationCombo->PutComboBoxText(_bstr_t(AdoFldString(rsLocation, "Name", "")));
		}
	}
	
}

void CGeneral2Dlg::OnKillFocusAccidentDate() 
{
	// (e.lally 2007-01-31) - PLID 24518 - Like b.cardillo said in other places:
	// "Unless we explicitly flag something as 
	// having changed, the call to Save() will have no effect, and so nothing will be written 
	// to data.  So we just set the flag to true before calling save, since we know the user 
	// just changed something."
	
	//Check that the date is different than our member variable. It is critical that the member
	//variable is up to date.
	//DRT 10/6/2008 - PLID 31597 - We cannot compare DATE against COleDateTime anymore if one can be invalid
	COleDateTime dtCurrent(m_nxtAccidentDate->GetDateTime());
	if(m_nxtAccidentDate->GetStatus() == 3) {
		//the accident date is empty, so our datetime needs to be set to invalid, not valid and 1/1/1900.
		dtCurrent.SetStatus(COleDateTime::invalid);
	}
	//invalid != invalid, so we need to check specifically
	if(dtCurrent.GetStatus() != COleDateTime::invalid || m_dtAccident.GetStatus() != COleDateTime::invalid) {
		//Now at least 1 of them is valid
		if(dtCurrent != m_dtAccident) {
			m_changed = true;
			Save(IDC_ACCIDENT_DATE);
		}
	}
}

void CGeneral2Dlg::OnRButtonUpReferralList(long nRow, short nCol, long x, long y, long nFlags) 
{
	m_referralSourceList->CurSel = nRow;
	OnSelChangedReferralList(nRow);

	//(a.wilson 2012-5-23) PLID 48537 - add permissions for referral sources.
	CPermissions permReferrals = GetCurrentUserPermissions(bioPatientReferralSources);

	if (!(permReferrals & (sptRead)))
		return;
	
	CMenu menPopup;
	menPopup.m_hMenu = CreatePopupMenu();

	menPopup.InsertMenu(1, MF_BYPOSITION | ((permReferrals & (sptWrite | sptWriteWithPass)) ? MF_ENABLED : MF_DISABLED | MF_GRAYED), ID_ADD, "Add");
	if (nRow != -1) {
		menPopup.InsertMenu(2, MF_BYPOSITION | ((permReferrals & (sptWrite | sptWriteWithPass)) ? MF_ENABLED : MF_DISABLED | MF_GRAYED), ID_REMOVE, "Remove");

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		BOOL bEnabled = !ReturnsRecordsParam("SELECT PersonID FROM PatientsT WHERE PersonID = {INT} AND ReferralID = {INT}", m_id, VarLong(m_referralSourceList->GetValue(nRow, 0)));

		menPopup.InsertMenu(3, MF_BYPOSITION | ((bEnabled && (permReferrals & (sptDynamic0 | sptDynamic0WithPass))) ? MF_ENABLED : MF_DISABLED | MF_GRAYED), ID_MAKE_PRIMARY, "Make Primary");
	}

	CPoint pt(x,y);
	CWnd* pWnd = GetDlgItem(IDC_REFERRAL_LIST);
	if (pWnd != NULL)
	{	pWnd->ClientToScreen(&pt);
		menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
	}
	else HandleException(NULL, "An error ocurred while creating menu");
}

// Returns -1 if no row selected
// Throws exceptions if the value in the specified column of the current row is not a VT_I4
long GetLongFromCurRow(NXDATALISTLib::_DNxDataList *lpdl, short nCol)
{
	NXDATALISTLib::_DNxDataListPtr pdl(lpdl);
	long nCurSel = pdl->GetCurSel();
	if (nCurSel != -1) {
		return VarLong(pdl->GetValue(nCurSel, nCol));
	} else {
		return -1;
	}
}

long CGeneral2Dlg::AddNewRefPhys()
{
	bool bTableCheckerChanged = m_refphyChecker.m_changed;
	// by the time this variable receives the message that this table has changed, we've already done our refresh so
	// it think it doesn't need to update the ref phys list, setting this variable to true ensures that we will refresh
	// the lists of one is added, if a new contact is not added, then we reset the variable to whatever it was when
	// originally
	m_refphyChecker.m_changed = true;
	
	// this will create a contact and return the ID of the contact that was added
	long nNewRefPhysID = GetMainFrame()->AddContact(GetMainFrame()->dctRefPhys);
	if(nNewRefPhysID == -1){
		m_refphyChecker.m_changed = bTableCheckerChanged;
	}
	
	// MSC - 3/24/04 - We need to see if the new contact is actually a referring physician.
	// If they aren't, then return a -1
	// Otherwise, return the ID of the new referring physician
	if(IsRecordsetEmpty("SELECT * FROM ReferringPhysT WHERE PersonID = %li", nNewRefPhysID)){
		return -1;
	}
	else{
		return nNewRefPhysID;
	}
}

void CGeneral2Dlg::OnNewRefPhys()
{
	// get the current referring physician ID if there is one
	long nCurRefID = GetLongFromCurRow(m_ReferringPhyCombo, rpcID);

	long nNewRefPhysID = AddNewRefPhys();
	if(nNewRefPhysID != -1 && nCurRefID == -1){
		// a new contact was successfully added and they didn't have a referring physician selected before
		// so set it to the new ref phys
		OnSelChosenReferringPhysicianCombo(m_ReferringPhyCombo->SetSelByColumn(rpcID, nNewRefPhysID));
	}
	//TES 3/25/2004: Don't set focus if they said Save and Edit Further (and are therefore in the Contacts module.
	if(GetMainFrame()->GetActiveView() && GetMainFrame()->GetActiveView()->GetSafeHwnd() == GetParent()->GetSafeHwnd())
		GetDlgItem(IDC_REFERRING_PHYSICIAN_COMBO)->SetFocus();
}

void CGeneral2Dlg::OnNewPcp() 
{
	// get the current Primary Physician ID if there is one
	long nCurPcpID = GetLongFromCurRow(m_PCPCombo, 0);

	long nNewRefPhysID = AddNewRefPhys();
	if(nNewRefPhysID != -1 && nCurPcpID == -1){
		// a new contact was successfully added and they didn't have a primary care physician selected before
		// so set it to the new primary care phys
		OnSelChosenPcpCombo(m_PCPCombo->SetSelByColumn(0, nNewRefPhysID));
	}
	//TES 3/25/2004: Don't set focus if they said Save and Edit Further (and are therefore in the Contacts module.
	// (j.jones 2012-08-08 10:11) - PLID 51063 - added check that the active view is non-null
	if(GetMainFrame()->GetActiveView() && GetMainFrame()->GetActiveView()->GetSafeHwnd() == GetParent()->GetSafeHwnd())
		GetDlgItem(IDC_PCP_COMBO)->SetFocus();
}

void CGeneral2Dlg::OnExpireDate() 
{
	// (b.cardillo 2006-07-19 15:51) - PLID 21518 - Unless we explicitly flag something as 
	// having changed, the call to Save() will have no effect, and so nothing will be written 
	// to data.  So we just set the flag to true before calling save, since we know the user 
	// just changed something.
	m_changed = true;

	if (IsDlgButtonChecked(IDC_EXPIRE_DATE)) {

		//enable the date picker
		GetDlgItem(IDC_EXPIRE_DATE_PICKER)->EnableWindow(TRUE);
		Save(IDC_EXPIRE_DATE);
	}
	else {
		GetDlgItem(IDC_EXPIRE_DATE_PICKER)->EnableWindow(FALSE);
		Save(IDC_EXPIRE_DATE);
	}
		
}

void CGeneral2Dlg::OnChangeExpireDatePicker(NMHDR* pNMHDR, LRESULT* pResult)
{
	// (b.cardillo 2006-07-19 15:51) - PLID 21518 - Unless we explicitly flag something as 
	// having changed, the call to Save() will have no effect, and so nothing will be written 
	// to data.  So we just set the flag to true before calling save, since we know the user 
	// just changed something.
	m_changed = true;

	Save(IDC_EXPIRE_DATE_PICKER);

	*pResult = 0;
	
}

void CGeneral2Dlg::OnChangeCompany() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CNxDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.



	//DRT 1/28/2004 - PLID 10244 - Make the company name auto-fill itself based on other companies found in the database.

	try {
		if(!m_bSettingCompany) {

			//The item has now changed.
			// (m.cable 6/28/2004 09:52) - PLID 13213 - if this member variable wasn't set to true,
			// it won't save the company field if it's changed
				m_changed = true;

			//OK, see if there's a project that matches what they've started to type.
			CString strEntered;
			GetDlgItemText(IDC_COMPANY, strEntered);
			int nNewLength = strEntered.GetLength();

			//Only do this if they have added more characters.
			if(nNewLength > m_nLastEditLength && nNewLength > 0) {			
				_RecordsetPtr rsProject = CreateRecordset("SELECT TOP 1 Company FROM PersonT WHERE Company LIKE '%s%%' ORDER BY Company", _Q(strEntered));
				if(!rsProject->eof) {
					CString strRest = AdoFldString(rsProject, "Company").Mid(strEntered.GetLength());
					strEntered += strRest;
					m_bSettingCompany = true;
					SetDlgItemText(IDC_COMPANY, strEntered);
					m_bSettingCompany = false;
					((CNxEdit*)GetDlgItem(IDC_COMPANY))->SetSel(nNewLength, strEntered.GetLength());
				}
			}
			m_nLastEditLength = nNewLength;
			//PLID 14690 - make it capitalize
			Capitalize(IDC_COMPANY);
		}
	}NxCatchAll("Error in CGeneral2Dlg::OnChangeCompany()");

}

void CGeneral2Dlg::OnRequeryFinishedReferralList(short nFlags) 
{
	//enable/disable the appropriate buttons since the list won't have a selection upon requery
	OnSelChangedReferralList(-1);

	//we need to select the row that is the primary

	// (c.haag 2006-04-13 10:00) - PLID 20119 - We don't need to pull from data anymore. Now
	// we pull from m_nReferralID
	if (-1 != m_nReferralID) {
		NXDATALISTLib::IRowSettingsPtr pRow = m_referralSourceList->GetRow(m_referralSourceList->FindByColumn(0, m_nReferralID, 0, false));
		//DRT 10/9/02 - This was returning a null pointer when there was nothing in the list to select, which then yielded an exception
		if(pRow) {
			pRow->ForeColor = RGB(255,0,0);
			pRow->ForeColorSel = RGB(255,0,0);
		}
	}
	/*
	_RecordsetPtr rs;
	rs = CreateRecordset("SELECT ReferralID FROM PatientsT WHERE PersonID = %li", m_id);
	if(!rs->eof && m_referralSourceList->GetRowCount() > 0){
		IRowSettingsPtr pRow;
		pRow = m_referralSourceList->GetRow(m_referralSourceList->FindByColumn(0, rs->Fields->Item["ReferralID"]->Value, 0, false));
		//DRT 10/9/02 - This was returning a null pointer when there was nothing in the list to select, which then yielded an exception
		if(pRow) {
			pRow->ForeColor = RGB(255,0,0);
			pRow->ForeColorSel = RGB(255,0,0);
		}
	}*/
}

void CGeneral2Dlg::UpdateChangedDate(long nAuditID)
{
	// (j.jones 2005-04-14 09:52) - We simply need to pull out the stored date from the given AuditID.
	// If we were to simply use COleDateTime::GetCurrentTime() we would be off by a second or so
	// from the date that would be displayed when the user hits "Refresh", and we wouldn't want that, would we?

	_RecordsetPtr rs = CreateRecordset("SELECT ModifiedDate FROM PatientsT WHERE PersonID = %li", m_id);

	if(!rs->eof) {
		SetDlgItemText(IDC_LAST_MODIFIED, FormatDateTimeForInterface(AdoFldDateTime(rs, "ModifiedDate")));
	}
	rs->Close();
}

void CGeneral2Dlg::OnRequeryFinishedPatientTypeCombo(short nFlags) 
{
	NXDATALISTLib::IRowSettingsPtr pRow;
	_variant_t var;
	pRow = m_PatientTypeCombo->GetRow(-1);
	var.vt = VT_NULL;
	pRow->PutValue(0,var);
	pRow->PutValue(1,"<No Type Selected>");
	m_PatientTypeCombo->InsertRow(pRow,0);
}

void CGeneral2Dlg::OnEditNexwebInfo() 
{
	try {
		//(e.lally 2011-08-09) PLID 37287 - check read permissions
		if(!CheckCurrentUserPermissions(bioPatientNexWebLogin, sptRead)){
			return;
		}
		CNexWebLoginInfoDlg dlg(m_id, this);

		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
	
}

void CGeneral2Dlg::OnSelChangingRelation(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	// (a.walling 2006-11-14 16:20) - PLID 22715 - Cancel if no selection.
	if (*lppNewSel == NULL) {
		// (a.walling 2006-11-17 11:34) - PLID 22715 - Explicityly AddRef() to this LDISPATCH,
		//	otherwise we leak memory at best and crash at worst.
		if (lpOldSel != NULL)
			lpOldSel->AddRef();
		*lppNewSel = lpOldSel;	
	}
}

void CGeneral2Dlg::OnSelChosenRelation(LPDISPATCH lpRow) 
{
	// (a.walling 2006-11-14 13:25) - PLID 22715 - Store the related patient
	// For now, we will remove any other relations this person may have had, to enforce
	// membership to only one family. This can easily be modified to allow for a more
	// featureful approach to families if beneficial.

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRelatedPatientList->GetCurSel();
	if (pRow) {
		long nRelPatID = VarLong(pRow->GetValue(0), -1);

		if (nRelPatID == m_nRelatedPatient) {
			// nothing has changed, so don't bother with all this.
			return;
		}

		long nPersonFamilyID = -1;
		if (nRelPatID != -1) {
			nPersonFamilyID = FamilyUtils::GetFamilyID(nRelPatID);
		}

		if (nRelPatID == GetActivePatientID()) {
			AfxMessageBox("A patient may not mark themselves as their immediate relative.");
			TrySetRelatedPatient(); // this will set according to the previous value of m_nRelatedPatient
			return;
		}

		
		long nRelatives = FamilyUtils::CountRelatives(GetActivePatientID());
		long nFamilyID = FamilyUtils::GetFamilyID(GetActivePatientID());
		

		if ( (nRelatives > 0) && (nFamilyID != nPersonFamilyID) ) {
			CString strMessage;
			strMessage.Format("%li patients have marked this patient as their immediate relative. "
				"Removing them from this family will update those patients' immediate relative to another family member. "
				"Do you want to continue?", nRelatives);
			if (nRelatives == 1) {
				strMessage.Replace("patients ", "patient ");
				strMessage.Replace("have", "has");
			}
			long nResult = MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION | MB_YESNO);

			if (nResult == IDNO) {
				TrySetRelatedPatient(); // reset to previous selection
				return;
			}
		}

		if (nRelPatID > 0) {
			if (nPersonFamilyID == -1) { // relative not in family
				// create a new family record and relationship
				FamilyUtils::ClearRelationships(GetActivePatientID());
				nFamilyID = FamilyUtils::CreateNewFamilyAndRelationship(GetActivePatientID(), nRelPatID);
			} else {
				// save the data into PersonFamilyT (this will handle preexisting relationships as well)
				FamilyUtils::InsertRelationship(GetActivePatientID(), nRelPatID, nPersonFamilyID);
			}
		
			GetDlgItem(IDC_BTN_FAMILY)->EnableWindow(true);
		} else {
			// they set their relation to <No relative>!


			FamilyUtils::ClearRelationships(GetActivePatientID());
			GetDlgItem(IDC_BTN_FAMILY)->EnableWindow(false);
		}

		m_nRelatedPatient = nRelPatID;

		FamilyUtils::CleanUp();
	}
}


// (a.walling 2006-11-14 17:28) - PLID 22715 - Handle invalid/inactive patients
// (a.walling 2011-08-01 17:34) - PLID 44788 - Irrelevant
/*
void CGeneral2Dlg::OnTrySetSelFinishedRelation(long nRowEnum, long nFlags) 
{
	if (nRowEnum == NXDATALIST2Lib::sriNoRow) {
		// the patient was not found.. must have been inactivated or deleted
		m_pRelatedPatientList->PutCurSel(NULL);
		CString strName = GetExistingPatientName(m_nRelatedPatient);
		if (strName.IsEmpty()) {
			ASSERT(FALSE);
			strName = "<Invalid Patient>";
		}
		m_pRelatedPatientList->PutComboBoxText((_bstr_t)strName);
	}
}
*/

// (a.walling 2006-11-14 17:28) - PLID 22715 - Edit/view the family
void CGeneral2Dlg::OnBtnEditFamilies() 
{
	CEditFamilyDlg dlg(this);

	dlg.m_nPersonID = GetActivePatientID();
	dlg.m_nRelativeID = m_nRelatedPatient;

	long nResult = dlg.DoModal();

	if (nResult == IDOK) {
		// reload since data may have changed
		_RecordsetPtr prs = CreateRecordset("SELECT RelativeID FROM PersonFamilyT WHERE PersonID = %li", GetActivePatientID());
		if (!prs->eof) {
			m_nRelatedPatient = AdoFldLong(prs, "RelativeID", -1);
		} else {
			m_nRelatedPatient = -1;
		}

		TrySetRelatedPatient();
	}
}

// (a.walling 2011-08-01 17:34) - PLID 44788 - Try to set the related patient, or use the combo text
void CGeneral2Dlg::TrySetRelatedPatient()
{
	m_pRelatedPatientList->PutCurSel(NULL);
	GetDlgItem(IDC_BTN_FAMILY)->EnableWindow(m_nRelatedPatient >= 0);

	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
	if (m_pRelatedPatientList->TrySetSelByColumn_Deprecated(0, m_nRelatedPatient) < 0) {
		if (m_nRelatedPatient == -1) {
			// no relative; so select the <No Relation> item by default (m_nRelatedPatient == -1)
			m_pRelatedPatientList->ComboBoxText = "<No Relation>";
		} else {
			m_pRelatedPatientList->ComboBoxText = (LPCTSTR)GetExistingPatientName(m_nRelatedPatient);			
		}
	}
}

// (a.walling 2011-08-01 17:34) - PLID 44788 - Try to set the referring patient, or use the combo text
void CGeneral2Dlg::TrySetReferringPatient()
{
	if (!m_referringPatientCombo) return;

	m_referringPatientCombo->CurSel = -1;

	if (m_referringPatientCombo->TrySetSelByColumn(0, m_nReferringPatientID) < 0) {		
		if (m_nReferringPatientID == -1) {
			m_referringPatientCombo->ComboBoxText = "<No Patient Selected>";
		} else if (m_nReferringPatientID == -2) {
			m_referringPatientCombo->ComboBoxText = "<Unspecified>";
		} else {
			m_referringPatientCombo->ComboBoxText = (LPCTSTR)GetExistingPatientName(m_nReferringPatientID);
		}
	}
}

void CGeneral2Dlg::OnGotoRefPhys() 
{
	// (a.walling 2007-04-06 17:21) - PLID 18643 - add a go button for ref phys
	try {
		//make sure we have a valid selection
		long nCurSel = m_ReferringPhyCombo->GetCurSel();
		long nID;

		if(nCurSel == -1){
			//The referring physician might be inactive
			_RecordsetPtr rs = CreateRecordset("SELECT DefaultReferringPhyID FROM PatientsT "
				"WHERE PersonID = %li", m_id);
			if(rs->eof){
				//There is no contact so just return
				return;
			}
			else{
				nID = AdoFldLong(rs, "DefaultReferringPhyID", -1);
			}
		}
		else{
			//get the id
			nID = VarLong(m_ReferringPhyCombo->GetValue(nCurSel, rpcID));
		}

		if (nID < 0) return;

		//do the flipping
		CMainFrame *p = GetMainFrame();
		CNxTabView *pView;

		if(p->FlipToModule(CONTACT_MODULE_NAME)) {

			if (nID != GetActiveContactID())
			{
				p->m_contactToolBar.SetActiveContactID(nID);
			}

			pView = (CNxTabView *)p->GetOpenView(CONTACT_MODULE_NAME);
			if (pView) 
			{	if(pView->GetActiveTab()==0)
					pView->UpdateView();
				else
					pView->SetActiveTab(0);
			}

			// (k.messina 2010-04-12 11:16) - PLID 37957 lock internal contact notes
			if(IsNexTechInternal()) {
				((CContactView*)GetMainFrame()->GetOpenView(CONTACT_MODULE_NAME))->CheckViewNotesPermissions();
			}
		}	
	} NxCatchAll("Error in OnGotoRefPhys()");
}

void CGeneral2Dlg::OnGotoRefPat()
{
	// (c.haag 2010-02-01 13:28) - PLID 34727 - Added a button for going to the referring patient
	try {
		// Make sure we have a valid selection. The combo includes inactive patients, so we will simply
		// take whatever the current selection is, and run with it.
		//long nCurSel = m_referringPatientCombo->GetCurSel();
		if (GetMainFrame()) {
			long nID = m_nReferringPatientID;
			if (nID > 0 && nID != GetActivePatientID()) {

				// Set the active patient
				if(GetMainFrame()->m_patToolBar.TrySetActivePatientID(nID)) {
					CPatientView* pView = (CPatientView*)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
					if (pView) {
						// Go to the general 1 tab
						pView->SetActiveTab(PatientsModule::General1Tab);
						// Update the view
						pView->UpdateView();
					}
				}

			}
			else {
				// Either no valid patient is selected, or, the ID is already the active patient (not that it should be possible)
			}
		} else {
			// Either there is no valid current selection, or, we have no frame window
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CGeneral2Dlg::OnGotoPcp() 
{
	// (a.walling 2007-04-06 17:21) - PLID 18643 - add a go button for pcp
	try {
		//make sure we have a valid selection
		long nCurSel = m_PCPCombo->GetCurSel();
		long nID;

		if(nCurSel == -1){
			//The primary care provider might be inactive
			_RecordsetPtr rs = CreateRecordset("SELECT PCP FROM PatientsT "
				"WHERE PersonID = %li", m_id);
			if(rs->eof){
				//There is no contact so just return
				return;
			}
			else{
				nID = AdoFldLong(rs, "PCP", -1);
			}
		}
		else{
			//get the id
			nID = VarLong(m_PCPCombo->GetValue(nCurSel, 0));
		}

		if (nID < 0) return;

		//do the flipping
		CMainFrame *p = GetMainFrame();
		CNxTabView *pView;

		if(p->FlipToModule(CONTACT_MODULE_NAME)) {

			if (nID != GetActiveContactID())
			{
				p->m_contactToolBar.SetActiveContactID(nID);
			}

			pView = (CNxTabView *)p->GetOpenView(CONTACT_MODULE_NAME);
			if (pView) 
			{	if(pView->GetActiveTab()==0)
					pView->UpdateView();
				else
					pView->SetActiveTab(0);
			}

			// (k.messina 2010-04-12 11:16) - PLID 37957 lock internal contact notes
			if(IsNexTechInternal()) {
				((CContactView*)GetMainFrame()->GetOpenView(CONTACT_MODULE_NAME))->CheckViewNotesPermissions();
			}
		}	
	} NxCatchAll("Error in OnGotoPcp()");	
}

// (j.jones 2008-03-20 16:06) - PLID 29334 - will hide the serialized product list
// and move other controls to take up its space
void CGeneral2Dlg::HideSerializedProductList()
{
	try {

		if(m_bHasAdvInventoryLicense) {
			return;
		}

		//first hide the list and its label
		GetDlgItem(IDC_SERIALIZED_PRODUCTS_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SERIALIZED_PRODUCT_LIST)->ShowWindow(SW_HIDE);
		//TES 5/30/2008 - PLID 29999 - Also hide the new "New Allocation" button.
		GetDlgItem(IDC_ADD_ALLOCATION)->ShowWindow(SW_HIDE);

		//now all the referral controls have to move

		CRect rcSerializedList;
		GetDlgItem(IDC_SERIALIZED_PRODUCT_LIST)->GetWindowRect(&rcSerializedList);
		ScreenToClient(&rcSerializedList);

		//get the prospects referred controls
		CRect rcProspectLabel;
		GetDlgItem(IDC_NUM_PROSPECTS_REFERRED_LABEL)->GetWindowRect(&rcProspectLabel);
		ScreenToClient(&rcProspectLabel);
		CRect rcProspectValue;
		GetDlgItem(IDC_NUM_PROSPECTS_REF)->GetWindowRect(&rcProspectValue);
		ScreenToClient(&rcProspectValue);
		CRect rcProspectBtn;
		GetDlgItem(IDC_REFERRED_PROSPECTS)->GetWindowRect(&rcProspectBtn);
		ScreenToClient(&rcProspectBtn);

		//get the patients referred controls
		CRect rcPatientLabel;
		GetDlgItem(IDC_NUM_PATIENTS_REFERRED_LABEL)->GetWindowRect(&rcPatientLabel);
		ScreenToClient(&rcPatientLabel);
		CRect rcPatientValue;
		GetDlgItem(IDC_NUM_PATIENTS_REF)->GetWindowRect(&rcPatientValue);
		ScreenToClient(&rcPatientValue);
		CRect rcPatientBtn;
		GetDlgItem(IDC_REFERRED_PATS)->GetWindowRect(&rcPatientBtn);
		ScreenToClient(&rcPatientBtn);

		long nDiffBetweenPatientAndProspect = rcProspectLabel.top - rcPatientLabel.bottom;

		//get the referral list controls
		CRect rcReferralList;
		GetDlgItem(IDC_REFERRAL_LIST)->GetWindowRect(&rcReferralList);
		ScreenToClient(&rcReferralList);
		CRect rcAddBtn;
		GetDlgItem(IDC_ADD_REFERRAL)->GetWindowRect(&rcAddBtn);
		ScreenToClient(&rcAddBtn);
		CRect rcRemoveBtn;
		GetDlgItem(IDC_REMOVE_REFERRAL)->GetWindowRect(&rcRemoveBtn);
		ScreenToClient(&rcRemoveBtn);
		CRect rcMakePrimaryBtn;
		GetDlgItem(IDC_MAKE_PRIMARY_REFERRAL)->GetWindowRect(&rcMakePrimaryBtn);
		ScreenToClient(&rcMakePrimaryBtn);

		long nDiffBetweenPatientAndRefButtons = rcPatientLabel.top - rcAddBtn.bottom;
		long nDiffBetweenRefButtonsAndList = rcAddBtn.top - rcReferralList.bottom;

		//move the prospects referred controls
		{
			long nProspectInfoHeight = rcProspectLabel.Height();
			rcProspectLabel.bottom = rcSerializedList.bottom;
			rcProspectLabel.top = rcProspectLabel.bottom - nProspectInfoHeight;
			GetDlgItem(IDC_NUM_PROSPECTS_REFERRED_LABEL)->MoveWindow(rcProspectLabel);
			rcProspectValue.bottom = rcProspectLabel.bottom;
			rcProspectValue.top = rcProspectLabel.top;
			GetDlgItem(IDC_NUM_PROSPECTS_REF)->MoveWindow(rcProspectValue);
			rcProspectBtn.bottom = rcProspectLabel.bottom;
			rcProspectBtn.top = rcProspectLabel.top;
			GetDlgItem(IDC_REFERRED_PROSPECTS)->MoveWindow(rcProspectBtn);
		}

		//move the patients referred controls
		{
			long nPatientInfoHeight = rcPatientLabel.Height();
			rcPatientLabel.bottom = rcProspectLabel.top - nDiffBetweenPatientAndProspect;
			rcPatientLabel.top = rcPatientLabel.bottom - nPatientInfoHeight;
			GetDlgItem(IDC_NUM_PATIENTS_REFERRED_LABEL)->MoveWindow(rcPatientLabel);
			rcPatientValue.bottom = rcPatientLabel.bottom;
			rcPatientValue.top = rcPatientLabel.top;
			GetDlgItem(IDC_NUM_PATIENTS_REF)->MoveWindow(rcPatientValue);
			rcPatientBtn.bottom = rcPatientLabel.bottom;
			rcPatientBtn.top = rcPatientLabel.top;
			GetDlgItem(IDC_REFERRED_PATS)->MoveWindow(rcPatientBtn);
		}

		//move the referral list controls
		{
			//first the buttons
			long nReferralButtonHeight = rcAddBtn.Height();
			rcAddBtn.bottom = rcPatientLabel.top - nDiffBetweenPatientAndRefButtons;
			rcAddBtn.top = rcAddBtn.bottom - nReferralButtonHeight;
			GetDlgItem(IDC_ADD_REFERRAL)->MoveWindow(rcAddBtn);
			rcRemoveBtn.bottom = rcAddBtn.bottom;
			rcRemoveBtn.top = rcAddBtn.top;
			GetDlgItem(IDC_REMOVE_REFERRAL)->MoveWindow(rcRemoveBtn);
			rcMakePrimaryBtn.bottom = rcAddBtn.bottom;
			rcMakePrimaryBtn.top = rcAddBtn.top;
			GetDlgItem(IDC_MAKE_PRIMARY_REFERRAL)->MoveWindow(rcMakePrimaryBtn);

			//now lengthen the list
			rcReferralList.bottom = rcAddBtn.top - nDiffBetweenRefButtonsAndList;
			GetDlgItem(IDC_REFERRAL_LIST)->MoveWindow(rcReferralList);
		}

	}NxCatchAll("Error in CGeneral2Dlg::HideSerializedProductList");
}

// (j.jones 2008-03-21 08:56) - PLID 29334 - the clause is rather large, so it's
// going into its own function for readability
void CGeneral2Dlg::SetSerializedProductListFromClause()
{
	try {

		//the structure of this query was lifted mainly from the
		//Serial Numbered / Expirable Products By Patient report,
		//but altered to include only serial numbered products

		// (j.jones 2008-03-31 10:37) - PLID 29458 - added allocation notes

		//TES 5/29/2008 - PLID 29999 - Reworked this query to include allocated but unused items, and a "Status" that's the
		// same as the one in the Inventory Overview tab, as well as showing an appropriate date for each status.
		// (j.jones 2009-02-09 12:56) - PLID 32775 - renamed 'Charged' to 'Billed'
		CString strFrom;
		strFrom.Format("(SELECT "
			"ProductID AS ProductID, Name, SerialNum, ExpDate, ServiceDate, PatientID, "
			"Coalesce(LocName,'') AS LocName, LocationID AS LocID, Status, Notes, AllocationID FROM "
			"(SELECT "
			"ProductItemID, ProductT.ID AS ProductID, ServiceT.Name, ProductItemsT.SerialNum, ProductItemsT.ExpDate, LineItemT.Date AS ServiceDate, "
			"PersonT.ID AS PatientID, LocationsT.Name AS LocName, LocationsT.ID AS LocationID, "
			"'' AS Notes, "
			"'Billed' AS Status, NULL AS AllocationID "
			"FROM "
			"ProductT "
			"LEFT JOIN ProductItemsT ON ProductT.ID = ProductItemsT.ProductID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"INNER JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"INNER JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"WHERE /*ProductT.HasSerialNum = 1 AND*/ (SerialNum Is Not Null /*OR ExpDate Is Not Null*/) AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND ProductItemsT.Deleted = 0 "
			"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
			"	AND PatientInvAllocationsT.Status = %li AND PatientInvAllocationDetailsT.Status = %li) "
			""
			"UNION SELECT "
			"ProductItemID, ProductT.ID AS ProductID, ServiceT.Name, ProductItemsT.SerialNum, ProductItemsT.ExpDate, CaseHistoryT.SurgeryDate AS ServiceDate, "			
			"PersonT.ID AS PatientID, LocationsT.Name AS LocName, LocationsT.ID AS LocationID, "
			"'' AS Notes, "
			"'Case History' AS Status, NULL AS AllocationID "
			"FROM "
			"ProductT "
			"LEFT JOIN ProductItemsT ON ProductT.ID = ProductItemsT.ProductID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"INNER JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"INNER JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
			"INNER JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
			"INNER JOIN PersonT ON CaseHistoryT.PersonID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "			
			"LEFT JOIN LocationsT ON CaseHistoryT.LocationID = LocationsT.ID "
			"WHERE (SerialNum Is Not Null /*OR ExpDate Is Not Null*/) AND CompletedDate Is Not Null AND ProductItemsT.Deleted = 0 "
			"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
			"	AND PatientInvAllocationsT.Status = %li AND PatientInvAllocationDetailsT.Status = %li) "
			""
			"UNION SELECT "
			"PatientInvAllocationDetailsT.ProductItemID, ProductT.ID AS ProductID, ServiceT.Name, ProductItemsT.SerialNum, ProductItemsT.ExpDate, "
			"Coalesce(LineItemT.Date, CaseHistoryT.CompletedDate, CASE WHEN PatientInvAllocationsT.Status = %li THEN COALESCE(AppointmentsT.StartTime, PatientInvAllocationsT.CompletionDate) ELSE NULL END, PatientInvAllocationsT.InputDate) AS ServiceDate, "
			"PersonT.ID AS PatientID, LocationsT.Name AS LocName, LocationsT.ID AS LocationID, "
			"PatientInvAllocationDetailsT.Notes, "
			"CASE "
			"WHEN LineItemT.ID Is Not Null THEN 'Billed' "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN 'Case History' "
			"WHEN PatientInvAllocationDetailsT.Status = %li THEN 'Used & Not Billed' "
			"ELSE 'Allocated' END AS Status, PatientInvAllocationsT.ID AS AllocationID "
			"FROM "
			"ProductT "
			"LEFT JOIN ProductItemsT ON ProductT.ID = ProductItemsT.ProductID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"INNER JOIN PatientInvAllocationDetailsT ON ProductItemsT.ID = PatientInvAllocationDetailsT.ProductItemID "
			"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"INNER JOIN PersonT ON PatientInvAllocationsT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
			"LEFT JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
			
			"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"LEFT JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
			"LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "			
			"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
			"LEFT JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
			
			"WHERE (SerialNum Is Not Null /*OR ExpDate Is Not Null*/) "
			"AND PatientInvAllocationsT.Status <> %li AND PatientInvAllocationDetailsT.Status <> %li AND "
			"PatientInvAllocationDetailsT.Status <> %li AND ProductItemsT.Deleted = 0) AS SerializedProductsQ "
			") AS FromClause",
			InvUtils::iasCompleted, InvUtils::iadsUsed, InvUtils::iasCompleted, InvUtils::iadsUsed, InvUtils::iadsUsed, InvUtils::iadsUsed, InvUtils::iasDeleted, InvUtils::iadsDeleted, InvUtils::iadsReleased);

		m_SerializedProductList->PutFromClause(_bstr_t(strFrom));

		//the Refresh() function will set the where clause and requery
	
	}NxCatchAll("Error in CGeneral2Dlg::SetSerializedProductListFromClause");
}

HBRUSH CGeneral2Dlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch(pWnd->GetDlgCtrlID())
	{
		case IDC_EDIT_WARNING_USER:
		{
			// (z.manning, 05/16/2008) - PLID 30050 - make borderless edit controls transparent
			pDC->SetBkColor(GetSolidBackgroundColor());
			return m_brBackground;
		}
		break;
	}

	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CGeneral2Dlg::OnRButtonUpSerializedProductList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {		
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			m_SerializedProductList->PutCurSel(pRow);
		}
		else {
			return;
		}

		 //TES 5/29/2008 - PLID 29999 - Is this product associated with an allocation?
		long nAllocationID = VarLong(pRow->GetValue(8), -1);
		//TES 5/30/2008 - PLID 29999 - Also make sure they have read permission.
		if(nAllocationID != -1 && (GetCurrentUserPermissions(bioInventoryAllocation) & SPT__R_________ANDPASS)) {
			//TES 5/29/2008 - PLID 29999 - It is, give them the option to open it.

			//Create the pop up menu
			CMenu mPopup;
			mPopup.CreatePopupMenu();

			const int OPEN_ALLOCATION	= 1;
			mPopup.AppendMenu(MF_ENABLED, OPEN_ALLOCATION, "&Open Allocation");

			//Pop up the menu
			CPoint pt;
			GetCursorPos(&pt);
			long nResult = mPopup.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y, this);

			// Handle the result
			if (nResult == OPEN_ALLOCATION) {
				//TES 5/29/2008 - PLID 29999 - They chose to open it, so let's do so, after confirming that they have permission.
				if(CheckCurrentUserPermissions(bioInventoryAllocation, sptRead)) {
					CInvPatientAllocationDlg dlg(this);
					dlg.m_nID = nAllocationID;
					if(dlg.DoModal() == IDOK) {
						//requery the list to reflect any changes to the allocation
						m_SerializedProductList->Requery();
					}
				}
			}
		}
	} NxCatchAll("Error in  CGeneral2Dlg::OnRButtonUpSerializedProductList");

}

void CGeneral2Dlg::OnAddAllocation() 
{
	try {
		//TES 5/30/2008 - PLID 29999 - Make sure they have permission.
		if(!CheckCurrentUserPermissions(bioInventoryAllocation, sptCreate)) {
			return;
		}
		//TES 5/29/2008 - PLID 29999 - Create a new allocation, defaulted to this patient.
		CInvPatientAllocationDlg dlg(this);
		dlg.m_nID = -1;
		dlg.m_nDefaultPatientID = GetActivePatientID();
		if(dlg.DoModal() == IDOK) {
			//requery the list to reflect any changes to the allocation
			m_SerializedProductList->Requery();
		}
	}NxCatchAll("Error in CGeneral2Dlg::OnAddAllocation()");
}

// (a.walling 2009-05-25 10:31) - PLID 27859 - Populate the language dropdown
void CGeneral2Dlg::PopulateLanguageCombo()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;

		//(e.lally 2011-04-11) PLID 43187 - Changed the language combo to use a static table instead of being hardcoded
			//put the selection for none at the top since we are going to requery.
		// (d.thompson 2013-05-08) - PLID 55940 - We are now using the 3 character language code instead of the 2 character
		//	language code.
#define ADD_LANGUAGE_ROW(code, name) pRow = m_languageCombo->GetNewRow();\
		pRow->PutValue(0, g_cvarNull);\
		pRow->PutValue(1, name);\
		pRow->PutValue(2, code);\
		m_languageCombo->AddRowBefore(pRow, NULL);

		m_languageCombo->Requery();

		ADD_LANGUAGE_ROW("", _bstr_t(l_strNoLanguageText));

	} NxCatchAll("Error populating language list");
}

// (j.jones 2009-10-19 10:34) - PLID 34327 - added ethnicity option
void CGeneral2Dlg::OnSelChosenEthnicityCombo(long nRow)
{
	try {
		// (j.armen 2012-06-07 14:51) - PLID 50825 - Refactored, Update HL7
		// (d.thompson 2012-08-09) - PLID 52045 - Reworked table structure
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON\r\n"
			"DECLARE @EthnicityID TABLE (OldID INT, NewID INT)\r\n"
			"UPDATE PersonT SET Ethnicity = {VT_I4} "
			"	OUTPUT deleted.Ethnicity, inserted.Ethnicity INTO @EthnicityID "
			"	WHERE ID = {INT}\r\n"
			"SET NOCOUNT OFF\r\n"
			"SELECT\r\n"
			"	OldID, OldEthnicityT.Name  AS OldName, NewID, NewEthnicityT.Name  AS NewName\r\n"
			"FROM @EthnicityID EthnicityID\r\n"
			"LEFT JOIN EthnicityT OldEthnicityT ON EthnicityID.OldID = OldEthnicityT.ID\r\n"
			"LEFT JOIN EthnicityT NewEthnicityT ON EthnicityID.NewID = NewEthnicityT.ID\r\n",
			((nRow == -1) ? g_cvarNull : m_ethnicityCombo->GetValue(nRow,ethnicityID)), m_id);

		if(!prs->eof)
		{
			CString strOldValue = AdoFldString(prs, "OldName", l_strNoEthnicityText);
			CString strNewValue = AdoFldString(prs, "NewName", l_strNoEthnicityText);

			if(AdoFldLong(prs, "OldID", -1) != AdoFldLong(prs, "NewID", -1))
			{
				//DRT 6/30/2005 - PLID 16654 - Auditing
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientEthnicity, m_id, strOldValue, strNewValue, aepMedium, aetChanged);
				UpdateChangedDate(nAuditID);
				UpdateExistingPatientInHL7(m_id);
			}
		}
	} NxCatchAll("Error changing patient ethnicity.");
}

// (a.walling 2010-06-30 17:43) - PLID 18081 - Warning categories
void CGeneral2Dlg::SelChangingListWarningCategories(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL || !(GetCurrentUserPermissions(bioPatientWarning) & SPT___W________ANDPASS)) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2010-06-30 17:43) - PLID 18081 - Warning categories
void CGeneral2Dlg::SelChosenListWarningCategories(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		_variant_t varWarningCategoryID = pRow->GetValue(0);

		if (VarLong(varWarningCategoryID, -1) == LONG_MAX) {
			// edit!
			if (IsCurrentUserAdministrator()) {

				_variant_t varPreviousValue = g_cvarNull;
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT WarningCategoriesT.ID, WarningCategoriesT.Name "
					"FROM PersonT "
					"INNER JOIN WarningCategoriesT "
						"ON PersonT.WarningCategoryID = WarningCategoriesT.ID "
					"WHERE PersonT.ID = {INT}", m_id);
				if (!prs->eof) {
					varPreviousValue = prs->Fields->Item["ID"]->Value;
				}

				m_pWarningCategoryList->SetSelByColumn(0, varPreviousValue);

				CEditWarningCategoriesDlg dlg(this);
				dlg.DoModal();

				RequeryListWarningCategories();

				m_pWarningCategoryList->SetSelByColumn(0, varPreviousValue);
			}

			return;
		}

		if (UserPermission(ChangeWarning)) {
			CString strNew = VarString(pRow->GetValue(1), (LPCTSTR)GetRemotePropertyText("DefaultWarningCategoryName", "General", 0, "<None>", true));
			
			//for auditing
			CString strOld;
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT WarningCategoriesT.ID, WarningCategoriesT.Name "
				"FROM PersonT "
				"INNER JOIN WarningCategoriesT "
					"ON PersonT.WarningCategoryID = WarningCategoriesT.ID "
				"WHERE PersonT.ID = {INT}", m_id);

			if (!prs->eof) {
				strOld = AdoFldString(prs, "Name", (LPCTSTR)GetRemotePropertyText("DefaultWarningCategoryName", "General", 0, "<None>", true));
			} else {
				strOld = GetRemotePropertyText("DefaultWarningCategoryName", "General", 0, "<None>", true);
			}

			ExecuteParamSql("UPDATE PersonT SET WarningCategoryID = {VT_I4} WHERE ID = {INT}", varWarningCategoryID, m_id);
			AuditEvent(m_id, m_strPatientName, BeginNewAuditEvent(), aeiPatientG2WarnCategory, m_id, strOld, strNew, aepMedium, aetChanged);
			UpdateChangedDate(-1);
		}
	} NxCatchAll(__FUNCTION__);
}

void CGeneral2Dlg::RequeryListWarningCategories()
{
	try {
		m_pWarningCategoryList->Requery();

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pWarningCategoryList->GetNewRow();
		pRow->PutValue(0, g_cvarNull);
		pRow->PutValue(1, (LPCTSTR)GetRemotePropertyText("DefaultWarningCategoryName", "General", 0, "<None>", true));
		// (a.walling 2010-07-30 17:30) - PLID 18081 - Default warning category color
		COLORREF defaultColor = GetRemotePropertyInt("DefaultWarningCategoryColor", 0, 0, "<None>", true);
		if (defaultColor == 0 || defaultColor == RGB(0xFF, 0xFF, 0xFF)) {
			pRow->PutValue(2, g_cvarNull);
		} else {
			pRow->PutValue(2, _variant_t((long)defaultColor));
			pRow->PutBackColor(defaultColor);
			pRow->PutBackColorSel(defaultColor);
		}
		pRow->PutValue(3, g_cvarNull);
		pRow->PutValue(4, (long)-1);

		m_pWarningCategoryList->AddRowSorted(pRow, NULL);
	} NxCatchAll(__FUNCTION__);
}

void CGeneral2Dlg::RequeryFinishedListWarningCategories(short nFlags)
{
	try {
		if (IsCurrentUserAdministrator()) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pWarningCategoryList->GetNewRow();
			pRow->PutValue(0, (long)LONG_MAX);
			pRow->PutValue(1, " (Edit)");
			pRow->PutValue(2, g_cvarNull);
			pRow->PutValue(3, g_cvarNull);
			pRow->PutValue(4, (long)LONG_MAX);

			m_pWarningCategoryList->AddRowSorted(pRow, NULL);
		}
	} NxCatchAll(__FUNCTION__);
}

void CGeneral2Dlg::CurSelWasSetListWarningCategories()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pWarningCategoryList->CurSel;

		_variant_t varWarningCategoryColor = g_cvarNull;
		if (pRow) {
			varWarningCategoryColor = pRow->GetValue(2);

			_variant_t varWarningCategoryID = pRow->GetValue(0);

			if (VarLong(varWarningCategoryID, -1) == LONG_MAX) {
				return;
			}
		}

		CNxEdit* pWarning = (CNxEdit*)GetDlgItem(IDC_WARNING);

		if (varWarningCategoryColor.vt != VT_I4) {
			pWarning->ResetBackgroundColorStandard();
			pWarning->ResetBackgroundColorFocus();
			pWarning->ResetBackgroundColorHovered();
			pWarning->ResetBackgroundColorHoveredFocus();
		} else {
			COLORREF color = (COLORREF)VarLong(varWarningCategoryColor);

			// (a.walling 2010-07-02 10:40) - PLID 39498 - Apply these colors even when disabled or readonly
			pWarning->SetBackgroundColorStandard(color, true, true);
			pWarning->SetBackgroundColorHovered(color, true, true);

			// Focus and HoveredFocus will not be touched (for readability)
			pWarning->ResetBackgroundColorFocus();
			pWarning->ResetBackgroundColorHoveredFocus();
		}

		pWarning->Invalidate();

	} NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-07-20 12:16) - PLID 30127
void CGeneral2Dlg::OnBnClickedAdjustRewardPoints()
{
	try
	{
		if(!CheckCurrentUserPermissions(bioPatientRewardPoints, sptWrite)) {
			return;
		}

		// (z.manning 2010-07-20 15:26) - PLID 30127 - We need to know how many reward points the patient currently has
		CString strCurrentRewardPoints;
		m_nxeditAccumRewardPts.GetWindowText(strCurrentRewardPoints);
		COleCurrency cyCurrentRewardPoints;
		if(!cyCurrentRewardPoints.ParseCurrency(strCurrentRewardPoints)) {
			cyCurrentRewardPoints = Rewards::GetTotalPoints(GetActivePatientID());
		}

		// (z.manning 2010-07-20 15:28) - PLID 30127 - Prompt for the new amount of reward points
		CString strNewRewardPoints;
		int nResult = InputBoxLimited(this, FormatString("Enter the total number of reward points that %s should have", GetActivePatientName())
			, strNewRewardPoints, "", 14, false, false, NULL);
		if(nResult == IDOK)
		{
			COleCurrency cyNewRewardPoints;
			if(!cyNewRewardPoints.ParseCurrency(strNewRewardPoints)) {
				MessageBox("Invalid reward point value", NULL, MB_ICONERROR);
				return;
			}

			Rewards::ManuallyAdjustRewardPoints(GetActivePatientID(), cyCurrentRewardPoints, cyNewRewardPoints);

			SetDlgItemText(IDC_ACCUM_REWARD_PTS, FormatCurrencyForInterface(cyNewRewardPoints, FALSE));
		}

	}NxCatchAll(__FUNCTION__);
}

//(c.copits 2010-10-29) PLID 38598 - Warranty tracking system
CString CGeneral2Dlg::MakeWarrantyDlgProductListFromClause()
{
	CString strFrom;

	try {

		// This query "borrowed" from CGeneral2Dlg::SetSerializedProductListFromClause()
		// (The function of the query is essentially the same)
		// The difference is that I added ProductItemsT.WarrantyExpDate

		strFrom.Format("(SELECT "
			"ProductID AS ProductID, Name, SerialNum, ExpDate, WarrantyExpDate, ServiceDate, PatientID, "
			"Coalesce(LocName,'') AS LocName, LocationID AS LocID, Status, Notes, AllocationID FROM "
			"(SELECT "
			"ProductItemID, ProductT.ID AS ProductID, ServiceT.Name, ProductItemsT.SerialNum, ProductItemsT.ExpDate, ProductItemsT.WarrantyExpDate, LineItemT.Date AS ServiceDate, "
			"PersonT.ID AS PatientID, LocationsT.Name AS LocName, LocationsT.ID AS LocationID, "
			"'' AS Notes, "
			"'Billed' AS Status, NULL AS AllocationID "
			"FROM "
			"ProductT "
			"LEFT JOIN ProductItemsT ON ProductT.ID = ProductItemsT.ProductID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"INNER JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"INNER JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN PersonT ON LineItemT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"WHERE /*ProductT.HasSerialNum = 1 AND*/ (SerialNum Is Not Null /*OR ExpDate Is Not Null*/) AND LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND ProductItemsT.Deleted = 0 "
			"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
			"	AND PatientInvAllocationsT.Status = %li AND PatientInvAllocationDetailsT.Status = %li) "
			""
			"UNION SELECT "
			"ProductItemID, ProductT.ID AS ProductID, ServiceT.Name, ProductItemsT.SerialNum, ProductItemsT.ExpDate, ProductItemsT.WarrantyExpDate, CaseHistoryT.SurgeryDate AS ServiceDate, "			
			"PersonT.ID AS PatientID, LocationsT.Name AS LocName, LocationsT.ID AS LocationID, "
			"'' AS Notes, "
			"'Case History' AS Status, NULL AS AllocationID "
			"FROM "
			"ProductT "
			"LEFT JOIN ProductItemsT ON ProductT.ID = ProductItemsT.ProductID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"INNER JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"INNER JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
			"INNER JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
			"INNER JOIN PersonT ON CaseHistoryT.PersonID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "			
			"LEFT JOIN LocationsT ON CaseHistoryT.LocationID = LocationsT.ID "
			"WHERE (SerialNum Is Not Null /*OR ExpDate Is Not Null*/) AND CompletedDate Is Not Null AND ProductItemsT.Deleted = 0 "
			"AND ProductItemsT.ID NOT IN (SELECT ProductItemID FROM PatientInvAllocationDetailsT "
			"	INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"	WHERE PatientInvAllocationDetailsT.ProductItemID Is Not Null "
			"	AND PatientInvAllocationsT.Status = %li AND PatientInvAllocationDetailsT.Status = %li) "
			""
			"UNION SELECT "
			"PatientInvAllocationDetailsT.ProductItemID, ProductT.ID AS ProductID, ServiceT.Name, ProductItemsT.SerialNum, ProductItemsT.ExpDate, ProductItemsT.WarrantyExpDate, "
			"Coalesce(LineItemT.Date, CaseHistoryT.CompletedDate, CASE WHEN PatientInvAllocationsT.Status = %li THEN COALESCE(AppointmentsT.StartTime, PatientInvAllocationsT.CompletionDate) ELSE NULL END, PatientInvAllocationsT.InputDate) AS ServiceDate, "
			"PersonT.ID AS PatientID, LocationsT.Name AS LocName, LocationsT.ID AS LocationID, "
			"PatientInvAllocationDetailsT.Notes, "
			"CASE "
			"WHEN LineItemT.ID Is Not Null THEN 'Billed' "
			"WHEN CaseHistoryDetailsT.ID Is Not Null THEN 'Case History' "
			"WHEN PatientInvAllocationDetailsT.Status = %li THEN 'Used & Not Billed' "
			"ELSE 'Allocated' END AS Status, PatientInvAllocationsT.ID AS AllocationID "
			"FROM "
			"ProductT "
			"LEFT JOIN ProductItemsT ON ProductT.ID = ProductItemsT.ProductID "
			"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID "
			"INNER JOIN PatientInvAllocationDetailsT ON ProductItemsT.ID = PatientInvAllocationDetailsT.ProductItemID "
			"INNER JOIN PatientInvAllocationsT ON PatientInvAllocationDetailsT.AllocationID = PatientInvAllocationsT.ID "
			"INNER JOIN PersonT ON PatientInvAllocationsT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
			"LEFT JOIN LocationsT ON PatientInvAllocationsT.LocationID = LocationsT.ID "
			
			"LEFT JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID "
			"LEFT JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID "
			"LEFT JOIN LineItemT ON ChargesT.ID = LineItemT.ID "			
			"LEFT JOIN CaseHistoryDetailsT ON ChargedProductItemsT.CaseHistoryDetailID = CaseHistoryDetailsT.ID "
			"LEFT JOIN CaseHistoryT ON CaseHistoryDetailsT.CaseHistoryID = CaseHistoryT.ID "
			
			"WHERE (SerialNum Is Not Null /*OR ExpDate Is Not Null*/) "
			"AND PatientInvAllocationsT.Status <> %li AND PatientInvAllocationDetailsT.Status <> %li AND "
			"PatientInvAllocationDetailsT.Status <> %li AND ProductItemsT.Deleted = 0) AS SerializedProductsQ "
			") AS FromClause",
			InvUtils::iasCompleted, InvUtils::iadsUsed, InvUtils::iasCompleted, InvUtils::iadsUsed, InvUtils::iadsUsed, InvUtils::iadsUsed, InvUtils::iasDeleted, InvUtils::iadsDeleted, InvUtils::iadsReleased);
	
	} NxCatchAll(__FUNCTION__);

	return strFrom;
}

//(c.copits 2010-10-28) PLID 38598 - Warranty tracking system
void CGeneral2Dlg::OnBnClickedWarranty()
{
	try {

		// Create where clause for warranty dialog
		CString strWhere, strFrom;
		strWhere.Format("PatientID = %li", m_id);

		// Set m_strWarrantyDlgFromClause
		strFrom = MakeWarrantyDlgProductListFromClause();

		// Create dialog and pass in clauses for datalist
		CWarrantyDlg dlg(this);
		dlg.SetFromClause(strFrom);
		dlg.SetWhereClause(strWhere);
		// (b.spivey, May 22, 2012) - PLID 50558 - We use the default patient blue always.
		dlg.SetWarrantyBkgColor(GetNxColor(GNC_PATIENT_STATUS, 1)); 

		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-08-01 17:34) - PLID 44788 - Dropping down, so populate if necessary
void CGeneral2Dlg::OnDroppingDownReferringPatientCombo()
{
	try {
		KillTimer(IDT_FREEBIGDATALISTS);
		if (m_referringPatientCombo->GetRowCount() == 0 && !m_referringPatientCombo->IsRequerying()) {
			m_referringPatientCombo->Requery();

			NXDATALISTLib::IRowSettingsPtr pRow;

			pRow = m_referringPatientCombo->GetRow(-1);
			pRow->PutValue(0, (long)-1);
			pRow->PutValue(1, "<No Patient Selected>");
			m_referringPatientCombo->InsertRow(pRow,0);

			pRow = m_referringPatientCombo->GetRow(-1);
			pRow->PutValue(0, (long)-2);
			pRow->PutValue(1, "<Unspecified>");
			m_referringPatientCombo->InsertRow(pRow,1);

			TrySetReferringPatient();
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-08-01 17:34) - PLID 44788 - Dropping down, so populate if necessary
void CGeneral2Dlg::OnDroppingDownRelation()
{
	try {
		KillTimer(IDT_FREEBIGDATALISTS);
		if (m_pRelatedPatientList->GetRowCount() == 0 && !m_pRelatedPatientList->IsRequerying()) {
			m_pRelatedPatientList->Requery();
			
			NXDATALIST2Lib::IRowSettingsPtr pNewRow;
			
			pNewRow = m_pRelatedPatientList->GetNewRow();
			pNewRow->PutValue(0, long(-1));
			pNewRow->PutValue(1, "<No Relation>");
			m_pRelatedPatientList->AddRowSorted(pNewRow, NULL);
			
			TrySetRelatedPatient();
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-08-01 17:34) - PLID 44788 - After a certain amount of time, free the memory
void CGeneral2Dlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	try {
		if (!bShow) {
			SetTimer(IDT_FREEBIGDATALISTS, GetRemotePropertyInt("FreeBigDataListsDelay", 1200000, 0, "<None>", true), NULL);
		} else {
			KillTimer(IDT_FREEBIGDATALISTS);
		}
	} NxCatchAll(__FUNCTION__);
	
	CPatientDialog::OnShowWindow(bShow, nStatus);
}

// (a.walling 2011-08-04 14:36) - PLID 44788 - Delay freeing the datalist when hiding / deactivating
void CGeneral2Dlg::OnParentViewActivate(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	try {
		if (!bActivate) {
			SetTimer(IDT_FREEBIGDATALISTS, GetRemotePropertyInt("FreeBigDataListsDelay", 1200000, 0, "<None>", true), NULL);
		} else {
			KillTimer(IDT_FREEBIGDATALISTS);
		}
	} NxCatchAll(__FUNCTION__);
	
	CPatientDialog::OnParentViewActivate(bActivate, pActivateView, pDeactiveView);
}

// (a.walling 2011-08-01 17:34) - PLID 44788 - After a certain amount of time, free the memory
void CGeneral2Dlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == IDT_FREEBIGDATALISTS) {
		try {
			KillTimer(IDT_FREEBIGDATALISTS);

			if (m_referringPatientCombo) {
				m_referringPatientCombo->Clear();
				TrySetReferringPatient();
			}

			if (m_pRelatedPatientList) {
				m_pRelatedPatientList->Clear();
				TrySetRelatedPatient();
			}
		} NxCatchAll(__FUNCTION__);
	}

	CPatientDialog::OnTimer(nIDEvent);
}

// (j.gruber 2011-09-27 15:29) - PLID 45357
void CGeneral2Dlg::SelChosenAffiliatePhysList(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			long nAffilID = VarLong(pRow->GetValue(afclID));
			if (nAffilID == -1) {
				if (m_nAffiliatePhysID != -1) {

					ExecuteParamSql("UPDATE PatientsT SET AffiliatePhysID = NULL WHERE PersonID = {INT}", m_id);
					
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(m_id, GetExistingPatientName(m_id), nAuditID, aeiPatientAffiliatePhysician, m_id, m_strAffiliateName, "<No Affiliate Physician Selected>", aepMedium, aetChanged);
				}
			}
			else {
				if (nAffilID != m_nAffiliatePhysID) {
					ExecuteParamSql("UPDATE PatientsT SET AffiliatePhysID = {INT} WHERE PersonID = {INT}", nAffilID, m_id);
					
					long nAuditID = BeginNewAuditEvent();
					CString strNewValue = VarString(pRow->GetValue(afclName), "");
					AuditEvent(m_id, GetExistingPatientName(m_id), nAuditID, aeiPatientAffiliatePhysician, m_id, m_strAffiliateName, strNewValue, aepMedium, aetChanged);
				}
			}
			m_nAffiliatePhysID = nAffilID;
			m_strAffiliateName = VarString(pRow->GetValue(afclName), "");
		}


	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2011-09-27 15:29) - PLID 45357
void CGeneral2Dlg::RequeryFinishedAffiliatePhysList(short nFlags)
{
	try {

		//add a no row
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAffiliateList->GetNewRow();
		pRow->PutValue(afclID, (long)-1);
		pRow->PutValue(afclName, _variant_t("<No Affiliate Physician>"));
		m_pAffiliateList->AddRowBefore(pRow, m_pAffiliateList->GetFirstRow());

		if (m_nAffiliatePhysID == -1) {
			m_pAffiliateList->CurSel = pRow;
		}
		//we have to do this in case we came from a requery from the table checker
		else if (m_nAffiliatePhysID == -2) {
			m_pAffiliateList->PutComboBoxText(_bstr_t(m_strAffiliateName));
		}
		else {
			m_pAffiliateList->SetSelByColumn(afclID, m_nAffiliatePhysID);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2011-09-27 15:52) - PLID 45357
void CGeneral2Dlg::OnBnClickedGotoAffiliate()
{
	try {
		long nID = m_nAffiliatePhysID; 
		//make sure we have a valid selection
		if(nID == -2){
			//inactive
			_RecordsetPtr rs = CreateParamRecordset("SELECT AffiliatePhysID FROM PatientsT "
				"WHERE PersonID = {INT}", m_id);
			if(rs->eof){
				//I don't really see how this is possible, but oh well
				return;
			}
			else{
				nID = AdoFldLong(rs, "AffiliatePhysID", -1);
			}
		}		

		if (nID < 0) return;

		//do the flipping
		CMainFrame *p = GetMainFrame();
		CNxTabView *pView;

		if(p->FlipToModule(CONTACT_MODULE_NAME)) {

			if (nID != GetActiveContactID())
			{
				p->m_contactToolBar.SetActiveContactID(nID);
			}

			pView = (CNxTabView *)p->GetOpenView(CONTACT_MODULE_NAME);
			if (pView) 
			{	if(pView->GetActiveTab()==0)
					pView->UpdateView();
				else
					pView->SetActiveTab(0);
			}

			// (k.messina 2010-04-12 11:16) - PLID 37957 lock internal contact notes
			if(IsNexTechInternal()) {
				((CContactView*)GetMainFrame()->GetOpenView(CONTACT_MODULE_NAME))->CheckViewNotesPermissions();
			}
		}	
	}NxCatchAll(__FUNCTION__);
}


void CGeneral2Dlg::SelChangingAffiliatePhysList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-10-17 14:04) - PLID 47289
void CGeneral2Dlg::SelChosenAffPhysType(LPDISPATCH lpRow)
{
	try {
		
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			AffiliatePhysType nAffilTypeID = (AffiliatePhysType)VarLong(pRow->GetValue(aptcID));
			if (nAffilTypeID == afptNone) {
				if (m_nAffiliatePhysTypeID != afptNone) {

					ExecuteParamSql("UPDATE PatientsT SET AffiliatePhysType = NULL WHERE PersonID = {INT}", m_id);
					
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(m_id, GetExistingPatientName(m_id), nAuditID, aeiPatientAffiliatePhysicianType, m_id, GetAffiliateTypeDescription((AffiliatePhysType) m_nAffiliatePhysTypeID), "<No Affiliate Physician Type Selected>", aepMedium, aetChanged);
				}
			}
			else {
				if (nAffilTypeID != m_nAffiliatePhysTypeID) {
					ExecuteParamSql("UPDATE PatientsT SET AffiliatePhysType = {INT} WHERE PersonID = {INT}", nAffilTypeID, m_id);
					
					long nAuditID = BeginNewAuditEvent();
					CString strNewValue = GetAffiliateTypeDescription((AffiliatePhysType) nAffilTypeID);
					AuditEvent(m_id, GetExistingPatientName(m_id), nAuditID, aeiPatientAffiliatePhysicianType, m_id, GetAffiliateTypeDescription((AffiliatePhysType) m_nAffiliatePhysTypeID), strNewValue, aepMedium, aetChanged);
				}
			}
			m_nAffiliatePhysTypeID = nAffilTypeID;		
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2012-10-17 14:04) - PLID 47289
void CGeneral2Dlg::SelChangingAffPhysType(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.luckoski 2013-03-04 11:49) - PLID 33548 - Set the check for reward prompts
void CGeneral2Dlg::OnBnClickedWarningCheck2()
{
	if (UserPermission(ChangeWarning))
	{	
		long nCheck = m_RewardsWarningCheck.GetCheck();

		if (nCheck != 0) {
			GetDlgItem(IDC_ADJUST_REWARD_POINTS)->SetFocus();
		}
		SetRewardsWarningBox();
					
	}
	else {
		//TS: Check this based on the data, NOT the current state of the checkbox, because in certain circumstances
		//the permission stuff can change the state of the checkbox
		_RecordsetPtr rsWarning = CreateRecordset("SELECT DisplayRewardsWarning FROM PersonT WHERE ID = %li", m_id);
		BOOL bCheck = AdoFldBool(rsWarning, "DisplayRewardsWarning");
		m_RewardsWarningCheck.SetCheck(bCheck);
	}
}

// (b.spivey, May 13, 2013) - PLID 56872
void CGeneral2Dlg::OnRequeryFinishedRaceList(short nFlags)
{
	try
	{
		//The datalist requeries every time, so we add the "no row" and "multi rows" here. 
		NXDATALISTLib::IRowSettingsPtr pRow;
		_variant_t var;
		//No Row. 
		pRow = m_raceCombo->GetRow(-1);
		var.vt = VT_NULL;
		pRow->PutValue(raceID, var);
		pRow->PutValue(raceName, _bstr_t(l_strNoRaceText));
		pRow->PutValue(raceRaceName,"");
		m_raceCombo->InsertRow(pRow,0);

		//Multi-Row. 
		pRow = m_raceCombo->GetRow(-1); 
		pRow->PutValue(raceID, rcrMultiSelectRow);
		pRow->PutValue(raceName, _bstr_t(l_strMultipleRaceText));
		pRow->PutValue(raceRaceName, "");
		m_raceCombo->InsertRow(pRow, 1);

		//If we have more than one race, set the selection to the multi row-- toggle labels will handle the rest.
		if(m_naryRaceIDs.GetCount() > 1) {
			m_raceCombo->SetSelByColumn(raceID, rcrMultiSelectRow);
		}
		//If we have one race, set the selection to the race.
		else if (m_naryRaceIDs.GetCount() == 1) {
			m_raceCombo->SetSelByColumn(raceID, m_naryRaceIDs.GetAt(0)); 
		}
		//If we have no race, set it to sriNoRow. 
		else {
			m_raceCombo->CurSel = NXDATALISTLib::sriNoRow; 
		}

		ToggleLabels(); 
	}NxCatchAll(__FUNCTION__);
}

// (b.spivey, May 28, 2013) - PLID 56872 - Toggle the labels to list multiple selections for certain datalists. 
void CGeneral2Dlg::ToggleLabels() 
{
	//RACE 
	//If we have multiple races then we should create a label. 
	if (m_naryRaceIDs.GetCount() > 1) { 

		CString strRaceLabel = "";
		CString strRaceToolTip = "";
		int nCount = 0, nSize = 0; 
		nSize = m_naryRaceIDs.GetSize(); 

		//Create the label using the loaded data list
		while(nCount < nSize) {
			long nRow = m_raceCombo->FindByColumn(0, _variant_t(m_naryRaceIDs.GetAt(nCount)), 0, FALSE);
			nCount++;
			if (nRow == NXDATALISTLib::sriNoRow) {
				continue; 
			}
			
			//Label is one line, ToolTip is multiline. 
			strRaceLabel += VarString(m_raceCombo->GetValue(nRow, raceName)) + ", ";
			strRaceToolTip += VarString(m_raceCombo->GetValue(nRow, raceName)) + "\r\n";
		}
		strRaceLabel.Trim(", "); 
		strRaceToolTip.Trim("\r\n"); 

		m_nxstaticMultiRaceLabel.SetText(strRaceLabel); 
		m_nxstaticMultiRaceLabel.SetToolTip(strRaceToolTip); 

		//Hide the datalist, show the label. 
		m_nxstaticMultiRaceLabel.EnableWindow(TRUE);
		m_nxstaticMultiRaceLabel.ShowWindow(SW_SHOWNA); 
		m_raceCombo->Enabled = FALSE;
		GetDlgItem(IDC_CODE_RACE)->ShowWindow(SW_HIDE); 

	}
	else {
		CString strRaceLabel = "";
		CString strRaceToolTip = "";

		m_nxstaticMultiRaceLabel.SetText(strRaceLabel); 
		m_nxstaticMultiRaceLabel.SetToolTip(strRaceToolTip); 

		//hide the label, show the datalist. 
		m_nxstaticMultiRaceLabel.EnableWindow(FALSE);
		m_nxstaticMultiRaceLabel.ShowWindow(SW_HIDE); 
		m_raceCombo->Enabled = TRUE;
		GetDlgItem(IDC_CODE_RACE)->ShowWindow(SW_SHOW); 

		//Special case, if they selected one from the multi-select dialog then I want to select it.
		//		If they selected none, I want nothing selected.
		if (m_naryRaceIDs.GetCount() == 0) {
			m_raceCombo->CurSel = NXDATALISTLib::sriNoRow; 
		}
		else if (m_naryRaceIDs.GetCount() == 1) {
			m_raceCombo->SetSelByColumn(raceID, m_naryRaceIDs.GetAt(0)); 
		}
	}
}


// (b.spivey, May 17, 2013) - PLID 56872 - Change the cursor so they know it's a link. 
BOOL CGeneral2Dlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try {
		//RACE LABEL
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if(m_nxstaticMultiRaceLabel.IsWindowVisible() && m_nxstaticMultiRaceLabel.IsWindowEnabled())
		{
			m_nxstaticMultiRaceLabel.GetWindowRect(rc);
			ScreenToClient(&rc);
			//Only if we're over the label does the cursor change. 
			if(rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}

		}
	}NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (b.spivey, May 17, 2013) - PLID 56872
LRESULT CGeneral2Dlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{
		switch(wParam) {
			// (b.spivey, May 28, 2013) - PLID 56872 - If they clicked the race label, then we 
			//		change and audit the patient's races based on the user's interactions. 
			case (IDC_RACE_LABEL):
			{
				MultiSelectRaceDlg(); 

				if(m_naryRaceIDs.GetCount() == 0) {
					m_raceCombo->CurSel = NXDATALISTLib::sriNoRow; 
				}
				else if (m_naryRaceIDs.GetCount() == 1) {
					m_raceCombo->SetSelByColumn(raceID, m_naryRaceIDs.GetAt(0)); 
				}


				CSqlFragment sqlInsertRace(" "); 	
				for (int i = 0; i < m_naryRaceIDs.GetCount(); i++) {
					sqlInsertRace += CSqlFragment("INSERT INTO PersonRaceT (RaceID, PersonID) VALUES ({INT}, @PersonID) ", 
														m_naryRaceIDs.GetAt(i));
				}

				_RecordsetPtr prs = CreateParamRecordset(
					"SET NOCOUNT ON "
					"DECLARE @PersonID INT "
					"SET @PersonID = {INT} "
					" "
					" "
					"SELECT LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) AS OldRaceName "
					"FROM "
					"( "
					"	SELECT ( "
					"		SELECT RT.Name + ', ' "
					"		FROM PersonRaceT PRT "
					"		INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
					"		WHERE PRT.PersonID = @PersonID  "
					"		FOR XML PATH(''), TYPE "
					"	).value('/','nvarchar(max)') "
					") RaceSubQ (RaceName) "
					" "
					" "
					"DELETE FROM PersonRaceT WHERE PersonID = @PersonID "
					" " 
					" "
					" {SQL} "
					" "
					" "
					"SELECT LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) AS NewRaceName "
					"FROM "
					"( "
					"	SELECT ( "
					"		SELECT RT.Name + ', ' "
					"		FROM PersonRaceT PRT "
					"		INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
					"		WHERE PRT.PersonID = @PersonID  "
					"		FOR XML PATH(''), TYPE "
					"	).value('/','nvarchar(max)') "
					") RaceSubQ (RaceName) "
					"SET NOCOUNT OFF\r\n",
					m_id, sqlInsertRace);

				if(!prs->eof) {
					CString strOldValue = AdoFldString(prs, "OldRaceName", l_strNoRaceText);
					prs = prs->NextRecordset(NULL); 
					CString strNewValue = l_strNoRaceText; 
					if(!prs->eof) {
						strNewValue = AdoFldString(prs, "NewRaceName", l_strNoRaceText);
					}

					//DRT 6/30/2005 - PLID 16654 - Auditing
					if(strOldValue.Compare(strNewValue) != 0) {
						long nAuditID = BeginNewAuditEvent();
						// (j.jones 2009-10-14 14:31) - PLID 34327 - renamed this audit
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiPatientRace, m_id, strOldValue, strNewValue, aepMedium, aetChanged);
						UpdateChangedDate(nAuditID);
						UpdateExistingPatientInHL7(m_id);
					}
				}


				ToggleLabels(); 
				break; 
			}

			default:
			{
				break;
			}
		}
		ToggleLabels(); 
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (b.spivey, May 28, 2013) - PLID 56872 - Function to show the multi-select dialog and let the user make selections. 
bool CGeneral2Dlg::MultiSelectRaceDlg()
{
	CMultiSelectDlg dlg(this, "Gen2MultiRaceSelectDlg");
	dlg.PreSelect(m_naryRaceIDs); 
	if (dlg.Open("RaceT", "", "ID", "Name", "Please select the patient's race(s)") == IDOK) {
		m_naryRaceIDs.RemoveAll(); 
		dlg.FillArrayWithIDs(m_naryRaceIDs); 
	}
	else {
		return false;
	}
	return true; 
}
void CGeneral2Dlg::SelChosenOccupationCensusCode(LPDISPATCH lpRow)
{
	try {
		//TES 11/13/2013 - PLID 59483 - Save the Census Code they selected
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			ExecuteParamSql("UPDATE PatientsT SET OccupationCensusCodeID = NULL WHERE PersonID = {INT}", m_id);
		}
		else {
			ExecuteParamSql("UPDATE PatientsT SET OccupationCensusCodeID = {INT} WHERE PersonID = {INT}", VarLong(pRow->GetValue(occlID)),m_id);
		}
	}NxCatchAll(__FUNCTION__);
}

void CGeneral2Dlg::SelChosenIndustryCensusCode(LPDISPATCH lpRow)
{
	try {
		//TES 11/13/2013 - PLID 59483 - Save the Census Code they selected
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			ExecuteParamSql("UPDATE PatientsT SET IndustryCensusCodeID = NULL WHERE PersonID = {INT}", m_id);
		}
		else {
			ExecuteParamSql("UPDATE PatientsT SET IndustryCensusCodeID = {INT} WHERE PersonID = {INT}", VarLong(pRow->GetValue(icclID)),m_id);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-02-18 16:54) - PLID 60719 - added the diagnosis search control
void CGeneral2Dlg::OnSelChosenDefaultDiagSearchList(LPDISPATCH lpRow)
{
	try {

		if(lpRow) {
			CDiagSearchResults results = DiagSearchUtils::ConvertPreferenceSearchResults(lpRow);

			if(results.m_ICD9.m_nDiagCodesID == -1 && results.m_ICD10.m_nDiagCodesID == -1) {
				//no code selected
				return;
			}
			
			//if we have 4 codes, do not add a 5th
			long nCountCodes = 0;
			long nFirstEmptyIndex = -1;
			NXDATALIST2Lib::IRowSettingsPtr pFirstEmptyRow = NULL;
			{
				
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_defaultDiagList->GetFirstRow();
				while(pRow) {
					long nDiagID9 = VarLong(pRow->GetValue(dclcDiagICD9CodeID), -1);
					long nDiagID10 = VarLong(pRow->GetValue(dclcDiagICD10CodeID), -1);
					if(nDiagID9 != -1 || nDiagID10 != -1) {
						nCountCodes++;
					}
					else if(nFirstEmptyIndex == -1) {
						pFirstEmptyRow = pRow;
						nFirstEmptyIndex = VarLong(pFirstEmptyRow->GetValue(dclcIndex));
					}
					pRow = pRow->GetNextRow();
				}
			}

			if(nCountCodes >= 4) {
				AfxMessageBox("No more than four default diagnosis codes can be added to a patient.");
				return;
			}

			if(pFirstEmptyRow == NULL || nFirstEmptyIndex == -1 || nFirstEmptyIndex > 4) {
				//above code should have caught this!
				ASSERT(FALSE);
				AfxMessageBox("No more than four default diagnosis codes can be added to a patient.");
				return;
			}

			CString strExecute = "UPDATE PatientsT SET DefaultDiagID[INDEX] = {VT_I4}, DefaultICD10DiagID[INDEX] = {VT_I4} WHERE PersonID = {INT}";
			strExecute.Replace("[INDEX]", AsString(nFirstEmptyIndex));

			_variant_t varICD9 = g_cvarNull;
			_variant_t varICD10 = g_cvarNull;
			if(results.m_ICD9.m_nDiagCodesID != -1) {
				varICD9 = results.m_ICD9.m_nDiagCodesID;
			}
			if(results.m_ICD10.m_nDiagCodesID != -1) {
				varICD10 = results.m_ICD10.m_nDiagCodesID;
			}

			//don't add identical duplicates
			NXDATALIST2Lib::IRowSettingsPtr pFoundRow = m_defaultDiagList->FindByColumn(dclcDiagICD9CodeID, results.m_ICD9.m_nDiagCodesID, m_defaultDiagList->GetFirstRow(), VARIANT_FALSE);
			while(pFoundRow) {
				//do the codes match?
				if(VarLong(pFoundRow->GetValue(dclcDiagICD9CodeID), -1) == results.m_ICD9.m_nDiagCodesID 
					&& VarLong(pFoundRow->GetValue(dclcDiagICD10CodeID), -1) == results.m_ICD10.m_nDiagCodesID) {
					AfxMessageBox("This diagnosis has already been selected.");
					return;
				}
				//now iterate through all remaining rows, the above check
				//will compare 9 and 10 codes
				pFoundRow = pFoundRow->GetNextRow();
			}

			ExecuteParamSql(strExecute, varICD9, varICD10, m_id);

			//update the row
			{
				pFirstEmptyRow->PutValue(dclcDiagICD9CodeID, varICD9);
				pFirstEmptyRow->PutValue(dclcDiagICD9Code, _bstr_t(results.m_ICD9.m_strCode));
				pFirstEmptyRow->PutValue(dclcDiagICD9Desc, _bstr_t(results.m_ICD9.m_strDescription));
				pFirstEmptyRow->PutValue(dclcDiagICD10CodeID, varICD10);
				pFirstEmptyRow->PutValue(dclcDiagICD10Code, _bstr_t(results.m_ICD10.m_strCode));
				pFirstEmptyRow->PutValue(dclcDiagICD10Desc, _bstr_t(results.m_ICD10.m_strDescription));
				m_defaultDiagList->Sort();
			}

			// this uses the PatG1 tablechecker as a general
			// "this patient's demographics have changed" message
			CClient::RefreshTable(NetUtils::PatG1, m_id);
		
			// UPDATE THE PALM RECORD
			UpdatePalm(TRUE);

			//should be impossible to add rows that require the columns
			//to resize, but check the column sizes for posterity
			UpdateDiagnosisListColumnSizes();
		}

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-02-18 17:00) - PLID 60719 - the diagnosis list
// should ideally be fixed to match the search style, which is
// controlled by a preference, but if it is only showing ICD-9
// or ICD-10, it needs to dynamically change if codes exist
// for the hidden columns
void CGeneral2Dlg::UpdateDiagnosisListColumnSizes()
{
	try {

		// (j.jones 2014-02-20 08:34) - PLID 60907 - this is now a global function
		DiagSearchUtils::SizeDiagnosisListColumnsBySearchPreference(m_defaultDiagList, dclcDiagICD9Code, dclcDiagICD10Code,
			50, 50, "", "", dclcDiagICD9Desc, dclcDiagICD10Desc, false, true, true);

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-02-19 11:23) - PLID 60864 - added right click ability to remove default codes
void CGeneral2Dlg::OnRButtonDownDefaultDiagCodeList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		m_defaultDiagList->CurSel = pRow;

		//if no codes are selected, do nothing
		long nDiagID9 = VarLong(pRow->GetValue(dclcDiagICD9CodeID), -1);
		long nDiagID10 = VarLong(pRow->GetValue(dclcDiagICD10CodeID), -1);
		if(nDiagID9 == -1 && nDiagID10 == -1) {
			return;
		}

		long nIndexToRemove = VarLong(pRow->GetValue(dclcIndex));

		enum {
			eRemoveDiag = 1,
		};

		// Create the menu
		CMenu mnu;
		mnu.CreatePopupMenu();
		CString strLabel;
		strLabel.Format("&Remove Default Diagnosis Code %li", nIndexToRemove);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, eRemoveDiag, strLabel);

		CPoint pt;
		GetCursorPos(&pt);

		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		if(nRet == eRemoveDiag) {

			//check permissions
			if (!CheckCurrentUserPermissions(bioPatient, sptWrite)) {
				return;
			}

			//remove this index, and move up any diagnoses below it

			CString strSqlBatch;
			CNxParamSqlArray aryParams;

			CString strDelete = "UPDATE PatientsT SET DefaultDiagID[INDEX] = NULL, DefaultICD10DiagID[INDEX] = NULL WHERE PersonID = {INT}";
			strDelete.Replace("[INDEX]", AsString(nIndexToRemove));
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, strDelete, m_id);

			//update the remaining indexes, if any, up through index 4
			for(int i=nIndexToRemove+1; i<=4; i++) {
				CString strUpdate = "UPDATE PatientsT SET DefaultDiagID[INDEXNEW] = DefaultDiagID[INDEXOLD], "
					"DefaultICD10DiagID[INDEXNEW] = DefaultICD10DiagID[INDEXOLD], "
					"DefaultDiagID[INDEXOLD] = NULL, "
					"DefaultICD10DiagID[INDEXOLD] = NULL "
					"WHERE PersonID = {INT}";
				strUpdate.Replace("[INDEXNEW]", AsString(i-1));
				strUpdate.Replace("[INDEXOLD]", AsString(i));
				AddParamStatementToSqlBatch(strSqlBatch, aryParams, strUpdate, m_id);
			}

			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

			//Requery to update our row and reflect any index changes
			//of later diagnosis codes. OnRequeryFinished will resize columns
			//as needed.
			m_defaultDiagList->Requery();

			// this uses the PatG1 tablechecker as a general
			// "this patient's demographics have changed" message
			CClient::RefreshTable(NetUtils::PatG1, m_id);
		
			// UPDATE THE PALM RECORD
			UpdatePalm(TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-03-06 11:35) - PLID 60719 - resize the columns to reflect the new data
void CGeneral2Dlg::OnRequeryFinishedDefaultDiagCodeList(short nFlags)
{
	try {

		UpdateDiagnosisListColumnSizes();

	}NxCatchAll(__FUNCTION__);
}

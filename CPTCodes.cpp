// CPTCodes.cpp : implementation file
//

#include "stdafx.h"
#include "ClaimSetupDLG.h"// (s.tullis 2014-05-21 10:27) - PLID 62018 - Remove the Claim Setup infomation from the Admin Billing dialog to a new dialog that launchs from the Additional Service Code Setup Menu
#include "CPTCodes.h"
#include "AnesthesiaFacilitySetupDLG.h"// (s.tullis 2014-05-21 10:27) - PLID 62023 - Remove the Anesthesia/Facility Setup fuctionality from the Admin Billing dialog to a new dialog that launchs from the Additional Service Code Setup Menu
#include "CPTAddNew.h"
#include "ModifierAddNew.h"
#include "DiagAddNew.h"
#include "PayCatDlg.h"
#include "GlobalFinancialUtils.h"
#include "InactiveServiceItems.h"
#include "InactiveICD9sDlg.h"
#include "AuditTrail.h"
#include "UpdatePriceDlg.h"
#include "ReceiptConfigDlg.h"
#include "MultiCategoryUpdateDlg.h"
#include "Barcode.h"
#include "ConfigBillColumnsDlg.h"
#include "AdvRevCodeSetupDlg.h"
#include "AdditionalServiceCodesSetup.h"
#include "MultipleRevCodesDlg.h" 
#include "UBSetupDLG.h"
#include "CPTDiagnosisLinkingDlg.h"
#include "UpdateFeesByRVUDlg.h"
#include "CategorySelectDlg.h"
#include "VersionInfo.h"
#include "ImportAMACodesDlg.h"
#include "AnesthesiaSetupDlg.h"
#include "UpdateMultipleTOSDlg.h"
#include "ChargeInterestConfigDlg.h"
#include "FacilityFeeConfigDlg.h"
#include "ProcedureDescriptionDlg.h"
#include "MultiFeesImportDlg.h"
#include "DiscountsSetupDlg.h"
#include "InactiveModifiersDlg.h"
#include "ICD9ProcedureSetupDlg.h"
#include "MultiServiceModifierLinkDlg.h"
#include "OHIPImportCodesDlg.h"
#include "OHIPPremiumCodesDlg.h"
#include "OHIPUtils.h"
#include "PayGroupsEditDlg.h"
#include "AdvServiceCodePayGroupSetupDlg.h"
#include "AssistingCodesSetupDlg.h"
#include "BillableCPTCodesDlg.h"
#include "AlbertaHLINKUtils.h"
#include "NDCDefSetupDlg.h"	// (j.dinatale 2012-06-12 13:20) - PLID 32795
#include "EditShopFeesDlg.h" // (j.gruber 2012-10-22 16:56) - PLID 53240
#include "PrescriptionUtilsNonAPI.h"
#include "NxAPI.h"
#include "CPTAdditionalInfoDlg.h" // (a.wilson 2014-01-13 15:53) - PLID 59956
#include "ImportICD10CodesDlg.h"	// (j.armen 2014-03-10 08:24) - PLID 61210
#include "OHIPSetupCPTDLG.h"// (s.tullis 2014-05-21 10:30) - PLID 62120 - Make a new OHIP setup dialog to be called from the additional service code setup menu.
#include "UpdateMultiCCDATypeDlg.h"
#include "DiagSearchUtils.h"
#include "DiagSearchConfig_FullICD9Search.h"
#include "DiagSearchConfig_FullICD10Search.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.armen 2014-03-06 09:59) - PLID 61209 - The default is now based on whether we are viewing 9 or 10 codes
#define DEFAULT_ICD9_WHERE_CLAUSE	(FormatBstr("(Active = 1 AND ICD10 = %li)", m_checkDiagICD9.GetCheck() == BST_CHECKED ? 0 : 1))
#define DEFAULT_CPT_WHERE_CLAUSE	"ServiceT.Active = 1"

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_UPDATE_BY_PERCENT	52743
#define ID_UPDATE_BY_RVU		52744
#define ID_UPDATE_FROM_FILE		52745

//(c.copits 2011-09-08) PLID 41478 - Allow global periods on service codes to exceed one year.
#define MAX_GLOBAL_PERIOD_DAYS	1825000		// Must not generate overflow when performing date / time calculations

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;
using namespace NXDATALISTLib;

// (j.jones 2010-07-30 10:35) - PLID 39728 - added pay group enum
enum PayGroupComboColumns {

	pgccID = 0,
	pgccName,
};
// (d.singleton 2011-09-23 12:16) - PLID 44947 - add enum for alberta mods list
enum AlbertaModsColumns {
	amMod = 0,
	amDescription,
	amAction,
};

// (j.jones 2012-01-03 15:08) - PLID 47300 - added NOC combo
enum NOCColumns {
	noccID = 0,
	noccName,
};

//TES 5/14/2013 - PLID 56631 - Created an enum for the DiagCode list
// (r.gonet 03/06/2014) - PLID 61129 - Removed ability to import from FDB and related controls
enum DiagCodeListColumns {
	dclcCodeNumber = 0,
	dclcCodeDesc = 1,
	dclcID = 2,
	dclcIsAMA = 3,
	dclcIsICD10 = 4,	// (j.jones 2013-11-21 14:48) - PLID 59640 - added column for ICD-10
};

// (c.haag 2015-05-13) - PLID 66017
enum CustomNexGEMsColumns {
	cngICD9ID = 0,
	cngICD9Code = 1,
	cngICD9Desc = 2,
	cngICD10ID = 3,
	cngICD10Code = 4,
	cngICD10Desc = 5,
	cngQuicklist = 6, // (c.haag 2015-05-15) - PLID 66021
};

// (c.haag 2015-07-14) - PLID 66017
enum AdminDisplayICDCodeVersionPreference 
{
	adicvpICD9Codes = 9,
	adicvpICD10Codes = 10,
	adicvpCustomNexGEMs = 0,
};

// (a.wilson 2014-01-14 14:46) - PLID 59956 - reusable function for saving values.
static _variant_t NullOrIntStringToVariant(const CString & str)
{
	if (str == "NULL" || str.IsEmpty())
		return g_cvarNull;
	else 
		return atol(str);
}

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CCPTCodes dialog


CCPTCodes::CCPTCodes(CWnd* pParent)
	: CNxDialog(CCPTCodes::IDD, pParent),
	m_CPTChecker(NetUtils::CPTCodeT),
	m_ModChecker(NetUtils::CPTModifierT),
	m_ICD9Checker(NetUtils::DiagCodes),
	m_CPTCategoryChecker(NetUtils::CPTCategories),
	m_UB92CatChecker(NetUtils::UB92Cats),
	m_providerChecker(NetUtils::Providers),
	m_PayGroupChecker(NetUtils::ServicePayGroupsT),	// (j.jones 2010-07-30 16:15) - PLID 39728
	m_CustomNexGEMsChecker(NetUtils::CustomNexGEMs), // (c.haag 2015-05-15) - PLID 66017
	m_nICD9IDForCustomNexGEM(-1), // (c.haag 2015-05-14) - PLID 66019
	m_nICD10IDForCustomNexGEM(-1), // (c.haag 2015-05-14) - PLID 66019
	m_pICD9WithPCSSearchConfig(NULL), // (c.haag 2015-05-14) - PLID 66019
	m_pICD10WithPCSSearchConfig(NULL) // (c.haag 2015-05-14) - PLID 66019
{
	//{{AFX_DATA_INIT(CCPTCodes)
	m_IsModal = FALSE;
	m_bIsSavingCPT = FALSE;
	m_CurServiceID = -1;
	m_bUseOHIP = FALSE;
	//}}AFX_DATA_INIT

	// (a.wilson 2014-01-14 12:04) - PLID 59956 - default values
	m_eCCDAType = cptInvalidType;

	m_bEditChanged = FALSE;
	
	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "System_Setup/Billing_Setup/Insurance_Billing_Setup/add_a_cpt_code.htm";
}

// (j.jones 2007-07-03 15:05) - PLID 26098 - added option to link service codes and modifiers
//DRT 4/2/2008 - PLID 29518 - Converted all edit controls to CNxEdit
void CCPTCodes::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CATEGORY_BOX, m_editCategory);
	
	//DDX_Control(pDX, IDC_ANESTH_BASE_UNITS, m_editAnesthBaseUnits);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	DDX_Control(pDX, IDC_CPT_BARCODE, m_editBarcode);
	DDX_Control(pDX, IDC_GLOBAL_PERIOD, m_editGlobalPeriod);
	DDX_Control(pDX, IDC_RVU, m_editRVU);
	DDX_Control(pDX, IDC_STD_FEE, m_editStdFee);
	DDX_Control(pDX, IDC_SHOP_FEE, m_editShopFee);
	DDX_Control(pDX, IDC_DESCRIPTION, m_editDescription);
	DDX_Control(pDX, IDC_PROCEDURE_DESCRIPTION, m_btnProcedureDescription);
	DDX_Control(pDX, IDC_BTN_SERVICE_MODIFIER_LINKING, m_btnServiceModifierLinking);
	//DDX_Control(pDX, IDC_BTN_SETUP_ICD9V3, m_btnSetupICD9v3);// (s.tullis 2014-05-22 09:13) - PLID 61829 - moved
	DDX_Control(pDX, IDC_BTN_DISCOUNT_CATEGORIES, m_btnDiscountCategories);
	//DDX_Control(pDX, IDC_CHECK_USE_FACILITY_BILLING, m_checkUseFacilityBilling);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//DDX_Control(pDX, IDC_CHECK_USE_ANESTHESIA_BILLING, m_checkUseAnesthesiaBilling);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//DDX_Control(pDX, IDC_BTN_FACILITY_FEE_SETUP, m_btnFacilityFeeSetup);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//DDX_Control(pDX, IDC_CHECK_FACILITY, m_checkFacility);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	DDX_Control(pDX, IDC_CONFIGURE_FINANCE_CHARGE_SETTINGS, m_btnConfigFinanceCharges);
	DDX_Control(pDX, IDC_UPDATE_TOS, m_btnUpdateTOS);
	//DDX_Control(pDX, IDC_BTN_ANESTHESIA_SETUP, m_btnAnesthesiaSetup);//s.tullis
	DDX_Control(pDX, IDC_BTN_IMPORT_AMA, m_btnImportAMA);
	DDX_Control(pDX, IDC_BTN_LAUNCH_CODELINK, m_btnLaunchCodeLink);
	DDX_Control(pDX, IDC_BTN_REMOVE_CATEGORY, m_btnRemoveCategory);
	DDX_Control(pDX, IDC_BTN_SELECT_CATEGORY, m_btnSelectCategory);
	DDX_Control(pDX, IDC_BTN_CPT_DIAGNOSIS_LINKING, m_btnCPTDiagnosisLinking);
	DDX_Control(pDX, IDC_BTN_IMPORT_CPT, m_btnImportCPT);
	//DDX_Control(pDX, IDC_CHECK_ANESTHESIA, m_checkAnesthesia);//s.tullis
	//DDX_Control(pDX, IDC_BTN_EDIT_MULTIPLE_REV_CODES, m_btnEditMultipleRevCodes);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//DDX_Control(pDX, IDC_CHECK_MULTIPLE_REV_CODES, m_checkMultipleRevCodes);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//DDX_Control(pDX, IDC_CHECK_SINGLE_REV_CODE, m_checkSingleRevCode);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	DDX_Control(pDX, IDC_ADD_SERVICE_CODE_SETUP, m_btnAdditionalServiceCodeSetup);// (s.tullis 2014-05-01 15:30) - PLID 61939 - Add “Additional Service Code Setup” button to the bottom of the CPT Configuration screen.
	//DDX_Control(pDX, IDC_ADV_REVCODE_SETUP, m_btnAdvRevCodeSetup);// (s.tullis 2014-05-21 16:59) - PLID 61829  moved
	DDX_Control(pDX, IDC_CONFIG_BILL_COLUMNS, m_btnConfigBillColumns);
	DDX_Control(pDX, IDC_FILTER_ICD9, m_filtericd9Button);
	DDX_Control(pDX, IDC_FILTER_CPT, m_filterCptButton);
	DDX_Control(pDX, IDC_UPDATE_CATEGORIES, m_btnUpdateCats);
	DDX_Control(pDX, IDC_RECEIPT_CONFIG, m_ReceiptConfig);
	DDX_Control(pDX, IDC_MARK_ICD9_INACTIVE, m_btnMarkICD9Inactive);
	DDX_Control(pDX, IDC_UPDATE_PRICES, m_btnUpdateFees);
	DDX_Control(pDX, IDC_RIGHT_CPT, m_btnCptRight);
	DDX_Control(pDX, IDC_LEFT_CPT, m_btnCptLeft);
	DDX_Control(pDX, IDC_PAYMENTCATEGORY, m_btnPayCats);
	DDX_Control(pDX, IDC_MARK_INACTIVE, m_markInactiveButton);
	DDX_Control(pDX, IDC_DELETE_CPT, m_deleteCPTButton);
	DDX_Control(pDX, IDC_DELETE_MODIFIER, m_deleteModifierButton);
	DDX_Control(pDX, IDC_DELETE_ICD9, m_deleteICD9Button);
	DDX_Control(pDX, IDC_ADD_MODIFIER, m_addModifierButton);
	DDX_Control(pDX, IDC_ADD_ICD9, m_addicd9Button);
	DDX_Control(pDX, IDC_ADD_CPT, m_addCptButton);
	DDX_Control(pDX, IDC_BTN_ADD_CUSTOM_MAPPING, m_addCustomMappingButton);
	DDX_Control(pDX, IDC_TAXABLE_2, m_Taxable2);
	DDX_Control(pDX, IDC_TAXABLE_1, m_Taxable1);
	DDX_Control(pDX, IDC_CPTSUBCODE, m_CPTSubCode);
	DDX_Control(pDX, IDC_CPTCODE, m_CPTCode);
	DDX_Control(pDX, IDC_AMA_TEXT_LABEL, m_nxlAMA);
	DDX_Control(pDX, IDC_MARK_MODIFIER_INACTIVE, m_btnMarkModifierInactive);
	DDX_Control(pDX, IDC_INACTIVE_CODES, m_btnInactiveCodes);
	//DDX_Control(pDX, IDC_UB92_CPT_LABEL, m_nxstaticUb92CptLabel);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	DDX_Control(pDX, IDC_SHOP_FEE_LABEL, m_nxstaticShopFeeLabel);
	//DDX_Control(pDX, IDC_ICD9V3_LABEL, m_nxstaticIcd9v3Label);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//DDX_Control(pDX, IDC_BTN_OHIP_PREMIUM_CODES, m_btnOHIPPremiumCodes);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	DDX_Control(pDX, IDC_BTN_EDIT_PAY_GROUPS, m_btnEditPayGroups);
	DDX_Control(pDX, IDC_BTN_ADV_PAY_GROUP_SETUP, m_btnAdvPayGroupSetup);
	//DDX_Control(pDX, IDC_CHECK_BATCH_WHEN_ZERO, m_checkBatchWhenZero);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//DDX_Control(pDX, IDC_CHECK_ASSISTING_CODE, m_checkAssistingCode);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//DDX_Control(pDX, IDC_ASSISTING_BASE_UNITS, m_editAssistingBaseUnits);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	DDX_Control(pDX, IDC_CPT_BILLABLE, m_checkBillable);
	DDX_Control(pDX, IDC_BTN_EDIT_BILLABLE_CODES, m_btnAdvBillableCPTSetup);
	// (j.jones 2011-07-06 15:57) - PLID 44358 - added UB Box 4// (s.tullis 2014-05-22 09:13) - PLID 61829 - moved
	//DDX_Control(pDX, IDC_UB_BOX4, m_editUBBox4);
	//DDX_Control(pDX, IDC_NOC_LABEL, m_nxlNOC);
	// (r.gonet 03/26/2012) - PLID 49043 - Added require referring physician checkbox
	//DDX_Control(pDX, IDC_CPT_REQUIRE_REF_PHYS, m_checkRequireRefPhys);// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	DDX_Control(pDX, IDC_CPT_TOS, m_editTOS); // (s.tullis 2014-05-19 09:37) - PLID 61829 - In the Administrator Module > Billing Tab, reorganize the service codes area per Jacob's design. (Master Item)
	DDX_Control(pDX, IDC_CPT_CODES_DIAG_ICD9, m_checkDiagICD9);		// (j.armen 2014-03-05 17:39) - PLID 61207
	DDX_Control(pDX, IDC_CPT_CODES_DIAG_ICD10, m_checkDiagICD10);	// (j.armen 2014-03-05 17:39) - PLID 61207
	DDX_Control(pDX, IDC_CPT_CODES_DIAG_CUSTOM_NEXGEMS, m_checkDiagCustomNexGEMs);	// (c.haag 2015-05-11) - PLID 66017
}


BEGIN_MESSAGE_MAP(CCPTCodes, CNxDialog)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_EN_KILLFOCUS(IDC_CPTCODE, OnKillfocusCPTCode)
	ON_EN_KILLFOCUS(IDC_CPTSUBCODE, OnKillfocusCPTSubcode)
	ON_EN_KILLFOCUS(IDC_STD_FEE, OnKillfocusStdFee)
	ON_BN_CLICKED(IDC_PAYMENTCATEGORY, OnPaymentCategory)
	ON_COMMAND(ID_MOD_ADD, OnMenuAddModifier)
	ON_COMMAND(ID_MOD_DELETE, OnMenuDeleteModifier)
	ON_COMMAND(ID_ICD_ADD, OnMenuAddICD9)
	ON_COMMAND(ID_ICD_DELETE, OnMenuDeleteICD9)
	ON_EN_KILLFOCUS(IDC_DESCRIPTION, OnKillfocusDescription)
	ON_EN_KILLFOCUS(IDC_RVU, OnKillfocusRvu)
	ON_EN_KILLFOCUS(IDC_CPT_TOS, OnKillfocusTOS)
	ON_EN_KILLFOCUS(IDC_GLOBAL_PERIOD, OnKillfocusGlobalPeriod)
	ON_BN_CLICKED(IDC_TAXABLE_1, OnTaxable1)
	ON_BN_CLICKED(IDC_TAXABLE_2, OnTaxable2)
	ON_BN_CLICKED(IDC_ADD_CPT, OnAddCpt)
	ON_BN_CLICKED(IDC_ADD_MODIFIER, OnAddModifier)
	ON_BN_CLICKED(IDC_DELETE_MODIFIER, OnDeleteModifier)
	ON_BN_CLICKED(IDC_ADD_ICD9, OnAddIcd9)
	ON_BN_CLICKED(IDC_DELETE_ICD9, OnDeleteIcd9)
	ON_BN_CLICKED(IDC_DELETE_CPT, OnDeleteCpt)
	ON_BN_CLICKED(IDC_MARK_INACTIVE, OnMarkInactive)
	ON_BN_CLICKED(IDC_LEFT_CPT, OnLeftCpt)
	ON_BN_CLICKED(IDC_RIGHT_CPT, OnRightCpt)
	ON_BN_CLICKED(IDC_UPDATE_PRICES, OnUpdatePrices)
	ON_COMMAND(ID_ICD_MARK_INACTIVE, OnMarkIcd9Inactive)
	ON_BN_CLICKED(IDC_RECEIPT_CONFIG, OnReceiptConfig)
	ON_BN_CLICKED(IDC_UPDATE_CATEGORIES, OnUpdateCategories)
	ON_COMMAND(ID_ICD_FILTER, OnFilterICD9)
	ON_BN_CLICKED(IDC_FILTER_CPT, OnFilterCPT)
	ON_EN_KILLFOCUS(IDC_CPT_BARCODE, OnKillfocusCptBarcode)
	ON_BN_CLICKED(IDC_CONFIG_BILL_COLUMNS, OnConfigBillColumns)
	//ON_BN_CLICKED(IDC_ADV_REVCODE_SETUP, OnAdvRevcodeSetup)
	//ON_BN_CLICKED(IDC_CHECK_SINGLE_REV_CODE, OnCheckSingleRevCode)//// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//ON_BN_CLICKED(IDC_CHECK_MULTIPLE_REV_CODES, OnCheckMultipleRevCodes)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//ON_BN_CLICKED(IDC_BTN_EDIT_MULTIPLE_REV_CODES, OnBtnEditMultipleRevCodes)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	ON_BN_CLICKED(IDC_ADD_SERVICE_CODE_SETUP, OnAdditionalServiceCodeSetup)// (s.tullis 2014-05-01 15:32) - PLID 61939 - Add “Additional Service Code Setup” button to the bottom of the CPT Configuration screen.
	//ON_BN_CLICKED(IDC_CHECK_ANESTHESIA, OnCheckAnesthesia)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	ON_BN_CLICKED(IDC_BTN_IMPORT_CPT, OnBtnImportCpt)
	ON_BN_CLICKED(IDC_BTN_CPT_DIAGNOSIS_LINKING, OnBtnCptDiagnosisLinking)
	ON_BN_CLICKED(IDC_BTN_SELECT_CATEGORY, OnBtnSelectCategory)
	ON_BN_CLICKED(IDC_BTN_REMOVE_CATEGORY, OnBtnRemoveCategory)
	ON_EN_KILLFOCUS(IDC_SHOP_FEE, OnKillfocusShopFee)
	ON_BN_CLICKED(IDC_BTN_LAUNCH_CODELINK, OnBtnLaunchCodelink)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BTN_IMPORT_AMA, OnBtnImportAma)
	//ON_BN_CLICKED(IDC_BTN_ANESTHESIA_SETUP, OnBtnAnesthesiaSetup)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//ON_EN_KILLFOCUS(IDC_ANESTH_BASE_UNITS, OnKillfocusAnesthBaseUnits)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moveds
	ON_BN_CLICKED(IDC_UPDATE_TOS, OnUpdateTos)
	ON_BN_CLICKED(IDC_CONFIGURE_FINANCE_CHARGE_SETTINGS, OnConfigureFinanceChargeSettings)
	//ON_BN_CLICKED(IDC_CHECK_FACILITY, OnCheckFacility) //s.tullis
	//ON_BN_CLICKED(IDC_BTN_FACILITY_FEE_SETUP, OnBtnFacilityFeeSetup)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	ON_BN_CLICKED(IDC_PROCEDURE_DESCRIPTION, OnProcedureDescription)
	//ON_BN_CLICKED(IDC_CHECK_USE_ANESTHESIA_BILLING, OnCheckUseAnesthesiaBilling)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//ON_BN_CLICKED(IDC_CHECK_USE_FACILITY_BILLING, OnCheckUseFacilityBilling)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	ON_BN_CLICKED(IDC_MARK_MODIFIER_INACTIVE, OnMarkModifierInactive)
	ON_BN_CLICKED(IDC_INACTIVE_CODES, OnInactiveCodes)
	ON_BN_CLICKED(IDC_BTN_DISCOUNT_CATEGORIES, OnBtnDiscountCategories)
	//ON_BN_CLICKED(IDC_BTN_SETUP_ICD9V3, OnBtnSetupIcd9v3)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	ON_COMMAND(ID_ICD_UNFILTER, OnFilterICD9)
	ON_COMMAND(ID_ICD_MAKE_DEFAULT, SetAsDefault)
	ON_COMMAND(ID_ICD_REMOVE_DEFAULT, RemoveDefault)
	ON_BN_CLICKED(IDC_MARK_ICD9_INACTIVE, OnMarkIcd9Inactive)
	ON_COMMAND(IDC_FILTER_CPT, OnFilterCPT)
	ON_COMMAND(IDC_FILTER_ICD9, OnFilterICD9)
	ON_BN_CLICKED(IDC_FILTER_ICD9, OnFilterICD9)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_COMMAND(ID_UPDATE_BY_PERCENT, OnUpdatePricesByPercentage)
	ON_COMMAND(ID_UPDATE_BY_RVU, OnUpdatePricesByRVU)
	ON_COMMAND(ID_UPDATE_FROM_FILE, OnUpdatePricesFromFile)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_COMMAND(ID_MOD_MARK_INACTIVE, OnMarkModifierInactive)
	// (j.jones 2007-07-03 15:05) - PLID 26098 - added option to link service codes and modifiers
	ON_BN_CLICKED(IDC_BTN_SERVICE_MODIFIER_LINKING, OnBtnServiceModifierLinking)
	//ON_BN_CLICKED(IDC_BTN_OHIP_PREMIUM_CODES, OnBtnOhipPremiumCodes)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	ON_BN_CLICKED(IDC_BTN_EDIT_PAY_GROUPS, OnBtnEditPayGroups)
	ON_BN_CLICKED(IDC_BTN_ADV_PAY_GROUP_SETUP, OnBtnAdvPayGroupSetup)
	//ON_BN_CLICKED(IDC_CHECK_BATCH_WHEN_ZERO, OnCheckBatchWhenZero)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//ON_BN_CLICKED(IDC_CHECK_ASSISTING_CODE, OnCheckAssistingCode)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//ON_EN_KILLFOCUS(IDC_ASSISTING_BASE_UNITS, OnKillfocusAssistingBaseUnits)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	ON_BN_CLICKED(IDC_CPT_BILLABLE, OnCPTBillable)
	ON_BN_CLICKED(IDC_BTN_EDIT_BILLABLE_CODES, OnBtnEditBillableCodes)
	//ON_EN_KILLFOCUS(IDC_UB_BOX4, OnEnKillfocusUbBox4)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	//ON_BN_CLICKED(IDC_CPT_REQUIRE_REF_PHYS, &CCPTCodes::OnBnClickedCptRequireRefPhys)// (s.tullis 2014-05-21 10:20) - PLID 61829 - moved
	ON_EN_KILLFOCUS(IDC_CPT_COST , &CCPTCodes::OnKillfocusDefaultCost)
	ON_BN_CLICKED(IDC_BTN_EDIT_BILLABLE_SHOP_FEES, &CCPTCodes::OnBnClickedBtnEditBillableShopFees)
	//ON_BN_CLICKED(IDC_CPT_ADDITIONAL_INFO_BTN, &CCPTCodes::OnBnClickedCptAdditionalInfoBtn)
	ON_BN_CLICKED(IDC_CPT_CODES_DIAG_ICD9, &CCPTCodes::OnBnClickedCptCodesDiagIcd)		// (j.armen 2014-03-05 17:39) - PLID 61207
	ON_BN_CLICKED(IDC_CPT_CODES_DIAG_ICD10, &CCPTCodes::OnBnClickedCptCodesDiagIcd)		// (j.armen 2014-03-05 17:39) - PLID 61207
	ON_BN_CLICKED(IDC_CPT_CODES_DIAG_CUSTOM_NEXGEMS, OnBnClickedCptCodesDiagIcd)		// (c.haag 2015-05-11) - PLID 66017
	ON_BN_CLICKED(IDC_BTN_ADD_CUSTOM_MAPPING, OnBnClickedAddCustomMapping) // (c.haag 2015-05-14) - PLID 66020
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CCPTCodes, CNxDialog)
	ON_EVENT(CCPTCodes, IDC_CPT_MODIFIERS, 6 /* RButtonDown */, OnRButtonDownCptModifiers, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCPTCodes, IDC_DIAG_CODES, 6 /* RButtonDown */, OnRButtonDownDiagCodes, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CCPTCodes, IDC_CPT_MODIFIERS, 10 /* EditingFinished */, OnEditingFinishedCptModifiers, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CCPTCodes, IDC_DIAG_CODES, 9 /* EditingFinishing */, OnEditingFinishingDiagCodes, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CCPTCodes, IDC_DIAG_CODES, 10 /* EditingFinished */, OnEditingFinishedDiagCodes, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CCPTCodes, IDC_CPT_MODIFIERS, 9 /* EditingFinishing */, OnEditingFinishingCptModifiers, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CCPTCodes, IDC_CPT_CODES, 18 /* RequeryFinished */, OnRequeryFinishedCptCodes, VTS_I2)
	ON_EVENT(CCPTCodes, IDC_CPT_CODES, 16 /* SelChosen */, OnSelChosenCptCodes, VTS_I4)
	//ON_EVENT(CCPTCodes, IDC_UB92_CPT_CATEGORIES, 16 /* SelChosen */, OnSelChosenUb92CptCategories, VTS_I4)// (s.tullis 2014-05-22 09:13) - PLID 61829 - moved
	ON_EVENT(CCPTCodes, IDC_CPT_CODES, 17 /* ColumnClicking */, OnColumnClickingCptCodes, VTS_I2 VTS_PBOOL)
	ON_EVENT(CCPTCodes, IDC_DEFAULT_CPT_PROVIDER, 16 /* SelChosen */, OnSelChosenDefaultCPTProvider, VTS_I4)
	ON_EVENT(CCPTCodes, IDC_DIAG_CODES, 18 /* RequeryFinished */, OnRequeryFinishedDiagCodes, VTS_I2)
	ON_EVENT(CCPTCodes, IDC_CPT_MODIFIERS, 8 /* EditingStarting */, OnEditingStartingCptModifiers, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CCPTCodes, IDC_DIAG_CODES, 8 /* EditingStarting */, OnEditingStartingDiagCodes, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CCPTCodes, IDC_CPT_CODES, 20 /* TrySetSelFinished */, OnTrySetSelFinishedCptCodes, VTS_I4 VTS_I4)
	//ON_EVENT(CCPTCodes, IDC_LIST_ICD9V3, 1 /* SelChanging */, OnSelChangingListIcd9v3, VTS_DISPATCH VTS_PDISPATCH)//s.tullis
//ON_EVENT(CCPTCodes, IDC_LIST_ICD9V3, 2 /* SelChanged */, OnSelChangedListIcd9v3, VTS_DISPATCH VTS_DISPATCH)//s.tullis
	ON_EVENT(CCPTCodes, IDC_PAY_GROUPS_COMBO, 16, OnSelChosenPayGroupsCombo, VTS_DISPATCH)
	ON_EVENT(CCPTCodes, IDC_ALBERTA_MODIFIERS, 10, CCPTCodes::EditingFinishedAlbertaModifiers, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//ON_EVENT(CCPTCodes, IDC_NOC_COMBO, 16, OnSelChosenNocCombo, VTS_DISPATCH)
	//ON_EVENT(CCPTCodes, IDC_NOC_COMBO, 1, OnSelChangingNocCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CCPTCodes, IDC_ICD9_DIAG_SEARCH_LIST, 16, OnSelChosenICD9DiagSearchList, VTS_DISPATCH)
	ON_EVENT(CCPTCodes, IDC_ICD10_DIAG_SEARCH_LIST, 16, OnSelChosenICD10DiagSearchList, VTS_DISPATCH)
	ON_EVENT(CCPTCodes, IDC_DIAG_CUSTOM_NEXGEMS, 6 /* RButtonDown */, OnRButtonDownCustomNexGEMsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CCPTCodes message handlers

BOOL CCPTCodes::OnInitDialog() 
{
	__super::OnInitDialog();

	try{
		//(e.lally 2011-08-26) PLID 45210 - Moved up here so we can grab it in our bulk cache
		CString strPropName;
		strPropName.Format("AMANotice_%02i.%02i.%04i.%04i", PRODUCT_VERSION_VAL);

		// (j.jones 2006-05-04 11:26) - PLID 20441 - Load all common properties into the
		// NxPropManager cache
		g_propManager.BulkCache("CPTCodes", propbitNumber | propbitDateTime,
			"(Username = '<None>' OR Username = '%s' OR Username = '%s') AND ("
			"Name = '%s' OR "
			"Name = 'CPTEditSort' OR "
			"Name = 'LastCPTUsed' OR "
			"Name = 'UpdateSurgeryPrices' OR "
			"Name = 'UpdateSurgeryPriceWhen' OR "
			"Name = 'EnableCodeLinkLaunching' OR "
			"Name = 'DefaultICD9Code' OR "
			"Name = 'UseOHIP' OR "	// (j.jones 2009-03-31 15:11) - PLID 32324
			"Name = 'UpdatePreferenceCardPrices' OR "	// (j.jones 2009-08-26 08:39) - PLID 35124
			"Name = 'UpdatePreferenceCardPriceWhen' "
			"OR Name = 'GlobalPeriod_OnlySurgicalCodes' "	// (j.jones 2012-07-23 17:24) - PLID 51651
			"OR Name = 'AdminDisplayICDCodeVersion' "	// (j.armen 2014-03-05 17:39) - PLID 61207
			")", _Q(GetCurrentUserName()),
			_Q(GetAdvancedUsername()),
			_Q(strPropName) );

		//DRT 5/4/2004 - PLID 12196 - Make a 1 time popup (per ws/winuser/path, per version) that states the AMA's
		//	initial text popup requirement.
		{
			//if they have not set this yet, we need to prompt
			if (GetRemotePropertyDateTime(strPropName, NULL, 0, GetAdvancedUsername(), false).GetStatus() != COleDateTime::valid) {
				PopupAMACopyright();
				SetRemotePropertyDateTime(strPropName, COleDateTime::GetCurrentTime(), 0, GetAdvancedUsername());
			}
		}

		// (j.armen 2014-03-05 17:39) - PLID 61207 - Set the correct check based on ICD preference
		// (c.haag 2015-05-11) - PLID 66017 - We now support a check to reveal custom mappings
		switch (GetRemotePropertyInt("AdminDisplayICDCodeVersion", adicvpICD9Codes, 0, GetCurrentUserName()))
		{
		case adicvpICD9Codes:
			m_checkDiagICD9.SetCheck(BST_CHECKED);
			break;
		case adicvpICD10Codes:
			m_checkDiagICD10.SetCheck(BST_CHECKED);
			break;
		case adicvpCustomNexGEMs:
			m_checkDiagCustomNexGEMs.SetCheck(BST_CHECKED);
			break;
		default:
			// We should never get here; do as the legacy code did and don't assign a checkbox
			break;
		}

		//DRT 4/2/2008 - PLID 29518 - Cleaned up NxIconButton auto settings.
		m_deleteCPTButton.AutoSet(NXB_DELETE);
		m_deleteModifierButton.AutoSet(NXB_DELETE);
		m_deleteICD9Button.AutoSet(NXB_DELETE);
		m_btnRemoveCategory.AutoSet(NXB_DELETE);

		m_addModifierButton.AutoSet(NXB_NEW);
		m_addicd9Button.AutoSet(NXB_NEW);
		m_addCptButton.AutoSet(NXB_NEW);
		m_btnSelectCategory.AutoSet(NXB_NEW);

		m_btnUpdateCats.AutoSet(NXB_MODIFY);
		//m_btnEditMultipleRevCodes.AutoSet(NXB_MODIFY);
		m_btnMarkICD9Inactive.AutoSet(NXB_MODIFY);
		m_markInactiveButton.AutoSet(NXB_MODIFY);
		m_btnMarkModifierInactive.AutoSet(NXB_MODIFY);

		// (j.jones 2010-08-02 14:32) - PLID 39912 - added ability to edit adv. pay group settings
		m_btnAdvPayGroupSetup.AutoSet(NXB_MODIFY);

		// (c.haag 2015-05-14) - PLID 66020
		m_addCustomMappingButton.AutoSet(NXB_NEW);

		m_nxlAMA.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_nxlAMA.SetType(dtsHyperlink);
		m_nxlAMA.SetSingleLine(false);
	
		//SetWindowPos(GetDlgitem(IDC_BILLING_BAR), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		// (s.tullis 2014-05-22 09:13) - PLID 61829 - Set Limit of TOS textbox to 2 characters
		m_editTOS.SetLimitText(2);

		/*// (j.jones 2012-01-03 17:04) - PLID 47300 - added NOC label
		m_nxlNOC.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_nxlNOC.SetType(dtsHyperlink);
		m_nxlNOC.SetSingleLine(true);
		m_nxlNOC.SetText("NOC Code");*/

		// (j.jones 2010-11-22 12:59) - PLID 41544 - limit the length of the units
		//m_editAssistingBaseUnits.SetLimitText(5);
		//m_editAnesthBaseUnits.SetLimitText(5);

		// (j.jones 2011-07-06 15:57) - PLID 44358 - added UB Box 4
		//m_editUBBox4.SetLimitText(20);

		// (j.jones 2009-03-30 16:59) - PLID 32324 - show/hide the premium codes button
		// based on the OHIP preference, and show/hide the ICD9 V3 code alternatively
		// This has to be done in OnInitDialog for when being loaded from a bill,
		// and in Refresh when in the Billing tab of Admin.
		// (j.jones 2010-07-08 14:01) - PLID 39566 - this is now cached as a member variable
	/*	m_bUseOHIP = UseOHIP();//s.tullis
		if(m_bUseOHIP) {
			m_btnOHIPPremiumCodes.ShowWindow(SW_SHOW);
			//m_btnSetupICD9v3.ShowWindow(SW_HIDE);
			//m_nxstaticIcd9v3Label.ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_LIST_ICD9V3)->ShowWindow(SW_HIDE);
		}
		else {
			m_btnOHIPPremiumCodes.ShowWindow(SW_HIDE);
		//	m_btnSetupICD9v3.ShowWindow(SW_SHOW);
			//m_nxstaticIcd9v3Label.ShowWindow(SW_SHOW);
		//	GetDlgItem(IDC_LIST_ICD9V3)->ShowWindow(SW_SHOW);
		}*/

		// (c.haag 2015-05-14) - PLID 66019 - Diagnosis searches for custom crosswalks, these always include PCS codes
		m_pICD9WithPCSSearchConfig = dynamic_cast<CDiagSearchConfig *>(new CDiagSearchConfig_FullICD9Search(true));
		m_pICD10WithPCSSearchConfig = dynamic_cast<CDiagSearchConfig *>(new CDiagSearchConfig_FullICD10Search(true));

		m_pDiag9Search = DiagSearchUtils::CommonBindSearchListCtrl(this, IDC_ICD9_DIAG_SEARCH_LIST, GetRemoteData(), m_pICD9WithPCSSearchConfig);
		m_pDiag9Search->PutSearchPlaceholderText("ICD-9 Search...");
		m_pDiag10Search = DiagSearchUtils::CommonBindSearchListCtrl(this, IDC_ICD10_DIAG_SEARCH_LIST, GetRemoteData(), m_pICD10WithPCSSearchConfig);
		m_pDiag10Search->PutSearchPlaceholderText("ICD-10 Search...");

		m_pCodeNames = BindNxDataListCtrl(IDC_CPT_CODES,false);
		GetDlgItem(IDC_DELETE_CPT)->EnableWindow(FALSE);
		
		int nDefCol = GetRemotePropertyInt("CPTEditSort", 0, 0, GetCurrentUserName());
		if(nDefCol < 1 || nDefCol > 4){
			nDefCol = 1;
			SetRemotePropertyInt("CPTEditSort", nDefCol, 0, GetCurrentUserName());
		}
		
		IColumnSettingsPtr pCol = m_pCodeNames->GetColumn(nDefCol);
		if (pCol) {
			pCol->SortPriority = 0;
		}

		m_bRequeryFinished = false;
		m_pCodeNames->Requery();

		m_CurServiceID = GetRemotePropertyInt("LastCPTUsed",-1,0,GetCurrentUserName(),FALSE);
		if(m_CurServiceID != -1)
			m_pCodeNames->TrySetSelByColumn(0,m_CurServiceID);

		m_pModifiers = BindNxDataListCtrl(IDC_CPT_MODIFIERS);
		
		// (j.armen 2014-03-06 09:59) - PLID 61209 - Don't requery right away.  We need to add our new where clause
		m_pDiagCodes = BindNxDataListCtrl(IDC_DIAG_CODES, false);
		m_pDiagCodes->WhereClause = DEFAULT_ICD9_WHERE_CLAUSE;
		// (c.haag 2015-05-12) - PLID 66017 - Bind the custom NexGEMs datalist and load the 9/10 codes only if they're
		// going to be visible by default
		m_pDiagCodeCustomNexGEMs = BindNxDataList2Ctrl(IDC_DIAG_CUSTOM_NEXGEMS, false);
		if (adicvpCustomNexGEMs == GetRemotePropertyInt("AdminDisplayICDCodeVersion", adicvpICD9Codes, 0, GetCurrentUserName()))
		{
			RefreshCustomNexGEMsList();
		}
		else
		{
			m_pDiagCodes->Requery();
		}

		m_DefaultProviderCombo = BindNxDataListCtrl(IDC_DEFAULT_CPT_PROVIDER);
		// (s.tullis 2014-05-22 09:13) - PLID 61829 - moved
		// (a.walling 2007-06-20 12:01) - PLID 26413 - setup the list
		//m_ICD9V3List = BindNxDataList2Ctrl(IDC_LIST_InCD9V3, GetRemoteData(), true);// s.tullis comment this out later

		/*NXDATALIST2Lib::IRowSettingsPtr pNoRow = m_ICD9V3List->GetNewRow();
		if (pNoRow) {
			pNoRow->PutValue(0, _variant_t(long(-1)));
			pNoRow->PutValue(1, _variant_t(""));
			pNoRow->PutValue(2, _variant_t("<None>"));

			m_ICD9V3List->AddRowSorted(pNoRow, NULL);
		}*/

		// (j.jones 2010-07-30 10:35) - PLID 39728 - added support for pay groups
		m_PayGroupCombo = BindNxDataList2Ctrl(IDC_PAY_GROUPS_COMBO, true);
		NXDATALIST2Lib::IRowSettingsPtr pNoPayGroup = m_PayGroupCombo->GetNewRow();
		pNoPayGroup->PutValue(pgccID, (long)-1);
		pNoPayGroup->PutValue(pgccName, _bstr_t(" <No Default Pay Group>"));
		m_PayGroupCombo->AddRowSorted(pNoPayGroup, NULL);

		// (j.jones 2012-01-03 15:08) - PLID 47300 - added NOC combo
		//m_NOCCombo = BindNxDataList2Ctrl(IDC_NOC_COMBO, false);

		//add NOC options for <Default>, No, and Yes
	/*	NXDATALIST2Lib::IRowSettingsPtr pNOCRow = m_NOCCombo->GetNewRow();
		pNOCRow->PutValue(noccID, (long)noctDefault);
		pNOCRow->PutValue(noccName, "<Default>");
		m_NOCCombo->AddRowAtEnd(pNOCRow, NULL);
		pNOCRow = m_NOCCombo->GetNewRow();
		pNOCRow->PutValue(noccID, (long)noctNo);
		pNOCRow->PutValue(noccName, "No");
		m_NOCCombo->AddRowAtEnd(pNOCRow, NULL);
		pNOCRow = m_NOCCombo->GetNewRow();
		pNOCRow->PutValue(noccID, (long)noctYes);
		pNOCRow->PutValue(noccName, "Yes");
		m_NOCCombo->AddRowAtEnd(pNOCRow, NULL);*/

		_variant_t var;
		var.vt = VT_NULL;
		IRowSettingsPtr pRow;

		// (j.jones 2011-06-24 15:13) - PLID 22586 - now we have an option to "use default",
		// and one to literally default to no provider
		pRow = m_DefaultProviderCombo->GetRow(-1);
		pRow->PutValue(0,var);
		pRow->PutValue(1,_variant_t(" {Use Default Bill Provider}"));
		m_DefaultProviderCombo->AddRow(pRow);
		pRow = m_DefaultProviderCombo->GetRow(-1);
		pRow->PutValue(0,(long)-2);
		pRow->PutValue(1,_variant_t(" {Use No Provider}"));
		m_DefaultProviderCombo->AddRow(pRow);		
		// (s.tullis 2014-05-22 09:13) - PLID 61829 - moved
		/*m_UB92_Category = BindNxDataListCtrl(IDC_UB92_CPT_CATEGORIES);//s.tullis comment out later
		m_UB92_Category->Enabled = FALSE;
		m_btnEditMultipleRevCodes.EnableWindow(FALSE);
		
		pRow = m_UB92_Category->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, (LPCTSTR)"<None>");
		pRow->PutValue(2, (LPCTSTR)"<No Category Selected>");
		m_UB92_Category->AddRow(pRow);*/
				
		_variant_t		tmpVar;
		_RecordsetPtr	rs;
		FieldsPtr		Fields;
		CString			item;

		//DRT 4/16/2004 - If they do not have a NexSpa liense, hide the Shop Fee fields
		if(!IsSpa(FALSE)) {
			GetDlgItem(IDC_SHOP_FEE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SHOP_FEE)->ShowWindow(SW_HIDE);
		}

		m_bIsAdding = false;

		m_btnCptLeft.AutoSet(NXB_LEFT);
		m_btnCptLeft.EnableWindow(FALSE);
		m_btnCptRight.AutoSet(NXB_RIGHT);
		if(m_pCodeNames->GetRowCount() < 2) {
			m_btnCptRight.EnableWindow(FALSE);
		}
		
		// Added by Chris (7/8)
		if (m_IsModal) {
			//(e.lally 2006-10-10) PLID 22936 - The loading code will get called from 
			//NxDialog:: UpdateView (the refresh function then gets called in this class) 
			//so this load call only needs to be made if it is created from a modal place like
			//a new bill.
			Load();
			//(e.lally) This code was no longer executed until I made my change so I am commenting it out.
			//SetWindowPos(&wndTopMost, 50,50,750,690, 0);
			//SetControlPositions();
			//SetParent(NULL);
			//ModifyStyle(WS_CHILD, WS_CAPTION|WS_POPUPWINDOW, SWP_DRAWFRAME);
			//SetWindowText("Add/Edit Service Codes");
			//ShowWindow(SW_SHOW);
		} 

		m_CPTCode.SetLimitText(50);
		m_CPTSubCode.SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_DESCRIPTION))->SetLimitText(255);
		((CNxEdit*)GetDlgItem(IDC_CPT_BARCODE))->SetLimitText(80);

		// (d.singleton 2011-09-21 14:40) - PLID 44946 - hide the alberta mods datalist by default
		m_pAlbertaModifiers = BindNxDataListCtrl(IDC_ALBERTA_MODIFIERS, false);

		// (d.singleton 2011-09-14 17:56) - PLID - 44946 - if using alberta billing show the alberta mods datalist and hide traditional mods list
		if(UseAlbertaHLINK())
		{
			//hide current modifiers window and display read only version for alberta,  show mods and rules that pertain to currently selected cpt
			GetDlgItem(IDC_CPT_MODIFIERS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_ALBERTA_MODIFIERS)->ShowWindow(SW_SHOW);

			//create where clause
			CString strAlbertaModsWhere;
			strAlbertaModsWhere.Format("ServiceID = %li", m_CurServiceID);

			//requery with new where clause
			m_pAlbertaModifiers->PutWhereClause(_bstr_t(strAlbertaModsWhere));
			m_pAlbertaModifiers->Requery();
		}

		// (c.haag 2015-05-11) - PLID 66017 - Update the diag list visibility based on the checked item
		if (m_checkDiagCustomNexGEMs.GetCheck() == BST_CHECKED) {
			ShowDiagDatalist(CustomNexGEMs);			
		} else if (m_checkDiagICD10.GetCheck() == BST_CHECKED) {
			ShowDiagDatalist(DiagCodes);
			// (j.armen 2014-03-25 09:21) - PLID 61534 - Disable editing on code number / description for ICD-10's
			m_pDiagCodes->GetColumn(dclcCodeNumber)->Editable = VARIANT_FALSE;
			m_pDiagCodes->GetColumn(dclcCodeDesc)->Editable = VARIANT_FALSE;
			// (j.armen 2014-03-25 09:22) - PLID 61535 - Hide AMA Column when showing ICD 10's
			m_pDiagCodes->GetColumn(dclcIsAMA)->StoredWidth = 0;
		} else {
			ShowDiagDatalist(DiagCodes);
			m_pDiagCodes->GetColumn(dclcCodeNumber)->Editable = VARIANT_TRUE;
			m_pDiagCodes->GetColumn(dclcCodeDesc)->Editable = VARIANT_TRUE;
			m_pDiagCodes->GetColumn(dclcIsAMA)->StoredWidth = 55;
		}

		SecureControls();
	}NxCatchAll("Error in OnInitDialog");

	return TRUE;
}

BOOL CCPTCodes::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (HIWORD(wParam))
	{	
		case EN_CHANGE:
		{
			switch (LOWORD(wParam))
			{	case IDC_CPTCODE:
				case IDC_CPTSUBCODE:
				case IDC_DESCRIPTION:
				case IDC_STD_FEE:
				case IDC_CPT_COST:// (s.dhole 2012-03-02 18:06) - PLID 47399
				case IDC_RVU:
				case IDC_CPT_TOS:
				case IDC_GLOBAL_PERIOD:
				case IDC_CPT_BARCODE:
				case IDC_SHOP_FEE:
				case IDC_ANESTH_BASE_UNITS:
					// (j.jones 2010-11-19 16:27) - PLID 41544 - supported assisting codes
				case IDC_ASSISTING_BASE_UNITS:
				// (j.jones 2011-07-06 16:34) - PLID 44358 - added UB Box 4
				//case IDC_UB_BOX4:
					m_bEditChanged = TRUE;
					break;
			}
		}
		break;
	}	

	return CNxDialog::OnCommand(wParam, lParam);
}

void CCPTCodes::OnDestroy()
{
	// (c.haag 2015-05-14) - PLID 66019 - Destroy our search configurations
	try
	{
		if (nullptr != m_pICD9WithPCSSearchConfig)
		{
			delete m_pICD9WithPCSSearchConfig;
			m_pICD9WithPCSSearchConfig = nullptr;
		}
		if (nullptr != m_pICD10WithPCSSearchConfig)
		{
			delete m_pICD10WithPCSSearchConfig;
			m_pICD10WithPCSSearchConfig = nullptr;
		}

		__super::OnDestroy();
	}
	NxCatchAll(__FUNCTION__);
}

void CCPTCodes::Load()
{
	_RecordsetPtr	tmpRS;
	CString			value;
	_variant_t		tmpVar;
	COleCurrency	tmpCy;
	CString			tmpStr;
	

	if(m_CurServiceID == -1) {
		if(!m_pCodeNames->IsRequerying() && m_pCodeNames->CurSel == -1) {
			m_pCodeNames->CurSel = 0;
			if(m_pCodeNames->CurSel == -1) {
				m_CurServiceID = -1;
				return;
			}
			else {
				m_CurServiceID = VarLong(m_pCodeNames->GetValue(m_pCodeNames->CurSel,0));
			}
		}
	}

	if(m_CurServiceID == -1)
		return;

	try 
	{
		// (j.jones 2012-07-24 09:10) - PLID 51651 - load a bool for whether the code is a surgical code
		// (j.gruber 2012-10-19 13:07) - PLID 53240 - changed to support multiple shop fees
		tmpRS = CreateParamRecordset( 
			"SELECT CPTCodeT.*, ServiceT.*, CategoriesT.Name AS CategoryName, "
			"Convert(bit, CASE WHEN Coalesce(ServicePayGroupsT.Category, {CONST}) = {CONST} THEN 1 ELSE 0 END) AS IsSurgicalCode, "
			" CASE WHEN ShopFeeInfoQ.ManagedLocCount = ShopFeeInfoQ.CountShopFees THEN ShopFeeInfoQ.ShopFee ELSE CONVERT(money, -1) END as ShopFee "
			"FROM ServiceT "
			"INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID "
			"LEFT JOIN CategoriesT ON ServiceT.Category = CategoriesT.ID "
			"LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
			" INNER JOIN   "
			" (SELECT TOP 1 (SELECT Count(*) FROM LocationsT WHERE Managed = 1 AND Active = 1) as ManagedLocCount, "
			" 				ServiceID, ShopFee, Count(*) as CountShopFees "
			"				FROM ServiceLocationInfoT  "
			"				INNER JOIN LocationsT ON ServiceLocationInfoT.LocationID = LocationsT.ID "
			" 				WHERE ServiceLocationInfoT.ServiceID = {INT} AND LocationsT.Managed = 1 AND LocationsT.Active = 1 "
			" 				GROUP BY ServiceID, ShopFee) ShopFeeInfoQ ON ServiceT.ID = ShopFeeInfoQ.ServiceID "
			"WHERE ServiceT.ID = {INT}",
			PayGroupCategory::NoCategory, PayGroupCategory::SurgicalCode,
			m_CurServiceID, m_CurServiceID);

		if (tmpRS->eof) {
			//if we get here, it means another user has deleted that CPT code, so say as much and refresh
			//however, we will refresh beforehand so the list can load while the user is reading the message
			//(and, we'll manually remove the row incase tablecheckers are disabled and the list doesn't requery)

			// (j.jones 2006-05-04 12:07) - we need a current selection before we can continue
			if(m_pCodeNames->GetCurSel() == -1) {
				//we need to turn our "trysetsel" into a "setsel"
				CWaitCursor pWait;
				m_pCodeNames->SetSelByColumn(0, m_CurServiceID);
			}

			m_pCodeNames->RemoveRow(m_pCodeNames->CurSel);
			m_pCodeNames->CurSel = 0;
			if(m_pCodeNames->CurSel == -1) {
				m_CurServiceID = -1;
				return;
			}
			else {
				m_CurServiceID = VarLong(m_pCodeNames->GetValue(m_pCodeNames->CurSel,0));
			}

			if(m_CurServiceID == -1)
				return;

			Refresh();

			AfxMessageBox("Practice could not load your selected Service Code. It is likely this code has been deleted.");
			return;
		}
	}
	NxCatchAll("couldn't find Service code information")

	try	
	{
		long ServiceID = m_CurServiceID;
		SetRemotePropertyInt("LastCPTUsed",ServiceID,0,GetCurrentUserName());

		CString csPhone;
		FieldsPtr Fields;
		Fields = tmpRS->Fields;
		SetDlgItemVar (IDC_CPTCODE,		Fields->GetItem("Code")->Value, true, true);
		SetDlgItemVar (IDC_CPTSUBCODE,	Fields->GetItem("SubCode")->Value, true, true);
		SetDlgItemVar (IDC_DESCRIPTION,	Fields->GetItem("Name")->Value, true, true);

		_variant_t var;
		tmpCy = Fields->GetItem("Price")->Value.cyVal;
		tmpStr = FormatCurrencyForInterface(tmpCy);
		CCPTCodes::SetDlgItemText(IDC_STD_FEE,tmpStr);
		//DRT 4/15/2004 - Shop fee loading
		//CCPTCodes::SetDlgItemText(IDC_SHOP_FEE, FormatCurrencyForInterface(VarCurrency(Fields->GetItem("ShopFee")->Value)));
		// (j.gruber 2012-10-19 13:08) - PLID 53240 - new shop fee loading
		m_cyShopFee = AdoFldCurrency(Fields, "ShopFee");
		if (m_cyShopFee < COleCurrency(0,0)) {
			//we don't allow negative shop fees, so we know that this is multiple
			//show multiple in the box and make it uneditable
			GetDlgItem(IDC_SHOP_FEE)->EnableWindow(FALSE);
			CCPTCodes::SetDlgItemText(IDC_SHOP_FEE, "<Multiple>");
		}
		else {
			//there is only one for all locations, so load it
			GetDlgItem(IDC_SHOP_FEE)->EnableWindow(TRUE);
			CCPTCodes::SetDlgItemText(IDC_SHOP_FEE, FormatCurrencyForInterface(m_cyShopFee));
		}

		// (s.dhole 2012-03-02 18:06) - PLID 47399
		CCPTCodes::SetDlgItemText(IDC_CPT_COST, FormatCurrencyForInterface(VarCurrency(Fields->GetItem("ServiceDefaultCost")->Value)));

		CString str;

		tmpVar = Fields->GetItem("Taxable1")->Value;
		if (tmpVar.vt == VT_BOOL && tmpVar.boolVal)
			m_Taxable1.SetCheck(TRUE);
		else
			m_Taxable1.SetCheck(FALSE);

		tmpVar = Fields->GetItem("Taxable2")->Value;
		if (tmpVar.vt == VT_BOOL && tmpVar.boolVal)
			m_Taxable2.SetCheck(TRUE);
		else
			m_Taxable2.SetCheck(FALSE);
		
		// (j.jones 2011-03-28 09:31) - PLID 43012 - added Billable checkbox
		m_checkBillable.SetCheck(VarBool(Fields->GetItem("Billable")->Value));

		//see if the standard fee should be grayed out (for Anesth./Facility purposes)
		CheckEnableStandardFee();

		tmpVar = Fields->GetItem("RVU")->Value;
		if (tmpVar.vt == VT_R8)	
			str.Format("%0.09g", tmpVar.dblVal);
		else str = " ";
		SetDlgItemText (IDC_RVU, str);

		// (j.jones 2012-07-24 08:57) - PLID 51651 - Added a preference to only track global periods for
		// surgical codes only, this will control whether the edit box is enabled for all codes or not.
		// We would still load the content even if was disabled.
		ReflectGlobalPeriodReadOnly(Fields->GetItem("IsSurgicalCode")->Value);

		tmpVar = Fields->GetItem("GlobalPeriod")->Value;
		if (tmpVar.vt == VT_I4)	
			str.Format("%li", tmpVar.lVal);
		else str = "";
		SetDlgItemText (IDC_GLOBAL_PERIOD, str);

		SetDlgItemText(IDC_CPT_BARCODE, VarString(Fields->GetItem("Barcode")->Value,""));
		
		// (a.wilson 2014-01-14 09:19) - PLID 59956 - load ccdatype and tos into member variables.
		m_strTOS = AdoFldString(Fields, "TypeOfService", "");
		SetDlgItemText(IDC_CPT_TOS, m_strTOS);
		m_eCCDAType = (CCDAProcedureType)AdoFldLong(Fields, "CCDAType", -1);
		
		// (j.jones 2015-03-02 15:15) - PLID 64963 - load all the category names
		CString strCategoryNames;
		std::vector<long> aryCategoryIDs;
		long nDefaultCategoryID = -1;
		LoadServiceCategories(ServiceID, aryCategoryIDs, strCategoryNames, nDefaultCategoryID);

		SetDlgItemText(IDC_CATEGORY_BOX, strCategoryNames);

		m_DefaultProviderCombo->SetSelByColumn(0,Fields->GetItem("ProviderID")->Value);

		//DRT 5/3/2004 - PLID 12198 - If a code is licensed by the AMA, then we need to disable
		//	the code, subcode, and description fields, they are not allowed to be edited.
		//PLID 12197 - Set the label text to say "Hey, this is an AMA Code!" if it is.
		BYTE bAMA = VarByte(Fields->Item["IsAMA"]->Value, 0);
		if(bAMA) {
			((CNxEdit*)GetDlgItem(IDC_CPTCODE))->SetReadOnly(TRUE);
			((CNxEdit*)GetDlgItem(IDC_CPTSUBCODE))->SetReadOnly(TRUE);
			//DRT 4/24/2008 - PLID 29780 - Clients are allowed to change the description after it has been imported.
			//((CNxEdit*)GetDlgItem(IDC_DESCRIPTION))->SetReadOnly(TRUE);
			GetDlgItem(IDC_AMA_TEXT_LABEL)->ShowWindow(SW_SHOW);
			m_nxlAMA.SetText("This CPT® code is licensed by the American Medical Association.");
			m_nxlAMA.SetType(dtsHyperlink);
			m_nxlAMA.Invalidate();
		}
		else {
			((CNxEdit*)GetDlgItem(IDC_CPTCODE))->SetReadOnly(FALSE);
			((CNxEdit*)GetDlgItem(IDC_CPTSUBCODE))->SetReadOnly(FALSE);
			//DRT 4/24/2008 - PLID 29780 - Clients are allowed to change the description after it has been imported.
			//((CNxEdit*)GetDlgItem(IDC_DESCRIPTION))->SetReadOnly(FALSE);
			GetDlgItem(IDC_AMA_TEXT_LABEL)->ShowWindow(SW_HIDE);
			m_nxlAMA.SetType(dtsDisabledHyperlink);
			m_nxlAMA.Invalidate();
		}
		// (s.tullis 2014-05-22 09:13) - PLID 61829 - moved
		// (a.walling 2007-06-20 12:02) - PLID 26413 - Select the ICD9v3 code if we have one
	/*	if (Fields->Item["ICD9ProcedureCodeID"]->Value.vt == VT_I4) {//s.tullis
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_ICD9V3List->TrySetSelByColumn_Deprecated(0, _variant_t(AdoFldLong(Fields, "ICD9ProcedureCodeID")));
		} else {
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			m_ICD9V3List->TrySetSelByColumn_Deprecated(0, _variant_t(long(-1)));
		}*/

		// (j.jones 2010-07-30 10:16) - PLID 39728 - added support for pay groups
		long nPayGroupID = VarLong(Fields->Item["PayGroupID"]->Value, -1);
		m_PayGroupCombo->SetSelByColumn(pgccID, (long)nPayGroupID);

		// (j.jones 2012-01-03 16:33) - PLID 47300 - added NOCType
		//if NULL, it's Default, otherwise it's a boolean for Yes/No
		/*_variant_t varNOCType = Fields->Item["NOCType"]->Value;
		NOCTypes eType = noctDefault;
		if(varNOCType.vt == VT_BOOL) {
			BOOL bNOCType = VarBool(varNOCType);
			eType = bNOCType ? noctYes : noctNo;
		}
		m_NOCCombo->SetSelByColumn(noccID, (long)eType);*/
	} 
	NxCatchAll("couldn't load service code information");
	tmpRS->Close();

	//Double-check the enabling of the arrows
	if(m_pCodeNames->CurSel < 1) {
		m_btnCptLeft.EnableWindow(FALSE);
	}
	else {
		m_btnCptLeft.EnableWindow(TRUE);
	}
	if(m_pCodeNames->CurSel < m_pCodeNames->GetRowCount()-1 && m_pCodeNames->CurSel != -1) {
		m_btnCptRight.EnableWindow(TRUE);
	}
	else {
		m_btnCptRight.EnableWindow(FALSE);
	}

	CCPTCodes::UpdateData(FALSE);
	m_bEditChanged = FALSE;
}


bool CCPTCodes::Save()
{
	_variant_t		tmpVar;
	CString			csCode, csSubCode;
	CString			sql, code, subcode, description, service, stdfee, catnum, rvu, globalperiod, barcode, strShopFee;// anesthbaseunits;
	CString stdCost;
	//CString			strUBBox4;
	long tax1, tax2;// /*promptcopay*/ anesthesia, useanesthesia, facilityfee, usefacilityfee;
	long nBatchWhenZero = 0;
	long nAssistingCode = 0;
	CString strAssistingBaseUnits;
	// (a.wilson 2014-01-14 11:47) - PLID 59956 - added ccdatype variable to save functionality
	_variant_t vCCDAType;

	if(m_CurServiceID == -1)
		return false;

	long ServiceID = m_CurServiceID;

	GetDlgItemText (IDC_CPTCODE,code);
	if(code == "") code = " ";
	GetDlgItemText (IDC_CPTSUBCODE,subcode);
	if(subcode == "") subcode = " ";
	GetDlgItemText (IDC_DESCRIPTION,description);
	if(description == "") description = " ";
	// (a.wilson 2014-01-14 11:47) - PLID 59956 - get data for TOS and ccdatype.
	//service = m_strTOS;
	//service.TrimRight();
	//vCCDAType = (m_eCCDAType == cptInvalidType ? g_cvarNull : (long)m_eCCDAType);
	GetDlgItemText(IDC_CPT_TOS, service);
	service.TrimRight();
	GetDlgItemText (IDC_STD_FEE,stdfee);
	GetDlgItemText (IDC_CPT_COST,stdCost);
	GetDlgItemText(IDC_SHOP_FEE, strShopFee);
	COleCurrency  cy;
	// (s.dhole 2012-03-02 18:15) - PLID 47399
	if (! IsValidCurrencyText(stdCost)) {
		MsgBox("Please enter a valid default cost");
		return false;
	}
	if (! IsValidCurrencyText(stdfee)) {
		MsgBox("Please enter a valid standard fee");
		return false;
	}
	
	// (j.gruber 2012-12-04 10:12) - PLID 53240 - if its multiple, we arne't saving the shop fee here
	BOOL bSaveShopFee = TRUE;
	if (strShopFee == "<Multiple>"){
		bSaveShopFee = FALSE;
	}

	if(bSaveShopFee && !IsValidCurrencyText(strShopFee)) {		
		MsgBox("Please enter a valid currency amount for the shop fee field.");
		return false;		
	}

	//convert the currency
	// (s.dhole 2012-03-02 18:16) - PLID 47399
	cy =ParseCurrencyFromInterface(stdCost);
	stdCost= FormatCurrencyForSql(cy);
	if(stdCost == "" || cy.GetStatus() == COleCurrency::invalid) stdCost = "0";

	cy = ParseCurrencyFromInterface(stdfee);
	stdfee = FormatCurrencyForSql(cy);

	//DRT 10/15/03 - PLID 9830 - Not sure how it ever happened in the first case, but the ParseInterface returned
	//		the string 'Invalid Currency'.  Then we tried to convert that, and it failed.  So in the future, if we
	//		get an invalid currency, just set it to $0.  There has been other code improved so we should never reach
	//		this case, but we should handle it just in case.	
	if(stdfee == "" || cy.GetStatus() == COleCurrency::invalid) stdfee = "0";

	//DRT 4/16/2004 - do the same as above for the shop fee
	// (j.gruber 2012-10-19 14:40) - PLID 53240 - use our member variable	
	if (bSaveShopFee) {
		strShopFee = FormatCurrencyForSql(m_cyShopFee);
		if(strShopFee == "" || m_cyShopFee.GetStatus() == COleCurrency::invalid) strShopFee = "0";
	}
	// (s.dhole 2012-03-02 18:17) - PLID 47399
	//if we are changing the price, then check against existing surgeries
	if(!IsRecordsetEmpty("SELECT ID FROM ServiceT WHERE ID = %li AND Price <> Convert(money,'%s') ",ServiceID,stdfee)) { 

		long nUpdateSurgeryPrices = GetRemotePropertyInt("UpdateSurgeryPrices",1,0,"<None>",TRUE);
		long nUpdateSurgeryPriceWhen = GetRemotePropertyInt("UpdateSurgeryPriceWhen",1,0,"<None>",TRUE);
		
		//3 means do nothing, so skip the check
		if(nUpdateSurgeryPrices != 3
			&& !(nUpdateSurgeryPriceWhen == 2 && IsRecordsetEmpty("SELECT ID FROM SurgeryDetailsT WHERE ServiceID = %li AND ServiceID IN (SELECT ID FROM ServiceT WHERE Price = Amount)",ServiceID))
			&& !(nUpdateSurgeryPriceWhen == 1 && IsRecordsetEmpty("SELECT ID FROM SurgeryDetailsT WHERE ServiceID = %li AND Amount <> Convert(money,'%s')",ServiceID,stdfee))) {

			//2 means to always update, so don't prompt. 1 means to prompt.
			if(nUpdateSurgeryPrices == 2 || (nUpdateSurgeryPrices == 1 && IDYES==MessageBox("There are surgeries that use this product but list a different price for it.\n"
				"Do you wish to update these surgeries to match this new price?","Practice",MB_ICONQUESTION|MB_YESNO))) {

				if(nUpdateSurgeryPriceWhen == 1)
					ExecuteSql("UPDATE SurgeryDetailsT SET Amount = Convert(money,'%s') WHERE ServiceID = %li",stdfee,ServiceID);
				else {
					ExecuteSql("UPDATE SurgeryDetailsT SET Amount = Convert(money,'%s') WHERE ServiceID = %li AND ServiceID IN (SELECT ID FROM ServiceT WHERE Price = Amount)",stdfee,ServiceID);
				}
			}
		}

		// (j.jones 2009-08-26 08:40) - PLID 35124 - do the same for preference cards
		if(IsSurgeryCenter(false)) {
			long nUpdatePreferenceCardPrices = GetRemotePropertyInt("UpdatePreferenceCardPrices",nUpdateSurgeryPrices,0,"<None>",TRUE);
			long nUpdatePreferenceCardPriceWhen = GetRemotePropertyInt("UpdatePreferenceCardPriceWhen",nUpdateSurgeryPriceWhen,0,"<None>",TRUE);

			//3 means do nothing, so skip the check
			if(nUpdatePreferenceCardPrices != 3
				&& !(nUpdatePreferenceCardPrices == 2 && IsRecordsetEmpty("SELECT ID FROM PreferenceCardDetailsT WHERE ServiceID = %li AND ServiceID IN (SELECT ID FROM ServiceT WHERE Price = Amount)",ServiceID))
				&& !(nUpdatePreferenceCardPrices == 1 && IsRecordsetEmpty("SELECT ID FROM PreferenceCardDetailsT WHERE ServiceID = %li AND Amount <> Convert(money,'%s')",ServiceID,stdfee))) {

				//2 means to always update, so don't prompt. 1 means to prompt.
				if(nUpdatePreferenceCardPrices == 2 || (nUpdatePreferenceCardPrices == 1 && IDYES==MessageBox("There are preference cards that use this service code but list a different price for it.\n"
					"Do you wish to update these preference cards to match this new price?","Practice",MB_ICONQUESTION|MB_YESNO))) {

					if(nUpdatePreferenceCardPriceWhen == 1)
						ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Amount = Convert(money,{STRING}) WHERE ServiceID = {INT}",stdfee,ServiceID);
					else {
						ExecuteParamSql("UPDATE PreferenceCardDetailsT SET Amount = Convert(money,{STRING}) WHERE ServiceID = {INT} AND ServiceID IN (SELECT ID FROM ServiceT WHERE Price = Amount)",stdfee,ServiceID);
					}
				}
			}
		}
	}
	
	GetDlgItemText(IDC_RVU, rvu);
	//Take the input, convert it to a float, convert that to a string, save it, put it on the screen.
	//Then we can be sure there are no inconvenient 's or \s
	double dRvuValue = atof((LPCTSTR)rvu);
	if(dRvuValue < 0)
		dRvuValue = -dRvuValue;
	rvu.Format("%0.09g", dRvuValue);
	SetDlgItemText(IDC_RVU, rvu);

	GetDlgItemText(IDC_GLOBAL_PERIOD, globalperiod);
	//(c.copits 2011-09-08) PLID 41478 - Allow global periods on service codes to exceed one year.
	if(atoi(globalperiod) > MAX_GLOBAL_PERIOD_DAYS) {
		CString strMessage;
		strMessage.Format("Please enter a global period no greater than %lu.", MAX_GLOBAL_PERIOD_DAYS);
		MsgBox(strMessage);
		_RecordsetPtr prs = CreateRecordset("SELECT GlobalPeriod FROM CPTCodeT WHERE ID = %li", ServiceID);
		if (!prs->eof)
		{
			long gp = AdoFldLong(prs, "GlobalPeriod",-1);
			if(gp != -1) {
				SetDlgItemInt(IDC_GLOBAL_PERIOD, gp);
			}
			GetDlgItem(IDC_GLOBAL_PERIOD)->SetFocus();
			return false;
		}
		prs->Close();
		return false;
	}

	// (j.jones 2011-07-06 16:14) - PLID 44358 - added UB Box 4
	//GetDlgItemText(IDC_UB_BOX4, strUBBox4);//s.tullis com

	//GetDlgItemText(IDC_ANESTH_BASE_UNITS, anesthbaseunits);	
	//make sure it's numeric only
	//anesthbaseunits = anesthbaseunits.SpanIncluding("0123456789");
	//SetDlgItemText(IDC_ANESTH_BASE_UNITS, anesthbaseunits);

	GetDlgItemText(IDC_CPT_BARCODE, barcode);
	barcode.TrimRight();
	
	if(barcode == "")
		barcode = "NULL";
	else {

		try {
			_RecordsetPtr prs;
						
			//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
			// Switch order of checking; check service code collision first because that is the deal-breaker.
			prs = CreateRecordset("SELECT Code FROM CPTCodeT INNER JOIN ServiceT ON ServiceT.ID = CPTCodeT.ID WHERE Barcode = '%s' AND ServiceT.ID <> %li", _Q(barcode), ServiceID);
			if (!prs->eof)
			{
				MsgBox("The barcode '%s' already exists for the service code '%s'", barcode, AdoFldString(prs, "Code"));
				GetDlgItem(IDC_CPT_BARCODE)->SetWindowText("");
				GetDlgItem(IDC_CPT_BARCODE)->SetFocus();
				return false;
			}
			prs->Close();

			prs = CreateRecordset("SELECT Name FROM ProductT INNER JOIN ServiceT ON ServiceT.ID = ProductT.ID WHERE Barcode = '%s'", _Q(barcode));
			if (!prs->eof)
			{
				//(c.copits 2010-09-09) PLID 40317 - Allow duplicate UPC codes for FramesData certification
				// Allow collisions between inventory item UPCs and service code UPCs, but do not allow collisions
				// between service code UPCs (in other words, maintain only 1 unique UPC code for a given service code)
				//MsgBox("The barcode '%s' already exists for the product '%s'", barcode, AdoFldString(prs, "Name"));
				CString strWarn;
				strWarn.Format("The barcode '%s' already exists for the product '%s'\n"
					"Would you like to save this barcode anyway?", barcode, AdoFldString(prs, "Name"));
				if( IDNO == MessageBox(strWarn, "Practice", MB_ICONQUESTION|MB_YESNO) ) {
					GetDlgItem(IDC_CPT_BARCODE)->SetWindowText("");
					GetDlgItem(IDC_CPT_BARCODE)->SetFocus();
					return false;
				}
			}
			prs->Close();

		}NxCatchAll("Error checking for duplicate barcodes.");

		barcode = "'" + _Q(barcode) + "'";
	}

	if(m_Taxable1.GetCheck())
		tax1 = 1;
	else
		tax1 = 0;
	
	if(m_Taxable2.GetCheck())
		tax2 = 1;
	else
		tax2 = 0;

	// (j.jones 2010-09-29 15:08) - PLID 40686 - added ability to batch even with zero
	/*if(m_checkBatchWhenZero.GetCheck()) {//s.tullis
		nBatchWhenZero = 1;
	}
	else {
		nBatchWhenZero = 0;
	}*/

	// (j.gruber 2010-08-03 10:31) - PLID 39944 - removed
	/*if(m_checkPromptForCoPay.GetCheck())
		promptcopay = 1;
	else
		promptcopay = 0;*/

/*	if(m_checkAnesthesia.GetCheck())//s.tullis
		anesthesia = 1;
	else
		anesthesia = 0;

	if(m_checkUseAnesthesiaBilling.GetCheck())
		useanesthesia = 1;
	else
		useanesthesia = 0;

	if(m_checkFacility.GetCheck())
		facilityfee = 1;
	else
		facilityfee = 0;

	if(m_checkUseFacilityBilling.GetCheck())
		usefacilityfee = 1;
	else
		usefacilityfee = 0;
*/
	// (j.jones 2010-11-19 16:55) - PLID 41544 - supported assisting codes
	/*if(m_checkAssistingCode.GetCheck()) {
		nAssistingCode = 1;
	}
	else {
		nAssistingCode = 0;
	}*/

	// (j.jones 2011-03-28 09:24) - PLID 43012 - supported Billable field
	long nBillable = 1;
	if(!m_checkBillable.GetCheck()) {
		nBillable = 0;
	}

	//GetDlgItemText(IDC_ASSISTING_BASE_UNITS, strAssistingBaseUnits);	
	//make sure it's numeric only
	//strAssistingBaseUnits = strAssistingBaseUnits.SpanIncluding("0123456789");
	//SetDlgItemText(IDC_ASSISTING_BASE_UNITS, strAssistingBaseUnits);

	// (r.gonet 03/07/2012) - PLID 48528 - Save where this code requires a referring physician or not
	//long nRequireRefPhys = 0;
	/*if(m_checkRequireRefPhys.GetCheck()){//s.tullis
		nRequireRefPhys = 1;
	}*/

	//DRT 3/13/03 - Moved the code that checks for duplicates in here, it doesn't make sense to keep it elsewhere
	try{
		if(!IsRecordsetEmpty("SELECT ServiceT.ID FROM ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID WHERE CPTCodeT.Code = '%s' AND CPTCodeT.SubCode = '%s' AND ServiceT.ID <> %li", _Q(code), _Q(subcode), ServiceID)) {

			// (j.jones 2006-05-04 12:07) - we need a current selection before we can continue
			if(m_pCodeNames->GetCurSel() == -1) {
				//we need to turn our "trysetsel" into a "setsel"
				CWaitCursor pWait;
				m_pCodeNames->SetSelByColumn(0, m_CurServiceID);
			}

			long CurSel = m_pCodeNames->GetCurSel();
			SetDlgItemText(IDC_CPTCODE,CString(m_pCodeNames->GetValue(CurSel,2).bstrVal));
			SetDlgItemText(IDC_CPTSUBCODE,CString(m_pCodeNames->GetValue(CurSel,3).bstrVal));
			m_CPTSubCode.SetFocus();
			AfxMessageBox(IDS_CPT_DUPLICATE);
			m_bIsSavingCPT = FALSE;
			return false;
		}
	}NxCatchAll("Error checking for duplicate Service Code");

	//if that succeeded, we can now update the data
	try {

		// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove prompt for copay
		// (j.jones 2010-09-29 15:08) - PLID 40686 - added ability to batch even with zero
		// (j.jones 2010-11-19 16:55) - PLID 41544 - supported assisting codes
		// (j.jones 2011-03-28 09:24) - PLID 43012 - supported Billable field
		// (r.gonet 03/26/2012) - PLID 49043 - Added in Require Referring Physician	
		// (a.wilson 2014-01-14 11:47) - PLID 59956 - parameterized and added ccdatype to update.//s.tullis
		ExecuteParamSql("UPDATE CPTCodeT SET Code = {STR}, SubCode = {STR}, TypeOfService = {STR}, "
			"RVU = {STR}, GlobalPeriod = {VT_I4}, "// BatchIfZero = {INT}, "
			/*"AssistingCode = {INT}, AssistingBaseUnits = {VT_I4},*/" Billable = { INT }, "
			"ServiceDefaultCost = CONVERT(money, {STR}) "
			"WHERE ID = {INT}", code, subcode, service, rvu, 
			NullOrIntStringToVariant(globalperiod),  
			//nBatchWhenZero,/* nAssistingCode, NullOrIntStringToVariant(strAssistingBaseUnits), */
			nBillable,  stdCost,  ServiceID);
		
		// (j.jones 2011-07-06 16:14) - PLID 44358 - added UBBox4
		// (s.dhole 2012-03-02 18:19) - PLID 47399
		// (j.gruber 2012-10-19 14:42) - PLID 53240 - change for new shop fee structure
		// (j.jones 2013-06-03 14:23) - PLID 54091 - added setting to send the claim provider
		// as ordering physician when this service is billed   // s.tullis remove strUbBox4 
		ExecuteSql("UPDATE ServiceT SET Name = '%s', Price = %s, Taxable1 = %li, Taxable2 = %li, Barcode = %s "
			//"Anesthesia = %li, UseAnesthesiaBilling = %li, FacilityFee = %li, UseFacilityBilling = %li "
			" WHERE ServiceT.ID = %li;"
			" IF 1 = %li BEGIN \r\n "
			" UPDATE ServiceLocationInfoT SET ShopFee = CONVERT(money, '%s')  \r\n"
			" FROM ServiceLocationInfoT INNER JOIN LocationsT ON ServiceLocationInfoT.LocationID = LocationsT.ID \r\n"
			" WHERE ServiceID = %li AND LocationsT.Managed = 1 AND LocationsT.Active = 1;\r\n"
			" END\r\n ",
			_Q(description), stdfee, tax1,tax2, barcode,/* anesthesia, useanesthesia, 
			facilityfee, usefacilityfee,*/ ServiceID, 
			bSaveShopFee ? 1 : 0,
			strShopFee, ServiceID);

		m_bEditChanged = FALSE;

		// (a.walling 2007-08-06 12:18) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		m_CPTChecker.Refresh(ServiceID);
	} NxCatchAll("Error in updating service code.");

	return true;	//success!
}

CString CCPTCodes::StripNonNum(CString inStr)
{
	CString outStr;

	for (int i = 0; i < inStr.GetLength(); i++) {
		if ((inStr.GetAt(i) > 47 && inStr.GetAt(i) < 58) || inStr.GetAt(i) == '.') 
			outStr+= (CString)inStr.GetAt(i);
	}
	
	return outStr;
}

void CCPTCodes::OnKillfocusCPTCode() 
{
	try{
		if(m_bIsSavingCPT || !m_bEditChanged)
			return;

		m_bIsSavingCPT = TRUE;

		CString str, str2, code,subcode;
		CString strOldCode, strOldSubCode;	//for auditing
		GetDlgItemText(IDC_CPTCODE,code);
		GetDlgItemText(IDC_CPTSUBCODE,subcode);

		if(m_CurServiceID == -1) {	//nothing is selected, so the data won't save anyways
			m_bIsSavingCPT = FALSE;
			return;
		}

		if (code == "" && subcode == "") {
			m_bIsSavingCPT = FALSE;
			return;
		}

		long ServiceID = m_CurServiceID;

		// (j.jones 2006-05-04 12:07) - we need a current selection before we can continue
		if(m_pCodeNames->GetCurSel() == -1) {
			//we need to turn our "trysetsel" into a "setsel"
			CWaitCursor pWait;
			m_pCodeNames->SetSelByColumn(0, m_CurServiceID);
		}

		strOldCode = CString(m_pCodeNames->GetValue(m_pCodeNames->CurSel, 2).bstrVal);
		strOldSubCode = CString(m_pCodeNames->GetValue(m_pCodeNames->CurSel, 3).bstrVal);

		//DRT 3/13/03 - I moved the code that was checking for duplicates inside the Save() function.  We still get very rare occasions
		//		when we get an exception report about entering a duplicate, so something is by-passing this function.  Not to mention that
		//		it's silly to check here, do 10 more things, then write the data.

		if(strOldCode != code) {

			// (j.jones 2007-08-07 09:32) - PLID 26988 - added a stronger warning, and included EMNs and case histories in the count

			long nCountOnBills = 0;
			long nCountOnBillsWithSameCode = 0;
			long nCountOnCaseHistories = 0;
			long nCountOnUnlockedEMNs = 0;
			long nCountOnLockedEMNs = 0;

			_RecordsetPtr rs = CreateRecordset("SELECT CountServices, CountType FROM ("
				//total bill/quote charges with this service
				"SELECT Count(ServiceID) AS CountServices, 1 AS CountType "
				"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE Deleted = 0 AND ServiceID = %li "
				"UNION "
				//total bill/quote charges with this service and same code
				"SELECT Count(ServiceID) AS CountServices, 2 AS CountType "
				"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE Deleted = 0 AND ServiceID = %li AND ItemCode = '%s' "
				"UNION "
				//total case history details with this service
				"SELECT Count(ItemID) AS CountServices, 3 AS CountType "
				"FROM CaseHistoryDetailsT WHERE ItemType = -2 AND ItemID = %li "
				"UNION "
				//total unlocked EMN charges with this service
				"SELECT Count(EMRMasterT.ID) AS CountServices, 4 AS CountType FROM EMRChargesT "
				"INNER JOIN EMRMasterT ON EMRChargesT.EMRID = EMRMasterT.ID "
				"WHERE EMRMasterT.Deleted = 0 AND EMRChargesT.Deleted = 0 AND EMRMasterT.Status <> 2 AND ServiceID = %li "
				"UNION "
				//total locked EMN charges with this service
				"SELECT Count(EMRMasterT.ID) AS CountServices, 5 AS CountType FROM EMRChargesT "
				"INNER JOIN EMRMasterT ON EMRChargesT.EMRID = EMRMasterT.ID "
				"WHERE EMRMasterT.Deleted = 0 AND EMRChargesT.Deleted = 0 AND EMRMasterT.Status = 2 AND ServiceID = %li "
				") AS ServicesInUseQ ORDER BY CountType ",
				ServiceID, ServiceID, _Q(strOldCode), ServiceID, ServiceID, ServiceID);
			if(!rs->eof) {

				nCountOnBills = AdoFldLong(rs, "CountServices",0);

				rs->MoveNext();

				nCountOnBillsWithSameCode = AdoFldLong(rs, "CountServices",0);

				rs->MoveNext();

				nCountOnCaseHistories = AdoFldLong(rs, "CountServices",0);

				rs->MoveNext();

				nCountOnUnlockedEMNs = AdoFldLong(rs, "CountServices",0);

				rs->MoveNext();

				nCountOnLockedEMNs = AdoFldLong(rs, "CountServices",0);
			}
			rs->Close();

			//first check locked EMNs, and disallow changing if any exist
			if(nCountOnLockedEMNs > 0) {
				
				CString str;
				str.Format("This service is in use for %li charges on locked EMN records. The code cannot be changed.\n"
					"You may, however, change the subcode.", nCountOnLockedEMNs);				
				AfxMessageBox(str);

				SetDlgItemText(IDC_CPTCODE,strOldCode);
				m_bIsSavingCPT = FALSE;
				return;
			}

			//if we have any other hits, they are allowed to change the code, but they need to be warned first
			if(nCountOnBills > 0 || nCountOnCaseHistories > 0 || nCountOnUnlockedEMNs > 0) {

				CString strWarnings;

				if(nCountOnBills > 0) {
					CString str;
					str.Format("\n - %li bill or quote charges have this service selected.", nCountOnBills);
					strWarnings += str;

					//always report how many use the same code, even if zero
					str.Format("\n - %li bill or quote charges have this service selected and use the same code.", nCountOnBillsWithSameCode);
					strWarnings += str;
				}

				if(nCountOnCaseHistories > 0) {
					CString str;
					str.Format("\n - %li details on case histories have this service selected.", nCountOnCaseHistories);
					strWarnings += str;
				}

				if(nCountOnUnlockedEMNs > 0) {
					CString str;
					str.Format("\n - %li charges on unlocked EMNs have this service selected.", nCountOnUnlockedEMNs);
					strWarnings += str;
				}

				CString str;
				str.Format("The service you are attempting to change is in use in the following patient records:\n"
					"%s\n\n"
					"It is recommended that you do not change this code, and make a new one instead.\n"
					"However if you do choose to change this code, existing bill and quote charges will continue\n"
					"to use the old code, while only new charges will use the new code.\n"
					"EMN records, Case History records, and administrative configurations, such as surgeries,\n"
					"will use the new code immediately.\n\n"
					"Are you sure you wish to change this code?", strWarnings);

				if(IDNO == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {

					SetDlgItemText(IDC_CPTCODE,strOldCode);
					m_bIsSavingCPT = FALSE;
					return;
				}
			}
		}

		if(!Save()) {
			m_bIsSavingCPT = FALSE;
			return;
		}

		//DRT 3/13/03 - Moved this below the save.  If the save failed, we don't want to change what
		//		is seen in the datalist
		long CurSel = m_pCodeNames->GetCurSel();
		if(CurSel!=-1) {
			m_pCodeNames->PutValue(CurSel,2,_bstr_t(code));
			m_pCodeNames->PutValue(CurSel,3,_bstr_t(subcode));
			m_pCodeNames->Sort();
		}

		//auditing
		long nAuditID = -1;
		if(strOldCode != code) {
			nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiCPTCode, -1, strOldCode, code, aepMedium, aetChanged);
		}
		//(e.lally 2006-09-12) PLID 22299 - Audit changes to the subcode. 
		if(strOldSubCode != subcode) {
			if(nAuditID == -1)
				nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiCPTSubCode, -1, strOldSubCode, subcode, aepMedium, aetChanged);
		}

		m_bIsSavingCPT = FALSE;
	}NxCatchAll("Error in CCPTCodes::OnKillfocusCPTCode");
}


void CCPTCodes::OnKillfocusCPTSubcode() 
{
	try{
		if(m_bIsSavingCPT || !m_bEditChanged)
			return;
	
		OnKillfocusCPTCode();
	}NxCatchAll("Error saving subcode");
}

void CCPTCodes::OnKillfocusStdFee() 
{
	if(m_bIsSavingCPT || !m_bEditChanged)
		return;

	try {

		m_bIsSavingCPT = TRUE;

		CString str;
		COleCurrency cyTmp;
		GetDlgItemText(IDC_STD_FEE,str);
		cyTmp = ParseCurrencyFromInterface(str);

		BOOL bFailed = FALSE;
		if(cyTmp.GetStatus()==COleCurrency::invalid) {
			AfxMessageBox("An invalid currency was entered as the standard fee.\n"
				"Please correct this.");
			bFailed = TRUE;
		}

		//DRT 10/15/03 - must check for the previous condition to fail or we cannot examine cyTmp
		if(!bFailed && cyTmp < COleCurrency(0,0)) {
			AfxMessageBox("Practice does not allow a negative amount for a standard fee.\n"
				"Please correct this.");
			bFailed = TRUE;
		}

		if(!bFailed && cyTmp > COleCurrency(100000000,0)) {
			CString str;
			str.Format("Practice does not allow an amount greater than %s.",FormatCurrencyForInterface(COleCurrency(100000000,0),TRUE,TRUE));
			AfxMessageBox(str);
			bFailed = TRUE;
		}

		//for auditing and resetting
		COleCurrency cyOld = COleCurrency(0,0);
		long nServiceID = -1;
		if(m_CurServiceID != -1) {
			nServiceID = m_CurServiceID;
			_RecordsetPtr rs = CreateRecordset("SELECT Price FROM ServiceT WHERE ID = %li", nServiceID);
			if(!rs->eof) {
				cyOld = AdoFldCurrency(rs, "Price", COleCurrency(0,0));
			}
			rs->Close();
		}

		if(bFailed) {
			//load the old fee
			SetDlgItemText(IDC_STD_FEE, FormatCurrencyForInterface(cyOld));
			m_bIsSavingCPT = FALSE;
			return;
		}

		str = FormatCurrencyForInterface(cyTmp);
		SetDlgItemText(IDC_STD_FEE,str);
		long CurSel = m_pCodeNames->GetCurSel();
		if(CurSel!=-1) {
			m_pCodeNames->PutValue(CurSel,4,_bstr_t(str));
			m_pCodeNames->Sort();
		}
		Save();

		//DRT 7/20/2005 - PLID 15303 - Audit changes to the standard fees
		if(cyOld != cyTmp) {
			//They did change the fee, audit it
			CString strCodeName, strSubCodeName;
			GetDlgItemText(IDC_CPTCODE, strCodeName);
			GetDlgItemText(IDC_CPTSUBCODE, strSubCodeName);

			//piece these together
			strCodeName += " - " + strSubCodeName;

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, strCodeName, nAuditID, aeiCPTFee, nServiceID, FormatCurrencyForInterface(cyOld), FormatCurrencyForInterface(cyTmp), aepMedium, aetChanged);
		}

	} NxCatchAll("Error saving standard fee.");

	m_bIsSavingCPT = FALSE;
}

void CCPTCodes::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// Do not call CNxDialog::OnPaint() for painting messages
}

void CCPTCodes::OnPaymentCategory() 
{
	try{
		CPayCatDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll("Error in CCPTCodes::OnPaymentCategory");
}

//there is no longer an OK or Cancel button, but hitting enter is ok, and hitting escape is cancel
void CCPTCodes::OnCancel() 
{
}

void CCPTCodes::OnOK() 
{
}

void CCPTCodes::StoreDetails()
{
	try {

		// (j.jones 2010-01-27 13:11) - PLID 37001 - this function is only called if the tab changed,
		// but all of our saving is dependent on killfocus, so if something changed, force a killfocus,
		// which in turn forces a save
		if(!m_bIsSavingCPT && m_bEditChanged) {
			CWnd* pWnd = GetFocus();
			if(pWnd) {
				GetDlgItem(IDC_CPT_CODES)->SetFocus();
			}
		}

	}NxCatchAll("Error in CCPTCodes::StoreDetails");
}

void CCPTCodes::OnRButtonDownCptModifiers(long nRow, long nCol, long x, long y, long nFlags) 
{
	try{
		m_pModifiers->CurSel = nRow;
		CMenu* pMenu;
		pMenu = new CMenu;
		pMenu->CreatePopupMenu();
		if(GetCurrentUserPermissions(bioServiceModifiers) & SPT____C_______ANDPASS) {
			pMenu->InsertMenu(-1, MF_BYPOSITION, ID_MOD_ADD, "Add Free Text Code");
		}
		if(GetCurrentUserPermissions(bioServiceModifiers) & SPT___W________ANDPASS) {
			pMenu->InsertMenu(-1, MF_BYPOSITION, ID_MOD_MARK_INACTIVE, "Mark Inactive");
		}
		if(GetCurrentUserPermissions(bioServiceModifiers) & SPT_____D______ANDPASS) {
			pMenu->InsertMenu(-1, MF_BYPOSITION, ID_MOD_DELETE, "Remove");
		}
		CPoint pt;
		GetCursorPos(&pt);
		pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		delete pMenu;
	}NxCatchAll("Error in CCPTCodes::OnRButtonDownCptModifiers");
	
}

void CCPTCodes::OnMenuAddModifier()
{
	try{
		AddModifier();
	}NxCatchAll("Error adding Modifier");
}

void CCPTCodes::AddModifier()
{
	if(!CheckCurrentUserPermissions(bioServiceModifiers, sptCreate)) return;

	CModifierAddNew Add(this);
	if(Add.DoModal()==IDOK) {
		
		//add the row manually
		IRowSettingsPtr pRow;
		pRow = m_pModifiers->GetRow(-1);

		pRow->PutValue(0, (_bstr_t)Add.strMod);
		pRow->PutValue(1, (_bstr_t)Add.strDesc);
		pRow->PutValue(2, (double)(Add.nMultiplier));
		pRow->PutValue(3, (_bstr_t)"");
				
		m_pModifiers->AddRow(pRow);

		try{
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiModifierCreate, -1, "", Add.strDesc, aepMedium, aetCreated);
		}NxCatchAll("Error in Auditing Modifiers");


		m_ModChecker.Refresh();
	}
}

void CCPTCodes::OnMenuDeleteModifier()
{
	try{
		DeleteModifier();
	}NxCatchAll("Error deleting Modifier");
}

void CCPTCodes::DeleteModifier()
{
	if(!CheckCurrentUserPermissions(bioServiceModifiers, sptDelete)) return;

	if(m_pModifiers->CurSel == -1) {
		AfxMessageBox("Please select a modifier before deleting.");
		return;
	}

	//for auditing
	CString strOld = CString(m_pModifiers->GetValue(m_pModifiers->CurSel, 1).bstrVal);

	_variant_t var = m_pModifiers->GetValue(m_pModifiers->CurSel,0);
	
	if (var.vt != VT_NULL && var.vt != VT_EMPTY) {

		try {
			CString strNumber = (LPCTSTR)_bstr_t(var);

			if(!IsRecordsetEmpty("SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE (CPTModifier = '%s' OR CPTModifier2 = '%s' OR CPTModifier3 = '%s' OR CPTModifier4 = '%s') AND Deleted = 0",_Q(strNumber),_Q(strNumber),_Q(strNumber),_Q(strNumber))) {
				//if there are any charges using this modifier, stop!!!
				MessageBox("There are charges using this modifier. You cannot delete a modifier that is in use.","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}

			// (j.jones 2011-04-05 15:14) - PLID 42372 - warn about CLIA Setup
			if(ReturnsRecordsParam("SELECT PersonID FROM InsuranceCoT "
				"WHERE CLIAModifier = {STRING}", strNumber)) {
				MessageBox("There are Insurance Companies using this modifier in their CLIA Setup. You cannot delete a modifier that is in use.","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}

			if(!IsRecordsetEmpty("SELECT ID FROM EMRChargesT WHERE "
				"(CPTModifier1 = '%s' OR CPTModifier2 = '%s' OR CPTModifier3 = '%s' OR CPTModifier4 = '%s')",_Q(strNumber),_Q(strNumber),_Q(strNumber),_Q(strNumber))) {
				//if there are any EMRs using this modifier, stop!!!
				MessageBox("There are EMR records using this modifier. You cannot delete a modifier that is in use.","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}

			if(!IsRecordsetEmpty("SELECT EMRTemplateID FROM EMRTemplateChargesT WHERE "
				"(CPTModifier1 = '%s' OR CPTModifier2 = '%s' OR CPTModifier3 = '%s' OR CPTModifier4 = '%s')",_Q(strNumber),_Q(strNumber),_Q(strNumber),_Q(strNumber))) {
				//if there are any EMR templates using this modifier, stop!!!
				MessageBox("There are EMR records using this modifier. You cannot delete a modifier that is in use.","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}

			//DRT 1/23/2007 - PLID 24181 - Do not allow deletion if charge actions have this set as a default.
			if(!IsRecordsetEmpty("SELECT Modifier1Number FROM EMRActionChargeDataT WHERE Modifier1Number = '%s' "
				"UNION SELECT Modifier2Number FROM EMRActionChargeDataT WHERE Modifier2Number = '%s' "
				"UNION SELECT Modifier3Number FROM EMRActionChargeDataT WHERE Modifier3Number = '%s' "
				"UNION SELECT Modifier4Number FROM EMRActionChargeDataT WHERE Modifier4Number = '%s' ", _Q(strNumber), _Q(strNumber), _Q(strNumber), _Q(strNumber))) 
			{
				MessageBox("There are EMR charge actions set to spawn this modifier by default.  You may not delete a modifier that is in use.");
				return;
			}

			// (j.jones 2007-05-23 14:57) - PLID 8993 - disallow if in an eligibility request
			if(!IsRecordsetEmpty("SELECT ID FROM EligibilityRequestsT WHERE "
				"(Modifier1 = '%s' OR Modifier2 = '%s' OR Modifier3 = '%s' OR Modifier4 = '%s')",_Q(strNumber),_Q(strNumber),_Q(strNumber),_Q(strNumber))) {
				MessageBox("There are E-Eligibility Requests using this modifier. You cannot delete a modifier that is in use.","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}

			// (d.singleton 2011-12-06 09:00) - PLID 44947 - disallow deletion if it has links to ALberta Billing Modifiers ( this should never happen in real life )
			if(!IsRecordsetEmpty("SELECT ID FROM AlbertaCptModLinkT WHERE Mod = '%s'", _Q(strNumber))) {
				MessageBox("This modifier is used for Alberta billing and is not able to be deleted", "Practice", MB_OK|MB_ICONINFORMATION);
				return;
			}

			//If anything else is added here, add the same check to CMainFrame::OnDeleteUnusedModifiers

			if(MessageBox("Are you SURE you wish to delete this modifier?", "Delete Modifier", MB_YESNO) == IDNO)
				return;

			// (j.jones 2012-03-27 09:41) - PLID 47448 - deleting CPT modifiers is now done in a shared function
			CString strSqlBatch;
			CString strModifierInClause;
			strModifierInClause.Format("'%s'", _Q(strNumber));

			DeleteCPTModifiers(strSqlBatch, strModifierInClause);

			ExecuteSqlBatch(strSqlBatch);		

		m_pModifiers->RemoveRow(m_pModifiers->CurSel);

		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiModifierDelete, -1, strOld, "<Deleted>", aepMedium, aetDeleted);

		}NxCatchAll("Error in DeleteModifier()");
		m_ModChecker.Refresh();
	}
}

void CCPTCodes::OnRButtonDownDiagCodes(long nRow, long nCol, long x, long y, long nFlags) 
{
	try{
		m_pDiagCodes->CurSel = nRow;

		CNxMenu menu;
		menu.CreatePopupMenu();
		bool bOneAdded = false;
		if(GetCurrentUserPermissions(bioDiagCodes) & SPT____C_______ANDPASS) {
			// (j.armen 2014-03-11 14:16) - PLID 61316 - Show entry for either Free Text Code (ICD-9) or ICD-10 Import
			if(m_checkDiagICD9.GetCheck() == BST_CHECKED) {
				menu.InsertMenu(-1, MF_BYPOSITION, ID_ICD_ADD, "&Add Free Text Code" );
			bOneAdded = true;
		}
			else if(m_checkDiagICD10.GetCheck() == BST_CHECKED) {
				menu.InsertMenu(-1, MF_BYPOSITION, ID_ICD_ADD, "&Import ICD-10 Codes" );
				bOneAdded = true;
			}
		}
		if(GetCurrentUserPermissions(bioDiagCodes) & SPT___W________ANDPASS) {
			menu.InsertMenu(-1, MF_BYPOSITION, ID_ICD_MARK_INACTIVE, "&Mark Inactive");
			bOneAdded = true;
		}
		if(GetCurrentUserPermissions(bioDiagCodes) & SPT_____D______ANDPASS) {
			menu.InsertMenu(-1, MF_BYPOSITION, ID_ICD_DELETE, "&Remove");
			bOneAdded = true;
		}
		if(bOneAdded) {
			menu.InsertMenu(-1, MF_BYPOSITION|MF_SEPARATOR);
		}

		if(GetCurrentUserPermissions(bioDiagCodes) & SPT___W________ANDPASS) {
			if(!IsDefaultICDSelected())
				menu.InsertMenu(-1, MF_BYPOSITION, ID_ICD_MAKE_DEFAULT, "Set As &Default");
			else
				menu.InsertMenu(-1, MF_BYPOSITION, ID_ICD_REMOVE_DEFAULT, "Remove &Default");
		}
		if (m_pDiagCodes->WhereClause == DEFAULT_ICD9_WHERE_CLAUSE)
			menu.InsertMenu(-1, MF_BYPOSITION, ID_ICD_FILTER, "&Filter...");
		else
			menu.InsertMenu(-1, MF_BYPOSITION, ID_ICD_UNFILTER, "Un&filter");

		// (r.gonet 03/06/2014) - PLID 61129 - Removed ability to import from FDB and related controls
		CPoint pt;
		GetCursorPos(&pt);
		menu.TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
		menu.DestroyMenu();

	}NxCatchAll("Error in CCPTCodes::OnRButtonDownDiagCodes");
}

void CCPTCodes::OnMenuAddICD9()
{
	try{
		OnAddIcd9();
	}NxCatchAll("Error adding Diagnois code");
}

void CCPTCodes::AddICD9()
{
	if(!CheckCurrentUserPermissions(bioDiagCodes, sptCreate)) return;

	// (c.haag 2003-07-28 11:54) - Unfilter the list first
	UnfilterIcd9();

	CDiagAddNew Add(this);
	if(Add.DoModal()==IDOK) {

		//add the row manually
		IRowSettingsPtr pRow;
		pRow = m_pDiagCodes->GetRow(-1);

		//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
		pRow->PutValue(dclcCodeNumber, (_bstr_t)Add.strCode);
		pRow->PutValue(dclcCodeDesc, (_bstr_t)Add.strDesc);
		pRow->PutValue(dclcID, (long)Add.m_ID);
		pRow->PutValue(dclcIsAMA, (_bstr_t)"");
		// (r.gonet 03/06/2014) - PLID 61129 - Removed ability to import from FDB and related controls
		// (j.jones 2013-11-21 14:48) - PLID 59640 - added column for ICD-10
		pRow->PutValue(dclcIsICD10, g_cvarFalse);
				
		m_pDiagCodes->AddRow(pRow);
	}
}

void CCPTCodes::OnMenuDeleteICD9()
{
	try{
		DeleteICD9();
	}NxCatchAll("Error deleting Diagnosis code");
}

void CCPTCodes::DeleteICD9()
{
	try {
		if(!CheckCurrentUserPermissions(bioDiagCodes, sptDelete)) return;


		// (j.armen 2014-03-11 14:43) - PLID 61321 - Renamed to diagnosis code
		if(m_pDiagCodes->CurSel == -1) {
			AfxMessageBox("Please select a diagnosis code before deleting.");
			return;
		}
		
		//for auditing
		//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
		CString strOld = CString(m_pDiagCodes->GetValue(m_pDiagCodes->CurSel, dclcCodeNumber).bstrVal);
		
		_variant_t	tmpVar;
		//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
		tmpVar = m_pDiagCodes->GetValue(m_pDiagCodes->CurSel,dclcCodeNumber);
		if(tmpVar.vt != VT_EMPTY)
		{
			//(e.lally 2006-10-06) PLID 22602 - We want to check for references in data in this order:
			//EMR, billing, patient default diag, insurance referrals. We also want to prevent deletion if it
			//appears on a bill or as a patient default diagnosis code as it is considered important enough
			//data not to wipe it out.
			// (j.armen 2012-04-03 15:51) - PLID 48299 - Made sure all of these functions are parameratized

			//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
			long nID = VarLong(m_pDiagCodes->GetValue(m_pDiagCodes->CurSel,dclcID),-1);
			//Check if it is on an EMR
			// (j.kuziel 2014-04-01 16:24) - PLID 61624 - Also check ICD-10 foreign keys.
			if(ReturnsRecordsParam("SELECT DiagCodeID FROM EMRDiagCodesT WHERE DiagCodeID = {INT} OR DiagCodeID_ICD10 = {INT}", nID, nID)) {
				MessageBox("This Diagnosis Code exists in at least one EMR. Please remove it from all EMR records before deleting.\n"
					"(You can, however, mark it inactive.)","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}
			// (z.manning, 10/16/2006) - PLID 23042 - Need to also check EMN templates.
			if(ReturnsRecordsParam("SELECT DiagCodeID FROM EmrTemplateDiagCodesT WHERE DiagCodeID = {INT} OR DiagCodeID_ICD10 = {INT}", nID, nID)) {
				MessageBox("This Diagnosis Code exists in at least one EMN template. Please remove it from all EMN templates before deleting.\n"
					"(You can, however, mark it inactive.)","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}
			//Check if it is referenced on a bill
			
			// (a.walling 2014-02-28 08:56) - PLID 61087 - BillDiagCodeT - check when deleting ICD9
			if(ReturnsRecordsParam("SELECT ID FROM BillDiagCodeT WHERE ICD9DiagID = {INT} OR ICD10DiagID = {INT}", nID, nID)) {
				MessageBox("This Diagnosis Code exists in at least one bill. Please remove it from all Billing records before deleting.\n"
					"(You can, however, mark it inactive.)","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}

			//Check if it is a patient's default diagnosis code
			// (j.jones 2014-02-18 15:50) - PLID 60719 - added a check for their ICD10 codes
			if(ReturnsRecordsParam("SELECT PersonID FROM PatientsT "
				"WHERE DefaultDiagID1 = {INT} OR DefaultDiagID2 = {INT} OR DefaultDiagID3 = {INT} OR DefaultDiagID4 = {INT} "
				"OR DefaultICD10DiagID1 = {INT} OR DefaultICD10DiagID2 = {INT} OR DefaultICD10DiagID3 = {INT} OR DefaultICD10DiagID4 = {INT}",
				nID, nID, nID, nID, nID, nID, nID, nID)) {
				MessageBox("This Diagnosis Code is marked as at least one patient's Default Diagnosis code. Please remove it from all patient General 2 records before deleting.\n"
					"(You can, however, mark it inactive.)","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}
			//Check if it is on an insurance referral
			if(ReturnsRecordsParam("SELECT DiagID FROM InsuranceReferralDiagsT WHERE DiagID = {INT}", nID)) {
				MessageBox("This Diagnosis Code exists in at least one Insurance Referral. Please remove it from all Insurance Referrals before deleting.\n"
					"(You can, however, mark it inactive.)","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}

			// (j.jones 2007-05-23 14:59) - PLID 8993 - disallow if in an eligibility request
			if(ReturnsRecordsParam("SELECT ID FROM EligibilityRequestsT "
				"WHERE Diagnosis1 = {INT} OR Diagnosis2 = {INT} OR Diagnosis3 = {INT} OR Diagnosis4 = {INT}", nID, nID, nID, nID)) {
				MessageBox("This Diagnosis Code exists in at least one E-Eligibility Request, and cannot be deleted.\n"
					"(You can, however, mark it inactive.)","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}
			
			// (j.gruber 2008-08-13 09:32) - PLID 15807 - check for it in the case history
			// (a.wilson 2014-03-06 09:25) - PLID 60774 - include icd10 fields.
			if(ReturnsRecordsParam("SELECT ID FROM CaseHistoryT "
				"WHERE PreOpDx1 = {INT} OR PreOpDx2 = {INT} OR PreOpDx3 = {INT} OR PreOpDx4 = {INT} "
				" OR PostOpDx1 = {INT} OR PostOpDx2 = {INT} OR PostOpDx3 = {INT} OR PostOpDx4 = {INT} "
				" OR PreOpDx1ICD10 = {INT} OR PreOpDx2ICD10 = {INT} OR PreOpDx3ICD10 = {INT} OR PreOpDx4ICD10 = {INT} "
				" OR PostOpDx1ICD10 = {INT} OR PostOpDx2ICD10 = {INT} OR PostOpDx3ICD10 = {INT} OR PostOpDx4ICD10 = {INT} ", 
				nID, nID, nID, nID, nID, nID, nID, nID, nID, nID, nID, nID, nID, nID, nID, nID)) {
				MessageBox("This Diagnosis Code exists in at least one Case History, and cannot be deleted.\n"
					"(You can, however, mark it inactive.)","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}

			//TES 2/18/2009 - PLID 28522 - Check for it on any prescriptions
			if(ReturnsRecordsParam("SELECT DiagCodeID FROM PatientMedicationDiagCodesT WHERE DiagCodeID = {INT}", nID)) {
				MessageBox("This Diagnosis Code exists on at least one Prescription, and cannot be deleted.\n"
					"(You can, however, mark it inactive.)","Practice",MB_OK|MB_ICONINFORMATION);
				return;
			}

			// (j.armen 2012-04-03 14:53) - PLID 48299 - Make don't allow deletion if used on a recall
			if(ReturnsRecordsParam("SELECT DiagCodeID FROM RecallT WHERE DiagCodeID = {INT}", nID)) {
				MessageBox("This Diagnosis Code exists on at least one Recall, and cannot be deleted.\n"
					"(You can, however, mark it inactive.)", "Practice", MB_OK|MB_ICONINFORMATION);
				return;
			}

			// (a.walling 2009-05-04 09:18) - PLID 28495 - Check usage on problems
			// (j.jones 2014-02-26 10:31) - PLID 60781 - and check the ICD-10 field
			{
				long nProblemCount = 0;
				long nDeletedProblemCount = 0;
				long nHistoryCount = 0;
				_RecordsetPtr prsHistory = CreateParamRecordset("SELECT "
					"(SELECT COUNT(*) FROM EMRProblemsT WHERE Deleted = 0 AND (DiagCodeID = {INT} OR DiagCodeID_ICD10 = {INT})) AS ProblemCount, "
					"(SELECT COUNT(*) FROM EMRProblemsT WHERE Deleted = 1 AND (DiagCodeID = {INT} OR DiagCodeID_ICD10 = {INT})) AS DeletedProblemCount, "
					"(SELECT COUNT(*) FROM EMRProblemHistoryT WHERE (DiagCodeID = {INT} OR DiagCodeID_ICD10 = {INT})) AS HistoryCount",
					nID, nID, nID, nID, nID, nID);
				if (!prsHistory->eof) {
					nProblemCount = AdoFldLong(prsHistory, "ProblemCount", 0);
					nDeletedProblemCount = AdoFldLong(prsHistory, "DeletedProblemCount", 0);
					nHistoryCount = AdoFldLong(prsHistory, "HistoryCount", 0); // includes the first/initial status
				}

				CString strMessage;

				if (nProblemCount != 0) {
					strMessage += FormatString("%li problems.\r\n", nProblemCount);
				}
				if (nDeletedProblemCount != 0) {
					strMessage += FormatString("%li deleted problems.\r\n", nDeletedProblemCount);
				}
				if (nHistoryCount != 0) {
					strMessage += FormatString("%li problem history records.\r\n", nHistoryCount);
				}

				if (!strMessage.IsEmpty()) {
					MessageBox(FormatString("This diagnosis code cannot be deleted. It is in use by the following problem data:\r\n\r\n%s\r\n(You can, however, mark it inactive.)", strMessage), "Practice", MB_ICONINFORMATION);
					return;
				}
			}

			// (b.savon 2014-12-26 15:06) - PLID 64355 - Don't let them delete diagnosis codes tied to actions 
			{
				_RecordsetPtr prsActions = CreateParamRecordset(
					R"(
SET NOCOUNT ON
DECLARE @spawningSourceInfo NVARCHAR(MAX) 
SET @spawningSourceInfo = ''

DECLARE @SourcesProcessed TABLE(
	sourceType INT, 
	sourceID INT
)

DECLARE @actionID INT, @sourceType INT, @sourceID INT, @temp NVARCHAR(MAX), @temp2 NVARCHAR(MAX), @temp3 NVARCHAR(MAX)

DECLARE @action_cursor CURSOR 
SET @action_cursor = CURSOR LOCAL SCROLL STATIC 
    FOR
SELECT	EmrActionsT.ID 
FROM	EmrActionDiagnosisDataT
	INNER JOIN EmrActionsT ON EmrActionDiagnosisDataT.EmrActionID = EmrActionsT.ID
WHERE	EmrActionsT.DestType = {INT} AND EmrActionsT.Deleted = 0
	AND	(EmrActionDiagnosisDataT.DiagCodeID_ICD9 = {INT} OR EmrActionDiagnosisDataT.DiagCodeID_ICD10 = {INT})

OPEN @action_cursor
FETCH NEXT FROM @action_cursor INTO @actionID

WHILE @@FETCH_STATUS = 0
BEGIN

	SET @sourceType = NULL
	SET @sourceID = NULL
	SET @temp = NULL
	SET @temp2 = NULL
	SET @temp3 = NULL

	SELECT @sourceType = A.SourceType, @sourceID = A.SourceID
	FROM EmrActionsT A
	LEFT JOIN @SourcesProcessed AS SP ON A.SourceType = SP.sourceType AND A.SourceID = SP.sourceID
	WHERE A.ID = @actionID
	AND SP.sourceType IS NULL AND SP.sourceID IS NULL

	INSERT INTO @SourcesProcessed(sourceType, sourceID) VALUES(@sourceType, @sourceID)

	IF (@sourceType = {INT}) BEGIN
		SELECT @temp = I.Name
		FROM EmrInfoT I
		WHERE I.ID = @sourceID

		SET @spawningSourceInfo = @spawningSourceInfo + ' - Item: ' + ISNULL(@temp, '') + CHAR(13)+CHAR(10)
	END
	ELSE IF (@sourceType = {INT}) BEGIN
		SELECT @temp = I.Name, @temp2 = D.Data
		FROM EmrDataT D
		INNER JOIN EmrInfoT I ON I.ID = D.EmrInfoID
		WHERE D.ID = @sourceID

		SET @spawningSourceInfo = @spawningSourceInfo + ' - Item: ' + ISNULL(@temp, '') + ', List element: ' + ISNULL(@temp2, '') + CHAR(13)+CHAR(10)
	END
	ELSE IF (@sourceType = {INT}) BEGIN
		SELECT @temp = P.Name
		FROM ProcedureT P
		WHERE P.ID = @sourceID

		SET @spawningSourceInfo = @spawningSourceInfo + ' - Procedure: ' + ISNULL(@temp, '') + CHAR(13)+CHAR(10)
	END
	ELSE IF (@sourceType = {INT}) BEGIN
		SELECT @temp = I.Name, @temp2 = HSAL.AnatomicLocation
		FROM EMRImageHotSpotsT HS
		LEFT JOIN EmrHotSpotAnatomicLocationQ HSAL ON HSAL.EmrHotSpotID = HS.ID
		INNER JOIN EmrInfoT I ON I.ID = HS.EmrInfoID
		WHERE HS.ID = @sourceID

		SET @spawningSourceInfo = @spawningSourceInfo + ' - Item: ' + ISNULL(@temp, '') + ', Hot spot: ' + ISNULL(@temp2, '') + CHAR(13)+CHAR(10)
	END
	ELSE IF (@sourceType = {INT}) BEGIN
		DECLARE @elementType NVARCHAR(10)
		SELECT @temp = I.Name, @temp2 = D.Data, @temp3 = DD.Data
			, @elementType = CASE WHEN I.TableRowsAsFields = 1 THEN 'Row' ELSE 'Column' END
		FROM EmrTableDropdownInfoT DD
		INNER JOIN EmrDataT D ON D.ID = DD.EMRDataID
		INNER JOIN EmrInfoT I ON I.ID = D.EMRInfoID
		WHERE DD.ID = @sourceID    

		SET @spawningSourceInfo = @spawningSourceInfo + ' - Item: ' + ISNULL(@temp, '') + ', ' + @elementType + ': ' + @temp2 + ', Dropdown element: ' + @temp3 + CHAR(13)+CHAR(10)
	END
	ELSE IF (@sourceType = {INT}) BEGIN
		SELECT @temp = S.StampText, @temp2 = S.TypeName
		FROM EmrImageStampsT S
		WHERE S.ID = @sourceID

		SET @spawningSourceInfo = @spawningSourceInfo + ' - Stamp: ' + ISNULL(@temp, '') + ', Type: ' + @temp2 + CHAR(13)+CHAR(10)
	END

    FETCH NEXT FROM @action_cursor INTO @actionID    

END
CLOSE @action_cursor
DEALLOCATE @action_cursor

SET NOCOUNT OFF
SELECT @spawningSourceInfo AS SpawningSourceInfo
					)",
					eaoDiagnosis,
					nID,
					nID,
					eaoEmrItem,
					eaoEmrDataItem,
					eaoProcedure,
					eaoEmrImageHotSpot,
					eaoEmrTableDropDownItem,
					eaoSmartStamp
				);

				if (!prsActions->eof){
					CString strSpawningSource = AdoFldString(prsActions->Fields, "SpawningSourceInfo", "");
					if (!strSpawningSource.IsEmpty()){
						CString strMessage;

						strMessage.Format(
							"This diagnosis code cannot be deleted because it is in use on the following EMR actions:\r\n\r\n%s\r\nYou may inactivate the diagnosis code instead of deleting it.",
							AdoFldString(prsActions->Fields, "SpawningSourceInfo", "")
							);
						MessageBox(strMessage, "Practice", MB_OK | MB_ICONINFORMATION);
						return;
					}
				}
			}

			//If anything else is added here, add the same check to CMainFrame::OnDeleteUnusedDiagcodes

			if(IDYES ==	AfxMessageBox(IDS_DIAG_DELETE, MB_YESNO))
			{
				try{

					CWaitCursor pWait;

					//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
					long nID = VarLong(m_pDiagCodes->GetValue(m_pDiagCodes->CurSel,dclcID),-1);

					// (j.jones 2012-03-27 10:49) - PLID 45752 - deleting diagnosis codes is now done in a shared function
					// (j.armen 2012-04-03 15:51) - PLID 48299 - Changed to a param batch, take in a CSqlFragment IN Clause
					CParamSqlBatch sqlBatch;
					DeleteDiagnosisCodes(sqlBatch, CSqlFragment("{INT}", nID));
					sqlBatch.Execute(GetRemoteData());

					m_pDiagCodes->RemoveRow(m_pDiagCodes->CurSel);

					// (a.walling 2007-08-06 12:25) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
					m_ICD9Checker.Refresh(nID);
				}NxCatchAll("Error in DeleteICD9()");
			}
	
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, "", nAuditID, aeiICD9Delete, -1, strOld, "<Deleted>", aepMedium, aetDeleted);
		}
	// (r.gonet 03/07/2014) - PLID 61117 - Remove references to ICD-9 in exceptions (aside from function names)
	}NxCatchAll("Error deleting diagnosis code");
}

void CCPTCodes::OnKillfocusDescription() 
{
	try{
		if(m_bIsSavingCPT || !m_bEditChanged)
			return;

		m_bIsSavingCPT = TRUE;

		CString strOldDescription, strDescription;
		GetDlgItemText(IDC_DESCRIPTION,strDescription);

		long CurSel = m_pCodeNames->GetCurSel();

		//For auditing
		strOldDescription = VarString(m_pCodeNames->GetValue(CurSel, 1), "");
		
		if(!Save()){
			m_bIsSavingCPT = FALSE;
			return;
		}

		//If the save fails, we don't want to update the datalist.
		if(CurSel!=-1) {
			m_pCodeNames->PutValue(CurSel,1,_bstr_t(strDescription));
			m_pCodeNames->Sort();
		}

		//Auditing
		//(e.lally 2006-09-12) PLID 22299 - Audit changes to the service code description
		if(strOldDescription != strDescription){
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiCPTDescription, -1, strOldDescription, strDescription, aepMedium, aetChanged);
		}

		m_bIsSavingCPT = FALSE;
	}NxCatchAll("Error saving description");
}
void CCPTCodes::OnKillfocusTOS(){


	try{
		if (m_bIsSavingCPT || !m_bEditChanged)
			return;

		m_bIsSavingCPT = TRUE;

		Save();

		m_bIsSavingCPT = FALSE;
	}NxCatchAll("Error saving RVU");




}
void CCPTCodes::OnKillfocusRvu() 
{
	try{
		if(m_bIsSavingCPT || !m_bEditChanged)
			return;

		m_bIsSavingCPT = TRUE;

		Save();

		m_bIsSavingCPT = FALSE;
	}NxCatchAll("Error saving RVU");
}

void CCPTCodes::OnEditingFinishedCptModifiers(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try{
		switch(nCol){
		case 0:
			if(bCommit) //If this is a duplicate, bCommit will be false.
				ExecuteSql("UPDATE CPTModifierT SET Number = '%s' WHERE Number = '%s';", _Q(VarString(varNewValue, "")), _Q(VarString(varOldValue, ""))); 
			break;
		case 1:
			ExecuteSql("UPDATE CPTModifierT SET Note = '%s' WHERE Number = '%s';", _Q(VarString(varNewValue, "")), _Q(VarString(m_pModifiers->GetValue(nRow, 0), "")));
			break;
		case 2:
			if(bCommit) { //If this multiplier is in use, bCommit will be false.
				CString multiplier;
				if(varNewValue.vt == VT_I4)
					multiplier.Format("%0.09g",(((double)varNewValue.lVal) + 100.0) / 100.0);
				else
					multiplier.Format("%0.09g",(varNewValue.dblVal + 100.0) / 100.0);
				ExecuteSql("UPDATE CPTModifierT SET Multiplier = %s WHERE Number = '%s';", multiplier, _Q(VarString(m_pModifiers->GetValue(nRow, 0), "")));
			}
			break;
		}

		m_ModChecker.Refresh();
	}NxCatchAll("Error in CCPTCodes::OnEditingFinishedCptModifiers");
}

void CCPTCodes::OnEditingFinishingDiagCodes(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	_RecordsetPtr rs;
	try{
		switch(nCol){
		case dclcCodeNumber:
			if(*pbCommit) {
				CString strTmp = strUserEntered;
				strTmp.TrimLeft();
				strTmp.TrimRight();
				if(strTmp == "") {
					MsgBox("You cannot enter a blank diagnosis code.");
					*pbCommit = false;
					return;
				}
				if(VarString(varOldValue, "") != VarString(*pvarNewValue, "")) {
					// (a.wilson 2013-03-28 11:29) - PLID 55594 - if only the alphabetical case was changed then continue and ignore other checks.
					if (VarString(varOldValue, "").CompareNoCase(VarString(*pvarNewValue, "")) == 0)
						return;

					//DRT 9/22/03 - PLID 9488 - Diag Codes were moved from ChargesT to BillsT.  They were also renamed to be DiagIDs, not codes in both BillsT
					//	and PatientsT.
					//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
					long nOldID = VarLong(m_pDiagCodes->GetValue(nRow, dclcID));

					// (j.jones 2007-08-07 09:10) - PLID 26988 - if on an EMN, abort
					long nLockedEMNCount = 0, nTotalEMNCount = 0;
					{
						//check locked EMNs
						_RecordsetPtr rs = CreateRecordset("SELECT Count(DiagCodeID) AS CountDiags FROM EMRDiagCodesT INNER JOIN EMRMasterT ON EMRDiagCodesT.EMRID = EMRMasterT.ID "
								"WHERE EMRMasterT.Status = 2 AND EMRDiagCodesT.Deleted = 0 AND EMRMasterT.Deleted = 0 AND EMRDiagCodesT.DiagCodeID = %li", nOldID);
						if(!rs->eof) {
							nLockedEMNCount = AdoFldLong(rs, "CountDiags",0);
						}
						rs->Close();

						//check all EMNs
						rs = CreateRecordset("SELECT Count(DiagCodeID) AS CountDiags FROM EMRDiagCodesT INNER JOIN EMRMasterT ON EMRDiagCodesT.EMRID = EMRMasterT.ID "
								"WHERE EMRDiagCodesT.Deleted = 0 AND EMRMasterT.Deleted = 0 AND EMRDiagCodesT.DiagCodeID = %li", nOldID);
						if(!rs->eof) {
							nTotalEMNCount = AdoFldLong(rs, "CountDiags",0);
						}
						rs->Close();
					}

					//should be impossible to have nLockedEMNCount > 0 and nTotalEMNCount = 0
					if(nTotalEMNCount > 0) {
						CString str;
						str.Format("You may not change this diagnosis code because it is in use on %li EMN records (%li are locked).", nTotalEMNCount, nLockedEMNCount);
						AfxMessageBox(str);
						*pbCommit = false;
						return;
					}

					// (j.jones 2014-02-26 10:31) - PLID 60781 - added check for EMR problems
					{
						long nProblemCount = 0;
						long nDeletedProblemCount = 0;
						long nHistoryCount = 0;
						_RecordsetPtr prsHistory = CreateParamRecordset("SELECT "
							"(SELECT COUNT(*) FROM EMRProblemsT WHERE Deleted = 0 AND (DiagCodeID = {INT} OR DiagCodeID_ICD10 = {INT})) AS ProblemCount, "
							"(SELECT COUNT(*) FROM EMRProblemsT WHERE Deleted = 1 AND (DiagCodeID = {INT} OR DiagCodeID_ICD10 = {INT})) AS DeletedProblemCount, "
							"(SELECT COUNT(*) FROM EMRProblemHistoryT WHERE (DiagCodeID = {INT} OR DiagCodeID_ICD10 = {INT})) AS HistoryCount",
							nOldID, nOldID, nOldID, nOldID, nOldID, nOldID);
						if (!prsHistory->eof) {
							nProblemCount = AdoFldLong(prsHistory, "ProblemCount", 0);
							nDeletedProblemCount = AdoFldLong(prsHistory, "DeletedProblemCount", 0);
							nHistoryCount = AdoFldLong(prsHistory, "HistoryCount", 0); // includes the first/initial status
						}

						CString strMessage;

						if (nProblemCount != 0) {
							strMessage += FormatString("%li problems.\r\n", nProblemCount);
						}
						if (nDeletedProblemCount != 0) {
							strMessage += FormatString("%li deleted problems.\r\n", nDeletedProblemCount);
						}
						if (nHistoryCount != 0) {
							strMessage += FormatString("%li problem history records.\r\n", nHistoryCount);
						}

						if (!strMessage.IsEmpty()) {
							MessageBox(FormatString("You may not change this diagnosis code because it is in use by the following problem data:\r\n\r\n%s", strMessage), "Practice", MB_ICONINFORMATION);
							return;
						}
					}

					long nPtCnt = 0, nBillCnt = 0;
					{
						// (a.walling 2014-03-13 14:28) - PLID 61087 - BillDiagCodeT - check when deleting ICD9
						_RecordsetPtr prs = CreateRecordset("SELECT COUNT(DISTINCT BillID) AS Cnt FROM BillDiagCodeT WHERE ICD9DiagID = %li OR ICD10DiagID = %li", 
							nOldID, nOldID);
						if(!prs->eof)
							nBillCnt = AdoFldLong(prs, "Cnt");
						prs->Close();

						// (j.jones 2014-02-18 15:50) - PLID 60719 - added a check for their ICD10 codes
						prs = CreateParamRecordset("SELECT COUNT(PersonID) AS Cnt FROM PatientsT "
							"WHERE DefaultDiagID1 = {INT} OR DefaultDiagID2 = {INT} OR DefaultDiagID3 = {INT} OR DefaultDiagID4 = {INT} "
							"OR DefaultICD10DiagID1 = {INT} OR DefaultICD10DiagID2 = {INT} OR DefaultICD10DiagID3 = {INT} OR DefaultICD10DiagID4 = {INT}",
							nOldID, nOldID, nOldID, nOldID, nOldID, nOldID, nOldID, nOldID);
						if(!prs->eof) {
							nPtCnt = AdoFldLong(prs, "Cnt");
						}
						prs->Close();
					}

					//Only need to bother if we have changes to make
					if(nPtCnt > 0 || nBillCnt > 0) {

						// (j.jones 2006-10-23 10:50) - PLID 23183 - do NOT remove the diag codes from bills or patients,
						// instead warn them that the code is in use and will change on those accounts and bills

						CString strWarning;
						strWarning.Format("This diagnosis code is already in use on %li patient's bills and is selected in General 2 for %li patients.\n"
							"Changing this code will also change it on these patients accounts! It is recommended that you make a new diagnosis code.\n"
							"Do you wish to continue changing this code number?", nBillCnt, nPtCnt);

						// (j.jones 2007-08-07 09:45) - PLID 26988 - added second confirmation message
						if(IDNO == MessageBox(strWarning,"Practice",MB_ICONEXCLAMATION|MB_YESNO) ||
							IDNO == MessageBox("Are you SURE you wish to update the existing patient data?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
							*pbCommit = false;
							return;
						}
					}

					// (r.gonet 02/20/2014) - PLID 60778 - Renamed the table to remove the reference to ICD9
					if(!IsRecordsetEmpty("SELECT ServiceID FROM CPTDiagnosisGroupsT WHERE DiagCodeID = %li",nOldID)) {

						int nResult = MessageBox("This diagnosis code is linked with Service Codes.\n"
								"Do you wish to keep the link with the new code?\n","Practice",MB_ICONEXCLAMATION|MB_YESNOCANCEL);

						if(nResult == IDCANCEL) {
							*pbCommit = false;
							return;
						}
						else if(nResult == IDNO) {
							// (r.gonet 02/20/2014) - PLID 60778 - Renamed the table to remove the reference to ICD9
							ExecuteSql("DELETE FROM CPTDiagnosisGroupsT WHERE DiagCodeID = %li",nOldID);
						}

						//code for removing from bills moved to EditingFinished.
					}

					rs = CreateRecordset("SELECT Count(CodeNumber) AS NumCount FROM DiagCodes WHERE CodeNumber = '%s'", _Q(VarString(*pvarNewValue, "")));
					if(VarLong(rs->Fields->GetItem("NumCount")->Value, 0) != 0) {//This number already exists.
						MsgBox(RCS(IDS_DIAG_DUPLICATE));
						*pbCommit = false;
					}
					rs->Close();
				}
				else
					//DRT 2/10/2006 - PLID 19268 - OnEditingFinished handles removing the diagnosis code from all existing bills, if the user
					//	agrees to do so ... but previous to today, if you didn't actually change the code, you didn't get the message, so it
					//	just blindly erased all your diagnosis codes from bills and general2.
					*pbCommit = false;
			}
		}

	}NxCatchAll("Error in CCPTCodes::OnEditingFinishingDiagCodes");
}

void CCPTCodes::OnEditingFinishedDiagCodes(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		//If this is a duplicate, bCommit will be false.
		if(!bCommit) {
			return;
		}

		// (a.walling 2007-08-06 12:27) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
		long nID = VarLong(m_pDiagCodes->GetValue(nRow, dclcID));

		switch(nCol){
		case dclcCodeNumber: {
			// (m.hancock 2006-10-03 10:10) - PLID 22298 - Only proceed if the data has actually changed.
			if(VarString(varNewValue, "") == VarString(varOldValue, ""))
				return;

			ExecuteParamSql("UPDATE DiagCodes SET CodeNumber = {STRING} WHERE ID = {INT}", VarString(varNewValue, ""), nID);
			
			// (j.jones 2006-10-23 11:08) - PLID 23183 - if the code was selected as a DiagCs pointer, it needs updated
			// (j.gruber 2014-02-28 13:27) - PLID 61106 - we don't store the code anymore, so this is not needed
			//ExecuteParamSql("UPDATE ChargesT SET WhichCodes = {STRING} WHERE WhichCodes = {STRING}", VarString(varNewValue, ""), VarString(varOldValue, ""));

			// (m.hancock 2006-09-13 15:40) - PLID 22298 - Audit updating diag code number and description
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiICD9Code, nID, VarString(varOldValue, ""), VarString(varNewValue, ""), aepMedium, aetChanged);
			break;
			}
		case dclcCodeDesc: {
			// (m.hancock 2006-10-03 10:10) - PLID 22298 - Only proceed if the data has actually changed.
			if(VarString(varNewValue, "") == VarString(varOldValue, ""))
				return;
			ExecuteParamSql("UPDATE DiagCodes SET CodeDesc = {STRING} WHERE ID = {INT}", VarString(varNewValue, ""), nID);

			// (m.hancock 2006-09-13 16:19) - PLID 22298 - Audit updating diag code number and description
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiICD9Description, nID, VarString(varOldValue, ""), VarString(varNewValue, ""), aepMedium, aetChanged);
			break;
			}
		// (j.jones 2013-11-21 14:57) - PLID 59640 - added ICD-10 as a hidden column
	    // (j.jones 2014-01-21 16:37) - PLID 60418 - this is not editable anymore
		/*
		case dclcIsICD10: {
			BOOL bOldValue = VarBool(varOldValue, FALSE);
			BOOL bNewValue = VarBool(varNewValue, FALSE);
			if(bOldValue == bNewValue) {
				return;
			}
			ExecuteParamSql("UPDATE DiagCodes SET ICD10 = {BIT} WHERE ID = {INT}", bNewValue ? 1 : 0, nID);

			CString strOld, strNew;
			strOld.Format("Code: %s, Is ICD-10: %s", VarString(m_pDiagCodes->GetValue(nRow, dclcCodeNumber), ""), bOldValue ? "Yes" : "No");
			strNew.Format("Is ICD-10: %s", bNewValue ? "Yes" : "No");
			
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiDiagCodeIsICD10, nID, strOld, strNew, aepMedium, aetChanged);
			break;
			}
		*/
		}
		
		// (a.walling 2007-08-06 12:26) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		m_ICD9Checker.Refresh(nID);
	}NxCatchAll("Error in CCPTCodes::OnEditingFinishedDiagCodes");
}

void CCPTCodes::OnEditingFinishingCptModifiers(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try{
		_RecordsetPtr rs;
		switch(nCol){
		case 0:
			if(VarString(varOldValue, "") != VarString(*pvarNewValue, "")) {
				CString mod = VarString(varOldValue, "");
				if(!IsRecordsetEmpty("SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"WHERE (CPTModifier = '%s' OR CPTModifier2 = '%s' OR CPTModifier3 = '%s' OR CPTModifier4 = '%s') AND Deleted = 0",_Q(mod),_Q(mod),_Q(mod),_Q(mod))) {
					//if there are any charges using this modifier, stop!!!
					//re-set it and continue or else it will be impossible to leave the box
					*pbCommit = FALSE;					
					// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
					VariantClear(pvarNewValue);
					*pvarNewValue = _variant_t(varOldValue).Detach();
					MessageBox("There are charges using this modifier. Please make a new modifier instead.","Practice",MB_OK|MB_ICONINFORMATION);
					break;
				}

				// (j.jones 2011-04-05 15:14) - PLID 42372 - warn about CLIA Setup
				if(ReturnsRecordsParam("SELECT PersonID FROM InsuranceCoT "
					"WHERE CLIAModifier = {STRING}", mod)) {
					MessageBox("There are Insurance Companies using this modifier in their CLIA Setup. Please make a new modifier instead.","Practice",MB_OK|MB_ICONINFORMATION);
					return;
				}

				if(!IsRecordsetEmpty("SELECT EmrChargesT.ID FROM EmrChargesT "
					"WHERE (CPTModifier1 = '%s' OR CPTModifier2 = '%s' OR CPTModifier3 = '%s' OR CPTModifier4 = '%s')",_Q(mod),_Q(mod),_Q(mod),_Q(mod))) {
					//if there are any EMNs using this modifier, stop!!!
					//re-set it and continue or else it will be impossible to leave the box
					*pbCommit = FALSE;
					// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
					VariantClear(pvarNewValue);
					*pvarNewValue = _variant_t(varOldValue).Detach();
					MessageBox("There are EMNs using this modifier. Please make a new modifier instead.","Practice",MB_OK|MB_ICONINFORMATION);
					break;
				}
				if(!IsRecordsetEmpty("SELECT EmrTemplateChargesT.ID FROM EmrTemplateChargesT "
					"WHERE (CPTModifier1 = '%s' OR CPTModifier2 = '%s' OR CPTModifier3 = '%s' OR CPTModifier4 = '%s')",_Q(mod),_Q(mod),_Q(mod),_Q(mod))) {
					//if there are any EMNs using this modifier, stop!!!
					//re-set it and continue or else it will be impossible to leave the box
					*pbCommit = FALSE;
					// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
					VariantClear(pvarNewValue);
					*pvarNewValue = _variant_t(varOldValue).Detach();
					MessageBox("There are EMN Templates using this modifier. Please remove this modifier from any EMN templates which use it, or make a new modifier instead.","Practice",MB_OK|MB_ICONINFORMATION);
					break;
				}

				rs = CreateRecordset("SELECT Active FROM CPTModifierT WHERE Number = '%s'", _Q(VarString(*pvarNewValue, "")));
				if(!rs->eof) {
					//This number already exists.
					if(AdoFldBool(rs, "Active")) {
						// (a.walling 2010-08-16 17:30) - PLID 40136 - MessageBox does not automatically load string resources
						MessageBox(CString(RCS(IDS_MODIFIER_DUPLICATE)));
					}
					else {
						// (z.manning, 05/01/2007) - PLID 16623 - If it's inactive, let them know so they can find it.
						MessageBox("This modifier already exists, but is inactive. Please click the 'Inactive Codes' button if you'd like to reactivate it.");
					}
					*pbCommit = false;
					*pbContinue = false;
				}
				rs->Close();				
			}
			break;
		case 1: {
			if(VarString(*pvarNewValue,"").GetLength() > 255) {
				*pbCommit = false;
				*pbContinue = false;
				AfxMessageBox("The description you entered is too long, please shorten it to less than 255 characters.");
			}
			break;
		}
		case 2: {
			CString mod = VarString(m_pModifiers->GetValue(nRow, 0), "");
			CString oldVal, newVal;
			oldVal.Format("%0.09g",varOldValue.dblVal);
			if(pvarNewValue->vt == VT_I4)
				newVal.Format("%li",pvarNewValue->lVal);
			else
				newVal.Format("%0.09g",pvarNewValue->dblVal);
			if(!IsRecordsetEmpty("SELECT ChargesT.ID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE (CPTModifier = '%s' OR CPTModifier2 = '%s' OR CPTModifier3 = '%s' OR CPTModifier4 = '%s') AND Deleted = 0",_Q(mod),_Q(mod),_Q(mod),_Q(mod))
				&& (oldVal != newVal)) {
				//if there are any charges using this modifier, prompt them
				if(IDNO == MessageBox("There are charges using this modifier. Changing the value of its multiplier will only affect new charges.\n"
									  "Existing charges will continue to use the old multiplier unless the modifier is re-selected on each charge.\n\n"
									  "Are you sure you wish to change this multipler?","Practice",MB_YESNO|MB_ICONINFORMATION)) {
					*pbCommit = FALSE;
					*pvarNewValue = varOldValue;
					return;
				}
			}

			// (j.jones 2004-06-01 14:15) - don't let them save a value less than -100
			if(pvarNewValue->dblVal < -100.0) {				
				if(varOldValue.dblVal < -100.0) {
					//if the old value was invalid (from an old version, perhaps),
					//then set the new value to -100 and let it save
					*pvarNewValue = _variant_t((double)-100.0);
				}
				else {
					*pbCommit = FALSE;
					*pvarNewValue = varOldValue;
				}
				AfxMessageBox("You cannot have a multiplier less than -100.");
				return;
			}

			break;
		}
		}

	}NxCatchAll("Error in CCPTCodes::OnEditingFinishingCptModifiers");
}

void CCPTCodes::OnRequeryFinishedCptCodes(short nFlags) 
{
	try{
		m_bRequeryFinished = true;

		// (j.jones 2006-05-04 14:03) - I've seen TrySetSel ultimately not work if you play around
		// with the code list while it is requerying for a long time. So, force a setsel.
		// (e.lally 2006-10-10) PLID 22786 - New users do not have a last used CPT code so the current service ID
		// would be -1. We need to attempt to set the current selection to the first in the list.
		if(m_CurServiceID != -1)
			m_pCodeNames->SetSelByColumn(0, m_CurServiceID);
		else{
			m_pCodeNames->CurSel = 0;
			if(m_pCodeNames->CurSel == -1){
				//We tried to set the current selection to the first in the list and that failed after the list
				//is done requerying. We will make sure the current service ID is still -1 and give up.
				m_CurServiceID = -1;
			}
			else{
				m_CurServiceID = VarLong(m_pCodeNames->GetValue(m_pCodeNames->CurSel, 0), -1);
				Load();
			}
		}

		//Double-check the enabling of the arrows
		if(m_pCodeNames->CurSel < 1) {
			m_btnCptLeft.EnableWindow(FALSE);
		}
		else {
			m_btnCptLeft.EnableWindow(TRUE);
		}
		if(m_pCodeNames->CurSel < m_pCodeNames->GetRowCount()-1 && m_pCodeNames->CurSel != -1) {
			m_btnCptRight.EnableWindow(TRUE);
		}
		else {
			m_btnCptRight.EnableWindow(FALSE);
		}

		GetDlgItem(IDC_DELETE_CPT)->EnableWindow(GetCurrentUserPermissions(bioServiceCodes) & SPT_____D______ANDPASS);
	}NxCatchAll("Error in CCPTCodes::OnRequeryFinishedCptCodes");
}

void CCPTCodes::OnKillfocusGlobalPeriod() 
{
	try{
		if(m_bIsSavingCPT || !m_bEditChanged)
			return;

		m_bIsSavingCPT = TRUE;

		Save();

		m_bIsSavingCPT = FALSE;
	}NxCatchAll("Error saving Global Period");
}

void CCPTCodes::Refresh()
{
	if (m_CPTChecker.Changed())
	{
		m_bRequeryFinished = false;
		m_pCodeNames->Requery();
		m_CurServiceID = GetRemotePropertyInt("LastCPTUsed",-1,0,GetCurrentUserName(),FALSE);
		if(m_CurServiceID != -1)
			m_pCodeNames->TrySetSelByColumn(0,m_CurServiceID);
	}
	if (m_ModChecker.Changed())
	{
		// (d.singleton 2011-11-22 10:35) - PLID 44946 - once alberta mods import is done requery the datalist
		if(UseAlbertaHLINK())
		{
			m_pAlbertaModifiers->Requery();
		}
		else
		{
			m_pModifiers->Requery();
		}
	}
	if (m_ICD9Checker.Changed())
	{
		m_pDiagCodes->Requery();
		// (c.haag 2015-06-24) - PLID 66018 - Also refresh the custom crosswalk list in case someone changed 
		// a code number or deleted an ICD code.
		RefreshCustomNexGEMsList();
	}
	if (m_CPTCategoryChecker.Changed())
	{
		//do nothing now
	}
	/*if(m_UB92CatChecker.Changed()) {
		m_UB92_Category->Requery();
		IRowSettingsPtr pRow;
		pRow = m_UB92_Category->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, (LPCTSTR)"<None>");
		pRow->PutValue(2, (LPCTSTR)"<No Category Selected>");
		m_UB92_Category->AddRow(pRow);
	}*/

	if(m_providerChecker.Changed()) {
		m_DefaultProviderCombo->Requery();

		IRowSettingsPtr pRow;
		_variant_t var;
		var.vt = VT_NULL;
		
		// (j.jones 2011-06-24 15:13) - PLID 22586 - now we have an option to "use default",
		// and one to literally default to no provider
		pRow = m_DefaultProviderCombo->GetRow(-1);
		pRow->PutValue(0,var);
		pRow->PutValue(1,_variant_t(" {Use Default Bill Provider}"));
		m_DefaultProviderCombo->AddRow(pRow);
		pRow = m_DefaultProviderCombo->GetRow(-1);
		pRow->PutValue(0,(long)-2);
		pRow->PutValue(1,_variant_t(" {Use No Provider}"));
		m_DefaultProviderCombo->AddRow(pRow);	
	}

	// (j.jones 2010-07-30 16:15) - PLID 39728 - supported pay groups
	if(m_PayGroupChecker.Changed()) {

		//we don't need to retain the current selection,
		//because Load() is always called by this function

		m_PayGroupCombo->Requery();

		NXDATALIST2Lib::IRowSettingsPtr pNoPayGroup = m_PayGroupCombo->GetNewRow();
		pNoPayGroup->PutValue(pgccID, (long)-1);
		pNoPayGroup->PutValue(pgccName, _bstr_t(" <No Default Pay Group>"));
		m_PayGroupCombo->AddRowSorted(pNoPayGroup, NULL);
	}

	// (c.haag 2015-05-15) - PLID 66017
	if (m_CustomNexGEMsChecker.Changed())
	{
		RefreshCustomNexGEMsList();
	}

	if(GetRemotePropertyInt("EnableCodeLinkLaunching", 0, 0, "<None>", true) == 0) {
		GetDlgItem(IDC_BTN_LAUNCH_CODELINK)->ShowWindow(SW_HIDE);
	}
	else {		
		GetDlgItem(IDC_BTN_LAUNCH_CODELINK)->ShowWindow(SW_SHOW);
	}

	// (j.jones 2009-03-30 16:59) - PLID 32324 - show/hide the premium codes button
	// based on the OHIP preference, and show/hide the ICD9 V3 code alternatively
	// This has to be done in OnInitDialog for when being loaded from a bill,
	// and in Refresh when in the Billing tab of Admin.
	// (j.jones 2010-07-08 14:01) - PLID 39566 - this is now cached as a member variable,
	// so compare the current setting to the cached setting - if the setting didn't change,
	// we don't need to alter the display (doing so steals focus)
	BOOL bUseOHIP = UseOHIP();
	if(bUseOHIP != m_bUseOHIP) {
		//the setting changed
		m_bUseOHIP = bUseOHIP;
		if(m_bUseOHIP) {
		//	m_btnOHIPPremiumCodes.ShowWindow(SW_SHOW);
			//m_btnSetupICD9v3.ShowWindow(SW_HIDE);
			//m_nxstaticIcd9v3Label.ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_LIST_ICD9V3)->ShowWindow(SW_HIDE);// (s.tullis 2014-05-22 09:13) - PLID 61829 - moved
		}
		else {
		//	m_btnOHIPPremiumCodes.ShowWindow(SW_HIDE);
			//m_btnSetupICD9v3.ShowWindow(SW_SHOW);
			//m_nxstaticIcd9v3Label.ShowWindow(SW_SHOW);
			//GetDlgItem(IDC_LIST_ICD9V3)->ShowWindow(SW_SHOW);
		}
	}

	Load();
}

void CCPTCodes::OnTaxable1() 
{
	try{
		Save();
	}NxCatchAll("Error saving Taxable 1");
}

void CCPTCodes::OnTaxable2() 
{
	try{
		Save();
	}NxCatchAll("Error saving Taxable 2");
}

void CCPTCodes::OnAddCpt() 
{
	try{
		if(!CheckCurrentUserPermissions(bioServiceCodes, sptCreate)) {
			return;
		}

		// (c.haag 2003-07-28 11:56) - Unfilter the list first
		UnfilterCPT();

		//DRT 3/13/03 - I'm very suspicious of this Save(), I don't really see any reason it needs to be here...
		//		So I'm commenting it out for now.
		//Save();
		
		CCPTAddNew dlgAdd(this);

		if (dlgAdd.DoModal() == IDOK) {

			//TS 6/4/2001: It used to be that this would be reenabled in OnRequeryFinish, but there's no
			//requery anymore, fool!
			//GetDlgItem(IDC_DELETE_CPT)->EnableWindow(FALSE);

			if(m_CPTChecker.Changed()) {
				m_bRequeryFinished = false;
				m_pCodeNames->Requery();
			}
			else {
				//if the CPT list has not changed (by another user), then manually add the row		
			
				IRowSettingsPtr pRow;
				pRow = m_pCodeNames->GetRow(-1);
				pRow->PutValue(0, (long)dlgAdd.m_nID);
				pRow->PutValue(1, (_bstr_t)dlgAdd.strName);
				pRow->PutValue(2, (_bstr_t)dlgAdd.strCode);
				pRow->PutValue(3, (_bstr_t)dlgAdd.strSubCode);
				// (j.jones 2010-01-11 09:11) - PLID 24504 - changed the member cost to a currency
				pRow->PutValue(4, (_bstr_t)FormatCurrencyForInterface(dlgAdd.m_cyPrice));
				m_pCodeNames->AddRow(pRow);
			}

			//and now select the CPT you just added
			m_CurServiceID = dlgAdd.m_nID;
			m_pCodeNames->TrySetSelByColumn(0, m_CurServiceID);

			// (d.singleton 2011-10-19 17:53) - PLID refresh alberta mods data
			if(UseAlbertaHLINK())
			{
				CString strAlbertaModsWhere;
				strAlbertaModsWhere.Format("ServiceID = %li", m_CurServiceID);
				m_pAlbertaModifiers->PutWhereClause(_bstr_t(strAlbertaModsWhere));
				m_pAlbertaModifiers->Requery();
			}

			//and send a table checker
			// (a.walling 2007-08-06 12:20) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			m_CPTChecker.Refresh(m_CurServiceID);
			
			try{
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1)
					AuditEvent(-1, "", nAuditID, aeiCPTCreate, -1, "", dlgAdd.strCode, aepMedium, aetCreated);
			}NxCatchAll("Error in CPT");		

			Load();
			m_CPTCode.SetFocus();
		}	
	}NxCatchAll("Error adding Service Code");
}

void CCPTCodes::OnAddModifier() 
{
	try{
		AddModifier();	
	}NxCatchAll("Error adding Modifier");
}

void CCPTCodes::OnDeleteModifier() 
{
	try{
		DeleteModifier();
	}NxCatchAll("Error deleting Modifier");
}

void CCPTCodes::OnAddIcd9() 
{
	try{
		// (j.armen 2014-03-11 14:16) - PLID 61316 - When Add is clicked, either add ICD9 or ICD10
		// No more context menu
		if(m_checkDiagICD9.GetCheck() == BST_CHECKED) {
			// Old free text function
			AddICD9();
			Refresh();
		}	
		else if(m_checkDiagICD10.GetCheck() == BST_CHECKED) {
			// (j.armen 2014-03-10 08:24) - PLID 61210 - Added option for adding ICD10 Codes
			CImportICD10CodesDlg dlg(this);
			if(dlg.DoModal() == IDOK) {
				m_pDiagCodes->Requery();
			}
		}
	}NxCatchAll("Error adding Diagnosis code");
}

void CCPTCodes::OnDeleteIcd9() 
{
	try{
		DeleteICD9();
	}NxCatchAll("Error deleting Diagnosis code");
}

void CCPTCodes::OnDeleteCpt() 
{
	try {
		if(!CheckCurrentUserPermissions(bioServiceCodes, sptDelete)) {
			return;
		}

		//for auditing

		// (j.jones 2006-05-04 12:07) - we need a current selection before we can continue
		if(m_pCodeNames->GetCurSel() == -1) {
			//we need to turn our "trysetsel" into a "setsel"
			CWaitCursor pWait;
			m_pCodeNames->SetSelByColumn(0, m_CurServiceID);
		}

		CString strOld = CString(m_pCodeNames->GetValue(m_pCodeNames->CurSel, 2).bstrVal);
		
			
		if(m_CurServiceID == -1) {
			AfxMessageBox("Please select a service code before deleting.");
			return;
		}

		long ServiceID = m_CurServiceID;

		CString			sql;
		COleVariant		tmpVar;
		CString			csCode, csSubCode;

		if(!IsRecordsetEmpty("SELECT ID FROM ChargesT WHERE ServiceID = %li",ServiceID)) {
			MessageBox("This Service Code exists in at least one bill. You cannot delete this code. You can, however, mark it inactive.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (c.haag 2009-10-12 12:56) - PLID 35722 - Check MailSentServiceCptT
		if(!IsRecordsetEmpty("SELECT ID FROM MailSentServiceCptT WHERE ServiceID = %li",ServiceID)) {
			MessageBox("This Service Code is currently linked with at least one patient photo. You cannot delete this code. You can, however, mark it inactive.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		if(!IsRecordsetEmpty("SELECT ID FROM EMRChargesT WHERE ServiceID = %li",ServiceID)) {
			MessageBox("This Service Code exists in at least one EMR record. You cannot delete this code. You can, however, mark it inactive.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (z.manning, 05/21/2008) - PLID 27042 - Need to check EMN templates as well
		if(!IsRecordsetEmpty("SELECT ID FROM EMRTemplateChargesT WHERE ServiceID = %li",ServiceID)) {
			MessageBox("This Service Code exists on at least one EMR template. You cannot delete this code. You can, however, mark it inactive.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		if(!IsRecordsetEmpty("SELECT SurgeryID FROM SurgeryDetailsT WHERE ServiceID = %li",ServiceID)) {
			MessageBox("This Service Code exists in at least one Surgery. Please remove it from all Surgeries before deleting.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (j.jones 2009-08-27 10:12) - PLID 35124 - check Preference Cards
		if(!IsRecordsetEmpty("SELECT PreferenceCardID FROM PreferenceCardDetailsT WHERE ServiceID = %li",ServiceID)) {
			MessageBox("This Service Code exists in at least one Preference Card. Please remove it from all Preference Cards before deleting.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		if(!IsRecordsetEmpty("SELECT ServiceID FROM InsuranceReferralCPTCodesT WHERE ServiceID = %li",ServiceID)) {
			MessageBox("This Service Code exists in at least one Insurance Referral. Please remove it from all Insurance Referrals before deleting.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (j.jones 2007-05-23 14:57) - PLID 8993 - disallow if in an eligibility request
		if(!IsRecordsetEmpty("SELECT ID FROM EligibilityRequestsT WHERE ServiceID = %li",ServiceID)) {
			MessageBox("This Service Code exists in at least one E-Eligibility Request. You cannot delete this code. You can, however, mark it inactive.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (a.walling 2007-05-30 12:43) - PLID 25356 - Prevent deleting if suggested sales master item
		if(!IsRecordsetEmpty("SELECT MasterServiceID FROM SuggestedSalesT WHERE MasterServiceID = %lu", ServiceID)) {
			MessageBox("This Service Code has suggested sales attached. You cannot delete this code until they are removed. You can, however, mark it inactive.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (j.jones 2007-08-23 15:24) - PLID 27055 - disallow if on an E/M Checklist Coding Level
		if(!IsRecordsetEmpty("SELECT ID FROM EMChecklistCodingLevelsT WHERE ServiceID = %li",ServiceID)) {
			MessageBox("This Service Code exists in at least one E/M Checklist Coding Level. You cannot delete this code. You can, however, mark it inactive.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (j.jones 2009-12-28 13:18) - PLID 36711 - fixed two dependencies we never caught in the past
		if(!IsRecordsetEmpty("SELECT ID FROM BillsT WHERE UB92Box44 = %li",ServiceID)) {
			MessageBox("This Service Code is selected as a UB Box 44 code on at least one bill. You cannot delete this code. You can, however, mark it inactive.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		if(!IsRecordsetEmpty("SELECT CPTID FROM ServiceToProductLinkT WHERE CPTID = %li",ServiceID)) {
			MessageBox("This Service Code has inventory items linked to it. You cannot delete this code. You can, however, mark it inactive.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		//TES 4/25/2011 - PLID 41113 - Can't delete items that are in the "Items to Bill" section of a Glasses Order
		// (j.dinatale 2012-05-10 17:52) - PLID 49078 - Glasses orders are now optical orders
		if(!IsRecordsetEmpty("SELECT TOP 1 ServiceID FROM GlassesOrderServiceT WHERE ServiceID = %li",ServiceID)) {
			MessageBox("This Service Code is in use on a Optical Order.  You cannot delete this code.  You can, however, mark it inactive.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (z.manning 2011-07-05 10:39) - PLID 44421 - Can't delete a code in an EMR coding group
		_RecordsetPtr prsCodingGroup = CreateParamRecordset(
			"SELECT TOP 1 Name \r\n"
			"FROM EmrCodingGroupDetailsT \r\n"
			"INNER JOIN EmrCodingGroupRangesT ON EmrCodingGroupDetailsT.EmrCodingGroupRangeID = EmrCodingGroupRangesT.ID \r\n"
			"INNER JOIN EmrCodingGroupsT ON EmrCodingGroupRangesT.EmrCodingGroupID = EmrCodingGroupsT.ID \r\n"
			"WHERE CptCodeID = {INT} \r\n"
			, ServiceID);
		if(!prsCodingGroup->eof) {
			CString strCodingGroupName = AdoFldString(prsCodingGroup, "Name", "");
			MessageBox("This Service Code is used in EMR coding group " + strCodingGroupName + 
				".  You cannot delete this code.  You can, however, mark it inactive.", NULL, MB_OK|MB_ICONINFORMATION);
			return;
		}
		// (s.dhole 2012-04-09 15:37) - PLID 43849 check cpt code
		// (s.dhole 2012-04-24 08:55) -  Change IsRecordsetEmpty to ReturnsRecordsParam
		// (j.dinatale 2012-04-17 17:26) - PLID 49078 - changed glasses order to optical order
		if(ReturnsRecordsParam(
			" Select top 1 CptId  FROM ( \r\n"
			" SELECT top 1 CptId  FROM GlassesCatalogDesignsCptT  WHERE CPTID ={INT}   \r\n"
			" UNION \r\n"
			" SELECT top 1 CptId FROM GlassesCatalogTreatmentsCptT WHERE CPTID ={INT}    \r\n"
			" UNION \r\n"
			" SELECT  top 1 CptId FROM GlassesCatalogMaterialsCptT WHERE CPTID ={INT}   ) _Q   \r\n" 
			, ServiceID, ServiceID, ServiceID)) {
				MessageBox("This Service Code is in use on an Optical Order.  You cannot delete this code.  You can, however, mark it inactive.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}


		/********* If you add more restrictions from deleting Service Codes, be sure to update CMainFrame::OnDeleteUnusedServiceCodes as well *********/

		if(MessageBox("Are you SURE you wish to delete this Service Code?", "Delete Service Code", MB_YESNO) == IDNO)
			return;
			
		CString strSqlBatch;

		// (j.jones 2009-12-28 09:58) - PLID 32150 - deletion queries are now in this global function
		DeleteServiceCodes(strSqlBatch, AsString(ServiceID));

		//executing in a batch means we no longer need the forced transaction that used to be here
		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		// (j.jones 2006-05-04 12:07) - we need a current selection before we can continue
		if(m_pCodeNames->GetCurSel() == -1) {
			//we need to turn our "trysetsel" into a "setsel"
			CWaitCursor pWait;
			m_pCodeNames->SetSelByColumn(0, m_CurServiceID);
		}

		int CurSel = m_pCodeNames->CurSel;			
		m_pCodeNames->RemoveRow(m_pCodeNames->CurSel);			
		//try to select the next code
		if(CurSel == m_pCodeNames->GetRowCount())
			CurSel--;
		m_pCodeNames->CurSel = CurSel;
		if(m_pCodeNames->CurSel == -1) {
			m_pCodeNames->CurSel = 0;
			if(m_pCodeNames->CurSel == -1) {
				m_CurServiceID = -1;
				return;
			}
			else {
				m_CurServiceID = VarLong(m_pCodeNames->GetValue(m_pCodeNames->CurSel,0));
			}
		}
		else {
			m_CurServiceID = VarLong(m_pCodeNames->GetValue(m_pCodeNames->CurSel,0));
		}
		m_CPTChecker.Refresh(ServiceID);

		Load();
		
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiCPTDelete, ServiceID, strOld, "<Deleted>",aepMedium, aetDeleted);
		
	} NxCatchAll("Error in OnClickDeleteCPT");
}

void CCPTCodes::OnMarkInactive() 
{
	try {

		if(m_CurServiceID == -1) {
			AfxMessageBox("Please select a service code before marking inactive.");
			return;
		}

		if(MessageBox("Deactivating a Service Code will make it unavailable on this screen, as well as a bill or quote. However, you can still view it in reports."
			"\nYou can re-activate this Service Code by clicking 'Inactive Codes' at the bottom of this screen.'"
			"\n\nAre you SURE you wish to deactivate this Service Code?", "Deactivate Service Code", MB_YESNO|MB_ICONEXCLAMATION) == IDNO)
			return;

		CString strSql = BeginSqlBatch();

		// (j.jones 2006-05-04 12:07) - we need a current selection before we can continue
		if(m_pCodeNames->GetCurSel() == -1) {
			//we need to turn our "trysetsel" into a "setsel"
			CWaitCursor pWait;
			m_pCodeNames->SetSelByColumn(0, m_CurServiceID);
		}

		long ServiceID = m_CurServiceID;
		CString strName = VarString(m_pCodeNames->GetValue(m_pCodeNames->CurSel,2),"") + " " + VarString(m_pCodeNames->GetValue(m_pCodeNames->CurSel,3),"")
				+ " - " + VarString(m_pCodeNames->GetValue(m_pCodeNames->CurSel,1),"");

		CString			sql;
		COleVariant		tmpVar;
		CString			csCode, csSubCode;

		if(!IsRecordsetEmpty("SELECT SurgeryID FROM SurgeryDetailsT WHERE ServiceID = %li",ServiceID)) {
			MessageBox("This Service Code exists in at least one Surgery. Please remove it from all Surgeries before deactivating.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (j.jones 2009-08-27 10:12) - PLID 35124 - check Preference Cards
		if(!IsRecordsetEmpty("SELECT PreferenceCardID FROM PreferenceCardDetailsT WHERE ServiceID = %li",ServiceID)) {
			MessageBox("This Service Code exists in at least one Preference Card. Please remove it from all Preference Cards before deactivating.","Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// check and see if the CPT code is associated with any procedures, if so, let the user know which one
		_RecordsetPtr rs = CreateRecordset("Select ProcedureT.Name AS Name FROM ProcedureT Left Join ServiceT ON ProcedureT.ID = ServiceT.ProcedureID WHERE ServiceT.ID = %li", ServiceID);
		if(!rs->eof){			
			CString strMessage, strProcedure = AdoFldString(rs->GetFields(), "Name", "");
			strMessage.Format("This Service Code exists in the procedure %s.  Please remove it from this procedure before deactivating this Service Code.", strProcedure);
			MessageBox(strMessage,"Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}

		// (j.gruber 2010-07-21 11:02) - PLID 30481 - make sure its not linked to any appt types
		rs = CreateRecordset("Select AptTypeT.Name AS Name FROM ApptTypeServiceLinkT LEFT JOIN AptTypeT ON ApptTypeServiceLinkT.AptTypeID = AptTypeT.ID WHERE ApptTypeServiceLinkT.ServiceID = %li", ServiceID);
		if(!rs->eof){			
			CString strMessage;
			CString strNames;
			while (!rs->eof) {
				strNames += AdoFldString(rs->GetFields(), "Name", "") + "\r\n";				
				rs->MoveNext();
			}
			strMessage.Format("This Service Code is linked to the following Appointment Type(s):\r\n%sPlease remove it before deactivating this Service Code.", strNames);
			MessageBox(strMessage,"Practice",MB_OK|MB_ICONINFORMATION);
			return;
		}
		
		// (a.walling 2007-07-27 09:00) - PLID 15998 - Check to see if it has a sale applied
		rs = CreateRecordset("SELECT COUNT(*) AS SaleCount FROM SaleItemsT WHERE ServiceID = %li GROUP BY ServiceID", ServiceID);
		if (!rs->eof) {
			long nSaleCount = AdoFldLong(rs, "SaleCount", 0);
			if(nSaleCount > 0) {
				CString strMessage;
				strMessage.Format("This Service Code exists in %li sale%s. Marking it as inactive will remove its discount information from all sales. Would you like to continue inactivating this code?", nSaleCount, nSaleCount > 0 ? "s" : "");
				if (IDNO == MessageBox(strMessage,"Practice",MB_YESNO|MB_ICONINFORMATION))
					return;
				
				AddStatementToSqlBatch(strSql, "DELETE FROM SaleItemsT WHERE ServiceID = %li", ServiceID);
			}
		}

		// remove sale info
		// (a.walling 2007-07-27 09:08) - PLID 15998 - Clear out sale info
		AddStatementToSqlBatch(strSql, "UPDATE ServiceT SET Active = 0 WHERE ID = %li", ServiceID);
		AddStatementToSqlBatch(strSql, "UPDATE ServiceT SET ProcedureID = NULL WHERE ID = %li", ServiceID);
		ExecuteSqlBatch(strSql);

		// (j.jones 2006-05-04 12:07) - we need a current selection before we can continue
		if(m_pCodeNames->GetCurSel() == -1) {
			//we need to turn our "trysetsel" into a "setsel"
			CWaitCursor pWait;
			m_pCodeNames->SetSelByColumn(0, m_CurServiceID);
		}

		int CurSel = m_pCodeNames->CurSel;
		m_pCodeNames->RemoveRow(m_pCodeNames->CurSel);			
		//try to select the next code
		if(CurSel == m_pCodeNames->GetRowCount())
			CurSel--;
		m_pCodeNames->CurSel = CurSel;
		if(m_pCodeNames->CurSel == -1) {
			m_pCodeNames->CurSel = 0;
			if(m_pCodeNames->CurSel == -1) {
				m_CurServiceID = -1;
				return;
			}
			else {
				m_CurServiceID = VarLong(m_pCodeNames->GetValue(m_pCodeNames->CurSel,0));
			}
		}
		else {
			m_CurServiceID = VarLong(m_pCodeNames->GetValue(m_pCodeNames->CurSel,0));
		}
		m_CPTChecker.Refresh(ServiceID);

		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, strName, nAuditID, aeiCPTActive, ServiceID, "", "<Marked Inactive>",aepMedium, aetDeleted);

		Load();

	} NxCatchAll("Error in OnClickDeleteCPT");
}

void CCPTCodes::OnSelChosenCptCodes(long nRow) 
{
	try{
		if(m_pCodeNames->CurSel == -1) {
			m_pCodeNames->CurSel = 0;		
			if(m_pCodeNames->CurSel == -1) {
				m_CurServiceID = -1;
				return;
			}
			else {
				m_CurServiceID = VarLong(m_pCodeNames->GetValue(m_pCodeNames->CurSel,0));
			}
		}

		if(m_pCodeNames->CurSel != -1) {
			m_CurServiceID = VarLong(m_pCodeNames->GetValue(m_pCodeNames->CurSel,0));
		}

		// (d.singleton 2011-12-05 11:00) - PLID 44947 if using alberta billing the mods datalist will requery when changing the service code,  as each service code can have a different list of mods.
		if(UseAlbertaHLINK())
		{
			CString strAlbertaModsWhere;
			strAlbertaModsWhere.Format("ServiceID = %li", m_CurServiceID);
			m_pAlbertaModifiers->PutWhereClause(_bstr_t(strAlbertaModsWhere));
			m_pAlbertaModifiers->Requery();
		}

		Load();
	}NxCatchAll("Error in CCPTCodes::OnSelChosenCptCodes");
}
/*// // (s.tullis 2014-05-22 09:13) - PLID 61829 - moved
void CCPTCodes::OnSelChosenUb92CptCategories(long nRow) 
{
	if(nRow == -1)
		return;
	
	try {

		long category = m_UB92_Category->GetValue(nRow, 0).lVal;

		//Check whether they have "<none>" selected
		if(category == -1){
			ExecuteSql("UPDATE ServiceT SET UB92Category = NULL WHERE ID = %li", m_CurServiceID);
			m_UB92_Category->CurSel = -1; //We don't want it to show "<none>"
		}
		else{
			ExecuteSql("UPDATE ServiceT SET UB92Category = %li WHERE ID = %li", category, m_CurServiceID);
		}

	}NxCatchAll("Could not save category.");
}*/

void CCPTCodes::OnLeftCpt() 
{
	try {
		if(m_pCodeNames->CurSel > 0) {
			m_pCodeNames->CurSel = m_pCodeNames->CurSel-1;
			OnSelChosenCptCodes(m_pCodeNames->CurSel);
			if(m_pCodeNames->CurSel < 1) {
				m_btnCptLeft.EnableWindow(FALSE);
			}
			if(m_pCodeNames->CurSel == m_pCodeNames->GetRowCount()-1) {
				m_btnCptRight.EnableWindow(FALSE);
			}
			else {
				m_btnCptRight.EnableWindow(TRUE);
			}
		}
	}NxCatchAll("Error in CCPTCodes::OnLeftCpt()");
}

void CCPTCodes::OnRightCpt() 
{
	try {
		if(m_pCodeNames->CurSel < m_pCodeNames->GetRowCount()-1 && m_pCodeNames->CurSel != -1) {
			m_pCodeNames->CurSel = m_pCodeNames->CurSel+1;
			OnSelChosenCptCodes(m_pCodeNames->CurSel);
			if(m_pCodeNames->CurSel == m_pCodeNames->GetRowCount()-1) {
				m_btnCptRight.EnableWindow(FALSE);
			}
			if(m_pCodeNames->CurSel < 1) {
				m_btnCptLeft.EnableWindow(FALSE);
			}
			else {
				m_btnCptLeft.EnableWindow(TRUE);
			}
		}
	}NxCatchAll("Error in CCPTCodes::OnRightCpt()");
}

void CCPTCodes::OnColumnClickingCptCodes(short nCol, BOOL FAR* bAllowSort) 
{
	try {

		if(nCol >= 1 && nCol <= 4) {
			SetRemotePropertyInt("CPTEditSort", nCol, 0, GetCurrentUserName());
		}
	} NxCatchAll("CCPTCodes::Save:StorePrimarySortCol");
}

void CCPTCodes::OnUpdatePrices() 
{
	try{
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_UPDATE_BY_PERCENT, "Update By &Percentage...");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_UPDATE_BY_RVU, "Update By &RVU...");
		// (a.walling 2007-02-22 13:09) - PLID 2470 - Update standard prices from a CSV file
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_UPDATE_FROM_FILE, "Update From &File...");
		
		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_UPDATE_PRICES);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}	
	}NxCatchAll("Error in CCPTCodes::OnUpdatePrices");
}

void CCPTCodes::OnUpdatePricesByPercentage()
{
	try{
		CUpdatePriceDlg dlg(this);
		dlg.nType = 1;	//cpt codes
		if(dlg.DoModal() == IDOK) {
			m_CPTChecker.Refresh();
			//TES 2/19/2004: We have to refresh our own list ourself, not using the Checker.
			m_bRequeryFinished = false;
			m_pCodeNames->Requery();
			m_CurServiceID = GetRemotePropertyInt("LastCPTUsed",-1,0,GetCurrentUserName(),FALSE);
			if(m_CurServiceID != -1)
				m_pCodeNames->TrySetSelByColumn(0,m_CurServiceID);
			Refresh();
		}
	}NxCatchAll("Error in CCPTCodes::OnUpdatePricesByPercentage");
}

void CCPTCodes::OnUpdatePricesByRVU()
{
	try{
		CUpdateFeesByRVUDlg dlg(this);
		dlg.DoModal();
		m_CPTChecker.Refresh();
		Refresh();
	}NxCatchAll("Error in CCPTCodes::OnUpdatePricesByRVU");
}

// (a.walling 2007-02-22 13:09) - PLID 2470 - Update standard prices from a CSV file
void CCPTCodes::OnUpdatePricesFromFile()
{
	try {
		CMultiFeesImportDlg dlg(this);
		dlg.m_nFeeGroup = -1; // to update standard fees

		dlg.DoModal();
		m_CPTChecker.Refresh();

		// (a.walling 2007-02-26 16:57) - PLID 2470 - According to Tom above, we need to do this ourselves
		m_bRequeryFinished = false;
		m_pCodeNames->Requery();
		m_CurServiceID = GetRemotePropertyInt("LastCPTUsed",-1,0,GetCurrentUserName(),FALSE);
		if(m_CurServiceID != -1)
			m_pCodeNames->TrySetSelByColumn(0,m_CurServiceID);
		Refresh();
	}NxCatchAll("Error in CCPTCodes::OnUpdatePricesFromFile");
}

void CCPTCodes::OnMarkIcd9Inactive() 
{
	try {

		// (j.kuziel 2014-03-10) - PLID 61211 - ICD-10 codes can also be inactivated and will pass through this logic.
		if(m_pDiagCodes->CurSel == -1) {
			AfxMessageBox("Please select a diagnosis code before marking inactive.");
			return;
		}

		if(MessageBox("Deactivating a diagnosis code will make it unavailable on this screen, as well as on a bill or General 2. However, you can still view it in reports."
			"\nYou can re-activate this diagnosis code by clicking 'Inactive Codes' at the bottom of this screen."
			"\n\nAre you SURE you wish to deactivate this diagnosis code?", "Deactivate Diagnosis Code", MB_YESNO|MB_ICONEXCLAMATION) == IDNO)
			return;

		//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
		long DiagID = m_pDiagCodes->GetValue(m_pDiagCodes->CurSel, dclcID).lVal;
		CString strName = VarString(m_pDiagCodes->GetValue(m_pDiagCodes->CurSel,dclcCodeNumber),"") + " - " + VarString(m_pDiagCodes->GetValue(m_pDiagCodes->CurSel,dclcCodeDesc),"");

		CString			sql;
		COleVariant		tmpVar;
		CString			csCode, csSubCode;

		ExecuteSql("UPDATE DiagCodes SET Active = 0 WHERE ID = %li", DiagID);
		m_pDiagCodes->RemoveRow(m_pDiagCodes->CurSel);
		m_ICD9Checker.Refresh(DiagID);

		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, strName, nAuditID, aeiICD9Active, DiagID, "", "<Marked Inactive>",aepMedium, aetDeleted);

	} NxCatchAll("Error in OnMarkIcd9Inactive");
}

void CCPTCodes::OnReceiptConfig() 
{
	try{
		CReceiptConfigDlg  dlg(this);
		dlg.DoModal();
	}NxCatchAll("Error in CCPTCodes::OnReceiptConfig");
}

void CCPTCodes::OnSelChosenDefaultCPTProvider(long nRow) 
{
	if(m_CurServiceID == -1)
		return;

	try {

		_variant_t varProvID = g_cvarNull;

		// (j.jones 2011-06-24 15:18) - PLID 22586 - we now support -2 for literally having "no" provider
		// on charges created with this item, but will save NULL instead of -1 which means we aren't
		// specifying any special provider
		if(nRow != -1) {
			_variant_t var = m_DefaultProviderCombo->GetValue(nRow,0);
			if(var.vt == VT_I4 && VarLong(var) != -1) {
				varProvID = var;
			}
		}

		ExecuteParamSql("UPDATE ServiceT SET ProviderID = {VT_I4} WHERE ID = {INT}", varProvID, m_CurServiceID);

	}NxCatchAll("Error setting default provider.");
}

//check to see if a given row is a default row
BOOL CCPTCodes::IsDefaultICDSelected() {

	if(m_pDiagCodes->CurSel == -1)
		return FALSE;

	long nDefault = GetRemotePropertyInt("DefaultICD9Code",-1,0,"<None>",TRUE);
	int nRow = m_pDiagCodes->FindByColumn(2,(long)nDefault,0,FALSE);
	return (nRow == m_pDiagCodes->CurSel);
}

void CCPTCodes::SetAsDefault()
{
	if(m_pDiagCodes->CurSel == -1)
		return;

	//first remove colors
	long nOldDefault = GetRemotePropertyInt("DefaultICD9Code",-1,0,"<None>",TRUE);
	//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
	int nRow = m_pDiagCodes->FindByColumn(dclcID,(long)nOldDefault,0,FALSE);
	if(nRow >= 0)
		IRowSettingsPtr(m_pDiagCodes->GetRow(nRow))->PutForeColor(RGB(0,0,0));

	//save the default
	//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
	long nNewDefault = m_pDiagCodes->GetValue(m_pDiagCodes->CurSel,dclcID).lVal;
	SetRemotePropertyInt("DefaultICD9Code",nNewDefault,0,"<None>");

	//now set colors
	IRowSettingsPtr(m_pDiagCodes->GetRow(m_pDiagCodes->CurSel))->PutForeColor(RGB(255,0,0));	

	// (a.walling 2007-08-06 12:27) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
	m_ICD9Checker.Refresh(nNewDefault);
}

void CCPTCodes::RemoveDefault()
{
	if(m_pDiagCodes->CurSel == -1)
		return;

	//first remove colors
	long nOldDefault = GetRemotePropertyInt("DefaultICD9Code",-1,0,"<None>",TRUE);
	//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
	int nRow = m_pDiagCodes->FindByColumn(dclcID,(long)nOldDefault,0,FALSE);
	if(nRow >= 0)
		IRowSettingsPtr(m_pDiagCodes->GetRow(nRow))->PutForeColor(RGB(0,0,0));
	//now remove the default
	SetRemotePropertyInt("DefaultICD9Code",-1,0,"<None>");

	// (a.walling 2007-08-06 12:27) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
	m_ICD9Checker.Refresh(nOldDefault);
}

void CCPTCodes::OnRequeryFinishedDiagCodes(short nFlags) 
{
	try{
		long nDefault = GetRemotePropertyInt("DefaultICD9Code",-1,0,"<None>",TRUE);
		//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
		int nRow = m_pDiagCodes->FindByColumn(dclcID,(long)nDefault,0,FALSE);
		if(nRow >= 0) {
			IRowSettingsPtr(m_pDiagCodes->GetRow(nRow))->PutForeColor(RGB(255,0,0));
		}
		// (r.gonet 03/06/2014) - PLID 61129 - Removed ability to import from FDB and related controls
	}NxCatchAll("Error in CCPTCodes::OnRequeryFinishedDiagCodes");
}

void CCPTCodes::OnUpdateCategories() 
{
	try{
		//opens a dialog that allows the user to update the categories on multiple
		//cpt codes
		CMultiCategoryUpdateDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			m_CPTChecker.Refresh();	//fire the cpt checker - categories changed on them

			UpdateView();
		}
	}NxCatchAll("Error managing Service code categories");
}

void CCPTCodes::UnfilterIcd9()
{
	if (m_pDiagCodes->WhereClause == DEFAULT_ICD9_WHERE_CLAUSE)
		return;

	// (c.haag 2003-07-28 10:20) - Restore the filter and column colors
	m_pDiagCodes->WhereClause = DEFAULT_ICD9_WHERE_CLAUSE;
	for (short i=0; i < m_pDiagCodes->ColumnCount; i++) {
		if(GetCurrentUserPermissions(bioDiagCodes) & SPT___W________ANDPASS) {
			m_pDiagCodes->GetColumn(i)->BackColor = RGB(255,255,255);
		}
		else {
			m_pDiagCodes->GetColumn(i)->BackColor = GetSysColor(COLOR_BTNFACE);
		}
	} 
	m_pDiagCodes->Requery();

	SetDlgItemText(IDC_FILTER_ICD9, "Filter");
}

void CCPTCodes::UnfilterCPT()
{
	if (CString((LPCTSTR)m_pCodeNames->WhereClause) == DEFAULT_CPT_WHERE_CLAUSE)
		return;

	// (c.haag 2003-07-28 10:20) - Restore the filter and column colors
	m_pCodeNames->WhereClause = DEFAULT_CPT_WHERE_CLAUSE;
	for (short i=0; i < m_pCodeNames->ColumnCount; i++)
		m_pCodeNames->GetColumn(i)->BackColor = RGB(255,255,255);

	m_bRequeryFinished = false;
	m_pCodeNames->Requery();

	// (c.haag 2003-07-28 11:45) - Restore the current value
	if (m_CurServiceID != -1) {
		m_pCodeNames->TrySetSelByColumn(0, m_CurServiceID);
		Load();
	}
	else {
		m_pCodeNames->CurSel = 0;
		OnSelChosenCptCodes(m_pCodeNames->CurSel);
	}

	SetDlgItemText(IDC_FILTER_CPT, "Filter");
}

void CCPTCodes::OnFilterICD9() 
{
	try{
		if (m_pDiagCodes->WhereClause == DEFAULT_ICD9_WHERE_CLAUSE)
		{
			if (FilterDatalist(m_pDiagCodes, 1, 0))
				SetDlgItemText(IDC_FILTER_ICD9, "Unfilter");
		}
		else
		{
			UnfilterIcd9();
		}
	}NxCatchAll("Error in CCPTCodes::OnFilterICD9");
}

void CCPTCodes::OnFilterCPT() 
{
	try{
		if (CString((LPCTSTR)m_pCodeNames->WhereClause) == DEFAULT_CPT_WHERE_CLAUSE)
		{
			if (FilterDatalist(m_pCodeNames, 1, 0))
			{
				SetDlgItemText(IDC_FILTER_CPT, "Unfilter");
				OnSelChosenCptCodes(m_pCodeNames->CurSel);
			}
		}
		else
		{
			UnfilterCPT();
		}
	}NxCatchAll("Error in CCPTCodes::OnFilterCPT");
}

void CCPTCodes::OnKillfocusCptBarcode() 
{
	try{
		if(m_bIsSavingCPT || !m_bEditChanged)
			return;

		m_bIsSavingCPT = TRUE;

		Save();

		m_bIsSavingCPT = FALSE;	
	}NxCatchAll("Error saving Barcode");
}

LRESULT CCPTCodes::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	//This will call Save() for whatever has focus, meaning that when we call save in a second, the only thing that 
	//could fail will be the Barcode.
	GetDlgItem(IDC_CPT_BARCODE)->SetFocus();

	CString strPreviousCode;
	GetDlgItemText(IDC_CPT_BARCODE, strPreviousCode);
	// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
	_bstr_t bstr = (BSTR)lParam;
	GetDlgItem(IDC_CPT_BARCODE)->SetWindowText((LPCTSTR)bstr);
	m_bIsSavingCPT = TRUE;
	try{
		if(Save()) {
			((CNxEdit*)GetDlgItem(IDC_CPT_BARCODE))->SetSel(0, -1);
		}
		else {
			GetDlgItem(IDC_CPT_BARCODE)->SetWindowText(strPreviousCode);
		}
	}NxCatchAll("Error in OnBarcodeScan");
	m_bIsSavingCPT = FALSE;
	return 0;
}

void CCPTCodes::OnConfigBillColumns() 
{
	try{
		CConfigBillColumnsDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll("Error managing visible bill columns");
}

// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove promptforcopay
/*void CCPTCodes::OnCheckPromptForCopay() 
{
	try{
		Save();
	}NxCatchAll("Error saving Prompt for CoPay option");
}*/

/*void CCPTCodes::OnAdvRevcodeSetup() 
{
	try{
		CAdvRevCodeSetupDlg dlg(this);
		dlg.DoModal();

		//reflect any change that could have been made
		Refresh();
	}NxCatchAll("Error managing advanced revenue code setup");
}*/
// (s.tullis 2014-04-29 16:04) - // (s.tullis 2014-05-22 09:13) - PLID 61829 - moved
/*void CCPTCodes::OnCheckSingleRevCode() // s.tullis comment out later
{
	long RevCodeUse = 0;

	if(m_checkSingleRevCode.GetCheck()) {
		//single
		RevCodeUse = 1;
		m_checkMultipleRevCodes.SetCheck(FALSE);
		m_UB92_Category->Enabled = TRUE;
		m_btnEditMultipleRevCodes.EnableWindow(FALSE);
	}
	else {
		//none
		RevCodeUse = 0;
		m_checkMultipleRevCodes.SetCheck(FALSE);
		m_UB92_Category->Enabled = FALSE;
		m_btnEditMultipleRevCodes.EnableWindow(FALSE);
	}

	try	{
		
		if(m_CurServiceID == -1)
			return;

		ExecuteSql("UPDATE ServiceT SET RevCodeUse = %li WHERE ID = %li",
			RevCodeUse, m_CurServiceID);

		// (a.walling 2007-08-06 12:21) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		m_CPTChecker.Refresh(m_CurServiceID);

	}NxCatchAll("Could not update revenue code setup.");
}

void CCPTCodes::OnCheckMultipleRevCodes() //s.tullis comment later
{
	long RevCodeUse = 0;

	if(m_checkMultipleRevCodes.GetCheck()) {
		//multiple
		RevCodeUse = 2;
		m_checkSingleRevCode.SetCheck(FALSE);
		m_UB92_Category->Enabled = FALSE;
		m_btnEditMultipleRevCodes.EnableWindow(TRUE);
	}
	else {
		//none
		RevCodeUse = 0;
		m_checkSingleRevCode.SetCheck(FALSE);
		m_UB92_Category->Enabled = FALSE;
		m_btnEditMultipleRevCodes.EnableWindow(FALSE);
	}

	try	{
		
		if(m_CurServiceID == -1)
			return;

		ExecuteSql("UPDATE ServiceT SET RevCodeUse = %li WHERE ID = %li",
			RevCodeUse, m_CurServiceID);

		// (a.walling 2007-08-06 12:21) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		m_CPTChecker.Refresh(m_CurServiceID);

	}NxCatchAll("Could not update revenue code setup.");
}*/


/*//s.tullis PLID 61829 move to UB Setup
void CCPTCodes::OnBtnEditMultipleRevCodes() 
{
	try{
		if(m_CurServiceID == -1)
			return;
	
		int nRet= -1;
		CUBSetupDLG dlg(this);
		dlg.m_nServiceID = m_CurServiceID;
		nRet =dlg.DoModal();

		//reflect any change that could have been made
		//Refresh();
	}NxCatchAll("Error editing multiple revenue code setup");
}
*///s.tullis PLID 61829 move to Anes/Facil Setup
/*void CCPTCodes::OnCheckAnesthesia() 
{
	if(m_CurServiceID == -1)
		return;

	try {

		//only one can be toggled at a time
		if(m_checkAnesthesia.GetCheck() && m_checkFacility.GetCheck()) {
			m_checkFacility.SetCheck(FALSE);
			m_checkUseFacilityBilling.SetCheck(FALSE);
		}
		// (j.jones 2010-11-19 16:50) - PLID 41544 - uncheck assisting code
		if(m_checkAnesthesia.GetCheck() && m_checkAssistingCode.GetCheck()) {
			m_checkAssistingCode.SetCheck(FALSE);
		}

		if(!m_checkAnesthesia.GetCheck()) {
			m_checkUseAnesthesiaBilling.SetCheck(FALSE);
		}

		// (j.jones 2010-11-19 16:38) - PLID 41544 - renamed to include assisting codes
		CheckEnableAnesthesiaFacilityAssistingControls();

		Save();

		//see if the standard fee should be grayed out (for Anesth./Facility purposes)
		CheckEnableStandardFee();

		// (j.jones 2010-11-19 17:03) - PLID 41544 - hide assisting code info. if not OHIP, or if Anesthesia is checked
		CheckDisplayAssistingCodeControls();

	}NxCatchAll("Error updating Anesthesia status.");
}*/

void CCPTCodes::OnBtnImportCpt() 
{
	try{
		// (a.walling 2008-12-10 14:45) - PLID 32355 - Merged the OHIP Code Importer into Practice
		// This means we now have a choice, whether to import an AMA file or an OHIP file.
		CMenu mnu;
		if (mnu.CreatePopupMenu()) {
			enum EImportCodeOptions {
				eCPT = 100,
				eOHIP = 101,
				eAlberta = 102,
			};

			mnu.AppendMenu(MF_STRING, eCPT, "Import &AMA Code File");
			mnu.AppendMenu(MF_STRING, eOHIP, "Import &OHIP Code File");
			if(UseAlbertaHLINK())
			{
				mnu.AppendMenu(MF_STRING, eAlberta, "Import Alberta Code File");
			}

			CPoint pt;
			
			CWnd *pWnd = GetDlgItem(IDC_BTN_IMPORT_CPT);
			if (pWnd) {
				CRect rc;
				pWnd->GetWindowRect(&rc);
				pt.x = rc.right;
				pt.y = rc.top;
			} else {
				GetMessagePos(pt);
			}
			DWORD dwReturn = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y,this);	

			switch (dwReturn) {
				case eCPT:
					ImportCPTCodes();
					Refresh();
					break;
				case eOHIP:
					{
						// (a.walling 2008-12-10 14:45) - PLID 32355 - Import the OHIP file
						COHIPImportCodesDlg dlg(this);
						dlg.DoModal();
					}
					Refresh();
					break;
				case eAlberta:					
					ImportAlbertaCodes();
					Refresh();
					break;
			}
		}

	}NxCatchAll("Error importing Service codes");
}

// (r.gonet 02/20/2014) - PLID 60778 - Renamed the function to remove the reference to ICD9
void CCPTCodes::OnBtnCptDiagnosisLinking() 
{
	try{
		// (r.gonet 02/20/2014) - PLID 60778 - Renamed the function to remove the reference to ICD9
		CCPTDiagnosisLinkingDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll("Error linking service codes and diagnosis codes");
}

void CCPTCodes::OnBtnSelectCategory() 
{
	try {

		if(m_CurServiceID == -1)
			return;

		long nServiceID = m_CurServiceID;
		
		// (j.jones 2015-03-02 14:05) - PLID 64963 - there are now multiple categories per service code
		CString strOldCategoryNames;
		std::vector<long> aryOldCategoryIDs;
		long nOldDefaultCategoryID = -1;
		LoadServiceCategories(nServiceID, aryOldCategoryIDs, strOldCategoryNames, nOldDefaultCategoryID);

		// (j.jones 2015-02-27 16:22) - PLID 64962 - added bAllowMultiSelect, true for service codes
		// (j.jones 2015-03-02 15:36) - PLID 64970 - added strItemType
		CCategorySelectDlg dlg(this, true, "service code");
		
		// (j.jones 2015-03-02 08:55) - PLID 64962 - this dialog supports multiple categories
		if (aryOldCategoryIDs.size() > 0) {
			dlg.m_aryInitialCategoryIDs.insert(dlg.m_aryInitialCategoryIDs.end(), aryOldCategoryIDs.begin(), aryOldCategoryIDs.end());
			dlg.m_nInitialDefaultCategoryID = nOldDefaultCategoryID;
		}

		// (j.jones 2015-03-02 10:18) - PLID 64962 - this now is just an OK/Cancel dialog
		if(IDOK == dlg.DoModal()) {	//save it

			// (j.jones 2015-03-02 14:26) - PLID 64963 - now save the multiple categories and the optional default

			//save & audit using the global function, which uses the API
			std::vector<long> aryServiceIDs;
			aryServiceIDs.push_back(nServiceID);
			UpdateServiceCategories(aryServiceIDs, dlg.m_arySelectedCategoryIDs, dlg.m_nSelectedDefaultCategoryID, false);
			
			//now load the proper display string
			CString strNewCategoryNames;
			LoadServiceCategories(dlg.m_arySelectedCategoryIDs, dlg.m_nSelectedDefaultCategoryID, strNewCategoryNames);
			
			SetDlgItemText(IDC_CATEGORY_BOX, strNewCategoryNames);
		}

	} NxCatchAll("Error changing category.");
}

void CCPTCodes::OnBtnRemoveCategory() 
{
	try {

		if(m_CurServiceID == -1)
			return;

		if(IDNO == MessageBox("Are you sure you wish to remove this code's categories?","Practice",MB_ICONQUESTION|MB_YESNO))
			return;

		// (j.jones 2015-03-02 15:16) - PLID 64963 - use the global function that calls the API
		std::vector<long> aryServiceIDs;
		aryServiceIDs.push_back(m_CurServiceID);
		std::vector<long> aryCategoryIDs;
		UpdateServiceCategories(aryServiceIDs, aryCategoryIDs, -1, false);

		SetDlgItemText(IDC_CATEGORY_BOX, "");

	} NxCatchAll("Error removing category.");
}

void CCPTCodes::OnKillfocusShopFee() 
{	//DRT 4/16/2004 - Copied this from the Standard fee KillFocus() function

	try {
		if(m_bIsSavingCPT || !m_bEditChanged)
			return;

		//can't save if nothing is selected in the list
		if(m_pCodeNames->GetCurSel() == sriNoRow)
			return;

		m_bIsSavingCPT = TRUE;

		long nServiceID = m_CurServiceID;
		CString str;
		COleCurrency cyTmp;
		GetDlgItemText(IDC_SHOP_FEE, str);
		cyTmp = ParseCurrencyFromInterface(str);

		BOOL bFailed = FALSE;
		if(cyTmp.GetStatus() == COleCurrency::invalid) {
			AfxMessageBox("An invalid currency was entered as the shop fee.\n"
				"Please correct this.");
			bFailed = TRUE;
		}

		//DRT 10/15/03 - must check for the previous condition to fail or we cannot examine cyTmp
		if(!bFailed && cyTmp < COleCurrency(0,0)) {
			AfxMessageBox("Practice does not allow a negative amount for a shop fee.\n"
				"Please correct this.");
			bFailed = TRUE;
		}

		if(!bFailed && cyTmp > COleCurrency(100000000,0)) {
			CString str;
			str.Format("Practice does not allow a shop fee greater than %s.",FormatCurrencyForInterface(COleCurrency(100000000,0),TRUE,TRUE));
			AfxMessageBox(str);
			bFailed = TRUE;
		}

		//DRT 4/23/2004 - The shop fee can't be larger than the cost of the item.
		if (!bFailed) {
			COleCurrency cyItem;
			CString strTmp;
			GetDlgItemText(IDC_STD_FEE, strTmp);
			cyItem = ParseCurrencyFromInterface(strTmp);
			if (cyTmp > cyItem) {
				MsgBox("You cannot enter a shop fee greater than the item's standard price.");
				bFailed = TRUE;
			}
		}

		// (j.gruber 2012-10-19 12:41) - PLID 53240 - make it use our new stored variable
		if(bFailed) {
			//load the old fee
			if (m_cyShopFee < COleCurrency(0,0)) {
				str = "<Multiple>";
			}
			else {
				str = FormatCurrencyForInterface(m_cyShopFee);
			}

			SetDlgItemText(IDC_SHOP_FEE, str);
			m_bIsSavingCPT = FALSE;
			return;
		}

		//format it correctly
		str = FormatCurrencyForInterface(cyTmp);
		SetDlgItemText(IDC_SHOP_FEE, str);
	

		//for auditing
		CString strOld;
		if(!bFailed) {
			//if we have an enabled dialog, it means we only have one shop fee for all locations	
			//there is a slight possibility that someone else updated the data while we already have this code opened, but that is super slight and auding will reflect what happened, so we are OK with it
			_RecordsetPtr prs = CreateParamRecordset("SELECT top 1 ShopFee FROM ServiceLocationInfoT INNER JOIN LocationsT ON ServiceLocationInfoT.LocationID = LocationsT.ID WHERE ServiceID = {INT} AND LocationsT.Managed = 1 AND LocationsT.Active = 1", nServiceID);
			if(!prs->eof) {
				strOld = "For All Managed, Active Locations: " + FormatCurrencyForInterface(AdoFldCurrency(prs->Fields,"ShopFee"));
			}
		}

		// (j.gruber 2012-10-19 14:51) - PLID 53240 - set the member variable
		m_cyShopFee = cyTmp;

		//save to data once we've made it this far
		Save();

		//auditing
		if(strOld != str) {
			long nAuditID = BeginNewAuditEvent();

			// (j.jones 2006-05-04 12:07) - we need a current selection before we can continue
			if(m_pCodeNames->GetCurSel() == -1) {
				//we need to turn our "trysetsel" into a "setsel"
				CWaitCursor pWait;
				m_pCodeNames->SetSelByColumn(0, m_CurServiceID);
			}

			AuditEvent(-1, VarString(m_pCodeNames->GetValue(m_pCodeNames->GetCurSel(), 1)), nAuditID, aeiShopFee, nServiceID, strOld, str, aepHigh, aetChanged);
		}

		m_bIsSavingCPT = FALSE;

	} NxCatchAll(__FUNCTION__);
}

void CCPTCodes::OnBtnLaunchCodelink() 
{
	try{
		LaunchCodeLink(GetSafeHwnd());
	}NxCatchAll("Error launching code link");
}

LRESULT CCPTCodes::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{
		UINT nIdc = (UINT)wParam;
		switch(nIdc) {
		case IDC_AMA_TEXT_LABEL:
			PopupAMACopyright();
			break;
		// (j.jones 2012-01-03 17:04) - PLID 47300 - added NOC label
		//case IDC_NOC_LABEL:
			//PopupNOCDescription();
		//	break;
		default:
			//What?  Some strange NxLabel is posting messages to us?
			ASSERT(FALSE);
			break;
		}
	}NxCatchAll("Error in OnLabelClick");
	return 0;
}

void CCPTCodes::PopupAMACopyright()
{
	CString strPopup;
	/*strPopup.Format("CPT © 2003 American Medical Association.  All Rights Reserved.\r\n"
		"No fee schedules, basic units, relative values or related listings are included in CPT.  AMA does not directly or indirectly practice medicine or dispense medical services.  AMA assumes no liability for data contained or not contained herein.\r\n"
		"CPT is a registered trademark of the American Medical Association");*/
	
	//m.hancock - 11/25/2005 - PLID 18389 - update the AMA copyright message to show the year for the AMA code version
	//in AMAVersionHistoryT.  I've also updated the copyright message to reflect the AMA's notice at
	//http://www.ama-assn.org/ama/pub/category/3676.html
	
	//Get the year stored in AMAVersionHistoryT
	//DRT 4/23/2007 - PLID 25598 - Changed so it reads the CPT data file.  If there is no file (this is now an optional download), 
	//	then we will pop up the current year (it's our best guess).  This popup is not related to the download, it's related to
	//	our use of the acronym "AMA" in the interface, so it must continue to popup even if they do not use it.
	long nCurrentYear = GetAMAVersionOfFile(0);
	if(nCurrentYear == 0)
		nCurrentYear = COleDateTime::GetCurrentTime().GetYear();

	CString strYear;
	strYear.Format("%li", nCurrentYear);
	//Display the copyright notice
	strPopup.Format("Current Procedural Terminology (CPT) is copyright %s American Medical Association. All Rights Reserved. \r\n"
		"No fee schedules, basic units, relative values, or related listings are included in CPT. "
		"The AMA assumes no liability for the data contained herein. Applicable FARS/DFARS restrictions apply to government use.",
		strYear);
	MsgBox(strPopup);
}

BOOL CCPTCodes::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	//if they are over our ama label, change to a hand
	CRect rcAMA;
	GetDlgItem(IDC_AMA_TEXT_LABEL)->GetWindowRect(rcAMA);
	ScreenToClient(&rcAMA);

	// (j.jones 2012-01-03 17:41) - PLID 47300 - check the NOC label
	//CRect rcNOC;
	//GetDlgItem(IDC_NOC_LABEL)->GetWindowRect(rcNOC);
	//ScreenToClient(&rcNOC);

	// MSC 5-18-2004 - We also need to make sure the AMA label is being shown, otherwise it shows the hand when
	// the label is not visible
	if (/*rcNOC.PtInRect(pt) ||*/
		(rcAMA.PtInRect(pt) && GetDlgItem(IDC_AMA_TEXT_LABEL)->IsWindowVisible())) {
		SetCursor(GetLinkCursor());
		return TRUE;
	}

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CCPTCodes::OnBtnImportAma() 
{
	try{
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();

		BOOL bAllowCPT = GetCurrentUserPermissions(bioServiceCodes) & SPT____C_______ANDPASS;	
		BOOL bAllowICD9 = GetCurrentUserPermissions(bioDiagCodes) & SPT____C_______ANDPASS;
		BOOL bAllowMod = GetCurrentUserPermissions(bioServiceModifiers) & SPT____C_______ANDPASS;

		mnu.InsertMenu(0, MF_BYPOSITION|(bAllowCPT ? 0 : MF_GRAYED), 1, "Import AMA Service &Codes...");
		mnu.InsertMenu(1, MF_BYPOSITION|(bAllowICD9 ? 0 : MF_GRAYED), 2, "Import AMA &Diagnosis Codes...");
		mnu.InsertMenu(2, MF_BYPOSITION|(bAllowMod ? 0 : MF_GRAYED), 3, "Import AMA Service &Modifiers...");

		long nMenuChoice = 0;

		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_BTN_IMPORT_AMA);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			nMenuChoice = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD , rc.right, rc.top, this, NULL);
			mnu.DestroyMenu();
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			nMenuChoice = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD , pt.x, pt.y, this, NULL);
			mnu.DestroyMenu();
		}

		if(nMenuChoice == 0)
			return;

		switch(nMenuChoice) {
			case 1:
				//service codes
				if(!CheckCurrentUserPermissions(bioServiceCodes, sptCreate))
					return;

				ImportCodes(0);
				break;
			case 2:
				//diagnosis codes
				if(!CheckCurrentUserPermissions(bioDiagCodes, sptCreate))
					return;

				ImportCodes(1);
				break;
			case 3:
				//modifiers
				if(!CheckCurrentUserPermissions(bioServiceModifiers, sptCreate))
					return;

				ImportCodes(2);
				break;
		}	
	}NxCatchAll("Error in OnBtnImportAma");
}

//nType is the type of thing we are importing:
//	0 = CPT Code
//	1 = Diag Code
//	2 = Modifier
void CCPTCodes::ImportCodes(int nType)
{
	if(nType < 0 || nType > 2) {
		MsgBox("Invalid type passed to Importcodes()");
		return;
	}

	try {
		CImportAMACodesDlg dlg(this);
		dlg.SetType(nType);
		if(dlg.DoModal() == IDOK) {
			//import passed - we have to refresh the details

			switch(nType) {
			case 0:	//CPT
			{
				Refresh();	//this will update the tab details, but not the datalist

				//save the selection
				long nServiceID = -1;
				if(m_pCodeNames->GetCurSel() != sriNoRow) {
					nServiceID = m_CurServiceID;
				}

				m_bRequeryFinished = false;
				m_pCodeNames->Requery();

				//reset the selection
				m_CurServiceID = nServiceID;
				m_pCodeNames->TrySetSelByColumn(0, (long)m_CurServiceID);

				// (a.walling 2007-02-23 10:14) - PLID 24745 - Also flag the table checker so it will become available on a bill.
				m_CPTChecker.Refresh();
			}
			break;
			case 1:	//Diag
				m_pDiagCodes->Requery();

				// (a.walling 2007-02-23 10:14) - PLID 24745 - Also flag the table checker so it will become available on a bill.
				m_ICD9Checker.Refresh();
				break;
			case 2:	//Modifier
				m_pModifiers->Requery();

				// (a.walling 2007-02-23 10:14) - PLID 24745 - Also flag the table checker so it will become available on a bill.
				m_ModChecker.Refresh();
				break;
			}
		}
	} NxCatchAll("Error importing AMA codes");
}

void CCPTCodes::OnEditingStartingCptModifiers(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		//they cannot edit the code or description, but they can edit the multiplier
		if(nRow == sriNoRow)
			return;

		if(nCol == 2)
			return;	//allow them to continue

		//DRT 4/24/2008 - PLID 29780 - You may edit AMA descriptions now.
/*		//get the value of the IsAMA
		CString str = VarString(m_pModifiers->GetValue(nRow, 3), "");
		if(!str.IsEmpty()) {	//if it's ama, it'll be "*"
			*pbContinue = FALSE;
			// (j.jones 2007-02-20 10:26) - PLID 23268 - warn the user
			AfxMessageBox("You cannot edit AMA modifiers.");
			return;
		}
*/
	} NxCatchAll("Error starting editing modifier list.");
}

void CCPTCodes::OnEditingStartingDiagCodes(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {
		//doesn't matter what row they are editing ... if this is an AMA code, they are not allowed.
		if(nRow == sriNoRow)
			return;

		//DRT 4/24/2008 - PLID 29780 - You may edit AMA descriptions now.
		// (j.luckoski 2012-10-24 10:11) - PLID 53366 - Lock editing the code when a ICD9 code is AMA or FDB for breaking
		// interaction checks with FDB.

		//get the value of the IsAMA
		//TES 5/14/2013 - PLID 56631 - Converted column numbers to enums
		CString strAMA = VarString(m_pDiagCodes->GetValue(nRow, dclcIsAMA), "");
		// (r.gonet 03/06/2014) - PLID 61129 - Removed ability to import from FDB and related controls

		// (j.jones 2013-11-21 15:23) - PLID 59640 - disallowed editing AMA & FDB codes for ICD-10 flags

		//if it's ama, it'll be "*"
		if(!strAMA.IsEmpty()) {
			// (j.jones 2007-02-20 10:26) - PLID 23268 - warn the user
			if(nCol == dclcCodeNumber) {
				*pbContinue = FALSE;
				AfxMessageBox("You cannot edit AMA diagnosis code numbers.");
				return;
			}
			else if(nCol == dclcIsICD10) {
				*pbContinue = FALSE;
				AfxMessageBox("You cannot edit the ICD-10 status on an AMA diagnosis code.");
				return;
			}
		}

	} NxCatchAll("Error starting editing diag code list.");
}
//s.tullis PLID 61829 move to Anes/Facility setup
/*void CCPTCodes::OnBtnAnesthesiaSetup() 
{
	try{

		// (j.jones 2010-11-19 17:28) - PLID 41544 - this button now has dual-use,
		// in OHIP it can launch the assisting codes setup
		if(/*m_checkAssistingCode.GetCheck() &&*/ /*UseOHIP()) {

			CAssistingCodesSetupDlg dlg(this);
			dlg.DoModal();
		}
		else {
			//regular anesthesia setup

			// (j.jones 2007-10-15 10:39) - PLID 27757 - required a service ID

			if(m_CurServiceID == -1) {
				AfxMessageBox("Please select a service code before editing the anesthesia setup.");
				return;
			}

			CString strCode;
			GetDlgItemText(IDC_CPTCODE, strCode);

			CAnesthesiaSetupDlg dlg(this);
			dlg.m_nServiceID = m_CurServiceID;		
			dlg.m_strServiceCode = strCode;
			dlg.DoModal();
		}

		//see if the standard fee should be grayed out (for Anesth./Facility purposes)
		CheckEnableStandardFee();

		// (j.jones 2010-11-19 17:03) - PLID 41544 - hide assisting code info. if not OHIP, or if Anesthesia is checked
		//CheckDisplayAssistingCodeControls();

		//they may have chosen to update the price from within the dialog, so reload this item	
		Load();

	}NxCatchAll(__FUNCTION__);
}*/

/*void CCPTCodes::OnKillfocusAnesthBaseUnits() 
{
	try{
		if(m_bIsSavingCPT || !m_bEditChanged)
			return;

		m_bIsSavingCPT = TRUE;

		Save();

		m_bIsSavingCPT = FALSE;
	}NxCatchAll("Error saving anesthesia base units");
}*/

//DRT 8/12/2004 - PLID 13856 - Open the interface to update multiple
//	TOS codes.
void CCPTCodes::OnUpdateTos() 
{
	try {
		CUpdateMultipleTOSDlg dlg(this);
		dlg.DoModal();
	} NxCatchAll("Error updating type of service codes.");
}

void CCPTCodes::OnConfigureFinanceChargeSettings() 
{
	try{
		if(!CheckCurrentUserPermissions(bioFinanceCharges,sptWrite))
			return;

		CChargeInterestConfigDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll("Error configuring finance charge settings");
}


void CCPTCodes::SecureControls()
{
	//CPT Codes.
	if(!m_pCodeNames->IsRequerying()) {
		GetDlgItem(IDC_DELETE_CPT)->EnableWindow(GetCurrentUserPermissions(bioServiceCodes) & SPT_____D______ANDPASS);
	}
	else {
		GetDlgItem(IDC_DELETE_CPT)->EnableWindow(FALSE);
	}

	GetDlgItem(IDC_ADD_CPT)->EnableWindow(GetCurrentUserPermissions(bioServiceCodes) & SPT____C_______ANDPASS);
	GetDlgItem(IDC_BTN_IMPORT_CPT)->EnableWindow(GetCurrentUserPermissions(bioServiceCodes) & SPT____C_______ANDPASS);	

	 bCanWrite = GetCurrentUserPermissions(bioServiceCodes) & SPT___W________ANDPASS;

	GetDlgItem(IDC_MARK_INACTIVE)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_CPTCODE)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_CPTSUBCODE)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_DESCRIPTION)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_STD_FEE)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_CPT_TOS)->EnableWindow(bCanWrite);
	// (s.dhole 2012-03-02 18:09) - PLID 47399
	GetDlgItem(IDC_CPT_COST)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_UPDATE_PRICES)->EnableWindow(bCanWrite);
	//GetDlgItem(IDC_CPT_ADDITIONAL_INFO_BTN)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_TAXABLE_1)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_RVU)->EnableWindow(bCanWrite);
	
	GetDlgItem(IDC_TAXABLE_2)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_GLOBAL_PERIOD)->EnableWindow(bCanWrite);
	// (j.gruber 2010-08-03 10:32) - PLID 39944 - remove promptforcopay
	//GetDlgItem(IDC_CHECK_PROMPT_FOR_COPAY)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_CPT_BARCODE)->EnableWindow(bCanWrite);
	//GetDlgItem(IDC_CHECK_SINGLE_REV_CODE)->EnableWindow(bCanWrite);// (s.tullis 2014-05-22 09:09) - PLID 61829 - moved
	//GetDlgItem(IDC_UB92_CPT_CATEGORIES)->EnableWindow(bCanWrite); // (s.tullis 2014-05-22 09:09) - PLID 61829 - moved
	//GetDlgItem(IDC_CHECK_MULTIPLE_REV_CODES)->EnableWindow(bCanWrite);// (s.tullis 2014-05-22 09:09) - PLID 61829 - moved
	//GetDlgItem(IDC_BTN_EDIT_MULTIPLE_REV_CODES)->EnableWindow(bCanWrite);// (s.tullis 2014-05-22 09:09) - PLID 61829 - moved
	GetDlgItem(IDC_SHOP_FEE)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_DEFAULT_CPT_PROVIDER)->EnableWindow(bCanWrite);
	//GetDlgItem(IDC_CHECK_ANESTHESIA)->EnableWindow(bCanWrite);// (s.tullis 2014-05-22 09:09) - PLID 61829 - moved
	//GetDlgItem(IDC_ANESTH_BASE_UNITS)->EnableWindow(bCanWrite);// (s.tullis 2014-05-22 09:09) - PLID 61829 - moved
	//GetDlgItem(IDC_BTN_ANESTHESIA_SETUP)->EnableWindow(bCanWrite);// (s.tullis 2014-05-22 09:09) - PLID 61829 - moved
	GetDlgItem(IDC_BTN_SELECT_CATEGORY)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_BTN_REMOVE_CATEGORY)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_UPDATE_CATEGORIES)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_UPDATE_TOS)->EnableWindow(bCanWrite);
	//GetDlgItem(IDC_ADV_REVCODE_SETUP)->EnableWindow(bCanWrite);// (s.tullis 2014-05-22 09:09) - PLID 61829 - moved
	// (j.jones 2010-07-30 10:45) - PLID 39728 - enable/disable pay group buttons
	GetDlgItem(IDC_BTN_EDIT_PAY_GROUPS)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_PAY_GROUPS_COMBO)->EnableWindow(bCanWrite);
	// (j.jones 2010-08-02 16:49) - PLID 39912 - enable/disable the adv. pay group config
	GetDlgItem(IDC_BTN_ADV_PAY_GROUP_SETUP)->EnableWindow(bCanWrite);
	// (j.jones 2010-11-19 16:27) - PLID 41544 - supported assisting codes
	//GetDlgItem(IDC_CHECK_ASSISTING_CODE)->EnableWindow(bCanWrite);// (s.tullis 2014-05-22 09:09) - PLID 61829 - moved
	//GetDlgItem(IDC_ASSISTING_BASE_UNITS)->EnableWindow(bCanWrite);// (s.tullis 2014-05-22 09:09) - PLID 61829 - moved
	// (j.jones 2011-03-28 14:27) - PLID 43012 - disable the Billable options
	GetDlgItem(IDC_CPT_BILLABLE)->EnableWindow(bCanWrite);
	GetDlgItem(IDC_BTN_EDIT_BILLABLE_CODES)->EnableWindow(bCanWrite);
	// (j.jones 2011-07-06 16:34) - PLID 44358 - added UB Box 4
	//GetDlgItem(IDC_UB_BOX4)->EnableWindow(bCanWrite);// (s.tullis 2014-05-22 09:09) - PLID 61829 - moved

	//CPT Modifiers
	GetDlgItem(IDC_DELETE_MODIFIER)->EnableWindow(GetCurrentUserPermissions(bioServiceModifiers) & SPT_____D______ANDPASS);
	GetDlgItem(IDC_ADD_MODIFIER)->EnableWindow(GetCurrentUserPermissions(bioServiceModifiers) & SPT____C_______ANDPASS);	

	bCanWrite = GetCurrentUserPermissions(bioServiceModifiers) & SPT___W________ANDPASS;
	m_pModifiers->PutReadOnly(bCanWrite?VARIANT_FALSE:VARIANT_TRUE);
	// (z.manning, 05/01/2007) - PLID 16623 - Disable the inactivate button if not write permission.
	GetDlgItem(IDC_MARK_MODIFIER_INACTIVE)->EnableWindow(bCanWrite);

	//Diagnosis Codes
	GetDlgItem(IDC_DELETE_ICD9)->EnableWindow(GetCurrentUserPermissions(bioDiagCodes) & SPT_____D______ANDPASS);
	GetDlgItem(IDC_ADD_ICD9)->EnableWindow(GetCurrentUserPermissions(bioDiagCodes) & SPT____C_______ANDPASS);	

	bCanWrite = GetCurrentUserPermissions(bioDiagCodes) & SPT___W________ANDPASS;
	m_pDiagCodes->PutReadOnly(bCanWrite?VARIANT_FALSE:VARIANT_TRUE);
	GetDlgItem(IDC_MARK_ICD9_INACTIVE)->EnableWindow(bCanWrite);

	// (d.singleton 2011-09-21 16:16) - PLID 44946
	if(UseAlbertaHLINK())
	{
		//make everything else read only if we are dealing with Alberta Billing
		GetDlgItem(IDC_ADD_MODIFIER)->EnableWindow(FALSE);
		GetDlgItem(IDC_MARK_MODIFIER_INACTIVE)->EnableWindow(FALSE);
		GetDlgItem(IDC_DELETE_MODIFIER)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_SERVICE_MODIFIER_LINKING)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_LAUNCH_CODELINK)->EnableWindow(FALSE);
	}
	// (r.gonet 03/06/2014) - PLID 61129 - Removed ability to import from FDB and related controls
}

/*void CCPTCodes::OnCheckFacility() //PLID 61829 move to Anesthesia/Facility Setup
{
	if(m_CurServiceID == -1)
		return;

	try {

		//only one can be toggled at a time
		if(m_checkFacility.GetCheck() && m_checkAnesthesia.GetCheck()) {
			m_checkAnesthesia.SetCheck(FALSE);
			m_checkUseAnesthesiaBilling.SetCheck(FALSE);
		}
		// (j.jones 2010-11-19 16:50) - PLID 41544 - uncheck assisting code
		if(m_checkFacility.GetCheck() && m_checkAssistingCode.GetCheck()) {
			m_checkAssistingCode.SetCheck(FALSE);
		}

		if(!m_checkFacility.GetCheck()) {
			m_checkUseFacilityBilling.SetCheck(FALSE);
		}

		// (j.jones 2010-11-19 16:38) - PLID 41544 - renamed to include assisting codes
		CheckEnableAnesthesiaFacilityAssistingControls();

		Save();

		//see if the standard fee should be grayed out (for Anesth./Facility purposes)
		CheckEnableStandardFee();

		// (j.jones 2010-11-19 17:03) - PLID 41544 - hide assisting code info. if not OHIP, or if Anesthesia is checked
		CheckDisplayAssistingCodeControls();

	}NxCatchAll("Error updating Facility Fee status.");
}

/*void CCPTCodes::OnBtnFacilityFeeSetup() //PLID 61829 move to Anesthesia/Facility Setup
{
	try{

		// (j.jones 2007-10-15 10:39) - PLID 27757 - required a service ID

		if(m_CurServiceID == -1) {
			AfxMessageBox("Please select a service code before editing the facility fee setup.");
			return;
		}

		CString strCode;
		GetDlgItemText(IDC_CPTCODE, strCode);

		CFacilityFeeConfigDlg dlg(this);
		dlg.m_nServiceID = m_CurServiceID;
		dlg.m_strServiceCode = strCode;
		dlg.DoModal();

		//see if the standard fee should be grayed out (for Anesth./Facility purposes)
		CheckEnableStandardFee();

		// (j.jones 2010-11-19 17:03) - PLID 41544 - hide assisting code info. if not OHIP, or if Anesthesia is checked
		CheckDisplayAssistingCodeControls();

	}NxCatchAll("Error in facility fee setup");
}
*/
void CCPTCodes::OnProcedureDescription() 
{
	try {

		if(m_CurServiceID == -1) {
			return;
		}

		// (j.jones 2012-04-11 16:17) - PLID 47313 - this is now a pop-out menu
		// (j.dinatale 2012-06-12 13:14) - PLID 32795 - added a menu option for the default NDC info
		enum MenuOptions {
			moProcedureDescription = 1,
			moClaimNote,
			moNDCDefInfo,
		};

		// (j.jones 2012-04-11 16:21) - PLID 47313 - If a claim note is in use, check off the menu option.
		// And in turn, might as well do it for procedure description too, for consistency.
		// The places that load the procedure description compare to empty or NULL, but ClaimNote is trimmed.
		BOOL bClaimNoteInUse = FALSE, bProcedureDescInUse = FALSE;
		_RecordsetPtr rs = CreateParamRecordset("SELECT Convert(bit, CASE WHEN LTRIM(RTRIM(ClaimNote)) = '' THEN 0 ELSE 1 END) AS ClaimNoteInUse, "
			"Convert(bit, CASE WHEN Convert(nvarchar, ServiceT.ProcedureDescription) = '' OR ServiceT.ProcedureDescription IS NULL THEN 0 ELSE 1 END) AS ProcedureDescInUse "
			"FROM ServiceT WHERE ID = {INT}", m_CurServiceID);
		if(!rs->eof) {
			bClaimNoteInUse = VarBool(rs->Fields->Item["ClaimNoteInUse"]->Value);
			bProcedureDescInUse = VarBool(rs->Fields->Item["ProcedureDescInUse"]->Value);
		}
		rs->Close();

		// (j.dinatale 2012-06-12 13:14) - PLID 32795 - added a menu option for the default NDC info
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		//mnu.InsertMenu(0, MF_BYPOSITION|(bClaimNoteInUse ? MF_CHECKED : 0), moClaimNote, "Edit Claim &Note..."); //PLID 61829 moved to claim setup dialog
		mnu.InsertMenu(1, MF_BYPOSITION|(bProcedureDescInUse ? MF_CHECKED : 0), moProcedureDescription, "Edit &Procedure Description for Quotes...");
		//mnu.InsertMenu(2, MF_BYPOSITION, moNDCDefInfo, "NDC &Defaults...");//PLID 61829 moved to claim setup dialog

		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_PROCEDURE_DESCRIPTION);
		CPoint pt;
		GetCursorPos(&pt);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			pt.SetPoint(rc.right, rc.top);
		}
		
		long nMenuChoice = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD , pt.x, pt.y, this, NULL);
		mnu.DestroyMenu();

		if(nMenuChoice == moProcedureDescription) {
			CProcedureDescriptionDlg dlg(m_CurServiceID, this);
			dlg.DoModal();
		}
		// (j.jones 2012-04-11 16:17) - PLID 47313 - added claim notes
		else if(nMenuChoice == moClaimNote) {
			EnterClaimNote();	
		}
		// (j.dinatale 2012-06-12 13:14) - PLID 32795 - added a menu option for the default NDC info
		else if(nMenuChoice == moNDCDefInfo){ 
			CNDCDefSetupDlg dlg(m_CurServiceID, false, this);
			if(dlg.DoModal() == IDOK){
				m_CPTChecker.Refresh(m_CurServiceID);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

LRESULT CCPTCodes::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {

		// (j.jones 2010-07-08 14:06) - PLID 39566 - There is no reason for
		// this tab to be immediately responding to tablecheckers. It should
		// only respond to them in the Refresh() function.

		
		switch(wParam) {
			case NetUtils::CPTCodeT:
			case NetUtils::CPTModifierT:
			case NetUtils::DiagCodes:
			case NetUtils::CPTCategories:
			case NetUtils::UB92Cats:
			case NetUtils::Providers: {
				try {
					UpdateView();
				} NxCatchAll("Error in CCPTCodes::OnTableChanged:Generic");
				break;
			}
		}
		

	} NxCatchAll("Error in CCPTCodes::OnTableChanged");

	return 0;
}

void CCPTCodes::CheckEnableStandardFee() {// (s.tullis 2014-05-20 17:35) - PLID 61829 - This only relies on if billiable is checked now since all the rest the checks are now in diffent dialogs

	//don't allow them to edit the standard fee when an anesthesia code
	//and when "Enable Anesthesia Billing" is enabled in the Ansesthesia Setup,
	//or the same for Facility Codes
	// (j.jones 2010-11-19 16:37) - PLID 41544 - also disabled if OHIP and Assisting Codes are checked
	// (j.jones 2011-03-28 09:21) - PLID 43012 - also disabled if not billable
	if(/*m_checkAnesthesia.GetCheck() && m_checkUseAnesthesiaBilling.GetCheck()) ||
		(m_checkFacility.GetCheck() && m_checkUseFacilityBilling.GetCheck()) ||*/
		/*(m_checkAssistingCode.GetCheck() && UseOHIP())
		||*/ !m_checkBillable.GetCheck()) {
	
		((CNxEdit*)GetDlgItem(IDC_STD_FEE))->SetReadOnly(TRUE);
		GetDlgItem(IDC_UPDATE_PRICES)->EnableWindow(FALSE && (GetCurrentUserPermissions(bioServiceCodes) & SPT___W________ANDPASS));
	}
	else {
		((CNxEdit*)GetDlgItem(IDC_STD_FEE))->SetReadOnly(FALSE);
		GetDlgItem(IDC_UPDATE_PRICES)->EnableWindow(GetCurrentUserPermissions(bioServiceCodes) & SPT___W________ANDPASS);
	}
}

/*void CCPTCodes::OnCheckUseAnesthesiaBilling() //PLID 61829 move to Anesthesia/Facility Setup
{
	if(m_CurServiceID == -1)
		return;

	try {

		// (j.jones 2010-11-19 16:38) - PLID 41544 - renamed to include assisting codes
		CheckEnableAnesthesiaFacilityAssistingControls();

		Save();

		//see if the standard fee should be grayed out (for Anesth./Facility purposes)
		CheckEnableStandardFee();

		// (j.jones 2010-11-19 17:03) - PLID 41544 - hide assisting code info. if not OHIP, or if Anesthesia is checked
		CheckDisplayAssistingCodeControls();

	}NxCatchAll("Error updating Anesthesia billing status.");
}

void CCPTCodes::OnCheckUseFacilityBilling() // //PLID 61829 move to Anesthesia/Facility Setup
{
	if(m_CurServiceID == -1)
		return;

	try {

		// (j.jones 2010-11-19 16:38) - PLID 41544 - renamed to include assisting codes
		CheckEnableAnesthesiaFacilityAssistingControls();

		Save();

		//see if the standard fee should be grayed out (for Anesth./Facility purposes)
		CheckEnableStandardFee();

		// (j.jones 2010-11-19 17:03) - PLID 41544 - hide assisting code info. if not OHIP, or if Anesthesia is checked
		CheckDisplayAssistingCodeControls();

	}NxCatchAll("Error updating Facility billing status.");
}
*/
void CCPTCodes::OnTrySetSelFinishedCptCodes(long nRowEnum, long nFlags) 
{
	try{
		//Double-check the enabling of the arrows
		if(m_pCodeNames->CurSel < 1) {
			m_btnCptLeft.EnableWindow(FALSE);
		}
		else {
			m_btnCptLeft.EnableWindow(TRUE);
		}
		if(m_pCodeNames->CurSel < m_pCodeNames->GetRowCount()-1 && m_pCodeNames->CurSel != -1) {
			m_btnCptRight.EnableWindow(TRUE);
		}
		else {
			m_btnCptRight.EnableWindow(FALSE);
		}

		GetDlgItem(IDC_DELETE_CPT)->EnableWindow(GetCurrentUserPermissions(bioServiceCodes) & SPT_____D______ANDPASS);
	}NxCatchAll("Error in OnTrySetSelFinishedCptCodes");
}

// (a.wetta 2007-05-16 09:47) - PLID 25960 - No longer on this tab
/*void CCPTCodes::OnBtnSuggestedSales() 
{
	try {
		ASSERT(m_CurServiceID != -1);

		CSuggestedSalesDlg dlg;

		dlg.m_nServiceID = m_CurServiceID;
		
		GetDlgItemText(IDC_DESCRIPTION, dlg.m_strDescription);

		dlg.DoModal();
	} NxCatchAll("Error in OnBtnSuggestedSales");
}*/

// (z.manning, 05/01/2007) - PLID 16623 - Inactivated the currently selected modifier.
void CCPTCodes::OnMarkModifierInactive()
{
	try {

		if(m_pModifiers->CurSel == -1) {
			MessageBox("Please select a modifier before marking inactive.");
			return;
		}

		if(IDYES != MessageBox("Deactivating a modifier will make it unavailable on this screen, as well as on a bill. However, you can still view it in reports."
			"\nYou can re-activate this modifier by clicking 'Inactive Codes' at the bottom of this screen.'"
			"\n\nAre you SURE you wish to deactivate this modifier?", "Deactivate Modifier", MB_YESNO|MB_ICONEXCLAMATION))
		{
			return;
		}

		CString strModifierID = VarString(m_pModifiers->GetValue(m_pModifiers->CurSel, 0));

		ExecuteSql("UPDATE CptModifierT SET Active = 0 WHERE Number = '%s'", _Q(strModifierID));
		m_pModifiers->RemoveRow(m_pModifiers->CurSel);
		m_ModChecker.Refresh();

	}NxCatchAll("CCPTCodes::OnMarkModifierInactive");
}

// (z.manning, 05/02/2007) - PLID 16623 - The inactive cpt code and diag code dialogs used to be accessed
// by their own, separate buttons.  Now we have just one button for all inactive codes, which now includes modifiers.
void CCPTCodes::OnInactiveCodes()
{
	try {

		enum
		{
			miInactiveCptCodes = 1,
			miInactiveDiagCodes,
			miInactiveModifiers,
		};

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();

		// (z.manning, 05/02/2007) - PLID 16623 - Make sure they have write permission before we let them try to reactivate.
		BOOL bAllowCPT = GetCurrentUserPermissions(bioServiceCodes) & SPT___W________ANDPASS;
		BOOL bAllowICD9 = GetCurrentUserPermissions(bioDiagCodes) & SPT___W________ANDPASS;
		BOOL bAllowMod = GetCurrentUserPermissions(bioServiceModifiers) & SPT___W________ANDPASS;

		mnu.InsertMenu(0, MF_BYPOSITION|(bAllowCPT ? 0 : MF_GRAYED), miInactiveCptCodes, "Inactive &Service Codes...");
		mnu.InsertMenu(1, MF_BYPOSITION|(bAllowICD9 ? 0 : MF_GRAYED), miInactiveDiagCodes, "Inactive &Diagnosis Codes...");
		mnu.InsertMenu(2, MF_BYPOSITION|(bAllowMod ? 0 : MF_GRAYED), miInactiveModifiers, "Inactive &Modifiers...");

		long nMenuChoice = 0;

		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_INACTIVE_CODES);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			nMenuChoice = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD , rc.right, rc.top, this, NULL);
			mnu.DestroyMenu();
		} 
		else {
			CPoint pt;
			GetCursorPos(&pt);
			nMenuChoice = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD , pt.x, pt.y, this, NULL);
			mnu.DestroyMenu();
		}

		if(nMenuChoice == 0) {
			return;
		}

		switch(nMenuChoice) 
		{
			// (z.manning, 05/03/2007) - PLID 16623 - We used to refresh the dialog if we detected that anything
			// changed in one of these dialogs. However, the all send table checkers when something changes, so
			// refreshing was redundant.
			case miInactiveCptCodes:
			{
				CInactiveServiceItems dlg(this);
				dlg.m_ServiceType = 1;
				dlg.DoModal();

				// (j.dinatale 2011-06-17 14:57) - PLID 43462 - need to requery the service code list afterwards
				m_bRequeryFinished = false;
				m_pCodeNames->Requery();
			}
			break;

			case miInactiveDiagCodes:
			{
				CInactiveICD9sDlg dlg(this);
				dlg.DoModal();

				// (j.dinatale 2011-06-17 14:57) - PLID 43462 - need to requery the diag code list afterwards
				m_pDiagCodes->Requery();
			}
			break;

			case miInactiveModifiers:
			{
				CInactiveModifiersDlg dlg(this);
				dlg.DoModal();

				// (j.dinatale 2011-06-17 14:57) - PLID 43462 - need to requery the modifiers list afterwards
				m_pModifiers->Requery();
			}
			break;
		}

	}NxCatchAll("CCPTCodes::OnInactiveCodes");
}

// (a.wetta 2007-05-16 09:53) - PLID 25163 - Added discount categories dialog
void CCPTCodes::OnBtnDiscountCategories() 
{
	try {
		CDiscountsSetupDlg dlg(this);

		dlg.DoModal();

	} NxCatchAll("Error in CCPTCodes::OnBtnDiscountCategories");	
}

/*void CCPTCodes::OnBtnSetupIcd9v3() //s.tullis PLID 61829 move to UB Setup
{
	// (a.walling 2007-06-20 11:59) - PLID 26413 - setup the icd9-cm procedure codes and linking
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(m_ICD9V3List->GetCurSel());

		CICD9ProcedureSetupDlg dlg(this);
		dlg.DoModal();

		// (a.walling 2007-06-21 10:13) - PLID 26413 - Our code could have been changed
		_RecordsetPtr prs = CreateRecordset("SELECT ICD9ProcedureCodeID FROM CPTCodeT WHERE ID = %li", m_CurServiceID);

		long nID = -1;

		if (!prs->eof) {
			nID = AdoFldLong(prs, "ICD9ProcedureCodeID", -1);
		}

		m_ICD9V3List->Requery();
		NXDATALIST2Lib::IRowSettingsPtr pNoRow = m_ICD9V3List->GetNewRow();
		if (pNoRow) {
			pNoRow->PutValue(0, _variant_t(long(-1)));
			pNoRow->PutValue(1, _variant_t(""));
			pNoRow->PutValue(2, _variant_t("<None>"));

			m_ICD9V3List->AddRowSorted(pNoRow, NULL);
		}

		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
		m_ICD9V3List->TrySetSelByColumn_Deprecated(0, _variant_t(nID));

	} NxCatchAll("Error setting up ICD9 Procedure codes");
}*/

/*void CCPTCodes::OnSelChangingListIcd9v3(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) //s.tullis PLID 61829 move to UB Setup
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}	
	} NxCatchAll("Error in OnSelChangingListIcd9v3");
}*///s.tullis PLID 61829 move to UB Setup
// (s.tullis 2014-05-22 09:13) - PLID 61829 - moved
/*void CCPTCodes::OnSelChangedListIcd9v3(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) //s.tullis Remove later
{
	// (a.walling 2007-06-20 12:02) - PLID 26413 - Update the ICD9v3 link
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

		if (pRow) {
			long nID = VarLong(pRow->GetValue(0));

			CString strNewID;

			if (nID != -1) {
				strNewID = FormatString("%li", nID);
			} else {
				strNewID = "NULL";
			}

			ExecuteSql("UPDATE CPTCodeT SET ICD9ProcedureCodeID = %s WHERE ID = %li", strNewID, m_CurServiceID);
		}
	} NxCatchAll("Error in CCPTCodes::OnSelChangedListIcd9v3");	
}*/

// (j.jones 2007-07-03 15:05) - PLID 26098 - added option to link service codes and modifiers
void CCPTCodes::OnBtnServiceModifierLinking() 
{
	try {

		CMultiServiceModifierLinkDlg dlg(this);
		dlg.DoModal();
		
	}NxCatchAll("Error in CCPTCodes::OnBtnServiceModifierLinking");
}

// (j.jones 2007-10-15 10:41) - PLID 27757 - unified this code into one function
// (j.jones 2010-11-19 16:38) - PLID 41544 - renamed to include assisting codes

/*void CCPTCodes::CheckEnableAnesthesiaFacilityAssistingControls()//s.tullis PLID 61829 move to Anesthesia/ Facility Setup
{
	try {

		// (j.jones 2010-11-19 16:41) - PLID 41544 - streamlined this code, and supported assisting codes
		// (assisting codes reuse the anesthesia setup button
		BOOL bHasPermission = (GetCurrentUserPermissions(bioServiceCodes) & SPT___W________ANDPASS);
		BOOL bAnesthesiaIsChecked = FALSE;
		BOOL bFacilityIsChecked = FALSE;
		BOOL bAssistingCodeIsChecked = FALSE;

		//these are mutually exclusive
		if(m_checkAnesthesia.GetCheck()) {
			bAnesthesiaIsChecked = TRUE;
		}
		else if(m_checkFacility.GetCheck()) {
			bFacilityIsChecked = TRUE;
		}
		else if(UseOHIP() && m_checkAssistingCode.GetCheck()) {
			bAssistingCodeIsChecked = TRUE;
		}

		//now update each field
		GetDlgItem(IDC_CHECK_USE_ANESTHESIA_BILLING)->EnableWindow(bHasPermission && bAnesthesiaIsChecked);
		GetDlgItem(IDC_CHECK_USE_FACILITY_BILLING)->EnableWindow(bHasPermission && bFacilityIsChecked);

		GetDlgItem(IDC_ANESTH_BASE_UNITS)->EnableWindow(bHasPermission && bAnesthesiaIsChecked);
		GetDlgItem(IDC_ASSISTING_BASE_UNITS)->EnableWindow(bHasPermission && bAssistingCodeIsChecked);
				
		GetDlgItem(IDC_BTN_ANESTHESIA_SETUP)->EnableWindow(bHasPermission && (bAnesthesiaIsChecked || bAssistingCodeIsChecked));
		GetDlgItem(IDC_BTN_FACILITY_FEE_SETUP)->EnableWindow(bHasPermission && bFacilityIsChecked);

	}NxCatchAll(__FUNCTION__);
}*/
/*
// (j.jones 2009-03-30 16:56) - PLID 32324 - added OnBtnOhipPremiumCodes
void CCPTCodes::OnBtnOhipPremiumCodes()// s.tullis
{
	try {

		// (j.jones 2010-07-19 11:39) - PLID 39566 - use the cached value
		if(!m_bUseOHIP) {
			//how is this button even accessible?
			ASSERT(FALSE);
			return;
		}

		COHIPPremiumCodesDlg dlg(this);
		dlg.m_nDefaultServiceID = m_CurServiceID;
		dlg.DoModal();

	}NxCatchAll("Error in CCPTCodes::OnBtnOhipPremiumCodes");
}*/

// (j.jones 2010-07-30 10:16) - PLID 39728 - added support for pay groups
void CCPTCodes::OnBtnEditPayGroups()
{
	try {

		//store our current selection
		long nCurPayGroupID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_PayGroupCombo->GetCurSel();
		if(pRow) {
			nCurPayGroupID = VarLong(pRow->GetValue(pgccID),-1);
		}

		CPayGroupsEditDlg dlg(this);
		if(dlg.DoModal() == IDOK) {

			m_PayGroupCombo->Requery();

			NXDATALIST2Lib::IRowSettingsPtr pNoPayGroup = m_PayGroupCombo->GetNewRow();
			pNoPayGroup->PutValue(pgccID, (long)-1);
			pNoPayGroup->PutValue(pgccName, _bstr_t(" <No Default Pay Group>"));
			m_PayGroupCombo->AddRowSorted(pNoPayGroup, NULL);

			//retain our current selection, if it still exists
			NXDATALIST2Lib::IRowSettingsPtr pPayGroupRow = m_PayGroupCombo->SetSelByColumn(pgccID, (long)nCurPayGroupID);
			if(pPayGroupRow == NULL) {
				//select the -1 row
				m_PayGroupCombo->SetSelByColumn(pgccID, (long)-1);
			}

			//this will clear the changed status for our tablechecker,
			//so we won't have to respond to it on the next refresh,
			//as we just handled the change right now
			m_PayGroupChecker.Changed();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-07-30 10:16) - PLID 39728 - added support for pay groups
void CCPTCodes::OnSelChosenPayGroupsCombo(LPDISPATCH lpRow)
{
	try {

		if(m_CurServiceID == -1) {
			return;
		}

		long nPayGroupID = -1;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			nPayGroupID = VarLong(pRow->GetValue(pgccID),-1);			
		}

		_variant_t varPayGroupID = g_cvarNull;
		if(nPayGroupID != -1) {
			varPayGroupID = (long)nPayGroupID;
		}

		//save the setting
		ExecuteParamSql("UPDATE ServiceT SET PayGroupID = {VT_I4} WHERE ID = {INT}", varPayGroupID, m_CurServiceID);

		// (j.jones 2012-07-24 08:57) - PLID 51651 - Added a preference to only track global periods for
		// surgical codes only, this will control whether the edit box is enabled for all codes or not.
		// We would still load the content even if was disabled.
		ReflectGlobalPeriodReadOnly();

		// (j.jones 2012-07-23 14:30) - PLID 51698 - send a tablechecker
		m_CPTChecker.Refresh(m_CurServiceID);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-08-02 14:32) - PLID 39912 - added ability to edit adv. pay group settings
void CCPTCodes::OnBtnAdvPayGroupSetup()
{
	try {

		CAdvServiceCodePayGroupSetupDlg dlg(this);
		dlg.DoModal();

		//the pay group list may have changed, and our current CPT code may have been affected,
		//so reload the entire screen now and let tablecheckers do the work for the pay group list
		Refresh();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-09-29 15:08) - PLID 40686 - added ability to batch even with zero
/*void CCPTCodes::OnCheckBatchWhenZero()
{
	try {

		DontShowMeAgain(this, "Changing the setting to 'include on claims even with a zero charge amount' will only "
			"take effect on new charges for this service code.\n"
			"Existing bills containing this service code may need the charge's batch status to be "
			"manually changed in order to submit on claims.", "ServiceCodeBatchWhenZero");

		Save();

	}NxCatchAll(__FUNCTION__);
}*/

// (j.jones 2010-11-19 16:34) - PLID 41544 - supported assisting codes s.tullis
/*void CCPTCodes::OnCheckAssistingCode()
{
	if(m_CurServiceID == -1) {
		return;
	}

	try {
		
		if(m_checkAssistingCode.GetCheck() && m_checkAnesthesia.GetCheck()) {
			m_checkAnesthesia.SetCheck(FALSE);
			m_checkUseAnesthesiaBilling.SetCheck(FALSE);
		}
		if(m_checkAssistingCode.GetCheck() && m_checkFacility.GetCheck()) {
			m_checkFacility.SetCheck(FALSE);
			m_checkUseFacilityBilling.SetCheck(FALSE);
		}

		// (j.jones 2007-10-15 10:41) - PLID 27757 - unified this code into one function
		// (j.jones 2010-11-19 16:38) - PLID 41544 - renamed to include assisting codes
		CheckEnableAnesthesiaFacilityAssistingControls();

		Save();

		//see if the standard fee should be grayed out
		CheckEnableStandardFee();

		// (j.jones 2010-11-19 17:03) - PLID 41544 - hide assisting code info. if not OHIP, or if Anesthesia is checked
		CheckDisplayAssistingCodeControls();

	}NxCatchAll(__FUNCTION__);
}*/
/*
void CCPTCodes::OnKillfocusAssistingBaseUnits()
{
	try {

		if(m_bIsSavingCPT || !m_bEditChanged) {
			return;
		}

		m_bIsSavingCPT = TRUE;

		Save();

		m_bIsSavingCPT = FALSE;

	}NxCatchAll(__FUNCTION__);
}
*/
// (j.jones 2010-11-19 17:02) - PLID 41544 - added hides assisting code info. if not OHIP
// or if Anesthesia is checked
/*
void CCPTCodes::CheckDisplayAssistingCodeControls()
{
	try {

		//these should only be visible if OHIP, and anesthesia is NOT enabled
		if(UseOHIP() /*&& !m_checkAnesthesia.GetCheck()) {
			GetDlgItem(IDC_CHECK_ASSISTING_CODE)->ShowWindow(SW_SHOWNA);
			GetDlgItem(IDC_ASSISTING_BASE_UNITS)->ShowWindow(SW_SHOWNA);
			//GetDlgItem(IDC_CHECK_USE_ANESTHESIA_BILLING)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_ANESTH_BASE_UNITS)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_BTN_ANESTHESIA_SETUP)->SetWindowText("Assisting Codes Setup...");
		}
		else {
			GetDlgItem(IDC_CHECK_ASSISTING_CODE)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_ASSISTING_BASE_UNITS)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_CHECK_USE_ANESTHESIA_BILLING)->ShowWindow(SW_SHOWNA);
			//GetDlgItem(IDC_ANESTH_BASE_UNITS)->ShowWindow(SW_SHOWNA);
			//GetDlgItem(IDC_BTN_ANESTHESIA_SETUP)->SetWindowText("Anesthesia Setup...");
		}

	}NxCatchAll(__FUNCTION__);
}
*/

// (j.jones 2011-03-28 09:18) - PLID 43012 - added Billable checkbox
void CCPTCodes::OnCPTBillable()
{
	try {

		if(m_CurServiceID == -1) {
			AfxMessageBox("Please select a service code before deleting.");
			return;
		}

		long nServiceID = m_CurServiceID;

		//if we are disabling the billable status, check whether
		//we can permit such a thing
		if(!m_checkBillable.GetCheck()) {
			
			//disallow if the code exists in a surgery
			if(ReturnsRecordsParam("SELECT TOP 1 ID FROM SurgeryDetailsT WHERE ServiceID = {INT}", nServiceID)) {
				MessageBox("This Service Code exists in at least one Surgery. Please remove it from all Surgeries before removing the billable status.","Practice",MB_OK|MB_ICONINFORMATION);
				m_checkBillable.SetCheck(TRUE);
				return;
			}

			//disallow if the code exists in a preference card
			if(ReturnsRecordsParam("SELECT TOP 1 ID FROM PreferenceCardDetailsT WHERE ServiceID = {INT}", nServiceID)) {
				MessageBox("This Service Code exists in at least one Preference Card. Please remove it from all Preference Cards before removing the billable status.","Practice",MB_OK|MB_ICONINFORMATION);
				m_checkBillable.SetCheck(TRUE);
				return;
			}
			
			//disallow if the code is linked to a product
			if(ReturnsRecordsParam("SELECT TOP 1 CPTID FROM ServiceToProductLinkT WHERE CPTID = {INT}", nServiceID)) {
				MessageBox("This Service Code has inventory items linked to it, and cannot have its billable status removed.","Practice",MB_OK|MB_ICONINFORMATION);
				m_checkBillable.SetCheck(TRUE);
				return;
			}
			
			//disallow if the code is linked to a procedure
			_RecordsetPtr rs = CreateParamRecordset("Select ProcedureT.Name AS Name FROM ProcedureT Left Join ServiceT ON ProcedureT.ID = ServiceT.ProcedureID WHERE ServiceT.ID = {INT}", nServiceID);
			if(!rs->eof){			
				CString strMessage, strProcedure = AdoFldString(rs->GetFields(), "Name", "");
				strMessage.Format("This Service Code exists in the procedure %s. Please remove it from this procedure before removing the billable status.", strProcedure);
				MessageBox(strMessage,"Practice",MB_OK|MB_ICONINFORMATION);
				m_checkBillable.SetCheck(TRUE);
				return;
			}
			rs->Close();
			
			//warn if the code is in any previous quote or previous case history (even if they have been billed)
			if(ReturnsRecordsParam("SELECT TOP 1 LineItemT.ID "
				"FROM LineItemT "
				"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
				"WHERE Deleted = 0 AND Type = 11 AND ServiceID = {INT}", nServiceID)) {

				if(IDNO == MessageBox("This Service Code exists in patients' quotes. If you remove its billable status, billing these quotes will skip adding this service.\n\n"
					"Are you sure you wish to mark this code as not billable?",
					"Practice", MB_YESNO|MB_ICONINFORMATION)) {
					
					m_checkBillable.SetCheck(TRUE);
					return;
				}
			}
			
			//CaseHistoryDetailsT
			if(ReturnsRecordsParam("SELECT TOP 1 ID FROM CaseHistoryDetailsT WHERE ItemType = -2 AND ItemID = {INT}", nServiceID)) {

				if(IDNO == MessageBox("This Service Code exists in patients' case histories. If you remove its billable status, billing these case histories will skip adding this service.\n\n"
					"Are you sure you wish to mark this code as not billable?",
					"Practice", MB_YESNO|MB_ICONINFORMATION)) {
					
					m_checkBillable.SetCheck(TRUE);
					return;
				}
			}
		}

		Save();

		//this will disable the standard fee if the current code is not billable
		CheckEnableStandardFee();

	}NxCatchAll(__FUNCTION__);
}

void CCPTCodes::OnBtnEditBillableCodes()
{
	try {

		if(!CheckCurrentUserPermissions(bioServiceCodes, sptWrite)) {
			return;
		}

		CBillableCPTCodesDlg dlg(this);
		if(dlg.DoModal() == IDOK) {

			//reload the current code, which will update
			//the billable checkbox and the read-only state
			//of the standard fee
			Load();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-07-06 15:57) - PLID 44358 - added UB Box 4
/*
void CCPTCodes::OnEnKillfocusUbBox4()
{
	try {

		if(m_bIsSavingCPT || !m_bEditChanged)
			return;

		m_bIsSavingCPT = TRUE;

		Save();

		m_bIsSavingCPT = FALSE;

	}NxCatchAll(__FUNCTION__);
}
*/
// (d.singleton 2011-12-05 11:03) - PLID 44947 since we do not parse mod descriptions into database the user can fill in their own,  check input and save to db accordingly
void CCPTCodes::EditingFinishedAlbertaModifiers(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		CString strMod = VarString(m_pAlbertaModifiers->GetValue(nRow, amMod));
		//trim to remove any bad spaces
		CString strNewVal = AsString(varNewValue);
		CString strOldVal = AsString(varOldValue);

		//if the input is the same ( case sensitive!! ) we do not save to db
		if(strOldVal.Compare(strNewVal) != 0 && bCommit)
		{
			ExecuteParamSql("Update CptModifierT SET Note = {STRING} WHERE Number = {STRING}", strNewVal, strMod);
		}
	
		CClient::RefreshTable(NetUtils::CPTModifierT, -1);

		Refresh();

	}NxCatchAll(__FUNCTION__);
}
/*
// (j.jones 2012-01-03 15:36) - PLID 47300 - added NOC Combo
void CCPTCodes::OnSelChosenNocCombo(LPDISPATCH lpRow)
{
	try {

		if(m_CurServiceID == -1) {
			return;
		}

		NOCTypes eType = noctDefault;
		NOCTypes eOldType = noctDefault;
		_variant_t varType = g_cvarNull;
		
		// (j.jones 2013-07-15 16:17) - PLID 57566 - this is now in ServiceT
		_RecordsetPtr rs = CreateParamRecordset("SELECT NOCType FROM ServiceT WHERE ID = {INT}", m_CurServiceID);
		if(!rs->eof) {
			//if NULL, it's Default, otherwise it's a boolean for Yes/No
			_variant_t var = rs->Fields->Item["NOCType"]->Value;
			if(var.vt == VT_BOOL) {
				BOOL bNOCType = VarBool(var);
				eOldType = bNOCType ? noctYes : noctNo;
			}
		}
		rs->Close();

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			eType = (NOCTypes)VarLong(pRow->GetValue(noccID), (long)noctDefault);
			if(eType == noctYes) {
				varType = g_cvarTrue;
			}
			else if(eType == noctNo) {
				varType = g_cvarFalse;
			}
		}

		//save the setting
		// (j.jones 2013-07-15 16:17) - PLID 57566 - this is now in ServiceT
		ExecuteParamSql("UPDATE ServiceT SET NOCType = {VT_BOOL} WHERE ID = {INT}", varType, m_CurServiceID);

		//audit this
		CString strOldValue, strNewValue;
		switch(eOldType) {
			case noctYes:
				strOldValue = "Yes";
				break;
			case noctNo:
				strOldValue = "No";
				break;
			case noctDefault:
			default:
				strOldValue = "<Default>";
				break;
		}
		switch(eType) {
			case noctYes:
				strNewValue = "Yes";
				break;
			case noctNo:
				strNewValue = "No";
				break;
			case noctDefault:
			default:
				strNewValue = "<Default>";
				break;
		}


		CString strCodeName, strSubCodeName;
		GetDlgItemText(IDC_CPTCODE, strCodeName);
		GetDlgItemText(IDC_CPTSUBCODE, strSubCodeName);
		strSubCodeName.TrimLeft(); strSubCodeName.TrimRight();
		if(!strSubCodeName.IsEmpty()) {
			strCodeName += " - " + strSubCodeName;
		}

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, VarString(m_pCodeNames->GetValue(m_pCodeNames->CurSel,1)), nAuditID, aeiCPTNOCType, m_CurServiceID, strCodeName + ": " + strOldValue, strNewValue, aepLow, aetChanged);

		// (j.jones 2012-04-11 17:06) - PLID 47313 - If they selected "Yes", ask if they want to enter a claim note.
		// Prompt even if a claim note already exists.
		if(eType == noctYes && IDYES == MessageBox("A 'Not Otherwise Classified' (NOC) code requires a billing note with the \"Claim\" box checked.\n\n"
			"A default claim note can be entered for this service code by clicking the \"...\" button next to the Description field, "
			"and selecting \"Edit Claim Note\". This note will automatically add to bills with the \"Claim\" box checked.\n\n"
			"Would you like to enter a default claim note now?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
			EnterClaimNote();
		}

	}NxCatchAll(__FUNCTION__);
}*/
/*
// (j.jones 2012-01-03 15:36) - PLID 47300 - added NOC Combo
void CCPTCodes::OnSelChangingNocCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}	
	} NxCatchAll(__FUNCTION__);
}
*/
// (j.jones 2012-01-03 17:04) - PLID 47300 - added NOC label
/*void CCPTCodes::PopupNOCDescription()
{
	try {

		if(m_CurServiceID == -1) {
			return;
		}

		CString strCodeName, strSubCodeName;
		GetDlgItemText(IDC_CPTCODE, strCodeName);
		GetDlgItemText(IDC_CPTSUBCODE, strSubCodeName);
		strSubCodeName.TrimLeft(); strSubCodeName.TrimRight();
		if(!strSubCodeName.IsEmpty()) {
			strCodeName += " - " + strSubCodeName;
		}
		
		//run a recordset to use the exact same SQL logic the ebilling export will use
		// (j.jones 2013-07-15 16:17) - PLID 57566 - NOCType is now in ServiceT
		BOOL bIsNOCCode = ReturnsRecordsParam("SELECT ServiceT.ID FROM CPTCodeT "
			"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
			"WHERE CPTCodeT.ID = {INT} "
			"AND ("
				" Name LIKE '%Not Otherwise Classified%' "
				" OR Name LIKE '%Not Otherwise%' "
				" OR Name LIKE '%Unlisted%' "
				" OR Name LIKE '%Not listed%' "
				" OR Name LIKE '%Unspecified%' "
				" OR Name LIKE '%Unclassified%' "
				" OR Name LIKE '%Not otherwise specified%' "
				" OR Name LIKE '%Non-specified%' "
				" OR Name LIKE '%Not elsewhere specified%' "
				" OR Name LIKE '%Not elsewhere%' "
				" OR Name LIKE '%nos' "
				" OR Name LIKE '%nos %' "
				" OR Name LIKE '%nos;%' "
				" OR Name LIKE '%nos,%' "
				" OR Name LIKE '%noc' "
				" OR Name LIKE '%noc %' "
				" OR Name LIKE '%noc;%' "
				" OR Name LIKE '%noc,%' "
			")", m_CurServiceID);

		// (j.jones 2012-04-11 17:06) - PLID 47313 - updated this comment to mention the default claim note feature
		CString strMessage;
		strMessage.Format("A \"Not Otherwise Classified\" (NOC) code is any service code with the following words or phrases in the description:\n\n"
			"- Not Otherwise Classified \n"
			"- Not Otherwise \n"
			"- Unlisted \n"
			"- Not Listed \n"
			"- Unspecified \n"
			"- Unclassified \n"
			"- Not Otherwise Specified \n"
			"- Non-specified \n"
			"- Not Elsewhere Specified \n"
			"- Not Elsewhere \n"
			"- Nos (Note: Includes \"nos\", \"nos;\", \"nos,\") \n"
			"- Noc (Note: Includes \"noc\", \"noc;\", \"noc,\") \n\n"
			"Practice will treat all codes with these descriptions as NOC codes, unless specifically noted in this setting.\n\n"
			"This service code, %s, defaults to %sbeing reported as an NOC code. "
			"If this default is not correct, change the NOC code setting to Yes or No to force NOC on or off for this service code.\n\n"
			"NOC codes also require a description to be sent in ANSI claims in Loop 2400 SV101-7. "
			"Charges for NOC codes will need a billing note entered with the \"Claim\" box checked in order to enter this description. "
			"Practice will automatically report this note in the correct field for NOC codes.\n\n"
			"A default claim note can be entered for this service code by clicking the \"...\" button next to the Description field, "
			"and selecting \"Edit Claim Note\". This note will automatically add to bills with the \"Claim\" box checked.",
			strCodeName, bIsNOCCode ? "" : "not ");

		MsgBox(strMessage);

	} NxCatchAll(__FUNCTION__);
}*/

// (r.gonet 03/26/2012) - PLID 49043 - Save and audit when the user toggles whether this code requires a ref phys
/*
void CCPTCodes::OnBnClickedCptRequireRefPhys()
{
	try {
		// (r.gonet 03/26/2012) - PLID 49043 - Get the old value for auditing
		BOOL bOldRequireRefPhys = FALSE;
		_RecordsetPtr prs = CreateParamRecordset("SELECT RequireRefPhys FROM CPTCodeT WHERE ID = {INT}", m_CurServiceID);
		if(!prs->eof) {
			bOldRequireRefPhys = VarBool(prs->Fields->Item["RequireRefPhys"]->Value); // not null
		} else {
			// (r.gonet 03/26/2012) - PLID 49043 - Not sure what happened here, we should have a code. Anyway, we do have the default.
		}
		CString strOldValue = bOldRequireRefPhys != FALSE ? "Yes" : "No";

		// (r.gonet 03/26/2012) - PLID 49043 - Get the new value for auditing
		BOOL bNewRequireRefPhys = m_checkRequireRefPhys.GetCheck() ? TRUE : FALSE;   
		CString strNewValue = bNewRequireRefPhys != FALSE ? "Yes" : "No";

		// Get the code for auditing
		CString strCodeName, strSubCodeName;
		GetDlgItemText(IDC_CPTCODE, strCodeName);
		GetDlgItemText(IDC_CPTSUBCODE, strSubCodeName);
		strSubCodeName.TrimLeft(); strSubCodeName.TrimRight();
		if(!strSubCodeName.IsEmpty()) {
			strCodeName += " - " + strSubCodeName;
		}

		// (r.gonet 03/26/2012) - PLID 49043 - Commit the value to the database
		Save();

		// (r.gonet 03/26/2012) - PLID 49043 - Now audit
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, VarString(m_pCodeNames->GetValue(m_pCodeNames->CurSel,1)), nAuditID, aeiCPTReqRefPhys, m_CurServiceID, strCodeName + ": " + strOldValue, strNewValue, aepLow, aetChanged);
	} NxCatchAll(__FUNCTION__);
}
*/
// (s.dhole 2012-04-10 09:41) - PLID 47399
void CCPTCodes::OnKillfocusDefaultCost()
{	
	if(m_bIsSavingCPT || !m_bEditChanged)
		return;

	try {

		m_bIsSavingCPT = TRUE;

		CString str;
		COleCurrency cyTmp;
		GetDlgItemText(IDC_CPT_COST,str);
		cyTmp = ParseCurrencyFromInterface(str);

		BOOL bFailed = FALSE;
		if(cyTmp.GetStatus()==COleCurrency::invalid) {
			AfxMessageBox("An invalid currency was entered as the default cost.\n"
				"Please correct this.");
			bFailed = TRUE;
		}

		//must check for the previous condition to fail or we cannot examine cyTmp
		if(!bFailed && cyTmp < COleCurrency(0,0)) {
			AfxMessageBox("Practice does not allow a negative amount for a default cost.\n"
				"Please correct this.");
			bFailed = TRUE;
		}

		if(!bFailed && cyTmp > COleCurrency(100000000,0)) {
			CString str;
			str.Format("Practice does not allow an amount greater than %s.",FormatCurrencyForInterface(COleCurrency(100000000,0),TRUE,TRUE));
			AfxMessageBox(str);
			bFailed = TRUE;
		}

		//for auditing and resetting
		COleCurrency cyOld = COleCurrency(0,0);
		long nServiceID = -1;
		if(m_CurServiceID != -1) {
			nServiceID = m_CurServiceID;
			_RecordsetPtr rs = CreateRecordset("SELECT ServiceDefaultCost FROM CPTCodeT WHERE ID = %li", nServiceID);
			if(!rs->eof) {
				cyOld = AdoFldCurrency(rs, "ServiceDefaultCost", COleCurrency(0,0));
			}
			rs->Close();
		}

		if(bFailed) {
			//load the old fee
			SetDlgItemText(IDC_CPT_COST, FormatCurrencyForInterface(cyOld));
			m_bIsSavingCPT = FALSE;
			return;
		}

		str = FormatCurrencyForInterface(cyTmp);
		SetDlgItemText(IDC_CPT_COST,str);

		Save();

		//Audit changes to the default cost
		if(cyOld != cyTmp) {
			//They did change the fee, audit it
			CString strCodeName, strSubCodeName;
			GetDlgItemText(IDC_CPTCODE, strCodeName);
			GetDlgItemText(IDC_CPTSUBCODE, strSubCodeName);

			//piece these together
			strCodeName += " - " + strSubCodeName;

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, strCodeName, nAuditID, aeiCPTDefaultCost, nServiceID, FormatCurrencyForInterface(cyOld), FormatCurrencyForInterface(cyTmp), aepMedium, aetChanged);
		}

	} NxCatchAll("Error saving default cost.");

	m_bIsSavingCPT = FALSE;
}

// (j.jones 2012-04-11 17:11) - PLID 47313 - added ServiceT.ClaimNote
void CCPTCodes::EnterClaimNote()
{
	try {

		if(m_CurServiceID == -1) {
			return;
		}

		CString strOldClaimNote = "";
		_RecordsetPtr rs = CreateParamRecordset("SELECT ClaimNote FROM ServiceT WHERE ID = {INT}", m_CurServiceID);
		if(!rs->eof) {
			strOldClaimNote = VarString(rs->Fields->Item["ClaimNote"]->Value, "");
			strOldClaimNote.TrimLeft(); strOldClaimNote.TrimRight();
		}
		rs->Close();

		CString strNewClaimNote = strOldClaimNote;
		if(IDOK == InputBoxLimited(this, "Enter a default billing note to add to charges with \"Claim\" checked:", strNewClaimNote, "", 80, false, false, NULL)) {
			strNewClaimNote.TrimLeft();
			strNewClaimNote.TrimRight();

			//ensure it's not > 80 characters
			if(strNewClaimNote.GetLength() > 80) {
				strNewClaimNote = strNewClaimNote.Left(80);
			}

			if(strNewClaimNote.CompareNoCase(strOldClaimNote) != 0) {
				ExecuteParamSql("UPDATE ServiceT SET ClaimNote = {STRING} WHERE ID = {INT}", strNewClaimNote, m_CurServiceID);

				//audit this
				CString strCodeName, strSubCodeName;
				GetDlgItemText(IDC_CPTCODE, strCodeName);
				GetDlgItemText(IDC_CPTSUBCODE, strSubCodeName);
				strSubCodeName.TrimLeft(); strSubCodeName.TrimRight();
				if(!strSubCodeName.IsEmpty()) {
					strCodeName += " - " + strSubCodeName;
				}

				long nAuditID = BeginNewAuditEvent();
				// (j.dinatale 2012-06-15 12:04) - PLID 51000 - changed the audit enum name
				AuditEvent(-1, VarString(m_pCodeNames->GetValue(m_pCodeNames->CurSel,1)), nAuditID, aeiDefClaimNote, m_CurServiceID, strCodeName + ": " + strOldClaimNote, strNewClaimNote, aepLow, aetChanged);

				//send a tablechecker
				m_CPTChecker.Refresh(m_CurServiceID);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-07-24 09:00) - PLID 51651 - Added a preference to only track global periods for
// surgical codes only, this will control whether the edit box is enabled for all codes or not.
void CCPTCodes::ReflectGlobalPeriodReadOnly(_variant_t varIsSurgicalCode /*= g_cvarNull*/)
{
	try {

		if(m_CurServiceID == -1) {
			return;
		}

		BOOL bIsSurgicalCode = FALSE;
		if(varIsSurgicalCode.vt == VT_BOOL) {
			//our caller gave us this value
			bIsSurgicalCode = VarBool(varIsSurgicalCode);
		}
		else {
			//we need to load the value from data
			_RecordsetPtr rsPayCat = CreateParamRecordset("SELECT ServicePayGroupsT.Category FROM ServiceT "
				"INNER JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
				"WHERE ServiceT.ID = {INT}", m_CurServiceID);
			if(!rsPayCat->eof) {
				PayGroupCategory::Category ePayGroupCategory = (PayGroupCategory::Category)VarLong(rsPayCat->Fields->Item["Category"]->Value, (long)PayGroupCategory::NoCategory);							
				if(ePayGroupCategory == PayGroupCategory::SurgicalCode) {
					bIsSurgicalCode = TRUE;
				}
			}
			rsPayCat->Close();
		}

		long nSurgicalCodesOnly = GetRemotePropertyInt("GlobalPeriod_OnlySurgicalCodes", 1, 0, "<None>", true);
		BOOL bCanWrite = GetCurrentUserPermissions(bioServiceCodes) & SPT___W________ANDPASS;
		m_editGlobalPeriod.SetReadOnly(!bCanWrite || (nSurgicalCodesOnly == 1 && !bIsSurgicalCode));

	}NxCatchAll(__FUNCTION__);
}
// (j.gruber 2012-10-25 15:52) - PLID 53240
void CCPTCodes::OnBnClickedBtnEditBillableShopFees()
{
	try {
		CString strTmp;
		COleCurrency cyItem;
		GetDlgItemText(IDC_STD_FEE, strTmp);
		cyItem = ParseCurrencyFromInterface(strTmp);

		CEditShopFeesDlg dlg(m_CurServiceID, VarString(m_pCodeNames->GetValue(m_pCodeNames->CurSel,1)), cyItem, GetNxColor(GNC_ADMIN, 0));
		dlg.DoModal();

		//reload the dialog so it refreshes
		UpdateView();
	}NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-5-5) PLID 61831 - remove the old ServiceT.SendOrderingPhy field code.

// (a.wilson 2014-01-13 16:37) - PLID 59956 - Launches the additional info. dialog.
/*
void CCPTCodes::OnBnClickedCptAdditionalInfoBtn()
{
	try {
		//Initialize dialog
		CCPTAdditionalInfoDlg dlg(this);

		//Pass current values
		dlg.m_strTOS = m_strTOS;
		dlg.m_eCCDAType = m_eCCDAType;

		//Run dialog
		dlg.DoModal();

		//Compare values for changes.  if changes exist then save cpt code.
		if (m_strTOS != dlg.m_strTOS || m_eCCDAType != dlg.m_eCCDAType)
		{
			//get new values.
			m_strTOS = dlg.m_strTOS;
			m_eCCDAType = dlg.m_eCCDAType;

			//save cpt code.
			m_bIsSavingCPT = TRUE;

			Save();

			m_bIsSavingCPT = FALSE;
		}

	} NxCatchAll(__FUNCTION__);
}
*/

// (c.haag 2015-05-11) - PLID 66017 - Shows or hides the diagnosis list or diagnosis custom crosswalks
void CCPTCodes::ShowDiagDatalist(DiagDatalist listToShow)
{
	UINT uShowDiagCodes = (DiagCodes == listToShow) ? SW_SHOW : SW_HIDE;
	UINT uShowCustomNexGEMs = (CustomNexGEMs == listToShow) ? SW_SHOW : SW_HIDE;

	GetDlgItem(IDC_DIAG_CODES)->ShowWindow(uShowDiagCodes);
	GetDlgItem(IDC_ADD_ICD9)->ShowWindow(uShowDiagCodes);
	GetDlgItem(IDC_MARK_ICD9_INACTIVE)->ShowWindow(uShowDiagCodes);
	GetDlgItem(IDC_DELETE_ICD9)->ShowWindow(uShowDiagCodes);
	GetDlgItem(IDC_FILTER_ICD9)->ShowWindow(uShowDiagCodes);

	GetDlgItem(IDC_DIAG_CUSTOM_NEXGEMS)->ShowWindow(uShowCustomNexGEMs);
	// (c.haag 2015-05-14) - PLID 66019 - Include the diagnosis search lists
	GetDlgItem(IDC_ICD9_DIAG_SEARCH_LIST)->ShowWindow(uShowCustomNexGEMs);
	GetDlgItem(IDC_EDIT_SELECTED_ICD9)->ShowWindow(uShowCustomNexGEMs);
	GetDlgItem(IDC_ICD10_DIAG_SEARCH_LIST)->ShowWindow(uShowCustomNexGEMs);
	GetDlgItem(IDC_EDIT_SELECTED_ICD10)->ShowWindow(uShowCustomNexGEMs);
	// (c.haag 2015-05-14) - PLID 66020 - Adding mappings
	GetDlgItem(IDC_BTN_ADD_CUSTOM_MAPPING)->ShowWindow(uShowCustomNexGEMs);
}

// (c.haag 2015-05-11) - PLID 66017 - Refreshes the custom NexGEMs list if it's visible
void CCPTCodes::RefreshCustomNexGEMsList()
{
	if (adicvpCustomNexGEMs != GetRemotePropertyInt("AdminDisplayICDCodeVersion", adicvpICD9Codes, 0, GetCurrentUserName()))
	{
		// List isn't visible
	}
	else
	{
		// Clear the list
		m_pDiagCodeCustomNexGEMs->Clear();

		// Populate the list
		NexTech_Accessor::_DiagSearchResultsPtr pDiagResults = GetAPI()->GetCustomNexGEMs(GetAPISubkey(), GetAPILoginToken(), _bstr_t(""), g_cvarTrue);
		Nx::SafeArray<IUnknown *> saResults = pDiagResults->SearchResults;
		for each(NexTech_Accessor::_DiagSearchResultPtr pResult in saResults)
		{
			AddCustomNexGEMToList(pResult);
		}
	}
}

// (j.armen 2014-03-05 17:39) - PLID 61207 - Update preference when button is clicked
void CCPTCodes::OnBnClickedCptCodesDiagIcd()
{
	try
	{
		if(m_checkDiagICD9.GetCheck() == BST_CHECKED)
			SetRemotePropertyInt("AdminDisplayICDCodeVersion", adicvpICD9Codes, 0, GetCurrentUserName());
		else if(m_checkDiagICD10.GetCheck() == BST_CHECKED)
			SetRemotePropertyInt("AdminDisplayICDCodeVersion", adicvpICD10Codes, 0, GetCurrentUserName());
		else if (m_checkDiagCustomNexGEMs.GetCheck() == BST_CHECKED) // (c.haag 2015-05-11) - PLID 66017
			SetRemotePropertyInt("AdminDisplayICDCodeVersion", adicvpCustomNexGEMs, 0, GetCurrentUserName());

		if (m_checkDiagCustomNexGEMs.GetCheck() == BST_CHECKED)
		{
			// (c.haag 2015-05-11) - PLID 66017 - If we chose to show the custom NexGEM mappings, then hide
			// the old buttons and datalist
			ShowDiagDatalist(CustomNexGEMs);
			// Populate the custom NexGEMs list
			RefreshCustomNexGEMsList();
		}
		else
		{
			// (c.haag 2015-05-11) - PLID 66017 - If we chose to hide the custom NexGEM mappings, then show
			// the old buttons and datalist
			ShowDiagDatalist(DiagCodes);

			// (j.armen 2014-03-06 09:59) - PLID 61209 - Replace the Active = 1 AND ICD10 = ? portion of the query
			// It is possible it could have other filters associated that should not be changed
			CString strWhereClause = VarString(m_pDiagCodes->WhereClause);
			if (m_checkDiagICD9.GetCheck() == BST_CHECKED)
				strWhereClause.Replace("(Active = 1 AND ICD10 = 1)", "(Active = 1 AND ICD10 = 0)");
			else if (m_checkDiagICD10.GetCheck() == BST_CHECKED)
				strWhereClause.Replace("(Active = 1 AND ICD10 = 0)", "(Active = 1 AND ICD10 = 1)");

			m_pDiagCodes->WhereClause = _bstr_t(strWhereClause);

			m_pDiagCodes->Requery();

			if (m_checkDiagICD10.GetCheck() == BST_CHECKED)
			{
				// (j.armen 2014-03-25 09:21) - PLID 61534 - Disable editing on code number / description for ICD-10's
				m_pDiagCodes->GetColumn(dclcCodeNumber)->Editable = VARIANT_FALSE;
				m_pDiagCodes->GetColumn(dclcCodeDesc)->Editable = VARIANT_FALSE;
				// (j.armen 2014-03-25 09:22) - PLID 61535 - Hide AMA Column when showing ICD 10's
				m_pDiagCodes->GetColumn(dclcIsAMA)->StoredWidth = 0;
			}
			else
			{
				m_pDiagCodes->GetColumn(dclcCodeNumber)->Editable = VARIANT_TRUE;
				m_pDiagCodes->GetColumn(dclcCodeDesc)->Editable = VARIANT_TRUE;
				m_pDiagCodes->GetColumn(dclcIsAMA)->StoredWidth = 55;
			}
		}

	}NxCatchAll(__FUNCTION__);
}


// (s.tullis 2014-05-01 15:48) - PLID 61939 - Add “Additional Service Code Setup” button to the bottom of the CPT Configuration screen.
void CCPTCodes::OnAdditionalServiceCodeSetup(){

	try{

		//(s.tullis 2014 - 05 - 20 17:28) - PLID 61939 no service codes with id of -1 
		if (m_CurServiceID == -1)
			return;

		enum MenuOptions {
			moClaimSetup = 1,
			moUBSetup,
			moCCDASetup,
			moAnesthesiaFacilitySetup,
			moOHIPSetup,
		};

		BOOL bUB4inUse = FALSE;
		BOOL bClaimSetupInUse = FALSE;
		BOOL bAnesthesiaFacilityInUse = FALSE;
		BOOL bCCDASetupInUse = FALSE;
		BOOL bOHIPSetupInUse = FALSE;
		CString CCDAtype;



		_RecordsetPtr	tmpRS;
		FieldsPtr Fields;


		//(s.tullis 2014-05-20 17:28) - PLID 61939 query to check if any of the dialogs in the menu are in use
		// (r.gonet 07/08/2014) - PLID 62572 - Added CPTCodeT.DefaultAsOnHold to the claim setup dialog check
		// (r.gonet 08/06/2014) - PLID 63096 - Added LabCharge to the case statement for determining if Claim Setup is in use.
		//  Also formatted the query to make a bit more readable.
		tmpRS = CreateParamRecordset(
			"Select CONVERT(bit, Case when LTRIM(RTRIM(UBBOX4)) = '' AND RevCodeUse = 0  AND ICD9ProcedureCodeID IS NULL  THEN 0 ELSE 1 END) as UB4inUse, "
			"CONVERT(bit, Case when "
			"	LTRIM(RTRIM(ClaimNote)) = '' "
			"	AND NOCType IS NULL "
			"	AND BatchIFZero = 0 "
			"	AND RequireRefPhys = 0 "
			"	AND CPTCodeT.DefaultAsOnHold = 0 "
			"	AND ServiceT.LabCharge = 0 "
			"	AND LTRIM(RTRIM(DefNDCCode)) = '' "
			"	AND LTRIM(RTRIM(DefNDCUnitType)) = '' "
			"	AND DefNDCQty = 0 "
			"	AND DefNDCUnitPrice = 0.00 THEN 0 ELSE 1 END ) as ClaimSetupInUse, "
			"CONVERT(bit, Case When  "
			"	Anesthesia = 0 "
			"	AND FacilityFee= 0 THEN 0 ELSE 1 END) as AnesthesiaFacilityInUse, "
			"CONVERT(bit, Case WHEN "
			"	AssistingCode = 0 "
			"	AND OHIPPremiumCode=0 THEN 0 ELSE 1 END) as OHIPSetupInUse, "
			"CONVERT(bit, CASE WHEN "
			"	CCDAType IS NULL THEN 0 ELSE 1 END) AS CCDASetupInUse, "
			"CASE WHEN CPTCodeT.CCDAType = 1 THEN 'Procedure' "
			"WHEN CPTCodeT.CCDAType = 2 THEN 'Observation' "
			"WHEN CPTCodeT.CCDAType = 3 THEN 'Act' "
			"WHEN CPTCodeT.CCDAType = 4 THEN 'Encounter' "
			"ELSE '' END AS CCDAType "
			" FROM ServiceT "
			" Inner Join CPTCodeT ON CPTCodeT.ID = serviceT.ID "
			" WHERE ServiceT.ID = { INT } ", m_CurServiceID);
		//(s.tullis 2014-05-20 17:28) - PLID 61939- get values
		Fields = tmpRS->Fields;
		if (!tmpRS->eof){
			bUB4inUse = VarBool(Fields->GetItem("UB4inUse")->Value);
			bClaimSetupInUse = VarBool(Fields->GetItem("ClaimSetupInUse")->Value);
			bAnesthesiaFacilityInUse = VarBool(Fields->GetItem("AnesthesiaFacilityInUse")->Value);
			bCCDASetupInUse = VarBool(Fields->GetItem("CCDASetupInUse")->Value);
			bOHIPSetupInUse = VarBool(Fields->GetItem("OHIPSetupInUse")->Value);
			CCDAtype = VarString(Fields->GetItem("CCDAType")->Value, "");

		}


		tmpRS->Close();
		CMenu menu;


		menu.m_hMenu = CreatePopupMenu();
		//(s.tullis 2014-05-20 17:28) - PLID 61939 - insert menu options and a check next to the option if the corresponding dialog is in use
		if (bCCDASetupInUse && !CCDAtype.IsEmpty()){
			CCDAtype = "(" + CCDAtype + ")";
			menu.InsertMenu(2, MF_BYPOSITION | (bCCDASetupInUse ? MF_CHECKED : 0), moCCDASetup, "CCDA Setup... " + CCDAtype);
		}
		else{
			menu.InsertMenu(2, MF_BYPOSITION | (bCCDASetupInUse ? MF_CHECKED : 0), moCCDASetup, "CCDA Setup...");
		}
		menu.InsertMenu(0, MF_BYPOSITION | (bClaimSetupInUse ? MF_CHECKED : 0), moClaimSetup, "Claim Setup...");
		menu.InsertMenu(1, MF_BYPOSITION | (bUB4inUse ? MF_CHECKED : 0), moUBSetup, "UB Setup...");
		menu.InsertMenu(3, MF_BYPOSITION | (bAnesthesiaFacilityInUse ? MF_CHECKED : 0), moAnesthesiaFacilitySetup, "Anesthesia/Facility Setup...");

		if (UseOHIP()){// (s.tullis 2014-05-20 17:28) - PLID 61939 only insert the OHIP option if the OHIP preference is enabled

			menu.InsertMenuA(4, MF_BYPOSITION | (bOHIPSetupInUse ? MF_CHECKED : 0), moOHIPSetup, "OHIP Setup...");

		}


		// (s.tullis 2014-05-20 17:28) - PLID 61939 get the a control to the additionaly service code setup button and get a pointer to the topleft corner of the button so the menu can be stacked on the button and aligned to the left
		CRect rc;
		CWnd *pWnd = GetDlgItem(IDC_ADD_SERVICE_CODE_SETUP);
		CPoint pt;
		GetCursorPos(&pt);
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			pt = rc.TopLeft();
		}
		//(s.tullis 2014-05-20 17:28) - PLID 61939 Enable menu then get the result
		long nMenuChoice = menu.TrackPopupMenu(TPM_BOTTOMALIGN | TPM_RETURNCMD, pt.x, pt.y, this, NULL);
		menu.DestroyMenu();

		//(s.tullis 2014-05-20 17:28) - PLID 61939 get values for auditing in the dialog choices
		CString strCodeName, strSubCodeName, strCPTCodeDesc;
		GetDlgItemText(IDC_CPTCODE, strCodeName);
		GetDlgItemText(IDC_CPTSUBCODE, strSubCodeName);
		GetDlgItemText(IDC_DESCRIPTION, strCPTCodeDesc);
		

		//(s.tullis 2014-05-20 17:28) - PLID 61939 lauch dialog and pass values based on menu choice
		if (nMenuChoice == moClaimSetup) {
			// Claim Setup
			CClaimSetup dlg(this);
			dlg.m_nServiceID = m_CurServiceID;
			dlg.strCodeName = strCodeName;
			dlg.strSubCodeName = strSubCodeName;
			dlg.strCPTCodeDesc = strCPTCodeDesc;
			dlg.DoModal();
		}
		else if (nMenuChoice == moUBSetup) {
			//UB setup
			CUBSetupDLG dlg(this);
			dlg.bCanWrite = bCanWrite;
			dlg.m_nServiceID = m_CurServiceID;
			dlg.DoModal();
		}
		else if (nMenuChoice == moCCDASetup){
			// CCDA Setup
			CUpdateMultiCCDATypeDlg dlg(this);
			dlg.DoModal();
		}
		else if (nMenuChoice == moAnesthesiaFacilitySetup){
			//Anethesia Facility Setup
			CString strCode;
			GetDlgItemText(IDC_CPTCODE, strCode);

			CAnesthesiaFacilitySetupDlg dlg(this);
			dlg.bCanWrite = bCanWrite;
			dlg.m_nServiceID = m_CurServiceID;
			dlg.m_strServCode = strCode;
			dlg.DoModal();
		}
		else if (nMenuChoice == moOHIPSetup){
			// OHIP setup
			COHIPCPTSetupDlg dlg(this);
			dlg.m_nServiceID = m_CurServiceID;
			dlg.bCanWrite = bCanWrite;
			dlg.DoModal();
		}

	}NxCatchAll(__FUNCTION__)
}

void CCPTCodes::OnSelChosenICD9DiagSearchList(LPDISPATCH lpRow)
{
	// (c.haag 2015-05-14) - PLID 66019 - Called to handle when the user explicitly chooses
	// a code from the ICD-9 search dropdown
	try
	{
		if (nullptr == lpRow)
		{
			// No row selected; nothing to do
		}
		else
		{
			CDiagSearchResults results = DiagSearchUtils::Common_ConvertPreferenceSearchResults(lpRow, m_pICD9WithPCSSearchConfig);
			if (results.m_ICD9.m_nDiagCodesID == -1)
			{
				// No code selected; nothing to do
			}
			else
			{
				// Got a valid code
				m_nICD9IDForCustomNexGEM = results.m_ICD9.m_nDiagCodesID;
				SetDlgItemText(IDC_EDIT_SELECTED_ICD9, results.m_ICD9.m_strCode);
				UpdateAddCustomMappingButton();
			}
		}
	}
	NxCatchAll(__FUNCTION__)
}

void CCPTCodes::OnSelChosenICD10DiagSearchList(LPDISPATCH lpRow)
{
	// (c.haag 2015-05-14) - PLID 66019 - Called to handle when the user explicitly chooses
	// a code from the ICD-10 search dropdown
	try
	{
		if (nullptr == lpRow)
		{
			// No row selected; nothing to do
		}
		else
		{
			CDiagSearchResults results = DiagSearchUtils::Common_ConvertPreferenceSearchResults(lpRow, m_pICD10WithPCSSearchConfig);
			if (results.m_ICD10.m_nDiagCodesID == -1) 
			{
				// No code selected; nothing to do
			}
			else
			{
				// Got a valid code
				m_nICD10IDForCustomNexGEM = results.m_ICD10.m_nDiagCodesID;
				SetDlgItemText(IDC_EDIT_SELECTED_ICD10, results.m_ICD10.m_strCode);
				UpdateAddCustomMappingButton();
			}
		}
	}
	NxCatchAll(__FUNCTION__)
}

// (c.haag 2015-05-14) - PLID 66020 - Updates the enabled state of the add custom mapping button
void CCPTCodes::UpdateAddCustomMappingButton()
{
	GetDlgItem(IDC_BTN_ADD_CUSTOM_MAPPING)->EnableWindow(-1 != m_nICD9IDForCustomNexGEM && -1 != m_nICD10IDForCustomNexGEM);
}

// (c.haag 2015-05-14) - PLID 66020 - Adds a custom NexGEM to the list
void CCPTCodes::AddCustomNexGEMToList(LPDISPATCH lpDiagResult)
{
	NexTech_Accessor::_DiagSearchResultPtr pResult(lpDiagResult);
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pDiagCodeCustomNexGEMs->GetNewRow();

	NexTech_Accessor::_SearchDiagCodePtr pICD9Code = pResult->ICD9Code;
	if (nullptr != pICD9Code)
	{
		pRow->Value[cngICD9ID] = atoi(pICD9Code->DiagCodesID);
		pRow->Value[cngICD9Code] = pICD9Code->Code;
		pRow->Value[cngICD9Desc] = pICD9Code->description;
	}

	NexTech_Accessor::_SearchDiagCodePtr pICD10Code = pResult->ICD10Code;
	if (nullptr != pICD10Code)
	{
		pRow->Value[cngICD10ID] = atoi(pICD10Code->DiagCodesID);
		pRow->Value[cngICD10Code] = pICD10Code->Code;
		pRow->Value[cngICD10Desc] = pICD10Code->description;
	}

	// (c.haag 2015-05-15) - PLID 66021
	pRow->Value[cngQuicklist] = (VARIANT_FALSE != pResult->InCurrentUserQuickList);

	m_pDiagCodeCustomNexGEMs->AddRowSorted(pRow, NULL);
}

// (c.haag 2015-05-14) - PLID 66020 - Called when the user pushes the Add Custom Mapping button
void CCPTCodes::OnBnClickedAddCustomMapping()
{	
	try
	{
		// Permission check
		if (!CheckCurrentUserPermissions(bioDiagCodes, sptCreate)) 
			return;

		// These should never be true, but we should check anyway
		if (-1 == m_nICD9IDForCustomNexGEM)
		{
			AfxMessageBox("Please select an ICD-9 code to assign to the custon mapping before continuing.", MB_OK | MB_ICONSTOP);
		}
		else if (-1 == m_nICD10IDForCustomNexGEM)
		{
			AfxMessageBox("Please select an ICD-10 code to assign to the custon mapping before continuing.", MB_OK | MB_ICONSTOP);
		}
		else
		{
			try
			{
				NexTech_Accessor::_DiagSearchResultPtr pResult = GetAPI()->AddCustomNexGEM(GetAPISubkey(), GetAPILoginToken(), _bstr_t(m_nICD9IDForCustomNexGEM), _bstr_t(m_nICD10IDForCustomNexGEM));
				AddCustomNexGEMToList(pResult);
			}
			catch (_com_error& e)
			{
				CString strError((LPCTSTR)e.Description());
				if (-1 != strError.Find("The specified codes already exist as a custom NexGEM!"))
				{
					AfxMessageBox("The code combination already exists in data as a custom NexGEM.", MB_OK | MB_ICONSTOP);
					// Requery in case someone else added it
					RefreshCustomNexGEMsList();
				}
				else
				{
					throw;
				}
			}

			// Now clear out the desired fields since they exist in the list
			m_nICD9IDForCustomNexGEM = -1;
			m_nICD10IDForCustomNexGEM = -1;
			SetDlgItemText(IDC_EDIT_SELECTED_ICD9, "");
			SetDlgItemText(IDC_EDIT_SELECTED_ICD10, "");
			UpdateAddCustomMappingButton();
		}
	}
	NxCatchAll(__FUNCTION__)
}

// (c.haag 2015-05-15) - PLID 66021 - Handle right-clicks on the custom NexGEMs list
void CCPTCodes::OnRButtonDownCustomNexGEMsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (nullptr != pRow)
		{
			// Permission check
			if (!CheckCurrentUserPermissions(bioDiagCodes, sptDelete))
				return;

			// Highlight the row so the user knows which one is being asked about
			m_pDiagCodeCustomNexGEMs->CurSel = pRow;

			// Show the delete menu
			CNxMenu menu;
			const int nCommand = 50000;
			menu.CreatePopupMenu();
			menu.AppendMenu(MF_ENABLED, nCommand, "Remove");
			CPoint pt;
			GetCursorPos(&pt);
			int nMenuCmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this);
			if (nCommand == nMenuCmd)
			{
				CString strMsg = VarBool(pRow->Value[cngQuicklist]) ?
					"Are you sure you wish to delete the selected ICD-9/ICD-10 custom mapping and also remove it from your Quicklist?"
					: "Are you sure you wish to delete the selected ICD-9/ICD-10 custom mapping?";

				NxTaskDialog dlgDelete;
				dlgDelete.Config()
					.WarningIcon()
					.ZeroButtons()
					.MainInstructionText(strMsg)
					.AddCommand(1000, "Delete Custom Mapping")
					.AddCommand(1001, "Cancel")
					.DefaultButton(1001);

				if (1000 == dlgDelete.DoModal())
				{
					// Determine if this exists in any other users' Quicklist
					NexTech_Accessor::_GetQuickListUsersUsingCrosswalkResultPtr result =  GetAPI()->GetQuickListUsersUsingCrosswalk(GetAPISubkey(), GetAPILoginToken(), _bstr_t(VarLong(pRow->Value[cngICD9ID])), _bstr_t(VarLong(pRow->Value[cngICD10ID])));
					Nx::SafeArray<IUnknown *> saResults(result->QuickListUsers);
					_bstr_t myUserID(GetCurrentUserID());
					BOOL bFoundInOtherUsersQuickLists = FALSE;
					for each(NexTech_Accessor::_UserPtr user in saResults)
					{
						if (user->ID != myUserID)
						{
							bFoundInOtherUsersQuickLists = TRUE;
							break;
						}
					}

					if (bFoundInOtherUsersQuickLists)
					{
						NxTaskDialog dlgConfirm;
						dlgConfirm.Config()
							.WarningIcon()
							.ZeroButtons()
							.MainInstructionText(R"(The selected ICD-9/ICD-10 Custom Mapping is in another user’s Diagnosis Quicklist.

Removing this custom mapping will also remove it from any user’s Diagnosis Quicklist.)")
							.AddCommand(1000, "Delete Custom Mapping")
							.AddCommand(1001, "Cancel")
							.DefaultButton(1001);
						if (1000 != dlgConfirm.DoModal())
						{
							return;
						}
					}

					// Now delete the NexGEM
					GetAPI()->DeleteCustomNexGEM(GetAPISubkey(), GetAPILoginToken(), _bstr_t(VarLong(pRow->Value[cngICD9ID])), _bstr_t(VarLong(pRow->Value[cngICD10ID])));

					// Remove the selected row
					m_pDiagCodeCustomNexGEMs->RemoveRow(pRow);
				}
			}

		}
	}
	NxCatchAll(__FUNCTION__)
}
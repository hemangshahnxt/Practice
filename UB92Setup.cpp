// UB92Setup.cpp : implementation file
//

#include "stdafx.h"
#include "PracProps.h"
#include "UB92Setup.h"
#include "UB92Categories.h"
#include "EditInsInfoDlg.h"
#include "PrintAlignDlg.h"
#include "UB92SetupInfo.h"
#include "UB92DatesDlg.h"
#include "AuditTrail.h"
#include "AdvEbillingSetup.h"
#include "AdministratorRc.h"
#include "MultiSelectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



const int BACKGROUND_COLOR  = GetNxColor(GNC_ADMIN, 0);

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CUB92Setup dialog


CUB92Setup::CUB92Setup(CWnd* pParent)
	: CNxDialog(CUB92Setup::IDD, pParent),
	m_labelChecker(NetUtils::CustomLabels),
	m_companyChecker(NetUtils::InsuranceCoT), // All insurance companies
	m_groupsChecker(NetUtils::InsuranceGroups)
{
	//{{AFX_DATA_INIT(CUB92Setup)
	m_loading = false;
	m_strOldBox76Qualifier = "";
	//}}AFX_DATA_INIT

	//(j.anspach 06-13-2005 13:35 PLID 16662) - Adding a specific help file reference for Administrator -> UB92
	// (j.jones 2007-10-24 10:03) - PLID 27855 - moved the location of the setup html file in the help layout
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "System_Setup/Billing_Setup/Insurance_Billing_Setup/configure_the_ub92_form.htm";
}


void CUB92Setup::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUB92Setup)
	DDX_Control(pDX, IDC_BTN_EXPAND_CODES, m_nxibCodesToExpand);
	DDX_Control(pDX, IDC_CHECK_USE_ICD9_PROCEDURE_CODES, m_checkUseICD9);
	DDX_Control(pDX, IDC_CHECK_USE_THREE, m_checkUseThree);
	DDX_Control(pDX, IDC_CHECK_BOX_42_LINE_23, m_checkBox42Line23);
	DDX_Control(pDX, IDC_BOX_57_CHECK, m_checkBox57);
	DDX_Control(pDX, IDC_BOX_51_CHECK, m_checkBox51);
	DDX_Control(pDX, IDC_64_CHECK, m_checkBox64);
	DDX_Control(pDX, IDC_RADIO_BOX5_LOC_EIN, m_radioBox5LocEIN);
	DDX_Control(pDX, IDC_RADIO_BOX5_PROV_FEDID, m_radioBox5ProvTaxID);
	DDX_Control(pDX, IDC_ADV_EBILLING_SETUP, m_btnAdvEbillingSetup);
	DDX_Control(pDX, IDC_CHECK_SHOW_EMPLOYER_COMPANY, m_checkShowEmployerCompany);
	DDX_Control(pDX, IDC_RADIO_BOX_80_DIAG, m_radioBox80Diag);
	DDX_Control(pDX, IDC_RADIO_BOX_80_CPT, m_radioBox80CPT);
	DDX_Control(pDX, IDC_RADIO_BOX_80_NONE, m_radioBox80None);
	DDX_Control(pDX, IDC_BTN_UB92_DATES, m_btnEditUB92Dates);
	DDX_Control(pDX, IDC_CHECK_SHOW_APPLIES, m_checkShowApplies);
	DDX_Control(pDX, IDC_CHECK_FILL_42_WITH_44, m_checkFill42With44);
	DDX_Control(pDX, IDC_76_CHECK, m_checkBox76);
	DDX_Control(pDX, IDC_CHECK_PUNCTUATE_DIAGS, m_checkPunctuateDiagCodes);
	DDX_Control(pDX, IDC_CHECK_PUNCTUATE_CHARGE_LINES, m_checkPunctuateChargeLines);
	DDX_Control(pDX, IDC_RADIO_BOX1_POS, m_radioBox1POS);
	DDX_Control(pDX, IDC_RADIO_BOX1_LOCATION, m_radioBox1Location);
	DDX_Control(pDX, IDC_CHECK_SHOW_INS_ADDR_BOX_50, m_checkShowInsAddr50);
	DDX_Control(pDX, IDC_CHECK_SHOW_INSCO_ADDRESS, m_checkShowInscoAdd);
	DDX_Control(pDX, IDC_CHECK_SHOW_TOTAL_AMOUNT, m_checkShowTotals);
	DDX_Control(pDX, IDC_CHECK_SHOW_PHONE_NUMBER, m_checkShowPhoneNum);
	DDX_Control(pDX, IDC_GROUP_CHARGES_CHECK, m_checkGroupCharges);
	DDX_Control(pDX, IDC_85_CHECK, m_checkBox85);
	DDX_Control(pDX, IDC_RADIO_DATE_YEARFIRST, m_radioDateYearFirst);
	DDX_Control(pDX, IDC_RADIO_DATE_MONTHFIRST, m_radioDateMonthFirst);
	DDX_Control(pDX, IDC_EDIT_UB92_CATEGORIES, m_btnEditCategories);
	DDX_Control(pDX, IDC_RADIO_BOX38_INSURED_PARTY, m_radioBox38InsuredParty);
	DDX_Control(pDX, IDC_RADIO_BOX38_INSURANCE_CO, m_radioBox38InsuranceCo);
	DDX_Control(pDX, IDC_NEW_UB92_GROUP, m_btnNewGroup);
	DDX_Control(pDX, IDC_EDIT_INS_INFO, m_btnEditInsInfo);
	DDX_Control(pDX, IDC_DELETE_UB92_GROUP, m_btnDeleteGroup);
	DDX_Control(pDX, IDC_ALIGN_FORM, m_btnAlignForm);
	DDX_Control(pDX, IDC_ADD_COMPANY, m_btnAddOne);
	DDX_Control(pDX, IDC_ADD_ALL_COMPANIES, m_btnAddAll);
	DDX_Control(pDX, IDC_REMOVE_COMPANY, m_btnRemOne);
	DDX_Control(pDX, IDC_REMOVE_ALL_COMPANIES, m_btnRemAll);
	DDX_Control(pDX, IDC_EDIT_BOX42LINE23, m_nxeditEditBox42line23);
	DDX_Control(pDX, IDC_EDIT_BOX51_DEFAULT, m_nxeditEditBox51Default);
	DDX_Control(pDX, IDC_EDIT_BOX4, m_nxeditEditBox4);
	DDX_Control(pDX, IDC_EDIT_BOX8, m_nxeditEditBox8);
	DDX_Control(pDX, IDC_EDIT_BOX10, m_nxeditEditBox10);
	DDX_Control(pDX, IDC_EDIT_BOX18, m_nxeditEditBox18);
	DDX_Control(pDX, IDC_EDIT_BOX19, m_nxeditEditBox19);
	DDX_Control(pDX, IDC_EDIT_BOX20, m_nxeditEditBox20);
	DDX_Control(pDX, IDC_EDIT_BOX22, m_nxeditEditBox22);
	DDX_Control(pDX, IDC_EDIT_BOX32, m_nxeditEditBox32);
	DDX_Control(pDX, IDC_EDIT_BOX79, m_nxeditEditBox79);
	DDX_Control(pDX, IDC_EDIT_BOX66, m_nxeditEditBox66);
	DDX_Control(pDX, IDC_BOX81A_QUAL, m_nxeditBox81aQual);
	DDX_Control(pDX, IDC_BOX81B_QUAL, m_nxeditBox81bQual);
	DDX_Control(pDX, IDC_BOX81C_QUAL, m_nxeditBox81cQual);
	DDX_Control(pDX, IDC_BOX76_QUAL, m_nxeditBox76Qual);
	DDX_Control(pDX, IDC_BOX78_QUAL, m_nxeditBox78Qual);
	DDX_Control(pDX, IDC_BOX77_QUAL, m_nxeditBox77Qual);
	DDX_Control(pDX, IDC_BOX51_LABEL, m_nxstaticBox51Label);
	DDX_Control(pDX, IDC_BOX_56_LABEL, m_nxstaticBox56Label);
	DDX_Control(pDX, IDC_DEF_BATCH_LABEL, m_nxstaticDefBatchLabel);
	DDX_Control(pDX, IDC_DEF_UB92_BATCH_NOT_PRIMARY_LABEL, m_nxstaticDefUb92BatchNotPrimaryLabel);
	DDX_Control(pDX, IDC_BOX2_LABEL, m_nxstaticBox2Label);
	DDX_Control(pDX, IDC_BOX_80_LABEL, m_nxstaticBox80Label);
	DDX_Control(pDX, IDC_BOX8_LABEL, m_nxstaticBox8Label);
	DDX_Control(pDX, IDC_BOX10_LABEL, m_nxstaticBox10Label);
	DDX_Control(pDX, IDC_BOX18_LABEL, m_nxstaticBox18Label);
	DDX_Control(pDX, IDC_BOX19_LABEL, m_nxstaticBox19Label);
	DDX_Control(pDX, IDC_BOX20_LABEL, m_nxstaticBox20Label);
	DDX_Control(pDX, IDC_BOX22_LABEL, m_nxstaticBox22Label);
	DDX_Control(pDX, IDC_BOX32_LABEL, m_nxstaticBox32Label);
	DDX_Control(pDX, IDC_BOX79_LABEL, m_nxstaticBox79Label);
	DDX_Control(pDX, IDC_BOX66_LABEL, m_nxstaticBox66Label);
	DDX_Control(pDX, IDC_BOX_81_LABEL, m_nxstaticBox81Label);
	DDX_Control(pDX, IDC_LABEL_BOX_81A, m_nxstaticLabelBox81A);
	DDX_Control(pDX, IDC_LABEL_BOX_81B, m_nxstaticLabelBox81B);
	DDX_Control(pDX, IDC_LABEL_BOX_81C, m_nxstaticLabelBox81C);
	DDX_Control(pDX, IDC_BOX82_LABEL, m_nxstaticBox82Label);
	DDX_Control(pDX, IDC_BOX77_LABEL, m_nxstaticBox77Label);
	DDX_Control(pDX, IDC_BOX83_LABEL, m_nxstaticBox83Label);
	DDX_Control(pDX, IDC_82_83_FORMAT_LABEL, m_nxstatic8283FormatLabel);
	DDX_Control(pDX, IDC_BOX_82_83_LABEL, m_nxstaticBox8283Label);
	DDX_Control(pDX, IDC_CHECK_USE_ANSI_INS_REL_CODES_BOX59, m_checkUseAnsiInsRelCodesBox59);
	DDX_Control(pDX, IDC_BOX_74_QUAL_LABEL, m_nxstaticBox74QualLabel);
	DDX_Control(pDX, IDC_CHECK_DONT_BATCH_SECONDARY, m_checkDontBatchSecondary);
	DDX_Control(pDX, IDC_BTN_EDIT_SECONDARY_EXCLUSIONS, m_btnSecondaryExclusions);
	DDX_Control(pDX, IDC_FILL_BOX_16, m_checkBox16);
	DDX_Control(pDX, IDC_FILL_BOX_12_13, m_checkBox12_13);
	DDX_Control(pDX, IDC_BOX39_LABEL, m_nxstaticBox39Label);
	DDX_Control(pDX, IDC_EDIT_BOX39_CODE, m_nxeditBox39Code);
	DDX_Control(pDX, IDC_EDIT_BOX39_VALUE, m_nxeditBox39Value);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUB92Setup, CNxDialog)
	//{{AFX_MSG_MAP(CUB92Setup)
	ON_BN_CLICKED(IDC_ADD_COMPANY, OnAddCompany)
	ON_BN_CLICKED(IDC_ADD_ALL_COMPANIES, OnAddAllCompanies)
	ON_BN_CLICKED(IDC_REMOVE_COMPANY, OnRemoveCompany)
	ON_BN_CLICKED(IDC_REMOVE_ALL_COMPANIES, OnRemoveAllCompanies)
	ON_BN_CLICKED(IDC_NEW_UB92_GROUP, OnNewUb92Group)
	ON_BN_CLICKED(IDC_DELETE_UB92_GROUP, OnDeleteUb92Group)
	ON_BN_CLICKED(IDC_EDIT_INS_INFO, OnEditInsInfo)
	ON_BN_CLICKED(IDC_ALIGN_FORM, OnAlignForm)
	ON_BN_CLICKED(IDC_EDIT_UB92_CATEGORIES, OnEditCategories)
	ON_BN_CLICKED(IDC_85_CHECK, On85Check)
	ON_BN_CLICKED(IDC_GROUP_CHARGES_CHECK, OnGroupChargesCheck)
	ON_BN_CLICKED(IDC_CHECK_SHOW_PHONE_NUMBER, OnCheckShowPhoneNumber)
	ON_BN_CLICKED(IDC_CHECK_SHOW_TOTAL_AMOUNT, OnCheckShowTotalAmount)
	ON_BN_CLICKED(IDC_CHECK_SHOW_INSCO_ADDRESS, OnCheckShowInscoAddress)
	ON_BN_CLICKED(IDC_CHECK_SHOW_INS_ADDR_BOX_50, OnCheckShowInsAddrBox50)
	ON_BN_CLICKED(IDC_CHECK_PUNCTUATE_CHARGE_LINES, OnCheckPunctuateChargeLines)
	ON_BN_CLICKED(IDC_CHECK_PUNCTUATE_DIAGS, OnCheckPunctuateDiags)
	ON_BN_CLICKED(IDC_76_CHECK, On76Check)
	ON_BN_CLICKED(IDC_CHECK_FILL_42_WITH_44, OnCheckFill42With44)
	ON_BN_CLICKED(IDC_CHECK_SHOW_APPLIES, OnCheckShowApplies)
	ON_BN_CLICKED(IDC_BTN_UB92_DATES, OnBtnUb92Dates)
	ON_BN_CLICKED(IDC_CHECK_SHOW_EMPLOYER_COMPANY, OnCheckShowEmployerCompany)
	ON_BN_CLICKED(IDC_ADV_EBILLING_SETUP, OnAdvEbillingSetup)
	ON_BN_CLICKED(IDC_64_CHECK, On64Check)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BOX_51_CHECK, OnBox51Check)
	ON_BN_CLICKED(IDC_BOX_57_CHECK, OnBox57Check)
	ON_BN_CLICKED(IDC_CHECK_BOX_42_LINE_23, OnCheckBox42Line23)
	ON_BN_CLICKED(IDC_CHECK_USE_THREE, OnCheckUseThree)
	ON_BN_CLICKED(IDC_CHECK_USE_ICD9_PROCEDURE_CODES, OnCheckUseIcd9ProcedureCodes)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_BTN_EXPAND_CODES, OnBtnExpandCodes)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_USE_ANSI_INS_REL_CODES_BOX59, OnBnClickedCheckUseAnsiInsRelCodesBox59)
	ON_BN_CLICKED(IDC_CHECK_DONT_BATCH_SECONDARY, OnCheckDontBatchSecondary)
	ON_BN_CLICKED(IDC_BTN_EDIT_SECONDARY_EXCLUSIONS, OnBtnEditSecondaryExclusions)
	ON_BN_CLICKED(IDC_FILL_BOX_12_13, OnCheckFill12_13)
	ON_BN_CLICKED(IDC_FILL_BOX_16, OnCheckFill16)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUB92Setup message handlers

BOOL CUB92Setup::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_btnAddOne.AutoSet(NXB_RIGHT);
	m_btnRemOne.AutoSet(NXB_LEFT);
	m_btnAddAll.AutoSet(NXB_RRIGHT);
	m_btnRemAll.AutoSet(NXB_LLEFT);
	// (z.manning, 05/14/2008) - PLID 29566
	m_btnEditCategories.AutoSet(NXB_MODIFY);
	m_btnAdvEbillingSetup.AutoSet(NXB_MODIFY);
	m_btnEditUB92Dates.AutoSet(NXB_MODIFY);
	m_nxibCodesToExpand.AutoSet(NXB_MODIFY);
	m_btnEditInsInfo.AutoSet(NXB_MODIFY);
	m_btnAlignForm.AutoSet(NXB_MODIFY);

	m_btnNewGroup.AutoSet(NXB_NEW);
	m_btnDeleteGroup.AutoSet(NXB_DELETE);

	m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));

	((CNxEdit*)GetDlgItem(IDC_EDIT_BOX51_DEFAULT))->LimitText(50);
	((CNxEdit*)GetDlgItem(IDC_EDIT_BOX4))->LimitText(20);
	((CNxEdit*)GetDlgItem(IDC_EDIT_BOX8))->LimitText(20);
	((CNxEdit*)GetDlgItem(IDC_EDIT_BOX10))->LimitText(20);
	((CNxEdit*)GetDlgItem(IDC_EDIT_BOX18))->LimitText(20);
	((CNxEdit*)GetDlgItem(IDC_EDIT_BOX19))->LimitText(20);
	((CNxEdit*)GetDlgItem(IDC_EDIT_BOX20))->LimitText(20);
	((CNxEdit*)GetDlgItem(IDC_EDIT_BOX22))->LimitText(20);
	((CNxEdit*)GetDlgItem(IDC_EDIT_BOX32))->LimitText(20);
	((CNxEdit*)GetDlgItem(IDC_EDIT_BOX79))->LimitText(20);	
	// (a.walling 2007-08-13 09:56) - PLID 26219
	((CNxEdit*)GetDlgItem(IDC_EDIT_BOX42LINE23))->LimitText(20);
	// (j.jones 2008-05-21 10:26) - PLID 30129 - added UB04 Box 66
	m_nxeditEditBox66.SetLimitText(20);
	// (j.jones 2016-05-06 8:53) - NX-100514 - set a limit for Box 39
	m_nxeditBox39Code.SetLimitText(10);
	m_nxeditBox39Value.SetLimitText(20);

	try {

		g_propManager.CachePropertiesInBulk("CUB92Setup", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'dontshow DontBatchSecondary' " // (j.jones 2013-07-18 09:33) - PLID 41823
			")",
			_Q(GetCurrentUserName()));

		m_pGroups = BindNxDataListCtrl(IDC_UB92_GROUPS);

		m_pGroups->PutCurSel(0);

		m_BatchCombo = BindNxDataListCtrl(IDC_BATCH_COMBO, false);
		m_SecondaryBatchCombo = BindNxDataListCtrl(IDC_SECONDARY_BATCH_COMBO, false);
		m_Box82Combo = BindNxDataListCtrl(IDC_BOX_82_COMBO, false);
		m_Box77Combo = BindNxDataList2Ctrl(IDC_BOX_77_COMBO, false);
		m_Box83Combo = BindNxDataListCtrl(IDC_BOX_83_COMBO, false);
		m_Box82Number = BindNxDataListCtrl(IDC_BOX_82_NUMBER, false);
		m_Box77Number = BindNxDataList2Ctrl(IDC_BOX_77_NUMBER, false);
		m_Box83Number = BindNxDataListCtrl(IDC_BOX_83_NUMBER, false);
		m_8283Format = BindNxDataListCtrl(IDC_8283_FORMAT, false);
		m_Box53Accepted = BindNxDataListCtrl(IDC_BOX_53_ACCEPTED, false);
		m_pFormTypes = BindNxDataList2Ctrl(IDC_UB_FORM_TYPE, false);
		m_Box56NPICombo = BindNxDataList2Ctrl(IDC_BOX_56_NPI, false);
		m_Box81aCombo = BindNxDataList2Ctrl(IDC_BOX81A_TAXONOMY_LIST, false);
		m_Box81bCombo = BindNxDataList2Ctrl(IDC_BOX81B_TAXONOMY_LIST, false);
		m_Box81cCombo = BindNxDataList2Ctrl(IDC_BOX81C_TAXONOMY_LIST, false);
		// (a.walling 2007-06-05 12:16) - PLID 26228
		// (a.walling 2007-08-17 14:14) - PLID 27092 - Multiple codes now
		// m_RevCodeExpand = BindNxDataList2Ctrl(IDC_LIST_EXPAND_REVCODE, true);
		// (j.jones 2007-07-11 15:46) - PLID 26621 - added Box 2 setup
		m_Box2Combo = BindNxDataList2Ctrl(IDC_BOX2_OPTIONS, false);
		// (j.jones 2007-07-11 15:46) - PLID 26625 - added new Box 1 setup (UB04 only)
		m_Box1Combo = BindNxDataList2Ctrl(IDC_BOX1_OPTIONS, false);

		// (j.jones 2007-07-11 17:21) - PLID 26621 - added Box 2 setup
		NXDATALIST2Lib::IRowSettingsPtr pRow2 = m_Box2Combo->GetNewRow();
		pRow2->PutValue(0,(long)-1);
		pRow2->PutValue(1,_bstr_t("<Do Not Fill>"));
		m_Box2Combo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box2Combo->GetNewRow();
		pRow2->PutValue(0,(long)1);
		pRow2->PutValue(1,_bstr_t("Show Bill Location"));
		m_Box2Combo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box2Combo->GetNewRow();
		pRow2->PutValue(0,(long)2);
		pRow2->PutValue(1,_bstr_t("Show Place Of Service"));
		m_Box2Combo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box2Combo->GetNewRow();
		pRow2->PutValue(0,(long)3);
		pRow2->PutValue(1,_bstr_t("Show Bill Provider (w/Location Address)"));
		m_Box2Combo->AddRowAtEnd(pRow2, NULL);

		// (j.jones 2007-07-12 10:47) - PLID 26625 - added new Box 1 setup (UB04 only)
		pRow2 = m_Box1Combo->GetNewRow();
		pRow2->PutValue(0,(long)-1);
		pRow2->PutValue(1,_bstr_t("<Do Not Fill>"));
		m_Box1Combo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box1Combo->GetNewRow();
		pRow2->PutValue(0,(long)1);
		pRow2->PutValue(1,_bstr_t("Show Bill Location"));
		m_Box1Combo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box1Combo->GetNewRow();
		pRow2->PutValue(0,(long)2);
		pRow2->PutValue(1,_bstr_t("Show Place Of Service"));
		m_Box1Combo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box1Combo->GetNewRow();
		pRow2->PutValue(0,(long)3);
		pRow2->PutValue(1,_bstr_t("Show Bill Provider (w/Location Address)"));
		m_Box1Combo->AddRowAtEnd(pRow2, NULL);

		//Box 82
		IRowSettingsPtr pRow = m_Box82Combo->GetRow(-1);
		pRow->PutValue(0,(long)1);
		pRow->PutValue(1,_bstr_t("Use Bill Provider"));
		m_Box82Combo->AddRow(pRow);
		pRow = m_Box82Combo->GetRow(-1);
		pRow->PutValue(0,(long)2);
		pRow->PutValue(1,_bstr_t("Use General 1 Provider"));
		m_Box82Combo->AddRow(pRow);
		pRow = m_Box82Combo->GetRow(-1);
		pRow->PutValue(0,(long)3);
		pRow->PutValue(1,_bstr_t("Use Referring Physician"));
		m_Box82Combo->AddRow(pRow);

		//TES 3/19/2007 - PLID 25235 - Box 77
		pRow2 = m_Box77Combo->GetNewRow();
		//TES 3/26/2007 - PLID 25335 - Added a {Do Not Fill} option.
		pRow2->PutValue(0,(long)0);
		pRow2->PutValue(1,_bstr_t("{Do Not Fill}"));
		m_Box77Combo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Combo->GetNewRow();
		pRow2->PutValue(0,(long)1);
		pRow2->PutValue(1,_bstr_t("Use Bill Provider"));
		m_Box77Combo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Combo->GetNewRow();
		pRow2->PutValue(0,(long)2);
		pRow2->PutValue(1,_bstr_t("Use General 1 Provider"));
		m_Box77Combo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Combo->GetNewRow();
		pRow2->PutValue(0,(long)3);
		pRow2->PutValue(1,_bstr_t("Use Referring Physician"));
		m_Box77Combo->AddRowAtEnd(pRow2, NULL);

		//Box 83
		pRow = m_Box83Combo->GetRow(-1);
		pRow->PutValue(0,(long)0);
		pRow->PutValue(1,_bstr_t("{Do Not Fill}"));
		m_Box83Combo->AddRow(pRow);
		pRow = m_Box83Combo->GetRow(-1);
		pRow->PutValue(0,(long)1);
		pRow->PutValue(1,_bstr_t("Use Bill Provider"));
		m_Box83Combo->AddRow(pRow);
		pRow = m_Box83Combo->GetRow(-1);
		pRow->PutValue(0,(long)2);
		pRow->PutValue(1,_bstr_t("Use General 1 Provider"));
		m_Box83Combo->AddRow(pRow);
		pRow = m_Box83Combo->GetRow(-1);
		pRow->PutValue(0,(long)3);
		pRow->PutValue(1,_bstr_t("Use Referring Physician"));
		m_Box83Combo->AddRow(pRow);

		//82/83 Format
		pRow = m_8283Format->GetRow(-1);
		pRow->PutValue(0,(long)1);
		pRow->PutValue(1,_bstr_t("Line A: ID, Line B: Name"));
		m_8283Format->AddRow(pRow);
		pRow = m_8283Format->GetRow(-1);
		pRow->PutValue(0,(long)2);
		pRow->PutValue(1,_bstr_t("Line A: Blank, Line B: ID, Name"));
		m_8283Format->AddRow(pRow);
		pRow = m_8283Format->GetRow(-1);
		pRow->PutValue(0,(long)3);
		pRow->PutValue(1,_bstr_t("Line A: ID, Line B: Blank"));
		m_8283Format->AddRow(pRow);

		//Default Batch
		pRow = m_BatchCombo->GetRow(-1);
		pRow->PutValue(0,_bstr_t("Paper"));
		m_BatchCombo->AddRow(pRow);
		pRow = m_BatchCombo->GetRow(-1);
		pRow->PutValue(0,_bstr_t("Electronic"));
		m_BatchCombo->AddRow(pRow);
		pRow = m_BatchCombo->GetRow(-1);
		pRow->PutValue(0,_bstr_t("None"));
		m_BatchCombo->AddRow(pRow);

		pRow = m_SecondaryBatchCombo->GetRow(-1);
		pRow->PutValue(0,_bstr_t("Paper"));
		m_SecondaryBatchCombo->AddRow(pRow);
		pRow = m_BatchCombo->GetRow(-1);
		pRow->PutValue(0,_bstr_t("Electronic"));
		m_SecondaryBatchCombo->AddRow(pRow);
		pRow = m_BatchCombo->GetRow(-1);
		pRow->PutValue(0,_bstr_t("None"));
		m_SecondaryBatchCombo->AddRow(pRow);

		// Box82Num (1 - UPIN, 2 - SSN, 3 - EIN, 4 - Medicare, 5 - Medicaid, 6 - BCBS, 7 - DEA, 8 - Workers Comp, 9 - OtherID, 10 - Box51, 11 - GroupID, 12 - NPI)

		// (j.jones 2008-05-21 15:42) - PLID 30139 - added option to "Do Not Fill"

		//Box 82 Number
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,_bstr_t(" { Do Not Fill } "));
		m_Box82Number->AddRow(pRow);
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)1);
		pRow->PutValue(1,_bstr_t("UPIN"));
		m_Box82Number->AddRow(pRow);
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)2);
		pRow->PutValue(1,_bstr_t("Social Security Number"));
		m_Box82Number->AddRow(pRow);
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)3);
		pRow->PutValue(1,_bstr_t("Fed Employer ID"));
		m_Box82Number->AddRow(pRow);
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)4);
		pRow->PutValue(1,_bstr_t("Medicare Number"));
		m_Box82Number->AddRow(pRow);
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)5);
		pRow->PutValue(1,_bstr_t("Medicaid Number"));
		m_Box82Number->AddRow(pRow);
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)6);
		pRow->PutValue(1,_bstr_t("BCBS Number"));
		m_Box82Number->AddRow(pRow);
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)7);
		pRow->PutValue(1,_bstr_t("DEA Number"));
		m_Box82Number->AddRow(pRow);
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)8);
		pRow->PutValue(1,_bstr_t("Workers Comp. Number"));
		m_Box82Number->AddRow(pRow);
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)9);
		pRow->PutValue(1,_bstr_t("Other ID Number"));
		m_Box82Number->AddRow(pRow);
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)10);
		pRow->PutValue(1,_bstr_t("Box 51 Number"));
		m_Box82Number->AddRow(pRow);
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)11);
		pRow->PutValue(1,_bstr_t("Group Number"));
		m_Box82Number->AddRow(pRow);
		pRow = m_Box82Number->GetRow(-1);
		pRow->PutValue(0,(long)12);
		pRow->PutValue(1,_bstr_t("NPI"));
		m_Box82Number->AddRow(pRow);

		// (j.jones 2008-05-21 15:42) - PLID 30139 - added option to "Do Not Fill"

		//TES 3/19/2007 - PLID 25235 - Box 77 Number
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)-1);
		pRow2->PutValue(1,_bstr_t(" { Do Not Fill } "));
		m_Box77Number->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)1);
		pRow2->PutValue(1,_bstr_t("UPIN"));
		m_Box77Number->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)2);
		pRow2->PutValue(1,_bstr_t("Social Security Number"));
		m_Box77Number->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)3);
		pRow2->PutValue(1,_bstr_t("Fed Employer ID"));
		m_Box77Number->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)4);
		pRow2->PutValue(1,_bstr_t("Medicare Number"));
		m_Box77Number->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)5);
		pRow2->PutValue(1,_bstr_t("Medicaid Number"));
		m_Box77Number->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)6);
		pRow2->PutValue(1,_bstr_t("BCBS Number"));
		m_Box77Number->AddRowAtEnd(pRow2,	 NULL);
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)7);
		pRow2->PutValue(1,_bstr_t("DEA Number"));
		m_Box77Number->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)8);
		pRow2->PutValue(1,_bstr_t("Workers Comp. Number"));
		m_Box77Number->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)9);
		pRow2->PutValue(1,_bstr_t("Other ID Number"));
		m_Box77Number->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)10);
		pRow2->PutValue(1,_bstr_t("Other Prv ID"));
		m_Box77Number->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)11);
		pRow2->PutValue(1,_bstr_t("Group Number"));
		m_Box77Number->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box77Number->GetNewRow();
		pRow2->PutValue(0,(long)12);
		pRow2->PutValue(1,_bstr_t("NPI"));
		m_Box77Number->AddRowAtEnd(pRow2, NULL);

		// (j.jones 2008-05-21 15:42) - PLID 30139 - added option to "Do Not Fill"

		//Box 83 Number
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,_bstr_t(" { Do Not Fill } "));
		m_Box83Number->AddRow(pRow);
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)1);
		pRow->PutValue(1,_bstr_t("UPIN"));
		m_Box83Number->AddRow(pRow);
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)2);
		pRow->PutValue(1,_bstr_t("Social Security Number"));
		m_Box83Number->AddRow(pRow);
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)3);
		pRow->PutValue(1,_bstr_t("Fed Employer ID"));
		m_Box83Number->AddRow(pRow);
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)4);
		pRow->PutValue(1,_bstr_t("Medicare Number"));
		m_Box83Number->AddRow(pRow);
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)5);
		pRow->PutValue(1,_bstr_t("Medicaid Number"));
		m_Box83Number->AddRow(pRow);
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)6);
		pRow->PutValue(1,_bstr_t("BCBS Number"));
		m_Box83Number->AddRow(pRow);
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)7);
		pRow->PutValue(1,_bstr_t("DEA Number"));
		m_Box83Number->AddRow(pRow);
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)8);
		pRow->PutValue(1,_bstr_t("Workers Comp. Number"));
		m_Box83Number->AddRow(pRow);
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)9);
		pRow->PutValue(1,_bstr_t("Other ID Number"));
		m_Box83Number->AddRow(pRow);
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)10);
		pRow->PutValue(1,_bstr_t("Box 51 Number"));
		m_Box83Number->AddRow(pRow);
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)11);
		pRow->PutValue(1,_bstr_t("Group Number"));
		m_Box83Number->AddRow(pRow);
		pRow = m_Box83Number->GetRow(-1);
		pRow->PutValue(0,(long)12);
		pRow->PutValue(1,_bstr_t("NPI"));
		m_Box83Number->AddRow(pRow);

		//Box 53 Accepted		
		pRow = m_Box53Accepted->GetRow(-1);
		pRow->PutValue(0,(long)0);
		pRow->PutValue(1,_bstr_t("Use Accepted Status"));
		m_Box53Accepted->AddRow(pRow);
		pRow = m_Box53Accepted->GetRow(-1);
		pRow->PutValue(0,(long)1);
		pRow->PutValue(1,_bstr_t("Always Yes"));
		m_Box53Accepted->AddRow(pRow);
		pRow = m_Box53Accepted->GetRow(-1);
		pRow->PutValue(0,(long)2);
		pRow->PutValue(1,_bstr_t("Always No"));
		m_Box53Accepted->AddRow(pRow);
			
		m_pSelected = BindNxDataListCtrl(IDC_UB92_SELECTED,false);
		CString strWhere;
		if(m_pGroups->GetCurSel()!=-1) {
			strWhere.Format("UB92SetupGroupID = %li", m_pGroups->GetValue(m_pGroups->CurSel,0).lVal);
			m_pSelected->WhereClause = (LPCTSTR)strWhere;
			m_pSelected->Requery();
		}

		m_pUnselected = BindNxDataListCtrl(IDC_UB92_UNSELECTED);

		// (j.jones 2007-03-20 13:00) - PLID 25278 - supported UB04 Box 56 NPI
		pRow2 = m_Box56NPICombo->GetNewRow();
		pRow2->PutValue(0, (long)1);
		pRow2->PutValue(1, _bstr_t("Use Bill Provider NPI"));
		m_Box56NPICombo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box56NPICombo->GetNewRow();
		pRow2->PutValue(0, (long)2);
		pRow2->PutValue(1, _bstr_t("Use G1 Provider NPI"));
		m_Box56NPICombo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box56NPICombo->GetNewRow();
		pRow2->PutValue(0, (long)3);
		pRow2->PutValue(1, _bstr_t("Use Bill Location NPI"));
		m_Box56NPICombo->AddRowAtEnd(pRow2, NULL);

		// (j.jones 2007-03-21 09:43) - PLID 25279 - supported UB04 Box 81 Taxonomy Codes
		pRow2 = m_Box81aCombo->GetNewRow();
		pRow2->PutValue(0, (long)-1);
		pRow2->PutValue(1, _bstr_t("< Do Not Fill >"));
		m_Box81aCombo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box81aCombo->GetNewRow();
		pRow2->PutValue(0, (long)1);
		pRow2->PutValue(1, _bstr_t("Use Bill Provider"));
		m_Box81aCombo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box81aCombo->GetNewRow();
		pRow2->PutValue(0, (long)2);
		pRow2->PutValue(1, _bstr_t("Use G1 Provider"));
		m_Box81aCombo->AddRowAtEnd(pRow2, NULL);
		// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore
		/*
		pRow2 = m_Box81aCombo->GetNewRow();
		pRow2->PutValue(0, (long)3);
		pRow2->PutValue(1, _bstr_t("Use Referring Physician"));
		m_Box81aCombo->AddRowAtEnd(pRow2, NULL);
		*/

		pRow2 = m_Box81bCombo->GetNewRow();
		pRow2->PutValue(0, (long)-1);
		pRow2->PutValue(1, _bstr_t("< Do Not Fill >"));
		m_Box81bCombo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box81bCombo->GetNewRow();
		pRow2->PutValue(0, (long)1);
		pRow2->PutValue(1, _bstr_t("Use Bill Provider"));
		m_Box81bCombo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box81bCombo->GetNewRow();
		pRow2->PutValue(0, (long)2);
		pRow2->PutValue(1, _bstr_t("Use G1 Provider"));
		m_Box81bCombo->AddRowAtEnd(pRow2, NULL);
		// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore
		/*
		pRow2 = m_Box81bCombo->GetNewRow();
		pRow2->PutValue(0, (long)3);
		pRow2->PutValue(1, _bstr_t("Use Referring Physician"));
		m_Box81bCombo->AddRowAtEnd(pRow2, NULL);
		*/

		pRow2 = m_Box81cCombo->GetNewRow();
		pRow2->PutValue(0, (long)-1);
		pRow2->PutValue(1, _bstr_t("< Do Not Fill >"));
		m_Box81cCombo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box81cCombo->GetNewRow();
		pRow2->PutValue(0, (long)1);
		pRow2->PutValue(1, _bstr_t("Use Bill Provider"));
		m_Box81cCombo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box81cCombo->GetNewRow();
		pRow2->PutValue(0, (long)2);
		pRow2->PutValue(1, _bstr_t("Use G1 Provider"));
		m_Box81cCombo->AddRowAtEnd(pRow2, NULL);
		// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore
		/*
		pRow2 = m_Box81cCombo->GetNewRow();
		pRow2->PutValue(0, (long)3);
		pRow2->PutValue(1, _bstr_t("Use Referring Physician"));
		m_Box81cCombo->AddRowAtEnd(pRow2, NULL);
		*/

		// (j.jones 2012-09-05 14:37) - PLID 52191 - added Box74Qual
		m_Box74QualCombo = BindNxDataList2Ctrl(IDC_BOX_74_QUAL_COMBO, false);
		pRow2 = m_Box74QualCombo->GetNewRow();
		pRow2->PutValue(0, (long)0);
		pRow2->PutValue(1, _bstr_t(" <Use Default Qualifier>"));
		m_Box74QualCombo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box74QualCombo->GetNewRow();
		pRow2->PutValue(0, (long)1);
		pRow2->PutValue(1, _bstr_t("CAH"));
		m_Box74QualCombo->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_Box74QualCombo->GetNewRow();
		pRow2->PutValue(0, (long)2);
		pRow2->PutValue(1, _bstr_t("BR"));
		m_Box74QualCombo->AddRowAtEnd(pRow2, NULL);

		//TES 3/12/2007 - PLID 24993 - The different UB forms.
		pRow2 = m_pFormTypes->GetNewRow();
		pRow2->PutValue(0, (long)0);
		pRow2->PutValue(1, _bstr_t("UB92"));
		m_pFormTypes->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_pFormTypes->GetNewRow();
		pRow2->PutValue(0, (long)1);
		pRow2->PutValue(1, _bstr_t("UB04"));
		m_pFormTypes->AddRowAtEnd(pRow2, NULL);

		//TES 3/12/2007 - PLID 24993 - Now, load our preference.
		m_pFormTypes->SetSelByColumn(0, (long)GetUBFormType());
		if(m_pFormTypes->CurSel == NULL) {
			//This shouldn't be possible!
			ASSERT(FALSE);
			m_pFormTypes->CurSel = m_pFormTypes->GetFirstRow();
		}
		ReflectFormType();

	}NxCatchAll("Error in OnInitDialog()");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CUB92Setup, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CUB92Setup)
	ON_EVENT(CUB92Setup, IDC_UB92_GROUPS, 16 /* SelChosen */, OnSelChosenUb92Groups, VTS_I4)
	ON_EVENT(CUB92Setup, IDC_UB92_UNSELECTED, 3 /* DblClickCell */, OnDblClickCellUb92Unselected, VTS_I4 VTS_I2)
	ON_EVENT(CUB92Setup, IDC_UB92_SELECTED, 3 /* DblClickCell */, OnDblClickCellUb92Selected, VTS_I4 VTS_I2)
	ON_EVENT(CUB92Setup, IDC_BATCH_COMBO, 16 /* SelChosen */, OnSelChosenBatchCombo, VTS_I4)
	ON_EVENT(CUB92Setup, IDC_SECONDARY_BATCH_COMBO, 16 /* SelChosen */, OnSelChosenSecondaryBatchCombo, VTS_I4)
	ON_EVENT(CUB92Setup, IDC_BOX_82_COMBO, 16 /* SelChosen */, OnSelChosenBox82Combo, VTS_I4)
	ON_EVENT(CUB92Setup, IDC_BOX_83_COMBO, 16 /* SelChosen */, OnSelChosenBox83Combo, VTS_I4)
	ON_EVENT(CUB92Setup, IDC_8283_FORMAT, 16 /* SelChosen */, OnSelChosen8283Format, VTS_I4)
	ON_EVENT(CUB92Setup, IDC_BOX_82_NUMBER, 16 /* SelChosen */, OnSelChosenBox82Number, VTS_I4)
	ON_EVENT(CUB92Setup, IDC_BOX_83_NUMBER, 16 /* SelChosen */, OnSelChosenBox83Number, VTS_I4)
	ON_EVENT(CUB92Setup, IDC_BOX_53_ACCEPTED, 16 /* SelChosen */, OnSelChosenBox53Accepted, VTS_I4)
	ON_EVENT(CUB92Setup, IDC_UB_FORM_TYPE, 1 /* SelChanging */, OnSelChangingUbFormType, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CUB92Setup, IDC_UB_FORM_TYPE, 16 /* SelChosen */, OnSelChosenUbFormType, VTS_DISPATCH)
	ON_EVENT(CUB92Setup, IDC_BOX_77_NUMBER, 16 /* SelChosen */, OnSelChosenBox77Number, VTS_DISPATCH)
	ON_EVENT(CUB92Setup, IDC_BOX_77_COMBO, 16 /* SelChosen */, OnSelChosenBox77Combo, VTS_DISPATCH)
	ON_EVENT(CUB92Setup, IDC_BOX_56_NPI, 16 /* SelChosen */, OnSelChosenBox56Npi, VTS_DISPATCH)
	ON_EVENT(CUB92Setup, IDC_BOX81A_TAXONOMY_LIST, 16 /* SelChosen */, OnSelChosenBox81aTaxonomyList, VTS_DISPATCH)
	ON_EVENT(CUB92Setup, IDC_BOX81B_TAXONOMY_LIST, 16 /* SelChosen */, OnSelChosenBox81bTaxonomyList, VTS_DISPATCH)
	ON_EVENT(CUB92Setup, IDC_BOX81C_TAXONOMY_LIST, 16 /* SelChosen */, OnSelChosenBox81cTaxonomyList, VTS_DISPATCH)
	ON_EVENT(CUB92Setup, IDC_BOX2_OPTIONS, 16 /* SelChosen */, OnSelChosenBox2Options, VTS_DISPATCH)
	ON_EVENT(CUB92Setup, IDC_BOX1_OPTIONS, 16 /* SelChosen */, OnSelChosenBox1Options, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CUB92Setup, IDC_BOX_74_QUAL_COMBO, 1, OnSelChangingBox74QualCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CUB92Setup, IDC_BOX_74_QUAL_COMBO, 16, OnSelChosenBox74QualCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CUB92Setup::OnSelChosenUb92Groups(long nRow) 
{
	Load();
}

void CUB92Setup::OnDblClickCellUb92Unselected(long nRowIndex, short nColIndex) 
{
	OnAddCompany();
}

void CUB92Setup::OnDblClickCellUb92Selected(long nRowIndex, short nColIndex) 
{
	OnRemoveCompany();
}

void CUB92Setup::OnAddCompany() 
{
	CWaitCursor pWait;

	try{
		if(m_pGroups->CurSel==-1 || m_pUnselected->CurSel == -1)
			return;
		
		long i = 0;
		long p = m_pUnselected->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{	i++;
			m_pUnselected->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			ExecuteSql("UPDATE InsuranceCoT SET UB92SetupGroupID = %li WHERE InsuranceCoT.PersonID = %li;", m_pGroups->GetValue(m_pGroups->CurSel,0).lVal, pRow->GetValue(0).lVal);

			//audit
			long nAuditID = BeginNewAuditEvent();
			//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
			AuditEvent(-1, CString(pRow->GetValue(1).bstrVal), nAuditID, aeiInsCoUB92, long(pRow->GetValue(0).lVal), "<No UB Group>", CString(m_pGroups->GetValue(m_pGroups->CurSel,1).bstrVal), aepMedium, aetChanged);

			pDisp->Release();
		}

		m_pSelected->TakeCurrentRow(m_pUnselected);

	}NxCatchAll("Error in OnAddCompany()");
}

void CUB92Setup::OnAddAllCompanies() 
{
	try{
		if(m_pGroups->CurSel==-1)
			return;

		ExecuteSql("UPDATE InsuranceCoT SET UB92SetupGroupID = %li WHERE (UB92SetupGroupID = 0 OR UB92SetupGroupID Is Null) AND PersonID NOT IN (SELECT ID FROM PersonT WHERE Archived = 1)", m_pGroups->GetValue(m_pGroups->CurSel,0).lVal);

		//audit each row
		long nAuditID = BeginNewAuditEvent();
		for(int i=0; i<m_pUnselected->GetRowCount();i++) {
			//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
			AuditEvent(-1, CString(m_pUnselected->GetValue(i, 1).bstrVal), nAuditID, aeiInsCoUB92, long(m_pUnselected->GetValue(i, 0).lVal), "<No UB Group>", CString(m_pGroups->GetValue(m_pGroups->CurSel,1).bstrVal), aepMedium, aetChanged);
		}

		m_pSelected->TakeAllRows(m_pUnselected);

	}NxCatchAll("Error in OnAddAll()");
}

void CUB92Setup::OnRemoveCompany() 
{
	CWaitCursor pWait;

	try{
		if(m_pGroups->CurSel==-1 || m_pSelected->CurSel == -1)
			return;

		long i = 0;
		long p = m_pSelected->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{	i++;
			m_pSelected->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);
			bool bUpdate = true;

			//DRT 7/31/03 - If they are removing an inactive company, warn them that it will no longer be available
			if(ReturnsRecords("SELECT ID FROM PersonT WHERE ID = %li AND Archived = 1", pRow->GetValue(0).lVal)) {
				//it is inactive
				if(MsgBox(MB_YESNO, "The company '%s' is an inactive insurance company.  If you remove it from this group, it will "
					"not be in the unselected list.  Do you wish to continue?", VarString(pRow->GetValue(1))) == IDNO) {
					bUpdate = false;
					
					//need to unselect this row
					pRow->Selected = FALSE;
				}
				else {
					pRow->Selected = FALSE;
					m_pSelected->RemoveRow(m_pSelected->FindByColumn(0, pRow->GetValue(0), 0, VARIANT_FALSE));
				}
			}

			if(bUpdate) {
				ExecuteSql("UPDATE InsuranceCoT SET UB92SetupGroupID = NULL WHERE PersonID = %li;", pRow->GetValue(0).lVal);

				//audit
				long nAuditID = BeginNewAuditEvent();
				//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
				AuditEvent(-1, CString(pRow->GetValue(1).bstrVal), nAuditID, aeiInsCoUB92, long(pRow->GetValue(0).lVal), CString(m_pGroups->GetValue(m_pGroups->CurSel,1).bstrVal), "<No UB Group>", aepMedium, aetChanged);
			}

			pDisp->Release();
		}

		m_pUnselected->TakeCurrentRow(m_pSelected);

	}NxCatchAll("Error in OnRemoveCompany");
}

void CUB92Setup::OnRemoveAllCompanies() 
{
	
	try{
		if(m_pGroups->CurSel==-1)
			return;

		long i = 0;
		long p = m_pSelected->GetFirstRowEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{	i++;
			m_pSelected->GetNextRowEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			ExecuteSql("UPDATE InsuranceCoT SET UB92SetupGroupID = NULL WHERE PersonID = %li;", pRow->GetValue(0).lVal);

			pDisp->Release();
		}

		//audit each row
		long nAuditID = BeginNewAuditEvent();
		for(i=0; i<m_pSelected->GetRowCount();i++) {
			//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
			AuditEvent(-1, CString(m_pSelected->GetValue(i, 1).bstrVal), nAuditID, aeiInsCoUB92, long(m_pSelected->GetValue(i, 0).lVal), CString(m_pGroups->GetValue(m_pGroups->CurSel,1).bstrVal), "<No UB Group>", aepMedium, aetChanged);
		}

		m_pUnselected->TakeAllRows(m_pSelected);

	}NxCatchAll("Error in OnRemoveAllCompanies()");
}

void CUB92Setup::OnNewUb92Group() 
{
	CWaitCursor pWait;

	CString strResult, sql;
	int nResult = InputBoxLimited(this, "Enter Name", strResult, "",100,false,false,NULL);
	if(nResult == IDCANCEL)
		return;

	long id;

	strResult.TrimRight();

	while(strResult == "") {
		AfxMessageBox("Cannot insert a blank group name.  Please enter some text for the name.");
		if(InputBoxLimited(this, "Enter Name", strResult, "",100,false,false,NULL) == IDCANCEL)
			return;
		strResult.TrimRight();
	}
	
	
	if (nResult == IDOK && strResult != "")
	{	
		try {

			// (j.jones 2005-01-28 14:05) - PLID 13829 - the database defaults the 'Group Charges By Revenue Code'
			//to being 1, but we've since decided to default it to 0. I deliberately did not change the data
			//as this may revert back later. But for now, we'll default it to 0.
			// (j.jones 2009-11-24 15:41) - PLID 36411 - default PriorAuthQualifier to G1, it defaults to blank in the database
			// (j.jones 2010-03-02 16:20) - PLID 37584 - PriorAuthQualifier is now blank by default
			ExecuteParamSql("INSERT INTO UB92SetupT (ID, Name, GroupRev) VALUES ({INT}, {STRING}, 0)",
				id = NewNumber("UB92SetupT", "ID"), strResult);

		}NxCatchAll("Error creating new UB Group");

		m_pGroups->Requery();
		m_pGroups->SetSelByColumn(1,_bstr_t(strResult));
		Load();

		m_groupsChecker.Refresh();

		if (IDYES == AfxMessageBox(IDS_HCFA_ADDALL, MB_YESNO | MB_ICONQUESTION))
			OnAddAllCompanies();
	}
}

void CUB92Setup::OnDeleteUb92Group() 
{
	CString sql;

	if(m_pGroups->CurSel==-1)
		return;	

	//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
	if(IDNO==MessageBox("Are you sure you wish to delete this UB Group?","Practice",MB_YESNO|MB_ICONQUESTION))
		return;

	BEGIN_TRANS("DeleteUB92Group") {

		long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;

		//this will audit the changes
		OnRemoveAllCompanies();
		
		ExecuteParamSql("UPDATE InsuranceCoT SET UB92SetupGroupID = NULL WHERE UB92SetupGroupID = {INT}",GroupID);

		// (j.jones 2013-07-18 09:25) - PLID 41823 - delete from UBSecondaryExclusionsT
		ExecuteParamSql("DELETE FROM UBSecondaryExclusionsT WHERE UBSetupID = {INT}", GroupID);

		// (j.jones 2010-05-12 15:42) - PLID 38622 - delete from UB04MultiGroupRevExpandT
		ExecuteParamSql("DELETE FROM UB04MultiGroupRevExpandT WHERE UB92SetupID = {INT}", GroupID);

		// (j.jones 2010-02-03 09:01) - PLID 37159 - delete from UB_EbillingAllowedAdjCodesT
		ExecuteParamSql("DELETE FROM UB_EbillingAllowedAdjCodesT WHERE UBSetupID = {INT}", GroupID);

		// (j.jones 2008-05-12 17:45) - PLID 30023 - ensure we delete from the UB92EbillingSetupT table
		ExecuteParamSql("DELETE FROM UB92EbillingSetupT WHERE SetupGroupID = {INT}",GroupID);

		ExecuteParamSql("DELETE FROM UB92SetupT WHERE ID = {INT}",GroupID);
		
	} END_TRANS_CATCH_ALL("DeleteUB92Group");

	//JJ - this quickly refreshes the lists
	int nCount = m_pSelected->GetRowCount();
	IRowSettingsPtr pRow;
	m_pUnselected->SetRedraw(FALSE);
	m_pSelected->SetRedraw(FALSE);
	for(int i = nCount - 1; i >= 0; i--){
		pRow = m_pSelected->GetRow(i);
		m_pSelected->RemoveRow(i);
		m_pUnselected->AddRow(pRow);
	}
	m_pUnselected->SetRedraw(TRUE);
	m_pSelected->SetRedraw(TRUE);

	m_pGroups->RemoveRow(m_pGroups->CurSel);
	m_pGroups->CurSel = 0;

	Load();

	m_groupsChecker.Refresh();
}

void CUB92Setup::OnEditInsInfo() 
{
	CEditInsInfoDlg EditInsInfo(this);

	if (CheckCurrentUserPermissions(bioInsuranceCo,sptWrite))
	{
		EditInsInfo.DisplayWindow(BACKGROUND_COLOR);
		UpdateView();
	}
}

void CUB92Setup::OnAlignForm() 
{
	CPrintAlignDlg dlg(this);
	dlg.m_FormID = 1;
	dlg.DoModal();
}

void CUB92Setup::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// (z.manning, 5/19/2006, PLID 20726) - We may still have focus on a field that has been changed,
	// so let's kill it's focus to ensure that no changes are lost.
	// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages (now part of CNexTechDialog)
	CheckFocus();

	if (m_companyChecker.Changed())
	{
		m_pUnselected->Requery();
		if(m_pGroups->CurSel!=-1)
			m_pSelected->Requery();
	}

	// (a.walling 2007-08-17 14:28) - PLID 27092 - We use a button now
	/*
	if (m_groupsChecker.Changed())
	{
		_variant_t varTmp;
		if(m_pGroups->CurSel != -1)
			varTmp = m_pGroups->GetValue(m_pGroups->CurSel, 0);

		m_pGroups->Requery();
		
		int nRow = m_pGroups->SetSelByColumn(0, varTmp);		
		if(nRow == -1) {
			m_pGroups->CurSel = 0;
			OnSelChosenUb92Groups(0);
		}
		else {
			OnSelChosenUb92Groups(nRow);
		}
	}
	else {
		m_RevCodeExpand->Requery();
	}
	*/

	Load();
}

void CUB92Setup::Load() 
{
	try
	{
		if(m_pGroups->CurSel==-1)
			return;
		m_loading = true;

		long group = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
		CString strWhere;
		strWhere.Format("UB92SetupGroupID = %li", group);
		m_pSelected->WhereClause = (LPCTSTR)strWhere;
		m_pSelected->Requery();

		CUB92SetupInfo UB92Info(group);

		//Box38
		SetRadio (m_radioBox38InsuredParty, m_radioBox38InsuranceCo, UB92Info.Box38 == 0);
	
		//DefBatch
		switch(UB92Info.DefBatch) {
			case(0):
				m_BatchCombo->PutCurSel(2);
				break;
			// (j.jones 2006-11-15 15:15) - PLID 23558 - this was using a 3, which is obsolete
			case(1):
				m_BatchCombo->PutCurSel(0);
				break;
			case(2):
				m_BatchCombo->PutCurSel(1);
				break;
		}

		//DefBatchSecondary
		switch(UB92Info.DefBatchSecondary) {
			case 0:
				m_SecondaryBatchCombo->PutCurSel(2);
				break;
			case 1:
				m_SecondaryBatchCombo->PutCurSel(0);
				break;
			case 2:
				m_SecondaryBatchCombo->PutCurSel(1);
				break;
		}

		//DateFmt
		SetRadio (m_radioDateYearFirst, m_radioDateMonthFirst, UB92Info.DateFmt == 0);
	
		//Box4
		SetDlgItemText(IDC_EDIT_BOX4,UB92Info.Box4);
	
		//Box79
		SetDlgItemText(IDC_EDIT_BOX79,UB92Info.Box79);
	
		//Box82Setup
		m_Box82Combo->SetSelByColumn(0,UB92Info.Box82Setup);

		//TES 3/19/2007 - PLID 25235 - UB04 Box 77 Setup
		m_Box77Combo->SetSelByColumn(0, UB92Info.UB04Box77Setup);
	
		//Box83Setup
		m_Box83Combo->SetSelByColumn(0,UB92Info.Box83Setup);
	
		//Box8283Format
		m_8283Format->SetSelByColumn(0,UB92Info.Box8283Format);
	
		//Box82Num
		m_Box82Number->SetSelByColumn(0,UB92Info.Box82Num);
	
		//TES 3/19/2007 - PLID 25235 - UB04 Box 77 Number
		m_Box77Number->SetSelByColumn(0, UB92Info.UB04Box77Num);

		//Box83Num
		m_Box83Number->SetSelByColumn(0,UB92Info.Box83Num);
	
		//TES 3/19/2007 - PLID 25235 - UB04 Box 76 Qualifier
		SetDlgItemText(IDC_BOX76_QUAL, UB92Info.UB04Box76Qual);

		// (j.jones 2010-04-16 09:11) - PLID 38149 - track the Box76 qualifier
		m_strOldBox76Qualifier = UB92Info.UB04Box76Qual;

		//TES 3/19/2007 - PLID 25235 - UB04 Box 77 Qualifier
		SetDlgItemText(IDC_BOX77_QUAL, UB92Info.UB04Box77Qual);

		//TES 3/19/2007 - PLID 25235 - UB04 Box 78 Qualifier
		SetDlgItemText(IDC_BOX78_QUAL, UB92Info.UB04Box78Qual);

		//Box64Show
		m_checkBox64.SetCheck(UB92Info.Box64Show);

		//Box76Show
		m_checkBox76.SetCheck(UB92Info.Box76Show);

		//Box85Show
		m_checkBox85.SetCheck(UB92Info.Box85Show);
	
		//GroupRev
		m_checkGroupCharges.SetCheck(UB92Info.GroupRev);

		//Box51
		SetDlgItemText(IDC_EDIT_BOX51_DEFAULT,UB92Info.Box51Default);

		//Box8
		SetDlgItemText(IDC_EDIT_BOX8,UB92Info.Box8);
	
		//Box10
		SetDlgItemText(IDC_EDIT_BOX10,UB92Info.Box10);
	
		//Box18
		SetDlgItemText(IDC_EDIT_BOX18,UB92Info.Box18);
	
		//Box19
		SetDlgItemText(IDC_EDIT_BOX19,UB92Info.Box19);
	
		//Box20
		SetDlgItemText(IDC_EDIT_BOX20,UB92Info.Box20);
	
		//Box22
		SetDlgItemText(IDC_EDIT_BOX22,UB92Info.Box22);

		// (a.walling 2007-08-13 09:56) - PLID 26219
		//Box42, Line 23
		SetDlgItemText(IDC_EDIT_BOX42LINE23,UB92Info.Box42Line23);
	
		//ShowPhone
		m_checkShowPhoneNum.SetCheck(UB92Info.ShowPhone);
	
		//ShowTotals
		m_checkShowTotals.SetCheck(UB92Info.ShowTotals);
	
		//ShowInsAdd
		m_checkShowInscoAdd.SetCheck(UB92Info.ShowInsAdd);
	
		//InsAddr50
		m_checkShowInsAddr50.SetCheck(UB92Info.InsAddr50);

		//Box1Loc
		SetRadio(m_radioBox1Location, m_radioBox1POS, UB92Info.Box1Loc == 0);

		//PunctuateChargeLines
		m_checkPunctuateChargeLines.SetCheck(UB92Info.PunctuateChargeLines);

		//PunctuateDiagCodes
		m_checkPunctuateDiagCodes.SetCheck(UB92Info.PunctuateDiagCodes);

		//Box32
		SetDlgItemText(IDC_EDIT_BOX32,UB92Info.Box32);

		//Fill42With44
		m_checkFill42With44.SetCheck(UB92Info.Fill42With44);

		//ShowApplies
		m_checkShowApplies.SetCheck(UB92Info.ShowApplies);

		//Box80Number
		if(UB92Info.Box80Number == 0) {
			m_radioBox80CPT.SetCheck(TRUE);
			m_radioBox80Diag.SetCheck(FALSE);
			m_radioBox80None.SetCheck(FALSE);

			// (a.walling 2007-06-20 13:51) - PLID 26414 - Only useful if using the service code
			m_checkUseICD9.EnableWindow(TRUE);
			m_checkUseICD9.RedrawWindow();
		}
		else if(UB92Info.Box80Number == 1) {
			m_radioBox80CPT.SetCheck(FALSE);
			m_radioBox80Diag.SetCheck(TRUE);
			m_radioBox80None.SetCheck(FALSE);

			// (a.walling 2007-06-20 13:51) - PLID 26414
			m_checkUseICD9.EnableWindow(FALSE);
		}
		else {
			m_radioBox80CPT.SetCheck(FALSE);
			m_radioBox80Diag.SetCheck(FALSE);
			m_radioBox80None.SetCheck(TRUE);

			// (a.walling 2007-06-20 13:51) - PLID 26414
			m_checkUseICD9.EnableWindow(FALSE);
		}

		// (j.jones 2012-09-05 14:37) - PLID 52191 - added Box74Qual
		NXDATALIST2Lib::IRowSettingsPtr p74QualRow = m_Box74QualCombo->SetSelByColumn(0, (long)UB92Info.Box74Qual);
		if(p74QualRow == NULL) {
			//shouldn't really be possible, but bad data would act as "Use Default", so set that value
			m_Box74QualCombo->SetSelByColumn(0, (long)0);
		}

		//ShowCompanyAsInsurer
		m_checkShowEmployerCompany.SetCheck(UB92Info.ShowCompanyAsInsurer);

		//Box53Accepted
		m_Box53Accepted->SetSelByColumn(0,UB92Info.Box53Accepted);

		//Box5ID
		SetRadio(m_radioBox5LocEIN, m_radioBox5ProvTaxID, UB92Info.Box5ID == 0);

		// (j.jones 2007-03-20 13:03) - PLID 25278 - supported UB04 Box 56 NPI
		//UB04Box56NPI
		m_Box56NPICombo->SetSelByColumn(0, UB92Info.UB04Box56NPI);

		// (j.jones 2007-03-21 10:04) - PLID 25279 - supported UB04 Box 81 Taxonomy Codes
		// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore,
		// in the event that they had selected referring physician before, it will select nothing
		//UB04Box81aQual
		SetDlgItemText(IDC_BOX81A_QUAL, UB92Info.UB04Box81aQual);
		//UB04Box81a
		m_Box81aCombo->SetSelByColumn(0, UB92Info.UB04Box81a);
		//UB04Box81bQual
		SetDlgItemText(IDC_BOX81B_QUAL, UB92Info.UB04Box81bQual);
		//UB04Box81b
		m_Box81bCombo->SetSelByColumn(0, UB92Info.UB04Box81b);
		//UB04Box81cQual
		SetDlgItemText(IDC_BOX81C_QUAL, UB92Info.UB04Box81cQual);
		//UB04Box81c
		m_Box81cCombo->SetSelByColumn(0, UB92Info.UB04Box81c);

		//TES 3/21/2007 - PLID 25295 - Two new checkboxes to fill the "Other Prv ID" in Boxes 51 or 57
		m_checkBox51.SetCheck(UB92Info.UB04Box51Show);
		m_checkBox57.SetCheck(UB92Info.UB04Box57Show);

		// (a.walling 2007-06-05 09:32) - PLID 26219 - checkbox to zero fill page numbers
		m_checkBox42Line23.SetCheck(UB92Info.UB04Box42Line23);
		// (a.walling 2007-08-13 10:38) - PLID 26219 - fill the custom text as well
		SetDlgItemText(IDC_EDIT_BOX42LINE23, UB92Info.Box42Line23);

		// (a.walling 2007-08-13 10:26) - PLID 26219 - default values for Box42:23
		GetDlgItem(IDC_EDIT_BOX42LINE23)->EnableWindow(UB92Info.UB04Box42Line23 ? TRUE : FALSE);			

		// (a.walling 2007-06-05 11:03) - PLID 26223 - Option to only use three lines for ins co
		m_checkUseThree.SetCheck(UB92Info.UB04Box80UseThree);
		if (UB92Info.ShowInsAdd) {
			m_checkUseThree.EnableWindow(TRUE);
		} else {
			m_checkUseThree.EnableWindow(FALSE);
		}
		m_checkUseThree.RedrawWindow();

		// (a.walling 2007-06-05 13:35) - PLID 26228
		// (a.walling 2007-08-17 14:09) - PLID 27092 - We support multiple ones now, so no more dropdown, we'll use a multiselectdlg
		/*
		m_RevCodeExpand->SetSelByColumn(0, UB92Info.UB04GroupRevExpand);
		*/
		if (UB92Info.GroupRev)
			GetDlgItem(IDC_BTN_EXPAND_CODES)->EnableWindow(TRUE);
		else
			GetDlgItem(IDC_BTN_EXPAND_CODES)->EnableWindow(FALSE);

		// (a.walling 2007-06-20 13:32) - PLID 26414
		m_checkUseICD9.SetCheck((UB92Info.UB04UseICD9ProcedureCodes == 1));

		// (j.jones 2007-07-12 08:26) - PLID 26621 - added Box 2 setup
		m_Box2Combo->SetSelByColumn(0, UB92Info.UB04Box2Setup);

		// (j.jones 2007-07-12 10:50) - PLID 26625 - added new Box 1 setup (UB04 only)
		m_Box1Combo->SetSelByColumn(0, UB92Info.UB04Box1Setup);

		// (j.jones 2008-05-21 10:27) - PLID 30129 - added UB04 Box 66
		SetDlgItemText(IDC_EDIT_BOX66, UB92Info.UB04Box66);

		// (j.jones 2012-05-24 10:57) - PLID 50597 - added InsRelANSI
		m_checkUseAnsiInsRelCodesBox59.SetCheck(UB92Info.InsRelANSI == 1);

		// (j.jones 2013-07-18 09:22) - PLID 41823 - added DontBatchSecondary
		m_checkDontBatchSecondary.SetCheck(UB92Info.DontBatchSecondary == 1);
		m_btnSecondaryExclusions.EnableWindow(UB92Info.DontBatchSecondary == 1);

		// (b.spivey July 8, 2015) - PLID 66515 -added FillBox12 and 16 checks. 
		m_checkBox12_13.SetCheck(UB92Info.FillBox12_13);
		m_checkBox16.SetCheck(UB92Info.FillBox16);

		// (j.jones 2016-05-06 8:53) - NX-100514 - added Box 39
		m_nxeditBox39Code.SetWindowText("");
		m_nxeditBox39Value.SetWindowText("");
		if (UB92Info.ub04ClaimInfo.values.size() > 0) {
			auto val = UB92Info.ub04ClaimInfo.values.at(0);
			if (val.code.GetLength() > 0) {
				m_nxeditBox39Code.SetWindowText(val.code);
			}
			if (val.amount.GetStatus() == COleCurrency::valid) {
				m_nxeditBox39Value.SetWindowText(FormatCurrencyForInterface(val.amount));
			}
		}

		m_loading = false;

	}NxCatchAll("Error in Load()");
}

void CUB92Setup::SetRadio (CButton &radioTrue, CButton &radioFalse, const bool isTrue)
{//Eliminates a few lines
	if (isTrue)
	{	radioTrue.SetCheck(true);
		radioFalse.SetCheck(false);
	}
	else 
	{	radioTrue.SetCheck(false);
		radioFalse.SetCheck(true);
	}
}

BOOL CUB92Setup::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD	nID;
	CString box,
			text;
	long	value;
	_RecordsetPtr rs;

	switch (HIWORD(wParam))
	{	case BN_CLICKED:
		{	switch (nID = LOWORD(wParam))
			{
				case IDC_RADIO_BOX38_INSURED_PARTY:
				case IDC_RADIO_DATE_YEARFIRST:
				case IDC_RADIO_BOX1_LOCATION:
				case IDC_RADIO_BOX5_LOC_EIN:
				case IDC_RADIO_BOX_80_CPT:
					value = 0;

					// (a.walling 2007-06-20 13:54) - PLID 26414
					if (nID == IDC_RADIO_BOX_80_CPT) {
						m_checkUseICD9.EnableWindow(TRUE);
						m_checkUseICD9.RedrawWindow();
					}
					break;
				case IDC_RADIO_BOX38_INSURANCE_CO:
				case IDC_RADIO_DATE_MONTHFIRST:
				case IDC_RADIO_BOX1_POS:
				case IDC_RADIO_BOX5_PROV_FEDID:
				case IDC_RADIO_BOX_80_DIAG:
					value = 1;

					// (a.walling 2007-06-20 13:54) - PLID 26414
					if (nID == IDC_RADIO_BOX_80_DIAG) {
						m_checkUseICD9.EnableWindow(FALSE);
					}
					break;
				case IDC_RADIO_BOX_80_NONE:
					// (a.walling 2007-06-20 13:54) - PLID 26414
					m_checkUseICD9.EnableWindow(FALSE);
					value = 2;
					break;
				default: return CNxDialog::OnCommand(wParam, lParam);//handles anything else here
			}
			switch (nID = LOWORD(wParam))
			{
				case IDC_RADIO_BOX38_INSURED_PARTY:
				case IDC_RADIO_BOX38_INSURANCE_CO:
					box = "Box38";
					break;
				case IDC_RADIO_DATE_YEARFIRST:
				case IDC_RADIO_DATE_MONTHFIRST:
					box = "DateFmt";
					break;
				case IDC_RADIO_BOX1_LOCATION:
				case IDC_RADIO_BOX1_POS:
					box = "Box1Loc";
					break;
				case IDC_RADIO_BOX5_LOC_EIN:
				case IDC_RADIO_BOX5_PROV_FEDID:
					box = "Box5ID";
					break;
				case IDC_RADIO_BOX_80_CPT:
				case IDC_RADIO_BOX_80_DIAG:
				case IDC_RADIO_BOX_80_NONE:
					box = "Box80Number";
					break;
			}
			try
			{
				if(m_pGroups->CurSel==-1)
					return CNxDialog::OnCommand(wParam, lParam);

				UpdateTable(box,value);

			}NxCatchAll("Error in OnCommand()");
			break;
		}
		case EN_CHANGE:
			m_changed = true;
			break;
		case EN_KILLFOCUS: {
			switch (nID = LOWORD(wParam))
			{
				case IDC_EDIT_BOX51_DEFAULT:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {

						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET Box51Default = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_EDIT_BOX4:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {

						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET Box4 = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_EDIT_BOX79:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET Box79 = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_EDIT_BOX8:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET Box8 = '%s' WHERE ID = %li", _Q(text), GroupID);
						
					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_EDIT_BOX10:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET Box10 = '%s' WHERE ID = %li", _Q(text), GroupID);
						
					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_EDIT_BOX18:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET Box18 = '%s' WHERE ID = %li", _Q(text), GroupID);
						
					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_EDIT_BOX19:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET Box19 = '%s' WHERE ID = %li", _Q(text), GroupID);
						
					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_EDIT_BOX20:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET Box20 = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_EDIT_BOX22:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET Box22 = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_EDIT_BOX32:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET Box32 = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_EDIT_BOX42LINE23:
					// (a.walling 2007-08-13 10:00) - PLID 26219 - Custom text for box42:23
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET Box42Line23 = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				// (j.jones 2007-03-21 10:25) - PLID 25279 - supported Box 81 Taxonomy qualifiers
				case IDC_BOX81A_QUAL:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET UB04Box81aQual = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_BOX81B_QUAL:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET UB04Box81bQual = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;	
				case IDC_BOX81C_QUAL:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET UB04Box81cQual = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				//TES 3/22/2007 - PLID 25235 - Support qualifiers for Boxes 76-78
				case IDC_BOX76_QUAL:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					text.TrimLeft();
					text.TrimRight();
					SetDlgItemText(nID,text);
					try {

						// (j.jones 2010-04-16 09:01) - PLID 38149 - warn if the qualifier has been changed to XX
						if(m_strOldBox76Qualifier.CompareNoCase(text) != 0 && text.CompareNoCase("XX") == 0) {
							if(IDNO == MessageBox("The Box 76 qualifier is XX, which is not a valid qualifier to use.\n"
								"This configuration may cause your claims to be rejected.\n\n"
								"Are you sure you want to send XX as a Box 76 qualifier?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
								SetDlgItemText(IDC_BOX76_QUAL,m_strOldBox76Qualifier);
								return CNxDialog::OnCommand(wParam, lParam);;
							}
						}
						m_strOldBox76Qualifier = text;
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET UB04Box76Qual = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_BOX77_QUAL:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET UB04Box77Qual = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;	
				case IDC_BOX78_QUAL:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
						
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE UB92SetupT SET UB04Box78Qual = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				// (j.jones 2008-05-21 10:27) - PLID 30129 - added UB04 Box 66
				case IDC_EDIT_BOX66:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(nID,text);
					if (text.IsEmpty())
						text = "";
					try {
					
						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteParamSql("UPDATE UB92SetupT SET UB04Box66 = {STRING} WHERE ID = {INT}", text, GroupID);

					}NxCatchAll("Error in OnCommand() : UB04 Box 66");
				break;
				// (j.jones 2016-05-06 8:53) - NX-100514 - added Box 39
				case IDC_EDIT_BOX39_CODE:
				case IDC_EDIT_BOX39_VALUE:					
					try {

						if (m_pGroups->CurSel == -1)
							return CNxDialog::OnCommand(wParam, lParam);

						CString strBox39Code, strBox39Value;
						GetDlgItemText(IDC_EDIT_BOX39_CODE, strBox39Code);
						GetDlgItemText(IDC_EDIT_BOX39_VALUE, strBox39Value);

						strBox39Code.Trim();
						strBox39Value.Trim();

						COleCurrency cyBox39Value = COleCurrency(0, 0);
						if (!strBox39Value.IsEmpty()) {
							cyBox39Value = ParseCurrencyFromInterface(strBox39Value);
							if (cyBox39Value.GetStatus() == COleCurrency::invalid) {
								MessageBox("Please enter a valid Box 39 value.", NULL, MB_OK | MB_ICONEXCLAMATION);
								SetDlgItemText(IDC_EDIT_BOX39_VALUE, "");
								GetDlgItem(IDC_EDIT_BOX39_VALUE)->SetFocus();
								return CNxDialog::OnCommand(wParam, lParam);
							}
						}

						UB04::ClaimInfo ub04ClaimInfo;
						if (!strBox39Code.IsEmpty() || !strBox39Value.IsEmpty()) {
							ub04ClaimInfo.values.push_back({ strBox39Code, cyBox39Value });
							//re-set the currency field
							m_nxeditBox39Value.SetWindowText(FormatCurrencyForInterface(cyBox39Value));
						}
						else {
							m_nxeditBox39Value.SetWindowText("");
						}

						long GroupID = VarLong(m_pGroups->GetValue(m_pGroups->CurSel, 0));
						ExecuteParamSql("UPDATE UB92SetupT SET UB04ClaimInfo = {STRING} WHERE ID = {INT}", ub04ClaimInfo.ToXml(), GroupID);

					}NxCatchAll("Error in CUB92Setup::OnCommand() : UB04 Box 39");
					break;
				default:
					Save(nID);
					break;
			}
			break;
		}
	}
	return CNxDialog::OnCommand(wParam, lParam);
}

void CUB92Setup::Save (int nID)
{
	if(m_pGroups->CurSel==-1)
		return;

	//nothing to save yet

	m_changed = false;
}

void CUB92Setup::OnEditCategories() 
{
	CUB92Categories dlg(this);
	dlg.DoModal();

	// (a.walling 2007-06-05 16:41) - PLID 26228 - Need to refresh in case some categories were removed or renamed
	// (a.walling 2007-08-17 14:15) - PLID 27092 - no longer
	// UpdateView();
}

void CUB92Setup::OnSelChosenBatchCombo(long nRow) 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;
	_RecordsetPtr rs;
	try {

		long batch = 0;

		if(m_BatchCombo->GetCurSel()==0)
			// (j.jones 2006-11-15 15:15) - PLID 23558 - this was using a 3, which is obsolete
			batch = 1;
		else if(m_BatchCombo->GetCurSel()==1)
			batch = 2;
		else
			batch = 0;

		UpdateTable("DefBatch",batch);
	}
	NxCatchAll("Error in OnSelChosenBatchCombo()");
}

void CUB92Setup::OnSelChosenSecondaryBatchCombo(long nRow) 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	
		long batch = 0;

		if(m_SecondaryBatchCombo->GetCurSel()==0)
			// (j.jones 2006-11-15 15:15) - PLID 23558 - this was using a 3, which is obsolete
			batch = 1;
		else if(m_SecondaryBatchCombo->GetCurSel()==1)
			batch = 2;
		else
			batch = 0;

		UpdateTable("DefBatchSecondary",batch);
	}
	NxCatchAll("Error in OnSelChosenSecondaryBatchCombo()");
}

void CUB92Setup::OnSelChosenBox82Combo(long nRow) 
{
	try {

		long value = 1;

		if(nRow == -1)
			value = 1;
		else {
			value = m_Box82Combo->GetValue(nRow,0).lVal;
		}

		UpdateTable("Box82Setup",value);

	} NxCatchAll("Error in OnSelChosenBox82Combo()");
}

void CUB92Setup::OnSelChosenBox83Combo(long nRow) 
{
	try {

		long value = 0;

		if(nRow == -1)
			value = 0;
		else {
			value = m_Box83Combo->GetValue(nRow,0).lVal;
		}

		UpdateTable("Box83Setup",value);

	} NxCatchAll("Error in OnSelChosenBox83Combo()");
}

void CUB92Setup::OnSelChosen8283Format(long nRow) 
{
	try {

		long value = 1;

		if(nRow == -1)
			value = 1;
		else {
			value = m_8283Format->GetValue(nRow,0).lVal;
		}

		UpdateTable("Box8283Format",value);

	} NxCatchAll("Error in OnSelChosenBox8283Format()");
}

void CUB92Setup::OnSelChosenBox82Number(long nRow) 
{
	try {

		long value = 1;

		if(nRow == -1)
			value = 1;
		else {
			value = m_Box82Number->GetValue(nRow,0).lVal;
		}

		UpdateTable("Box82Num",value);

		//TES 3/19/2007 - PLID 25235 - Default the qualifier based on the number they selected, if possible.
		NXDATALIST2Lib::IRowSettingsPtr pFormTypeRow = m_pFormTypes->CurSel;
		ASSERT(pFormTypeRow != NULL);
		long nType = VarLong(pFormTypeRow->GetValue(0));
		//calculate the ideal qualifier
		CString strQual = "";
		switch(value) {
		case 1: { //UPIN

			//1G - Provider UPIN Number
			strQual = "1G";
			break;
		}
		case 2: { //Social Security Number

			//SY - Social Security Number
			strQual = "SY";
			break;
		}
		case 3: { //Federal ID Number

			//EI - Employer's Identification Number
			strQual = "EI";
			break;
		}
		case 4: { //Medicare Number

			//1C - Medicare Provider Number
			strQual = "1C";
			break;
		}
		case 5: { //Medicaid Number

			//1D - Medicaid Provider Number
			strQual = "1D";
			break;
		}		
		case 6: { //BCBS Number
			
			//1B - Blue Cross Provider Number
			strQual = "1B";
			break;
		}
		case 7: { //DEA Number

			//0B - State License Number
			strQual = "0B";
			break;
		}
		case 12: { //NPI

			//XX - NPI Number
			strQual = "XX";
			break;
		}
		default:
			strQual = "";
			break;
		}

		CString strExistingQual;
		GetDlgItemText(IDC_BOX76_QUAL, strExistingQual);
		if(strQual.CompareNoCase(strExistingQual) != 0) {
			//they don't match, so use the new qualifier
			SetDlgItemText(IDC_BOX76_QUAL, strQual);
			m_strOldBox76Qualifier = strQual;
			ExecuteSql("UPDATE UB92SetupT SET UB04Box76Qual = '%s' WHERE ID = %li", _Q(strQual), m_pGroups->GetValue(m_pGroups->CurSel,0).lVal);
		}

		//TES 3/19/2007 - PLID 25235 - Only warn them if they're using the UB04
		if(GetUBFormType() == eUB04 && value != -1 && strQual.IsEmpty()) {
			AfxMessageBox("There is no default qualifier for the selected Box 76 number. Please manually enter a qualifier before submitting claims.\n"
				"If you are unsure of what qualifier to use, contact the insurance company to receive the correct qualifier.");
		}

		// (j.jones 2010-04-16 09:07) - PLID 38149 - yell if they picked NPI
		if(value == 12) {
			if(GetUBFormType() == eUB04) {
				AfxMessageBox("Using NPI in Box 76 will likely cause your claims to reject.\n"
					"Please select another ID other than NPI unless you are absolutely certain the NPI is needed here.");
			}
			else {
				//there is no qualifier in the old UB format but it could affect ANSI if they select NPI
				AfxMessageBox("Using NPI in Box 82 will likely cause your claims to reject.\n"
					"Please select another ID other than NPI unless you are absolutely certain the NPI is needed here.");
			}
		}

	} NxCatchAll("Error in OnSelChosenBox82Number()");
}

void CUB92Setup::OnSelChosenBox83Number(long nRow) 
{
	try {

		long value = 1;

		if(nRow == -1)
			value = 1;
		else {
			value = m_Box83Number->GetValue(nRow,0).lVal;
		}

		UpdateTable("Box83Num",value);

		//TES 3/19/2007 - PLID 25235 - Default the qualifier based on the number they selected, if possible.
		NXDATALIST2Lib::IRowSettingsPtr pFormTypeRow = m_pFormTypes->CurSel;
		ASSERT(pFormTypeRow != NULL);
		long nType = VarLong(pFormTypeRow->GetValue(0));
		//calculate the ideal qualifier
		CString strQual = "";
		switch(value) {
		case 1: { //UPIN

			//1G - Provider UPIN Number
			strQual = "1G";
			break;
		}
		case 2: { //Social Security Number

			//SY - Social Security Number
			strQual = "SY";
			break;
		}
		case 3: { //Federal ID Number

			//EI - Employer's Identification Number
			strQual = "EI";
			break;
		}
		case 4: { //Medicare Number

			//1C - Medicare Provider Number
			strQual = "1C";
			break;
		}
		case 5: { //Medicaid Number

			//1D - Medicaid Provider Number
			strQual = "1D";
			break;
		}		
		case 6: { //BCBS Number
			
			//1B - Blue Cross Provider Number
			strQual = "1B";
			break;
		}
		case 7: { //DEA Number

			//0B - State License Number
			strQual = "0B";
			break;
		}
		case 12: { //NPI

			//XX - NPI Number
			strQual = "XX";
			break;
		}
		default:
			strQual = "";
			break;
		}

		CString strExistingQual;
		GetDlgItemText(IDC_BOX78_QUAL, strExistingQual);
		if(strQual.CompareNoCase(strExistingQual) != 0) {
			//they don't match, so use the new qualifier
			SetDlgItemText(IDC_BOX78_QUAL, strQual);
			ExecuteSql("UPDATE UB92SetupT SET UB04Box78Qual = '%s' WHERE ID = %li", _Q(strQual), m_pGroups->GetValue(m_pGroups->CurSel,0).lVal);
		}

		//TES 3/19/2007 - PLID 25235 - Only warn them if they're using the UB04
		if(GetUBFormType() == eUB04 && value != -1 && strQual.IsEmpty()) {
			AfxMessageBox("There is no default qualifier for the selected Box 78 number. Please manually enter a qualifier before submitting claims.\n"
				"If you are unsure of what qualifier to use, contact the insurance company to receive the correct qualifier.");
		}

	} NxCatchAll("Error in OnSelChosenBox83Number()");
}

void CUB92Setup::On85Check() 
{
	try {

		long value = 1;
		if(m_checkBox85.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("Box85Show",value);

	} NxCatchAll("Error in On85Check()");
}

void CUB92Setup::OnGroupChargesCheck() 
{
	try {

		// (a.walling 2007-06-05 13:32) - PLID 26228 - Show/hide the expansion dropdown
		// (a.walling 2007-08-17 14:15) - PLID 27092 - it's a button now
		long value = 1;
		if(m_checkGroupCharges.GetCheck())
		{
			GetDlgItem(IDC_BTN_EXPAND_CODES)->EnableWindow(TRUE);
			value = 1;
		}
		else
		{
			GetDlgItem(IDC_BTN_EXPAND_CODES)->EnableWindow(FALSE);
			value = 0;
		}

		UpdateTable("GroupRev",value);

	} NxCatchAll("Error in OnGroupChargesCheck()");
}

void CUB92Setup::UpdateTable(CString BoxName, long data)
{
	if(m_pGroups->CurSel == -1)
		return;

	long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;

	ExecuteSql("UPDATE UB92SetupT SET %s = %li WHERE ID = %li", BoxName, data, GroupID);
}

void CUB92Setup::OnCheckShowPhoneNumber() 
{
	try {

		long value = 1;
		if(m_checkShowPhoneNum.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("ShowPhone",value);

	}NxCatchAll("Error in OnCheckShowPhoneNumber");
}

// (b.spivey July 8, 2015) - PLID 66515 -added FillBox12 and 16 checks. 
void CUB92Setup::OnCheckFill12_13()
{
	try {

		long value = 1;
		if (m_checkBox12_13.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("FillBox12_13", value);

	}NxCatchAll(__FUNCTION__);
}

// (b.spivey July 8, 2015) - PLID 66515 -added FillBox12 and 16 checks. 
void CUB92Setup::OnCheckFill16()
{
	try {

		long value = 1;
		if (m_checkBox16.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("FillBox16", value);

	}NxCatchAll(__FUNCTION__);
}

void CUB92Setup::OnCheckShowTotalAmount() 
{
	try {

		long value = 1;
		if(m_checkShowTotals.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("ShowTotals",value);

	}NxCatchAll("Error in OnCheckShowTotalAmount");
}

void CUB92Setup::OnCheckShowInscoAddress() 
{
	try {

		long value = 1;
		// (a.walling 2007-06-05 11:01) - PLID 26223 - Enabled/disable the use three box as appropriate
		if(m_checkShowInscoAdd.GetCheck()) {
			value = 1;
			m_checkUseThree.EnableWindow(TRUE);
			m_checkUseThree.RedrawWindow();
		}
		else {
			value = 0;
			m_checkUseThree.EnableWindow(FALSE);
			m_checkUseThree.RedrawWindow();
		}

		UpdateTable("ShowInsAdd",value);

	}NxCatchAll("Error in OnCheckShowInscoAddress");
}

void CUB92Setup::OnCheckShowInsAddrBox50() 
{
	try {

		long value = 1;
		if(m_checkShowInsAddr50.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("InsAddr50",value);

	}NxCatchAll("Error in OnCheckShowInsAddrBox50()");
}

void CUB92Setup::OnCheckPunctuateChargeLines() 
{
	try {

		long value = 1;
		if(m_checkPunctuateChargeLines.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("PunctuateChargeLines",value);

	}NxCatchAll("Error in OnCheckPunctuateChargeLines()");
}

void CUB92Setup::OnCheckPunctuateDiags() 
{
	try {

		long value = 1;
		if(m_checkPunctuateDiagCodes.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("PunctuateDiagCodes",value);

	}NxCatchAll("Error in OnCheckPunctuateDiags()");
}

void CUB92Setup::On76Check() 
{
	try {

		long value = 1;
		if(m_checkBox76.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("Box76Show",value);

	} NxCatchAll("Error in On76Check()");
}

void CUB92Setup::OnCheckFill42With44() 
{
	try {

		long value = 1;
		if(m_checkFill42With44.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("Fill42With44",value);

	} NxCatchAll("Error in OnCheckFill42With44()");
}

void CUB92Setup::OnCheckShowApplies() 
{
	try {

		long value = 1;
		if(m_checkShowApplies.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("ShowApplies",value);

	} NxCatchAll("Error in OnCheckShowApplies()");
}

void CUB92Setup::OnBtnUb92Dates() 
{
	if(m_pGroups->CurSel==-1)
		return;

	CUB92DatesDlg dlg(this);
	dlg.m_UB92SetupGroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
	dlg.m_strGroupName = CString(m_pGroups->GetValue(m_pGroups->CurSel,1).bstrVal);
	dlg.DoModal();
}

void CUB92Setup::OnCheckShowEmployerCompany() 
{
	try {

		long value = 1;
		if(m_checkShowEmployerCompany.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("ShowCompanyAsInsurer",value);

	} NxCatchAll("Error in OnCheckShowEmployerCompany()");
}

void CUB92Setup::OnSelChosenBox53Accepted(long nRow) 
{
	try {

		long value = 1;

		if(nRow == -1)
			value = 1;
		else {
			value = m_Box53Accepted->GetValue(nRow,0).lVal;
		}

		UpdateTable("Box53Accepted",value);

	} NxCatchAll("Error in OnSelChosenBox53Accepted()");
}

void CUB92Setup::OnAdvEbillingSetup() 
{
	if(m_pGroups->CurSel==-1)
		return;

	CAdvEbillingSetup dlg(this);
	dlg.m_bIsUB92 = TRUE;
	dlg.m_GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
	dlg.DoModal();	
}

LRESULT CUB92Setup::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {
			case NetUtils::CustomLabels:
			case NetUtils::InsuranceCoT:
			case NetUtils::InsuranceGroups: {
				try {
					UpdateView();
				} NxCatchAll("Error in CUB92Setup::OnTableChanged:Generic");
				break;
			}
		}
	} NxCatchAll("Error in CUB92Setup::OnTableChanged");

	return 0;
}

void CUB92Setup::On64Check() 
{
	try {

		long value = 1;
		if(m_checkBox64.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("Box64Show",value);

	} NxCatchAll("Error in On64Check()");
}

void CUB92Setup::ReflectFormType()
{
	//TES 3/14/2007 - PLID 24993 - Update all our labels to show the correct Box number, based on whether we're using the
	// UB04 or the UB92.

	//Get the preference out of the datalist.
	NXDATALIST2Lib::IRowSettingsPtr pFormTypeRow = m_pFormTypes->CurSel;
	ASSERT(pFormTypeRow != NULL);
	long nType = VarLong(pFormTypeRow->GetValue(0));
	switch(nType) {

	case 1://UB04
		SetDlgItemText(IDC_CHECK_SHOW_INSCO_ADDRESS, "Show Insurance Company Mailing Address In Box 80");
		//UB92 Box 64 doesn't have a corresponding UB04 box.
		GetDlgItem(IDC_64_CHECK)->ShowWindow(SW_HIDE);
		SetDlgItemText(IDC_76_CHECK, "Fill Box 69");
		SetDlgItemText(IDC_BOX_80_LABEL, "Box 74");
		// (j.jones 2012-09-05 14:37) - PLID 52191 - added Box74Qual
		SetDlgItemText(IDC_BOX_74_QUAL_LABEL, "Box 74 ANSI Qualifier");
		//UB92 Boxes 7-10 don't have a corresponding UB04 box.
		GetDlgItem(IDC_BOX8_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_BOX8)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX10_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_BOX10)->ShowWindow(SW_HIDE);
		// (j.jones 2013-06-07 16:48) - PLID 41479 - Box 18 on the UB is no longer Box 13 in the UB04. It is hidden now.
		GetDlgItem(IDC_BOX18_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_BOX18)->ShowWindow(SW_HIDE);
		SetDlgItemText(IDC_BOX19_LABEL, "Box 14");
		SetDlgItemText(IDC_BOX20_LABEL, "Box 15");
		SetDlgItemText(IDC_BOX22_LABEL, "Box 17");
		SetDlgItemText(IDC_BOX32_LABEL, "Box 31");
		GetDlgItem(IDC_BOX79_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_BOX79)->ShowWindow(SW_HIDE);
		
		// (j.jones 2008-05-21 10:28) - PLID 30129 - added UB04 Box 66
		GetDlgItem(IDC_EDIT_BOX66)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_BOX66_LABEL)->ShowWindow(SW_SHOWNA);

		SetDlgItemText(IDC_BOX_82_83_LABEL, "Boxes 76-78 Setup");
		SetDlgItemText(IDC_BOX82_LABEL, "Box 76 Use:");
		SetDlgItemText(IDC_BOX83_LABEL, "Box 78 Use:");
		GetDlgItem(IDC_CHECK_SHOW_TOTAL_AMOUNT)->ShowWindow(SW_HIDE);
		
		//TES 3/19/2007 - PLID 24993 - The Box 82/83 Format isn't used on the UB04.
		GetDlgItem(IDC_82_83_FORMAT_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_8283_FORMAT)->ShowWindow(SW_HIDE);

		//TES 3/19/2007 - PLID 25235 - Box 77 doesn't exist on the UB92.
		GetDlgItem(IDC_BOX77_LABEL)->ShowWindow(SW_SHOWNA);
		ShowDlgItem(IDC_BOX_77_COMBO, SW_SHOWNA);
		GetDlgItem(IDC_BOX77_QUAL)->ShowWindow(SW_SHOWNA);
		ShowDlgItem(IDC_BOX_77_NUMBER, SW_SHOWNA);

		//TES 3/19/2007 - PLID 25235 - The Box 76 and 78 qualifiers don't exist on the UB92.
		GetDlgItem(IDC_BOX76_QUAL)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_BOX78_QUAL)->ShowWindow(SW_SHOWNA);

		// (j.jones 2007-03-20 13:11) - PLID 25278 - The Box 56 NPI is UB04 only
		GetDlgItem(IDC_BOX_56_LABEL)->ShowWindow(SW_SHOWNA);
		ShowDlgItem(IDC_BOX_56_NPI, SW_SHOWNA);

		// (j.jones 2007-03-21 10:34) - PLID 25279 - the Box 81 Taxonomy info. is UB04 only
		GetDlgItem(IDC_BOX_81_LABEL)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_LABEL_BOX_81A)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_BOX81A_QUAL)->ShowWindow(SW_SHOWNA);
		ShowDlgItem(IDC_BOX81A_TAXONOMY_LIST, SW_SHOWNA);
		GetDlgItem(IDC_LABEL_BOX_81B)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_BOX81B_QUAL)->ShowWindow(SW_SHOWNA);
		ShowDlgItem(IDC_BOX81B_TAXONOMY_LIST, SW_SHOWNA);
		GetDlgItem(IDC_LABEL_BOX_81C)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_BOX81C_QUAL)->ShowWindow(SW_SHOWNA);
		ShowDlgItem(IDC_BOX81C_TAXONOMY_LIST, SW_SHOWNA);

		//TES 3/21/2007 - PLID 25295 - Box 51 is now "Default Other Prv ID", and has options for what to fill
		SetDlgItemText(IDC_BOX51_LABEL, "Default Other Prv ID");
		GetDlgItem(IDC_BOX_51_CHECK)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_BOX_57_CHECK)->ShowWindow(SW_SHOWNA);
		m_Box82Number->PutValue(m_Box82Number->FindByColumn(0,(long)10,0,FALSE),1,_bstr_t("Other Prv ID"));
		m_Box83Number->PutValue(m_Box83Number->FindByColumn(0,(long)10,0,FALSE),1,_bstr_t("Other Prv ID"));

		//TES 3/21/2007 - PLID 24993 - There is no Box 85 on the UB04
		GetDlgItem(IDC_85_CHECK)->ShowWindow(SW_HIDE);

		// (a.walling 2007-06-05 09:42) - PLID 26219
		GetDlgItem(IDC_CHECK_BOX_42_LINE_23)->ShowWindow(SW_SHOWNA);
		// (a.walling 2007-06-05 11:42) - PLID 26223
		GetDlgItem(IDC_CHECK_USE_THREE)->ShowWindow(SW_SHOWNA);
		
		// (a.walling 2007-08-13 10:00) - PLID 26219
		GetDlgItem(IDC_EDIT_BOX42LINE23)->ShowWindow(SW_SHOWNA);

		// (a.walling 2007-06-05 13:34) - PLID 26228
		// (a.walling 2007-08-17 16:01) - PLID 27092
		ShowDlgItem(IDC_BTN_EXPAND_CODES, SW_SHOWNA);

		// (a.walling 2007-06-20 13:33) - PLID 26414
		m_checkUseICD9.ShowWindow(SW_SHOWNA);
		if (m_radioBox80CPT.GetCheck()) {
			m_checkUseICD9.EnableWindow(TRUE);
		} else {
			m_checkUseICD9.EnableWindow(FALSE);
		}

		// (j.jones 2007-07-12 08:44) - PLID 26621 - added Box 2 setup
		ShowDlgItem(IDC_BOX2_OPTIONS, SW_SHOWNA);
		GetDlgItem(IDC_BOX2_LABEL)->ShowWindow(SW_SHOWNA);

		// (j.jones 2007-07-12 11:01) - PLID 26625 - added new Box 1 setup (UB04 only)
		ShowDlgItem(IDC_BOX1_OPTIONS, SW_SHOWNA);
		GetDlgItem(IDC_RADIO_BOX1_LOCATION)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO_BOX1_POS)->ShowWindow(SW_HIDE);

		// (j.jones 2012-05-24 10:57) - PLID 50597 - added InsRelANSI, UB04 only
		ShowDlgItem(IDC_CHECK_USE_ANSI_INS_REL_CODES_BOX59, SW_SHOWNA);

		// (b.spivey July 8, 2015) - PLID 66515 - Show for UB04
		GetDlgItem(IDC_FILL_BOX_12_13)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_FILL_BOX_16)->ShowWindow(SW_SHOW);

		// (j.jones 2016-05-06 8:53) - NX-100514 - show Box 39
		GetDlgItem(IDC_BOX39_LABEL)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_BOX39_CODE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_BOX39_VALUE)->ShowWindow(SW_SHOW);

		break;

	case 0://UB92
		SetDlgItemText(IDC_CHECK_SHOW_INSCO_ADDRESS, "Show Insurance Company Mailing Address In Box 84");
		//UB92 Box 64 doesn't have a corresponding UB04 box.
		GetDlgItem(IDC_64_CHECK)->ShowWindow(SW_SHOWNA);
		SetDlgItemText(IDC_76_CHECK, "Fill Box 76");
		SetDlgItemText(IDC_BOX_80_LABEL, "Box 80");
		// (j.jones 2012-09-05 14:37) - PLID 52191 - added Box74Qual
		SetDlgItemText(IDC_BOX_74_QUAL_LABEL, "Box 80 ANSI Qualifier");
		//UB92 Boxes 7-10 don't have a corresponding UB04 box.
		GetDlgItem(IDC_BOX8_LABEL)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_EDIT_BOX8)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_BOX10_LABEL)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_EDIT_BOX10)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_BOX18_LABEL)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_EDIT_BOX18)->ShowWindow(SW_SHOWNA);
		SetDlgItemText(IDC_BOX18_LABEL, "Box 18");
		SetDlgItemText(IDC_BOX19_LABEL, "Box 19");
		SetDlgItemText(IDC_BOX20_LABEL, "Box 20");
		SetDlgItemText(IDC_BOX22_LABEL, "Box 22");
		SetDlgItemText(IDC_BOX32_LABEL, "Box 32");
		GetDlgItem(IDC_BOX79_LABEL)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_EDIT_BOX79)->ShowWindow(SW_SHOWNA);

		// (j.jones 2008-05-21 10:28) - PLID 30129 - added UB04 Box 66
		GetDlgItem(IDC_EDIT_BOX66)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX66_LABEL)->ShowWindow(SW_HIDE);

		SetDlgItemText(IDC_BOX_82_83_LABEL, "Boxes 82/83 Setup");
		SetDlgItemText(IDC_BOX82_LABEL, "Box 82 Use:");
		SetDlgItemText(IDC_BOX83_LABEL, "Box 83 Use:");
		GetDlgItem(IDC_CHECK_SHOW_TOTAL_AMOUNT)->ShowWindow(SW_SHOWNA);
		
		//TES 3/19/2007 - PLID 24993 - The Box 82/83 Format is obsolete on the UB04.
		GetDlgItem(IDC_82_83_FORMAT_LABEL)->ShowWindow(SW_SHOWNA);
		ShowDlgItem(IDC_8283_FORMAT, SW_SHOWNA);

		//TES 3/19/2007 - PLID 25235 - Box 77 doesn't exist on the UB04.
		GetDlgItem(IDC_BOX77_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX_77_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX77_QUAL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX_77_NUMBER)->ShowWindow(SW_HIDE);

		//TES 3/19/2007 - PLID 25235 - The Box 76 and 78 qualifiers don't exist on the UB04.
		GetDlgItem(IDC_BOX76_QUAL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX78_QUAL)->ShowWindow(SW_HIDE);

		// (j.jones 2007-03-20 13:11) - PLID 25278 - The Box 56 NPI is UB04 only
		GetDlgItem(IDC_BOX_56_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX_56_NPI)->ShowWindow(SW_HIDE);

		// (j.jones 2007-03-21 10:34) - PLID 25279 - the Box 81 Taxonomy info. is UB04 only
		GetDlgItem(IDC_BOX_81_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_BOX_81A)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX81A_QUAL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX81A_TAXONOMY_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_BOX_81B)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX81B_QUAL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX81B_TAXONOMY_LIST)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_LABEL_BOX_81C)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX81C_QUAL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX81C_TAXONOMY_LIST)->ShowWindow(SW_HIDE);

		//TES 3/21/2007 - PLID 25295 - Box 51 is now "Default Other Prv ID", and has options for what to fill
		SetDlgItemText(IDC_BOX51_LABEL, "Default Box 51 #");
		GetDlgItem(IDC_BOX_51_CHECK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX_57_CHECK)->ShowWindow(SW_HIDE);
		m_Box82Number->PutValue(m_Box82Number->FindByColumn(0,(long)10,0,FALSE),1,_bstr_t("Box 51 Number"));
		m_Box83Number->PutValue(m_Box83Number->FindByColumn(0,(long)10,0,FALSE),1,_bstr_t("Box 51 Number"));

		//TES 3/21/2007 - PLID 24993 - There is no Box 85 on the UB04
		GetDlgItem(IDC_85_CHECK)->ShowWindow(SW_SHOWNA);

		// (a.walling 2007-06-05 09:44) - PLID 26219
		GetDlgItem(IDC_CHECK_BOX_42_LINE_23)->ShowWindow(SW_HIDE);
		
		// (a.walling 2007-08-13 10:00) - PLID 26219
		GetDlgItem(IDC_EDIT_BOX42LINE23)->ShowWindow(SW_HIDE);

		// (a.walling 2007-06-05 11:42) - PLID 26223
		GetDlgItem(IDC_CHECK_USE_THREE)->ShowWindow(SW_HIDE);

		// (a.walling 2007-06-05 13:34) - PLID 26228
		// (a.walling 2007-08-17 16:00) - PLID 27092 - It's a button now
		GetDlgItem(IDC_BTN_EXPAND_CODES)->ShowWindow(SW_HIDE);	
		
		// (a.walling 2007-06-20 13:33) - PLID 26414
		GetDlgItem(IDC_CHECK_USE_ICD9_PROCEDURE_CODES)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHECK_USE_ICD9_PROCEDURE_CODES)->EnableWindow(FALSE);

		// (j.jones 2007-07-12 08:44) - PLID 26621 - added Box 2 setup
		GetDlgItem(IDC_BOX2_OPTIONS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_BOX2_LABEL)->ShowWindow(SW_HIDE);

		// (j.jones 2007-07-12 11:01) - PLID 26625 - added new Box 1 setup (UB04 only)
		GetDlgItem(IDC_BOX1_OPTIONS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO_BOX1_LOCATION)->ShowWindow(SW_SHOWNA);
		GetDlgItem(IDC_RADIO_BOX1_POS)->ShowWindow(SW_SHOWNA);

		// (j.jones 2012-05-24 10:57) - PLID 50597 - added InsRelANSI, UB04 only
		GetDlgItem(IDC_CHECK_USE_ANSI_INS_REL_CODES_BOX59)->ShowWindow(SW_HIDE);

		// (b.spivey July 8, 2015) - PLID 66515 - Hide for UB92
		GetDlgItem(IDC_FILL_BOX_12_13)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_FILL_BOX_16)->ShowWindow(SW_HIDE);

		// (j.jones 2016-05-06 8:53) - NX-100514 - hide Box 39
		GetDlgItem(IDC_BOX39_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_BOX39_CODE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_BOX39_VALUE)->ShowWindow(SW_HIDE);

		break;
	default:
		//What?  This shouldn't be possible!
		AfxThrowNxException("Invalid UB Form Type %li found!", nType);
		break;
	}
}

void CUB92Setup::OnSelChangingUbFormType(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		//TES 3/12/2007 - PLID 24993 - Don't allow them to clear the selection.
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll("Error in CUB92Setup::OnSelChangingUbFormType()");
}

void CUB92Setup::OnSelChosenUbFormType(LPDISPATCH lpRow) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		ASSERT(pRow != NULL);//This should have been handled by the OnSelChanging();

		//TES 3/12/2007 - PLID 24993 - Update the preference
		SetRemotePropertyInt("UBFormType", VarLong(pRow->GetValue(0)), 0, "<None>");
		//Update the screen.
		ReflectFormType();

	}NxCatchAll("Error in CUB92Setup::OnSelChosenUbFormType()");
}

HBRUSH CUB92Setup::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	switch (pWnd->GetDlgCtrlID())  {
		case IDC_BOX8_LABEL:
		case IDC_BOX18_LABEL:
		case IDC_BOX19_LABEL:
		case IDC_BOX20_LABEL:
		case IDC_BOX22_LABEL:
		case IDC_BOX32_LABEL:
		case IDC_CHECK_SHOW_INSCO_ADDRESS:
		case IDC_BOX_80_LABEL:
		case IDC_BOX82_LABEL:
		case IDC_BOX77_LABEL:
		case IDC_BOX83_LABEL:
		case IDC_82_83_FORMAT_LABEL:
		case IDC_BOX51_LABEL:
		case IDC_BOX_56_NPI:
		case IDC_BOX_81_LABEL:
		case IDC_LABEL_BOX_81A:
		case IDC_LABEL_BOX_81B:
		case IDC_LABEL_BOX_81C:
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(0x008080FF));
			return m_brush;
		break;
		default:
		break;
	}

	return hbr;
	*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CUB92Setup::OnSelChosenBox77Number(LPDISPATCH lpRow) 
{
	try {
		//TES 3/19/2007 - PLID 25235 - Save the new value.

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		long nBox77 = 1;
		if(pRow == NULL)
			nBox77 = 1;
		else {
			nBox77 = VarLong(pRow->GetValue(0));
		}

		UpdateTable("UB04Box77Num",nBox77);

		//TES 3/19/2007 - PLID 25235 - calculate the ideal qualifier
		CString strQual = "";
		switch(nBox77) {
		case 1: { //UPIN

			//1G - Provider UPIN Number
			strQual = "1G";
			break;
		}
		case 2: { //Social Security Number

			//SY - Social Security Number
			strQual = "SY";
			break;
		}
		case 3: { //Federal ID Number

			//EI - Employer's Identification Number
			strQual = "EI";
			break;
		}
		case 4: { //Medicare Number

			//1C - Medicare Provider Number
			strQual = "1C";
			break;
		}
		case 5: { //Medicaid Number

			//1D - Medicaid Provider Number
			strQual = "1D";
			break;
		}		
		case 6: { //BCBS Number
			
			//1B - Blue Cross Provider Number
			strQual = "1B";
			break;
		}
		case 7: { //DEA Number

			//0B - State License Number
			strQual = "0B";
			break;
		}
		case 12: { //NPI

			//XX - NPI Number
			strQual = "XX";
			break;
		}
		default:
			strQual = "";
			break;
		}

		CString strExistingQual;
		GetDlgItemText(IDC_BOX77_QUAL, strExistingQual);
		if(strQual.CompareNoCase(strExistingQual) != 0) {
			//they don't match, so use the new qualifier
			SetDlgItemText(IDC_BOX77_QUAL, strQual);
			ExecuteSql("UPDATE UB92SetupT SET UB04Box77Qual = '%s' WHERE ID = %li", _Q(strQual), m_pGroups->GetValue(m_pGroups->CurSel,0).lVal);
		}

		if(nBox77 != -1 && strQual.IsEmpty()) {
			AfxMessageBox("There is no default qualifier for the selected Box 77 number. Please manually enter a qualifier before submitting claims.\n"
				"If you are unsure of what qualifier to use, contact the insurance company to receive the correct qualifier.");
		}

	} NxCatchAll("Error in OnSelChosenBox77Number()");
}

void CUB92Setup::OnSelChosenBox77Combo(LPDISPATCH lpRow) 
{
	try {
		//TES 3/19/2007 - PLID 25235 - Save the new value.
		long value = 1;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL)
			value = 1;
		else {
			value = VarLong(pRow->GetValue(0));
		}

		UpdateTable("UB04Box77Setup",value);

	} NxCatchAll("Error in OnSelChosenBox77Combo()");
}

void CUB92Setup::OnSelChosenBox56Npi(LPDISPATCH lpRow)
{
	try {
		
		// (j.jones 2007-03-20 12:56) - PLID 25278 - save the new value
		long value = 1;

		//1 - Bill Provider
		//2 - General 2 Provider
		//3 - Location

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			value = 1;
			m_Box56NPICombo->SetSelByColumn(0, (long)1);
		}
		else {
			value = VarLong(pRow->GetValue(0));
		}

		UpdateTable("UB04Box56NPI",value);

	} NxCatchAll("Error in OnSelChosenBox56Npi()");
}

void CUB92Setup::OnSelChosenBox81aTaxonomyList(LPDISPATCH lpRow) 
{
	try {
		
		// (j.jones 2007-03-21 10:27) - PLID 25279 - save the new value
		long value = 1;

		//-1 - None
		//1 - Bill Provider
		//2 - General 2 Provider
		//3 - Referring Physician	// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			value = -1;
			m_Box81aCombo->SetSelByColumn(0, (long)-1);
		}
		else {
			value = VarLong(pRow->GetValue(0));
		}

		UpdateTable("UB04Box81a",value);

	} NxCatchAll("Error in OnSelChosenBox81aTaxonomyList()");
}

void CUB92Setup::OnSelChosenBox81bTaxonomyList(LPDISPATCH lpRow) 
{
	try {
		
		// (j.jones 2007-03-21 10:27) - PLID 25279 - save the new value
		long value = 1;

		//-1 - None
		//1 - Bill Provider
		//2 - General 2 Provider
		//3 - Referring Physician	// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			value = -1;
			m_Box81bCombo->SetSelByColumn(0, (long)-1);
		}
		else {
			value = VarLong(pRow->GetValue(0));
		}

		UpdateTable("UB04Box81b",value);

	} NxCatchAll("Error in OnSelChosenBox81bTaxonomyList()");
}

void CUB92Setup::OnSelChosenBox81cTaxonomyList(LPDISPATCH lpRow) 
{
	try {
		
		// (j.jones 2007-03-21 10:27) - PLID 25279 - save the new value
		long value = 1;

		//-1 - None
		//1 - Bill Provider
		//2 - General 2 Provider
		//3 - Referring Physician	// (j.jones 2013-04-05 15:56) - PLID 40960 - Referring Physicians don't have taxonomy codes anymore

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			value = -1;
			m_Box81cCombo->SetSelByColumn(0, (long)-1);
		}
		else {
			value = VarLong(pRow->GetValue(0));
		}

		UpdateTable("UB04Box81c",value);

	} NxCatchAll("Error in OnSelChosenBox81cTaxonomyList()");
}

void CUB92Setup::OnBox51Check() 
{
	try {

		long value = 1;
		if(m_checkBox51.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("UB04Box51Show",value);

	} NxCatchAll("Error in OnBox51Check()");
}

void CUB92Setup::OnBox57Check() 
{
	try {

		long value = 1;
		if(m_checkBox57.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("UB04Box57Show",value);

	} NxCatchAll("Error in OnBox57Check()");
}

// (a.walling 2007-06-05 09:30) - PLID 26219
void CUB92Setup::OnCheckBox42Line23() 
{
	try {
		long value = 1;
		if(m_checkBox42Line23.GetCheck())
			value = 1;
		else
			value = 0;

		// (a.walling 2007-08-13 09:53) - PLID 26219 - Enable the default value box
		GetDlgItem(IDC_EDIT_BOX42LINE23)->EnableWindow(value == 1 ? TRUE : FALSE);

		UpdateTable("UB04Box42Line23",value);

	}NxCatchAll("Error in OnCheckBox42Line23");		
}

void CUB92Setup::OnCheckUseThree() 
{
	// (a.walling 2007-06-05 11:00) - PLID 26223
	try {
		long value = 1;
		if(m_checkUseThree.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("UB04Box80UseThree",value);

	}NxCatchAll("Error in OnCheckUseThree");	
}

// (a.walling 2007-08-17 14:10) - PLID 27092 - Use multiple codes now
/*
void CUB92Setup::OnSelChangingListExpandRevcode(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	if(lppNewSel == NULL) {
		//Don't let them select no row.
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}	
}

void CUB92Setup::OnSelChangedListExpandRevcode(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	// (a.walling 2007-06-05 13:46) - PLID 26228 - Save the category choice
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

		if (pRow) {
			long nRevID = VarLong(pRow->GetValue(0), -1);

			UpdateTable("UB04GroupRevExpand", nRevID);
		}
	}NxCatchAll("Error in OnSelChangedListExpandRevcode");	
}
*/

// (a.walling 2007-06-20 13:35) - PLID 26414 - Save the preference
void CUB92Setup::OnCheckUseIcd9ProcedureCodes() 
{
	try {
		long value = 1;
		if(m_checkUseICD9.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("UB04UseICD9ProcedureCodes", value);
	}NxCatchAll("Error in OnCheckUseIcd9ProcedureCodes");
}

// (j.jones 2007-07-11 15:46) - PLID 26621 - added Box 2 setup
void CUB92Setup::OnSelChosenBox2Options(LPDISPATCH lpRow) 
{
	try {

		long value = -1;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			value = VarLong(pRow->GetValue(0), -1);
		}

		UpdateTable("UB04Box2Setup",value);

	} NxCatchAll("Error in OnSelChosenBox2Options()");
}

// (j.jones 2007-07-12 10:22) - PLID 26625 - added new Box 1 setup (UB04 only)
void CUB92Setup::OnSelChosenBox1Options(LPDISPATCH lpRow) 
{
	try {

		long value = -1;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			value = VarLong(pRow->GetValue(0), -1);
		}

		UpdateTable("UB04Box1Setup",value);

	} NxCatchAll("Error in OnSelChosenBox1Options()");
}

// (a.walling 2007-08-17 15:57) - PLID 27092
void CUB92Setup::OnBtnExpandCodes() 
{
	try {
		if (m_pGroups->CurSel == sriNoRow)
			return;

		long nGroup = VarLong(m_pGroups->GetValue(m_pGroups->CurSel,0), -1);

		if (nGroup == -1) return;

		CUB92SetupInfo ubInfo(nGroup);
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "UB92CategoriesT");

		dlg.PreSelect(ubInfo.arUB04GroupRevExpand);

		CStringArray saFields, saNames;
		saFields.Add("Code");
		saNames.Add("Code");
		// (a.walling 2007-10-01 14:13) - PLID 27092 - Don't change anything if they cancelled out of the dialog
		if (IDCANCEL != dlg.Open("UB92CategoriesT", "1=1", "ID", "Name", "Select revenue codes to expand", 0, 0xFFFFFFFF, &saFields, &saNames))
		{
			CArray<long, long> arSelected;
			dlg.FillArrayWithIDs(arSelected);

			CString strBatch = BeginSqlBatch();
			CString strDeleteIDs;
			long nChanged = 0;

			for (int i = 0; i < ubInfo.arUB04GroupRevExpand.GetSize(); i++) {
				long nCurID = ubInfo.arUB04GroupRevExpand[i];
				BOOL bFound = FALSE;
				
				for (int j = 0; j < arSelected.GetSize(); j++) {
					if (nCurID == arSelected[j]) {
						arSelected[j] = -1;
						nChanged++;
						bFound = TRUE;
						break;
					}
				}

				if (!bFound) {
					// was never found, needs to be removed
					nChanged++;
					strDeleteIDs += FormatString("%li,", nCurID);
				}
			}

			for (i = 0; i < arSelected.GetSize(); i++) {
				if (arSelected[i] != -1) {
					nChanged++;
					// item is newly selected, so add.
					AddStatementToSqlBatch(strBatch, "INSERT INTO UB04MultiGroupRevExpandT (UB92SetupID, UBCategoryID) VALUES(%li, %li)", nGroup, arSelected[i]);
				}
			}

			strDeleteIDs.TrimRight(",");
			if (strDeleteIDs.GetLength() > 0)
				AddStatementToSqlBatch(strBatch, "DELETE FROM UB04MultiGroupRevExpandT WHERE UB92SetupID = %li AND UBCategoryID IN (%s)", nGroup, strDeleteIDs);

			if (nChanged > 0) {
				ExecuteSqlBatch(strBatch);
			}
		}

	} NxCatchAll("Error in CUB92Setup::OnBtnExpandCodes");
}

// (j.jones 2012-05-24 10:57) - PLID 50597 - added InsRelANSI
void CUB92Setup::OnBnClickedCheckUseAnsiInsRelCodesBox59()
{
	try {

		long value = 1;
		if(m_checkUseAnsiInsRelCodesBox59.GetCheck())
			value = 1;
		else
			value = 0;

		UpdateTable("InsRelANSI", value);

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-09-05 15:04) - PLID 52191 - added Box74Qual
void CUB92Setup::OnSelChangingBox74QualCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Don't allow them to clear the selection.
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-09-05 15:04) - PLID 52191 - added Box74Qual
void CUB92Setup::OnSelChosenBox74QualCombo(LPDISPATCH lpRow)
{
	try {
		
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		long nBox74Qual = 0;	//0 - Use Default
		if(pRow) {
			nBox74Qual = VarLong(pRow->GetValue(0));
		}

		UpdateTable("Box74Qual", nBox74Qual);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-07-18 09:15) - PLID 41823 - added DontBatchSecondary
void CUB92Setup::OnCheckDontBatchSecondary()
{
	try {

		if (m_pGroups->CurSel == sriNoRow)
			return;

		if (m_loading)
			return;

		long nDontBatchSecondary = m_checkDontBatchSecondary.GetCheck();

		if(nDontBatchSecondary == 1) {
			//warn what this feature is doing (this same 'dont show' is shared with the HCFA tab)
			DontShowMeAgain(this, "The \"Primary automatically sends claims to Secondary\" setting is used for claims where a primary insurance company "
				"in this UB group automatically sends the claim on to the patient's secondary insurance company. The secondary company does not need to be in this UB group.\n\n"
				"If this setting is enabled, Practice will not batch the claim for the secondary insurance after the primary insurance pays their responsibility. "
				"An entry in Claim History will be made for the secondary insurance at the time the primary insurance pays.\n\n"
				"If any secondary insurance companies do not accept crossover claims, click the \"...\" button to choose which companies should be excluded from this rule. "
				"These excluded secondary companies will always have their claims batched.", "DontBatchSecondary", "Practice");
		}
		
		UpdateTable("DontBatchSecondary", nDontBatchSecondary);

		m_btnSecondaryExclusions.EnableWindow(nDontBatchSecondary == 1);

		//audit this
		long nGroupID = VarLong(m_pGroups->GetValue(m_pGroups->CurSel,0));
		CString strGroupName = VarString(m_pGroups->GetValue(m_pGroups->CurSel,1));

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, strGroupName, nAuditID, aeiUBSetup_DontBatchSecondary, nGroupID, nDontBatchSecondary == 1 ? "Disabled" : "Enabled", nDontBatchSecondary == 1 ? "Enabled" : "Disabled", aepMedium, aetChanged);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-07-18 09:19) - PLID 41823 - added 'DontBatchSecondary' exclusions
void CUB92Setup::OnBtnEditSecondaryExclusions()
{
	long nAuditTransactionID = -1;

	try {

		if (m_pGroups->CurSel == sriNoRow)
			return;

		if (m_loading)
			return;

		//this button shouldn't be enabled unless DontBatchSecondary is enabled
		if(!m_checkDontBatchSecondary.GetCheck()) {
			m_btnSecondaryExclusions.EnableWindow(FALSE);
			return;
		}

		long nGroupID = VarLong(m_pGroups->GetValue(m_pGroups->CurSel,0));
		CString strGroupName = VarString(m_pGroups->GetValue(m_pGroups->CurSel,1));

		CMultiSelectDlg dlg(this, "UBSecondaryExclusionsT");
	
		//pre-select any already-excluded companies

		CMap<long, long, CString, LPCTSTR> mapInsuranceIDToName;
		CDWordArray dwaOldCompanies;
		std::set<long> setOldCompanies;

		_RecordsetPtr rsExisting = CreateParamRecordset("SELECT UBSecondaryExclusionsT.InsuranceCoID, InsuranceCoT.Name "
			"FROM UBSecondaryExclusionsT "
			"INNER JOIN InsuranceCoT ON UBSecondaryExclusionsT.InsuranceCoID = InsuranceCoT.PersonID "
			"WHERE UBSecondaryExclusionsT.UBSetupID = {INT}", nGroupID);		
		while(!rsExisting->eof) {
			long nInsuranceCoID = VarLong(rsExisting->Fields->Item["InsuranceCoID"]->Value);
			CString strName = VarString(rsExisting->Fields->Item["Name"]->Value);
			dwaOldCompanies.Add(nInsuranceCoID);
			setOldCompanies.insert(nInsuranceCoID);
			//store the name for auditing later
			mapInsuranceIDToName.SetAt(nInsuranceCoID, strName);
			rsExisting->MoveNext();
		}
		rsExisting->Close();
		dlg.PreSelect(dwaOldCompanies);
		
		CDWordArray dwaNewCompanies;
		std::set<long> setNewCompanies;
		//this intentionally includes inactive insurance companies
		UINT nResult = dlg.Open("InsuranceCoT", "", "InsuranceCoT.PersonID", "Name", "Select secondary insurance companies that should always be batched:");
		if(nResult == IDOK) {
			dlg.FillArrayWithIDs(dwaNewCompanies);

			CVariantArray aryNames;
			dlg.FillArrayWithNames(aryNames);

			if(aryNames.GetSize() != dwaNewCompanies.GetSize()) {
				//should be impossible
				ThrowNxException("CMultiSelectDlg returned results with mismatched sizes.");
			}

			for(int i=0; i<dwaNewCompanies.GetSize();i++) {
				long nSelCompany = (long)dwaNewCompanies.GetAt(i);
				setNewCompanies.insert(nSelCompany);

				//fill our map
				mapInsuranceIDToName.SetAt(nSelCompany, VarString(aryNames.GetAt(i)));
			}
		}
		else {
			//if they cancelled, don't apply any changes
			return;
		}

		//find the ones to remove and the ones to add
		std::set<long> setRemovedCompanies;
		std::set<long> setAddedCompanies;
		boost::set_difference(setOldCompanies, setNewCompanies, inserter(setRemovedCompanies, setRemovedCompanies.end()));
		boost::set_difference(setNewCompanies, setOldCompanies, inserter(setAddedCompanies, setAddedCompanies.end()));

		CString strSqlBatch;
		CNxParamSqlArray aryParams;
		foreach(long nRemovedInsuranceCoID, setRemovedCompanies) {
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM UBSecondaryExclusionsT "
				"WHERE InsuranceCoID = {INT} AND UBSetupID = {INT}", nRemovedInsuranceCoID, nGroupID);

			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}			
			CString strOld, strNew;
			CString strInsuranceCoName;
			if(!mapInsuranceIDToName.Lookup(nRemovedInsuranceCoID, strInsuranceCoName)) {
				//should be impossible
				ThrowNxException("Could not find removed insurance company name.");
			}
			strOld.Format("%s in exclusion list", strInsuranceCoName);
			strNew = "Removed from exclusion list";
			AuditEvent(-1, strGroupName, nAuditTransactionID, aeiUBSetup_InsCoSecondaryExcluded, nGroupID, strOld, strNew, aepMedium, aetDeleted); 
		}
		foreach(long nAddedInsuranceCoID, setAddedCompanies) {
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO UBSecondaryExclusionsT (InsuranceCoID, UBSetupID) "
				"VALUES ({INT}, {INT})", nAddedInsuranceCoID, nGroupID);

			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			CString strOld, strNew;
			CString strInsuranceCoName;
			if(!mapInsuranceIDToName.Lookup(nAddedInsuranceCoID, strInsuranceCoName)) {
				//should be impossible
				ThrowNxException("Could not find added insurance company name.");
			}
			strOld.Format("%s not in exclusion list", strInsuranceCoName);
			strNew = "Added to exclusion list";
			AuditEvent(-1, strGroupName, nAuditTransactionID, aeiUBSetup_InsCoSecondaryExcluded, nGroupID, strOld, strNew, aepMedium, aetCreated); 
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
		}
		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}

	}NxCatchAllCall(__FUNCTION__, {
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	});
}
// HcfaSetup.cpp : implementation file

#include "stdafx.h"
#include "PracProps.h"
#include "HcfaSetup.h"
#include "EditInsInfoDlg.h"
#include "NxStandard.h"
#include "PrintAlignDlg.h"
#include "globaldatautils.h"
#include "AdvEbillingSetup.h"
#include "AdvHCFAPinSetup.h"
#include "AuditTrail.h"
#include "HCFADatesDlg.h"
#include "HCFASetupInfo.h"
#include "EditDefaultPOSCodesDlg.h"
#include "HCFASetupDoNotFill.h"
#include "AdvHCFAClaimProviderSetupDlg.h"
#include "HCFAUpgradeDateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const int BACKGROUND_COLOR  = GetNxColor(GNC_ADMIN, 0);
// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CHcfaSetup dialog


CHcfaSetup::CHcfaSetup(CWnd* pParent)
	: CNxDialog(CHcfaSetup::IDD, pParent),
	m_labelChecker(NetUtils::CustomLabels)
	,
	m_companyChecker(NetUtils::InsuranceCoT), // All insurance companies
	m_groupsChecker(NetUtils::InsuranceGroups)
{
	m_loading = false;
	m_strOldBox33bQualifier = "";
	//{{AFX_DATA_INIT(CHcfaSetup)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "System_Setup/Billing_Setup/Insurance_Billing_Setup/configure_the_hcfa_form.htm";
}

void CHcfaSetup::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHcfaSetup)
	DDX_Control(pDX, IDC_CHECK_VALIDATE_REF_PHY, m_checkValidateRefPhy);
	DDX_Control(pDX, IDC_BTN_CONFIG_CLAIM_PROVIDERS, m_btnConfigClaimProviders);
	DDX_Control(pDX, IDC_CHECK_PUT_SPACE_AFTER_QUALIFIERS, m_checkPutSpaceAfterQualifiers);
	DDX_Control(pDX, IDC_RADIO_USE_LOC_NPI, m_radioUseLocNPI);
	DDX_Control(pDX, IDC_RADIO_USE_PROV_NPI, m_radioUseProvNPI);
	DDX_Control(pDX, IDC_CHECK_USE_1A_IN_9A_ALWAYS, m_checkUse1aIn9aAlways);
	DDX_Control(pDX, IDC_CHECK_HIDE_PHONE_NUM, m_checkHidePhoneNum);
	DDX_Control(pDX, IDC_CHECK_USE_1A_IN_9A, m_checkUse1aIn9a);
	DDX_Control(pDX, IDC_CHECK_USE_OVERRIDE_NAME_IN_BOX_31, m_checkUseOverrideInBox31);
	DDX_Control(pDX, IDC_CHECK_USE_23_IN_17A_AND_NOT_23, m_checkUse23In17aAndNotIn23);
	DDX_Control(pDX, IDC_CHECK_USE_23_IN_17A, m_checkUse23In17a);
	DDX_Control(pDX, IDC_BTN_DO_NOT_FILL, m_btnDoNotFillSetup);
	DDX_Control(pDX, IDC_RADIO_REF_FML, m_radioRefFML);
	DDX_Control(pDX, IDC_RADIO_REF_LFM, m_radioRefLFM);	
	DDX_Control(pDX, IDC_BTN_ADV_PIN_SETUP, m_btnAdvPINSetup);
	DDX_Control(pDX, IDC_ADV_POS_CODE_SETUP, m_btnAdvPOSCodeSetup);
	DDX_Control(pDX, IDC_ADV_EBILLING_SETUP, m_btnAdvEbillingSetup);
	DDX_Control(pDX, IDC_BTN_HCFA_DATES, m_btnHCFADates);
	DDX_Control(pDX, IDC_CHECK_DEFAULT_IN_BOX19, m_checkOverrideBox19);	
	DDX_Control(pDX, IDC_CHECK_REFPHYID_IN_BOX19, m_checkBox19RefPhyID);	
	DDX_Control(pDX, IDC_RADIO_USE_LOC_ADDR, m_radioUseLocationAddr);
	DDX_Control(pDX, IDC_RADIO_USE_CONTACT_ADDR, m_radioUseContactAddr);	
	DDX_Control(pDX, IDC_CHECK_USE_EMPLOYER, m_checkUseEmployer);		
	DDX_Control(pDX, IDC_RADIO_FML, m_radioFML);
	DDX_Control(pDX, IDC_RADIO_LFM, m_radioLFM);	
	DDX_Control(pDX, IDC_ALIGN_FORM, m_btnAlignForm);
	DDX_Control(pDX, IDC_EDIT_INS_INFO, m_btnEditInsList);
	DDX_Control(pDX, IDC_RADIO_OVERRIDE, m_radioOverride);
	DDX_Control(pDX, IDC_RADIO_USE_IF_BLANK, m_radioUseIfBlank);	
	DDX_Control(pDX, IDC_DELETE_GROUP, m_deleteGroup);
	DDX_Control(pDX, IDC_NEW_GROUP, m_newGroup);	
	DDX_Control(pDX, IDC_RADIO_EIN, m_25ein);
	DDX_Control(pDX, IDC_RADIO_SSN, m_25ssn);
	DDX_Control(pDX, IDC_REMOVE_COMPANY, m_remBtn);
	DDX_Control(pDX, IDC_REMOVE_ALL_COMPANIES, m_remAllBtn);
	DDX_Control(pDX, IDC_ADD_COMPANY, m_addBtn);
	DDX_Control(pDX, IDC_ADD_ALL_COMPANIES, m_addAllBtn);
	DDX_Control(pDX, IDC_RADIO_USEBILLPROV, m_useBillProvider);
	DDX_Control(pDX, IDC_RADIO_USEGEN1PROV, m_useMainProvider);	
	DDX_Control(pDX, IDC_RADIO_USEOVRRDPROV,m_overideProvider);
	DDX_Control(pDX, IDC_RADIO_USE_BILL_LOCATION,m_useBillLocation);
	DDX_Control(pDX, IDC_EDIT_BOXGRP, m_33GRP);
	DDX_Control(pDX, IDC_BOX_11, m_nxeditBox11);
	DDX_Control(pDX, IDC_BOX_19, m_nxeditBox19);
	DDX_Control(pDX, IDC_EDIT_PRIOR_AUTH, m_nxeditEditPriorAuth);
	DDX_Control(pDX, IDC_17A_QUAL, m_nxedit17AQual);
	DDX_Control(pDX, IDC_33B_QUAL, m_nxedit33BQual);
	DDX_Control(pDX, IDC_NAME, m_nxeditName);
	DDX_Control(pDX, IDC_ADDRESS, m_nxeditAddress);
	DDX_Control(pDX, IDC_ZIP, m_nxeditZip);
	DDX_Control(pDX, IDC_CITY, m_nxeditCity);
	DDX_Control(pDX, IDC_STATE, m_nxeditState);
	DDX_Control(pDX, IDC_PHONE, m_nxeditPhone);
	DDX_Control(pDX, IDC_DEF_BATCH_LABEL, m_nxstaticDefBatchLabel);
	DDX_Control(pDX, IDC_DEF_BATCH_NOT_PRIMARY_LABEL, m_nxstaticDefBatchNotPrimaryLabel);
	DDX_Control(pDX, IDC_BOX33_NAME_LABEL, m_nxstaticBox33NameLabel);
	DDX_Control(pDX, IDC_BOX_33_ADDRESS_LABEL, m_nxstaticBox33AddressLabel);
	DDX_Control(pDX, IDC_O1STATIC, m_nxstaticO1static);
	DDX_Control(pDX, IDC_O2STATIC, m_nxstaticO2static);
	DDX_Control(pDX, IDC_O6STATIC, m_nxstaticO6static);
	DDX_Control(pDX, IDC_O3STATIC, m_nxstaticO3static);
	DDX_Control(pDX, IDC_O5STATIC, m_nxstaticO5static);
	DDX_Control(pDX, IDC_O4STATIC, m_nxstaticO4static);
	DDX_Control(pDX, IDC_BOX33_LOC_NPI_LABEL, m_nxstaticBox33LocNpiLabel);
	DDX_Control(pDX, IDC_RADIO_ACC_USE_BILL, m_radioAccUseBill);	
	DDX_Control(pDX, IDC_RADIO_ACC_USE_INSURED_PARTY, m_radioAccUseInsuredParty);
	DDX_Control(pDX, IDC_CHECK_SEND_TPL_IN_9A, m_checkUseTPLIn9a);
	DDX_Control(pDX, IDC_CHECK_USE_REF_PHY_GROUP_NPI, m_checkUseRefPhyGroupNPI);
	DDX_Control(pDX, IDC_CHECK_DONT_BATCH_SECONDARY, m_checkDontBatchSecondary);
	DDX_Control(pDX, IDC_BTN_EDIT_SECONDARY_EXCLUSIONS, m_btnSecondaryExclusions);
	DDX_Control(pDX, IDC_HCFA_UPGRADE_DATE_LABEL, m_lblUpgradeDate);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHcfaSetup, CNxDialog)
	//{{AFX_MSG_MAP(CHcfaSetup)
	ON_BN_CLICKED(IDC_ADD_ALL_COMPANIES, OnAddAll)
	ON_BN_CLICKED(IDC_ADD_COMPANY, OnAddCompany)
	ON_BN_CLICKED(IDC_REMOVE_COMPANY, OnRemoveCompany)
	ON_BN_CLICKED(IDC_REMOVE_ALL_COMPANIES, OnRemoveAllCompanies)
	ON_BN_CLICKED(IDC_RADIO_USEBILLPROV, On33BillProvider)
	ON_BN_CLICKED(IDC_RADIO_USEGEN1PROV, On33MainProvider)
	ON_BN_CLICKED(IDC_RADIO_USE_BILL_LOCATION, On33BillLocation)
	ON_BN_CLICKED(IDC_RADIO_USEOVRRDPROV, On33Override)
	ON_EN_CHANGE(IDC_PHONE, OnChangePhone)
	ON_BN_CLICKED(IDC_EDIT_INS_INFO, OnEditInsInfo)
	ON_BN_CLICKED(IDC_ALIGN_FORM, OnAlignForm)
	ON_BN_CLICKED(IDC_NEW_GROUP, OnNewGroup)
	ON_BN_CLICKED(IDC_DELETE_GROUP, OnDeleteGroup)	
	ON_BN_CLICKED(IDC_ADV_EBILLING_SETUP, OnAdvEbillingSetup)	
	ON_BN_CLICKED(IDC_BTN_HCFA_DATES, OnBtnHcfaDates)
	ON_BN_CLICKED(IDC_BTN_ADV_PIN_SETUP, OnBtnAdvPinSetup)
	ON_BN_CLICKED(IDC_CHECK_USE_EMPLOYER, OnCheckUseEmployer)
	ON_BN_CLICKED(IDC_ADV_POS_CODE_SETUP, OnAdvPosCodeSetup)		
	ON_BN_CLICKED(IDC_CHECK_REFPHYID_IN_BOX19, OnCheckRefphyidInBox19)	
	ON_BN_CLICKED(IDC_CHECK_DEFAULT_IN_BOX19, OnCheckDefaultInBox19)	
	ON_BN_CLICKED(IDC_BTN_DO_NOT_FILL, OnBtnDoNotFill)
	ON_BN_CLICKED(IDC_CHECK_USE_23_IN_17A, OnCheckUse23In17a)
	ON_BN_CLICKED(IDC_CHECK_USE_23_IN_17A_AND_NOT_23, OnCheckUse23In17aAndNot23)
	ON_BN_CLICKED(IDC_CHECK_USE_OVERRIDE_NAME_IN_BOX_31, OnCheckUseOverrideNameInBox31)
	ON_BN_CLICKED(IDC_CHECK_USE_1A_IN_9A, OnCheckUse1aIn9a)
	ON_BN_CLICKED(IDC_CHECK_HIDE_PHONE_NUM, OnCheckHidePhoneNum)
	ON_BN_CLICKED(IDC_CHECK_USE_1A_IN_9A_ALWAYS, OnCheckUse1aIn9aAlways)
	ON_BN_CLICKED(IDC_CHECK_PUT_SPACE_AFTER_QUALIFIERS, OnCheckPutSpaceAfterQualifiers)
	ON_BN_CLICKED(IDC_BTN_CONFIG_CLAIM_PROVIDERS, OnBtnConfigClaimProviders)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_CHECK_VALIDATE_REF_PHY, OnCheckValidateRefPhy)
	ON_BN_CLICKED(IDC_RADIO_ACC_USE_BILL, OnRadioAccUseBill)
	ON_BN_CLICKED(IDC_RADIO_ACC_USE_INSURED_PARTY, OnRadioAccUseInsuredParty)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_SEND_TPL_IN_9A, OnCheckSendTPLIn9a)
	ON_BN_CLICKED(IDC_CHECK_USE_REF_PHY_GROUP_NPI, OnCheckUseRefPhyGroupNpi)
	ON_BN_CLICKED(IDC_CHECK_DONT_BATCH_SECONDARY, OnCheckDontBatchSecondary)
	ON_BN_CLICKED(IDC_BTN_EDIT_SECONDARY_EXCLUSIONS, OnBtnEditSecondaryExclusions)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CHcfaSetup, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CHcfaSetup)
	ON_EVENT(CHcfaSetup, IDC_COMBO_GROUP_NAME, 1 /* SelectionChange */, OnChangeGroup, VTS_I4)
	ON_EVENT(CHcfaSetup, IDC_INSCO_IN, -601 /* DblClick */, OnDblClickInscoIn, VTS_NONE)
	ON_EVENT(CHcfaSetup, IDC_INSCO_OUT, -601 /* DblClick */, OnDblClickInscoOut, VTS_NONE)
	ON_EVENT(CHcfaSetup, IDC_HCFA_SELECTED, 3 /* DblClickCell */, OnDblClickCellHcfaSelected, VTS_I4 VTS_I2)
	ON_EVENT(CHcfaSetup, IDC_HCFA_UNSELECTED, 3 /* DblClickCell */, OnDblClickCellHcfaUnselected, VTS_I4 VTS_I2)
	ON_EVENT(CHcfaSetup, IDC_17A, 16 /* SelChosen */, OnSelChosen17a, VTS_I4)
	ON_EVENT(CHcfaSetup, IDC_33, 16 /* SelChosen */, OnSelChosen33, VTS_I4)	
	ON_EVENT(CHcfaSetup, IDC_BATCH_COMBO, 16 /* SelChosen */, OnSelChosenBatchCombo, VTS_I4)
	ON_EVENT(CHcfaSetup, IDC_SECONDARY_BATCH_COMBO, 16 /* SelChosen */, OnSelChosenSecondaryBatchCombo, VTS_I4)
	ON_EVENT(CHcfaSetup, IDC_HCFA_GROUPS, 16 /* SelChosen */, OnSelChosenHcfaGroups, VTS_I4)
	ON_EVENT(CHcfaSetup, IDC_BOX_24C_COMBO, 16 /* SelChosen */, OnSelChosenBox24CCombo, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHcfaSetup message handlers

void CHcfaSetup::AddItemTo33Combo(long id, CString item)
{
	IRowSettingsPtr pRow = m_33->GetRow(-1);
	pRow->Value[0] = id;
	pRow->Value[1] = _bstr_t(item);
	m_33->AddRow(pRow);
}

void CHcfaSetup::AddItemTo17aCombo(long id, CString item)
{
	IRowSettingsPtr pRow = m_17a->GetRow(-1);
	pRow->Value[0] = id;
	pRow->Value[1] = _bstr_t(item);
	m_17a->AddRow(pRow);
}

BOOL CHcfaSetup::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	m_addBtn.AutoSet(NXB_RIGHT);
	m_remBtn.AutoSet(NXB_LEFT);
	m_addAllBtn.AutoSet(NXB_RRIGHT);
	m_remAllBtn.AutoSet(NXB_LLEFT);

	m_newGroup.AutoSet(NXB_NEW);
	m_deleteGroup.AutoSet(NXB_DELETE);
	// (z.manning, 05/14/2008) - PLID 29566
	m_btnEditInsList.AutoSet(NXB_MODIFY);
	m_btnAlignForm.AutoSet(NXB_MODIFY);
	//// (j.jones 2011-04-05 14:48) - PLID 42372 - removed the CLIA setup
	m_btnAdvEbillingSetup.AutoSet(NXB_MODIFY);
	m_btnHCFADates.AutoSet(NXB_MODIFY);
	m_btnDoNotFillSetup.AutoSet(NXB_MODIFY);
	m_btnAdvPOSCodeSetup.AutoSet(NXB_MODIFY);
	// (j.jones 2008-08-01 10:46) - PLID 30918 - I removed the claim providers button
	// coloring, so now it is always modify
	m_btnConfigClaimProviders.AutoSet(NXB_MODIFY);
	// (z.manning, 05/14/2008) - PLID 29566 - Don't set a style for these buttons
	// because they change text color based on their data	
	//m_btnAdvPINSetup.AutoSet(NXB_MODIFY);	

	try {

		g_propManager.CachePropertiesInBulk("CHcfaSetup-1", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'FormatPhoneNums' "
			"OR Name = 'dontshow DontBatchSecondary' " // (j.jones 2013-07-18 09:33) - PLID 41823
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("CHcfaSetup-2", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'PhoneFormatString' "
			")",
			_Q(GetCurrentUserName()));

		m_pGroups = BindNxDataListCtrl(IDC_HCFA_GROUPS);
		m_17a = BindNxDataListCtrl(IDC_17A, false);
		m_33 = BindNxDataListCtrl(IDC_33, false);		
		m_BatchCombo = BindNxDataListCtrl(IDC_BATCH_COMBO, false);
		m_SecondaryBatchCombo = BindNxDataListCtrl(IDC_SECONDARY_BATCH_COMBO, false);

		IRowSettingsPtr pRow;

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

		m_Box24CCombo = BindNxDataListCtrl(IDC_BOX_24C_COMBO, false);
		pRow = m_Box24CCombo->GetRow(-1);
		pRow->PutValue(0, _bstr_t(""));
		m_Box24CCombo->AddRow(pRow);
		pRow = m_Box24CCombo->GetRow(-1);
		pRow->PutValue(0, _bstr_t("Y"));
		m_Box24CCombo->AddRow(pRow);
		pRow = m_Box24CCombo->GetRow(-1);
		pRow->PutValue(0, _bstr_t("N"));
		m_Box24CCombo->AddRow(pRow);

		m_pGroups->PutCurSel(0);
		
		m_pSelected = BindNxDataListCtrl(IDC_HCFA_SELECTED,false);

		m_pUnselected = BindNxDataListCtrl(IDC_HCFA_UNSELECTED);
	}NxCatchAll("Error in OnInitDialog()");
	
	Build17aCombo();

	Build33Combo();

	AddCustomInfo();

	((CNxEdit*)GetDlgItem(IDC_BOX_11))->SetLimitText(100);
	// (j.jones 2011-09-16 15:09) - PLID 40530 - I increased this to 80
	((CNxEdit*)GetDlgItem(IDC_BOX_19))->SetLimitText(80);
	((CNxEdit*)GetDlgItem(IDC_EDIT_PRIOR_AUTH))->SetLimitText(50);
	((CNxEdit*)GetDlgItem(IDC_ADDRESS))->SetLimitText(75);
	((CNxEdit*)GetDlgItem(IDC_ZIP))->SetLimitText(20);
	((CNxEdit*)GetDlgItem(IDC_CITY))->SetLimitText(50);
	((CNxEdit*)GetDlgItem(IDC_STATE))->SetLimitText(20);
	((CNxEdit*)GetDlgItem(IDC_EDIT_BOXGRP))->SetLimitText(50);
	((CNxEdit*)GetDlgItem(IDC_17A_QUAL))->SetLimitText(10);
	((CNxEdit*)GetDlgItem(IDC_33B_QUAL))->SetLimitText(10);

	// (j.jones 2013-08-05 11:46) - PLID 57805 - added link to the HCFA upgrade date setup
	m_lblUpgradeDate.SetText("");
	m_lblUpgradeDate.SetType(dtsDisabledHyperlink);

	return TRUE;
}

void CHcfaSetup::SetRadio (CButton &radioTrue, CButton &radioFalse, const bool isTrue)
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

void CHcfaSetup::Load() 
{
	try
	{
		if(m_pGroups->CurSel==-1)
			return;
		m_loading = true;
		
		long nGroupID = VarLong(m_pGroups->GetValue(m_pGroups->CurSel,0));
		CString strWhere;
		strWhere.Format("HCFASetupGroupID = %li", nGroupID);
		m_pSelected->WhereClause = (LPCTSTR)strWhere;
		m_pSelected->Requery();
		
		CHCFASetupInfo HCFAInfo(nGroupID);

		//now load the HCFASetupT record
	
		//Box25
		SetRadio (m_25ssn, m_25ein, HCFAInfo.Box25 == 1);

		//Box11Rule
		SetRadio (m_radioOverride, m_radioUseIfBlank, HCFAInfo.Box11Rule == 1);

		//Box17a
		m_17a->SetSelByColumn(0, HCFAInfo.Box17a);

		//Box33
		m_33->SetSelByColumn(0, HCFAInfo.Box33);

		//Box33Setup
		//1 = bill provider, 2 = General1, 3 = override, 4 = location
		switch (HCFAInfo.Box33Setup) {
			case 2:
				m_useBillProvider.SetCheck(false);
				m_useMainProvider.SetCheck(true);
				m_useBillLocation.SetCheck(false);
				m_overideProvider.SetCheck(false);				
				break;
			case 3:
				m_useBillProvider.SetCheck(false);
				m_useMainProvider.SetCheck(false);
				m_useBillLocation.SetCheck(false);
				m_overideProvider.SetCheck(true);
				break;
			case 4:
				m_useBillProvider.SetCheck(false);
				m_useMainProvider.SetCheck(false);
				m_useBillLocation.SetCheck(true);
				m_overideProvider.SetCheck(false);
				break;
			case 1:
			default:
				m_useBillProvider.SetCheck(true);
				m_useMainProvider.SetCheck(false);
				m_useBillLocation.SetCheck(false);
				m_overideProvider.SetCheck(false);
		}

		// (j.jones 2006-11-15 17:26) - PLID 23563 - converted the old
		// "Hide Override" / "Show Override" function into "ShowBox33Controls"
		ShowBox33Controls(HCFAInfo.Box33Setup);

		//Box33Num		
		try
		{
			// (j.jones 2009-06-09 17:19) - PLID 34557 - ensured the Box 33 controls are properly loaded
			if(HCFAInfo.Box33Num <= 0) {
				int nID = NewNumber("PersonT", "ID");
				ExecuteParamSql("INSERT INTO PersonT (ID) VALUES ({INT})", nID);
				UpdateTable("Box33Num",nID);
				HCFAInfo.Box33Num = nID;
			}

			_RecordsetPtr rs = CreateRecordset("SELECT ID, Note, Address1, City, WorkPhone, State, Zip FROM PersonT WHERE ID = %li", HCFAInfo.Box33Num);
			if (rs->eof) {
				ThrowNxException("Could not load Box33 override info!");
			}
			FieldsPtr Fields = rs->Fields;
			SetDlgItemVar(IDC_NAME,		Fields->GetItem("Note")->Value, true, true);
			SetDlgItemVar(IDC_ADDRESS,	Fields->GetItem("Address1")->Value, true, true);
			SetDlgItemVar(IDC_CITY,		Fields->GetItem("City")->Value, true, true);
			SetDlgItemVar(IDC_STATE,	Fields->GetItem("State")->Value, true, true);
			SetDlgItemVar(IDC_ZIP,		Fields->GetItem("Zip")->Value, true, true);
			SetDlgItemVar(IDC_PHONE,	Fields->GetItem("WorkPhone")->Value, true, true);
			rs->Close();

		}NxCatchAll("Error loading override info.");
	
		//Box33GRP
		m_33GRP.SetWindowText(HCFAInfo.Box33GRP);

		//Box11
		SetDlgItemText(IDC_BOX_11,HCFAInfo.Box11);
	
		//JMJ 5/14/2004 - this is obsolete, but still in the database for the time being.
		//It's just no longer configurable.
		//ExportFA8
		//m_checkExportFA8.SetCheck(HCFAInfo.ExportFA8);		

		//DefBatch
		switch(HCFAInfo.DefBatch) {
			case 0:
				m_BatchCombo->PutCurSel(2);
				break;
			case 1:
				m_BatchCombo->PutCurSel(0);
				break;
			case 2:
				m_BatchCombo->PutCurSel(1);
				break;
		}

		//DefBatchSecondary
		switch(HCFAInfo.DefBatchSecondary) {
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

		//Box 24C
		switch(HCFAInfo.Box24C) {
			case 0:
				m_Box24CCombo->PutCurSel(0);
				break;
			case 1:
				m_Box24CCombo->PutCurSel(1);
				break;
			case 2:
				m_Box24CCombo->PutCurSel(2);
				break;
		}

		//Box33Order
		SetRadio (m_radioLFM, m_radioFML, HCFAInfo.Box33Order == 1);

		//Box17Order
		SetRadio (m_radioRefLFM, m_radioRefFML, HCFAInfo.Box17Order == 1);
	
		//UseEmp
		m_checkUseEmployer.SetCheck(HCFAInfo.UseEmp);

		//HidePhoneBox33
		m_checkHidePhoneNum.SetCheck(HCFAInfo.HidePhoneBox33 == 1);

		//DefaultPriorAuthNum
		SetDlgItemText(IDC_EDIT_PRIOR_AUTH,HCFAInfo.DefaultPriorAuthNum);

		//DocAddress
		SetRadio (m_radioUseLocationAddr, m_radioUseContactAddr, HCFAInfo.DocAddress == 1);

		//Box19RefPhyID
		m_checkBox19RefPhyID.SetCheck(HCFAInfo.Box19RefPhyID);

		//Box19Override
		m_checkOverrideBox19.SetCheck(HCFAInfo.Box19Override);
		GetDlgItem(IDC_BOX_19)->EnableWindow(m_checkOverrideBox19.GetCheck());

		//Box19
		SetDlgItemText(IDC_BOX_19,HCFAInfo.Box19);

		//UseBox23InBox17a
		m_checkUse23In17a.SetCheck(HCFAInfo.UseBox23InBox17a != 0);
		OnCheckUse23In17a();
		m_checkUse23In17aAndNotIn23.SetCheck(HCFAInfo.UseBox23InBox17a == 2);

		//UseOverrideInBox31
		m_checkUseOverrideInBox31.SetCheck(HCFAInfo.UseOverrideInBox31);

		//Use1aIn9a
		m_checkUse1aIn9a.SetCheck(HCFAInfo.Use1aIn9a);
		GetDlgItem(IDC_CHECK_USE_1A_IN_9A_ALWAYS)->ShowWindow(m_checkUse1aIn9a.GetCheck());

		// (j.jones 2010-08-31 16:46) - PLID 40303 - supported TPLIn9a
		m_checkUseTPLIn9a.SetCheck(HCFAInfo.TPLIn9a);

		//Box17aQual
		SetDlgItemText(IDC_17A_QUAL,HCFAInfo.Box17aQual);

		//Box33bQual
		SetDlgItemText(IDC_33B_QUAL,HCFAInfo.Box33bQual);

		// (j.jones 2010-04-16 09:01) - PLID 38149 - track the old 33b qualifier
		m_strOldBox33bQualifier = HCFAInfo.Box33bQual;

		//Use1aIn9aAlways
		m_checkUse1aIn9aAlways.SetCheck(HCFAInfo.Use1aIn9aAlways);

		// (j.jones 2006-11-15 17:24) - PLID 23563 - added LocationNPIUsage

		//LocationNPIUsage
		SetRadio(m_radioUseLocNPI, m_radioUseProvNPI, HCFAInfo.LocationNPIUsage == 1);

		// (j.jones 2007-02-21 14:55) - PLID 24776 - supported option to put a space between qualifiers and IDs
		m_checkPutSpaceAfterQualifiers.SetCheck(HCFAInfo.QualifierSpace == 1);

		// (j.jones 2007-02-21 13:19) - PLID 24580 - colorized the Adv PIN Setup button when in use
		UpdateAdvancedPINSetupAppearance();

		// (j.jones 2008-06-10 14:50) - PLID 25834 - added m_checkValidateRefPhy
		m_checkValidateRefPhy.SetCheck(HCFAInfo.ValidateRefPhy == 1);

		// (j.jones 2009-03-06 11:21) - PLID 33354 - added m_radioAccUseBill and m_radioAccUseInsuredParty
		//1 - pull from Bill
		//2 - pull from Insured Party
		m_radioAccUseBill.SetCheck(HCFAInfo.PullCurrentAccInfo == 1);
		m_radioAccUseInsuredParty.SetCheck(HCFAInfo.PullCurrentAccInfo == 2);

		// (j.jones 2013-06-10 16:26) - PLID 56255 - added UseRefPhyGroupNPI
		m_checkUseRefPhyGroupNPI.SetCheck(HCFAInfo.UseRefPhyGroupNPI == 1);

		// (j.jones 2013-07-17 16:06) - PLID 41823 - added DontBatchSecondary
		m_checkDontBatchSecondary.SetCheck(HCFAInfo.DontBatchSecondary == 1);
		m_btnSecondaryExclusions.EnableWindow(HCFAInfo.DontBatchSecondary == 1);

		// (j.jones 2013-08-05 11:46) - PLID 57805 - added link to the HCFA upgrade date setup
		LoadHCFAUpgradeDateLabel();
		
		m_loading = false;

	}NxCatchAll("Error in Load()");
}

void CHcfaSetup::OnChangeGroup(long iNewRow) 
{
	Load();
}

void CHcfaSetup::OnDblClickInscoIn() 
{
	OnRemoveCompany();
}

void CHcfaSetup::OnDblClickInscoOut() 
{
	OnAddCompany();
}

void CHcfaSetup::OnAddAll() 
{
	try{
		if(m_pGroups->CurSel==-1)
			return;

		CWaitCursor pWait;

		ExecuteSql("UPDATE InsuranceCoT SET HCFASetupGroupID = %li WHERE (HCFASetupGroupID = 0 OR HCFASetupGroupID Is Null) AND PersonID NOT IN (SELECT ID FROM PersonT WHERE Archived = 1)", m_pGroups->GetValue(m_pGroups->CurSel,0).lVal);

		//audit each row
		long nAuditID = BeginNewAuditEvent();
		for(int i=0; i<m_pUnselected->GetRowCount();i++) {
			AuditEvent(-1, CString(m_pUnselected->GetValue(i, 1).bstrVal), nAuditID, aeiInsCoHCFA, long(m_pUnselected->GetValue(i, 0).lVal), "<No HCFA Group>", CString(m_pGroups->GetValue(m_pGroups->CurSel,1).bstrVal), aepMedium, aetChanged);
		}

		m_pSelected->TakeAllRows(m_pUnselected);

		// (j.jones 2013-08-05 14:09) - PLID 57805 - refresh the HCFA upgrade label, these companies might have different dates now
		LoadHCFAUpgradeDateLabel();

	}NxCatchAll("Error in OnAddAll()");
}

void CHcfaSetup::OnAddCompany() 
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

			ExecuteSql("UPDATE InsuranceCoT SET HCFASetupGroupID = %li WHERE InsuranceCoT.PersonID = %li;", m_pGroups->GetValue(m_pGroups->CurSel,0).lVal, pRow->GetValue(0).lVal);

			//audit
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, CString(pRow->GetValue(1).bstrVal), nAuditID, aeiInsCoHCFA, long(pRow->GetValue(0).lVal), "<No HCFA Group>", CString(m_pGroups->GetValue(m_pGroups->CurSel,1).bstrVal), aepMedium, aetChanged);

			pDisp->Release();
		}

		m_pSelected->TakeCurrentRow(m_pUnselected);

		// (j.jones 2013-08-05 14:09) - PLID 57805 - refresh the HCFA upgrade label, these companies might have different dates now
		LoadHCFAUpgradeDateLabel();

	}NxCatchAll("Error in OnAddCompany()");
}

void CHcfaSetup::OnRemoveCompany() 
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
				ExecuteSql("UPDATE InsuranceCoT SET HCFASetupGroupID = NULL WHERE PersonID = %li;", pRow->GetValue(0).lVal);

				//audit
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, CString(pRow->GetValue(1).bstrVal), nAuditID, aeiInsCoHCFA, long(pRow->GetValue(0).lVal), CString(m_pGroups->GetValue(m_pGroups->CurSel,1).bstrVal), "<No HCFA Group>", aepMedium, aetChanged);
			}

			pDisp->Release();
		}

		m_pUnselected->TakeCurrentRow(m_pSelected);

		// (j.jones 2013-08-05 14:09) - PLID 57805 - refresh the HCFA upgrade label, these companies might have different dates now
		LoadHCFAUpgradeDateLabel();

	}NxCatchAll("Error in OnRemoveCompany");
}

void CHcfaSetup::OnRemoveAllCompanies() 
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

			ExecuteSql("UPDATE InsuranceCoT SET HCFASetupGroupID = NULL WHERE PersonID = %li;", pRow->GetValue(0).lVal);

			pDisp->Release();
		}

		//audit each row
		long nAuditID = BeginNewAuditEvent();
		for(i=0; i<m_pSelected->GetRowCount();i++) {
			AuditEvent(-1, CString(m_pSelected->GetValue(i, 1).bstrVal), nAuditID, aeiInsCoHCFA, long(m_pSelected->GetValue(i, 0).lVal), CString(m_pGroups->GetValue(m_pGroups->CurSel,1).bstrVal), "<No HCFA Group>", aepMedium, aetChanged);
		}

		m_pUnselected->TakeAllRows(m_pSelected);

		// (j.jones 2013-08-05 14:09) - PLID 57805 - refresh the HCFA upgrade label, these companies might have different dates now
		LoadHCFAUpgradeDateLabel();

	}NxCatchAll("Error in OnRemoveAllCompanies()");
}

BOOL CHcfaSetup::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD	nID;
	long	value;
	CString box,
			text;
	_RecordsetPtr rs;

	switch (HIWORD(wParam))
	{	case BN_CLICKED:
		{	switch (nID = LOWORD(wParam))
			{
				case IDC_RADIO_EIN:
				case IDC_RADIO_USE_IF_BLANK:
				case IDC_RADIO_USE_CONTACT_ADDR:
				case IDC_RADIO_USE_PROV_NPI:
					value = 2;
					break;
				case IDC_RADIO_SSN:
				case IDC_RADIO_OVERRIDE:
				case IDC_RADIO_USE_LOC_ADDR:
				case IDC_RADIO_USE_LOC_NPI:
					value = 1;
					break;
				case IDC_RADIO_LFM:
				case IDC_RADIO_REF_LFM:
					value = 1;
					break;
				case IDC_RADIO_FML:
				case IDC_RADIO_REF_FML:
					value = 2;
					break;
				default: return CNxDialog::OnCommand(wParam, lParam);//handles anything else here
			}
			switch (nID = LOWORD(wParam))
			{
				case IDC_RADIO_EIN:
				case IDC_RADIO_SSN:
					box = "Box25";
					break;
				case IDC_RADIO_OVERRIDE:
				case IDC_RADIO_USE_IF_BLANK:
					box = "Box11Rule";
					break;
				case IDC_RADIO_LFM:
				case IDC_RADIO_FML:
					box = "Box33Order";
					break;
				case IDC_RADIO_REF_LFM:
				case IDC_RADIO_REF_FML:
					box = "Box17Order";
					break;
				case IDC_RADIO_USE_LOC_ADDR:
				case IDC_RADIO_USE_CONTACT_ADDR:
					box = "DocAddress";
					break;
				case IDC_RADIO_USE_LOC_NPI:
				case IDC_RADIO_USE_PROV_NPI:
					box = "LocationNPIUsage";
					break;
			}
			try
			{
				if(m_pGroups->CurSel==-1)
					return CNxDialog::OnCommand(wParam, lParam);

				long nRecordsAffected = UpdateTable(box,value);
				
				if(nRecordsAffected != 0) {

					// (j.jones 2009-03-02 15:35) - PLID 31913 - added auditing for HCFA Box 25 setup
					if(nID == IDC_RADIO_EIN || nID == IDC_RADIO_SSN) {

						if(m_pGroups->CurSel != -1) {

							long nGroupID = VarLong(m_pGroups->GetValue(m_pGroups->CurSel,0));
							CString strGroupName = VarString(m_pGroups->GetValue(m_pGroups->CurSel,1));

							//since nRecordsAffected tells us if anything changed, we know that we
							//can't reach this code unless the setting was truly flipped, in which
							//case we know what the opposite setting was without having to run a
							//recordset
							CString strOld, strNew;
							if(nID == IDC_RADIO_EIN) {
								strOld = "SSN";
								strNew = "EIN";
							}
							else {
								strOld = "EIN";
								strNew = "SSN";
							}

							long nAuditID = -1;
							nAuditID = BeginNewAuditEvent();
							if(nAuditID != -1) {
								AuditEvent(-1, strGroupName, nAuditID, aeiHCFA_Box25, nGroupID, strOld, strNew, aepMedium, aetChanged); 
							}
						}
						else {
							//saving wouldn't have done anything anyways, so we can simply log this
							ASSERT(FALSE);
							Log("Could not audit Box 25 change because no HCFA group is selected.");
						}
					}else{
						// (j.dinatale 2012-04-12 17:10) - PLID 49314 - auditing for box 33
						if(nID == IDC_RADIO_USE_PROV_NPI || nID == IDC_RADIO_USE_LOC_NPI) {
							if(m_pGroups->CurSel != -1) {
								long nGroupID = VarLong(m_pGroups->GetValue(m_pGroups->CurSel,0));
								CString strGroupName = VarString(m_pGroups->GetValue(m_pGroups->CurSel,1));

								CString strOld, strNew;
								if(nID == IDC_RADIO_USE_PROV_NPI) {
									strOld = "Location NPI";
									strNew = "Provider NPI";
								}
								else {
									strOld = "Provider NPI";
									strNew = "Location NPI";
								}

								long nAuditID = -1;
								nAuditID = BeginNewAuditEvent();
								if(nAuditID != -1) {
									AuditEvent(-1, strGroupName, nAuditID, aeiHCFA_Box33NPI, nGroupID, strOld, strNew, aepMedium, aetChanged); 
								}
							}
							else {
								//saving wouldn't have done anything anyways, so we can simply log this
								ASSERT(FALSE);
								Log("Could not audit Box 33 change because no HCFA group is selected.");
							}
						}
					}
				}
				
			}NxCatchAll("Error in OnCommand()");
			break;
		case EN_CHANGE://they may want weird, lower case stuff
			// (d.moore 2007-04-23 12:11) - PLID 23118 - 
			//  Capitalize letters in the zip code as they are typed in. Canadian postal
			//    codes need to be formatted this way.
			nID = LOWORD(wParam);
			if (nID == IDC_ZIP) {
				CapitalizeAll(IDC_ZIP);
			}
			m_changed = true;
			break;
		case EN_KILLFOCUS:
			switch (nID = LOWORD(wParam))
			{	case IDC_ZIP:
				{
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					_RecordsetPtr rs = CreateRecordset("SELECT Box33Num FROM HCFASetupT WHERE ID = %li", m_pGroups->GetValue(m_pGroups->CurSel,0).lVal);
					StoreZipBoxes(this, IDC_CITY, IDC_STATE, IDC_ZIP, 
						"PersonT", "ID", rs->Fields->GetItem("Box33Num")->Value.lVal,
						"City", "State", "Zip");
					rs->Close();
					m_changed = false;
					break;
				}
				case IDC_EDIT_BOXGRP:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					m_33GRP.GetWindowText(text);
					if (text.IsEmpty())
						text = "";
					try {

						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE HCFASetupT SET Box33GRP = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_BOX_11:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(IDC_BOX_11,text);
					if (text.IsEmpty())
						text = "";
					try {

						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE HCFASetupT SET Box11 = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_EDIT_PRIOR_AUTH:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(IDC_EDIT_PRIOR_AUTH,text);
					if (text.IsEmpty())
						text = "";
					try {

						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE HCFASetupT SET DefaultPriorAuthNum = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_BOX_19:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(IDC_BOX_19,text);
					if (text.IsEmpty())
						text = "";
					try {

						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE HCFASetupT SET Box19 = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_17A_QUAL:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(IDC_17A_QUAL,text);
					if (text.IsEmpty())
						text = "";
					try {

						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE HCFASetupT SET Box17aQual = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
					break;
				case IDC_33B_QUAL:
					if(m_pGroups->CurSel==-1)
						return CNxDialog::OnCommand(wParam, lParam);
					GetDlgItemText(IDC_33B_QUAL,text);
					text.TrimLeft();
					text.TrimRight();
					SetDlgItemText(IDC_33B_QUAL,text);
					try {

						// (j.jones 2010-04-16 09:01) - PLID 38149 - warn if the qualifier has been changed to XX
						if(m_strOldBox33bQualifier.CompareNoCase(text) != 0 && text.CompareNoCase("XX") == 0) {
							if(IDNO == MessageBox("The Box33b qualifier is XX, which is not a valid qualifier to use.\n"
								"This configuration may cause your claims to be rejected.\n\n"
								"Are you sure you want to send XX as a Box 33b qualifier?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
								SetDlgItemText(IDC_33B_QUAL,m_strOldBox33bQualifier);
								return CNxDialog::OnCommand(wParam, lParam);;
							}
						}
						m_strOldBox33bQualifier = text;

						long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
						ExecuteSql("UPDATE HCFASetupT SET Box33bQual = '%s' WHERE ID = %li", _Q(text), GroupID);

					}NxCatchAll("Error in OnCommand()");
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

void CHcfaSetup::Save(int nID)
{
	if(m_pGroups->CurSel==-1)
		return;

	CString field, value;
	switch (nID)
	{	case IDC_NAME:
			field = "Note";
			break;
		case IDC_ADDRESS:
			field = "Address1";
			break;
		case IDC_CITY:
			field = "City";
			break;
		case IDC_STATE:
			field = "State";
			break;
		case IDC_ZIP:
			field = "Zip";
			break;
		case IDC_PHONE:
			field = "WorkPhone";
			break;
		default:
			return;
	}
	GetDlgItemText (nID, value);
	value.TrimRight();
	try{
		_RecordsetPtr rs = CreateRecordset("SELECT Box33Num FROM HCFASetupT WHERE ID = %li", m_pGroups->GetValue(m_pGroups->CurSel,0).lVal);
		ExecuteSql("UPDATE PersonT SET %s = '%s' WHERE ID = %li", field, _Q(value), rs->Fields->GetItem("Box33Num")->Value.lVal);
		rs->Close();
	}NxCatchAll("Error in Save()");
	m_changed = false;
}

// (j.jones 2006-11-15 17:26) - PLID 23563 - converted the old
// "Hide Override" / "Show Override" function into "ShowBox33Controls"
void CHcfaSetup::ShowBox33Controls(long nBox33Type) {

	//first hide/show the override controls

	int nCmdShow = nBox33Type == 3 ? SW_SHOW : SW_HIDE;

	GetDlgItem(IDC_O1STATIC)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_O2STATIC)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_O3STATIC)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_O4STATIC)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_O5STATIC)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_O6STATIC)->ShowWindow(nCmdShow);

	GetDlgItem(IDC_NAME)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_ADDRESS)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_CITY)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_STATE)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_ZIP)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_PHONE)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_CHECK_USE_OVERRIDE_NAME_IN_BOX_31)->ShowWindow(nCmdShow);

	//now decide whether to show or hide the provider fields
	switch(nBox33Type) {

		case 1:	//bill provider
		case 2:	//G1 provider
			GetDlgItem(IDC_BOX33_NAME_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_RADIO_LFM)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_RADIO_FML)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BOX_33_ADDRESS_LABEL)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_RADIO_USE_LOC_ADDR)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_RADIO_USE_CONTACT_ADDR)->ShowWindow(SW_SHOW);
			break;

		case 3:	//override
		case 4:	//bill location
			GetDlgItem(IDC_BOX33_NAME_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_LFM)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_FML)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BOX_33_ADDRESS_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_USE_LOC_ADDR)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_USE_CONTACT_ADDR)->ShowWindow(SW_HIDE);
			break;
	}

	// (j.jones 2007-01-18 11:32) - PLID 24264 - we decided to always use the NPI controls
	/*
	//decide whether to show the location NPI fields

	nCmdShow = nBox33Type == 4 ? SW_SHOW : SW_HIDE;

	GetDlgItem(IDC_BOX33_LOC_NPI_LABEL)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_RADIO_USE_LOC_NPI)->ShowWindow(nCmdShow);
	GetDlgItem(IDC_RADIO_USE_PROV_NPI)->ShowWindow(nCmdShow);
	*/
}

void CHcfaSetup::On33BillProvider() 
{
	if(m_pGroups->CurSel==-1)
		return;

	ShowBox33Controls(1);
	_RecordsetPtr rs;
	
	try{
		
		UpdateTable("Box33Setup",1);
		
	}NxCatchAll("Error in On33BillProvider()");
}

void CHcfaSetup::On33MainProvider() 
{
	if(m_pGroups->CurSel==-1)
		return;

	ShowBox33Controls(2);
	_RecordsetPtr rs;
	
	try{
		
		UpdateTable("Box33Setup",2);
		
	}NxCatchAll("Error in On33MainProvider()");
}

void CHcfaSetup::On33BillLocation() 
{
	if(m_pGroups->CurSel==-1)
		return;

	ShowBox33Controls(4);
	_RecordsetPtr rs;
	
	try{

		UpdateTable("Box33Setup",4);

	}NxCatchAll("Error in On33BillLocation()");
}

void CHcfaSetup::On33Override() 
{
	if(m_pGroups->CurSel==-1)
		return;

	ShowBox33Controls(3);
	_RecordsetPtr rs;
	
	try{
		
		UpdateTable("Box33Setup",3);
		
	}NxCatchAll("Error in On33Override()");

	try{
		rs = CreateRecordset("SELECT Box33Num FROM HCFASetupT WHERE ID = %li", m_pGroups->GetValue(m_pGroups->CurSel,0).lVal);
		if(rs->eof || rs->Fields->GetItem("Box33Num")->Value.vt != VT_I4 || rs->Fields->GetItem("Box33Num")->Value.lVal <= 0)
		{	
			int nID = NewNumber("PersonT", "ID");
			ExecuteSql("INSERT INTO PersonT (ID) VALUES (%li)", nID);
			UpdateTable("Box33Num",nID);
		}
		rs->Close();
	}NxCatchAll("Error in On33Override()");
	Load();
}

void CHcfaSetup::OnDblClickCellHcfaSelected(long nRowIndex, short nColIndex) 
{
	OnRemoveCompany();
}

void CHcfaSetup::OnDblClickCellHcfaUnselected(long nRowIndex, short nColIndex) 
{
	OnAddCompany();
}

void CHcfaSetup::OnChangePhone() 
{
	CString str;
	GetDlgItemText(IDC_PHONE, str);
	str.TrimRight();
	if (str != "") {
		if(GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true) == 1) {
			FormatItem (IDC_PHONE, GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true));
		}
	}
}

void CHcfaSetup::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
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

	if (m_labelChecker.Changed())
	{
		m_17a->Clear();
		Build17aCombo();
		
		m_33->Clear();
		Build33Combo();
		
		AddCustomInfo();
	}

	if (m_groupsChecker.Changed())
	{
		_variant_t varTmp;
		if(m_pGroups->CurSel != -1)
			varTmp = m_pGroups->GetValue(m_pGroups->CurSel, 0);

		m_pGroups->Requery();
		
		int nRow = m_pGroups->SetSelByColumn(0, varTmp);		
		if(nRow == -1) {
			m_pGroups->CurSel = 0;
			OnSelChosenHcfaGroups(0);
		}
		else {
			OnSelChosenHcfaGroups(nRow);
		}
	}
	else {
		Load();
	}
}

void CHcfaSetup::OnEditInsInfo() 
{
	CEditInsInfoDlg EditInsInfo(this);

	if (CheckCurrentUserPermissions(bioInsuranceCo,sptWrite))
	{
		EditInsInfo.DisplayWindow(BACKGROUND_COLOR);
		UpdateView();
	}
}

void CHcfaSetup::OnAlignForm() 
{
	CPrintAlignDlg dlg(this);
	dlg.DoModal();	
}

void CHcfaSetup::OnNewGroup() 
{
	CWaitCursor pWait;

	CString strResult, sql;
	int nResult = InputBoxLimited(this, "Enter Name", strResult, "",100,false,false,NULL);
	long id;

	if (nResult == IDOK && strResult != "")
	{	
		try {

			// (j.jones 2009-11-24 15:41) - PLID 36411 - default PriorAuthQualifier to G1, it defaults to blank in the database
			// (j.jones 2010-03-02 16:20) - PLID 37584 - PriorAuthQualifier is now blank by default
			ExecuteParamSql("INSERT INTO HCFASetupT (ID, Name) VALUES ({INT}, {STRING}) ", id = NewNumber("HCFASetupT", "ID"), strResult);

			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, "", nAuditID, aeiHcfaGroupCreate, id, "", strResult, aepMedium, aetCreated);

		}NxCatchAll("Error creating new group.");

		m_pGroups->Requery();
		m_pGroups->SetSelByColumn(1,_bstr_t(strResult));
		Load();

		m_groupsChecker.Refresh();

		if (IDYES == AfxMessageBox(IDS_HCFA_ADDALL, MB_YESNO | MB_ICONQUESTION))
			OnAddAll();
	}	
}

void CHcfaSetup::OnDeleteGroup() 
{
	CString sql;

	if(m_pGroups->CurSel==-1)
		return;	

	if(IDNO==MessageBox("Are you sure you wish to delete this HCFA Group?","Practice",MB_YESNO|MB_ICONQUESTION))
		return;

	BEGIN_TRANS("DeleteHCFAGroup") {
		CString strOld = CString(m_pGroups->GetValue(m_pGroups->CurSel, 1).bstrVal);

		long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;

		//this will audit the changes
		OnRemoveAllCompanies();

		ExecuteParamSql("UPDATE InsuranceCoT SET HCFASetupGroupID = NULL WHERE HCFASetupGroupID = {INT}", GroupID);
		// (j.jones 2013-07-18 09:25) - PLID 41823 - delete from HCFASecondaryExclusionsT
		ExecuteParamSql("DELETE FROM HCFASecondaryExclusionsT WHERE HCFASetupID = {INT}", GroupID);
		ExecuteParamSql("DELETE FROM AdvHCFAPinT WHERE SetupGroupID = {INT}", GroupID);
		// (j.jones 2010-02-03 09:01) - PLID 37159 - delete from HCFA_EbillingAllowedAdjCodesT
		ExecuteParamSql("DELETE FROM HCFA_EbillingAllowedAdjCodesT WHERE HCFASetupID = {INT}", GroupID);
		ExecuteParamSql("DELETE FROM POSLocationLinkT WHERE HCFASetupGroupID = {INT}", GroupID);
		ExecuteParamSql("DELETE FROM EbillingSetupT WHERE SetupGroupID = {INT}", GroupID);
		// (j.jones 2008-06-23 10:21) - PLID 30434 - added EligibilitySetupT
		ExecuteParamSql("DELETE FROM EligibilitySetupT WHERE HCFASetupID = {INT}", GroupID);
		// (a.wilson 2014-5-5) PLID 61831 - clear out ChargeLevelProviderConfigT where hcfagroup was used.
		ExecuteParamSql("DELETE FROM ChargeLevelProviderConfigT WHERE HCFAGroupID = {INT}", GroupID);
		// (j.jones 2008-04-02 14:30) - PLID 28995 - ensure we delete from HCFAClaimProvidersT
		// (j.jones 2008-08-01 10:25) - PLID 30917 - not anymore, HCFAClaimProvidersT is per company now
		ExecuteParamSql("DELETE FROM HCFASetupT WHERE ID = {INT}", GroupID);
		
		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiHcfaGroupDelete, GroupID, strOld, "<Deleted>", aepMedium, aetDeleted);

	} END_TRANS_CATCH_ALL("DeleteHCFAGroup");

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

void CHcfaSetup::OnSelChosen17a(long nRow) 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try	{
		if(m_17a->CurSel == -1)
			m_17a->CurSel = 0;

		long nBox17a = VarLong(m_17a->Value[m_17a->CurSel][0]);		

		UpdateTable("Box17a",nBox17a);

		//calculate the ideal qualifier
		CString strQual = "";
		switch(nBox17a) {
		case 1: { //NPI
			//XX - Referring NPI Number
			strQual = "XX";
			break;
		}
		case 3: { //Referring UPIN

			//1G - Provider UPIN Number
			strQual = "1G";
			break;
		}
		case 4: { //Referring Blue Shield

			// (b.spivey February 19, 2015) - PLID 56651 - Default to G2
			//1B - BCBS Provider Number
			strQual = "G2";
			break;
		}
		case 5:	//Referring FedEmployerID

			// (b.spivey February 19, 2015) - PLID 56651 - Default to G2
			//EI - Employer's Identification Number
			strQual = "G2";
			break;
		case 6:	//Referring DEANumber

			//0B - State License Number
			strQual = "0B";
			break;
		case 7:	//Referring MedicareNumber

			//1C - Medicare Provider Number
			// (b.spivey February 19, 2015) - PLID 56651 - Default to G2
			strQual = "G2";
			break;
		case 8:	//Referring MedicaidNumber

			// (b.spivey February 19, 2015) - PLID 56651 - Default to G2
			//1D - Medicaid Provider Number
			strQual = "G2";
			break;
		// (j.jones 2007-04-24 12:27) - PLID 25764 - supported Taxonomy Code
		// (j.jones 2012-03-07 15:58) - PLID 48676 - removed Taxonomy Code
		/*
		case 12: //Referring Taxonomy Code

			//ZZ - Taxonomy Code
			strQual = "ZZ";
			break;
		*/
		default:
			strQual = "";
			break;
		}

		CString strExistingQual;
		GetDlgItemText(IDC_17A_QUAL, strExistingQual);
		if(strQual.CompareNoCase(strExistingQual) != 0) {
			//they don't match, so use the new qualifier
			SetDlgItemText(IDC_17A_QUAL, strQual);
			ExecuteSql("UPDATE HCFASetupT SET Box17aQual = '%s' WHERE ID = %li", _Q(strQual), m_pGroups->GetValue(m_pGroups->CurSel,0).lVal);
		}

		if(nBox17a != -1 && strQual.IsEmpty()) {
			AfxMessageBox("There is no default qualifier for the selected Box 17a number. Please manually enter a qualifier before submitting claims.\n"
				"If you are unsure of what qualifier to use, contact the insurance company to receive the correct qualifier.");
		}
	}
	NxCatchAll("Error in OnChange17a()");	
}

void CHcfaSetup::OnSelChosen33(long nRow) 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try {
		if(m_33->CurSel == -1)
			m_33->CurSel = 0;

		long nBox33 = VarLong(m_33->Value[m_33->CurSel][0]);

		UpdateTable("Box33",nBox33);

		//calculate the ideal qualifier
		CString strQual = "";
		switch(nBox33) {
		case 1: { //NPI

			//XX - NPI Number
			strQual = "XX";
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
		case 4: { //DEA Number

			//0B - State License Number
			strQual = "0B";
			break;
		}
		case 5: { //BCBS Number
			
			//1B - Blue Cross Provider Number
			strQual = "1B";
			break;
		}
		case 6: { //Medicare Number

			//1C - Medicare Provider Number
			strQual = "1C";
			break;
		}
		case 7: { //Medicaid Number

			//1D - Medicaid Provider Number
			strQual = "1D";
			break;
		}		
		case 11: { //UPIN

			//1G - Provider UPIN Number
			strQual = "1G";
			break;
		}
		// (j.jones 2007-04-24 12:28) - PLID 25764 - supported Taxonomy Code
		case 14: { //Provider Taxonomy Code

			//ZZ - Taxonomy Code
			strQual = "ZZ";
			break;
		}
		default:
			strQual = "";
			break;
		}

		CString strExistingQual;
		GetDlgItemText(IDC_33B_QUAL, strExistingQual);
		if(strQual.CompareNoCase(strExistingQual) != 0) {
			//they don't match, so use the new qualifier
			SetDlgItemText(IDC_33B_QUAL, strQual);
			m_strOldBox33bQualifier = strQual;
			ExecuteSql("UPDATE HCFASetupT SET Box33bQual = '%s' WHERE ID = %li", _Q(strQual), m_pGroups->GetValue(m_pGroups->CurSel,0).lVal);
		}

		if(nBox33 != -1 && strQual.IsEmpty()) {
			AfxMessageBox("There is no default qualifier for the selected Box 33b number. Please manually enter a qualifier before submitting claims.\n"
				"If you are unsure of what qualifier to use, contact the insurance company to receive the correct qualifier.");
		}

		// (j.jones 2010-04-16 09:07) - PLID 38149 - yell if they picked NPI
		if(nBox33 == 1) {
			AfxMessageBox("Using NPI in Box33b will likely cause your claims to reject.\n"
				"Please select another ID other than NPI unless you are absolutely certain the NPI is needed here.");
		}
	}
	NxCatchAll("Error in OnChange33()");	
}

void CHcfaSetup::Build17aCombo()
{
	AddItemTo17aCombo(-1, " { Do Not Fill } ");
	AddItemTo17aCombo(1, "NPI");
	AddItemTo17aCombo(2, "ID Number");
	AddItemTo17aCombo(3, "UPIN");
	AddItemTo17aCombo(4, "Blue Shield ID");

	AddItemTo17aCombo(5, "Fed Employer ID");
	AddItemTo17aCombo(6, "DEA Number");
	AddItemTo17aCombo(7, "Medicare Number");
	AddItemTo17aCombo(8, "Medicaid Number");
	AddItemTo17aCombo(9, "Workers Comp Number");
	AddItemTo17aCombo(10, "Other ID Number");
	AddItemTo17aCombo(11, "License");

	// (j.jones 2007-04-24 11:56) - PLID 25764 - supported Taxonomy Code
	// (j.jones 2012-03-07 15:58) - PLID  48676 - removed Taxonomy Code
	//AddItemTo17aCombo(12, "Taxonomy Code");
}

void CHcfaSetup::Build33Combo()
{
	AddItemTo33Combo(-1, " { Do Not Fill } ");
	AddItemTo33Combo(1, "NPI");
	AddItemTo33Combo(2, "Social Security Number");
	AddItemTo33Combo(3, "Fed Employer ID");
	AddItemTo33Combo(4, "DEA Number");
	AddItemTo33Combo(5, "BCBS Number");
	AddItemTo33Combo(6, "Medicare Number");
	AddItemTo33Combo(7, "Medicaid Number");
	AddItemTo33Combo(8, "Workers Comp Number");
	AddItemTo33Combo(9, "Other ID Number");
	AddItemTo33Combo(11, "UPIN");
	AddItemTo33Combo(12, "Box24J");
	// (j.jones 2007-01-16 15:24) - PLID 24273 - added support for old GRP number
	AddItemTo33Combo(13, "GRP Number");
	// (j.jones 2007-04-24 11:56) - PLID 25764 - supported Taxonomy Code
	AddItemTo33Combo(14, "Taxonomy Code");
}

void CHcfaSetup::AddCustomInfo()
{
	_RecordsetPtr rs;
	_variant_t var;
	CString str;

	try {

		rs = CreateRecordset("SELECT Name FROM CustomFieldsT WHERE ID = 6");
		if(!rs->eof) {
			var = rs->Fields->Item["Name"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "Custom Contact Info 1";
		}
		else
			str = "Custom Contact Info 1";
		rs->Close();

		AddItemTo17aCombo(16, str);
		AddItemTo33Combo(16, str);

		rs = CreateRecordset("SELECT Name FROM CustomFieldsT WHERE ID = 7");
		if(!rs->eof) {
			var = rs->Fields->Item["Name"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "Custom Contact Info 2";
		}
		else
			str = "Custom Contact Info 2";
		rs->Close();

		AddItemTo17aCombo(17, str);
		AddItemTo33Combo(17, str);

		rs = CreateRecordset("SELECT Name FROM CustomFieldsT WHERE ID = 8");
		if(!rs->eof) {
			var = rs->Fields->Item["Name"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "Custom Contact Info 3";
		}
		else
			str = "Custom Contact Info 3";
		rs->Close();

		AddItemTo17aCombo(18, str);
		AddItemTo33Combo(18, str);

		rs = CreateRecordset("SELECT Name FROM CustomFieldsT WHERE ID = 9");
		if(!rs->eof) {
			var = rs->Fields->Item["Name"]->Value;
			if(var.vt==VT_BSTR)
				str = CString(var.bstrVal);
			else
				str = "Custom Contact Info 4";
		}
		else
			str = "Custom Contact Info 4";
		rs->Close();

		AddItemTo17aCombo(19, str);
		AddItemTo33Combo(19, str);

	}NxCatchAll("Error loading custom field info.");
}

/*
void CHcfaSetup::OnExportFa8() 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{
		long ExportFA8 = m_checkExportFA8.GetCheck();

		UpdateTable("ExportFA8",ExportFA8);
	}
	NxCatchAll("Error in OnExportFa8()");	
}
*/

void CHcfaSetup::OnAdvEbillingSetup() 
{
	if(m_pGroups->CurSel==-1)
		return;

	CAdvEbillingSetup dlg(this);
	dlg.m_bIsUB92 = FALSE;
	dlg.m_GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
	dlg.DoModal();
}

void CHcfaSetup::OnSelChosenBatchCombo(long nRow) 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	
		long batch = 0;

		if(m_BatchCombo->GetCurSel()==0)
			batch = 1;
		else if(m_BatchCombo->GetCurSel()==1)
			batch = 2;
		else
			batch = 0;

		UpdateTable("DefBatch",batch);
	}
	NxCatchAll("Error in OnSelChosenBatchCombo()");
}

void CHcfaSetup::OnSelChosenSecondaryBatchCombo(long nRow) 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	
		long batch = 0;

		if(m_SecondaryBatchCombo->GetCurSel()==0)
			batch = 1;
		else if(m_SecondaryBatchCombo->GetCurSel()==1)
			batch = 2;
		else
			batch = 0;

		UpdateTable("DefBatchSecondary",batch);
	}
	NxCatchAll("Error in OnSelChosenSecondaryBatchCombo()");
}

void CHcfaSetup::OnSelChosenBox24CCombo(long nRow) 
{
	if(m_pGroups->CurSel == -1)
		return;

	if (m_loading)
		return;

	if(m_Box24CCombo->GetCurSel() == sriNoRow){
		return;
	}
		
	try{
		long nEmergency = 0;

		if(m_Box24CCombo->GetCurSel()==2){
			nEmergency = 2;
		}
		else if(m_Box24CCombo->GetCurSel()==1){
			nEmergency = 1;
		}
		else{
			nEmergency = 0;
		}

		UpdateTable("Box24C", nEmergency);
	}NxCatchAll("Error in OnSelChosenBox24CCombo");
}

void CHcfaSetup::OnBtnHcfaDates() 
{
	if(m_pGroups->CurSel==-1)
		return;

	CHCFADatesDlg dlg(this);
	dlg.m_HCFASetupGroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
	dlg.m_strGroupName = CString(m_pGroups->GetValue(m_pGroups->CurSel,1).bstrVal);
	dlg.DoModal();
}

// (j.jones 2009-03-02 15:46) - PLID 31913 - changed this function to return how many records were affected
long CHcfaSetup::UpdateTable(CString BoxName, long data)
{
	if(m_pGroups->CurSel == -1) {
		return 0;
	}

	long GroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;

	// (j.jones 2009-03-02 15:46) - PLID 31913 - for auditing purposes, we can alter this
	// update statement slightly to see if anything changed (this logic won't be used
	// in all uses of this function, but could be in the future)
	// (a.walling 2009-05-19 11:29) - PLID 34294 - This used to include 'AND Field <> Value', but that overlooked
	// NULL values, where the conditional NULL <> Value would always fail, causing the table to never update.
	long nRecordsAffected = 0;
	ExecuteSql(&nRecordsAffected, adCmdText, 
		"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
		"UPDATE HCFASetupT SET %s = %li WHERE ID = %li AND (%s IS NULL OR %s <> %li)", BoxName, data, GroupID, BoxName, BoxName, data);

	return nRecordsAffected;
}

void CHcfaSetup::OnSelChosenHcfaGroups(long nRow) 
{
	Load();
}

void CHcfaSetup::OnBtnAdvPinSetup() 
{
	if(m_pGroups->CurSel == -1)
		return;

	if(IDNO == MessageBox("You do not need to configure the Advanced ID Setup unless instructed to do so by NexTech, "
				  "or if you are required to use a different Box 33b number per Provider, per Insurance Company, per Location.\n\n"
				  "Would you still like to configure the Advanced ID Setup?","Practice",MB_ICONINFORMATION|MB_YESNO)) {
		return;
	}

	CAdvHCFAPinSetup dlg(this);
	dlg.m_HCFAGroup = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
	dlg.DoModal();

	// (j.jones 2007-02-21 13:19) - PLID 24580 - colorized the Adv PIN Setup button when in use
	// now update the button to reflect any changes we may have made
	UpdateAdvancedPINSetupAppearance();
}

void CHcfaSetup::OnCheckUseEmployer() 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{
		long UseEmp = m_checkUseEmployer.GetCheck();

		UpdateTable("UseEmp",UseEmp);
	}
	NxCatchAll("Error in OnCheckUseEmployer()");	
}

void CHcfaSetup::OnAdvPosCodeSetup() 
{
	if(m_pGroups->CurSel == -1)
		return;

	CEditDefaultPOSCodesDlg dlg(this);
	dlg.m_HCFASetupGroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
	dlg.DoModal();
}

void CHcfaSetup::OnCheckRefphyidInBox19() 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	
		long Box19RefPhyID = m_checkBox19RefPhyID.GetCheck();
		
		if(m_checkBox19RefPhyID.GetCheck()) {
			m_checkOverrideBox19.SetCheck(FALSE);
			OnCheckDefaultInBox19();
		}

		UpdateTable("Box19RefPhyID",Box19RefPhyID);
	}
	NxCatchAll("Error in OnCheckRefphyidInBox19()");
}

void CHcfaSetup::OnCheckDefaultInBox19() 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	
		long Box19Override = m_checkOverrideBox19.GetCheck();

		if(m_checkOverrideBox19.GetCheck()) {
			m_checkBox19RefPhyID.SetCheck(FALSE);
			OnCheckRefphyidInBox19();			
		}

		GetDlgItem(IDC_BOX_19)->EnableWindow(m_checkOverrideBox19.GetCheck());

		UpdateTable("Box19Override",Box19Override);
	}
	NxCatchAll("Error in OnCheckDefaultInBox19()");
}

void CHcfaSetup::OnBtnDoNotFill() 
{
	if(m_pGroups->CurSel==-1)
		return;

	CHCFASetupDoNotFill dlg(this);
	dlg.m_nHCFASetupGroupID = m_pGroups->GetValue(m_pGroups->CurSel,0).lVal;
	dlg.DoModal();
}

LRESULT CHcfaSetup::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {
			case NetUtils::InsuranceCoT:
			case NetUtils::InsuranceGroups: {
				try {
					UpdateView();
				} NxCatchAll("Error in CHcfaSetup::OnTableChanged:Generic");
				break;
			}
		}
	} NxCatchAll("Error in CHcfaSetup::OnTableChanged");

	return 0;
}

void CHcfaSetup::OnCheckUse23In17a() 
{
	GetDlgItem(IDC_CHECK_USE_23_IN_17A_AND_NOT_23)->ShowWindow(m_checkUse23In17a.GetCheck());

	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	

		long UseBox23InBox17aCheck = m_checkUse23In17a.GetCheck() ? 1 : 0;
		long UseBox23InBox17aAndNotIn23Check = m_checkUse23In17aAndNotIn23.GetCheck() ? 1 : 0;

		long UseBox23InBox17a = 0;
		if(UseBox23InBox17aCheck == 1) {
			if(UseBox23InBox17aAndNotIn23Check == 0)
				UseBox23InBox17a = 1;
			else
				UseBox23InBox17a = 2;
		}
		else
			UseBox23InBox17a = 0;

		UpdateTable("UseBox23InBox17a",UseBox23InBox17a);
	}
	NxCatchAll("Error in OnCheckUse23In17a()");
}

void CHcfaSetup::OnCheckUse23In17aAndNot23() 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	
		long UseBox23InBox17aCheck = m_checkUse23In17a.GetCheck() ? 1 : 0;
		long UseBox23InBox17aAndNotIn23Check = m_checkUse23In17aAndNotIn23.GetCheck() ? 1 : 0;

		long UseBox23InBox17a = 0;
		if(UseBox23InBox17aCheck == 1) {
			if(UseBox23InBox17aAndNotIn23Check == 0)
				UseBox23InBox17a = 1;
			else
				UseBox23InBox17a = 2;
		}
		else
			UseBox23InBox17a = 0;

		UpdateTable("UseBox23InBox17a",UseBox23InBox17a);
	}
	NxCatchAll("Error in OnCheckUse23In17aAndNot23()");	
}

void CHcfaSetup::OnCheckUseOverrideNameInBox31() 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	
		long UseOverrideInBox31 = m_checkUseOverrideInBox31.GetCheck();
		
		UpdateTable("UseOverrideInBox31",UseOverrideInBox31);
	}
	NxCatchAll("Error in OnCheckUseOverrideInBox31()");
}

void CHcfaSetup::OnCheckUse1aIn9a() 
{
	// (j.jones 2010-08-31 16:43) - PLID 40303 - m_checkUseTPLIn9a is mutually exclusive
	if(m_checkUse1aIn9a.GetCheck()) {
		m_checkUseTPLIn9a.SetCheck(FALSE);
		OnCheckSendTPLIn9a();
	}

	GetDlgItem(IDC_CHECK_USE_1A_IN_9A_ALWAYS)->ShowWindow(m_checkUse1aIn9a.GetCheck());

	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	
		long Use1aIn9a = m_checkUse1aIn9a.GetCheck();
		
		UpdateTable("Use1aIn9a",Use1aIn9a);
	}
	NxCatchAll("Error in OnCheckUse1aIn9a()");	
}

void CHcfaSetup::OnCheckHidePhoneNum() 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	
		long HidePhoneBox33 = 0;
		if(m_checkHidePhoneNum.GetCheck())
			HidePhoneBox33 = 1;

		UpdateTable("HidePhoneBox33", HidePhoneBox33);
	}
	NxCatchAll("Error in OnCheckHidePhoneBox33()");
}

void CHcfaSetup::OnCheckUse1aIn9aAlways() 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	
		long Use1aIn9aAlways = m_checkUse1aIn9aAlways.GetCheck();
		
		UpdateTable("Use1aIn9aAlways",Use1aIn9aAlways);
	}
	NxCatchAll("Error in OnCheckUse1aIn9aAlways()");	
}

void CHcfaSetup::OnCheckPutSpaceAfterQualifiers() 
{
	// (j.jones 2007-02-21 14:55) - PLID 24776 - supported option to put a space between qualifiers and IDs

	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	
		long QualifierSpace = m_checkPutSpaceAfterQualifiers.GetCheck() ? 1 : 0;
		
		UpdateTable("QualifierSpace",QualifierSpace);
	}
	NxCatchAll("Error in OnCheckPutSpaceAfterQualifiers()");
}

// (j.jones 2007-02-21 13:19) - PLID 24580 - colorize the Adv PIN Setup button when in use
void CHcfaSetup::UpdateAdvancedPINSetupAppearance()
{
	if(m_pGroups->CurSel == -1)
		return;

	try {

		long nSetupGroupID = VarLong(m_pGroups->GetValue(m_pGroups->CurSel,0), -1);

		// (j.jones 2007-05-07 14:21) - PLID 25922 - added Box33Name override
		// (j.jones 2007-08-08 10:12) - PLID 25395 - added NPI values
		// (j.jones 2008-09-10 11:24) - PLID 30788 - added Box25Check
		// (j.jones 2010-02-01 09:31) - PLID 37137 - added the Box 33 address override
		if(ReturnsRecords("SELECT SetupGroupID FROM AdvHCFAPinT "
			"WHERE (PIN <> '' OR GRP <> '' OR EIN <> '' OR Box19 <> '' OR Box33bQual <> '' OR Box24JQual <> '' OR Box24J <> '' "
				"OR Box33Name <> '' OR Box24JNPI <> '' OR Box33aNPI <> '' OR Box25Check <> 0 "
				"OR Box33_Address1 <> '' OR Box33_Address2 <> '' OR Box33_City <> '' OR Box33_State <> '' OR Box33_Zip <> '' "
			") "
			"AND SetupGroupID = %li", nSetupGroupID)) {
			SetDlgItemText(IDC_BTN_ADV_PIN_SETUP, "Advanced ID Setup (In Use)");
			m_btnAdvPINSetup.SetTextColor(RGB(255,0,0));
			m_btnAdvPINSetup.Invalidate();
		}
		else {
			SetDlgItemText(IDC_BTN_ADV_PIN_SETUP, "Advanced ID Setup");
			m_btnAdvPINSetup.SetTextColor(RGB(0,0,0));
			m_btnAdvPINSetup.Invalidate();
		}				
	}
	NxCatchAll("Error in UpdateAdvancedPINSetupAppearance()");
}

// (j.jones 2008-04-03 12:06) - PLID 28995 - added ability to configure claim providers per HCFA group
void CHcfaSetup::OnBtnConfigClaimProviders() 
{
	try {

		if(m_pGroups->CurSel == -1) {
			return;
		}

		long nSetupGroupID = VarLong(m_pGroups->GetValue(m_pGroups->CurSel,0), -1);

		// (j.jones 2008-08-01 10:37) - PLID 30918 - now this function opens up
		// the advanced setup to apply claim providers across multiple insurance companies
		CAdvHCFAClaimProviderSetupDlg dlg(this);
		dlg.m_nColor = BACKGROUND_COLOR;
		dlg.m_nDefaultHCFAGroupID = nSetupGroupID;	//this will pre-select the HCFA group's companies
		dlg.DoModal();

	}NxCatchAll("Error in CHcfaSetup::OnBtnConfigClaimProviders()");
}

// (j.jones 2008-06-10 14:47) - PLID 25834 - added OnCheckValidateRefPhy
void CHcfaSetup::OnCheckValidateRefPhy() 
{
	if(m_pGroups->CurSel==-1)
		return;

	if (m_loading)
		return;

	try
	{	
		long ValidateRefPhy = m_checkValidateRefPhy.GetCheck() ? 1 : 0;
		
		UpdateTable("ValidateRefPhy",ValidateRefPhy);
	}
	NxCatchAll("Error in OnCheckValidateRefPhy()");
}

// (j.jones 2009-03-06 11:39) - PLID 33354 - added controls to update PullCurrentAccInfo
void CHcfaSetup::OnRadioAccUseBill()
{
	try {

		if(m_pGroups->CurSel == -1) {
			return;
		}

		if(m_loading) {
			return;
		}

		//1 - pull from Bill
		//2 - pull from Insured Party
		long PullCurrentAccInfo = 1;
		if(m_radioAccUseInsuredParty.GetCheck()) {
			PullCurrentAccInfo = 2;
		}
		
		UpdateTable("PullCurrentAccInfo", PullCurrentAccInfo);

	}NxCatchAll("Error in CHcfaSetup::OnRadioAccUseBill");
}

void CHcfaSetup::OnRadioAccUseInsuredParty()
{
	try {

		if(m_pGroups->CurSel == -1) {
			return;
		}

		if(m_loading) {
			return;
		}
	
		OnRadioAccUseBill();

	}NxCatchAll("Error in CHcfaSetup::OnRadioAccUseInsuredParty");
}

// (j.jones 2010-08-31 16:33) - PLID 40303 - supported TPLIn9a
void CHcfaSetup::OnCheckSendTPLIn9a()
{
	try {

		long nTPLIn9a = 0;
		if(m_checkUseTPLIn9a.GetCheck()) {
			nTPLIn9a = 1;

			//m_checkUse1aIn9a is mutually exclusive
			m_checkUse1aIn9a.SetCheck(FALSE);
			OnCheckUse1aIn9a();
		}

		if(m_pGroups->CurSel == -1) {
			return;
		}

		if(m_loading) {
			return;
		}
	
		UpdateTable("TPLIn9a", nTPLIn9a);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-06-10 16:26) - PLID 56255 - added UseRefPhyGroupNPI
void CHcfaSetup::OnCheckUseRefPhyGroupNpi()
{
	try {

		if(m_pGroups->CurSel==-1)
			return;

		if (m_loading)
			return;

		long nUseRefPhyGroupNPI = m_checkUseRefPhyGroupNPI.GetCheck();
		
		UpdateTable("UseRefPhyGroupNPI", nUseRefPhyGroupNPI);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-07-17 15:49) - PLID 41823 - added DontBatchSecondary
void CHcfaSetup::OnCheckDontBatchSecondary()
{
	try {

		if(m_pGroups->CurSel==-1)
			return;

		if (m_loading)
			return;

		long nDontBatchSecondary = m_checkDontBatchSecondary.GetCheck();

		if(nDontBatchSecondary == 1) {
			//warn what this feature is doing (this same 'dont show' is shared with the UB tab)
			DontShowMeAgain(this, "The \"Primary automatically sends claims to Secondary\" setting is used for claims where a primary insurance company "
				"in this HCFA group automatically sends the claim on to the patient's secondary insurance company. The secondary company does not need to be in this HCFA group.\n\n"
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
		AuditEvent(-1, strGroupName, nAuditID, aeiHCFASetup_DontBatchSecondary, nGroupID, nDontBatchSecondary == 1 ? "Disabled" : "Enabled", nDontBatchSecondary == 1 ? "Enabled" : "Disabled", aepMedium, aetChanged); 

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-07-17 15:58) - PLID 41823 - added 'DontBatchSecondary' exclusions
void CHcfaSetup::OnBtnEditSecondaryExclusions()
{
	long nAuditTransactionID = -1;

	try {

		if(m_pGroups->CurSel==-1)
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

		CMultiSelectDlg dlg(this, "HCFASecondaryExclusionsT");
	
		//pre-select any already-excluded companies

		CMap<long, long, CString, LPCTSTR> mapInsuranceIDToName;
		CDWordArray dwaOldCompanies;
		std::set<long> setOldCompanies;

		_RecordsetPtr rsExisting = CreateParamRecordset("SELECT HCFASecondaryExclusionsT.InsuranceCoID, InsuranceCoT.Name "
			"FROM HCFASecondaryExclusionsT "
			"INNER JOIN InsuranceCoT ON HCFASecondaryExclusionsT.InsuranceCoID = InsuranceCoT.PersonID "
			"WHERE HCFASecondaryExclusionsT.HCFASetupID = {INT}", nGroupID);		
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
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM HCFASecondaryExclusionsT "
				"WHERE InsuranceCoID = {INT} AND HCFASetupID = {INT}", nRemovedInsuranceCoID, nGroupID);

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
			AuditEvent(-1, strGroupName, nAuditTransactionID, aeiHCFASetup_InsCoSecondaryExcluded, nGroupID, strOld, strNew, aepMedium, aetDeleted); 
		}
		foreach(long nAddedInsuranceCoID, setAddedCompanies) {
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO HCFASecondaryExclusionsT (InsuranceCoID, HCFASetupID) "
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
			AuditEvent(-1, strGroupName, nAuditTransactionID, aeiHCFASetup_InsCoSecondaryExcluded, nGroupID, strOld, strNew, aepMedium, aetCreated); 
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

// (j.jones 2013-08-05 11:46) - PLID 57805 - added link to the HCFA upgrade date setup
BOOL CHcfaSetup::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {

		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		CRect rcUpgradeLabel;
		GetDlgItem(IDC_HCFA_UPGRADE_DATE_LABEL)->GetWindowRect(rcUpgradeLabel);
		ScreenToClient(&rcUpgradeLabel);

		if(rcUpgradeLabel.PtInRect(pt) && m_lblUpgradeDate.GetType() == dtsHyperlink) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

	} NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (j.jones 2013-08-05 11:46) - PLID 57805 - added link to the HCFA upgrade date setup
LRESULT CHcfaSetup::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		
		switch ((UINT)wParam) {
			case IDC_HCFA_UPGRADE_DATE_LABEL:
				{
					if(m_pGroups->CurSel == -1) {
						return 0;
					}
					
					long nGroupID = VarLong(m_pGroups->GetValue(m_pGroups->CurSel,0));
					if(nGroupID == -1) {
						return 0;
					}

					CHCFAUpgradeDateDlg dlg;
					if(dlg.DoModal(nGroupID, true, GetNxColor(GNC_ADMIN, 0)) == IDOK) {
						LoadHCFAUpgradeDateLabel();
					}
				}
				break;
			default:
				//what control caused this?
				ASSERT(FALSE);
				break;
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.jones 2013-08-05 11:46) - PLID 57805 - loads the proper label text for m_lblUpgradeDate
void CHcfaSetup::LoadHCFAUpgradeDateLabel()
{
	try {

		//if there is no group selected, or there are no companies
		//in the group, the label will have its text cleared and
		//will not be enabled
		CString strHCFAUpgradeText = "";

		if(m_pGroups->CurSel != -1) {
		
			long nGroupID = VarLong(m_pGroups->GetValue(m_pGroups->CurSel,0));

			if(nGroupID != -1) {

				COleDateTime dtNow = COleDateTime::GetCurrentTime();

				_RecordsetPtr rsUpgradeDate = CreateParamRecordset("SELECT Min(HCFAUpgradeDate) AS FirstDate, Max(HCFAUpgradeDate) AS LastDate "
					"FROM InsuranceCoT "
					"WHERE HCFASetupGroupID = {INT} "
					"GROUP BY HCFASetupGroupID", nGroupID);
				if(!rsUpgradeDate->eof) {
					COleDateTime dtFirst = VarDateTime(rsUpgradeDate->Fields->Item["FirstDate"]->Value);
					COleDateTime dtLast = VarDateTime(rsUpgradeDate->Fields->Item["LastDate"]->Value);
					//in most cases, the dates won't differ between companies in the same group,
					//but we should give a context-sensitive label in the cases where they do
					//no companies in this group, use the default date (4/1/2014)
					if(dtFirst != dtLast) {
						//make the past/future context change follow the 'last' date
						strHCFAUpgradeText.Format("Companies in this group %s to the new HCFA form between %s and %s.",
							dtLast < dtNow ? "switched" : "will switch",
							FormatDateTimeForInterface(dtFirst, NULL, dtoDate), FormatDateTimeForInterface(dtLast, NULL, dtoDate));
					}
					else {
						strHCFAUpgradeText.Format("This group %s to the new HCFA form on %s.",
							dtFirst < dtNow ? "switched" : "will switch",
							FormatDateTimeForInterface(dtFirst, NULL, dtoDate));
					}
				}
				rsUpgradeDate->Close();
			}
		}

		m_lblUpgradeDate.SetText(strHCFAUpgradeText);

		//if the text is blank, disable the hyperlink
		if(strHCFAUpgradeText.IsEmpty()) {
			m_lblUpgradeDate.SetType(dtsDisabledHyperlink);
		}
		else {
			m_lblUpgradeDate.SetType(dtsHyperlink);
		}

	}NxCatchAll(__FUNCTION__);
}
// EditInsInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EditInsInfoDlg.h"
#include "GlobalUtils.h"
#include "PracProps.h"
#include "GetNewIDName.h"
#include "NxStandard.h"
#include "InsuranceGroupsDlg.h"
#include "EditBox24J.h"
#include "EditBox31.h"
#include "EditBox51.h"
#include "GlobalDataUtils.h"
#include "EditInsPayersDlg.h"
#include "Client.h"
#include "EditNetworkID.h"
#include "EditFacilityID.h"
#include "AuditTrail.h"
#include "ListMergeDlg.h"
#include "DontShowDlg.h"
#include "EditInsAcceptedDlg.h"
#include "CPTCodeInsNotesDlg.h"
#include "EditComboBox.h"
#include "AdvInsurancePayDescriptionSetupDlg.h"
#include "HCFAClaimProviderSetupDlg.h"
#include "HL7Utils.h"
#include "PatientsRc.h"
#include "FinancialClassDlg.h"
#include "ConfigPayerIDsPerLocationDlg.h"
#include "HL7ParseUtils.h"
#include "CLIASetup.h"
#include "EditDefaultDeductible.h" //(8/17/2012) r.wilson - PLID 50636
#include "PayCatDlg.h"
#include "HCFAUpgradeDateDlg.h"
#include "ICD10GoLiveDateDlg.h"
#include "GlobalFinancialUtils.h"
#include "ClaimFormLocationInsuranceSetup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#define DEFAULT_COMPANY_NAME "Enter Company Name"

// (j.jones 2008-07-10 16:40) - PLID 28756 - added pay/adj category combos
enum PayCategoryCombo {

	pccID = 0,
	pccName,
};

enum AdjCategoryCombo {

	accID = 0,
	accName,
};

// (j.jones 2008-09-08 17:55) - PLID 18695 - added Ins. Type enum
enum InsTypeCombo {

	itcID = 0,
	itcName,
	itcANSI,
	itcNSF,
};

// (j.jones 2012-08-07 09:17) - PLID 51914 - added enum for the HCFA payer ID field
enum HCFAPayerIDCombo {

	hpicID = 0,
	hpicCode = 1,
	hpicName = 2,
};

// (j.jones 2008-09-09 13:58) - PLID 31138 - added Eligibility Payer ID list
enum EligibilityPayerIDCombo {

	epicID = 0, //oh, it's epic alright
	epicCode = 1,
	epicName = 2,
};

// (j.jones 2009-12-16 15:00) - PLID 36237 - added UB payer ID
enum UBPayerIDCombo {

	ubpicID = 0,
	ubpicCode = 1,
	ubpicName = 2,
};

// (j.jones 2012-10-19 09:56) - PLID 51551 - added enum for the insurance contact column
enum InsContactCombo {

	iccID = 0,
	iccLast,
	iccFirst,
	iccPhone,
	iccFullName,
};

// CH 3/22: Used in automatically making a new plan for
// a company. I don't like using static globals, or
// even member variables in this nature.
static BOOL g_boNewCompany = FALSE;
// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
//using namespace NXDATALISTLib; // (c.haag 2010-05-06 12:29) - PLID 37525 - We now use both lists

/////////////////////////////////////////////////////////////////////////////
// CEditInsInfoDlg dialog

CEditInsInfoDlg::CEditInsInfoDlg(CWnd* pParent)
	: CNxDialog(CEditInsInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditInsInfoDlg)
	m_bAutoSetContact = TRUE;
	m_nCompanyID = -1;
	m_nContactID = -1;
	//}}AFX_DATA_INIT
	m_bSelChangeInProgress = FALSE; // (c.haag 2010-05-06 12:29) - PLID 37525
}


void CEditInsInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditInsInfoDlg)
	DDX_Control(pDX, IDC_BTN_CLAIM_PROVIDER_SETUP, m_btnClaimProviderSetup);
	DDX_Control(pDX, IDC_BTN_ADV_INS_DESC_SETUP, m_btnAdvInsDescSetup);
	DDX_Control(pDX, IDC_EDIT_DEF_ADJ_DESCRIPTION, m_nxeditDefAdjDescription);
	DDX_Control(pDX, IDC_EDIT_DEF_PAY_DESCRIPTION, m_nxeditDefPayDescription);
	DDX_Control(pDX, IDC_BTN_EDIT_ADJ_CATS, m_btnEditAdjCats);
	DDX_Control(pDX, IDC_BTN_EDIT_PAY_CATS, m_btnEditPayCats);	
	DDX_Control(pDX, IDC_BTN_BOX31, m_btnBox31);
	DDX_Control(pDX, IDC_BTN_EDIT_CPT_INS_NOTES, m_btnEditCPTNotes);
	DDX_Control(pDX, IDC_INACTIVE_CHECK, m_btnInactive);
	DDX_Control(pDX, IDC_WORKERS_COMP_CHECK, m_checkWorkersComp);
	DDX_Control(pDX, IDC_CHECK_USE_REFERRALS, m_ReferralCheck);
	DDX_Control(pDX, IDC_RADIO_TAX_PATIENT, m_radioTaxPatient);
	DDX_Control(pDX, IDC_RADIO_TAX_NONE, m_radioTaxNone);
	DDX_Control(pDX, IDC_RADIO_TAX_INS, m_radioTaxIns);
	DDX_Control(pDX, IDC_BTN_ACCEPTED, m_btnAcceptAssignment);
	DDX_Control(pDX, IDC_MANAGE_CONTACTS, m_btnManageContacts);
	DDX_Control(pDX, IDC_CHECK_MAKE_DEFAULT_CONTACT, m_checkMakeDefaultContact);
	DDX_Control(pDX, IDC_DELETE_CONTACT, m_btnDeleteContact);
	DDX_Control(pDX, IDC_ADD_CONTACT, m_btnAddContact);
	DDX_Control(pDX, IDC_BTN_FACILITY_ID, m_btnFacilityID);
	DDX_Control(pDX, IDC_BTN_BOX51, m_btnBox51);
	DDX_Control(pDX, IDC_HMO_CHECK, m_HMOCheck);
	DDX_Control(pDX, IDC_EDIT_NETWORKID, m_btnEditNetworkID);
	DDX_Control(pDX, IDC_BTN_OK, m_btnClose);
	DDX_Control(pDX, IDC_DELETE_BTTN, m_btnDelete);
	DDX_Control(pDX, IDC_ADD_BTTN, m_btnAdd);
	DDX_Control(pDX, IDC_DELETE_PLAN, m_btnDelPlan);
	DDX_Control(pDX, IDC_BTN_GROUPS, m_btnGroups);
	DDX_Control(pDX, IDC_BTN_BOX24J, m_btnBox24J);
	DDX_Control(pDX, IDC_BTN_EDIT_DEFAULT_DEDUCTIBLE, m_btnEditDefaultDeductible);
	DDX_Control(pDX, IDC_ADD_PLAN, m_btnAddPlan);
	DDX_Control(pDX, IDC_EDIT_INS_BKG1, m_bkg);
	DDX_Control(pDX, IDC_EDIT_INS_BKG2, m_bkg2);
	DDX_Control(pDX, IDC_EDIT_INS_BKG3, m_bkg3);
	DDX_Control(pDX, IDC_EDIT_INS_BKG4, m_bkg4);
	DDX_Control(pDX, IDC_EDIT_INS_BKG5, m_bkg5);
	DDX_Control(pDX, IDC_EDIT_INS_BKG6, m_bkg6);
	DDX_Control(pDX, IDC_EDIT_INS_BKG7, m_bkg7);
	DDX_Control(pDX, IDC_EDIT_INS_BKG8, m_bkg8);
	DDX_Control(pDX, IDC_INSURANCE_PLAN_BOX, m_nxeditInsurancePlanBox);
	DDX_Control(pDX, IDC_CONTACT_ADDRESS_BOX, m_nxeditContactAddressBox);
	DDX_Control(pDX, IDC_CONTACT_ADDRESS2_BOX, m_nxeditContactAddress2Box);
	DDX_Control(pDX, IDC_CONTACT_ZIP_BOX, m_nxeditContactZipBox);
	DDX_Control(pDX, IDC_CONTACT_CITY_BOX, m_nxeditContactCityBox);
	DDX_Control(pDX, IDC_CONTACT_STATE_BOX, m_nxeditContactStateBox);
	DDX_Control(pDX, IDC_RVU_BOX, m_nxeditRvuBox);
	DDX_Control(pDX, IDC_CO_MEMO, m_nxeditCoMemo);
	DDX_Control(pDX, IDC_EDIT_CROSSOVER_CODE, m_nxeditEditCrossoverCode);
	DDX_Control(pDX, IDC_CONTACT_FIRST_BOX, m_nxeditContactFirstBox);
	DDX_Control(pDX, IDC_CONTACT_LAST_BOX, m_nxeditContactLastBox);
	DDX_Control(pDX, IDC_CONTACT_TITLE_BOX, m_nxeditContactTitleBox);
	DDX_Control(pDX, IDC_CONTACT_PHONE_BOX, m_nxeditContactPhoneBox);
	DDX_Control(pDX, IDC_CONTACT_EXT_BOX, m_nxeditContactExtBox);
	DDX_Control(pDX, IDC_CONTACT_FAX_BOX, m_nxeditContactFaxBox);
	DDX_Control(pDX, IDC_BTN_CONFIGURE_LOCATION_PAYER_IDS, m_btnConfigLocationPayerIDs);
	DDX_Control(pDX, IDC_EDIT_TPL_CODE, m_nxeditTPLCode);
	DDX_Control(pDX, IDC_CLIA_SETUP, m_btnCLIASetup);
	DDX_Control(pDX, IDC_BTN_EDIT_INSCO_PREV, m_btnInsCoPrev);
	DDX_Control(pDX, IDC_BTN_EDIT_INSCO_NEXT, m_btnInsCoNext);
	DDX_Control(pDX, IDC_HCFA_UPGRADE_DATE_LABEL, m_lblHCFAUpgradeDate);
	DDX_Control(pDX, IDC_ICD10_GO_LIVE_DATE, m_lblICD10GoLiveDate); 
	DDX_Control(pDX, IDC_BTN_CLAIM_FORM_SETUP, m_btnDefaultClaimFormSetup);
	//}}AFX_DATA_MAP
}

// (j.jones 2008-09-09 16:05) - PLID 31138 - made the regular payer ID edit button and eligibility edit button
// reference the same function
BEGIN_MESSAGE_MAP(CEditInsInfoDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEditInsInfoDlg)
	ON_WM_SHOWWINDOW()
	ON_EN_CHANGE(IDC_CONTACT_FAX_BOX, OnChangeContactFaxBox)
	ON_EN_CHANGE(IDC_CONTACT_PHONE_BOX, OnChangeContactPhoneBox)
	ON_BN_CLICKED(IDC_EDIT_PAYER_IDS, OnEditPayerIDs)
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
	//ON_BN_CLICKED(IDC_EDIT_THIN_LIST, OnEditThinList)
	ON_BN_CLICKED(IDC_ADD_BTTN, OnAddCompany)
	ON_BN_CLICKED(IDC_DELETE_BTTN, OnDeleteCompany)
	ON_BN_CLICKED(IDC_BTN_OK, OnBtnOk)
	ON_BN_CLICKED(IDC_ADD_PLAN, OnAddPlan)
	ON_BN_CLICKED(IDC_DELETE_PLAN, OnDeletePlan)
	ON_BN_CLICKED(IDC_BTN_GROUPS, OnBtnGroups)
	ON_BN_CLICKED(IDC_BTN_BOX24J, OnBtnBox24J)
	ON_BN_CLICKED(IDC_EDIT_NETWORKID, OnEditNetworkid)
	ON_BN_CLICKED(IDC_HMO_CHECK, OnHmoCheck)
	ON_BN_CLICKED(IDC_BTN_BOX51, OnBtnBox51)
	ON_BN_CLICKED(IDC_BTN_FACILITY_ID, OnBtnFacilityId)
	ON_BN_CLICKED(IDC_ADD_CONTACT, OnAddContact)
	ON_BN_CLICKED(IDC_DELETE_CONTACT, OnDeleteContact)
	ON_BN_CLICKED(IDC_CHECK_MAKE_DEFAULT_CONTACT, OnCheckMakeDefaultContact)
	ON_BN_CLICKED(IDC_MANAGE_CONTACTS, OnManageContacts)
	ON_BN_CLICKED(IDC_BTN_ACCEPTED, OnBtnAccepted)
	ON_BN_CLICKED(IDC_RADIO_TAX_INS, OnRadioTaxIns)
	ON_BN_CLICKED(IDC_RADIO_TAX_PATIENT, OnRadioTaxPatient)
	ON_BN_CLICKED(IDC_RADIO_TAX_NONE, OnRadioTaxNone)
	ON_BN_CLICKED(IDC_CHECK_USE_REFERRALS, OnCheckUseReferrals)
	ON_BN_CLICKED(IDC_WORKERS_COMP_CHECK, OnWorkersCompCheck)
	ON_BN_CLICKED(IDC_INACTIVE_CHECK, OnInactiveCheck)
	ON_BN_CLICKED(IDC_BTN_EDIT_CPT_INS_NOTES, OnBtnEditCptNotes)
	ON_BN_CLICKED(IDC_BTN_BOX31, OnBtnBox31)	
	ON_BN_CLICKED(IDC_BTN_EDIT_PAY_CATS, OnBtnEditPayCats)
	ON_BN_CLICKED(IDC_BTN_EDIT_ADJ_CATS, OnBtnEditAdjCats)
	ON_BN_CLICKED(IDC_BTN_ADV_INS_DESC_SETUP, OnBtnAdvInsDescSetup)
	ON_BN_CLICKED(IDC_BTN_CLAIM_PROVIDER_SETUP, OnBtnClaimProviderSetup)	
	ON_BN_CLICKED(IDC_BTN_EDIT_FINANCIAL_CLASS, OnBnClickedBtnEditFinancialClass)
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_LOCATION_PAYER_IDS, OnBtnConfigureLocationPayerIds)
	ON_BN_CLICKED(IDC_CLIA_SETUP, OnCliaSetup)
	ON_BN_CLICKED(IDC_BTN_EDIT_INSCO_PREV, OnBtnPrevInsuranceCo)
	ON_BN_CLICKED(IDC_BTN_EDIT_INSCO_NEXT, OnBtnNextInsuranceCo)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_EDIT_DEFAULT_DEDUCTIBLE, &CEditInsInfoDlg::OnBnClickedBtnEditDefaultDeductible)
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(IDC_BTN_CLAIM_FORM_SETUP, &CEditInsInfoDlg::OnBnClickedBtnClaimFormSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditInsInfoDlg message handlers

BEGIN_EVENTSINK_MAP(CEditInsInfoDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditInsInfoDlg)
	ON_EVENT(CEditInsInfoDlg, IDC_HCFA_SETUP, 16 /* SelChosen */, OnSelChosenHcfaSetup, VTS_I4)
	ON_EVENT(CEditInsInfoDlg, IDC_INS_PLANS, 9 /* EditingFinishing */, OnEditingFinishingInsPlans, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CEditInsInfoDlg, IDC_INS_PLANS, 10 /* EditingFinished */, OnEditingFinishedInsPlans, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEditInsInfoDlg, IDC_ELIGIBILITY_PAYER, 16 /* SelChosen */, OnSelChosenEligibility, VTS_I4)
	ON_EVENT(CEditInsInfoDlg, IDC_ENVOY, 16 /* SelChosen */, OnSelChosenEnvoy, VTS_I4)
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
	//ON_EVENT(CEditInsInfoDlg, IDC_THIN, 16 /* SelChosen */, OnSelChosenThin, VTS_I4)
	ON_EVENT(CEditInsInfoDlg, IDC_UB92_SETUP, 16 /* SelChosen */, OnSelChosenUb92Setup, VTS_I4)
	ON_EVENT(CEditInsInfoDlg, IDC_INS_CONTACTS_LIST, 16 /* SelChosen */, OnSelChosenInsContactsList, VTS_I4)
	ON_EVENT(CEditInsInfoDlg, IDC_INS_CONTACTS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedInsContactsList, VTS_I2)
	ON_EVENT(CEditInsInfoDlg, IDC_INS_TYPE_COMBO, 16 /* SelChosen */, OnSelChosenInsTypeCombo, VTS_I4)
	ON_EVENT(CEditInsInfoDlg, IDC_INS_PLANS, 19 /* LeftClick */, OnLeftClickInsPlans, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEditInsInfoDlg, IDC_INS_PLANS, 2 /* SelChanged */, OnSelChangedInsPlans, VTS_I4)
	ON_EVENT(CEditInsInfoDlg, IDC_COMBO_DEF_PAY_CATEGORY, 16 /* SelChosen */, OnSelChosenComboDefPayCategory, VTS_I4)
	ON_EVENT(CEditInsInfoDlg, IDC_COMBO_DEF_ADJ_CATEGORY, 16 /* SelChosen */, OnSelChosenComboDefAdjCategory, VTS_I4)
	ON_EVENT(CEditInsInfoDlg, IDC_COMBO_FINANCIAL_CLASS, 16, SelChosenComboFinancialClass, VTS_I4)
	ON_EVENT(CEditInsInfoDlg, IDC_UB_PAYER_IDS, 16, OnSelChosenUbPayerIds, VTS_I4)
	//}}AFX_EVENTSINK_MAP		
	ON_EVENT(CEditInsInfoDlg, IDC_INS_LIST, 1, CEditInsInfoDlg::SelChangingInsList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEditInsInfoDlg, IDC_INS_LIST, 2, CEditInsInfoDlg::SelChangedInsList, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

void CEditInsInfoDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CEditInsInfoDlg::OnOK() 
{
	// This is mislabeled 'OnOK' because all it really does is handle the enter character
	// It moves the current focus to the next item in the tab order
	//CWnd* pNextTabItem = GetNextDlgTabItem(GetFocus());
	//pNextTabItem->SetFocus();
}

void CEditInsInfoDlg::DisplayWindow(OLE_COLOR BackColor, long nCompanyID /*= -1*/, long nContactID /*= -1*/)
{
	CString tmpStr;

	m_nColor = BackColor;
	if(nCompanyID != -1)
		m_nCompanyID = nCompanyID;
	if(nContactID != -1)
		m_nContactID = nContactID;
	CNxDialog::DoModal();
}

void CEditInsInfoDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	CString str;
	_RecordsetPtr rs;
	FieldsPtr Fields;

	try {

		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow)
			return;

		//(e.lally 2011-07-01) PLID 41207 - Enable previous/next buttons as appropriate
		m_btnInsCoPrev.EnableWindow(pCurInsRow->GetPreviousRow() != NULL ? TRUE : FALSE);
		m_btnInsCoNext.EnableWindow(pCurInsRow->GetNextRow() != NULL ? TRUE : FALSE);

		long nInsCoID = VarLong(pCurInsRow->GetValue(0));
		if(nInsCoID == -1) {
			return;
		}

		rs = CreateParamRecordset("SELECT PersonT.*, InsuranceCoT.* FROM InsuranceCoT INNER JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID WHERE InsuranceCoT.PersonID = {INT}", nInsCoID);

		if(rs->eof)
			return;

		Fields = rs->Fields;

		SetDlgItemText(IDC_INSURANCE_PLAN_BOX, VarString(Fields->GetItem("Name")->Value, ""));
		SetDlgItemText(IDC_CONTACT_ADDRESS_BOX, VarString(Fields->GetItem("Address1")->Value, ""));
		SetDlgItemText(IDC_CONTACT_ADDRESS2_BOX, VarString(Fields->GetItem("Address2")->Value, ""));
		SetDlgItemText(IDC_CONTACT_CITY_BOX, VarString(Fields->GetItem("City")->Value, ""));
		SetDlgItemText(IDC_CONTACT_STATE_BOX, VarString(Fields->GetItem("State")->Value, ""));
		SetDlgItemText(IDC_CONTACT_ZIP_BOX, VarString(Fields->GetItem("Zip")->Value, ""));
		SetDlgItemInt(IDC_RVU_BOX, VarLong(Fields->GetItem("RVUMultiplier")->Value));
		SetDlgItemText(IDC_CO_MEMO, VarString(Fields->GetItem("Note")->Value, ""));

		// (j.jones 2008-07-10 16:52) - PLID 28756 - added default payment/adjustment description/category information
		m_pDefPayCategoryCombo->SetSelByColumn(pccID, Fields->GetItem("DefaultPayCategoryID")->Value);
		SetDlgItemText(IDC_EDIT_DEF_PAY_DESCRIPTION, VarString(Fields->GetItem("DefaultPayDesc")->Value, ""));
		m_pDefAdjCategoryCombo->SetSelByColumn(accID, Fields->GetItem("DefaultAdjCategoryID")->Value);
		SetDlgItemText(IDC_EDIT_DEF_ADJ_DESCRIPTION, VarString(Fields->GetItem("DefaultAdjDesc")->Value, ""));

		// (j.jones 2012-08-07 09:11) - PLID 51914 - we now pull the ID, not the code
		m_pEnvoyList->SetSelByColumn(hpicID,Fields->GetItem("HCFAPayerID")->Value);
		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
		//m_pTHINList->SetSelByColumn(1,Fields->GetItem("THIN")->Value);
		// (j.jones 2009-12-16 15:00) - PLID 36237 - added UB payerID
		m_pUBPayerList->SetSelByColumn(ubpicID, Fields->GetItem("UBPayerID")->Value);
		// (j.jones 2008-09-09 13:57) - PLID 31138 - converted to use EligibilityPayerID
		// (j.jones 2012-08-07 09:11) - PLID 51914 - we now pull the ID, not the code
		m_pEligibilityPayerIDList->SetSelByColumn(epicID, Fields->GetItem("EligPayerID")->Value);	
		m_HMOCheck.SetCheck(VarBool(Fields->GetItem("HMO")->Value));
		m_ReferralCheck.SetCheck(VarBool(Fields->GetItem("UseReferrals")->Value));
		m_checkWorkersComp.SetCheck(VarBool(Fields->GetItem("WorkersComp")->Value));

		// (j.jones 2007-09-18 17:23) - PLID 27428 - added Crossover Code
		SetDlgItemText(IDC_EDIT_CROSSOVER_CODE, VarString(Fields->GetItem("CrossoverCode")->Value, ""));

		// (j.jones 2010-08-30 16:02) - PLID 40302 - added TPL Code
		m_nxeditTPLCode.SetWindowText(VarString(Fields->GetItem("TPLCode")->Value, ""));

		if(VarBool(Fields->Item["Archived"]->Value))
			m_btnInactive.SetCheck(TRUE);
		else
			m_btnInactive.SetCheck(FALSE);

		long TaxType = VarLong(Fields->GetItem("TaxType")->Value,2);

		switch(TaxType) {
		case 1:
			m_radioTaxIns.SetCheck(TRUE);
			m_radioTaxPatient.SetCheck(FALSE);
			m_radioTaxNone.SetCheck(FALSE);
			break;
		case 2:
			m_radioTaxIns.SetCheck(FALSE);
			m_radioTaxPatient.SetCheck(TRUE);
			m_radioTaxNone.SetCheck(FALSE);
			break;
		case 3:
			m_radioTaxIns.SetCheck(FALSE);
			m_radioTaxPatient.SetCheck(FALSE);
			m_radioTaxNone.SetCheck(TRUE);
			break;
		}
			
		// (j.jones 2008-09-08 17:53) - PLID 18695 - renamed the NSF list to InsuranceTypeList
		m_pInsuranceTypeList->CurSel = -1;
		m_pInsuranceTypeList->SetSelByColumn(itcID, Fields->GetItem("InsType")->Value);
		m_pHCFAGroups->SetSelByColumn(0,Fields->GetItem("HCFASetupGroupID")->Value);
		m_pUB92Groups->SetSelByColumn(0,Fields->GetItem("UB92SetupGroupID")->Value);
		// (c.haag 2009-03-16 10:04) - PLID 9148 - Financial class
		m_pFinancialClassCombo->SetSelByColumn(0,Fields->GetItem("FinancialClassID")->Value);

		rs->Close();

		// (j.jones 2008-08-01 10:33) - PLID 30917 - colorize the Claim Provider Setup button when in use
		UpdateClaimProviderBtnAppearance();

		// (j.jones 2009-08-04 17:33) - PLID 34467 - colorize the payer ID by location button when in use
		UpdateConfigLocationPayerIDBtnAppearance();

		// (j.jones 2013-08-05 14:35) - PLID 57805 - loads the proper label text for m_lblHCFAUpgradeDate
		LoadHCFAUpgradeDateLabel();

		// (b.spivey - March 6th, 2014) - PLID 61196 - load the proper date on the label. 
		LoadICD10GoLiveDateLabel(); 

		//now load the contact info

		if(m_pContactList->CurSel == -1)
			return;

		rs = CreateRecordset("SELECT * FROM InsuranceContactsT INNER JOIN PersonT ON InsuranceContactsT.PersonID = PersonT.ID WHERE ID = %li",VarLong(m_pContactList->GetValue(m_pContactList->CurSel, iccID)));

		if(rs->eof)
			return;

		Fields = rs->Fields;

		SetDlgItemText(IDC_CONTACT_FIRST_BOX, VarString(Fields->GetItem("First")->Value, ""));
		SetDlgItemText(IDC_CONTACT_LAST_BOX, VarString(Fields->GetItem("Last")->Value, ""));
		SetDlgItemText(IDC_CONTACT_TITLE_BOX, VarString(Fields->GetItem("Title")->Value, ""));

		if(m_bFormatPhoneNums) {
			FormatItemText(GetDlgItem(IDC_CONTACT_PHONE_BOX), VarString(Fields->GetItem("WorkPhone")->Value, ""), m_strPhoneFormat);
		}
		else {
			SetDlgItemText(IDC_CONTACT_PHONE_BOX,VarString(Fields->GetItem("WorkPhone")->Value, ""));
		}

		SetDlgItemText(IDC_CONTACT_EXT_BOX, VarString(Fields->GetItem("Extension")->Value, ""));

		if(m_bFormatPhoneNums) {
			FormatItemText(GetDlgItem(IDC_CONTACT_FAX_BOX), VarString(Fields->GetItem("Fax")->Value, ""), m_strPhoneFormat);
		}
		else {
			SetDlgItemText(IDC_CONTACT_FAX_BOX,VarString(Fields->GetItem("Fax")->Value, ""));
		}

		rs->Close();

	}NxCatchAll("Error loading EditInsInfoDlg");
}

BOOL CEditInsInfoDlg::OnInitDialog()
{
	CWaitCursor pWait;

	CNxDialog::OnInitDialog();

	SetColor(m_nColor);
	
	try{

		// (d.singleton 2012-06-18 14:13) - PLID 51029 add limit to address 1 and 2 fields
		m_nxeditContactAddressBox.SetLimitText(150);
		m_nxeditContactAddress2Box.SetLimitText(150);

		//(e.lally 2011-07-01) PLID 41207
		m_btnInsCoPrev.AutoSet(NXB_LEFT);
		m_btnInsCoNext.AutoSet(NXB_RIGHT);

		m_pInsList = BindNxDataList2Ctrl(IDC_INS_LIST); // (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		m_pHCFAGroups = BindNxDataListCtrl(IDC_HCFA_SETUP);
		m_pUB92Groups = BindNxDataListCtrl(IDC_UB92_SETUP);
		m_pFinancialClassCombo = BindNxDataListCtrl(IDC_COMBO_FINANCIAL_CLASS); // (c.haag 2009-03-16 10:04) - PLID 9148
		m_pEnvoyList = BindNxDataListCtrl(IDC_ENVOY);
		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
		//m_pTHINList = BindNxDataListCtrl(IDC_THIN);
		m_pEligibilityPayerIDList = BindNxDataListCtrl(IDC_ELIGIBILITY_PAYER);
		// (j.jones 2009-12-16 15:00) - PLID 36237 - added UB payerID
		m_pUBPayerList = BindNxDataListCtrl(IDC_UB_PAYER_IDS);
		m_pInsuranceTypeList = BindNxDataListCtrl(IDC_INS_TYPE_COMBO,false);

		// (j.jones 2008-07-10 16:38) - PLID 28756 - added pay/adj category datalists
		m_pDefPayCategoryCombo = BindNxDataListCtrl(IDC_COMBO_DEF_PAY_CATEGORY);
		m_pDefAdjCategoryCombo = BindNxDataListCtrl(IDC_COMBO_DEF_ADJ_CATEGORY);

		{
			NXDATALISTLib::IRowSettingsPtr pRow;
			pRow = m_pDefPayCategoryCombo->GetRow(-1);
			pRow->PutValue(pccID, long(-1));
			pRow->PutValue(pccName, _bstr_t("<No Default Category>"));
			m_pDefPayCategoryCombo->AddRow(pRow);
		}

		{
			NXDATALISTLib::IRowSettingsPtr pRow;
			pRow = m_pDefAdjCategoryCombo->GetRow(-1);
			pRow->PutValue(accID, long(-1));
			pRow->PutValue(accName, _bstr_t("<No Default Category>"));
			m_pDefAdjCategoryCombo->AddRow(pRow);
		}

		// (j.jones 2007-09-20 10:31) - PLID 27428 - added Crossover Code
		((CNxEdit*)GetDlgItem(IDC_EDIT_CROSSOVER_CODE))->SetLimitText(50);

		// (j.jones 2010-08-30 16:02) - PLID 40302 - added TPL Code
		m_nxeditTPLCode.SetLimitText(50);

		// (j.jones 2008-07-10 16:38) - PLID 28756 - added pay/adj descriptions
		m_nxeditDefPayDescription.SetLimitText(255);
		m_nxeditDefAdjDescription.SetLimitText(255);

		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pNewSel;

		if(m_nCompanyID > 0) {			
			pNewSel = m_pInsList->SetSelByColumn(0, m_nCompanyID);
		} else {
			if (NULL != m_pInsList->GetFirstRow()) {
				m_pInsList->PutCurSel(m_pInsList->GetFirstRow());
				pNewSel = m_pInsList->GetCurSel();
			} else {
				// If we get here, the datalist hasn't actually fetched a row yet. Lets do it ourselves.
				// Fetch the first insurance ID (and yes the list includes inactive companies)
				_RecordsetPtr prs = CreateRecordset(
					"SELECT TOP 1 PersonID FROM InsuranceCoT "
					"INNER JOIN PersonT On InsuranceCoT.PersonID = PersonT.ID");
				if (!prs->eof) {
					long nID = AdoFldLong(prs, "PersonID");
					pNewSel = m_pInsList->SetSelByColumn(0, nID);
				} else {
					pNewSel = NULL; // There seem to be no records in data.
				}
			}
		}

		//m.hancock - 2/3/2006 - PLID 19085 - User should be able to remove a selection for the NSF code.
		//Existing data stores the string for each NSF code, so it is necessary to add an additional column for proper display.
		// (j.jones 2008-09-09 08:59) - PLID 18695 - I re-worked this dropdown completely to show the type name,
		// ANSI code, and NSF code.
		///***If you ever add a new row to this list, make sure it is reflected in the GlobalFinancialUtils
		//enum for InsuranceTypeCode, and the functions GetANSISBR09CodeFromInsuranceType(),
		//GetNameFromInsuranceType(), and the AdvPayerIDDlg list
		NXDATALISTLib::IRowSettingsPtr pRow = m_pInsuranceTypeList->GetRow(-1);
		pRow->PutValue(itcID, (long)itcInvalid);
		pRow->PutValue(itcName, " <No Type Selected>");
		pRow->PutValue(itcANSI, "");
		pRow->PutValue(itcNSF, "");
		m_pInsuranceTypeList->AddRow(pRow);

		AddNewRowToInsuranceTypeList(itcSelfPay);
		AddNewRowToInsuranceTypeList(itcCentralCertification);
		AddNewRowToInsuranceTypeList(itcOtherNonFederalPrograms);
		AddNewRowToInsuranceTypeList(itcPPO);
		AddNewRowToInsuranceTypeList(itcPOS);
		AddNewRowToInsuranceTypeList(itcEPO);
		AddNewRowToInsuranceTypeList(itcIndemnityInsurance);
		AddNewRowToInsuranceTypeList(itcHMO_MedicareRisk);
		AddNewRowToInsuranceTypeList(itcDentalMaintenanceOrganization);	// (j.jones 2010-10-15 14:36) - PLID 40953 - new for 5010
		AddNewRowToInsuranceTypeList(itcAutomobileMedical);
		AddNewRowToInsuranceTypeList(itcBCBS, TRUE);
		AddNewRowToInsuranceTypeList(itcChampus);
		AddNewRowToInsuranceTypeList(itcCommercial, TRUE);
		AddNewRowToInsuranceTypeList(itcCommercialSupplemental);	// (j.jones 2009-03-27 17:18) - PLID 33724
		AddNewRowToInsuranceTypeList(itcDisability);
		AddNewRowToInsuranceTypeList(itcFederalEmployeesProgram);	// (j.jones 2010-10-15 14:36) - PLID 40953 - new for 5010
		AddNewRowToInsuranceTypeList(itcHMO);
		AddNewRowToInsuranceTypeList(itcLiability);
		AddNewRowToInsuranceTypeList(itcLiabilityMedical);
		AddNewRowToInsuranceTypeList(itcMedicarePartA);				// (j.jones 2010-10-15 14:36) - PLID 40953 - new for 5010
		AddNewRowToInsuranceTypeList(itcMedicarePartB, TRUE);
		AddNewRowToInsuranceTypeList(itcMedicaid, TRUE);
		AddNewRowToInsuranceTypeList(itcOtherFederalProgram);
		AddNewRowToInsuranceTypeList(itcTitleV);
		AddNewRowToInsuranceTypeList(itcVeteranAdministrationPlan);
		AddNewRowToInsuranceTypeList(itcWorkersComp, TRUE);
		// (j.jones 2013-11-13 12:52) - PLID 58931 - added HRSA Other, for MU purposes
		AddNewRowToInsuranceTypeList(itcHRSAOther);
		AddNewRowToInsuranceTypeList(itcOther);

		// If nNewSel is NULL, that means our logic above failed somehow because its entire 
		// purpose is to set the selection to an actual row.  Notice both the IF and the 
		// ELSE set the selection to something expected to exist in the list.
		// (c.haag 2010-05-06 12:29) - PLID 37525 - We don't need m_nCurInsCoID. If anything, a disparity
		// between that value and the list's current selection can spell trouble. (Also, this assertion was here
		// in the past; I just changed it to DL2 style checking)
		ASSERT(NULL != pNewSel);
		/*if (NULL != pNewSel) {
			m_nCurInsCoID = VarLong(pNewSel->GetValue(0));
		} else {
			m_nCurInsCoID = -1;
		}*/

		m_bAutoSetContact = TRUE;

		//TES 3/12/2007 - PLID 25295 - On the UB04, we'll call this the Other Prv ID, because it can be in multiple boxes.
		if(GetUBFormType() == eUB04) {//UB04
			SetDlgItemText(IDC_BTN_BOX51, "UB Other Prv ID");
		}
		else {//UB92
			SetDlgItemText(IDC_BTN_BOX51, "UB92 Box 51");
		}

		// (j.gruber 2009-10-06 17:02) - PLID 35825 - reset here in case they changed the preference
		m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");

		if (m_bLookupByCity) {
			ChangeZOrder(IDC_CONTACT_ZIP_BOX, IDC_CONTACT_STATE_BOX);
		} else {
			ChangeZOrder(IDC_CONTACT_ZIP_BOX, IDC_CONTACT_ADDRESS2_BOX);
		}

		m_pInsPlans = BindNxDataListCtrl(IDC_INS_PLANS, false);
		CString strWhere;

		if(NULL != pNewSel) {
			strWhere.Format("InsCoID = %li", VarLong(pNewSel->GetValue(0), -1));
			m_pInsPlans->WhereClause = (LPCTSTR)strWhere;
			m_pInsPlans->Requery();
		}

		m_pContactList = BindNxDataListCtrl(IDC_INS_CONTACTS_LIST,false);

		if(NULL != pNewSel) {
			strWhere.Format("InsuranceContactsT.InsuranceCoID = %li",pNewSel->GetValue(0).lVal);
			m_pContactList->WhereClause = (LPCTSTR)strWhere;
			m_pContactList->Requery();
		}

		_variant_t var;
		var.vt = VT_NULL;
		pRow = m_pEnvoyList->GetRow(-1);
		pRow->PutValue(hpicID,(long)-1);
		pRow->PutValue(hpicCode,(_bstr_t)"");
		pRow->PutValue(hpicName,(_bstr_t)"<No Payer ID Selected>");
		m_pEnvoyList->AddRow(pRow);
		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
		/*
		pRow = m_pTHINList->GetRow(-1);
		pRow->PutValue(0,(long)-1);
		pRow->PutValue(1,(_bstr_t)"");
		pRow->PutValue(2,(_bstr_t)"<No ID Selected>");
		m_pTHINList->AddRow(pRow);
		*/
		pRow = m_pEligibilityPayerIDList->GetRow(-1);
		pRow->PutValue(epicID,(long)-1);
		pRow->PutValue(epicCode,(_bstr_t)"");
		pRow->PutValue(epicName,(_bstr_t)"<No Eligibility ID Selected>");
		m_pEligibilityPayerIDList->AddRow(pRow);

		// (j.jones 2009-12-16 15:00) - PLID 36237 - added UB payerID
		pRow = m_pUBPayerList->GetRow(-1);
		pRow->PutValue(ubpicID,(long)-1);
		pRow->PutValue(ubpicCode,(_bstr_t)"");
		pRow->PutValue(ubpicName,(_bstr_t)"<No UB ID Selected>");
		m_pUBPayerList->AddRow(pRow);

		m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true); 
		m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

		m_bIsNewCo = false;
		UpdateView();

		RefreshPlanButtons();

	}NxCatchAll("Error in OnInitDialog()");

	return TRUE;
}

void CEditInsInfoDlg::SetColor(OLE_COLOR nNewColor)
{
	m_bkg.SetColor(nNewColor);
	m_bkg2.SetColor(nNewColor);
	m_bkg3.SetColor(nNewColor);
	m_bkg4.SetColor(nNewColor);
	m_bkg5.SetColor(nNewColor);
	m_bkg6.SetColor(nNewColor);
	m_bkg7.SetColor(nNewColor);
	m_bkg8.SetColor(nNewColor); // (c.haag 2009-03-16 10:27) - PLID 9148

	m_btnAdd.AutoSet(NXB_NEW);
	m_btnDelete.AutoSet(NXB_DELETE);
	m_btnClose.AutoSet(NXB_CLOSE);
	m_btnAddContact.AutoSet(NXB_NEW);
	m_btnDeleteContact.AutoSet(NXB_DELETE);
	//DRT 4/15/2008 - PLID 29663 - This is NOT an OK button
	//m_btnManageContacts.AutoSet(NXB_OK);
	m_btnAddPlan.AutoSet(NXB_NEW);
	m_btnDelPlan.AutoSet(NXB_DELETE);
	// (c.haag 2008-05-07 10:43) - PLID 29817 - NxIconify buttons
	m_btnBox24J.AutoSet(NXB_MODIFY);
	m_btnFacilityID.AutoSet(NXB_MODIFY);
	m_btnEditNetworkID.AutoSet(NXB_MODIFY);
	m_btnGroups.AutoSet(NXB_MODIFY);
	m_btnBox31.AutoSet(NXB_MODIFY);
	m_btnBox51.AutoSet(NXB_MODIFY);
	m_btnAcceptAssignment.AutoSet(NXB_MODIFY);
	m_btnEditCPTNotes.AutoSet(NXB_MODIFY);
	m_btnConfigLocationPayerIDs.AutoSet(NXB_MODIFY);
	// (j.jones 2011-04-05 14:49) - PLID 42372 - added the CLIA Setup
	m_btnCLIASetup.AutoSet(NXB_MODIFY);

	// (j.jones 2008-07-11 09:32) - PLID 30679 - added m_btnAdvInsDescSetup
	m_btnAdvInsDescSetup.AutoSet(NXB_MODIFY);
	// (j.jones 2008-08-01 10:35) - PLID 30917 - added m_btnClaimProviderSetup
	m_btnClaimProviderSetup.AutoSet(NXB_MODIFY);

	m_btnDefaultClaimFormSetup.AutoSet(NXB_MODIFY);

	//(8/17/2012) r.wilson - PLID 50636 - added m_btnEditDefaultDeductible
	m_btnEditDefaultDeductible.AutoSet(NXB_MODIFY);
}

void CEditInsInfoDlg::GetBox(int nID, CString & data)
{
	try{
		_variant_t tmpVar;
		switch (nID) {
		//case IDC_CONTACT_ZIP_BOX:
		//	GetDlgItemText(nID,data);
		//	break;
		case IDC_CONTACT_PHONE_BOX:
		case IDC_CONTACT_FAX_BOX: {
				GetDlgItemText(nID,data);
				CString str = data;
				UnformatText(str);
				str.TrimRight();
				if(str=="")
					data="";
				break;
			}
		case IDC_RADIO_TAX_INS:			
		case IDC_RADIO_TAX_PATIENT:
		case IDC_RADIO_TAX_NONE:
			if(m_radioTaxIns.GetCheck())
				data = "1";
			else if(m_radioTaxPatient.GetCheck())
				data = "2";
			else if(m_radioTaxNone.GetCheck())
				data = "3";
			break;
		case IDC_HMO_CHECK:
			if(m_HMOCheck.GetCheck()){
				data = "Yes";
			}else{
				data = "No";}
			break;
		case IDC_CHECK_USE_REFERRALS:
			if(m_ReferralCheck.GetCheck()){
				data = "Yes";
			}else{
				data = "No";}
			break;
		case IDC_WORKERS_COMP_CHECK:
			if(m_checkWorkersComp.GetCheck()) {
				data = "Yes";
			}else{
				data = "No";
			}
			break;
		case IDC_ENVOY:
			if (m_pEnvoyList->CurSel == -1) data = "";
			else data = VarString(m_pEnvoyList->GetValue(m_pEnvoyList->CurSel, hpicCode), "");
			break;
		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
		/*
		case IDC_THIN:
			if (m_pTHINList->CurSel == -1) data = "";
			else data = VarString(m_pTHINList->GetValue(m_pTHINList->CurSel, 1), "");
			break;
		*/
		case IDC_ELIGIBILITY_PAYER:
			if (m_pEligibilityPayerIDList->CurSel == -1) {
				data = "";
			}
			else {
				data = VarString(m_pEligibilityPayerIDList->GetValue(m_pEligibilityPayerIDList->CurSel, epicCode), "");
			}
			break;

		// (j.jones 2009-12-16 15:00) - PLID 36237 - added UB payerID
		case IDC_UB_PAYER_IDS:
			if (m_pUBPayerList->CurSel == -1) {
				data = "NULL";
			}
			else {
				data = AsString(VarLong(m_pUBPayerList->GetValue(m_pUBPayerList->CurSel, ubpicID), -1));
				if(data == "-1") {
					data = "NULL";
				}
			}
			break;		

		case IDC_INS_TYPE_COMBO:
			if (m_pInsuranceTypeList->CurSel == -1) {
				data = "NULL";
			}
			else {
				data = AsString(VarLong(m_pInsuranceTypeList->GetValue(m_pInsuranceTypeList->CurSel, 0), (long)itcInvalid));
				if(data == "-1") {
					data = "NULL";
				}
			}
			break;
		case IDC_CO_MEMO:
		case IDC_CONTACT_EXT_BOX:
		// (j.jones 2008-07-11 11:47) - PLID 28756 - added default payment/adjustment description
		case IDC_EDIT_DEF_PAY_DESCRIPTION:
		case IDC_EDIT_DEF_ADJ_DESCRIPTION:
			GetDlgItemText(nID, data);
			break;
		case IDC_HCFA_SETUP:
			tmpVar = m_pHCFAGroups->GetValue(m_pHCFAGroups->CurSel, 1);
			if (tmpVar.vt != VT_EMPTY) 
				data = VarString(tmpVar, "");
			else data = "";
			break;
		default:
			GetDlgItemText(nID, data);
			if(data != ""){
				SetUppersInString(GetDlgItem(nID), data);
				SetDlgItemText(nID, data);
			}
			break;
		}
	}NxCatchAll("Error in GetBox()");
}

LRESULT CEditInsInfoDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	try{
	if (message == WM_COMMAND) 
	{
		int nID = LOWORD(wParam);
		int nPersonID = 0;
		if((HIWORD(wParam) == EN_KILLFOCUS)  && !m_bSettingBox)
		{
			// (z.manning, 08/07/2007) - PLID 26979 - We had not been setting the m_bSettingBox variable, but now are.
			CGuardLock guardSettingBox(m_bSettingBox); // (a.walling 2011-08-29 12:20) - PLID 45232 - Use safe guard locks
			// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
			NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
			switch(nID)
			{
			case IDC_INSURANCE_PLAN_BOX: {					
					if(NULL == pCurInsRow)
						break;
					nPersonID = VarLong(pCurInsRow->GetValue(0), -1);
					CString strNewName, strOldName;
					GetDlgItemText(IDC_INSURANCE_PLAN_BOX, strNewName);
					//(e.lally 2007-08-08) PLID 26964 - We need to count the number of insured parties with this insurance.
					long nNumInsParties =0;
					_RecordsetPtr rs = CreateRecordset("SELECT InsuranceCoT.Name, "
						"	count(InsuranceCoID) AS NumInsParties "
						"FROM InsuranceCoT "
						"LEFT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
						"WHERE InsuranceCoT.PersonID = %li "
						"GROUP BY Name ",nPersonID);
					if(!rs->eof) {
						strOldName = AdoFldString(rs, "Name","");
						nNumInsParties = AdoFldLong(rs, "NumInsParties", 0);
					}
					rs->Close();

					// (c.haag 2006-10-06 16:15) - PLID 22904 - Don't let a user blank out an insurance company name.
					CString strCompare = strNewName;
					strCompare.TrimRight();
					if (strCompare.IsEmpty()) {
						MsgBox("You may not leave an insurance company name blank.");
						SetDlgItemText(IDC_INSURANCE_PLAN_BOX, strOldName);
						GetDlgItem(IDC_INSURANCE_PLAN_BOX)->SetFocus();
						((CNxEdit *)GetDlgItem(IDC_INSURANCE_PLAN_BOX))->SetSel( 0,-1, FALSE );
					}
					// (j.jones 2008-04-29 14:15) - PLID 26552 - we need to allow changing the
					// capitalization of an insurance company name, we just don't need to warn
					// them about it in this scenario, so I put the warning inside the
					// CompareNoCase() call, but updated the data anytime the formatting of the
					// name changed
					else if(strNewName != strOldName) {

						::SetFocus(NULL);

						if(strNewName.CompareNoCase(strOldName) != 0) {
							//different name
							//(e.lally 2007-08-08) PLID 26964 - We need to count the number of insured parties that
								//will be affected and report that number to make this warning stronger.
							CString strInsuredPartyMessage;
							if(nNumInsParties > 0){
								strInsuredPartyMessage.Format("There are %li insured parties that will be updated.\n\n",
								nNumInsParties);
							}
							CString strWarn;
							strWarn.Format("You are about to change the name of this insurance company from '%s' to '%s'.\n"
								"%s"
								"Are you sure you wish to do this?",strOldName,strNewName, strInsuredPartyMessage);
							if(IDNO == MessageBox(strWarn,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
								SetDlgItemText(IDC_INSURANCE_PLAN_BOX, strOldName);
							}
						}

						StoreBox(nID);
						GetDlgItem(IDC_INSURANCE_PLAN_BOX)->SetFocus();
					}
				}
				break;

			case IDC_CONTACT_ZIP_BOX:
				if(NULL == pCurInsRow)
					break;;
				// (j.gruber 2009-10-07 16:28) - PLID 35825 
				if (!m_bLookupByCity) {
					nPersonID = VarLong(pCurInsRow->GetValue(0), -1);
					StoreZipBoxes(this, IDC_CONTACT_CITY_BOX, IDC_CONTACT_STATE_BOX, IDC_CONTACT_ZIP_BOX, "PersonT", "ID", nPersonID, "City", "State", "Zip");
				}
				else {
					StoreBox(nID);
				}
			break;
			case IDC_CONTACT_CITY_BOX:
				if(NULL == pCurInsRow)
					break;;
				// (j.gruber 2009-10-07 16:28) - PLID 35825 
				if (m_bLookupByCity) {
					nPersonID = VarLong(pCurInsRow->GetValue(0), -1);
					StoreZipBoxes(this, IDC_CONTACT_CITY_BOX, IDC_CONTACT_STATE_BOX, IDC_CONTACT_ZIP_BOX, "PersonT", "ID", nPersonID, "City", "State", "Zip");
				}
				else {
					StoreBox(nID);
				}
			break;
			case IDC_CO_MEMO:
				if(ForceFieldLength((CNxEdit*)GetDlgItem(IDC_CO_MEMO))){
					StoreBox(nID);
				}
				//Otherwise, the focus has been put back, so we'll get another chance at it later.
			// (j.jones 2008-07-11 11:56) - PLID 28756 - added default pay/adj descriptions
			case IDC_EDIT_DEF_PAY_DESCRIPTION:
				if(ForceFieldLength((CNxEdit*)GetDlgItem(IDC_EDIT_DEF_PAY_DESCRIPTION))){
					StoreBox(nID);
				}
			case IDC_EDIT_DEF_ADJ_DESCRIPTION:
				if(ForceFieldLength((CNxEdit*)GetDlgItem(IDC_EDIT_DEF_ADJ_DESCRIPTION))){
					StoreBox(nID);
				}
			break;

			case IDC_CONTACT_PHONE_BOX:
			case IDC_CONTACT_FAX_BOX:
				if (SaveAreaCode(nID)) {
					StoreBox(nID);
				}
			break;
			default:
				StoreBox(nID);
			}
		}
		else if ((HIWORD(wParam) == EN_SETFOCUS) && !m_bSettingBox) {
			switch (nID) {
				case IDC_CONTACT_PHONE_BOX:
				case IDC_CONTACT_FAX_BOX:
					if (ShowAreaCode() ) {
						FillAreaCode(nID);
					}
				break;
				default:
				break;
			}
		}
	}
	}NxCatchAll("Error in CEditInsInfoDlg::WindowProc")
	return CNxDialog::WindowProc(message, wParam, lParam);
}

void CEditInsInfoDlg::StoreBox(int nID)
{
	try {
		//The current selection must be a valid one to store anything
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		ASSERT(NULL != pCurInsRow);
		if(NULL == pCurInsRow)
			return;
		_RecordsetPtr rs;
		rs = CreateRecordset("SELECT Count(PersonID) AS IDCount FROM InsuranceCoT WHERE PersonID = %li", VarLong(pCurInsRow->GetValue(0), 0));
		if (rs->Fields->GetItem("IDCount")->Value.lVal != 0) {
			GetBox(nID, m_strCurDataTmp);
			WriteField(nID, m_strCurDataTmp);
		}
		rs->Close();
	}NxCatchAll("Error in StoreBox()");
}

void CEditInsInfoDlg::WriteField(int nID, CString & data)
{
	CString table, field, keyfield, str, strTemp;
	//(e.lally 2011-07-15) PLID 42357 - Capture the original saved value without the quote markup for auditing.
	CString strAuditNew = data;
	long item = 0;

	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	ASSERT(NULL != pCurInsRow);
	if(NULL == pCurInsRow)
		return;

	long ID = VarLong(pCurInsRow->GetValue(0), -1);

	int nRvu;

	BOOL bUpdateHL7 = FALSE;

	// (b.eyers 2015-11-12) - PLID 66977
	BOOL bUpdateContactHL7 = FALSE;

	// (z.manning, 08/07/2007) - PLID 26797 - If we change an insurance contact's name and it's selected on patients
	// then we are going to eventually warn the user about it.
	long nInsuredPartyWarningCount = 0;
	_RecordsetPtr prsCount = NULL;
	
	if (data == "") data = " ";
	switch (nID) {
	case IDC_INSURANCE_PLAN_BOX:
		table = "InsuranceCoT";
		field = "Name";
		data = _Q(data);
		keyfield = "PersonID";
		GetDlgItemText(IDC_INSURANCE_PLAN_BOX,str);
		str.TrimLeft(" ");
		pCurInsRow->PutValue(1,_bstr_t(str));
		m_pInsList->Sort();
		item = aeiInsCoName;
		bUpdateHL7 = TRUE; // (z.manning 2009-01-08 17:45) - PLID 32663
		break;
	case IDC_CONTACT_ADDRESS_BOX:
		table = "PersonT";
		field = "Address1";
		keyfield = "ID";
		data = _Q(data);
		GetDlgItemText(IDC_CONTACT_ADDRESS_BOX,str);
		str.TrimLeft(" ");
		if(str!="") {
			pCurInsRow->PutValue(2,_bstr_t(str));
			m_pInsList->Sort();
		}
		item = aeiInsCoAddr;
		bUpdateHL7 = TRUE; // (z.manning 2009-01-08 17:45) - PLID 32663
		break;
	case IDC_CONTACT_ADDRESS2_BOX:
		table = "PersonT";
		field = "Address2";
		keyfield = "ID";
		data = _Q(data);
		item = aeiInsCoAddr2;
		bUpdateHL7 = TRUE; // (z.manning 2009-01-08 17:45) - PLID 32663
		break;
	case IDC_CONTACT_CITY_BOX:
		table = "PersonT";
		field = "City";
		keyfield = "ID";
		data = _Q(data);
		item = aeiInsCoCity;
		bUpdateHL7 = TRUE; // (z.manning 2009-01-08 17:45) - PLID 32663
		break;
	case IDC_CONTACT_STATE_BOX:
		table = "PersonT";
		field = "State";
		keyfield = "ID";
		data = _Q(data);
		item = aeiInsCoState;
		bUpdateHL7 = TRUE; // (z.manning 2009-01-08 17:45) - PLID 32663
		break;
	case IDC_CONTACT_ZIP_BOX:
		table = "PersonT";
		field = "Zip";
		keyfield = "ID";
		data = _Q(data);
		item = aeiInsCoZip;
		bUpdateHL7 = TRUE; // (z.manning 2009-01-08 17:45) - PLID 32663
		break;
	case IDC_RVU_BOX:
		table = "InsuranceCoT";
		field = "RVUMultiplier";
		keyfield = "PersonID";
		//convert to int and back
		nRvu = atoi((LPCTSTR)data);
		data.Format("%li", nRvu);
		SetDlgItemText(IDC_RVU_BOX, data);
		break;
	case IDC_CO_MEMO:
		table = "PersonT";
		field = "Note";
		keyfield = "ID";
		data = _Q(data);
		break;
	// (j.jones 2007-09-18 17:23) - PLID 27428 - added Crossover Code
	case IDC_EDIT_CROSSOVER_CODE:
		table = "InsuranceCoT";
		field = "CrossoverCode";
		keyfield = "PersonID";
		data = _Q(data);
		item = aeiInsCoCrossoverCode;
		break;

	// (j.jones 2010-08-30 16:02) - PLID 40302 - added TPL Code
	case IDC_EDIT_TPL_CODE:
		table = "InsuranceCoT";
		field = "TPLCode";
		keyfield = "PersonID";
		data = _Q(data);
		item = aeiInsCoTPLCode;
		break;

	// (j.jones 2008-07-11 11:53) - PLID 28756 - added default payment/adjustment description
	case IDC_EDIT_DEF_PAY_DESCRIPTION:
		table = "InsuranceCoT";
		field = "DefaultPayDesc";
		keyfield = "PersonID";
		data = _Q(data);
		item = aeiInsCoDefPaymentDescription;
		break;
	case IDC_EDIT_DEF_ADJ_DESCRIPTION:
		table = "InsuranceCoT";
		field = "DefaultAdjDesc";
		keyfield = "PersonID";
		data = _Q(data);
		item = aeiInsCoDefAdjustmentDescription;
		break;

	case IDC_CONTACT_FIRST_BOX: {
		table = "PersonT";
		field = "First";
		keyfield = "ID";
		if(m_pContactList->CurSel == -1)
			return;
		ID = VarLong(m_pContactList->GetValue(m_pContactList->CurSel, iccID), -1);
		m_pContactList->PutValue(m_pContactList->CurSel,iccFirst,_bstr_t(data));

		// (j.jones 2012-10-19 09:55) - PLID 51551 - we now show <No Contact Name Entered> in the combo, instead of just a comma,
		// if the name is blank
		CString strContactName;
		CString strFirst = data;
		CString strLast = VarString(m_pContactList->GetValue(m_pContactList->CurSel, iccLast), "");
		//trim spaces, for display purposes
		strFirst.TrimLeft(); strFirst.TrimRight();
		strLast.TrimLeft(); strLast.TrimRight();
		if(strFirst.IsEmpty() && strLast.IsEmpty()) {
			strContactName = "<No Contact Name Entered>";
		}
		else {
			strContactName = strLast;
			if(!strLast.IsEmpty() && !strFirst.IsEmpty()) {
				strContactName += ", ";
			}
			strContactName += strFirst;
		}

		m_pContactList->PutValue(m_pContactList->CurSel, iccFullName, (LPCTSTR)strContactName);
		m_pContactList->Sort();
		data = _Q(data);
		item = aeiInsCoContactFirst;
		// (z.manning, 08/07/2007) - PLID 26979 - We need to warn if this contact is selected on patients.
		prsCount = CreateParamRecordset("SELECT COUNT(*) AS Count FROM InsuredPartyT WHERE InsuranceContactID = {INT}", ID);
		nInsuredPartyWarningCount = AdoFldLong(prsCount, "Count", 0);
		prsCount->Close();
		// (b.eyers 2015-11-12) - PLID 66977
		bUpdateContactHL7 = TRUE;
		//if(m_checkMakeDefaultContact.GetCheck() == BST_CHECKED) { bUpdateHL7 = TRUE; } // (z.manning 2009-01-08 17:45) - PLID 32663
		break;
		}
	case IDC_CONTACT_LAST_BOX: {
		table = "PersonT";
		field = "Last";
		keyfield = "ID";
		if(m_pContactList->CurSel == -1)
			return;
		ID = VarLong(m_pContactList->GetValue(m_pContactList->CurSel, iccID), -1);
		m_pContactList->PutValue(m_pContactList->CurSel,iccLast,_bstr_t(data));
		
		// (j.jones 2012-10-19 09:55) - PLID 51551 - we now show <No Contact Name Entered> in the combo, instead of just a comma,
		// if the name is blank
		CString strContactName;
		CString strFirst = VarString(m_pContactList->GetValue(m_pContactList->CurSel, iccFirst), "");
		CString strLast = data;
		//trim spaces, for display purposes
		strFirst.TrimLeft(); strFirst.TrimRight();
		strLast.TrimLeft(); strLast.TrimRight();
		if(strFirst.IsEmpty() && strLast.IsEmpty()) {
			strContactName = "<No Contact Name Entered>";
		}
		else {
			strContactName = strLast;
			if(!strLast.IsEmpty() && !strFirst.IsEmpty()) {
				strContactName += ", ";
			}
			strContactName += strFirst;
		}

		m_pContactList->PutValue(m_pContactList->CurSel, iccFullName, (LPCTSTR)strContactName);

		m_pContactList->Sort();
		data = _Q(data);
		item = aeiInsCoContactLast;
		// (z.manning, 08/07/2007) - PLID 26979 - We need to warn if this contact is selected on patients.
		prsCount = CreateParamRecordset("SELECT COUNT(*) AS Count FROM InsuredPartyT WHERE InsuranceContactID = {INT}", ID);
		nInsuredPartyWarningCount = AdoFldLong(prsCount, "Count", 0);
		prsCount->Close();
		// (b.eyers 2015-11-12) - PLID 66977
		bUpdateContactHL7 = TRUE;
		//if(m_checkMakeDefaultContact.GetCheck() == BST_CHECKED) { bUpdateHL7 = TRUE; } // (z.manning 2009-01-08 17:45) - PLID 32663
		break;
		}
	case IDC_CONTACT_TITLE_BOX:
		table = "PersonT";
		field = "Title";
		keyfield = "ID";
		if(m_pContactList->CurSel == -1)
			return;
		ID = VarLong(m_pContactList->GetValue(m_pContactList->CurSel, iccID), -1);
		data = _Q(data);
		break;
	case IDC_CONTACT_PHONE_BOX:
		table = "PersonT";
		field = "WorkPhone";
		keyfield = "ID";
		if(m_pContactList->CurSel == -1)
			return;
		ID = VarLong(m_pContactList->GetValue(m_pContactList->CurSel, iccID), -1);
		m_pContactList->PutValue(m_pContactList->CurSel,iccPhone,_bstr_t(data));
		m_pContactList->Sort();
		data = _Q(data);
		item = aeiInsCoContactPhone;
		// (b.eyers 2015-11-12) - PLID 66977
		bUpdateContactHL7 = TRUE;
		//if(m_checkMakeDefaultContact.GetCheck() == BST_CHECKED) { bUpdateHL7 = TRUE; } // (z.manning 2009-01-08 17:45) - PLID 32663
		break;
	case IDC_CONTACT_EXT_BOX:
		table = "PersonT";
		field = "Extension";
		keyfield = "ID";
		if(m_pContactList->CurSel == -1)
			return;
		ID = VarLong(m_pContactList->GetValue(m_pContactList->CurSel, iccID), -1);
		data = _Q(data);
		item = aeiInsCoContactExt;
		// (b.eyers 2015-11-12) - PLID 66977
		bUpdateContactHL7 = TRUE;
		//if(m_checkMakeDefaultContact.GetCheck() == BST_CHECKED) { bUpdateHL7 = TRUE; } // (z.manning 2009-01-08 17:45) - PLID 32663
		break;
	case IDC_CONTACT_FAX_BOX:
		table = "PersonT";
		field = "Fax";
		if(m_pContactList->CurSel == -1)
			return;
		ID = VarLong(m_pContactList->GetValue(m_pContactList->CurSel, iccID), -1);
		keyfield = "ID";
		data = _Q(data);
		item = aeiInsCoContactFax;
		break;
	case IDC_ENVOY: {
		// (j.jones 2012-08-07 09:22) - PLID 51914 - we now save the ID, not the code itself
		_RecordsetPtr rs = CreateParamRecordset("SELECT EBillingInsCoIDs.EBillingID FROM InsuranceCoT "
			"INNER JOIN EBillingInsCoIDs ON InsuranceCoT.HCFAPayerID = EBillingInsCoIDs.ID "
			"WHERE PersonID = {INT}", ID);
		CString strOld = "<None>";
		if(!rs->eof) {
			strOld = AdoFldString(rs, "EBillingID", "<None>");
		}
		rs->Close();

		CString strNew = "<None>";
		long nHCFAID = -1;
		_variant_t varID = g_cvarNull;
		if(m_pEnvoyList->GetCurSel() != -1) {
			nHCFAID = VarLong(m_pEnvoyList->GetValue(m_pEnvoyList->GetCurSel(), hpicID), -1);
			if(nHCFAID != -1) {
				varID = nHCFAID;
				strNew = VarString(m_pEnvoyList->GetValue(m_pEnvoyList->GetCurSel(), hpicCode), "<None>");
			}
		}

		ExecuteParamSql("UPDATE InsuranceCoT SET HCFAPayerID = {VT_I4} WHERE PersonID = {INT}", varID, ID);

		//auditing
		if(strOld != strNew) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoPayer, long(ID), strOld, strNew, aepMedium, aetChanged);
		}

		//Update HL7 patients
		// (d.thompson 2012-08-28) - PLID 52348 - Only do this if the audit values actually changed - otherwise we're doing unnecessary updates
		// (j.jones 2015-10-30 16:26) - PLID 67481 - payer IDs aren't used in HL7

		return;

		}
		break;
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
	/*
	case IDC_THIN:
		table = "InsuranceCoT";
		field = "THIN";
		keyfield = "PersonID";
		data = _Q(data);
		item = aeiInsCoTHIN;
		break;
	*/
	case IDC_ELIGIBILITY_PAYER: {
		// (j.jones 2012-08-07 09:22) - PLID 51914 - we now save the ID, not the code itself
		_RecordsetPtr rs = CreateParamRecordset("SELECT EBillingInsCoIDs.EBillingID FROM InsuranceCoT "
			"INNER JOIN EBillingInsCoIDs ON InsuranceCoT.EligPayerID = EBillingInsCoIDs.ID "
			"WHERE PersonID = {INT}", ID);
		CString strOld = "<None>";
		if(!rs->eof) {
			strOld = AdoFldString(rs, "EBillingID", "<None>");
		}
		rs->Close();

		CString strNew = "<None>";
		long nEligID = -1;
		_variant_t varID = g_cvarNull;
		if(m_pEligibilityPayerIDList->GetCurSel() != -1) {
			nEligID = VarLong(m_pEligibilityPayerIDList->GetValue(m_pEligibilityPayerIDList->GetCurSel(), epicID), -1);
			if(nEligID != -1) {
				varID = nEligID;
				strNew = VarString(m_pEligibilityPayerIDList->GetValue(m_pEligibilityPayerIDList->GetCurSel(), epicCode), "<None>");
			}
		}

		ExecuteParamSql("UPDATE InsuranceCoT SET EligPayerID = {VT_I4} WHERE PersonID = {INT}", varID, ID);

		//auditing
		if(strOld != strNew) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoEligibility, long(ID), strOld, strNew, aepMedium, aetChanged);
		}

		//Update HL7 patients
		// (d.thompson 2012-08-28) - PLID 52348 - Only do this if the audit values actually changed - otherwise we're doing unnecessary updates
		// (j.jones 2015-10-30 16:26) - PLID 67481 - payer IDs aren't used in HL7

		return;

		}
		break;

	// (j.jones 2009-12-16 15:00) - PLID 36237 - added UB payerID
	case IDC_UB_PAYER_IDS: {

		//this change saves immediately

		_RecordsetPtr rs = CreateParamRecordset("SELECT EBillingInsCoIDs.EBillingID FROM InsuranceCoT "
			"INNER JOIN EBillingInsCoIDs ON InsuranceCoT.UBPayerID = EBillingInsCoIDs.ID "
			"WHERE PersonID = {INT}", ID);
		CString strOld = "<None>";
		if(!rs->eof) {
			strOld = AdoFldString(rs, "EBillingID", "<None>");
		}
		rs->Close();

		CString strNew = "<None>";
		long nUBID = -1;
		_variant_t varID = g_cvarNull;
		if(m_pUBPayerList->GetCurSel() != -1) {
			nUBID = VarLong(m_pUBPayerList->GetValue(m_pUBPayerList->GetCurSel(), ubpicID), -1);
			if(nUBID != -1) {
				varID = nUBID;
				strNew = VarString(m_pUBPayerList->GetValue(m_pUBPayerList->GetCurSel(), ubpicCode), "<None>");
			}
		}

		ExecuteParamSql("UPDATE InsuranceCoT SET UBPayerID = {VT_I4} WHERE PersonID = {INT}", varID, ID);

		//auditing
		if(strOld != strNew) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoUBPayerID, long(ID), strOld, strNew, aepMedium, aetChanged);
		}

		//Update HL7 patients
		// (d.thompson 2012-08-28) - PLID 52348 - Only do this if the audit values actually changed - otherwise we're doing unnecessary updates
		// (j.jones 2015-10-30 16:26) - PLID 67481 - payer IDs aren't used in HL7

		return;

		}
		break;

	case IDC_HMO_CHECK:
		{
		CString strNew, strOld;
		strNew = data;
		strOld = (data == "Yes") ? "No" : "Yes";
		table = "InsuranceCoT";
		field = "HMO";
		keyfield = "PersonID";
		data = (data == "Yes") ? "1": "0";
		//check boxes... 
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoHMO, long(pCurInsRow->GetValue(0).lVal), strOld, strNew, aepMedium, aetChanged);
		//
		}
		break;
	case IDC_INS_TYPE_COMBO: {;
		// (j.jones 2008-09-09 08:45) - PLID 18695 - renamed to InsType, and changed to save immediately
		_RecordsetPtr rs = CreateParamRecordset("SELECT InsType FROM InsuranceCoT WHERE PersonID = {INT}", ID);
		CString strOld = "<None>";
		if(!rs->eof) {
			InsuranceTypeCode eInsType = (InsuranceTypeCode)AdoFldLong(rs, "InsType", (long)itcInvalid);
			strOld = GetNameFromInsuranceType(eInsType);
			if(strOld.IsEmpty()) {
				strOld = "<None>";
			}
		}
		rs->Close();

		CString strNew = "<None>";
		long nInsType = atoi(data);
		if(data != "NULL" && nInsType > 0) {
			InsuranceTypeCode eInsType = (InsuranceTypeCode)nInsType;
			strNew = GetNameFromInsuranceType(eInsType);
			if(strNew.IsEmpty()) {
				strNew = "<None>";
			}
		}
		ExecuteSql("UPDATE InsuranceCoT SET InsType = %s WHERE PersonID = %li", data, ID);

		//auditing
		if(strOld != data) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoInsType, long(ID), strOld, strNew, aepMedium, aetChanged);
		}

		// (z.manning 2009-01-08 17:48) - PLID 32663 - Update HL7 patients
		// (r.goldschmidt 2015-11-04 16:38) - PLID 67520 - Do not mass export ADT messages updates when editing the insurance type

		return;
		}
		break;
	case IDC_RADIO_TAX_INS:
	case IDC_RADIO_TAX_PATIENT:
	case IDC_RADIO_TAX_NONE:
		{
			table = "InsuranceCoT";
			field = "TaxType";
			keyfield = "PersonID";
			//auditing
			CString strOld, strNew;
			//get the old value
			_RecordsetPtr rs = CreateRecordset("SELECT TaxType FROM InsuranceCoT WHERE PersonID = %li", long(pCurInsRow->GetValue(0).lVal));
			switch(AdoFldLong(rs, "TaxType", -1)) {
			case 3:	strOld = "No Tax";	break;
			case 1:	strOld = "Tax Insurance";	break;
			case 2:	strOld = "Tax Patient";	break;
			default:	strOld = "";	break;
			}
			switch(atoi(data)) {
			case 3:	strNew = "No Tax";	break;
			case 1:	strNew = "Tax Insurance";	break;
			case 2:	strNew = "Tax Patient";	break;
			default:	strNew = "";	break;
			}

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoTaxType, long(pCurInsRow->GetValue(0).lVal), strOld, strNew, aepMedium, aetChanged);
			//end audit
		}
		break;
	case IDC_CHECK_USE_REFERRALS: {
		table = "InsuranceCoT";
		field = "UseReferrals";
		keyfield = "PersonID";
		CString strNew, strOld;
		strNew = data;
		strOld = (data == "Yes") ? "No" : "Yes";
		data = (data == "Yes") ? "1": "0";
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoUseReferrals, long(pCurInsRow->GetValue(0).lVal), strOld, strNew, aepMedium, aetChanged);
		}
		break;
	case IDC_WORKERS_COMP_CHECK:
		table = "InsuranceCoT";
		field = "WorkersComp";
		keyfield = "PersonID";
		data = (data == "Yes") ? "1": "0";
		break;
	}

	try{
		//for auditing, and detecting whether network code needs to be sent
		CString strOld;
		_RecordsetPtr rs = CreateRecordset("SELECT %s AS Field FROM %s WHERE %s = %li", field, table, keyfield, ID);
		_variant_t var = rs->Fields->Item["Field"]->Value;
		// (j.jones 2006-11-03 12:36) - PLID 23346 - AsString doesn't support VT_BOOL
		if(var.vt == VT_BOOL) {
			strOld = VarBool(var) ? "1" : "0";
		}
		else if(var.vt != VT_NULL) {
			strOld = AsString(var);
		}
		else {
			strOld = "";
		}

		// (z.manning, 08/07/2007) - PLID 26979 - Warn them if applicable.
		//(e.lally 2011-07-15) PLID 42357 - Compare on the original saved value without the quote markup
		if(nInsuredPartyWarningCount > 0 && strOld != strAuditNew)
		{
			int nResult = MessageBox(FormatString(
				"Changing this value will change it everywhere in the program including %li patient insured party record(s).\r\n\r\n"
				"Are you sure you want to change it?", nInsuredPartyWarningCount), NULL, MB_YESNO);
			if(nResult != IDYES) {
				// (z.manning, 08/07/2007) - PLID 26979 - They chose to not commit, so put the old value back in the text box.
				if(var.vt == VT_BSTR) {
					SetDlgItemText(nID, VarString(var));
				}
				else {
					ASSERT(FALSE);
				}
				return;
			}
		}

		ExecuteSql("UPDATE %s SET %s = '%s' WHERE %s = %li", table, field, data, keyfield, ID);
		//auditing
		strOld.TrimRight();
		strAuditNew.TrimRight();
		data.TrimRight();
		//(e.lally 2011-07-15) PLID 42357 - Audit the original saved value without the quote markup.
		//	I don't know what the purpose of the trim above is (after the commit to data), but I'll keep that same behavior here.
		if(item != 0 && strOld != strAuditNew) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, item, long(ID), strOld, strAuditNew, aepMedium, aetChanged);
		}
		// (b.cardillo 2006-09-19 14:37) - PLID 22570 - Only send the table-checker message 
		// if something actually changed.  Otherwise we risk (and indeed we actually get) an 
		// infinite loop of refreshing because this checker is picked up by the insurance 
		// sheet of admin, which refreshes in response, which kills focus on the control the 
		// user is currently in, which is undoubtedly the control associated with this call 
		// to WriteField, which means we'll get called again.  If we send a table-checker 
		// every time (which we were until now), then it would just start the loop over.  So 
		// now sending the table-checker only if something changed, we stop the loop exactly 
		// at the point that it needs to be stopped.
		//(e.lally 2011-07-15) PLID 42357 - Compare on the original saved value without the quote markup
		if (strOld != strAuditNew) {
			CClient::RefreshTable(NetUtils::InsuranceCoT);
		}

		// (b.eyers 2015-11-12) - PLID 66977
		if (bUpdateContactHL7 && strOld != strAuditNew) {
			UpdateExistingHL7PatientsByInsContact(VarLong(m_pContactList->GetValue(m_pContactList->CurSel, iccID), -1));
		}
		// (z.manning 2009-01-08 17:48) - PLID 32663 - Do we need to update HL7 patients?
		// (d.thompson 2012-08-28) - PLID 52348 - Don't update patients unless the data actually changed!
		if(bUpdateHL7 && strOld != strAuditNew) {
			UpdateExistingHL7PatientsByInsCoID(VarLong(pCurInsRow->GetValue(0), -1));
		}

	}NxCatchAll("Error in WriteField()");

	if (data == " ") data = "";
}

void CEditInsInfoDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);

	m_bAutoSetContact = TRUE;
}

void CEditInsInfoDlg::OnAddPlan() 
{
	CGetNewIDName GetName(this);
	CString NewName;
	CString* pNewName = &NewName;
	GetName.m_pNewName = pNewName;
	GetName.m_nMaxLength = 255;
	if (GetName.DoModal() == IDCANCEL) return;

	//THIS NAME IS ALLOWED TO BE BLANK AND IS ALLOWED TO BE SPACES - FOR EBILLING. CHANGE IT AND FACE THE WRATH OF ALL.

//	long nID = NewNumber("InsurancePlanT","ID");
	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	long nInsCo = VarLong(pCurInsRow->GetValue(0), -1);
	
	try {
		if(ReturnsRecords("SELECT ID FROM InsurancePlansT WHERE PlanName = '%s' AND InsCoID = %li", _Q(NewName), nInsCo)) {
			MessageBox("This company already has a plan with this name.");
			return;
		}

		//DRT 7/22/03 - Added a preference for default plan type - use it.
		CString strType = GetRemotePropertyText("DefaultInsPlanType", "Other", 0, "<None>", true);

		//PLID 20359 //changed to insert a row instead of requery
		
		long nNewPlanID = NewNumber("InsurancePlansT", "ID");
		ExecuteSql("INSERT INTO InsurancePlansT (ID, PlanName, PlanType, InsCoID) VALUES (%li,'%s', '%s', %li);", nNewPlanID, _Q(NewName), _Q(strType), nInsCo);

		NXDATALISTLib::IRowSettingsPtr pRow = m_pInsPlans->GetRow(-1);
		pRow->PutValue(0, nNewPlanID);
		pRow->PutValue(1, _variant_t(NewName));
		pRow->PutValue(2, _variant_t(strType));
		m_pInsPlans->AddRow(pRow);

		//auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoPlanAdd, long(pCurInsRow->GetValue(0).lVal), "", NewName, aepMedium, aetCreated);
		//
	}NxCatchAll("Error in OnAddPlan()");
	
	//PLID 20359: changed to add a row instead of requery
	//m_pInsPlans->Requery();

	RefreshPlanButtons();

	// Network change event
	CClient::RefreshTable(NetUtils::InsurancePlansT);
}

void CEditInsInfoDlg::OnDeletePlan() 
{
	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	if(NULL == pCurInsRow) {
		return;
	}

	if(m_pInsPlans->CurSel == -1) {
		AfxMessageBox("Please select a plan to delete.");
		return;
	}

	_variant_t tmpVar = m_pInsPlans->GetValue(m_pInsPlans->CurSel, 0);
	if (tmpVar.vt == VT_NULL) return;
	try {
		long nPlanID = VarLong(tmpVar);
		long nPatientCount = 0;
		//(e.lally 2012-02-06) PLID 27754 - Get the number of patients with at least one insured party that has this plan. Warn the user how many patient this will affect.
		_RecordsetPtr rs = CreateParamRecordset("SELECT COUNT(DISTINCT PatientID) AS PatientCount FROM InsuredPartyT WHERE InsPlan = {INT} ", nPlanID);
		if(!rs->eof){
			nPatientCount = AdoFldLong(rs->Fields, "PatientCount", 0);
		}
		CString strPatients = (nPatientCount == 1 ? "There is 1 patient" : FormatString("There are %li patients", nPatientCount));
		if(IDNO == MessageBox(FormatString("%s with this plan.\r\n"
			"Are you sure you wish to delete this plan?", strPatients)
			,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
			return;
		}

		CString strOld = CString(m_pInsPlans->GetValue(m_pInsPlans->CurSel, 1).bstrVal);

		// (d.thompson 2009-03-20) - PLID 33061 - There is now a preference for default plan we may need to clear as well.
		//	I parameterized this query while I was at it.  We just wipe the configrt preference altogether so it goes back
		//	to its default.
		CString strSqlBatch;
		CNxParamSqlArray args;
		// (d.thompson 2012-08-28) - PLID 52129 - We now need to update HL7 messages for anyone using this batch, since it's being deleted.  Obviously
		//	I can't query the patients using it after it's deleted, so we're going to need to select them all first, then delete them, then pass
		//	that set of patients over.
		AddParamStatementToSqlBatch(strSqlBatch, args, "SELECT PatientID FROM InsuredPartyT WHERE InsPlan = {INT}", nPlanID);
		AddDeclarationToSqlBatch(strSqlBatch, "SET NOCOUNT ON;\r\n");
		//(e.lally 2012-02-06) PLID 27754 - Remove the FK reference to this InsPlan for all insured parties with it.
		AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE InsuredPartyT SET InsPlan = NULL WHERE InsPlan = {INT};\r\n", nPlanID);
		AddParamStatementToSqlBatch(strSqlBatch, args, "DELETE FROM InsurancePlansT WHERE ID = {INT};\r\n", nPlanID);
		AddDeclarationToSqlBatch(strSqlBatch, "SET NOCOUNT OFF;\r\n");
		AddStatementToSqlBatch(strSqlBatch, "SELECT IntParam FROM ConfigRT WHERE Name = 'DefaultInsurancePlanNewPatient'");

		// (e.lally 2009-06-21) PLID 34679 - Fixed to use create recordset function.
		// (d.thompson 2012-08-28) - PLID 52129 - First recordset is now the patients who used to have this plan.
		_RecordsetPtr prsPatientsUsingPlan = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, args);
		_RecordsetPtr prs = prsPatientsUsingPlan->NextRecordset(NULL);
		if(!prs->eof) {
			long nRetVal = AdoFldLong(prs, "IntParam");
			if(nRetVal == nPlanID) {
				//The configRT setting matches this plan, so wipe it.  We do it here because the caching must
				//	be updated.
				SetRemotePropertyInt("DefaultInsurancePlanNewPatient", -1, 0, "<None>");
			}
		}
		else {
			//It may not exist if noone ever referenced the preference
		}

		//auditing
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoPlanDelete, long(pCurInsRow->GetValue(0).lVal), strOld, "<Deleted>", aepMedium, aetDeleted);
		//

		// (d.thompson 2012-08-28) - PLID 52129 - We removed this plan, so update HL7 as it may be used - we queried the patients before we did the delete.
		// (r.goldschmidt 2015-11-09 10:57) - PLID 67517 - need to know if this is an attempted mass insurance update
		UpdateExistingHL7PatientsByRecordset(prsPatientsUsingPlan, true);

	}NxCatchAll("Error in OnDeletePlan()");
	m_pInsPlans->RemoveRow(m_pInsPlans->CurSel);
	m_pInsPlans->CurSel = 0;

	RefreshPlanButtons();

	// Network change event
	CClient::RefreshTable(NetUtils::InsurancePlansT);
}

// Throws exceptions
// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
void CEditInsInfoDlg::ReflectInsListSelection(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// Store the new id in a local variable, to be used in this function 
	// and then it is what we set our shared member variable to once this 
	// function has completed successfully
	long nNewInsCoID = VarLong(pRow->GetValue(0));
	
	// Refresh the plans combo box, based on the insurance company that 
	// we're now going to look at
	{
		CString strSql;
		strSql.Format("InsurancePlansT.InsCoID = %li", nNewInsCoID);
		m_pInsPlans->WhereClause = (LPCTSTR)strSql;
		m_pInsPlans->Requery();
	}

	// Refresh the contacts combo box, based on the insurance company that 
	// we're now going to look at
	{
		CString strSql;
		strSql.Format("InsuranceContactsT.InsuranceCoID = %li", nNewInsCoID);
		m_pContactList->WhereClause = (LPCTSTR)strSql;
		m_pContactList->Requery();
	}

	// Finally reflect the new ID now that everything has gone through
	// (c.haag 2010-05-06 12:29) - PLID 37525 - We don't need m_nCurInsCoID. If anything, a disparity
	// between that value and the list's current selection can spell trouble.
	//m_nCurInsCoID = nNewInsCoID;
	// We also know that we're no longer on a new entry
	m_bIsNewCo = false;

	// TODO: Ultimately the UpdateView would simply call ReflectInsListSelection 
	// instead of vice versa, and this function would do all the loading.
	UpdateView();
}

void CEditInsInfoDlg::OnSelChosenHcfaSetup(long nNewSel) 
{
	try{
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow)
			return;

		if(nNewSel == -1)
			return;

		//for auditing
		CString strOld;
		_RecordsetPtr rs = CreateRecordset("SELECT HCFASetupT.Name FROM HCFASetupT INNER JOIN InsuranceCoT ON HCFASetupT.ID = InsuranceCoT.HCFASetupGroupID WHERE InsuranceCoT.PersonID = %li", VarLong(pCurInsRow->GetValue(0)));
		if(!rs->eof && rs->Fields->Item["Name"]->Value.vt == VT_BSTR)
			strOld = CString(rs->Fields->Item["Name"]->Value.bstrVal);
		//

		ExecuteSql("UPDATE InsuranceCoT SET HCFASetupGroupID = %li WHERE PersonID = %li;", VarLong(m_pHCFAGroups->GetValue(m_pHCFAGroups->CurSel,0), -1), VarLong(pCurInsRow->GetValue(0), -1));

		//auditing
		if(strOld != CString(m_pHCFAGroups->GetValue(nNewSel, 1).bstrVal)) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoHCFA, long(pCurInsRow->GetValue(0).lVal), strOld, CString(m_pHCFAGroups->GetValue(nNewSel, 1).bstrVal), aepMedium, aetChanged);
		}
	}NxCatchAll("Error in OnSelChangedHcfaSetup()");
}

// (c.haag 2009-03-16 10:06) - PLID 9148 - Financial class combo selection change
void CEditInsInfoDlg::SelChosenComboFinancialClass(long nRow)
{
	try {
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}

		if(nRow == -1) {
			return;
		}

		const long nInsCoID = VarLong(pCurInsRow->GetValue(0));
		const CString strInsCoName = VarString(pCurInsRow->GetValue(1));
		const _variant_t vNewFinancialClassID = m_pFinancialClassCombo->GetValue(nRow,0);
		CString strNew;
		if (VT_NULL != vNewFinancialClassID.vt) {
			strNew = VarString(m_pFinancialClassCombo->GetValue(nRow, 1));
		}

		// Verify we're actually making a change, and fetch the old name if we are
		CString strOld;
		_RecordsetPtr rs = CreateParamRecordset("SELECT FinancialClassT.ID, FinancialClassT.Name FROM FinancialClassT INNER JOIN InsuranceCoT ON FinancialClassT.ID = InsuranceCoT.FinancialClassID WHERE InsuranceCoT.PersonID = {INT}", nInsCoID);
		if (!rs->eof) {
			if (vNewFinancialClassID == rs->Fields->Item["ID"]->Value) {
				return; // No change
			}
			strOld = AdoFldString(rs, "Name"); // Names are non-nullable
		} else {
			if (vNewFinancialClassID.vt == VT_NULL) {
				return; // No change
			}
			strOld = "";
		}

		if(strOld != strNew) {
			// Update the insurance company
			ExecuteParamSql("UPDATE InsuranceCoT SET FinancialClassID = {VT_I4} WHERE PersonID = {INT}", vNewFinancialClassID, nInsCoID);
			// Audit
			AuditEvent(-1, strInsCoName, BeginNewAuditEvent(), aeiInsCoFinancialClass, nInsCoID, strOld, strNew, aepMedium, aetChanged);
		}
	}
	NxCatchAll("Error in CEditInsInfoDlg::SelChosenComboFinancialClass");
}

void CEditInsInfoDlg::OnChangeContactFaxBox() 
{
	CString str;
	GetDlgItemText(IDC_CONTACT_FAX_BOX, str);
	str.TrimRight();
	if (str != "") {
		if(m_bFormatPhoneNums) {
			FormatItem(IDC_CONTACT_FAX_BOX, m_strPhoneFormat);
		}
	}
}

void CEditInsInfoDlg::OnChangeContactPhoneBox() 
{
	CString str;
	GetDlgItemText(IDC_CONTACT_PHONE_BOX, str);
	str.TrimRight();
	if (str != "") {
		if(m_bFormatPhoneNums) {
			FormatItem(IDC_CONTACT_PHONE_BOX, m_strPhoneFormat);
		}
	}
}

void CEditInsInfoDlg::OnEditingFinishingInsPlans(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{

	// (j.gruber 2007-08-07 12:17) - PLID 26965 - warn if there are patients with this plan
	switch(nCol) {
		case 1:
			//get the ID first;
			if (*pbCommit) {
				long nPlanID = VarLong(m_pInsPlans->GetValue(nRow, 0));
				_RecordsetPtr rsCount = CreateParamRecordset("SELECT COUNT(*) AS Count FROM InsuredPartyT WHERE InsPlan = {INT}", nPlanID);
				if (! rsCount->eof) {
					long nCount = AdoFldLong(rsCount, "Count", -1);
					CString strMsg;
					if (nCount == 1 && (VarString(varOldValue, "").CompareNoCase(VarString(pvarNewValue, "")) != 0)) {
						strMsg.Format("Changing the name of this Insurance Plan will change it everywhere in Practice, including\n"
							" %li Insured Party.  Are you sure you want to do this?", nCount);
					}
					else if (nCount > 1 && (VarString(varOldValue, "").CompareNoCase(VarString(pvarNewValue, "")) != 0)) {
						strMsg.Format("Changing the name of this Insurance Plan will change it everywhere in Practice, including\n"
							" %li Insured Parties.  Are you sure you want to do this?", nCount);
					}

					if (!strMsg.IsEmpty()) {
						
						if (IDNO == MsgBox(MB_YESNO, strMsg)) {
							//they said no
							*pbCommit = FALSE;
							*pbContinue = TRUE;
						}
					}
				}
			}
		break;
	}		

			
}

void CEditInsInfoDlg::OnEditingFinishedInsPlans(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	CString str,name;

	try {
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}

		long nPlanID = m_pInsPlans->GetValue(nRow,0).lVal;

		switch(nCol) {
			case 1:
			{
				// (j.gruber 2007-08-07 12:17) - PLID 26965 - added the bCommit check
				if (bCommit) {
					name = VarString(varNewValue);
					long nInsCo = VarLong(pCurInsRow->GetValue(0), -1);
					if(ReturnsRecords("SELECT ID FROM InsurancePlansT WHERE PlanName = '%s' AND InsCoID = %li AND ID <> %li", _Q(name), nInsCo, nPlanID)) {
						MessageBox("This company already has a plan with this name.");
						//put the name back
						m_pInsPlans->PutValue(nRow, 1, &varOldValue);
						return;
					}

					//DO NOT EVER MAKE IT SO A PLANNAME CANNOT BE BLANK. EVER. They need it for ebilling!
					// (j.jones 2010-04-19 16:16) - PLID 30852 - corrected potential formatting issues by simply parameterizing
					ExecuteParamSql("UPDATE InsurancePlansT SET PlanName = {STRING} WHERE ID = {INT}",name,nPlanID);

					// (d.thompson 2012-08-28) - PLID 52129 - We updated this plan, so update HL7 as it may be used
					UpdateExistingHL7PatientsByInsCoIDAndPlan(nInsCo, nPlanID);

				}
			}
			break;
			case 2:
				name = VarString(varNewValue);
				// (j.jones 2010-04-19 16:16) - PLID 30852 - corrected potential formatting issues by simply parameterizing
				ExecuteParamSql("UPDATE InsurancePlansT SET PlanType = {STRING} WHERE ID = {INT}",name,nPlanID);
			break;
		}
	} NxCatchAll("Error in changing Insurance Plan data.");

	//m_PlanList->Requery();	
}

// (j.jones 2009-12-16 15:00) - PLID 36237 - renamed to OnEditPayerIDs
void CEditInsInfoDlg::OnEditPayerIDs() 
{
	_variant_t var1, var2, var3;

	try {
		//save the selections
		if(m_pEnvoyList->GetCurSel()!=-1) {
			var1 = m_pEnvoyList->GetValue(m_pEnvoyList->GetCurSel(), hpicID);
		}
		if(m_pEligibilityPayerIDList->GetCurSel()!=-1) {
			var2 = m_pEligibilityPayerIDList->GetValue(m_pEligibilityPayerIDList->GetCurSel(), epicID);
		}
		// (j.jones 2009-12-16 15:00) - PLID 36237 - added UB payer ID
		if(m_pUBPayerList->GetCurSel()!=-1) {
			var3 = m_pUBPayerList->GetValue(m_pUBPayerList->GetCurSel(), ubpicID);
		}

		CEditInsPayersDlg dlg(this);
		// (j.jones 2009-08-04 11:34) - PLID 14573 - removed format type
		//dlg.m_FormatType = 1;
		dlg.DoModal();

		//requery all lists
		m_pEnvoyList->Requery();
		m_pEligibilityPayerIDList->Requery();
		m_pUBPayerList->Requery();

		// (j.jones 2009-12-17 09:34) - PLID 36237 - none of these ever re-added the "none" row, now they do
		NXDATALISTLib::IRowSettingsPtr pRow;
		_variant_t var;
		var.vt = VT_NULL;
		pRow = m_pEnvoyList->GetRow(-1);
		pRow->PutValue(hpicID,(long)-1);
		pRow->PutValue(hpicCode,(_bstr_t)"");
		pRow->PutValue(hpicName,(_bstr_t)"<No Payer ID Selected>");
		m_pEnvoyList->AddRow(pRow);

		pRow = m_pEligibilityPayerIDList->GetRow(-1);
		pRow->PutValue(epicID,(long)-1);
		pRow->PutValue(epicCode,(_bstr_t)"");
		pRow->PutValue(epicName,(_bstr_t)"<No Eligibility ID Selected>");
		m_pEligibilityPayerIDList->AddRow(pRow);

		pRow = m_pUBPayerList->GetRow(-1);
		pRow->PutValue(ubpicID,(long)-1);
		pRow->PutValue(ubpicCode,(_bstr_t)"");
		pRow->PutValue(ubpicName,(_bstr_t)"<No UB ID Selected>");
		m_pUBPayerList->AddRow(pRow);

		if(var1.vt == VT_I4) {
			m_pEnvoyList->SetSelByColumn(hpicID, var1);
		}
		
		if(var2.vt == VT_I4) {
			m_pEligibilityPayerIDList->SetSelByColumn(epicID, var2);
		}

		if(var3.vt == VT_I4) {
			m_pUBPayerList->SetSelByColumn(ubpicID, var3);
		}

		UpdateView();

	}NxCatchAll("Error editing payer list.");
}

// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
/*
void CEditInsInfoDlg::OnEditThinList() 
{
	_variant_t var;

	try {
		//save the selection
		if(m_pTHINList->GetCurSel()!=-1)
			var = m_pTHINList->GetValue(m_pTHINList->GetCurSel(),0);

		CEditInsPayersDlg dlg;
		dlg.m_FormatType = 2;
		dlg.DoModal();

		m_pTHINList->Requery();
		if(var.vt==VT_I4)
			m_pTHINList->SetSelByColumn(0,var);

		UpdateView();

	}NxCatchAll("Error editing THIN payer list.");
}
*/

void CEditInsInfoDlg::OnAddCompany() 
{

	if(!CheckCurrentUserPermissions(bioInsuranceCo,sptCreate))
		return;

	//int result = AfxMessageBox("Are you sure you would like to add and insrance company?",	MB_ICONQUESTION|MB_YESNO);
	//if (result == IDYES) {

	// Warn the user if anything is out of whack
	if(!CheckOnExitingCurrentInsCo(ecicaCreatingInsCo)){
			return;
	}
	
	CString strResult;
		
	try {
		int nResult = InputBoxLimited(this, "Enter a name for this Insurance Company:", strResult, "",255,false,false,NULL);

		strResult.TrimRight();
		
		if (nResult == IDOK)
		{
			while (strResult == "" || strResult.GetLength() > 255) {
				if(strResult == "") 
					MessageBox("You cannot add an insurance company with a blank name.");
				else 
					MessageBox("An insurance company cannot have a name longer than 255 characters.");

				nResult = InputBoxLimited(this, "Enter a name for this Insurance Company:", strResult, "",255,false,false,NULL);
				if (nResult == IDCANCEL) {
					return;
				}
				strResult.TrimRight();
				
			}

			//quick and dirty capitalization
			CString firstchar = strResult.GetAt(0);
			firstchar.MakeUpper();
			strResult.SetAt(0,firstchar.GetAt(0));

			int nNewNum = NewNumber("PersonT","ID");

			// Save the new record
			// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
			{
				CSqlTransaction trans;
				trans.Begin();
				ExecuteSql("INSERT INTO PersonT (ID, UserID) VALUES (%li, %li)", nNewNum, GetCurrentUserID());
				// (j.jones 2010-07-19 10:09) - PLID 39702 - InsType now defaults to NULL
				// (j.jones 2012-08-07 09:22) - PLID 51914 - EbillingID no longer exists
				ExecuteSql("INSERT INTO InsuranceCoT (PersonID, Name, UserDefinedID, RVUMultiplier, EBillingClaimOffice, InsType, TaxType) VALUES (%li, '%s', '', '', '', NULL, %li)", nNewNum, _Q(strResult), GetRemotePropertyInt("InsDefaultTaxType", 2, 0, "<None>",TRUE));
				ExecuteSql("INSERT INTO InsuranceAcceptedT (InsuranceCoID, ProviderID, Accepted) SELECT %li AS InsCoID, PersonID, %li AS Accepted FROM ProvidersT",nNewNum,GetRemotePropertyInt("DefaultInsAcceptAssignment",1,0,"<None>",TRUE) == 1 ? 1 : 0);
				trans.Commit();
			}

			// Requery the screen
			// (c.haag 2010-05-06 13:21) - PLID 37525 - Don't requery the whole list; just add the row sorted.
			//m_pInsList->Requery();
			//m_pInsList->SetSelByColumn(0, (long)nNewNum);
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pInsList->GetNewRow();
			pNewRow->Value[0] = (long)nNewNum;		// ID
			pNewRow->Value[1] = _bstr_t(strResult);		// Name
			pNewRow->Value[2] = "";							// Address 1
			NXDATALIST2Lib::IRowSettingsPtr pAddedRow = m_pInsList->AddRowSorted(pNewRow, NULL);
			m_pInsList->CurSel = pAddedRow;

			m_bAutoSetContact = FALSE;

			ReflectInsListSelection(m_pInsList->CurSel);

			// Set the appropriate focus
			GetDlgItem(IDC_INSURANCE_PLAN_BOX)->SetFocus();
			((CNxEdit *)GetDlgItem(IDC_INSURANCE_PLAN_BOX))->SetSel( 0,-1, FALSE );

			// CH 3/22
			g_boNewCompany = TRUE;

			int nNewContact = NewNumber("PersonT","ID");
			ExecuteSql("INSERT INTO PersonT (ID, UserID) VALUES (%li, %li)", nNewContact, GetCurrentUserID());
			ExecuteSql("INSERT INTO InsuranceContactsT (PersonID, InsuranceCoID, [Default]) VALUES (%li,%li,1)", nNewContact, nNewNum);
			m_bAutoSetContact = TRUE;
			m_pContactList->Requery();

			ExecuteSql("INSERT INTO InsurancePlansT (ID, PlanName, PlanType, InsCoID) VALUES (%li,'%s', 'Other', %li);", NewNumber("InsurancePlansT", "ID"), _Q(strResult), nNewNum);
			m_pInsPlans->Requery();

			// Network change event
			CClient::RefreshTable(NetUtils::InsuranceCoT);
			CClient::RefreshTable(NetUtils::InsuranceContactsT);
			CClient::RefreshTable(NetUtils::InsurancePlansT);

			m_bIsNewCo = true;
			
			// The current insurance company ID has to be the ID of the new insurance company
			// (c.haag 2010-05-06 12:29) - PLID 37525 - We don't need m_nCurInsCoID. If anything, a disparity
			// between that value and the list's current selection can spell trouble.
			//ASSERT(m_nCurInsCoID = nNewNum);
		}

	// (a.walling 2010-09-08 13:26) - PLID 40377 - Use CSqlTransaction
	}NxCatchAll("Error in OnAddCompany()");	
	
}

void CEditInsInfoDlg::OnDeleteCompany() 
{
	if(!CheckCurrentUserPermissions(bioInsuranceCo,sptDelete))
		return;

	try {
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			AfxMessageBox("Please select an insurance company.");
			return;
		}

		// Get the id of the item to be deleted
		_variant_t tmpVar;
		tmpVar = pCurInsRow->GetValue(0);

		if (tmpVar.vt == VT_EMPTY || tmpVar.vt == VT_NULL) {
			MsgBox(RCS(IDS_INS_NO_DEL_GEN));
			return;
		}

		_RecordsetPtr rs;//don't delete used InsCo
		rs = CreateParamRecordset("SELECT Count(InsuranceCoID) AS InsCount FROM InsuredPartyT "
			"WHERE InsuranceCoID = {INT}", VarLong(tmpVar));

		// (s.tullis 2014-12-12 12:07) - PLID 64440 
		if (ReturnsRecordsParam("Select InsuranceCoID FROM ScheduleMixRuleInsuranceCosT WHERE InsuranceCoID  = {INT} ", VarLong(tmpVar)))
		{
			MsgBox("This Insurance is used on one or more Scheduling Mix Rule Templates.  Please delete or re-assign these templates before deleting.");
			return;
		}
		
		if (rs->Fields->GetItem("InsCount")->Value.lVal != 0)
		{	MsgBox(RCS(IDS_INS_NO_DEL_PARTY));
			rs->Close();
			return;
		}
		rs->Close();

		

		// (j.jones 2010-10-20 09:21) - PLID 27897 - disallow deleting if used on a batch payment
		if(ReturnsRecordsParam("SELECT TOP 1 ID FROM BatchPaymentsT WHERE InsuranceCoID = {INT}", VarLong(tmpVar))) {
			MsgBox("You cannot delete this insurance company from your database because it has been used on batch payments.");
			return;
		}
		
		//for audit
		CString strOld;
		strOld = CString(pCurInsRow->GetValue(1).bstrVal);

		// If we made it to here, we are ready 
		// so get confirmation of delete from user
		CString msg, name;
		name = VarString(pCurInsRow->GetValue(1), "");
		msg.Format(GetStringOfResource(IDS_INS_DEL_CONFIRM), name);
		int result = AfxMessageBox(msg,	MB_ICONQUESTION|MB_YESNO);
		if (result == IDYES) {
			// If the user said yes, then delete it
			tmpVar = pCurInsRow->GetValue(0);
			long ID = VarLong(tmpVar);

			// (d.thompson 2009-03-20) - PLID 33061 - There is now a preference for default ins co we may need to clear as well.
			//	I parameterized these queries while I was at it.  We just wipe the configrt preference altogether so it goes back
			//	to its default.
			//I removed the explicit transaction since this is now all 1 SQL batch, thus it's a transaction of its own
			CString strSqlBatch;
			CNxParamSqlArray args;

			//Gather this first
			AddStatementToSqlBatch(strSqlBatch, "SELECT IntParam FROM ConfigRT WHERE Name = 'DefaultInsuranceCoNewPatient'");
			AddDeclarationToSqlBatch(strSqlBatch, "SET NOCOUNT ON;\r\n");

			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @InsCoID int;\r\n");
			AddParamStatementToSqlBatch(strSqlBatch, args, "SET @InsCoID = {INT};\r\n", ID);

			//Old behavior, parameterized
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM Tops_InsuranceLinkT WHERE PracInsuranceID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM GroupDetailsT WHERE PersonID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM MultiFeeInsuranceT WHERE InsuranceCoID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceGroups WHERE InsCoID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceBox24J WHERE InsCoID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceNetworkID WHERE InsCoID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceBox51 WHERE InsCoID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceBox31 WHERE InsCoID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceFacilityID WHERE InsCoID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceAcceptedT WHERE InsuranceCoID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsurancePlansT WHERE InsCoID = @InsCoID;\r\n");

			// (j.jones 2009-08-05 08:42) - PLID 34467 - delete from InsuranceLocationPayerIDsT
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceLocationPayerIDsT WHERE InsuranceCoID = %li", ID);

			//There's no reason to select and loop here, we can do it with a temp table and 2 deletes
			/*rs = CreateRecordset("SELECT PersonID FROM InsuranceContactsT WHERE InsuranceCoID = %li",ID);
			while(!rs->eof) {
				long PersonID = rs->Fields->Item["PersonID"]->Value.lVal;
				ExecuteSql("DELETE FROM InsuranceContactsT WHERE PersonID = %li",PersonID);
				ExecuteSql("DELETE FROM PersonT WHERE ID = %li",PersonID);
				rs->MoveNext();
			}
			rs->Close();*/
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @tmpContactTable table (ID int NOT NULL);\r\n");
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO @tmpContactTable (ID) (SELECT PersonID FROM InsuranceContactsT WHERE InsuranceCoID = @InsCoID);\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceContactsT WHERE PersonID IN (SELECT ID FROM @tmpContactTable);\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM PersonT WHERE ID IN (SELECT ID FROM @tmpContactTable);\r\n");

			// (j.jones 2013-07-18 09:25) - PLID 41823 - delete from our secondary exclusion tables
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM HCFASecondaryExclusionsT WHERE InsuranceCoID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM UBSecondaryExclusionsT WHERE InsuranceCoID = @InsCoID;\r\n");

			// (j.jones 2010-08-03 08:58) - PLID 39912 - clear out InsCoServicePayGroupLinkT
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsCoServicePayGroupLinkT WHERE InsuranceCoID = @InsCoID;\r\n");
			//TES 7/23/2010 - PLID 39320 - Clear out any linked HL7 codes
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM HL7CodeLinkT WHERE PracticeID = @InsCoID AND Type = %i;\r\n", hclrtInsCo);
			// (j.jones 2008-08-01 10:27) - PLID 30917 - ensure we delete from HCFAClaimProvidersT
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM HCFAClaimProvidersT WHERE InsuranceCoID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CPTInsNotesT WHERE InsuranceCoID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM ServiceRevCodesT WHERE InsuranceCompanyID = @InsCoID;\r\n");
			// (j.jones 2011-04-05 15:14) - PLID 42372 - remove CLIA setup
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM CLIANumbersT WHERE InsuranceCoID = @InsCoID;\r\n");
			//(e.lally 2011-05-03) PLID 43481 - remove NexWeb display setup
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM NexWebDisplayT WHERE InsuranceCoID = @InsCoID;\r\n");
			// (j.camacho 2013-06-28 11:40) - PLID 51069 - Update the diagnosis from the insurance group to the new insurance
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM diaginsnotest WHERE InsuranceCoID = @InsCoID");
			// (r.farnworth 2013-11-26 14:11) - PLID 59520 - Need to check for NexWebCCDAAccess before deleting MailSent items.
			AddStatementToSqlBatch(strSqlBatch, "DELETE E FROM NexWebCcdaAccessHistoryT E INNER JOIN MailSent W ON E.MailSentMailID = W.MailID WHERE W.PersonID = @InsCoId");
			// (s.tullis 2016-02-19 16:21) - PLID 68318
			AddStatementToSqlBatch(strSqlBatch, " Delete FROM ClaimFormLocationInsuranceSetupT WHERE InsuranceID = @InsCoId ");
			// (j.camacho 2013-06-28 12:06) - PLID 51069 - Update the mail sent table to show the new insurance company
			//TES 12/12/2014 - PLID 64435 - Also need to delete from MailSentNotesT
			AddStatementToSqlBatch(strSqlBatch, "DELETE N FROM MailSentNotesT N INNER JOIN MailSent M ON N.MailID = M.MailID WHERE M.PersonID = @InsCoId");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM mailsent WHERE personid = @InsCoId");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM InsuranceCoT WHERE PersonID = @InsCoID;\r\n");
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM PersonT WHERE ID = @InsCoID;\r\n");

			//Do the delete
			// (e.lally 2009-06-21) PLID 34679 - Fixed to use create recordset function.
			_RecordsetPtr prsResults = CreateParamRecordsetBatch(GetRemoteData(), strSqlBatch, args);
			if(!prsResults->eof) {
				long nRetVal = AdoFldLong(prsResults, "IntParam");
				if(nRetVal == ID) {
					//The configRT setting matches this ins co, so wipe it.  We do it here because the caching must
					//	be updated.  We also wipe the plan too, as it is ins co - dependant
					SetRemotePropertyInt("DefaultInsuranceCoNewPatient", -1, 0, "<None>");
					SetRemotePropertyInt("DefaultInsurancePlanNewPatient", -1, 0, "<None>");
				}
			}
			else {
				//It may be empty if the preference was never used
			}
			
			
			m_pInsList->RemoveRow(pCurInsRow);	

			// Network change event
			CClient::RefreshTable(NetUtils::InsuranceCoT);
			CClient::RefreshTable(NetUtils::InsuranceContactsT);
			CClient::RefreshTable(NetUtils::InsurancePlansT);

			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, strOld, nAuditID, aeiInsCoDelete, -1, "", "<Deleted>", aepHigh, aetDeleted);

			// And requery the screen
			m_pInsList->CurSel = m_pInsList->GetFirstRow();
			ReflectInsListSelection(m_pInsList->CurSel);
		}
	}NxCatchAll("Error in OnClickDeleteBttn()");	
}

void CEditInsInfoDlg::OnBtnOk() 
{
	// Warn the user if anything is out of whack
	if(!CheckOnExitingCurrentInsCo(ecicaClosing)){
		return;
	}

	StoreBox(IDC_ENVOY);
	// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
	//StoreBox(IDC_THIN);
	StoreBox(IDC_ELIGIBILITY_PAYER);
	// (j.jones 2009-12-16 15:00) - PLID 36237 - added UB payerID
	StoreBox(IDC_UB_PAYER_IDS);
	CDialog::OnOK();
}

// (j.jones 2009-08-04 11:29) - PLID 14573 - removed an inaccurate comment about EBillingInsCoIDs

void CEditInsInfoDlg::OnBtnGroups() 
{
	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	if(NULL == pCurInsRow) {
		return;
	}

	CInsuranceGroupsDlg dlg(this);
	dlg.m_strInsCo = CString(pCurInsRow->GetValue(1).bstrVal);
	dlg.m_iInsuranceCoID = pCurInsRow->GetValue(0).lVal;
	dlg.DoModal();
}

void CEditInsInfoDlg::OnBtnBox24J() 
{
	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	if(NULL == pCurInsRow) {
		return;
	}

	CEditBox24J dlg(this);
	dlg.m_strInsCo = CString(pCurInsRow->GetValue(1).bstrVal);
	dlg.m_iInsuranceCoID = pCurInsRow->GetValue(0).lVal;
	dlg.DoModal();	
}

void CEditInsInfoDlg::OnEditNetworkid() 
{
	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	if(NULL == pCurInsRow) {
		return;
	}

	CEditNetworkID dlg(this);
	dlg.m_strInsCo = CString(pCurInsRow->GetValue(1).bstrVal);
	dlg.m_iInsuranceCoID = pCurInsRow->GetValue(0).lVal;
	dlg.DoModal();
}

void CEditInsInfoDlg::OnHmoCheck() 
{
	StoreBox(IDC_HMO_CHECK);
}

void CEditInsInfoDlg::OnSelChosenEnvoy(long nRow) 
{
	StoreBox (IDC_ENVOY);
}

// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
/*
void CEditInsInfoDlg::OnSelChosenThin(long nRow) 
{
	StoreBox(IDC_THIN);
}
*/

void CEditInsInfoDlg::OnSelChosenEligibility(long nRow) 
{
	StoreBox(IDC_ELIGIBILITY_PAYER);
}

bool CEditInsInfoDlg::SaveAreaCode(long nID) {

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

void CEditInsInfoDlg::FillAreaCode(long nID)  {

	
		//first check to see if anything is in this box
		CString strPhone;
		GetDlgItemText(nID, strPhone);
		if (! ContainsDigit(strPhone)) {
			// (j.gruber 2009-10-07 16:13) - PLID 35825 - use city info if looking up by city
			CString strAreaCode, strZip, strCity;
			BOOL bResult = FALSE;
			GetDlgItemText(IDC_CONTACT_ZIP_BOX, strZip);
			GetDlgItemText(IDC_CONTACT_CITY_BOX, strCity);
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

void CEditInsInfoDlg::OnSelChosenUb92Setup(long nRow) 
{
	if(nRow == -1)
		return;

	try{
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}

		//for auditing
		CString strOld;
		_RecordsetPtr rs = CreateRecordset("SELECT UB92SetupT.Name FROM UB92SetupT INNER JOIN InsuranceCoT ON UB92SetupT.ID = InsuranceCoT.UB92SetupGroupID WHERE InsuranceCoT.PersonID = %li", VarLong(pCurInsRow->GetValue(0)));
		if(!rs->eof && rs->Fields->Item["Name"]->Value.vt == VT_BSTR)
			strOld = CString(rs->Fields->Item["Name"]->Value.bstrVal);
		//

		ExecuteSql("UPDATE InsuranceCoT SET UB92SetupGroupID = %li WHERE PersonID = %li;", VarLong(m_pUB92Groups->GetValue(nRow,0), -1), VarLong(pCurInsRow->GetValue(0), -1));

		//auditing
		if(strOld != CString(m_pUB92Groups->GetValue(nRow, 1).bstrVal)) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoUB92, long(pCurInsRow->GetValue(0).lVal), strOld, CString(m_pUB92Groups->GetValue(nRow, 1).bstrVal), aepMedium, aetChanged);
		}

	}NxCatchAll("Error in OnSelChosenUB92Setup()");
}

void CEditInsInfoDlg::OnBtnBox51() 
{
	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	if(NULL == pCurInsRow) {
		return;
	}

	CEditBox51 dlg(this);
	dlg.m_strInsCo = CString(pCurInsRow->GetValue(1).bstrVal);
	dlg.m_iInsuranceCoID = pCurInsRow->GetValue(0).lVal;
	dlg.DoModal();	
}

void CEditInsInfoDlg::OnBtnFacilityId() 
{
	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	if(NULL == pCurInsRow) {
		return;
	}

	CEditFacilityID dlg(this);
	dlg.m_strInsCo = CString(pCurInsRow->GetValue(1).bstrVal);
	dlg.m_iInsuranceCoID = pCurInsRow->GetValue(0).lVal;
	dlg.DoModal();	
}

void CEditInsInfoDlg::OnSelChosenInsContactsList(long nRow) 
{
	if(nRow == -1)
		return;

	try {

		_RecordsetPtr rs = CreateRecordset("SELECT * FROM InsuranceContactsT INNER JOIN PersonT ON InsuranceContactsT.PersonID = PersonT.ID WHERE ID = %li",VarLong(m_pContactList->GetValue(m_pContactList->CurSel, iccID)));

		if(rs->eof)
			return;

		FieldsPtr Fields = rs->Fields;

		SetDlgItemText(IDC_CONTACT_FIRST_BOX, VarString(Fields->GetItem("First")->Value, ""));
			
		SetDlgItemText(IDC_CONTACT_LAST_BOX, VarString(Fields->GetItem("Last")->Value, ""));
			
		SetDlgItemText(IDC_CONTACT_TITLE_BOX, VarString(Fields->GetItem("Title")->Value, ""));
			
		if(m_bFormatPhoneNums) {
			FormatItemText(GetDlgItem(IDC_CONTACT_PHONE_BOX), VarString(Fields->GetItem("WorkPhone")->Value, ""), m_strPhoneFormat);
		}
		else {
			SetDlgItemText(IDC_CONTACT_PHONE_BOX,VarString(Fields->GetItem("WorkPhone")->Value, ""));
		}
		
		SetDlgItemText(IDC_CONTACT_EXT_BOX, VarString(Fields->GetItem("Extension")->Value, ""));
			
		if(m_bFormatPhoneNums) {
			FormatItemText(GetDlgItem(IDC_CONTACT_FAX_BOX), VarString(Fields->GetItem("Fax")->Value, ""), m_strPhoneFormat);
		}
		else {
			SetDlgItemText(IDC_CONTACT_FAX_BOX,VarString(Fields->GetItem("Fax")->Value, ""));
		}

		m_checkMakeDefaultContact.SetCheck(VarBool(Fields->GetItem("Default")->Value,FALSE));

		rs->Close();

	}NxCatchAll("Error loading contact information.");
}

void CEditInsInfoDlg::OnAddContact() 
{
	try {
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}
			
		CString strResult;

		int nResult = InputBoxLimited(this, "Enter a last name for this contact:", strResult, "",50,false,false,NULL);

		strResult.TrimRight();
		
		if (nResult == IDOK)
		{
			//the name can be blank, the prompt was just to get them started

			if(strResult.GetLength() > 0) {
				CString firstchar = strResult.GetAt(0);
				firstchar.MakeUpper();
				strResult.SetAt(0,firstchar.GetAt(0));
			}

			int nNewNum = NewNumber("PersonT","ID");
			ExecuteSql("INSERT INTO PersonT (ID, Last, UserID) VALUES (%li, '%s', %li)", nNewNum, _Q(strResult), GetCurrentUserID());
			ExecuteSql("INSERT INTO InsuranceContactsT (PersonID, InsuranceCoID, [Default]) VALUES (%li,%li,0)", nNewNum, VarLong(pCurInsRow->GetValue(0)), 0);

			// Requery the screen
			m_bAutoSetContact = FALSE;
			m_pContactList->Requery();
			int row = m_pContactList->SetSelByColumn(iccID, (long)nNewNum);

			OnSelChosenInsContactsList(row);

			m_bAutoSetContact = TRUE;

			// Set the appropriate focus
			GetDlgItem(IDC_CONTACT_FIRST_BOX)->SetFocus();
			((CNxEdit *)GetDlgItem(IDC_CONTACT_FIRST_BOX))->SetSel( 0,-1, FALSE );

			// Network change event
			CClient::RefreshTable(NetUtils::InsuranceContactsT);
		}

	}NxCatchAll("Error adding new contact.");
}

void CEditInsInfoDlg::OnDeleteContact() 
{
	try {
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}

		if(m_pContactList->CurSel == -1)
			return;

		long ID = VarLong(m_pContactList->GetValue(m_pContactList->CurSel, iccID));

		if(IsRecordsetEmpty("SELECT PersonID FROM InsuranceContactsT WHERE InsuranceCoID = %li AND PersonID <> %li",VarLong(pCurInsRow->GetValue(0)),ID)) {
			AfxMessageBox("There must be at least one contact available per insurance company. Please change the details of this contact rather than deleting it.");
			return;
		}

		if(!IsRecordsetEmpty("SELECT PersonID FROM InsuredPartyT WHERE InsuranceContactID = %li",ID)) {
			AfxMessageBox("There are insured parties currently using this contact, please correct their accounts before deleting.");
			return;
		}

		if(MessageBox("This will permanently delete this contact. Are you sure you wish to do this?","Practice",MB_ICONEXCLAMATION|MB_YESNO) == IDNO)
			return;

		ExecuteSql("DELETE FROM InsuranceContactsT WHERE PersonID = %li",ID);
		ExecuteSql("DELETE FROM PersonT WHERE ID = %li",ID);

		//for auditing
		CString strOld = VarString(m_pContactList->GetValue(m_pContactList->CurSel, iccFullName),"");

		m_pContactList->RemoveRow(m_pContactList->GetCurSel());

		//audit the deletion
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoContactDelete, ID, strOld, "<Deleted>", aepMedium, aetDeleted);

		//send a network message
		CClient::RefreshTable(NetUtils::InsuranceContactsT);

		// (z.manning 2009-01-09 09:46) - PLID 32663 - Update HL7 if necessary
		long nInsCoID = VarLong(pCurInsRow->GetValue(0));
		if(m_checkMakeDefaultContact.GetCheck() == BST_CHECKED) {
			UpdateExistingHL7PatientsByInsCoID(nInsCoID);
		}

		//now try to select the best contact
		_RecordsetPtr rs = CreateRecordset("SELECT PersonID FROM InsuranceContactsT WHERE InsuranceCoID = %li AND [Default] = 1", nInsCoID);

		if(rs->eof)
			m_pContactList->PutCurSel(0);
		else {
			long ID = rs->Fields->Item["PersonID"]->Value.lVal;
			//set the default contact
			m_pContactList->SetSelByColumn(iccID,(long)ID);
		}

		rs->Close();		

		OnSelChosenInsContactsList(m_pContactList->GetCurSel());

	}NxCatchAll("Error deleting contact.");	
}

void CEditInsInfoDlg::OnCheckMakeDefaultContact() 
{
	try {
		if(m_pContactList->CurSel == -1)
			return;

		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}

		BOOL bMakeDefault = FALSE;

		if(m_checkMakeDefaultContact.GetCheck())
			bMakeDefault = TRUE;

		long ID = VarLong(m_pContactList->GetValue(m_pContactList->CurSel, iccID));
		long InsID = VarLong(pCurInsRow->GetValue(0));

		//for auditing
		CString strOld = "<None>";
		// (j.jones 2012-10-19 10:03) - PLID 51551 - we now use <No Contact Name Entered>, instead of just a comma, if the name is blank
		_RecordsetPtr rs = CreateParamRecordset("SELECT CASE WHEN Coalesce(Last, '') = '' AND Coalesce(First, '') = '' THEN '<No Contact Name Entered>' "
			"ELSE Last + CASE WHEN Coalesce(Last, '') <> '' AND Coalesce(First, '') <> '' THEN ', ' ELSE '' END + First END AS FullName "
			"FROM PersonT "
			"INNER JOIN InsuranceContactsT ON PersonT.ID = InsuranceContactsT.PersonID "
			"WHERE InsuranceCoID = {INT} AND [Default] = 1", InsID);

		if(!rs->eof) {
			strOld = AdoFldString(rs, "FullName","<None>");
		}
		rs->Close();

		if(bMakeDefault) {
			//clear the rest
			ExecuteSql("UPDATE InsuranceContactsT SET [Default] = 0 WHERE InsuranceCoID = %li",InsID);
			//now set ours
			ExecuteSql("UPDATE InsuranceContactsT SET [Default] = 1 WHERE PersonID = %li",ID);
		}
		else
			ExecuteSql("UPDATE InsuranceContactsT SET [Default] = 0 WHERE PersonID = %li",ID);
		
		CString strNew = VarString(m_pContactList->GetValue(m_pContactList->CurSel, iccFullName),"");

		//audit the change
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, CString(pCurInsRow->GetValue(1).bstrVal), nAuditID, aeiInsCoDefaultContact, ID, strOld, strNew, aepMedium, aetChanged);
		
		// (z.manning 2009-01-08 17:48) - PLID 32663 - Update HL7 patients
		// (b.eyers 2015-11-12) - PLID 66977 - the default ins co contact doesn't get sent now on IN1
		//UpdateExistingHL7PatientsByInsCoID(VarLong(pCurInsRow->GetValue(0), -1));

	}NxCatchAll("Error designating default contact.");	
}

void CEditInsInfoDlg::OnManageContacts() 
{
	if(!CheckCurrentUserPermissions(bioInsuranceCo,sptDelete))
		return;

		try{
			DontShowMeAgain(this, "Be careful when using this utility.\n"
			"You can change large amounts of data at once,\n"
			"and your changes cannot be undone", 
			"ManageInsContacts");

			CListMergeDlg dlg(this);
			//(e.lally 2011-07-07) PLID 31585 - We now support multiple list types, so we need to set which one is being used
			dlg.m_eListType = mltInsuranceCompanies;
			dlg.DoModal();
		
			if(dlg.m_bCombined) {
				//requery the screen
				m_pInsList->Requery();
				//TES 7/23/2010 - PLID 39804 - We need to let the requery finish, otherwise there won't be a first row yet.
				m_pInsList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
				m_pInsList->CurSel = m_pInsList->GetFirstRow();
				ReflectInsListSelection(m_pInsList->CurSel);
			}
		}NxCatchAll("Error in CEditInsInfoDlg::OnManageContacts()");
}

void CEditInsInfoDlg::OnRequeryFinishedInsContactsList(short nFlags) 
{
	try {
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}

		if(m_bAutoSetContact && m_pContactList->GetCurSel() == -1) {

			//first try to set the stored contact
			if(m_nContactID != -1) {
				if(m_pContactList->SetSelByColumn(iccID,(long)m_nContactID) != -1) {
					OnSelChosenInsContactsList(m_pContactList->GetCurSel());
					//reset the stored contact
					m_nContactID = -1;
					return;
				}
			}

			_RecordsetPtr rs = CreateRecordset("SELECT PersonID FROM InsuranceContactsT WHERE InsuranceCoID = %li AND [Default] = 1",VarLong(pCurInsRow->GetValue(0)));

			if(rs->eof)
				m_pContactList->PutCurSel(0);
			else {
				long ID = rs->Fields->Item["PersonID"]->Value.lVal;
				//set the default contact
				m_pContactList->SetSelByColumn(0,(long)ID);
			}

			rs->Close();		

			OnSelChosenInsContactsList(m_pContactList->GetCurSel());
		}
	
	}NxCatchAll("Error loading default contact.");
}

void CEditInsInfoDlg::OnBtnAccepted() 
{
	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	if(NULL == pCurInsRow) {
		return;
	}

	CEditInsAcceptedDlg dlg(this);
	dlg.m_strInsCo = CString(pCurInsRow->GetValue(1).bstrVal);
	dlg.m_iInsuranceCoID = pCurInsRow->GetValue(0).lVal;
	dlg.DoModal();
}

void CEditInsInfoDlg::OnRadioTaxIns() 
{
	StoreBox(IDC_RADIO_TAX_INS);
}

void CEditInsInfoDlg::OnRadioTaxPatient() 
{
	StoreBox(IDC_RADIO_TAX_PATIENT);
}

void CEditInsInfoDlg::OnRadioTaxNone() 
{
	StoreBox(IDC_RADIO_TAX_NONE);
}

void CEditInsInfoDlg::OnCheckUseReferrals() 
{
	StoreBox(IDC_CHECK_USE_REFERRALS);
}

bool CEditInsInfoDlg::CheckUnusualInsuranceInfo(IN const EExitingCurInsCoAction eecica)
{
	CString strWarnings;
	
	//first see if they have HCFA as part of their license and if they've selected a HCFA group yet
	if(g_pLicense->CheckForLicense(CLicense::lcHCFA, CLicense::cflrUse) && m_pHCFAGroups->GetCurSel() == (long)-1)
	{
		strWarnings = "- You have not chosen a HCFA group for this insurance company.\r\n";
	}

	//get the current Ebilling settings
	// (j.jones 2008-12-02 09:00) - PLID 32291 - changed the default to be ANSI (3)
	int FormatType = GetRemotePropertyInt("EbillingFormatType",3,0,"<None>",TRUE);
	
	//see if they have an EBilling license
	if(g_pLicense->CheckForLicense(CLicense::lcEbill, CLicense::cflrUse)){
		//they have ebilling, if the format type isn't image (FormatType == 2) they will need a PayerID or THIN ID
		if(FormatType != 2){
			// (j.jones 2009-08-04 11:34) - PLID 14573 - removed THIN payer ID
			if(m_pEnvoyList->GetCurSel() == -1 || CString(m_pEnvoyList->GetValue(m_pEnvoyList->GetCurSel(),hpicCode).bstrVal) == ""){
				//they need a Payer ID
				strWarnings += "- No Payer ID has been selected.  You will need a payer ID if you plan to send claims electronically to this insurance company.\r\n";
			}
		}
	}

	// (j.jones 2010-07-19 10:11) - PLID 39702 - warn if they do not have an insurance type selected
	// (this is mainly used in E-Billing but some other code such as HL7 and AHCA exports also depend on it)
	InsuranceTypeCode eInsType = itcInvalid;
	if(m_pInsuranceTypeList->CurSel != -1) {
		eInsType = (InsuranceTypeCode)VarLong(m_pInsuranceTypeList->GetValue(m_pInsuranceTypeList->CurSel, 0), (long)itcInvalid);
	}
	if(eInsType == itcInvalid) {
		strWarnings += "- No Insurance Type has been selected.  You will need an Insurance Type if you plan to send claims electronically to this insurance company. Many export features also require this field to be filled in.\r\n";
	}

	// Delete the trailing "\r\n"
	if (strWarnings.Right(2) == "\r\n") {
		strWarnings.Delete(strWarnings.GetLength() - 2, 2);
	}

	//a message needs to be displayed if any problems have been found
	if(!strWarnings.IsEmpty()){
		// Get a nice description of the particular action being taken
		CString strAction;
		switch (eecica) {
		case ecicaClosing:
			strAction = "continue and close the insurance company editor";
			break;
		case ecicaChangingSel:
			strAction = "continue changing to a different insurance company";
			break;
		case ecicaCreatingInsCo:
			strAction = "continue and create a new insurance company";
			break;
		}

		// Format the message to make it presentable to the user
		CString strMsg;
		strMsg.Format(
			"The following warnings have been found:\r\n"
			"%s\r\n\r\n"
			"Are you sure you are finished editing the insurance company?\r\n\r\n"
			"Click 'Yes' to %s.\r\n"
			"Click 'No' to resume editing the previous insurance company.\r\n",
			strWarnings, strAction);
		
		//display the problems that were found and see if the user wants to make any changes to the new company
		if(IDYES != MessageBox(strMsg, "Resume editing?", MB_YESNO|MB_ICONQUESTION)){
			// The user clicked NO, so tell the caller he is not allowed to proceed
			return false;
		} else {
			// The user clicked yes, so we're good to go, tell the caller to proceed
			return true;
		}
	} else {
		// There were no warnings related to HCFA or EBilling info so the caller can proceed
		return true;
	}
}

bool CEditInsInfoDlg::CheckOnExitingCurrentInsCo(IN const EExitingCurInsCoAction eecica)
{
	
	if(m_bIsNewCo && !CheckUnusualInsuranceInfo(eecica))
	{
		// This check failed, caller is not allowed to proceed
		return false;
	}

	// All warnings passed, so caller can proceed
	return true;
}

void CEditInsInfoDlg::OnSelChosenInsTypeCombo(long nRow) 
{
	StoreBox (IDC_INS_TYPE_COMBO);
}

void CEditInsInfoDlg::OnWorkersCompCheck() 
{
	StoreBox(IDC_WORKERS_COMP_CHECK);
}

void CEditInsInfoDlg::OnInactiveCheck() 
{
	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	if(NULL == pCurInsRow) {
		return;
	}

	long nCheck;
	if(IsDlgButtonChecked(IDC_INACTIVE_CHECK)) {
		if(MsgBox(MB_YESNO, "You are marking this insurance company inactive.  The information will still be available for any patients which currently "
			"have this company selected, but will be unavailable for any new patients.\n "
			"Are you sure you wish to do this?") == IDNO) {
			CheckDlgButton(IDC_INACTIVE_CHECK, FALSE);
			return;
		}
		nCheck = 1;
	}
	else {
		if(MsgBox(MB_YESNO, "You are marking this insurance company as active.  This will make it available for use for any patient.\n"
			"Are you sure you wish to do this?") == IDNO) {
			CheckDlgButton(IDC_INACTIVE_CHECK, TRUE);
			return;
		}
		nCheck = 0;
	}

	try {
		ExecuteSql("UPDATE PersonT SET Archived = %li WHERE ID = %li", nCheck, VarLong(pCurInsRow->GetValue(0)));

		CClient::RefreshTable(NetUtils::InsuranceCoT);
	} NxCatchAll("Error setting inactive status");

}

void CEditInsInfoDlg::OnBtnEditCptNotes() 
{
	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	if(NULL == pCurInsRow) {
		return;
	}

	CCPTCodeInsNotesDlg dlg(this);
	if(pCurInsRow != NULL)
		dlg.m_nInsCoID = VarLong(pCurInsRow->GetValue(0), -1);
	dlg.DoModal();
}

void CEditInsInfoDlg::OnBtnBox31() 
{
	// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	if(NULL == pCurInsRow) {
		return;
	}

	CEditBox31 dlg(this);
	dlg.m_strInsCo = CString(pCurInsRow->GetValue(1).bstrVal);
	dlg.m_iInsuranceCoID = pCurInsRow->GetValue(0).lVal;
	dlg.DoModal();	
}

void CEditInsInfoDlg::OnLeftClickInsPlans(long nRow, short nCol, long x, long y, long nFlags) 
{
	// (m.cable 06/14/2004 10:22) - this is now handled in OnSelChangedInsPlans
	//RefreshPlanButtons();
}

void CEditInsInfoDlg::RefreshPlanButtons()
{
	if(m_pInsPlans->GetCurSel() == NXDATALISTLib::sriNoRow){
		GetDlgItem(IDC_DELETE_PLAN)->EnableWindow(FALSE);
	}
	else{
		GetDlgItem(IDC_DELETE_PLAN)->EnableWindow(TRUE);
	}
}

void CEditInsInfoDlg::OnSelChangedInsPlans(long nNewSel) 
{
	RefreshPlanButtons();	
}

// (j.jones 2008-07-10 16:26) - PLID 28756 - added button to edit payment categories
void CEditInsInfoDlg::OnBtnEditPayCats() 
{
	try {

		{
			long nCurSel = m_pDefPayCategoryCombo->GetCurSel();
			long nCurCatID = -1;
			// if there is something selected, then save it so we can set it back after editing the categories
			if(nCurSel != NXDATALISTLib::sriNoRow){
				nCurCatID = VarLong(m_pDefPayCategoryCombo->GetValue(nCurSel, pccID), -1);
			}
			else{
				nCurCatID = -1;
			}

			// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
			// (j.gruber 2012-11-15 13:50) - PLID 53752 - changed the dialog
			CPayCatDlg dlg(this);
			if (dlg.DoModal()) {
				m_pDefPayCategoryCombo->Requery();
				m_pDefPayCategoryCombo->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);
			}
			//CEditComboBox(this, 3, m_pDefPayCategoryCombo, "Edit Combo Box").DoModal();

			NXDATALISTLib::IRowSettingsPtr pRow;
			pRow = m_pDefPayCategoryCombo->GetRow(-1);
			pRow->PutValue(pccID, long(-1));
			pRow->PutValue(pccName, _bstr_t("<No Default Category>"));
			m_pDefPayCategoryCombo->AddRow(pRow);

			// set the selection back to what it was before
			m_pDefPayCategoryCombo->TrySetSelByColumn(pccID, nCurCatID);
		}

		//now do the same for adjustments
		{
			long nCurSel = m_pDefAdjCategoryCombo->GetCurSel();
			long nCurCatID = -1;
			// if there is something selected, then save it so we can set it back after editing the categories
			if(nCurSel != NXDATALISTLib::sriNoRow){
				nCurCatID = VarLong(m_pDefAdjCategoryCombo->GetValue(nCurSel, accID), -1);
			}
			else{
				nCurCatID = -1;
			}

			m_pDefAdjCategoryCombo->Requery();

			NXDATALISTLib::IRowSettingsPtr pRow;
			pRow = m_pDefAdjCategoryCombo->GetRow(-1);
			pRow->PutValue(accID, long(-1));
			pRow->PutValue(accName, _bstr_t("<No Default Category>"));
			m_pDefAdjCategoryCombo->AddRow(pRow);

			// set the selection back to what it was before
			m_pDefAdjCategoryCombo->TrySetSelByColumn(accID, nCurCatID);
		}

	}NxCatchAll("Error in CEditInsInfoDlg::OnBtnEditPayCats");
}

// (j.jones 2008-07-10 16:26) - PLID 28756 - added button to edit payment categories (but next to the adjustment list)
void CEditInsInfoDlg::OnBtnEditAdjCats() 
{
	try {

		{
			long nCurSel = m_pDefAdjCategoryCombo->GetCurSel();
			long nCurCatID = -1;
			// if there is something selected, then save it so we can set it back after editing the categories
			if(nCurSel != NXDATALISTLib::sriNoRow){
				nCurCatID = VarLong(m_pDefAdjCategoryCombo->GetValue(nCurSel, accID), -1);
			}
			else{
				nCurCatID = -1;
			}

			// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
			// (j.gruber 2012-11-15 13:50) - PLID 53752 - changed the dialog
			//CEditComboBox(this, 3, m_pDefAdjCategoryCombo, "Edit Combo Box").DoModal();
			CPayCatDlg dlg(this);
			if (dlg.DoModal()) {
				m_pDefAdjCategoryCombo->Requery();
				m_pDefAdjCategoryCombo->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);
			}

			NXDATALISTLib::IRowSettingsPtr pRow;
			pRow = m_pDefAdjCategoryCombo->GetRow(-1);
			pRow->PutValue(accID, long(-1));
			pRow->PutValue(accName, _bstr_t("<No Default Category>"));
			m_pDefAdjCategoryCombo->AddRow(pRow);

			// set the selection back to what it was before
			m_pDefAdjCategoryCombo->TrySetSelByColumn(accID, nCurCatID);
		}

		//now do the same for payments
		{
			long nCurSel = m_pDefPayCategoryCombo->GetCurSel();
			long nCurCatID = -1;
			// if there is something selected, then save it so we can set it back after editing the categories
			if(nCurSel != NXDATALISTLib::sriNoRow){
				nCurCatID = VarLong(m_pDefPayCategoryCombo->GetValue(nCurSel, pccID), -1);
			}
			else{
				nCurCatID = -1;
			}

			m_pDefPayCategoryCombo->Requery();

			NXDATALISTLib::IRowSettingsPtr pRow;
			pRow = m_pDefPayCategoryCombo->GetRow(-1);
			pRow->PutValue(pccID, long(-1));
			pRow->PutValue(pccName, _bstr_t("<No Default Category>"));
			m_pDefPayCategoryCombo->AddRow(pRow);

			// set the selection back to what it was before
			m_pDefPayCategoryCombo->TrySetSelByColumn(pccID, nCurCatID);
		}

	}NxCatchAll("Error in CEditInsInfoDlg::OnBtnEditAdjCats");
}

// (j.jones 2008-07-11 09:31) - PLID 30679 - added button to mass update payment/adjustment categories and descriptions
void CEditInsInfoDlg::OnBtnAdvInsDescSetup() 
{
	try {
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Don't use m_nCurInsCoID. If anything, a disparity
		// between that value and the list's current selection can spell trouble.
		long nCurInsCoID = VarLong(pCurInsRow->GetValue(0));

		CAdvInsurancePayDescriptionSetupDlg dlg(this);
		dlg.m_nCurInsCoID = nCurInsCoID;
		dlg.DoModal();

		//if this insurance company was modified, reload the default descriptions and categories
		if(dlg.m_bChangesMadeToCurrentIns) {

			_RecordsetPtr rs = CreateParamRecordset("SELECT DefaultPayDesc, DefaultPayCategoryID, "
				"DefaultAdjDesc, DefaultAdjCategoryID "
				"FROM InsuranceCoT "
				"WHERE PersonID = {INT}", nCurInsCoID);

			if(!rs->eof) {
				m_pDefPayCategoryCombo->SetSelByColumn(pccID, rs->Fields->Item["DefaultPayCategoryID"]->Value);
				SetDlgItemText(IDC_EDIT_DEF_PAY_DESCRIPTION, AdoFldString(rs, "DefaultPayDesc", ""));
				m_pDefAdjCategoryCombo->SetSelByColumn(accID, rs->Fields->Item["DefaultAdjCategoryID"]->Value);
				SetDlgItemText(IDC_EDIT_DEF_ADJ_DESCRIPTION, AdoFldString(rs, "DefaultAdjDesc", ""));
			}
			rs->Close();
		}

	}NxCatchAll("Error in CEditInsInfoDlg::OnBtnAdvInsDescSetup");
}

// (j.jones 2008-07-11 11:54) - PLID 28756 - added default payment category
void CEditInsInfoDlg::OnSelChosenComboDefPayCategory(long nRow) 
{
	try {

		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}

		// (c.haag 2010-05-06 12:29) - PLID 37525 - Don't use m_nCurInsCoID. If anything, a disparity
		// between that value and the list's current selection can spell trouble.
		long nCurInsCoID = VarLong(pCurInsRow->GetValue(0));

		//don't use the StoreBox logic, just save, audit, and send network code here

		long nNewCategoryID = -1;
		CString strNewCategoryName = "";

		if(nRow != -1) {
			nNewCategoryID = VarLong(m_pDefPayCategoryCombo->GetValue(nRow, pccID), -1);
			strNewCategoryName = VarString(m_pDefPayCategoryCombo->GetValue(nRow, pccName), "");
		}

		CString strNewCategoryID = "NULL";
		if(nNewCategoryID != -1) {
			strNewCategoryID.Format("%li", nNewCategoryID);
		}
		else {
			strNewCategoryName = "";
		}

		//now compare against the existing data
		_RecordsetPtr rs = CreateParamRecordset("SELECT DefaultPayCategoryID, PaymentGroupsT.GroupName "
			"FROM InsuranceCoT "
			"LEFT JOIN PaymentGroupsT ON InsuranceCoT.DefaultPayCategoryID = PaymentGroupsT.ID "
			"WHERE PersonID = {INT}", nCurInsCoID);

		if(!rs->eof) {
			
			long nOldCategoryID = AdoFldLong(rs, "DefaultPayCategoryID", -1);
			CString strOldCategoryName = AdoFldString(rs, "GroupName", "");

			//always update, following the same logic as the rest of this dialog
			ExecuteSql("UPDATE InsuranceCoT SET DefaultPayCategoryID = %s WHERE PersonID = %li", strNewCategoryID, nCurInsCoID);

			if(nNewCategoryID != nOldCategoryID) {

				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, VarString(pCurInsRow->GetValue(1), ""), nAuditID, aeiInsCoDefPaymentCategory, nCurInsCoID, strOldCategoryName, strNewCategoryName, aepMedium, aetChanged);

				CClient::RefreshTable(NetUtils::InsuranceCoT);
			}
		}
		rs->Close();

	}NxCatchAll("Error in CEditInsInfoDlg::OnSelChosenComboDefPayCategory");
}

// (j.jones 2008-07-11 11:54) - PLID 28756 - added default adjustment category
void CEditInsInfoDlg::OnSelChosenComboDefAdjCategory(long nRow) 
{
	try {

		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Don't use m_nCurInsCoID. If anything, a disparity
		// between that value and the list's current selection can spell trouble.
		long nCurInsCoID = VarLong(pCurInsRow->GetValue(0));

		//don't use the StoreBox logic, just save, audit, and send network code here

		long nNewCategoryID = -1;
		CString strNewCategoryName = "";

		if(nRow != -1) {
			nNewCategoryID = VarLong(m_pDefAdjCategoryCombo->GetValue(nRow, accID), -1);
			strNewCategoryName = VarString(m_pDefAdjCategoryCombo->GetValue(nRow, accName), "");
		}

		CString strNewCategoryID = "NULL";
		if(nNewCategoryID != -1) {
			strNewCategoryID.Format("%li", nNewCategoryID);
		}
		else {
			strNewCategoryName = "";
		}

		//now compare against the existing data
		_RecordsetPtr rs = CreateParamRecordset("SELECT DefaultAdjCategoryID, PaymentGroupsT.GroupName "
			"FROM InsuranceCoT "
			"LEFT JOIN PaymentGroupsT ON InsuranceCoT.DefaultAdjCategoryID = PaymentGroupsT.ID "
			"WHERE PersonID = {INT}", nCurInsCoID);

		if(!rs->eof) {
			
			long nOldCategoryID = AdoFldLong(rs, "DefaultAdjCategoryID", -1);
			CString strOldCategoryName = AdoFldString(rs, "GroupName", "");

			//always update, following the same logic as the rest of this dialog
			ExecuteSql("UPDATE InsuranceCoT SET DefaultAdjCategoryID = %s WHERE PersonID = %li", strNewCategoryID, nCurInsCoID);

			if(nNewCategoryID != nOldCategoryID) {

				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, VarString(pCurInsRow->GetValue(1), ""), nAuditID, aeiInsCoDefAdjustmentCategory, nCurInsCoID, strOldCategoryName, strNewCategoryName, aepMedium, aetChanged);

				CClient::RefreshTable(NetUtils::InsuranceCoT);
			}
		}
		rs->Close();

	}NxCatchAll("Error in CEditInsInfoDlg::OnSelChosenComboDefAdjCategory");
}


// (j.jones 2008-08-01 10:33) - PLID 30917 - colorize the Claim Provider Setup button when in use
void CEditInsInfoDlg::UpdateClaimProviderBtnAppearance()
{
	try {

		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Don't use m_nCurInsCoID. If anything, a disparity
		// between that value and the list's current selection can spell trouble.
		long nCurInsCoID = VarLong(pCurInsRow->GetValue(0));

		_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoID FROM HCFAClaimProvidersT "
			"WHERE (ANSI_2010AA_ProviderID Is Not Null OR ANSI_2310B_ProviderID Is Not Null OR "
			"Box33_ProviderID Is Not Null OR Box24J_ProviderID Is Not Null) "
			"AND InsuranceCoID = {INT}", nCurInsCoID);
		if(!rs->eof) {
			SetDlgItemText(IDC_BTN_CLAIM_PROVIDER_SETUP, "Claim Provider Setup (In Use)");
			m_btnClaimProviderSetup.SetTextColor(RGB(255,0,0));
			m_btnClaimProviderSetup.Invalidate();
		}
		else {
			SetDlgItemText(IDC_BTN_CLAIM_PROVIDER_SETUP, "Claim Provider Setup");
			m_btnClaimProviderSetup.AutoSet(NXB_MODIFY);
			m_btnClaimProviderSetup.Invalidate();
		}				
	}
	NxCatchAll("Error in CEditInsInfoDlg::UpdateClaimProviderBtnAppearance()");
}

// (j.jones 2008-08-01 10:35) - PLID 30917 - added OnBtnClaimProviderSetup
void CEditInsInfoDlg::OnBtnClaimProviderSetup() 
{
	try {

		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Don't use m_nCurInsCoID. If anything, a disparity
		// between that value and the list's current selection can spell trouble.
		long nCurInsCoID = VarLong(pCurInsRow->GetValue(0));

		if(nCurInsCoID == -1) {
			return;
		}

		CHCFAClaimProviderSetupDlg dlg(this);
		dlg.m_nInsCoID = nCurInsCoID;
		dlg.m_nColor = m_nColor;
		dlg.DoModal();

		//now colorize the Claim Provider Setup button to reflect any changes we may have made
		UpdateClaimProviderBtnAppearance();

	}NxCatchAll("Error in CEditInsInfoDlg::OnBtnClaimProviderSetup()");
}

// (j.jones 2008-09-09 08:47) - PLID 18695 - this function will add a new row to m_pInsuranceTypeList
void CEditInsInfoDlg::AddNewRowToInsuranceTypeList(InsuranceTypeCode eCode, BOOL bColorize /*= FALSE*/)
{
	try {

		//call global functions for consistency
		CString strName = GetNameFromInsuranceType(eCode);
		// (j.jones 2010-10-15 14:47) - PLID 40953 - show the 5010 code
		CString strANSI = GetANSI5010_SBR09CodeFromInsuranceType(eCode);
		CString strNSF = GetNSFCodeFromInsuranceType(eCode);

		NXDATALISTLib::IRowSettingsPtr pRow = m_pInsuranceTypeList->GetRow(-1);
		pRow->PutValue(itcID, (long)eCode);
		pRow->PutValue(itcName, _bstr_t(strName));
		pRow->PutValue(itcANSI, _bstr_t(strANSI));
		pRow->PutValue(itcNSF, _bstr_t(strNSF));
		
		if(bColorize) {
			pRow->PutForeColor(RGB(0,0,255));
		}

		m_pInsuranceTypeList->AddRow(pRow);

	}NxCatchAll("Error in CEditInsInfoDlg::AddNewRowToInsuranceTypeList");
}

// (c.haag 2009-03-17 11:21) - PLID 9148 - This function will invoke the financial
// class dialog. This allows users to bulk-assign companies to financial classes.
void CEditInsInfoDlg::OnBnClickedBtnEditFinancialClass()
{
	try {
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}
		CFinancialClassDlg dlg(this);
		dlg.SetStartupColor(m_nColor);
		dlg.DoModal();
		// Update the financial class combo
		m_pFinancialClassCombo->Requery();
		// Update the current selection
		_RecordsetPtr rs = CreateParamRecordset("SELECT FinancialClassID FROM InsuranceCoT WHERE InsuranceCoT.PersonID = {INT}", VarLong(pCurInsRow->GetValue(0)));
		if (!rs->eof) {
			FieldsPtr Fields = rs->Fields;
			m_pFinancialClassCombo->SetSelByColumn(0,Fields->GetItem("FinancialClassID")->Value);
		} else {
			_variant_t vNull;
			vNull.vt = VT_NULL;
			m_pFinancialClassCombo->SetSelByColumn(0,vNull);
		}
	}
	NxCatchAll("Error in CEditInsInfoDlg::OnBnClickedBtnEditFinancialClass");
}

// (j.jones 2009-08-04 12:56) - PLID 34467 - added ability to configure payer IDs per location
void CEditInsInfoDlg::OnBtnConfigureLocationPayerIds()
{
	try {

		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}

		_variant_t var1, var2, var3;

		//save the selections, incase the user edits the lists inside this screen
		if(m_pEnvoyList->GetCurSel()!=-1) {
			var1 = m_pEnvoyList->GetValue(m_pEnvoyList->GetCurSel(), hpicID);
		}
		if(m_pEligibilityPayerIDList->GetCurSel()!=-1) {
			var2 = m_pEligibilityPayerIDList->GetValue(m_pEligibilityPayerIDList->GetCurSel(), epicID);
		}
		// (j.jones 2009-12-16 15:00) - PLID 36237 - added UB payer ID
		if(m_pUBPayerList->GetCurSel()!=-1) {
			var3 = m_pUBPayerList->GetValue(m_pUBPayerList->GetCurSel(), ubpicID);
		}

		CConfigPayerIDsPerLocationDlg dlg(this);
		dlg.m_nInsuranceCoID = VarLong(pCurInsRow->GetValue(0));
		dlg.m_strInsuranceCoName = VarString(pCurInsRow->GetValue(1), "");
		dlg.m_nColor = m_nColor;
		dlg.DoModal();

		//requery all lists
		m_pEnvoyList->Requery();
		m_pEligibilityPayerIDList->Requery();
		m_pUBPayerList->Requery();

		// (j.jones 2009-12-17 09:34) - PLID 36237 - none of these ever re-added the "none" row, now they do
		NXDATALISTLib::IRowSettingsPtr pRow;
		_variant_t var;
		var.vt = VT_NULL;
		pRow = m_pEnvoyList->GetRow(-1);
		pRow->PutValue(hpicID,(long)-1);
		pRow->PutValue(hpicCode,(_bstr_t)"");
		pRow->PutValue(hpicName,(_bstr_t)"<No Payer ID Selected>");
		m_pEnvoyList->AddRow(pRow);

		pRow = m_pEligibilityPayerIDList->GetRow(-1);
		pRow->PutValue(epicID,(long)-1);
		pRow->PutValue(epicCode,(_bstr_t)"");
		pRow->PutValue(epicName,(_bstr_t)"<No Eligibility ID Selected>");
		m_pEligibilityPayerIDList->AddRow(pRow);

		pRow = m_pUBPayerList->GetRow(-1);
		pRow->PutValue(ubpicID,(long)-1);
		pRow->PutValue(ubpicCode,(_bstr_t)"");
		pRow->PutValue(ubpicName,(_bstr_t)"<No UB ID Selected>");
		m_pUBPayerList->AddRow(pRow);

		if(var1.vt == VT_I4) {
			m_pEnvoyList->SetSelByColumn(hpicID, var1);
		}

		if(var2.vt == VT_I4) {
			m_pEligibilityPayerIDList->SetSelByColumn(epicID, var2);
		}

		if(var3.vt == VT_I4) {
			m_pUBPayerList->SetSelByColumn(ubpicID, var3);
		}

		UpdateView();

	}NxCatchAll("Error in CEditInsInfoDlg::OnBtnConfigureLocationPayerIds");
}

// (j.jones 2009-08-04 17:33) - PLID 34467 - colorize the payer ID by location button when in use
void CEditInsInfoDlg::UpdateConfigLocationPayerIDBtnAppearance()
{
	try {
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2
		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
		if(NULL == pCurInsRow) {
			return;
		}
		// (c.haag 2010-05-06 12:29) - PLID 37525 - Don't use m_nCurInsCoID. If anything, a disparity
		// between that value and the list's current selection can spell trouble.
		long nCurInsCoID = VarLong(pCurInsRow->GetValue(0));

		//if an ID is in use for any location, color the button red
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ClaimPayerID, EligibilityPayerID, UBPayerID "
			"FROM InsuranceLocationPayerIDsT "
			"WHERE InsuranceCoID = {INT} AND (ClaimPayerID Is Not Null OR EligibilityPayerID Is Not Null)", nCurInsCoID);
		if(!rs->eof) {
			m_btnConfigLocationPayerIDs.SetTextColor(RGB(255,0,0));
			m_btnConfigLocationPayerIDs.Invalidate();
		}
		else {
			m_btnConfigLocationPayerIDs.AutoSet(NXB_MODIFY);
			m_btnConfigLocationPayerIDs.Invalidate();
		}
		rs->Close();

	}NxCatchAll("Error in CEditInsInfoDlg::UpdateConfigLocationPayerIDBtnAppearance");
}

// (j.jones 2009-12-16 15:00) - PLID 36237 - added UB payer ID
void CEditInsInfoDlg::OnSelChosenUbPayerIds(long nRow)
{
	try {

		StoreBox(IDC_UB_PAYER_IDS);

	}NxCatchAll("Error in CEditInsInfoDlg::OnSelChosenUbPayerIds");
}

// (c.haag 2010-05-06 12:29) - PLID 37525 - Converted to DL2 and we now use SelChanging
void CEditInsInfoDlg::SelChangingInsList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try{
		//selecting nothing is meaningless, so disable that ability
		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	}NxCatchAll("Error in CEditInsInfoDlg::OnSelChosenInsList");
}

// (c.haag 2010-05-06 12:29) - PLID 37525 - Completes a selection change
void CEditInsInfoDlg::SelChangedInsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		//(e.lally 2011-07-01) PLID 41207 - Moved code that handles the selection into its own function
		NXDATALIST2Lib::IRowSettingsPtr pOldSel(lpOldSel);
		NXDATALIST2Lib::IRowSettingsPtr pNewSel(lpNewSel);

		HandleSelChangedInsList(pOldSel, pNewSel);
	} NxCatchAll(__FUNCTION__);	
}

//(e.lally 2011-07-01) PLID 41207
void CEditInsInfoDlg::HandleSelChangedInsList(NXDATALIST2Lib::IRowSettingsPtr pOldSel, NXDATALIST2Lib::IRowSettingsPtr pNewSel)
{
	// If a message box appears by virtue of CheckOnExitingCurrentInsCo being called, then
	// it's possible for this event to get fired a second time. Check for this condition, and ignore
	// any subsequent events.
	if (m_bSelChangeInProgress) {
		return;
	}

	// Flag the fact that a selection change is in progress
	m_bSelChangeInProgress = TRUE;


	// Make sure we have a selection
	if(NULL != pNewSel) {
		// Warn the user if anything is out of whack
		if(CheckOnExitingCurrentInsCo(ecicaChangingSel)){

			// We passed validation. Safe to change.
			m_pInsList->CurSel = pNewSel;
			ReflectInsListSelection(pNewSel);

		} else {
			// The user was warned about some unusual info on the screen, and the 
			// user chose to revert back to the existing insurance entry so the 
			// unusual data could be repaired. No need to call ReflectInsListSelection
			// since the form data hasn't changed from the old selection.
			m_pInsList->CurSel = pOldSel;
		}
	} else {
		// The new selection was invalid so revert to the last good selection.
		// No need to call ReflectInsListSelection since the form data hasn't changed from the old selection.
		m_pInsList->CurSel = pOldSel;
	}

	// Flag the fact that a selection change is not in progress
	m_bSelChangeInProgress = FALSE;
}

// (j.jones 2011-04-05 14:49) - PLID 42372 - added the CLIA setup
void CEditInsInfoDlg::OnCliaSetup() 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pInsList->GetCurSel();;
		if(pRow == NULL) {
			return;
		}

		CCLIASetup dlg(this);
		dlg.DoModal(VarLong(pRow->GetValue(0)), m_nColor);
	
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-07-01) PLID 41207 - Load the previous insurance company in the list
void CEditInsInfoDlg::OnBtnPrevInsuranceCo()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pInsList->GetCurSel();
		if(pCurRow != NULL){
			NXDATALIST2Lib::IRowSettingsPtr pPrevRow = pCurRow->GetPreviousRow();
			if(pPrevRow != NULL){
				HandleSelChangedInsList(pCurRow, pPrevRow);
				m_btnInsCoPrev.SetFocus();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-07-01) PLID 41207 - Load the next insurance company in the list
void CEditInsInfoDlg::OnBtnNextInsuranceCo()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pInsList->GetCurSel();
		if(pCurRow != NULL){
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pCurRow->GetNextRow();
			if(pNextRow != NULL){
				HandleSelChangedInsList(pCurRow, pNextRow);
				m_btnInsCoNext.SetFocus();
			}
		}

	}NxCatchAll(__FUNCTION__);
}

//(8/17/2012) r.wilson - PLID 50636
void CEditInsInfoDlg::OnBnClickedBtnEditDefaultDeductible()
{
	try
	{
		CEditDefaultDeductible dlg_EditDefaultDeductible;
		dlg_EditDefaultDeductible.m_nInsuranceCoID = VarLong(m_pInsList->CurSel->GetValue(0),-1);
		dlg_EditDefaultDeductible.m_nColor = m_nColor;
		dlg_EditDefaultDeductible.DoModal();
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2013-08-05 14:32) - PLID 57805 - added link to the HCFA upgrade date setup
BOOL CEditInsInfoDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {

		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		CRect rcUpgradeLabel;
		GetDlgItem(IDC_HCFA_UPGRADE_DATE_LABEL)->GetWindowRect(rcUpgradeLabel);
		ScreenToClient(&rcUpgradeLabel);

		if(rcUpgradeLabel.PtInRect(pt) && m_lblHCFAUpgradeDate.GetType() == dtsHyperlink) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

		// (b.spivey - March 6th, 2014) - PLID 61196 - This should always be a hyper link, really.
		//	 But I replicated what's above for consistency. 
		CRect rcGoLiveLabel;
		m_lblICD10GoLiveDate.GetWindowRect(rcGoLiveLabel); 
		ScreenToClient(&rcGoLiveLabel);

		if(rcGoLiveLabel.PtInRect(pt) && m_lblICD10GoLiveDate.GetType() == dtsHyperlink) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

		

	} NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (j.jones 2013-08-05 14:32) - PLID 57805 - added link to the HCFA upgrade date setup
LRESULT CEditInsInfoDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try {
		
		switch ((UINT)wParam) {
			case IDC_HCFA_UPGRADE_DATE_LABEL:
				{
					NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
					if(pCurInsRow) {
		
						long nInsCoID = VarLong(pCurInsRow->GetValue(0), -1);

						if(nInsCoID != -1) {
							CHCFAUpgradeDateDlg dlg;
							if(dlg.DoModal(nInsCoID, false, m_nColor) == IDOK) {
								LoadHCFAUpgradeDateLabel();
							}
						}
					}
				}
				break;
			// (b.spivey - March 6th, 2014) - PLID 61196 - If they click, lets let them edit the go live date. 
			case IDC_ICD10_GO_LIVE_DATE:
				{
					NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
					if(pCurInsRow) {
		
						long nInsCoID = VarLong(pCurInsRow->GetValue(0), -1);

						if(nInsCoID != -1) {
							CICD10GoLiveDateDlg dlg(nInsCoID);
							dlg.DoModal();
							LoadICD10GoLiveDateLabel();
						}
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

// (j.jones 2013-08-05 14:31) - PLID 57805 - loads the proper label text for m_lblHCFAUpgradeDate
void CEditInsInfoDlg::LoadHCFAUpgradeDateLabel()
{
	try {

		//if there is no group selected, or there are no companies
		//in the group, the label will have its text cleared and
		//will not be enabled
		CString strHCFAUpgradeText = "";

		NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	

		if(pCurInsRow) {
		
			long nInsCoID = VarLong(pCurInsRow->GetValue(0), -1);

			if(nInsCoID != -1) {

				COleDateTime dtNow = COleDateTime::GetCurrentTime();

				_RecordsetPtr rsUpgradeDate = CreateParamRecordset("SELECT HCFAUpgradeDate "
					"FROM InsuranceCoT "
					"WHERE PersonID = {INT}", nInsCoID);
				if(!rsUpgradeDate->eof) {
					COleDateTime dtUpgrade = VarDateTime(rsUpgradeDate->Fields->Item["HCFAUpgradeDate"]->Value);
					strHCFAUpgradeText.Format("This company %s to the new HCFA form on %s.",
						dtUpgrade < dtNow ? "switched" : "will switch",
						FormatDateTimeForInterface(dtUpgrade, NULL, dtoDate));
				}
				rsUpgradeDate->Close();
			}
		}

		m_lblHCFAUpgradeDate.SetText(strHCFAUpgradeText);

		//if the text is blank, disable the hyperlink
		if(strHCFAUpgradeText.IsEmpty()) {
			m_lblHCFAUpgradeDate.SetType(dtsDisabledHyperlink);
		}
		else {
			m_lblHCFAUpgradeDate.SetType(dtsHyperlink);
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.spivey - March 6th, 2014) - PLID 61196 - This function updates the label text 
//	 to properly display the date. 
void CEditInsInfoDlg::LoadICD10GoLiveDateLabel()
{
	CString strLabelText = "";

	NXDATALIST2Lib::IRowSettingsPtr pCurInsRow = m_pInsList->CurSel;
	
	if(pCurInsRow) {
	
		long nInsCoID = VarLong(pCurInsRow->GetValue(0), -1);

		if(nInsCoID != -1) {

			COleDateTime dtNow = COleDateTime::GetCurrentTime();

			_RecordsetPtr rsGoLiveDate = CreateParamRecordset("SELECT ICD10GoLiveDate "
				"FROM InsuranceCoT "
				"WHERE PersonID = {INT}", nInsCoID);
			if(!rsGoLiveDate->eof) {
				COleDateTime dtGoLive = AdoFldDateTime(rsGoLiveDate->Fields, "ICD10GoLiveDate", g_cdtNull);
				if (dtGoLive == g_cdtNull) {
					strLabelText.Format("This company has no ICD-10 go live date."); 
				} 
				else {
					strLabelText.Format("This company %s to ICD-10 on %s.",
						dtGoLive < dtNow ? "switched" : "will switch",
						FormatDateTimeForInterface(dtGoLive, NULL, dtoDate));
				}
			}
			rsGoLiveDate->Close();
		}
	}

	m_lblICD10GoLiveDate.SetText(strLabelText);

	//if the text is blank, disable the hyperlink
	if(strLabelText.IsEmpty()) {
		m_lblICD10GoLiveDate.SetType(dtsDisabledHyperlink);
	}
	else {
		m_lblICD10GoLiveDate.SetType(dtsHyperlink);
	}
}
// (s.tullis 2016-02-29 21:21) - PLID 68262
void CEditInsInfoDlg::OnBnClickedBtnClaimFormSetup()
{
	try {
		CClaimFormLocationInsuranceSetup dlg;
		dlg.m_nColor = m_nColor;
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__)
}

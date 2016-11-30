// InsuranceDlg.cpp : implementation file
#include "stdafx.h"
#include "Practice.h"
#include "InsuranceDlg.h"
#include "MainFrm.h"
#include "GlobalUtils.h"
#include "EditInsInfoDlg.h"
#include "NxStandard.h"
#include "PracProps.h"
#include "GlobalFinancialUtils.h"
#include "GlobalDataUtils.h"
#include "EditComboBox.h"
#include "AuditTrail.h"
#include "InsuranceReferralsDlg.h"
#include "EditInsuranceRespTypesDlg.h"
#include "InternationalUtils.h"
#include "NewInsuredPartyDlg.h"
#include "CPTCodeInsNotesDlg.h"
#include "GlobalDrawingUtils.h"
#include "HL7Utils.h"
#include "GlobalInsuredPartyUtils.h"
#include "EligibilityRequestDlg.h"
#include "InsuranceDeductOOPDlg.h"
#include "EEligibility.h"

//resp type combo columns
#define RTC_ID			0
#define RTC_NAME		1
#define RTC_PRIORITY	2

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXTIMELib;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

// (j.jones 2012-11-12 13:38) - PLID 53622 - added country dropdown
enum CountryComboColumn {
	cccID = 0,
	cccName,
	cccISOCode,
};

/////////////////////////////////////////////////////////////////////////////
// CInsuranceDlg dialog

// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker
CInsuranceDlg::CInsuranceDlg(CWnd* pParent)
	: CPatientDialog(CInsuranceDlg::IDD, pParent)
	,
	m_companyChecker(NetUtils::InsuranceCoT),
	m_inscontactChecker(NetUtils::InsuranceContactsT),
	m_planChecker(NetUtils::InsurancePlansT)
{
	//{{AFX_DATA_INIT(CInsuranceDlg)
	//}}AFX_DATA_INIT
	m_bSettingBox = false;

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Billing/Insurance_Billing/add_a_new_insured_party.htm";
	// (a.walling 2010-10-13 10:44) - PLID 40978 - Dead code
	//m_pGiveFocusToAfterUpdate = NULL;
	m_nCurrentInsCoID = -1;
	m_bInsPlanIsRequerying = false;
	m_bFormattingField = false;
	m_bFormattingAreaCode = false;

	// (a.walling 2010-10-13 10:35) - PLID 40978
	m_id = -1;
}
// (s.dhole 07/23/2012) PLID 48693
CInsuranceDlg::~CInsuranceDlg()
{
	/*GetMainFrame()->IsEligibilityRequestDlgOpen(this); */
}

void CInsuranceDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsuranceDlg)
	DDX_Control(pDX, IDC_SWAP, m_btnSwap);
	//DDX_Control(pDX, IDC_RADIO_COPAY_AMOUNT, m_radioCopayAmount);
	//DDX_Control(pDX, IDC_RADIO_COPAY_PERCENT, m_radioCopayPercent);
	DDX_Control(pDX, IDC_BTN_EDIT_CPT_NOTES, m_btnEditCPTNotes);
	//DDX_Control(pDX, IDC_CHECK_WARN_COPAY, m_checkWarnCoPay);
	DDX_Control(pDX, IDC_EDIT_REFERRALS, m_btnEditReferrals);
	DDX_Control(pDX, IDC_COPY_EMP_INFO, m_copyEmployerInfoButton);
	DDX_Control(pDX, IDC_EDIT_INSURANCE_LIST, m_editInsList);
	DDX_Control(pDX, IDC_BTN_PREV_PARTY, m_prev);
	DDX_Control(pDX, IDC_BTN_NEXT_PARTY, m_next);
	DDX_Control(pDX, IDC_BTN_DELETE_INSURANCE, m_delete);
	DDX_Control(pDX, IDC_BTN_ADD_INSURANCE, m_add);
	DDX_Control(pDX, IDC_INS_COUNT, m_insCountButton);
	DDX_Control(pDX, IDC_COPY_PATIENT_INFO, m_copyPatientInfoButton);
	DDX_Control(pDX, IDC_CONTACT_BKG, m_contactBkg);
	DDX_Control(pDX, IDC_PATIENT_BKG, m_patientBkg);
	DDX_Control(pDX, IDC_PLACEMENT_BKG, m_placementBkg);
	DDX_Control(pDX, IDC_SELECTION_BKG, m_selectionBkg);
	DDX_Control(pDX, IDC_FIRST_NAME_BOX, m_nxeditFirstNameBox);
	DDX_Control(pDX, IDC_MIDDLE_NAME_BOX, m_nxeditMiddleNameBox);
	DDX_Control(pDX, IDC_LAST_NAME_BOX, m_nxeditLastNameBox);
	DDX_Control(pDX, IDC_TITLE_BOX, m_nxeditTitleBox);
	DDX_Control(pDX, IDC_ADDRESS1_BOX, m_nxeditAddress1Box);
	DDX_Control(pDX, IDC_ADDRESS2_BOX, m_nxeditAddress2Box);
	DDX_Control(pDX, IDC_ZIP_BOX, m_nxeditZipBox);
	DDX_Control(pDX, IDC_CITY_BOX, m_nxeditCityBox);
	DDX_Control(pDX, IDC_STATE_BOX, m_nxeditStateBox);
	DDX_Control(pDX, IDC_EMPLOYER_SCHOOL_BOX, m_nxeditEmployerSchoolBox);
	DDX_Control(pDX, IDC_INS_PARTY_SSN, m_nxeditInsPartySsn);
	DDX_Control(pDX, IDC_PHONE_BOX, m_nxeditPhoneBox);
	DDX_Control(pDX, IDC_MEMO_BOX, m_nxeditMemoBox);
	DDX_Control(pDX, IDC_CONTACT_ADDRESS_BOX, m_nxeditContactAddressBox);
	DDX_Control(pDX, IDC_CONTACT_CITY_BOX, m_nxeditContactCityBox);
	DDX_Control(pDX, IDC_CONTACT_STATE_BOX, m_nxeditContactStateBox);
	DDX_Control(pDX, IDC_CONTACT_ZIP_BOX, m_nxeditContactZipBox);
	DDX_Control(pDX, IDC_CONTACT_PHONE_BOX, m_nxeditContactPhoneBox);
	DDX_Control(pDX, IDC_CONTACT_EXT_BOX, m_nxeditContactExtBox);
	DDX_Control(pDX, IDC_CONTACT_FAX_BOX, m_nxeditContactFaxBox);
	//DDX_Control(pDX, IDC_COPAY, m_nxeditCopay);
	DDX_Control(pDX, IDC_CO_MEMO, m_nxeditCoMemo);
	DDX_Control(pDX, IDC_TYPE_BOX, m_nxeditTypeBox);
	DDX_Control(pDX, IDC_INSURANCE_ID_BOX, m_nxeditInsuranceIdBox);
	DDX_Control(pDX, IDC_FECA_BOX, m_nxeditFecaBox);
	DDX_Control(pDX, IDC_ACC_BKG, m_accBkg);
	DDX_Control(pDX, IDC_EDIT_ACC_STATE, m_editAccState);
	DDX_Control(pDX, IDC_RADIO_EMPLOYMENT, m_radioEmploymentAcc);
	DDX_Control(pDX, IDC_RADIO_AUTO_ACCIDENT, m_radioAutoAcc);
	DDX_Control(pDX, IDC_RADIO_OTHER_ACCIDENT, m_radioOtherAcc);
	DDX_Control(pDX, IDC_RADIO_NONE, m_radioNoneAcc);
	DDX_Control(pDX, IDC_CHECK_SUBMIT_AS_PRIMARY, m_checkSubmitAsPrimary);
	DDX_Control(pDX, IDC_INS_PLAN_NAME_LABEL, m_nxstaticInsPlanNameLabel);
	DDX_Control(pDX, IDC_INS_PLAN_TYPE_LABEL, m_nxstaticInsPlanTypeLabel);
	DDX_Control(pDX, IDC_ID_FOR_INSURANCE_LABEL, m_nxstaticIDForInsuranceLabel);
	DDX_Control(pDX, IDC_PATIENT_ID_FOR_INSURANCE_LABEL, m_nxstaticPatientIDForInsuranceLabel);
	DDX_Control(pDX, IDC_PATIENT_INSURANCE_ID_BOX, m_nxeditPatientIDForInsurance);
	DDX_Control(pDX, IDC_BTN_CREATE_INS_ELIGIBILITY_REQUEST, m_btnCreateEligibilityRequest);
	DDX_Control(pDX, IDC_INS_EDIT_DEDUCT_OOP, m_btnDeductOOP);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInsuranceDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInsuranceDlg)
	ON_BN_CLICKED(IDC_BTN_NEXT_PARTY, OnBtnNextParty)
	ON_BN_CLICKED(IDC_BTN_PREV_PARTY, OnBtnPrevParty)
	ON_BN_CLICKED(IDC_BTN_ADD_INSURANCE, OnBtnAddInsurance)
	ON_BN_CLICKED(IDC_BTN_DELETE_INSURANCE, OnBtnDeleteInsurance)
	ON_BN_CLICKED(IDC_COPY_PATIENT_INFO, OnCopyPatientInfo)
	ON_BN_CLICKED(IDC_EDIT_INSURANCE_LIST, OnEditInsuranceList)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_COPY_EMP_INFO, OnCopyEmpInfo)
	ON_BN_CLICKED(IDC_EDIT_REFERRALS, OnEditReferrals)
	ON_BN_CLICKED(IDC_SWAP, OnSwap)
	ON_BN_CLICKED(IDC_EDIT_INS_RESP_TYPES, OnEditInsRespTypes)
	//ON_BN_CLICKED(IDC_CHECK_WARN_COPAY, OnCheckWarnCopay)
	ON_BN_CLICKED(IDC_BTN_EDIT_CPT_NOTES, OnBtnEditCptNotes)
	ON_EN_KILLFOCUS(IDC_INS_PARTY_SSN, OnKillfocusSsn)
//	ON_BN_CLICKED(IDC_RADIO_COPAY_AMOUNT, OnRadioCopayAmount)
	//ON_BN_CLICKED(IDC_RADIO_COPAY_PERCENT, OnRadioCopayPercent)
	ON_BN_CLICKED(IDC_RADIO_EMPLOYMENT, OnRadioEmployment)
	ON_BN_CLICKED(IDC_RADIO_AUTO_ACCIDENT, OnRadioAutoAccident)
	ON_BN_CLICKED(IDC_RADIO_OTHER_ACCIDENT, OnRadioOtherAccident)
	ON_BN_CLICKED(IDC_RADIO_NONE, OnRadioNone)
	ON_BN_CLICKED(IDC_CHECK_SUBMIT_AS_PRIMARY, OnCheckSubmitAsPrimary)
	// (j.gruber 2006-12-29 15:14) - PLID 23972 - take this out
	//ON_EN_KILLFOCUS(IDC_DEDUCTIBLE, OnKillfocusDeductible)
	//}}AFX_MSG_MAP	
	ON_BN_CLICKED(IDC_BTN_CREATE_INS_ELIGIBILITY_REQUEST, OnBtnCreateInsEligibilityRequest)
	ON_BN_CLICKED(IDC_INS_EDIT_DEDUCT_OOP, &CInsuranceDlg::OnBnClickedInsEditDeductOop)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsuranceDlg message handlers

BEGIN_EVENTSINK_MAP(CInsuranceDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInsuranceDlg)
	ON_EVENT(CInsuranceDlg, IDC_COMPANY_COMBO, 9 /* Return */, OnReturnCompanyCombo, VTS_NONE)
	ON_EVENT(CInsuranceDlg, IDC_INSURANCE_PLAN_BOX, 16 /* SelChosen */, OnSelChosenInsurancePlan, VTS_I4)
	ON_EVENT(CInsuranceDlg, IDC_INSURANCE_PLAN_BOX, 9 /* Return */, OnReturnInsurancePlanBox, VTS_NONE)
	ON_EVENT(CInsuranceDlg, IDC_COMPANY_COMBO, 16 /* SelChosen */, OnSelChosenCompanyCombo, VTS_I4)
	ON_EVENT(CInsuranceDlg, IDC_COMPANY_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedCompanyCombo, VTS_I2)
	ON_EVENT(CInsuranceDlg, IDC_INSURANCE_PLAN_BOX, 18 /* RequeryFinished */, OnRequeryFinishedInsurancePlanBox, VTS_I2)
	ON_EVENT(CInsuranceDlg, IDC_GENDER_LIST, 11 /* ClosedUp */, OnClosedUpGenderList, VTS_I4)
	ON_EVENT(CInsuranceDlg, IDC_CONTACTS_COMBO, 16 /* SelChosen */, OnSelChosenContactsCombo, VTS_I4)
	ON_EVENT(CInsuranceDlg, IDC_CONTACTS_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedContactsCombo, VTS_I2)
	ON_EVENT(CInsuranceDlg, IDC_RESP_TYPE_COMBO, 16 /* SelChosen */, OnSelChosenRespTypeCombo, VTS_I4)
	ON_EVENT(CInsuranceDlg, IDC_BIRTH_DATE_BOX, 1 /* KillFocus */, OnKillFocusBirthDateBox, VTS_NONE)
	ON_EVENT(CInsuranceDlg, IDC_EFFECTIVE_DATE, 1 /* KillFocus */, OnKillFocusEffectiveDate, VTS_NONE)
	ON_EVENT(CInsuranceDlg, IDC_COMPANY_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedCompanyCombo, VTS_I4 VTS_I4)
	ON_EVENT(CInsuranceDlg, IDC_INACTIVE_DATE, 1 /* KillFocus */, OnKillFocusInactiveDate, VTS_NONE)
	ON_EVENT(CInsuranceDlg, IDC_RELATE_COMBO, 16 /* SelChosen */, OnSelChosenRelateCombo, VTS_I4)
	ON_EVENT(CInsuranceDlg, IDC_INSURANCE_PLAN_BOX, 1 /* SelChanging */, OnSelChangingInsurancePlanBox, VTS_PI4)
	ON_EVENT(CInsuranceDlg, IDC_COMPANY_COMBO, 1 /* SelChanging */, OnSelChangingCompanyCombo, VTS_PI4)
	ON_EVENT(CInsuranceDlg, IDC_GENDER_LIST, 2 /* SelChanged */, OnSelChangedGenderList, VTS_I4)
	ON_EVENT(CInsuranceDlg, IDC_GENDER_LIST, 24 /* FocusLost */, OnFocusLostGenderList, VTS_NONE)
	ON_EVENT(CInsuranceDlg, IDC_SECONDARY_REASON_CODE, 16 /* SelChosen */, OnSelChosenSecondaryReasonCode, VTS_DISPATCH)
	ON_EVENT(CInsuranceDlg, IDC_EDIT_CUR_ACC_DATE, 1, OnKillFocusCurAccDate, VTS_NONE)
	//}}AFX_EVENTSINK_MAP	
	ON_EVENT(CInsuranceDlg, IDC_PAY_GROUP_LIST, 8, CInsuranceDlg::EditingStartingPayGroupList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CInsuranceDlg, IDC_PAY_GROUP_LIST, 10, CInsuranceDlg::EditingFinishedPayGroupList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CInsuranceDlg, IDC_PAY_GROUP_LIST, 9, CInsuranceDlg::EditingFinishingPayGroupList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CInsuranceDlg, IDC_COUNTRY_LIST, 1, OnSelChangingCountryList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CInsuranceDlg, IDC_COUNTRY_LIST, 16, OnSelChosenCountryList, VTS_DISPATCH)
END_EVENTSINK_MAP()

int CInsuranceDlg::Hotkey(int key)
{
	if (key == VK_ESCAPE) {
		UpdateView();
		return 0;
	}

	//unhandled
	return 1;
}

void CInsuranceDlg::SetColor(OLE_COLOR nNewColor)
{
	m_contactBkg.SetColor(nNewColor);
	m_patientBkg.SetColor(nNewColor);
	m_placementBkg.SetColor(nNewColor);
	m_selectionBkg.SetColor(nNewColor);
//	m_checkWarnCoPay.SetColor(nNewColor);
	//m_radioCopayAmount.SetColor(nNewColor);
	//m_radioCopayPercent.SetColor(nNewColor);
	// (j.jones 2009-03-05 09:17) - PLID 28834 - added accident controls
	m_accBkg.SetColor(nNewColor);

	// (b.spivey, May 22, 2012) - PLID 50558 - default patient color always
	m_nColor = GetNxColor(GNC_PATIENT_STATUS, 1); 

	m_prev.AutoSet(NXB_LEFT);
	m_next.AutoSet(NXB_RIGHT);

	if (m_brush.m_hObject) m_brush.DeleteObject();
	m_brush.CreateSolidBrush(PaletteColor(nNewColor));

	CPatientDialog::SetColor(nNewColor);
}

BOOL CInsuranceDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (d.singleton 2012-06-18 11:48) - PLID 51029
		m_nxeditAddress1Box.SetLimitText(150);
		m_nxeditAddress2Box.SetLimitText(150);

		// (j.jones 2012-10-24 14:28) - PLID 36305 - added Title
		m_nxeditTitleBox.SetLimitText(50);

		m_insCountButton.EnableMouseOver(false);
		m_insCountButton.EnableShowClick(false);
		//DRT 4/11/2008 - PLID 29636 - NxIconify
		m_add.AutoSet(NXB_NEW);
		m_delete.AutoSet(NXB_DELETE);
		m_copyPatientInfoButton.AutoSet(NXB_MODIFY);
		m_copyEmployerInfoButton.AutoSet(NXB_MODIFY);
		m_editInsList.AutoSet(NXB_MODIFY);
		m_btnEditReferrals.AutoSet(NXB_MODIFY);
		m_btnEditCPTNotes.AutoSet(NXB_MODIFY);
		m_btnSwap.AutoSet(NXB_MODIFY);
		// (j.jones 2010-07-19 10:35) - PLID 31082 - added ability to create an eligibility request
		m_btnCreateEligibilityRequest.AutoSet(NXB_NEW);
		// (j.gruber 2010-07-30 10:49) - PLID 39727 - edit deduct/oop
		m_btnDeductOOP.AutoSet(NXB_MODIFY);

		// (j.jones 2010-07-19 10:37) - PLID 31082 - hide the eligibility button if they don't have the license
		if(!g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrSilent)) {
			m_btnCreateEligibilityRequest.ShowWindow(SW_HIDE);
		}

		// (j.jones 2009-03-05 09:17) - PLID 28834 - limit the accident state to 10 characters
		m_editAccState.SetLimitText(10);

		// (j.jones 2010-05-18 11:37) - PLID 37788 - limit the ID For Insurance fields to 50 characters
		m_nxeditInsuranceIdBox.SetLimitText(50);
		m_nxeditPatientIDForInsurance.SetLimitText(50);

		// (j.jones 2009-03-06 09:34) - PLID 28834 - added accident date
		m_nxtAccidentDate = GetDlgItemUnknown(IDC_EDIT_CUR_ACC_DATE);
		m_dtAccidentDate.m_dt = 0;
		m_dtAccidentDate.SetStatus(COleDateTime::invalid);
		m_bSavingAccidentDate = false;

		m_nxtBirthDate = GetDlgItemUnknown(IDC_BIRTH_DATE_BOX);
		//m_bSavingBirthDate = false;
		//(e.lally 2006-09-14) PLID 22530 - This still not does work and was missed as
		//a part of 20116, making it possible to re-save the birthdate as null before it is loaded.
		m_dtBirthDate.m_dt = 0;
		m_dtBirthDate.SetStatus(COleDateTime::invalid);
		m_nxtEffectiveDate = GetDlgItemUnknown(IDC_EFFECTIVE_DATE);
		m_bSavingEffectiveDate = false;
		//(e.lally 2006-09-14) PLID 22530 - This still not does work and was missed as
		//a part of 20116, I am changing it to avoid similar problems as the birthdate.
		m_dtEffectiveDate.m_dt = 0;
		m_dtEffectiveDate.SetStatus(COleDateTime::invalid);
		m_nxtInactiveDate = GetDlgItemUnknown(IDC_INACTIVE_DATE);
		m_bSavingInactiveDate = false;
		//(e.lally 2006-09-14) PLID 22530 - This still not does work and was missed as
		//a part of 20116, I am changing it to avoid similar problems as the birthdate.
		m_dtInactiveDate.m_dt = 0;
		m_dtInactiveDate.SetStatus(COleDateTime::invalid);
		// (j.jones 2006-12-18 11:31) - PLID 23898 - made the inactive date read only, instead of disabled
		// (j.jones 2008-09-10 17:20) - PLID 14002 - allowed the inactive date to be editable, at all times
		//m_nxtInactiveDate->SetReadOnly(TRUE);

		// (j.jones 2010-05-18 10:24) - PLID 37788 - cached preferences
		g_propManager.CachePropertiesInBulk("CInsuranceDlg-1", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'ShowPatientIDForInsurance' OR "
			"Name = 'FormatPhoneNums' OR "
			"Name = 'LookupZipStateByCity' OR "
			"Name = 'AutoCapitalizeMiddleInitials' OR "
			"Name = 'FormatStyle' OR "	// (j.jones 2010-07-19 10:52) - PLID 31082
			// (j.jones 2011-06-24 16:45) - PLID 31005 - added AutoCapitalizeInsuranceIDs
			"Name = 'AutoCapitalizeInsuranceIDs' "
			")", _Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("CInsuranceDlg-2", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'PhoneFormatString' OR "
			// (j.jones 2011-06-24 12:24) - PLID 29885 - added DefaultMedicareSecondaryReasonCode
			"Name = 'DefaultMedicareSecondaryReasonCode' "
			")", _Q(GetCurrentUserName()));

		// (j.jones 2010-05-18 10:21) - PLID 37788 - do they want to show two ID For Insurance fields?
		// (j.gruber 2010-08-02 10:44) - PLID 39729 - changed the boxes around on the dlg, so changed this around accordingly
		if(GetRemotePropertyInt("ShowPatientIDForInsurance", 0, 0, "<None>", true) == 1) {

			//just rearrange fields in OnInitDialog, this is not a setting that will be changed
			//more than once, and the preferences state that they need to reopen the Patients module

			CRect rcNotesBox, rcInsPlanNameLabel, rcInsPlanNameBox, rcInsPlanTypeLabel, rcInsPlanTypeBox,
				rcIDForInsLabel, rcIDForInsBox, rcPatientsIDForInsLabel, rcPatientsIDForInsBox,
				rcDeductibleButton, rcEditRefButton, rcPlanGroupDl;

			GetDlgItem(IDC_CO_MEMO)->GetWindowRect(rcNotesBox);
			ScreenToClient(rcNotesBox);
			GetDlgItem(IDC_INS_PLAN_NAME_LABEL)->GetWindowRect(rcInsPlanNameLabel);
			ScreenToClient(rcInsPlanNameLabel);
			GetDlgItem(IDC_INSURANCE_PLAN_BOX)->GetWindowRect(rcInsPlanNameBox);
			ScreenToClient(rcInsPlanNameBox);
			GetDlgItem(IDC_INS_PLAN_TYPE_LABEL)->GetWindowRect(rcInsPlanTypeLabel);
			ScreenToClient(rcInsPlanTypeLabel);
			GetDlgItem(IDC_TYPE_BOX)->GetWindowRect(rcInsPlanTypeBox);
			ScreenToClient(rcInsPlanTypeBox);
			GetDlgItem(IDC_ID_FOR_INSURANCE_LABEL)->GetWindowRect(rcIDForInsLabel);
			ScreenToClient(rcIDForInsLabel);
			GetDlgItem(IDC_INSURANCE_ID_BOX)->GetWindowRect(rcIDForInsBox);
			ScreenToClient(rcIDForInsBox);
			GetDlgItem(IDC_PATIENT_ID_FOR_INSURANCE_LABEL)->GetWindowRect(rcPatientsIDForInsLabel);
			ScreenToClient(rcPatientsIDForInsLabel);
			GetDlgItem(IDC_PATIENT_INSURANCE_ID_BOX)->GetWindowRect(rcPatientsIDForInsBox);
			ScreenToClient(rcPatientsIDForInsBox);

			GetDlgItem(IDC_EDIT_REFERRALS)->GetWindowRect(rcEditRefButton);
			ScreenToClient(rcEditRefButton);
			GetDlgItem(IDC_INS_EDIT_DEDUCT_OOP)->GetWindowRect(rcDeductibleButton);
			ScreenToClient(rcDeductibleButton);
			GetDlgItem(IDC_PAY_GROUP_LIST)->GetWindowRect(rcPlanGroupDl);
			ScreenToClient(rcPlanGroupDl);


			//now start moving the controls
			int nDiff = rcInsPlanTypeLabel.top - rcInsPlanNameLabel.bottom;
			rcDeductibleButton.top = rcPatientsIDForInsBox.top - nDiff - 1 - rcDeductibleButton.Height();
			rcDeductibleButton.bottom = rcPatientsIDForInsBox.top - nDiff - 1;
			rcEditRefButton.top = rcDeductibleButton.top - nDiff - 1 - rcEditRefButton.Height();
			rcEditRefButton.bottom = rcDeductibleButton.top - nDiff - 1;
			rcNotesBox.bottom = rcEditRefButton.top - nDiff - 1;
			rcPlanGroupDl.bottom = rcPatientsIDForInsBox.top - nDiff - 1;
			CRect rcTopLabel = rcPatientsIDForInsLabel;
			CRect rcTopBox = rcPatientsIDForInsBox;
			rcPatientsIDForInsLabel = rcIDForInsLabel;
			rcPatientsIDForInsBox = rcIDForInsBox;
			rcIDForInsLabel = rcInsPlanTypeLabel;
			rcIDForInsBox = rcInsPlanTypeBox;
			rcInsPlanTypeLabel = rcInsPlanNameLabel;
			rcInsPlanTypeBox = rcInsPlanTypeBox;
			rcInsPlanNameLabel = rcTopLabel;
			rcInsPlanTypeBox = rcTopBox;

			GetDlgItem(IDC_CO_MEMO)->MoveWindow(rcNotesBox);
			GetDlgItem(IDC_INS_PLAN_NAME_LABEL)->MoveWindow(rcInsPlanNameLabel);
			GetDlgItem(IDC_INSURANCE_PLAN_BOX)->MoveWindow(rcInsPlanNameBox);
			GetDlgItem(IDC_INS_PLAN_TYPE_LABEL)->MoveWindow(rcInsPlanTypeLabel);
			GetDlgItem(IDC_TYPE_BOX)->MoveWindow(rcInsPlanTypeBox);
			GetDlgItem(IDC_ID_FOR_INSURANCE_LABEL)->MoveWindow(rcIDForInsLabel);
			GetDlgItem(IDC_INSURANCE_ID_BOX)->MoveWindow(rcIDForInsBox);
			GetDlgItem(IDC_PATIENT_ID_FOR_INSURANCE_LABEL)->MoveWindow(rcPatientsIDForInsLabel);
			GetDlgItem(IDC_PATIENT_INSURANCE_ID_BOX)->MoveWindow(rcPatientsIDForInsBox);
			GetDlgItem(IDC_EDIT_REFERRALS)->MoveWindow(rcEditRefButton);
			GetDlgItem(IDC_INS_EDIT_DEDUCT_OOP)->MoveWindow(rcDeductibleButton);
			GetDlgItem(IDC_PAY_GROUP_LIST)->MoveWindow(rcPlanGroupDl);


			GetDlgItem(IDC_ID_FOR_INSURANCE_LABEL)->SetWindowText("Subscriber Insurance ID Num.");
			GetDlgItem(IDC_PATIENT_ID_FOR_INSURANCE_LABEL)->SetWindowText("Patient's Insurance ID Number");

			//this resets the stored control positions so NxDialog's resizing
			//now resizes using the current placements, instead of what's in the .rc
			GetControlPositions();
		}
		else {
			//most of the time this field will be hidden
			GetDlgItem(IDC_PATIENT_ID_FOR_INSURANCE_LABEL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_PATIENT_INSURANCE_ID_BOX)->ShowWindow(SW_HIDE);
		}

		m_bDropPlanBox = FALSE;
		m_RelateCombo = BindNxDataListCtrl(IDC_RELATE_COMBO,false);
		m_CompanyCombo = BindNxDataListCtrl(IDC_COMPANY_COMBO);
		m_PlanList = BindNxDataListCtrl(IDC_INSURANCE_PLAN_BOX,false);
		m_GenderCombo = BindNxDataListCtrl(IDC_GENDER_LIST, false);
		m_ContactsCombo = BindNxDataListCtrl(IDC_CONTACTS_COMBO,false);
		m_listRespType = BindNxDataListCtrl(IDC_RESP_TYPE_COMBO, false);
		
		// (j.jones 2012-11-12 10:49) - PLID 53622 - added country dropdown
		m_CountryList = BindNxDataList2Ctrl(IDC_COUNTRY_LIST, true);
		NXDATALIST2Lib::IRowSettingsPtr pCountryRow = m_CountryList->GetNewRow();
		pCountryRow->PutValue(cccID, (long)-1);
		pCountryRow->PutValue(cccName, (LPCTSTR)"");	//ideally should be null, but set as an empty string for sorting
		pCountryRow->PutValue(cccISOCode, (LPCTSTR)"");
		m_CountryList->AddRowSorted(pCountryRow, NULL);

		// (j.gruber 2012-07-26 10:02) - PLID 51777 - add categoires
		m_listRespType->FromClause = "(SELECT ID, TypeName, Priority, CASE WHEN Priority IN (1,2) THEN 'Medical' "
			" WHEN Priority = -1 THEN '' ELSE "
			" CASE WHEN CategoryType = 2 THEN 'Vision' "
			" WHEN CategoryType = 3 THEN 'Auto' "
			" WHEN CategoryType = 4 THEN 'Workers'' Comp.' "
			" WHEN CategoryType = 5 THEN 'Dental' "
			" WHEN CategoryType = 6 THEN 'Study' "
			" WHEN CategoryType = 7 THEN 'Letter of Protection' "
			" WHEN CategoryType = 8 THEN 'Letter of Agreement' "
			" ELSE 'Medical' END END AS CategoryType FROM RespTypeT) AS RespTypeQ ";
		m_listRespType->Requery();

		m_pSecondaryReasonCode = BindNxDataList2Ctrl(IDC_SECONDARY_REASON_CODE, false);

		// (j.gruber 2010-08-02 13:41) - PLID 39729 - InsuredPartyCopay list
		m_pPayGroupsList = BindNxDataList2Ctrl(IDC_PAY_GROUP_LIST, false);
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pPayGroupsList->GetColumn(gpgcCopayMoney);
		if (pCol) {
			pCol->ColumnTitle= _bstr_t("Copay " + GetCurrencySymbol());
		}


		IColumnSettingsPtr(m_RelateCombo->GetColumn(m_RelateCombo->InsertColumn(0,"Relation","Relation",-1,csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;

		IRowSettingsPtr pRow;
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Self");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Child");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Spouse");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Other");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Unknown");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Employee");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Organ Donor");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Cadaver Donor");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Life Partner");
		m_RelateCombo->AddRow(pRow);

		// (j.jones 2011-06-15 17:00) - PLID 40959 - the following are no longer valid entries in our system,
		// and no longer exist in data
		/*
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Grandparent");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Grandchild");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Nephew Or Niece");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Adopted Child");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Foster Child");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Ward");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Stepchild");	
		m_RelateCombo->AddRow(pRow);	
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Handicapped Dependent");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Sponsored Dependent");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Dependent of a Minor Dependent");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Significant Other");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Mother");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Father");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Other Adult");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Emancipated Minor");
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Injured Plaintiff");   // (s.dhole 2010-08-31 15:13) - PLID 40114 All our relationship lists say "Injured Plantiff" instead of "Injured Plaintiff"
		m_RelateCombo->AddRow(pRow);
		pRow = m_RelateCombo->GetRow(-1);
		pRow->PutValue(0,"Child Where Insured Has No Financial Responsibility");
		m_RelateCombo->AddRow(pRow);
		*/

		//setup gender combo
		IColumnSettingsPtr(m_GenderCombo->GetColumn(m_GenderCombo->InsertColumn(0, _T("Gender"), _T("Gender"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
		pRow = m_GenderCombo->GetRow(-1);
		_variant_t var = _bstr_t("");
		pRow->PutValue(0, var);
		m_GenderCombo->AddRow(pRow);
		pRow = m_GenderCombo->GetRow(-1);
		var = _bstr_t("Male");
		pRow->PutValue(0, var);
		m_GenderCombo->AddRow(pRow);
		pRow = m_GenderCombo->GetRow(-1);
		var = _bstr_t("Female");
		pRow->PutValue(0, var);
		m_GenderCombo->AddRow(pRow);

		//TES 6/11/2007 - PLID 26257 - Fill in the list of Secondary Reason Codes (I copied this list out of ebilling.cpp, I presume
		// that it originated with Medicare).
		// (j.jones 2011-06-24 12:15) - PLID 29885 - if you ever change this list, change the preferences, because you are allowed
		// to select a default value
		NXDATALIST2Lib::IRowSettingsPtr pRow2 = m_pSecondaryReasonCode->GetNewRow();
		pRow2->PutValue(0,"12");
		pRow2->PutValue(1,"Medicare Secondary Working Aged Beneficiary or Spouse with Employer Group Health Plan");
		m_pSecondaryReasonCode->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_pSecondaryReasonCode->GetNewRow();
		pRow2->PutValue(0,"13");
		pRow2->PutValue(1,"Medicare Secondary End-Stage Renal Disease Beneficiary in the 12 month coordination period with an employer’s group health plan");
		m_pSecondaryReasonCode->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_pSecondaryReasonCode->GetNewRow();
		pRow2->PutValue(0,"14");
		pRow2->PutValue(1,"Medicare Secondary, No-fault Insurance including Auto is Primary");
		m_pSecondaryReasonCode->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_pSecondaryReasonCode->GetNewRow();
		pRow2->PutValue(0,"15");
		pRow2->PutValue(1,"Medicare Secondary Worker’s Compensation");
		m_pSecondaryReasonCode->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_pSecondaryReasonCode->GetNewRow();
		pRow2->PutValue(0,"16");
		pRow2->PutValue(1,"Medicare Secondary Public Health Service (PHS) or Other Federal Agency");
		m_pSecondaryReasonCode->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_pSecondaryReasonCode->GetNewRow();
		pRow2->PutValue(0,"41");
		// (j.gruber 2013-03-18 16:32) - PLID 55735 - fix the description
		pRow2->PutValue(1,"Medicare Secondary Black Lung");
		m_pSecondaryReasonCode->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_pSecondaryReasonCode->GetNewRow();
		pRow2->PutValue(0,"42");
		pRow2->PutValue(1,"Medicare Secondary Veteran’s Administration");
		m_pSecondaryReasonCode->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_pSecondaryReasonCode->GetNewRow();
		pRow2->PutValue(0,"43");
		pRow2->PutValue(1,"Medicare Secondary Disabled Beneficiary Under Age 65 with Large Group Health Plan (LGHP)");
		m_pSecondaryReasonCode->AddRowAtEnd(pRow2, NULL);
		pRow2 = m_pSecondaryReasonCode->GetNewRow();
		pRow2->PutValue(0,"47");
		pRow2->PutValue(1,"Medicare Secondary, Other Liability Insurance is Primary");
		m_pSecondaryReasonCode->AddRowAtEnd(pRow2, NULL);

		CString tmpSearch;

		// (a.walling 2010-10-13 10:35) - PLID 40978 - Don't load the id until UpdateView is called
		/*
		m_id = GetActivePatientID();
		m_strPatientName = GetExistingPatientName(m_id);
		*/

		m_CurrentID = -1;
		// (a.walling 2010-10-13 10:44) - PLID 40978 - Dead code
		//m_lastID = -25;
		m_lastIns = -25;

		m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true); 
		m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

		SecureControls();
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

int m_iPrevPatient = -9999;

void CInsuranceDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {
		// (a.walling 2010-10-12 17:43) - PLID 40978 - Forces focus lost messages
		CheckFocus();

		// (z.manning, 07/25/2006) - PLID 20726 - Need to store detials so that any possible changes aren't lost.
		StoreDetails();

		// (j.gruber 2009-10-06 17:02) - PLID 35826 - reset here in case they changed the preference
		m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");

		if (m_bLookupByCity) {
			ChangeZOrder(IDC_ZIP_BOX, IDC_STATE_BOX);
		} else {
			ChangeZOrder(IDC_ZIP_BOX, IDC_ADDRESS2_BOX);
		}

		CWaitCursor pwait;

		// (a.walling 2010-10-12 14:43) - PLID 40978
		long nCurrentlyLoadedID = m_id;
		m_id = GetActivePatientID();
		m_strPatientName = GetExistingPatientName(m_id);

		if(m_iPrevPatient!=m_id) {
			m_CurrentID = -1;
			m_ForceRefresh = true;
		}

		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;
		}

		// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker
		if (m_companyChecker.PeekChanged() || m_planChecker.PeekChanged() || m_inscontactChecker.PeekChanged()) {
			m_ForceRefresh = true;
		}

		if (bForceRefresh || m_ForceRefresh) {			
			LoadInsInfo();
			Load();
			m_iPrevPatient = m_id;
		}
		m_ForceRefresh = false;

	}NxCatchAll("Error in UpdateView()");
}

// (a.walling 2010-10-13 11:08) - PLID 40978
void CInsuranceDlg::LoadInsInfo()
{	
	_RecordsetPtr rsActive, rsInactive;
	CString sql;
	m_next.EnableWindow(FALSE);
	m_prev.EnableWindow(FALSE);
	m_delete.EnableWindow(FALSE);

	//JMJ - 7/14/2003 - This seems unnecessary but what we achieve here is the ability to have the order
	//sorted by priority, with inactives at the end.
	//DRT 10/21/2008 - PLID 31774 - Parameterized
	// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading Insurance dialog
	rsActive = CreateParamRecordset(GetRemoteDataSnapshot(), _T("SELECT InsuredPartyT.PersonID AS ID FROM InsuredPartyT "
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"WHERE InsuredPartyT.PatientID = {INT} AND RespTypeT.Priority <> -1 "
		"ORDER BY RespTypeT.Priority"), m_id);

	//DRT 10/21/2008 - PLID 31774 - Parameterized
	// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading Insurance dialog
	rsInactive = CreateParamRecordset(GetRemoteDataSnapshot(), _T("SELECT InsuredPartyT.PersonID AS ID FROM InsuredPartyT "
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"WHERE InsuredPartyT.PatientID = {INT} AND RespTypeT.Priority = -1 "
		"ORDER BY InsuredPartyT.PersonID"), m_id);

	// Rebuild Ins. a of b button
	if (rsActive->eof && rsInactive->eof)
	{	SetDlgItemText(IDC_INS_COUNT, "No Insurance");
		m_next.EnableWindow(FALSE);
		m_prev.EnableWindow(FALSE);
		m_delete.EnableWindow(FALSE);
		EnableInsuredInfo(false);
	}
	else
	{
		long cur=0,total=0;
		while(!rsActive->eof) {
			total++;
			if(m_CurrentID == -1){
				cur = 1;
				m_CurrentID = rsActive->Fields->Item["ID"]->Value.lVal;
			}
			else if(rsActive->Fields->Item["ID"]->Value.lVal == m_CurrentID) {
				cur = total;
			}
			rsActive->MoveNext();
		}
		rsActive->Close();

		while(!rsInactive->eof) {
			total++;
			if(m_CurrentID == -1){
				cur = 1;
				m_CurrentID = rsInactive->Fields->Item["ID"]->Value.lVal;
			}
			else if(rsInactive->Fields->Item["ID"]->Value.lVal == m_CurrentID) {
				cur = total;
			}
			rsInactive->MoveNext();
		}
		rsInactive->Close();

		CString str;
		str.Format(_T("Ins. %li of %li"), cur, total);
		SetDlgItemText(IDC_INS_COUNT, str);
		//Make sure they can enter insured party information
		EnableInsuredInfo(true);
		if(cur!=total)
			m_next.EnableWindow();
		if(cur!=1)
			m_prev.EnableWindow();
		m_delete.EnableWindow();
	}
}

// (a.walling 2010-10-13 11:08) - PLID 40978
void CInsuranceDlg::HandleTableCheckers()
{
	if (m_companyChecker.Changed())
	{
		_variant_t varOld;
		if(m_CompanyCombo->GetCurSel()!=-1)
			varOld = m_CompanyCombo->GetValue(m_CompanyCombo->GetCurSel(),0);
		else
			//they must have an inactive selected
			varOld = m_nCurrentInsCoID;
		
		m_CompanyCombo->Requery();

		if(varOld.vt!=VT_NULL) {
		//	m_CompanyCombo->TrySetSelByColumn(0,varOld);
			m_CompanyCombo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			SetCompanyWithInactive(VarLong(varOld));
		}
	}
	if (m_planChecker.Changed())
	{
		// (j.jones 2005-09-12 16:01) - this is always requeried in the Load() further down
	}
	if (m_inscontactChecker.Changed())
	{
		_variant_t varOld;
		if(m_ContactsCombo->GetCurSel()!=-1)
			varOld = m_ContactsCombo->GetValue(m_ContactsCombo->GetCurSel(),0);
		
		m_ContactsCombo->Requery();

		if(varOld.vt!=VT_NULL)
			m_ContactsCombo->TrySetSelByColumn(0,varOld);
	}
}

void CInsuranceDlg::Load()
{
	_RecordsetPtr rs;
	CString sql;
	CWaitCursor pwait;
	_variant_t var;

	try{

		// (a.walling 2010-10-13 11:08) - PLID 40978
		HandleTableCheckers();

		// (j.jones 2014-08-08 13:34) - PLID 63250 - we no longer have a ConfigRT tablechecker,
		// these properties are cached, so just update the member variables from the cache
		m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true);
		m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

		// (m.hancock 2006-11-02 09:58) - PLID 21274 - Added Deductible to query for loading.
		// (j.gruber 2006-12-29 15:14) - PLID 23972 - take Deductible out
		//TES 6/8/2007 - PLID 26257 - Added NSFCode, need it to know whether to enable the Secondary Reason Code.  Also,
		// added the Secondary Reason Code.
		// (j.jones 2008-09-09 11:25) - PLID 18695 - NSF Code is now InsType
		//DRT 10/21/2008 - PLID 31774 - Parameterized
		// (j.jones 2009-03-06 09:35) - PLID 28834 - added accident data
		// (j.jones 2009-06-23 11:39) - PLID 34689 - added ability to submit as primary, when not actually primary
		// (j.jones 2010-05-18 11:12) - PLID 37788 - added Patient_IDForInsurance
		// (j.gruber 2010-08-02 09:19) - PLID 39729 - take out old copay fields, add new
		// (j.jones 2012-10-24 14:29) - PLID 36305 - added Title
		// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
		// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading Insurance dialog
		rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuredPartyT.PersonID AS ID, InsuredPartyT.RespTypeID, PersonT.[First], PersonT.Middle, PersonT.[Last], PersonT.Title, "
			"PersonT.Address1, PersonT.Address2, PersonT.City, PersonT.State, PersonT.Zip, PersonT.Country, PersonT.HomePhone, PersonT.BirthDate, "
			"PersonT.Gender, PersonT.SocialSecurity AS SSN, InsuredPartyT.Employer, InsuredPartyT.PolicyGroupNum, InsuredPartyT.IDForInsurance, InsuredPartyT.Patient_IDForInsurance, "
			"InsuredPartyT.InsuranceCoID, PersonT.Note, InsuredPartyT.RelationToPatient, PersonT_4.[First] AS ContactFirstName, "
			"PersonT_4.[Last] AS ContactLastName, PersonT_3.Address1 AS BillingAddress, PersonT_3.City AS BillingCity, "
			"PersonT_3.State AS BillingState, PersonT_3.Zip AS BillingPostalCode, PersonT_4.WorkPhone AS BillingPhone, "
			"PersonT_4.Extension AS BillingExt, PersonT_4.Fax AS BillingFax, PersonT_3.Note AS BillingNotes, "
			"InsurancePlansT.PlanType, InsuredPartyT.InsPlan, InsuredPartyT.[ExpireDate], InsuredPartyT.EffectiveDate, "
			"InsuredPartyT.InsuranceContactID, "
			"InsuranceCoT.InsType, InsuredPartyT.SecondaryReasonCode, "
			"InsuredPartyT.DateOfCurAcc, InsuredPartyT.AccidentType, InsuredPartyT.AccidentState, "
			"InsuredPartyT.SubmitAsPrimary "
			"FROM (InsuredPartyT LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
			"LEFT JOIN (SELECT * FROM PersonT) AS PersonT_1 ON InsuredPartyT.PersonID = PersonT_1.ID "
			"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"LEFT JOIN (SELECT * FROM PersonT) AS PersonT_2 ON InsuranceCoT.PersonID = PersonT_2.ID) "
			"LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID "
			"LEFT JOIN (SELECT * FROM PersonT) AS PersonT_3 ON InsuredPartyT.InsuranceCoID = PersonT_3.[ID] "
			"LEFT JOIN (SELECT * FROM PersonT) AS PersonT_4 ON InsuredPartyT.InsuranceContactID = PersonT_4.ID "
			"WHERE (((InsuredPartyT.PersonID)={INT})) ORDER BY InsuredPartyT.PersonID", m_CurrentID);
		m_bSettingBox = true;
		if (rs->eof)
		{	SetDlgItemText (IDC_FIRST_NAME_BOX,	"");
			SetDlgItemText (IDC_MIDDLE_NAME_BOX,"");
			SetDlgItemText (IDC_LAST_NAME_BOX,"");
			// (j.jones 2012-10-24 14:29) - PLID 36305 - added Title
			SetDlgItemText (IDC_TITLE_BOX, "");
			SetDlgItemText (IDC_ADDRESS1_BOX,"");
			SetDlgItemText (IDC_ADDRESS2_BOX,"");
			SetDlgItemText (IDC_CITY_BOX,"");
			SetDlgItemText (IDC_STATE_BOX,"");
			SetDlgItemText (IDC_ZIP_BOX,"");
			// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
			m_CountryList->SetSelByColumn(cccName, "");
			SetDlgItemText (IDC_EMPLOYER_SCHOOL_BOX,"");
			SetDlgItemText (IDC_INS_PARTY_SSN,"");
			
			m_nxtBirthDate->Clear();
			//(e.lally 2006-09-14) PLID 22530 - This still not does work and was missed as
			//a part of 20116, making it possible to re-save the birthdate as null before it is loaded.
			//m_dtBirthDate.SetDateTime(0,0,0,0,0,0);
			m_dtBirthDate.m_dt = 0;
			m_dtBirthDate.SetStatus(COleDateTime::invalid);
			SetDlgItemText (IDC_PHONE_BOX,"");		
			SetDlgItemText (IDC_MEMO_BOX,"");		
			SetDlgItemText (IDC_INSURANCE_ID_BOX,"");
			// (j.jones 2010-05-18 11:12) - PLID 37788 - added Patient_IDForInsurance
			SetDlgItemText (IDC_PATIENT_INSURANCE_ID_BOX,"");
			SetDlgItemText (IDC_TYPE_BOX,"");
			SetDlgItemText (IDC_FECA_BOX,"");
			
			//TES 6/8/2007 - PLID 26257 - Disable the Secondary Reason Code (since it is not the case that a 
			// non-primary Medicare party is selected).
			GetDlgItem(IDC_SECONDARY_REASON_CODE)->EnableWindow(FALSE);
			m_pSecondaryReasonCode->CurSel = NULL;

			// (j.jones 2009-06-23 11:39) - PLID 34689 - the ability to submit as primary is
			// only allowed if the insured party is not primary
			m_checkSubmitAsPrimary.EnableWindow(FALSE);
			m_checkSubmitAsPrimary.SetCheck(FALSE);

			SetDlgItemText (IDC_CONTACT_FIRST_BOX,"");
			SetDlgItemText (IDC_CONTACT_LAST_BOX,"");
			SetDlgItemText (IDC_CONTACT_ADDRESS_BOX,"");
			SetDlgItemText (IDC_CONTACT_CITY_BOX,"");
			SetDlgItemText (IDC_CONTACT_STATE_BOX,"");
			SetDlgItemText (IDC_CONTACT_ZIP_BOX,"");
			SetDlgItemText (IDC_CONTACT_PHONE_BOX,"");
			SetDlgItemText (IDC_CONTACT_FAX_BOX,"");
			SetDlgItemText (IDC_CO_MEMO,"");
			// (z.manning, 02/05/2007) - PLID 24581 - Don't forget the contact's extension.
			SetDlgItemText (IDC_CONTACT_EXT_BOX,"");

			// (j.jones 2009-03-06 09:36) - PLID 28834 - added accident controls
			m_nxtAccidentDate->Clear();
			m_dtAccidentDate.m_dt = 0;
			m_dtAccidentDate.SetStatus(COleDateTime::invalid);
			m_radioEmploymentAcc.SetCheck(FALSE);
			m_radioAutoAcc.SetCheck(FALSE);
			m_radioOtherAcc.SetCheck(FALSE);
			m_radioNoneAcc.SetCheck(TRUE);
			SetDlgItemText(IDC_EDIT_ACC_STATE, "");

			m_nxtEffectiveDate->Clear();
			//(e.lally 2006-09-14) PLID 22530 - This still not does work and was missed as
			//a part of 20116, I am changing it to avoid similar problems as the birthdate.
			m_dtEffectiveDate.m_dt = 0;
			m_dtEffectiveDate.SetStatus(COleDateTime::invalid);

			m_nxtInactiveDate->Clear();
			//(e.lally 2006-09-14) PLID 22530 - This still not does work and was missed as
			//a part of 20116, I am changing it to avoid similar problems as the birthdate.
			m_dtInactiveDate.m_dt = 0;
			m_dtInactiveDate.SetStatus(COleDateTime::invalid);

			/*SetDlgItemText (IDC_COPAY,"");
			m_checkWarnCoPay.SetCheck(FALSE);
			m_radioCopayAmount.SetCheck(FALSE);
			m_radioCopayPercent.SetCheck(FALSE);*/

			m_CompanyCombo->CurSel = -1;
			m_ContactsCombo->CurSel = -1;
			m_PlanList->CurSel = -1;
			m_PlanList->Clear();
			m_RelateCombo->CurSel = -1;
			// (j.jones 2010-05-18 11:24) - PLID 37788 - if "Self", the "Patient ID For Insurance" field is worthless
			GetDlgItem(IDC_PATIENT_INSURANCE_ID_BOX)->EnableWindow(FALSE);

			m_GenderCombo->CurSel = -1;

			// (m.hancock 2006-11-02 09:46) - PLID 21274 - Adding deductible field to insurance tab
			// (j.gruber 2006-12-29 15:14) - PLID 23972 - take this out
			//SetDlgItemText (IDC_DEDUCTIBLE,"");

		}
		else
		{	SetDlgItemText (IDC_FIRST_NAME_BOX,		CString(rs->Fields->Item["First"]->Value.bstrVal));
			SetDlgItemText (IDC_MIDDLE_NAME_BOX,		CString(rs->Fields->Item["Middle"]->Value.bstrVal));
			SetDlgItemText (IDC_LAST_NAME_BOX,		CString(rs->Fields->Item["Last"]->Value.bstrVal));
			// (j.jones 2012-10-24 14:29) - PLID 36305 - added Title
			SetDlgItemText (IDC_TITLE_BOX,			VarString(rs->Fields->Item["Title"]->Value, ""));
			SetDlgItemText (IDC_ADDRESS1_BOX,		CString(rs->Fields->Item["Address1"]->Value.bstrVal));
			SetDlgItemText (IDC_ADDRESS2_BOX,		CString(rs->Fields->Item["Address2"]->Value.bstrVal));
			SetDlgItemText (IDC_CITY_BOX,			CString(rs->Fields->Item["City"]->Value.bstrVal));
			SetDlgItemText (IDC_STATE_BOX,			CString(rs->Fields->Item["State"]->Value.bstrVal));
			SetDlgItemText (IDC_ZIP_BOX,				CString(rs->Fields->Item["Zip"]->Value.bstrVal));
			// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
			m_CountryList->SetSelByColumn(cccName, (LPCTSTR)VarString(rs->Fields->Item["Country"]->Value, ""));
			SetDlgItemText (IDC_EMPLOYER_SCHOOL_BOX,	CString(rs->Fields->Item["Employer"]->Value.bstrVal));
			
			// (f.dinatale 2011-02-02) - PLID 42260 - Format the SSN properly when it's loaded.
			CString strSocialSecurity = CString(rs->Fields->Item["SSN"]->Value.bstrVal);
			if(CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
				strSocialSecurity = FormatSSNText(strSocialSecurity, eSSNNoMask, "###-##-####");
			} else {
				if(CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && !CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
					strSocialSecurity = FormatSSNText(strSocialSecurity, eSSNPartialMask, "###-##-####");
				} else {
					strSocialSecurity = FormatSSNText(strSocialSecurity, eSSNFullMask, "###-##-####");
				}
			}
			// (f.dinatale 2011-02-09) - PLID 42260 - Set the formatting boolean so that the event handler doesn't get fired.
			m_bFormattingField = true;
			SetDlgItemText(IDC_INS_PARTY_SSN, strSocialSecurity);
			m_bFormattingField = false;

			SetDlgItemVar (IDC_BIRTH_DATE_BOX,		rs->Fields->Item["BirthDate"]->Value);
			SetDlgItemText (IDC_PHONE_BOX,			CString(rs->Fields->Item["HomePhone"]->Value.bstrVal));
			SetDlgItemText (IDC_MEMO_BOX,			CString(rs->Fields->Item["Note"]->Value.bstrVal));
			SetDlgItemText (IDC_INSURANCE_ID_BOX,	CString(rs->Fields->Item["IDForInsurance"]->Value.bstrVal));
			// (j.jones 2010-05-18 11:12) - PLID 37788 - added Patient_IDForInsurance
			SetDlgItemText (IDC_PATIENT_INSURANCE_ID_BOX,	AdoFldString(rs, "Patient_IDForInsurance", ""));
			SetDlgItemText (IDC_FECA_BOX,			CString(rs->Fields->Item["PolicyGroupNum"]->Value.bstrVal));

			SetDlgItemText (IDC_CONTACT_FIRST_BOX,	CString(rs->Fields->Item["ContactFirstName"]->Value.bstrVal));
			SetDlgItemText (IDC_CONTACT_LAST_BOX,	CString(rs->Fields->Item["ContactLastName"]->Value.bstrVal));
			SetDlgItemText (IDC_CONTACT_ADDRESS_BOX,	CString(rs->Fields->Item["BillingAddress"]->Value.bstrVal));
			SetDlgItemText (IDC_CONTACT_CITY_BOX,	CString(rs->Fields->Item["BillingCity"]->Value.bstrVal));
			SetDlgItemText (IDC_CONTACT_STATE_BOX,	CString(rs->Fields->Item["BillingState"]->Value.bstrVal));
			SetDlgItemText (IDC_CONTACT_ZIP_BOX,		CString(rs->Fields->Item["BillingPostalCode"]->Value.bstrVal));
			SetDlgItemText (IDC_CONTACT_PHONE_BOX,	CString(rs->Fields->Item["BillingPhone"]->Value.bstrVal));
			SetDlgItemText (IDC_CONTACT_EXT_BOX, CString(rs->Fields->Item["BillingExt"]->Value.bstrVal));
			SetDlgItemText (IDC_CONTACT_FAX_BOX,	CString(rs->Fields->Item["BillingFax"]->Value.bstrVal));
			SetDlgItemText (IDC_CO_MEMO,				CString(rs->Fields->Item["BillingNotes"]->Value.bstrVal));

			// (j.jones 2009-03-06 09:36) - PLID 28834 - added accident controls
			var = rs->Fields->Item["DateOfCurAcc"]->Value;
			if(var.vt == VT_DATE) {
				m_dtAccidentDate = VarDateTime(var);
				m_nxtAccidentDate->SetDateTime(m_dtAccidentDate);				
			}
			else {
				m_dtAccidentDate.m_dt = 0;
				m_dtAccidentDate.SetStatus(COleDateTime::invalid);
				m_nxtAccidentDate->Clear();
			}

			InsuredPartyAccidentType ipatAccidentType = (InsuredPartyAccidentType)AdoFldLong(rs, "AccidentType", ipatNone);
			m_radioEmploymentAcc.SetCheck(ipatAccidentType == ipatEmployment);
			m_radioAutoAcc.SetCheck(ipatAccidentType == ipatAutoAcc);
			m_radioOtherAcc.SetCheck(ipatAccidentType == ipatOtherAcc);
			m_radioNoneAcc.SetCheck(ipatAccidentType == ipatNone);
			
			SetDlgItemText(IDC_EDIT_ACC_STATE, AdoFldString(rs, "AccidentState", ""));

			var = rs->Fields->Item["BirthDate"]->Value;
			if(var.vt == VT_DATE) {
				m_dtBirthDate = VarDateTime(var);
				m_nxtBirthDate->SetDateTime(m_dtBirthDate);
			}
			else {
				//m_dtBirthDate.SetDateTime(0,0,0,0,0,0); // (c.haag 2006-04-14 10:13) - PLID 20116 - This doesn't do anything!
				m_dtBirthDate.m_dt = 0;
				m_dtBirthDate.SetStatus(COleDateTime::invalid);
				m_nxtBirthDate->Clear();
			}

			var = rs->Fields->Item["EffectiveDate"]->Value;
			if(var.vt == VT_DATE) {
				m_dtEffectiveDate = VarDateTime(var);
				m_nxtEffectiveDate->SetDateTime(m_dtEffectiveDate);				
			}
			else {
				//m_dtEffectiveDate.SetDateTime(0,0,0,0,0,0);  // (c.haag 2006-04-14 10:13) - PLID 20116 - This doesn't do anything!
				//(e.lally 2006-09-14) PLID 22530 - This still not does work and was missed as
				//a part of 20116, I am changing it to avoid similar problems as the birthdate.
				m_dtEffectiveDate.m_dt = 0;
				m_dtEffectiveDate.SetStatus(COleDateTime::invalid);
				m_nxtEffectiveDate->Clear();
			}
			var = rs->Fields->Item["ExpireDate"]->Value;
			if(var.vt == VT_DATE) {
				m_dtInactiveDate = VarDateTime(var);
				m_nxtInactiveDate->SetDateTime(m_dtInactiveDate);
			}
			else {
				//(e.lally 2006-09-14) PLID 22530 - This still not does work and was missed as
				//a part of 20116, I am changing it to avoid similar problems as the birthdate.
				//m_dtInactiveDate.SetDateTime(0,0,0,0,0,0);
				m_dtInactiveDate.m_dt = 0;
				m_dtInactiveDate.SetStatus(COleDateTime::invalid);
				m_nxtInactiveDate->Clear();
			}


			// (j.jones 2006-04-12 12:01) - PLID 9749 - we support copay percentages or flat amounts

			/*long nCopayType = AdoFldLong(rs, "CopayType",0);
			
			if(nCopayType == 0) {
				//flat amount
				m_radioCopayAmount.SetCheck(TRUE);
				m_radioCopayPercent.SetCheck(FALSE);

				CString strCopay = FormatCurrencyForInterface(COleCurrency(0,0));

				var = rs->Fields->Item["Copay"]->Value;
				if(var.vt == VT_CY) {
					COleCurrency cyCoPay;
					cyCoPay = var.cyVal;
					if(cyCoPay.m_status!=COleCurrency::invalid) {
						strCopay = FormatCurrencyForInterface(cyCoPay);
					}
				}
				SetDlgItemText(IDC_COPAY, strCopay);
			}
			else {
				//percent amount
				m_radioCopayAmount.SetCheck(FALSE);
				m_radioCopayPercent.SetCheck(TRUE);

				CString strCopay = "0%";

				long nPercent = AdoFldLong(rs, "CopayPercent",0);
				strCopay.Format("%li%%",nPercent);

				SetDlgItemText(IDC_COPAY, strCopay);
			}
			
			m_checkWarnCoPay.SetCheck(AdoFldBool(rs, "WarnCopay",FALSE));*/

			var = rs->Fields->Item["PlanType"]->Value;
			if(var.vt != VT_NULL)
				SetDlgItemText (IDC_TYPE_BOX,CString(var.bstrVal));
			else
				SetDlgItemText (IDC_TYPE_BOX,"");


			long InsCoID;
			var = rs->Fields->Item["InsuranceCoID"]->Value;
			if(var.vt!=VT_NULL) {
				InsCoID = var.lVal;
				SetCompanyWithInactive(InsCoID);
				CString strInsCoWhere;
				strInsCoWhere.Format("InsuranceCoID = %li",InsCoID);
				m_ContactsCombo->WhereClause = _bstr_t(strInsCoWhere);
				m_ContactsCombo->Requery();
			}

			long InsContactID;
			var = rs->Fields->Item["InsuranceContactID"]->Value;
			if(var.vt!=VT_NULL) {
				InsContactID = var.lVal;			
				m_ContactsCombo->TrySetSelByColumn(0,(long)InsContactID);
			}

			m_RelateCombo->SetSelByColumn(0,rs->Fields->Item["RelationToPatient"]->Value);

			// (j.jones 2010-05-18 11:24) - PLID 37788 - if "Self", the "Patient ID For Insurance" field is worthless
			BOOL bPerm = (GetCurrentUserPermissions(bioPatientInsurance) & SPT___W_______);
			GetDlgItem(IDC_PATIENT_INSURANCE_ID_BOX)->EnableWindow(bPerm && AdoFldString(rs, "RelationToPatient", "") != "Self");

			var = rs->Fields->Item["Gender"]->Value;
			if (var.vt != VT_NULL)
			{	
				if (VarByte(var,0) == 1)
					m_GenderCombo->CurSel = 1;
				else if (VarByte(var,0) == 2)
					m_GenderCombo->CurSel = 2;
				else
					m_GenderCombo->CurSel = 0;
			}
			else
				m_GenderCombo->CurSel = 0;

			//Plan Combo
			CString tmpSQL;
			tmpSQL.Format("InsCoID = %i",m_lastIns = InsCoID);
			m_PlanList->WhereClause = _bstr_t(tmpSQL);
			//TES 12/23/2003: Sometimes Load is called twice in rapid succession; in that case, these requeries can mess
			//each other up.
			if(!m_PlanList->DropDownState && !m_bInsPlanIsRequerying) {
				m_bInsPlanIsRequerying = true;
				m_PlanList->Requery();
			}

			var = rs->Fields->Item["InsPlan"]->Value;
			if(var.vt!=VT_NULL && var.lVal > 0)
				m_PlanList->SetSelByColumn(0,var);

			long nRespTypeID = AdoFldLong(rs, "RespTypeID");
			m_listRespType->SetSelByColumn(0, nRespTypeID);

			//TES 6/8/2007 - PLID 26257 - If we are on a non-primary party, whose NSFCode is "C" (Medicare), then we
			// should enable the Secondary Type Code.
			// (j.jones 2008-09-09 11:26) - PLID 18695 - this is now InsType, checking the Medicare type
			// (j.jones 2010-10-15 14:49) - PLID 40953 - added Part A as well
			InsuranceTypeCode eInsType = (InsuranceTypeCode)AdoFldLong(rs, "InsType", (long)itcInvalid);			
			if(nRespTypeID != 1 && (eInsType == itcMedicarePartB || eInsType == itcMedicarePartA)) {
				BOOL bPerm = (GetCurrentUserPermissions(bioPatientInsurance) & SPT___W_______);
				GetDlgItem(IDC_SECONDARY_REASON_CODE)->EnableWindow(bPerm);
				//TES 6/8/2007 - PLID 26257 - And we'll set the actual code, of course.
				m_pSecondaryReasonCode->SetSelByColumn(0, rs->Fields->GetItem("SecondaryReasonCode")->Value);
			}
			else {
				GetDlgItem(IDC_SECONDARY_REASON_CODE)->EnableWindow(FALSE);
				m_pSecondaryReasonCode->CurSel = NULL;
			}

			// (j.jones 2009-06-23 11:39) - PLID 34689 - the ability to submit as primary is
			// only allowed if the insured party is not primary
			if(nRespTypeID == 1) {
				m_checkSubmitAsPrimary.EnableWindow(FALSE);
				m_checkSubmitAsPrimary.SetCheck(TRUE);
			}
			else {
				BOOL bPerm = (GetCurrentUserPermissions(bioPatientInsurance) & SPT___W_______);
				m_checkSubmitAsPrimary.EnableWindow(bPerm);
				m_checkSubmitAsPrimary.SetCheck(AdoFldBool(rs, "SubmitAsPrimary", FALSE));
			}
		}

		// (j.jones 2012-01-26 09:03) - PLID 47787 - call UpdateDeductibleOOPLabel to
		// color the button red if there is content in use
		UpdateDeductibleOOPLabel();


		// (j.gruber 2010-08-02 13:41) - PLID 39729 - setup the new copay box
		CString strFrom;
		strFrom.Format("ServicePayGroupsT LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID AND InsuredPartyPayGroupsT.InsuredPartyID = %li ", m_CurrentID);
		m_pPayGroupsList->FromClause = _bstr_t(strFrom);
		m_pPayGroupsList->Requery();

		m_ForceRefresh = false;
	} NxCatchAll("Error Loading Insurance: ");

	m_changed = m_bSettingBox = false;
}

void CInsuranceDlg::Save(int nID)
{
	//CString sql;
	//_RecordsetPtr rs;

	try {
	// (c.haag 2006-04-13 08:49) - PLID 20116 - Stop doing this!
	//sql.Format(_T("SELECT InsuredPartyT.PersonID AS ID FROM InsuredPartyT WHERE InsuredPartyT.PatientID=%li"),m_id);
	//rs = CreateRecordsetStd(sql);
	//if(rs->eof) return;

	//Some of us don't care about m_changed!
	// (c.haag 2006-04-14 09:43) - PLID 20116 - Yes we do care!
	//if(nID != IDC_BIRTH_DATE_BOX && nID != IDC_EFFECTIVE_DATE) {
	//	if (!m_changed)
	//		return;
	//}

	if (!m_changed) {
		return;
	}

	m_changed = false;

	//rs->Close();
	CString value;

	// for auditing
	long nAuditID;
	CString sql;
	CString strCurrent;
	_RecordsetPtr rs;

	m_bFormattingField = true;
	GetDlgItemText (nID, value);
	value.TrimRight();
	value.TrimLeft();
	SetDlgItemText (nID, value); // to reflect the value we're actually storing (post-trimming)
	m_bFormattingField = false;

	switch (nID) 
	{	case IDC_FIRST_NAME_BOX:

			// Get previous information
			sql.Format ("SELECT First FROM PersonT WHERE ID = %li",m_CurrentID);
			rs = CreateRecordsetStd(sql);
			if (rs->eof) return; // Can't save data that no longer exists
			strCurrent = AdoFldString(rs, "First");
			rs->Close();

			if (value == strCurrent)
				return;

			ExecuteSql("UPDATE PersonT SET First = '%s' WHERE ID = %li",_Q(value),m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyFirst, m_id, strCurrent, value, aepMedium, aetChanged);

			// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
			UpdatePatientForHL7();

			break;
		case IDC_MIDDLE_NAME_BOX:
			// Get previous information
			sql.Format ("SELECT Middle FROM PersonT WHERE ID = %li",m_CurrentID);
			rs = CreateRecordsetStd(sql);
			if (rs->eof) return; // Can't save data that no longer exists
			strCurrent = AdoFldString(rs, "Middle");
			rs->Close();

			if (value == strCurrent)
				return;

			ExecuteSql("UPDATE PersonT SET Middle = '%s' WHERE ID = %li",_Q(value),m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyMiddle, m_id, strCurrent, value, aepMedium, aetChanged);

			// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
			UpdatePatientForHL7();

			break;
		case IDC_LAST_NAME_BOX:
			// Get previous information
			sql.Format ("SELECT Last FROM PersonT WHERE ID = %li",m_CurrentID);
			rs = CreateRecordsetStd(sql);
			if (rs->eof) return; // Can't save data that no longer exists
			strCurrent = AdoFldString(rs, "Last");
			rs->Close();

			if (value == strCurrent)
				return;

			ExecuteSql("UPDATE PersonT SET Last = '%s' WHERE ID = %li",_Q(value),m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyLast, m_id, strCurrent, value, aepMedium, aetChanged);

			// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
			UpdatePatientForHL7();

			break;

		// (j.jones 2012-10-24 14:29) - PLID 36305 - added Title
		case IDC_TITLE_BOX:
			// Get previous information
			rs = CreateParamRecordset("SELECT Title FROM PersonT WHERE ID = {INT}", m_CurrentID);
			if (rs->eof) return; // Can't save data that no longer exists
			strCurrent = AdoFldString(rs, "Title");
			rs->Close();

			if (value == strCurrent)
				return;

			ExecuteParamSql("UPDATE PersonT SET Title = {STRING} WHERE ID = {INT}", value, m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyTitle, m_id, strCurrent, value, aepMedium, aetChanged);

			UpdatePatientForHL7();

			break;
		case IDC_ADDRESS1_BOX:
			// Get previous information
			sql.Format ("SELECT Address1 FROM PersonT WHERE ID = %li",m_CurrentID);
			rs = CreateRecordsetStd(sql);
			if (rs->eof) return; // Can't save data that no longer exists
			strCurrent = AdoFldString(rs, "Address1");
			rs->Close();

			if (value == strCurrent)
				return;

			ExecuteSql("UPDATE PersonT SET Address1 = '%s' WHERE ID = %li",_Q(value),m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyAddress1, m_id, strCurrent, value, aepMedium, aetChanged);

			// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
			UpdatePatientForHL7();

			break;
		case IDC_ADDRESS2_BOX:
			// Get previous information
			sql.Format ("SELECT Address2 FROM PersonT WHERE ID = %li",m_CurrentID);
			rs = CreateRecordsetStd(sql);
			if (rs->eof) return; // Can't save data that no longer exists
			strCurrent = AdoFldString(rs, "Address2");
			rs->Close();

			if (value == strCurrent)
				return;

			ExecuteSql("UPDATE PersonT SET Address2 = '%s' WHERE ID = %li",_Q(value),m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyAddress2, m_id, strCurrent, value, aepMedium, aetChanged);

			// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
			UpdatePatientForHL7();

			break;
		case IDC_CITY_BOX:
					if (m_bLookupByCity) {

						CString zip, state, tempZip, tempState, value;
						CString strCurrentCity, strCurrentState, strCurrentZip;
						GetDlgItemText(IDC_CITY_BOX, value);
						GetDlgItemText(IDC_ZIP_BOX, tempZip);
						GetDlgItemText(IDC_STATE_BOX, tempState);
						tempZip.TrimRight(); tempZip.TrimLeft();
						tempState.TrimRight(); tempState.TrimLeft();
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
							else zip = tempZip;
							if(tempState == "")
								SetDlgItemText(IDC_STATE_BOX, state);
							else state = tempState;

							// Get previous information
							sql.Format ("SELECT City,State,Zip FROM PersonT WHERE ID = %li",m_CurrentID);
							rs = CreateRecordsetStd(sql);
							strCurrentCity = AdoFldString(rs, "City");
							strCurrentState = AdoFldString(rs, "State");
							strCurrentZip = AdoFldString(rs, "Zip");
							rs->Close();

							if (value == strCurrentCity)
								break;

							ExecuteSql("UPDATE PersonT SET City = '%s', Zip = '%s', State = '%s' WHERE ID = %li",_Q(value),_Q(zip),_Q(state),m_CurrentID);

							// Audit it
							nAuditID = BeginNewAuditEvent();
							if (nAuditID != -1)
								AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyCity, m_id, strCurrentCity, value, aepMedium, aetChanged);

							nAuditID = BeginNewAuditEvent();
							if (nAuditID != -1)
								AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyState, m_id, strCurrentState, state, aepMedium, aetChanged);

							nAuditID = BeginNewAuditEvent();
							if (nAuditID != -1)
								AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyZip, m_id, strCurrentZip, zip, aepMedium, aetChanged);

						}
						else
						{
							// Get previous information
							sql.Format ("SELECT City,State,Zip FROM PersonT WHERE ID = %li",m_CurrentID);
							rs = CreateRecordsetStd(sql);
							strCurrentCity = AdoFldString(rs, "City");
							rs->Close();

							if (value == strCurrentCity)
								break;

							ExecuteSql("UPDATE PersonT SET City = '%s' WHERE ID = %li",_Q(value),m_CurrentID);

							// Audit it
							nAuditID = BeginNewAuditEvent();
							if (nAuditID != -1)
								AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyCity, m_id, strCurrentCity, value, aepMedium, aetChanged);
						}
						
						// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
						UpdateExistingPatientInHL7(m_id, TRUE);

					}
					else {
			
						// Get previous information
						sql.Format ("SELECT City FROM PersonT WHERE ID = %li",m_CurrentID);
						rs = CreateRecordsetStd(sql);
						if (rs->eof) return; // Can't save data that no longer exists
						strCurrent = AdoFldString(rs, "City");
						rs->Close();

						if (value == strCurrent)
							return;

						ExecuteSql("UPDATE PersonT SET City = '%s' WHERE ID = %li",_Q(value),m_CurrentID);

						// Audit it
						nAuditID = BeginNewAuditEvent();
						if (nAuditID != -1)
							AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyCity, m_id, strCurrent, value, aepMedium, aetChanged);

						// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
						UpdatePatientForHL7();
					}
			break;
		case IDC_STATE_BOX:
			// Get previous information
			sql.Format ("SELECT State FROM PersonT WHERE ID = %li",m_CurrentID);
			rs = CreateRecordsetStd(sql);
			if (rs->eof) return; // Can't save data that no longer exists
			strCurrent = AdoFldString(rs, "State");
			rs->Close();

			if (value == strCurrent)
				return;

			ExecuteSql("UPDATE PersonT SET State = '%s' WHERE ID = %li",_Q(value),m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyState, m_id, strCurrent, value, aepMedium, aetChanged);
			
			// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
			UpdatePatientForHL7();

			break;

		case IDC_ZIP_BOX:
			{	if(m_CurrentID == -1)
					break;				

				try {
					if (!m_bLookupByCity) {

						CString city, state, tempCity, tempState, value, strTempZip;
						CString strCurrentCity, strCurrentState, strCurrentZip;

						GetDlgItemText(IDC_ZIP_BOX, value);
						GetDlgItemText(IDC_CITY_BOX, tempCity);
						GetDlgItemText(IDC_STATE_BOX, tempState);
						tempCity.TrimRight(); tempCity.TrimLeft();
						tempState.TrimRight(); tempCity.TrimLeft();
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
							// (s.tullis 2013-10-21 15:18) - PLID 45031 - If 9-digit zipcode match fails compair it with the 5-digit zipcode.
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

							// Get previous information
							sql.Format ("SELECT City,State,Zip FROM PersonT WHERE ID = %li",m_CurrentID);
							rs = CreateRecordsetStd(sql);
							strCurrentCity = AdoFldString(rs, "City");
							strCurrentState = AdoFldString(rs, "State");
							strCurrentZip = AdoFldString(rs, "Zip");
							rs->Close();

							if (value == strCurrentZip)
								break;

							ExecuteSql("UPDATE PersonT SET Zip = '%s', City = '%s', State = '%s' WHERE ID = %li",_Q(value),_Q(city),_Q(state),m_CurrentID);

							// Audit it
							nAuditID = BeginNewAuditEvent();
							if (nAuditID != -1 && strCurrentCity != city)
								AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyCity, m_id, strCurrentCity, city, aepMedium, aetChanged);

							nAuditID = BeginNewAuditEvent();
							if (nAuditID != -1&& strCurrentState != state)
								AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyState, m_id, strCurrentState, state, aepMedium, aetChanged);

							nAuditID = BeginNewAuditEvent();
							if (nAuditID != -1 && strCurrentZip != value){
								AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyZip, m_id, strCurrentZip, value, aepMedium, aetChanged);
							}

						}
						else
						{
							// Get previous information
							sql.Format ("SELECT City,State,Zip FROM PersonT WHERE ID = %li",m_CurrentID);
							rs = CreateRecordsetStd(sql);
							strCurrentZip = AdoFldString(rs, "Zip");
							rs->Close();

							if (value == strCurrentZip)
								break;

							ExecuteSql("UPDATE PersonT SET Zip = '%s' WHERE ID = %li",_Q(value),m_CurrentID);

							// Audit it
							nAuditID = BeginNewAuditEvent();
							if (nAuditID != -1)
								AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyZip, m_id, strCurrentZip, value, aepMedium, aetChanged);
						}
						
						// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
						UpdateExistingPatientInHL7(m_id, TRUE);

					}
					else {
						// Get previous information
						sql.Format ("SELECT Zip FROM PersonT WHERE ID = %li",m_CurrentID);
						rs = CreateRecordsetStd(sql);
						if (rs->eof) return; // Can't save data that no longer exists
						strCurrent = AdoFldString(rs, "Zip");
						rs->Close();

						if (value == strCurrent)
							return;

						ExecuteSql("UPDATE PersonT SET Zip = '%s' WHERE ID = %li",_Q(value),m_CurrentID);

						// Audit it
						nAuditID = BeginNewAuditEvent();
						if (nAuditID != -1)
							AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyZip, m_id, strCurrent, value, aepMedium, aetChanged);

						// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
						UpdatePatientForHL7();

					}


				} NxCatchAll("InsuranceDlg: Error in saving zip code information");
			break;
			}

		case IDC_EMPLOYER_SCHOOL_BOX:
			// Get previous information
			sql.Format ("SELECT Employer FROM InsuredPartyT WHERE PersonID = %li",m_CurrentID);
			rs = CreateRecordsetStd(sql);
			if (rs->eof) return; // Can't save data that no longer exists
			strCurrent = AdoFldString(rs, "Employer");
			rs->Close();

			if (value == strCurrent)
				return;

			ExecuteSql("UPDATE InsuredPartyT SET Employer = '%s' WHERE PersonID = %li",_Q(value),m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyEmployer, m_id, strCurrent, value, aepMedium, aetChanged);

			// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
			UpdateExistingPatientInHL7(m_id, TRUE);

			break;
		case IDC_INS_PARTY_SSN: {

			// (s.tullis 2016-02-12 10:14) - PLID 68212 
			if (value == "###-##-####") {
				value = "";
				FormatItemText(GetDlgItem(IDC_INS_PARTY_SSN), value, "");
			}

			// (f.dinatale 2011-02-01) - PLID 42260 - If the SSN is not masked, save it.  Otherwise, ignore it so that we don't overwrite the valid SSNs.
			if (CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
				// Get previous information
				sql.Format("SELECT SocialSecurity FROM PersonT WHERE ID = %li", m_CurrentID);
				rs = CreateRecordsetStd(sql);
				if (rs->eof) return; // Can't save data that no longer exists
				strCurrent = AdoFldString(rs, "SocialSecurity");
				rs->Close();

				strCurrent.TrimRight();

				value.Replace("#", " "); //take care of bad characters
				value.TrimRight();

				if (value == strCurrent)
					return;

				ExecuteSql("UPDATE PersonT SET SocialSecurity = '%s' WHERE ID = %li", _Q(value), m_CurrentID);

				// Audit it
				nAuditID = BeginNewAuditEvent();
				if (nAuditID != -1)
					AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartySSN, m_id, strCurrent, value, aepMedium, aetChanged);

				//TES 8/1/2010 - PLID 38838 - This is now included in HL7 exports.
				UpdatePatientForHL7();
			}
			break;
		}

		// m.carlson 6/9/2004 PL 11504 - Since value was retrieved from GetDlgItemText, and GetDlgItemText
		// doesn't work with NxDateTime controls, value is always blank in this 'case:'! So don't use "value"!
		case IDC_BIRTH_DATE_BOX: {
			//if(!m_bSavingBirthDate) {
				//m_bSavingBirthDate = true;

				CString strOldBirthDate, strNewBirthDate;
				_RecordsetPtr rs = CreateRecordset("SELECT BirthDate FROM PersonT WHERE ID = %li",m_CurrentID);
				if (rs->eof) { /*m_bSavingBirthDate = false;*/ return; } // Can't save data that no longer exists
				//for auditing
				_variant_t var = rs->Fields->Item["BirthDate"]->Value;
				if(var.vt == VT_DATE)
					strOldBirthDate = FormatDateTimeForInterface(COleDateTime(var), dtoDate);
				rs->Close();

				if (m_nxtBirthDate->GetStatus() == 3) {
					ExecuteSql("UPDATE PersonT SET BirthDate = NULL WHERE ID = %li",m_CurrentID);
					strNewBirthDate = "";

					// (c.haag 2006-04-14 09:50) - PLID 20116 - Make this consistent with General 1
					m_dtBirthDate.SetDateTime(0,0,0,0,0,0);
					m_dtBirthDate.m_dt = 0;
					m_dtBirthDate.SetStatus(COleDateTime::invalid);
					m_nxtBirthDate->Clear();
				}
				else {
					COleDateTime dt, dttemp, dtNow;
					dttemp.ParseDateTime("01/01/1800");
					dt = m_nxtBirthDate->GetDateTime();
					dtNow = COleDateTime::GetCurrentTime();

					if(dt > dtNow) {
						AfxMessageBox("You have entered a birthdate in the future. This will be adjusted to a valid date.");

						while(dt > dtNow)
							dt.SetDate(dt.GetYear() - 100, dt.GetMonth(), dt.GetDay());
					}

					strNewBirthDate = FormatDateTimeForInterface(dt, dtoDate);
					
					if (strNewBirthDate == strOldBirthDate)
					{
						//m_bSavingBirthDate = false;
						return;
					}

					ExecuteSql("UPDATE PersonT SET BirthDate = '%s' WHERE ID = %li",_Q(FormatDateTimeForSql(dt)),m_CurrentID);
					m_dtBirthDate = dt;
					m_nxtBirthDate->SetDateTime(m_dtBirthDate);		
				}

				if(strOldBirthDate != strNewBirthDate) {
					long nAuditID = -1;
					nAuditID = BeginNewAuditEvent();
					if(nAuditID != -1) 
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyDateofBirth, m_CurrentID, strOldBirthDate, strNewBirthDate, aepMedium, aetChanged);
				}

				// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
				UpdateExistingPatientInHL7(m_id, TRUE);

				//m_bSavingBirthDate = false;
			//}
			return;
			break;
		}
		case IDC_PHONE_BOX:
			// Get previous information
			sql.Format ("SELECT HomePhone FROM PersonT WHERE ID = %li",m_CurrentID);
			rs = CreateRecordsetStd(sql);
			if (rs->eof) return; // Can't save data that no longer exists
			strCurrent = AdoFldString(rs, "HomePhone");
			rs->Close();

			if (value == strCurrent)
				return;

			ExecuteSql("UPDATE PersonT SET HomePhone = '%s' WHERE ID = %li",_Q(value),m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyPhone, m_id, strCurrent, value, aepMedium, aetChanged);

			//TES 8/1/2010 - PLID 38838 - This is now included in HL7 exports.
			UpdatePatientForHL7();

			break;
		case IDC_MEMO_BOX:
			// Get previous information
			sql.Format ("SELECT Note FROM PersonT WHERE ID = %li",m_CurrentID);
			rs = CreateRecordsetStd(sql);
			if (rs->eof) return; // Can't save data that no longer exists
			strCurrent = AdoFldString(rs, "Note");
			rs->Close();

			if (value == strCurrent)
				return;

			ExecuteSql("UPDATE PersonT SET Note = '%s' WHERE ID = %li",_Q(value),m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyNote, m_id, strCurrent, value, aepMedium, aetChanged);

			break;
		case IDC_INSURANCE_ID_BOX:
			// Get previous information
			sql.Format ("SELECT IDForInsurance FROM InsuredPartyT WHERE PersonID = %li",m_CurrentID);
			rs = CreateRecordsetStd(sql);
			if (rs->eof) return; // Can't save data that no longer exists
			strCurrent = AdoFldString(rs, "IDForInsurance");
			rs->Close();

			// (j.jones 2011-06-24 16:49) - PLID 31005 - if they want the ID capitalized, do so now
			if(GetRemotePropertyInt("AutoCapitalizeInsuranceIDs", 0, 0, "<None>", true) == 1) {
				value.MakeUpper();
				SetDlgItemText(nID, value);
			}

			if (value == strCurrent)
				return;

			ExecuteSql("UPDATE InsuredPartyT SET IDForInsurance = '%s' WHERE PersonID = %li",_Q(value),m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyIDForIns, m_id, strCurrent, value, aepMedium, aetChanged);

			// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
			UpdateExistingPatientInHL7(m_id, TRUE);

			break;

		// (j.jones 2010-05-18 11:12) - PLID 37788 - added Patient_IDForInsurance
		case IDC_PATIENT_INSURANCE_ID_BOX:
			// Get previous information
			rs = CreateParamRecordset("SELECT Patient_IDForInsurance FROM InsuredPartyT WHERE PersonID = {INT}",m_CurrentID);
			if (rs->eof) {
				// Can't save data that no longer exists
				return;
			}
			strCurrent = AdoFldString(rs, "Patient_IDForInsurance");
			rs->Close();

			// (j.jones 2011-06-24 16:49) - PLID 31005 - if they want the ID capitalized, do so now
			if(GetRemotePropertyInt("AutoCapitalizeInsuranceIDs", 0, 0, "<None>", true) == 1) {
				value.MakeUpper();
				SetDlgItemText(nID, value);
			}

			if(value == strCurrent) {
				return;
			}

			ExecuteParamSql("UPDATE InsuredPartyT SET Patient_IDForInsurance = {STRING} WHERE PersonID = {INT}", value, m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1) {
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyIDForIns_Patient, m_id, strCurrent, value, aepMedium, aetChanged);
			}

			//this field is not sent to HL7

			break;

		case IDC_FECA_BOX:
			// Get previous information
			sql.Format ("SELECT PolicyGroupNum FROM InsuredPartyT WHERE PersonID = %li",m_CurrentID);
			rs = CreateRecordsetStd(sql);
			if (rs->eof) return; // Can't save data that no longer exists
			strCurrent = AdoFldString(rs, "PolicyGroupNum");
			rs->Close();

			// (j.jones 2011-06-24 16:49) - PLID 31005 - if they want the ID capitalized, do so now
			if(GetRemotePropertyInt("AutoCapitalizeInsuranceIDs", 0, 0, "<None>", true) == 1) {
				value.MakeUpper();
				SetDlgItemText(nID, value);
			}

			if (value == strCurrent)
				return;

			ExecuteSql("UPDATE InsuredPartyT SET PolicyGroupNum = '%s' WHERE PersonID = %li",_Q(value),m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyPolicyGroupNum, m_id, strCurrent, value, aepMedium, aetChanged);

			// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
			UpdateExistingPatientInHL7(m_id, TRUE);

			break;
		// (j.jones 2009-03-06 10:14) - PLID 28834 - added accident state
		case IDC_EDIT_ACC_STATE:
			// Get previous information for auditing
			rs = CreateParamRecordset("SELECT AccidentState FROM InsuredPartyT WHERE PersonID = {INT}", m_CurrentID);
			if(rs->eof) {
				// Can't save data that no longer exists
				return;
			}
			else {
				strCurrent = AdoFldString(rs, "AccidentState");
			}
			rs->Close();

			if(value == strCurrent) {
				return;
			}

			ExecuteParamSql("UPDATE InsuredPartyT SET AccidentState = {STRING} WHERE PersonID = {INT}", value, m_CurrentID);

			// Audit it
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1) {
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyAccidentState, m_id, strCurrent, value, aepMedium, aetChanged);
			}
			break;

		// (j.jones 2009-03-06 09:40) - PLID 28834 - added accident date
		case IDC_EDIT_CUR_ACC_DATE: {
			// Since value was retrieved from GetDlgItemText, and GetDlgItemText doesn't work with NxDateTime controls,
			// value is always blank in this 'case:'!
			if(!m_bSavingAccidentDate) {
				m_bSavingAccidentDate = true;
				
				CString strOldAccidentDate, strNewAccidentDate;

				_RecordsetPtr rs = CreateParamRecordset("SELECT DateOfCurAcc FROM InsuredPartyT WHERE PersonID = {INT}", m_CurrentID);
				if(rs->eof) {
					// Can't save data that no longer exists
					m_bSavingAccidentDate = false;
					return;
				}

				//for auditing
				_variant_t var = rs->Fields->Item["DateOfCurAcc"]->Value;
				if(var.vt == VT_DATE) {
					strOldAccidentDate = FormatDateTimeForInterface(COleDateTime(var), dtoDate);
				}
				rs->Close();

				if (m_nxtAccidentDate->GetStatus() == 3)
				{
					ExecuteParamSql("UPDATE InsuredPartyT SET DateOfCurAcc = NULL WHERE PersonID = {INT}",m_CurrentID);
					strNewAccidentDate = "";

					m_dtAccidentDate.m_dt = 0;
					m_dtAccidentDate.SetStatus(COleDateTime::invalid);
					m_nxtAccidentDate->Clear();
				}
				else {
					COleDateTime dt, dttemp;
					dttemp.ParseDateTime("01/01/1800");
					dt = m_nxtAccidentDate->GetDateTime();
					if(m_nxtAccidentDate->GetStatus() == 1 && dt.m_dt >= dttemp.m_dt) {
						strNewAccidentDate = FormatDateTimeForInterface(dt, dtoDate);
						_variant_t varDate = COleVariant(COleDateTime(dt));
						ExecuteParamSql("UPDATE InsuredPartyT SET DateOfCurAcc = {VT_DATE} WHERE PersonID = {INT}", varDate, m_CurrentID);
						m_dtAccidentDate = dt;
						m_nxtAccidentDate->SetDateTime(m_dtAccidentDate);
					}
				}
				if(strOldAccidentDate != strNewAccidentDate) {
					long nAuditID = -1;
					nAuditID = BeginNewAuditEvent();
					if(nAuditID != -1) 
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyAccidentDate, m_CurrentID, strOldAccidentDate, strNewAccidentDate, aepMedium, aetChanged);
				}
				m_bSavingAccidentDate = false;
			}
			return;
			break;
		}
		case IDC_EFFECTIVE_DATE: {
			// Since value was retrieved from GetDlgItemText, and GetDlgItemText doesn't work with NxDateTime controls,
			// value is always blank in this 'case:'!
			if(!m_bSavingEffectiveDate) {
				m_bSavingEffectiveDate = true;
				
				CString strOldEffectiveDate,strNewEffectiveDate;

				_RecordsetPtr rs = CreateRecordset("SELECT EffectiveDate FROM InsuredPartyT WHERE PersonID = %li",m_CurrentID);
				if (rs->eof) { m_bSavingEffectiveDate = false; return; } // Can't save data that no longer exists
				//for auditing
				_variant_t var = rs->Fields->Item["EffectiveDate"]->Value;
				if(var.vt == VT_DATE)
					strOldEffectiveDate = FormatDateTimeForInterface(COleDateTime(var), dtoDate);
				rs->Close();
				if (m_nxtEffectiveDate->GetStatus() == 3)
				{
					ExecuteSql("UPDATE InsuredPartyT SET EffectiveDate = NULL WHERE PersonID = %li",m_CurrentID);
					strNewEffectiveDate = "";

					// (c.haag 2006-04-14 09:50) - PLID 20116 - Make this consistent with General 1
					m_dtEffectiveDate.SetDateTime(0,0,0,0,0,0);
					m_dtEffectiveDate.m_dt = 0;
					m_dtEffectiveDate.SetStatus(COleDateTime::invalid);
					m_nxtEffectiveDate->Clear();
				}
				else {
					COleDateTime dt, dttemp;
					dttemp.ParseDateTime("01/01/1800");
					dt = m_nxtEffectiveDate->GetDateTime();
					if(m_nxtEffectiveDate->GetStatus() == 1 && dt.m_dt >= dttemp.m_dt) {
						strNewEffectiveDate = FormatDateTimeForInterface(dt, dtoDate);
						ExecuteSql("UPDATE InsuredPartyT SET EffectiveDate = '%s' WHERE PersonID = %li",_Q(FormatDateTimeForSql(dt)),m_CurrentID);
						m_dtEffectiveDate = dt;
						m_nxtEffectiveDate->SetDateTime(m_dtEffectiveDate);
					}
				}
				if(strOldEffectiveDate != strNewEffectiveDate) {
					long nAuditID = -1;
					nAuditID = BeginNewAuditEvent();
					if(nAuditID != -1) 
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyEffectiveDate, m_CurrentID, strOldEffectiveDate, strNewEffectiveDate, aepMedium, aetChanged);
				}
				m_bSavingEffectiveDate = false;

				// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
				UpdatePatientForHL7();
			}
			return;
			break;
		}
		// (c.haag 2006-04-14 09:20) - PLID 20116 - At the present time, we are not supposed to be able to
		// change the inactive date. Josh tells me there is a PL item to allow users to do it.
		//
		// (j.jones 2008-09-10 17:22) - PLID 14002 - The time has come, inactive dates are now editable,
		// for all insurances. It needs to warn when you enter in one that makes an insurance inactive right now,
		// and do so if they agree to continue.
		case IDC_INACTIVE_DATE: {
			if(!m_bSavingInactiveDate) {
				m_bSavingInactiveDate = true;

				CString strOldInactiveDate, strNewInactiveDate;
				COleDateTime dtOldInactiveDate;
				dtOldInactiveDate.SetStatus(COleDateTime::invalid);
				long nCurRespType = -1;

				_RecordsetPtr rs = CreateParamRecordset("SELECT RespTypeID, ExpireDate FROM InsuredPartyT WHERE PersonID = {INT}", m_CurrentID);
				if(rs->eof) {
					// Can't save data that no longer exists
					m_bSavingInactiveDate = false;
					return;
				}
				else {
					nCurRespType = AdoFldLong(rs, "RespTypeID");

					//for auditing
					_variant_t var = rs->Fields->Item["ExpireDate"]->Value;
					if(var.vt == VT_DATE) {
						dtOldInactiveDate = VarDateTime(var);
						strOldInactiveDate = FormatDateTimeForInterface(dtOldInactiveDate, dtoDate);
					}
				}
				rs->Close();

				if (m_nxtInactiveDate->GetStatus() == 3) {

					// (j.jones 2008-09-10 17:26) - PLID 14002 - If the status of this insured party is currently
					// inactive, don't allow them to manually clear the date, instead tell the user they must
					// reassign the insured party to a valid placement (Primary, Secondary, etc.).
					// However if the insured party is not yet inactive, let them clear the date.

					if(nCurRespType == -1) {
						//it is inactive, so reset the date and prompt
						if(dtOldInactiveDate.GetStatus() == COleDateTime::invalid) {
							m_dtInactiveDate.SetDateTime(0,0,0,0,0,0);
							m_dtInactiveDate.m_dt = 0;
							m_dtInactiveDate.SetStatus(COleDateTime::invalid);
							m_nxtInactiveDate->Clear();
						}
						else {
							m_dtInactiveDate = dtOldInactiveDate;
							m_nxtInactiveDate->SetDateTime(m_dtInactiveDate);
						}

						AfxMessageBox("Inactive insured parties cannot have their inactive dates removed. "
							"If you wish to reactivate this insured party, please select a new placement "
							"(Primary, Secondary, etc.) from the list on the upper right of this screen.");

						m_bSavingInactiveDate = false;
						return;
					}

					ExecuteSql("UPDATE InsuredPartyT SET ExpireDate = NULL WHERE PersonID = %li",m_CurrentID);

					strNewInactiveDate = "";

					m_dtInactiveDate.SetDateTime(0,0,0,0,0,0);
					m_dtInactiveDate.m_dt = 0;
					m_dtInactiveDate.SetStatus(COleDateTime::invalid);
					m_nxtInactiveDate->Clear();
				}
				else {

					COleDateTime dt, dttemp;
					dttemp.ParseDateTime("01/01/1800");
					dt = m_nxtInactiveDate->GetDateTime();
					if(m_nxtInactiveDate->GetStatus() == 1 && dt.m_dt >= dttemp.m_dt) {

						COleDateTime dtToday = COleDateTime::GetCurrentTime();
						COleDateTime dtNewInactiveDate = dt;
						dtToday.SetDateTime(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay(), 0, 0, 0);
						dtNewInactiveDate.SetDateTime(dtNewInactiveDate.GetYear(), dtNewInactiveDate.GetMonth(), dtNewInactiveDate.GetDay(), 0, 0, 0);

						// (j.jones 2008-09-10 17:26) - PLID 14002 - If the status of this insured party is currently
						// active, warn them that the insured party will automatically switch to become inactive on the
						// day after it expires. If the date is earlier than today, warn that it will become inactive
						// right now, and mark it inactive right now.
						// However if the insured party is already inactive, let them change the date. If the date is
						// in the future, allow it, but tell them that they should manually assign it to an active resp type.

						if(nCurRespType != -1) {
							//it is not inactive, so first check the date they entered

							if(dtNewInactiveDate > dtToday) {							
								//it's in the future, so warn them what will happen when the inactive date passes
								AfxMessageBox("Since the new inactivation date is in the future, this insured party will automatically "
									"switch to being inactive on that date. It will remain active up until that date.");
							}
							else {
								//it is today or earlier, which means we should be inactivating this insurance now
								if(!InactivateResp(TRUE)) {
									
									//if they cancelled, reset the date and tell them they can't change it
									
									if(dtOldInactiveDate.GetStatus() == COleDateTime::invalid) {
										m_dtInactiveDate.SetDateTime(0,0,0,0,0,0);
										m_dtInactiveDate.m_dt = 0;
										m_dtInactiveDate.SetStatus(COleDateTime::invalid);
										m_nxtInactiveDate->Clear();
									}
									else {
										m_dtInactiveDate = dtOldInactiveDate;
										m_nxtInactiveDate->SetDateTime(m_dtInactiveDate);
									}

									AfxMessageBox("Active insured parties cannot have their inactive dates changed to "
										"today's date or earlier without inactivating the insured party. "
										"The inactive date has been reset.");

									m_bSavingInactiveDate = false;
									return;
								}
							}
						}
						else {
							///it is inactive, is the date in the future?
							if(dtNewInactiveDate > dtToday) {
								AfxMessageBox("Since the new inactivation date is in the future, you may wish to select a new placement "
									"(Primary, Secondary, etc.) from the list on the upper right of this screen. "
									"This insured party will remain inactive unless a new placement is selected.");
							}
						}
					
						strNewInactiveDate = FormatDateTimeForInterface(dt, dtoDate);
						ExecuteSql("UPDATE InsuredPartyT SET ExpireDate = '%s' WHERE PersonID = %li",_Q(FormatDateTimeForSql(dt)),m_CurrentID);
						m_dtInactiveDate = dt;
						m_nxtInactiveDate->SetDateTime(m_dtInactiveDate);
					}
				}
				
				if(strOldInactiveDate != strNewInactiveDate) {
					long nAuditID = -1;
					nAuditID = BeginNewAuditEvent();
					if(nAuditID != -1) {
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyInactiveDate, m_CurrentID, strOldInactiveDate, strNewInactiveDate, aepMedium, aetChanged);
					}
				}

				// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
				UpdatePatientForHL7();

				m_bSavingInactiveDate = false;
			}
			return;
			break;
		}
		/*case IDC_COPAY: {
			// Get previous information
			if(m_radioCopayAmount.GetCheck()) {
				//flat fee

				COleCurrency curCurrent;
				curCurrent.ParseCurrency("$0.00",0);
				sql.Format ("SELECT CoPay FROM InsuredPartyT WHERE PersonID = %li",m_CurrentID);
				rs = CreateRecordsetStd(sql);
				if (rs->eof) return; // Can't save data that no longer exists
				curCurrent = AdoFldCurrency(rs, "CoPay", curCurrent);
				rs->Close();

				if (value == "")
				{
					if (curCurrent.Format() == "$0.00")
						return;

					ExecuteSql("UPDATE InsuredPartyT SET CoPay = NULL WHERE PersonID = %li",m_CurrentID);

					// Audit it
					nAuditID = BeginNewAuditEvent();
					if (nAuditID != -1)
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyCopay, m_id, curCurrent.Format(), value, aepMedium, aetChanged);

					break;
				}
				COleCurrency cyTmp;
				cyTmp = ParseCurrencyFromInterface(value);
				//cyTmp.ParseCurrency(value);
				if (cyTmp.GetStatus() == COleCurrency::invalid) {
					MsgBox("You have entered an invalid amount in the 'Co Pay' box.");
					LoadCopayInfo();
					return;
				}
				CString strNewValue = FormatCurrencyForInterface(cyTmp);
				SetDlgItemText(IDC_COPAY,strNewValue);
				value = FormatCurrencyForSql(cyTmp);

				ExecuteSql("UPDATE InsuredPartyT SET CoPay = Convert(money,'%s') WHERE PersonID = %li",_Q(value),m_CurrentID);

				CString strOldValue = FormatCurrencyForInterface(curCurrent);

				if (strNewValue != strOldValue) {

					// Audit it
					nAuditID = BeginNewAuditEvent();
					if (nAuditID != -1)
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyCopay, m_id, strOldValue, strNewValue, aepMedium, aetChanged);
				}
			}
			else {
				//percentage

				long nCurrent = 0;
				CString strCurrent;
				sql.Format ("SELECT CoPayPercent FROM InsuredPartyT WHERE PersonID = %li",m_CurrentID);
				rs = CreateRecordsetStd(sql);
				if (rs->eof) return; // Can't save data that no longer exists
				nCurrent = AdoFldLong(rs, "CoPayPercent", 0);
				rs->Close();

				strCurrent.Format("%li%%",nCurrent);

				if (value == "")
				{
					if (nCurrent == 0)
						return;

					ExecuteSql("UPDATE InsuredPartyT SET CoPayPercent = 0 WHERE PersonID = %li",m_CurrentID);
					SetDlgItemText(IDC_COPAY,"0%");

					// Audit it
					nAuditID = BeginNewAuditEvent();
					if (nAuditID != -1)
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyCopayPercent, m_id, strCurrent, "0%", aepMedium, aetChanged);

					break;
				}

				CString strNewVal = "";
				for(int i=0;i<value.GetLength();i++) {
					if(value.GetAt(i)>='0' && value.GetAt(i)<='9')
						strNewVal += value.GetAt(i);
				}

				long nNewPercent = atoi(strNewVal);

				if (nNewPercent < 0 || nNewPercent > 100) {
					MsgBox("You have entered an invalid percentage in the 'Co Pay' box.");
					LoadCopayInfo();
					return;
				}

				CString strNewValue;
				strNewValue.Format("%li%%",nNewPercent);
				SetDlgItemText(IDC_COPAY,strNewValue);
				value = strNewValue;

				ExecuteSql("UPDATE InsuredPartyT SET CopayPercent = %li WHERE PersonID = %li",nNewPercent,m_CurrentID);

				if (nNewPercent != nCurrent) {

					// Audit it
					nAuditID = BeginNewAuditEvent();
					if (nAuditID != -1)
						AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyCopayPercent, m_id, strCurrent, strNewValue, aepMedium, aetChanged);
				}
			}

			break;
		}*/

		// (j.gruber 2006-12-29 15:15) - PLID 23972 - take deductible out
		/*case IDC_DEDUCTIBLE: {

			// (m.hancock 2006-11-02 10:15) - PLID 21274 - Save the value of the deductible field.
			
			//Get the new value for deductible
			COleCurrency cyNew = ParseCurrencyFromInterface(value);

			//Check to see if the new value is valid
			if (cyNew.GetStatus() == COleCurrency::invalid) {
				//If value is empty, then the new value is still valid.  So, only display an error if value is not empty.
				if(!value.IsEmpty()) {
					MsgBox("You have entered an invalid amount in the 'Deductible' box.");
					// (m.hancock 2006-11-27 10:41) - PLID 21274 - If deductible is invalid, set the deductible back to $0.00.
					COleCurrency cyZero = COleCurrency(0,0);
					SetDlgItemText(IDC_DEDUCTIBLE, FormatCurrencyForInterface(cyZero));
					return;
				}
			}

			//We already have the previous information stored in m_cyDeductible.  Just compare the current
			//information against the previous information to see if a change has been made.
			if(cyNew != m_cyDeductible) {
				
				CString strNewValue;

				//If value is empty, then display nothing in the edit box and enter NULL into the data.
				if(value.IsEmpty())	{
					strNewValue = "$0.00";
					SetDlgItemText(IDC_DEDUCTIBLE, "");
					value = _Q("NULL");
				}

				//The deductible has changed, so go ahead and update the record.
				else {
					strNewValue = FormatCurrencyForInterface(cyNew);
					SetDlgItemText(IDC_DEDUCTIBLE, strNewValue);
					value.Format("Convert(money,'%s')", _Q(FormatCurrencyForSql(cyNew)));
				}

				ExecuteSql("UPDATE InsuredPartyT SET Deductible = %s WHERE PersonID = %li", value, m_CurrentID);

				// Audit the change
				strCurrent = FormatCurrencyForInterface(m_cyDeductible);
				nAuditID = BeginNewAuditEvent();
				if (nAuditID != -1)
					AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyDeductible, m_id, strCurrent, strNewValue, aepMedium, aetChanged);

			}

			break;
		}*/
		
		//(e.lally 2007-02-20) PLID 23762 - Moved gender save code to the save function. Instead of using
			//the nSelRow parameter from the datalist, we'll set it ourselves based on the current selection.
			//We will also execute the save before we audit, in case of an exception while saving.
		case IDC_GENDER_LIST: {
			int nAuditID = -1;
			long nSelRow = m_GenderCombo->GetCurSel();

			_RecordsetPtr rs = CreateRecordset("SELECT Gender FROM PersonT WHERE ID = %li",m_CurrentID);

			_variant_t var = rs->Fields->Item["Gender"]->Value;

			// (a.walling 2006-10-05 14:53) - PLID 22873 - Prevent this from trying to write -1 to data
			if (nSelRow == sriNoRow) {
				m_GenderCombo->PutCurSel(0); // no selection
				OnClosedUpGenderList(0);
				return;
			}

			if (VarByte(var,0) == nSelRow)
				return;

			// now let's audit!
			nAuditID = BeginNewAuditEvent();

			CString strOldGender,strNewGender;

			if (VarByte(var,0) == 1) strOldGender = "Male";
			else if (VarByte(var,0) == 2) strOldGender = "Female";

			if (nSelRow == 1) strNewGender = "Male";
			else if (nSelRow == 2) strNewGender = "Female";

			ExecuteSql("UPDATE PersonT SET Gender = %li WHERE ID = %li",nSelRow,m_CurrentID);

			AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyGender, m_id, strOldGender, strNewGender, aepMedium, aetChanged);

			//TES 8/1/2010 - PLID 38838 - This is now included in HL7 exports.
			UpdatePatientForHL7();
			return;
		}

		default:
			return;
	}

	} NxCatchAll("Error in InsuranceDlg::Save");
}

void CInsuranceDlg::TransferResponsibilities(int iInsuredParty, CString strSourceResp, CString strDestResp)
{
	//JJ - with the new Insurance structure, it appears that this is not used or needed at all.
	//if we need it, all that needs to be done is to go through ChargeRespT and change the insured party ID
	//for all charges with the old insured party ID.
}

void CInsuranceDlg::OnSelChosenCompanyCombo(long nRow) 
{
	_RecordsetPtr rs;
	CString str;
	long InsCoID, InsContactID;
	CString InsPlanID = "NULL";
	BOOL bMultiplePlans = FALSE;

	try {
		long nOldID = -1;
		CString strOld;

		// (a.walling 2006-10-23 09:52) - PLID 23173 - Prevent audit/changing when re-selecting same insurance co.
		//		also ran into a difficulty where the below recordset was using PatientID as criteria, and returning
		//		multiple records when it expected just one. Reduced to one query.

		_RecordsetPtr rsOldCo = CreateRecordset("SELECT InsuranceCoT.PersonID, InsuranceCoT.Name FROM InsuranceCoT LEFT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE InsuredPartyT.PersonID = %li", m_CurrentID);
		if(!rsOldCo->eof && rsOldCo->Fields->Item["PersonID"]->Value.vt != VT_NULL)
			nOldID = rsOldCo->Fields->Item["PersonID"]->Value.lVal;
		if(!rsOldCo->eof && rsOldCo->Fields->Item["Name"]->Value.vt != VT_NULL)
			strOld = CString(rsOldCo->Fields->Item["Name"]->Value.bstrVal);

		if (nOldID == VarLong(m_CompanyCombo->GetValue(nRow, 0)))
			return;

		// (a.walling 2006-10-05 15:05) - PLID 22846 - Prevent errors with the blank selection
		if( (nRow == sriNoRow) || (IDNO == MessageBox("You are about to change the insurance company for this insured party!\n"
			"If this is not simply a minor correction, you should make this insured party inactive and then create a new one.\n\n"
			"Are you sure you wish to change to this insurance company?","Practice",MB_ICONEXCLAMATION|MB_YESNO))) {

			SetCompanyWithInactive(nOldID);
			Load();
			return;
		}

		//for audit
		rs = CreateRecordset("SELECT InsuredPartyT.PersonID AS ID FROM InsuredPartyT WHERE InsuredPartyT.PatientID=%li",m_id);
		if (rs->eof) {
			SetCompanyWithInactive(-1);
			rs->Close();
			return;
		}
		rs->Close();
		InsCoID = m_CompanyCombo->GetValue(nRow,0).lVal;
		rs = CreateRecordset("SELECT ID FROM InsurancePlansT WHERE InsCoID = %li",InsCoID);
		if(!rs->eof) {
			InsPlanID.Format("%li",rs->Fields->Item["ID"]->Value.lVal);
			rs->MoveNext();
			if(!rs->eof) {
				InsPlanID = "NULL";
				m_bDropPlanBox = TRUE;
			}
		}
		rs->Close();

		//try to use a default contact, if one exists
		if(!IsRecordsetEmpty("SELECT PersonID FROM InsuranceContactsT WHERE InsuranceCoID = %li AND [Default] = 1",InsCoID))
			str.Format("SELECT PersonID AS ID FROM InsuranceContactsT WHERE InsuranceCoID = %li AND [Default] = 1",InsCoID);
		else
			str.Format("SELECT TOP 1 PersonID AS ID FROM InsuranceContactsT WHERE InsuranceCoID = %li",InsCoID);

		rs = CreateRecordset(str);
		if(!rs->eof) {
			InsContactID = rs->Fields->Item["ID"]->Value.lVal;
		}
		else {
			AfxMessageBox("The insurance company you selected does not have an insurance contact.\n"
				"Please go to 'Edit Insurance List' to add a contact prior to assigning to a patient.");
			
			SetCompanyWithInactive(nOldID);
			Load();
			return;
		}
		rs->Close();

		ExecuteSql("UPDATE InsuredPartyT SET InsuranceCoID = %li, InsuranceContactID = %li, InsPlan = %s WHERE PersonID = %li",InsCoID,InsContactID,InsPlanID,m_CurrentID);

		//if Worker's Comp., see if the patient has a default date of injury and use that as the effective date
		if(ReturnsRecords("SELECT PersonID FROM InsuranceCoT WHERE WorkersComp = 1 AND InsuranceCoT.PersonID = %li",InsCoID)
			&& ReturnsRecords("SELECT DefaultInjuryDate FROM PatientsT WHERE PersonID = %li AND DefaultInjuryDate Is Not Null",m_id)) {
			ExecuteSql("UPDATE InsuredPartyT SET EffectiveDate = (SELECT DefaultInjuryDate FROM PatientsT WHERE PatientsT.PersonID = %li) WHERE InsuredPartyT.PersonID = %li", m_id, m_CurrentID);
		}

		//auditing
		CString strNew;
		strNew = CString(m_CompanyCombo->GetValue(nRow, 1).bstrVal);
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1) {
			//we are changing the ins. co.
			AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsuredPartyInsCoChanged, m_id, strOld, strNew, aepMedium, aetChanged);
		}

		// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
		UpdatePatientForHL7();

		// The insurance billing dialog needs to be updated
		// through the network code
		CClient::RefreshTable(NetUtils::PatInsParty);

	}NxCatchAll("Error in changing Company");
	Load();
}

//DRT - 12/17/01 - returns 0 if cancelled, 1 if successful
int CInsuranceDlg::AddInsuredPartyRecord()
{
	try {
		// (d.thompson 2009-03-19) - PLID 33590 - Moved functionality to GlobalInsuredPartyUtils
		long nNewInsPartyPersonID, nRespTypeID, nInsCoID;
		_variant_t varPlanID;
		if(!CreateNewInsuredPartyRecord(m_id, m_strPatientName, nNewInsPartyPersonID, varPlanID, nRespTypeID, nInsCoID)) {
			return 0;
		}

		// (r.goldschmidt 2014-08-01 11:48) - PLID 63111 - Rework saving of Default Insured Party Pay Groups and Insurance Company Deductibles
		InsertDefaultDeductibles(nInsCoID, nNewInsPartyPersonID);

		//Update our current flag
		m_CurrentID = nNewInsPartyPersonID;

		//Behavior from the old function that is interface-specific
		m_CompanyCombo->SetSelByColumn(0, (long)nInsCoID);

		//If no plan was set, we need to drop down the box for the plan
		if(varPlanID.vt == VT_NULL) {
			m_bDropPlanBox = TRUE;
		}

		//Success
		return 1;

	} NxCatchAll("Error adding Insurance: ");

	//exception
	return 0;
}

void CInsuranceDlg::OnReturnCompanyCombo() 
{
	//DRT 5/8/03 - I can find no way to make this code execute.

	OnSelChosenCompanyCombo(m_CompanyCombo->GetCurSel());	
}

// (d.thompson 2012-08-28) - PLID 52333 - Changed from SelChanged to SelChosen
void CInsuranceDlg::OnSelChosenInsurancePlan(long nRow) 
{
	_RecordsetPtr rs;
	CString strSQL;

	// (a.walling 2006-10-05 14:38) - PLID 22845 - Selecting a blank insurance plan throws an exception.

	if (nRow == sriNoRow) return;

	try{
		strSQL.Format("UPDATE InsuredPartyT SET InsPlan = %li WHERE PersonID = %li",
			m_PlanList->GetValue(m_PlanList->GetCurSel(),0).lVal,m_CurrentID);
		ExecuteSql(strSQL);
		rs = CreateRecordset("SELECT PlanType FROM InsurancePlansT LEFT JOIN InsuredPartyT ON "
			"InsurancePlansT.ID = InsuredPartyT.InsPlan WHERE InsuredPartyT.PersonID = %li",m_CurrentID);
		if(!rs->eof) {
			_variant_t var = rs->Fields->Item["PlanType"]->Value;
			if(var.vt==VT_BSTR)
				SetDlgItemText(IDC_TYPE_BOX, CString(var.bstrVal));
		}
		rs->Close();

		// (d.thompson 2012-08-28) - PLID 52129 - This can now be part of HL7, so export the message
		UpdatePatientForHL7();

	}NxCatchAll("Error in setting InsPlan.");
}

void CInsuranceDlg::OnReturnInsurancePlanBox() 
{
	//DRT 5/8/03 - I can find no way to make this code execute.

	// (d.thompson 2012-08-28) - PLID 52333 - Using SelChosen now
	OnSelChosenInsurancePlan(0);	
}


void CInsuranceDlg::OnBtnNextParty() 
{
	_RecordsetPtr rsActive, rsInactive;
	long ID = -1;
	CString sql;

	try {
		//JMJ - 7/14/2003 - This seems unnecessary but what we achieve here is the ability to have the order
		//sorted by priority, with inactives at the end.
		rsActive = CreateRecordset(_T("SELECT InsuredPartyT.PersonID AS ID FROM InsuredPartyT "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE InsuredPartyT.PatientID=%li AND RespTypeT.Priority <> -1 "
			"ORDER BY RespTypeT.Priority"),m_id);

		rsInactive = CreateRecordset(_T("SELECT InsuredPartyT.PersonID AS ID FROM InsuredPartyT "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE InsuredPartyT.PatientID=%li AND RespTypeT.Priority = -1 "
			"ORDER BY InsuredPartyT.PersonID"),m_id);

		if(rsActive->eof && rsInactive->eof)
			return;

		//first look through active ones
		while(!rsActive->eof && rsActive->Fields->Item["ID"]->Value.lVal != m_CurrentID) {
			rsActive->MoveNext();
		}
		if(!rsActive->eof) {
			rsActive->MoveNext();
			if(!rsActive->eof) {
				ID = rsActive->Fields->Item["ID"]->Value.lVal;
			}
			else {
				//if this line gets called, it means the next record is the first inactive company
				ID = rsInactive->Fields->Item["ID"]->Value.lVal;
			}			
		}
		rsActive->Close();

		if(ID == -1) {
			//not found, so now try Inactive
			while(!rsInactive->eof && rsInactive->Fields->Item["ID"]->Value.lVal != m_CurrentID) {
				rsInactive->MoveNext();
			}
			if(!rsInactive->eof)
				rsInactive->MoveNext();
			if(!rsInactive->eof) {
				ID = rsInactive->Fields->Item["ID"]->Value.lVal;
			}
			rsInactive->Close();
		}

		if(ID != -1) {
			m_CurrentID = ID;
			UpdateView();
		}
	}NxCatchAll("Error on move next insurance: ");
}

void CInsuranceDlg::OnBtnPrevParty() 
{
	_RecordsetPtr rsActive, rsInactive;
	long ID = -1;
	CString sql;

	try {
		//JMJ - 7/14/2003 - This seems unnecessary but what we achieve here is the ability to have the order
		//sorted by priority, with inactives at the end.
		rsActive = CreateRecordset(_T("SELECT InsuredPartyT.PersonID AS ID FROM InsuredPartyT "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE InsuredPartyT.PatientID=%li AND RespTypeT.Priority <> -1 "
			"ORDER BY RespTypeT.Priority"),m_id);

		rsInactive = CreateRecordset(_T("SELECT InsuredPartyT.PersonID AS ID FROM InsuredPartyT "
			"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
			"WHERE InsuredPartyT.PatientID=%li AND RespTypeT.Priority = -1 "
			"ORDER BY InsuredPartyT.PersonID"),m_id);

		if(rsActive->eof && rsInactive->eof)
			return;

		//first look through Inactive ones
		if(!rsInactive->eof)
			rsInactive->MoveLast();
		if(!rsActive->eof)
			rsActive->MoveLast();
		while(!rsInactive->bof && rsInactive->Fields->Item["ID"]->Value.lVal != m_CurrentID) {
			rsInactive->MovePrevious();
		}
		if(!rsInactive->bof) {
			rsInactive->MovePrevious();
			if(!rsInactive->bof) {
				ID = rsInactive->Fields->Item["ID"]->Value.lVal;
			}
			else {
				//if this line gets called, it means the previous record is the last active company
				ID = rsActive->Fields->Item["ID"]->Value.lVal;
			}
		}
		rsInactive->Close();

		if(ID == -1) {
			//not found, so now try active			
			while(!rsActive->bof && rsActive->Fields->Item["ID"]->Value.lVal != m_CurrentID) {
				rsActive->MovePrevious();
			}
			if(!rsActive->bof)
				rsActive->MovePrevious();
			if(!rsActive->bof) {
				ID = rsActive->Fields->Item["ID"]->Value.lVal;
			}
			rsActive->Close();
		}

		if(ID != -1) {
			m_CurrentID = ID;
			UpdateView();
		}
	}NxCatchAll("Error on move previous insurance: ");
}

void CInsuranceDlg::OnBtnAddInsurance() 
{
	long OldID = m_CurrentID;

	if (CheckCurrentUserPermissions(bioPatientInsurance,sptCreate))
	{	int succeed = AddInsuredPartyRecord();
		if(succeed){//only do these if we made it through the previous function
			CopyPatientInfo();					

			//(e.lally 2006-05-12) PLID 20598 - Update_View_() needs to be called outside of Assign_Resp and
			//	Copy_Patient_Info as it only needs to be called once (or not at all).
			UpdateView();
		}
		else {
			m_CurrentID = OldID;
		}
	}
}

BOOL CInsuranceDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID;
	static CString str;							

	switch (HIWORD(wParam))
	{	case EN_CHANGE:
			// (c.haag 2006-04-13 11:04) - PLID 20123 - If we're formatting a field, ignore the change
			if (m_bFormattingField || m_bFormattingAreaCode)
				break;
			switch (nID = LOWORD(wParam))
			{
				case IDC_MIDDLE_NAME_BOX:
					// (c.haag 2006-08-02 11:40) - PLID 21740 - We now check for auto-capitalization
					// for middle name boxes
					if (GetRemotePropertyInt("AutoCapitalizeMiddleInitials", 1, 0, "<None>", true)) {
						Capitalize(nID);
					}
					break;
				case IDC_FIRST_NAME_BOX:
				case IDC_LAST_NAME_BOX:
				case IDC_TITLE_BOX: 
				case IDC_ADDRESS1_BOX:
				case IDC_ADDRESS2_BOX:
				case IDC_CITY_BOX:
				case IDC_STATE_BOX:
				case IDC_EMPLOYER_SCHOOL_BOX:				
					Capitalize (nID);
					break;
				case IDC_ZIP_BOX:
				case IDC_EDIT_ACC_STATE:	// (j.jones 2009-03-06 10:20) - PLID 28834
					// (d.moore 2007-04-23 12:11) - PLID 23118 - 
					//  Capitalize letters in the zip code as they are typed in. Canadian postal
					//    codes need to be formatted this way.
					CapitalizeAll(nID);
					//FormatItem (nID, "#####-nnnn");
					break;
				case IDC_PHONE_BOX:
				case IDC_CONTACT_PHONE_BOX:
				case IDC_CONTACT_FAX_BOX:
					GetDlgItemText(nID, str);
					str.TrimRight();
					if (str != "") {
						if(m_bFormatPhoneNums) {
							m_bFormattingField = true;
							FormatItem (nID, m_strPhoneFormat);
							m_bFormattingField = false;
						}
					}
					break;
				case IDC_INS_PARTY_SSN: 
					// (f.dinatale 2011-02-01) - PLID 42260 - Mask the SSN with the appropriate mask.
					if(CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
						m_bFormattingField = true;	
						FormatSSN(SafeGetDlgItem<CEdit>(IDC_INS_PARTY_SSN), "###-##-####");
						m_bFormattingField = false;
					}
					break;
			}
			// (f.dinatale 2011-02-17) - PLID 42260 - Fixed the misplacing of the m_changed boolean which caused fields other than the SSN to not save.
			m_changed = true;
			break;
		case EN_KILLFOCUS:
		switch (LOWORD(wParam))
			{
				case IDC_MEMO_BOX:
					if(ForceFieldLength((CNxEdit*)GetDlgItem(IDC_MEMO_BOX))){
						Save(LOWORD(wParam));
					}
				//Otherwise, it still has focus, so we'll be back.
				break;

				case IDC_PHONE_BOX:
					if (SaveAreaCode(LOWORD(wParam))) {
						Save(LOWORD(wParam));
					}
				break;

				default:
					Save(LOWORD(wParam));
				break;
			}
		break;

		case EN_SETFOCUS:

			switch (LOWORD(wParam)) {
				case IDC_PHONE_BOX:
					if (ShowAreaCode()) {
						FillAreaCode(LOWORD(wParam));
					}
								
				default:
				break;
			}
		break;
	}		
	return CNxDialog::OnCommand(wParam, lParam);
}

void CInsuranceDlg::StoreDetails()
{
	try {

		//We can't trust m_changed on NxTimes;
		/*if(m_nxtBirthDate->GetDateTime() != m_dtBirthDate) {
			Save(IDC_BIRTH_DATE_BOX);
		}
		if(m_nxtEffectiveDate->GetDateTime() != m_dtEffectiveDate) {
			Save(IDC_EFFECTIVE_DATE);
		}
		if(m_nxtInactiveDate->GetDateTime() != m_dtInactiveDate) {
			Save(IDC_INACTIVE_DATE);
		}*/
		// (z.manning, 07/25/2006) - PLID 20726 - Save returns before doing anything if m_changed is false.
		OnKillFocusBirthDateBox();
		OnKillFocusEffectiveDate();
		OnKillFocusInactiveDate();
		// (j.jones 2009-03-06 09:43) - PLID 28834 - added accident date
		OnKillFocusCurAccDate();

		if (m_changed) {
			if(GetFocus()) {
				Save (GetFocus()->GetDlgCtrlID());
			}
		}

	}NxCatchAll("Error in StoreDetails()");
}

// (j.jones 2008-09-11 10:50) - PLID 14002 - added varExpireDate
void CInsuranceDlg::ReactivateInsurance(int iInsType, _variant_t varExpireDate)
{

	try {

		CString strInsType, str;

		strInsType = VarString(m_listRespType->GetValue(m_listRespType->GetCurSel(), RTC_NAME));

		if (IDNO == MessageBox("Reactivating this company will cause all known charge responsibilities and applies to be directed back to the insurance type you selected. Are you SURE you wish to do this?", "Practice - Reactivating Insurance", MB_ICONQUESTION|MB_YESNO)){
			Load(); //TODO - This really just needs to reset the combo to what it used to be, instead of loading the whole page
			return;
		}

		//JJ - were we to put any code concerning switching charge resp., it would be right here
		//however, the code below does it all!

		/////////////////////////////////////////////////////////////////////////////////////
		// Remove the expiration date from the insurance party record itself
		// (j.jones 2008-09-11 09:53) - PLID 14002 - If the date is in the future, prompt first before doing this,
		// with "Yes" being the action that clears the date. Otherwise, always clear it.
		BOOL bClearExpireDate = TRUE;

		if(varExpireDate.vt == VT_DATE && VarDateTime(varExpireDate).GetStatus() != COleDateTime::invalid) {

			//we have a date, is it in the future?
			COleDateTime dtExpireDate = VarDateTime(varExpireDate);
			COleDateTime dtTomorrow = COleDateTime::GetCurrentTime();
			dtTomorrow.SetDateTime(dtTomorrow.GetYear(), dtTomorrow.GetMonth(), dtTomorrow.GetDay(), 0, 0, 0);
			COleDateTimeSpan dtSpan;
			dtSpan.SetDateTimeSpan(1, 0, 0, 0);
			dtTomorrow += dtSpan;

			if(dtExpireDate >= dtTomorrow
				&& IDNO == MessageBox("This insured party has an inactivation date set in the future. "
				"Do you wish to remove this date?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				
				//it is in the future, but they chose not to reset it
				bClearExpireDate = FALSE;
			}
		}
			
		if(bClearExpireDate) {
			ExecuteParamSql("UPDATE InsuredPartyT SET ExpireDate = NULL, RespTypeID = {INT} WHERE PersonID = {INT}",
				iInsType, m_CurrentID);
		}
		else {
			ExecuteParamSql("UPDATE InsuredPartyT SET RespTypeID = {INT} WHERE PersonID = {INT}",
				iInsType, m_CurrentID);
		}

		long AuditID = -1;
		AuditID = BeginNewAuditEvent();
		if(AuditID!=-1)
			AuditEvent(m_id, m_strPatientName,AuditID,aeiInsuredPartyInsType,m_CurrentID,"Inactive",strInsType,aepMedium);

		//We need to update the view so the order is correct again
		UpdateView();

	}NxCatchAll("Error in ReactivateInsurance");
}

void CInsuranceDlg::OnBtnDeleteInsurance() 
{
	if(!CheckCurrentUserPermissions(bioPatientInsurance,sptDelete))
		return;

	_RecordsetPtr rs(__uuidof(Recordset));
	int result = AfxMessageBox("Are you sure you want to delete the insured party?",	MB_ICONQUESTION|MB_YESNO);
	if (result == IDYES) 
	{	
		try {
			CWaitCursor cuWait;



			BOOL bExistsChargeResps = ExistsInTable("ChargeRespT", "InsuredPartyID = %li", m_CurrentID);
			BOOL bExistsPayments = ExistsInTable("PaymentsT", "InsuredPartyID = %li", m_CurrentID);

			if(bExistsChargeResps && !bExistsPayments) {
				// there are ChargeRespT records but no payments
				if(IsRecordsetEmpty("SELECT ID FROM ChargeRespT WHERE InsuredPartyID = %li AND Amount <> Convert(money,0)", m_CurrentID)) {
					//we only have $0.00 charge resps
					if(IsRecordsetEmpty("SELECT ID FROM AppliesT WHERE RespID IN (SELECT ID FROM ChargeRespT WHERE InsuredPartyID = %li AND Amount = Convert(money,0))", m_CurrentID)) {
						//and nothing is applied to them! we can delete this!

						// (a.walling 2007-11-29 11:07) - PLID 28231 - Don't delete now!

						//ExecuteSql("DELETE FROM ChargeRespDetailT WHERE ChargeRespID IN (SELECT ID FROM ChargeRespT WHERE InsuredPartyID = %li AND Amount = Convert(money,0))", m_CurrentID);
						//ExecuteSql("DELETE FROM ChargeRespT WHERE InsuredPartyID = %li AND Amount = Convert(money,0)", m_CurrentID);
						//bExistsChargeResps = ExistsInTable("ChargeRespT", "InsuredPartyID = %li", m_CurrentID);

						// analagous to the above logic, instead of ExistsInTable (which is a bad idea to begin with), do the same thing while
						// filtering out those with 0 amounts.
						bExistsChargeResps = ReturnsRecords("SELECT ID FROM ChargeRespT WHERE InsuredPartyID = %li AND Amount <> Convert(money,0)", m_CurrentID);
					}
				}
			}

			CString sql;
			if(bExistsChargeResps || bExistsPayments){
				AfxMessageBox("You may not delete insured parties which have bills or payments assigned.\n"
					"Please reassign these items before deleting this insured party.\n\n"
					"Note: If there are deleted charges or payments that were formerly assigned to this insured party,\n"
					"then this insured party cannot be deleted for financial auditing purposes.", MB_OK);
				return;
			}

			// (j.dinatale 2012-01-11 10:08) - PLID 47456 - Prevent deletion of insurance associated with EMR charges
			if(ReturnsRecordsParam("SELECT TOP 1 1 FROM EMRChargeRespT WHERE InsuredPartyID = {INT}", m_CurrentID)){
				AfxMessageBox("This insured party could not be deleted because it is assigned to existing EMR charges.\n\n"
							"Please unassign all EMR charges from this insured party. If some charges cannot be reassigned, "
							"then this insured party must be inactivated.", MB_OK);
				return;
			}

			// (a.walling 2006-07-25 14:21) - PLID 21575 - Prevent deletion of insurance for labs
			if(ReturnsRecords("SELECT ID FROM LabsT WHERE InsuredPartyID = %li", m_CurrentID)) {
				AfxMessageBox("This insured party is selected on a lab form.\n"
							"You must reassign the lab in order to delete this insured party.", MB_OK);
				return;
			}

			//TES 4/21/2006 - PLID 3392 - Don't let them delete an insured party on a batched claim.
			if(ReturnsRecords("SELECT HcfaTrackT.ID FROM HcfaTrackT INNER JOIN BillsT ON HcfaTrackT.BillID = BillsT.ID "
				"WHERE BillsT.InsuredPartyID = %li OR BillsT.OthrInsuredPartyID = %li", m_CurrentID, m_CurrentID)) {
				AfxMessageBox("This insured party is selected on a batched insurance claim.\n"
					"You must reassign or unbatch this claim in order to delete this insured party.", MB_OK);
				return;
			}

			// (j.jones 2007-10-01 16:32) - PLID 8993 - disallow if in an Eligibility Request
			if (ReturnsRecords("SELECT ID FROM EligibilityRequestsT WHERE InsuredPartyID = %li", m_CurrentID)) {
				AfxMessageBox("You may not delete this insured party because it is selected in at least one E-Eligibility Request.");
				return;
			}
			// (s.dhole 2012-05-14 10:51) - PLID 49992 disallow if it assign to any glasses order
			if (ReturnsRecords("SELECT ID FROM GlassesOrderT WHERE InsuredPartyID = %li", m_CurrentID)) {
				AfxMessageBox("This insured party is selected on a Glasses or Contact Lens Order.\n"
					"Please reassign these items before deleting this insured party.\n\n"
					"Note: If there are deleted optical orders that were formerly assigned to this insured party,\n"
					"then this insured party cannot be deleted.", MB_OK);
				return;
			}

			// (j.gruber 2012-08-08 09:45) - PLID 51869 - check for appointments
			if (ReturnsRecordsParam(" SELECT AppointmentID FROM AppointmentInsuredPartyT WHERE InsuredPartyID = {INT}", m_CurrentID)) {
				MsgBox("This insured party is selected on an appointment.\n"
					"Please reassign or remove the insured party from the appointment before deleting this insured party.");
				return;
			}
			
			// (b.cardillo 2015-11-20 13:56) - PLID 67614 - Check for charges whose allowables reference this insured party
			if (ReturnsRecordsParam("SELECT * FROM ChargesT WHERE AllowableInsuredPartyID = {INT}", m_CurrentID)) {
				AfxMessageBox(
					"You may not delete this insured party because charges are using it to calculate their allowable amounts."
					, MB_OK | MB_ICONINFORMATION);
				return;
			}

			//for audit
			CString strOld;
			_RecordsetPtr rsOld = CreateRecordset("SELECT Name FROM InsuranceCoT LEFT JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID WHERE InsuredPartyT.PersonID = %li", m_CurrentID);
			if(!rsOld->eof && rsOld->Fields->Item["Name"]->Value.vt != VT_NULL)
				strOld = CString(rsOld->Fields->Item["Name"]->Value.bstrVal);
			rsOld->Close();

			//need to clear from GroupDetailsT!
			//(e.lally 2006-05-18) PLID 20681 - We can batch these sql statements together to increase the speed.
			CString strSql = BeginSqlBatch();

			// (a.walling 2007-11-29 11:26) - PLID 28231 - We fix the 0-amount ChargeRespT records here
			// First delete the 0 amount ChargeRespT and ChargeRespDetailT records where they are not the only chargeresp for that charge.
			AddStatementToSqlBatch(strSql, "DELETE FROM ChargeRespDetailT WHERE ChargeRespID IN (SELECT ID FROM ChargeRespT WHERE InsuredPartyID = %li AND Amount = Convert(money,0) AND ChargeID IN ( "
					"SELECT ChargeID FROM ChargeRespT GROUP BY ChargeID HAVING COUNT(*) > 1 " // charges with more than one ChargeRespT record
				") )", m_CurrentID);
			AddStatementToSqlBatch(strSql, "DELETE FROM ChargeRespT WHERE InsuredPartyID = %li AND Amount = Convert(money,0) AND ChargeID IN ( "
					"SELECT ChargeID FROM ChargeRespT GROUP BY ChargeID HAVING COUNT(*) > 1 " // charges with more than one ChargeRespT record
				")", m_CurrentID);
			// Then update the remaining ChargeResps to NULL, since they are the only ones remaining
			AddStatementToSqlBatch(strSql, "UPDATE ChargeRespT SET InsuredPartyID = NULL WHERE InsuredPartyID = %li", m_CurrentID);

			AddStatementToSqlBatch(strSql, "DELETE FROM GroupDetailsT WHERE PersonID = %li; \r\n", m_CurrentID);
			AddStatementToSqlBatch(strSql, "DELETE FROM ERemittanceHistoryT WHERE InsuredPartyID = %li;  \r\n",m_CurrentID);
			AddStatementToSqlBatch(strSql, "UPDATE BillsT SET InsuranceReferralID = NULL WHERE InsuranceReferralID IN (SELECT ID FROM InsuranceReferralsT WHERE InsuredPartyID = %li); \r\n",m_CurrentID);
			AddStatementToSqlBatch(strSql, "DELETE FROM InsuranceReferralDiagsT WHERE ReferralID IN (SELECT ID FROM InsuranceReferralsT WHERE InsuredPartyID = %li); \r\n",m_CurrentID);
			AddStatementToSqlBatch(strSql, "DELETE FROM InsuranceReferralCPTCodesT WHERE ReferralID IN (SELECT ID FROM InsuranceReferralsT WHERE InsuredPartyID = %li); \r\n",m_CurrentID);
			AddStatementToSqlBatch(strSql, "DELETE FROM InsuranceReferralsT WHERE InsuredPartyID = %li; \r\n",m_CurrentID);
			// (j.jones 2009-03-11 08:46) - PLID 32864 - we intentionally do not audit this
			// (j.jones 2009-09-21 11:37) - PLID 35564 - -1 is no longer allowed, so update to NULL
			AddStatementToSqlBatch(strSql, "UPDATE BillsT SET InsuredPartyID = NULL WHERE InsuredPartyID = %li; \r\n",m_CurrentID);
			AddStatementToSqlBatch(strSql, "UPDATE BillsT SET OthrInsuredPartyID = NULL WHERE OthrInsuredPartyID = %li;  \r\n",m_CurrentID);
			// (j.jones 2012-01-19 15:21) - PLID 47653 - update BilledEMNsT
			AddStatementToSqlBatch(strSql, "UPDATE BilledEMNsT SET InsuredPartyID = NULL WHERE InsuredPartyID = %li;  \r\n",m_CurrentID);
			// (j.gruber 2010-08-04 15:34) - PLID 39729 - InsuredPartyPayGroups
			AddStatementToSqlBatch(strSql, "DELETE FROM InsuredPartyPayGroupsT WHERE InsuredPartyID = %li; \r\n",m_CurrentID);
			// (j.jones 2011-04-26 16:52) - PLID 42705 - delete from ChargeAllowablesT
			AddStatementToSqlBatch(strSql, "DELETE FROM ChargeAllowablesT WHERE InsuredPartyID = %li; \r\n",m_CurrentID);
			// (j.jones 2011-12-19 15:39) - PLID 43925 - delete from ChargeCoinsuranceT
			AddStatementToSqlBatch(strSql, "DELETE FROM ChargeCoinsuranceT WHERE InsuredPartyID = %li; \r\n",m_CurrentID);
			// (j.jones 2015-10-20 09:50) - PLID 67385 - handle BatchPaymentCapitationDetailsT
			AddStatementToSqlBatch(strSql, "DELETE FROM BatchPaymentCapitationDetailsT WHERE InsuredPartyID = %li; \r\n", m_CurrentID);
			AddStatementToSqlBatch(strSql, "DELETE FROM InsuredPartyT WHERE PersonID = %li; \r\n",m_CurrentID);
			AddStatementToSqlBatch(strSql, "DELETE FROM PersonT WHERE ID = %li; \r\n",m_CurrentID);
			ExecuteSqlBatch(strSql);

/* TODO - 
			Not 100% sure about this code.  If we are deleting primary, it asks to move secondary up (if one exists).
		It doesn't do any prompting otherwise.  Eventually we should probably make it do some other prompting.
*/
			long nCurInsType = VarLong(m_listRespType->GetValue(m_listRespType->GetCurSel(), RTC_ID), -1);

			if(nCurInsType == 1) {
				rs = CreateRecordset("SELECT PersonID AS ID FROM InsuredPartyT WHERE PatientID = %li AND RespTypeID = 2", m_id);	//2 is always sec
				if(!rs->eof) {
					// (j.jones 2010-08-17 09:58) - PLID 40134 - clarify that these are pri/sec Medical resps.
					result = AfxMessageBox("You are deleting the patient's primary medical insurance.\n"
					"Would you like to switch the secondary medical insurance to Primary?", MB_ICONQUESTION|MB_YESNO);
					if(result==IDYES) {
						ExecuteSql("UPDATE InsuredPartyT SET RespTypeID = 1 WHERE PersonID = %li",rs->Fields->Item["ID"]->Value.lVal);	//1 is always pri
					}
				}
				rs->Close();
			}

			long AuditID = -1;
			AuditID = BeginNewAuditEvent();
			if(AuditID!=-1)
				AuditEvent(m_id, m_strPatientName,AuditID,aeiInsuredPartyDeleted,m_CurrentID,strOld,"<Deleted>",aepMedium,aetDeleted);

			// (a.walling 2010-10-13 10:44) - PLID 40978 - Dead code
			//m_lastID = -25;//forces a complete requery
			m_CurrentID = -1;

			// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
			UpdatePatientForHL7();

			// The insurance billing dialog needs to be updated
			// through the network code
			CClient::RefreshTable(NetUtils::PatInsParty);

			UpdateView();
		} NxCatchAll("Error deleting insured party: ");
	}
}

void CInsuranceDlg::OnRequeryFinishedCompanyCombo(short nFlags) 
{
}

void CInsuranceDlg::OnRequeryFinishedInsurancePlanBox(short nFlags) 
{
	m_bInsPlanIsRequerying = false;
	if(m_bDropPlanBox) {
		m_bDropPlanBox = FALSE;
		m_PlanList->DropDownState = TRUE;
	}
}


//nToRespID is what we will be assigning the current ins party to
void CInsuranceDlg::AssignResp(long nToRespID)
{
	try {

		CString strTypeName, strOldTypeName;
		BOOL Audit = FALSE;

		//lookup the type name in the datalist
		strTypeName = VarString(m_listRespType->GetValue(m_listRespType->FindByColumn(RTC_ID, long(nToRespID), 0, false), RTC_NAME), "");

		if(m_bSettingBox) {
			return;
		}

		//lookup the resp type and expiration date of the current insured party
		_RecordsetPtr rs = CreateParamRecordset("SELECT ExpireDate, RespTypeID FROM InsuredPartyT WHERE InsuredPartyT.PersonID = {INT}", m_CurrentID);
		if(rs->eof) {
			return;
		}

		long nCurrentRespID = AdoFldLong(rs, "RespTypeID");
		if(nCurrentRespID != -1 && nCurrentRespID != nToRespID)	{	//not what we're looking to switch to or inactive
			Audit = TRUE;
			strTypeName = VarString(m_listRespType->GetValue(m_listRespType->FindByColumn(RTC_ID, long(nCurrentRespID), 0, false), RTC_NAME), "");
		}

		//reactivate if inactive
		if(nCurrentRespID == -1 && rs->Fields->Item["ExpireDate"]->Value.vt == VT_DATE) {
			ReactivateInsurance(nToRespID, rs->Fields->Item["ExpireDate"]->Value);
			return;
		}
		rs->Close();

		// See if there is currently an ins co of our to-be type assigned to an existing insured party for this patient

		//this does not apply if we're setting to inactive
		if(nToRespID != -1) {
			//(This code will never get executed, but we can leave it in for now)
			if (GetInsuranceIDFromType(m_id, nToRespID) != -1) {
				MsgBox("You are attempting to change this insurance responsibility to %s.  "
					"Another company is already set as the %s responsibility, please move it first.", strTypeName, strTypeName);
				UpdateView();
				return;
			}
		}

		//make the change
		ExecuteParamSql("UPDATE InsuredPartyT SET RespTypeID = {INT} WHERE PersonID = {INT}", nToRespID, m_CurrentID);

		if(Audit) {
			long AuditID = -1;
			AuditID = BeginNewAuditEvent();
			if(AuditID!=-1) {
				AuditEvent(m_id, m_strPatientName,AuditID,aeiInsuredPartyInsType,m_CurrentID,strOldTypeName,strTypeName,aepMedium);
			}
		}

		//TES 6/11/2007 - PLID 26257 - We need to refresh the screen, the Secondary Reason Code may change based on the
		// new responsibility.
		UpdateView();
	} NxCatchAll("Error in InsuranceDlg::AssignResp");
}

void CInsuranceDlg::CopyPatientInfo()
{
	// (d.thompson 2009-03-19) - PLID 33590 - Moved this functionality to global use
	CopyPatientInfoToInsuredParty(m_id, m_CurrentID, m_strPatientName);
}


void CInsuranceDlg::EnableInsuredInfo(bool bEnable)
{
	BOOL bPerm = (GetCurrentUserPermissions(bioPatientInsurance) & SPT___W_______);

	if (bEnable && !bPerm) {
		bEnable = FALSE;
	}

	GetDlgItem(IDC_FIRST_NAME_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_LAST_NAME_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_MIDDLE_NAME_BOX)->EnableWindow(bEnable);
	// (j.jones 2012-10-24 14:29) - PLID 36305 - added Title
	GetDlgItem(IDC_TITLE_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_ADDRESS1_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_ADDRESS2_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_CITY_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATE_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_ZIP_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_EMPLOYER_SCHOOL_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_PHONE_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_RELATE_COMBO)->EnableWindow(bEnable);
	GetDlgItem(IDC_BIRTH_DATE_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_GENDER_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_MEMO_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_EFFECTIVE_DATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_INACTIVE_DATE)->EnableWindow(bEnable);
	//GetDlgItem(IDC_COPAY)->EnableWindow(bEnable);
	//GetDlgItem(IDC_CHECK_WARN_COPAY)->EnableWindow(bEnable);
	GetDlgItem(IDC_INSURANCE_PLAN_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_INSURANCE_ID_BOX)->EnableWindow(bEnable);
	// (j.jones 2012-11-12 10:49) - PLID 53622 - added country dropdown
	GetDlgItem(IDC_COUNTRY_LIST)->EnableWindow(bEnable);

	// (j.jones 2010-05-18 11:24) - PLID 37788 - if the relation to patient is "Self", the "Patient ID For Insurance" field is worthless
	CString strRelation = "";
	if(m_RelateCombo->CurSel != -1) {
		strRelation = VarString(m_RelateCombo->GetValue(m_RelateCombo->CurSel,0), "");
	}
	GetDlgItem(IDC_PATIENT_INSURANCE_ID_BOX)->EnableWindow(bEnable && strRelation != "Self");

	GetDlgItem(IDC_FECA_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_COMPANY_COMBO)->EnableWindow(bEnable);
	GetDlgItem(IDC_CONTACTS_COMBO)->EnableWindow(bEnable);
	GetDlgItem(IDC_RESP_TYPE_COMBO)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_REFERRALS)->EnableWindow(bEnable);

	// (f.dinatale 2011-02-01) - PLID 42260 - Disable the social security field if they do not have full access to view SSNs.
	if(CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
		SafeGetDlgItem<CEdit>(IDC_INS_PARTY_SSN)->SetReadOnly(!bEnable);
		SafeGetDlgItem<CEdit>(IDC_INS_PARTY_SSN)->EnableWindow(bEnable);
	} else {
		SafeGetDlgItem<CEdit>(IDC_INS_PARTY_SSN)->SetReadOnly(TRUE);
		SafeGetDlgItem<CEdit>(IDC_INS_PARTY_SSN)->EnableWindow(FALSE);
	}

	//GetDlgItem(IDC_RADIO_COPAY_AMOUNT)->EnableWindow(bEnable); // (z.manning, 10/18/2006) - PLID 23131 - Make sure radio buttons are enabled/disabled properly.
	//GetDlgItem(IDC_RADIO_COPAY_PERCENT)->EnableWindow(bEnable);	

	// (j.jones 2009-03-06 09:45) - PLID 28834 - added accident controls
	GetDlgItem(IDC_EDIT_CUR_ACC_DATE)->EnableWindow(bEnable);
	GetDlgItem(IDC_RADIO_EMPLOYMENT)->EnableWindow(bEnable);
	GetDlgItem(IDC_RADIO_AUTO_ACCIDENT)->EnableWindow(bEnable);
	//GetDlgItem(IDC_RADIO_COPAY_PERCENT)->EnableWindow(bEnable);
	GetDlgItem(IDC_RADIO_OTHER_ACCIDENT)->EnableWindow(bEnable);
	GetDlgItem(IDC_RADIO_NONE)->EnableWindow(bEnable);
	GetDlgItem(IDC_EDIT_ACC_STATE)->EnableWindow(bEnable);

	// (j.gruber 2006-12-29 15:14) - PLID 23972 - take this out
	//GetDlgItem(IDC_DEDUCTIBLE)->EnableWindow(bEnable); // (m.hancock 2006-11-02 10:24) - PLID 21274 - Enable / disable the deductible field.

	GetDlgItem(IDC_COPY_EMP_INFO)->EnableWindow(bPerm);
	GetDlgItem(IDC_COPY_PATIENT_INFO)->EnableWindow(bPerm);

	// (j.gruber 2010-08-02 11:02) - PLID 39729 - Pay group DL
	GetDlgItem(IDC_PAY_GROUP_LIST)->EnableWindow(bEnable);	
	// (j.gruber 2010-08-02 11:02) - PLID 39727 - duduct/oop button
	GetDlgItem(IDC_INS_EDIT_DEDUCT_OOP)->EnableWindow(bEnable);

	//TES 6/8/2007 - PLID 26257 - If this should be disabled because we're on a party that's not a non-primary Medicare,
	// it's OK, because the only place where true is passed in, it then calls Load(), which will make sure this field
	// is disabled if necessary.
	GetDlgItem(IDC_SECONDARY_REASON_CODE)->EnableWindow(bEnable);
	if(!bEnable) {
		m_pSecondaryReasonCode->CurSel = NULL;
	}

	// (j.jones 2009-06-23 11:39) - PLID 34689 - added ability to submit as primary, when not actually primary
	// Like the secondary reason code, this is sometimes disabled, but Load() is always
	// called after this is enabled, so it will be properly re-disabled then as needed.
	m_checkSubmitAsPrimary.EnableWindow(bEnable);

	// (j.jones 2010-07-19 10:54) - PLID 31802 - enable/disable the eligibility request button
	// (it requires E-Billing write permissions)
	// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
	m_btnCreateEligibilityRequest.EnableWindow(bEnable && (GetCurrentUserPermissions(bioEEligibility) & (sptWrite|sptWriteWithPass)));
}

void CInsuranceDlg::OnCopyPatientInfo() 
{
	BOOL boAddedInsurance = FALSE;

	if(IsRecordsetEmpty("SELECT InsuredPartyT.PersonID AS ID FROM InsuredPartyT WHERE InsuredPartyT.PatientID=%li",m_id))
	{
		if (IDYES == AfxMessageBox ("Patient has no insurance, add one?", MB_YESNO | MB_ICONQUESTION) 
			&& CheckCurrentUserPermissions(bioPatientInsurance,sptCreate)) {
			
			if(AddInsuredPartyRecord())
				boAddedInsurance = TRUE;
			else
				return;
		}
		else return;
	}

	try
	{
		if(boAddedInsurance || IDYES==MessageBox("This will overwrite all existing insured party information. \n"
			"Are you sure you wish to refresh this data?","Practice", MB_YESNO | MB_ICONQUESTION)) {

			CopyPatientInfo();
		}
	}NxCatchAll("Error copying patient info: ");
	UpdateView();
	GetDlgItem(IDC_FIRST_NAME_BOX)->SetFocus();
}

void CInsuranceDlg::OnEditInsuranceList() 
{
	CEditInsInfoDlg EditInsInfo(this);

	if (CheckCurrentUserPermissions(bioInsuranceCo,sptWrite))
	{
		long InsID = -1;
		long nContactID = -1;

		if(m_CompanyCombo->GetCurSel()!=-1) {
			InsID = m_CompanyCombo->GetValue(m_CompanyCombo->GetCurSel(),0).lVal;			
		}
		else {
			//if there is no current sel, it must have an inactive ins co, so check the m_nCurrentInsCoID variable
			InsID = m_nCurrentInsCoID;
		}

		if(m_ContactsCombo->GetCurSel()!=-1) {
			nContactID = m_ContactsCombo->GetValue(m_ContactsCombo->GetCurSel(),0).lVal;			
		}
		EditInsInfo.DisplayWindow(m_nColor, InsID, nContactID);
		UpdateView();
		GetDlgItem(IDC_COMPANY_COMBO)->SetFocus();
	}
}

HBRUSH CInsuranceDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	//to avoid drawing problems with transparent text and disabled ites
	//override the NxDialog way of doing text with a non-grey background
	//NxDialog relies on the NxColor to draw the background, then draws text transparently
	//instead, we actually color the background of the STATIC text

	if(nCtlColor == CTLCOLOR_STATIC) {
		if (pWnd->GetDlgCtrlID() == IDC_CHECK_WARN_COPAY) {
			extern CPracticeApp theApp;
			pDC->SelectPalette(&theApp.m_palette, FALSE);
			pDC->RealizePalette();
			pDC->SetBkColor(PaletteColor(m_nColor));
			return m_brush;
		}

		//don't make the memo transparent, weird things happen
		if(pWnd->GetDlgCtrlID() != IDC_CO_MEMO)
			pDC->SetBkMode(TRANSPARENT);
	}
	*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}



void CInsuranceDlg::FillAreaCode(long nPhoneID)  {

	
		//first check to see if anything is in this box
		CString strPhone;
		GetDlgItemText(nPhoneID, strPhone);
		if (! ContainsDigit(strPhone)) {
			// (j.gruber 2009-10-07 16:44) - PLID 35826 - updated for city lookup
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
				m_bFormattingAreaCode = true;
				SetDlgItemText(nPhoneID, strAreaCode);
				CString str = strAreaCode;
				str.TrimRight();
				if (str != "") {
					if(m_bFormatPhoneNums) {
						m_bFormattingField = true;
						FormatItem (nPhoneID, m_strPhoneFormat);
						m_bFormattingField = false;
					}
				}
				m_bFormattingAreaCode = false;
				
				//set the member variable 
				m_strAreaCode.Format("(%s) ###-####", strAreaCode);


				//set the cursor
				::PostMessage(GetDlgItem(nPhoneID)->GetSafeHwnd(), EM_SETSEL, 5, 5);
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


bool CInsuranceDlg::SaveAreaCode(long nID) {

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
			m_bFormattingAreaCode = true;
			SetDlgItemText(nID, "");
			m_bFormattingAreaCode = false;
			return false;
		}
		else {
			return true;
		}

	}
	//set out member variable to blank
	m_strAreaCode = "";

}

void CInsuranceDlg::OnCopyEmpInfo() 
{
	BOOL boAddedInsurance = FALSE;
	long nPatID;

	if(IsRecordsetEmpty("SELECT InsuredPartyT.PersonID AS ID FROM InsuredPartyT WHERE InsuredPartyT.PatientID=%li",m_id))
	{
		if (IDYES == AfxMessageBox ("Patient has no insurance, add one?", MB_YESNO | MB_ICONQUESTION) 
			&& CheckCurrentUserPermissions(bioPatientInsurance,sptCreate)) {
			
			if(AddInsuredPartyRecord())
				boAddedInsurance = TRUE;
			else
				return;
		}
		else return;
	}

	//TES 5/1/2008 - PLID 27576 - Need to be able to rollback if there's an exception.
	long nAuditTransactionID = -1;
	try
	{
		if(boAddedInsurance || IDYES==MessageBox("This will overwrite all existing insured party information. \n"
			"Are you sure you wish to refresh this data?","Practice", MB_YESNO | MB_ICONQUESTION)) {

			// m.carlson 5/18/2004, PL 11504 (for auditing)

			CString strOldBDay,strOldFirst,strOldMiddle,strOldLast,strOldTitle,strOldAddr1,strOldAddr2,strOldCity,strOldState,strOldZip,strOldHomePhone,strOldEmployer,strOldRelation,strOldSSN;
			COleDateTime dtOldBirthDate;
			CString strNewBDay,strNewFirst,strNewMiddle,strNewLast,strNewTitle,strNewAddr1,strNewAddr2,strNewCity,strNewState,strNewZip,strNewHomePhone,strNewEmployer,strNewRelation;
			COleDateTime dtNewBirthDate;
			_variant_t vOldGender;
			CString strOldGender;
			//_RecordsetPtr rsOld = CreateRecordset("SELECT Company,First,Middle,Last,Address1,Address2,City,State,Zip,HomePhone,BirthDate,Gender, SocialSecurity FROM PersonT WHERE ID=%li",m_CurrentID);	//for auditing
			// (j.jones 2012-10-24 15:28) - PLID 36305 - added Title
			_RecordsetPtr rsInsParty = CreateParamRecordset("SELECT InsPersonT.Company,InsPersonT.First, InsPersonT.Middle, "
				" InsPersonT.Last, InsPersonT.Title, InsPersonT.Address1,InsPersonT.Address2,InsPersonT.City,InsPersonT.State,InsPersonT.Zip, "
				" InsPersonT.HomePhone,InsPersonT.BirthDate, InsPersonT.Gender, InsPersonT.SocialSecurity, "
				" Employer,RelationToPatient,PatientID, "
				" EmployerFirst,EmployerMiddle,EmployerLast,EmployerAddress1,EmployerAddress2,EmployerCity,EmployerState,EmployerZip, "
				" PatPersonT.Company as PatCompany, PatPersonT.Gender AS PatGender "
				" FROM  "
				" PersonT InsPersonT INNER JOIN InsuredPartyT ON InsPersonT.ID = InsuredPartyT.PersonID "
				" INNER JOIN PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
				" INNER JOIN PersonT PatPersonT ON PatientsT.PersonID = PatPersonT.ID "
				" WHERE InsPersonT.ID={INT} ", m_CurrentID);

			if (! rsInsParty->eof) {

				FieldsPtr flds = rsInsParty->Fields;

				strOldFirst = AdoFldString(flds,"First");
				strOldMiddle = AdoFldString(flds,"Middle");
				strOldLast = AdoFldString(flds,"Last");
				// (j.jones 2012-10-24 15:28) - PLID 36305 - added Title
				strOldTitle = AdoFldString(flds,"Title");
				strOldAddr1 = AdoFldString(flds,"Address1");
				strOldAddr2 = AdoFldString(flds,"Address2");
				strOldCity = AdoFldString(flds,"City");
				strOldState = AdoFldString(flds,"State");
				strOldZip = AdoFldString(flds,"Zip");
				strOldHomePhone = AdoFldString(flds,"HomePhone");
				dtOldBirthDate = AdoFldDateTime(flds,"BirthDate",COleDateTime::COleDateTime(1800,1,1,1,25,25));
				vOldGender = flds->Item["Gender"]->Value;
				strOldSSN = AdoFldString(flds,"SocialSecurity");

			//_RecordsetPtr rsOld2 = CreateRecordset("SELECT Employer,RelationToPatient,PatientID FROM InsuredPartyT WHERE PersonID=%li",m_CurrentID);

				strOldEmployer = AdoFldString(flds,"Employer");
				strOldRelation = AdoFldString(flds,"RelationToPatient");
				nPatID = AdoFldLong(flds,"PatientID");

			//_RecordsetPtr rsNew = CreateRecordset("SELECT EmployerFirst,EmployerMiddle,EmployerLast,EmployerAddress1,EmployerAddress2,EmployerCity,EmployerState,EmployerZip FROM PatientsT WHERE PersonID = %li",nPatID);

			//_RecordsetPtr rsNew2 = CreateRecordset("SELECT Company,Gender FROM PersonT WHERE ID=%li",nPatID);

				strNewEmployer = AdoFldString(flds,"PatCompany");
				strNewFirst = AdoFldString(flds,"EmployerFirst");
				strNewMiddle = AdoFldString(flds,"EmployerMiddle");
				strNewLast = AdoFldString(flds,"EmployerLast");
				// (j.jones 2012-10-24 15:28) - PLID 36305 - added Title, always blank for employers
				strNewTitle = "";
				strNewAddr1 = AdoFldString(flds,"EmployerAddress1");
				strNewAddr2 = AdoFldString(flds,"EmployerAddress2");
				strNewCity = AdoFldString(flds,"EmployerCity");
				strNewState = AdoFldString(flds,"EmployerState");
				strNewZip = AdoFldString(flds,"EmployerZip");
				strNewHomePhone = "";

				strNewRelation = "Other";

				if (strOldFirst != strNewFirst) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyFirst, m_id, strOldFirst, strNewFirst, aepMedium, aetChanged);
				}
				if (strOldMiddle != strNewMiddle) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyMiddle, m_id, strOldMiddle, strNewMiddle, aepMedium, aetChanged);
				}
				if (strOldLast != strNewLast) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}					 
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyLast, m_id, strOldLast, strNewLast, aepMedium, aetChanged);
				}
				// (j.jones 2012-10-24 15:28) - PLID 36305 - added Title
				if (strOldTitle != strNewTitle) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}					 
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyTitle, m_id, strOldTitle, strNewTitle, aepMedium, aetChanged);
				}
				if (strOldAddr1 != strNewAddr1) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyAddress1, m_id, strOldAddr1, strNewAddr1, aepMedium, aetChanged);
				}
				if (strOldAddr2 != strNewAddr2) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyAddress2, m_id, strOldAddr2, strNewAddr2, aepMedium, aetChanged);
				}
				if (strOldCity != strNewCity) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyCity, m_id, strOldCity, strNewCity, aepMedium, aetChanged);
				}
				if (strOldState != strNewState ) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyState, m_id, strOldState, strNewState, aepMedium, aetChanged);
				}
				if (strOldZip != strNewZip) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyZip, m_id, strOldZip, strNewZip, aepMedium, aetChanged);
				}
				if (strOldHomePhone != strNewHomePhone) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyPhone, m_id, strOldHomePhone, strNewHomePhone, aepMedium, aetChanged);
				}
				if (strOldRelation != strNewRelation) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyRelation, m_id, strOldRelation, strNewRelation, aepMedium, aetChanged);
				}
				if (strOldEmployer != strNewEmployer) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyEmployer, m_id, strOldEmployer, strNewEmployer, aepMedium, aetChanged);
				}
				if (vOldGender.iVal != 0) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					if (VarByte(vOldGender) == 1) strOldGender = "Male";
					else if (VarByte(vOldGender) == 2) strOldGender = "Female";
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyGender, m_id, strOldGender, "", aepMedium, aetChanged);
				}
				

				if (dtOldBirthDate != COleDateTime::COleDateTime(1800,1,1,1,25,25))
					CString strOldBDay = FormatDateTimeForInterface(COleDateTime(dtOldBirthDate), dtoDate);

				if (strOldBDay != strNewBDay) {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartyDateofBirth, m_id, strOldBDay, strNewBDay, aepMedium, aetChanged);
				}

				if (strOldSSN != "") {
					if (nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					AuditEvent(m_id, m_strPatientName, nAuditTransactionID, aeiInsPartySSN, m_id, strOldSSN, "", aepMedium, aetChanged);
				}

				// m.carlson 5/18/2004 : end auditing stuff

				CString sql;
				// (j.jones 2012-10-24 15:28) - PLID 36305 - added Title, blank for employers
				sql.Format("UPDATE PersonT SET First = EmployerFirst, Middle = EmployerMiddle, Last = EmployerLast, Title = '', Address1 = EmployerAddress1, Address2 = EmployerAddress2, City = EmployerCity, State = EmployerState, Zip = EmployerZip, HomePhone = '', BirthDate = NULL, Gender = 0, SocialSecurity = '' "
						"FROM PatientsT RIGHT JOIN InsuredPartyT ON PatientsT.PersonID = InsuredPartyT.PatientID INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
						"WHERE InsuredPartyT.PersonID = %li",m_CurrentID);
				ExecuteSql("%s", sql);
				sql.Format("UPDATE InsuredPartyT SET InsuredPartyT.Employer = PersonT_1.Company, InsuredPartyT.RelationToPatient = 'Other' "
						"FROM PatientsT RIGHT JOIN InsuredPartyT ON PatientsT.PersonID = InsuredPartyT.PatientID INNER JOIN (SELECT * FROM PersonT) AS PersonT_1 ON PatientsT.PersonID = PersonT_1.ID INNER JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID "
						"WHERE InsuredPartyT.PersonID = %li",m_CurrentID);
				ExecuteSql("%s", sql);

				//commit the transaction
				//TES 5/1/2008 - PLID 27576 - We need to do this AFTER the changes have been successfully committed.
				if (nAuditTransactionID != -1) {
					CommitAuditTransaction(nAuditTransactionID);
				}

				// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
				UpdatePatientForHL7();
			}
		}
	}NxCatchAllCall("Error copying employer info: ",
		if(nAuditTransactionID != -1) {
			//TES 5/1/2008 - PLID 27576 - Need to rollback.
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);

	UpdateView();
	GetDlgItem(IDC_FIRST_NAME_BOX)->SetFocus();
}

void CInsuranceDlg::OnClosedUpGenderList(long nSelRow) 
{
	try{
		//(e.lally 2007-02-20) PLID 23762 - Moved the save code to the Save function. We need to set the change
			//flag. We can either force a save now, or wait for the focus to be lost, I chose to force a save now
			//to be consistent with the previous behavior.
		m_changed = TRUE;
		Save(IDC_GENDER_LIST);
	}NxCatchAll("Error setting gender.");
}

void CInsuranceDlg::SecureControls()
{
	// Return if we have write access
	if (GetCurrentUserPermissions(bioPatient) & SPT___W_______)
		return;

	// Disable write access
	EnableInsuredInfo(FALSE);

	// Disable some buttons
	GetDlgItem(IDC_COPY_PATIENT_INFO)->EnableWindow(FALSE);
	GetDlgItem(IDC_COPY_EMP_INFO)->EnableWindow(FALSE);
}

void CInsuranceDlg::OnSelChosenContactsCombo(long nRow) 
{
	if(nRow == -1)
		return;

	CString strOld, strNew;		//for auditing

	try {

		long ID = m_ContactsCombo->GetValue(nRow,0).lVal;
		_RecordsetPtr rsOld = CreateRecordset("SELECT First + ' ' + Last AS Name FROM PersonT WHERE ID IN (SELECT InsuranceContactID FROM InsuredPartyT WHERE PersonID = %li)", m_CurrentID);	//for auditing
		if(!rsOld->eof && rsOld->Fields->Item["Name"]->Value.vt == VT_BSTR)
			strOld = CString(rsOld->Fields->Item["Name"]->Value.bstrVal);

		ExecuteSql("UPDATE InsuredPartyT SET InsuranceContactID = %li WHERE PersonID = %li",ID,m_CurrentID);

		_RecordsetPtr rs = CreateRecordset("SELECT First, Last, WorkPhone, Fax, Extension FROM PersonT WHERE ID = %li",ID);

		if(!rs->eof) {
			SetDlgItemText (IDC_CONTACT_FIRST_BOX,	CString(rs->Fields->Item["First"]->Value.bstrVal));
			SetDlgItemText (IDC_CONTACT_LAST_BOX,	CString(rs->Fields->Item["Last"]->Value.bstrVal));
			SetDlgItemText (IDC_CONTACT_PHONE_BOX,	CString(rs->Fields->Item["WorkPhone"]->Value.bstrVal));
			SetDlgItemText (IDC_CONTACT_EXT_BOX, CString(rs->Fields->Item["Extension"]->Value.bstrVal));
			SetDlgItemText (IDC_CONTACT_FAX_BOX,	CString(rs->Fields->Item["Fax"]->Value.bstrVal));
		}

		//auditing
		strNew = CString(rs->Fields->Item["First"]->Value.bstrVal) + " " + CString(rs->Fields->Item["Last"]->Value.bstrVal);
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyContact, m_id, strOld, strNew, aepMedium, aetChanged);
		//end audit

		// (b.eyers 2015-11-12) - PLID 66977 - patient's ins contact changed, send an hl7 message
		UpdatePatientForHL7();

		rs->Close();
	}NxCatchAll("Error setting insurance contact.");
	
}

void CInsuranceDlg::OnRequeryFinishedContactsCombo(short nFlags) 
{

}

void CInsuranceDlg::OnEditReferrals() 
{
	CInsuranceReferralsDlg dlg(this);
	dlg.m_InsuredPartyID = m_CurrentID;
	dlg.DoModal();
	GetDlgItem(IDC_COMPANY_COMBO)->SetFocus();
}

void CInsuranceDlg::OnSwap() 
{
	//DRT TODO - 
	//		This just is wierd if they have more than just pri/sec... not sure what we should do with it.

	if(!CheckCurrentUserPermissions(bioPatientInsurance,sptWrite))
		return;

	_RecordsetPtr	rsPrimary(__uuidof(Recordset));
	_RecordsetPtr	rsSecondary(__uuidof(Recordset));

	CString	sql;

	try{
		//check to see if there is a Primary Insurance Company
		sql.Format ("SELECT PersonID FROM InsuredPartyT WHERE RespTypeID = 1 AND PatientID = %li", m_id);	//1 is always primary
		rsPrimary = CreateRecordsetStd(sql);
		if(rsPrimary->eof) {
			// (j.jones 2010-08-17 09:58) - PLID 40134 - clarify that these are pri/sec Medical resps.
			MsgBox("There is no primary medical insurance currently associated with this patient.");
			return;
		}

		//make sure there is only 1 primary insurance company
		if(rsPrimary->GetRecordCount() > 1)
		{
			// (j.jones 2010-08-17 09:58) - PLID 40134 - clarify that these are pri/sec Medical resps.
			AfxMessageBox("There is more then one primary medical insurance company. \n Please mark one of these inactive.");
			return;
		}

		long nPrimaryInsuredParty = long(rsPrimary->Fields->Item["PersonID"]->Value.lVal);

		//check to see if there is Secondary Insurance Company
		sql.Format ("SELECT PersonID FROM InsuredPartyT WHERE RespTypeID = 2 AND PatientID = %li", m_id);	//2 is always secondary
		rsSecondary = CreateRecordsetStd(sql);
		if(rsSecondary->eof) {
			// (j.jones 2010-08-17 09:58) - PLID 40134 - clarify that these are pri/sec Medical resps.
			MsgBox("There is no secondary medical insurance currently associated with this patient.");
			return;
		}

		//make sure there is only 1 secondary insurance company
		if(rsSecondary->GetRecordCount() > 1)
		{
			// (j.jones 2010-08-17 09:58) - PLID 40134 - clarify that these are pri/sec Medical resps.
			AfxMessageBox("There is more then one secondary medical insurance company. \n Please mark one of these inactive.");
			return;
		}

		long nSecondaryInsuredParty = long(rsSecondary->Fields->Item["PersonID"]->Value.lVal);

		//Ask the user to make sure they want to swap
		// (j.jones 2010-08-17 10:15) - PLID 40134 - clarify that these are pri/sec Medical resps.
		if (IDNO == MessageBox("This will swap the primary and secondary medical insurance companies.  \n Are you sure you wish to do this?", NULL, MB_YESNO)) {
			return;
		}

		//use BEGIN TRANS because if one of them fails we don't to set the other RespTypeID
		BEGIN_TRANS("INSURANCE SWAP"){
			ExecuteSql("UPDATE InsuredPartyT SET RespTypeID = 2 Where PersonID = %li", nPrimaryInsuredParty);	//2 is always sec.
			ExecuteSql("UPDATE InsuredPartyT SET RespTypeID = 1 Where PersonID = %li", nSecondaryInsuredParty);	//1 is always pri.
		}END_TRANS("INSURANCE SWAP");

		long nCurType = VarLong(m_listRespType->GetValue(m_listRespType->GetCurSel(), RTC_ID));

		if(nCurType == 1) {
			//we're on pri now, set it to secondary
			m_listRespType->SetSelByColumn(RTC_ID, long(2));
		}
		else if(nCurType == 2) {
			//we're on sec now, set it to primary
			m_listRespType->SetSelByColumn(RTC_ID, long(1));
		}
		else {
			//we're not on either one, leave it as is
		}

		UpdateView();

		// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
		UpdatePatientForHL7();

	}NxCatchAll("Error in CInsuranceDlg::OnSwap");
}

void CInsuranceDlg::OnSelChosenRespTypeCombo(long nRow) 
{
	//changes this insured parties responsibility.

	try {

		if(nRow == -1) {
			// (j.jones 2008-09-11 09:46) - PLID 14002 - disallow unselecting this box, 
			// instead reselect the stored data and return
			_RecordsetPtr rs = CreateParamRecordset("SELECT RespTypeID FROM InsuredPartyT WHERE PersonID = {INT}", m_CurrentID);
			if(!rs->eof) {
				long nCurRespType = AdoFldLong(rs, "RespTypeID");
				m_listRespType->SetSelByColumn(RTC_ID, nCurRespType);
			}
			else {
				ThrowNxException("RespTypeID lookup failed due to no insured party record found!");
			}
			rs->Close();
			return;
		}

		//1)  See what we're looking for.
		long nToRespID = -1;
		CString strRespName;

		nToRespID = VarLong(m_listRespType->GetValue(nRow, RTC_ID));
		strRespName = VarString(m_listRespType->GetValue(nRow, RTC_NAME), "");

		//2)  Lookup the resp for what we changed to.  If it already exists, tell them
		//		so and quit.
		//		This only applies if they are not marking it inactive - they can do that as
		//		often and as many times as they like.
		if(nToRespID != -1) {
			if(GetInsuranceIDFromType(m_id, nToRespID) != -1) {
				//another insured party exists for this type.
				MsgBox("Another insured party already exists for this responsibility type.\n"
					"Please change it before attempting to set the type of this insured party.");
				Load();	//TODO - This really just needs to reset the combo to what it used to be, instead of loading the whole page
				return;
			}
		}

		//warn them
		if(MsgBox(MB_YESNO, "Are you absolutely sure you want to change the responsibility of this insured party?") == IDNO) {
			Load();
			return;
		}

		//3)  If not, update the InsuredPartyID for this record to reflect the current
		//		changes.
		if(nToRespID != -1) {
			AssignResp(nToRespID);
		}
		else {
			InactivateResp();
		}

		// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
		UpdatePatientForHL7();

	} NxCatchAll("Error choosing new responsibility type.");

}

// (j.jones 2008-09-11 10:32) - PLID 14002 - Made InactivateResp return TRUE if it succeeded,
// and FALSE if it failed (or was cancelled by the user). Also added bSkipExpireDateCheck
// which, if enabled, will tell the function not to validate or alter the ExpireDate.
BOOL CInsuranceDlg::InactivateResp(BOOL bSkipExpireDateCheck /*= FALSE*/)
{
	_RecordsetPtr rs;
	CString sql;

	if(m_bSettingBox) {
		return FALSE;
	}

	try {

		CString strFormat, str;

		// (j.jones 2008-09-11 09:46) - PLID 14002 - added ExpireDate
		_RecordsetPtr rs = CreateParamRecordset("SELECT RespTypeID, ExpireDate FROM InsuredPartyT WHERE PersonID = {INT}", m_CurrentID);
		if(rs->eof) {
			ThrowNxException("Insured Party lookup failed due to no insured party record found!");
		}

		long nCurRespType = AdoFldLong(rs, "RespTypeID");
		long nFindRow = m_listRespType->FindByColumn(RTC_ID, nCurRespType, 0, VARIANT_FALSE);
		CString strCurrentType = VarString(m_listRespType->GetValue(nFindRow, RTC_NAME));

		//pull the ExpireDate
		_variant_t varExpireDate = rs->Fields->Item["ExpireDate"]->Value;

		rs->Close();

		if (!strCurrentType.IsEmpty()) {
			strFormat.Format("Deactivating this company for the patient will remove all %s insurance responsibilities from all the bills of this patient!\n"
				"This data may be recovered for historical bookkeeping, however. Are you sure you wish to mark this company as inactive?",
				strCurrentType);

			if (IDNO == MessageBox(strFormat, "Practice - Marking Insurance Company Inactive", MB_YESNO|MB_ICONQUESTION)) {
				//they said no, reset the box
				m_listRespType->SetSelByColumn(RTC_ID, long(nCurRespType));
				return FALSE;
			}
		}

		// (j.jones 2008-09-11 09:48) - PLID 14002 - If the insured party already has an expire date,
		// and the date is in the future, ask if they want to reset it to today's date. But skip
		// this check and the date change entirely if bSkipExpireDateCheck is true.
		BOOL bSetExpireDate = FALSE;
		if(!bSkipExpireDateCheck) {
			if(varExpireDate.vt == VT_DATE && VarDateTime(varExpireDate).GetStatus() != COleDateTime::invalid) {

				//we have a date, is it in the future?
				COleDateTime dtExpireDate = VarDateTime(varExpireDate);
				COleDateTime dtTomorrow = COleDateTime::GetCurrentTime();
				dtTomorrow.SetDateTime(dtTomorrow.GetYear(), dtTomorrow.GetMonth(), dtTomorrow.GetDay(), 0, 0, 0);
				COleDateTimeSpan dtSpan;
				dtSpan.SetDateTimeSpan(1, 0, 0, 0);
				dtTomorrow += dtSpan;

				if(dtExpireDate >= dtTomorrow
					&& IDYES == MessageBox("This insured party already has an inactivation date set in the future. "
					"Do you wish to reset it to today's date?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
					
					//it is in the future and they agreed to reset it
					bSetExpireDate = TRUE;
				}
			}
			else {
				//we have no existing date, so update to the current date
				bSetExpireDate = TRUE;
			}
		}
		
		if(bSetExpireDate) {
			//update the date and the status
			ExecuteParamSql("UPDATE InsuredPartyT SET ExpireDate = GetDate(), RespTypeID = -1 WHERE PersonID = {INT}", m_CurrentID);
		}
		else {
			//update the status only
			ExecuteParamSql("UPDATE InsuredPartyT SET RespTypeID = -1 WHERE PersonID = {INT}", m_CurrentID);
		}

		long AuditID = -1;
		AuditID = BeginNewAuditEvent();
		if(AuditID!=-1) {
			AuditEvent(m_id, m_strPatientName, AuditID, aeiInsuredPartyInsType, m_CurrentID, strCurrentType, "Inactive", aepMedium);
		}

		/////////////////////
		//JJ 7/7/2000 - Now the user can transfer responsibilities of an expired insurance to Patient Resp.
		/////////////////////
		if (!strCurrentType.IsEmpty()) {
			str.Format("Do you want to transfer the unpaid %s insurance responsibilities from all of this patient's bills to patient responsibility?\n"
				"\nSelecting 'No' will place all the responsibilities in the inactive insurance column, and will reassign them if the insurance is reactivated.\n"
				"\nSelecting 'Yes' will keep any paid amounts in the inactive column, but unpaid amounts will IRREVERSIBLY be assigned to the patient.",
				strCurrentType);
			if (IDYES == MessageBox(str, "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {

				_RecordsetPtr rs1;
				//get all charges with this insured party that have a balance
				str.Format("SELECT ChargesT.ID AS ChargeID FROM ChargesT INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
					"INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
					"WHERE BillsT.PatientID = %li AND ChargeRespT.InsuredPartyID = %li", m_id, m_CurrentID);
				rs1 = CreateRecordset(str);

				while(!rs1->eof) {
					long ChargeID = rs1->Fields->Item["ChargeID"]->Value.lVal;

					// (j.jones 2005-11-07 17:12) - PLID 18252 - you'd otherwise get a warning
					// if you tried to ShiftInsBalance() on a charge is no balance
					if(GetChargeInsBalance(ChargeID, m_id, m_CurrentID) > COleCurrency(0,0)) {

						//JJ - 5/4/2001 - by having this 3 instead of -1, it only shifts the unpaid amount!
						//DRT 5/8/03 - uses ins parties, not type ids now.  Also tells it not to prompt
						//		with the transfer dialog.
						// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
						ShiftInsBalance(ChargeID, m_id, m_CurrentID, -1, "Charge", "after inactivating the insured party", false);
					}
					rs1->MoveNext();
				}
				rs1->Close();
			}
		}

		//We need this update the view so the order is correct again.
		UpdateView();

		return TRUE;

	}NxCatchAll("Error in InsuranceDlg::OnInactivateResp");

	//We need this update the view so the order is correct again.
	UpdateView();

	return FALSE;
}

void CInsuranceDlg::OnEditInsRespTypes() 
{
	try {
		CEditInsuranceRespTypesDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			//save current selection
			//DRT 3/17/2004 - PLID 11466 - If there's no insurance on this patient, we need to handle that.
			long nCurSel = m_listRespType->GetCurSel();
			long nCurID = -1;
			if(nCurSel != sriNoRow)
				nCurID = VarLong(m_listRespType->GetValue(nCurSel, RTC_ID), -1);

			//refresh the list
			m_listRespType->Requery();

			if(nCurSel != sriNoRow)
				m_listRespType->TrySetSelByColumn(RTC_ID, long(nCurID));
		}
		GetDlgItem(IDC_RESP_TYPE_COMBO)->SetFocus();
	} NxCatchAll("Error in OnEditInsRespTypes()");
}

BOOL CInsuranceDlg::IsDateValid(_DNxTimePtr& pdt) 
{
	COleDateTime dt, dttemp, dtNow;
	dttemp.ParseDateTime("01/01/1800");
	dt = pdt->GetDateTime();
	dtNow = COleDateTime::GetCurrentTime();
	if(pdt->GetStatus() != 3 && !(pdt->GetStatus() == 1 && dt.m_dt >= dttemp.m_dt)) {
		return FALSE;
	}
	return TRUE;
}

void CInsuranceDlg::OnKillFocusBirthDateBox() 
{
	// (c.haag 2006-04-14 10:23) - PLID 20116 - New way of handling dates
	if (!IsDateValid(m_nxtBirthDate)) {
		//The new date is invalid, set it back to the previous value or clear it
		//if the previous value was blank.
		if(m_dtBirthDate.GetStatus() == COleDateTime::invalid)
			m_nxtBirthDate->Clear();
		else
			m_nxtBirthDate->SetDateTime(m_dtBirthDate);
		AfxMessageBox("You have entered an invalid date. The Birth Date has been reset.");
	}
	else if ((long)m_nxtBirthDate->GetDateTime() != (long)m_dtBirthDate.m_dt) {
		//(e.lally 2006-09-15) PLID 22530 - We weren't checking for the invalid status and
		//the variable was not being set to what was assumed to be 0, so I am setting the value to 0
		//and also check for validity to cover all other places of this assumption.

		//The date values are not the same, let's check the status because we still need to.
		//If one of the dates is valid, then it is a true change, otherwise even though the values are different, they
		//are both the same (invalid).
		if(m_nxtBirthDate->GetStatus() == 1 /*valid*/ || m_dtBirthDate.GetStatus() == COleDateTime::valid){
			m_changed = true;
			Save(IDC_BIRTH_DATE_BOX);
		}
	}
}

void CInsuranceDlg::OnKillFocusEffectiveDate() 
{
	// (c.haag 2006-04-14 10:23) - PLID 20116 - New way of handling dates
	if (!IsDateValid(m_nxtEffectiveDate)) {
		//The new date is invalid, set it back to the previous value or clear it
		//if the previous value was blank.
		if(m_dtEffectiveDate.GetStatus() == COleDateTime::invalid)
			m_nxtEffectiveDate->Clear();
		else
			m_nxtEffectiveDate->SetDateTime(m_dtEffectiveDate);
		AfxMessageBox("You have entered an invalid date. The Effective Date has been reset.");
	}

	else if((long)m_nxtEffectiveDate->GetDateTime() != (long)m_dtEffectiveDate.m_dt) {

		//The date values are not the same, let's check the status because we still need to.
		//If one of the dates is valid, then it is a true change, otherwise even though the values are different, they
		//are both the same (invalid).
		if(m_nxtEffectiveDate->GetStatus() == 1 /*valid*/ || m_dtEffectiveDate.GetStatus() == COleDateTime::valid){
			m_changed = true;
			Save(IDC_EFFECTIVE_DATE);
		}
	}
}

//sets the company to the ID given, if it's inactive, it puts the right
//company name in the list
//throws exceptions
void CInsuranceDlg::SetCompanyWithInactive(long nCompanyID)
{
	if(m_CompanyCombo->TrySetSelByColumn(0, (long)nCompanyID) == -1) {
		//may have an inactive company
		_RecordsetPtr rsIns = CreateRecordset("SELECT InsuranceCoT.PersonID, Name FROM InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = %li)", m_CurrentID);
		if(!rsIns->eof) {
			m_CompanyCombo->PutCurSel(-1);
			m_CompanyCombo->PutComboBoxText(_bstr_t(AdoFldString(rsIns, "Name", "")));
			m_nCurrentInsCoID = AdoFldLong(rsIns, "PersonID");
		}
		else {
			//this is actually possible if they have no insurance for this patient
		}
	}
}

void CInsuranceDlg::OnTrySetSelFinishedCompanyCombo(long nRowEnum, long nFlags) 
{
	try {
		if(nFlags == dlTrySetSelFinishedFailure) {
			//may have an inactive company
			_RecordsetPtr rsIns = CreateRecordset("SELECT InsuranceCoT.PersonID, Name FROM InsuranceCoT WHERE PersonID = (SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = %li)", m_CurrentID);
			if(!rsIns->eof) {
				m_CompanyCombo->PutCurSel(-1);
				m_CompanyCombo->PutComboBoxText(_bstr_t(AdoFldString(rsIns, "Name", "")));
				m_nCurrentInsCoID = AdoFldLong(rsIns, "PersonID");
			}
			else {
				//this is actually possible if they have no insurance for this patient
			}
		}
	} NxCatchAll("Error in CInsuranceDlg::OnTrySetSelFinishedCompanyCombo()");
}

/*void CInsuranceDlg::OnCheckWarnCopay() 
{
	try {

		ExecuteSql("UPDATE InsuredPartyT SET WarnCoPay = %li WHERE PersonID = %li",m_checkWarnCoPay.GetCheck() ? 1 : 0, m_CurrentID);

	}NxCatchAll("Error in OnCheckWarnCopay");
}*/

void CInsuranceDlg::OnBtnEditCptNotes() 
{
	CCPTCodeInsNotesDlg dlg(this);

	_RecordsetPtr rs = CreateRecordset("SELECT InsuranceCoID FROM InsuredPartyT WHERE PersonID = %li",m_CurrentID);
	if(!rs->eof) {
		dlg.m_nInsCoID = AdoFldLong(rs, "InsuranceCoID",-1);
	}
	rs->Close();	

	dlg.DoModal();
}

void CInsuranceDlg::OnKillFocusInactiveDate() 
{
	// (j.jones 2008-09-11 09:29) - PLID 14002 - we can now change the inactive date
	if (!IsDateValid(m_nxtInactiveDate)) {
		//The new date is invalid, set it back to the previous value or clear it
		//if the previous value was blank.
		if(m_dtInactiveDate.GetStatus() == COleDateTime::invalid)
			m_nxtInactiveDate->Clear();
		else
			m_nxtInactiveDate->SetDateTime(m_dtInactiveDate);
		AfxMessageBox("You have entered an invalid date. The Inactive Date has been reset.");
	}

	else if((long)m_nxtInactiveDate->GetDateTime() != (long)m_dtInactiveDate.m_dt) {

		//The date values are not the same, let's check the status because we still need to.
		//If one of the dates is valid, then it is a true change, otherwise even though the values are different, they
		//are both the same (invalid).
		if(m_nxtInactiveDate->GetStatus() == 1 /*valid*/ || m_dtInactiveDate.GetStatus() == COleDateTime::valid){
			m_changed = true;
			Save(IDC_INACTIVE_DATE);
		}
	}
}
// (s.tullis 2016-02-12 10:14) - PLID 68212 
void CInsuranceDlg::OnKillfocusSsn() 
{
	try {
		CString strSsn;
		GetDlgItemText(IDC_INS_PARTY_SSN, strSsn);
		if (strSsn == "###-##-####") {
			strSsn = "";
			FormatItemText(GetDlgItem(IDC_INS_PARTY_SSN), strSsn, "");
		}
	}NxCatchAll(__FUNCTION__)
}

void CInsuranceDlg::OnSelChosenRelateCombo(long nRow) 
{
	if(nRow == -1) {
		m_RelateCombo->CurSel = 0;
	}

	try {

		CString strOldValue, strNewValue;

		_RecordsetPtr rs = CreateRecordset("SELECT RelationToPatient FROM InsuredPartyT WHERE PersonID = %li",m_CurrentID);
		if(!rs->eof)
			strOldValue = AdoFldString(rs,"RelationToPatient","");

		strNewValue = CString(m_RelateCombo->GetValue(m_RelateCombo->CurSel,0).bstrVal);

		if(strNewValue != strOldValue) {

			int nAuditID = BeginNewAuditEvent();		

			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyRelation, m_id, strOldValue,strNewValue,aepMedium, aetCreated);
		}

		ExecuteSql("UPDATE InsuredPartyT SET RelationToPatient = '%s' WHERE PersonID = %li", _Q(strNewValue), m_CurrentID);

		// (j.jones 2010-05-18 11:24) - PLID 37788 - if "Self", the "Patient ID For Insurance" field is worthless
		GetDlgItem(IDC_PATIENT_INSURANCE_ID_BOX)->EnableWindow(strNewValue != "Self");

		// (z.manning 2009-01-08 15:36) - PLID 32663 - Update this patient for HL7
		UpdatePatientForHL7();

	}NxCatchAll("Error in changing relation combo.");
}

/*void CInsuranceDlg::OnRadioCopayAmount() 
{
	OnRadioCopay();
}

void CInsuranceDlg::OnRadioCopayPercent() 
{
	OnRadioCopay();
}

void CInsuranceDlg::OnRadioCopay()
{
	if(m_radioCopayAmount.GetCheck()) {
		//flat amount
		m_radioCopayPercent.SetCheck(FALSE);

		if(IsRecordsetEmpty("SELECT CopayType FROM InsuredPartyT WHERE CopayType = 0 AND PersonID = %li",m_CurrentID)) {

			ExecuteSql("UPDATE InsuredPartyT SET CopayType = 0 WHERE PersonID = %li",m_CurrentID);

			long nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyCopayType, m_id, "Percentage Copay", "Flat Copay Amount", aepMedium, aetChanged);
		}
	}
	else {
		//percent amount
		m_radioCopayAmount.SetCheck(FALSE);
		m_radioCopayPercent.SetCheck(TRUE);
		if(IsRecordsetEmpty("SELECT CopayType FROM InsuredPartyT WHERE CopayType = 1 AND PersonID = %li",m_CurrentID)) {

			ExecuteSql("UPDATE InsuredPartyT SET CopayType = 1 WHERE PersonID = %li",m_CurrentID);

			long nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyCopayType, m_id, "Flat Copay Amount", "Percentage Copay", aepMedium, aetChanged);
		}
	}

	LoadCopayInfo();
}

void CInsuranceDlg::LoadCopayInfo()
{
	_RecordsetPtr rs = CreateRecordset("SELECT InsuredPartyT.CopayType, InsuredPartyT.Copay, InsuredPartyT.CopayPercent "
			"FROM InsuredPartyT WHERE PersonID = %li", m_CurrentID);

	if(!rs->eof) {

		long nCopayType = AdoFldLong(rs, "CopayType",0);
			
		if(nCopayType == 0) {
			//flat amount
			CString strCopay = FormatCurrencyForInterface(COleCurrency(0,0));

			_variant_t var = rs->Fields->Item["Copay"]->Value;
			if(var.vt == VT_CY) {
				COleCurrency cyCoPay;
				cyCoPay = var.cyVal;
				if(cyCoPay.m_status!=COleCurrency::invalid) {
					strCopay = FormatCurrencyForInterface(cyCoPay);
				}
			}
			SetDlgItemText(IDC_COPAY, strCopay);
		}
		else {
			//percent amount
			CString strCopay = "0%";

			long nPercent = AdoFldLong(rs, "CopayPercent",0);
			strCopay.Format("%li%%",nPercent);

			SetDlgItemText(IDC_COPAY, strCopay);
		}
	}
	rs->Close();
}*/

void CInsuranceDlg::OnSelChangingInsurancePlanBox(long FAR* nNewSel) 
{
	// (a.walling 2006-10-16 10:52) - PLID 22845 - Prevent selection of blank plan
	if (*nNewSel == sriNoRow) {
		*nNewSel = m_PlanList->CurSel;
	}
}

void CInsuranceDlg::OnSelChangingCompanyCombo(long FAR* nNewSel) 
{
	// (a.walling 2006-10-23 09:05) - PLID 23173 - Don't fire the chosen event if the selection is invalid.
	if (*nNewSel == sriNoRow) {
		*nNewSel = m_CompanyCombo->CurSel;
	}
}

// (j.gruber 2006-12-29 15:14) - PLID 23972 - take this out
/*void CInsuranceDlg::OnKillfocusDeductible() 
{
	try {
		// (m.hancock 2006-11-02 11:23) - PLID 21274 - Format the currency for the interface so it displays "$xx.xx"
		CString strDeductible;
		GetDlgItemText(IDC_DEDUCTIBLE, strDeductible);
		strDeductible = FormatCurrencyForInterface(ParseCurrencyFromInterface(strDeductible));
		SetDlgItemText(IDC_DEDUCTIBLE, strDeductible);
		
	}NxCatchAll("Error in CInsuranceDlg::OnKillfocusDeductible()");
}*/

void CInsuranceDlg::OnSelChangedGenderList(long nNewSel) 
{
	//(e.lally 2007-02-20) PLID 23762 - Added function to set our changed boolean when we switch selections
	try{
		m_changed = TRUE;
	}NxCatchAll("An error occurred while changing the gender.");
}

void CInsuranceDlg::OnFocusLostGenderList() 
{
	//(e.lally 2007-02-20) PLID 23762 - When we lose focus of the gender list, we want to make our call to save.
		//only when the changed boolean is set will the save take place. That needs to be set before now.

	//Save function has error handling already, so no try/catch will be made here.
	Save(IDC_GENDER_LIST);
}

void CInsuranceDlg::OnSelChosenSecondaryReasonCode(LPDISPATCH lpRow) 
{
	try{
		//TES 6/11/2007 - PLID 26257 - Save the selected value to data.
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		CString strCode;
		if(pRow) {
			strCode = VarString(pRow->GetValue(0));
		}
		ExecuteSql("UPDATE InsuredPartyT SET SecondaryReasonCode = '%s' WHERE PersonID = %li",
			_Q(strCode),m_CurrentID);
	}NxCatchAll("Error in setting Secondary Reason Code.");
}

// (z.manning 2009-01-08 17:12) - PLID 32663
void CInsuranceDlg::UpdatePatientForHL7()
{
	try
	{
		UpdateExistingPatientInHL7(m_id, TRUE);

	}NxCatchAll("CInsuranceDlg::UpdatePatientForHL7");
}

// (j.jones 2009-03-05 09:33) - PLID 28834 - added OnKillFocusCurAccDate
void CInsuranceDlg::OnKillFocusCurAccDate()
{
	try  {

		if (!IsDateValid(m_nxtAccidentDate)) {
			//The new date is invalid, set it back to the previous value or clear it
			//if the previous value was blank.
			if(m_dtAccidentDate.GetStatus() == COleDateTime::invalid)
				m_nxtAccidentDate->Clear();
			else
				m_nxtAccidentDate->SetDateTime(m_dtAccidentDate);
			AfxMessageBox("You have entered an invalid date. The Accident Date has been reset.");
		}
		else if((long)m_nxtAccidentDate->GetDateTime() != (long)m_dtAccidentDate.m_dt) {

			//The date values are not the same, let's check the status because we still need to.
			//If one of the dates is valid, then it is a true change, otherwise even though the values are different, they
			//are both the same (invalid).
			if(m_nxtAccidentDate->GetStatus() == 1 /*valid*/ || m_dtAccidentDate.GetStatus() == COleDateTime::valid){
				m_changed = true;
				Save(IDC_EDIT_CUR_ACC_DATE);
			}
		}

	}NxCatchAll("CInsuranceDlg::OnKillFocusCurAccDate");
}

// (j.jones 2009-03-05 09:33) - PLID 28834 - added accident functions
void CInsuranceDlg::OnRadioEmployment()
{
	try {

		OnAccidentTypeChanged();

	}NxCatchAll("Error in CInsuranceDlg::OnRadioEmployment");
}

void CInsuranceDlg::OnRadioAutoAccident()
{
	try {

		OnAccidentTypeChanged();

	}NxCatchAll("Error in CInsuranceDlg::OnRadioAutoAccident");
}

void CInsuranceDlg::OnRadioOtherAccident()
{
	try {

		OnAccidentTypeChanged();

	}NxCatchAll("Error in CInsuranceDlg::OnRadioOtherAccident");
}

void CInsuranceDlg::OnRadioNone()
{
	try {

		OnAccidentTypeChanged();

	}NxCatchAll("Error in CInsuranceDlg::OnRadioNone");
}

// (j.jones 2009-03-06 10:01) - PLID 28834 - added OnAccidentTypeChanged
void CInsuranceDlg::OnAccidentTypeChanged()
{
	//throw exceptions to our caller

	InsuredPartyAccidentType ipatAccidentType = ipatNone;

	if(m_radioEmploymentAcc.GetCheck()) {
		ipatAccidentType = ipatEmployment;
	}
	else if(m_radioAutoAcc.GetCheck()) {
		ipatAccidentType = ipatAutoAcc;
	}
	else if(m_radioOtherAcc.GetCheck()) {
		ipatAccidentType = ipatOtherAcc;
	}

	//get the old value for auditing
	InsuredPartyAccidentType ipatOldAccidentType = ipatNone;
	_RecordsetPtr rs = CreateParamRecordset("SELECT AccidentType FROM InsuredPartyT WHERE PersonID = {INT}", m_CurrentID);
	if(rs->eof) {
		ASSERT(FALSE);
		//we can't update a record that doesn't exist
		return;
	}
	else {
		ipatOldAccidentType = (InsuredPartyAccidentType)AdoFldLong(rs, "AccidentType", ipatNone);
	}
	rs->Close();

	//now save the new value
	ExecuteParamSql("UPDATE InsuredPartyT SET AccidentType = {INT} WHERE PersonID = {INT}", (long)ipatAccidentType, m_CurrentID);

	//and now audit
	if(ipatOldAccidentType != ipatAccidentType) {

		CString strOldAccName, strNewAccName;

		switch(ipatOldAccidentType) {
			case ipatEmployment:
				strOldAccName = "Employment Accident";
				break;
			case ipatAutoAcc:
				strOldAccName = "Auto Accident";
				break;
			case ipatOtherAcc:
				strOldAccName = "Other Accident";
				break;
			case ipatNone:
			default:
				strOldAccName = "None";
				break;
		}

		switch(ipatAccidentType) {
			case ipatEmployment:
				strNewAccName = "Employment Accident";
				break;
			case ipatAutoAcc:
				strNewAccName = "Auto Accident";
				break;
			case ipatOtherAcc:
				strNewAccName = "Other Accident";
				break;
			case ipatNone:
			default:
				strNewAccName = "None";
				break;
		}

		long nAuditID = BeginNewAuditEvent();
		if (nAuditID != -1) {
			AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyAccidentType, m_id, strOldAccName, strNewAccName, aepMedium, aetChanged);
		}
	}
}

// (j.jones 2009-06-23 11:39) - PLID 34689 - added ability to submit as primary, when not actually primary
void CInsuranceDlg::OnCheckSubmitAsPrimary()
{
	try {

		if(m_CurrentID == -1) {
			ASSERT(FALSE);
			return;
		}

		ExecuteParamSql("UPDATE InsuredPartyT SET SubmitAsPrimary = {INT} WHERE PersonID = {INT}",
			m_checkSubmitAsPrimary.GetCheck() ? 1 : 0, m_CurrentID);

		long nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1) {
			AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartySubmitAsPrimary, m_id, m_checkSubmitAsPrimary.GetCheck() ? "Disabled" : "Enabled", m_checkSubmitAsPrimary.GetCheck() ? "Enabled" : "Disabled", aepMedium, aetChanged);
		}

	}NxCatchAll("Error in CInsuranceDlg::OnCheckSubmitAsPrimary");
}

// (j.jones 2010-07-19 10:35) - PLID 31082 - added ability to create an eligibility request
void CInsuranceDlg::OnBtnCreateInsEligibilityRequest()
{
	try {

		if(!g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrUse)) {
			return;
		}

		// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
		if(!CheckCurrentUserPermissions(bioEEligibility,sptWrite))
			return;

		// (j.jones 2016-05-19 10:22) - NX-100685 - added a universal function for getting the default E-Billing / E-Eligibility format
		long nFormatID = GetDefaultEbillingANSIFormatID();

		GetMainFrame()->ShowEligibilityRequestDlg(this,-1, 
			nFormatID, m_CurrentID, GetNxColor(GNC_PATIENT_STATUS, 1));// Use default patint dlg color

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-07-30 10:47) - PLID 39727 - edit deductible and OOP
void CInsuranceDlg::OnBnClickedInsEditDeductOop()
{
	try {

		// (j.jones 2011-12-23 14:52) - PLID 47013 - this dialog never used the correct color, now it does
		CInsuranceDeductOOPDlg dlg(m_CurrentID, m_id, m_strPatientName, m_nColor, this);
		if(IDOK == dlg.DoModal()) {
			// (j.jones 2011-12-23 14:53) - PLID 47013 - the dialog can now change the coinsurance,
			// so requery the pay group list if they clicked OK and the coinsurance changed
			if(dlg.m_bCoinsuranceChanged) {
				m_pPayGroupsList->Requery();
			}

			// (j.jones 2012-01-26 09:03) - PLID 47787 - call UpdateDeductibleOOPLabel to
			// color the button red if there is content in use
			UpdateDeductibleOOPLabel();
		}
		
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-08-05 09:30) - PLID 39729 - added new list
void CInsuranceDlg::EditingStartingPayGroupList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			//this is a global function
			return OnEditingStartingPayGroupList_Global(this, m_pPayGroupsList, pRow, nCol, pvarValue, pbContinue);
		}
		
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-08-05 09:30) - PLID 39729 - added new list
void CInsuranceDlg::EditingFinishedPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			//this is a global function
			return OnEditingFinishedPayGroupList_Global(this, m_pPayGroupsList, pRow, 
				m_id, m_strPatientName, m_CurrentID, 
				nCol, varOldValue, varNewValue, bCommit);
		}

	}NxCatchAll(__FUNCTION__);
}


// (j.gruber 2010-08-05 09:30) - PLID 39729 - added new list
void CInsuranceDlg::EditingFinishingPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			//this a global function
			return OnEditingFinishingPayGroupList_Global(this, m_pPayGroupsList, pRow, nCol, varOldValue, strUserEntered, pvarNewValue, pbCommit, pbContinue);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-01-26 09:03) - PLID 47787 - UpdateDeductibleOOPLabel can
// color the button red if there is content in use
void CInsuranceDlg::UpdateDeductibleOOPLabel()
{
	try {

		//if any field is filled out, set the color to red
		// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading Insurance dialog
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Convert(bit, Max(CASE WHEN "
			"(DeductiblePerPayGroup = 0 "
			"	AND (InsuredPartyT.DeductibleRemaining Is Not Null OR InsuredPartyT.TotalDeductible Is Not Null "
			"	OR InsuredPartyT.OOPRemaining Is Not Null OR InsuredPartyT.TotalOOP Is Not Null)) "
			"OR "
			"(DeductiblePerPayGroup = 1 "
			"	AND (InsuredPartyPayGroupsT.DeductibleRemaining Is Not Null OR InsuredPartyPayGroupsT.TotalDeductible Is Not Null "
			"	OR InsuredPartyPayGroupsT.OOPRemaining Is Not Null OR InsuredPartyPayGroupsT.TotalOOP Is Not Null)) "
			"THEN 1 ELSE 0 END)) AS InUse "
			"FROM InsuredPartyT "
			"LEFT JOIN InsuredPartyPayGroupsT ON InsuredPartyT.PersonID = InsuredPartyPayGroupsT.InsuredPartyID "
			"WHERE InsuredPartyT.PersonID = {INT} "
			"GROUP BY InsuredPartyT.PersonID ", m_CurrentID);
		BOOL bInUse = FALSE;
		if(!rs->eof) {
			bInUse = VarBool(rs->Fields->Item["InUse"]->Value);
		}
		rs->Close();

		if(!bInUse) {
			m_btnDeductOOP.SetWindowText("Deductible/OOP");
			m_btnDeductOOP.AutoSet(NXB_MODIFY);
		}
		else {
			m_btnDeductOOP.SetWindowText("Deductible/OOP (In Use)");
			m_btnDeductOOP.SetTextColor(RGB(255,0,0));
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-11-12 10:49) - PLID 53622 - added country dropdown
void CInsuranceDlg::OnSelChangingCountryList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll( __FUNCTION__);
}

// (j.jones 2012-11-12 10:49) - PLID 53622 - added country dropdown
void CInsuranceDlg::OnSelChosenCountryList(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pCurSel(lpRow);
		_variant_t vtNewValue = g_cvarNull;
		//blank strings are not allowed, have to save NULL in that case
		if(pCurSel != NULL && VarString(pCurSel->GetValue(cccName), "") != "") {
			vtNewValue = pCurSel->GetValue(cccName);
		}

		//update the country, and get the old value
		_RecordsetPtr prs = CreateParamRecordset(
			"UPDATE PersonT SET Country = {VT_BSTR} OUTPUT deleted.Country WHERE ID = {INT}",
			vtNewValue, m_CurrentID);

		if(!prs->eof) {
			_variant_t vtOldValue = prs->Fields->Item["Country"]->Value;
			if(vtOldValue != vtNewValue) {
				long nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1) {
					AuditEvent(m_id, m_strPatientName, nAuditID, aeiInsPartyCountry, m_id, VarString(vtOldValue, ""), VarString(vtNewValue, ""), aepMedium, aetChanged);
				}

				//not needed, we don't send the country in HL7
				//UpdatePatientForHL7();
			}
		}

	}NxCatchAll( __FUNCTION__);
}

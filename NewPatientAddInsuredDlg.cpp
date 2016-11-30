// NewPatientAddInsuredDlg.cpp : implementation file
//

// (j.gruber 2009-10-13 09:34) - PLID 10723 - created for
// (c.haag 2010-10-04 10:16) - PLID 39447 - Expanded functionality to include
// more fields

#include "stdafx.h"
#include "Practice.h"
#include "NewPatientAddInsuredDlg.h"
#include "EditInsInfoDlg.h"
#include "PatientsRc.h"

// (c.haag 2010-10-04 10:16) - PLID 39447 - Insurance-related list column enumerations
// (j.gruber 2012-08-01 15:20) - PLID 51908 - added color and can add
enum RespListColumn {
	rlcID = 0,
	rlcName,
	rlcPriority,
	rlcCategory,
	rlsColor,
	rlsCanAdd,
};

// (r.goldschmidt 2014-07-30 08:40) - PLID 62775 - added deductible, oop, perpaygroup
enum InsuranceListColumn {
	ilcID = 0,
	ilcName,
	ilcAddress,
	ilcTotalDeductible,
	ilcTotalOOP,
	ilcPerPayGroup
};

// (j.jones 2012-11-12 13:38) - PLID 53622 - added country dropdown
enum CountryComboColumn {
	cccID = 0,
	cccName,
	cccISOCode,
};

// (r.goldschmidt 2014-07-23 15:58) - PLID 62775 - Add Pay Group List
enum PayGroupsColumn {
	pgcID = 0,
	pgcName,
	pgcCoInsPercent,
	pgcCopayMoney,
	pgcCopayPercent,
	pgcTotalDeductible,
	pgcTotalOOP
};

// CNewPatientAddInsuredDlg dialog

IMPLEMENT_DYNAMIC(CNewPatientAddInsuredDlg, CNxDialog)

// (j.gruber 2012-08-01 15:20) - PLID 51908 - added from new patient and patientID
CNewPatientAddInsuredDlg::CNewPatientAddInsuredDlg(
	CNewPatientInsuredParty& party, CNewPatientInsuredParty& patient,
	CWnd* pParent, BOOL bFromNewPatientDlg /*=TRUE*/, long nExistingPatientID /*=-1*/)
	: CNxDialog(CNewPatientAddInsuredDlg::IDD, pParent),
	m_Party(party),m_Patient(patient), m_bFromNewPatientDlg(bFromNewPatientDlg), m_nExistingPatientID(nExistingPatientID)
{
}

CNewPatientAddInsuredDlg::~CNewPatientAddInsuredDlg()
{
}

// (c.haag 2010-10-04 15:38) - PLID 39447 - Enables the patient demographic fields
void CNewPatientAddInsuredDlg::EnableControls(BOOL bEnable)
{
	GetDlgItem(IDC_FIRST_NAME_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_MIDDLE_NAME_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_LAST_NAME_BOX)->EnableWindow(bEnable);
	// (j.jones 2012-10-25 10:50) - PLID 36305 - added Title
	GetDlgItem(IDC_TITLE_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_ADDRESS1_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_ADDRESS2_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_CITY_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_STATE_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_ZIP_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_PHONE_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_SSN_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_EMPLOYER_SCHOOL_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_BIRTH_DATE_BOX)->EnableWindow(bEnable);
	GetDlgItem(IDC_GENDER_LIST)->EnableWindow(bEnable);
	GetDlgItem(IDC_COPY_PATIENT_INFO)->EnableWindow(bEnable);
	// (j.jones 2012-11-12 10:49) - PLID 53622 - added country dropdown
	GetDlgItem(IDC_COUNTRY_LIST)->EnableWindow(bEnable);
}

void CNewPatientAddInsuredDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FIRST_NAME_BOX, m_edtFirst);
	DDX_Control(pDX, IDC_MIDDLE_NAME_BOX, m_edtMiddle);
	DDX_Control(pDX, IDC_LAST_NAME_BOX, m_edtLast);
	DDX_Control(pDX, IDC_TITLE_BOX, m_edtTitle);
	DDX_Control(pDX, IDC_ADDRESS1_BOX, m_edtAddress1);
	DDX_Control(pDX, IDC_ADDRESS2_BOX, m_edtAddress2);
	DDX_Control(pDX, IDC_ZIP_BOX, m_edtZip);
	DDX_Control(pDX, IDC_CITY_BOX, m_edtCity);
	DDX_Control(pDX, IDC_STATE_BOX, m_edtState);
	DDX_Control(pDX, IDC_EMPLOYER_SCHOOL_BOX, m_edtEmployer);
	DDX_Control(pDX, IDC_SSN_BOX, m_edtSSN);
	DDX_Control(pDX, IDC_PHONE_BOX, m_edtPhone);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_COPY_PATIENT_INFO, m_btnCopyPatientInfo); 
	DDX_Control(pDX, IDC_ALL_PAY_GROUPS_RADIO, m_radioAllPaygroups);
	DDX_Control(pDX, IDC_INDIVIDUAL_PAY_GROUPS_RADIO, m_radioIndividualPaygroups);
	DDX_Control(pDX, IDC_TOTAL_DEDUCTIBLE_EDIT, m_edtTotalDeductible);
	DDX_Control(pDX, IDC_TOTAL_OOP_EDIT, m_edtTotalOOP);
}


BEGIN_MESSAGE_MAP(CNewPatientAddInsuredDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CNewPatientAddInsuredDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CNewPatientAddInsuredDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_COPY_PATIENT_INFO, OnCopyPatientInfo)
	ON_EN_SETFOCUS(IDC_PHONE_BOX, OnSetFocusPhoneNumber)	
	ON_EN_KILLFOCUS(IDC_ZIP_BOX, OnKillFocusZipCode)	
	ON_EN_KILLFOCUS(IDC_CITY_BOX, OnKillFocusCityBox)	
	ON_EN_CHANGE(IDC_SSN_BOX, OnChangeSSN)
	ON_BN_CLICKED(IDC_ALL_PAY_GROUPS_RADIO, &CNewPatientAddInsuredDlg::OnBnClickedAllPayGroupsRadio)
	ON_BN_CLICKED(IDC_INDIVIDUAL_PAY_GROUPS_RADIO, &CNewPatientAddInsuredDlg::OnBnClickedIndividualPayGroupsRadio)
	ON_EN_KILLFOCUS(IDC_TOTAL_DEDUCTIBLE_EDIT, &CNewPatientAddInsuredDlg::OnEnKillfocusTotalDeductibleEdit)
	ON_EN_KILLFOCUS(IDC_TOTAL_OOP_EDIT, &CNewPatientAddInsuredDlg::OnEnKillfocusTotalOopEdit)
	ON_EN_KILLFOCUS(IDC_SSN_BOX, &CNewPatientAddInsuredDlg::OnEnKillfocusSsnBox)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CNewPatientAddInsuredDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNewPatient)
	ON_EVENT(CNewPatientAddInsuredDlg, IDC_GENDER_LIST, 1, CNewPatientAddInsuredDlg::SelChangingGenderList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CNewPatientAddInsuredDlg, IDC_NEWPAT_INS_CO_LIST, 1, CNewPatientAddInsuredDlg::SelChangingNewpatInsCoList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CNewPatientAddInsuredDlg, IDC_NEWPAT_INS_CO_LIST, 16, CNewPatientAddInsuredDlg::SelChosenNewpatInsCoList, VTS_DISPATCH)
	ON_EVENT(CNewPatientAddInsuredDlg, IDC_NEWPAT_INS_CO_LIST, 18, CNewPatientAddInsuredDlg::RequeryFinishedNewpatInsCoList, VTS_I2)
	ON_EVENT(CNewPatientAddInsuredDlg, IDC_NEWPAT_INS_RELATION, 1, CNewPatientAddInsuredDlg::SelChangingNewpatInsRelation, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CNewPatientAddInsuredDlg, IDC_NEWPAT_INS_RELATION, 16, SelChosenNewpatInsRelation, VTS_DISPATCH)
	ON_EVENT(CNewPatientAddInsuredDlg, IDC_COUNTRY_LIST, 1, OnSelChangingCountryList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CNewPatientAddInsuredDlg, IDC_NEWPAT_PAY_GROUP_LIST, 8, CNewPatientAddInsuredDlg::EditingStartingNewpatPayGroupList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CNewPatientAddInsuredDlg, IDC_NEWPAT_PAY_GROUP_LIST, 9, CNewPatientAddInsuredDlg::EditingFinishingNewpatPayGroupList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CNewPatientAddInsuredDlg, IDC_NEWPAT_PAY_GROUP_LIST, 18, CNewPatientAddInsuredDlg::RequeryFinishedNewpatPayGroupList, VTS_I2)
END_EVENTSINK_MAP()


// CNewPatientAddInsuredDlg message handlers
BOOL CNewPatientAddInsuredDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnCopyPatientInfo.AutoSet(NXB_MODIFY);

		m_nxtBirthDate = GetDlgItemUnknown(IDC_BIRTH_DATE_BOX);	

		m_pGenderList = BindNxDataList2Ctrl(IDC_GENDER_LIST, false);

		// (j.jones 2012-11-12 10:49) - PLID 53622 - added country dropdown
		m_pCountryList = BindNxDataList2Ctrl(IDC_COUNTRY_LIST, true);
		NXDATALIST2Lib::IRowSettingsPtr pCountryRow = m_pCountryList->GetNewRow();
		pCountryRow->PutValue(cccID, (long)-1);
		pCountryRow->PutValue(cccName, (LPCTSTR)"");	//ideally should be null, but set as an empty string for sorting
		pCountryRow->PutValue(cccISOCode, (LPCTSTR)"");
		m_pCountryList->AddRowSorted(pCountryRow, NULL);

		// (c.haag 2010-10-04 12:09) - PLID 39447 - Responsibility dropdown
		// (j.gruber 2012-07-25 16:42) - PLID 51777 - new categories
		// (j.gruber 2012-08-01 15:13) - PLID 51908 - grey out existing resps
		m_pRespList = BindNxDataList2Ctrl(IDC_COMBO_NEW_INS_RESP_TYPE, false);
		if (m_nExistingPatientID == -1 ){ 
			m_pRespList->FromClause = "(SELECT ID, TypeName, Priority, "
				" 0 AS Color, 1 as CanAdd, "
				" CASE WHEN Priority IN (1,2) THEN 'Medical' "
				" WHEN Priority = -1 THEN '' ELSE CASE "
				" WHEN CategoryType = 2 THEN 'Vision' "
				" WHEN CategoryType = 3 THEN 'Auto' "
				" WHEN CategoryType = 4 THEN 'Workers'' Comp.'  "
				" WHEN CategoryType = 5 THEN 'Dental'  "
				" WHEN CategoryType = 6 THEN 'Study'  "
				" WHEN CategoryType = 7 THEN 'Letter of Protection'  "
				" WHEN CategoryType = 8 THEN 'Letter of Agreement'  "
				" ELSE 'Medical' END END AS CategoryType FROM RespTypeT) AS RespTypeQ";
		}
		else {
			CString strFrom;
			strFrom.Format("(SELECT ID, TypeName, Priority, "
				" CASE WHEN PatRespQ.RespTypeID IS NULL THEN 0 ELSE 7303023 END AS Color, "
				" CASE WHEN PatRespQ.RespTypeID IS NULL THEN 1 ELSE 0 END AS CanAdd, "
				" CASE WHEN Priority IN (1,2) THEN 'Medical' "
				" WHEN Priority = -1 THEN '' ELSE CASE "
				" WHEN CategoryType = 2 THEN 'Vision' "
				" WHEN CategoryType = 3 THEN 'Auto' "
				" WHEN CategoryType = 4 THEN 'Workers'' Comp.'  "
				" WHEN CategoryType = 5 THEN 'Dental'  "
				" WHEN CategoryType = 6 THEN 'Study'  "
				" WHEN CategoryType = 7 THEN 'Letter of Protection'  "
				" WHEN CategoryType = 8 THEN 'Letter of Agreement'  "
				" ELSE 'Medical' END END AS CategoryType FROM RespTypeT "
				" LEFT JOIN (SELECT RespTypeID FROM InsuredPartyT WHERE PatientID = %li AND RespTypeID != -1) PatRespQ "
				" ON RespTypeT.ID = PatRespQ.RespTypeID "		
				" ) AS RespTypeQ", m_nExistingPatientID);
			m_pRespList->FromClause = _bstr_t(strFrom);
			
		}

		m_pRespList->Requery();

		// (r.goldschmidt 2014-07-23 16:05) - PLID 62775 - Pay Group List
		m_pPayGroupsList = BindNxDataList2Ctrl(IDC_NEWPAT_PAY_GROUP_LIST, false);
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pPayGroupsList->GetColumn(pgcCopayMoney);
		if (pCol) {
			pCol->ColumnTitle = _bstr_t("Copay " + GetCurrencySymbol());
		}
		m_pPayGroupsList->Requery();
	
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pGenderList->GetNewRow();
		_variant_t var = _bstr_t("");
		pRow->PutValue(0, (long)0);
		pRow->PutValue(1, var);
		m_pGenderList->AddRowAtEnd(pRow, NULL);

		pRow = m_pGenderList->GetNewRow();
		var = _bstr_t("Male");
		pRow->PutValue(0, (long)1);
		pRow->PutValue(1, var);
		m_pGenderList->AddRowAtEnd(pRow, NULL);
		
		pRow = m_pGenderList->GetNewRow();
		pRow->PutValue(0, (long)2);
		var = _bstr_t("Female");
		pRow->PutValue(1, var);
		m_pGenderList->AddRowAtEnd(pRow, NULL);

		g_propManager.CachePropertiesInBulk("NewPatientAddInsured", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'LookupZipStateByCity' OR "
			"Name = 'FormatPhoneNums' OR "			
			"Name = 'AutoCapitalizeMiddleInitials' OR "
			// (j.jones 2011-06-24 16:45) - PLID 31005 - added AutoCapitalizeInsuranceIDs
			"Name = 'AutoCapitalizeInsuranceIDs' "
			")",
			_Q(GetCurrentUserName()));


		m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");
		m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true); 
		m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);
		m_bAutoCapMiddle = GetRemotePropertyInt("AutoCapitalizeMiddleInitials", 1, 0, "<None>", true);

		if (m_bLookupByCity) {
			ChangeZOrder(IDC_ZIP_BOX, IDC_STATE_BOX);
		} else {
			ChangeZOrder(IDC_ZIP_BOX, IDC_ADDRESS2_BOX);
		}

		m_edtFirst.SetLimitText(50);
		m_edtMiddle.SetLimitText(50);
		m_edtLast.SetLimitText(50);
		// (j.jones 2012-10-25 10:50) - PLID 36305 - added Title
		m_edtTitle.SetLimitText(50);
		m_edtAddress1.SetLimitText(75);
		m_edtAddress2.SetLimitText(75);
		m_edtCity.SetLimitText(50);
		m_edtState.SetLimitText(20);
		m_edtZip.SetLimitText(20);
		m_edtPhone.SetLimitText(20);
		m_edtEmployer.SetLimitText(50);		

		// (c.haag 2010-10-04 10:16) - PLID 39447 - Insurance-related lists
		m_pInsCoList = BindNxDataList2Ctrl(IDC_NEWPAT_INS_CO_LIST, false);
		m_pRelationToPatientList = BindNxDataList2Ctrl(IDC_NEWPAT_INS_RELATION, false);
		LoadRelToPatList();
		m_pInsCoList->Requery();

		// (c.haag 2010-10-04 12:59) - PLID 39447 - Load the default values from m_Party
		LoadDemographics(m_Party);
		LoadInsurance(m_Party);
		LoadPayGroupInfo(m_Party);

		// (c.haag 2010-10-04 12:59) - PLID 39447 - Now enable/disable the demographic controls
		BOOL bEnable = TRUE;
		if (NULL != m_pRelationToPatientList->CurSel &&
			VarString(m_pRelationToPatientList->CurSel->GetValue(0),"") == "Self")
		{
			bEnable = FALSE;
		}
		EnableControls(bEnable);

		// (r.goldschmidt 2014-07-28 15:06) - PLID 62775 - enable/disable pay group controls

		if (NULL == m_pInsCoList->CurSel ||
			VarLong(m_pInsCoList->CurSel->GetValue(ilcID), -1) == -1) {
			// no insurance company selected, disable all
			DisablePayGroupControls();
		}
		else {
			// toggle based on per pay group radio button
			TogglePayGroupControls();
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (c.haag 2010-10-04 12:59) - PLID 39447 - Loads the default values from m_Party when
// the dialog is initialized
void CNewPatientAddInsuredDlg::LoadDemographics(CNewPatientInsuredParty& party)
{
	SetDlgItemText(IDC_FIRST_NAME_BOX, party.m_strInsFirst);
	SetDlgItemText(IDC_MIDDLE_NAME_BOX, party.m_strInsMiddle);
	SetDlgItemText(IDC_LAST_NAME_BOX, party.m_strInsLast);
	// (j.jones 2012-10-25 10:50) - PLID 36305 - added Title
	SetDlgItemText(IDC_TITLE_BOX, party.m_strInsTitle);
	SetDlgItemText(IDC_ADDRESS1_BOX, party.m_strInsAddress1);
	SetDlgItemText(IDC_ADDRESS2_BOX, party.m_strInsAddress2);
	SetDlgItemText(IDC_CITY_BOX, party.m_strInsCity);
	SetDlgItemText(IDC_STATE_BOX, party.m_strInsState);
	SetDlgItemText(IDC_ZIP_BOX, party.m_strInsZip);
	// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
	m_pCountryList->SetSelByColumn(cccName, (LPCSTR)party.m_strInsCountry);
	SetDlgItemText(IDC_PHONE_BOX, party.m_strInsPhone);
	// (s.tullis 2016-02-11 11:43) - PLID 68212 - Don't set this if our SSN value is empty
	if (!party.m_strInsSSN.IsEmpty()) {
		SetDlgItemText(IDC_SSN_BOX, party.m_strInsSSN);
	}
	SetDlgItemText(IDC_EMPLOYER_SCHOOL_BOX, party.m_strInsEmployer);
	if (party.m_dtInsBirthDate.GetStatus() == COleDateTime::valid)
	{
		m_nxtBirthDate->SetDateTime(party.m_dtInsBirthDate);
	}
	else {
		m_nxtBirthDate->Clear();
	}
	m_pGenderList->SetSelByColumn(0, party.m_nInsGender);
	if (!party.m_strRelationToPt.IsEmpty()) {
		m_pRelationToPatientList->SetSelByColumn(0, _bstr_t(party.m_strRelationToPt));
	}
}

// (c.haag 2010-10-04 12:59) - PLID 39447 - Loads the insurance values from the party object
void CNewPatientAddInsuredDlg::LoadInsurance(CNewPatientInsuredParty& party)
{
	SetDlgItemText(IDC_NEWPAT_INS_ID_BOX, party.m_strPatientIDNumber);
	SetDlgItemText(IDC_NEWPAT_INS_FECA_BOX, party.m_strGroupNumber);
	m_pRespList->SetSelByColumn(rlcID, party.GetRespTypeID());
	m_pInsCoList->SetSelByColumn(ilcID, party.GetInsuranceCompanyID());	
}

// (r.goldschmidt 2014-07-28 13:26) - PLID 62775 - Loads the deductible/pay group info from the party object
void CNewPatientAddInsuredDlg::LoadPayGroupInfo(CNewPatientInsuredParty& party)
{
	m_strTotalDeductible = "";
	if (party.m_cyTotalDeductible.GetStatus() == COleCurrency::valid) {
		if (party.m_cyTotalDeductible >= COleCurrency(0, 0)){
			m_strTotalDeductible = FormatCurrencyForInterface(party.m_cyTotalDeductible);			
		}
	}
	SetDlgItemText(IDC_TOTAL_DEDUCTIBLE_EDIT, m_strTotalDeductible);

	m_strTotalOOP = "";
	if (party.m_cyTotalOOP.GetStatus() == COleCurrency::valid) {
		if (party.m_cyTotalOOP >= COleCurrency(0, 0)){
			m_strTotalOOP = FormatCurrencyForInterface(party.m_cyTotalOOP);
		}
	}
	SetDlgItemText(IDC_TOTAL_OOP_EDIT, m_strTotalOOP);

	m_radioIndividualPaygroups.SetCheck(party.m_bPerPayGroup);
	m_radioAllPaygroups.SetCheck(!party.m_bPerPayGroup);

	POSITION pos = party.m_mapPayGroupVals.GetStartPosition();
	long nID;
	InsuredPartyPayGroupValues sPayGroupVals;
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	while (pos != NULL){
		party.m_mapPayGroupVals.GetNextAssoc(pos, nID, sPayGroupVals);
		pRow = m_pPayGroupsList->FindByColumn(pgcID, nID, NULL, FALSE);
		if (sPayGroupVals.isCoInsPercentValid()) {
			pRow->PutValue(pgcCoInsPercent, variant_t(sPayGroupVals.m_nCoInsPercent));
		}
		if (sPayGroupVals.isCopayMoneyValid()) {
			pRow->PutValue(pgcCopayMoney, variant_t(sPayGroupVals.m_cyCopayMoney));
		}
		if (sPayGroupVals.isCopayPercentValid()) {
			pRow->PutValue(pgcCopayPercent, variant_t(sPayGroupVals.m_nCopayPercent));
		}
		if (sPayGroupVals.isTotalDeductibleValid()) {
			pRow->PutValue(pgcTotalDeductible, variant_t(sPayGroupVals.m_cyTotalDeductible));
		}
		if (sPayGroupVals.isTotalOOPValid()) {
			pRow->PutValue(pgcTotalOOP, variant_t(sPayGroupVals.m_cyTotalOOP));
		}
	}
}

// (c.haag 2010-10-04 15:59) - PLID 39447 - Copys patient information into the
// demographic fields
void CNewPatientAddInsuredDlg::CopyPatientInfo()
{
	SetDlgItemText(IDC_FIRST_NAME_BOX, m_Patient.m_strInsFirst);
	SetDlgItemText(IDC_MIDDLE_NAME_BOX, m_Patient.m_strInsMiddle);
	SetDlgItemText(IDC_LAST_NAME_BOX, m_Patient.m_strInsLast);
	// (j.jones 2012-10-25 10:50) - PLID 36305 - added Title, though the New Patient screen does not have this field
	SetDlgItemText(IDC_TITLE_BOX, m_Patient.m_strInsTitle);
	SetDlgItemText(IDC_ADDRESS1_BOX, m_Patient.m_strInsAddress1);
	SetDlgItemText(IDC_ADDRESS2_BOX, m_Patient.m_strInsAddress2);
	SetDlgItemText(IDC_CITY_BOX, m_Patient.m_strInsCity);
	SetDlgItemText(IDC_STATE_BOX, m_Patient.m_strInsState);
	SetDlgItemText(IDC_ZIP_BOX, m_Patient.m_strInsZip);
	// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
	m_pCountryList->SetSelByColumn(cccName, (LPCSTR)m_Patient.m_strInsCountry);
	SetDlgItemText(IDC_PHONE_BOX, m_Patient.m_strInsPhone);
	SetDlgItemText(IDC_SSN_BOX, m_Patient.m_strInsSSN);
	SetDlgItemText(IDC_EMPLOYER_SCHOOL_BOX, m_Patient.m_strInsEmployer);
	if (m_Patient.m_dtInsBirthDate.GetStatus() == COleDateTime::valid)
	{
		m_nxtBirthDate->SetDateTime(m_Patient.m_dtInsBirthDate);
	}
	else {
		m_nxtBirthDate->Clear();
	}
	m_pGenderList->SetSelByColumn(0, m_Patient.m_nInsGender);			
}

// (r.goldschmidt 2014-07-28 15:34) - PLID 62775 - disable pay group controls (for when insurance company is set to none)
void CNewPatientAddInsuredDlg::DisablePayGroupControls()
{
	GetDlgItem(IDC_ALL_PAY_GROUPS_RADIO)->EnableWindow(FALSE);
	GetDlgItem(IDC_INDIVIDUAL_PAY_GROUPS_RADIO)->EnableWindow(FALSE);
	GetDlgItem(IDC_TOTAL_DEDUCTIBLE_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_TOTAL_OOP_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_NEWPAT_PAY_GROUP_LIST)->EnableWindow(FALSE);
}

// (r.goldschmidt 2014-07-28 15:34) - PLID 62775 - clear and disable pay group controls (for when insurance company is set to none)
void CNewPatientAddInsuredDlg::ClearAndDisablePayGroupControls()
{
	m_radioIndividualPaygroups.SetCheck(TRUE);
	m_radioAllPaygroups.SetCheck(FALSE);
	SetDlgItemText(IDC_TOTAL_DEDUCTIBLE_EDIT, "");
	SetDlgItemText(IDC_TOTAL_OOP_EDIT, "");
	TogglePayGroupControls(); // to restore pay group list columns after reset of radio button
	DisablePayGroupControls();
}

// (r.goldschmidt 2014-07-28 15:34) - PLID 62775 - toggle pay group controls depending on radio button status
void CNewPatientAddInsuredDlg::TogglePayGroupControls()
{
	bool bPerPayGroup = !!m_radioIndividualPaygroups.GetCheck();

	GetDlgItem(IDC_ALL_PAY_GROUPS_RADIO)->EnableWindow(TRUE);
	GetDlgItem(IDC_INDIVIDUAL_PAY_GROUPS_RADIO)->EnableWindow(TRUE);
	GetDlgItem(IDC_NEWPAT_PAY_GROUP_LIST)->EnableWindow(TRUE);
	GetDlgItem(IDC_TOTAL_DEDUCTIBLE_EDIT)->EnableWindow(!bPerPayGroup);
	GetDlgItem(IDC_TOTAL_OOP_EDIT)->EnableWindow(!bPerPayGroup);
	
	NXDATALIST2Lib::IColumnSettingsPtr pColName, pColCoIns, pColCopayMoney, pColCopayPercent, pColTotalDeduct, pColTotalOOP;
	pColName = m_pPayGroupsList->GetColumn(pgcName);
	pColCoIns = m_pPayGroupsList->GetColumn(pgcCoInsPercent);
	pColCopayMoney = m_pPayGroupsList->GetColumn(pgcCopayMoney);
	pColCopayPercent = m_pPayGroupsList->GetColumn(pgcCopayPercent);
	pColTotalDeduct = m_pPayGroupsList->GetColumn(pgcTotalDeductible);
	pColTotalOOP = m_pPayGroupsList->GetColumn(pgcTotalOOP);

	long csTotalDeduct = pColTotalDeduct->ColumnStyle;
	long csTotalOOP = pColTotalOOP->ColumnStyle;

	// format columns
	if (bPerPayGroup) {
		pColName->PutStoredWidth(18);
		pColCoIns->PutStoredWidth(12);
		pColCopayMoney->PutStoredWidth(12);
		pColCopayPercent->PutStoredWidth(12);
		pColTotalDeduct->PutStoredWidth(22);
		pColTotalOOP->PutStoredWidth(24);
		pColTotalDeduct->ColumnStyle = csTotalDeduct | NXDATALIST2Lib::EColumnStyle::csEditable;
		pColTotalOOP->ColumnStyle = csTotalOOP | NXDATALIST2Lib::EColumnStyle::csEditable;
	}
	else {
		pColName->PutStoredWidth(36);
		pColCoIns->PutStoredWidth(21);
		pColCopayMoney->PutStoredWidth(21);
		pColCopayPercent->PutStoredWidth(21);
		pColTotalDeduct->PutStoredWidth(0);
		pColTotalOOP->PutStoredWidth(0);
		pColTotalDeduct->ColumnStyle = csTotalDeduct & (~NXDATALIST2Lib::EColumnStyle::csEditable);
		pColTotalOOP->ColumnStyle = csTotalOOP & (~NXDATALIST2Lib::EColumnStyle::csEditable);
	}

}

// (c.haag 2010-10-04 12:31) - PLID 39447 - Changed code to pass information into m_Party
void CNewPatientAddInsuredDlg::OnBnClickedOk()
{
	try {

		//check the birthdate
		COleDateTime dt, dtNow;		

		// (c.haag 2010-10-04 12:31) - PLID 39447 - Insurance fields validation
		if (NULL == m_pRespList->CurSel) {
			AfxMessageBox("You must select an insurance placement.", MB_OK | MB_ICONSTOP);
			GetDlgItem(IDC_COMBO_NEW_INS_RESP_TYPE)->SetFocus();
			return;
		}

		// (j.gruber 2012-08-01 15:27) - PLID 51908 - make sure said placement is valid
		NXDATALIST2Lib::IRowSettingsPtr pRowCheck(m_pRespList->CurSel);
		if (pRowCheck) {
			long nCanAdd = VarLong(pRowCheck->GetValue(rlsCanAdd));
			if (nCanAdd == 0) {
				AfxMessageBox("The placement you selected has already been used by this patient.\r\n"
				"Please select a placement that has not already been used. ");
				return;
			}
		}



		if (NULL == m_pInsCoList->CurSel || VarLong(m_pInsCoList->CurSel->GetValue(ilcID),0) <= 0) {
			AfxMessageBox("You must select an insurance company.", MB_OK | MB_ICONSTOP);
			GetDlgItem(IDC_NEWPAT_INS_CO_LIST)->SetFocus();
			return;
		}

		if (NULL == m_pRelationToPatientList->CurSel) {
			AfxMessageBox("You must select a relationship.", MB_OK | MB_ICONSTOP);
			GetDlgItem(IDC_NEWPAT_INS_RELATION)->SetFocus();
			return;
		}

		// (c.haag 2010-10-13 10:04) - PLID 39447 - Don't validate demographics if we're self responsibility
		// because we have no control over those values.
		if (	VarString(m_pRelationToPatientList->CurSel->GetValue(0),"") != "Self")
		{
			if(m_nxtBirthDate->GetStatus() == 2) {
				MessageBox("You have entered an invalid birthdate");
				m_nxtBirthDate->Clear();
				return;
			}

			if (m_nxtBirthDate->GetStatus() == 1) {
				dt = m_nxtBirthDate->GetDateTime();
				dtNow = COleDateTime::GetCurrentTime();

				if(dt > dtNow) {
					AfxMessageBox("You have entered a birthdate in the future. This will be adjusted to a valid date.");

					while(dt > dtNow) {
						dt.SetDate(dt.GetYear() - 100, dt.GetMonth(), dt.GetDay());
					}
				}

				m_Party.m_dtInsBirthDate = dt;
			}
			else {
				m_Party.m_dtInsBirthDate = COleDateTime(0,0,0,0,0,0);
			}
		}

		//set our variables
		GetDlgItemText(IDC_FIRST_NAME_BOX, m_Party.m_strInsFirst);
		GetDlgItemText(IDC_MIDDLE_NAME_BOX, m_Party.m_strInsMiddle);
		GetDlgItemText(IDC_LAST_NAME_BOX, m_Party.m_strInsLast);
		// (j.jones 2012-10-25 10:50) - PLID 36305 - added Title
		GetDlgItemText(IDC_TITLE_BOX, m_Party.m_strInsTitle);
		GetDlgItemText(IDC_ADDRESS1_BOX, m_Party.m_strInsAddress1);
		GetDlgItemText(IDC_ADDRESS2_BOX, m_Party.m_strInsAddress2);
		GetDlgItemText(IDC_CITY_BOX, m_Party.m_strInsCity);
		GetDlgItemText(IDC_STATE_BOX, m_Party.m_strInsState);
		GetDlgItemText(IDC_ZIP_BOX, m_Party.m_strInsZip);

		// (j.jones 2012-11-12 13:32) - PLID 53622 - added Country
		NXDATALIST2Lib::IRowSettingsPtr pCountryRow = m_pCountryList->GetCurSel();
		m_Party.m_strInsCountry = "";
		if(pCountryRow) {
			m_Party.m_strInsCountry = pCountryRow->GetValue(cccName);
		}

		GetDlgItemText(IDC_PHONE_BOX, m_Party.m_strInsPhone);
		GetDlgItemText(IDC_SSN_BOX, m_Party.m_strInsSSN);
		GetDlgItemText(IDC_EMPLOYER_SCHOOL_BOX, m_Party.m_strInsEmployer);
		
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pGenderList->CurSel;
		if (pRow) {
			m_Party.m_nInsGender = VarLong(pRow->GetValue(0), 0);
		}				
		else {
			m_Party.m_nInsGender = 0;
		}

		// (c.haag 2010-10-04 12:31) - PLID 39447 - Added insurance-related variables.
		// The row selections for the insurance dropdowns cannot be null.
		GetDlgItemText(IDC_NEWPAT_INS_ID_BOX, m_Party.m_strPatientIDNumber);
		GetDlgItemText(IDC_NEWPAT_INS_FECA_BOX, m_Party.m_strGroupNumber);

		pRow = m_pRelationToPatientList->CurSel;
		m_Party.m_strRelationToPt = VarString(pRow->GetValue(0),"");

		pRow = m_pRespList->CurSel;
		m_Party.SetRespType(VarLong(pRow->GetValue(rlcID)), 
			VarString(pRow->GetValue(rlcName),""));

		pRow = m_pInsCoList->CurSel;
		m_Party.SetInsuranceCompany(VarLong(pRow->GetValue(ilcID)), 
			VarString(pRow->GetValue(ilcName),""));

		// (r.goldschmidt 2014-07-24 12:29) - PLID 62775 - Added deductible/pay group values from list
		if (m_strTotalDeductible.IsEmpty()){
			m_Party.m_cyTotalDeductible = COleCurrency();
			m_Party.m_cyTotalDeductible.SetStatus(COleCurrency::invalid);
		}
		else {
			m_Party.m_cyTotalDeductible = ParseCurrencyFromInterface(m_strTotalDeductible);
		}
		if (m_strTotalOOP.IsEmpty()){
			m_Party.m_cyTotalOOP = COleCurrency();
			m_Party.m_cyTotalOOP.SetStatus(COleCurrency::invalid);
		}
		else {
			m_Party.m_cyTotalOOP = ParseCurrencyFromInterface(m_strTotalOOP);
		}
		m_Party.m_bPerPayGroup = !!m_radioIndividualPaygroups.GetCheck();
		m_Party.m_mapPayGroupVals.RemoveAll();
		pRow = m_pPayGroupsList->FindAbsoluteFirstRow(VARIANT_TRUE);
		InsuredPartyPayGroupValues sPayGroupValsInvalid; //constructor automatically builds invalid values; invalid values are set to g_cvarNull in db
		while (pRow != NULL){
			long nID = VarLong(pRow->GetValue(pgcID), -1);
			InsuredPartyPayGroupValues sPayGroupVals;
			sPayGroupVals.m_strName = VarString(pRow->GetValue(pgcName), "");
			sPayGroupVals.m_nCoInsPercent = VarLong(pRow->GetValue(pgcCoInsPercent), sPayGroupValsInvalid.m_nCoInsPercent);
			sPayGroupVals.m_cyCopayMoney = VarCurrency(pRow->GetValue(pgcCopayMoney), sPayGroupValsInvalid.m_cyCopayMoney);
			sPayGroupVals.m_nCopayPercent = VarLong(pRow->GetValue(pgcCopayPercent), sPayGroupValsInvalid.m_nCopayPercent);
			sPayGroupVals.m_cyTotalDeductible = VarCurrency(pRow->GetValue(pgcTotalDeductible), sPayGroupValsInvalid.m_cyTotalDeductible);
			sPayGroupVals.m_cyTotalOOP = VarCurrency(pRow->GetValue(pgcTotalOOP), sPayGroupValsInvalid.m_cyTotalOOP);
			if ((nID != -1) && (sPayGroupVals.isCoInsPercentValid() || sPayGroupVals.isCopayPercentValid() || sPayGroupVals.isCopayMoneyValid() || sPayGroupVals.isTotalDeductibleValid() || sPayGroupVals.isTotalOOPValid())){
				m_Party.m_mapPayGroupVals.SetAt(nID, sPayGroupVals);
			}
			pRow = m_pPayGroupsList->FindAbsoluteNextRow(pRow, VARIANT_TRUE);
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
	
}

void CNewPatientAddInsuredDlg::OnBnClickedCancel()
{
	CNxDialog::OnCancel();
}

void CNewPatientAddInsuredDlg::OnSetFocusPhoneNumber() {

	try {
		if (ShowAreaCode()) {
			//first check to see if anything is in this box
			CString strPhone;
			GetDlgItemText(IDC_PHONE_BOX, strPhone);
			long nPhoneID = IDC_PHONE_BOX;
			if (! ContainsDigit(strPhone)) {			
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
					SetDlgItemText(nPhoneID, strAreaCode);
					CString str = strAreaCode;
					str.TrimRight();
					if (str != "") {
						if(m_bFormatPhoneNums) {
							FormatItem (nPhoneID, m_strPhoneFormat);						
						}
					}			

					//set the cursor
					::PostMessage(GetDlgItem(IDC_PHONE_BOX)->GetSafeHwnd(), EM_SETSEL, 5, 5);
				}						
			}
  		}		
	}NxCatchAll(__FUNCTION__);
}

void CNewPatientAddInsuredDlg::OnKillFocusZipCode() {

	try {
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
				// (s.tullis 2013-10-21 10:17) - PLID 45031 - If 9-digit zipcode match fails compair it with the 5-digit zipcode.
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
	}NxCatchAll( __FUNCTION__);
}

void CNewPatientAddInsuredDlg::OnKillFocusCityBox() {

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

void CNewPatientAddInsuredDlg::OnChangeSSN() 
{
	try {
		CString str;
		GetDlgItemText (IDC_SSN_BOX, str);
		FormatItemText (GetDlgItem(IDC_SSN_BOX), str, "###-##-####");
	}NxCatchAll( __FUNCTION__);
}

BOOL CNewPatientAddInsuredDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID;
	static CString str;							

	switch (HIWORD(wParam))
	{	
		case EN_CHANGE:			
			switch (nID = LOWORD(wParam))
			{
				case IDC_MIDDLE_NAME_BOX:					
					if (m_bAutoCapMiddle) {
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
					CapitalizeAll(nID);			
					break;
				case IDC_PHONE_BOX:				
					GetDlgItemText(nID, str);
					str.TrimRight();
					if (str != "") {
						if(m_bFormatPhoneNums) {
							FormatItem (nID, m_strPhoneFormat);
						}
					}
					break;
			}
			break;

		case EN_KILLFOCUS:			
			switch (nID = LOWORD(wParam))
			{
				// (j.jones 2011-06-24 16:49) - PLID 31005 - if they want the IDs capitalized, do so now
				case IDC_NEWPAT_INS_ID_BOX:
				case IDC_NEWPAT_INS_FECA_BOX:
					if(GetRemotePropertyInt("AutoCapitalizeInsuranceIDs", 0, 0, "<None>", true) == 1) {
						CapitalizeAll(nID);
					}
					break;
			}
			break;
	}
	return CNxDialog::OnCommand(wParam, lParam);
}


void CNewPatientAddInsuredDlg::SelChangingGenderList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll( __FUNCTION__);
}

// (c.haag 2010-10-04 10:16) - PLID 39447 - Populate the patient relationship list for insurance
void CNewPatientAddInsuredDlg::LoadRelToPatList() {

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Self");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		//set self to be the cursel
		m_pRelationToPatientList->CurSel = pRow;
		
		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Child");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Spouse");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Other");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Unknown");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Employee");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Organ Donor");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Cadaver Donor");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Life Partner");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		// (j.jones 2011-06-15 17:00) - PLID 40959 - the following are no longer valid entries in our system,
		// and no longer exist in data
		/*
		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Grandparent");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Grandchild");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Nephew Or Niece");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Adopted Child");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Foster Child");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Ward");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Stepchild");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Handicapped Dependent");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Sponsored Dependent");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Dependent of a Minor Dependent");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Significant Other");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Mother");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Father");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Other Adult");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Emancipated Minor");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Injured Plaintiff");// (s.dhole 2010-08-31 15:13) - PLID 40114 All our relationship lists say "Injured Plantiff" instead of "Injured Plaintiff"
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);

		pRow = m_pRelationToPatientList->GetNewRow();
		pRow->PutValue(0,"Child Where Insured Has No Financial Responsibility");
		m_pRelationToPatientList->AddRowAtEnd(pRow, NULL);
		*/

	}NxCatchAll(__FUNCTION__);

}

// (c.haag 2010-10-04 10:16) - PLID 39447 - Insurance
void CNewPatientAddInsuredDlg::SelChangingNewpatInsCoList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll( __FUNCTION__);
}

// (c.haag 2010-10-04 10:16) - PLID 39447 - Insurance
void CNewPatientAddInsuredDlg::SelChosenNewpatInsCoList(LPDISPATCH lpRow)
{
	try {
		//check to see if they chose a special row; requery pay group list otherwise
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			long nInsuranceCoID = VarLong(pRow->GetValue(ilcID));

			CString strFrom;
			strFrom.Format("ServicePayGroupsT "
				" LEFT JOIN InsuranceCoPayGroupsDefaultsT "
				" ON ServicePayGroupsT.ID = InsuranceCoPayGroupsDefaultsT.PayGroupID "
				" AND InsuranceCoPayGroupsDefaultsT.InsuranceCoID = %li ", nInsuranceCoID);
			m_pPayGroupsList->FromClause = _bstr_t(strFrom);
			m_pPayGroupsList->Requery();
			
			if (nInsuranceCoID == -2) {

				//add new

				//pop up the edit insurance list dialog
				CEditInsInfoDlg EditInsInfo(this);

				if (CheckCurrentUserPermissions(bioInsuranceCo,sptWrite))
				{
					long InsID = -1;
					long nContactID = -1;

					OLE_COLOR nColor = RGB(245, 227, 156);
					
					EditInsInfo.DisplayWindow(nColor, InsID, nContactID);
					
					//requery the list
					m_pInsCoList->Requery();
					
				}
			}	
			
			else {

				if (nInsuranceCoID == -1) { // selected <None>
					ClearAndDisablePayGroupControls();
				}
				else{ // any other insurance co selection
					bool bPerPayGroup = !!VarBool(pRow->GetValue(ilcPerPayGroup));
					m_radioIndividualPaygroups.SetCheck(bPerPayGroup);
					m_radioAllPaygroups.SetCheck(!bPerPayGroup);

					COleCurrency cyInvalidCurrency = COleCurrency();
					cyInvalidCurrency.SetStatus(COleCurrency::invalid);

					m_strTotalDeductible = "";
					COleCurrency cyTempCurrency = VarCurrency(pRow->GetValue(ilcTotalDeductible), cyInvalidCurrency);
					if (cyTempCurrency.GetStatus() == COleCurrency::valid){
						if (cyTempCurrency >= COleCurrency(0, 0)){
							m_strTotalDeductible = FormatCurrencyForInterface(cyTempCurrency);
						}
					}
					SetDlgItemText(IDC_TOTAL_DEDUCTIBLE_EDIT, m_strTotalDeductible);

					m_strTotalOOP = "";
					cyTempCurrency = VarCurrency(pRow->GetValue(ilcTotalOOP), cyInvalidCurrency);
					if (cyTempCurrency.GetStatus() == COleCurrency::valid){
						if (cyTempCurrency >= COleCurrency(0, 0)){
							m_strTotalOOP = FormatCurrencyForInterface(cyTempCurrency);
						}
					}
					SetDlgItemText(IDC_TOTAL_OOP_EDIT, m_strTotalOOP);

					TogglePayGroupControls();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (c.haag 2010-10-04 10:16) - PLID 39447 - Insurance
void CNewPatientAddInsuredDlg::RequeryFinishedNewpatInsCoList(short nFlags)
{
	try {

		//add None and Add New at the beginning

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pInsCoList->GetNewRow();
		
		
		if (pRow) {
			pRow->PutValue(ilcID, (long)-2);
			pRow->PutValue(ilcName, _variant_t("<Add New Company>"));
			pRow->PutValue(ilcAddress, _variant_t(""));

			m_pInsCoList->AddRowBefore(pRow, m_pInsCoList->GetFirstRow());
		}

		pRow = m_pInsCoList->GetNewRow();
		if (pRow) {
			pRow->PutValue(ilcID, (long)-1);
			pRow->PutValue(ilcName, _variant_t("<None>"));
			pRow->PutValue(ilcAddress, _variant_t(""));

			m_pInsCoList->AddRowBefore(pRow, m_pInsCoList->GetFirstRow());

			//set the cursel
			// (c.haag 2010-10-04 13:24) - PLID 39447 - Only set it to <None> if
			// nothing was already selected
			if (NULL == m_pInsCoList->CurSel) {
				m_pInsCoList->CurSel = pRow;
				ClearAndDisablePayGroupControls(); // (r.goldschmidt 2014-07-29 17:49) - PLID 62775
			}
		}

	}NxCatchAll( __FUNCTION__ );
}

// (c.haag 2010-10-04 10:16) - PLID 39447 - Insurance
void CNewPatientAddInsuredDlg::SelChangingNewpatInsRelation(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
		else if (lpOldSel != *lppNewSel)
		{
			NXDATALIST2Lib::IRowSettingsPtr pNewSel(*lppNewSel);
			if (VarString(pNewSel->GetValue(0),"") == "Self") {
				// (j.gruber 2012-08-01 11:49) - PLID 51908 - don't say its the new patient dlg if its not
				CString str;
				if (m_bFromNewPatientDlg) {
					str = "New Patient window.";
				}
				else {
					str = "current patient's information.";
				}
				if (IDNO == AfxMessageBox("This selection will disable the demographic boxes and fill them with data "
					"from the " + str + " Are you sure you wish to continue?", MB_YESNO | MB_ICONQUESTION))
				{
					SafeSetCOMPointer(lppNewSel, lpOldSel);
					EnableControls(TRUE);
				}
				else {
					LoadDemographics(m_Patient);
					EnableControls(FALSE);
				}
			}			
		}

	} NxCatchAll( __FUNCTION__);
}

// (c.haag 2010-10-04 10:16) - PLID 39447 - Fired when the relationship dropdown selection is finalized
void CNewPatientAddInsuredDlg::SelChosenNewpatInsRelation(LPDISPATCH lpRow)
{
	try {
		// If the user elected Self responsibility, then disable the demographic fields
		BOOL bEnable = TRUE;
		if (NULL != m_pRelationToPatientList->CurSel &&
			VarString(m_pRelationToPatientList->CurSel->GetValue(0),"") == "Self")
		{
			bEnable = FALSE;
		}
		EnableControls(bEnable);
	} NxCatchAll( __FUNCTION__);
}

// (c.haag 2010-10-04 10:16) - PLID 39447 - Copy demographic insurance information 
// from the new patient's demographics
void CNewPatientAddInsuredDlg::OnCopyPatientInfo()
{
	try {
		if (IDYES == MessageBox("This will overwrite the existing insured party information. \n"
			"Are you sure you wish to continue?","Practice", MB_YESNO | MB_ICONQUESTION)) 
		{
			CopyPatientInfo();
		}
	} 
	NxCatchAll( __FUNCTION__);
}

// (j.jones 2012-11-12 10:49) - PLID 53622 - added country dropdown
void CNewPatientAddInsuredDlg::OnSelChangingCountryList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}

	}NxCatchAll( __FUNCTION__);
}

// (r.goldschmidt 2014-07-23 16:22) - PLID 62775 - disallow editing of certain fields in pay group list
void CNewPatientAddInsuredDlg::EditingStartingNewpatPayGroupList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			_variant_t varCopay;
			_variant_t varPercent;

			varCopay = pRow->GetValue(pgcCopayMoney);
			varPercent = pRow->GetValue(pgcCopayPercent);

			CString strPayGroupName = VarString(pRow->GetValue(pgcName), _T(""));

			switch (nCol) {

			// (r.goldschmidt 2014-08-04 18:05) - PLID 63064 - When setting up pay group values for an insured party, a copay percent and a copay money cannot be set, but it can be set as a default for an insurance party. Allow both fields to be set.
			//    (commented out the code in case there is a need to restore functionality)
			// (b.spivey, March 26, 2015) - PLID 56838 -Added this back in.
			case pgcCopayMoney:
				//we can't have money if the percent is already filled in
				if (varPercent.vt != VT_NULL && varPercent.vt != VT_EMPTY && (varCopay.vt == VT_NULL || varCopay.vt == VT_EMPTY)) {
					MessageBox("You cannot enter both a copay currency amount and a copay percent amount.");
					*pbContinue = FALSE;
				}
				break;

			case pgcCopayPercent:
				//we can't have percent if the money is already filled in
				if (varCopay.vt != VT_NULL && varCopay.vt != VT_EMPTY && (varPercent.vt == VT_NULL || varPercent.vt == VT_EMPTY)) {
					MessageBox("You cannot enter both a copay currency amount and a copay percent amount.");
					*pbContinue = FALSE;
				}
				break;
			
			case pgcTotalDeductible:
				if (strPayGroupName.CompareNoCase(_T("Copay")) == 0){
					MessageBox("\"Total Deductible\" cannot be used on the Copay pay group.");
					*pbContinue = FALSE;
				}
				break;
			case pgcTotalOOP:
				if (strPayGroupName.CompareNoCase(_T("Copay")) == 0){
					MessageBox("\"Total Out of Pocket\" cannot be used on the Copay pay group.");
					*pbContinue = FALSE;
				}
				break;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-07-23 16:22) - PLID 62775 - Add Pay Group List - validate and/or correct the input
void CNewPatientAddInsuredDlg::EditingFinishingNewpatPayGroupList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (*pbContinue && *pbCommit) {

			CString strEntered = strUserEntered;

			_variant_t varCopay;
			_variant_t varPercent;

			varCopay = pRow->GetValue(pgcCopayMoney);
			varPercent = pRow->GetValue(pgcCopayPercent);

			switch (nCol) {
			// (b.spivey, March 26, 2015) - PLID 56838 - Don't allow them to save a value in here if there's one in the other column
			case pgcCopayPercent:
				//we can't have percent if the money is already filled in
				if (varCopay.vt != VT_NULL && varCopay.vt != VT_EMPTY && (!strEntered.IsEmpty())) {
					MessageBox("You cannot enter both a copay currency amount and a copay percent amount.");
					*pvarNewValue = g_cvarNull;
					return; 
				}
			case pgcCoInsPercent:
			{
				if (strEntered.IsEmpty()) {
					//put a null value in
					*pvarNewValue = g_cvarNull;
					return;
				}

				//check to make sure its a number
				if (!IsNumeric(strEntered)) {
					*pvarNewValue = g_cvarNull;
					MsgBox("Please enter a valid percentage value.");
					*pbContinue = FALSE;
					*pbCommit = FALSE;
					return;
				}

				long nPercent = atoi(strEntered);
				if (nPercent < 0 || nPercent > 100) {
					*pvarNewValue = g_cvarNull;
					MsgBox("Please enter a valid percentage value.");
					*pbContinue = FALSE;
					*pbCommit = FALSE;
					return;
				}
			}

				break;

			// (b.spivey, March 26, 2015) - PLID 56838 - Don't allow them to save a value in here if there's one in the other column
			case pgcCopayMoney:
				//we can't have money if the percent is already filled in
				if (varPercent.vt != VT_NULL && varPercent.vt != VT_EMPTY && (!strEntered.IsEmpty())) {
					MessageBox("You cannot enter both a copay currency amount and a copay percent amount.");
					*pvarNewValue = g_cvarNull;
					return; 
				}

			case pgcTotalDeductible:
			case pgcTotalOOP:
			{
				if (strEntered.IsEmpty()) {
					//put a null value in
					*pvarNewValue = g_cvarNull;
					return;
				}

				//check to make sure its a valid currency
				COleCurrency cyAmt = ParseCurrencyFromInterface(strEntered);
				if (cyAmt.GetStatus() != COleCurrency::valid) {
					*pvarNewValue = g_cvarNull;
					MsgBox("Please enter a valid currency.");
					*pbContinue = FALSE;
					*pbCommit = FALSE;
					return;
				}

				if (cyAmt < COleCurrency(0, 0)) {
					*pvarNewValue = g_cvarNull;
					MsgBox("Please enter a currency greater than 0.");
					*pbContinue = FALSE;
					*pbCommit = FALSE;
					return;
				}
			}

				break;

			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-07-28 18:07) - PLID 62775 - when toggling per pay group setting
void CNewPatientAddInsuredDlg::OnBnClickedAllPayGroupsRadio()
{
	try{
		TogglePayGroupControls();
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-07-28 18:07) - PLID 62775 - when toggling per pay group setting
void CNewPatientAddInsuredDlg::OnBnClickedIndividualPayGroupsRadio()
{
	try{
		TogglePayGroupControls();
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-07-28 18:07) - PLID 62775 - validate entered currency
void CNewPatientAddInsuredDlg::OnEnKillfocusTotalDeductibleEdit()
{
	try{
		bool bValid = true;
		COleCurrency cyCurrentValue;
		CString strCurrentValue;
		GetDlgItemText(IDC_TOTAL_DEDUCTIBLE_EDIT, strCurrentValue);

		// if empty, don't do anything
		if (strCurrentValue.IsEmpty()){
			m_strTotalDeductible = "";
			return;
		}

		cyCurrentValue = ParseCurrencyFromInterface(strCurrentValue);

		// if currency invalid
		if (cyCurrentValue.GetStatus() != COleCurrency::valid){
			MsgBox("An invalid currency has been entered.\nThe Total Deductible has been reset.");
			bValid = false;
		}

		// if currency is less than zero
		else if (cyCurrentValue < COleCurrency(0, 0)){
			MsgBox("A currency less than zero has been entered.\nThe Total Deductible has been reset.");
			bValid = false;
		}

		if (bValid){
			m_strTotalDeductible = FormatCurrencyForInterface(cyCurrentValue);
		}

		SetDlgItemText(IDC_TOTAL_DEDUCTIBLE_EDIT, m_strTotalDeductible);
	
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-07-28 18:07) - PLID 62775 - validate entered currency
void CNewPatientAddInsuredDlg::OnEnKillfocusTotalOopEdit()
{
	try{
		bool bValid = true;
		COleCurrency cyCurrentValue;
		CString strCurrentValue;
		GetDlgItemText(IDC_TOTAL_OOP_EDIT, strCurrentValue);

		// if empty, don't do anything
		if (strCurrentValue.IsEmpty()){
			m_strTotalOOP = "";
			return;
		}

		cyCurrentValue = ParseCurrencyFromInterface(strCurrentValue);

		// if currency invalid
		if (cyCurrentValue.GetStatus() != COleCurrency::valid){
			MsgBox("An invalid currency has been entered.\nThe Total Out of Pocket has been reset.");
			bValid = false;
		}

		// if currency is less than zero
		else if (cyCurrentValue < COleCurrency(0, 0)){
			MsgBox("A currency less than zero has been entered.\nThe Total Out of Pocket has been reset.");
			bValid = false;
		}

		if (bValid){
			m_strTotalOOP = FormatCurrencyForInterface(cyCurrentValue);
		}

		SetDlgItemText(IDC_TOTAL_OOP_EDIT, m_strTotalOOP);

	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2014-07-29 15:32) - PLID 62775 - make sure Deductible and OOP fields for CoPay are grayed out
void CNewPatientAddInsuredDlg::RequeryFinishedNewpatPayGroupList(short nFlags)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRowCopay = m_pPayGroupsList->SearchByColumn(pgcName, _T("Copay"), m_pPayGroupsList->GetFirstRow(), FALSE);
		OLE_COLOR colorGrayed = RGB(205, 201, 201);
		pRowCopay->PutCellBackColor(pgcTotalDeductible, colorGrayed);
		pRowCopay->PutCellBackColor(pgcTotalOOP, colorGrayed);

	}NxCatchAll(__FUNCTION__);
}

// (s.tullis 2016-02-11 11:43) - PLID 68212 - if they edited the box but did not put anything in reset the box
void CNewPatientAddInsuredDlg::OnEnKillfocusSsnBox()
{
	try {
		CString strSsn;
		GetDlgItemText(IDC_SSN_BOX, strSsn);
		if (strSsn == "###-##-####") {
			strSsn = "";
			FormatItemText(GetDlgItem(IDC_SSN_BOX), strSsn, "");
		}
	}NxCatchAll(__FUNCTION__)
}

// EligibilityCreateForScheduledPatientsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EligibilityCreateForScheduledPatientsDlg.h"
#include "GlobalFinancialUtils.h"
#include "GlobalUtils.h"
#include "EEligibility.h"

// CEligibilityCreateForScheduledPatientsDlg dialog

// (j.jones 2009-09-15 17:37) - PLID 26481 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum LocationComboColumn {

	lccID = 0,
	lccName,
};

enum ApptTypeListColumn {

	atlcID = 0,
	atlcName,
};

enum ResourceListColumn {

	rlcID = 0,
	rcName,
};

enum InsCoListColumn {

	iclcID = 0,
	iclcName,
	iclcAddress,
};

enum BenefitCategoryComboColumn {

	bcccID = 0,	// (j.jones 2013-03-28 15:18) - PLID 52182 - categories now have IDs
	bcccCode,
	bcccCategory,
};

CEligibilityCreateForScheduledPatientsDlg::CEligibilityCreateForScheduledPatientsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEligibilityCreateForScheduledPatientsDlg::IDD, pParent)
{
	m_nFormatID = -1;
}

void CEligibilityCreateForScheduledPatientsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_CREATE_REQUESTS, m_btnCreateRequests);
	DDX_Control(pDX, IDC_BTN_CLOSE_ELIG, m_btnClose);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_APPT_TYPE, m_btnSelectOneApptType);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL_APPT_TYPES, m_btnSelectAllApptTypes);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_APPT_TYPE, m_btnUnselectOneApptType);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_APPT_TYPES, m_btnUnselectAllApptTypes);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_RESOURCE, m_btnSelectOneResource);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL_RESOURCES, m_btnSelectAllResources);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_RESOURCE, m_btnUnselectOneResource);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_RESOURCES, m_btnUnselectAllResources);
	DDX_Control(pDX, IDC_BTN_ELIG_SELECT_ONE_INSCO, m_btnSelectOneInsco);
	DDX_Control(pDX, IDC_BTN_ELIG_SELECT_ALL_INSCOS, m_btnSelectAllInscos);
	DDX_Control(pDX, IDC_BTN_ELIG_UNSELECT_ONE_INSCO, m_btnUnselectOneInsco);
	DDX_Control(pDX, IDC_BTN_ELIG_UNSELECT_ALL_INSCOS, m_btnUnselectAllInscos);
	DDX_Control(pDX, IDC_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_TO, m_dtTo);
	DDX_Control(pDX, IDC_RADIO_USE_APPT_RESOURCE_PROVIDER, m_radioUseApptResourceProvider);
	DDX_Control(pDX, IDC_RADIO_USE_G1_PROVIDER, m_radioUseG1Provider);
	DDX_Control(pDX, IDC_RADIO_USE_APPT_LOCATION, m_radioUseApptLocation);
	DDX_Control(pDX, IDC_RADIO_USE_G2_LOCATION, m_radioUseG2Location);
	DDX_Control(pDX, IDC_RADIO_USE_CURRENT_LOCATION, m_radioUseCurLocation);
	DDX_Control(pDX, IDC_RADIO_ALL_INSURED_PARTIES, m_radioAllInsuredParties);
	DDX_Control(pDX, IDC_RADIO_PRIMARY_ONLY, m_radioPrimaryInsuredParties);
	DDX_Control(pDX, IDC_CHECK_USE_DEFAULT_DIAG_CODES, m_checkUseG2DiagCodes);
}


BEGIN_MESSAGE_MAP(CEligibilityCreateForScheduledPatientsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_CLOSE_ELIG, OnBtnClose)
	ON_BN_CLICKED(IDC_BTN_CREATE_REQUESTS, OnBtnCreateRequests)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_APPT_TYPE, OnBtnSelectOneApptType)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL_APPT_TYPES, OnBtnSelectAllApptTypes)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_APPT_TYPE, OnBtnUnselectOneApptType)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_APPT_TYPES, OnBtnUnselectAllApptTypes)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_RESOURCE, OnBtnSelectOneResource)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL_RESOURCES, OnBtnSelectAllResources)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_RESOURCE, OnBtnUnselectOneResource)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_RESOURCES, OnBtnUnselectAllResources)
	ON_BN_CLICKED(IDC_BTN_ELIG_SELECT_ONE_INSCO, OnBtnSelectOneInsco)
	ON_BN_CLICKED(IDC_BTN_ELIG_SELECT_ALL_INSCOS, OnBtnSelectAllInscos)
	ON_BN_CLICKED(IDC_BTN_ELIG_UNSELECT_ONE_INSCO, OnBtnUnselectOneInsco)
	ON_BN_CLICKED(IDC_BTN_ELIG_UNSELECT_ALL_INSCOS, OnBtnUnselectAllInscos)
END_MESSAGE_MAP()


// CEligibilityCreateForScheduledPatientsDlg message handlers

BOOL CEligibilityCreateForScheduledPatientsDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		g_propManager.CachePropertiesInBulk("CEligibilityCreateForScheduledPatientsDlg_1", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'EligibilityCreateForScheduled_DefLocationID' OR "
			"Name = 'EligibilityCreateForScheduled_DefProviderType' OR "
			"Name = 'EligibilityCreateForScheduled_DefLocationType' OR "
			"Name = 'EligibilityCreateForScheduled_DefInsuranceType' OR "
			"Name = 'EligibilityCreateForScheduled_SendDiagCodes' OR "
			"Name = 'GEDIEligibilityRealTime_Enabled' " // (j.jones 2010-07-07 17:49) - PLID 39515
			")"
			, _Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("CEligibilityCreateForScheduledPatientsDlg_2", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			// (j.jones 2013-03-28 15:50) - PLID 52182 - removed EligibilityCreateForScheduled_DefCategory,
			// replaced with DefaultEligibilityBenefitCategory, which applies every time
			"Name = 'DefaultEligibilityBenefitCategory' OR "
			"Name = 'EligibilityCreateForScheduled_DefApptTypeIDs' OR "
			"Name = 'EligibilityCreateForScheduled_DefResourceIDs' "
			")"
			, _Q(GetCurrentUserName()));

		// (a.walling 2010-01-26 17:06) - PLID 37017 - Use memo for the insurance co IDs since there can easily be a lot of them. 
		// Text can hold a little more than 30 ids in most cases, which is fine for these other properties.
		g_propManager.CachePropertiesInBulk("CEligibilityCreateForScheduledPatientsDlg_3", propMemo,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'EligibilityCreateForScheduled_DefInsCoIDs' "
			")"
			, _Q(GetCurrentUserName()));
		
		m_btnCreateRequests.AutoSet(NXB_NEW);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnSelectOneApptType.AutoSet(NXB_DOWN);
		m_btnSelectAllApptTypes.AutoSet(NXB_DDOWN);
		m_btnUnselectOneApptType.AutoSet(NXB_UP);
		m_btnUnselectAllApptTypes.AutoSet(NXB_UUP);
		m_btnSelectOneResource.AutoSet(NXB_DOWN);
		m_btnSelectAllResources.AutoSet(NXB_DDOWN);
		m_btnUnselectOneResource.AutoSet(NXB_UP);
		m_btnUnselectAllResources.AutoSet(NXB_UUP);
		m_btnSelectOneInsco.AutoSet(NXB_DOWN);
		m_btnSelectAllInscos.AutoSet(NXB_DDOWN);
		m_btnUnselectOneInsco.AutoSet(NXB_UP);
		m_btnUnselectAllInscos.AutoSet(NXB_UUP);

		m_LocationCombo = BindNxDataList2Ctrl(IDC_ELIG_APPT_LOCATION_FILTER);
		m_UnselectedApptTypeList = BindNxDataList2Ctrl(IDC_ELIG_APPT_TYPE_UNSELECTED_LIST);
		m_SelectedApptTypeList = BindNxDataList2Ctrl(IDC_ELIG_APPT_TYPE_SELECTED_LIST, false);
		m_UnselectedResourceList = BindNxDataList2Ctrl(IDC_ELIG_APPT_RESOURCE_UNSELECTED_LIST);
		m_SelectedResourceList = BindNxDataList2Ctrl(IDC_ELIG_APPT_RESOURCE_SELECTED_LIST, false);
		m_UnselectedInsCoList = BindNxDataList2Ctrl(IDC_ELIG_INSCOS_UNSELECTED_LIST);
		m_SelectedInsCoList = BindNxDataList2Ctrl(IDC_ELIG_INSCOS_SELECTED_LIST, false);
		m_BenefitCategoryCombo = BindNxDataList2Ctrl(IDC_ELIG_SCHED_CATEGORY_COMBO, true);

		//add an "all locations" row
		IRowSettingsPtr pLocRow = m_LocationCombo->GetNewRow();
		pLocRow->PutValue(lccID, (long)-1);
		pLocRow->PutValue(lccName, " {All Locations}");
		m_LocationCombo->AddRowSorted(pLocRow, NULL);

		//the category combo is built from scratch
		// (j.jones 2013-03-28 15:25) - PLID 52182 - this is now in data, and thus obsolete
		//BuildEligibilityBenefitCategoryCombo(m_BenefitCategoryCombo, (long)bcccCode, (long)bcccCategory);
		
		//default the dates to tomorrow's date
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dtTomorrow(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0,0,0);
		dtTomorrow += COleDateTimeSpan(1, 0, 0,0);
		m_dtFrom.SetValue(COleVariant(dtTomorrow));
		m_dtTo.SetValue(COleVariant(dtTomorrow));

		//set default values

		//location should default to -1 for all locations,
		//but also saves the last selection per user
		long nLocationID = GetRemotePropertyInt("EligibilityCreateForScheduled_DefLocationID", -1, 0, GetCurrentUserName(), TRUE);
		if(NULL == m_LocationCombo->SetSelByColumn(lccID, (long)nLocationID)) {
			m_LocationCombo->SetSelByColumn(lccID, (long)-1);
		}

		//appt. type IDs are stored comma-delimited
		CString strApptTypeIDs = GetRemotePropertyText("EligibilityCreateForScheduled_DefApptTypeIDs", "", 0, GetCurrentUserName(), TRUE);
		if(!strApptTypeIDs.IsEmpty()) {
			//try to select each ID, don't worry if the ID no longer exists

			CDWordArray dwIDs;
			GetIDsFromCommaDelimitedString(&dwIDs, strApptTypeIDs);

			// (a.walling 2011-02-23 14:51) - PLID 42552 - Temporary workaround for datalist concurrency issues:
			// wait for requery to finish before messing with things
			// (this pretty much happens now anyway due to the wait in FindByColumn)
			m_UnselectedApptTypeList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			for(int i=0; i<dwIDs.GetSize(); i++) {
				IRowSettingsPtr pRow = m_UnselectedApptTypeList->FindByColumn(atlcID, (long)dwIDs.GetAt(i), m_UnselectedApptTypeList->GetFirstRow(), FALSE);
				if(pRow) {
					m_SelectedApptTypeList->TakeRowAddSorted(pRow);
				}
			}
		}

		//resource IDs are stored comma-delimited
		CString strResourceIDs = GetRemotePropertyText("EligibilityCreateForScheduled_DefResourceIDs", "", 0, GetCurrentUserName(), TRUE);
		if(!strResourceIDs.IsEmpty()) {
			//try to select each ID, don't worry if the ID no longer exists

			CDWordArray dwIDs;
			GetIDsFromCommaDelimitedString(&dwIDs, strResourceIDs);

			// (a.walling 2011-02-23 14:51) - PLID 42552 - Temporary workaround for datalist concurrency issues:
			// wait for requery to finish before messing with things
			// (this pretty much happens now anyway due to the wait in FindByColumn)
			m_UnselectedResourceList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			for(int i=0; i<dwIDs.GetSize(); i++) {
				IRowSettingsPtr pRow = m_UnselectedResourceList->FindByColumn(rlcID, (long)dwIDs.GetAt(i), m_UnselectedResourceList->GetFirstRow(), FALSE);
				if(pRow) {
					m_SelectedResourceList->TakeRowAddSorted(pRow);
				}
			}
		}

		//insurance company IDs are stored comma-delimited
		// (a.walling 2010-01-26 17:06) - PLID 37017 - Use memo for the insurance co IDs since there can easily be a lot of them. 
		// Text can hold a little more than 30 ids in most cases, which is fine for these other properties.
		CString strInsCoIDs = GetRemotePropertyMemo("EligibilityCreateForScheduled_DefInsCoIDs", "", 0, GetCurrentUserName(), TRUE);
		if(!strInsCoIDs.IsEmpty()) {
			//try to select each ID, don't worry if the ID no longer exists

			CDWordArray dwIDs;
			GetIDsFromCommaDelimitedString(&dwIDs, strInsCoIDs);

			// (a.walling 2011-02-23 14:51) - PLID 42552 - Temporary workaround for datalist concurrency issues:
			// wait for requery to finish before messing with things
			// (this pretty much happens now anyway due to the wait in FindByColumn)
			m_UnselectedInsCoList->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			for(int i=0; i<dwIDs.GetSize(); i++) {
				IRowSettingsPtr pRow = m_UnselectedInsCoList->FindByColumn(iclcID, (long)dwIDs.GetAt(i), m_UnselectedInsCoList->GetFirstRow(), FALSE);
				if(pRow) {
					m_SelectedInsCoList->TakeRowAddSorted(pRow);
				}
			}
		}

		//the benefit category should default to 30,
		//but can have the default changed in preferences
		//auto-select "30" otherwise
		// (j.jones 2013-03-28 15:50) - PLID 52182 - removed EligibilityCreateForScheduled_DefCategory,
		// replaced with DefaultEligibilityBenefitCategory, which applies every time
		CString strCategory = GetRemotePropertyText("DefaultEligibilityBenefitCategory", "30", 0, "<None>", TRUE);
		m_BenefitCategoryCombo->TrySetSelByColumn_Deprecated(bcccCode, _bstr_t(strCategory));

		//the provider should default to the resource provider
		long nProviderType = GetRemotePropertyInt("EligibilityCreateForScheduled_DefProviderType", 1, 0, GetCurrentUserName(), TRUE);
		//1 - appt. resource, 2 - G1 Provider
		if(nProviderType == 2) {
			m_radioUseG1Provider.SetCheck(TRUE);
		}
		else {
			m_radioUseApptResourceProvider.SetCheck(TRUE);
		}

		//the location should default to the current location
		long nLocationType = GetRemotePropertyInt("EligibilityCreateForScheduled_DefLocationType", 3, 0, GetCurrentUserName(), TRUE);
		//1 - appt. location, 2 - G2 location, 3 - current location
		if(nLocationType == 1) {
			m_radioUseApptLocation.SetCheck(TRUE);
		}
		else if(nLocationType == 2) {
			m_radioUseG2Location.SetCheck(TRUE);
		}
		else {
			m_radioUseCurLocation.SetCheck(TRUE);
		}

		//the insurance type should default to primary only
		long nInsuranceType = GetRemotePropertyInt("EligibilityCreateForScheduled_DefInsuranceType", 2, 0, GetCurrentUserName(), TRUE);
		//1 - all, 2 - primary
		if(nInsuranceType == 1) {
			m_radioAllInsuredParties.SetCheck(TRUE);
		}
		else {
			m_radioPrimaryInsuredParties.SetCheck(TRUE);
		}

		//diag. codes should default to off
		long nSendDiagCodes = GetRemotePropertyInt("EligibilityCreateForScheduled_SendDiagCodes", 0, 0, GetCurrentUserName(), TRUE);
		m_checkUseG2DiagCodes.SetCheck(nSendDiagCodes == 1);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnInitDialog");
	
	return TRUE;
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnClose()
{
	try {

		CNxDialog::OnCancel();

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnClose");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnCreateRequests()
{
	try {

		// (j.jones 2010-07-08 08:49) - PLID 39515 - cache the settings
		// (j.jones 2010-10-21 14:21) - PLID 40914 - renamed to reflect that this is not clearinghouse-specific
		BOOL bUseRealTimeElig = (GetRemotePropertyInt("GEDIEligibilityRealTime_Enabled", 0, 0, "<None>", true) == 1);

		//get our settings

		long nLocationID = -1;
		{
			IRowSettingsPtr pRow = m_LocationCombo->GetCurSel();
			if(pRow) {
				nLocationID = VarLong(pRow->GetValue(lccID));
			}
		}

		CString strApptTypeIDs;
		{
			IRowSettingsPtr pRow = m_SelectedApptTypeList->GetFirstRow();
			while(pRow) {

				long nApptTypeID = VarLong(pRow->GetValue(atlcID));
				if(!strApptTypeIDs.IsEmpty()) {
					strApptTypeIDs += ",";
				}
				strApptTypeIDs += AsString(nApptTypeID);

				pRow = pRow->GetNextRow();
			}
		}

		CString strResourceIDs;
		{
			IRowSettingsPtr pRow = m_SelectedResourceList->GetFirstRow();
			while(pRow) {

				long nResource = VarLong(pRow->GetValue(rlcID));
				if(!strResourceIDs.IsEmpty()) {
					strResourceIDs += ",";
				}
				strResourceIDs += AsString(nResource);

				pRow = pRow->GetNextRow();
			}
		}

		CString strInsCoIDs;
		{
			IRowSettingsPtr pRow = m_SelectedInsCoList->GetFirstRow();
			while(pRow) {

				long nInsCoID = VarLong(pRow->GetValue(iclcID));
				if(!strInsCoIDs.IsEmpty()) {
					strInsCoIDs += ",";
				}
				strInsCoIDs += AsString(nInsCoID);

				pRow = pRow->GetNextRow();
			}
		}

		// (j.jones 2013-03-28 15:07) - PLID 52182 - changed the category to be an ID, not a string,
		// but this is used in a non-param batch so it needs to be NULL if no category is used
		long nCategoryID = -1;
		{
			IRowSettingsPtr pRow = m_BenefitCategoryCombo->GetCurSel();
			if(pRow) {
				nCategoryID = VarLong(pRow->GetValue(bcccID));
			}
			else {
				//the batch eligibility creation doesn't allow requests by service code,
				//so a benefit category is required
				AfxMessageBox("No Benefit Category is selected for these requests. Please correct this before creating requests.");
				return;
			}
		}

		long nProviderType = 1;
		//1 - appt. resource, 2 - G1 Provider
		if(m_radioUseG1Provider.GetCheck()) {
			nProviderType = 2;
		}

		long nLocationType = 3;
		//1 - appt. location, 2 - G2 location, 3 - current location
		if(m_radioUseApptLocation.GetCheck()) {
			nLocationType = 1;
		}
		else if(m_radioUseG2Location.GetCheck()) {
			nLocationType = 2;
		}

		long nInsuranceType = 2;
		//1 - all, 2 - primary
		if(m_radioAllInsuredParties.GetCheck()) {
			nInsuranceType = 1;
		}

		long nSendDiagCodes = m_checkUseG2DiagCodes.GetCheck() ? 1 : 0;

		//save our settings for this user
		SetRemotePropertyInt("EligibilityCreateForScheduled_DefLocationID", nLocationID, 0, GetCurrentUserName());
		SetRemotePropertyText("EligibilityCreateForScheduled_DefApptTypeIDs", strApptTypeIDs, 0, GetCurrentUserName());
		SetRemotePropertyText("EligibilityCreateForScheduled_DefResourceIDs", strResourceIDs, 0, GetCurrentUserName());
		// (a.walling 2010-01-26 17:06) - PLID 37017 - Use memo for the insurance co IDs since there can easily be a lot of them. 
		// Text can hold a little more than 30 ids in most cases, which is fine for these other properties.
		SetRemotePropertyMemo("EligibilityCreateForScheduled_DefInsCoIDs", strInsCoIDs, 0, GetCurrentUserName());
		// (j.jones 2013-03-28 15:50) - PLID 52182 - removed EligibilityCreateForScheduled_DefCategory,
		// we now configure this as DefaultEligibilityBenefitCategory in preferences, as a global setting
		//SetRemotePropertyText("EligibilityCreateForScheduled_DefCategory", strCategory, 0, GetCurrentUserName());
		SetRemotePropertyInt("EligibilityCreateForScheduled_DefProviderType", nProviderType, 0, GetCurrentUserName());
		SetRemotePropertyInt("EligibilityCreateForScheduled_DefLocationType", nLocationType, 0, GetCurrentUserName());
		SetRemotePropertyInt("EligibilityCreateForScheduled_DefInsuranceType", nInsuranceType, 0, GetCurrentUserName());
		SetRemotePropertyInt("EligibilityCreateForScheduled_SendDiagCodes", nSendDiagCodes, 0, GetCurrentUserName());

		COleDateTime dtFrom = m_dtFrom.GetValue();
		COleDateTime dtTo = m_dtTo.GetValue();
		
		//validate the dates
		if(dtFrom.GetStatus() == COleDateTime::invalid) {
			AfxMessageBox("The 'From' date is an invalid date. Please correct this before creating requests.");
			return;
		}		
		if(dtTo.GetStatus() == COleDateTime::invalid) {
			AfxMessageBox("The 'To' date is an invalid date. Please correct this before creating requests.");
			return;
		}
		if(dtFrom > dtTo) {
			AfxMessageBox("The 'To' date earlier than your 'From' date. Please correct this before creating requests.");
			return;
		}

		//ok, now create the requests!

		CString strWhere;

		if(!strApptTypeIDs.IsEmpty()) {
			CString str;
			str.Format("AND AppointmentsT.AptTypeID IN (%s)", strApptTypeIDs);
			strWhere += str;
		}

		if(!strResourceIDs.IsEmpty()) {
			CString str;
			str.Format("AND ResourceT.ID IN (%s)", strResourceIDs);
			strWhere += str;
		}

		if(!strInsCoIDs.IsEmpty()) {
			CString str;
			str.Format("AND InsuranceCoT.PersonID IN (%s)", strInsCoIDs);
			strWhere += str;
		}

		if(nLocationID != -1) {
			CString str;
			str.Format("AND AppointmentsT.LocationID = %li", nLocationID);
			strWhere += str;
		}

		//2 - primary insurances only
		if(nInsuranceType == 2) {
			strWhere += "AND InsuredPartyT.RespTypeID = 1";
		}

		//This lookup query will find all matching appointments, and insured parties
		//that we need to create requests for. It will return a record for each
		//appointment, possibly more if there are multiple resources. The code
		//that reads the results is responsible for only making one request
		//per insured party. This query will ignore insured parties that
		//already have a request currently in the batch.

		//can't be parameterized
		// (c.haag 2010-10-15 9:57) - PLID 40352 - Added ApptPOSID for ANSI 5010.
		// (j.jones 2011-10-07 15:55) - PLID 37659 - added AppointmentID
		// (j.jones 2014-03-10 08:42) - PLID 61243 - added ICD-10 codes and the insurance company ICD-10 date
		_RecordsetPtr rs = CreateParamRecordset(FormatString("SELECT AppointmentsT.ID AS AppointmentID, AppointmentsT.PatientID, "
			"InsuredPartyT.PersonID AS InsuredPartyID, PatientsT.MainPhysician, "
			"PatientsT.DefaultDiagID1, PatientsT.DefaultDiagID2, PatientsT.DefaultDiagID3, PatientsT.DefaultDiagID4, "
			"PatientsT.DefaultICD10DiagID1, PatientsT.DefaultICD10DiagID2, PatientsT.DefaultICD10DiagID3, PatientsT.DefaultICD10DiagID4, "
			"Convert(bit, CASE WHEN InsuranceCoT.ICD10GoLiveDate Is Not Null AND InsuranceCoT.ICD10GoLiveDate <= AppointmentsT.Date THEN 1 ELSE 0 END) AS UseICD10Codes, "
			"PatLocationsQ.ID AS PatientLocationID, AptLocationsQ.ID AS ApptLocationID, "
			"ResourceProviderLinkT.ProviderID AS ApptProviderID, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
			"AptLocationsQ.POSID AS ApptPOSID "
			"FROM PatientsT "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"INNER JOIN InsuredPartyT ON PatientsT.PersonID = InsuredPartyT.PatientID "
			"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"INNER JOIN AppointmentsT ON PatientsT.PersonID = AppointmentsT.PatientID "
			"LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
			"LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
			"LEFT JOIN ResourceProviderLinkT ON ResourceT.ID = ResourceProviderLinkT.ResourceID "
			"LEFT JOIN (SELECT ID, POSID FROM LocationsT WHERE Active = 1 AND TypeID = 1 AND Managed = 1) AptLocationsQ "
			"	ON AppointmentsT.LocationID = AptLocationsQ.ID "
			"LEFT JOIN (SELECT ID FROM LocationsT WHERE Active = 1 AND TypeID = 1 AND Managed = 1) PatLocationsQ "
			"	ON PersonT.Location = PatLocationsQ.ID "
			"WHERE AppointmentsT.Status <> 4 "
			"AND InsuredPartyT.RespTypeID <> -1 "
			"AND AppointmentsT.PatientID > 0 "
			"AND AppointmentsT.Date >= {STRING} AND AppointmentsT.Date <= {STRING} "
			"AND InsuredPartyT.PersonID NOT IN (SELECT InsuredPartyID FROM EligibilityRequestsT WHERE Batched = 1) "
			"%s "
			"ORDER BY AppointmentsT.Date ASC, AppointmentsT.StartTime ASC, "
			"CASE WHEN ResourceProviderLinkT.ProviderID Is Null THEN 0 ELSE 1 END DESC", strWhere),
			FormatDateTimeForSql(dtFrom), FormatDateTimeForSql(dtTo));

		if(rs->eof) {
			AfxMessageBox("There are no insured patients that match your filters and have appointments in the range given "
				"and do not already have an E-Eligiblity request batched.");
			return;
		}

		CMap<long, long, long, long> mapInsuredPartyIDs;
		CMap<long, long, long, long> mapSkippedPatientIDs;

		CString strSqlBatch;
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewID INT");
		// (j.jones 2010-07-07 17:58) - PLID 39515 - we need to know the new IDs we created
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @NewIDsT TABLE (RequestID INT NOT NULL PRIMARY KEY)");		

		long nCountCreated = 0;
		CString strPatientsWithNoProvider;
		
		while(!rs->eof) {

			// (j.jones 2011-10-07 15:55) - PLID 37659 - added AppointmentID
			long nAppointmentID = AdoFldLong(rs, "AppointmentID");

			//first make sure we haven't already created a
			//request for this insured party
			long nInsuredPartyID = AdoFldLong(rs, "InsuredPartyID");
			long nPatientID = AdoFldLong(rs, "PatientID");

			long nFound = 0;
			if(mapInsuredPartyIDs.Lookup(nInsuredPartyID, nFound)) {
				//this insured party already exists in our map,
				//so skip this record
				rs->MoveNext();
				continue;
			}

			//get the provider ID per our settings
			//1 - appt. resource, 2 - G1 Provider
			long nProviderIDToUse = -1;
			if(nProviderType == 1) {
				nProviderIDToUse = AdoFldLong(rs, "ApptProviderID", -1);
			}
			if(nProviderIDToUse == -1) {
				//if nProviderType = 2 or the appointment had no provider
				nProviderIDToUse = AdoFldLong(rs, "MainPhysician", -1);

				if(nProviderIDToUse == -1) {
					//if still no provider, add to our error list

					long nFound = 0;
					if(mapSkippedPatientIDs.Lookup(nPatientID, nFound)) {
						//this patient already exists in our map,
						//so skip this record
						rs->MoveNext();
						continue;
					}

					CString str;
					str.Format("\n%s", AdoFldString(rs, "PatientName", ""));
					strPatientsWithNoProvider += str;

					mapSkippedPatientIDs.SetAt(nPatientID, (long)1);

					//skip this record
					rs->MoveNext();
					continue;
				}
			}

			//get the location ID per our settings
			//1 - appt. location, 2 - G2 location, 3 - current location
			long nLocationIDToUse = -1;		
			if(nLocationType == 1) {
				nLocationIDToUse = AdoFldLong(rs, "ApptLocationID", -1);
			}
			else if(nLocationType == 2) {
				nLocationIDToUse = AdoFldLong(rs, "PatientLocationID", -1);
			}
			if(nLocationIDToUse == -1) {
				//if nLocationType = 3 or we failed to load a location
				nLocationIDToUse = GetCurrentLocationID();
			}

			//get the diagnosis codes, if requested
			CString strDiagID1 = "NULL";
			CString strDiagID2 = "NULL";
			CString strDiagID3 = "NULL";
			CString strDiagID4 = "NULL";

			if(nSendDiagCodes == 1) {

				long nDiagID1 = -1;
				long nDiagID2 = -1;
				long nDiagID3 = -1;
				long nDiagID4 = -1;

				// (j.jones 2014-03-10 08:19) - PLID 61243 - Eligibility requests only have 4 codes,
				// of any type, whereas G2 defaults have 4 pairs of codes. Follow the insurance
				// setting to determine which set to use.				
				bool bUseICD10 = AdoFldBool(rs, "UseICD10Codes", FALSE) ? true : false;
				if(bUseICD10) {
					//use ICD-10
					nDiagID1 = AdoFldLong(rs, "DefaultICD10DiagID1", -1);
					nDiagID2 = AdoFldLong(rs, "DefaultICD10DiagID2", -1);
					nDiagID3 = AdoFldLong(rs, "DefaultICD10DiagID3", -1);
					nDiagID4 = AdoFldLong(rs, "DefaultICD10DiagID4", -1);
				}
				else {
					//use ICD-9
					nDiagID1 = AdoFldLong(rs, "DefaultDiagID1", -1);
					nDiagID2 = AdoFldLong(rs, "DefaultDiagID2", -1);
					nDiagID3 = AdoFldLong(rs, "DefaultDiagID3", -1);
					nDiagID4 = AdoFldLong(rs, "DefaultDiagID4", -1);
				}

				if(nDiagID1 != -1) {
					strDiagID1 = AsString(nDiagID1);
				}
				if(nDiagID2 != -1) {
					strDiagID2 = AsString(nDiagID2);
				}
				if(nDiagID3 != -1) {
					strDiagID3 = AsString(nDiagID3);
				}
				if(nDiagID4 != -1) {
					strDiagID4 = AsString(nDiagID4);
				}
			}

			// (c.haag 2010-10-15 9:57) - PLID 40352 - Get the appoinment place of service designation.
			// This is required for ANSI 5010. Meanwhile, ANSI 4010 will just ignore this.
			CString strPOSID = "NULL";
			long nPOSID = AdoFldLong(rs, "ApptPOSID", -1);
			if (-1 != nPOSID) {
				strPOSID = AsString(nPOSID);
			}

			// (j.jones 2011-10-07 15:55) - PLID 37659 - added AppointmentID
			// (j.jones 2013-03-28 15:07) - PLID 52182 - changed the category to be an ID, not a string
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewID = (SELECT COALESCE(Max(ID), 0) + 1 FROM EligibilityRequestsT)");
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EligibilityRequestsT (ID, "
				"InsuredPartyID, ProviderID, LocationID, Batched, Selected, "
				"CreateDate, LastSentDate, BenefitCategoryID, "
				"Diagnosis1, Diagnosis2, Diagnosis3, Diagnosis4, PlaceOfServiceID, AppointmentID) "
				"VALUES (@nNewID, "
				"%li, %li, %li, 1, 0, "
				"GetDate(), NULL, %li, "
				"%s, %s, %s, %s, %s, %li)", nInsuredPartyID, nProviderIDToUse, nLocationIDToUse,
				nCategoryID, strDiagID1, strDiagID2, strDiagID3, strDiagID4, strPOSID, nAppointmentID);

			// (j.jones 2010-07-07 17:58) - PLID 39515 - we need to know the new IDs we created
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO @NewIDsT (RequestID) VALUES (@nNewID)");

			nCountCreated++;

			//track that we made a request for this insured party
			mapInsuredPartyIDs.SetAt(nInsuredPartyID, (long)1);

			rs->MoveNext();
		}
		rs->Close();

		if(nCountCreated > 0) {			
			_RecordsetPtr rs = CreateRecordset("SET NOCOUNT ON \r\n"
				"BEGIN TRAN \r\n "
				"%s "
				"COMMIT TRAN \r\n "
				"SET NOCOUNT OFF \r\n"
				"SELECT RequestID FROM @NewIDsT", strSqlBatch);

			// (j.jones 2010-07-07 17:56) - PLID 39515 - if real-time exporting is enabled, do so now
			if(bUseRealTimeElig) {

				//if we were not given a format, load it now
				if (m_nFormatID == -1) {
					// (j.jones 2016-05-19 10:22) - NX-100685 - added a universal function for getting the default E-Billing / E-Eligibility format
					m_nFormatID = GetDefaultEbillingANSIFormatID();
				}

				CEEligibility dlg(this);
				dlg.m_FormatID = m_nFormatID;
				// (j.jones 2010-10-21 12:34) - PLID 40914 - renamed to reflect that this is not Trizetto-only
				dlg.m_bUseRealTimeElig = TRUE;

				while(!rs->eof) {

					//add our new requests
					dlg.m_aryRequestIDsToExport.Add(AdoFldLong(rs, "RequestID"));

					rs->MoveNext();
				}

				rs->Close();
				
				dlg.DoModal();
			}
			else {
				rs->Close();
			}
		}

		CString strMessage, str;
		if(nCountCreated == 0) {
			strMessage = "E-Eligibility request generation is complete. No new E-Eligibility requests were created.";
		}
		else if(nCountCreated == 1) {
			strMessage = "E-Eligibility request generation is complete. 1 new E-Eligibility request was created.";
		}
		else {
			strMessage.Format("E-Eligibility request generation is complete. %li new E-Eligibility requests were created.", nCountCreated);
		}

		if(!strPatientsWithNoProvider.IsEmpty()) {
			str.Format("\n\nThe following patients have no provider selected on their account and could not have a request created:"
				"%s", strPatientsWithNoProvider);
			strMessage += str;
		}

		// (j.jones 2010-07-08 08:51) - PLID 39515 - If we successfully exported requests and have
		// no warning messages, then don't display a confirmation message.
		if(!bUseRealTimeElig || nCountCreated == 0 || !strPatientsWithNoProvider.IsEmpty()) {
			AfxMessageBox(strMessage);
		}

		CNxDialog::OnOK();

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnCreateRequests");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnSelectOneApptType()
{
	try {

		m_SelectedApptTypeList->TakeCurrentRowAddSorted(m_UnselectedApptTypeList, NULL);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnSelectOneApptType");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnSelectAllApptTypes()
{
	try {

		m_SelectedApptTypeList->TakeAllRows(m_UnselectedApptTypeList);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnSelectAllApptTypes");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnUnselectOneApptType()
{
	try {

		m_UnselectedApptTypeList->TakeCurrentRowAddSorted(m_SelectedApptTypeList, NULL);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnUnselectOneApptType");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnUnselectAllApptTypes()
{
	try {

		m_UnselectedApptTypeList->TakeAllRows(m_SelectedApptTypeList);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnUnselectAllApptTypes");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnSelectOneResource()
{
	try {

		m_SelectedResourceList->TakeCurrentRowAddSorted(m_UnselectedResourceList, NULL);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnSelectOneResource");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnSelectAllResources()
{
	try {

		m_SelectedResourceList->TakeAllRows(m_UnselectedResourceList);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnSelectAllResources");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnUnselectOneResource()
{
	try {

		m_UnselectedResourceList->TakeCurrentRowAddSorted(m_SelectedResourceList, NULL);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnUnselectOneResource");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnUnselectAllResources()
{
	try {

		m_UnselectedResourceList->TakeAllRows(m_SelectedResourceList);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnUnselectAllResources");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnSelectOneInsco()
{
	try {

		m_SelectedInsCoList->TakeCurrentRowAddSorted(m_UnselectedInsCoList, NULL);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnSelectOneInsco");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnSelectAllInscos()
{
	try {

		m_SelectedInsCoList->TakeAllRows(m_UnselectedInsCoList);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnSelectAllInscos");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnUnselectOneInsco()
{
	try {

		m_UnselectedInsCoList->TakeCurrentRowAddSorted(m_SelectedInsCoList, NULL);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnUnselectOneInsco");
}

void CEligibilityCreateForScheduledPatientsDlg::OnBtnUnselectAllInscos()
{
	try {

		m_UnselectedInsCoList->TakeAllRows(m_SelectedInsCoList);

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnBtnUnselectAllInscos");
}

BEGIN_EVENTSINK_MAP(CEligibilityCreateForScheduledPatientsDlg, CNxDialog)
ON_EVENT(CEligibilityCreateForScheduledPatientsDlg, IDC_ELIG_APPT_TYPE_UNSELECTED_LIST, 3, OnDblClickCellApptTypeUnselectedList, VTS_DISPATCH VTS_I2)
ON_EVENT(CEligibilityCreateForScheduledPatientsDlg, IDC_ELIG_APPT_TYPE_SELECTED_LIST, 3, OnDblClickCellApptTypeSelectedList, VTS_DISPATCH VTS_I2)
ON_EVENT(CEligibilityCreateForScheduledPatientsDlg, IDC_ELIG_APPT_RESOURCE_UNSELECTED_LIST, 3, OnDblClickCellApptResourceUnselectedList, VTS_DISPATCH VTS_I2)
ON_EVENT(CEligibilityCreateForScheduledPatientsDlg, IDC_ELIG_APPT_RESOURCE_SELECTED_LIST, 3, OnDblClickCellApptResourceSelectedList, VTS_DISPATCH VTS_I2)
ON_EVENT(CEligibilityCreateForScheduledPatientsDlg, IDC_ELIG_INSCOS_UNSELECTED_LIST, 3, OnDblClickCellInscosUnselectedList, VTS_DISPATCH VTS_I2)
ON_EVENT(CEligibilityCreateForScheduledPatientsDlg, IDC_ELIG_INSCOS_SELECTED_LIST, 3, OnDblClickCellInscosSelectedList, VTS_DISPATCH VTS_I2)
ON_EVENT(CEligibilityCreateForScheduledPatientsDlg, IDC_ELIG_APPT_LOCATION_FILTER, 16, OnSelChosenApptLocationFilter, VTS_DISPATCH)
ON_EVENT(CEligibilityCreateForScheduledPatientsDlg, IDC_ELIG_SCHED_CATEGORY_COMBO, 16, OnSelChosenSchedCategoryCombo, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CEligibilityCreateForScheduledPatientsDlg::OnDblClickCellApptTypeUnselectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBtnSelectOneApptType();

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnDblClickCellApptTypeUnselectedList");
}

void CEligibilityCreateForScheduledPatientsDlg::OnDblClickCellApptTypeSelectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBtnUnselectOneApptType();

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnDblClickCellApptTypeSelectedList");
}

void CEligibilityCreateForScheduledPatientsDlg::OnDblClickCellApptResourceUnselectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBtnSelectOneResource();		

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnDblClickCellApptResourceUnselectedList");
}

void CEligibilityCreateForScheduledPatientsDlg::OnDblClickCellApptResourceSelectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBtnUnselectOneResource();		

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnDblClickCellApptResourceSelectedList");
}

void CEligibilityCreateForScheduledPatientsDlg::OnDblClickCellInscosUnselectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBtnSelectOneInsco();		

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnDblClickCellInscosUnselectedList");
}

void CEligibilityCreateForScheduledPatientsDlg::OnDblClickCellInscosSelectedList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		OnBtnUnselectOneInsco();

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnDblClickCellInscosSelectedList");
}

void CEligibilityCreateForScheduledPatientsDlg::OnSelChosenApptLocationFilter(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//reselect their default
			long nLocationID = GetRemotePropertyInt("EligibilityCreateForScheduled_DefLocationID", -1, 0, GetCurrentUserName(), TRUE);
			if(NULL == m_LocationCombo->SetSelByColumn(lccID, (long)nLocationID)) {
				m_LocationCombo->SetSelByColumn(lccID, (long)-1);
			}
		}

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnSelChosenApptLocationFilter");
}

void CEligibilityCreateForScheduledPatientsDlg::OnSelChosenSchedCategoryCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			//reselect their default
			// (j.jones 2013-03-28 15:50) - PLID 52182 - removed EligibilityCreateForScheduled_DefCategory,
			// replaced with DefaultEligibilityBenefitCategory, which applies every time
			CString strCategory = GetRemotePropertyText("DefaultEligibilityBenefitCategory", "30", 0, "<None>", TRUE);
			m_BenefitCategoryCombo->TrySetSelByColumn_Deprecated(bcccCode, _bstr_t(strCategory));
		}

	}NxCatchAll("Error in CEligibilityCreateForScheduledPatientsDlg::OnSelChosenSchedCategoryCombo");
}

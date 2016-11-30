// (r.gonet 08/05/2013) - PLID 58200
// MedicationHistoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "Practice.h"
#include "MedicationHistoryDlg.h"
#include "PrescriptionUtilsAPI.h"
#include "PrescriptionUtilsNonAPI.h"
#include "MedicationHistoryMedInfoPopupDlg.h"
#include "foreach.h"
#include "FirstDataBankUtils.h"
#include "PrescriptionQueueDlg.h"
#include "DontShowDlg.h"
#include "MsgBox.h"
#include "MedicationHistoryUtils.h"
#include "SSEligibilityDlg.h"
using namespace ADODB;
using namespace NXDATALIST2Lib;
using namespace MedicationHistoryUtils;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (r.gonet 08/09/2013) - PLID 57981 - Timer for the Medication History Request timeout.
#define ID_MED_HISTORY_REQUEST_TIMEOUT 1001
// (r.gonet 08/09/2013) - PLID 57981 - Timer for a manual request check every few seconds because we don't trust table checkers.
#define ID_MED_HISTORY_REQUEST_STATUS_CHECK 1002
// (r.gonet 08/09/2013) - PLID 57981 - Color of pending requests. Dull yellow.
#define ROW_REQUESTING_COLOR		RGB(255,253,170)
// (r.gonet 08/09/2013) - PLID 57981 - What color should the error rows be in the Request list. Light red.
#define ROW_NEEDS_ATTENTION_COLOR	RGB(255,127,127)

namespace MedHistoryList
{
	// (a.wilson 2013-08-19 10:08) - PLID 57844 -  added import column.
	enum MedHistoryListCols
	{
		GeneratedKey = 0,
		Import,
		RxDescription,
		RxLastFillDate,
		PrescriberName,						
		Pharmacy,
		RxSourceDescription,		
	};
}
/////////////////////////////////////////////////////////////////////////////
// CMedicationHistoryDlg dialog

// (r.gonet 08/09/2013) - PLID 58200 - Constructs the medication history dialog.
// - pParent: Parent window
// - nPatientID: The current patient. We make requests for this patient.
CMedicationHistoryDlg::CMedicationHistoryDlg(CWnd* pParent, long nPatientID)
	: CNxDialog(CMedicationHistoryDlg::IDD, pParent),
	m_checkMedHistory_SequenceCompleted(NetUtils::MedHistory_SequenceCompleted),
	m_checkMedicationHistoryResponses(NetUtils::MedicationHistoryResponseT)
{
	//{{AFX_DATA_INIT(CMedicationHistoryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// (r.gonet 08/09/2013) - PLID 58200 - Initialize the patient ID we will be requesting for.
	m_nPatientID = nPatientID;
	// (r.gonet 09/20/2013) - PLID 57978 - When we start out, there is no master request loaded.
	m_pCurrentMasterRequest = NULL;
	// (a.wilson 2013-08-30 16:52) - PLID 57844
	m_bImportedHistoryMeds = false;
	// (r.gonet 09/20/2013) - PLID 58200 - Create a new "fine print" font for the disclaimer and the last requested by label.
	m_pFinePrint = new CFont;
	CreateCompatiblePointFont(m_pFinePrint, 80, "Arial");
	// (r.gonet 09/20/2013) - PLID 58200 - Default the background color to be the patient color.
	m_nColor = GetNxColor(GNC_PATIENT_STATUS, 1, m_nPatientID);
}

// (r.gonet 08/13/2013) - PLID 58200 - Destructor
CMedicationHistoryDlg::~CMedicationHistoryDlg()
{
	// (r.gonet 09/20/2013) - PLID 58200 - Clean up the fine print.
	if(m_pFinePrint != NULL) {
		delete m_pFinePrint;
		m_pFinePrint = NULL;
	}
}

void CMedicationHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMedicationHistoryDlg)
	DDX_Control(pDX, IDC_MED_HISTORY_REQUEST_COLOR, m_nxcolorRequest);
	DDX_Control(pDX, IDC_MED_HISTORY_REQUEST_BTN, m_nxbRequestMedicationHistory);
	DDX_Control(pDX, IDC_MED_HISTORY_LAST_REQUEST_LABEL, m_nxsLastRequestedLabel);
	DDX_Control(pDX, IDC_MED_HISTORY_SEEING_PT_WITHIN_3_DAYS_CHECK, m_checkSeeingPatientWithinThreeDays);
	DDX_Control(pDX, IDC_MED_HISTORY_VIEW_ELIGIBILITY_DETAILS_BTN, m_nxbViewEligibilityDetails);
	DDX_Control(pDX, IDC_MED_HISTORY_REQUEST_LABEL, m_nxsRequestLabel);
	DDX_Control(pDX, IDC_MED_HISTORY_REQ_PROVIDER_LBL, m_nxsRequestingProvider);
	DDX_Control(pDX, IDC_MED_HISTORY_FROM_DATE_CHECK, m_checkRangeFrom);
	DDX_Control(pDX, IDC_MED_HISTORY_REQ_FROM_DTP, m_dtpFromDate);
	DDX_Control(pDX, IDC_MED_HISTORY_TO_LABEL, m_nxsTo);
	DDX_Control(pDX, IDC_MED_HISTORY_REQ_TO_DTP, m_dtpToDate);
	DDX_Control(pDX, IDC_MED_HISTORY_CONSENT_CHECK, m_checkPatientConsentObtained);
	DDX_Control(pDX, IDC_MED_HISTORY_REQUESTS_STATUSES_LABEL, m_nxsRequestsStatusesLabel);
	DDX_Control(pDX, IDC_MED_HISTORY_REQUESTS_STATUSES_COLOR, m_nxcolorRequestsStatuses);
	DDX_Control(pDX, IDC_MED_HISTORY_DISCLAIMER_EDIT, m_nxeditDisclaimer);
	DDX_Control(pDX, IDC_MED_HISTORY_RESPONSES_COLOR, m_nxcolorResponses);
	DDX_Control(pDX, IDC_MED_HISTORY_RESPONSES_LABEL, m_nxsResponsesLabel);
	DDX_Control(pDX, IDOK, m_nxbClose);
	DDX_Control(pDX, IDC_IMPORT_MEDICATION_BTN, m_nxbImport);
	DDX_Control(pDX, IDC_CHECK_ALL_MEDICATIONS_BTN, m_nxbCheckAllImport);
	DDX_Control(pDX, IDC_HISTORY_MEDICATION_IMPORT_RESULTS, m_nxMedImportStatus);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMedicationHistoryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMedicationHistoryDlg)
	ON_BN_CLICKED(IDC_MED_HISTORY_REQUEST_BTN, &CMedicationHistoryDlg::OnBnClickedMedHistoryRequestBtn)
	ON_BN_CLICKED(IDC_MED_HISTORY_VIEW_ELIGIBILITY_DETAILS_BTN, &CMedicationHistoryDlg::OnBnClickedMedHistoryViewEligibilityDetailsBtn)
	ON_BN_CLICKED(IDC_MED_HISTORY_FROM_DATE_CHECK, &CMedicationHistoryDlg::OnBnClickedMedHistoryFromDateCheck)
	ON_BN_CLICKED(IDC_MED_HISTORY_CONSENT_CHECK, &CMedicationHistoryDlg::OnBnClickedMedHistoryConsentCheck)
	ON_MESSAGE(WM_TABLE_CHANGED, &CMedicationHistoryDlg::OnTableChanged)	
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_IMPORT_MEDICATION_BTN, &CMedicationHistoryDlg::OnBnClickedImportMedicationBtn)
	ON_BN_CLICKED(IDC_CHECK_ALL_MEDICATIONS_BTN, &CMedicationHistoryDlg::OnBnClickedCheckAllMedicationsBtn)
	ON_NOTIFY(NM_CLICK, IDC_HISTORY_MEDICATION_IMPORT_RESULTS, &CMedicationHistoryDlg::OnNMClickHistoryMedicationImportResults)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMedicationHistoryDlg message handlers

BEGIN_EVENTSINK_MAP(CMedicationHistoryDlg, CNxDialog)
	//{{AFX_EVENTSINK_MAP(CMedicationHistoryDlg)
	ON_EVENT(CMedicationHistoryDlg, IDC_MED_HISTORY_LOCATION_COMBO, 18 /* RequeryFinished */, CMedicationHistoryDlg::OnRequeryFinishedLocationCombo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CMedicationHistoryDlg, IDC_MED_HISTORY_REQ_PROVIDER_COMBO, 1, CMedicationHistoryDlg::SelChangingMedHistoryReqProviderCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMedicationHistoryDlg, IDC_MED_HISTORY_LOCATION_COMBO, 1, CMedicationHistoryDlg::SelChangingMedHistoryLocationCombo, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CMedicationHistoryDlg, IDC_MED_HISTORY_LIST, 3, CMedicationHistoryDlg::DblClickCellMedHistoryList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CMedicationHistoryDlg, IDC_MED_HISTORY_REQUESTS_LIST, 19, CMedicationHistoryDlg::LeftClickMedHistoryRequestsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMedicationHistoryDlg, IDC_MED_HISTORY_LOCATION_COMBO, 16, CMedicationHistoryDlg::SelChosenLocationCombo, VTS_DISPATCH)
	ON_EVENT(CMedicationHistoryDlg, IDC_MED_HISTORY_LIST, 19, CMedicationHistoryDlg::LeftClickMedHistoryList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// (r.gonet 08/05/2013) - PLID 58200 - Initializes the dialog control values.
BOOL CMedicationHistoryDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//Set our icons and colors
		SetTitleBarIcon(IDI_MEDICATION_HISTORY);

		// (r.gonet 09/20/2013) - PLID 58397 - Cache the properties
		g_propManager.BulkCache("CMedicationHistoryDlg", propbitNumber, " (Username = '<None>' OR Username = '%s') AND ("
			"Name = 'MedHistory_UseLastFillDateRange' "
			"OR Name = 'MedHistory_DefaultFromNumMonthsInPast' "
			")", _Q(GetCurrentUserName()));

		// (r.gonet 08/19/2013) - PLID 58200 - Added an icon to the request button
		m_nxbRequestMedicationHistory.AutoSet(NXB_SENDTO);
		//m_nxbViewEligibilityDetails.AutoSet(
		// (a.wilson 2013-08-19 10:13) - PLID 57844 - button icon.
		m_nxbImport.AutoSet(NXB_IMPORTBOX);
		m_nxbClose.AutoSet(NXB_CLOSE);

		// (r.gonet 09/20/2013) - PLID 58200 - Make the label that shows who requested the last medication history and when 
		// have a fine print font to cram information on the screen.
		m_nxsLastRequestedLabel.SetFont(m_pFinePrint);
		m_nxsLastRequestedLabel.SetWindowText("");
		// (r.gonet 09/20/2013) - PLID 58200 - EMR wants the disclaimer SureScripts is making us display shoved into the corner in a small font.
		m_nxeditDisclaimer.SetFont(m_pFinePrint, FALSE);
		m_nxeditDisclaimer.SetWindowText(
			"DISCLAIMER: Certain information may not be available or accurate in this report, including items "
			"that the patient asked not be disclosed due to patient privacy concerns, over-the-counter "
			"medications, low cost prescriptions, prescriptions paid for by the patient or "
			"non-participating sources, or errors in insurance claims information. The provider should "
			"independently verify medication history with the patient.");

		// (r.gonet 08/09/2013) - PLID 58200 - Set our nxcolor colors to reflect the patients module.
		m_nxcolorRequest.SetColor(m_nColor);
		m_nxcolorRequestsStatuses.SetColor(m_nColor);
		m_nxcolorResponses.SetColor(m_nColor);

		// (r.gonet 08/05/2013) - PLID 58397 - Remember if they used the last fill date range filter last time. Default to unused.
		if(GetRemotePropertyInt("MedHistory_UseLastFillDateRange", FALSE, 0, GetCurrentUserName(), true)) {
			m_checkRangeFrom.SetCheck(BST_CHECKED);	
		} else {
			m_checkRangeFrom.SetCheck(BST_UNCHECKED);
		}
		
		// (r.gonet 09/20/2013) - PLID 58397 - EMR couldn't come to agreement on a default from date, so my suggestion
		//  was to remember whatever it was before per-user. Default to 2 years in the past.
		long nDefaultFromNumMonthsInPast = GetRemotePropertyInt("MedHistory_DefaultFromNumMonthsInPast", 24, 0, GetCurrentUserName(), true);
		if(nDefaultFromNumMonthsInPast < 0) {
			// This is invalid. Revert to 2 years.
			nDefaultFromNumMonthsInPast = 24;
		}
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		// Calculate the number of years and months from the total months
		long nYears = nDefaultFromNumMonthsInPast / 12;
		long nMonths = nDefaultFromNumMonthsInPast % 12;
		COleDateTime dtFrom;
		dtFrom.SetDate(dtNow.GetYear() - nYears, dtNow.GetMonth() - nMonths, 1);
		m_dtpFromDate.SetValue(dtFrom);

		// (r.gonet 08/09/2013) - PLID 58200
		m_pLocationCombo = BindNxDataList2Ctrl(IDC_MED_HISTORY_LOCATION_COMBO, true);
		// (r.gonet 08/09/2013) - PLID 58200 - Don't query providers since we will be filling that manually each time a location is chosen.
		m_pRequestingProviderCombo = BindNxDataList2Ctrl(IDC_MED_HISTORY_REQ_PROVIDER_COMBO, false);
		// (r.gonet 08/09/2013) - PLID 57981 - Request list will be filled manually when statuses come back in after making the request.
		m_pMedicationHistoryRequestList = BindNxDataList2Ctrl(IDC_MED_HISTORY_REQUESTS_LIST, false);
		// (r.gonet 09/21/2013) - PLID 58200 - Bind the response list.
		m_pMedicationHistoryList = BindNxDataList2Ctrl(IDC_MED_HISTORY_LIST, false);

		_RecordsetPtr prs = CreateParamRecordset(
			// (r.gonet 09/20/2013) - PLID 58200 - Get the patient's name and ID for the title bar.
			"SELECT PersonT.FullName, PatientsT.UserDefinedID "
			"FROM PatientsT "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"WHERE PersonT.ID = {INT}; "
			""
			// (r.gonet 09/20/2013) - PLID 57978 - Get whether the patient has an appointment in 72 hours time
			"SELECT AppointmentsT.ID "
			"FROM AppointmentsT "
			"WHERE AppointmentsT.PatientID = {INT} "
			"	AND AppointmentsT.Status <> 4 "
			"	AND AppointmentsT.StartTime >= DATEADD(hh, -72, GETDATE()) "
			"	AND AppointmentsT.StartTime <= DATEADD(hh, 72, GETDATE()); "
			""
			// (r.gonet 08/05/2013) - PLID 57979 - We have an opt out system. See if the patient has opted in or out.
			"SELECT MedicationHistoryConsent FROM PatientsT WHERE PersonID = {INT}; ",
			m_nPatientID,
			m_nPatientID,
			m_nPatientID);
		if(!prs->eof) {
			// (r.gonet 09/20/2013) - PLID 58200 - Get the patient's name and id for the title bar
			long nUserDefinedID = AdoFldLong(prs->Fields, "UserDefinedID");
			CString strPatientFullName = AdoFldString(prs->Fields, "FullName");
			CString strWindowTitle;
			this->GetWindowText(strWindowTitle);
			strWindowTitle += FormatString(" - %s (%li)", strPatientFullName, nUserDefinedID);
			this->SetWindowText(strWindowTitle);
		}
		prs = prs->NextRecordset(NULL);
		if(!prs->eof) {
			// (r.gonet 09/20/2013) - PLID 57978 - Patient has an appointment, so we'll be requesting eligibility
			m_checkSeeingPatientWithinThreeDays.SetCheck(BST_CHECKED);
		} else {
			// (r.gonet 09/20/2013) - PLID 57978 - Patient does not have an appointment, so we won't request eligibility unless they check this box manually.
			m_checkSeeingPatientWithinThreeDays.SetCheck(BST_UNCHECKED);
		}
		prs = prs->NextRecordset(NULL);
		if(!prs->eof) {
			// (r.gonet 09/20/2013) - PLID 57979 - Patient has not consented to medication history retrieval!
			BOOL bConsentObtained = AdoFldBool(prs->Fields, "MedicationHistoryConsent", TRUE);
			m_checkPatientConsentObtained.SetCheck(bConsentObtained ? BST_CHECKED : BST_UNCHECKED);
		}
		prs->Close();

		// (r.gonet 10/24/2013) - PLID 59104
		CheckEligibilityResponse();

		// (r.gonet 09/20/2013) - PLID 58200 - Load up the patient's current medication history if we have it already.
		LoadLastMasterRequest();
		if(m_pCurrentMasterRequest != NULL) {
			LoadResponseList();
		}
		// (r.gonet 08/09/2013) - PLID 58200 - Ensure that all controls that should
		// be disabled are disabled.
		EnsureControls();

		// (r.gonet 09/20/2013) - PLID 57981 - Start looking out for changes to tables.
		GetMainFrame()->RequestTableCheckerMessages(this->GetSafeHwnd());		

		// (r.gonet 08/09/2013) - PLID 58200 - Describe some of how the dialog works.
		DontShowMeAgain(this, 
			"This screen allows you to obtain and view the patient's medication history. In order to submit a request, "
			"the requesting provider must be registered with SureScripts and the patient must consent to medication "
			"history retrieval. Your request will be made to SureScripts to obtain historical eRx fill data and also to Pharmacy "
			"Benefit Managers (PBMs) associated with all eligible coverages.", "MedicationHistoryHelp", "Medication History Help", 
			FALSE, FALSE, FALSE);

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (r.gonet 10/24/2013) - PLID 59104 - Checks for failures in the eligibility response. Sets the appropriate controls
void CMedicationHistoryDlg::CheckEligibilityResponse()
{
	_RecordsetPtr prs = CreateRecordset(
		"SELECT * \r\n"
		"FROM \r\n"
		"%s "
		"WHERE PatientID = %li",
		GetFormularyInsuranceSQL(), m_nPatientID);

	bool bOneFailed = false;
	bool bDemographicsChanged = false;
	while(!prs->eof) {
		if(AdoFldBool(prs->Fields, "IsFailure", FALSE)) {
			bOneFailed = true;
		}
		if(AdoFldBool(prs->Fields, "DemographicsHaveChanged", FALSE)) {
			bDemographicsChanged = true;
		}
		prs->MoveNext();
	}
	if(bOneFailed) {
		m_nxbViewEligibilityDetails.AutoSet(NXB_WARNING);
	} else {
		m_nxbViewEligibilityDetails.AutoSet(NXB_NOTUSED);
	}
	if(bDemographicsChanged) {
		MessageBox("The patient demographics returned in the eligibility request do not match exactly what is "
			"stored in Practice for this patient. Please review the demographics by clicking 'View Eligibility Details' to be "
			"sure that the patient has been correctly matched.");
	}

	prs->Close();
}

void CMedicationHistoryDlg::CheckForRxHistoryTruncation(long nPatientID, long nProviderID, long nLocationID)
{
	CString strTruncationWarnings;
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT TOP 1 \r\n"
            "NexERxPrescriberT.SPI AS PrescriberSPI, \r\n"
            "NexERxPrescriberT.NPI AS PrescriberNPI, \r\n"
            "NexERxPrescriberT.StateLicenseNumber AS PrescriberStateLicenseNumber, \r\n"
            "NexERxPrescriberT.DEA AS PrescriberDEA, \r\n"
            "NexERxPrescriberT.ClinicName AS PrescriberClinicName, \r\n"
            "NexERxPrescriberT.FirstName AS PrescriberFirst, \r\n"
            "NexERxPrescriberT.MiddleName AS PrescriberMiddle, \r\n"
            "NexERxPrescriberT.LastName AS PrescriberLast, \r\n"
            "NexERxPrescriberT.PrefixName AS PrescriberPrefix, \r\n"
            "NexERxPrescriberT.SuffixName AS PrescriberSuffix, \r\n"
            "NexERxPrescriberT.AddressLine1 AS PrescriberAddress1, \r\n"
            "NexERxPrescriberT.AddressLine2 AS PrescriberAddress2, \r\n"
            "NexERxPrescriberT.City AS PrescriberCity, \r\n"
            "NexERxPrescriberT.PhonePrimary AS PrescriberMainPhone, \r\n"
            "NexERxPrescriberT.Fax AS PrescriberFax \r\n"
		"FROM ProvidersT  \r\n"
		// We need the SPI, so the provider must be registered with SureScripts "
		"INNER JOIN NexERxPrescriberRegistrationT ON NexERxPrescriberRegistrationT.ProviderID = ProvidersT.PersonID \r\n"
		// Use the location the master request was made at. "
		"AND NexERxPrescriberRegistrationT.LocationID = {INT} \r\n"
		"INNER JOIN NexERxPrescriberT ON NexERxPrescriberRegistrationT.SPI = NexERxPrescriberT.SPI \r\n"
		"WHERE ProvidersT.PersonID = {INT}; \r\n"
        "\r\n"
		"SELECT TOP 1 \r\n"
            "PatientPersonT.SocialSecurity AS PatientSocialSecurity, \r\n"
            "PatientPrefixT.Prefix AS PatientPrefix, \r\n"
            "PatientPersonT.First AS PatientFirst, \r\n"
            "PatientPersonT.Middle AS PatientMiddle, \r\n"
            "PatientPersonT.Last AS PatientLast, \r\n"
            "PatientPersonT.Suffix AS PatientSuffix, \r\n"
            "PatientPersonT.Address1 AS PatientAddress1, \r\n"
            "PatientPersonT.Address2 AS PatientAddress2, \r\n"
            "PatientPersonT.City AS PatientCity, \r\n"
            "REPLACE(REPLACE(REPLACE(REPLACE(PatientPersonT.HomePhone, '-', ''), '(', ''), ')', ''), ' ', '') AS PatientHomePhone, \r\n"
            "REPLACE(REPLACE(REPLACE(REPLACE((PatientPersonT.WorkPhone + CASE WHEN PatientPersonT.Extension <> '' THEN 'X' + PatientPersonT.Extension ELSE '' END), '-', ''), '(', ''), ')', ''), ' ', '') AS PatientWorkPhone, \r\n"
            "REPLACE(REPLACE(REPLACE(REPLACE(PatientPersonT.CellPhone, '-', ''), '(', ''), ')', ''), ' ', '') AS PatientCellPhone, \r\n"
            "REPLACE(REPLACE(REPLACE(REPLACE(PatientPersonT.Fax, '-', ''), '(', ''), ')', ''), ' ', '') AS PatientFax, \r\n"
            "REPLACE(REPLACE(REPLACE(REPLACE(PatientPersonT.Pager, '-', ''), '(', ''), ')', ''), ' ', '') AS PatientPager, \r\n"
            "REPLACE(REPLACE(REPLACE(REPLACE(PatientPersonT.OtherPhone, '-', ''), '(', ''), ')', ''), ' ', '') AS PatientOtherPhone \r\n"
		"FROM PatientsT \r\n"
		"INNER JOIN PersonT PatientPersonT ON PatientsT.PersonID = PatientPersonT.ID \r\n"
		"LEFT JOIN PrefixT AS PatientPrefixT ON PatientPersonT.PrefixID = PatientPrefixT.ID \r\n"
		"WHERE PatientsT.PersonID = {INT}; ",
		nLocationID, nProviderID, nPatientID);
	if(!prs->eof) {
		CString strProviderTruncationWarning;
		CheckForTruncation(prs->Fields, "PrescriberSPI", 35, "SPI", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberNPI", 35, "NPI", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberStateLicenseNumber", 35, "License Number", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberDEA", 35, "DEA", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberClinicName", 35, "Clinic Name", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberFirst", 35, "First Name", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberMiddle", 35, "Middle Name", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberLast", 35, "Last Name", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberPrefix", 10, "Prefix", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberSuffix", 10, "Suffix", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberAddress1", 35, "Address Line 1", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberAddress2", 35, "Address Line 2", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberCity", 35, "City", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberMainPhone", 80, "Work Phone + Ext.", strProviderTruncationWarning);
		CheckForTruncation(prs->Fields, "PrescriberFax", 80, "Fax", strProviderTruncationWarning);
		if(!strProviderTruncationWarning.IsEmpty()) {
			strProviderTruncationWarning = FormatString("The following provider fields are longer than what the medication history request "
				"allows and will be truncated in the request:\r\n%s", strProviderTruncationWarning);
			MessageBox(strProviderTruncationWarning, "Truncation Warning", MB_ICONEXCLAMATION|MB_OK);
		}
	}
	prs = prs->NextRecordset(NULL);
	if(!prs->eof) {
		CString strPatientTruncationWarning;
		CheckForTruncation(prs->Fields, "PatientSocialSecurity", 35, "Social Security Number", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientPrefix", 10, "Prefix", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientFirst", 35, "First Name", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientMiddle", 35, "Middle Name", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientLast", 35, "Last Name", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientSuffix", 10, "Suffix", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientAddress1", 35, "Address Line 1", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientAddress2", 35, "Address Line 2", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientCity", 35, "City", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientHomePhone", 80, "Home Phone", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientWorkPhone", 80, "Work Phone + Ext.", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientCellPhone", 80, "Mobile Phone", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientFax", 80, "Fax", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientPager", 80, "Pager", strPatientTruncationWarning);
		CheckForTruncation(prs->Fields, "PatientOtherPhone", 80, "Other Phone Number", strPatientTruncationWarning);
		if(!strPatientTruncationWarning.IsEmpty()) {
			strPatientTruncationWarning = FormatString("The following patient fields are longer than what the medication history request "
				"allows and will be truncated in the request:\r\n%s", strPatientTruncationWarning);
			MessageBox(strPatientTruncationWarning, "Truncation Warning", MB_ICONEXCLAMATION|MB_OK);
		}
	}
}

void CMedicationHistoryDlg::CheckForTruncation(ADODB::FieldsPtr pFields, CString strFieldName, int nMaxLength, CString strFriendlyName, IN OUT CString &strDisplayMessage)
{
	CString strValue = AsString(pFields->Item[(LPCTSTR)strFieldName]->Value);
	if(strValue.GetLength() > nMaxLength) {
		if(!strDisplayMessage.IsEmpty()) {
			strDisplayMessage += "\r\n";
		}
		strDisplayMessage += FormatString("- %s is more than %li character%s", strFriendlyName, nMaxLength, (nMaxLength == 1 ? "" : "s"));
	}
}

// (r.gonet 08/09/2013) - PLID 58200 - Load the provider combo. Once something has been chosen in 
// the location combo, we can pull the registered prescribers for that location and put them into 
// the provider combo.
void CMedicationHistoryDlg::LoadProviderCombo()
{
	// (r.gonet 08/09/2013) - PLID 58200 - Remove the providers associated with the old location.
	m_pRequestingProviderCombo->Clear();

	NXDATALIST2Lib::IRowSettingsPtr pLocationRow = m_pLocationCombo->CurSel;
	if(pLocationRow) {
		long nLocationID = VarLong(pLocationRow->GetValue(emhlcID));
		// (r.gonet 12/16/2013) - PLID 58199 - Changed public API arguments from integer to string.
		CString strLocationID = FormatString("%li", nLocationID);
		CString strCurrentUserID = FormatString("%li", GetCurrentUserID());
		// (r.gonet 09/20/2013) - PLID 58200 - Get the prescribers that are registered at this location and the current user can prescribe under.
		NexTech_Accessor::_RegisteredUserPrescriberArrayPtr pPrescriberArray = GetAPI()->GetRegisteredUserPrescribers(GetAPISubkey(), GetAPILoginToken(), _bstr_t(strCurrentUserID), _bstr_t(strLocationID));
		Nx::SafeArray<IUnknown*> saPrescribers(pPrescriberArray->Prescribers);
		CArray<long, long> aryProviderIDs;
		foreach(NexTech_Accessor::_RegisteredUserPrescriberPtr pPrescriber, saPrescribers) {
			// (r.gonet 12/16/2013) - PLID 58199 - Changed public API properties from integer to string.
			long nPrescriberID = atol((LPCTSTR)pPrescriber->PrescriberID);
			aryProviderIDs.Add(nPrescriberID);
		}
		if(aryProviderIDs.GetSize() > 0) {
			// (r.gonet 08/09/2013) - PLID 58200 - Get the names of the providers so we can add them to the selection combo.
			_RecordsetPtr prsRegisteredPrescribers = CreateParamRecordset(
				"SELECT ProvidersT.PersonID AS ID, ProviderPersonT.Last, ProviderPersonT.First, ProviderPersonT.Middle, ProviderPersonT.FullName "
				"FROM ProvidersT "
				"INNER JOIN PersonT ProviderPersonT ON ProvidersT.PersonID = ProviderPersonT.ID "
				"WHERE ID IN ({INTARRAY}) "
				, aryProviderIDs);
			// (r.gonet 08/09/2013) - PLID 58200 - Add all the registered prescribers
			// for this location to the providers combo.
			while(!prsRegisteredPrescribers->eof) {
				NXDATALIST2Lib::IRowSettingsPtr pProviderRow = m_pRequestingProviderCombo->GetNewRow();
				pProviderRow->PutValue(emhpcID, _variant_t(AdoFldLong(prsRegisteredPrescribers->Fields, "ID"), VT_I4));
				pProviderRow->PutValue(emhpcLast, _bstr_t(AdoFldString(prsRegisteredPrescribers->Fields, "Last")));
				pProviderRow->PutValue(emhpcFirst, _bstr_t(AdoFldString(prsRegisteredPrescribers->Fields, "First")));
				pProviderRow->PutValue(emhpcMiddle, _bstr_t(AdoFldString(prsRegisteredPrescribers->Fields, "Middle")));
				pProviderRow->PutValue(emhpcFullName, _bstr_t(AdoFldString(prsRegisteredPrescribers->Fields, "FullName")));
				m_pRequestingProviderCombo->AddRowAtEnd(pProviderRow, NULL);
				if(m_pRequestingProviderCombo->CurSel == NULL) {
					// (r.gonet 08/09/2013) - PLID 58200 - Pick something by default.
					m_pRequestingProviderCombo->CurSel = pProviderRow;
				}
				prsRegisteredPrescribers->MoveNext();
			}
			prsRegisteredPrescribers->Close();
		} else {
			// (r.gonet 08/09/2013) - PLID 58200 - No registered prescribers for this location.
			NXDATALIST2Lib::IRowSettingsPtr pProviderRow = m_pRequestingProviderCombo->GetNewRow();
			pProviderRow->PutValue(emhpcID, _variant_t(-1L, VT_I4));
			pProviderRow->PutValue(emhpcFullName, _bstr_t(" < No SureScripts Registered Providers Found >"));
			m_pRequestingProviderCombo->AddRowAtEnd(pProviderRow, NULL);
			m_pRequestingProviderCombo->CurSel = pProviderRow;
		}
	} else {
		// (r.gonet 08/09/2013) - PLID 58200 - No location selected.
		// Registered prescribers are associated with a location.
	}
}

// (r.gonet 08/05/2013) - PLID 57981 - Clean up any resources upon destruction of the window.
void CMedicationHistoryDlg::OnDestroy()
{
	try {
		// (r.gonet 09/20/2013) - PLID 57981 - Stop requesting table checker messages.
		GetMainFrame()->UnrequestTableCheckerMessages(this->GetSafeHwnd());
	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnDestroy();
}

// (r.gonet 08/05/2013) - PLID 58200 - Ensure every dialog control is enabled/disabled that should 
// be enabled/disabled.
void CMedicationHistoryDlg::EnsureControls()
{
	m_dtpFromDate.EnableWindow(m_checkRangeFrom.GetCheck());
	m_dtpToDate.EnableWindow(m_checkRangeFrom.GetCheck());
	// (r.gonet 08/09/2013) - PLID 57979 - Don't let the user request medication history if
	// the patient gave consent or if we are in the middle of a request session.
	m_nxbRequestMedicationHistory.EnableWindow(m_checkPatientConsentObtained.GetCheck() && AreAllRequestsComplete() == true ? TRUE : FALSE);
	// (r.gonet 08/09/2013) - PLID 58200 - Provider combo should only be available if
	// it actually has registered providers in it.
	NXDATALIST2Lib::IRowSettingsPtr pProviderRow = m_pRequestingProviderCombo->GetFirstRow();
	if(pProviderRow == NULL || VarLong(pProviderRow->GetValue(emhpcID)) == -1) {
		GetDlgItem(IDC_MED_HISTORY_REQ_PROVIDER_COMBO)->EnableWindow(FALSE);
		m_pRequestingProviderCombo->ReadOnly = VARIANT_TRUE;
	} else {
		GetDlgItem(IDC_MED_HISTORY_REQ_PROVIDER_COMBO)->EnableWindow(TRUE);
		m_pRequestingProviderCombo->ReadOnly = VARIANT_FALSE;
	}
}

// (r.gonet 08/05/2013) - PLID 58200 - Set the NxColor background color.
void CMedicationHistoryDlg::SetColor(OLE_COLOR nColor)
{
	m_nColor = nColor;
}

// CMedicationHistoryDlg message handlers

// (r.gonet 08/05/2013) - PLID 57978 - Handler for the Request Med History button. Validates control
// values and starts a request from SureScripts.
void CMedicationHistoryDlg::OnBnClickedMedHistoryRequestBtn()
{
	try {
		if(!CheckCurrentUserPermissions(bioPatientMedicationHistory, sptDynamic0)) {
			// Permission denied to request history!
			return;
		}
		// (r.gonet 08/09/2013) - PLID 57978 - Kill any lingering requests. This shouldn't do anything
		// if we have been handling things right. Just a precaution.
		StopWaitingForRequestStatusUpdates();		

		NXDATALIST2Lib::IRowSettingsPtr pLocationRow = m_pLocationCombo->CurSel;
		long nLocationID;
		if(pLocationRow == NULL || (nLocationID = VarLong(pLocationRow->GetValue(emhlcID), -1)) == -1) {
			// Location ID is mandatory.
			MessageBox("Please choose a location before requesting medication history.", 0, MB_ICONERROR);
			return;
		}

		// (r.gonet 08/09/2013) - PLID 57978 - Force the user to select a provider.
		NXDATALIST2Lib::IRowSettingsPtr pProviderRow = m_pRequestingProviderCombo->CurSel;
		long nProviderID;
		if(pProviderRow == NULL || (nProviderID = VarLong(pProviderRow->GetValue(emhpcID))) == -1) {
			MessageBox("Please choose a provider before requesting medication history.", 0, MB_ICONERROR);
			return;
		}

		// (r.gonet 08/09/2013) - PLID 57979 - SureScripts requirement. Ensure we have the patient's consent.
		if(m_checkPatientConsentObtained.GetCheck() != BST_CHECKED) {
			// (r.gonet 08/09/2013) - PLID 57979 - This should not occur since we would have the button disabled.
			ASSERT(FALSE);
			MessageBox("The patient has not consented to medication history retrieval.", 0, MB_ICONERROR);
			return;
		}

		// (r.gonet 09/20/2013) - PLID 57978 - Validate the last fill date range.
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dtFrom, dtTo; 
		if(m_checkRangeFrom.GetCheck() == BST_CHECKED) {
			dtFrom = m_dtpFromDate.GetValue();
			dtTo = m_dtpToDate.GetValue();
			if(m_checkRangeFrom.GetCheck() == BST_CHECKED && (dtFrom.GetStatus() != COleDateTime::valid || dtTo.GetStatus() != COleDateTime::valid || dtFrom > dtTo)) {
				// (r.gonet 09/20/2013) - PLID 57978 - The from date should not be more than the to date.
				MessageBox("Please enter a valid Last Fill date range.", 0, MB_ICONERROR);
				return;
			}
			long nMonths = ((long)dtNow.GetYear() * 12L + (long)dtNow.GetMonth()) - ((long)dtFrom.GetYear() * 12L + (long)dtFrom.GetMonth());
			if(nMonths > 24) {
				int nResult = DontShowMeAgain(this, "The Last Fill From date is more than two years in the past. Claim related medication history requests for dates more than two years in the past may return errors. Are you sure you want to continue?", "MedHistoryLastFillFromDateWarning", "Warning", FALSE, TRUE, FALSE);
				if(nResult == IDNO) {
					return;
				}
			}
			// (r.gonet 09/20/2013) - PLID 58397 - Save the number of months in the past they selected so
			// that we can load it next time.
			SetRemotePropertyInt("MedHistory_DefaultFromNumMonthsInPast", nMonths, 0, GetCurrentUserName());
		} else {
			// (r.gonet 09/20/2013) - PLID 57978 - They don't want to filter on a date range, leave the dates as null.
			dtFrom = dtTo = GetDateTimeNull();
		}
		// (r.gonet 09/20/2013) - PLID 58397 - Remember their choice of filtering on the date range or not.
		SetRemotePropertyInt("MedHistory_UseLastFillDateRange", (m_checkRangeFrom.GetCheck() == BST_CHECKED ? TRUE : FALSE), 0, GetCurrentUserName());

		GenerateTruncationWarning(m_nPatientID, nProviderID, this);
		// (r.gonet 10/24/2013) - PLID 59104 - Make an eligibility request and check the response. Set the appropriate controls.
		MedicationHistoryUtils::RequestEligibility(this->GetSafeHwnd(), m_nPatientID, nProviderID, nLocationID, m_checkSeeingPatientWithinThreeDays.GetCheck() == BST_CHECKED ? true : false);
		CheckEligibilityResponse();
		// Warn about truncation if necessary.
		CheckForRxHistoryTruncation(m_nPatientID, nProviderID, nLocationID);
		// (r.gonet 09/20/2013) - PLID 57978 - Request the medication history from SureScripts.
		NexTech_Accessor::_RxHistoryMasterRequestPtr pMasterRequest = MedicationHistoryUtils::RequestMedicationHistory(this->GetSafeHwnd(), m_nPatientID, nProviderID, nLocationID, dtFrom, dtTo);
		if(pMasterRequest) {
			// (r.gonet 09/20/2013) - PLID 57978 - Success! Our request went through. Prepare for the asynchronous responses by clearing out the request and response lists.
			m_pCurrentMasterRequest = pMasterRequest;
			m_pMedicationHistoryRequestList->Clear();
			m_pMedicationHistoryList->Clear();

			// (r.gonet 09/20/2013) - PLID 57978 - Update the last-requested-by label
			// (r.gonet 12/16/2013) - PLID 58199 - Changed public API properties from integer to string.
			CString strRequestedByUserID((LPCTSTR)m_pCurrentMasterRequest->RequestedByUserID);
			if(strRequestedByUserID != "") {
				long nRequestedByUserID = atol(strRequestedByUserID);
				_RecordsetPtr prsUser = CreateParamRecordset("SELECT Username FROM UsersT WHERE PersonID = {int}", nRequestedByUserID);
				CString strUserName = AdoFldString(prsUser, "Username");
				m_nxsLastRequestedLabel.SetWindowText(FormatString("Last requested on %s by %s", FormatDateTimeForInterface(m_pCurrentMasterRequest->RequestedDate), strUserName));
			} else {
				m_nxsLastRequestedLabel.SetWindowText(FormatString("Last requested on %s", FormatDateTimeForInterface(m_pCurrentMasterRequest->RequestedDate)));
			}

			// (r.gonet 08/08/2013) - PLID 57978 - Wait on the head request ids returned.
			ReloadRequestList(false);
			//r.wilson 8/20/2013 - Update the count for the med history list
			UpdateDisplayedMedicationCount();
			StartWaitingForRequestStatusUpdates();
		} else {
			// (r.gonet 09/20/2013) - PLID 57978 - The request failed somehow.
		}
	} NxCatchAllCall(__FUNCTION__,
	{
		StopWaitingForRequestStatusUpdates();
	});
}

// (r.gonet 08/05/2013) - PLID 58200 - Handler for clicking the last fill date range checkbox.
void CMedicationHistoryDlg::OnBnClickedMedHistoryFromDateCheck()
{
	try {
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/05/2013) - PLID 57979 - Handler for clicking the consent checkbox. Saves the consent value to the database.
void CMedicationHistoryDlg::OnBnClickedMedHistoryConsentCheck()
{
	try {
		// (r.gonet 09/20/2013) - PLID 57979 - Save the consent value.
		BOOL bConsentObtained = m_checkPatientConsentObtained.GetCheck() ? TRUE : FALSE;
		ExecuteParamSql("UPDATE PatientsT SET MedicationHistoryConsent = {BOOL} WHERE PersonID = {INT}", bConsentObtained, m_nPatientID); 
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

void CMedicationHistoryDlg::OnBnClickedMedHistoryViewEligibilityDetailsBtn()
{
	try {
		// (j.fouts 2013-09-19 11:56) - PLID 58701 - Open the detail dlg
		CSSEligibilityDlg dlg(this);
		dlg.DoModal(m_nPatientID);
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/05/2013) - PLID 58200 - Handler for when the location combo box finishes querying. Starts the load
// of the provider combo box.
void CMedicationHistoryDlg::OnRequeryFinishedLocationCombo(short nFlags)
{
	try {
		m_pLocationCombo->SetSelByColumn(emhlcID, GetCurrentLocationID());
		// (r.gonet 08/09/2013) - PLID 58200 - Provider combo is dependent on the location combo.
		LoadProviderCombo();
		// (r.gonet 08/09/2013) - PLID 58200 - Make sure the provider combo gets enabled or disabled as needed.
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/05/2013) - PLID 57981 - Handler for table checker messages.
LRESULT CMedicationHistoryDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		switch(wParam)
		{
			case NetUtils::MedicationHistoryResponseT:
			{					
				LoadResponseList();
				break;
			}
			// (r.gonet 08/09/2013) - PLID 57981 - Indicates that a sequence of requests
			// to systematically get all mediaction history for a particular coverage
			// or fill data has been completed. Note that each request's response
			// is limited to 50 meds, so additional requests in a sequence are made.
			// This is fired when there are no more meds in that sequence.
			case NetUtils::MedHistory_SequenceCompleted:
			{
				if(m_pCurrentMasterRequest == NULL) {
					// We haven't requested anything yet!
					break;
				}
				// (r.gonet 08/09/2013) - PLID 57981 - We got one back. Pause the timer.
				KillTimer(ID_MED_HISTORY_REQUEST_TIMEOUT);
				KillTimer(ID_MED_HISTORY_REQUEST_STATUS_CHECK);

				// (r.gonet 08/09/2013) - PLID 57981 - One of the request sequences has completed. Was it one of the ones we were waiting on?
				long nHeadRequestID = (long)lParam;
				bool bFound = false;
				Nx::SafeArray<IUnknown *> saryRequests(m_pCurrentMasterRequest->RxHistoryRequests);
				foreach(NexTech_Accessor::_RxHistoryRequestPtr pRxHistoryRequest, saryRequests)
				{
					// (r.gonet 12/16/2013) - PLID 58199 - Changed public API properites from integer to string.
					if(atol((LPCTSTR)pRxHistoryRequest->HeadRequestID) == nHeadRequestID) {
						bFound = true;
						break;
					}
				}
				if(bFound) {
					if(AreAllRequestsComplete()) {
						StopWaitingForRequestStatusUpdates();
					} else {
						// (r.gonet 08/09/2013) - PLID 57981
						ReloadRequestList();
						// (r.gonet 08/09/2013) - PLID 57981 - It was one that we requested, so it was now completed.
						if(AreAllRequestsComplete()) {
							StopWaitingForRequestStatusUpdates();
						} else {
							// (r.gonet 08/09/2013) - PLID 57981 - Still more to get. Resume the timer.
							SetTimer(ID_MED_HISTORY_REQUEST_TIMEOUT, 32 * 1000, NULL);
							SetTimer(ID_MED_HISTORY_REQUEST_STATUS_CHECK, 5 * 1000, NULL);
						}
					}
				} else {
					// (r.gonet 08/09/2013) - PLID 57981 - We didn't make that request, so ignore it.
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
	return 0;
}

// (r.gonet 08/09/2013) - PLID 57981 - Returns true if all of the requests in the current request session have either succeeded or failed.
bool CMedicationHistoryDlg::AreAllRequestsComplete()
{
	// (r.gonet 09/20/2013) - PLID 57981 - Check the entire list of requests for any that aren't complete, ie error or success
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMedicationHistoryRequestList->GetFirstRow();
	while(pRow) {
		PrescriptionQueueStatus status = (PrescriptionQueueStatus)VarLong(pRow->GetValue(erlcStatusID));
		if(status != pqseTransmitSuccess && status != pqseTransmitError) {
			// This request is not done yet!
			return false;
		}
		pRow = pRow->GetNextRow();
	}
	return true;
}

// (r.gonet 08/09/2013) - PLID 57981 - Syncs the request list with the request statuses in the database.
void CMedicationHistoryDlg::ReloadRequestList(bool bRequery/*=true*/)
{
	if(m_pCurrentMasterRequest == NULL) {
		// (r.gonet 09/20/2013) - PLID 57981 - We need a request in order to update the request list.
		return;
	}

	if(bRequery) {
		// (r.gonet 08/12/2013) - PLID 57981 - Go get the master request, which will contain any new rxhistory requests made under the master request.
		NexTech_Accessor::_NullableRxHistoryMasterRequestPtr pNullableMasterRequest = GetAPI()->GetRxHistoryMasterRequest(GetAPISubkey(), GetAPILoginToken(), m_pCurrentMasterRequest->ID);
		if (pNullableMasterRequest != NULL && pNullableMasterRequest->value != NULL) {
			m_pMedicationHistoryRequestList->Clear();
			m_pCurrentMasterRequest = pNullableMasterRequest->value;
		} else {
			// (r.gonet 12/16/2013) - PLID 58199 - Changed public API properites from integer to string.
			long nID = atol((LPCTSTR)m_pCurrentMasterRequest->ID);
			ThrowNxException("%s : Could not retrieve master request with id = %li", __FUNCTION__, nID);
		}
	} else {
		// (r.gonet 08/12/2013) - PLID 57981 - Just use the current master request's values. This means that the caller thinks it is up to date.
	}
	
	// (r.gonet 08/12/2013) - PLID 57981 - Reinitialize the requests in the requests list.
	Nx::SafeArray<IUnknown*> saryRequests(m_pCurrentMasterRequest->RxHistoryRequests);
	foreach(NexTech_Accessor::_RxHistoryRequestPtr pRxHistoryRequest, saryRequests) {
		if(pRxHistoryRequest->ID != pRxHistoryRequest->HeadRequestID) {
			// (r.gonet 08/12/2013) - PLID 57981 - This isn't the first request in the sequence
			continue;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRequestRow = m_pMedicationHistoryRequestList->GetNewRow();
		pRequestRow->PutValue(erlcID, _variant_t(atol((LPCTSTR)pRxHistoryRequest->ID), VT_I4));

		long nEligibilityResponseDetailID = -1;
		// (r.gonet 12/16/2013) - PLID 58199 - Changed public API properties from integer to string.
		CString strEligibilityResponseDetailID((LPCTSTR)pRxHistoryRequest->EligibilityResponseDetailID);
		if(strEligibilityResponseDetailID != "") {
			nEligibilityResponseDetailID = atol(strEligibilityResponseDetailID);
			CString strPBMName((LPTSTR)pRxHistoryRequest->PBMName), strPlanName((LPTSTR)pRxHistoryRequest->PlanName);
			CString strSourceName = strPBMName;
			if(strPlanName != "") {
				strSourceName += " - " + strPlanName;
			}
			pRequestRow->PutValue(erlcSource, _bstr_t(strSourceName));
		} else {
			pRequestRow->PutValue(erlcSource, _bstr_t("SureScripts"));
		}
		if(!m_pCurrentMasterRequest->fromDate->IsNull()) {
			COleDateTime dtFrom = m_pCurrentMasterRequest->fromDate->GetValue();
			pRequestRow->PutValue(erlcFromDate, _variant_t(dtFrom, VT_DATE));
		}
		
		if(!m_pCurrentMasterRequest->toDate->IsNull()) {
			COleDateTime dtTo = m_pCurrentMasterRequest->toDate->GetValue();
			pRequestRow->PutValue(erlcToDate, _variant_t(dtTo, VT_DATE));
		}
		pRequestRow->PutValue(erlcStatusID, g_cvarNull);
		pRequestRow->PutValue(erlcStatusText, _bstr_t(""));
		pRequestRow->PutValue(erlcFullErrorMessage, _bstr_t(""));
		m_pMedicationHistoryRequestList->AddRowAtEnd(pRequestRow, NULL);
	}

	// (r.gonet 08/12/2013) - PLID 57981 - Go through all of the individual requests and update the request status list
	foreach(NexTech_Accessor::_RxHistoryRequestPtr pRxHistoryRequest, saryRequests){
		// (r.gonet 12/16/2013) - PLID 58199 - Changed public API properties from integer to string.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMedicationHistoryRequestList->FindByColumn(erlcID, _variant_t(atol((LPCTSTR)pRxHistoryRequest->HeadRequestID), VT_I4), NULL, VARIANT_FALSE);
		if(!pRow) {
			ThrowNxException("%s : Could not find row for request ID.", __FUNCTION__);
		}
		
		PrescriptionQueueStatus status = MapAccessorToQueueStatus(pRxHistoryRequest->status);
		PrescriptionQueueStatus overallStatus = (PrescriptionQueueStatus)VarLong(pRow->GetValue(erlcStatusID), pqseTransmitSuccess);

		if(status == pqseTransmitError) {
			// (r.gonet 08/09/2013) - PLID 57981 - Errors take first priority. If there is an error in
			// one of the requests in the request sequence, that ends the request sequence.
			pRow->PutValue(erlcStatusID, _variant_t((long)status, VT_I4));
			CString strShortErrorMessage((LPCTSTR)(pRxHistoryRequest->shortErrorMessage));
			if(strShortErrorMessage == "") {
				strShortErrorMessage = "Failure - Click for Details";
			}
			pRow->PutValue(erlcStatusText, _bstr_t(strShortErrorMessage));
			CString strDisplayMessage((LPCTSTR)(pRxHistoryRequest->fullErrorMessage));
			pRow->PutValue(erlcFullErrorMessage, _bstr_t(strDisplayMessage));

			pRow->PutBackColor(ROW_NEEDS_ATTENTION_COLOR);
			NXDATALIST2Lib::IFormatSettingsPtr pFormatSettings(__uuidof(NXDATALIST2Lib::FormatSettings));
			pFormatSettings->FieldType = NXDATALIST2Lib::cftTextWordWrapLink;
			pRow->CellFormatOverride[erlcStatusText] = pFormatSettings;
		} else if((status == pqseTransmitPending || status == pqseTransmitAuthorized) && overallStatus != pqseTransmitError) {
			// (r.gonet 08/09/2013) - PLID 57981 - This request sequence is not done yet. At least one request is outstanding.
			pRow->PutValue(erlcStatusID, _variant_t((long)status, VT_I4));
			pRow->PutValue(erlcStatusText, _bstr_t("Requesting..."));
			pRow->PutValue(erlcFullErrorMessage, _bstr_t(""));

			pRow->PutBackColor(ROW_REQUESTING_COLOR);
		} else if((status == pqseTransmitSuccess) && overallStatus != pqseTransmitError && overallStatus != pqseTransmitPending && overallStatus != pqseTransmitAuthorized) {
			// (r.gonet 08/09/2013) - PLID 57981 - This request sequence is not done yet. At least one request is outstanding.
			pRow->PutValue(erlcStatusID, _variant_t((long)status, VT_I4));
			pRow->PutValue(erlcStatusText, _bstr_t("Success"));
			pRow->PutValue(erlcFullErrorMessage, _bstr_t(""));

			pRow->PutBackColor(RGB(255,255,255));
		}
	}
}

// (r.gonet 08/05/2013) - PLID 58200 - Handler for when the user selects a row in the location combo but before the selection is committed.
void CMedicationHistoryDlg::SelChangingMedHistoryLocationCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		// Don't allow the user to select nothing
		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/05/2013) - PLID 58200 - Handler for when the user selects a row in the provider combo but before the selection is committed.
void CMedicationHistoryDlg::SelChangingMedHistoryReqProviderCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		// Don't allow the user to select nothing
		if (*lppNewSel == NULL) {			
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/20/2013) - PLID 57981 - Populates the the request list with the patient's most recent request
// and the response list with the medication history for that recent request.
void CMedicationHistoryDlg::LoadLastMasterRequest()
{
	StopWaitingForRequestStatusUpdates();
	// (r.gonet 09/20/2013) - PLID 57981 - No request means no responses, so try to get the request.
	m_pCurrentMasterRequest = MedicationHistoryUtils::GetLastRxHistoryMasterRequest(m_nPatientID);
	
	if(m_pCurrentMasterRequest != NULL) {
		// (r.gonet 09/20/2013) - PLID 57981 - Fill the last-requested-by label with the user who made the request.
		// (r.gonet 12/16/2013) - PLID 58199 - Changed public API properties from integer to string.
		CString strRequestedByUserID((LPCTSTR)m_pCurrentMasterRequest->RequestedByUserID);
		if(strRequestedByUserID != "") {
			long nRequestedByUserID = atol(strRequestedByUserID);
			_RecordsetPtr prsUser = CreateParamRecordset("SELECT Username FROM UsersT WHERE PersonID = {int}", nRequestedByUserID);
			CString strUserName = AdoFldString(prsUser, "Username");
			m_nxsLastRequestedLabel.SetWindowText(FormatString("Last requested on %s by %s", FormatDateTimeForInterface(m_pCurrentMasterRequest->RequestedDate), strUserName));
		} else {
			m_nxsLastRequestedLabel.SetWindowText(FormatString("Last requested on %s", FormatDateTimeForInterface(m_pCurrentMasterRequest->RequestedDate)));
		}

		// (r.gonet 09/20/2013) - PLID 57981 - Populate the request list.
		ReloadRequestList(false);
		if(!AreAllRequestsComplete()) {
			// (r.gonet 09/20/2013) - PLID 57981 - If the most recent request is still being requested,
			// (like the background thread started to pre-fetch the results and then they clicked on 
			// the med history button) start waiting on it.
			StartWaitingForRequestStatusUpdates();
		}
	} else {
		// (r.gonet 09/20/2013) - PLID 57981 - No prior request.
		m_nxsLastRequestedLabel.SetWindowText("");
	}
}

 
void CMedicationHistoryDlg::LoadResponseList()
{
	if(m_pCurrentMasterRequest == NULL) {
		// (r.gonet 09/20/2013) - PLID 57981 - No current request, no responses. Go get one.
		LoadLastMasterRequest();
		if(m_pCurrentMasterRequest == NULL) {
			// (r.gonet 09/20/2013) - PLID 57981 - Nope, they don't have any requests.
			return;
		}
	}

	// (r.gonet 12/20/2013) - PLID 57957 - Changed public API master request ID argument from int to string.
	// Get the responses associated with the most recent request.
	NexTech_Accessor::_PatientRxHistoryArrayPtr pRxHistoryResults = GetAPI()->GetPatientRxHistory(GetAPISubkey(), GetAPILoginToken(), bstr_t(GetExistingPatientUserDefinedID(m_nPatientID)), m_pCurrentMasterRequest->ID);
	// Don't keep repeating the history each time the response table is updated.
	
	PopulateMedicationHistoryList(pRxHistoryResults);
	RemoveDuplicateMedications();
	UpdateDisplayedMedicationCount();
}

void CMedicationHistoryDlg::PopulateMedicationHistoryList(NexTech_Accessor::_PatientRxHistoryArrayPtr pRxHistoryResults)
{	
	Nx::SafeArray<IUnknown *> saryRxHistoryResults = pRxHistoryResults->PatientRxHistories;
	long nGeneratedKey = 1;	
	
	m_pMedicationHistoryList->Clear();
	m_mapMedHistoryInfoRows.clear();

	foreach(NexTech_Accessor::_PatientRxHistoryPtr pRxHistory, saryRxHistoryResults){
		NexTech_Accessor::_RxHistoryRequestPtr pRxHistoryRequest = pRxHistory->RxHistoryRequest;
		// (r.gonet 12/16/2013) - PLID 58199 - Changed public API properties from integer to string.
		CString strEligibilityResponseDetailID((LPCTSTR)pRxHistoryRequest->EligibilityResponseDetailID);
		long nEligibilityResponseDetailID;
		if(strEligibilityResponseDetailID != "") {
			nEligibilityResponseDetailID = atol(strEligibilityResponseDetailID);
		} else {
			nEligibilityResponseDetailID = -1;
		}
		
		
		if(pRxHistoryRequest == NULL) {
			ThrowNxException("%s : No request found with response!", __FUNCTION__);
		}
		// (r.gonet 12/16/2013) - PLID 58199 - Changed public API properties from integer to string.
		if(atol((LPCTSTR)pRxHistoryRequest->rxHistoryMasterRequestID) != atol((LPCTSTR)m_pCurrentMasterRequest->ID)) {
			// Not what we are waiting on. Ignore it.
			continue;
		}

		NexTech_Accessor::_ERxPrescriberInfoPtr pPrescriber = pRxHistory->GetPrescriberInfo();									

		Nx::SafeArray<IUnknown *> saryDispensedMedInfo = pRxHistory->GetDispensedMedicationInfos();	
		// (r.gonet 12/16/2013) - PLID 58199 - Changed public API properties from integer to string.
		long nMasterRequestID = atol((LPCTSTR)pRxHistoryRequest->rxHistoryMasterRequestID);
										
		BOOL bCheckIfPrevIsDuplicate = TRUE;
		foreach(NexTech_Accessor::_ERxMedicationInfoPtr pMedInfo, saryDispensedMedInfo){				
			AddRowToMedHistoryDatalist(pRxHistory, pMedInfo, emitDespensed, nGeneratedKey, nEligibilityResponseDetailID ,bCheckIfPrevIsDuplicate);
			bCheckIfPrevIsDuplicate = FALSE;
		}

		Nx::SafeArray<IUnknown *> saryPrescribedMedInfo = pRxHistory->GetPrescribedMedicationInfos();
			
		bCheckIfPrevIsDuplicate = TRUE;
		// Get Prescribed Medications 			
		foreach(NexTech_Accessor::_ERxMedicationInfoPtr pMedInfoPrescribed, saryPrescribedMedInfo){	
			AddRowToMedHistoryDatalist(pRxHistory, pMedInfoPrescribed, emitPrescribed , nGeneratedKey, nEligibilityResponseDetailID, bCheckIfPrevIsDuplicate);
			bCheckIfPrevIsDuplicate = FALSE;
		}					
	}
}

void CMedicationHistoryDlg::AddRowToMedHistoryDatalist(NexTech_Accessor::_PatientRxHistoryPtr pRxHistory, /*NexTech_Accessor::_ERxPrescriberInfoPtr pPrescriberInfo ,*/NexTech_Accessor::_ERxMedicationInfoPtr pMedInfo, EERxMedicationInfoType eMedInfoType, long &nGeneratedKey, long nEligibilityResponseDetailID, BOOL bCheckIfPrevIsDuplicate)
{
	EERxMedicationInfoType eMedicationInfoType = eMedInfoType;
	CString strPrescriberFullName;

	COleDateTime dtLastFillDate(pMedInfo->LastFillDate);
	CString strMedDescription(AsString(pMedInfo->Getdescription()));
	CString strRefills(AsString(pMedInfo->GetRefills()));				
	NexTech_Accessor::_ERxPrescriberInfoPtr  pPrescriberInfo = pMedInfo->GetPrescriberInfo();
	NexTech_Accessor::_ERxHistorySourcePtr pHistorySourceInfo =  pMedInfo->GetHistorySourceInfo();
	NexTech_Accessor::_RxHistoryRequestPtr pRxHistoryRequest = pRxHistory->RxHistoryRequest;
	CString strSourceDescription;

	// (r.gonet 12/16/2013) - PLID 58199 - Changed public API properites from integer to string.
	CString strEligibilityResponseDetailID((LPCTSTR)pRxHistoryRequest->EligibilityResponseDetailID);
	if(strEligibilityResponseDetailID != "") {
		CString strPBMName((LPTSTR)pRxHistoryRequest->PBMName), strPlanName((LPTSTR)pRxHistoryRequest->PlanName);
		strSourceDescription = strPBMName;
		if(strPlanName != "") {
			strSourceDescription += " - " + strPlanName;
		}
	} else {
		strSourceDescription = "SureScripts";
	}
										
	NexTech_Accessor::_ERxPharmacyInfoPtr pPharmacyInfo = pMedInfo->GetPharmacyInfo();
					
	//r.wilson -  Need to check and make sure that pPrescriberInfo is not NULL .. Later
	strPrescriberFullName = FormatString("%s, %s %s", FormatName(pPrescriberInfo->GetLastName()), 
	FormatName(pPrescriberInfo->GetFirstName()), FormatName(pPrescriberInfo->GetMiddleName()) );	
	CString strPharmacy = AsString(pPharmacyInfo->GetName());

	IRowSettingsPtr pNewRow = m_pMedicationHistoryList->GetNewRow();

	// generated key needs to go here
	pNewRow->PutValue(MedHistoryList::GeneratedKey, nGeneratedKey);
	// (a.wilson 2013-08-19 10:06) - PLID 57844 - new column to import meds from history into patient's current meds.
	pNewRow->PutValue(MedHistoryList::Import, g_cvarFalse);
	pNewRow->PutValue(MedHistoryList::PrescriberName, _bstr_t(strPrescriberFullName));
	pNewRow->PutValue(MedHistoryList::RxLastFillDate, _variant_t(dtLastFillDate, VT_DATE));
	pNewRow->PutValue(MedHistoryList::RxDescription, _bstr_t(strMedDescription));
	pNewRow->PutValue(MedHistoryList::Pharmacy, _bstr_t(strPharmacy));
	pNewRow->PutValue(MedHistoryList::RxSourceDescription , _bstr_t(strSourceDescription));

	m_pMedicationHistoryList->AddRowAtEnd(pNewRow, NULL);

	shared_ptr<MedHistoryRowInfo> sp_MedicationRowInfo(new MedHistoryRowInfo(pRxHistory,pMedInfo, nEligibilityResponseDetailID ,eMedicationInfoType));
	std::map<long, shared_ptr<MedHistoryRowInfo>>::iterator it = m_mapMedHistoryInfoRows.find(nGeneratedKey);
	if(it != m_mapMedHistoryInfoRows.end()) {
		m_mapMedHistoryInfoRows.erase(it);
	}
	m_mapMedHistoryInfoRows.insert(std::pair<long, shared_ptr<MedHistoryRowInfo>>(nGeneratedKey, sp_MedicationRowInfo));
	nGeneratedKey++;

}
BOOL CMedicationHistoryDlg::MedsEqual(shared_ptr<MedHistoryRowInfo> pMedHistoryRowInfo1, shared_ptr<MedHistoryRowInfo> pMedHistoryRowInfo2)
{
	if(pMedHistoryRowInfo1->m_nEligibilityResponseDetailID != pMedHistoryRowInfo2->m_nEligibilityResponseDetailID){
		return FALSE;
	}
	NexTech_Accessor::_ERxMedicationInfoPtr pMedInfo1 = pMedHistoryRowInfo1->pMedicationInfo;
	NexTech_Accessor::_ERxMedicationInfoPtr pMedInfo2 = pMedHistoryRowInfo2->pMedicationInfo;

	if(pMedInfo1->ID != pMedInfo2->ID) { return FALSE; }
	if(AsString(pMedInfo1->GetNDC()) != AsString(pMedInfo2->GetNDC())) { return FALSE; }
	if(_bstr_t(pMedInfo1->Getdescription()) != _bstr_t(pMedInfo2->Getdescription()) ) {return FALSE;}	
	if(pMedInfo1->DaysSupply != pMedInfo2->DaysSupply) { return FALSE; }
	if(pMedInfo1->Substitution != pMedInfo2->Substitution) { return FALSE; }
	if(pMedInfo1->Refills != pMedInfo2->Refills) { return FALSE; }
	if(pMedInfo1->RefillQualifier != pMedInfo2->RefillQualifier) { return FALSE; }
	if(pMedInfo1->Quantity != pMedInfo2->Quantity) { return FALSE; }
	if(pMedInfo1->QuantityUnits != pMedInfo2->QuantityUnits) { return FALSE; }
	if(pMedInfo1->Strength != pMedInfo2->Strength) { return FALSE; }
	if(pMedInfo1->StrengthUnits != pMedInfo2->StrengthUnits) { return FALSE; }
	if(pMedInfo1->DosageForm != pMedInfo2->DosageForm) { return FALSE; }	
	if(pMedInfo1->WrittenDate != pMedInfo2->WrittenDate) { return FALSE; }
	if(COleDateTime(pMedInfo1->GetLastFillDate()) != COleDateTime(pMedInfo2->GetLastFillDate())) {return FALSE;}
	if(pMedInfo1->Directions != pMedInfo2->Directions) { return FALSE; }
	if(pMedInfo1->notes != pMedInfo2->notes) { return FALSE; }
	if(pMedInfo1->DEASchedule != pMedInfo2->DEASchedule) { return FALSE; }
	if(pMedInfo1->IsSupply != pMedInfo2->IsSupply) { return FALSE; }
	if(pMedInfo1->IsCompoundDrug != pMedInfo2->IsCompoundDrug) { return FALSE; }
	if(pMedInfo1->PriorAuthorization != pMedInfo2->PriorAuthorization) { return FALSE; }
	if(pMedInfo1->PharmacyInfo != NULL && pMedInfo2->PharmacyInfo != NULL) {
		if(pMedInfo1->PharmacyInfo->ID != pMedInfo2->PharmacyInfo->ID) { return FALSE; }
		if(pMedInfo1->PharmacyInfo->NCPDPID != pMedInfo2->PharmacyInfo->NCPDPID) { return FALSE; }
		if(pMedInfo1->PharmacyInfo->Name != pMedInfo2->PharmacyInfo->Name) { return FALSE; }
		if(pMedInfo1->PharmacyInfo->Address1 != pMedInfo2->PharmacyInfo->Address1) { return FALSE; }
		if(pMedInfo1->PharmacyInfo->Address2 != pMedInfo2->PharmacyInfo->Address2) { return FALSE; }
		if(pMedInfo1->PharmacyInfo->City != pMedInfo2->PharmacyInfo->City) { return FALSE; }
		if(pMedInfo1->PharmacyInfo->State != pMedInfo2->PharmacyInfo->State) { return FALSE; }
		if(pMedInfo1->PharmacyInfo->Zip != pMedInfo2->PharmacyInfo->Zip) { return FALSE; }
		if(pMedInfo1->PharmacyInfo->Specialty != pMedInfo2->PharmacyInfo->Specialty) { return FALSE; }
		Nx::SafeArray<IUnknown*> saryPhoneNumbers1(pMedInfo1->PharmacyInfo->PhoneNumbers);
		Nx::SafeArray<IUnknown*> saryPhoneNumbers2(pMedInfo2->PharmacyInfo->PhoneNumbers);
		if(saryPhoneNumbers1.GetSize() != saryPhoneNumbers2.GetSize()) { return FALSE; }
		for(long i = 0; i < (long)saryPhoneNumbers1.GetSize(); i++) {
			NexTech_Accessor::_ERxPhoneNumbersPtr pPhoneNumber1 = saryPhoneNumbers1.GetAt(i);
			NexTech_Accessor::_ERxPhoneNumbersPtr pPhoneNumber2 = saryPhoneNumbers2.GetAt(i);
			if(pPhoneNumber1->Number != pPhoneNumber2->Number) { return FALSE; }
			if(pPhoneNumber1->PhoneType != pPhoneNumber2->PhoneType) { return FALSE; }
		}
	}
	if(pMedInfo1->PrescriberInfo != NULL && pMedInfo2->PrescriberInfo != NULL) {
		if(pMedInfo1->PrescriberInfo->ID != pMedInfo2->PrescriberInfo->ID) { return FALSE; }
		if(pMedInfo1->PrescriberInfo->FirstName != pMedInfo2->PrescriberInfo->FirstName) { return FALSE; }
		if(pMedInfo1->PrescriberInfo->MiddleName != pMedInfo2->PrescriberInfo->MiddleName) { return FALSE; }
		if(pMedInfo1->PrescriberInfo->LastName != pMedInfo2->PrescriberInfo->LastName) { return FALSE; }
		if(pMedInfo1->PrescriberInfo->Address1 != pMedInfo2->PrescriberInfo->Address1) { return FALSE; }
		if(pMedInfo1->PrescriberInfo->Address2 != pMedInfo2->PrescriberInfo->Address2) { return FALSE; }
		if(pMedInfo1->PrescriberInfo->City != pMedInfo2->PrescriberInfo->City) { return FALSE; }
		if(pMedInfo1->PrescriberInfo->State != pMedInfo2->PrescriberInfo->State) { return FALSE; }
		if(pMedInfo1->PrescriberInfo->Zip != pMedInfo2->PrescriberInfo->Zip) { return FALSE; }
		Nx::SafeArray<IUnknown*> saryPhoneNumbers1(pMedInfo1->PrescriberInfo->PhoneNumbers);
		Nx::SafeArray<IUnknown*> saryPhoneNumbers2(pMedInfo2->PrescriberInfo->PhoneNumbers);
		if(saryPhoneNumbers1.GetSize() != saryPhoneNumbers2.GetSize()) { return FALSE; }
		for(long i = 0; i < (long)saryPhoneNumbers1.GetSize(); i++) {
			NexTech_Accessor::_ERxPhoneNumbersPtr pPhoneNumber1 = saryPhoneNumbers1.GetAt(i);
			NexTech_Accessor::_ERxPhoneNumbersPtr pPhoneNumber2 = saryPhoneNumbers2.GetAt(i);
			if(pPhoneNumber1->Number != pPhoneNumber2->Number) { return FALSE; }
			if(pPhoneNumber1->PhoneType != pPhoneNumber2->PhoneType) { return FALSE; }
		}
	}
	if(pMedInfo1->HistorySourceInfo != NULL && pMedInfo2->HistorySourceInfo != NULL) {
		if(pMedInfo1->HistorySourceInfo->FillNumber != pMedInfo2->HistorySourceInfo->FillNumber) { return FALSE; }
		if(pMedInfo1->HistorySourceInfo->SourceReference_IDQualifier != pMedInfo2->HistorySourceInfo->SourceReference_IDQualifier) { return FALSE; }
		if(pMedInfo1->HistorySourceInfo->SourceReference_IDValue != pMedInfo2->HistorySourceInfo->SourceReference_IDValue) { return FALSE; }
		if(pMedInfo1->HistorySourceInfo->SourceDescription != pMedInfo2->HistorySourceInfo->SourceDescription) { return FALSE; }
		if(pMedInfo1->HistorySourceInfo->SourceQualifier != pMedInfo2->HistorySourceInfo->SourceQualifier) { return FALSE; }
		if(pMedInfo1->HistorySourceInfo->SourceReference != pMedInfo2->HistorySourceInfo->SourceReference) { return FALSE; }
	}

	return TRUE;
}

// (r.gonet 08/09/2013) - PLID 57981 - Handle timeouts.
void CMedicationHistoryDlg::OnTimer(UINT nIDEvent) 
{
	try {

		switch(nIDEvent) {
		case ID_MED_HISTORY_REQUEST_TIMEOUT:
			{
				// (r.gonet 08/09/2013) - PLID 57981 - Request session timed out. Something is wrong.
				StopWaitingForRequestStatusUpdates();
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMedicationHistoryRequestList->GetFirstRow();
				while(pRow) {
					PrescriptionQueueStatus status = (PrescriptionQueueStatus)VarLong(pRow->GetValue(erlcStatusID));
					if(status != pqseTransmitSuccess && status != pqseTransmitError) {
						pRow->PutValue(erlcStatusID, _variant_t((long)pqseTransmitError, VT_I4));
						pRow->PutValue(erlcStatusText, _bstr_t("Timeout Occurred"));
						pRow->PutValue(erlcFullErrorMessage, _bstr_t("Timeout occurred while requesting medication history. No response was received."));

						pRow->PutBackColor(ROW_NEEDS_ATTENTION_COLOR);
						NXDATALIST2Lib::IFormatSettingsPtr pFormatSettings(__uuidof(NXDATALIST2Lib::FormatSettings));
						pFormatSettings->FieldType = NXDATALIST2Lib::cftTextWordWrapLink;
						pRow->CellFormatOverride[erlcStatusText] = pFormatSettings;

						// (r.gonet 12/16/2013) - PLID 58199 - Changed public API arguments from integer to string.
						CString strRxHistoryRequestID = FormatString("%li", VarLong(pRow->GetValue(erlcID)));
						// (r.gonet 09/20/2013) - PLID 57981 - Save the timeout status to the database.
						GetAPI()->UpdateRxHistoryRequestStatus(GetAPISubkey(), GetAPILoginToken(), 
							_bstr_t(strRxHistoryRequestID), MapQueueStatusToAccessor(pqseTransmitError), _bstr_t("Timeout Occurred"), _bstr_t("Timeout Occurred"));
					}
					pRow = pRow->GetNextRow();
				}
				EnsureControls();				
			}
			break;
		case ID_MED_HISTORY_REQUEST_STATUS_CHECK:
			{
				// (r.gonet 08/09/2013) - PLID 57981 - We want to manually check every few seconds
				// what the request statuses are. I know we expect table checkers to trigger
				// these checks, but I don't trust them always getting here.
				KillTimer(ID_MED_HISTORY_REQUEST_STATUS_CHECK);

				// (r.gonet 09/20/2013) - PLID 57981 - Sync the request status list with the database.
				ReloadRequestList();
				if(!AreAllRequestsComplete()) {
					// (r.gonet 09/20/2013) - PLID 57981 - Check again on the status in a little bit.
					SetTimer(ID_MED_HISTORY_REQUEST_STATUS_CHECK, 5 * 1000, NULL);
				} else {
					// (r.gonet 09/20/2013) - PLID 57981 - The requests are all done.
					StopWaitingForRequestStatusUpdates();
				}
			}
			break;
		}

	}NxCatchAll("Error in CMedicationHistoryDlg::OnTimer");

	CNxDialog::OnTimer(nIDEvent);
}

// (r.gonet 08/09/2013) - PLID 57981 - Start waiting for request status updates.
void CMedicationHistoryDlg::StartWaitingForRequestStatusUpdates()
{
	// Timeout after 32 seconds
	SetTimer(ID_MED_HISTORY_REQUEST_TIMEOUT, 32 * 1000, NULL);
	// Check the status manually every 5 seconds
	SetTimer(ID_MED_HISTORY_REQUEST_STATUS_CHECK, 5 * 1000, NULL);
	// Disable the request button
	EnsureControls();
}

// (r.gonet 08/09/2013) - PLID 57981 - Stop waiting for request status updates
void CMedicationHistoryDlg::StopWaitingForRequestStatusUpdates()
{
	// (r.gonet 08/09/2013) - PLID 57981 - Don't timeout while we are stopping.
	KillTimer(ID_MED_HISTORY_REQUEST_TIMEOUT);
	// (r.gonet 08/09/2013) - PLID 57981 - Stop periodically checking
	KillTimer(ID_MED_HISTORY_REQUEST_STATUS_CHECK);

	// (r.gonet 08/09/2013) - PLID 57981 - In case we never received a table checker message for the responses, manually go and update the list.
	if(m_pCurrentMasterRequest != NULL) {
		LoadResponseList();
	}
	// (r.gonet 08/09/2013) - PLID 57981 - Make the request button enabled again.
	EnsureControls();
}

void CMedicationHistoryDlg::RemoveDuplicateMedications()
{	
	std::map<long, shared_ptr<MedHistoryRowInfo>>::iterator it, it2;
	std::map<long, shared_ptr<MedHistoryRowInfo>> mapUniqueRows;	

	for(it = m_mapMedHistoryInfoRows.begin(); it != m_mapMedHistoryInfoRows.end(); ++it)
	{
		BOOL bFoundDuplicate = FALSE;
		for(it2 = mapUniqueRows.begin(); it2 != mapUniqueRows.end(); ++it2)
		{			
			if(it->first != it2->first && MedsEqual(it->second, it2->second))
			{
				bFoundDuplicate = TRUE;
				break;
			}
		}
		if(!bFoundDuplicate){
			mapUniqueRows.insert(std::make_pair(it->first, it->second));
		}
	}

		swap(mapUniqueRows, m_mapMedHistoryInfoRows);	

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMedicationHistoryList->GetFirstRow();
	NXDATALIST2Lib::IRowSettingsPtr pRowNext;
	while(pRow)
	{
		long nGeneratedKey = VarLong(pRow->GetValue(MedHistoryList::GeneratedKey));
		std::map<long, shared_ptr<MedHistoryRowInfo>>::iterator itTmp = m_mapMedHistoryInfoRows.find(nGeneratedKey);
		if(itTmp == m_mapMedHistoryInfoRows.end())
		{
			pRowNext = pRow->GetNextRow();
			m_pMedicationHistoryList->RemoveRow(pRow);
			pRow = pRowNext;
		}
		else
		{
			pRow = pRow->GetNextRow();
		}

	}


}


//(r.wilson 8/6/2013) PLID 57947 
void CMedicationHistoryDlg::DblClickCellMedHistoryList(LPDISPATCH lpRow, short nColIndex)
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		ShowPopupForClickedRow(pRow);
	}NxCatchAll(__FUNCTION__)
}

void CMedicationHistoryDlg::ShowPopupForClickedRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	long nGeneratedKey = VarLong(pRow->GetValue(MedHistoryList::GeneratedKey));

	shared_ptr<MedHistoryRowInfo> sp_MedHistoryRowInfo = m_mapMedHistoryInfoRows[nGeneratedKey];
	
	NexTech_Accessor::_ERxMedicationInfoPtr pMedicationInfo = sp_MedHistoryRowInfo->pMedicationInfo;

	NexTech_Accessor::_ERxPrescriberInfoPtr pPrescriberInfo = pMedicationInfo->GetPrescriberInfo();
	CString strPrescriber = FormatString("%s, %s %s", FormatName(pPrescriberInfo->GetLastName()),FormatName(pPrescriberInfo->GetFirstName()), FormatName(pPrescriberInfo->GetMiddleName()));
	COleDateTime dtWrittenDate(pMedicationInfo->GetWrittenDate());
	CString strMedicationDescription = AsString(pMedicationInfo->Getdescription());
	CString strSourceDescription;
	NexTech_Accessor::_ERxHistorySourcePtr pHistoricalSource = pMedicationInfo->GetHistorySourceInfo();
	if(pHistoricalSource){
		strSourceDescription = AsString(pHistoricalSource->GetSourceDescription());
	}

	CString strMedHistroy = FormatString("Prescriber: %s \r\n Written Date: %s \r\n Medication Description: %s \r\n %s", strPrescriber, dtWrittenDate.Format("%d/%m/%Y"), strMedicationDescription, strSourceDescription);

	std::vector<shared_ptr<CMedicationHistoryDlg::MedHistoryRowInfo>> vDisplayMedications;
	if(CString((LPCTSTR)sp_MedHistoryRowInfo->pMedicationInfo->Getdescription()).Find("PHENYL") >= 0) {
		int a = 0;
	}
	vDisplayMedications.insert(vDisplayMedications.end(), sp_MedHistoryRowInfo);

	MedicationHistory::PopupInfo(this, vDisplayMedications);	
}

CString CMedicationHistoryDlg::FormatName(const _variant_t& varValue)
{

	CString strName = AsString(varValue);
	strName.Trim();
	strName.MakeLower();
	if(strName.IsEmpty()){
		return "";
	}

	char cNewFirstLetter =  toupper(strName[0]);
	strName.SetAt(0,cNewFirstLetter);

	return strName;

}

// (r.gonet 08/09/2013) - PLID 57981 - Handler for when the user clicks on a cell in the request list. 
// Display any error message when the user clicks on the status text column when it is an error.
void CMedicationHistoryDlg::LeftClickMedHistoryRequestsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol)
		{
			case erlcStatusText:
			{
				// (r.gonet 08/09/2013) - PLID 57981 - Popup the full error if there is one.
				PrescriptionQueueStatus statusID = (PrescriptionQueueStatus)VarLong(pRow->GetValue(erlcStatusID));
				if(statusID == pqseTransmitError) {
					CString strDisplayMessage = VarString(pRow->GetValue(erlcFullErrorMessage), "");
					if(strDisplayMessage == "") {
						strDisplayMessage = "Unknown error occurred.";
					}
					MessageBox(strDisplayMessage, "Status Message", MB_ICONERROR|MB_OK);
				}
			}
			break;
			
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/09/2013) - PLID 58200 - Handler for when the user selects a row from the location combo. 
// Load the provider combo when we make a selection in the location combo.
void CMedicationHistoryDlg::SelChosenLocationCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(!pRow) {
			// (r.gonet 08/09/2013) - PLID 58200 - They chose nothing somehow. Ensure the provider combo anyway.
			EnsureControls();
			return;
		}
		// (r.gonet 08/09/2013) - PLID 58200 - Provider combo is dependent on the location combo.
		LoadProviderCombo();
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-08-19 10:08) - PLID 57844 -  when clicked will add all checked meds to current med list.
void CMedicationHistoryDlg::OnBnClickedImportMedicationBtn()
{
	try {
		//check permissions and licensing before continuing.

		//check to ensure they have permission to add current meds.
		if(!CheckCurrentUserPermissions(bioPatientCurrentMeds, sptCreate)) {
			return;
		}

		//check to ensure they have FirstDataBank
		if(g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) 
		{
			//Also check that the database exists
			if(!FirstDataBank::EnsureDatabase(NULL, true)) 
			{
				//If they have FDB License, but the database is not set up yet we can't really use the nexerx yet.
				return;
			}
		}

		//list of keys that point to the medication info in the map.
		CArray<long> arMedicationKeys;
		//collect any rows that are checked to import.
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMedicationHistoryList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			if (AsBool(pRow->GetValue(MedHistoryList::Import))) {
				arMedicationKeys.Add(pRow->GetValue(MedHistoryList::GeneratedKey));
			}
		}

		//check for no rows checked. if so then escape this function.
		if (arMedicationKeys.GetCount() == 0) {
			MessageBox("You must check at least one historical medication in the list to import.", 
				"No Historical Medications Checked", MB_OK|MB_ICONERROR);
			return;
		}

		//pull all the data needed to attempt to import meds and add them to the patient's current meds list.

		//collect data necessary for import into the drug list.
		CArray<NexTech_Accessor::_RxMedicationHistoryImportPtr> arImportMeds;
		//keep track of meds that failed to meet import requirements.
		CArray<NexTech_Accessor::_RxMedicationHistoryImportResultPtr> arMedResults;
		
		//collect the information we need to import these meds.
		for (int i = 0; i < arMedicationKeys.GetCount(); i++)
		{
			std::map<long, shared_ptr<MedHistoryRowInfo>>::iterator it;
			it = m_mapMedHistoryInfoRows.find(arMedicationKeys.GetAt(i));
			if (it != m_mapMedHistoryInfoRows.end())
			{
				NexTech_Accessor::_ERxMedicationInfoPtr p =	it->second->pMedicationInfo;

				//check to ensure NDC exists.
				if (AsString(p->GetNDC()).IsEmpty()) {
					NexTech_Accessor::_RxMedicationHistoryImportResultPtr pResult(__uuidof(NexTech_Accessor::RxMedicationHistoryImportResult));
					pResult->PutDrugName(p->Getdescription());
					//pResult->Putsig(p->GetDirections());
					pResult->PutSuccess(VARIANT_FALSE);
					pResult->PutReason(_bstr_t("Did not contain a NDC."));
					arMedResults.Add(pResult);
				}
				//If it has a NDC then add to import list.
				else {
					NexTech_Accessor::_RxMedicationHistoryImportPtr pImport(__uuidof(NexTech_Accessor::RxMedicationHistoryImport));
					pImport->PutNDC(p->GetNDC());
					pImport->PutDrugName(p->Getdescription());
					pImport->PutSig(p->GetDirections());
					pImport->PutLastFillDate(p->GetLastFillDate());
					arImportMeds.Add(pImport);
				}
			}
		}

		//error checking. This should never happen.
		if (arImportMeds.GetCount() == 0 && arMedResults.GetCount() == 0) {
			ASSERT(FALSE);
		}	

		//call api to get results of import into patient's current meds.
		{
			CArray<NexTech_Accessor::_RxMedicationHistoryImportResultPtr> arImportResults;
			Nx::SafeArray<IUnknown *> saryImport = Nx::SafeArray<IUnknown *>::From(arImportMeds);

			NexTech_Accessor::_RxMedicationHistoryImportResultArrayPtr pResults = GetAPI()->ImportPatientMedicationHistoryToCurrentMeds(GetAPISubkey(), GetAPILoginToken(), saryImport, 
			_bstr_t(GetExistingPatientUserDefinedID(m_nPatientID)));
			Nx::SafeArray<IUnknown *> sarImportResults(pResults->GetImportResults());
			sarImportResults.ToArray(arImportResults);
			//add to existing array of results.
			arMedResults.Append(arImportResults);
		}

		//render all results and display them appropriately.
		CString strFailedReasons;
		long nSuccessful = 0, nFailed = 0;

		foreach(NexTech_Accessor::_RxMedicationHistoryImportResultPtr pResult, arMedResults)
		{
			if (pResult->GetSuccess() == VARIANT_FALSE) {
				strFailedReasons += FormatString("%s - %s\r\n", AsString(pResult->GetDrugName()), AsString(pResult->GetReason()));
				nFailed++;
			} else {
				nSuccessful++;
			}
		}

		if (nSuccessful > 0) {
			m_bImportedHistoryMeds = true;
		}

		if (nFailed > 0) {
			m_strImportFailedPopupText = FormatString("The following historical medications could not be Imported:\r\n\r\n%s\r\n"
				"You can enter these historical medications manually on the Medications Tab if necessary.", strFailedReasons);
		}

		SetDlgItemText(IDC_HISTORY_MEDICATION_IMPORT_RESULTS, FormatString("Import Results:\r\n%sSucceeded: %li", 
			(nFailed > 0 ? FormatString("<a>Failed: %li</a>\r\n", nFailed) : ""), nSuccessful));

		//remove checkboxes from meds that were attempted using generated key array. this is only temporary while the datalist contains this information.
		for (int i = 0; i < arMedicationKeys.GetCount(); i++) {
			IRowSettingsPtr pRow = m_pMedicationHistoryList->FindByColumn(MedHistoryList::GeneratedKey, arMedicationKeys.GetAt(i), NULL, VARIANT_FALSE);
		
			if (pRow) {
				pRow->PutValue(MedHistoryList::Import, g_cvarNull);
			}
		}

	} NxCatchAll(__FUNCTION__);
}

// Gets the number medications inside the medications datalist and updates the static text to show the number
void CMedicationHistoryDlg::UpdateDisplayedMedicationCount()
{		
	SetDlgItemText(IDC_STATIC_TOTAL_MED_COUNT,  FormatString("Total Medications in List: %li", m_pMedicationHistoryList->GetRowCount()));
}

// (a.wilson 2013-08-19 10:08) - PLID 57844 - check off all rows to import into current meds.
void CMedicationHistoryDlg::OnBnClickedCheckAllMedicationsBtn()
{
	try {
		for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMedicationHistoryList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			_variant_t varImport = pRow->GetValue(MedHistoryList::Import);
			if(varImport.vt != VT_NULL) {
				pRow->PutValue(MedHistoryList::Import, g_cvarTrue);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.wilson 2013-08-30 14:36) - PLID 57844 - displays a list of medcations that did not import successfully.
void CMedicationHistoryDlg::OnNMClickHistoryMedicationImportResults(NMHDR *pNMHDR, LRESULT *pResult)
{
	try {
		CString strWindowText = ("Failed to Import Historical Medications:");
		if (m_strImportFailedPopupText.GetLength() > 2000) {
			CMsgBox dlg(this);
			dlg.m_strWindowText = strWindowText;
			dlg.msg = m_strImportFailedPopupText;
			dlg.DoModal();
		} else {
			MessageBox(m_strImportFailedPopupText, strWindowText, MB_OK|MB_ICONERROR);
		}
	} NxCatchAll(__FUNCTION__);

	*pResult = 0;
}

void CMedicationHistoryDlg::LeftClickMedHistoryList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	// TODO: Add your message handler code here
	try
	{
		if(nCol == MedHistoryList::RxDescription)
		{
			IRowSettingsPtr pRow(lpRow);
			ShowPopupForClickedRow(pRow);
		}

	}NxCatchAll(__FUNCTION__);
}

HBRUSH CMedicationHistoryDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	try {
		switch(pWnd->GetDlgCtrlID())
		{
			case IDC_HISTORY_MEDICATION_IMPORT_RESULTS:
			{
				// (r.gonet 12/20/2013) - PLID 57844 - Make the label transparent by making its background color the same color
				// as the dialog's background.
				pDC->SetBkColor(GetSolidBackgroundColor());
				return m_brBackground;
			}
			break;
		}
	} NxCatchAll(__FUNCTION__);

	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

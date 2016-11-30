// MedicationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "MedicationDlg.h"
#include "NxTabView.h"
#include "GlobalUtils.h"
#include "MainFrm.h"
#include "pracprops.h"
#include "NxStandard.h"
#include "GlobalDataUtils.h"
#include "RegUtils.h"
#include "MedSchedule.h"
#include "HistoryDlg.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "PatientView.h"
#include "TaskEditDlg.h"
#include "EmrUtils.h"
#include "PrescriptionEditDlg.h"
#include "MergeEngine.h"
#include "EditMedicationListDlg.h"
#include "DontShowDlg.h"
#include "PrescriptionTemplateSetupDlg.h"
#include "NewCropBrowserDlg.h"
#include "NewCropUtils.h"
#include "SureScriptsCommDlg.h"
#include "DecisionRuleUtils.h"
#include "foreach.h"
#include "NxAPIManager.h"
#include "NxDataUtilitiesLib\SafeArrayUtils.h"
#include "FirstDataBankUtils.h"
#include "NxModalParentDlg.h"
#include "PrescriptionQueueDlg.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here
#include "ReconcileMedicationsUtils.h"
#include "EditAllergyListDlg.h"
#include "PrescriptionUtilsAPI.h" // (b.savon 2013-06-19 17:12) - PLID 56880
#include "MedicationHistoryDlg.h" // (r.gonet 09/20/2013) - PLID 58416
#include "MedlinePlusUtils.h"
#include "ReconciliationDlg.h"
#include "SharedScheduleUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.
// (j.fouts 2012-11-14 11:42) - PLID 53439 - Moved writing, printing, interactions, and printer templates out of this dlg and into the queue so it can be reused in 
//	EMR and anywhere else we are going to integrate this in the future
// (j.fouts 2012-12-26 12:42) - PLID 54340 - Removed the NewCrop button and functionality it is now handled in PrescriptionQueueDlg

// (a.walling 2012-11-05 11:58) - PLID 53588 - Resolve conflict with mshtmcid.h

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define IDM_PREVIEW_SELECTED	41764
#define IDM_DELETE_MEDICATION	41765
#define IDM_ADD_SCHED		41766
#define IDM_EDIT_SCHED		41767
#define IDM_DEL_SCHED		41768
//#define IDM_SEND_TO_NEWCROP	41769	// (j.jones 2009-02-20 16:34) - PLID 33165 (temporarily removed)
#define IDM_SHOW_MONO		41770	// (j.fouts 2012-09-07 14:24) - PLID 51997 - Show Monograph command
#define IDM_DELETE_CURRENT_MED	41771 // (b.savon 2013-01-14 12:33) - PLID 54592 - Delete current med
#define IDM_DELETE_CURRENT_ALLERGY 41772 // (b.savon 2013-01-14 17:38) - PLID 54613 - Delete current allergy
#define IDM_DISCONTINUE_CURRENT_MED 41773 // (b.savon 2013-01-25 12:30) - PLID 54854 - Discontinue current med/allergy
#define IDM_DISCONTINUE_CURRENT_ALLERGY 41774
#define IDM_SHOW_LEAFLET	41775   // (j.luckoski 2012-10-04 16:56) - PLID 53042 - Show leaflet command

// (j.gruber 2009-05-19 10:21) - PLID 34291 - changed the colors
/*****IF YOU CHANGE THESE, ALSO CHANGE THEM IN EMNMoreInfoDlg.cpp as well as PrescriptionQueueDlg.cpp******/
#define EPRESCRIBE_COLOR_DISCONTINUED RGB(222, 225, 231)
#define EPRESCRIBE_COLOR_ACTIVE RGB(178, 251, 197)

enum AllergyListColumns {
	alcID = 0,
	alcEnteredDate = 1, // (c.haag 2009-12-22 13:01) - PLID 35766
	alcName = 2,
	aldNotes = 3,
	alcFromNewCrop = 4,
	alcIsDiscontinued = 5,
	alcRXCUI = 6, // (a.walling 2010-01-15 15:21) - PLID 36902 - RXCUI
	alcFDBImport = 7, // (b.savon 2012-09-26 17:28) - PLID 52874
	alcAllergyID = 8, // (j.luckoski 2012-11-16 14:39) - PLID 53806 - Stores the AllergyID, and not the PatientAllergyT ID.
	alcFDBOutOfDate = 9, //TES 5/9/2013 - PLID 56614
	alcLastUpdateDate = 10, // (s.dhole 2013-07-05 15:47) - PLID 56931
};

enum CurMedListColumns {
	cmlcID = 0,
	cmlcPatientID,
	cmlcMedName,
	cmlcInfoButton,		// (j.jones 2013-10-17 15:27) - PLID 58983 - added infobutton and medication ID
	cmlcMedicationID,
	cmlcEMRDataID,
	cmlcNewCropGUID,
	cmlcDiscontinued,
	cmlcSig,			// (j.jones 2011-05-02 14:55) - PLID 43450
	cmlcFDBID,			// (j.fouts 2012-08-30 16:34) - PLID 52090
	cmlcFDBOutOfDate,	//TES 5/9/2013 - PLID 56614
	cmlcStartDate,	// (s.dhole 2013-06-06 13:10) - PLID 56926
	cmlcLastUpdateDate,	// (s.dhole 2013-07-05 15:51) - PLID 56926
};




// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;

/////////////////////////////////////////////////////////////////////////////
// CMedicationDlg dialog

// (j.fouts 2012-11-14 11:42) - PLID 53439 - Added prescription queue initialization
// (j.fouts 2012-11-15 10:36) - PLID 53573 - Initialize the prescription queue as embeded
// (r.gonet 09/20/2013) - PLID 58396 - Added a table checker for MedicationHistoryResponseT
CMedicationDlg::CMedicationDlg(CWnd* pParent)
	: CPatientDialog(CMedicationDlg::IDD, pParent),
	m_CurrentMedsChecker(NetUtils::CurrentPatientMedsT),
	m_PatientAllergiesChecker(NetUtils::PatientAllergyT),
	m_MedicationHistoryResponseChecker(NetUtils::MedicationHistoryResponseT)
{
	//{{AFX_DATA_INIT(CMedicationDlg)

	// (a.walling 2010-10-14 15:09) - PLID 40978 - Dead code
	//m_boAllowUpdate = TRUE;
	m_tempMedSchedID = -1;
	bMedSchedPrintInfoLoaded = FALSE;
	m_nMedSchedDays = 0;
	m_bIsMedSchedPrinting = FALSE;
	m_bToggleEndMedSchedPrinting = FALSE;
	//}}AFX_DATA_INIT
	m_nDefTemplateRowIndex = -1;

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Patient_Information/Medications/write_and_print_a_prescription.htm";

	// (a.walling 2010-10-14 15:57) - PLID 40978
	m_id = -1;

	// (j.jones 2012-11-30 15:13) - PLID 53968 - changed this dialog to a pointer
	m_pPrescriptionQueueDlg = NULL;

	// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon
	m_hInfoButtonIcon = NULL;
}

// (j.jones 2012-11-30 15:16) - PLID 53968 - added destructor
CMedicationDlg::~CMedicationDlg()
{
	try {

		
		// (j.jones 2012-11-30 15:16) - PLID 53968 - destroy the queue dialog
		if (m_pPrescriptionQueueDlg) {
			m_pPrescriptionQueueDlg->DestroyWindow();
			delete m_pPrescriptionQueueDlg;
			m_pPrescriptionQueueDlg = NULL;
		}

		// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon
		if (m_hInfoButtonIcon) {
			DestroyIcon(m_hInfoButtonIcon);
		}
	}NxCatchAll(__FUNCTION__);
}

void CMedicationDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMedicationDlg)		
	DDX_Control(pDX, IDC_CHECK_REVIEWED_ALLERGIES, m_checkReviewedAllergies);
	DDX_Control(pDX, IDC_ALLERGYWARNING, m_btnShowAllergyWarning);
	DDX_Control(pDX, IDC_DELETE_MEDSCHED, m_btnDeleteMedSched);
	DDX_Control(pDX, IDC_ADD_MEDSCHED, m_btnAddMedSched);
	DDX_Control(pDX, IDC_MEDICATIONS_BKG, m_bkg);
	DDX_Control(pDX, IDC_MEDICATIONS_BKG3, m_bg3);
	DDX_Control(pDX, IDC_MEDICATIONS_BKG4, m_bkg4);
	DDX_Control(pDX, IDC_MED_HIDE_INACTIVE, m_chkHideInactive);
	DDX_Control(pDX, IDC_CHECK_HAS_NO_ALLERGIES, m_checkHasNoAllergies);
	DDX_Control(pDX, IDC_CHECK_HAS_NO_MEDS, m_checkHasNoMeds);
	DDX_Control(pDX, IDC_CHECK_INTERACTIONS, m_btnInteractions);
	DDX_Control(pDX, IDC_MEDICATION_HISTORY_BTN, m_btnMedicationHistory);
	DDX_Control(pDX, IDC_BTN_EDIT_MEDS, m_btnEditMeds);
	DDX_Control(pDX, IDC_CHECK_HIDE_DISCONTINUED_ALLERGIES, m_checkHideDiscontinuedAllergies);
	DDX_Control(pDX, IDC_MED_INCLUDE_FREE_TEXT_MEDS, m_checkIncludeFreeTextCurrentMeds);
	DDX_Control(pDX, IDC_MED_INCLUDE_FREE_TEXT_ALLERGIES, m_checkIncludeFreeTextAllergies);
	DDX_Control(pDX, IDC_MED_ABOUT_CURRENT_MEDICATION_COLORS, m_icoAboutCurrentMedicationColors); 
	DDX_Control(pDX, IDC_MED_ABOUT_CURRENT_ALLERGIES_COLORS, m_icoAboutCurrentAllergiesColors);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMedicationDlg, CNxDialog)
	//{{AFX_MSG_MAP(CMedicationDlg)
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_ADD_MEDSCHED, OnAddMedsched)
	ON_BN_CLICKED(IDC_DELETE_MEDSCHED, OnDeleteMedsched)
	ON_BN_CLICKED(IDC_ALLERGYWARNING, OnWarn)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_CHECK_REVIEWED_ALLERGIES, OnCheckReviewedAllergies)
	ON_MESSAGE(NXM_NEWCROP_BROWSER_DLG_CLOSED, OnNewCropBrowserClosed)
	//}}AFX_MSG_MAP	
	ON_BN_CLICKED(IDC_MED_HIDE_INACTIVE, OnBnClickedMedHideInactive)
	ON_BN_CLICKED(IDC_CHECK_HAS_NO_ALLERGIES, OnCheckHasNoAllergies)
	ON_BN_CLICKED(IDC_CHECK_HAS_NO_MEDS, OnCheckHasNoMeds)
	ON_BN_CLICKED(IDC_CHECK_INTERACTIONS, &CMedicationDlg::OnBnClickedInteractions)
	ON_BN_CLICKED(IDC_BTN_EDIT_MEDS, &CMedicationDlg::OnBnClickedBtnEditMeds)
	ON_BN_CLICKED(IDC_BTN_EDIT_ALLERGIES, &CMedicationDlg::OnBnClickedBtnEditAllergies)
	ON_BN_CLICKED(IDC_MEDICATION_HISTORY_BTN, &CMedicationDlg::OnBnClickedMedicationHistoryBtn)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CHECK_HIDE_DISCONTINUED_ALLERGIES, OnCheckHideDiscontinuedAllergies)
	ON_MESSAGE(NXM_BACKGROUND_MED_HISTORY_REQUEST_COMPLETE, &CMedicationDlg::OnBackgroundMedHistoryRequestComplete)
	ON_BN_CLICKED(IDC_MED_INCLUDE_FREE_TEXT_MEDS, OnMedIncludeFreeTextMeds)
	ON_BN_CLICKED(IDC_MED_INCLUDE_FREE_TEXT_ALLERGIES, OnMedIncludeFreeTextAllergies)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMedicationDlg message handlers

BEGIN_EVENTSINK_MAP(CMedicationDlg, CNxDialog)
	//{{AFX_EVENTSINK_MAP(CMedicationDlg)
	//ON_EVENT(CMedicationDlg, IDC_NXDLHISTORY, 7 /* RButtonUp */, OnRButtonUpNxdlhistory, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMedicationDlg, IDC_NXDLPATALLERGIES, 10 /* EditingFinished */, OnEditingFinishedNxdlpatallergies, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_I4)
	ON_EVENT(CMedicationDlg, IDC_NXDLMEDSCHEDULES, 6 /* RButtonDown */, OnRButtonDownNxdlmedschedules, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMedicationDlg, IDC_NXDLMEDSCHEDULES, 3 /* DblClickCell */, OnDblClickCellNxdlmedschedules, VTS_I4 VTS_I2)
	ON_EVENT(CMedicationDlg, IDC_NXDLPATALLERGIES, 9 /* EditingFinishing */, OnEditingFinishingNxdlpatallergies, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CMedicationDlg, IDC_CURRENT_MED_LIST, 2 /* SelChanged */, OnSelChangedCurrentMedList, VTS_I4)
	ON_EVENT(CMedicationDlg, IDC_NXDLPATALLERGIES, 2 /* SelChanged */, OnSelChangedNxdlpatallergies, VTS_I4)
	ON_EVENT(CMedicationDlg, IDC_NXDLMEDSCHEDULES, 2 /* SelChanged */, OnSelChangedNxdlmedschedules, VTS_I4)
	//ON_EVENT(CMedicationDlg, IDC_NXDLHISTORY, 19 /* LeftClick */, OnLeftClickNxdlhistory, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	//ON_EVENT(CMedicationDlg, IDC_NXDLHISTORY, 18, CMedicationDlg::RequeryFinishedNxdlhistory, VTS_I2)
	ON_EVENT(CMedicationDlg, IDC_NXDLPATALLERGIES, 18, CMedicationDlg::RequeryFinishedNxdlpatallergies, VTS_I2)
	ON_EVENT(CMedicationDlg, IDC_CURRENT_MED_LIST, 18, CMedicationDlg::RequeryFinishedCurrentMedList, VTS_I2)
	ON_EVENT(CMedicationDlg, IDC_CURRENT_MED_LIST, 8, CMedicationDlg::OnEditingStartingCurrentMedList, VTS_I4 VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CMedicationDlg, IDC_CURRENT_MED_LIST, 10, CMedicationDlg::OnEditingFinishedCurrentMedList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CMedicationDlg, IDC_CURRENT_MED_LIST, 7, CMedicationDlg::RButtonUpCurrentMedList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMedicationDlg, IDC_NXDLPATALLERGIES, 7, CMedicationDlg::RButtonUpNxdlpatallergies, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMedicationDlg, IDC_CURRENT_MED_LIST, 19, CMedicationDlg::OnLeftClickCurrentMedList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMedicationDlg, IDC_CURRENT_MED_LIST, 9, CMedicationDlg::EditingFinishingCurrentMedList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CMedicationDlg, IDC_CURRENT_MED_LIST, 22, CMedicationDlg::ColumnSizingFinishedCurrentMedList, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CMedicationDlg, IDC_NXDLPATALLERGIES, 22, CMedicationDlg::ColumnSizingFinishedpatallergies, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CMedicationDlg, IDC_NXDL_MED_SEARCH_RESULTS, 16, CMedicationDlg::SelChosenNxdlMedSearchResults, VTS_DISPATCH)
	ON_EVENT(CMedicationDlg, IDC_NXDL_ALLERGY_SEARCH_RESULTS, 16, CMedicationDlg::SelChosenNxdlAllergySearchResults, VTS_DISPATCH)
END_EVENTSINK_MAP()

void CMedicationDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {

		// (a.walling 2010-10-14 15:09) - PLID 40978 - Dead code
		/*if (m_boAllowUpdate == FALSE)
		return;*/

		// (a.walling 2010-10-14 15:58) - PLID 40978
		bool bPatientChanged = false;
		long nCurrentlyLoadedID = m_id;
		m_id = GetActivePatientID();

		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;

			// (j.jones 2012-09-26 11:25) - PLID 52872 - track that the patient changed
			bPatientChanged = true;
		}

		if (m_CurrentMedsChecker.Changed()) {
			m_ForceRefresh = true;
		}

		if (m_CurrentMedsChecker.Changed()) {
			m_ForceRefresh = true;
		}

		if (bForceRefresh || m_ForceRefresh) {
			Refresh();
		}
		m_ForceRefresh = false;

		EnableAppropriateButtons();

		// (j.jones 2012-09-26 11:25) - PLID 52872 - if the patient changed, show drug interactions (if any exist)
		if (bPatientChanged) {
			// (r.gonet 09/20/2013) - PLID 58396 - Set the text to black to start because we don't know 
			// if the patient has medication history. If they do, we'll change it to red later.
			m_btnMedicationHistory.SetTextColor(RGB(0, 0, 0));
			// (j.fouts 2012-11-14 11:42) - PLID 53439 - Handle changing patients
			m_pPrescriptionQueueDlg->ChangePatient(m_id);
		}
		else
		{
			// (j.fouts 2013-04-22 14:13) - PLID 54719 - UpdateView on the queue since preferances changed
			m_pPrescriptionQueueDlg->UpdateView();
			m_pPrescriptionQueueDlg->RequeryQueue();
			// (a.wilson 2013-02-12 15:46) - PLID 53797
			m_pPrescriptionQueueDlg->RequeryRenewalRequests();
		}

		bool bShowPatientEducationButton = (GetRemotePropertyInt("ShowPatientEducationButtons", 1, 0, GetCurrentUserName()) ? true : false);
		bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
		NXDATALISTLib::IColumnSettingsPtr pInfoButtonColumn = m_CurMedList->GetColumn(cmlcInfoButton);
		if (bShowPatientEducationButton) {
			pInfoButtonColumn->ColumnStyle |= NXDATALISTLib::csVisible;
		}
		else {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Hide the patient education info button column
			pInfoButtonColumn->ColumnStyle &= ~NXDATALISTLib::csVisible;
		}
		NXDATALISTLib::IColumnSettingsPtr pMedNameColumn = m_CurMedList->GetColumn(cmlcMedName);
		if (bShowPatientEducationLink) {
			pMedNameColumn->FieldType = NXDATALISTLib::cftTextSingleLineLink;
		}
		else {
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Make the med name a non-hyperlink.
			pMedNameColumn->FieldType = NXDATALISTLib::cftTextSingleLine;
		}

		// (s.dhole 2013-06-07 11:44) - PLID 56926
		CString strColWidths;
		// (j.jones 2016-01-12 09:33) - PLID 67855 - renamed CurrentPatientMedicationColumnWidth to CurrentPatientMedicationColumnWidths
		// to effectively reset the stored widths for current medications
		strColWidths = GetRemotePropertyText("CurrentPatientMedicationColumnWidths", "", 0, GetCurrentUserName(), false);
		ResizeCurrentMedicationColumns(strColWidths);
		// (s.dhole 2013-07-08 14:29) - PLID  56931 load column width
		strColWidths = GetRemotePropertyText("CurrentPatientAllergyColumnWidth", "", 0, GetCurrentUserName(), false);
		ResizeAllergyColumns(strColWidths);
	}NxCatchAll("Error in CMedicationDlg::UpdateView");
}

// (a.walling 2010-10-14 16:00) - PLID 40978
void CMedicationDlg::Refresh()
{
	try {

		CWnd* pFocus = GetFocus();

		CString strSql;

		// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkbox to show/hide discontinued allergies
		m_checkHideDiscontinuedAllergies.SetCheck(GetRemotePropertyInt("HideDiscontinuedAllergies", 0, 0, GetCurrentUserName(), true));
		m_chkHideInactive.SetCheck(GetRemotePropertyInt("HideInactiveMedications", 0, 0, GetCurrentUserName(), true));

		if (m_pPatAllergies != NULL) {
			// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkbox to show/hide discontinued allergies
			if (m_checkHideDiscontinuedAllergies.GetCheck()) {
				strSql.Format("PersonID = %li AND Discontinued = 0", m_id);
			}
			else {
				strSql.Format("PersonID = %li", m_id);
			}
			m_pPatAllergies->WhereClause = (bstr_t)strSql;
			m_pPatAllergies->Requery();
		}

		if (m_NxDlMedSchedules != NULL) {
			strSql.Format("PatientID = %li", m_id);
			m_NxDlMedSchedules->WhereClause = (bstr_t)strSql;
			m_NxDlMedSchedules->Requery();
		}

		if (m_CurMedList != NULL) {
			// (j.gruber 2009-06-02 16:43) - PLID 34449 - hide inactive if the checkbox is checked
			if (m_chkHideInactive.GetCheck()) {
				strSql.Format("PatientID = %li AND Discontinued = 0", m_id);
			}
			else {
				strSql.Format("PatientID = %li", m_id);
			}
			m_CurMedList->WhereClause = (bstr_t)strSql;
			m_CurMedList->Requery();
		}

		// (j.jones 2008-11-25 09:40) - PLID 28508 - load the value for the allergy review checkbox,
		// and get DisplayAllergyWarning at the same time
		BOOL bDisplayAllergyWarning = FALSE;
		BOOL bReviewedAllergies = FALSE;
		COleDateTime dtReviewedOn, dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);
		dtReviewedOn = dtInvalid;
		CString strReviewedBy;
		BOOL bHasNoAllergies = FALSE;
		BOOL bHasNoMeds = FALSE;

		// (j.jones 2012-10-16 14:35) - PLID 53179 - added HasNoAllergies
		// (j.jones 2012-10-17 13:24) - PLID 51713 - added HasNoMeds
		_RecordsetPtr rs = CreateParamRecordset("SELECT PersonT.DisplayAllergyWarning, "
			"PatientsT.AllergiesReviewedOn, UsersT.Username, PatientsT.HasNoAllergies, PatientsT.HasNoMeds "
			"FROM PatientsT "
			"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
			"LEFT JOIN UsersT ON PatientsT.AllergiesReviewedBy = UsersT.PersonID "
			"WHERE PatientsT.PersonID = {INT}", m_id);
		if (!rs->eof) {
			bDisplayAllergyWarning = AdoFldBool(rs, "DisplayAllergyWarning", FALSE);
			dtReviewedOn = AdoFldDateTime(rs, "AllergiesReviewedOn", dtInvalid);
			if (dtReviewedOn.GetStatus() != COleDateTime::invalid) {
				bReviewedAllergies = TRUE;
				strReviewedBy = AdoFldString(rs, "Username", "<Unknown User>");
			}
			else {
				bReviewedAllergies = FALSE;
				strReviewedBy = "";
			}
			// (j.jones 2012-10-16 14:35) - PLID 53179 - added HasNoAllergies
			bHasNoAllergies = AdoFldBool(rs, "HasNoAllergies", FALSE);
			// (j.jones 2012-10-17 13:24) - PLID 51713 - added HasNoMeds
			bHasNoMeds = AdoFldBool(rs, "HasNoMeds", FALSE);
		}
		else {
			//this should be impossible
			AfxThrowNxException("Could not load allergy status for patient ID %li", m_id);
		}
		rs->Close();

		// (j.jones 2012-10-16 14:35) - PLID 53179 - update the "no known allergies" checkbox
		m_checkHasNoAllergies.SetCheck(bHasNoAllergies);

		// (j.jones 2012-10-17 13:25) - PLID 51713 - update the "no medications" checkbox
		m_checkHasNoMeds.SetCheck(bHasNoMeds);

		//update the allergy review checkbox
		UpdateAllergyReviewLabel(bReviewedAllergies, dtReviewedOn, strReviewedBy);
		m_checkReviewedAllergies.SetCheck(bReviewedAllergies);

		//this field is now pulled in the above recordset
		((CButton*)GetDlgItem(IDC_ALLERGYWARNING))->SetCheck(bDisplayAllergyWarning);

		m_ForceRefresh = false;

	} NxCatchAllThrow(__FUNCTION__);
}

void CMedicationDlg::SetColor(OLE_COLOR nNewColor)
{
	m_bkg.SetColor(nNewColor);
	m_bg3.SetColor(nNewColor);
	m_bkg4.SetColor(nNewColor);

	// (j.fouts 2012-11-14 11:42) - PLID 53439 - Queue needs to have a consistent color
	m_pPrescriptionQueueDlg->SetColor(nNewColor);

	CPatientDialog::SetColor(nNewColor);
}

void CMedicationDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	EnableAppropriateButtons();
	// (j.fouts 2012-09-07 13:18) - PLID 52482 - Check/Show interactions when we open this tab
	// (j.jones 2012-09-26 11:29) - PLID 52872 - drug interaction handling has been moved to UpdateView
}

BOOL CMedicationDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_pPatAllergies = BindNxDataListCtrl(IDC_NXDLPATALLERGIES, false);
		m_NxDlMedSchedules = BindNxDataListCtrl(IDC_NXDLMEDSCHEDULES, false);
		m_CurMedList = BindNxDataListCtrl(IDC_CURRENT_MED_LIST, false);

		//DRT 4/11/2008 - PLID 29636 - NxIconify
		m_btnAddMedSched.AutoSet(NXB_NEW);
		m_btnDeleteMedSched.AutoSet(NXB_DELETE);
		// (j.fouts 2013-01-07 10:44) - PLID 54468 - Moved the drug interactions button back to the medications tab
		m_btnInteractions.AutoSet(NXB_INTERACTIONS);
		// (r.gonet 09/20/2013) - PLID 58416 - Add an icon to the med history button.
		m_btnMedicationHistory.AutoSet(NXB_MEDICATION_HISTORY);

		// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon
		m_hInfoButtonIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INFO_ICON), IMAGE_ICON, 16, 16, 0);

		// (j.jones 2009-03-24 14:44) - PLID 33652 - supported the NewCrop license
		//TES 4/7/2009 - PLID 33882 - Changed this function to return an enum
		CLicense::EPrescribingType ept = g_pLicense->HasEPrescribing(CLicense::cflrSilent);
		if (ept != CLicense::eptNewCrop) {
			// (a.walling 2010-01-15 15:20) - PLID 36902 - Hide the RXCUI column
			m_pPatAllergies->GetColumn(alcRXCUI)->PutStoredWidth(0);
			m_pPatAllergies->GetColumn(alcRXCUI)->PutColumnStyle(csVisible | csFixedWidth);
		}

		// (c.haag 2009-08-18 13:06) - PLID 15479 - We need some ConfigRT bulk caching here
		// (c.haag 2010-02-17 11:28) - PLID 37384 - ReconcileNewRxWithCurMeds
		g_propManager.CachePropertiesInBulk("PatientMedications", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'HideInactiveMedications' OR "
			"Name = 'PatientMedMergePrescriptionToPrinter' OR "
			"Name = 'ShowEnglishPrescriptionColumn' OR "
			"Name = 'MedicationsTabShowPharmacyPhone' OR "
			"Name = 'ReconcileNewRxWithCurMeds' "
			// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkbox to show/hide discontinued allergies
			"OR Name = 'HideDiscontinuedAllergies' OR "
			// (s.dhole 2013-07-08 14:29) - PLID 56926
			// (j.jones 2016-01-12 09:33) - PLID 67855 - renamed CurrentPatientMedicationColumnWidth to CurrentPatientMedicationColumnWidths
			// to effectively reset the stored widths for current medications
			"Name = 'CurrentPatientMedicationColumnWidths' OR "
			"Name = 'CurrentPatientAllergyColumnWidth' OR "// (s.dhole 2013-07-08 14:29) - PLID 56931
			"Name = 'ShowPatientEducationLinks' OR " // (r.gonet 2014-01-27 15:29) - PLID 59339
			"Name = 'ShowPatientEducationButtons' " // (r.gonet 2014-01-27 15:29) - PLID 59339
			"OR Name = 'IncludeFreeTextFDBSearchResults'" // (j.jones 2016-01-21 09:44) - PLID 67994
			")",
			_Q(GetCurrentUserName()));

		// (j.gruber 2009-06-02 17:11) - PLID 34449 - get our inactive setting
		long nHideInactive = GetRemotePropertyInt("HideInactiveMedications", 0, 0, GetCurrentUserName(), true);
		if (nHideInactive == 0){
			m_chkHideInactive.SetCheck(0);
		}
		else {
			m_chkHideInactive.SetCheck(1);
		}

		// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkbox to show/hide discontinued allergies
		m_checkHideDiscontinuedAllergies.SetCheck(GetRemotePropertyInt("HideDiscontinuedAllergies", 0, 0, GetCurrentUserName(), true));

		// (j.fouts 2012-11-14 11:42) - PLID 53439 - Create the queue and display it
		// (j.jones 2012-11-30 15:15) - PLID 53968 - changed this to a pointer
		if (m_pPrescriptionQueueDlg) {
			//this should be null
			ThrowNxException("Failed to create Prescription Queue due to invalid existing pointer.");
		}

		// (j.fouts 2013-01-07 10:44) - PLID 54468 - Moved the drug interactions button back to the medications tab
		m_pPrescriptionQueueDlg = new CPrescriptionQueueDlg(this, GetActivePatientID(), -1, false, true);
		m_pPrescriptionQueueDlg->Create(IDD_PRESCRIPTION_QUEUE, this);
		if (!m_pPrescriptionQueueDlg) {
			//this should not have failed
			ThrowNxException("Prescription Queue failed to create.");
		}

		
		// (j.jones 2016-01-21 09:47) - PLID 67994 - if they do not have FDB, don't show the free text meds option,
		// because all searches are free text only without FDB
		// (j.jones 2016-01-21 14:58) - PLID 67995 - same for allergies
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			//no FDB? no checkbox, and always include free-text meds, because that's all you've got
			m_checkIncludeFreeTextCurrentMeds.SetCheck(TRUE);
			m_checkIncludeFreeTextCurrentMeds.ShowWindow(SW_HIDE);
			//same for allergies
			m_checkIncludeFreeTextAllergies.SetCheck(TRUE);
			m_checkIncludeFreeTextAllergies.ShowWindow(SW_HIDE);
			// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med/allergy search color, hide if no fdb
			m_icoAboutCurrentAllergiesColors.ShowWindow(SW_HIDE);
			m_icoAboutCurrentMedicationColors.ShowWindow(SW_HIDE);
		}
		else {
			// (j.jones 2016-01-21 09:47) - PLID 67994 - added a checkbox to include free text meds in current meds.
			// This preference intentionally only controls the default when entering the tab, never on refresh.
			// This means that changing the pref. while the tab is open intentionally does not update the checkbox.
			long nIncludeFreeTextFDBSearchResults = GetRemotePropertyInt("IncludeFreeTextFDBSearchResults", 0, 0, GetCurrentUserName(), true);
			m_checkIncludeFreeTextCurrentMeds.SetCheck(nIncludeFreeTextFDBSearchResults & INCLUDEFREETEXT_CURMEDS);
			// (j.jones 2016-01-21 15:07) - PLID 67995 - supported this preference in allergies
			m_checkIncludeFreeTextAllergies.SetCheck(nIncludeFreeTextFDBSearchResults & INCLUDEFREETEXT_ALLERGIES);

			// (b.eyers 2016-02-090 - PLID 67979 - added an icon for information about med/allergy search color
			CString strMedsToolTipText = "All medications with a salmon background are imported and are checked for interactions. \r\n"
				"All medications with a red background have changed since being imported, and must be updated before being used on new prescriptions. \r\n"
				"Using free text medications (white background) rather than those from the imported lists (salmon background) will result in a lack of interaction warnings.";
			CString strAllergyToolTipText = "All allergies with a salmon background are imported and are checked for interactions. \r\n"
				"All allergies with a red background have changed since being imported, and should be updated. \r\n"
				"Using free text allergies (white background) rather than those from the imported lists (salmon background) will result in a lack of interaction warnings.";
			m_icoAboutCurrentMedicationColors.LoadToolTipIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK), strMedsToolTipText, false, false, false);
			m_icoAboutCurrentAllergiesColors.LoadToolTipIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDB_QUESTION_MARK), strAllergyToolTipText, false, false, false);
			m_icoAboutCurrentAllergiesColors.EnableClickOverride();
			m_icoAboutCurrentMedicationColors.EnableClickOverride();
		}

		// (b.savon 2013-01-14 12:52) - PLID 54592
		//(s.dhole 2/9/2015 10:46 AM ) - PLID 64561
		// (j.jones 2016-01-21 09:47) - PLID 67994 - moved the bind to ResetCurrentMedsSearchProvider
		ResetCurrentMedsSearchProvider();
		
		// (b.savon 2013-01-14 16:12) - PLID 54613
		//(s.dhole 3/10/2015 5:24 PM ) - PLID 64564
		// (j.jones 2016-01-21 14:58) - PLID 67995 - moved the bind to ResetAllergySearchProvider
		ResetAllergySearchProvider();

		// (a.walling 2010-10-14 15:58) - PLID 40978 - Don't set m_id until UpdateView
		//m_id = GetActivePatientID();
		SecureControls();
	}NxCatchAll("Error in CMedicationDlg::OnInitDialog");

	return FALSE;
}

// (j.fouts 2012-11-14 11:42) - PLID 53439 -- Commenting this out for now. We will likely move this to the queue for consisancy
//void CMedicationDlg::OnRButtonUpNxdlhistory(long nRow, long nCol, long x, long y, long nFlags) 
//{	
//	//TS:  I'm taking all this out on Kamal's advice; but I am intentionally leaving the functions to
//	//actually delete Prescriptions intact, so if it ever turns out someone desperately needs this 
//	//functionality, all we'll have to do is uncomment this code right here.  Basically what I'm saying
//	//is, removing functionality makes me nervous.
//
//	//TS:  TODO: My foresight has been rewarded, and this code is back in.  However, we need to come 
//	//up with some sort of real resolution to this, instead of taking this code in and out as the whim takes
//	//us, confusing our clients and ourselves, and in the end leaving our code, and our psyches, as shattered, 
//	//spaghetti-like clumps of random commands with no logic or guiding purpose.
//
//	if (nRow == -1) {
//		return;
//	}
//
//	m_nSelItem = nRow;
//	m_NxDlHistory->PutCurSel(nRow);
//
//	// Build a menu popup with the ability to delete the current row
//	CMenu  menPopup;
//	menPopup.m_hMenu = CreatePopupMenu();
//
//	// (c.haag 2009-08-18 13:04) - PLID 15479 - Update the wording based on the preview status. This menu is
//	// synonymous with pressing the button on the dialog
//	long nMergeToPrinter = GetRemotePropertyInt("PatientMedMergePrescriptionToPrinter", 0, 0, GetCurrentUserName(), true);
//	if (nMergeToPrinter == 0) {
//		menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_PREVIEW_SELECTED, "Preview Selected");
//	} else {
//		menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_PREVIEW_SELECTED, "Print Selected");
//	}
//	// (j.jones 2009-02-20 16:35) - PLID 33165 - added option to send to NewCrop (temporarily removed)
//	//menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_SEND_TO_NEWCROP, "Submit Prescription Electronically");
//	if(GetCurrentUserPermissions(bioPatientMedication) & SPT_____D______ANDPASS)
//		menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_DELETE, "Delete");
//	else
//		menPopup.InsertMenu(-1, MF_BYPOSITION|MF_GRAYED, IDM_DELETE, "Delete");
//
//	CPoint pt;
//	pt.x = x;
//	pt.y = y;
//	GetCursorPos(&pt);
//	menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
//	
//}

BOOL CMedicationDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{

	switch ((wParam)) {
	case IDM_DELETE_MEDICATION:
		// (j.fouts 2012-11-14 11:42) - PLID 53439 -- Commenting this out for now. We will likely move this to the queue for consisancy
		//try {
		//	if (m_bHistoryUse) {
		//		if (!CheckCurrentUserPermissions(bioPatientMedication, sptDelete))
		//			break;

		//		//m.hancock - 4/9/2006 - PLID 20020 - You should not be able to delete medications if they exist for EMN's
		//		//Check if this medication is associated with an EMN
		//		long nSelVal = VarLong(m_NxDlHistory->GetValue(m_nSelItem, COLUMN_PRESC_ID));
		//		if(!IsRecordsetEmpty("SELECT EMRID FROM EMRMedicationsT WHERE Deleted = 0 AND MedicationID = %li", nSelVal)) {
		//			//State why the medication cannot be deleted
		//			AfxMessageBox("This medication cannot be deleted because it exists on an EMN for this patient.");
		//		}
		//		else {
		//			// (a.walling 2009-04-23 13:12) - PLID 34046 - Warn if e-presciption
		//			CString strMsg;

		//			// (c.haag 2009-07-02 11:57) - PLID 34102 - Don't discriminate on the message type being mtPendingRx. Just don't allow the deletion
		//			// if the prescription exists in the SureScripts table.
		//			// (j.jones 2012-10-29 14:45) - PLID 53259 - also cannot delete if the prescription status is E-Prescribed
		//			if(ReturnsRecordsParam("SELECT TOP 1 SureScriptsMessagesT.ID "
		//				"FROM SureScriptsMessagesT "
		//				"WHERE PatientMedicationID = {INT} "
		//				"UNION SELECT TOP 1 PatientMedications.ID "
		//				"FROM PatientMedications "
		//				"WHERE PatientMedications.ID = {INT} AND PatientMedications.QueueStatus IN ({SQL})",
		//				nSelVal, nSelVal, GetERxStatusFilter())) {

		//				// (a.walling 2009-04-24 13:53) - PLID 34046 - Prevent deleting entirely
		//				MessageBox("This prescription cannot be deleted because it has been electronically prescribed, or scheduled to be electronically prescribed.", NULL, MB_ICONSTOP);

		//				return CNxDialog::OnCommand(wParam, lParam);
		//			}

		//			strMsg += "Are you sure you want to remove this prescription?";

		//			if (IDYES == MsgBox(MB_YESNO, strMsg)) {
		//				m_NxDlHistory->SetSelByColumn(0, m_nSelItem);
		//				CString strOld = CString(m_NxDlHistory->GetValue(m_nSelItem, COLUMN_NAME).bstrVal);
		//				//strOld.TrimRight(" - {Inactive}");
		//				CString strTimeOfDelete = _Q(FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDateTime));
		//				//long nSelVal = VarLong(m_NxDlHistory->GetValue(m_nSelItem, COLUMN_PRESC_ID));
		//				ExecuteSql("UPDATE PatientMedications SET Deleted = 1, DeletedBy = '%s', DeletedDate = '%s' WHERE ID = %li", _Q(GetCurrentUserName()), strTimeOfDelete, nSelVal);
		//				m_NxDlHistory->RemoveRow(m_nSelItem);
		//				m_NxDlHistory->PutCurSel(-1);

		//				//auditing
		//				long nAuditID= -1;
		//				nAuditID = BeginNewAuditEvent();
		//				if(nAuditID != -1) {
		//					AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientPrescDelete, GetActivePatientID(), strOld, "<Deleted>", aepMedium, aetDeleted);
		//					_RecordsetPtr rs = CreateRecordset("SELECT EMRID FROM EMRMedicationsT WHERE MedicationID = %li", nSelVal);
		//					if(!rs->eof) {
		//						//also audit EMR
		//						AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiEMNPrescriptionRemoved, AdoFldLong(rs, "EMRID",-1), strOld, "<Deleted>", aepMedium, aetDeleted);
		//					}
		//					rs->Close();
		//				}
		//				
		//				// (a.walling 2009-04-22 18:01) - PLID 34046
		//				SureScripts::DeletePendingNewRx(GetRemoteData(), nSelVal);

		//				// (j.jones 2012-10-29 11:28) - PLID 53454 - show drug interactions, if any
		//				CheckInteractions();
		//			}
		//		}
		//	}
		//	
		//}NxCatchAll("Error deleting prescription.");
		break;

	case IDM_PREVIEW_SELECTED:
		//OnBtnPreview();			
		break;

		// (j.jones 2009-02-20 16:35) - PLID 33165 - added option to send to NewCrop (temporarily removed)
		/*
		case IDM_SEND_TO_NEWCROP:
		if(m_nSelItem != -1) {
		long nID = VarLong(m_NxDlHistory->GetValue(m_nSelItem, COLUMN_PRESC_ID));

		CString strWindowDescription;
		strWindowDescription.Format("Accessing Electronic Prescription Account for Patient: %s", GetExistingPatientName(m_id));

		CNewCropBrowserDlg dlgNewCrop;
		dlgNewCrop.DoModal(strWindowDescription, ncatSubmitPrescription, nID);
		}
		break;
		*/

	case IDM_ADD_SCHED:
		OnAddMedsched();
		break;
	case IDM_EDIT_SCHED: {
		_variant_t var = m_NxDlMedSchedules->GetValue(m_nMedSchedSelItem, 0);
		OnEditMedSchedule(var.lVal);
		break;
	}
	case IDM_DEL_SCHED:
		OnDeleteMedsched();
		break;
	case IDM_SHOW_MONO:
		try
		{
			// (j.fouts 2012-08-10 09:44) - PLID 52090 - Show Monograph was clicked so we should display the monograph dlg
			if (m_CurMedList->CurSel >= 0)
			{
				//Get the MedId for the selected drug
				long nFDBMedID = VarLong(m_CurMedList->GetValue(m_CurMedList->CurSel, cmlcFDBID), -1);

				// (j.fouts 2012-08-20 09:26) - PLID 51719 - Don't give an option for monograph to non FDB drugs
				if (nFDBMedID >= 0)
				{
					// (j.fouts 2012-09-25 09:27) - PLID 52825 - Check that the database exists
					if (FirstDataBank::EnsureDatabase(this, true))
					{
						ShowMonograph(nFDBMedID, this);
					}
				}
			}
		}
		NxCatchAll(__FUNCTION__);
		break;
	case IDM_SHOW_LEAFLET: // (j.luckoski 2012-10-04 16:57) - Load the leaflet if you have a current medication
		try
		{
			if (m_CurMedList->CurSel >= 0)
			{
				long nFDBMedID = VarLong(m_CurMedList->GetValue(m_CurMedList->CurSel, cmlcFDBID), -1);

				if (nFDBMedID >= 0)
				{
					// (j.fouts 2013-06-10 11:18) - PLID 56808 - If the database does not exist we cannot query it
					if (FirstDataBank::EnsureDatabase(this, true))
					{
						ShowLeaflet(nFDBMedID, this);
					}
				}
			}
		}
		NxCatchAll(__FUNCTION__);
		break;
	case IDM_DELETE_CURRENT_MED:
		try {

			long nCurSel = m_CurMedList->GetCurSel();

			if (nCurSel == -1) {

				MsgBox("Please choose a medication to delete");
				return CNxDialog::OnCommand(wParam, lParam);
			}

			//give them a messagebox to make sure they are sure they want to do this
			if (IDYES == MsgBox(MB_YESNO, "This action is unrecoverable, are you sure you want to delete this medication?")) {

				//let's do it!
				long nID = VarLong(m_CurMedList->GetValue(nCurSel, 0));

				ExecuteSql("DELETE FROM CurrentPatientMedsT WHERE ID = %li", nID);

				// (c.haag 2007-10-19 17:13) - PLID 27827 - Audit
				long nAuditID = BeginNewAuditEvent();
				long nEMRDataID = VarLong(m_CurMedList->GetValue(nCurSel, 3));
				CString strName = VarString(m_CurMedList->GetValue(nCurSel, 2), "");
				//strName.TrimRight(" - {Inactive}");
				AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiCurrentPatientMedsDeleted, nEMRDataID, strName, "<Deleted>", aepHigh, aetDeleted);

				m_CurMedList->RemoveRow(nCurSel);

				// (j.jones 2008-11-25 12:24) - PLID 32183 - send a tablechecker
				m_CurrentMedsChecker.Refresh(m_id);

				// (c.haag 2007-01-25 12:42) - PLID 24420 - Warn the user about any EMR's with "official"
				// Current Medications details that are inconsistent with the patient's official medication list
				// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
				// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
				if (g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
					WarnMedicationDiscrepanciesWithEMR(m_id);
				}
				// (c.haag 2010-09-21 13:43) - PLID 40610 - Do not create todo alarms for decisions if we're only deleting a med
				//TodoCreateForDecisionRules(GetRemoteData(), m_id);
			}

			EnableAppropriateButtons();
			// (j.fouts 2012-08-10 09:46) - PLID 52089 - Meds Changed Check Interactions
			// (j.fouts 2013-01-07 10:44) - PLID 54468 - Check interactions rather than requering the whole queue
			// (j.fouts 2013-02-28 14:24) - PLID 54429 - Update the count with the return value
			SetInteractionCount(GetMainFrame()->ShowDrugInteractions(GetActivePatientID()));
		}NxCatchAll("Error Removing Current Medication!");
		break;
		// (b.savon 2013-01-25 12:41) - PLID 54854 - Discontinue Med
	case IDM_DISCONTINUE_CURRENT_MED:
		try {
			long nCurSel = m_CurMedList->GetCurSel();

			if (nCurSel == -1) {

				MsgBox("Please choose a medication to discontinue.");
				return CNxDialog::OnCommand(wParam, lParam);
			}

			if (MsgBox(MB_YESNO, "Are you sure you wish to discontinue this medication?") == IDNO) {
				return CNxDialog::OnCommand(wParam, lParam);
			}

			// (s.dhole 2013-06-07 11:44) - PLID 56926
			UpdateCurrentMedRecord(nCurSel, cmlcDiscontinued, _variant_t(1L));

			// (c.haag 2007-10-19 17:13) - PLID 27827 - Audit
			long nAuditID = BeginNewAuditEvent();
			long nEMRDataID = VarLong(m_CurMedList->GetValue(nCurSel, 3));
			CString strName = VarString(m_CurMedList->GetValue(nCurSel, 2), "");
			//strName.TrimRight(" - {Inactive}");
			AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiCurrentMedicationInactivated, nEMRDataID, strName, "<Discontinued>", aepHigh, aetChanged);

			if (m_chkHideInactive.GetCheck() == BST_CHECKED){
				m_CurMedList->RemoveRow(nCurSel);
			}
			else{
				m_CurMedList->Requery();
			}

			// (j.jones 2008-11-25 12:24) - PLID 32183 - send a tablechecker
			m_CurrentMedsChecker.Refresh(m_id);

			// (c.haag 2007-01-25 12:42) - PLID 24420 - Warn the user about any EMR's with "official"
			// Current Medications details that are inconsistent with the patient's official medication list
			// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
			// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
			if (g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
				WarnMedicationDiscrepanciesWithEMR(m_id);
			}

			// (b.savon 2013-03-25 15:54) - PLID 54854 - Check interactions
			SetInteractionCount(GetMainFrame()->ShowDrugInteractions(GetActivePatientID()));
		}NxCatchAll(__FUNCTION__);
		break;
	case IDM_DELETE_CURRENT_ALLERGY:
		try {
			//if they selected something, delete it, otherwise tell them to select something
			long nCurSel = m_pPatAllergies->GetCurSel();

			if (nCurSel == -1) {
				//they didn't select anything
				MsgBox("Please select an allergy to delete");
				return CNxDialog::OnCommand(wParam, lParam);
			}

			if (AfxMessageBox("Are you sure you wish to delete this allergy?", MB_YESNO) == IDNO) {
				return CNxDialog::OnCommand(wParam, lParam);
			}

			// (z.manning 2010-01-06 09:42) - PLID 35766 - Use the alcName enum here instead of hard-coded column value
			CString strOld = CString(m_pPatAllergies->GetValue(nCurSel, alcName).bstrVal);

			// (j.gruber 2009-06-08 15:40) - PLID 34395 - trim the {Inactive} off
			// (j.jones 2012-04-04 11:17) - PLID 49411 - We should not have used TrimRight,
			// it would trim off additional letters in the word Inactive. However, we actually
			// want to keep the word Inactive in our audit, so we know that an inactive allergy
			// was deleted.
			//strOld.TrimRight(" - {Inactive}");

			long nDelID = VarLong(m_pPatAllergies->GetValue(nCurSel, alcID));
			ExecuteSql("DELETE FROM AllergyIngredientT WHERE PatientAllergyID = %li", nDelID);
			ExecuteSql("DELETE FROM PatientAllergyT WHERE ID = %li", nDelID);
			//update the datalist
			m_pPatAllergies->Requery();

			// (c.haag 2007-04-05 17:54) - PLID 25524 - Warn the user about any EMR's with "official"
			// Allergies details that are inconsistent with the patient's official allergy list
			// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
			// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
			if (g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
				WarnAllergyDiscrepanciesWithEMR(m_id);
			}

			//audit
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientAllergyDelete, GetActivePatientID(), strOld, "<Deleted>", aepMedium, aetDeleted);

			// (j.jones 2008-11-25 11:25) - PLID 28508 - clear the allergy review status
			UpdateAllergyReviewStatus(FALSE);

			// (j.jones 2008-11-25 12:20) - PLID 32183 - send a tablechecker, after the allergy review status is changed
			m_PatientAllergiesChecker.Refresh(m_id);

			EnableAppropriateButtons();

			// (j.fouts 2012-08-10 09:46) - PLID 52089 - Allergies Changed Check Interactions	
			// (j.fouts 2013-01-07 10:44) - PLID 54468 - Don't requery the whole queue, just check interactions
			// (j.fouts 2013-02-28 14:24) - PLID 54429 - Update the count with the return value
			SetInteractionCount(GetMainFrame()->ShowDrugInteractions(GetActivePatientID()));
		}NxCatchAll("Error in OnClickDeleteAllergy");
		break;
		// (b.savon 2013-01-25 12:41) - PLID 54854 - Discontinue Allergy
	case IDM_DISCONTINUE_CURRENT_ALLERGY:
		try{
			//if they selected something, delete it, otherwise tell them to select something
			long nCurSel = m_pPatAllergies->GetCurSel();

			if (nCurSel == -1) {
				//they didn't select anything
				MsgBox("Please select an allergy to discontinue.");
				return CNxDialog::OnCommand(wParam, lParam);
			}

			if (MsgBox(MB_YESNO, "Are you sure you wish to discontinue this allergy?") == IDNO) {
				return CNxDialog::OnCommand(wParam, lParam);
			}

			// (z.manning 2010-01-06 09:42) - PLID 35766 - Use the alcName enum here instead of hard-coded column value
			CString strOld = CString(m_pPatAllergies->GetValue(nCurSel, alcName).bstrVal);

			// (j.gruber 2009-06-08 15:40) - PLID 34395 - trim the {Inactive} off
			// (j.jones 2012-04-04 11:17) - PLID 49411 - We should not have used TrimRight,
			// it would trim off additional letters in the word Inactive. However, we actually
			// want to keep the word Inactive in our audit, so we know that an inactive allergy
			// was deleted.
			//strOld.TrimRight(" - {Inactive}");

			long nDelID = VarLong(m_pPatAllergies->GetValue(nCurSel, alcID));
			long nAllergyID = VarLong(m_pPatAllergies->GetValue(nCurSel, alcAllergyID));
			ExecuteSql("DELETE FROM AllergyIngredientT WHERE PatientAllergyID = %li", nDelID);
			// (s.dhole 2013-07-05 16:04) - PLID 56931 Added updatedate
			ExecuteParamSql("UPDATE PatientAllergyT SET Discontinued = 1, DiscontinuedDate = GETDATE(), LastUpdateDate = GETDATE() WHERE ID = {INT}", nDelID);

			//update the datalist
			m_pPatAllergies->Requery();

			// (c.haag 2007-04-05 17:54) - PLID 25524 - Warn the user about any EMR's with "official"
			// Allergies details that are inconsistent with the patient's official allergy list
			// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
			// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
			if (g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
				WarnAllergyDiscrepanciesWithEMR(m_id);
			}

			//audit
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if (nAuditID != -1)
				AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientInactivateAllergy, GetActivePatientID(), strOld, "<Discontinued>", aepHigh, aetChanged);

			// (j.jones 2008-11-25 11:25) - PLID 28508 - clear the allergy review status
			UpdateAllergyReviewStatus(FALSE);

			// (j.jones 2008-11-25 12:20) - PLID 32183 - send a tablechecker, after the allergy review status is changed
			m_PatientAllergiesChecker.Refresh(m_id);

			EnableAppropriateButtons();

			// (b.savon 2013-03-25 15:54) - PLID 54854 - Check interactions
			SetInteractionCount(GetMainFrame()->ShowDrugInteractions(GetActivePatientID()));
		}NxCatchAll(__FUNCTION__);
		break;
	}

	return CNxDialog::OnCommand(wParam, lParam);
}

void CMedicationDlg::OnEditingFinishedNxdlpatallergies(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, long bCommit)
{
	// (c.haag 2010-01-04 14:12) - PLID 35766 - This conditional used to test the number 2 instead
	// of the proper enumeration. I am not a number! I am a free enum!
	if (nCol == aldNotes) {
		//The user is adding a description to the allergy
		try {
			CString strOldDesc = (VT_EMPTY == varOldValue.vt) ? "" : VarString(varOldValue, "");
			CString strNewDesc = VarString(varNewValue, "");

			// (c.haag 2007-10-19 12:01) - PLID 27822 - Do nothing if the names match
			if (strOldDesc == strNewDesc)
				return;

			//get the ID of the allergy
			long nAllergyID;
			nAllergyID = VarLong(m_pPatAllergies->GetValue(nRow, alcID));
			// (s.dhole 2013-07-05 16:31) - PLID 56931 Added last Update Date
			ADODB::_RecordsetPtr pRs = CreateParamRecordset("UPDATE PatientAllergyT SET Description = {STRING} , LastUpdateDate = GETDATE() "
				" OUTPUT inserted.LastUpdateDate  "
				" WHERE ID = {INT};", strNewDesc, nAllergyID);
			if (!pRs->eof){
				_variant_t var = pRs->Fields->Item["LastUpdateDate"]->Value;
				m_pPatAllergies->PutValue(nRow, alcLastUpdateDate, var);
			}

			// (c.haag 2007-10-19 12:01) - PLID 27822 - Audit the description change
			long nAuditID = BeginNewAuditEvent();
			CString strOldAudit, strNewAudit;
			CString strAllergyName;
			// (j.gruber 2009-06-08 15:40) - PLID 34395 - trim the {Inactive} off
			strAllergyName = VarString(m_pPatAllergies->GetValue(nRow, alcName));
			// (j.jones 2012-04-04 11:17) - PLID 49411 - We should not have used TrimRight,
			// it would trim off additional letters in the word Inactive. However, we actually
			// want to keep the word Inactive in our audit, so we know that an inactive allergy
			// was changed.
			//strAllergyName.TrimRight(" - {Inactive}");
			strOldAudit.Format("Allergy: '%s' - Description: %s", strAllergyName, strOldDesc);
			strNewAudit.Format("Description: %s", strNewDesc);
			AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientAllergyDescription, GetActivePatientID(), strOldAudit, strNewAudit, aepMedium, aetChanged);

			m_pPatAllergies->Requery();

			// (j.jones 2008-11-25 11:25) - PLID 28508 - clear the allergy review status
			UpdateAllergyReviewStatus(FALSE);

			// (j.jones 2008-11-25 12:20) - PLID 32183 - send a tablechecker, after the allergy review status is changed
			m_PatientAllergiesChecker.Refresh(m_id);

		}NxCatchAll("Error in OnEditingFinishedNxdlpatallergies");
	}
}

void CMedicationDlg::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	if (!bMedSchedPrintInfoLoaded)
		LoadMedSchedPrintInfo();

	COleDateTime dtOriginalDate = COleDateTime::GetCurrentTime();
	BOOL bStartDate = FALSE;
	_RecordsetPtr rs = CreateRecordset("SELECT ProcedureID, StartDate FROM MedScheduleT WHERE ID = %li", m_tempMedSchedID);
	if (!rs->eof) {
		_variant_t var = rs->Fields->Item["StartDate"]->Value;
		if (var.vt == VT_DATE) {
			dtOriginalDate = VarDateTime(var);
			bStartDate = TRUE;
		}
	}

	if (bStartDate) {
		//calc pages based on how many months we need
		long nMaxPage = 1;
		long nMaxMonth = COleDateTime(dtOriginalDate + COleDateTimeSpan(m_nMedSchedDays, 0, 0, 0)).GetMonth();
		long nStartMonth = dtOriginalDate.GetMonth();
		nMaxPage = nMaxMonth - nStartMonth + 1;
		if (nMaxPage <= 0 || (nMaxPage == 1 && m_nMedSchedDays > 31))
			nMaxPage += 12;
		pInfo->SetMaxPage(nMaxPage);
	}
	else {
		//calc how many cells we need
		long nPages = m_nMedSchedDays / 42 + (m_nMedSchedDays % 42 > 0 ? 1 : 0);
		if (nPages <= 0)
			nPages = 1;
		pInfo->SetMaxPage(nPages);
	}
}


void CMedicationDlg::Print(CDC *pDC, CPrintInfo *pInfo)
{
	if (!bMedSchedPrintInfoLoaded)
		LoadMedSchedPrintInfo();

	const char* month_names[12] = { "January", "February", "March",
		"April", "May", "June", "July", "August", "September",
		"October", "November", "December" };

	// Some useful drawing tools
	CBrush greybrush(RGB(192, 192, 192));
	CBrush whitebrush(RGB(255, 255, 255));
	CBrush* pOldBrush = pDC->SelectObject(&whitebrush);
	CPen pen(PS_SOLID, 2, RGB(0, 0, 0));
	CPen* pOldPen = pDC->SelectObject(&pen);
	CFont* pOldFont;

	// Decide if we're printing in color or not
	BOOL bBlackAndWhite;
	if (pDC->GetDeviceCaps(BITSPIXEL) > 1) {
		bBlackAndWhite = FALSE;
	}
	else {
		bBlackAndWhite = TRUE;
	}

	// (j.gruber 2010-06-01 16:11) - PLID 38809 - nActiveMonth needs to be initialized
	long nActiveMonth = -1;
	int nActiveYear;
	int nActiveDay;

	long nDaysVisible = 42;
	long nRowCount = 6;

	long nProcId = -1;
	COleDateTime dtStartDate = COleDateTime::GetCurrentTime();
	COleDateTime dtOriginalDate = COleDateTime::GetCurrentTime();
	BOOL bStartDate = FALSE;
	_RecordsetPtr rs = CreateRecordset("SELECT ProcedureID, StartDate FROM MedScheduleT WHERE ID = %li", m_tempMedSchedID);
	if (!rs->eof) {
		_variant_t var = rs->Fields->Item["ProcedureID"]->Value;
		if (var.vt == VT_I4) {
			nProcId = var.lVal;
		}

		var = rs->Fields->Item["StartDate"]->Value;
		if (var.vt == VT_DATE) {
			dtOriginalDate = dtStartDate = VarDateTime(var);
			bStartDate = TRUE;
		}
	}

	pInfo->SetMinPage(1);
	pInfo->SetMaxPage(1);

	long nPageID = pInfo->m_nCurPage;

	int CurrDay = 1;

	if (nPageID > 1) {

		for (int i = 1; i<nPageID; i++) {
			if (bStartDate) {
				//if we are using dates
				dtStartDate += COleDateTimeSpan(GetDaysInMonth(dtStartDate.GetMonth(), dtStartDate.GetYear()), 0, 0, 0);
			}
			else {
				//just increment the number
				CurrDay += 42;
			}
		}
	}

	//////////////////////////
	COleDateTime dtActiveDate = COleDateTime::GetCurrentTime();
	// Calculate important information based on the active date
	if (bStartDate) {

		dtActiveDate = dtStartDate;

		nActiveMonth = dtActiveDate.GetMonth();
		nActiveYear = dtActiveDate.GetYear();
		nActiveDay = dtActiveDate.GetDay();
		// Move to the beginning of the month and get the day of the week
		dtActiveDate -= COleDateTimeSpan(nActiveDay - 1, 0, 0, 0);
		const int nWeekDay = dtActiveDate.GetDayOfWeek();
		// Move to the next previous sunday
		dtActiveDate -= COleDateTimeSpan(nWeekDay - 1, 0, 0, 0);

		// Calculate the number of rows we'll need
		nDaysVisible = (GetDaysInMonth(nActiveMonth, nActiveYear) + nWeekDay - 1);
		nRowCount = nDaysVisible / 7 + ((nDaysVisible % 7) ? 1 : 0);
	}
	/////////////////////////

	if (bStartDate) {
		//calc pages based on how many months we need
		long nMaxPage = 1;
		long nMaxMonth = COleDateTime(dtOriginalDate + COleDateTimeSpan(m_nMedSchedDays, 0, 0, 0)).GetMonth();
		long nStartMonth = dtOriginalDate.GetMonth();
		nMaxPage = nMaxMonth - nStartMonth + 1;
		if (nMaxPage <= 0 || (nMaxPage == 1 && m_nMedSchedDays > 31))
			nMaxPage += 12;
		pInfo->SetMaxPage(nMaxPage);
	}
	else {
		//calc how many cells we need
		long nPages = m_nMedSchedDays / 42 + (m_nMedSchedDays % 42 > 0 ? 1 : 0);
		if (nPages <= 0)
			nPages = 1;
		pInfo->SetMaxPage(nPages);
	}

	// Set the draw info
	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SetBkMode(TRANSPARENT);

	// Output the page header	
	int starty = DrawPageHeader(pDC, pInfo->m_rectDraw, nProcId);

	// Calculate other useful values
	int boxwidth = pInfo->m_rectDraw.Width() / 8;  // Width of calendar day
	int boxheight = (pInfo->m_rectDraw.Height() - starty - 300) / nRowCount; // Height of calendar day
	int startx = (pInfo->m_rectDraw.Width() - boxwidth * 7) / 2;

	// Select the font we will be using for the rest of the print
	CFont font;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&font, 70, "Arial", pDC);
	pOldFont = pDC->SelectObject(&font);

	// Calculate the height of the text based on the font we will be using to draw it
	LOGFONT lf;
	font.GetLogFont(&lf);
	int textheight = lf.lfHeight;
	if (textheight < 0) {
		textheight = -textheight;
	}
	else if (textheight == 0) {
		textheight = 30;
	}

	// Some variables used repeatedly in the upcoming loop
	CStringArray* pastrNames = NULL;
	CString strDayText;
	long nDayHeaderBottom;
	BOOL bIsDayActive;

	if (bStartDate)
		CurrDay = (long)(dtActiveDate - dtOriginalDate) + 1;

	COleDateTime dtCurrDay = COleDateTime::GetCurrentTime();
	if (bStartDate) {
		dtCurrDay = dtActiveDate;
	}

	// Draw day header
	for (int x = startx; x < startx + boxwidth * 7; x += boxwidth) {
		strDayText = " ";
		//this loop will draw the 7 day headers
		//but if you move this line before the DrawDayNumber, you can have this line on every week

		if (bStartDate) {
			strDayText = FormatDateTimeForInterface(dtCurrDay, "%A");
			COleDateTimeSpan dtOneDay;
			dtOneDay.SetDateTimeSpan(1, 0, 0, 0);
			dtCurrDay += dtOneDay;
		}

		nDayHeaderBottom = DrawDayHeader(pDC, x, starty, boxwidth, boxheight, strDayText, TRUE, FALSE);
	}

	dtCurrDay = dtActiveDate;

	starty = nDayHeaderBottom;

	// Show all the days  (nRowCount rows, seven columns)
	for (int y = starty; y < starty + boxheight*nRowCount; y += boxheight) {
		for (int x = startx; x < startx + boxwidth * 7; x += boxwidth) {
			// Generate this day's rect
			CRect rc(x, y, x + boxwidth + 1, y + boxheight + 1);

			//IsTheDayGray?
			bIsDayActive = dtCurrDay.GetMonth() == nActiveMonth;

			// Draw day rect
			{
				if (bStartDate && !bIsDayActive) {
					// Nope, so color it grey and make sure no events are printed
					pOldBrush = pDC->SelectObject(&greybrush);
					pDC->SetBkColor(RGB(192, 192, 192));
					pastrNames = NULL;
				}
				else {
					// Is this day part of our schedule?
					if (CurrDay > 0 && CurrDay <= m_nMedSchedDays)
						//Yep, so get a pointer to the beginning of this day's array
						pastrNames = &m_astrMedSchedEntries[CurrDay];
					else
						pastrNames = NULL;
				}

				// Plot rectangle that represents this day
				pDC->Rectangle(&rc);
			}

			// Draw day number
			{
				// Generate the day number text
				strDayText = " ";

				if (bStartDate) {
					if (dtActiveDate.GetDay() == 1 || (x == startx && y == starty)) {
						// This is the first VISIBLE day of the given month so
						// prepend the month's common name to the day value
						strDayText.Format("%s %d", month_names[dtActiveDate.GetMonth() - 1], dtActiveDate.GetDay());
					}
					else {
						// This is a regular day so just use the day number
						strDayText.Format("%d", dtActiveDate.GetDay());
					}
					COleDateTimeSpan dtOneDay;
					dtOneDay.SetDateTimeSpan(1, 0, 0, 0);
					dtActiveDate += dtOneDay;
				}

				// Draw the day header
				nDayHeaderBottom = DrawDayNumber(pDC, x, y, boxwidth, boxheight, strDayText, TRUE, FALSE);
			}


			// Print day entries
			{
				if (pastrNames != NULL) {
					// Decide where the first appointment will display
					int text_y = nDayHeaderBottom;

					// Decide which entry to display first
					int nStart = 0; //m_nVisibleNamesOffset;
					int nMaxStart = pastrNames->GetSize() - 5; //m_nVisibleNamesPerDay;
					if (nMaxStart < 0) nMaxStart = 0;
					if (nStart > nMaxStart) {
						nStart = nMaxStart;
					}

					if (nStart > 0) {
						// If we're starting anywhere but the first entry, 
						// calculate the total height of all entries and if 
						// they'll fit then start at 0
						if (DoesDayOutputFit(pDC, pastrNames, boxwidth - 20, rc.Height() - textheight)) {
							// The output will fit
							nStart = 0;
						}
					}

					// If we are starting anywhere but the beginning 
					// of the list, put three dots at the right
					if (nStart > 0) {
						pDC->SetTextColor(RGB(0, 0, 0));
						pDC->TextOut(x + boxwidth - 110, y + 50, "***");
					}
					for (int i = nStart; i < pastrNames->GetSize() && text_y <= rc.bottom - textheight; i++) {
						// Set color of text
						COLORREF clr;
						if (bBlackAndWhite) {
							// Use black
							clr = RGB(0, 0, 0);
						}
						else {
							// Use purposeset color
							clr = (COLORREF)m_adwMedSchedColors[CurrDay].GetAt(i);
							// Tone down if too bright
							// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
							clr = GetDarkerColorForApptText(clr);
						}
						pDC->SetTextColor(clr);
						text_y += pDC->DrawText(pastrNames->GetAt(i),
							CRect(x + 20, text_y, rc.right, rc.bottom),
							DT_WORDBREAK | DT_TOP | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS);
					}
					if (i < pastrNames->GetSize()) {
						pDC->SetTextColor(RGB(0, 0, 0));
						pDC->TextOut(x + boxwidth - 110, y + boxheight - textheight + 50, "***");
					}
				}
			}


			// Default back to white if greyed
			if (dtCurrDay.GetMonth() != nActiveMonth) {
				pDC->SelectObject(pOldBrush);
				pDC->SetBkColor(RGB(255, 255, 255));
			}

			// Move to next day
			dtCurrDay += COleDateTimeSpan(1, 0, 0, 0);

			CurrDay++;
		}
	}
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldFont);

	if (m_bToggleEndMedSchedPrinting) {
		m_bIsMedSchedPrinting = FALSE;
		m_bToggleEndMedSchedPrinting = FALSE;
	}
}

//TES 3/31/2008 - PLID 29485 - This function has never NOT been dead code.  It was checked in with all the actual
// drawing code commented out in 2002, then in 2003, all that commented-out code was removed, leaving a bunch of code
// that didn't actually do anything, and there it's remained ever since.  This is surely at least a minimal speed hit,
// so I'm taking it out.
/*void CMedicationDlg::OnPaint()
{
char* month_names[12] = { "January", "February", "March",
"April", "May", "June", "July", "August", "September",
"October", "November", "December" };
CPaintDC dc(this); // device context for painting
CBrush greenbrush(RGB(192, 255, 192));
CBrush greybrush(RGB(192, 192, 192));
CBrush redbrush(RGB(255, 64, 64));
CBrush whitebrush(RGB(255, 255, 255));
CBrush* pOldBrush = dc.SelectObject(&whitebrush);
CPen pen(PS_SOLID, 1, RGB(0,0,0));
CPen* pOldPen = dc.SelectObject(&pen);
dc.SetTextColor(RGB(0,0,0));
CFont* pOldFont;
CFont font;
font.CreatePointFont(80, "MS Serif");
pOldFont = dc.SelectObject(&font);

COleDateTime date;
//GetActiveDate(date);
int activemonth = date.GetMonth();
date -= COleDateTimeSpan(date.GetDay()-1,0,0,0);
int weekday = date.GetDayOfWeek();
date -= COleDateTimeSpan(weekday-1, 0,0,0);

CString strRes = VarString("");

// Set this to FALSE, and then if any visible day doesn't
// paint all its appointments, this will be set back to TRUE
BOOL bAllowScrollDown = FALSE;


// Draw event names
dc.SetBkMode(TRANSPARENT);
dc.SetTextColor(RGB(255,255,255));

dc.SetBkMode(OPAQUE);

dc.SelectObject(pOldPen);
dc.SelectObject(pOldBrush);
dc.SelectObject(pOldFont);
}*/

// Returns the bottom of the drawn header (i.e. where anything else 
// could start drawing safely without overlapping the header area)
long CMedicationDlg::DrawPageHeader(CDC *pDC, LPRECT rcPage, long nProcId)
{
	// Calculate the header text
	CString strHdr;
	CString strProcType = "";

	// See if we are displaying a particular procedure
	if (nProcId != -1) {

		// Create the header text
		_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ProcedureT WHERE ID = %li", nProcId);
		if (!rs->eof) {
			CString name = CString(rs->Fields->Item["Name"]->Value.bstrVal);
			strProcType.Format("%s ", name);
		}
		rs->Close();
	}

	// Create the header text
	_RecordsetPtr rs = CreateRecordset("SELECT First + ' ' + Last AS Name FROM PersonT WHERE ID = %li", GetActivePatientID());
	if (!rs->eof) {
		CString name = CString(rs->Fields->Item["Name"]->Value.bstrVal);
		strHdr.Format("%sMedication Schedule for %s", strProcType, name);
	}
	else
		strHdr.Format("%sMedication Schedule", strProcType);
	rs->Close();

	// Create and select an appropriate font
	CFont fontHdr, *pOldFont;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&fontHdr, 240, "Arial", pDC);
	pOldFont = pDC->SelectObject(&fontHdr);
	// Set header text color
	pDC->SetTextColor(RGB(0, 0, 0));
	// Determine the header area
	const int nHeaderTop = 80;
	CRect rcHeader(rcPage);
	rcHeader.DeflateRect(0, nHeaderTop, 0, 0);
	// Draw the header, storing its height
	long nHeight = nHeaderTop + pDC->DrawText(strHdr, rcHeader, DT_TOP |/*DT_SINGLELINE|*/DT_NOPREFIX | DT_CENTER);
	// Unselect the header font
	pDC->SelectObject(pOldFont);

	// Return the bottom of the drawn header (i.e. where anything else 
	// could start drawing safely without overlapping the header area)
	return rcHeader.top + nHeight;
}

// Returns the bottom of the drawn header (i.e. where anything else 
// could start drawing safely without overlapping the header area)
long CMedicationDlg::DrawDayHeader(CDC *pDC, long x, long y, long nBoxWidth, long nBoxHeight, LPCTSTR strDayText, BOOL bDrawBorder, BOOL bDrawGrey)
{
	// Set up the colors appropriately
	pDC->SetTextColor(RGB(0, 0, 0));
	if (bDrawGrey)
		pDC->SetBkColor(RGB(192, 192, 192));
	CBrush greyBrush(RGB(192, 192, 192)), whiteBrush(RGB(255, 255, 255)), *pOldBrush;
	if (bDrawGrey)
		pOldBrush = pDC->SelectObject(&greyBrush);
	else
		pOldBrush = pDC->SelectObject(&whiteBrush);

	// Margin around the header
	const long mx = 10, my = 4;

	// Set up the text rect for the day header and calc the size of the text
	CRect rcDayHeaderText(x + mx, y + my, x + nBoxWidth - mx, y + nBoxHeight - my);
	pDC->DrawText(strDayText, rcDayHeaderText, DT_CALCRECT | DT_TOP | DT_NOPREFIX | DT_LEFT);

	rcDayHeaderText.left += mx;
	rcDayHeaderText.right += mx;

	// Set up the rect for the entire day header area and color it grey
	CRect rcDayHeaderBox(x, y, x + nBoxWidth + 1, y + (nBoxHeight / 5) + 1);
	if (bDrawBorder)
		pDC->Rectangle(rcDayHeaderBox);

	// Actually draw the text in the day header area
	pDC->DrawText(strDayText, rcDayHeaderText, DT_TOP | DT_NOPREFIX | DT_LEFT);

	// We're done with the brush
	pDC->SelectObject(pOldBrush);

	// Return the bottom of the drawn header (i.e. where anything else 
	// could start drawing safely without overlapping the header area)
	return rcDayHeaderBox.bottom;
}

// Returns the bottom of the drawn header (i.e. where anything else 
// could start drawing safely without overlapping the header area)
long CMedicationDlg::DrawDayNumber(CDC *pDC, long x, long y, long nBoxWidth, long nBoxHeight, LPCTSTR strDayText, BOOL bDrawBorder, BOOL bDrawGrey)
{
	// Set up the colors appropriately
	pDC->SetTextColor(RGB(0, 0, 0));
	if (bDrawGrey)
		pDC->SetBkColor(RGB(192, 192, 192));
	CBrush greyBrush(RGB(192, 192, 192)), whiteBrush(RGB(255, 255, 255)), *pOldBrush;
	if (bDrawGrey)
		pOldBrush = pDC->SelectObject(&greyBrush);
	else
		pOldBrush = pDC->SelectObject(&whiteBrush);

	// Margin around the header
	const long mx = 10, my = 4;

	// Set up the text rect for the day header and calc the size of the text
	CRect rcDayNumberText(x + mx, y + my, x + nBoxWidth - mx, y + nBoxHeight - my);
	pDC->DrawText(strDayText, rcDayNumberText, DT_CALCRECT | DT_TOP | DT_NOPREFIX | DT_LEFT);

	rcDayNumberText.left += mx;
	rcDayNumberText.right += mx;

	const long defRight = x + (nBoxWidth / 5), defBottom = y + (nBoxHeight / 5);

	long boxRight, boxBottom;
	if (rcDayNumberText.right + mx < defRight)
		boxRight = defRight;
	else
		boxRight = rcDayNumberText.right + mx;
	if (rcDayNumberText.bottom + my < defBottom)
		boxBottom = defBottom;
	else
		boxBottom = rcDayNumberText.bottom + my;

	CRect rcDayNumberBox(x, y, boxRight, boxBottom);
	if (bDrawBorder)
		pDC->Rectangle(rcDayNumberBox);

	// Actually draw the text in the day header area
	pDC->DrawText(strDayText, rcDayNumberText, DT_TOP | DT_NOPREFIX | DT_LEFT);

	// We're done with the brush
	pDC->SelectObject(pOldBrush);

	// Return the bottom of the drawn header (i.e. where anything else 
	// could start drawing safely without overlapping the header area)
	return rcDayNumberBox.bottom;
}

void CMedicationDlg::LoadMedSchedPrintInfo() {

	try {

		for (int i = 0; i <= 366; i++) {
			m_astrMedSchedEntries[i].RemoveAll();
			m_adwMedSchedColors[i].RemoveAll();
			m_nMedSchedDays = 0;
		}

		if (m_tempMedSchedID == -1)
			return;

		_RecordsetPtr rs = CreateRecordset("SELECT * FROM MedSchedDetailsT WHERE MedSchedID = %li ORDER BY Priority", m_tempMedSchedID);

		_variant_t var;

		while (!rs->eof) {
			//load the start day, the type, start note, and color, as all items have these
			long StartDay;
			StartDay = rs->Fields->Item["StartDay"]->Value.lVal;
			int Type = rs->Fields->Item["Type"]->Value.iVal;

			CString StartNote;
			var = rs->Fields->Item["StartNote"]->Value;
			if (var.vt == VT_BSTR) {
				StartNote = CString(var.bstrVal);
			}

			DWORD color;
			var = rs->Fields->Item["Color"]->Value;
			if (var.vt == VT_I4) {
				color = var.lVal;
			}

			if (StartDay > 366) {
				rs->MoveNext();
				continue;
			}

			//add the start note and color on the start day
			m_astrMedSchedEntries[StartDay].Add(StartNote);
			m_adwMedSchedColors[StartDay].Add(color);

			if (m_nMedSchedDays < StartDay)
				m_nMedSchedDays = StartDay;

			//if a Medication, we have much more work to do
			if (Type == 1) {
				int DurationType = 1;
				var = rs->Fields->Item["DurationType"]->Value;
				if (var.vt == VT_UI1) {
					DurationType = var.iVal;
				}

				int StopMethod = 1;
				var = rs->Fields->Item["StopMethod"]->Value;
				if (var.vt == VT_UI1) {
					StopMethod = var.iVal;
				}

				CString MiddleNote;
				var = rs->Fields->Item["MiddleNote"]->Value;
				if (var.vt == VT_BSTR) {
					MiddleNote = CString(var.bstrVal);
				}

				CString StopNote;
				var = rs->Fields->Item["StopNote"]->Value;
				if (var.vt == VT_BSTR) {
					StopNote = CString(var.bstrVal);
				}

				long StopDay = StartDay;
				var = rs->Fields->Item["StopDay"]->Value;
				if (var.vt == VT_I4) {
					StopDay = var.lVal;
				}

				//if DurationType = 2, we are going to run for "StopDay" days
				if (DurationType == 2)
					StopDay = StartDay + StopDay - 1;

				//if StopMethod = 2, we will place the stop note on the day after the official "Stop Day"
				if (StopMethod == 2)
					StopDay++;

				if (StopDay > 366)
					StopDay = 366;

				if (m_nMedSchedDays < StopDay)
					m_nMedSchedDays = StopDay;


				for (int i = StartDay + 1; i < StopDay; i++) {
					m_astrMedSchedEntries[i].Add(MiddleNote);
					m_adwMedSchedColors[i].Add(color);
				}

				//don't display twice if only one day
				if (StopDay > StartDay) {
					m_astrMedSchedEntries[StopDay].Add(StopNote);
					m_adwMedSchedColors[StopDay].Add(color);
				}
			}

			rs->MoveNext();
		}
		rs->Close();

		bMedSchedPrintInfoLoaded = TRUE;

	}NxCatchAll("Error generating Medication Schedule.");
}

BOOL CMedicationDlg::DoesDayOutputFit(CDC *pDC, CStringArray *pastrNames, long nBoxWidth, long nBoxHeight)
{
	if (pastrNames) {
		long y = 0;
		// Loop until we've extended past box height
		for (int i = 0; i < pastrNames->GetSize() && y <= nBoxHeight; i++) {
			// Don't actually draw the text, just calculate
			y += pDC->DrawText((pastrNames->GetAt(i)),
				CRect(0, 0, nBoxWidth, nBoxHeight),
				DT_CALCRECT | DT_WORDBREAK | DT_TOP | DT_LEFT | DT_NOPREFIX | DT_EXPANDTABS);
		}
		if (y > nBoxHeight) {
			// We extended past box height so "No, the day output doesn't fit"
			return FALSE;
		}
		else {
			// We extended past box height so "Yes, the day output does fit"
			return TRUE;
		}
	}
	else {
		// We got a NULL pastrNames.  That shouldn't happen
		ASSERT(FALSE);
		return FALSE;
	}
}

void CMedicationDlg::OnAddMedsched()
{
	if (!CheckCurrentUserPermissions(bioPatientMedication, sptCreate))
		return;

	CMedSchedule dlg(this);
	dlg.DoModal();

	m_tempMedSchedID = dlg.m_MedSchedID;
	bMedSchedPrintInfoLoaded = FALSE;

	if (dlg.m_bPrintOnClose) {

		m_bIsMedSchedPrinting = TRUE;

		//CNxTabView *pView = (CNxTabView*)GetMainFrame()->GetActiveView();
		//pView->OnFilePrintPreview();

		CPatientView *pView = (CPatientView*)GetMainFrame()->GetActiveView();
		if (pView) {
			pView->OnFilePrintPreview();
		}
	}

	EnableAppropriateButtons();
	UpdateView();
}

void CMedicationDlg::OnDeleteMedsched()
{
	if (!CheckCurrentUserPermissions(bioPatientMedication, sptDelete))
		return;

	try {

		long curSel = m_NxDlMedSchedules->GetCurSel();

		if (curSel == -1)
			return;

		_variant_t var = m_NxDlMedSchedules->GetValue(curSel, 0);

		CString strOld = CString(m_NxDlMedSchedules->GetValue(curSel, 1).bstrVal);

		if (IDYES == MessageBox("This will permanently delete this schedule! Are you sure you wish to continue?", "Practice", MB_ICONEXCLAMATION | MB_YESNO)) {

			ExecuteSql("DELETE FROM MedSchedDetailsT WHERE MedSchedID = %li", var.lVal);
			ExecuteSql("DELETE FROM MedScheduleT WHERE ID = %li", var.lVal);
		}

		m_NxDlMedSchedules->Requery();

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if (nAuditID != -1)
			AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientMedSchedDelete, GetActivePatientID(), strOld, "<Deleted>", aepMedium, aetDeleted);

		EnableAppropriateButtons();

	} NxCatchAll("Error deleting medication schedule.");
}

void CMedicationDlg::OnRButtonDownNxdlmedschedules(long nRow, short nCol, long x, long y, long nFlags)
{
	CMenu  menPopup;
	menPopup.m_hMenu = CreatePopupMenu();

	menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_ADD_SCHED, "Add");

	if (nRow != -1) {
		m_nMedSchedSelItem = nRow;
		m_NxDlMedSchedules->PutCurSel(nRow);

		// m.carlson 4/15/2005
		// Add the menu items (appropriately disabled according to user's permissions)
		if (!(GetCurrentUserPermissions(bioPatientMedication) & SPT___W________ANDPASS))
			menPopup.InsertMenu(-1, MF_BYPOSITION | MF_GRAYED, IDM_EDIT_SCHED, "Edit");
		else
			menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_EDIT_SCHED, "Edit");

		if (!(GetCurrentUserPermissions(bioPatientMedication) & SPT_____D______ANDPASS))
			menPopup.InsertMenu(-1, MF_BYPOSITION | MF_GRAYED, IDM_DEL_SCHED, "Delete");
		else
			menPopup.InsertMenu(-1, MF_BYPOSITION, IDM_DEL_SCHED, "Delete");
	}

	CPoint pt;
	pt.x = x;
	pt.y = y;
	GetCursorPos(&pt);
	menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
}

void CMedicationDlg::OnDblClickCellNxdlmedschedules(long nRowIndex, short nColIndex)
{
	if (nRowIndex == -1)
		return;

	_variant_t var = m_NxDlMedSchedules->GetValue(nRowIndex, 0);

	OnEditMedSchedule(var.lVal);
}

void CMedicationDlg::OnEditMedSchedule(long MedSchedID) {

	if (!CheckCurrentUserPermissions(bioPatientMedication, sptWrite))
		return;

	CMedSchedule dlg(this);
	dlg.m_MedSchedID = MedSchedID;
	dlg.DoModal();

	m_tempMedSchedID = dlg.m_MedSchedID;
	bMedSchedPrintInfoLoaded = FALSE;

	if (dlg.m_bPrintOnClose) {

		m_bIsMedSchedPrinting = TRUE;

		//CNxTabView *pView = (CNxTabView*)GetMainFrame()->GetActiveView();
		//pView->OnFilePrintPreview();

		CPatientView *pView = (CPatientView*)GetMainFrame()->GetActiveView();
		// (j.jones 2012-08-08 10:11) - PLID 51063 - added check that the active view is non-null
		if (pView) {
			pView->OnFilePrintPreview();
		}
	}

	UpdateView();
}

void CMedicationDlg::SecureControls()
{
	// Return if we have write access

	BOOL bCreate = GetCurrentUserPermissions(bioPatientMedication) & SPT____C_______ANDPASS;
	BOOL bDelete = GetCurrentUserPermissions(bioPatientMedication) & SPT_____D______ANDPASS;

	// (a.walling 2006-11-27 14:45) - PLID 20179 - Seperate Allergy permissions from Medications
	BOOL bAllergyCreate = GetCurrentUserPermissions(bioPatientAllergies) & SPT____C_______ANDPASS;
	BOOL bAllergyDelete = GetCurrentUserPermissions(bioPatientAllergies) & SPT_____D______ANDPASS;
	BOOL bAllergyWrite = GetCurrentUserPermissions(bioPatientAllergies) & SPT___W________ANDPASS;

	// (j.jones 2008-11-25 10:58) - PLID 28508 - added allergy review permission
	BOOL bAllergyReview = GetCurrentUserPermissions(bioPatientAllergyReview) & SPT___W________ANDPASS;

	// (r.gonet 09/19/2013) - PLID 58416 - Get the permission to be able to read medication history.
	BOOL bMedicationHistoryRead = GetCurrentUserPermissions(bioPatientMedicationHistory) & SPT__R_________ANDPASS;

	// Make the controls read-only
	GetDlgItem(IDC_ADD_MEDSCHED)->EnableWindow(bCreate);
	GetDlgItem(IDC_DELETE_MEDSCHED)->EnableWindow(bDelete);

	GetDlgItem(IDC_ALLERGYWARNING)->EnableWindow(bAllergyWrite);

	GetDlgItem(IDC_CHECK_REVIEWED_ALLERGIES)->EnableWindow(bAllergyReview);

	// (r.gonet 09/19/2013) - PLID 58416 - Disable the med history button if we can't read med history.
	GetDlgItem(IDC_MEDICATION_HISTORY_BTN)->EnableWindow(bMedicationHistoryRead);

	// (j.jones 2012-10-16 14:35) - PLID 53179 - added "no known allergies" checkbox, uses the allergy create permission
	m_checkHasNoAllergies.EnableWindow(bAllergyCreate);

	// (b.savon 2013-06-03 11:47) - PLID 56979

	// (j.jones 2012-10-17 11:06) - PLID 51713 - added "no current meds." checkbox, uses the med. create permission
	m_checkHasNoMeds.EnableWindow((BOOL)(GetCurrentUserPermissions(bioPatientCurrentMeds) & SPT____C_______ANDPASS));
}

void CMedicationDlg::OnEditingFinishingNxdlpatallergies(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue)
{
	// (a.walling 2006-11-27 14:47) - PLID 20179 - Prevent editing of notes if Write is disabled
	if (!CheckCurrentUserPermissions(bioPatientAllergies, sptWrite)) {
		// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
		VariantClear(pvarNewValue);
		*pvarNewValue = _variant_t(varOldValue).Detach();
		*pbCommit = FALSE;
		return;
	}
}

void CMedicationDlg::OnWarn()
{
	try {
		// (a.walling 2006-11-27 15:01) - PLID 20179 - Return if they do not have permission to set the warning on/off
		if (!CheckCurrentUserPermissions(bioPatientAllergies, sptWrite)) {
			// the button has already been checked, but they don't have permission. So we need to toggle it again.
			CheckDlgButton(IDC_ALLERGYWARNING, !IsDlgButtonChecked(IDC_ALLERGYWARNING));
			return;
		}

		//update the data
		ExecuteSql("Update PersonT SET DisplayAllergyWarning = %li WHERE ID = %li", IsDlgButtonChecked(IDC_ALLERGYWARNING), GetActivePatientID());

	}NxCatchAll("Error Setting Warning");
}

void CMedicationDlg::EnableAppropriateButtons()
{
	// m.carlson 4/15/2005
	// If they have no permission to delete items, we should return immediately
	// because the entire purpose of this function is to enable / disable the
	// delete button according to whether the lists are blank.

	// (d.thompson 2010-01-13) - PLID 30376 - Current meds are now their own permission.  Also fixed this
	//	function which wasn't operating correctly if deleting meds was off (allergies would never be touched)

	if ((GetCurrentUserPermissions(bioPatientMedication) & SPT_____D______ANDPASS)) {
		if (m_NxDlMedSchedules->GetCurSel() == sriNoRow){
			GetDlgItem(IDC_DELETE_MEDSCHED)->EnableWindow(FALSE);
		}
		else{
			GetDlgItem(IDC_DELETE_MEDSCHED)->EnableWindow(TRUE);
		}
	}

	//(j.fouts 2013-03-19) - PLID 54468 - Hide this if they don't have the license
	// (j.fouts 2013-05-30 09:47) - PLID 56807 - Drug Interactions is now tied to NexERx
	if (g_pLicense->HasEPrescribing(CLicense::cflrSilent) != CLicense::eptSureScripts)
	{
		m_btnInteractions.EnableWindow(FALSE);
		m_btnInteractions.ShowWindow(SW_HIDE);
		// (r.gonet 09/20/2013) - PLID 58416 - Hide the med history button
		m_btnMedicationHistory.EnableWindow(FALSE);
		m_btnMedicationHistory.ShowWindow(SW_HIDE);
	}
}

void CMedicationDlg::OnSelChangedCurrentMedList(long nNewSel)
{
	EnableAppropriateButtons();

}

void CMedicationDlg::OnSelChangedNxdlpatallergies(long nNewSel)
{
	EnableAppropriateButtons();

}

void CMedicationDlg::OnSelChangedNxdlmedschedules(long nNewSel)
{
	EnableAppropriateButtons();

}

LRESULT CMedicationDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	// (c.haag 2007-01-25 11:56) - PLID 24396 - Refresh the view if the
	// patient medications table changed
	try {
		//(j.jones 2008-11-25 11:42) - PLID 32183 - make sure we check the patient ID
		if (NetUtils::CurrentPatientMedsT == wParam && m_id == (long)lParam) {
			//check the local checker too, because we want to ignore our own messages
			if (m_CurrentMedsChecker.Changed()) {
				UpdateView();
			}
		}
		// (c.haag 2007-04-06 09:45) - PLID 25525 - Refresh the view if
		// the patient allergies table changed
		// (j.jones 2008-11-25 11:42) - PLID 32183 - make sure we check the patient ID
		if (NetUtils::PatientAllergyT == wParam && m_id == (long)lParam) {
			//check the local checker too, because we want to ignore our own messages
			if (m_PatientAllergiesChecker.Changed()) {
				UpdateView();
			}
		}
		if (NetUtils::MedicationHistoryResponseT == wParam && m_id == (long)lParam) {
			// (r.gonet 09/20/2013) - PLID 58396 - The patient has medication history, set the
			// font color of the med history button to red to indicate this.
			if (m_MedicationHistoryResponseChecker.Changed()) {
				m_btnMedicationHistory.SetTextColor(RGB(255, 0, 0));
			}
		}
	}
	NxCatchAll("Error in CMedicationDlg::OnTableChanged");
	return 0;
}

// (j.fouts 2012-11-14 11:42) - PLID 53439 -- Commenting this out for now. We will likely move this to the queue for consistency
//// (j.jones 2008-05-14 11:07) - PLID 29732 - now prescriptions are edited in their own dialog
//void CMedicationDlg::OnLeftClickNxdlhistory(long nRow, short nCol, long x, long y, long nFlags) 
//{
//	try {
//
//		if(nRow == -1 || nCol != COLUMN_NAME) {
//			return;
//		}
//
//		long nPrescriptionID = VarLong(m_NxDlHistory->GetValue(nRow, COLUMN_PRESC_ID), -1);
//
//		if(nPrescriptionID == -1) {
//			return;
//		}
//
//		//check for write permissions, if they don't have any, it will open as read-only
//		BOOL bHasWritePermissions = FALSE;
//		if((GetCurrentUserPermissions(bioPatientMedication) & sptWrite) ||
//			((GetCurrentUserPermissions(bioPatientMedication) & sptWriteWithPass)
//			&& CheckCurrentUserPassword())) {
//
//			//they have write permissions
//			bHasWritePermissions = TRUE;
//		}
//		else {
//			AfxMessageBox("You do not have permission to edit prescriptions. The prescription will be opened for viewing only.");
//		}
//
//		// (j.gruber 2009-03-30 09:55) - PLID 33728 - check to see if there is a newcrop guid
//		CString strNewCropGUID = VarString(m_NxDlHistory->GetValue(nRow, COLUMN_NEWCROP_GUID), "");
//		if (!strNewCropGUID.IsEmpty()) {
//			//make it read only
//			bHasWritePermissions = FALSE;
//		}
//
//		//the dialog will disable its controls if the user doesn't have write
//		//permissions, don't need to check read permissions because otherwise
//		//the user wouldn't be able to get into this tab
//
//		CPrescriptionEditDlg dlg(this);
//		// (j.jones 2008-10-13 10:09) - PLID 17235 - this function now requires a patient ID
//		if(IDOK == dlg.EditPrescription(nPrescriptionID, m_id, !bHasWritePermissions)) {
//			//reload the list
//			m_NxDlHistory->Requery();
//		}
//
//		// (j.jones 2012-10-29 11:28) - PLID 53454 - show drug interactions, if any (the queue could have edited any prescription)
//		CheckInteractions();
//
//	}NxCatchAll("Error in CMedicationDlg::OnLeftClickNxdlhistory");	
//}

// (j.jones 2008-11-25 08:50) - PLID 28508 - added OnCheckReviewedAllergies
void CMedicationDlg::OnCheckReviewedAllergies()
{
	try {

		BOOL bReviewedAllergies = m_checkReviewedAllergies.GetCheck();

		if (!CheckCurrentUserPermissions(bioPatientAllergyReview, sptWrite)) {
			//reverse the checkbox
			m_checkReviewedAllergies.SetCheck(!bReviewedAllergies);
			return;
		}

		//UpdateAllergyReviewStatus will handle the data change, interface change, and auditing
		UpdateAllergyReviewStatus(bReviewedAllergies);

		//send a tablechecker
		m_PatientAllergiesChecker.Refresh(m_id);

	}NxCatchAll("Error in CMedicationDlg::OnCheckReviewedAllergies");
}

// (j.jones 2008-11-25 09:22) - PLID 28508 - UpdateAllergyReviewLabel is a modular function that
// will calculate the label for the allergy review checkbox, and apply it
void CMedicationDlg::UpdateAllergyReviewLabel(BOOL bReviewedAllergies, COleDateTime dtReviewedOn, CString strUserName)
{
	//throw exceptions to the caller

	CString strLabel;

	if (bReviewedAllergies) {
		strLabel.Format("Allergy information has been reviewed by %s on %s.", strUserName, FormatDateTimeForInterface(dtReviewedOn, DTF_STRIP_SECONDS, dtoDateTime));
	}
	else {
		strLabel = "Have the allergies been reviewed with the patient?";
	}

	//NxDialog annoyingly likes to resize this checkbox based on its text,
	//so we need to force it to be the same width as the allergy datalist
	CRect rcList, rcCheck;
	GetDlgItem(IDC_NXDLPATALLERGIES)->GetWindowRect(rcList);
	GetDlgItem(IDC_CHECK_REVIEWED_ALLERGIES)->GetWindowRect(rcCheck);
	ScreenToClient(rcList);
	ScreenToClient(rcCheck);
	rcCheck.left = rcList.left;
	rcCheck.right = rcList.right;
	GetDlgItem(IDC_CHECK_REVIEWED_ALLERGIES)->MoveWindow(rcCheck, TRUE);

	m_checkReviewedAllergies.SetWindowText(strLabel);
}

// (j.jones 2008-11-25 09:50) - PLID 28508 - UpdateAllergyReviewStatus can be called
// by a permissioned user when checking/unchecking the allergy review checkbox, or
// can be called by any user if they change allergy information
void CMedicationDlg::UpdateAllergyReviewStatus(BOOL bReviewedAllergies)
{
	//throw exceptions to the caller

	//UpdatePatientAllergyReviewStatus will save the status change and audit
	COleDateTime dtReviewedOn = UpdatePatientAllergyReviewStatus(m_id, bReviewedAllergies);

	//update the label
	UpdateAllergyReviewLabel(bReviewedAllergies, dtReviewedOn, GetCurrentUserName());
	CString strLabel;
	m_checkReviewedAllergies.GetWindowText(strLabel);
	m_checkReviewedAllergies.SetCheck(bReviewedAllergies);

	//do not send a tablechecker, the caller should be responsible for that
}

LRESULT CMedicationDlg::OnNewCropBrowserClosed(WPARAM wParam, LPARAM lParam)
{
	try {
		// (c.haag 2009-05-13 16:18) - PLID 34257 - wParam now holds a pointer to a structure
		NewCropBrowserResult* pNCBR = (NewCropBrowserResult*)wParam;

		if (pNCBR->nPatientID == m_id) {

			// (j.gruber 2009-03-30 09:36) - PLID 33728 - we'll need to requery the list since we are syncing medications back into Practice now
			/*m_NxDlHistory->Requery();

			// (j.gruber 2009-05-13 10:26) - PLID 34251 - we need to requery the allergy list now too
			m_pPatAllergies->Requery();

			// (j.gruber 2009-05-14 11:22) - PLID 34259 - requery the current meds list
			m_CurMedList->Requery();*/

			// (c.haag 2010-02-18 09:57) - PLID 37424 - Let the user add newly added prescriptions to the current medication list
			CStringArray astrNewCropRxGUIDs;
			for (int i = 0; i < pNCBR->aNewlyAddedPatientPrescriptions.GetSize(); i++) {
				astrNewCropRxGUIDs.Add(pNCBR->aNewlyAddedPatientPrescriptions[i].strPrescriptionGUID);
			}
			// (j.jones 2013-01-09 11:55) - PLID 54530 - renamed the function, it also now takes in an
			// optional prescription ID list, but it is not used here
			if (astrNewCropRxGUIDs.GetSize() > 0) {
				CArray<long, long> aryNewPrescriptionIDs;
				CDWordArray arNewCDSInterventions;
				//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
				ReconcileCurrentMedicationsWithMultipleNewPrescriptions(m_id, aryNewPrescriptionIDs, astrNewCropRxGUIDs, m_bkg.GetColor(), this, arNewCDSInterventions);
				GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
			}

			// (j.gruber 2009-06-12 10:05) - PLID 34251 - we now need to update the review box for allergies too, so let's just reload the whole tab
			UpdateView();
		}
		else {
			ASSERT(FALSE);
		}

		// (c.haag 2009-05-13 16:20) - PLID 34257 - Needs to be deleted
		delete pNCBR;

	}NxCatchAll("Error in CMedicationDlg::OnNewCropBrowserClosed");

	return 0;
}

// (j.gruber 2009-05-13 12:42) - PLID 34251 - sync back allergies
void CMedicationDlg::RequeryFinishedNxdlpatallergies(short nFlags)
{
	try {
		//we don't need to check for the license because maybe they had newcrop and ditched it, they'll still need to see the prescriptions
		long p = m_pPatAllergies->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		while (p) {
			m_pPatAllergies->GetNextRowEnum(&p, &lpDisp);
			NXDATALISTLib::IRowSettingsPtr pRow(lpDisp); lpDisp->Release();

			BOOL bFromNewCrop = VarBool(pRow->GetValue(alcFromNewCrop), FALSE);

			// (b.savon 2013-01-25 12:58) - PLID 54854 - Rework this to allow discontinued for our erx
			BOOL bIsDiscoed = VarBool(pRow->GetValue(alcIsDiscontinued), FALSE);
			long nFromFDB = VarLong(pRow->GetValue(alcFDBImport), -1);
			if (bIsDiscoed) {
				// (j.gruber 2009-05-19 14:40) - PLID 34291 - change the color
				pRow->PutBackColor(EPRESCRIBE_COLOR_DISCONTINUED);
			}
			else{
				if (bFromNewCrop) {
					// (j.gruber 2009-05-19 14:40) - PLID 34291 - change the color
					pRow->PutBackColor(EPRESCRIBE_COLOR_ACTIVE);
				}
				else if (nFromFDB > 0){ // (b.savon 2013-01-07 13:18) - PLID 54459 - Color imported meds
					//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
					if (VarBool(pRow->GetValue(alcFDBOutOfDate), 0)) {
						pRow->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
					}
					else {
						pRow->PutBackColor(ERX_IMPORTED_COLOR);
					}
				}
			}
		}
	}NxCatchAll("Error in CMedicationDlg::RequeryFinishedNxdlpatallergies");

}

// (j.gruber 2009-05-14 13:29) - PLID 34259 - colorize list
void CMedicationDlg::RequeryFinishedCurrentMedList(short nFlags)
{
	try {

		// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon
		_variant_t varInfoButtonIcon = (long)m_hInfoButtonIcon;

		//we don't need to check for the license because maybe they had newcrop and ditched it, they'll still need to see the prescriptions
		long p = m_CurMedList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;
		while (p) {
			m_CurMedList->GetNextRowEnum(&p, &lpDisp);
			NXDATALISTLib::IRowSettingsPtr pRow(lpDisp); lpDisp->Release();

			// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon
			pRow->PutValue(cmlcInfoButton, varInfoButtonIcon);

			CString strNewCropGUID = VarString(pRow->GetValue(cmlcNewCropGUID), "");

			// (b.savon 2013-01-25 12:58) - PLID 54854 - Rework this to allow discontinued for our erx
			BOOL bIsDiscoed = VarBool(pRow->GetValue(cmlcDiscontinued), FALSE);
			long nFromFDB = VarLong(pRow->GetValue(cmlcFDBID), -1);
			if (bIsDiscoed) {
				// (j.gruber 2009-05-19 14:40) - PLID 34291 - change the color
				pRow->PutBackColor(EPRESCRIBE_COLOR_DISCONTINUED);
			}
			else{
				if (!strNewCropGUID.IsEmpty()) {
					// (j.gruber 2009-05-19 14:40) - PLID 34291 - change the color
					pRow->PutBackColor(EPRESCRIBE_COLOR_ACTIVE);
				}
				else if (nFromFDB > 0){ // (b.savon 2013-01-07 13:18) - PLID 54459 - Color imported meds
					//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
					if (VarBool(pRow->GetValue(cmlcFDBOutOfDate), FALSE)) {
						pRow->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
					}
					else {
						pRow->PutBackColor(ERX_IMPORTED_COLOR);
					}
				}
			}
		}
	}NxCatchAll("Error in CMedicationDlg::RequeryFinishedCurrentMedList");
}

// (j.gruber 2009-06-02 17:26) - PLID 34449 - added inactive checkbox for medications
void CMedicationDlg::OnBnClickedMedHideInactive()
{
	try {
		//save our checkbox
		SetRemotePropertyInt("HideInactiveMedications", m_chkHideInactive.GetCheck(), 0, GetCurrentUserName());

		UpdateView();
	}NxCatchAll("Error in CMedicationDlg::OnBnClickedMedHideInactive()");
}

// (j.jones 2011-05-02 16:49) - PLID 43450 - supported editing the Sig
void CMedicationDlg::OnEditingStartingCurrentMedList(long nRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		if (nRow == -1) {
			return;
		}

		if (nCol == cmlcSig) {

			// (j.jones 2011-05-03 08:43) - PLID 43450 - disallow editing the sig if it's a NewCrop medication
			CString strNewCropGUID = VarString(m_CurMedList->GetValue(nRow, cmlcNewCropGUID), "");

			if (!strNewCropGUID.IsEmpty()) {
				//silently leave, the NewCrop meds are already colored and read-only elsewhere
				*pbContinue = FALSE;
				return;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-05-02 16:49) - PLID 43450 - supported editing the Sig
void CMedicationDlg::OnEditingFinishedCurrentMedList(long nRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		if (nRow == -1) {
			return;
		}

		if (bCommit) {

			// (s.dhole 2013-06-07 11:44) - PLID 56926
			switch (nCol){
			case cmlcSig:
			{
				CString strOldSig = VarString(varOldValue, "");
				CString strNewSig = VarString(varNewValue, "");
				if (strOldSig != strNewSig) {
					CString strDrugName = VarString(m_CurMedList->GetValue(nRow, cmlcMedName));
					// (s.dhole 2013-06-07 11:44) - PLID 56926
					UpdateCurrentMedRecord(nRow, cmlcSig, varNewValue);
					//audit
					long nAuditID = BeginNewAuditEvent();
					CString strPatientName = GetExistingPatientName(m_id);
					CString strOldAudit;
					strOldAudit.Format("Medication: %s, Sig: %s", strDrugName, strOldSig);
					AuditEvent(m_id, strPatientName, nAuditID, aeiCurrentMedicationSig, m_id, strOldAudit, strNewSig, aepHigh, aetChanged);

					//send a tablechecker
					m_CurrentMedsChecker.Refresh(m_id);
				}
			}
			break;
			case cmlcStartDate:
			{
				COleDateTime dtInvalid;
				dtInvalid.SetStatus(COleDateTime::invalid);
				COleDateTime dtNew = VarDateTime(varNewValue, dtInvalid);
				COleDateTime dtOld = VarDateTime(varOldValue, dtInvalid);
				if (dtNew.m_dt != dtOld.m_dt) {
					// (s.dhole 2013-06-07 11:44) - PLID 56926
					UpdateCurrentMedRecord(nRow, cmlcStartDate, varNewValue);
					CString strDrugName = VarString(m_CurMedList->GetValue(nRow, cmlcMedName));
					// (s.dhole 2013-06-18 14:38) - PLID 56927 audit changes
					long nAuditID = BeginNewAuditEvent();
					CString strPatientName = GetExistingPatientName(m_id);
					CString strOldAudit;
					if (dtOld.m_status == COleDateTime::invalid || (dtOld.m_dt < 1.0))
					{
						strOldAudit.Format("Medication: %s, Start Date: -", strDrugName);
					}
					else
					{
						strOldAudit.Format("Medication: %s, Start Date: %s", strDrugName, FormatDateTimeForInterface(dtOld));
					}

					CString strNewDate = FormatDateTimeForInterface(dtOld);
					if (dtNew.m_status == COleDateTime::invalid || (dtNew.m_dt < 1.0))
					{
						strNewDate.Format("%s", " - ");
					}
					else
					{
						strNewDate.Format("%s", FormatDateTimeForInterface(dtNew));
					}

					AuditEvent(m_id, strPatientName, nAuditID, aeiCurrentMedicationStartDate, m_id, strOldAudit, strNewDate, aepHigh, aetChanged);

					m_CurrentMedsChecker.Refresh(m_id);
				}
			}
			break;
			}

		}

	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-08-10 09:48) - PLID 52090 - Give the option to show a monograph when we right click a drug
void CMedicationDlg::RButtonUpCurrentMedList(long nRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		if (nRow != -1)
		{
			// (b.savon 2013-01-14 12:30) - PLID 54592
			// (j.fouts 2013-06-10 11:19) - PLID 56808 - This is tied to NexERx, not FDB
			BOOL bNexERxLicense = g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts;
			BOOL bPermission = CheckCurrentUserPermissions(bioPatientCurrentMeds, sptDelete);
			CMenu pMenu;
			pMenu.CreatePopupMenu();
			int nMenuIndex = 0;

			if (bPermission){
				m_CurMedList->CurSel = nRow;
				pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_DELETE_CURRENT_MED, "Delete");
				// (b.savon 2013-01-25 12:44) - PLID 54854
				if (!VarBool(m_CurMedList->GetValue(nRow, cmlcDiscontinued), FALSE)){
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_DISCONTINUE_CURRENT_MED, "Discontinue");
				}
				if (bNexERxLicense){
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, MF_SEPARATOR);
				}
			}

			// (j.fouts 2012-09-25 09:27) - PLID 52825 - Only enable monographs if they have the FDB license
			if (bNexERxLicense) {
				long nFDBMedID = VarLong(m_CurMedList->GetValue(nRow, cmlcFDBID), -1);

				m_CurMedList->CurSel = nRow;



				// (j.fouts 2012-08-20 09:26) - PLID 51719 - Don't give an option for monograph to non FDB drugs
				//Get the MedId for the selected drug
				// (j.luckoski 2012-10-04 16:57) - PLID 53042 - Hide the leaflets for non-FDB drugs as well
				if (nFDBMedID >= 0)
				{
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_SHOW_MONO, "View Monograph");
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_SHOW_LEAFLET, "View Leaflet");
				}
				else
				{
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION | MF_DISABLED, IDM_SHOW_MONO, "No Monograph Available");
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION | MF_DISABLED, IDM_SHOW_LEAFLET, "No Leaflet Available");
				}
			}

			// (b.savon 2013-01-14 17:40) - PLID 54592 - Pop it if we have 1 of the 2
			if (bPermission || bNexERxLicense){
				CPoint pt;
				GetCursorPos(&pt);
				pMenu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-10-16 14:35) - PLID 53179 - added "no known allergies" checkbox
void CMedicationDlg::OnCheckHasNoAllergies()
{
	try {

		BOOL bHasNoAllergies = m_checkHasNoAllergies.GetCheck();

		//since this is just as important as adding an allergy, use the same permission
		if (!CheckCurrentUserPermissions(bioPatientAllergies, sptCreate)) {
			//reverse their selection
			m_checkHasNoAllergies.SetCheck(!bHasNoAllergies);
			return;
		}

		if (bHasNoAllergies) {
			//if they have active allergies selected, disallow checking this box
			//(check in data for accuracy)
			if (ReturnsRecordsParam("SELECT TOP 1 ID FROM PatientAllergyT WHERE Discontinued = 0 AND PersonID = {INT}", m_id)) {
				m_checkHasNoAllergies.SetCheck(FALSE);
				MessageBox("The 'patient has no known allergies' status cannot be selected while the patient has active allergies in their list.", "Practice", MB_OK | MB_ICONINFORMATION);
				return;
			}
		}

		//this function will save, audit, and fire a tablechecker
		// (j.fouts 2013-01-25 13:37) - PLID 53574 - Made this a utility function
		UpdateHasNoAllergiesStatus(bHasNoAllergies, m_id, &m_PatientAllergiesChecker);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2012-10-17 11:06) - PLID 51713 - added "no current meds." checkbox
void CMedicationDlg::OnCheckHasNoMeds()
{
	try {

		BOOL bHasNoMeds = m_checkHasNoMeds.GetCheck();

		//since this is just as important as adding a medication, we use the same permission
		if (!CheckCurrentUserPermissions(bioPatientCurrentMeds, sptCreate)) {
			//reverse their selection
			m_checkHasNoMeds.SetCheck(!bHasNoMeds);
			return;
		}

		if (bHasNoMeds) {
			//if they have active current meds. selected, disallow checking this box
			//(check in data for accuracy)
			if (ReturnsRecordsParam("SELECT TOP 1 ID FROM CurrentPatientMedsT WHERE Discontinued = 0 AND PatientID = {INT}", m_id)) {
				m_checkHasNoMeds.SetCheck(FALSE);
				MessageBox("The 'patient has no medications' status cannot be selected while the patient has active current medications in their list.", "Practice", MB_OK | MB_ICONINFORMATION);
				return;
			}
		}

		//this function will save, audit, and fire a tablechecker
		// (j.fouts 2013-01-25 13:37) - PLID 53574 - Made this a utility function
		UpdateHasNoMedsStatus(bHasNoMeds, m_id, &m_CurrentMedsChecker);

	}NxCatchAll(__FUNCTION__);
}

// (j.fouts 2012-11-14 11:42) - PLID 53439 - We want to reposition the queue manualy
int CMedicationDlg::SetControlPositions()
{
	int nVal = CNxDialog::SetControlPositions();

	try
	{
		RepositionQueue(SWP_SHOWWINDOW);
	}
	NxCatchAll(__FUNCTION__);

	return nVal;
}

// (j.fouts 2012-11-14 11:42) - PLID 53439 - Updates the position/size of the queue, calls SetWindowPos with the specified flags
void CMedicationDlg::RepositionQueue(int nSetWindowPosFlags /*=0*/)
{
	CRect rectWnd;
	GetClientRect(&rectWnd);

	CRect rectCurrentMedsColor;
	m_bkg.GetWindowRect(&rectCurrentMedsColor);
	ScreenToClient(&rectCurrentMedsColor);

	CRect rectMedSchedColor;
	m_bkg4.GetWindowRect(&rectMedSchedColor);
	ScreenToClient(&rectMedSchedColor);

	// (b.savon 2013-01-07 12:02) - PLID 54461 - changed to the has no allergies since I removed edit pharmacies
	m_pPrescriptionQueueDlg->SetWindowPos(
		&m_checkHasNoAllergies,
		rectCurrentMedsColor.right + 6,
		rectCurrentMedsColor.top,
		rectMedSchedColor.right - rectCurrentMedsColor.right - 6,
		rectCurrentMedsColor.Height(),
		nSetWindowPosFlags
		);

	m_pPrescriptionQueueDlg->UpdateShowState();
}

// (j.fouts 2013-01-03 14:47) - PLID 54429 - Updates the Interaction count for its embeded queue
// (j.fouts 2013-01-07 10:44) - PLID 54468 - The medications dlg now tracks its own interactions
void CMedicationDlg::SetInteractionCount(long nInteractionCount)
{
	try
	{
		CString strText;
		if (nInteractionCount > 0)
		{
			strText.Format("Interactions (%li)", nInteractionCount);
		}
		else
		{
			strText = "Interactions";
		}

		m_btnInteractions.SetWindowText(strText);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.fouts 2013-01-07 10:44) - PLID 54468 - Open the dlg when clicked
void CMedicationDlg::OnBnClickedInteractions()
{
	try
	{
		// (j.fouts 2013-05-30 09:47) - PLID 56807 - Drug Interactions is now tied to NexERx
		if (g_pLicense->HasEPrescribing(CLicense::cflrSilent) == CLicense::eptSureScripts) {

			//(j.fouts 2013-03-19) - PLID 54468 - Check License and EnsureDatabase
			if (g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent))
			{
				//Also check that the database exists
				if (!FirstDataBank::EnsureDatabase(this, true))
				{
					//We decided not to have an additional warning here about drug interactions not
					//being check because if FDB is not built yet then there should really be no way
					//that they have FDB drugs/allergies in their database yet.
					return;
				}
			}

			// (j.fouts 2013-02-28 14:24) - PLID 54429 - Update the count with the return value
			SetInteractionCount(GetMainFrame()->ShowDrugInteractions(GetActivePatientID(), true, true, true));
		}
	}
	NxCatchAll(__FUNCTION__);
}


// (b.savon 2013-01-15 11:24) - PLID 54592 - Moved this from the past event handler
void CMedicationDlg::AddMedToCurrentMedicationsList(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try{

		CString sql, description;

		//Make sure we have a valid row
		if (pRow == NULL){
			return;
		}

		//check to see that they have permissions - this will prompt a password - Split 'Current Meds' to their own permission
		if (!CheckCurrentUserPermissions(bioPatientCurrentMeds, sptCreate))
			return;

		//Get the MedicationID and FirstDataBankID
		//(s.dhole 3/10/2015 10:56 AM ) - PLID 64561 
		long nMedicationID = VarLong(pRow->GetValue(mrcMedicationID), -1);

		// (b.savon 2013-01-30 09:53) - PLID 54922
		//If the medicationid is -1, we need to import it from FDB
		if (nMedicationID == -1){
			ImportMedication(pRow, nMedicationID);
		}
		//(s.dhole 3/10/2015 10:56 AM ) - PLID 64561 
		long nFirstDataBankID = VarLong(pRow->GetValue(mrcFirstDataBankID), -1);
		//TES 5/9/2013 - PLID 56614 - Note whether the code is out of date
		//(s.dhole 3/10/2015 10:56 AM ) - PLID 64561 
		// (j.jones 2015-05-20 10:33) - PLID 65518 - treat the 0 FDBID as never being out of date
		BOOL bFirstDataBankOutOfDate = VarBool(pRow->GetValue(mrcFDBOutOfDate), FALSE) && nFirstDataBankID > 0;

		//check to make sure that it isn't a duplicate
		// (b.savon 2013-03-26 08:51) - PLID 54854 - Parameterized and only check if the med is a duplicate of a active current med.
		// If there is a med that is discontinued, we'll update its status to active.
		if (ReturnsRecordsParam("SELECT ID FROM CurrentPatientMedsT WHERE PatientID = {INT} AND MedicationID = {INT} AND Discontinued = 0", m_id, nMedicationID)) {

			MsgBox("This medication is already in the list for this patient.");
			return;
		}

		//Check that the patient isn't allergic to it.
		CString strAllergies;
		int nAllergyCount = 0;
		if (!CheckAllergies(GetActivePatientID(), nMedicationID, TRUE, &strAllergies, &nAllergyCount)) {
			CString strMessage;
			if (nAllergyCount > 1) {
				strMessage.Format("Warning: The patient has the following allergies, which conflict with this medication:\r\n%s", strAllergies);
			}
			else {
				strMessage.Format("Warning: The patient has an allergy (%s) which conflicts with this medication.", strAllergies);
			}
			MsgBox("%s", strMessage);
		}

		// (c.haag 2007-02-02 17:26) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
		// (c.haag 2007-10-22 10:35) - PLID 27827 - We also need the Emr Data ID
		_RecordsetPtr rs = CreateParamRecordset("SELECT EMRDataT.Data AS Name, DrugList.EMRDataID FROM DrugList "
			"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"WHERE DrugList.ID = {INT}", nMedicationID);

		FieldsPtr fields;
		fields = rs->Fields;

		//Put the variables from the recordset into local variables
		CString strName;
		long nEmrDataID;

		strName = VarString(fields->Item["Name"]->Value);
		nEmrDataID = VarLong(fields->Item["EMRDataID"]->Value);

		rs->Close();

		//Insert all the defaults into the PatientMedications Table
		// (j.jones 2010-01-22 11:31) - PLID 37016 - supported InputByUserID
		// (b.savon 2013-03-26 09:14) - PLID 54854 - Update the status if discontinued, otherwise add it as new.
		// (s.dhole 2013-06-18 11:40) - PLID 57149 - Update date
		// (j.armen 2013-06-27 17:10) - PLID 57359 - Idenitate CurrentPatientMedsT.  Also return ID when updating
		_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON \r\n"
			" IF EXISTS (SELECT MedicationID FROM CurrentPatientMedsT WHERE MedicationID = {INT} AND PatientID = {INT} AND Discontinued = 1)  "
			"BEGIN "
			"	UPDATE CurrentPatientMedsT SET Discontinued = 0, DiscontinuedDate = NULL , lastUpdateDate= GETDATE()   "
			"	OUTPUT inserted.ID "
			"	WHERE MedicationID = {INT} AND PatientID = {INT} \r\n"
			"END "
			"ELSE "
			"BEGIN "

			"	INSERT INTO CurrentPatientMedsT ( PatientID, MedicationID, InputByUserID ,lastUpdateDate) "
			"	OUTPUT inserted.ID "
			"	VALUES ( {INT}, {INT}, {INT} ,GETDATE())"
			"END ",
			nMedicationID, m_id, nMedicationID, m_id, m_id, nMedicationID, GetCurrentUserID());


		long nID = AdoFldLong(prs, "ID");

		// (c.haag 2007-10-19 17:13) - PLID 27827 - Audit
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiCurrentPatientMedsAdd, nEmrDataID, "", strName, aepHigh, aetCreated);

		// (j.jones 2012-10-17 16:11) - PLID 51713 - clear the HasNoMeds status
		if (m_checkHasNoMeds.GetCheck()) {
			m_checkHasNoMeds.SetCheck(FALSE);
			//this function will save, audit, and fire a tablechecker
			// (j.fouts 2013-01-25 13:37) - PLID 53574 - Made this a utility function
			UpdateHasNoMedsStatus(FALSE, m_id, &m_CurrentMedsChecker);
		}

		//Add the row to the Current Medication datalist
		IRowSettingsPtr pRow;
		pRow = m_CurMedList->GetRow(-1);
		pRow->PutValue(cmlcID, (long)nID);
		pRow->PutValue(cmlcPatientID, (long)m_id);
		pRow->PutValue(cmlcMedName, _variant_t(strName));
		// (j.jones 2013-10-17 15:00) - PLID 58983 - added infobutton icon and medication ID
		_variant_t varInfoButtonIcon = (long)m_hInfoButtonIcon;
		pRow->PutValue(cmlcInfoButton, varInfoButtonIcon);
		pRow->PutValue(cmlcMedicationID, nMedicationID);
		// (c.haag 2007-10-22 15:33) - PLID 27827 - Column 3 is now the EMR data ID
		pRow->PutValue(cmlcEMRDataID, nEmrDataID);
		// (j.jones 2011-05-02 14:55) - PLID 43450 - added the Sig column, and noticed
		// that we didn't fill the other columns either
		pRow->PutValue(cmlcNewCropGUID, g_cvarNull);
		pRow->PutValue(cmlcDiscontinued, g_cvarFalse);
		pRow->PutValue(cmlcSig, "");
		// (b.savon 2012-09-05 11:31) - PLID 52454 - Propagate the FDBID for meds from import to current list.
		pRow->PutValue(cmlcFDBID, nFirstDataBankID);
		//TES 5/9/2013 - PLID 56614 - Fill the FDBOutOfDate column
		// (j.jones 2015-05-20 10:33) - PLID 65518 - treat the 0 FDBID as never being out of date
		pRow->PutValue(cmlcFDBOutOfDate, bFirstDataBankOutOfDate && nFirstDataBankID  > 0 ? g_cvarTrue : g_cvarFalse);

		// (b.savon 2013-01-07 13:30) - PLID 54459 - Color the row if imported.
		if (nFirstDataBankID != -1){
			//TES 5/9/2013 - PLID 56614 - Highlight the outdated codes
			if (bFirstDataBankOutOfDate) {
				pRow->PutBackColor(ERX_IMPORTED_OUTOFDATE_COLOR);
			}
			else {
				pRow->PutBackColor(ERX_IMPORTED_COLOR);
			}
		}
		else{
			// (b.savon 2013-03-20 18:00) - PLID 54854 - Weird, if I don't set this, there can be a case where when you add
			// a new med, that was previously discontinued and deleted in the same row with the same med name as the other,
			// it puts the row color as gray.
			pRow->PutBackColor(RGB(255, 255, 255));
		}

		m_CurMedList->InsertRow(pRow, -1);

		// (j.jones 2008-11-25 12:24) - PLID 32183 - send a tablechecker
		m_CurrentMedsChecker.Refresh(m_id);

		// (c.haag 2007-01-25 12:42) - PLID 24420 - Warn the user about any EMR's with "official"
		// Current Medications details that are inconsistent with the patient's official medication list
		// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
		// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
		if (g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
			WarnMedicationDiscrepanciesWithEMR(m_id);
		}
		// (c.haag 2010-09-21 13:43) - PLID 40610 - Create todo alarms for decisions
		CDWordArray arNewCDSInterventions;
		//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
		UpdateDecisionRules(GetRemoteData(), m_id, arNewCDSInterventions);
		GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

		EnableAppropriateButtons();

		//must requery now, because Latin naming may have changed
		// (j.fouts 2012-11-14 11:59) - PLID 53439 - Requery the queue
		m_pPrescriptionQueueDlg->RequeryQueue();

		UpdateView();

	}NxCatchAll(__FUNCTION__);
}


// (b.savon 2013-01-15 11:24) - PLID 54592 - Moved this from the medication pick dlg
void CMedicationDlg::OnBnClickedBtnEditMeds()
{
	try{
		// (will prompt for passwords)
		if (!CheckCurrentUserPermissions(bioPatientMedication, sptDynamic0)) {
			return;
		}

		//open the edit medication dialog

		CEditMedicationListDlg dlg(this);
		dlg.DoModal();
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-15 10:58) - PLID 54613 - Moved this from the Allergy Pick list dlg
void CMedicationDlg::OnBnClickedBtnEditAllergies()
{
	try{
		//Don't open the list if they don't have permission to edit it.
		if (!CheckCurrentUserPermissions(bioPatientAllergies, sptDynamic0))
			return;

		//They want to edit the medication list so we need to open that dialog
		CEditAllergyListDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/08/2013) - PLID 58416 - Handler for when the user clicks on the med history button.
void CMedicationDlg::OnBnClickedMedicationHistoryBtn()
{
	try {
		// (r.gonet 09/20/2013) - PLID 58416 - Check for the license plus the permission to read.
		if (g_pLicense->HasEPrescribing(CLicense::cflrSilent) != CLicense::eptSureScripts ||
			!CheckCurrentUserPermissions(bioPatientMedicationHistory, sptRead)) {
			return;
		}
		// (r.gonet 08/08/2013) - PLID 58416 - Open the medication history dialog.
		CMedicationHistoryDlg dlg(this, GetActivePatientID());
		// (r.gonet 09/03/2013) - PLID 58416 - Set the child dialog's background color to be the same as the parent dialog.
		dlg.SetColor(m_bkg.GetColor());
		dlg.DoModal();
		// (a.wilson 2013-08-30 16:55) - PLID 57844
		if (dlg.m_bImportedHistoryMeds == true) {
			m_CurMedList->Requery();
			// (r.gonet 12/20/2013) - PLID 57844 - Show the interactions if there are any.
			SetInteractionCount(GetMainFrame()->ShowDrugInteractions(GetActivePatientID()));
		}
	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-15 10:59) - PLID 54613 - Moved from previous event handler
void CMedicationDlg::AddAllergyToCurrentAllergyList(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try{
		//Don't do anything if they don't have permissions
		if (!CheckCurrentUserPermissions(bioPatientAllergies, sptCreate))
			return;

		if (pRow){
			long nAllergyID = VarLong(pRow->GetValue(aslcAllergyID), -1);
			long nMedFromFDB = VarLong(pRow->GetValue(aslcStatus), -1);
			long nConceptIDType = VarLong(pRow->GetValue(aslcConceptTypeID), -1);
			// (b.savon 2013-01-30 09:53) - PLID 54922
			//If the medicationid is -1, we need to import it from FDB
			if (nAllergyID == -1){
				ImportAllergy(pRow, nAllergyID);
			}

			CString strAllergyName = VarString(pRow->GetValue(aslcAllergyName), "");
			if (strAllergyName.IsEmpty()){
				return;
			}

			// (b.savon 2012-09-26 09:56) - PLID 52107 - check to make sure that it isn't a duplicate from Practice/FDB Imported Meds.
			// NewCropBrowserDlg line 1802 it says "we allow duplicate entries of allergies, so if they already have a non-newcrop allergy
			// that is the exact same as the newcrop allergy, we are going to add the duplicate"
			if (ReturnsRecordsParam(
				"SELECT	1 \r\n"
				"FROM	PatientAllergyT  \r\n"
				"	INNER JOIN AllergyT ON PatientAllergyT.AllergyID = AllergyT.ID  \r\n"
				"	INNER JOIN EMRDataT ON AllergyT.EmrDataID = EMRDataT.ID \r\n"
				"WHERE PersonID = {INT} AND PatientAllergyT.RXCUI = '' AND EMRDataT.Data = {STRING} AND Discontinued = 0 \r\n"
				, m_id, strAllergyName)) {

				MsgBox((LPCTSTR)(strAllergyName + " is already in the list for this patient."));
				return;
			}

			// (c.haag 2007-02-02 17:26) - PLID 24561 - We now store medication names in EmrDataT.Data rather than DrugList.Name
			_RecordsetPtr rsDrugs = CreateRecordset("SELECT EMRDataT.Data AS Name FROM DrugList INNER JOIN DrugAllergyT ON DrugList.ID = "
				"DrugAllergyT.DrugID "
				"LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
				"WHERE DrugAllergyT.AllergyID = %li AND (DrugList.ID IN "
				"(SELECT MedicationID FROM CurrentPatientMedsT WHERE PatientID = %li) OR DrugList.ID IN "
				"(SELECT MedicationID FROM PatientMedications WHERE PatientID = %li AND Deleted = 0))", nAllergyID, GetActivePatientID(),
				GetActivePatientID());
			if (!rsDrugs->eof) {
				CString strDrugNames;
				while (!rsDrugs->eof) {
					strDrugNames += AdoFldString(rsDrugs, "Name") + "\r\n";
					rsDrugs->MoveNext();
				}
				strDrugNames = strDrugNames.Left(strDrugNames.GetLength() - 2);
				MsgBox("Warning!  The following drugs are contraindicated for patients with this allergy, and are currently being taken by this patient: \r\n%s", strDrugNames);
			}

			long nPatAllergyID = NewNumber("PatientAllergyT", "ID");

			//Insert the new record if the clicked OK;
			// (c.haag 2009-12-22 12:30) - PLID 35766 - Added EnteredDate
			// (b.savon 2013-03-26 09:35) - PLID 54854 - parameterized and update discontinue flag if set for allergy
			// (s.dhole 2013-07-05 16:04) - PLID 56931 Added updatedate
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON; "
				"DECLARE @allergyID INT  "
				"SET @allergyID = (SELECT TOP 1 ID FROM PatientAllergyT WHERE AllergyID = {INT} AND PersonID = {INT} AND Discontinued = 1 ORDER BY ID DESC)  "
				"IF @allergyID IS NOT NULL  "
				"BEGIN  "
				"	UPDATE PatientAllergyT SET Discontinued = 0, DiscontinuedDate = NULL , LastUpdateDate = GETDATE() WHERE ID = @allergyID "
				"	SET NOCOUNT OFF;  "
				"    SELECT @allergyID AS ID  "
				"END "
				"ELSE "
				"BEGIN "
				"	INSERT INTO PatientAllergyT (ID, PersonID, AllergyID, EnteredDate,LastUpdateDate) "
				"						 VALUES ({INT}, {INT}, {INT}, GetDate(),GETDATE())"
				"	SET NOCOUNT OFF; "
				"	SELECT {INT} AS ID "
				"END ",
				nAllergyID, m_id, nPatAllergyID, m_id, nAllergyID, nPatAllergyID);


			long nIngredientAllergyID = -1;
			if (!prs->eof){
				nIngredientAllergyID = AdoFldLong(prs->Fields, "ID", -1);
			}

			// (c.haag 2007-10-19 11:56) - PLID 27822 - Do the audit, yo!
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiPatientAllergyAdd, GetActivePatientID(), "", strAllergyName, aepMedium, aetCreated);

			// (j.luckoski 2012-11-16 15:07) - PLID 53806 - Set up our array and call the api to import these ingredients, but only if its status = 1
			// which signifies it is imported from FDB.
			if (FirstDataBank::EnsureDatabase(NULL, true) && SureScripts::IsEnabled() && nMedFromFDB == 1 && nConceptIDType == 2 && nIngredientAllergyID != -1) {
				CArray<NexTech_Accessor::_FDBIngredientImportInputPtr, NexTech_Accessor::_FDBIngredientImportInputPtr> aryIngredients;

				NexTech_Accessor::_FDBIngredientImportInputPtr ingredient(__uuidof(NexTech_Accessor::FDBIngredientImportInput));

				ingredient->PatAllergyID = nIngredientAllergyID;
				aryIngredients.Add(ingredient);


				if (aryIngredients.GetSize() > 0) {
					//	Create our SAFEARRAY to be passed to the IngredientImport function in the API
					Nx::SafeArray<IUnknown *> saryIngredients = Nx::SafeArray<IUnknown *>::From(aryIngredients);

					//	Call the API to import the ingredients and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
					NexTech_Accessor::_FDBIngredientImportResultsArrayPtr importResults = GetAPI()->IngredientImport(GetAPISubkey(), GetAPILoginToken(), saryIngredients);

					// (j.luckoski 2013-02-27 09:32) - PLID 53806 - Don't display the results as multiple medications could 
					// have the same ingredient causing the issue, but I am not deleting in case we decide to use
					// it in the future as it could be valuable.

					/*Nx::SafeArray<IUnknown *> saryIngredientResults(importResults->FDBIngredientImportResults);

					CString strFailure = "\r\nThe following ingredients are already imported:\r\n";
					foreach(NexTech_Accessor::_FDBIngredientImportOutputPtr pIngredientResult, saryIngredientResults)
					{
					if( pIngredientResult->Success == VARIANT_FALSE ){
					strFailure += CString((LPCTSTR)pIngredientResult->AllergyName) + "\r\n";
					}
					}

					CString strResults;

					if( strFailure != "\r\nThe following ingredients are already imported:\r\n" ){
					strResults += strFailure;
					}

					if( !strResults.IsEmpty() ){
					MsgBox("%s", strResults);
					}*/

				}
			}


			m_pPatAllergies->Requery();

			// (j.jones 2012-10-17 10:01) - PLID 53179 - clear the HasNoAllergies status
			if (m_checkHasNoAllergies.GetCheck()) {
				m_checkHasNoAllergies.SetCheck(FALSE);
				//this function will save, audit, and fire a tablechecker
				// (j.fouts 2013-01-25 13:37) - PLID 53574 - Made this a utility function
				UpdateHasNoAllergiesStatus(FALSE, m_id, &m_PatientAllergiesChecker);
			}

			// (j.jones 2008-11-25 11:25) - PLID 28508 - clear the allergy review status
			UpdateAllergyReviewStatus(FALSE);

			// (j.jones 2008-11-25 12:20) - PLID 32183 - send a tablechecker, after the allergy review status is changed
			m_PatientAllergiesChecker.Refresh(m_id);

			// (c.haag 2007-04-05 17:55) - PLID 25524 - Warn the user about any EMR's with "official"
			// Allergies details that are inconsistent with the patient's official allergy list
			// (a.walling 2007-11-28 13:05) - PLID 28044 - Check for expired EMR license
			// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
			if (g_pLicense->HasEMR(CLicense::cflrSilent, TRUE) == 2) {
				WarnAllergyDiscrepanciesWithEMR(m_id);
			}

			// (j.fouts 2012-08-10 09:46) - PLID 52089 - Allergies Changed Check Interactions
			// (j.fouts 2013-01-07 10:44) - PLID 54468 - Don't requery the whole queue, just check interactions
			// (j.fouts 2013-02-28 14:24) - PLID 54429 - Update the count with the return value
			SetInteractionCount(GetMainFrame()->ShowDrugInteractions(GetActivePatientID()));

			//TES 11/13/2013 - PLID 59475 - Check CDS rules
			CDWordArray arNewCDSInterventions;
			UpdateDecisionRules(GetRemoteData(), m_id, arNewCDSInterventions);
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

			EnableAppropriateButtons();
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-15 11:14) - PLID 54854 - Add the right click delete option for allergies
void CMedicationDlg::RButtonUpNxdlpatallergies(long nRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		// if its a valid row and column
		if (nRow > -1 && nCol > -1)
		{
			// (b.savon 2013-01-14 12:30) - PLID 54613
			if (CheckCurrentUserPermissions(bioPatientAllergies, sptDelete)){
				m_pPatAllergies->CurSel = nRow;
				CMenu pMenu;
				pMenu.CreatePopupMenu();
				int nMenuIndex = 0;
				pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_DELETE_CURRENT_ALLERGY, "Delete");
				// (b.savon 2013-01-25 12:43) - PLID 54854
				if (!VarBool(m_pPatAllergies->GetValue(nRow, alcIsDiscontinued), FALSE)){
					pMenu.InsertMenu(nMenuIndex++, MF_BYPOSITION, IDM_DISCONTINUE_CURRENT_ALLERGY, "Discontinue");
				}
				CPoint pt;
				GetCursorPos(&pt);
				pMenu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}



VARIANT_BOOL CMedicationDlg::ImportMedication(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nNewDrugListID)
{
	VARIANT_BOOL vbSuccess = VARIANT_FALSE;
	try{
		CArray<NexTech_Accessor::_FDBMedicationImportInputPtr, NexTech_Accessor::_FDBMedicationImportInputPtr> aryMedications;
		NexTech_Accessor::_FDBMedicationImportInputPtr med(__uuidof(NexTech_Accessor::FDBMedicationImportInput));
		//(s.dhole 3/10/2015 10:56 AM ) - PLID 64561 
		long nFDBID = VarLong(pRow->GetValue(mrcFirstDataBankID), -1);
		//(s.dhole 3/10/2015 10:56 AM ) - PLID 64561 
		CString strMedname = VarString(pRow->GetValue(mrcMedicationName), "");
		med->FirstDataBankID = nFDBID;
		med->MedicationName = AsBstr(strMedname);

		aryMedications.Add(med);

		//	Create our SAFEARRAY to be passed to the AllergyImport function in the API
		Nx::SafeArray<IUnknown *> saryMedications = Nx::SafeArray<IUnknown *>::From(aryMedications);

		CWaitCursor cwait;

		//	Call the API to import the meds and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
		NexTech_Accessor::_FDBMedicationImportResultsArrayPtr importResults = GetAPI()->MedicationImport(GetAPISubkey(), GetAPILoginToken(), saryMedications);

		//	If for some reason we get nothing back (although, this should never happen), tell the user and bail.
		if (importResults->FDBMedicationImportResults == NULL){
			//MsgBox("There were no medications to import."); This shouldn't happen, if it did, there is some accessor error
			return VARIANT_FALSE;
		}

		Nx::SafeArray<IUnknown *> saryMedicationResults(importResults->FDBMedicationImportResults);

		// Hand back our new DrugListID
		foreach(NexTech_Accessor::_FDBMedicationImportOutputPtr pMedicationResult, saryMedicationResults)
		{
			nNewDrugListID = pMedicationResult->DrugListID;
			vbSuccess = pMedicationResult->Success;
		}

		return vbSuccess;
	}NxCatchAll(__FUNCTION__);
	return vbSuccess;
}

// (b.savon 2013-01-30 10:56) - PLID 54927
VARIANT_BOOL CMedicationDlg::ImportAllergy(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nNewAllergyID)
{
	VARIANT_BOOL vbSuccess = VARIANT_FALSE;
	try{

		CArray<NexTech_Accessor::_FDBAllergyImportInputPtr, NexTech_Accessor::_FDBAllergyImportInputPtr> aryAllergies;
		NexTech_Accessor::_FDBAllergyImportInputPtr allergy(__uuidof(NexTech_Accessor::FDBAllergyImportInput));

		allergy->AllergyName = _bstr_t(pRow->GetValue(aslcAllergyName));
		allergy->ConceptID = VarLong(pRow->GetValue(aslcConceptID));
		allergy->ConceptTypeID = VarLong(pRow->GetValue(aslcConceptTypeID));

		aryAllergies.Add(allergy);

		//	Create our SAFEARRAY to be passed to the AllergyImport function in the API
		Nx::SafeArray<IUnknown *> saryAllergies = Nx::SafeArray<IUnknown *>::From(aryAllergies);

		CWaitCursor cwait;

		//	Call the API to import the allergies and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
		NexTech_Accessor::_FDBAllergyImportResultsArrayPtr importResults = GetAPI()->AllergyImport(GetAPISubkey(), GetAPILoginToken(), saryAllergies);

		//	If for some reason we get nothing back (although, this should never happen), tell the user and bail.
		if (importResults->FDBAllergyImportResults == NULL){
			//MsgBox("There were no allergies to import.");
			return VARIANT_FALSE;
		}

		Nx::SafeArray<IUnknown *> saryAllergyResults(importResults->FDBAllergyImportResults);

		//	Prepare our results message so the user knows which imported succesfully (or not)
		foreach(NexTech_Accessor::_FDBAllergyImportOutputPtr pAllergyResult, saryAllergyResults)
		{
			nNewAllergyID = pAllergyResult->AllergyID;
			vbSuccess = pAllergyResult->Success;
		}

		return vbSuccess;
	}NxCatchAll(__FUNCTION__);
	return vbSuccess;
}

void CMedicationDlg::OnSize(UINT nType, int cx, int cy)
{
	try {
		CNxDialog::OnSize(nType, cx, cy);

		if (AreAllergyControlsLoaded()){
			// (b.savon 2013-04-16 13:57) - PLID 54613 - Sizing the windows does some weird stuff with focus and datalists
			// As a workaround, until this is changed to a control, let's hide the DL, show the check boxes, and set focus
			// to the nx color. That way, when they click in the text box again, the results will appear. (How gaining focus works
			// currently for the search results).  If we call OnEnChangeEditAllergySearch, it is going to hit the API everytime. 
			// This is fine for doing a maximized/restore but if we do a drag to resize, it'll hit a ton of times; I think
			// abs(cxstart - cxend) / 4 times if doing a simple, swift width resize.
			GetDlgItem(IDC_CHECK_REVIEWED_ALLERGIES)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_CHECK_HAS_NO_ALLERGIES)->ShowWindow(SW_SHOW);
			m_bkg.SetFocus();
		}

		if (AreMedicationConrolsLoaded()){
			// (b.savon 2013-04-16 16:07) - PLID 54592 - Similar comment as above
			GetDlgItem(IDC_CHECK_HAS_NO_MEDS)->ShowWindow(SW_SHOW);
			m_bkg.SetFocus();
		}

	} NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-04-16 14:09) - PLID 54613 - If the window is resized, handle a workaround case.
BOOL CMedicationDlg::AreAllergyControlsLoaded()
{
	return  m_bkg.m_hWnd != NULL &&
		m_nxdlAllergySearchResults != NULL &&
		GetDlgItem(IDC_CHECK_REVIEWED_ALLERGIES) != NULL &&
		GetDlgItem(IDC_CHECK_HAS_NO_ALLERGIES) != NULL;
}

// (b.savon 2013-04-16 16:05) - PLID 54592 - If the window is resized, handle a workaroudn case.
BOOL CMedicationDlg::AreMedicationConrolsLoaded()
{
	return  m_bkg.m_hWnd != NULL &&
		m_nxdlMedSearchResults != NULL &&
		GetDlgItem(IDC_CHECK_HAS_NO_MEDS) != NULL;
}

// (j.jones 2013-08-19 11:15) - PLID 57906 - added checkbox to show/hide discontinued allergies
void CMedicationDlg::OnCheckHideDiscontinuedAllergies()
{
	try {

		SetRemotePropertyInt("HideDiscontinuedAllergies", m_checkHideDiscontinuedAllergies.GetCheck() ? 1 : 0, 0, GetCurrentUserName());

		UpdateView();

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 09/20/2013) - PLID 58396 - Handler for the refresh medication history button message.
LRESULT CMedicationDlg::OnBackgroundMedHistoryRequestComplete(WPARAM wParam, LPARAM lParam)
{
	try
	{
		long nPatientID = (long)wParam;
		BOOL bHasMedicationHistory = (BOOL)lParam;
		if (nPatientID != m_id) {
			return 0;
		}
		// (r.gonet 09/20/2013) - PLID 58396 - Set the text color to red if they have med history.
		m_btnMedicationHistory.SetTextColor(bHasMedicationHistory ? RGB(255, 0, 0) : RGB(0, 0, 0));
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.jones 2013-10-17 15:55) - PLID 58983 - added left click handler
void CMedicationDlg::OnLeftClickCurrentMedList(long nRow, short nCol, long x, long y, long nFlags)
{
	try {

		if (nRow == -1) {
			return;
		}

		m_CurMedList->CurSel = nRow;

		switch (nCol) {
			// (j.jones 2013-10-17 12:05) - PLID 58983 - added Patient Education support
		case cmlcMedName:
		case cmlcInfoButton:
		{
			// (r.gonet 2014-01-27 15:29) - PLID 59339 - Only if we have the med name as a link.
			bool bShowPatientEducationLink = (GetRemotePropertyInt("ShowPatientEducationLinks", 0, 0, GetCurrentUserName()) ? true : false);
			if (nCol == cmlcMedName && (!bShowPatientEducationLink || m_CurMedList->GetColumn(cmlcMedName)->FieldType != NXDATALISTLib::cftTextSingleLineLink)) {
				break;
			}
			IRowSettingsPtr pRow = m_CurMedList->GetRow(nRow);
			long nMedicationID = VarLong(pRow->GetValue(cmlcMedicationID), -1);
			if (nMedicationID == -1) {
				//should not be possible
				ASSERT(FALSE);
				ThrowNxException("No valid medication ID was found.");
			}

			if (nCol == cmlcMedName) {
				// (r.gonet 10/30/2013) - PLID 58980 - The drug name hyperlink goes to the MedlinePlus website
				LookupMedlinePlusInformationViaSearch(this, mlpDrugListID, nMedicationID);
			}
			else if (nCol == cmlcInfoButton) {
				//the info button goes to the MedlinePlus website using the HL7 InfoButton URL standard
				LookupMedlinePlusInformationViaURL(this, mlpDrugListID, nMedicationID);
			}
			break;
		}
		}

	}NxCatchAll(__FUNCTION__);
}



// (s.dhole 2013-06-07 11:44) - PLID 56926
void CMedicationDlg::EditingFinishingCurrentMedList(long nRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try{
		// (s.dhole 2014-01-16 12:39) - PLID 60367 check write permission
		if (!pbCommit || *pbCommit == FALSE){
			return;
		}

		if (!CheckCurrentUserPermissions(bioPatientCurrentMeds, sptWrite)) {
			*pbCommit = FALSE;
			return;
		}
		switch (nCol){
		case cmlcStartDate:
		{
			COleDateTime dt = pvarNewValue->date;
			if (dt.m_dt == 0.0) {
				_variant_t varNull;
				varNull.vt = VT_NULL;
				*pvarNewValue = varNull;
				return;
			}
			else if (dt.m_status == COleDateTime::invalid || (dt.m_dt < 1.0) || dt.GetYear() < 1900) {
				MsgBox("Please enter a valid date.");
				*pbCommit = FALSE;
				*pbContinue = FALSE;
				return;
			}
		}
		break;
		}
	} NxCatchAll(__FUNCTION__);
}

// (s.dhole 2013-06-07 11:44) - PLID 56926
void CMedicationDlg::ResizeCurrentMedicationColumns(const CString strColWidths)
{

	short i;
	if (strColWidths.IsEmpty()){
		return;
	}
	else{
		for (i = cmlcID; i <= cmlcLastUpdateDate; i++){
			// we should not do this but err can cause issue to show hiddent coulmn
			switch (i){
			case cmlcMedName:
			case cmlcSig:
			case cmlcStartDate:
			case cmlcLastUpdateDate:
			case cmlcInfoButton:
				m_CurMedList->GetColumn(i)->StoredWidth = GetColumnWidth(strColWidths, i);
				break;
			default:
				m_CurMedList->GetColumn(i)->StoredWidth = 0;
				break;
			}
		}
	}
}

// (s.dhole 2013-06-07 11:44) - PLID 56931 Resize coumn
void CMedicationDlg::ResizeAllergyColumns(const CString strColWidths)
{
	short i;
	if (strColWidths.IsEmpty()){
		return;
	}
	else{
		for (i = cmlcID; i <= alcLastUpdateDate; i++){
			m_pPatAllergies->GetColumn(i)->StoredWidth = GetColumnWidth(strColWidths, i);
			// we should not do this but err can cause issue to show hiddent coulmn
			switch (i){
			case alcEnteredDate:
			case alcName:
			case aldNotes:
			case alcLastUpdateDate:
				m_pPatAllergies->GetColumn(i)->StoredWidth = GetColumnWidth(strColWidths, i);
				break;
			default:
				m_pPatAllergies->GetColumn(i)->StoredWidth = 0;
				break;
			}
		}
	}
}


// (s.dhole 2013-06-07 11:44) - PLID 56926
CString  CMedicationDlg::ReadColumnWidths(NXDATALISTLib::_DNxDataListPtr pList)
{
	CString strWidths;
	for (short i = 0; i < pList->ColumnCount; i++)
	{
		IColumnSettingsPtr pCol = pList->GetColumn(i);
		CString str;
		if (i == 0){
			str.Format("%d", pCol->StoredWidth);
		}
		else{
			str.Format(",%d", pCol->StoredWidth);
		}
		strWidths += str;
	}
	return strWidths;

}
// (s.dhole 2013-06-07 11:44) - PLID 56926 save column width
void CMedicationDlg::ColumnSizingFinishedCurrentMedList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try{
		// (j.jones 2016-01-12 09:33) - PLID 67855 - renamed CurrentPatientMedicationColumnWidth to CurrentPatientMedicationColumnWidths
		// to effectively reset the stored widths for current medications
		SetRemotePropertyText("CurrentPatientMedicationColumnWidths", ReadColumnWidths(m_CurMedList), 0, GetCurrentUserName());
	} NxCatchAll(__FUNCTION__);
}

// (s.dhole 2013-07-08 14:30) - PLID 56931 Save column width
void CMedicationDlg::ColumnSizingFinishedpatallergies(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try{
		SetRemotePropertyText("CurrentPatientAllergyColumnWidth", ReadColumnWidths(m_pPatAllergies), 0, GetCurrentUserName());
	} NxCatchAll(__FUNCTION__);
}

// (s.dhole 2013-06-07 11:44) - PLID 56926 Read column width
long CMedicationDlg::GetColumnWidth(const CString& strColWidths, short nColumn)
{
	CString str;
	int nIndex = 0, nEndIndex = 0;
	for (short i = 0; i < nColumn && nIndex != -1; i++){
		nIndex = strColWidths.Find(',', nIndex + 1);
	}
	if (nIndex == -1){
		return -1;
	}
	nEndIndex = strColWidths.Find(',', nIndex + 1);
	if (nEndIndex == -1){
		nEndIndex = strColWidths.GetLength();
	}
	str = strColWidths.Mid(nIndex == 0 ? 0 : (nIndex + 1), nEndIndex - (nIndex == 0 ? 0 : nIndex + 1));
	return atoi(str);
}

// (s.dhole 2013-06-07 11:44) - PLID 56926 Update current Medication
// no try catch since it is called from another function
void  CMedicationDlg::UpdateCurrentMedRecord(long nRow, short nColumn, const VARIANT&  varValue)
{
	long nID = VarLong(m_CurMedList->GetValue(nRow, cmlcID), -1);
	_variant_t varNewValue;
	if (nID >0){
		CString strFiled;
		switch (nColumn)
		{
		case cmlcSig:
		{
			strFiled = "Sig = {VT_BSTR} ";
			varNewValue = _variant_t(varValue.bstrVal);
		}
		break;
		case cmlcDiscontinued:
		{
			strFiled = "Discontinued = {INT} , DiscontinuedDate = GETDATE() ";
			varNewValue = _variant_t(varValue.lVal, VT_I4);
		}
		break;
		case cmlcStartDate:
		{
			strFiled = "StartDate = {VT_DATE}";

			if (varValue.vt == VT_NULL) {
				varNewValue = g_cvarNull;
			}
			else {
				varNewValue = _variant_t(varValue.date, VT_DATE);
			}
		}
		break;
		}
		ADODB::_RecordsetPtr pRs = CreateParamRecordset(FormatString(
			" SET NOCOUNT ON \r\n"
			" DECLARE @ID AS INT \r\n"
			" SET @ID = {INT} \r\n"
			" UPDATE CurrentPatientMedsT SET %s, LastUpdateDate =  GETDATE()   \r\n"
			" OUTPUT inserted.LastUpdateDate  \r\n"
			" WHERE ID = @ID ; \r\n", strFiled)
			, nID, varNewValue);
		if (!pRs->eof){
			_variant_t var = pRs->Fields->Item["LastUpdateDate"]->Value;
			m_CurMedList->PutValue(nRow, cmlcLastUpdateDate, var);
		}
	}

}

//(s.dhole 2/18/2015 11:41 AM ) - PLID 64561
void CMedicationDlg::SelChosenNxdlMedSearchResults(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(mrcMedicationID)) == NO_RESULTS_ROW){
				return;
			}
			AddMedToCurrentMedicationsList(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

//(s.dhole 3/10/2015 5:25 PM ) - PLID 64564
void CMedicationDlg::SelChosenNxdlAllergySearchResults(LPDISPATCH lpRow)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			if (VarLong(pRow->GetValue(aslcAllergyID)) == NO_RESULTS_ROW){
				return;
			}
			AddAllergyToCurrentAllergyList(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-21 09:40) - PLID 67994 - added checkbox to include free text meds in current meds
void CMedicationDlg::OnMedIncludeFreeTextMeds()
{
	try {

		//make sure they know what they are doing
		if (m_checkIncludeFreeTextCurrentMeds.GetCheck()) {

			if (!ConfirmFreeTextSearchWarning(this, false)) {
				//they changed their minds
				m_checkIncludeFreeTextCurrentMeds.SetCheck(FALSE);
				return;
			}

			//there are some weird focus issues here where the messagebox takes away focus
			//and the datalist placeholder text can't figure out where focus is,
			//so set the focus back to the checkbox
			m_checkIncludeFreeTextCurrentMeds.SetFocus();
		}

		//reflect their choice in the search results
		ResetCurrentMedsSearchProvider();

		//we do NOT update the preference default here

	}NxCatchAll(__FUNCTION__);
}


// (j.jones 2016-01-21 09:40) - PLID 67994 - resets the current med search provider based
// on the value of the current meds' 'include free text' checkbox
void CMedicationDlg::ResetCurrentMedsSearchProvider()
{
	try {

		bool bIncludeFDBMedsOnly = m_checkIncludeFreeTextCurrentMeds.GetCheck() ? false : true;

		//force this to false if they don't have FDB
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			bIncludeFDBMedsOnly = false;
		}

		//re-bind the control to the new provider
		m_nxdlMedSearchResults = BindMedicationSearchListCtrl(this, IDC_NXDL_MED_SEARCH_RESULTS, GetRemoteData(), false, bIncludeFDBMedsOnly);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-21 14:52) - PLID 67995 - added checkbox to include free text allergies
void CMedicationDlg::OnMedIncludeFreeTextAllergies()
{
	try {

		//make sure they know what they are doing
		if (m_checkIncludeFreeTextAllergies.GetCheck()) {

			if (!ConfirmFreeTextSearchWarning(this, true)) {
				//they changed their minds
				m_checkIncludeFreeTextAllergies.SetCheck(FALSE);
				return;
			}

			//there are some weird focus issues here where the messagebox takes away focus
			//and the datalist placeholder text can't figure out where focus is,
			//so set the focus back to the checkbox
			m_checkIncludeFreeTextAllergies.SetFocus();
		}

		//reflect their choice in the search results
		ResetAllergySearchProvider();

		//we do NOT update the preference default here

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2016-01-21 14:55) - PLID 67995 - resets the allergy search provider based
// on the value of the allergy 'include free text' checkbox
void CMedicationDlg::ResetAllergySearchProvider()
{
	try {

		bool bIncludeFDBAllergiesOnly = m_checkIncludeFreeTextAllergies.GetCheck() ? false : true;

		//force this to false if they don't have FDB
		if (!g_pLicense->CheckForLicense(CLicense::lcFirstDataBank, CLicense::cflrSilent)) {
			bIncludeFDBAllergiesOnly = false;
		}

		//re-bind the control to the new provider
		m_nxdlAllergySearchResults = BindAllergySearchListCtrl(this, IDC_NXDL_ALLERGY_SEARCH_RESULTS, GetRemoteData(), bIncludeFDBAllergiesOnly);

	}NxCatchAll(__FUNCTION__);
}
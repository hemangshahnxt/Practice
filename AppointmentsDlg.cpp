// AppointmentsDlg.cpp : implementation file
// Appointments tab in patient module
// Last significant change by Brad Bellomo
#include "stdafx.h"
#include "Practice.h"
#include "AppointmentsDlg.h"
#include "MainFrm.h"
#include "NxStandard.h"
#include "schedulerView.h"
#include "NxSchedulerDlg.h"
#include "GlobalUtils.h"
#include "GlobalDataUtils.h"
#include "AuditTrail.h"
#include "GlobalSchedUtils.h"
#include "ResLinkDlg.h"
#include "InternationalUtils.h"
#include "CaseHistoryDlg.h"
#include "ReasonDlg.h"
#include "InvUtils.h"
#include "InvPatientAllocationDlg.h"
#include "GlobalFinancialUtils.h"
#include "HL7Utils.h"
#include "PatientsRc.h"
#include "RoomManagerDlg.h"				// (d.lange 2010-11-08 10:58) - PLID 41192 - Need access for the struct DevicePlugins
#include "ApptPopupMenuManager.h"
#include "DateTimeUtils.h"				// (j.dinatale 2011-10-14 13:00) - PLID 44597
#include "RecallsNeedingAttentionDlg.h"	// (j.armen 2012-02-29 10:33) - PLID 48487
#include "RecallUtils.h"				// (a.wilson 2012-3-6) PLID 48485
#include "DeviceLaunchUtils.h"			// (j.gruber 2013-04-03 15:16) - PLID 56012
#include "PatientEncounterDlg.h"	// (b.savon 2014-12-01 10:33) - PLID 53162 - Connectathon - HL7 Dialog for Patient Encounter
#include "NotesDlg.h"			// (j.gruber 2014-12-15 11:57) - PLID 64419 - Rescheduling Queue - add notes icon
#include "NxModalParentDlg.h"  // (j.gruber 2014-12-15 11:57) - PLID 64419 - Rescheduling Queue - add notes icon
#include <NxPracticeSharedLib\SharedScheduleUtils.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;


// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



extern CPracticeApp theApp;

// (b.savon 2012-03-27 16:53) - PLID 49074 - Added alcRecall
enum EAppointmentListColumns {
	
	alcResID = 0, 
	alcID,	
	alcColor,
	alcHasExtraNotes, // (j.gruber 2014-12-15 11:57) - PLID 64419 - Rescheduling Queue - add notes icon
	alcExtraNotesIcon, // (j.gruber 2014-12-15 11:57) - PLID 64419 - Rescheduling Queue - add notes icon
	alcDate, 
	alcDay, 
	alcStartTime, 
	alcPurpose, 
	alcItem, 
	alcConfirmed, 
	alcNotes,
	alcRecall,
	alcStatus, 
	alcLastLM,
	alcLoginName, 
	alcInputDate, 
	alcModifiedDate, 
	alcModifiedBy, 
	alcCancelledDate,
	alcCancelledBy,
	alcCancelledReason,
	alcNoShowReason,
	alcLocationID,
	alcLocationName,			// (d.thompson 2009-08-10) - PLID 11746
};

/////////////////////////////////////////////////////////////////////////////
// CAppointmentsDlg dialog
// (j.jones 2014-08-07 15:10) - PLID 63168 - added tablechecker for ShowStates
CAppointmentsDlg::CAppointmentsDlg(CWnd* pParent)
	: CPatientDialog(CAppointmentsDlg::IDD, pParent),
	m_AptShowStateChecker(NetUtils::AptShowStateT)
{
	//{{AFX_DATA_INIT(CAppointmentsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Patient_Information/view_a_patient's_appointment_history.htm";

	// (a.walling 2010-10-13 16:23) - PLID 40977
	m_id = -1;

	// (b.savon 2012-03-29 11:43) - PLID 49074
	m_hiRecall = NULL;
	// (j.gruber 2014-12-15 12:38) - PLID 64419 - Rescheduling Queue - Notes for Appts
	m_hExtraNotesIcon = NULL;
}

// (b.savon 2012-03-29 11:13) - PLID 49074 - Remove icon
CAppointmentsDlg::~CAppointmentsDlg()
{
	try{

		if(m_hiRecall != NULL) {
			DestroyIcon((HICON)m_hiRecall);
		}
		// (j.gruber 2014-12-15 12:38) - PLID 64419 - Rescheduling Queue - Notes for Appts
		if (m_hExtraNotesIcon != NULL) {
			DestroyIcon((HICON)m_hExtraNotesIcon);
		}

	}NxCatchAll(__FUNCTION__);
}

void CAppointmentsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAppointmentsDlg)
	DDX_Control(pDX, IDC_SHOW_NO_SHOW_REASON, m_btnShowReasonForNoShow);
	DDX_Control(pDX, IDC_INCLUDE_CANCELED_DATE_COLUMNS, m_btnShowCancelledDate);
	DDX_Control(pDX, IDC_INCLUDE_MODIFIED_DATE_COLUMNS, m_btnShowModifiedDate);
	DDX_Control(pDX, IDC_APPTS_REMEMBER_COL_SETTINGS, m_btnRememberColumnSettings);
	DDX_Control(pDX, IDC_APPT_BKG, m_bg);
	DDX_Control(pDX, IDC_APPTS_NUM_CANCELLATIONS, m_edtNumCancellations);
	DDX_Control(pDX, IDC_APPTS_NUM_NO_SHOWS, m_edtNumNoShows);
	DDX_Control(pDX, IDC_SHOWCANCELLED, m_btnShowCancelled);
	DDX_Control(pDX, IDC_SHOWNOSHOWS, m_btnShowNoShows);
	DDX_Control(pDX, IDC_RECALL_PATIENT, m_btnRecall);	// (j.armen 2012-02-29 10:54) - PLID 48487
	DDX_Control(pDX, IDC_BTN_PATIENT_ENCOUNTER, m_btnPatientEncounter); // (b.savon 2014-12-01 10:33) - PLID 53162 - Connectathon - HL7 Dialog for Patient Encounter
	//}}AFX_DATA_MAP
}

// (j.jones 2007-11-21 14:20) - PLID 28147 - added ability to create a new inventory allocation
// (j.jones 2008-03-18 14:26) - PLID 29309 - added ability to create a new inventory order
// (j.jones 2008-06-23 16:16) - PLID 30455 - added ability to create a bill
// (j.gruber 2008-09-11 11:08) - PLID 30282 - added ability to edit an allocation
// (j.jones 2009-08-28 12:55) - PLID 35381 - added case history options
BEGIN_MESSAGE_MAP(CAppointmentsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAppointmentsDlg)
	ON_BN_CLICKED(IDC_GOTOAPP, OnGotoAppointment)
	ON_COMMAND(ID_APPT_RESCHEDULE, OnRescheduleAppointment)
	ON_COMMAND(ID_APPT_DELETE, OnDeleteAppointment)
	ON_COMMAND(ID_APPT_RESTORE, OnRestoreAppointment)
	ON_COMMAND(ID_APPT_CANCEL, OnCancelAppointment)
	ON_COMMAND(ID_APPT_NOSHOW, OnMarkNoShow)
	ON_COMMAND(ID_APPT_SHOW, OnMarkShow)
	ON_COMMAND(ID_APPT_NEW_INV_ALLOCATION, OnNewInvAllocation)
	ON_COMMAND(ID_APPT_EDIT_INV_ALLOCATION, OnEditInvAllocation)
	ON_COMMAND(ID_APPT_CREATE_CASE_HISTORY, OnApptCreateCaseHistory)
	ON_COMMAND(ID_APPT_EDIT_CASE_HISTORY, OnApptEditCaseHistory)
	ON_COMMAND(ID_APPT_NEW_INV_ORDER, OnNewInvOrder)
	ON_COMMAND(ID_APPT_NEW_BILL, OnNewBill)
	ON_COMMAND(ID_APPT_VIEW_ELIGIBILITY_RESPONSES, OnViewEligibilityResponses)
	ON_COMMAND(ID_APPT_CONFIRMED, OnMarkConfirmed)
	ON_COMMAND(ID_APPT_MOVE_UP, OnMarkMoveUp)
	ON_COMMAND(ID_APPT_UNCONFIRMED, OnMarkUnConfirmed)
	ON_COMMAND(ID_APPT_REMOVE_MOVE_UP, OnRemoveMoveUp)
	ON_COMMAND(ID_APPT_LINK, OnLinkAppointment)
	ON_BN_CLICKED(IDC_INCLUDE_MODIFIED_DATE_COLUMNS, OnIncludeModifiedDateColumns)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_INCLUDE_CANCELED_DATE_COLUMNS, OnIncludeCancelledDateColumns)
	ON_BN_CLICKED(IDC_APPTS_REMEMBER_COL_SETTINGS, OnRememberColumnSettings)
	ON_WM_SHOWWINDOW()
	ON_COMMAND(ID_APPT_GOTO, OnGotoAppointment)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_BN_CLICKED(IDC_SHOW_NO_SHOW_REASON, OnShowNoShowReason)
	ON_COMMAND(ID_APPT_LEFTMESSAGE, OnMarkLeftMessage)
	ON_BN_CLICKED(IDC_SHOWCANCELLED, OnShowCancelled)
	ON_BN_CLICKED(IDC_SHOWNOSHOWS, OnShowNoShows)
	ON_BN_CLICKED(IDC_RECALL_PATIENT, OnBtnRecallClicked)	// (j.armen 2012-02-29 10:33) - PLID 48487
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_PATIENT_ENCOUNTER, &CAppointmentsDlg::OnBnClickedBtnPatientEncounter)
	ON_COMMAND(ID_APPT_MANAGE_PAYMENT_PROFILES, OnManagePaymentProfiles)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CAppointmentsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAppointmentsDlg)
	ON_EVENT(CAppointmentsDlg, IDC_APPOINT_LIST, 3, CAppointmentsDlg::DblClickCellApptList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CAppointmentsDlg, IDC_APPOINT_LIST, 6, CAppointmentsDlg::RButtonDownApptList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CAppointmentsDlg, IDC_APPOINT_LIST, 10, CAppointmentsDlg::EditingFinishedApptList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CAppointmentsDlg, IDC_APPOINT_LIST, 9, CAppointmentsDlg::EditingFinishingApptList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CAppointmentsDlg, IDC_APPOINT_LIST, 18, CAppointmentsDlg::RequeryFinishedApptList, VTS_I2)
	ON_EVENT(CAppointmentsDlg, IDC_APPOINT_LIST, 2, CAppointmentsDlg::SelChangedApptList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CAppointmentsDlg, IDC_APPOINT_LIST, 8, CAppointmentsDlg::EditingStartingApptList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CAppointmentsDlg, IDC_APPOINT_LIST, 22, CAppointmentsDlg::ColumnSizingFinishedApptList, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CAppointmentsDlg, IDC_APPOINT_LIST, 19, CAppointmentsDlg::LeftClickApptList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CAppointmentsDlg message handlers

void CAppointmentsDlg::SetColor(OLE_COLOR nNewColor)
{
	m_bg.SetColor(nNewColor);
	CPatientDialog::SetColor(nNewColor);
}

void CAppointmentsDlg::LoadIncludeModifiedDateColumns()
{
	// Load the data and set the checkbox on screen appropriately
	if (GetRemotePropertyInt("ApptTabIncludeModifiedDate", 0, 0, GetCurrentUserName(), false) != 0) {
		CheckDlgButton(IDC_INCLUDE_MODIFIED_DATE_COLUMNS, 1);
	} else {
		CheckDlgButton(IDC_INCLUDE_MODIFIED_DATE_COLUMNS, 0);
	}

	// Reflect the state of the checkbox
	ReflectIncludeModifiedDateColumns();
}

void CAppointmentsDlg::ReflectIncludeModifiedDateColumns()
{
	if (IsDlgButtonChecked(IDC_INCLUDE_MODIFIED_DATE_COLUMNS)) {
		// Make sure the modified date and modified by fields have the csVisible flag
		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pApptList->GetColumn(alcModifiedDate);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthData);
		pCol->PutStoredWidth(40);
		pCol = m_pApptList->GetColumn(alcModifiedBy);
		pCol->PutStoredWidth(40);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthData);
	} else {
		// Make sure the modified date and modified by fields don't have the csVisible flag
		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pApptList->GetColumn(alcModifiedDate);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth);
		pCol->PutStoredWidth(0);
		pCol = m_pApptList->GetColumn(alcModifiedBy);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth);
		pCol->PutStoredWidth(0);
	}
}

BOOL CAppointmentsDlg::OnInitDialog() 
{
	try {

		m_IsComboVisible = FALSE;
		CNxDialog::OnInitDialog();

		// (j.dinatale 2011-06-24 17:06) - PLID 30025 - Should really bulk cache properties
		g_propManager.CachePropertiesInBulk("AppointmentsDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'ApptDlgShowCancelled' OR "
			"Name = 'ApptDlgShowNoShows' OR "
			"Name = 'ApptTabIncludeNoShowReasons' OR "
			"Name = 'DefaultAppointmentsColumnSizes' OR "
			"Name = 'ApptTabIncludeCancelledDate' OR "
			"Name = 'RememberAppointmentsColumns' OR "
			"Name = 'ShowVoidSuperbillPrompt' OR "
			"Name = 'ApptAutoOpenResEntry' OR "
			"Name = 'ApptTabIncludeModifiedDate' OR "
			"Name = 'ShowPatientEncounter' "
			")",
			_Q(GetCurrentUserName()));
		
		// (j.dinatale 2011-06-24 17:10) - PLID 30025 - Set the checks accordingly for show cancelled/no show
		m_btnShowCancelled.SetCheck(GetRemotePropertyInt("ApptDlgShowCancelled", 1, 0, GetCurrentUserName(), true));
		m_btnShowNoShows.SetCheck(GetRemotePropertyInt("ApptDlgShowNoShows", 1, 0, GetCurrentUserName(), true));

		m_pApptList = BindNxDataList2Ctrl(IDC_APPOINT_LIST, false);

		// (j.armen 2012-03-28 09:36) - PLID 48480 - Only show the recall button if they have the license for it
		//	Moved permission check into this section
		if (g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent))
		{
			m_btnRecall.AutoSet(NXB_RECALL);	// (j.armen 2012-02-29 10:33) - PLID 48487
			m_btnRecall.ShowWindow(SW_SHOWNA);

			// (a.wilson 2012-3-23) PLID 48472 - check whether current user has permission to access recall system.
			if ((GetCurrentUserPermissions(bioRecallSystem) & (sptRead | sptReadWithPass))) {
				m_btnRecall.EnableWindow(TRUE);
			}
			else {
				m_btnRecall.EnableWindow(FALSE);
			}
		}
		else
		{
			m_btnRecall.EnableWindow(FALSE);
			m_btnRecall.ShowWindow(SW_HIDE);
		}

		// (b.savon 2014-12-01 10:33) - PLID 53162 - Connectathon - HL7 Dialog for Patient Encounter -- Hide/Show functionality
		if (GetRemotePropertyInt("ShowPatientEncounter", 0, 0, "<None>") == 0){
			m_btnPatientEncounter.EnableWindow(FALSE);
			m_btnPatientEncounter.ShowWindow(SW_HIDE);
		}
		else{
			m_btnPatientEncounter.EnableWindow(TRUE);
			m_btnPatientEncounter.ShowWindow(SW_SHOW);
			m_btnPatientEncounter.AutoSet(NXB_ENCOUNTER);
		}

		// Optionally hide the "modified by","modified date", "canceled by", and "canceled date" columns
		LoadIncludeModifiedDateColumns();
		LoadIncludeCancelledDateColumns();
		LoadIncludeNoShowReasonColumn();

		LoadShowStateArray();

		// (a.wilson 2012-3-23) PLID 48472 - check whether current user has permission to access recall system.
		if ((GetCurrentUserPermissions(bioRecallSystem) & (sptRead | sptReadWithPass))) {
			GetDlgItem(IDC_RECALL_PATIENT)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_RECALL_PATIENT)->EnableWindow(FALSE);
		}

		// (a.walling 2010-10-13 16:25) - PLID 40977 - Wait until UpdateView. This was effectively refreshing twice upon first load.
		//GenerateWhere();
		m_bAllowUpdate = true;

		// (b.savon 2012-03-29 11:47) - PLID 49074 - Load the icon
		m_hiRecall = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_RECALL), IMAGE_ICON, 16, 16, 0);
		// (j.gruber 2014-12-15 12:38) - PLID 64419 - Rescheduling Queue - Notes for Appts
		m_hExtraNotesIcon = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BILL_NOTES), IMAGE_ICON, 16, 16, 0);

		// (j.jones 2014-06-09 09:26) - PLID 62335 - moved this load to OnInit
		if (GetRemotePropertyInt("RememberAppointmentsColumns", 0, 0, GetCurrentUserName(), true) == 1) {
			CheckDlgButton(IDC_APPTS_REMEMBER_COL_SETTINGS, TRUE);
			SetColumnSizes();
		}
		else {
			CheckDlgButton(IDC_APPTS_REMEMBER_COL_SETTINGS, FALSE);
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CAppointmentsDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {
		if(!m_bAllowUpdate) 
			return;

		//set the where clause to be just for this patient
		long nCurrentlyLoadedID = m_id;
		m_id = GetActivePatientID();

		// (a.walling 2010-10-13 16:12) - PLID 40977
		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;
		}
		
		// (j.jones 2014-08-07 15:10) - PLID 63168 - added tablechecker for ShowStates
		if (m_AptShowStateChecker.Changed()) {
			LoadShowStateArray();
		}

		// (a.walling 2010-10-13 16:23) - PLID 40977
		if (bForceRefresh || m_ForceRefresh) {
			// (j.dinatale 2011-08-12 17:59) - PLID 30025 - Renamed to GenerateWhere() to RefreshList(), to improve readability
			RefreshList();

			//disable the gotoAppointment button since nothing is selected 
			GetDlgItem(IDC_GOTOAPP)->EnableWindow(false); 	

			LoadNoShowCancelBoxes();
			// (a.walling 2013-12-12 16:51) - PLID 59999 - Recalls being reloaded / queries constantly for recall button color
			m_btnRecall.SetTextColor(RecallUtils::GeneratePatientRecallStatusTextColor(m_id));
		}

		//(a.wilson 2012-3-5) PLID 48485 - check to see what the current patients 
		//recall status is, then update the recall buttons text color.
		UpdateWindow();

		m_ForceRefresh = false;

	} NxCatchAll(__FUNCTION__);
}

void CAppointmentsDlg::LoadNoShowCancelBoxes() {

	// (j.gruber 2010-01-11 14:19) - PLID 22964 - load the number boxes
	long nActivePatient = GetActivePatientID();
	// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading Appointments dialog
	_RecordsetPtr rsCounts = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT (SELECT Count(ID) FROM AppointmentsT WHERE PatientID = {INT} AND Status = 4) AS NumCancel, "
		" (SELECT Count(ID) FROM AppointmentsT WHERE PatientID = {INT} AND ShowState = 3 AND Status <> 4) AS NumNoShows ", nActivePatient, nActivePatient);

	if (!rsCounts->eof) {
		SetDlgItemInt(IDC_APPTS_NUM_CANCELLATIONS, AdoFldLong(rsCounts, "NumCancel", 0));
		SetDlgItemInt(IDC_APPTS_NUM_NO_SHOWS, AdoFldLong(rsCounts, "NumNoShows", 0));
	}

}

// (j.dinatale 2011-08-12 17:59) - PLID 30025 - Renamed to GenerateWhere() to RefreshList(), to improve readability
void CAppointmentsDlg::RefreshList()
{
	// (j.gruber 2014-12-15 17:42) - PLID 64419 - Rescheduling Queue - Moved out of resources for easier reading
	CString strFrom = "(SELECT ResBasicQ.*, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
		"dbo.GetResourceString(ResBasicQ.ID) AS Item, "
		"CASE WHEN ResBasicQ.AptPurposeName <> '' THEN AptTypeT.Name + ' - ' + ResBasicQ.AptPurposeName ELSE AptTypeT.Name END AS Purpose, "
		"CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS Color, 0 AS SetID, '' AS Day "
		"FROM AptTypeT RIGHT JOIN PersonT RIGHT JOIN "
		"( "
		"	SELECT AppointmentsT.CancelledDate, AppointmentsT.CancelledBy, "
		"	CASE WHEN AppointmentsT.CancelReasonID IS NULL THEN AppointmentsT.CancelledReason ELSE AptCancelReasonT.Description END AS CancelledReason, "
		"	CASE WHEN AppointmentsT.NoShowReasonID IS NULL THEN AppointmentsT.NoShowReason ELSE AptNoShowReasonT.Description END AS NoShowReason, "
		"	AppointmentsT.ID, AppointmentsT.PatientID, AppointmentsT.AptTypeID, "
		"	dbo.GetPurposeString(AppointmentsT.ID) AS AptPurposeName, "
		"	CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) AS Date, "
		"	convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS StartTime, "
		"	AppointmentsT.Confirmed, AppointmentsT.Notes, CONVERT(bit, CASE WHEN WaitingListT.ID > 0 THEN 1 ELSE 0 END) AS MoveUp, "
		"	AppointmentsT.LocationID, AppointmentsT.RecordID, AppointmentsT.Status, PersonT.Archived, "
		"	AppointmentsT.ShowState, AppointmentsT.CreatedDate, AppointmentsT.LastLM, AppointmentsT.CreatedLogin, "
		"	AppointmentsT.ModifiedDate, AppointmentsT.ModifiedLogin, LocationsT.Name AS LocName, 		"
		"	CASE WHEN Recall.RecallAppointmentID IS NULL THEN CONVERT(bit, 0) ELSE CONVERT(bit, 1) END AS HasRecall,  "
		"	CONVERT(bit, CASE WHEN NotesQ.AppointmentID IS NOT NULL THEN 1 ELSE 0 END) AS HasExtraNotes, "
		"   CONVERT(bit, CASE WHEN ReschedulingQueueT.AppointmentID IS NOT NULL THEN 1 ELSE 0 END) AS IsInReschedulingQueue "
		"	FROM AppointmentsT LEFT JOIN WaitingListT ON WaitingListT.AppointmentID = AppointmentsT.ID "
		"	LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
		"	LEFT JOIN AptCancelReasonT ON AppointmentsT.CancelReasonID = AptCancelReasonT.ID "
		"	LEFT JOIN AptNoShowReasonT ON AppointmentsT.NoShowReasonID = AptNoShowReasonT.ID "
		"	LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID "
		"   LEFT JOIN ReschedulingQueueT ON AppointmentsT.ID = ReschedulingQueueT.AppointmentID "
		"	LEFT JOIN(SELECT AppointmentID FROM Notes WHERE AppointmentID IS NOT NULL GROUP BY AppointmentID) NotesQ  ON AppointmentsT.ID = NotesQ.AppointmentID "
		"	OUTER APPLY (SELECT TOP 1 RecallAppointmentID FROM RecallT WHERE AppointmentsT.ID = RecallT.RecallAppointmentID) Recall"
		") ResBasicQ ON PersonT.ID = ResBasicQ.PatientID ON AptTypeT.ID = ResBasicQ.AptTypeID) AS ResComplexQ ";

	m_pApptList->FromClause = (LPCTSTR)strFrom;

	// (a.walling 2010-10-13 16:22) - PLID 40977
	m_id = GetActivePatientID();
	CString strWhere;
	strWhere.Format("PatientID = %li", GetActivePatientID());

	// (j.dinatale 2011-06-24 17:47) - PLID 37940
	BOOL bShowCancel = m_btnShowCancelled.GetCheck();
	BOOL bShowNoShows = m_btnShowNoShows.GetCheck();

	if(!bShowCancel){
		strWhere += " AND Status <> 4 ";
	}

	if(!bShowNoShows){
		strWhere += " AND ShowState <> 3 ";
	}

	m_pApptList->WhereClause = (LPCTSTR)strWhere;
	m_pApptList->Requery();
	m_ForceRefresh = false;
}

void CAppointmentsDlg::SetAppointmentColors()
{
	// (j.gruber 2015-01-08 14:37) - PLID 64545 - Change how we loop for DL2
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->GetFirstRow();
	while (pRow) {
	
		OLE_COLOR color = pRow->GetValue(alcColor).lVal;
		// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
		color = GetDarkerColorForApptText(color);
		pRow->PutForeColor(color);

		pRow = pRow->GetNextRow();
	}
}

void CAppointmentsDlg::OnGotoAppointment() 
{
	SwitchToAppointment();
}

void CAppointmentsDlg::SwitchToAppointment() {
	
	try {
		EnsureRemoteData();
		//Open the recordset
		//First make sure the they have selected a record
		// (j.gruber 2015-01-08 14:40) - PLID 64545 - Change to DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
				
		if (!pRow) {
			MsgBox("Please select an appointment");
			return;
		}
		//Now, see whether the appointments been cancelled
		// (a.walling 2014-12-22 16:44) - PLID 64382 - handle astToBeRescheduled
		long nStatus = VarLong(pRow->GetValue(alcStatus));
		if(nStatus == astCancelled || nStatus == astCancelledNoShow || nStatus == astToBeRescheduled) {
			MsgBox("This appointment has been cancelled");
			return;
		}
	
		CMainFrame  *pMainFrame;
		pMainFrame = GetMainFrame();
		if (pMainFrame != NULL) {
			if (pMainFrame->FlipToModule(SCHEDULER_MODULE_NAME)) {
				CNxTabView *pView = pMainFrame->GetActiveView();
				if (pView && pView->IsKindOf(RUNTIME_CLASS(CSchedulerView))) {
					((CSchedulerView *)pView)->OpenAppointment(VarLong(pRow->GetValue(alcResID)), GetRemotePropertyInt("ApptAutoOpenResEntry", 1, 0, GetCurrentUserName(), true) ? TRUE : FALSE);
				}//end pView
			}
		}//end pMainFrame
		else {
			//MainFrame pointer is null
			MsgBox(RCS(IDS_APPOINTMENT_ERROR_1667));
		}//end else
	}NxCatchAll("Error in SwitchToAppointment()");
}


void CAppointmentsDlg::OnDeleteAppointment()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
	if (!pRow){
		return;
	}

	try {

		long lApptID = VarLong(pRow->GetValue(alcResID));
	
		// (a.walling 2013-01-17 09:26) - PLID 54651 - Check for any appointments linked to EMNs
		CString strLinkedEMNs = GetLinkedEMNDescriptionsFromAppointment(lApptID);
		if (!strLinkedEMNs.IsEmpty()) {
			AfxMessageBox(FormatString("This appointment is linked to the following EMN data and may not be deleted:\r\n\r\n%s", strLinkedEMNs), MB_ICONERROR);
			return;
		}

		// (j.jones 2006-10-09 15:04) - PLID 22887 - if currently tied to a room, disallow deletion
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if(ReturnsRecordsParam("SELECT ID FROM RoomAppointmentsT WHERE AppointmentID = {INT} AND StatusID <> -1", lApptID)) {
			AfxMessageBox("This appointment is currently assigned to a room. You may not cancel an appointment while it is assigned to a room.");
			return;
		}

		// (c.haag 2009-10-12 13:14) - PLID 35722 - If currently tied to a history item, disallow deletion. Currently only applies to photos.
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if(ReturnsRecordsParam("SELECT ID FROM MailSentProcedureT WHERE AppointmentsID = {INT}", lApptID)) {
			AfxMessageBox("This appointment is currently assigned to a patient photo.\n\nYou may not cancel an appointment while it is assigned to a photo.");
			return;
		}

		CString str = "";

		BOOL bTracking = FALSE, bCaseHistory = FALSE;

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if(ReturnsRecordsParam("SELECT ID FROM ProcInfoT WHERE SurgeryApptID = {INT}",lApptID) || 
			ReturnsRecordsParam("SELECT ProcInfoID FROM ProcInfoAppointmentsT WHERE AppointmentID = {INT}",lApptID)) {
			bTracking = TRUE;
		}
		
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if(IsSurgeryCenter(true) && ReturnsRecordsParam("SELECT ID FROM CaseHistoryT WHERE AppointmentID = {INT}",lApptID)) {
			bCaseHistory = TRUE;
		}

		if(bTracking && bCaseHistory)
			str = "This appointment is attached to both a tracked procedure and a case history. Are you sure you wish to delete this appointment?";
		else if(bTracking)
			str = "This appointment is attached to a tracked procedure. Are you sure you wish to delete this appointment?";
		else if(bCaseHistory) {

			if(theApp.m_arypCaseHistories.GetSize() > 0) {
				long nSize = theApp.m_arypCaseHistories.GetSize();
				for (long i=0; i<nSize; i++) {
					CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
					if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
						MessageBox("Please close all open Case Histories before continuing.");
						return;
					}
				}
			}

			str = "This appointment is attached to a case history. Are you sure you wish to delete this appointment?";
		}

		if(str == "")
			str = CString(RCS(IDS_QUESTION_DELETE_APPOINTMENT));

		if(HasPermissionForResource(lApptID,sptDelete,sptDeleteWithPass)) {
			if (IDYES == MsgBox(MB_YESNO, str)) {
					//DRT 7/8/2005 - PLID 16664 - I hate to do this here, but the way the whole appointment delete thing is structured
					//	I don't have much choice short of rewriting all of it.  I added code to AppointmentDeleteNoHistory() that warns
					//	for superbill IDs when deleting appointments.  However, that function is never called with prompting turned on.
					//	So I'm going to duplicate the prompting here, since this is the primary spot for appointment deleting, and the only
					//	place that the user should be warned.
					//If any kind souls come through and redesign this delete structure, this warning can be thrown out in favor of the
					//	one in the AppointmentDeleteNoHistory() function.
					bool bVoidSuperbills = false;
					if((GetRemotePropertyInt("ShowVoidSuperbillPrompt", 1, 0, "<None>", false) == 1) && (GetCurrentUserPermissions(bioVoidSuperbills) & sptWrite))
					{
						_RecordsetPtr prsSuperbill = CreateRecordset("SELECT SavedID FROM PrintedSuperbillsT WHERE ReservationID = %li AND Void = 0", lApptID);
						CString strSuperbillIDs = "";
						long nSuperbillIDCnt = 0;
						while(!prsSuperbill->eof) {
							long nID = AdoFldLong(prsSuperbill, "SavedID", -1);
							if(nID > -1) {
								CString str;	str.Format("%li, ", nID);
								strSuperbillIDs += str;
								nSuperbillIDCnt++;
							}

							prsSuperbill->MoveNext();
						}
						strSuperbillIDs.TrimRight(", ");
						prsSuperbill->Close();

						if(nSuperbillIDCnt > 0) {
							//They are tied to superbills, we will warn the user and give them an opportunity to give up
							CString strFmt;
							strFmt.Format("This appointment is tied to %li superbill%s (ID%s:  %s). "
								"Would you like to mark these superbills VOID?\r\n\r\n"
								" - If you choose YES, the appointment will be deleted and all related superbill IDs will be marked VOID.\r\n"
								" - If you choose NO, the appointment will be deleted, but all related superbill IDs will remain.\r\n"
								" - IF you choose CANCEL, the appointment will remain.", nSuperbillIDCnt, nSuperbillIDCnt == 1 ? "" : "s", nSuperbillIDCnt == 1 ? "" : "s", strSuperbillIDs);

							int nRes = AfxMessageBox(strFmt, MB_YESNOCANCEL);
							if(nRes == IDCANCEL)
								return;
							else if(nRes == IDYES) 
								bVoidSuperbills = true;
						}
					}

					// (j.jones 2007-11-09 16:56) - PLID 27987 - handle inventory allocations linked to appts.
					// Check to see if there are any open allocations linked to this appt.
					bool bReturnAllocations = false;
					if(ReturnsRecords("SELECT TOP 1 ID FROM PatientInvAllocationsT WHERE AppointmentID = %li AND Status = %li", lApptID, InvUtils::iasActive)) {
						//there are undeleted allocations, so warn the user
						//(yes, the data structure lets them have more than one, but that's rare - and we may forbid it -
						//so make the message make sense in the 99% of cases where they have only one allocation)
						// (j.jones 2007-11-29 09:57) - PLID 28196 - reworded this to properly explain that only
						// unused products can be returned to stock
						CString strFmt = "This patient has an inventory allocation tied to this appointment.\n"
							"Would you like to return any unused products in this allocation back to purchased stock?\r\n\r\n"

							" - If you choose YES, the appointment will be deleted and any unused allocated products will be returned to stock.\r\n"
							" - If you choose NO, the appointment will be deleted and all allocated products will remain allocated to this patient.\r\n"
							" - If you choose CANCEL, the appointment and allocated products will remain.";

						int nRes = AfxMessageBox(strFmt, MB_YESNOCANCEL);
						if(nRes == IDCANCEL)
							return;
						else if(nRes == IDYES)
							bReturnAllocations = true;
					}

					// (j.jones 2008-03-18 15:48) - PLID 29309 - warn if the appt. is linked to an order
					// (j.jones 2008-09-24 16:37) - PLID 31493 - changed to check if multiple orders were linked
					_RecordsetPtr rsOrders = CreateParamRecordset("SELECT ID FROM OrderAppointmentsT "
						"WHERE AppointmentID = {INT}", lApptID);
					if(!rsOrders->eof) {
						CString strWarning = "This appointment is linked to an inventory order, are you sure you wish to delete it?";
						if(rsOrders->GetRecordCount() > 1) {
							strWarning = "This appointment is linked to multiple inventory orders, are you sure you wish to delete it?";
						}

						int nRes = MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO);
						if(nRes != IDYES) {
							return;
						}
					}
					rsOrders->Close();

					//

	//				long nAuditID = BeginNewAuditEvent();					
	//				AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiApptStatus, lApptID, CString(m_pAppointments->GetValue(m_pAppointments->CurSel, 9).bstrVal), "Deleted", aepHigh, aetDeleted);
					// (a.walling 2008-05-23 09:01) - PLID 28411 - These events were being unapplied too soon. They should run post-deletion. You can even break out of deleting after this!
					// This is all handled in AppointmentDeleteNoHistory in globalschedutils.cpp.
					/*
					PhaseTracking::UnapplyEvent(GetActivePatientID(), PhaseTracking::ET_AppointmentCreated, lApptID);
					PhaseTracking::UnapplyEvent(GetActivePatientID(), PhaseTracking::ET_ActualAppointment, lApptID);
					*/

					//see if they want to delete the case histories
					_RecordsetPtr rs = CreateRecordset("SELECT Count(ID) AS CountOfID FROM CaseHistoryT WHERE AppointmentID = %li",lApptID);
					if(!rs->eof) {
						long Count = AdoFldLong(rs, "CountOfID",0);
						BOOL bDelete = FALSE;
						if(Count == 1) {
							if(IDYES == MsgBox(MB_ICONQUESTION|MB_YESNO,"This appointment had a case history associated with it. Do you want to delete this case history?")) {
								bDelete = TRUE;
							}
						}
						else if(Count > 1) {
							CString str;
							str.Format("This appointment had %li case histories associated with it. Do you want to delete these case histories?",Count);
							if(IDYES == MsgBox(MB_ICONQUESTION|MB_YESNO,str)) {
								bDelete = TRUE;
							}
						}

						CString strApptDesc;
						_RecordsetPtr rs2 = CreateParamRecordset("SELECT ID FROM CaseHistoryT WHERE AppointmentID = {INT}", lApptID);
						while(!rs2->eof) {

							long nCaseHistoryID = AdoFldLong(rs2, "ID",-1);
							bool bDeleted = false;
							if(bDelete) {
								// Make sure it hasn't been billed
								if (CCaseHistoryDlg::IsCaseHistoryBilled(nCaseHistoryID)) {
									AfxMessageBox("You may not delete this case history because it has already been billed.");
								}
								// (j.jones 2009-08-06 11:10) - PLID 7397 - added check to see if the case is linked to a PIC
								else if(CCaseHistoryDlg::IsCaseHistoryInProcInfo(nCaseHistoryID)) {
									if(IDYES == MessageBox("This case history is linked to a Procedure Information Center. Are you sure you wish to delete it?",
										"Practice", MB_ICONQUESTION|MB_YESNO)) {

										CCaseHistoryDlg::DeleteCaseHistory(nCaseHistoryID);
										bDeleted = true;
									}
								}
								else {
									CCaseHistoryDlg::DeleteCaseHistory(nCaseHistoryID);
									bDeleted = true;
								}
							}
							if(!bDeleted) {
								ExecuteSql("UPDATE CaseHistoryT SET AppointmentID = NULL WHERE ID = %li", nCaseHistoryID);
								//TES 1/9/2007 - PLID 23575 - Audit that this case history's appointment has changed.
								if(strApptDesc.IsEmpty()) {
									// This is designed to display the appointment information in the same format as it appears
									// on the case history itself, see CCaseHistoryDlg::DisplayAppointmentDesc().
									COleDateTime dtDate = VarDateTime(pRow->GetValue(alcDate));
									COleDateTime dtStartTime = VarDateTime(pRow->GetValue(alcStartTime));
									CString strType = VarString(GetTableField("AptTypeT INNER JOIN AppointmentsT ON AptTypeT.ID = AppointmentsT.AptTypeID","AptTypeT.Name", "AppointmentsT.ID", lApptID),"<No Type>");
									CString strPurpose = VarString(pRow->GetValue(alcPurpose));
									if(strPurpose.IsEmpty()) strPurpose = "<No Purpose>";
									CString strResource = VarString(pRow->GetValue(alcItem));
									strApptDesc.Format("%s %s, %s - %s, %s",FormatDateTimeForInterface(dtDate,NULL,dtoDate),
										FormatDateTimeForInterface(dtStartTime, DTF_STRIP_SECONDS, dtoTime),strType,strPurpose,strResource);
								}
								AuditEvent(GetActivePatientID(), GetActivePatientName(), BeginNewAuditEvent(), aeiCaseHistoryAppointment, nCaseHistoryID, strApptDesc, "<None> (Appt was deleted)", aepMedium, aetChanged);
							}
							rs2->MoveNext();
						}
						rs2->Close();
					}
					rs->Close();
					
					AppointmentDeleteNoHistory(lApptID, TRUE, false, NULL, NULL, bVoidSuperbills, bReturnAllocations);
					UpdateViewBySingleAppt(lApptID);
			}
		}
	}NxCatchAll("Error in OnDeleteAppointment()");
}

void CAppointmentsDlg::OnCancelAppointment()
{
	// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to DL2
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
	if(!pRow)
		return;

	try {

		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptDynamic2,sptDynamic2WithPass))
			return;

		m_bAllowUpdate = false;		

		AppointmentCancel(lApptID);		

		// (j.dinatale 2011-08-12 17:46) - PLID 30025 - remove the row instead of updating it if we know that we dont want to see it
		if(m_btnShowCancelled.GetCheck()){
			UpdateViewBySingleAppt(lApptID);
		}else{
			m_pApptList->RemoveRow(pRow);

			// (j.dinatale 2011-08-15 15:51) - PLID 30025 - apparently UpdateViewBySingleAppt ends up updating our row counts, so we need to do that too
			LoadNoShowCancelBoxes();
		}

		m_bAllowUpdate = true;

	}NxCatchAll("Error in OnCancelAppointment()");
}

void CAppointmentsDlg::OnRestoreAppointment()
{
	try {
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if(!pRow)
			return;

		int nResult;
		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(HasPermissionForResource(lApptID,sptDynamic2,sptDynamic2WithPass)) {
			nResult = MsgBox(MB_YESNO, RCS(IDS_QUESTION_RESTORE_APPOINTMENT));
		}
		else nResult = IDNO;
		if (nResult == IDYES) 
		{	
			m_bAllowUpdate = false;
			try{
				// (j.jones 2014-12-02 11:36) - PLID 64183 - added pParentWnd
				AppointmentUncancel(this, lApptID);
			}NxCatchAll("Error in OnRestoreAppointment()");
			UpdateViewBySingleAppt(lApptID);
			m_bAllowUpdate = true;
		}

	}NxCatchAll("Error 2 in CAppointmentsDlg::OnRestoreAppointment()");
}

void CAppointmentsDlg::OnMarkNoShow()
{	
	try{
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow){
			return;
		}

		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;

		//(e.lally 2005-07-21) PLID 16962 - We have a global scheduling utility for this so let's use it. The sql update 
			//statement did not have all the proper information included plus the Palm and PPC links were not
			//getting updated.
		//This global scheduling utility has a known bug that has audits performed when the no-show reason dlg is canceled.
		//That bug needs to be fixed in order for this change to also work properly.
		if(AppointmentMarkNoShow(lApptID)) {			

			// (j.dinatale 2011-08-12 17:46) - PLID 30025 - remove the row instead of updating it if we know that we dont want to see it
			if(m_btnShowNoShows.GetCheck()){
				UpdateViewBySingleAppt(lApptID);
			}else{
				m_pApptList->RemoveRow(pRow);

				// (j.dinatale 2011-08-15 15:51) - PLID 30025 - apparently UpdateViewBySingleAppt ends up updating our row counts, so we need to do that too
				LoadNoShowCancelBoxes();
			}

		}
	}NxCatchAll("Error in OnMarkNoShow()");
}

void CAppointmentsDlg::OnMarkShow()
{
	
	try {
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}	

		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;

		//(e.lally 2005-07-21) PLID 16962 - This is actually the code for marking the appointment as Pending.
			//We have a global scheduling utility for pending so let's use it. The sql update 
			//statement did not have all the proper information included plus the Palm and PPC links were not
			//getting updated.
			//The other function calls to update the appt table and the view were intermixed with the original 
			//code to mark an appointment Pending.
			//I verified that this new order will not affect the designed functionality.
		AppointmentMarkPending(lApptID);

		UpdateViewBySingleAppt(lApptID);

	}NxCatchAll("Error in OnMarkShow()");
}

void CAppointmentsDlg::OnMarkIn()
{
	try{
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}

		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;

		//(e.lally 2005-07-21) PLID 16962 - We have a global scheduling utility for this so let's use it. The sql update 
			//statement did not have all the proper information included plus the Palm and PPC links were not
			//getting updated.
			//The other function calls to update the appt table and the view were intermixed with the original 
			//code to mark an appointment In.
			//I verified that this new order will not affect the designed functionality.
		AppointmentMarkIn(lApptID);

		UpdateViewBySingleAppt(lApptID);

	}NxCatchAll("Error in OnMarkIn()");
}



void CAppointmentsDlg::OnMarkOut()
{
	try{

		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}	

		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;

		//(e.lally 2005-07-21) PLID 16962 - We have a global scheduling utility for this so let's use it. The sql update 
			//statement did not have all the proper information included plus the Palm and PPC links were not
			//getting updated.
			//The other function calls to update the appt table and the view were intermixed with the original 
			//code to mark an appointment Out.
			//I verified that this new order will not affect the designed functionality.
		AppointmentMarkOut(lApptID);

		UpdateViewBySingleAppt(lApptID);

	}NxCatchAll("Error in OnMarkOut()");
}

void CAppointmentsDlg::OnMarkPending()
{
	try {
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow){
			return;
		}
	
		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;

		//(e.lally 2005-07-21) PLID 16962 - We have a global scheduling utility for this so let's use it. The sql update 
			//statement did not have all the proper information included plus the Palm and PPC links were not
			//getting updated.
			//The other function calls to update the appt table and the view were intermixed with the original 
			//code to mark an appointment Pending.
			//I verified that this new order will not affect the designed functionality.
		AppointmentMarkPending(lApptID);

		UpdateViewBySingleAppt(lApptID);

	}NxCatchAll("Error in OnMarkPending()");
}

void CAppointmentsDlg::OnMarkReceived()
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}
	
		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;

		//(e.lally 2005-07-21) PLID 16962 - We have a global scheduling utility for this so let's use it. The sql update 
			//statement did not have all the proper information included.
			//The other function calls to update the appt table and the view were intermixed with the original 
			//code to mark an appointment Received.
			//I verified that this new order will not affect the designed functionality.
		AppointmentMarkReceived(lApptID);

		UpdateViewBySingleAppt(lApptID);

	}NxCatchAll("Error in OnMarkReceived()");
}

// (a.wilson 2014-08-21 14:33) - PLID 63170 - use the refreshappointmenttable() with all the parameters to prevent extra recordsets.
void CAppointmentsDlg::OnMarkUserDefinedShowState(EAppointmentShowState nState)
{
	try {
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}

		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;

		long nLocationID = -1, nStatus = -1;
		CString strOldStateName = "", strResourceIDs = "";
		COleDateTime dtStart = g_cdtMin, dtEnd = g_cdtMin;
		_RecordsetPtr rs = CreateParamRecordset("SELECT StartTime, EndTime, Status, ShowState, LocationID, dbo.GetResourceIDString(ID) AS ResourceIDs FROM AppointmentsT WHERE ID = {INT}", lApptID);
		if (!rs->eof) {
			strOldStateName = GetShowStateName((EAppointmentShowState)AdoFldLong(rs, "ShowState"));
			strResourceIDs = AdoFldString(rs, "ResourceIDs", "");
			nLocationID = AdoFldLong(rs, "LocationID");
			nStatus = (long)AdoFldByte(rs, "Status");
			dtStart = AdoFldDateTime(rs, "StartTime");
			dtEnd = AdoFldDateTime(rs, "EndTime");
		}
		rs->Close();
		
		ExecuteSql("UPDATE AppointmentsT SET ShowState = %d WHERE AppointmentsT.ID = %li", (long)nState, lApptID);

		long nPatientID = GetActivePatientID();
		CString strNewStateName = GetShowStateName(nState);
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(nPatientID, GetActivePatientName(), nAuditID, aeiApptShowState, lApptID, strOldStateName, strNewStateName, aepMedium);

		// send a network message
		CClient::RefreshAppointmentTable(lApptID, nPatientID, dtStart, dtEnd, nStatus, nState, nLocationID, strResourceIDs);

		UpdateViewBySingleAppt(lApptID);

		LogShowStateTime(lApptID, nState);
		
		// (z.manning, 07/26/2007) - PLID 14579 - See if the user wants to update the status of any linked appts.
		UpdateLinkedAppointmentShowState(lApptID, nState, strNewStateName);

		// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
		// (r.gonet 12/03/2012) - PLID 54107 - Updated to use refactored function.
		SendUpdateAppointmentHL7Message(lApptID);

	}NxCatchAll("Error in OnMarkUserDefinedShowState()");}

void CAppointmentsDlg::OnMarkConfirmed()
{
	try {
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}
		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;
		//(e.lally 2009-09-28) PLID 18809 - Use the global function
		AppointmentMarkConfirmed(lApptID, acsConfirmed);

		//update the view
		UpdateViewBySingleAppt(lApptID);
	
	}NxCatchAll("Error in OnMarkConfirmed()");
}

void CAppointmentsDlg::OnMarkUnConfirmed()
{
	try {
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}
		
		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;
		//(e.lally 2009-09-28) PLID 18809 - Use the global function
		AppointmentMarkConfirmed(lApptID, acsUnconfirmed);

		//update the view
		UpdateViewBySingleAppt(lApptID);
	}NxCatchAll("Error in OnMarkUnConfirmed()");
}

//(e.lally 2009-09-28) PLID 18809 - Created function to handle marking an appointment as Left Message
void CAppointmentsDlg::OnMarkLeftMessage()
{
	try {
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}

		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;

		//(e.lally 2009-09-28) PLID 18809 - Use the global function
		AppointmentMarkConfirmed(lApptID, acsLeftMessage);

		//update the view
		UpdateViewBySingleAppt(lApptID);

	}NxCatchAll(__FUNCTION__);
}

void CAppointmentsDlg::OnMarkMoveUp()
{
	try {
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}
	
		//TES 12/17/2008 - PLID 32478 - This ability is only available in the Enterprise edition, so check the license.
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Designating appointments as requesting a move up", "The_Scheduler_Module/manage_my_moveup_appointments.htm")) {
			return;
		}

		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;

		//(e.lally 2005-07-21) PLID 16962 - We have a global scheduling utility for this so let's use it. The sql update 
			//statement did not have all the proper information included plus the Palm and PPC links were not
			//getting updated.
		AppointmentMarkMoveUp(lApptID, TRUE);

		UpdateViewBySingleAppt(lApptID);
	}NxCatchAll("Error in OnMarkMoveUp()");
}

void CAppointmentsDlg::OnRemoveMoveUp()
{
	try {
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}

		//TES 12/17/2008 - PLID 32478 - This ability is only available in the Enterprise edition, so check the license.
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Designating appointments as requesting a move up", "The_Scheduler_Module/manage_my_moveup_appointments.htm")) {
			return;
		}

		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;

		//(e.lally 2005-07-21) PLID 16962 - We have a global scheduling utility for this so let's use it. The sql update 
			//statement did not have all the proper information included plus the Palm and PPC links were not
			//getting updated.
		AppointmentMarkMoveUp(lApptID, FALSE);

		UpdateViewBySingleAppt(lApptID);
	}NxCatchAll("Error in OnRemoveMoveUp()");
}

void CAppointmentsDlg::OnLinkAppointment()
{
	try {
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}	

		//TES 12/18/2008 - PLID 32508 - This is not available for Scheduler Standard users.
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Linking related appointments for a single patient")) {
			return;
		}

		long lApptID = VarLong(pRow->GetValue(alcResID));

		if(!HasPermissionForResource(lApptID,sptWrite,sptWriteWithPass))
			return;

		CResLinkDlg dlg(this);
		dlg.Open(lApptID, GetActivePatientID());

	}NxCatchAll("Error in OnLinkAppointment()");
}


void CAppointmentsDlg::SetDays()
{
	COleDateTime dt;
	CString day;

	// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2, so change how we loop
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->GetFirstRow();

	while (pRow)	
	{		
		dt = COleDateTime(pRow->GetValue(alcDate));
		day = FormatDateTimeForInterface(dt, "%a");
		pRow->PutValue(alcDay, (_bstr_t)day);

		pRow = pRow->GetNextRow();
	}
}

void CAppointmentsDlg::LoadShowStateArray()
{
	try {
		m_adwShowState.RemoveAll();
		m_astrShowState.RemoveAll();
		_RecordsetPtr prs = CreateRecordset("SELECT ID, [Name] FROM AptShowStateT ORDER BY ID");
		while (!prs->eof)
		{
			CString strName = AdoFldString(prs, "Name", "");
			m_adwShowState.Add(AdoFldLong(prs, "ID"));
			m_astrShowState.Add(strName);
			prs->MoveNext();
		}
	}
	NxCatchAll("Error refreshing appointment status list");
}

LRESULT CAppointmentsDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		if (wParam == NetUtils::AppointmentsT)
		{
			// (j.jones 2014-08-07 14:36) - PLID 63168 - we no longer respond to non-Ex
			// AppointmentsT messages, we only respond to Ex messages
			return 0;
		}
		else if (wParam == NetUtils::AptShowStateT)
		{
			// (j.jones 2014-08-07 15:12) - PLID 63168 - this is now handled in UpdateView
			// via a normal tablechecker object
			return 0;
		}
	} NxCatchAll("Error in CAppointmentsDlg::OnTableChanged");
	return 0;
}

// (j.jones 2014-08-07 14:33) - PLID 63168 - added an Ex handler
LRESULT CAppointmentsDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {

		switch (wParam) {
		case NetUtils::AppointmentsT:

			try {

				// (j.jones 2014-08-07 14:38) - PLID 63168 - ignore messages that
				// are not for our patient, or don't have an appointment ID
				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				long nApptID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiAppointmentID), -1);
				long nPatientID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiPatientID), -1);

				if (m_id != nPatientID || nApptID == -1) {
					//this change is not for our patient, or did not provide enough information
					return 0;
				}

				//DRT 1/24/2005 - PLID 15394 - Loading the appts tab can cause an UpdateView call, 
				//	which requeries the appt list.  If it's requerying, we have no need to update
				//	a single appt, it just got updated.
				if (m_pApptList->IsRequerying()) {
					return 0;
				}
				
				UpdateViewBySingleAppt(nApptID);

			}NxCatchAll("Error in CAppointmentsDlg::OnTableChangedEx:AppointmentsT");

			break;
		}

	} NxCatchAll(__FUNCTION__);

	return 0;
}

void CAppointmentsDlg::OnIncludeModifiedDateColumns() 
{
	// Reflect the state of the checkbox
	ReflectIncludeModifiedDateColumns();

	// Save the setting to data
	BOOL bInclude = (IsDlgButtonChecked(IDC_INCLUDE_MODIFIED_DATE_COLUMNS) != FALSE);
	SetRemotePropertyInt("ApptTabIncludeModifiedDate", bInclude ? 1 : 0, 0, GetCurrentUserName());

	// If we're adding them back then we need to requery because they may not have been visible 
	// the last time we requeried
	// (b.cardillo 2003-07-14 13:48) I'm beginning to think the datalist should still load data 
	// even if the column doesn't have the csVisible flag.  Then the flag could finally fulfill 
	// the purpose people seem to be trying to use it for.  (i.e. we wouldn't have to requery here...)
	if (bInclude) {
		m_pApptList->Requery();
	}

	// now save the column sizes
	SaveColumnSizes();
}

void CAppointmentsDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	try {
		// (j.gruber 2015-01-08 14:45) - PLID 64545 - Change to a DL2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;

		if (!pRow) {
			return;
		}

		long nAppointmentID = VarLong(pRow->GetValue(alcResID));

		// (d.moore 2007-05-22 11:29) - PLID 4013 - Changed the MoveUp bit to query the waiting list table.
		// (j.jones 2009-08-28 13:07) - PLID 35381 - added AptTypeCategory
		// (a.walling 2014-12-22 15:51) - PLID 64418 - Check if on Rescheduling Queue and provide option to open if so
		_RecordsetPtr rs = CreateParamRecordset(
			"DECLARE @ApptID INT; "
			"SET @ApptID = {INT}; \r\n"
			"SELECT PatientID, Status, ShowState, Confirmed, AptTypeT.Category AS AptTypeCategory, "
			"CONVERT(bit, CASE WHEN EXISTS (SELECT ID FROM WaitingListT WHERE AppointmentID = @ApptID) THEN 1 ELSE 0 END) AS MoveUp, "
			"CONVERT(bit, CASE WHEN EXISTS (SELECT AppointmentID FROM ReschedulingQueueT WHERE AppointmentID = @ApptID) THEN 1 ELSE 0 END) AS InReschedulingQueue "
			"FROM AppointmentsT "
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE AppointmentsT.ID = @ApptID",
			nAppointmentID);
		if (rs->eof) {
			rs->Close();
			return;
		}
		long nPatientID = AdoFldLong(rs, "PatientID");
		_variant_t vartmp = rs->Fields->GetItem("Status")->Value;
		_variant_t varShow = rs->Fields->GetItem("ShowState")->Value;
		BOOL nConfirmed = AdoFldLong(rs, "Confirmed", 0);
		BOOL bMoveUp = AdoFldBool(rs, "MoveUp", FALSE);
		bool bInReschedulingQueue = !!AdoFldBool(rs, "InReschedulingQueue", FALSE);
		long nAptTypeCategory = AdoFldByte(rs, "AptTypeCategory", -1);

		// (j.jones 2010-09-24 11:45) - PLID 34518 - added ability to create a copayment
		CArray<long, long> aryCopayInsuredPartyIDs;
		long nPrimaryInsuredPartyID = -1;
		const int MIN_ID_COPAYMENT_RESPS = 20000;

		rs->Close();
		if (vartmp.vt != VT_NULL && varShow.vt != VT_NULL)
		{
			CMenu* pMenu;
			pMenu = new CMenu;
			pMenu->CreatePopupMenu();

			// (c.haag 2011-06-17) - PLID 36477 - We now handle some of the menu options in here (code refactoring)
			CApptPopupMenuManager apmm(*pMenu, nPrimaryInsuredPartyID, aryCopayInsuredPartyIDs, this, nAppointmentID, m_id);

			if (bInReschedulingQueue) {
				bool canAccessScheduler = !!(GetCurrentUserPermissions(bioSchedulerModule) & (sptView | sptViewWithPass));
				pMenu->InsertMenu(-1, MF_BYPOSITION|(canAccessScheduler ? 0 : MF_DISABLED | MF_GRAYED), ID_APPT_RESCHEDULE, "Resc&hedule Appointment");
				pMenu->InsertMenu(-1, MF_BYPOSITION);
			}

			if (vartmp.lVal == 4){
				pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_RESTORE, GetStringOfResource(IDS_RESTORE_APPOINTMENT));
				pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_DELETE, GetStringOfResource(IDS_DELETE_APPOINTMENT));
			}
			else{
				pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_CANCEL, GetStringOfResource(IDS_CANCEL_APPOINTMENT));

				pMenu->InsertMenu(-1, MF_BYPOSITION);
				if (VarLong(varShow) == 3) {
					pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_SHOW, GetStringOfResource(IDS_UNMARK_NO_SHOW));
				}
				else {
					pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_NOSHOW, GetStringOfResource(IDS_MARK_NO_SHOW));
					/*
					if (varShow.bVal != 0)
					pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_PENDING, "Mark as &Pending");
					if (varShow.bVal != 1)
					pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_IN, "Mark as &In");
					if (varShow.bVal != 2)
					pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_OUT, "Mark as &Out");
					if (varShow.bVal != 4)
					pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_RECEIVED, "Mark as &Received");*/

					// (j.jones 2014-08-07 15:10) - PLID 63168 - added tablechecker for ShowStates,
					// make sure it's up to date when they right click
					if (m_AptShowStateChecker.Changed()) {
						LoadShowStateArray();
					}

					for (long i = 0; i < m_adwShowState.GetSize(); i++)
					{
						if (m_adwShowState[i] == 3)	// Exclude NoShow from the populated list
							continue;
						CString str;
						str.Format("Mark as %s", GetShowStateName((EAppointmentShowState)m_adwShowState.GetAt(i)));
						if (VarLong(varShow) != (long)m_adwShowState[i])
							pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_PENDING + m_adwShowState[i], str);
					}

				}

				pMenu->InsertMenu(-1, MF_BYPOSITION);

				//(e.lally 2009-09-28) PLID 18809 - Show both confirm/unconfirm menu options if the state is Left Message (LM)
				//Added a Left Message menu option
				// (c.haag 2011-06-17) - PLID 36477 - This is now handled in a utility function
				apmm.FillConfirmedOptions(nConfirmed);

				if (bMoveUp)
					pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_REMOVE_MOVE_UP, "Remove &Move-Up Status");
				else
					pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_MOVE_UP, "Mark as &Move-Up");

				pMenu->InsertMenu(-1, MF_BYPOSITION);
				pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_DELETE, GetStringOfResource(IDS_DELETE_APPOINTMENT));

				if (GetAppointmentLinkingAllowed()) {
					pMenu->InsertMenu(-1, MF_BYPOSITION);
					//(e.lally 2009-09-28) PLID 18809 - Switched K to the hotkey here
					pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_LINK, "Appointment Lin&king...");
				}

				// (j.jones 2007-11-21 14:45) - PLID 28147 - add the ability to create an allocation,
				// provided the have the license and potential permission to do so
				// (c.haag 2011-06-23) - PLID 44287 - Populating inventory options is done in a utility function now
				BOOL bAddedSeparator = apmm.FillInventoryOptions();

				// (c.haag 2011-06-24) - PLID 44317 - Populating billing options is done in a utility function now. Pass
				// in FALSE so that a separator is wedged between these and any previous items.
				bAddedSeparator = apmm.FillBillingOptions(FALSE);

				// (c.haag 2011-06-24) - PLID 44319 - Populating e-eligibility options is done in a utility function.
				bAddedSeparator = apmm.FillEEligibilityOptions(bAddedSeparator);

				// (c.haag 2011-06-24) - PLID 44319 - Populating case history options is done in a utility function.
				bAddedSeparator = apmm.FillCaseHistoryOptions(bAddedSeparator, nAptTypeCategory);

				pMenu->InsertMenu(-1, MF_BYPOSITION);
				pMenu->InsertMenu(-1, MF_BYPOSITION, ID_APPT_GOTO, GetStringOfResource(IDS_GOTO_APPOINTMENT));
			}

			// (d.lange 2010-11-08 09:32) - PLID 41192 - add menu items for every device plugin thats enabled and has the ability to send to devic
			CArray<DeviceLaunchUtils::DevicePlugin*, DeviceLaunchUtils::DevicePlugin*> aryLoadedPlugins;
			// (j.gruber 2013-04-03 14:48) - PLID 56012
			long nBlank = -1;
			DeviceLaunchUtils::GenerateDeviceMenu(aryLoadedPlugins, pMenu, nBlank, FALSE, -1);

			CPoint pt(point);
			GetCursorPos(&pt);
			// (j.jones 2010-09-24 14:05) - PLID 34518 - converted to return the selection here for copayment reasons
			long nSelection = pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, this);
			// (c.haag 2011-06-23) - PLID 36477 - Do NOT use CApptPopupMenuManager::HandlePopupMenuResult on nSelection 
			// because command messages like ID_APPT_CONFIRMED need to be routed through the parent (refer to the later call
			// to SendMessage(WM_COMMAND...).

			// (d.lange 2010-11-08 09:38) - PLID 41192 - return TRUE if the user selected a send to device menu item
			// (j.gruber 2013-04-03 14:49) - PLID 56012 - consolidate
			BOOL bLaunchDevice = DeviceLaunchUtils::LaunchDevice(aryLoadedPlugins, nSelection, nPatientID);
			if (!bLaunchDevice) {
				// (j.jones 2010-09-24 13:24) - PLID 34518 - check for dynamic copayment selections
				if ((nSelection >= MIN_ID_COPAYMENT_RESPS && nSelection < MIN_ID_COPAYMENT_RESPS + aryCopayInsuredPartyIDs.GetSize())
					|| (nSelection == ID_APPT_NEW_PRIMARY_COPAY && nPrimaryInsuredPartyID != -1)) {

					long nInsuredPartyID = -1;
					if (nSelection == ID_APPT_NEW_PRIMARY_COPAY && nPrimaryInsuredPartyID != -1) {
						nInsuredPartyID = nPrimaryInsuredPartyID;
					}
					else if (nSelection >= MIN_ID_COPAYMENT_RESPS && nSelection < MIN_ID_COPAYMENT_RESPS + aryCopayInsuredPartyIDs.GetSize()) {
						nInsuredPartyID = aryCopayInsuredPartyIDs.GetAt(nSelection - MIN_ID_COPAYMENT_RESPS);
					}

					if (nInsuredPartyID == -1) {
						//should be impossible
						ThrowNxException("Copay creation failed - no insured party selected!");
					}

					PromptForCopay(nInsuredPartyID);
				}
				else if (nSelection != 0) {
					//create our WM_COMMAND message and dispatch it now
					SendMessage(WM_COMMAND, MAKEWPARAM(nSelection, 0), 0);
				}
			}
			// (d.lange 2010-11-08 09:41) - PLID 41192 - Destory all loaded device plugins, so we don't leak memory
			// (j.gruber 2013-04-03 14:50) - PLID 56012 - consolidate
			DeviceLaunchUtils::DestroyLoadedDevicePlugins(aryLoadedPlugins);

			delete pMenu;
		}
	}NxCatchAll(__FUNCTION__);
}

void CAppointmentsDlg::UpdateViewBySingleAppt(long nApptID)
{
	try {

		//save this so we know if we are updating the currently selected row
		// (j.gruber 2015-01-08 15:13) - PLID 64545 - Change tp DL 2
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = m_pApptList->CurSel;		
		long nCurAptID = -1;
		if (pCurSel) {
			nCurAptID = VarLong(pCurSel->GetValue(alcResID));
		}

		CWaitCursor cu;
		// (j.dinatale 2011-10-12 09:18) - PLID 44597 - need to pull Last LM as a string, not a datetime
		// (a.walling 2010-10-13 16:28) - PLID 40977 - Parameterized
		// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading Appointments dialog
		// (j.gruber 2014-12-15 12:38) - PLID 64419 - Rescheduling Queue - Notes for Appts
		// (a.walling 2015-01-28 13:51) - PLID 64382 - Rescheduling Queue status column
		_RecordsetPtr rsAppt = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ResComplexQ.ID, PatientID, Color, Date, StartTime, "
			"Purpose, Item, Confirmed, Notes, CONVERT(nvarchar(50), LastLM, 101) AS LastLM, "
			"CASE WHEN Status=4 THEN CASE WHEN IsInReschedulingQueue = 1 THEN -3 ELSE (CASE WHEN ShowState=3 THEN -2 ELSE -1 END) END ELSE ShowState END AS Status, "
			"CreatedLogin, CreatedDate, ModifiedDate, "
			"ModifiedLogin, CancelledDate, CancelledBy, NoShowReason, NoShowReasonID, CancelledReason, CancelReasonID, HasExtraNotes "
			"FROM (SELECT ResBasicQ.*, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName, "
			"dbo.GetResourceString(ResBasicQ.ID) AS Item, "
			"CASE WHEN ResBasicQ.AptPurposeName <> '' THEN AptTypeT.Name + ' - ' + ResBasicQ.AptPurposeName ELSE AptTypeT.Name END AS Purpose, "
			"CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS Color, 0 AS SetID "
			"FROM AptTypeT RIGHT JOIN PersonT RIGHT JOIN (SELECT AppointmentsT.ID, AppointmentsT.PatientID, "
			"AppointmentsT.AptTypeID, dbo.GetPurposeString(AppointmentsT.ID) AS AptPurposeName, CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 1)) AS Date, "
			"convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS StartTime, AppointmentsT.Confirmed, "
			"AppointmentsT.Notes, CONVERT(bit, CASE WHEN EXISTS (SELECT ID FROM WaitingListT WHERE AppointmentID={INT}) THEN 1 ELSE 0 END) AS MoveUp, "
			"AppointmentsT.RecordID, AppointmentsT.Status, PersonT.Archived, "
			"AppointmentsT.ShowState, AppointmentsT.CreatedDate, AppointmentsT.CreatedLogin, AppointmentsT.ModifiedDate, AppointmentsT.LastLM, "
			"AppointmentsT.ModifiedLogin, AppointmentsT.CancelledDate, AppointmentsT.CancelledBy, AppointmentsT.NoShowReason, AppointmentsT.NoShowReasonID, AppointmentsT.CancelledReason, AppointmentsT.CancelReasonID, "
			"CONVERT(bit, CASE WHEN NotesQ.AppointmentID IS NOT NULL THEN 1 ELSE 0 END) AS HasExtraNotes, "
			"CONVERT(bit, CASE WHEN ReschedulingQueueT.AppointmentID IS NOT NULL THEN 1 ELSE 0 END) AS IsInReschedulingQueue "
			"FROM AppointmentsT LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			"LEFT JOIN ReschedulingQueueT ON AppointmentsT.ID = ReschedulingQueueT.AppointmentID "
			"LEFT JOIN(SELECT AppointmentID FROM Notes WHERE AppointmentID IS NOT NULL GROUP BY AppointmentID) NotesQ  ON AppointmentsT.ID = NotesQ.AppointmentID) "
			"ResBasicQ ON PersonT.ID = ResBasicQ.PatientID ON AptTypeT.ID = ResBasicQ.AptTypeID) AS ResComplexQ "
			"WHERE ResComplexQ.ID = {INT}", nApptID, nApptID);
		if(rsAppt->eof || AdoFldLong(rsAppt, "PatientID") != m_id) {
			//This appt has been deleted, find it and remove it.
			NXDATALIST2Lib::IRowSettingsPtr pRowLoop = m_pApptList->GetFirstRow();						
			bool bRowRemoved = false;
			int nCurrentRow = 0;//This is valid, because Bob tells me so.
			while (pRowLoop && !bRowRemoved)
			{						
				
				if(VarLong(pRowLoop->GetValue(alcResID)) == nApptID) {
					m_pApptList->RemoveRow(pRowLoop);
					bRowRemoved = true;					
				}								
				pRowLoop = pRowLoop->GetNextRow();
				
				nCurrentRow++;
			}
		}
		else {
			//Find this row, update it.			
			bool bNewRow = true;
			NXDATALIST2Lib::IRowSettingsPtr pModifiedRow;
			NXDATALIST2Lib::IRowSettingsPtr pRowLoop = m_pApptList->GetFirstRow();						
			int nCurrentRow = 0;//This is valid, because Bob tells me so.
			while (pRowLoop && bNewRow)
			{				
				if(VarLong(pRowLoop->GetValue(alcResID)) == nApptID) {
					pModifiedRow = pRowLoop;
					bNewRow = false;
				}
				nCurrentRow++;
				pRowLoop = pRowLoop->GetNextRow();
			}

			if (bNewRow) {
				pModifiedRow = m_pApptList->GetNewRow();
			}

			FieldsPtr fAppt = rsAppt->Fields;

			pModifiedRow->PutValue(alcResID, fAppt->GetItem("ID")->Value);
			pModifiedRow->PutValue(alcID, fAppt->GetItem("PatientID")->Value);
			pModifiedRow->PutValue(alcColor, fAppt->GetItem("Color")->Value);
			pModifiedRow->PutValue(alcDate, fAppt->GetItem("Date")->Value);
			pModifiedRow->PutValue(alcDay, (LPCTSTR)FormatDateTimeForInterface(VarDateTime(fAppt->GetItem("Date")->Value), "%a"));
			pModifiedRow->PutValue(alcStartTime, fAppt->GetItem("StartTime")->Value);
			pModifiedRow->PutValue(alcPurpose, fAppt->GetItem("Purpose")->Value);
			pModifiedRow->PutValue(alcItem, fAppt->GetItem("Item")->Value);
			pModifiedRow->PutValue(alcConfirmed, fAppt->GetItem("Confirmed")->Value);
			pModifiedRow->PutValue(alcNotes, fAppt->GetItem("Notes")->Value);
			pModifiedRow->PutValue(alcStatus, fAppt->GetItem("Status")->Value);
			pModifiedRow->PutValue(alcLastLM, fAppt->GetItem("LastLM")->Value);
			pModifiedRow->PutValue(alcLoginName, fAppt->GetItem("CreatedLogin")->Value);
			pModifiedRow->PutValue(alcInputDate, fAppt->GetItem("CreatedDate")->Value);
			pModifiedRow->PutValue(alcModifiedDate, fAppt->GetItem("ModifiedDate")->Value);
			pModifiedRow->PutValue(alcModifiedBy, fAppt->GetItem("ModifiedLogin")->Value);
			pModifiedRow->PutValue(alcCancelledDate, fAppt->GetItem("CancelledDate")->Value);
			pModifiedRow->PutValue(alcCancelledBy, fAppt->GetItem("CancelledBy")->Value);
			pModifiedRow->PutValue(alcCancelledReason, fAppt->GetItem("CancelledReason")->Value);
			pModifiedRow->PutValue(alcNoShowReason, fAppt->GetItem("NoShowReason")->Value);

			// (j.gruber 2014-12-15 12:38) - PLID 64419 - Rescheduling Queue - Notes for Appts
			pModifiedRow->PutValue(alcHasExtraNotes, fAppt->GetItem("HasExtraNotes")->Value);
			if (AdoFldBool(fAppt, "HasExtraNotes"))
			{
				pModifiedRow->PutValue(alcExtraNotesIcon, (long)m_hExtraNotesIcon);
			}
			else {
				pModifiedRow->PutValue(alcExtraNotesIcon, (LPCTSTR)"BITMAP:FILE");
			}

#pragma TODO("A recordset is being created for each cancelled appointment with a reason ID")
#pragma TODO("A recordset is being created for each noshow appointment with a reason ID")

			if (fAppt->GetItem("CancelReasonID")->Value.vt != VT_NULL) {
				// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading Appointments dialog
				_RecordsetPtr rsCancelID = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Description "
														"FROM AptCancelReasonT "
														"WHERE ID = {INT}", VarLong(fAppt->GetItem("CancelReasonID")->Value));
				if (!rsCancelID->eof)
					pModifiedRow->PutValue(alcCancelledReason, rsCancelID->Fields->Item["Description"]->Value);
			}

			if (fAppt->GetItem("NoShowReasonID")->Value.vt != VT_NULL) {
				// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading Appointments dialog
				_RecordsetPtr rsNoShowReasonID = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Description "
														"FROM AptNoShowReasonT "
														"WHERE ID = {INT}", VarLong(fAppt->GetItem("NoShowReasonID")->Value));
				if (!rsNoShowReasonID->eof)
					pModifiedRow->PutValue(alcNoShowReason, rsNoShowReasonID->Fields->Item["Description"]->Value);
			}
			
			if (bNewRow) {
				m_pApptList->AddRowSorted(pModifiedRow, NULL);
			}

			//update the "Go To Appointment" button if we are updating the currently selected row
			if(nCurAptID == nApptID) {
				SelChangedApptList(pModifiedRow, pModifiedRow);				
			}
		}

		// (j.gruber 2010-01-11 14:55) - PLID 22964 - update the counts
		LoadNoShowCancelBoxes();
	}NxCatchAll("Error in CAppointmentsDlg::UpdateViewBySingleAppt()");
}

void CAppointmentsDlg::OnIncludeCancelledDateColumns() 
{
	// Reflect the state of the checkbox
	ReflectIncludeCancelledDateColumns();

	// Save the setting to data
	BOOL bInclude = (IsDlgButtonChecked(IDC_INCLUDE_CANCELED_DATE_COLUMNS) != FALSE);
	SetRemotePropertyInt("ApptTabIncludeCancelledDate", bInclude ? 1 : 0, 0, GetCurrentUserName());

	if(bInclude){
		m_pApptList->Requery();
	}	

	// now save the column sizes
	SaveColumnSizes();
}

void CAppointmentsDlg::ReflectIncludeCancelledDateColumns()
{
	if (IsDlgButtonChecked(IDC_INCLUDE_CANCELED_DATE_COLUMNS)) {
		// Make sure the canceled date and canceled by fields have the csVisible flag
		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pApptList->GetColumn(alcCancelledDate);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthData);
		pCol->PutStoredWidth(40);
		pCol = m_pApptList->GetColumn(alcCancelledBy);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthData);
		pCol->PutStoredWidth(40);
		pCol = m_pApptList->GetColumn(alcCancelledReason);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthData);
		pCol->PutStoredWidth(40);
	} else {
		// Make sure the modified date and modified by fields don't have the csVisible flag
		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pApptList->GetColumn(alcCancelledDate);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth);
		pCol->PutStoredWidth(0);
		pCol = m_pApptList->GetColumn(alcCancelledBy);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth);
		pCol->PutStoredWidth(0);
		pCol = m_pApptList->GetColumn(alcCancelledReason);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth);
		pCol->PutStoredWidth(0);
	}
}

void CAppointmentsDlg::LoadIncludeCancelledDateColumns()
{
	// Load the data and set the checkbox on screen appropriately
	if (GetRemotePropertyInt("ApptTabIncludeCancelledDate", 0, 0, GetCurrentUserName(), false) != 0) {
		CheckDlgButton(IDC_INCLUDE_CANCELED_DATE_COLUMNS, 1);
	} else {
		CheckDlgButton(IDC_INCLUDE_CANCELED_DATE_COLUMNS, 0);
	}

	// Reflect the state of the checkbox
	ReflectIncludeCancelledDateColumns();
}

BOOL CAppointmentsDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (LOWORD(wParam))
	{
	case ID_APPT_IN: OnMarkIn(); break;
	case ID_APPT_OUT: OnMarkOut(); break;
	case ID_APPT_PENDING: OnMarkPending(); break;
	case ID_APPT_RECEIVED: OnMarkReceived(); break;
	default:
		if (LOWORD(wParam) > ID_APPT_RECEIVED) {
			long nID = LOWORD(wParam) - ID_APPT_PENDING;
			for (long i=0; i < m_adwShowState.GetSize(); i++)
			{
				if ((long)m_adwShowState[i] == nID)
				{
					OnMarkUserDefinedShowState((EAppointmentShowState)m_adwShowState[i]);
					break;
				}
			}
			break;
		}		
	}
	return CNxDialog::OnCommand(wParam, lParam);
}

CString CAppointmentsDlg::GetShowStateName(EAppointmentShowState type)
{
	// (c.haag 2003-10-16 09:22) - Cancelled is not a show state type; it's
	// an appointment status. However, users do not distinguish statuses
	// from states, and we should not hold back on having Cancelled as a visible
	// option just for that reason.
	// (a.walling 2014-12-22 16:49) - PLID 64382 - handle astToBeRescheduled
	if (type == astToBeRescheduled)
		return "To Be Rescheduled";
	else if (type == astCancelledNoShow)
		return "Cancelled (No Show)";
	else if (type == astCancelled)
		return "Cancelled";
	else {
		//TES 2004-01-20: The index in the array is NOT the same as the show state ID!!!.
		bool bIDFound = false;
		// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
		int i = 0;
		for(i = 0; i < m_adwShowState.GetSize() && !bIDFound; i++) {
			if(m_adwShowState.GetAt(i) == (DWORD)type) bIDFound = true;
		}
		if(bIDFound) {
			//i will have gotten incremented one last time before the loop exited.
			return m_astrShowState[i-1];
		}
		else {
			ASSERT(FALSE);
			return "";
		}
	}
}

void CAppointmentsDlg::OnRememberColumnSettings() 
{
	try {

		//save the setting
		long nRemember = 0;	//default off
		if (IsDlgButtonChecked(IDC_APPTS_REMEMBER_COL_SETTINGS))
			nRemember = 1;
		else
			nRemember = 0;
		SetRemotePropertyInt("RememberAppointmentsColumns", nRemember, 0, GetCurrentUserName());

		//size the datalist appropriately
		if (!IsDlgButtonChecked(IDC_APPTS_REMEMBER_COL_SETTINGS)) {
			ResetColumnSizes();
		}
		else {
			// (j.jones 2014-06-09 09:22) - PLID 62336 - this now saves your current sizes
			// and then sets the columns to load as fixed width
			SaveColumnSizes();
			SetColumnSizes();
		}

	}NxCatchAll(__FUNCTION__);
}

void CAppointmentsDlg::ResetColumnSizes()
{
	//parse the columns out and set them
	m_pApptList->SetRedraw(FALSE);
	for (int i = 0; i < alcNoShowReason; i++) {

		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pApptList->GetColumn(i);
		if (pCol) {

			switch (i) {

				//reset to their original styles and widths
			case alcResID:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(0);
				break;
			case alcID:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(0);
				break;
			case alcColor:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(0);
				break;
			case alcHasExtraNotes: // (j.gruber 2014-12-15 16:00) - PLID 64419 - Rescheduling Queue - Add notes icon
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(0);
			break;
			case alcExtraNotesIcon: // (j.gruber 2014-12-15 16:00) - PLID 64419 - Rescheduling Queue - Add notes icon
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(42);
			break;
			case alcDate:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(70);
				break;
			case alcDay:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(35);
				break;
			case alcStartTime:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(55);
				break;
			case alcPurpose:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthPercent;
				pCol->PutStoredWidth(10);
				break;
			case alcItem:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthPercent;
				pCol->PutStoredWidth(12);
				break;
			case alcConfirmed:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(39);
				break;
			case alcNotes:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthAuto;
				break;
				// (b.savon 2012-03-27 16:54) - PLID 49074 - Added alcRecall
			case alcRecall:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(42);
			case alcStatus:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(58);
				break;
			case alcLastLM:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(65);
				break;
			case alcLoginName:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(70);
				break;
			case alcInputDate:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(70);
				break;
			case alcLocationID:
				pCol->ColumnStyle = NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth;
				pCol->PutStoredWidth(0);
				break;
			}
		}

		ReflectIncludeModifiedDateColumns();
		ReflectIncludeCancelledDateColumns();
		ReflectIncludeNoShowReasonColumn();

		// (j.jones 2014-06-09 09:57) - PLID 62336 - this is only called when we are switching
		// from remembered to not-remembered, so there is no need to save changes
		//SaveColumnSizes();
	}
	m_pApptList->SetRedraw(TRUE);
}

void CAppointmentsDlg::SetColumnSizes()
{
	//TES 2/9/04: Plagiarized from ToDoAlarm
	//DRT - 6/6/03 - This function takes the saved column sizes out of ConfigRT
	//		IF the box is checked.

	//don't want to remember
	if(!IsDlgButtonChecked(IDC_APPTS_REMEMBER_COL_SETTINGS)) {
		return;
	}

	m_pApptList->SetRedraw(FALSE);
	//Make sure all the styles are fixed-width type styles.
	for (short i = 0; i < m_pApptList->ColumnCount; i++)
	{
		long nStyle = m_pApptList->GetColumn(i)->ColumnStyle;
		nStyle &= ~(NXDATALIST2Lib::csWidthPercent | NXDATALIST2Lib::csWidthAuto | NXDATALIST2Lib::csWidthData);
		m_pApptList->GetColumn(i)->ColumnStyle = nStyle;
	}

	CString strCols = GetRemotePropertyText("DefaultAppointmentsColumnSizes", "", 0, GetCurrentUserName(), false);
	if(strCols.IsEmpty())
		ResetColumnSizes();

	if(!strCols.IsEmpty()) {
		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		int nWidth = 0, i = 0;

		//parse the columns out and set them
		int nComma = strCols.Find(",");
		while(nComma > 0) {
			nWidth = atoi(strCols.Left(nComma));
			strCols = strCols.Right(strCols.GetLength() - (nComma+1));

			pCol = m_pApptList->GetColumn(i);
			if(pCol)
				pCol->PutStoredWidth(nWidth);

			i++;
			nComma = strCols.Find(",");
		}
	}

	m_pApptList->SetRedraw(TRUE);
}


void CAppointmentsDlg::SaveColumnSizes()
{
	// (j.jones 2014-06-09 10:05) - PLID 62336 - don't save if remember column widths is not checked
	if (!IsDlgButtonChecked(IDC_APPTS_REMEMBER_COL_SETTINGS)) {
		return;
	}
	//save width of each column
	NXDATALIST2Lib::IColumnSettingsPtr pCol;
	CString str, strList;

	for (int i = 0; i < m_pApptList->GetColumnCount(); i++) {
		pCol = m_pApptList->GetColumn(i);
		if(pCol)
			str.Format("%li,", pCol->GetStoredWidth());

		strList += str;
	}

	//write it to ConfigRT
	SetRemotePropertyText("DefaultAppointmentsColumnSizes", strList, 0, GetCurrentUserName());
}

void CAppointmentsDlg::OnShowNoShowReason() 
{
	// Reflect the state of the checkbox
	ReflectIncludeNoShowReasonColumn();

	// Save the setting to data
	BOOL bInclude = (IsDlgButtonChecked(IDC_SHOW_NO_SHOW_REASON) != FALSE);
	SetRemotePropertyInt("ApptTabIncludeNoShowReasons", bInclude ? 1 : 0, 0, GetCurrentUserName());

	// If we're adding them back then we need to requery because they may not have been visible 
	// the last time we requeried
	// (b.cardillo 2003-07-14 13:48) I'm beginning to think the datalist should still load data 
	// even if the column doesn't have the csVisible flag.  Then the flag could finally fulfill 
	// the purpose people seem to be trying to use it for.  (i.e. we wouldn't have to requery here...)
	if (bInclude) {
		m_pApptList->Requery();
	}

	// now save the column sizes
	SaveColumnSizes();
	
}

void CAppointmentsDlg::ReflectIncludeNoShowReasonColumn()
{
	if (IsDlgButtonChecked(IDC_SHOW_NO_SHOW_REASON)) {
		// Make sure the no show reason field has the csVisible flag
		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pApptList->GetColumn(alcNoShowReason);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csWidthData);
		pCol->PutStoredWidth(100);
	} else {
		// Make sure the no show reason field doesn't have the csVisible flag
		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pApptList->GetColumn(alcNoShowReason);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth);
		pCol->PutStoredWidth(0);
	}
}

void CAppointmentsDlg::LoadIncludeNoShowReasonColumn()
{
	// Load the data and set the checkbox on screen appropriately
	if (GetRemotePropertyInt("ApptTabIncludeNoShowReasons", 0, 0, GetCurrentUserName(), false) != 0) {
		CheckDlgButton(IDC_SHOW_NO_SHOW_REASON, 1);
	} else {
		CheckDlgButton(IDC_SHOW_NO_SHOW_REASON, 0);
	}

	// Reflect the state of the checkbox
	ReflectIncludeNoShowReasonColumn();


}

// (j.jones 2007-11-21 14:20) - PLID 28147 - added ability to create a new inventory allocation
void CAppointmentsDlg::OnNewInvAllocation()
{
	try {
		// (j.gruber 2015-01-08 15:47) - PLID 64545 - Change for DL 2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}
		

		long nApptID = VarLong(pRow->GetValue(alcResID));
		long nLocationID = VarLong(pRow->GetValue(alcLocationID));

		// (c.haag 2011-06-23) - PLID 44287 - The act of creating the allocation is now done in a utility function
		AppointmentCreateInvAllocation(this, nApptID, FALSE, m_id, nLocationID);

	} NxCatchAll("Error in CAppointmentsDlg::OnNewInvAllocation");
}

// (j.gruber 2008-09-10 15:39) - PLID 30282 - added ability to edit an allocation from the right click menu
void CAppointmentsDlg::OnEditInvAllocation()
{
	try {
		// (j.gruber 2015-01-08 15:47) - PLID 64545 - Change for DL 2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}
	

		long nApptID = VarLong(pRow->GetValue(alcResID));
		long nLocationID = VarLong(pRow->GetValue(alcLocationID));

		// (c.haag 2011-06-23) - PLID 44287 - The act of editing the allocation is now done in a utility function
		AppointmentEditInvAllocation(this, nApptID, FALSE, m_id, nLocationID);

	} NxCatchAll("Error in CAppointmentsDlg::OnEditInvAllocation");
}

// (j.jones 2008-03-18 14:26) - PLID 29309 - added ability to create a new inventory order
void CAppointmentsDlg::OnNewInvOrder()
{
	try {
		// (j.gruber 2015-01-08 15:47) - PLID 64545 - Change for DL 2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;	
		if (!pRow) {
			return;
		}
		

		long nApptID = VarLong(pRow->GetValue(alcResID));
		long nLocationID = VarLong(pRow->GetValue(alcLocationID));

		// (c.haag 2011-06-23) - PLID 44287 - This is now done in a utility function
		AppointmentCreateInvOrder(this, nApptID, FALSE, m_id, nLocationID);

	} NxCatchAll("Error in CAppointmentsDlg::OnNewInvOrder");
}

// (j.jones 2008-06-23 16:16) - PLID 30455 - added ability to create a bill
void CAppointmentsDlg::OnNewBill()
{
	try {
		// (j.gruber 2015-01-08 15:47) - PLID 64545 - Change for DL 2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}


		long nApptID = VarLong(pRow->GetValue(alcResID));

		// (c.haag 2011-06-23) - PLID 44287 - This is now done in a utility function
		AppointmentCreateNewBill(this, nApptID, FALSE, m_id);

	} NxCatchAll("Error in CAppointmentsDlg::OnNewBill");
}

// (j.jones 2009-08-28 12:55) - PLID 35381 - added case history options
void CAppointmentsDlg::OnApptCreateCaseHistory()
{

	try {
		// (j.gruber 2015-01-08 15:47) - PLID 64545 - Change for DL 2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if(!pRow) {
			return;
		}

		long nApptID = VarLong(pRow->GetValue(alcResID));

		// (c.haag 2011-06-23) - PLID 44319 - The case history is now created from a utility function
		AppointmentCreateCaseHistory(nApptID, m_id);

	} NxCatchAll("Error in CAppointmentsDlg::OnApptCreateCaseHistory");
}

void CAppointmentsDlg::OnApptEditCaseHistory()
{
	try {
		// (j.gruber 2015-01-08 15:47) - PLID 64545 - Change for DL 2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if(!pRow) {
			return;
		}

		long nApptID = VarLong(pRow->GetValue(alcResID));

		// (c.haag 2011-06-23) - PLID 44319 - The case history is now edited from a utility function
		AppointmentEditCaseHistory(nApptID, m_id);

	} NxCatchAll("Error in CAppointmentsDlg::OnApptEditCaseHistory");
}

// (j.jones 2010-09-27 11:40) - PLID 38447 - added ability to view eligibility responses
void CAppointmentsDlg::OnViewEligibilityResponses()
{
	try{
		// (j.gruber 2015-01-08 15:47) - PLID 64545 - Change for DL 2
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if(!pRow) {
			return;
		}

		long nApptID = VarLong(pRow->GetValue(alcResID));
		long nPatientID = GetActivePatientID();

		ShowAllEligibilityRequestsForInsuredParty_ByPatientOrAppt(this, nPatientID, nApptID);

	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2010-11-08 09:34) - PLID 41192 - We return TRUE if we call LaunchDevice for the loaded device plugin
// (d.lange 2011-01-13 10:02) - PLID 41192 - When a plugin is not configured properly, display the message and return TRUE
// (j.gruber 2013-04-03 14:51) - PLID 56012 - not needed anymore

// (j.dinatale 2011-06-24 17:02) - PLID 30025 - Show Cancelled Appts
void CAppointmentsDlg::OnShowCancelled()
{
	try{
		BOOL bShowCancelled = m_btnShowCancelled.GetCheck();
		SetRemotePropertyInt("ApptDlgShowCancelled", bShowCancelled, 0, GetCurrentUserName());

		RefreshList();
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2011-06-24 17:02) - PLID 30025 - Show No Show Appts
void CAppointmentsDlg::OnShowNoShows()
{
	try{
		BOOL bShowNoShows = m_btnShowNoShows.GetCheck();
		SetRemotePropertyInt("ApptDlgShowNoShows", bShowNoShows, 0, GetCurrentUserName());

		RefreshList();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-02-29 10:34) - PLID 48487
void CAppointmentsDlg::OnBtnRecallClicked()
{
	try {
		GetMainFrame()->ShowRecallsNeedingAttention(true);
	} NxCatchAll(__FUNCTION__);
}


// (j.gruber 2014-12-15 12:38) - PLID 64419 - Rescheduling Queue - Changed to include notes icon too
// (b.savon 2012-03-27 16:11) - PLID 49074 - Add Recall icon for recall appointments
void CAppointmentsDlg::ShowIcons()
{
	try{
		// (j.armen 2012-03-28 14:18) - PLID 48480 - If they don't have the license, don't worry about doing this
		BOOL bHasRecalls = g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent);
		
		
		//Run through our list, setting the recall icon if it's linked
		// (j.gruber 2014-12-15 12:38) - PLID 64419  - changed the way this loops
		// (j.gruber 2015-01-08 15:47) - PLID 64545 - Change for DL 2 - change how it loops
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->GetFirstRow();
		
		while (pRow) {
			// Get the next row in the datalist while at the same time getting this a smart-pointer to the current row			
			if (bHasRecalls)
			{
				if (AsLong(pRow->GetValue(alcRecall)) == 1){
					pRow->PutValue(alcRecall, (long)m_hiRecall);
				}
			}

			// (j.gruber 2014-12-15 12:38) - PLID 64419  - check our HasNotes 
			BOOL bHasExtraNotes = VarBool(pRow->GetValue(alcHasExtraNotes));
			if (bHasExtraNotes)
			{
				pRow->PutValue(alcExtraNotesIcon, (long)m_hExtraNotesIcon);
			}
			else {
				pRow->PutValue(alcExtraNotesIcon, (LPCTSTR)"BITMAP:FILE");
			}	

			pRow = pRow->GetNextRow();
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-12-01 10:47) - PLID 53162 - Connectathon - HL7 Dialog for Patient Encounter
void CAppointmentsDlg::OnBnClickedBtnPatientEncounter()
{
	try{

		CPatientEncounterDlg dlg;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}




// (a.walling 2014-12-22 16:01) - PLID 64418 - Open Rescheduling Queue with this appt selected and filters reset
void CAppointmentsDlg::OnRescheduleAppointment()
{
	try {
		// (j.gruber 2015-01-08 15:47) - PLID 64545 - Change for DL 2 
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (!pRow) {
			return;
		}
		long lApptID = VarLong(pRow->GetValue(alcResID));

		GetMainFrame()->ShowReschedulingQueue(lApptID);
	} NxCatchAll(__FUNCTION__);
}


// (j.gruber 2015-01-08 14:35) - PLID 64545 - Reworked to change to DL2
void CAppointmentsDlg::DblClickCellApptList(LPDISPATCH lpRow, short nColIndex)
{
	SwitchToAppointment();
}

// (j.gruber 2015-01-08 14:35) - PLID 64545 - Reworked to change to DL2
void CAppointmentsDlg::RButtonDownApptList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	// (j.gruber 2008-09-10 15:44) - PLID 30282 - I added this with my pl item because OnContextMenu calls recordsets and 
	//I was adding one and I thought it should be checked in case of error and this seemed a better 
	//place to handle it then in OnContextMenu
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			m_pApptList->CurSel = pRow;

			SelChangedApptList(pRow, pRow);

			OnContextMenu(GetDlgItem(IDC_APPOINT_LIST), CPoint(x, y));
		}	
		else {
			m_pApptList->CurSel = NULL;
		}
		
	}NxCatchAll("Error in CAppointmentsDlg::RButtonDownApptList");
}

// (j.gruber 2015-01-08 14:35) - PLID 64545 - Reworked to change to DL2
void CAppointmentsDlg::EditingFinishedApptList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		if (varNewValue.vt == VT_NULL || varNewValue.vt == VT_EMPTY)
			return;

		if (!bCommit) {
			// If the user didn't commit the change, then do nothing
			return;
		}

		// (j.gruber 2011-07-15 15:22) - PLID 44065 - get the current time, note this may be slightly off
		//from the server time, but its worth not doing a refresh	
		CString strNow = FormatDateTimeForInterface(COleDateTime::GetCurrentTime(), DTF_STRIP_SECONDS);
		CString strUser = GetCurrentUserName();

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow){
			return;
		}
		long lApptID = VarLong(pRow->GetValue(alcResID));

		switch (nCol) {
		case alcConfirmed:  //confirmed checkbox

			try {
				CString strOld, strNew;

				switch (VarLong(varOldValue))
				{
				case 0:	strOld = "Unconfirmed";	break;
				case 1: strOld = "Confirmed"; break;
				case 2: strOld = "Left message"; break;
				default: strOld = "Unknown"; break;
				}
				switch (VarLong(varNewValue))
				{
				case 0:	strNew = "Unconfirmed";	break;
				case 1: strNew = "Confirmed"; break;
				case 2: strNew = "Left message"; break;
				default: strNew = "Unknown"; break;
				}

				long nAuditID = BeginNewAuditEvent();
				AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiApptConfirm, lApptID, strOld, strNew, aepMedium);

				// (j.armen 2011-07-01 15:36) - PLID 44205 - Added ConfirmedBy field
				// (j.gruber 2011-07-15 13:51) - PLID 44065 - update modified date
				ExecuteParamSql("UPDATE AppointmentsT SET Confirmed = {INT}, ConfirmedBy = {STRING}, ModifiedDate = GetDate(), ModifiedLogin = {STRING} WHERE ID = {INT}", VarLong(varNewValue), VarLong(varNewValue) == acsConfirmed ? GetCurrentUserName() : "", strUser, lApptID);


				if (VarLong(varNewValue) == 2)
				{
					COleDateTime dtInvalid;
					dtInvalid.SetStatus(COleDateTime::invalid);

					// (j.dinatale 2011-10-11 17:18) - PLID 44597 - using getvalue on the Last LM column yields a VT_BSTR, need to parse that into a datetime
					COleDateTime dt;
					dt.ParseDateTime(VarString(pRow->GetValue(alcLastLM), ""));

					if (dt.GetStatus() == COleDateTime::invalid)
					{
						ExecuteSql("UPDATE AppointmentsT SET LastLM = GetDate() WHERE ID = %d", lApptID);
						long nAuditID = BeginNewAuditEvent();
						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiApptLeftMessageDate, lApptID, "", FormatDateTimeForInterface(dtNow, dtoDateTime), aepMedium);
					}
				}

				// (j.gruber 2011-07-15 15:26) - PLID 44065 - refresh modified date
				pRow->PutValue(alcModifiedDate, _bstr_t(strNow));
				pRow->PutValue(alcModifiedBy, _bstr_t(strUser));

				// send a network message
				CClient::RefreshAppointmentTable(lApptID);

				// (r.gonet 2015-06-15 12:07) - PLID 66202 - Send an update appointment message.
				SendUpdateAppointmentHL7Message(lApptID);


			}NxCatchAll("Error in EditingFinishedApptList::Confirmed");
			break;

		case alcNotes: // Notes
			try {
				CString strOld;
				CString strNew;

				// (j.gruber 2011-07-15 13:52) - PLID 44065 - add modified date
				ExecuteSql("Update AppointmentsT SET Notes = '%s', ModifiedDate = GetDate(), ModifiedLogin = '%s' WHERE ID = %li", _Q(VarString(varNewValue)), _Q(strUser), lApptID);

				// (b.eyers 2013-8-7) - PLID 46481 - Audit appointment notes
				strNew = VarString(varNewValue);
				strOld = VarString(varOldValue);
				if (strOld != strNew){
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(m_id, GetExistingPatientName(m_id), nAuditID, aeiApptNotes, lApptID, strOld, strNew, aepMedium);
				}

				// (j.gruber 2011-07-15 15:26) - PLID 44065 - refresh modified date
				pRow->PutValue(alcModifiedDate, _bstr_t(strNow));
				pRow->PutValue(alcModifiedBy, _bstr_t(strUser));

				// send a network message
				CClient::RefreshAppointmentTable(lApptID);

				// (r.gonet 2016-01-18 13:46) - PLID 67930 - Send an update appointment message.
				if (strOld != strNew) {
					SendUpdateAppointmentHL7Message(lApptID);
				}

			}NxCatchAll("Error in EditingFinishedApptList::Notes");
			break;

		case alcLastLM:
			try {
				CString strOld;
				CString strNew;
				COleDateTime dtNew;
				COleDateTime dtOld;

				// (j.dinatale 2011-10-11 17:48) - PLID 44597 - the Last LM column is now a string, parse it to a datetime, and then we can use it to compare
				//		and see if the two dates are the same.
				dtOld.ParseDateTime(VarString(varOldValue, ""));
				dtNew.ParseDateTime(VarString(varNewValue, ""));

				if (dtNew.GetStatus() == COleDateTime::valid && dtNew.m_dt > 0)
				{
					// (r.gonet 02/06/2012) - PLID 47424 - If they didn't enter a date, then we set the date component to the today's date.
					COleDateTime dtMin(1900, 1, 1, 0, 0, 0);
					if (dtNew < dtMin) {
						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						dtNew.SetDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), dtNew.GetHour(), dtNew.GetMinute(), dtNew.GetSecond());
						// (r.gonet 02/06/2012) - PLID 47424 - We have auto added the current date to the user's entered time. We need to update the row
						//  to reflect what will save.
						pRow->PutValue(alcLastLM, _bstr_t(FormatDateTimeForInterface(dtNew, NULL, dtoDate)));
					}

					// (j.gruber 2011-07-15 13:52) - PLID 44065 - modified date
					ExecuteSql("UPDATE AppointmentsT SET LastLM = '%s', ModifiedDate = GetDate(), ModifiedLogin = '%s' WHERE ID = %d", FormatDateTimeForSql(dtNew, dtoDateTime), _Q(strUser), lApptID);
					strNew = FormatDateTimeForInterface(dtNew, dtoDateTime);
				}
				else
				{	// (j.gruber 2011-07-15 13:53) - PLID 44065 - modified date
					ExecuteSql("UPDATE AppointmentsT SET LastLM = NULL, ModifiedDate = GetDate(), ModifiedLogin = '%s' WHERE ID = %d", _Q(strUser), lApptID);
				}

				// (b.eyers 2015-03-13) - PLID 46696 - If there was no old date, it was auditing 'Bad DateTime'
				if (dtOld.GetStatus() == COleDateTime::invalid)
				{
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiApptLeftMessageDate, lApptID, "", strNew, aepMedium);
				}
				else
				{
					long nAuditID = BeginNewAuditEvent();
					AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiApptLeftMessageDate, lApptID, FormatDateTimeForInterface(dtOld, dtoDateTime), strNew, aepMedium);
				}

				// (j.dinatale 2011-10-11 16:43) - PLID 44597 - use the parsed datetimes and our new CompareDates function to compare
				long nOldStatus = VarLong(pRow->GetValue(alcConfirmed), -1);
				if (nOldStatus != 2 && CompareDatesNoTime(dtOld, dtNew) != 0)
				{
					if (IDYES == MsgBox(MB_YESNO, "Would you like to change the appointment confirmed status to 'Left Message'?"))
					{
						long nAuditID = BeginNewAuditEvent();

						switch (nOldStatus)
						{
						case 0:	strOld = "Unconfirmed";	break;
						case 1: strOld = "Confirmed"; break;
						case 2: strOld = "Left message"; break;
						default: strOld = "Unknown"; break;
						}
						strNew = "Left Message";

						// (j.armen 2011-07-01 15:36) - PLID 44205 - Added confirmed by field
						ExecuteParamSql("UPDATE AppointmentsT SET Confirmed = {INT}, ConfirmedBy = {STRING} WHERE ID = {INT}", 2, GetCurrentUserName(), lApptID);

						AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiApptConfirm, lApptID, strOld, strNew, aepMedium);

						// (r.gonet 2015-06-15 12:07) - PLID 66202 - Send an update appointment message.
						SendUpdateAppointmentHL7Message(lApptID);
					}
				}

				// (j.gruber 2011-07-15 15:26) - PLID 44065 - refresh modified date
				pRow->PutValue(alcModifiedDate, _bstr_t(strNow));
				pRow->PutValue(alcModifiedBy, _bstr_t(strUser));

				// send a network message
				CClient::RefreshAppointmentTable(lApptID);

			}NxCatchAll("Error in EditingFinishedApptList::LastLM");
			break;

		case alcStatus:
			try {
				// Get the old and the new items
				EAppointmentShowState nOld = (EAppointmentShowState)VarLong(varOldValue);
				EAppointmentShowState nNew = (EAppointmentShowState)VarLong(varNewValue);
				// First see if the user changed anything
				if (nNew != nOld) {

					// The user changed something, so write the new value to data
					if (nNew == astToBeRescheduled) {
						// (a.walling 2014-12-22 16:44) - PLID 64382 - handle astToBeRescheduled
						// this should never be able to be a new status!
						ASSERT(FALSE);
					}
					else if (nNew == astCancelledNoShow) {
						// Status=4 THEN CASE WHEN ShowState=3 
						if (nOld == astCancelled || AppointmentCancel(lApptID)) {
							// Good proceed with marking it no-show
							OnMarkNoShow();
						}
						else {
							// The user decided not to cancel the appointment so we have to refresh to get back to the state it was in before
							UpdateViewBySingleAppt(lApptID);
						}
					}
					else if (nNew == astCancelled) {
						// Status=4 THEN CASE WHEN ShowState<>3
						// Marking it cancelled, so first write that part to data
						if (nOld == astCancelledNoShow || AppointmentCancel(lApptID)) {
							// Now if the ShowState wasn't No Show then just mark it cancelled and leave the ShowState as 
							// is, but if the the previous ShowState WAS No Show, then change it to Pending (because if 
							// the user wanted it to be No Show then he would have set the status string to "Cancelled 
							// (No Show)")
							if (nOld == astCancelledNoShow || nOld == astNoShow) {
								// It was no-show before and now it's not, so make it "not no show".  This is the "Unmark No Show" task.
								OnMarkShow();
							}
						}
						else {
							// The user decided not to cancel the appointment so we have to refresh to get back to the state it was in before
							UpdateViewBySingleAppt(lApptID);
						}
					}
					else if (nNew == astPending) {
						// ShowState=0 
						OnMarkPending();
					}
					else if (nNew == astIn) {
						// ShowState=1 
						OnMarkIn();
					}
					else if (nNew == astOut) {
						// ShowState=2 
						OnMarkOut();
					}
					else if (nNew == astNoShow) {
						// ShowState=3 
						OnMarkNoShow();
					}
					else if (nNew == astReceived) {
						// ShowState=4 
						OnMarkReceived();
					}
					else {
						// ShowState>4
						OnMarkUserDefinedShowState(nNew);
					}

					if (nNew != astCancelled && nNew != astNoShow && nNew != astCancelledNoShow) {

						LogShowStateTime(lApptID, nNew);
					}

				}
			} NxCatchAll("CAppointmentsDlg::EditingFinishedApptList::alcStatus");
			break;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2015-01-08 14:35) - PLID 64545 - Reworked to change to DL2
void CAppointmentsDlg::EditingFinishingApptList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		if (!pvarNewValue || pvarNewValue->vt == VT_NULL || pvarNewValue->vt == VT_EMPTY)
			return;
		if (!(*pbCommit))
			return;

		switch (nCol) {
		case alcStatus:
			try {
				// Get the old and the new strings
				long nOld = VarLong(varOldValue);
				long nNew = VarLong(*pvarNewValue);
				// First see if the user changed anything
				if (nNew != nOld) {
					
					// (a.walling 2015-01-28 13:51) - PLID 64382 - To Be Rescheduled should never be a new status!
					if (nNew == astToBeRescheduled) {						
						*pbCommit = FALSE;
						return;
					}
					// (a.walling 2015-01-28 13:51) - PLID 64382 - To Be Rescheduled is not mutable
					if (nOld == astToBeRescheduled) {						
						*pbCommit = FALSE;
						return;
					}

					// (c.haag 2003-10-15 17:27) - Uncancel the appointment if we're going
					// to set it to a status that is not that of a cancelled appointment. If
					// the user changes their mind or the uncancel fails, don't pursue the
					// commit.
					// (a.walling 2014-12-22 16:44) - PLID 64382 - handle astToBeRescheduled
					if ((nOld == astCancelledNoShow || nOld == astCancelled) &&
						nNew != astCancelledNoShow && nNew != astCancelled)
					{
						NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
						if (pRow)
						{
							long lApptID = VarLong(pRow->GetValue(alcResID));
							if (IDNO == MsgBox(MB_YESNO, "Assigning a status of '%s' will uncancel this appointment. Are you sure you wish to continue?",
								GetShowStateName((EAppointmentShowState)nNew)))
							{
								*pbCommit = FALSE;
								return;
							}
							// (j.jones 2014-12-02 11:36) - PLID 64183 - added pParentWnd
							if (!AppointmentUncancel(this, lApptID))
							{
								*pbCommit = FALSE;
								return;
							}
						}
					}
				}
				break;
			}
			NxCatchAll("Error in EditingFinishingApptList::Status");
			*pbCommit = FALSE;
			break;

		case alcLastLM:
		{
			CString str = strUserEntered;
			str.TrimLeft(" \r\n");
			str.TrimRight(" \r\n");
			if (str.GetLength())
			{
				COleDateTime dt;
				dt.ParseDateTime(str);
				if (dt.GetStatus() != COleDateTime::valid || dt.m_dt == 0)
				{
					MsgBox("The text you have entered is not a valid date.");
					*pbCommit = FALSE;
				}
			}
			else
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				if (pRow) {
					COleVariant var = pRow->GetValue(alcConfirmed);
					if (VarLong(var, 0) == 2)
					{
						if (IDNO == MsgBox(MB_YESNO, "The confirmed status is currently set to 'Left Message.' Are you sure you wish to clear the LM time?"))
							*pbCommit = FALSE;
					}
				}
			}
		}
		break;

		default:
			break;
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2015-01-08 14:35) - PLID 64545 - Reworked to change to DL2
void CAppointmentsDlg::RequeryFinishedApptList(short nFlags)
{
	try {
		//JJ - Ideally we would want to stop the redraw before the requery, but
		//that causes other drawing problems, so this is quite acceptable
		m_pApptList->SetRedraw(FALSE);

		if (m_pApptList->GetRowCount() > 0)
			SetAppointmentColors();

		SetDays();

		// (j.jones 2014-06-09 09:22) - PLID 62336 - just call SetColumnSizes here
		SetColumnSizes();

		m_pApptList->SetRedraw(TRUE);

		// now save the column sizes
		// (j.jones 2014-06-09 09:29) - PLID 62336 - we no longer save in this function
		//SaveColumnSizes();

		// (b.savon 2012-03-29 11:31) - PLID 49074 - Add Recall Icon
		// (j.gruber 2014-12-15 12:38) - PLID 64419 - Rescheduling Queue - Add Notes Icon
		ShowIcons();
	}NxCatchAll(__FUNCTION__);	
}

// (j.gruber 2015-01-08 14:35) - PLID 64545 - Reworked to change to DL2
void CAppointmentsDlg::SelChangedApptList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {

		//check to see if the appointment is cancelled or if there is no appointment selected
		NXDATALIST2Lib::IRowSettingsPtr pNewSel(lpNewSel);		
		if (!pNewSel) {
			//disable the goto appointment button
			GetDlgItem(IDC_GOTOAPP)->EnableWindow(false);
		}
		else {
			//check to see if the appointment is cancelled
			// (a.walling 2013-12-12 16:51) - PLID 60002 - Snapshot isolation loading Appointments dialog
			_RecordsetPtr rsAppointments = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Status FROM AppointmentsT WHERE ID = {INT}", VarLong(pNewSel->GetValue(alcResID)));
			FieldsPtr Fields = rsAppointments->Fields;
			short nStatus = AdoFldByte(Fields, "Status", -1);
			if (nStatus == 4) {
				GetDlgItem(IDC_GOTOAPP)->EnableWindow(false);
			}
			else if (nStatus == -1) {
				GetDlgItem(IDC_GOTOAPP)->EnableWindow(false);
			}
			else {
				GetDlgItem(IDC_GOTOAPP)->EnableWindow(true);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2015-01-08 14:35) - PLID 64545 - Reworked to change to DL2
void CAppointmentsDlg::EditingStartingApptList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {			
			long lApptID = VarLong(pRow->GetValue(alcResID));

			if (!HasPermissionForResource(lApptID, sptWrite, sptWriteWithPass)) {
				*pbContinue = FALSE;
				return;
			}

			if (nCol == alcStatus) {
				if (pvarValue && pvarValue->vt == VT_I4) {					
					long nStatus = VarLong(*pvarValue);
					if (nStatus == astToBeRescheduled) {
						// (a.walling 2015-01-28 14:04) - PLID 64382 - Should not be editable
						*pbContinue = FALSE;
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2015-01-08 14:35) - PLID 64545 - Reworked to change to DL2
void CAppointmentsDlg::ColumnSizingFinishedApptList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {
		//TES 2/9/2004: Plagiarized from ToDoAlarm
		//DRT 6/6/03 - Saves the sizes of all columns if bCommitted is set and the checkbox is set

		//uncommitted
		if (!bCommitted)
			return;

		//don't want to remember
		if (!IsDlgButtonChecked(IDC_APPTS_REMEMBER_COL_SETTINGS))
			return;


		SaveColumnSizes();
		// (j.jones 2014-06-09 09:22) - PLID 62336 - just call SetColumnSizes here
		SetColumnSizes();
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2015-01-08 14:35) - PLID 64545 - Reworked to change to DL2
// (j.gruber 2014-12-15 11:57) - PLID 64419 - Rescheduling Queue - add notes icon
void CAppointmentsDlg::LeftClickApptList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		switch (nCol)
		{
		case alcExtraNotesIcon:
			// (j.gruber 2014-12-15 11:57) - PLID 64419 - Rescheduling Queue - add notes icon
		{
			// If the cell is empty, do nothing
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if (pRow) {
				_variant_t v = pRow->GetValue(nCol);
				if (v.vt == VT_NULL || v.vt == VT_EMPTY) {
					// If we get here, it means the row cannot have notes assigned to it (it most likely does not have a MailID)
					return;
				}
				long nApptID = pRow->GetValue(alcResID);
				if (-1 == nApptID) {
					// We should never get into this state, because it means a appointment item has a notes icon but no ID in the
					// appointments table! That means the note cannot possibly be tied to the appointment. We should throw an exception.
					ThrowNxException("Attempted to add a note to an appointment that is not in the appointments table!");
				}

				// Let the user add extra notes to this history item
				CNotesDlg dlgNotes(this);
				// (a.walling 2010-09-20 11:41) - PLID 40589 - Set the patient ID
				dlgNotes.SetPersonID(m_id);
				COleDateTime dtAppt = VarDateTime(pRow->GetValue(alcDate));
				COleDateTime dtStartTime = VarDateTime(pRow->GetValue(alcStartTime));
				COleDateTime dtFullStartTime;
				dtFullStartTime.SetDateTime(dtAppt.GetYear(), dtAppt.GetMonth(), dtAppt.GetDay(), dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
				dlgNotes.SetApptInformation(nApptID, dtFullStartTime, VarString(pRow->GetValue(alcPurpose), ""), VarString(pRow->GetValue(alcItem), ""));
				dlgNotes.m_bIsForPatient = true;
				dlgNotes.m_clrHistoryNote = m_bg.GetColor(); // (c.haag 2010-08-26 14:49) - PLID 39473			
				CNxModalParentDlg dlg(this, &dlgNotes, CString("Appointment Notes"));
				dlg.DoModal();

				// Now update the icon				
				_RecordsetPtr prs = CreateParamRecordset(
					"SELECT CAST(CASE WHEN NoteInfoT.AppointmentID IS NULL THEN 0 ELSE 1 END AS BIT) AS HasExtraNotes "
					"FROM AppointmentsT LEFT JOIN NoteInfoT ON AppointmentsT.ID = NoteInfoT.AppointmentID WHERE AppointmentsT.ID = {INT}", nApptID);
				if (prs->eof) {
					AfxMessageBox("The appointment could not be found; it may have been deleted by another user. Your screen will now refresh.", MB_OK | MB_ICONWARNING);
					UpdateView();
				}
				else {
					BOOL bHasNotes = AdoFldBool(prs, "HasExtraNotes");
					if (bHasNotes) {
						pRow->PutValue(alcHasExtraNotes, g_cvarTrue);
						pRow->PutValue(alcExtraNotesIcon, (long)m_hExtraNotesIcon);
					}
					else {
						pRow->PutValue(alcExtraNotesIcon, (LPCTSTR)"BITMAP:FILE");
						pRow->PutValue(alcHasExtraNotes, g_cvarFalse);
					}
				}
			}
		}
		break;
		}
	}NxCatchAll(__FUNCTION__)
}

// (z.manning 2015-07-22 15:18) - PLID 67241
void CAppointmentsDlg::OnManagePaymentProfiles()
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptList->CurSel;
		if (pRow == NULL) {
			return;
		}

		long nPatientID = GetActivePatientID();

		OpenPaymentProfileDlg(nPatientID, this);
	}
	NxCatchAll(__FUNCTION__);
}


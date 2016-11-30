// RoomManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RoomManagerDlg.h"
#include "RoomSetupDlg.h"
#include "RoomStatusSetupDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "SchedulerView.h"
#include "GlobalSchedUtils.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"
#include "MultiSelectDlg.h"
#include "SendMessageDlg.h"
#include "GlobalReportUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "PatientView.h"
#include "PatientEmrByDateDlg.h"
#include "SelectDlg.h"
#include "RoomManagerConfigureColumnsDlg.h"		// (d.lange 2010-06-28 15:08) - PLID 37317 - Added Configure Columns button
#include "ApptPopupMenuManager.h"
#include "FileUtils.h" //(a.wilson 2011-11-4) PLID 45936
#include "RecallUtils.h"	// (j.armen 2012-03-05 11:17) - PLID 48555
#include "DeviceLaunchUtils.h" // (j.gruber 2013-04-02 11:48) - PLID 46012
#include "EMRPreviewPopupDlg.h"
#include "BillingModuleDlg.h"
#include "FinancialDlg.h"
#include "PatientDocumentStorageWarningDlg.h"
#include "HistoryUtils.h"
#include "GlobalFinancialUtils.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (a.walling 2013-11-25 11:38) - PLID 60005 - RoomManager only cares about past due or needing scheduled recalls
namespace RecallUtils
{
CSqlFragment SelectRecallStatusesPastDueOrNeeded()
{
	return CSqlFragment(
		"SELECT "
			"  RecallT.PatientID "
			", MAX( "
				"CASE "
					"WHEN RecallT.Discontinued = 1 THEN 1 "
					"WHEN RecallT.RecallAppointmentID IS NOT NULL AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND GETDATE() >= AppointmentsT.StartTime THEN 0 "
					"WHEN RecallT.RecallAppointmentID IS NOT NULL AND AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3 AND GETDATE() <  AppointmentsT.StartTime THEN 3 "
					"WHEN (RecallT.RecallAppointmentID IS NULL OR AppointmentsT.Status = 4 OR AppointmentsT.ShowState = 3) AND dbo.AsDateNoTime(GETDATE()) < dbo.AsDateNoTime(RecallT.RecallDate) THEN 4 "
					"ELSE 5 "
				"END "
			") AS RecallStatusID "
		"FROM RecallT "
		"LEFT JOIN AppointmentsT "
			"ON RecallT.RecallAppointmentID = AppointmentsT.ID "
		"WHERE RecallT.RecallDate >= dbo.AsDateNoTime(GetDate()) "
		"GROUP BY RecallT.PatientID "
	);
}
}

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.

enum {
	miGoToPatient = -1,
	miGoToAppointment = -2,
	// (j.jones 2010-12-09 10:35) - PLID 41763 - removed these as fixed options,
	// they are now handled the same as all other apt. statuses, including custom statuses
	//miMarkAsPending = -3,
	//miMarkAsIn = -4,
	//miMarkAsOut = -5,
	miCheckoutPatient = -6,	// (j.jones 2008-05-29 14:49) - PLID 27797
	// (j.jones 2010-12-09 10:56) - PLID 41763 - supported no show
	miMarkAsNoShow = -7,
	miUnmarkNoShow = -8,
	// (j.jones 2010-07-13 10:42) - PLID 36477 - added several more right-click options	
	// (c.haag 2011-09-14) - PLID 36477 - Deprecated in lieu of CApptPopupMenuManager. We now actually
	// use the definitions in AppointmentsDlg.h for the menu options.
	/*
	miMarkAsConfirmed = -9,
	miMarkAsUnconfirmed = -10,
	miMarkAsLM = -11,
	miMarkAsMoveUp = -12,
	miUnmarkMoveUp = -13,
	miPrintSuperbill = -14,
	miEditInvAllocation = -15,
	miNewInvAllocation = -16,
	miNewInvOrder = -17,
	miNewBill = -18,
	miNewCaseHistory = -19,
	miEditCaseHistory = -20,*/
	// (j.jones 2010-10-13 16:46) - PLID 39778 - added ability to bill today's EMN
	miBillTodaysEMN_Default = -21,
	miBillTodaysEMN_Patient = -22,
	//miWaitingRoomPatient =-25, // (s.dhole 2010-11-16 15:31) - PLID  39200 Menu item status
	// (j.jones 2011-12-16 17:23) - PLID 46289 - supported all active resp type IDs, dynamically
	miBillTodaysEMN_RespTypeIDPivotMin = -50,
	miBillTodaysEMN_RespTypeIDPivotMax = -100,
	//...
	// (j.jones 2011-12-16 17:23) - the next available ID is -101, the RespType code enforces it
	// will not use an ID past -100

	// (c.haag 2010-11-02 12:10) - PLID 39199 - Pivot point for assigning appointments to custom statuses.
	// Do NOT create any new enumerations after this number.
	// (j.jones 2010-12-09 10:27) - PLID 41763 - renamed as this now handles all appt. statuses, custom or fixed
	miMarkAsShowStateStatus = -1000,
};

enum RoomManagerPatientListColumns {

	rmplcID = 0,
	rmplcParentID,
	rmplcApptID,
	rmplcPatientID,
	rmplcPatientName,
	rmplcApptTime,
	rmplcPurpose,	// (j.jones 2008-11-12 16:36) - PLID 28035 - added purpose column
	rmplcHour,
	rmplcMinute,
	rmplcRoomStatusID,
	rmplcApptShowState,
	rmplcAptTypeColor,	// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
	rmplcAptWaitingArea, // (c.haag 2010-10-26 10:07) - PLID 39199 - Added ApptWaitingArea (AptShowStateT.WaitingArea)
	rmplcPatientPicturePath, // (a.wilson 2011-11-7) PLID 45936
};

enum RoomManagerRoomListColumns {

	rmrlcRoomID = 0,
	rmrlcRoomAppointmentID,
	rmrlcApptID,
	rmrlcPatientID,
	rmrlcRoomName,
	rmrlcPatientName,
	rmrlcRecallStatusID,	// (j.armen 2012-03-05 11:17) - PLID 48555 - Added Recall Column
	rmrlcRecallStatus,		// (j.armen 2012-03-05 11:17) - PLID 48555
	rmrlcPurpose,	// (j.jones 2008-11-12 16:36) - PLID 28035 - added purpose column
	rmrlcApptTime,	// (j.jones 2010-08-27 09:37) - PLID 39774 - added appt. time
	rmrlcArrivalTime,		// (d.thompson 2009-07-10) - PLID 26860
	rmrlcCheckInTime,
	rmrlcTimeLastSeen,
	rmrlcStatusID,
	rmrlcWithPerson,
	rmrlcWaiting,
	rmrlcProvider,	// (d.lange 2010-08-30 09:08) - PLID 39431 - added provider column
	rmrlcLastUserID,
	rmrlcLastUserName,
	rmrlcAptTypeColor,	// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
	rmrlcResources,// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource column
	rmrlcPreviewEMN,	// (d.lange 2010-11-29 12:53) - PLID 40295 - added Preview EMNs column
};

enum RoomManagerWaitingAreaListColumns {

	rmwalcApptID = 0,
	rmwalcWaitingRoomID,	// (j.jones 2010-12-01 17:38) - PLID 38597 - added Waiting Room fields
	rmwalcWaitingRoomName,
	rmwalcWaitingRoomAppointmentID,
	rmwalcPatientID,
	rmwalcPatientName,
	rmwalcRecallStatusID,	// (j.armen 2012-03-05 11:17) - PLID 48555 - Added Recall Column
	rmwalcRecallStatus,		// (j.armen 2012-03-05 11:17) - PLID 48555
	rmwalcApptTime,
	rmwalcPurpose,	// (j.jones 2008-11-12 16:36) - PLID 28035 - added purpose column
	rmwalcCheckInTime,
	rmwalcTimeLastSeen,
	rmwalcWaiting,
	rmwalcProvider,	// (d.lange 2010-08-30 17:32) - PLID 39431 - added provider column
	rmwalcCheckedInBy,	// (j.jones 2009-08-03 09:34) - PLID 26862 - added checked in username
	rmwalcAptTypeColor,	// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
	rmwalcResources,// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource column
	rmwalcPreviewEMN,	// (d.lange 2010-11-29 11:03) - PLID 40295 - Added Preview EMN columns
};

// (j.jones 2008-05-29 09:09) - PLID 27797 - added the checkout list
enum RoomManagerCheckoutListColumns {

	rmclcRoomAppointmentID = 0,
	rmclcApptID,
	rmclcPatientID,
	rmclcPatientName,
	rmclcRecallStatusID,	// (j.armen 2012-03-05 11:18) - PLID 48555 - Added Recall Column
	rmclcRecallStatus,		// (j.armen 2012-03-05 11:18) - PLID 48555
	rmclcApptTime,
	rmclcPurpose,	// (j.jones 2008-11-12 16:36) - PLID 28035 - added purpose column
	rmclcCheckInTime,
	rmclcTimeLeftRoom,
	rmclcWaiting,
	rmclcProvider,	// (d.lange 2010-08-30 17:32) - PLID 39431 - added provider column
	rmclcLastUserID,
	rmclcLastUserName,
	rmclcAptTypeColor,	// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
	rmclcResources,// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource column
	rmclcPreviewEMN,	// (d.lange 2010-11-29 14:35) - PLID 40295 - added Preview EMN column
};

// (j.jones 2009-09-22 09:51) - PLID 25232 - added location combo enum
enum LocationComboColumns {

	lccID = 0,
	lccName,
};

// (j.jones 2009-09-22 09:51) - PLID 25232 - added resource combo enum
enum ResourceFilterComboColumns {

	rfccID = 0,
	rfccName,
};
// (a.wilson 2011-11-7) PLID 46314 - enum for picture tooltip preferences
enum PicturToolTipPreference {
	Show = 0,
	ShowNoDelay = 1,
	Hide = 2,
};


// (s.dhole 2010-02-11 16:38) - PLID 37112 Workflow change from room manager -> EMR for doctors
enum RoomListPrefereance {
	lpDoNothing= 0,
	lpOpenGeneral1,
	lpOpenEMR,
	lpOpenExistEMN,
	lpCreateEMN,
};

// (j.jones 2010-12-02 09:03) - PLID 38597 - added the waiting room combo
enum WaitingRoomComboColumns {

	wrccID = 0,
	wrccName,
};

/////////////////////////////////////////////////////////////////////////////
// CRoomManagerDlg dialog


// (a.walling 2013-02-11 17:25) - PLID 54087 - PracYakker, Room Manager should always stay on top of the main Practice window.
CRoomManagerDlg::CRoomManagerDlg(CWnd* pParent)
	: CNxModelessOwnedDialog(CRoomManagerDlg::IDD, pParent)
{
	EnableDragHandle(false);	// (j.armen 2012-05-30 11:46) - PLID 49854 - Drag handle doesn't look good here.  Disable it.
	m_nTimerHourExpansion = 42420; //just initializing the value to something
	m_nTimerHourlyServerTimeCheck = 42421;  //just initializing the value to something
	m_nTimerFullRefresh = 42422;

	m_clrApptCheckedOut = RGB(192,192,192);
	m_clrApptCheckedIn = RGB(0,0,255);
	m_clrApptReadyToCheckOut = RGB(0,192,0);
	m_clrApptNoShow = RGB(255,0,0);

	m_clrFolderEmpty = RGB(192,192,192);

	m_nPendingShowAppointment = -1;

	m_bIsResourceComboHidden = FALSE;
	m_bEMRExpired=false;

	m_nTopRoomID = -1; // (c.haag 2010-05-07 11:17) - PLID 35702
	m_nTopWaitingAreaID = -1; // (c.haag 2010-08-02 10:46) - PLID 35702
	m_nTopCheckoutID = -1; // (c.haag 2010-08-02 10:46) - PLID 35702

	m_eRoomMgrPatientNameDisplayPref = patnameFullName;

	m_bColorApptListByType = FALSE;
	m_bColorPurposeColumnByType = FALSE;

	// (d.lange 2010-12-08 14:58) - PLID 40295 - Added EMR Preview window
	m_pEMRPreviewPopupDlg = NULL;

	//m_rcMultiResourceLabel.top = m_rcMultiResourceLabel.bottom = m_rcMultiResourceLabel.left = m_rcMultiResourceLabel.right = 0;
}


void CRoomManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	// (j.gruber 2009-07-10 10:44) - PLID 28792 - remember column widths check
	// (d.lange 2010-06-28 15:00) - PLID 37317 - Configure Columns
	CNxModelessOwnedDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRoomManagerDlg)
	DDX_Control(pDX, IDC_CLOSE_ROOM_MANAGER, m_btnClose);
	DDX_Control(pDX, IDC_BTN_ROOM_EDITOR, m_btnEditRooms);
	DDX_Control(pDX, IDC_BTN_EDIT_ROOM_STATUS, m_btnEditStatus);
	DDX_Control(pDX, IDC_ROOM_MGR_MULTI_RESOURCE_LABEL, m_nxlRoomMgrMultiResourceLabel);// (s.dhole 2010-06-30 16:58) - PLID  38947 Add some color to the Room Manager
	DDX_Control(pDX, IDC_ROOM_MANAGER_REMEMBER_COL_WIDTHS, m_chkRememberColumnWidths);
	// (j.jones 2009-09-21 14:41) - PLID 25232 - added print option
	DDX_Control(pDX, IDC_BTN_PREVIEW_ROOM_MGR, m_btnPrintPreview);
	DDX_Control(pDX, IDC_BTN_CONFIG_COLUMNS, m_btnConfigColumns);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRoomManagerDlg, CNxModelessOwnedDialog)
	ON_BN_CLICKED(IDC_CLOSE_ROOM_MANAGER, OnClose)
	ON_BN_CLICKED(IDC_BTN_ROOM_EDITOR, OnBtnRoomEditor)
	ON_BN_CLICKED(IDC_BTN_EDIT_ROOM_STATUS, OnBtnEditRoomStatus)
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_WM_ACTIVATE()	//(a.wilson 2011-11-4) PLID 45936
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)// (s.dhole 2010-06-30 16:59) - PLID  38947 Add some color to the Room Manager
	ON_BN_CLICKED(IDC_ROOM_MANAGER_REMEMBER_COL_WIDTHS, &CRoomManagerDlg::OnRememberColWidths)
	ON_BN_CLICKED(IDC_BTN_PREVIEW_ROOM_MGR, &CRoomManagerDlg::OnBtnPreviewRoomMgr)
	ON_BN_CLICKED(IDC_BTN_CONFIG_COLUMNS, OnBnClickedBtnConfigColumns)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRoomManagerDlg message handlers

BOOL CRoomManagerDlg::OnInitDialog() 
{
	CNxModelessOwnedDialog::OnInitDialog();
	// (s.dhole 2010-02-08 13:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
	m_bEMRExpired = g_pLicense->HasEMR(CLicense::cflrSilent) != 2;
	// (z.manning, 04/30/2008) - PLID 29845 - Set button styles
	m_btnClose.AutoSet(NXB_CLOSE);
	m_btnEditRooms.AutoSet(NXB_MODIFY);
	m_btnEditStatus.AutoSet(NXB_MODIFY);
	// (d.lange 2010-06-28 14:54) - PLID 37317 - Added 'Configure Columns' button
	m_btnConfigColumns.AutoSet(NXB_MODIFY);

	// (j.jones 2009-09-21 14:41) - PLID 25232 - added print option
	m_btnPrintPreview.AutoSet(NXB_PRINT_PREV);

	// (z.manning 2009-07-10 13:44) - PLID 34848 - Added bulk caching
	// (j.gruber 2009-07-15 11:34) - PLID 28792 - added remember columns
	g_propManager.CachePropertiesInBulk("CRoomManagerDlg", propNumber,
		"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
		"	'DisplayTaskbarIcons', \r\n"
		"	'RememberRoomManagerColumns', \r\n"
		"	'RoomManagerFullRefreshSeconds', \r\n"
		"	'MarkApptInOnRoomAssign', \r\n"
		"	'RoomManagerMaxWaitTime', \r\n"
		"	'RoomManagerExpandMinutesInAdvance', \r\n"
		"   'RememberRoomManagerColumns', \r\n"
		"	'RoomMgrPatientSelection', \r\n"  //(s.dhole 2010-02-24 16:46) - PLID 37112 Workflow change from room manager -> EMR for doctors.
		"	'RoomMgrPatientSelection_Action', \r\n" //(s.dhole 2010-02-24 16:46) - PLID 37112 Workflow change from room manager -> EMR for doctors.
		"	'RoomMgrPatientSelection_RoomStatusID', \r\n" //(s.dhole 2010-02-24 16:46) - PLID 37112 Workflow change from room manager -> EMR for doctors.
		"	'RoomMgrPatientNameDisplay', \r\n"	// (j.jones 2010-08-27 10:17) - PLID 36975 - added pref. for patient name display
		// (j.jones 2010-08-31 10:54) - PLID 35012 - added prefs to color by appt. type
		"	'RoomMgr_ColorApptListByType', \r\n"
		"	'ShowRoomManagerWaitingAreaColumnResources', \r\n" //// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource
		"	'ShowRoomManagerWaitingAreaColumnProvider', \r\n"	// (d.lange 2010-11-10 12:44) - PLID 39431 - Added Provider
		"	'ShowRoomManagerWaitingAreaColumnPreview EMN', \r\n"	// (d.lange 2010-11-30 11:07) - PLID 40295 - Added Preview EMNs
		"	'ShowRoomManagerWaitingAreaColumnRecall', \r\n"		// (j.armen 2012-03-05 11:18) - PLID 48555
		"	'ShowRoomManagerRoomsColumnResources', \r\n" //// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource
		"	'ShowRoomManagerRoomsColumnProvider', \r\n"		// (d.lange 2010-11-10 12:42) - PLID 39431 - Added Provider
		"	'ShowRoomManagerRoomsColumnPreview EMN', \r\n"	// (d.lange 2010-11-30 11:07) - PLID 40295 - Added Preview EMNs
		"	'ShowRoomManagerRoomsColumnRecall', \r\n"			// (j.armen 2012-03-05 11:18) - PLID 48555
		"	'ShowRoomManagerCheckoutColumnResources', \r\n" //// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource
		"	'ShowRoomManagerCheckoutColumnProvider', \r\n"	// (d.lange 2010-11-10 12:42) - PLID 39431 - Added Provider
		"	'ShowRoomManagerCheckoutColumnPreview EMN', \r\n"	// (d.lange 2010-11-30 11:07) - PLID 40295 - Added Preview EMNs
		"	'ShowRoomManagerCheckoutColumnRecall', \r\n"		// (j.armen 2012-03-05 11:18) - PLID 48555
		"	'RoomMgr_ColorPurposeColumnByType', \r\n"
		"	'BillEMNTo', \r\n"	// (j.jones 2010-10-13 17:25) - PLID 39778 - added the ability to bill today's EMN
		"	'RoomManagerWaitingRoomFilter', \r\n"	// (j.jones 2010-12-02 09:09) - PLID 38597 - added waiting room filter
		// (j.jones 2011-02-08 14:02) - PLID 39774 - added some more missing cached values
		"	'ShowRoomManagerRoomsColumnRoom', \r\n"
		"	'ShowRoomManagerRoomsColumnPatient Name', \r\n"
		"	'ShowRoomManagerRoomsColumnPurpose', \r\n"
		"	'ShowRoomManagerRoomsColumnAppointment Time', \r\n"
		"	'ShowRoomManagerRoomsColumnArrival Time', \r\n"
		"	'ShowRoomManagerRoomsColumnChecked In', \r\n"
		"	'ShowRoomManagerRoomsColumnLast Seen', \r\n"
		"	'ShowRoomManagerRoomsColumnStatus', \r\n"
		"	'ShowRoomManagerRoomsColumnWaiting', \r\n"
		"	'ShowRoomManagerRoomsColumnLast Updated By', \r\n"
		"	'ShowRoomManagerWaitingAreaColumnRoom', \r\n"
		"	'ShowRoomManagerWaitingAreaColumnPatient Name', \r\n"
		"	'ShowRoomManagerWaitingAreaColumnAppointment Time', \r\n"
		"	'ShowRoomManagerWaitingAreaColumnPurpose', \r\n"
		"	'ShowRoomManagerWaitingAreaColumnChecked In', \r\n"
		"	'ShowRoomManagerWaitingAreaColumnChecked In By', \r\n"
		"	'ShowRoomManagerWaitingAreaColumnLast Seen', \r\n"
		"	'ShowRoomManagerWaitingAreaColumnWaiting', \r\n"
		"	'ShowRoomManagerCheckoutColumnPatient Name', \r\n"
		"	'ShowRoomManagerCheckoutColumnAppointment Time', \r\n"
		"	'ShowRoomManagerCheckoutColumnPurpose', \r\n"
		"	'ShowRoomManagerCheckoutColumnChecked In', \r\n"
		"	'ShowRoomManagerCheckoutColumnTime Left Room', \r\n"
		"	'ShowRoomManagerCheckoutColumnWaiting', \r\n"
		"	'ShowRoomManagerCheckoutColumnLast Updated By', \r\n"
		"	'RoomMgr_KeepStatusWhenSwitchingRooms', \r\n"	//(a.wilson 2011-10-3) PLID 33592 - caching keepstatus preference.
		"	'RoomMgr_PatientListPictureToolTips' \r\n"	//(a.wilson 2011-11-7) PLID 46314 - caching preference for tooltips.
		")"
		, _Q(GetCurrentUserName()));

	g_propManager.CachePropertiesInBulk("CRoomManagerDlg_Text", propText,
		"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
		" 'DefaultRoomManagerWaitingColumnSizes', \r\n"
		" 'DefaultRoomManagerRoomColumnSizes', \r\n"
		" 'DefaultRoomManagerCheckOutColumnSizes', \r\n"
		" 'RoomManagerResourceFilter' \r\n"
		")"
		, _Q(GetCurrentUserName()));

	// PLID 22856 - Set my windows free! By which I could only mean, let this window
	//		have its own taskbar icon and not be constrained by the MDI window.
	if (GetRemotePropertyInt("DisplayTaskbarIcons", 1, 0, GetCurrentUserName(), true) == 1) {
		HWND hwnd = GetSafeHwnd();
		long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
		nStyle |= WS_EX_APPWINDOW;
		SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
	}

	CalculateLocalTimeOffset();

	// (j.jones 2010-08-27 10:26) - PLID 36975 - load the patient name preference
	m_eRoomMgrPatientNameDisplayPref = (ERoomMgrPatientNameDisplayPref)GetRemotePropertyInt("RoomMgrPatientNameDisplay", (long)patnameFullName, 0, GetCurrentUserName(), true);

	// (j.jones 2010-08-31 14:45) - PLID 35012 - cache the coloring preferences
	m_bColorApptListByType = GetRemotePropertyInt("RoomMgr_ColorApptListByType", 0, 0, GetCurrentUserName(), true) == 1;	
	m_bColorPurposeColumnByType = GetRemotePropertyInt("RoomMgr_ColorPurposeColumnByType", 0, 0, GetCurrentUserName(), true) == 1;
	
	// (j.jones 2006-12-21 15:54) - PLID 23196 - added resource filter
	m_pResourceCombo = BindNxDataList2Ctrl(IDC_ROOM_MGR_RESOURCE_COMBO,false);
	m_pPatientList = BindNxDataList2Ctrl(IDC_TODAYS_PATIENTS_LIST,false);
	m_pRoomList = BindNxDataList2Ctrl(IDC_ROOM_LIST,false);
	// (j.jones 2007-02-07 08:41) - PLID 24595 - added waiting area
	m_pWaitingAreaList = BindNxDataList2Ctrl(IDC_WAITING_AREA_LIST,false);
	// (j.jones 2008-05-28 17:36) - PLID 27797 - added the checkout list
	m_pCheckoutList = BindNxDataList2Ctrl(IDC_CHECKOUT_LIST,false);
	// (j.jones 2009-08-03 17:28) - PLID 24600 - added the location combo
	m_pLocationCombo = BindNxDataList2Ctrl(IDC_ROOM_MGR_LOCATION_COMBO,true);
	// (j.jones 2010-12-02 09:03) - PLID 38597 - added the waiting room combo
	m_pWaitingRoomCombo = BindNxDataList2Ctrl(IDC_ROOM_MGR_WAITINGROOM_COMBO, false);
	
	//default to the current location
	long nLocationID = GetCurrentLocationID();	
	m_pLocationCombo->SetSelByColumn(lccID, GetCurrentLocationID());
	// (s.tullis 2013-11-01 10:30) - PLID 37164 - In room manager, the client only wants the resources with scheduled appts to be populated in the Filter on Resource dropdown. 
	RequeryResourceList();
	
	// (j.jones 2006-12-22 08:57) - PLID 23196 - find their saved resource filter (defaults to all resources)
	m_strResourceIDs = GetRemotePropertyText("RoomManagerResourceFilter", "", 0, GetCurrentUserName(), true);
	
	//now display the resources accordingly
	ToggleResourceDisplay();

	// (j.jones 2010-12-02 09:03) - PLID 38597 - added the waiting room combo,
	// which filters by location, and remembers the user's last setting
	RequeryWaitingRoomCombo();	
	
	RequeryPatientList(TRUE);

	//added location filter, filtering on the room location, not the appt. location,
	//though they should be the same
	CString strRoomWhere;
	strRoomWhere.Format("RoomLocationID = %li", nLocationID);
	m_pRoomList->WhereClause = _bstr_t(strRoomWhere);

	RequeryRoomList(); // (c.haag 2010-05-07 11:16) - PLID 35702 - Requery the room list in its own function	

	// (j.jones 2007-02-07 09:19) - PLID 24595 - requery the Waiting Area list
	RequeryWaitingAreaList();

	// (j.jones 2008-05-29 10:53) - PLID 27797 - requery the Checkout list
	RequeryCheckoutList();

	// (j.gruber 2009-07-10 10:13) - PLID 28792 - remember column widths
	SetDefaultColumnWidths();

	if(GetRemotePropertyInt("RememberRoomManagerColumns", 0, 0, GetCurrentUserName(), true) == 1) {
		CheckDlgButton(IDC_ROOM_MANAGER_REMEMBER_COL_WIDTHS, TRUE);
		OnRememberColWidths();
	}
	else {
		CheckDlgButton(IDC_ROOM_MANAGER_REMEMBER_COL_WIDTHS, FALSE);
		// (d.lange 2010-06-28 16:35) - PLID 37317 - Ensures which columns have been configured to show in Waiting Area datalist
		EnsureWaitingAreaColumns();

		// (d.lange 2010-06-30 10:30) - PLID 37317 - Ensures which columns have been configured to show in Rooms datalist
		EnsureRoomsColumns();

		// (d.lange 2010-06-30 10:51) - PLID 37317 - Ensures which columns have been configured to show in Checkout datalist
		EnsureCheckoutColumns();
	}

	long nFullRefreshSeconds = GetRemotePropertyInt("RoomManagerFullRefreshSeconds", 300, 0, "<None>", true);
	m_nTimerFullRefresh = SetTimer(m_nTimerFullRefresh, nFullRefreshSeconds * 1000, NULL);

	// (c.haag 2010-10-26 10:07) - PLID 39199 - Update the map that associates show state ID's with waiting area bits.
	// (j.jones 2010-12-09 10:20) - PLID 41763 - renamed this function, we now load all show states
	UpdateCachedShowStateInfo();

	// (d.lange 2010-12-08 16:28) - PLID 40295 - EMR Preview Icon
	m_hIconPreview = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INSPECT), IMAGE_ICON, 16,16, 0);

	// (j.armen 2012-03-05 11:19) - PLID 48555 - Flag Icon
	m_hIconRecallFlag = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_RECALL), IMAGE_ICON, 16, 16, 0);

	//(a.wilson 2011-10-27) PLID 45936 - initialize the picture tool tip object.
	//(a.wilson 2011-11-7) PLID 46314 - only initialize if the preference is correct.
	long nPictureToolTipPref = GetRemotePropertyInt("RoomMgr_PatientListPictureToolTips", 0, 0, 
		GetCurrentUserName(), true);

	switch (nPictureToolTipPref) {
		case Show:
			m_pttPatientList.Initialize(m_hWnd);
			break;
		case ShowNoDelay:
			m_pttPatientList.Initialize(m_hWnd, TRUE);
			break;
		case Hide:	//nothing happens here, NxPictureToolTip object handles this.
			break;
		default:
			m_pttPatientList.Initialize(m_hWnd);
			break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRoomManagerDlg::OnCancel() {

	CloseRoomManager();	
	//CNxModelessOwnedDialog::OnCancel();
}

void CRoomManagerDlg::OnClose() 
{
	CloseRoomManager();
	//CNxModelessOwnedDialog::OnOK();
}

void CRoomManagerDlg::CloseRoomManager()
{
	// (d.lange 2010-12-08 17:27) - PLID 40295 - Destroy EMR Preview object
	if (m_pEMRPreviewPopupDlg) {
		m_pEMRPreviewPopupDlg->DestroyWindow();
		delete m_pEMRPreviewPopupDlg;
		m_pEMRPreviewPopupDlg = NULL;
	}

	// (d.lange 2011-01-17 17:27) - PLID 40295 - Need to destroy the icon on close
	if(m_hIconPreview) {
		DestroyIcon(m_hIconPreview);
	}
	m_hIconPreview = NULL;

	// (j.armen 2012-03-05 11:19) - PLID 48555 - Destroy flag icon
	if(m_hIconRecallFlag) 
		DestroyIcon(m_hIconRecallFlag);

	GetMainFrame()->PostMessage(NXM_ROOM_MANAGER_CLOSED);

	// (a.walling 2013-02-11 17:25) - PLID 54087 - Call CNxModelessOwnedDialog::CloseDialog immediately here
	CloseDialog();
}

// (c.haag 2010-10-26 10:07) - PLID 39199 - Populates a map where the key is AptShowStateT.ID and the value
// is the AptShowStateT.WaitingArea bit. We need this map when processing table checkers.
// (j.jones 2010-12-09 10:20) - PLID 41763 - renamed this function, we now load all show states
void CRoomManagerDlg::UpdateCachedShowStateInfo()
{
	m_mapShowStateIsWaitingArea.RemoveAll();
	m_mapShowStateNames.RemoveAll();
	m_aryShowStates.RemoveAll();

	// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
	_RecordsetPtr prs = CreateRecordset(GetRemoteDataSnapshot(), "SELECT Name, ID, WaitingArea FROM AptShowStateT");
	FieldsPtr f = prs->Fields;
	while (!prs->eof) {
		long nID = AdoFldLong(f, "ID");
		m_aryShowStates.Add(nID);
		m_mapShowStateIsWaitingArea.SetAt(nID, AdoFldBool(f, "WaitingArea")); // Both are non-null
		m_mapShowStateNames.SetAt(nID, AdoFldString(f, "Name")); // Both are non-null
		prs->MoveNext();
	}
	prs->Close();
}

void CRoomManagerDlg::RequeryAll(BOOL bRebuildFolders)
{
	RequeryPatientList(bRebuildFolders);

	// (j.jones 2009-08-04 08:55) - PLID 24600 - added location filter,
	// filtering on the room location, not the appt. location, though they should be the same
	long nLocationID = -1;
	{
		IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
		if(pRow == NULL) {
			nLocationID = GetCurrentLocationID();
			m_pLocationCombo->SetSelByColumn(lccID, nLocationID);
		}
		else {
			nLocationID = VarLong(pRow->GetValue(lccID));
		}
	}

	// (j.jones 2010-12-02 09:09) - PLID 38597 - requery waiting room combo
	RequeryWaitingRoomCombo();

	CString strRoomWhere;
	strRoomWhere.Format("RoomLocationID = %li", nLocationID);
	m_pRoomList->WhereClause = _bstr_t(strRoomWhere);
	RequeryRoomList(); // (c.haag 2010-05-07 11:16) - PLID 35702 - Requery the room list in its own function

	RequeryWaitingAreaList();

	// (j.jones 2008-05-29 10:53) - PLID 27797 - requery the Checkout list
	RequeryCheckoutList();
	
}

void CRoomManagerDlg::RequeryPatientList(BOOL bRebuildFolders)
{
	try {

		// (s.dhole 2010-02-08 13:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
		m_nRoomMgrPatientSelection=GetRemotePropertyInt("RoomMgrPatientSelection", 0, 0, GetCurrentUserName(), true);
		// (s.dhole 2010-02-08 13:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
		m_nRoomMgrPatientSelection_action=GetRemotePropertyInt("RoomMgrPatientSelection_Action", 0, 0, GetCurrentUserName(), true);
		if (( m_nRoomMgrPatientSelection== 1) || (m_nRoomMgrPatientSelection_action>0)) 
		{
			NXDATALIST2Lib::IColumnSettingsPtr ptr= m_pRoomList->GetColumn(rmrlcPatientName) ;
			ptr->FieldType= cftTextSingleLineLink;
		}
		else
		{
			NXDATALIST2Lib::IColumnSettingsPtr ptr= m_pRoomList->GetColumn(rmrlcPatientName) ;
			ptr->FieldType= cftTextSingleLine;
		}


		//folders need rebuilt if the following has changed:
		//- an appointment is outside the office hours
		//- the office hours have changed
		//- an event has been made, when no previous events exist
		//- the resource filter has changed
		
		//if not rebuilding, do a simple requery and leave
		if(!bRebuildFolders && AsString(m_pPatientList->FromClause) != "") {
			m_pPatientList->Requery();

			// appointment was added requery resource list	
			// (s.tullis 2013-11-13 14:03) - PLID 37164 - In room manager, the client only wants the resources with scheduled appts to be populated in the Filter on Resource dropdown.
			RequeryResourceList();
			return;
		}

		//now, rebuild the folder structure

		//first we need to know our time range (default to 7am-7pm)
		long nFirstHour = 7;
		long nLastHour = 19;
		
		//TODO: if we add the ability to filter on location, use the office hours for that location.
		//Do so by passing in a locationID as the third parameter.
		CalculateTimeRange(nFirstHour, nLastHour);
		if (nFirstHour == -1 || nLastHour == -1) {
			// (a.walling 2007-07-02 15:38) - PLID 26169
			// this is only possible if there are no appointments on this day and the office hours are closed
			return;
		}

		// (j.jones 2006-12-22 09:12) - PLID 23196 - utilize the resource filter
		CString strResourceFilter, strResourceIDs;
		strResourceIDs = m_strResourceIDs;
		strResourceIDs.TrimRight();
		if(!strResourceIDs.IsEmpty()) {
			strResourceIDs.Replace(" ", ",");
			strResourceFilter.Format(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN (%s))", strResourceIDs);
		}

		// (j.jones 2009-08-04 08:55) - PLID 24600 - added location filter
		long nLocationID = -1;
		{
			IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
			if(pRow == NULL) {
				nLocationID = GetCurrentLocationID();
				m_pLocationCombo->SetSelByColumn(lccID, nLocationID);
			}
			else {
				nLocationID = VarLong(pRow->GetValue(lccID));
			}
		}

		// (a.walling 2014-10-15 06:44) - PLID 63884 - AppointmentsT range queries on StartTime generate disastrously inaccurate cardinality estimates in the SQL optimizer

		CString literalDateFrom = FormatDateTimeForSql(AsDateNoTime(COleDateTime::GetCurrentTime()));
		CString literalDateTo = FormatDateTimeForSql(AsDateNoTime(COleDateTime::GetCurrentTime() + COleDateTimeSpan(1, 0, 0, 0)));

		//generate the sql for today's appointments
		CString strApptSql;
		// (j.jones 2007-09-06 11:58) - PLID 27312 - Events now have a parent ID of -2, and we calculate events the same way we do
		// program wide, which is a start time in the 12am hour and an identical end time. The EventHour does not need changed.
		// (j.jones 2007-09-06 15:52) - PLID 27318 - fixed so we don't report NULL as the start/end times for events
		// (j.jones 2008-11-12 16:56) - PLID 28035 - added purpose text
		// (j.jones 2010-08-27 10:31) - PLID 36975 - the patient name is now calculated by a preference			
		// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor
		// (c.haag 2010-10-26 10:07) - PLID 39199 - Added ApptWaitingArea
		// (a.wilson 2011-10-27) PLID 45936 - Added the PicturePath Field for Picture tool tips.
		// (a.walling 2013-07-11 15:34) - PLID 57526 - Filter on StartTime range to avoid table scans
		// (a.walling 2013-08-02 14:55) - PLID 57841 - PicturePathQ removed join on personID; can cause sql to scan rather than seek. PatPrimaryHistImage is already the ID!
		strApptSql.Format("SELECT NULL AS ID, CASE WHEN StartTime = EndTime AND DatePart(hh, StartTime) = 0 THEN -2 ELSE DATEPART(hh, StartTime) END AS ParentID, '' AS FolderName, "
			"AppointmentsT.ID AS ApptID, PersonT.ID AS PatientID, %s AS PatientName, "
			"AppointmentsT.StartTime, AppointmentsT.EndTime, "
			"DATEPART(hh, StartTime) AS Hour, DATEPART(mi, StartTime) AS Minute, "
			""
			//find the current status ID, in order of "none", "not checked out", or "checked out" / "ready to check out"
			// (j.jones 2010-12-02 16:49) - PLID 38597 - ignore waiting rooms
			"CASE WHEN AppointmentsT.ID NOT IN (SELECT AppointmentID FROM RoomAppointmentsT "
			"	INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
			"	WHERE AppointmentID = AppointmentsT.ID) THEN NULL "
			"WHEN AppointmentsT.ID IN (SELECT AppointmentID FROM RoomAppointmentsT "
			"	INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
			"	WHERE AppointmentID = AppointmentsT.ID AND StatusID NOT IN (0, -1) AND RoomsT.WaitingRoom = 1) THEN NULL "
			"WHEN AppointmentsT.ID IN (SELECT AppointmentID FROM RoomAppointmentsT "
			"	INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
			"	WHERE AppointmentID = AppointmentsT.ID AND StatusID NOT IN (0, -1) AND RoomsT.WaitingRoom = 0) THEN (SELECT TOP 1 StatusID FROM RoomAppointmentsT INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID WHERE AppointmentID = AppointmentsT.ID AND StatusID NOT IN (0, -1) AND RoomsT.WaitingRoom = 0 ORDER BY CheckInTime DESC) "				
			"WHEN AppointmentsT.ID IN (SELECT AppointmentID FROM RoomAppointmentsT "
			"	INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
			"	WHERE AppointmentID = AppointmentsT.ID AND StatusID IN (0, -1) AND RoomsT.WaitingRoom = 0) THEN (SELECT TOP 1 StatusID FROM RoomAppointmentsT INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID WHERE AppointmentID = AppointmentsT.ID AND StatusID IN (0, -1) AND RoomsT.WaitingRoom = 0 ORDER BY CheckInTime DESC) "
			"ELSE NULL "
			"END AS StatusID, "
			""
			"AppointmentsT.ShowState, AptShowStateT.WaitingArea AS ApptWaitingArea, "
			"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, "
			"AptTypeT.Name AS TypeName, CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE "
			"AptTypeT.Color END AS AptTypeColor, PicturePathQ.PathName AS PicturePath "
			"FROM AppointmentsT "
			"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"LEFT JOIN (SELECT MailID, PathName FROM MailSent WHERE dbo.IsImageFile(PathName) = 1 AND "
			"(MailSent.IsPhoto IS NULL OR MailSent.IsPhoto = 1)) AS PicturePathQ ON PicturePathQ.MailID = PatientsT.PatPrimaryHistImage "
			"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID "
			"WHERE AppointmentsT.Status <> 4 "
			"AND AppointmentsT.LocationID = %li "
			"AND AppointmentsT.StartTime >= '%s' AND AppointmentsT.StartTime < '%s' "
			"%s"
			, GetPatientNameSqlFormat().Flatten()
			, nLocationID
			, literalDateFrom, literalDateTo
			, strResourceFilter
		);

		//now generate the sql for the "folders", one for each hour
		//don't forget to add a folder for events, if any exist

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (a.walling 2013-07-11 15:34) - PLID 57526 - Filter on StartTime range to avoid table scans
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
		// (a.walling 2014-10-15 06:44) - PLID 63884 - AppointmentsT range queries on StartTime generate disastrously inaccurate cardinality estimates in the SQL optimizer
		BOOL bEventsExist = ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM AppointmentsT WHERE Status <> 4 AND "
			"AppointmentsT.StartTime = dbo.AsDateNoTime(GetDate()) "
			"AND StartTime = EndTime AND DatePart(hh, StartTime) = 0 "
			"AND AppointmentsT.LocationID = {INT} "
			"{CONST_STRING}", nLocationID, strResourceFilter);

		CString strFolderSql;
		if(bEventsExist) {
			// (j.jones 2007-09-06 11:57) - PLID 27312 - The Event folder now has an ID of -2, not 0, and an Hour of -2 as well,
			// although Events inside this folder will continue to have an hour of 0.
			//(a.wilson 2011-10-27) PLID 45936 - need to set PicturePath to NULL HERE for union
			strFolderSql += "SELECT -2 AS ID, NULL AS ParentID, 'Events' AS FolderName, NULL AS ApptID, NULL AS PatientID, "
				"NULL AS PatientName, NULL AS StartTime, NULL AS EndTime, "
				"-2 AS Hour, 0 AS Minute, NULL AS StatusID, NULL AS ShowState, NULL AS ApptWaitingArea, '' AS PurposeName, '' AS TypeName, 0 AS AptTypeColor, '' AS PicturePath "
				"UNION ALL ";
		}
		for(int i=nFirstHour; i<nLastHour; i++) {
			CString strSql, strFolderName;
			COleDateTime dtIntervalStart, dtIntervalEnd;
			dtIntervalStart.SetTime(i,0,0);
			// (j.jones 2007-02-23 09:18) - PLID 24898 - if our index is 23, do not
			// create a time with an hour of 24 (which is invalid), it should be 0
			//(a.wilson 2011-10-27) PLID 45936 - added PicturePath as Null for Folders
			dtIntervalEnd.SetTime(i == 23 ? 0 : i+1,0,0);
			strFolderName.Format("%s - %s", FormatDateTimeForInterface(dtIntervalStart, DTF_STRIP_SECONDS, dtoTime), FormatDateTimeForInterface(dtIntervalEnd, DTF_STRIP_SECONDS, dtoTime));
			strSql.Format("SELECT %li AS ID, NULL AS ParentID, '%s' AS FolderName, NULL AS ApptID, NULL AS PatientID, "
				"NULL AS PatientName, NULL AS StartTime, NULL AS EndTime, "
				"%li AS Hour, 0 AS Minute, NULL AS StatusID, NULL AS ShowState, NULL AS ApptWaitingArea, '' AS PurposeName, '' AS TypeName, 0 AS AptTypeColor, '' AS PicturePath "
				"UNION ALL ", i, strFolderName, i);
			strFolderSql += strSql;
		}

		CString strDataListSql;
		strDataListSql.Format("(%s %s) AS TodaysPatientsQ", strFolderSql, strApptSql);
		m_pPatientList->FromClause = _bstr_t(strDataListSql);
		m_pPatientList->Requery();
		
		// appointment was added requery resource list	
		// (s.tullis 2013-11-13 14:03) - PLID 37164 - In room manager, the client only wants the resources with scheduled appts to be populated in the Filter on Resource dropdown.
		RequeryResourceList();
		
	}NxCatchAll("Error in CRoomManagerDlg::RequeryPatientList");
}

void CRoomManagerDlg::CalculateTimeRange(long &nFirstHour, long &nLastHour, long nLocationID /*= -1*/)
{
	try {

		//first set the office hours to be the earliest / latest available times of the day		
		
		COleDateTime dtStartTime, dtEndTime;
		//initialize to 7am - 7pm
		dtStartTime.SetDateTime(0,0,0,7,0,0);
		dtEndTime.SetDateTime(0,0,0,19,0,0);

		GetLocationOpenCloseTimes(dtStartTime, dtEndTime, nLocationID);

		long nStartHour = dtStartTime.GetHour();
		long nEndHour = dtEndTime.GetHour();
		//(e.lally 2008-04-01) PLID 29499 - We must account for Office Hours closing at a time not on the hour
			//e.g. 8:00am-5:30pm, we need the extra hour block to cover from 5-6
		if(dtEndTime.GetMinute() > 0 || dtEndTime.GetSecond() > 0){
			if(nEndHour >= 0 && nEndHour <= 23){
				nEndHour++;
			}
		}

		//now, are any of today's non-event appointments outside our office hours?
		COleDateTime dtStartCheck, dtEndCheck;
		COleDateTime dtNow = COleDateTime::GetCurrentTime() + m_dtOffset;
		if (dtStartTime.GetStatus() == COleDateTime::valid) {
			dtStartCheck.SetDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), dtStartTime.GetHour(), dtStartTime.GetMinute(), dtStartTime.GetSecond());
		} else {
			// use the absolute minimum time
			dtStartCheck.SetDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 0, 0, 0);
			nStartHour = 0;
		}
		if (dtEndTime.GetStatus() == COleDateTime::valid) {
			dtEndCheck.SetDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), dtEndTime.GetHour(), dtEndTime.GetMinute(), dtEndTime.GetSecond());
		} else {
			// use the absolute maximum time
			dtStartCheck.SetDateTime(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay(), 23, 59, 59);
			nEndHour = 23;
		}
		
		//find any appt. before our open time or after our close time
		// (j.jones 2007-02-23 09:30) - PLID 24882 - ensure we check for appointments
		//that start at the close time as well
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized (well not really)
		// (a.walling 2013-07-11 15:34) - PLID 57526 - Filter on StartTime range to avoid table scans (2 or 3 in this case!)
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
		_RecordsetPtr rsApptCheck = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Min(StartTime) AS MinTime, Max(StartTime) AS MaxTime FROM AppointmentsT "
			//for today's date
			"WHERE AppointmentsT.Status <> 4 "
			"AND AppointmentsT.StartTime >= dbo.AsDateNoTime(GetDate()) AND AppointmentsT.StartTime < DATEADD(d, 1, dbo.AsDateNoTime(GetDate())) "
			//before or after our office hours
			// (a.walling 2007-07-02 15:39) - PLID 26169 - Just get the min and max for the entire day. We'll compare it with the office hours
			// in the code below. This appeared to make no significant performance change
			//and filter out events
			"AND NOT (StartTime = EndTime AND DatePart(hh, StartTime) = 0)");
		if(!rsApptCheck->eof) {
			//will always return at least the min and max, which may be null

			_variant_t varMin = rsApptCheck->Fields->Item["MinTime"]->Value;
			if(varMin.vt == VT_DATE) {
				COleDateTime dtNewStartTime = VarDateTime(varMin);
				//remember, we queried max and min, this could be the max
				if(dtNewStartTime < dtStartCheck) {
					dtStartCheck = dtNewStartTime;
					dtStartTime = dtNewStartTime;
					nStartHour = dtNewStartTime.GetHour();
				}
			} else if (varMin.vt == VT_NULL && dtStartTime.GetStatus() != COleDateTime::valid) {
				// (a.walling 2007-07-02 15:38) - PLID 26169
				// if we have no min and max AND the locations are closed this day, flag the start hour as -1
				nStartHour = -1;
			}

			_variant_t varMax = rsApptCheck->Fields->Item["MaxTime"]->Value;
			if(varMax.vt == VT_DATE) {
				COleDateTime dtNewEndTime = VarDateTime(varMax);
				//remember, we queried max and min, this could be the min
				// (j.jones 2007-02-23 09:30) - PLID 24882 - ensure we check for appointments
				//that start at the close time as well
				if(dtNewEndTime >= dtEndCheck) {
					dtEndCheck = dtNewEndTime;
					dtEndTime = dtNewEndTime;
					nEndHour = dtNewEndTime.GetHour();
					//increase by one, because truly our "end hour" would just be the hour after this appt. starts
					//(doesn't matter when the appt. ends)
					nEndHour++;
				}
			} else if (varMax.vt == VT_NULL && dtEndTime.GetStatus() != COleDateTime::valid) {
				// (a.walling 2007-07-02 15:37) - PLID 26169
				// if we have no min and max AND the locations are closed this day, flag the end hour as -1
				nEndHour = -1;
			}
		}
		rsApptCheck->Close();

		nFirstHour = nStartHour;
		nLastHour = nEndHour;

	}NxCatchAll("Error in CRoomManagerDlg::CalculateTimeRange");
}

void CRoomManagerDlg::GetLocationOpenCloseTimes(COleDateTime &dtOpen, COleDateTime &dtClose, long nLocationID /*= -1*/)
{
	try {

		//If nLocationID == -1, look for the earliest open time and latest close time across all locations,
		//otherwise filter on location;

		COleDateTime dtToday = COleDateTime::GetCurrentTime() + m_dtOffset;
		long nDayOfWeek = dtToday.GetDayOfWeek();

		CString strDay;

		switch(nDayOfWeek) {
			case 1: //Sunday
				strDay = "Sunday";
				break;
			case 2:	//Monday
				strDay = "Monday";
				break;
			case 3:	//Tuesday
				strDay = "Tuesday";
				break;
			case 4:	//Wednesday
				strDay = "Wednesday";
				break;
			case 5:	//Thursday
				strDay = "Thursday";
				break;
			case 6:	//Friday
				strDay = "Friday";
				break;
			case 7:	//Saturday
				strDay = "Saturday";
				break;
		}

		//it should be impossible not to have a strDay now
		if(strDay == "") {
			ASSERT(FALSE);
			return;
		}

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		_RecordsetPtr rs;
		CString strSql;
		if(nLocationID == -1) {
			//get min/max times for all locations
			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
			rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Min({CONST_STRING}Open) AS TimeOpen, Max({CONST_STRING}Close) AS TimeClose FROM LocationsT", strDay, strDay);
		}
		else {
			//get times for the filtered location
			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
			rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT {CONST_STRING}Open AS TimeOpen, {CONST_STRING}Close AS TimeClose FROM LocationsT WHERE ID = {INT}", strDay, strDay, nLocationID);
		}

		if(!rs->eof) {
			_variant_t varOpen = rs->Fields->Item["TimeOpen"]->Value;
			if(varOpen.vt == VT_DATE) {
				dtOpen = VarDateTime(varOpen);
			} else {
				// (a.walling 2007-05-29 16:36) - PLID 26169 - Return a valid time even if we can't retrieve any
				// actually, return an invalid datetime so we know there is none set. Then we can just use the earliest
				// and latest appointment.
				dtOpen.m_dt = 0;
				dtOpen.SetStatus(COleDateTime::null);
			}

			_variant_t varClosed = rs->Fields->Item["TimeClose"]->Value;
			if(varClosed.vt == VT_DATE) {
				dtClose = VarDateTime(varClosed);
			} else {
				// (a.walling 2007-05-29 16:36) - PLID 26169 - Return a valid time even if we can't retrieve any
				// actually, return an invalid datetime so we know there is none set. Then we can just use the earliest
				// and latest appointment.
				dtClose.m_dt = 0;
				dtClose.SetStatus(COleDateTime::null);
			}
		} else {
			// (a.walling 2007-05-29 16:36) - PLID 26169 - Return a valid time even if we can't retrieve any
			
			// actually, return an invalid datetime so we know there is none set. Then we can just use the earliest
			// and latest appointment.

			dtOpen.m_dt = 0;
			dtOpen.SetStatus(COleDateTime::null);

			dtClose.m_dt = 0;
			dtClose.SetStatus(COleDateTime::null);
		}
		rs->Close();

	}NxCatchAll("Error in CRoomManagerDlg::GetOpenCloseTimes");
}

BEGIN_EVENTSINK_MAP(CRoomManagerDlg, CNxModelessOwnedDialog)
    //{{AFX_EVENTSINK_MAP(CRoomManagerDlg)
	ON_EVENT(CRoomManagerDlg, IDC_TODAYS_PATIENTS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedTodaysPatientsList, VTS_I2)
	ON_EVENT(CRoomManagerDlg, IDC_TODAYS_PATIENTS_LIST, 6 /* RButtonDown */, OnRButtonDownTodaysPatientsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CRoomManagerDlg, IDC_ROOM_LIST, 6 /* RButtonDown */, OnRButtonDownRoomList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CRoomManagerDlg, IDC_TODAYS_PATIENTS_LIST, 19 /* LeftClick */, OnLeftClickTodaysPatientsList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CRoomManagerDlg, IDC_ROOM_LIST, 8 /* EditingStarting */, OnEditingStartingRoomList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CRoomManagerDlg, IDC_ROOM_LIST, 9 /* EditingFinishing */, OnEditingFinishingRoomList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CRoomManagerDlg, IDC_ROOM_LIST, 10 /* EditingFinished */, OnEditingFinishedRoomList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CRoomManagerDlg, IDC_ROOM_LIST, 18 /* RequeryFinished */, OnRequeryFinishedRoomList, VTS_I2)
	ON_EVENT(CRoomManagerDlg, IDC_ROOM_MGR_RESOURCE_COMBO, 16 /* SelChosen */, OnSelChosenRoomMgrResourceCombo, VTS_DISPATCH)
	ON_EVENT(CRoomManagerDlg, IDC_WAITING_AREA_LIST, 19 /* LeftClick */, OnLeftClickWaitingAreaList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CRoomManagerDlg, IDC_WAITING_AREA_LIST, 6 /* RButtonDown */, OnRButtonDownWaitingAreaList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CRoomManagerDlg, IDC_WAITING_AREA_LIST, 18 /* RequeryFinished */, OnRequeryFinishedWaitingAreaList, VTS_I2)
	ON_EVENT(CRoomManagerDlg, IDC_CHECKOUT_LIST, 6 /* RButtonDown */, OnRButtonDownCheckoutList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CRoomManagerDlg, IDC_CHECKOUT_LIST, 18 /* RequeryFinished */, OnRequeryFinishedCheckoutList, VTS_I2)
	ON_EVENT(CRoomManagerDlg, IDC_CHECKOUT_LIST, 19 /* LeftClick */, OnLeftClickCheckoutList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CRoomManagerDlg, IDC_WAITING_AREA_LIST, 22, CRoomManagerDlg::ColumnSizingFinishedWaitingAreaList, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CRoomManagerDlg, IDC_ROOM_LIST, 22, CRoomManagerDlg::ColumnSizingFinishedRoomList, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CRoomManagerDlg, IDC_CHECKOUT_LIST, 22, CRoomManagerDlg::ColumnSizingFinishedCheckoutList, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CRoomManagerDlg, IDC_ROOM_MGR_LOCATION_COMBO, 16, CRoomManagerDlg::OnSelChosenRoomMgrLocationCombo, VTS_DISPATCH)
	ON_EVENT(CRoomManagerDlg, IDC_ROOM_MGR_WAITINGROOM_COMBO, 16, CRoomManagerDlg::OnSelChosenRoomMgrWaitingRoomCombo, VTS_DISPATCH)
	ON_EVENT(CRoomManagerDlg, IDC_ROOM_LIST, 19, CRoomManagerDlg::OnLeftClickRoomList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP

END_EVENTSINK_MAP()

void CRoomManagerDlg::OnRequeryFinishedTodaysPatientsList(short nFlags) 
{
	try {

		KillTimer(m_nTimerHourExpansion);

		//loop through the list and:
		//	- color any childless parents gray,
		//	- remove hyperlinks from all parents		
		//	- color children by status

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPatientList->GetFirstRow();
		while(pRow) {

			//remove the patient color if there is one
			pRow->PutCellForeColor(rmplcPatientName, dlColorNotSet);

			//remove the link style from the parent row only
			pRow->PutCellLinkStyle(rmplcPatientName, NXDATALIST2Lib::dlLinkStyleFalse);

			if(pRow->GetFirstChildRow() == NULL) {
				//no children, color gray		
				pRow->PutForeColor(m_clrFolderEmpty);
			}
			else {

				//color children by status and show state
				NXDATALIST2Lib::IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();
				while(pChildRow) {

					ColorApptRow(pChildRow);
					
					//(a.wilson 2011-10-27) PLID 45963 - get the patients image path from their documents
					long nPatientID = VarLong(pChildRow->GetValue(rmplcPatientID), 0);
					_variant_t varPictureName = pChildRow->GetValue(rmplcPatientPicturePath);

					if (nPatientID != 0 && varPictureName.vt == VT_BSTR) {
						CString strPictureName = VarString(varPictureName, "");
						pChildRow->PutValue(rmplcPatientPicturePath, _bstr_t(GetPatientDocumentPath(nPatientID) ^ strPictureName));
					} else if (varPictureName.vt == VT_EMPTY || varPictureName.vt == VT_NULL) {
						pChildRow->PutValue(rmplcPatientPicturePath, _bstr_t(""));
					}

					pChildRow = pChildRow->GetNextRow();
				}
			}

			pRow = pRow->GetNextRow();
		}

		//now call our function to expand the current hour
		ExpandCurrentHour(TRUE, TRUE);

		//now start the timer up again for the next time it needs to fire
		ResetHourTimer();

	} catch(CNxPersonDocumentException *pException) {
		// (d.thompson 2013-07-01) - PLID 13764 - Specially handle exceptions regarding the person documents
		CPatientDocumentStorageWarningDlg dlg(pException->m_strPath);
		//No longer need the exception, clean it up
		pException->Delete();

		//Inform the user
		dlg.DoModal();
	}NxCatchAll("Error in CRoomManagerDlg::OnRequeryFinishedTodaysPatientsList");
}

void CRoomManagerDlg::OnBtnRoomEditor() 
{
	// (j.jones 2007-03-05 12:37) - PLID 25059 - added a permission check
	if(!CheckCurrentUserPermissions(bioAdminScheduler,sptView|sptRead))
		return;

	CRoomSetupDlg dlg(this);
	dlg.DoModal();

	//we will requery if a network message tells us to
}

void CRoomManagerDlg::OnBtnEditRoomStatus() 
{
	// (j.jones 2007-03-05 12:37) - PLID 25059 - added a permission check
	if(!CheckCurrentUserPermissions(bioAdminScheduler,sptView|sptRead))
		return;

	CRoomStatusSetupDlg dlg(this);
	dlg.DoModal();

	//we will requery if a network message tells us to
}

void CRoomManagerDlg::OnRButtonDownTodaysPatientsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL)
			return;

		m_pPatientList->PutCurSel(pRow);

		//if a parent row, return
		if(pRow->GetValue(rmplcPatientID).vt == VT_NULL) {
			return;
		}

		CPoint pt;
		GetCursorPos(&pt);
		GenerateAppointmentMenu(VarLong(pRow->GetValue(rmplcPatientID),-1), VarLong(pRow->GetValue(rmplcApptID),-1), pt);

	}NxCatchAll("Error in CRoomManagerDlg::OnRButtonDownTodaysPatientsList");
}

void CRoomManagerDlg::OnLeftClickTodaysPatientsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL)
			return;

		m_pPatientList->PutCurSel(pRow);

		//if a parent row, return
		if(pRow->GetValue(rmplcPatientID).vt == VT_NULL) {
			return;
		}

		CPoint pt;
		GetCursorPos(&pt);
		GenerateAppointmentMenu(VarLong(pRow->GetValue(rmplcPatientID),-1), VarLong(pRow->GetValue(rmplcApptID),-1), pt);

	}NxCatchAll("Error in CRoomManagerDlg::OnLeftClickTodaysPatientsList");	
}

// (c.haag 2010-11-02 12:10) - PLID 39199 - Add custom statuses with the Waiting Area bit set to a menu of statuses.
// We also take in a state to exclude in case the appointment is already assigned to it
// (j.jones 2010-12-09 10:25) - PLID 41763 - renamed this function, we now support all custom statuses
void CRoomManagerDlg::AddShowStateStatusesToMenu(CMenu& mnu, long nShowStateToExclude)
{
	for(int i=0; i<m_aryShowStates.GetSize();i++) {
		long nID = (long)m_aryShowStates.GetAt(i);
		//do not add the 'no show' state in this function
		if(nID != nShowStateToExclude && nID != assNoShow) {
			CString strName;
			if(m_mapShowStateNames.Lookup(nID, strName)) {
				mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, miMarkAsShowStateStatus - nID, "Mark as " + strName);
			}
			else {
				//should be impossible, don't throw an exception, but investigate
				//if we manage to hit this while debugging
				ASSERT(FALSE);
			}
		}
	}
}

// (j.jones 2007-02-07 10:57) - PLID 24595 - converted GeneratePatientMenu
// to GenerateAppointmentMenu, to reuse the code in the Waiting Area list
// (j.jones 2008-05-29 14:43) - PLID 27797 - added checkout option, and optional RoomAppointmentID
// (j.jones 2010-12-02 16:36) - PLID 38597 - renamed the checkout option to bFromCheckoutList, and added bFromWaitingArea and nRoomID
void CRoomManagerDlg::GenerateAppointmentMenu(long nPatientID, long nApptID, CPoint &pt, BOOL bFromCheckoutList /*= FALSE*/, long nRoomAppointmentID /*= -1*/, BOOL bFromWaitingArea /*= FALSE*/, long nRoomID /*= -1*/)
{
	if(nPatientID == -1 || nApptID == -1) {
		//should be impossible
		ASSERT(FALSE);
		return;
	}

	CMenu mnu;
	mnu.CreatePopupMenu();
	// (c.haag 2011-06-17) - PLID 36477 - We now handle some of the menu options in here (code refactoring). Note: Although
	// aryCopayInsuredPartyIDs and nPrimaryInsuredPartyID are not used in this function, they are referenced within CApptPopupMenuManager.
	// If you study how other functions use CApptPopupMenuManager, you will see that they themselves deal with insurance and copays.
	// In this function, they are dealt with in the call to HandlePopupMenuResult(). At the time of this writing, this is the only place that calls
	// HandlePopupMenuResult().
	CArray<long, long> aryCopayInsuredPartyIDs;
	long nPrimaryInsuredPartyID = -1;
	CApptPopupMenuManager apmm(mnu, nPrimaryInsuredPartyID, aryCopayInsuredPartyIDs, this, nApptID, nPatientID);

	// (j.jones 2008-05-29 14:50) - PLID 27797 - if requested, show the checkout option first
	// (j.jones 2010-12-02 16:37) - PLID 38597 - show this if in the checkout list or the waiting room list,
	// only supported in either list if the patient is in a room
	if((bFromWaitingArea || bFromCheckoutList) && nRoomAppointmentID != -1) {
		
		mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, miCheckoutPatient, "Check Out Patient");
		//if(!bFromWaitingArea) {
		//	mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, miWaitingRoomPatient , "Waiting Area"); // (s.dhole 2010-11-16 15:31) - PLID  39200 Add menu option for Waiting Room
		//}
		mnu.AppendMenu(MF_SEPARATOR, 0, "");
	}

	//create the pop-out room selections
	CMenu pSubMenu1;
	pSubMenu1.CreatePopupMenu();

	long nAddedItems = 0;

	// (j.jones 2009-08-04 08:55) - PLID 24600 - added location filter
	long nLocationID = -1;
	{
		IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
		if(pRow == NULL) {
			nLocationID = GetCurrentLocationID();
			m_pLocationCombo->SetSelByColumn(lccID, nLocationID);
		}
		else {
			nLocationID = VarLong(pRow->GetValue(lccID));
		}
	}

	//find all rooms, and their in-use status
	// (j.jones 2009-08-04 09:36) - PLID 24600 - filter by location
	// (j.jones 2010-12-01 09:59) - PLID 38597 - do not flag waiting rooms as In Use (and sort them to show up last)
	// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT RoomsT.ID, RoomsT.Name, "
		"CASE WHEN RoomsT.WaitingRoom = 0 AND RoomsT.ID IN (SELECT RoomID FROM RoomAppointmentsT WHERE StatusID NOT IN (0, -1)) THEN 1 ELSE 0 END AS InUse "
		"FROM RoomsT "
		"WHERE Inactive = 0 "
		"AND LocationID = {INT} "
		"ORDER BY WaitingRoom ASC, Name ASC", nLocationID);
	while(!rs->eof) {

		long nCurRoomID = AdoFldLong(rs, "ID");
		CString strRoomName = AdoFldString(rs, "Name","");
		long nInUse = AdoFldLong(rs, "InUse", 0);
		
		//TES 8/29/2008 - PLID 26534 - They're now allowed to assign patients to rooms that already have somebody in them.
		if(nInUse == 1) {
			strRoomName += " <In Use>";
		}

		// (j.jones 2010-12-02 16:39) - PLID 38597 - if in a waiting room, gray out that room
		BOOL bGrayed = (bFromWaitingArea && nRoomID != -1 && nRoomID == nCurRoomID);

		pSubMenu1.InsertMenu(nAddedItems++, MF_BYPOSITION | (bGrayed ? MF_GRAYED : 0), nCurRoomID, strRoomName);

		rs->MoveNext();
	}
	rs->Close();

	// (j.jones 2008-05-29 16:42) - PLID 26855 - we allow switching rooms, so instead of disabling the ability
	// when a patient is in a room, just change the label
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	// (j.jones 2010-12-01 09:56) - PLID 38597 - only set this flag to true if they are in an actual room, not a waiting room
	// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
	BOOL bPatientIsInARoom = ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT RoomAppointmentsT.ID "
		"FROM RoomAppointmentsT "
		"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
		"WHERE RoomAppointmentsT.AppointmentID = {INT} "
		"AND RoomAppointmentsT.StatusID NOT IN (0, -1) "
		"AND RoomsT.WaitingRoom = 0", nApptID);

	mnu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT)pSubMenu1.m_hMenu, bPatientIsInARoom ? "Switch Rooms..." : "Assign To Room...");
	mnu.AppendMenu(MF_SEPARATOR, 0, "");
	mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, miGoToPatient, "Go To Patient");
	mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, miGoToAppointment, "Go To Appointment");

	// (c.haag 2011-06-24) - PLID 44319 - We need the appointment type category, too
	long nCurShowState = -1;
	long nConfirmed = 0;
	long nAptTypeCategory = -1;
	{
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		// (c.haag 2011-06-17) - PLID 36477 - Also grab Confirmed
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), 
			"SELECT ShowState, Confirmed, AptTypeT.Category AS AptTypeCategory "
			"FROM AppointmentsT "
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE AppointmentsT.ID={INT}", nApptID);
		if (!rs->eof) {
			nCurShowState = AdoFldLong(rs, "ShowState");
			nConfirmed = AdoFldLong(rs, "Confirmed",0);
			nAptTypeCategory = AdoFldByte(rs, "AptTypeCategory", -1);
		}
	}

	// (d.moore 2007-07-11 14:19) - PLID 24668 - Add options for 'Mark In' and 'Mark Out'.
	if (HasPermissionForResource(nApptID, sptWrite, sptWriteWithPass)) {
		// (j.jones 2010-12-09 10:58) - PLID 41763 - add the no show status, if it is no show,
		// only add as unmark no show, just like the scheduler does

		mnu.AppendMenu(MF_SEPARATOR);

		if(nCurShowState == assNoShow) {
			mnu.AppendMenu(MF_ENABLED, miUnmarkNoShow, GetStringOfResource(IDS_UNMARK_NO_SHOW));
		}
		else {
			mnu.AppendMenu(MF_ENABLED, miMarkAsNoShow, GetStringOfResource(IDS_MARK_NO_SHOW));

			// (c.haag 2010-11-02 12:10) - PLID 39199 - Now add custom statuses with the Waiting Area bit set
			// (j.jones 2010-12-09 10:25) - PLID 41763 - renamed this function, it now adds all possible statuses
			// other than the current appointment status
			AddShowStateStatusesToMenu(mnu, nCurShowState);
		}

		// (c.haag 2011-06-17) - PLID 36477 - Add confirmed
		mnu.AppendMenu(MF_SEPARATOR);
		apmm.FillConfirmedOptions(nConfirmed);
	}

	// (c.haag 2011-06-17) - PLID 36477 - Populate Superbill menu items
	apmm.FillSuperbillOptions();

	// (c.haag 2011-06-23) - PLID 44287 - Populating inventory options is done in a utility function now
	BOOL bAddedSeparator = apmm.FillInventoryOptions();

	// (j.jones 2010-10-13 16:47) - PLID 39778 - add the ability to bill today's EMN

	long nTodaysEMNIDToBill = -1;
	long nBillToInsuredPartyID = -2;	//-2 means we don't have a default, -1 is patient
	
	// (j.jones 2011-12-19 09:44) - PLID 46289 - map our insured party IDs to resp type IDs
	CMap<long, long, long, long> mapInsPartyIDToRespTypeID;

	if(!m_bEMRExpired) {	//previously cached value for the EMR license	
		mnu.AppendMenu(MF_SEPARATOR);
		nTodaysEMNIDToBill = GetTodaysBillableEMNForPatient(nPatientID);
		//if -1, or they can't create bills, disable the menu item indicating the option is not valid
		if(nTodaysEMNIDToBill == -1 || !(GetCurrentUserPermissions(bioBill) & sptWrite|sptWriteWithPass)) {
			mnu.AppendMenu(MF_BYPOSITION|MF_GRAYED, miBillTodaysEMN_Default, "&Bill Today's EMN");
		}
		else {

			long nBillEMNTo = GetRemotePropertyInt("BillEMNTo", 0, 0, GetCurrentUserName(), true);
			//1 - patient responsibility
			//2 - primary responsibility if it exists
			//0 - pop-out a menu of options of available responsibilities

			// (j.jones 2011-12-16 15:39) - PLID 46289 - we now pop out the menu if they
			// select primary, but have more than one primary (medical and vision)
			BOOL bPopOutMenu = FALSE;
	
			if(nBillEMNTo == 1) {
				//patient responsibility
				nBillToInsuredPartyID = -1;
				mnu.AppendMenu(MF_BYPOSITION|MF_ENABLED, miBillTodaysEMN_Default, "&Bill Today's EMN");
			}
			else if(nBillEMNTo == 2) {
				//primary responsibility if it exists
				// (j.jones 2011-12-16 16:43) - PLID 46289 - find all Primary resps, there could be Primary Medical or Vision, or both
				// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
				_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuredPartyT.PersonID FROM InsuredPartyT "
					"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"WHERE PatientID = {INT} AND RespTypeT.CategoryPlacement = 1", nPatientID);
				if(!rs->eof) {
					// (j.jones 2011-12-16 16:41) - PLID 46289 - if multiple records,
					// we will pop out the menu
					if(rs->GetRecordCount() > 1) {
						nBillToInsuredPartyID = -1;
						bPopOutMenu = TRUE;
					}
					else {
						nBillToInsuredPartyID = AdoFldLong(rs, "PersonID",-1);
					}
				}
				else {
					nBillToInsuredPartyID = -1;
				}
				rs->Close();

				if(!bPopOutMenu) {
					mnu.AppendMenu(MF_BYPOSITION|MF_ENABLED, miBillTodaysEMN_Default, "&Bill Today's EMN");
				}
			}
			else {
				bPopOutMenu = TRUE;
			}

			if(bPopOutMenu) {	
				//pop-out a menu of options of available responsibilities
				
				//if there is no insurance, use patient

				// (j.jones 2011-12-16 16:43) - PLID 46289 - find all active resps
				// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
				_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuredPartyT.PersonID, InsuranceCoT.Name, RespTypeT.TypeName, RespTypeT.ID AS RespTypeID "
					"FROM InsuredPartyT "
					"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"WHERE PatientID = {INT} AND RespTypeT.Priority <> -1 "
					"ORDER BY Coalesce(RespTypeT.CategoryPlacement,1000), RespTypeT.CategoryType, RespTypeT.Priority", nPatientID);
				if(!rs->eof) {

					CMenu pSubMenu;
					pSubMenu.CreatePopupMenu();
					long nIndex = 0;
					//add a line for patient
					pSubMenu.InsertMenu(nIndex++, MF_BYPOSITION, miBillTodaysEMN_Patient, "For Patient Responsibility");

					//add a line for each responsibility
					while(!rs->eof) {

						long nInsPartyID = AdoFldLong(rs, "PersonID",-1);
						long nRespTypeID = AdoFldLong(rs, "RespTypeID",-1);
						CString strInsCoName = AdoFldString(rs, "Name","");
						CString strRespTypeName = AdoFldString(rs, "TypeName","");
						CString strLabel;
						strLabel.Format("For %s (%s)", strInsCoName, strRespTypeName);

						// (j.jones 2011-12-19 09:44) - PLID 46289 - map our insured party ID to resp type ID
						mapInsPartyIDToRespTypeID.SetAt(nRespTypeID, nInsPartyID);
						long nMenuID = miBillTodaysEMN_RespTypeIDPivotMin - nRespTypeID;

						//no office is actually going to have enough resp type IDs to exceed
						//miBillTodaysEMN_RespTypeIDPivotMax, but catch it nonetheless
						if(nMenuID >= miBillTodaysEMN_RespTypeIDPivotMax) {
							pSubMenu.InsertMenu(nIndex++, MF_BYPOSITION, nMenuID, strLabel);
						}
						rs->MoveNext();
					}

					mnu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT)pSubMenu.m_hMenu, "&Bill Today's EMN...");
				}
				else {
					nBillToInsuredPartyID = -1;
					mnu.AppendMenu(MF_BYPOSITION|MF_ENABLED, miBillTodaysEMN_Default, "&Bill Today's EMN");
				}
				rs->Close();
			}
		}
	}
	// (c.haag 2011-06-24) - PLID 44317 - Populating billing options is done in a utility function now. Pass in bAddedSeparator
	// so a separator is not placed in-between Bill Today's EMN and Create Bill.
	bAddedSeparator = apmm.FillBillingOptions(bAddedSeparator);

	// (c.haag 2011-06-24) - PLID 44319 - Populating e-eligibility options is done in a utility function.
	bAddedSeparator = apmm.FillEEligibilityOptions(bAddedSeparator);

	// (c.haag 2011-06-24) - PLID 44319 - Populating case history options is done in a utility function.
	bAddedSeparator = apmm.FillCaseHistoryOptions(bAddedSeparator, nAptTypeCategory);

	CArray<DeviceLaunchUtils::DevicePlugin *, DeviceLaunchUtils::DevicePlugin*> aryLoadedPlugins;
	long nBogus;
	DeviceLaunchUtils::GenerateDeviceMenu(aryLoadedPlugins, &mnu, nBogus, FALSE, -1);

	// Pop up the menu
	int nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
	// (c.haag 2011-06-17) - PLID 36477 - Try to handle unhandled results here first
	if (!apmm.HandlePopupMenuResult(nCmdId)) 
	{
		// (c.haag 2010-11-02 12:10) - PLID 39199 - Special handling for custom statuses
		// (j.jones 2010-12-09 10:37) - PLID 41764 - this now handles all statuses
		if (nCmdId <= miMarkAsShowStateStatus) {
			long nSelectedStatusID = -(nCmdId - (int)miMarkAsShowStateStatus);
			CString strName;
			if (!m_mapShowStateNames.Lookup(nSelectedStatusID, strName)) {
				ThrowNxException("Could not find status %d!", nSelectedStatusID);
			}		
			switch(nSelectedStatusID)
			{
				case assIn:
					AppointmentMarkIn(nApptID);
					break;

				case assPending:
					AppointmentMarkPending(nApptID);
					break;

				case assOut:
					AppointmentMarkOut(nApptID);
					break;

				case assReceived:
					AppointmentMarkReceived(nApptID);
					break;

				default:
					AppointmentMarkUserDefined(nApptID, strName);
					break;
			}
		}
		// (j.jones 2011-12-19 09:44) - PLID 46289 - is the index a resp type ID?
		else if(nCmdId <= miBillTodaysEMN_RespTypeIDPivotMin && nCmdId >= miBillTodaysEMN_RespTypeIDPivotMax) {

			long nRespTypeID = miBillTodaysEMN_RespTypeIDPivotMin - nCmdId;
			if(!mapInsPartyIDToRespTypeID.Lookup(nRespTypeID, nBillToInsuredPartyID)) {
				//should be impossible
				ThrowNxException("Could not find an insured party ID for RespTypeID %li!", nRespTypeID);
			}

			if(nTodaysEMNIDToBill != -1 && nBillToInsuredPartyID != -2) {
				BillEMNForPatient(nPatientID, nTodaysEMNIDToBill, nBillToInsuredPartyID);
			}
		}
		else switch(nCmdId) {

			case 0: // no selection
				break;

			case miGoToPatient:

				GoToPatient(nPatientID);
				break;

			case miGoToAppointment:

				GoToAppointment(nApptID);			
				break;

			// (j.jones 2010-12-09 10:57) - PLID 41763 - supported no show
			case miMarkAsNoShow:
				AppointmentMarkNoShow(nApptID);
				break;

			case miUnmarkNoShow:
				//unmarking no show always marks as pending
				AppointmentMarkPending(nApptID);
				break;

			// (j.jones 2008-05-29 14:51) - PLID 27797 - added checkout patient option
			case miCheckoutPatient:			
				//force a checkout status
				ChangeRoomAppointmentStatus(nRoomAppointmentID, nApptID, -1);
				break;

			// (j.jones 2010-10-13 16:52) - PLID 39778 - added ability to bill today's EMN
			case miBillTodaysEMN_Default:
				if(nTodaysEMNIDToBill != -1 && nBillToInsuredPartyID != -2) {
					BillEMNForPatient(nPatientID, nTodaysEMNIDToBill, nBillToInsuredPartyID);
				}
				break;

			case miBillTodaysEMN_Patient:
				if(nTodaysEMNIDToBill != -1) {
					BillEMNForPatient(nPatientID, nTodaysEMNIDToBill, -1);
				}
				break;

			//case miWaitingRoomPatient:			// (s.dhole 2010-11-16 15:52) - PLID  39200 User select waiting area status
			//	//force a Waitting  status
			//	ChangeRoomAppointmentStatus(nRoomAppointmentID, nApptID, -3);
			//	break;
			default:

				// (d.lange 2010-11-03 12:30) - PLID 41211 - return TRUE if the user selected a send to device menu item
				// (j.gruber 2013-04-02 12:17) - PLID 56012 - consolidated functions
				BOOL bLaunchDevice = DeviceLaunchUtils::LaunchDevice(aryLoadedPlugins, nCmdId, nPatientID);

				if(!bLaunchDevice)
					//if not 0, and not our fixed values, it must be a room ID
					AssignToRoom(nApptID, nCmdId);
				break;
		}
	}
	// (d.lange 2010-11-04 09:37) - PLID 41211 - Destory all loaded device plugins, so we don't leak memory
	// (j.gruber 2013-04-02 12:18) - PLID 56012 - consolidated
	DeviceLaunchUtils::DestroyLoadedDevicePlugins(aryLoadedPlugins);

	mnu.DestroyMenu();
}

void CRoomManagerDlg::OnRButtonDownRoomList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL)
			return;

		m_pRoomList->PutCurSel(pRow);

		//if an open room, return
		if(pRow->GetValue(rmrlcRoomAppointmentID).vt == VT_NULL) {
			return;
		}
		
		CPoint pt;
		GetCursorPos(&pt);
		GenerateRoomMenu(lpRow, pt);
	}NxCatchAll("Error in CRoomManagerDlg::OnRButtonDownRoomList");	
}

void CRoomManagerDlg::GenerateRoomMenu(LPDISPATCH lpRow, CPoint &pt)
{
	IRowSettingsPtr pRow = lpRow;
	if(pRow == NULL)
		return;

	//if an open room, return
	if(pRow->GetValue(rmrlcRoomAppointmentID).vt == VT_NULL) {
		return;
	}

	//TES 8/29/2008 - PLID 26534 - Track which room they right-clicked on.
	long nCurrentRoomID = VarLong(pRow->GetValue(rmrlcRoomID),-1);
	enum {
		miGoToPatient = -100,
		miGoToAppointment = -200,
		// (j.jones 2010-10-13 16:46) - PLID 39778 - added ability to bill today's EMN
		miBillTodaysEMN_Default = -300,
		miBillTodaysEMN_Patient = -301,
		// (j.jones 2011-12-16 17:23) - PLID 46289 - supported all active resp type IDs, dynamically
		miBillTodaysEMN_RespTypeIDPivotMin = -302,
		miBillTodaysEMN_RespTypeIDPivotMax = -352,
		// (j.jones 2011-12-16 17:23) - the next available ID is -353, the RespType code enforces it
		// will not use an ID past -352
	};

	CMenu mnu;
	mnu.CreatePopupMenu();

	// (j.jones 2008-05-29 16:51) - PLID 26855 - because we need to return unique IDs
	// for status and room IDs, we need a clever way to make them unique, which we will do
	// by incrementing room IDs to be greater than the greatest status ID
	// (z.manning 2009-05-07) - PLID 34195 - Very clever indeed, if only you had acutally
	// done that. Instead this variable used to be named nCountStatuses and was in fact
	// a count of the number of active status. This meant that if you, for example, had
	// 3 active statuses with IDs of 1,2, and 4 then when you selected the status with ID
	// 4 it would try to change the room instead of the status! I changed this to actually
	// use the max status ID as the original comment describes.
	long nMaxStatusID = 0;

	//first create the pop-out status selections
	{
		CMenu pSubMenu1;
		pSubMenu1.CreatePopupMenu();

		long nAddedItems = 0;

		//find all statuses
		// (j.jones 2008-05-29 09:52) - PLID 27797 - order the hard coded options first
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
		_RecordsetPtr rs = CreateRecordset(GetRemoteDataSnapshot(), "SELECT RoomStatusT.ID, RoomStatusT.Name "
			"FROM RoomStatusT "
			"WHERE Inactive = 0 "
			"ORDER BY (CASE WHEN ID IN (-1, 0, 1) THEN ID ELSE -2 END) DESC, Name ASC");
		while(!rs->eof) {

			long nRoomStatusID = AdoFldLong(rs, "ID");
			CString strRoomStatusName = AdoFldString(rs, "Name","");

			// (j.jones 2008-05-29 11:09) - PLID 27797 - convert 0 to -2, for menu compatibility
			if(nRoomStatusID == 0) {
				nRoomStatusID = -2;
			}
			
			pSubMenu1.InsertMenu(nAddedItems++, MF_BYPOSITION, nRoomStatusID, strRoomStatusName);

			// (z.manning 2009-05-07) - PLID 34195 - This should be the max ID instead of a count
			// as it used to be.
			if(nRoomStatusID > nMaxStatusID) {
				nMaxStatusID = nRoomStatusID;
			}

			rs->MoveNext();
		}
		rs->Close();

		mnu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT)pSubMenu1.m_hMenu, "Change Status...");		
	}

	// (j.jones 2008-05-29 16:43) - PLID 26855 - now we allow switching rooms

	//create the pop-out room selections
	{
		CMenu pSubMenu2;
		pSubMenu2.CreatePopupMenu();

		long nAddedItems = 0;

		// (j.jones 2009-08-04 08:55) - PLID 24600 - added location filter
		long nLocationID = -1;
		{
			IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
			if(pRow == NULL) {
				nLocationID = GetCurrentLocationID();
				m_pLocationCombo->SetSelByColumn(lccID, nLocationID);
			}
			else {
				nLocationID = VarLong(pRow->GetValue(lccID));
			}
		}

		//find all rooms, and their in-use status
		// (j.jones 2009-08-04 09:36) - PLID 24600 - filter by location
		// (j.jones 2010-12-01 09:59) - PLID 38597 - do not flag waiting rooms as In Use (and sort them to show up last)
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT RoomsT.ID, RoomsT.Name, "
			"CASE WHEN RoomsT.WaitingRoom = 0 AND RoomsT.ID IN (SELECT RoomID FROM RoomAppointmentsT WHERE StatusID NOT IN (0, -1)) THEN 1 ELSE 0 END AS InUse "
			"FROM RoomsT "
			"WHERE Inactive = 0 "
			"AND LocationID = {INT} "
			"ORDER BY WaitingRoom ASC, Name ASC", nLocationID);
		while(!rs->eof) {

			long nRoomID = AdoFldLong(rs, "ID");
			CString strRoomName = AdoFldString(rs, "Name","");
			long nInUse = AdoFldLong(rs, "InUse", 0);

			//now the index needs to be the nCountStatuses + nRoomID
			//TES 8/29/2008 - PLID 26534 - They're now allowed to switch patients into rooms that already have a patient in them.
			// We'll only gray out the menu item for the room they're already right-clicking on.
			if(nInUse == 1) {
				strRoomName += " <In Use>";
			}
			pSubMenu2.InsertMenu(nAddedItems++, MF_BYPOSITION|(nRoomID==nCurrentRoomID?MF_GRAYED:0), nMaxStatusID + nRoomID, strRoomName);

			rs->MoveNext();
		}
		rs->Close();

		mnu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT)pSubMenu2.m_hMenu, "Switch Rooms...");
	}

	mnu.AppendMenu(MF_SEPARATOR, 0, "");
	mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, miGoToPatient, "Go To Patient");
	mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, miGoToAppointment, "Go To Appointment");

	long nPatientID = VarLong(pRow->GetValue(rmrlcPatientID), -1);

	// (j.jones 2010-10-13 16:47) - PLID 39778 - add the ability to bill today's EMN	
	long nTodaysEMNIDToBill = -1;
	long nBillToInsuredPartyID = -2;	//-2 means we don't have a default, -1 is patient
	
	// (j.jones 2011-12-19 09:44) - PLID 46289 - map our insured party IDs to resp type IDs
	CMap<long, long, long, long> mapInsPartyIDToRespTypeID;

	if(!m_bEMRExpired) {	//previously cached value for the EMR license	
		nTodaysEMNIDToBill = GetTodaysBillableEMNForPatient(nPatientID);
		//if -1, or they can't create bills, disable the menu item indicating the option is not valid
		if(nTodaysEMNIDToBill == -1 || !(GetCurrentUserPermissions(bioBill) & sptWrite|sptWriteWithPass)) {
			mnu.AppendMenu(MF_BYPOSITION|MF_GRAYED, miBillTodaysEMN_Default, "&Bill Today's EMN");
		}
		else {

			long nBillEMNTo = GetRemotePropertyInt("BillEMNTo", 0, 0, GetCurrentUserName(), true);
			//1 - patient responsibility
			//2 - primary responsibility if it exists
			//0 - pop-out a menu of options of available responsibilities

			// (j.jones 2011-12-16 15:39) - PLID 46289 - we now pop out the menu if they
			// select primary, but have more than one primary (medical and vision)
			BOOL bPopOutMenu = FALSE;
	
			if(nBillEMNTo == 1) {
				//patient responsibility
				nBillToInsuredPartyID = -1;
				mnu.AppendMenu(MF_BYPOSITION|MF_ENABLED, miBillTodaysEMN_Default, "&Bill Today's EMN");
			}
			else if(nBillEMNTo == 2) {
				//primary responsibility if it exists
				// (j.jones 2011-12-16 16:43) - PLID 46289 - find all Primary resps, there could be Primary Medical or Vision, or both
				// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
				_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuredPartyT.PersonID FROM InsuredPartyT "
					"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"WHERE PatientID = {INT} AND RespTypeT.CategoryPlacement = 1", nPatientID);
				if(!rs->eof) {
					// (j.jones 2011-12-16 16:41) - PLID 46289 - if multiple records,
					// we will pop out the menu
					if(rs->GetRecordCount() > 1) {
						nBillToInsuredPartyID = -1;
						bPopOutMenu = TRUE;
					}
					else {
						nBillToInsuredPartyID = AdoFldLong(rs, "PersonID",-1);
					}
				}
				else {
					nBillToInsuredPartyID = -1;
				}
				rs->Close();
				
				if(!bPopOutMenu) {
					mnu.AppendMenu(MF_BYPOSITION|MF_ENABLED, miBillTodaysEMN_Default, "&Bill Today's EMN");
				}
			}
			else {
				bPopOutMenu = TRUE;
			}

			if(bPopOutMenu) {
				//pop-out a menu of options of available responsibilities
				
				//if there is no insurance, use patient

				// (j.jones 2011-12-16 16:43) - PLID 46289 - find all active resps
				// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
				_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuredPartyT.PersonID, InsuranceCoT.Name, RespTypeT.TypeName, RespTypeT.ID AS RespTypeID "
					"FROM InsuredPartyT "
					"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"WHERE PatientID = {INT} AND RespTypeT.Priority <> -1 "
					"ORDER BY Coalesce(RespTypeT.CategoryPlacement,1000), RespTypeT.CategoryType, RespTypeT.Priority", nPatientID);
				if(!rs->eof) {

					CMenu pSubMenu;
					pSubMenu.CreatePopupMenu();
					long nIndex = 0;
					//add a line for patient
					pSubMenu.InsertMenu(nIndex++, MF_BYPOSITION, miBillTodaysEMN_Patient, "For Patient Responsibility");

					//add a line for each responsibility
					while(!rs->eof) {

						long nInsPartyID = AdoFldLong(rs, "PersonID",-1);
						long nRespTypeID = AdoFldLong(rs, "RespTypeID",-1);
						CString strInsCoName = AdoFldString(rs, "Name","");
						CString strRespTypeName = AdoFldString(rs, "TypeName","");
						CString strLabel;
						strLabel.Format("For %s (%s)", strInsCoName, strRespTypeName);

						// (j.jones 2011-12-19 09:44) - PLID 46289 - map our insured party ID to resp type ID
						mapInsPartyIDToRespTypeID.SetAt(nRespTypeID, nInsPartyID);
						long nMenuID = miBillTodaysEMN_RespTypeIDPivotMin - nRespTypeID;

						//no office is actually going to have enough resp type IDs to exceed
						//miBillTodaysEMN_RespTypeIDPivotMax, but catch it nonetheless
						if(nMenuID >= miBillTodaysEMN_RespTypeIDPivotMax) {
							pSubMenu.InsertMenu(nIndex++, MF_BYPOSITION, nMenuID, strLabel);
						}
						rs->MoveNext();
					}

					mnu.AppendMenu(MF_BYPOSITION|MF_POPUP, (UINT)pSubMenu.m_hMenu, "&Bill Today's EMN...");
				}
				else {
					nBillToInsuredPartyID = -1;
					mnu.AppendMenu(MF_BYPOSITION|MF_ENABLED, miBillTodaysEMN_Default, "&Bill Today's EMN");
				}
				rs->Close();				
			}
		}
	}

	// (d.lange 2010-11-04 08:54) - PLID 41211 - add menu items for every device plugin thats enabled and has the ability to send to devic
	CArray<DeviceLaunchUtils::DevicePlugin*, DeviceLaunchUtils::DevicePlugin*> aryLoadedPlugins;
	long nBlank = -1;
	DeviceLaunchUtils::GenerateDeviceMenu(aryLoadedPlugins, &mnu, nBlank, FALSE, -1);
	
	// Pop up the menu
	int nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, pt.x, pt.y, this, NULL);
	// (j.jones 2011-12-19 09:44) - PLID 46289 - is the index a resp type ID?
	if(nCmdId <= miBillTodaysEMN_RespTypeIDPivotMin && nCmdId >= miBillTodaysEMN_RespTypeIDPivotMax) {

		long nRespTypeID = miBillTodaysEMN_RespTypeIDPivotMin - nCmdId;
		if(!mapInsPartyIDToRespTypeID.Lookup(nRespTypeID, nBillToInsuredPartyID)) {
			//should be impossible
			ThrowNxException("Could not find an insured party ID for RespTypeID %li!", nRespTypeID);
		}

		if(nTodaysEMNIDToBill != -1 && nBillToInsuredPartyID != -2) {
			BillEMNForPatient(nPatientID, nTodaysEMNIDToBill, nBillToInsuredPartyID);
		}
	}
	else {

		switch(nCmdId) {

			case 0: // no selection
				break;

			case miGoToPatient:

				GoToPatient(nPatientID);
				break;

			case miGoToAppointment:

				GoToAppointment(VarLong(pRow->GetValue(rmrlcApptID),-1));			
				break;

			// (j.jones 2010-10-13 16:52) - PLID 39778 - added ability to bill today's EMN
			case miBillTodaysEMN_Default:
				if(nTodaysEMNIDToBill != -1 && nBillToInsuredPartyID != -2) {
					BillEMNForPatient(nPatientID, nTodaysEMNIDToBill, nBillToInsuredPartyID);
				}
				break;

			case miBillTodaysEMN_Patient:
				if(nTodaysEMNIDToBill != -1) {
					BillEMNForPatient(nPatientID, nTodaysEMNIDToBill, -1);
				}
				break;

			default: {
				// (d.lange 2010-11-04 08:55) - PLID 41211 - return TRUE if the user selected a send to device menu item
				// (j.gruber 2013-04-02 12:30) - PLID 56012 - Consolidate
				BOOL bLaunchDevice = DeviceLaunchUtils::LaunchDevice(aryLoadedPlugins, nCmdId, nPatientID);

				// (j.jones 2008-05-29 16:54) - PLID 26855 - if the index is <= nCountStatuses,
				// the result is a status ID, otherwise the result is a room ID, but really it is nCountStatuses + nRoomID
				if(nCmdId <= nMaxStatusID && !bLaunchDevice) {

					//the user is changing the status

					long nRoomStatusID = nCmdId;

					// (j.jones 2008-05-29 11:09) - PLID 27797 - convert -2 to 0, to support "Ready To Check Out"
					if(nRoomStatusID == -2) {
						nRoomStatusID = 0;
					}

					//if not 0, and not our fixed values, it must be a status ID
					ChangeRoomAppointmentStatus(VarLong(pRow->GetValue(rmrlcRoomAppointmentID),-1), VarLong(pRow->GetValue(rmrlcApptID),-1), nRoomStatusID);
				}
				else if(!bLaunchDevice) {

					//the user is switching rooms

					long nRoomID = nCmdId - nMaxStatusID;
					AssignToRoom(VarLong(pRow->GetValue(rmrlcApptID),-1), nRoomID);
				}
				break;
			 }
		}
	}
	// (d.lange 2010-11-04 09:37) - PLID 41211 - Destory all loaded device plugins, so we don't leak memory
	// (j.gruber 2013-04-02 12:29) - PLID 56012 - Consolidate
	DeviceLaunchUtils::DestroyLoadedDevicePlugins(aryLoadedPlugins);

	mnu.DestroyMenu();
}


// (s.dhole 2010-02-24 16:46) - PLID 37112 Workflow change from room manager -> EMR for doctors.
// This  function midify to support tab information
bool CRoomManagerDlg::GoToPatient(long nPatientID, short nTab)
{
	bool bResult=false;
	try {

		//Set the active patient
		CMainFrame *pMainFrame;
		pMainFrame = GetMainFrame();
		if (pMainFrame != NULL) {

			//CNxModelessOwnedDialog::OnOK();

			if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
				if(IDNO == MessageBox("This patient is not in the current lookup. \n"
					"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return false;
				}
			}

			// (s.dhole 2010-03-03 12:51) - PLID  PLID 37112 Workflow change from room manager -> EMR for doctors.
			if (nTab==-1  ) 
			{
				// (z.manning 2009-08-18 11:27) - PLID 28014 - Minimize this dialog when going to a patient
				MinimizeWindow();
			}
			//TES 1/7/2010 - PLID 36761 - This function may fail now
			if(!pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {
				return false;
			}

			//Now just flip to the patient's module and set the active Patient
			pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
			CNxTabView *pView = pMainFrame->GetActiveView();
			if(pView)
				if (nTab>-1  )// (s.dhole 2010-02-24 16:46) - PLID 37112 Workflow change from room manager -> EMR for doctors.
					pView->SetActiveTab(nTab);// (s.dhole 2010-02-24 16:46) - PLID 37112 Workflow change from room manager -> EMR for doctors.
			pView->UpdateView();

			return true; 

		}//end if MainFrame
		else {
			MsgBox(MB_ICONSTOP|MB_OK, "Error in CRoomManagerDlg::GoToPatient: Cannot Open Mainframe");
			return false;
		}//end else pMainFrame

	}NxCatchAll("Error in CRoomManagerDlg::GoToPatient");
	return false;
}

void CRoomManagerDlg::GoToAppointment(long nApptID) {
	
	try {

		CMainFrame  *pMainFrame;
		pMainFrame = GetMainFrame();
		if (pMainFrame != NULL) {

			//CNxModelessOwnedDialog::OnOK();

			if (pMainFrame->FlipToModule(SCHEDULER_MODULE_NAME)) {
				CNxTabView *pView = pMainFrame->GetActiveView();
				if (pView && pView->IsKindOf(RUNTIME_CLASS(CSchedulerView))) {
					((CSchedulerView *)pView)->OpenAppointment(nApptID, GetRemotePropertyInt("ApptAutoOpenResEntry", 1, 0, GetCurrentUserName(), true) ? TRUE : FALSE);
				}//end pView
			}
		}//end pMainFrame
		else {
			//MainFrame pointer is null
			MsgBox(MB_ICONSTOP|MB_OK, "Error in CRoomManagerDlg::GoToAppointment: Cannot Open Mainframe");
		}//end else

	}NxCatchAll("Error in CRoomManagerDlg::GoToAppointment");
}

void CRoomManagerDlg::AssignToRoom(long nApptID, long nRoomID)
{
	long nAuditTransactionID = -1;
	//(a.wilson 2011-10-3) PLID 33592 - set to 1 so we don't have to copy code later to do the same thing.
	long nStatusID = 1;

	try {

		if(!HasPermissionForResource(nApptID,sptWrite,sptWriteWithPass))
			return;

		CString strRoomName;

		// (j.jones 2008-05-29 16:24) - PLID 26855 - get the new room name, while performing the check
		// that ensures the room is not in use, inactive, or deleted
		//TES 8/29/2008 - PLID 26534 - They can assign patients to rooms that are in use now, we just need to warn them.
		// (j.jones 2010-12-01 09:59) - PLID 38597 - do not flag waiting rooms as In Use
		_RecordsetPtr rs = CreateParamRecordset("SELECT ID, Name, RoomsT.WaitingRoom, "
			"CASE WHEN RoomsT.WaitingRoom = 0 AND RoomsT.ID IN (SELECT RoomID FROM RoomAppointmentsT WHERE StatusID NOT IN (0, -1)) THEN 1 ELSE 0 END AS InUse "
			"FROM RoomsT "
			"WHERE Inactive = 0 "
			"AND ID = {INT} ", nRoomID);

		BOOL bIsBeingAssignedToWaitingRoom = FALSE;

		if(!rs->eof) {
			//the room is available, so get its name
			strRoomName = AdoFldString(rs, "Name","");

			bIsBeingAssignedToWaitingRoom = AdoFldBool(rs, "WaitingRoom", FALSE);
			
			long nInUse = AdoFldLong(rs, "InUse");
			if(nInUse == 1) {
				//TES 8/29/2008 - PLID 26534 - The room is in use, so warn them, but let them go ahead if they want.
				if(IDNO == MsgBox(MB_YESNO, "The room '%s' is already in use, are you sure you want to assign another patient to this room?", strRoomName)) {
					return;
				}
			}			
		}
		else {

			//no longer available, it's either in use, inactive, or deleted
			//the calling code should have already checked this, so the odds of this are slim

			//requery the room list to reflect the change
			RequeryRoomList(); // (c.haag 2010-05-07 11:16) - PLID 35702 - Requery the room list in its own function

			//now explain ourselves
			AfxMessageBox("This room is no longer available to use, please assign the patient to a different room.");			
			return;
		}
		rs->Close();

		//ensure the appt. is not cancelled or deleted
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if(!ReturnsRecordsParam("SELECT ID FROM AppointmentsT WHERE Status <> 4 AND ID = {INT}", nApptID)) {
			
			//no longer available

			// (j.jones 2006-10-03 10:18) - note, if we ever remove the requery here, remember
			// to auto-collapse the previous hour if the last appt. is checked out

			//requery the appt. lists to reflect the change
			RequeryPatientList(TRUE);
			RequeryWaitingAreaList();
			// (j.jones 2008-05-29 10:53) - PLID 27797 - requery the Checkout list
			RequeryCheckoutList();
			
			
			//now explain ourselves
			AfxMessageBox("This appointment has either been cancelled or deleted. The patient cannot be moved into the room.");
			return;
		}

		// (j.jones 2008-05-29 15:34) - PLID 26855 - Before we looked to see if the patient was in any other room,
		// disallowed reassigning to another room if they were currently in one, and simply warned if they had
		// previously checked into and out of a room.
		// Now we allow moving directly from room to room, so what we need to do is only warn if they are
		// not in a room now, but were once in the past, then make sure the patient is 'Checked Out' of all previous
		// rooms, before being checked into the new room.

		//see if the appt. is not in a room now, but has been in the past
		// (j.jones 2010-12-01 10:16) - PLID 38597 - ignore waiting rooms
		if(!bIsBeingAssignedToWaitingRoom) {
			rs = CreateParamRecordset("SELECT ID FROM AppointmentsT "
				"WHERE ID = {INT} "
				"AND ID IN (SELECT AppointmentID FROM RoomAppointmentsT "
				"	INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"	WHERE RoomsT.WaitingRoom = 0 AND RoomAppointmentsT.StatusID IN (0, -1)) "
				"AND ID NOT IN (SELECT AppointmentID FROM RoomAppointmentsT "
				"	INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"	WHERE RoomsT.WaitingRoom = 0 AND StatusID NOT IN (0, -1))", nApptID);
			if(!rs->eof) {
				//the appointment was already in and out of a room before, and is not currently in a room,
				//so prompt to make sure they aren't re-checking-in on accident

				if(IDNO == MessageBox("This patient's appointment has already been assigned to, and checked out of, a room.\n\n"
					"Are you sure you wish to move the patient back into another room?","Practice", MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
			rs->Close();
		}

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		//CString strSql = BeginSqlBatch();
		CParamSqlBatch batch;
		BOOL bPrompted = FALSE;

		batch.Declare("SET NOCOUNT ON");
		batch.Declare("DECLARE @nNewRoomAppointmentID INT");
		batch.Declare("DECLARE @nNewRoomAppointmentHistoryID INT");

		//track all the rooms we check out, so we can send tablecheckers
		CArray<long, long> aryCheckedOutRoomIDs;
		
		//now find what rooms this patient is already in, and not checked out of (Status = -1),
		//this intentionally includes 'ready to check out' entries (Status = 0)
		//(a.wilson 2011-10-3) PLID 33592 - added the status id to the query.
		rs = CreateParamRecordset("SELECT RoomAppointmentsT.ID, PatientID, "
			"RoomsT.Name AS RoomName, RoomStatusT.Name AS StatusName, RoomStatusT.ID AS StatusID "
			"FROM RoomAppointmentsT "
			"INNER JOIN AppointmentsT ON RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
			"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
			"INNER JOIN RoomStatusT ON RoomAppointmentsT.StatusID = RoomStatusT.ID "
			"WHERE AppointmentID = {INT} AND StatusID <> -1 ", nApptID);
		while(!rs->eof) {

			//in theory there should never be more than 1 result in the above recordset

			//mark this room appointment as checked out

			long nRoomAppointmentID = AdoFldLong(rs, "ID", -1);
			long nPatientID = AdoFldLong(rs, "PatientID", -1);
			CString strOldRoomName = AdoFldString(rs, "RoomName", "");
			CString strOldStatus = AdoFldString(rs, "StatusName", "");
			//(a.wilson 2-11=10=3) PLID 33592 - get the previous status for the last room if the user wants to keep it.
			// (b.spivey, May 08, 2013) - PLID 52670 - Grab the status, if it's ready to be checked out 
			//	(checked out patients disappear entirely) then we need to set it back to 1.
			// (b.spivey, June 11, 2013) - PLID 52670 - rearranged this so that it wouldn't set the statusID unnecessarily. 
			if (GetRemotePropertyInt("RoomMgr_KeepStatusWhenSwitchingRooms", 0, 0, GetCurrentUserName()) == 1) {
				nStatusID = AdoFldLong(rs, "StatusID", 1);
				if (nStatusID == 0) {
					//we came from the checkout, so we need to change the status or else we'll be stuck
					nStatusID = 1;
				}
			}

			batch.Add("UPDATE RoomAppointmentsT SET StatusID = -1, LastUpdateTime = GetDate(), LastUpdateUserID = {INT} WHERE ID = {INT}", GetCurrentUserID(), nRoomAppointmentID);
			
			//and log this in the history
			// (a.walling 2013-06-07 09:46) - PLID 57078 - RoomAppointmentHistoryT no longer has an ID column
			batch.Add("INSERT INTO RoomAppointmentHistoryT (RoomAppointmentID, UpdateUserID, StatusID) "
				"VALUES ({INT}, {INT}, -1)", nRoomAppointmentID, GetCurrentUserID());

			//track that we checked this room out
			aryCheckedOutRoomIDs.Add(nRoomAppointmentID);

			//audit that we checked out the room, but note that it moved to another room
			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}

			CString strOldValue, strNewValue;
			strOldValue.Format("Room: '%s', Status: '%s'", strOldRoomName, strOldStatus);
			strNewValue.Format("Moved to room: '%s'", strRoomName);

			AuditEvent(nPatientID == -25 ? -1 : nPatientID, nPatientID == -25 ? "" : GetExistingPatientName(nPatientID), nAuditTransactionID, aeiRoomApptCheckout, nApptID, strOldValue, strNewValue, aepMedium, aetChanged);

			rs->MoveNext();
		}
		rs->Close();

		//ok, now assign to the room!

		//we will use the hard-coded status of 1 for checked-in,
		//and allow the times to use their default of GetDate()
				
		//(a.wilson 2011-10-3) PLID 33592 - added the nstatusid variable to change the status depending on pref.
		// (a.walling 2013-06-07 09:50) - PLID 57079 - RoomAppointmentsT.ID now an identity; no more NewNumber / max(ID)
		batch.Add("INSERT INTO RoomAppointmentsT (RoomID, AppointmentID, LastUpdateUserID, StatusID) "
			"VALUES ({INT}, {INT}, {INT}, {INT})", nRoomID, nApptID, GetCurrentUserID(), nStatusID);
		batch.Add("SET @nNewRoomAppointmentID = SCOPE_IDENTITY()");
		
		//and log this in the history		
		// (a.walling 2013-06-07 09:46) - PLID 57078 - RoomAppointmentHistoryT no longer has an ID column
		batch.Add("INSERT INTO RoomAppointmentHistoryT (RoomAppointmentID, UpdateUserID, StatusID) "
			"VALUES (@nNewRoomAppointmentID, {INT}, {INT})", GetCurrentUserID(), nStatusID);

		batch.Declare("SET NOCOUNT OFF");

		//we need to audit this, so just pull the information out of this batch
		batch.Declare(
				"SELECT @nNewRoomAppointmentID AS ID, PatientID, StartTime, Name, "
				"CASE WHEN Convert(datetime, Convert(nvarchar, AppointmentsT.Date, 1)) = Convert(datetime, Convert(nvarchar, GetDate(), 1)) "
				"AND StartTime = EndTime AND DatePart(hh, StartTime) = 0 THEN 1 "
				"ELSE 0 END AS IsEvent "
				"FROM RoomAppointmentsT "
				"INNER JOIN AppointmentsT ON RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
				"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"WHERE RoomAppointmentsT.ID = @nNewRoomAppointmentID");

		rs = batch.CreateRecordset(GetRemoteData());

		long nRoomAppointmentID = -1;

		if(!rs->eof) {
			
			nRoomAppointmentID = AdoFldLong(rs, "ID", -1);

			//for auditing
			long nPatientID = AdoFldLong(rs, "PatientID",-1);
			CString strRoomName = AdoFldString(rs, "Name","");
			long nIsEvent = AdoFldLong(rs, "IsEvent",0);
			COleDateTime dtStartTime = AdoFldDateTime(rs, "StartTime");

			CString strOldValue, strNewValue;

			strOldValue.Format("Appt. Time: %s", nIsEvent == 1 ? "<Event>" : FormatDateTimeForInterface(dtStartTime, NULL, dtoDateTime));
			strNewValue.Format("Assigned to '%s'", strRoomName);

			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}

			AuditEvent(nPatientID == -25 ? -1 : nPatientID, nPatientID == -25 ? "" : GetExistingPatientName(nPatientID), nAuditTransactionID, aeiRoomApptAssign, nApptID, strOldValue, strNewValue, aepMedium, aetCreated);
		}
		else {
			//should be impossible
			ThrowNxException("Failed to create a new RoomAppointment record!");
		}
		rs->Close();

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
		}

		//not needed - the network code will do this
		//m_pRoomList->Requery();

		//send a network message for each room we checked out (if any)
		//in theory this should never be more than 1
		for(int i=0;i<aryCheckedOutRoomIDs.GetSize();i++) {
			//this tablechecker will check out the appointment,
			//but the next tablechecker below will re-check it into the new room
			CClient::RefreshRoomAppointmentTable(aryCheckedOutRoomIDs.GetAt(i));			
		}
		aryCheckedOutRoomIDs.RemoveAll();

		//and send a network message for our new room appointment
		CClient::RefreshRoomAppointmentTable(nRoomAppointmentID);

		//if the preference says to mark the appointment 'In', do so
		if(GetRemotePropertyInt("MarkApptInOnRoomAssign", 0, 0, GetCurrentUserName(), true) == 1) {

			// (j.jones 2006-10-23 15:15) - PLID 23174 - also mark in if a no show appt,
			
			//but only if the appointment is currently 'Pending' (or 'No Show')
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			if(ReturnsRecordsParam("SELECT ID FROM AppointmentsT WHERE ID = {INT} AND (ShowState = 0 OR ShowState = 3)",nApptID)) {

				//the appointment is 'Pending', so mark it 'In'
				AppointmentMarkIn(nApptID);
			}
		}

		//color the patient row blue
		IRowSettingsPtr pRow = m_pPatientList->FindByColumn(rmplcApptID, (long)nApptID, 0, FALSE);
		if(pRow) {
			//color the appt. row normally
			ColorApptRow(pRow);
		}

	}NxCatchAllCall("Error in CRoomManagerDlg::AssignToRoom",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	);
}

void CRoomManagerDlg::OnEditingStartingRoomList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL) {
			return;
		}

		if(pRow->GetValue(rmrlcRoomAppointmentID).vt != VT_I4) {
			*pbContinue = FALSE;
			return;
		}

		if(!HasPermissionForResource(VarLong(pRow->GetValue(rmrlcApptID),-1),sptWrite,sptWriteWithPass)) {
			*pbContinue = FALSE;
			return;
		}

	}NxCatchAll("CRoomManagerDlg::OnEditingStartingRoomList");
}

void CRoomManagerDlg::OnEditingFinishingRoomList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {

	}NxCatchAll("CRoomManagerDlg::OnEditingFinishingRoomList");	
}

void CRoomManagerDlg::OnEditingFinishedRoomList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL) {
			return;
		}

		if(_variant_t(varOldValue) == _variant_t(varNewValue)) {
			return;
		}

		if(nCol == rmrlcStatusID) {

			long nRoomAppointmentID = VarLong(pRow->GetValue(rmrlcRoomAppointmentID),-1);
			long nAppointmentID = VarLong(pRow->GetValue(rmrlcApptID),-1);
			long nStatus = VarLong(varNewValue,-1);

			// (j.jones 2008-06-02 10:06) - PLID 30219 - called ChangeRoomAppointmentStatus
			// because this code previously duplicated that function
			ChangeRoomAppointmentStatus(nRoomAppointmentID, nAppointmentID, nStatus);
		}

	}NxCatchAll("CRoomManagerDlg::OnEditingFinishedRoomList");
}

void CRoomManagerDlg::ChangeRoomAppointmentStatus(long nRoomAppointmentID, long nAppointmentID, long nNewStatusID)
{
	try {

		// (c.haag 2009-09-28 15:39) - PLID 35101 - If the room appointment ID is -1, that means the room
		// appointment was likely changed from another computer just moments ago. Give a warning to the user,
		// refresh the entire manager, and then exit.
		if (-1 == nRoomAppointmentID) {
			AfxMessageBox("The appointment status could not be updated. This may be because it was just modified by "
				"another user.", MB_OK | MB_ICONERROR);
			RequeryAll(TRUE);
			return;
		}

		//for auditing

		long nPatientID = -25;
		CString strRoomName, strOldStatus, strNewStatus;
		CString strAppointmentDescription;
		CString strPatientFullName, strPatientLast, strPatientFirst;

		// (j.jones 2008-06-02 09:40) - PLID 30219 - added an appointment description for the potential pracyak message
		_RecordsetPtr rs = CreateParamRecordset("SELECT PatientID, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS FullName, PersonT.Last, PersonT.First, "
			"RoomsT.Name AS RoomName, RoomStatusT.Name AS RoomStatusName, "
			"CAST(CASE WHEN DATEPART(hh, StartTime) = 0 THEN 12 WHEN DATEPART(hh, StartTime) <= 12 THEN DATEPART(hh, StartTime) ELSE DATEPART(hh, StartTime) - 12 END AS nvarchar) + ':' + CASE WHEN DATEPART(mi, StartTime) < 10 THEN '0' ELSE '' END + CAST(DATEPART(mi, StartTime) AS nvarchar) + CASE WHEN DATEPART(hh, StartTime) >= 12 THEN 'PM' ELSE 'AM' END "
			" + ' - ' + "
			"CAST(CASE WHEN DATEPART(hh, EndTime) = 0 THEN 12 WHEN DATEPART(hh, EndTime) <= 12 THEN DATEPART(hh, EndTime) ELSE DATEPART(hh, EndTime) - 12 END AS nvarchar) + ':' + CASE WHEN DATEPART(mi, EndTime) < 10 THEN '0' ELSE '' END + CAST(DATEPART(mi, EndTime) AS nvarchar) + CASE WHEN DATEPART(hh, EndTime) >= 12 THEN 'PM' ELSE 'AM' END "
			"	+ ', ' + COALESCE(AptTypeT.Name, '<No Type>') "
			"	+ ' - ' + COALESCE(dbo.GetPurposeString(AppointmentsT.ID), '<No Purpose>') AS ApptDesc "
			"FROM RoomAppointmentsT "
			"INNER JOIN AppointmentsT ON RoomAppointmentsT.AppointmentID = AppointmentsT.ID "			
			"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
			"INNER JOIN RoomStatusT ON RoomAppointmentsT.StatusID = RoomStatusT.ID "
			"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE RoomAppointmentsT.ID = {INT}", nRoomAppointmentID);
		if(!rs->eof) {

			nPatientID = AdoFldLong(rs, "PatientID",-1);

			if(nPatientID != -25) {
				strPatientFullName = AdoFldString(rs, "FullName","");
				strPatientLast = AdoFldString(rs, "Last","");
				strPatientFirst = AdoFldString(rs, "First","");
			}

			strRoomName = AdoFldString(rs, "RoomName","");
			strOldStatus = AdoFldString(rs, "RoomStatusName","");
			strAppointmentDescription = AdoFldString(rs, "ApptDesc","");
		}
		else {
			// (c.haag 2009-09-28 15:39) - PLID 35101 - If the room appointment ID is -1, that means the room
			// appointment was likely changed from another computer just moments ago. Give a warning to the user,
			// refresh the entire manager, and then exit.
			AfxMessageBox("The appointment status could not be updated. This may be because it was just modified by "
				"another user.", MB_OK | MB_ICONERROR);
			RequeryAll(TRUE);
		}
		rs->Close();

		if(nNewStatusID != -1) {
			rs = CreateParamRecordset("SELECT Name FROM RoomStatusT WHERE ID = {INT}", nNewStatusID);
			if(!rs->eof) {
				strNewStatus = AdoFldString(rs, "Name","");
			}
			rs->Close();
		}

		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		CParamSqlBatch batch;
		  
		// (s.dhole 2010-11-16 15:34) - PLID 39200 we will mark item to Check out, We are moving out patient form a room
		//if (nNewStatusID==-3)
		//{
		//	batch.Add( "UPDATE AppointmentsT SET ShowState = 1, ModifiedDate = GetDate(), ModifiedLogin = {STRING} WHERE AppointmentsT.ID = {INT}", _Q(GetCurrentUserName()), nAppointmentID);
		//	batch.Add("UPDATE RoomAppointmentsT SET StatusID = -1, LastUpdateTime = GetDate(), LastUpdateUserID = {INT} WHERE ID = {INT}",  GetCurrentUserID(), nRoomAppointmentID);
		//}
		//else
		//{
		batch.Add("UPDATE RoomAppointmentsT SET StatusID = {INT}, LastUpdateTime = GetDate(), LastUpdateUserID = {INT} WHERE ID = {INT}", nNewStatusID, GetCurrentUserID(), nRoomAppointmentID);
		//}
		//and log this in the history
		// (a.walling 2013-06-07 09:46) - PLID 57078 - RoomAppointmentHistoryT no longer has an ID column
		batch.Add("INSERT INTO RoomAppointmentHistoryT (RoomAppointmentID, UpdateUserID, StatusID) "
			"VALUES ({INT}, {INT}, {INT})", nRoomAppointmentID, GetCurrentUserID(), nNewStatusID);

		batch.Execute(GetRemoteData());

		//now audit the change
		{
			CString strOldValue, strNewValue;

			strOldValue.Format("Room: '%s', Status: '%s'", strRoomName, strOldStatus);
			//if(nNewStatusID == -3) // (s.dhole 2010-11-16 15:34) - PLID 39200 If user mark room or in appointment as Waitting room
			//	strNewValue = "Waiting Area";
			//else 
			if(nNewStatusID == -1)
				strNewValue = "Checked Out";
			else
				strNewValue.Format("Status: '%s'", strNewStatus);

			long nAuditID = BeginNewAuditEvent();
			//strPatientFullName will be blank if the patient ID is -25
			AuditEvent(nPatientID == -25 ? -1 : nPatientID, strPatientFullName, nAuditID, nNewStatusID == -1 ? aeiRoomApptCheckout : aeiRoomApptStatus, nAppointmentID, strOldValue, strNewValue, aepMedium, aetChanged);
		}

		//update the status in the patient row
		IRowSettingsPtr pPatientRow = m_pPatientList->FindByColumn(rmplcApptID, nAppointmentID, 0, FALSE);
		if(pPatientRow) {
			pPatientRow->PutValue(rmplcRoomStatusID, nNewStatusID);
		}

		// (j.jones 2008-05-30 09:04) - PLID 27797 - color appropriately
		ColorApptRow(pPatientRow);

		//if we checked out the patient, try to collapse the row
		if(nNewStatusID == -1) {

			//collapse the previous hour 
			CheckAutoCollapseFinishedHours();

			//check and see if we need to mark the appt. 'Out'
			PostCheckedOut(nRoomAppointmentID, nAppointmentID);
		}

		//in all cases requery the room list to update the time (with server time), status,
		//and "with person" indicator

		//not needed, the network code will do this
		//m_pRoomList->Requery();

		//and send a network message
		CClient::RefreshRoomAppointmentTable(nRoomAppointmentID);
		// (s.dhole 2010-11-16 15:35) - PLID 39200 if user change status to waititng room than refresh all lists
		//if(nNewStatusID ==-3) {
		//	CClient::RefreshAppointmentTable(nAppointmentID);
		//}
		// (j.jones 2008-06-02 09:13) - PLID 30219 - call OnReadyToCheckOut so a PracYak can be sent
		if(nNewStatusID == 0) {
			OnReadyToCheckOut(nPatientID, strPatientFirst, strPatientLast, strAppointmentDescription);
		}

	}NxCatchAll("CRoomManagerDlg::ChangeRoomAppointmentStatus");
}

//returns TRUE if the appointment was modified
// (j.jones 2008-05-30 10:10) - PLID 27797 - renamed to PostCheckedOut, formerly was called PostRemovedFromRoom
BOOL CRoomManagerDlg::PostCheckedOut(long nRoomAppointmentID, long nAppointmentID)
{
	try {

		//if the preference says to mark the appointment 'Out', do so
		if(GetRemotePropertyInt("MarkApptOutOnRoomClear", 0, 0, GetCurrentUserName(), true) == 1) {
			
			//but only if the appointment is not already 'Out'
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			if(ReturnsRecordsParam("SELECT ID FROM AppointmentsT WHERE ID = {INT} AND ShowState <> 2",nAppointmentID)) {

				//the appointment is not 'Out', so mark it as such
				AppointmentMarkOut(nAppointmentID);
				return TRUE;
			}
		}

	}NxCatchAll("CRoomManagerDlg::PostCheckedOut");

	return FALSE;
}

LRESULT CRoomManagerDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {

			case NetUtils::AppointmentsT: {

				//use the extra details to update the screen
				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				ReflectChangedAppointment(pDetails);
				}
				break;
			
			case NetUtils::RoomAppointmentsT: {

				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				ReflectChangedRoomAppointment(pDetails);
				}
				break;
			case NetUtils::PatCombo:
				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				long nPatientID = VarLong(pDetails->GetDetailData(CClient::pcdiPersonID), -1);

				//if the ID is in the today's patient list, requery
				if (m_pPatientList->FindByColumn(rmplcPatientID, nPatientID, 0, FALSE) != NULL) {
					m_pPatientList->Requery();
				}

				//if the ID is in the room list, requery
				if (m_pRoomList->FindByColumn(rmrlcPatientID, nPatientID, 0, FALSE) != NULL) {
					RequeryRoomList(); // (c.haag 2010-05-07 11:16) - PLID 35702 - Requery the room list in its own function
				}

				//if the ID is in the waiting area list, requery
				if (m_pWaitingAreaList->FindByColumn(rmwalcPatientID, nPatientID, 0, FALSE) != NULL) {
					RequeryWaitingAreaList();
				}

				// (j.jones 2008-05-29 10:53) - PLID 27797 - requery the Checkout list if the appt. is in it
				if (m_pCheckoutList->FindByColumn(rmclcPatientID, nPatientID, 0, FALSE) != NULL) {
					RequeryCheckoutList();
				}

				// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
				Flash();
				break;
		}

	} NxCatchAll("Error in CRoomManagerDlg::OnTableChangedEx");

	return 0;
}

LRESULT CRoomManagerDlg::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {

			case NetUtils::RoomSetup:
				
				// (j.jones 2010-12-02 09:09) - PLID 38597 - requery waiting room combo
				// and the waiting area list
				RequeryWaitingRoomCombo();
				RequeryWaitingAreaList();
				
				
				RequeryRoomList(); // (c.haag 2010-05-07 11:16) - PLID 35702 - Requery the room list in its own function

				// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
				Flash();
				break;
				
			case NetUtils::DeletedPtAppt:
				{
				//note: as long as the PatCombo message requeries, this code is likely never
				//going to actually find any appointments because this message is received second

				//if the appointment is in our patient list, remove it
				IRowSettingsPtr pRow = m_pPatientList->FindByColumn(rmplcApptID, (long)lParam, 0, FALSE);
				if(pRow) {
					IRowSettingsPtr pParentRow = pRow->GetParentRow();

					m_pPatientList->RemoveRow(pRow);

					if(pParentRow != NULL) {
						if(pParentRow->GetFirstChildRow()) {
							//color black
							pParentRow->PutForeColor(RGB(0,0,0));
						}
						else {
							//color gray
							pParentRow->PutForeColor(m_clrFolderEmpty);
						}
					}
					CheckAutoCollapseFinishedHours();
				}

				//if the appointment is in the room list (which shouldn't be possible), requery
				if(m_pRoomList->FindByColumn(rmrlcApptID, (long)lParam, 0, FALSE) != NULL) {
					RequeryRoomList(); // (c.haag 2010-05-07 11:16) - PLID 35702 - Requery the room list in its own function
				}
				
				//if the appointment is in the waiting area list, requery
				if(m_pWaitingAreaList->FindByColumn(rmwalcApptID, (long)lParam, 0, FALSE) != NULL) {
					RequeryWaitingAreaList();
				}

				// (j.jones 2008-05-29 10:53) - PLID 27797 - requery the Checkout list if the appt. is in it
				if(m_pCheckoutList->FindByColumn(rmclcApptID, (long)lParam, 0, FALSE) != NULL) {
					RequeryCheckoutList();
				}

				// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
				Flash();
				}
				break;

			case NetUtils::PatCombo:
				
				//if the ID is in the today's patient list, requery
				if(m_pPatientList->FindByColumn(rmplcPatientID, (long)lParam, 0, FALSE) != NULL) {
					m_pPatientList->Requery();
				}

				//if the ID is in the room list, requery
				if(m_pRoomList->FindByColumn(rmrlcPatientID, (long)lParam, 0, FALSE) != NULL) {
					RequeryRoomList(); // (c.haag 2010-05-07 11:16) - PLID 35702 - Requery the room list in its own function
				}

				//if the ID is in the waiting area list, requery
				if(m_pWaitingAreaList->FindByColumn(rmwalcPatientID, (long)lParam, 0, FALSE) != NULL) {
					RequeryWaitingAreaList();
				}

				// (j.jones 2008-05-29 10:53) - PLID 27797 - requery the Checkout list if the appt. is in it
				if(m_pCheckoutList->FindByColumn(rmclcPatientID, (long)lParam, 0, FALSE) != NULL) {
					RequeryCheckoutList();
				}

				// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
				Flash();
				break;
			
			case NetUtils::LocationsT: {

					long nLocationID = -1;
					{
						IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
						if(pRow == NULL) {
							nLocationID = GetCurrentLocationID();
						}
						else {
							nLocationID = VarLong(pRow->GetValue(lccID));
						}
					}
					m_pLocationCombo->Requery();
					if(m_pLocationCombo->SetSelByColumn(lccID, nLocationID) == NULL) {
						m_pLocationCombo->SetSelByColumn(lccID, GetCurrentLocationID());
					}
					
					RequeryAll(TRUE);

					// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
					Flash();
					break;
				}

			case NetUtils::AppointmentsT:
				// (j.jones 2006-10-06 14:52) - near as I can figure, this message
				// either never happens, or happens very infrequently. Either way,
				// we don't get any information this way that can help us.
				// (j.jones 2014-08-05 11:08) - PLID 63185 - do not requery on non-EX messages
				break;

			case NetUtils::RoomAppointmentsT:
				// (j.jones 2006-10-06 15:02) - this should never happen,
				// but if it does, requery everything
				// (j.jones 2014-08-18 15:40) - PLID 63185 - do not requery on non-EX messages
				break;

			// (c.haag 2010-10-26 10:07) - PLID 39199 - If the AptShowStateT changed, we have
			// to update the map that associates ID's with waiting area bits.
			case NetUtils::AptShowStateT:
				// (j.jones 2010-12-09 10:20) - PLID 41763 - renamed this function, we now load all show states
				UpdateCachedShowStateInfo();
				// We also have to requery the rooms. This may be redundant as code that fires this
				// should also fire appointment table checkers, but we need to future-proof ourselves
				// from times where we get the appointment refresh -before- we get the show state
				// refresh.
				RequeryAll(TRUE);
				Flash();
				break;
		}

	} NxCatchAll("Error in CRoomManagerDlg::OnTableChanged");

	return 0;
}

void CRoomManagerDlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == m_nTimerHourlyServerTimeCheck) {

		KillTimer(m_nTimerHourlyServerTimeCheck);

		//CalculateLocalTimeOffsetensures our offset
		//between server time and local time,
		//and restarts the timer
		CalculateLocalTimeOffset();
	}
	else if(nIDEvent == m_nTimerHourExpansion) {
		
		KillTimer(m_nTimerHourExpansion);
		
		ExpandCurrentHour(TRUE, TRUE);

		CheckAutoCollapseFinishedHours();

		//now start the timer up again
		//(from here it's virtually always going to be an hour, but this is the more proper calculation)
		ResetHourTimer();
	}
	// (z.manning 2009-07-10 13:50) - PLID 34848 - Added a timer to do a full refresh every so often
	else if(nIDEvent == m_nTimerFullRefresh) {
		RequeryAll(TRUE);
		// (z.manning 2009-07-10 14:11) - PLID 34848 - Let's not flash since it's possible nothing changed.
	}
	else {
		//presumably it's an AppointmentID

		KillTimer(nIDEvent);

		if(!m_pPatientList->IsRequerying()) {

			//this function will restart its timer, if necessary

			// (j.jones 2007-04-18 17:24) - PLID 25707 - an appointment will
			// never be in both lists, so only update the list that has the appointment
			// (j.jones 2008-05-29 11:17) - PLID 27797 - now there are three lists,
			// but the appt. will only be in one
	
			if(m_pRoomList->FindByColumn(rmrlcApptID, (long)nIDEvent, 0, FALSE) != NULL) {
				UpdateRoomWaitTime(nIDEvent);
			}
			else if(m_pWaitingAreaList->FindByColumn(rmwalcApptID, (long)nIDEvent, 0, FALSE) != NULL) {
				UpdateWaitingAreaWaitTime(nIDEvent);
			}
			else if(m_pCheckoutList->FindByColumn(rmclcApptID, (long)nIDEvent, 0, FALSE) != NULL) {
				UpdateCheckoutListWaitTime(nIDEvent);
			}
		}
	}
	
	CNxModelessOwnedDialog::OnTimer(nIDEvent);
}

void CRoomManagerDlg::OnRequeryFinishedRoomList(short nFlags) 
{
	try {

		//for each room, start/stop timers
		UpdateAllRoomWaitTimes(FALSE);

		// (c.haag 2010-05-07 11:11) - PLID 35702 - Restore the current topmost row to keep the
		// vertical scrollbar as consistent as possible with its pre-requery position
		if (-1 != m_nTopRoomID) {
			IRowSettingsPtr pCurrentTopRow = m_pRoomList->TopRow;
			IRowSettingsPtr pDesiredTopRow = m_pRoomList->FindByColumn(rmrlcRoomID, m_nTopRoomID, NULL, VARIANT_FALSE);
			// Only set the top row if:					
			if (NULL != pDesiredTopRow  // A. We found the top row in the list from before
				&& pCurrentTopRow == m_pRoomList->GetFirstRow() // B. The current top row is the first row in the list. If not, it means the user scrolled it during the requery.				
				) 
			{
				m_pRoomList->TopRow = pDesiredTopRow;
			}
			// Discard the ID; we don't need it anymore and we shouldn't leave it hanging around.
			m_nTopRoomID = -1;
		}

		// (d.lange 2010-12-08 17:11) - PLID 40295 - add EMR Preview icon for each row after requery
		for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRoomList->FindAbsoluteFirstRow(VARIANT_TRUE);
			pRow != NULL; pRow = pRow->GetNextRow()) {
				_variant_t varCheckInTime = pRow->GetValue(rmrlcCheckInTime);
				//COleDateTime dtCheckInTime = VarDateTime(varCheckInTime);
				if(varCheckInTime.vt != VT_NULL) {
					pRow->PutValue(rmrlcPreviewEMN, (long)m_hIconPreview);
				}
		}

		// (j.armen 2012-03-05 11:19) - PLID 48555 - When requery finishes, add the recall flag
		EnsureRecallFlagColumn(m_pRoomList, rmrlcRecallStatusID, rmrlcRecallStatus, "ShowRoomManagerRoomsColumnRecall");

	}NxCatchAll("Error in CRoomManagerDlg::OnRequeryFinishedRoomList");
}

// (j.jones 2007-02-07 11:22) - PLID 24595 - renamed the PatientWaitTime functions
// to reflect RoomWaitTimes
void CRoomManagerDlg::UpdateAllRoomWaitTimes(BOOL bClearColors)
{
	try {

		//for each room, start/stop timers

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pRoomList->GetFirstRow();
		while(pRow) {
			
			long nAppointmentID = VarLong(pRow->GetValue(rmrlcApptID), -1);

			//clearing colors isn't necessary if we're at the end of a requery,
			//otherwise we should make everything black first
			if(bClearColors) {
				//remove the purpose color if there is one
				pRow->PutCellForeColor(rmrlcPurpose, dlColorNotSet);
				pRow->PutForeColor(RGB(0,0,0));
				pRow->PutCellForeColor(rmrlcWaiting, RGB(0,0,0));
			}

			//UpdateRoomWaitTime will stop the timer, if it exists,
			//will detect if a time needs to be calculated, do so,
			//and start a new timer, if necessary.
			UpdateRoomWaitTime(nAppointmentID);

			pRow = pRow->GetNextRow();
		}

	}NxCatchAll("Error in CRoomManagerDlg::UpdateAllRoomWaitTimes");
}

void CRoomManagerDlg::UpdateRoomWaitTime(long nAppointmentID)
{
	try {

		//UpdateRoomWaitTime will stop the timer, if it exists,
		//will detect if a time needs to be calculated, do so,
		//and start a new timer, if necessary

		KillTimer(nAppointmentID);

		IRowSettingsPtr pRow = m_pRoomList->FindByColumn(rmrlcApptID, (long)nAppointmentID, 0, FALSE);
		if(pRow == NULL) {
			return;
		}

		// (j.jones 2010-08-31 15:24) - PLID 35012 - color the purpose by the apt. type color,
		// if the entire row is turned red later it will override this color
		
		//remove the color if there is one
		pRow->PutCellForeColor(rmrlcPurpose, dlColorNotSet);
		
		if(m_bColorPurposeColumnByType) {
			long nAptTypeColor = VarLong(pRow->GetValue(rmrlcAptTypeColor), 0);
			pRow->PutCellForeColor(rmrlcPurpose, nAptTypeColor);
		}

		//if the patient is with a person now, do nothing
		if(VarBool(pRow->GetValue(rmrlcWithPerson), FALSE)) {
			//ensure the row is black and "Waiting?" says "No"
			
			//remove the purpose color if there is one
			pRow->PutCellForeColor(rmrlcPurpose, dlColorNotSet);
			pRow->PutForeColor(RGB(0,0,0));
			pRow->PutCellForeColor(rmrlcWaiting, RGB(0,0,0));
			
			// (j.jones 2010-08-31 12:39) - PLID 35012 - fix the purpose color
			if(m_bColorPurposeColumnByType) {
				long nAptTypeColor = VarLong(pRow->GetValue(rmrlcAptTypeColor), 0);
				pRow->PutCellForeColor(rmrlcPurpose, nAptTypeColor);
			}

			pRow->PutValue(rmrlcWaiting, _bstr_t("No"));
			return;
		}

		//calculate the minutes the patient has been waiting
		COleDateTime dtNow = COleDateTime::GetCurrentTime() + m_dtOffset;
		COleDateTime dtLast;
		_variant_t var = pRow->GetValue(rmrlcTimeLastSeen);
		if(var.vt == VT_DATE) {
			dtLast = VarDateTime(var);			
		}
		else {
			//if not seen, use check-in time
			dtLast = VarDateTime(pRow->GetValue(rmrlcCheckInTime));
		}
		COleDateTimeSpan dtSpan;
		dtSpan = dtNow - dtLast;
		long nMinuteDifference = (dtSpan.GetHours() * 60) + dtSpan.GetMinutes();

		if(nMinuteDifference < 0) {
			//The offset should make this impossible in a real environment,
			//but not difficult to achieve in a developer's locally docked
			//environment. This would mean that the local time plus the
			//offset is still earlier than the "last seen" time, which can
			//really only occur if the server time has been changed to be
			//earlier than the timestamp for "last seen"
			CString strWaitTime = "0:00";
			pRow->PutValue(rmrlcWaiting, _bstr_t(strWaitTime));
		}
		else {
			CString strWaitTime;
			strWaitTime.Format("%li:%s%li", dtSpan.GetHours(), dtSpan.GetMinutes() < 10 ? "0" : "", dtSpan.GetMinutes());
			pRow->PutValue(rmrlcWaiting, _bstr_t(strWaitTime));
		}

		if(GetRemotePropertyInt("RoomManagerMaxWaitTime", 5, 0, GetCurrentUserName(), true) <= nMinuteDifference) {
			//if exceeding the maximum wait time, color the whole line red
			//remove the purpose color if there is one
			pRow->PutCellForeColor(rmrlcPurpose, dlColorNotSet);
			pRow->PutForeColor(RGB(255,0,0));
		}
		else {
			// (j.jones 2009-06-25 15:48) - PLID 34729 - ensure the row is black
			//remove the purpose color if there is one
			pRow->PutCellForeColor(rmrlcPurpose, dlColorNotSet);
			pRow->PutForeColor(RGB(0,0,0));

			// (j.jones 2010-08-31 12:39) - PLID 35012 - fix the purpose color
			if(m_bColorPurposeColumnByType) {
				long nAptTypeColor = VarLong(pRow->GetValue(rmrlcAptTypeColor), 0);
				pRow->PutCellForeColor(rmrlcPurpose, nAptTypeColor);
			}

			//just color the time red
			pRow->PutCellForeColor(rmrlcWaiting, RGB(255,0,0));
		}

		//now, start a new timer for the next minute since the patient has been waiting
		long nSecondsUntilNextMinute = 60 - dtSpan.GetSeconds();
		SetTimer(nAppointmentID, nSecondsUntilNextMinute * 1000, NULL);

	}NxCatchAllCall("Error in CRoomManagerDlg::UpdateRoomWaitTime",
		// (j.jones 2007-04-20 16:38) - PLID 25707 - give an explanation that the timer will not be reset
		AfxMessageBox("The Room Manager failed to update the wait time for a room, and the clock for that room has been disabled.\n"
			"You will need to close and reopen the Room Manager or Practice in order to restart the clock for the affected room.\n"
			"Please contact NexTech to resolve this issue.");
	);

}

// (j.jones 2007-02-07 11:22) - PLID 24595 - added WaitingAreaWaitTime functions
void CRoomManagerDlg::UpdateAllWaitingAreaWaitTimes(BOOL bClearColors)
{
	try {

		//for each room, start/stop timers

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pWaitingAreaList->GetFirstRow();
		while(pRow) {
			
			long nAppointmentID = VarLong(pRow->GetValue(rmwalcApptID), -1);

			//clearing colors isn't necessary if we're at the end of a requery,
			//otherwise we should make everything black first
			if(bClearColors) {
				//remove the purpose color if there is one
				pRow->PutCellForeColor(rmwalcPurpose, dlColorNotSet);
				pRow->PutForeColor(RGB(0,0,0));
				pRow->PutCellForeColor(rmwalcWaiting, RGB(0,0,0));
			}

			//UpdateWaitingAreaWaitTime will stop the timer, if it exists,
			//will detect if a time needs to be calculated, do so,
			//and start a new timer, if necessary.
			UpdateWaitingAreaWaitTime(nAppointmentID);

			pRow = pRow->GetNextRow();
		}

	}NxCatchAll("Error in CRoomManagerDlg::UpdateAllWaitingAreaWaitTimes");
}

void CRoomManagerDlg::UpdateWaitingAreaWaitTime(long nAppointmentID)
{
	try {

		//UpdateWaitingAreaWaitTime will stop the timer, if it exists,
		//will detect if a time needs to be calculated, do so,
		//and start a new timer, if necessary

		KillTimer(nAppointmentID);

		IRowSettingsPtr pRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, (long)nAppointmentID, 0, FALSE);
		if(pRow == NULL) {
			return;
		}

		// (j.jones 2010-08-31 12:39) - PLID 35012 - fix the purpose color
		if(m_bColorPurposeColumnByType) {
			long nAptTypeColor = VarLong(pRow->GetValue(rmwalcAptTypeColor), 0);
			pRow->PutCellForeColor(rmwalcPurpose, nAptTypeColor);
		}

		//calculate the minutes the patient has been waiting
		COleDateTime dtNow = COleDateTime::GetCurrentTime() + m_dtOffset;
		COleDateTime dtLast;
		_variant_t var = pRow->GetValue(rmwalcTimeLastSeen);
		if(var.vt == VT_DATE) {
			dtLast = VarDateTime(var);
		}
		else {
			//if not seen, use check-in time
			dtLast = VarDateTime(pRow->GetValue(rmwalcCheckInTime), VarDateTime(pRow->GetValue(rmwalcApptTime)));
		}
		COleDateTimeSpan dtSpan;
		dtSpan = dtNow - dtLast;
		long nMinuteDifference = (dtSpan.GetHours() * 60) + dtSpan.GetMinutes();

		if(nMinuteDifference < 0) {
			//The offset should make this impossible in a real environment,
			//but not difficult to achieve in a developer's locally docked
			//environment. This would mean that the local time plus the
			//offset is still earlier than the "last seen" time, which can
			//really only occur if the server time has been changed to be
			//earlier than the timestamp for "last seen"
			CString strWaitTime = "0:00";
			pRow->PutValue(rmwalcWaiting, _bstr_t(strWaitTime));
		}
		else {
			CString strWaitTime;
			strWaitTime.Format("%li:%s%li", dtSpan.GetHours(), dtSpan.GetMinutes() < 10 ? "0" : "", dtSpan.GetMinutes());
			pRow->PutValue(rmwalcWaiting, _bstr_t(strWaitTime));
		}

		//use the same MaxWaitTime as the Room list
		if(GetRemotePropertyInt("RoomManagerMaxWaitTime", 5, 0, GetCurrentUserName(), true) <= nMinuteDifference) {
			//if exceeding the maximum wait time, color the whole line red
			//remove the purpose color if there is one
			pRow->PutCellForeColor(rmwalcPurpose, dlColorNotSet);
			pRow->PutForeColor(RGB(255,0,0));
		}
		else {
			//just color the time red
			pRow->PutCellForeColor(rmwalcWaiting, RGB(255,0,0));
		}

		//now, start a new timer for the next minute since the patient has been waiting
		long nSecondsUntilNextMinute = 60 - dtSpan.GetSeconds();
		SetTimer(nAppointmentID, nSecondsUntilNextMinute * 1000, NULL);

	}NxCatchAllCall("Error in CRoomManagerDlg::UpdateWaitingAreaWaitTime",
		// (j.jones 2007-04-20 16:38) - PLID 25707 - give an explanation that the timer will not be reset
		AfxMessageBox("The Room Manager failed to update the wait time for a patient in the Waiting Area, and the clock for that patient has been disabled.\n"
			"You will need to close and reopen the Room Manager or Practice in order to restart the clock for the affected patient.\n"
			"Please contact NexTech to resolve this issue.");
	);
}

// (j.jones 2008-05-29 10:44) - PLID 27797 - added checkout list
void CRoomManagerDlg::UpdateAllCheckoutListWaitTimes(BOOL bClearColors)
{
	try {

		//for each room, start/stop timers

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCheckoutList->GetFirstRow();
		while(pRow) {
			
			long nAppointmentID = VarLong(pRow->GetValue(rmclcApptID), -1);

			//clearing colors isn't necessary if we're at the end of a requery,
			//otherwise we should make everything black first
			if(bClearColors) {
				//remove the purpose color if there is one
				pRow->PutCellForeColor(rmclcPurpose, dlColorNotSet);
				pRow->PutForeColor(RGB(0,0,0));
				pRow->PutCellForeColor(rmclcWaiting, RGB(0,0,0));
			}

			//UpdateCheckoutListWaitTime will stop the timer, if it exists,
			//will detect if a time needs to be calculated, do so,
			//and start a new timer, if necessary.
			UpdateCheckoutListWaitTime(nAppointmentID);

			pRow = pRow->GetNextRow();
		}

	}NxCatchAll("Error in CRoomManagerDlg::UpdateAllCheckoutListWaitTimes");
}

// (j.jones 2008-05-29 10:44) - PLID 27797 - added checkout list
void CRoomManagerDlg::UpdateCheckoutListWaitTime(long nAppointmentID)
{
	try {

		//UpdateCheckoutListWaitTime will stop the timer, if it exists,
		//will detect if a time needs to be calculated, do so,
		//and start a new timer, if necessary

		KillTimer(nAppointmentID);

		IRowSettingsPtr pRow = m_pCheckoutList->FindByColumn(rmclcApptID, (long)nAppointmentID, 0, FALSE);
		if(pRow == NULL) {
			return;
		}

		// (j.jones 2010-08-31 15:24) - PLID 35012 - color the purpose by the apt. type color,
		// if the entire row is turned red later it will override this color
		if(m_bColorPurposeColumnByType) {
			long nAptTypeColor = VarLong(pRow->GetValue(rmclcAptTypeColor), 0);
			pRow->PutCellForeColor(rmclcPurpose, nAptTypeColor);
		}

		//calculate the minutes the patient has been waiting
		COleDateTime dtNow = COleDateTime::GetCurrentTime() + m_dtOffset;
		COleDateTime dtLast;
		_variant_t var = pRow->GetValue(rmclcTimeLeftRoom);
		if(var.vt == VT_DATE) {
			dtLast = VarDateTime(var);			
		}
		else {
			//if not seen, use check-in time (should be impossible to get here)
			dtLast = VarDateTime(pRow->GetValue(rmclcCheckInTime), VarDateTime(pRow->GetValue(rmclcApptTime)));
		}
		COleDateTimeSpan dtSpan;
		dtSpan = dtNow - dtLast;
		long nMinuteDifference = (dtSpan.GetHours() * 60) + dtSpan.GetMinutes();

		if(nMinuteDifference < 0) {
			//The offset should make this impossible in a real environment,
			//but not difficult to achieve in a developer's locally docked
			//environment. This would mean that the local time plus the
			//offset is still earlier than the "last seen" time, which can
			//really only occur if the server time has been changed to be
			//earlier than the timestamp for "last seen"
			CString strWaitTime = "0:00";
			pRow->PutValue(rmclcWaiting, _bstr_t(strWaitTime));
		}
		else {
			CString strWaitTime;
			strWaitTime.Format("%li:%s%li", dtSpan.GetHours(), dtSpan.GetMinutes() < 10 ? "0" : "", dtSpan.GetMinutes());
			pRow->PutValue(rmclcWaiting, _bstr_t(strWaitTime));
		}

		//use the same MaxWaitTime as the Room list
		if(GetRemotePropertyInt("RoomManagerMaxWaitTime", 5, 0, GetCurrentUserName(), true) <= nMinuteDifference) {
			//if exceeding the maximum wait time, color the whole line red
			//remove the purpose color if there is one
			pRow->PutCellForeColor(rmclcPurpose, dlColorNotSet);
			pRow->PutForeColor(RGB(255,0,0));
		}
		else {
			//just color the time red
			pRow->PutCellForeColor(rmclcWaiting, RGB(255,0,0));
		}

		//now, start a new timer for the next minute since the patient has been waiting
		long nSecondsUntilNextMinute = 60 - dtSpan.GetSeconds();
		SetTimer(nAppointmentID, nSecondsUntilNextMinute * 1000, NULL);

	}NxCatchAllCall("Error in CRoomManagerDlg::UpdateCheckoutListWaitTime",		
		AfxMessageBox("The Room Manager failed to update the wait time for a patient in the Checkout list, and the clock for that patient has been disabled.\n"
			"You will need to close and reopen the Room Manager or Practice in order to restart the clock for the affected patient.\n"
			"Please contact NexTech to resolve this issue.");
	);
}

void CRoomManagerDlg::ExpandCurrentHour(BOOL bMaintainLastHour, BOOL bTryShowNextHour)
{
	try {

		//find the current hour and expand it
		COleDateTime dtNow = COleDateTime::GetCurrentTime() + m_dtOffset;
		long nHour = dtNow.GetHour();
		long nLastHour = nHour - 1;
		long nNextHour = nHour + 1;

		// (j.jones 2006-10-03 12:18) - show the next hour X minutes beforehand,
		// where X is configurable by preferences, and no more than 60
		BOOL bShowNextHour = FALSE;		
		if(bTryShowNextHour) {
			long nNextHourIncrement = GetRemotePropertyInt("RoomManagerExpandMinutesInAdvance", 15, 0, GetCurrentUserName(), true);
			if(dtNow.GetMinute() >= (60 - nNextHourIncrement)) {
				bShowNextHour = TRUE;
			}
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPatientList->GetFirstRow();
		while(pRow) {

			if(bMaintainLastHour) {
				
				//if the previous hour, check and see if any appts. are not checked out,
				//and if so, expand that hour
				if(VarLong(pRow->GetValue(rmplcID), -1) == nLastHour) {

					BOOL bShowLastHour = FALSE;
					NXDATALIST2Lib::IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();
					while(pChildRow != NULL && !bShowLastHour) {

						//if any appointment is not checked out, show that hour
						long nStatusID = VarLong(pChildRow->GetValue(rmplcRoomStatusID), -2);
						if(nStatusID != -1) {
							bShowLastHour = TRUE;
						}

						pChildRow = pChildRow->GetNextRow();
					}

					//if we decided to show it, do so!
					if(bShowLastHour) {

						//first adjust the parent row into view
						m_pPatientList->EnsureRowInView(m_pPatientList->GetLastRow());
						m_pPatientList->EnsureRowInView(pRow);

						//if there are children
						if(pRow->GetFirstChildRow()) {

							//now expand the row
							pRow->PutExpanded(VARIANT_TRUE);

							//now try to show all the children
							m_pPatientList->EnsureRowInView(pRow->GetLastChildRow());
							m_pPatientList->EnsureRowInView(pRow->GetFirstChildRow());
						}
					}
				}
			}

			if(bShowNextHour) {
				
				//if the next hour, expand it
				if(VarLong(pRow->GetValue(rmplcID), -1) == nNextHour) {

					//we show the last row, then this row, so we can advance this row towards the top of the screen
					m_pPatientList->EnsureRowInView(m_pPatientList->GetLastRow());
					m_pPatientList->EnsureRowInView(pRow);

					//if there are children
					if(pRow->GetFirstChildRow()) {

						//if the next hour, expand the row, and try to ensure we can see all the children
						if(VarLong(pRow->GetValue(rmplcID), -1) == nNextHour) {

							pRow->PutExpanded(VARIANT_TRUE);

							m_pPatientList->EnsureRowInView(pRow->GetLastChildRow());
							m_pPatientList->EnsureRowInView(pRow->GetFirstChildRow());
						}
					}
				}
			}

			// (j.jones 2006-10-03 12:21) - try showing the current hour last, because while we attempt
			// to adjust the screen to display the other hours as well, this is the key one that must
			// be adjusted into an ideal position

			//if the current hour, ensure we can see the parent row, even if it has no children
			if(VarLong(pRow->GetValue(rmplcID), -1) == nHour) {

				//we show the last row, then this row, so we can advance this row towards the top of the screen
				m_pPatientList->EnsureRowInView(m_pPatientList->GetLastRow());
				m_pPatientList->EnsureRowInView(pRow);

				//if there are children
				if(pRow->GetFirstChildRow()) {

					//if the current hour, expand the row, and try to ensure we can see all the children
					if(VarLong(pRow->GetValue(rmplcID), -1) == nHour) {

						pRow->PutExpanded(VARIANT_TRUE);

						m_pPatientList->EnsureRowInView(pRow->GetLastChildRow());
						m_pPatientList->EnsureRowInView(pRow->GetFirstChildRow());
					}
				}
			}

			pRow = pRow->GetNextRow();
		}

		// (j.jones 2006-10-23 12:10) - PLID 23169 - be able to expand a given appointment
		ShowAppointment(m_nPendingShowAppointment);
		m_nPendingShowAppointment = -1;

	}NxCatchAll("Error in CRoomManagerDlg::ExpandCurrentHour");
}

void CRoomManagerDlg::CheckAutoCollapseFinishedHours()
{
	try {

		//find the previous hours before "now", and collapse it if it is
		//expanded and all the appts. are checked out

		//find the current hour
		COleDateTime dtNow = COleDateTime::GetCurrentTime() + m_dtOffset;
		long nHour = dtNow.GetHour();

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pPatientList->GetFirstRow();
		while(pRow) {

			//quit if we have reached the current hour
			if(VarLong(pRow->GetValue(rmplcID), -1) >= nHour) {
				return;
			}
				
			//if a previous hour, check and see if expanded, and if so,
			//if all appts. are not checked out. If all this is true,
			//then collapse that hour
			if(VarLong(pRow->GetValue(rmplcID), -1) < nHour
				&& pRow->GetFirstChildRow() != NULL
				&& pRow->GetExpanded()) {

				BOOL bCollapseLastHour = TRUE;
				NXDATALIST2Lib::IRowSettingsPtr pChildRow = pRow->GetFirstChildRow();
				while(pChildRow != NULL && bCollapseLastHour) {

					//if any appointment is not checked out, do not collapse that hour
					long nStatusID = VarLong(pChildRow->GetValue(rmplcRoomStatusID), -2);
					if(nStatusID != -1) {
						bCollapseLastHour = FALSE;
					}

					pChildRow = pChildRow->GetNextRow();
				}

				//if we decided to collapse it, do so!
				if(bCollapseLastHour) {
					
					//collapse the row
					pRow->PutExpanded(VARIANT_FALSE);
				}
			}

			pRow = pRow->GetNextRow();
		}

	}NxCatchAll("Error in CRoomManagerDlg::CheckAutoCollapseFinishedHours");
}

void CRoomManagerDlg::ResetHourTimer()
{
	try {

		KillTimer(m_nTimerHourExpansion);

		// (j.jones 2006-10-03 12:44) - use the preference to to determine how long
		// before the next hour we need to expand the appts., then calculate that time
		long nNextHourMinuteIncrement = GetRemotePropertyInt("RoomManagerExpandMinutesInAdvance", 15, 0, GetCurrentUserName(), true);
		long nNextHourSecondIncrement = nNextHourMinuteIncrement * 60;

		COleDateTime dtNow = COleDateTime::GetCurrentTime() + m_dtOffset;
		long nSecondsToNextHour = ((60 - dtNow.GetMinute()) * 60) - dtNow.GetSecond();
		long nTimerCalc = 320000;
		if(nSecondsToNextHour <= nNextHourSecondIncrement) {
			//we are not in the next hour but should have already shown it, so the time will be:
			//(seconds to next hour + seconds in an hour - expand seconds before next hour) * (milliseconds per second)
			nTimerCalc = (nSecondsToNextHour + 3600 - nNextHourSecondIncrement) * 1000;
		}
		else {
			//(seconds to next hour - expand seconds before next hour) * (milliseconds per second)
			nTimerCalc = (nSecondsToNextHour - nNextHourSecondIncrement) * 1000;
		}
		m_nTimerHourExpansion = SetTimer(m_nTimerHourExpansion, nTimerCalc, NULL);

	}NxCatchAll("Error in CRoomManagerDlg::ResetHourTimer");
}

void CRoomManagerDlg::CalculateLocalTimeOffset()
{
	try {

		//calculate the difference between the local time and the server time,
		//and store the offset in m_dtOffset

		KillTimer(m_nTimerHourlyServerTimeCheck);

		_RecordsetPtr rs = CreateRecordset("SELECT GetDate() AS Now");
		if(!rs->eof) {
			COleDateTime dtServerNow = AdoFldDateTime(rs, "Now");
			COleDateTime dtLocalNow = COleDateTime::GetCurrentTime();

			m_dtOffset = dtServerNow - dtLocalNow;
			
			//now we can add m_dtOffset to COleDateTime::GetCurrentTime() to get a closer
			//approximation of what the server time actually is, without having to ask
			//the server
		}
		rs->Close();

		//check again in 60 minutes
		m_nTimerHourlyServerTimeCheck = SetTimer(m_nTimerHourlyServerTimeCheck, 3600000, NULL);

	}NxCatchAll("Error in CRoomManagerDlg::CalculateLocalTimeOffset");
}

void CRoomManagerDlg::FireTimeChanged()
{
	//if the local time changed, recalculate our timers
	
	//first reset the hour timer and recalculate the hour expansion
	KillTimer(m_nTimerHourExpansion);

	//compare our server-to-local time offset again, hourly
	CalculateLocalTimeOffset();
	
	ExpandCurrentHour(TRUE, TRUE);

	CheckAutoCollapseFinishedHours();

	//now start the hour timer up again
	ResetHourTimer();

	//now update all wait times
	UpdateAllRoomWaitTimes(TRUE);
	UpdateAllWaitingAreaWaitTimes(TRUE);
}

void CRoomManagerDlg::ReflectChangedRoomAppointment(CTableCheckerDetails* pDetails)
{
	try {

		// (j.jones 2014-08-05 10:34) - PLID 63167 - this now uses an enumeration
		long nRoomAppointmentID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::RoomAppointments_DetailIndex::radiRoomAppointmentID), -1);
		long nRoomID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::RoomAppointments_DetailIndex::radiRoomID), -1);
		long nAppointmentID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::RoomAppointments_DetailIndex::radiAppointmentID), -1);
		long nStatusID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::RoomAppointments_DetailIndex::radiStatus), -1);

		_variant_t varNull;
		varNull.vt = VT_NULL;

		//first find the Room row
		IRowSettingsPtr pRoomRow = m_pRoomList->FindByColumn(rmrlcRoomID, nRoomID, 0, FALSE);

		if(pRoomRow == NULL) {
			// (j.jones 2010-12-02 10:32) - PLID 38597 - is this perhaps a waiting room instead?
			// search our combo of available waiting rooms to see if it is a waiting room
			// for the currently selected location
			if(m_pWaitingRoomCombo->FindByColumn(wrccID, nRoomID, 0, FALSE)) {
				//this is a waiting room, so call ReflectChangedWaitingRoomAppointment() to process this change
				ReflectChangedWaitingRoomAppointment(nRoomAppointmentID, nRoomID, nAppointmentID, nStatusID);				
				return;
			}
			else {
				// (j.jones 2009-08-04 09:12) - PLID 24600 - There are now legitimate reasons why the room would not
				// exist - if it is for a different location. If the room isn't in our view, do nothing.
				return;
			}
		}

		//TES 8/29/2008 - PLID 26534 - This all just got more complicated.  There can now be more than one row for the same
		// room, and we may need to add/remove rows as a result of this change.  So, we need to loop through ALL the rows
		// for this room, and process them accordingly.

		//TES 8/29/2008 - PLID 26534 - Various variables to keep track of what we've done.
		IRowSettingsPtr pClearedRow = NULL;
		long nRowCount = 0;
		bool bChangeMade = false;
		//TES 8/29/2008 - PLID 26534 - Loop until we either get NULL, or the row that we found the first time through (because
		// the datalist's FindByColumn() function wraps around).
		IRowSettingsPtr pFirstFoundRow = NULL;
		while(pRoomRow != NULL && pRoomRow != pFirstFoundRow) {
			//TES 8/29/2008 - PLID 26534 - Increment the number of room rows, and track if this is the first one.
			nRowCount++;
			if(pFirstFoundRow == NULL) {
				pFirstFoundRow = pRoomRow;
			}

			//is the room currently empty, and needs filled?
			// (j.jones 2010-10-12 11:28) - PLID 39960 - this recordset is not necessary if the status is 0,
			// which means the appointment is in the checkout list
			if(nStatusID != -1 && nStatusID != 0 && pRoomRow->GetValue(rmrlcRoomAppointmentID).vt != VT_I4) {

				//the room is empty, so we need to populate it with this room appointment's details

				// (j.armen 2012-03-05 11:20) - PLID 48555 - Select the highest prority recall status for this patient
				// (a.walling 2013-12-12 16:51) - PLID 60005 - RoomManager only needs a smaller subset of the entire recall query - SelectRecallStatusesPastDueOrNeeded
				CSqlFragment sqlRecall("(SELECT MAX(RecallStatusID) AS RecallStatusID FROM ({SQL}) SubQ WHERE SubQ.PatientID = ApptQ.PatientID)", RecallUtils::SelectRecallStatusesPastDueOrNeeded());

				// (j.jones 2008-11-12 16:56) - PLID 28035 - added purpose text
				// (d.thompson 2009-07-10) - PLID 26860 - Added physical arrival time
				// (j.jones 2010-08-27 10:31) - PLID 36975 - the patient name is now calculated by a preference	
				// (d.lange 2010-08-30 09:11) - PLID 39431 - added provider column
				// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor
				// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource
				// (c.haag 2010-10-26 10:07) - PLID 39199 - Changed how we determine whether the appt should be in the waiting area (added WaitingArea)
				// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
				_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PatientID, PatientName, {SQL} AS RecallStatusID, "
					"CASE WHEN PurposeName <> '' THEN TypeName + ' - ' + PurposeName ELSE TypeName END AS Purpose, AptTypeColor, "
					"StartTime, LastArrivalTime, CheckInTime, TimeLastSeenByPerson, WithPerson, LastUpdateUserID, Username, Provider,Resources "
					"FROM (SELECT AppointmentsT.PatientID AS PatientID, "
						"{SQL} AS PatientName, "
						"AppointmentsT.StartTime, "
						"(SELECT MAX(TimeStamp) FROM AptShowStateHistoryT WHERE AptShowStateHistoryT.AppointmentID = AppointmentsT.ID AND ShowStateID IN (SELECT ID FROM AptShowStateT WHERE WaitingArea = CONVERT(BIT,1))) AS LastArrivalTime, "
						"RoomAppointmentsT.CheckInTime, "
						""
						"(SELECT Min(UpdateTime) AS FirstTimeWithoutPerson FROM RoomAppointmentHistoryT "
						"	WHERE StatusID IN (SELECT ID FROM RoomStatusT WHERE WithPerson = 0) "
						"	AND RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID "
						"	AND UpdateTime > (SELECT Max(UpdateTime) AS LastStartTimeWithPerson FROM RoomAppointmentHistoryT "
						"	WHERE StatusID IN (SELECT ID FROM RoomStatusT WHERE WithPerson = 1) "
						"	AND RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID)) "
						"AS TimeLastSeenByPerson, "
						""
						"RoomStatusT.WithPerson, RoomAppointmentsT.LastUpdateUserID, "
						"UsersT.Username, "
						"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, "
						"AptTypeT.Name AS TypeName, CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS AptTypeColor, "
						"Provider.Last + ', ' + Provider.First + ' ' + Provider.Middle AS Provider, "
						" dbo.GetResourceString(AppointmentsT.ID) AS Resources "
						"FROM RoomsT "
						"INNER JOIN (SELECT * FROM RoomAppointmentsT WHERE StatusID NOT IN (0, -1)) AS RoomAppointmentsT ON RoomsT.ID = RoomAppointmentsT.RoomID "
						"INNER JOIN AppointmentsT ON RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
						"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
						"INNER JOIN RoomStatusT ON RoomAppointmentsT.StatusID = RoomStatusT.ID "
						"INNER JOIN UsersT ON RoomAppointmentsT.LastUpdateUserID = UsersT.PersonID "
						"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
						"LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
						"LEFT JOIN PersonT AS Provider ON PatientsT.MainPhysician = Provider.ID "
						"WHERE RoomAppointmentsT.ID = {INT} "
					") AS ApptQ", sqlRecall, GetPatientNameSqlFormat(), nRoomAppointmentID);
				if(rs->eof) {
					// (j.jones 2010-10-12 11:28) - PLID 39960 - This is possible, but should be very rare,
					// as this should only occur if the database values have changed since the tablechecker
					// was sent, and the room appt. status is no longer accurate. We don't know what is correct
					// any longer, so we must requery the lists.
					RequeryAll(FALSE);
					// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
					Flash();
					return;
				}

				pRoomRow->PutValue(rmrlcRoomAppointmentID, (long)nRoomAppointmentID);
				pRoomRow->PutValue(rmrlcApptID, (long)nAppointmentID);
				pRoomRow->PutValue(rmrlcPatientID, rs->Fields->Item["PatientID"]->Value);
				pRoomRow->PutValue(rmrlcPatientName, rs->Fields->Item["PatientName"]->Value);
				// (j.jones 2008-11-12 16:53) - PLID 28035 - added purpose field
				pRoomRow->PutValue(rmrlcPurpose, rs->Fields->Item["Purpose"]->Value);
				// (j.jones 2010-08-27 09:39) - PLID 39774 - added appt. time
				pRoomRow->PutValue(rmrlcApptTime, rs->Fields->Item["StartTime"]->Value);
				// (d.lange 2010-08-30 09:11) - PLID 39431 - added provider column
				pRoomRow->PutValue(rmrlcProvider, rs->Fields->Item["Provider"]->Value);
				// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource 
				pRoomRow->PutValue(rmrlcResources, rs->Fields->Item["Resources"]->Value);
				
				// (d.thompson 2009-07-10) - PLID 26860 - added physical arrival time
				pRoomRow->PutValue(rmrlcArrivalTime, rs->Fields->Item["LastArrivalTime"]->Value);
				pRoomRow->PutValue(rmrlcCheckInTime, rs->Fields->Item["CheckInTime"]->Value);
				pRoomRow->PutValue(rmrlcTimeLastSeen, rs->Fields->Item["TimeLastSeenByPerson"]->Value);
				pRoomRow->PutValue(rmrlcStatusID, (long)nStatusID);
				pRoomRow->PutValue(rmrlcWithPerson, rs->Fields->Item["WithPerson"]->Value);
				pRoomRow->PutValue(rmrlcWaiting, AdoFldBool(rs, "WithPerson", FALSE) ? _bstr_t("No") : _bstr_t("''"));
				pRoomRow->PutValue(rmrlcLastUserID, rs->Fields->Item["LastUpdateUserID"]->Value);
				pRoomRow->PutValue(rmrlcLastUserName, rs->Fields->Item["Username"]->Value);
				// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
				pRoomRow->PutValue(rmrlcAptTypeColor, rs->Fields->Item["AptTypeColor"]->Value);
				if(m_bColorPurposeColumnByType) {
					pRoomRow->PutCellForeColor(rmrlcPurpose, AdoFldLong(rs, "AptTypeColor", 0));
				}
				// (d.lange 2010-11-29 11:54) - PLID 40295 - Added Preview EMN column
				pRoomRow->PutValue(rmrlcPreviewEMN, (long)m_hIconPreview);

				// (j.armen 2012-03-05 11:19) - PLID 48555 - Get the RecallStatusID and Ensure the Flag Column to make sure that the column is set properly
				pRoomRow->PutValue(rmrlcRecallStatusID, rs->Fields->Item["RecallStatusID"]->Value);
				EnsureRecallFlagColumn(m_pRoomList, rmrlcRecallStatusID, rmrlcRecallStatus, "ShowRoomManagerRoomsColumnRecall");

				rs->Close();

				//color-code the patient in today's appt. list
				IRowSettingsPtr pPatientRow = m_pPatientList->FindByColumn(rmplcApptID, nAppointmentID, 0, FALSE);
				if(pPatientRow) {
					pPatientRow->PutValue(rmplcRoomStatusID, nStatusID);

					// (j.jones 2008-05-30 09:04) - PLID 27797 - color appropriately
					ColorApptRow(pPatientRow);
				}

				// (j.jones 2007-02-07 09:56) - PLID 24595 - if the appt. was in the waiting area,
				// remove it
				IRowSettingsPtr pWaitingAreaRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, nAppointmentID, 0, FALSE);
				if(pWaitingAreaRow != NULL) {
					m_pWaitingAreaList->RemoveRow(pWaitingAreaRow);
					
				}

				// (j.jones 2008-05-29 12:07) - PLID 27797 - if the appt. was in the checkout list, remove it
				IRowSettingsPtr pCheckoutRow = m_pCheckoutList->FindByColumn(rmclcApptID, nAppointmentID, 0, FALSE);
				if(pCheckoutRow != NULL) {
					m_pCheckoutList->RemoveRow(pCheckoutRow);
					
				}

				//fire timers, if necessary
				//remove the purpose color if there is one
				pRoomRow->PutCellForeColor(rmrlcPurpose, dlColorNotSet);
				pRoomRow->PutForeColor(RGB(0,0,0));
				pRoomRow->PutCellForeColor(rmrlcWaiting, RGB(0,0,0));
				UpdateRoomWaitTime(nAppointmentID);

				//TES 8/29/2008 - PLID 26534 - Don't return (we need to keep looping) but track that we did actually make
				// a change as a result of this refresh.
				bChangeMade = true;
				// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
				/*Flash();
				return;*/
			}

			if(pRoomRow->GetValue(rmrlcRoomAppointmentID).vt == VT_I4 && VarLong(pRoomRow->GetValue(rmrlcRoomAppointmentID)) != nRoomAppointmentID) {
				//TES 8/29/2008 - PLID 26534 - This is now possible, since you can have more than one patient per room.  So,
				// just skip over this row.

				/*//it should be impossible for a room to receive a network message if the room
				//has another room appointment ID in it.
				//but if we get it, we have no idea what's going on, so just requery
				RequeryAll(FALSE);

				// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
				Flash();
				return;*/
			}
			else {
				//ok, if we get here, the room is filled with the room appointment being modified
				// (s.dhole 2010-11-16 15:37) - PLID 39200 Since we insert row at client sitr need to check sttus as -3
				//if(nStatusID == -1  || nStatusID == -3) {
				if(nStatusID == -1  ) {
					//the room is being checked out, so clear its data

					KillTimer(nRoomAppointmentID);

					// (j.jones 2008-05-29 12:07) - PLID 27797 - if the appt. was in the checkout list, remove it
					IRowSettingsPtr pCheckoutRow = m_pCheckoutList->FindByColumn(rmclcApptID, nAppointmentID, 0, FALSE);
					if(pCheckoutRow != NULL) {
						m_pCheckoutList->RemoveRow(pCheckoutRow);
					}

					//clear the room row only if it is filled with our room appointment
					if(pRoomRow->GetValue(rmrlcRoomAppointmentID).vt == VT_I4 && VarLong(pRoomRow->GetValue(rmrlcRoomAppointmentID)) == nRoomAppointmentID) {
						//remove the purpose color if there is one
						pRoomRow->PutCellForeColor(rmrlcPurpose, dlColorNotSet);
						pRoomRow->PutForeColor(RGB(0,0,0));
						pRoomRow->PutCellForeColor(rmrlcWaiting, RGB(0,0,0));

						pRoomRow->PutValue(rmrlcRoomAppointmentID, varNull);
						pRoomRow->PutValue(rmrlcApptID, varNull);
						pRoomRow->PutValue(rmrlcPatientID, varNull);
						pRoomRow->PutValue(rmrlcPatientName, varNull);
						// (j.jones 2008-11-12 16:53) - PLID 28035 - added purpose field
						pRoomRow->PutValue(rmrlcPurpose, varNull);
						// (j.jones 2010-08-27 09:39) - PLID 39774 - added appt. time
						pRoomRow->PutValue(rmrlcApptTime, varNull);
						// (d.lange 2010-08-30 09:13) - PLID 39431 - added provider column
						pRoomRow->PutValue(rmrlcProvider, varNull);
						// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource 
						pRoomRow->PutValue(rmrlcResources, varNull);
						// (d.thompson 2009-07-10) - PLID 26860 - added physical arrival time
						pRoomRow->PutValue(rmrlcArrivalTime, varNull);
						pRoomRow->PutValue(rmrlcCheckInTime, varNull);
						pRoomRow->PutValue(rmrlcTimeLastSeen, varNull);
						pRoomRow->PutValue(rmrlcStatusID, varNull);
						pRoomRow->PutValue(rmrlcWithPerson, varNull);
						pRoomRow->PutValue(rmrlcWaiting, varNull);
						pRoomRow->PutValue(rmrlcLastUserID, varNull);
						pRoomRow->PutValue(rmrlcLastUserName, varNull);
						// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
						pRoomRow->PutValue(rmrlcAptTypeColor, (long)RGB(0,0,0));
						pRoomRow->PutCellForeColor(rmrlcPurpose, (long)RGB(0,0,0));
						// (d.lange 2010-11-29 12:53) - PLID 40295 - added Preview EMNs column
						pRoomRow->PutValue(rmrlcPreviewEMN, varNull);

						// (j.armen 2012-03-05 11:19) - PLID 48555 - Get the RecallStatusID and Ensure the Flag Column to make sure that the column is set properly
						pRoomRow->PutValue(rmrlcRecallStatusID, varNull);
						EnsureRecallFlagColumn(m_pRoomList, rmrlcRecallStatusID, rmrlcRecallStatus, "ShowRoomManagerRoomsColumnRecall");
						
						//TES 8/29/2008 - PLID 26534 - If there is more than one row for this room, then we'll want to remove this
						// row, not just clear it out, but we won't know that until we're done looping.  So, track that we cleared
						// this row (it shouldn't be possible to clear multiple rows while processing a single change).
						ASSERT(pClearedRow == NULL);
						pClearedRow = pRoomRow;
					}

					//color-code the patient in today's appt. list
					IRowSettingsPtr pPatientRow = m_pPatientList->FindByColumn(rmplcApptID, nAppointmentID, 0, FALSE);
					if(pPatientRow) {
						pPatientRow->PutValue(rmplcRoomStatusID, nStatusID);

						// (j.jones 2008-05-30 09:04) - PLID 27797 - color appropriately
						ColorApptRow(pPatientRow);
					}

					//and potentially collapse completed hours
					CheckAutoCollapseFinishedHours();

					// (j.jones 2008-05-29 10:16) - PLID 27797 - removed waiting area code,
					// the patient doesn't get put back into the waiting area anymore
				}
				// (j.jones 2008-05-29 10:15) - PLID 27797 - supported the 'ready to check out' status
				else if(nStatusID == 0) {	//Ready To Check Out

					//first clear the room
					KillTimer(nRoomAppointmentID);

					//remove the purpose color if there is one
					pRoomRow->PutCellForeColor(rmrlcPurpose, dlColorNotSet);
					pRoomRow->PutForeColor(RGB(0,0,0));
					pRoomRow->PutCellForeColor(rmrlcWaiting, RGB(0,0,0));

					pRoomRow->PutValue(rmrlcRoomAppointmentID, varNull);
					pRoomRow->PutValue(rmrlcApptID, varNull);
					pRoomRow->PutValue(rmrlcPatientID, varNull);
					pRoomRow->PutValue(rmrlcPatientName, varNull);
					// (j.jones 2008-11-12 16:53) - PLID 28035 - added purpose field
					pRoomRow->PutValue(rmrlcPurpose, varNull);
					// (j.jones 2010-08-27 09:39) - PLID 39774 - added appt. time
					pRoomRow->PutValue(rmrlcApptTime, varNull);
					// (d.lange 2010-08-30 09:13) - PLID 39431 - added provider column
					pRoomRow->PutValue(rmrlcProvider, varNull);
					// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource 
					pRoomRow->PutValue(rmrlcResources, varNull);
					// (d.thompson 2009-07-10) - PLID 26860 - added physical arrival time
					pRoomRow->PutValue(rmrlcArrivalTime, varNull);
					pRoomRow->PutValue(rmrlcCheckInTime, varNull);
					pRoomRow->PutValue(rmrlcTimeLastSeen, varNull);
					pRoomRow->PutValue(rmrlcStatusID, varNull);
					pRoomRow->PutValue(rmrlcWithPerson, varNull);
					pRoomRow->PutValue(rmrlcWaiting, varNull);
					pRoomRow->PutValue(rmrlcLastUserID, varNull);
					pRoomRow->PutValue(rmrlcLastUserName, varNull);
					// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
					pRoomRow->PutValue(rmrlcAptTypeColor, (long)RGB(0,0,0));
					pRoomRow->PutCellForeColor(rmrlcPurpose, (long)RGB(0,0,0));
					// (d.lange 2010-11-29 12:53) - PLID 40295 - added Preview EMNs column
					pRoomRow->PutValue(rmrlcPreviewEMN, varNull);

					// (j.armen 2012-03-05 11:19) - PLID 48555 - Get the RecallStatusID and Ensure the Flag Column to make sure that the column is set properly
					pRoomRow->PutValue(rmrlcRecallStatusID, varNull);
					EnsureRecallFlagColumn(m_pRoomList, rmrlcRecallStatusID, rmrlcRecallStatus, "ShowRoomManagerRoomsColumnRecall");

					//TES 8/29/2008 - PLID 26534 - If there is more than one row for this room, then we'll want to remove this
					// row, not just clear it out, but we won't know that until we're done looping.  So, track that we cleared
					// this row (it shouldn't be possible to clear multiple rows while processing a single change).
					ASSERT(pClearedRow == NULL);
					pClearedRow = pRoomRow;

					//if the appt. was in the waiting area, remove it
					IRowSettingsPtr pWaitingAreaRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, nAppointmentID, 0, FALSE);
					if(pWaitingAreaRow != NULL) {
						m_pWaitingAreaList->RemoveRow(pWaitingAreaRow);
					}

					//now update the checkout list

					//if it exists, remove it, we may re-add it
					IRowSettingsPtr pCheckoutRow = m_pCheckoutList->FindByColumn(rmclcApptID, nAppointmentID, 0, FALSE);
					if(pCheckoutRow != NULL) {
						m_pCheckoutList->RemoveRow(pCheckoutRow);
					}
					
					//utilize the resource filter
					CString strResourceFilter, strResourceIDs;
					strResourceIDs = m_strResourceIDs;
					strResourceIDs.TrimRight();
					if(!strResourceIDs.IsEmpty()) {
						strResourceIDs.Replace(" ", ",");
						strResourceFilter.Format(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN (%s))", strResourceIDs);
					}

					// (j.jones 2009-08-04 08:55) - PLID 24600 - added location filter
					long nLocationID = -1;
					{
						IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
						if(pRow == NULL) {
							nLocationID = GetCurrentLocationID();
							m_pLocationCombo->SetSelByColumn(lccID, nLocationID);			
						}
						else {
							nLocationID = VarLong(pRow->GetValue(lccID));
						}
					}

					// (j.armen 2012-03-05 11:20) - PLID 48555 - Select the highest prority recall status for this patient
					// (a.walling 2013-12-12 16:51) - PLID 60005 - RoomManager only needs a smaller subset of the entire recall query - SelectRecallStatusesPastDueOrNeeded
					CSqlFragment sqlRecall("(SELECT MAX(RecallStatusID) AS RecallStatusID FROM ({SQL}) SubQ WHERE SubQ.PatientID = ApptQ.PatientID)", RecallUtils::SelectRecallStatusesPastDueOrNeeded());

					//copied/modified from RequeryCheckoutList
					// (j.jones 2008-11-12 16:42) - PLID 28035 - added purpose text
					// (j.jones 2010-08-27 10:31) - PLID 36975 - the patient name is now calculated by a preference	
					// (d.lange 2010-08-31 10:21) - PLID 39431 - added provider column
					// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor
					// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource 
					// (c.haag 2010-10-26 10:07) - PLID 39199 - Changed how we determine whether the appt should be in the waiting area (added WaitingArea)
					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT RoomAppointmentID, AppointmentID, "
						"PatientID, PatientName, {SQL} AS RecallStatusID, StartTime, TimeMarkedIn, TimeLeftRoom, LastUpdateUserID, UserName, "
						"CASE WHEN PurposeName <> '' THEN TypeName + ' - ' + PurposeName ELSE TypeName END AS Purpose, AptTypeColor, Provider,Resources " 
						""
						"FROM (SELECT RoomAppointmentsT.ID AS RoomAppointmentID, AppointmentsT.ID AS AppointmentID, "
						"AppointmentsT.PatientID AS PatientID, {SQL} AS PatientName, "
						"AppointmentsT.StartTime, TimeMarkedIn, "
						""				
						"	(SELECT Max(UpdateTime) AS TimeLeftRoom FROM RoomAppointmentHistoryT "
						"	WHERE RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID) "
						"	AS TimeLeftRoom, "
						""
						"RoomAppointmentsT.LastUpdateUserID, UsersT.Username, "
						"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, "
						"AptTypeT.Name AS TypeName, CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS AptTypeColor, "
						"Provider.Last + ', ' + Provider.First + ' ' + Provider.Middle AS Provider, "
						" dbo.GetResourceString(AppointmentsT.ID) AS Resources  "
						"FROM AppointmentsT "
						"INNER JOIN RoomAppointmentsT ON AppointmentsT.ID = RoomAppointmentsT.AppointmentID "
						"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
						"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
						"LEFT JOIN (SELECT AppointmentID, Max(TimeStamp) AS TimeMarkedIn "
						"	FROM AptShowStateHistoryT "
						"	LEFT JOIN AptShowStateT on AptShowStateT.ID = AptShowStateHistoryT.ShowStateID "
						"	WHERE AptShowStateT.WaitingArea = CONVERT(BIT,1) GROUP BY AppointmentID, AptShowStateT.WaitingArea) "
						"AS AptShowStateHistoryQ ON AppointmentsT.ID = AptShowStateHistoryQ.AppointmentID "
						"LEFT JOIN UsersT ON RoomAppointmentsT.LastUpdateUserID = UsersT.PersonID "
						"LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
						"LEFT JOIN PersonT AS Provider ON PatientsT.MainPhysician = Provider.ID "
						//not cancelled
						"WHERE AppointmentsT.Status <> 4 "
						//uses our filtered location
						"AND AppointmentsT.LocationID = {INT} "
						//on today's date
						"AND Convert(datetime, Convert(nvarchar, AppointmentsT.Date, 1)) = Convert(datetime, Convert(nvarchar, GetDate(), 1)) "
						//currently marked 'Ready To Check Out'
						"AND RoomAppointmentsT.StatusID = 0 "
						//optionally filtered by resource
						"{CONST_STRING}"
						//and lastly filtered by this appt. ID
						"AND AppointmentsT.ID = {INT}) AS ApptQ", 
						sqlRecall, GetPatientNameSqlFormat(), nLocationID, strResourceFilter, nAppointmentID);

					//if not empty, re-add to the list
					if(!rs->eof) {
						IRowSettingsPtr pNewRow = m_pCheckoutList->GetNewRow();
						
						pNewRow->PutValue(rmclcRoomAppointmentID, rs->Fields->Item["RoomAppointmentID"]->Value);
						pNewRow->PutValue(rmclcApptID, (long)nAppointmentID);
						pNewRow->PutValue(rmclcPatientID, rs->Fields->Item["PatientID"]->Value);
						pNewRow->PutValue(rmclcPatientName, rs->Fields->Item["PatientName"]->Value);
						// (j.jones 2008-11-12 17:15) - PLID 28035 - added purpose field
						pNewRow->PutValue(rmclcPurpose, rs->Fields->Item["Purpose"]->Value);
						pNewRow->PutValue(rmclcApptTime, rs->Fields->Item["StartTime"]->Value);
						pNewRow->PutValue(rmclcCheckInTime, rs->Fields->Item["TimeMarkedIn"]->Value);
						pNewRow->PutValue(rmclcTimeLeftRoom, rs->Fields->Item["TimeLeftRoom"]->Value);
						pNewRow->PutValue(rmclcWaiting, _bstr_t(""));
						// (d.lange 2010-08-31 10:22) - PLID 39431 - added provider column
						pNewRow->PutValue(rmclcProvider, rs->Fields->Item["Provider"]->Value);
						// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource
						pNewRow->PutValue(rmclcResources , rs->Fields->Item["Resources"]->Value);
						pNewRow->PutValue(rmclcLastUserID, rs->Fields->Item["LastUpdateUserID"]->Value);
						pNewRow->PutValue(rmclcLastUserName, rs->Fields->Item["Username"]->Value);
						// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
						pNewRow->PutValue(rmclcAptTypeColor, rs->Fields->Item["AptTypeColor"]->Value);
						if(m_bColorPurposeColumnByType) {
							pNewRow->PutCellForeColor(rmclcPurpose, AdoFldLong(rs, "AptTypeColor", 0));
						}
						// (d.lange 2010-11-29 12:53) - PLID 40295 - added Preview EMNs column
						pNewRow->PutValue(rmclcPreviewEMN, (long)m_hIconPreview);

						// (j.armen 2012-03-05 11:19) - PLID 48555 - Get the RecallStatusID and Ensure the Flag Column to make sure that the column is set properly
						pNewRow->PutValue(rmclcRecallStatusID, rs->Fields->Item["RecallStatusID"]->Value);

						m_pCheckoutList->AddRowSorted(pNewRow,NULL);

						EnsureRecallFlagColumn(m_pCheckoutList, rmclcRecallStatusID, rmclcRecallStatus, "ShowRoomManagerCheckoutColumnRecall");

						UpdateCheckoutListWaitTime(nAppointmentID);

						//TES 8/29/2008 - PLID 26534 - We don't want to do this until the end.
						// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
						//Flash();
					}
					rs->Close();

					//update the patient in today's appt. list
					IRowSettingsPtr pPatientRow = m_pPatientList->FindByColumn(rmplcApptID, nAppointmentID, 0, FALSE);
					if(pPatientRow) {
						pPatientRow->PutValue(rmplcRoomStatusID, nStatusID);				
						
						// (j.jones 2008-05-30 09:04) - PLID 27797 - color appropriately
						ColorApptRow(pPatientRow);
					}
				}
				else {
					//the status changed to something other than being checked out or ready to check out
					//so update the status
					pRoomRow->PutValue(rmrlcStatusID, (long)nStatusID);

					// (j.jones 2008-05-29 12:07) - PLID 27797 - if the appt. was in the checkout list, remove it
					IRowSettingsPtr pCheckoutRow = m_pCheckoutList->FindByColumn(rmclcApptID, nAppointmentID, 0, FALSE);
					if(pCheckoutRow != NULL) {
						m_pCheckoutList->RemoveRow(pCheckoutRow);
					}

					//is the patient "with a person"?

					// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
					// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager	
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT "
							"(SELECT Min(UpdateTime) AS FirstTimeWithoutPerson FROM RoomAppointmentHistoryT "
							"	WHERE StatusID IN (SELECT ID FROM RoomStatusT WHERE WithPerson = 0) "
							"	AND RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID "
							"	AND UpdateTime > (SELECT Max(UpdateTime) AS LastStartTimeWithPerson FROM RoomAppointmentHistoryT "
							"	WHERE StatusID IN (SELECT ID FROM RoomStatusT WHERE WithPerson = 1) "
							"	AND RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID)) "
							"AS TimeLastSeenByPerson, "
							""
							"RoomStatusT.WithPerson, RoomAppointmentsT.LastUpdateUserID, "
							"UsersT.Username "
							"FROM RoomsT "
							"INNER JOIN (SELECT * FROM RoomAppointmentsT WHERE StatusID NOT IN (0, -1)) AS RoomAppointmentsT ON RoomsT.ID = RoomAppointmentsT.RoomID "
							"INNER JOIN AppointmentsT ON RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
							"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
							"INNER JOIN RoomStatusT ON RoomAppointmentsT.StatusID = RoomStatusT.ID "
							"INNER JOIN UsersT ON RoomAppointmentsT.LastUpdateUserID = UsersT.PersonID "
							"WHERE RoomAppointmentsT.ID = {INT}", nRoomAppointmentID);
					if(rs->eof) {
						//shouldn't be possible!
						ASSERT(FALSE);
						RequeryAll(FALSE);

						// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
						Flash();
						return;
					}

					pRoomRow->PutValue(rmrlcApptID, (long)nAppointmentID);
					pRoomRow->PutValue(rmrlcTimeLastSeen, rs->Fields->Item["TimeLastSeenByPerson"]->Value);
					pRoomRow->PutValue(rmrlcStatusID, (long)nStatusID);
					pRoomRow->PutValue(rmrlcWithPerson, rs->Fields->Item["WithPerson"]->Value);
					pRoomRow->PutValue(rmrlcWaiting, AdoFldBool(rs, "WithPerson", FALSE) ? _bstr_t("No") : _bstr_t("''"));
					pRoomRow->PutValue(rmrlcLastUserID, rs->Fields->Item["LastUpdateUserID"]->Value);
					pRoomRow->PutValue(rmrlcLastUserName, rs->Fields->Item["Username"]->Value);

					rs->Close();

					//color-code the patient in today's appt. list
					IRowSettingsPtr pPatientRow = m_pPatientList->FindByColumn(rmplcApptID, nAppointmentID, 0, FALSE);
					if(pPatientRow) {
						pPatientRow->PutValue(rmplcRoomStatusID, nStatusID);

						// (j.jones 2008-05-30 09:04) - PLID 27797 - color appropriately
						ColorApptRow(pPatientRow);
					}

					//fire the waiting timers
					//remove the purpose color if there is one
					pRoomRow->PutCellForeColor(rmrlcPurpose, dlColorNotSet);
					pRoomRow->PutForeColor(RGB(0,0,0));
					pRoomRow->PutCellForeColor(rmrlcWaiting, RGB(0,0,0));
					UpdateRoomWaitTime(nAppointmentID);

					//TES 8/29/2008 - PLID 26534 - Don't do this until the end.
					// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
					//Flash();
				}
				bChangeMade = true;
			}
			//TES 8/29/2008 - PLID 26534 - Now, keep searching for this room ID, starting with the next row (if any).
			pRoomRow = pRoomRow->GetNextRow();
			if(pRoomRow) {
				pRoomRow = m_pRoomList->FindByColumn(rmrlcRoomID, nRoomID, pRoomRow, FALSE);
			}
		}
		//TES 8/29/2008 - PLID 26534 - Now, have we processed this change yet?
		if(!bChangeMade) {
			//TES 9/2/2008 - PLID 26534 - We have not. Therefore, we must be dealing with an appointment that belongs to
			// an in-use room, but that isn't already in there.  Therefore, either it's in the checkout list and being 
			// checked out, or it's anywhere else and being added to the existing room.
			
			if(nStatusID == -1) {
				//the room is being checked out, so clear its data

				KillTimer(nRoomAppointmentID);

				// (j.jones 2008-05-29 12:07) - PLID 27797 - if the appt. was in the checkout list, remove it
				IRowSettingsPtr pCheckoutRow = m_pCheckoutList->FindByColumn(rmclcApptID, nAppointmentID, 0, FALSE);
				if(pCheckoutRow != NULL) {
					m_pCheckoutList->RemoveRow(pCheckoutRow);
				}

				//color-code the patient in today's appt. list
				IRowSettingsPtr pPatientRow = m_pPatientList->FindByColumn(rmplcApptID, nAppointmentID, 0, FALSE);
				if(pPatientRow) {
					pPatientRow->PutValue(rmplcRoomStatusID, nStatusID);

					// (j.jones 2008-05-30 09:04) - PLID 27797 - color appropriately
					ColorApptRow(pPatientRow);
				}

				//and potentially collapse completed hours
				CheckAutoCollapseFinishedHours();

				// (j.jones 2008-05-29 10:16) - PLID 27797 - removed waiting area code,
				// the patient doesn't get put back into the waiting area anymore
			}
			else {
				// (j.armen 2012-03-05 11:20) - PLID 48555 - Select the highest prority recall status for this patient
				// (a.walling 2013-12-12 16:51) - PLID 60005 - RoomManager only needs a smaller subset of the entire recall query - SelectRecallStatusesPastDueOrNeeded
				CSqlFragment sqlRecall("(SELECT MAX(RecallStatusID) AS RecallStatusID FROM ({SQL}) SubQ WHERE SubQ.PatientID = ApptQ.PatientID)", RecallUtils::SelectRecallStatusesPastDueOrNeeded());

				//TES 9/2/2008 - PLID 26534 - We must be adding to an already used room, and therefore
				// we need to add a new row, with the same room information, for this appointment.
				// (j.jones 2008-11-12 16:56) - PLID 28035 - added purpose text
				// (d.thompson 2009-07-10) - PLID 26860 - added physical arrival time
				// (j.jones 2010-08-27 10:31) - PLID 36975 - the patient name is now calculated by a preference	
				// (d.lange 2010-08-31 10:24) - PLID 39431 - added provider column
				// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor
				// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource 
				// (c.haag 2010-10-26 10:07) - PLID 39199 - Changed how we determine whether the appt should be in the waiting area (added WaitingArea)
				// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
				_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PatientID, PatientName, {SQL} AS RecallStatusID, "
					"CASE WHEN PurposeName <> '' THEN TypeName + ' - ' + PurposeName ELSE TypeName END AS Purpose, AptTypeColor, "
					"StartTime, LastArrivalTime, CheckInTime, TimeLastSeenByPerson, WithPerson, LastUpdateUserID, Username, RoomName, Provider, Resources "
					"FROM (SELECT AppointmentsT.PatientID AS PatientID, "
					"{SQL} AS PatientName, "
					"AppointmentsT.StartTime, "
					"(SELECT MAX(TimeStamp) FROM AptShowStateHistoryT WHERE AptShowStateHistoryT.AppointmentID = AppointmentsT.ID AND ShowStateID IN (SELECT ID FROM AptShowStateT WHERE WaitingArea = CONVERT(BIT,1))) AS LastArrivalTime, "
					"RoomAppointmentsT.CheckInTime, "
					""
					"(SELECT Min(UpdateTime) AS FirstTimeWithoutPerson FROM RoomAppointmentHistoryT "
					"	WHERE StatusID IN (SELECT ID FROM RoomStatusT WHERE WithPerson = 0) "
					"	AND RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID "
					"	AND UpdateTime > (SELECT Max(UpdateTime) AS LastStartTimeWithPerson FROM RoomAppointmentHistoryT "
					"	WHERE StatusID IN (SELECT ID FROM RoomStatusT WHERE WithPerson = 1) "
					"	AND RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID)) "
					"AS TimeLastSeenByPerson, "
					""
					"RoomStatusT.WithPerson, RoomAppointmentsT.LastUpdateUserID, "
					"UsersT.Username, RoomsT.Name AS RoomName, "
					"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, "
					"AptTypeT.Name AS TypeName, CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS AptTypeColor, "
					"Provider.Last + ', ' + Provider.First + ' ' + Provider.Middle AS Provider, "
					" dbo.GetResourceString(AppointmentsT.ID) AS Resources "
					"FROM RoomsT "
					"INNER JOIN (SELECT * FROM RoomAppointmentsT WHERE StatusID NOT IN (0, -1)) AS RoomAppointmentsT ON RoomsT.ID = RoomAppointmentsT.RoomID "
					"INNER JOIN AppointmentsT ON RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
					"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
					"INNER JOIN RoomStatusT ON RoomAppointmentsT.StatusID = RoomStatusT.ID "
					"INNER JOIN UsersT ON RoomAppointmentsT.LastUpdateUserID = UsersT.PersonID "
					"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
					"LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
					"LEFT JOIN PersonT AS Provider ON PatientsT.MainPhysician = Provider.ID "
					"WHERE RoomAppointmentsT.ID = {INT} "
					") AS ApptQ", sqlRecall, GetPatientNameSqlFormat(), nRoomAppointmentID);
				if(rs->eof) {
					//shouldn't be possible!
					ASSERT(FALSE);
					RequeryAll(FALSE);
					// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
					Flash();
					return;
				}

				IRowSettingsPtr pNewRow = m_pRoomList->GetNewRow();
				pNewRow->PutValue(rmrlcRoomID, nRoomID);
				pNewRow->PutValue(rmrlcRoomAppointmentID, (long)nRoomAppointmentID);
				pNewRow->PutValue(rmrlcApptID, (long)nAppointmentID);
				pNewRow->PutValue(rmrlcPatientID, rs->Fields->Item["PatientID"]->Value);
				pNewRow->PutValue(rmrlcRoomName, rs->Fields->Item["RoomName"]->Value);
				pNewRow->PutValue(rmrlcPatientName, rs->Fields->Item["PatientName"]->Value);
				// (j.jones 2008-11-12 16:53) - PLID 28035 - added purpose field
				pNewRow->PutValue(rmrlcPurpose, rs->Fields->Item["Purpose"]->Value);
				// (j.jones 2010-08-27 09:39) - PLID 39774 - added appt. time
				pNewRow->PutValue(rmrlcApptTime, rs->Fields->Item["StartTime"]->Value);
				// (d.thompson 2009-07-10) - PLID 26860 - added physical arrival time
				pNewRow->PutValue(rmrlcArrivalTime, rs->Fields->Item["LastArrivalTime"]->Value);
				pNewRow->PutValue(rmrlcCheckInTime, rs->Fields->Item["CheckInTime"]->Value);
				pNewRow->PutValue(rmrlcTimeLastSeen, rs->Fields->Item["TimeLastSeenByPerson"]->Value);
				pNewRow->PutValue(rmrlcStatusID, (long)nStatusID);
				pNewRow->PutValue(rmrlcWithPerson, rs->Fields->Item["WithPerson"]->Value);
				pNewRow->PutValue(rmrlcWaiting, AdoFldBool(rs, "WithPerson", FALSE) ? _bstr_t("No") : _bstr_t("''"));
				// (d.lange 2010-08-31 10:24) - PLID 39431 - added provider column
				pNewRow->PutValue(rmrlcProvider, rs->Fields->Item["Provider"]->Value);
				// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource
				pNewRow->PutValue(rmrlcResources , rs->Fields->Item["Resources"]->Value);
				// (d.lange 2010-11-30 11:17) - PLID 40295 - Added Preview EMN
				pNewRow->PutValue(rmrlcPreviewEMN, (long)m_hIconPreview);
				pNewRow->PutValue(rmrlcLastUserID, rs->Fields->Item["LastUpdateUserID"]->Value);
				pNewRow->PutValue(rmrlcLastUserName, rs->Fields->Item["Username"]->Value);
				// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
				pNewRow->PutValue(rmrlcAptTypeColor, rs->Fields->Item["AptTypeColor"]->Value);
				if(m_bColorPurposeColumnByType) {
					pNewRow->PutCellForeColor(rmrlcPurpose, AdoFldLong(rs, "AptTypeColor", 0));
				}
				// (j.armen 2012-03-05 11:19) - PLID 48555 - Get the RecallStatusID and Ensure the Flag Column to make sure that the column is set properly
				pNewRow->PutValue(rmrlcRecallStatusID, rs->Fields->Item["RecallStatusID"]->Value);

				m_pRoomList->AddRowSorted(pNewRow, NULL);

				EnsureRecallFlagColumn(m_pRoomList, rmrlcRecallStatusID, rmrlcRecallStatus, "ShowRoomManagerRoomsColumnRecall");

				//color-code the patient in today's appt. list
				IRowSettingsPtr pPatientRow = m_pPatientList->FindByColumn(rmplcApptID, nAppointmentID, 0, FALSE);
				if(pPatientRow) {
					pPatientRow->PutValue(rmplcRoomStatusID, nStatusID);

					// (j.jones 2008-05-30 09:04) - PLID 27797 - color appropriately
					ColorApptRow(pPatientRow);
				}

				// (j.jones 2007-02-07 09:56) - PLID 24595 - if the appt. was in the waiting area,
				// remove it
				IRowSettingsPtr pWaitingAreaRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, nAppointmentID, 0, FALSE);
				if(pWaitingAreaRow != NULL) {
					m_pWaitingAreaList->RemoveRow(pWaitingAreaRow);
				}

				// (j.jones 2008-05-29 12:07) - PLID 27797 - if the appt. was in the checkout list, remove it
				IRowSettingsPtr pCheckoutRow = m_pCheckoutList->FindByColumn(rmclcApptID, nAppointmentID, 0, FALSE);
				if(pCheckoutRow != NULL) {
					m_pCheckoutList->RemoveRow(pCheckoutRow);
				}

				//TES 8/29/2008 - PLID 26534 - Set the timer.
				UpdateRoomWaitTime(nAppointmentID);

				rs->Close();
			}
		}

		else {
			//TES 8/29/2008 - PLID 26534 - We did process the change.  Did we clear out a row?
			if(pClearedRow) {
				if(nRowCount > 1) {
					//TES 8/29/2008 - PLID 26534 - We cleared out a row for this room, but this room has multiple rows.  Therefore
					// we don't need to keep the row we cleared as a placeholder, so just remove it entirely.
					m_pRoomList->RemoveRow(pClearedRow);
				}
			}
		}

		//TES 8/29/2008 - PLID 26534 - Now we're done, so we can flash.
		// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
		Flash();
		return;

	}NxCatchAll("Error in CRoomManagerDlg::ReflectChangedRoomAppointment");
}

// (j.jones 2010-12-02 10:30) - PLID 38597 - if ReflectChangedRoomAppointment is for a waiting room,
// this function is called to reflect the WaitingRoom list
void CRoomManagerDlg::ReflectChangedWaitingRoomAppointment(long nRoomAppointmentID, long nRoomID, long nAppointmentID, long nStatusID)
{
	try {

		//this room appointment is for a patient in a waiting room, not a regular room

		//see if this appointment row already exists, it may or may not be tied to a room
		IRowSettingsPtr pRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, nAppointmentID, 0, FALSE);

		//is this a row that needs to exist in the waiting area?	
		if(nStatusID != -1 && nStatusID != 0) {
				
			//fill every field in the row, whether it exists or is brand new
			
			CSqlFragment sqlPatientNameSQLFormat = GetPatientNameSqlFormat();

			//utilize the resource filter
			CString strResourceIDs;
			strResourceIDs = m_strResourceIDs;
			strResourceIDs.TrimRight();
			CSqlFragment sqlResourceFilter;
			if(!strResourceIDs.IsEmpty()) {
				strResourceIDs.Replace(" ", ",");
				sqlResourceFilter = CSqlFragment(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN ({INTSTRING}))", strResourceIDs);
			}

			// (j.jones 2009-08-04 08:55) - PLID 24600 - added location filter
			long nLocationID = -1;
			{
				IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
				if(pRow == NULL) {
					nLocationID = GetCurrentLocationID();
					m_pLocationCombo->SetSelByColumn(lccID, nLocationID);			
				}
				else {
					nLocationID = VarLong(pRow->GetValue(lccID));
				}
			}	

			// (j.jones 2010-12-02 09:18) - PLID 38597 - added a waiting room filter
			long nWaitingRoomID = -1;
			CSqlFragment sqlWaitingRoomFilter;
			//-1 is all rooms, -2 is no assigned room
			{
				IRowSettingsPtr pRow = m_pWaitingRoomCombo->GetCurSel();
				if(pRow == NULL) {
					//set to all rooms
					m_pWaitingRoomCombo->SetSelByColumn(wrccID, (long)-1);			
				}
				else {
					nWaitingRoomID = VarLong(pRow->GetValue(wrccID));
				}

				//if -1, we use no filter, but -2 is a valid filter value
				if(nWaitingRoomID != -1) {
					sqlWaitingRoomFilter = CSqlFragment(" AND RoomsT.ID = {INT} ", nWaitingRoomID);
				}
			}

			// (j.armen 2012-03-05 11:20) - PLID 48555 - Select the highest prority recall status for this patient
			// (a.walling 2013-12-12 16:51) - PLID 60005 - RoomManager only needs a smaller subset of the entire recall query - SelectRecallStatusesPastDueOrNeeded
			CSqlFragment sqlRecall("(SELECT MAX(RecallStatusID) AS RecallStatusID FROM ({SQL}) SubQ WHERE SubQ.PatientID = AppointmentsT.PatientID)", RecallUtils::SelectRecallStatusesPastDueOrNeeded());

			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
			_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT "
				"AppointmentsT.ID AS AppointmentID, AppointmentsT.PatientID, {SQL} AS PatientName, {SQL} AS RecallStatusID, "
				"AppointmentsT.StartTime, TimeMarkedIn, CheckedInBy, "
				""
				//modified from the way the "time last seen" is calculated per room, to include
				//checkout time as a time last seen
				// (j.jones 2010-12-02 16:17) - PLID 38597 - ignore waiting rooms, they do not reset the wait timer
				"(SELECT Max(UpdateTime) AS FirstTimeWithoutPerson FROM RoomAppointmentHistoryT "
				"	WHERE RoomAppointmentHistoryT.RoomAppointmentID IN ("
				"		SELECT RoomAppointmentsT.ID FROM RoomAppointmentsT "
				"		INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"		WHERE RoomsT.WaitingRoom = 0 AND RoomAppointmentsT.AppointmentID = AppointmentsT.ID)"
				"	) "
				"AS TimeLastSeenByPerson, "
				""
				"CASE WHEN dbo.GetPurposeString(AppointmentsT.ID) <> '' THEN AptTypeT.Name + ' - ' + dbo.GetPurposeString(AppointmentsT.ID) ELSE AptTypeT.Name END AS Purpose, "
				"CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS AptTypeColor, "
				"Provider.Last + ', ' + Provider.First + ' ' + Provider.Middle AS Provider, "
				" dbo.GetResourceString(AppointmentsT.ID) AS Resources,  "		
				"RoomsT.ID AS WaitingRoomID, "
				"RoomsT.Name AS WaitingRoomName, "
				"RoomAppointmentsT.ID AS WaitingRoomAppointmentID "
				"FROM AppointmentsT "
				"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "		
				"LEFT JOIN (SELECT AppointmentID, Max(TimeStamp) AS TimeMarkedIn, Max(UsersT.UserName) AS CheckedInBy "
				"	FROM AptShowStateHistoryT "
				"	LEFT JOIN UsersT ON AptShowStateHistoryT.UserID = UsersT.PersonID "
				"	LEFT JOIN AptShowStateT on AptShowStateT.ID = AptShowStateHistoryT.ShowStateID "
				"	WHERE AptShowStateT.WaitingArea = CONVERT(BIT,1) GROUP BY AppointmentID, AptShowStateT.WaitingArea) "
				"	AS AptShowStateHistoryQ ON AppointmentsT.ID = AptShowStateHistoryQ.AppointmentID "
				"INNER JOIN RoomAppointmentsT ON AppointmentsT.ID = RoomAppointmentsT.AppointmentID "
				"INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				"LEFT JOIN PersonT AS Provider ON PatientsT.MainPhysician = Provider.ID "
				//not cancelled, marked 'In' appointments
				"WHERE AppointmentsT.Status <> 4 "
				//using our location filter 
				"AND AppointmentsT.LocationID = {INT} "
				//on today's date
				"AND Convert(datetime, Convert(nvarchar, AppointmentsT.Date, 1)) = Convert(datetime, Convert(nvarchar, GetDate(), 1)) "
				"AND RoomsT.WaitingRoom = 1 AND RoomAppointmentsT.StatusID NOT IN (0, -1) "
				"AND RoomAppointmentsT.ID = {INT} "
				
				//optionally filtered by resource
				" {SQL} "

				//optionally filtered by waiting room
				" {SQL} ", sqlPatientNameSQLFormat, sqlRecall, nLocationID, nRoomAppointmentID, sqlResourceFilter, sqlWaitingRoomFilter);

			if(rs->eof) {
				//do nothing, the record could just be for something we aren't filtering on
				// (j.gruber 2016-06-27 14:04) - PLID-68657 - check to see though if we are filtered on No room and the appt just got a room
				if (nWaitingRoomID == -2 && nRoomID != -2)
				{
					//we need to remove the row
					if (pRow) {
						m_pWaitingAreaList->RemoveRow(pRow);
					}

				}
				return;
			}

			//if the row does not exist, create it
			if(pRow == NULL) {
				pRow = m_pWaitingAreaList->GetNewRow();
				m_pWaitingAreaList->AddRowSorted(pRow, NULL);
			}

			pRow->PutValue(rmwalcApptID, rs->Fields->Item["AppointmentID"]->Value);
			pRow->PutValue(rmwalcWaitingRoomID, rs->Fields->Item["WaitingRoomID"]->Value);
			pRow->PutValue(rmwalcWaitingRoomName, rs->Fields->Item["WaitingRoomName"]->Value);
			pRow->PutValue(rmwalcWaitingRoomAppointmentID, rs->Fields->Item["WaitingRoomAppointmentID"]->Value);
			pRow->PutValue(rmwalcPatientID, rs->Fields->Item["PatientID"]->Value);
			pRow->PutValue(rmwalcPatientName, rs->Fields->Item["PatientName"]->Value);
			pRow->PutValue(rmwalcApptTime, rs->Fields->Item["StartTime"]->Value);
			pRow->PutValue(rmwalcPurpose, rs->Fields->Item["Purpose"]->Value);
			pRow->PutValue(rmwalcCheckInTime, rs->Fields->Item["TimeMarkedIn"]->Value);
			pRow->PutValue(rmwalcTimeLastSeen, rs->Fields->Item["TimeLastSeenByPerson"]->Value);
			pRow->PutValue(rmwalcWaiting, (LPCTSTR)"");
			pRow->PutValue(rmwalcProvider, rs->Fields->Item["Provider"]->Value);
			pRow->PutValue(rmwalcCheckedInBy, rs->Fields->Item["CheckedInBy"]->Value);
			pRow->PutValue(rmwalcAptTypeColor, rs->Fields->Item["AptTypeColor"]->Value);
			pRow->PutValue(rmwalcResources, rs->Fields->Item["Resources"]->Value);
			// (d.lange 2010-12-08 16:39) - PLID 40295 - Added EMR Preview icon
			pRow->PutValue(rmwalcPreviewEMN, (long)m_hIconPreview);
			// (j.armen 2012-03-05 11:19) - PLID 48555 - Get the RecallStatusID and Ensure the Flag Column to make sure that the column is set properly
			pRow->PutValue(rmwalcRecallStatusID, rs->Fields->Item["RecallStatusID"]->Value);

			EnsureRecallFlagColumn(m_pWaitingAreaList, rmwalcRecallStatusID, rmwalcRecallStatus, "ShowRoomManagerWaitingAreaColumnRecall");

			rs->Close();

			UpdateWaitingAreaWaitTime(nAppointmentID);

			//color-code the patient in today's appt. list
			IRowSettingsPtr pPatientRow = m_pPatientList->FindByColumn(rmplcApptID, nAppointmentID, 0, FALSE);
			if(pPatientRow) {
				//all "waiting" patients have no status in the patient list, so this is always NULL
				pPatientRow->PutValue(rmplcRoomStatusID, g_cvarNull);

				//color appropriately
				ColorApptRow(pPatientRow);
			}

			//if the appt. was in a regular room, a second tablechecker should be handling it

			//if the appt. was in the checkout list, remove it
			IRowSettingsPtr pCheckoutRow = m_pCheckoutList->FindByColumn(rmclcApptID, nAppointmentID, 0, FALSE);
			if(pCheckoutRow != NULL) {
				m_pCheckoutList->RemoveRow(pCheckoutRow);					
			}
		}
		else {
			//this row does not need to exist, remove it if it does exist
			if(pRow) {
				m_pWaitingAreaList->RemoveRow(pRow);
			}
		}

		//flash to reflect the change (only if minimized)
		Flash();
		return;

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-08-18 13:28) - PLID 63404 - fills a ReflectChangedAppointment_ApptInfo if
// m_bIsLoaded is false, avoids the recordset entirely if already loaded
void CRoomManagerDlg::LoadReflectChangedAppointmentInfo(long nAppointmentID, ReflectChangedAppointment_ApptInfo &eInfo, CSqlFragment sqlLastArrivalTime /*= CSqlFragment("NULL")*/)
{
	if (eInfo.m_bIsLoaded) {

		//make sure the loaded data is for the same appointment ID
		if (nAppointmentID != eInfo.m_nAppointmentID) {
			//this should never actually happen, because the ApptInfo object
			//is local to ReflectChangedAppointment, for only one appointment
			ASSERT(FALSE);

			//reset the information
			eInfo = ReflectChangedAppointment_ApptInfo();
		}
		else {
			//we have already loaded this appointment data,
			//we do not need to do so again
			return;
		}
	}

	eInfo.m_nAppointmentID = nAppointmentID;

	// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PatientID, PatientName, "
		"CASE WHEN PurposeName <> '' THEN TypeName + ' - ' + PurposeName ELSE TypeName END AS Purpose, AptTypeColor, "
		"StartTime, LastArrivalTime "
		"FROM ("
		"SELECT PatientID, "
		"{SQL} AS PatientName, "
		"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, "
		"AptTypeT.Name AS TypeName, CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS AptTypeColor, "
		"AppointmentsT.StartTime, "
		"{SQL} AS LastArrivalTime "
		"FROM AppointmentsT "
		"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
		"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
		"WHERE AppointmentsT.ID = {INT} "
		") AS ApptsQ", GetPatientNameSqlFormat(), sqlLastArrivalTime, nAppointmentID);
	if (!rs->eof) {
		eInfo.m_bExists = true;
		eInfo.m_nPatientID = AdoFldLong(rs, "PatientID", -1);
		eInfo.m_strPatientName = AdoFldString(rs, "PatientName", "");
		eInfo.m_strPurpose = AdoFldString(rs, "Purpose", "");
		eInfo.m_nAptTypeColor = AdoFldLong(rs, "AptTypeColor", 0);
		//this is often null
		eInfo.m_varArrivalTime = rs->Fields->Item["LastArrivalTime"]->Value;
	}
	else {
		//track that this appointment no longer exists
		eInfo.m_bExists = false;
	}
	rs->Close();

	//set the data as having been loaded
	eInfo.m_bIsLoaded = true;
}

void CRoomManagerDlg::ReflectChangedAppointment(CTableCheckerDetails* pDetails)
{
	try {

		// (j.jones 2014-08-05 10:34) - PLID 63167 - this now uses an enumeration
		long nResID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiAppointmentID), -1);
		COleDateTime dtStart = VarDateTime(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiStartTime), g_cdtInvalid);
		// (j.jones 2007-09-06 15:20) - PLID 27312 - added EndTime to the tablechecker
		COleDateTime dtEnd = VarDateTime(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiEndTime), g_cdtInvalid);
		long nStatus = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiStatus), -1); // 4 = cancelled
		long nShowState = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiShowState), -1);
		// (j.jones 2014-08-05 10:50) - PLID 63185 - get the patientID, LocationID, ResourceIDs
		long nPatientID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiPatientID), -1);
		long nLocationID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiLocationID), -1);
		CString strResourceIDs = VarString(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiResourceIDs), "");

		// (j.jones 2014-08-18 14:41) - PLID 63404 - track a ReflectChangedAppointment_ApptInfo object
		// such that once we load more information from this appointment, we only do so once
		ReflectChangedAppointment_ApptInfo eApptInfo;

		BOOL bWaitingArea = FALSE;
		// (c.haag 2010-10-26 10:07) - PLID 39199 - See whether the appt should be in the Waiting Area
		// list based on the show state
		if (!m_mapShowStateIsWaitingArea.Lookup(nShowState, bWaitingArea))
		{
			// Odd, this must be a new state we've never seen before. Refresh the list and try again.
			// (j.jones 2010-12-09 10:20) - PLID 41763 - renamed this function, we now load all show states
			UpdateCachedShowStateInfo();
			if (!m_mapShowStateIsWaitingArea.Lookup(nShowState, bWaitingArea)) {
				// This is highly improbable though not impossible in a busy multi-user environment with a lot
				// of people configuring the scheduler. Just leave bWaitingArea as FALSE.
			}
		}

		// (j.jones 2010-12-02 17:26) - PLID 38597 - if the appointment is in the waiting area due to a waiting room, set the flag
		IRowSettingsPtr pWaitingRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, nResID, 0, FALSE);
		if(pWaitingRow != NULL && VarLong(pWaitingRow->GetValue(rmwalcWaitingRoomID), -2) > 0) {
			bWaitingArea = TRUE;
		}
		
		BOOL bIsEvent = (dtStart == dtEnd && dtStart.GetHour() == 0 && dtStart.GetMinute() == 0);

		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);

		// (j.jones 2010-08-27 09:46) - PLID 39774 - update the appt. time in the room list, always
		IRowSettingsPtr pRoomRow = m_pRoomList->FindByColumn(rmrlcApptID, nResID, 0, FALSE);
		if(pRoomRow) {
			_variant_t varDate = dtStart;
			varDate.vt = VT_DATE;
			pRoomRow->PutValue(rmrlcApptTime, varDate);
		}

		//first update the placement of the appointment in the
		//"today's patients" list

		IRowSettingsPtr pRow = m_pPatientList->FindByColumn(rmplcApptID, nResID, 0, FALSE);
		if(pRow != NULL) {
			//the appointment exists in our list - should we update it or remove it?
			
			//is the appointment cancelled?
			if(nStatus == 4) {

				//j.gruber 2007-04-19 17:21) - PLID 25646 - update the color if the parent is empty
				IRowSettingsPtr pParentRow = pRow->GetParentRow();
				
				//remove the row
				m_pPatientList->RemoveRow(pRow);

				//patient appointment has been cancelled, requery resource combo
				//(s.tullis 2013-11-13 14:03) - PLID 37164 - In room manager, the client only wants the resources with scheduled appts to be populated in the Filter on Resource dropdown.
				RequeryResourceList();

				if(pParentRow != NULL) {
					if(pParentRow->GetFirstChildRow()) {
						//color black
						pParentRow->PutForeColor(RGB(0,0,0));
					}
					else {
						//color gray
						pParentRow->PutForeColor(m_clrFolderEmpty);
					}
				}

				// (j.jones 2007-02-21 17:56) - PLID 24867 - also remove from the Waiting Area, if it is present
				IRowSettingsPtr pWaitingRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, nResID, 0, FALSE);
				if(pWaitingRow != NULL)
					m_pWaitingAreaList->RemoveRow(pWaitingRow);

				//if the appointment is in a room, clear the room

				IRowSettingsPtr pRoomRow = m_pRoomList->FindByColumn(rmrlcApptID, nResID, 0, FALSE);
				if(pRoomRow) {

					_variant_t varNull;
					varNull.vt = VT_NULL;

					KillTimer(VarLong(pRoomRow->GetValue(rmrlcRoomAppointmentID), -1));

					//remove the purpose color if there is one
					pRow->PutCellForeColor(rmrlcPurpose, dlColorNotSet);
					pRoomRow->PutForeColor(RGB(0,0,0));
					pRoomRow->PutCellForeColor(rmrlcWaiting, RGB(0,0,0));

					pRoomRow->PutValue(rmrlcRoomAppointmentID, varNull);
					pRoomRow->PutValue(rmrlcApptID, varNull);
					pRoomRow->PutValue(rmrlcPatientID, varNull);
					pRoomRow->PutValue(rmrlcPatientName, varNull);
					// (j.jones 2008-11-12 16:53) - PLID 28035 - added purpose field
					pRoomRow->PutValue(rmrlcPurpose, varNull);
					// (j.jones 2010-08-27 09:39) - PLID 39774 - added appt. time
					pRoomRow->PutValue(rmrlcApptTime, varNull);
					// (d.thompson 2009-07-10) - PLID 26860 - added physical arrival time
					pRoomRow->PutValue(rmrlcArrivalTime, varNull);
					pRoomRow->PutValue(rmrlcCheckInTime, varNull);
					pRoomRow->PutValue(rmrlcTimeLastSeen, varNull);
					pRoomRow->PutValue(rmrlcStatusID, varNull);
					pRoomRow->PutValue(rmrlcWithPerson, varNull);
					pRoomRow->PutValue(rmrlcWaiting, varNull);
					// (d.lange 2010-08-30 09:18) - PLID 39431 - added provider
					pRoomRow->PutValue(rmrlcProvider, varNull);
						// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource column
					pRoomRow->PutValue(rmrlcResources, varNull);
					pRoomRow->PutValue(rmrlcLastUserID, varNull);
					pRoomRow->PutValue(rmrlcLastUserName, varNull);
					// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
					pRoomRow->PutValue(rmrlcAptTypeColor, (long)RGB(0,0,0));
					pRoomRow->PutCellForeColor(rmrlcPurpose, (long)RGB(0,0,0));
					// (d.lange 2010-11-29 12:48) - PLID 40295 - added Preview EMNs column
					pRoomRow->PutValue(rmrlcPreviewEMN, varNull);
					// (j.armen 2012-03-05 11:19) - PLID 48555 - Get the RecallStatusID and Ensure the Flag Column to make sure that the column is set properly
					pRoomRow->PutValue(rmrlcRecallStatusID, varNull);

					EnsureRecallFlagColumn(m_pRoomList, rmrlcRecallStatusID, rmrlcRecallStatus, "ShowRoomManagerRoomsColumnRecall");
				}

				// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
				Flash();
				return;
			}

			//has the status changed?
			bool bReloadArrivalTime = false;
			if(VarLong(pRow->GetValue(rmplcApptShowState), 0) != nShowState) {
				//it did, so update, and we will color later
				pRow->PutValue(rmplcApptShowState, (long)nShowState);

				// (d.thompson 2009-07-10) - PLID 26860 - The "arrival time" is based off the "Marked In" status.  So
				//	if this appointment just got marked in, we need to update that value.
				// (c.haag 2010-10-26 10:07) - PLID 39199 - This is now done in the next code block. We no longer
				// look at the state; we now look at the waiting area bit.
				/*if(nShowState == 1) {
					//For efficiency, we'll load this below.  There's a query for some other testing that is run every
					//	single time, so there's no sense also querying here.
					bReloadArrivalTime = true;
				}*/
			}

			// (c.haag 2010-10-26 10:07) - PLID 39199 - If the waiting area bit has changed, we may need to set 
			// the reload arrival time flag.
			if(VarBool(pRow->GetValue(rmplcAptWaitingArea), FALSE) != bWaitingArea) {
				//it did, so update
				pRow->PutValue(rmplcAptWaitingArea, (bWaitingArea) ? g_cvarTrue : g_cvarFalse);
				if(bWaitingArea) {
					//For efficiency, we'll load this below.  There's a query for some other testing that is run every
					//	single time, so there's no sense also querying here.
					bReloadArrivalTime = true;
				}
			}

			//has the time changed?
			COleDateTime dtOldStart = VarDateTime(pRow->GetValue(rmplcApptTime), dtInvalid);
			if(dtOldStart != dtStart) {

				//is it a different day?
				// (j.jones 2007-02-22 10:41) - PLID 24877 - this did not previously check
				// that the day OR year changed, it errantly checked AND
				if(dtOldStart.GetDayOfYear() != dtStart.GetDayOfYear()
					|| dtOldStart.GetYear() != dtStart.GetYear()) {

					IRowSettingsPtr pParentRow = pRow->GetParentRow();

					//remove from our view
					m_pPatientList->RemoveRow(pRow);

					// (j.jones 2007-02-21 17:56) - PLID 24867 - also remove from the Waiting Area, if it is present
					IRowSettingsPtr pWaitingRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, nResID, 0, FALSE);
					if(pWaitingRow != NULL)
						m_pWaitingAreaList->RemoveRow(pWaitingRow);

					//now, do we need to re-color the parent row?				
					if(pParentRow != NULL) {
						if(pParentRow->GetFirstChildRow()) {
							//color black
							pParentRow->PutForeColor(RGB(0,0,0));
						}
						else {
							//color gray
							pParentRow->PutForeColor(m_clrFolderEmpty);
						}
					}

					// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
					Flash();
					return;
				}

				//confirm that the time actually changed
				long nNewHour = dtStart.GetHour();
				long nNewMinute = dtStart.GetMinute();

				// (j.jones 2007-09-06 16:28) - PLID 27312 - also handle if it is now an event and formerly was not
				if(dtOldStart.GetHour() != nNewHour || (bIsEvent && dtOldStart.GetMinute() != nNewMinute)) {
					//it changed hours, we need to reassign the parent in the appt. list

					//now update the appt. list row
					_variant_t varDate = dtStart;
					varDate.vt = VT_DATE;
					pRow->PutValue(rmplcApptTime, varDate);
					pRow->PutValue(rmplcHour, (long)nNewHour);
					pRow->PutValue(rmplcMinute, (long)nNewMinute);

					// (j.jones 2007-09-06 15:38) - PLID 27312 - Events now have an hour of -2
					long nParentID = nNewHour;
					if(bIsEvent)
						nParentID = -2;

					pRow->PutValue(rmplcParentID, (long)nParentID);
					pRow->PutValue(rmplcApptShowState, (long)nShowState);
					// (c.haag 2010-10-26 10:07) - PLID 39199
					pRow->PutValue(rmplcAptWaitingArea, (bWaitingArea) ? g_cvarTrue : g_cvarFalse);

					IRowSettingsPtr pParentRow = m_pPatientList->FindByColumn(rmplcID, nParentID, 0, FALSE);
					if(pParentRow == NULL) {
						//if it doesn't exist, we need to requery
						RequeryPatientList(TRUE);
						
						// (j.jones 2007-02-22 09:03) - PLID 24867 - if we are requerying the patient list,
						// better requery the Waiting Area list too
						RequeryWaitingAreaList();

						// (j.jones 2008-05-29 10:53) - PLID 27797 - requery the Checkout list as well
						RequeryCheckoutList();

						// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
						Flash();
						return;
					}
					else {
						//ensure it's actually a parent!
						if(pParentRow->GetParentRow() != NULL) {
							//yikes, we managed to grab a child! Try again, this should only happen once.

							pParentRow = m_pPatientList->FindByColumn(rmplcID, nParentID, pParentRow->GetNextRow(), FALSE);
							if(pParentRow == NULL) {
								//if it doesn't exist, we need to requery
								RequeryPatientList(TRUE);
								
								// (j.jones 2007-02-22 09:03) - PLID 24867 - if we are requerying the patient list,
								// better requery the Waiting Area list too
								RequeryWaitingAreaList();

								// (j.jones 2008-05-29 10:53) - PLID 27797 - requery the Checkout list as well
								RequeryCheckoutList();

								// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
								Flash();
								return;
							}
						}
					}

					//now finally add sorted

					IRowSettingsPtr pOldParentRow = pRow->GetParentRow();

					m_pPatientList->RemoveRow(pRow);
					m_pPatientList->AddRowSorted(pRow, pParentRow);

					//now, do we need to re-color any parent rows?

					if(pOldParentRow != NULL) {
						if(pOldParentRow->GetFirstChildRow()) {
							//color black
							pOldParentRow->PutForeColor(RGB(0,0,0));
						}
						else {
							//color gray
							pOldParentRow->PutForeColor(m_clrFolderEmpty);
						}
					}
					if(pParentRow != NULL) {
						if(pParentRow->GetFirstChildRow()) {
							//color black
							pParentRow->PutForeColor(RGB(0,0,0));
						}
						else {
							//color gray
							pParentRow->PutForeColor(m_clrFolderEmpty);
						}
					}

					//and now expand/collapse based on this new knowledge
					ExpandCurrentHour(TRUE, TRUE);
					CheckAutoCollapseFinishedHours();
				}
				else if(dtOldStart.GetMinute() != nNewMinute) {

					//it changed times within the same hour, so just update
					_variant_t varDate = dtStart;
					varDate.vt = VT_DATE;
					pRow->PutValue(rmplcApptTime, varDate);
					pRow->PutValue(rmplcMinute, (long)nNewMinute);
					pRow->PutValue(rmplcApptShowState, (long)nShowState);
					// (c.haag 2010-10-26 10:07) - PLID 39199
					pRow->PutValue(rmplcAptWaitingArea, (bWaitingArea) ? g_cvarTrue : g_cvarFalse);

					//now sort under the parent, first by validating we have a parent

					IRowSettingsPtr pParentRow = pRow->GetParentRow();
					if(pParentRow == NULL) {
						//I can't fathom how this is possible, but if it happens, find the parent hour
						pParentRow = m_pPatientList->FindByColumn(rmplcHour, nNewHour, 0, FALSE);
						if(pParentRow == NULL) {
							//if it doesn't exist, we need to requery
							RequeryPatientList(TRUE);
							
							// (j.jones 2007-02-22 09:03) - PLID 24867 - if we are requerying the patient list,
							// better requery the Waiting Area list too
							RequeryWaitingAreaList();

							// (j.jones 2008-05-29 10:53) - PLID 27797 - requery the Checkout list as well
							RequeryCheckoutList();
							

							// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
							Flash();
							return;
						}
					}

					IRowSettingsPtr pOldParentRow = pRow->GetParentRow();

					m_pPatientList->RemoveRow(pRow);
					m_pPatientList->AddRowSorted(pRow, pParentRow);

					//now, do we need to re-color any parent rows?

					if(pOldParentRow != NULL) {
						if(pOldParentRow->GetFirstChildRow()) {
							//color black
							pOldParentRow->PutForeColor(RGB(0,0,0));
						}
						else {
							//color gray
							pOldParentRow->PutForeColor(m_clrFolderEmpty);
						}
					}
					if(pParentRow != NULL) {
						if(pParentRow->GetFirstChildRow()) {
							//color black
							pParentRow->PutForeColor(RGB(0,0,0));
						}
						else {
							//color gray
							pParentRow->PutForeColor(m_clrFolderEmpty);
						}
					}
				}
			}

			
			//has the resource changed?
			// (j.jones 2007-02-21 17:37) - PLID 24866 - added the resource check
			{
				CString strFilteredResourceIDs;
				strFilteredResourceIDs = m_strResourceIDs;
				strFilteredResourceIDs.TrimRight();
				if (!strFilteredResourceIDs.IsEmpty()) {

					// (j.jones 2014-08-15 13:52) - PLID 63185 - compare to the resource IDs provided by the tablechecker
					if (!DoesResourceExistInFilter(strResourceIDs, strFilteredResourceIDs)) {

						//not our resources
						IRowSettingsPtr pParentRow = pRow->GetParentRow();

						//we're filtering on resources, and the appointment doesn't have one of them, so buh-bye
						m_pPatientList->RemoveRow(pRow);

						//also remove from the Waiting Area, if it is present
						IRowSettingsPtr pWaitingRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, nResID, 0, FALSE);
						if (pWaitingRow != NULL)
							m_pWaitingAreaList->RemoveRow(pWaitingRow);

						// (j.jones 2009-08-04 10:16) - PLID 35099 - also remove from the Checkout Area, if it is present
						IRowSettingsPtr pCheckoutRow = m_pCheckoutList->FindByColumn(rmclcApptID, nResID, 0, FALSE);
						if (pCheckoutRow != NULL) {
							m_pCheckoutList->RemoveRow(pCheckoutRow);
						}

						//now, do we need to re-color the parent row?				
						if (pParentRow != NULL) {
							if (pParentRow->GetFirstChildRow()) {
								//color black
								pParentRow->PutForeColor(RGB(0, 0, 0));
							}
							else {
								//color gray
								pParentRow->PutForeColor(m_clrFolderEmpty);
							}
						}

						Flash();
						return;
					}
				}
			}

			// (j.jones 2009-08-04 08:55) - PLID 24600 - did the location change?
			long nFilteredLocationID = -1;
			{
				IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
				if(pRow == NULL) {
					nFilteredLocationID = GetCurrentLocationID();
					m_pLocationCombo->SetSelByColumn(lccID, nFilteredLocationID);
				}
				else {
					nFilteredLocationID = VarLong(pRow->GetValue(lccID));
				}
			}

			// (j.jones 2014-08-15 13:52) - PLID 63185 - compare to the location ID provided by the tablechecker
			if(nFilteredLocationID != nLocationID) {

				IRowSettingsPtr pParentRow = pRow->GetParentRow();

				//doesn't use our location
				m_pPatientList->RemoveRow(pRow);

				//also remove from the Waiting Area, if it is present
				IRowSettingsPtr pWaitingRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, nResID, 0, FALSE);
				if(pWaitingRow != NULL) {
					m_pWaitingAreaList->RemoveRow(pWaitingRow);
				}

				//also remove from the Checkout Area, if it is present
				IRowSettingsPtr pCheckoutRow = m_pCheckoutList->FindByColumn(rmclcApptID, nResID, 0, FALSE);
				if(pCheckoutRow != NULL) {
					m_pCheckoutList->RemoveRow(pCheckoutRow);
				}

				//now, do we need to re-color the parent row?				
				if(pParentRow != NULL) {
					if(pParentRow->GetFirstChildRow()) {
						//color black
						pParentRow->PutForeColor(RGB(0,0,0));
					}
					else {
						//color gray
						pParentRow->PutForeColor(m_clrFolderEmpty);
					}
				}

				Flash();
				return;
			}

			//if this appointment is in a room, find that room
			IRowSettingsPtr pCurRoomRow = m_pRoomList->FindByColumn(rmrlcApptID, nResID, 0, FALSE);

			// (j.jones 2014-08-15 15:56) - PLID 63185 - we don't need to calculate LastArrivalTime
			// if bReloadArrivalTime is false
			CSqlFragment sqlLastArrivalTime("NULL");
			if (bReloadArrivalTime && pCurRoomRow) {
				sqlLastArrivalTime = CSqlFragment("(SELECT MAX(TimeStamp) FROM AptShowStateHistoryT WHERE AptShowStateHistoryT.AppointmentID = AppointmentsT.ID AND ShowStateID IN (SELECT ID FROM AptShowStateT WHERE WaitingArea = CONVERT(BIT,1)))");
			}

			// (j.jones 2007-02-28 15:46) - PLID 24907 - check to see if the patient changed
			// (j.jones 2008-11-12 16:42) - PLID 28035 - added purpose text, which we also need to check here
			// (d.thompson 2009-07-10) - PLID 28680 - Added physical arrival time for efficiency.  It's checked above
			//	in this function, but since this query is run every time, I put the loading here.
			// (j.jones 2010-08-27 10:31) - PLID 36975 - the patient name is now calculated by a preference	
			// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor
			// (c.haag 2010-10-26 10:07) - PLID 39199 - Changed how we determine whether the appt should be in the waiting area (added WaitingArea)
			long nRowPatientID = VarLong(pRow->GetValue(rmplcPatientID), -1);
			CString strPurpose = VarString(pRow->GetValue(rmplcPurpose), "");

			//**********************************************************************
			//	This code is hit when the appointment is currently in the 
			//	PATIENT list, and should be there, and just needs to be updated.
			//**********************************************************************

			// (j.jones 2014-08-18 14:40) - PLID 63404 - call LoadReflectChangedAppointmentInfo to reduce recordset uses
			LoadReflectChangedAppointmentInfo(nResID, eApptInfo, sqlLastArrivalTime);

			if (eApptInfo.m_bExists) {
				
				//see if the patient changed or if the purpose text changed
				bool bSomethingChanged = false;

				// (d.thompson 2009-07-10) - PLID 28680 - Physical arrival time.  It's checked above, but loaded down here				
				if (bReloadArrivalTime && pCurRoomRow) {
					pCurRoomRow->PutValue(rmrlcArrivalTime, eApptInfo.m_varArrivalTime);
					bReloadArrivalTime = false;
					bSomethingChanged = true;
				}

				IRowSettingsPtr pWaitingRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, nResID, 0, FALSE);
				IRowSettingsPtr pRoomRow = m_pRoomList->FindByColumn(rmrlcApptID, nResID, 0, FALSE);
				IRowSettingsPtr pCheckoutRow = m_pCheckoutList->FindByColumn(rmclcApptID, nResID, 0, FALSE);

				//if the patient changed on the appointment, or the type/purpose changed, update the fields
				if (eApptInfo.m_nPatientID != nRowPatientID || eApptInfo.m_strPurpose != strPurpose) {

					bSomethingChanged = true;

					pRow->PutValue(rmplcPatientID, (long)eApptInfo.m_nPatientID);
					pRow->PutValue(rmplcPatientName, _bstr_t(eApptInfo.m_strPatientName));

					// (j.jones 2008-11-12 16:53) - PLID 28035 - added purpose field
					pRow->PutValue(rmplcPurpose, _bstr_t(eApptInfo.m_strPurpose));

					//also find/fix in the waiting list or the room list (as the room tablechecker won't be fired)					
					if (pWaitingRow != NULL) {
						pWaitingRow->PutValue(rmwalcPatientID, (long)eApptInfo.m_nPatientID);
						pWaitingRow->PutValue(rmwalcPatientName, _bstr_t(eApptInfo.m_strPatientName));
						pWaitingRow->PutValue(rmwalcPurpose, _bstr_t(eApptInfo.m_strPatientName));
						// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
						pWaitingRow->PutValue(rmwalcAptTypeColor, (long)eApptInfo.m_nAptTypeColor);
						if (m_bColorPurposeColumnByType) {
							pWaitingRow->PutCellForeColor(rmwalcPurpose, (long)eApptInfo.m_nAptTypeColor);
						}
					}
					
					if (pRoomRow != NULL) {
						pRoomRow->PutValue(rmrlcPatientID, (long)eApptInfo.m_nPatientID);
						pRoomRow->PutValue(rmrlcPatientName, _bstr_t(eApptInfo.m_strPatientName));
						pRoomRow->PutValue(rmrlcPurpose, _bstr_t(eApptInfo.m_strPatientName));
						// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
						pRoomRow->PutValue(rmrlcAptTypeColor, (long)eApptInfo.m_nAptTypeColor);
						if (m_bColorPurposeColumnByType) {
							pRoomRow->PutCellForeColor(rmrlcPurpose, (long)eApptInfo.m_nAptTypeColor);
						}
					}

					if (pCheckoutRow != NULL) {
						pCheckoutRow->PutValue(rmclcPatientID, (long)eApptInfo.m_nPatientID);
						pCheckoutRow->PutValue(rmclcPatientName, _bstr_t(eApptInfo.m_strPatientName));
						pCheckoutRow->PutValue(rmclcPurpose, _bstr_t(eApptInfo.m_strPatientName));
						// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
						pCheckoutRow->PutValue(rmclcAptTypeColor, (long)eApptInfo.m_nAptTypeColor);
						if (m_bColorPurposeColumnByType) {
							pCheckoutRow->PutCellForeColor(rmclcPurpose, (long)eApptInfo.m_nAptTypeColor);
						}
					}

					ColorApptRow(pRow);
				}
				
				if (bSomethingChanged) {
					Flash();
				}

				// (j.jones 2014-08-18 13:10) - PLID 63404 - do not return, we may need
				// to update the other lists next
			}
			
			// (j.jones 2014-08-18 13:10) - PLID 63404 - don't color or flash, because we didn't change anything,
			// and also don't return, we may need to check the other lists next
		}
		else {
			//the appointment doesn't exist in our list, so see if it is perhaps a new appointment
			//for today's date, and add it to the list if it is needed

			//only if it is today's date do we add
			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			if(dtStart.GetDayOfYear() != dtToday.GetDayOfYear()
				|| dtStart.GetYear() != dtToday.GetYear()) {

				//not today's date
				return;
			}

			// (j.jones 2006-12-22 09:26) - PLID 23196 - also check the resource filter, don't add if it's not
			// part of our resources
			CString strResourceFilter, strFilteredResourceIDs;
			strFilteredResourceIDs = m_strResourceIDs;
			strFilteredResourceIDs.TrimRight();
			if (!strFilteredResourceIDs.IsEmpty()) {

				// (j.jones 2014-08-15 13:52) - PLID 63185 - compare to the resource IDs provided by the tablechecker
				if (!DoesResourceExistInFilter(strResourceIDs, strFilteredResourceIDs)) {
					//not our resources
					return;
				}

				strFilteredResourceIDs.Replace(" ", ",");
				strResourceFilter.Format(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN (%s))", strFilteredResourceIDs);
			}

			// (j.jones 2009-08-04 08:55) - PLID 24600 - added location filter
			long nFilteredLocationID = -1;
			{
				IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
				if(pRow == NULL) {
					nFilteredLocationID = GetCurrentLocationID();
					m_pLocationCombo->SetSelByColumn(lccID, nFilteredLocationID);
				}
				else {
					nFilteredLocationID = VarLong(pRow->GetValue(lccID));
				}
			}

			// (j.jones 2014-08-15 13:52) - PLID 63185 - compare to the location ID provided by the tablechecker
			if (nFilteredLocationID != nLocationID) {
				//not our location
				return;
			}

			// (j.jones 2014-08-18 13:12) - PLID 63404 - moved the parent row calculation outside of
			// the recordset, since it may requery if we could not find it

			// (j.jones 2007-09-06 15:38) - PLID 27312 - Events now have an hour of -2
			long nParentID = dtStart.GetHour();
			if (bIsEvent) {
				nParentID = -2;
			}

			//find the parent row
			IRowSettingsPtr pParentRow = m_pPatientList->FindByColumn(rmplcID, (long)nParentID, 0, FALSE);
			if (pParentRow == NULL) {
				//if it doesn't exist, we need to requery
				RequeryPatientList(TRUE);

				// (j.jones 2007-02-22 09:03) - PLID 24867 - if we are requerying the patient list,
				// better requery the Waiting Area list too
				RequeryWaitingAreaList();

				// (j.jones 2008-05-29 10:53) - PLID 27797 - requery the Checkout list as well
				RequeryCheckoutList();

				// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
				Flash();
				return;
			}
			else {
				//ensure it's actually a parent!
				if (pParentRow->GetParentRow() != NULL) {
					//yikes, we managed to grab a child! Try again, this should only happen once.

					pParentRow = m_pPatientList->FindByColumn(rmplcID, (long)nParentID, pParentRow->GetNextRow(), FALSE);
					if (pParentRow == NULL) {
						//if it doesn't exist, we need to requery
						RequeryPatientList(TRUE);

						// (j.jones 2007-02-22 09:03) - PLID 24867 - if we are requerying the patient list,
						// better requery the Waiting Area list too
						RequeryWaitingAreaList();

						// (j.jones 2008-05-29 10:53) - PLID 27797 - requery the Checkout list as well
						RequeryCheckoutList();


						// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
						Flash();
						return;
					}
				}
			}

			// (j.jones 2008-11-12 16:42) - PLID 28035 - added purpose text
			// (j.jones 2010-08-27 10:31) - PLID 36975 - the patient name is now calculated by a preference	
			// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			// (c.haag 2010-11-02 11:22) - PLID 39199 - Added ApptWaitingArea

			//**********************************************************************
			//	This code is hit when the appointment is NOT currently in the 
			//	PATIENT list, and should be there, and needs to be added.
			//**********************************************************************

			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
			_RecordsetPtr rsAppt = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PatientID, PatientName, "
				"CASE WHEN PurposeName <> '' THEN TypeName + ' - ' + PurposeName ELSE TypeName END AS Purpose, AptTypeColor, "
				"StatusID, ShowState, IsEvent, ApptWaitingArea "
				"FROM (SELECT PersonT.ID AS PatientID, {SQL} AS PatientName, "
				"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, AptTypeT.Name AS TypeName, CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS AptTypeColor, "
				""
				//find the current status ID, in order of "none", "not checked out", or "checked out" / "ready to check out"
				// (j.jones 2010-12-02 16:49) - PLID 38597 - ignore waiting rooms
				"CASE WHEN AppointmentsT.ID NOT IN (SELECT AppointmentID FROM RoomAppointmentsT "
				"	INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"	WHERE AppointmentID = AppointmentsT.ID) THEN NULL "
				"WHEN AppointmentsT.ID IN (SELECT AppointmentID FROM RoomAppointmentsT "
				"	INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"	WHERE AppointmentID = AppointmentsT.ID AND StatusID NOT IN (0, -1) AND RoomsT.WaitingRoom = 1) THEN NULL "
				"WHEN AppointmentsT.ID IN (SELECT AppointmentID FROM RoomAppointmentsT "
				"	INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"	WHERE AppointmentID = AppointmentsT.ID AND StatusID NOT IN (0, -1) AND RoomsT.WaitingRoom = 0) THEN (SELECT TOP 1 StatusID FROM RoomAppointmentsT INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID WHERE AppointmentID = AppointmentsT.ID AND StatusID NOT IN (0, -1) AND RoomsT.WaitingRoom = 0 ORDER BY CheckInTime DESC) "				
				"WHEN AppointmentsT.ID IN (SELECT AppointmentID FROM RoomAppointmentsT "
				"	INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"	WHERE AppointmentID = AppointmentsT.ID AND StatusID IN (0, -1) AND RoomsT.WaitingRoom = 0) THEN (SELECT TOP 1 StatusID FROM RoomAppointmentsT INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID WHERE AppointmentID = AppointmentsT.ID AND StatusID IN (0, -1) AND RoomsT.WaitingRoom = 0 ORDER BY CheckInTime DESC) "
				"ELSE NULL "
				"END AS StatusID, "
				""
				"AppointmentsT.ShowState, AptShowStateT.WaitingArea AS ApptWaitingArea, "
				""
				"CASE WHEN Convert(datetime, Convert(nvarchar, AppointmentsT.Date, 1)) = Convert(datetime, Convert(nvarchar, GetDate(), 1)) "
				"AND StartTime = EndTime AND DatePart(hh, StartTime) = 0 THEN 1 "
				"ELSE 0 END AS IsEvent "
				""
				"FROM AppointmentsT "
				"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
				"INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"LEFT JOIN AptShowStateT ON AppointmentsT.ShowState = AptShowStateT.ID "
				"WHERE AppointmentsT.Status <> 4 "
				"AND AppointmentsT.ID = {INT} "
				"AND AppointmentsT.LocationID = {INT} "
				"{CONST_STRING}) AS ApptQ",
				GetPatientNameSqlFormat(),
				nResID, nFilteredLocationID,
				strResourceFilter);

			if(!rsAppt->eof) {

				IRowSettingsPtr pNewRow = m_pPatientList->GetNewRow();

				_variant_t varNull;
				varNull.vt = VT_NULL;

				pNewRow->PutValue(rmplcID, varNull);

				pNewRow->PutValue(rmplcParentID, (long)nParentID);
				pNewRow->PutValue(rmplcApptID, (long)nResID);
				pNewRow->PutValue(rmplcPatientID, rsAppt->Fields->Item["PatientID"]->Value);
				pNewRow->PutValue(rmplcPatientName, rsAppt->Fields->Item["PatientName"]->Value);

				// (j.jones 2008-11-12 16:53) - PLID 28035 - added purpose field
				pNewRow->PutValue(rmplcPurpose, rsAppt->Fields->Item["Purpose"]->Value);

				_variant_t varDate = dtStart;
				varDate.vt = VT_DATE;

				long nIsEvent = AdoFldLong(rsAppt, "IsEvent",0);

				pNewRow->PutValue(rmplcApptTime, varDate);
				pNewRow->PutValue(rmplcHour, (long)dtStart.GetHour());
				pNewRow->PutValue(rmplcMinute, (long)dtStart.GetMinute());
				pNewRow->PutValue(rmplcRoomStatusID, rsAppt->Fields->Item["StatusID"]->Value);
				pNewRow->PutValue(rmplcApptShowState, rsAppt->Fields->Item["ShowState"]->Value);
				// (c.haag 2010-10-26 10:07) - PLID 39199
				pNewRow->PutValue(rmplcAptWaitingArea, rsAppt->Fields->Item["ApptWaitingArea"]->Value);

				// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
				pNewRow->PutValue(rmplcAptTypeColor, rsAppt->Fields->Item["AptTypeColor"]->Value);
				if(m_bColorApptListByType) {
					pNewRow->PutCellForeColor(rmplcPatientName, AdoFldLong(rsAppt, "AptTypeColor", 0));
				}
					
				//color the row appropriately
				ColorApptRow(pNewRow);

				//now add the row
				m_pPatientList->AddRowSorted(pNewRow, pParentRow);

				// appointment was added requery resource list	
				// (s.tullis 2013-11-13 14:03) - PLID 37164 - In room manager, the client only wants the resources with scheduled appts to be populated in the Filter on Resource dropdown.
				// (j.jones 2014-08-15 15:37) - PLID 63185 - we do not need to do this unless we are showing all resources,
				// because if we were already filtering by resource, we would not have added the appt. if it didn't qualify for our filter				
				if(strFilteredResourceIDs.IsEmpty()) {
					RequeryResourceList();
				}				

				//now, do we need to re-color the parent row?				
				if(pParentRow != NULL) {
					if(pParentRow->GetFirstChildRow()) {
						//color black
						pParentRow->PutForeColor(RGB(0,0,0));
					}
					else {
						//color gray
						pParentRow->PutForeColor(m_clrFolderEmpty);
					}
				}

				//and now expand/collapse based on this new knowledge
				ExpandCurrentHour(TRUE, TRUE);
				CheckAutoCollapseFinishedHours();

				// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
				Flash();
			}
			rsAppt->Close();
		}
		
		// (j.jones 2007-02-07 10:06) - PLID 24595 - now determine if we need
		// to add/remove/update the Waiting Area

		pRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, nResID, 0, FALSE);
		if(pRow != NULL) {

			// (j.jones 2008-11-13 09:16) - You can reach this code by moving an appt.
			// to a different time while the appt. is in the waiting list. Unlikely,
			// but possible.

			//the appointment exists in our list - should we update it or remove it?
			
			//is the appointment cancelled, or is the Show State no longer 'In'?
			// (c.haag 2010-11-08 13:52) - PLID 39199 - Check the waiting area bit instead of the state
			if(nStatus == 4 || !bWaitingArea) {

				//remove the row
				m_pWaitingAreaList->RemoveRow(pRow);
				// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
				Flash();
				return;
			}

			//has the time changed?
			COleDateTime dtOldStart = VarDateTime(pRow->GetValue(rmwalcApptTime), dtInvalid);
			if(dtOldStart != dtStart && dtOldStart != 0) {

				//is it a different day?
				// (j.jones 2007-02-22 10:41) - PLID 24877 - this did not previously check
				// that the day OR year changed, it errantly checked AND
				if(dtOldStart.GetDayOfYear() != dtStart.GetDayOfYear()
					|| dtOldStart.GetYear() != dtStart.GetYear()) {

					//remove from our view
					m_pWaitingAreaList->RemoveRow(pRow);
					// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
					Flash();
					return;
				}

				//update the start time and sort
				_variant_t varDate = dtStart;
				varDate.vt = VT_DATE;
				pRow->PutValue(rmwalcApptTime, varDate);
				m_pWaitingAreaList->Sort();

				UpdateWaitingAreaWaitTime(nResID);

				// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
				Flash();
			}
			
			//has the resource changed?
			// (j.jones 2007-02-21 17:37) - PLID 24866 - added the resource check
			{
				CString strFilteredResourceIDs;
				strFilteredResourceIDs = m_strResourceIDs;
				strFilteredResourceIDs.TrimRight();
				if (!strFilteredResourceIDs.IsEmpty()) {

					// (j.jones 2014-08-15 13:52) - PLID 63185 - compare to the resource IDs provided by the tablechecker
					if (!DoesResourceExistInFilter(strResourceIDs, strFilteredResourceIDs)) {

						//we're filtering on resources, and the appointment doesn't have one of them, so buh-bye
						m_pWaitingAreaList->RemoveRow(pRow);

						Flash();
						return;
					}
				}
			}

			// (j.jones 2009-08-04 08:55) - PLID 24600 - did the location change?
			long nFilteredLocationID = -1;
			{
				IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
				if(pRow == NULL) {
					nFilteredLocationID = GetCurrentLocationID();
					m_pLocationCombo->SetSelByColumn(lccID, nFilteredLocationID);
				}
				else {
					nFilteredLocationID = VarLong(pRow->GetValue(lccID));
				}
			}

			// (j.jones 2014-08-15 13:52) - PLID 63185 - compare to the location ID provided by the tablechecker
			if (nFilteredLocationID != nLocationID) {

				//doesn't use our location
				m_pWaitingAreaList->RemoveRow(pRow);

				Flash();
				return;
			}

			// (j.jones 2007-02-28 15:46) - PLID 24907 - check to see if the patient changed
			// (j.jones 2008-11-12 17:12) - PLID 28035 - added purpose text, which we also need to check here
			// (j.jones 2010-08-27 10:31) - PLID 36975 - the patient name is now calculated by a preference	
			// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor
			long nRowPatientID = VarLong(pRow->GetValue(rmwalcPatientID), -1);
			CString strPurpose = VarString(pRow->GetValue(rmwalcPurpose), "");

			//**********************************************************************
			//	This code is hit when the appointment is currently in the 
			//	WAITING list, and should be there, and just needs to be updated.
			//**********************************************************************

			// (j.jones 2014-08-18 14:40) - PLID 63404 - call LoadReflectChangedAppointmentInfo to reduce recordset uses
			LoadReflectChangedAppointmentInfo(nResID, eApptInfo);

			if (eApptInfo.m_bExists) {
				// (j.jones 2008-11-12 16:44) - PLID 28035 - see if the patient changed or if the purpose text changed

				//if the patient changed on the appointment, or the type/purpose changed, update the fields
				if (eApptInfo.m_nPatientID != nRowPatientID || eApptInfo.m_strPurpose != strPurpose) {

					pRow->PutValue(rmwalcPatientID, (long)eApptInfo.m_nPatientID);
					pRow->PutValue(rmwalcPatientName, _bstr_t(eApptInfo.m_strPatientName));

					// (j.jones 2008-11-12 17:12) - PLID 28035 - added purpose field
					pRow->PutValue(rmwalcPurpose, _bstr_t(eApptInfo.m_strPurpose));
					// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
					pRow->PutValue(rmwalcAptTypeColor, (long)eApptInfo.m_nAptTypeColor);
					if(m_bColorPurposeColumnByType) {
						pRow->PutCellForeColor(rmwalcPurpose, (long)eApptInfo.m_nAptTypeColor);
					}
				}

				//no need to check the other lists, an appt. would not be in the
				//waiting area and room list at the same time, and if it were in
				//the patient list we would have already caught this case
				
				Flash();
				return;
			}

			//don't check the rooms, the room tablechecker should handle that
		}
		// (c.haag 2010-10-26 10:07) - PLID 39199 - Check bWaitingArea instead of the "In" show state		
		else if(nStatus != 4 && bWaitingArea) {
			//the appointment doesn't exist in our list, and it is not cancelled
			//and it is marked 'In', see if it is perhaps a new appointment
			//for today's date, and not in a room and add it to the list if it is needed

			//is it in a room?
			IRowSettingsPtr pRoomRow = m_pRoomList->FindByColumn(rmrlcApptID, nResID, 0, FALSE);
			if(pRoomRow) {
				//it's in a room, don't add to the Waiting Area
				return;
			}

			//only if it is today's date do we add
			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			if(dtStart.GetDayOfYear() != dtToday.GetDayOfYear()
				|| dtStart.GetYear() != dtToday.GetYear()) {

				//not today's date
				return;
			}

			// (j.jones 2006-12-22 09:26) - PLID 23196 - also check the resource filter, don't add if it's not
			// part of our resources
			CSqlFragment sqlResourceFilter;
			CString strFilteredResourceIDs;
			strFilteredResourceIDs = m_strResourceIDs;
			strFilteredResourceIDs.TrimRight();
			if (!strFilteredResourceIDs.IsEmpty()) {
							
				// (j.jones 2014-08-15 13:52) - PLID 63185 - compare to the resource IDs provided by the tablechecker
				if (!DoesResourceExistInFilter(strResourceIDs, strFilteredResourceIDs)) {
					//not our resources
					return;
				}

				strFilteredResourceIDs.Replace(" ", ",");
				sqlResourceFilter = CSqlFragment(" AND AppointmentsT.ID IN (SELECT ApptRes.AppointmentID FROM AppointmentResourceT ApptRes WHERE ApptRes.ResourceID IN ({INTSTRING}))"
					, strFilteredResourceIDs);
			}

			// (j.jones 2009-08-04 08:55) - PLID 24600 - added location filter
			long nFilteredLocationID = -1;
			{
				IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
				if(pRow == NULL) {
					nFilteredLocationID = GetCurrentLocationID();
					m_pLocationCombo->SetSelByColumn(lccID, nFilteredLocationID);
				}
				else {
					nFilteredLocationID = VarLong(pRow->GetValue(lccID));
				}
			}

			// (j.jones 2014-08-15 13:52) - PLID 63185 - compare to the location ID provided by the tablechecker
			if (nFilteredLocationID != nLocationID) {
				//not our location
				return;
			}

			// (j.armen 2012-03-05 11:20) - PLID 48555 - Select the highest prority recall status for this patient
			// (a.walling 2013-12-12 16:51) - PLID 60005 - RoomManager only needs a smaller subset of the entire recall query - SelectRecallStatusesPastDueOrNeeded
			CSqlFragment sqlRecall("(SELECT MAX(RecallStatusID) AS RecallStatusID FROM ({SQL}) SubQ WHERE SubQ.PatientID = ApptQ.PatientID)", RecallUtils::SelectRecallStatusesPastDueOrNeeded());

			// (j.jones 2007-02-07 10:15) - PLID 24595 - copied/modified from RequeryWaitingAreaList
			// (j.jones 2008-05-29 11:00) - PLID 27797 - ensured this now just shows appointments that have never been in a room
			// (j.jones 2008-11-12 17:13) - PLID 28035 - added purpose field
			// (j.jones 2009-09-11 16:59) - PLID 35145 - AptShowStateHistoryT now has a UserID and not a Username, so the code
			// for the CheckedInBy column needed to be updated
			// (d.lange 2010-08-31 09:27) - PLID 39431 - added provider column
			// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor
			// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource
			// (c.haag 2010-10-26 10:07) - PLID 39199 - Changed how we determine whether the appt should be in the waiting area (added WaitingArea)
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			// (j.jones 2010-12-02 16:26) - PLID 38597 - added support for waiting rooms

			//**********************************************************************
			//	This code is hit when the appointment is NOT currently in the 
			//	WAITING list, and should be there, and needs to be added.
			//**********************************************************************

			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
			_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PatientID, PatientName, {SQL} AS RecallStatusID, "
				"CASE WHEN PurposeName <> '' THEN TypeName + ' - ' + PurposeName ELSE TypeName END AS Purpose, AptTypeColor, "
				"StartTime, TimeMarkedIn, TimeLastSeenByPerson, CheckedInBy, Provider,Resources, "
				"WaitingRoomID, WaitingRoomName, WaitingRoomAppointmentID "
				"FROM (SELECT "
				"AppointmentsT.PatientID AS PatientID, {SQL} AS PatientName, "
				"AppointmentsT.StartTime, TimeMarkedIn, CheckedInBy, "
				""
				//modified from the way the "time last seen" is calculated per room, to include
				//checkout time as a time last seen
				// (j.jones 2010-12-02 16:17) - PLID 38597 - ignore waiting rooms, they do not reset the wait timer
				"(SELECT Max(UpdateTime) AS FirstTimeWithoutPerson FROM RoomAppointmentHistoryT "
				"	WHERE RoomAppointmentHistoryT.RoomAppointmentID IN ("
				"		SELECT RoomAppointmentsT.ID FROM RoomAppointmentsT "
				"		INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"		WHERE RoomsT.WaitingRoom = 0 AND RoomAppointmentsT.AppointmentID = AppointmentsT.ID)"
				"	) "
				"AS TimeLastSeenByPerson, "
				""
				"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, "
				"AptTypeT.Name AS TypeName, CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS AptTypeColor, "
				"Provider.Last + ', ' + Provider.First + ' ' + Provider.Middle AS Provider, "
				" dbo.GetResourceString(AppointmentsT.ID) AS Resources, "
				"Coalesce(WaitingRoomQ.ID, -2) AS WaitingRoomID, "	//-2 means no assigned room
				"Coalesce(WaitingRoomQ.Name, '<No Assigned Room>') AS WaitingRoomName, "
				"WaitingRoomQ.RoomAppointmentID AS WaitingRoomAppointmentID "
				"FROM AppointmentsT "
				"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"LEFT JOIN (SELECT AppointmentID, Max(TimeStamp) AS TimeMarkedIn, Max(UsersT.UserName) AS CheckedInBy "				
				"	FROM AptShowStateHistoryT "
				"	LEFT JOIN UsersT ON AptShowStateHistoryT.UserID = UsersT.PersonID "
				"	LEFT JOIN AptShowStateT on AptShowStateT.ID = AptShowStateHistoryT.ShowStateID "
				"	WHERE AptShowStateT.WaitingArea = CONVERT(BIT,1) "
				"	GROUP BY AppointmentID, AptShowStateT.WaitingArea) "
				"	AS AptShowStateHistoryQ ON AppointmentsT.ID = AptShowStateHistoryQ.AppointmentID "
				"LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				"LEFT JOIN PersonT AS Provider ON PatientsT.MainPhysician = Provider.ID "
				//should not be possible for a patient to be in multiple waiting rooms, if so, they would be displayed here twice
				"LEFT JOIN (SELECT RoomsT.ID, RoomsT.Name, RoomAppointmentsT.AppointmentID, RoomAppointmentsT.ID AS RoomAppointmentID "
				"	FROM RoomAppointmentsT "
				"	INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"	WHERE RoomsT.WaitingRoom = 1 AND RoomAppointmentsT.StatusID NOT IN (0, -1) "
				"	) AS WaitingRoomQ ON AppointmentsT.ID = WaitingRoomQ.AppointmentID "
				//not cancelled, marked 'In' appointments
				"WHERE AppointmentsT.Status <> 4 "
				//in our filtered location
				"AND AppointmentsT.LocationID = {INT} "
				//on today's date
				"AND Convert(datetime, Convert(nvarchar, AppointmentsT.Date, 1)) = Convert(datetime, Convert(nvarchar, GetDate(), 1)) "
				//never been in a room
				// (j.jones 2010-12-01 10:28) - PLID 38597 - ensure they are either marked In and never in any non-waiting room,
				// or are specifically in a waiting room
				"AND ("
					//appts. not in a non-waiting room
				"	(AppointmentsT.ShowState IN (SELECT ID FROM AptShowStateT WHERE WaitingArea = CONVERT(BIT,1)) "
				"	AND "
				"	AppointmentsT.ID NOT IN (SELECT RoomAppointmentsT.AppointmentID FROM RoomAppointmentsT "
				"		INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"		WHERE RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
				"		AND RoomsT.WaitingRoom = 0) "
				"	) "
				"	OR "
					//appts. in a waiting room
				"	AppointmentsT.ID IN (SELECT RoomAppointmentsT.AppointmentID FROM RoomAppointmentsT "
				"		INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
				"		WHERE RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
				"		AND RoomsT.WaitingRoom = 1 "
				"		AND RoomAppointmentsT.StatusID NOT IN (0, -1) "
				"	) "
				") "
				//optionally filtered by resource
				"{SQL} "
				//and lastly filtered by this appt. ID
				"AND AppointmentsT.ID = {INT}) AS ApptQ", 
				sqlRecall, GetPatientNameSqlFormat(), nFilteredLocationID, sqlResourceFilter, nResID);

			if (!rs->eof)
			{
				long nWaitingRoomID = AdoFldLong(rs, "WaitingRoomID");
				IRowSettingsPtr pWaitingRoomFilterRow = m_pWaitingRoomCombo->GetCurSel();
				if (pWaitingRoomFilterRow != nullptr)
				{
					long nFilteredWaitingRoomID = VarLong(pWaitingRoomFilterRow->GetValue(wrccID), -1);
					if (nFilteredWaitingRoomID != -1)
					{
						// A waiting room filter is selected so only add a row if the selected waiting room
						// matches the filter.
						if (nWaitingRoomID != nFilteredWaitingRoomID)
						{
							return;
						}
					}
				}

				IRowSettingsPtr pNewRow = m_pWaitingAreaList->GetNewRow();

				pNewRow->PutValue(rmwalcApptID, (long)nResID);
				// (j.jones 2010-12-02 16:29) - PLID 38597 - supported waiting rooms
				pNewRow->PutValue(rmwalcWaitingRoomID, nWaitingRoomID);
				pNewRow->PutValue(rmwalcWaitingRoomName, rs->Fields->Item["WaitingRoomName"]->Value);
				pNewRow->PutValue(rmwalcWaitingRoomAppointmentID, rs->Fields->Item["WaitingRoomAppointmentID"]->Value);
				pNewRow->PutValue(rmwalcPatientID, rs->Fields->Item["PatientID"]->Value);
				pNewRow->PutValue(rmwalcPatientName, rs->Fields->Item["PatientName"]->Value);
				// (j.jones 2008-11-12 17:13) - PLID 28035 - added purpose field
				pNewRow->PutValue(rmwalcPurpose, rs->Fields->Item["Purpose"]->Value);
				pNewRow->PutValue(rmwalcApptTime, rs->Fields->Item["StartTime"]->Value);
				pNewRow->PutValue(rmwalcCheckInTime, rs->Fields->Item["TimeMarkedIn"]->Value);
				pNewRow->PutValue(rmwalcTimeLastSeen, rs->Fields->Item["TimeLastSeenByPerson"]->Value);
				pNewRow->PutValue(rmwalcWaiting, _bstr_t(""));
				// (d.lange 2010-08-31 09:56) - PLID 39431 - added the provider column
				pNewRow->PutValue(rmwalcProvider, rs->Fields->Item["Provider"]->Value);
				// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource
				pNewRow->PutValue(rmwalcResources, rs->Fields->Item["Resources"]->Value);
				
				// (j.jones 2009-08-03 09:34) - PLID 26862 - added checked in username
				pNewRow->PutValue(rmwalcCheckedInBy, rs->Fields->Item["CheckedInBy"]->Value);
				// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
				pNewRow->PutValue(rmwalcAptTypeColor, rs->Fields->Item["AptTypeColor"]->Value);
				if(m_bColorPurposeColumnByType) {
					pNewRow->PutCellForeColor(rmwalcPurpose, AdoFldLong(rs, "AptTypeColor", 0));
				}
				// (d.lange 2010-11-29 12:01) - PLID 40295 - added Preview EMN column
				pNewRow->PutValue(rmwalcPreviewEMN, (long)m_hIconPreview);
				// (j.armen 2012-03-05 11:19) - PLID 48555 - Get the RecallStatusID and Ensure the Flag Column to make sure that the column is set properly
				pNewRow->PutValue(rmwalcRecallStatusID, rs->Fields->Item["RecallStatusID"]->Value);

				m_pWaitingAreaList->AddRowSorted(pNewRow,NULL);

				EnsureRecallFlagColumn(m_pWaitingAreaList, rmwalcRecallStatusID, rmwalcRecallStatus, "ShowRoomManagerWaitingAreaColumnRecall");

				UpdateWaitingAreaWaitTime(nResID);

				// (j.jones 2007-02-20 13:46) - PLID 24365 - flash to reflect the change (only if minimized)
				Flash();

				// (j.jones 2014-08-18 13:16) - PLID 63404 - 				
				//no need to check the other lists, an appt. would not be in the
				//waiting area and room list at the same time
				return;
			}
			rs->Close();
		}

		// (j.jones 2009-08-04 10:25) - PLID 35099 - do we need to update the checkout list?
		pRow = m_pCheckoutList->FindByColumn(rmclcApptID, nResID, 0, FALSE);
		if(pRow != NULL) {

			// You can reach this code by moving an appt. to a different time
			// while the appt. is in the checkout list. Unlikely, but possible.

			//the appointment exists in our list - should we update it or remove it?
			
			//is the appointment cancelled?
			// (j.jones 2010-12-09 11:38) - PLID 41779 - This code used to incorrectly check to see
			// if the appt. was marked 'In' (or a status that was configured to show in the waiting area),
			// but it should not filter out any status, not even 'Out', because the checkout list query
			// does not exclude any status but cancelled. If the patient is in a room and marked
			// "ready to checkout", it should show in this list regardless of its in/out status.
			if(nStatus == 4) {
				//remove the row
				m_pCheckoutList->RemoveRow(pRow);
				//flash to reflect the change (only if minimized)
				Flash();
				return;
			}

			//has the time changed?
			COleDateTime dtOldStart = VarDateTime(pRow->GetValue(rmclcApptTime), dtInvalid);
			if(dtOldStart != dtStart && dtOldStart != 0) {

				//is it a different day?
				if(dtOldStart.GetDayOfYear() != dtStart.GetDayOfYear()
					|| dtOldStart.GetYear() != dtStart.GetYear()) {

					//remove from our view
					m_pCheckoutList->RemoveRow(pRow);
					//flash to reflect the change (only if minimized)
					Flash();
					return;
				}

				//update the start time and sort
				_variant_t varDate = dtStart;
				varDate.vt = VT_DATE;
				pRow->PutValue(rmclcApptTime, varDate);
				m_pCheckoutList->Sort();

				UpdateCheckoutListWaitTime(nResID);

				//flash to reflect the change (only if minimized)
				Flash();
			}

			//has the resource changed?
			{
				CString strFilteredResourceIDs;
				strFilteredResourceIDs = m_strResourceIDs;
				strFilteredResourceIDs.TrimRight();
				if (!strFilteredResourceIDs.IsEmpty()) {

					// (j.jones 2014-08-15 13:52) - PLID 63185 - compare to the resource IDs provided by the tablechecker
					if (!DoesResourceExistInFilter(strResourceIDs, strFilteredResourceIDs)) {

						//we're filtering on resources, and the appointment doesn't have one of them, so buh-bye
						m_pCheckoutList->RemoveRow(pRow);

						Flash();
						return;
					}
				}
			}

			// (j.jones 2009-08-04 08:55) - PLID 24600 - did the location change?
			long nFilteredLocationID = -1;
			{
				IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
				if(pRow == NULL) {
					nFilteredLocationID = GetCurrentLocationID();
					m_pLocationCombo->SetSelByColumn(lccID, nFilteredLocationID);
				}
				else {
					nFilteredLocationID = VarLong(pRow->GetValue(lccID));
				}
			}

			// (j.jones 2014-08-15 13:52) - PLID 63185 - compare to the location ID provided by the tablechecker
			if (nFilteredLocationID != nLocationID) {

				//doesn't use our location
				m_pCheckoutList->RemoveRow(pRow);

				Flash();
				return;
			}

			//check to see if the patient changed
			long nRowPatientID = VarLong(pRow->GetValue(rmclcPatientID), -1);
			CString strPurpose = VarString(pRow->GetValue(rmclcPurpose), "");
			// (j.jones 2010-08-27 10:31) - PLID 36975 - the patient name is now calculated by a preference	
			// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor

			//**********************************************************************
			//	This code is hit when the appointment is currently in the 
			//	CHECKOUT list, and should be there, and just needs to be updated.
			//**********************************************************************

			// (j.jones 2014-08-18 14:40) - PLID 63404 - call LoadReflectChangedAppointmentInfo to reduce recordset uses
			LoadReflectChangedAppointmentInfo(nResID, eApptInfo);

			if (eApptInfo.m_bExists) {
				// (j.jones 2008-11-12 16:44) - PLID 28035 - see if the patient changed or if the purpose text changed

				//if the patient changed on the appointment, or the type/purpose changed, update the fields
				if (eApptInfo.m_nPatientID != nRowPatientID || eApptInfo.m_strPurpose != strPurpose) {

					pRow->PutValue(rmclcPatientID, (long)eApptInfo.m_nPatientID);
					pRow->PutValue(rmclcPatientName, _bstr_t(eApptInfo.m_strPatientName));
					pRow->PutValue(rmclcPurpose, _bstr_t(eApptInfo.m_strPurpose));
					// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
					pRow->PutValue(rmclcAptTypeColor, (long)eApptInfo.m_nAptTypeColor);
					if(m_bColorPurposeColumnByType) {
						pRow->PutCellForeColor(rmclcPurpose, (long)eApptInfo.m_nAptTypeColor);
					}
				}

				//no need to check the other lists, an appt. would not be in the
				//checkout and room list at the same time, and if it were in
				//the patient list we would have already caught this case
				
				Flash();
				return;
			}

			//don't check the rooms, the room tablechecker should handle that
		}
		// (c.haag 2010-10-26 10:07) - PLID 39199 - Check bWaitingArea instead of the "In" show state
		else if(nStatus != 4 && bWaitingArea) {
			//the appointment doesn't exist in our list, and it is not cancelled
			//and it is marked 'In', see if it is perhaps a new appointment
			//for today's date, and not in a room and add it to the list if it is needed

			//is it in a room?
			IRowSettingsPtr pRoomRow = m_pRoomList->FindByColumn(rmrlcApptID, nResID, 0, FALSE);
			if(pRoomRow) {
				//it's in a room, don't add to the checkout list
				return;
			}

			//only if it is today's date do we add
			COleDateTime dtToday = COleDateTime::GetCurrentTime();
			if(dtStart.GetDayOfYear() != dtToday.GetDayOfYear()
				|| dtStart.GetYear() != dtToday.GetYear()) {

				//not today's date
				return;
			}

			//also check the resource filter, don't add if it's not part of our resources
			CString strResourceFilter, strFilteredResourceIDs;
			strFilteredResourceIDs = m_strResourceIDs;
			strFilteredResourceIDs.TrimRight();
			if (!strFilteredResourceIDs.IsEmpty()) {
								
				// (j.jones 2014-08-15 13:52) - PLID 63185 - compare to the resource IDs provided by the tablechecker
				if (!DoesResourceExistInFilter(strResourceIDs, strFilteredResourceIDs)) {
					//not our resources
					return;
				}

				strFilteredResourceIDs.Replace(" ", ",");
				strResourceFilter.Format(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN (%s))", strFilteredResourceIDs);
			}

			// (j.jones 2009-08-04 08:55) - PLID 24600 - added location filter
			long nFilteredLocationID = -1;
			{
				IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
				if(pRow == NULL) {
					nFilteredLocationID = GetCurrentLocationID();
					m_pLocationCombo->SetSelByColumn(lccID, nFilteredLocationID);
				}
				else {
					nFilteredLocationID = VarLong(pRow->GetValue(lccID));
				}
			}

			// (j.jones 2014-08-15 13:52) - PLID 63185 - compare to the location ID provided by the tablechecker
			if (nFilteredLocationID != nLocationID) {
				//not our location
				return;
			}

			// (j.armen 2012-03-05 11:20) - PLID 48555 - Select the highest prority recall status for this patient
			// (a.walling 2013-12-12 16:51) - PLID 60005 - RoomManager only needs a smaller subset of the entire recall query - SelectRecallStatusesPastDueOrNeeded
			CSqlFragment sqlRecall("(SELECT MAX(RecallStatusID) AS RecallStatusID FROM ({SQL}) SubQ WHERE SubQ.PatientID = ApptQ.PatientID)", RecallUtils::SelectRecallStatusesPastDueOrNeeded());

			//copied/modified from RequeryCheckoutList
			// (j.jones 2010-08-27 10:31) - PLID 36975 - the patient name is now calculated by a preference	
			// (d.lange 2010-08-31 09:30) - PLID 39431 - added the provider column
			// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor
			// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource
			// (c.haag 2010-10-26 10:07) - PLID 39199 - Changed how we determine whether the appt should be in the waiting area (added WaitingArea)
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized

			//**********************************************************************
			//	This code is hit when the appointment is NOT currently in the 
			//	CHECKOUT list, and should be there, and needs to be added.
			//**********************************************************************

			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
			_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT RoomAppointmentID, AppointmentID, "
				"PatientID, PatientName, {SQL} AS RecallStatusID, StartTime, TimeMarkedIn, TimeLeftRoom, LastUpdateUserID, UserName, "
				"CASE WHEN PurposeName <> '' THEN TypeName + ' - ' + PurposeName ELSE TypeName END AS Purpose, AptTypeColor, Provider,Resources "
				""
				"FROM (SELECT RoomAppointmentsT.ID AS RoomAppointmentID, AppointmentsT.ID AS AppointmentID, "
				"AppointmentsT.PatientID AS PatientID, {SQL} AS PatientName, "
				"AppointmentsT.StartTime, TimeMarkedIn, "
				""				
				"	(SELECT Max(UpdateTime) AS TimeLeftRoom FROM RoomAppointmentHistoryT "
				"	WHERE RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID) "
				"	AS TimeLeftRoom, "
				""
				"RoomAppointmentsT.LastUpdateUserID, UsersT.Username, "
				"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, "
				"AptTypeT.Name AS TypeName, CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS AptTypeColor, "
				"Provider.Last + ', ' + Provider.First + ' ' + Provider.Middle AS Provider, "
				" dbo.GetResourceString(AppointmentsT.ID) AS Resources "
				"FROM AppointmentsT "
				"INNER JOIN RoomAppointmentsT ON AppointmentsT.ID = RoomAppointmentsT.AppointmentID "
				"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"LEFT JOIN (SELECT AppointmentID, Max(TimeStamp) AS TimeMarkedIn "
				"	FROM AptShowStateHistoryT "
				"	LEFT JOIN AptShowStateT on AptShowStateT.ID = AptShowStateHistoryT.ShowStateID "
				"	WHERE AptShowStateT.WaitingArea = CONVERT(BIT,1) GROUP BY AppointmentID, AptShowStateT.WaitingArea) "
				"	AS AptShowStateHistoryQ ON AppointmentsT.ID = AptShowStateHistoryQ.AppointmentID "
				"LEFT JOIN UsersT ON RoomAppointmentsT.LastUpdateUserID = UsersT.PersonID "
				"LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
				"LEFT JOIN PersonT AS Provider ON PatientsT.MainPhysician = Provider.ID "
				//not cancelled
				"WHERE AppointmentsT.Status <> 4 "
				//uses our filtered location
				"AND AppointmentsT.LocationID = {INT} "
				//on today's date
				"AND Convert(datetime, Convert(nvarchar, AppointmentsT.Date, 1)) = Convert(datetime, Convert(nvarchar, GetDate(), 1)) "
				//currently marked 'Ready To Check Out'
				"AND RoomAppointmentsT.StatusID = 0 "
				//optionally filtered by resource
				"{CONST_STRING}"
				//and lastly filtered by this appt. ID
				"AND AppointmentsT.ID = {INT}) AS ApptQ", 
				sqlRecall, GetPatientNameSqlFormat(), nFilteredLocationID, strResourceFilter, nResID);

			//if not empty, re-add to the list
			if(!rs->eof) {
				IRowSettingsPtr pNewRow = m_pCheckoutList->GetNewRow();
				
				pNewRow->PutValue(rmclcRoomAppointmentID, rs->Fields->Item["RoomAppointmentID"]->Value);
				pNewRow->PutValue(rmclcApptID, (long)nResID);
				pNewRow->PutValue(rmclcPatientID, rs->Fields->Item["PatientID"]->Value);
				pNewRow->PutValue(rmclcPatientName, rs->Fields->Item["PatientName"]->Value);
				pNewRow->PutValue(rmclcPurpose, rs->Fields->Item["Purpose"]->Value);
				pNewRow->PutValue(rmclcApptTime, rs->Fields->Item["StartTime"]->Value);
				pNewRow->PutValue(rmclcCheckInTime, rs->Fields->Item["TimeMarkedIn"]->Value);
				pNewRow->PutValue(rmclcTimeLeftRoom, rs->Fields->Item["TimeLeftRoom"]->Value);
				pNewRow->PutValue(rmclcWaiting, _bstr_t(""));
				// (d.lange 2010-08-31 09:32) - PLID 39431 - added the provider column
				pNewRow->PutValue(rmclcProvider, rs->Fields->Item["Provider"]->Value);
				// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource 
				pNewRow->PutValue(rmclcResources , rs->Fields->Item["Resources"]->Value);
				pNewRow->PutValue(rmclcLastUserID, rs->Fields->Item["LastUpdateUserID"]->Value);
				pNewRow->PutValue(rmclcLastUserName, rs->Fields->Item["Username"]->Value);
				// (j.jones 2010-08-31 10:58) - PLID 35012 - added apt. type color
				pNewRow->PutValue(rmclcAptTypeColor, rs->Fields->Item["AptTypeColor"]->Value);
				if(m_bColorPurposeColumnByType) {
					pNewRow->PutCellForeColor(rmclcPurpose, AdoFldLong(rs, "AptTypeColor", 0));
				}
				// (d.lange 2010-11-29 12:50) - PLID 40295 - added Preview EMNs column
				pNewRow->PutValue(rmclcPreviewEMN, (long)m_hIconPreview);
				// (j.armen 2012-03-05 11:19) - PLID 48555 - Get the RecallStatusID and Ensure the Flag Column to make sure that the column is set properly
				pNewRow->PutValue(rmclcRecallStatusID, rs->Fields->Item["RecallStatusID"]->Value);

				m_pCheckoutList->AddRowSorted(pNewRow,NULL);

				EnsureRecallFlagColumn(m_pCheckoutList, rmclcRecallStatusID, rmclcRecallStatus, "ShowRoomManagerCheckoutColumnRecall");

				UpdateCheckoutListWaitTime(nResID);

				//flash to reflect the change (only if minimized)
				Flash();
			}
			rs->Close();
		}

	}NxCatchAll("Error in CRoomManagerDlg::ReflectChangedAppointment");
}

// (j.jones 2014-08-15 14:06) - PLID 63185 - given two space-delimited resource strings,
// does one have any ID that exists in the other?
bool CRoomManagerDlg::DoesResourceExistInFilter(CString strApptResourceIDs, CString strResourceIDFilter)
{
	if (strResourceIDFilter.IsEmpty()) {
		//this should not have been called on an empty string
		ASSERT(FALSE);

		//we interpret this to mean no filter is in use, so yes
		//the appointment should be shown
		return true;
	}

	if (strApptResourceIDs.IsEmpty()) {
		//this should not have been called on an empty string
		ASSERT(FALSE);

		//technically no, none of the provided IDs are in our filter
		return false;
	}
	
	if (strResourceIDFilter == strApptResourceIDs) {
		//common if we're filtering on one resource and the appointment is for that resource
		return true;
	}

	CDWordArray aryApptResourceIDs;
	ParseDelimitedStringToDWordArray(strApptResourceIDs, " ", aryApptResourceIDs);

	CDWordArray aryFilteredResourceIDs;
	ParseDelimitedStringToDWordArray(strResourceIDFilter, " ", aryFilteredResourceIDs);

	for (int i = 0; i < aryApptResourceIDs.GetSize(); i++) {
		for (int j = 0; j < aryFilteredResourceIDs.GetSize(); j++) {

			if (aryApptResourceIDs[i] == aryFilteredResourceIDs[j]) {
				//the appt. has a resource ID in our filter
				return true;
			}
		}
	}

	//if we get here, the appt. did not have a resource ID in our filter
	return false;
}

void CRoomManagerDlg::ColorApptRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if(pRow) {

		long nStatusID = VarLong(pRow->GetValue(rmplcRoomStatusID), -2);
		long nShowState = VarLong(pRow->GetValue(rmplcApptShowState), 0);
		long nAptTypeColor = VarLong(pRow->GetValue(rmplcAptTypeColor), 0);

		//remove the purpose color if there is one
		pRow->PutCellForeColor(rmplcPatientName, dlColorNotSet);
		
		// (j.jones 2008-05-30 09:03) - PLID 27797 - color "ready to check out" appointments correctly
		if(nStatusID == 0) {
			//ready to check out
			pRow->PutForeColor(m_clrApptReadyToCheckOut);
		}
		else if(nStatusID == -1) {
			//checked out
			pRow->PutForeColor(m_clrApptCheckedOut);
		}
		else if(nStatusID != -2) {
			//checked in to a room
			pRow->PutForeColor(m_clrApptCheckedIn);
		}
		else if(nShowState == 3) {
			//not checked in/out, and No Show
			pRow->PutForeColor(m_clrApptNoShow);
		}
		else {
			//normal (could be marked in, but is not in a non-waiting room)
			pRow->PutForeColor(RGB(0,0,0));
		}

		// (j.jones 2010-08-31 15:24) - PLID 35012 - now color the patient name by the apt. type color,
		// if enabled (leave gray if checked out)
		if(m_bColorApptListByType && nStatusID != -1) {
			pRow->PutCellForeColor(rmplcPatientName, nAptTypeColor);
		}
	}
}

void CRoomManagerDlg::ShowAppointment(long nAppointmentID)
{
	try {

		// (j.jones 2006-10-23 12:10) - PLID 23169 - be able to expand a given appointment

		//find the appointment in our list
		IRowSettingsPtr pRow = m_pPatientList->FindByColumn(rmplcApptID, nAppointmentID, 0, FALSE);
		if(pRow) {
			IRowSettingsPtr pParentRow = pRow->GetParentRow();
			if(pParentRow) {
				pParentRow->PutExpanded(VARIANT_TRUE);
				m_pPatientList->EnsureRowInView(pRow->GetLastChildRow());
				m_pPatientList->EnsureRowInView(pRow->GetFirstChildRow());
			}
		}
		else {
			m_nPendingShowAppointment = nAppointmentID;
		}

	}NxCatchAll("Error in CRoomManagerDlg::ShowAppointment");
}

// (j.jones 2006-12-21 16:27) - PLID 23196 - added resource filter
void CRoomManagerDlg::OnSelChosenRoomMgrResourceCombo(LPDISPATCH lpRow) 
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(pRow == NULL) {
			//disallow this, select "all resources"
			pRow = m_pResourceCombo->SetSelByColumn(rfccID, (long)-1);

			if(pRow == NULL) {
				//this should be impossible
				ASSERT(FALSE);
				return;
			}
		}

		long nID = VarLong(pRow->GetValue(rfccID));

		//if the ID is -2, it's the "multiple resources" row
		if(nID == -2) {
			m_pResourceCombo->PutCurSel(NULL);
			OnMultipleResources();
			//OnMultipleResources handles the ConfigRT and the requery
			return;
		}
		//if the ID is -1, it's the "all resources" row
		else if(nID == -1) {

			//do not requery if we didn't change anything
			if(m_strResourceIDs == "") {
				ToggleResourceDisplay();
				return;
			}

			m_strResourceIDs = "";
		}
		else {

			CString strResourceIDs;
			strResourceIDs.Format("%li", nID);

			//do not requery if we didn't change anything
			if(m_strResourceIDs == strResourceIDs) {
				ToggleResourceDisplay();
				return;
			}

			m_strResourceIDs = strResourceIDs;
		}

		//save this filter to the user's ConfigRT
		SetRemotePropertyText("RoomManagerResourceFilter", m_strResourceIDs, 0, GetCurrentUserName());

		//now rebuild the list with our changes
		RequeryPatientList(TRUE);
		
		RequeryWaitingAreaList();
		// (j.jones 2008-05-29 10:53) - PLID 27797 - requery the Checkout list
		RequeryCheckoutList();

	}NxCatchAll("Error in CRoomManagerDlg::OnSelChosenRoomMgrResourceCombo");
}

void CRoomManagerDlg::OnMultipleResources()
{
	try {

		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "AptPurposeT");

		//see if we have anything already
		CString str = m_strResourceIDs + " ";
		str.TrimLeft();
		long nSpace = str.Find(" ");
		while(nSpace > 0) {
			dlg.PreSelect(atoi(str.Left(nSpace)));
			str = str.Right(str.GetLength() - (nSpace + 1));

			nSpace = str.Find(" ");
		}

		dlg.m_strNameColTitle = "Resource";

		int res = dlg.Open("ResourceT", "Inactive = 0", "ResourceT.ID", "ResourceT.Item", "Select Resources");

		BringWindowToTop();

		if(res == IDCANCEL) {
			ToggleResourceDisplay();
			return;
		}

		//save all our id's for later parsing
		CString strResourceIDs = dlg.GetMultiSelectIDString();

		//do not requery if we didn't change anything
		if(m_strResourceIDs == strResourceIDs) {
			ToggleResourceDisplay();
			return;
		}

		m_strResourceIDs = strResourceIDs;

		ToggleResourceDisplay();
		

		//now rebuild the list with our changes
		RequeryPatientList(TRUE);
	
		// (j.jones 2009-08-10 10:04) - PLID 35146 - requery the waiting list and checkout lists as well
		RequeryWaitingAreaList();		
		RequeryCheckoutList();

		//save this filter to the user's ConfigRT
		SetRemotePropertyText("RoomManagerResourceFilter", m_strResourceIDs, 0, GetCurrentUserName());

	} NxCatchAll("Error in CRoomManagerDlg::OnMultipleResources");
}

void CRoomManagerDlg::ToggleResourceDisplay()
{
	try {
		//if we only have 1 item, select it in the datalist, don't bother setting this all up
		CString strTemp = m_strResourceIDs;
		if(strTemp.Find(" ") == -1) {
			m_strResourceList = "";
			m_bIsResourceComboHidden = FALSE;
			((CWnd*)GetDlgItem(IDC_ROOM_MGR_RESOURCE_COMBO))->ShowWindow(SW_SHOW);
			// (s.dhole 2010-06-30 17:03) - PLID  38947 Add some color to the Room Manager
			ShowDlgItem(IDC_ROOM_MGR_MULTI_RESOURCE_LABEL, SW_HIDE);
			InvalidateDlgItem(IDC_ROOM_MGR_MULTI_RESOURCE_LABEL);

			if(!strTemp.IsEmpty())
				m_pResourceCombo->SetSelByColumn(rfccID, long(atoi(strTemp)));
			else
				m_pResourceCombo->SetSelByColumn(rfccID, (long)-1);
			return;
		}

		m_bIsResourceComboHidden = TRUE;

		//populate the readable string
		m_strResourceList = GetStringOfResources();
		// (s.dhole 2010-06-30 17:02) - PLID  38947 Add some color to the Room Manager
		m_nxlRoomMgrMultiResourceLabel.SetText(m_strResourceList);
		m_nxlRoomMgrMultiResourceLabel.SetType(dtsHyperlink);
		ShowDlgItem(IDC_ROOM_MGR_RESOURCE_COMBO, SW_HIDE);
		InvalidateDlgItem(IDC_ROOM_MGR_RESOURCE_COMBO);
		ShowDlgItem(IDC_ROOM_MGR_MULTI_RESOURCE_LABEL, SW_SHOW);
		InvalidateDlgItem(IDC_ROOM_MGR_MULTI_RESOURCE_LABEL);
	
	}NxCatchAll("Error displaying Resource filter information.");
}

CString CRoomManagerDlg::GetStringOfResources() {
	
	CString str = "";

	CString strResources = m_strResourceIDs;
	strResources.TrimLeft();
	strResources.TrimRight();
	strResources.Replace(" "," OR ID = ");
	// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
	_RecordsetPtr rs = CreateRecordset(GetRemoteDataSnapshot(), "SELECT Item FROM ResourceT WHERE ID = %s", strResources);
	while(!rs->eof) {
		str += AdoFldString(rs, "Item","");
		str += ", ";
		rs->MoveNext();
	}
	rs->Close();

	str.TrimRight(", ");

	return str;
}
// (s.dhole 2010-06-30 17:05) - PLID  38947 Add some color to the Room Manager
//void CRoomManagerDlg::DrawResourceLabel(CDC *pdc)
//{
//	// Draw the resources
//	//if(m_bIsResourceComboHidden)
//
//	// (j.jones 2008-05-01 16:42) - PLID 29874 - Set background color to transparent
//	DrawTextOnDialog(this, pdc, m_rcMultiResourceLabel, m_strResourceList, m_bIsResourceComboHidden?dtsHyperlink:dtsDisabledHyperlink, false, DT_LEFT, true, false, 0);
//}

// (s.dhole 2010-06-30 17:05) - PLID  38947 Add some color to the Room Manager
BOOL CRoomManagerDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (m_bIsResourceComboHidden)
	{
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		GetDlgItem(IDC_ROOM_MGR_MULTI_RESOURCE_LABEL)->GetWindowRect(rc);
		ScreenToClient(&rc);
		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}
	}
	return CNxModelessOwnedDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (s.dhole 2010-06-30 17:01) - PLID  38947 Add some color to the Room Manager
LRESULT CRoomManagerDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	OnMultipleResources();
	return 0;
}
// (s.dhole 2010-06-30 16:59) - PLID  38947 Add some color to the Room Manager
//void CRoomManagerDlg::OnLButtonDown(UINT nFlags, CPoint point) 
//{
//	DoClickHyperlink(nFlags, point);
//	CNxModelessOwnedDialog::OnLButtonDown(nFlags, point);
//}
//
//void CRoomManagerDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
//{
//	DoClickHyperlink(nFlags, point);
//	CNxModelessOwnedDialog::OnLButtonDblClk(nFlags, point);
//}
//
//void CRoomManagerDlg::DoClickHyperlink(UINT nFlags, CPoint point)
//{
//	if (m_bIsResourceComboHidden) {
//		if (m_rcMultiResourceLabel.PtInRect(point)) {
//			OnMultipleResources();
//		}
//	}
//}

//void CRoomManagerDlg::OnPaint() 
//{
//	CPaintDC dc(this); // device context for painting
//	
//	DrawResourceLabel(&dc);
//}

// (s.tullis 2013-11-01 10:30) - PLID 37164 - In room manager, the client only wants the resources with scheduled appts to be populated in the Filter on Resource dropdown.
void CRoomManagerDlg::RequeryResourceList()
{
	// (j.jones 2014-08-15 15:04) - PLID 63384 - we need to remember the current selection, if one exists
	long nCurID = -1;
	{
		IRowSettingsPtr pCurRow = m_pResourceCombo->GetCurSel();
		if (pCurRow) {
			nCurID = VarLong(pCurRow->GetValue(rfccID), -1);
		}
	}

	CString literalDateFrom = FormatDateTimeForSql(AsDateNoTime(COleDateTime::GetCurrentTime()));
	CString literalDateTo = FormatDateTimeForSql(AsDateNoTime(COleDateTime::GetCurrentTime() + COleDateTimeSpan(1, 0, 0, 0)));

	CString strResourceWhere = FormatString("Inactive = 0 AND ID IN (SELECT ResourceID FROM AppointmentResourceT "
		"INNER JOIN AppointmentsT ON AppointmentResourceT.AppointmentID = AppointmentsT.ID WHERE (AppointmentsT.StartTime >= '%s' AND (AppointmentsT.StartTime < '%s' AND Status <> 4)))"
		, literalDateFrom, literalDateTo
	);
	m_pResourceCombo->WhereClause= _bstr_t(strResourceWhere);
	m_pResourceCombo->Requery();
	//add "all resources" and "multiple resources"
	IRowSettingsPtr pRow = m_pResourceCombo->GetNewRow();
	pRow->PutValue(rfccID, (long)-1);
	pRow->PutValue(rfccName, " <All Resources>");
	m_pResourceCombo->AddRowSorted(pRow, NULL);
	m_pResourceCombo->PutCurSel(pRow);
	pRow = m_pResourceCombo->GetNewRow();
	pRow->PutValue(rfccID, (long)-2);
	pRow->PutValue(rfccName, " <Multiple Resources>");
	m_pResourceCombo->AddRowSorted(pRow, NULL);

	// (j.jones 2014-08-15 15:04) - PLID 63384 - we need to remember the current selection, if one exists
	IRowSettingsPtr pNewlySelectedRow = m_pResourceCombo->SetSelByColumn(rfccID, nCurID);
	if (pNewlySelectedRow == NULL) {
		//their selected resource is not in the list, so revert to "all"
		pNewlySelectedRow = m_pResourceCombo->SetSelByColumn(rfccID, -1);
		//apply this filter
		OnSelChosenRoomMgrResourceCombo(pNewlySelectedRow);
	}
}

// (j.jones 2007-02-07 09:37) - PLID 24595 - requeries the Waiting Area list
void CRoomManagerDlg::RequeryWaitingAreaList()
{
	//utilize the resource filter
	CString strResourceFilter, strResourceIDs;
	strResourceIDs = m_strResourceIDs;
	strResourceIDs.TrimRight();
	if(!strResourceIDs.IsEmpty()) {
		strResourceIDs.Replace(" ", ",");
		strResourceFilter.Format(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN (%s))", strResourceIDs);
	}

	// (j.jones 2009-08-04 08:55) - PLID 24600 - added location filter
	long nLocationID = -1;
	{
		IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
		if(pRow == NULL) {
			nLocationID = GetCurrentLocationID();
			m_pLocationCombo->SetSelByColumn(lccID, nLocationID);			
		}
		else {
			nLocationID = VarLong(pRow->GetValue(lccID));
		}
	}	

	// (j.jones 2010-12-02 09:18) - PLID 38597 - added a waiting room filter
	long nWaitingRoomID = -1;
	CString strWaitingRoomFilter = "";
	//-1 is all rooms, -2 is no assigned room
	{
		IRowSettingsPtr pRow = m_pWaitingRoomCombo->GetCurSel();
		if(pRow == NULL) {
			//set to all rooms
			m_pWaitingRoomCombo->SetSelByColumn(wrccID, (long)-1);			
		}
		else {
			nWaitingRoomID = VarLong(pRow->GetValue(wrccID));
		}

		//if -1, we use no filter, but -2 is a valid filter value
		if(nWaitingRoomID != -1) {
			strWaitingRoomFilter.Format(" AND Coalesce(WaitingRoomQ.ID, -2) = %li ", nWaitingRoomID);
		}
	}
	
	// (a.walling 2014-10-15 06:44) - PLID 63884 - AppointmentsT range queries on StartTime generate disastrously inaccurate cardinality estimates in the SQL optimizer

	CString literalDateFrom = FormatDateTimeForSql(AsDateNoTime(COleDateTime::GetCurrentTime()));
	CString literalDateTo = FormatDateTimeForSql(AsDateNoTime(COleDateTime::GetCurrentTime() + COleDateTimeSpan(1, 0, 0, 0)));

	// (j.armen 2012-03-05 11:20) - PLID 48555 - Select the highest prority recall status for this patient
	// (a.walling 2013-12-12 16:51) - PLID 60005 - RoomManager only needs a smaller subset of the entire recall query - SelectRecallStatusesPastDueOrNeeded
	CSqlFragment sqlRecall("(SELECT MAX(RecallStatusID) AS RecallStatusID FROM ({SQL}) SubQ WHERE SubQ.PatientID = AppointmentsT.PatientID)", RecallUtils::SelectRecallStatusesPastDueOrNeeded());

	// (j.jones 2008-05-29 11:00) - PLID 27797 - ensured this now just shows appointments that have never been in a room

	//display all of today's appts. that are not cancelled, marked In, never been in a room,
	//and comply with our resource filter
	CString str;
	// (j.jones 2008-11-12 16:56) - PLID 28035 - added purpose text
	// (j.jones 2009-08-03 09:34) - PLID 26862 - added checked in username
	// (j.jones 2009-09-11 16:59) - PLID 35145 - AptShowStateHistoryT now has a UserID and not a Username, so the code
	// for the CheckedInBy column needed to be updated
	// (j.jones 2010-08-27 10:31) - PLID 36975 - the patient name is now calculated by a preference	
	// (d.lange 2010-08-30 17:49) - PLID 39431 - added provider column
	// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor
	// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource 
	// (c.haag 2010-10-26 10:07) - PLID 39199 - Changed how we determine whether the appt should be in the waiting area (added WaitingArea)
	// (s.dhole 2010-11-16 15:43) - PLID 39200- check appointment is check in and move to waiting room status
	// (j.jones 2010-12-01 17:35) - PLID 38597 - added Waiting Room info, if they are checked into a waiting room
	// (a.walling 2013-07-11 15:34) - PLID 57526 - Filter on StartTime range to avoid table scans
	str.Format("(SELECT "
		"AppointmentsT.ID AS AppointmentID, AppointmentsT.PatientID, %s AS PatientName, "
		"%s AS RecallStatusID, "
		"AppointmentsT.StartTime, TimeMarkedIn, CheckedInBy, "
		""
		//modified from the way the "time last seen" is calculated per room, to include
		//checkout time as a time last seen
		// (j.jones 2010-12-02 16:17) - PLID 38597 - ignore waiting rooms, they do not reset the wait timer
		"(SELECT Max(UpdateTime) AS FirstTimeWithoutPerson FROM RoomAppointmentHistoryT "
		"	WHERE RoomAppointmentHistoryT.RoomAppointmentID IN ("
		"		SELECT RoomAppointmentsT.ID FROM RoomAppointmentsT "
		"		INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
		"		WHERE RoomsT.WaitingRoom = 0 AND RoomAppointmentsT.AppointmentID = AppointmentsT.ID)"
		"	) "
		"AS TimeLastSeenByPerson, "
		""
		"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, "
		"AptTypeT.Name AS TypeName, CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS AptTypeColor, "
		"Provider.Last + ', ' + Provider.First + ' ' + Provider.Middle AS Provider, "
		" dbo.GetResourceString(AppointmentsT.ID) AS Resources,  "		
		"Coalesce(WaitingRoomQ.ID, -2) AS WaitingRoomID, "	//-2 means no assigned room
		"Coalesce(WaitingRoomQ.Name, '<No Assigned Room>') AS WaitingRoomName, "
		"WaitingRoomQ.RoomAppointmentID AS WaitingRoomAppointmentID "
		"FROM AppointmentsT "
		"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
		"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "		
		"LEFT JOIN (SELECT AppointmentID, Max(TimeStamp) AS TimeMarkedIn, Max(UsersT.UserName) AS CheckedInBy "
		"	FROM AptShowStateHistoryT "
		"	LEFT JOIN UsersT ON AptShowStateHistoryT.UserID = UsersT.PersonID "
		"	LEFT JOIN AptShowStateT on AptShowStateT.ID = AptShowStateHistoryT.ShowStateID "
		"	WHERE AptShowStateT.WaitingArea = CONVERT(BIT,1) GROUP BY AppointmentID, AptShowStateT.WaitingArea) "
		"	AS AptShowStateHistoryQ ON AppointmentsT.ID = AptShowStateHistoryQ.AppointmentID "
		//should not be possible for a patient to be in multiple waiting rooms, if so, they would be displayed here twice
		"LEFT JOIN (SELECT RoomsT.ID, RoomsT.Name, RoomAppointmentsT.AppointmentID, RoomAppointmentsT.ID AS RoomAppointmentID "
		"	FROM RoomAppointmentsT "
		"	INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
		"	WHERE RoomsT.WaitingRoom = 1 AND RoomAppointmentsT.StatusID NOT IN (0, -1) "
		"	) AS WaitingRoomQ ON AppointmentsT.ID = WaitingRoomQ.AppointmentID "
		"LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
		"LEFT JOIN PersonT AS Provider ON PatientsT.MainPhysician = Provider.ID "
		//not cancelled, marked 'In' appointments
		"WHERE AppointmentsT.Status <> 4 "
		//using our location filter 
		"AND AppointmentsT.LocationID = %li "
		//on today's date
		"AND AppointmentsT.StartTime >= '%s' AND AppointmentsT.StartTime < '%s' "
		
		//never been in a room		
		// (j.jones 2010-12-01 10:28) - PLID 38597 - ensure they are either NOT in a non-waiting room,
		// or specifically in a waiting room
		"AND ("
			//appts. not in a non-waiting room
		"	(AppointmentsT.ShowState IN (SELECT ID FROM AptShowStateT WHERE WaitingArea = CONVERT(BIT,1)) "
		"	AND "
		"	AppointmentsT.ID NOT IN (SELECT RoomAppointmentsT.AppointmentID FROM RoomAppointmentsT "
		"		INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
		"		WHERE RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
		"		AND RoomsT.WaitingRoom = 0) "
		"	) "
		"	OR "
			//appts. in a waiting room
		"	AppointmentsT.ID IN (SELECT RoomAppointmentsT.AppointmentID FROM RoomAppointmentsT "
		"		INNER JOIN RoomsT ON RoomAppointmentsT.RoomID = RoomsT.ID "
		"		WHERE RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
		"		AND RoomsT.WaitingRoom = 1 "
		"		AND RoomAppointmentsT.StatusID NOT IN (0, -1) "
		"	) "
		") "

		//optionally filtered by resource
		" %s "

		//optionally filtered by waiting room
		" %s "

		") AS WaitingAreaQ"
		, GetPatientNameSqlFormat().Flatten()
		, sqlRecall.Flatten()
		, nLocationID
		, literalDateFrom, literalDateTo
		, strResourceFilter
		, strWaitingRoomFilter
	);

	m_pWaitingAreaList->FromClause = _bstr_t(str);

	// (c.haag 2010-08-02 10:33) - PLID 35702 - Assign m_nTopWaitingAreaID to the Appointment ID of the first visible row
	m_nTopWaitingAreaID = -1;
	IRowSettingsPtr pTopRow = m_pWaitingAreaList->TopRow;
	if (NULL != pTopRow) {
		m_nTopWaitingAreaID = VarLong(pTopRow->GetValue(rmwalcApptID),-1); 
	}

	m_pWaitingAreaList->Requery();
}

// (j.jones 2008-05-29 10:45) - PLID 27797 - requeries the Checkout list
void CRoomManagerDlg::RequeryCheckoutList()
{
	//utilize the resource filter
	CString strResourceFilter, strResourceIDs;
	strResourceIDs = m_strResourceIDs;
	strResourceIDs.TrimRight();
	if(!strResourceIDs.IsEmpty()) {
		strResourceIDs.Replace(" ", ",");
		strResourceFilter.Format(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN (%s))", strResourceIDs);
	}

	// (j.jones 2009-08-04 08:55) - PLID 24600 - added location filter
	long nLocationID = -1;
	{
		IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
		if(pRow == NULL) {
			nLocationID = GetCurrentLocationID();
			m_pLocationCombo->SetSelByColumn(lccID, nLocationID);			
		}
		else {
			nLocationID = VarLong(pRow->GetValue(lccID));
		}
	}

	// (j.armen 2012-03-05 11:20) - PLID 48555 - Select the highest prority recall status for this patient
	// (a.walling 2013-12-12 16:51) - PLID 60005 - RoomManager only needs a smaller subset of the entire recall query - SelectRecallStatusesPastDueOrNeeded
	CSqlFragment sqlRecall("(SELECT MAX(RecallStatusID) AS RecallStatusID FROM ({SQL}) SubQ WHERE SubQ.PatientID = AppointmentsT.PatientID)", RecallUtils::SelectRecallStatusesPastDueOrNeeded());

	// (a.walling 2014-10-15 06:44) - PLID 63884 - AppointmentsT range queries on StartTime generate disastrously inaccurate cardinality estimates in the SQL optimizer
	CString literalDateFrom = FormatDateTimeForSql(AsDateNoTime(COleDateTime::GetCurrentTime()));
	CString literalDateTo = FormatDateTimeForSql(AsDateNoTime(COleDateTime::GetCurrentTime() + COleDateTimeSpan(1, 0, 0, 0)));

	//display all of today's non-cancelled appts. that comply with our resource filter,
	//that are marked "Ready To Check Out"
	CString str;
	// (j.jones 2008-11-12 16:56) - PLID 28035 - added purpose text	
	// (j.jones 2010-08-27 10:31) - PLID 36975 - the patient name is now calculated by a preference	
	// (d.lange 2010-08-30 17:54) - PLID 39431 - added provider column
	// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor
	// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource
	// (c.haag 2010-10-26 10:07) - PLID 39199 - Changed how we determine whether the appt should be in the waiting area (added WaitingArea)
	// (a.walling 2013-07-11 15:34) - PLID 57526 - Filter on StartTime range to avoid table scans
	str.Format("(SELECT RoomAppointmentsT.ID AS RoomAppointmentID, AppointmentsT.ID AS AppointmentID, "
		"AppointmentsT.PatientID AS PatientID, %s AS PatientName, %s AS RecallStatusID, "
		"AppointmentsT.StartTime, TimeMarkedIn, "
		""
		"	(SELECT Max(UpdateTime) AS TimeLeftRoom FROM RoomAppointmentHistoryT "
		"	WHERE RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID) "
		"	AS TimeLeftRoom, "
		""
		"RoomAppointmentsT.LastUpdateUserID, UsersT.Username, "
		"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, "
		"AptTypeT.Name AS TypeName, CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS AptTypeColor, "
		"Provider.Last + ', ' + Provider.First + ' ' + Provider.Middle AS Provider, "
		" dbo.GetResourceString(AppointmentsT.ID) AS Resources "
		"FROM AppointmentsT "
		"INNER JOIN RoomAppointmentsT ON AppointmentsT.ID = RoomAppointmentsT.AppointmentID "
		"INNER JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
		"LEFT JOIN (SELECT AppointmentID, Max(TimeStamp) AS TimeMarkedIn "
		"	FROM AptShowStateHistoryT "
		"	LEFT JOIN AptShowStateT on AptShowStateT.ID = AptShowStateHistoryT.ShowStateID "
		"	WHERE AptShowStateT.WaitingArea = CONVERT(BIT,1) GROUP BY AppointmentID, AptShowStateT.WaitingArea) "
		"	AS AptShowStateHistoryQ ON AppointmentsT.ID = AptShowStateHistoryQ.AppointmentID "
		"LEFT JOIN UsersT ON RoomAppointmentsT.LastUpdateUserID = UsersT.PersonID "
		"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
		"LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
		"LEFT JOIN PersonT AS Provider ON PatientsT.MainPhysician = Provider.ID "
		//not cancelled
		"WHERE AppointmentsT.Status <> 4 "
		//uses the filtered location
		"AND AppointmentsT.LocationID = %li "
		//on today's date
		"AND AppointmentsT.StartTime >= '%s' AND AppointmentsT.StartTime < '%s' "
		//currently marked 'Ready To Check Out'
		"AND RoomAppointmentsT.StatusID = 0 "
		//optionally filtered by resource
		"%s"
		") AS CheckoutListQ"
		, GetPatientNameSqlFormat().Flatten()
		, sqlRecall.Flatten()
		, nLocationID
		, literalDateFrom, literalDateTo
		, strResourceFilter
	);

	m_pCheckoutList->FromClause = _bstr_t(str);

	// (c.haag 2010-08-02 10:33) - PLID 35702 - Assign m_nTopCheckoutID to the room appointment ID of the first visible row
	m_nTopCheckoutID = -1;
	IRowSettingsPtr pTopRow = m_pCheckoutList->TopRow;
	if (NULL != pTopRow) {
		m_nTopCheckoutID = VarLong(pTopRow->GetValue(rmclcRoomAppointmentID),-1);
	}

	m_pCheckoutList->Requery();
}

// (j.jones 2007-02-07 10:57) - PLID 24595 - added Waiting Area list
void CRoomManagerDlg::OnLeftClickWaitingAreaList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL)
			return;

		m_pWaitingAreaList->PutCurSel(pRow);

		// (d.lange 2010-11-29 11:18) - PLID 41336 - If the Preview icon is clicked, bring up the EMR Preview window
		if(nCol == rmwalcPreviewEMN) {
			_variant_t var = pRow->GetValue(rmwalcPreviewEMN);
			if(var.vt == VT_I4 && g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
				long nIndex = 0;
				CreateEmrPreviewDialog(VarLong(pRow->GetValue(rmwalcPatientID), -1));

				// (z.manning 2012-09-10 15:51) - PLID 52543 - Use the new EmnPreviewPopup struct
				CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
				//Get array of patient EMNs
				// (z.manning 2011-04-06 12:31) - PLID 43160 - Moved this function to EmrUtils (and renamed it)
				GetEmnIDsByPatientID(VarLong(pRow->GetValue(rmwalcPatientID), -1), aryEMNs);
				m_pEMRPreviewPopupDlg->PreviewEMN(aryEMNs, nIndex);

				//Bring the dialog to the foreground if it is minimized
				WINDOWPLACEMENT wp;
				wp.length = sizeof(WINDOWPLACEMENT);
				if(m_pEMRPreviewPopupDlg->GetWindowPlacement(&wp)) {
					if(m_pEMRPreviewPopupDlg->IsIconic()) {
						if (wp.flags & WPF_RESTORETOMAXIMIZED) {
							wp.showCmd = SW_MAXIMIZE;
						} else {
							wp.showCmd = SW_RESTORE;
						}
						m_pEMRPreviewPopupDlg->SetWindowPlacement(&wp);
						m_pEMRPreviewPopupDlg->SetForegroundWindow();
					}
				}

				if(!m_pEMRPreviewPopupDlg->IsWindowVisible()) {
					m_pEMRPreviewPopupDlg->ShowWindow(SW_SHOW);
				}
			}
		}else if(nCol == rmwalcRecallStatus){
			// (j.armen 2012-03-05 11:25) - PLID 48555 - Pass along the row clicked, and the enum for the column the RecallStatusID is located in
			OnRecallFlagClicked(pRow, rmwalcRecallStatusID, rmwalcPatientID);
		} else {
			
			CPoint pt;
			GetCursorPos(&pt);
			// (j.jones 2010-12-02 16:42) - PLID 38597 - denote that this is called from the waiting area
			GenerateAppointmentMenu(VarLong(pRow->GetValue(rmwalcPatientID),-1), VarLong(pRow->GetValue(rmwalcApptID),-1), pt, FALSE, VarLong(pRow->GetValue(rmwalcWaitingRoomAppointmentID),-1), TRUE, VarLong(pRow->GetValue(rmwalcWaitingRoomID),-1));
		}

	}NxCatchAll("Error in CRoomManagerDlg::OnLeftClickWaitingAreaList");	
}

void CRoomManagerDlg::OnRButtonDownWaitingAreaList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL)
			return;

		m_pWaitingAreaList->PutCurSel(pRow);

		CPoint pt;
		GetCursorPos(&pt);
		// (j.jones 2010-12-02 16:42) - PLID 38597 - denote that this is called from the waiting area
		GenerateAppointmentMenu(VarLong(pRow->GetValue(rmwalcPatientID),-1), VarLong(pRow->GetValue(rmwalcApptID),-1), pt, FALSE, VarLong(pRow->GetValue(rmwalcWaitingRoomAppointmentID),-1), TRUE, VarLong(pRow->GetValue(rmwalcWaitingRoomID),-1));

	}NxCatchAll("Error in CRoomManagerDlg::OnRButtonDownWaitingAreaList");
}

void CRoomManagerDlg::OnRequeryFinishedWaitingAreaList(short nFlags) 
{
	try {

		//for each appt, start/stop timers
		UpdateAllWaitingAreaWaitTimes(FALSE);

		// (c.haag 2010-08-02 10:48) - PLID 35702 - Restore the current topmost row to keep the
		// vertical scrollbar as consistent as possible with its pre-requery position
		if (-1 != m_nTopWaitingAreaID) {
			IRowSettingsPtr pCurrentTopRow = m_pWaitingAreaList->TopRow;
			IRowSettingsPtr pDesiredTopRow = m_pWaitingAreaList->FindByColumn(rmwalcApptID, m_nTopWaitingAreaID, NULL, VARIANT_FALSE);
			// Only set the top row if:					
			if (NULL != pDesiredTopRow  // A. We found the top row in the list from before
				&& pCurrentTopRow == m_pWaitingAreaList->GetFirstRow() // B. The current top row is the first row in the list. If not, it means the user scrolled it during the requery.				
				) 
			{
				m_pWaitingAreaList->TopRow = pDesiredTopRow;
			}
			// Discard the ID; we don't need it anymore and we shouldn't leave it hanging around.
			m_nTopWaitingAreaID = -1;
		}

		// (d.lange 2010-12-08 17:11) - PLID 40295 - add EMR Preview icon for each row after requery
		for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pWaitingAreaList->FindAbsoluteFirstRow(VARIANT_TRUE);
			pRow != NULL; pRow = pRow->GetNextRow()) {
				pRow->PutValue(rmwalcPreviewEMN, (long)m_hIconPreview);
		}

		// (j.armen 2012-03-05 11:19) - PLID 48555 - When requery finishes, add the recall flag
		EnsureRecallFlagColumn(m_pWaitingAreaList, rmwalcRecallStatusID, rmwalcRecallStatus, "ShowRoomManagerWaitingAreaColumnRecall");

	}NxCatchAll("Error in CRoomManagerDlg::OnRequeryFinishedWaitingAreaList");
}

// (j.jones 2007-02-20 13:40) - PLID 24365 - used to flash the window when minimized or inactive
void CRoomManagerDlg::Flash()
{
	try {

		HWND hwndActive = ::GetActiveWindow();
		HWND hwndDlg = this->GetSafeHwnd();
		if (hwndActive == hwndDlg) {
			return; // do not flash if we are the active window
		}

		// (j.jones 2007-02-20 16:06) - FlashWindowEx doesn't work as intended for me,
		// it only flashes one time, a likely theory is that FLASHW_TIMERNOFG stops
		//immediately when it realizes this is a child of the active program
		/*
		FLASHWINFO fwi;
		fwi.cbSize = sizeof(FLASHWINFO);
		fwi.hwnd = this->GetSafeHwnd();
		fwi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
		fwi.dwTimeout = 0;
		fwi.uCount = 10;
		FlashWindowEx(&fwi);
		*/

		//use FlashWindow instead, it flashes once and leaves the task button inverted,
		//which is 1. not annoying, and 2. what I have witnessed other programs do
		FlashWindow(TRUE);

	}NxCatchAll("Error in CRoomManagerDlg::Flash");
}

// (j.jones 2008-05-29 11:29) - PLID 27797 - added checkout list functions
void CRoomManagerDlg::OnRButtonDownCheckoutList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL) {
			return;
		}

		m_pCheckoutList->PutCurSel(pRow);

		CPoint pt;
		GetCursorPos(&pt);
		GenerateAppointmentMenu(VarLong(pRow->GetValue(rmclcPatientID),-1), VarLong(pRow->GetValue(rmclcApptID),-1), pt, TRUE, VarLong(pRow->GetValue(rmclcRoomAppointmentID),-1));

	}NxCatchAll("Error in CRoomManagerDlg::OnRButtonDownCheckoutList");
}

void CRoomManagerDlg::OnRequeryFinishedCheckoutList(short nFlags) 
{
	try {

		//for each appt, start/stop timers
		UpdateAllCheckoutListWaitTimes(FALSE);

		// (c.haag 2010-08-02 10:48) - PLID 35702 - Restore the current topmost row to keep the
		// vertical scrollbar as consistent as possible with its pre-requery position
		if (-1 != m_nTopCheckoutID) {
			IRowSettingsPtr pCurrentTopRow = m_pCheckoutList->TopRow;
			IRowSettingsPtr pDesiredTopRow = m_pCheckoutList->FindByColumn(rmclcRoomAppointmentID, m_nTopCheckoutID, NULL, VARIANT_FALSE);
			// Only set the top row if:					
			if (NULL != pDesiredTopRow  // A. We found the top row in the list from before
				&& pCurrentTopRow == m_pCheckoutList->GetFirstRow() // B. The current top row is the first row in the list. If not, it means the user scrolled it during the requery.				
				) 
			{
				m_pCheckoutList->TopRow = pDesiredTopRow;
			}
			// Discard the ID; we don't need it anymore and we shouldn't leave it hanging around.
			m_nTopCheckoutID = -1;
		}

		// (d.lange 2010-12-08 17:11) - PLID 40295 - add EMR Preview icon for each row after requery
		for(NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCheckoutList->FindAbsoluteFirstRow(VARIANT_TRUE);
			pRow != NULL; pRow = pRow->GetNextRow()) {
				pRow->PutValue(rmclcPreviewEMN, (long)m_hIconPreview);
		}

		// (j.armen 2012-03-05 11:19) - PLID 48555 - When requery finishes, add the recall flag
		EnsureRecallFlagColumn(m_pCheckoutList, rmclcRecallStatusID, rmclcRecallStatus, "ShowRoomManagerCheckoutColumnRecall");

	}NxCatchAll("Error in CRoomManagerDlg::OnRequeryFinishedCheckoutList");
}

void CRoomManagerDlg::OnLeftClickCheckoutList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL) {
			return;
		}

		m_pCheckoutList->PutCurSel(pRow);

		// (d.lange 2010-11-29 14:54) - PLID 40295 - When the Preview icon is selected, open the EMR Preview window
		if(nCol == rmclcPreviewEMN) {
			_variant_t var = pRow->GetValue(rmclcPreviewEMN);
			if(var.vt == VT_I4 && g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
				long nIndex = 0;
				CreateEmrPreviewDialog(VarLong(pRow->GetValue(rmclcPatientID), -1));

				// (z.manning 2012-09-10 15:51) - PLID 52543 - Use the new EmnPreviewPopup struct
				CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
				//Get array of patient EMNs
				// (z.manning 2011-04-06 12:31) - PLID 43160 - Moved this function to EmrUtils (and renamed it)
				GetEmnIDsByPatientID(VarLong(pRow->GetValue(rmclcPatientID), -1), aryEMNs);
				m_pEMRPreviewPopupDlg->PreviewEMN(aryEMNs, nIndex);

				//Bring the dialog to the foreground if it is minimized
				WINDOWPLACEMENT wp;
				wp.length = sizeof(WINDOWPLACEMENT);
				if(m_pEMRPreviewPopupDlg->GetWindowPlacement(&wp)) {
					if(m_pEMRPreviewPopupDlg->IsIconic()) {
						if (wp.flags & WPF_RESTORETOMAXIMIZED) {
							wp.showCmd = SW_MAXIMIZE;
						} else {
							wp.showCmd = SW_RESTORE;
						}
						m_pEMRPreviewPopupDlg->SetWindowPlacement(&wp);
						m_pEMRPreviewPopupDlg->SetForegroundWindow();
					}
				}

				if(!m_pEMRPreviewPopupDlg->IsWindowVisible()) {
					m_pEMRPreviewPopupDlg->ShowWindow(SW_SHOW);
				}
			}
		}else if (nCol == rmclcRecallStatus)
		{
			// (j.armen 2012-03-05 11:25) - PLID 48555 - Pass along the row clicked, and the enum for the column the RecallStatusID is located in
			OnRecallFlagClicked(pRow, rmclcRecallStatusID, rmclcPatientID);
		}else {
			CPoint pt;
			GetCursorPos(&pt);
			GenerateAppointmentMenu(VarLong(pRow->GetValue(rmclcPatientID),-1), VarLong(pRow->GetValue(rmclcApptID),-1), pt, TRUE, VarLong(pRow->GetValue(rmclcRoomAppointmentID),-1));
		}
	}NxCatchAll("Error in CRoomManagerDlg::OnLeftClickCheckoutList");
}

// (j.jones 2008-06-02 09:59) - PLID 30219 - ensures that any place that marks an appt. 'Ready To Check Out'
// will call the appropriate PracYakker code
void CRoomManagerDlg::OnReadyToCheckOut(long nPatientID, CString strPatientFirstName, CString strPatientLastName, CString strAppointmentDescription)
{
	try {

		if(nPatientID != 25 && GetMainFrame()) {
			//TES 1/12/2009 - PLID 32525 - Don't bother checking the preference if they can't access the Yak anyway.
			// (v.maida 2014-12-27 10:19) - PLID 27381 - permission for using PracYakker.
			if (!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent, "PracYakker intra-office messaging", "PracYakker/pracyakker.htm") || !CheckCurrentUserPermissions(bioPracYakker, sptView, 0, 0, TRUE)) {
				return;
			}
			//check the preference
			long nSendPracYak = GetRemotePropertyInt("RoomMgrReadyCheckout_SendPracYak", 2, 0, GetCurrentUserName(), true);
			//1 - always, 2 - prompt, 0 - never

			if(nSendPracYak == 1 || (nSendPracYak == 2 &&
				IDYES == MessageBox("Would you like to send a PracYakker message regarding this appointment now?",
				"Practice", MB_ICONQUESTION|MB_YESNO))) {

				//check the group preference
				long nPracYakGroupID = GetRemotePropertyInt("RoomMgrReadyCheckout_PracYakGroupID", -1, 0, GetCurrentUserName(), true);

				CSendMessageDlg *pdlgYakMessage = GetMainFrame()->m_pdlgSendYakMessage;
				if (pdlgYakMessage == NULL) {
					//this should never be NULL, the code is currently initializing
					//this window upon startup
					ThrowNxException("PracYakker is not initialized!");
				}
				else {
					CString strDefaultText;
					//the message will show the full patient's name in the 'regarding' field,
					//but show the more casual "Jane Doe" text in the default message
					strDefaultText.Format("%s %s is ready to be checked out. (Appointment: %s)\r\n", strPatientFirstName, strPatientLastName, strAppointmentDescription);
					// (j.gruber 2010-07-16 14:07) - PLID 39463 - added ThreadID
					pdlgYakMessage->PopUpMessage(-1, nPatientID, NULL, NULL, strDefaultText, nPracYakGroupID);
				}
			}
		}

	}NxCatchAll("Error in CRoomManagerDlg::OnReadyToCheckOut");
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CRoomManagerDlg::OnOK()
{
	//Eat the message
}


// (j.gruber 2009-07-10 10:15) - PLID 28792 - remember the column widths
void CRoomManagerDlg::OnRememberColWidths() 
{
	try {
		//save the setting
		long nRemember = 0;	//default off
		if(IsDlgButtonChecked(IDC_ROOM_MANAGER_REMEMBER_COL_WIDTHS))
			nRemember = 1;
		else
			nRemember = 0;
		SetRemotePropertyInt("RememberRoomManagerColumns", nRemember, 0, GetCurrentUserName());

		//size the datalist appropriately
		if(!IsDlgButtonChecked(IDC_ROOM_MANAGER_REMEMBER_COL_WIDTHS)) {
			ResetColumnSizes();
		}
		else {	

			//Make sure all the styles are fixed-width type styles.
			m_pRoomList->SetRedraw(FALSE);		
			for (short i=0; i < m_pRoomList->ColumnCount; i++)
			{
				long nStyle = m_pRoomList->GetColumn(i)->ColumnStyle;
				nStyle &= ~(csWidthPercent | csWidthAuto);
				//nStyle = csVisible;
				m_pRoomList->GetColumn(i)->ColumnStyle = nStyle;
			}

			m_pWaitingAreaList->SetRedraw(FALSE);
			for (short i=0; i < m_pWaitingAreaList->ColumnCount; i++)
			{
				long nStyle = m_pWaitingAreaList->GetColumn(i)->ColumnStyle;
				nStyle &= ~(csWidthPercent | csWidthAuto);
				//nStyle = csVisible;
				m_pWaitingAreaList->GetColumn(i)->ColumnStyle = nStyle;
			}

			m_pCheckoutList->SetRedraw(FALSE);
			for (short i=0; i < m_pCheckoutList->ColumnCount; i++)
			{
				long nStyle = m_pCheckoutList->GetColumn(i)->ColumnStyle;
				nStyle &= ~(csWidthPercent | csWidthAuto);				
				//nStyle = csVisible;
				m_pCheckoutList->GetColumn(i)->ColumnStyle = nStyle;
			}

			SetColumnSizes();
			m_pRoomList->SetRedraw(TRUE);
			m_pWaitingAreaList->SetRedraw(TRUE);
			m_pCheckoutList->SetRedraw(TRUE);
		}
	}NxCatchAll("Error in CRoomManagerDlg::OnRememberColWidths");
}


void CRoomManagerDlg::ResetColumnSizes()
{
	// (j.gruber 2009-07-10 13:31) - PLID 28792 - Plagiarized from Tracking tab.

	if(m_strRoomColumnWidths.IsEmpty() || m_strCheckoutColumnWidths.IsEmpty() || m_strWaitingColumnWidths.IsEmpty()) {
		//not sure why we wouldn't have any, but better to leave them as is
		//than to set to empty
		return;
	}

	CString strCols = m_strRoomColumnWidths;
	int nWidth = 0, i = 0;

	//parse the columns out and set them
	int nComma = strCols.Find(",");
	while(nComma > 0) {
		nWidth = atoi(strCols.Left(nComma));
		strCols = strCols.Right(strCols.GetLength() - (nComma+1));

		IColumnSettingsPtr pCol = m_pRoomList->GetColumn(i);
		if(pCol) {
			pCol->PutStoredWidth(nWidth);		
		}

		i++;
		nComma = strCols.Find(",");
	}

	//set the styles back also
	strCols = m_strRoomColumnStyles;
	long nStyle = 0;
	i = 0;
	nComma = strCols.Find(",");
	while(nComma > 0) {
		nStyle = atoi(strCols.Left(nComma));
		strCols = strCols.Right(strCols.GetLength() - (nComma+1));

		IColumnSettingsPtr pCol = m_pRoomList->GetColumn(i);
		if(pCol) {
			pCol->PutColumnStyle(nStyle);		
		}

		i++;
		nComma = strCols.Find(",");
	}

	//now the waiting list
	strCols = m_strWaitingColumnWidths;
	nWidth = 0;
	i = 0;

	//parse the columns out and set them
	nComma = strCols.Find(",");
	while(nComma > 0) {
		nWidth = atoi(strCols.Left(nComma));
		strCols = strCols.Right(strCols.GetLength() - (nComma+1));

		IColumnSettingsPtr pCol = m_pWaitingAreaList->GetColumn(i);
		if(pCol) {
			pCol->PutStoredWidth(nWidth);
		}

		i++;
		nComma = strCols.Find(",");
	}

	//set the styles back also
	strCols = m_strWaitingColumnStyles;
	nStyle = 0;
	i = 0;
	nComma = strCols.Find(",");
	while(nComma > 0) {
		nStyle = atoi(strCols.Left(nComma));
		strCols = strCols.Right(strCols.GetLength() - (nComma+1));

		IColumnSettingsPtr pCol = m_pWaitingAreaList->GetColumn(i);
		if(pCol) {
			pCol->PutColumnStyle(nStyle);		
		}

		i++;
		nComma = strCols.Find(",");
	}


	//and finally the check out list

	strCols = m_strCheckoutColumnWidths;
	nWidth = 0;
	i = 0;

	//parse the columns out and set them
	nComma = strCols.Find(",");
	while(nComma > 0) {
		nWidth = atoi(strCols.Left(nComma));
		strCols = strCols.Right(strCols.GetLength() - (nComma+1));

		IColumnSettingsPtr pCol = m_pCheckoutList->GetColumn(i);
		if(pCol) {
			pCol->PutStoredWidth(nWidth);
		}

		i++;
		nComma = strCols.Find(",");
	}

	//set the styles back also
	strCols = m_strCheckoutColumnStyles;
	nStyle = 0;
	i = 0;
	nComma = strCols.Find(",");
	while(nComma > 0) {
		nStyle = atoi(strCols.Left(nComma));
		strCols = strCols.Right(strCols.GetLength() - (nComma+1));

		IColumnSettingsPtr pCol = m_pCheckoutList->GetColumn(i);
		if(pCol) {
			pCol->PutColumnStyle(nStyle);		
		}

		i++;
		nComma = strCols.Find(",");
	}
}

void CRoomManagerDlg::SetColumnSizes()
{
	// (j.gruber 2009-07-10 13:31) - PLID 28792 - Plagiarized from Tracking
	

	//don't want to remember
	if(!IsDlgButtonChecked(IDC_ROOM_MANAGER_REMEMBER_COL_WIDTHS)) {
		return;
	}

	//first the waiting room list
	CString strCols = GetRemotePropertyText("DefaultRoomManagerWaitingColumnSizes", "", 0, GetCurrentUserName(), true);
	if(strCols.IsEmpty()) strCols = m_strWaitingColumnWidths;

	if(!strCols.IsEmpty()) {
		CArray<long, long> ary;
		StringAsArray(strCols, ary, true);
		//(e.lally 2011-10-10) PLID 44507 - Reset if the column count is different from the saved widths count
		if(ary.GetCount() != m_pWaitingAreaList->ColumnCount){
			//Reset back to the default even if the new column was added to the end. We have no way of knowing where it was inserted.
			strCols = m_strWaitingColumnWidths;
			SetRemotePropertyText("DefaultRoomManagerWaitingColumnSizes", m_strWaitingColumnWidths, 0, GetCurrentUserName());
		}

		IColumnSettingsPtr pCol;
		int nWidth = 0, i = 0;

		//parse the columns out and set them
		int nComma = strCols.Find(",");
		while(nComma > 0) {
			nWidth = atoi(strCols.Left(nComma));
			strCols = strCols.Right(strCols.GetLength() - (nComma+1));

			pCol = m_pWaitingAreaList->GetColumn(i);
			if(pCol != NULL)
				pCol->PutStoredWidth(nWidth);

			i++;
			nComma = strCols.Find(",");
		}
	}

	//now the rooms
	strCols = GetRemotePropertyText("DefaultRoomManagerRoomColumnSizes", "", 0, GetCurrentUserName(), true);
	if(strCols.IsEmpty()) strCols = m_strRoomColumnWidths;

	if(!strCols.IsEmpty()) {
		CArray<long, long> ary;
		StringAsArray(strCols, ary, true);
		//(e.lally 2011-10-10) PLID 44507 - Reset if the column count is different from the saved widths count
		if(ary.GetCount() != m_pRoomList->ColumnCount){
			//Reset back to the default even if the new column was added to the end. We have no way of knowing where it was inserted.
			strCols = m_strRoomColumnWidths;
			SetRemotePropertyText("DefaultRoomManagerRoomColumnSizes", m_strRoomColumnWidths, 0, GetCurrentUserName());
		}

		IColumnSettingsPtr pCol;
		int nWidth = 0, i = 0;

		//parse the columns out and set them
		int nComma = strCols.Find(",");
		while(nComma > 0) {
			nWidth = atoi(strCols.Left(nComma));
			strCols = strCols.Right(strCols.GetLength() - (nComma+1));

			pCol = m_pRoomList->GetColumn(i);

			if(pCol != NULL)
				pCol->PutStoredWidth(nWidth);

			i++;
			nComma = strCols.Find(",");
		}
	}

	//and finally the check out area
	strCols = GetRemotePropertyText("DefaultRoomManagerCheckOutColumnSizes", "", 0, GetCurrentUserName(), true);
	if(strCols.IsEmpty()) strCols = m_strCheckoutColumnWidths;

	if(!strCols.IsEmpty()) {
		CArray<long, long> ary;
		StringAsArray(strCols, ary, true);
		//(e.lally 2011-10-10) PLID 44507 - Reset if the column count is different from the saved widths count
		if(ary.GetCount() != m_pCheckoutList->ColumnCount){
			//Reset back to the default even if the new column was added to the end. We have no way of knowing where it was inserted.
			strCols = m_strCheckoutColumnWidths;
			SetRemotePropertyText("DefaultRoomManagerCheckOutColumnSizes", m_strCheckoutColumnWidths, 0, GetCurrentUserName());
		}

		IColumnSettingsPtr pCol;
		int nWidth = 0, i = 0;

		//parse the columns out and set them
		int nComma = strCols.Find(",");
		while(nComma > 0) {
			nWidth = atoi(strCols.Left(nComma));
			strCols = strCols.Right(strCols.GetLength() - (nComma+1));

			pCol = m_pCheckoutList->GetColumn(i);
			if(pCol != NULL)
				pCol->PutStoredWidth(nWidth);

			i++;
			nComma = strCols.Find(",");
		}
	}

}

void CRoomManagerDlg::ColumnSizingFinishedWaitingAreaList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {
		// (j.gruber 2009-07-10 10:56) - PLID 28792  - Plagiarized from Tracking

		//uncommitted
		if(!bCommitted)
			return;

		//don't want to remember
		if(!IsDlgButtonChecked(IDC_ROOM_MANAGER_REMEMBER_COL_WIDTHS))
			return;	
		
		//save width of each column
		IColumnSettingsPtr pCol;
		CString str, strList;

		for(int i = 0; i < m_pWaitingAreaList->GetColumnCount(); i++) {
			pCol = m_pWaitingAreaList->GetColumn(i);
			if(pCol)
				str.Format("%li,", pCol->GetStoredWidth());

			strList += str;
		}

		//write it to ConfigRT
		SetRemotePropertyText("DefaultRoomManagerWaitingColumnSizes", strList, 0, GetCurrentUserName());

		SetColumnSizes();
	}NxCatchAll("Error in CRoomManagerDlg::ColumnSizingFinishedWaitingAreaList");
}

void CRoomManagerDlg::ColumnSizingFinishedRoomList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {
	
		// (j.gruber 2009-07-10 10:56) - PLID 28792  - Plagiarized from Tracking

		//uncommitted
		if(!bCommitted)
			return;

		//don't want to remember
		if(!IsDlgButtonChecked(IDC_ROOM_MANAGER_REMEMBER_COL_WIDTHS))
			return;	
		
		//save width of each column
		IColumnSettingsPtr pCol;
		CString str, strList;

		for(int i = 0; i < m_pRoomList->GetColumnCount(); i++) {
			pCol = m_pRoomList->GetColumn(i);
			if(pCol)
				str.Format("%li,", pCol->GetStoredWidth());

			strList += str;
		}

		//write it to ConfigRT
		SetRemotePropertyText("DefaultRoomManagerRoomColumnSizes", strList, 0, GetCurrentUserName());

		SetColumnSizes();
	}NxCatchAll("Error in CRoomManagerDlg::ColumnSizingFinishedRoomList");
}

void CRoomManagerDlg::ColumnSizingFinishedCheckoutList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {
		// (j.gruber 2009-07-10 10:56) - PLID 28792  - Plagiarized from Tracking

		//uncommitted
		if(!bCommitted)
			return;

		//don't want to remember
		if(!IsDlgButtonChecked(IDC_ROOM_MANAGER_REMEMBER_COL_WIDTHS))
			return;	
		
		//save width of each column
		IColumnSettingsPtr pCol;
		CString str, strList;

		for(int i = 0; i < m_pCheckoutList->GetColumnCount(); i++) {
			pCol = m_pCheckoutList->GetColumn(i);
			if(pCol)
				str.Format("%li,", pCol->GetStoredWidth());

			strList += str;
		}

		//write it to ConfigRT
		SetRemotePropertyText("DefaultRoomManagerCheckOutColumnSizes", strList, 0, GetCurrentUserName());

		SetColumnSizes();
	}NxCatchAll("Error in CRoomManagerDlg::ColumnSizingFinishedCheckoutList");
}

void CRoomManagerDlg::SetDefaultColumnWidths()
{
	try {
		if(m_strRoomColumnWidths.IsEmpty()) {
			//First, remember the original settings
			CString strOrig, str, strStyles, strOrigStyles;
			for(int i = 0; i < m_pRoomList->GetColumnCount(); i++) {
				IColumnSettingsPtr pCol = m_pRoomList->GetColumn(i);
				str.Format("%li,", pCol->GetStoredWidth());				
				strStyles.Format("%li,", pCol->GetColumnStyle());				

				strOrig += str;
				strOrigStyles += strStyles;
			}

			m_strRoomColumnWidths = strOrig;		
			m_strRoomColumnStyles = strOrigStyles;
		}

		if(m_strWaitingColumnWidths.IsEmpty()) {
			//First, remember the original settings
			CString strOrig, str, strOrigStyles, strStyles;
			for(int i = 0; i < m_pWaitingAreaList->GetColumnCount(); i++) {
				IColumnSettingsPtr pCol = m_pWaitingAreaList->GetColumn(i);
				str.Format("%li,", pCol->GetStoredWidth());
				strStyles.Format("%li,", pCol->GetColumnStyle());

				strOrig += str;
				strOrigStyles += strStyles;
			}

			m_strWaitingColumnWidths = strOrig;		
			m_strWaitingColumnStyles = strOrigStyles;		

		}

		if(m_strCheckoutColumnWidths.IsEmpty()) {
			//First, remember the original settings
			CString strOrig, str, strOrigStyles, strStyles;
			for(int i = 0; i < m_pCheckoutList->GetColumnCount(); i++) {
				IColumnSettingsPtr pCol = m_pCheckoutList->GetColumn(i);
				str.Format("%li,", pCol->GetStoredWidth());
				strStyles.Format("%li,", pCol->GetColumnStyle());

				strOrig += str;
				strOrigStyles += strStyles;
			}

			m_strCheckoutColumnWidths = strOrig;		
			m_strCheckoutColumnStyles = strOrigStyles;		
		}
	}NxCatchAll("Error in CRoomManagerDlg::SetDefaultColumnWidths");
}

// (j.jones 2009-08-04 09:12) - PLID 24600 - added location filter
void CRoomManagerDlg::OnSelChosenRoomMgrLocationCombo(LPDISPATCH lpRow)
{
	try {

		RequeryAll(TRUE);

	}NxCatchAll("Error in CRoomManagerDlg::OnSelChosenRoomMgrLocationCombo");	
}

// (z.manning 2009-08-18 11:27) - PLID 28014
void CRoomManagerDlg::MinimizeWindow()
{
	try
	{
		WINDOWPLACEMENT wp;
		wp.length = sizeof(WINDOWPLACEMENT);
		if(this->GetWindowPlacement(&wp))
		{
			//Check if we are not minimized
			if(!this->IsIconic()) {
				wp.showCmd = SW_MINIMIZE;
				this->SetWindowPlacement(&wp);
				//ensure that the main app. has focus now
				if(GetMainFrame()) {
					GetMainFrame()->SetForegroundWindow();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-09-21 15:41) - PLID 25232 - added print option
void CRoomManagerDlg::OnBtnPreviewRoomMgr()
{
	try {

		CWaitCursor pWait;

		CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(679)]);

		//pass in the resource filter as strExtraText
		CString strResourceFilter, strResourceIDs, strResourceFilterName;
		strResourceIDs = m_strResourceIDs;
		strResourceIDs.TrimRight();
		if(!strResourceIDs.IsEmpty()) {
			strResourceIDs.Replace(" ", ",");
			strResourceFilter.Format(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID IN (%s))", strResourceIDs);
			if(strResourceIDs.Find(",") == -1 && m_pResourceCombo->GetCurSel()) {
				strResourceFilterName = VarString(m_pResourceCombo->GetCurSel()->GetValue(rfccName));
			}
			else {
				strResourceFilterName = m_strResourceList;
			}
		}
		else {
			strResourceFilterName = "<All Resources>";
		}
		infReport.strExtraText = strResourceFilter;

		//filter on location ID
		long nLocationID = -1;
		CString strLocationName = "";
		{
			IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
			if(pRow == NULL) {
				nLocationID = GetCurrentLocationID();
				strLocationName = GetCurrentLocationName();
				m_pLocationCombo->SetSelByColumn(lccID, nLocationID);
			}
			else {
				nLocationID = VarLong(pRow->GetValue(lccID));
				strLocationName = VarString(pRow->GetValue(lccName));
			}
		}
		infReport.nLocation = nLocationID;

		//Set up the parameters.
		CPtrArray paParams;
		CRParameterInfo *paramInfo;
		
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = GetCurrentUserName();
		paramInfo->m_Name = "CurrentUserName";
		paParams.Add(paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strLocationName;
		paramInfo->m_Name = "LocationName";
		paParams.Add((void *)paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strResourceFilterName;
		paramInfo->m_Name = "ResourceFilter";
		paParams.Add((void *)paramInfo);

		RunReport(&infReport, &paParams, TRUE, (CWnd *)this, "Room Manager");
		ClearRPIParameterList(&paParams);

		//minimize the window so it doesn't overlap the report
		MinimizeWindow();

	}NxCatchAll("Error in CRoomManagerDlg::OnBtnPreviewRoomMgr");
}

// (s.dhole 2010-02-08 13:45) - PLID 37112 Workflow change from room manager -> EMR for doctors.
void CRoomManagerDlg::OnLeftClickRoomList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try 
	{
		IRowSettingsPtr pRow = lpRow;
		if(pRow == NULL)
			return;

		//Load the person regardless of column
		if (nCol==rmrlcPatientName)
		{	
			long nAppointmentID = VarLong(pRow->GetValue(rmrlcApptID),-1);
			if (nAppointmentID >-1)
			{
				long nPatientID= VarLong(pRow->GetValue(rmrlcPatientID),-1);

				if (m_nRoomMgrPatientSelection==1)
				{
					long nRoomAppointmentID = VarLong(pRow->GetValue(rmrlcRoomAppointmentID),-1);	
					// Since we are not considring Checked Out = -1 and  Ready To Check Out= 0 for preferences, 0  will be default value 
					//and we check same before fliping room status. 
					long nStatus = GetRemotePropertyInt("RoomMgrPatientSelection_RoomStatusID", 0, 0, GetCurrentUserName(), true);
					// Ignoring Checked Out = -1 and  Ready To Check Out= 0
					if (nStatus>0)
						ChangeRoomAppointmentStatus(nRoomAppointmentID, nAppointmentID, nStatus);
				}

				switch (m_nRoomMgrPatientSelection_action)
				{
				case lpOpenGeneral1:
					if (GoToPatient(nPatientID,PatientsModule::General1Tab))
						MinimizeWindow();
					break;
				case lpOpenEMR:
					// (z.manning 2015-05-05 16:19) - NX-100433 - There is now a preference to determine what the NexEMR tab is
					if (GoToPatient(nPatientID, GetMainFrame()->GetPrimaryEmrTab())) {
						MinimizeWindow();
					}
					break;
				case lpOpenExistEMN: //Emn Today

					if (!m_bEMRExpired)
					{
						CString strPatientName= VarString(pRow->GetValue(rmrlcPatientName));
						CheckEMNExist(nPatientID,strPatientName);
					}
					break;
				case lpCreateEMN:
					// (z.manning 2015-05-06 11:08) - NX-100433 - Now have a preference to determine which tab to use
					if (!m_bEMRExpired && GoToPatient(nPatientID, GetMainFrame()->GetPrimaryEmrTab()))
					{
						CheckEMRTemplateWithCollection(nAppointmentID, nPatientID);
					}
					break;
				}
			}
			// (d.lange 2010-11-29 14:54) - PLID 40295 - EMR Preview window
		}else if(nCol == rmrlcPreviewEMN) {
			_variant_t var = pRow->GetValue(rmrlcPreviewEMN);
			if(var.vt == VT_I4 && g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
				long nIndex = 0;
				//Create and hide the EMR Preview dialog if not already initalized
				CreateEmrPreviewDialog(VarLong(pRow->GetValue(rmrlcPatientID), -1));

				// (z.manning 2012-09-10 15:50) - PLID 52543 - Use the new EmnPreviewPopup struct
				CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
				//Get array of patient EMNs
				// (z.manning 2011-04-06 12:31) - PLID 43160 - Moved this function to EmrUtils (and renamed it)
				GetEmnIDsByPatientID(VarLong(pRow->GetValue(rmrlcPatientID), -1), aryEMNs);
				m_pEMRPreviewPopupDlg->PreviewEMN(aryEMNs, nIndex);

				//Bring the dialog to the foreground if it is minimized
				WINDOWPLACEMENT wp;
				wp.length = sizeof(WINDOWPLACEMENT);
				if(m_pEMRPreviewPopupDlg->GetWindowPlacement(&wp)) {
					if(m_pEMRPreviewPopupDlg->IsIconic()) {
						if (wp.flags & WPF_RESTORETOMAXIMIZED) {
							wp.showCmd = SW_MAXIMIZE;
						} else {
							wp.showCmd = SW_RESTORE;
						}
						m_pEMRPreviewPopupDlg->SetWindowPlacement(&wp);
						m_pEMRPreviewPopupDlg->SetForegroundWindow();
					}
				}

				if(!m_pEMRPreviewPopupDlg->IsWindowVisible()) {
					m_pEMRPreviewPopupDlg->ShowWindow(SW_SHOW);
				}
			}
		} else if (nCol == rmrlcRecallStatus) {
			// (j.armen 2012-03-05 11:25) - PLID 48555 - Pass along the row clicked, and the enum for the column the RecallStatusID is located in
			OnRecallFlagClicked(pRow, rmrlcRecallStatusID, rmrlcPatientID);
		}
	}NxCatchAll("Error in CRoomManagerDlg::LeftClickRoomList");
}
// (s.dhole 2010-02-11 10:35) - PLID 37112 Workflow change from room manager -> EMR for doctors. 
BOOL CRoomManagerDlg::CheckEMNWarning(long nTemplateID)
{
	// (a.walling 2013-07-09 14:20) - PLID 57487 - Return true if -1 of course
	if (nTemplateID == -1) {
		return TRUE;
	}

	if(GetRemotePropertyInt("WarnWhenCreatingNexWebEmn", 1, 0, GetCurrentUserName())) {
		//TES 11/4/2009 - PLID 35807 - They do, so check whether it is.
		//(e.lally 2011-05-04) PLID 43537 - Use new NexWebDisplayT structure
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
		_RecordsetPtr rsNexWebTemplate = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT EmrTemplateID FROM NexWebDisplayT "
			"WHERE EmrTemplateID = {INT} AND Visible = 1 ", nTemplateID);
		if(!rsNexWebTemplate->eof) {
			// (b.cardillo 2010-09-22 09:18) - PLID 39568 - Corrected wording now that we allow more than one NexWeb EMR template
			if(IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "Warning: This template is a NexWeb template.  It is designed "
				"to be created and filled out by your patients, through your website.  "
				"Are you sure you wish to continue creating an EMN based on this template?")) {
				return FALSE;
			}
		}
	}
	return TRUE;
}


// (s.dhole 2010-02-11 12:05) - PLID 37112 Workflow change from room manager -> EMR for doctors.
// Check any  existing EMN for appoinment date, if only one EMN exist than open same, if  more than one EMN exists than show list

void CRoomManagerDlg::CheckEMNExist(long nPatientID,CString strPatientName )
{
	
	if (m_bEMRExpired) {
		ASSERT(FALSE); // This should not execute. But incase if execute we should able to cache this in debug session
		return;
	}
	try 
	{
		// (z.manning 2011-05-20 14:08) - PLID 33114 - This now filters out EMRs that the user can't access due to chart permissions.
		// (j.jones 2011-07-05 17:49) - PLID 44432 - supported custom statuses
		// (a.walling 2013-07-09 14:20) - PLID 57487 - Inner join with EMRMasterT, PicT
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
		_RecordsetPtr rsEMNCheck = CreateParamRecordset(GetRemoteDataSnapshot(), " SELECT EMRGroupsT.ID AS EMRID, EMRMasterT.ID AS EMNID, PicT.ID AS PicID, EMRMasterT.PatientID AS PatID,  "
                  " dbo.GetEmnProviderList(EMRMasterT.ID) AS ProviderName, EMRMasterT.Description, "
                  " EMRStatusListT.Name AS Status, LocationsT.Name AS LocName, EMRMasterT.Date, EMRMasterT.InputDate, EMRMasterT.ModifiedDate,  "
                  " EMRGroupsT.Description AS EmrDescription,EMRMasterT.TemplateID  "
				" FROM EMRGroupsT INNER JOIN "
                  " EMRMasterT ON EMRGroupsT.ID = EMRMasterT.EMRGroupID INNER JOIN "
				  " PicT ON PicT.EmrGroupID = EMRGroupsT.ID LEFT OUTER JOIN "
                  " LocationsT ON EMRMasterT.LocationID = LocationsT.ID "
				  " LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
				  " LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
				 " WHERE     (EMRMasterT.Deleted = 0) AND (EMRGroupsT.Deleted = 0) AND (PicT.IsCommitted = 1 OR "
                  " PicT.IsCommitted IS NULL) AND (PicT.ID IS NOT NULL)   "
				  " and  EMRMasterT.Date = dbo.AsDateNoTime(GETDATE()) AND  EMRMasterT.PatientID= {INT} {SQL} "
				  , nPatientID, GetEmrChartPermissionFilter());
		if(!rsEMNCheck  ->eof) {
			// (z.manning 2015-05-06 11:08) - NX-100433 - Now have a preference to determine which tab to use
			if (GoToPatient(nPatientID, GetMainFrame()->GetPrimaryEmrTab()))
			{
				long lCount = rsEMNCheck->GetRecordCount();
				if (lCount==1)
				{
					long nEMNID = AdoFldLong(rsEMNCheck, "EMNID");
					long nPicID = AdoFldLong(rsEMNCheck, "PicID");
					// (a.walling 2013-07-09 14:20) - PLID 57487 - TemplateID can correctly be NULL
					long nTemplateID = AdoFldLong(rsEMNCheck, "TemplateID", -1);
					if (CheckEMNWarning(nTemplateID))
					{
						GetMainFrame()->EditEmrRecord(nPicID, nEMNID);
						MinimizeWindow();	
					}
				}
				else if (lCount>1)
				{
					CPatientEmrByDateDlg dlg(this);
					dlg.m_strTitle ="Patient [ " + strPatientName  + "]"; 
					dlg.m_nPatientID =nPatientID;
					long nResult =dlg.DoModal();
					if ( nResult != IDCANCEL) 
						MinimizeWindow();
				}
			}
		}
		else
		{
			MessageBox("There is no existing EMN for this appointment date.","NexEMR",MB_ICONINFORMATION|MB_OK);
		}
		rsEMNCheck->Close();
		}
	NxCatchAll("Error in CRoomManagerDlg::CheckEMNExist");
}

// (s.dhole 2010-02-11 12:05) - PLID 37262 Workflow change from room manager -> EMR for doctors.
// Check appointment associated EMN Collection, if only one EMN exist than open same, if  more than one EMN exists than show list 
void CRoomManagerDlg::CheckEMRTemplateWithCollection(long nAppointmentID,long nPatientID)
{
	if (m_bEMRExpired) {
		ASSERT(FALSE); // This should not execute. But incase if execute we should able to cache this in debug session
		return;
	}
	try 
	{
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
		_RecordsetPtr rsEMRCollectionCheck = CreateParamRecordset(GetRemoteDataSnapshot(), " SELECT   EMRCollectionT.ID,EMRCollectionT.Name,AppointmentsT.AptTypeId ,EmrTemplateT.id as TemplateID"
			" FROM         AppointmentsT inner join EMRCollectionT on (AppointmentsT.AptTypeID = EMRCollectionT.AptTypeID ) "
			" INNER JOIN EmrTemplateT ON EmrTemplateT.CollectionID = EmrCollectionT.ID "
			" where EMRCollectionT.AptTypeID>0 and EMRCollectionT.inactive=0 AND  AppointmentsT.id= {INT} "
			" Order by menuorder ", nAppointmentID);
		if(!rsEMRCollectionCheck  ->eof) {
			long lCount=VarLong(rsEMRCollectionCheck ->GetRecordCount());
			if (lCount==1)
			{
				long nTemplateID= VarLong(rsEMRCollectionCheck ->Fields->Item["TemplateID"]->Value);
				if (CheckEMNWarning(nTemplateID))
				{
					GetMainFrame()->StartNewEMRRecord(nPatientID, -1, nTemplateID);
					MinimizeWindow();
				}
			}
			else if (lCount>1)
			{
				long nAptTypeID = VarLong(rsEMRCollectionCheck ->Fields->Item["AptTypeId"]->Value);
				ShowEMRTemplateCollection(nAptTypeID,nPatientID );

			}
		}
		else
		{
			MessageBox("There is no active EMN template assigned to the current appointment type. ","NexEMR",MB_ICONINFORMATION|MB_OK);
		}
		rsEMRCollectionCheck->Close();

	}NxCatchAll("Error in CRoomManagerDlg::CheckEMRTemplateWithCollection");
}
// (s.dhole 2010-02-11 12:05) - PLID 37262 Workflow change from room manager -> EMR for doctors.
// Display appointment associated EMN Collection
void CRoomManagerDlg::ShowEMRTemplateCollection(long nAptTypeID,long nPatientID)
{
	try
	{
		CSelectDlg dlgSelectMint(this);
		dlgSelectMint.m_strTitle = "Create EMN";
		dlgSelectMint.m_strCaption = "Please select the EMN you wish to create:";
		dlgSelectMint.m_strFromClause.Format("(Select EmrTemplateT.id as EmrTemplateT_id, EmrTemplateT.name as EmrTemplateT_name,EmrCollectionT.id as EmrCollectionT_id,EmrCollectionT.name  as EmrCollectionT_name  ,EMRCollectionT.AptTypeID  From (EmrTemplateT INNER JOIN EmrCollectionT ON EmrTemplateT.CollectionID = EmrCollectionT.ID) Where EMRCollectionT.AptTypeID>0 and EMRCollectionT.inactive=0 and EMRCollectionT.AptTypeID =%li ) Q", nAptTypeID);
		// Tell it to use the ID column
		dlgSelectMint.AddColumn("EmrTemplateT_id", "EmrTemplateT_id", FALSE, FALSE);
		dlgSelectMint.AddColumn("EmrCollectionT_name", "Collection", TRUE, FALSE);
		dlgSelectMint.AddColumn("EmrTemplateT_name", "EMN", TRUE, FALSE);
		if(dlgSelectMint.DoModal() == IDOK) {
			// The user made a selection
			long nEMRTemplateID = VarLong(dlgSelectMint.m_arSelectedValues[0]);
			if ((nEMRTemplateID != -1) && (CheckEMNWarning(nEMRTemplateID))) {
				AddToExistsEMR(nEMRTemplateID,nPatientID);
			}
		} else {
			// The user cancelled
			return;
		}
	}NxCatchAll("Error in CRoomManagerDlg::ShowEMRTemplateCollection");
}
// (s.dhole 2010-02-03 11:45) - PLID 37262 Workflow change from room manager -> EMR for doctors.
//use this code form CPatientNexEMRDlg::OnLeftClickNewList 	case neltTemplate:
// We are not chacking ladder informatin in this code
//This routine will check Add New EMNs To EMR preference setting and display information
void CRoomManagerDlg::AddToExistsEMR(long nEMRTemplateID,long nPatientID)
{
	try
	{
		if(GetRemotePropertyInt("PromptAddNewEMNsToEMR", 1, 0, GetCurrentUserName(), true) == 1) {
			//do they have existing EMRs?
			// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager			
			_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 EMRGroupsT.ID "
				"FROM EMRGroupsT "
				"INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID "
				"WHERE (PicT.IsCommitted IS NULL OR PicT.IsCommitted = 1) AND EMRGroupsT.Deleted = 0 "
				"AND EMRGroupsT.PatientID = {INT} "
				"ORDER BY EMRGroupsT.InputDate DESC", nPatientID);
			if(!rs->eof) {
				//they have at least one EMR, so we will be prompting the user with all EMRs the patient has
				// (j.gruber 2010-01-19 09:12) - PLID 36963 - fixed the ID sent to CalcProcInfoName
				CString strFromClause;
				strFromClause.Format("(SELECT -1 AS PicID, ' {Create New EMR}' AS Description, "
					"'' AS ProcedureNames, NULL AS MinDate "
					""
					"UNION SELECT "
					"PicT.ID AS PicID, "
					"EMRGroupsT.Description, dbo.CalcProcInfoName(PicT.ProcInfoID) AS ProcedureNames, "
					"(SELECT Min(dbo.AsDateNoTime(Date)) AS Date FROM EMRMasterT WHERE Deleted = 0 AND EMRGroupID = EMRGroupsT.ID) AS MinDate "
					"FROM EMRGroupsT "
					"INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID "							
					"WHERE (PicT.IsCommitted IS NULL OR PicT.IsCommitted = 1) AND EMRGroupsT.Deleted = 0 "
					"AND EMRGroupsT.PatientID = %li) Q", nPatientID);

				//there is at least one EMR, so let's give them the list of options
				CSelectDlg dlg(this);
				dlg.m_strTitle = "Add to an existing EMR";
				dlg.m_strCaption = "This patient has existing EMR records. "
					"Please select an existing EMR to associate your new EMN with, or select {Create New EMR} or cancel to create a new EMR record.";
				dlg.m_strFromClause = strFromClause;
				dlg.AddColumn("PicID", "PicID", FALSE, FALSE);
				dlg.AddColumn("Description", "EMR Description", TRUE, TRUE);
				dlg.AddColumn("ProcedureNames", "Procedures", TRUE, TRUE);
				dlg.AddColumn("MinDate", "First EMN Date", TRUE, FALSE, TRUE, 100, 0);
				if(dlg.DoModal() == IDOK) {
					long nPicID = VarLong(dlg.m_arSelectedValues[0]);
					GetMainFrame()->StartNewEMRRecord(nPatientID, nPicID, nEMRTemplateID);
					MinimizeWindow();
					return;
					//if they picked -1, they specifically want to make a new EMR,
					//so we will go on to the ladder check below
				}
			}
			rs->Close();
		}

		// (j.jones 2011-06-17 16:06) - PLID 39632 - these should not have been else cases,
		// in any case we don't add to an existing EMR, make a new EMR

		//No Emr exist create a new EMR,
		GetMainFrame()->StartNewEMRRecord(nPatientID, -1, nEMRTemplateID);
		MinimizeWindow();
	}
	NxCatchAll("Error in CRoomManagerDlg::AddToExistsEMR");
}


// (c.haag 2010-05-07 11:11) - PLID 35702 - This function requeries the room list in such a way as
// to try to keep the vertical scroll position as close to where it was as possible. The other part
// of the functionality is in OnRequeryFinished.
void CRoomManagerDlg::RequeryRoomList()
{
	// First, assign m_nTopRoomID to the Room ID of the first visible row
	m_nTopRoomID = -1;
	IRowSettingsPtr pTopRow = m_pRoomList->TopRow;
	if (NULL != pTopRow) {
		m_nTopRoomID = VarLong(pTopRow->GetValue(rmrlcRoomID),-1);
	}

	// (j.armen 2012-03-05 11:20) - PLID 48555 - Select the highest prority recall status for this patient
	// (a.walling 2013-12-12 16:51) - PLID 60005 - RoomManager only needs a smaller subset of the entire recall query - SelectRecallStatusesPastDueOrNeeded
	CSqlFragment sqlRecall("(SELECT MAX(RecallStatusID) AS RecallStatusID FROM ({SQL}) SubQ WHERE SubQ.PatientID = AppointmentsT.PatientID)", RecallUtils::SelectRecallStatusesPastDueOrNeeded());

	//there's no functional reason to put the datalist's from clause here, it's just easier to read
	// (j.jones 2008-05-29 11:13) - PLID 27797 - ensured we ignore the 'Ready To Check Out' status
	// (j.jones 2008-11-12 16:56) - PLID 28035 - added purpose text
	// (d.thompson 2009-07-10) - PLID 26860 - Added physical arrival time
	// (j.jones 2010-08-27 10:31) - PLID 36975 - the patient name is now calculated by a preference,
	// and that needs to be checked whenever this is requeried
	// (j.jones 2010-08-31 12:03) - PLID 35012 - added AptTypeColor
	// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource
	// (c.haag 2010-10-26 10:07) - PLID 39199 - Changed how we determine whether the appt should be in the waiting area (added WaitingArea)
	// (s.dhole 2010-11-16 15:44) - PLID 39200 - Due to switching between each list , we will chck out appointment before move into  waititng room so we need to chack appointment with chaeckout status
	// (j.jones 2010-12-01 10:08) - PLID 38597 - hide waiting rooms
	CString strRoomFromClause;
	strRoomFromClause.Format("(SELECT "
		"RoomsT.ID AS RoomID, RoomAppointmentsT.ID AS RoomAppointmentID, "
		"AppointmentsT.ID AS AppointmentID, PersonT.ID AS PatientID, RoomsT.LocationID AS RoomLocationID, "
		"RoomsT.Name AS RoomName, %s AS PatientName, %s AS RecallStatusID, "
		"AppointmentsT.StartTime, "
		"(SELECT MAX(TimeStamp) FROM AptShowStateHistoryT WHERE AptShowStateHistoryT.AppointmentID = AppointmentsT.ID AND ShowStateID IN (SELECT ID FROM AptShowStateT WHERE WaitingArea = CONVERT(BIT,1))) AS LastArrivalTime, "
		"RoomAppointmentsT.CheckInTime, "
		""
		"(SELECT Min(UpdateTime) AS FirstTimeWithoutPerson FROM RoomAppointmentHistoryT "
		"	WHERE StatusID IN (SELECT ID FROM RoomStatusT WHERE WithPerson = 0) "
		"	AND RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID "
		"	AND UpdateTime > (SELECT Max(UpdateTime) AS LastStartTimeWithPerson FROM RoomAppointmentHistoryT "
		"	WHERE StatusID IN (SELECT ID FROM RoomStatusT WHERE WithPerson = 1) "
		"	AND RoomAppointmentHistoryT.RoomAppointmentID = RoomAppointmentsT.ID)) "
		"AS TimeLastSeenByPerson, "
		""
		"RoomAppointmentsT.StatusID, RoomStatusT.Name AS Status, "
		"RoomStatusT.WithPerson, RoomAppointmentsT.LastUpdateUserID, "
		"UsersT.Username, "
		"dbo.GetPurposeString(AppointmentsT.ID) AS PurposeName, "
		"AptTypeT.Name AS TypeName, CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS AptTypeColor, "
		"Provider.Last + ', ' + Provider.First + ' ' + Provider.Middle AS Provider, "
		" dbo.GetResourceString(AppointmentsT.ID) AS Resources "
		"FROM RoomsT "
		"LEFT JOIN (SELECT * FROM RoomAppointmentsT WHERE StatusID NOT IN (0, -1 )) AS RoomAppointmentsT ON RoomsT.ID = RoomAppointmentsT.RoomID " // (s.dhole 2010-11-16 15:44) - PLID 39200
		"LEFT JOIN AppointmentsT ON RoomAppointmentsT.AppointmentID = AppointmentsT.ID "
		"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
		"LEFT JOIN RoomStatusT ON RoomAppointmentsT.StatusID = RoomStatusT.ID "
		"LEFT JOIN UsersT ON RoomAppointmentsT.LastUpdateUserID = UsersT.PersonID "
		"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
		"LEFT JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID "
		"LEFT JOIN PersonT AS Provider ON PatientsT.MainPhysician = Provider.ID "
		"WHERE RoomsT.Inactive = 0 AND RoomsT.WaitingRoom = 0 "
		") AS RoomDataQ", GetPatientNameSqlFormat().Flatten(), sqlRecall.Flatten());

	m_pRoomList->FromClause = _bstr_t(strRoomFromClause);

	// Now do the requery
	m_pRoomList->Requery();
	
}

// (d.lange 2010-06-28 14:57) - PLID 37317 - This function will launch the Configure Columns dialog for configuring the three datalists
void CRoomManagerDlg::OnBnClickedBtnConfigColumns()
{
	try{
		CRoomManagerConfigureColumnsDlg dlg(this);
		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2010-06-28 16:10) - PLID 37317 - Determines which columns to show in the Waiting Area datalist
void CRoomManagerDlg::EnsureWaitingAreaColumns()
{
	try {
		NXDATALIST2Lib::IColumnSettingsPtr pCol;

		// (j.jones 2010-12-01 17:49) - PLID 38597 - the room column is hidden by default
		pCol = m_pWaitingAreaList->GetColumn(rmwalcWaitingRoomName);
		long nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerWaitingAreaColumnRoom", 0, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 120;
			nStyle = (nStyle & ~NXDATALIST2Lib::csFixedWidth) | NXDATALIST2Lib::csWidthAuto;
		}else {
			nStyle = (nStyle & ~NXDATALIST2Lib::csWidthAuto) | NXDATALIST2Lib::csFixedWidth;
			pCol->StoredWidth = 0;
		}

		pCol = m_pWaitingAreaList->GetColumn(rmwalcPatientName);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerWaitingAreaColumnPatient Name", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 140; 
		}else{
			nStyle = NXDATALIST2Lib::csFixedWidth | NXDATALIST2Lib::csVisible;
			pCol->ColumnStyle = nStyle;
			pCol->StoredWidth = 0;
		}

		pCol = m_pWaitingAreaList->GetColumn(rmwalcApptTime);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerWaitingAreaColumnAppointment Time", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 125;
			nStyle = (nStyle & ~NXDATALIST2Lib::csFixedWidth) | NXDATALIST2Lib::csWidthAuto;
		}else {
			nStyle = (nStyle & ~NXDATALIST2Lib::csWidthAuto) | NXDATALIST2Lib::csFixedWidth; 
			pCol->StoredWidth = 0;
		}
		//pCol->PutColumnStyle(nStyle);

		pCol = m_pWaitingAreaList->GetColumn(rmwalcPurpose);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerWaitingAreaColumnPurpose", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 170;
		}else { 
			pCol->StoredWidth = 0;
		}

		pCol = m_pWaitingAreaList->GetColumn(rmwalcCheckInTime);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerWaitingAreaColumnChecked In", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 82;
			nStyle = (nStyle & ~NXDATALIST2Lib::csFixedWidth) | NXDATALIST2Lib::csWidthAuto;
		}else {
			nStyle = (nStyle & ~NXDATALIST2Lib::csWidthAuto) | NXDATALIST2Lib::csFixedWidth;
			pCol->StoredWidth = 0;
		}
		//pCol->PutColumnStyle(nStyle);

		pCol = m_pWaitingAreaList->GetColumn(rmwalcTimeLastSeen);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerWaitingAreaColumnLast Seen", 0, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 60;
			nStyle = (nStyle & ~NXDATALIST2Lib::csFixedWidth) | NXDATALIST2Lib::csWidthAuto;
		}else {
			nStyle = (nStyle & ~NXDATALIST2Lib::csWidthAuto) | NXDATALIST2Lib::csFixedWidth; 
			pCol->StoredWidth = 0;
		}
		//pCol->PutColumnStyle(nStyle);

		pCol = m_pWaitingAreaList->GetColumn(rmwalcWaiting);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerWaitingAreaColumnWaiting", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 65;
		}else {
			pCol->StoredWidth = 0;
		}

		// (d.lange 2010-08-31 08:42) - PLID 39431 - added the provider column, hidden by default
		pCol = m_pWaitingAreaList->GetColumn(rmwalcProvider);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerWaitingAreaColumnProvider", 0, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 120;
			nStyle = (nStyle & ~NXDATALIST2Lib::csFixedWidth) | NXDATALIST2Lib::csWidthAuto;
		}else {
			nStyle = (nStyle & ~NXDATALIST2Lib::csWidthAuto) | NXDATALIST2Lib::csFixedWidth;
			pCol->StoredWidth = 0;
		}

		// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource
		pCol = m_pWaitingAreaList->GetColumn(rmwalcResources);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerWaitingAreaColumnResources", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 120;
			nStyle = (nStyle & ~NXDATALIST2Lib::csFixedWidth) | NXDATALIST2Lib::csWidthAuto;
		}else {
			nStyle = (nStyle & ~NXDATALIST2Lib::csWidthAuto) | NXDATALIST2Lib::csFixedWidth;
			pCol->StoredWidth = 0;
		}

		pCol = m_pWaitingAreaList->GetColumn(rmwalcCheckedInBy);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerWaitingAreaColumnChecked In By", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 130;
		}else { 
			pCol->StoredWidth = 0;
		}

		// (d.lange 2010-11-29 14:47) - PLID 40295 - added Preview EMNs
		pCol = m_pWaitingAreaList->GetColumn(rmwalcPreviewEMN);
		nStyle = pCol->GetColumnStyle();
		if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
			if(1 == GetRemotePropertyInt("ShowRoomManagerWaitingAreaColumnPreview EMN", 0, 0, GetCurrentUserName(), true)) {
				pCol->StoredWidth = 20;
			}else {
				pCol->ColumnStyle = NXDATALIST2Lib::csFixedWidth | NXDATALIST2Lib::csVisible;
				pCol->StoredWidth = 0;
			}
		}else {
			// If EMR is not activated, hide the column and make sure you can't resize to see it
			pCol->ColumnStyle = NXDATALIST2Lib::csFixedWidth | NXDATALIST2Lib::csVisible;
			pCol->StoredWidth = 0;
		}

		// (j.armen 2012-06-14 14:55) - PLID 48555 - Update Recall Flag
		EnsureRecallFlagColumn(m_pWaitingAreaList, rmwalcRecallStatusID, rmwalcRecallStatus, "ShowRoomManagerWaitingAreaColumnRecall");

		CString str, strList;

		for(int i = 0; i < m_pWaitingAreaList->GetColumnCount(); i++) {
			pCol = m_pWaitingAreaList->GetColumn(i);
			if(pCol)
				str.Format("%li,", pCol->GetStoredWidth());

			strList += str;
		}

		//write it to ConfigRT
		SetRemotePropertyText("DefaultRoomManagerWaitingColumnSizes", strList, 0, GetCurrentUserName());

		SetColumnSizes();

		m_strWaitingColumnWidths = "";
		SetDefaultColumnWidths();

	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2010-06-30 10:13) - PLID 37317 - Determines which columns to show in the Rooms datalist
void CRoomManagerDlg::EnsureRoomsColumns()
{
	try {
		NXDATALIST2Lib::IColumnSettingsPtr pCol;

		pCol = m_pRoomList->GetColumn(rmrlcRoomName);
		long nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnRoom", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 120;

		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pRoomList->GetColumn(rmrlcPatientName);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnPatient Name", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 120;

		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pRoomList->GetColumn(rmrlcPurpose);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnPurpose", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 85;

		}else {
			pCol->StoredWidth = 0;
		}

		// (j.jones 2010-08-27 09:35) - PLID 39774 - added appt. time, defaults to hidden
		pCol = m_pRoomList->GetColumn(rmrlcApptTime);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnAppointment Time", 0, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 125;

		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pRoomList->GetColumn(rmrlcArrivalTime);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnArrival Time", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 90;

		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pRoomList->GetColumn(rmrlcCheckInTime);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnChecked In", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 82;

		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pRoomList->GetColumn(rmrlcTimeLastSeen);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnLast Seen", 0, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 60;

		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pRoomList->GetColumn(rmrlcStatusID);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnStatus", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 140;

		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pRoomList->GetColumn(rmrlcWaiting);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnWaiting", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 65;

		}else {
			pCol->StoredWidth = 0;
		}

		// (d.lange 2010-08-27 18:09) - PLID 39431 - Added the Provider column, hidden by default
		pCol = m_pRoomList->GetColumn(rmrlcProvider);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnProvider", 0, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 120;
		}else {
			pCol->StoredWidth = 0;
		}


		// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource column, hidden by default
		pCol = m_pRoomList->GetColumn(rmrlcResources );
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnResources", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 120;
		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pRoomList->GetColumn(rmrlcLastUserName);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnLast Updated By", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 130;

		}else {
			pCol->StoredWidth = 0;
		}

		// (d.lange 2010-11-29 14:47) - PLID 40295 - added Preview EMNs
		pCol = m_pRoomList->GetColumn(rmrlcPreviewEMN);
		nStyle = pCol->GetColumnStyle();
		if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
			if(1 == GetRemotePropertyInt("ShowRoomManagerRoomsColumnPreview EMN", 0, 0, GetCurrentUserName(), true)) {
				pCol->StoredWidth = 20;
			}else {
				pCol->ColumnStyle = NXDATALIST2Lib::csFixedWidth | NXDATALIST2Lib::csVisible;
				pCol->StoredWidth = 0;
			}
		}else {
			// If EMR is not activated, hide the column and make sure you can't resize to see it
			pCol->ColumnStyle = NXDATALIST2Lib::csFixedWidth | NXDATALIST2Lib::csVisible;
			pCol->StoredWidth = 0;
		}

		// (j.armen 2012-06-14 14:55) - PLID 48555 - Update Recall Flag
		EnsureRecallFlagColumn(m_pRoomList, rmrlcRecallStatusID, rmrlcRecallStatus, "ShowRoomManagerRoomsColumnRecall");

		CString str, strList;

		for(int i = 0; i < m_pRoomList->GetColumnCount(); i++) {
			pCol = m_pRoomList->GetColumn(i);
			if(pCol)
				str.Format("%li,", pCol->GetStoredWidth());

			strList += str;
		}

		//write it to ConfigRT
		SetRemotePropertyText("DefaultRoomManagerRoomColumnSizes", strList, 0, GetCurrentUserName());

		SetColumnSizes();

		m_strRoomColumnWidths = "";
		SetDefaultColumnWidths();

	} NxCatchAll(__FUNCTION__);
}

// (d.lange 2010-06-30 10:36) - PLID 37317 - Determines which columns to show in the Checkout datalist
void CRoomManagerDlg::EnsureCheckoutColumns()
{
	try {
		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pCheckoutList->GetColumn(rmclcPatientName);
		long nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerCheckoutColumnPatient Name", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 150;
		}else {
			nStyle = NXDATALIST2Lib::csFixedWidth | NXDATALIST2Lib::csVisible;
			pCol->ColumnStyle = nStyle;
			pCol->StoredWidth = 0;
		}

		pCol = m_pCheckoutList->GetColumn(rmclcApptTime);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerCheckoutColumnAppointment Time", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 125;
		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pCheckoutList->GetColumn(rmclcPurpose);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerCheckoutColumnPurpose", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 120;
		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pCheckoutList->GetColumn(rmclcCheckInTime);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerCheckoutColumnChecked In", 0, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 125;
		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pCheckoutList->GetColumn(rmclcTimeLeftRoom);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerCheckoutColumnTime Left Room", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 110;
		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pCheckoutList->GetColumn(rmclcWaiting);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerCheckoutColumnWaiting", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 65;
		}else {
			pCol->StoredWidth = 0;
		}

		// (d.lange 2010-08-31 08:51) - PLID 39431 - added provider column, hidden by default
		pCol = m_pCheckoutList->GetColumn(rmclcProvider);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerCheckoutColumnProvider", 0, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 120;
		}else {
			pCol->StoredWidth = 0;
		}

		// (s.dhole 2010-10-06 11:17) - PLID 30662 - Added Resource column, hidden by default
		pCol = m_pCheckoutList->GetColumn(rmclcResources );
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerCheckoutColumnResources", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 120;
		}else {
			pCol->StoredWidth = 0;
		}

		pCol = m_pCheckoutList->GetColumn(rmclcLastUserName);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowRoomManagerCheckoutColumnLast Updated By", 1, 0, GetCurrentUserName(), true)) {
			pCol->StoredWidth = 120;
		}else {
			pCol->StoredWidth = 0;
		}

		// (d.lange 2010-11-29 14:47) - PLID 40295 - added Preview EMNs
		pCol = m_pCheckoutList->GetColumn(rmclcPreviewEMN);
		nStyle = pCol->GetColumnStyle();
		if(g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
			if(1 == GetRemotePropertyInt("ShowRoomManagerCheckoutColumnPreview EMN", 0, 0, GetCurrentUserName(), true)) {
				pCol->StoredWidth = 20;
			}else {
				pCol->ColumnStyle = NXDATALIST2Lib::csFixedWidth | NXDATALIST2Lib::csVisible;
				pCol->StoredWidth = 0;
			}
		}else {
			// If EMR is not activated, hide the column and make sure you can't resize to see it
			pCol->ColumnStyle = NXDATALIST2Lib::csFixedWidth | NXDATALIST2Lib::csVisible;
			pCol->StoredWidth = 0;
		}

		// (j.armen 2012-06-14 14:55) - PLID 48555 - Update Recall Flag
		EnsureRecallFlagColumn(m_pCheckoutList, rmclcRecallStatusID, rmclcRecallStatus, "ShowRoomManagerCheckoutColumnRecall");

		CString str, strList;

		for(int i = 0; i < m_pCheckoutList->GetColumnCount(); i++) {
			pCol = m_pCheckoutList->GetColumn(i);
			if(pCol)
				str.Format("%li,", pCol->GetStoredWidth());

			strList += str;
		}

		//write it to ConfigRT
		SetRemotePropertyText("DefaultRoomManagerCheckoutColumnSizes", strList, 0, GetCurrentUserName());

		SetColumnSizes();

		m_strCheckoutColumnWidths = "";
		SetDefaultColumnWidths();

	} NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-08-27 10:22) - PLID 36975 - called when Preferences are changed,
// it means this dialog must check to see if the patient name preference has changed
void CRoomManagerDlg::OnPreferencesChanged()
{
	//this function can be used to check if any preferences changed,
	//but in the initial implementation it is only checking the patient name preference

	try {

		BOOL bRequeryAll = FALSE;

		// (j.jones 2010-08-27 10:29) - PLID 36975 - reload the patient name preference
		ERoomMgrPatientNameDisplayPref eNewRoomMgrPatientNameDisplayPref = (ERoomMgrPatientNameDisplayPref)GetRemotePropertyInt("RoomMgrPatientNameDisplay", (long)patnameFullName, 0, GetCurrentUserName(), true);
		if(m_eRoomMgrPatientNameDisplayPref != eNewRoomMgrPatientNameDisplayPref) {
			//it changed, we need to fire a full reload
			m_eRoomMgrPatientNameDisplayPref = eNewRoomMgrPatientNameDisplayPref;
			bRequeryAll = TRUE;
		}

		// (j.jones 2010-08-31 14:45) - PLID 35012 - reload the coloring preferences
		BOOL bNewColorApptListByType = GetRemotePropertyInt("RoomMgr_ColorApptListByType", 0, 0, GetCurrentUserName(), true) == 1;		
		if(bNewColorApptListByType != m_bColorApptListByType) {
			m_bColorApptListByType = bNewColorApptListByType;
			bRequeryAll = TRUE;
		}
		
		BOOL bNewColorPurposeColumnByType = GetRemotePropertyInt("RoomMgr_ColorPurposeColumnByType", 0, 0, GetCurrentUserName(), true) == 1;
		if(bNewColorPurposeColumnByType != m_bColorPurposeColumnByType) {
			m_bColorPurposeColumnByType = bNewColorPurposeColumnByType;
			bRequeryAll = TRUE;
		}

		if(bRequeryAll) {
			RequeryAll(TRUE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-08-27 10:33) - PLID 36975 - returns how the patient name should
// be loaded in a query, based on the patient name preference
// (j.jones 2014-08-15 15:58) - PLID 63185 - changed to return a SQL fragment
CSqlFragment CRoomManagerDlg::GetPatientNameSqlFormat()
{
	switch(m_eRoomMgrPatientNameDisplayPref) {

		case patnameFirstNameLastInitial:
			return CSqlFragment("PersonT.First + ' ' + CASE WHEN PersonT.Last = '' THEN '' ELSE Left(PersonT.Last, 1) + '.' END");
			break;
		case patnameFirstInitialLastName:
			return CSqlFragment("CASE WHEN PersonT.First = '' THEN '' ELSE Left(PersonT.First, 1) + '.' END + ' ' + PersonT.Last");
			break;
		case patnameLastNameFirstInitial:
			return CSqlFragment("PersonT.Last + CASE WHEN PersonT.First = '' THEN '' ELSE ', ' + Left(PersonT.First, 1) + '.' END");
			break;
		case patnameFirstNameOnly:
			return CSqlFragment("PersonT.First");
			break;
		case patnameLastNameOnly:
			return CSqlFragment("PersonT.Last");
			break;
		case patnameFirstInitialLastInitial:
			return CSqlFragment("CASE WHEN PersonT.First = '' THEN '' ELSE Left(PersonT.First, 1) + '.' END + ' ' + CASE WHEN PersonT.Last = '' THEN '' ELSE Left(PersonT.Last, 1) + '.' END");
			break;
		case patnameFullName:
		default:
			return CSqlFragment("PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle");
			break;
	}
}

// (j.jones 2010-10-13 15:51) - PLID 39778 - returns today's EMNID, sorted by unbilled first,
// filtered only on EMNs that have charges
long CRoomManagerDlg::GetTodaysBillableEMNForPatient(const long nPatientID)
{
	try {

		long nEMNID = -1;

		//find an EMN for this patient with today's date, that has charges, sorted by
		//the first EMN entered that has no bill, otherwise just the first EMN entered
		// (a.walling 2013-12-12 16:51) - PLID 59997 - Snapshot isolation in RoomManager
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 EMRMasterT.ID AS EMNID "
			"FROM EMRMasterT "
			"INNER JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
			"INNER JOIN PicT ON PicT.EmrGroupID = EMRGroupsT.ID "
			"LEFT JOIN (SELECT EMNID FROM BilledEMNsT "
			"	INNER JOIN BillsT ON BilledEMNsT.BillID = BillsT.ID "
			"	WHERE BillsT.Deleted = 0 AND BillsT.EntryType = 1 AND BillsT.PatientID = {INT}) AS BilledEMNsQ ON EMRMasterT.ID = BilledEMNsQ.EMNID "
			"INNER JOIN (SELECT EMRID FROM EMRChargesT "
			"	WHERE EMRChargesT.Deleted = 0 "
			"	GROUP BY EMRID) AS HasEMRChargesQ ON EMRMasterT.ID = HasEMRChargesQ.EMRID "
			"WHERE EMRMasterT.Deleted = 0 AND EMRGroupsT.Deleted = 0 AND PicT.IsCommitted = 1 "
			"AND dbo.AsDateNoTime(EMRMasterT.Date) = dbo.AsDateNoTime(GetDate()) "
			"AND EMRMasterT.PatientID = {INT} "
			"ORDER BY (CASE WHEN BilledEMNsQ.EMNID Is Null THEN 0 ELSE 1 END) ASC, EMRMasterT.ID ASC", nPatientID, nPatientID);
		if(!rs->eof) {
			nEMNID = AdoFldLong(rs, "EMNID", -1);
		}
		rs->Close();

		return nEMNID;

	}NxCatchAll(__FUNCTION__);

	return -1;
}

// (j.jones 2010-10-14 08:49) - PLID 39778 - open a bill with a given EMNID, may generate a menu first
void CRoomManagerDlg::BillEMNForPatient(const long nPatientID, const long nEMNID, const long nInsuredPartyID)
{
	try {

		if(nPatientID == -1 || nEMNID == -1 || nInsuredPartyID == -2) {
			return;
		}

		if(!CheckCurrentUserPermissions(bioBill,sptCreate)) {
			return;
		}

		//Set the active patient
		CMainFrame *pMainFrame;
		pMainFrame = GetMainFrame();
		if (pMainFrame != NULL) {

			if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
				if(IDNO == MessageBox("This patient is not in the current lookup. \n"
					"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
					return;
				}
			}
			//TES 1/7/2010 - PLID 36761 - This function may fail now
			if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

				//Now just flip to the patient's module and set the active Patient
				pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
				CNxTabView *pView = pMainFrame->GetActiveView();
				if(pView)
					pView->UpdateView();
			}
			else {
				return;
			}

		}//end if MainFrame
		else {
			MsgBox(MB_ICONSTOP|MB_OK, "ERROR - CRoomManagerDlg::BillEMNForPatient: Cannot Open Mainframe");
			return;
		}//end else pMainFrame

		//checks for any active global period, and warns accordingly
		// (a.walling 2008-07-07 18:04) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
		if(!CheckWarnGlobalPeriod(nPatientID))
			return;

		//bill this EMR

		// (a.walling 2009-12-22 17:27) - PLID 7002 - Maintain only one instance of a bill
		if (GetMainFrame()->IsBillingModuleOpen(true)) {
			return;
		}

		CPatientView* pView = (CPatientView*)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
		if(pView) {
			pView->SetActiveTab(PatientsModule::BillingTab);
			CBillingModuleDlg *pBillingDlg = pView->GetBillingDlg();
			if(pBillingDlg) {
				pBillingDlg->m_pFinancialDlg = pView->GetFinancialDlg();
				pBillingDlg->m_nPatientID = nPatientID;
				
				// (j.jones 2011-06-30 09:00) - PLID 43770 - track if we are being created from an EMR
				// (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum
				pBillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1, BillFromType::EMR);
				pBillingDlg->PostMessage(NXM_BILL_EMR, nEMNID, nInsuredPartyID);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

	// (j.gruber 2013-04-02 12:29) - PLID 56012 - Not Needed Anymore// (j.jones 2010-12-02 09:09) - PLID 38597 - requery waiting room combo
void CRoomManagerDlg::RequeryWaitingRoomCombo()
{
	//get the location ID
	long nLocationID = -1;
	{
		IRowSettingsPtr pRow = m_pLocationCombo->GetCurSel();
		if(pRow == NULL) {
			nLocationID = GetCurrentLocationID();
			m_pLocationCombo->SetSelByColumn(lccID, nLocationID);			
		}
		else {
			nLocationID = VarLong(pRow->GetValue(lccID));
		}
	}

	//get the current setting, if there isn't one, load the user's
	//last setting (which should always happen on the first requery),
	//and if that doesn't exist, just select "all rooms" but do
	//not reset their stored value
	long nWaitingRoomID = -1;
	{
		IRowSettingsPtr pRow = m_pWaitingRoomCombo->GetCurSel();
		if(pRow == NULL) {
			//nothing selected, load their saved value
			nWaitingRoomID = GetRemotePropertyInt("RoomManagerWaitingRoomFilter", -1, 0, GetCurrentUserName(), true);
		}
		else {
			nWaitingRoomID = VarLong(pRow->GetValue(wrccID));
		}
	}
	
	CString strWaitingRoomWhere;
	strWaitingRoomWhere.Format("Inactive = 0 AND WaitingRoom = 1 AND LocationID = %li", nLocationID);
	m_pWaitingRoomCombo->PutWhereClause(_bstr_t(strWaitingRoomWhere));
	m_pWaitingRoomCombo->Requery();

	IRowSettingsPtr pRow = m_pWaitingRoomCombo->GetNewRow();
	pRow->PutValue(wrccID, (long)-1);
	pRow->PutValue(wrccName, " <All Waiting Rooms>");
	m_pWaitingRoomCombo->AddRowSorted(pRow, NULL);
	pRow = m_pWaitingRoomCombo->GetNewRow();
	pRow->PutValue(wrccID, (long)-2);
	pRow->PutValue(wrccName, " <No Assigned Room>");
	m_pWaitingRoomCombo->AddRowSorted(pRow, NULL);

	//set the default room
	IRowSettingsPtr pSelRow = m_pWaitingRoomCombo->SetSelByColumn(wrccID, (long)nWaitingRoomID);
	if(pSelRow == NULL) {
		//select "all rooms"
		m_pWaitingRoomCombo->SetSelByColumn(wrccID, (long)-1);
	}

	//do not requery the waiting area list, the caller is responsible for that
}

// (j.jones 2010-12-02 09:24) - PLID 38597 - added waiting room combo
void CRoomManagerDlg::OnSelChosenRoomMgrWaitingRoomCombo(LPDISPATCH lpRow)
{
	try {

		//ensure a selection is made
		long nWaitingRoomID = -1;
		IRowSettingsPtr pRow = m_pWaitingRoomCombo->GetCurSel();
		if(pRow == NULL) {
			//set to all rooms
			m_pWaitingRoomCombo->SetSelByColumn(wrccID, (long)-1);			
		}
		else {
			nWaitingRoomID = VarLong(pRow->GetValue(wrccID));
		}

		//requery the waiting area
		RequeryWaitingAreaList();

		//save this setting per user
		SetRemotePropertyInt("RoomManagerWaitingRoomFilter", nWaitingRoomID, 0, GetCurrentUserName());
		
	}NxCatchAll(__FUNCTION__);
}

// (d.lange 2010-12-03 12:53) - PLID 40295 - Ensure that the member EMRPreviewPopupDlg is created
void CRoomManagerDlg::CreateEmrPreviewDialog(long nPatientID)
{
	try {
		if(m_pEMRPreviewPopupDlg == NULL) {

			m_pEMRPreviewPopupDlg = new CEMRPreviewPopupDlg(this);
			m_pEMRPreviewPopupDlg->Create(IDD_EMR_PREVIEW_POPUP, this);

			CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
			m_pEMRPreviewPopupDlg->SetPatientID(nPatientID, aryEMNs);
			m_pEMRPreviewPopupDlg->RestoreSize("");

			m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
		}

	} NxCatchAll("CRoomManagerDlg::CreateEmrPreviewDialog");
}
//(a.wilson 2011-10-27) PLID 45936 - to capture the mouse move event.
BOOL CRoomManagerDlg::PreTranslateMessage(MSG* pMsg)
{
	try {
		if (pMsg->message == WM_MOUSEMOVE) {
			//check to make sure the picturetooltip object has been registered before continuing.
			if (m_pttPatientList.IsInitialized()) {	//(a.wilson 2011-11-7) PLID 46314 - already handled
				CPoint ptCursor;
				GetCursorPos(&ptCursor);
				CRect rcPatientList;
				GetDlgItem(IDC_TODAYS_PATIENTS_LIST)->GetWindowRect(&rcPatientList);

				//check to see if the cursor is over the datalist in question
				if (rcPatientList.PtInRect(ptCursor)) {
					IRowSettingsPtr pRow = m_pPatientList->GetRowFromPoint(ptCursor.x - rcPatientList.left, ptCursor.y - rcPatientList.top);
					if (pRow) {	//ensure valid row
						CString strPicturePath = "";
						CString strPatientName = VarString(pRow->GetValue(rmplcPatientName), "");
						long nPatientID = VarLong(pRow->GetValue(rmplcPatientID), 0);
						_variant_t varPicturePath = pRow->GetValue(rmplcPatientPicturePath);

						if (varPicturePath.vt == VT_BSTR) {
							strPicturePath = VarString(varPicturePath, "");
						}
						//check to make sure the row is a patient and the picture is valid.
						if (!strPatientName.IsEmpty() && nPatientID > 0 && !strPicturePath.IsEmpty() && FileUtils::DoesFileOrDirExist(strPicturePath)) {
							ptCursor.Offset(15, 15);
							m_pttPatientList.SetValues(strPicturePath, ptCursor);
							m_pttPatientList.Show();
						} else {//if anything does not occur then we need to make sure the tooltip is hidden.
							m_pttPatientList.Hide();
						}
					} else {
						m_pttPatientList.Hide();
					}
				} else {
					m_pttPatientList.Hide();
				}
			}
		}
	} NxCatchAll(__FUNCTION__);

	return CNxModelessOwnedDialog::PreTranslateMessage(pMsg);
}
//(a.wilson 2011-11-4) PLID 45936 - to ensure the tooltip hides when the room manager is not active.
void CRoomManagerDlg::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
	try {
		//(a.wilson 2011-11-7) PLID 46314 - need to check if object was initialized.
		if (m_pttPatientList.IsInitialized()) {
			if ((nState == WA_INACTIVE || bMinimized == TRUE)) {
				m_pttPatientList.Hide();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-03-05 11:28) - PLID 48555 - Handle when a recall flag is clicked.  Perform action based on the status of the flag
void CRoomManagerDlg::OnRecallFlagClicked(NXDATALIST2Lib::IRowSettingsPtr& pRow, const short& nStatusCol, const short& nPatientIDCol)
{
	// (j.armen 2012-03-28 10:37) - PLID 48480 - Only run these functions if we have the license
	if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) {
		switch(AsLong(pRow->GetValue(nStatusCol)))
		{
			case RecallUtils::eNone:
				// Pt has No Recalls at all
				break;
			case RecallUtils::eDiscontinued:
				// Pt's highest recall priority is Discontinued
				break;
			case RecallUtils::eComplete:
				// Pt's highest recall priority is Complete
				break;
			case RecallUtils::eScheduled:
				// Pt's highest recall priority is Scheduled
				break;
			case RecallUtils::eNeedToSchedule:
				// Pt's highest recall priority is Need to Schedule
				GetMainFrame()->GotoPatient(VarLong(pRow->GetValue(nPatientIDCol)));
				GetMainFrame()->ShowRecallsNeedingAttention(true);
				break;
			case RecallUtils::ePastDue:
				// Pt's highest recall priority is Past Due
				GetMainFrame()->GotoPatient(VarLong(pRow->GetValue(nPatientIDCol)));
				GetMainFrame()->ShowRecallsNeedingAttention(true);
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
}

// (j.armen 2012-03-05 11:29) - PLID 48555 - Make sure that the recall flags show correctly, and that the column follows the preference
void CRoomManagerDlg::EnsureRecallFlagColumn(NXDATALIST2Lib::_DNxDataListPtr& pDL, const short& nStatusCol, const short& nIconCol, const CString& strPreference)
{
	bool bExpandCol = false;

	// (j.armen 2012-03-28 10:41) - PLID 48480 - If we don't have the license, then this column should never show
	if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent))
	{
		if(1 == GetRemotePropertyInt(strPreference, 1, 0, GetCurrentUserName(), true))
		{
			for(NXDATALIST2Lib::IRowSettingsPtr pRow = pDL->FindAbsoluteFirstRow(VARIANT_TRUE);
				pRow != NULL; pRow = pRow->GetNextRow()) {
					switch(AsLong(pRow->GetValue(nStatusCol)))
					{
						case RecallUtils::eNone:
							// Pt has No Recalls at all
							break;
						case RecallUtils::eDiscontinued:
							// Pt's highest recall priority is Discontinued
							break;
						case RecallUtils::eComplete:
							// Pt's highest recall priority is Complete
							break;
						case RecallUtils::eScheduled:
							// Pt's highest recall priority is Scheduled
							break;
						case RecallUtils::eNeedToSchedule:
							// Pt's highest recall priority is Need to Schedule
							bExpandCol = true;
							pRow->PutValue(nIconCol, (long)m_hIconRecallFlag);
							break;
						case RecallUtils::ePastDue:
							// Pt's highest recall priority is Past Due
							bExpandCol = true;
							pRow->PutValue(nIconCol, (long)m_hIconRecallFlag);
							break;
						default:
							// Unknown status - need to add to enumeration
							AfxThrowNxException("Unknown Recall Status");
							break;
					}
			}
		}
	}

	pDL->GetColumn(nIconCol)->StoredWidth = bExpandCol ? 50 : 0;
}
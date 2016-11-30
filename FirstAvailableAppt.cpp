// FirstAvailableAppt.cpp : implementation file
//
// (c.haag 2006-11-21 09:51) - When we re-write FFA, we should use this query to pull
// all template rules that would affect a search.
/*

-- This query returns a list of all template rules that may affect the outcome of a
-- Find First Available appointment search for a given time range

-- TemplateRuleDetailsT.ObjectType:
-- 	0 == ignore type list
--	1 == an appointment type IS in the list
--	2 == an appointment purpose IS in the list
--	3 == may be booked against n or more appointments
--	101 == an appointment type IS NOT in the list
--	102 == an appointment purpose IS NOT in the list

DECLARE @AptTypeID int
DECLARE @AptPurposeID int

-- Designate the appointment type ID (this is currently a random value used for testing)
SET @AptTypeID = 43
-- Designate the appointment purpose ID (this is currently a random value used for testing)
SET @AptPurposeID = -1

-- Main query
SELECT TemplateRuleT.*
FROM TemplateRuleT
WHERE
	-- The rule applies to any appointment
	AllAppts = 1
	OR (
		-- Any of the following can be true
		AndDetails = 0 AND (
			-- The appointment type must be in the list
			(@AptTypeID IN (SELECT ObjectID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 1))
			OR
			-- The appointment type must not be in the list
			(EXISTS(SELECT TOP 1 ID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 101) AND @AptTypeID NOT IN (SELECT ObjectID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 101))
			-- If we are checking appointment purposes
			OR
			-- The appointment purpose must be in the list
			(@AptPurposeID IN (SELECT ObjectID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 2))
			OR
			-- The appointment purpose must not be in the list
			(EXISTS(SELECT TOP 1 ID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 102) AND @AptPurposeID NOT IN (SELECT ObjectID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 102))
			OR
			-- The appointment would be booked against n or more appointments. Rather than running queries on appointments, which is slow, we will simply
			-- say that this template -may- prevent the user from scheduling an appointment
			(EXISTS(SELECT TOP 1 ID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 3))
		)
	)
	OR (
		-- All of the following must be true
		AndDetails = 1 AND (
			(
				-- The appointment type must be in the list
				(CASE
					WHEN @AptTypeID IN (SELECT ObjectID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 1) THEN 1
					WHEN NOT EXISTS(SELECT TOP 1 ID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 1) THEN 1
					ELSE 0 END
				) &
				-- The appointment type must not be in the list
				(CASE
					WHEN EXISTS(SELECT TOP 1 ID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 101) AND @AptTypeID NOT IN (SELECT ObjectID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 101) THEN 1
					WHEN NOT EXISTS(SELECT TOP 1 ID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 101) THEN 1
					ELSE 0 END
				) &
				-- The appointment purpose must be in the list
				(CASE
					WHEN @AptPurposeID IN (SELECT ObjectID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 2) THEN 1
					WHEN NOT EXISTS(SELECT TOP 1 ID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 2) THEN 1
					ELSE 0 END
				) &
				-- The appointment purpose must not be in the list
				(CASE
					WHEN EXISTS(SELECT TOP 1 ID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 102) AND @AptPurposeID NOT IN (SELECT ObjectID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 102) THEN 1
					WHEN NOT EXISTS(SELECT TOP 1 ID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 102) THEN 1
					ELSE 0 END
				)
			) = 1
			-- The appointment would be booked against n or more appointments. Rather than running queries on appointments, which is slow, we will simply
			-- say that this template -may- prevent the user from scheduling an appointment.
			OR EXISTS(SELECT TOP 1 ID FROM TemplateRuleDetailsT WHERE TemplateRuleID = TemplateRuleT.ID AND ObjectType = 3)
		)
	)
*/


#include "stdafx.h"
#include "schedulerrc.h"
#include "FirstAvailableAppt.h"
#include "FirstAvailList.h"
#include "DateTimeUtils.h"
#include "SchedulerView.h"
#include "InternationalUtils.h"
#include "TemplateLineItemDlg.h"
#include "TemplateExceptionInfo.h"
#include "GlobalSchedUtils.h"
#include "MsgBox.h"
#include "OHIPUtils.h"
#include "AlbertaHLINKUtils.h"
#include "SharedInsuranceUtils.h" // (r.gonet 2014-11-19) - PLID 64173 - Added since we now have an insurance dropdown.
#include "CommonFFAUtils.h"
#include "NxSchedulerDlg.h"
#include "SharedScheduleUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//(s.dhole 5/12/2015 4:20 PM ) - PLID 65620  Define constant
#define FFA_DEFAULT_ANY_LOCATION -100
#define FFA_DEFAULT_CURRENT_LOGEDIN_LOCATION -101
#define FFA_DEFAULT_CURRENT_SCHEDULER_LOCATION -102

//(s.dhole 5/13/2015 3:07 PM ) - 65621 default resource ID 
#define FFA_DEFAULT_ANY_RESOURCE -200
#define FFA_DEFAULT_CURRENT_SCHEDULER_RESOURCE -201
#define FFA_MULTIPLE_SCHEDULER_RESOURCE -202
#define FFA_USE_PATIENT_GENEREAL1_PROVIDER_AS_RESOURCE -204

using namespace ADODB;
using namespace NXDATALIST2Lib;


//(s.dhole 5/12/2015 3:47 PM ) - PLID 65620  default location combo enum 
enum DefaultLocationColumns
{
	fdlcID = 0,
	fdlcName = 1,
	FdlcOrder = 2,
	
};

//(s.dhole 5/13/2015 3:07 PM ) - 65621 default resource ID 
enum DefaultResourceColumns
{
	fdrcID = 0,
	fdrcItem = 1,
	FdrcOrder = 2,
};

// (r.farnworth 2015-05-13 11:28) - PLID 65625 - Add new dropdown for Appointment Location showing only general locations.
enum LocationComboColumns {
	lccID = 0,
	lccName,
};

// (v.maida 2016-01-28 10:22) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
enum EDurationHoursColumn
{
	dhcHours = 0,
};

enum EDurationMinsColumn
{
	dmcMinutes = 0,
};

//(s.dhole 5/18/2015 3:29 PM ) - PLID 65627
// (v.maida 2016-02-17 14:49) - PLID 68237 - Added ahcRowForeColor
enum eFFAApptHistory
{
	ahcApptID,
	ahcDate,
	ahcStartTime,
	ahcEndTime,
	ahcResource,
	ahcResourceIds,
	ahcAptTypeID,
	ahcType,
	ahcPurposeID,
	ahcPurpose,
	ahcLocID,
	ahcLocation,
	ahcPatientID,
	ahcUserDefinedID,
	ahcInsurancePriId,
	ahcInsuCoId,
	ahcOrd,
	ahcRowBkColor,
	ahcRowForeColor,
};

//(s.dhole 5/20/2015 11:41 AM ) - PLID 66144
enum eFFAApptType
{
	atcID,
	atcTypeName,
	atcDefaultArrivalMins,
	atcCategory,
};

//(s.dhole 5/20/2015 11:41 AM ) - PLID 66144
enum eFFAApptPurpose
{
	apcID,
	apcPurpose,
	apcArrivalPrepMinutes,
	
};
// This function assists with the FFA logic where we need to find
// all the slots that a start time, together with an end time, occupy.
int GetSlotFromTime(const COleDateTime& dt, bool bIsEndTime)
{
	return GetSlotFromTime(dt.GetHour(), dt.GetMinute(), dt.GetSecond(), bIsEndTime);
}

int GetSlotFromTime(long nHour, long nMinute, long nSeconds, bool bIsEndTime)
{
	int nSlot = nHour * FFA_SLOTS_PER_HOUR + nMinute * FFA_SLOTS_PER_MINUTE +
		nSeconds / FFA_SECONDS_PER_SLOT;

	if (bIsEndTime)
		nSlot--;
	return nSlot;
}

int GetLengthInSlots(long nHours, long nMinutes, long nSeconds)
{
	long nLength = nHours * FFA_SLOTS_PER_HOUR + nMinutes * FFA_SLOTS_PER_MINUTE +
		nSeconds / FFA_SECONDS_PER_SLOT;
	return nLength;
}

COleDateTime GetTimeFromSlot(int nSlot)
{
	COleDateTime dt;
	long nHour = 0, nMinute = 0, nSecond = 0;
	while (nSlot >= FFA_SLOTS_PER_HOUR)
	{
		nHour++;
		nSlot -= FFA_SLOTS_PER_HOUR;
	}
	nMinute = (FFA_SECONDS_PER_SLOT * nSlot) / 60;
	nSecond = (FFA_SECONDS_PER_SLOT * nSlot) % 60;
	dt.SetTime(nHour,nMinute,nSecond);
	return dt;
}

// (j.gruber 2010-06-07 13:26) - PLID 38854 - changed dwPurposeID from DWORD to long
BOOL EstimateApptDuration(long nPurposeID, COleDateTimeSpan& dtsResult)
{
	try {

		COleDateTime dt;
		int nHours, nMinutes;
		_RecordsetPtr rs = CreateRecordset("SELECT AVG(CONVERT(float, EndTime, 1) - CONVERT(float, StartTime, 1)) AS AvgTime FROM AppointmentsT WHERE ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID = %d)", nPurposeID);
		if (rs->eof) {
			return FALSE;
		}

		dt = rs->Fields->Item["AvgTime"]->Value.dblVal;
		nHours = dt.GetHour();
		nMinutes = dt.GetMinute();

		// Round the minutes
		if (nMinutes <= 7) nMinutes = 5;
		else if (nMinutes <= 7 + 5) nMinutes = 10;
		else if (nMinutes <= 7 + 10) nMinutes = 15;
		else if (nMinutes <= 7 + 15) nMinutes = 20;
		else if (nMinutes <= 7 + 20) nMinutes = 25;
		else if (nMinutes <= 7 + 25) nMinutes = 30;
		else if (nMinutes <= 7 + 30) nMinutes = 35;
		else if (nMinutes <= 7 + 35) nMinutes = 40;
		else if (nMinutes <= 7 + 40) nMinutes = 45;
		else if (nMinutes <= 7 + 45) nMinutes = 50;
		else if (nMinutes <= 7 + 50) nMinutes = 55;
		else {
			nHours++; nMinutes = 0;
		}

		dtsResult.SetDateTimeSpan(0, dt.GetHour(), dt.GetMinute(), 0);
		return TRUE;

	} NxCatchAll("Error in EstimateApptDuration()");
	return FALSE;
}

// (j.politis 2015-06-02 12:24) - PLID 65647 - Make the time anchors in FFA use ConfigRT instead of being hard-coded.
//	Moved it here because it's not dependant on any member variables and functions.  Plus, we only call it from GetHourText.
CString IntToLocale(int nHour)
{
	CString str1;
	COleDateTime dt;

	// Convert them to foreign times
	dt.ParseDateTime(FormatString("%li:00", nHour));
	return FormatDateTimeForInterface(dt, "%X");
}

// (j.politis 2015-06-02 12:24) - PLID 65647 - Make the time anchors in FFA use ConfigRT instead of being hard-coded.
CString GetHourSpanText(int nHour)
{
	//If nHour is valid, put it into the same string format that is used by the listbox.
	if (nHour >= 0 && nHour <= 23)
	{
		return IntToLocale(nHour) + " - " + IntToLocale(nHour == 23 ? 0 : nHour + 1);
	}
	else
	{
		//If not valid, just return a blank string.
		return "";
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFirstAvailableAppt dialog


CFirstAvailableAppt::CFirstAvailableAppt(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFirstAvailableAppt::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFirstAvailableAppt)
	m_boStartFromRadio = -1;
	m_strDaysAfter = _T("7");
	//m_boTextResults = TRUE;PLID 65643 Remove
	m_iSearchInterval = 0;
	m_iExtraBookingLimit = 0;
	m_dtStartDate = COleDateTime::GetCurrentTime();
	m_nAptLength = 0;
	m_boOfficeHoursOnly = FALSE;
	m_nPatientID = -1;
	//}}AFX_DATA_INIT
	// (c.haag 2007-02-20 12:09) - PLID 24567 - Used for default durations calculations
	m_bDurationGroupsLoaded = FALSE;
	//(e.lally 2011-05-16) PLID 41013 - Variables for preselecting values on a revised search 
	m_nPreselectPatientID = -1;
	m_nPreselectApptTypeID = -1;
	m_nPreselectApptPurposeID = -1;
	// (r.gonet 2014-11-19) - PLID 64173 - Initialize the previous selected insured party to nothing.
	m_nPreselectInsuredPartyID = -1;
	m_nPreselectDurHours = 0;
	m_nPreselectDurMins = 0;
	m_nPreselectIntervalMins = 0;
	m_bPreselectAnyTimeChecked = FALSE;
	m_bPreselectAnyResource = FALSE; //(s.dhole 5/15/2015 9:43 AM ) - PLID 65624  Default value
	m_bPreselectOfficeHoursOnly = FALSE;
	m_dtPreselectStart = COleDateTime::GetCurrentTime();
	m_bPreselectStartDateRadio = FALSE;
	m_bPreselectUserEditedDuration = FALSE;
	//(a.wilson 2012-3-27) PLID 49245 - initially set all values to false.
	m_bDefaultOverrides.SetAllTo(false);
	//TES 12/3/2014 - PLID 64180
	m_bRunImmediately = false;
	//TES 12/18/2014 - PLID 64466
	m_bReturnSlot = false;
	m_bReSchedule = false;
	// (r.farnworth 2015-05-14 07:22) - PLID 65625
	m_nLastLocationID = -1;
	// (v.maida 2016-03-23 16:15) - PLID 65623
	m_nLastResourceID = -1;
	//(s.dhole 5/27/2015 3:04 PM ) - PLID 65624
	m_bSelectAnyResource = FALSE;
}

CFirstAvailableAppt::~CFirstAvailableAppt()
{
	// (r.gonet 2014-12-17) - PLID 64428 - Removed the clearing of maps, which was associated with the old FFA implementation.

	// (c.haag 2007-02-20 12:14) - PLID 24567 - Delete any loaded duration groups
	InvalidateDurationGroups();
}

void CFirstAvailableAppt::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFirstAvailableAppt)
	DDX_Control(pDX, IDC_LIST_TIME_PREFS, m_TimePrefs);
	DDX_Radio(pDX, IDC_RADIO_START_FROM, m_boStartFromRadio);
	DDX_Control(pDX, IDC_DATE_START, m_dtDateCtrl);
	DDX_Text(pDX, IDC_EDIT_DAYS_AFTER, m_strDaysAfter);
	//DDX_Check(pDX, IDC_CHECK_RESULTS, m_boTextResults); //(s.dhole 6/1/2015 4:34 PM ) - PLID 65643 Remove functionality
	DDX_Text(pDX, IDC_EDIT_SEARCH_INTERVAL, m_iSearchInterval);
	DDX_Check(pDX, IDC_CHECK_OFFICE_HOURS_ONLY, m_boOfficeHoursOnly);
	DDX_Control(pDX, IDC_EDIT_DAYS_AFTER, m_nxeditEditDaysAfter);
	DDX_Control(pDX, IDC_EDIT_SEARCH_INTERVAL, m_nxeditEditSearchInterval);
	DDX_Control(pDX, IDC_EDIT_SEARCH_DAYS, m_nxeditEditSearchDays);
	DDX_Control(pDX, IDC_STATIC_DAYSAFTER, m_nxstaticDaysafter);
	DDX_Control(pDX, IDC_STATIC_SHOW_IN, m_nxstaticShowIn);
	DDX_Control(pDX, IDC_STATIC_SEARCH_UP_TO, m_nxstaticSearchUpTo);
	DDX_Control(pDX, IDC_STATIC_DAYS_IN_FUTURE, m_nxstaticDaysInFuture);
	DDX_Control(pDX, IDC_STATIC_MININTERVALS, m_nxstaticMinintervals);
	DDX_Control(pDX, IDC_STATIC_NODAYSAFTER, m_nxstaticNodaysafter);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_DAY_GROUPBOX, m_btnDayGroupbox);
	DDX_Control(pDX, IDC_TIME_GROUPBOX, m_btnTimeGroupbox);
	DDX_Control(pDX, IDC_ADV_SETTINGS_GROUPBOX, m_btnAdvSettingsGroupbox);
	DDX_Control(pDX, IDC_CRITERIA_GROUPBOX, m_btnCriteriaGroupbox);
	DDX_Control(pDX, ID_APP_HISTORY_BTN, m_btnFFAApptHist);
	
	//(s.dhole 6/2/2015 9:04 AM ) - PLID 65643 Remove functionality
	//DDX_Control(pDX, IDC_CHECK_RESULTS, m_checkResults);
	DDX_Control(pDX, IDC_RADIO_START_FROM, m_radioStartFrom);
	DDX_Control(pDX, IDC_RADIO_STARTPAT, m_radioStartPat);
	DDX_Control(pDX, IDC_CHECK_EXCLUDE_WARNINGS, m_checkExcludeWarnings);
	DDX_Control(pDX, IDC_CHECK_MON, m_checkMon);
	DDX_Control(pDX, IDC_CHECK_TUE, m_checkTue);
	DDX_Control(pDX, IDC_CHECK_WED, m_checkWed);
	DDX_Control(pDX, IDC_CHECK_THU, m_checkThu);
	DDX_Control(pDX, IDC_CHECK_FRI, m_checkFri);
	DDX_Control(pDX, IDC_CHECK_SAT, m_checkSat);
	DDX_Control(pDX, IDC_CHECK_SUN, m_checkSun);
	DDX_Control(pDX, IDC_CHECK_ANY_TIME, m_checkAnyTime);
	DDX_Control(pDX, IDC_CHECK_EARLY_MORN, m_checkEarlyMorn);
	DDX_Control(pDX, IDC_CHECK_LATE_MORN, m_checkLateMorn);
	DDX_Control(pDX, IDC_CHECK_EARLY_AFTN, m_checkEarlyAftn);
	DDX_Control(pDX, IDC_CHECK_LATE_AFTN, m_checkLateAftn);
	DDX_Control(pDX, IDC_CHECK_OFFICE_HOURS_ONLY, m_checkOfficeHoursOnly);
	DDX_Control(pDX, IDC_START_DATE_GROUPBOX, m_btnDayGroupbox);
	DDX_Control(pDX, IDC_MULTI_LOCATION, m_nxstaticMultiLocationLabel);
	DDX_Control(pDX, ID_UNSELECTED_RESOURCES_STATIC, m_nxlUnselectedResourceLabel);
	DDX_Control(pDX, ID_ALL_NONE_RESOURCES_STATIC, m_nxlIsResourcesUseAndFilterLabel);
	DDX_Control(pDX, ID_ALL_ANY_OF_STATIC, m_nxlIsResourcesOFLabel);
	
	
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFirstAvailableAppt, CNxDialog)
	//{{AFX_MSG_MAP(CFirstAvailableAppt)
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_CHECK_EARLY_MORN, OnCheckEarlyMorn)
	ON_BN_CLICKED(IDC_CHECK_LATE_MORN, OnCheckLateMorn)
	ON_BN_CLICKED(IDC_CHECK_EARLY_AFTN, OnCheckEarlyAftn)
	ON_BN_CLICKED(IDC_CHECK_LATE_AFTN, OnCheckLateAftn)
	ON_BN_CLICKED(IDC_CHECK_OFFICE_HOURS_ONLY, OnCheckOfficeHoursOnly)
	//(s.dhole 6/2/2015 9:04 AM ) - PLID 65643 Remove functionality
	//ON_BN_CLICKED(IDC_CHECK_RESULTS, OnCheckResults)
	ON_LBN_SELCHANGE(IDC_LIST_TIME_PREFS, OnSelchangeListTimePrefs)
	ON_BN_CLICKED(IDC_CHECK_ANY_TIME, OnCheckAnyTime)
	ON_EN_KILLFOCUS(IDC_EDIT_SEARCH_INTERVAL, OnKillFocusSearchInterval)
	ON_BN_CLICKED(IDC_BTN_HELP, OnHelp)
	ON_BN_CLICKED(IDC_BTN_SHOWEXINFO, OnShowExInfo)
	
	ON_MESSAGE(NXM_AUTO_RUN_FFA, OnAutoRunFfa)
	ON_MESSAGE(NXM_ON_CLOSE_FIRST_AVAIL_LIST, OnCloseFirstAvailList)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(ID_APP_HISTORY_BTN, OnShowAppHistory)
	

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFirstAvailableAppt message handlers

int CFirstAvailableAppt::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

// (a.wilson 2014-01-16 10:31) - PLID 15410 - repurposed this to get the earliest open and latest close since it was dead code.
// (v.maida 2016-02-05 09:53) - PLID 68132 - Added bUseAllLocations, if this flag is set then the earliest opening / maximum closing time out of 
// all locations, managed and unmanaged will be returned.
void CFirstAvailableAppt::GetOfficeHours(long& nStart, long& nEnd, BOOL bUseAllLocations)
{
	//(s.dhole 6/1/2015 4:29 PM ) - PLID 65645 added code to load login location time range
	long nMinOpen = 23, nMaxClose = 0;
	long nLocationID = GetCurrentLocationID();
	if (nLocationID < 1)
	{
		ASSERT(FALSE);
		// this shouild not happen , somthing is wrong , since we do not have login location
		return;
	}

	// (v.maida 2016-02-05 09:53) - PLID 68132 - Declare and set a SQL variable that corresponds to the bUseAllLocations flag. If the flag is set, return
	// results for all locations, managed and unmanaged, rather than just the current one.
	_RecordsetPtr prs = CreateParamRecordset(
		"DECLARE @UseAllLocations BIT; \r\n"
		"SET @UseAllLocations = {BIT}; \r\n"
		"SELECT SundayOpen, SundayClose, "
		"MondayOpen, MondayClose, TuesdayOpen, TuesdayClose, WednesdayOpen, WednesdayClose, "
		"ThursdayOpen, ThursdayClose, FridayOpen, FridayClose, SaturdayOpen, SaturdayClose "
		"FROM LocationsT WHERE (@UseAllLocations = 0 AND ID = {INT}) OR (@UseAllLocations = 1 AND ID IN (SELECT ID FROM LocationsT WHERE TypeID = 1 and Active = 1))"
		, bUseAllLocations, nLocationID);
	if (!prs->eof)
	{
		FieldsPtr pflds = prs->GetFields();
		// (v.maida 2016-02-05 09:53) - PLID 68132 - Added a while loop since it's now possible that results for multiple locations have been returned
		while (!prs->eof)
		{
			for (int i = 0; i < 7; i++)
			{

				COleDateTime dtOpen, dtClose;

				// Get the name of the day
				CString strDayName = GetDayText(i, TRUE);
				// Get the values into variants
				_variant_t varDayOpen = pflds->GetItem((LPCTSTR)(strDayName + _T("Open")))->GetValue();
				_variant_t varDayClose = pflds->GetItem((LPCTSTR)(strDayName + _T("Close")))->GetValue();
				// Handle the variants
				if (varDayOpen.vt != VT_NULL && varDayClose.vt != VT_NULL) {
					// Both non-null so store them in variables
					dtOpen = VarDateTime(varDayOpen);
					dtClose = VarDateTime(varDayClose);
					dtOpen.SetTime(dtOpen.GetHour(), dtOpen.GetMinute(), dtOpen.GetSecond());
					dtClose.SetTime(dtClose.GetHour(), dtClose.GetMinute(), dtClose.GetSecond());

				}
				else {
					// One of the dates was null, so this day has invalid office hours
					dtOpen = COleDateTime(0, 0, 0, 0, 0, 0);
					dtClose = COleDateTime(0, 0, 0, 0, 0, 0);

				}
				if ((dtOpen.m_status == COleDateTime::valid && dtOpen.GetHour() < nMinOpen))
				{
					nMinOpen = dtOpen.GetHour();
				}
				if (dtClose.m_status == COleDateTime::valid && dtClose.GetHour() > nMaxClose)
				{
					nMaxClose = (dtClose.GetMinute() != 0 ? dtClose.GetHour() + 1 : dtClose.GetHour());
				}
			}
			prs->MoveNext();
		}
	}

	if (nMinOpen == 23 && nMaxClose == 0) {
		// (v.maida 2016-02-05 12:07) - PLID 68132 - If we get here, then either no query results were returned, or all NULL records were returned,
		// since the Admin module->Locations tab doesn't actually allow an opening time of 23 (11:00 PM) and closing time of 0 (12:00 AM) together.
		// This used to change nMinOpen to 7 (7:00 AM) and nMaxClose to 19 (7:00 PM), but we now allow midnight to midnight times, so I've changed it
		// to 0 to 23 (12:00 AM - 1:00 AM span to 11:00 PM - 12:00 AM).
		nMinOpen = 0;
		nMaxClose = 23;
	}

	nStart = nMinOpen;
	nEnd = nMaxClose;
}

void CFirstAvailableAppt::OnCheckEarlyMorn() 
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_EARLY_MORN);
	// (b.savon 2016-03-15 15:22) - PLID 65647 -- Pass in defaults and use preference
	OnCheckAnchor(pButton->GetCheck(), "FFA_DefaultEarlyMorningFromID", "FFA_DefaultEarlyMorningToID", FFA_AnchorDefault::EarlyMorningFrom, FFA_AnchorDefault::EarlyMorningTo);
}

void CFirstAvailableAppt::OnCheckLateMorn() 
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_LATE_MORN);
	// (b.savon 2016-03-15 15:22) - PLID 65647 -- Pass in defaults and use preference
	OnCheckAnchor(pButton->GetCheck(), "FFA_DefaultLateMorningFromID", "FFA_DefaultLateMorningToID", FFA_AnchorDefault::LateMorningFrom, FFA_AnchorDefault::LateMorningTo);
}

void CFirstAvailableAppt::OnCheckEarlyAftn() 
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_EARLY_AFTN);
	// (b.savon 2016-03-15 15:22) - PLID 65647 -- Pass in defaults and use preference
	OnCheckAnchor(pButton->GetCheck(), "FFA_DefaultEarlyAfternoonFromID", "FFA_DefaultEarlyAfternoonToID", FFA_AnchorDefault::EarlyAfternoonFrom, FFA_AnchorDefault::EarlyAfternoonTo);
}

void CFirstAvailableAppt::OnCheckLateAftn() 
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_LATE_AFTN);
	// (b.savon 2016-03-15 15:22) - PLID 65647 -- Pass in defaults and use preference
	OnCheckAnchor(pButton->GetCheck(), "FFA_DefaultLateAfternoonFromID", "FFA_DefaultLateAfternoonToID", FFA_AnchorDefault::LateAfternoonFrom, FFA_AnchorDefault::LateAfternoonTo);
}

// (b.savon 2016-03-15 15:22) - PLID 65647 -- Pass in defaults and use preference
void CFirstAvailableAppt::OnCheckAnchor(int nChecked, const CString &strFromProperty, const CString &strToProperty, byte defaultFrom, byte defaultTo)
{
	try{
		//BOOL is used because listbox uses BOOL for SetSel
		BOOL bToSelect = nChecked == 1 ? TRUE : FALSE;
		// (b.savon 2016-03-15 15:22) - PLID 65647 -- Use the default
		int nFrom = GetRemotePropertyInt(strFromProperty, defaultFrom, 0, "<None>", true);
		int nTo = GetRemotePropertyInt(strToProperty, defaultTo, 0, "<None>", true);

		for (; nFrom < nTo; nFrom++)
		{
			SetHourSelected(nFrom, bToSelect);
		}

		// (a.wilson 2014-01-16 10:31) - PLID 15410 - add update
		UpdateTimePrefCheckboxes();

	}NxCatchAll(__FUNCTION__);
}

// (j.politis 2015-06-02 12:24) - PLID 65647 - Make the time anchors in FFA use ConfigRT instead of being hard-coded.
void CFirstAvailableAppt::SetHourSelected(int nHour, BOOL bToSelect)
{
	int nIndex = GetHourIndex(nHour);
	
	//If we allow -1 to be selected, then all rows end up being selected.
	if (nIndex >= 0)
	{
		m_TimePrefs.SetSel(nIndex, bToSelect);
	}
	else
	{
		//Nothing to do here.  When we receive a request to set a time span that doesn't exists(yet) in the
		//listbox, then gethourindex will return -1.  We DO NOT WANT TO SET IT.
	}
}

// (j.politis 2015-06-02 12:24) - PLID 65647 - Make the time anchors in FFA use ConfigRT instead of being hard-coded.
int CFirstAvailableAppt::GetHourSelected(int nHour)
{
	return m_TimePrefs.GetSel(GetHourIndex(nHour));
}

// (j.politis 2015-06-02 12:24) - PLID 65647 - Make the time anchors in FFA use ConfigRT instead of being hard-coded.
int CFirstAvailableAppt::GetHourIndex(int nHour)
{
	return m_TimePrefs.FindString(0, GetHourSpanText(nHour));
}

// (a.wilson 2014-01-16 10:31) - PLID 15410 - try to check off selections for every hour in the day.
void CFirstAvailableAppt::OnCheckAnyTime() 
{
	try{
		BOOL bSelect = FALSE;
		CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_ANY_TIME);
		bSelect = pButton->GetCheck();

		for (int i = 0; i < 24; i++) {
			m_TimePrefs.SetSel(i, bSelect);
		}
		UpdateTimePrefCheckboxes();
	}NxCatchAll(__FUNCTION__);
}

void CFirstAvailableAppt::OnCheckOfficeHoursOnly() 
{
	UpdateData(TRUE);

	// (j.politis 2015-06-22 16:09) - PLID 66414 - Changed checking the “Only Include Office Hours” checkbox to no longer disable the time preference UI elements.
}

//(s.dhole 6/2/2015 9:04 AM ) - PLID 65643 Remove functionality
//void CFirstAvailableAppt::OnCheckResults()
//{
//	BOOL bEnable = ((CButton*)GetDlgItem(IDC_CHECK_RESULTS))->GetCheck();
//	GetDlgItem(IDC_EDIT_SEARCH_INTERVAL)->EnableWindow(bEnable);
//	GetDlgItem(IDC_EDIT_SEARCH_DAYS)->EnableWindow(bEnable);
//	GetDlgItem(IDC_STATIC_SHOW_IN)->EnableWindow(bEnable);
//	GetDlgItem(IDC_STATIC_MININTERVALS)->EnableWindow(bEnable);
//	GetDlgItem(IDC_STATIC_SEARCH_UP_TO)->EnableWindow(bEnable);
//	GetDlgItem(IDC_STATIC_DAYS_IN_FUTURE)->EnableWindow(bEnable);
//}

void CFirstAvailableAppt::UpdateTimePrefCheckboxes()
{
	// (j.politis 2015-06-02 12:24) - PLID 65647 - Make the time anchors in FFA use ConfigRT instead of being hard-coded.
	//Set anchor time check boxes
	{
		bool isEarlyMorningSet = IsAnchorSpanSet(ETimeAnchors::EarlyMorning);
		bool isLateMorningSet = IsAnchorSpanSet(ETimeAnchors::LateMorning);
		bool isEarlyAfternoonSet = IsAnchorSpanSet(ETimeAnchors::EarlyAfternoon);
		bool isLateAfternoonSet = IsAnchorSpanSet(ETimeAnchors::LateAfternoon);

		((CButton*)GetDlgItem(IDC_CHECK_EARLY_MORN))->SetCheck(isEarlyMorningSet ? 1 : 0);
		((CButton*)GetDlgItem(IDC_CHECK_LATE_MORN))->SetCheck(isLateMorningSet ? 1 : 0);
		((CButton*)GetDlgItem(IDC_CHECK_EARLY_AFTN))->SetCheck(isEarlyAfternoonSet ? 1 : 0);
		((CButton*)GetDlgItem(IDC_CHECK_LATE_AFTN))->SetCheck(isLateAfternoonSet ? 1 : 0);
	}

	//Set any time check box
	{
		for (int i = 0; i < m_TimePrefs.GetCount(); i++) {
			if (!m_TimePrefs.GetSel(i)) break;
		}
		if (i == m_TimePrefs.GetCount())
			((CButton*)GetDlgItem(IDC_CHECK_ANY_TIME))->SetCheck(1);
		else
			((CButton*)GetDlgItem(IDC_CHECK_ANY_TIME))->SetCheck(0);
	}
}

// (j.politis 2015-06-02 12:24) - PLID 65647 - Make the time anchors in FFA use ConfigRT instead of being hard-coded.
bool CFirstAvailableAppt::IsTimeSpanSet(int nFrom, int nTo)
{
	for (; nFrom < nTo; nFrom++)
	{
		if (!GetHourSelected(nFrom))
		{
			return false;
		}
	}
	return true;
}

// (j.politis 2015-06-02 12:24) - PLID 65647 - Make the time anchors in FFA use ConfigRT instead of being hard-coded.
bool CFirstAvailableAppt::IsAnchorSpanSet(ETimeAnchors::_Enum eAnchor)
{
	int nFrom, nTo;

	switch (eAnchor)
	{
	case ETimeAnchors::EarlyMorning:
	{
		nFrom = GetRemotePropertyInt("FFA_DefaultEarlyMorningFromID", 6, 0, "<None>", true);
		nTo = GetRemotePropertyInt("FFA_DefaultEarlyMorningToID", 9, 0, "<None>", true);
		break;
	}
	case ETimeAnchors::LateMorning:
	{
		nFrom = GetRemotePropertyInt("FFA_DefaultLateMorningFromID", 9, 0, "<None>", true);
		nTo = GetRemotePropertyInt("FFA_DefaultLateMorningToID", 12, 0, "<None>", true);
		break;
	}
	case ETimeAnchors::EarlyAfternoon:
	{
		nFrom = GetRemotePropertyInt("FFA_DefaultEarlyAfternoonFromID", 12, 0, "<None>", true);
		nTo = GetRemotePropertyInt("FFA_DefaultEarlyAfternoonToID", 14, 0, "<None>", true);
		break;
	}
	case ETimeAnchors::LateAfternoon:
	{
		nFrom = GetRemotePropertyInt("FFA_DefaultLateAfternoonFromID", 14, 0, "<None>", true);
		nTo = GetRemotePropertyInt("FFA_DefaultLateAfternoonToID", 18, 0, "<None>", true);
		break;
	}
	default:
	{
		//We should never reach here.  If we do, it's because the developer created a new enum
		//and never inserted it here as well.
		ASSERT(FALSE);
		return false;
	}
	}
	
	return nFrom < nTo ? IsTimeSpanSet(nFrom, nTo) : false;
}

void CFirstAvailableAppt::OnSelchangeListTimePrefs() 
{
	try
	{
		UpdateTimePrefCheckboxes();
	}
	NxCatchAll(__FUNCTION__);
}

// (r.gonet 2014-12-17) - PLID 64428 - Renamed from LookupResourceEx to CreateResourceMaps since the non-ex function no longer exists and this name is more descriptive
// of what we actually do.
// (r.gonet 2014-12-17) - PLID 64428 - Removed the strSQL argument. The function now knows what SQL to run.
// (r.gonet 2014-12-17) - PLID 64428 - All old arguments were part of the new CFFASearchSettings object, so I just passed that instead.
// (r.gonet 2014-12-17) - PLID 64428 - Refactored to remove any references to controls since we now call this when the class does not have a window.
void CFirstAvailableAppt::CreateResourceMaps(CFFASearchSettingsPtr pSettings)
{
	if(pSettings == NULL) {
		ThrowNxException("%s : pSettings is NULL.", __FUNCTION__);
	}
	if (pSettings->m_dtStartDate.GetStatus() != COleDateTime::valid) {
		ThrowNxException("%s : Invalid start date in pSettings.", __FUNCTION__);
	}

	// (r.gonet 2014-12-17) - PLID 64428 - Instead of taking the SQL string as an argument to this function, we now just run it here.
	// Reason being was that this function is now called in multiple places, which would have necessitated creation of a function to get
	// the SQL to pass in but also because this function was not being called anywhere else with any different SQL. So why pass it in then?
	// (r.gonet 2014-12-17) - PLID 64428 - Parameterized.
	CParamSqlBatch sqlBatch;
	sqlBatch.Add("SELECT [Date], StartTime, EndTime, ResourceT.Item FROM AppointmentsT LEFT JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID LEFT JOIN ResourceT ON AppointmentResourceT.ResourceID = ResourceT.ID "
		"WHERE ([Date] >= DATEADD(dd, 0, DATEDIFF(dd, 0, {OLEDATETIME}))) AND AppointmentResourceT.ResourceID IN ({INTARRAY}) AND AppointmentsT.Status <> 4 ORDER BY [Date]",
		pSettings->m_dtStartDate, pSettings->m_arynResourceIDs);

#ifdef _DEBUG
	TRACE("FFA Search SQL: ");
	//DRT 4/13/2004 - TRACE is limited to 512 chars or it gives an assertion
	TRACE(sqlBatch.Flatten().Left(512));
	TRACE("\n");
#endif

	try {
		_RecordsetPtr rs = sqlBatch.CreateRecordset(GetRemoteDataSnapshot());
		_variant_t var;

		// (r.gonet 2014-12-17) - PLID 64428 - Make sure the maps are empty before we set them. Free up any allocated memory.
		pSettings->ClearResourceMaps();

		CMapDateTimeToTimeList* pTimeListMap;
		long* pTimeList;

		/* Make sure every resource is included in the map */
		for (int i=0; i < pSettings->m_arynResourceIDs.GetSize(); i++) {
			CString strResourceName = pSettings->m_aryResourceNames[i];
			pSettings->m_mapResToDate[strResourceName] = pTimeListMap = new CMapDateTimeToTimeList;
			pSettings->m_mapResItemToID[strResourceName] = pSettings->m_arynResourceIDs[i];
		}

		while (!rs->eof) {			
			COleDateTime dtDate;
			/* Go to the next entry which meets the day-of-week	criteria */
			int weekday = 0;
			while (!rs->eof) {

				/* Make sure this resource exists in the resource=>date/time list map */
				//var = rs->Fields->Item["Item"]->Value;
				//if (!mapResToDate.Lookup(CString(var.bstrVal), pTimeListMap))
				//{
					//mapResToDate[CString(var.bstrVal)] = pTimeListMap = new CMapDateTimeToTimeList;
				//}
				//if (!mapResToID.Lookup(CString(var.bstrVal), dwResID))
				//{
				//	mapResToID[CString(var.bstrVal)] = rs->Fields->Item["ResrcID"]->Value.lVal;
				//}

				var = rs->Fields->Item["Date"]->Value;
				dtDate = var.date;
				weekday = dtDate.GetDayOfWeek();
				if (m_aResWeekdays[ weekday-1 ] == 0)
					rs->MoveNext();
				else break;
			}
			if (rs->eof) return;

			/* */

			var = rs->Fields->Item["Item"]->Value;			
			pSettings->m_mapResToDate.Lookup(CString(var.bstrVal), pTimeListMap);

			var = rs->Fields->Item["Date"]->Value;
			if (!pTimeListMap->Lookup(COleDateTime(var.date), pTimeList))
			{
				pTimeList = new TimeList;
				memset(pTimeList, 0, sizeof(TimeList));
				pTimeListMap->SetAt(COleDateTime(var.date), pTimeList);

				/* Fill array of available times with hilited listbox time interval entries
				(these is the user-defined times of availability, defined by hilite). */
				// (r.gonet 2014-12-17) - PLID 64428 - Changed to a literal 24. The count is always 24. Changed because there was no way of getting the count now.
				// (j.politis 2015-06-25 11:26) - PLID 65642 - Extend the time preferences listbox to go from midnight to midnight.
				// Comment for (r.gonet 2014-12-17) - PLID 64428 mentions a quirk.  Fixxed it.  1/0 represents hour to be used.  index i used for actual hour value
				for (int i = 0; i < 24; i++) {
					int hour_selected = pSettings->m_anTimePrefList[i];
					// (r.gonet 2014-12-17) - PLID 64428 - Previously, we would look at the list box and see what times were selected. Now though, we look to the time pref array in settings,
					// which includes the same information but without getting data from a control. hour_data will be 0 if that time is excluded. Interestingly, I don't think midnight ever worked
					// right because we are using a sentinel 0 value. Midnight would be 0 as well though, meaning it gets excluded from FFA. I'm keeping with that quirk.
					if (hour_selected) {
						int nStartSlot = GetSlotFromTime(i, 0, 0);
						for (int iSlot = nStartSlot; iSlot < nStartSlot + FFA_SLOTS_PER_HOUR; iSlot++) {
							pTimeList[iSlot] = 1;
						}
					}
				}
			}

			/* Go through all the taken up times for this given day according
			to the recordset. Mark them all to be FALSE */
			COleDateTime dtStart;
			COleDateTime dtEnd;
			var = rs->Fields->Item["StartTime"]->Value;
			dtStart = var.date;
			var = rs->Fields->Item["EndTime"]->Value;
			dtEnd = var.date;

			// Decrement the slot values in the time list for this
			// duration. In english, we are telling FFA "There is one
			// more appointment taking up these times, so there is one
			// less booking available on these times"
			int iStart = GetSlotFromTime(dtStart);
			int iEnd = GetSlotFromTime(dtEnd, true);

			for (int i=iStart; i <= iEnd; i++) {
				if (pTimeList[i] > 0)
					pTimeList[i]--;
			}
			rs->MoveNext();
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2014-12-17) - PLID 64428 - Removed old implementation of FFA, now dead code, in effort to use the API implementation instead.
void CFirstAvailableAppt::OnOK()
{
	//TES 1/7/2015 - PLID 64466 - Moved code to a shared function
	LaunchFirstAvailList(m_bReturnSlot);

	//TES 1/7/2015 - PLID 64513 - Don't close if we're returning a slot
	//(s.dhole 6/2/2015 8:54 AM ) - PLID 65643 remove functionality
	if (!m_bReturnSlot) {
		CNxDialog::OnOK();
	}
	
}

void CFirstAvailableAppt::OnCancel()
{
	//TES 1/7/2015 - PLID 64466 - Make sure we're not returning a slot in future calls.
	m_bReturnSlot = false;
	m_bReSchedule = false;
	CNxDialog::OnCancel();
}
//(s.dhole 5/14/2015 2:50 PM ) - PLID 66125 This is existing logic, we enable go button only if  resource is selected
void CFirstAvailableAppt::SetGoStatus()
{
	try {
		
		if (m_arySelectedResources.GetCount() > 0)
		{
			((CButton*)GetDlgItem(IDOK))->EnableWindow(TRUE);
		}
		else
		{
			((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);
		}
	}NxCatchAll("Error in SetGoStatus()");
}


//(s.dhole 5/12/2015 2:48 PM ) - PLID 66144 update  IDC_PATIENT_LIST_COMBO,IDC_APTTYPE_COMBO, IDC_APTPURPOSE_COMBO  to datalist 2
// (j.politis 2015-05-18 12:35) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
BEGIN_EVENTSINK_MAP(CFirstAvailableAppt, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CFirstAvailableAppt)
	
	ON_EVENT(CFirstAvailableAppt, IDC_APTTYPE_COMBO, 16 /* SelChanged */, OnSelChosenApttypeCombo, VTS_DISPATCH)
	ON_EVENT(CFirstAvailableAppt, IDC_APTPURPOSE_COMBO, 16 /* SelChanged */, OnSelChosenAptpurposeCombo, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CFirstAvailableAppt, IDC_PATIENT_LIST_COMBO, 16, CFirstAvailableAppt::OnSelChosenPatientListCombo, VTS_DISPATCH)
	ON_EVENT(CFirstAvailableAppt, IDC_FFA_INSURED_PARTY_COMBO, 1, CNxDialog::RequireDataListSel, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CFirstAvailableAppt, IDC_FFA_DEFAULT_LOCATION_COMBO, 16, CFirstAvailableAppt::OnSelChosenDefaultLocationCombo, VTS_DISPATCH)
	ON_EVENT(CFirstAvailableAppt, IDC_FFA_DEFAULT_RESOURCE_COMBO, 16, CFirstAvailableAppt::OnSelChosenDefaultResourceCombo, VTS_DISPATCH)
	ON_EVENT(CFirstAvailableAppt, IDC_FFA_LOCATIONS_COMBO, 16, CFirstAvailableAppt::OnSelChosenLocationListCombo, VTS_DISPATCH)
	ON_EVENT(CFirstAvailableAppt, IDC_UNSELECTED_RESOURCES_COMBO, 16, CFirstAvailableAppt::OnSelChosenUnselectedResource, VTS_DISPATCH)
	ON_EVENT(CFirstAvailableAppt, IDC_UNSELECTED_RESOURCES_COMBO, 1, CFirstAvailableAppt::OnSelChangingUnselectedResource, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CFirstAvailableAppt, IDC_FFA_HOURS_COMBO, 2, CFirstAvailableAppt::OnSelChangedFfaHoursCombo, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CFirstAvailableAppt, IDC_FFA_MINS_COMBO, 2, CFirstAvailableAppt::OnSelChangedFfaMinsCombo, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CFirstAvailableAppt, IDC_FFA_APP_HIST_LIST, 16, CFirstAvailableAppt::OnSelChosenAppHistoryCombo, VTS_DISPATCH)
	ON_EVENT(CFirstAvailableAppt, IDC_FFA_APP_HIST_LIST, 30 /* ClosedUp */, OnClosedUpAppHistoryCombo, VTS_DISPATCH)
	
END_EVENTSINK_MAP()

// (r.gonet 2014-12-17) - PLID 64428 - Callers must now specify the resource ID as well.
//(s.dhole 6/2/2015 8:42 AM ) - PLID 65645 Remove functionality
//BOOL CFirstAvailableAppt::IsDayAvailable(COleDateTime date, long nResourceID, CString strResource)
//{
//	BOOL boTmp;
//	if (!m_mapResources.Lookup(strResource, boTmp))
//		return FALSE;
//
//	// (r.gonet 2014-12-16) - PLID 64428 - The non-text list mode uses the API FFA now rather than doing its own thing.
//	// (r.gonet 2014-12-16) - PLID 64428 - Ensure that the availability lookup table of the open days for resources in the current month is loaded.
//	LoadAvailabilityForMonth(date.GetYear(), date.GetMonth());
//
//	// (r.gonet 2014-12-16) - PLID 64428 - Check in the availability lookup whether the resource is available on the passed date.
//	std::pair<COleDateTime, long> key;
//	key.first = date;
//	key.second = nResourceID;
//	if (m_setAvailability.find(key) != m_setAvailability.end()) {
//		return TRUE;
//	} else {
//		// (r.gonet 2014-12-16) - PLID 64428 - They weren't in the lookup, so they aren't available on this day.
//	}
//	
//	return FALSE;
//}

//(s.dhole 6/2/2015 8:42 AM ) - PLID  65643 Remove functionality
//void CFirstAvailableAppt::RemoveAvailableDays()
//{
//	m_mapResDt.RemoveAll();		// Clear available dates list
//	//(s.dhole 6/2/2015 8:42 AM ) - PLID  65643 Remove functionality
//	//m_mapResources.RemoveAll();	// Clear used resources list
//}

//(a.wilson 2012-3-27) PLID 49245 - implementing a struct of overrides for the preselect values per
//instance where we will be not using the default values.  This will require some code to be moved around.
BOOL CFirstAvailableAppt::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		// (z.manning 2010-10-28 12:56) - PLID 41129 - bulk caching
		// (b.savon 2016-01-27 10:03) - PLID 67956 - Use global preference -- FFA_AllowBookingsAgainst, FFA_AllowBookingsAgainst_NumberAppointments
		// (v.maida 2016-03-14 17:56) - PLID 68448 - Added FFAOnlyShowLocationSpecificResults and dontshow FFAOnlyShowLocationSpecificResultsWarning
		g_propManager.CachePropertiesInBulk("CFirstAvailableAppt", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( \r\n"
			"	'PtTB_Security_Group' \r\n"
			"	, 'UseOHIP' \r\n"
			"	, 'ResEntryRequireAptType' \r\n"
			"	, 'ResEntryRequireAptPurpose' \r\n"
			"	, 'FFAConsiderOfficeHours' \r\n"
			"	, 'FFA_ExcludeAvailOnWarnings' \r\n"
			"	, 'FFA_AllowBookingsAgainst' \r\n"
			"	, 'FFA_AllowBookingsAgainst_NumberAppointments' \r\n"
			"	, 'HideEmptyTypes' \r\n"
			//"	, 'ShowFFAAdvancedSettings' \r\n" //(s.dhole 5/21/2015 12:00 PM ) - PLID 65620  we added option in dropdown
			"	, 'FFA_EstimateAverageApptLength' \r\n"
			"	, 'FFA_CalcDefaultApptLength' \r\n"
			// (j.jones 2010-11-03 15:08) - PLID 39620 - added Alberta option
			"	, 'UseAlbertaHLINK' \r\n"
			"	, 'Alberta_PatientULICustomField' \r\n"
			// (b.savon 2013-07-26 13:25) - PLID 50524 - Preference for Resource selection on FFA
			//"	, 'FFA_DefaultResourceToPatientsGen1Provider' \r\n" //(s.dhole 6/4/2015 9:10 AM ) - PLID 65643  dead code  added prefrece to default dropdown
			// (d.singleton 2014-10-13 08:33) - PLID 62634
			"   , 'NexWebPromptForLoginOnNewAppt' \r\n"
			"   , 'NexWebPromptForLoginOnNewApptThreshhold' \r\n"
			// (r.gonet 2014-12-09) - PLID 64173 - Cache the auto fill insurance preferences.
			"	, 'AutoFillApptInsurance' \r\n"
			"	, 'AutoFillApptInsurance_DefaultCategory' \r\n"
			//(s.dhole 5/12/2015 3:50 PM ) - PLID 65620
			"	, 'FFA_DefaultLocationID' \r\n"
			//(s.dhole 5/13/2015 4:23 PM ) - PLID 65621
			"	, 'FFA_DefaultResourceID' \r\n"
			"	, 'FFAOnlyShowLocationSpecificResults' \r\n"
			"   , 'dontshow FFAOnlyShowLocationSpecificResultsWarning' \r\n"
			")"
			, _Q(GetCurrentUserName()));
		
		// (c.haag 2006-11-27 11:20) - PLID 20772 - Make sure the results list
		// is empty before populating it with new information
		// (z.manning 2011-06-15 09:50) - PLID 44120 - The list dialog object is now a pointer
		CFirstAvailList *pdlgAvailList = GetMainFrame()->GetFirstAvailList();
		CWaitCursor wc;
		pdlgAvailList->Clear();

		// (c.haag 2008-04-24 14:24) - PLID 29776 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		//(s.dhole 5/22/2015 3:18 PM ) - PLID 65627
		m_btnFFAApptHist.AutoSet(NXB_FFAHISTORY);
		

		//(e.lally 2011-05-16) PLID 41013 - Use the preselected values from the previous search when it is a revised one
		//(a.wilson 2012-3-27) PLID 49245 - check preselects per override -- UserEditedDuration
		if(m_bDefaultOverrides.bUserEditedDuration){
			m_bUserEditedDuration = m_bPreselectUserEditedDuration;
		}
		else {
			//This signifies that the user made a selection from the drop down.
			//This tells us NOT to change the hours and minutes whenever appointment type is changed.
			m_bUserEditedDuration = FALSE;
		}
		m_strLastResource.Empty();
		// (c.haag 2007-02-20 12:09) - PLID 24567 - Used for default durations calculations
		m_bDurationGroupsLoaded = FALSE; 
		
		//(s.dhole 5/12/2015 2:48 PM ) - PLID 66144 update  to Datalist2 
		m_PatientCombo = BindNxDataList2Ctrl(this, IDC_PATIENT_LIST_COMBO, GetRemoteData(), false);

		if (m_bReSchedule) {
			GetDlgItem(IDC_PATIENT_LIST_COMBO)->EnableWindow(FALSE);
		}

		// (r.farnworth 2015-05-13 11:28) - PLID 65625 - Add new dropdown for Appointment Location showing only general locations.
		m_FFALocationCombo = BindNxDataList2Ctrl(this, IDC_FFA_LOCATIONS_COMBO, GetRemoteData(), true);

		//(s.dhole 5/12/2015 3:29 PM ) - PLID 65620
		m_FFADefaultLocationCombo = BindNxDataList2Ctrl(this, IDC_FFA_DEFAULT_LOCATION_COMBO, GetRemoteData(), false);
		m_FFADefaultLocationCombo->FromClause = bstr_t(FormatString( "(SELECT ID ,Name , 10 AS Ord FROM LocationsT WHERE TypeID=1 and Active = 1  "
			" UNION "
			" SELECT %li AS ID, '{ Any Location }' AS Name, 1 AS Ord "
			" UNION "
			" SELECT %li AS ID, '{ Current Logged-In Location }' AS Name, 2 AS Ord "
			" UNION "
			" SELECT %li AS ID, '{ Current Scheduler Location }' AS Name, 3 AS Ord ) AS DefaultLocationQ ", FFA_DEFAULT_ANY_LOCATION, FFA_DEFAULT_CURRENT_LOGEDIN_LOCATION, FFA_DEFAULT_CURRENT_SCHEDULER_LOCATION));
		m_FFADefaultLocationCombo->Requery();
		// (v.maida 2016-01-22 10:32) - PLID 68033 - For consistency's sake, if the default location is no longer in the location dropdown,
		// which could be either due to the location being deleted or becoming inactive, then just default the datalist selection to {Any Location},
		// which is the FFA Default Location preference default, instead of the currently logged-in location.
		long nDefaultlocation = GetRemotePropertyInt("FFA_DefaultLocationID", FFA_DEFAULT_ANY_LOCATION /*-100 {Any Location}*/, 0, GetCurrentUserName(), true);
		//If last selected locationis inactive then swch back to logged in location
		if (!m_FFADefaultLocationCombo->FindByColumn(fdlcID, nDefaultlocation, NULL, VARIANT_TRUE))
		{
			m_FFADefaultLocationCombo->FindByColumn(fdlcID, long(FFA_DEFAULT_ANY_LOCATION), NULL, VARIANT_TRUE);
		}

		//(s.dhole 5/14/2015 5:01 PM ) - PLID 65621 New default preference Patient's Gen. 1 provider
		m_FFADefaultResourceCombo = BindNxDataList2Ctrl(this, IDC_FFA_DEFAULT_RESOURCE_COMBO, GetRemoteData(), false);
		m_FFADefaultResourceCombo->FromClause = bstr_t(FormatString("(SELECT ID, Item, 10 AS Ord FROM ResourceT WHERE Inactive = 0 "
			" UNION "
			" SELECT %li AS ID, '{ Current Scheduler Resource }' AS  Item, 2 AS Ord FROM ResourceT WHERE Inactive = 0 "
			" UNION "
			" SELECT %li AS ID, '{ Any Resource }' AS  Item, 1 AS Ord FROM ResourceT WHERE Inactive = 0 "
			" UNION "
			" SELECT %li AS ID, '{ Patient''s Gen. 1 provider }' AS  Item, 3 AS Ord FROM ResourceT WHERE Inactive = 0 ) AS DefaultResourceQ ", FFA_DEFAULT_CURRENT_SCHEDULER_RESOURCE, FFA_DEFAULT_ANY_RESOURCE,  FFA_USE_PATIENT_GENEREAL1_PROVIDER_AS_RESOURCE));
		m_FFADefaultResourceCombo->Requery();
		// (v.maida 2016-03-21 08:57) - PLID 65621 - Changed default to Current Scheduler Resource, rather than Any Resource.
		long nDefaultResourceID = GetRemotePropertyInt("FFA_DefaultResourceID", FFA_DEFAULT_CURRENT_SCHEDULER_RESOURCE, 0, GetCurrentUserName());
		if (!m_FFADefaultResourceCombo->FindByColumn(fdrcID, nDefaultResourceID, NULL, VARIANT_TRUE))
		{
			// (v.maida 2016-03-21 08:57) - PLID 65621 - if last selected resource is inactive or deleted then select Current Scheduler Resource.
			m_FFADefaultResourceCombo->FindByColumn(fdrcID, (long)FFA_DEFAULT_CURRENT_SCHEDULER_RESOURCE, NULL, VARIANT_TRUE);
		}

		
		// (s.dhole 2015-05-13 17:01) - PLID 65623 - Update to datlist 2
		//(s.dhole 5/21/2015 5:03 PM ) - PLID 66125
		CString strHiddenResourceIDs = CalcExcludedResourceIDs();
		CString strSqlHiddenResourceID = "";
		if (!strHiddenResourceIDs.IsEmpty())
		{
			strSqlHiddenResourceID = FormatString(" AND ResourceT.ID NOT IN (%s) ", strHiddenResourceIDs);
		}

		m_Resources = BindNxDataList2Ctrl(this, IDC_UNSELECTED_RESOURCES_COMBO, GetRemoteData(), false);
		m_Resources->FromClause = bstr_t(FormatString("(SELECT ID, Item, 10 AS Ord FROM ResourceT WHERE Inactive = 0  %s "
			" UNION "
			" SELECT %li AS ID, '{ Multiple Resources }' AS  Item, 2 AS Ord FROM ResourceT WHERE Inactive = 0 "
			" UNION "
			" SELECT %li AS ID, '{ Any Resource }' AS  Item, 1 AS Ord FROM ResourceT WHERE Inactive = 0) AS DefaultResourceQ ", strSqlHiddenResourceID, FFA_MULTIPLE_SCHEDULER_RESOURCE, FFA_DEFAULT_ANY_RESOURCE));
		m_Resources->Requery();
		//(s.dhole 5/21/2015 11:03 AM ) - PLID 65623
		m_nxlUnselectedResourceLabel.SetType(dtsHyperlink);
		m_nxlUnselectedResourceLabel.SetSingleLine();

		//(s.dhole 5/15/2015 9:32 AM ) - PLID 65624 Intialize labele
		m_nxlIsResourcesUseAndFilterLabel.SetType(dtsHyperlink);
		// (b.cardillo 2016-01-21 15:20) - PLID 65624 (supplemental) - Make it right-justified so spacing with "of" is consistent
		m_nxlIsResourcesUseAndFilterLabel.SetHzAlign(DT_RIGHT);
		m_nxlIsResourcesUseAndFilterLabel.SetSingleLine();
		m_nxlIsResourcesOFLabel.SetSingleLine();
		m_nxlIsResourcesOFLabel.SetText("of");


		// (z.manning 2010-10-28 12:45) - PLID 41129 - Added columns to the patient combo to make it similar to
		// the res entry dialog's patient combo and also added code to show/hide the OHIP ID column.
		//(s.dhole 5/12/2015 2:48 PM ) - PLID 66144 update  to Datalist2 
		NXDATALIST2Lib::IColumnSettingsPtr pSSNCol = m_PatientCombo->GetColumn(pccSSN);
		//(s.dhole 5/12/2015 2:48 PM ) - PLID 66144 update  to Datalist2 
		NXDATALIST2Lib::IColumnSettingsPtr pHealthCardCol = m_PatientCombo->GetColumn(pccHealthCardNum);
		if(UseOHIP()) {
			pHealthCardCol->PutColumnStyle(csVisible);
			pHealthCardCol->PutStoredWidth(85);
			pSSNCol->PutColumnStyle(csVisible|csFixedWidth);
			pSSNCol->PutStoredWidth(0);
			pHealthCardCol->PutColumnTitle(_bstr_t(GetOHIPHealthTitle()));
		}
		// (j.jones 2010-11-08 13:55) - PLID 39620 - supported Alberta
		else if(UseAlbertaHLINK()) {
			pHealthCardCol->PutColumnStyle(csVisible);
			pHealthCardCol->PutStoredWidth(85);
			pSSNCol->PutColumnStyle(csVisible|csFixedWidth);
			pSSNCol->PutStoredWidth(0);
			pHealthCardCol->PutColumnTitle(_bstr_t(GetAlbertaHLINKHealthTitle()));
		}
		else {
			// (z.manning 2010-11-08 17:43) - PLID 41129 - If they don't have permission to read SSNs and disable
			// masking then hide the SSN column.
			if(!CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) || !CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
				pSSNCol->PutStoredWidth(0);
				pSSNCol->PutColumnStyle(csVisible|csFixedWidth);
			}
			else {
				pSSNCol->PutColumnStyle(csVisible);
				pSSNCol->PutStoredWidth(85);
			}
			pHealthCardCol->PutColumnStyle(csVisible|csFixedWidth);
			pHealthCardCol->PutStoredWidth(0);
		}
		//(s.dhole 5/12/2015 2:48 PM ) - PLID 66144 update  to Datalist2 
		NXDATALIST2Lib::IColumnSettingsPtr pSecurityGroupCol = m_PatientCombo->GetColumn(pccSecurityGroup);
		//if they have the preference set for the patient toolbar, show it here too
		if (GetRemotePropertyInt("PtTB_Security_Group", -9, 0, GetCurrentUserName(), true) > 0) {
			pSecurityGroupCol->PutColumnStyle(csVisible);
			pSecurityGroupCol->PutStoredWidth(70);		
		}
		else {
			pSecurityGroupCol->PutColumnStyle(csVisible|csFixedWidth);
			pSecurityGroupCol->PutStoredWidth(0);
		}
		// (z.manning 2010-10-28 11:58) - PLID 41129 - Use the advanced patient from clause
		m_PatientCombo->PutFromClause(_bstr_t(::GetPatientFromClause()));
		m_PatientCombo->Requery();
	
		// (r.farnworth 2015-05-13 11:39) - PLID 65625 - AppointmentLocation Dropdown needs to include Any Location and Multiple Locations
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_FFALocationCombo->GetNewRow();
		pNewRow = m_FFALocationCombo->GetNewRow();
		pNewRow->PutValue(lccID, _variant_t((long)-2));
		pNewRow->PutValue(lccName, _variant_t("{ Multiple Locations }"));
		m_FFALocationCombo->AddRowBefore(pNewRow, m_FFALocationCombo->GetFirstRow());

		pNewRow = m_FFALocationCombo->GetNewRow();
		pNewRow->PutValue(lccID, _variant_t((long)-1));
		pNewRow->PutValue(lccName, _variant_t("{ Any Location }"));
		m_FFALocationCombo->AddRowBefore(pNewRow, m_FFALocationCombo->GetFirstRow());

		// (r.farnworth 2015-06-08 12:53) - PLID 65639
		long nLocIDtoSet;
		if (m_bDefaultOverrides.bLocation){
			m_varLocIDs.Copy(m_aryPreselectLocations);
			ToggleLocationLabel();
		}
		else {
			// (r.farnworth 2015-05-21 17:29) - PLID 65625 - Pre-select the default location
			NXDATALIST2Lib::IRowSettingsPtr pDefLocRow = m_FFADefaultLocationCombo->GetCurSel();
			nLocIDtoSet = pDefLocRow->GetValue(fdlcID);

			if (nLocIDtoSet == FFA_DEFAULT_CURRENT_LOGEDIN_LOCATION || nLocIDtoSet == FFA_DEFAULT_CURRENT_SCHEDULER_LOCATION){
				nLocIDtoSet = GetDefaultLocationComboValue(pDefLocRow);
			}
			else if (nLocIDtoSet == FFA_DEFAULT_ANY_LOCATION) {
				nLocIDtoSet = -1;
			}

			m_FFALocationCombo->SetSelByColumn(lccID, nLocIDtoSet);
			m_nLastLocationID = nLocIDtoSet;
			m_varLocIDs.RemoveAll();
			m_varLocIDs.Add(variant_t(nLocIDtoSet));
		}

		m_nxstaticMultiLocationLabel.SetType(dtsHyperlink);
		m_nxstaticMultiLocationLabel.SetSingleLine();

		//(s.dhole 5/18/2015 3:05 PM ) - PLID 65627
		m_pAppHistoryList = BindNxDataList2Ctrl(IDC_FFA_APP_HIST_LIST, false);
		{
			CWnd *pWnd = GetDlgItem(IDC_FFA_APP_HIST_LIST);
			if (pWnd) {
				// Make the window zero size; Since this is an unusual use of the dl2, I 
				// set it to size 0 and no tab stop instead of making the window technically not "visible" so as to avoid 
				// any strange potential behavior changes that might come from that.)
				pWnd->SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
			}
		}
		//(a.wilson 2012-3-27) PLID 49245 - check preselects per override -- Patient
		// (b.savon 2013-07-29 10:46) - PLID 50524 - Move this up here
		if(m_bDefaultOverrides.bPatient){
			m_nPatientID = m_nPreselectPatientID;
		}
		else {
			m_nPatientID = GetActivePatientID();
		}

		// (r.gonet 2014-11-19) - PLID 64173 - Bind the insured party dropdown.
		m_dlInsuredPartyCombo = BindNxDataList2Ctrl(IDC_FFA_INSURED_PARTY_COMBO, false);
		// (r.gonet 2014-11-19) - PLID 64173 - Fill it with the patient's insured parties. Preselect the old
		// selected insured party only if we are also preselecting the same patient (diff patient would have diff
		// insured parties) and we should preselect the insurance.
		ReloadInsuredPartyCombo(m_bDefaultOverrides.bPatient && m_bDefaultOverrides.bInsurance);
		//(s.dhole 5/20/2015 11:12 AM ) - PLID 66144 Datalist 2 update
		m_dlAptType = BindNxDataList2Ctrl(this, IDC_APTTYPE_COMBO, GetRemoteData(), false);
		m_dlAptPurpose = BindNxDataList2Ctrl(this, IDC_APTPURPOSE_COMBO, GetRemoteData(), false);
		m_dlAptPurpose->Clear();
		m_dlAptPurpose->WhereClause = "";

		//(s.dhole 5/14/2015 5:04 PM ) - PLID 66125  will check overide first and then  regular  option
		if(m_bDefaultOverrides.bResources){
			//(e.lally 2011-05-16) PLID 41013 - Use the preselected values from the previous search when it is a revised one
			//(a.wilson 2012-3-27) PLID 49245 - check preselects per override -- Resources
			m_arySelectedResources.Copy(m_aryPreselectResources);
			//(s.dhole 5 / 22 / 2015 12:51 PM) - PLID 65621
			m_Resources->FindByColumn(fdrcID, (long)m_nPreselectResourceTypeSelection, NULL, VARIANT_TRUE);
			m_bSelectAnyResource = m_bPreselectAnyResource;
			ToggleResourcesSelections();
		}
		else {
			//(s.dhole 5/15/2015 4:54 PM ) - PLID 66125
			SelectResources();
		}	
		
		GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(FALSE);

		RequeryAptTypeCombo();
		
		InitializeHoursDropdown();
		InitializeMinsDropdown();

		//Get Preselected hours and minutes
		long nHours, nMinutes, nInterval;

		//(e.lally 2011-05-16) PLID 41013 - Use the preselected values from the previous search when it is a revised one
		//(a.wilson 2012-3-27) PLID 49245 - check preselects per override -- Durations
		if(m_bDefaultOverrides.bDurations){
			nHours = (m_nPreselectDurHours >= 1 && m_nPreselectDurHours <= 8) ? m_nPreselectDurHours : 0;
			nMinutes = m_nPreselectDurMins;
			nInterval = m_nPreselectIntervalMins;
			}
		else{
			nHours = 0;
			nMinutes = 15;
			nInterval = 15;
		}

		// (r.farnworth 2016-01-25 17:08) - PLID 65638
		RoundAppointmentDuration(nHours, nMinutes);
		//Set the dropdowns
		SetDropdownHours(nHours);
		SetDropdownMinutes(nMinutes);

		//Set the member variable for search interval and do updatedata
		SetSearchIntervalAndUpdate(nInterval, false);

		// Build time preference list box		

		// (j.politis 2015-06-05 11:10) - PLID 65642 - Extend the time preferences listbox to go from midnight to midnight.
		//generate all the hourly list selections in a loop.
		for (int i = 0; i < 24; i++) {
			m_TimePrefs.AddString(GetHourSpanText(i));
			m_TimePrefs.SetItemData(i, i);
		}

		//(e.lally 2011-05-16) PLID 41013 - Use the preselected values from the previous search when it is a revised one
		//(a.wilson 2012-3-27) PLID 49245 - check preselects per override -- Days
		if(m_bDefaultOverrides.bDaysAvailable) {
			//Preselect the days
			for(int i=0; i<m_aryPreselectWeekDays.GetCount(); i++){
				long nDay = m_aryPreselectWeekDays[i];
				// (z.manning 2013-11-19 10:09) - PLID 58756 - This is now zero-based
				switch(nDay){
					case 0: ((CButton*)GetDlgItem(IDC_CHECK_SUN))->SetCheck(1); break;
					case 1: ((CButton*)GetDlgItem(IDC_CHECK_MON))->SetCheck(1); break;
					case 2: ((CButton*)GetDlgItem(IDC_CHECK_TUE))->SetCheck(1); break;
					case 3: ((CButton*)GetDlgItem(IDC_CHECK_WED))->SetCheck(1); break;
					case 4: ((CButton*)GetDlgItem(IDC_CHECK_THU))->SetCheck(1); break;
					case 5: ((CButton*)GetDlgItem(IDC_CHECK_FRI))->SetCheck(1); break;
					case 6: ((CButton*)GetDlgItem(IDC_CHECK_SAT))->SetCheck(1); break;
					default: ASSERT(FALSE); break;
				}
			}
		}
		else {
			// Default all weekdays to available
			((CButton*)GetDlgItem(IDC_CHECK_MON))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_CHECK_TUE))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_CHECK_WED))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_CHECK_THU))->SetCheck(1);
			((CButton*)GetDlgItem(IDC_CHECK_FRI))->SetCheck(1);
		}

		//(a.wilson 2012-3-27) PLID 49245 - check preselects per override -- StartDate
		if (m_bDefaultOverrides.bStartDate) {

			if(m_bPreselectStartDateRadio){
				m_boStartFromRadio = 0;
				m_dtDateCtrl.SetValue(m_dtPreselectStart);
			}
			else {
				m_boStartFromRadio = 1;
				COleVariant varCurDate = COleDateTime::GetCurrentTime();
				m_dtDateCtrl.SetValue(varCurDate);
			}
		} else {
			COleVariant varCurDate = COleDateTime::GetCurrentTime();
			m_boStartFromRadio = 0;
			m_dtDateCtrl.SetValue(varCurDate);
		}

		// Default all times to available
		/*((CButton*)GetDlgItem(IDC_CHECK_ANY_TIME))->SetCheck(1);
		OnCheckAnyTime();*/
		
		//TES 5/11/2006 - FindByColumn is a speed hit because it requires waiting for the patient list requery to finish.
		//Instead, we'll just tell it to set the selection when it can, and reference our member variable instead of the
		//datalist when we need to know the ID.
		//m_PatientCombo->FindByColumn(0, m_nPatientID, 0, 1);
		CString strName = GetActivePatientName();
		if(m_nPatientID > 0 && strName == "") {
			//Oh, for crying out loud, even the patient toolbar hasn't finished loading yet.  Let's look up the name we
			//need real quick.
			strName = VarString(GetTableField("PersonT", "Last + ', ' + First + ' ' + Middle", "ID", m_nPatientID),"");
		}
		m_PatientCombo->PutComboBoxText(_bstr_t(strName));
		//(s.dhole 5/12/2015 2:48 PM ) - PLID 66144 update  to Datalist2 
		m_PatientCombo->SetSelByColumn(pccID, m_nPatientID);		

		UpdateData(FALSE);

		//(e.lally 2011-05-16) PLID 41013 - Use the preselected values from the previous search when it is a revised one
		//(a.wilson 2012-3-27) PLID 49245 - check preselects per override -- Appt
		if(m_bDefaultOverrides.bAppt){
			//(s.dhole 5/20/2015 10:47 AM ) - PLID 66144
			IRowSettingsPtr pNewSel = m_dlAptType->FindByColumn(atcID, m_nPreselectApptTypeID, NULL, VARIANT_TRUE);
			if (pNewSel){
				//(s.dhole 5/20/2015 10:47 AM ) - PLID 66144
				OnSelChosenApttypeCombo(pNewSel);
				if(m_nPreselectApptPurposeID != -1){
					pNewSel = m_dlAptPurpose->FindByColumn(apcID, m_nPreselectApptPurposeID, NULL, VARIANT_TRUE);
					if(pNewSel){
						//(s.dhole 5/20/2015 10:47 AM ) - PLID 66144
						OnSelChosenAptpurposeCombo(pNewSel);
					}
				}
			}
		}

		// Set the status of the Go button
		SetGoStatus();

		// Mark the 'start n days after last appointment of patient' radio
		// disabled if the patient has no appointments
		ValidatePtFilter();

		// (c.haag 2003-07-15 16:54) - Enable the search interval edit box
		// based on the Show Results in List check state.
		//(s.dhole 6/2/2015 9:04 AM ) - PLID 65643 Remove functionality
		//OnCheckResults();

		// (c.haag 2003-09-03 13:42) - Change the times and names to the
		// country's defaults.
		EnsureInternationalInterface();

		// (c.haag 2004-12-27 16:20) - PLID 14967 - We now have an office
		// hours checkbox
		//(e.lally 2011-05-16) PLID 41013 - Use the preselected values from the previous search when it is a revised one
		//(a.wilson 2012-3-27) PLID 49245 - check preselects per override -- OfficeHours
		if(m_bDefaultOverrides.bOfficeHours){
			CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_OFFICE_HOURS_ONLY);
			pButton->SetCheck(m_bPreselectOfficeHoursOnly ? 1 : 0);
		}
		else {
			if(GetRemotePropertyInt("FFAConsiderOfficeHours", 0, 0, "<None>", true))
			{
				CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_OFFICE_HOURS_ONLY);
				pButton->SetCheck(1);
			}
			else {
				CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_OFFICE_HOURS_ONLY);
				pButton->SetCheck(0);
			}
		}
		OnCheckOfficeHoursOnly();

		///////////////////////
		// DRT 12/16/2004 - PLID 14968
		// Load some settings that they probably use repeatedly
		{	//Exclude Available times on template warnings
			// (r.farnworth 2016-02-03 11:59) - PLID 67954 - Default the FFA "Exclude available times that fall on template warnings" checkbox (user preference) to ON for new users
			UINT nExclude = GetRemotePropertyInt("FFA_ExcludeAvailOnWarnings", 1, 0, GetCurrentUserName(), true);
			CheckDlgButton(IDC_CHECK_EXCLUDE_WARNINGS, nExclude);
		}

		{	
			// (b.savon 2016-01-27 10:03) - PLID 67956 - Use global preference
			//Allow booking against...
			if (GetRemotePropertyInt("FFA_AllowBookingsAgainst", 0, 0, "<None>", true)  != 0) {
				m_iExtraBookingLimit = GetRemotePropertyInt("FFA_AllowBookingsAgainst_NumberAppointments", 0, 0, "<None>", true);
			}
			else {
				m_iExtraBookingLimit = 0;
			}
		}

		long nDaysToSearch = GetRemotePropertyInt("FFA_DaysToSearch", 45, 0, GetCurrentUserName(), true);
		SetDlgItemInt(IDC_EDIT_SEARCH_DAYS, nDaysToSearch);

		// Done loading settings
		///////////////////////

		// (c.haag 2006-11-21 13:17) - PLID 23629 - Populate as many times
		// as possible in the time list
		//(e.lally 2011-05-16) PLID 41013 - Use the preselected values from the previous search when it is a revised one
		//(a.wilson 2012-3-27) PLID 49245 - check preselects per override -- Times (timepref and anytime)
		// (a.wilson 2014-02-04 15:58) - PLID 15410 - simplified this to not require hardcoded values.
		if(m_bDefaultOverrides.bTimes)
		{
			for(int i=0; i< m_aryPreselectTimes.GetCount(); i++){
				long nTimeVal = m_aryPreselectTimes.GetAt(i);
				// (j.politis 2015-06-02 12:24) - PLID 65647 - Make the time anchors in FFA use ConfigRT instead of being hard-coded.
				SetHourSelected(nTimeVal, TRUE);
			}
			UpdateTimePrefCheckboxes();
			if(m_bPreselectAnyTimeChecked){
				CheckDlgButton(IDC_CHECK_ANY_TIME, BST_CHECKED);
				OnCheckAnyTime();
			}
		}
		else {
			// (z.manning 2013-11-21 16:29) - PLID 59638 - Instead of checking all times by default, let's just check
			// a more reasonable 9-5.
			CheckDlgButton(IDC_CHECK_ANY_TIME, BST_UNCHECKED);

			// (b.savon 2016-02-16 15:12) - PLID 65647 - Default anchor time spans, not hard coded times
			CheckDlgButton(IDC_CHECK_EARLY_MORN, BST_CHECKED);
			OnCheckEarlyMorn();

			CheckDlgButton(IDC_CHECK_LATE_MORN, BST_CHECKED);
			OnCheckLateMorn();

			CheckDlgButton(IDC_CHECK_EARLY_AFTN, BST_CHECKED);
			OnCheckEarlyAftn();

			CheckDlgButton(IDC_CHECK_LATE_AFTN, BST_CHECKED);
			OnCheckLateAftn();

			UpdateTimePrefCheckboxes();
		}

		// (v.maida 2016-02-05 09:53) - PLID 68132 - Scroll to earliest opening time, based off all available locations.
		ScrollToEarliestLocationOpeningTime();

		//(e.lally 2011-05-16) PLID 41013 - Now that we've potentially used the preselect values, reset them.
		ResetPreselects();

		// (c.haag 2006-11-21 10:47) - PLID 23623 - Resize the dialog according to
		// whether the user wants to see the advanced settings
		//(s.dhole 5/12/2015 2:48 PM ) - PLID 65619 set initial value
		m_bShowingAllFields = FALSE;
		UpdateWindowSize();

		// (v.maida 2016-03-14 17:59) - PLID 68448 - Show don't remind me dialog, if the "Only show time slots with location templates" preference is
		// not set and the user has multiple active locations
		{
			bool bOnlySlotsWithLocations = GetRemotePropertyInt("FFAOnlyShowLocationSpecificResults", 0, 0, "<None>", true) == 0 ? false : true;
			if (bOnlySlotsWithLocations == false) {
				long nActiveLocations;
				{
					_RecordsetPtr rsTemp = CreateRecordset("SELECT COUNT(ID) AS LocationCount FROM LocationsT WHERE TypeID = 1 AND Active = 1");
					nActiveLocations = AdoFldLong(rsTemp, "LocationCount");
				}
				if (nActiveLocations > 1)
				{
					DontShowMeAgain(this, "Because you have multiple active locations, there is a global preference for Find First Available called \"Only show time slots with Location Templates\" that should "
						"be turned on once you have Location Templates set up for all of your active locations. If you wish to learn more about how to set up Scheduler Location Templating, please visit the Product Education section of the Nextech Community Portal."
						, "FFAOnlyShowLocationSpecificResultsWarning", "FFA Location Template Preference Not Set");
				}
			}
		}

		//(s.dhole 6/2/2015 9:04 AM ) - PLID 65643 Remove functionality
		//if (m_bReturnSlot) {
		//	//TES 1/9/2015 - PLID 64180 - Make sure this is checked and disabled
		//	m_checkResults.SetCheck(1);
		//	m_checkResults.EnableWindow(FALSE);
		//}
		//else {
		//	//TES 1/12/2015 - PLID 64180 - Make sure the box is enabled
		//	m_checkResults.EnableWindow(TRUE);
		//}
		if (m_bRunImmediately) {
			//TES 12/3/2014 - PLID 64180 - Post a message to make sure the positioning on the list window is correct
			PostMessage(NXM_AUTO_RUN_FFA);
		}

	} NxCatchAll("Error in initializing FirstAvailableAppointment");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (j.politis 2015-05-18 12:35) - PLID 65626 - Adds list of hours or minutes values to the hours or minutes datalist.
long CFirstAvailableAppt::AddDurationRows(NXDATALIST2Lib::_DNxDataListPtr pList, const std::initializer_list<_variant_t> &varAryValues)
{
	//Count how many rows we insert
	long count = 0;

	for (const _variant_t& value : varAryValues)
	{
		IRowSettingsPtr pRow = pList->GetNewRow();
		pRow->PutValue(0, value);
		//Make a new row and add it
		pList->AddRowSorted(pRow, NULL);
		count++;
	}

	//Return the number of rows we inserted
	return count;
}

// (j.politis 2015-05-18 13:02) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
long CFirstAvailableAppt::CalculateApptDuration(long nHours, long nMins)
{
	//Quick calc of total minutes from hours plus mins
	return nHours * 60 + nMins;
}

// (j.politis 2015-05-13 14:41) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns. 
void CFirstAvailableAppt::InitializeHoursDropdown()
{
	//Bind member variable to the dl2
	m_dlHours = BindNxDataList2Ctrl(this, IDC_FFA_HOURS_COMBO, GetRemoteData(), false);

	//Hard code the duration in hours that an appointment could possibly take.
	AddDurationRows
		(
		m_dlHours,
		{
			"0"
			, "1"
			, "2"
			, "3"
			, "4"
			, "5"
			, "6"
			, "7"
		}
	);
}

// (j.politis 2015-05-13 14:41) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns. 
void CFirstAvailableAppt::InitializeMinsDropdown()
{
	//Bind member variable to the dl2
	m_dlMinutes = BindNxDataList2Ctrl(this, IDC_FFA_MINS_COMBO, GetRemoteData(), false);

	//Hard code the duration in minutes that an appointment could possibly take.
	AddDurationRows
		(
		m_dlMinutes,
		{
			"00"
			, "05"
			, "07"
			, "10"
			, "15"
			, "20"
			, "25"
			, "30"
			, "35"
			, "40"
			, "45"
			, "50"
			, "55"
		}
	);
}

void CFirstAvailableAppt::CalculateStartDate()
{
	switch (m_boStartFromRadio)
	{
	case 0:{ // Start from selected date
		// (a.walling 2008-05-13 15:15) - PLID 27591 - Don't need .date anymore
		m_dtStartDate = m_dtDateCtrl.GetValue();
//The control gives us the current time, we just want the date
//m_dtStartDate.SetDateTime(m_dtStartDate.GetYear(), m_dtStartDate.GetMonth(), m_dtStartDate.GetDay(), 7,0,0);
		break;
		   }
	case 1: // Start n days after the patient's next appointment
		{
			CString strSQL;
			strSQL.Format("SELECT Max(AppointmentsT.[StartTime]) AS LastTime FROM AppointmentsT WHERE PatientID = %d",
				m_nPatientID);
			_RecordsetPtr prs = CreateRecordset(strSQL);
			if (!prs->eof)
			{
				COleVariant var = prs->Fields->Item["LastTime"]->Value;
				if (var.vt == VT_NULL || var.vt == VT_EMPTY)
				{
					 // This should never happen, but lets not leave
					// any airholes around.
					m_dtStartDate = COleDateTime::GetCurrentTime();
				}
				else {
					m_dtStartDate = prs->Fields->Item["LastTime"]->Value.date;
					m_dtStartDate += COleDateTimeSpan(atoi(m_strDaysAfter), 0,0,0);
				}
			}
			else
			{
				m_dtStartDate = COleDateTime::GetCurrentTime();
			}
		}
		break;
	}
}

void CFirstAvailableAppt::ValidatePtFilter()
{
	CString strSQL;
	strSQL.Format("SELECT Max(AppointmentsT.[StartTime]) AS LastTime FROM AppointmentsT WHERE PatientID = %d",
		m_nPatientID);
	_RecordsetPtr prs = CreateRecordset(strSQL);
	if (!prs->eof)
	{
		COleVariant var = prs->Fields->Item["LastTime"]->Value;
		if (var.vt == VT_NULL || var.vt == VT_EMPTY)
		{
			GetDlgItem(IDC_EDIT_DAYS_AFTER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_STARTPAT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_RADIO_STARTPAT)->EnableWindow(FALSE);
			GetDlgItem(IDC_STATIC_DAYSAFTER)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_NODAYSAFTER)->ShowWindow(SW_SHOW);
			m_boStartFromRadio = 0;
			UpdateData(FALSE);
		}
		else {
			GetDlgItem(IDC_EDIT_DAYS_AFTER)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_RADIO_STARTPAT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_RADIO_STARTPAT)->EnableWindow(TRUE);
			GetDlgItem(IDC_STATIC_DAYSAFTER)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_NODAYSAFTER)->ShowWindow(SW_HIDE);
		}
	}
	else
	{
		GetDlgItem(IDC_EDIT_DAYS_AFTER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO_STARTPAT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RADIO_STARTPAT)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_DAYSAFTER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_NODAYSAFTER)->ShowWindow(SW_SHOW);
		m_boStartFromRadio = 0;
		UpdateData(FALSE);
	}
}


//(s.dhole 5/20/2015 10:47 AM ) - PLID 66144 updat code to datalist 2
void CFirstAvailableAppt::OnSelChosenApttypeCombo(LPDISPATCH lpRow)
{
	try {

		// Then apply the selection change to the purpose combo list
		_variant_t varAptTypeID;
		varAptTypeID.vt = VT_NULL;
		IRowSettingsPtr pRow = m_dlAptType->GetCurSel();
		if (pRow) {
			varAptTypeID = pRow->GetValue(atcID);
		}
		if (varAptTypeID.vt != VT_NULL) {
			// Generate the new filter based on the apt type
			CString strPurposeFilter;
			CString strResources;
			long nResources = 0;
			//(s.dhole 5/14/2015 5:06 PM ) - PLID 66125 Change Code to support new structure		
			for (int i = 0; i < m_arySelectedResources.GetCount(); i++)
			{
				CString str;
				str.Format(",%d ", m_arySelectedResources[i] );
				strResources += str;
				nResources++;
			}
			if (nResources)
			{
				// (r.farnworth 2016-02-12 08:20) - PLID 67960 - The having clause would restrict purposes for all resources from appearing in the list. If we select Any, we want them all to show/
				CString strHaving = "";
				if (!m_bSelectAnyResource || GetResourceComboSelectedValue() != FFA_MULTIPLE_SCHEDULER_RESOURCE)
				{
					strHaving.Format("having count(aptpurposeid) = %d", nResources);
				}

				//TES 7/15/2004 - PLID 13466 - The purpose needs to be valid both by resource AND type.
				// (c.haag 2008-12-18 12:55) - PLID 32376 - Filter out inactive procedures
				// (r.farnworth 2016-02-12 08:20) - PLID 67960 - Made the Having clause conditional
				strResources = strResources.Right(strResources.GetLength() - 1);
				strPurposeFilter.Format("AptPurposeT.ID IN (select aptpurposeid from (select resourceid, aptpurposeid from resourcepurposetypet where apttypeid = %d group by resourceid, aptpurposeid) SubQ where SubQ.ResourceID in (%s) "
					"AND AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li) "
					"AND AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) "
					"group by aptpurposeid %s) ",
					VarLong(varAptTypeID), strResources, VarLong(varAptTypeID), strHaving);
			}
			else
			{
				// (c.haag 2004-04-01 16:16) - If no resources are selected, just
				// user the older filter
				// (c.haag 2008-12-18 12:55) - PLID 32376 - Filter out inactive procedures
				strPurposeFilter.Format(
					"AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li) "
					"AND AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1) ", 
					VarLong(varAptTypeID));
			}

			if (m_dlAptPurpose->WhereClause != _bstr_t(strPurposeFilter)) {
				m_dlAptPurpose->WhereClause = _bstr_t(strPurposeFilter);
				// If the filter has changed we want to requery the purposes
				try {
					RequeryAptPurposes();
				}NxCatchAll("Error loading purpose list");
			}
			GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(TRUE);

		} else {
			m_dlAptPurpose->Clear();
			m_dlAptPurpose->WhereClause = "";
			GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(FALSE);
		}

		// (c.haag 2007-02-20 15:28) - PLID 24567 - We now have a standalone function for
		// automatically updating the appointment length list
		if (!m_bUserEditedDuration) {
			UpdateAppointmentLength();
		}

	}NxCatchAll("CFirstAvailableAppt::OnSelChosenApttypeCombo");
}

//(s.dhole 5/20/2015 10:47 AM ) - PLID 66144
void CFirstAvailableAppt::OnSelChosenAptpurposeCombo(LPDISPATCH lpRow)
{
	try {
		// (c.haag 2007-02-20 12:43) - PLID 24567 - We now have a standalone function for
		// automatically updating the appointment length list
		if (!m_bUserEditedDuration) {
			UpdateAppointmentLength();
		}
	}NxCatchAll("CFirstAvailableAppt::OnSelChosenAptpurposeCombo");
}

void CFirstAvailableAppt::RequeryAptPurposes()
{
	//TES 7/15/2004: The {No Purpose} or {No Type} row may be selected.
	long nAptTypeID = GetSelectedAppointmentType();
	long nPurposeID = GetSelectedAppointmentPurpose();
	if (nAptTypeID == -1)
	{
		m_dlAptPurpose->Clear();
		return;
	}

	// Requery the datalist because that's what this function does
	m_dlAptPurpose->Requery();

	//Wait until the requery finishes
	m_dlAptPurpose->WaitForRequery(dlPatienceLevelWaitIndefinitely);

	// Add a row to allow the user to select "no appointment type"
	//(s.dhole 5/20/2015 11:01 AM ) - PLID 66144 Update datalist 2
	_variant_t varNull;
	varNull.vt = VT_NULL;
	IRowSettingsPtr pRow = m_dlAptPurpose->GetNewRow();
	pRow->PutValue(apcID, varNull);
	pRow->PutValue(apcPurpose, _bstr_t("{ No Purpose }"));
	pRow->PutValue(apcArrivalPrepMinutes, (long)0);
	m_dlAptPurpose->AddRowBefore(pRow, m_dlAptPurpose->GetFirstRow());
	pRow = NULL;
	if (nPurposeID > -1) {
		pRow =m_dlAptPurpose->FindByColumn(apcID, nPurposeID, NULL, VARIANT_TRUE);
	}
	// check if we have row sselected
	if (!pRow)
	{
		m_dlAptPurpose->FindByColumn(apcID, varNull, NULL, VARIANT_TRUE);
	}
	
}

void CFirstAvailableAppt::OnKillFocusSearchInterval()
{
	try
{
		//Get m_iSearchInterval updated with whats in the editbox
		UpdateData(TRUE);

		//Then, we pass it to this function to set itself and update the gui
		//because this function handles some logic before it does so.
		SetSearchIntervalAndUpdate(m_iSearchInterval, false);

	}NxCatchAll(__FUNCTION__);
}

void CFirstAvailableAppt::OnHelp() 
{
	//(j.anspach 06-09-2005 10:53 PLID 16662) - Updating the OpenManual call to work with the new help system.
	OpenManual("NexTech_Practice_Manual.chm", "The_Scheduler_Module/find_the_first_available_appointment.htm");
}

void CFirstAvailableAppt::EnsureInternationalInterface()
{
	// (c.haag 2003-09-03 13:45) - Begin with the day-of-the-week names.
	CString strText;
	strText = GetDayText(1, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_CHECK_MON, strText);
	strText = GetDayText(2, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_CHECK_TUE, strText);
	strText = GetDayText(3, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_CHECK_WED, strText);
	strText = GetDayText(4, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_CHECK_THU, strText);
	strText = GetDayText(5, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_CHECK_FRI, strText);
	strText = GetDayText(6, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_CHECK_SAT, strText);
	strText = GetDayText(0, false); if (strText.GetLength() > 3) strText = strText.Left(3); strText += ".";
	SetDlgItemText(IDC_CHECK_SUN, strText);
}

void CFirstAvailableAppt::RequeryAptTypeCombo()
{
	CString strResources;
	long nResources = 0;
	long nAptTypeID = GetSelectedAppointmentType();

	//DRT 5/17/2004 - PLID 12406 - This needs to be preferenced, otherwise it will hide types
	//	and the user may not have a clue why.
	if(GetRemotePropertyInt("HideEmptyTypes", 0, 0, "<None>", true)) {
		//(s.dhole 5/14/2015 5:06 PM ) - PLID 	65623 Change Code to support new structure
		for (int i = 0; i < m_arySelectedResources.GetCount(); i++)
		{
			CString str;
			str.Format(",%d ", m_arySelectedResources[i]);
			strResources += str;
			nResources++;

		}
		if (strResources.IsEmpty())
		{
			m_dlAptType->WhereClause = "Inactive = 0";
		}
		else
		{
			CString strWhere;
			strResources = strResources.Right(strResources.GetLength() - 1);

			// (r.farnworth 2016-02-12 08:20) - PLID 67960 - The having clause would restrict types for all resources from appearing in the list. If we select Any, we want them all to show
			CString strHaving = "";
			if (!m_bSelectAnyResource || GetResourceComboSelectedValue() != FFA_MULTIPLE_SCHEDULER_RESOURCE)
			{
				strHaving.Format("having count(apttypeid) = %d", nResources);
			}

			// (r.farnworth 2016-02-12 08:20) - PLID 67960 - Made the Having clause optional now
			strWhere.Format("Inactive = 0 AND ID IN (select apttypeid from (select resourceid, apttypeid from resourcepurposetypet group by resourceid, apttypeid) SubQ where SubQ.ResourceID in (%s) group by apttypeid %s)",
				strResources, strHaving);
			m_dlAptType->WhereClause = _bstr_t(strWhere);
		}
	}

	m_dlAptType->Requery();

	//Wait until the requery finishes
	m_dlAptType->WaitForRequery(dlPatienceLevelWaitIndefinitely);

	// Add a row to allow the user to select "no appointment type"
	_variant_t varNull;
	varNull.vt = VT_NULL;
	IRowSettingsPtr pRow = m_dlAptType->GetNewRow() ;
	pRow->PutValue(atcID, varNull);
	pRow->PutValue(atcTypeName, _bstr_t("{ No Appointment Type }"));
	pRow->PutValue(atcDefaultArrivalMins, (long)0); // DefaultArrivalMins
	pRow->PutValue(atcCategory, (byte)-1); // Category
	m_dlAptType->AddRowBefore(pRow, m_dlAptType->GetFirstRow());

	if (!m_dlAptType->FindByColumn(atcID, nAptTypeID, NULL, VARIANT_TRUE))
	{
		m_dlAptType->FindByColumn(atcID, varNull, NULL, VARIANT_TRUE);
	}
	
}

BOOL CFirstAvailableAppt::SearchOfficeHoursOnly()
{
	UpdateData(TRUE);
	return m_boOfficeHoursOnly;
}


//(s.dhole 5/20/2015 11:13 AM ) - PLID 66144 update to datalist 2 code
int CFirstAvailableAppt::GetDefaultArrivalMins()
{
	int nMins = 0;

	
	IRowSettingsPtr pRow;
	// First, let's try to set arrival time based on the procedure's preptime if it's a procedural type.
	if( IsAptTypeCategorySurgical(GetCurAptTypeCategory()) ) {
		pRow = m_dlAptPurpose->GetCurSel() ;
		if (pRow) {
			nMins = VarLong(pRow->GetValue(apcArrivalPrepMinutes));
		}
	}

	// If that didn't work, let's try the type
	pRow = m_dlAptType->GetCurSel();
	if (nMins <= 0 && pRow) {
		nMins = VarLong(pRow->GetValue(atcDefaultArrivalMins));
	}

	return nMins;
}

//Get the appropriate category for our current type.
//(s.dhole 5/20/2015 11:13 AM ) - PLID 66144 update to datalist 2 code
short CFirstAvailableAppt::GetCurAptTypeCategory()
{
	IRowSettingsPtr pRow = m_dlAptType->GetCurSel();
	if (pRow) {
		return VarByte(pRow->GetValue(atcCategory), -1);
	} else {
		return -1;
	}
}
//(s.dhole 5/12/2015 2:48 PM ) - PLID 65619  
void CFirstAvailableAppt::OnShowExInfo()
{
	//
	// (c.haag 2006-11-21 11:46) - PLID 23623 - Toggle the current user's FFA visibility settings
	//
	//(s.dhole 5/12/2015 2:48 PM ) - PLID 65619 
	try{
		m_bShowingAllFields = !m_bShowingAllFields;
		UpdateWindowSize();
	}NxCatchAll(__FUNCTION__);
}

void CFirstAvailableAppt::UpdateWindowSize()
{	
	//
	// (c.haag 2006-11-21 11:42) - PLID 23623 - This function will resize the entire FFA interface
	// based on whether the user wants to see the advanced options (i.e. choosing resources, times)
	//
	//(s.dhole 5/12/2015 2:48 PM ) - PLID 65619 
	if (m_bShowingAllFields){
		ShowMoreFields();
	} else {
		ShowLessFields();
	}
}
//(s.dhole 5/12/2015 2:48 PM ) - PLID 65619 
void CFirstAvailableAppt::ShowMoreFields()
{
	ShowAdvOptions(TRUE);
	SetDlgItemText(IDC_BTN_SHOWEXINFO, "<< Hide Advanced Settings");
	m_bShowingAllFields = TRUE;
}
//(s.dhole 5/12/2015 2:48 PM ) - PLID 65619 
void CFirstAvailableAppt::ShowLessFields()
{
	ShowAdvOptions(FALSE);
	SetDlgItemText(IDC_BTN_SHOWEXINFO, "Advanced Settings >>");
	m_bShowingAllFields = FALSE;
}

//(s.dhole 5/12/2015 2:48 PM ) - PLID 65619  Added functio to resize window
void CFirstAvailableAppt::ShowAdvOptions(BOOL bShow)
{
	CRect rcWindow;
	GetWindowRect(rcWindow);
	MoveWindow(rcWindow.left, rcWindow.top, rcWindow.Width() + ((bShow ? 1 : -1) * 350), rcWindow.Height(), SWP_NOZORDER);
}

BOOL CFirstAvailableAppt::GotoAlternateNextDlgTabItem(BOOL bGoToPreviousCtrl)
{
	// (c.haag 2006-11-28 14:28) - PLID 23629 - This function is called when the
	// user is tabbing in collapsed mode. If the current control is the More/Less
	// button, then we want to jump to the patient dropdown. If we're on the patient
	// dropdown, we need to go to the More/Less button
	CWnd *pWndCurrent = GetFocus();
	if (pWndCurrent && IsWindow(pWndCurrent->GetSafeHwnd())) {

		// (c.haag 2006-11-28 14:30) - If we are tabbing forward from the More/Less button,
		// then jump to the patient combo
		if (!bGoToPreviousCtrl && pWndCurrent->GetSafeHwnd() == GetDlgItem(IDC_BTN_SHOWEXINFO)->GetSafeHwnd()) {
			GotoDlgCtrl( GetDlgItem(IDC_PATIENT_LIST_COMBO) );
			return TRUE;
		}
		// (c.haag 2006-11-28 14:31) - Otherwise, if we are tabbing backward from the patient
		// dropdown, go to the More/Less button
		else if (bGoToPreviousCtrl && pWndCurrent->GetSafeHwnd() == GetDlgItem(IDC_PATIENT_LIST_COMBO)->GetSafeHwnd()) {
			GotoDlgCtrl( GetDlgItem(IDC_BTN_SHOWEXINFO) );
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CFirstAvailableAppt::PreTranslateMessage(MSG* pMsg) 
{
	// (c.haag 2006-11-28 14:20) - PLID 23629 - If the window is in collapsed mode,
	// then we need to make it so tabbing to the last visible control takes us back to
	// the first visible control
	if (!m_bShowingAllFields && WM_KEYDOWN == pMsg->message && VK_TAB == pMsg->wParam) {
		if (GotoAlternateNextDlgTabItem(GetAsyncKeyState(VK_SHIFT) & 0x80000000)) {
			return TRUE;
		}
	}
	//TES 1/23/2015 - PLID 64513 - If we're disabled, ignore everything except the message to launch the list, and the message that it's been closed.
	if (!IsWindowEnabled() && pMsg->message != NXM_AUTO_RUN_FFA && pMsg->message != NXM_ON_CLOSE_FIRST_AVAIL_LIST) {
		return FALSE;
	}
	return CNxDialog::PreTranslateMessage(pMsg);
}


long CFirstAvailableAppt::GetSelectedAppointmentType()
{
	//(s.dhole 5/20/2015 11:13 AM ) - PLID 66144 update to datalist 2 code
	IRowSettingsPtr pRow = m_dlAptType->GetCurSel();
	return (pRow) ? VarLong(pRow->GetValue(atcID), -1) : -1;
}

long CFirstAvailableAppt::GetSelectedAppointmentPurpose()
{
	//(s.dhole 5/20/2015 11:13 AM ) - PLID 66144 update to datalist 2 code
	IRowSettingsPtr pRow = m_dlAptPurpose->GetCurSel();
	return (pRow) ? VarLong(pRow->GetValue(apcID), -1) : -1;
}

void CFirstAvailableAppt::EnsureDurationGroups()
{
	//
	// (c.haag 2007-02-20 12:07) - PLID 24567 - This function populates our default duration
	// groups lists. The query was copied from ResEntryDlg.cpp.
	//
	if (m_bDurationGroupsLoaded)
		return;

	// Ensure that the group list is clear
	const int nGroups = m_aDurationGroups.GetSize();
	int i;
	for (i=0; i < nGroups; i++) {
		delete m_aDurationGroups[i];
	}
	m_aDurationGroups.RemoveAll();

	// (j.jones 2010-10-22 16:00) - PLID 36473 - we now have limited support
	// for default durations for multiple resources, in that we use the longest
	// applicable duration
	CArray<long,long> arynResourceIDs;
	//(s.dhole 5/14/2015 5:06 PM ) - PLID 	65623 Change Code to support new structure
	arynResourceIDs.Copy(m_arySelectedResources);
	// Now populate the group list
	// (z.manning 2011-05-10 09:04) - PLID 43601 - Moved the logic to load default durations to a utility function
	FillDefaultDurationGroups(&m_aDurationGroups, &arynResourceIDs);

	m_bDurationGroupsLoaded = TRUE;
}

void CFirstAvailableAppt::InvalidateDurationGroups()
{
	const int nGroups = m_aDurationGroups.GetSize();
	for (int i=0; i < nGroups; i++) {
		delete m_aDurationGroups[i];
	}
	m_aDurationGroups.RemoveAll();
	m_bDurationGroupsLoaded = FALSE;
}

BOOL CFirstAvailableAppt::CalculateDefaultDuration(OUT COleDateTimeSpan& dts)
{
	//
	// (c.haag 2007-02-20 10:35) - PLID 24567 - This function will calculate the default durations given
	// the selected criteria on the dialog. The logic mimics CResEntryDlg::FillDefaultEndTime. This function
	// throws an exception on failure.
	//
	long nAptTypeID = GetSelectedAppointmentType();
	long nAptPurposeID = GetSelectedAppointmentPurpose();

	// Fail if we have no appointment type
	if (-1 == nAptTypeID)
		return FALSE;

	// Make sure we've loaded our duration group list so we can do proper calculations later
	EnsureDurationGroups();

	// Traverse each group to find the first one where all of the
	// purposes are an exact match.
	long nDuration = VarLong(GetTableField("AptTypeT", "DefaultDuration", "ID", nAptTypeID));
	BOOL bDurationGroupFound = FALSE;
	if (-1 != nAptPurposeID) { 
		for (int i=0; i < m_aDurationGroups.GetSize() && !bDurationGroupFound; i++)
		{
			CDurationGroup* pGroup = m_aDurationGroups[i];

			// We only allow one purpose to be scheduled in FFA, so the default duration group
			// must also have only one purpose
			if(1 != pGroup->GetPurposeCount()) {
				continue;
			}
			// The group appointment type and appointment type must match
			if (pGroup->GetAptTypeID() != nAptTypeID) {
				continue;
			}

			// Check whether the purpose ID is part of the group
			if (pGroup->InGroup(nAptPurposeID))
			{
				// Yes, we have a perfect match!
				nDuration = pGroup->GetDuration();
				bDurationGroupFound = TRUE;
			}
		}
	}

	// If the duration is zero, that means there is no default duration
	if (0 == nDuration) {
		return FALSE;
	} else {
		dts = COleDateTimeSpan(0,0,nDuration,0);
		return TRUE;
	}
}

void CFirstAvailableAppt::UpdateAppointmentLength()
{
	// (j.politis 2015-05-18 13:30) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
	try{
		COleDateTimeSpan dts;

		if (GetDefaultSearchInterval(dts))
		{
			long nHours = dts.GetHours();
			long nMinutes = dts.GetMinutes();

			// (r.farnworth 2016-01-25 17:08) - PLID 65638 - Broke the rounding out into it's own function
			RoundAppointmentDuration(nHours, nMinutes);

			//Set the hours
			SetDropdownHours(nHours);
			//Set the minutes
			SetDropdownMinutes(nMinutes);

			// (c.haag 2007-02-20 15:19) - PLID 24829 - Now update the search interval.
			// m_iSearchInterval does tie into a form control, so we need to be careful.
			SetSearchIntervalAndUpdate((int)dts.GetTotalMinutes() , true);
		}
	}NxCatchAllIgnore();  //This is a utility function, but we don't want this to cause the calling function to fail because it is not critical.
}

// (r.farnworth 2016-01-25 17:08) - PLID 65638 - Broke out into it's own function
void CFirstAvailableAppt::RoundAppointmentDuration(long &nHours, long &nMinutes)
{
	// (z.manning, 08/14/2006) - PLID 21956 - We don't want to set the duration to zero.
	// (j.gruber 2010-06-07 13:32) - PLID 38854 - or -1
	if (nHours <= 0 && nMinutes <= 0) {
		return;
	}

	//Calculate what we'll set the minutes at
	if (nMinutes > 55) {
		//Round up an hour
		nHours++;
		nMinutes = 0;
	}
	else
	{
		//If we are 6, then round up to 7, OR
		//Round up to the nearest number divisible by 5
		while (nMinutes != 7 && nMinutes % 5)
		{
			nMinutes++;
		}
	}

	//Calculate what we'll set the hours at
	// (v.maida 2016-02-02 12:22) - PLID 65626 - Removed the 8 option from the Hours dropdown.
	if (nHours >= 8)
	{
		//This is the highest we can go
		nHours = 7;
		nMinutes = 55;
	}
}

// (j.politis 2015-05-18 13:30) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
bool CFirstAvailableAppt::GetDefaultSearchInterval(COleDateTimeSpan &dts)
{
	// (j.jones 2006-08-01 16:31) - PLID 21438 - check the preference before estimating appt. durations
	// (c.haag 2007-02-20 10:20) - PLID 24567 - We now have three options: Calculate based on previous
	// appointments, calculate based on default durations, and don't calculate at all.
	const long nDefaultPref = (1 == GetRemotePropertyInt("FFA_EstimateAverageApptLength", 1, 0, "<None>", true)) ? 0 : 2;

	switch (GetRemotePropertyInt("FFA_CalcDefaultApptLength", nDefaultPref, 0, "<None>", true)) {
	default:
	case 0: // Check the average length of past appointments
		// (j.gruber 2010-06-07 13:29) - PLID 38854 - check to see what GetSelectAppointmentPurpose returns
	{
		long nPurposeID = GetSelectedAppointmentPurpose();
		if (nPurposeID == -1) {
			return false;
		}
		if (!EstimateApptDuration(nPurposeID, dts)) {
			return false;
		}
		// (c.haag 2007-02-20 17:03) - PLID 24829 - Round up 5 minutes
			{
				int nTotalMinutes = (int)dts.GetTotalMinutes();
				while (nTotalMinutes % 5) nTotalMinutes++;
				dts = COleDateTimeSpan(0, 0, nTotalMinutes, 0);
				return true;
			}
	}
	case 1: // Calculate the default duration
		return !!CalculateDefaultDuration(dts);
	case 2: // Do not assign a default length
		return false;
	}
}

//(s.dhole 5/12/2015 2:48 PM ) - PLID 66144 update function to datalist 2
void CFirstAvailableAppt::OnSelChosenPatientListCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}
		
		//TES 1/14/2010 - PLID 36762 - If this patient is blocked, then there's no point looking up a slot for them, because
		// we can't schedule an appointment for them anyway.
		long nPatientID = VarLong(pRow->GetValue(pccID));
		if(!GetMainFrame()->CanAccessPatient(nPatientID, false)) {
			pRow->PutValue(pccID, m_nPatientID);
		}
		else {
			m_nPatientID = nPatientID;
		}

		// (b.savon 2013-07-26 16:56) - PLID 50524 - Populate the resources if the preference is set.
		// (v.maida 2016-03-23 15:58) - PLID 66125 - Only change the resource selection after changing a patient if the default resource preference
		// is set to use the patient's General 1 provider as a resource, otherwise don't change the current resource.
		IRowSettingsPtr pDefaultResourceSel = m_FFADefaultResourceCombo->GetCurSel();
		if (pDefaultResourceSel)
		{
			long nDefaultResourceID = VarLong(pDefaultResourceSel->GetValue(fdrcID), FFA_DEFAULT_CURRENT_SCHEDULER_RESOURCE);
			if (nDefaultResourceID == FFA_USE_PATIENT_GENEREAL1_PROVIDER_AS_RESOURCE)
			{
				SelectResources();
			}
		}
		
		
		ValidatePtFilter();

		// (r.gonet 2014-11-19) - PLID 64173 - They selected a new patient. We don't want the old patient's insured
		// parties to still be there though, so reload the insured parties dropdown with the current patient's insured 
		// parties.
		ReloadInsuredPartyCombo(false);

	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-05-16) PLID 41013 - Clears out any values that were previously remembered for preselecting on a revised search
//(a.wilson 2012-3-28) PLID 49245 - set the struct for overriding defaults to false.
void CFirstAvailableAppt::ResetPreselects()
{
	m_bDefaultOverrides.SetAllTo(false);
	m_dtPreselectStart.m_dt = COleDateTime::GetCurrentTime();
	m_nPreselectPatientID = -1;
	m_nPreselectApptTypeID = -1;
	m_nPreselectApptPurposeID = -1;
	// (r.gonet 2014-11-19) - PLID 64173 - Forget the remembered insured party ID
	m_nPreselectInsuredPartyID = -1;
	m_nPreselectDurHours = 0;
	m_nPreselectDurMins = 0;
	m_nPreselectIntervalMins = 0;
	//(s.dhole 5 / 22 / 2015 12:51 PM) - PLID 65621
	m_nPreselectResourceTypeSelection = FFA_DEFAULT_ANY_RESOURCE;
	m_aryPreselectResources.RemoveAll();
	m_aryPreselectWeekDays.RemoveAll();
	m_aryPreselectTimes.RemoveAll();
	// (r.farnworth 2016-01-27 12:28) - PLID 65638 - New ability to open find first available from an appointment in the rescheduling queue.	
	m_aryPreselectLocations.RemoveAll();
	m_bPreselectAnyTimeChecked = FALSE;
	m_bPreselectAnyResource = FALSE;//(s.dhole 5/15/2015 9:42 AM ) - PLID  65624 Default value is false
	m_bPreselectOfficeHoursOnly = FALSE;

	m_bPreselectStartDateRadio = FALSE;
	m_bPreselectUserEditedDuration = FALSE;
	// (r.gonet 2014-12-17) - PLID 64428 - Reset the availability lookup and remembered settings.
	//(s.dhole 6/2/2015 8:42 AM ) - PLID  65643 Remove functionality
	/*m_setAvailability.clear();
	m_pCurrentSettings.reset();
	m_pOriginalSettings.reset();*/

}



// (z.manning 2013-07-26 13:59) - PLID 32160
//(s.dhole 5/14/2015 11:37 AM ) - PLID  65623 
void CFirstAvailableAppt::ToggleResourcesSelections()
{
	if (GetResourceComboSelectedValue() == FFA_MULTIPLE_SCHEDULER_RESOURCE &&  m_arySelectedResources.GetCount() > 1)
	{
		
		
		CArray<long, long> aryResources;
		CString strTemp = "";
		//(s.dhole 5/14/2015 5:06 PM ) - PLID 	65623 Change Code to support new structure
		for (int i = 0; i < m_arySelectedResources.GetCount(); i++)
		{
			long nResourceID = m_arySelectedResources[i];
			IRowSettingsPtr pRow = m_Resources->FindByColumn(fdrcID, nResourceID, NULL, VARIANT_FALSE);
			if (pRow)
			{
				strTemp += VarString(pRow->GetValue(fdrcItem), "") ; 
				strTemp += ", ";
				// another set of check if item is exist in dropdown
				aryResources.Add(VarLong(pRow->GetValue(fdrcID)));
			}
		}
		if (!strTemp.IsEmpty())
		{
			strTemp.TrimRight(", ");
		}
		m_nxlUnselectedResourceLabel.SetText(strTemp);
		
		// (v.maida 2016-03-21 08:57) - PLID 65621 - Nothing has been selected, so switch to Current Scheduler Resource 
		if (m_arySelectedResources.GetCount() == 0)
		{
			SetResourceComboToSchedulerResource(aryResources);
		}
		else
		{
			HandelMultiResourceLabel(FALSE);
		}
		m_arySelectedResources.Copy(aryResources);
	}
	else
	{
		m_nxlUnselectedResourceLabel.SetText("");
		HandelMultiResourceLabel(FALSE);
		if (GetResourceComboSelectedValue() != FFA_DEFAULT_ANY_RESOURCE)
		{
			long nResourceID = m_arySelectedResources[0];
			m_Resources->FindByColumn(fdrcID, nResourceID, NULL, VARIANT_TRUE);
			// (v.maida 2016-03-23 16:15) - PLID 65623 - Store last selected resource ID
			m_nLastResourceID = nResourceID;
		}
		else
		{
			// set any resource
			SetResourceComboToAnyResource(m_arySelectedResources);
		}
	}
	MangeResourceFilterOR_AND();
	SetGoStatus();
}



// (r.gonet 2014-11-19) - PLID 64173 - Reload the insured party dropdown with the current patient's
// insured parties. Useful when the dialog is first loaded and also when they switch patients.
// bUsePreSelection causes us to try to select the remembered insured party from last time (in case
// the user pressed Exit & New Search in the results list.
void CFirstAvailableAppt::ReloadInsuredPartyCombo(bool bUsePreSelection)
{
	// (r.gonet 2014-11-19) - PLID 64173 - The from clause didn't really need to be in this function
	// but it looks neater when used since it encapsulates the entire loading of the combo rather than
	// require the caller to ensure the from clause is set.

	// (r.gonet 2014-11-19) - PLID 64173 - Basically, just grab all the patient's
	// insured parties and show them in this dropdown. We also want to show the responsibility type name
	// and category too.
	// (r.gonet 2014-12-10) - PLID 64174 - Added CategoryID
	m_dlInsuredPartyCombo->FromClause = _bstr_t(FormatString(
		"InsuredPartyT "
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"INNER JOIN "
		"( "
		"	SELECT "
		"		CategoryType AS ID, "
		"		CASE WHEN CategoryType = %li THEN 'Medical' "
		"			WHEN CategoryType = %li THEN 'Vision' "
		"			WHEN CategoryType = %li THEN 'Auto' "
		"			WHEN CategoryType = %li THEN 'Workers'' Comp' "
		"			WHEN CategoryType = %li THEN 'Dental' "
		"			WHEN CategoryType = %li THEN 'Study' "
		"			WHEN CategoryType = %li THEN 'Letter of Protection' "
		"			WHEN CategoryType = %li THEN 'Letter of Agreement' "
		"			ELSE '< Unknown Type! >' "
		"		END AS Name "
		"	FROM RespTypeT "
		"	WHERE CategoryType >= %li AND CategoryType <= %li "
		"	GROUP BY CategoryType "
		") InsuranceCategoryQ ON RespTypeT.CategoryType = InsuranceCategoryQ.ID ",
		RespCategoryType::rctMedical,
		RespCategoryType::rctVision,
		RespCategoryType::rctAuto,
		RespCategoryType::rctWorkersComp,
		RespCategoryType::rctDental,
		RespCategoryType::rctStudy,
		RespCategoryType::rctLOP,
		RespCategoryType::rctLOA,
		RespCategoryType::rctMedical, RespCategoryType::rctLOA));
	// (r.gonet 2014-11-19) - PLID 64173 - Filter on the current patient. Show only the primary category placement insured parties.
	m_dlInsuredPartyCombo->WhereClause = _bstr_t(FormatString("InsuredPartyT.PatientID = %li AND InsuredPartyT.RespTypeID <> -1 AND RespTypeT.CategoryPlacement = 1", m_nPatientID));
	m_dlInsuredPartyCombo->Requery();

	// (r.gonet 2014-11-19) - PLID 64173 - Need a row in case they want to either not filter on insurance
	// or if they want to find a slot for a patient resp appt.
	NXDATALIST2Lib::IRowSettingsPtr pNoInsuranceRow = m_dlInsuredPartyCombo->GetNewRow();
	pNoInsuranceRow->PutValue((short)EInsuredPartyComboColumns::ID, _variant_t(-1L, VT_I4));
	pNoInsuranceRow->PutValue((short)EInsuredPartyComboColumns::RespType, _bstr_t(""));
	pNoInsuranceRow->PutValue((short)EInsuredPartyComboColumns::InsuranceCoID, _variant_t(-1L, VT_I4));
	pNoInsuranceRow->PutValue((short)EInsuredPartyComboColumns::InsuranceCoName, _bstr_t(" {No Insurance}"));
	pNoInsuranceRow->PutValue((short)EInsuredPartyComboColumns::Priority, _variant_t(-2L, VT_I4));
	// (r.gonet 2014-12-10) - PLID 64174 - Added CategoryID, needed by the FFA results list when creating appointments to fill the insurance fields.
	pNoInsuranceRow->PutValue((short)EInsuredPartyComboColumns::CategoryID, _variant_t(-2L, VT_I4));
	pNoInsuranceRow->PutValue((short)EInsuredPartyComboColumns::CategoryName, _bstr_t(""));
	m_dlInsuredPartyCombo->AddRowSorted(pNoInsuranceRow, NULL);

	NXDATALIST2Lib::IRowSettingsPtr pNewSel = NULL;
	if (bUsePreSelection) {
		// (r.gonet 2014-11-19) - PLID 64173 - Try to select the rule we remembered from last time.
		pNewSel = m_dlInsuredPartyCombo->FindByColumn((short)EInsuredPartyComboColumns::ID, _variant_t(m_nPreselectInsuredPartyID, VT_I4), m_dlInsuredPartyCombo->GetFirstRow(), VARIANT_TRUE);
	}
	if (pNewSel == NULL)
	{
		// Either we are not doing another search based on a first search or we somehow failed to fill in the old insured party.
		// (r.gonet 2014-11-19) - PLID 64173 - Select the row based on the preferences. Note that these are both user preferences.
		bool bAutoFillApptInsurance = !!GetRemotePropertyInt("AutoFillApptInsurance", FALSE, 0, GetCurrentUserName(), false);
		long nDefaultCategory = GetRemotePropertyInt("AutoFillApptInsurance_DefaultCategory", -1, 0, GetCurrentUserName(), false);
		NXDATALIST2Lib::IRowSettingsPtr pNewSel = NULL;
		if (bAutoFillApptInsurance && nDefaultCategory != -1) {
			// (r.gonet 2014-11-19) - PLID 64173 - Select the insured party with the category specified by the preference.
			pNewSel = m_dlInsuredPartyCombo->FindByColumn((short)EInsuredPartyComboColumns::CategoryID, _variant_t(nDefaultCategory, VT_I4), m_dlInsuredPartyCombo->GetFirstRow(), VARIANT_TRUE);
			if (pNewSel == NULL) {
				if (pNoInsuranceRow->GetNextRow() != NULL) {
					// (r.gonet 2015-01-23) - PLID 64173 - Select the next primary insured party based on priority.
					pNewSel = pNoInsuranceRow->GetNextRow();
					m_dlInsuredPartyCombo->CurSel = pNewSel;
				} else {
					// (r.gonet 2015-01-23) - PLID 64173 - They have no primary insured parties. We'll select the no insurance row below.
				}
			}
		}
		if(pNewSel == NULL) {
			// (r.gonet 2014-11-19) - PLID 64173 - So this could happen if they don't have the default insurance category preference set or if
			// they just don't have any primary insured party. If these happen, select No Insurance.
			m_dlInsuredPartyCombo->SetSelByColumn((short)EInsuredPartyComboColumns::ID, _variant_t(-1L, VT_I4));
		}
	}
}

// (b.savon 2013-07-29 10:45) - PLID 50524 - Default resources function.
void CFirstAvailableAppt::SelectResources()
{
	
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_FFADefaultResourceCombo->GetCurSel();
		if (pRow)
		{
			//(s.dhole 5/15/2015 4:54 PM ) - PLID 66125
			LoadDefaultResourceComboValue(pRow);
		}
		else
		{
			// (v.maida 2016-03-21 17:03) - PLID 65621 - Use the Current Scheduler Resource, if no default resource row could be obtainted.
			m_arySelectedResources.Add(GetCurrentSchedulerResource());
		}
		ToggleResourcesSelections();
		//(s.dhole 5/27/2015 3:04 PM ) - PLID 65623
		// (b.cardillo 2016-01-22 13:51) - PLID 65623 - Changed back to 'Any'
		m_bSelectAnyResource = TRUE; //Default to 'Any'
		
		// Set the status of the Go button
		SetGoStatus();

		// Mark the 'start n days after last appointment of patient' radio
		// disabled if the patient has no appointments
		ValidatePtFilter();

		// (v.maida 2016-03-23 16:15) - PLID 65623 - Store last selected resource ID
		m_nLastResourceID = GetResourceComboSelectedValue();

		// Enable the search interval edit box
		// based on the Show Results in List check state.
		//(s.dhole 6/2/2015 9:04 AM ) - PLID 65643 Remove functionality
		//OnCheckResults();
	}NxCatchAll(__FUNCTION__);
}

LRESULT CFirstAvailableAppt::OnAutoRunFfa(WPARAM wParam, LPARAM lParam)
{
	try {
		//TES 12/3/2014 - PLID 64180 - Clear out the flag, and pop up the window
		m_bRunImmediately = false; 
		//TES 1/7/2015 - PLID 64466 - Call the shared function
		LaunchFirstAvailList(true);
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

//(s.dhole 6/1/2015 4:42 PM ) - PLID 65643 Remove functionality
// (r.gonet 2014-12-17) - PLID 64428 - Added. This function loads the availability lookup table for a given month.
//(s.dhole 6/2/2015 8:42 AM ) - PLID  65643 Remove functionality
//void CFirstAvailableAppt::LoadAvailabilityForMonth(long nYear, long nMonth, bool bForceLoad/*=false*/)
//{
//	if (m_pCurrentSettings == NULL || m_pOriginalSettings == NULL) {
//		// (r.gonet 2014-12-17) - PLID 64428 - The user hasn't searched FFA for anything. The lookup should be empty at this time. Ensure it and return.
//		m_setAvailability.clear();
//		return;
//	}
//
//	if (COleDateTime(nYear, nMonth, 1, 0, 0, 0).GetStatus() != COleDateTime::valid) {
//		ThrowNxException("%s : Invalid year or month passed as arguments.", __FUNCTION__);
//	}
//
//	if (m_pCurrentSettings->m_dtStartDate.GetStatus() != COleDateTime::valid) {
//		ThrowNxException("%s : Invalid start date in current settings.", __FUNCTION__);
//	}
//	if (m_pOriginalSettings->m_dtStartDate.GetStatus() != COleDateTime::valid) {
//		ThrowNxException("%s : Invalid start date in original settings.", __FUNCTION__);
//	}
//
//	if (nYear == m_pCurrentSettings->m_dtStartDate.GetYear()
//		&& nMonth == m_pCurrentSettings->m_dtStartDate.GetMonth()
//		&& !bForceLoad) {
//		// Short circuit. The month didn't change. No need to do anything because we should already have the availability lookup filled.
//		// bForceLoad should be specified on the initial search, so the availability lookup gets initialized.
//		return;
//	} else {
//		// (r.gonet 2014-12-17) - PLID 64428 - Clear the cache since that was for a different month.
//		m_setAvailability.clear();
//
//		// (r.gonet 2014-12-17) - PLID 64428 - We need to search FFA again but this time for the month specified in the arguments.
//		// We don't know the start date of the search yet. This next bit gets the end date of the search though.
//		
//		COleDateTime dtLastDayOfMonth;
//		long nYearOfNextMonth = (nYear * 12 + (nMonth - 1) + 1) / 12;
//		long nNextMonth = ((nYear * 12 + (nMonth - 1) + 1) % 12) + 1;
//		dtLastDayOfMonth.SetDate(
//			nYearOfNextMonth,
//			nNextMonth,
//			1);
//		dtLastDayOfMonth -= COleDateTimeSpan(1, 0, 0, 0);
//
//		bool bPastMonth = false;
//		if (nYear < m_pOriginalSettings->m_dtStartDate.GetYear()
//			|| (nYear == m_pOriginalSettings->m_dtStartDate.GetYear()
//			&& nMonth < m_pOriginalSettings->m_dtStartDate.GetMonth())) {
//			// (r.gonet 2014-12-17) - PLID 64428 - Past months (relative to the original start date) return no openings. For the purpose of logic, we need to set the start date though as if we are
//			// querying for this month.
//			bPastMonth = true;
//			m_pCurrentSettings->m_dtStartDate.SetDate(dtLastDayOfMonth.GetYear(), dtLastDayOfMonth.GetMonth(), 1);
//			m_pCurrentSettings->m_nDaysToSearch = dtLastDayOfMonth.GetDay();
//		} else if (nMonth == m_pOriginalSettings->m_dtStartDate.GetMonth()
//			&& nYear == m_pOriginalSettings->m_dtStartDate.GetYear()) {
//			// (r.gonet 2014-12-17) - PLID 64428 - Same month as the original search, use the original start date, which is somewhere in the month.
//			m_pCurrentSettings->m_dtStartDate = m_pOriginalSettings->m_dtStartDate;
//			m_pCurrentSettings->m_nDaysToSearch = dtLastDayOfMonth.GetDay() - m_pOriginalSettings->m_dtStartDate.GetDay() + 1;
//		} else {
//			// (r.gonet 2014-12-17) - PLID 64428 - Future month. We should search all the month.
//			m_pCurrentSettings->m_dtStartDate.SetDate(dtLastDayOfMonth.GetYear(), dtLastDayOfMonth.GetMonth(), 1);
//			m_pCurrentSettings->m_nDaysToSearch = dtLastDayOfMonth.GetDay();
//		}
//		// (r.gonet 2014-12-17) - PLID 64428 - Resource map settings depend on the start date. Rebuild them.
//		CreateResourceMaps(m_pCurrentSettings);
//		
//		if (bPastMonth) {
//			// (r.gonet 2014-12-17) - PLID 64428 - We already know there will be no results with past months,
//			// so short circuit.
//			return;
//		}
//
//		// (r.gonet 2014-12-17) - PLID 64428 - Run the FFA with the settings we have now.
//		CWaitCursor wc;
//		shared_ptr<CFFASearchThread> pFFASearch = make_shared<CFFASearchThread>();
//		// (r.gonet 2014-12-17) - PLID 64428 - We don't want the thread notifying any windows with messages, since this
//		// will be synchronous. You may be asking why this is synchronous. Why not let the thread run async? Well, because 1) it
//		// is more complicated to have the month view be repainted when the thread terminates. But more importantly because
//		// 2) look at the options we have for what to do with an async background thread that may take several seconds to complete.
//		// We could somehow iteratively repaint day backgrounds as we get FFA day results back in. This improves responsiveness, but 
//		// this reveals an actionable, intermediate state to the user, which is undesirable.
//		// We could let the thread run async and then *BAM* repaint the month all at once. But this may be several seconds later
//		// and it is not obvious that something is happening behind the scenes.
//		// So we block the UI thread for a little bit and throw up a wait cursor indicating a time consuming operation is occurring.
//		pFFASearch->Run(NULL, m_pCurrentSettings);
//		if (!pFFASearch->WaitUntilFinished()) {
//			ThrowNxException("%s : Find First Available failed before it finished.", __FUNCTION__);
//		}
//
//		// (r.gonet 2014-12-17) - PLID 64428 - Load the availability lookup using the time slots
//		// returned by the FFA. A day is "available" for a resource if there is at least one slot
//		// returned by FFA for that day, for that resource.
//		for (size_t i = 0; i < pFFASearch->m_vecFFAResultSet.size(); i++) {
//			FFAOpening *pOpening = pFFASearch->m_vecFFAResultSet[i];
//			COleDateTime dtNoTime;
//			dtNoTime.SetDate(pOpening->dt.GetYear(), pOpening->dt.GetMonth(), pOpening->dt.GetDay());
//			CDWordArray aryIds;
//			ParseCommaDeliminatedText(aryIds, pOpening->strResourceIDs);
//			for (long j = 0; j < aryIds.GetSize(); j++) {
//				std::pair<COleDateTime, long> key;
//				key.first = dtNoTime;
//				key.second = aryIds[j];
//				std::set<std::pair<COleDateTime, long> >::iterator it;
//				// (r.gonet 2014-12-17) - PLID 64428 - One value of sets is that they are unique. The insert will not add duplicates.
//				m_setAvailability.insert(key);
//			}
//		}
//	}
//}

//TES 1/7/2015 - PLID 64466 - Moved code to a shared function
void CFirstAvailableAppt::LaunchFirstAvailList(bool bReturnSlot)
{
	//(e.lally 2005-10-21) PLID 17128 - Put the FFA into an ad-hoc procedure to fix the way it searched
	//so we needed to add more variables and store preferences differently to send the preferences 
	//into that procedure.
	CString strItems;
	CString strSql;
	// (z.manning 2010-10-28 15:25) - PLID 41177 - This is now an array
	CArray<long, long> arynWeekDayPrefList;
	CArray<long, long> arynResourceIDs;
	// (r.gonet 2014-12-17) - PLID 64428 - Store the resource names too.
	CArray<CString, CString&> aryResourceNames;
	CString strTempHours = "";
	CString strResourceNameList = "";
	int nHours, nMinutes;
	CDWordArray adwResources;

	// (z.manning 2011-06-17 12:32) - PLID 43769 - Check if the validation fails and if so don't continue.
	if (!UpdateData(TRUE)) {
		return;
	}
	////(s.dhole 6/2/2015 9:04 AM ) - PLID 65643 Remove functionality
	//if (((CButton*)GetDlgItem(IDC_CHECK_RESULTS))->GetCheck())
	{
		if (m_iSearchInterval < 5)
		{
			MsgBox("Please enter a search interval greater than or equal to five minutes");
			GetDlgItem(IDC_EDIT_SEARCH_INTERVAL)->SetFocus();
			((CNxEdit*)GetDlgItem(IDC_EDIT_SEARCH_INTERVAL))->SetSel(0, -1);
			// (c.haag 2006-11-21 13:27) - PLID 23623 - Make sure all fields are visible
			ShowMoreFields();
			return;
		}
		else if (m_iSearchInterval >= 60 * 8)
		{
			MsgBox("Please enter a search interval less than eight hours");
			GetDlgItem(IDC_EDIT_SEARCH_INTERVAL)->SetFocus();
			((CNxEdit*)GetDlgItem(IDC_EDIT_SEARCH_INTERVAL))->SetSel(0, -1);
			// (c.haag 2006-11-21 13:27) - PLID 23623 - Make sure all fields are visible
			ShowMoreFields();
			return;
		}
	}

	// (j.politis 2015-05-18 13:02) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
	//Get the hours and the mins in int
	nHours = GetDropdownHours();
	nMinutes = GetDropdownMinutes();

	if (nHours == 0 && nMinutes == 0){
		MsgBox("Please select an appointment length that is greater than zero.");
		// (c.haag 2006-11-21 13:27) - PLID 23623 - Make sure all fields are visible
		ShowMoreFields();
		return;
	}

	long nDaysToSearch = GetDlgItemInt(IDC_EDIT_SEARCH_DAYS);
	//only validate if we are searching in a text list
	//(s.dhole 6/2/2015 9:04 AM ) - PLID 65643 Remove functionality
	//if (((CButton*)GetDlgItem(IDC_CHECK_RESULTS))->GetCheck()) 
	{
		if (nDaysToSearch <= 0) {
			MsgBox("You must search for at least one day into the future.");
			// (c.haag 2006-11-21 13:27) - PLID 23623 - Make sure all fields are visible
			ShowMoreFields();
			return;
		}
		else if (nDaysToSearch > 365) {
			MsgBox("You may not search for more than 365 days into the future.");
			// (c.haag 2006-11-21 13:27) - PLID 23623 - Make sure all fields are visible
			ShowMoreFields();
			return;
		}
	}

	// Store the first selected resource name
	//(s.dhole 5/14/2015 5:06 PM ) - PLID 66125 Change Code to support new structure
	for (int i = 0; i < m_arySelectedResources.GetCount(); i++)
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_Resources->FindByColumn(fdrcID, m_arySelectedResources[i], NULL, VARIANT_FALSE);
		if (pRow)
		{
			m_strFirstSelectedResource = VarString(pRow->GetValue(fdrcItem), "");
			break;
		}
		
	}

	// Store the appointment type and purpose
	//(s.dhole 5/20/2015 11:05 AM ) - PLID 66144
	IRowSettingsPtr pRow = m_dlAptType->GetCurSel();
	if (!pRow){
		m_nAptType = -1;
	}
	else{
		m_nAptType = VarLong(pRow->GetValue(atcID), -1);
	}

	pRow = m_dlAptPurpose->GetCurSel();
	if (!pRow){
		m_nAptPurpose = -1;
	}
	else{
		m_nAptPurpose = VarLong(pRow->GetValue(apcID), -1);
	}

	//
	// (c.haag 2006-08-01 12:11) - PLID 21542 - Warn the user if they want to show results
	// in a text list and require appointment types or purposes; but don't have a type or
	// purpose selected
	//
	//(s.dhole 6/1/2015 4:34 PM ) - PLID 65643 keep all other functionality considering  m_boTextResults=true
//	if (m_boTextResults) {
		// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
		if (-1 == m_nAptType && GetRemotePropertyInt("ResEntryRequireAptType", 1, 0, "<None>", true)) {
			if (IDNO == MsgBox(MB_YESNO, "No appointment type was specified in your search criteria; therefore, you will not be able to schedule an appointment "
				"from the search results list. Do you wish to continue?"))
			{
				return;
			}
		}
		// Don't warn them twice
		// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
		else if (-1 == m_nAptPurpose && GetRemotePropertyInt("ResEntryRequireAptPurpose", 1, 0, "<None>", true)) {
			if (IDNO == MsgBox(MB_YESNO, "No appointment purpose was specified in your search criteria; therefore, you will not be able to schedule an appointment "
				"from the search results list. Do you wish to continue?"))
			{
				return;
			}
		}
	//}


	////////////////////////////////////////////////////////
	// Calculate the appointment length, and make sure it's 
	// mathematically possible to make an appointment.
	{
		long abAvailTimes[FFA_TIME_LIST_SIZE];
		memset(abAvailTimes, 0, sizeof(abAvailTimes));
		m_nAptLength = GetLengthInSlots(nHours, nMinutes, 0);

		for (int i = 0; i < m_TimePrefs.GetCount(); i++) {
			if (m_TimePrefs.GetSel(i)) {
				int hour_data = m_TimePrefs.GetItemData(i);
				int nStartSlot = GetSlotFromTime(hour_data, 0, 0);
				for (int iSlot = nStartSlot; iSlot < nStartSlot + FFA_SLOTS_PER_HOUR; iSlot++) {
					abAvailTimes[iSlot] = 1;
				}
			}
		}
		int nAvailSlots = 0;
		for (int j = 0; j < FFA_TIME_LIST_SIZE - m_nAptLength; j++) {
			if (abAvailTimes[j] > 0) {
				for (int i = j; i < j + m_nAptLength; i++) {
					if (abAvailTimes[i] == 0) break;
				}
				if (i == j + m_nAptLength) nAvailSlots++;
			}
		}
		// No available slot for current day. Mark as undoable.
		if (0 == nAvailSlots) {
			AfxMessageBox("You have chosen an appointment duration that cannot be made with your time preferences. Please adjust your time preferences or duration.");
			// (c.haag 2006-11-21 13:27) - PLID 23623 - Make sure all fields are visible
			ShowMoreFields();
			return;
		}
	}

	// (r.gonet 2014-12-17) - PLID 64428 - Removed the initialization of the time slot availabilities array. It was only used in the
	// old FFA implementation, which was removed.
	// Empty the maps
	//(s.dhole 6/2/2015 8:42 AM ) - PLID  65643 Remove functionality
	//RemoveAvailableDays();

	// Update our variables
	UpdateData();

	// Get the start and end dates
	CalculateStartDate();

	COleDateTime dtToday = COleDateTime::GetCurrentTime();
	dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());
	if (m_dtStartDate < dtToday){
		MsgBox("The start date you selected is in the past, \nthe search will continue with a start date of today");
		m_dtStartDate = dtToday;
	}

	// Mark days of week as checked or not
	//Enter it into the array for the calendar highlighting
	m_aResWeekdays[0] = ((CButton*)GetDlgItem(IDC_CHECK_SUN))->GetCheck();
	m_aResWeekdays[1] = ((CButton*)GetDlgItem(IDC_CHECK_MON))->GetCheck();
	m_aResWeekdays[2] = ((CButton*)GetDlgItem(IDC_CHECK_TUE))->GetCheck();
	m_aResWeekdays[3] = ((CButton*)GetDlgItem(IDC_CHECK_WED))->GetCheck();
	m_aResWeekdays[4] = ((CButton*)GetDlgItem(IDC_CHECK_THU))->GetCheck();
	m_aResWeekdays[5] = ((CButton*)GetDlgItem(IDC_CHECK_FRI))->GetCheck();
	m_aResWeekdays[6] = ((CButton*)GetDlgItem(IDC_CHECK_SAT))->GetCheck();

	//These strings are harcoded for the default SQL @@DATEFIRST value
	// (z.manning 2013-11-18 13:54) - PLID 58756 - Now that we use the API, this is zero-based
	if (((CButton*)GetDlgItem(IDC_CHECK_SUN))->GetCheck()){
		arynWeekDayPrefList.Add(0);
	}
	if (((CButton*)GetDlgItem(IDC_CHECK_MON))->GetCheck()){
		arynWeekDayPrefList.Add(1);
	}
	if (((CButton*)GetDlgItem(IDC_CHECK_TUE))->GetCheck()){
		arynWeekDayPrefList.Add(2);
	}
	if (((CButton*)GetDlgItem(IDC_CHECK_WED))->GetCheck()){
		arynWeekDayPrefList.Add(3);
	}
	if (((CButton*)GetDlgItem(IDC_CHECK_THU))->GetCheck()){
		arynWeekDayPrefList.Add(4);
	}
	if (((CButton*)GetDlgItem(IDC_CHECK_FRI))->GetCheck()){
		arynWeekDayPrefList.Add(5);
	}
	if (((CButton*)GetDlgItem(IDC_CHECK_SAT))->GetCheck()){
		arynWeekDayPrefList.Add(6);
	}

	if (arynWeekDayPrefList.GetSize() == 0) {
		MsgBox("Please select at least one day of the week in the Day Preferences");
		// (c.haag 2006-11-21 13:27) - PLID 23623 - Make sure all fields are visible
		ShowMoreFields();
		return;
	}

	// (r.gonet 2014-12-17) - PLID 64428 - Moved the assignment of the SQL statement to get appointments after the start date
	// into the CreateResourceMaps function. There was no reason we needed to define the SQL outside of that function, and it
	// complicated using the function's reuse.

	///////////////////////
	// DRT 12/16/2004 - PLID 14968
	// Save some settings that they probably use repeatedly
	{	//Exclude Available times on template warnings
		UINT nExclude = IsDlgButtonChecked(IDC_CHECK_EXCLUDE_WARNINGS);
		SetRemotePropertyInt("FFA_ExcludeAvailOnWarnings", nExclude, 0, GetCurrentUserName());
	}

	//only save if we are searching in a text list, which is when it would have been validated
	//(s.dhole 6/2/2015 9:04 AM ) - PLID 65643 Remove functionality
	//if (((CButton*)GetDlgItem(IDC_CHECK_RESULTS))->GetCheck()) {
		SetRemotePropertyInt("FFA_DaysToSearch", nDaysToSearch, 0, GetCurrentUserName());
	//}

	// Done saving settings
	///////////////////////

	// (r.gonet 2014-12-17) - PLID 64428 - All of the search settings that used to be member variables of CFirstAvailList are now
	// members of CFFASearchSettings. We used to only need to reference these settings in the CFirstAvailList but now that the
	// non-text list mode's uses the API implementation, we need these settings elsewhere. Also, in the non-text list mode, we'll 
	// need to reference them later when there is not a window up.
	CFFASearchSettingsPtr pSettings = make_shared<CFFASearchSettings>();

	pSettings->m_nHours = nHours;
	pSettings->m_nMinutes = nMinutes;
	pSettings->m_nArrivalMins = GetDefaultArrivalMins();
	pSettings->m_dtStartDate = m_dtStartDate;
	pSettings->m_nAptType = m_nAptType;
	pSettings->m_nAptPurpose = m_nAptPurpose;
	pSettings->m_dwPatientID = m_nPatientID;
	// (r.farnworth 2015-06-08 10:47) - PLID 65635
	pSettings->m_arynAptLocations.Copy(m_varLocIDs);
	//(s.dhole 5/15/2015 9:52 AM ) - PLID 65624
	pSettings->m_bAnyOpenResource = m_bSelectAnyResource;
	//(e.lally 2011-05-16) PLID 41013
	pSettings->m_bAnyTimeChecked = ((CButton*)GetDlgItem(IDC_CHECK_ANY_TIME))->GetCheck();
	pSettings->m_bStartDateRadioChecked = m_boStartFromRadio == 0 ? TRUE : FALSE;
	pSettings->m_bUserEditedDuration = m_bUserEditedDuration;

	pSettings->m_bUseTemplateRules = TRUE;
	pSettings->m_nOverBookingAllowed = m_iExtraBookingLimit;
	pSettings->m_bExcludeTemplatesWithWarnings = ((CButton*)GetDlgItem(IDC_CHECK_EXCLUDE_WARNINGS))->GetCheck();
	memcpy(pSettings->m_aResWeekdays, m_aResWeekdays, sizeof(m_aResWeekdays));
	pSettings->m_arynWeekDayPrefList.Copy(arynWeekDayPrefList);
	pSettings->m_bSearchOfficeHoursOnly = SearchOfficeHoursOnly();
	pSettings->m_nSearchIntervalMinutes = m_iSearchInterval;
	pSettings->m_nDaysToSearch = nDaysToSearch;

	//(s.dhole 5/20/2015 11:05 AM ) - PLID 66144
	IRowSettingsPtr pRowAptType = m_dlAptType->GetCurSel();
	if (!pRowAptType){
		m_nAptType = -1;
	}
	else{
		m_nAptType = VarLong(pRowAptType->GetValue(atcID), -1);
	}

	IRowSettingsPtr pRowAptPurpose = m_dlAptPurpose->GetCurSel();
	if (!pRowAptPurpose){
		m_nAptPurpose = -1;
	}
	else{
		m_nAptPurpose = VarLong(pRowAptPurpose->GetValue(apcID), -1);
	}

	
	// (r.gonet 2014-11-19) - PLID 64173 - Grab the selected insured party, responsibility type name, insurance company ID and name.
	NXDATALIST2Lib::IRowSettingsPtr pInsuredPartySelRow = m_dlInsuredPartyCombo->CurSel;
	if (pInsuredPartySelRow != NULL) {
		pSettings->m_pInsurance = make_shared<CFFAInsurance>();
		// (r.gonet 2014-12-10) - PLID 64174 - The FFA results list needs the insured party ID in order to fill in the insurance fields on new appointments created from it.
		pSettings->m_pInsurance->m_nInsuredPartyID = VarLong(pInsuredPartySelRow->GetValue((short)EInsuredPartyComboColumns::ID), -1);
		// (r.gonet 2014-12-10) - PLID 64173 - The FFA results list needs the insurance co ID in order to pass it to the FFA thread, which will call the API FFA search with it.
		pSettings->m_pInsurance->m_nInsuranceCoID = VarLong(pInsuredPartySelRow->GetValue((short)EInsuredPartyComboColumns::InsuranceCoID), -1);
		// (r.gonet 2014-12-10) - PLID 64174 - The FFA results list needs the insurance co name in order to fill in the insurance fields on new appointments created from it.
		pSettings->m_pInsurance->m_strInsuranceCoName = VarString(pInsuredPartySelRow->GetValue((short)EInsuredPartyComboColumns::InsuranceCoName), "");
		// (r.gonet 2014-12-10) - PLID 64174 - The FFA results list needs the responsibility type name in order to fill in the insurance fields on new appointments created from it.
		pSettings->m_pInsurance->m_strRespType = VarString(pInsuredPartySelRow->GetValue((short)EInsuredPartyComboColumns::RespType), "");
		// (r.gonet 2014-12-10) - PLID 64174 - The FFA results list needs the category ID in order to fill in the insurance fields on new appointments created from it.
		pSettings->m_pInsurance->m_nCategoryID = VarLong(pInsuredPartySelRow->GetValue((short)EInsuredPartyComboColumns::CategoryID), -1);
	}
	else {
		// (r.gonet 2014-11-19) - PLID 64173 - Nothing is selected! Oh.
		pSettings->m_pInsurance.reset();
	}

	/* Fill array of available times with hilited listbox time interval entries
	(these is the user-defined times of availability, defined by hilite). */
	for (int i = 0; i < FFA_TIME_LIST_SIZE; i++) {
		pSettings->m_abAvailTimes[i] = 0;
	}
	// (a.wilson 2014-02-04 16:01) - PLID 15410 - update to handle all hour selections.
	for (i = 0; i < 24; i++) {
		pSettings->m_anTimePrefList[i] = 0;
	}
	BOOL bTimeSelectFlag = FALSE;
	int nCurrentTimePos = 0;
	// (j.politis 2015-06-25 11:29) - PLID 65642 - Extend the time preferences listbox to go from midnight to midnight.
	for (i = 0; i < m_TimePrefs.GetCount(); i++) {
		pSettings->m_anTimePrefList[i] = m_TimePrefs.GetSel(i);

		if (bTimeSelectFlag == FALSE && m_TimePrefs.GetSel(i))
		{
			bTimeSelectFlag = TRUE;

		}
	}

	if (bTimeSelectFlag == FALSE) {
		MsgBox("Please select at least one time range from the Time Preferences");
		// (c.haag 2006-11-21 13:27) - PLID 23623 - Make sure all fields are visible
		ShowMoreFields();
		return;
	}

	long nNumResources = 0;
	//(s.dhole 5/14/2015 5:06 PM ) - PLID 66125 Change Code to support new structure
	for (int i = 0; i < m_arySelectedResources.GetCount(); i++)
	{
		long nResourceID = m_arySelectedResources[i];
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_Resources->FindByColumn(fdrcID, nResourceID, NULL, VARIANT_FALSE);
		if (pRow)
		{
			arynResourceIDs.Add(VarLong(pRow->GetValue(fdrcID)));
			CString strItemName = VarString(pRow->GetValue(fdrcItem));
			// (r.gonet 2014-12-17) - PLID 64428 - Remember the resource name too.
			aryResourceNames.Add(strItemName);
			strResourceNameList += strItemName + "; ";
			nNumResources++;
			pSettings->m_adwAptResources.Add(VarLong(pRow->GetValue(fdrcID)));
		}
	}
	//(s.dhole 5 / 22 / 2015 12:51 PM) - PLID 66125
	pSettings->m_nResourceTypeSelection = GetResourceComboSelectedValue();
	
	strResourceNameList.TrimRight("; ");
	//set the variable for the available list dialog
	pSettings->m_arynResourceIDs.Copy(arynResourceIDs);
	pSettings->m_aryResourceNames.Copy(aryResourceNames);
	pSettings->m_strResourceNameList = strResourceNameList;
	pSettings->m_nNumResources = nNumResources;

	// (r.gonet 2014-12-17) - PLID 64428 - Moved the resource map fill a little earlier than it was before.
	//(s.dhole 5/14/2015 5:06 PM ) - PLID 	66125 Change Code to support new structure
	//(s.dhole 6/2/2015 8:42 AM ) - PLID  65643 Remove functionality
	//for (int i = 0; i < m_arySelectedResources.GetCount(); i++)
	//{
	//	long nResourceID = m_arySelectedResources[i];
	//	NXDATALIST2Lib::IRowSettingsPtr pRow = m_Resources->FindByColumn(fdrcID, nResourceID, NULL, VARIANT_FALSE);
	//	if (pRow)
	//	{
	//		CString strItem = VarString(pRow->GetValue(fdrcItem));
	//		m_mapResources[strItem] = TRUE;
	//	}
	//}

	// (r.gonet 2014-12-17) - PLID 64428 - Instead of passing like 5 member variables of pSettings into the resource maps creation function,
	// just pass pSettings.

	CreateResourceMaps(pSettings);
	// (r.gonet 2014-12-17) - PLID 64428 - Remember these settings as the ones we are using currently for FFA.
	//(s.dhole 6/2/2015 8:42 AM ) - PLID  65643 Remove functionality
	//m_pCurrentSettings = pSettings;
	// (r.gonet 2014-12-17) - PLID 64428 - This is for the non-text list mode. In that mode, we'll be searching the FFA every time
	// the user changes the month, but using the same search settings (with different start date and num days to search).
	// We'll update m_pCurrentSettings each time. However, we need to reference back to what the original settings were. So copy the settings
	// into the following member variable, m_pOriginalSettings.
	//(s.dhole 6/2/2015 8:42 AM ) - PLID  65643 Remove functionality
	//m_pOriginalSettings = make_shared<CFFASearchSettings>(const_cast<const CFFASearchSettings&>(*m_pCurrentSettings.get()));

	//(s.dhole 6/1/2015 4:37 PM ) - PLID 65643 We will remove this functionality where m_boTextResults=false
	//if (!m_boTextResults)
	//{
	//	// (r.gonet 2014-12-17) - PLID 64428 - Non-text list mode, which is by month. Search FFA and load an availability 
	//	// lookup table that is keyed by the date and resource. If a resource has slots available for that date, thne it will be
	//	// in the availability lookup table. Force the lookup table load. It would not otherwise because the start date is the same as
	//	// the current settings date.
	//	LoadAvailabilityForMonth(pSettings->m_dtStartDate.GetYear(), pSettings->m_dtStartDate.GetMonth(), true);
	//}
	//else 
	{
		// (z.manning 2011-06-15 09:54) - PLID 44120 - The first available dialog object is now a pointer
		CFirstAvailList *pdlgAvailList = GetMainFrame()->GetFirstAvailList();

		// (c.haag 2006-11-27 11:20) - PLID 20772 - Make sure the results list
		// is empty before populating it with new information
		//TES 1/20/2015 - PLID 64513 - Hide the window if it's visible.
		pdlgAvailList->ShowWindow(SW_HIDE);
		pdlgAvailList->Clear();
		// (r.gonet 2014-12-17) - PLID 64465 - Initialize its search settings.
		pdlgAvailList->SetSearchSettings(pSettings);
		//TES 1/7/2015 - PLID 64466 - Pass in whether we're returning a slot, and if we are, set the slot and a pointer to ourselves as well
		pdlgAvailList->m_bReturnSlot = bReturnSlot;
		pdlgAvailList->m_bReSchedule = m_bReSchedule;
		if (m_bReturnSlot) {
			m_UserSelectedSlot = SelectedFFASlotPtr(new SelectedFFASlot);
			pdlgAvailList->m_UserSelectedSlot = m_UserSelectedSlot;
			pdlgAvailList->m_pCallingDlg = this;
		}
		// (c.haag 2006-11-27 12:20) - PLID 20772 - This is no longer modal
		if (bReturnSlot) {
			//TES 1/9/2015 - PLID 64513 - If we're returning a slot, fake like we're in modal mode (actually calling DoModal() causes all sorts of problems)
			GetMainFrame()->EnableWindow(FALSE);
			EnableWindow(FALSE);
		}
		pdlgAvailList->ShowWindow(SW_SHOW);
		//TES 1/9/2015 - PLID 64513 - Make sure it doesn't pop up behind us
		pdlgAvailList->BringWindowToTop();

		// Closing the FFA Results window while minimized causes future windows to open in an iconic state. Added logic to open it in its default restored state.
		if (pdlgAvailList->IsIconic()) {
			pdlgAvailList->ShowWindow(SW_RESTORE);
			pdlgAvailList->CenterWindow();
		}

		/*if (IDCANCEL == pdlgAvailList->DoModal()) {
		// (b.cardillo 2005-07-29 12:02) - PLID 17131 - If the user cancelled the results
		// listing, it's likely because he wants to search again with slightly different
		// criteria.  So don't close out of our FFA window.
		return;
		} else {
		CNxDialog::OnOK();
		return;
		}*/
	}
}

//TES 1/7/2015 - PLID 64466 - Sent when a slot is selected from the first available list
LRESULT CFirstAvailableAppt::OnCloseFirstAvailList(WPARAM wParam, LPARAM lParam)
{
	//TES 1/9/2015 - PLID 64513 - Undo our fake modal mode
	EnableWindow(TRUE);
	//TES 1/9/2015 - PLID 64513 - Make sure we're on top
	BringWindowToTop();
	//TES 1/7/2015 - PLID 64466 - Reset the relevant variables in the list dialog, so as to make sure it doesn't think it's returning a slot next time it's called.
	CFirstAvailList *pdlgAvailList = GetMainFrame()->GetFirstAvailList();
	pdlgAvailList->m_bReturnSlot = false;
	pdlgAvailList->m_UserSelectedSlot = NULL;
	pdlgAvailList->m_pCallingDlg = NULL;
	m_nNewResID = pdlgAvailList->m_nNewResID;

	//TES 1/9/2015 - PLID 64513 - lParam whether or not to keep this dialog open
	switch ((FirstAvailListCloseAction)lParam) {
	case falcaOK:
		//TES 1/12/2015 - PLID 64180 - Make sure we're not returning a slot in future manual calls.
		m_bReturnSlot = false;
		m_bReSchedule = false;
		CNxDialog::OnOK();
		break;
	case falcaCancel:
		//TES 1/12/2015 - PLID 64180 - Make sure we're not returning a slot in future manual calls.
		m_bReturnSlot = false;
		m_bReSchedule = false;
		CNxDialog::OnCancel();
		break;
	case falcaNewSearch:
		//Do nothing
		//TES 1/12/2015 - PLID 64180 - The list just set the presets for a new search, but we didn't actually apply any of them, because we know they are the same
		// as the list was launched with. So, now that we've "applied" the presets, we need to reset them so they don't get used in a future search
		ResetPreselects();
		break;
	}
	return 0;
}



//(s.dhole 5/12/2015 3:41 PM ) - PLID 65620 
void CFirstAvailableAppt::OnSelChosenDefaultLocationCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow)
		{
			return;
		}
		long nDefaultlocation = VarLong(pRow->GetValue(fdlcID));
		SetRemotePropertyInt("FFA_DefaultLocationID", nDefaultlocation, 0, GetCurrentUserName());
	}NxCatchAll(__FUNCTION__);
}

//(s.dhole 5/12/2015 3:41 PM ) - PLID 65620 
long CFirstAvailableAppt::GetDefaultLocationComboValue(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// set default value
	long nDefaultlocation = VarLong(pRow->GetValue(fdlcID), FFA_DEFAULT_ANY_LOCATION);
	switch (nDefaultlocation)
	{
	case FFA_DEFAULT_ANY_LOCATION:
		// no locaton selected
		break;
	case FFA_DEFAULT_CURRENT_LOGEDIN_LOCATION:
		// Use current LoglinLocation
	{
		nDefaultlocation = GetCurrentLocationID();
	}
	break;
	case FFA_DEFAULT_CURRENT_SCHEDULER_LOCATION:
	{
		// Use current scheduler location
		CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
		if (pView)
		{
			long nTempLocation = -3;
			CNxSchedulerDlg *pSchedSheet = (CNxSchedulerDlg *)pView->GetActiveSheet();
			if (pSchedSheet)
			{
				nTempLocation = pSchedSheet->GetLocationId();
			}

			//nTempLocation=-2  MEAN we have selected multiple filter at scheduler  , so will used logged-in  location
			// (r.farnworth 2015-05-20 10:55) - PLID 65625 - Default to currently logged in if all location is selected
			if (nTempLocation == -1 || nTempLocation == -2) // All location is selected
			{
				nDefaultlocation = GetCurrentLocationID();
			}
			else
			{
				nDefaultlocation = nTempLocation;
			}

		}
		else
		{///scheduler module is not open , So used Logged-in location

			nDefaultlocation = GetCurrentLocationID();
		}
	}
	break;
	default:
		//use nDefaultlocation 
		break;
	}
	return nDefaultlocation;
}


//(s.dhole 5/12/2015 3:41 PM ) - PLID 65621 default resource ID
void CFirstAvailableAppt::OnSelChosenDefaultResourceCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow)
		{
			return;
		}
		
		// (v.maida 2016-03-21 08:57) - PLID 65621 - Changed to Current Scheduler Resource.
		long nDefaultResourceID = VarLong(pRow->GetValue(fdrcID), FFA_DEFAULT_CURRENT_SCHEDULER_RESOURCE);
		SetRemotePropertyInt("FFA_DefaultResourceID", nDefaultResourceID, 0, GetCurrentUserName());
	}NxCatchAll(__FUNCTION__);
}

//(s.dhole 5/12/2015 3:41 PM ) - PLID 66125 default resource ID
void  CFirstAvailableAppt::LoadDefaultResourceComboValue(NXDATALIST2Lib::IRowSettingsPtr pRow)
{

	CArray<long, long> aryPreselectResources;
	// (v.maida 2016-03-21 17:03) - PLID 65621 - Use Current Scheduler Resource as the default value, if one can't be acquired from the row.
	long nDefaultResourceID = VarLong(pRow->GetValue(fdrcID), FFA_DEFAULT_CURRENT_SCHEDULER_RESOURCE);
	switch (nDefaultResourceID)
	{
	case FFA_DEFAULT_ANY_RESOURCE:
		// no resource selected
	{
		SetResourceComboToAnyResource(aryPreselectResources);
	}
	break;
	case FFA_DEFAULT_CURRENT_SCHEDULER_RESOURCE:
	{
		SetResourceComboToSchedulerResource(aryPreselectResources);
	}
	break;
	case FFA_USE_PATIENT_GENEREAL1_PROVIDER_AS_RESOURCE:
	{
		//(s.dhole 5/21/2015 5:02 PM ) - PLID 66125
		CString strHiddenResourceIDs = CalcExcludedResourceIDs();
		CString strSqlHiddenResourceID = "";
		if (!strHiddenResourceIDs.IsEmpty())
		{
			strSqlHiddenResourceID = FormatString(" AND ResourceT.ID NOT IN (%s) ", strHiddenResourceIDs);
		}

		ADODB::_RecordsetPtr prs = CreateRecordset(
			"SELECT	ResourceID \r\n"
			"FROM	PatientsT \r\n"
			"	INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID \r\n"
			"	INNER JOIN ResourceProviderLinkT ON PatientsT.MainPhysician = ResourceProviderLinkT.ProviderID \r\n"
			"	INNER JOIN ResourceT ON ResourceProviderLinkT.ResourceID = ResourceT.ID \r\n"
			"WHERE	ResourceT.Inactive = 0  %s\r\n"
			"	AND	PatientsT.PersonID = %li \r\n",
			strSqlHiddenResourceID, m_nPatientID);

		while (!prs->eof){
			long nResourceID = AdoFldLong(prs->Fields, "ResourceID", -1);
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_Resources->FindByColumn(fdrcID, nResourceID, NULL, VARIANT_FALSE);
			if (pRow){
				aryPreselectResources.Add(nResourceID);
			}
			prs->MoveNext();
		}
		if (aryPreselectResources.GetCount() == 0)
		{
			// (v.maida 2016-03-21 08:57) - PLID 65621 - We do not have a provider selected. Assign to Current Scheduler Resource
			SetResourceComboToSchedulerResource(aryPreselectResources);
		}
		else if (aryPreselectResources.GetCount() == 1)
		{
			m_Resources->FindByColumn(fdrcID, aryPreselectResources[0], NULL, VARIANT_TRUE);
			// (v.maida 2016-03-23 16:15) - PLID 65623 - Store last selected resource ID
			m_nLastResourceID = aryPreselectResources[0];
		}
		else
		{
			m_Resources->FindByColumn(fdrcID, (long)FFA_MULTIPLE_SCHEDULER_RESOURCE, NULL, VARIANT_TRUE);
		}

	}
	break;
	default:
		//use nDefaultResourceID 
	{
		if (!m_Resources->FindByColumn(fdrcID, nDefaultResourceID, NULL, VARIANT_TRUE))
		{
			// (v.maida 2016-03-21 08:57) - PLID 65621 - Default resource does not exist in dropdown, switch to Current Scheduler Resource
			SetResourceComboToSchedulerResource(aryPreselectResources);
		}
		else
		{
			aryPreselectResources.Add(nDefaultResourceID);
			// (v.maida 2016-03-23 16:15) - PLID 65623 - Store last selected resource ID
			m_nLastResourceID = nDefaultResourceID;
		}
	}
	break;
	}
	m_arySelectedResources.Copy(aryPreselectResources);
}

void CFirstAvailableAppt::OnSelChosenLocationListCombo(LPDISPATCH lpRow)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

	if (pRow) {
		long nID = VarLong(pRow->GetValue(lccID), -1);

		if (nID == -2)
		{
			CMultiSelectDlg dlg(this, "LocationsT");
			dlg.PreSelect(m_varLocIDs);
			HRESULT hRes = dlg.Open("LocationsT", "Active = 1 AND TypeID = 1", "ID", "Name", "Select Multiple Locations", 1);

			if (hRes == IDOK)
			{
				m_varLocIDs.RemoveAll();
				dlg.FillArrayWithIDs(m_varLocIDs);

				if (m_varLocIDs.GetCount() == 1){
					m_FFALocationCombo->SetSelByColumn(lccID, AsLong(m_varLocIDs.GetAt(0)));
					m_nLastLocationID = AsLong(m_varLocIDs.GetAt(0));
				}
			}
			else 
			{
				// (r.farnworth 2015-05-14 07:20) - PLID 65625 - Reset to the user's last choice when they hit Cancel
				if (m_nLastLocationID != -2)
					m_FFALocationCombo->SetSelByColumn(lccID, m_nLastLocationID);
			}
				
		}
		else if (nID != -2)
		{
			m_nLastLocationID = nID;
			m_varLocIDs.RemoveAll();
			m_varLocIDs.Add(variant_t(nID));
		}

		// (r.farnworth 2015-06-08 10:53) - PLID 65635
		ToggleLocationLabel();
	}
}

// (r.farnworth 2015-05-14 08:57) - PLID 65625 - Add new dropdown for Appointment Location showing only general locations.
void CFirstAvailableAppt::ToggleLocationLabel()
{
	if (m_varLocIDs.GetCount() > 1) {
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		CString strLocationLabel = "";
		int nCount = 0, nSize = 0;
		nSize = m_varLocIDs.GetSize();

		//Create the label using the loaded data list
		while (nCount < nSize) {
			pRow = m_FFALocationCombo->FindByColumn(lccID, _variant_t(m_varLocIDs.GetAt(nCount)), NULL, VARIANT_FALSE);
			nCount++;
			if (pRow == NULL) {
				continue;
			}
			
			strLocationLabel += VarString(pRow->GetValue(lccName)) + ", ";
		}
		strLocationLabel.Trim(", ");

		m_nxstaticMultiLocationLabel.SetText(strLocationLabel);
		//Hide the datalist, show the label. 
		m_nxstaticMultiLocationLabel.EnableWindow(TRUE);
		m_nxstaticMultiLocationLabel.ShowWindow(SW_SHOWNA);
		m_FFALocationCombo->Enabled = FALSE;
		GetDlgItem(IDC_FFA_LOCATIONS_COMBO)->ShowWindow(SW_HIDE);
	}
	else {
		CString strLocationLabel = "";

		m_nxstaticMultiLocationLabel.SetText(strLocationLabel);

		//hide the label, show the datalist. 
		m_nxstaticMultiLocationLabel.EnableWindow(FALSE);
		m_nxstaticMultiLocationLabel.ShowWindow(SW_HIDE);
		m_FFALocationCombo->Enabled = TRUE;
		GetDlgItem(IDC_FFA_LOCATIONS_COMBO)->ShowWindow(SW_SHOW);

		//Special case, if they selected one from the multi-select dialog then I want to select it.
		//		If they selected none, I want nothing selected.
		if (m_varLocIDs.GetCount() == 1) {

			// (r.farnworth 2016-02-11 08:19) - PLID 65638 - Need to handle the special case where that location we need is no longer in the list
			if (m_FFALocationCombo->FindByColumn(lccID, m_varLocIDs.GetAt(0), NULL, FALSE))
			{
				m_FFALocationCombo->SetSelByColumn(lccID, m_varLocIDs.GetAt(0));
				m_nLastLocationID = m_varLocIDs.GetAt(0);
			}
			else
			{
				m_FFALocationCombo->SetSelByColumn(lccID, -1);
				m_varLocIDs.RemoveAll();
				m_varLocIDs.Add(variant_t(-1));
			}
		}
	}
}

// (r.farnworth 2015-05-14 08:55) - PLID 65625 - Change the cursor so they know its a link
BOOL CFirstAvailableAppt::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if (m_nxstaticMultiLocationLabel.IsWindowVisible() && m_nxstaticMultiLocationLabel.IsWindowEnabled()) 
		{
			m_nxstaticMultiLocationLabel.GetWindowRect(rc);
			ScreenToClient(&rc);
			//Only if we're over the label does the cursor change. 
			if (rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		if (m_nxlUnselectedResourceLabel.IsWindowVisible() && m_nxlUnselectedResourceLabel.IsWindowEnabled())
		{
			m_nxlUnselectedResourceLabel.GetWindowRect(rc);
			ScreenToClient(&rc);
			//Only if we're over the label does the cursor change. 
			if (rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		//(s.dhole 5/15/2015 9:34 AM ) - PLID  65623
		if (m_nxlIsResourcesUseAndFilterLabel.IsWindowVisible() && m_nxlIsResourcesUseAndFilterLabel.IsWindowEnabled())
		{
			m_nxlIsResourcesUseAndFilterLabel.GetWindowRect(rc);
			ScreenToClient(&rc);
			//Only if we're over the label does the cursor change. 
			if (rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

	}NxCatchAll(__FUNCTION__);

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (r.farnworth 2015-05-14 08:55) - PLID 65625 - Change the cursor so they know its a link
LRESULT CFirstAvailableAppt::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try{

		switch (wParam)
		{
		case IDC_MULTI_LOCATION:
		{
			CMultiSelectDlg dlg(this, "LocationsT");
			dlg.PreSelect(m_varLocIDs);
			HRESULT hRes = dlg.Open("LocationsT", "Active = 1 AND TypeID = 1", "ID", "Name", "Select Multiple Locations", 1);
			if (hRes == IDOK)
			{
				m_varLocIDs.RemoveAll();
				dlg.FillArrayWithIDs(m_varLocIDs);
			}
			ToggleLocationLabel();
		}
		break;
		//(s.dhole 5/15/2015 9:54 AM ) - PLID 65623
		case ID_UNSELECTED_RESOURCES_STATIC:
		{
			if (MultiResourcesSelections())
			{
				ToggleResourcesSelections();
				// (r.farnworth 2016-02-12 11:47) - PLID 67960 - We need to toggle purposes and types here now
				RequeryAptTypeCombo();
				OnSelChosenApttypeCombo(m_dlAptType->GetCurSel());
			}
			
		}
		break;
		//(s.dhole 5/15/2015 9:54 AM ) - PLID 65624
		case ID_ALL_NONE_RESOURCES_STATIC:
		{
			m_bSelectAnyResource = !m_bSelectAnyResource;
			MangeResourceFilterOR_AND();
			// (r.farnworth 2016-02-12 11:47) - PLID 67960 - We need to toggle purposes and types here now
			RequeryAptTypeCombo();
			OnSelChosenApttypeCombo(m_dlAptType->GetCurSel());
		}
		default:
		{
			break;
		}
		}
	
	}NxCatchAll(__FUNCTION__);
	return 0;
}


//(s.dhole 5/14/2015 10:38 AM ) - PLID 66125
void CFirstAvailableAppt::OnSelChosenUnselectedResource(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow)
		{
			return;
		}
		CArray<long, long> aryResources;
		long nID = VarLong(pRow->GetValue(fdrcID));
		// (v.maida 2016-03-23 16:15) - PLID 65623 - Edited to use last selected resource, if the multi-select dialog was canceled out of.
		if (nID == FFA_MULTIPLE_SCHEDULER_RESOURCE)
		{
			if (MultiResourcesSelections())
			{
				ToggleResourcesSelections();
			}
			else 
			{
				// Cancel was clicked within the multi-select dialog, so just set the selection back to the last selected resource, and appropriately
				// update the array of selected resources to reflect that prior selection
				if (m_nLastResourceID == FFA_DEFAULT_ANY_RESOURCE)
				{
					LoadAnyResources(aryResources);
				}
				else 
				{
					aryResources.Add(m_nLastResourceID);
				}
				m_arySelectedResources.Copy(aryResources);
				m_Resources->FindByColumn(fdrcID, m_nLastResourceID, NULL, VARIANT_TRUE);
			}
		}
		else if (nID == FFA_DEFAULT_ANY_RESOURCE)
		{
			// (r.farnworth 2015-05-19 16:16) - PLID 65633 - For the sake of the results screen, we need to pass in all resources.
			LoadAnyResources(aryResources);
			m_arySelectedResources.Copy(aryResources);
			// (v.maida 2016-03-23 16:15) - PLID 65623 - Store last selected resource ID
			m_nLastResourceID = FFA_DEFAULT_ANY_RESOURCE;
		}
		else
		{
			aryResources.Add(nID);
			m_arySelectedResources.Copy(aryResources);
			// (v.maida 2016-03-23 16:15) - PLID 65623 - Store last selected resource ID
			m_nLastResourceID = nID;
		}

		RequeryAptTypeCombo();
		//(s.dhole 5/20/2015 10:47 AM ) - PLID 66144
		OnSelChosenApttypeCombo(m_dlAptType->GetCurSel());
		SetGoStatus();
		// (c.haag 2007-02-20 12:37) - PLID 24567 - Since we changed the purpose, we must
		// clear out our default durations
		InvalidateDurationGroups();
	}NxCatchAll(__FUNCTION__);
}
//(s.dhole 5/15/2015 4:49 PM ) - PLID 66125
void CFirstAvailableAppt::OnSelChangingUnselectedResource(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		//Check your pointer...
		IRowSettingsPtr pRow(*lppNewSel);
		if (!pRow) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
		IRowSettingsPtr pRowOld(lpOldSel);
		if (pRowOld)
		{
			if (VarLong(pRowOld->GetValue(fdrcID)) == FFA_DEFAULT_ANY_RESOURCE && VarLong(pRow->GetValue(fdrcID)) != FFA_DEFAULT_ANY_RESOURCE)
			{
				//if old selection was any 
				m_arySelectedResources.RemoveAll();
			}
		}
		
	}NxCatchAll("Error checking OnSelChangingUnselectedResource.");
}


//(s.dhole 5/21/2015 3:11 PM ) - PLID 65623
BOOL CFirstAvailableAppt::MultiResourcesSelections()
{
	CMultiSelectDlg dlg(this, "ResourceT");
	for (int i = 0; i < m_arySelectedResources.GetCount(); i++)
	{
		dlg.PreSelect(m_arySelectedResources[i]);
	}

	//(s.dhole 5/21/2015 5:02 PM ) - PLID 66125
	CString strHiddenResourceIDs = CalcExcludedResourceIDs();
	CString strSqlHiddenResourceID = "";
	if (!strHiddenResourceIDs.IsEmpty())
	{
		strSqlHiddenResourceID = FormatString(" AND ResourceT.ID NOT IN (%s) ", strHiddenResourceIDs);
	}

	CString strWhere = FormatString(" Inactive = 0 %s ", strSqlHiddenResourceID);
	//see if we have anything already
	dlg.m_strNameColTitle = "Resource";
	int res = dlg.Open("ResourceT", strWhere, "ResourceT.ID", "ResourceT.Item", "Select Resources", 1);
	BringWindowToTop();
	if (res == IDCANCEL) {
		return FALSE;
	}
	dlg.FillArrayWithIDs(m_arySelectedResources);

	// (v.maida 2016-03-23 16:15) - PLID 65623 - Store last selected resource ID
	if (m_arySelectedResources.GetCount() == 1)
	{
		m_nLastResourceID = m_arySelectedResources[0];
	}
	return TRUE;
}

// (j.politis 2015-05-18 13:02) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
// (v.maida 2016-01-28 10:50) - PLID 65626 - Edited to account for null selections.
void CFirstAvailableAppt::OnSelChangedFfaHoursCombo(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		//Try to stop a non-row from being selected
		if (lpNewSel == NULL) {
			lpNewSel = lpOldSel;
			m_dlHours->CurSel = IRowSettingsPtr(lpOldSel);
		}
		else {
			OnSelectionsChanged();
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.politis 2015-05-18 13:02) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
// (v.maida 2016-01-28 10:50) - PLID 65626 - Edited to account for null selections.
void CFirstAvailableAppt::OnSelChangedFfaMinsCombo(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		//Try to stop a non-row from being selected
		if (lpNewSel == NULL) {
			lpNewSel = lpOldSel;
			m_dlMinutes->CurSel = IRowSettingsPtr(lpOldSel);
		}
		else {
			OnSelectionsChanged();
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.politis 2015-05-18 13:02) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
void CFirstAvailableAppt::OnSelectionsChanged()
{
	try
	{
		//This signifies that the user made a selection from the drop down.
		//This tells us NOT to change the hours and minutes whenever appointment type is changed.
		m_bUserEditedDuration = TRUE;

		//Obviously we want to update the search interval whenever a user chooses a new hours or minutes duration.
		SetSearchIntervalAndUpdate(CalculateApptDuration(GetDropdownHours(), GetDropdownMinutes()), true);
	}NxCatchAll(__FUNCTION__);
}

// (j.politis 2015-05-18 13:02) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
long CFirstAvailableAppt::GetDropdownHours()
{
	try 
	{
		IRowSettingsPtr pRow = m_dlHours->GetCurSel();
		if (pRow)
		{
			return AsLong(pRow->GetValue(EDurationHoursColumn::dhcHours));
		}
	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.politis 2015-05-18 13:02) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
long CFirstAvailableAppt::GetDropdownMinutes()
{
	try 
	{
		IRowSettingsPtr pRow = m_dlMinutes->GetCurSel();
		if (pRow)
		{
			return AsLong(pRow->GetValue(EDurationMinsColumn::dmcMinutes));
		}
	}NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.politis 2015-05-18 13:02) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
void CFirstAvailableAppt::SetDropdownHours(long Hours)
{
	try {
		//Set the hours
		_bstr_t str = AsBstr(FormatString("%li", Hours));
		m_dlHours->FindByColumn(EDurationHoursColumn::dhcHours, str, NULL, VARIANT_TRUE);
	}NxCatchAll(__FUNCTION__);
}

// (j.politis 2015-05-18 13:02) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
void CFirstAvailableAppt::SetDropdownMinutes(long Minutes)
{
	try {
		//Set the minutes
		_bstr_t str = AsBstr(FormatString("%02li", Minutes));
		m_dlMinutes->FindByColumn(EDurationMinsColumn::dmcMinutes, str, NULL, VARIANT_TRUE);
	}NxCatchAll(__FUNCTION__);
}

// (j.politis 2015-05-18 13:02) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
void CFirstAvailableAppt::SetSearchIntervalAndUpdate(int searchInterval, bool bDoInitialUpdate)
{
	//Never allow below 5
	if (searchInterval < 5)
	{
		searchInterval = 5;
	}

	//There was only 1 spot where we were not calling UpdateData TRUE before setting the interval and then doing Update FALSE
	//So i've provided the bool as a parameter just for that one case.
	if (bDoInitialUpdate)
	{
		UpdateData(TRUE);
	}

	//Set the search intervale.
	//Before changing this code in the future, please remember that this variable is tied to an edit box
	m_iSearchInterval = searchInterval;
	UpdateData(FALSE);
}


//(s.dhole 5/15/2015 9:48 AM ) - PLID 65624
void CFirstAvailableAppt::MangeResourceFilterOR_AND()
{
	if (GetResourceComboSelectedValue() == FFA_MULTIPLE_SCHEDULER_RESOURCE)
	{
		if (m_arySelectedResources.GetCount() > 1)
		{
			HandelMultiResourceLabel(TRUE );
		}
		else
		{
			HandelMultiResourceLabel(FALSE);
		}
		if (m_bSelectAnyResource)
		{
			m_nxlIsResourcesUseAndFilterLabel.SetText("Any");
		}
		else
		{
			m_nxlIsResourcesUseAndFilterLabel.SetText("All");
		}
	}
	else
	{
		HandelMultiResourceLabel(FALSE);
	}
}
//(s.dhole 5/20/2015 5:32 PM ) - PLID 65627
void CFirstAvailableAppt::OnShowAppHistory()
{
	try {
		
		LoadPatientAppHistory(m_nPatientID);
		// (b.cardillo 2016-01-20 15:49) - PLID 65623 (supplemental) - For appt history, hide the separator bar if at top or bottom
		if (m_pAppHistoryList->GetRowCount() > 0)
		{
			GetDlgItem(IDC_FFA_APP_HIST_LIST)->ShowWindow(SW_SHOW);
			m_pAppHistoryList->DropDownState = VARIANT_TRUE;
		}
		else
		{
		// if no appointment found then show message
			AfxMessageBox("There is no appointment scheduled for this patient.");
		}
	}NxCatchAll(__FUNCTION__);
}



//(s.dhole 5/20/2015 5:32 PM ) - PLID 65627
void CFirstAvailableAppt::LoadPatientAppHistory(long nPatientId)
{
	try {
		CWaitCursor wc;
		// try to set new where clause no rows  in combobox for selected patient
		if (m_pAppHistoryList->FindByColumn(ahcPatientID, nPatientId, NULL, VARIANT_FALSE) == nullptr)
		{
			m_pAppHistoryList->Clear();
			m_pAppHistoryList->WhereClause = "";
			// (b.cardillo 2016-02-03 16:12) - PLID 65627 (supplemental) - Removed separator bar, just made past appts grey
			// (b.cardillo 2016-02-16 15:12) - PLID 65627 (supplemental) - Combined date-time column, and combined type-purpose column
			// (v.maida 2016-02-17 13:04) - PLID 68237 - Added ForeColor.
			// (v.maida 2016-02-17 13:04) - PLID 68237 - Added darkened forecolor calculation.
			// (b.cardillo 2016-03-01 16:41) - PLID 65627 (supplemental) - Fixed duplication due to multiple purposes; fixed incorrect insured party selection, pull from appointment placement 1 rather than try to guess from the patient
			m_pAppHistoryList->FromClause = bstr_t(FormatString(R"(
(
 SELECT   TOP 10  
    AppointmentsT.ID AS ApptID
  , AppointmentsT.StartTime
  , AppointmentsT.EndTime
  , AppointmentsT.Date
  , ISNULL(AptTypeT.ID,-1) AS AptTypeID
  , AptTypeT.Name AS AptTypeName
  , dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDString
  , dbo.GetResourceString(AppointmentsT.ID) AS ResourceName
  , ISNULL((SELECT TOP 1 AppointmentPurposeT.PurposeID FROM AppointmentPurposeT INNER JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID WHERE AppointmentPurposeT.AppointmentID = AppointmentsT.ID ORDER BY AptPurposeT.Name ASC), -1) AS PurposeID
  , ISNULL(AptTypeT.Name, '') + ISNULL(' - ' + dbo.GetPurposeString(AppointmentsT.ID), '') AS TypeAndPurpose
  , ISNULL(LocationsT.ID,-1) AS LocID, LocationsT.Name AS LocationName
  , ISNULL(InsuredPartyT.PersonID,-1) AS InsurancePriId
  , ISNULL(InsuredPartyT.InsuranceCoID,-1) AS InsuranceCoID
  , AppointmentsT.PatientID
  , PatientsT.UserDefinedID, -1 AS Ord
  , %li AS BKColor
  , CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE  0x00001 * ((((AptTypeT.Color & 0x0000ff) / 0x00001) * 2) / 3) + 0x00100 * ((((AptTypeT.Color & 0x00ff00) / 0x00100) *2) / 3) + 0x10000 * ((((AptTypeT.Color & 0xff0000) / 0x10000) * 2) / 3) END AS ForeColor 
 FROM AppointmentsT 
 INNER JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID 
 LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID 
 LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID 
 LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = (SELECT AppointmentInsuredPartyT.InsuredPartyID FROM AppointmentInsuredPartyT WHERE AppointmentInsuredPartyT.AppointmentID = AppointmentsT.ID AND AppointmentInsuredPartyT.Placement = 1) -- Tell SQL there's only ONE (or zero) of these records
 WHERE (AppointmentsT.Status <> 4) 
  AND (PatientsT.PersonID > 0) 
  AND (AppointmentsT.ShowState <> 3) 
  AND Date <= CAST(CAST(DATEPART(YYYY, GETDATE()) AS VARCHAR(10)) + '-' + CAST(DATEPART(M, GETDATE())AS VARCHAR(10)) + '-' + CAST(DATEPART(D, GETDATE()) AS VARCHAR(10)) + ' 23:59:00:000' AS DATETIME) 
  AND AppointmentsT.PatientID = %li 
 ORDER BY AppointmentsT.Date DESC, AppointmentsT.StartTime DESC 
 UNION ALL 
 SELECT   
    AppointmentsT.ID AS ApptID
  , AppointmentsT.StartTime
  , AppointmentsT.EndTime
  , AppointmentsT.Date
  , ISNULL(AptTypeT.ID,-1) AS AptTypeID
  , AptTypeT.Name AS AptTypeName
  , dbo.GetResourceIDString(AppointmentsT.ID) AS ResourceIDString
  , dbo.GetResourceString(AppointmentsT.ID) AS ResourceName
  , ISNULL((SELECT TOP 1 AppointmentPurposeT.PurposeID FROM AppointmentPurposeT INNER JOIN AptPurposeT ON AppointmentPurposeT.PurposeID = AptPurposeT.ID WHERE AppointmentPurposeT.AppointmentID = AppointmentsT.ID ORDER BY AptPurposeT.Name ASC), -1) AS PurposeID
  , ISNULL(AptTypeT.Name, '') + ISNULL(' - ' + dbo.GetPurposeString(AppointmentsT.ID), '') AS TypeAndPurpose
  , ISNULL(LocationsT.ID,-1)  AS LocID
  , LocationsT.Name AS LocationName
  , ISNULL(InsuredPartyT.PersonID,-1) AS InsurancePriId
  , ISNULL(InsuredPartyT.InsuranceCoID,-1) AS InsuranceCoID
  , AppointmentsT.PatientID
  , PatientsT.UserDefinedID
  , 1 AS Ord
  , %li AS BKColor
  , CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE  0x00001 * ((((AptTypeT.Color & 0x0000ff) / 0x00001) * 2) / 3) + 0x00100 * ((((AptTypeT.Color & 0x00ff00) / 0x00100) *2) / 3) + 0x10000 * ((((AptTypeT.Color & 0xff0000) / 0x10000) * 2) / 3) END AS ForeColor 
 FROM AppointmentsT 
 INNER JOIN PatientsT ON AppointmentsT.PatientID = PatientsT.PersonID 
 LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID 
 LEFT JOIN LocationsT ON AppointmentsT.LocationID = LocationsT.ID 
 LEFT JOIN InsuredPartyT ON InsuredPartyT.PersonID = (SELECT AppointmentInsuredPartyT.InsuredPartyID FROM AppointmentInsuredPartyT WHERE AppointmentInsuredPartyT.AppointmentID = AppointmentsT.ID AND AppointmentInsuredPartyT.Placement = 1) -- Tell SQL there's only ONE (or zero) of these records
 WHERE (AppointmentsT.Status <> 4) 
  AND (PatientsT.PersonID > 0) 
  AND (AppointmentsT.ShowState <> 3) 
  AND Date > CAST(CAST(DATEPART(YYYY, GETDATE()) AS VARCHAR(10)) + '-' + CAST(DATEPART(M, GETDATE())AS VARCHAR(10)) + '-' + CAST(DATEPART(D, GETDATE()) AS VARCHAR(10)) + ' 23:59:00:000' AS DATETIME) 
  AND AppointmentsT.PatientID = %li 
 ) AS _Q  
)"
				, RGB(0xe0, 0xe0, 0xe0), nPatientId
				, NXDATALIST2Lib::dlColorNotSet, nPatientId)
				);
		}
		// will Requery  in case some one else added or change appointments
		m_pAppHistoryList->Requery();
		m_pAppHistoryList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	}NxCatchAll(__FUNCTION__);
}


//(s.dhole 5/20/2015 5:32 PM ) - PLID 65628
void CFirstAvailableAppt::OnSelChosenAppHistoryCombo(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (!pRow) {
			return;
		}
		if (m_pAppHistoryList->GetRowCount() > 0)
		{
			// only process this request if there is data
			if (VarLong(pRow->GetValue(ahcOrd), 0) != 0)
			{
				NXDATALIST2Lib::IRowSettingsPtr pRowTemp;
				CString  strResourceIds = VarString(pRow->GetValue(ahcResourceIds), "");
				if (!strResourceIds.IsEmpty())
				{
					LoadResourceIDStringIntoArray(strResourceIds, m_arySelectedResources);
					if (m_arySelectedResources.GetCount() > 1)
					{
							m_Resources->FindByColumn(fdrcID, (long)FFA_MULTIPLE_SCHEDULER_RESOURCE, NULL, VARIANT_TRUE);
							m_bSelectAnyResource = FALSE;
					} else if (m_arySelectedResources.GetCount() == 1) {
						// (b.cardillo 2016-01-20 15:49) - PLID 65623 (supplemental) - For appt history, select resource even if only one
						m_Resources->FindByColumn(fdrcID, (long)m_arySelectedResources[0], NULL, VARIANT_TRUE);
						m_bSelectAnyResource = FALSE;
						// (v.maida 2016-03-23 16:15) - PLID 65623 - Store last selected resource ID
						m_nLastResourceID = m_arySelectedResources[0];
					}

					ToggleResourcesSelections();
				}
				else
				{
					// (v.maida 2016-03-21 08:57) - PLID 65621 - Switch to Current Scheduler Resource if there are no resources found
					SetResourceComboToSchedulerResource(m_arySelectedResources);
				}
       
				// (b.cardillo 2016-01-20 15:49) - PLID 65623 (supplemental) - For appt history, select location even if user had selected multiple
				// Choose the single location based on the selected appointment's location
				{
					m_varLocIDs.RemoveAll();
					long nLocID = VarLong(pRow->GetValue(ahcLocID), -1);
					if (nLocID != -1) {
						m_varLocIDs.Add(nLocID);
					} else {
						m_FFALocationCombo->FindByColumn(lccID, -1L, NULL, VARIANT_TRUE);
					}
					ToggleLocationLabel();
				}

				pRowTemp = NULL;

				if (VarLong(pRow->GetValue(ahcInsurancePriId), -1) != -1)
				{
					 pRowTemp= m_dlInsuredPartyCombo->FindByColumn((short)EInsuredPartyComboColumns::ID, VarLong(pRow->GetValue(ahcInsurancePriId), -1), NULL, VARIANT_TRUE);
					// If empty then set back to '{No Insurance}'
					if (!pRowTemp)
					{
						m_dlInsuredPartyCombo->FindByColumn((short)EInsuredPartyComboColumns::ID, -1L, NULL, VARIANT_TRUE);
					}
				}
				else
				{
				// If empty then set back to '{No Insurance}'
					m_dlInsuredPartyCombo->FindByColumn((short)EInsuredPartyComboColumns::ID, -1L, NULL, VARIANT_TRUE);
				}

				pRowTemp = NULL;
				_variant_t varNull;
				varNull.vt = VT_NULL;
				if (VarLong(pRow->GetValue(ahcAptTypeID), -1) != -1)
				{
					pRowTemp = m_dlAptType->FindByColumn(atcID, VarLong(pRow->GetValue(ahcAptTypeID), -1), NULL, VARIANT_TRUE);
					// If empty then set back to '{ No Appointment Type }'
					if (!pRowTemp)
					{
						pRowTemp = m_dlAptType->FindByColumn(atcID, varNull, NULL, VARIANT_TRUE);
					}
					
				}
				else
				{// If empty then set back to '{ No Appointment Type }'
					pRowTemp = m_dlAptType->FindByColumn(atcID, varNull, NULL, VARIANT_TRUE);
				}
				OnSelChosenApttypeCombo(pRowTemp);

				pRowTemp = NULL;
				if (VarLong(pRow->GetValue(ahcPurposeID), -1) != -1)
				{
					long nV = VarLong(pRow->GetValue(ahcPurposeID), -1);
					pRowTemp = m_dlAptPurpose->FindByColumn(apcID, VarLong(pRow->GetValue(ahcPurposeID), -1), NULL, VARIANT_TRUE);
					if (!pRowTemp)
					{
						m_dlAptPurpose->FindByColumn(apcID, varNull, NULL, VARIANT_TRUE);
						GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(FALSE);
					}
					else
					{
						GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(TRUE);
					}
				}
				else
				{
					m_dlAptPurpose->FindByColumn(apcID, varNull, NULL, VARIANT_TRUE);
					GetDlgItem(IDC_APTPURPOSE_COMBO)->EnableWindow(FALSE);
				}
				GetDlgItem(ID_APP_HISTORY_BTN)->SetFocus();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CFirstAvailableAppt::OnClosedUpAppHistoryCombo(LPDISPATCH lpRow)
{
	try {
		GetDlgItem(ID_APP_HISTORY_BTN)->SetFocus();
	}NxCatchAll(__FUNCTION__);
}

//(s.dhole 5/21/2015 5:26 PM ) - PLID 66125
void CFirstAvailableAppt::LoadResourceIDStringIntoArray(CString strResourceIDs, CArray<long, long> &arIDs)
{
	arIDs.RemoveAll();
	while (strResourceIDs.GetLength()) {
		long nID;
		if (-1 != strResourceIDs.Find(" ")) {
			nID = atoi(strResourceIDs.Left(strResourceIDs.Find(" ")));
			strResourceIDs = strResourceIDs.Right(strResourceIDs.GetLength() - strResourceIDs.Find(" ") - 1);
		}
		else  {
			nID = atoi(strResourceIDs);
			strResourceIDs.Empty();
		}
		arIDs.Add(nID);
	}
}
//(s.dhole 5/21/2015 5:26 PM ) - PLID 66125
long CFirstAvailableAppt::GetResourceComboSelectedValue()
{
	IRowSettingsPtr pRow = m_Resources->GetCurSel();
	if (pRow)
	{
		return VarLong(pRow->GetValue(fdrcID));
	}
	else
	{
		// (v.maida 2016-03-21 08:57) - PLID 65621 - Return the current scheduler resource, if no row could be obtained.
		return GetCurrentSchedulerResource();
	}
	
}
//(s.dhole 5/21/2015 5:26 PM ) - PLID 66125
void CFirstAvailableAppt::LoadAnyResources( CArray<long, long> &arIDs)
{
	arIDs.RemoveAll();
	IRowSettingsPtr pDropdownRow = m_Resources->GetFirstRow();
	long pID;
	while (pDropdownRow != NULL)
	{
		pID = VarLong(pDropdownRow->GetValue(fdrcID));
		if (pID >= 0) {
			arIDs.Add(pID);
		}
		pDropdownRow = pDropdownRow->GetNextRow();
	}
	//(s.dhole 5/27/2015 3:04 PM ) - PLID 65624
	m_bSelectAnyResource = TRUE;
}
//(s.dhole 5/21/2015 10:57 AM ) - PLID 65624
void CFirstAvailableAppt::HandelMultiResourceLabel(BOOL bShow)
{
	if (bShow == FALSE)
	{
		GetDlgItem(ID_UNSELECTED_RESOURCES_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(ID_ALL_NONE_RESOURCES_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(ID_ALL_ANY_OF_STATIC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_UNSELECTED_RESOURCES_COMBO)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(ID_UNSELECTED_RESOURCES_STATIC)->ShowWindow(SW_SHOW );
		GetDlgItem(ID_ALL_NONE_RESOURCES_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(ID_ALL_ANY_OF_STATIC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_UNSELECTED_RESOURCES_COMBO)->ShowWindow(SW_HIDE);
	}

}
//(s.dhole 5 / 22 / 2015 12:51 PM) - PLID 65621
void CFirstAvailableAppt::SetResourceComboToAnyResource(CArray<long, long> &arIDs)
{
	if (m_Resources->FindByColumn(fdrcID, FFA_DEFAULT_ANY_RESOURCE, NULL, VARIANT_TRUE))
	{
		LoadAnyResources(arIDs);
		// (v.maida 2016-03-23 16:15) - PLID 65623 - Store last selected resource ID
		m_nLastResourceID = FFA_DEFAULT_ANY_RESOURCE;
	}
	else
	{
		//(s.dhole 5/27/2015 3:04 PM ) - PLID 65624
		m_bSelectAnyResource = FALSE;
	}
}

// (r.farnworth 2015-06-08 16:56) - PLID 65639
long CFirstAvailableAppt::GetLocationComboSelectedValue()
{
	IRowSettingsPtr pRow = m_FFALocationCombo->GetCurSel();
	if (pRow)
	{
		return VarLong(pRow->GetValue(fdrcID));
	}
	else
	{
		return FFA_DEFAULT_ANY_LOCATION;
	}

}

// (v.maida 2016-02-05 09:31) - PLID 68132 - Autoscrolls the time preferences listbox to the earliest opening time out of all locations.
void CFirstAvailableAppt::ScrollToEarliestLocationOpeningTime()
{
	try {
		long nMinOpen, nMaxClose, nIndex;
		CString strMinOpen;
		// store earliest opening office hours in nMinOpen
		GetOfficeHours(nMinOpen, nMaxClose, TRUE);
		// convert the earliest opening office hours to a string containing the hour span of that time
		strMinOpen = GetHourSpanText(nMinOpen);
		// find the span within the list
		nIndex = m_TimePrefs.FindString(0, strMinOpen);
		// set the time as the topmost visible element in the list
		m_TimePrefs.SetTopIndex(nIndex);
	}NxCatchAll(__FUNCTION__);
}

// (v.maida 2016-03-21 08:57) - PLID 65621 - Created
void CFirstAvailableAppt::SetResourceComboToSchedulerResource(CArray<long, long> &aryPreselectResources)
{
	try 
	{
		// Use current scheduler location
		CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
		if (pView)
		{

			long  nTempResource = ((CSchedulerView*)pView)->GetActiveResourceID();
			// if mutiresource tab is selected then we shoud select any resource
			if (GetMainFrame()->IsActiveView(SCHEDULER_MODULE_NAME) && pView->GetActiveTab() == 2 /* MultiResourceTab */)
			{
				// select any resource
				SetResourceComboToAnyResource(aryPreselectResources);
			}
			else
			{
				m_Resources->FindByColumn(fdrcID, (long)nTempResource, NULL, VARIANT_TRUE);
				aryPreselectResources.Add(nTempResource);
				// (v.maida 2016-03-23 16:15) - PLID 65623 - Store last selected resource ID
				m_nLastResourceID = nTempResource;
			}
		}
		else
		{
			// (v.maida 2016-02-10 10:46) - PLID 66125 - The scheduler isn't open, so just use any resource.
			SetResourceComboToAnyResource(aryPreselectResources);
		}
	}NxCatchAll(__FUNCTION__);
}

// (v.maida 2016-03-21 08:57) - PLID 65621 - Created
long CFirstAvailableAppt::GetCurrentSchedulerResource()
{
	try 
	{
		// Use current scheduler location
		CNxTabView* pView = (CNxTabView *)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
		if (pView)
		{

			long  nTempResource = ((CSchedulerView*)pView)->GetActiveResourceID();
			// if mutiresource tab is selected then we shoud select any resource
			if (GetMainFrame()->IsActiveView(SCHEDULER_MODULE_NAME) && pView->GetActiveTab() == 2 /* MultiResourceTab */)
			{
				// return any resource
				return FFA_DEFAULT_ANY_RESOURCE;
			}
			else
			{
				return nTempResource;
			}
		}
		else 
		{
			return FFA_DEFAULT_ANY_RESOURCE;
		}
	}NxCatchAll(__FUNCTION__);
	return FFA_DEFAULT_ANY_RESOURCE;
}
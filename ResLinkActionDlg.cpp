// ResLinkActionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "schedulerrc.h"
#include "ResLinkActionDlg.h"
#include "SchedulerView.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "globalschedutils.h"
#include "ppclink.h"
#include "AuditTrail.h"
#include "TodoUtils.h"
#include "HL7Utils.h"
#include "CommonSchedUtils.h"
#include "SharedScheduleUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



typedef enum {
	eColApptID = 0,
	eColPatID = 1,
	eColColor = 2,
	eColCheckbox = 3,
	eColCurDate = 4,
	eColCurDay = 5,
	eColNewDate = 6,
	eColNewDay = 7,
	eColStartTime = 8,
	eColPurpose = 9,
	eColResource = 10,
	eColConfirmed = 11,
	eColNotes = 12,
	eColStatus = 13,
} EListColumns;

/////////////////////////////////////////////////////////////////////////////
// CResLinkActionDlg dialog


CResLinkActionDlg::CResLinkActionDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CResLinkActionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResLinkActionDlg)
	m_nResID = -1;
	m_iRecourse = 2; // (c.haag 2003-07-03 11:08) - Default to doing nothing
	m_nDurationChange = 0;
	//}}AFX_DATA_INIT
}


void CResLinkActionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResLinkActionDlg)
	DDX_Radio(pDX, IDC_RADIO_MOVEALL, m_iRecourse);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_RADIO_MOVEALL, m_radioMoveAll);
	DDX_Control(pDX, IDC_RADIO_CREATETODO, m_radioCreateTodo);
	DDX_Control(pDX, IDC_RADIO_DONOTHING, m_radioDoNothing);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResLinkActionDlg, CNxDialog)
	//{{AFX_MSG_MAP(CResLinkActionDlg)
	ON_COMMAND(ID_LINKACTION_GOTOAPPT, OnGoAppointment)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResLinkActionDlg message handlers

BOOL CResLinkActionDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (c.haag 2008-04-24 14:51) - PLID 29776 - Nxiconify buttons
	m_btnOK.AutoSet(NXB_CLOSE); // It actually says "Close" on the button
	
	// Build the appointment list
	m_dlAptList = BindNxDataListCtrl(this, IDC_APPOINTMENTS, GetRemoteData(), false);
	Load();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CResLinkActionDlg::Open(long nResID)
{
	m_nResID = nResID;
	return DoModal();
}

void CResLinkActionDlg::Load()
{
	enum { eMoveSameDayOnly, eMoveAll } eMoveType;
	CString strFrom;
	CString strWhere;

	// If an appointment moved by less than a 24 hour span, we only move
	// linked appointments on the same day. Otherwise, we move them all
	// in daily increments. We need to figure this out because it will influence
	// the SQL statement.
	// (z.manning, PLID 16947, 07/12/05)
	// There is now an option to move linked appts by the same duration no matter what.
	if ((m_nDurationChange < 0 ? -m_nDurationChange : m_nDurationChange) < 60*24
		&& 0 == GetRemotePropertyInt("LinkedApptsMoveSameDuration", 0, 0, "<None>", true))
	{
		eMoveType = eMoveSameDayOnly;
	}
	else {
		eMoveType = eMoveAll;
	}

	// (d.moore 2007-05-22 11:43) - PLID 4013 - Changed the MoveUp bit to query the waiting list table.
	// Requery the datalist for this patient with the exception of the
	// "Linked" column
	strFrom.Format(		"(SELECT     ResBasicQ.*, dbo.PersonT.Last + ', ' + dbo.PersonT.First + ' ' + dbo.PersonT.Middle AS PatientName, dbo.GetResourceString(ResBasicQ.ID) AS Item, CASE WHEN dbo.GetPurposeString(ResBasicQ.ID) <> '' THEN AptTypeT.Name + ' - ' + dbo.GetPurposeString(ResBasicQ.ID) ELSE AptTypeT.Name END AS Purpose, AptTypeT.Color, 0 AS SetID, '' AS Day, DATEADD(minute, %d, %s) AS NewDate "
		"FROM         AptTypeT RIGHT OUTER JOIN "
		"                      PersonT RIGHT OUTER JOIN "
		"                          (SELECT     AppointmentsT.ID, AppointmentsT.PatientID, AppointmentsT.AptTypeID, CONVERT(datetime, CONVERT(varchar, StartTime, 1)) AS Date, "
		"                                 StartTime, AppointmentsT.Confirmed, AppointmentsT.Notes, "
		"                                 CONVERT(bit, CASE WHEN WaitingListT.ID > 0 THEN 1 ELSE 0 END) AS MoveUp, "
		"                                 AppointmentsT.RecordID, AppointmentsT.Status, PersonT.Archived, AppointmentsT.ShowState, AppointmentsT.CreatedDate, "
		"                                 AppointmentsT.CreatedLogin "
		"                           FROM  AppointmentsT LEFT JOIN "
		"                                 PersonT ON AppointmentsT.PatientID = PersonT.ID "
		"                                 LEFT JOIN WaitingListT ON AppointmentsT.ID = WaitingListT.AppointmentID "
		"							WHERE AppointmentsT.Status <> 4 "		
		"							) ResBasicQ ON dbo.PersonT.ID = ResBasicQ.PatientID ON "
		"			            AptTypeT.ID = ResBasicQ.AptTypeID "
		"						) AS ResComplexQ", m_nDurationChange, (eMoveType == eMoveAll) ? "[Date]" : "[StartTime]");

	m_dlAptList->FromClause = (LPCTSTR)strFrom;

	strWhere.Format("(ID <> %d) AND ID IN (SELECT AppointmentID FROM AptLinkT WHERE GroupID IN (SELECT GroupID FROM AptLinkT WHERE AppointmentID = %d))", m_nResID, m_nResID);
	m_dlAptList->WhereClause = (LPCTSTR)strWhere;
	m_dlAptList->Requery();

	// Now draw the colors
	SetAppointmentColors();
}

void CResLinkActionDlg::SetAppointmentColors()
{
	long Start, End;
	Start = 0;
	End = m_dlAptList->GetRowCount();

	for(long i = Start; i < End; i++){
		OLE_COLOR color = VarLong(m_dlAptList->GetValue(i, 2), RGB(0, 0, 0));
		// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
		color = GetDarkerColorForApptText(color);
		IRowSettingsPtr pRow = m_dlAptList->GetRow(i);
		pRow->PutForeColor(color);

		COleDateTime dt = COleDateTime(pRow->GetValue(eColNewDate));
		if (dt.GetDayOfWeek() == 1 || dt.GetDayOfWeek() == 7)
		{
			pRow->PutCellForeColor(eColNewDay, RGB(255,0,0));
		}
	}
}

void CResLinkActionDlg::SetDays()
{
	enum { eMoveSameDayOnly, eMoveAll } eMoveType;
	COleDateTime dt;
	CString day;
	COleVariant v;
	v.vt = VT_BOOL;
	
	// If an appointment moved by less than a 24 hour span, we only move
	// linked appointments on the same day. Otherwise, we move them all
	// in daily increments.
	// (z.manning, PLID 16947, 07/12/05)
	// There is now an option to move linked appts by the same duration no matter what.
	if ((m_nDurationChange < 0 ? -m_nDurationChange : m_nDurationChange) < 60*24
		&& 0 == GetRemotePropertyInt("LinkedApptsMoveSameDuration", 0, 0, "<None>", true))
	{
		eMoveType = eMoveSameDayOnly;
	}
	else {
		eMoveType = eMoveAll;
	}

	for(int i = 0; i < m_dlAptList->GetRowCount(); i++)
	{
		IRowSettingsPtr pRow = m_dlAptList->GetRow(i);
		dt = VarDateTime(pRow->GetValue(eColCurDate));
		day = FormatDateTimeForInterface(dt, "%a");
		pRow->PutValue(eColCurDay, (_bstr_t)day.Left(eColCurDate));

		dt = VarDateTime(pRow->GetValue(eColNewDate));
		day = FormatDateTimeForInterface(dt, "%a");
		pRow->PutValue(eColNewDay, (_bstr_t)day.Left(eColNewDate));

		// If the new date is on a Saturday or a Sunday, and we move
		// in day increments, we want to make sure the checkbox is unchecked.
		if ((dt.GetDayOfWeek() == 7 || dt.GetDayOfWeek() == 1) &&
			eMoveType == eMoveAll)
		{
			v.boolVal = 0;
		}
		else
		{
			// If the new date is not the same as the old date, and we
			// move in hour increments, we want to make sure the checkbox
			// is unchecked
			dt = VarDateTime(pRow->GetValue(eColCurDate));
			dt.SetDate(dt.GetYear(), dt.GetMonth(), dt.GetDay());
			if (m_dtPivot != dt && eMoveType == eMoveSameDayOnly)
			{
				v.boolVal = 0;
			}
			else
			{
				v.boolVal = 1;
			}
			
		}
		pRow->PutValue(eColCheckbox, v);
	}
}

BEGIN_EVENTSINK_MAP(CResLinkActionDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CResLinkActionDlg)
	ON_EVENT(CResLinkActionDlg, IDC_APPOINTMENTS, 18 /* RequeryFinished */, OnRequeryFinishedAppointments, VTS_I2)
	ON_EVENT(CResLinkActionDlg, IDC_APPOINTMENTS, 2 /* SelChanged */, OnSelChangedAppointments, VTS_I4)
	ON_EVENT(CResLinkActionDlg, IDC_APPOINTMENTS, 3 /* DblClickCell */, OnDblClickCellAppointments, VTS_I4 VTS_I2)
	ON_EVENT(CResLinkActionDlg, IDC_APPOINTMENTS, 6 /* RButtonDown */, OnRButtonDownAppointments, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CResLinkActionDlg::OnRequeryFinishedAppointments(short nFlags) 
{
	m_dlAptList->SetRedraw(FALSE);
	SetAppointmentColors(); // Set the colors
	SetDays();				// Refresh the day column
	m_dlAptList->SetRedraw(TRUE);
}

void CResLinkActionDlg::OnSelChangedAppointments(long nNewSel) 
{
	// Set the recourse to be going to another appointment
//	m_iRecourse = 0;
//	UpdateData(FALSE);
}

#define VALIDATE_BY_RULES(patid, resourceid, location, date, start, end, type, purpose, ignoreapptid, fatal, nonfatal) { \
	if (!AppointmentValidateByRules(patid, resourceid, location, date, start, end, type, purpose, ignoreapptid, TRUE, fatal, nonfatal, &nRuleWarningResult)) { \
		if (!strFatalWarnings.IsEmpty()) { \
			MsgBox(MB_ICONEXCLAMATION|MB_OK, "A linked appointment on %s could not be saved for the following reasons:\n\n%s\n\nThis appointment will be unselected from the list.", date.Format("%x"), strFatalWarnings); \
		} else { \
			MsgBox(MB_ICONEXCLAMATION|MB_OK, "A linked appointment on %s could not be saved because it breaks at least one template rule.\n\nThis appointment will be unselected from the list.", date.Format("%x")); \
		} \
	  	if (nRuleWarningResult & rwHasWarnings) { \
			if (!strNonfatalWarnings.IsEmpty()) { \
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "The linked appointment also triggers the following warnings:\n\n%s", strNonfatalWarnings); \
			} else { \
				MsgBox(MB_ICONEXCLAMATION|MB_OK, "The linked appointment also triggers at least one unspecified warning."); \
			}  \
		} \
		for (int j=0; j < m_dlAptList->GetRowCount(); j++) { \
			if (VarLong(m_dlAptList->GetValue(j, eColApptID)) == ignoreapptid) { \
				COleVariant varBool; \
				varBool.vt = VT_BOOL; varBool.boolVal = 0; \
				m_dlAptList->PutValue(j, eColCheckbox, varBool); \
				break; \
			} \
		} \
		return TRUE; \
	} \
	if (strNonfatalWarnings.GetLength() && -1 == strCompleteNonFatalWarnings.Find(strNonfatalWarnings)) \
		strCompleteNonFatalWarnings += strNonfatalWarnings + "\n"; \
	nTotalRuleWarningResult |= nRuleWarningResult; \
}

BOOL CResLinkActionDlg::LinkedApptsViolateTemplates()
{
	try {
		CString strCompleteNonFatalWarnings;
		_RecordsetPtr prsAppts;
		CString strRestrict;
		long nTotalRuleWarningResult = 0;

		// Step 1: Build our SQL statement of linked appointments
		strRestrict.Format("AptLinkT.AppointmentID <> %d ", m_nResID);
		for (int i=0; i < m_dlAptList->GetRowCount(); i++)
		{
			if (!VarBool(m_dlAptList->GetValue(i, eColCheckbox)))
			{
				CString str;
				str.Format("AND AptLinkT.AppointmentID <> %d ", VarLong(m_dlAptList->GetValue(i, eColApptID)));
				strRestrict += str;
			}
		}

		// Step 2: Get the recordset containing the linked appointments
		prsAppts = CreateRecordset("SELECT AppointmentID, PatientID, LocationID, AptTypeID, "
			"DATEADD(minute, %d, StartTime) AS StartTime, DATEADD(minute, %d, EndTime) AS EndTime, "
			"DATEADD(minute, %d, ArrivalTime) AS ArrivalTime "
			"FROM AptLinkT LEFT JOIN AppointmentsT ON AppointmentsT.ID = AptLinkT.AppointmentID "
			"WHERE AptLinkT.GroupID IN (SELECT GroupID FROM AptLinkT WHERE AppointmentID = %d) "
			"AND %s AND AppointmentsT.Status <> 4 ",
			m_nDurationChange, m_nDurationChange, m_nDurationChange, m_nResID, strRestrict);

		// Step 3: Traverse the recordset and check each linked appointment individually
		// for violations
		while (!prsAppts->eof)
		{
			// Step 3a. Get the resource and purpose lists.
			// (c.haag 2003-07-03 10:20) - In the next release I would like to write a mod
			// that returns the ID's of all the purposes and resources in a whitespace-separated
			// string.
			CDWordArray adwResourceID, adwAptPurpose;
			long nAptType = AdoFldLong(prsAppts, "AptTypeID", -1);
			long nIgnoreApptID = AdoFldLong(prsAppts, "AppointmentID");
			long nPatientID = AdoFldLong(prsAppts, "PatientID");
			long nLocationID = AdoFldLong(prsAppts, "LocationID");
			COleDateTime dtStartTime = AdoFldDateTime(prsAppts, "StartTime");
			COleDateTime dtEndTime = AdoFldDateTime(prsAppts, "EndTime");
			COleDateTime dtDate;
			dtDate.SetDate(dtStartTime.GetYear(), dtStartTime.GetMonth(), dtStartTime.GetDay());

			// Get the list of purposes for this linked appointment
			_RecordsetPtr prsPurposes = CreateRecordset("SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = %d",
				AdoFldLong(prsAppts, "AppointmentID"));
			while (!prsPurposes->eof)
			{
				adwAptPurpose.Add(AdoFldLong(prsPurposes, "PurposeID"));
				prsPurposes->MoveNext();
			}
			prsPurposes->Close();

			// Get the list of resources for this linked appointment
			_RecordsetPtr prsResources = CreateRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = %d",
				AdoFldLong(prsAppts, "AppointmentID"));
			while (!prsResources->eof)
			{
				adwResourceID.Add(AdoFldLong(prsResources, "ResourceID"));
				prsResources->MoveNext();
			}
			prsResources->Close();

			// Step 3b. Validate the new time of the linked appt
			{
				COleDateTime dtCurStart, dtCurEnd;
				dtCurStart.SetDateTime(dtStartTime.GetYear(), dtStartTime.GetMonth(), dtStartTime.GetDay(), 0, 0, 0);	//don't want times
				dtCurEnd.SetDateTime(dtEndTime.GetYear(), dtEndTime.GetMonth(), dtEndTime.GetDay(), 0, 0, 0);	//don't want times

				//now do comparisons.  We need to make sure that the current start time and the current end time are on the same day
				if(dtCurStart != dtCurEnd) {
					MsgBox("A linked appointment on %s could not be saved because the start time and end time are on different days.\n\nThis appointment will be unselected from the list.", dtDate.Format("%x"));
					for (int j=0; j < m_dlAptList->GetRowCount(); j++) {
						if (VarLong(m_dlAptList->GetValue(j, eColApptID)) == nIgnoreApptID) {
							COleVariant varBool;
							varBool.vt = VT_BOOL; varBool.boolVal = 0;
							m_dlAptList->PutValue(j, eColCheckbox, varBool);
							break;
						}
					}
					return TRUE;
				}

				//(z.manning, PLID 16947, 07/12/05)
				//This preference concerns moving appts across days, and whether or not to change the times relatively
				//If this preference is disabled, then moving linked appts has to allow the day to change.
				if(0 == GetRemotePropertyInt("LinkedApptsMoveSameDuration", 0, 0, "<None>", true)) {
					//If we're moving in hourly increments, make sure the day doesn't change
					if ((m_nDurationChange < 0 ? -m_nDurationChange : m_nDurationChange) < 60*24)
					{
						COleDateTime dtOldStart = dtStartTime - COleDateTimeSpan(0,0,m_nDurationChange,0);
						dtOldStart.SetDateTime(dtOldStart.GetYear(), dtOldStart.GetMonth(), dtOldStart.GetDay(), 0, 0, 0);	//don't want times
						if (dtOldStart != dtDate)
						{
							MsgBox("A linked appointment on %s could not be saved because it would have moved to a different day.\n\nThis appointment will be unselected from the list.", dtDate.Format("%x"));
							for (int j=0; j < m_dlAptList->GetRowCount(); j++) {
								if (VarLong(m_dlAptList->GetValue(j, eColApptID)) == nIgnoreApptID) {
									COleVariant varBool;
									varBool.vt = VT_BOOL; varBool.boolVal = 0;
									m_dlAptList->PutValue(j, eColCheckbox, varBool);
									break;
								}
							}
							return TRUE;
						}
					}
				}
			}

			// Step 3c. Iterate through each resource for this appt and determine if
			// a violation can take place
			for (int i=0; i < adwResourceID.GetSize(); i++)
			{
				CString strFatalWarnings;
				CString strNonfatalWarnings;
				long nRuleWarningResult = 0;

				if (!adwAptPurpose.GetSize())
				{
					VALIDATE_BY_RULES(nPatientID, adwResourceID[i], nLocationID, dtDate, dtStartTime, dtEndTime, nAptType, -1,
						nIgnoreApptID, &strFatalWarnings, &strNonfatalWarnings);
				}
				else
				{
					for (int j=0; j < adwAptPurpose.GetSize(); j++)
					{
						VALIDATE_BY_RULES(nPatientID, adwResourceID[i], nLocationID, dtDate, dtStartTime, dtEndTime, nAptType, adwAptPurpose[j],
							nIgnoreApptID, &strFatalWarnings, &strNonfatalWarnings);
					}
				}
			}
			prsAppts->MoveNext();
		}

		// Now display the warnings
		if (nTotalRuleWarningResult & rwHasWarnings) {
			UINT nResult = IDNO;
			if (!strCompleteNonFatalWarnings.IsEmpty()) {
				nResult = MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This action triggers the following warnings:\n\n%s\nWould you like to save it anyway?", strCompleteNonFatalWarnings);
			} else {
				nResult = MsgBox(MB_ICONEXCLAMATION|MB_YESNO, "This action triggers at least one unspecified warning.\n\nWould you like to save it anyway?");
			}
			if (nResult == IDYES) {
				// The user wants to save in spite of the warnings
				return FALSE;
			} else {
				// The user did not want to save because of the warnings
				return TRUE;
			}
		}
	}
	NxCatchAll("Error checking linked appts against templates");
	return FALSE;
}

// (a.wilson 2014-08-21 14:33) - PLID 63170 - use the refreshappointmenttable() with all the parameters to prevent extra recordsets.
BOOL CResLinkActionDlg::MoveLinkedAppointments()
{
	if (!m_nDurationChange)
		return TRUE;

	try
	{
		_RecordsetPtr prsAppts;
		CString strRestrict;

		for (long i=0; i < m_dlAptList->GetRowCount(); i++)
		{
			if (VarBool(m_dlAptList->GetValue(i, eColCheckbox)))
				break;
		}
		if (i == m_dlAptList->GetRowCount())
		{
			MsgBox("Please select at least one linked appointment before continuing");
			return FALSE;
		}

		if (LinkedApptsViolateTemplates())
			return FALSE;

		strRestrict.Format("AptLinkT.AppointmentID <> %d ", m_nResID);
		for (i=0; i < m_dlAptList->GetRowCount(); i++)
		{
			if (!VarBool(m_dlAptList->GetValue(i, eColCheckbox)))
			{
				CString str;
				str.Format("AND AptLinkT.AppointmentID <> %d ", VarLong(m_dlAptList->GetValue(i, eColApptID)));
				strRestrict += str;
			}
		}
		prsAppts = CreateRecordset("SELECT AppointmentID, PatientID, Date, StartTime, EndTime, ArrivalTime, "
			"Status, ShowState, LocationID, dbo.GetResourceIDString(AppointmentID) AS ResourceIDs "
			"FROM AptLinkT LEFT JOIN AppointmentsT ON AppointmentsT.ID = AptLinkT.AppointmentID "
			"WHERE AptLinkT.GroupID IN (SELECT GroupID FROM AptLinkT WHERE AppointmentID = %d) "
			"AND %s AND AppointmentsT.Status <> 4",
			m_nResID, strRestrict);

		while (!prsAppts->eof)
		{
			COleDateTime dtOldDate = AdoFldDateTime(prsAppts, "Date");
			COleDateTime dtOldStart = AdoFldDateTime(prsAppts, "StartTime");
			COleDateTime dtOldEnd = AdoFldDateTime(prsAppts, "EndTime");
			COleDateTime dtOldArrival = AdoFldDateTime(prsAppts, "ArrivalTime");
			long nPatientID = AdoFldLong(prsAppts, "PatientID");
			long nCurID = AdoFldLong(prsAppts, "AppointmentID");
			long nLocationID = AdoFldLong(prsAppts, "LocationID");
			long nStatus = (long)AdoFldByte(prsAppts, "Status");
			long nShowState = AdoFldLong(prsAppts, "ShowState");
			CString strResourceIDs = AdoFldString(prsAppts, "ResourceIDs", "");

			COleDateTimeSpan dtsDurationChange(0, 0, m_nDurationChange, 0);
			COleDateTime dtNewStart = dtOldStart + dtsDurationChange;
			COleDateTime dtNewEnd = dtOldEnd + dtsDurationChange;
			COleDateTime dtNewArrival = dtOldArrival + dtsDurationChange;
			// Make sure start time and arrival time have the same date.
			if(dtNewStart.GetDay() != dtNewArrival.GetDay()) {
				dtNewArrival.SetDateTime(dtNewStart.GetYear(), dtNewStart.GetMonth(), dtNewStart.GetDay(), 0, 0, 0);
			}
			// (c.haag 2010-04-01 15:10) - PLID 38005 - Delete from the appointment reminder table
			ExecuteSql("UPDATE AppointmentsT SET StartTime = DATEADD(minute, %d, StartTime), "
				"EndTime = DATEADD(minute, %d, EndTime),  "
				"ArrivalTime = '%s' WHERE ID = %d; "
				"DELETE FROM AppointmentRemindersT WHERE AppointmentID = %d "
				,m_nDurationChange, m_nDurationChange, FormatDateTimeForSql(dtNewArrival), nCurID, nCurID);
			ExecuteSql("UPDATE AppointmentsT SET Date = CONVERT(datetime, CONVERT(nvarchar, StartTime, 101)) WHERE ID = %d", nCurID);

			//DRT 7/8/2005 - PLID 16664 - If the days changed, and there are superbill IDs tied to this appointment, give the user the option of 
			//	marking those superbills VOID.
			COleDateTime dtNewDate = dtOldDate + COleDateTimeSpan(0, 0, m_nDurationChange, 0);
			//compare YMD - if any are different it moved days.
			if( (dtNewDate.GetYear() != dtOldDate.GetYear() || dtNewDate.GetMonth() != dtOldDate.GetMonth() || dtNewDate.GetDay() != dtOldDate.GetDay()) && (GetRemotePropertyInt("ShowVoidSuperbillPrompt", 1, 0, "<None>", false) == 1) && (GetCurrentUserPermissions(bioVoidSuperbills) & sptWrite)) {
				//The date has changed, now look for superbills
				_RecordsetPtr prsSuperbill = CreateRecordset("SELECT SavedID FROM PrintedSuperbillsT WHERE ReservationID = %li AND Void = 0", nCurID);
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
					long nRow = m_dlAptList->FindByColumn(0, (long)nCurID, 0, VARIANT_FALSE);
					CString strPurp, strDate;
					if(nRow > sriNoRow) {
						strPurp = VarString(m_dlAptList->GetValue(nRow, 9), "");
						strDate = FormatDateTimeForInterface(VarDateTime(m_dlAptList->GetValue(nRow, 6)));
					}

					CString strFmt;
					strFmt.Format("This appointment (%s for %s) is tied to %li superbill%s (ID%s:  %s).  "
						"Do you wish to mark these superbills as VOID?\r\n\r\n"
						" - If you choose YES, all related superbill IDs will be marked VOID.\r\n"
						" - If you choose NO, the superbills will remain tied to this appointment.", 
						strPurp, strDate, nSuperbillIDCnt, nSuperbillIDCnt == 1 ? "" : "s", nSuperbillIDCnt == 1 ? "" : "s", strSuperbillIDs);

					if(AfxMessageBox(strFmt, MB_YESNO) == IDYES) {
						//void these superbills
						ExecuteSql("UPDATE PrintedSuperbillsT SET Void = 1, VoidDate = GetDate(), VoidUser = '%s' WHERE PrintedSuperbillsT.ReservationID = %li AND Void = 0", _Q(GetCurrentUserName()), nCurID);
					}
				}
			}

			//update case history dates, if needed.
			if(IsSurgeryCenter(false)) {
				COleDateTime dtNewDate, dtNewStartTime, dtNewEndTime;
				_RecordsetPtr rs = CreateRecordset("SELECT Date, StartTime, EndTime, PatientID FROM AppointmentsT WHERE ID = %li",nCurID);
				if(!rs->eof) {
					dtNewDate = AdoFldDateTime(rs, "Date");
					dtNewStartTime = AdoFldDateTime(rs, "StartTime");
					dtNewEndTime = AdoFldDateTime(rs, "EndTime");

					// (z.manning, 10/27/05, PLID 17501) - Audit when linked appts are moved.
					long nAuditID = BeginNewAuditEvent();
					CString strOldStart = FormatDateTimeForInterface(dtOldStart);
					CString strOldEnd = FormatDateTimeForInterface(dtOldEnd);
					CString strOldArrival = FormatDateTimeForInterface(dtOldArrival);
					CString strNewStart = FormatDateTimeForInterface(dtNewStartTime);
					CString strNewEnd = FormatDateTimeForInterface(dtNewEndTime);
					CString strNewArrival = FormatDateTimeForInterface(dtNewArrival);
					CString strPatientName = GetExistingPatientName(AdoFldLong(rs->Fields->Item["PatientID"]));
					if(strNewStart != strOldStart) { //These are formatted in the same way above, so they should match if the same
						AuditEvent(AdoFldLong(rs->Fields->Item["PatientID"]), strPatientName, nAuditID, aeiApptStartTime, nCurID, strOldStart, strNewStart, aepHigh);
					}
					if(strNewEnd != strOldEnd) {
						AuditEvent(AdoFldLong(rs->Fields->Item["PatientID"]), strPatientName, nAuditID, aeiApptEndTime, nCurID, strOldEnd, strNewEnd, aepHigh);
					}
					if(strNewArrival != strOldArrival) {
						AuditEvent(AdoFldLong(rs->Fields->Item["PatientID"]), strPatientName, nAuditID, aeiApptArrivalTime, nCurID, strOldArrival, strNewArrival, aepMedium);
					}
					// Make sure the new appointment location is valid
					long nNewLocationID = nLocationID;
					if (GetValidAppointmentLocation(nLocationID, nNewLocationID, nCurID, dtNewDate, dtNewStart))
					{
						// If the appointment was dropped into a new location template, make sure the LocationID is updated
						if (nLocationID != nNewLocationID)
						{
							// Audit the location change for the appointment
							CString strOldLocation = GetLocationName(nLocationID);
							CString strNewLocation = GetLocationName(nNewLocationID);
							AuditEvent(AdoFldLong(rs->Fields->Item["PatientID"]), strPatientName, nAuditID, aeiApptLocation, nCurID, strOldLocation, strNewLocation, aepHigh);
							nLocationID = nNewLocationID;
							ExecuteParamSql("UPDATE AppointmentsT SET LocationID = {INT} WHERE ID = {INT};", nLocationID, nCurID);
						}
					}
				}
				rs->Close();

				if(dtOldDate != dtNewDate) {
					COleDateTime dtDateOnly;
					dtDateOnly.SetDate(dtNewDate.GetYear(),dtNewDate.GetMonth(),dtNewDate.GetDay());
					long count = 0;
					_RecordsetPtr rs = CreateRecordset("SELECT Count(ID) AS CountOfID FROM CaseHistoryT WHERE AppointmentID = %li AND SurgeryDate <> '%s'", nCurID, FormatDateTimeForSql(dtDateOnly,dtoDate));
					if(!rs->eof) {
						count = AdoFldLong(rs, "CountOfID",0);
					}
					rs->Close();
					if(count > 0) {
						CString str;
						if(count > 1) {
							str.Format("There are %li case histories that are attached to one of your linked appointments and specify a different surgery date.\n"
								"Would you like to update the surgery date on these case histories to reflect the new appointment date?",count);
						}
						else {
							str = "There is a case history that is attached to one of your linked appointments and specifies a different surgery date.\n"
								"Would you like to update the surgery date on this case history to reflect the new appointment date?";
						}
						if(IDYES == MsgBox(MB_ICONQUESTION|MB_YESNO,str)) {
							//TES 1/9/2007 - PLID 23575 - Go through each one and audit.
							// (a.walling 2008-06-04 15:26) - PLID 29900 - Get the patientID here
							_RecordsetPtr rsCaseHistories = CreateRecordset("SELECT ID, SurgeryDate, PersonID FROM CaseHistoryT WHERE AppointmentID = %li AND SurgeryDate <> '%s'", nCurID, FormatDateTimeForSql(dtDateOnly,dtoDate));
							while(!rsCaseHistories->eof) {
								long nCaseHistoryID = AdoFldLong(rsCaseHistories, "ID");
								_variant_t varOldDate = rsCaseHistories->Fields->GetItem("SurgeryDate")->Value;
								long nCaseHistoryPatientID = AdoFldLong(rsCaseHistories, "PersonID");
								CString strOldDate;
								if(varOldDate.vt == VT_DATE) {
									strOldDate = FormatDateTimeForInterface(VarDateTime(varOldDate), NULL, dtoDate);
								}
								else {
									strOldDate = "<None>";
								}

								ExecuteSql("UPDATE CaseHistoryT SET SurgeryDate = '%s' WHERE ID = %li",FormatDateTimeForSql(dtDateOnly,dtoDate), nCaseHistoryID);
								// (a.walling 2008-06-04 15:23) - PLID 29900 - Use correct patient name
								AuditEvent(nCaseHistoryPatientID, GetExistingPatientName(nCaseHistoryPatientID), BeginNewAuditEvent(), aeiCaseHistorySurgeryDate, nCaseHistoryID, strOldDate, FormatDateTimeForInterface(dtDateOnly,NULL,dtoDate), aepMedium, aetChanged);
								rsCaseHistories->MoveNext();
							}
						}
					}
				}
			}

			// Update our external links
			UpdatePalmSyncT(nCurID);
			PPCModifyAppt(nCurID);

			// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
			// (r.gonet 12/03/2012) - PLID 54107 - Changed to use refactored send function.
			SendUpdateAppointmentHL7Message(nCurID);

			//CClient::RefreshTable(NetUtils::AppointmentsT, nCurID);
			CClient::RefreshAppointmentTable(nCurID, nPatientID, dtNewStart, dtNewEnd, nStatus, nShowState, nLocationID, strResourceIDs);

			// TODO: How do we audit this?
			// (a.wilson 2014-08-12 14:48) - PLID 63199 - unneccessary.
			//GetMainFrame()->PostMessage(WM_TABLE_CHANGED, NetUtils::AppointmentsT, nCurID);
			prsAppts->MoveNext();
		}
		return TRUE;
	}
	NxCatchAll("Error moving linked appointments");
	return FALSE;
}

void CResLinkActionDlg::OnOK()
{
	try {
		UpdateData(TRUE);

		switch ((ERecourse)m_iRecourse)
		{
		case eMoveLinkedAppts: // Auto-move all the linked appointments
			if (!m_nDurationChange)
				break;
			if (!MoveLinkedAppointments())
				return;
			break;
		case eMakeTodo: // Make a todo reminder
			try
			{
				_RecordsetPtr prs = CreateRecordset("SELECT PatientID, CASE WHEN AptTypeT.ID IS NULL THEN 'No Type' ELSE AptTypeT.Name END AS TypeName, StartTime "
					"FROM AppointmentsT LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID WHERE AppointmentsT.ID = %d", m_nResID);
				CString strNotes;
				long nTodoID;
				COleDateTime dtAppt = AdoFldDateTime(prs, "StartTime");

				strNotes.Format("Changed appt. (%s) now scheduled for %s; need to change related appts.",
					AdoFldString(prs, "TypeName", ""), FormatDateTimeForInterface(dtAppt,DTF_STRIP_SECONDS, dtoDateTime));

				long nCatID = -1;
				_RecordsetPtr rs = CreateRecordset("SELECT ID FROM NoteCatsF WHERE Description = 'Appointments'");
				if(!rs->eof) {
					nCatID = rs->Fields->Item["ID"]->Value.lVal;
				}
				else {
					//TES 8/1/2011 - PLID 44716 - Moved to GlobalUtils function
					nCatID = CreateNoteCategory("Appointments");
				}
				rs->Close();

				// (c.haag 2008-06-09 11:19) - PLID 30321 - Use the new global todo create function
				nTodoID = TodoCreate(COleDateTime::GetCurrentTime(), COleDateTime::GetCurrentTime(), GetCurrentUserID(), strNotes, "Phone", AdoFldLong(prs, "PatientID"), ttPatientContact,
					AdoFldLong(prs, "PatientID"), -1, ttpLow, nCatID);
				// (s.tullis 2014-08-21 10:09) - 63344 -Changed to Ex Todo
				CClient::RefreshTodoTable(nTodoID, AdoFldLong(prs, "PatientID",-1), GetCurrentUserID(), TableCheckerDetailIndex::tddisAdded);
			}
			NxCatchAll("Error creating todo reminder");
			break;
/*		case eGoToAppt: // Go to an appointment
			if (m_dlAptList->CurSel != -1)
			{
				long nResID = m_dlAptList->GetValue(m_dlAptList->CurSel, 0).lVal;

				CNxTabView *pView = GetMainFrame()->GetActiveView();
				if (pView && pView->IsKindOf(RUNTIME_CLASS(CSchedulerView))) {
					((CSchedulerView*)pView)->OpenAppointment(nResID);
				}
			}
			else {
				//no current selection
				AfxMessageBox("Please select an item to go to first.");
				return;
			}
			break;*/
		case eDoNothing: // Do nothing
			break;
		default:
			break;
		}
		CNxDialog::OnOK();
	
	}NxCatchAll("Error in OnOK()");
}

void CResLinkActionDlg::OnCancel() {
	//do nothing, we don't want hitting the escape key to close
}

void CResLinkActionDlg::OnDblClickCellAppointments(long nRowIndex, short nColIndex) 
{
	m_nActiveLinkedID = VarLong(m_dlAptList->GetValue(nRowIndex, eColApptID));
	OnGoAppointment();
}

void CResLinkActionDlg::OnRButtonDownAppointments(long nRow, short nCol, long x, long y, long nFlags) 
{
	//DRT 1/12/2004 - PLID 10659 - Can't work on row -1...
	if(nRow == -1)
		return;

	m_dlAptList->CurSel = nRow;

	m_nActiveLinkedID = VarLong(m_dlAptList->GetValue(nRow, eColApptID));
	CMenu* pMenu;
	pMenu = new CMenu;
	pMenu->CreatePopupMenu();
	pMenu->InsertMenu(-1, MF_BYPOSITION, ID_LINKACTION_GOTOAPPT, "&Go to Appointment");
	CPoint pt;
	GetCursorPos(&pt);
	pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, this, NULL);
	delete pMenu;
}

void CResLinkActionDlg::OnGoAppointment()
{
	try {
		switch (MsgBox(MB_YESNOCANCEL, "Would you like to move all the selected linked appointments before going to this appointment?"))
		{
		case IDYES:
			if (!MoveLinkedAppointments())
				return;
			break;
		case IDNO:
			break;
		case IDCANCEL:
			return;
		}

		CNxTabView *pView = GetMainFrame()->GetActiveView();
		if (pView && pView->IsKindOf(RUNTIME_CLASS(CSchedulerView))) {
			((CSchedulerView*)pView)->OpenAppointment(m_nActiveLinkedID);
		}
		CNxDialog::OnOK();
	}
	NxCatchAll("An error occured while going to the appointment");
}
// AttendanceEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ContactsRc.h"
#include "AttendanceEntryDlg.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

enum EUserListColumns
{
	// (z.manning 2008-11-18 11:04) - PLID 31782 - Changed column 0 to ID instead of a pointer
	ulcID = 0,
	ulcName,
};

/////////////////////////////////////////////////////////////////////////////
// CAttendanceEntryDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29145 - Created
// (d.thompson 2014-02-27) - PLID 61016 - Ripped out all sick hours times entirely, they are no longer part of this dialog.

CAttendanceEntryDlg::CAttendanceEntryDlg(AttendanceInfo *pInfo, CWnd* pParent)
	: CNxDialog(CAttendanceEntryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAttendanceEntryDlg)
	//}}AFX_DATA_INIT

	m_pAttendanceInfo = pInfo;
	m_pOriginalAttendanceInfo = pInfo;
	m_pAttendanceData = NULL;
	m_bReadOnly = FALSE;
}

CAttendanceEntryDlg::~CAttendanceEntryDlg()
{
	// (z.manning 2008-11-18 10:44) - PLID 31782 - If the current attendance data is not the
	// same as the one from CAttendanceDlg then we don't need it anymore when this dialog closes.
	if(m_pAttendanceInfo->m_nYear != m_pOriginalAttendanceInfo->m_nYear) {
		delete m_pAttendanceInfo;
		m_pAttendanceInfo = NULL;
	}
}


void CAttendanceEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAttendanceEntryDlg)
	DDX_Control(pDX, IDC_ATTENDANCE_DATE_END, m_dtpDateEnd);
	DDX_Control(pDX, IDC_EMAIL_MANAGERS, m_btnEmailMgrs);
	DDX_Control(pDX, IDC_FLOATING_HOLIDAY, m_btnFloating);
	DDX_Control(pDX, IDC_SAVE_ATTENDANCE_APPOINTMENT, m_btnSave);
	DDX_Control(pDX, IDC_REQUEST_ATTENDANCE_APPOINTMENT, m_btnRequest);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ATTENDANCE_DATE_START, m_dtpDateStart);
	DDX_Control(pDX, IDC_VACATION_HOURS, m_nxeditVacationHours);
	DDX_Control(pDX, IDC_HOURS_WORKED, m_nxeditHoursWorked);
	DDX_Control(pDX, IDC_OTHER_HOURS, m_nxeditOtherHours);
	DDX_Control(pDX, IDC_ATTENDANCE_ENTRY_NOTES, m_nxeditAttendanceEntryNotes);
	DDX_Control(pDX, IDC_APPROVED_BY, m_nxeditApprovedBy);
	DDX_Control(pDX, IDC_VACATION_AVAILABLE, m_nxeditVacationAvailable);
	DDX_Control(pDX, IDC_VACATION_ACCRUED, m_nxeditVacationAccrued);
	DDX_Control(pDX, IDC_VACATION_USED, m_nxeditVacationUsed);
	DDX_Control(pDX, IDC_VACATION_BALANCE, m_nxeditVacationBalance);
	DDX_Control(pDX, IDC_ATTENDANCE_INPUT_DATE, m_nxeditAttendanceInputDate);
	DDX_Control(pDX, IDC_STATIC_VACATION, m_nxstaticVacation);
	DDX_Control(pDX, IDC_STATIC_HOURS_WORKED, m_nxstaticHoursWorked);
	DDX_Control(pDX, IDC_STATIC_YEAR, m_nxstaticYear);
	DDX_Control(pDX, IDC_DATE_THROUGH_LABEL, m_nxstaticDateThroughLabel);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CAttendanceEntryDlg, IDC_ATTENDANCE_DATE_START, 2 /* Change */, OnChangeAttendanceDateStart, VTS_NONE)

// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
BEGIN_MESSAGE_MAP(CAttendanceEntryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAttendanceEntryDlg)
	ON_BN_CLICKED(IDC_REQUEST_ATTENDANCE_APPOINTMENT, OnRequestAttendanceAppointment)
	ON_BN_CLICKED(IDC_SAVE_ATTENDANCE_APPOINTMENT, OnSaveAttendanceAppointment)
	ON_EN_KILLFOCUS(IDC_VACATION_HOURS, OnKillfocusVacationHours)
	ON_EN_KILLFOCUS(IDC_OTHER_HOURS, OnKillfocusOtherHours)
	ON_EN_KILLFOCUS(IDC_UNPAID_HOURS, OnKillfocusUnpaidHours)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_ATTENDANCE_DATE_START, OnChangeAttendanceDateStart)
	ON_EN_KILLFOCUS(IDC_HOURS_WORKED, OnKillfocusHoursWorked)
	// (z.manning 2009-08-03 10:09) - PLID 32907 - No more floating holiday
	//ON_BN_CLICKED(IDC_FLOATING_HOLIDAY, OnFloatingHoliday)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_ATTENDANCE_DATE_END, OnDatetimechangeAttendanceDateEnd)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CAttendanceEntryDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAttendanceEntryDlg)
	ON_EVENT(CAttendanceEntryDlg, IDC_USERS_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedUsersCombo, VTS_I2)
	ON_EVENT(CAttendanceEntryDlg, IDC_USERS_COMBO, 16 /* SelChosen */, OnSelChosenUsersCombo, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttendanceEntryDlg message handlers

BOOL CAttendanceEntryDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		// (d.thompson 2013-01-24) - PLID 54811 - Bulk cache properties
		g_propManager.CachePropertiesInBulk("AttendanceEntry-Int", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'AttendanceEmailManagers' "
			")",
			_Q(GetCurrentUserName()));

		m_pdlUsers = BindNxDataList2Ctrl(this, IDC_USERS_COMBO, GetRemoteData(), false);

		m_btnRequest.AutoSet(NXB_OK);
		m_btnSave.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (z.manning, 12/05/2007) - Populate our user combo with all users.
		for(int nUserIndex = 0; nUserIndex < m_pAttendanceInfo->m_arypAttendanceUsers.GetSize(); nUserIndex++)
		{
			AttendanceUser *pUser = m_pAttendanceInfo->m_arypAttendanceUsers.GetAt(nUserIndex);
			IRowSettingsPtr pRow = m_pdlUsers->GetNewRow();
			pRow->PutValue(ulcID, pUser->m_nUserID);
			pRow->PutValue(ulcName, _bstr_t(pUser->GetFullName()));
			m_pdlUsers->AddRowSorted(pRow, NULL);
		}

		try
		{
			ASSERT(m_pAttendanceData->pUser->m_nUserID > 0);
			if(m_pdlUsers->SetSelByColumn(ulcID, m_pAttendanceData->pUser->m_nUserID) == NULL) {
				ThrowNxException("Invalid user (ID = %li) on attendance appointment ID of %li", m_pAttendanceData->pUser->m_nUserID, m_pAttendanceData->nID);
			}

		}NxCatchAllCall("CAttendanceEntryDlg::OnInitDialog (Select user for existing appointment)", EndDialog(IDCANCEL));

		BOOL bIsCurrentUserManager = m_pAttendanceInfo->GetAttendanceUserByID(GetCurrentUserID())->IsManager();
		BOOL bIsCurrentUserAdmin = CheckCurrentUserPermissions(bioAttendance, sptDynamic0, FALSE, 0, TRUE);

		// (z.manning, 02/27/2008) - PLID 28295 - Managers and admins can save attendance appointments,
		// regular users can only request.
		if(bIsCurrentUserManager || bIsCurrentUserAdmin) {
			GetDlgItem(IDC_SAVE_ATTENDANCE_APPOINTMENT)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_REQUEST_ATTENDANCE_APPOINTMENT)->ShowWindow(SW_HIDE);
		}
		else {
			GetDlgItem(IDC_SAVE_ATTENDANCE_APPOINTMENT)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_REQUEST_ATTENDANCE_APPOINTMENT)->ShowWindow(SW_SHOW);
		}

		((CNxEdit*)GetDlgItem(IDC_ATTENDANCE_ENTRY_NOTES))->SetLimitText(255);

		Load();

		// (z.manning, 05/19/2008) - PLID 30102 - Don't check this option by default for hourly employees
		if(IsNew() && m_pAttendanceData->pUser->m_eType != autHourly) {
			// (d.thompson 2013-01-24) - PLID 54811 - Remember the setting whether it's checked or not.  Default on.
			long nChecked = GetRemotePropertyInt("AttendanceEmailManagers", 1, 0, GetCurrentUserName(), true);
			CheckDlgButton(IDC_EMAIL_MANAGERS, nChecked);
		}

		UpdateView();

	}NxCatchAll("CAttendanceEntryDlg::OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAttendanceEntryDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// (a.walling 2008-05-14 12:39) - PLID 27591 - VarDateTime no longer necessary
	COleDateTime dtStartDate = (m_dtpDateStart.GetValue());
	COleDateTime dtEndDate = m_dtpDateEnd.GetValue();
	dtStartDate.SetDate(dtStartDate.GetYear(), dtStartDate.GetMonth(), dtStartDate.GetDay());
	dtEndDate.SetDate(dtEndDate.GetYear(), dtEndDate.GetMonth(), dtEndDate.GetDay());

	SetDlgItemText(IDC_STATIC_YEAR, FormatString("%i", dtStartDate.GetYear()));

	BOOL bIsCurrentUserManager = m_pAttendanceInfo->GetAttendanceUserByID(GetCurrentUserID())->IsManager();
	BOOL bIsCurrentUserAdmin = CheckCurrentUserPermissions(bioAttendance, sptDynamic0, FALSE, 0, TRUE);

	IRowSettingsPtr pUserRow = m_pdlUsers->GetCurSel();
	if(pUserRow != NULL)
	{
		AttendanceUser *pUser = m_pAttendanceInfo->GetAttendanceUserByID(VarLong(pUserRow->GetValue(ulcID)));
		AttendanceInfo TempInfo;
		if(m_pAttendanceInfo->m_nYear != dtStartDate.GetYear()) {
			TempInfo.LoadAllByYear(dtStartDate.GetYear());
			pUser = TempInfo.GetAttendanceUserByID(pUser->m_nUserID);
		}

		// (z.manning, 12/05/2007) - Hide the vacation field for hourly employees.
		if(pUser->m_eType == autHourly) {
			GetDlgItem(IDC_STATIC_VACATION)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_VACATION_HOURS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_HOURS_WORKED)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_HOURS_WORKED)->ShowWindow(SW_SHOW);
			// (z.manning, 05/19/2008) - PLID 30105 - No date range for hourly employees
			GetDlgItem(IDC_DATE_THROUGH_LABEL)->ShowWindow(SW_HIDE);
			m_dtpDateEnd.ShowWindow(SW_HIDE);
		}
		else {
			ASSERT(pUser->m_eType == autSalary);
			GetDlgItem(IDC_STATIC_VACATION)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_VACATION_HOURS)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_HOURS_WORKED)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_HOURS_WORKED)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DATE_THROUGH_LABEL)->ShowWindow(SW_SHOW);
			m_dtpDateEnd.ShowWindow(SW_SHOW);
		}

		// (z.manning, 12/05/2007) - Set the usage/total values.
		SetDlgItemText(IDC_VACATION_AVAILABLE, AsString((long)pUser->GetAvailableVacation()));
		SetDlgItemText(IDC_VACATION_ACCRUED, FormatAttendanceValue(pUser->GetAccruedVacation()));
		SetDlgItemText(IDC_VACATION_USED, FormatAttendanceValue(pUser->GetTotalUsedVacation()));
		// (z.manning 2008-11-21 12:37) - PLID 32139 - We now have a function to get vacation
		// balance text that also inlucdes FH if they have their floating holiday left.
		SetDlgItemText(IDC_VACATION_BALANCE, pUser->GetBalanceVacationText());

		if(!IsNew())
		{
			// (z.manning, 02/13/2008) - PLID 28295 - They are allowed to edit an existing time off request if...
			//  A. The user is an admin
			//  B. The user is a manager is the request is not yet approved
			//  C. The request is not approved and the same user wants to edit it
			//  D. The manager who approved it can edit it so long as the pay period in which the request occurs isn't locked
			if( bIsCurrentUserAdmin || (bIsCurrentUserManager && !m_pAttendanceData->IsApproved())
				|| (m_pAttendanceData->pUser->m_nUserID == GetCurrentUserID() && !m_pAttendanceData->IsApproved()) 
				|| (bIsCurrentUserManager && VarLong(m_pAttendanceData->varApproverUserID,-1) == GetCurrentUserID() && !m_pAttendanceInfo->IsDateLocked(m_pAttendanceData->dtDate))
				)
			{
				if(dtEndDate > dtStartDate)	{
					UpdateViewForMultipleDays();
				}
				else {
					ASSERT(dtStartDate == dtEndDate);
					SetReadOnly(FALSE);
				}
			}
			else {
				SetReadOnly(TRUE);
			}

			// (z.manning, 12/06/2007) - It's an existing appointment so the user can never change.
			m_pdlUsers->PutReadOnly(VARIANT_TRUE);
			// (z.manning 2008-11-13 17:06) - PLID 31782 - Same with the dates
			m_dtpDateStart.EnableWindow(FALSE);
			m_dtpDateEnd.EnableWindow(FALSE);

			// (z.manning, 01/16/2008) - They can't change the floating holiday status on existing time off data.
			GetDlgItem(IDC_FLOATING_HOLIDAY)->EnableWindow(FALSE);
			if(IsDlgButtonChecked(IDC_FLOATING_HOLIDAY) == BST_CHECKED) {
				SetReadOnly(TRUE);
			}
		}
		else
		{
			if(dtEndDate > dtStartDate)	{
				UpdateViewForMultipleDays();
			}
			else {
				ASSERT(dtStartDate == dtEndDate);
				OnKillfocusVacationHours();
				if(IsDlgButtonChecked(IDC_FLOATING_HOLIDAY) == BST_CHECKED) {
					SetReadOnly(TRUE);
					GetDlgItem(IDC_FLOATING_HOLIDAY)->EnableWindow(TRUE);
				}
				else {
					SetReadOnly(FALSE);
				}
			}

			// (z.manning, 12/06/2007) - PLID 28295 - It's a new attendance request, so only let managers change the user.
			if(!m_bReadOnly && (bIsCurrentUserManager || bIsCurrentUserAdmin)) {
				m_pdlUsers->PutReadOnly(VARIANT_FALSE);
			}
			else {
				m_pdlUsers->PutReadOnly(VARIANT_TRUE);
			}
		}

		// (z.manning, 01/16/2008) - Floating holidays don't apply to hourly employees.
		// (z.manning 2009-08-03 10:09) - PLID 32907 - No more floating holiday
		/*if(pUser->m_eType == autHourly) {
			GetDlgItem(IDC_FLOATING_HOLIDAY)->ShowWindow(SW_HIDE);
		}
		else {
			GetDlgItem(IDC_FLOATING_HOLIDAY)->ShowWindow(SW_SHOW);
		}*/
	}
}

void CAttendanceEntryDlg::UpdateViewForMultipleDays()
{
	// (z.manning, 05/19/2008) - PLID 30105 - We now support date ranges, but to avoid situations
	// where users enter odd values that don't make sense, we only support it for vacation and
	// they can't enter the hours manually (it's hardcoded to 8 hours per weekday).
	SetReadOnly(TRUE);
	// (z.manning, 05/20/2008) - PLID 30105 - Do not allow users to change the date range on existing time
	// off requests.
	m_dtpDateStart.EnableWindow(IsNew());
	m_dtpDateEnd.EnableWindow(IsNew());
	m_nxeditAttendanceEntryNotes.SetReadOnly(FALSE);
	SetDlgItemInt(IDC_OTHER_HOURS, 0);
	SetDlgItemInt(IDC_UNPAID_HOURS, 0);
	if(IsNew()) {
		// (z.manning 2011-08-02 12:48) - PLID 44524 - If this is a new request then just set the total vacation to
		// 8 hours per day off. Any handling of this user being over the allowed vacation amount will be handled when saving.
		SetDlgItemInt(IDC_VACATION_HOURS, GetActualDaysOff() * 8);
	}
	else {
		// (z.manning 2011-08-02 12:49) - PLID 44524 - This is an existing time off request so use the values loaded from data
		// since you are currently unable to change existing requests that span multiple days.
		SetDlgItemText(IDC_VACATION_HOURS, FormatAttendanceValue(GetActualDaysOff() * m_pAttendanceData->cyVacation));
		SetDlgItemText(IDC_UNPAID_HOURS, FormatAttendanceValue(GetActualDaysOff() * m_pAttendanceData->cyUnpaid));
	}
}

void CAttendanceEntryDlg::OnRequeryFinishedUsersCombo(short nFlags)
{
	try
	{
		

	}NxCatchAll("CAttendanceEntryDlg::OnRequeryFinishedUsersCombo");
}

BOOL CAttendanceEntryDlg::IsNew()
{
	return (m_pAttendanceData->nID == -1);
}


BOOL CAttendanceEntryDlg::Validate()
{
	BOOL bIsCurrentUserAdmin = CheckCurrentUserPermissions(bioAttendance, sptDynamic0, FALSE, 0, TRUE);

	CString strNote;
	GetDlgItemText(IDC_ATTENDANCE_ENTRY_NOTES, strNote);
	if(strNote.GetLength() > 255) {
		// (z.manning, 12/03/2007) - The edit box should have a text limit.
		ASSERT(FALSE);
		SetDlgItemText(IDC_ATTENDANCE_ENTRY_NOTES, strNote.Left(255));
	}

	IRowSettingsPtr pUserRow = m_pdlUsers->GetCurSel();
	if(pUserRow == NULL) {
		MessageBox("You must select a user.");
		return FALSE;
	}

	// (a.walling 2008-05-14 12:39) - PLID 27591 - VarDateTime no longer necessary
	COleDateTime dtStartDate = (m_dtpDateStart.GetValue());
	COleDateTime dtEndDate = m_dtpDateEnd.GetValue();
	AttendanceUser *pUser = m_pAttendanceInfo->GetAttendanceUserByID(VarLong(pUserRow->GetValue(ulcID)));
	if(m_pAttendanceData->nID == -1) {
		// (z.manning 2010-09-09 13:22) - PLID 40456 - This needs to check data, not memory which may not be
		// up-to-date with changes made by other users.
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			"SELECT TOP 1 Date FROM AttendanceAppointmentsT \r\n"
			"WHERE UserID = {INT} AND Date >= {OLEDATETIME} AND Date <= {OLEDATETIME} \r\n"
			, pUser->m_nUserID, dtStartDate, dtEndDate);
		if(!prs->eof) {
			COleDateTime dt = AdoFldDateTime(prs->GetFields(), "Date");
			MessageBox(FormatString("There is already attendance data for %s on %s. You must either edit the existing one or have a manager edit it for you.", pUser->GetFullName(), FormatDateTimeForInterface(dt,0,dtoDate)));
			return FALSE;
		}
	}

	// (z.manning, 05/19/2008) - PLID 30105 - Ensure the end time is after the start time
	if(dtEndDate < dtStartDate) {
		MessageBox("The end date can not be before the start date.");
		return FALSE;
	}

	if(dtEndDate.GetYear() != dtStartDate.GetYear()) {
		MessageBox("The start date and end date must be in the same year.");
		return FALSE;
	}

	// (z.manning, 02/12/2008) - PLID 28885 - Do lot let non-admins create or request any time off during a pay
	// period that has already been closed.
	if(m_pAttendanceInfo->IsDateLocked(dtStartDate) && !bIsCurrentUserAdmin) {
		MessageBox(FormatString("Payroll for %s has already been closed, so you may not enter any time off then.", FormatDateTimeForInterface(dtStartDate,0,dtoDate)));
		return FALSE;
	}

	// (z.manning, 02/08/2008) - Only 1 floating holiday per user.
	if(IsNew() && IsDlgButtonChecked(IDC_FLOATING_HOLIDAY) == BST_CHECKED)
	{
		// (z.manning 2009-08-03 10:09) - PLID 32907 - No more floating holiday
		//if(pUser->GetFloatingHolidayCount() > 0) {
			MessageBox("You have already used your maximum allowance of floating holidays.");
			return FALSE;
		//}
	}

	CString strVacation, strHoursWorked, strUnpaid;
	GetDlgItemText(IDC_VACATION_HOURS, strVacation);
	// (z.manning 2011-08-02 11:44) - PLID 44524 - Other has been removed
	//GetDlgItemText(IDC_OTHER_HOURS, strOther);
	GetDlgItemText(IDC_UNPAID_HOURS, strUnpaid);
	GetDlgItemText(IDC_HOURS_WORKED, strHoursWorked);
	// (z.manning, 12/03/2007) - These should all already be valid currency values as we ensure that when
	// killing focus from each of these fields.
	COleCurrency cyVacation, cyHoursWorked, cyUnpaid;
	if(!cyVacation.ParseCurrency(strVacation)) {
		ASSERT(FALSE);
		cyVacation.SetCurrency(0, 0);
	}
	// (z.manning 2011-08-02 11:49) - PLID 44524 - Replaced other with unpaid
	/*if(!cyOther.ParseCurrency(strOther)) {
		ASSERT(FALSE);
		cyOther.SetCurrency(0, 0);
	}*/
	if(!cyUnpaid.ParseCurrency(strUnpaid)) {
		ASSERT(FALSE);
		cyUnpaid.SetCurrency(0, 0);
	}
	if(!cyHoursWorked.ParseCurrency(strHoursWorked)) {
		ASSERT(FALSE);
		cyHoursWorked.SetCurrency(0, 0);
	}

	// (z.manning, 12/18/2007) - Make sure they have entered something or there's no point to saving it.
	strNote.TrimRight();
	COleCurrency cyZero(0, 0);
	if(strNote.IsEmpty() && cyVacation == cyZero && cyUnpaid == cyZero && cyHoursWorked == cyZero) {
		MessageBox("You can not save blank time off requests.");
		return FALSE;
	}

	// (z.manning, 04/11/2008) - PLID 29628 - No editing hours for any date other than the current one.
	COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
	dtCurrent.SetDate(dtCurrent.GetYear(), dtCurrent.GetMonth(), dtCurrent.GetDay());
	if(cyHoursWorked > cyZero && dtStartDate != dtCurrent) {
		MessageBox("You can only enter hours worked for the current date.");
		return FALSE;
	}

	// (z.manning, 02/11/2008) - If they entered sick or other time off then require notes.
	if(cyUnpaid > cyZero && strNote.IsEmpty()) {
		MessageBox("You must enter a note if you are taking unpaid time off.");
		return FALSE;
	}

	return TRUE;
}

void CAttendanceEntryDlg::Save(BOOL bRequesting)
{
	CString strVacation, strHoursWorked, strUnpaid;
	GetDlgItemText(IDC_VACATION_HOURS, strVacation);
	// (z.manning 2011-08-02 11:44) - PLID 44524 - Other has been replaced with unpaid
	//GetDlgItemText(IDC_OTHER_HOURS, strOther);
	GetDlgItemText(IDC_UNPAID_HOURS, strUnpaid);
	GetDlgItemText(IDC_HOURS_WORKED, strHoursWorked);
	COleDateTime dtStart = m_dtpDateStart.GetValue();
	COleDateTime dtEnd = m_dtpDateEnd.GetValue();
	long nNumberOfDays = GetActualDaysOff();
	if(nNumberOfDays <= 0) {
		nNumberOfDays = 1;
	}

	// (z.manning, 12/03/2007) - These should all already be valid currency values as we ensure that when
	// killing focus from each of these fields.
	COleCurrency cyVacation, cyOther, cyHoursWorked;
	COleCurrency cyUnpaid(0, 0), cyZero(0, 0);
	if(!cyVacation.ParseCurrency(strVacation)) {
		ASSERT(FALSE);
		cyVacation.SetCurrency(0, 0);
	}
	// (z.manning 2011-08-02 11:51) - PLID 44524 - Replaced other with unpaid
	/*if(!cyOther.ParseCurrency(strOther)) {
		ASSERT(FALSE);
		cyOther.SetCurrency(0, 0);
	}*/
	if(!cyUnpaid.ParseCurrency(strUnpaid)) {
		ASSERT(FALSE);
		cyUnpaid.SetCurrency(0, 0);
	}
	if(!cyHoursWorked.ParseCurrency(strHoursWorked)) {
		ASSERT(FALSE);
		cyHoursWorked.SetCurrency(0, 0);
	}

	m_pAttendanceData->pUser = m_pAttendanceInfo->GetAttendanceUserByID(VarLong(m_pdlUsers->GetCurSel()->GetValue(ulcID)));

	// (z.manning 2008-11-21 11:48) - PLID 32117 - If they went over their vacation or sick
	// balance move the extra amount to unpaid and warn them about it.
	if(bRequesting)
	{
		COleCurrency cyDifference = cyVacation - (m_pAttendanceData->pUser->GetBalanceVacation() + m_pAttendanceData->cyVacation * (long)GetActualDaysOff());
		if(cyVacation > cyZero && cyDifference > cyZero) {
			cyVacation -= cyDifference;
			cyUnpaid += cyDifference;
			// (d.thompson 2014-02-27) - PLID 61016 - Rename vacation to PTO
			CString strMsg = "This request will put you over your allowed PTO balance. Your requested PTO hours will show up in the unpaid column.";
			MessageBox(strMsg, NULL, MB_ICONEXCLAMATION);
		}
	}

	SetDlgItemText(IDC_UNPAID_HOURS, FormatAttendanceValue(cyUnpaid));

	// (z.manning, 12/03/2007) - If this is just a request the we need to mark it as needing approval.
	if(bRequesting) {
		m_pAttendanceData->varApproverUserID.vt = VT_NULL;
	}
	else {
		m_pAttendanceData->varApproverUserID = GetCurrentUserID();
	}
	m_pAttendanceData->cyVacation = cyVacation / nNumberOfDays;
	m_pAttendanceData->cySick = COleCurrency(0, 0);		// (d.thompson 2014-02-27) - PLID 61016 - Removed sick time from entry display
	m_pAttendanceData->cyOther = cyOther / nNumberOfDays;
	m_pAttendanceData->cyHoursWorked = cyHoursWorked / nNumberOfDays;
	// (z.manning 2008-11-21 11:49) - PLID 32117 - We may now have unpaid hours to save.
	m_pAttendanceData->cyUnpaid = cyUnpaid / nNumberOfDays;

	CString strNotes;
	GetDlgItemText(IDC_ATTENDANCE_ENTRY_NOTES, strNotes);
	m_pAttendanceData->strNotes = strNotes;

	// (a.walling 2008-05-14 12:39) - PLID 27591 - VarDateTime no longer necessary
	m_pAttendanceData->dtDate = dtStart;

	// (z.manning, 02/08/2008) - Is this a floating holiday?
	if(IsDlgButtonChecked(IDC_FLOATING_HOLIDAY) == BST_CHECKED) {
		m_pAttendanceData->eType = atFloatingHoliday;
	}
	else {
		m_pAttendanceData->eType = atNormal;
	}

	// (z.manning, 12/18/2007) - If this is new data then we need to store it in the user's data array.
	if(IsNew()) {
		m_pAttendanceData->pUser->AddAttendanceData(m_pAttendanceData);
	}

	// (z.manning, 12/03/2007) - Now save to data.
	m_pAttendanceData->SaveToData();

	// (z.manning, 05/19/2008) - PLID 30105 - We now support date ranges
	if(nNumberOfDays > 1)
	{
		for(COleDateTime dt = dtStart + ONE_DAY; dt <= dtEnd; dt += ONE_DAY)
		{
			if(IsValidVacationDay(dt))
			{
				AttendanceData *pData = m_pAttendanceData->pUser->GetAttendanceDataForDate(dt);
				long nDataID;
				if(pData == NULL) {
					pData = new AttendanceData;
					nDataID = -1;
				}
				else {
					nDataID = pData->nID;
				}
				ASSERT(m_pAttendanceData->nID > 0);
				*pData = *m_pAttendanceData;
				pData->nID = nDataID;
				pData->dtDate = dt;
				pData->varParentID = m_pAttendanceData->nID;
				if(VarLong(pData->varParentID) != m_pAttendanceData->nID) {
					ThrowNxException("CAttendanceEntryDlg::Save - Attendance appointment parent mismatch");
				}
				if(pData->nID == -1) {
					m_pAttendanceData->pUser->AddAttendanceData(pData);
				}

				pData->SaveToData();
			}
		}
	}
}

void CAttendanceEntryDlg::OnSelChosenUsersCombo(LPDISPATCH lpRow) 
{
	try
	{
		// (z.manning, 01/11/2008) - We should never be able to change the row on an existing attendance appt.
		ASSERT(IsNew());

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			ThrowNxException("CAttendanceEntryDlg::OnSelChosenUsersCombo - Invalid row selected");
		}
		else {
			m_pAttendanceData->pUser = m_pAttendanceInfo->GetAttendanceUserByID(VarLong(pRow->GetValue(ulcID)));
		}

		UpdateView();

	}NxCatchAll("CAttendanceEntryDlg::OnSelChosenUsersCombo");
}

void CAttendanceEntryDlg::OnRequestAttendanceAppointment() 
{
	try
	{
		BOOL bIsNew = IsNew();

		// (z.manning, 12/18/2007) - Kill the focus of any field we may be in because the kill focus even is necessary
		// to validate certain fields on the fly.
		// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages (now part of CNexTechDialog)
		CheckFocus();

		if(!Validate()) {
			return;
		}

		Save(TRUE);

		// (z.manning, 01/07/2008) - PLID 28461 - If the option is checked, let's email this user's managers.
		if(IsDlgButtonChecked(IDC_EMAIL_MANAGERS) == BST_CHECKED) {
			m_pAttendanceInfo->EmailManagers(m_pAttendanceData);
		}

		// (d.thompson 2013-01-24) - PLID 54811 - Remember the state of the email managers box.  Only do this for new 
		//	requests.  Edited requests will always have the checkbox off by default.
		if(bIsNew) {
			SaveSettings();
		}
		EndDialog(IDOK);

	}NxCatchAll("CAttendanceEntryDlg::OnRequestAttendanceAppointment");
}

void CAttendanceEntryDlg::OnSaveAttendanceAppointment() 
{
	try
	{
		// (z.manning, 12/18/2007) - Kill the focus of any field we may be in because the kill focus even is necessary
		// to validate certain fields on the fly.
		// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages (now part of CNexTechDialog)
		CheckFocus();

		if(!Validate()) {
			return;
		}

		BOOL bIsNew = IsNew();

		Save(FALSE);

		// (z.manning, 01/07/2008) - PLID 28461 - If the option is checked, let's email this user's managers.
		if(IsDlgButtonChecked(IDC_EMAIL_MANAGERS) == BST_CHECKED) {
			m_pAttendanceInfo->EmailManagers(m_pAttendanceData);
		}

		// (d.thompson 2013-01-24) - PLID 54811 - Remember the state of the email managers box.  Only do this for new 
		//	requests.  Edited requests will always have the checkbox off by default.
		if(bIsNew) {
			SaveSettings();
		}
		EndDialog(IDOK);

	}NxCatchAll("CAttendanceEntryDlg::OnSaveAttendanceAppointment");	
}

int CAttendanceEntryDlg::DoModal(AttendanceData *pData)
{
	try
	{
		if(pData == NULL) {
			m_pAttendanceData = new AttendanceData;
		}
		else {
			m_pAttendanceData = pData;
		}

		if(m_pAttendanceData->dtDate.GetStatus() != COleDateTime::valid) {
			COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
			m_pAttendanceData->dtDate.SetDate(dtCurrent.GetYear(), dtCurrent.GetMonth(), dtCurrent.GetDay());
		}

		return CDialog::DoModal();

	}NxCatchAll("CAttendanceEntryDlg::DoModal");

	return IDCANCEL;
}

void CAttendanceEntryDlg::Load()
{
	// (a.walling 2008-05-14 12:39) - PLID 27591 - COleVariant no longer necessary
	COleDateTime dtStart, dtEnd;
	m_pAttendanceData->GetDateRange(dtStart, dtEnd);
	m_dtpDateStart.SetValue(dtStart);
	m_dtpDateEnd.SetValue(dtEnd);
	if(!IsNew()) {
		m_pAttendanceData = m_pAttendanceData->pUser->GetAttendanceDataForDate(dtStart);
		if(m_pAttendanceData == NULL) {
			ThrowNxException("CAttendanceEntryDlg::Load - NULL attendance appointment for date %s", FormatDateTimeForInterface(dtStart,0,dtoDate));
		}
	}

	SetDlgItemText(IDC_VACATION_HOURS, FormatAttendanceValue(m_pAttendanceData->cyVacation));
	SetDlgItemText(IDC_OTHER_HOURS, FormatAttendanceValue(m_pAttendanceData->cyOther));
	SetDlgItemText(IDC_UNPAID_HOURS, FormatAttendanceValue(m_pAttendanceData->cyUnpaid)); // (z.manning 2011-08-02 11:47) - PLID 44524
	SetDlgItemText(IDC_HOURS_WORKED, FormatAttendanceValue(m_pAttendanceData->cyHoursWorked));
	SetDlgItemText(IDC_ATTENDANCE_ENTRY_NOTES, m_pAttendanceData->strNotes);
	if(m_pAttendanceData->dtInputDate.GetStatus() == COleDateTime::valid) {
		SetDlgItemText(IDC_ATTENDANCE_INPUT_DATE, FormatDateTimeForInterface(m_pAttendanceData->dtInputDate));
	}

	if(m_pAttendanceData->varApproverUserID.vt == VT_I4) {
		AttendanceUser *pApprover = m_pAttendanceInfo->GetAttendanceUserByID(VarLong(m_pAttendanceData->varApproverUserID));
		SetDlgItemText(IDC_APPROVED_BY, pApprover->GetFullName());
	}

	if(m_pAttendanceData->eType == atFloatingHoliday) {
		CheckDlgButton(IDC_FLOATING_HOLIDAY, BST_CHECKED);
	}
}

void CAttendanceEntryDlg::OnCancel() 
{
	try
	{
		// (z.manning, 11/30/2007) - If this is a new attendance appointment, but we aren't keeping it,
		// then we need to deallocate the data object.
		if(IsNew()) {
			if(m_pAttendanceData != NULL) {
				delete m_pAttendanceData;
				m_pAttendanceData = NULL;
			}
		}

		CDialog::OnCancel();

	}NxCatchAll("CAttendanceEntryDlg::OnCancel");
}

void CAttendanceEntryDlg::OnKillfocusVacationHours() 
{
	try
	{
		// (z.manning, 05/20/2008) - PLID 30105 - If the box is read-only then it may be a range of dates
		// in which case we set the value manually so no validation should be necessary.
		DWORD dwStyle = m_nxeditVacationHours.GetStyle();
		DWORD dw = dwStyle & ES_READONLY;
		if((dwStyle & ES_READONLY) == ES_READONLY) {
			return;
		}

		CString str;
		GetDlgItemText(IDC_VACATION_HOURS, str);
		EnsureValidAttendanceString(str, 24);
		SetDlgItemText(IDC_VACATION_HOURS, str);

	}NxCatchAll("CAttendanceEntryDlg::OnKillfocusVacationHours");
}

void CAttendanceEntryDlg::OnKillfocusOtherHours() 
{
	try
	{
		CString str;
		GetDlgItemText(IDC_OTHER_HOURS, str);
		EnsureValidAttendanceString(str, 24);
		SetDlgItemText(IDC_OTHER_HOURS, str);

	}NxCatchAll("CAttendanceEntryDlg::OnKillfocusOtherHours");	
}

// (z.manning 2011-08-02 11:47) - PLID 44524
void CAttendanceEntryDlg::OnKillfocusUnpaidHours() 
{
	try
	{
		CString str;
		GetDlgItemText(IDC_UNPAID_HOURS, str);
		EnsureValidAttendanceString(str, 24);
		SetDlgItemText(IDC_UNPAID_HOURS, str);

	}NxCatchAll("CAttendanceEntryDlg::OnKillfocusOtherHours");	
}

void CAttendanceEntryDlg::OnKillfocusHoursWorked() 
{
	try
	{
		CString str;
		GetDlgItemText(IDC_HOURS_WORKED, str);
		EnsureValidAttendanceString(str, 24);
		SetDlgItemText(IDC_HOURS_WORKED, str);

	}NxCatchAll("CAttendanceEntryDlg::OnKillfocusHoursWorked");
}

AttendanceData* CAttendanceEntryDlg::GetAttendanceData()
{
	return m_pAttendanceData;
}

// (a.walling 2008-05-14 12:22) - PLID 27591 - Use the new notify events
void CAttendanceEntryDlg::OnChangeAttendanceDateStart(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try
	{
		COleDateTime dtNewStart = m_dtpDateStart.GetValue();
		COleDateTime dtNewEnd = m_dtpDateEnd.GetValue();
		if(dtNewEnd < dtNewStart) {
			m_dtpDateEnd.SetValue(dtNewStart);
		}

		HandleDateChange();

	}NxCatchAll("CAttendanceEntryDlg::OnChangeAttendanceDateStart");

	*pResult = 0;
}


void CAttendanceEntryDlg::OnDatetimechangeAttendanceDateEnd(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try
	{
		COleDateTime dtNewStart = m_dtpDateStart.GetValue();
		COleDateTime dtNewEnd = m_dtpDateEnd.GetValue();
		if(dtNewEnd < dtNewStart) {
			m_dtpDateStart.SetValue(dtNewEnd);
		}

		HandleDateChange();

	}NxCatchAll("CAttendanceEntryDlg::OnDatetimechangeAttendanceDateEnd");
	
	*pResult = 0;
}

void CAttendanceEntryDlg::HandleDateChange()
{
	COleDateTime dtNewStart = m_dtpDateStart.GetValue();
	if(dtNewStart.GetYear() != m_pAttendanceInfo->m_nYear)
	{
		if(m_pAttendanceInfo->m_nYear != m_pOriginalAttendanceInfo->m_nYear) {
			delete m_pAttendanceInfo;
			m_pAttendanceInfo = NULL;
		}

		if(dtNewStart.GetYear() == m_pOriginalAttendanceInfo->m_nYear) {
			m_pAttendanceInfo = m_pOriginalAttendanceInfo;
		}
		else {
			m_pAttendanceInfo = new AttendanceInfo;
			m_pAttendanceInfo->LoadAllByYear(dtNewStart.GetYear());
		}
	}

	UpdateView();
}

void CAttendanceEntryDlg::SetReadOnly(BOOL bReadOnly)
{
	m_bReadOnly = bReadOnly;
	m_pdlUsers->PutReadOnly(bReadOnly ? VARIANT_TRUE : VARIANT_FALSE);
	m_dtpDateStart.EnableWindow(!bReadOnly);
	m_dtpDateEnd.EnableWindow(!bReadOnly);
	// (z.manning 2009-08-03 10:09) - PLID 32907 - No more floating holiday
	//GetDlgItem(IDC_FLOATING_HOLIDAY)->EnableWindow(!bReadOnly);
	((CNxEdit*)GetDlgItem(IDC_VACATION_HOURS))->SetReadOnly(bReadOnly);
	((CNxEdit*)GetDlgItem(IDC_OTHER_HOURS))->SetReadOnly(bReadOnly);
	((CNxEdit*)GetDlgItem(IDC_UNPAID_HOURS))->SetReadOnly(bReadOnly); // (z.manning 2011-08-02 11:48) - PLID 44524
	((CNxEdit*)GetDlgItem(IDC_HOURS_WORKED))->SetReadOnly(bReadOnly);
	((CNxEdit*)GetDlgItem(IDC_ATTENDANCE_ENTRY_NOTES))->SetReadOnly(bReadOnly);
}

void CAttendanceEntryDlg::OnFloatingHoliday() 
{
	try
	{
		// (z.manning, 01/16/2008) - They should only be allowed to toggle this on new requests.
		ASSERT(IsNew());

		const CString strFloatingHolidayNote = "Floating holiday";
		if(IsDlgButtonChecked(IDC_FLOATING_HOLIDAY) == BST_CHECKED)
		{
			// (z.manning, 01/16/2008) - They want this vacation request to be a floating holiday,
			// so make the dialog read-only and then force 8 hours vacation only
			SetDlgItemText(IDC_VACATION_HOURS, "8");
			SetDlgItemText(IDC_OTHER_HOURS, "0");
			SetDlgItemText(IDC_UNPAID_HOURS, "0");
			SetDlgItemText(IDC_HOURS_WORKED, "0");
			SetDlgItemText(IDC_ATTENDANCE_ENTRY_NOTES, strFloatingHolidayNote);
		}
		else {
			CString strNote;
			GetDlgItemText(IDC_ATTENDANCE_ENTRY_NOTES, strNote);
			if(strNote == strFloatingHolidayNote) {
				SetDlgItemText(IDC_ATTENDANCE_ENTRY_NOTES, "");
			}
		}
		
		UpdateView();

	}NxCatchAll("CAttendanceEntryDlg::OnFloatingHoliday");
}

int CAttendanceEntryDlg::GetActualDaysOff()
{
	COleDateTime dtStartDate = m_dtpDateStart.GetValue();
	COleDateTime dtEndDate = m_dtpDateEnd.GetValue();
	int nDayCount = 0;
	for(COleDateTime dtTemp = dtStartDate; dtTemp <= dtEndDate; dtTemp += ONE_DAY) {
		if(IsValidVacationDay(dtTemp)) {
			nDayCount++;
		}
	}

	return nDayCount;
}

// (d.thompson 2013-01-24) - PLID 54811
void CAttendanceEntryDlg::SaveSettings()
{
	// (d.thompson 2013-01-24) - PLID 54811 - Save the 'email managers' checkbox state
	long nChecked = 0;
	if(IsDlgButtonChecked(IDC_EMAIL_MANAGERS)) {
		nChecked = 1;
	}
	SetRemotePropertyInt("AttendanceEmailManagers", nChecked, 0, GetCurrentUserName());
}

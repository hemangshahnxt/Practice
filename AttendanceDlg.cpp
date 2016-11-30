// AttendanceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ContactsRc.h"
#include "AttendanceDlg.h"
#include "AttendanceEntryDlg.h"
#include "ConfigureDepartmentsDlg.h"
#include "AttendanceUserSetupDlg.h"
#include "AttendanceCustomDateRangeSetupDlg.h"
#include "InternationalUtils.h"
#include "AttendanceOptionsDlg.h"
#include "EnterActiveDateDlg.h"
#include "AuditTrail.h"		// (j.luckoski 2012-07-24 11:26) - PLID 29122 - For LockPayment audit

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

#define UNAPPROVED_FORE_COLOR RGB(255,0,0)

// (z.manning, 01/15/2008) - Have a max column width (in pixels) to prevent columns (specifically the notes column)
// from getting needlessly wide.
#define MAX_COL_WIDTH 150

#define ALL_HOURLY_DEPT_ID -99
#define ALL_SALARY_DEPT_ID -98

enum ESpecialUserRowID
{
	suriAll = -1,
	suriAnyTimeOff = -2,
	suriVacation = -3,
	suriSick = -4,
	suriOther = -5,
	suriUnapprovedTimeOff = -6, // (z.manning, 05/19/2008) - PLID 30102
	// (z.manning 2008-11-13 15:07) - PLID 31831 - Paid/Unpaid
	suriPaid = -7,
	suriUnpaid = -8,
};

/////////////////////////////////////////////////////////////////////////////
// CAttendanceDlg dialog
//
// (z.manning, 02/28/2008) - PLID 29139 - Created

CAttendanceDlg::CAttendanceDlg(CWnd* pParent)
	: CNxDialog(CAttendanceDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAttendanceDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nLastSelectedDepartmentID = -1;
	m_nLastSelectedUserID = -1;
	m_pClippedAttendanceData = NULL;
	m_bInitialLoad = TRUE;
	m_bIsCurrentUserManager = FALSE;
}

CAttendanceDlg::~CAttendanceDlg()
{
	if(m_pClippedAttendanceData != NULL) {
		delete m_pClippedAttendanceData;
	}
}


void CAttendanceDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAttendanceDlg)
	DDX_Control(pDX, IDC_RADIO_DAY, m_btnDay);
	DDX_Control(pDX, IDC_RADIO_MONTH, m_btnMonth);
	DDX_Control(pDX, IDC_RADIO_YEAR, m_btnYear);
	DDX_Control(pDX, IDC_RADIO_CUSTOM, m_btnCustom);
	DDX_Control(pDX, IDC_DATE_CURRENT_PAY_PERIOD, m_btnDateCurrentPayPeriod);
	DDX_Control(pDX, IDC_LOCK_PAYROLL, m_btnLockPayroll);
	DDX_Control(pDX, IDC_ATTENDANCE_OPTIONS, m_btnAttendanceOptions);
	DDX_Control(pDX, IDC_RESET_DATE_FILTER, m_btnResetDateFilter);
	DDX_Control(pDX, IDC_EDIT_CUSTOM_DATE_RANGE, m_btnEditCustomDateRange);
	DDX_Control(pDX, IDC_REQUEST_VACATION, m_btnRequestVacation);
	DDX_Control(pDX, IDC_USER_SETUP, m_btnUserSetup);
	DDX_Control(pDX, IDC_CONFIGURE_DEPARTMENTS, m_btnConfigureDepartments);
	DDX_Control(pDX, IDC_MOVE_NEXT_YEAR, m_btnMoveNextMonth);
	DDX_Control(pDX, IDC_MOVE_PREVIOUS_YEAR, m_btnMovePreviousMonth);
	DDX_Control(pDX, IDC_DATE_FILTER_START, m_dtpDateFilterStart);
	DDX_Control(pDX, IDC_DATE_FILTER_END, m_dtpDateFilterEnd);
	DDX_Control(pDX, IDC_ATTENDANCE_YEAR, m_nxeditAttendanceYear);
	DDX_Control(pDX, IDC_MOVE_PREVIOUS_USER, m_btnPreviousUser);
	DDX_Control(pDX, IDC_MOVE_NEXT_USER, m_btnNextUser);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CAttendanceDlg, IDC_DATE_FILTER_START, 2 /* Change */, OnChangeDateFilterStart, VTS_NONE)
//	ON_EVENT(CAttendanceDlg, IDC_DATE_FILTER_END, 2 /* Change */, OnChangeDateFilterEnd, VTS_NONE)

// (a.walling 2008-05-14 12:37) - PLID 27591 - Use Notify handlers for DateTimePicker
BEGIN_MESSAGE_MAP(CAttendanceDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAttendanceDlg)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE_FILTER_START, OnChangeDateFilterStart)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE_FILTER_END, OnChangeDateFilterEnd)
	ON_BN_CLICKED(IDC_MOVE_NEXT_YEAR, OnMoveNextYear)
	ON_BN_CLICKED(IDC_MOVE_PREVIOUS_YEAR, OnMovePreviousYear)
	ON_BN_CLICKED(IDC_RADIO_DAY, OnRadioDay)
	ON_BN_CLICKED(IDC_RADIO_MONTH, OnRadioMonth)
	ON_BN_CLICKED(IDC_RADIO_CUSTOM, OnRadioCustom)
	ON_BN_CLICKED(IDC_RADIO_YEAR, OnRadioYear)
	ON_BN_CLICKED(IDC_CONFIGURE_DEPARTMENTS, OnConfigureDepartments)
	ON_BN_CLICKED(IDC_USER_SETUP, OnUserSetup)
	ON_BN_CLICKED(IDC_REQUEST_VACATION, OnRequestVacation)
	ON_BN_CLICKED(IDC_EDIT_CUSTOM_DATE_RANGE, OnEditCustomDateRange)
	ON_BN_CLICKED(IDC_RESET_DATE_FILTER, OnResetDateFilter)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_ATTENDANCE_OPTIONS, OnAttendanceOptions)
	ON_BN_CLICKED(IDC_LOCK_PAYROLL, OnLockPayroll)
	ON_BN_CLICKED(IDC_DATE_CURRENT_PAY_PERIOD, OnDateCurrentPayPeriod)
	ON_EN_KILLFOCUS(IDC_ATTENDANCE_YEAR, OnKillfocusAttendanceYear)
	ON_BN_CLICKED(IDC_MOVE_PREVIOUS_USER, OnMovePreviousUser)
	ON_BN_CLICKED(IDC_MOVE_NEXT_USER, OnMoveNextUser)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_ATTENDANCE_TODAY_BTN, &CAttendanceDlg::OnBnClickedAttendanceTodayBtn)
END_MESSAGE_MAP()


BEGIN_EVENTSINK_MAP(CAttendanceDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAttendanceDlg)
	ON_EVENT(CAttendanceDlg, IDC_ATTENDANCE_LIST, 19 /* LeftClick */, OnLeftClickAttendanceList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CAttendanceDlg, IDC_DEPARTMENT_FILTER, 18 /* RequeryFinished */, OnRequeryFinishedDepartmentFilter, VTS_I2)
	ON_EVENT(CAttendanceDlg, IDC_DEPARTMENT_FILTER, 16 /* SelChosen */, OnSelChosenDepartmentFilter, VTS_DISPATCH)
	ON_EVENT(CAttendanceDlg, IDC_ATTENDANCE_USER_LIST, 18 /* RequeryFinished */, OnRequeryFinishedAttendanceUserList, VTS_I2)
	ON_EVENT(CAttendanceDlg, IDC_ATTENDANCE_USER_LIST, 16 /* SelChosen */, OnSelChosenAttendanceUserList, VTS_DISPATCH)
	ON_EVENT(CAttendanceDlg, IDC_ATTENDANCE_LIST, 6 /* RButtonDown */, OnRButtonDownAttendanceList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CAttendanceDlg, IDC_ATTENDANCE_LIST, 8 /* EditingStarting */, OnEditingStartingAttendanceList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CAttendanceDlg, IDC_ATTENDANCE_LIST, 9 /* EditingFinishing */, OnEditingFinishingAttendanceList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CAttendanceDlg, IDC_ATTENDANCE_LIST, 10 /* EditingFinished */, OnEditingFinishedAttendanceList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CAttendanceDlg, IDC_ATTENDANCE_LIST, 3 /* DblClickCell */, OnDblClickCellAttendanceList, VTS_DISPATCH VTS_I2)
	// (b.cardillo 2008-06-30 14:03) - PLID 30529 - Changed it to use the new ClosedUp event ordinal and parameter list
	ON_EVENT(CAttendanceDlg, IDC_DEPARTMENT_FILTER, 30 /* ClosedUp */, OnClosedUpDepartmentFilter, VTS_DISPATCH)
	ON_EVENT(CAttendanceDlg, IDC_ATTENDANCE_USER_LIST, 30 /* ClosedUp */, OnClosedUpAttendanceUserList, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttendanceDlg message handlers

BOOL CAttendanceDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		g_propManager.CachePropertiesInBulk("CAttendanceOptionsDlg", propNumber,
			"(Username = '%s' OR Username = '<None>') AND Name IN ( \r\n"
			"	'AttendanceDepartmentID' \r\n"
			"	, 'AttendanceSelectedUserID' \r\n"
			"	, 'AttendanceListShowGrid' \r\n"
			// (z.manning 2012-03-27 11:42) - PLID 49227 - Added row color prefs
			"	, 'AttendanceAccruedRowColor' \r\n"
			"	, 'AttendanceAvailableRowColor' \r\n"
			"	, 'AttendanceTotalUsedRowColor' \r\n"
			"	, 'AttendanceBalanceRowColor' \r\n"
			")",
			_Q(GetCurrentUserName()));

		m_pdlAttendance = BindNxDataList2Ctrl(IDC_ATTENDANCE_LIST, false);
		m_pdlUsers = BindNxDataList2Ctrl(IDC_ATTENDANCE_USER_LIST, false);
		m_pdlDepartments = BindNxDataList2Ctrl(IDC_DEPARTMENT_FILTER, false);

		CheckDlgButton(IDC_RADIO_DAY, BST_CHECKED);
		m_nLastSelectedDepartmentID = GetRemotePropertyInt("AttendanceDepartmentID", -1, 0, GetCurrentUserName(), true);
		m_nLastSelectedUserID = GetRemotePropertyInt("AttendanceSelectedUserID", GetCurrentUserID(), 0, GetCurrentUserName(), true);

		// (z.manning, 11/28/2007) - Default to the current year.
		int nYear = COleDateTime::GetCurrentTime().GetYear();
		SetDlgItemInt(IDC_ATTENDANCE_YEAR, nYear);

		// (z.manning, 11/28/2007) - Default the date filter to the entire year.
		// (a.walling 2008-05-14 12:42) - PLID 27591 - COleVariant no longer necessary
		// (a.walling 2008-05-19 13:32) - PLID 27591 - Restore custom date format
		m_dtpDateFilterStart.SetFormat("M/d");
		m_dtpDateFilterEnd.SetFormat("M/d");
		m_dtpDateFilterStart.SetValue((COleDateTime(nYear, 1, 1, 0, 0, 0)));
		m_dtpDateFilterEnd.SetValue((COleDateTime(nYear, 12, 31, 0, 0, 0)));

		m_btnMoveNextMonth.AutoSet(NXB_RIGHT);
		m_btnMovePreviousMonth.AutoSet(NXB_LEFT);
		m_btnConfigureDepartments.AutoSet(NXB_MODIFY);
		m_btnUserSetup.AutoSet(NXB_MODIFY);
		m_btnLockPayroll.AutoSet(NXB_MODIFY);
		m_btnAttendanceOptions.AutoSet(NXB_MODIFY);
		m_btnRequestVacation.AutoSet(NXB_NEW);
		// (z.manning 2008-12-04 10:50) - PLID 32333 - Added left/right buttons for user combo
		m_btnNextUser.AutoSet(NXB_RIGHT);
		m_btnPreviousUser.AutoSet(NXB_LEFT);

		// (z.manning, 11/28/2007) - This will make it such that the currently logged in user is selected by default.
		m_arynSelectedUserIDs.Add(GetCurrentUserID());

	}NxCatchAll("CAttendanceDlg::OnInitDialog");
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAttendanceDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	CWaitCursor wc;
	try
	{
		// (z.manning, 11/13/2007) - This will load all attendance information from data.
		int nYear = GetDlgItemInt(IDC_ATTENDANCE_YEAR);
		m_AttendanceInfo.LoadAllByYear(nYear);

		if(nYear <= MIN_YEAR) {
			GetDlgItem(IDC_MOVE_PREVIOUS_YEAR)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_MOVE_PREVIOUS_YEAR)->EnableWindow(TRUE);
		}
		if(nYear >= MAX_YEAR) {
			GetDlgItem(IDC_MOVE_NEXT_YEAR)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_MOVE_NEXT_YEAR)->EnableWindow(TRUE);
		}

		m_bIsCurrentUserManager = m_AttendanceInfo.GetAttendanceUserByID(GetCurrentUserID())->IsManager();

		// (z.manning, 02/11/2008) - PLID 28295 - Non-managerial users should not be allowed to view other users.
		BOOL bIsCurrentUserAdmin = CheckCurrentUserPermissions(bioAttendance, sptDynamic0, FALSE, 0, TRUE);
		if(m_bIsCurrentUserManager || bIsCurrentUserAdmin) {
			m_pdlUsers->PutReadOnly(VARIANT_FALSE);
			m_pdlDepartments->PutReadOnly(VARIANT_FALSE);

			// (z.manning 2008-12-04 11:16) - PLID 32333
			m_btnPreviousUser.EnableWindow(TRUE);
			m_btnNextUser.EnableWindow(TRUE);
		}
		else {
			m_pdlUsers->PutReadOnly(VARIANT_TRUE);
			m_pdlDepartments->PutReadOnly(VARIANT_TRUE);
			// (z.manning 2008-12-04 11:16) - PLID 32333
			m_btnPreviousUser.EnableWindow(FALSE);
			m_btnNextUser.EnableWindow(FALSE);
		}

		// (z.manning, 11/13/2007) - Requery the department dropdown. When that finished it will requery the user
		// list and then when that finishes we will load the attendance list.
		m_pdlDepartments->Requery();

		// (z.manning, 12/11/2007) - Show/hide gridlines depending on the option.
		m_pdlAttendance->PutGridVisible(GetRemotePropertyInt("AttendanceListShowGrid", 0, 0, GetCurrentUserName(), true) == 0 ? VARIANT_FALSE : VARIANT_TRUE);

	}NxCatchAll("CAttendanceDlg::UpdateView");
}

void CAttendanceDlg::LoadAttendanceList(BOOL bEnsureCurrentDateVisible)
{
	m_pdlAttendance->SetRedraw(VARIANT_FALSE);

	CWaitCursor wc;
	try
	{
		// (z.manning, 11/28/2007) - First clear out all rows and remove the dynamic columns.
		m_pdlAttendance->Clear();
		RemoveDynamicColumns();
		// (z.manning, 11/28/2007) - Now re-add columns based on the number of users currently selected.
		AddDynamicColumns();

		// (z.manning, 11/28/2007) - Add rows to the datalist based on the currently selected date radio button option.
		if(IsDlgButtonChecked(IDC_RADIO_DAY) == BST_CHECKED) {
			AddRowsToDatalist_Daily();
		}
		else if(IsDlgButtonChecked(IDC_RADIO_MONTH) == BST_CHECKED) {
			AddRowsToDatalist_Monthly();
		}
		else if(IsDlgButtonChecked(IDC_RADIO_YEAR) == BST_CHECKED) {
			AddRowsToDatalist_Yearly();
		}
		else if(IsDlgButtonChecked(IDC_RADIO_CUSTOM) == BST_CHECKED) {
			AddRowsToDatalist_Custom();
		}
		else {
			ASSERT(FALSE);
		}

		HandleDateFilterChange();

		// (z.manning, 11/28/2007) - Get user data for all selected users.
		CArray<AttendanceUser*,AttendanceUser*> arypAttendanceUsers;
		m_AttendanceInfo.GetAttendanceData(m_arynSelectedUserIDs, arypAttendanceUsers);

		// (z.manning, 11/28/2007) - Now go through each selected user and add his/her info to the list.
		for(int nUserIndex = 0; nUserIndex < arypAttendanceUsers.GetSize() && nUserIndex < GetMaxVisibleUsers(); nUserIndex++)
		{
			AttendanceUser *pUser = arypAttendanceUsers.GetAt(nUserIndex);
			AddAttendanceDataToDatalist(pUser);
		}

		m_bInitialLoad = FALSE;

	}NxCatchAllCallThrow("CAttendanceDlg::LoadAttendanceList", m_pdlAttendance->SetRedraw(VARIANT_TRUE););

	m_pdlAttendance->SetRedraw(VARIANT_TRUE);

	if(bEnsureCurrentDateVisible) {
		COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
		// (z.manning 2008-11-18 11:48) - PLID 31782 - Only do this if the year we're on is the current year.
		if(dtCurrent.GetYear() == m_AttendanceInfo.m_nYear) {
			EnsureDateVisible(dtCurrent);
		}
	}
}

void CAttendanceDlg::OnMoveNextYear() 
{
	try
	{
		int nNextYear = GetDlgItemInt(IDC_ATTENDANCE_YEAR) + 1;
		SetDlgItemInt(IDC_ATTENDANCE_YEAR, nNextYear);
		EnsureValidDateFilter();
		UpdateView();

	}NxCatchAll("CAttendanceDlg::OnMoveNextMonth");
}

void CAttendanceDlg::OnMovePreviousYear() 
{
	try
	{
		int nPreviousYear = GetDlgItemInt(IDC_ATTENDANCE_YEAR) - 1;
		SetDlgItemInt(IDC_ATTENDANCE_YEAR, nPreviousYear);
		EnsureValidDateFilter();
		UpdateView();

	}NxCatchAll("CAttendanceDlg::OnMovePreviousMonth");
}

void CAttendanceDlg::OnRadioDay() 
{
	try
	{
		LoadAttendanceList(TRUE);

	}NxCatchAll("CAttendanceDlg::OnRadioDay");
}

void CAttendanceDlg::OnRadioMonth() 
{
	try
	{
		LoadAttendanceList(TRUE);

	}NxCatchAll("CAttendanceDlg::OnRadioMonth");
}

void CAttendanceDlg::OnRadioCustom() 
{
	try
	{
		LoadAttendanceList(TRUE);

	}NxCatchAll("CAttendanceDlg::OnRadioWeek");
}

void CAttendanceDlg::OnRadioYear() 
{
	try
	{
		LoadAttendanceList(TRUE);

	}NxCatchAll("CAttendanceDlg::OnRadioYear");
}

void CAttendanceDlg::OnLeftClickAttendanceList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		CWaitCursor wc;
		switch(nCol)
		{
			// (z.manning, 11/28/2007) - If someone clicks on the date row, open the attendance entry dialog
			// for that date.
			case alcDisplayDate:
			{
				if(pRow->GetValue(alcStartDate).vt == VT_DATE)
				{
					COleDateTime dtDate = GetDateFromAttendanceRow(lpRow);
					AttendanceUser *pUser = m_AttendanceInfo.GetAttendanceUserByID(GetCurrentUserID());
					AttendanceData *pData = pUser->GetAttendanceDataForDate(dtDate);
					if(pData == NULL) {
						pData = new AttendanceData;
						pData->pUser = pUser;
						pData->dtDate = dtDate;
					}
					OpenAttendanceEntryDlg(pData);
				}
			}
			break;
		}

	}NxCatchAll("CAttendanceDlg::OnLeftClickAttendanceList");
}

COleDateTime CAttendanceDlg::GetDateFromAttendanceRow(LPDISPATCH lpRow)
{
	COleDateTime dtDate;
	dtDate.SetStatus(COleDateTime::invalid);

	IRowSettingsPtr pRow(lpRow);
	if(pRow != NULL)
	{
		if(pRow->GetValue(alcStartDate).vt == VT_DATE) {
			dtDate = VarDateTime(pRow->GetValue(alcStartDate));
			ASSERT(dtDate.GetStatus() == COleDateTime::valid);
		}
	}

	return dtDate;
}

void CAttendanceDlg::OnRequeryFinishedDepartmentFilter(short nFlags) 
{
	try
	{
		IRowSettingsPtr pRow;

		// (z.manning, 12/11/2007) - Add rows for all hourly and salary.
		pRow = m_pdlDepartments->GetNewRow();
		pRow->PutValue(dcID, (long)ALL_HOURLY_DEPT_ID);
		pRow->PutValue(dcName, "{ All Hourly }");
		m_pdlDepartments->AddRowBefore(pRow, m_pdlDepartments->GetFirstRow());

		pRow = m_pdlDepartments->GetNewRow();
		pRow->PutValue(dcID, (long)ALL_SALARY_DEPT_ID);
		pRow->PutValue(dcName, "{ All Salary }");
		m_pdlDepartments->AddRowBefore(pRow, m_pdlDepartments->GetFirstRow());

		// (z.manning, 11/13/2007) - Add a row for all departments.
		pRow = m_pdlDepartments->GetNewRow();
		pRow->PutValue(dcID, (long)-1);
		pRow->PutValue(dcName, "{ All }");
		m_pdlDepartments->AddRowBefore(pRow, m_pdlDepartments->GetFirstRow());

		m_pdlDepartments->SetSelByColumn(dcID, m_nLastSelectedDepartmentID);

		// (z.manning, 11/28/2007) - Now requery the user list.
		m_pdlUsers->Requery();

	}NxCatchAll("CAttendanceDlg::OnRequeryFinishedDepartmentFilter");
}

void CAttendanceDlg::RefilterUserList()
{
	long nDepartmentID = -1;
	if(m_pdlDepartments->GetCurSel() != NULL) {
		nDepartmentID = VarLong(m_pdlDepartments->GetCurSel()->GetValue(dcID));
	}

	IRowSettingsPtr pRow = m_pdlUsers->FindAbsoluteFirstRow(VARIANT_FALSE);
	while(pRow != NULL)
	{
		// (z.manning, 11/28/2007) - Filter out any users that aren't in the currently selected department.
		long nUserID = VarLong(pRow->GetValue(ulcID));
		AttendanceUser *pUser = NULL;
		if(nUserID > 0) {
			pUser = m_AttendanceInfo.GetAttendanceUserByID(nUserID);
		}

		if(nDepartmentID == ALL_HOURLY_DEPT_ID) {
			if(nUserID < 0 || (pUser != NULL && pUser->m_eType == autHourly)) {
				pRow->PutVisible(VARIANT_TRUE);
			}
			else {
				pRow->PutVisible(VARIANT_FALSE);
			}
		}
		else if(nDepartmentID == ALL_SALARY_DEPT_ID) {
			if(nUserID < 0 || (pUser != NULL && pUser->m_eType == autSalary)) {
				pRow->PutVisible(VARIANT_TRUE);
			}
			else {
				pRow->PutVisible(VARIANT_FALSE);
			}
		}
		else if(nDepartmentID == -1 || nUserID < 0 || (pUser != NULL && pUser->IsInDepartment(nDepartmentID))) {
			pRow->PutVisible(VARIANT_TRUE);
		}
		else {
			pRow->PutVisible(VARIANT_FALSE);
		}

		pRow = m_pdlUsers->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
	}
}

void CAttendanceDlg::OnSelChosenDepartmentFilter(LPDISPATCH lpRow) 
{
	CWaitCursor wc;
	try
	{
		IRowSettingsPtr pUserCurSel = m_pdlUsers->GetCurSel();

		if(lpRow == NULL) {
			// (z.manning, 11/13/2007) - If they chose nothing, choose the all departments row.
			if(m_pdlDepartments->SetSelByColumn(dcID, (long)-1) == NULL) {
				ASSERT(FALSE);
			}
		}
		else {
			IRowSettingsPtr pNewSel(lpRow);
			m_nLastSelectedDepartmentID = VarLong(pNewSel->GetValue(dcID));
		}

		RefilterUserList();

		// (z.manning, 12/11/2007) - If we filtered out the selected user then let's just select all users.
		if(pUserCurSel != NULL && !pUserCurSel->GetVisible()) {
			if(m_pdlUsers->SetSelByColumn(dcID, (long)-1) == NULL) {
				ASSERT(FALSE);
			}
		}

		UpdateSelectedUserIDs();
		LoadAttendanceList(FALSE);

	}NxCatchAll("CAttendanceDlg::OnSelChosenDepartmentFilter");
}

void CAttendanceDlg::OnRequeryFinishedAttendanceUserList(short nFlags) 
{
	try
	{
		RefilterUserList();

		IRowSettingsPtr pRow;
		
		pRow = m_pdlUsers->GetNewRow();
		pRow->PutValue(dcID, (long)suriOther);
		pRow->PutValue(dcName, "{ Other }");
		m_pdlUsers->AddRowBefore(pRow, m_pdlUsers->GetFirstRow());
		
		// (z.manning 2008-11-13 15:08) - PLID 31831 - Added Paid/Unpaid
		pRow = m_pdlUsers->GetNewRow();
		pRow->PutValue(dcID, (long)suriUnpaid);
		pRow->PutValue(dcName, "{ Unpaid }");
		m_pdlUsers->AddRowBefore(pRow, m_pdlUsers->GetFirstRow());

		pRow = m_pdlUsers->GetNewRow();
		pRow->PutValue(dcID, (long)suriPaid);
		pRow->PutValue(dcName, "{ Paid }");
		m_pdlUsers->AddRowBefore(pRow, m_pdlUsers->GetFirstRow());
		
		pRow = m_pdlUsers->GetNewRow();
		pRow->PutValue(dcID, (long)suriSick);
		pRow->PutValue(dcName, "{ Sick }");
		m_pdlUsers->AddRowBefore(pRow, m_pdlUsers->GetFirstRow());
		
		pRow = m_pdlUsers->GetNewRow();
		pRow->PutValue(dcID, (long)suriVacation);
		pRow->PutValue(dcName, "{ PTO }");								// (d.thompson 2014-02-27) - PLID 61016 - Vacation now called PTO
		m_pdlUsers->AddRowBefore(pRow, m_pdlUsers->GetFirstRow());
		
		pRow = m_pdlUsers->GetNewRow();
		pRow->PutValue(dcID, (long)suriUnapprovedTimeOff);
		pRow->PutValue(dcName, "{ Unapproved time off }");
		m_pdlUsers->AddRowBefore(pRow, m_pdlUsers->GetFirstRow());
		
		pRow = m_pdlUsers->GetNewRow();
		pRow->PutValue(dcID, (long)suriAnyTimeOff);
		pRow->PutValue(dcName, "{ Any time off }");
		m_pdlUsers->AddRowBefore(pRow, m_pdlUsers->GetFirstRow());
		
		pRow = m_pdlUsers->GetNewRow();
		pRow->PutValue(dcID, (long)suriAll);
		pRow->PutValue(dcName, "{ All }");
		m_pdlUsers->AddRowBefore(pRow, m_pdlUsers->GetFirstRow());

		BOOL bIsCurrentUserAdmin = CheckCurrentUserPermissions(bioAttendance, sptDynamic0, FALSE, 0, TRUE);

		long nSelUserID;
		if(m_bIsCurrentUserManager || bIsCurrentUserAdmin) {
			nSelUserID = m_nLastSelectedUserID;
		}
		else {
			// (z.manning, 02/28/2008) - Normal users should never have been able to change the user dropdown's selection.
			nSelUserID = GetCurrentUserID();
		}

		IRowSettingsPtr pSel = m_pdlUsers->SetSelByColumn(ulcID, nSelUserID);
		if(pSel == NULL) {
			// (z.manning, 02/28/2008) - The selection failed. If the user is a manager or admin, then just select the "all" row.
			if(m_bIsCurrentUserManager || bIsCurrentUserAdmin) {
				pSel = m_pdlUsers->SetSelByColumn(ulcID, (long)-1);
			}

			// (z.manning, 02/28/2008) - We should have had a valid selection by now.
			if(pSel == NULL) {
				ThrowNxException("CAttendanceDlg::OnRequeryFinishedAttendanceUserList - Failed to select user column with ID %li", nSelUserID);
			}
		}

		UpdateSelectedUserIDs();
		LoadAttendanceList(m_bInitialLoad);

	}NxCatchAll("CAttendanceDlg::OnRequeryFinishedAttendanceUserList");
}

void CAttendanceDlg::UpdateSelectedUserIDs()
{
	m_arynSelectedUserIDs.RemoveAll();
	long nSelectedUserID = VarLong(m_pdlUsers->GetCurSel()->GetValue(ulcID));
	if(nSelectedUserID > 0) {
		m_arynSelectedUserIDs.Add(nSelectedUserID);
	}
	else
	{
		// (a.walling 2008-05-14 12:42) - PLID 27591 - VarDateTime no longer necessary
		COleDateTime dtStart = (m_dtpDateFilterStart.GetValue());
		COleDateTime dtEnd = (m_dtpDateFilterEnd.GetValue());
		for(IRowSettingsPtr pRow = m_pdlUsers->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
		{
			long nTempUserID = VarLong(pRow->GetValue(ulcID));
			if(nTempUserID > 0 && pRow->GetVisible())
			{
				AttendanceUser *pUser = m_AttendanceInfo.GetAttendanceUserByID(nTempUserID);
				BOOL bAddUser = FALSE;
				BOOL bUnApprovedOnly = FALSE;
				switch(nSelectedUserID)
				{
					case suriAll:
						bAddUser = TRUE;
						break;

					// (z.manning, 05/19/2008) - PLID 30102 - Added new filter for unapproved time off
					// (z.manning 2008-11-13 14:59) - PLID 31831 - Added paid/unpaid
					case suriUnapprovedTimeOff:
						bUnApprovedOnly = TRUE;
					case suriAnyTimeOff:
						bAddUser = pUser->GetTotalUsedVacationByDate(dtStart, dtEnd, FALSE, bUnApprovedOnly) > COleCurrency(0,0)
							|| pUser->GetTotalUsedSickByDate(dtStart, dtEnd, bUnApprovedOnly) > COleCurrency(0,0)
							|| pUser->GetTotalUsedPaidByDate(dtStart, dtEnd, bUnApprovedOnly) > COleCurrency(0,0)
							|| pUser->GetTotalUsedUnpaidByDate(dtStart, dtEnd, bUnApprovedOnly) > COleCurrency(0,0)
							|| pUser->GetTotalUsedOtherByDate(dtStart, dtEnd, bUnApprovedOnly) > COleCurrency(0,0);
						break;

					case suriVacation:
						bAddUser = pUser->GetTotalUsedVacationByDate(dtStart, dtEnd, FALSE) > COleCurrency(0,0);
						break;

					case suriSick:
						bAddUser = pUser->GetTotalUsedSickByDate(dtStart, dtEnd) > COleCurrency(0,0);
						break;

					case suriOther:
						bAddUser = pUser->GetTotalUsedOtherByDate(dtStart, dtEnd) > COleCurrency(0,0);
						break;

					// (z.manning 2008-11-13 15:18) - Added paid/unpaid
					case suriPaid:
						bAddUser = pUser->GetTotalUsedPaidByDate(dtStart, dtEnd) > COleCurrency(0,0);
						break;

					case suriUnpaid:
						bAddUser = pUser->GetTotalUsedUnpaidByDate(dtStart, dtEnd) > COleCurrency(0,0);
						break;

					default:
						ASSERT(FALSE);
						break;
				}

				if(bAddUser) {
					m_arynSelectedUserIDs.Add(nTempUserID);
				}
			}
		}
	}
}

void CAttendanceDlg::OnConfigureDepartments() 
{
	try
	{
		// (z.manning, 12/06/2007) - PLID 28295 - Check permission
		if(!m_bIsCurrentUserManager) {
			if(!CheckCurrentUserPermissions(bioAttendance, sptDynamic0)) {
				return;
			}
		}

		CConfigureDepartmentsDlg dlg(&m_AttendanceInfo, this);
		dlg.DoModal();
		RefilterUserList();
		UpdateView();

	}NxCatchAll("CAttendanceDlg::OnConfigureDepartments");
}

void CAttendanceDlg::RemoveDynamicColumns()
{
	// (z.manning, 11/28/2007) - Remove all the user attendance data columns.
	short nCol = alcBeginDynamicColumns;
	while(nCol < m_pdlAttendance->GetColumnCount()) {
		m_pdlAttendance->RemoveColumn(nCol);
	}
}

void CAttendanceDlg::AddDynamicColumns()
{
	IColumnSettingsPtr pCol;
	short nColumnIndex = m_pdlAttendance->GetColumnCount();

	BOOL bIsCurrentUserAdmin = CheckCurrentUserPermissions(bioAttendance, sptDynamic0, FALSE, 0, TRUE);

	// (z.manning, 11/20/2007) - We hide the notes colum in month & year views.
	int nNotesVisible = 0, nAdminNotesVisible = 0;
	if(!(IsDlgButtonChecked(IDC_RADIO_MONTH) == BST_CHECKED || IsDlgButtonChecked(IDC_RADIO_YEAR) == BST_CHECKED)) {
		nNotesVisible = csVisible;

		if(bIsCurrentUserAdmin || m_bIsCurrentUserManager) {
			nAdminNotesVisible = csVisible;
		}
	}

	// (z.manning, 11/28/2007) - For each selected user, add columns to show their attendance information.
	for(int nUserIndex = 0; nUserIndex < m_arynSelectedUserIDs.GetSize() && nUserIndex < GetMaxVisibleUsers(); nUserIndex++)
	{
		AttendanceUser *pUser = m_AttendanceInfo.GetAttendanceUserByID(m_arynSelectedUserIDs.GetAt(nUserIndex));

		// (d.thompson 2014-02-24) - PLID 61015 - Hours worked is now displayed for all types of users
		// (d.thompson 2014-02-27) - PLID 61016 - Previously we showed vacation/sick for salary and just sick for hourly.  We're combining
		//	these into a single PTO field, so now everyone will see PTO.

		// (z.manning 2009-02-10 11:31) - PLID 31829 - We used to set many of these columns to the
		// csWidthData style, however, that was causing massive slowness when many columns were visible.
		// We now use csFixedWidth here and CAttendanceDlg::AddAttendanceDataToDatalist which size
		// the necessary columns based on data (which it had already been doing anyway).

		pCol = m_pdlAttendance->GetColumn(m_pdlAttendance->InsertColumn(nColumnIndex++, _T(""), _T(""), 5, csVisible|csFixedWidth));

		pCol = m_pdlAttendance->GetColumn(m_pdlAttendance->InsertColumn(nColumnIndex++, _T(""), _T("Hours"), 0, csVisible|csFixedWidth|csEditable));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		// (z.manning 2009-02-10 11:00) - Width is set in CAttendanceDlg::AddAttendanceDataToDatalist

		// (d.thompson 2014-02-27) - PLID 61016 - Vacation now called PTO
		pCol = m_pdlAttendance->GetColumn(m_pdlAttendance->InsertColumn(nColumnIndex++, _T(""), _T("PTO"), 0, csVisible|csFixedWidth|csEditable));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		// (z.manning 2009-02-10 11:00) - Width is set in CAttendanceDlg::AddAttendanceDataToDatalist

		// (d.thompson 2014-02-27) - PLID 61016 - Vacation and sick are combined into PTO now.  For historical reasons, I want to leave the "sick" field available
		//	and sizeable, but always 0 width.  I am going to make it no longer editable.
		pCol = m_pdlAttendance->GetColumn(m_pdlAttendance->InsertColumn(nColumnIndex++, _T(""), _T("Sick"), 0, csVisible));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		// (z.manning 2009-02-10 11:00) - Width is set in CAttendanceDlg::AddAttendanceDataToDatalist

		// (z.manning 2008-11-13 13:03) - PLID 31831 - Added Paid/Unpaid columns
		pCol = m_pdlAttendance->GetColumn(m_pdlAttendance->InsertColumn(nColumnIndex++, _T(""), _T("Paid"), 0, csVisible|csFixedWidth|csEditable));
		pCol->StoredWidth = 50;
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol = m_pdlAttendance->GetColumn(m_pdlAttendance->InsertColumn(nColumnIndex++, _T(""), _T("Unpaid"), 0, csVisible|csFixedWidth|csEditable));
		pCol->StoredWidth = 50;
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;

		pCol = m_pdlAttendance->GetColumn(m_pdlAttendance->InsertColumn(nColumnIndex++, _T(""), _T("Other"), 0, csVisible|csFixedWidth|csEditable));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol->StoredWidth = 50;

		pCol = m_pdlAttendance->GetColumn(m_pdlAttendance->InsertColumn(nColumnIndex++, _T(""), _T("Notes"), 0, nNotesVisible|csEditable));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol->MaxTextLen = 255;

		pCol = m_pdlAttendance->GetColumn(m_pdlAttendance->InsertColumn(nColumnIndex++, _T(""), _T("Admin Notes"), 0, nAdminNotesVisible|csEditable));
		pCol->FieldType = cftTextSingleLine;
		pCol->DataType = VT_BSTR;
		pCol->MaxTextLen = 255;
	}
}

void CAttendanceDlg::OnSelChosenAttendanceUserList(LPDISPATCH lpRow) 
{
	CWaitCursor wc;
	try
	{
		if(lpRow == NULL) {
			ASSERT(FALSE);
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		m_nLastSelectedUserID = VarLong(pRow->GetValue(ulcID));

		UpdateSelectedUserIDs();
		LoadAttendanceList(FALSE);

	}NxCatchAll("CAttendanceDlg::OnSelChosenAttendanceUserList");
}

void CAttendanceDlg::OnUserSetup() 
{
	CWaitCursor wc;
	try
	{
		// (z.manning, 12/06/2007) - PLID 28295 - Check permission
		if(!m_bIsCurrentUserManager) {
			if(!CheckCurrentUserPermissions(bioAttendance, sptDynamic0)) {
				return;
			}
		}

		CAttendanceUserSetupDlg dlg(&m_AttendanceInfo, this);
		if(m_pdlUsers->GetCurSel() != NULL) {
			dlg.SetDefaultUserID(VarLong(m_pdlUsers->GetCurSel()->GetValue(ulcID)));
		}
		dlg.DoModal();

		// (z.manning, 12/04/2007) - We need to refresh even if they didn't hit ok because changing users.
		if(dlg.m_bAtLeastOneSave) {
			UpdateView();
		}

	}NxCatchAll("CAttendanceDlg::OnUserSetup");
}

void CAttendanceDlg::OnRequestVacation() 
{
	try
	{
		AttendanceData *pData = new AttendanceData;
		pData->pUser = m_AttendanceInfo.GetAttendanceUserByID(GetCurrentUserID());
		OpenAttendanceEntryDlg(pData);

	}NxCatchAll("CAttendanceDlg::OnRequestVacation");
}

void CAttendanceDlg::EnsureDateVisible(COleDateTime dtDate)
{
	// (z.manning, 9/2/2008) - PLID 31162 - Scroll to the bottom first so when we ensure
	// the current date is visible it will be at the top instead of the bottom.
	m_pdlAttendance->EnsureRowInView(m_pdlAttendance->GetLastRow());

	// (z.manning, 11/28/2007) - Make sure we aren't dealing with any time stuff here.
	dtDate.SetDate(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay());
	for(IRowSettingsPtr pRow = m_pdlAttendance->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow())
	{
		if(pRow->GetValue(alcStartDate).vt == VT_DATE && pRow->GetValue(alcEndDate).vt == VT_DATE)
		{
			if(dtDate >= VarDateTime(pRow->GetValue(alcStartDate)) && dtDate <= VarDateTime(pRow->GetValue(alcEndDate))) {
				m_pdlAttendance->EnsureRowInView(pRow);
				return;
			}
		}
	}
}

void CAttendanceDlg::OnEditCustomDateRange() 
{
	try
	{
		CAttendanceCustomDateRangeSetupDlg dlg(this);
		if(dlg.DoModal() == IDOK)
		{
			// (z.manning, 11/27/2007) - We only need to reload the attendance list if the custom radio option
			// is checked.
			if(IsDlgButtonChecked(IDC_RADIO_CUSTOM) == BST_CHECKED) {
				LoadAttendanceList(FALSE);
			}
		}

	}NxCatchAll("CAttendanceDlg::OnEditCustomDateRange");
}

// (a.walling 2008-05-14 12:22) - PLID 27591 - Use the new notify events
void CAttendanceDlg::OnChangeDateFilterStart(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try
	{
		EnsureValidDateFilter();
		// (z.manning, 02/12/2008) - We need to update selected user IDs because we may be filtering
		// on users with a certain kind of time off in the given time range.
		UpdateSelectedUserIDs();
		LoadAttendanceList(FALSE);

	}NxCatchAll("CAttendanceDlg::OnChangeDateFilterStart");

	*pResult = 0;
}

// (a.walling 2008-05-14 12:22) - PLID 27591 - Use the new notify events
void CAttendanceDlg::OnChangeDateFilterEnd(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try
	{
		EnsureValidDateFilter();
		// (z.manning, 02/12/2008) - We need to update selected user IDs because we may be filtering
		// on users with a certain kind of time off in the given time range.
		UpdateSelectedUserIDs();
		LoadAttendanceList(FALSE);

	}NxCatchAll("CAttendanceDlg::OnChangeDateFilterEnd");	
	
	*pResult = 0;
}

void CAttendanceDlg::EnsureValidDateFilter()
{
	// (a.walling 2008-05-14 12:42) - PLID 27591 - VarDateTime no longer necessary
	COleDateTime dtStart = (m_dtpDateFilterStart.GetValue());
	COleDateTime dtEnd = (m_dtpDateFilterEnd.GetValue());

	// (z.manning, 11/28/2007) - The year is a separate option, so no matter what they choose for the filters,
	// make sure it stays in the same year.
	int nActiveYear = GetDlgItemInt(IDC_ATTENDANCE_YEAR);
	if(dtStart.GetYear() != nActiveYear) {
		dtStart.SetDate(nActiveYear, dtStart.GetMonth(), dtStart.GetDay());
	}
	if(dtEnd.GetYear() != nActiveYear) {
		dtEnd.SetDate(nActiveYear, dtEnd.GetMonth(), dtEnd.GetDay());
	}

	// (z.manning, 11/28/2007) - Make sure the start date is not later than the end date.
	if(dtStart > dtEnd) {
		dtEnd = dtStart;
	}

	// (a.walling 2008-05-14 12:42) - PLID 27591 - COleVariant no longer necessary
	m_dtpDateFilterStart.SetValue((dtStart));
	m_dtpDateFilterEnd.SetValue((dtEnd));
}

void CAttendanceDlg::OnResetDateFilter() 
{
	try
	{
		// (z.manning, 11/28/2007) - Reset the date filter to the entire current active year.
		int nYear = GetDlgItemInt(IDC_ATTENDANCE_YEAR);
		// (a.walling 2008-05-14 12:42) - PLID 27591 - COleVariant no longer necessary
		m_dtpDateFilterStart.SetValue((COleDateTime(nYear, 1, 1, 0, 0, 0)));
		m_dtpDateFilterEnd.SetValue((COleDateTime(nYear, 12, 31, 0, 0, 0)));

		// (z.manning, 05/19/2008) - We need to update selected user IDs because we may be filtering
		// on users with a certain kind of time off in the given time range.
		UpdateSelectedUserIDs();
		LoadAttendanceList(FALSE);

	}NxCatchAll("CAttendanceDlg::OnResetDateFilter");
}

void CAttendanceDlg::HandleDateFilterChange()
{
	// (z.manning, 11/30/2007) - We don't need to do anything if we're in year view.
	if(IsDlgButtonChecked(IDC_RADIO_YEAR) == BST_CHECKED) {
		return;
	}

	// (z.manning, 11/27/2007) - Now filter out any rows that don't fit within the specified date filter.
	IRowSettingsPtr pRow = m_pdlAttendance->GetFirstRow();
	while(pRow != NULL)
	{
		if(pRow->GetValue(alcStartDate).vt == VT_DATE && pRow->GetValue(alcEndDate).vt == VT_DATE)
		{
			// (a.walling 2008-05-14 12:42) - PLID 27591 - VarDateTime no longer necessary
			if( VarDateTime(pRow->GetValue(alcStartDate)) < (m_dtpDateFilterStart.GetValue()) ||
				VarDateTime(pRow->GetValue(alcEndDate)) > (m_dtpDateFilterEnd.GetValue()) )
			{
				pRow->PutVisible(VARIANT_FALSE);
			}
			else {
				pRow->PutVisible(VARIANT_TRUE);
			}
		}
		pRow = pRow->GetNextRow();
	}
}

// (z.manning, 11/28/2007) - Adds a row to the datalist for each day in the given year.
void CAttendanceDlg::AddRowsToDatalist_Daily()
{
	COleDateTime dtDate;
	dtDate.SetDate(m_AttendanceInfo.m_nYear, 1, 1);
	while(dtDate.GetYear() == m_AttendanceInfo.m_nYear)
	{
		IRowSettingsPtr pRow = m_pdlAttendance->GetNewRow();
		SetAttendanceRowDefaults(m_pdlAttendance->GetColumnCount(), pRow);
		pRow->PutValue(alcDisplayDate, _bstr_t(FormatDateTimeForInterface(dtDate,DTF_STRIP_YEARS,dtoDate)));
		pRow->PutValue(alcStartDate, COleVariant(dtDate));
		pRow->PutValue(alcEndDate, COleVariant(dtDate));

		CString strHoliday;
		// (z.manning, 11/20/2007) - Make weekend rows gray.
		if(dtDate.GetDayOfWeek() == 1 || dtDate.GetDayOfWeek() == 7) {
			pRow->PutBackColor(RGB(210,210,210));
		}
		// (z.manning, 12/11/2007) - Same with holidays
		else if(IsHoliday(dtDate, strHoliday)) {
			pRow->PutBackColor(RGB(210,210,210));
			CString strDisplayDate = VarString(pRow->GetValue(alcDisplayDate));
			// (z.manning 2009-08-03 10:43) - PLID 32907 - Check for empty string
			if(!strHoliday.IsEmpty()) {
				strDisplayDate += " (" + strHoliday + ')';
			}
			pRow->PutValue(alcDisplayDate, _bstr_t(strDisplayDate));
		}

		m_pdlAttendance->AddRowAtEnd(pRow, NULL);

		// (z.manning, 02/11/2008) - PLID 28885 - Is this the end date of a pay period?
		if(m_AttendanceInfo.IsPayrollCloseDate(dtDate))
		{
			// (z.manning, 02/11/2008) - If so, then let's add a placeholder row indicating as much.
			pRow = m_pdlAttendance->GetNewRow();
			SetAttendanceRowDefaults(m_pdlAttendance->GetColumnCount(), pRow);
			pRow->PutValue(alcDisplayDate, "End Pay Period");
			pRow->PutValue(alcStartDate, COleVariant(dtDate));
			pRow->PutValue(alcEndDate, COleVariant(dtDate));
			pRow->PutBackColor(RGB(255,220,180));
			m_pdlAttendance->AddRowAtEnd(pRow, NULL);
		}

		dtDate += ONE_DAY;
	}

	AddHeaderFooterRows();
}

// (z.manning, 11/28/2007) - Adds a row to the datalist for each month in the given year.
void CAttendanceDlg::AddRowsToDatalist_Monthly()
{
	for(int nMonth = 1; nMonth <= 12; nMonth++)
	{
		COleDateTime dtFirstOfMonth;
		dtFirstOfMonth.SetDate(m_AttendanceInfo.m_nYear, nMonth, 1);

		IRowSettingsPtr pRow = m_pdlAttendance->GetNewRow();
		SetAttendanceRowDefaults(m_pdlAttendance->GetColumnCount(), pRow);
		pRow->PutValue(alcDisplayDate, _bstr_t(dtFirstOfMonth.Format("%B")));
		pRow->PutValue(alcStartDate, COleVariant(dtFirstOfMonth));
		COleDateTime dtLastOfMonth;
		if(nMonth <= 11) {
			dtLastOfMonth.SetDate(m_AttendanceInfo.m_nYear, nMonth + 1, 1);
			dtLastOfMonth -= ONE_DAY;
		}
		else {
			ASSERT(nMonth == 12);
			dtLastOfMonth.SetDate(m_AttendanceInfo.m_nYear, nMonth, 31);
		}
		pRow->PutValue(alcEndDate, COleVariant(dtLastOfMonth));
		m_pdlAttendance->AddRowAtEnd(pRow, NULL);
	}

	AddHeaderFooterRows();
}

void CAttendanceDlg::AddRowsToDatalist_Yearly()
{
	// (z.manning, 11/28/2007) - Add just one invisible row with the full date range.
	IRowSettingsPtr pRow = m_pdlAttendance->GetNewRow();
	SetAttendanceRowDefaults(m_pdlAttendance->GetColumnCount(), pRow);
	pRow->PutValue(alcDisplayDate, _bstr_t(AsString(m_AttendanceInfo.m_nYear)));
	pRow->PutValue(alcStartDate, COleVariant(COleDateTime(m_AttendanceInfo.m_nYear, 1, 1, 0, 0, 0)));
	pRow->PutValue(alcEndDate, COleVariant(COleDateTime(m_AttendanceInfo.m_nYear, 12, 31, 0, 0, 0)));
	pRow->PutVisible(VARIANT_FALSE);
	m_pdlAttendance->AddRowAtEnd(pRow, NULL);

	AddHeaderFooterRows();
}

void CAttendanceDlg::AddRowsToDatalist_Custom()
{
	ECustomDateRangeTypes nType;
	int nFrequency;
	LoadAttendanceCustomDateRangeProperty(nType, nFrequency);

	COleDateTime dtRangeStart, dtRangeEnd;
	dtRangeStart.SetDate(m_AttendanceInfo.m_nYear, 1, 1);
	int nMonthIterationCount = 0;
	while(dtRangeStart.GetYear() == m_AttendanceInfo.m_nYear)
	{
		IRowSettingsPtr pRow = m_pdlAttendance->GetNewRow();
		SetAttendanceRowDefaults(m_pdlAttendance->GetColumnCount(), pRow);
		if(nType == cdrtWeekly)
		{
			dtRangeEnd = dtRangeStart + COleDateTimeSpan(nFrequency * 7 - dtRangeStart.GetDayOfWeek());
		}
		else if(nType == cdrtMonthly)
		{
			// (z.manning, 11/27/2007) - Keep count of the number of times we do this for each month.
			nMonthIterationCount++;

			// (z.manning, 11/27/2007) - Determine how many days should be in a date range based on a 30 day month
			// (always use 30 to be consistent for all months).
			int nDaysToAdd = 30 / nFrequency - 1;
			dtRangeEnd = dtRangeStart + COleDateTimeSpan(nDaysToAdd);

			// (z.manning, 11/27/2007) - Make sure we didn't jump into the next month already.
			while(dtRangeEnd.GetMonth() != dtRangeStart.GetMonth()) {
				dtRangeEnd -= ONE_DAY;
			}

			// (z.manning, 11/27/2007) - Now check and see if we've reach the maximum iteration count for this month
			// and if we have, make sure we extend this last date range for that month to the last day of the month.
			if(nMonthIterationCount >= nFrequency)
			{
				while(dtRangeEnd.GetMonth() == dtRangeStart.GetMonth()) {
					dtRangeEnd += ONE_DAY;
				}
				ASSERT(dtRangeEnd.GetDay() == 1);
				dtRangeEnd -= ONE_DAY;
			}

			// (z.manning, 11/29/2007) - Shade the rows slightly differently for every other month.
			if(dtRangeStart.GetMonth() % 2 == 0) {
				pRow->PutBackColor(RGB(240,240,255));
			}
		}
		else {
			ASSERT(FALSE);
		}

		if(dtRangeEnd.GetYear() > m_AttendanceInfo.m_nYear) {
			dtRangeEnd.SetDate(m_AttendanceInfo.m_nYear, 12, 31);
		}

		CString strDisplayDate;
		if(nType == cdrtMonthly) {
			// (z.manning, 11/30/2007) - PLID 28218 - The following line of code is internationally compliant, but Kamal
			// wants the format to be Jan 1 - 15, for example, and InternationUtils don't seem to suppor that yet, so I'm
			// just going to do it the American way and we can decide if we need to fix it later if we ever give this to clients.
			ASSERT(dtRangeStart.GetMonth() == dtRangeEnd.GetMonth());
			strDisplayDate = FormatString("%s - %li", dtRangeStart.Format("%b %#d"), dtRangeEnd.GetDay());
		}
		else {
			strDisplayDate = FormatDateTimeForInterface(dtRangeStart,DTF_STRIP_YEARS,dtoDate) + " - " + FormatDateTimeForInterface(dtRangeEnd,DTF_STRIP_YEARS,dtoDate);
		}
		pRow->PutValue(alcDisplayDate, _bstr_t(strDisplayDate));
		pRow->PutValue(alcStartDate, COleVariant(dtRangeStart));
		pRow->PutValue(alcEndDate, COleVariant(dtRangeEnd));
		m_pdlAttendance->AddRowAtEnd(pRow, NULL);

		dtRangeStart = dtRangeEnd + ONE_DAY;

		// (z.manning, 11/27/2007) - If we've moved into a new month, reset the month iteration counter.
		if(dtRangeStart.GetMonth() != dtRangeEnd.GetMonth()) {
			nMonthIterationCount = 0;
		}
	}

	AddHeaderFooterRows();
}

void CAttendanceDlg::AddHeaderFooterRows()
{
	IRowSettingsPtr pFooterTemplateRow = m_pdlAttendance->GetNewRow();
	_variant_t varNull;
	varNull.vt = VT_NULL;
	
	SetAttendanceRowDefaults(m_pdlAttendance->GetColumnCount(), pFooterTemplateRow);
	// (z.manning, 11/16/2007) - Add the template row to the datalist temporarily so we can make copies
	// of it without having to share its pointer.
	m_pdlAttendance->AddRowAtEnd(pFooterTemplateRow, NULL);

	// (z.manning, 11/15/2007) - If the order of the footer columns changes at all then you will also
	// need to make changes to AttendanceUser::AddAttendanceDataToDatalist.
	IRowSettingsPtr pFooterRow1 = m_pdlAttendance->AddRowAtEnd(pFooterTemplateRow, NULL);
	pFooterRow1->PutValue(alcDisplayDate, varNull);
	pFooterRow1->PutBackColor(RGB(0,0,0));
	// (z.manning, 11/15/2007) - This isn't supported yet by the datalist, but once it is I think we 
	// should make the border row before the totals a bit smaller.
	//pFooterRow1->PutHeight(pFooterRow->GetHeight() / 2);
	// (z.manning 2008-12-04 14:21) - PLID 32342 - Reordered these columns
	// (z.manning 2012-03-27 12:03) - PLID 49227 - Use the new row color options
	IRowSettingsPtr pFooterRow2 = m_pdlAttendance->AddRowAtEnd(pFooterTemplateRow, NULL);
	pFooterRow2->PutValue(alcDisplayDate, "YTD Accrued:");
	pFooterRow2->PutBackColor(GetAccruedRowColor());
	IRowSettingsPtr pFooterRow3 = m_pdlAttendance->AddRowAtEnd(pFooterTemplateRow, NULL);
	pFooterRow3->PutValue(alcDisplayDate, "Total Available:");
	pFooterRow3->PutBackColor(GetAvailableRowColor());
	IRowSettingsPtr pFooterRow4 = m_pdlAttendance->AddRowAtEnd(pFooterTemplateRow, NULL);
	pFooterRow4->PutValue(alcDisplayDate, "Total Used:");
	pFooterRow4->PutBackColor(GetTotalUsedRowColor());
	IRowSettingsPtr pFooterRow5 = m_pdlAttendance->AddRowAtEnd(pFooterTemplateRow, NULL);
	pFooterRow5->PutValue(alcDisplayDate, "Balance:");
	pFooterRow5->PutBackColor(GetBalanceRowColor());

	IRowSettingsPtr pHeaderRow = m_pdlAttendance->AddRowBefore(pFooterTemplateRow, m_pdlAttendance->GetFirstRow());
	pHeaderRow->PutBackColor(RGB(255,220,180));

	// (z.manning, 11/16/2007) - Now remove the template footer row that we were just using to copy from.
	m_pdlAttendance->RemoveRow(pFooterTemplateRow);
}

void CAttendanceDlg::SetAttendanceRowDefaults(short nColumnCount, IRowSettingsPtr pRow)
{
	_variant_t varNull;
	varNull.vt = VT_NULL;
	for(short nCol = 0; nCol < nColumnCount; nCol++)
	{
		pRow->PutValue(nCol, varNull);

		if((nCol + aldcPointer - alcBeginDynamicColumns) % aldcDynamicColumnCount == 0) {
			// (z.manning, 11/15/2007) - This is a border row so color it in.
			pRow->PutCellBackColor(nCol, RGB(0,0,0));
		}
	}
}

#define UPDATE_CURRENCY_CELL(field) \
	CString str##field = VarString(pRow->GetValue(nStartColumn + aldc##field), "0"); \
	COleCurrency cy##field; \
	if(!cy##field.ParseCurrency(str##field)) { \
		ASSERT(FALSE); \
		cy##field.SetCurrency(0, 0); \
	} \
	cy##field += pData->cy##field; \
	if(cy##field != COleCurrency(0,0)) { \
		pRow->PutValue(nStartColumn + aldc##field, _bstr_t(FormatAttendanceValue(cy##field))); \
		/* (z.manning, 11/20/2007) - If this is unapproved, color the text red. */ \
		UpdateCellColor(pRow, nStartColumn + aldc##field, pData); \
	} \

void CAttendanceDlg::AddAttendanceDataToDatalist(AttendanceUser *pUser)
{
	if(m_pdlAttendance->GetRowCount() <= 0) {
		// (z.manning, 11/15/2007) - We should already have rows for each date.
		ASSERT(FALSE);
		return;
	}

	// (z.manning, 11/15/2007) - The first row should be a header row.
	IRowSettingsPtr pRow = m_pdlAttendance->GetFirstRow();
	ASSERT(pRow->GetValue(alcStartDate).vt == VT_NULL && pRow->GetValue(alcEndDate).vt == VT_NULL);
	int nStartColumn = alcBeginDynamicColumns;
	while(nStartColumn < m_pdlAttendance->GetColumnCount() && pRow->GetValue(nStartColumn + aldcPointer).vt != VT_NULL) {
		nStartColumn += aldcDynamicColumnCount;
	}

	if(nStartColumn >= m_pdlAttendance->GetColumnCount()) {
		// (z.manning, 11/15/2007) - It seems we did not have enough dynamic attendance columns created.
		
		// (j.jones 2012-12-24 12:02) - I discovered this also is hit if aldcPointer is not null, which happens if
		// we're trying to call this function for the same user twice. That means bad data in our master attendance list,
		// as this tab does not work properly if duplicate user objects exist in our m_arypAttendanceUsers array.
		ASSERT(FALSE);
		return;
	}

	// (z.manning, 12/03/2007) - Store a pointer to the user object in the first header row.
	pRow->PutValue(nStartColumn + aldcPointer, (long)pUser);
	// (z.manning, 11/28/2007) - Put the username in the header row so that we know which set of columns belongs
	// to which user.
	// (d.thompson 2014-02-24) - PLID 61015 - Hours worked now displays for all.  Move last name to hours worked, and first name 
	//	will now be in vacation/sick depending on hourly/salary.
	short nLastNameColIndex = nStartColumn + aldcHoursWorked;
	short nFirstNameColIndex = nStartColumn + aldcPTO;
	//Hourly users don't have a vacation column, so put the name in sick instead
	// (d.thompson 2014-02-27) - PLID 61016 - Hourly users (and thus everyone) now use PTO, sick is deprecated. 

	// (z.manning 2009-02-10 11:15) - PLID 31829 - Auto size the name columns right here since it
	// will be much quick before we add the rest of the rows which simply contain numbers whatever
	// width we set here we assume to be sufficient for the entire column.
	pRow->PutValue(nLastNameColIndex, _bstr_t(pUser->m_strLastName + ','));
	IColumnSettingsPtr pCol = m_pdlAttendance->GetColumn(nLastNameColIndex);
	pCol->PutStoredWidth(pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE));
	pRow->PutValue(nFirstNameColIndex, _bstr_t(pUser->m_strFirstName));
	pCol = m_pdlAttendance->GetColumn(nFirstNameColIndex);
	pCol->PutStoredWidth(pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE));

	pRow = pRow->GetNextRow();

	// (z.manning, 11/28/2007) - Sort the attednance data by date so that we only have to traverse the datalist
	// once, since we know it's in date order.
	pUser->SortByDate();

	COleCurrency cyTotalHoursWorked(0,0), cyTotalVacation(0,0), cyTotalSick(0,0), cyTotalOther(0,0), cyTotalPaid(0, 0), cyTotalUnpaid(0, 0);
	int nDataIndex = 0;
	for(; pRow != NULL && pRow->GetValue(alcStartDate).vt == VT_DATE; pRow = pRow->GetNextRow())
	{
		COleDateTime dtRowStart = VarDateTime(pRow->GetValue(alcStartDate));
		COleDateTime dtRowEnd = VarDateTime(pRow->GetValue(alcEndDate));
		if( (pUser->m_dtDateOfHire.GetStatus() == COleDateTime::valid && dtRowStart < pUser->m_dtDateOfHire) ||
			(pUser->m_dtDateOfTermination.GetStatus() == COleDateTime::valid && dtRowEnd > pUser->m_dtDateOfTermination) )
		{
			// (z.manning, 11/20/2007) - Gray out rows that come before the employee's date of hire or 
			// after their date of termination.
			for(int nCol = nStartColumn + 1; nCol < aldcDynamicColumnCount + nStartColumn; nCol++) {
				pRow->PutCellBackColor(nCol, RGB(180,180,180));
			}
		}

		// (z.manning, 11/28/2007) - We know the datalist is in ascending date order and before we started looping
		// we made sure the attendance data was also in date order. So, as long we haven't handled all the
		// attendance data for this user, then check and see if the next attendance appointment falls within
		// the date range of the current row we're traversing, and if it does then update this row appropriately.
		while( nDataIndex < pUser->m_arypAttendanceData.GetSize() &&
			pUser->m_arypAttendanceData.GetAt(nDataIndex)->dtDate >= dtRowStart &&
			pUser->m_arypAttendanceData.GetAt(nDataIndex)->dtDate <= dtRowEnd )
		{
			AttendanceData *pData = pUser->m_arypAttendanceData.GetAt(nDataIndex);

			// (z.manning, 11/28/2007) - Increment our totals but only if it falls within the date filter
			// (a.walling 2008-05-14 12:42) - PLID 27591 - VarDateTime no longer necessary
			if(pData->dtDate >= (m_dtpDateFilterStart.GetValue()) && pData->dtDate <= (m_dtpDateFilterEnd.GetValue())){
				cyTotalHoursWorked += pData->cyHoursWorked;
				// (z.manning, 02/08/2008) - Don't count floating holidays toward total
				if(pData->eType != atFloatingHoliday) {
					cyTotalVacation += pData->cyVacation;
				}
				cyTotalSick += pData->cySick;
				cyTotalOther += pData->cyOther;
				// (z.manning 2008-11-13 12:32) - PLID 31831 - paid/unpaid
				cyTotalPaid += pData->cyPaid;
				cyTotalUnpaid += pData->cyUnpaid;
			}

			UPDATE_CURRENCY_CELL(HoursWorked);
			// (d.thompson 2014-02-27) - PLID 61016 - Hack.  I don't want to go rename variables all over and this macro 
			//	unfortunately requires several to be in sync.  we'll just define a quick local enum to support the name matching.
			{
				enum eTempVacation {
					aldcVacation = aldcPTO,	//temporarily define that enum
				};
				UPDATE_CURRENCY_CELL(Vacation);
			}
			UPDATE_CURRENCY_CELL(Sick);
			// (z.manning 2008-11-13 13:15) - PLID 31831 - Paid/Unpaid columns
			UPDATE_CURRENCY_CELL(Paid);
			UPDATE_CURRENCY_CELL(Unpaid);
			UPDATE_CURRENCY_CELL(Other);
			
			CString strPreviousNote = VarString(pRow->GetValue(nStartColumn + aldcNotes), "");
			CString strNote;
			if(strPreviousNote.IsEmpty()) {
				strNote = pData->strNotes;
			}
			else {
				strNote = strPreviousNote + ", " + pData->strNotes;
			}
			pRow->PutValue(nStartColumn + aldcNotes, _bstr_t(strNote));
			
			CString strPreviousAdminNote = VarString(pRow->GetValue(nStartColumn + aldcAdminNotes), "");
			CString strAdminNote;
			if(strPreviousAdminNote.IsEmpty()) {
				strAdminNote = pData->strAdminNotes;
			}
			else {
				strAdminNote = strPreviousAdminNote + ", " + pData->strAdminNotes;
			}
			pRow->PutValue(nStartColumn + aldcAdminNotes, _bstr_t(strAdminNote));

			nDataIndex++;
		}
	}

	// (z.manning, 11/28/2007) - We better have gone through all the data.
	ASSERT(nDataIndex >= pUser->m_arypAttendanceData.GetSize());

	// (z.manning, 11/15/2007) - Handle the totals. First make sure we read to the footer rows.
	while(pRow->GetValue(alcStartDate).vt == VT_DATE) {
		pRow = pRow->GetNextRow();
	}
	// (z.manning, 11/15/2007) - First footer row should be border.
	if(pRow == NULL) {
		ASSERT(FALSE);
		return;
	}
	ASSERT(pRow->GetValue(alcDisplayDate).vt == VT_NULL);

	// (z.manning, 11/15/2007) - If the order of the footer columns changes at all then you will also
	// need to make changes to AddHeaderFooterRows.
	// (z.manning 2008-12-04 15:28) - PLID 32342 - Re-ordered the footer columns

	// (z.manning, 11/19/2007) - Now the year-to-date accrued amount.
	IRowSettingsPtr pAccruedRow = pRow->GetNextRow();
	if(pAccruedRow == NULL) {
		ASSERT(FALSE);
		return;
	}
	pAccruedRow->PutValue(nStartColumn + aldcPTO, _bstr_t(FormatAttendanceValue(pUser->GetAccruedVacation())));
	// (d.thompson 2014-02-27) - PLID 61016 - Combine sick into PTO - No changes here, we keep loading historic sick time for previous years.
	pAccruedRow->PutValue(nStartColumn + aldcSick, _bstr_t(FormatAttendanceValue(pUser->GetAccruedSick())));
	
	// (z.manning, 11/19/2007) - Now the grand total (available) row.
	IRowSettingsPtr pYearTotalRow = pAccruedRow->GetNextRow();
	if(pYearTotalRow == NULL) {
		ASSERT(FALSE);
		return;
	}	
	pYearTotalRow->PutValue(nStartColumn + aldcPTO, (long)pUser->GetAvailableVacation());
	// (d.thompson 2014-02-27) - PLID 61016 - Combine sick into PTO - No changes here, we keep loading historic sick time for previous years.
	pYearTotalRow->PutValue(nStartColumn + aldcSick, (long)pUser->GetAvailableSick());

	// (z.manning, 11/19/2007) - Now the subtotal (used) row.
	IRowSettingsPtr pSubtotalRow = pYearTotalRow->GetNextRow();
	if(pSubtotalRow == NULL) {
		ASSERT(FALSE);
		return;
	}
	pSubtotalRow->PutValue(nStartColumn + aldcHoursWorked, _bstr_t(FormatAttendanceValue(cyTotalHoursWorked)));
	pSubtotalRow->PutValue(nStartColumn + aldcPTO, _bstr_t(FormatAttendanceValue(cyTotalVacation)));
	// (d.thompson 2014-02-27) - PLID 61016 - Combine sick into PTO - No changes here, we keep loading historic sick time for previous years.
	pSubtotalRow->PutValue(nStartColumn + aldcSick, _bstr_t(FormatAttendanceValue(cyTotalSick)));
	// (z.manning 2008-11-13 12:26) - PLID 31831 - Added paid and unpaid columns
	pSubtotalRow->PutValue(nStartColumn + aldcPaid, _bstr_t(FormatAttendanceValue(cyTotalPaid)));
	pSubtotalRow->PutValue(nStartColumn + aldcUnpaid, _bstr_t(FormatAttendanceValue(cyTotalUnpaid)));
	pSubtotalRow->PutValue(nStartColumn + aldcOther, _bstr_t(FormatAttendanceValue(cyTotalOther)));

	// (z.manning, 11/19/2007) - Now the net balance
	IRowSettingsPtr pBalanceRow = pSubtotalRow->GetNextRow();
	if(pBalanceRow == NULL) {
		ASSERT(FALSE);
		return;
	}	
	// (z.manning, 01/16/2008) - We used to calculate this as accrued - total used, however, that didn't make much
	// sense because you don't technically accrue all your vacation for the year until the final day of the year.
	// This would mean that once you scheduled all your vacation, you would have a negative balance. So, after talking
	// with Najla, we agreed to make this total available - total used. (However, we are going to leave the accrued
	// rows there as a reference.)
	// (z.manning 2008-11-21 12:37) - PLID 32139 - We now have a function to get vacation
	// balance text that also inlucdes FH if they have their floating holiday left.
	pBalanceRow->PutValue(nStartColumn + aldcPTO, _bstr_t(pUser->GetBalanceVacationText()));
	// (d.thompson 2014-02-27) - PLID 61016 - Combine sick into PTO - No changes here, we keep loading historic sick time for previous years.
	pBalanceRow->PutValue(nStartColumn + aldcSick, _bstr_t(FormatAttendanceValue(pUser->GetBalanceSick())));

	// (z.manning, 11/28/2007) - Size the columns based on the data we just put in.
	// (z.manning 2009-02-10 11:13) - PLID 31829 - Nevermind, this was slow and not necessary for all columns.
	// The columns that show first and last name are now handled earlier in this function. The only
	// other ones we handle size here now are the notes columns.
	/*for(short nColIndex = nStartColumn + aldcHoursWorked; nColIndex < nStartColumn + aldcDynamicColumnCount; nColIndex++)
	{
		IColumnSettingsPtr pCol = m_pdlAttendance->GetColumn(nColIndex);
		if(pCol != NULL && pCol->GetColumnStyle() & csVisible) {
			long nCalculatedColWidth = m_pdlAttendance->CalcColumnWidthFromData(nColIndex, VARIANT_TRUE, VARIANT_FALSE);
			pCol->PutStoredWidth(m_arynSelectedUserIDs.GetSize() > 1 && nCalculatedColWidth > MAX_COL_WIDTH ? MAX_COL_WIDTH : nCalculatedColWidth);
		}
	}*/
	long nCalculatedColWidth;
	pCol = m_pdlAttendance->GetColumn(nStartColumn + aldcNotes);
	nCalculatedColWidth = pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE);
	pCol->PutStoredWidth(m_arynSelectedUserIDs.GetSize() > 1 && nCalculatedColWidth > MAX_COL_WIDTH ? MAX_COL_WIDTH : nCalculatedColWidth);
	pCol = m_pdlAttendance->GetColumn(nStartColumn + aldcAdminNotes);
	nCalculatedColWidth = pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE);
	pCol->PutStoredWidth(m_arynSelectedUserIDs.GetSize() > 1 && nCalculatedColWidth > MAX_COL_WIDTH ? MAX_COL_WIDTH : nCalculatedColWidth);
}

void CAttendanceDlg::RefreshAttendanceInfoByUser(long nUserID)
{
	AttendanceUser *pUser = m_AttendanceInfo.GetAttendanceUserByID(nUserID);

	IRowSettingsPtr pRow = m_pdlAttendance->GetFirstRow();
	ASSERT(pRow->GetValue(alcStartDate).vt == VT_NULL && pRow->GetValue(alcEndDate).vt == VT_NULL);
	BOOL bFound = FALSE;
	int nStartColumn = alcBeginDynamicColumns + aldcPointer;
	while(nStartColumn < m_pdlAttendance->GetColumnCount() && pRow->GetValue(nStartColumn + aldcPointer).vt != VT_NULL) {
		if(VarLong(pRow->GetValue(nStartColumn + aldcPointer)) == (long)pUser) {
			// (z.manning, 12/03/2007) - We found the columns for the user we're looking for.
			bFound = TRUE;
			break;
		}
		nStartColumn += aldcDynamicColumnCount;
	}

	if(bFound)
	{
		// (z.manning, 12/03/2007) - Clear out everything for this user's attendance columns.
		_variant_t varNull;
		varNull.vt = VT_NULL;
		for(; pRow != NULL; pRow = pRow->GetNextRow())
		{
			for(short nCol = nStartColumn; nCol < nStartColumn + aldcDynamicColumnCount; nCol++) {
				pRow->PutValue(nCol, varNull);
			}
		}

		// (z.manning, 12/03/2007) - Now re-add the data for this user.
		AddAttendanceDataToDatalist(pUser);
	}
}

void CAttendanceDlg::OnRButtonDownAttendanceList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);

		// (z.manning, 12/03/2007) - If the row isn't already, ensure it's highlighted.
		if(pRow != NULL) {
			if(!pRow->IsHighlighted()) {
				m_pdlAttendance->PutCurSel(pRow);
			}
		}

		enum EMenuOptions
		{
			moApprove = 1,
			moDeny,
			moViewExisting,
			moNewRequest,
			moCopy,
			moPaste,
			moDelete,
		};

		CMenu mnu;
		mnu.CreatePopupMenu();

		BOOL bIsCurrenUserAdmin = CheckCurrentUserPermissions(bioAttendance, sptDynamic0, FALSE, 0, TRUE);

		AttendanceUser *pUser = GetAttendanceUserFromColumn(nCol);
		UINT nCopyMenuFlags = MF_GRAYED, nPasteMenuFlags = MF_GRAYED;
		if(pUser != NULL && m_pClippedAttendanceData != NULL && pUser->m_nUserID == GetCurrentUserID()) {
			nPasteMenuFlags = MF_ENABLED;
		}
		UINT nViewExistingFlags = MF_GRAYED;
		CArray<AttendanceData*,AttendanceData*> arypDataToApprove;
		CArray<AttendanceData*,AttendanceData*> arypDataToDelete;
		if(pUser != NULL)
		{
			// (b.cardillo 2008-07-17 16:41) - PLID 30745 - I put a try/catch around the code that uses 
			// arylpRows, and while I was at it I moved the declaration of arylpRows inside here since 
			// it's only used here.  The try/catch is necessary so we can be sure to safely release all 
			// the rows that are referenced by the array.
			CArray<LPDISPATCH,LPDISPATCH> arylpRows;
			try {
				GetAllHighlightedRows(arylpRows);

				for(int nRowIndex = 0; nRowIndex < arylpRows.GetSize(); nRowIndex++)
				{
					IRowSettingsPtr pRow(arylpRows.GetAt(nRowIndex));
					ASSERT(pRow != NULL);
					if(pRow->GetValue(alcStartDate).vt == VT_DATE && pRow->GetValue(alcEndDate).vt == VT_DATE)
					{
						COleDateTime dtStart = VarDateTime(pRow->GetValue(alcStartDate));
						COleDateTime dtEnd = VarDateTime(pRow->GetValue(alcEndDate));
						for(int nDataIndex = 0; nDataIndex < pUser->m_arypAttendanceData.GetSize(); nDataIndex++)
						{
							AttendanceData *pData = pUser->m_arypAttendanceData.GetAt(nDataIndex);
							if(pData->dtDate >= dtStart && pData->dtDate <= dtEnd)
							{
								if(!pData->IsApproved()) {
									arypDataToApprove.Add(pData);
								}

								// (z.manning, 02/13/2008) - PLID 28295 - They are allowed to delete an existing time off request if...
								//  A. The user is an admin
								//  B. The user is a manager is the request is not yet approved
								//  C. The request is not approved and the same user wants to edit it
								//  D. The manager who approved it can edit it so long as the pay period in which the request occurs isn't locked
								if( bIsCurrenUserAdmin ||
									(m_bIsCurrentUserManager && VarLong(pData->varApproverUserID,-1) == GetCurrentUserID() && !m_AttendanceInfo.IsDateLocked(pData->dtDate)) ||
									((pUser->m_nUserID == GetCurrentUserID() || m_bIsCurrentUserManager) && !pData->IsApproved())
									)
								{
									ASSERT(pData->nID > 0);
									arypDataToDelete.Add(pData);
								}
							}

							if(arylpRows.GetSize() == 1 && dtStart == pData->dtDate) {
								nViewExistingFlags = MF_ENABLED;
							}

							if(arylpRows.GetSize() == 1 && pUser->m_nUserID == GetCurrentUserID())
							{
								ASSERT(nRowIndex == 0);
								if(dtStart == pData->dtDate) {
									nCopyMenuFlags = MF_ENABLED;
									nPasteMenuFlags = MF_GRAYED;
								}
							}
							else {
								nCopyMenuFlags = MF_GRAYED;
								nPasteMenuFlags = MF_GRAYED;
							}
						}
					}
					else
					{
						nCopyMenuFlags = MF_GRAYED;
						nPasteMenuFlags = MF_GRAYED;
					}
				}
				// (b.cardillo 2008-07-17 16:41) - PLID 30745 - Now that we reference-count the elements in the array 
				// we need to release them all before arylpRows goes out of scope.
				SafeEmptyArrayOfCOMPointers(arylpRows);
			} catch (...) {
				// (b.cardillo 2008-07-17 16:41) - PLID 30745 - We don't want an exception to result in a memory leak 
				// so we empty the array here before letting the exception fly (but ignore any exceptions IT might throw).
				try {
					SafeEmptyArrayOfCOMPointers(arylpRows);
				} catch (...) { }
				throw;
			}
		}

		// (z.manning, 12/06/2007) - PLID 28295 - Only managers and those with admin permissions may approve/deny requests.
		if((m_bIsCurrentUserManager || bIsCurrenUserAdmin) && pUser != NULL)
		{
			UINT nApprovalFlags;
			if(arypDataToApprove.GetSize() > 0) {
				nApprovalFlags = MF_ENABLED;
			}
			else {
				nApprovalFlags = MF_GRAYED;
			}
			mnu.AppendMenu(nApprovalFlags, moApprove, FormatString("Approve attendance for '%s'", pUser->GetFullName()));
			mnu.AppendMenu(nApprovalFlags, moDeny, FormatString("Deny attendance for '%s'", pUser->GetFullName()));
			mnu.AppendMenu(MF_SEPARATOR);
		}

		// (z.manning, 12/03/2007) - Only allow copy/pasting in day view.
		if(IsDlgButtonChecked(IDC_RADIO_DAY) == BST_CHECKED) {
			mnu.AppendMenu(nCopyMenuFlags, moCopy, "Copy");
			mnu.AppendMenu(nPasteMenuFlags, moPaste, "Paste");
			mnu.AppendMenu(MF_SEPARATOR);
			mnu.AppendMenu(nViewExistingFlags, moViewExisting, "View Existing Time Off");
		}
		mnu.AppendMenu(MF_ENABLED, moNewRequest, "Request Time Off");
		mnu.AppendMenu(MF_SEPARATOR);
		mnu.AppendMenu(arypDataToDelete.GetSize() > 0 ? MF_ENABLED : MF_GRAYED, moDelete, "Delete");

		CPoint ptClicked(x, y);
		GetDlgItem(IDC_ATTENDANCE_LIST)->ClientToScreen(&ptClicked);
		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);

		switch(nResult)
		{
			case moApprove:
			{
				// (z.manning, 01/11/2008) - PLID 28461 - Need to pass in a parent wnd for use with emailing.
				pUser->ApproveAttendanceData(arypDataToApprove, this);
				RefreshAttendanceInfoByUser(pUser->m_nUserID);
			}
			break;

			case moDeny:
			{
				// (z.manning, 01/11/2008) - PLID 28461 - Need to pass in a parent wnd for use with emailing.
				pUser->DenyAttendanceData(arypDataToApprove, this);
				RefreshAttendanceInfoByUser(pUser->m_nUserID);				
			}
			break;

			case moCopy:
			{
				if(pUser != NULL) {
					if(m_pClippedAttendanceData == NULL) {
						m_pClippedAttendanceData = new AttendanceData;
					}
					*m_pClippedAttendanceData = *pUser->GetAttendanceDataForDate(VarDateTime(pRow->GetValue(alcStartDate)));
				}
			}
			break;

			case moPaste:
			{
				if(m_pClippedAttendanceData != NULL)
				{
					AttendanceData *pNewData = new AttendanceData;
					*pNewData = *m_pClippedAttendanceData;
					pNewData->nID = -1;
					// (z.manning, 05/20/2008) - PLID 30105 - Reset the parent ID so this attendance appt
					// is not grouped with attendance appontments potentially months apart.
					pNewData->varParentID.vt = VT_NULL;
					if(m_pClippedAttendanceData->varApproverUserID.vt == VT_NULL || !(bIsCurrenUserAdmin || m_bIsCurrentUserManager)) {
						pNewData->varApproverUserID.vt = VT_NULL;
					}
					pNewData->dtDate = VarDateTime(pRow->GetValue(alcStartDate));

					pNewData->SaveToData();
					pUser->AddAttendanceData(pNewData);
					RefreshAttendanceInfoByUser(pUser->m_nUserID);
				}
				else {
					ASSERT(FALSE);
				}
			}
			break;

			case moNewRequest:
			{
				AttendanceData *pData = new AttendanceData;
				if((m_bIsCurrentUserManager || bIsCurrenUserAdmin) && pUser != NULL) {
					pData->pUser = pUser;
				}
				else {
					pData->pUser = m_AttendanceInfo.GetAttendanceUserByID(GetCurrentUserID());
				}
				if(pRow != NULL) {
					if(pRow->GetValue(alcStartDate).vt == VT_DATE) {
						pData->dtDate = VarDateTime(pRow->GetValue(alcStartDate));
					}
				}
				OpenAttendanceEntryDlg(pData);
			}
			break;

			case moViewExisting:
			{
				AttendanceData *pData = pUser->GetAttendanceDataForDate(VarDateTime(pRow->GetValue(alcStartDate)));
				OpenAttendanceEntryDlg(pData);
			}
			break;

			case moDelete:
			{
				int nResult = MessageBox("Are you sure you want to delete the selected time off request(s)?", NULL, MB_YESNO);
				if(nResult == IDYES)
				{
					for(int nDataToDeleteIndex = 0; nDataToDeleteIndex < arypDataToDelete.GetSize(); nDataToDeleteIndex++)
					{
						AttendanceData *pData = arypDataToDelete.GetAt(nDataToDeleteIndex);
						pData->DeleteFromData();
						pUser->RemoveAttendanceData(pData);
					}
					RefreshAttendanceInfoByUser(pUser->m_nUserID);
				}
			}
			break;
		}

	}NxCatchAll("CAttendanceDlg::OnRButtonDownAttendanceList");
}

void CAttendanceDlg::GetAllHighlightedRows(OUT CArray<LPDISPATCH,LPDISPATCH> &arylpRows)
{
	// (b.cardillo 2008-07-17 16:41) - PLID 30745 - We used to just empty the array here, which, now 
	// that we do reference counting on the elements in the array, would lead to memory leaks.  So we 
	// empty it properly using the "safe" function.
	SafeEmptyArrayOfCOMPointers(arylpRows);

	// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
	for (IRowSettingsPtr pRow = m_pdlAttendance->GetFirstSelRow(); pRow != NULL; pRow = pRow->GetNextSelRow()) {
		LPDISPATCH pDisp = ((IDispatchPtr)pRow).Detach();
		arylpRows.Add(pDisp);
		// (b.cardillo 2008-07-17 16:41) - PLID 30745 - We used to release pDisp here, but since we just 
		// placed it in the array, we need to retain one reference count.  Thus we no longer release, and 
		// we rely on the caller to release when they are done.
	}
}

AttendanceUser* CAttendanceDlg::GetAttendanceUserFromColumn(const short nColumnIndex)
{
	short nStartIndex = GetUserStartColumn(nColumnIndex);
	if(nStartIndex < 0) {
		return NULL;
	}

	ASSERT(m_pdlAttendance->GetFirstRow() != NULL);
	if(m_pdlAttendance->GetFirstRow()->GetValue(nStartIndex + aldcPointer).vt != VT_I4) {
		ThrowNxException("CAttendanceDlg::GetAttendanceUserFromColumn - Looking for user pointer in column %i which is not valid.", nColumnIndex);
	}

	return (AttendanceUser*)VarLong(m_pdlAttendance->GetFirstRow()->GetValue(nStartIndex + aldcPointer));
}

short CAttendanceDlg::GetUserStartColumn(const short nRelativeToColumnIndex)
{
	short nStartColumn = nRelativeToColumnIndex;
	// (z.manning, 12/03/2007) - If they clicked to the left of the user columns, then we can't return a user
	if(nStartColumn < alcBeginDynamicColumns) {
		return -1;
	}

	while((nStartColumn - alcBeginDynamicColumns) % aldcDynamicColumnCount != 0) {
		nStartColumn--;
	}
	
	return nStartColumn;
}

void CAttendanceDlg::OnEditingStartingAttendanceList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (z.manning, 12/04/2007) - None of the non-dynamic columns should be editable
		if(nCol < alcBeginDynamicColumns) {
			ASSERT(FALSE);
			*pbContinue = FALSE;
			return;
		}
		
		// (z.manning, 12/06/2007) - Only allow editing on day view.
		if(IsDlgButtonChecked(IDC_RADIO_DAY) != BST_CHECKED) {
			*pbContinue = FALSE;
			return;
		}

		// (z.manning, 12/10/2007) - Only the date rows are ever editable.
		if(pRow->GetValue(alcStartDate).vt != VT_DATE) {
			*pbContinue = FALSE;
			return;
		}

		// (z.manning, 12/06/2007) - Admins can always change anything.
		if(CheckCurrentUserPermissions(bioAttendance, sptDynamic0, FALSE, 0, TRUE)) {
			return;
		}

		COleDateTime dtRowDate = VarDateTime(pRow->GetValue(alcStartDate));
		AttendanceUser *pUser = GetAttendanceUserFromColumn(nCol);
		AttendanceData *pData = pUser->GetAttendanceDataForDate(dtRowDate);

		// (z.manning, 12/06/2007) - PLID 28295 - If not an admin, then you can only edit yourself unless you're a manager.
		if(pUser->m_nUserID != GetCurrentUserID() && !m_bIsCurrentUserManager) {
			*pbContinue = FALSE;
			return;
		}

		// (z.manning, 02/26/2008) - PLID 28885 - Don't let them enter anything if payroll is closed for this date.
		if(m_AttendanceInfo.IsDateLocked(dtRowDate)) {
			*pbContinue = FALSE;
			return;
		}

		if(pData != NULL) {
			// (z.manning, 12/04/2007) - PLID 28295 - There is already data for this date.  They can only edit it if it has
			// not yet been approved.
			// (z.manning, 02/13/2008) - Also allow managers to edit things they approved so long as payroll
			// has not been locked.
			if( pData->IsApproved() &&
				!(m_bIsCurrentUserManager && VarLong(pData->varApproverUserID,-1) == GetCurrentUserID())
				)
			{
				*pbContinue = FALSE;
				return;
			}
		}

		switch((nCol - alcBeginDynamicColumns) % aldcDynamicColumnCount)
		{
			case aldcHoursWorked:
			{
				// (z.manning, 12/04/2007) - No editing hours worked for days in the future.
				// (z.manning, 04/11/2008) - PLID 29628 - No editing hours for any date other than the current one.
				COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
				dtCurrent.SetDate(dtCurrent.GetYear(), dtCurrent.GetMonth(), dtCurrent.GetDay());
				if(dtRowDate != dtCurrent) {
					*pbContinue = FALSE;
				}
			}
			break;

			// (z.manning 2011-08-02 12:51) - PLID 44524 - Changed the logic here a bit-- normal users can now request
			// unpaid time off but no longer can request other.

			case aldcPaid:
			case aldcOther:
				if(!m_bIsCurrentUserManager) {
					// (z.manning 2008-11-13 15:31) - PLID 31831 - Only managers may edit the paid/other columns
					*pbContinue = FALSE;
					return;
				}
				// (z.manning 2008-11-13 15:32) - Intentionally no break here.
			case aldcPTO:
			case aldcUnpaid:
			{
				// (z.manning, 05/20/2008) - PLID 30105 - No editing time off from here if we're dealing with a group
				if(pData != NULL && pData->IsPartOfGroup()) {
					*pbContinue = FALSE;
				}
			}
			break;

			case aldcSick:
				// (d.thompson 2014-02-27) - PLID 61016 - Combine sick into PTO - Sick is no longer editable, so if you got here we have
				//	a serious problem.
				AfxThrowNxException("OnEditingStartingAttendanceList:  Sick time should no longer be editable.");
				break;
		}

	}NxCatchAll("CAttendanceDlg::OnEditingStartingAttendanceList");
}

void CAttendanceDlg::OnEditingFinishingAttendanceList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (z.manning, 12/04/2007) - None of the non-dynamic columns should be editable
		if(nCol < alcBeginDynamicColumns) {
			ASSERT(FALSE);
			return;
		}

		ASSERT(IsDlgButtonChecked(IDC_RADIO_DAY) == BST_CHECKED);

		COleDateTime dtRowDate = VarDateTime(pRow->GetValue(alcStartDate));

		AttendanceUser *pUser = GetAttendanceUserFromColumn(nCol);
		AttendanceData *pData = pUser->GetAttendanceDataForDate(dtRowDate);

		switch((nCol - alcBeginDynamicColumns) % aldcDynamicColumnCount)
		{
			case aldcSick:
				// (d.thompson 2014-02-27) - PLID 61016 - Combine sick into PTO - Sick is no longer editable, so if you got here we have
				//	a serious problem.
				AfxThrowNxException("OnEditingFinishingAttendanceList:  Sick time should no longer be editable.");
				break;

			case aldcHoursWorked:
			case aldcPTO:
			case aldcOther:
			case aldcPaid:
			case aldcUnpaid:
			{
				CString strOld = VarString(varOldValue, "");
				EnsureValidAttendanceString(strOld, 24);
				CString strNew = VarString(*pvarNewValue, "");
				EnsureValidAttendanceString(strNew, 24);
				if(strOld == strNew) {
					*pbCommit = FALSE;
				}
				else {
					_variant_t varNew(strNew);
					COleCurrency cy;
					if(cy.ParseCurrency(strNew)) {
						if(cy == COleCurrency(0,0)) {
							varNew.vt = VT_NULL;
						}
					}
					*pvarNewValue = varNew.Detach();
				}
			}
			break;
		}

	}NxCatchAll("CAttendanceDlg::OnEditingFinishingAttendanceList");
}

void CAttendanceDlg::OnEditingFinishedAttendanceList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (z.manning, 12/04/2007) - We shouldn't need to do anything if they didn't commit.
		if(!bCommit) {
			return;
		}

		// (z.manning, 12/04/2007) - None of the non-dynamic columns should be editable
		if(nCol < alcBeginDynamicColumns) {
			ASSERT(FALSE);
			return;
		}

		BOOL bUpdateCurrentUser = FALSE;
		BOOL bIsCurrenUserAdmin = CheckCurrentUserPermissions(bioAttendance, sptDynamic0, FALSE, 0, TRUE);

		ASSERT(IsDlgButtonChecked(IDC_RADIO_DAY) == BST_CHECKED);

		COleDateTime dtRowDate = VarDateTime(pRow->GetValue(alcStartDate));

		AttendanceUser *pUser = GetAttendanceUserFromColumn(nCol);
		AttendanceData *pData = pUser->GetAttendanceDataForDate(dtRowDate);

		if(pData != NULL && pData->nID > 0)
		{
			// (z.manning, 12/04/2007) - If every colum is now null then we should delete this record from the database
			// as it's now pointless.
			BOOL bAllNull = TRUE;
			int nStartCol = GetUserStartColumn(nCol);
			for(short nUserCol = nStartCol; nUserCol < nStartCol + aldcDynamicColumnCount; nUserCol++)
			{
				if(pRow->GetValue(nUserCol).vt != VT_NULL && pRow->GetValue(nUserCol) != _variant_t("")) {
					bAllNull = FALSE;
					break;
				}
			}

			if(bAllNull) {
				pData->DeleteFromData();
				pUser->RemoveAttendanceData(pData);
				return;
			}
		}

		
		COleCurrency cyNew;
		if(!cyNew.ParseCurrency(VarString(varNewValue, ""))) {
			cyNew.SetCurrency(0, 0);
		}

		_variant_t varNull;
		varNull.vt = VT_NULL;
		BOOL bOpenAttendanceEntryDlg = FALSE;
		short nRelativeCol = (nCol - alcBeginDynamicColumns) % aldcDynamicColumnCount;
		switch(nRelativeCol)
		{
			case aldcHoursWorked:
			{
				if(pData == NULL) {
					pData = new AttendanceData;
					pData->dtDate = dtRowDate;
					pData->pUser = pUser;
					pUser->AddAttendanceData(pData);
				}
				pData->cyHoursWorked = cyNew;
				pData->SaveToData();
				bUpdateCurrentUser = TRUE; // (z.manning 2009-10-27 12:20) - PLID 36063
			}
			break;

			case aldcPTO:
			{
				if(pData != NULL) {
					// (z.manning, 12/04/2007) - It's an existing attendance instance, so just update it.
					pData->cyVacation = cyNew;
					pData->SaveToData();
				}
				else {
					// (z.manning, 12/04/2007) - New time off request, so open the attendance entry dialog.
					pData = new AttendanceData;
					pData->cyVacation = cyNew;

					// (d.thompson 2014-02-27) - PLID 61016 - Do not open it for hourly users.
					if(pUser->m_eType != autHourly) {
						bOpenAttendanceEntryDlg = TRUE;
					}
					else {
						pData->dtDate = dtRowDate;
						pData->pUser = pUser;
						pUser->AddAttendanceData(pData);
						if(bIsCurrenUserAdmin || m_bIsCurrentUserManager) {
							pData->varApproverUserID = GetCurrentUserID();
						}
						pData->SaveToData();
					}
				}
				bUpdateCurrentUser = TRUE; // (z.manning 2009-10-27 12:20) - PLID 36063
			}
			break;

			case aldcSick:
			{
				// (d.thompson 2014-02-27) - PLID 61016 - Combine sick into PTO - Sick is no longer editable, so you shouldn't be able to get here.
				AfxThrowNxException("Cannot save changes to sick time.");
			}
			break;

			// (z.manning 2008-11-13 13:11) - PLID 31831 - Added paid and unpaid columns
			case aldcPaid:
			{
				if(pData == NULL) {
					pData = new AttendanceData;
					pData->dtDate = dtRowDate;
					pData->pUser = pUser;
					pUser->AddAttendanceData(pData);
					if(bIsCurrenUserAdmin || m_bIsCurrentUserManager) {
						pData->varApproverUserID = GetCurrentUserID();
					}
				}
				pData->cyPaid = cyNew;
				pData->SaveToData();
				bUpdateCurrentUser = TRUE; // (z.manning 2009-10-27 12:20) - PLID 36063
			}
			break;

			// (z.manning 2008-11-13 13:11) - PLID 31831 - Added paid and unpaid columns
			case aldcUnpaid:
			{
				if(pData != NULL) {
					// (z.manning, 12/04/2007) - It's an existing attendance instance, so just update it.
					pData->cyUnpaid = cyNew;
					pData->SaveToData();
				}
				else {
					// (z.manning, 12/04/2007) - New time off request, so open the attendance entry dialog.
					pData = new AttendanceData;
					pData->cyUnpaid = cyNew;
					// (z.manning 2011-08-02 12:55) - PLID 44524 - We now open the attendance entry dialog when entring
					// unpaid time off.
					bOpenAttendanceEntryDlg = TRUE;
				}
				bUpdateCurrentUser = TRUE; // (z.manning 2009-10-27 12:20) - PLID 36063
			}
			break;

			case aldcOther:
			{
				if(pData == NULL) {
					pData = new AttendanceData;
					pData->dtDate = dtRowDate;
					pData->pUser = pUser;
					pUser->AddAttendanceData(pData);
					if(bIsCurrenUserAdmin || m_bIsCurrentUserManager) {
						pData->varApproverUserID = GetCurrentUserID();
					}
				}
				pData->cyOther = cyNew;
				pData->SaveToData();
				bUpdateCurrentUser = TRUE; // (z.manning 2009-10-27 12:20) - PLID 36063
			}
			break;

			case aldcNotes:
			{
				if(pData == NULL) {
					pData = new AttendanceData;
					pData->dtDate = dtRowDate;
					pData->pUser = pUser;
					pUser->AddAttendanceData(pData);
				}
				pData->strNotes = VarString(varNewValue);
				pData->SaveToData();
			}
			break;

			case aldcAdminNotes:
			{
				if(pData == NULL) {
					pData = new AttendanceData;
					pData->dtDate = dtRowDate;
					pData->pUser = pUser;
					pUser->AddAttendanceData(pData);
				}
				pData->strAdminNotes = VarString(varNewValue);
				pData->SaveToData();
			}
			break;
		}

		if(bOpenAttendanceEntryDlg) {
			pData->dtDate = dtRowDate;
			pData->pUser = pUser;
			long nOldUserID = pUser->m_nUserID;
			AttendanceData *pAttendanceEntryData = NULL;
			if(OpenAttendanceEntryDlg(pData, FALSE, pAttendanceEntryData) == IDOK) {
				bUpdateCurrentUser = TRUE;
			}
			else {
				pData = pAttendanceEntryData;
			}

			if(pData == NULL || pData->pUser->m_nUserID != nOldUserID) {
				bUpdateCurrentUser = TRUE;
			}
		}

		if(bUpdateCurrentUser) {
			// (z.manning 2009-10-27 12:26) - PLID 36063 - We need to refresh everything for this user.
			RefreshAttendanceInfoByUser(pUser->m_nUserID);
		}
		else {
			if(pData != NULL && nRelativeCol != aldcNotes && nRelativeCol != aldcAdminNotes) {
				UpdateCellColor(pRow, nCol, pData);
			}

			IColumnSettingsPtr pCol = m_pdlAttendance->GetColumn(nCol);
			if(pCol != NULL && pCol->GetColumnStyle() & csVisible) {
				long nCalculatedColWidth = m_pdlAttendance->CalcColumnWidthFromData(nCol, VARIANT_TRUE, VARIANT_FALSE);
				pCol->PutStoredWidth(m_arynSelectedUserIDs.GetSize() > 1 && nCalculatedColWidth > MAX_COL_WIDTH ? MAX_COL_WIDTH : nCalculatedColWidth);
			}
		}

	}NxCatchAll("CAttendanceDlg::OnEditingFinishedAttendanceList");
}

void CAttendanceDlg::OnDestroy() 
{
	try
	{
		// (z.manning, 12/05/2007) - Remember what department was selected.
		SetRemotePropertyInt("AttendanceDepartmentID", m_nLastSelectedDepartmentID, 0, GetCurrentUserName());
		SetRemotePropertyInt("AttendanceSelectedUserID", m_nLastSelectedUserID, 0, GetCurrentUserName());

		CNxDialog::OnDestroy();	

	}NxCatchAll("CAttendanceDlg::OnDestroy");
}

void CAttendanceDlg::UpdateCellColor(LPDISPATCH lpRow, short nCol, AttendanceData *pData)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL || pData == NULL) {
		ASSERT(FALSE);
		return;
	}

	// (z.manning, 12/11/2007) - Update the cell's text color based on the approval status.
	if(pData->IsApproved()) {
		pRow->PutCellForeColor(nCol, RGB(0,0,0));
		// (z.manning, 11/24/2008) - PLID 32168 - Changed the approved time off selection foreground
		// color to white so you can read it better when the row is highlighted.
		pRow->PutCellForeColorSel(nCol, RGB(255,255,255));
	}
	else {
		pRow->PutCellForeColor(nCol, UNAPPROVED_FORE_COLOR);
		pRow->PutCellForeColorSel(nCol, UNAPPROVED_FORE_COLOR);
	}
}

void CAttendanceDlg::OnAttendanceOptions() 
{
	try
	{
		CAttendanceOptionsDlg dlg(this);
		if(dlg.DoModal() == IDOK) {
			UpdateView();
		}

	}NxCatchAll("CAttendanceDlg::OnAttendanceOptions");
}

void CAttendanceDlg::OnDblClickCellAttendanceList(LPDISPATCH lpRow, short nColIndex) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (z.manning, 12/03/2007) - If they clicked to the left of the user columns, then we don't do anything.
		if(nColIndex < alcBeginDynamicColumns) {
			return;
		}

		// (z.manning, 12/18/2007) - If this isn't a date row (i.e. header/footer rows) then we do nothing.
		if(pRow->GetValue(alcStartDate).vt != VT_DATE) {
			return;
		}
		COleDateTime dtStart = VarDateTime(pRow->GetValue(alcStartDate));
		COleDateTime dtEnd = VarDateTime(pRow->GetValue(alcEndDate));

		AttendanceUser *pUser = GetAttendanceUserFromColumn(nColIndex);
		if(pUser == NULL) {
			ASSERT(FALSE);
			return;
		}

		BOOL bIsCurrenUserAdmin = CheckCurrentUserPermissions(bioAttendance, sptDynamic0, FALSE, 0, TRUE);

		CWaitCursor wc;
		if(IsDlgButtonChecked(IDC_RADIO_DAY) == BST_CHECKED)
		{
			// (z.manning, 12/18/2007) - For day view, we simply open the attendance entry dialog when they double click.
			AttendanceData *pData = pUser->GetAttendanceDataForDate(dtStart);
			if(pData == NULL) {
				pData = new AttendanceData;
				pData->dtDate = dtStart;
				if((m_bIsCurrentUserManager || bIsCurrenUserAdmin) && pUser != NULL) {
					pData->pUser = pUser;
				}
				else {
					pData->pUser = m_AttendanceInfo.GetAttendanceUserByID(GetCurrentUserID());
				}
			}
			OpenAttendanceEntryDlg(pData);
		}
		else
		{
			// (z.manning, 12/18/2007) - When double clicking a row in non-day view, open day view with the
			// date range of the row they clicked.
			// (a.walling 2008-05-14 12:42) - PLID 27591 - COleVariant no longer necessary
			m_dtpDateFilterStart.SetValue((dtStart));
			m_dtpDateFilterEnd.SetValue((dtEnd));
			CheckDlgButton(IDC_RADIO_DAY, BST_CHECKED);
			CheckDlgButton(IDC_RADIO_MONTH, BST_UNCHECKED);
			CheckDlgButton(IDC_RADIO_YEAR, BST_UNCHECKED);
			CheckDlgButton(IDC_RADIO_CUSTOM, BST_UNCHECKED);
			LRESULT dummy;
			OnChangeDateFilterStart(NULL, &dummy);
		}

	}NxCatchAll("CAttendanceDlg::OnDblClickCellAttendanceList");
}

void CAttendanceDlg::OnLockPayroll() 
{
	try
	{
		if(!CheckCurrentUserPermissions(bioAttendance, sptDynamic0)) {
			return;
		}

		CEnterActiveDateDlg dlg(this);
		dlg.m_bAllowCancel = true;
		dlg.m_strWindowTitle = "Choose payroll close date";
		dlg.m_strPrompt = "Enter the date that the pay period ends";
		dlg.m_dtDate = COleDateTime::GetCurrentTime();
		if(dlg.DoModal() == IDOK)
		{
			COleDateTime dtLockDate = dlg.m_dtDate;
			dtLockDate.SetDate(dtLockDate.GetYear(), dtLockDate.GetMonth(), dtLockDate.GetDay());
			CString strSqlDate = FormatDateTimeForSql(dtLockDate, dtoDate);

			// (z.manning, 02/12/2008) - PLID 28885 - Don't allow locking future dates.
			if(dtLockDate > COleDateTime::GetCurrentTime()) {
				MessageBox("You may not lock payroll on a date in the future.");
				return;
			}

			// (z.manning, 02/11/2008) - PLID 28885 - Don't allow users to enter the same date more than once.
			if(ReturnsRecords("SELECT EndDate FROM AttendancePayPeriodsT WHERE EndDate = '%s'", strSqlDate)) {
				MessageBox(FormatString("You have already locked payroll on %s.", FormatDateTimeForInterface(dtLockDate, 0, dtoDate)));
				return;
			}

			CString strMessage = FormatString("Closing the payroll will approve and lock all hours worked and time "
				"off on and prior to %s.\r\n\r\nAre you sure you want to close payroll?", FormatDateTimeForInterface(dtLockDate, 0, dtoDate));
			if(MessageBox(strMessage, "Lock Payroll", MB_YESNO|MB_ICONQUESTION) != IDYES) {
				return;
			}

			// (z.manning, 02/11/2008) - PLID 28885 - Ok, let's go ahead and approve all vacation prior
			// to the date they just selected and then save the payroll date in data.
			ADODB::_RecordsetPtr prsLockPayroll = CreateParamRecordset(
				"SET NOCOUNT ON \r\n"
				"DECLARE @AttendanceIDs TABLE (ID INT NOT NULL PRIMARY KEY) \r\n"
				"INSERT INTO @AttendanceIDs \r\n"
				"SELECT ID FROM AttendanceAppointmentsT \r\n"
				"	WHERE ApprovedBy IS NULL AND Date <= {STRING} \r\n"
				"\r\n"
				"UPDATE AttendanceAppointmentsT SET ApprovedBy = {INT} \r\n"
				"WHERE ID IN (SELECT ID FROM @AttendanceIDs) \r\n"
				"\r\n"
				"INSERT INTO AttendancePayPeriodsT (EndDate) VALUES ({STRING}) \r\n"
				"\r\n"
				"SET NOCOUNT OFF \r\n"
				"SELECT ID FROM @AttendanceIDs \r\n"
				, strSqlDate, GetCurrentUserID(), strSqlDate);

			// (z.manning, 05/19/2008) - PLID 30102 - Delete any associated to-do tasks
			CArray<long,long> arynAttendanceIDs;
			for(; !prsLockPayroll->eof; prsLockPayroll->MoveNext()) {
				arynAttendanceIDs.Add(AdoFldLong(prsLockPayroll, "ID"));
			}
			DeleteAssociatedTodos(arynAttendanceIDs);

			// (z.manning, 02/12/2008) - We potentially changed quite a bit here, so do a full refresh.
			UpdateView();

			// (j.luckoski 2012-07-24 09:00) - PLID 29122 - Audit the payroll locking feature differently from the other auditing function
			// (j.luckoski 2012-08-29 15:16) - PLID 29122 - Used a better description to signify the date it has been locked.
			AuditEvent(-1, "", 
				BeginNewAuditEvent(), aeiAttendancePayLock, -1, 
				"", "Payroll locked on " + strSqlDate, 
				aepMedium, aetChanged); 
		}

	}NxCatchAll("CAttendanceDlg::OnLockPayroll");
}

// (z.manning, 02/12/2008) - PLID 28885 - Sets the date range filter to the current pay period through the current date.
void CAttendanceDlg::OnDateCurrentPayPeriod() 
{
	try
	{
		COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
		dtCurrent.SetDate(dtCurrent.GetYear(), dtCurrent.GetMonth(), dtCurrent.GetDay());
		// (z.manning, 02/12/2008) - We only support this if we're in the same year that's currently loaded.
		if(dtCurrent.GetYear() != m_AttendanceInfo.m_nYear) {
			return;
		}

		// (z.manning, 02/12/2008) - PLID 28885 - Find the first payroll close date before today and then add one day
		COleDateTime dtStart = m_AttendanceInfo.GetFirstPayrollCloseDateBefore(dtCurrent);
		if(dtStart.GetStatus() == COleDateTime::valid) {
			// (z.manning, 02/12/2008) - We found a payroll close date so let's set the start date of the filter
			// to the day after it.
			dtStart += ONE_DAY;
		}
		else {
			// (z.manning, 02/12/2008) - No payroll close date found, so let's just use the 1st of the year.
			dtStart.SetDate(dtCurrent.GetYear(), 1, 1);
		}

		if(dtStart > dtCurrent) {
			// (z.manning, 02/12/2008) - This shouldn't be possible.
			ASSERT(FALSE);
			dtStart = dtCurrent;
		}

		// (a.walling 2008-05-14 12:42) - PLID 27591 - COleVariant no longer necessary
		m_dtpDateFilterStart.SetValue((dtStart));
		m_dtpDateFilterEnd.SetValue((dtCurrent));
		// (z.manning, 02/12/2008) - We need to update selected user IDs because we may be filtering
		// on users with a certain kind of time off in the given time range.
		UpdateSelectedUserIDs();
		LoadAttendanceList(FALSE);

	}NxCatchAll("CAttendanceDlg::OnDateCurrentPayPeriod");
}

// (b.cardillo 2008-06-30 14:03) - PLID 30529 - Changed it to use the new ClosedUp event ordinal and parameter list
void CAttendanceDlg::OnClosedUpDepartmentFilter(LPDISPATCH lpSel) 
{
	try
	{
		// (z.manning, 11/13/2007) - Take focus away from the dropdown when a selection is made to avoid annoying
		// inadvertant scrolling using the mouse wheel.
		CWnd *pwnd = GetDlgItem(IDC_ATTENDANCE_LIST);
		if(pwnd != NULL) {
			pwnd->SetFocus();
		}

	}NxCatchAll("CAttendanceDlg::OnClosedUpDepartmentFilter");
}

	// (b.cardillo 2008-06-30 14:03) - PLID 30529 - Changed it to use the new ClosedUp event ordinal and parameter list
void CAttendanceDlg::OnClosedUpAttendanceUserList(LPDISPATCH lpSel) 
{
	try
	{
		// (z.manning, 11/13/2007) - Take focus away from the dropdown when a selection is made to avoid annoying
		// inadvertant scrolling using the mouse wheel.
		CWnd *pWnd = GetDlgItem(IDC_ATTENDANCE_LIST);
		if(pWnd != NULL) {
			pWnd->SetFocus();
		}

	}NxCatchAll("CAttendanceDlg::OnClosedUpAttendanceUserList");
}

void CAttendanceDlg::OnKillfocusAttendanceYear() 
{
	try
	{
		int nYear = GetDlgItemInt(IDC_ATTENDANCE_YEAR);
		if(nYear < MIN_YEAR || nYear > MAX_YEAR) {
			SetDlgItemInt(IDC_ATTENDANCE_YEAR, m_AttendanceInfo.m_nYear);
			return;
		}

		if(nYear != m_AttendanceInfo.m_nYear) {
			EnsureValidDateFilter();
			UpdateView();
		}

	}NxCatchAll("CAttendanceDlg::OnKillfocusAttendanceYear");
}

// (z.manning 2008-12-04 11:10) - PLID 32333
void CAttendanceDlg::OnMoveNextUser()
{
	try
	{
		IRowSettingsPtr pCurSel = m_pdlUsers->GetCurSel();
		if(pCurSel == NULL) {
			return;
		}
		IRowSettingsPtr pNextRow;
		for(pNextRow = pCurSel->GetNextRow(); pNextRow != NULL && !pNextRow->GetVisible(); pNextRow = pNextRow->GetNextRow());
		if(pNextRow != NULL) {
			m_pdlUsers->PutCurSel(pNextRow);
			OnSelChosenAttendanceUserList(pNextRow);
		}

	}NxCatchAll("CAttendanceDlg::OnMoveNextUser");
}

// (z.manning 2008-12-04 11:10) - PLID 32333
void CAttendanceDlg::OnMovePreviousUser()
{
	try
	{
		IRowSettingsPtr pCurSel = m_pdlUsers->GetCurSel();
		if(pCurSel == NULL) {
			return;
		}
		IRowSettingsPtr pPrevRow;
		for(pPrevRow = pCurSel->GetPreviousRow(); pPrevRow != NULL && !pPrevRow->GetVisible(); pPrevRow = pPrevRow->GetPreviousRow());
		if(pPrevRow != NULL) {
			m_pdlUsers->PutCurSel(pPrevRow);
			OnSelChosenAttendanceUserList(pPrevRow);
		}

	}NxCatchAll("CAttendanceDlg::OnMovePreviousUser");
}

// (z.manning 2010-09-03 17:38) - PLID 40046
void CAttendanceDlg::OnBnClickedAttendanceTodayBtn()
{
	try
	{
		// (z.manning 2010-09-03 17:40) - PLID 40046 - Assume the currently loaded year is the current year
		// ensure that the current date is visible.
		COleDateTime dtCurrent = COleDateTime::GetCurrentTime();
		if(dtCurrent.GetYear() == m_AttendanceInfo.m_nYear) {
			EnsureDateVisible(dtCurrent);
		}

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-09-09 12:14) - PLID 40456
int CAttendanceDlg::OpenAttendanceEntryDlg(AttendanceData *pData, BOOL bRefreshOnOk /* = TRUE */, OUT AttendanceData *pAttendanceEntryData /* = NULL */)
{
	CAttendanceEntryDlg dlg(&m_AttendanceInfo, this);
	int nResult = dlg.DoModal(pData);
	pAttendanceEntryData = dlg.GetAttendanceData();
	if(bRefreshOnOk && nResult == IDOK) {
		RefreshAttendanceInfoByUser(pData->pUser->m_nUserID);
	}

	return nResult;
}
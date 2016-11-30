// ApptReminderScheduleExceptionDlg.cpp : implementation file
// (r.gonet 2010-06-23) - PLID 40575 - Created. All comments, unless marked otherwise, are for this PLID.

#include "stdafx.h"
#include "PracticeRc.h"
#include "ApptReminderScheduleExceptionDlg.h"
#include "InternationalUtils.h"


// CApptReminderScheduleExceptionDlg dialog

IMPLEMENT_DYNAMIC(CApptReminderScheduleExceptionDlg, CNxDialog)

CApptReminderScheduleExceptionDlg::CApptReminderScheduleExceptionDlg(CApptReminderException &areException, CWnd* pParent /*=NULL*/)
	: m_areException(areException), CNxDialog(CApptReminderScheduleExceptionDlg::IDD, pParent)
{
}

// Implementation


void CApptReminderScheduleExceptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_APPT_RMND_EX_DATE_RANGE, m_chbDateRange);
	DDX_Control(pDX, IDC_APPT_RMND_EX_TIME_RANGE, m_chbTimeRange);
	DDX_Control(pDX, IDC_APPTREMINDER_EX_DAY_CHECK, m_chbWeekDays);
	DDX_Control(pDX, IDC_APPT_RMND_EX_START_DATE, m_dtStartDate);
	DDX_Control(pDX, IDC_APPT_RMND_EX_END_DATE, m_dtEndDate);
	DDX_Control(pDX, IDC_APPT_RMND_EX_SUNDAY, m_chbSunday);
	DDX_Control(pDX, IDC_APPT_RMND_EX_MONDAY, m_chbMonday);
	DDX_Control(pDX, IDC_APPT_RMND_EX_TUESDAY, m_chbTuesday);
	DDX_Control(pDX, IDC_APPT_RMND_EX_WEDNESDAY, m_chbWednesday);
	DDX_Control(pDX, IDC_APPT_RMND_EX_THURSDAY, m_chbThursday);
	DDX_Control(pDX, IDC_APPT_RMND_EX_FRIDAY, m_chbFriday);
	DDX_Control(pDX, IDC_APPT_RMND_EX_SATURDAY, m_chbSaturday);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_APPTREMINDER_EX_COLOR, m_nxcolorException);
	DDX_Control(pDX, IDC_APPT_RMND_EX_HEADER, m_nxlblHeader);
	DDX_Control(pDX, IDC_APPT_RMND_EX_START_DATE_LABEL, m_nxlblDateStart);
	DDX_Control(pDX, IDC_APPT_RMND_EX_END_DATE_LABEL, m_nxlblDateEnd);
	DDX_Control(pDX, IDC_APPT_RMND_EX_START_TIME_LABEL, m_nxlblTimeStart);
	DDX_Control(pDX, IDC_APPT_RMND_EX_END_TIME_LABEL, m_nxlblTimeEnd);
}


BEGIN_MESSAGE_MAP(CApptReminderScheduleExceptionDlg, CNxDialog)
	ON_BN_CLICKED(IDC_APPT_RMND_EX_DATE_RANGE, &CApptReminderScheduleExceptionDlg::OnBnClickedApptRmndExDateRange)
	ON_BN_CLICKED(IDC_APPT_RMND_EX_TIME_RANGE, &CApptReminderScheduleExceptionDlg::OnBnClickedApptRmndExTimeRange)
	ON_BN_CLICKED(IDOK, &CApptReminderScheduleExceptionDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_APPTREMINDER_EX_DAY_CHECK, &CApptReminderScheduleExceptionDlg::OnBnClickedApptreminderExDayCheck)
END_MESSAGE_MAP()


// CApptReminderScheduleExceptionDlg message handlers

extern CPracticeApp theApp;

BOOL CApptReminderScheduleExceptionDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_nxcolorException.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_dtStartTime = BindNxTimeCtrl(this, IDC_APPTREMINDER_EX_STARTTIME);
		m_dtEndTime = BindNxTimeCtrl(this, IDC_APPTREMINDER_EX_ENDTIME);
		
		// Fill in the checkboxes from our exception
		LoadControls();
		UpdateControls();
	}
	NxCatchAll(__FUNCTION__);

	// Return FALSE because we set the focus, we don't want to framework to set it to something else
	return FALSE;
}

// Loads the controls with values from the exception
void CApptReminderScheduleExceptionDlg::LoadControls()
{
	// Date range
	m_chbDateRange.SetCheck(m_areException.GetStartDate().GetStatus() == COleDateTime::valid);
	if(m_chbDateRange.GetCheck()) {
		m_dtStartDate.SetValue(m_areException.GetStartDate());
		m_dtEndDate.SetValue(m_areException.GetEndDate());
	}
	// Time range
	m_chbTimeRange.SetCheck(m_areException.GetStartTime().GetStatus() == COleDateTime::valid);
	if(m_chbTimeRange.GetCheck()) {
		COleDateTime tmp = m_areException.GetStartTime();
		COleDateTime dtStart;
		dtStart.SetTime(tmp.GetHour(), tmp.GetMinute(), tmp.GetSecond());
		tmp = m_areException.GetEndTime();
		COleDateTime dtEnd;
		dtEnd.SetTime(tmp.GetHour(), tmp.GetMinute(), tmp.GetSecond());
		m_dtStartTime->SetDateTime(dtStart);
		m_dtEndTime->SetDateTime(dtEnd);
	} else {
		COleDateTime tmpStart, tmpEnd;
		tmpStart.SetTime(21, 0, 0);
		tmpEnd.SetTime(9, 0, 0);
		m_dtStartTime->SetDateTime(tmpStart);
		m_dtEndTime->SetDateTime(tmpEnd);
	}

	// Weekdays
	m_chbWeekDays.SetCheck(m_areException.GetFilterOnWeekDays());
	if(m_chbWeekDays.GetCheck()) {
		m_chbSunday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowSunday));
		m_chbMonday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowMonday));
		m_chbTuesday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowTuesday));
		m_chbWednesday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowWednesday));
		m_chbThursday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowThursday));
		m_chbFriday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowFriday));
		m_chbSaturday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowSaturday));
	}
}

// Put all the controls in a valid state
void CApptReminderScheduleExceptionDlg::UpdateControls()
{
	BOOL bDateRangeChecked = (m_chbDateRange.GetCheck() ? TRUE : FALSE);
	EnableDlgItem(IDC_APPT_RMND_EX_START_DATE_LABEL, bDateRangeChecked);
	EnableDlgItem(IDC_APPT_RMND_EX_END_DATE_LABEL, bDateRangeChecked);
	m_dtStartDate.EnableWindow(bDateRangeChecked);
	m_dtEndDate.EnableWindow(bDateRangeChecked);

	BOOL bTimeRangeChecked = (m_chbTimeRange.GetCheck() ? TRUE : FALSE);
	EnableDlgItem(IDC_APPT_RMND_EX_START_TIME_LABEL, bTimeRangeChecked);
	EnableDlgItem(IDC_APPT_RMND_EX_END_TIME_LABEL, bTimeRangeChecked);
	m_dtStartTime->Enabled = bTimeRangeChecked;
	m_dtEndTime->Enabled = bTimeRangeChecked;

	BOOL bWeekDaysChecked = (m_chbWeekDays.GetCheck() ? TRUE : FALSE);
	EnableDlgItem(IDC_APPT_RMND_EX_SUNDAY, bWeekDaysChecked);
	EnableDlgItem(IDC_APPT_RMND_EX_MONDAY, bWeekDaysChecked);
	EnableDlgItem(IDC_APPT_RMND_EX_TUESDAY, bWeekDaysChecked);
	EnableDlgItem(IDC_APPT_RMND_EX_WEDNESDAY, bWeekDaysChecked);
	EnableDlgItem(IDC_APPT_RMND_EX_THURSDAY, bWeekDaysChecked);
	EnableDlgItem(IDC_APPT_RMND_EX_FRIDAY, bWeekDaysChecked);
	EnableDlgItem(IDC_APPT_RMND_EX_SATURDAY, bWeekDaysChecked);
}

void CApptReminderScheduleExceptionDlg::OnBnClickedApptRmndExDateRange()
{
	try {
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

void CApptReminderScheduleExceptionDlg::OnBnClickedApptRmndExTimeRange()
{
	try {
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

// Validate the rule then close the editor
void CApptReminderScheduleExceptionDlg::OnBnClickedOk()
{
	try {
		if(!ValidateException()) {
			return;
		}
		Save();
	} NxCatchAll(__FUNCTION__);
	CNxDialog::OnOK();
}

// Save the controls' values to the exception object
void CApptReminderScheduleExceptionDlg::Save()
{	
	COleDateTime dtNull;
	dtNull.SetStatus(COleDateTime::null);

	// Swap the controls' values into the exception object
	if(m_chbDateRange.GetCheck()) {
		m_areException.SetStartDate(VarDateTime(m_dtStartDate.GetValue()));
		m_areException.SetEndDate(VarDateTime(m_dtEndDate.GetValue()));
	} else {
		m_areException.SetStartDate(dtNull);
		m_areException.SetEndDate(dtNull);
	}

	if(m_chbTimeRange.GetCheck()) {
		m_areException.SetStartTime(m_dtStartTime->GetDateTime());
		m_areException.SetEndTime(m_dtEndTime->GetDateTime());
	} else {
		m_areException.SetStartTime(dtNull);
		m_areException.SetEndTime(dtNull);
	}

	if(m_chbWeekDays.GetCheck()) {
		m_areException.SetFilterOnWeekDays(TRUE);
		m_areException.SetWeekday(CApptReminderException::dowSunday, m_chbSunday.GetCheck());
		m_areException.SetWeekday(CApptReminderException::dowMonday, m_chbMonday.GetCheck());
		m_areException.SetWeekday(CApptReminderException::dowTuesday, m_chbTuesday.GetCheck());
		m_areException.SetWeekday(CApptReminderException::dowWednesday, m_chbWednesday.GetCheck());
		m_areException.SetWeekday(CApptReminderException::dowThursday, m_chbThursday.GetCheck());
		m_areException.SetWeekday(CApptReminderException::dowFriday, m_chbFriday.GetCheck());
		m_areException.SetWeekday(CApptReminderException::dowSaturday, m_chbSaturday.GetCheck());
	} else {
		m_areException.SetFilterOnWeekDays(FALSE);
	}
}

// Go through all of the controls and validate the user's input. Returns true on valid exception. False otherwise.
BOOL CApptReminderScheduleExceptionDlg::ValidateException()
{
	// Check for no input at all
	if(!m_chbDateRange.GetCheck() && !m_chbTimeRange.GetCheck() &&
		(!m_chbWeekDays.GetCheck() || ( m_chbWeekDays.GetCheck() && !m_chbSunday.GetCheck() && !m_chbMonday.GetCheck() && !m_chbTuesday.GetCheck() && !m_chbWednesday.GetCheck() &&
		!m_chbThursday.GetCheck() && !m_chbFriday.GetCheck() && !m_chbSaturday.GetCheck()))) {
		MessageBox("You need to make at least one selection before saving the exception.", "Error", MB_ICONHAND);
		return FALSE;
	}

	// Check for invalid date ranges
	if(m_chbDateRange.GetCheck() && VarDateTime(m_dtStartDate.GetValue()) > VarDateTime(m_dtEndDate.GetValue())) {
		MessageBox("The start date cannot be later than the end date.", "Error", MB_ICONHAND);
		return FALSE;
	}

	// Check if the user did not specify any days. This is a wierd case and we would like to warn them of it.
	if(m_chbWeekDays.GetCheck() && !m_chbSunday.GetCheck() && !m_chbMonday.GetCheck() && !m_chbTuesday.GetCheck() && !m_chbWednesday.GetCheck() &&
		!m_chbThursday.GetCheck() && !m_chbFriday.GetCheck() && !m_chbSaturday.GetCheck()) {
		MessageBox("At least one of the seven days in the week must be selected or week day filtering should be unchecked.", "Error", MB_ICONHAND);
		return FALSE;
	}

	return TRUE;
}

// Toggle day filtering
void CApptReminderScheduleExceptionDlg::OnBnClickedApptreminderExDayCheck()
{
	try
	{
		UpdateControls();
	}NxCatchAll(__FUNCTION__);
}

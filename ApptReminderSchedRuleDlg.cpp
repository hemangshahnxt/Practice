// ApptReminderSchedRuleDlg.cpp : implementation file
// (r.gonet 2010-06-23) - PLID 39012 - Created. All comments, unless marked otherwise, are for this PLID.

#include "stdafx.h"
#include "PracticeRc.h"
#include "ApptReminderSchedRuleDlg.h"


// CApptReminderSchedRuleDlg dialog

IMPLEMENT_DYNAMIC(CApptReminderSchedRuleDlg, CDialog)

CApptReminderSchedRuleDlg::CApptReminderSchedRuleDlg(CApptReminderException &areException, CWnd* pParent /*=NULL*/)
	: m_areException(areException), CDialog(CApptReminderSchedRuleDlg::IDD, pParent)
{
}

CApptReminderSchedRuleDlg::~CApptReminderSchedRuleDlg()
{
}

// Implementation



void CApptReminderSchedRuleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_APPT_RMND_EX_DATE_RANGE, m_chbDateRange);
	DDX_Control(pDX, IDC_APPT_RMND_EX_TIME_RANGE, m_chbTimeRange);
	DDX_Control(pDX, IDC_APPT_RMND_EX_START_DATE, m_dtStartDate);
	DDX_Control(pDX, IDC_APPT_RMND_EX_END_DATE, m_dtEndDate);
	DDX_Control(pDX, IDC_APPT_RMND_EX_START_TIME, m_dtStartTime);
	DDX_Control(pDX, IDC_APPT_RMND_EX_END_TIME, m_dtEndTime);
	DDX_Control(pDX, IDC_APPT_RMND_EX_SUNDAY, m_chbSunday);
	DDX_Control(pDX, IDC_APPT_RMND_EX_MONDAY, m_chbMonday);
	DDX_Control(pDX, IDC_APPT_RMND_EX_TUESDAY, m_chbTuesday);
	DDX_Control(pDX, IDC_APPT_RMND_EX_WEDNESDAY, m_chbWednesday);
	DDX_Control(pDX, IDC_APPT_RMND_EX_THURSDAY, m_chbThursday);
	DDX_Control(pDX, IDC_APPT_RMND_EX_FRIDAY, m_chbFriday);
	DDX_Control(pDX, IDC_APPT_RMND_EX_SATURDAY, m_chbSaturday);
}


BEGIN_MESSAGE_MAP(CApptReminderSchedRuleDlg, CDialog)
	ON_BN_CLICKED(IDC_APPT_RMND_EX_DATE_RANGE, &CApptReminderSchedRuleDlg::OnBnClickedApptRmndExDateRange)
	ON_BN_CLICKED(IDC_APPT_RMND_EX_TIME_RANGE, &CApptReminderSchedRuleDlg::OnBnClickedApptRmndExTimeRange)
	ON_BN_CLICKED(IDOK, &CApptReminderSchedRuleDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CApptReminderSchedRuleDlg message handlers

extern CPracticeApp theApp;

BOOL CApptReminderSchedRuleDlg::OnInitDialog() 
{
	try {
		CDialog::OnInitDialog();
		
		// Fill in the checkboxes from our exception
		LoadControls();
		UpdateControls();
	}
	NxCatchAll(__FUNCTION__);

	// Return FALSE because we set the focus, we don't want to framework to set it to something else
	return FALSE;
}

void CApptReminderSchedRuleDlg::LoadControls()
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
		m_dtStartTime.SetValue(m_areException.GetStartTime());
		m_dtEndTime.SetValue(m_areException.GetEndTime());
	}
	// Weekdays
	m_chbSunday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowSunday));
	m_chbMonday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowMonday));
	m_chbTuesday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowTuesday));
	m_chbWednesday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowWednesday));
	m_chbThursday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowThursday));
	m_chbFriday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowFriday));
	m_chbSaturday.SetCheck(m_areException.GetWeekday(CApptReminderException::dowSaturday));
}

void CApptReminderSchedRuleDlg::UpdateControls()
{
	if(m_chbDateRange.GetCheck()) {
		m_dtStartDate.EnableWindow(TRUE);
		m_dtEndDate.EnableWindow(TRUE);
	} else {
		m_dtStartDate.EnableWindow(FALSE);
		m_dtEndDate.EnableWindow(FALSE);
	}

	if(m_chbTimeRange.GetCheck()) {
		m_dtStartTime.EnableWindow(TRUE);
		m_dtEndTime.EnableWindow(TRUE);
	} else {
		m_dtStartTime.EnableWindow(FALSE);
		m_dtEndTime.EnableWindow(FALSE);
	}
}
void CApptReminderSchedRuleDlg::OnBnClickedApptRmndExDateRange()
{
	try {
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

void CApptReminderSchedRuleDlg::OnBnClickedApptRmndExTimeRange()
{
	try {
		UpdateControls();
	} NxCatchAll(__FUNCTION__);
}

void CApptReminderSchedRuleDlg::OnBnClickedOk()
{
	try {
		if(!ValidateException()) {
			return;
		}
		
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
			m_areException.SetStartTime(VarDateTime(m_dtStartTime.GetValue()));
			m_areException.SetEndTime(VarDateTime(m_dtEndTime.GetValue()));
		} else {
			m_areException.SetStartTime(dtNull);
			m_areException.SetEndTime(dtNull);
		}

		m_areException.SetWeekday(CApptReminderException::dowSunday, m_chbSunday.GetCheck());
		m_areException.SetWeekday(CApptReminderException::dowMonday, m_chbMonday.GetCheck());
		m_areException.SetWeekday(CApptReminderException::dowTuesday, m_chbTuesday.GetCheck());
		m_areException.SetWeekday(CApptReminderException::dowWednesday, m_chbWednesday.GetCheck());
		m_areException.SetWeekday(CApptReminderException::dowThursday, m_chbThursday.GetCheck());
		m_areException.SetWeekday(CApptReminderException::dowFriday, m_chbFriday.GetCheck());
		m_areException.SetWeekday(CApptReminderException::dowSaturday, m_chbSaturday.GetCheck());
	} NxCatchAll(__FUNCTION__);
	OnOK();
}

BOOL CApptReminderSchedRuleDlg::ValidateException()
{
	// Go through all of the controls and validate the user's input
	
	// Check for no input at all
	if(!m_chbDateRange.GetCheck() && !m_chbTimeRange.GetCheck() &&
		!m_chbSunday.GetCheck() && !m_chbMonday.GetCheck() && !m_chbTuesday.GetCheck() && !m_chbWednesday.GetCheck() &&
		!m_chbThursday.GetCheck() && !m_chbFriday.GetCheck() && !m_chbSaturday.GetCheck()) {
		MessageBox("You need to make at least one selection before saving the exception.", "Error", MB_ICONEXCLAMATION);
		return FALSE;
	}

	// Check for invalid date ranges
	if(m_chbDateRange.GetCheck() && VarDateTime(m_dtStartDate.GetValue()) > VarDateTime(m_dtEndDate.GetValue())) {
		MessageBox("The start date cannot be later than the end date.", "Error", MB_ICONEXCLAMATION);
		return FALSE;
	}

	// Check for invalid time ranges. This is iffy here, what if the user really does mean 6pm - 6am: Do not send at night.
	if(m_chbTimeRange.GetCheck() && VarDateTime(m_dtStartTime.GetValue()) > VarDateTime(m_dtEndTime.GetValue())) {
		MessageBox("The start time cannot be after the end time.", "Error", MB_ICONEXCLAMATION);
		return FALSE;
	}

	return TRUE;
}
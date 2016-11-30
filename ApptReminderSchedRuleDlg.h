#pragma once

#include "Practice.h"
#include "ApptReminderSchedule.h"

// CApptReminderSchedRuleDlg dialog
// (r.gonet 2010-06-23) - PLID 39012 - Created. All comments, unless marked otherwise, are for this PLID.

class CApptReminderSchedRuleDlg : public CDialog
{
	DECLARE_DYNAMIC(CApptReminderSchedRuleDlg)

private:
	// Fields
	CApptReminderException &m_areException;

	// Controls
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	NxButton m_chbDateRange;
	NxButton m_chbTimeRange;
	NxButton m_chbWeekDays;
	CDateTimePicker m_dtStartDate;
	CDateTimePicker m_dtEndDate;
	CDateTimePicker m_dtStartTime;
	CDateTimePicker m_dtEndTime;
	NxButton m_chbSunday;
	NxButton m_chbMonday;
	NxButton m_chbTuesday;
	NxButton m_chbWednesday;
	NxButton m_chbThursday;
	NxButton m_chbFriday;
	NxButton m_chbSaturday;
public:
	CApptReminderSchedRuleDlg(CApptReminderException &areException, CWnd* pParent = NULL);   // standard constructor
	virtual ~CApptReminderSchedRuleDlg();

// Dialog Data
	enum { IDD = IDD_APPT_REMINDER_SCHEDULE_EXCEPTION_DLG };

private:
	// Internal Implementation
	void LoadControls();
	void UpdateControls();
	BOOL ValidateException();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedApptRmndExDateRange();
	afx_msg void OnBnClickedApptRmndExTimeRange();
	afx_msg void OnBnClickedOk();
};

#pragma once

#include "Practice.h"
#include "ApptReminderSchedule.h"

// CApptReminderScheduleExceptionDlg dialog
// (r.gonet 2010-06-23) - PLID 40575 - Created. All comments, unless marked otherwise, are for this PLID.

class CApptReminderScheduleExceptionDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CApptReminderScheduleExceptionDlg)

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
	// Switched the tims to be Time controls rather than DateTimePickers
	NXTIMELib::_DNxTimePtr m_dtStartTime;
	NXTIMELib::_DNxTimePtr m_dtEndTime;
	NxButton m_chbSunday;
	NxButton m_chbMonday;
	NxButton m_chbTuesday;
	NxButton m_chbWednesday;
	NxButton m_chbThursday;
	NxButton m_chbFriday;
	NxButton m_chbSaturday;
	// Added some NexTech controls for transparency issues with the new nxcolor control
	CNxColor m_nxcolorException;
	CNxStatic m_nxlblHeader;
	CNxStatic m_nxlblDateStart;
	CNxStatic m_nxlblDateEnd;
	CNxStatic m_nxlblTimeStart;
	CNxStatic m_nxlblTimeEnd;

public:
	CApptReminderScheduleExceptionDlg(CApptReminderException &areException, CWnd* pParent);   // standard constructor

// Dialog Data
	enum { IDD = IDD_APPT_REMINDER_SCHEDULE_EXCEPTION_DLG };

private:
	// Internal Implementation
	void Save();
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
	afx_msg void OnBnClickedApptRmndExSunday();
	afx_msg void OnBnClickedApptreminderExDayCheck();
};

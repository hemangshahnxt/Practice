#pragma once

#include "PracticeRc.h"
#include "ApptReminderSchedule.h"

// CAppointmentReminderSetupDlg dialog
// (c.haag 2010-03-25 11:54) - PLID 37891 - Initial implementation

class CAppointmentReminderSetupDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAppointmentReminderSetupDlg)

private:
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnEditFormat;
	NxButton m_radioOn;
	NxButton m_radioOff;
	CNxEdit m_editNotificationFormat;
	CString m_strMsgFormat;
	CDateTimePicker m_dtStartDate;
	CNxColor m_nxcolorBackground;
	NxButton m_grp1, m_grp3;
	CNxStatic m_nxstaticFormatLength; // (z.manning 2010-05-04 14:49) - PLID 38492
	// (r.gonet 2010-06-23) - PLID 39281 - We now let the user configure the country code to send SMS with. Default is 1 for USA.
	CString m_strCountryCode;

	// (r.gonet 2010-06-25) - PLID 39012 - Map the open schedule dialog button to an object
	CNxIconButton m_btnApptReminderSchedule;

	// (r.gonet 2010-06-25) - PLID 39012 - Hold the schedule as an encapsulated object. We load this, pass it to the schedule editor, 
	//  and then save this to the database after it is filled by the schedule editor.
	CApptReminderSchedule m_arsSchedule;

	// (r.gonet 01-11-2011) - PLID 42010
	CNxIconButton m_btnPolicy;

	// (r.gonet 01-13-2011) - PLID 42010 - Keep track of whether NexReminder is enabled
	BOOL m_bNexReminderEnabled;
	// (r.gonet 01-13-2011) - PLID 42010 - Keep track of whether the policy dialog was shown and completed by the user
	BOOL m_bPolicyDlgCompleted;

public:
	CAppointmentReminderSetupDlg(CWnd* pParent);   // standard constructor
	virtual ~CAppointmentReminderSetupDlg();

// Dialog Data
	enum { IDD = IDD_APPOINTMENT_REMINDER_SETUP_DLG };

	// (z.manning 2011-11-02 10:22) - PLID 46220 - Simple struc to keep track of advanced settings
	struct AdvancedSettings
	{
		CString strLeadingDigitToTrim;
	};
	AdvancedSettings m_AdvancedSettings;

private:
	// Validates the data content in this dialog
	BOOL ValidateData();
	// Saves the data content
	void SaveData();

	void UpdateMessageFormatLength(); // (z.manning 2010-05-04 14:50) - PLID 38492

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	void UpdateControls();

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRadioOn();
	afx_msg void OnBnClickedRadioOff();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnArEditFormat();
	afx_msg void OnEnChangeEditArFormat();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	// (r.gonet 2010-06-23) - PLID 39281 - Open the reminder schedule edit dialog to edit our loaded schedule
	afx_msg void OnBnClickedBtnApptReminderSched();
	afx_msg void OnBnClickedNxreminderConfirmCheck(); // (z.manning 2010-11-17 17:53) - PLID 41495
	// (r.gonet 01-11-2011) - PLID 42010 - Let the user view the policy at any time
	afx_msg void OnBnClickedNxreminderTxtMsgPolicyBtn();
	afx_msg void OnBnClickedNxreminderAdvanced(); // (z.manning 2011-11-02 09:49) - PLID 46220
};

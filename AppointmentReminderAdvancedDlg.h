#pragma once


#include "AppointmentReminderSetupDlg.h"


// CAppointmentReminderAdvancedDlg dialog
// (z.manning 2011-11-02 09:58) - PLID 46220 - Created

class CAppointmentReminderAdvancedDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAppointmentReminderAdvancedDlg)

public:
	CAppointmentReminderAdvancedDlg(CAppointmentReminderSetupDlg::AdvancedSettings *pSettings, CWnd* pParent);   // standard constructor
	virtual ~CAppointmentReminderAdvancedDlg();

// Dialog Data
	enum { IDD = IDD_APPT_REMINDER_ADVANCED };

protected:
	CAppointmentReminderSetupDlg::AdvancedSettings *m_pSettings;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void OnOK();

	DECLARE_MESSAGE_MAP()

	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	CNxColor m_nxcolor;
};

// (r.gonet 2010-06-23) - PLID 40574 - Created. All comments, unless marked otherwise, are for this PLID.

#pragma once

#include "ApptReminderScheduleRule.h"

// CApptReminderScheduleRuleDlg dialog
// Changed all references to CNxDialog for transparency requirements.
class CApptReminderScheduleRuleDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CApptReminderScheduleRuleDlg)

private:
	CApptReminderScheduleRule &m_arrRule;
	NxButton m_chbIgnoreExceptions;
	// (r.gonet 2010-09-03) - PLID 39282 - Add filters for type and resource
	CArray<long, long> m_arynAppointmentTypes;
	CArray<long, long> m_arynAppointmentResources;
	NxButton m_chbFilterOnTypes;
	NxButton m_chbFilterOnResources;
	CNxIconButton m_btnEditTypes;
	CNxIconButton m_btnEditResources;
	CNxEdit m_nxeditTypes;
	CNxEdit m_nxeditResources;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	// (r.gonet 2010-12-13) - PLID 39012 - Added in an NxColor control and limited the length of the days and hours.
	CNxColor m_nxcolorRule;
	CNxEdit m_nxeditDays;
	CNxEdit m_nxeditHours;

public:
	CApptReminderScheduleRuleDlg(CApptReminderScheduleRule &arrRule, CWnd* pParent);   // standard constructor
	virtual ~CApptReminderScheduleRuleDlg();

// Dialog Data
	enum { IDD = IDD_APPT_REMINDER_SCHEDULE_RULE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	NXTIMELib::_DNxTimePtr m_nxtSendTime; // (z.manning 2011-05-16 10:23) - PLID 43708
	COleDateTime m_dtOriginalSendTime;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
private:
	void Save();
	void LoadControls();
	void UpdateControls();
	BOOL ValidateRule();
	// (r.gonet 2010-09-03) - PLID 39282 - Load associated appointment types and resources
	void LoadAppointmentTypes();
	void LoadAppointmentResources();
	
	afx_msg void OnEnKillfocusApptRmndrSchedRuleHours();
	afx_msg void OnBnClickedEditApptTypes();
	afx_msg void OnBnClickedEditApptResources();
	afx_msg void OnBnClickedFilterOnApptTypes();
	afx_msg void OnBnClickedFilterOnApptResources();
	afx_msg void OnBnClickedApptReminderBeforeRadio();
	afx_msg void OnBnClickedApptReminderSpecificTimeRadio();
};

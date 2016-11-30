#pragma once

#include "Practice.h"
#include "ApptReminderSchedule.h"
#include "ApptReminderScheduleRule.h"


// CApptReminderSchedDlg dialog
// (r.gonet 2010-06-23) - PLID 39012 - Created. All comments, unless marked otherwise, are for this PLID.

class CApptReminderSchedDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CApptReminderSchedDlg)

private:
	// Datalist enumerations
	enum ERulesList
	{
		rlID,
		rlDescription,
		rlIgnoreExceptions
	};

	enum EExceptionsList
	{
		elID,
		elDescription
	};

	// Fields
	CNxColor m_nxcolorRules;
	CNxColor m_nxcolorExceptions;
	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Added sending limit nxcolor
	CNxColor m_nxcolorSendingLimits;
	CNxIconButton m_btnAddRule;
	CNxIconButton m_btnEditRule;
	CNxIconButton m_btnRemoveRule;
	CNxIconButton m_btnAddException;
	CNxIconButton m_btnEditException;
	CNxIconButton m_btnRemoveException;
	CNxIconButton m_btnClose;
	CNxStatic m_nxlblHeader;
	CNxStatic m_nxlblRulesHeader;
	CNxStatic m_nxlblExceptionsHeader;
	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Added sending limit controls.
	CNxStatic m_nxlblSendingLimitsHeader;
	NxButton m_radioPerAppointmentPerRule;
	NxButton m_radioPerDayPatientAppointmentDate;
	NxButton m_checkAndResource;
	NxButton m_checkAndLocation;

	CApptReminderSchedule &m_arsSchedule;
	NXDATALIST2Lib::_DNxDataListPtr m_pRules;
	NXDATALIST2Lib::_DNxDataListPtr m_pExceptions;
public:
	CApptReminderSchedDlg(CApptReminderSchedule &arsSchedule, CWnd* pParent);   // standard constructor
	virtual ~CApptReminderSchedDlg();
private:
	// Private implementation
	void ReloadRulesList();
	void ReloadExceptionsList();
	void UpdateControls();

// Dialog Data
	enum { IDD = IDD_APPT_REMINDER_SCHED_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedApptReminderAddExceptionBtn();
	afx_msg void OnBnClickedApptReminderEditExceptionBtn();
	afx_msg void OnBnClickedApptReminderRemoveExceptionBtn();
	afx_msg void OnBnClickedAddApptSchedRuleBtn();
	afx_msg void OnBnClickedApptRmndSchedEditRule();
	afx_msg void OnBnClickedRemoveApptSchedRuleBtn();
	DECLARE_EVENTSINK_MAP()
	void EditingFinishedApptReminderSchedRulesDl(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedApptRmndSchedRuleGb();
	void DblClickCellApptReminderSchedRulesDl(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellApptReminderSchedExceptionsDl(LPDISPATCH lpRow, short nColIndex);
	void SelChangedApptReminderSchedRulesDl(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void SelChangedApptReminderSchedExceptionsDl(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnBnClickedRuleshelpbtn();
	afx_msg void OnBnClickedExceptionshelpbtn();
	// (r.gonet 2015-11-16 02:33) - PLID 67587 - Sending limit radio/checkbox handlers.
	afx_msg void OnBnClickedApptRmndrSchedSendingLimitsOpt1Radio();
	afx_msg void OnBnClickedApptRmndrSchedSendingLimitsOpt2Radio();
	afx_msg void OnBnClickedApptRmndrSchedSendingLimitsResourceCheck();
	afx_msg void OnBnClickedApptRmndrSchedSendingLimitsLocationCheck();
};

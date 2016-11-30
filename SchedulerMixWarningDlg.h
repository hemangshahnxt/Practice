#pragma once

// (j.jones 2014-11-26 09:31) - PLID 64179 - created

// CSchedulerMixWarningDlg dialog

#include "SchedulerRc.h"

class SchedulerMixRule;

//Possible return values for the dialog. smwrvCancelled intentionally shares the same value as IDCANCEL.
enum SchedulerMixWarningReturnValue {

	smwrvCancelled = 2,	//the user cancelled (or said OK if they have no override permission)
	smwrvOverride = -2,	//the user agreed to override the rule (permissions already checked)
	smwrvFFA = -3,		//the user wanted to use FFA to reschedule the appt.
};

class CSchedulerMixWarningDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CSchedulerMixWarningDlg)

public:
	
	CSchedulerMixWarningDlg(CWnd* pParent = NULL);
	virtual ~CSchedulerMixWarningDlg();

	virtual int DoModal(const std::vector<SchedulerMixRule> &exceededMixRules);

	//tracks the ID & name of the rules that were exceeded
	std::vector<SchedulerMixRule> m_exceededMixRules;

// Dialog Data
	enum { IDD = IDD_SCHEDULER_MIX_WARNING_DLG };
	CNxIconButton	m_btnOverride;
	CNxIconButton	m_btnFFA;
	CNxIconButton	m_btnCancel;
	CNxStatic		m_nxstaticWarningLabel;

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnOverride();
	afx_msg void OnBtnFFA();
	afx_msg void OnBtnCancel();
};

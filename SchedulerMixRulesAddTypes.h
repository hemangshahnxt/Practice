#pragma once
#include "SchedulingMixRulesConfigDLG.h"

// (s.tullis 2014-12-30 14:39) - PLID 64144 - Created

class CSchedulerMixRulesAddTypes : public CNxDialog
{
	DECLARE_DYNAMIC(CSchedulerMixRulesAddTypes)

public:
	CSchedulerMixRulesAddTypes(long nRuleID,long nResourceID, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSchedulerMixRulesAddTypes();
	BOOL OnInitDialog();
	CNxIconButton m_btnSave;
	CNxIconButton m_btnSaveAndAddAnother;
	CNxIconButton m_btnCancel;
	CNxEdit m_editMaxAppt;
	long m_nMaxNumAppt;
	long m_RuleID;
	long m_ResourceID;
	RuleDetail m_ruleDetail;
	NXDATALIST2Lib::_DNxDataListPtr m_pApptType;
	NXDATALIST2Lib::_DNxDataListPtr m_pApptPurpose;
	

	BOOL Save();
// Dialog Data
	enum { IDD = IDD_SCHEDULE_MIX_RULES_ADD_TYPES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeMaxAppt();
	DECLARE_EVENTSINK_MAP()
	void SelChangedRuleApptType(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnBnClickedRuleDetailSaveAddAnother();
	afx_msg void OnBnClickedOk();
};

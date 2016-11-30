#pragma once

//TES 11/18/2014 - PLID 64121 - Created
// CViewSchedulerMixRulesDlg dialog

class CViewSchedulerMixRulesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CViewSchedulerMixRulesDlg)

public:
	CViewSchedulerMixRulesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CViewSchedulerMixRulesDlg();
	//TES 11/18/2014 - PLID 64123, 64124 - Track the original and selected date and resource
	COleDateTime m_dtInitialDate, m_dtNewDate;
	long m_nInitialResourceID, m_nNewResourceID;

// Dialog Data
	enum { IDD = IDD_VIEW_SCHEDULER_MIX_RULES_DLG };

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pResourceList;
	NXDATALIST2Lib::_DNxDataListPtr m_pRuleList; //TES 11/19/2014 - PLID 64122
	CNxIconButton m_btnClose, m_btnDateLeft, m_btnDateRight;
	CDateTimePicker	m_dtpRuleDate;

	void LoadRules();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedGoToSchedule();
	DECLARE_EVENTSINK_MAP()
	void SelChosenRuleResource(LPDISPATCH lpRow);
	afx_msg void OnDtnDatetimechangeRuleDate(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedDateLeft();
	afx_msg void OnBnClickedDateRight();
};

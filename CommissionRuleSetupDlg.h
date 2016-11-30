#if !defined(AFX_COMMISSIONRULESETUPDLG_H__9FAC4EE5_2251_4D71_B9C1_90DE6D8F8DE8__INCLUDED_)
#define AFX_COMMISSIONRULESETUPDLG_H__9FAC4EE5_2251_4D71_B9C1_90DE6D8F8DE8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CommissionRuleSetupDlg.h : header file
//

// (a.wetta 2007-03-27 10:34) - PLID 25327 - Dialog created
#include "ContactsRc.h"

struct CommissionRuleInfo
{
	long nID;
	CString strName;
	CString strOldName;
	COleDateTime dtApplyStart;
	COleDateTime dtOldApplyStart;
	COleDateTime dtApplyEnd;
	COleDateTime dtOldApplyEnd;
	double dblCommissionPercentage;
	double dblOldCommissionPercentage;
	COleDateTime dtBasedOnStart;
	COleDateTime dtOldBasedOnStart;
	COleDateTime dtBasedOnEnd;
	COleDateTime dtOldBasedOnEnd;
	COleCurrency cyBasedOnMoneyThreshold;
	COleCurrency cyOldBasedOnMoneyThreshold;
	// (j.jones 2009-11-18 14:53) - PLID 29046 - supported tiered commissions
	BOOL bOverwritePriorRules;
	BOOL bOldOverwritePriorRules;
	// (b.eyers 2015-05-14) - PLID 65978
	COleCurrency cyCommissionAmount;
	COleCurrency cyOldCommissionAmount;

	CommissionRuleInfo() {
		nID = -1;
		strName = "";
		strOldName = "";
		dtApplyStart = COleDateTime(1899,12,30,0,0,0);
		dtOldApplyStart = COleDateTime(1899,12,30,0,0,0);
		dtApplyEnd = COleDateTime(1899,12,30,0,0,0);
		dtOldApplyEnd = COleDateTime(1899,12,30,0,0,0);
		dblCommissionPercentage = 0;
		dblOldCommissionPercentage = 0;
		dtBasedOnStart = COleDateTime(1899,12,30,0,0,0);
		dtOldBasedOnStart = COleDateTime(1899,12,30,0,0,0);
		dtBasedOnEnd = COleDateTime(1899,12,30,0,0,0);
		dtOldBasedOnEnd = COleDateTime(1899,12,30,0,0,0);
		cyBasedOnMoneyThreshold = COleCurrency(0,0);
		cyOldBasedOnMoneyThreshold = COleCurrency(0,0);
		// (j.jones 2009-11-18 14:53) - PLID 29046 - supported tiered commissions
		bOverwritePriorRules = TRUE;
		bOldOverwritePriorRules = TRUE;
		// (b.eyers 2015-05-14) - PLID 65978
		cyCommissionAmount = COleCurrency(0, 0);
		cyOldCommissionAmount = COleCurrency(0, 0);
	}
};

enum CommissionRuleFields {
	crfID = 0,
	crfName,
	crfOldName,
	crfApplyStart,
	crfOldApplyStart,
	crfApplyEnd,
	crfOldApplyEnd,
	crfBasedOnStart,
	crfOldBasedOnStart,
	crfBasedOnEnd,
	crfOldBasedOnEnd,
	crfCommissionPercentage,
	crfOldCommissionPercentage,
	crfMoneyThreshold,
	crfOldMoneyThreshold,
	// (j.jones 2009-11-19 10:20) - PLID 29046 - support OverwritePriorRules
	crfOverwritePriorRules,
	crfOldOverwritePriorRules,
	// (b.eyers 2015-05-14) - PLID 65978
	crfCommissionAmount,
	crfOldCommissionAmount,
};

/////////////////////////////////////////////////////////////////////////////
// CCommissionRuleSetupDlg dialog

class CCommissionRuleSetupDlg : public CNxDialog
{
// Construction
public:
	CCommissionRuleSetupDlg(CWnd* pParent);   // standard constructor

	// (a.wetta 2007-03-30 10:59) - PLID 24872 - Background color
	DWORD m_backgroundColor;

	// (j.jones 2009-11-18 14:48) - PLID 29046 - m_bUseTieredRules determines
	// whether we show rules where IsTieredCommission = 1, or not
	BOOL m_bUseTieredRules;

	BOOL m_bPercent; // (b.eyers 2015-05-14) - PLID 65978

// Dialog Data
		// (a.wetta 2007-03-29 14:57) - PLID 25407 - Make sure the buttons match in the Retail tab (they must be CNxIconButtons)
	
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	//{{AFX_DATA(CCommissionRuleSetupDlg)
	enum { IDD = IDD_COMMISSION_RULE_SETUP_DLG };
	NxButton	m_btnShowOutdated;
	CNxIconButton	m_btnOK;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnRemoveRule;
	CNxIconButton	m_btnAddRule;
	CDateTimePicker	m_ctrlStartBasedDate;
	CDateTimePicker	m_ctrlStartApplyDate;
	CDateTimePicker	m_ctrlEndBasedDate;
	CDateTimePicker	m_ctrlEndApplyDate;
	CNxEdit	m_editRuleName;
	CNxColor	m_bkgColor2;
	CNxColor	m_bkgColor5;
	CNxEdit	m_nxeditMoneyThreshold;
	CNxEdit	m_nxeditCommissionPercentage;
	// (j.jones 2009-11-18 15:20) - PLID 29046 - supported tiered commissions
	CNxStatic	m_nxstaticAmountSoldLabel;
	CNxStatic	m_nxstaticAdvNoteLabel;
	CNxStatic	m_nxstaticApplyCommissionLabel;
	NxButton	m_radioThisTierOnly;
	NxButton	m_radioAllLowerTiers;
	CNxStatic	m_nxstaticBasedOnDateRangeLabel;
	CNxStatic	m_nxstaticBasedOnDateStartLabel;
	CNxStatic	m_nxstaticBasedOnDateToLabel;
	CNxStatic	m_nxstaticBasedOnDateEndLabel;
	CNxStatic	m_nxstaticApplyDateLabel;
	// (b.eyers 2015-05-14) - PLID 65978
	CNxLabel m_nxlabelCommissionLabel; 
	CNxEdit m_nxeditCommissionAmount; 
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommissionRuleSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pRuleList;

	void UpdateCommissionInfoToScreen(CommissionRuleInfo criRuleInfo);
	CommissionRuleInfo GetCommissionRuleInfoFromRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	CommissionRuleInfo GetCommissionRuleInfoFromScreen();
	//BOOL UpdateDataAndRowFromScreen(NXDATALIST2Lib::IRowSettingsPtr pUpdateRow);
	void UpdateRowWithCommissionInfo(CommissionRuleInfo criRuleInfo, NXDATALIST2Lib::IRowSettingsPtr pUpdateRow);
	void UpdateDataFromCommissionRuleInfo(long lRuleID, CommissionRuleInfo criRuleInfo);
	void EnableEditBoxes(bool bEnable);
	void SelChangedCommissionRuleList(NXDATALIST2Lib::IRowSettingsPtr pOldSel, NXDATALIST2Lib::IRowSettingsPtr pNewSel);
	BOOL ValidateData(CommissionRuleInfo criRuleInfo);
	BOOL UpdateRowFromScreen(NXDATALIST2Lib::IRowSettingsPtr pUpdateRow);

	bool m_bChanged;
	bool m_bUpdating;

	CMap<long, long, CommissionRuleInfo, CommissionRuleInfo> m_mapChangedRules;
	CMap<long, long, CommissionRuleInfo, CommissionRuleInfo> m_mapAddRules;
	CArray<CommissionRuleInfo, CommissionRuleInfo> m_aryDeleteRules;

	LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam); // (b.eyers 2015-05-14) - PLID 65978

	long m_nNewID;

	BOOL Save();

	// (a.wetta 2007-05-18 13:21) - PLID 25394 - Used to undo a change if need be
	CommissionRuleInfo m_criCurrentInfo;

	// Generated message map functions
	// (a.walling 2008-05-14 12:22) - PLID 27591 - Use the new notify events
	//{{AFX_MSG(CCommissionRuleSetupDlg)
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnAddRule();
	afx_msg void OnRemoveRule();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangedCommissionRuleList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnChangeRuleName();
	afx_msg void OnChangeMoneyThreshold(); // (a.vengrofski 2009-12-22 10:39) - PLID <36362> - Added to save the money, from a KillFocus hicup.
	afx_msg void OnChangeCommissionPercentage();// (a.vengrofski 2009-12-22 10:52) - PLID <36364> - Added to save the commission rate, from a KillFocus hicup.
	afx_msg void OnChangeEndApplyDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEndBasedDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeStartApplyDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeStartBasedDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowOutdated();
	afx_msg void OnTrySetSelFinishedCommissionRuleList(long nRowEnum, long nFlags);
	afx_msg void OnKillfocusRuleName();
	afx_msg void OnKillfocusMoneyThreshold();
	afx_msg void OnKillfocusCommissionPercentage();
	afx_msg void OnKillfocusCommissionAmount(); // (b.eyers 2015-05-14) - PLID 65978
	afx_msg void OnChangeCommissionAmount(); // (b.eyers 2015-05-14) - PLID 65978
	// (j.jones 2009-11-19 10:20) - PLID 29046 - supported OverwritePriorRules
	afx_msg void OnRadioThisTierOnly();
	afx_msg void OnRadioAllLowerTiers();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMISSIONRULESETUPDLG_H__9FAC4EE5_2251_4D71_B9C1_90DE6D8F8DE8__INCLUDED_)

#pragma once

#include "AdministratorRc.h"

// SchedulingMixRulesDLG dialog


class CSchedulingMixRulesDLG : public CNxDialog
{
	DECLARE_DYNAMIC(CSchedulingMixRulesDLG)

public:
	CSchedulingMixRulesDLG(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSchedulingMixRulesDLG();
	BOOL OnInitDialog();
	void SetRuleFilters(BOOL bLoad = FALSE);// (s.tullis 2014-12-08 15:53) - PLID 64126 
	BOOL GetStrRuleFromInput(CString &rule);
	void FilterRuleList();// (s.tullis 2014-12-08 15:53) - PLID 64126 
	void SetControls(BOOL onOFF);
// Dialog Data
	enum { IDD = IDD_SCHEDULING_MIX_RULES };

	
	CNxIconButton m_btnAddRule;// (s.tullis 2014-12-08 16:15) - PLID 64129 -
	CNxIconButton m_btnEditRule;// (s.tullis 2014-12-08 16:15) - PLID 64129 -
	CNxIconButton m_btnCopyRule;// (s.tullis 2014-12-08 15:57) - PLID 64127 
	CNxIconButton m_btnDeleteRule;// (s.tullis 2014-12-08 15:57) - PLID 64127 
	CNxIconButton m_btnClose;// (s.tullis 2014-12-08 15:49) - PLID 64125 
	NxButton m_checkHideExpiredRules;
	//NXDATALIST2Lib::_DNxDataListPtr
	NXDATALIST2Lib::_DNxDataListPtr m_pResourceList;// (s.tullis 2014-12-08 15:53) - PLID 64126 
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationList;// (s.tullis 2014-12-08 15:53) - PLID 64126 
	NXDATALIST2Lib::_DNxDataListPtr m_pInsuranceList;// (s.tullis 2014-12-08 15:53) - PLID 64126 
	NXDATALIST2Lib::_DNxDataListPtr m_pRulesList; 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	void SetExpiredRuleRowsToGrey();// (s.tullis 2014-12-08 15:49) - PLID 64125 
	void RButtonDownScheduleRulesList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);// (s.tullis 2014-12-08 15:49) - PLID 64125 
	afx_msg void OnBnClickedAddRule();
	afx_msg void OnBnClickedCopyRule();// (s.tullis 2014-12-08 15:57) - PLID 64127 
	afx_msg void OnBnClickedDeleteRule(); // (s.tullis 2014 - 12 - 08 15:57) - PLID 64127
	void SelChangedScheduleRulesList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void DblClickCellScheduleRulesList(LPDISPATCH lpRow, short nColIndex);
	void DeleteRule( NXDATALIST2Lib::IRowSettingsPtr pRow);// (s.tullis 2014 - 12 - 08 15:57) - PLID 64127
	void RenameRule( NXDATALIST2Lib::IRowSettingsPtr pRow);// (s.tullis 2014 - 12 - 08 15:57) - PLID 64127
	void CopyRule(NXDATALIST2Lib::IRowSettingsPtr pRow);// (s.tullis 2014 - 12 - 08 15:57) - PLID 64127
	void EditRule(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void CreateRule(CString strRuleName);// (s.tullis 2014-12-08 16:15) - PLID 64129 
	afx_msg void OnBnClickedHideExpiredCheck();
	afx_msg void OnBnClickedEditMixRule();// (s.tullis 2014-12-08 16:15) - PLID 64129 
	void RequeryFinishedScheduleRulesList(short nFlags);
	void SelChangingRuleResourceList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingRuleLocationList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingRuleInsuranceList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingScheduleRulesList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChosenRuleInsuranceList(LPDISPATCH lpRow);
	void SelChosenRuleLocationList(LPDISPATCH lpRow);
	void SelChosenRuleResourceList(LPDISPATCH lpRow);
};

#pragma once
#include "commondialog.h"

#include "AdministratorRc.h"

// (s.tullis 2014-12-08 16:48) - PLID 64134 - Created
// SchedulingMixRulesConfigDLG dialog
enum SchedulingMixRulesConfigMode{
	Create= 0,
	Edit,
	Copy,
};

typedef long nResourceID;

struct RuleDetail
{
	long nRuleDetailID;
	long nApptTypeID;
	long nPurposeID;
	long nMaxappts;
	RuleDetail *pDetailNext;

	RuleDetail(long nRuleDID, long nApptType,  long nPurpose, long nMaxappt ){
		nRuleDetailID = nRuleDID;
		nApptTypeID = nApptType;
		nPurposeID = nPurpose;
		nMaxappts = nMaxappt;
		pDetailNext = NULL;
	}

	RuleDetail(){
		 nRuleDetailID=-1;
		 nApptTypeID=-1;
		 nPurposeID= -1;
		 nMaxappts =-1;
		 pDetailNext= NULL;
	}

};

class CSchedulingMixRulesConfigDLG : public CNxDialog
{
	DECLARE_DYNAMIC(CSchedulingMixRulesConfigDLG)

public:
	CSchedulingMixRulesConfigDLG(SchedulingMixRulesConfigMode ConfigMode, CString strRuleName, COleDateTime dtStartDate, COleDateTime dtEndDate ,long nColor=-1, long nRuleID = -1, CWnd* pParent = NULL);   // standard constructor
	virtual ~CSchedulingMixRulesConfigDLG();
	BOOL OnInitDialog();

	NxButton m_checkExpiredDate;// (s.tullis 2014-12-08 16:49) - PLID 64135 - Check for expired Date
	NxButton m_checkSetReminder; 
	NxButton m_checkSelectColor;
	CNxColor	m_typeColor; // (s.tullis 2014-12-08 16:49) - PLID 64135 NxColor Control
	CCommonDialog60	m_ctrlRuleColorPicker;
	CDateTimePicker m_dateRuleStart;// (s.tullis 2014-12-08 16:49) - PLID 64135 - Start time picker
	CDateTimePicker m_dateRuleEnd;// (s.tullis 2014-12-08 16:49) - PLID 64135 - End time picker
	CNxEdit m_TodoDayExpired;
	CNxIconButton m_btnLocation_Left;
	CNxIconButton m_btnLocation_Right;
	CNxIconButton m_btnLocation_Left_All;
	CNxIconButton m_btnLocation_Right_All;
	CNxIconButton m_btnInsurance_Left;
	CNxIconButton m_btnInsurance_Right;
	CNxIconButton m_btnInsurance_Left_All;
	CNxIconButton m_btnInsurance_Right_All;
	CNxIconButton m_btnAddTypes;
	CNxIconButton m_btnCopyTo;
	CNxIconButton m_btnResource_Left;
	CNxIconButton m_btnResource_Right;
	CNxIconButton m_btnOk;
	CNxIconButton m_btnCancel;
	COleDateTime m_dateStart;
	COleDateTime m_dateEnd;
	CNxLabel m_MultiAssignedTodoUsers;
	
	NXDATALIST2Lib::_DNxDataListPtr m_pUnSelectedLocations;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelectedLocations;
	NXDATALIST2Lib::_DNxDataListPtr m_pUnSelectedInsurance;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelectedInsurance;
	NXDATALIST2Lib::_DNxDataListPtr m_pResourceList;
	NXDATALIST2Lib::_DNxDataListPtr m_pRuleDetailList;
	NXDATALIST2Lib::_DNxDataListPtr m_pTodoPriority;
	NXDATALIST2Lib::_DNxDataListPtr m_pTodoCategory;
	NXDATALIST2Lib::_DNxDataListPtr m_pTodoUserAssigned;
	

// Dialog Data
	enum { IDD = IDD_SCHEDULING_MIX_RULES_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	COleDateTime m_dtStartDate;// (s.tullis 2014-12-08 16:49) - PLID 64135 
	COleDateTime m_dtEndDate;// (s.tullis 2014-12-08 16:49) - PLID 64135 
	OLE_COLOR m_nColor;// (s.tullis 2014-12-08 16:49) - PLID 64135 
	long m_nTodoTaskID;
	long m_nRuleID;
	CString m_strTodoAssignedIDS;
	CString m_strRuleName;
	std::map<nResourceID, RuleDetail*> m_mDetailRuleMap; // (s.tullis 2014-12-08 17:16) - PLID 64137 - to keep the Rule Details in memory
	CArray<long, long> m_arrAssignedTodoUsers;
	SchedulingMixRulesConfigMode m_ConfigurationMode;// (s.tullis 2014-12-08 15:49) - PLID 64125 -  Need a variable to tell us what mode we're in
	DECLARE_MESSAGE_MAP()
public:
	DECLARE_EVENTSINK_MAP()
	// (s.tullis 2014-12-08 16:48) - PLID 64134 ----------------------------------
	afx_msg void OnBnClickedOk();
	BOOL Validate();
	void Load();
	void Save();
	afx_msg void OnBnClickedCancel();
	
	//----------------------------------------------------------------------------
	// (s.tullis 2014-12-08 17:04) - PLID 64136 ----------------------------------
	void GetSelectedLocationQuery(CParamSqlBatch &batch);
	void GetSelectedInsQuery(CParamSqlBatch &batch);
	afx_msg void OnBnClickedLocationRight();
	afx_msg void OnBnClickedLocationLeft();
	afx_msg void OnBnClickedInsuranceRight();
	afx_msg void OnBnClickedInsuranceLeft();
	afx_msg void OnBnClickedLocationRightAll();
	afx_msg void OnBnClickedLocationLeftAll();
	afx_msg void OnBnClickedInsuranceRightAll();
	afx_msg void OnBnClickedInsuranceLeftAll();
	void DblClickCellUnselectedLocations(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellUnselectedInsurance(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellSelectedLocations(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellSelectedInsurance(LPDISPATCH lpRow, short nColIndex);
	//-------------------------------------------------------------------------
	
	
	
	// (s.tullis 2014-12-08 17:41) - PLID 64138 -------------------------------
	long CreateTodo();
	void SelChangedTodoUsersAssigned(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	void SetTodoControls(BOOL OnOFF);
	void SetTodoUsers(CString strTodoAssigned, CString strTodoAssignedToNames="");
	void HandleMultiUserSel();
	void SetTodoLists();
	afx_msg void OnBnClickedRuleTodoCheck();
	afx_msg void OnEnKillfocusExpirationEdit();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//-------------------------------------------------------------------------
	// (s.tullis 2014-12-08 17:16) - PLID 64137 -------------------------------
	afx_msg void OnBnClickedResourceRight();
	afx_msg void OnBnClickedResourceLeft();
	CString GetApptTypeComboSource();
	CString GetApptPurposeComboSource(long nApptTypeID = -1, long nRuleID =-1);
	void AddRuleDetail(RuleDetail &rd);
	void LoadRuleDetails();
	void CopyToResource(CArray<long, long> &arrIDs);
	void ClearAllResources();
	void SelChangedRuleResourceList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnBnClickedRuleCopyTo();
	afx_msg void OnBnClickedRuleDetailAddTypes();
	void EditingFinishingRuleDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void EditingFinishedRuleDetailList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	void RButtonDownRuleDetailList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	BOOL FindDetailDelete(long nResourceID, long nApptID, long nPurposeID);
	BOOL DeleteRuleDetailRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
	RuleDetail *FindDetail(long nResourceID, long nAppttypeID, long nPurposeID);
	void UpdateRuleDetail(NXDATALIST2Lib::IRowSettingsPtr pRow, long nResourceID, long nOldApptValue = -2);
	void ClearResource(long nResourceID);
	void GetRuleDetailsQuery(CParamSqlBatch &batch); 
	void ReflectRuleDetailListChange(long ResourceID);
	//--------------------------------------------------------------------------
	// (s.tullis 2014-12-08 16:49) - PLID 64135 -----------------------------------
	afx_msg void OnBnClickedSelectColorCheck();
	void ClickRuleTypeColor();
	afx_msg void OnBnClickedSetDateCheck();
	//----------------------------------------------------------------------------
	
	
};

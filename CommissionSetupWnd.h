#if !defined(AFX_COMMISSIONSETUPWND_H__DCEED2CD_4C58_4D19_B30D_695191D83259__INCLUDED_)
#define AFX_COMMISSIONSETUPWND_H__DCEED2CD_4C58_4D19_B30D_695191D83259__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CommissionSetupWnd.h : header file
//

// (a.wetta 2007-03-28 16:10) - PLID 25360 - Created this class to support the new commission setup

#include "ContactsRc.h"

enum TypeOfChange {
	tocNone = 0,
	tocUpdate = 1,
	tocRemove = 2,
	tocChange = 3, // (b.eyers 2015-05-12) - PLID 65976
};

struct ServiceUpdate {	
	double dblPercentage;
	BOOL bNew;
	CArray<long,long> aryServiceIDs;	
	COleCurrency cyAmount; // (b.eyers 2015-05-11) - PLID 65976
};

// (j.jones 2009-11-18 16:13) - PLID 29046 - renamed to reflect that this is
// for "basic" rules stored in CommissionRulesLinkT
struct BasicRuleInfo {
	long nRuleID;
	long nProvID;
	long nServiceID;
	CString strRuleName;
	CString strServiceName;
};

// (j.jones 2009-11-18 16:13) - PLID 29046 - added to track "tiered" rules
// stored in TieredCommissionRulesLinkT
struct TieredRuleInfo {
	long nRuleID;
	long nProvID;
	long nRuleType;	//0 - Services, 1 - Products, 2 - Gift Certs., -1 - All
	CString strRuleName;
};

/////////////////////////////////////////////////////////////////////////////
// CCommissionSetupWnd dialog

class CCommissionSetupWnd : public CNxDialog
{
// Construction
public:
	CCommissionSetupWnd(CWnd* pParent);   // standard constructor

	long m_nProviderID;
	// (a.wetta 2007-04-02 09:49) - PLID 25360 - Tab mode should be set to true if this window will be shown on a tab as 
	// opposed to a pop up dialog.  In tab mode the window saves changes as they are made.  In non-tab mode, changes
	// are saved when the window is closed giving the user a chance to cancel changes.
	BOOL m_bTabMode;
	BOOL CloseWindow(BOOL bSaveData);
	BOOL m_bPercent; // (b.eyers 2015-05-13) - PLID 65977
	// (a.wetta 2007-03-30 10:59) - PLID 24872 - Background color
	DWORD m_backgroundColor;
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Dialog Data
	// (a.wetta 2007-03-29 14:57) - PLID 25407 - Make sure the buttons match in the Retail tab (they must be CNxIconButtons)
	//{{AFX_DATA(CCommissionSetupWnd)
	enum { IDD = IDD_COMMISSION_SETUP_WND };
	NxButton	m_btnShowOutdatedRules;
	CNxIconButton	m_btnHideShowRules;
	CNxIconButton	m_btnAdvSetup;
	CNxIconButton	m_btnCommRuleSetup;
	CNxIconButton	m_btnApplyQuickRule;
	CNxIconButton	m_btnApplyQuickComm;
	CNxColor	m_bkgColor;
	CNxEdit	m_nxeditQuickComm;
	CNxEdit m_nxeditQuickCommAmount; // (b.eyers 2015-05-13) - PLID 65977
	CNxStatic	m_nxstaticCommissionTitle;
	// (j.jones 2009-11-18 09:46) - PLID 29046 - added tiered commissions
	NxButton	m_radioBasicCommissions;
	NxButton	m_radioTieredCommissions;
	NxButton	m_checkIgnoreShopFee;
	CNxStatic	m_nxstaticQuickSetupLabel;
	CNxStatic	m_nxstaticSetCommissionLabel1;
	CNxLabel	m_nxstaticSetCommissionLabel2;
	CNxStatic	m_nxstaticCurrentCommissionLabel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCommissionSetupWnd)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pProvCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pCurrentList;
	NXDATALIST2Lib::_DNxDataListPtr m_pQuickRuleCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pCommTypeCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_pRuleTypeCombo;
	// (j.jones 2009-11-18 09:46) - PLID 29046 - added tiered commissions
	NXDATALIST2Lib::_DNxDataListPtr m_pTieredRuleList;

	bool m_bChanged;
	bool m_bShowOutdated;

	// (j.jones 2009-11-18 11:05) - PLID 29046 - renamed RefreshCurrentList() to Load()
	// as it now does much more than simply requery a list
	void Load();
	void LaunchCommissionRuleEditor();
	void ClearSelected();
	void EnableControls(bool bEnable);
	// (b.savon 2011-12-07 09:05) - PLID 46902 - Don't allow users with no permissions to view provider commissions
	void EnforceNoProviderCommissionPermissions();
	// (j.jones 2009-11-18 11:10) - PLID 29046 - added DisplayControls(), which will
	// display the proper controls for basic or tiered commissions, as indicated
	// by the parameter
	void DisplayControls(BOOL bUseTieredCommissions);
	void CreateDeleteRecord(IN NXDATALIST2Lib::IRowSettingsPtr pRow, IN OUT CArray<ServiceUpdate*,ServiceUpdate*> &aryRemoveList, long &nAuditTransactionID);
	void CreateSaveRecord(IN NXDATALIST2Lib::IRowSettingsPtr pRow, IN OUT CArray<ServiceUpdate*,ServiceUpdate*> &aryUpdateList, long &nAuditTransactionID);
	bool Save();
	void RemovePercentage(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void ApplyPercentage(NXDATALIST2Lib::IRowSettingsPtr pRow, double dblPercent);
	// (b.eyers 2015-05-11) - PLID 65976
	void RemoveAmount(NXDATALIST2Lib::IRowSettingsPtr pRow);
	void ApplyAmount(NXDATALIST2Lib::IRowSettingsPtr pRow, COleCurrency cyAmount);
	// (j.jones 2009-11-18 16:13) - PLID 29046 - renamed these functions to reflect that these are
	// for "basic" rules stored in CommissionRulesLinkT
	void AddBasicRuleToRow(NXDATALIST2Lib::IRowSettingsPtr pRow, long nRuleID, CString strRuleName = "", double dblRulePercentage = 0, COleCurrency cyRuleAmount = COleCurrency(0,0)); // (b.eyers 2015-05-15) - PLID 65978
	void RemoveBasicRule(NXDATALIST2Lib::IRowSettingsPtr pRuleRow);
	// (j.jones 2009-11-18 16:13) - PLID 29046 - added to track "tiered" rules
	// stored in TieredCommissionRulesLinkT
	void RemoveTieredRule(NXDATALIST2Lib::IRowSettingsPtr pRuleRow);
	void ExpandAllRows(VARIANT_BOOL bExpand);

	// (b.eyers 2015-05-13) - PLID 65977
	LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);

	// (j.jones 2009-11-18 16:13) - PLID 29046 - renamed these arrays to reflect that these are
	// for "basic" rules stored in CommissionRulesLinkT
	CArray<BasicRuleInfo,BasicRuleInfo> m_aryAddedBasicRules;
	CArray<BasicRuleInfo,BasicRuleInfo> m_aryRemovedBasicRules;

	// (j.jones 2009-11-18 16:13) - PLID 29046 - added to track "tiered" rules
	// stored in TieredCommissionRulesLinkT
	CArray<TieredRuleInfo,TieredRuleInfo> m_aryAddedTieredRules;
	CArray<TieredRuleInfo,TieredRuleInfo> m_aryRemovedTieredRules;

	// Keep track of the current percent value
	double m_dblPercent;

	// (b.eyers 2015-05-13) - PLID 65977
	COleCurrency m_cyAmount;

	// Generated message map functions
	//{{AFX_MSG(CCommissionSetupWnd)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdvSetup();
	afx_msg void OnCommRuleSetup();
	afx_msg void OnEditingFinishedCurrentList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRButtonDownCurrentList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnShowOutdatedRules();
	afx_msg void OnSelChangingCommissionProvList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChosenCommissionProvList(LPDISPATCH lpRow);
	afx_msg void OnApplyQuickComm();
	afx_msg void OnApplyQuickRule();
	afx_msg void OnHideshowRules();
	afx_msg void OnSize(UINT nType, int cx, int cy);  // (a.wetta 2007-03-29 13:05) - PLID 25407 - Make the dialog sizeable
	afx_msg void OnEditingStartingCurrentList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnEditingFinishingCurrentList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnKillfocusQuickComm();
	afx_msg void OnKillfocusQuickCommAmount(); // (b.eyers 2015-05-13) - PLID 65977
	// (j.jones 2009-11-18 09:46) - PLID 29046 - added tiered commissions
	afx_msg void OnRadioBasicCommissions();
	afx_msg void OnRadioTieredCommissions();
	afx_msg void OnCheckIgnoreShopFee();
	afx_msg void OnRButtonDownTieredRuleList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMMISSIONSETUPWND_H__DCEED2CD_4C58_4D19_B30D_695191D83259__INCLUDED_)

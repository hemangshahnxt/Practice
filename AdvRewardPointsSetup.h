#if !defined(AFX_ADVREWARDPOINTSSETUP_H__E0036067_17E6_4240_8939_5D1C07846360__INCLUDED_)
#define AFX_ADVREWARDPOINTSSETUP_H__E0036067_17E6_4240_8939_5D1C07846360__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AdvRewardPointsSetup.h : header file
//

// (a.wetta 2007-05-21 16:53) - PLID 26078 - Created AdvRewardPointsSetup dialog

enum RewardItemTypes {
	ritServiceCode = 0,
	ritInventoryItem,
	ritGiftCertificate,
};

enum RewardListFields {
	rlfID = 0,
	rlfType,
	rlfCategoryID,
	rlfCategory,
	rlfCode,
	rlfName,
	rlfPoints,
	rlfOldPoints,
	rlfPrice, // (a.walling 2007-09-21 13:37) - PLID 26172 - Column for price
	rlfDiscountID,
	rlfPercent,
	rlfOldPercent,
	rlfDollars,
	rlfOldDollars	 
};

/////////////////////////////////////////////////////////////////////////////
// CAdvRewardPointsSetup dialog

class CAdvRewardPointsSetup : public CNxDialog
{
// Construction
public:
	CAdvRewardPointsSetup(CWnd* pParent);   // standard constructor

	enum ERewardPointsSetupType {
		erpstAccumulate = 0,
		erpstRedeem
	};

	inline void SetType(ERewardPointsSetupType erpst) {m_erpstType = erpst;};

// Dialog Data
	//{{AFX_DATA(CAdvRewardPointsSetup)
	enum { IDD = IDD_ADV_REWARD_POINTS_SETUP };
	NxButton	m_btnShowPrice;
	NxButton	m_btnDollars;
	NxButton	m_btnPercent;
	CNxIconButton	m_btnApplyDiscount;
	CNxIconButton	m_btnOk;
	CNxIconButton	m_btnCancel;
	CNxIconButton	m_btnApply;
	CNxEdit	m_nxeditQuickRewardValue;
	CNxEdit	m_nxeditQuickRewardDiscount;
	CNxStatic	m_nxstaticSetRewardValue;
	CNxStatic	m_nxstaticRewardValuesForAll;
	CNxStatic	m_nxstaticSetRewardDiscount;
	CNxStatic	m_nxstaticRewardDiscountsForAll;
	CNxStatic	m_nxstaticCategoryRewardText;
	CNxStatic	m_nxstaticSupplierRewardText;
	CNxStatic	m_nxstaticRewardsListHeader;
	NxButton	m_chkDiscount;	// (j.gruber 2010-07-29 12:07) - PLID 31147 - reward discounts
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAdvRewardPointsSetup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pItemTypeList;
	NXDATALIST2Lib::_DNxDataListPtr m_pItemTypeList2;
	NXDATALIST2Lib::_DNxDataListPtr m_pCategoryList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSupplierList;
	NXDATALIST2Lib::_DNxDataListPtr m_pRewardValueList;

	void RefreshRewardValueList();

	bool m_bChanged;

	// (a.walling 2007-05-29 17:39) - PLID 26172
	ERewardPointsSetupType m_erpstType;

	inline BOOL IsAccumulate() {return m_erpstType == erpstAccumulate;};
	inline BOOL IsRedeem() {return m_erpstType == erpstRedeem;};

	BOOL Save();

	// Generated message map functions
	//{{AFX_MSG(CAdvRewardPointsSetup)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnSelChangedRewardItemTypeCombo(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnEditingFinishedRewardValueList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnEditingFinishingRewardValueList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnKillfocusQuickRewardValue();
	afx_msg void OnSelChangingRewardSupplierCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingRewardCategoryCombo(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnApplyQuickRewardValue();
	afx_msg void OnApplyQuickRewardDiscounts();
	afx_msg void OnKillfocusQuickRewardDiscount();
	afx_msg void OnSelChangedRewardItemTypeCombo2(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	afx_msg void OnRequeryFinishedCategoryCombo(short nFlags); // (a.walling 2007-09-21 13:44) - PLID 26172 - Added afx_msg to reenable class wizard
	afx_msg void OnRequeryFinishedSupplierCombo(short nFlags);
	afx_msg void OnShowPrice();
	afx_msg void OnRadioRewardPercent();
	afx_msg void OnRadioRewardDollars();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADVREWARDPOINTSSETUP_H__E0036067_17E6_4240_8939_5D1C07846360__INCLUDED_)

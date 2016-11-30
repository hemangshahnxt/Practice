#if !defined(AFX_MULTIFEES_H__A1FDD163_ECED_11D2_B6BE_00104B2FE914__INCLUDED_)
#define AFX_MULTIFEES_H__A1FDD163_ECED_11D2_B6BE_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MultiFees.h : header file
//

#include "client.h"

/////////////////////////////////////////////////////////////////////////////
// CMultiFees dialog

class CMultiFees : public CNxDialog
{
// Construction
public:
	// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Made this static since it has nothing to do with our current fee schedule
	static bool CheckValidFeeGroup(long insuranceCoID, long providerID, long locationID, const COleDateTime &effectiveFromDate, const COleDateTime &effectiveToDate);

	// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Static utility to warn about potential changes to the current fee schedule
	// Pass an empty CSqlFragment for anything you want to pull from the existing data state, or a 
	// populated CSqlFragment for anything you want to simulate changing to warn the user of a conflict.
	static bool CheckFeeGroupChange(long feeGroupID, const CSqlFragment &sqlChangeInactive, const CSqlFragment &sqlChangeEffectiveFromDate, const CSqlFragment &sqlChangeEffectiveToDate);

	// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Utility member function to warn about potential new dates for the current fee schedule
	bool CheckEffectiveDateChange(const COleDateTime &effectiveFromDate, const COleDateTime &effectiveToDate);

	CMultiFees(CWnd* pParent);   // standard constructor
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

// Dialog Data
	//{{AFX_DATA(CMultiFees)
	enum { IDD = IDD_MULTI_FEES };
	// (j.jones 2007-05-03 11:19) - PLID 25840 - added option to include patient resp. in allowable
	NxButton	m_checkIncludePatRespAllowable;
	NxButton	m_checkInactive;
	NxButton	m_checkIncludeCoPayAllowable;
	CNxIconButton	m_btnRemLoc;
	CNxIconButton	m_btnAddLoc;
	NxButton	m_btnWarnAllowable;
	CNxIconButton	m_btnUpdateFees;
	CNxIconButton	m_deleteButton;
	// (b.cardillo 2015-09-28 16:08) - PLID 67123 - Rename button
	CNxIconButton	m_renameButton;
	CNxIconButton	m_addButton;
	CNxIconButton	m_remProvBtn;
	CNxIconButton	m_remInsBtn;
	CNxIconButton	m_addProvBtn;
	CNxIconButton	m_addInsBtn;
	// (d.lange 2015-09-28 10:36) - PLID 67118
	CNxIconButton	m_previousFeeSchedBtn;
	CNxIconButton	m_nextFeeSchedBtn;
	NxButton	m_btnWarnMultiFee;
	NxButton	m_btnStandard;
	NxButton	m_btnAccept;
	COleVariant m_varBoundItem;
	CNxEdit	m_nxeditNotes;
	CNxStatic	m_nxstaticAnesthFeeLabel;
	// (j.jones 2009-10-19 09:43) - PLID 18558 - supported per-POS fee schedules
	CNxStatic	m_nxstaticLocationUnselectedLabel;
	CNxStatic	m_nxstaticLocationSelectedLabel;
	NxButton	m_radioLocation;
	NxButton	m_radioPOS;
	// (j.jones 2013-04-11 14:40) - PLID 12136 - supported toggling between services & products
	NxButton	m_radioServiceCodes;
	NxButton	m_radioInventoryItems;
	// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
	CDateTimePicker	m_dtEffectiveDateFromDate;
	CDateTimePicker	m_dtEffectiveDateToDate;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr m_pFeeGroups;
	NXDATALISTLib::_DNxDataListPtr m_pInsYes;
	NXDATALISTLib::_DNxDataListPtr m_pInsNo;
	NXDATALISTLib::_DNxDataListPtr m_pProvYes;
	NXDATALISTLib::_DNxDataListPtr m_pProvNo;
	NXDATALISTLib::_DNxDataListPtr m_pLocYes;
	NXDATALISTLib::_DNxDataListPtr m_pLocNo;
	NXDATALISTLib::_DNxDataListPtr m_pCPTCodes;
	// (j.jones 2013-04-11 15:10) - PLID 12136 - added products
	NXDATALIST2Lib::_DNxDataListPtr m_pProducts;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiFees)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	CTableChecker m_companyChecker, m_feegroupsChecker;
	CTableChecker m_providerChecker, m_CPTChecker, m_feeitemsChecker, m_locationChecker;
	// (j.jones 2013-04-11 15:10) - PLID 12136 - added products
	CTableChecker m_ProductChecker;

	// (j.jones 2013-04-11 15:10) - PLID 12136 - added products, but we can disable that
	// feature if they don't have the license
	bool m_bHasInventory;

	void OnUpdatePricesByPercentage();
	void OnUpdatePricesByRVU();
	void OnUpdatePricesFromFile();

	// (j.jones 2007-05-03 11:37) - PLID 25840 - if both pat. resp. and copay allowable options
	// are checked, give a don't show warning
	void TryWarnAllowableBehavior();

	// (d.lange 2015-09-28 11:12) - PLID 67118
	void UpdateFeeSchedNavigationButtons();

	// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
	void UpdateEffectiveDateControls();
	void StoreEffectiveDates();
	// (b.cardillo 2015-11-23 16:44) - PLID 67610 - Quick way to get the current dates from screen
	COleDateTime GetEffectiveFromDate();
	COleDateTime GetEffectiveToDate();

	// (b.cardillo 2015-10-05 16:56) - PLID 67113 - Ability to hide inactive / out of date
	void RequeryFeeGroups();

	// (d.lange 2015-10-06 09:09) - PLID 67122 - Updates the Fee Schedule display name to include the effective date or date range
	void UpdateFeeSchedDisplayName();

	// Generated message map functions
	//{{AFX_MSG(CMultiFees)
	virtual BOOL OnInitDialog();
	afx_msg void RequeryInsurance();
	afx_msg void RequeryCPT();
	afx_msg void RequeryProviders();
	afx_msg void OnSelectionChangeFeeGroups(long iNewRow);
	afx_msg void OnAddIns();
	afx_msg void OnRemoveIns();
	afx_msg void OnAddProvider();
	afx_msg void OnRemoveProvider();
	afx_msg void OnPaint();
	afx_msg void Load();
	afx_msg void OnKillfocusNotes();
	afx_msg void OnWarnMultiFee();
	afx_msg void OnAccept();
	afx_msg void OnBillstandard();
	afx_msg void OnDblClickInsno(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickInsyes(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickProvno(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickProvyes(long nRowIndex, short nColIndex);
	afx_msg void OnEditingFinishedCptcodes(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedCptcodes(short nFlags);
	afx_msg void OnAdd();
	// (b.cardillo 2015-09-28 16:08) - PLID 67123 - Added rename button to change the name of an existing fee schedule
	afx_msg void OnRename();
	afx_msg void OnDelete();
	afx_msg void OnEditingFinishingCptcodes(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnUpdateFees();
	afx_msg void OnWarnAllowable();
	afx_msg void OnRemoveLocation();
	afx_msg void OnAddLocation();
	afx_msg void OnDblClickCellLocno(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellLocyes(long nRowIndex, short nColIndex);
	afx_msg void OnIncludeCopayAllowable();
	afx_msg void OnEditingStartingCptcodes(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	afx_msg void OnInactiveFeeSchedule();
	// (j.jones 2007-05-03 11:19) - PLID 25840 - added option to include patient resp. in allowable
	afx_msg void OnIncludePatrespAllowable();
	// (j.jones 2009-10-19 09:43) - PLID 18558 - supported per-POS fee schedules
	afx_msg void OnRadioFeeSchedLocation();
	afx_msg void OnRadioFeeSchedPos();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2013-04-11 14:40) - PLID 12136 - supported toggling between services & products
	afx_msg void OnRadioFeeSchedCpt();
	afx_msg void OnRadioFeeSchedInventory();
	// (j.jones 2013-04-11 16:55) - PLID 12136 - supported products
	afx_msg void OnEditingStartingProducts(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	afx_msg void OnEditingFinishingProducts(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	afx_msg void OnEditingFinishedProducts(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	// (d.lange 2015-09-28 10:41) - PLID 67118
	afx_msg void OnPreviousFeeSchedClicked();
	afx_msg void OnPreviousFeeSchedDoubleClicked();
	afx_msg void OnNextFeeSchedClicked();
	afx_msg void OnNextFeeSchedDoubleClicked();
	// (b.cardillo 2015-10-02 22:54) - PLID 67120 - Effective date from/to fields
	afx_msg void OnChangeMultiFeeEffectiveDateFromDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeMultiFeeEffectiveDateToDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMultiFeeEffectiveDatesCheck();
	afx_msg void OnMultiFeeEffectiveDateToCheck();
	// (b.cardillo 2015-10-05 16:56) - PLID 67113 - Ability to hide inactive / out of date
	afx_msg void OnHideInactiveCheck();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTIFEES_H__A1FDD163_ECED_11D2_B6BE_00104B2FE914__INCLUDED_)

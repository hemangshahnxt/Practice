#if !defined(AFX_MAINSURGERIES_H__DCE73073_D6F3_11D2_B6B3_00104B2FE914__INCLUDED_)
#define AFX_MAINSURGERIES_H__DCE73073_D6F3_11D2_B6B3_00104B2FE914__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MainSurgeries.h : header file
//
#include "client.h"
#include "AdministratorRc.h"

// (c.haag 2010-10-05 09:05) - PLID 37811 - Don't use an ID of 1!
#define ID_DELETE_ITEM	34930
/////////////////////////////////////////////////////////////////////////////
// CMainSurgeries dialog

class CMainSurgeries : public CNxDialog
{
// Construction
public:
	CMainSurgeries(CWnd* pParent);   // standard constructor
	virtual void Refresh();
	//void UpdateView();

// Dialog Data
	//{{AFX_DATA(CMainSurgeries)
	enum { IDD = IDD_MAIN_SURGERIES };
	NxButton	m_btnSearchView;
	CNxIconButton	m_btnSaveAs;
	CNxIconButton	m_btnAdvSurgeryEdit;
	CNxIconButton	m_btnQuoteAdmin;
	CNxIconButton	m_addButton;
	CNxIconButton	m_renameButton;
	CNxIconButton	m_deleteButton;
	_variant_t m_varBoundItem;
	CNxEdit	m_nxeditNotes;
	CNxStatic	m_nxstaticNotesLabel;

	// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
	// which is now its own dialog and data structure
	//CNxStatic	m_nxstaticPracticeCostLabel;
	//CNxStatic	m_nxstaticPracticeCost;
	//CNxStatic	m_nxstaticLabelPersons;
	//CNxIconButton	m_btnLinkToProviders;

	CNxStatic	m_nxstaticPracticeTotalLabel;
	CNxStatic	m_nxstaticPracticeTotal;	
	CNxStatic	m_nxstaticOutsideTotalLabel;
	CNxStatic	m_nxstaticOutsideTotal;
	CNxStatic	m_nxstaticTotalLabel;
	CNxStatic	m_nxstaticTotal;
	// (j.jones 2010-01-05 12:22) - PLID 15997 - added package info
	NxButton	m_checkPackage;
	NxButton	m_radioRepeatPackage;
	NxButton	m_radioMultiUsePackage;
	CNxEdit		m_nxeditPackageTotalCount;
	CNxEdit		m_nxeditPackageTotalCost;
	CNxStatic	m_nxstaticPackageTotalCountLabel;
	CNxStatic	m_nxstaticPackageTotalCostLabel;
	// (j.jones 2010-01-06 09:00) - PLID 36763 - added package/surgery filters
	NxButton	m_radioShowAll;
	NxButton	m_radioShowSurgeries;
	NxButton	m_radioShowPackages;
	// (j.jones 2010-01-18 09:35) - PLID 24479 - added default anes/facility times
	CNxEdit		m_nxeditDefAnesthMinutes;	
	CNxEdit		m_nxeditDefFacilityMinutes;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr	m_pSurgeryItems;
	NXDATALISTLib::_DNxDataListPtr	m_pSurgeryNames;
	NXDATALISTLib::_DNxDataListPtr	m_pCPTCodes;
	NXDATALISTLib::_DNxDataListPtr	m_pProducts;

	// (j.jones 2010-01-18 09:35) - PLID 24479 - added default anes/facility times
	NXTIMELib::_DNxTimePtr	m_nxtDefAnesthStart, m_nxtDefAnesthEnd;
	NXTIMELib::_DNxTimePtr	m_nxtDefFacilityStart, m_nxtDefFacilityEnd;

	// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
	// which is now its own dialog and data structure
	//NXDATALISTLib::_DNxDataListPtr	m_pPersons;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainSurgeries)
	public:
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (j.jones 2010-01-05 15:52) - PLID 15997 - if bUpdatePackageCost is true, if the practice
	// total mismatches the package total, the package total will be replaced with the package total
	void CMainSurgeries::UpdateTotals(BOOL bUpdatePackageCost);

	//sorting setup
	void EnsureLineOrder();
	void RenumberIDs();
	bool SaveCurrentSurgeryOrder();
	bool IsListDefaultSorted();
	void IncrementListOrder(long nStartAt = 1);
	long FindPositionByAmount(COleCurrency cyFind);
	long PrepareOrderForAdd(long nType);
	bool m_bManualChange;
	void RefreshButtons();

	BOOL CheckAllowAddAnesthesiaFacilityCharge(long nServiceID);

	// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
	// which is now its own dialog and data structure
	CTableChecker m_CPTChecker, m_InventoryChecker, m_SrgyChecker /*, m_ProviderChecker, m_UserChecker, m_ContactChecker*/;

	// (j.jones 2010-01-05 15:18) - PLID 15997 - added more "changed" booleans
	BOOL m_bNotesChanged;
	BOOL m_bPackageCostChanged;
	BOOL m_bPackageCountChanged;

	// (j.gruber 2009-03-19 12:50) - PLID 33361 - add function to calculate discount amt
	void UpdateTotalDiscountAmt(long nRow);

	// (j.jones 2010-01-05 14:59) - PLID 15997 - added package info
	void DisplayPackageInfo();

	// (j.jones 2010-01-18 09:35) - PLID 24479 - added default anes/facility times
	BOOL m_bAnesthMinutesChanged;
	BOOL m_bAnesthStartTimeChanged;
	BOOL m_bAnesthEndTimeChanged;
	BOOL m_bFacilityMinutesChanged;
	BOOL m_bFacilityStartTimeChanged;
	BOOL m_bFacilityEndTimeChanged;

	// Generated message map functions
	//{{AFX_MSG(CMainSurgeries)
	virtual BOOL OnInitDialog();
	afx_msg void OnAddRecordExtraCharges();
	afx_msg void OnSelectionChangeExtraCharges(long iNewRow);
	afx_msg void OnDeleteRecordExtraCharges(const VARIANT FAR& varBoundItem);
	afx_msg void SetTotal();
	afx_msg void InitCombo();
	afx_msg void OnReturnExtraCharges();
	afx_msg void OnKillfocusNotes();
	afx_msg void OnSelChosenSurgeryNames(long nNewSel);
	afx_msg void OnSelChosenSurgeryCptcodes(long nNewSel);
	afx_msg void OnSelChosenSurgeryProducts(long nNewSel);
	afx_msg void OnRButtonDownSurgeryItems(long nRow, long nCol, long x, long y, long nFlags);
	afx_msg void OnDeleteItem();
	afx_msg void OnEditingFinishedSurgeryItems(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit);
	afx_msg void OnRequeryFinishedSurgeryItems(short nFlags);
	afx_msg void OnAdd();
	afx_msg void OnRename();
	afx_msg void OnDelete();
	afx_msg void OnQuoteAdmin();
	afx_msg void OnEditingFinishingSurgeryItems(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue);
	afx_msg void OnEditingStartingSurgeryItems(long nRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue);
	// (j.jones 2009-08-20 17:19) - PLID 35271 - removed features that were only for preference cards,
	// which is now its own dialog and data structure
	//afx_msg void OnLinkToProviders();
	//afx_msg void OnSelChosenSurgeryPersons(long nRow);
	afx_msg void OnRequeryFinishedSurgeryNames(short nFlags);
	afx_msg void OnDragEndSurgeryItems(long nRow, short nCol, long nFromRow, short nFromCol, long nFlags);
	afx_msg void OnColumnClickingSurgeryItems(short nCol, BOOL FAR* bAllowSort);
	afx_msg void OnBtnAdvSurgeryEdit();
	afx_msg void OnSearchVew();
	afx_msg void OnSurgerySaveAs();
	afx_msg void OnLButtonUpSurgeryItems(long nRow, short nCol, long x, long y, long nFlags);
	// (j.gruber 2007-05-14 16:57) - PLID 25173 - adding discount categories
	afx_msg void OnLeftClickSurgeryItems(long nRow, short nCol, long x, long y, long nFlags);
	// (j.jones 2010-01-05 13:52) - PLID 15997 - added package info
	afx_msg void OnSurgeryPackageCheck();
	afx_msg void OnRadioSurgeryRepeatPackage();
	afx_msg void OnRadioSurgeryMultiusePackage();
	afx_msg void OnKillfocusSurgeryPackageTotalCost();
	afx_msg void OnKillfocusSurgeryPackageTotalCount();
	// (j.jones 2010-01-06 09:00) - PLID 36763 - added package/surgery filters
	afx_msg void OnRadioSurgeryFilterChanged();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.jones 2010-01-18 09:35) - PLID 24479 - added default anes/facility times
	afx_msg void OnKillfocusEditDefAnesthMinutes();
	void OnKillFocusAnesthDefStartTime();
	void OnKillFocusAnesthDefEndTime();
	afx_msg void OnKillfocusEditDefFacilityMinutes();
	void OnKillFocusFacilityDefStartTime();
	void OnKillFocusFacilityDefEndTime();
	afx_msg void OnChangeEditDefAnesthMinutes();
	void OnChangedAnesthDefStartTime();
	void OnChangedAnesthDefEndTime();
	afx_msg void OnChangeEditDefFacilityMinutes();
	void OnChangedFacilityDefStartTime();
	void OnChangedFacilityDefEndTime();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINSURGERIES_H__DCE73073_D6F3_11D2_B6B3_00104B2FE914__INCLUDED_)

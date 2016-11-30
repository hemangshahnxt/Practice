#pragma once

// CPreferenceCardsDlg dialog

// (j.jones 2009-08-20 15:00) - PLID 35338 - created

#include "AdministratorRc.h"

class CPreferenceCardsDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPreferenceCardsDlg)

public:
	CPreferenceCardsDlg(CWnd* pParent);   // standard constructor

	// (j.jones 2009-08-31 16:09) - PLID 17732 - added procedure checker
	CTableChecker m_CPTChecker, m_InventoryChecker, m_ProviderChecker, m_UserChecker, m_ContactChecker, m_ProcedureChecker;

	long GetCurrentPreferenceCardID();

// Dialog Data
	enum { IDD = IDD_PREFERENCE_CARDS_DLG };
	CNxIconButton	m_btnLinkToProviders;
	CNxIconButton	m_btnAdd;
	CNxIconButton	m_btnRename;
	CNxIconButton	m_btnDelete;
	CNxIconButton	m_btnSaveAs;
	CNxIconButton	m_btnAdvPrefCardEdit;
	CNxStatic	m_nxstaticPracticeCostLabel;
	CNxStatic	m_nxstaticPracticeCost;
	CNxStatic	m_nxstaticPracticeTotalLabel;
	CNxStatic	m_nxstaticPracticeTotal;
	CNxStatic	m_nxstaticTotalLabel;
	CNxStatic	m_nxstaticTotal;
	CNxEdit		m_editNotes;
	// (j.jones 2009-08-31 16:24) - PLID 17732 - added ability to link procedures
	CNxIconButton	m_btnSelectOneProcedure;
	CNxIconButton	m_btnUnselectOneProcedure;
	CNxIconButton	m_btnUnselectAllProcedures;
	//(e.lally 2010-05-04) PLID 37048
	CNxIconButton	m_btnPrefCardPrev;
	CNxIconButton	m_btnPrefCardNext;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void Refresh();

	void UpdateTotals();
	void RefreshButtons();
	BOOL CheckAllowAddAnesthesiaFacilityCharge(long nPreferenceCardID, long nServiceID);
	
	BOOL m_bEditChanged;

	NXDATALIST2Lib::_DNxDataListPtr	m_pPreferenceCardCombo;
	NXDATALIST2Lib::_DNxDataListPtr	m_pPreferenceCardList;	
	NXDATALIST2Lib::_DNxDataListPtr	m_pProducts;
	NXDATALIST2Lib::_DNxDataListPtr	m_pPersons;
	NXDATALIST2Lib::_DNxDataListPtr	m_pServiceCodes;

	// (j.jones 2009-08-31 11:04) - PLID 29531 - added provider filter
	NXDATALIST2Lib::_DNxDataListPtr	m_pProviderFilterCombo;

	// (j.jones 2009-08-31 16:11) - PLID 17732 - added procedure lists
	NXDATALIST2Lib::_DNxDataListPtr	m_pUnselectedProcedureList;
	NXDATALIST2Lib::_DNxDataListPtr	m_pSelectedProcedureList;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	void OnSelChosenPreferenceCardsCombo(LPDISPATCH lpRow);
	void OnSelChosenPrefCardsProductsCombo(LPDISPATCH lpRow);
	void OnSelChosenPrefCardsPersonnelCombo(LPDISPATCH lpRow);
	void OnSelChosenPrefCardsServiceCombo(LPDISPATCH lpRow);
	afx_msg void OnLinkToProviders();
	afx_msg void OnAdd();
	afx_msg void OnRename();
	afx_msg void OnDelete();
	afx_msg void OnPrefCardSaveAs();
	afx_msg void OnBtnAdvPrefCardEdit();
	void OnEditingStartingPreferenceCardList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void OnEditingFinishingPreferenceCardList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue);
	void OnEditingFinishedPreferenceCardList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnEnChangeNotes();
	afx_msg void OnEnKillfocusNotes();
	void OnRequeryFinishedPreferenceCardList(short nFlags);
	void OnRButtonDownPreferenceCardList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void OnLButtonDownPreferenceCardList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (j.jones 2009-08-31 11:04) - PLID 29531 - added provider filter
	void OnSelChosenPreferenceCardProvCombo(LPDISPATCH lpRow);
	// (j.jones 2009-08-31 16:24) - PLID 17732 - added ability to link procedures
	afx_msg void OnBtnSelectOneProcedure();
	afx_msg void OnBtnUnselectOneProcedure();
	afx_msg void OnBtnUnselectAllProcedures();
	void OnDblClickCellUnselectedPrefCardProceduresList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedPrefCardProceduresList(LPDISPATCH lpRow, short nColIndex);
	//(e.lally 2010-05-04) PLID 37048
	afx_msg void OnBtnPrefCardPrev();
	afx_msg void OnBtnPrefCardNext();
	void HandleSelectedPrefCard();
};

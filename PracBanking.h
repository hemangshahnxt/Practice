#include "NxTabView.h"
#if !defined(AFX_PRACBANKING_H__3A1B1FD5_18F7_11D3_936E_00104B318376__INCLUDED_)
#define AFX_PRACBANKING_H__3A1B1FD5_18F7_11D3_936E_00104B318376__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PracBanking.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PracBanking dialog

// (j.jones 2008-05-08 17:53) - PLID 25338 - a few moduler functions reference list type,
// better to use an enumeration for that
enum EListType {

	eltCash = 0,
	eltCheck,
	eltCredit,
};

// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
enum EDetailedListType {
	eAll = 0,
	eCashAvail,
	eCashSelected,
	eCheckAvail,
	eCheckSelected,
	eCreditAvail,
	eCreditSelected,
};

class PracBanking : public CNxDialog
{
// Construction
public:

	PracBanking(CWnd* pParent);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PracBanking)
	enum { IDD = IDD_PRACBANKING };
	NxButton	m_btnIncludeRefunds;
	NxButton	m_btnIncludeTips;
	CNxIconButton	m_btnPrepareRefunds;
	CNxIconButton	m_btnRestoreDeposits;
	CNxIconButton	m_btnDepositItems;
	CNxIconButton	m_btnPrintDeposit;
	CNxIconButton	m_btnCreditRemoveOne;
	CNxIconButton	m_btnCreditSelectOne;
	CNxIconButton	m_btnCreditRemoveAll;
	CNxIconButton	m_btnCreditSelectAll;
	CNxIconButton	m_btnCheckRemoveOne;
	CNxIconButton	m_btnCheckRemoveAll;
	CNxIconButton	m_btnCheckSelectOne;
	CNxIconButton	m_btnCheckSelectAll;
	CNxIconButton	m_btnCashRemoveOne;
	CNxIconButton	m_btnCashRemoveAll;
	CNxIconButton	m_btnCashSelectOne;
	CNxIconButton	m_btnCashSelectAll;
	NxButton	m_AllPayCats;
	NxButton	m_rSinglePayCat;
	CNxIconButton	m_btnSendToQBooks;
	NxButton	m_rSingleLocation;
	NxButton	m_AllLocations;
	NxButton	m_radioInputDate;
	NxButton	m_radioServiceDate;
	NxButton	m_AllDates;
	NxButton	m_DateRange;
	NxButton	m_rAllProvider;
	NxButton	m_rSelectProvider;
	NxButton	m_rSingleProvider;
	CDateTimePicker	m_from;
	CDateTimePicker	m_to;
	CDateTimePicker	m_dtDeposit;
	CNxEdit	m_nxeditCashAvail;
	CNxEdit	m_nxeditCashSel;
	CNxEdit	m_nxeditCheckAvail;
	CNxEdit	m_nxeditCheckSel;
	CNxEdit	m_nxeditCreditAvail;
	CNxEdit	m_nxeditCreditSel;
	CNxEdit	m_nxeditTotalAvail;
	CNxEdit	m_nxeditTotalSel;
	NxButton m_checkShowPmtDates;
	NxButton m_checkShowInputDates;
	// (r.gonet 09-13-2010 12:40) - PLID 37458 - Label for a multi selection of payment categories
	CNxLabel m_nxlMultiPaymentCategories;
	// (j.jones 2011-06-16 11:36) - PLID 42038 - added filter for input user
	NxButton	m_rAllUsers;
	NxButton	m_rSingleUser;
	// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
	NxButton	m_checkRememberColWidths;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PracBanking)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// (r.gonet 09-13-2010 16:24) - PLID 37458 - Store the payment category ids currently being filtered on
	CArray<long, long> m_aryCategoryIDs;

	NXDATALISTLib::_DNxDataListPtr m_CashAvailList;
	NXDATALISTLib::_DNxDataListPtr m_CashSelectedList;
	NXDATALISTLib::_DNxDataListPtr m_CheckAvailList;
	NXDATALISTLib::_DNxDataListPtr m_CheckSelectedList;
	NXDATALISTLib::_DNxDataListPtr m_CreditAvailList;
	NXDATALISTLib::_DNxDataListPtr m_CreditSelectedList;

	NXDATALIST2Lib::_DNxDataListPtr m_pRespFilter;



	BOOL m_bCashUnselectedDone,
		m_bCheckUnselectedDone,
		m_bCreditUnselectedDone;
	// (r.gonet 2010-09-13 12:40) - PLID 37458 - Related to label notification
	BOOL m_bNotifyOnce;

	NXDATALISTLib::_DNxDataListPtr m_ProvSelect;
	NXDATALISTLib::_DNxDataListPtr m_LocSelect;
	NXDATALISTLib::_DNxDataListPtr m_CategoryCombo;
	// (j.jones 2011-06-16 11:36) - PLID 42038 - added filter for input user
	NXDATALIST2Lib::_DNxDataListPtr m_UserSelect;

	// (j.jones 2008-05-07 17:04) - PLID 25338 - removed UpdateCurrentSelect functions, and replaced with
	// new alternatives for filtering

	// (j.jones 2008-05-07 17:33) - PLID 25338 - ensures all payments are moved to the unselected lists
	void UnselectAllPayments();
	// (j.jones 2008-05-07 17:33) - PLID 25338 - reviews the unselected lists and moves payments to the
	// selected lists if they match our filters
	void FilterAllPayments();
	// (j.jones 2008-05-07 17:34) - PLID 25338 - given the payment information and the filter information,
	// this function determines whether the payment matches our filters, and if so, returns TRUE, else returns FALSE
	// (r.gonet 2010-09-13 17:40) - PLID 37458 - We now filter on an array of category ids, no longer just one category id
	// (j.gruber 2010-10-22 16:42) - PLID  37457 - added resp filter
	// (j.jones 2011-06-16 13:11) - PLID 42038 - added user filter
	BOOL DoesPaymentMatchFilter(COleDateTime dtServiceDate, COleDateTime dtInputDate, long nProviderID,
		long nLocationID, long nCategoryID, long nRespID, CString strUserName,
		BOOL bUsingDateRange, BOOL bUsingServiceDate, COleDateTime dtFrom, COleDateTime dtTo,
		BOOL bUseProviderFilter, long nProviderFilterID,
		BOOL bUseLocationFilter, long nLocationFilterID,
		BOOL bUseCategoryFilter, CArray<long, long> &aryCategoryIDs,
		long nRespFilter,
		BOOL bUseUserFilter, CString strUserFilterName);

	void SendDepositToQuickBooks();
	void SendPaymentsToQuickBooks();

	// (z.manning, 03/21/2007) - PLID 25294 - Added a parameter for the column index where the amount field is.
	COleCurrency GetTotal(NXDATALISTLib::_DNxDataListPtr nxdl, short nAmountColumnIndex);
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);

	// (j.jones 2008-05-08 14:19) - PLID 25338 - the selected lists are never requeried now,
	// so I broke up the UpdateTotals() function into UpdateAvailTotals() and UpdateSelectedTotals()
	void UpdateAvailTotals();
	void UpdateSelectedTotals();

	// (j.jones 2008-05-08 08:38) - PLID 25338 - removed RequeryAll and replaced it with
	// RefreshLists() and RequeryAvailLists()
	void RefreshLists();
	void RequeryAvailLists(BOOL bForceWaitForRequery);

	void PrintTotals(COleCurrency cash, COleCurrency check, COleCurrency credit, COleCurrency tot, CDC *pDC);	

	void OnSelectAllCardType();
	void OnUnselectAllCardType();

	// (c.haag 2009-08-31 12:10) - PLID 13175 - Updates the widths for optional columns.
	// (j.jones 2013-06-24 16:15) - PLID 35059 - Added booleans to tell this function that
	// we just caused new columns to be shown, which will force columns "remembered" at 0 width
	// to expand to a new default width.
	void UpdateVisibleColumns(bool bPaymentDatesJustShown = false, bool bInputDatesJustShown = false);
	// (j.jones 2013-06-24 09:48) - PLID 35059 - split out modular code for updating the cash,
	// check, and credit card list columns with default or remembered values
	void UpdateVisibleCashColumns(NXDATALISTLib::_DNxDataListPtr pCashList, CString strRememberedWidthsConfigRTName, bool bPaymentDatesJustShown, bool bInputDatesJustShown);
	void UpdateVisibleCheckColumns(NXDATALISTLib::_DNxDataListPtr pCheckList, CString strRememberedWidthsConfigRTName, bool bPaymentDatesJustShown, bool bInputDatesJustShown);
	void UpdateVisibleCreditColumns(NXDATALISTLib::_DNxDataListPtr pCreditList, CString strRememberedWidthsConfigRTName, bool bPaymentDatesJustShown, bool bInputDatesJustShown);

	// (j.jones 2013-06-24 09:48) - PLID 35059 - made one modular function for saving column widths	
	void SaveColumnWidths(NXDATALISTLib::_DNxDataListPtr pList, CString strRememberedWidthsConfigRTName);

	bool m_bAllSelected;
	CString sql;
	bool m_bInitPaint;
	COleCurrency cy;
	CBrush m_brush;
	CTableChecker m_ProviderChecker, m_LocationChecker, m_PaymentGroupChecker;
	// (j.jones 2007-03-13 09:03) - PLID 25118 - added DepositedPayments tablechecker
	CTableChecker m_DepositedPaymentsChecker;
	// (j.jones 2011-06-16 13:11) - PLID 42038 - added user checker
	CTableChecker m_UserChecker;

	// (j.jones 2008-05-08 14:32) - PLID 25338 - store the selected IDs in these arrays before a requery
	CArray<long, long> m_aryCashSelectedPaymentIDs;
	CArray<long, long> m_aryCashSelectedPaymentTipIDs;
	CArray<long, long> m_aryCheckSelectedPaymentIDs;
	CArray<long, long> m_aryCheckSelectedBatchPaymentIDs;
	CArray<long, long> m_aryCheckSelectedPaymentTipIDs;
	CArray<long, long> m_aryCreditSelectedPaymentIDs;
	CArray<long, long> m_aryCreditSelectedPaymentTipIDs;

	// (j.jones 2008-08-13 10:28) - PLID 25338 - store the unselected IDs as well
	CArray<long, long> m_aryCashUnselectedPaymentIDs;
	CArray<long, long> m_aryCashUnselectedPaymentTipIDs;
	CArray<long, long> m_aryCheckUnselectedPaymentIDs;
	CArray<long, long> m_aryCheckUnselectedBatchPaymentIDs;
	CArray<long, long> m_aryCheckUnselectedPaymentTipIDs;
	CArray<long, long> m_aryCreditUnselectedPaymentIDs;
	CArray<long, long> m_aryCreditUnselectedPaymentTipIDs;

	// (j.jones 2008-05-08 14:35) - PLID 25338 - will populate the selected & unselected arrays with the current list values,
	// returns TRUE if we cached anything
	BOOL CacheSelectionStates();

	// (j.jones 2008-05-08 14:35) - PLID 25338 - will reset the selected & unselected payments to their proper lists
	void RestoreCachedSelections();

	// (j.jones 2008-05-09 15:39) - PLID 25338 - clears the cached IDs
	void ClearCachedValues();

	// (j.jones 2008-05-08 14:51) - PLID 25338 - utility function used by RestoreCachedValues(), will
	// search a given array, return TRUE if it was found, and remove from the array if found
	BOOL FindAndRemoveFromArray(CArray<long, long> &aryIDs, long nValue);

	// (j.jones 2008-05-08 16:09) - PLID 25338 - utility function used by OnBtnDeposit(), will
	// convert a given array into a comma-delimited string
	void AppendArrayIntoString(CArray<long, long> &aryIDs, CString &strIDs);

	// (r.gonet 2010-09-13 12:40) - PLID 37458 - Retrieves multiple payment categories of the user's selection
	void GetMultipleCategories();
	// (r.gonet 2010-09-13 12:40) - PLID 37458 - Updates the interface to reflect changes to the payment categories filter
	void UpdateCategoriesFilterControls();

	// (j.jones 2011-06-24 12:42) - PLID 22833 - added ability to go to patient
	void GoToPatient(long nPatientID);

	// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
	bool m_bRememberColumns;

	// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
	void SaveColumns(EDetailedListType eListToSave = eAll);

	// Generated message map functions
	//{{AFX_MSG(PracBanking)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnCashSelectAll();
	afx_msg void OnCashSelectOne();
	afx_msg void OnCashUnSelectOne();
	afx_msg void OnCashUnSelectAll();
	afx_msg void OnCheckSelectAll();
	afx_msg void OnCheckSelectOne();
	afx_msg void OnCheckUnSelectAll();
	afx_msg void OnCreditSelectAll();
	afx_msg void OnCreditSelectOne();
	afx_msg void OnCreditUnSelectAll();
	afx_msg void OnCreditUnSelectOne();
	afx_msg void Print();
	afx_msg void OnCheckUnSelectOne();
	afx_msg void OnBtnDeposit();
	afx_msg void OnSelectionChangeProvselector(long iNewRow);
	afx_msg void OnDblClickCellCashAvailList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellCheckAvailList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellCheckSelectedList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellCreditAvailList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellCreditSelectedList(long nRowIndex, short nColIndex);
	afx_msg void OnDblClickCellCashSelectedList(long nRowIndex, short nColIndex);
	afx_msg void OnChangeFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRequeryFinishedCashavaillist(short nFlags);
	afx_msg void OnRequeryFinishedCheckavaillist(short nFlags);
	afx_msg void OnRequeryFinishedCreditavaillist(short nFlags);
	afx_msg void OnAlldates();
	afx_msg void OnAllproviders();
	afx_msg void OnDaterange();
	afx_msg void OnSelectProviderRadio();
	afx_msg void OnRadioInputDate();
	afx_msg void OnRadioServiceDate();
	afx_msg void OnAlllocations();
	afx_msg void OnSelectLocationRadio();
	afx_msg void OnSelChosenProvselector(long nRow);
	afx_msg void OnSelChosenLocselector(long nRow);
	afx_msg void OnSendToQuickbooks();
	afx_msg void OnAllPayCats();
	afx_msg void OnSelectPaycatRadio();
	afx_msg void OnSelChosenComboCategory(long nRow);
	afx_msg void OnRestorePastDeposits();
	afx_msg void OnIncludeTipsCheck();
	afx_msg void OnRButtonDownCreditavaillist(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRButtonDownCreditselectedlist(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnPrepareRefunds();
	afx_msg void OnIncludeRefundsCheck();
	// (j.jones 2007-03-13 09:04) - PLID 25118 - supported OnTableChanged
	afx_msg LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCheckShowPmtDates();
	afx_msg void OnCheckShowInputDates();
	// (r.gonet 2010-09-13 12:40) - PLID 37458 - Handle a left click on a label
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	// (r.gonet 2010-09-13 12:40) - PLID 37458 - Override the setting of the cursor so that we can use the hand on CNxLabel links
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// (j.gruber 2010-10-25 15:48) - PLID 37457
	void SelChosenRespFilterList(LPDISPATCH lpRow);
	void RequeryFinishedRespFilterList(short nFlags);
	// (j.jones 2011-06-16 13:54) - PLID 42038 - added user filter
	afx_msg void OnBnClickedAllusers();
	afx_msg void OnBnClickedSelectUserRadio();
	void OnSelChosenComboUserFilter(LPDISPATCH lpRow);
	// (j.jones 2011-06-24 12:40) - PLID 22833 - added right click handlers
	void OnRButtonDownCashavaillist(long nRow, short nCol, long x, long y, long nFlags);
	void OnRButtonDownCashselectedlist(long nRow, short nCol, long x, long y, long nFlags);
	void OnRButtonDownCheckavaillist(long nRow, short nCol, long x, long y, long nFlags);
	void OnRButtonDownCheckselectedlist(long nRow, short nCol, long x, long y, long nFlags);
	// (j.jones 2013-06-21 10:51) - PLID 35059 - added ability to remember column widths
	afx_msg void OnCheckRememberBankingColWidths();
	void OnColumnSizingFinishedCashavaillist(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void OnColumnSizingFinishedCashselectedlist(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void OnColumnSizingFinishedCheckavaillist(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void OnColumnSizingFinishedCheckselectedlist(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void OnColumnSizingFinishedCreditavaillist(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	void OnColumnSizingFinishedCreditselectedlist(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRACBANKING_H__3A1B1FD5_18F7_11D3_936E_00104B318376__INCLUDED_)

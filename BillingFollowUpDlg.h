#if !defined(AFX_BILLINGFOLLOWUPDLG_H__FC11EACB_2716_4D13_9CF7_7608500F5790__INCLUDED_)
#define AFX_BILLINGFOLLOWUPDLG_H__FC11EACB_2716_4D13_9CF7_7608500F5790__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BillingFollowUpDlg.h : header file

#include <NxUILib/NxStaticIcon.h>
//

/////////////////////////////////////////////////////////////////////////////
// CBillingFollowUpDlg dialog
// (a.walling 2008-05-28 14:01) - PLID 27591 - Use CDateTimePicker
// (a.wilson 2014-07-08 11:35) - PLID 62573 - cleaned up the code while implementing redesign.

// (a.wilson 2014-07-08 15:36) - PLID 62528
struct BillingColumn {
	long nColumnID;
	long nOrderIndex;
	long nStoredWidth;
	CString strName;

	BillingColumn()
	{
		nColumnID = -1;
		nOrderIndex = -1;
		nStoredWidth = -1;
		strName = "";
	}

	BillingColumn(long _nColumnID, long _nOrderIndex, long _nStoredWidth, CString _strName) {
		nColumnID = _nColumnID;
		nOrderIndex = _nOrderIndex;
		nStoredWidth = _nStoredWidth;
		strName = _strName;
	}
};

class CBillingFollowUpDlg : public CNxDialog
{
// Construction
public:
	CBillingFollowUpDlg(CWnd* pParent);   // standard constructor
	virtual ~CBillingFollowUpDlg(); // (r.gonet 07/28/2014) - PLID 63078 - standard destructor

	CDWordArray m_aryRemovedBills;
	CTableChecker m_companyChecker, m_patientChecker;
	CTableChecker m_locationChecker; // (j.jones 2009-08-11 08:50) - PLID 28131 - added location checker
	CTableChecker m_providerChecker; //(e.lally 2009-08-14) PLID 30119 - Added provider checker

	CString GetWhereClause();
	CString GetIgnoredBillList();
	void Refresh();
	void ClickRadioTFInfo();

// Dialog Data
	//{{AFX_DATA(CBillingFollowUpDlg)
	enum { IDD = IDD_BILLING_FOLLOWUP_DLG };
	// (s.tullis 2014-08-25 09:10) - PLID 62577 
	bool m_bAllStatusesSelected = true;
	CArray<long, long> m_aryStatusesSelected;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBillingFollowUpDlg)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
	CNxStaticIcon m_icoReset; // (a.wilson 2014-07-16 08:17) - PLID 62905
	CNxIconButton	m_btnWriteOffAccts; // (j.jones 2008-06-27 13:45) - PLID 27647 - added m_btnWriteOffAccts
	CNxIconButton	m_btnSearchAllowables;
	NxButton	m_checkSaveAsTracer;
	NxButton	m_checkNoPatientPays;
	CNxIconButton	m_btnDisplayResults;
	CNxIconButton	m_btnPreviewList;
	NxButton	m_checkNoInsPays;
	CNxIconButton	m_btnSendToPaper;
	CNxIconButton	m_btnSendToEbilling;
	NxButton	m_radioBetweenDates;
	NxButton	m_radioGreaterThanDays;
	CNxIconButton	m_btnCreateMergeGroup;
	CNxIconButton	m_btnPreviewTracers;
	CNxIconButton	m_btnMergeToWord;
	NxButton	m_checkTracerFormSent;
	NxButton	m_checkUseBalance;
	NxButton	m_radioTFSentYes;
	NxButton	m_radioTFSentNo;
	NxButton	m_radioTFSentDate;
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	NxButton	m_btnMergeToPrinter;
	CNxEdit	m_nxeditEditDaysOld;
	CNxEdit	m_nxeditEditTfDays;
	CNxEdit	m_nxeditEditDollars;
	CNxStatic	m_nxstaticUnpaidBillsLabel;
	CNxStatic	m_nxstaticResultsCount;
	CNxLabel	m_nxlLocationLabel; // (j.gruber 2013-02-27 13:59) - PLID 54882
	CNxLabel m_nxlMultipleBillStatus; // (s.tullis 2014-07-01 11:05) - PLID 62577 -
	NxButton	m_checkRememberColumnWidths; // (a.wilson 2014-07-08 12:02) - PLID 62528
	// (s.tullis 2014-08-11 11:51) - PLID 62577 - Table Checker Support For Bill Status
	CTableChecker m_BillStatusChecker;
	long n_CurrentBillStatusID;

	CMap<short, short, BillingColumn, BillingColumn> m_mapResultColumns; // (a.wilson 2014-07-08 16:20) - PLID 62528

	NXDATALISTLib::_DNxDataListPtr m_PatientList;
	long m_nCurrentPatientID;	// (a.wilson 2014-07-03 16:26) - PLID 62809
	NXDATALIST2Lib::_DNxDataListPtr m_pOnHoldFilterList; // (s.tullis 2014-06-30 09:41) - PLID 62574 - In the Billing Follow Up tab, in the ‘Claim Status’ section; add a filter drop down for “On Hold”.
	NXDATALISTLib::_DNxDataListPtr m_LocationCombo; // (j.jones 2009-08-11 08:37) - PLID 28131 - added location filter
	long m_nLocationID; // (j.gruber 2013-02-27 14:21) - PLID 54882
	CDWordArray m_dwLocations; // (j.gruber 2013-02-27 14:21) - PLID 54882
	NXDATALISTLib::_DNxDataListPtr m_pProviderCombo; //(e.lally 2009-08-14) PLID 30119 - added provider filter
	long m_nCurrentProviderID;	// (a.wilson 2014-07-07 09:22) - PLID 62573
	NXDATALIST2Lib::_DNxDataListPtr m_pBillStatusFilter; // (s.tullis 2014-06-30 17:54) - PLID 62577 - 
	NXDATALIST2Lib::_DNxDataListPtr m_pDateFilter; // (a.wilson 2014-07-07 12:05) - PLID 62809 - new datalist controls.
	NXDATALIST2Lib::_DNxDataListPtr m_pRespFilter;
	NXDATALIST2Lib::_DNxDataListPtr m_pInsuranceFilter;
	long m_nCurrentInsuranceID;	// (a.wilson 2014-07-08 10:48) - PLID 62809
	NXDATALIST2Lib::_DNxDataListPtr m_pHCFAFilter;
	NXDATALIST2Lib::_DNxDataListPtr m_pClaimSentFilter;
	NXDATALIST2Lib::_DNxDataListPtr m_pResultsList; // (a.wilson 2014-07-08 13:09) - PLID 62528 - upgraded to make things easier.
	
	// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - Use CIncreaseCommandTimeout instead of accessing g_ptrRemoteData
	CString m_strCurrentFrom;	//used for the report
	scoped_ptr<CIncreaseCommandTimeout> m_pIncreaseCommandTimeout;

	// (r.gonet 07/25/2014) - PLID 62920 - Count of the static columns currently in the bill results list.
	// Now stored as a member variable rather than a magic number.
	long m_nResultsListStaticColumnCount;

	// (r.gonet 07/28/2014) - PLID 63078 - Hand icon for the On Hold column.
	HICON m_hIconBillOnHold;
	HICON m_hBillNote;	// (a.wilson 2014-08-01 10:00) - PLID 62784

	void EnableListControls(BOOL bEnabled);

	// (r.gonet 07/25/2014) - PLID 62920 - Changes the bill's on hold state.
	bool SetBillOnHold(long nBillID, BOOL bOnHold);

	//DRT 6/14/2007 - PLID 21395 - This holds the current "date range" that is shown on the report.  It's kept in a member
	//	because the "display results" is not tied to the "preview list", and we do not want them to differ from what
	//	was actually used.
	CString m_strCurrentDateRangeForReport;

	// (j.gruber 2013-02-27 13:59) - PLID 54882
	void OnLocationList();

	// (s.tullis 2014-06-24 14:47) - PLID 62506 - Permission: Billing Followup User Permission to Control Read and Writing in the Billing Followup Tab
	void SecureControls();
	


	
	BOOL IsFilteringPatientResp(); // (j.gruber 2013-02-26 14:56) - PLID 47780

	// (j.jones 2008-06-27 13:45) - PLID 27647 - added OnBtnWriteOffAccts
	// Generated message map functions
	//{{AFX_MSG(CBillingFollowUpDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow();
	afx_msg void OnGoToPatient();
	afx_msg void OnRemoveSelectedBills();
	// (r.gonet 07/25/2014) - PLID 62920 - Handles when the user clicks the Mark Bill On Hold menu option
	// from the context menu of the bill results list.
	afx_msg void OnMarkBillOnHold();
	// (r.gonet 07/25/2014) - PLID 62920 - Handles when the user clicks the Remove Bill Hold menu option
	// from the context menu of the bill results list.
	afx_msg void OnRemoveBillHold();
	afx_msg void OnCheckUseBalance();
	afx_msg void OnBtnLoad();
	afx_msg void OnPreviewForms();
	afx_msg void OnRequeryFinishedBillFuList(short nFlags);
	afx_msg void OnSendToPaperBatch();
	afx_msg void OnSendToEbilling();
	afx_msg void OnMergeDocument();
	afx_msg void OnCheckTracerFormSent();
	afx_msg void OnRadioTfSentYes();
	afx_msg void OnRadioTfSentNo();
	afx_msg void OnRadioTfSentDate();	
	afx_msg void OnCreateLwGroup();
	afx_msg void OnRadioGreaterThanDays();
	afx_msg void OnRadioBetweenDateRange();
	afx_msg void OnPreviewList();
	afx_msg void OnCheckSaveMergeAsTracer();
	afx_msg void OnBtnSearchAllowables();
	afx_msg void OnBtnWriteOffAccts(); // (j.jones 2008-06-27 13:45) - PLID 27647 - added OnBtnWriteOffAccts
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message); // (j.gruber 2013-02-27 13:57) - PLID 54882
	afx_msg void OnSelChosenLocation(long nRow); // (j.gruber 2013-02-27 13:57) - PLID 54882
	afx_msg void OnRequeryFinishedLocation(short nFlags); // (j.gruber 2013-02-27 14:57) - PLID 54882
	afx_msg void OnRequeryFinishedPatient(short nFlags); // (a.wilson 2014-07-03 15:52) - PLID 62809
	afx_msg void OnRequeryFinishedProvider(short nFlags); // (a.wilson 2014-07-03 15:52) - PLID 62809
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam); // (j.gruber 2013-02-27 14:57) - PLID 54882
	void SelChangingPatientList(long FAR* nNewSel); // (a.wilson 2014-07-03 16:30) - PLID 62809
	void SelChangingLocationCombo(long FAR* nNewSel); // (a.wilson 2014-07-03 16:30) - PLID 62809
	void SelChangingProviderCombo(long FAR* nNewSel); // (a.wilson 2014-07-03 16:30) - PLID 62809
	DECLARE_EVENTSINK_MAP()
	DECLARE_MESSAGE_MAP()
	void HandleBillStatusTableCheckerRefresh(long nCurSel); // (s.tullis 2014-07-01 09:53) - PLID 62577 -
	void RequeryBillStatus(long nCurSelect, BOOL btablechecker); // (s.tullis 2014-07-01 09:53) - PLID 62577 -
	void SelChosenBillstatusControl(LPDISPATCH lpRow); // (s.tullis 2014-07-01 09:53) - PLID 62577 -
	void HandleMultiStatusSelection(); // (s.tullis 2014-07-01 09:53) - PLID 62577 -
	void SetBillStatusText(CString strText, CString strToolTip); // (s.tullis 2014-08-25 09:10) - PLID 62577 - 
	void HandleMultipleSelection( 
		CNxLabel &nxstaticMultiLabel, NXDATALIST2Lib::_DNxDataListPtr pCombo, UINT nComboControlID, CArray<long, long> &aryCurrentSelections,
		bool *pbAllSelected, CString strConfigRTName, CString strDescription,
		short nIDColumnIndex = 0, short nDescriptionColumnIndex = 1, CArray<short, short> *paryExtraColumnIndices = NULL); // (s.tullis 2014-07-01 09:53) - PLID 62577 -
	CString GetArrayOfIDsForSQL(CArray<long, long> &aryCurrentSelections); // (s.tullis 2014-07-01 09:53) - PLID 62577 -
	void BuildDateFilter();
	void SelChangingFilterDateList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void RequeryFinishedFilterResp(short nFlags);
	void SelChangingFilterResp(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void UpdateResponsibilityFilterState();
	void UpdateDateFilterState();
	void SelChangedFilterResp(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void SelChangedFilterDateList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void RequeryFinishedFilterInsuranceList(short nFlags);
	void RequeryFinishedFilterHcfaList(short nFlags);
	void SelChangingFilterInsuranceList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingFilterHcfaList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingFilterClaimList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingOnholdFilter(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SelChangingBillstatusControl(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	void SetupResultListColumns();
	void CreateResultListColumn(BillingColumn bcColumn);
	void SaveResultListColumnsSetup(bool bSaveColumnWidths = true);
	afx_msg void OnBnClickedBillingfollowupRememberColumns();
	afx_msg void OnBnClickedConfigureColumnsBtn();
	void RButtonDownBillfuResultsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	void RequeryFinishedBillfuResultsList(short nFlags);
	void ColumnSizingFinishedBillfuResultsList(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	afx_msg void OnDestroy();
public:
	void SelChosenOnholdFilter(LPDISPATCH lpRow);// (s.tullis 2014-07-14 16:15) - PLID 62574 - 
	afx_msg void OnStnClickedBillingfuReset();
	afx_msg void OnOpenBill(); //TES 7/28/2014 - PLID 62785
	void LeftClickBillfuResultsList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BILLINGFOLLOWUPDLG_H__FC11EACB_2716_4D13_9CF7_7608500F5790__INCLUDED_)

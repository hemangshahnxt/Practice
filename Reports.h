#if !defined(AFX_REPORTS_H__54C9DEC1_DD4A_11D2_B68F_0000C0832801__INCLUDED_)
#define AFX_REPORTS_H__54C9DEC1_DD4A_11D2_B68F_0000C0832801__INCLUDED_

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and ResolveDefaultTab to the modules code

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Reports.h : header file
//

#include "Client.h"


// (z.manning 2009-12-07 10:20) - Define this to show the button to verify all reports
//#define ENABLE_VERIFY_ALL_REPORTS


class CReportInfo;

// (j.gruber 2010-09-08 09:52) - PLID 37425 - changed to an enum
enum EReportDateValues {
	erdvSeparator =  -2,
	erdvAll = -1,
	erdvOneYear = 1,
	erdvCustom,
	erdvToday,	
	erdvThisWeek,
	erdvThisMonth,
	erdvThisQuarter,
	erdvThisYear,
	erdvThisMonthToDate,
	erdvThisQuarterToDate,
	erdvThisYearToDate,
	erdvYesterday,
	erdvLastWeek,
	erdvLastMonth,
	erdvLastQuarter,
	erdvLastYear,
};



// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
// using namespace NxTab;

/////////////////////////////////////////////////////////////////////////////
// CReports dialog

class CReports : public CNxDialog
{
public:
	CReportInfo *CurrReport;
	
	// Construction
public:
	short GetDefaultTab();
	bool DoInitLists(short nNewActiveTab = -1);
	CReports(CWnd* pParent);
	~CReports();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);
	void Print(bool bPreview = true, CPrintInfo *pInfo = 0);
	void LoadBatch(LPCTSTR strBatchName, LPCTSTR strUserName);
	long CheckAllowClose();	
	CString AddBatchProviders(CString strCurList); // (j.luckoski 2012-10-03 17:21) - PLID 24684

	// (a.walling 2010-11-26 13:08) - PLID 40444 - Allow interaction from the view
	short GetActiveTab();
	// (c.haag 2009-01-12 16:45) - PLID 32683 - Sets the active tab in the reports sheet.
	// Returns TRUE on success, and FALSE on failure.
	BOOL SetActiveTab(ReportsModule::Tab tab);
	

	CTableChecker m_patientChecker, m_doctorChecker, m_locationChecker, m_groupChecker, m_filterChecker;

	// (j.jones 2010-06-14 16:53) - PLID 39117 - cache the values for whether we include inactive
	// patients, providers, or locations in the dropdown filters
	BOOL m_bFilterShowInactivePatients;
	BOOL m_bFilterShowInactiveProviders;
	BOOL m_bFilterShowInactiveLocations;

public:
	// (c.haag 2009-01-12 17:02) - PLID 32683 - Called to clear the current batch
	void ClearSelection() { OnClearSelect(); }

protected:
	NxTab::_DNxTabPtr m_tab;
	CBrush m_brush;
	long m_nCurrentGroup;

	// (j.gruber 2008-07-11 13:19) - PLID 30692 - add new filtering
	NXDATALIST2Lib::_DNxDataListPtr m_pDateOptionList;
	void ChangeSelectionQuickDateFilter(LPDISPATCH lpRow);


	// (a.walling 2008-04-22 13:47) - PLID 29642 - Use NxButton/CNxStatic
	// (z.manning, 04/28/2008) - PLID 29807 - Added NxIconButtons
	// (a.walling 2008-05-13 15:04) - PLID 27591 - Use CDateTimePicker
	// (j.gruber 2009-12-28 16:55) - PLID 19189 - added apply filters button
// Dialog Data
	//{{AFX_DATA(CReports)
	enum { IDD = IDD_REPORTS };
	NxButton	m_btnAllYears;
	CNxLabel	m_nxlProviderLabel;
	CNxLabel	m_nxlExtLabel;
	NxButton	m_rSelectProv;
	NxButton	m_rSelectLoc;
	NxButton	m_rAllLocations;
	NxButton	m_rAllProviders;
	NxButton	m_SummaryCheck;
	NxButton	m_DetailCheck;
	NxButton	m_rAllPatients;
	NxButton	m_rSelectPat;
	NxButton	m_rAllDates;
	NxButton	m_rDateRange;
	NxButton	m_rDateRangeOptions;
	NxButton	m_rServDate;
	NxButton	m_rInputDate;
	NxButton	m_UseExtended;
	CNxStatic	m_CurrName;
	NxButton	m_No_Group_Filter;
	NxButton	m_UseFilter;
	NxButton	m_UseGroup;
	CNxIconButton	m_clearSelectBtn;
	CNxIconButton	m_killReportBtn;
	CNxIconButton	m_addReportBtn;
	CDateTimePicker	m_to;
	CDateTimePicker	m_from;
	CToolTipCtrl	m_ToolCtrl;
	bool	m_bToolTipsActive;
	CNxColor	m_NameBox;
	CNxStatic	m_nxstaticIngroup;
	CNxStatic	m_nxstaticLabelFilter;
	CNxStatic	m_nxstaticLabelFromdate;
	CNxStatic	m_nxstaticLabelToDate;
	CNxIconButton	m_btnNewBatch;
	CNxIconButton	m_btnDeleteBatch;
	CNxIconButton	m_btnCreateMergeGroup;
	CNxIconButton	m_btnEditReports;
	CNxIconButton	m_btnSaveBatch;
	CNxIconButton	m_btnSaveBatchAs;
	CNxLabel	m_nxlLocationLabel;
	CNxIconButton m_btnApplyFilters;
	CNxIconButton m_btnLaunchSSRS;
	//}}AFX_DATA

	NXDATALISTLib::_DNxDataListPtr m_ReportList;
	NXDATALISTLib::_DNxDataListPtr m_SelectList;
	NXDATALISTLib::_DNxDataListPtr m_PatSelect;
	NXDATALISTLib::_DNxDataListPtr m_ProvSelect;
	NXDATALISTLib::_DNxDataListPtr m_LocationSelect;
	NXDATALISTLib::_DNxDataListPtr m_GroupSelect;
	NXDATALISTLib::_DNxDataListPtr m_ExtFilterList;
	NXDATALISTLib::_DNxDataListPtr m_pDateOptions;
	NXDATALISTLib::_DNxDataListPtr m_BatchList;
	NXDATALISTLib::_DNxDataListPtr m_pReportTypeList;

	// (a.walling 2011-08-04 14:36) - PLID 44788 - Keep track of the currently selected patient
	long m_nSelectedPatientID;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReports)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	
	// (a.walling 2011-08-04 14:36) - PLID 44788 - Delay freeing the datalist when hiding / deactivating
	virtual void OnParentViewActivate(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	
	//}}AFX_VIRTUAL

// Implementation
protected:
	void AddExtItem(LPCSTR bound, LPCSTR display);
	void LoadExtendedFilter(CReportInfo *pReport);//Fills the datalist
	CString GetExtItemName(LPCTSTR strID); //Returns the value corresponding to the selected id in the extended list.
	void InitGroupCombo();
	void EnableFilterDateOptions(bool enable = true);
	void EnableDateRange(bool enable /*=true*/);
	long ShowTabs();
	void RequeryProviderList();
	// (j.jones 2010-07-19 09:54) - PLID 39117 - this now takes in a default where clause
	void RequeryLocationList(CString strDefaultWhereClause);
	BOOL CheckReportTabAvailability(const short nTabIndex);

	//for Tooltips
	void DestroyTT();
	CString GetCurrentReportDesc();
	HWND m_hTipWnd;
	CString m_strCurrentToolText;


	// Override for finding tool tip
    //virtual int  OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	// (a.walling 2008-05-13 14:57) - PLID 27591 - Use Notify handlers for DateTimePicker

	// Generated message map functions
	//{{AFX_MSG(CReports)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnSelChangedReports(long nRow);
	afx_msg void OnSelChangedSelected(long nRow);
	afx_msg void OnClearSelect();
	afx_msg void OnAddReport();
	afx_msg void OnRemoveReport();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblClickReportsavail(long nRow, long nCol);
	afx_msg void OnDblClickSelectlist(long nRow, long nCol);
	afx_msg void OnCreateMergeGroup();
	afx_msg void OnSelChangedProvselect(long nRow);
	afx_msg void OnSelChangedPatselect(long nRow);
	afx_msg void OnDroppingDownPatselect(); // (a.walling 2011-08-04 14:36) - PLID 44788
	afx_msg void OnChangeGroup();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnChangeFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUsefilter();
	afx_msg void OnUsegroup();
	afx_msg void OnSelChangedGroup(long nNewSel);
	afx_msg void OnNofilgroup();
	afx_msg void OnSelectTab(short newTab, short oldTab);
	afx_msg void OnUseExtended();
	afx_msg void OnServiceDate();
	afx_msg void OnInputDate();
	afx_msg void OnAllDates();
	afx_msg void OnDateRange();
	afx_msg void OnAllpats();
	afx_msg void OnSelectpat();
	afx_msg void OnDetailed();
	afx_msg void OnSummary();
	afx_msg void OnAlllocations();
	afx_msg void OnSelectlocation();
	afx_msg void OnAllprovs();
	afx_msg void OnSelectprov();
	afx_msg void OnSelChosenExtFilter(long nRow);
	afx_msg void OnSelChosenBatchList(long nRow);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSelChosenGroupdl(long nRow);
	afx_msg void OnRequeryFinishedGroupdl(short nFlags);
	afx_msg void OnEditreport();
	afx_msg void OnDateRangeOptions();
	afx_msg void OnSelChosenDateOptions(long nRow);
	afx_msg void OnEditFilter();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnProvList();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSelChosenProvselect(long nRow);
	afx_msg void OnPaint();
	afx_msg void OnClickColor02();
	afx_msg LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNewBatch();
	afx_msg void SaveBatchChanges();
	afx_msg void OnDeleteBatch();
	afx_msg void OnSavebatchAs();
	afx_msg void OnReportHelp();
	afx_msg void OnSelChangingProvselect(long FAR* nNewSel);
	afx_msg void OnExtList();
	afx_msg void OnSelChosenReportTypeList(long nRow);
	afx_msg void OnSearchReports();
	afx_msg void OnSelChosenReportsQuickDateFilter(LPDISPATCH lpRow);
	afx_msg void OnSelChangingReportsQuickDateFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnReportsAllYear();
	afx_msg void OnSelChosenLocationSelect(long nRow);
	afx_msg void OnLocationList();
	afx_msg void OnSelChangingLocationSelect(long FAR* nNewSel);
	afx_msg void OnSelChangingReportType(long FAR* nNewSel); // (j.gruber 2011-06-17 12:36) - PLID 38835
	// (a.walling 2011-08-04 14:36) - PLID 44788
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CString strTmp;
	CPtrArray		m_paramList;		//Crystal ParameterList
	BOOL m_bModified;
	CString m_strCurrentBatchName;

	CRPEngine *RepEngine;

	bool	m_bIsLoading;
	short	m_currentTab;

public:
	// (c.haag 2009-01-13 11:46) - PLID 32683 - This is similar to AddReport, but OnAddReport and all
	// its internal checks are done beforehand. This function assumes the active tab has the report.
	void AddReportFromExternalModule(long nReportID);
private:
	void AddReport(long nReportID = -1, BOOL bAutoSelect = TRUE);
	void RemoveReport(long nReportID = -1);
	void LoadFilters(CReportInfo *pReport);
	void ResetFilters();
	void LoadFormatFilters(CReportInfo *pReport);
	void LoadDateFilters(CReportInfo *pReport);


	// TODO: Phasing this variable out////////////////
	CPtrArray m_RBatch;					// the array to hold multiple report settings
	/////////////////////////////////////////////////

	int RepSeek(long nReportID, int nFailValue = 0);		// Seek function to find the right report, returns index position

	void SetReportParameters(CString csReport, CReportInfo *rpt);
	BOOL GenerateGroupFilter(IN CReportInfo *pReport, OUT CString *pstrOutFilter = NULL, OUT long *pnItemCount = NULL);
	///BOOL GenerateFilter(IN CReportInfo *pReport, OUT CString *pstrOutFilter = NULL, OUT long *pnItemCount = NULL);

public:
	// These are static and so always available even without instanciating an object
	static const CReportInfo gcs_aryKnownReports[];
	static const long gcs_nKnownReportCount;
	CReportInfo *FindReport(long nReportId);
	
protected:
	CMapPtrToPtr m_lstReports;		// List of loaded reports keyed on ID
	CReportInfo *LoadReport(const CReportInfo &repCopyFrom, bool bAutoReload = false);
	BOOL PrintReport(CReportInfo *PrintReport, bool bPreview, CPrintInfo *pInfo);
public:
	void OnSelChangingComboReportbatches(long* nNewSel);
	afx_msg void OnBnClickedVerifyAllReports();
	

protected:
	// (j.gruber 2009-12-28 13:13) - PLID 19189 - added ability to copy filters
	BOOL GetAllowedFilteredLocations(CString strLocationFilter, long nCopyFromLocation, CDWordArray *dwCopyFromLocations, CDWordArray *dwAllowedLocations);
	afx_msg void OnBnClickedReportsApplyFilters();
public:
	afx_msg void OnBnClickedBtnLaunchSsrsReporting();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REPORTS_H__54C9DEC1_DD4A_11D2_B68F_0000C0832801__INCLUDED_)

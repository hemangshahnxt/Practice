#if !defined(AFX_EMRSEARCH_H__2833B09F_AD0E_4E6B_8631_97BB92E33C40__INCLUDED_)
#define AFX_EMRSEARCH_H__2833B09F_AD0E_4E6B_8631_97BB92E33C40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMRSearch.h : header file
//

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////
// CEMRSearch dialog

enum EEMNListColumns {
	eelcEMRID = 0,
	eelcEMNID,
	eelcPICID,
	eelcPatientID,
	eelcCheckForSignature, // (z.manning 2011-10-28 15:37) - PLID 44594
	eelcProvider,
	eelcSecProvider, // (z.manning 2009-01-06 16:26) - PLID 26167
	eelcTechnician,	// (d.lange 2011-04-25 10:21) - PLID 43381
	eelcPreview, // (a.walling 2010-01-11 13:38) - PLID 31482
	eelcPatient,
	eelcDescription,
	eelcStatusID, // (z.manning 2012-07-05 15:48) - PLID 50958
	eelcStatus,
	eelcLocation,
	eelcDate,
	eelcAppointmentID, // (z.manning, 03/03/2008) - PLID 29158
	eelcTime, // (z.manning, 02/29/2008) - PLID 29158
	eelcResource, // (z.manning, 02/29/2008) - PLID 29158
	eelcShowState, // (z.manning, 03/03/2008) - PLID 29158
	eelcCreated,
	eelcModified,
	eelcIsEmn,
	eelcHasBeenBilled, // (z.manning, 02/29/2008) - PLID 29158
};

enum EGenericIDNameColumns {
	egcID = 0,
	egcName
};

enum EDateTypeList {
	dtlEMNLastModifiedDate = 0,
	dtlEMNDate,
	dtlEMNInputDate,
	dtlPatientSeen
};

class CEMRSearch : public CNxDialog
{
// Construction
public:
	CEMRSearch(CWnd* pParent);   // standard constructor
	CString VariantArrayToString(CVariantArray &va);
	void RefreshList(bool bForce = false);

	void SetSeenToday(bool bSeen = true); // hide advanced controls and set to patient seen today!

	// (z.manning 2013-10-17 09:01) - PLID 59061 - This dialog no longer responds to table checkers
	/*// (j.jones 2006-11-02 11:47) - PLID 23321 - supported receiving tablecheckers
	//virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	// (z.manning, 03/03/2008) - PLID 29158 - We now handle appointment table checkers so I added OnTableChangedEx
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);*/

	// (a.walling 2008-04-02 09:51) - PLID 29497 - Added NxButtons
// Dialog Data
	//{{AFX_DATA(CEMRSearch)
	enum { IDD = IDD_EMR_SEARCH };
	NxButton	m_btnRememberColumns;
	// (j.jones 2014-05-15 16:40) - PLID 61798 - renamed the existing refresh checkbox to clearly
	// state it only applies to filters, and created a new checkbox for refreshing upon window restore
	NxButton	m_btnRefreshOnFilterChange;
	NxButton	m_btnRefreshOnWindowRestore;
	CNxIconButton m_btnRefresh;
	NxButton	m_btnStatusFinished;
	NxButton	m_btnStatusLocked;
	NxButton	m_btnStatusOpen;
	// (j.jones 2011-07-06 09:58) - PLID 44451 - added "Other" status check
	NxButton	m_btnStatusOther;
	NxButton	m_checkShowSeenWith;
	NxButton	m_checkShowSeen;
	NxButton	m_checkExcludePending; // (a.walling 2011-01-13 09:58) - PLID 41542
	CDateTimePicker	m_dtFrom;
	CDateTimePicker	m_dtTo;
	CNxStatic	m_nxstaticFrom;
	CNxStatic	m_nxstaticTo;
	CNxStatic	m_nxstaticProc;
	CNxStatic	m_nxstaticDiag;
	CNxStatic	m_nxstaticStatuslabel;
	CNxStatic	m_nxstaticProb;
	CNxStatic	m_nxstaticProv;
	CNxStatic	m_nxstaticSecProv;
	CNxStatic	m_nxstaticTech;		// (d.lange 2011-04-22 14:44) - PLID 43381 - Assistant/Technician label
	CNxStatic	m_nxstaticLoc;
	CNxStatic	m_nxstaticFilter;
	CNxIconButton m_btnEMRSearchClose;
	NxButton	m_btnDateGroup;
	NxButton	m_btnEmrgroup1;
	NxButton	m_btnEmrgroupStatus;
	NxButton	m_btnEmrgroup2;
	CNxIconButton m_btnEMRSearchPreview;
	NxButton	m_btnShowCurrentPatientOnly;
	CNxIconButton m_btnSignSelected; // (z.manning 2011-11-03 15:25) - PLID 44594
	// (j.jones 2013-07-15 14:49) - PLID 57477 - added chart & category filters
	CNxStatic	m_nxstaticChart;
	CNxStatic	m_nxstaticCategory;
	// (a.wilson 2014-03-04 15:24) - PLID 60780
	NxButton	m_btnDiagnosisIncludeNone;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMRSearch)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXDATALIST2Lib::_DNxDataListPtr m_dlDateType;
	NXDATALIST2Lib::_DNxDataListPtr m_dlProcedure;
	NXDATALIST2Lib::_DNxDataListPtr m_dlProblems;
	NXDATALIST2Lib::_DNxDataListPtr m_dlProvider;
	NXDATALIST2Lib::_DNxDataListPtr m_dlSecProvider; // (z.manning 2008-11-12 14:33) - PLID 26167
	NXDATALIST2Lib::_DNxDataListPtr m_dlTechnician;	// (d.lange 2011-04-22 14:41) - PLID 43381 - Added Assistant/Technician filter dropdown
	NXDATALIST2Lib::_DNxDataListPtr m_dlLocation;
	NXDATALIST2Lib::_DNxDataListPtr m_dlFilter;
	// (j.jones 2011-07-06 09:58) - PLID 44451 - added "Other" status dropdown
	NXDATALIST2Lib::_DNxDataListPtr m_dlOtherStatusCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_dlApptRefPhys;	// (j.dinatale 2013-01-23 11:49) - PLID 54777
	// (j.jones 2013-07-15 15:09) - PLID 57477 - added chart & category filters
	NXDATALIST2Lib::_DNxDataListPtr m_dlChartCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_dlCategoryCombo;
	// (a.wilson 2014-02-28 12:17) - PLID 60780 - diagnosis search and list.
	NXDATALIST2Lib::_DNxDataListPtr m_dlDiagnosisSearch;
	NXDATALIST2Lib::_DNxDataListPtr m_dlDiagnosisList;

	HICON m_hIconPreview; // (a.walling 2010-01-11 12:11) - PLID 31482
	class CEMRPreviewPopupDlg* m_pEMRPreviewPopupDlg;

	// (a.walling 2010-01-11 12:52) - PLID 31482 - Show the emn preview popup
	// (z.manning 2012-09-10 16:06) - PLID 52543 - Added modified date
	void ShowPreview(long nPatID, long nEMNID, COleDateTime dtEmnModifiedDate);

	enum ListSentinelValues {
		lsvAllID = -3,
		lsvNoneID = -2,
		lsvMultipleID = -1,
		lsvNewID = lsvMultipleID,
	};

	CVariantArray m_vaProcedure;
	CVariantArray m_vaDiagnosis;
	CVariantArray m_vaProblems;
	CVariantArray m_vaProvider;
	CVariantArray m_vaSecProvider; // (z.manning 2008-11-12 14:44) - PLID 26167
	CVariantArray m_vaTechnician;	// (d.lange 2011-04-22 14:42) - PLID 43381 - Array of Assistant/Technician
	CVariantArray m_vaLocation;
	// (j.jones 2011-07-07 09:15) - PLID 44451 - added "Other" status option
	CVariantArray m_vaOtherStatus;
	// (j.dinatale 2013-01-24 09:39) - PLID 54777 - add appt ref phys
	CVariantArray m_vaApptRefPhys;
	// (j.jones 2013-07-15 14:49) - PLID 57477 - added chart & category filters
	CVariantArray m_vaChart;
	CVariantArray m_vaCategory;

	long m_nSelProcedure;
	long m_nSelProblems;
	long m_nSelProvider;
	long m_nSelSecProvider; // (z.manning 2008-11-12 14:46) - PLID 26167
	long m_nSelTechnician;	// (d.lange 2011-04-22 14:42) - PLID 43381 - Selected Assistant/Technician
	long m_nSelLocation;
	long m_nSelOtherStatus; // (j.jones 2011-07-06 10:27) - PLID 44451
	long m_nSelApptRefPhys;	// (j.dinatale 2013-01-24 09:39) - PLID 54777 - add appt ref phys
	// (j.jones 2013-07-15 14:49) - PLID 57477 - added chart & category filters
	long m_nSelChart;
	long m_nSelCategory;

	bool m_bAdvanced; // whether advanced controls are shown (dialog is not in patient seen today mode)
	bool m_bDialogInitialized;

	bool m_bNoStatus; // this variable is set to true only if no status checkbox is selected, which means "show nothing!"
						// we can then use this to display an informative message for the user.

	long m_nFilterID;
	CString m_strFilterString;
	int FilterEditorDlg();

	bool m_bSuppressSeenPatients; // this is set to true to prevent loading seen patients, which can be a performance issue for huge date ranges.
	bool m_bShowSeenPatientsWith; // (a.walling 2007-06-15 10:35) - PLID 24859 - Whether we show patients with EMNs.
	bool m_bExcludePending; // (a.walling 2011-01-13 12:03) - PLID 41542
	// (j.jones 2014-05-15 16:40) - PLID 61798 - renamed the existing refresh checkbox to clearly
	// state it only applies to filters, and created a new checkbox for refreshing upon window restore
	bool m_bRefreshOnFilterChange;
	bool m_bRefreshOnWindowRestore;
	bool m_bRememberColumns;

	bool m_bDateFromDown;
	bool m_bDateToDown;

	bool m_bProcedureAny;

	// (b.spivey, January 29, 2013) - PLID 48370 - track if we're showing just the current patient only. 
	bool m_bShowCurrentPatientOnly; 

	NXDATALIST2Lib::_DNxDataListPtr m_dlEMNList;

	bool OpenMultiSelectList(NXDATALIST2Lib::_DNxDataListPtr &list, IN OUT CVariantArray &va, IN OUT long &nCurSel, const IN CString &strFieldID, const IN CString &strFieldValue, const IN CString &strDescription, IN CStringArray *straryExtraColumnFields = NULL, IN CStringArray *straryExtraColumnNames = NULL);
	void RequeryLists();
	void SetListDefaults();
	CString GenerateFromClause(); // (z.manning, 02/29/2008) - PLID 29158
	// (z.manning 2008-06-12 15:59) - PLID 29380 - Added optional bSkipSeenWithCheck parameter
	// (j.gruber 2013-06-10 11:20) - PLID 43442 - add optional Date Only
	CString GenerateWhereClause(OPTIONAL bool bSkipSeenWithCheck = false, OPTIONAL bool bDateFilterOnly = false);
	CString GenerateDateRange();
	// (z.manning, 02/29/2008) - PLID 29158 - Added an overloaded GenerateDateRange where you can manually specify
	// the sql field that you want to use.
	CString GenerateDateRange(IN const CString strSqlFieldName);
	CString GenerateSeenSql(bool bForReport = false);
	void ShowAdvanced(bool bShow);
	void WarnSeenPatients(); // set m_bSuppressSeenPatients if they decide not to display them. Used for large date ranges.

	void SaveColumns();
	void RestoreColumns();

	void GotoPatient(long nPatID);

	// (a.walling 2007-06-28 10:54) - PLID 26353 - We are keeping the from clause locally
	CString m_strWhere;
	CString m_strFrom;

	// (j.jones 2008-09-30 17:30) - PLID 28011 - added function to minimize this window
	void MinimizeWindow();

	// (j.jones 2014-05-15 17:25) - PLID 62171 - we now cache the current EMN and next EMN,
	// for potential re-selection after a refresh
	long m_nCurEMNID;
	long m_nNextEMNID;
	long m_nTopRowEMNID;

	// Generated message map functions
	//{{AFX_MSG(CEMRSearch)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnDatefilterQuick();
	afx_msg void OnClose();
	afx_msg void OnSelChosenListProcedure(LPDISPATCH lpRow);
	afx_msg void OnSelChosenListProblems(LPDISPATCH lpRow);
	afx_msg void OnSelChosenListProvider(LPDISPATCH lpRow);
	afx_msg void OnSelChosenListLocation(LPDISPATCH lpRow);
	afx_msg void OnEditFilter();
	afx_msg void OnSelChosenListEmrfilter(LPDISPATCH lpRow);
	afx_msg void OnRememberColumns();
	// (j.jones 2014-05-15 16:40) - PLID 61798 - renamed the existing refresh checkbox to clearly
	// state it only applies to filters, and created a new checkbox for refreshing upon window restore
	afx_msg void OnCheckRefreshOnFilterChange();
	afx_msg void OnCheckRefreshWhenWindowRestored();
	afx_msg void OnSelChosenDatefilterType(LPDISPATCH lpRow);
	afx_msg void OnChangeDateFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeDateTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropDownDateFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpDateFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCloseUpDateTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDropDownDateTo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnStatusOpen();
	afx_msg void OnStatusFinished();
	afx_msg void OnStatusLocked();
	afx_msg void OnCancel();
	afx_msg void OnShowAdvanced();
	afx_msg void OnEmrSearchRefresh();
	afx_msg void OnRButtonDownEmnlist(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnLeftClickEmnlist(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnRequeryFinishedEmnlist(short nFlags);
	afx_msg void OnEmrsearchPreview();
	afx_msg void OnShowSeenPatients();
	afx_msg void OnShowSeenpatientsWith();
	afx_msg void OnExcludePending();
	afx_msg void OnSelChosenListSecProvider(LPDISPATCH lpRow);
	afx_msg void OnSelChosenListTechnician(LPDISPATCH lpRow);
	// (a.walling 2010-01-11 12:25) - PLID 31482	
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedShowCurrentPatient(); // (z.manning 2009-05-19 15:51) - PLID 28512
	// (j.jones 2011-07-06 09:58) - PLID 44451 - added "Other" status option
	afx_msg void OnStatusOther();
	// (j.jones 2011-07-06 09:58) - PLID 44451 - added "Other" status option
	void OnSelChosenListOtherStatus(LPDISPATCH lpRow);
	// (j.jones 2011-07-07 10:17) - PLID 44451 - added "Other" status option
	void OnRequeryFinishedListOtherStatus(short nFlags);
	afx_msg void OnBnClickedEmrsearchSignSelected(); // (z.manning 2011-10-28 15:36) - PLID 44594
	// (b.spivey, January 23, 2013) - PLID 48370 - Load previous filters and the member variable arrays. 
	void LoadPreviousFilters(); 
	void FillVariantArrayFromString(CVariantArray &vary, CString strValues);
	void SelChosenListApptRefPhys(LPDISPATCH lpRow);
	// (j.jones 2013-07-15 14:49) - PLID 57477 - added chart & category filters
	afx_msg void OnSelChosenComboChart(LPDISPATCH lpRow);
	afx_msg void OnSelChosenComboCategories(LPDISPATCH lpRow);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// (a.wilson 2014-02-28 13:13) - PLID 60780 & 60873
	void SelChosenEmrSearchDiagnosisSearch(LPDISPATCH lpRow);
	void RButtonDownEmrSearchDiagnosisList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnBnClickedEmrSearchDiagnosisNone();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMRSEARCH_H__2833B09F_AD0E_4E6B_8631_97BB92E33C40__INCLUDED_)

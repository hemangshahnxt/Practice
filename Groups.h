#include "Client.h"
#if !defined(AFX_GROUPS_H__040A62C3_0186_11D3_9447_00C04F4C8415__INCLUDED_)
#define AFX_GROUPS_H__040A62C3_0186_11D3_9447_00C04F4C8415__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Groups.h : header file
//
#include "FilterFieldInfo.h"
/////////////////////////////////////////////////////////////////////////////
// CGroups dialog

#define LWL_GROUP_LIST		0x01
#define LWL_FILTER_LIST		0x02
#define LWL_ALL_LISTS		LWL_GROUP_LIST|LWL_FILTER_LIST

#define GROUP_ID_CURRENT_PATIENT		0
#define GROUP_ID_NEW					-1
#define GROUP_ID_SAVE_CANCELED			-2
#define GROUP_ID_UNSPECIFIED			-3
#define GROUP_ID_CURRENT_LOOKUP			-4
#define GROUP_ID_CURRENT_FILTER			-5

#define FILTER_ID_ALL					0
#define FILTER_ID_NEW					-1
#define FILTER_ID_SAVE_CANCELED			-2
#define FILTER_ID_UNSPECIFIED			-3
#define FILTER_ID_TEMPORARY				-4


#define NXT_CHECK_CURRENT_PATIENT	101

enum FilterBasedOnEnum {
	fboPerson			= 0x0001,
	fboEMN				= 0x0002,	
	fboAppointment		= 0x0003,
	fboTodo				= 0x0004,
	fboEMR				= 0x0005,
	fboPayment			= 0x0006,
	fboLabResult		= 0x0007,	//TES 9/9/2010 - PLID 40457
	fboImmunization		= 0x0008,	//TES 9/9/2010 - PLID 40470
	fboEmrProblem		= 0x0009,	//TES 9/9/2010 - PLID 40471
	fboMedication		= 0x000A,   //r.wilson 59121 - Patient Lists Test
	fboAllergy  		= 0x000B,   //r.wilson 59121 - Patient Lists Test
	fboLab				= 0x000C,   //r.wilson 59121 - Patient Lists Test
};

enum ExistingFilterListColumns {
	eflcID = 0,
	eflcName = 1,
};

enum ExistingGroupListColumns {
	eglcID = 0,
	eglcName = 1,
};

class CGroups : public CNxDialog
{
// Construction
public:
	CGroups(CWnd* pParent);   // standard constructor

	CTableChecker m_groupChecker;
	CTableChecker m_filterChecker;

	virtual int SetControlPositions();
	// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	virtual void UpdateView(bool bForceRefresh = true);//used for refreshing of the module and requeries on table checker messages

	// (z.manning, 04/25/2008) - PLID 29795 - Added NxIconButtons
// Dialog Data
	//{{AFX_DATA(CGroups)
	enum { IDD = IDD_GROUPS };
	NxButton	m_btnRememberColumns;
	CNxIconButton	m_removeAllBtn;
	CNxIconButton	m_removeBtn;
	CNxIconButton	m_addAllBtn;
	CNxIconButton	m_addBtn;
	CNxStatic	m_nxstaticLabel;
	CNxStatic	m_nxstaticAvailtext;
	CNxStatic	m_nxstaticLabel3;
	CNxStatic	m_nxstaticAvailcount;
	CNxStatic	m_nxstaticSelcount;
	CNxStatic	m_nxstaticReposition;
	CNxIconButton	m_btnNewFilter;
	CNxIconButton	m_btnEditFilter;
	CNxIconButton	m_btnSaveFilter;
	CNxIconButton	m_btnDeleteFilter;
	CNxIconButton	m_btnNewGroup;
	CNxIconButton	m_btnSaveGroup;
	CNxIconButton	m_btnDelete;
	//}}AFX_DATA

	//TES 2/22/2007 - PLID 20642 - Replaced the simple combo for the filter and group lists with a datalist2.
	NXDATALIST2Lib::_DNxDataListPtr m_pFilterList;
	NXDATALIST2Lib::_DNxDataListPtr m_pGroupList;

	NXDATALISTLib::_DNxDataListPtr m_selected;
	NXDATALISTLib::_DNxDataListPtr m_unselected;

	NXDATALISTLib::_DNxDataListPtr m_dlTypeOfPerson;

	bool m_bCurrentFilterGroupNeedsRefresh;

	static CString GetDefaultWhereClause(long nBaseJoinFlags, long fboFilterBase);

	//callback functions, passed to filters (should be in filterdetailcallback)
	static BOOL WINAPI IsActionSupported(SupportedActionsEnum saAction, long nFilterType);
	static BOOL WINAPI CommitSubfilterAction(SupportedActionsEnum saAction, long nFilterType, long &nID, CString &strName, CString &strFilter, CWnd *pParentWnd);
	static BOOL WINAPI CheckAllowDeleteFilter(long nDeleteFilterId);


	static const TCHAR *CalcInternationalMonthList();

	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGroups)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void OnSelectTab(short newTab, short oldTab);//used for the new NxTab
	//}}AFX_VIRTUAL

// Implementation
protected:
	//TES 3/15/2007 - PLID 24895 - These two variables no longer seem to serve any useful purpose.
	//BOOL m_bFilterAccess;
	//BOOL m_bGroupAccess;

	long m_nStyle;
	CString m_strGroupList; // Comma-delimited list of selected IDs (this may not be useful at all)
	CString GetStyleName(long nStyle, BOOL bSingular);
	void ReflectListCount(UINT nList);
	void UpdateToolBars(long nStyle);
	// (j.jones 2016-04-15 09:16) - NX-100214 - no longer need a brush
	//CBrush m_brush;
	// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
	NxTab::_DNxTabPtr m_tab;

	BOOL m_bAppointTabLoaded; // (z.manning 2013-07-17 11:31) - PLID 57609

	CString BuildBaseFromClause(long nBaseJoinFlags);
	void SetSelectedColumnSizes();
	void SetUnselectedColumnSizes();
	void SaveSelectedColumnSizes();
	void SaveUnselectedColumnSizes();
	void SetApptSelectedColumnSizes();
	void SetApptUnselectedColumnSizes();
	void SaveApptSelectedColumnSizes();
	void SaveApptUnselectedColumnSizes();
	void ChangeColumnStyle(NXDATALISTLib::IColumnSettingsPtr pCol, long csStyle);

	void OnPersonAdd();
	void OnPersonAddAll();
	void OnPersonRemove();
	void OnPersonRemoveAll();

	CString CreateColumnSizeList(NXDATALISTLib::_DNxDataListPtr pDataList);
	void SetColumnSizes(CString &strSizesList, NXDATALISTLib::_DNxDataListPtr pDataList);
	
	// Generated message map functions
	//{{AFX_MSG(CGroups)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnAddAll();
	afx_msg void OnRemove();
	afx_msg void OnRemoveAll();
	afx_msg void OnDblClickCellSelected(long nRow, short nCol);
	afx_msg void OnDblClickCellUnselected(long nRow, short nCol);
	afx_msg void OnEditFilters();
	afx_msg void OnSaveGroup();
	afx_msg void OnDeleteGroup();
	afx_msg void OnDeleteFilter();
	afx_msg void OnNewGroupBtn();
	afx_msg void OnNewFilterBtn();
	afx_msg void OnSaveFiltersBtn();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnRequeryFinishedSelected(short nFlags);
	afx_msg void OnRequeryFinishedUnselected(short nFlags);
	afx_msg void OnSelChosenPersonTypeCombo(long nRow);
	// (j.jones 2016-04-15 09:16) - NX-100214 - no longer need OnCtlColor
	//afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnColumnSizingFinishedSelected(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	afx_msg void OnColumnSizingFinishedUnselected(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth);
	afx_msg void OnRememberColumns();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnInvalidFilter(WPARAM wParam, LPARAM lParam);
	afx_msg void OnApptSelectorNew();
	afx_msg void OnApptSelectorEdit();
	afx_msg void OnDblClickCellApptSelected(long nRow, short nCol);
	afx_msg void OnDblClickCellApptUnselected(long nRow, short nCol);
	afx_msg void OnRequeryFinishedApptSelected(short nFlags);
	afx_msg void OnRequeryFinishedApptUnselected(short nFlags);
	afx_msg void OnFilterEditorNew();
	afx_msg void OnFilterEditorEdit();
	afx_msg void OnSelChosenExistingFilterList(LPDISPATCH lpRow);
	afx_msg void OnSelChosenExistingGroupList(LPDISPATCH lpRow);
	afx_msg void OnSelChangingExistingFilterList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void OnSelChangingExistingGroupList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	//(c.copits 2011-09-13) PLID 43485 - "Go to Patient" right-click option in letter writing
	afx_msg void OnRButtonDownPatientUnselectedList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnGoToPatientUnselectedList();
	afx_msg void OnRButtonDownPatientSelectedList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnGoToPatientSelectedList();
	afx_msg void OnRButtonDownApptUnselectedList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnGoToApptUnselectedList();
	afx_msg void OnRButtonDownApptSelectedList(long nRow, short nCol, long x, long y, long nFlags);
	afx_msg void OnGoToApptSelectedList();
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// (m.hancock 2006-12-04 11:32) - PLID 21965 - Returns the index of the selected tab.
	long GetSelectedTab();

	/////////////////////////////////
	//////// Letter Writing 5.0
protected:
	// General Implementation Functions
	void RefreshCurrentListings(bool bIncludeGroupListing = true, bool bForceFilterListing = false);
	bool OnSelectedChanged();
protected:
	// General Implementation Data
	long m_nCurSelectionId;

public:
	// General Interface Functions
	bool DoInitDialog(long nInitAsStyle, long nChangeToFilterId = FILTER_ID_UNSPECIFIED, long nChangeToGroupId = GROUP_ID_UNSPECIFIED);
	long CheckAllowClose();
	void PreClose();
	void DoRefresh(); // Not using "Refresh" because NxDialog uses it differently

	///////////////////////////////
	//////// Groups 5.0 
protected:
	// Implementation Functions
	UINT FillGroupCombo(long nSelectId);
	bool ChangeGroupSelection(long nGroupId, bool bAutoRefreshListings = true);
	void RefreshGroupAccess(long nGroupId);
	void SetCurrentGroupModified(bool bModified = true);
	bool IsCurrentGroupModified();
	long DoSaveCurrentGroup(long nSaveGroupId, long nSelectDifferentId = GROUP_ID_UNSPECIFIED);
	bool DoDeleteGroup(long nDeleteGroupId);
	long GetGroupIdLast();
	void SetGroupIdLast(long nGroupId);
	void EnableGroupAccess(BOOL bEnable);
	void SetColumnWidths();
protected:
	// Implementation Data
	//TES 2/22/2007 - PLID 20642 - Now that this is a datalist, we store the ID rather than the index.
	long m_nCurGroupID;;
	bool m_bGroupModified;
public:
	// Exported Functions
	static bool IsUserDefinedGroup(long nGroupId);
	static bool IsGroupReadOnly(long nGroupId);
	CString GetGroupName(long nGroupId);

	//////////////////////////
	/////// Filters 5.0 
protected:
	// Implementation Functions
	UINT FillFilterCombo(long nSelectId, LPCTSTR strUnsavedString = NULL);
	bool ChangeFilterSelection(long nFilterId, bool bAutoRefreshListings = true);
	void RefreshFilterAccess(long nFilterId);
	long DoSaveCurrentFilter(long nSaveFilterId, long nSelectDifferentId = FILTER_ID_UNSPECIFIED);
	bool DoDeleteFilter(long nDeleteFilterId);	
	long GetFilterIdLast(BOOL bAppointmentFilter = FALSE);
	void SetFilterIdLast(long nFilterId, BOOL bAppointmentFilter = FALSE);
	void EnableFilterAccess(BOOL bEnable);
protected:
	// Implementation Data
	//TES 2/22/2007 - PLID 20642 - Store the ID rather than the index, now that we have a datalist instead of a MS Combo.
	long m_nCurFilterID;
	CString m_strUnsavedFilterString;
public:
	// Exported Functions
	static bool IsUserDefinedFilter(long nFilterId);

	////////////////////////
	////// Appointment Based Merging
public:
	BOOL m_bAppointmentMerge;

	NXDATALISTLib::_DNxDataListPtr m_apptSelected;
	NXDATALISTLib::_DNxDataListPtr m_apptUnselected;


protected:
	void LoadAppointmentDataList();

	void OnApptAdd();
	void OnApptAddAll();
	void OnApptRemove();
	void OnApptRemoveAll();

	int FilterEditorDlg(BOOL bNewFilter = FALSE);

	void RepositionControls();

	void SetListCountToLoading();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GROUPS_H__040A62C3_0186_11D3_9447_00C04F4C8415__INCLUDED_)

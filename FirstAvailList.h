#if !defined(AFX_FIRSTAVAILLIST_H__F5CA77F4_1631_4D8A_A7C6_B9938296CF57__INCLUDED_)
#define AFX_FIRSTAVAILLIST_H__F5CA77F4_1631_4D8A_A7C6_B9938296CF57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FirstAvailList.h : header file
//

#include "FirstAvailableAppt.h" // (r.gonet 2014-11-19) - PLID 64174 - Ugh. I hate including headers in headers but adding forward declarations in
								// this case would have been messy, duplicated, and the benefit not that great.
#include "CommonFFAUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CFirstAvailList dialog

struct VisibleTemplate;
struct VisibleAppointment;

class CFirstAvailList : public CNxDialog
{
// Construction
public:
	CFirstAvailList(CWnd* pParent);   // standard constructor
	~CFirstAvailList(); // (z.manning, 02/27/2007) - PLID 24969

	// (a.walling 2007-05-04 09:55) - PLID 4850 - Called when the user switches. Refresh user settings.
	void OnUserChanged();

public:
	// (r.gonet 2014-12-17) - PLID 64465 - Replaced the member variables that were FFA search settings with
	// one CFFASearchSettings object.
	// (c.haag 2008-04-24 15:07) - PLID 29776 - The search progress dialog now
	// has its own class so that we can make the Cancel button a CNxIconButton

	// (z.manning 2010-11-01 15:58) - PLID 41272 - This is now a member function
	// (j.gruber 2011-05-11 16:04) - PLID 41131 - added tempalte list
	// (z.manning 2013-11-19 10:27) - PLID 58756 - Redid the params here now that the FFA search is done in the API
	// (j.politis 2015-07-09 12:35) - PLID 65629 - Add a new “Location” column in the results list after the “Resources” column.
	// (b.cardillo 2016-01-31 14:08) - PLID 65630 - We don't try to use location color anymore.
	BOOL TryAddApptToList(COleDateTime dt, const CString &strResourceIDs, const CString &strResourceNames, const CString &strLocationName, long nLocationID, const CString &strTemplateNames);

	//TES 1/7/2015 - PLID 64466 - If m_bReturnSlot is true, then rather than scheduling an appointment, we will fill in the values for m_UserSelectedSlot, and 
	// then post a NXM_ON_CLOSE_FIRST_AVAIL_LIST to m_pCallingDlg, with ourselves as the wParam
	bool m_bReturnSlot;
	bool m_bReSchedule;
	long m_nNewResID;

	SelectedFFASlotPtr m_UserSelectedSlot;
	CFirstAvailableAppt *m_pCallingDlg;

// Dialog Data
	//{{AFX_DATA(CFirstAvailList)
	enum { IDD = IDD_FIRST_AVAIL_LIST };
	CNxIconButton	m_btnScheduleEdit;
	CNxIconButton	m_btnSchedule;
	CNxIconButton	m_btnScheduleKeepOpen; // (z.manning, 02/21/2007) - PLID 23906
	CNxIconButton	m_btnNewSearch;
	CNxIconButton	m_btnGoSelectedDay;
	CNxIconButton	m_btnCancel;
	NxButton	m_checkGoSelectedDay;
	//}}AFX_DATA

private:
	// (r.gonet 2014-12-17) - PLID 64465 - Added to encapsulate the FFA search settings. We reuse this elsewhere.
	CFFASearchSettingsPtr m_pSearchSettings;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFirstAvailList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OnDisplayWindow();
	void OnHideWindow();
	void UpdateButtons();
	void ConfigureResourceControls(); // (r.farnworth 2015-05-18 14:50) - PLID 65633
	// (r.farnworth 2015-05-19 10:55) - PLID 65633 - Created
	// (r.farnworth 2015-07-14 12:34) - PLID 65635 - Added LocationDropdownRow 
	// (j.politis 2015-07-13 17:13) - PLID 65637 - In the FFA results screen, add a checkbox that reads 'Only show time slots with location templates'.
	void FilterResultsList();
	void PopulateList();

	//TES 1/7/2015 - PLID 64466 - Pass in true to fill m_UserSelectedSlot rather than creating an appointment
	long TryScheduleAppt(bool bFillSlotOnly = false);

public:
	// (r.gonet 2014-12-17) - PLID 64465 - Sets the FFA search settings to be used.
	void SetSearchSettings(CFFASearchSettingsPtr pSearchSettings);
	void Clear();
	// (z.manning 2010-11-02 09:51) - PLID 41272 - Moved the logic to stop the thread out of Clear into its own function.
	void StopPopulateThread();
	// (r.farnworth 2015-05-18 16:02) - PLID 65633 - Dropdown in FFA results to filter by resource. The dropdown list will contain only resources referenced in any results, alphabetical by name, plus < All Resources > at the top
	void OnSelChosenResourceListCombo(LPDISPATCH lpRow);
	void OnSelChangingUnselectedResource(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	// (r.farnworth 2015-06-05 10:59) - PLID 65635 - Dropdown in FFA results to filter by location. The dropdown list will contain only locations referenced in any results, alphabetical by name, plus < All Locations > at the top
	void OnSelChosenLocationListCombo(LPDISPATCH lpRow);
	void OnSelChangingUnselectedLocation(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	// (v.maida 2016-02-18 10:17) - PLID 68368 - Made below row visibility functions part of the FirstAvailList class.
	// (v.maida 2016-03-14 17:03) - PLID 68448 - Removed bOnlyWithLocation parameter, since the API now handles filtering or not filtering location-less results.
	bool ShouldRowBeVisible_ByResource(long nSelectedResourceID, const CDWordArray &aryDataListIDs);
	bool ShouldRowBeVisible_ByLocation(long nSelectedLocationID, const _variant_t &varLocationID);
	bool ShouldRowBeVisible(long nSelectedResourceID, const CDWordArray &aryDataListIDs, long nSelectedLocationID, const _variant_t &varLocationID);


protected:
	// (r.gonet 2014-12-17) - PLID 64465 - A thread that performs the FFA search. It will notify this dialog
	// about progress through posting messages.
	shared_ptr<CFFASearchThread> m_pFFASearch;
	// (d.thompson 2010-11-01) - PLID 41274 - Progress bar is now on the dialog
	CProgressCtrl m_ctrlProgressBar;
	CNxStatic m_nxstaticProgressText;
	// (d.thompson 2010-11-02) - PLID 41284 - Converted dl1 to dl2
	NXDATALIST2Lib::_DNxDataListPtr m_pResults;
	// (r.farnworth 2015-05-18 10:34) - PLID 65634 - Left/right buttons to filter results by resource in FFA results.
	CNxIconButton	m_resourceLeftButton;
	CNxIconButton	m_resourceRightButton;
	// (r.farnworth 2015-06-05 12:46) - PLID 65636 - Left/right buttons to filter results by location in FFA results.
	CNxIconButton	m_locationLeftButton;
	CNxIconButton	m_locationRightButton;
	NXDATALIST2Lib::_DNxDataListPtr  m_FilterResourceCombo; // (r.farnworth 2015-05-18 15:59) - PLID 65633
	NXDATALIST2Lib::_DNxDataListPtr  m_FilterLocationCombo; // (r.farnworth 2015-06-05 11:01) - PLID 65635
	// (v.maida 2016-02-24 15:07) - PLID 68385 - Store current resource and location selections.
	long m_nCurrentLocation;
	long m_nCurrentResource;

protected:
	// Generated message map functions
	//{{AFX_MSG(CFirstAvailList)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual void OnGoSelectedDay();
	virtual void OnNewSearch();
	virtual void OnBtnSchedule();
	virtual void OnBtnScheduleKeepOpen(); // (z.manning, 02/22/2007) - PLID 23906
	virtual void OnBtnScheduleEdit();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnMoveResourceLeft();
	afx_msg void OnMoveResourceRight();
	afx_msg void OnMoveLocationLeft();
	afx_msg void OnMoveLocationRight();
	afx_msg void OnDoubleClickResourceLeft();
	afx_msg void OnDoubleClickResourceRight();
	afx_msg void OnDoubleClickLocationLeft();
	afx_msg void OnDoubleClickLocationRight();
	afx_msg void OnCheckGoSelectedDay();
	afx_msg LRESULT OnFFAListRequeryFinished(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFFAListProcessData(WPARAM wParam, LPARAM lParam); // (z.manning 2010-11-01 15:53) - PLID 41272
	afx_msg LRESULT OnSetFFAProgressMinMax(WPARAM wParam, LPARAM lParam);		// (d.thompson 2010-11-01) - PLID 41274
	afx_msg LRESULT OnSetFFAProgressPosition(WPARAM wParam, LPARAM lParam);			// (d.thompson 2010-11-01) - PLID 41274
	afx_msg LRESULT OnSetFFAProgressText(WPARAM wParam, LPARAM lParam);			// (d.thompson 2010-11-01) - PLID 41274
	void OnSelChangedFFAResultsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);			// (d.thompson 2010-11-02) - PLID 41284
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIRSTAVAILLIST_H__F5CA77F4_1631_4D8A_A7C6_B9938296CF57__INCLUDED_)

#if !defined(AFX_FIRSTAVAILABLEAPPT_H__CEA5E263_1BE2_11D2_B1EA_0000C0832801__INCLUDED_)
#define AFX_FIRSTAVAILABLEAPPT_H__CEA5E263_1BE2_11D2_B1EA_0000C0832801__INCLUDED_

#include "globaldatautils.h"
#include "CommonFFAUtils.h"

// (j.politis 2015-06-02 12:24) - PLID 65647 - Make the time anchors in FFA use ConfigRT instead of being hard-coded.
namespace ETimeAnchors
{
	enum _Enum
	{
		EarlyMorning = 0,
		LateMorning,
		EarlyAfternoon,
		LateAfternoon,
	};
}

// (r.gonet 2014-12-17) - PLID 64428 - Moved some defines for slot length to CommonFFAUtils.h

int GetSlotFromTime(const COleDateTime& dt, bool bIsEndTime = false);
int GetSlotFromTime(long nHours, long nMinutes, long nSeconds, bool bIsEndTime = false);
int GetLengthInSlots(long nHours, long nMinutes, long nSeconds);
COleDateTime GetTimeFromSlot(int nSlot);

// (r.gonet 2014-12-17) - PLID 64428 - Moved some map typedefs to CommonFFAUtils.h
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FirstAvailableAppt.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CFirstAvailableAppt dialog

// (r.gonet 2014-12-16) - PLID 64428 - Removed all template related checks. We now use
// the API's implementation of FFA in both the text-list mode and non-text list mode (previously
// only the text list mode used it and we had a separate implementation in Practice).

//(a.wilson 2012-3-27) PLID 49245 - create a struct for all the available overrides
//when constructing the FFA with preselects to override the defaults.
// (r.gonet 2014-11-19) - PLID 64173 - Added bInsurance to know when to load back the previous insurance value.
// (r.farnworth 2015-06-08 12:46) - PLID 65639 - Added Location
struct FFAOverride {
	bool bPatient, bUserEditedDuration, bResources, 
		bDurations, bDaysAvailable, bStartDate, bAppt, 
		bOfficeHours, bTimes, bInsurance, bLocation;

	void SetAllTo(bool bValue) {
		bPatient = bUserEditedDuration = bResources = bAppt = 
		bDurations = bDaysAvailable = bStartDate = bOfficeHours = 
		bTimes = bInsurance = bLocation = bValue;
	}
};

class CDurationGroup;
// (r.gonet 2014-12-17) - PLID 64428 - Forward declarations.
class CFFASearchSettings;
typedef shared_ptr<CFFASearchSettings> CFFASearchSettingsPtr;

//TES 1/13/2015 - PLID 64180 - Used to tell this dialog how the FirstAvailList was dismissed, when returning a slot
enum FirstAvailListCloseAction
{
	falcaOK,
	falcaCancel,
	falcaNewSearch,
};

class CFirstAvailableAppt : public CNxDialog
{
// Construction
public:	
	//(s.dhole 5/20/2015 10:49 AM ) - PLID 66144
	NXDATALIST2Lib::_DNxDataListPtr m_dlAptType, m_dlAptPurpose;
	// (j.politis 2015-05-18 17:02) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
	NXDATALIST2Lib::_DNxDataListPtr m_dlHours, m_dlMinutes;
	//(s.dhole 5/12/2015 2:48 PM ) - PLID 65619 update function to datalist 2
	//(s.dhole 5/13/2015 5:00 PM ) - PLID 65623 update to datalist 2
	NXDATALIST2Lib::_DNxDataListPtr  m_PatientCombo, m_Resources;
	NXDATALIST2Lib::_DNxDataListPtr  m_FFADefaultLocationCombo;
	// (r.farnworth 2015-05-13 11:28) - PLID 65625 - Add new dropdown for Appointment Location showing only general locations.
	NXDATALIST2Lib::_DNxDataListPtr  m_FFALocationCombo;
	CNxLabel	m_nxstaticMultiLocationLabel;
	//(s.dhole 5/13/2015 3:07 PM ) - 65621 default resource ID 
	NXDATALIST2Lib::_DNxDataListPtr  m_FFADefaultResourceCombo;
	// (r.gonet 2014-11-19) - PLID 64173 - Combo that holds all the patient's insured parties with primary category placement.
	NXDATALIST2Lib::_DNxDataListPtr m_dlInsuredPartyCombo;

	NXDATALIST2Lib::_DNxDataListPtr m_pAppHistoryList;
	// (z.manning 2010-10-28 10:28) - PLID 41129 - Added an enum for patient list
	enum EPatientComboColumns {
		pccID = 0,
		pccLast,
		pccFirst,
		pccMiddle,
		pccUserDefinedID,
		pccDOB,
		pccSSN,
		pccHealthCardNum,
		pccSecurityGroup,
	};
	// (v.maida 2016-01-25 11:09) - PLID 65643 - Removed functionality.
	//void RemoveAvailableDays();
	CString m_strLastResource;
	//(s.dhole 6/1/2015 4:43 PM ) - PLID 65643
	//BOOL IsDayAvailable(COleDateTime date, long nResourceID, CString strResource);
	//CString m_strDefaultRes;
	void SetGoStatus();
	void CalculateStartDate();
	void RequeryAptPurposes();
	//(s.dhole 6/2/2015 8:42 AM ) - PLID  65643 Remove functionality
	//CMapDateTimeToBool m_mapResDt;
	BOOL m_aResWeekdays[7];
	long m_lDefaultResource;

	CString m_strFirstSelectedResource; // CAH 9/28 - for the monthview

	//(e.lally 2011-05-16) PLID 41013 - Variables for preselecting values on a revised search 
	COleDateTime m_dtPreselectStart;
	long m_nPreselectPatientID;
	long m_nPreselectApptTypeID;
	long m_nPreselectApptPurposeID;
	// (r.gonet 2014-11-19) - PLID 64173 - The previous insured party that was selected. We'll select it again maybe when the dialog loads.
	long m_nPreselectInsuredPartyID;
	long m_nPreselectDurHours;
	long m_nPreselectDurMins;
	long m_nPreselectIntervalMins;
	long m_nPreselectResourceTypeSelection;//(s.dhole 5/22/2015 12:51 PM ) - PLID 65621
	CArray<long, long> m_aryPreselectLocations; // (r.farnworth 2015-06-08 12:44) - PLID 65639
	CArray<long, long> m_aryPreselectResources;
	CArray<long, long> m_aryPreselectWeekDays;
	CArray<long, long> m_aryPreselectTimes;
	CArray<long, long> m_arySelectedResources;//(s.dhole 5/20/2015 5:47 PM ) - PLID 66125
	BOOL m_bPreselectAnyResource;
	BOOL m_bPreselectOfficeHoursOnly;
	BOOL m_bPreselectAnyTimeChecked;
	BOOL m_bPreselectStartDateRadio;
	BOOL m_bPreselectUserEditedDuration;
	CArray<long, long> m_varLocIDs; // (r.farnworth 2015-05-13 15:57) - PLID  65625
	long m_nLastLocationID; // (r.farnworth 2015-05-14 15:57) - PLID 65625
	long m_nLastResourceID;	// (v.maida 2016-03-23 16:15) - PLID 65623
	BOOL m_bSelectAnyResource; //(s.dhole 5/27/2015 3:04 PM ) - PLID 65624
	//(a.wilson 2012-3-27) PLID 49245 - struct which will need passed over values.
	FFAOverride m_bDefaultOverrides;

	// (r.gonet 2014-12-17) - PLID 64428 - Removed template field names enum and member maps, which was associated with the old FFA implementation.
	COleDateTime m_dtStartDate;

	//TES 12/3/2014 - PLID 64180 - When set, the FFA will immediately run after loading any presets
	bool m_bRunImmediately;
	//TES 12/18/2014 - PLID 64466 - When set, the FFA will not create an appointment, but will fill in the information they selected in m_UserSelectedSlot
	bool m_bReturnSlot;
	bool m_bReSchedule;
	long m_nNewResID;
	SelectedFFASlotPtr m_UserSelectedSlot;

	//TES 1/7/2015 - PLID 64466 - Moved code to a shared function
	void LaunchFirstAvailList(bool bReturnSlot);

	CFirstAvailableAppt(CWnd* pParent);   // standard constructor
	virtual ~CFirstAvailableAppt();

// Dialog Data
	// (a.walling 2008-05-13 14:56) - PLID 27591 - Use DateTimePicker
	//{{AFX_DATA(CFirstAvailableAppt)
	enum { IDD = IDD_FIRST_AVAILABLE_APPT };
	CListBox	m_TimePrefs;
	int		m_boStartFromRadio;
	CDateTimePicker	m_dtDateCtrl;
	CString	m_strDaysAfter;
	//BOOL	m_boTextResults; //(s.dhole 6/1/2015 4:34 PM ) - PLID 65643 Remove
	int		m_iSearchInterval;
	int		m_iExtraBookingLimit;
	BOOL	m_boOfficeHoursOnly;
	CNxEdit	m_nxeditEditDaysAfter;
	CNxEdit	m_nxeditEditSearchInterval;
	CNxEdit	m_nxeditEditSearchDays;
	CNxEdit	m_nxeditEditHours;
	CNxEdit	m_nxeditEditMinutes;
	CNxStatic	m_nxstaticDaysafter;
	CNxStatic	m_nxstaticShowIn;
	CNxStatic	m_nxstaticSearchUpTo;
	CNxStatic	m_nxstaticDaysInFuture;
	CNxStatic	m_nxstaticMinintervals;
	CNxStatic	m_nxstaticSecintervals;
	CNxStatic	m_nxstaticNodaysafter;
	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnFFAApptHist;
	
	NxButton	m_btnDayGroupbox;
	NxButton	m_btnTimeGroupbox;
	NxButton	m_btnAdvSettingsGroupbox;
	NxButton	m_btnCriteriaGroupbox;
	
	//NxButton	m_checkResults;//(s.dhole 6/2/2015 9:04 AM ) - PLID 65643 Remove functionality
	NxButton	m_radioStartFrom;
	NxButton	m_radioStartPat;
	NxButton	m_checkExcludeWarnings;
	
	
	NxButton	m_checkMon;
	NxButton	m_checkTue;
	NxButton	m_checkWed;
	NxButton	m_checkThu;
	NxButton	m_checkFri;
	NxButton	m_checkSat;
	NxButton	m_checkSun;
	NxButton	m_checkAnyTime;
	NxButton	m_checkEarlyMorn;
	NxButton	m_checkLateMorn;
	NxButton	m_checkEarlyAftn;
	NxButton	m_checkLateAftn;
	NxButton	m_checkOfficeHoursOnly;
	NxButton	m_btnStartDayGroupbox;//(s.dhole 5/12/2015 2:48 PM ) - PLID 65619 
	CNxLabel	m_nxlUnselectedResourceLabel;
	CNxLabel	m_nxlIsResourcesUseAndFilterLabel; //(s.dhole 5/15/2015 9:31 AM ) - PLID 65624
	CNxLabel	m_nxlIsResourcesOFLabel;//(s.dhole 5/21/2015 10:59 AM ) - PLID 65624 	
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFirstAvailableAppt)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// (j.politis 2015-06-02 12:24) - PLID 65647 - Make the time anchors in FFA use ConfigRT instead of being hard-coded.
	bool IsAnchorSpanSet(ETimeAnchors::_Enum eAnchor);
	bool IsTimeSpanSet(int nFrom, int nTo);
	// (j.politis 2015-05-18 17:02) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
	void InitializeHoursDropdown();
	void InitializeMinsDropdown();
	long CalculateApptDuration(long nHours, long nMins);
	long AddDurationRows(NXDATALIST2Lib::_DNxDataListPtr pList, const std::initializer_list<_variant_t> &varAryValues);
	BOOL GotoAlternateNextDlgTabItem(BOOL bGoToPreviousCtrl);
	BOOL SearchOfficeHoursOnly();
	// (v.maida 2016-02-05 09:53) - PLID 68132 - Added bUseAllLocations
	void GetOfficeHours(long& dtStart, long& dtEnd, BOOL bUseAllLocations);
	// (r.gonet 2014-12-17) - PLID 64428 - Removed old FFA implementation related functions that checked scheduling templates.
	void EnsureInternationalInterface();
	void UpdateTimePrefCheckboxes();
	int GetDefaultArrivalMins();
	short GetCurAptTypeCategory();
	void UpdateWindowSize();
	void ShowMoreFields();
	void ShowLessFields();
	// (r.gonet 2014-12-17) - PLID 64428 - Removed ClearAllMaps(), which freed memory from scheduling template related maps, since removed.
	// (b.savon 2013-07-26 16:54) - PLID 50524 - Select the resources
	void SelectResources();
	void ToggleLocationLabel(); // (r.farnworth 2015-05-14 08:28) - PLID 65625
	// (v.maida 2016-02-05 09:30) - PLID 68132 - Added
	void ScrollToEarliestLocationOpeningTime();

	//We store the current patient ID here, if we store it in the patient datalist then we have to wait for that to 
	//finish requerying before we display.
	long m_nPatientID;
	// (r.farnworth 2015-06-08 10:47) - PLID 65635
	CArray <long, long> m_arynAptLocations;
	// Generated message map functions
	//{{AFX_MSG(CFirstAvailableAppt)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSelchangeListHours();
	afx_msg void OnSelchangeListMinutes();
	afx_msg void OnCheckEarlyMorn();
	afx_msg void OnCheckLateMorn();
	afx_msg void OnCheckEarlyAftn();
	afx_msg void OnCheckLateAftn();
	afx_msg void OnCheckOfficeHoursOnly();
	//afx_msg void OnCheckResults();//(s.dhole 6/2/2015 9:04 AM ) - PLID 65643 Remove functionality
	afx_msg void OnCheckTemplate();
	afx_msg void OnSelchangeListTimePrefs();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnCheckAnyTime();
	virtual BOOL OnInitDialog();
	//(s.dhole 5/20/2015 10:39 AM ) - PLID 66144 update function to datalist 2
	afx_msg void OnSelChosenApttypeCombo(LPDISPATCH lpRow);
	afx_msg void OnSelChosenAptpurposeCombo(LPDISPATCH lpRow);
	afx_msg void OnKillFocusSearchInterval();
	afx_msg void OnChangeEditHours();
	afx_msg void OnChangeEditMinutes();
	afx_msg void OnSelchangeListSeconds();
	afx_msg void OnHelp();
	
	afx_msg void OnShowExInfo();
	//(s.dhole 5/18/2015 2:56 PM ) - PLID 65627
	virtual void OnShowAppHistory();

	
	afx_msg LRESULT OnAutoRunFfa(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseFirstAvailList(WPARAM wparam, LPARAM lParam); //TES 1/7/2015 - PLID 64466
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message); // (r.farnworth 2015-05-14 08:56) - PLID 65625
	LRESULT OnLabelClick(WPARAM wParam, LPARAM lParam); // (r.farnworth 2015-05-14 08:56) - PLID 65625
	DECLARE_EVENTSINK_MAP() 
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	// (r.gonet 2014-11-19) - PLID 64173 - Added an enumeration for the insurance dropdown
	// (r.gonet 2014-12-10) - PLID 64174 - Added CategoryID of the RespType in order to pass it to the FFA results list later.
	enum class EInsuredPartyComboColumns {
		ID = 0,
		RespType,
		InsuranceCoID,
		InsuranceCoName,
		Priority,
		CategoryID,
		CategoryName,
	};

	//(s.dhole 6/2/2015 8:42 AM ) - PLID  65643 Remove functionality
	//CMap<CString, LPCTSTR, BOOL, BOOL> m_mapResources;
	void CreateResourceMaps(CFFASearchSettingsPtr pSettings);
	void ValidatePtFilter();
	void RequeryAptTypeCombo();

private:
	// (j.politis 2015-05-18 13:30) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
	bool GetDefaultSearchInterval(COleDateTimeSpan &dts);
	long GetSelectedAppointmentType();
	long GetSelectedAppointmentPurpose();
	void EnsureDurationGroups();
	void InvalidateDurationGroups();
	BOOL CalculateDefaultDuration(OUT COleDateTimeSpan& dts);
	void UpdateAppointmentLength();
	// (b.savon 2016-03-15 15:22) - PLID 65647 -- Pass in defaults and use preference
	void OnCheckAnchor(int nChecked, const CString &strFromProperty, const CString &strToProperty, byte defaultFrom, byte defaultTo);
	void SetHourSelected(int nHour, BOOL bToSelect);
	int GetHourSelected(int nHour);
	int GetHourIndex(int nHour);
	void ToggleResourcesSelections(); //(s.dhole 5/14/2015 11:38 AM ) - PLID  65623// (z.manning 2013-07-26 13:57) - PLID 32160
	// (r.gonet 2014-11-19) - PLID 64173 - Reload the insured party dropdown with the current patient's
	// insured parties. Useful when the dialog is first loaded and also when they switch patients.
	// bUsePreSelection causes us to try to select the remembered insured party from last time (in case
	// the user pressed Exit & New Search in the results list.
	void ReloadInsuredPartyCombo(bool bUsePreSelection);
	void ShowAdvOptions(BOOL bShow);
	// (r.farnworth 2016-01-25 17:08) - PLID 65638
	void RoundAppointmentDuration(long &nHours, long &nMinutes);
private:
	BOOL m_bUserEditedDuration;

	// The length of the appointment in intervals of 15 minutes,
	// only set after OnOK is called.
	int m_nAptLength;

	// The type of appointment we want to make; only set after
	// OnOK is called.
	int m_nAptType;

	// The purpose of appointment we want to make; only set after
	// OnOK is called.
	int m_nAptPurpose;

	// (r.gonet 2014-12-17) - PLID 64428 - Removed m_abAvailableSlots since it was only used with the old, removed FFA implementation.
	// (c.haag 2006-11-21 13:47) - PLID 23623 - Set to true if we are
	// showing the entire dialog, or false if we are just showing the
	// topmost buttons
	BOOL m_bShowingAllFields;

	//// (r.gonet 2014-12-17) - PLID 64428 - Stores the settings we are currently using to search FFA.
	//CFFASearchSettingsPtr m_pCurrentSettings;
	//// (r.gonet 2014-12-17) - PLID 64428 - Stores the settings we are originally used to search FFA (for use in the non-text list mode, which calls back into the CFirstAvailableAppt object for more searching).
	//CFFASearchSettingsPtr m_pOriginalSettings;
	//// (r.gonet 2014-12-17) - PLID 64428 - A lookup table that tells us if a resource is available for certain date in some time slot.
	//// It will be filled using the FFA results after searching using the m_pCurrentSettings search settings.
	//// Indexed by the date to check for open slots and the resource ID. An element's existence in the set means the
	//// resource is available on that date.
	//std::set<std::pair<COleDateTime, long> > m_setAvailability;

private:
	// (c.haag 2007-02-20 12:10) - PLID 24567 - The array of duration groups used to
	// calculate default durations
	CArray<CDurationGroup*, CDurationGroup*> m_aDurationGroups;

	// (c.haag 2007-02-20 12:06) - PLID 24567 - TRUE if we've populated m_aDurationGroups
	BOOL m_bDurationGroupsLoaded;
	//(e.lally 2011-05-16) PLID 41013 - clear out saved selections
	void ResetPreselects();
	//(s.dhole 5/12/2015 3:41 PM ) - PLID 65620
	void OnSelChosenDefaultLocationCombo(LPDISPATCH lpRow);
	//(s.dhole 5/12/2015 3:41 PM ) - PLID 65620 
	long GetDefaultLocationComboValue(NXDATALIST2Lib::IRowSettingsPtr pRow);

	//(s.dhole 5/13/2015 3:00 PM ) - PLID 65621 default resource ID
	void OnSelChosenDefaultResourceCombo(LPDISPATCH lpRow);
	void LoadDefaultResourceComboValue(NXDATALIST2Lib::IRowSettingsPtr pRow);
	//(s.dhole 5/14/2015 10:37 AM ) - PLID 65623
	void OnSelChosenUnselectedResource(LPDISPATCH lpRow);
	void OnSelChangingUnselectedResource(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel);
	//(s.dhole 5/15/2015 9:35 AM ) - PLID 65624
	void MangeResourceFilterOR_AND();
public:
	//(s.dhole 5/12/2015 2:48 PM ) - PLID 65619 update function to datalist 2
	void OnSelChosenPatientListCombo(LPDISPATCH lpRow);
	// (r.farnworth 2015-05-13 15:40) - PLID 65625 - Add new dropdown for Appointment Location showing only general locations.
	void OnSelChosenLocationListCombo(LPDISPATCH lpRow);
	// (r.gonet 2014-12-17) - PLID 64428 - Added. This function loads the availability lookup table for a given month.
	// - nYear - Year of the month to load availability for.
	// - nMonth - 1 based month number of the month to load availability for.
	// - bForceLoad - If the nYear and nMonth are the same as the m_pCurrentSettings, then 
	//   this function short circuits and won't reload. Passing true for bForceLoad forces the reload
	//   to happen..
	//(s.dhole 6/1/2015 4:39 PM ) - PLID 65643 commented this code
	//void LoadAvailabilityForMonth(long nYear, long nMonth, bool bForceLoad = false);
	//(s.dhole 5/14/2015 11:12 AM ) - PLID 65623
	BOOL  MultiResourcesSelections();
	void LoadPatientAppHistory(long nPatientId);
	void OnSelChosenAppHistoryCombo(LPDISPATCH lpRow);
	void OnClosedUpAppHistoryCombo(LPDISPATCH lpRow);
	void  LoadResourceIDStringIntoArray(CString strResourceIDs, CArray<long, long> &arIDs);
	// (j.politis 2015-05-18 13:30) - PLID 65626 - Change the existing How Long fields from two listboxes to two dropdowns.
	void OnSelChangedFfaHoursCombo(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void OnSelChangedFfaMinsCombo(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel);
	void OnSelectionsChanged();
	long GetDropdownHours();
	long GetDropdownMinutes();
	void SetDropdownHours(long Hours);
	void SetDropdownMinutes(long Minutes);
	void SetSearchIntervalAndUpdate(int SearchInterval, bool bDoInitialUpdate);
	//(s.dhole 5/20/2015 5:47 PM ) - PLID 66125
	long GetResourceComboSelectedValue();
	// (r.farnworth 2015-06-08 16:58) - PLID 65639
	long GetLocationComboSelectedValue();
	void LoadAnyResources(CArray<long, long> &arIDs);
	//(s.dhole 5/21/2015 10:54 AM ) - PLID 65624
	void HandelMultiResourceLabel(BOOL bShow);
	void SetResourceComboToAnyResource(CArray<long, long> &arIDs); //(s.dhole 5 / 22 / 2015 12:51 PM) - PLID 65621
	void SetResourceComboToSchedulerResource(CArray<long, long> &arIDs); // (v.maida 2016-03-21 08:57) - PLID 65621 - Created
	long GetCurrentSchedulerResource(); // (v.maida 2016-03-21 08:57) - PLID 65621 - Created

	// (b.cardillo 2016-02-03 16:12) - PLID 65627 (supplemental) - Removed separator bar, just made past appts grey
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIRSTAVAILABLEAPPT_H__CEA5E263_1BE2_11D2_B1EA_0000C0832801__INCLUDED_)


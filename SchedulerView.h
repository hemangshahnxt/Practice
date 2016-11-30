#if !defined(AFX_SCHEDULERVIEW_H__8D6FB325_B8E2_11D1_B2DD_00001B4B970B__INCLUDED_)
#define AFX_SCHEDULERVIEW_H__8D6FB325_B8E2_11D1_B2DD_00001B4B970B__INCLUDED_

// (a.walling 2008-04-10 12:52) - tab.h is no longer used
// (a.walling 2008-09-18 17:02) - PLID 28040 - Ditto with SchedulerUpdateThread

// (j.jones 2013-05-07 14:03) - PLID 53969 - removed the .h files for the child tabs
class COneDayDlg;
class CWeekDayDlg;
class CMultiResourceDlg;
class CMonthDlg;
class CMoveUpListDlg;
class CLocationOfficeHours;
class CNxSchedulerDlg;
class CReservation;

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and ResolveDefaultTab to the modules code

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SchedulerView.h : header file
//

#include "CommonSchedUtils.h"

// (a.walling 2007-08-31 10:18) - PLID 27265 - Helper macro for the m_pResDispatch
#define IS_VALID_DISPATCH(pDisp)	( (pDisp != NULL) && (pDisp != (LPDISPATCH)-1) )

#define TIME_BUTTON_PRINT_WIDTH_PERCENT 0.068

#define ID_RES_EDIT_TEMPLATES				32785
// (j.jones 2011-07-15 14:47) - PLID 39838 - created ID_RES_EDIT_LOCATION_TEMPLATES
#define ID_RES_EDIT_LOCATION_TEMPLATES		32777
// (z.manning 2014-12-01 17:34) - PLID 64205 - Added ID_RES_EDIT_TEMPLATE_COLLECTIONS
#define ID_RES_EDIT_TEMPLATE_COLLECTIONS	32778

#define SCHEDULER_TEXT_APPOINTMENT_TYPE__NO_TYPE				" { No Appointment Type }"
#define SCHEDULER_TEXT_APPOINTMENT_PURPOSE__NO_PURPOSE			" { No Purpose } "
#define SCHEDULER_TEXT_APPOINTMENT_PURPOSE__MULTIPLE_PURPOSES	" { Multiple Purposes } "
#define SCHEDULER_TEXT_RESOURCE__MULTIPLE_RESOURCES				" { Multiple Resources } "
#define SCHEDULER_TEXT_PATIENT__NO_PATIENT						" { No Patient Selected } "

#define SCHEDULER_TEXT_FILTER__ALL_TYPES						" { Show All Types } "
#define SCHEDULER_TEXT_FILTER__ALL_PURPOSES						" { Show All Purposes } "
#define SCHEDULER_TEXT_FILTER__MULTI_TYPES						" { Multiple Types } "
#define SCHEDULER_TEXT_FILTER__MULTI_PURPOSES					" { Multiple Purposes } "

//TES 6/21/2010 - PLID 21341
#define SCHEDULER_TEXT_FILTER__ALL_LOCATIONS					" { Show All Locations } "
// (d.singleton 2012-06-07 09:48) - PLID 47473
#define SCHEDULER_TEXT_FILTER__MULTI_LOCATIONS					" { Multiple Locations } "

void GetApptCountForResource(long nMasterItemId, OUT long &nApptCount);

enum EFindElementInArrayResult {
	feiarNotFound = -1,
};

long FindElementInArray(const CDWordArray &ary, const DWORD dwElement);

///////////////////////////////////////////////////////////////////////////////
// The NXSINGLEDAYPRINT structure below is shared between Practice.exe and 
// SingleDay.ocx; ideally they should both #include the same file, but for 
// now please just make sure that if you make changes to it here, you make 
// corresponding changes in SingleDay.ocx
struct NXSINGLEDAYPRINT
{
public:
	NXSINGLEDAYPRINT() { bPrintBlockColor = FALSE; bPrintBlockText = FALSE; bPrintGrid = FALSE;};

public:
	BOOL bPrintBlockColor;
	BOOL bPrintBlockText;
	BOOL bPrintGrid;
};
///////////////////////////////////////////////////////////////////////////////

// (j.jones 2013-05-07 14:08) - PLID 53969 - moved CLocationOfficeHours to globalschedutls

class CResourceEntry
{
public:
	CResourceEntry() 
		: m_nResourceID(-1), m_strResourceName("") { };
	
	CResourceEntry(long nResourceID, const CString &strResourceName, long nDefaultInterval)
		: m_nResourceID(nResourceID), m_strResourceName(strResourceName), m_nDefaultInterval(nDefaultInterval) { };

	CResourceEntry(const CResourceEntry &reFrom)
		: m_nResourceID(reFrom.m_nResourceID), m_strResourceName(reFrom.m_strResourceName), m_nDefaultInterval(reFrom.m_nDefaultInterval) { };

public:
	CResourceEntry &operator =(const CResourceEntry &reFrom) { m_nResourceID = reFrom.m_nResourceID; m_strResourceName = reFrom.m_strResourceName; m_nDefaultInterval = reFrom.m_nDefaultInterval; return *this; };

public:
	long m_nResourceID;
	CString m_strResourceName;
	long m_nDefaultInterval;
};

class CResourceEntryPtrArray : public CArray<const CResourceEntry *, const CResourceEntry *>
{
public:
	~CResourceEntryPtrArray() { RemoveAllAndFreeEntries(); };

public:
	void RemoveAllAndFreeEntries()
	{
		long nCount = GetSize();
		for (long i=0; i<nCount; i++) {
			delete GetAt(i);
		}
		CArray<const CResourceEntry *, const CResourceEntry *>::RemoveAll();
	}

	void RemoveAll(); // Intentionally not impelemnted because it doesn't deallocate the memory.  Call RemoveAllAndFreeEntries() instead.
};




// The CSchedViewColumnInfo class is used to store information about a certain column within the view.  For example, if you 
// were to get the view column info of the 3rd column of the view when the user is in the multi-resource tab you would 
// get the active date and the resourceid of resource whose appointments are showing in the 3rd column.  If the user 
// happened to be in the week view, you'd end up with the date whose appointments are showing in the 3rd column, and 
// the one active resource id.  The reason the resource id is given in an array is that even though we don't have any
// views that allow more than one resource in a given column, such a concept is not impossible, plus anything that 
// deals with resources at all already has to deal with multiple resources so we might as well try to be as forward-
// compatible as possible.
class CSchedViewColumnInfo
{
public:
	CSchedViewColumnInfo() { Clear(); };
	CSchedViewColumnInfo(const COleDateTime &dtDate, long nResourceID) { Set(dtDate, nResourceID); };

public:
	void Set(const COleDateTime &dtDate, long nResourceID) { m_dtDate = dtDate; m_nResourceID = nResourceID; m_bIsValid = TRUE; };
	void Clear() { m_bIsValid = FALSE; };

public:
	const COleDateTime &GetDate() const { if (m_bIsValid) { return m_dtDate; } else { ThrowNxException("CSchedViewColumnInfo::GetDate: Object is not set!"); } };
	long GetResourceID() const { if (m_bIsValid) { return m_nResourceID; } else { ThrowNxException("CSchedViewColumnInfo::GetResource: Object is not set!"); } };
	BOOL IsValid() const { if (m_bIsValid) { return TRUE; } else { return FALSE; } };

private:
	COleDateTime m_dtDate;
	long m_nResourceID;

private:
	BOOL m_bIsValid;
};


class CClipRes : public CClip
{
	DECLARE_DYNAMIC( CClipRes );
public:
	CClipRes() {m_nResID = -1; m_nResourceID = -1;};
	long m_nResID; //The id of the appointment put on the clipboard.
	long m_nResourceID; //The resource from which the appointment was put on the clipboard (the resource that was right-clicked on).
	void Clear() {m_nResID = -1; m_nResourceID = -1;};
	CClipRes(CClipRes &refObject) {m_nResID = refObject.m_nResID; m_nResourceID = refObject.m_nResourceID;};
	CClipRes &operator=(CClipRes &refObject) {m_nResID = refObject.m_nResID; m_nResourceID = refObject.m_nResourceID;};
};

/////////////////////////////////////////////////////////////////////////////
// CSchedulerView view

//To add an Extra Info option:
//1.) add an enum here.
//2.) in the resentry show dlg, add the name of your option to each of the combo boxes, in the position corresponding
//	  to the enum.
//3.) add a case statement to CResEntryDlg::GetExtraFieldText (you will probably also have to add a field to the
//    query in CResEntryDlg::CreatePatientInfoRecordset).
//4.) add a case statement to CResEntryDlg::FormatTextForReadset.
//5.) add a case statement to CReservationReadSet::GetExtraInfo (you will probably need to modify 
//    resextendedq, sorry )

enum ExtraFields {
	efID = 0,
	efHomePhone = 1,
	efWorkPhone = 2,
	efBirthDate = 3,
	efMobilePhone = 4,
	efPrefContact = 5,
	efPatBalance = 6,
	efInsBalance = 7,
	efRefSource = 8,
	efDuration = 9,
	efPager = 10,
	efOtherPhone = 11,
	efEmail = 12,
	efLastLM = 13,
	efInputName = 14,
	efModifiedLogin = 15,
	efModifiedDate = 16,
	efPrimaryInsurance = 17, // (d.moore 2007-06-05 15:20) - PLID 13550 - Adding extra fields.
	efPrimaryCoPay = 18,
	efSecondaryInsurance = 19,
	efSecondaryCoPay = 20,
	efCheckInTime = 21,		//DRT 9/10/2008 - PLID 31127
	efCheckOutTime = 22,	//DRT 9/10/2008 - PLID 31127
	efInputDate = 23,		//TES 11/12/2008 - PLID 11465 - Added InputDate
	efRefPhysName = 24,		//TES 11/12/2008 - PLID 12057 - Added Referring Physician info
	efRefPhysWorkPhone = 25,
	efRefPhysFax = 26,
	efAge = 27, // (z.manning 2008-11-19 14:24) - PLID 12282
	efInsuranceReferrals = 28, // (c.haag 2008-11-20 16:09) - PLID 31128
	efConfirmedBy = 29, // (j.armen 2011-07-01 15:45) - PLID 44205 - Added ConfirmedBy
	efArrivalTime = 30, // (z.manning 2011-07-01 16:04) - PLID 23273
	efCancelledDate = 31, // (j.luckoski 2012-04-30 09:57) - PLID 11597 - Added CancelledDate
	efApptPrimaryIns = 32, // (j.gruber 2012-08-06 09:16) - PLID 51926 - insurance information
	efApptSecondaryIns = 33, // (j.gruber 2012-08-06 09:16) - PLID 51926 - insurance information
	efApptInsList = 34,  // (j.gruber 2012-08-06 09:16) - PLID 51926 - insurance information
	efApptRefPhys = 35, // (j.gruber 2013-01-08 15:18) - PLID 54497 - appt ref phys
    efNxWebCode=36, // (s.tullis 2014-01-21 09:25) - PLID 49748 - Be able to see the NexWeb # from the scheduler
	efNxWebCodeDate=37,// (s.tullis 2014-01-24 15:20) - PLID 60470 - Add nexweb security code experation date to the  extra info fields

	//These need to be last, as they will be dynamically added to the end of the combo boxes.
	efCustom1,
	efCustom2,
	efCustom3,
	efCustom4,
};

class CResEntryDlg;
class CScheduledPatientsDlg;

class CSchedulerView : public CNxTabView
{
// (b.cardillo 2003-06-30 17:59) TODO: Need to not make the nxschedulerdlg and resentrydlg friends of schedulerview, and vice-versa.
friend CNxSchedulerDlg;
friend CResEntryDlg;

public:
	enum ESpecialResourceView {
		srvUnknownResourceView = -1, // This value is for code only, it's a sentinal value that should never be stored in data.
		srvCurrentUserDefaultView = -2, // This value gets stored in the ConfigRT table sometimes to indicate there is no ID, to just use the user's default view.
	};
	enum ESpecialDefaultResource {
		sdrUnknownResource = -1,
	};
	enum ESwitchToTab {
		// I'm making these negative because maybe someday we'll let the users define their own tabs, in which case 
		// they'd be stored as data (with positive IDs for each record)
		sttOneDayView = -1, 
		sttWeekView = -2, 
		sttMultiResourceView = -3, 
		sttMonthView = -4, 
	};
	


protected:
	CSchedulerView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CSchedulerView)

// Attributes
public:
	// (a.walling 2015-01-22 16:08) - PLID 64662 - Rescheduling Queue - selecting specific appointments
	void ShowReschedulingQueueEmbedded(bool bShow, long nApptID = -1); // (a.walling 2015-01-12 10:28) - PLID 64558 - Rescheduling Queue - Dock and embed

	//TES 2/11/2015 - PLID 64120 - Used when first creating a mix rule, to update whatever scheduler tab might already be open
	void ShowMixRulesButton();

	LPDISPATCH m_pResDispatch;
	CReservation &m_pResDispatchPtr; // (a.walling 2007-08-31 12:14) - PLID 27265
	LPDISPATCH m_pDispatchClip;
	LPDISPATCH m_pResDragging;  // (c.haag 2003-07-02 9:00) This is the reservation object
								// the user is dragging or resizing, or NULL if the user is
								// not dragging or resizing anything. The purpose for this
								// is to allow the scheduler to prevent the appointment from
								// being modified on the screen when another user changes it.
								//
								// I would have preferred making m_pResDispatch a
								// universal "The user exclusively owns this reservation"
								// object, but I can't find a clean way to set it to
								// NULL when it's no longer in use.

	CNxDialog *m_pClipSheet;
	BOOL m_bShowApptShowState;  // True if we want to show the show state in an appointment in a printout
	BOOL m_bShowMonthProcedures; // True if we want procedure names in month view printouts
	BOOL m_bShowTemplateColors; // True if we want to show template colors in a printout
	bool m_bShowAppointmentLocations; // Only used for remembering whether the menu item is checked or not
	bool m_bShowAppointmentResources; // Only used for remembering whether the menu item is checked or not
	
	BOOL m_bQuickScheduleNavigation; // True if we want to reflect the date as the user navigates through the date picker combo

	BOOL m_bShowPatientInformation; //Do we want to display "Individually Identifiable Health Information" on the screen.

	// (c.haag 2008-06-18 16:14) - PLID 26135 - Set to true if we want to print templates
	// in the month view
	BOOL m_bPrintTemplates;

// Operations
public:
	BOOL CheckPermissions();

	void ShowPatientWarning(long nPatientID,
		BOOL bSuppressPtWarning = FALSE,
		BOOL bSuppressInsReferralWarning = FALSE,
		BOOL bSuppressCoPayWarning = FALSE,
		BOOL bSuppressAllergyWarning = FALSE,
		BOOL bSuppressEMRProblemWarning = FALSE,
		BOOL bSuppressRewardsWarning = FALSE); // (j.luckoski 2013-03-04 13:59) - PLID 33548

	// (c.haag 2003-08-07 16:46) - Variables for tracking if appointments were
	// modified by someone else while you were modifying them
	COleDateTime m_dtActiveApptLastModified;
	long m_nTrackingResID;
	void ResetTrackedAppt();
	void TrackLastModifiedDate(long nResID);
	void TrackLastModifiedDate(long nResID, COleDateTime dtModifiedDate);
	BOOL WasTrackedApptModified(BOOL bReset, OUT CString& strModifiedLogin);

//	CSchedulerUpdateThread *m_pUpdateThread;
	virtual int Hotkey (int key);
	void ClearDispatchClip(CNxDialog *pSheet);
	void EnsureOfficeHoursTemplate();
	void RequeryAllPurposeFilters();
	void SetAllMultiTypeStrings(CString strTypes);
	void SetAllMultiPurposeStrings(CString strPurposes);
	// (d.singleton 2012-06-07 11:54) - PLID 47473
	void SetAllMultiLocationStrings(CString strLocations);
	BOOL m_bExtraInfo[10];
	int m_nExtraFields[10];
	int m_nExtraInfoStyles[10];//0=normal,1=bold, may be more someday.
	void ObtainExtraInfo();
	bool m_ShowExtraInfo[20];
	// (z.manning 2009-06-30 16:06) - PLID 34744 - Added this function so we only call
	// dbo.InsReferralSummary when necessary as it has been know to cause slowness issues.
	BOOL NeedToLoadInsReferralSummary();
	// (z.manning 2009-06-30 16:29) - PLID 34744 - This function will return the text to
	// load the insurance referral summary based on if we need to or not.
	CString GetInsReferralSummarySql(const CString &strPatientID);
	CDWordArray m_adwShowState; // Array of possible show state ID's (mirror of AptShowStateT)
	CStringArray m_astrShowState; // Array of possible show state names

	//DWORD m_dwClickResourceID;
	//COleDateTime m_dtClickTime;
	//COleDateTime m_dtClickDate;
	//BOOL m_bClickedEvent;//Was the clicked slot the event slot?
	//long m_nClickDay;
	Nx::Scheduler::TargetInfo m_clickTargetInfo;

	// (a.walling 2015-01-28 13:08) - PLID 64412 - need to know if source is the rescheduling queue
	long PasteAppointment(long nClipResID, long nClipResourceID, int nMethod, bool fromReschedulingQueue = false);

	// (a.walling 2015-01-28 13:08) - PLID 64412 - need to know if source is the rescheduling queue
	bool CopyRes(long fromResID, long fromResourceID, long toResID, long nMethod, bool bWantReschedule, bool fromReschedulingQueue = false);
	void SwitchToTab(IN const ESwitchToTab esttNewTab);
	long GetInterval();
	long GetTopSlot();
	CResEntryDlg * GetResEntry();
	void GetPivotDate(COleDateTime &theDate);
	BOOL SetPivotDate(const COleDateTime &newDate);
	const COleDateTime &GetPivotDate();
	void StoreDetails(CNxDialog *oldSheet);
	long GetPersonIdFromRes(LPDISPATCH lpResDispatch);
	void OpenAppointment(long nAppointmentID, bool bShowResEntry = false);
	long GetFirstHourOfDay(int iDayOfWeek = COleDateTime::GetCurrentTime().GetDayOfWeek());

	// (j.jones 2013-05-07 14:09) - PLID 53969 - changed this to be declared by reference,
	// which avoids requiring its .h file in this file
	CLocationOfficeHours &m_lohOfficeHours;
	void ReloadLocationDisplay(); //Forces us to check ConfigRT for whether or not to show the location name.
	void LoadShowStateArray(); // Loads an array of possible appt. show states

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSchedulerView)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnFinalRelease();
	virtual void OnInitialUpdate();
	protected:
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTableChanged(WPARAM wParam, LPARAM lParam);
	virtual LRESULT OnTableChangedEx(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnSelectTab(short oldTab, short newTab);
	afx_msg LRESULT OnHideResentry(WPARAM wParam, LPARAM lParam);
 
	//}}AFX_VIRTUAL

// Implementation
protected:
	NXSINGLEDAYPRINT m_nxspSingledayPrintInfo;

protected:
	virtual ~CSchedulerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	CString m_strAMPMBtn;
	long m_nPurposeSetId, m_nPurposeId;
	//TES 6/21/2010 - PLID 21341
	long m_nLocationId;
	BOOL m_bShowTemplateText;
	BOOL m_bShowPatientWarning;
	bool m_bGoToAM;
	long m_nInterval;
	long m_nTopSlot;
	CString m_BoxFormat;
	CResEntryDlg *m_ResEntry;
	COleDateTime m_PivotDate;

	// (j.jones 2013-05-07 14:10) - PLID 53969 - changed the child tabs to be declared by reference,
	// which avoids requiring their .h files in this file
	COneDayDlg &m_OneDaySheet;
	CWeekDayDlg &m_WeekDaySheet;
	CMultiResourceDlg &m_MultiResourceSheet;
	CMonthDlg &m_MonthSheet;

public:
	// (j.jones 2013-05-07 14:09) - PLID 53969 - changed this to be declared by reference,
	// which avoids requiring its .h file in this file
	CMoveUpListDlg &m_MoveUpDlg;
	CScheduledPatientsDlg *m_pdlgScheduledPatients;
	long m_nLastLoadedResourceViewID;
	CString m_strLastLoadedResourceViewName;

/////////////////////////////
// To work with resources
protected:
	// nNewResourceViewID is either the ID (from the database) of the view to be used, or one of the values from ESpecialResourceView
	void LoadResourceList(IN const long nNewResourceViewID, IN const long nTrySelectDefaultResourceID);
	CResourceEntryPtrArray m_arypCurrentResourceList;
	long m_nCurrentResourceID;
	
	void NotifyChildrenOfResourceListRefresh();
	void NotifyChildrenOfActiveResourceChange();

	void OnResMarkIn();
	void OnResMarkOut();
	void OnResMarkNoShow();
	void OnResMarkShow();
	void OnResMarkPending();
	void OnResMarkReceived();
	void OnResMarkUserDefinedShowState(const CString& strShowState);

	//BOOL AppointmentUpdate(ADODB::_RecordsetPtr pAppt, long nResIDToIgnore = -1);

public:
	void OnReloadCurrentResourceList();
	void ShowMoveUpList(const COleDateTime& dtStart, const CDWordArray& adwResourceFilter);

public:
	CString CalcCurrentResourceIDs();
	CString CalcCurrentResourceIDsUnionQuery(BOOL bIncludeRelevence);

public: 
	const CResourceEntryPtrArray &GetCurrentResourceList() const;
	const CResourceEntry *FindResourceInCurrentList(IN const long nResourceID) const;
	const CResourceEntry *FindResourceInCurrentList(const CDWordArray &arydwResourceIDs) const;

public:
	void SetActiveResourceID(IN const long nNewResourceID, BOOL bDoUpdateViewNow);
	long GetActiveResourceID() const;
	BOOL IsResourceInCurView(IN const long nResourceID) const;
	BOOL AreAllResourcesInCurView(IN const CDWordArray &arydwResourceIDs) const;
	BOOL AreAnyResourcesInCurView(IN const CDWordArray &arydwResourceIDs) const;
/////////////////////////////

	// (j.jones 2014-08-14 13:30) - PLID 63184 - added function that tells is if any resource
	// is in our current filter, not necessarily the resource view
	bool AreAnyResourcesInCurrentFilter(IN const CDWordArray &arydwResourceIDs) const;

public:
	void SetDefaultControlFocus();

public:
	void GetActiveDateRange(OUT COleDateTime &dtFrom, OUT COleDateTime &dtTo) const;
	BOOL IsDateInCurView(IN const COleDateTime &dtDate) const;

public:
	// These two functions take the pExpectActiveSheet to help ensure that people don't call it on a sheet 
	// that's not the one the user is looking at right now.  Doing so would be confusing and could even 
	// cause real bugs so I'm trying to do everything possible code-wise to make it impossible to screw up 
	// and call it on the wrong sheet.  It is still possible of course to fool the system, but of course 
	// you DON'T WANT TO DO THAT!
	void GetWorkingResourceAndDate(IN const CNxSchedulerDlg *pExpectActiveSheet, IN const long nColumnIndex, OUT long &nWorkingResourceID, OUT COleDateTime &dtWorkingDate) const;
	void GetWorkingResourceAndDate(IN const CNxSchedulerDlg *pExpectActiveSheet, IN const long nColumnIndex, OUT CSchedViewColumnInfo &vci) const;

	// The reason we don't have a "GetCurrentResourceViewID" is that if you want such a function you're 
	// probably doing something wrong.  That's because the CSchedulerView is responsible for maintaining the 
	// one and only "current list of resources", which is originally obtained by querying the data for all 
	// the resources in a given view.  But once the list of resources is loaded, all you should ever need to 
	// know is the resources that are in that list, NOT the view from which that list was generated.
	// The only reason this IsCurrentResourceViewID even exists is because the CNxSchedulerDlg shows a pop-
	// up menu listing the resource views, and it wants to be able to make the "current" one bold.  This 
	// IsCurrentResourceViewID function is really more of an "IsLastLoadedResourceViewID".
	const long GetLastLoadedResourceViewID(IN const CNxSchedulerDlg *pExpectActiveSheet) const;
	CString GetLastLoadedResourceViewName(IN const CNxSchedulerDlg *pExpectActiveSheet);
	//TES 7/26/2010 - PLID 39445 - Added nNewDefaultLocationID.  -2 = <No Default> (leave the location filter alone), -1 = {All Locations},
	// positive value = a specific location to filter on.
	void SetCurrentResourceViewID(IN const long nNewResourceViewID, IN const long nNewDefaultLocationID);
public:
	void LoadResourceViewName();

protected:
	CTableChecker m_resourceviewnameChecker;

protected:
	long QueryAppointmentExistsInCurrentView(IN const long nAppointmentID, OUT COleDateTime &dtChangeToDate, OUT long &nChangeToResourceID);


protected:
	BOOL DoPreparePrinting(CPrintInfo *pInfo);

protected:
	// (j.jones 2007-11-21 14:20) - PLID 28147 - added ability to create a new inventory allocation
	// (j.jones 2008-03-18 14:20) - PLID 29309 - added ability to create a new inventory order
	//DRT 6/12/2008 - PLID 9679 - Converted OnPrintSuperbill from ON_COMMAND to ON_MESSAGE
	// (j.jones 2008-06-23 16:16) - PLID 30455 - added ability to create a bill
	// (j.gruber 2008-09-10 16:00) - PLID 30282 added ability to edit an allocation
	//{{AFX_MSG(CSchedulerView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowTemplateNamesBtn();
	afx_msg void OnShowPatientWarningBtn();
	afx_msg void OnShowAppointmentLocations();
	afx_msg void OnShowAppointmentResources();
	afx_msg void OnQuickScheduleNavigation();
	afx_msg void OnShowPatientInformation();
	afx_msg void OnUpdateShowTemplateNamesBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowPatientWarningBtn(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowAppointmentLocations(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuickScheduleNavigation(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowPatientInformation(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowAppointmentResources(CCmdUI* pCmdUI);
	afx_msg LRESULT OnUpdateView(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnResEdit();
	afx_msg void OnResCut();
	afx_msg void OnResCopy();
	afx_msg void OnResPaste();
	afx_msg void OnResDelete();
	afx_msg void OnRestoreAppt(); // (j.luckoski 2012-05-02 16:18) - PLID 11597 - Restore cancelled appts
	afx_msg void OnResDeleteBlock();
	afx_msg void OnResMarkConfirmed();
	afx_msg void OnResMarkMoveUp();
	afx_msg void OnResMarkUnConfirmed();
	afx_msg void OnResRemoveMoveUp();
	afx_msg void OnEditTemplates();
	afx_msg void OnEditTemplateCollections(); // (z.manning 2014-12-01 17:39) - PLID 64205
	// (j.jones 2011-07-15 14:48) - PLID 39838 - added OnEditLocationTemplates
	afx_msg void OnEditLocationTemplates();
	afx_msg void OnGotopatient();
	afx_msg LRESULT OnPrintSuperbill(WPARAM wParam, LPARAM lParam);
	afx_msg void OnShowMoveUpList();
	afx_msg void OnUpdateShowScheduledPatients(CCmdUI* pCmdUI);
	afx_msg void OnShowScheduledPatients();
	afx_msg void OnCreateNewAppointment();	
	afx_msg void OnResNewInvAllocation();	
	afx_msg void OnResEditInvAllocation();	
	afx_msg void OnResNewInvOrder();
	afx_msg void OnResNewBill();
	// (j.jones 2009-08-28 12:55) - PLID 35381 - added case history options
	afx_msg void OnResCreateCaseHistory();
	afx_msg void OnResEditCaseHistory();
	//(e.lally 2009-09-28) PLID 18809
	afx_msg void OnResMarkLeftMessage();
	// (j.jones 2010-09-27 11:40) - PLID 38447 - added ability to view eligibility responses
	afx_msg void OnResViewEligibilityResponses();
	afx_msg void OnManagePaymentProfiles(); // (z.manning 2015-07-22 15:17) - PLID 67241
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CSchedulerView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCHEDULERVIEW_H__8D6FB325_B8E2_11D1_B2DD_00001B4B970B__INCLUDED_)

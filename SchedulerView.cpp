// SchedulerView.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SchedulerView.h"
#include "CommonSchedUtils.h"
#include "NxSchedulerDlg.h"
#include "GlobalUtils.h"
#include "PracProps.h"
#include "MainFrm.h"
#include "PrintScheduleDlg.h"
#include "TemplateLineItemInfo.h"
#include "TemplateRuleInfo.h"
#include "TemplateEntryDlg.h"
// (a.walling 2008-09-18 17:10) - PLID 28040 - Removed SchedulerUpdateThread.h
//#include "SchedulerUpdateThread.h"
#include "GlobalSchedUtils.h"
#include "ResEntryDlg.h"
#include "NxStandard.h"
#include "AuditTrail.h"
#include "schedulerrc.h"
#include "appointmentsdlg.h" // For pending/in/out/noshow popup menu options
#include "multiresourcepastedlg.h" // For pasting an appointment with multiple resources
#include "practicerc.h"
#include "userwarningdlg.h"
#include "Superbill.h"
#include "TemplateItemEntryGraphicalDlg.h"

#include "GlobalDataUtils.h"
// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace SINGLEDAYLib;
#include "ScheduledPatientsDlg.h"
#include "nxmessagedef.h"
#include "PPCLink.h"

#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "EmrUtils.h"
#include "InvUtils.h"
#include "InvPatientAllocationDlg.h"
#include "GlobalFinancialUtils.h"
#include "HL7Utils.h"
#include "DontShowDlg.h"
#include "CaseHistoryDlg.h"
#include "ChildFrm.h"

#include "TemplateHitSet.h"
#include "OneDayDlg.h"
#include "WeekDayDlg.h"
#include "MultiResourceDlg.h"
#include "MonthDlg.h"
#include "TemplatesDlg.h"
#include "MoveUpListDlg.h"
#include "ReschedulingUtils.h"
#include <NxPracticeSharedLib\SharedScheduleUtils.h>

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#define RES_AUTO_REFRESH_TIMER 1001

// These defines control the scheduler refresh
// RES_AUTO_REFRESH_TIME is in milliseconds (min is 100 I think)
// RES_AUTO_REFRESH determines whether or not the auto-refresh is enabled
//                              if you comment out this define, auto-refresh will be disabled

#define RES_AUTO_REFRESH_TIME	40
//#define RES_AUTO_REFRESH


////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: (b.cardillo 2002-07-02 15:37) GetAppointmentResourceIDs(), FindDWordArrayElement(), 
// CompareDWordArray(), and enum ECompareSetResult are all part of some temporary functionality.  We 
// may remove it but most likely we'll use it in some form.  When we start using it officially, it 
// needs to be moved out of this .cpp file and into the appropriate "utils" file.
void GetAppointmentResourceIDs(IN long nFromApptID, OUT CDWordArray &aryFromResourceIDs)
{
	// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
	_RecordsetPtr prs = CreateParamRecordset("SELECT ResourceID FROM AppointmentResourceT WHERE AppointmentID = {INT}", nFromApptID);
	FieldPtr pfldResourceID = prs->GetFields()->GetItem("ResourceID");
	while (!prs->eof) {
		aryFromResourceIDs.Add(AdoFldLong(pfldResourceID));
		prs->MoveNext();
	}
}

// Returns the index of the first occurrence of the given value in the given array
// Returns -1 if the value doesn't occur in the array
int FindDWordArrayElement(const CDWordArray &ary, DWORD dwFindValue)
{
	int nCount = ary.GetSize();
	for (int i=0; i<nCount; i++) {
		if (ary.GetAt(i) == dwFindValue) {
			return i;
		}
	}
	return -1;
}

long FindElementInArray(const CDWordArray &ary, const DWORD dwElement)
{
	// Search through until an element equal to the one given is found
	long nCount = ary.GetSize();
	for (long i=0; i<nCount; i++) {
		// See if this element is equal
		if (ary.GetAt(i) == dwElement) {
			// It is equal, so return this index
			return i;
		}
	}
	
	// If we made it here, the element wasn't found
	return feiarNotFound;
}

enum ECompareSetResult
{
	csrEqual,		// (eg. A={1,2,3}, B={2,1,3))	The sets are have the same elements
	csrSubset,		// (eg. A={1,2},   B={2,1,3))	All elements in the first set are in the second set, but not all elements in the second set are in the first
	csrSuperset,	// (eg. A={1,2,3}, B={1,3))		All elements in the second set are in the first set, but not all elements in the first set are in the second
	csrIntersect,	// (eg. A={1,2,3}, B={3,4,5))	The two sets share some elements, but each set has at least one element that is not in the other
	csrDisparate,	// (eg. A={1,2,3}, B={4,5,6))	The two sets share no elements
};

ECompareSetResult CompareDWordArray(const CDWordArray &ary1, const CDWordArray &ary2)
{
	long nCountDiff = ary1.GetSize() - ary2.GetSize();

	const CDWordArray &aryPrimary = *((nCountDiff >= 0) ? &ary1 : &ary2);
	const CDWordArray &arySecondary = *((nCountDiff >= 0) ? &ary2 : &ary1);

	int nFoundShared = 0;
	int nFoundUnshared = 0;
	int nCount = aryPrimary.GetSize();
	for (int i=0; i<nCount; i++) {
		if (FindDWordArrayElement(arySecondary, aryPrimary.GetAt(i)) != -1) {
			// Found one that the two sets share
			nFoundShared++;
		} else {
			// Found one that the two sets don't share
			nFoundUnshared++;
		}

		if (nFoundShared > 0 && nFoundUnshared > 0) {
			// This means we've found one that was shared and one that was not, so 
			// we can stop searching because we know that either the sets are 
			// intersecting or the primary is a superset of the secondary.
			
			// If there are any in the secondary not in the primary then they're 
			// intersecting, otherwise the primary is a superset

			// So loop through the secondary searching the primary for each 
			// secondary element
			int nCountSecondary = arySecondary.GetSize();
			for (int j=0; j<nCountSecondary; j++) {
				if (FindDWordArrayElement(aryPrimary, arySecondary.GetAt(j)) == -1) {
					// Found one in the secondary that was NOT in the primary, 
					// so the sets must be intersecting
					return csrIntersect;
				}
			}
			
			// If we made it here, we know that all elements in the secondary set 
			// also exist in the primary, which means the primary is a superset 
			// of the secondary because the primary has already been proven to 
			// contain one element that's not in the secondary.
			if (nCountDiff >= 0) {
				// ary1 is primary
				return csrSuperset;
			} else {
				// ary2 is primary
				return csrSubset;
			}
		}
	}

	// If we made it to here, then we KNOW the two sets must be equal, subset, superset, or disparate
	
	// First check to see if any were unshared (found in one and not the other)
	if (nFoundUnshared == 0) {
		// All elements were shared so we have equivalent sets (even if they're just both empty)
		return csrEqual;
	}

	// Then check the rest
	if (nFoundShared == 0) {
		// No elements were shared so we have disparate sets
		return csrDisparate;
	}

	// If we made it here, then one set was a subset of the other, which is which really just depends on which had more elements
	if (nCountDiff >= 0) {
		// ary1 is primary
		return csrSuperset;
	} else {
		// ary2 is primary
		return csrSubset;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////


void GetApptCountForResource(long nMasterItemId, OUT long &nApptCount)
{
	// Get the count of appointments under the resource
//	_RecordsetPtr prs = CreateRecordset(
//		"SELECT Count(*) AS ApptCount FROM AppointmentsT WHERE ResourceID = %li", nMasterItemId);

	// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT Count(*) AS ApptCount FROM AppointmentResourceT WHERE ResourceID = {INT}", nMasterItemId);

	// How could Count(*) not return a record?  
	nApptCount = AdoFldLong(prs, "ApptCount");
}


/////////////////////////////////////////////////////
/// Implementation of CLocationOfficeHours (helper class)
///
CLocationOfficeHours::CLocationOfficeHours()
{
	// Initialize to invalid
	Init(-1);
}

// Will clear everything and cause the next request to load from the database
void CLocationOfficeHours::Init(IN int nLocationID)
{
	// Remember the location ID
	m_nLocationID = nLocationID;

	// We haven't gotten the hours yet
	m_bGotOfficeHours = FALSE;
	
	// This shouldn't really be necessary because this variable won't be 
	// referenced until it is set, but I hate to not initialize variables
	for (int i=0; i<7; i++) {
		m_bDayValid[i] = FALSE;
	}
}

long CLocationOfficeHours::GetLocationID() const
{
	return m_nLocationID;
}

void CLocationOfficeHours::EnsureAllOfficeHours()
{
	// If we've been asked to reload from the database, or if we've never loaded in the first place, then load
	if (!m_bGotOfficeHours) {

		ASSERT(m_nLocationID != -1);
		if (m_nLocationID == -1) {
			AfxThrowNxException("CLocationOfficeHours::EnsureAllOfficeHours: The location for this office hours object has not been initialized");
		}

		// Open the recordset
		// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT SundayOpen, SundayClose, "
			"MondayOpen, MondayClose, TuesdayOpen, TuesdayClose, WednesdayOpen, WednesdayClose, "
			"ThursdayOpen, ThursdayClose, FridayOpen, FridayClose, SaturdayOpen, SaturdayClose "
			"FROM LocationsT WHERE ID = {INT}", m_nLocationID);

		// Loop through the 7 sets of fields in this one record
		FieldsPtr pflds = prs->GetFields();
		for (int i=0; i<7; i++) {
			// Get the name of the day
			CString strDayName = GetDayText(i,TRUE);
			// Get the values into variants
			_variant_t varDayOpen = pflds->GetItem((LPCTSTR)(strDayName + _T("Open")))->GetValue();
			_variant_t varDayClose = pflds->GetItem((LPCTSTR)(strDayName + _T("Close")))->GetValue();
			// Handle the variants
			if (varDayOpen.vt != VT_NULL && varDayClose.vt != VT_NULL) {
				// Both non-null so store them in variables
				COleDateTime dtOpen = VarDateTime(varDayOpen);
				COleDateTime dtClose = VarDateTime(varDayClose);
				m_dtDayOpen[i].SetTime(dtOpen.GetHour(), dtOpen.GetMinute(), dtOpen.GetSecond());
				m_dtDayClose[i].SetTime(dtClose.GetHour(), dtClose.GetMinute(), dtClose.GetSecond());
				m_bDayValid[i] = TRUE;
			} else {
				// One of the dates was null, so this day has invalid office hours
				m_dtDayOpen[i] = COleDateTime(0,0,0,0,0,0);
				m_dtDayClose[i] = COleDateTime(0,0,0,0,0,0);
				m_bDayValid[i] = FALSE;
			}
		}

		// We have our info
		m_bGotOfficeHours = TRUE;
	}
}

// Returns TRUE if the day has valid office hours, FALSE if not
// Throws _com_error, NxException
BOOL CLocationOfficeHours::GetOfficeHours(IN int nDayIndex, OUT COleDateTime &dtOpen, OUT COleDateTime &dtClose)
{
	if (nDayIndex >= 0 && nDayIndex < 7) {
		
		// Make sure we've obtained the info from the database
		EnsureAllOfficeHours();

		// Give back the results
		dtOpen = m_dtDayOpen[nDayIndex];
		dtClose = m_dtDayClose[nDayIndex];

		// Return valid or not
		return m_bDayValid[nDayIndex];
	} else {
		AfxThrowNxException("CLocationOfficeHours::GetOfficeHoursAsTemplateSet: Invalid index %i", nDayIndex);
		return FALSE;
	}
}

// (z.manning, 11/28/2006) - PLID 5812
// Returns TRUE as long as at least 1 day has valid office hours, otherwise FALSE;
BOOL CLocationOfficeHours::GetMinOfficeHours(OUT COleDateTime &dtOpen, OUT COleDateTime &dtClose)
{
	EnsureAllOfficeHours();
	
	COleDateTimeSpan dtsMinOpen(2,0,0,0);
	short nMinDayIndex = -1;

	// (z.manning, 11/28/2006) - PLID 5812 - Go through each day and find the minimum time range of days that have
	// office hours.
	for(short nDay = 0; nDay < 7; nDay++) {
		if(m_bDayValid[nDay]) {
			ASSERT(CompareTimes(m_dtDayOpen[nDay],m_dtDayClose[nDay]) & CT_LESS_THAN_OR_EQUAL);
			COleDateTimeSpan dtsCurrent = m_dtDayClose[nDay] - m_dtDayOpen[nDay];
			if(dtsCurrent < dtsMinOpen) {
				dtsMinOpen = dtsCurrent;
				nMinDayIndex = nDay;
			}
		}
	}

	// (z.manning, 11/28/2006) - PLID 5812 - If we found a valid day, set the times and return true;
	if(nMinDayIndex >=0 && nMinDayIndex < 7) {
		dtOpen = m_dtDayOpen[nMinDayIndex];
		dtClose = m_dtDayClose[nMinDayIndex];
		return TRUE;
	}
	
	// (z.manning, 11/28/2006) - PLID 5812 - If we get here, we must be closed every day, so return false.
	dtOpen.SetStatus(COleDateTime::invalid);
	dtClose.SetStatus(COleDateTime::invalid);
	return FALSE;
}

IMPLEMENT_DYNAMIC(CClipRes, CClip)

/////////////////////////////////////////////////////////////////////////////
// CSchedulerView

IMPLEMENT_DYNCREATE(CSchedulerView, CNxTabView)

// (j.jones 2013-05-07 14:13) - PLID 53969 - changed the child tabs to be declared by reference,
// which means their initialization is slightly different now
CSchedulerView::CSchedulerView()
	: CNxTabView(),
	m_resourceviewnameChecker(NetUtils::Resources),
	m_pResDispatchPtr(*(new CReservation("CSchedulerView")))
	, m_OneDaySheet(*(new COneDayDlg(this)))
	, m_WeekDaySheet(*(new CWeekDayDlg(this)))
	, m_MultiResourceSheet(*(new CMultiResourceDlg(this)))
	, m_MonthSheet(*(new CMonthDlg(this)))
	, m_MoveUpDlg(*(new CMoveUpListDlg(this)))
	, m_lohOfficeHours(*(new CLocationOfficeHours()))
{
	EnableAutomation();

	// Tell all the sheets who their parents are, we do this in our own constructor to help ensure the sheets ALWAYS have a parent
	m_OneDaySheet.InitSchedViewParent(this);
	m_WeekDaySheet.InitSchedViewParent(this);
	m_MultiResourceSheet.InitSchedViewParent(this);
	m_MonthSheet.InitSchedViewParent(this);
	
	// Set pivot date to be the current date
	COleDateTime tmpDate = COleDateTime::GetCurrentTime();
	m_PivotDate.SetDate(tmpDate.GetYear(), tmpDate.GetMonth(), tmpDate.GetDay());

	// (z.manning, 05/14/2008) - PLID 29680 - Removed references to 640x480 dialogs
	m_ResEntry = new CResEntryDlg(NULL, IDD_RES_ENTRY_DLG, this);
	
	m_OneDaySheet.m_pParent = this;
	m_WeekDaySheet.m_pParent = this;
	m_MultiResourceSheet.m_pParent = this;
	m_MonthSheet.m_pParent = this;

	m_pResDispatch = NULL;
	m_pDispatchClip = NULL;
	m_pClipSheet = NULL;
	m_pResDragging = NULL;

	// (a.walling 2007-08-31 12:14) - PLID 27265
	m_pResDispatchPtr.SetDispatch(NULL);

	m_bGoToAM = false;
	m_strAMPMBtn = "1:00 PM";//"12:00 PM";
	m_nInterval = 15;
	m_nPurposeSetId = -1;
	m_nPurposeId = -1;
	m_nTopSlot = -1;

	m_nTrackingResID = -1;
	m_dtActiveApptLastModified.m_status = COleDateTime::invalid;

	m_bShowTemplateText = FALSE;
	m_bShowApptShowState = TRUE; // True if we want to show the show state in an appointment in a print out
	m_bShowMonthProcedures = FALSE; // True if we want procedure names in month view printouts
	m_bShowTemplateColors = FALSE; // True if we want to show template colors in a printout
	m_bShowPatientWarning = TRUE;
	m_bShowAppointmentLocations = FALSE;
	m_bShowAppointmentResources = FALSE;	
	m_bQuickScheduleNavigation = TRUE;
	m_bPrintTemplates = FALSE; // (c.haag 2008-06-18 16:15) - PLID 26135 - True if we want to print templates

//	m_pUpdateThread = CreateNxThread(CSchedulerUpdateThread);
//	m_pUpdateThread->Go(this);

	m_pdlgScheduledPatients = NULL;

	m_nLastLoadedResourceViewID = srvUnknownResourceView;
}

CSchedulerView::~CSchedulerView()
{
	try {

		if (m_ResEntry) {
			if (m_ResEntry->GetSafeHwnd()) {
				m_ResEntry->DestroyWindow();
			}
			delete m_ResEntry;
		}

		if (m_pdlgScheduledPatients) {
			if (m_pdlgScheduledPatients->GetSafeHwnd()) {
				m_pdlgScheduledPatients->DestroyWindow();
			}
			delete m_pdlgScheduledPatients;
		}

		// (j.jones 2013-05-07 14:15) - PLID 53969 - changed the child tabs to be declared by reference,
		// which means we have to clean up their pointers now
		// They are never null. They have to have been filled in the constructor.
		delete &m_OneDaySheet;
		delete &m_WeekDaySheet;
		delete &m_MultiResourceSheet;
		delete &m_MonthSheet;
		delete &m_MoveUpDlg;
		delete &m_lohOfficeHours;
		delete &m_pResDispatchPtr;

	}NxCatchAll(__FUNCTION__);
}

void CSchedulerView::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CView::OnFinalRelease();
}


// (j.jones 2007-11-21 14:20) - PLID 28147 - added ability to create a new inventory allocation
// (j.jones 2008-03-18 14:20) - PLID 29309 - added ability to create a new inventory order
// (j.jones 2008-06-23 16:16) - PLID 30455 - added ability to create a bill
// (j.gruber 2008-09-10 15:50) - PLID 30282 - added allocation edit ability
// (j.jones 2009-08-28 12:55) - PLID 35381 - added case history options
BEGIN_MESSAGE_MAP(CSchedulerView, CNxTabView)
	//{{AFX_MSG_MAP(CSchedulerView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//ON_COMMAND(IDM_UPDATE_VIEW, OnUpdateView) // (c.haag 2003-10-15 12:38) - Already done in MainFrame
	ON_COMMAND(ID_SHOW_MOVEUP_LIST, OnShowMoveUpList)
	ON_WM_TIMER()
	ON_COMMAND(ID_RES_EDIT, OnResEdit)
	ON_COMMAND(ID_RES_CUT, OnResCut)
	ON_COMMAND(ID_RES_COPY, OnResCopy)
	ON_COMMAND(ID_RES_PASTE, OnResPaste)
	ON_COMMAND(ID_RES_DELETE, OnResDelete)
	ON_COMMAND(ID_RESTORE_APPT, OnRestoreAppt) // (j.luckoski 2012-05-02 16:14) - PLID 11597 - Restore appt context option
	ON_COMMAND(ID_RES_DELETEBLOCK, OnResDeleteBlock)	
	ON_COMMAND(ID_APPT_NEW_INV_ALLOCATION, OnResNewInvAllocation)	
	ON_COMMAND(ID_APPT_EDIT_INV_ALLOCATION, OnResEditInvAllocation)	
	ON_COMMAND(ID_APPT_CREATE_CASE_HISTORY, OnResCreateCaseHistory)
	ON_COMMAND(ID_APPT_EDIT_CASE_HISTORY, OnResEditCaseHistory)
	ON_COMMAND(ID_APPT_NEW_INV_ORDER, OnResNewInvOrder)
	ON_COMMAND(ID_APPT_NEW_BILL, OnResNewBill)
	ON_COMMAND(ID_APPT_VIEW_ELIGIBILITY_RESPONSES, OnResViewEligibilityResponses)
	ON_COMMAND(ID_APPT_CONFIRMED, OnResMarkConfirmed)
	ON_COMMAND(ID_APPT_MOVE_UP, OnResMarkMoveUp)
	ON_COMMAND(ID_APPT_UNCONFIRMED, OnResMarkUnConfirmed)
	ON_COMMAND(ID_APPT_REMOVE_MOVE_UP, OnResRemoveMoveUp)
	ON_COMMAND(ID_RES_EDIT_TEMPLATES, OnEditTemplates)
	ON_COMMAND(ID_RES_EDIT_TEMPLATE_COLLECTIONS, OnEditTemplateCollections) // (z.manning 2014-12-01 17:34) - PLID 64205
	ON_COMMAND(ID_RES_EDIT_LOCATION_TEMPLATES, OnEditLocationTemplates)
	ON_COMMAND(ID_GOTOPATIENT, OnGotopatient)
	ON_MESSAGE(ID_PRINTSUPERBILL, OnPrintSuperbill)
	ON_COMMAND(ID_SHOW_TEMPLATE_NAMES, OnShowTemplateNamesBtn)
	ON_COMMAND(ID_SHOW_PATIENT_WARNING, OnShowPatientWarningBtn)
	ON_COMMAND(ID_VIEW_SHOW_APPT_LOCATIONS, OnShowAppointmentLocations)
	ON_COMMAND(ID_VIEW_QUICK_SCHEDULE_NAVIGATION, OnQuickScheduleNavigation)
	ON_COMMAND(ID_VIEW_SHOWPATIENTINFORMATION, OnShowPatientInformation)
	ON_COMMAND(ID_VIEW_SHOW_APPT_RESOURCES, OnShowAppointmentResources)
	ON_COMMAND(ID_ACTIVITIES_CREATENEWAPPOINTMENT, OnCreateNewAppointment)
	ON_UPDATE_COMMAND_UI(ID_SHOW_TEMPLATE_NAMES, OnUpdateShowTemplateNamesBtn)
	ON_UPDATE_COMMAND_UI(ID_SHOW_PATIENT_WARNING, OnUpdateShowPatientWarningBtn)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_APPT_LOCATIONS, OnUpdateShowAppointmentLocations)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOW_APPT_RESOURCES, OnUpdateShowAppointmentResources)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QUICK_SCHEDULE_NAVIGATION, OnUpdateQuickScheduleNavigation)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWPATIENTINFORMATION, OnUpdateShowPatientInformation)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_UPDATE_COMMAND_UI(ID_SHOW_SCHEDULED_PATIENTS, OnUpdateShowScheduledPatients)
	ON_COMMAND(ID_SHOW_SCHEDULED_PATIENTS, OnShowScheduledPatients)
	ON_MESSAGE(NXM_HIDE_RESENTRY, OnHideResentry)
	ON_MESSAGE(NXM_UPDATEVIEW, OnUpdateView)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CNxTabView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CNxTabView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CNxTabView::OnFilePrintPreview)
	ON_COMMAND(ID_APPT_LEFTMESSAGE, OnResMarkLeftMessage)
	ON_UPDATE_COMMAND_UI(IDM_UPDATE_VIEW, CNxTabView::OnUpdateViewUI) // (z.manning 2010-07-19 16:06) - PLID 39222
	ON_COMMAND(ID_APPT_MANAGE_PAYMENT_PROFILES, OnManagePaymentProfiles)
END_MESSAGE_MAP()

	

BEGIN_DISPATCH_MAP(CSchedulerView, CNxTabView)
	//{{AFX_DISPATCH_MAP(CSchedulerView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_ISchedulerView to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {8D6FB324-B8E2-11D1-B2DD-00001B4B970B}
static const IID IID_ISchedulerView =
{ 0x8d6fb324, 0xb8e2, 0x11d1, { 0xb2, 0xdd, 0x0, 0x0, 0x1b, 0x4b, 0x97, 0xb } };

BEGIN_INTERFACE_MAP(CSchedulerView, CNxTabView)
	INTERFACE_PART(CSchedulerView, IID_ISchedulerView, Dispatch)
END_INTERFACE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSchedulerView drawing

/////////////////////////////////////////////////////////////////////////////
// CSchedulerView diagnostics

#ifdef _DEBUG
void CSchedulerView::AssertValid() const
{
	CNxTabView::AssertValid();
}

void CSchedulerView::Dump(CDumpContext& dc) const
{
	CNxTabView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSchedulerView message handlers

int CSchedulerView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// Create objects
	if (CNxTabView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// (c.haag 2006-04-13 15:44) - PLID 20131 - Load all patient toolbar properties into the
	// NxPropManager cache
	g_propManager.CachePropertiesInBulk("SCHEDULERVIEW", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name LIKE 'ResEntry%%' OR "
		"Name LIKE 'ResShow%%' OR "
		"Name = 'SchedShowActivePtOnly' OR "
		"Name = 'FormatPhoneNums' OR "
		"Name = 'ResShowResourceName' OR "
		"Name = 'ResQuickScheduleNavigation' OR "
		"Name = 'ResInterval' OR "
		"Name = 'ResCurItemID' OR "
		"Name = 'CalculateAmPmTime' OR "
		"Name = 'CurResourceView' OR "
		// (j.jones 2007-08-09 11:14) - PLID 27027 - cached the package preferences
		"Name = 'ShowPackageInfo' OR "
		"Name = 'ShowPackageInfo_PrepaidOnly' OR "
		"Name = 'IncludeAllPrePaysInPopUps' OR "
		// (c.haag 2008-03-24 09:29) - PLID 29328 - Cache open order / allocation preferences
		"Name = 'SchedShowOpenOrders' OR "
		"Name = 'SchedShowOpenAllocations' OR "
		// (j.jones 2009-10-26 09:23) - PLID 32904 - added PackageTaxEstimation
		"Name = 'PackageTaxEstimation' OR "
		// (j.jones 2010-05-04 10:52) - PLID 32325 - added caching of OHIP settings
		"Name = 'UseOHIP' OR "
		"Name = 'OHIP_HealthNumberCustomField' OR "
		"Name = 'WarnCopays' OR "	// (j.jones 2010-08-03 10:19) - PLID 39937
		// (j.jones 2010-09-03 11:48) - PLID 40397 - cache the appointment more info setting
		"Name = 'ResEntry_RememberShowExtraInfo' OR "
		// (c.haag 2010-09-08 15:17) - PLID 37734 - Cache waiting list popping up preference
		"Name = 'ApptMoveUpOpenWaitingList' OR "
		// (j.jones 2010-11-03 15:08) - PLID 39620 - added Alberta option
		"Name = 'UseAlbertaHLINK' OR "
		"Name = 'Alberta_PatientULICustomField' OR "
		// (c.haag 2010-11-16 13:49) - PLID 39444 - Added resource location abbreviation display
		"Name = 'SchedShowResLocationAbbrev' "
		//TES 3/9/2011 - PLID 41519 - Added SchedHighlightLastAppt and SchedHighlightLastApptColor, as well as a bunch of properties 
		// I found in NxSchedulerDlg that weren't cached already.
		"OR Name = 'SchedShowLocationTemplateTimesInDescription' " // (a.walling 2011-08-09 08:44) - PLID 45023
		// (b.cardillo 2016-06-01 14:11) - NX-100771 - Obliterate the old scheduler custom border width preference
		"OR Name = 'SchedulerTemplateCompactText' "
		"OR Name = 'SchedulerUseSimpleForeColors' "
		"OR Name = 'SnapResToGrid' "
		"OR Name = 'ShowAppointmentTooltip' "
		"OR Name = 'ShowAppointmentArrows' "
		"OR Name = 'RepositionAppointmentText' "
		"OR Name = 'SchedHighlightLastApptColor' "
		"OR Name = 'SchedHighlightLastAppt' "
		"OR Name = 'DefaultAmTime' "
		"OR Name = 'DefaultAmAnchorTime' "
		"OR Name = 'DefaultPmTime' "
		"OR Name = 'DefaultPmAnchorTime' "
		"OR Name = 'PromptOnModifyLinkedAppts' "
		"OR Name = 'ApptValidateByDuration' "
		"OR Name = 'ApptCheckOpenSlotsForMoveUps' "
		// (j.jones 2011-11-16 13:11) - PLID 44117 - added AutoActivateScheduledInactivePts
		"OR Name = 'AutoActivateScheduledInactivePts' "
		// (a.wilson 2012-06-14 15:38) - PLID 47966 - caching changed preferences.
		"OR Name = 'ColorApptBackground' "
		"OR Name = 'ApptBackgroundColorDelta' "
		"OR Name = 'ColorApptBgWithStatus' "
		")",
		_Q(GetCurrentUserName()));

	// Make sure we know the office hours for this location
	m_lohOfficeHours.Init(GetCurrentLocationID());

	// Make sure the m_ResEntry window is created
	if (!m_ResEntry->GetSafeHwnd()) {
		// (z.manning, 05/14/2008) - PLID 29680 - Removed references to 640x480 dialogs
		m_ResEntry->Create(IDD_RES_ENTRY_DLG, GetMainFrame());
	}

	long nFirstHour = GetFirstHourOfDay(), nFirstMin = 0;
	try {
		// (b.cardillo 2003-06-26 17:18) - This code is a shame and when I encountered it I wanted to 
		// clean it up, but the problem is it's really just a decent GUESS at what the "active" resource 
		// will be, in an effort to GUESS what the top appointment on the one-day view will be.  The 
		// whole point is to be able to determine what the top slot of should be when we start up.  So 
		// I'm leaving this as is even though it often doesn't work (and uses bad variable names).
		// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
		_CommandPtr pCmd = OpenParamQuery("");

		CString strItemWhere;
		long nItemID = GetRemotePropertyInt("ResCurItemID", -1, 0, GetCurrentUserName());
		if (nItemID != -1) {
			strItemWhere = "AND (AppointmentResourceT.ResourceID = ?) ";
			AddParameterLong(pCmd, "ResourceID", nItemID);
		}

		COleDateTime dtToday = COleDateTime::GetCurrentTime();
		dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());
		// The 12:01 AM is to avoid finding events when we're checking for the earliest appointment
		// (d.thompson 2010-05-06) - PLID 38525 - Parameterized.  I reworked the 12:01 thing.  The concept seems to be this:
		//	Take the 'dtToday' value and instead of searching for >= [date] 12:00 AM, instead search for [date] 12:01 am.  I
		//	can make this more parameter-able by doing a DATEADD of a minute.
		//I also moved the ItemWhere up, because we're adding the parameter before the time parameters.
		pCmd->PutCommandText(_bstr_t("SELECT TOP 1 StartTime "
			"FROM AppointmentsT INNER JOIN AppointmentResourceT ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
			"WHERE (Status <> 4) " + strItemWhere + " AND (StartTime >= DATEADD(mi, 1, ?) AND StartTime < ?) " 
			"ORDER BY StartTime"));
		AddParameterString(pCmd, "StartTime", FormatDateTimeForSql(dtToday, dtoDate));
		AddParameterString(pCmd, "EndTime", FormatDateTimeForSql(dtToday+COleDateTimeSpan(1,0,0,0), dtoDate));
		_RecordsetPtr prs = CreateRecordset(pCmd);
		if (!prs->eof) {
			COleDateTime dtStart = AdoFldDateTime(prs, "StartTime");
			nFirstHour = min(nFirstHour, dtStart.GetHour());
			nFirstMin = dtStart.GetMinute();
		}
	} NxCatchAll("CSchedulerView::OnCreate Error 1");

	m_bShowTemplateText = GetRemotePropertyInt("ResShowTemplateName", 0, 0, GetCurrentUserName(), false) ? TRUE : FALSE;
	m_bShowPatientWarning = GetRemotePropertyInt("ResShowPatientWarning", 1, 0, GetCurrentUserName(), false) ? TRUE : FALSE;
	m_bShowAppointmentLocations = GetRemotePropertyInt("ResShowLocName", 0, 0, GetCurrentUserName(), true) ? TRUE : FALSE;
	m_bShowAppointmentResources = GetRemotePropertyInt("ResShowResourceName", 0, 0, GetCurrentUserName(), false) ? TRUE : FALSE;
	m_bQuickScheduleNavigation = GetRemotePropertyInt("ResQuickScheduleNavigation", 1, 0, GetCurrentUserName(), false) ? TRUE : FALSE;
	m_bShowPatientInformation = GetRemotePropertyInt("ResShowPatInfo", 1, 0, GetCurrentUserName(), true) ? TRUE : FALSE;

	// Load the list of available showstate's
	LoadShowStateArray();

	// Load this user's last selected resource view, and last selected resource (we need to do this before 
	// calling create sheet because in the oninitdialog of each sheet, they try to reflect these selections)
	int nLastView = GetRemotePropertyInt("CurResourceView",
			GetRemotePropertyInt("CurResourceView", srvCurrentUserDefaultView, 0, GetCurrentUserName(), false),
			GetCurrentLocationID(),
			GetCurrentUserName(),
			false);
	if(nLastView == srvUnknownResourceView) {//TES 2004-02-09: This will happen if last time they were in Practice there was an error.
		nLastView = srvCurrentUserDefaultView;
	}
	//TES 7/26/2010 - PLID 39445 - Load the default location for this view.
	// (d.singleton 2012-06-21 11:49) - PLID 47473 changed to -1 as -2 is now a real option for < multiple locations > ,  -1 should just be < show all locations >
	long nDefaultLocationID = -1;
	if(nLastView > 0) {
		_RecordsetPtr rsLocationID = CreateParamRecordset("SELECT LocationID FROM ResourceViewsT WHERE ID = {INT}", nLastView);
		if(!rsLocationID->eof) {
			nDefaultLocationID = AdoFldLong(rsLocationID, "LocationID", -1);
		}
	}
	m_nLocationId = nDefaultLocationID;

	LoadResourceList(
		//GetRemotePropertyInt("CurResourceView", srvCurrentUserDefaultView, 0, GetCurrentUserName(), false),
		nLastView,
		GetRemotePropertyInt("ResCurItemID", sdrUnknownResource, 0, GetCurrentUserName()));

	// Create the sheets that show in each tab
	Modules::Tabs& tabs = g_Modules[Modules::Scheduler]->Reset().GetTabs();

	CreateSheet(&m_OneDaySheet, tabs, SchedulerModule::DayTab);
	CreateSheet(&m_WeekDaySheet, tabs, SchedulerModule::WeekTab);
	CreateSheet(&m_MultiResourceSheet, tabs, SchedulerModule::MultiResourceTab);
	CreateSheet(&m_MonthSheet, tabs, SchedulerModule::MonthTab);

	//DRT 4/18/2008 - PLID 29711 - Set the text color of the tabs.
	// (j.jones 2016-04-18 17:46) - NX-100214 - the NxTab control now defaults
	// to the value of CNexTechDialog::GetDefaultNxTabTextColor()

	if (GetRemotePropertyInt("ResShowSchedPats", 0, 0, GetCurrentUserName(), false)) {
		// Create the scheduled patients dialog (will automatically be filled by the scheduler update below)
		m_pdlgScheduledPatients = new CScheduledPatientsDlg(this);
		m_pdlgScheduledPatients->Create(m_pdlgScheduledPatients->IDD, this);
		m_pdlgScheduledPatients->ShowWindow(SW_SHOW);
	}

	UpdateView();//Updates the first sheet (may appear blank otherwise)
	return 0;
}

void CSchedulerView::StoreDetails(CNxDialog *oldSheet) 
{
}

void CSchedulerView::ClearDispatchClip(CNxDialog *pSheet)
{
	if (pSheet == m_pClipSheet) {
		m_pClipSheet = NULL;
		m_pDispatchClip = NULL;
	}
}

const COleDateTime &CSchedulerView::GetPivotDate()
{
	return m_PivotDate;
}

BOOL CSchedulerView::SetPivotDate(const COleDateTime & newDate)
{
	if (newDate.GetStatus()) return FALSE;
	
	m_PivotDate.SetDate(newDate.GetYear(), newDate.GetMonth(), newDate.GetDay());
	return TRUE;
}

void CSchedulerView::GetPivotDate(COleDateTime & theDate)
{
	theDate = m_PivotDate;
}

void CSchedulerView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	pInfo->SetMinPage(1);
	pInfo->SetMaxPage(1);
//	pInfo->SetMaxPage((m_OneDaySheet.m_OneDayCtrl.GetSlotTotalCount() *
//		m_OneDaySheet.m_OneDayCtrl.GetSlotTotalCount()) / (12 * 60) + 1);
	
	CNxTabView::OnBeginPrinting(pDC, pInfo);
}

void CSchedulerView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	// CAH 11/12/01: 
	// If in the process of printing out the scheduler form, the
	// m_bShowApptShowState flag is set to false, we set it back to true.
	// Printing is the only time it should ever be set to false.
	if (m_pActiveSheet && !m_bShowApptShowState) {
		m_bShowApptShowState = TRUE;
		try {
			((CNxSchedulerDlg*)m_pActiveSheet)->m_bNeedUpdate = true;
			((CNxSchedulerDlg*)m_pActiveSheet)->UpdateReservations(false,false);
			((CNxSchedulerDlg*)m_pActiveSheet)->UpdateBlocks(true, true);
			// Now we've successfully updated the controls so we just have to enable them now
			((CNxSchedulerDlg*)m_pActiveSheet)->m_pEventCtrl.PutEnabled(VARIANT_TRUE);
			((CNxSchedulerDlg*)m_pActiveSheet)->m_pSingleDayCtrl.PutEnabled(VARIANT_TRUE);
		} NxCatchAllCall("CSchedulerView::OnEndPrinting", ((CNxSchedulerDlg*)m_pActiveSheet)->ClearAndDisable());
	}
	else m_bShowApptShowState = TRUE;
	m_bShowMonthProcedures = FALSE;
	m_bShowTemplateColors = FALSE;	
	m_bPrintTemplates = FALSE; // (c.haag 2008-06-18 16:14) - PLID 26135

	// (c.haag 2003-10-20 11:21) - Percolate the event to the active sheet
	if (m_pActiveSheet) {
		m_pActiveSheet->PostPrint();
	}

	CNxTabView::OnEndPrinting(pDC, pInfo);
}

void CSchedulerView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	if (m_pActiveSheet) {
		m_pActiveSheet->Print(pDC, pInfo);
	}

	CView::OnPrint(pDC, pInfo);	
}

// NOTE: This function is NOT virtual in the base class, so our implementation 
// is not polymorphic.  I have no idea why Microsoft didn't make it virtual, but 
// I fear there is a reason.  I can't figure it out, but this seems to work.
BOOL CSchedulerView::DoPreparePrinting(CPrintInfo *pInfo)
{
	// Get the current handles
	extern CPracticeApp theApp;
	HANDLE hAppOldDevMode = theApp.GetDevModeHandle();
	HANDLE hAppOldDevNames = theApp.GetDevNamesHandle();
	
	// Set the new handles (if the handles aren't set, just leave the app's handles as is)
	if (pInfo->m_pPD->m_pd.hDevMode) theApp.SetDevModeHandle(pInfo->m_pPD->m_pd.hDevMode);
	if (pInfo->m_pPD->m_pd.hDevNames) theApp.SetDevNamesHandle(pInfo->m_pPD->m_pd.hDevNames);
	
	// Do the standard prepare printing (which uses the CWinApp's handles)
	BOOL bPrepared = CNxTabView::DoPreparePrinting(pInfo);

	// Return the handles to their original values
	theApp.SetDevModeHandle(hAppOldDevMode);
	theApp.SetDevNamesHandle(hAppOldDevNames);

	// Return the result of the DoPreparePrinting call
	return bPrepared;
}

BOOL CSchedulerView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	//check to see if there are any resources
	if (sdrUnknownResource == GetActiveResourceID()) {
		MsgBox("Please select a resource before trying to print");
		return FALSE;
	}

	// (d.thompson 2008-12-04) - PLID 32336 - We cannot let the user print if a ResEntry dialog is active
	//	on the screen.  Mostly it's just weird, but the finalizing of the print causes the screen to refresh, 
	//	which can wipe out your ResEntry CReservation object, and cause errors.
	if(m_ResEntry != NULL && m_ResEntry->m_Res != NULL) {
		MsgBox("You may not attempt to print while editing an appointment.  Please commit or cancel your changes "
			"and try printing again.");
		return FALSE;
	}
	
	CNxSchedulerDlg *pActiveSht = (CNxSchedulerDlg*)GetActiveSheet();
	
	CPrintScheduleDlg dlgPrint(this);
	
	// Prepare Dialog 
	dlgPrint.m_dtReportCenterDate = GetPivotDate();
	dlgPrint.m_strActiveItemID.Format("%li", GetActiveResourceID());
	dlgPrint.m_nPurposeSetId = m_nPurposeSetId;
	//(a.wilson 2012-2-16) PLID 39893 - adding the location filter for scheduling reports
	// (r.gonet 06/10/2013) - PLID 56503 - Pass the location IDs currently being filtered on. 
	//  If we are filtered on All Locations, then the dwLocations array will be empty.
	CDWordArray dwLocations;
	long nLocationID = pActiveSht->m_nActiveLocationId;
	if(nLocationID > 0) {
		// (r.gonet 06/11/2013) - PLID 56503 - Single Location ID
		dlgPrint.m_dwLocations.Add(nLocationID);
	} else if(nLocationID == -1) {
		// (r.gonet 06/11/2013) - PLID 56503 - All locations. Passing no location IDs to the print dialog will mean this.
	} else if(nLocationID == -2) {
		// (r.gonet 06/11/2013) - PLID 56503 - Multiple locations.
		pActiveSht->GetLocationFilterIDs(&dwLocations);
		for(int i = 0; i < dwLocations.GetSize(); i++) {
			dlgPrint.m_dwLocations.Add(dwLocations.GetAt(i));
		}
	}
	if (pActiveSht == &m_OneDaySheet) {
		dlgPrint.m_nPrintExtent = 0;
		dlgPrint.m_strMultiTypeIds = m_OneDaySheet.m_strMultiTypeIds;
	} else if (pActiveSht == &m_WeekDaySheet) {
		dlgPrint.m_nPrintExtent = 1;
		dlgPrint.m_dtReportEndDate = GetPivotDate() + COleDateTimeSpan(((CNxSchedulerDlg*)GetActiveSheet())->m_pSingleDayCtrl.GetDayVisibleCount()-1, 0, 0, 0);
		dlgPrint.m_strMultiTypeIds = m_WeekDaySheet.m_strMultiTypeIds;
	} else if (pActiveSht == &m_MonthSheet) {
		dlgPrint.m_nPrintExtent = 2;
		dlgPrint.m_strMultiTypeIds = m_MultiResourceSheet.m_strMultiTypeIds;
	} else if (pActiveSht == &m_MultiResourceSheet) {
		dlgPrint.m_nPrintExtent = 3;
		dlgPrint.m_strMultiTypeIds = m_MonthSheet.m_strMultiTypeIds;
	}
	
	// Dialog is prepared.  Now, open it.  If the user clicks OK,that means
	// he/she wants to proceed with either the print or the preview.  Only
	// if the print is to be Graphical (ie, not the MS Access report) do we
	// continue with the standard print/preview
	if (dlgPrint.ZoomPrint(pInfo) == IDOK) {

		if (dlgPrint.m_bShowApptShowState == 0)
		{
			m_bShowApptShowState = dlgPrint.m_bShowApptShowState;
			try {
				pActiveSht->m_bNeedUpdate = true;
				pActiveSht->UpdateReservations(false, false);
				pActiveSht->UpdateBlocks(true, true);
				// Now we've successfully updated the controls so we just have to enable them now
				pActiveSht->m_pEventCtrl.PutEnabled(VARIANT_TRUE);
				pActiveSht->m_pSingleDayCtrl.PutEnabled(VARIANT_TRUE);
			} NxCatchAllCall("CSchedulerView::OnPreparePrinting", pActiveSht->ClearAndDisable());
		}
		m_bShowMonthProcedures = dlgPrint.m_bShowMonthProcedures;
		m_bShowTemplateColors = dlgPrint.m_bShowTemplateColors;
		// (c.haag 2008-06-18 16:14) - PLID 26135 - Set to true if we want to print templates in the month view.
		// The flag is only used by the month view. If the flag is set, and we're in the month view, then the
		// month view needs to do some special prep work for the printing.
		if ((m_bPrintTemplates = dlgPrint.m_bPrintTemplates)) {
			CNxSchedulerDlg *pActiveSht = (CNxSchedulerDlg*)GetActiveSheet();
			if (&m_MonthSheet == pActiveSht) {
				m_MonthSheet.PrepareTemplatePrint();
			}
		}

		// The m_lpUserData is used strictly as a NXSINGLEDAYPRINT structure that 
		// tells the singleday how it should print the templates
		m_nxspSingledayPrintInfo.bPrintBlockColor = m_bShowTemplateColors;
		m_nxspSingledayPrintInfo.bPrintBlockText = m_bShowTemplateText;
		m_nxspSingledayPrintInfo.bPrintGrid = dlgPrint.m_bPrintGrid;
		pInfo->m_lpUserData = reinterpret_cast<void*>(&m_nxspSingledayPrintInfo);

		if (dlgPrint.GetPrintMode() == CPrintScheduleDlg::pmGraphical) {

			if (DoPreparePrinting(pInfo)) {
				return CView::OnPreparePrinting(pInfo);
			}
		}
	}	
	return FALSE;
}

CResEntryDlg * CSchedulerView::GetResEntry()
{
	return m_ResEntry;
}

void CSchedulerView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	
	ObtainExtraInfo();

#ifdef RES_AUTO_REFRESH	
	SetTimer(RES_AUTO_REFRESH_TIMER, RES_AUTO_REFRESH_TIME, NULL);
#endif

	if (m_pActiveSheet) m_pActiveSheet->RecallDetails();
}

CString AddWhereClausePart(IN const CString &strCurrentWhere, IN LPCTSTR strFmtPartToAdd, ...)
{
	if (strFmtPartToAdd && (*strFmtPartToAdd) != _T('\0')) {
		// Calculate the string that we're going to add
		CString strPartToAdd;
		{
			// Parse string inserting arguments where appropriate
			va_list argList;
			va_start(argList, strFmtPartToAdd);
			strPartToAdd.FormatV(strFmtPartToAdd, argList);
			va_end(argList);

			// Trim it too
			strPartToAdd.TrimLeft();
			strPartToAdd.TrimRight();
		}

		// Now add the part we just generated
		if (!strPartToAdd.IsEmpty()) {
			CString strAns(strCurrentWhere);
			strAns.TrimRight();
			if (strAns.IsEmpty()) {
				// The current where clause was empty so we don't need to put an AND in there, just return the part we're adding by itself
				return strPartToAdd;
			} else {
				// Need to add an AND and the end of the current where, and then add our new part afterwardsd
				return strAns + " AND " + strPartToAdd;
			}
		} else {
			// Nothing to add, just return whatever it was
			return strCurrentWhere;
		}
	} else {
		// Nothing to add, just return whatever it was
		return strCurrentWhere;
	}
}

// This function should ONLY be called from the CSchedulerView's OnInitialUpdate() or SetCurrentResourceViewID() functions.
// DO NOT BE FOOLED, you shouldn't reload the resource list in response to table checker messages!  This 
// list is how the current user on the current machine expects to see things.  To change it behind the 
// user's back would be confusing.
void CSchedulerView::LoadResourceList(IN const long nNewResourceViewID, IN const long nTrySelectDefaultResourceID)
{
	// First clear everything so we guarantee we can't get out of this function without having done so
	m_nLastLoadedResourceViewID = srvUnknownResourceView;
	m_strLastLoadedResourceViewName.Empty();
	m_nCurrentResourceID = sdrUnknownResource;
	m_arypCurrentResourceList.RemoveAllAndFreeEntries(); 

	try {
		// Generate the where and from clauses
		CString strWhere, strFrom;
		// (d.thompson 2010-05-06) - PLID 38525 - Parameterized completely if no hidden resource IDs.  Not otherwise, but we try our best.
		_CommandPtr pCmd = OpenParamQuery("");
		{
			// Find out which view we're looking at
			ASSERT(nNewResourceViewID >= 0 || nNewResourceViewID == srvCurrentUserDefaultView);
			if (nNewResourceViewID != srvCurrentUserDefaultView) {
				// Just a normal ID so set the from clause and the base where clause to use ResourceViewDetailsT
				strWhere = "ResourceViewDetailsT.ResourceViewID = ? AND Relevence >= 0 AND ResourceT.Inactive = 0 ";
				AddParameterLong(pCmd, "ViewID", nNewResourceViewID);
				strFrom = "ResourceT INNER JOIN ResourceViewDetailsT ON ResourceT.ID = ResourceViewDetailsT.ResourceID";
			} else {
				// We're using the user's default view so set the from clause and the base where clause to use UserResourcesT
				strWhere = "(UserResourcesT.UserID = ?) AND Relevence >= 0 AND ResourceT.Inactive = 0 ";
				AddParameterLong(pCmd, "UserID", GetCurrentUserID());
				strFrom = "ResourceT INNER JOIN UserResourcesT ON ResourceT.ID = UserResourcesT.ResourceID";
			}

			// Generate a list of the resource IDs this user is not allowed to see
			CString strHiddenResourceIDs = CalcExcludedResourceIDs();
			// Now add to the where clause if there are any resources that need to explicitly be excluded
			if (!strHiddenResourceIDs.IsEmpty()) {
				// (d.thompson 2010-05-06) - PLID 38525 - We cannot parameterize this if it's included
				strWhere = AddWhereClausePart(strWhere, "ResourceT.ID NOT IN (%s)", strHiddenResourceIDs);
			}
		}

		// Fill our array (m_aryCurrentResourceList) and set our current id (m_nCurrentResourceID); clear these 
		// variables first for code completeness
		m_arypCurrentResourceList.RemoveAllAndFreeEntries(); 
		m_nCurrentResourceID = sdrUnknownResource;
		{
			// Loop through the recordset generated based on the from and where clauses calculated above
			ASSERT(!strFrom.IsEmpty() && !strWhere.IsEmpty());
			// (d.thompson 2010-05-06) - PLID 38525 - Parameterized completely if no hidden resource IDs.  Not otherwise, but we try our best.
			CString strQuery;
			strQuery.Format(
				"SELECT ResourceT.ID, ResourceT.Item, DefaultInterval "
				"FROM %s "
				"WHERE %s "
				"ORDER BY Relevence DESC", strFrom, strWhere);
			pCmd->PutCommandText(_bstr_t(strQuery));
			_RecordsetPtr prs = CreateRecordset(pCmd);
			FieldsPtr pflds = prs->GetFields();
			FieldPtr fldResourceID = pflds->GetItem("ID");
			FieldPtr fldResourceName = pflds->GetItem("Item");
			FieldPtr fldDefaultInterval = pflds->GetItem("DefaultInterval");
			while (!prs->eof) {
				// Get the ID
				long nResourceID = AdoFldLong(fldResourceID);
				// See if the caller wants us to set a certain default resource id and if so, see if the id we're 
				// adding right now matches the one the caller wants
				if (nTrySelectDefaultResourceID != sdrUnknownResource && nResourceID == nTrySelectDefaultResourceID) {
					m_nCurrentResourceID = nResourceID;
				}
				// Add the id and it's name to the array
				m_arypCurrentResourceList.Add(new CResourceEntry(nResourceID, AdoFldString(fldResourceName),AdoFldLong(fldDefaultInterval,-1)));
				// Move to the next resource in the recordset
				prs->MoveNext();
			}
		}

		// If our "current resource id" wasn't set by the loop, then we need to pick a default ourselves, we 
		// choose the first one in the list
		if (m_nCurrentResourceID == sdrUnknownResource) {
			if (m_arypCurrentResourceList.GetSize() > 0) {
				// We're just picking the first entry
				m_nCurrentResourceID = m_arypCurrentResourceList.GetAt(0)->m_nResourceID;
			} else {
				// This view doesn't have any resources (at least none that the current user is allowed to see)
				m_nCurrentResourceID = sdrUnknownResource;
			}
		}
		// Finally our view is loaded so set our member variable
		m_nLastLoadedResourceViewID = nNewResourceViewID;

		// (c.haag 2006-04-12 12:59) - PLID 20104 - Store the name of the current resource view
		LoadResourceViewName();
	} NxCatchAllCall("CSchedulerView::LoadResourceList", {
		m_arypCurrentResourceList.RemoveAllAndFreeEntries(); 
		m_nCurrentResourceID = sdrUnknownResource;
		m_nLastLoadedResourceViewID = srvUnknownResourceView;
		m_strLastLoadedResourceViewName.Empty();
	});

	// At this point, we finally have our complete and ordered list of resources and we have an ID for the 
	// resource that is officially the current resource, so everyone who bases their interface on this 
	// information needs to be notified that the values are now set
	NotifyChildrenOfResourceListRefresh();
	NotifyChildrenOfActiveResourceChange();
	UpdateView();
}

void CSchedulerView::NotifyChildrenOfResourceListRefresh()
{
	try {
		// Tell all tabs what their official resource list must contain
		if (m_OneDaySheet.GetSafeHwnd()) {
			((CSchedulerViewSheet &)m_OneDaySheet).Internal_ReflectCurrentResourceListOnGUI();
		}
		if (m_WeekDaySheet.GetSafeHwnd()) {
			((CSchedulerViewSheet &)m_WeekDaySheet).Internal_ReflectCurrentResourceListOnGUI();
		}
		if (m_MultiResourceSheet.GetSafeHwnd()) {
			((CSchedulerViewSheet &)m_MultiResourceSheet).Internal_ReflectCurrentResourceListOnGUI();
		}
		if (m_MonthSheet.GetSafeHwnd()) {
			((CSchedulerViewSheet &)m_MonthSheet).Internal_ReflectCurrentResourceListOnGUI();
		}

	} NxCatchAllCall("CSchedulerView::NotifyChildrenOfResourceListRefresh", {
		try { m_OneDaySheet.ClearAndDisable(); } catch(...) { };
		try { m_WeekDaySheet.ClearAndDisable(); } catch(...) { };
		try { m_MultiResourceSheet.ClearAndDisable(); } catch(...) { };
		try { m_MonthSheet.ClearAndDisable(); } catch(...) { };
	});
}

void CSchedulerView::NotifyChildrenOfActiveResourceChange()
{
	try {
		// Tell all tabs what their official resource list must contain
		if (m_OneDaySheet.GetSafeHwnd()) {
			((CSchedulerViewSheet &)m_OneDaySheet).Internal_ReflectActiveResourceOnGUI();
		}
		if (m_WeekDaySheet.GetSafeHwnd()) {
			((CSchedulerViewSheet &)m_WeekDaySheet).Internal_ReflectActiveResourceOnGUI();
		}
		if (m_MultiResourceSheet.GetSafeHwnd()) {
			((CSchedulerViewSheet &)m_MultiResourceSheet).Internal_ReflectActiveResourceOnGUI();
		}
		if (m_MonthSheet.GetSafeHwnd()) {
			((CSchedulerViewSheet &)m_MonthSheet).Internal_ReflectActiveResourceOnGUI();
		}

	} NxCatchAllCall("CSchedulerView::NotifyChildrenOfActiveResourceChange", {
		try { m_OneDaySheet.ClearAndDisable(); } catch(...) { };
		try { m_WeekDaySheet.ClearAndDisable(); } catch(...) { };
		try { m_MultiResourceSheet.ClearAndDisable(); } catch(...) { };
		try { m_MonthSheet.ClearAndDisable(); } catch(...) { };
	});
}

// Returns a comma-delimited list of the resource ids that are in the current view
CString CSchedulerView::CalcCurrentResourceIDs()
{
	CString strAns;

	long nCount = m_arypCurrentResourceList.GetSize();
	for (long i=0; i<nCount; i++) {
		CString str;
		str.Format("%li, ", m_arypCurrentResourceList.GetAt(i)->m_nResourceID);
		strAns += str;
	}

	DeleteEndingString(strAns, ", ");
	
	return strAns;
}

// Returns a query string that when executed will return a recordset of all the resource ids in the 
// current view, with the optional exception of the active resource.  You might wonder why we 
// wouldn't just select the ids from the ResourceViewDetailsT or the UserResourcesT; the reason is 
// that we want the results to be bound to the interface not the data.  So if someone on another 
// machine has changed the list of resources associated with m_nLastLoadedResourceViewID it won't 
// affect the results of this query.
// NOTE: If there are no resources in the current view this function just returns an empty string
CString CSchedulerView::CalcCurrentResourceIDsUnionQuery(BOOL bIncludeRelevence)
{
	CString strAns;

	// Loop through all the current resources, adding a "SELECT ## UNION ALL" for each one
	long nCount = m_arypCurrentResourceList.GetSize();
	for (long i=0; i<nCount; i++) {
		// Start out with nothing
		CString strEntry = "";

		// Add the id part (always add this)
		{
			// Put the id in a string
			CString strResourceID;
			strResourceID.Format("%li", m_arypCurrentResourceList.GetAt(i)->m_nResourceID);
			// If this is the first entry, we need to add " AS ResourceID" after it so the column has a name
			if (strAns.IsEmpty()) {
				strResourceID += " AS ResourceID";
			}
			// Add this text to the entry
			strEntry += strResourceID;
		}

		// Optionally add the relevence part
		if (bIncludeRelevence) {
			// Put the relevence value 
			CString strRelevence;
			strRelevence.Format(", %li", nCount - i);
			// If this is the first entry, we need to add " AS Relevence" after it so the column has a name
			if (strAns.IsEmpty()) {
				strRelevence += " AS Relevence";
			}
			// Add this text to the entry
			strEntry += strRelevence;
		}

		// Then plug what we've got into the "SELECT [plug_it_in_here] UNION ALL" string and add it to our running sql string
		strAns += "SELECT " + strEntry + " UNION ALL ";
	}

	// Drop the final " UNION ALL" if there is one
	DeleteEndingString(strAns, " UNION ALL ");
	
	// Return the result
	return strAns;
}

const CResourceEntryPtrArray &CSchedulerView::GetCurrentResourceList() const
{
	return m_arypCurrentResourceList;
}

const CResourceEntry *CSchedulerView::FindResourceInCurrentList(IN const long nResourceID) const
{
	// Loop through the list of resources to see if the given id matches any of them
	long nCount = m_arypCurrentResourceList.GetSize();
	for (long i=0; i<nCount; i++) {
		const CResourceEntry *pre = m_arypCurrentResourceList.GetAt(i);
		if (nResourceID == pre->m_nResourceID) {
			// We found it YES it's in the list
			return pre;
		}
	}

	// If we made it here, it wasn't found
	return NULL;
}

const CResourceEntry *CSchedulerView::FindResourceInCurrentList(const CDWordArray &arydwResourceIDs) const
{
	// Loop through the given list of ids and see if each id can be found in our current list of resources 
	long nCount = arydwResourceIDs.GetSize();
	for (long i=0; i<nCount; i++) {
		const CResourceEntry *pre = FindResourceInCurrentList(arydwResourceIDs.GetAt(i));
		if (pre) {
			// We found it YES it's in the list
			return pre;
		}
	}

	// If we made it here it wasn't found
	return NULL;
}

BOOL CSchedulerView::IsResourceInCurView(IN const long nResourceID) const
{
	if (nResourceID == sdrUnknownResource) {
		// It's the special "unknown resource" so of course it's not in the list
		return FALSE;
	} else {
		// Search for the resource in this view
		if (FindResourceInCurrentList(nResourceID)) {
			// Found it
			return TRUE;
		} else {
			// Didn't find it
			return FALSE;
		}
	}
}

BOOL CSchedulerView::AreAllResourcesInCurView(IN const CDWordArray &arydwResourceIDs) const
{
	// Loop through the given array, seeing if each item is in the resource view
	long nCount = arydwResourceIDs.GetSize();
	for (long i=0; i<nCount; i++) {
		// See if this one's not in the current view
		if (!IsResourceInCurView(arydwResourceIDs.GetAt(i))) {
			// This resource is not in the current view so now we know that not all of them 
			// are, so we can return false immediately
			return FALSE;
		}
	}
	
	// If we made it here, all given resources are in the current view
	return TRUE;
}

BOOL CSchedulerView::AreAnyResourcesInCurView(IN const CDWordArray &arydwResourceIDs) const
{
	// Loop through the given array, seeing if each item is in the resource view
	long nCount = arydwResourceIDs.GetSize();
	for (long i=0; i<nCount; i++) {
		// See if this one's in the current view
		if (IsResourceInCurView(arydwResourceIDs.GetAt(i))) {
			// This resource is in the current view so now we know that at least one of them 
			// is, so we can return true immediately
			return TRUE;
		}
	}
	
	// If we made it here, none of the given resources are in the current view
	return FALSE;
}

// (j.jones 2014-08-14 13:30) - PLID 63184 - added function that tells is if any resource
// is in our current filter, not necessarily the resource view
bool CSchedulerView::AreAnyResourcesInCurrentFilter(IN const CDWordArray &arydwResourceIDs) const
{
	CNxSchedulerDlg *pActiveSht = (CNxSchedulerDlg*)GetActiveSheet();
	if (pActiveSht && pActiveSht == &m_MultiResourceSheet)
	{
		//if we're in Resource view, just look at our actual resource view
		return AreAnyResourcesInCurView(arydwResourceIDs) ? true : false;
	}
	else {
		//any other view filters by only one resource at a time
		for (int i = 0; i < arydwResourceIDs.GetSize(); i++) {
			if (m_nCurrentResourceID == (long)arydwResourceIDs.GetAt(i)) {
				return true;
			}
		}
		//not found
		return false;
	}
}

// Changes what the current resource id is and notifies everyone of the change
// NOTE: this function should only be called in response to the user deliberately changing what resource he or she wants to look at
// throws exceptions
void CSchedulerView::SetActiveResourceID(long nNewResourceID, BOOL bDoUpdateViewNow)
{
	// See if we've been given the special "unknown resource"
	if (nNewResourceID == sdrUnknownResource) {
		// We were given the special "unknown resource" so set it to that; this means when anyone tries to get the value they will get an exception
		m_nCurrentResourceID = sdrUnknownResource;
		// Notify all tabs of the change
		NotifyChildrenOfActiveResourceChange();
		// Optionally update the view
		if (bDoUpdateViewNow) {
			UpdateView();
		}
	} else {
		// See if the given resource id is in our current view's list of resources
		if (IsResourceInCurView(nNewResourceID)) {
			// It's perfectly valid so set it
			m_nCurrentResourceID = nNewResourceID;
			// Notify all tabs of the change
			NotifyChildrenOfActiveResourceChange();
			// Optionally update the view
			if (bDoUpdateViewNow) {
				UpdateView();
			}
		} else {
			// It wasn't found in our array so the caller screwed up.  Set it to unknown and throw an exception.
			m_nCurrentResourceID = sdrUnknownResource;
			// Notify all tabs of the change
			NotifyChildrenOfActiveResourceChange();
			// Optionally update the view
			if (bDoUpdateViewNow) {
				UpdateView();
			}
			// Throw the exception
			ThrowNxException("CSchedulerView::SetActiveResourceID: Cannot set the current resource to %li because that id was not loaded!", nNewResourceID);
		}
	}
}

long CSchedulerView::GetActiveResourceID() const
{
	// (j.jones 2003-07-03 11:42) there are a few valid ways to get a view with no resources, such as deleting
	//resources from a view or removing all permissions from resources in a view. In these cases,
	//we must allow access to the scheduler, but you won't be able to do anything except edit resource views.
	if(GetCurrentResourceList().GetSize() == 0) {
		return sdrUnknownResource;
	}

	// (b.cardillo 2003-06-30 18:03) TODO: Both GetActiveResourceID and GetActiveDate (when it's implemented) should 
	// take the active sheet just like GetLastLoadedResourceViewID does.
	if (m_nCurrentResourceID != sdrUnknownResource) {
		// Find it in our array
		if (IsResourceInCurView(m_nCurrentResourceID)) {
			// We found it so return it
			return m_nCurrentResourceID;
		} else {
			// It wasn't found in our array!  This is completely unexpected and not allowed.
			ThrowNxException("CSchedulerView::GetActiveResourceID: The current resource id %li was not loaded!", m_nCurrentResourceID);
		}
	} else {
		// You're not allowed to ask for the active resource id when there is none
		ThrowNxException("CSchedulerView::GetActiveResourceID: There is no current resource id right now!");		
	}
}

LRESULT CSchedulerView::OnUpdateView(WPARAM wParam, LPARAM lParam)
{
	BeginWaitCursor();
	if (m_pActiveSheet) {
		((CNxSchedulerDlg *)m_pActiveSheet)->m_bNeedUpdate = true;
		//TES 3/31/2011 - PLID 41519 - If we were given an appointment to highlight, pass that on to our sheet.
		if(wParam) {
			((CNxSchedulerDlg *)m_pActiveSheet)->m_nPendingHighlightID = (long)wParam;
		}
	}
	UpdateView();
	EndWaitCursor();
	return 0;
}

void CSchedulerView::OnTimer(UINT nIDEvent) 
{
	if (nIDEvent == RES_AUTO_REFRESH_TIMER) {
		UpdateView();
	} else {
		CView::OnTimer(nIDEvent);
	}
}

long CSchedulerView::GetTopSlot()
{
	return m_nTopSlot;
}

long CSchedulerView::GetInterval()
{
	return m_nInterval;
}

void CSchedulerView::SwitchToTab(IN const ESwitchToTab esttNewTab)
{
	// For now we only handle the predefined tabs
	switch (esttNewTab) {
	case sttOneDayView:
		// Right now this is hard-coded to be the first tab
		SetActiveTab(0);
		break;
	case sttWeekView:
		// Right now this is hard-coded to be the second tab
		SetActiveTab(1);
		break;
	case sttMultiResourceView:
		// Right now this is hard-coded to be the third tab
		SetActiveTab(2);
		break;
	case sttMonthView:
		// Right now this is hard-coded to be the fourth tab
		SetActiveTab(3);
		break;
	default:
		ASSERT(FALSE);
		ThrowNxException("CSchedulerView::SwitchToTab(%li): Invalid tab identifier given", esttNewTab);
		break;
	}
}

void CSchedulerView::OnResEdit()
{
	try {
		// (a.walling 2007-08-31 10:29) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (IS_VALID_DISPATCH(m_pResDispatch)) {
			((CNxSchedulerDlg *)GetActiveSheet())->OnReservationClick(m_pResDispatch);
		}
	} NxCatchAll("Error in CSchedulerView::OnResEdit"); // (a.walling 2007-08-31 10:06) - PLID 27265
}

void CSchedulerView::OnResCopy()
{
	try {
		if (!CheckCurrentUserPermissions(bioAppointment, sptCreate)) {
			return;
		}

		BeginWaitCursor();
		// (a.walling 2007-08-31 10:29) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (IS_VALID_DISPATCH(m_pResDispatch)) {
			CReservation pRes(__FUNCTION__, m_pResDispatch);
			long nResID = pRes.GetReservationID();		
			long nPatID = AppointmentGetPatientID(nResID);

			

			// Save the reservation ID and resource ID to the clipboard
			CClipRes *pClipRes = new CClipRes;
			pClipRes->m_nResID = nResID;
			COleDateTime dtTmp;
			GetWorkingResourceAndDate((CNxSchedulerDlg*)GetActiveSheet(), pRes.GetDay(), pClipRes->m_nResourceID, dtTmp);
			GetMainFrame()->SetClip(CLIP_RESERVATION, pClipRes);
			//MainFrame will handle the delete.
			
			// Save the actual reservation object for use later
			m_pClipSheet = m_pActiveSheet;
			m_pDispatchClip = m_pResDispatch;

			// Make sure we're not tracking the modified date
			ResetTrackedAppt();
		}
	} NxCatchAll("Error in CSchedulerView::OnResCopy"); // (a.walling 2007-08-31 10:06) - PLID 27265

	EndWaitCursor();
}

void CSchedulerView::OnResCut()
{
	try {
		//(e.lally 2011-07-15) PLID 40889 - Need to check for create permission too.
		// (j.jones 2011-12-23 11:40) - PLID 47191 - except Administrators have all permissions!
		// Rather than checking for Administrator, just confirm they do not have the regular create/cut perms.
		// For non-Administrators it would presumably be bad data if they have both, but support it anyways.
		CPermissions permsAppointment = GetCurrentUserPermissions(bioAppointment);
		if((permsAppointment & (sptCreateWithPass|sptDynamic1WithPass))
			&& (!(permsAppointment & sptCreate) || !(permsAppointment & sptDynamic1))
			){
			//Check password one time
			if(!CheckCurrentUserPassword()){
				PermissionsFailedMessageBox("", "access this function without entering your password");
				return;
			}
		}

		if (!CheckCurrentUserPermissions(bioAppointment, sptCreate, 0, 0, FALSE, TRUE)) {
			return;
		}

		if (!CheckCurrentUserPermissions(bioAppointment, sptDynamic1, 0, 0, FALSE, TRUE)) {
			return;
		}

		CWaitCursor wc;
		// (a.walling 2007-08-31 10:29) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (IS_VALID_DISPATCH(m_pResDispatch)) {
			CReservation pRes(__FUNCTION__, m_pResDispatch);
			long nResID = pRes.GetReservationID();	

			
			// (a.walling 2013-01-17 09:26) - PLID 54651 - Check for any appointments linked to EMNs
			CString strLinkedEMNs = GetLinkedEMNDescriptionsFromAppointment(nResID);
			if (!strLinkedEMNs.IsEmpty()) {
				AfxMessageBox(FormatString("This appointment is linked to the following EMN data and may not be cut:\r\n\r\n%s", strLinkedEMNs), MB_ICONERROR);
				return;
			}

			// (c.haag 2006-05-10 08:31) - PLID 20505 - Collect information about the appointment
			// and some patient warnings here so that we don't have to run several queries down the
			// road if we don't have to
			// (j.jones 2009-09-17 10:33) - PLID 35572 - ensure we do not warn about inactive insurances
			// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
			// (j.jones 2010-08-02 11:23) - PLID 39937 - WarnCoPay is now a preference, and no longer a setting per insured party,
			// but we can detect if they even have copays in this query
			// (j.luckoski 2013-03-04 13:56) - PLID 33548 - DisplayRewardsWarning
			_RecordsetPtr prs = CreateParamRecordset("SELECT PatientID, AppointmentsT.ModifiedDate, DisplayWarning, "
				"CASE WHEN PersonT.ID IN (SELECT PatientID FROM InsuredPartyT "
				"	INNER JOIN InsuredPartyPayGroupsT ON InsuredPartyT.PersonID = InsuredPartyPayGroupsT.InsuredPartyID "
				"	WHERE (CoPayMoney Is Not Null OR CopayPercentage Is Not Null) AND RespTypeID <> -1) THEN 1 ELSE 0 END AS HasCoPay, "
				"DisplayAllergyWarning, PersonT.DisplayRewardsWarning "
				"FROM AppointmentsT "
				"LEFT JOIN PersonT ON PersonT.ID = AppointmentsT.PatientID "
				"LEFT JOIN PatientsT ON PatientsT.PersonID = PersonT.ID "
				"WHERE AppointmentsT.ID = {INT}", nResID);
			if (!prs->eof) {
				FieldsPtr flds = prs->Fields;
				long nPatID = AdoFldLong(flds, "PatientID");
				COleDateTime dtModified = AdoFldDateTime(flds, "ModifiedDate");
				ShowPatientWarning(nPatID,
					AdoFldBool(flds, "DisplayWarning") ? FALSE : TRUE,
					FALSE, // Insurance referral warnings are complicated, so let this function handle it 
					AdoFldLong(flds, "HasCoPay") ? FALSE : TRUE,
					AdoFldBool(flds, "DisplayAllergyWarning") ? FALSE : TRUE,
					AdoFldBool(flds, "DisplayRewardsWarning") ? FALSE : TRUE);
				prs->Close();

				// Save the reservation ID to the clipboard
				CClipRes * pClipRes = new CClipRes;
				pClipRes->m_nResID = nResID;
				COleDateTime dtTmp;
				GetWorkingResourceAndDate((CNxSchedulerDlg*)GetActiveSheet(), pRes.GetDay(), pClipRes->m_nResourceID, dtTmp);
				GetMainFrame()->SetClip(CLIP_RESERVATION, pClipRes, CLIP_METHOD_CUT);
				//MainFrame will handle the delete
				
				// Save the actual reservation object for use later
				m_pClipSheet = m_pActiveSheet;
				m_pDispatchClip = m_pResDispatch;

				// Track the appointment modified date
				ResetTrackedAppt(); // Do this in case we cut the modified appointment after it was modified
				TrackLastModifiedDate(nResID, dtModified);
			} else {
				MsgBox("The appointment could not be cut because it has been removed from the system; possibly by another user.");
				// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
				PostMessage(NXM_UPDATEVIEW);
			}
		}
	}
	NxCatchAll("Error cutting appointment");
}

void CSchedulerView::OnResPaste()
{
	try {
		CWaitCursor waitCursor;
		// (a.walling 2007-08-31 10:32) - PLID 27265 - This function treats -1 as a special case for m_pResDispatch
		if (m_pResDispatch) {
			int nMethod;
			CClip *pClip;
			GetMainFrame()->GetClip(CLIP_RESERVATION, pClip, nMethod);
			if (!pClip || !pClip->IsKindOf(RUNTIME_CLASS(CClipRes))) {
				return;
			}

			CClipRes* p = (CClipRes*)pClip;

			long nClipResID = p->m_nResID;
			long nClipResourceID = p->m_nResourceID;

			// (c.haag 2003-08-07 17:32) - Check to see if the appointment was modified
			CString strModifiedLogin;
			if (WasTrackedApptModified(FALSE, strModifiedLogin))
			{
				if (IDNO == MsgBox(MB_YESNO, "This appointment was changed by %s since you started to modify it. Do you wish to overwrite that user's changes?", strModifiedLogin))
				{
					return;
				}
			}
			ResetTrackedAppt();

			long nPasteResID = PasteAppointment(nClipResID, nClipResourceID, nMethod);
			if (nPasteResID != -1) {
				if (nMethod == CLIP_METHOD_CUT) {
					GetMainFrame()->ClearClip();
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2015-01-28 13:08) - PLID 64412 - need to know if source is the rescheduling queue
long CSchedulerView::PasteAppointment(long nClipResID, long nClipResourceID, int nMethod, bool fromReschedulingQueue)
{
	long nPasteResID = -1;
	try {
		{
			// Only proceed if we have gotten good data from our 'clip'board
			if (nClipResID > 0) {
				// (c.haag 2005-10-18 16:15) - PLID 17990 - Get the old reservation ID and its start
				// time so we can send a table checker message with that information
				//
				COleDateTime dtOldStart, dtOldEnd;
				long nOldResID;
				long nShowState = 0;
				long nOldLocationID = -1;
				CString strOldResourceIDs;

				if (nMethod == CLIP_METHOD_CUT) {
					dtOldStart.m_status = COleDateTime::invalid;
					dtOldEnd.m_status = COleDateTime::invalid;
					nOldResID = nClipResID;
					// (j.jones 2014-08-14 10:11) - PLID 63167 - we now get the location and resourceIDs
					_RecordsetPtr pApptFrom = AppointmentGrab(nOldResID, TRUE);
					if (pApptFrom) {
						dtOldStart = AdoFldDateTime(pApptFrom, "StartTime", dtOldStart);
						dtOldEnd = AdoFldDateTime(pApptFrom, "EndTime", dtOldEnd);
						nShowState = AdoFldLong(pApptFrom, "ShowState");
						nOldLocationID = AdoFldLong(pApptFrom, "LocationID");
						strOldResourceIDs = AdoFldString(pApptFrom, "ResourceIDs");
						pApptFrom->Close();
					}
				}

				bool bWantsReschedule = false;
				// If this is a cut operation and we're not in the multi-resource view,
				// find out if we should cancel or delete the old appointment
				if (nMethod == CLIP_METHOD_CUT) {
					int result;

					result = AfxMessageBox("Would you like to retain the original as a cancelled appointment?", 
						MB_ICONQUESTION|MB_YESNOCANCEL);
					switch (result) {
					case IDYES:
						bWantsReschedule = true;
						break;
					case IDNO:
						bWantsReschedule = false;
						break;
					case IDCANCEL:
					default:
						return -1;
						break;
					}
				}

				//DRT 7/8/2005 - PLID 16664 - We need to get a list of all superbill IDs tied to 
				//	this appointment.  Then once it's pasted, we need to fix them, or void them, 
				//	based on user feedback.
				bool bVoidSuperbills = false;
				CDWordArray arySuperbillID;
				//DRT 10/20/2005 - PLID 16664 - This only should be prompting if we cut/paste, not copy/paste
				if (nMethod == CLIP_METHOD_CUT && (GetRemotePropertyInt("ShowVoidSuperbillPrompt", 1, 0, "<None>", false) == 1) && (GetCurrentUserPermissions(bioVoidSuperbills) & sptWrite)) {
					// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
					_RecordsetPtr prsSuperbill = CreateParamRecordset("SELECT SavedID FROM PrintedSuperbillsT WHERE ReservationID = {INT} AND Void = 0", nClipResID);
					CString strSuperbillIDs = "";
					long nSuperbillIDCnt = 0;
					while(!prsSuperbill->eof) {
						long nID = AdoFldLong(prsSuperbill, "SavedID", -1);
						if(nID > -1) {
							CString str;	str.Format("%li, ", nID);
							strSuperbillIDs += str;
							arySuperbillID.Add(nID);
							nSuperbillIDCnt++;
						}

						prsSuperbill->MoveNext();
					}
					strSuperbillIDs.TrimRight(", ");
					prsSuperbill->Close();

					if(nSuperbillIDCnt > 0) {
						//This appointment is tied to superbills, we will warn the user and give them an opportunity to give up
						CString strFmt;

						// (j.luckoski 2013-05-08 17:10) - PLID 53490 - If you cancel the appt, keep superbills on it except if its selected to void.
						if(!bWantsReschedule) {
							strFmt.Format("This appointment is tied to %li superbill%s (ID%s:  %s).  "
								"Do you wish to mark these superbills as VOID?\r\n\r\n"
								" - If you choose YES, the appointment will be moved and all related superbill IDs will be marked VOID.\r\n"
								" - If you choose NO, the appointment will be moved and all related superbill IDs will continue to be tied to this appointment.\r\n"
								" - If you choose CANCEL, this appointment will not be pasted.", nSuperbillIDCnt, nSuperbillIDCnt == 1 ? "" : "s", nSuperbillIDCnt == 1 ? "" : "s", strSuperbillIDs);
						} else {
							strFmt.Format("This appointment is tied to %li superbill%s (ID%s:  %s).  "
								"Do you wish to mark these superbills as VOID?\r\n\r\n"
								" - If you choose YES, the appointment will be moved and all related superbill IDs will be marked VOID.\r\n"
								" - If you choose NO, all related superbills will remain tied to this cancelled appointment.\r\n"
								" - If you choose CANCEL, this appointment will not be pasted.", nSuperbillIDCnt, nSuperbillIDCnt == 1 ? "" : "s", nSuperbillIDCnt == 1 ? "" : "s", strSuperbillIDs);
						}
						
						int nRes = AfxMessageBox(strFmt, MB_YESNOCANCEL);
						switch(nRes) {
						case IDYES:
							bVoidSuperbills = true;
							break;
						case IDNO:
							//do nothing
							break;
						case IDCANCEL:
						default:
							//Quit
							return -1;
							break;
						}
					} 
				}

				//TES 11/30/2010 - PLID 41664 - We used to track whether we created a new appointment, the alternative being that
				// we were overwriting an existing appointment.  But if you look at the below code, we took away the option to overwrite
				// an appointment back in 2006, so we're always creating new.  The result was that if CopyRes() failed, we weren't deleting
				// the temporary appointment we had created.
				//bool bHadToCreateNew = false;
				CString tmpSearchStr;
				long tmpPatID = AppointmentGetPatientID(nClipResID);

				if ((long)m_pResDispatch == -1) {
					// m_pResDispatch is -1 so we should make a new appointment and point to it

					// Set up an empty array to be our list of purposes (we're making the appointment without purposes because 
					// it's just a shell of an appointment record, we'll be updating it completely momentarily in the CopyRes)
					CDWordArray adwEmpty;

					// (c.haag 2003-07-29 11:12) - Make sure the resource exists
					// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
					// (j.luckoski 2012-05-09 16:07) - PLID 50264 - User ReturnsRecordsParam
					if (!ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT ID FROM ResourceT WHERE ID = {INT}", m_clickTargetInfo.resourceID))
					{
						AfxMessageBox("The appointment could not be saved because the resource you attempted to paste it to was deleted by another user.");
						return -1;
					}

					// Get the list of resources (right now we're guaranteed to have only one because m_pResDispatch is -1 so 
					// the "click" info describes the part of a singleday control that the user clicked on.
					CDWordArray arydwClickResourceID;
					arydwClickResourceID.Add(m_clickTargetInfo.resourceID);

					// Make the appointment (either as an event or as a normal appointment)
					if(m_clickTargetInfo.isEvent) {
						nPasteResID = AppointmentCreate(tmpPatID, arydwClickResourceID, GetCurrentLocationID(), m_clickTargetInfo.date, 
							COleDateTime(m_clickTargetInfo.date.GetYear(), m_clickTargetInfo.date.GetMonth(), m_clickTargetInfo.date.GetDay(),0,0,0), 
							COleDateTime(m_clickTargetInfo.date.GetYear(), m_clickTargetInfo.date.GetMonth(), m_clickTargetInfo.date.GetDay(),0,0,0), 
							COleDateTime(m_clickTargetInfo.date.GetYear(), m_clickTargetInfo.date.GetMonth(), m_clickTargetInfo.date.GetDay(),0,0,0), 
							false, false, false, "", -1, adwEmpty, FALSE, -1, FALSE);
					}
					else {
						nPasteResID = AppointmentCreate(tmpPatID, arydwClickResourceID, GetCurrentLocationID(), m_clickTargetInfo.date, 
							m_clickTargetInfo.time, COleDateTime(m_clickTargetInfo.date.GetYear(), m_clickTargetInfo.date.GetMonth(), m_clickTargetInfo.date.GetDay(),9,0,0), 
							m_clickTargetInfo.time, false, false, false, "", -1, adwEmpty, FALSE, -1, FALSE);
					}
				/*
				} else if (GetActiveSheet() == &m_MultiResourceSheet) {
					MsgBox("This operation is not supported in the resource view.");
					EndWaitCursor();
					return;
				*/
				} else {
					/*// m_pResDispatch is not NULL and is not -1 so it must be valid
					int result = AfxMessageBox("Are you sure you want to overwrite this appointment?", 
						MB_ICONQUESTION|MB_YESNO);
					if (result == IDYES) {
						CReservation pasteRes(m_pResDispatch);
						nPasteResID = pasteRes.GetReservationID();
					} else {
						//nPasteResID = -1;
						EndWaitCursor();
						return;
					}*/

					//TES 8/7/2006 - PLID 21409 - We no longer allow pasting over an appointment, so always create new.
					// Make the appointment (either as an event or as a normal appointment)
					CReservation pRes(__FUNCTION__, m_pResDispatch);
					CDWordArray arydwClickResourceID, adwEmpty;
					arydwClickResourceID.Add(m_clickTargetInfo.resourceID);
					nPasteResID = AppointmentCreate(tmpPatID, arydwClickResourceID, GetCurrentLocationID(), 
						COleDateTime(m_clickTargetInfo.date.GetYear(), m_clickTargetInfo.date.GetMonth(), m_clickTargetInfo.date.GetDay(), 0, 0, 0),
						pRes.GetStartTime(), pRes.GetEndTime(), pRes.GetStartTime(), 
						false, false, false, "", -1, adwEmpty, FALSE, -1, FALSE);			
				}
					
				// Do nothing if the user tried to paste to the clipped appointment
				if (nPasteResID == nClipResID) {
					return nPasteResID;
				}

				if (!CopyRes(nClipResID, nClipResourceID, nPasteResID, nMethod, bWantsReschedule, fromReschedulingQueue)) {
					// The copy res didn't work, so delete the 
					// temporarily created appointment if it's there
					if (nPasteResID > 0) {
						if (!AppointmentDeleteNoHistory(nPasteResID, true)) {
							CString str;
							str.Format(CString(
								"A temporary appointment could not be deleted.  Please delete manually.\n\n") +
								"Date: %s\nTime: %s", FormatDateTimeForInterface(m_clickTargetInfo.date, "%x"), FormatDateTimeForInterface(m_clickTargetInfo.time, DTF_STRIP_SECONDS, dtoTime));
							AfxMessageBox(str, MB_OK|MB_ICONEXCLAMATION);
						}
					}
				}
				else
				{
					if (fromReschedulingQueue) {
						Nx::Scheduler::RemoveFromReschedulingQueue(nClipResID); // (a.walling 2015-01-09 13:47) - PLID 64410
					}
					AppointmentCheckPatientProvider(tmpPatID, nPasteResID);

					//DRT 7/8/2005 - PLID 16664 - The appointment is now moved to nPasteResID, we need to fix their superbills.
					try {
						//DRT 10/20/2005 - PLID 16664 - This only should be prompting if we cut/paste, not copy/paste
						// (j.luckoski 2013-05-08 17:11) - PLID 53490 - If the old appt was cancelled and you selected not to void then do nothing, 
						//else allow it to be voided if chosen to do so, or allow the old choices of void and move to the new appt if the old one is gone.
						if (nMethod == CLIP_METHOD_CUT) {
							if(arySuperbillID.GetSize() > 0 && !(!bVoidSuperbills && bWantsReschedule)) {
								//This is a list of all non-void superbills.  Due to the way functions are called when pasting (deleting the old, creating a new), 
								//	these superbills will have been marked as VOID and then had the appointment ID removed.  We'll have to patch all that up 
								//	manually to fix it.
								for(int i = 0; i < arySuperbillID.GetSize(); i++) {
									long nSuperbillID = arySuperbillID.GetAt(i);

									//We have 2 states -- either the user said to void them all, or they said to move them.
									//	- If voiding, they already are void.  We just need to fix them back to this new appointment
									//		for data consistency.
									//	- If moving, we have to un-void them, then move fix the data consistency of the appointment ID.
									CString strUpdate, strTemp;
									strUpdate.Format("UPDATE PrintedSuperbillsT SET ");
									if(!bWantsReschedule) {
										strTemp.Format("ReservationID = %li,", nPasteResID);
										strUpdate += strTemp;
									} 
									
									if(!bVoidSuperbills) {
										//Undo the void action
										strUpdate += "Void = 0, VoidDate = NULL, VoidUser = ''";
									}

									else {
										//DRT 5/19/2006 - PLID 20712 - According to the comments above, these should have already been
										//	marked as void.  But somewhere along the line, someone changed that ability elsewhere (I
										//	think it would have been in the appointment creation code -- but I cannot find the specifics).
										//It is safest to have the voiding code here, rather than rely on other changes to work around
										//	this belief, so we will now mark the superbill IDs as void here.
										CString strTmp;
										strTmp.Format("Void = 1, VoidDate = GetDate(), VoidUser = '%s'", _Q(GetCurrentUserName()));
										strUpdate += strTmp;
									}

									//add the where
									CString str;	str.Format(" WHERE SavedID = %li", nSuperbillID);
									strUpdate += str;

									ExecuteSqlStd(strUpdate);
								}
							}
						}
					} NxCatchAll("Error updating superbill IDs.  Your superbill IDs may not have been correctly transferred to the pasted appointment.");

					// Now send the table checker message about the
					// new appointment
					//CClient::RefreshTable(NetUtils::AppointmentsT, nPasteResID);

					// (c.haag 2005-10-18 16:15) - PLID 17990 - Send a table checker with the old reservation ID
					// and its start time. This way, the receiving end will know it was for this
					// time (and therefore ignore it if it's out of range), and know what the ID
					// was, then not find it in data, and then remove it.
					//
					// We have to have a valid start time, or else it won't work.
					//
					if (nMethod == CLIP_METHOD_CUT) {
						if (dtOldStart.m_status != COleDateTime::invalid) {
							// (j.jones 2007-09-06 15:19) - PLID 27312 - required the EndTime as a parameter
							// (j.jones 2014-08-05 10:35) - PLID 63167 - added PatientID, LocationID, ResourceIDs
							CClient::RefreshAppointmentTable(nOldResID, tmpPatID, dtOldStart, dtOldEnd, 4 /* 4 means cancelled */, nShowState, nOldLocationID, strOldResourceIDs);
						}
					}
					// (j.jones 2006-10-23 10:33) - PLID 22887 - send a message if this changed a room
					// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
					_RecordsetPtr prs = CreateParamRecordset("SELECT Appt.PatientID, Appt.LocationID, Appt.Status, Appt.ShowState, Appt.StartTime, Appt.EndTime, dbo.GetResourceIDString(Appt.ID) AS ResourceIDs, "
						"RA.ID RaID, RoomID, StatusID FROM RoomAppointmentsT RA INNER JOIN AppointmentsT Appt ON RA.AppointmentID = Appt.ID WHERE Appt.ID = {INT}", nPasteResID);
					if (!prs->eof) {
						CClient::RefreshAppointmentTable(nPasteResID, AdoFldLong(prs, "PatientID"), AdoFldDateTime(prs, "StartTime"), AdoFldDateTime(prs, "EndTime"), 
							(long)AdoFldByte(prs, "Status"), AdoFldLong(prs, "ShowState"), AdoFldLong(prs, "LocationID"), AdoFldString(prs, "ResourceIDs", ""));
						CClient::RefreshRoomAppointmentTable(AdoFldLong(prs, "RaID"), AdoFldLong(prs, "RoomID"), nPasteResID, AdoFldLong(prs, "StatusID"));
					}
					prs->Close();
				}
			}
		}
		return nPasteResID;
	} NxCatchAll(__FUNCTION__);
	return -1;
}

void CSchedulerView::OnResDelete()
{
	try {
		if (!CheckCurrentUserPermissions(bioAppointment, SPT________2__))
			return;
		BeginWaitCursor();
		BOOL bIsDirty = IsSchedulerDirty();
		// (a.walling 2007-08-31 10:29) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (IS_VALID_DISPATCH(m_pResDispatch)) {
			long nDelResID;
			CReservation delRes(__FUNCTION__, m_pResDispatch);
			// Get the CReservation's id
			nDelResID = delRes.GetReservationID();
			long nPatID = AppointmentGetPatientID(nDelResID);
			ShowPatientWarning(nPatID);

			if (AppointmentCancel(nDelResID)) {
				// If we successfully deleted it from the data, then delete it from the view
				// (c.haag 2010-07-13 12:22) - PLID 39615 - Dereference from this view before deleting the reservation
				m_pResDispatchPtr.SetDispatch(NULL);
				m_pResDispatch = (LPDISPATCH)-1;

				// (j.luckoski 2012-04-30 09:15) - PLID 11597 - Don't delete and then redraw if preference is checked, just let 
				// updateView remove the cancelled appt from the list itself.
				// delRes.DeleteRes(__FUNCTION__);
			
				// (j.dinatale 2010-09-01) - PLID 39686 - Only if we cancel an appointment do we want to update our count labels.
				//		UpdateView does not update the view if we are using the right click context menu, so its best to just do it here.
				dynamic_cast<CNxSchedulerDlg *>( GetActiveSheet())->EnsureCountLabelText();
			}
			// Check if it was dirty before we tried to delete
			if (bIsDirty) {
				// If it was dirty, just refresh
				// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
				PostMessage(NXM_UPDATEVIEW);
			} else {
				SchedulerClean();
			}
		}
	} NxCatchAll("Error in CSchedulerView::OnResDelete"); // (a.walling 2007-08-31 10:06) - PLID 27265
	EndWaitCursor();
}

void CSchedulerView::OnResDeleteBlock()
{
	// (z.manning 2008-06-12 11:50) - PLID 29273 - Check template delete permission (even though
	// we aren't technically deleting a template or a template line item, from the user's perspective
	// we are).
	if(!CheckCurrentUserPermissions(bioSchedTemplating,sptDelete)) {
		return;
	}

	// (c.haag 2006-12-06 08:48) - PLID 23666 - This function is called when a
	// user wants to delete a scheduler template block 
	CWaitCursor wc;
	// (a.walling 2007-08-31 10:29) - PLID 27265 - Ensure valid m_pResDispatch pointer
	if (IS_VALID_DISPATCH(m_pResDispatch)) {
		try {
			long nDelTemplateItemID;
			CReservation delRes(__FUNCTION__, m_pResDispatch);
			CNxSchedulerDlg *dlgScheduler = (CNxSchedulerDlg *)GetActiveSheet();
			// Get the CReservation's template item id
			nDelTemplateItemID = delRes.GetTemplateItemID();
			if (-1 != nDelTemplateItemID) {
				if (IDNO == MsgBox(MB_ICONQUESTION | MB_YESNO, "This action will delete the precision template for today's date only. Are you sure you wish to continue?")) {
					return; // You get nothing, good day sir!
				}

				// (c.haag 2006-12-11 10:39) - PLID 23808 - Safety check - Make sure the block exists
				// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
				// (j.luckoski 2012-05-09 16:07) - PLID 50264 - User ReturnsRecordsParam
				if(!ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM TemplateItemT WHERE ID = {INT}", nDelTemplateItemID)) {
					MsgBox("This template item has been deleted by another user.");
				}
				else {
					// If we get here, there is no stopping us from deleting the template line item.
					// All we really need to do is determine the working date of the reservation block
					// as it is on the screen, and then add a line item exception for the template item
					long nColumn = delRes.GetDay();
					COleDateTime dtBlock = dlgScheduler->GetWorkingDate(nColumn);
					CString strBlock = FormatDateTimeForSql(dtBlock);
					// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
					// (j.luckoski 2012-05-09 16:07) - PLID 50264 - User ReturnsRecordsParam
					if (ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM TemplateItemExceptionT WHERE TemplateItemID = {INT} AND [Date] = {STRING}",
						nDelTemplateItemID, strBlock))
					{
						MsgBox("Another user has already removed this precision template from this specific day.");					
					}
					else {
						// (d.thompson 2010-05-06) - PLID 38525 - Parameterized. Also removed a NewNumber in favor of an inline ID lookup.
						ExecuteParamSql(
							"INSERT INTO TemplateItemExceptionT (ID, TemplateItemID, [Date]) "
							"SELECT (SELECT COALESCE(Max(ID), 0) + 1 FROM TemplateItemExceptionT), {INT}, {STRING} FROM TemplateItemT WHERE ID = {INT}",
							nDelTemplateItemID, strBlock, nDelTemplateItemID);
						// (c.haag 2006-12-11 16:28) - PLID 23808 - Notify everyone about the change
						CClient::RefreshTable(NetUtils::TemplateItemT, nDelTemplateItemID);
					}
				}
				
				// Invalidate and refresh the schedule
				dlgScheduler->InvalidateData();
			}
			else {
				// This should never happen!
				ASSERT(FALSE);
			}
		}
		NxCatchAll("Error deleting precision template");
	} // if (m_pResDispatch) {
}

void CSchedulerView::OnResMarkNoShow()
{
	try {
		// (a.walling 2007-08-31 10:35) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (!UserPermission(EditAppointment) || !IS_VALID_DISPATCH(m_pResDispatch))
			return;

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nPatID = AppointmentGetPatientID(pRes.GetReservationID());
		ShowPatientWarning(nPatID);

		if (AppointmentMarkNoShow(pRes.GetReservationID())) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			PostMessage(NXM_UPDATEVIEW);
		}
	} NxCatchAll("Error in CSchedulerView::OnResMarkNoShow"); // (a.walling 2007-08-31 10:06) - PLID 27265
}

void CSchedulerView::OnResMarkShow()
{
	try {
		// (a.walling 2007-08-31 10:36) - PLID 27265 - Ensure valid m_pResDispatch pointer - Also, no permission checks here? interesting.
		if (!IS_VALID_DISPATCH(m_pResDispatch))
			return;

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nPatID = AppointmentGetPatientID(pRes.GetReservationID());
		ShowPatientWarning(nPatID);

		if (AppointmentMarkPending(pRes.GetReservationID())) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			PostMessage(NXM_UPDATEVIEW);
		}
	} NxCatchAll("Error in CSchedulerView::OnResMarkShow"); // (a.walling 2007-08-31 10:06) - PLID 27265
}

void CSchedulerView::OnResMarkIn()
{
	try {
		// (a.walling 2007-08-31 10:35) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (!UserPermission(EditAppointment) || !IS_VALID_DISPATCH(m_pResDispatch))
			return;

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nPatID = AppointmentGetPatientID(pRes.GetReservationID());
		ShowPatientWarning(nPatID);

		// (b.spivey, June 01, 2012) - PLID 47435 - Need to check for insurance referrals when marking in
		COleDateTime dtAppt;
		long nTemp = 0; //Placeholder just so I can get date. 
		GetWorkingResourceAndDate((CNxSchedulerDlg*)GetActiveSheet(), pRes.GetDay(), nTemp, dtAppt);

		//Is it valid?
		if (dtAppt.m_status == COleDateTime::null || dtAppt.m_status == COleDateTime::invalid) {
			dtAppt = GetDateTimeZero(); 
		}

		if(!AttemptWarnForInsAuth(nPatID, dtAppt)) {
			return; //they decided they didn't want to mark the patient in. 
		}

		if (AppointmentMarkIn(pRes.GetReservationID())) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			PostMessage(NXM_UPDATEVIEW);
		}
	} NxCatchAll("Error in CSchedulerView::OnResMarkIn"); // (a.walling 2007-08-31 10:06) - PLID 27265
}

void CSchedulerView::OnResMarkOut()
{
	try {
		// (a.walling 2007-08-31 10:35) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (!UserPermission(EditAppointment) || !IS_VALID_DISPATCH(m_pResDispatch))
			return;

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nPatID = AppointmentGetPatientID(pRes.GetReservationID());
		ShowPatientWarning(nPatID);

		if (AppointmentMarkOut(pRes.GetReservationID())) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			PostMessage(NXM_UPDATEVIEW);
		}
	} NxCatchAll("Error in CSchedulerView::OnResMarkOut"); // (a.walling 2007-08-31 10:06) - PLID 27265
}

void CSchedulerView::OnResMarkPending()
{
	try {
		// (a.walling 2007-08-31 10:35) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (!UserPermission(EditAppointment) || !IS_VALID_DISPATCH(m_pResDispatch))
			return;

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nPatID = AppointmentGetPatientID(pRes.GetReservationID());
		ShowPatientWarning(nPatID);

		if (AppointmentMarkPending(pRes.GetReservationID())) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			PostMessage(NXM_UPDATEVIEW);
		}
	} NxCatchAll("Error in CSchedulerView::OnResMarkPending"); // (a.walling 2007-08-31 10:06) - PLID 27265
}

void CSchedulerView::OnResMarkReceived()
{
	try {
		// (a.walling 2007-08-31 10:36) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (!UserPermission(EditAppointment) || !IS_VALID_DISPATCH(m_pResDispatch))
			return;

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nPatID = AppointmentGetPatientID(pRes.GetReservationID());
		ShowPatientWarning(nPatID);

		if (AppointmentMarkReceived(pRes.GetReservationID())) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			PostMessage(NXM_UPDATEVIEW);
		}
	} NxCatchAll("Error in CSchedulerView::OnResMarkReceived"); // (a.walling 2007-08-31 10:06) - PLID 27265 ((a.walling 2008-05-22 13:14) - fixed a typo)
}

void CSchedulerView::OnResMarkUserDefinedShowState(const CString& strShowState)
{
	try {
		// (a.walling 2007-08-31 10:37) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (!UserPermission(EditAppointment) || !IS_VALID_DISPATCH(m_pResDispatch))
			return;

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nPatID = AppointmentGetPatientID(pRes.GetReservationID());
		ShowPatientWarning(nPatID);

		if (AppointmentMarkUserDefined(pRes.GetReservationID(), strShowState)) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			PostMessage(NXM_UPDATEVIEW);
		}
	} NxCatchAll("Error in CSchedulerView::OnResMarkUserDefinedShowState (" + strShowState + ")"); // (a.walling 2007-08-31 10:06) - PLID 27265
}

void CSchedulerView::OnResMarkConfirmed()
{
	try {
		// (a.walling 2007-08-31 10:35) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (!UserPermission(EditAppointment) || !IS_VALID_DISPATCH(m_pResDispatch))
			return;

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nPatID = AppointmentGetPatientID(pRes.GetReservationID());
		ShowPatientWarning(nPatID);

		//(e.lally 2009-09-28) PLID 18809 - Send in the integer value, not a boolean
		if (AppointmentMarkConfirmed(pRes.GetReservationID(), acsConfirmed)) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			PostMessage(NXM_UPDATEVIEW);
		}
	} NxCatchAll("Error in CSchedulerView::OnResMarkConfirmed"); // (a.walling 2007-08-31 10:06) - PLID 27265
}

void CSchedulerView::OnResMarkUnConfirmed()
{
	try {
		// (a.walling 2007-08-31 10:35) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (!UserPermission(EditAppointment) || !IS_VALID_DISPATCH(m_pResDispatch))
			return;

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nPatID = AppointmentGetPatientID(pRes.GetReservationID());
		ShowPatientWarning(nPatID);

		//(e.lally 2009-09-28) PLID 18809 - Send in the integer value, not a boolean
		if (AppointmentMarkConfirmed(pRes.GetReservationID(),acsUnconfirmed)) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			PostMessage(NXM_UPDATEVIEW);
		}
	} NxCatchAll("Error in CSchedulerView::OnResMarkUnConfirmed"); // (a.walling 2007-08-31 10:06) - PLID 27265
}

//(e.lally 2009-09-28) PLID 18809 - Created function for marking an appt Left Message for the confirmed field.
void CSchedulerView::OnResMarkLeftMessage()
{
	try {
		//Ensure valid m_pResDispatch pointer
		if (!UserPermission(EditAppointment) || !IS_VALID_DISPATCH(m_pResDispatch)){
			return;
		}

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nPatID = AppointmentGetPatientID(pRes.GetReservationID());
		ShowPatientWarning(nPatID);

		//(e.lally 2009-09-28) PLID 18809 - Send in the integer value, not a boolean
		if (AppointmentMarkConfirmed(pRes.GetReservationID(),acsLeftMessage)) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			PostMessage(NXM_UPDATEVIEW);
		}
	} NxCatchAll(__FUNCTION__);
}

void CSchedulerView::OnResMarkMoveUp()
{
	try {
		//TES 12/17/2008 - PLID 32478 - This ability is only available in the Enterprise edition, so check the license.
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Designating appointments as requesting a move up", "The_Scheduler_Module/manage_my_moveup_appointments.htm")) {
			return;
		}

		// (a.walling 2007-08-31 10:35) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (!UserPermission(EditAppointment) || !IS_VALID_DISPATCH(m_pResDispatch))
			return;

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nPatID = AppointmentGetPatientID(pRes.GetReservationID());
		ShowPatientWarning(nPatID);

		if (AppointmentMarkMoveUp(pRes.GetReservationID(),TRUE)) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			PostMessage(NXM_UPDATEVIEW);
		}
	} NxCatchAll("Error in CSchedulerView::OnResMarkMoveUp"); // (a.walling 2007-08-31 10:06) - PLID 27265
}

void CSchedulerView::OnResRemoveMoveUp()
{
	try {
		//TES 12/17/2008 - PLID 32478 - This ability is only available in the Enterprise edition, so check the license.
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Designating appointments as requesting a move up", "The_Scheduler_Module/manage_my_moveup_appointments.htm")) {
			return;
		}

		if (!UserPermission(EditAppointment) || !m_pResDispatch)
			return;

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nPatID = AppointmentGetPatientID(pRes.GetReservationID());
		ShowPatientWarning(nPatID);

		if (AppointmentMarkMoveUp(pRes.GetReservationID(),FALSE)) {
			// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
			PostMessage(NXM_UPDATEVIEW);
		}
	} NxCatchAll("Error in CSchedulerView::OnResRemoveMoveUp"); // (a.walling 2007-08-31 10:06) - PLID 27265
}

void CSchedulerView::OnEditTemplates()
{
	try{

		//(e.lally 2010-07-28) PLID 39887 - We aren't using the res, so release our reference to it.
		m_pResDispatchPtr.SetDispatch(NULL);
		m_pResDispatch = (LPDISPATCH)-1;
		if (CheckCurrentUserPermissions(bioSchedTemplating, sptRead)) {
			// (z.manning 2014-12-01 14:56) - PLID 64205 - Pass in the editor type
			CTemplateItemEntryGraphicalDlg dlgTemplateEditor(stetNormal, this);
			dlgTemplateEditor.SetDefaultResourceID(GetActiveResourceID());
			// (j.jones 2011-07-15 14:36) - PLID 39838 - give it the date we clicked on
			if(m_clickTargetInfo.date.GetStatus() == COleDateTime::valid) {
				dlgTemplateEditor.SetDefaultDate(m_clickTargetInfo.date);
			}
			dlgTemplateEditor.DoModal();

			CNxSchedulerDlg *dlgScheduler = (CNxSchedulerDlg *)GetActiveSheet();
			BeginWaitCursor();
			dlgScheduler->UpdateBlocks(true,true);
			CWnd::FromHandle((HWND)dlgScheduler->m_pSingleDayCtrl.GethWnd())->RedrawWindow();
			dlgScheduler->m_pSingleDayCtrl.Refresh();		
			if(dlgScheduler->m_pEventCtrl) {
				CWnd::FromHandle((HWND)(dlgScheduler->m_pEventCtrl.GethWnd()))->RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE);
				dlgScheduler->m_pEventCtrl.Refresh();
			}
			EndWaitCursor();
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-07-15 14:48) - PLID 39838 - added OnEditLocationTemplates
void CSchedulerView::OnEditLocationTemplates()
{
	try {

		m_pResDispatchPtr.SetDispatch(NULL);
		m_pResDispatch = (LPDISPATCH)-1;

		if(GetMainFrame()) {
			// (j.jones 2011-07-15 14:49) - PLID 39838 - send in the date we clicked on
			GetMainFrame()->EditLocationTemplating(m_clickTargetInfo.date);
		}
	}NxCatchAll(__FUNCTION__);	
}

// (z.manning 2014-12-01 17:40) - PLID 64205
void CSchedulerView::OnEditTemplateCollections()
{
	try
	{
		m_pResDispatchPtr.SetDispatch(NULL);
		m_pResDispatch = (LPDISPATCH)-1;

		if(GetMainFrame() != NULL) {
			GetMainFrame()->OpenSchedulerTemplateCollectionEditor();
		}
	}
	NxCatchAll(__FUNCTION__);
}

// (c.haag 2004-03-05 14:56) - New paradigm:
//
// o Rather than interleaved SQL execution and selection, we clearly separate the actions. We gather
//		everything first, and then execute all the logic in one big SQL statement. I proved with
//		NxQuery and with manually messing up the SQL statement in code that the entire statement is
//		rolled back if any part of it fails. The statement is also logged if it fails.
// o Subroutines like AppointmentCancel and AppointmentUpdate have been changed or overloaded to
//		support this paradigm
// o Auditing still happens in the same places. In the event of a failure, the audits are logically
//		rolled back (refer to NxCatchAllCode)
//
// (a.walling 2015-01-28 13:08) - PLID 64412 - need to know if source is the rescheduling queue
bool CSchedulerView::CopyRes(long fromResID, long fromResourceID, long toResID, long nMethod, bool bWantReschedule, bool fromReschedulingQueue)
{
	BOOL bWasDirty;
	_RecordsetPtr pApptFrom;
	_RecordsetPtr pApptTo;
	FieldsPtr pFromFlds;
	FieldsPtr pToFlds;
	// Other handy variables
	COleDateTime sTime, eTime;
	COleDateTime dtOldDate, dtOldStart, dtOldEnd, dtOldArrival;
	COleDateTime dtNewDate, dtNewStart, dtNewEnd, dtNewArrival;
	COleDateTime dtOverwrittenStart;
	long nPatientID, nLocationID, nAptTypeID, nOverwrittenPatientID, nOverwrittenApptID, nConfirmed, nRecordID = -1;
	COleDateTime dtNewCreatedDate;
	CString strNewCreatedLogin;
	COleDateTimeSpan dtDuration;
	CDWordArray arydwFromResourceIDs;
	CDWordArray arydwFinalResourceIDs, arydwFinalPurposeIDs;
	CString strSQLFinal;
	CString strConfirmedBy; // (j.armen 2011-11-17 10:52) - PLID 44205 - Added Confirmed By
	long nAuditFrom = -1, nAuditTo = -1, nAuditOverwrite = -1, nAuditDelete = -1;
	COleDateTime dtOldCancelled, dtNewCancelled;
	long nShowState; // (b.eyers 2015-03-12) - PLID 65288 
	CString strNotes; // (b.eyers 2015-03-12) - PLID 65288 
	// (d.moore 2007-10-16) - PLID 26546 - We need to keep track of the MoveUp status
	//  for the appointment being copied. This will be used later to determine if we
	//  need to check the waiting list for possible matches.
	BOOL bMoveUp = FALSE;

	long nOldStatus = 1;

	std::vector<SchedulerMixRule> overriddenMixRules;
	SelectedFFASlotPtr pSelectedFFASlot;
	pSelectedFFASlot.reset();
		

	// (a.walling 2015-01-29 09:30) - PLID 64746 - cache some resource names; describeResources now sorts lexicographically, case insensitive			
	std::map<DWORD, CString> resourceNameCache;
	auto describeResource = [&resourceNameCache](DWORD id)->CString {
		auto it = resourceNameCache.find(id);
		if (it != resourceNameCache.end()) {
			return it->second;
		}
		auto prs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Item FROM ResourceT WHERE ID = {INT}", id);
		CString name;
		if (prs && !prs->eof) {
			name = AdoFldString(prs, "Item", "");
		}
		resourceNameCache.emplace(id, name);
		return name;
	};
	auto describeResources = [&resourceNameCache, describeResource](const CDWordArray& dwa) {
		boost::container::flat_set<CiString> names;
		for (int i = 0; i < dwa.GetSize(); ++i) {
			names.insert(describeResource(dwa[i]));
		}
		CString str;
		for (const auto& name : names) {
			str += name;
			str += ", ";
		}
		str.TrimRight(", ");
		return str;
	};

	// (c.haag 2004-03-05 10:23) - Part 1: Gather information we need to write the
	// appointment with, and do some validation. No writing is done here.
	try {
		// Find out if we are dirty right now 
		bWasDirty = IsSchedulerDirty();
		// Set pApptFrom to point to the fromRes
		pApptFrom = AppointmentGrab(fromResID);
		if (pApptFrom == NULL)
		{
			// Failure
			AfxMessageBox("Could not copy appointment.  Try again after refreshing.");
			return false;
		}
		// Set pApptTo to point to the toRes
		pApptTo = AppointmentGrab(toResID);
		if (pApptTo == NULL)
		{
			return false;
		}
		// Get the fields object from the recordset pointer
		pFromFlds = pApptFrom->Fields;
		// Get the fields object from the recordset ptr
		pToFlds = pApptTo->Fields;

		//(e.lally 2006-07-11) PLID 19269 - Audit the appointment that was overwritten
		nOverwrittenPatientID = AdoFldLong(pToFlds, "PatientID", -25);
		dtOverwrittenStart = AdoFldDateTime(pToFlds, "StartTime", COleDateTime(0, 0, 0, 8, 0, 0));
		nOverwrittenApptID = VarLong(pToFlds->Item["ID"]->Value);
		ASSERT(nOverwrittenApptID == toResID);

		nOldStatus = AdoFldByte(pApptFrom, "Status", 1);

		// (c.haag 2005-07-11 16:46) - Read from the data fields here
		dtOldDate = AdoFldDateTime(pFromFlds, "Date");
		dtOldStart = sTime = AdoFldDateTime(pFromFlds, "StartTime", COleDateTime(0, 0, 0, 8, 0, 0));
		dtOldEnd = eTime = AdoFldDateTime(pFromFlds, "EndTime", COleDateTime(0, 0, 0, 9, 0, 0));
		dtOldArrival = AdoFldDateTime(pFromFlds, "ArrivalTime", COleDateTime(0, 0, 0, 8, 0, 0));
		nPatientID = AdoFldLong(pFromFlds, "PatientID", -25);
		nLocationID = AdoFldLong(pFromFlds, "LocationID", -1);
		nAptTypeID = AdoFldLong(pFromFlds, "AptTypeID", -1);
		// (d.moore 2007-10-16) - PLID 26546 - Store the MoveUp value for use later when
		//  we need to decide if we should check for matches in the waiting list.
		bMoveUp = AdoFldBool(pFromFlds, "MoveUp", FALSE);
		//TES 8/12/2009 - PLID 16749 - Store the Confirmed value, we may be prompting them whether or not to keep it.
		nConfirmed = AdoFldLong(pFromFlds, "Confirmed", 0);
		// (j.armen 2011-11-17 10:52) - PLID 44205 - Store the ConfirmedBy value, we may be prompting them whethor ot not to keep it.
		strConfirmedBy = AdoFldString(pFromFlds, "ConfirmedBy", "");

		// (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
		dtOldCancelled = AdoFldDateTime(pFromFlds, "CancelledDate", g_cdtInvalid);

		// (j.jones 2014-12-02 09:44) - PLID 64181 - get the primary insured ID
		long nPrimaryInsuredPartyID = AdoFldLong(pFromFlds, "PrimaryInsuredPartyID", -1);

		// (b.eyers 2015-03-12) - PLID 65288 
		nShowState = AdoFldLong(pFromFlds, "ShowState", 0);
		strNotes = AdoFldString(pFromFlds, "Notes", ""); 

		if (eTime < sTime)
		{
			AfxMessageBox("Could not copy an appointment that has an end time before its start time.");
			return false;
		}

		// Update the palm tables before we touch the old appointment
		UpdatePalmSyncT(fromResID);

		CString strFromResourceIDs;
		////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Warn the user if she is pasting to a different set of resources than she copied/cut from
		{
			// Get the IDs of all the resources on which the both the FROM and the TO appointments are based
			CDWordArray arydwToResourceIDs;
			GetAppointmentResourceIDs(fromResID, arydwFromResourceIDs);
			GetAppointmentResourceIDs(toResID, arydwToResourceIDs);

			// (j.jones 2011-02-11 12:01) - PLID 35180 - cache the string of "from" resource IDs
			strFromResourceIDs = GetCommaDeliminatedText(arydwFromResourceIDs);

			// (c.haag 2003-07-29 10:58) - Fail if any destination resource doesn't exist
			if (arydwToResourceIDs.GetSize() > 0) {
				_RecordsetPtr prs;
				// (d.thompson 2010-05-06) - PLID 38525 - Parameterized as best we can, per size.
				_CommandPtr pCmd = OpenParamQuery("");
				CString strSQL;
				strSQL = "SELECT ID FROM ResourceT WHERE ID = ? ";
				AddParameterLong(pCmd, "ResourceID", arydwToResourceIDs[0]);
				for (long i=1; i < arydwToResourceIDs.GetSize(); i++)
				{
					strSQL += "OR ID = ? ";
					AddParameterLong(pCmd, "ResourceID", arydwToResourceIDs[i]);
				}
				pCmd->PutCommandText(_bstr_t(strSQL));
				prs = CreateRecordset(pCmd);
				if (prs->RecordCount < arydwToResourceIDs.GetSize())
				{
					AfxMessageBox("This appointment cannot be saved because at least one resource that would be assigned to it was deleted by another user.");
					return false;
				}
			}

			// (c.haag 2002-07-02 17:00) This is the first logical step in multi-resource cut and
			// paste operations (This goes beyond just the scope of the resource view). If the
			// source appointment has multiple resource ID's, then we need the user to define
			// what resources are carried over. 
			// (b.cardillo 2002-10-22) We now check not just if the source appointment has multiple 
			// resource IDs, but if either the source or the destination has multiple resources
			if (arydwFromResourceIDs.GetSize() > 1 || arydwToResourceIDs.GetSize() > 1)
			{
				// (a.walling 2015-01-29 09:05) - PLID 64413 - Modify prompt if dropping from rescheduling queue
				CString overrideText;
				CString overrideTitle;
				if (fromReschedulingQueue) {
					overrideText = "The appointment you are trying to reschedule has multiple resources. Please check each resource you want to include in the new appointment, and uncheck each resource you do not want to include in the new appointment.";
					overrideTitle = "Multiple Resource Reschedule";
				}
				CMultiResourcePasteDlg dlg(this, overrideTitle, overrideText);
				CDWordArray& adwResources = dlg.m_adwSelected;

				// Combine the list of resources between the source and destination appointments.
				// Yes there can be duplicates, but the multi-resource-paste dialog will discard them.
				adwResources.Copy(arydwToResourceIDs);
				adwResources.Append(arydwFromResourceIDs);

				// If this is a cut operation, we want to remove the resource
				// we are cutting from our resource list (since it makes no
				// sense for the user to want that resource selected).
				if (nMethod == CLIP_METHOD_CUT)
				{
					// (c.haag 2004-02-10 17:24) - PL 10409 - Only do this in the
					// resource view.
					CNxSchedulerDlg *pActiveSht = (CNxSchedulerDlg*)GetActiveSheet();
					if (pActiveSht && pActiveSht == &m_MultiResourceSheet)
					{
						long nIndexInResourcesArray = FindElementInArray(adwResources, fromResourceID);
						if (nIndexInResourcesArray != feiarNotFound) {
							adwResources.RemoveAt(nIndexInResourcesArray);
						}
					}
				}

				// Commit the dialog.
				m_MultiResourceSheet.m_bAllowUpdate = false;
				int nSelectResourcesResult = dlg.DoModal();
				while (nSelectResourcesResult == IDOK && adwResources.GetSize() == 0) {
					MsgBox("Please select at least one resource for the destination appointment.");
					nSelectResourcesResult = dlg.DoModal();
				}
				if (IDCANCEL == nSelectResourcesResult)
				{
					// The user has chosen to abort, so lets get out of here.
					m_MultiResourceSheet.m_bAllowUpdate = true;
					return false;
				}

				// See if the currently selected resource is among those we are
				// pasting to, and if not, warn the user that the appointment will
				// disappear.
				CNxSchedulerDlg *p = (CNxSchedulerDlg *)GetActiveSheet();
				if (p && p != &m_MultiResourceSheet)
				{
					if (FindElementInArray(adwResources, GetActiveResourceID()) == feiarNotFound) {
						MsgBox("The appointment was created, but it will lose visibility in the scheduler because the scheduler is set to display a resource that the appointment is not associated with.");						
					}
				}
				m_MultiResourceSheet.m_bAllowUpdate = true;

				// Ok, now we know what resources the user wants to write to the new appointment,
				// so adjust the click array accordingly.
				arydwFinalResourceIDs.Copy(adwResources);
			} else {

				auto make_set = [](const CDWordArray& dwa)
				{
					return std::set<DWORD>(boost::begin(dwa), boost::end(dwa));
				};

				if (fromReschedulingQueue && (make_set(arydwToResourceIDs) != make_set(arydwFromResourceIDs))) {

					// (a.walling 2015-01-28 16:12) - PLID 64414 - Rescheduling Queue - resource has changed, but was not multiple to begin with, so we use a simpler messagebox
					CString strOldResources = describeResources(arydwFromResourceIDs);
					CString strNewResources = describeResources(arydwToResourceIDs);
					auto prompt = FormatString("You are attempting to reschedule this appointment for %s but the original appointment was for %s. Are you sure you wish to do this?", strNewResources, strOldResources);
					auto ret = MessageBox(prompt, nullptr, MB_YESNO);
					if (IDYES != ret) {
						return false;
					}
				}

				// No question, the destination will be the same resource that it already has
				arydwFinalResourceIDs.Copy(arydwToResourceIDs);
			}

			//TES 8/12/2009 - PLID 16749 - Do they want to retain the Confirmed status?
			long nCopyConfirmedPref = GetRemotePropertyInt("RetainConfirmationOnPastedAppointments", 2, 0, "<None>");
			if(nCopyConfirmedPref == 1) {
				//TES 8/12/2009 - PLID 16749 - No, reset it to 0 (Not Confirmed)
				nConfirmed = 0;
				strConfirmedBy = ""; // (j.armen 2011-11-17 10:54) - PLID 44205 - Added Confirmed By
			}
			else if(nCopyConfirmedPref == 2 && nConfirmed != 0) {
				//TES 8/12/2009 - PLID 16749 - They want to be prompted (note that there's no point in prompting them
				// if the copied appointment is unconfirmed).
				BOOL bDontShowChecked = FALSE;
				int nReturn = DontShowMeAgain(this, "The appointment being copied has a Confirmed status of '" + 
					CString(nConfirmed == 1 ? "Yes" : "LM") + "'.  Would you like to retain this Confirmed status on the "
					"pasted appointment?", bDontShowChecked, "Retain Confirmed Status", TRUE, TRUE);
				if(nReturn == IDYES) {
					//TES 8/12/2009 - PLID 16749 - They checked "Don't Show Me Again", so set the preference.
					if(bDontShowChecked) {
						SetRemotePropertyInt("RetainConfirmationOnPastedAppointments", 0, 0, "<None>");
					}
				}
				else {
					ASSERT(nReturn == IDNO);
					nConfirmed = 0;
					strConfirmedBy = ""; // (j.armen 2011-11-17 10:55) - PLID 44205 - Added Confirmed By
					//TES 8/12/2009 - PLID 16749 - They checked "Don't Show Me Again", so set the preference.
					if(bDontShowChecked) {
						SetRemotePropertyInt("RetainConfirmationOnPastedAppointments", 1, 0, "<None>");
					}
				}
			}
			else if(nCopyConfirmedPref == 0) {
				//TES 8/12/2009 - PLID 16749 -They want to retain it, so do nothing.
			}
		}

		dtNewCreatedDate = AdoFldDateTime(pToFlds, "CreatedDate");
		strNewCreatedLogin = AdoFldString(pToFlds, "CreatedLogin");
		////////////////////////////////////////////////////////////////////////////////////////////////////////

		bool bFromEvent = false;
		bool bToEvent = false;
		// Calculate the new end time based on the incoming appointment's duration
		dtDuration = eTime - sTime;


		// And the out-going appointment's start time.
		dtNewCancelled = dtOldCancelled;
		dtNewDate = AdoFldDateTime(pToFlds, "Date", COleDateTime(0, 0, 0, 8, 0, 0));
		dtNewStart = sTime = AdoFldDateTime(pToFlds, "StartTime", COleDateTime(0, 0, 0, 8, 0, 0));
		dtNewEnd = eTime = AdoFldDateTime(pToFlds, "EndTime", COleDateTime(0, 0, 0, 8, 0, 0));
		dtNewArrival = AdoFldDateTime(pToFlds, "ArrivalTime", COleDateTime(0, 0, 0, 8, 0, 0));
		if(sTime.GetHour() == 0 && sTime.GetMinute() == 0) {
			//this might be an event.
			if (dtNewEnd.GetHour() == 0 && dtNewEnd.GetMinute() == 0) {
				//This is an event.
				eTime = sTime;
				dtNewEnd = eTime;
				bToEvent = true;
			}
		}

		 // CAH 6/11/01: If the duration is 0, assume it to be an event //////
		// (c.haag 2003-07-17 17:20) - We need to not assume so much
		if(dtDuration == COleDateTimeSpan(0) && dtOldStart.GetHour() == 0 && dtOldStart.GetMinute() == 0 &&
			dtOldEnd.GetHour() == 0 && dtOldEnd.GetMinute() == 0) {
			bFromEvent = true;
			if(!bToEvent) {
				//We need a duration since this won't be an event anymore, let's just make it the slot height.
				dtDuration = COleDateTimeSpan(0,0,m_nInterval,0);
			}
		}

		/////////////////////////////////////////////////////////////////////
		if (bToEvent) {
			eTime = sTime;
			dtNewEnd = eTime;
		}
		else {
			eTime = sTime + dtDuration;
			dtNewEnd = eTime;
		}

		//Double-check that there's no date wrapping here.
		if( eTime.GetDayOfYear() != sTime.GetDayOfYear() ){
			//This should only happen if you got an appointment beginning on one day and ending on the next
			ASSERT(eTime > sTime);
			eTime.SetDateTime(sTime.GetYear(), sTime.GetMonth(), sTime.GetDay(), 23, 59, 0);
			dtNewEnd = eTime;
		}

		// Get some necessary info one way or another
		long nBorderColor, nBackgroundColor;
		CString strResText;
		if (m_pDispatchClip) {
			CReservation resFrom(__FUNCTION__, m_pDispatchClip);
			strResText = (LPCTSTR)resFrom.GetText();
			nBorderColor = resFrom.GetBorderColor();
			nBackgroundColor = resFrom.GetBackColor();
		} else {
			nBorderColor = AppointmentGetBorderColor(nAptTypeID);

			// (j.jones 2014-12-04 14:46) - PLID 64119 - use the mix rule color if one exists and
			// the appt. is pending, otherwise use the status color
			long nMixRuleColor = -1;
			long nStatusID = -1;
			OLE_COLOR clrStatus = AppointmentGetStatusColor(fromResID, &nStatusID);

			if (nStatusID == 0) {	//pending
				nMixRuleColor = GetAppointmentMixRuleColor(fromResID);
			}

			if (nMixRuleColor != -1) {
				nBackgroundColor = nMixRuleColor;
			}
			else {
				nBackgroundColor = clrStatus;
			}

			// (j.jones 2011-02-11 12:01) - PLID 35180 - required dates and a string of resource IDs
			strResText = AppointmentGetText(fromResID, dtOldStart, dtOldEnd, strFromResourceIDs, this);
		}

		// Copy CReservation object
		if (m_pResDispatch) {
			// Attach to the clipped object
			CNxSchedulerDlg *p = (CNxSchedulerDlg *)m_pClipSheet;
			if (p && (p->m_pSingleDayCtrl != NULL) && (!bToEvent || p->m_pEventCtrl != NULL)) {
				// We have to be able to get to the clipped sheet, of course
				short nDay = (short)m_clickTargetInfo.day;
				
				// Create the new object (possibly to replace the old one)
				if(bToEvent) {
					CReservation pRes = p->m_pEventCtrl.AddReservation(__FUNCTION__, nDay, 
						sTime, eTime, nBorderColor, TRUE);
					// (c.haag 2006-05-12 18:13) - PLID 20613 - Not border color! Status color!
					if (m_pDispatchClip) {
						pRes.PutBackColor(nBackgroundColor);
					} else {
						ColorReservationBackground(pRes, nBackgroundColor);
					}
					pRes.PutBorderColor(nBorderColor);

					// (j.luckoski 2012-05-07 11:03) - PLID 11597 - Show cancelled if appt is cancelled.
					// (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
					if (dtNewCancelled.GetStatus() != COleDateTime::valid) {
						// Set the appropriate text for the appointment
						pRes.PutText(_bstr_t(strResText));
						pRes.Data["CancelledDate"] = g_cvarNull;
					} else {
						CString strText = "%1bCANCELLED: %0b";
						pRes.PutText(_bstr_t(strText + strResText));
						pRes.Data["CancelledDate"] = _variant_t(dtNewCancelled, VT_DATE);
					}
					
					pRes.PutReservationID(toResID);
					// (c.haag 2009-12-21 17:21) - PLID 36691 - Assign the appointment type and patient ID
					pRes.PutAptTypeID(nAptTypeID);
					pRes.PutPersonID(nPatientID);
					p->m_pSingleDayCtrl.Resolve();
					p->m_pSingleDayCtrl.Refresh();
				}
				else {
					CReservation pRes(p->m_pSingleDayCtrl.AddReservation(__FUNCTION__, nDay, 
						sTime, eTime, nBorderColor, TRUE));
					//TES 3/22/2011 - PLID 41519 - This is now the "last edited" appointment, so highlight it if this is the singleday, and if
					// that's what we're doing.
					// (d.thompson 2012-08-01) - PLID 51898 - Changed default to Off
					if(GetRemotePropertyInt("SchedHighlightLastAppt", 0, 0, GetCurrentUserName(), true) 
						&& CWnd::FromHandle((HWND)pRes.GethWnd())->GetOwner()->m_hWnd == (HWND)p->m_pSingleDayCtrl.GethWnd()) {
						CSingleDay pSingleDay;
						CWnd* pWndNxTextbox = CWnd::FromHandle((HWND)pRes.GethWnd());
						pSingleDay.SetDispatch((LPDISPATCH)pWndNxTextbox->GetParent()->GetControlUnknown()); // (c.haag 2010-03-26 15:11) - PLID 37332 - Use SetDispatch
						pSingleDay.SetHighlightRes((LPDISPATCH)pRes);
					}
					// (c.haag 2006-05-12 18:13) - PLID 20613 - Not border color! Status color!
					if (m_pDispatchClip) {
						pRes.PutBackColor(nBackgroundColor);
					} else {
						ColorReservationBackground(pRes, nBackgroundColor);
					}
					// (j.luckoski 2012-05-07 11:03) - PLID 11597 - Show cancelled if appt is cancelled.
					// (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
					if (dtNewCancelled.GetStatus() != COleDateTime::valid) {
						// Set the appropriate text for the appointment
						pRes.PutText(_bstr_t(strResText));
						pRes.Data["CancelledDate"] = g_cvarNull;
					} else {
						CString strText = "%1bCANCELLED: %0b";
						pRes.PutText(_bstr_t(strText + strResText));
						pRes.Data["CancelledDate"] = _variant_t(dtNewCancelled, VT_DATE);
					}
					
					pRes.PutReservationID(toResID);
					// (c.haag 2009-12-21 17:21) - PLID 36691 - Assign the appointment type and patient ID
					pRes.PutAptTypeID(nAptTypeID);
					pRes.PutPersonID(nPatientID);
					p->m_pSingleDayCtrl.Resolve();
					p->m_pSingleDayCtrl.Refresh();
				}
			} else {
				bWasDirty = true;
			}
		}

		// (c.haag 2005-07-11 16:55) - Get the purpose list
		// Make sure that the appointment does not already have this purpose.
		// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
		_RecordsetPtr rsPurposes = CreateParamRecordset("SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = {INT} "
			"AND PurposeID NOT IN (SELECT PurposeID FROM AppointmentPurposeT WHERE AppointmentID = {INT})", fromResID, toResID);
		while (!rsPurposes->eof)
		{
			arydwFinalPurposeIDs.Add(AdoFldLong(rsPurposes, "PurposeID"));
			rsPurposes->MoveNext();
		}

		// If someone made an appointment in such a way that the end time is
		// 12:00am and the start time is not 12:00am, we assume the appt spilled
		// into the next day, and therefore, we have to change the end time to
		// 11:59pm
		if (eTime.GetHour() == 0 && eTime.GetMinute() == 0 &&
			!(sTime.GetHour() == 0 && sTime.GetMinute() == 0)) {
			sTime.SetTime(23,59,0);
		}

		// (c.haag 2003-07-31 09:48) - Make sure the location is valid
		long nNewLocationID;
		//TES 8/10/2010 - PLID 39264 - Pass in the Date and StartTime
		if (!GetValidAppointmentLocation(nLocationID, nNewLocationID, toResID, dtNewDate, sTime))
			return FALSE;
		nLocationID = nNewLocationID;

		// (c.haag 2005-07-11 16:44) - PLID 16853 - Now do appointment validations based on
		// rules. At this point, we should be done changing destination appointment information,
		// and we should be ensuring the changes are valid.
		if (!AppointmentValidateByPermissions(nPatientID, arydwFinalResourceIDs, nLocationID, dtNewDate,
			sTime, eTime, nAptTypeID, arydwFinalPurposeIDs, toResID)) {

			// (c.haag 2005-07-11 17:18) - I don't know why these table checkers are in here, but I'm
			// afraid I'll break something if I don't put it here because it was in the legacy logic.
			// (a.wilson 2014-08-12 14:40) - PLID 63199 - unneccessary.
			//PostMessage(WM_TABLE_CHANGED, NetUtils::AppointmentsT); 
			return FALSE;
		}

		// If the user will be prompted to decide how to manually deal with
		// linked appointments, we don't have a guarantee the user will
		// move them. Therefore, they should be ignored when we check templates.
		BOOL bIgnoreLinkedAppts;
		if (GetRemotePropertyInt("PromptOnModifyLinkedAppts", 0, 0, "<None>", true))
			bIgnoreLinkedAppts = TRUE;
		else
			bIgnoreLinkedAppts = FALSE;

		//TES 11/30/2010 - PLID 40868 - Create a VisibleAppt struct describing the appointment we're pasting, so that we can calculate
		// whether it should go onto a precision template.
		VisibleAppointment appt;
		appt.nReservationID = toResID;
		appt.nAptTypeID = nAptTypeID;
		appt.anResourceIDs.Copy(arydwFinalResourceIDs);
		appt.anPurposeIDs.Copy(arydwFinalPurposeIDs);
		appt.dtStart = sTime;
		appt.dtEnd = eTime;
		appt.nOffsetDay = (short)m_clickTargetInfo.day;
		CNxSchedulerDlg *p = (CNxSchedulerDlg *)GetActiveSheet();
		//TES 2/25/2011 - PLID 42523 - If we can't calculate the template item ID, pass in -2 (unknown), rather than -1.  -2 means unknown,
		// -1 means definitely not on a precision template.
		if (!AppointmentValidateByRules(nPatientID, arydwFinalResourceIDs, nLocationID, dtNewDate,
			sTime, eTime, nAptTypeID, arydwFinalPurposeIDs, toResID, bIgnoreLinkedAppts, FALSE, p?p->CalculateTemplateItemID(&appt):-2)) {
			// (a.wilson 2014-08-12 14:40) - PLID 63199 - unneccessary.
			//PostMessage(WM_TABLE_CHANGED, NetUtils::AppointmentsT); 
			return FALSE;
		}

		if (!AppointmentValidateByAlarms(nPatientID, dtNewDate, sTime, nAptTypeID, arydwFinalPurposeIDs, arydwFinalResourceIDs, toResID, nMethod == CLIP_METHOD_CUT ? fromResID : -1)) {
			// (a.wilson 2014-08-12 14:40) - PLID 63199 - unneccessary.
			//PostMessage(WM_TABLE_CHANGED, NetUtils::AppointmentsT); 
			return FALSE;
		}

		if (!AppointmentValidateByDuration(sTime, eTime, nAptTypeID, arydwFinalResourceIDs, arydwFinalPurposeIDs)) {
			// (a.wilson 2014-08-12 14:40) - PLID 63199 - unneccessary.
			//PostMessage(WM_TABLE_CHANGED, NetUtils::AppointmentsT); 
			return FALSE;
		}

		// (j.jones 2014-12-02 09:43) - PLID 64181 - Validate against Scheduler Mix Rules last.
		// Always ignore the new appt., because we already made it.
		// If we are cutting, ignore the old appointment.
		if (!AppointmentValidateByMixRules(this, nPatientID, AsDateNoTime(dtNewDate), nLocationID, nPrimaryInsuredPartyID, nAptTypeID, arydwFinalResourceIDs, arydwFinalPurposeIDs, overriddenMixRules, pSelectedFFASlot,
			toResID, nMethod == CLIP_METHOD_CUT ? fromResID : -1)) {
			return FALSE;
		}

	} NxCatchAllCall("CSchedulerView::CopyRes Error 1",return false);

	// (c.haag 2004-03-05 11:54) - Part 2: We do writing to the recordset here (but nothing permanent) and
	// we start building our huge SQL statement. We also begin auditing here
	try {
		CString strSQLTmp;

		// Calculate the arrival time.
		dtNewArrival = sTime - (dtOldStart - dtOldArrival);
		// Make sure this doesn't take arrival time to the previous day.
		if(dtNewArrival.GetDay() != dtNewDate.GetDay()) {
			dtNewArrival.SetDateTime(dtNewDate.GetYear(), dtNewDate.GetMonth(), dtNewDate.GetDay(), 0, 0, 0);
		}

		// (j.jones 2014-12-02 16:15) - PLID 64181 - if the user overrode a mix rule,
		// track that they did so
		if (overriddenMixRules.size() > 0) {
			TrackAppointmentMixRuleOverride(toResID, overriddenMixRules);
		}

		// (j.jones 2015-01-05 11:35) - PLID 64181 - if a new appointment slot was provided, move the appt.
		// to that slot
		long nOldLocationID = nLocationID;
		if (pSelectedFFASlot != NULL && pSelectedFFASlot->IsValid()) {
			dtNewStart = pSelectedFFASlot->dtStart;
			dtNewEnd = pSelectedFFASlot->dtEnd;
			dtNewArrival = pSelectedFFASlot->dtArrival;
			dtNewDate.SetDate(dtNewStart.GetYear(), dtNewStart.GetMonth(), dtNewStart.GetDay());
			arydwFinalResourceIDs.RemoveAll();
			arydwFinalResourceIDs.Append(pSelectedFFASlot->dwaResourceIDs);
			nLocationID = pSelectedFFASlot->nLocationID; // (r.farnworth 2016-02-02 12:08) - PLID 68116 - FFA results transmit location to new appointment.
		}

		////////////////////////////////////
		//DRT 6/3/02 - auditing!
		//show we got rid of the old one
		nAuditFrom = BeginAuditTransaction();
		//8/6/02 - this was putting in the new date, which made absolutely no sense
		// (a.walling 2015-01-29 09:30) - PLID 64746 - this was putting in the new id, which made absolutely no sense. Also: audit resource changes during appointment paste or drop
		COleDateTime oldStart, oldEnd;
		oldStart = pFromFlds->Item["StartTime"]->Value.date;
		oldEnd = pFromFlds->Item["EndTime"]->Value.date;
		CString strAuditLoc = "";
		if (nOldLocationID != nLocationID)
		{
			strAuditLoc.Format(" and location:  %s", GetLocationName(nOldLocationID));
		}
		auto strDetails = FormatString("Appointment copied from date:  %s with resources:  %s%s", 
			FormatDateTimeForInterface(oldStart, DTF_STRIP_SECONDS, dtoDateTime), describeResources(arydwFromResourceIDs), strAuditLoc);
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditFrom, aeiApptStatus, fromResID, "", strDetails, aepHigh);
		////////////////////////////////////

		/////////////////////////////////////////////////////////
		// Copy data                               //////////////

		// (j.jones 2007-02-27 15:49) - PLID 24955 - if the ShowState is not Pending,
		// create a new log time for the appt. creation time to reflect the time
		// we chose this ShowState
		long nShowState = VarLong(pFromFlds->Item["ShowState"]->Value,0);
		if(nShowState != 0) {
			// (j.jones 2009-08-10 08:39) - PLID 35145 - we now track the user ID, use the current user, not the user
			// who is tracked on the appointment to be copied
			strSQLTmp.Format("INSERT INTO AptShowStateHistoryT (AppointmentID, ShowStateID, TimeStamp, UserID) "
				"VALUES (%li, %li, GetDate(), %li)", toResID, nShowState, GetCurrentUserID());
			strSQLFinal += CString(";") + strSQLTmp;
		}

		if (nMethod == CLIP_METHOD_CUT)
		{			
			strSQLTmp.Format("UPDATE AptLinkT SET AppointmentID = %d WHERE AppointmentID = %d", toResID, fromResID);
			strSQLFinal += CString(";") + strSQLTmp;

			strSQLTmp.Format("UPDATE CaseHistoryT SET AppointmentID = %d WHERE AppointmentID = %d", toResID, fromResID);
			strSQLFinal += CString(";") + strSQLTmp;

			// (c.haag 2006-04-25 17:28) - PLID 20217 - Update the procedure info appointments, too
			strSQLTmp.Format("UPDATE ProcInfoAppointmentsT SET AppointmentID = %d WHERE AppointmentID = %d", toResID, fromResID);
			strSQLFinal += CString(";") + strSQLTmp;

			// If we're moving an appointment, retain the recordid and input information
			nRecordID = pFromFlds->Item["RecordID"]->Value;
			if (!bWantReschedule && (long)m_pResDispatch == -1) // If we're deleting the old appointment without a trace, transfer the record ID
			{
				strSQLTmp.Format("UPDATE PalmRecordT SET AppointmentID = %d WHERE AppointmentID = %d", toResID, fromResID);
				strSQLFinal += CString(";") + strSQLTmp;
				strSQLTmp.Format("DELETE FROM PalmSyncT WHERE AppointmentID = %d", toResID);
				strSQLFinal += CString(";") + strSQLTmp;
				strSQLTmp.Format("UPDATE PalmSyncT SET AppointmentID = %d WHERE AppointmentID = %d", toResID, fromResID);
				strSQLFinal += CString(";") + strSQLTmp;
			}
			if (!bWantReschedule)
			{
				dtNewCreatedDate = AdoFldDateTime(pFromFlds, "CreatedDate");
				strNewCreatedLogin = AdoFldString(pFromFlds, "CreatedLogin");
			}
		}
		else {
			nRecordID = -1; // Otherwise, assign a new one
		}

		// We have to loop this one because it doesn't have autonumber
		for (long nPurp=0; nPurp < arydwFinalPurposeIDs.GetSize(); nPurp++) {
			strSQLTmp.Format("INSERT INTO AppointmentPurposeT (AppointmentID, PurposeID) VALUES (%d, %d)", toResID, arydwFinalPurposeIDs[nPurp]);
			strSQLFinal += CString(";") + strSQLTmp;
		}

		// (j.gruber 2012-08-07 14:48) - PLID 52009 - insurance information 
		strSQLFinal += FormatString("INSERT INTO AppointmentInsuredPartyT (AppointmentID, InsuredPartyID, Placement) "
			" SELECT %li, InsuredPartyID, Placement FROM AppointmentInsuredPartyT WHERE AppointmentID = %li; \r\n" 
			, toResID, fromResID);


		////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Write the new resource IDs for the destination appointment
		{
			// Delete all the resource entries for the destination appointment
			strSQLTmp.Format("DELETE FROM AppointmentResourceT WHERE AppointmentID = %d", toResID);
			strSQLFinal += CString(";") + strSQLTmp;

			// Write the resource IDs from the array
			{
				CString strSqlBatch;
				int nCount = arydwFinalResourceIDs.GetSize();
				ASSERT(nCount > 0);
				for (int i=0; i<nCount; i++)
				{
					CString strSql;
					strSql.Format(
						"INSERT INTO AppointmentResourceT VALUES (%d, %d);", 
						toResID, arydwFinalResourceIDs.GetAt(i));
					strSqlBatch += strSql;
				}
				// Execute the batch
				strSQLFinal += CString(";") + strSqlBatch;
				//ExecuteSqlStd(strSqlBatch);
			}
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////



		// End copy data                           //////////////
		/////////////////////////////////////////////////////////

		//////////////////
		//DRT 6/3/02 - auditing!

		//show the new one
		nAuditTo = BeginAuditTransaction();

		strAuditLoc = "";
		if (nOldLocationID != nLocationID)
		{
			strAuditLoc.Format(" and location:  %s", GetLocationName(nLocationID));
		}
		// (a.walling 2015-01-29 09:30) - PLID 64746 - Audit resource changes during appointment paste or drop
		strDetails = FormatString("Copied over appointment to the date:  %s with resources:  %s%s", 
			FormatDateTimeForInterface(dtNewStart, DTF_STRIP_SECONDS, dtoDateTime), describeResources(arydwFinalResourceIDs), strAuditLoc);
		AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTo, aeiApptStatus, toResID, "", strDetails, aepHigh);

		//(e.lally 2006-07-11) PLID 19269 - Now audit the patient who had their appointment overwritten.
		nAuditOverwrite = BeginAuditTransaction();
		CString strOldApptStart = FormatDateTimeForInterface(dtOverwrittenStart, DTF_STRIP_SECONDS, dtoDateTime);
		AuditEvent(nOverwrittenPatientID, GetExistingPatientName(nOverwrittenPatientID), nAuditOverwrite, aeiApptStatus, nOverwrittenApptID, strOldApptStart, "Appointment copied over", aepHigh);

		//////////////////

	} NxCatchAllCall("CSchedulerView::CopyRes Error 2",RollbackAuditTransaction(nAuditFrom);RollbackAuditTransaction(nAuditTo);return false);

	// (c.haag 2004-03-05 12:00) - Part 3: We actually write the data
	try {
		CString strSQLTmp;
		//for the appointment update, if we are cutting, we'll pass the ID of the
		//soon-to-be-cancelled/deleted appt. so the booking alarms will ignore it
		{
			if (nMethod == CLIP_METHOD_CUT) {

				// (j.jones 2006-10-16 09:19) - PLID 22887 - update room information for the appointment
				strSQLTmp.Format("UPDATE RoomAppointmentsT SET AppointmentID = %li WHERE AppointmentID = %li", toResID, fromResID);
				strSQLFinal += CString(";") + strSQLTmp;
				strSQLTmp = "";

				// (c.haag 2009-10-12 13:17) - PLID 35722 - Update photo links
				strSQLTmp.Format("UPDATE MailSentProcedureT SET AppointmentsID = %li WHERE AppointmentsID = %li", toResID, fromResID);
				strSQLFinal += CString(";") + strSQLTmp;
				strSQLTmp = "";

				// (j.jones 2007-11-07 12:12) - PLID 27987 - update the allocation information for the appointment
				strSQLTmp.Format("UPDATE PatientInvAllocationsT SET AppointmentID = %li WHERE AppointmentID = %li", toResID, fromResID);
				strSQLFinal += CString(";") + strSQLTmp;
				strSQLTmp = "";

				// (j.jones 2008-03-18 15:56) - PLID 29309 - update the order information for the appointment
				// (j.jones 2008-03-19 12:05) - PLID 29316 - do not audit this, the user considers this to be the same appointment
				strSQLTmp.Format("UPDATE OrderAppointmentsT SET AppointmentID = %li WHERE AppointmentID = %li", toResID, fromResID);
				strSQLFinal += CString(";") + strSQLTmp;
				strSQLTmp = "";

				// (j.armen 2012-04-10 14:35) - PLID 48299 - Copy over the AppointmentID and RecallAppointmentID from any Recalls
				strSQLFinal += FormatString("UPDATE RecallT SET AppointmentID = %li WHERE AppointmentID = %li\r\n", toResID, fromResID);
				strSQLFinal += FormatString("UPDATE RecallT SET RecallAppointmentID = %li WHERE RecallAppointmentID = %li\r\n", toResID, fromResID);
				

				{
					// (j.jones 2007-11-19 09:57) - PLID 28043 - added allocation auditing
					// we have to audit the appt change for any linked allocations

					//for auditing, we need to look up the information from the allocation in data
					long nAllocationID = -1;
					_RecordsetPtr rs = CreateParamRecordset("SELECT PatientInvAllocationsT.ID, "
						"CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) + convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS ApptDateTime "
						"FROM PatientInvAllocationsT "
						"LEFT JOIN AppointmentsT ON PatientInvAllocationsT.AppointmentID = AppointmentsT.ID "
						"WHERE PatientInvAllocationsT.AppointmentID = {INT} AND PatientInvAllocationsT.Status <> {INT}", fromResID, InvUtils::iasDeleted);
					if(!rs->eof) {
						long nAllocationID = AdoFldLong(rs, "ID");
						_variant_t varApptDate = rs->Fields->Item["ApptDateTime"]->Value;					
						if(varApptDate.vt == VT_DATE) {
							//if not deleting the allocation, audit that the appt. changed
							CString strOldApptDate = FormatDateTimeForInterface(VarDateTime(varApptDate), NULL, dtoDateTime);
							CString strNewApptDate = FormatDateTimeForInterface(dtNewStart, NULL, dtoDateTime);
							CString strOldValue, strNewValue;
							strOldValue.Format("Appointment: %s", strOldApptDate);
							strNewValue.Format("Appointment: %s", strNewApptDate);
							AuditEvent(nPatientID, GetExistingPatientName(nPatientID), nAuditTo, aeiInvAllocationAppointment, nAllocationID, strOldValue, strNewValue, aepMedium, aetChanged);
						}
					}
					rs->Close();
				}

				// (a.walling 2015-02-04 14:09) - PLID 64412 - Try to cancel if deleting fails, and show a prompt when trying to delete.
				bool bNeedToCancelAppt = bWantReschedule;

				if (!bWantReschedule) {
					// If not retaining and rescheduling, this deletes everything, including the rescheduling notes and all that. 
					// now prompt!
					if (AppointmentDeleteNoHistory(fromResID, false, true, &strSQLTmp, &nAuditDelete, false, false, true))
					{
						bNeedToCancelAppt = false;
					}
					else if (nOldStatus == 4) {
						bNeedToCancelAppt = false;
						int nRet = MessageBox("The appointment could not be deleted; do you want to continue?", nullptr, MB_YESNO | MB_ICONERROR);
						if (nRet != IDYES) {
							return false;
						}
					} else {
						int nRet = MessageBox("The appointment could not be deleted; do you want to cancel the appointment instead?", nullptr, MB_YESNOCANCEL | MB_ICONERROR);
						if (nRet == IDYES) {
							bNeedToCancelAppt = true;
						}
						else if (nRet == IDNO) {
							bNeedToCancelAppt = false; // just skip it
						}
						else {
							return false;
						}
					}
				}

				// Check to see if they want to delete it without a history, or just mark it as canceled
				if (bNeedToCancelAppt) {
					// (a.walling 2015-01-09 13:47) - PLID 64410 - skip if it is already cancelled, of course
					if (nOldStatus != 4) {
						if (false == AppointmentCancel(fromResID, false, true, false, {}, true, &strSQLTmp, &nAuditDelete, false, true))
						{
							// If this failed for some reason, quit right now.
							return false;
						}
					}
				}

				strSQLFinal += CString(";") + strSQLTmp;

				//Update Tracking before we cancel/delete the event.
				strSQLTmp.Format("UPDATE EventsT SET ItemID = %li WHERE ItemID = %li AND PatientID = %li AND (Type = %li OR Type = %li)", 
					toResID, fromResID, nPatientID, PhaseTracking::ET_AppointmentCreated, PhaseTracking::ET_ActualAppointment);
				strSQLFinal += CString(";") + strSQLTmp;
			}

			// Track our modifications
			strSQLTmp.Format(
				"UPDATE AppointmentsT "
				"SET Status = 1, ModifiedDate = getdate(), ModifiedLogin = '%s' "
				"WHERE ID = %li", 
				_Q(GetCurrentUserName()), toResID);
			strSQLFinal += CString(";") + strSQLTmp;
			
			/////////////////////////////////////////////////////////////////////////
			// Write to the table and close the recordset
			/////////////////////////////////////////////////////////////////////////
			// Rather than use pApptTo->Update, we will build a SQL statement for it.
			// This way we can keep the entire transaction in one execute statement.
			CString strTypeTmp = "NULL", strPurposeTmp = "NULL", strRequestedResourceTmp = "NULL";;
			if (nAptTypeID != -1) {
				strTypeTmp.Format("%d", nAptTypeID);
			}
			if (pFromFlds->Item["AptPurposeID"]->Value.vt != VT_NULL) {
				strPurposeTmp.Format("%d", VarLong(pFromFlds->Item["AptPurposeID"]->Value));
			}
			if (pFromFlds->Item["RequestedResourceID"]->Value.vt != VT_NULL) {
				strRequestedResourceTmp.Format("%li", AdoFldLong(pFromFlds, "RequestedResourceID"));
			}
			
			// (d.moore 2007-05-22 16:01) - PLID 4013 - Removed the MoveUp field. 
			//  This functionality is now handled in the waiting list tables.
			// (j.armen 2011-11-17 10:30) - PLID 44205 - Added ConfirmedBy
			// (j.jones 2015-01-09 09:24) - PLID 64181 - changed the fields to reference the real variables, not pToFlds
			// (b.eyers 2015-03-12) - PLID 65288 - Notes and ShowState got left out from the above item, so neither were retained when copy/paste happened
			// (j.jones 2016-02-12 09:07) - PLID 68071 - properly escaped the ConfirmedBy username
			strSQLTmp.Format("UPDATE AppointmentsT SET PatientID = %d, Date = '%s', StartTime = '%s', EndTime = '%s', ArrivalTime = '%s', "
				"Confirmed = %d, ConfirmedBy = '%s', AptTypeID = %s, AptPurposeID = %s, LocationID = %d, RequestedResourceID = %s, "
				"RecordID = %li, CreatedDate = '%s', CreatedLogin = '%s', ShowState = %d, Notes = '%s' "
				"WHERE ID = %d",
				nPatientID,
				_Q(FormatDateTimeForSql(dtNewDate, dtoDate)),
				_Q(FormatDateTimeForSql(dtNewStart)),
				_Q(FormatDateTimeForSql(dtNewEnd)),
				_Q(FormatDateTimeForSql(dtNewArrival)),
				nConfirmed,
				_Q(strConfirmedBy),
				strTypeTmp,
				strPurposeTmp,
				nLocationID,
				strRequestedResourceTmp, 
				nRecordID,
				_Q(FormatDateTimeForSql(dtNewCreatedDate)),
				_Q(strNewCreatedLogin),
				nShowState, 
				_Q(strNotes), 
				toResID);
			strSQLFinal += CString(";") + strSQLTmp;

			// (c.haag 2010-04-01 15:10) - PLID 38005 - Delete from the appointment reminder table. We
			// cast the dates and times accordingly for safety purposes
			{
				COleDateTime dtOld_DateTime, dtNew_DateTime;
				dtOld_DateTime.SetDateTime(dtOldDate.GetYear(), dtOldDate.GetMonth(), dtOldDate.GetDay(),
					dtOldStart.GetHour(), dtOldStart.GetMinute(), dtOldStart.GetSecond());
				dtNew_DateTime.SetDateTime(dtNewDate.GetYear(), dtNewDate.GetMonth(), dtNewDate.GetDay(),
					dtNewStart.GetHour(), dtNewStart.GetMinute(), dtNewStart.GetSecond());
				if (dtOld_DateTime != dtNew_DateTime) {
					strSQLTmp.Format("DELETE FROM AppointmentRemindersT WHERE AppointmentID = %d;", toResID);
					strSQLFinal += CString(";") + strSQLTmp;
				}
			}

			// (d.moore 2007-05-23 13:44) - PLID 4013 - Add the SQL for copying the waiting list data.
			CString strWaitListSql;
			
			// Make variables for the From and To ID just to make things a little easier to read.
			strWaitListSql.Format(
				"; \r\n" // Just in case any content before didn't include this.
				"DECLARE @WL_Old_Appt_ID INT; \r\n"
				"DECLARE @WL_New_Appt_ID INT; \r\n"
				"SET @WL_Old_Appt_ID = %li; \r\n"
				"SET @WL_New_Appt_ID = %li;\r\n", 
				fromResID, toResID);

			// Now add the rest of the query.
			strWaitListSql += 
				// Get the correct wait list ID
				"DECLARE @To_Wait_List_ID INT; \r\n"
				"SET @To_Wait_List_ID = (SELECT ID FROM WaitingListT WHERE AppointmentID = @WL_New_Appt_ID); \r\n"
				// If there is any waiting list data already associated with the To
				//  appointment then delete it.
				"DELETE FROM WaitingListItemResourceT "
				"WHERE ItemID IN "
					"(SELECT ID FROM WaitingListItemT "
					"WHERE WaitingListID = @To_Wait_List_ID); "
				"\r\n"
				// Delete Days Query.
				"DELETE FROM WaitingListItemDaysT "
				"WHERE ItemID IN "
					"(SELECT ID FROM WaitingListItemT "
					"WHERE WaitingListID = @To_Wait_List_ID); "
				"\r\n"
				// Delete Line Items Query.
				"DELETE FROM WaitingListItemT "
				"WHERE WaitingListID = @To_Wait_List_ID; "
				"\r\n"
				// Delete Purpose Query.
				"DELETE FROM WaitingListPurposeT "
				"WHERE WaitingListID = @To_Wait_List_ID; "
				"\r\n"
				// Delete Waiting List Entry Query.
				"DELETE FROM WaitingListT WHERE ID = @To_Wait_List_ID;"
				"\r\n"
				// Start copying any waiting list data from one appointment to the other.
				// Check first though to make sure that any copying should be done at all.
				"DECLARE @From_Wait_List_ID INT; \r\n"
				"SET @From_Wait_List_ID = (SELECT ID FROM WaitingListT WHERE AppointmentID = @WL_Old_Appt_ID); \r\n"
				"IF @From_Wait_List_ID > 0 \r\n"
				"BEGIN \r\n"
				// New entry in WaitingListT
				"SET NOCOUNT ON \r\n"
				"INSERT INTO WaitingListT(AppointmentID, PatientID, TypeID, CreatedDate) "
				"SELECT @WL_New_Appt_ID, PatientID, "
					"(SELECT ID FROM AptTypeT WHERE ID = AptTypeID), "
					"CreatedDate "
				"FROM AppointmentsT WHERE ID = @WL_Old_Appt_ID; "
				"\r\n"
				"SET @To_Wait_List_ID = @@IDENTITY; \r\n"
				// Copy all appointment purpose data.
				"INSERT INTO WaitingListPurposeT(WaitingListID, PurposeID) "
				"SELECT @To_Wait_List_ID, PurposeID "
				"FROM WaitingListT INNER JOIN WaitingListPurposeT "
				"ON WaitingListT.ID = WaitingListPurposeT.WaitingListID "
				"WHERE WaitingListT.AppointmentID = @WL_Old_Appt_ID;\r\n"
				"\r\n"
				"DECLARE @New_Line_Item_ID INT; \r\n";
				
			// Get all of the Line Item IDs that need to be copied over.
			// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT WaitingListItemT.ID "
				"FROM WaitingListItemT INNER JOIN WaitingListT "
				"ON WaitingListItemT.WaitingListID = WaitingListT.ID "
				"WHERE WaitingListT.AppointmentID = {INT}", fromResID);

			FieldsPtr flds;
			CString strResourceQueries, strResSubQuery;
			long nLineItemID;
			if (!prs->eof) {
				flds = prs->Fields;
			}
			while (!prs->eof) {
				nLineItemID = AdoFldLong(flds, "ID");
				
				strResSubQuery.Format(
					"INSERT INTO WaitingListItemT "
						"(WaitingListID, StartDate, EndDate, StartTime, EndTime, AllResources) "
					"SELECT @To_Wait_List_ID, StartDate, EndDate, StartTime, EndTime, AllResources "
					"FROM WaitingListItemT WHERE ID = %li; \r\n"
					// Get new line item id
					"SET @New_Line_Item_ID = @@IDENTITY; \r\n"
					// Insert resources
					"INSERT INTO WaitingListItemResourceT (ItemID, ResourceID) "
					"SELECT @New_Line_Item_ID, ResourceID "
					"FROM WaitingListItemResourceT "
					"WHERE ItemID = %li; \r\n"
					// Insert days
					"INSERT INTO WaitingListItemDaysT (ItemID, DayOfWeekNum) "
					"SELECT @New_Line_Item_ID, DayOfWeekNum "
					"FROM WaitingListItemDaysT "
					"WHERE ItemID = %li; \r\n",
					nLineItemID, nLineItemID, nLineItemID);

				strResourceQueries += strResSubQuery;
				prs->MoveNext();
			}

			// (d.moore 2007-10-16) - PLID 26546 - The Waiting list queries needed to be
			//  moved ahead of the rest of the main query. When an appointment was being
			//  pasted after a cut the Waiting list data was not updating properly.
			strWaitListSql += strResourceQueries + "SET NOCOUNT OFF \r\n END \r\n";
			strSQLFinal = strWaitListSql + strSQLFinal;


			// Trim the left-most of strSQLFinal
			strSQLFinal = strSQLFinal.Right( strSQLFinal.GetLength() - 1 );
			// Make sure no double semi-colons exist
			while (-1 != strSQLFinal.Find(";;"))
			{
				strSQLFinal.Replace(";;", ";");
			}
			
			// (z.manning 2008-08-20 10:08) - PLID 30490 - If this is a CUT/paste, then we need to tell HL7
			// to cancel pre-existing appointment before we delete the appointment so that we have the data
			// to generate the HL7 message.
			// (z.manning 2008-08-22 11:04) - PLID 30490 - Only do this if they deleted the appointment.
			// Otherwise, AppointmentCancel will send the HL7 message.
			if(nMethod == CLIP_METHOD_CUT && !bWantReschedule) {
				// (r.gonet 12/03/2012) - PLID 54108 - Changed to use refactored send function.
				SendCancelAppointmentHL7Message(fromResID);
			}

			///////////////////////////////////////////////////////////////////////
			// Do the write to the database
			ExecuteSqlStd(strSQLFinal);
			///////////////////////////////////////////////////////////////////////
			// Audit our changes
			///////////////////////////////////////////////////////////////////////
			CommitAuditTransaction(nAuditFrom);
			CommitAuditTransaction(nAuditTo);
			CommitAuditTransaction(nAuditOverwrite);
			CommitAuditTransaction(nAuditDelete); 

			// Success
			if (nMethod == CLIP_METHOD_CUT) {
				// (c.haag 2004-03-05 14:56) -  This is usually done in ::AppointmentCancel and ::AppointmentDeleteNoHistory
				PhaseTracking::UnapplyEvent(nPatientID, PhaseTracking::ET_AppointmentCreated, fromResID);
				PhaseTracking::UnapplyEvent(nPatientID, PhaseTracking::ET_ActualAppointment, fromResID);

				if (m_pClipSheet && m_pDispatchClip) {
					CReservation resClip(__FUNCTION__, m_pDispatchClip);
					resClip.DeleteRes(__FUNCTION__);
				}
				CNxSchedulerDlg *p = (CNxSchedulerDlg *)m_pClipSheet;
				if (p && (p->m_pSingleDayCtrl != NULL)) {
					p->m_pSingleDayCtrl.Resolve();
					//CWnd::FromHandle((HWND)p->m_pSingleDayCtrl.GethWnd())->Refresh();
					p->m_pSingleDayCtrl.Refresh();
				}

				// If the paste date/time was different from the source appointment, invoke the apt linking dialog.
				if (dtOldStart != dtNewStart)
				{
					long nMinutes = GetSafeTotalMinuteDiff(dtNewStart, dtOldStart);

					//TODO - DRT 4/15/03 - Here's the deal.  We need to post a message, because otherwise very strange
					//		things happen b/c we're inside the res-drag still.  Mouse events get screwy, updates are disabled, 
					//		etc.  It's a big mess.  BUT, sometimes there are cases where our p pointer is null (I've seen it
					//		myself), so we can't always post the message.  So for now, I'm going to have it do the dialog if
					//		p is null, and we'll have to live with the wierd mouse effects until we can determine a better 
					//		course of action.
					if(p) {
						p->PostMessage(NXM_RESERVATION_INVOKE_RESLINK, toResID, nMinutes);
					}
					else {
						//TODO - See comment block above.  This code should really never be executed, but there are (I've seen them!)
						//		rare occasions when it can happen (I am unable to reproduce them).  In this case, we'll just pop up
						//		the dialog right away.  This causes some wierd mouse issues to happen (namely, you have to click once
						//		before you do anything).  We need to find out what causes p to be null and fix that so this code is 
						//		never necessary.
						AttemptAppointmentLinkAction(toResID, nMinutes);
					}
				}

				//update case history dates, if needed
				if(dtOldDate != dtNewDate && IsSurgeryCenter(false)) {
					COleDateTime dtDateOnly;
					dtDateOnly.SetDate(dtNewDate.GetYear(),dtNewDate.GetMonth(),dtNewDate.GetDay());
					// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
					_RecordsetPtr rs = CreateParamRecordset("SELECT Count(ID) AS CountOfID FROM CaseHistoryT WHERE AppointmentID = {INT} AND SurgeryDate <> {STRING}", toResID, FormatDateTimeForSql(dtDateOnly,dtoDate));
					long count = 0;
					if(!rs->eof) {
						count = AdoFldLong(rs, "CountOfID",0);
					}
					rs->Close();
					if(count > 0) {
						CString str;
						if(count > 1) {
							str.Format("There are %li case histories attached to this appointment that specify a different surgery date.\n"
								"Would you like to update the surgery date on these case histories to reflect the new appointment date?",count);
						}
						else {
							str = "There is a case history attached to this appointment that specifies a different surgery date.\n"
								"Would you like to update the surgery date on this case history to reflect the new appointment date?";
						}
						if(IDYES == MsgBox(MB_ICONQUESTION|MB_YESNO,str)) {
							//TES 1/9/2007 - PLID 23575 - Go through each one and audit.
							// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
							_RecordsetPtr rsCaseHistories = CreateParamRecordset("SELECT ID, SurgeryDate FROM CaseHistoryT WHERE AppointmentID = {INT} AND SurgeryDate <> {STRING}", toResID, FormatDateTimeForSql(dtDateOnly,dtoDate));
							while(!rsCaseHistories->eof) {
								long nCaseHistoryID = AdoFldLong(rsCaseHistories, "ID");
								_variant_t varOldDate = rsCaseHistories->Fields->GetItem("SurgeryDate")->Value;
								CString strOldDate;
								if(varOldDate.vt == VT_DATE) {
									strOldDate = FormatDateTimeForInterface(VarDateTime(varOldDate), NULL, dtoDate);
								}
								else {
									strOldDate = "<None>";
								}

								// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
								ExecuteParamSql("UPDATE CaseHistoryT SET SurgeryDate = {STRING} WHERE ID = {INT}", FormatDateTimeForSql(dtDateOnly,dtoDate), nCaseHistoryID);
								// (a.walling 2008-06-04 15:23) - PLID 29900 - Use correct patient name
								AuditEvent(nPatientID, GetExistingPatientName(nPatientID), BeginNewAuditEvent(), aeiCaseHistorySurgeryDate, nCaseHistoryID, strOldDate, FormatDateTimeForInterface(dtDateOnly,NULL,dtoDate), aepMedium, aetChanged);
								rsCaseHistories->MoveNext();
							}
						}
					}
				}
			}

			// Update the palm tables
			UpdatePalmSyncT(fromResID);
			UpdatePalmSyncT(toResID);
			// Update the Outlook calendar
			if (nMethod == CLIP_METHOD_CUT)
				PPCDeleteAppt(fromResID);
			else
				PPCModifyAppt(fromResID);
			PPCAddAppt(toResID);

			// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
			// (r.gonet 12/03/2012) - PLID 54106 - Changed to use refactored send function.
			SendNewAppointmentHL7Message(toResID, true);

			// (z.manning 2015-11-05 12:13) - PLID 57109 - Update recalls needing attention dialog
			GetMainFrame()->HandleRecallChanged();

			// Check if it was dirty before we tried to paste
			if (bWasDirty) {
				// If it was dirty, just refresh
				// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
				//TES 3/31/2011 - PLID 41519 - Pass in the new ID as a parameter, in order to tell ourselves to make sure that that appointment
				// is "highlighted" (since we couldn't do it ourselves, as we didn't have a ResDispatch.
				PostMessage(NXM_UPDATEVIEW, (WPARAM)toResID);
			} else {
				// If it wasn't dirty, our changes are displayed so clean it
				SchedulerClean();
			}
			
			// (d.moore 2007-10-16) - PLID 26546 - We need to check to see if the new appointment 
			//  might satisfy a request in the waiting list.
			if (toResID > 0 && !bMoveUp) {
				CheckAppointmentAgainstWaitingList(toResID, nPatientID, arydwFinalResourceIDs, dtNewDate, dtNewStart);
			}

			// Done
			return true;
		}

	} NxCatchAllCall("CSchedulerView::CopyRes Error 3",RollbackAuditTransaction(nAuditFrom);RollbackAuditTransaction(nAuditTo);RollbackAuditTransaction(nAuditDelete);Log(CString("Failed SQL Statement: ") + strSQLFinal);return false);
	
	return false;
}

long GetDefaultFieldByPosition(int nPosition) {
	switch(nPosition) {
	case 0:
		return efID;
	case 1:
		return efHomePhone;
	case 2:
		return efWorkPhone;
	case 3:
		return efBirthDate;
	case 4:
		return efMobilePhone;
	case 5:
		return efPrefContact;
	case 6:
		return efPatBalance;
	case 7:
		return efInsBalance;
	default:
		return -1;
	}
}
void CSchedulerView::ObtainExtraInfo()
{
	int i;
	for (i = 0; i < 8; i++) {
		m_bExtraInfo[i] = GetRemotePropertyInt("ResEntryMoreInf", TRUE, i);
		m_nExtraInfoStyles[i] = GetRemotePropertyInt("ResEntryMoreInfoStyle",0,i);
		int nField = GetRemotePropertyInt("ResEntryExtraField", GetDefaultFieldByPosition(i), i);
		switch(nField) {
		case -1:
			nField = efCustom1;
			break;
		case -2:
			nField = efCustom2;
			break;
		case -3:
			nField = efCustom3;
			break;
		case -4:
			nField = efCustom4;
			break;
		}
		m_nExtraFields[i] = nField;
	}
	if (m_ResEntry->GetSafeHwnd()) {
		m_ResEntry->CheckDlgButton(IDC_PATIENT_ID_CHECK, m_bExtraInfo[0]);
		((CComboBox*)m_ResEntry->GetDlgItem(IDC_EXTRA_FIELDS1))->SetCurSel(m_nExtraFields[0]);
		m_ResEntry->CheckDlgButton(IDC_EXTRA_BOLD1, m_nExtraInfoStyles[0]);
		m_ResEntry->CheckDlgButton(IDC_HOME_PHONE_CHECK, m_bExtraInfo[1]);
		((CComboBox*)m_ResEntry->GetDlgItem(IDC_EXTRA_FIELDS2))->SetCurSel(m_nExtraFields[1]);
		m_ResEntry->CheckDlgButton(IDC_EXTRA_BOLD2, m_nExtraInfoStyles[1]);
		m_ResEntry->CheckDlgButton(IDC_WORK_PHONE_CHECK, m_bExtraInfo[2]);
		((CComboBox*)m_ResEntry->GetDlgItem(IDC_EXTRA_FIELDS3))->SetCurSel(m_nExtraFields[2]);
		m_ResEntry->CheckDlgButton(IDC_EXTRA_BOLD3, m_nExtraInfoStyles[2]);
		m_ResEntry->CheckDlgButton(IDC_BIRTH_DATE_CHECK, m_bExtraInfo[3]);
		((CComboBox*)m_ResEntry->GetDlgItem(IDC_EXTRA_FIELDS4))->SetCurSel(m_nExtraFields[3]);
		m_ResEntry->CheckDlgButton(IDC_EXTRA_BOLD4, m_nExtraInfoStyles[3]);
		m_ResEntry->CheckDlgButton(IDC_CELL_PHONE_CHECK, m_bExtraInfo[4]);
		((CComboBox*)m_ResEntry->GetDlgItem(IDC_EXTRA_FIELDS5))->SetCurSel(m_nExtraFields[4]);
		m_ResEntry->CheckDlgButton(IDC_EXTRA_BOLD5, m_nExtraInfoStyles[4]);
		m_ResEntry->CheckDlgButton(IDC_PREFERRED_CONTACT_CHECK, m_bExtraInfo[5]);
		((CComboBox*)m_ResEntry->GetDlgItem(IDC_EXTRA_FIELDS6))->SetCurSel(m_nExtraFields[5]);
		m_ResEntry->CheckDlgButton(IDC_EXTRA_BOLD6, m_nExtraInfoStyles[5]);
		m_ResEntry->CheckDlgButton(IDC_PATBAL_CHECK, m_bExtraInfo[6]);
		((CComboBox*)m_ResEntry->GetDlgItem(IDC_EXTRA_FIELDS7))->SetCurSel(m_nExtraFields[6]);
		m_ResEntry->CheckDlgButton(IDC_EXTRA_BOLD7, m_nExtraInfoStyles[6]);
		m_ResEntry->CheckDlgButton(IDC_INSBAL_CHECK, m_bExtraInfo[7]);
		((CComboBox*)m_ResEntry->GetDlgItem(IDC_EXTRA_FIELDS8))->SetCurSel(m_nExtraFields[7]);
		m_ResEntry->CheckDlgButton(IDC_EXTRA_BOLD8, m_nExtraInfoStyles[7]);
	}
}

// (z.manning 2009-06-30 16:06) - PLID 34744 - Added this function so we only call
// dbo.InsReferralSummary when necessary as it has been know to cause slowness issues.
BOOL CSchedulerView::NeedToLoadInsReferralSummary()
{
	for(BYTE nMoreInfoIndex = 0; nMoreInfoIndex < 8; nMoreInfoIndex++)
	{
		if(m_nExtraFields[nMoreInfoIndex] == efInsuranceReferrals && m_bExtraInfo[nMoreInfoIndex]) {
			return TRUE;
		}
	}

	return FALSE;
}

// (z.manning 2009-06-30 16:29) - PLID 34744 - This function will return the text to
// load the insurance referral summary based on if we need to or not.
CString CSchedulerView::GetInsReferralSummarySql(const CString &strPatientID)
{
	if(NeedToLoadInsReferralSummary()) {
		return FormatString("dbo.InsReferralSummary(%s, GetDate())", strPatientID);
	}
	else {
		// (z.manning 2009-06-30 16:35) - PLID 34744 - It should not matter what we return
		// here since this should mean that insurance referrals are not displayed in the
		// schedule. But let's set it to this just in case as that will make a bug easier
		// to diagnose.
		return "'<not selected>'";
	}
}

void CSchedulerView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{//What a mess - BVB
//	if ((bActivate == TRUE) && ((CMainFrame *)AfxGetMainWnd())->IsPatientBarVisible() == TRUE){
//		((CMainFrame *)AfxGetMainWnd())->SetToolBars(SCHEDULER_MODULE_NAME);
//		if(((CMainFrame *)AfxGetMainWnd())->m_bUpdateScheduler = true) {//What kind of a condition is this? -BVB
//			UpdateView();
//			((CMainFrame *)AfxGetMainWnd())->m_bUpdateScheduler = false;
//		}
//	} else {
//		((CMainFrame *)AfxGetMainWnd())->m_bUpdateScheduler = false;
//	}
	CNxTabView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CSchedulerView::RequeryAllPurposeFilters()
{
	if(m_OneDaySheet.m_dlPurposeFilter != NULL)
		m_OneDaySheet.RequeryPurposeFilters();
	if(m_WeekDaySheet.m_dlPurposeFilter != NULL)
		m_WeekDaySheet.RequeryPurposeFilters();
	if(m_MultiResourceSheet.m_dlPurposeFilter != NULL)
		m_MultiResourceSheet.RequeryPurposeFilters();
	if(m_MonthSheet.m_dlPurposeFilter != NULL)
		m_MonthSheet.RequeryPurposeFilters();
}

void CSchedulerView::SetAllMultiTypeStrings(CString strTypes)
{
	m_OneDaySheet.m_strMultiTypeIds = strTypes;
	m_WeekDaySheet.m_strMultiTypeIds = strTypes;
	m_MultiResourceSheet.m_strMultiTypeIds = strTypes;
	m_MonthSheet.m_strMultiTypeIds = strTypes;
}

void CSchedulerView::SetAllMultiPurposeStrings(CString strPurposes)
{
	m_OneDaySheet.m_strMultiPurposeIds = strPurposes;
	m_WeekDaySheet.m_strMultiPurposeIds = strPurposes;
	m_MultiResourceSheet.m_strMultiPurposeIds = strPurposes;
	m_MonthSheet.m_strMultiPurposeIds = strPurposes;
}

// (d.singleton 2012-06-07 11:56) - PLID 47473
void CSchedulerView::SetAllMultiLocationStrings(CString strLocations)
{
	m_OneDaySheet.m_strMultiLocationIds = strLocations;
	m_WeekDaySheet.m_strMultiLocationIds = strLocations;
	m_MultiResourceSheet.m_strMultiLocationIds = strLocations;
	m_MonthSheet.m_strMultiLocationIds = strLocations;
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CSchedulerView::Hotkey(int key)
{	
	if(IsWindowEnabled()) {
		// (b.cardillo 2003-06-30 18:04) TODO: Why are we using E000 here instead of 0x80000000 like normal?
		int nKey = IgnoreRightAlt() ? VK_LMENU : VK_MENU;
		if (GetAsyncKeyState(nKey) & 0xE000) {
			switch (key) {
			case 'D':
				SwitchToTab(sttOneDayView);
				return 0;
			case 'K':
				SwitchToTab(sttWeekView);
				return 0;
			case 'R':
				SwitchToTab(sttMultiResourceView);
				return 0;
			case 'O':
				SwitchToTab(sttMonthView);
				return 0;
			default:
				break;
			}
		}
	}
	return CNxTabView::Hotkey(key);
}

LRESULT CSchedulerView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	try {
		switch (message) {
		case NXM_ALLOW_CLOSE:
		case NXM_PRE_CLOSE:
			// First store some settings
			{
				// Rmember whether the user wants to see the scheduled patients list or not
				BOOL bShowScheduledPatients = FALSE;
				if (m_pdlgScheduledPatients->GetSafeHwnd() && m_pdlgScheduledPatients->IsWindowVisible()) {
					bShowScheduledPatients = TRUE;
				}
				SetRemotePropertyInt("ResShowSchedPats", bShowScheduledPatients ? 1 : 0, 0, GetCurrentUserName());

				// Remember what the current resource is
				SetRemotePropertyInt("ResCurItemID", m_nCurrentResourceID, 0, GetCurrentUserName());

				// Remember what the last successfully loaded resource view is
				SetRemotePropertyInt("CurResourceView", m_nLastLoadedResourceViewID, /*0*/GetCurrentLocationID(), GetCurrentUserName());
			}


			// Kick it up to the base class.
			return CNxTabView::WindowProc(message, wParam, lParam);;
			break;
		case WM_TABLE_CHANGED:
			switch (wParam) {
			case NetUtils::OfficeHours:
				{
					try {
						// The OfficeHours tablechecker type doesn't allow -1, it requires a valid location ID
						ASSERT(lParam != -1);
						// See if it's our location ID
						long nLocID = m_lohOfficeHours.GetLocationID();
						if (lParam == nLocID) {
							// It is our location, so call init again so that it requeries
							m_lohOfficeHours.Init(nLocID);
							CNxSchedulerDlg *pDaySheet = (CNxSchedulerDlg *)GetActiveSheet();
							if (pDaySheet) {
								pDaySheet->m_bForceBlocksUpdate = TRUE;
							}
						}
					} NxCatchAll("Error in CSchedulerView::WindowProc:WM_TABLE_CHANGED:OfficeHours");
				}
				break;
			case NetUtils::AptShowStateT:
				// (c.haag 2003-08-07 11:31) - Refresh our array of show state ID's
				LoadShowStateArray();
				break;
			default:
				break;
			}
			return CNxTabView::WindowProc(message, wParam, lParam);
			break;
		
		default:
			return CNxTabView::WindowProc(message, wParam, lParam);
			break;
		}
	} NxCatchAll(FormatString(__FUNCTION__ ": Message: 0x%04x wParam: 0x%08x lParam: 0x%08x", message, wParam, lParam));

	// (a.walling 2007-08-31 12:00) - PLID 27265 - Sending the message to NxTabView
	// was dispatching it to SchedulerView::OnCommand, which may possibly cause the
	// same exception to happen again. So I'm adding try/catch handling there.
	return CNxTabView::WindowProc(message, wParam, lParam);
}

long CSchedulerView::GetPersonIdFromRes(LPDISPATCH lpResDispatch)
{
	// Get the reservation object
	CReservation pRes(__FUNCTION__, lpResDispatch);
	//Get the PatientID of the ReservationID
	// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
	_RecordsetPtr prs = CreateParamRecordset(
		"SELECT PatientID FROM AppointmentsT WHERE ID = {INT}", pRes.GetReservationID());
	// Get the patient id
	return AdoFldLong(prs, "PatientID");
}

//DRT 6/12/2008 - PLID 9679 - I converted this to an ON_MESSAGE, because it's sent from the nxschedulerdlg menu, 
//	with a new wParam value which is the path of the superbill to print.  Additionally, the lparam is a 1/0 flag
//	that indicates whether the user should be prompted to choose their superbill path.
LRESULT CSchedulerView::OnPrintSuperbill(WPARAM wParam, LPARAM lParam) {

	// (a.walling 2007-08-31 10:33) - PLID 27265 - Ensure valid m_pResDispatch pointer
	if (IS_VALID_DISPATCH(m_pResDispatch)) {
		CReservation pRes(__FUNCTION__, m_pResDispatch);
		long nApptID = pRes.GetReservationID();

		//DRT 6/12/2008 - PLID 9679 - We now sent the path to override the superbill as the wParam.
		CString strOverridePath;
		if(wParam != NULL) {
			//convert to _bstr_t
			_bstr_t bstr = (BSTR)wParam;
			//convert to _variant_t then CString
			strOverridePath = VarString(_variant_t(bstr));

			//Also, if the override path is empty, the lParam may tell us that we need to prompt
			//	the user for a path.
			if(strOverridePath.IsEmpty() && (UINT)lParam != 0) {
				//Yep, we need to prompt the user
				CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY|OFN_FILEMUSTEXIST, "Microsoft Word Templates|*.dot;*.dotx;*.dotm||", this);
				CString strInitialDir = GetSharedPath() ^ "Templates\\Forms";
				dlg.m_ofn.lpstrInitialDir = strInitialDir;
				int nResult = dlg.DoModal();
				if(nResult == IDOK) {
					strOverridePath = dlg.GetPathName();
				}
				else {
					//cancelled, just abandon
					//TES 1/18/2011 - PLID 47471 - As below, we need to clean up the Reservation object.
					m_pResDispatchPtr.SetDispatch(NULL);
					m_pResDispatch = (LPDISPATCH)-1;
					return 0;
				}
			}
		}
		SuperbillByAppt(nApptID, strOverridePath);

		// (z.manning 2008-09-19 15:40) - 31451 - Since PLID 9679 changed this function from a command
		// handler to a message hanlder, we need to clear this out to make sure the fix for 27265
		// still works.
		m_pResDispatchPtr.SetDispatch(NULL);
		m_pResDispatch = (LPDISPATCH)-1;
	}

	return 0;
}

void CSchedulerView::OnGotopatient() 
{
	try {
		// (a.walling 2007-08-31 10:33) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (IS_VALID_DISPATCH(m_pResDispatch)) {
			//get the patient id from the reservation that is clicked on
			long nPatId = GetPersonIdFromRes(m_pResDispatch);
			
			//Check to see that it is a valid patient
			if (nPatId > 0) {
				//Set the active patient
				CMainFrame *pMainFrame;
				pMainFrame = GetMainFrame();
				if (pMainFrame != NULL) {
					if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatId)) {
						if(IDNO == MessageBox("This patient is not in the current lookup. \n"
							"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
							return;
						}
					}
					//TES 1/7/2010 - PLID 36761 - This function may fail now
					if(!pMainFrame->m_patToolBar.TrySetActivePatientID(nPatId)) {
						return;
					}
					//Now just flip to the patient's module and set the active Patient
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
					CNxTabView *pView = pMainFrame->GetActiveView();
					if(pView)
						pView->UpdateView();
				}//end if MainFrame
				else {
					AfxThrowNxException("Cannot Open MainFrame");
				}//end else pMainFrame
			}//end if nPatID
			else {
				//if the appointment is not a patient
				AfxThrowNxException("This appointment must be a patient appointment");
			}//end else nPatID
		}//end if m_pResDispatch
	}NxCatchAll("CSchedulerView::OnGotopatient");
}

#define TABHEIGHT 23

void CSchedulerView::OnSize(UINT nType, int cx, int cy) 
{
	CNxView::OnSize(nType, cx, cy);

	//BVB - not everything in the scheduler responds to invalidate
//	SetRedraw(FALSE);
	// Resize the tab control to fix view
	if (m_wndTab.GetSafeHwnd())
	{	m_wndTab.MoveWindow(0, 0, cx, TABHEIGHT);

		// Recalc the sheet rectangle
		GetClientRect(&m_rectSheet);
		m_rectSheet.top += TABHEIGHT;

		// Reposition and resize whatever the current sheet is
		if (m_pActiveSheet && m_pActiveSheet->GetSafeHwnd()) {
			m_pActiveSheet->MoveWindow(m_rectSheet);
			m_pActiveSheet->SetControlPositions();
		}
	}
//	SetRedraw(TRUE);
//	Invalidate(FALSE);
}

LRESULT CSchedulerView::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {

		// (j.jones 2014-08-07 11:50) - PLID 63179 - do not process a table checker
		// if we have no active sheet
		if (m_pActiveSheet == NULL) {
			return 0;
		}

		// (j.jones 2014-08-07 16:29) - PLID 63232 - this is now a modular function
		bool bSchedulerIsActive = false;
		if (GetMainFrame() && GetMainFrame()->IsActiveView(SCHEDULER_MODULE_NAME)) {
			bSchedulerIsActive = true;
		}

		if(m_ResEntry != NULL)
		{
			m_ResEntry->SendMessage(WM_TABLE_CHANGED, wParam, lParam);
		}

		// (j.jones 2014-08-21 17:21) - PLID 63186 - send all messages to the MoveUpDlg
		if (m_MoveUpDlg.m_hWnd) {
			if (m_MoveUpDlg.IsWindowVisible()) {
				m_MoveUpDlg.SendMessage(WM_TABLE_CHANGED, wParam, lParam);
			}
		}

		if (wParam == NetUtils::AppointmentsT)
		{
			// (j.jones 2014-08-07 14:36) - PLID 63167 - we no longer respond to non-Ex
			// AppointmentsT messages, we only respond to Ex messages
		}
		else if (wParam == NetUtils::Resources)
		{
			try {

				// (j.jones 2014-08-07 16:28) - PLID 63232 - this is not needed if the scheduler
				// is not active, as resources will be checked in UpdateView()
				if (!bSchedulerIsActive) {
					//set our variable so we know we need an update when we come back
					((CNxSchedulerDlg*)m_pActiveSheet)->m_bNeedUpdate = true;
					return 0;
				}

				// See if an active resource was deleted
				CString strWhere;
				long nResources = 0;

				// (d.thompson 2010-05-06) - PLID 38525 - Parameterized as possible.  The resource list will often be common
				//	between clients (all resources, all doctors, etc), so this code, while not always the same # of parameters, 
				//	will frequently duplicate them.  So we'll param as best we can.
				_CommandPtr pCmd = OpenParamQuery("");

				for (long i=0; i< m_arypCurrentResourceList.GetSize(); i++)
				{
					strWhere += "ID = ? OR ";
					AddParameterLong(pCmd, "ResourceID", m_arypCurrentResourceList.GetAt(i)->m_nResourceID);
					nResources++;
				}
				DeleteEndingString(strWhere, "OR ");

				if (nResources)
				{
					// (d.thompson 2010-05-06) - PLID 38525 - Execute the parameterization
					pCmd->PutCommandText(_bstr_t("SELECT ID FROM ResourceT WHERE " + strWhere));
					_RecordsetPtr prs = CreateRecordset(pCmd);
					if (prs->eof || prs->GetRecordCount() < nResources)
					{
						// Clear the current list of visible resources. This way, if we get another resource
						// table checker message regarding a resource we do not have visible, we won't get
						// another message box.
						m_arypCurrentResourceList.RemoveAllAndFreeEntries();

						// Yes. We need to inform the user and change our active resource. The following logic is smart enough to move
						// us to a new resource in the event our working resource was the one that was deleted.
						MsgBox("A currently visible resource has been deleted by another user. Practice will now remove any deleted resources from the scheduler window.");

						// It's distracting to have it clear and disable and then redraw (it's way worse 
						// than just a flicker) so we're turning off the redraw for the whole module first.
						SetRedraw(FALSE);

						// Just load the resource list (that function cascades the changes down to each sheet)
						LoadResourceList(m_nLastLoadedResourceViewID, m_nCurrentResourceID);

						// Now turn the redraw back on, invalidate, and redraw the window (if we don't 
						// invalidate after turning redraw back on, then we don't get a paint message and the 
						// user is really really confused)
						SetRedraw(TRUE);
						RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN);
					}
				}
			} NxCatchAll("Error in CSchedulerView::OnTableChanged:Resources");
		}
		else if (wParam == NetUtils::TemplateT)
		{
			// (c.haag 2006-12-11 16:01) - PLID 23808 - Make sure the templates accurately display today.
			// We don't need to reload appointments.
			CNxSchedulerDlg* pSched = (CNxSchedulerDlg*)m_pActiveSheet;
			// (a.walling 2007-08-20 17:55) - PLID 27128 - If they are waiting for a timed update, then the
			// dropdown date does not match the actual date, which will cause an exception. If we _are_ waiting,
			// then UpdateView will UpdateBlocks anyway (and we will set m_bNeedUpdate to ensure the same logic occurs).
			// (j.jones 2014-08-07 16:28) - PLID 63232 - this is not needed if the scheduler
			// is not active, as templates will be checked in UpdateView()
			if ((!m_bQuickScheduleNavigation && pSched->m_bIsDateSelComboDroppedDown) || pSched->m_bIsWaitingForTimedUpdate
				|| !bSchedulerIsActive
				// (d.thompson 2009-09-21) - PLID 35601 - If the resentry is in use, we cannot allow the template blocks
				//	to be refreshed.
				// (d.thompson 2009-10-21) - PLID 35598 - In a similar vein to the above, if the
				//	context menu is in use, a reference to the reservation is held as well.  We can't
				//	allow the scheduler to be refreshed then either.
				// (c.haag 2010-08-27 11:38) - PLID 40108 - Consolidated into one function
				|| (!pSched->IsSafeToUpdateView())
				)
			{
				pSched->m_bNeedUpdate = TRUE;
			} else {
				pSched->UpdateBlocks(true, true);
			}
		}
		else if (wParam == NetUtils::TemplateItemT)
		{
			// (c.haag 2006-12-11 16:01) - PLID 23808 - Make sure the templates accurately display today
			// We don't need to reload appointments.
			CNxSchedulerDlg* pSched = (CNxSchedulerDlg*)m_pActiveSheet;
			// (a.walling 2007-08-20 17:55) - PLID 27128 - If they are waiting for a timed update, then the
			// dropdown date does not match the actual date, which will cause an exception. If we _are_ waiting,
			// then UpdateView will UpdateBlocks anyway (and we will set m_bNeedUpdate to ensure the same logic occurs).
			// (j.jones 2014-08-07 16:28) - PLID 63232 - this is not needed if the scheduler
			// is not active, as templates will be checked in UpdateView()
			if ((!m_bQuickScheduleNavigation && pSched->m_bIsDateSelComboDroppedDown) || pSched->m_bIsWaitingForTimedUpdate
				|| !bSchedulerIsActive
				// (d.thompson 2009-09-21) - PLID 35601 - If the resentry is in use, we cannot allow the template blocks
				//	to be refreshed.
				// (d.thompson 2009-10-21) - PLID 35598 - In a similar vein to the above, if the
				//	context menu is in use, a reference to the reservation is held as well.  We can't
				//	allow the scheduler to be refreshed then either.
				// (c.haag 2010-08-27 11:38) - PLID 40108 - Consolidated into one function
				|| (!pSched->IsSafeToUpdateView())
				) 
			{
				pSched->m_bNeedUpdate = TRUE;
			} else {
				pSched->UpdateBlocks(true, true);
			}
		}
		else if(wParam == NetUtils::ResourceAvailTemplateT || wParam == NetUtils::ResourceAvailTemplateItemT) {
			//TES 6/21/2010 - PLID 5888 - We only need to reload if we're currently filtering on a location, otherwise these templates
			// don't show on the scheduler.
			CNxSchedulerDlg* pSched = (CNxSchedulerDlg*)m_pActiveSheet;
			if(pSched->GetLocationId() != -1) {
				// (a.walling 2007-08-20 17:55) - PLID 27128 - If they are waiting for a timed update, then the
				// dropdown date does not match the actual date, which will cause an exception. If we _are_ waiting,
				// then UpdateView will UpdateBlocks anyway (and we will set m_bNeedUpdate to ensure the same logic occurs).
				// (j.jones 2014-09-11 15:57) - PLID 63232 - this is not needed if the scheduler
				// is not active, as templates will be checked in UpdateView()
				if ((!m_bQuickScheduleNavigation && pSched->m_bIsDateSelComboDroppedDown) || pSched->m_bIsWaitingForTimedUpdate
					|| !bSchedulerIsActive
					// (d.thompson 2009-09-21) - PLID 35601 - If the resentry is in use, we cannot allow the template blocks
					//	to be refreshed.
					// (d.thompson 2009-10-21) - PLID 35598 - In a similar vein to the above, if the
					//	context menu is in use, a reference to the reservation is held as well.  We can't
					//	allow the scheduler to be refreshed then either.
					// (c.haag 2010-08-27 11:38) - PLID 40108 - Consolidated into one function
					|| (!pSched->IsSafeToUpdateView())
					) 
				{
					pSched->m_bNeedUpdate = TRUE;
				} else {
					pSched->UpdateBlocks(true, true);
				}
			}
		}

	}NxCatchAll("Error in OnTableChanged()");

	return 0;
}

LRESULT CSchedulerView::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {

		// (j.jones 2014-08-07 11:50) - PLID 63179 - do not process a table checker
		// if we have no active sheet
		if (m_pActiveSheet == NULL) {
			return 0;
		}

		bool bSchedulerIsActive = false;
		CChildFrame *pActiveFrame = GetMainFrame()->GetActiveViewFrame();
		if (pActiveFrame) {
			if (pActiveFrame->IsOfType(SCHEDULER_MODULE_NAME)) {
				bSchedulerIsActive = true;
			}
		}

		// (j.jones 2014-08-12 15:34) - PLID 63200 - send all Ex messages to ResEntry
		if (m_ResEntry != NULL)
		{
			m_ResEntry->SendMessage(WM_TABLE_CHANGED_EX, wParam, lParam);
		}

		// (j.jones 2014-08-21 17:21) - PLID 63186 - send all Ex messages to the MoveUpDlg
		if (m_MoveUpDlg.m_hWnd) {
			if (m_MoveUpDlg.IsWindowVisible()) {				
				m_MoveUpDlg.SendMessage(WM_TABLE_CHANGED_EX, wParam, lParam);
			}
		}

		CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;

		switch (wParam) {
			case NetUtils::AppointmentsT:
			{
				try {
					long nResID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiAppointmentID), -1);

					// (j.gruber 2011-02-17 14:13) - PLID 42424 - don't fire if the scheduler is not the active module
					if (bSchedulerIsActive) {
						//refresh it all
						((CNxSchedulerDlg*)m_pActiveSheet)->UpdateViewBySingleAppt(pDetails);
					}
					else {
						//set our variable so we know we need an update when we come back
						((CNxSchedulerDlg*)m_pActiveSheet)->m_bNeedUpdate = true;
					}
				} NxCatchAll("Error in CSchedulerView::OnTableChangedEx:AppointmentsT");
			}
			break;
			
			
			case NetUtils::CustomFieldName:
				// (b.cardillo 2006-07-06 18:01) - PLID 21342 - Notify the resentry dialog if the custom 
				// field being changed is one of the four in general1.
				try {
					// See which fields are being changed
					long nCustomFieldID = VarLong(pDetails->GetDetailData(1), -1);
					if (nCustomFieldID >= 1 && nCustomFieldID <= 4) {
						// It's one of the general 1 custom fields, so notify the resentry if it exists.
						CResEntryDlg *pResEntry = GetResEntry();
						if (pResEntry) {
							pResEntry->m_bG1CustomFieldNameChanged = TRUE;
						}
					}
				} NxCatchAll("CSchedulerView::OnTableChangedEx:CustomFieldName");
				break;
		}
	}
	NxCatchAll("Error in OnTableChangedEx()");

	return 0;
}

enum EAppointmentVisibleResult
{
	// The "visible" values all have the 1st bit cleared
	avrVisible_NoChange					= 0x00000001,
	avrVisible_OnDifferentDate			= 0x00000002,
	avrVisible_OnDifferentResource		= 0x00000004,

	
	// The "not visible" values all have the 1st bit set
	avrNotVisible_ResourcesOutOfRange	= 0x80000001, 
	avrNotVisible_TypeOutOfRange		= 0x80000002, 
	avrNotVisible_PurposesOutOfRange	= 0x80000004, 
	avrNotVisible_AppointmentIsCanceled = 0x80000008, 
	avrNotVisible_AppointmentDoesntExist = 0x80000010, 
	//TES 6/21/2010 - PLID 21341
	avrNotVisible_LocationOutOfRange	= 0x80000020,
};

// Returns 0 if dtDate is in the range between dtInRangeFrom and dtInRangeTo, inclusive
// Returns positive (1) if dtDate is after dtInRangeTo
// Returns negative (-1) if dtDate is before dtInRangeFrom
int CompareDateToDateRangeIgnoreTime(IN const COleDateTime &dtDate, IN const COleDateTime &dtInRangeFrom, IN const COleDateTime &dtInRangeTo)
{
	// See if the date in question is greater than or equal to the beginning of the date range
	if (CompareDateIgnoreTime(dtDate, dtInRangeFrom) >= 0) {
		// The date is indeed after the beginning of the range, now see if the date is less than or equal to the end of the range
		if (CompareDateIgnoreTime(dtDate, dtInRangeTo) <= 0) {
			// The date is indeed before the end of the range, so we're done
			return 0;
		} else {
			// Out of range after dtInRangeTo
			return 1;
		}
	} else {
		// Out of range before dtInRangeFrom
		return -1;
	}
}

// NOTE: The reason we don't have a simple "GetActiveDate()" like we have a "GetActiveResourceID()" is that we know 
// the multi-date views don't have their own horizontal scrollbars.  In other words, the date RANGE is itself a 
// single value, it changes depending on what view the user is in.
void CSchedulerView::GetActiveDateRange(OUT COleDateTime &dtFrom, OUT COleDateTime &dtTo) const
{
	// Get the from date, that's easy because we DO store it
	dtFrom = m_PivotDate;

	// The only tab that has more than one date is the week view.  Yes, even the month view doesn't show a 
	// date range, it only shows one day but then has a big side-bar that lets you see a quick-view of the 
	// month that contains the one date.

	const CNxDialog *pCurSheet = GetActiveSheet();
	if (pCurSheet->GetSafeHwnd()) {
		const CNxSchedulerDlg *pCurSchedSheet = (const CNxSchedulerDlg *)pCurSheet;

		if (pCurSchedSheet->Internal_IsMultiDateSheet()) {
			dtTo = dtFrom + COleDateTimeSpan(pCurSchedSheet->m_pSingleDayCtrl.GetDayTotalCount()-1, 0, 0, 0);
		} else {
			dtTo = dtFrom;
		}
	
	} else {
		ThrowNxException("CSchedulerView::GetActiveDateRange: There is no current sheet!");
	}
}

BOOL CSchedulerView::IsDateInCurView(IN const COleDateTime &dtDate) const
{
	// Get the active date range
	COleDateTime dtFrom, dtTo;
	GetActiveDateRange(dtFrom, dtTo);
	// See if the given date is in the range between dtFrom and dtTo
	if (CompareDateToDateRangeIgnoreTime(dtDate, dtFrom, dtTo) == 0) {
		// Good it's within the range
		return TRUE;
	} else {
		// The given date is outside the range
		return FALSE;
	}
}

// If pdtStartTime is given, the value is always set (if no exception is thrown), regardless of the return value of the function as a whole
long CSchedulerView::QueryAppointmentExistsInCurrentView(IN const long nAppointmentID, OUT COleDateTime &dtChangeToDate, OUT long &nChangeToResourceID)
{
	// Get info about the current view
	CNxSchedulerDlg *pActiveSchedSheet = ((CNxSchedulerDlg *)m_pActiveSheet);
	
	// (b.cardillo 2003-06-30 18:04) TODO: Right now we pull these off the current tab, but the right way is to store the 
	// values in the CSchedulerView itself, like we do for the resources.
	long nCurFilterAptTypeID = pActiveSchedSheet->GetPurposeSetId();
	long nCurFilterAptPurposeID = pActiveSchedSheet->GetPurposeId();
	//TES 6/21/2010 - PLID 21341 - Added a Location filter.
	long nCurFilterLocationID = pActiveSchedSheet->GetLocationId();

	BOOL bIsSingleResourceView = pActiveSchedSheet->Internal_IsMultiResourceSheet() ? FALSE : TRUE;

	
	// To put extra fields in the select clause
	CString strExtraSelectFields;
	
	// If we're on a single-resource tab, we need to add a field (that tells us the 
	// resource id we need to switch to) to our query 
	if (bIsSingleResourceView) {
		// We're on a single-resource tab so decide if we need to change to one of our other available resources or not
		CString strChangeToResource, strResourceIDs = CalcCurrentResourceIDsUnionQuery(FALSE);
		if(strResourceIDs != "") {
			strChangeToResource.Format(
				"	CASE WHEN EXISTS "
				"		(SELECT * FROM AppointmentResourceT WHERE AppointmentID = AppointmentsT.ID AND ResourceID = %li) "
				"	 THEN -1 "
				"	ELSE ("
				"	 SELECT TOP 1 ResourceID "
				"	 FROM (%s) A "
				"	 WHERE ResourceID IN ("
				"		 SELECT ResourceID "
				"		 FROM AppointmentResourceT "
				"		 WHERE AppointmentID = AppointmentsT.ID"
				"		)"
				"	) END AS ChangeToResource ", GetActiveResourceID(), strResourceIDs);
		}
		else {
			strChangeToResource = " -2 AS ChangeToResource";
		}
		strExtraSelectFields += ", " + strChangeToResource;
	} else {
		// We're on a multi-resource tab so all we have to do is see if the appointment is on any of the current resources 
		CString strHasAnyCurrentResource;
		strHasAnyCurrentResource.Format(
			"	CASE WHEN EXISTS "
			"		(SELECT * FROM AppointmentResourceT A WHERE A.AppointmentID = AppointmentsT.ID AND A.ResourceID IN (%s)) "
			"	 THEN CONVERT(BIT, 1) ELSE CONVERT(BIT, 0) END AS HasAnyCurrentResource ", CalcCurrentResourceIDs());
		strExtraSelectFields += ", " + strHasAnyCurrentResource;
	}

	// See if we're filtering on a purpose
	if (nCurFilterAptPurposeID != -1) {
		// (r.gonet 10-05-2010 16:01) - PLID 39289 - If the purposes we are filtering on have multiple purposes, see if we have an intersection
		CString strHasPurpose;
		if (nCurFilterAptPurposeID == -2) {
			CString strMultiPurposeIDs;
			CDWordArray dwaryMultiPurposeIDs;
			// (r.gonet 10-05-2010 16:01) - PLID 39289 - Get all of the purposes the view is filtering on
			pActiveSchedSheet->GetPurposeFilterIDs(&dwaryMultiPurposeIDs);
			// (r.gonet 10-05-2010 16:01) - PLID 39289 - Then make the purposes into an sql friendly string
			for(int i = 0; i < dwaryMultiPurposeIDs.GetSize(); i++) {
				strMultiPurposeIDs.AppendFormat("%li,", (long)dwaryMultiPurposeIDs.GetAt(i));
			}
			strMultiPurposeIDs.TrimRight(",");
			// (r.gonet 10-05-2010 16:01) - PLID 39289 - If we have multiple purpose filters, we must have at least one purpose id
			ASSERT(!strMultiPurposeIDs.IsEmpty());
			strHasPurpose.Format(
				"	CASE WHEN EXISTS "
				"		(SELECT * FROM AppointmentPurposeT A WHERE A.AppointmentID = AppointmentsT.ID AND A.PurposeID IN (%s)) "
				"	 THEN CONVERT(BIT, 1) ELSE CONVERT(BIT, 0) END AS HasVisiblePurpose ", strMultiPurposeIDs);

		} else {
			strHasPurpose.Format(
				"	CASE WHEN EXISTS "
				"		(SELECT * FROM AppointmentPurposeT A WHERE A.AppointmentID = AppointmentsT.ID AND A.PurposeID = %li) "
				"	 THEN CONVERT(BIT, 1) ELSE CONVERT(BIT, 0) END AS HasVisiblePurpose ", nCurFilterAptPurposeID);
		}
		strExtraSelectFields += ", " + strHasPurpose;
	}

	//TES 6/21/2010 - PLID 21341 - See if we're filtering on a location
	if (nCurFilterLocationID != -1) {
		CString strHasLocation;
		// (d.singleton 2012-07-02 15:20) - PLID 47473 be able to choose multiple locations.
		if(nCurFilterLocationID == -2) {
			CString strMultiLocationIDs;
			CDWordArray dwaryLocationIDs;
			// (d.singleton 2012-07-02 15:22) - PLID 47473 get all locations in filter
			pActiveSchedSheet->GetLocationFilterIDs(&dwaryLocationIDs);
			// (d.singleton 2012-07-02 15:22) - PLID 47473 make sql friendly
			for(int i = 0; i < dwaryLocationIDs.GetCount(); i++) {
				strMultiLocationIDs.AppendFormat("%li,", (long)dwaryLocationIDs.GetAt(i));
			}
			strMultiLocationIDs.TrimRight(",");
			// (d.singleton 2012-07-02 15:29) - PLID 47473 if multiple selected must have at least one value
			ASSERT(!dwaryLocationIDs.IsEmpty());
			strHasLocation.Format(
			"	CASE WHEN LocationID IN (%s) "
			"	 THEN CONVERT(BIT, 1) ELSE CONVERT(BIT, 0) END AS HasVisibleLocation", strMultiLocationIDs);
			strExtraSelectFields += ", " + strHasLocation;
		}
		else {
			strHasLocation.Format(
				"	CASE WHEN LocationID = %li "
				"	 THEN CONVERT(BIT, 1) ELSE CONVERT(BIT, 0) END AS HasVisibleLocation", nCurFilterLocationID);
			strExtraSelectFields += ", " + strHasLocation;
		}
	}

	

	// Get the recorddset that gives one record with all the info we need
	// (d.thompson 2010-05-06) - PLID 38525 - Cannot be parameterized -- the select clause has several possible embedded ID lists.
	// (j.luckoski 2012-06-20 10:17) - PLID 11597 - Added Visible case to determine if we are going to see the cancelled appt or not 
	// if so then it doesn't prevent the viewing and shows the cancelled appt, if it is invisible it alerts you and does nothing on the schduler
	_RecordsetPtr prs;
	if(GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true)) {
		prs = CreateRecordset(
			"SELECT Status, StartTime, AptTypeID, "
			" CASE WHEN CancelledDate IS NULL THEN CONVERT(BIT, 1) WHEN CancelledDate > DATEADD(hh, -%li, StartTime) THEN CONVERT(BIT, 1) ELSE CONVERT(BIT, 0) END AS Visible "
			"   %s "
			"FROM AppointmentsT "
			"WHERE ID = %li ", GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true), strExtraSelectFields, nAppointmentID);
	} else {
		prs = CreateRecordset(
			"SELECT Status, StartTime, AptTypeID, "
			" CASE WHEN CancelledDate IS NULL THEN CONVERT(BIT,1) ELSE CONVERT(BIT, 0) END AS Visible "
			"   %s "
			"FROM AppointmentsT "
			"WHERE ID = %li ", strExtraSelectFields, nAppointmentID);
	}
	
	FieldsPtr pflds = prs->GetFields();

	// Generate our return value based on the fields in the record
	if (!prs->eof) {

		// Bit-wise OR everything else in as appropriate
		long nORedFailValues = 0;
		if (AdoFldByte(pflds, "Status") == 4) {
			// (j.luckoski 2012-06-20 10:18) - PLID 11597 - Wrap appointmentIsCancelled to determine if its visible or not, and if so then

			if(AdoFldBool(pflds, "Visible") == 0) {
				nORedFailValues |= avrNotVisible_AppointmentIsCanceled;
			}
		}
		if (nCurFilterAptTypeID != -1 /*-1 is ALL types*/) {

			long nAptTypeID = AdoFldLong(pflds, "AptTypeID", -1);
			if(nCurFilterAptTypeID != -2 /*-2 is Multiple types*/ 
				&& nAptTypeID != nCurFilterAptTypeID) {
				// (r.gonet 10-05-2010 16:01) - PLID 39289 - We are filtering on a single type and
				//   this appointment's type doesn't match the filter
				nORedFailValues |= avrNotVisible_TypeOutOfRange;
			
			} else if (nCurFilterAptTypeID == -2) {
				// (r.gonet 10-05-2010 16:01) - PLID 39289 - We are filtering on multiple types,
				//  see if our type is in the filter. If not, the appointment should not exist in the current view.
				CDWordArray dwaryMultiTypeIDs;
				pActiveSchedSheet->GetTypeFilterIDs(&dwaryMultiTypeIDs);
				BOOL bTypeFound = FALSE;
				for(int i = 0; i < dwaryMultiTypeIDs.GetSize(); i++) {
					if(dwaryMultiTypeIDs.GetAt(i) == (DWORD)nAptTypeID) {
						bTypeFound = TRUE;
						break;
					}
				}
				
				if(!bTypeFound) {
					// (r.gonet 10-05-2010 16:01) - PLID 39289 - There was no intersection, so the appointment will not be in the current view.
					nORedFailValues |= avrNotVisible_TypeOutOfRange;
				}
			}
		}
		if (nCurFilterAptPurposeID != -1 && !AdoFldBool(pflds, "HasVisiblePurpose")) {
			nORedFailValues |= avrNotVisible_PurposesOutOfRange;
		}
		//TES 6/21/2010 - PLID 21341 - Check the location filter
		if (nCurFilterLocationID != -1 && !AdoFldBool(pflds, "HasVisibleLocation")) {
			nORedFailValues |= avrNotVisible_LocationOutOfRange;
		}

		if (nORedFailValues == 0) {
			// The appointment is either visible in our current view, or it would be visible if we changed the 
			// date and/or active resource
			
			// First check the date
			{
				// Get the date range of the current sheet
				COleDateTime dtCurFromDate, dtCurToDate;
				GetActiveDateRange(dtCurFromDate, dtCurToDate);
				// And see if the appointment's date is in that range
				COleDateTime dtApptDate = AdoFldDateTime(pflds, "StartTime");
				if (CompareDateToDateRangeIgnoreTime(dtApptDate, dtCurFromDate, dtCurToDate) != 0) {
					// Need to change the date
					nORedFailValues |= avrVisible_OnDifferentDate;
					// Tell the caller what the date should be changed TO
					dtChangeToDate.SetDate(dtApptDate.GetYear(), dtApptDate.GetMonth(), dtApptDate.GetDay());
				}
			}

			// Now check the resources
			if (bIsSingleResourceView) {
				// For a single-resource sheet we may need to change resources to some other one that's available in our view
				long nChangeToResourceVal = AdoFldLong(pflds, "ChangeToResource", -2); // The value could be null, in which case we use -2 as our sentinel
				if (nChangeToResourceVal == -1) {
					// Our active resource is the one we want, so do nothing
				} else if (nChangeToResourceVal == -2) {
					// There is no resource in our current view that would have this appointment in it
					return avrNotVisible_ResourcesOutOfRange;
				} else {
					// Some resource in our current view has this appointment, but it's not our active resource so the caller needs to change to this resource
					nChangeToResourceID = nChangeToResourceVal;
					nORedFailValues |= avrVisible_OnDifferentResource;
				}
			} else {
				// For a multi-resource sheet the appointment is either on a resource in the current view, or not
				if (!AdoFldBool(pflds, "HasAnyCurrentResource")) {
					nORedFailValues |= avrNotVisible_ResourcesOutOfRange;
				}
			}

			// If after ALL that, nORedFailValues is still 0, that means there's absolutely nothing 
			// wrong and the appointment should be visible in the current view without any changes.
			if (nORedFailValues == 0) {
				nORedFailValues = avrVisible_NoChange;
			}
		}

		// Now this is an incomplete assert, but it could help us find bugs in certain cases.  If this assert fires 
		// it means either:
		//  - there is an appointment on screen right now that has the given appointment id yet the above query thinks it shouldn't be, or
		//  - the above code thinks the given appointment should be on screen right now but it's not
		ASSERT(
			((CNxSchedulerDlg *)m_pActiveSheet)->GetAppointmentReservation(__FUNCTION__, nAppointmentID, NULL) != NULL ? 
				(nORedFailValues == avrVisible_NoChange) : // the appointment's on screen so we better be returning "visible no change"
				(nORedFailValues != avrVisible_NoChange)); // the appointment's not on screen so we better not be returning "visible no change"

		// The OR'd value is our answer because either something failed and it's non-zero or 
		// nothing failed and it's still avrVisible
		return nORedFailValues;
	} else {
		// There was no appointment record with the given ID!
		return avrNotVisible_AppointmentDoesntExist;
	}
}

void CSchedulerView::OpenAppointment(long nAppointmentID, bool bShowResEntry /*=false*/)
{
	// Check to see if the appointment is visible given our current tab, and our current criteria
	COleDateTime dtChangeToDate;
	long nChangeToResourceID;
	long neavrs = QueryAppointmentExistsInCurrentView(nAppointmentID, dtChangeToDate, nChangeToResourceID);

	// See if it's visible in this view, even if we have to change dates or change resources to show it
	if ((neavrs & 0x80000000) == 0) {
		// It's visible in this view possibly with some change to the filtering in the view
		((CNxSchedulerDlg *)m_pActiveSheet)->ShowAppointment(
			nAppointmentID, bShowResEntry,
			(neavrs & avrVisible_OnDifferentDate) ? &dtChangeToDate : NULL, 
			(neavrs & avrVisible_OnDifferentResource) ? &nChangeToResourceID : NULL);
	} else {
		// It's not visible for some set of reasons, list the reasons
		// TODO: Some of these we might want to prompt the user with a friendly message asking if he wants us to 
		// change the view to make the appointment visible (like the type- and purpose-filters for example)
		CString strReasonsNotVisible;
		if ((neavrs & avrNotVisible_ResourcesOutOfRange) == avrNotVisible_ResourcesOutOfRange) {
			strReasonsNotVisible += "   - the appointment does not have any resources in the current view\r\n";
		}
		if ((neavrs & avrNotVisible_TypeOutOfRange) == avrNotVisible_TypeOutOfRange) {
			strReasonsNotVisible += "   - the current view is filtered on a type or types other than that of the appointment\r\n";
		}
		if ((neavrs & avrNotVisible_PurposesOutOfRange) == avrNotVisible_PurposesOutOfRange) {
			strReasonsNotVisible += "   - the current view is filtered on a purpose or purposes that the appointment does not use\r\n";
		}
		if ((neavrs & avrNotVisible_AppointmentIsCanceled) == avrNotVisible_AppointmentIsCanceled) {
			strReasonsNotVisible += "   - the appointment is cancelled\r\n";
		}
		//TES 6/21/2010 - PLID 21341 - Check the location filter.
		if ((neavrs & avrNotVisible_LocationOutOfRange) == avrNotVisible_LocationOutOfRange) {
			strReasonsNotVisible += "   - the current view is filtered on a location or locations other than that of the appointment\r\n";
		}
		if ((neavrs & avrNotVisible_AppointmentDoesntExist) == avrNotVisible_AppointmentDoesntExist) {
			strReasonsNotVisible += "   - the appointment could not be found\r\n";
		}
		DeleteEndingString(strReasonsNotVisible, "\r\n");
		
		// Generate the final string we'll be giving to the user to tell her why the appointment couldn't be displayed
		CString strMsg;
		if (!strReasonsNotVisible.IsEmpty()) {
			strMsg.Format(
				"The appointment could not be displayed for the following reason(s):\r\n\r\n"
				"%s", strReasonsNotVisible);
		} else {
			ASSERT(FALSE);
			strMsg = "The appointment could not be opened for an undetermined reason.";
		}
		MessageBox(strMsg, NULL, MB_OK|MB_ICONEXCLAMATION);
	}
}

long CSchedulerView::GetFirstHourOfDay(int iDayOfWeek /*= COleDateTime::GetCurrentTime().GetDayOfWeek()*/)
{
	CString strField;
	_RecordsetPtr rs;
	COleDateTime dt;

	switch (iDayOfWeek)
	{
	case 1: strField = "SundayOpen"; break;
	case 2: strField = "MondayOpen"; break;
	case 3: strField = "TuesdayOpen"; break;
	case 4: strField = "WednesdayOpen"; break;
	case 5: strField = "ThursdayOpen"; break;
	case 6: strField = "FridayOpen"; break;
	case 7: strField = "SaturdayOpen"; break;
	default: return -1;
	}

	// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
	rs = CreateParamRecordset( FormatString("SELECT %s FROM LocationsT WHERE ID = {INT}", strField), GetCurrentLocationID());
	if (rs->eof) return -1;

	dt = rs->Fields->Item[(LPCTSTR)strField]->Value;
	return dt.GetHour();
}

void CSchedulerView::OnShowTemplateNamesBtn()
{
	// Flip it
	CWaitCursor wc;
	m_bShowTemplateText = !m_bShowTemplateText;
	SetRemotePropertyInt("ResShowTemplateName", m_bShowTemplateText ? 1 : 0, 0, GetCurrentUserName());
	if (m_pActiveSheet) {
		((CNxSchedulerDlg *)m_pActiveSheet)->m_bNeedUpdate = true;
	}
	// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
	PostMessage(NXM_UPDATEVIEW);
}

void CSchedulerView::OnShowAppointmentLocations()
{
	// Flip it off, or on
	CWaitCursor wc;
	m_bShowAppointmentLocations = !m_bShowAppointmentLocations;
	SetRemotePropertyInt("ResShowLocName", m_bShowAppointmentLocations ? 1 : 0, 0, GetCurrentUserName());
	
	
	if (m_pActiveSheet) {
		((CNxSchedulerDlg *)m_pActiveSheet)->m_bNeedUpdate = true;
	}
	// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
	PostMessage(NXM_UPDATEVIEW);
}

void CSchedulerView::OnUpdateShowAppointmentLocations(CCmdUI* pCmdUI)
{
	m_bShowAppointmentLocations = GetRemotePropertyInt("ResShowLocName", 0, 0, GetCurrentUserName(), true) ? TRUE : FALSE;

	if (m_bShowAppointmentLocations) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void CSchedulerView::OnShowAppointmentResources()
{
	// Flip it off, or on
	CWaitCursor wc;
	m_bShowAppointmentResources = !m_bShowAppointmentResources;
	SetRemotePropertyInt("ResShowResourceName", m_bShowAppointmentResources ? 1 : 0, 0, GetCurrentUserName());
	
	
	if (m_pActiveSheet) {
		((CNxSchedulerDlg *)m_pActiveSheet)->m_bNeedUpdate = true;
	}
	// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
	PostMessage(NXM_UPDATEVIEW);
}

void CSchedulerView::OnUpdateShowAppointmentResources(CCmdUI* pCmdUI)
{
	m_bShowAppointmentResources = GetRemotePropertyInt("ResShowResourceName", 0, 0, GetCurrentUserName(), false) ? TRUE : FALSE;

	if (m_bShowAppointmentResources) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}


void CSchedulerView::OnQuickScheduleNavigation()
{
	CWaitCursor wc;
	m_bQuickScheduleNavigation = !m_bQuickScheduleNavigation;
	SetRemotePropertyInt("ResQuickScheduleNavigation", m_bQuickScheduleNavigation ? 1 : 0, 0, GetCurrentUserName());

	if (m_bQuickScheduleNavigation) {
		MessageBox(
			"You have turned 'Quick Schedule Navigation' ON.\r\n\r\n"
			"As you navigate through the drop down calendar (at the top of the Scheduler) the "
			"display will change to reflect the appointments for the highlighted date.\r\n\r\n"
			"NOTE: This user preference has no effect on the way appointments are saved or displayed.", 
			NULL, MB_OK|MB_ICONINFORMATION);
	} else {
		MessageBox(
			"You have turned 'Quick Schedule Navigation' OFF.\r\n\r\n"
			"As you navigate through the drop down calendar (at the top of the Scheduler) the "
			"display will not change until you have made your date selection.\r\n\r\n"
			"NOTE: This user preference has no effect on the way appointments are saved or displayed.", 
			NULL, MB_OK|MB_ICONINFORMATION);
	}
}

void CSchedulerView::OnUpdateQuickScheduleNavigation(CCmdUI* pCmdUI)
{
	if (m_bQuickScheduleNavigation) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void CSchedulerView::OnShowPatientInformation()
{
	CWaitCursor wc;
	m_bShowPatientInformation = !m_bShowPatientInformation;
	SetRemotePropertyInt("ResShowPatInfo", m_bShowPatientInformation ? 1 : 0, 0, GetCurrentUserName());
	
	
	if (m_pActiveSheet) {
		((CNxSchedulerDlg *)m_pActiveSheet)->m_bNeedUpdate = true;
	}
	// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
	PostMessage(NXM_UPDATEVIEW);
}

void CSchedulerView::OnUpdateShowPatientInformation(CCmdUI* pCmdUI)
{
	m_bShowPatientInformation = GetRemotePropertyInt("ResShowPatInfo", 1, 0, GetCurrentUserName(), true) ? TRUE : FALSE;

	if (m_bShowPatientInformation) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void CSchedulerView::OnShowPatientWarningBtn()
{
	// Flip it off, or on
	CWaitCursor wc;
	m_bShowPatientWarning = !m_bShowPatientWarning;
	SetRemotePropertyInt("ResShowPatientWarning", m_bShowPatientWarning ? 1 : 0, 0, GetCurrentUserName());
	if (m_pActiveSheet) {
		((CNxSchedulerDlg *)m_pActiveSheet)->m_bNeedUpdate = true;
	}
	// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
	PostMessage(NXM_UPDATEVIEW);
}

void CSchedulerView::OnUpdateShowPatientWarningBtn(CCmdUI* pCmdUI)
{
	if (m_bShowPatientWarning) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
	pCmdUI->Enable(TRUE);
}

void CSchedulerView::OnUpdateShowTemplateNamesBtn(CCmdUI* pCmdUI)
{
	if (m_bShowTemplateText) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
	pCmdUI->Enable(TRUE);
}

void CSchedulerView::OnShowMoveUpList() {
//this shouldn't be needed, but just in case

	//TES 12/17/2008 - PLID 32478 - This ability is only available in the Enterprise edition, so check the license.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Designating appointments as requesting a move up", "The_Scheduler_Module/manage_my_moveup_appointments.htm")) {
		return;
	}

	// (c.haag 2003-08-08 13:03) - Make sure it doesn't exist before
	// trying to create it.
	if (!m_MoveUpDlg.GetSafeHwnd())
	{
		m_MoveUpDlg.Create(IDD_SHOW_MOVEUP_LIST, GetMainFrame());
		m_MoveUpDlg.SetWindowPos(&CWnd::wndTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW | SWP_NOZORDER);
	}
	m_MoveUpDlg.ShowWindow(SW_SHOW);
	// (z.manning, 04/30/2008) - PLID 29814 - Center this when showing it!
	m_MoveUpDlg.CenterWindow();
}

void CSchedulerView::OnUpdateShowScheduledPatients(CCmdUI* pCmdUI)
{
	if (m_pdlgScheduledPatients->GetSafeHwnd() && m_pdlgScheduledPatients->IsWindowVisible()) {
		pCmdUI->SetCheck(1);
	} else {
		pCmdUI->SetCheck(0);
	}
}

void CSchedulerView::OnShowScheduledPatients()
{
	if (m_pdlgScheduledPatients->GetSafeHwnd() && m_pdlgScheduledPatients->IsWindowVisible()) {
		// Destroy and deallocate the existnig one
		if (m_pdlgScheduledPatients) {
			if (m_pdlgScheduledPatients->GetSafeHwnd()) {
				m_pdlgScheduledPatients->DestroyWindow();
			}
			delete m_pdlgScheduledPatients;
			m_pdlgScheduledPatients = NULL;
		}
	} else {
		// If it already exists destroy and de-allocate it
		if (m_pdlgScheduledPatients) {
			if (m_pdlgScheduledPatients->GetSafeHwnd()) {
				m_pdlgScheduledPatients->DestroyWindow();
			}
			delete m_pdlgScheduledPatients;
			m_pdlgScheduledPatients = NULL;
		}

		// Create it
		m_pdlgScheduledPatients = new CScheduledPatientsDlg(this);
		m_pdlgScheduledPatients->Create(m_pdlgScheduledPatients->IDD, this);
		m_pdlgScheduledPatients->ShowWindow(SW_SHOW);

		// Refresh the scheduler to fill it
		BeginWaitCursor();
		if (m_pActiveSheet) {
			((CNxSchedulerDlg *)m_pActiveSheet)->m_bNeedUpdate = true;
		}
		// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
		PostMessage(NXM_UPDATEVIEW);
		EndWaitCursor();
	}
}

void CSchedulerView::ReloadLocationDisplay()
{
	m_bShowAppointmentLocations = GetRemotePropertyInt("ResShowLocName", 0, 0, GetCurrentUserName(), true) == 0 ? false : true;
}

//TES 3/25/2008 - PLID 24157 - Moved to globalutils
/*BOOL IsWindowDescendent(const HWND hwndAncestor, const HWND hwndDescendent)
{
	HWND hwndParent = ::GetParent(hwndDescendent);
	while (hwndParent) {
		if (hwndAncestor == hwndParent) {
			return TRUE;
		} else {
			hwndParent = ::GetParent(hwndParent);
		}
	}
	return FALSE;
}*/

HWND GetSingleDayFromSheet(CNxDialog *pSheet)
{
	// Make sure the sheet exists and is derived from CNxSchedulerDlg
	if (pSheet->GetSafeHwnd() /*TODO: CNxSchedulerDlg needs to support IsKindOf    && pSheet->IsKindOf(RUNTIME_CLASS(CNxSchedulerDlg))*/) {
		// Now that we know it is a CNxSchedulerDlg we can cast it as such
		CNxSchedulerDlg *pDlg = (CNxSchedulerDlg *)pSheet;
		// Make sure it has a valid singleday control on it
		if (pDlg->m_pSingleDayCtrl) {
			// It has a valid singleday, so finally get the hwnd of the singleday
			return (HWND)pDlg->m_pSingleDayCtrl.GethWnd();
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

BOOL CSchedulerView::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message) {
	case WM_MOUSEWHEEL:
		{
			// Find out what window (child, popup, whatever) that the mouse is over while using the scrollwheel
			// (a.walling 2009-04-01 14:10) - PLID 33796 - using LOWORD/HIWORD will mess up the sign for multiple monitors
			CWnd *pWnd = CWnd::WindowFromPoint(CPoint(pMsg->lParam));
			if (pWnd->GetSafeHwnd()) {
				// Get the HWND of the singleday control on the active sheet
				HWND hWndSingleDay = GetSingleDayFromSheet(m_pActiveSheet);
				// Make sure it's not NULL
				if (hWndSingleDay) {
					// We have a singleday, now check to see if the singleday or one of its descendents 
					// was the window that the mouse was over while using the scrollwheel
					if (hWndSingleDay == pWnd->m_hWnd || IsWindowDescendent(hWndSingleDay, pWnd->m_hWnd)) {
						// The "scrollwheeled" window was the singleday control or one of its descendents so 
						// offer the scrollwheel action to that window instead of passing it to the parent
						if (::SendMessage(pWnd->m_hWnd, WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam) == 0) {
							return TRUE;
						}
					}
				}
			}
		}
		break;
	default:
		break;
	}
	
	return CNxTabView::PreTranslateMessage(pMsg);
}

void CSchedulerView::GetWorkingResourceAndDate(IN const CNxSchedulerDlg *pExpectActiveSheet, IN const long nColumnIndex, OUT long &nWorkingResourceID, OUT COleDateTime &dtWorkingDate) const
{
	CSchedViewColumnInfo vci;
	GetWorkingResourceAndDate(pExpectActiveSheet, nColumnIndex, vci);
	nWorkingResourceID = vci.GetResourceID();
	dtWorkingDate = vci.GetDate();
}

void CSchedulerView::GetWorkingResourceAndDate(IN const CNxSchedulerDlg *pExpectActiveSheet, IN const long nColumnIndex, OUT CSchedViewColumnInfo &vci) const
{
	if (pExpectActiveSheet) {
		if ((const CNxDialog *)pExpectActiveSheet == GetActiveSheet()) {
			// Ask the active sheet (note: the sheet may throw its own exceptions)
			long nLocWorkingResourceID;
			COleDateTime dtLocWorkingDate;
			pExpectActiveSheet->Internal_GetWorkingResourceAndDateFromGUI(nColumnIndex, nLocWorkingResourceID, dtLocWorkingDate);
			// See if the given resource is in view (I can't think of any reason the user would be able to click somehow 
			// and have an appointment pop up where the appointment STARTS OUT with a resource that somehow is not in the 
			// current view.  That just doesn't make sense.  I CAN believe the user might then want to change to have 
			// some resources that aren't in view, but the interface shouldn't let them start out that way.)
			ASSERT(IsResourceInCurView(nLocWorkingResourceID));
			if (IsResourceInCurView(nLocWorkingResourceID)) {
				// We have a good resource list, now see if we have a good date
				// (a.walling 2007-09-26 09:45) - PLID 27128
				// ASSERT(IsDateInCurView(dtLocWorkingDate));
				if (IsDateInCurView(dtLocWorkingDate)) {
					// All the info is good, go ahead and pass it out and we're done
					vci.Set(dtLocWorkingDate, nLocWorkingResourceID);
				} else {
					// The date was not in the current view
					// (a.walling 2007-08-13 15:36) - PLID 27128 - Help pin down this random but reoccuring error.
					// (a.walling 2007-08-20 16:14) - PLID 27128 - Well that was quick, I now know why this is occurring and also
					// have a plan to fix. See the assoc. PLID. Will be taken care of next scope. Hooray!
					// (a.walling 2007-09-26 08:57) - PLID 27128 - This is still happening, so I thought to myself, do we
					// really need an exception here? Is this really an error state? The answer is NO. We are getting the date from
					// the DTPicker, however that may not always be in sync with the actual displayed date. So if the DTPicker date
					// does not fit in the curview, what about the current pivot date? It is possible a user may be scrolling through
					// the picker and then ESC to not change the date.
					//COleDateTime dtFrom, dtTo;
					//GetActiveDateRange(dtFrom, dtTo);
					//ThrowNxException(FormatString("GetWorkingResourceAndDate: The working date (%s) is not available in the current view (%s to %s)!", FormatDateTimeForInterface(dtLocWorkingDate), FormatDateTimeForInterface(dtFrom), FormatDateTimeForInterface(dtTo)));
					dtLocWorkingDate = m_PivotDate;
					vci.Set(dtLocWorkingDate, nLocWorkingResourceID);
				}
			} else {
				// There was at least one resource that is not in view
				ThrowNxException("GetWorkingResourceAndDate: The working resource is not available in the current view!");
			}
		} else {
			// The given expected active sheet is not the actual active sheet
			ThrowNxException("GetWorkingResourceAndDate: The current tab is not the expected one!");
		}
	} else {
		// The given expected active sheet was not given
		ThrowNxException("GetWorkingResourceAndDate: No expected tab was given!");
	}
}

const long CSchedulerView::GetLastLoadedResourceViewID(IN const CNxSchedulerDlg *pExpectActiveSheet) const
{
	if (pExpectActiveSheet) {
		if ((CNxDialog *)pExpectActiveSheet == GetActiveSheet()) {
			if (m_nLastLoadedResourceViewID != srvUnknownResourceView) {
				// For now this is the extent of our checking (maybe someday the tabs will display the current 
				// resource view to the user, and in such an event we'd have to write a function called 
				// "Internal_GetCurrentResourceViewIDFromGUI" that would be a pure virtual in CSchedulerViewSheet)
				return m_nLastLoadedResourceViewID;
			} else {
				// The current view is not set
				ThrowNxException("GetCurrentResourceViewID: There current resource view has not bee set!");
			}
		} else {
			// The given expected active sheet is not the actual active sheet
			ThrowNxException("GetCurrentResourceViewID: The current tab is not the expected one!");
		}
	} else {
		// The given expected active sheet was not given
		ThrowNxException("GetCurrentResourceViewID: No expected tab was given!");
	}
}

CString CSchedulerView::GetLastLoadedResourceViewName(IN const CNxSchedulerDlg *pExpectActiveSheet)
{
	// (c.haag 2006-04-12 13:15) - PLID 20104 - Update the name of the current view
	// if someone changed it
	if (m_resourceviewnameChecker.Changed()) {
		LoadResourceViewName();
	}
	return m_strLastLoadedResourceViewName;
}

void CSchedulerView::LoadResourceViewName()
{
	// (c.haag 2006-04-12 13:12) - PLID 20104 - This loads the name of the current resource
	// view into memory so that we don't have to keep querying it each time we call UpdateView()
	// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
	_RecordsetPtr prsViews = CreateParamRecordset("SELECT Name FROM ResourceViewsT WHERE ID = {INT}", m_nLastLoadedResourceViewID);
	if (prsViews->eof) {
		m_strLastLoadedResourceViewName = "Standard View";
	} else {
		m_strLastLoadedResourceViewName = "View: " + AdoFldString(prsViews, "Name");
	}
}

//TES 7/26/2010 - PLID 39445 - Added nNewDefaultLocationID.  -2 = <No Default> (leave the location filter alone), -1 = {All Locations},
// positive value = a specific location to filter on.
void CSchedulerView::SetCurrentResourceViewID(const IN long nNewResourceViewID, const IN long nNewDefaultLocationID)
{
	// It's distracting to have it clear and disable and then redraw (it's way worse 
	// than just a flicker) so we're turning off the redraw for the whole module first.
	SetRedraw(FALSE);

	//TES 7/26/2010 - PLID 39445 - Set the default location.
	if(nNewDefaultLocationID != -2) {
		m_nLocationId = nNewDefaultLocationID;
		((CNxSchedulerDlg*)m_pActiveSheet)->SetActiveLocationId(m_nLocationId);
	}

	// Just load the resource list (that function cascades the changes down to each sheet)
	LoadResourceList(nNewResourceViewID, m_nCurrentResourceID);

	// Now turn the redraw back on, invalidate, and redraw the window (if we don't 
	// invalidate after turning redraw back on, then we don't get a paint message and the 
	// user is really really confused)
	SetRedraw(TRUE);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN);
}

// This is a UI handler, in other words only call this in response to the user clicking a button, etc.
void CSchedulerView::OnReloadCurrentResourceList()
{
	// It's distracting to have it clear and disable and then redraw (it's way worse 
	// than just a flicker) so we're turning off the redraw for the whole module first.
	SetRedraw(FALSE);

	// Just load the resource list (that function cascades the changes down to each sheet)
	LoadResourceList(m_nLastLoadedResourceViewID, m_nCurrentResourceID);

	// Now turn the redraw back on, invalidate, and redraw the window (if we don't 
	// invalidate after turning redraw back on, then we don't get a paint message and the 
	// user is really really confused)
	SetRedraw(TRUE);
	RedrawWindow(NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN);
}

void CSchedulerView::LoadShowStateArray()
{
	try {
		m_adwShowState.RemoveAll();
		m_astrShowState.RemoveAll();
		_RecordsetPtr prs = CreateRecordset("SELECT ID, [Name] FROM AptShowStateT ORDER BY ID");
		while (!prs->eof)
		{
			CString strName = AdoFldString(prs, "Name", "");
			m_adwShowState.Add(AdoFldLong(prs, "ID"));
			m_astrShowState.Add(strName);
			prs->MoveNext();
		}
	}
	NxCatchAll("Error refreshing appointment status list");
}

BOOL CSchedulerView::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {
		switch (LOWORD(wParam))
		{
		case ID_APPT_IN: OnResMarkIn(); break;
		case ID_APPT_OUT: OnResMarkOut(); break;
		case ID_APPT_PENDING: OnResMarkPending(); break;
		case ID_APPT_RECEIVED: OnResMarkReceived(); break;
		case ID_APPT_NOSHOWSTATUS: OnResMarkNoShow(); break;
		case ID_APPT_SHOW: OnResMarkShow(); break;
		default:
			if (LOWORD(wParam) > ID_APPT_RECEIVED) {
				long nID = LOWORD(wParam) - ID_APPT_PENDING;
				for (long i=0; i < m_adwShowState.GetSize(); i++)
				{
					if ((long)m_adwShowState[i] == nID)
					{
						OnResMarkUserDefinedShowState(m_astrShowState[i]);
						break;
					}
				}
				break;
			}		
		}
	} NxCatchAll("Error in CSchedulerView::OnCommand"); // (a.walling 2007-08-31 10:06) - PLID 27265

	// (a.walling 2007-08-31 12:29) - PLID 27265 - This will call any message handlers not explicitly defined here.
	BOOL bBaseResult = CNxTabView::OnCommand(wParam, lParam);

	try {
		// (a.walling 2007-08-31 14:57) - PLID 27265 - Only clear out the smart pointer if a menu action was processed
		switch(LOWORD(wParam)) {
			case ID_RES_PASTE:
			case ID_RES_EDIT:
			case ID_RES_CUT:
			case ID_RES_COPY:
			case ID_RES_DELETE:
			case ID_RES_DELETEBLOCK: // (c.haag 2007-11-30 13:19) - PLID 28096 - Clear out when deleting precision template blocks
			case ID_APPT_NOSHOWSTATUS:
			case ID_APPT_UNCONFIRMED:
			case ID_APPT_CONFIRMED:
			case ID_APPT_LEFTMESSAGE: //(e.lally 2009-09-28) PLID 18809
			case ID_APPT_REMOVE_MOVE_UP:
			case ID_APPT_MOVE_UP:
			case ID_APPT_IN:
			case ID_APPT_OUT:
			case ID_APPT_PENDING:
			case ID_APPT_RECEIVED:
			case ID_APPT_SHOW:
			case ID_GOTOPATIENT:
			case ID_RES_EDIT_TEMPLATES:
			case ID_RES_EDIT_TEMPLATE_COLLECTIONS: // (z.manning 2014-12-01 17:35) - PLID 64205
			case ID_RES_EDIT_LOCATION_TEMPLATES:	// (j.jones 2011-07-15 14:48) - PLID 39838
			case ID_APPT_NEW_INV_ALLOCATION:
			case ID_APPT_NEW_INV_ORDER:		// (j.jones 2008-03-18 14:25) - PLID 29309
			case ID_APPT_NEW_BILL:			// (j.jones 2008-03-18 14:25) - PLID 30455
			case ID_APPT_EDIT_INV_ALLOCATION: // (j.gruber 2008-09-10 15:51) - PLID 30282
			case ID_APPT_CREATE_CASE_HISTORY: // (j.jones 2009-08-28 12:55) - PLID 35381 - added case history options
			case ID_APPT_EDIT_CASE_HISTORY:
			case ID_APPT_NEW_PRIMARY_COPAY:	// (j.jones 2010-09-24 11:48) - PLID 34518 - added ability to create a copay
			case ID_APPT_VIEW_ELIGIBILITY_RESPONSES:	// (j.jones 2010-09-27 11:21) - PLID 38447 - added ability to view eligibility requests/responses
			case ID_APPT_MANAGE_PAYMENT_PROFILES:
				{
					// (a.walling 2007-08-31 12:36) - PLID 27265 - Now clear out our smart pointer, which releases our reference,
					// and set m_pResDispatch to -1.
					m_pResDispatchPtr.SetDispatch(NULL);
					m_pResDispatch = (LPDISPATCH)-1;
				}
				break;
			default: // handle the standard and user defined show state commands too
			if (LOWORD(wParam) > ID_APPT_RECEIVED) {
				long nID = LOWORD(wParam) - ID_APPT_PENDING;
				for (long i=0; i < m_adwShowState.GetSize(); i++)
				{
					if ((long)m_adwShowState[i] == nID)
					{
						// (a.walling 2007-08-31 15:03) - PLID 27265 - Now clear out our smart pointer, which releases our reference,
						// and set m_pResDispatch to -1.
						m_pResDispatchPtr.SetDispatch(NULL);
						m_pResDispatch = (LPDISPATCH)-1;
						break;
					}
				}
				break;
			}		
		}

	} NxCatchAll("Error releasing stored Reservation in CSchedulerView::OnCommand");

	return bBaseResult;
}

void CSchedulerView::ResetTrackedAppt()
{
	m_dtActiveApptLastModified.m_status = COleDateTime::invalid;
	m_nTrackingResID = -1;
}

void CSchedulerView::TrackLastModifiedDate(long nResID)
{
	try
	{
		// (c.haag 2006-08-04 16:37) - PLID 21801 - We always have to get the modified date of an appointment
		//if (m_nTrackingResID == nResID)
		//	return;
		// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
		_RecordsetPtr prs = CreateParamRecordset("SELECT ModifiedDate FROM AppointmentsT WHERE ID = {INT}", nResID);
		if (prs->eof) {
			//DRT 10/7/2008 - PLID 31350 - We got an error in CNxSchedulerDlg::AppointmentGrabWithValidation, that
			//	as best I can tell, can only happen if this condition fails.  But this condition should never fail,
			//	becaue we're dragging the appointment -- that would mean that while our mouse was down dragging an 
			//	appt (or between refreshes if disconnected), someone physically deleted the appointment from the
			//	database.  This is very unlikely.  So I'm throwing an ASSERT here, as this case should really never
			//	be hit, though it is possible in that case.  If this gets hit, then CNxSchedulerDlg::AppointmentGrabWithValidation
			//	may fail.  If this assessment is true, we need to modify the AppointmentGrab function to do something
			//	specific in that case.  I'm not going to guess at a solution now until we know what causes it.
			ASSERT(FALSE);
			m_dtActiveApptLastModified.m_status = COleDateTime::invalid;
		}
		else
			m_dtActiveApptLastModified = AdoFldDateTime(prs, "ModifiedDate");
		m_nTrackingResID = nResID;
		return;
	}
	NxCatchAll("Error tracking appointment modified date");
	ResetTrackedAppt();
}

void CSchedulerView::TrackLastModifiedDate(long nResID, COleDateTime dtModifiedDate)
{
	if (m_nTrackingResID == nResID)
		return;
	m_dtActiveApptLastModified = dtModifiedDate;
	m_nTrackingResID = nResID;
}

BOOL CSchedulerView::WasTrackedApptModified(BOOL bReset, OUT CString& strModifiedLogin)
{
	BOOL bRes = FALSE;
	if (m_dtActiveApptLastModified.m_status == COleDateTime::valid)
	{
		try
		{
			// (d.thompson 2010-05-06) - PLID 38525 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset("SELECT ModifiedDate, ModifiedLogin FROM AppointmentsT WHERE ID = {INT}", m_nTrackingResID);
			if (!prs->eof)
			{
				bRes = (m_dtActiveApptLastModified < AdoFldDateTime(prs, "ModifiedDate"));
				if (bRes)
					strModifiedLogin = AdoFldString(prs, "ModifiedLogin", "");
			}
		}
		NxCatchAll("Error affirming appointment modified date");
	}
	if (bReset)
	{
		ResetTrackedAppt();
	}
	return bRes;
}

void CSchedulerView::ShowMoveUpList(const COleDateTime& dtStart, const CDWordArray& adwResourceFilter)
{	
	//TES 12/17/2008 - PLID 32478 - This ability is only available in the Enterprise edition, so check the license.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Designating appointments as requesting a move up", "The_Scheduler_Module/manage_my_moveup_appointments.htm")) {
		return;
	}

	m_MoveUpDlg.EnableRequery(FALSE);
	OnShowMoveUpList();
	m_MoveUpDlg.EnableRequery(TRUE);
	if (m_MoveUpDlg.GetSafeHwnd())
	{
		m_MoveUpDlg.SetMoveUpDateTime(dtStart);
		m_MoveUpDlg.SetResourceFilter(adwResourceFilter);
		m_MoveUpDlg.SetDateRangeFilter(dtStart, dtStart);
		m_MoveUpDlg.Requery();
	}
}

void CSchedulerView::OnCreateNewAppointment()
{
	CNxSchedulerDlg *pActiveSchedSheet = ((CNxSchedulerDlg *)m_pActiveSheet);
	if (pActiveSchedSheet) {
		try {
			// Start the wait cursor
			CWaitCursor wc;
			// Get a variable for the correct singleday control
			CSingleDay psd = pActiveSchedSheet->m_pSingleDayCtrl;
			// Get the time for the appointment
			long nTopSlot = psd.GetTopSlot();
			CString strStartTimeString = (LPCTSTR)psd.SlotTime(nTopSlot);
			COleDateTime dt;
			if (!dt.ParseDateTime(strStartTimeString)) {
				ThrowNxException(
					"CSchedulerView::OnCreateNewAppointment: The singleday for this tab has "
					"returned an invalid time string '%s' for the top slot %li", strStartTimeString, nTopSlot);
			}
			// Finally create the appointment in that slot, with a duration of exactly one slot (our interval).  (It's 
			// kind of handy that the singleday automatically fires the ReservationAdded event for us, so that we don't 
			// have to do any of the resentry stuff here ourselves.)
			psd.AddReservation(__FUNCTION__, 0, dt, dt + COleDateTimeSpan(0,0,m_nInterval,0), 0, VARIANT_FALSE);
		} NxCatchAll("CSchedulerView::OnCreateNewAppointment");
	}
}

BOOL CSchedulerView::CheckPermissions()
{
	//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
	if(!g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrUse)) {
		return FALSE;
	}
	if(!UserPermission(SchedulerModuleItem)) {
		return FALSE;
	}
	return TRUE;
}

void CSchedulerView::OnSelectTab(short newTab, short oldTab)
{
	if(newTab != oldTab) {
		if(!((CNxSchedulerDlg*)GetActiveSheet())->AllowChangeView()) {
			m_tab->CurSel = oldTab;
			return;
		}
	}
	CNxTabView::OnSelectTab(newTab, oldTab);
}

LRESULT CSchedulerView::OnHideResentry(WPARAM wParam, LPARAM lParam)
{
	if(m_ResEntry && m_ResEntry->IsWindowVisible()) {
		m_ResEntry->SendMessage(WM_COMMAND, IDCANCEL);
		if (m_ResEntry->IsWindowVisible()) {
			// (c.haag 2005-11-01 8:35) - PLID 17979 - If the window is still visible,
			// return true.
			return TRUE;
		}
	}
	return 0;
}

void CSchedulerView::SetDefaultControlFocus()
{
}

void CSchedulerView::ShowPatientWarning(long nPatientID,
		BOOL bSuppressPtWarning /*= FALSE */,
		BOOL bSuppressInsReferralWarning /*= FALSE */,
		BOOL bSuppressCoPayWarning /*= FALSE */,
		BOOL bSuppressAllergyWarning /*= FALSE */,
		BOOL bSuppressEMRProblemWarning /*= FALSE */,
		BOOL bSuppressRewardsWarning /*= FALSE */) // (j.luckoski 2013-03-04 13:58) - PLID 33548
{
	// (c.haag 2011-06-23) - PLID 44287 - Moved the code into GlobalSchedUtils.cpp
	ShowSchedulerPatientWarning(this,
		nPatientID, 
		bSuppressPtWarning, 
		bSuppressInsReferralWarning, 
		bSuppressCoPayWarning, 
		bSuppressAllergyWarning,
		bSuppressEMRProblemWarning,
		bSuppressRewardsWarning); // (j.luckoski 2013-03-04 13:58) - PLID 33548
}

// (j.jones 2007-11-21 14:20) - PLID 28147 - added ability to create a new inventory allocation
void CSchedulerView::OnResNewInvAllocation()
{
	try {

		//ensure we have a valid m_pResDispatch pointer
		if (!IS_VALID_DISPATCH(m_pResDispatch)) {
			return;
		}

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		if(pRes == NULL) {
			return;
		}

		// (c.haag 2011-06-23) - PLID 44287 - The act of creating the allocation is now done in a utility function
		AppointmentCreateInvAllocation(this, pRes.GetReservationID(), TRUE);

	} NxCatchAll("Error in CSchedulerView::OnResNewInvAllocation");
}

// (j.gruber 2008-09-10 15:52) - PLID 30282 - ability to edit inventory allocation
void CSchedulerView::OnResEditInvAllocation()
{
	try {

		//ensure we have a valid m_pResDispatch pointer
		if (!IS_VALID_DISPATCH(m_pResDispatch)) {
			return;
		}

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		if(pRes == NULL) {
			return;
		}

		// (c.haag 2011-06-23) - PLID 44287 - The act of editing the allocation is now done in a utility function
		AppointmentEditInvAllocation(this, pRes.GetReservationID(), TRUE);

	} NxCatchAll("Error in CSchedulerView::OnResEditInvAllocation");
}

// (j.jones 2008-03-18 14:21) - PLID 29309 - added ability to create a new inventory order
void CSchedulerView::OnResNewInvOrder()
{
	try {

		//ensure we have a valid m_pResDispatch pointer
		if (!IS_VALID_DISPATCH(m_pResDispatch)) {
			return;
		}

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		if(pRes == NULL) {
			return;
		}

		// (c.haag 2011-06-23) - PLID 44287 - This is now done in a utility function
		AppointmentCreateInvOrder(this, pRes.GetReservationID(), TRUE);

	} NxCatchAll("Error in CSchedulerView::OnResNewInvOrder");
}


// (j.jones 2008-06-23 16:16) - PLID 30455 - added ability to create a bill
void CSchedulerView::OnResNewBill()
{
	try {

		//ensure we have a valid m_pResDispatch pointer
		if (!IS_VALID_DISPATCH(m_pResDispatch)) {
			return;
		}

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		if(pRes == NULL) {
			return;
		}

		long nResID = pRes.GetReservationID();
		long nPatientID = AppointmentGetPatientID(nResID);

		// (c.haag 2011-06-23) - PLID 44287 - This is now done in a utility function
		AppointmentCreateNewBill(this, nResID, TRUE, nPatientID);

	} NxCatchAll("Error in CSchedulerView::OnResNewBill");
}

// (j.jones 2009-08-28 12:55) - PLID 35381 - added case history options
void CSchedulerView::OnResCreateCaseHistory()
{
	try {

		//ensure we have a valid m_pResDispatch pointer
		if (!IS_VALID_DISPATCH(m_pResDispatch)) {
			return;
		}

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		if(pRes == NULL) {
			return;
		}

		long nResID = pRes.GetReservationID();
		long nPatientID = AppointmentGetPatientID(nResID);

		// (c.haag 2011-06-23) - PLID 44319 - The case history is now created from a utility function
		AppointmentCreateCaseHistory(nResID, nPatientID);
	}
	NxCatchAll(__FUNCTION__);
}

void CSchedulerView::OnResEditCaseHistory()
{
	try {

		//ensure we have a valid m_pResDispatch pointer
		if (!IS_VALID_DISPATCH(m_pResDispatch)) {
			return;
		}

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		if(pRes == NULL) {
			return;
		}

		long nResID = pRes.GetReservationID();
		long nPatientID = AppointmentGetPatientID(nResID);

		// (c.haag 2011-06-23) - PLID 44319 - The case history is now edited from a utility function
		AppointmentEditCaseHistory(nResID, nPatientID);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-09-27 11:40) - PLID 38447 - added ability to view eligibility responses
void CSchedulerView::OnResViewEligibilityResponses()
{
	try {

		//ensure we have a valid m_pResDispatch pointer
		if (!IS_VALID_DISPATCH(m_pResDispatch)) {
			return;
		}

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		if(pRes == NULL) {
			return;
		}

		long nResID = pRes.GetReservationID();
		long nPatientID = AppointmentGetPatientID(nResID);

		if(nPatientID == -25) {
			//do not give a warning
			return;
		}

		ShowAllEligibilityRequestsForInsuredParty_ByPatientOrAppt(NULL, nPatientID, nResID);

	} NxCatchAll(__FUNCTION__);
}

// (j.luckoski 2012-05-02 16:16) - PLID 11597 - OnRestoreAppt to restore cancelled appts.
void CSchedulerView::OnRestoreAppt()
{
	try {
		if (!CheckCurrentUserPermissions(bioAppointment, SPT________2__))
			return;
		BeginWaitCursor();
		BOOL bIsDirty = IsSchedulerDirty();
		// (a.walling 2007-08-31 10:29) - PLID 27265 - Ensure valid m_pResDispatch pointer
		if (IS_VALID_DISPATCH(m_pResDispatch)) {
			long nDelResID;
			CReservation delRes(__FUNCTION__, m_pResDispatch);
			// Get the CReservation's id
			nDelResID = delRes.GetReservationID();
			long nPatID = AppointmentGetPatientID(nDelResID);
			ShowPatientWarning(nPatID);

			// (j.jones 2014-12-02 11:36) - PLID 64183 - added pParentWnd
			if (AppointmentUncancel(this, nDelResID)) {
				// (j.luckoski 2012-04-30 09:15) - PLID 11597 - Don't delete and then redraw if preference is checked, just let 
				// updateView remove the cancelled appt from the list itself.
				// delRes.DeleteRes(__FUNCTION__);
			
				// (j.dinatale 2010-09-01) - PLID 39686 - Only if we cancel an appointment do we want to update our count labels.
				//		UpdateView does not update the view if we are using the right click context menu, so its best to just do it here.
				dynamic_cast<CNxSchedulerDlg *>( GetActiveSheet())->EnsureCountLabelText();
			}

			m_pResDispatchPtr.SetDispatch(NULL);
			m_pResDispatch = (LPDISPATCH)-1;

			// Check if it was dirty before we tried to delete
			if (bIsDirty) {
				// If it was dirty, just refresh
				// (z.manning 2010-10-14 10:10) - PLID 40929 - Post a message instead of calling UpdateView directly
				PostMessage(NXM_UPDATEVIEW);
			} else {
				SchedulerClean();
			}
		}
	} NxCatchAll("Error in CSchedulerView::OnRestoreAppt"); // (a.walling 2007-08-31 10:06) - PLID 27265
	EndWaitCursor();
}

// (a.walling 2015-01-12 10:28) - PLID 64558 - Rescheduling Queue - Dock and embed
void CSchedulerView::ShowReschedulingQueueEmbedded(bool bShow, long nApptID)
{
	try {
		if (bShow) {
			if (GetActiveTab() != SchedulerModule::DayTab) {
				SetActiveTab(SchedulerModule::DayTab);
			}
		}

		if (m_OneDaySheet) {	
			// (a.walling 2015-01-22 16:08) - PLID 64662 - Rescheduling Queue - selecting specific appointments		
			m_OneDaySheet.ShowReschedulingQueueEmbedded(bShow, nApptID);
		}
	} NxCatchAll(__FUNCTION__);
}

//TES 2/11/2015 - PLID 64120 - Used when first creating a mix rule, to update whatever scheduler tab might already be open
void CSchedulerView::ShowMixRulesButton()
{
	if (m_pActiveSheet) {
		((CNxSchedulerDlg*)m_pActiveSheet)->ShowMixRulesButton();
	}
}

// (z.manning 2015-07-22 15:18) - PLID 67241
void CSchedulerView::OnManagePaymentProfiles()
{
	try
	{
		if (!IS_VALID_DISPATCH(m_pResDispatch)) {
			return;
		}

		CReservation pRes(__FUNCTION__, m_pResDispatch);
		if (pRes == NULL) {
			return;
		}

		long nResID = pRes.GetReservationID();
		long nPatientID = AppointmentGetPatientID(nResID);

		OpenPaymentProfileDlg(nPatientID, this);
	} 
	NxCatchAll(__FUNCTION__);
}


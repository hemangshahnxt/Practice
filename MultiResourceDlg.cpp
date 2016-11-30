// MultiResourceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NxSchedulerDlg.h"
#include "MultiResourceDlg.h"
#include "ReservationReadSet.h"
#include "globaldatautils.h"
#include "SchedulerRc.h"
#include "ResEntryDlg.h"
#include "SchedCountSetupDlg.h"

#include "DateTimeUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "GlobalDataUtils.h"
#include "InternationalUtils.h"
using namespace ADODB;
// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace SINGLEDAYLib;

extern CPracticeApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMultiResourceDlg dialog


CMultiResourceDlg::CMultiResourceDlg(CWnd* pParent /*=NULL*/)
	: CNxSchedulerDlg(CMultiResourceDlg::IDD, pParent)
{
	DAY_LABEL_ARRAY(FILL_ARRAY_CMultiResourceDlg__DayLabels)

	m_dtLastActiveDate = 0.0;

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Scheduler_Module/schedule_an_appointment.htm";
}


void CMultiResourceDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxSchedulerDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiResourceDlg)
	DAY_LABEL_ARRAY(DDX_CMultiResourceDlg__DayLabels)
	DDX_Control(pDX, IDC_ACTIVE_DATE_LABEL, m_nxstaticActiveDateLabel);
	DDX_Control(pDX, IDC_CURRENT_VIEW_NAME, m_nxstaticCurrentViewName);
	DDX_Control(pDX, IDC_ACTIVE_DATE_APTCOUNT_LABEL, m_nxstaticActiveDateAptcountLabel);
	DDX_Control(pDX, IDC_ACTIVE_RESOURCES_APTCOUNT_LABEL, m_nxlabelActiveResourcesAptcountLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiResourceDlg, CNxSchedulerDlg)
	//{{AFX_MSG_MAP(CMultiResourceDlg)
	// (b.cardillo 2016-06-06 16:20) - NX-100773 - Moved event click handling to base class
	// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiResourceDlg message handlers

bool CMultiResourceDlg::ReFilter(CReservationReadSet &rsReadSet, bool bForceOpen /* = false */)
{
	// Build the query
	// (b.cardillo 2003-06-26 17:02) - I THINK we're sorting here in an effort to provide a 
	// consistent left-to-right order when appointments are double-booked.
	// (c.haag 2006-12-12 16:32) - PLID 23845 - I wrote an easier-to-maintain implementation
	// that supports more fields so that we can properly suppress template blocks in the face
	// of appointments
	// (z.manning, 05/24/2007) - PLID 26122 - Removed references to EnablePrecisionTemplating preference.
	// (c.haag 2008-11-20 16:32) - PLID 31128 - Insurance referrals
	// (z.manning 2009-06-30 16:40) - PLID 34744 - We only load insurance referrals if necessary now.
	//TES 1/14/2010 - PLID 36762 - We now need to load an additional recordset for the security groups that patients in this list are 
	// members of; that way CReservationReadSet can hide their demographic information.
	// (j.jones 2011-02-11 09:31) - PLID 35180 - ResExtendedQ is now a function that takes in:
	//@ExcludeCanceledAppointments BIT, @FromDate DATETIME, @ToDate DATETIME, @ResourceIDs NTEXT
	//ResourceIDs must be comma-delimited.
	// (j.gruber 2011-10-14 12:04) - PLID 45348 - calculate our own resources

	// (a.walling 2013-06-18 11:31) - PLID 57204 - Use new ResExtendedQ alternative in scheduler views for significantly better scheduler performance
	//TES 8/7/2013 - PLID 57921 - Restored the resource to the ORDER BY clause. This turns out to be crucial, because 
	// GetMultiPurposeStringFromCurrentReservationReadSetPosition() will be assuming that all the records composing a single on-screen appointment block
	// will be consecutive, so when not sorting by resource, (meaning those records were divided up by purpose), the resource view was showing
	// multiple blocks for every appointment with multiple purposes.

	// (j.jones 2014-12-03 14:53) - PLID 64275 - if the appointment has overridden a mix rule, show it first
	CSqlFragment query(
		"SET NOCOUNT ON; \r\n"
		"DECLARE @dateFrom DATETIME; DECLARE @dateTo DATETIME; \r\n"
		"SET @dateFrom = {OLEDATETIME}; SET @dateTo = DATEADD(d, 1, @dateFrom); \r\n"
		// (a.walling 2013-08-05 17:32) - PLID 57869 - Force seeks via table variables for scheduler refresh
		// (a.walling 2015-04-27 09:55) - PLID 65746 - Force StartTime_Cluster index so SQL doesn't try to scan the ID index for the whole table to get ordered ID for the primary key
		"DECLARE @apptIDs TABLE (ID INTEGER NOT NULL PRIMARY KEY); \r\n"
		"INSERT INTO @ApptIDs SELECT ID FROM AppointmentsT WITH(INDEX(StartTime_Cluster)) WHERE StartTime >= @dateFrom AND StartTime < @dateTo; \r\n"
		"{SQL} \r\n" // base query
		"WHERE AppointmentsT.ID IN (SELECT ID FROM @apptIDs) \r\n"
		"AND AppointmentResourceT.ResourceID IN ({CONST_STR}) \r\n"
		"{CONST_STR} \r\n" // extra filters
		"ORDER BY IsMixRuleOverridden DESC, AppointmentsT.StartTime, AppointmentsT.ID, ResourceT.Item, AptPurposeT.Name; \r\n"
		"\r\n"
		"SELECT DISTINCT SecurityGroupDetailsT.PatientID, SecurityGroupDetailsT.SecurityGroupID \r\n"
		"FROM SecurityGroupDetailsT "
		"INNER JOIN AppointmentsT \r\n"
		"ON SecurityGroupDetailsT.PatientID = AppointmentsT.PatientID \r\n"
		"WHERE AppointmentsT.StartTime >= @dateFrom AND AppointmentsT.StartTime < @dateTo \r\n"
		"ORDER BY SecurityGroupDetailsT.PatientID, SecurityGroupDetailsT.SecurityGroupID; "
		"SET NOCOUNT OFF; \r\n"
		, AsDateNoTime(GetActiveDate())
		, Nx::Scheduler::GetResExtendedBase(m_pParent, GetActiveDate(), GetActiveDate())
		, SortAndRemoveDuplicates(CalcCurrentResourceIDs()) // (a.walling 2013-06-20 13:49) - PLID 57204 - Handles space and comma separated values
		, GetExtraFilter()
	);

	CNxPerform nxp("ReFilter " __FUNCTION__);
	return rsReadSet.ReFilter(query);
}

// (j.gruber 2011-10-14 12:05) - PLID 45348 - this is just like our parent, except we account for hidden resources
CString CMultiResourceDlg::CalcCurrentResourceIDs()
{
	CString strAns;	

	long nCount = m_pParent->GetCurrentResourceList().GetSize();
	for (long i=0; i<nCount; i++) {
		//is it in our hidden list?
		long nResourceID = m_pParent->GetCurrentResourceList().GetAt(i)->m_nResourceID;
		long nBlank;
		if (!m_mapHiddenResources.Lookup(nResourceID, nBlank)) {
			//nope, we can add it
			CString str;
			str.Format("%li, ",nResourceID );
			strAns += str;
		}
	}

	DeleteEndingString(strAns, ", ");
	
	return strAns;


}

short CMultiResourceDlg::GetOffsetDay(CReservationReadSet &rsReadSet) 
{
	// This function is sort of like the opposite of GetResourceIDFromDayLabelIndex().  
	// The KEY difference is that it has nothing to do with the day labels (i.e. this is unaffected by 
	// the horizontal scroll position, while the day labels are set based on the left day)

	long nSearchingForResourceID = rsReadSet.GetResourceID();

	return GetOffsetDay(nSearchingForResourceID);
}

// (j.gruber 2011-10-25 12:46) - PLID 45348 - this accounts for hidden resources
short CMultiResourceDlg::GetOffsetDay(long nResourceID, BOOL bUseOriginal /*=FALSE*/) const
{
	if (m_mapHiddenResources.GetSize() == 0) {
		//if there are no hidden ones, we need to use the main list
		bUseOriginal = true;
	}

	if (bUseOriginal) {
		const CResourceEntryPtrArray &arypResourceList = m_pParent->GetCurrentResourceList();
		short nCount = arypResourceList.GetSize();
		for (int i=0; i<nCount; i++) {
			long nCurrentResourceID = arypResourceList.GetAt(i)->m_nResourceID;
			if (nCurrentResourceID == nResourceID) {
				return i;
			}
		}
	}
	else {
		//we are getting it taking hidden into account
		int nIndex = 0;
		const CResourceEntryPtrArray &arypResourceList = m_pParent->GetCurrentResourceList();
		short nCount = arypResourceList.GetSize();
		for (int i=0; i<nCount; i++) {
			long nCurrentResourceID = arypResourceList.GetAt(i)->m_nResourceID;

			if (nCurrentResourceID == nResourceID) {
				return nIndex;
			}

			//only increment if they are showing
			long nBlank;
			if (m_mapShowingResources.Lookup(nCurrentResourceID, nBlank)) {
				nIndex++;
			}
		}
	}

	
	//how did we not find it?
	ThrowNxException("Error in CMultiResourceDlg::GetOffsetDay: Could not find ResourceID");	
}


BEGIN_EVENTSINK_MAP(CMultiResourceDlg, CNxSchedulerDlg)
    //{{AFX_EVENTSINK_MAP(CMultiResourceDlg)
	ON_EVENT(CMultiResourceDlg, IDC_MULTI_RESOURCE_CTRL, -607 /* MouseUp */, CNxSchedulerDlg::OnMouseUpSingleday, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_RESOURCE_EVENT_CTRL, -607 /* MouseUp */, CNxSchedulerDlg::OnMouseUpEvent, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_MULTI_RESOURCE_CTRL, 4 /* ReservationRightClick */, CNxSchedulerDlg::OnReservationRightClick, VTS_DISPATCH VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_RESOURCE_EVENT_CTRL, 4 /* ReservationRightClick */, CNxSchedulerDlg::OnReservationRightClick, VTS_DISPATCH VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_MULTI_RESOURCE_CTRL, 1 /* ReservationClick */, CNxSchedulerDlg::OnReservationClick, VTS_DISPATCH)
	ON_EVENT(CMultiResourceDlg, IDC_RESOURCE_EVENT_CTRL, 1 /* ReservationClick */, CNxSchedulerDlg::OnReservationClick, VTS_DISPATCH)
	ON_EVENT(CMultiResourceDlg, IDC_MULTI_RESOURCE_CTRL, 10 /* ReservationEndDrag */, CNxSchedulerDlg::OnReservationEndDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_RESOURCE_EVENT_CTRL, 10 /* ReservationEndDrag */, CNxSchedulerDlg::OnReservationEndDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_MULTI_RESOURCE_CTRL, 2 /* ReservationAdded */, CNxSchedulerDlg::OnReservationAdded, VTS_DISPATCH)
	ON_EVENT(CMultiResourceDlg, IDC_RESOURCE_EVENT_CTRL, 2 /* ReservationAdded */, CNxSchedulerDlg::OnReservationAdded, VTS_DISPATCH)
	ON_EVENT(CMultiResourceDlg, IDC_MULTI_RESOURCE_CTRL, 12 /* ReservationEndResize */, CNxSchedulerDlg::OnReservationEndResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_RESOURCE_EVENT_CTRL, 12 /* ReservationEndResize */, CNxSchedulerDlg::OnReservationEndResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_MULTI_RESOURCE_CTRL, 13 /* HScroll */, OnHScrollMultiResourceCtrl, VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_RESOURCE_EVENT_CTRL, 13 /* HScroll */, OnHScrollMultiResourceEventCtrl, VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_MULTI_RESOURCE_CTRL, 6 /* ReservationMouseDown */, OnReservationMouseDown, VTS_DISPATCH VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_RESOURCE_EVENT_CTRL, 6 /* ReservationMouseDown */, OnReservationMouseDown, VTS_DISPATCH VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_MULTI_RESOURCE_CTRL, 9 /* ReservationDrag */, CNxSchedulerDlg::OnReservationDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_RESOURCE_EVENT_CTRL, 9 /* ReservationDrag */, CNxSchedulerDlg::OnReservationDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_MULTI_RESOURCE_CTRL, 11 /* ReservationResize */, CNxSchedulerDlg::OnReservationResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_RESOURCE_EVENT_CTRL, 11 /* ReservationResize */, CNxSchedulerDlg::OnReservationResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMultiResourceDlg, IDC_MULTI_RESOURCE_CTRL, 14 /* ReservationRecalculateColor */, CNxSchedulerDlg::OnReservationRecalculateColor, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CMultiResourceDlg::OnHScrollMultiResourceCtrl(long nNewLeftDay) 
{
	try {
		// Scroll the event singleday by the same amount
		m_pEventCtrl.PutLeftDay(nNewLeftDay);

		EnsureButtonLabels();

		//TES 9/8/03: Invalidate the count labels.
		// (c.haag 2010-07-14 12:06) - PLID 39564 - The day labels were converted into static
		// controls a while ago. Invalidating doesn't work any more; we have to update the controls
		// in the current implementation.
		/*CRect rc, rcParent;
		long x,y;
		GetDlgItem(IDC_MULTI_RESOURCE_CTRL)->GetWindowRect(&rc);
		GetWindowRect(&rcParent);
		x = rc.left - rcParent.left;
		y = (rc.bottom - rcParent.top) + 5;
		rc.left = x;
		rc.top = y;
		InvalidateRect(rc);*/
		EnsureCountLabelText();

	} NxCatchAllCall("CMultiResourceDlg::OnHScrollMultiResourceCtrl", ClearAndDisable());
}

void CMultiResourceDlg::OnHScrollMultiResourceEventCtrl(long nNewLeftDay) 
{
	try {
		// Scroll the main singleday by the same amount
		m_pSingleDayCtrl.PutLeftDay(nNewLeftDay);

		EnsureButtonLabels();

		//TES 9/8/03: Invalidate the count labels.
		CRect rc, rcParent;
		long x,y;
		GetDlgItem(IDC_MULTI_RESOURCE_CTRL)->GetWindowRect(&rc);
		GetWindowRect(&rcParent);
		x = rc.left - rcParent.left;
		y = (rc.bottom - rcParent.top) + 5;
		rc.left = x;
		rc.top = y;
		InvalidateRect(rc);
	} NxCatchAllCall("CMultiResourceDlg::OnHScrollMultiResourceEventCtrl", ClearAndDisable());
}

int CMultiResourceDlg::SetControlPositions()
{
	CNxSchedulerDlg::SetControlPositions();

	ResizeDayLabels();
	// (a.walling 2008-12-24 11:17) - PLID 32560
	EnsureCountLabelText();

	// (a.walling 2009-02-04 10:14) - PLID 31956 - No longer used
	//SetRgnBg();//should fix background drawing and flickering issues
	return 1;
}

// (a.walling 2008-05-13 10:31) - PLID 27591 - Use new notify events
void CMultiResourceDlg::OnChangeDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (m_PreviousDate == GetActiveDate())
		m_bNeedUpdate = true;

	CNxSchedulerDlg::OnChangeDateSelCombo(pNMHDR, pResult);
}

// Button Label stuff
//
// CAH 12/5/01: At this point we cannot assume that the number of entries in the resource combo is equal
// to the number of days in the control. It is therefore possible to attempt to grab an entry in the combo
// that does not even exist.
void CMultiResourceDlg::EnsureButtonLabels()
{
	// Get the left day so we can account for horizontal scrolling in our loop below
	long nLeftDayOffset = m_pSingleDayCtrl.GetLeftDay();

	// (b.cardillo 2003-06-24 12:18) I have no idea why this would be necessary unless 
	// there's a bug in the singleday
	m_pSingleDayCtrl.PutLeftDay(nLeftDayOffset);

	// Ask the scheduler view what the user's current list of resources consists of
	const CResourceEntryPtrArray &arypre = m_pParent->GetCurrentResourceList();
	
	// If this asserts, it's means that the scheduler view got a new or changed list 
	// of resources and SOMEHOW WE WERE NOT TOLD that it happened, or it's possible we
	// were told but we failed to handle it properly.  And when I say "we" I mean "we 
	// the CMultiResourceDlg class".  Right now I can't see how this assert could 
	// happen because the scheduler view promises it will call the virtual 
	// Internal_ReflectCurrentResourceListOnGUI() function when the resource list is 
	// loaded or re-loaded and our implementation updates the singledays before calling 
	// EnsureButtonLabels.
	// (j.gruber 2011-10-14 11:49) - PLID 45348 - take hidden columns into account
	ASSERT(m_pSingleDayCtrl.GetDayTotalCount() == GetColumnCount());
	if (m_pSingleDayCtrl.GetDayTotalCount() != GetColumnCount()) {
		ThrowNxException("CMultiResourceDlg::EnsureButtonLabels: The total day count does not match the number of resources in the current list!");
	}

	// Here's the main functionality of this function, we loop through the official current 
	// list of resources, starting at the entry corresponding to the left day (to account 
	// for horizontal scrolling) and going through the number of visible days
	long nVisibleDayCount = m_pSingleDayCtrl.GetDayVisibleCount();
	// (c.haag 2010-11-16 13:49) - PLID 39444 - Track resource names in this map. We used to rely on the button labels, but now they can
	// display things different than just the resource name.
	m_mapButtonResourceNames.RemoveAll();
	for (long i=0; i<nVisibleDayCount; i++) {
		// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
		CNxLabel* pButton = m_pDayLabelBtn[i];
		
		// (a.walling 2010-06-23 09:41) - PLID 39263 - Also update the description
		// (j.gruber 2011-10-25 13:25) - PLID 45348 - Get the original offset from the resourceID
		short nOriginalOffset = GetOffsetDay(GetResourceIDFromOffset(((short)i+(short)nLeftDayOffset)), TRUE);
		CString strInfo = m_ResourceAvailLocations.GetDayDescription(i+nLeftDayOffset);

		pButton->SetToolTip(strInfo);

		if (strInfo.IsEmpty()) {
			pButton->ClearTextColorOverride();
		} else {
			pButton->SetTextColorOverride(RGB(0xC0, 0x00, 0x00));
		}

		// (c.haag 2010-11-16 13:49) - PLID 39444 - Also include any resource location abbreviations
		CString strResourceName = arypre.GetAt(nOriginalOffset)->m_strResourceName;
		// (j.luckoski 2012-05-14 14:57) - PLID 48556 - Add true for bShowTimes and bIsResourceView
		pButton->SetText(m_ResourceAvailLocations.GetDayLabel(i+nLeftDayOffset, strResourceName, true));
		// (c.haag 2010-11-16 13:49) - PLID 39444 - Track resource names in this map. We used to rely on the button labels, but now they can
		// display things different than just the resource name.
		// (j.gruber 2011-10-25 13:27) - PLID 45348 - for this we need our current one, I think
		m_mapButtonResourceNames[i+nLeftDayOffset] = strResourceName;
	}
}

void UndefaultButton(CWnd *pButton)
{
	// Get the current style
	UINT nBtnStyle = (UINT)GetWindowLong(pButton->m_hWnd, GWL_STYLE) & 0xff;
	// Set the style to the existing style minus the defpushbutton flag, redraw
	pButton->SendMessage(BM_SETSTYLE, nBtnStyle & ~BS_DEFPUSHBUTTON, MAKELONG(TRUE, TRUE));
}

void UnsetDefaultButton(CDialog *pThis, UINT nNewFocusNonBtnID)
{
	// Undefault the button that currently has focus
	{
		CWnd *pFocus = pThis->GetFocus();
		if (pFocus) {
			UndefaultButton(pFocus);
		}
	}
	// Check to see what the current default button is
	DWORD dwGotDefID = pThis->GetDefID();

	// Set the new focus and unset the current default button
	{
		pThis->GetDlgItem(nNewFocusNonBtnID)->SetFocus();
		pThis->SetDefID(nNewFocusNonBtnID);
	}

	// If there was a default button, we know we just unset it but sometimes the buttons don't get 
	// their styles changed properly so we have to explicitly send the message to change styles
	{
		if (HIWORD(dwGotDefID) == DC_HASDEFID) {
			CWnd *pDefault = pThis->GetDlgItem(LOWORD(dwGotDefID));
			if (pDefault) {
				UndefaultButton(pDefault);
			}
		}
	}
}

long CMultiResourceDlg::GetResourceIDFromDayLabelIndex(long nDayLabelIndex) const
{
	// (b.cardillo 2003-06-30 15:40) TODO: Condsider storing our own array that is tightly bound to the 
	// buttons themselves instead of using the official resource list to get the info.  The reason we 
	// should do that is because this function (GetResourceIDFromDayLabelIndex) is meant to be as closely 
	// bound to the user-interface as possible, and as such is only really called as a way of certifying
	// that the official resource list is CORRECT!  So if we use the official list ourselves, then we 
	// can't be double-checked properly.
	
	// Right now we're implementing this in a simple but not perfectly independent way, where we get the 
	// array out of the parent and get the nth entry out of it, the assumption being that our buttons are 
	// always perfectly in sync with that array; note: the given index is from the left, so we need to 
	// adjust for the singledays' current left day, to account for horizontal scrolling.

	// Get the official array
	const CResourceEntryPtrArray &arypResources = m_pParent->GetCurrentResourceList();
	
	// Get the true index of the right element in the official array (here's where we adjust for the 
	// current horizontal scroll position
	long nResourceArrayIndex = nDayLabelIndex + m_pSingleDayCtrl.GetLeftDay();
	// (j.gruber 2011-10-25 14:31) - PLID 45348 - we are always given the current, so get the original when necessary
	long nOriginalIndex = GetOffsetDay(GetResourceIDFromOffset((short)nResourceArrayIndex), TRUE);
	
	// Get that element out of the array
	ASSERT(nOriginalIndex >= 0 && nOriginalIndex < arypResources.GetSize());
	if (nOriginalIndex >= 0 && nOriginalIndex < arypResources.GetSize()) {
		const CResourceEntry *pre = arypResources.GetAt(nOriginalIndex);

		// If our day label index is within range (i.e. it's one of the actual labels on the screen) then 
		// we can check our array to see what its text is as a way of trying catching invalid cases.  
		// NOTE: This technique is obviously not fully robust, in cases where there is no label for the 
		// given resource, we just have to hope it's right!
		if (nDayLabelIndex >= 0 && nDayLabelIndex < m_pSingleDayCtrl.GetDayVisibleCount()) {
			// Get the resource name out of the day label so we can compare it to the official name
			// (c.haag 2010-11-16 13:49) - PLID 39444 - We used to rely on the button labels, but now they can
			// display things different than just the resource name. So, use m_mapButtonResourceNames instead.
			CString strDayLabelText;
			// (j.gruber 2011-10-25 14:31) - PLID 45348 - need to use the one passed in here
			m_mapButtonResourceNames.Lookup(nResourceArrayIndex, strDayLabelText);
			// We expect the day label text to be the same as the text known to the official array
			ASSERT(pre->m_strResourceName == strDayLabelText);
			if (pre->m_strResourceName == strDayLabelText) {
				// We have our expected resource, let the caller have it
				return pre->m_nResourceID;
			} else {
				// The name didn't match what we expected
				ThrowNxException(
					"CMultiResourceDlg::GetResourceIDFromDayLabelIndex(%li): Expected resource name '%s' does "
					"not match label name '%s' for resource id %li in column %li!", 
					nDayLabelIndex, pre->m_strResourceName, strDayLabelText, pre->m_nResourceID, nOriginalIndex);
			}
		} else {
			// No way to double-check, we'll just have to trust that it's right
			return pre->m_nResourceID;
		}
	} else {
		// The index was out of range
		ThrowNxException(
			"CMultiResourceDlg::GetResourceIDFromDayLabelIndex(%li): Calculated resource index '%li' was "
			"out of range of the physical resource list!", 
			nDayLabelIndex, nOriginalIndex);
	}
}

void CMultiResourceDlg::OnDayLabel(long nDayLabelIndex) 
{
	try {
		// Get the resource this label represents
		long nNewResourceID = GetResourceIDFromDayLabelIndex(nDayLabelIndex);		

		// (j.jones 2010-10-06 15:24) - PLID 40718 - rearranged the order here such
		// that we switch to the tab first, and change the resource second

		// Switch to that tab
		m_pParent->SwitchToTab(CSchedulerView::sttOneDayView);
		// Set the official current resource to that
		m_pParent->SetActiveResourceID(nNewResourceID, TRUE);
	} NxCatchAll("CMultiResourceDlg::OnDayLabel");
}

BOOL CMultiResourceDlg::OnInitDialog() 
{
	try {
		// Must set m_pSingleDayCtrl before CNxSchedulerDlg::OnInitDialog is called
		// (c.haag 2010-03-26 15:05) - PLID 37332 - Use SetDispatch
		m_pSingleDayCtrl.SetDispatch((LPDISPATCH)GetDlgItem(IDC_MULTI_RESOURCE_CTRL)->GetControlUnknown());
		m_pEventCtrl.SetDispatch((LPDISPATCH)GetDlgItem(IDC_RESOURCE_EVENT_CTRL)->GetControlUnknown());

		// The multi-resource view DOES NOT have a resource dropdown, so we need to set this to NULL so that 
		// the CNxSchedulerDlg base class will fail if it tries to use it (this also means that we need to 
		// override some virtual functions from the base class so we can do our own multi-resource implementation)
		m_dlResources = NULL;
		
		CNxSchedulerDlg::OnInitDialog();
		
		GetDlgItem(IDC_ACTIVE_DATE_LABEL)->SetFont(&theApp.m_multiresFont);
		GetDlgItem(IDC_ACTIVE_RESOURCES_APTCOUNT_LABEL)->SetFont(&theApp.m_multiresFont);
		GetDlgItem(IDC_CURRENT_VIEW_NAME)->SetFont(&theApp.m_smallFont);

		// (j.gruber 2011-10-14 11:59) - PLID 45348
		long nOriginalResourceCount = m_pParent->GetCurrentResourceList().GetSize();
		long nTotalResourceCount = GetColumnCount();
		m_pSingleDayCtrl.PutDayTotalCount(__FUNCTION__, nTotalResourceCount);
		m_pEventCtrl.PutDayTotalCount(__FUNCTION__, nTotalResourceCount);
		InvalidateDayCountLabels(); // (a.walling 2008-10-01 11:06) - PLID 31445 - Ensure the labels match the daycount
		m_bNeedUpdate = true;

		// (j.gruber 2011-10-26 13:50) - PLID 45348 - cache
		// (j.luckoski 2012-05-08 17:10) - PLID 50242 - Add all properties to bulk cache
		// (j.luckoski 2012-05-09 17:08) - PLID 50242 - Deleted duplicate property
		g_propManager.CachePropertiesInBulk("MultiResourceDlg", propNumber,
				"(Username = '<None>' OR Username = '%s' OR Username = '%s') AND ("
				" Name = 'HideResourceBasedonTemplatesMultiResource' 	"
				" OR Name = 'ResourceViewVisibleColumns' "
				" OR Name = 'ResShowResourceName' "
				" OR Name = 'ShowCancelledAppointment' "
				" OR Name = 'CancelledDateRange' "
				" OR Name = 'SchedCancelledApptColor' "
				")",
				_Q(GetCurrentUserName()), _Q(GetCurrentUserComputerName()) );

		// (j.luckoski 2012-05-07 11:39) - PLID 11597 - Grab preferences for cancelled appts
		m_nCancelledAppt = GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true);
		// (j.luckoski 2012-06-12 15:58) - PLID 11597 - Did not default to 24
		m_nDateRange = GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true);
		m_nCancelColor = GetRemotePropertyInt("SchedCancelledApptColor", RGB(192,192,192), 0, GetCurrentUserName(), true);

		m_nColumnCount = GetRemotePropertyInt("ResourceViewVisibleColumns", 5, 0, "<None>", true);

		if(m_nColumnCount > nTotalResourceCount) {
			m_nColumnCount = nTotalResourceCount;
		}
		
		m_pSingleDayCtrl.PutDayVisibleCount(m_nColumnCount);
		m_pEventCtrl.PutDayVisibleCount(m_nColumnCount);
		GetDlgItem(IDC_RESOURCE_EVENT_CTRL)->ShowScrollBar(SB_HORZ, FALSE);

		// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
		DAY_LABEL_ARRAY(INIT_CMultiResourceDlg__DayLabels);

		// (c.haag 2009-12-22 13:44) - PLID 28977 - The Appt count label is now a hyperlink label
		m_nxlabelActiveResourcesAptcountLabel.SetType(dtsHyperlink);
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

bool CMultiResourceDlg::NeedUpdate()
{
	return CNxSchedulerDlg::NeedUpdate() || (m_dtLastActiveDate != GetActiveDate());
}

void CMultiResourceDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// (j.luckoski 2012-05-07 11:40) - PLID 11597 - Grab preferences for cancelled appts on refresh
	m_nCancelledAppt = GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true);
	// (j.luckoski 2012-06-12 15:58) - PLID 11597 - Did not default to 24
	m_nDateRange = GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true);
	m_nCancelColor = GetRemotePropertyInt("SchedCancelledApptColor", RGB(192,192,192), 0, GetCurrentUserName(), true);
	//DRT 7/17/2008 - PLID 30765 - We cannot allow the screen to be refreshed
	//	if the ResEntryDlg is open.  Otherwise the pointer it maintains to
	//	the currently clicked CReservation can become disconnected, and we
	//	may inadvertantly try to access data we no longer have.
	// (c.haag 2010-06-23 10:10) - PLID 39210 - Don't update if a reservation is locked by
	// a right-click action or the reservation window is open.
	// (c.haag 2010-08-27 11:38) - PLID 40108 - Moved the logic into one function
	if (!IsSafeToUpdateView())
	{
		//It is visible, do not update!
	}
	else {
		//////////////////////////////////////////////////////////////////////////////////////////
		// Resize the singleday control to ensure the number of actual and visible days is valid

		// (j.gruber 2011-10-14 11:42) - PLID 45348 - load our hidden resources
		LoadHiddenResourceMap();

		if (GetLocationId() != -1 && m_mapHiddenResources.GetSize() != 0 && m_mapShowingResources.GetSize() == 0) {
			//there are no resources on this location
			m_mapHiddenResources.RemoveAll();
			m_mapShowingResources.RemoveAll();
			m_dlLocationFilter->TrySetSelByColumn(0, (long)-1);
			SetActiveLocationId(-1);
			MsgBox("There are no resources available for the filtered location.  The location filter will be reset.");			
		}

		long nColumns = GetColumnCount();

		
		long nVisibleColumns = m_pSingleDayCtrl.GetDayVisibleCount();
		
		m_nColumnCount = GetRemotePropertyInt("ResourceViewVisibleColumns", 5, 0, "<None>", true);

		if (m_pSingleDayCtrl.GetDayTotalCount() != nColumns || m_pSingleDayCtrl.GetDayVisibleCount() != min(m_nColumnCount, m_pSingleDayCtrl.GetDayTotalCount())
			|| m_pEventCtrl.GetDayTotalCount() != nColumns || m_pEventCtrl.GetDayVisibleCount() != min(m_nColumnCount, m_pEventCtrl.GetDayTotalCount()))
		{
			//TES 3/8/2011 - PLID 41519 - Note: we're not trying to retain the "highlighted" appointment here; if the day count changed,
			// then this is a complete refresh, and that highlight should in fact be cleared.
			m_pSingleDayCtrl.Clear(__FUNCTION__);
			m_pSingleDayCtrl.PutDayTotalCount(__FUNCTION__, nColumns);
			InvalidateDayCountLabels(); // (a.walling 2008-10-01 11:06) - PLID 31445 - Ensure the labels match the daycount

			m_pEventCtrl.Clear(__FUNCTION__);
			m_pEventCtrl.PutDayTotalCount(__FUNCTION__, nColumns);

			// (j.gruber 2011-10-14 11:59) - PLID 45348 - user our count
			long nTotalResourceCount = GetColumnCount();
			if(m_nColumnCount > nTotalResourceCount) m_nColumnCount = nTotalResourceCount;

			m_pSingleDayCtrl.PutDayVisibleCount(m_nColumnCount);
			m_pEventCtrl.PutDayVisibleCount(m_nColumnCount);
			GetDlgItem(IDC_RESOURCE_EVENT_CTRL)->ShowScrollBar(SB_HORZ, FALSE);
			
			ResizeDayLabels();
		}
	}
	

	//Continue to call the base class regardless, it has always obeyed this rule.
	CNxSchedulerDlg::UpdateView(bForceRefresh); // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
	m_dtLastActiveDate = GetActiveDate();
}

// (j.gruber 2011-10-14 11:44) - PLID 45348 - Use this to take hidden resources into account
long CMultiResourceDlg::GetColumnCount()
{
	return m_pParent->GetCurrentResourceList().GetSize() - m_mapHiddenResources.GetSize();
}


// (j.gruber 2011-10-14 09:39) - PLID 45348 - overload to handle resource availalbity templates
void CMultiResourceDlg::LoadHiddenResourceMap()
{
	long nCurrentLocationID = GetLocationId();	

	//remove everything that is in there
	m_mapHiddenResources.RemoveAll();
	m_mapShowingResources.RemoveAll();

	//if they don't have the preference set, ignore
	// (d.thompson 2012-08-01) - PLID 51898 - Changed default to Hide (1)
	if (GetRemotePropertyInt("HideResourceBasedonTemplatesMultiResource", 1, 0, GetCurrentUserName()) == 0) {
		return;
	}

	if (nCurrentLocationID == -1) {
		//we aren't filtering on location, so just return
		return;
	}
	else {

		// (d.singleton 2012-07-03 10:47) - PLID 47473 support multi locations selected
		CDWordArray dwaryLocationIDs;
		if(nCurrentLocationID == -2) {
			GetLocationFilterIDs(&dwaryLocationIDs);
		}
		else {
			dwaryLocationIDs.Add(nCurrentLocationID);
		}

		// (c.haag 2013-01-14) - PLID 60082 - We used to use #temp tables in the proceeding calculation.
		// Now we just use table variables to save the SQL server from creating temp tables and frequent recompiling.
		CString strResources = m_pParent->CalcCurrentResourceIDs();

		//TES 5/23/2013 - PLID 56832 - If the string is empty (most likely because the user doesn't have permission for any resources in the current view)
		// then the queries below will throw errors.  Ensure that the string is a valid IN clause
		if(strResources.IsEmpty()) {
			strResources = "-1";
		}

		/*CSqlFragment sqlFrag1(""
			" SET NOCOUNT ON \r\n "
			" DECLARE @Date datetime; \r\n "
			" DECLARE @ResourceIDs nVarchar(4000); \r\n "
			"  \r\n "
			"  \r\n "			
			" SET @Date = dbo.AsDateNoTime({OLEDATETIME}); \r\n "
			" SET @ResourceIDs = ''; \r\n ", GetActiveDate());*/
			
		// (c.haag 2013-01-14) - PLID 60082 - We no longer use #temp tables
		// (r.gonet 2015-03-13 13:52) - PLID 65273 - Added missing template collection columns to the temp table
		// that holds the output of TemplateHitAllP.
		CString strSql =
			" SET NOCOUNT ON \r\n "
			" DECLARE @Date datetime; \r\n "
			" DECLARE @ResourceIDs nVarchar(4000); \r\n "
			"  \r\n "
			"  \r\n "
			" SET @Date = dbo.AsDateNoTime({STRING}); \r\n "
			" SET @ResourceIDs = {STRING}; \r\n "
			" DECLARE @t1 TABLE (TemplateID INT NOT NULL, Name nvarchar(255), TemplateLineItemID INT, StartTime DATETIME, EndTime DATETIME,  \r\n "
			" Color INT, IsBlock INT, Priority INT, RuleID INT, AndDetails INT, AllAppts INT, OverrideLocationTemplating BIT, ObjectType INT,  \r\n "
			" ObjectID INT, ResourceID INT, AllResources INT, LocationID INT, CollectionID INT, CollectionName NVARCHAR(50))  \r\n "
			"  \r\n "
			" DECLARE @t2 TABLE (ResourceID INT PRIMARY KEY) \r\n "
			"  \r\n "
			" INSERT INTO @t2 (ResourceID) \r\n"
			" SELECT DISTINCT ResourceT.ID FROM ResourceT \r\n"
			" INNER JOIN dbo.ParseStringToTable(@ResourceIDs, ',') R ON R.Data = ResourceT.ID \r\n"
			"  \r\n "
			" INSERT INTO @t1  exec TemplateHitAllP @Date, @ResourceIDs, 0,0,1 \r\n "
			"  \r\n "
			" IF EXISTS (SELECT top 1 TemplateID FROM @t1) BEGIN \r\n "
			" 	IF EXISTS (SELECT top 1 TemplateID FROM @t1 WHERE AllResources = 1 AND LocationID IN ({INTARRAY})) \r\n "
			"	BEGIN \r\n "
			"		DELETE FROM @t2 \r\n"
			"	END\r\n"
			"	ELSE BEGIN \r\n"
			"		DELETE FROM @t2 WHERE ResourceID IN \r\n"
			" 		(SELECT ResourceID FROM @t1 WHERE LocationID IN ({INTARRAY}) GROUP BY ResourceID) \r\n"
			"	END\r\n"
			"  END \r\n"
			"  \r\n "
			" SET NOCOUNT OFF; \r\n "
			"  \r\n "
			" SELECT ResourceID FROM @t2  \r\n ";

		// (d.singleton 2012-07-03 15:59) - PLID 47473 param dat query
		// (a.walling 2014-04-21 14:47) - PLID 60474 - TemplateHitAllP - open in snapshot isolation
		_RecordsetPtr rsExcludedResources = CreateParamRecordset(GetRemoteDataSnapshot(), strSql, FormatDateTimeForSql(GetActiveDate()), strResources, dwaryLocationIDs, dwaryLocationIDs);

		while (!rsExcludedResources->eof) {
			long nResourceID = AdoFldLong(rsExcludedResources->Fields, "ResourceID");
			m_mapHiddenResources.SetAt(nResourceID, GetOffsetDay(nResourceID, TRUE));
			rsExcludedResources->MoveNext();
		}

		//load up a map of showing resources also and they original offsets
		const CResourceEntryPtrArray &arypResourceList = m_pParent->GetCurrentResourceList();
		short nCount = arypResourceList.GetSize();
		long nBlank;
		for (int i=0; i<nCount; i++) {
			long nCurrentResourceID = arypResourceList.GetAt(i)->m_nResourceID;
			if (! m_mapHiddenResources.Lookup(nCurrentResourceID, nBlank)) {
				m_mapShowingResources.SetAt(nCurrentResourceID, i);
			}
		}		
	}
}

// (j.gruber 2011-10-25 13:23) - PLID 45348
long CMultiResourceDlg::GetResourceIDFromOffset(short nOffset) const
{
	if (m_mapHiddenResources.GetSize() == 0) {

		//we have no hidden, so they all are showing, just loop until the offset and get it
		const CResourceEntryPtrArray &arypResourceList = m_pParent->GetCurrentResourceList();
		short nCount = arypResourceList.GetSize();	
		for (int i=0; i<nCount; i++) {			
			if (i == nOffset) {
				long nCurrentResourceID = arypResourceList.GetAt(i)->m_nResourceID;		
				return nCurrentResourceID;
			}
		}
	}

	const CResourceEntryPtrArray &arypResourceList = m_pParent->GetCurrentResourceList();
	short nCount = arypResourceList.GetSize();
	long nBlank;
	long nCurrentOffset = 0;
	for (int i=0; i<nCount; i++) {
		long nCurrentResourceID = arypResourceList.GetAt(i)->m_nResourceID;
		
		//we were given an offset relative to hidden resources, so is this resource showing
		if (m_mapShowingResources.Lookup(nCurrentResourceID, nBlank)) {
			if (nCurrentOffset == nOffset) {
				return nCurrentResourceID;
			}
			nCurrentOffset++;
		}		
	}
	ThrowNxException("Error in CMultiResourceDlg::GetResourceIDFromOffset: Could not find index");	
}

void CMultiResourceDlg::Print(CDC *pDC, CPrintInfo *pInfo)
{
	CString strHdr, strFooter;
	CFont hdrFont, ftrFont;
	CFont subhdrFont;
	CFont* pOldFont;
	CRect rectHeader, rectSubHeader, rectFooter;
	CString strLabel;
	CRect rectTemp;

	// Set up header								
	//TES 11/10/2003: The new rule is: the header is 5.8 percent of page height.  The subheader (that has 
	//the resource names) is 5 percent of page height, or as much as it takes to draw the longest resource
	//name, whichever is larger.  For the moment, we'll just set it to 5 percent.
	rectHeader.SetRect(0, 0, pInfo->m_rectDraw.Width(), (int)((0.058) * (double)pInfo->m_rectDraw.Height()));
	rectSubHeader.SetRect(0, (int)((0.058) * (double)pInfo->m_rectDraw.Height()), pInfo->m_rectDraw.Width(), (int)((0.108) * (double)pInfo->m_rectDraw.Height()));
	rectFooter.SetRect(0, (int)((0.984) * (double)pInfo->m_rectDraw.Height()), pInfo->m_rectDraw.Width(), pInfo->m_rectDraw.Height());
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&hdrFont, 180, "Arial", pDC);
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&subhdrFont, 120, "Arial", pDC);
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&ftrFont, 90, "Arial", pDC);
	pDC->SetTextColor(RGB(0, 0, 0));

	// Print header
	CString strApptType;
	long nSetId = GetPurposeSetId();
	if (nSetId != -1) {
		long lRow = m_dlTypeFilter->FindByColumn(0, _variant_t(nSetId), 0, TRUE);
		if (lRow >= 0)
		{
			strApptType = VarString(m_dlTypeFilter->GetValue(lRow, 1));
		}		
		strApptType.Insert(0, "'");
		strApptType += "' ";
	} else {
		strApptType = "All ";
	}
	pOldFont = pDC->SelectObject(&hdrFont);
	COleDateTime dtPrintDate = COleDateTime::GetCurrentTime();
	dtPrintDate.SetDateTime(dtPrintDate.GetYear(), dtPrintDate.GetMonth(), dtPrintDate.GetDay(), 0, 0, 0);
	strHdr.Format("%sAppointments on %s", strApptType, 
		FormatDateTimeForInterface(GetActiveDate(), "%A ") + FormatDateTimeForInterface(GetActiveDate(), NULL, dtoDate));
	pDC->DrawText(strHdr, rectHeader, DT_CENTER);

	// Print subheaders
	pDC->SelectObject(&subhdrFont);
	int timeButtonPrintWidth = (int)((double)rectHeader.Width() * TIME_BUTTON_PRINT_WIDTH_PERCENT);
	int subwidth = (rectHeader.Width() - timeButtonPrintWidth) / 
		m_pSingleDayCtrl.GetDayVisibleCount();

	// (c.haag 2003-09-02 11:23) - Determine if we should use a smaller font
	CSize size;
	for (int i=0; i < m_pSingleDayCtrl.GetDayVisibleCount(); i++) {
		strLabel = m_pDayLabelBtn[i]->GetText();
		size = pDC->GetOutputTextExtent(strLabel);
		if (size.cx > timeButtonPrintWidth + subwidth)
		{
			pDC->SelectObject(&ftrFont);
			size = pDC->GetOutputTextExtent(strLabel);
			break;
		}
	}

	// (c.haag 2003-09-02 12:04) - Get the max label box dimensions
	for (i=0; i < m_pSingleDayCtrl.GetDayVisibleCount(); i++) {
		RECT rc = CRect(0,rectSubHeader.top, subwidth, rectSubHeader.bottom);
		strLabel = m_pDayLabelBtn[i]->GetText();
		pDC->DrawText(strLabel, &rc, DT_CALCRECT | DT_CENTER | DT_WORDBREAK);
		if (rc.bottom > rectSubHeader.bottom)
			rectSubHeader.bottom = rc.bottom;
	}

	// (c.haag 2003-09-02 11:24) - Draw the labels based on our calculations of
	// the most efficient placement for the labels
	for (i=0; i < m_pSingleDayCtrl.GetDayVisibleCount(); i++) {
		RECT rc;
		strLabel = m_pDayLabelBtn[i]->GetText();
		rc.top = rectSubHeader.top;
		rc.bottom = rectSubHeader.bottom;
		rc.left = timeButtonPrintWidth + i * subwidth;
		rc.right = timeButtonPrintWidth + (i+1) * subwidth;
		pDC->DrawText(strLabel, &rc, DT_CENTER | DT_WORDBREAK);
	}
	pDC->SelectObject(pOldFont);

	/*
	// Print the control in the area below the header area
	pInfo->m_rectDraw.top = rectHeader.bottom;
	m_pSingleDayCtrl.Print((IUnknown *)pDC, (IUnknown *)((CPrintInfo *)pInfo));
	*/

	pInfo->m_rectDraw.top = rectSubHeader.bottom;
	//Now, we need to draw the word "Event," just to the left of where the event ctrl will print out.
	//How much space do we need to leave?
	//This is a complicated calculation, but basically we're getting the TimeButtonWidth property, and scaling it
	//to fit our screen.  This is more or less copied from singledayctl.ppp's Print() function.
	//We need to calculate once, with a 1.4 multiplied; this is the rect from the left to the beginning of the eventctrl.
	//Then we'll calculate it again, w/o the 1.4, this is the actual width of the time button, in which we will 
	//center our text.
	double nTimeButtonWidthTotal = m_pSingleDayCtrl.GetTimeButtonWidth() * 1.4;
	nTimeButtonWidthTotal = nTimeButtonWidthTotal * ((double)pInfo->m_rectDraw.Width() / ((double)m_pSingleDayCtrl.GetDayWidth() * (double)m_pSingleDayCtrl.GetDayVisibleCount() + nTimeButtonWidthTotal));
	//To get the second line here, substitute nTimeButtonWidthActual*1.4 for nTimeButtonWidthTotal in the above equation, then solve for nTimeButtonWidthActual.
	double nTimeButtonWidthActual = m_pSingleDayCtrl.GetTimeButtonWidth();
	nTimeButtonWidthActual = (1.4*nTimeButtonWidthActual * ((double)pInfo->m_rectDraw.Width() / ((double)m_pSingleDayCtrl.GetDayWidth() * (double)m_pSingleDayCtrl.GetDayVisibleCount() + nTimeButtonWidthActual*1.4))) / 1.4;

	//Get the time of the first appointment.
	long nFirstPrintSlot, nLastPrintSlot;
	GetPrintSlotRange(nFirstPrintSlot, nLastPrintSlot, pInfo);
	long nSlotCount = nLastPrintSlot - nFirstPrintSlot + 1;
	
	//As we calculate the size of the event and singleday, we have to include a .5% spacer between them.
	//Also, we want the height to be equal to the number of slots it would take to show a 30 minute appt, but always
	//at least 1.0 slots (e.g, if the interval is 60, it should be the size of a 60 minute appt.
	double dSlotNum = (double)30 / (double)GetActiveInterval();
	if(dSlotNum < 1.0) dSlotNum = 1.0;
	double nSpacer = (0.005) * (double)pInfo->m_rectDraw.Height();
	double nEventCtrlHeight = ((double)pInfo->m_rectDraw.Height() - nSpacer) * (dSlotNum / ((double)nSlotCount + dSlotNum));

	//OK, so we need two rectangles of the same height, one from 0-nTimeButtonWidth, one from nTimeButtonWidth on.
	CRect rectEventCtrl, rectEventCaption, rectPage;

	//The event area will be just below the header area, using the height calculated above..  
	rectEventCaption.SetRect(0, rectSubHeader.bottom, (int)nTimeButtonWidthActual, rectSubHeader.bottom + (int)nEventCtrlHeight);
	rectEventCtrl.SetRect((int)nTimeButtonWidthTotal, rectSubHeader.bottom, pInfo->m_rectDraw.Width(), rectSubHeader.bottom + (int)nEventCtrlHeight);
	//This font is the same as the singleday uses.  I know because I looked at the code.
	CFont *oldFont, newFont;
	LOGFONT lfNew;
	lfNew.lfHeight = 84;
	lfNew.lfWidth = 0;//Don't care.
	lfNew.lfEscapement = 0;
	lfNew.lfOrientation = 0;
	lfNew.lfWeight = FW_BOLD;
	lfNew.lfItalic = FALSE;
	lfNew.lfUnderline = FALSE;
	lfNew.lfStrikeOut = FALSE;
	lfNew.lfCharSet = DEFAULT_CHARSET;
	lfNew.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lfNew.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lfNew.lfQuality = DEFAULT_QUALITY;
	lfNew.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
	strcpy(lfNew.lfFaceName, "Arial");

	//TES 4/2/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFontIndirect(&newFont, &lfNew, pDC);
	oldFont = pDC->SelectObject(&newFont);
	pDC->DrawText("  Event", rectEventCaption, DT_SINGLELINE|DT_VCENTER);
	pDC->SelectObject(oldFont);

	//Now we want to pass our eventctrl rect as part of the pInfo
	rectPage = pInfo->m_rectDraw;
	rectPage.bottom = (long)((float)rectPage.bottom * 0.97f); // Make room for the footer
	pInfo->m_rectDraw = rectEventCtrl;
	
	m_pEventCtrl.Print((IUnknown *)pDC, (IUnknown *)((CPrintInfo *)pInfo));

	// Print the control in the area below the header area
	pInfo->m_rectDraw = rectPage;
	pInfo->m_rectDraw.top = rectEventCtrl.bottom + (int)nSpacer;
	m_pSingleDayCtrl.Print((IUnknown *)pDC, (IUnknown *)((CPrintInfo *)pInfo));

	// Print the footer
	strFooter.Format("Printed On: %s", FormatDateTimeForInterface(dtPrintDate, NULL, dtoDate));
	pDC->SelectObject(&ftrFont);
	pDC->DrawText(strFooter, rectFooter, DT_RIGHT);
	pDC->SelectObject(pOldFont);
}

void CMultiResourceDlg::ResizeDayLabels()
{
	long nDayWidth, nTimeWidth;
	CPoint ptTopLeft;
	CRect tmpRect, btnRect, rcEvent, rcSingleDay;
	
	// Resize event button and event control to match the horizontal dimensions of the weekday control //
	GetDlgItem(IDC_BTN_NEW_EVENT)->GetClientRect(tmpRect);
	GetDlgItem(IDC_BTN_NEW_EVENT)->SetWindowPos(NULL, 0,0, m_pSingleDayCtrl.GetTimeButtonWidth(), tmpRect.Height(), SWP_NOZORDER | SWP_NOMOVE);

	GetDlgItem(IDC_BTN_NEW_EVENT)->GetWindowRect(tmpRect);
	ScreenToClient(tmpRect);
	CWnd::FromHandle((HWND)m_pEventCtrl.GethWnd())->GetWindowRect(rcEvent);
	ScreenToClient(rcEvent);
	CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd())->GetWindowRect(rcSingleDay);
	ScreenToClient(rcSingleDay);
	CWnd::FromHandle((HWND)m_pEventCtrl.GethWnd())->SetWindowPos(NULL, tmpRect.right, rcEvent.top, rcSingleDay.Width() - m_pSingleDayCtrl.GetTimeButtonWidth() - GetSystemMetrics(SM_CXVSCROLL), rcEvent.Height(), SWP_NOZORDER);
	//

	GetWindowRect(tmpRect);
	ptTopLeft = tmpRect.TopLeft();
	
	// Calculate important dimensions
	CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd())->GetWindowRect(tmpRect);
	m_pDayLabelBtn[0]->GetWindowRect(btnRect);
	tmpRect -= ptTopLeft;
	btnRect -= ptTopLeft;
	nTimeWidth = m_pSingleDayCtrl.GetTimeButtonWidth();
	///////////////////////////////////////////////////////
	// This used to use "(tmpRect.Width()-nTimeWidth)/m_pSingleDayCtrl.GetDayVisibleCount()" instead 
	// of "m_pSingleDayCtrl.GetDayWidth()", because of an alleged bug in the SingleDay control, but I 
	// can't find any bug and when I tried it the right way I couldn't find a problem.  I'm worried 
	// that whoever put it the other way knew something I don't.  But for now this seems okay.
	nDayWidth = m_pSingleDayCtrl.GetDayWidth();
	///////////////////////////////////////////////////////

	/* Not moving resource ordering button for any sheets, not even the multi-resource sheet
	// Resize the resource ordering button
	btnRect.left = tmpRect.left + nTimeWidth / 2;
	btnRect.right = tmpRect.left + nTimeWidth - 1;
	GetDlgItem(IDC_RESOURCE_ORDER_BTN)->MoveWindow(btnRect);
	SetDlgItemText(IDC_RESOURCE_ORDER_BTN, "-->");
	*/
	
	// Resize the first button label
	btnRect.left = tmpRect.left + nTimeWidth;
	btnRect.right = btnRect.left + nDayWidth;
	m_pDayLabelBtn[0]->ShowWindow(SW_SHOW);
	m_pDayLabelBtn[0]->MoveWindow(btnRect);
	
	// Resize the rest of the button labels
	int i;
	for (i=1; i<m_pSingleDayCtrl.GetDayVisibleCount(); i++) {
		btnRect.left = btnRect.right + 1;
		btnRect.right = btnRect.left + nDayWidth - 1;
		m_pDayLabelBtn[i]->ShowWindow(SW_SHOW);
		m_pDayLabelBtn[i]->MoveWindow(btnRect);
	}

	//Now go through the rest and hide them.
	for(i = m_pSingleDayCtrl.GetDayVisibleCount(); i < MAX_COLUMNS; i++) {
		m_pDayLabelBtn[i]->ShowWindow(SW_HIDE);
	}

}

ADODB::_RecordsetPtr CMultiResourceDlg::GetSingleApptQuery(long nResID)
{
	// Variables for use in generating the query
	// (a.walling 2013-05-31 14:06) - PLID 56961 - GetExtraResSimpleFilter is parameterized and simplified
	CSqlFragment filter = GetExtraResSimpleFilter();

	// Build the query
	// (b.cardillo 2003-06-26 17:02) - I THINK we're sorting here in an effort to provide a 
	// consistent left-to-right order when appointments are double-booked.
	// (j.jones 2011-02-11 09:31) - PLID 35180 - ResExtendedQ is now a function that takes in:
	//@ExcludeCanceledAppointments BIT, @FromDate DATETIME, @ToDate DATETIME, @ResourceIDs NTEXT
	// (j.luckoski 2012-05-07 11:40) - PLID 11597 - Alter query to show cancelled appts when appropriate
	// (a.walling 2013-05-31 14:06) - PLID 56961 - Filter cancelled appointments if out of range (or entirely, depending on the preference)
	if(m_nCancelledAppt == 1) {
		filter += CSqlFragment(
			"  AND (ResExQ.CancelledDate IS NULL OR (ResExQ.CancelledDate >= DATEADD(hh, -{CONST}, ResExQ.StartTime))) "
			, m_nDateRange
		);
	} else {
		filter += CSqlFragment(
			" AND ResExQ.Status <> 4 "
		);
	}

	// (a.walling 2013-05-31 14:06) - PLID 56961 - Select from GetResSimpleSql aliased to ResExQ, and restrict the date range and resource(s)
	// (a.walling 2013-06-18 11:12) - PLID 57196 - Use snapshot isolation for the main scheduler CReservationReadSet queries
	// (j.jones 2014-12-04 15:31) - PLID 64119 - added PrimaryInsuredPartyID
	return CreateParamRecordset(GetRemoteDataSnapshot(), 
		"DECLARE @QueryDate DATETIME; \r\n"
		"SET @QueryDate = {OLEDATETIME}; \r\n"
		"SELECT Status, StartTime, Convert(DateTime,ResExQ.Date) AS Date, EndTime, LocationID, AptTypeID, AptPurpose, AptPurposeID, ResExQ.ResourceID, PatientID, "
		"PatientName, ShowState, LastLM, ModifiedDate, ModifiedLogin, ShowStateColor, '' AS ResourceString, "
		"CASE WHEN ResExQ.Status = 4 THEN {INT} ELSE ResExQ.Color END AS Color, ShowStateColor, "
		"ResExQ.Status, ResExQ.CancelledDate, " // (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
		"PrimaryAppointmentInsuredPartyQ.InsuredPartyID AS PrimaryInsuredPartyID "
		"FROM ({SQL}) AS ResExQ "
		"LEFT JOIN ("
		"	SELECT AppointmentID, InsuredPartyID FROM AppointmentInsuredPartyT "
		"	WHERE Placement = 1 "
		") AS PrimaryAppointmentInsuredPartyQ ON ResExQ.ID = PrimaryAppointmentInsuredPartyQ.AppointmentID "
		"WHERE ResExQ.ID = {INT} "
		"AND ResExQ.ResourceID IN ({INTSTRING}) "
		"AND ResExQ.StartTime >= @QueryDate AND ResExQ.StartTime < DATEADD(day, 1, @QueryDate) "
		"{SQL} "
		"ORDER BY AptPurpose"
		, AsDateNoTime(GetActiveDate()) 
		, m_nCancelColor
		, Nx::Scheduler::GetResSimpleSql()
		, nResID
		, m_pParent->CalcCurrentResourceIDs()
		, filter
	);
}

BOOL CMultiResourceDlg::SingleApptFitsSlot(long iDay, ADODB::_RecordsetPtr& prsAppt)
{
	try {		
		// See if there are any records in the given recordset, if not we just want to fail
		if (!prsAppt->bof || !prsAppt->eof) {

			// Get the resource and date of the given column (referenced by iDay)
			long lSlotResource;
			{
				CSchedViewColumnInfo vci;
				m_pParent->GetWorkingResourceAndDate(this, iDay, vci);
				lSlotResource = vci.GetResourceID();
			}

			// Move to the first record (we're probably there already anyway but just in case)
			if (!prsAppt->bof) {
				prsAppt->MoveFirst();
			}

			// Loop through all the records until we find one that fits
			FieldPtr fldResourceID = prsAppt->GetFields()->GetItem("ResourceID");
			while (!prsAppt->eof)
			{
				// See if this record has a resource id equal to the one whose column we're looking in
				if (lSlotResource == AdoFldLong(fldResourceID))
				{
					// Found it, move the recordset back to its beginning and return success
					prsAppt->MoveFirst();
					return TRUE;
				} else {
					// No match yet, keep searching
					prsAppt->MoveNext();
				}
			}
			// No match in the whole recordset, move it back to its beginning and return failure
			prsAppt->MoveFirst();
			return FALSE;
		} else {
			// No records, how can we succeed?
			return FALSE;
		}
	} NxCatchAllCall("Error in SingleApptFitsSlot", return FALSE);
}

void CMultiResourceDlg::Internal_GetWorkingResourceAndDateFromGUI(IN const long nColumnIndex, OUT long &nWorkingResourceID, OUT COleDateTime &dtWorkingDate) const
{
	// The resource view has multiple columns, each column corresponding to a single resource.  So we 
	// just have to decide which resource the given column refers to and then the hard part is done.  
	// For the date, we do it just like the one-day view since the multi-resource view is a one-date 
	// view.

	// To get the resource to whom the given column applies, we just have to get the resource based on 
	// the day label for the given column. Note: the day label offset needs to be adjusted to account
	// for the current horizontal scroll position.
	nWorkingResourceID = GetResourceIDFromDayLabelIndex(nColumnIndex - m_pSingleDayCtrl.GetLeftDay());

	// The date needs to be pulled off the screen
	{
		// We happen to know for certain that "GetValue" is a const function, but because it's an 
		// activex function there's no way for the compiler to know; hence we have to deceive the 
		// compiler into believing it can safely call it (which it can).
		//CDTPicker *pdtp = const_cast<CDTPicker *>(&m_DateSelCombo); 
		// Get the date off the screen
		// (a.walling 2008-05-13 10:31) - PLID 27591 - No longer necessary with CDateTimePicker
		dtWorkingDate = m_DateSelCombo.GetValue();
	}
}

BOOL CMultiResourceDlg::Internal_IsMultiResourceSheet() const
{
	return TRUE;
}

BOOL CMultiResourceDlg::Internal_IsMultiDateSheet() const
{
	return FALSE;
}

void CMultiResourceDlg::Internal_ReflectCurrentResourceListOnGUI()
{
	try {
		
		// (j.gruber 2011-10-26 09:34) - PLID 45348 - we change the resource list, so clear our maps
		m_mapHiddenResources.RemoveAll();
		m_mapShowingResources.RemoveAll();
		long nTotalResourceCount = GetColumnCount();
		m_pSingleDayCtrl.PutDayTotalCount(__FUNCTION__, nTotalResourceCount);
		InvalidateDayCountLabels(); // (a.walling 2008-10-01 11:06) - PLID 31445 - Ensure the labels match the daycount
		m_pEventCtrl.PutDayTotalCount(__FUNCTION__, nTotalResourceCount);
		GetDlgItem(IDC_RESOURCE_EVENT_CTRL)->ShowScrollBar(SB_HORZ, FALSE);

		EnsureButtonLabels();
		
		ResizeDayLabels();
		
		ClearAndDisable();
		InvalidateData();

	} catch (...) {
		// If there were any exceptions whatsoever, we need to clear and disable this view
		ClearAndDisable();
		// And re-throw the exception
		throw;
	}
}

void CMultiResourceDlg::Internal_ReflectActiveResourceOnGUI()
{
	// Nothing major to do here, since this view doesn't show any single resource, it shows all of them
	// NOTE: We still have to invalidate because this function is called sometimes just when the user 
	// wants to refresh the given resource's list of appointments
	InvalidateData();
}

void CMultiResourceDlg::EnsureCountLabelText()
{
	try{
		int nApptCount = 0;
		if (m_pSingleDayCtrl != NULL)
		{
			// (c.haag 2009-12-22 17:34) - PLID 28977 - Filter on appointment types and appts
			CSchedulerCountSettings s;
			for(int i = 0; i < m_pSingleDayCtrl.GetDayTotalCount(); i++) {
				CReservation pRes(__FUNCTION__);
				int nResIndex = 0;
				while(NULL != (pRes = m_pSingleDayCtrl.GetReservation(__FUNCTION__, i, nResIndex))) {
					// (j.dinatale 2010-10-21) - PLID 36744 - Dont count the appt if not yet saved, meaning it will have a ReservationID of -1
					// (c.haag 2007-02-01 11:48) -  PLID 23666 - Don't count template blocks
					// (c.haag 2009-12-22 17:34) - PLID 28977 - Factor in type and person ID
					// (j.luckoski 2012-05-08 16:17) - PLID 50235 - Only count uncancelled appts.
					// (j.luckoski 2012-05-09 16:14) - PLID 50264 - Add GetRemoteDataSnapshot() to ReturnsRecordsParam
					// (j.luckoski 2012-06-11 10:14) - PLID 50235 - Only run to DB once and not for every apt.
					if (pRes.GetReservationID() != -1 && -1 == pRes.GetTemplateItemID() && s.IsPatientAllowed(pRes.GetPersonID()) && s.IsAptTypeAllowed(pRes.GetAptTypeID())) {
						// (a.walling 2013-01-21 16:48) - PLID 54745 - Appointment count calculation in scheduler reverted to stop using recordsets and instead use Reservation objects' Data[] property map
						if (pRes.Data["CancelledDate"].vt != VT_DATE) {
							nApptCount++;
						}
					}
					nResIndex++;
				}
			}
		}
		CString strCountLabelText;
		strCountLabelText.Format("%i Appt(s).Ttl.", nApptCount);
		// (c.haag 2009-12-22 13:44) - PLID 28977 - Now a CNxLabel
		m_nxlabelActiveResourcesAptcountLabel.SetText(strCountLabelText);

		{
			// (a.walling 2008-10-01 11:03) - PLID 31445 - Taken from the OnPaint handler. Rather than
			// paint these ourselves, we just set the window text of a dynamically created CNxStatic.
			// We will create them if necessary.
			// Now draw the appointments on a per-day basis
			CRect rc, rcParent;
			long nTimeButtonWidth = m_pSingleDayCtrl.GetTimeButtonWidth();
			long nWidth = m_pSingleDayCtrl.GetDayWidth();
			long nDays = m_pSingleDayCtrl.GetDayVisibleCount();
			long i, x, y;

			// (a.walling 2008-12-24 11:18) - PLID 32560 - Needs to be client coordinates
			GetDlgItem(IDC_MULTI_RESOURCE_CTRL)->GetWindowRect(&rc);
			ScreenToClient(rc);
			GetClientRect(&rcParent);
			y = (rc.bottom - rcParent.top) + 5;

			// (c.haag 2009-12-22 17:34) - PLID 28977 - Filter on appointment types and appts
			CSchedulerCountSettings s;

			//TES 9/8/03: Start with the first VISIBLE column
			int index = 0;
			for (index = 0, i=m_pSingleDayCtrl.GetLeftDay(), x = rc.left - rcParent.left + nTimeButtonWidth; i < m_pSingleDayCtrl.GetLeftDay()+nDays; index++, i++, x += nWidth)
			{
				CString str;
				int nResIndex = 0;
				int nApptCount = 0;
				CReservation pRes(__FUNCTION__);
				while(NULL != (pRes = m_pSingleDayCtrl.GetReservation(__FUNCTION__, i, nResIndex))) {
					// (j.dinatale 2010-10-21) - PLID 36744 - Dont count the appt if not yet saved, meaning it will have a ReservationID of -1
					// TES 2/23/2007 - PLID 24904 - Don't count template blocks
					// (c.haag 2010-01-04 10:11) - PLID 28977 - Factor in type and person ID
					if (pRes.GetReservationID() != -1 && -1 == pRes.GetTemplateItemID() && s.IsPatientAllowed(pRes.GetPersonID()) && s.IsAptTypeAllowed(pRes.GetAptTypeID())) {
						// (a.walling 2013-01-21 16:48) - PLID 54745 - Appointment count calculation in scheduler reverted to stop using recordsets and instead use Reservation objects' Data[] property map
						if (pRes.Data["CancelledDate"].vt != VT_DATE) {
							nApptCount++;
						}
					}
					nResIndex++;
				}
				if (nApptCount > 0)
				{
					if (nApptCount == 1)
						str.Format("%i Appt.", nApptCount);
					else
						str.Format("%i Appts.", nApptCount);
				}

				CRect rcLabel(x, y, x+nWidth, y+20);
				if (m_arDayCountLabels.GetSize() <= index) {
					CNxStatic* pLabel = new CNxStatic();
					pLabel->Create(str, WS_CHILD|WS_VISIBLE|SS_CENTER, rcLabel, this);	
					// (a.walling 2008-11-19 15:06) - PLID 32103 - This needs to be transparent
					pLabel->ModifyStyleEx(0, WS_EX_TRANSPARENT);
					pLabel->SetFont(&theApp.m_multiresFont);	
					m_arDayCountLabels.Add(pLabel);
				} else {
					// (a.walling 2008-12-24 11:18) - PLID 32560 - Move if needed
					CRect rcActual;
					m_arDayCountLabels[index]->GetWindowRect(rcActual);
					ScreenToClient(rcActual);
					if (rcActual != rcLabel) {
						m_arDayCountLabels[index]->MoveWindow(rcLabel);
					}
					m_arDayCountLabels[index]->SetWindowText(str);
				}
			}
		}

		// (a.walling 2008-09-22 13:28) - PLID 31445 - This is unnecessary now
		/*
		CRect rc, rcParent;
		long x,y;
		GetDlgItem(IDC_MULTI_RESOURCE_CTRL)->GetWindowRect(&rc);
		GetWindowRect(&rcParent);
		x = rc.left - rcParent.left;
		y = (rc.bottom - rcParent.top) + 5;
		rc.left = x;
		rc.top = y;
		InvalidateRect(rc);
		*/
	}	NxCatchAll(__FUNCTION__);
}


// (a.walling 2008-10-01 11:03) - PLID 31445 - Destroy any dynamically allocated labels
void CMultiResourceDlg::InvalidateDayCountLabels()
{
	try {
		for (int i = 0; i < m_arDayCountLabels.GetSize(); i++) {
			::DestroyWindow(m_arDayCountLabels[i]->GetSafeHwnd());
			delete m_arDayCountLabels[i];
		}
	} NxCatchAll("Error clearing resource count labels");

	m_arDayCountLabels.RemoveAll();
}

// (a.walling 2010-06-23 09:41) - PLID 39263 - Update all columns' available location info
void CMultiResourceDlg::UpdateColumnAvailLocationsDisplay()
{	
	EnsureButtonLabels();
}

void CMultiResourceDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	// (a.walling 2008-10-01 11:04) - PLID 31445 - This used to paint the day/resource count labels. Now
	// we use dynamically created CNxStatics for that.
}

void CMultiResourceDlg::PrePrint()
{
}

void CMultiResourceDlg::PostPrint()
{
}

// (z.manning, 05/24/2007) - PLID 26062 - Multi resource now has its own version of UpdateBlocks
// which allows us to tell the main implementation of UpdateBlocks to load all template info at
// once instead of once per resource.
void CMultiResourceDlg::UpdateBlocks(bool bUpdateTemplateBlocks, bool bForceUpdate)
{
	CNxSchedulerDlg::UpdateBlocks(bUpdateTemplateBlocks, bForceUpdate, true);
}

// (z.manning, 02/13/2008) - PLID 28909 - Function to return the open recordset to load attendance appointments.
_RecordsetPtr CMultiResourceDlg::GetAttendanceResRecordsetFromBaseQuery(CString strBaseQuery)
{
	return CreateParamRecordset(strBaseQuery + 
		FormatString(
		" AND ResourceT.ID IN (%s) AND AttendanceAppointmentsT.Date = {OLEDATETIME} "
		, m_pParent->CalcCurrentResourceIDs())
		, GetActiveDate());
}

// (a.walling 2008-10-01 11:02) - PLID 31445 - Cleanup any dynamically created labels
void CMultiResourceDlg::OnDestroy() 
{
	InvalidateDayCountLabels();

	CNxSchedulerDlg::OnDestroy();
}

// (c.haag 2009-12-22 14:02) - PLID 28977 - The user can now click on the count label to
// configure how counts are calculated
void CMultiResourceDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	try {
		DoClickHyperlink(nFlags, point);
	}
	NxCatchAll(__FUNCTION__);

	// (b.cardillo 2016-06-04 19:56) - NX-100776 - Discovered while doing this item that this was incorrectly 
	// calling the grand-base class instead of the direct base class.
	__super::OnLButtonDown(nFlags, point);
}

// (c.haag 2009-12-22 14:02) - PLID 28977 - The user can now click on the count label to
// configure how counts are calculated
void CMultiResourceDlg::DoClickHyperlink(UINT nFlags, CPoint point)
{
	// (v.maida 2016-04-22 16:27) - PLID 51696 - Don't continue if we're not on the correct tab.
	if (m_pParent && this != m_pParent->GetActiveSheet()) {
		return;
	}

	CRect rc;
	GetDlgItem(IDC_ACTIVE_RESOURCES_APTCOUNT_LABEL)->GetWindowRect(rc);
	ScreenToClient(&rc);

	if (rc.PtInRect(point)) {
		CSchedCountSetupDlg dlg(this);
		if (IDOK == dlg.DoModal()) {
			UpdateView();			
		}
	}
}

// (c.haag 2009-12-22 14:02) - PLID 28977 - Change the mouse cursor appearance if it's 
// hovering over a hyperlink 
BOOL CMultiResourceDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		// (b.cardillo 2016-06-07 02:45) - NX-100775 - Modularized the handling here to make the day label code cleaner
		CPoint pt;
		GetMessagePos(&pt);
		ScreenToClient(&pt);
		int nID;
		{
			CWnd *pChild = ChildWindowFromPoint(pt, CWP_SKIPINVISIBLE);
			if (pChild != NULL && pChild->IsWindowEnabled()) {
				nID = pChild->GetDlgCtrlID();
			} else {
				nID = 0;
			}
		}
		switch (nID) {
		case IDC_ACTIVE_RESOURCES_APTCOUNT_LABEL:
			SetCursor(GetLinkCursor());
			return TRUE;
		DAY_LABEL_ARRAY(CASE_CMultiResourceDlg__OnSetCursor) // (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
			break; // This break is not needed; it's only here to make this code appear less confusing
		default:
			break;
		}
	}
	NxCatchAll(__FUNCTION__);

	// (b.cardillo 2016-06-04 19:56) - NX-100776 - Discovered while doing this item that this was incorrectly 
	// calling the grand-base class instead of the direct base class.
	return __super::OnSetCursor(pWnd, nHitTest, message);
}

// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
LRESULT CMultiResourceDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UINT nIdc = (UINT)wParam;
		switch (nIdc) {
		DAY_LABEL_ARRAY(CASE_CMultiResourceDlg__OnDayLabel) // (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
			break; // This break is not needed; it's only here to make this code appear less confusing
		default:
			break;
		}
	} NxCatchAll(__FUNCTION__);

	return __super::OnLabelClick(wParam, lParam);
}

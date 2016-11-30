// WeekDayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NxSchedulerDlg.h"
#include "WeekDayDlg.h"
#include "SchedulerRc.h"
#include "globaldatautils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "ResEntryDlg.h"
#include "SchedCountSetupDlg.h"
using namespace ADODB; // (j.luckoski 2012-06-11 10:14) - PLID 50235 - Allow _RecordsetPtr

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace SINGLEDAYLib;

extern CPracticeApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CWeekDayDlg dialog


CWeekDayDlg::CWeekDayDlg(CWnd* pParent /*=NULL*/)
	: CNxSchedulerDlg(CWeekDayDlg::IDD, pParent)
{
	DAY_LABEL_ARRAY(FILL_ARRAY_CWeekDayDlg__DayLabels)


	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Scheduler_Module/schedule_an_appointment.htm";
}


void CWeekDayDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxSchedulerDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWeekDayDlg)
	DAY_LABEL_ARRAY(DDX_CWeekDayDlg__DayLabels)
	DDX_Control(pDX, IDC_MOVE_WEEK_BACK, m_btnWeekBack);
	DDX_Control(pDX, IDC_MOVE_WEEK_FORWARD, m_btnWeekForward);
	DDX_Control(pDX, IDC_ACTIVE_DATE_LABEL, m_nxstaticActiveDateLabel);
	DDX_Control(pDX, IDC_ACTIVE_DATE_APTCOUNT_LABEL, m_nxstaticActiveDateAptcountLabel);
	DDX_Control(pDX, IDC_ACTIVE_WEEK_APTCOUNT_LABEL, m_nxlabelActiveWeekAptcountLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWeekDayDlg, CNxSchedulerDlg)
	//{{AFX_MSG_MAP(CWeekDayDlg)
	// (b.cardillo 2016-06-06 16:20) - NX-100773 - Moved event click handling to base class
	// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(IDC_MOVE_WEEK_BACK, OnMoveWeekBack)
	ON_BN_CLICKED(IDC_MOVE_WEEK_FORWARD, OnMoveWeekForward)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWeekDayDlg message handlers

BOOL CWeekDayDlg::OnInitDialog()
{
	try {
		// (j.luckoski 2012-04-26 09:08) - PLID 11597 - Load properties for showing cancelled appts.
		// (j.luckoski 2012-05-08 17:10) - PLID 50242 - Add all properties to bulk cache
		g_propManager.CachePropertiesInBulk("WeekDayDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name IN ("
					"'ShowCancelledAppointment', 'CancelledDateRange', 'SchedCancelledApptColor', "
					"'WeekViewVisibleColumns', 'ResShowResourceName' "
			"))", _Q(GetCurrentUserName()));

		
		m_nCancelledAppt = GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true);
		// (j.luckoski 2012-06-12 15:58) - PLID 11597 - Did not default to 24
		m_nDateRange = GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true);
		m_nCancelColor = GetRemotePropertyInt("SchedCancelledApptColor", RGB(192,192,192), 0, GetCurrentUserName(), true);
		// Must set m_pSingleDayCtrl before CNxSchedulerDlg::OnInitDialog is called
		// (c.haag 2010-03-26 15:14) - PLID 37332 - Use SetDispatch
		m_pSingleDayCtrl.SetDispatch( (LPDISPATCH)GetDlgItem(IDC_WEEK_DAY_CTRL)->GetControlUnknown() );
		m_pEventCtrl.SetDispatch( (LPDISPATCH)GetDlgItem(IDC_WEEK_EVENT_CTRL)->GetControlUnknown() );

		// Get a variable so we can work with the resource combo for this view
		m_dlResources = BindNxDataListCtrl(IDC_RESOURCE_LIST, false);

		m_btnWeekBack.AutoSet(NXB_LLEFT);
		m_btnWeekForward.AutoSet(NXB_RRIGHT);

		m_nColumnCount = GetRemotePropertyInt("WeekViewVisibleColumns", 5, 0, "<None>", true);

		m_pSingleDayCtrl.PutDayVisibleCount(m_nColumnCount);
		m_pSingleDayCtrl.PutDayTotalCount(__FUNCTION__, m_nColumnCount);
		InvalidateDayCountLabels(); // (a.walling 2008-10-01 11:06) - PLID 31445 - Ensure the labels match the daycount
		m_pEventCtrl.PutDayVisibleCount(m_nColumnCount);
		m_pEventCtrl.PutDayTotalCount(__FUNCTION__, m_nColumnCount);
		GetDlgItem(IDC_ACTIVE_WEEK_APTCOUNT_LABEL)->SetFont(&theApp.m_multiresFont);

		// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
		DAY_LABEL_ARRAY(INIT_CWeekDayDlg__DayLabels);

		// (c.haag 2009-12-22 13:44) - PLID 28977 - The Appt count label is now a hyperlink label
		m_nxlabelActiveWeekAptcountLabel.SetType(dtsHyperlink);

		return CNxSchedulerDlg::OnInitDialog();
	}
	NxCatchAll(__FUNCTION__);
	return TRUE;
}

bool CWeekDayDlg::ReFilter(CReservationReadSet &rsReadSet, bool bForceOpen /* = false */)
{
	// Build the query
	// (c.haag 2006-12-12 16:32) - PLID 23845 - I wrote an easier-to-maintain implementation
	// that supports more fields so that we can properly suppress template blocks in the face
	// of appointments
	// (z.manning, 05/24/2007) - PLID 26122 - Removed references to EnablePrecisionTemplating preference.
	// (c.haag 2008-11-20 16:33) - PLID 31128 - Added InsuranceReferralSummary
	// (z.manning 2009-06-30 16:45) - PLID 34744 - Only load ins ref summary if necessary
	//TES 1/14/2010 - PLID 36762 - We now need to load an additional recordset for the security groups that patients in this list are 
	// members of; that way CReservationReadSet can hide their demographic information.
	// (j.jones 2011-02-11 09:31) - PLID 35180 - ResExtendedQ is now a function that takes in:
	//@ExcludeCanceledAppointments BIT, @FromDate DATETIME, @ToDate DATETIME, @ResourceIDs NTEXT
	//ResourceIDs must be comma-delimited.

	COleDateTime dtActiveFrom, dtActiveTo;
	m_pParent->GetActiveDateRange(dtActiveFrom, dtActiveTo);
	
	// (a.walling 2013-06-18 11:34) - PLID 57204 - Use new ResExtendedQ alternative in scheduler views for significantly better scheduler performance
	// (j.jones 2014-12-03 13:54) - PLID 64275 - if the appointment has overridden a mix rule, show it first
	CSqlFragment query(
		"SET NOCOUNT ON; \r\n"
		"DECLARE @dateFrom DATETIME; DECLARE @dateTo DATETIME; \r\n"
		"DECLARE @resourceID INT; \r\n"
		"SET @dateFrom = {OLEDATETIME}; SET @dateTo = DATEADD(d, 1, {OLEDATETIME}); \r\n"
		"SET @resourceID = {INT}; \r\n"
		// (a.walling 2013-08-05 17:32) - PLID 57869 - Force seeks via table variables for scheduler refresh
		// (a.walling 2015-04-27 09:55) - PLID 65746 - Force StartTime_Cluster index so SQL doesn't try to scan the ID index for the whole table to get ordered ID for the primary key
		"DECLARE @apptIDs TABLE (ID INTEGER NOT NULL PRIMARY KEY); \r\n"
		"INSERT INTO @ApptIDs SELECT ID FROM AppointmentsT WITH(INDEX(StartTime_Cluster)) WHERE StartTime >= @dateFrom AND StartTime < @dateTo; \r\n"
		"{SQL} \r\n" // base query
		"WHERE AppointmentsT.ID IN (SELECT ID FROM @apptIDs) \r\n"
		"AND AppointmentResourceT.ResourceID = @resourceID \r\n"
		"{CONST_STR} \r\n" // extra filters
		"ORDER BY IsMixRuleOverridden DESC, AppointmentsT.StartTime, AppointmentsT.ID, AptPurposeT.Name; \r\n"
		"\r\n"
		"SELECT DISTINCT SecurityGroupDetailsT.PatientID, SecurityGroupDetailsT.SecurityGroupID \r\n"
		"FROM SecurityGroupDetailsT "
		"INNER JOIN AppointmentsT \r\n"
		"ON SecurityGroupDetailsT.PatientID = AppointmentsT.PatientID \r\n"
		"WHERE AppointmentsT.StartTime >= @dateFrom AND AppointmentsT.StartTime < @dateTo \r\n"
		"ORDER BY SecurityGroupDetailsT.PatientID, SecurityGroupDetailsT.SecurityGroupID; \r\n"
		"SET NOCOUNT OFF; \r\n"
		, AsDateNoTime(dtActiveFrom), AsDateNoTime(dtActiveTo)
		, m_pParent->GetActiveResourceID()
		, Nx::Scheduler::GetResExtendedBase(m_pParent, dtActiveFrom, dtActiveTo)
		, GetExtraFilter()
	);

	CNxPerform nxp("ReFilter " __FUNCTION__);
	return rsReadSet.ReFilter(query);
}

BEGIN_EVENTSINK_MAP(CWeekDayDlg, CNxSchedulerDlg)
    //{{AFX_EVENTSINK_MAP(CWeekDayDlg)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_DAY_CTRL, -607 /* MouseUp */, CNxSchedulerDlg::OnMouseUpSingleday, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_DAY_CTRL, 4 /* ReservationRightClick */, CNxSchedulerDlg::OnReservationRightClick, VTS_DISPATCH VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_EVENT_CTRL, 4 /* ReservationRightClick */, CNxSchedulerDlg::OnReservationRightClick, VTS_DISPATCH VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_DAY_CTRL, 1 /* ReservationClick */, CNxSchedulerDlg::OnReservationClick, VTS_DISPATCH)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_EVENT_CTRL, 1 /* ReservationClick */, CNxSchedulerDlg::OnReservationClick, VTS_DISPATCH)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_DAY_CTRL, 10 /* ReservationEndDrag */, CNxSchedulerDlg::OnReservationEndDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_EVENT_CTRL, 10 /* ReservationEndDrag */, CNxSchedulerDlg::OnReservationEndDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_DAY_CTRL, 2 /* ReservationAdded */, CNxSchedulerDlg::OnReservationAdded, VTS_DISPATCH)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_EVENT_CTRL, 2 /* ReservationAdded */, CNxSchedulerDlg::OnReservationAdded, VTS_DISPATCH)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_DAY_CTRL, 12 /* ReservationEndResize */, CNxSchedulerDlg::OnReservationEndResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_EVENT_CTRL, -607 /* MouseUp */, CNxSchedulerDlg::OnMouseUpEvent, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_DAY_CTRL, 6 /* ReservationMouseDown */, OnReservationMouseDown, VTS_DISPATCH VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_EVENT_CTRL, 6 /* ReservationMouseDown */, OnReservationMouseDown, VTS_DISPATCH VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_DAY_CTRL, 9 /* ReservationDrag */, CNxSchedulerDlg::OnReservationDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_EVENT_CTRL, 9 /* ReservationDrag */, CNxSchedulerDlg::OnReservationDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_DAY_CTRL, 11 /* ReservationResize */, CNxSchedulerDlg::OnReservationResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_EVENT_CTRL, 11 /* ReservationResize */, CNxSchedulerDlg::OnReservationResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CWeekDayDlg, IDC_WEEK_DAY_CTRL, 14 /* ReservationRecalculateColor */, CNxSchedulerDlg::OnReservationRecalculateColor, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


void CWeekDayDlg::OnMoveDayBack() 
{
	MoveCurrentDate(COleDateTimeSpan(-1, 0, 0, 0));
}

void CWeekDayDlg::OnMoveDayForward() 
{
	MoveCurrentDate(COleDateTimeSpan(1, 0, 0, 0));
}

void CWeekDayDlg::OnMoveWeekBack() 
{
	COleDateTime newDate;
	newDate = m_DateSelCombo.GetValue();

//	MoveCurrentDate(COleDateTimeSpan(-7, 0, 0, 0));
	do {
		newDate -= COleDateTimeSpan(1,0,0,0);
	} while (newDate.GetDayOfWeek() != 2);

	BeginWaitCursor();
	SetActiveDate(newDate);
	OnSetDateSelCombo();
	EndWaitCursor();
}

void CWeekDayDlg::OnMoveWeekForward() 
{
	COleDateTime newDate;
	newDate = m_DateSelCombo.GetValue();

	// If we're on a sunday, move forward to a week from that following monday
	if (newDate.GetDayOfWeek() == 1 /* sunday */)
	{
		newDate += COleDateTimeSpan(8,0,0,0);
	}
	else {
		do {
			newDate += COleDateTimeSpan(1,0,0,0);
		} while (newDate.GetDayOfWeek() != 2 /* monday */);
	}

	BeginWaitCursor();
	SetActiveDate(newDate);
	OnSetDateSelCombo();
	EndWaitCursor();
}

int CWeekDayDlg::SetControlPositions()
{
	CNxSchedulerDlg::SetControlPositions();
	
	ResizeLabels();
	// (a.walling 2008-12-24 11:17) - PLID 32560
	EnsureCountLabelText();

	// (a.walling 2009-02-04 10:14) - PLID 31956 - No longer used
	//SetRgnBg();//should fix background drawing and flickering issues
	return 1;
}

void CWeekDayDlg::EnsureButtonLabels()
{
	COleDateTime dtDate = GetActiveDate();
	for (long i=0; i<m_nColumnCount; i++) {
		
		// (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
		CNxLabel* pButton = m_pDayLabelBtn[i];
		
		// (a.walling 2010-06-23 09:41) - PLID 39263 - Also update the description
		CString strInfo = m_ResourceAvailLocations.GetDayDescription(i);

		pButton->SetToolTip(strInfo);

		if (strInfo.IsEmpty()) {
			pButton->ClearTextColorOverride();
		} else {
			pButton->SetTextColorOverride(RGB(0xC0, 0x00, 0x00));
		}

		// (c.haag 2010-11-16 13:49) - PLID 39444 - Also include any resource location abbreviations
		pButton->SetText(m_ResourceAvailLocations.GetDayLabel(i, FormatDateTimeForInterface(dtDate + COleDateTimeSpan(i), "%a - ") + FormatDateTimeForInterface(dtDate + COleDateTimeSpan(i), DTF_STRIP_YEARS, dtoDate)));
	}
}

COleDateTime CWeekDayDlg::GetWorkingDate(int nDay /* = 0 */)
{
	return GetActiveDate() + (COleDateTimeSpan)nDay;
}


void CWeekDayDlg::OnDayLabel(long nDayLabelIndex)
{
	try {
		// Set the active date to the date corresponding to the column whose header the user clicked
		// This is safe because we can't scroll, so the label index is exactly the day offset
		SetActiveDate(GetWorkingDate(nDayLabelIndex));
		
		// Switch to the day tab
		m_pParent->SwitchToTab(CSchedulerView::sttOneDayView);
	} NxCatchAll("CWeekDayDlg::OnDayLabel");
}

void CWeekDayDlg::Print(CDC *pDC, CPrintInfo *pInfo)
{
	CString strHdr, strFooter;
	CFont hdrFont, ftrFont;
	CFont subhdrFont;
	CFont* pOldFont;
	CRect rectHeader, rectFooter;
	CString strLabel;
	CRect rectTemp;

	// Set up header								Header is 11.6 percent of page height
	rectHeader.SetRect(0, 0, pInfo->m_rectDraw.Width(), (int)((0.108) * (double)pInfo->m_rectDraw.Height()));
	rectFooter.SetRect(0, (int)((0.984) * (double)pInfo->m_rectDraw.Height()), pInfo->m_rectDraw.Width(), pInfo->m_rectDraw.Height());
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&hdrFont, 180, "Arial", pDC);
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&ftrFont, 90, "Arial", pDC);
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&subhdrFont, 120, "Arial", pDC);
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
	COleDateTime dtToDate = GetActiveDate() + COleDateTimeSpan(m_nColumnCount-1,0,0,0);
	strHdr.Format("%sAppointments for %s\n%s - %s", strApptType, 
		/*m_ResourceCombo.GetText()*/VarString(m_dlResources->GetValue( m_dlResources->GetCurSel(), 2)), FormatDateTimeForInterface(GetActiveDate(), "%a ") + FormatDateTimeForInterface(GetActiveDate(), DTF_STRIP_SECONDS, dtoDate),
		FormatDateTimeForInterface(dtToDate, "%a ") + FormatDateTimeForInterface(dtToDate, DTF_STRIP_SECONDS, dtoDate));
	pDC->DrawText(strHdr, rectHeader, DT_CENTER);

	// Print subheaders
	pDC->SelectObject(&subhdrFont);
	int timeButtonPrintWidth = (int)((double)rectHeader.Width() * TIME_BUTTON_PRINT_WIDTH_PERCENT);
	int subwidth = (rectHeader.Width() - timeButtonPrintWidth) / 
		m_pSingleDayCtrl.GetDayVisibleCount();
	for (int i=0; i < m_pSingleDayCtrl.GetDayVisibleCount(); i++) {
		strLabel = m_pDayLabelBtn[i]->GetText();
		rectTemp.SetRect(timeButtonPrintWidth + i * subwidth, rectHeader.top, 
			timeButtonPrintWidth + (i+1) * subwidth, rectHeader.bottom);
		pDC->DrawText(strLabel, rectTemp, DT_CENTER | DT_BOTTOM | DT_SINGLELINE);
	}
	pDC->SelectObject(pOldFont);

	/*
	// Print the control in the area below the header area
	pInfo->m_rectDraw.top = rectHeader.bottom;
	m_pSingleDayCtrl.Print((IUnknown *)pDC, (IUnknown *)((CPrintInfo *)pInfo));
	*/

	pInfo->m_rectDraw.top = rectHeader.bottom;
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
	rectEventCaption.SetRect(0, rectHeader.bottom, (int)nTimeButtonWidthActual, rectHeader.bottom + (int)nEventCtrlHeight);
	rectEventCtrl.SetRect((int)nTimeButtonWidthTotal, rectHeader.bottom, pInfo->m_rectDraw.Width(), rectHeader.bottom + (int)nEventCtrlHeight);
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
	COleDateTime dtPrintDate = COleDateTime::GetCurrentTime();
	strFooter.Format("Printed On: %s", FormatDateTimeForInterface(dtPrintDate, NULL, dtoDate));
	pDC->SelectObject(&ftrFont);
	pDC->DrawText(strFooter, rectFooter, DT_RIGHT);
	pDC->SelectObject(pOldFont);
}

ADODB::_RecordsetPtr CWeekDayDlg::GetSingleApptQuery(long nResID)
{
	// Variables for use in generating the query
	// (a.walling 2013-05-31 14:06) - PLID 56961 - GetExtraResSimpleFilter is parameterized and simplified
	CSqlFragment filter = GetExtraResSimpleFilter();

	// Build the query
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized	
	// (j.jones 2011-02-11 09:31) - PLID 35180 - ResExtendedQ is now a function that takes in:
	//@ExcludeCanceledAppointments BIT, @FromDate DATETIME, @ToDate DATETIME, @ResourceIDs NTEXT
	// (j.luckoski 2012-04-26 09:14) - PLID 11597 - Load extra filter based on properties

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

	// (j.luckoski 2012-04-26 09:15) - PLID 11597 - ResExtendedQ now takes property value to load cancelled appts or not.
	// (a.walling 2013-05-31 14:06) - PLID 56961 - Select from GetResSimpleSql aliased to ResExQ, and restrict the date range and resource(s)
	// (a.walling 2013-06-18 11:12) - PLID 57196 - Use snapshot isolation for the main scheduler CReservationReadSet queries
	// (j.jones 2014-12-04 15:31) - PLID 64119 - added PrimaryInsuredPartyID
	return CreateParamRecordset(GetRemoteDataSnapshot(), 
		"SELECT Status, StartTime, Convert(DateTime,ResExQ.Date) AS Date, EndTime, LocationID, AptTypeID, AptPurposeID, AptPurpose, PatientID, PatientName, "
		"ShowState, LastLM, ModifiedDate, ModifiedLogin, '' AS ResourceString, "
		"CASE WHEN ResExQ.Status = 4 THEN {INT} ELSE ResExQ.Color END AS Color, ShowStateColor, "
		"ResExQ.Status, ResExQ.CancelledDate, " // (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
		"PrimaryAppointmentInsuredPartyQ.InsuredPartyID AS PrimaryInsuredPartyID "
		"FROM ({SQL}) AS ResExQ "
		"LEFT JOIN ("
		"	SELECT AppointmentID, InsuredPartyID FROM AppointmentInsuredPartyT "
		"	WHERE Placement = 1 "
		") AS PrimaryAppointmentInsuredPartyQ ON ResExQ.ID = PrimaryAppointmentInsuredPartyQ.AppointmentID "
		"WHERE ResExQ.ID = {INT} "
		"AND ResExQ.ResourceID = {INT} "
		"AND ResExQ.StartTime >= {OLEDATETIME} AND ResExQ.StartTime < DATEADD(day, 1, {OLEDATETIME}) "	
		"{SQL} "
		"ORDER BY AptPurpose"
		, m_nCancelColor
		, Nx::Scheduler::GetResSimpleSql()
		, nResID
		, m_pParent->GetActiveResourceID()
		, AsDateNoTime(GetActiveDate())
		, AsDateNoTime(GetActiveDate() + COleDateTimeSpan(m_pSingleDayCtrl.GetDayTotalCount()-1, 0, 0, 0))
		, filter
	);
}

BOOL CWeekDayDlg::SingleApptFitsSlot(long iDay, ADODB::_RecordsetPtr& prsAppt)
{
	try {
		COleDateTime dtStart = prsAppt->Fields->Item["StartTime"]->Value.date;
		if (GetActiveDate() + COleDateTimeSpan(iDay,0,0,0) ==
			dtStart - COleDateTimeSpan(0, dtStart.GetHour(), dtStart.GetMinute(), dtStart.GetSecond()))
		{
			return TRUE;
		}
	}
	NxCatchAll("Error in SingleApptFitsSlot");
	return FALSE;
}

void CWeekDayDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// (j.luckoski 2012-04-26 09:15) - PLID 11597 - Load properties on refresh view (change a property, or alter appt)
	m_nCancelledAppt = GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true);
	// (j.luckoski 2012-06-12 15:58) - PLID 11597 - Did not default to 24
	m_nDateRange = GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true);
	m_nCancelColor = GetRemotePropertyInt("SchedCancelledApptColor", RGB(192,192,192), 0, GetCurrentUserName(), true);
	//DRT 7/17/2008 - PLID 30765 - We cannot allow the screen to be refreshed
	//	if the ResEntryDlg is open.  Otherwise the pointer it maintains to
	//	the currently clicked CReservation can become disconnected, and we
	//	may inadvertantly try to access data we no longer have.
	// (c.haag 2010-06-23 10:10) - PLID 39210 - Don't update if a reservation is locked by
	// a right-click action
	// (c.haag 2010-08-27 11:38) - PLID 40108 - Moved the logic into one function
	if (!IsSafeToUpdateView())
	{
		//It is visible, do not update!
	}
	else {
		//not visible, go ahead and do our updates
		m_nColumnCount = GetRemotePropertyInt("WeekViewVisibleColumns", 5, 0, "<None>", true);

		if(m_pSingleDayCtrl.GetDayTotalCount() != m_nColumnCount) {
			m_pSingleDayCtrl.PutDayTotalCount(__FUNCTION__, m_nColumnCount);
			InvalidateDayCountLabels(); // (a.walling 2008-10-01 11:06) - PLID 31445 - Ensure the labels match the daycount
			m_pSingleDayCtrl.PutDayVisibleCount(m_nColumnCount);
			m_pEventCtrl.PutDayTotalCount(__FUNCTION__, m_nColumnCount);
			m_pEventCtrl.PutDayVisibleCount(m_nColumnCount);
			m_bNeedUpdate = true;
			ResizeLabels();
		}
	}

	//Continue to call the base class regardless, it has always obeyed this rule.
	CNxSchedulerDlg::UpdateView(bForceRefresh); // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
}


void CWeekDayDlg::ResizeLabels()
{
	CRect tmpRect, btnRect, rcEvent, rcSingleDay;
	CPoint ptTopLeft;
	long nDayWidth;

	
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
	nDayWidth = m_pSingleDayCtrl.GetDayWidth();
	
	// Resize the first button label
	btnRect.left = tmpRect.left + m_pSingleDayCtrl.GetTimeButtonWidth();
	btnRect.right = btnRect.left + nDayWidth;
	m_pDayLabelBtn[0]->MoveWindow(btnRect);
	
	// Resize the rest of the button labels
	for (int i=1; i<m_nColumnCount; i++) {
		btnRect.left = btnRect.right + 1;
		btnRect.right = btnRect.left + nDayWidth - 1;
		m_pDayLabelBtn[i]->ShowWindow(SW_SHOW);
		m_pDayLabelBtn[i]->MoveWindow(btnRect);
	}
	for(i = m_nColumnCount; i < MAX_COLUMNS; i++) {
		m_pDayLabelBtn[i]->ShowWindow(SW_HIDE);
	}
	
}

void CWeekDayDlg::Internal_GetWorkingResourceAndDateFromGUI(IN const long nColumnIndex, OUT long &nWorkingResourceID, OUT COleDateTime &dtWorkingDate) const
{
	// The week view has multiple columns doesn't allow a scrollbar (unlike the multi-resource view 
	// which DOES allow a scrollbar), so each column corresponds to a single date and that date is 
	// very simply the scheduler active date plus nColumnIndex days.  It's a one-resource view so the
	// resource id is easy to calculate, it's just the resource currently selected in the dropdown.

	if(m_dlResources->GetCurSel() == -1) {
		m_dlResources->CurSel = 0;

		if(m_dlResources->GetCurSel() == -1) {
			ThrowNxException(
			"CWeekDayDlg::Internal_GetWorkingResourceAndDateFromGUI: There are no resources in the current view.");
		}
	}

	// The resource is simply the one active resource (we've ensured that there can never be no 
	// selection in the resource dropdown
	nWorkingResourceID = VarLong(m_dlResources->GetValue(m_dlResources->GetCurSel(), ResourceComboColumns::ID));

	// The date needs to be pulled off the screen, and then nColumnIndex days need to be added
	{
		// We happen to know for certain that "GetValue" is a const function, but because it's an 
		// activex function there's no way for the compiler to know; hence we have to deceive the 
		// compiler into believing it can safely call it (which it can).
		// (a.walling 2008-05-13 10:31) - PLID 27591 - No longer necessary with CDateTimePicker
		//CDTPicker *pdtp = const_cast<CDTPicker *>(&m_DateSelCombo); 
		// Get the date off the screen
		COleDateTime dtActiveDateOnScreen = m_DateSelCombo.GetValue();
		// Add nColumnIndex days
		dtWorkingDate = dtActiveDateOnScreen + COleDateTimeSpan(nColumnIndex, 0, 0, 0);
	}
}

BOOL CWeekDayDlg::Internal_IsMultiResourceSheet() const
{
	return FALSE;
}

BOOL CWeekDayDlg::Internal_IsMultiDateSheet() const
{
	return TRUE;
}

void CWeekDayDlg::EnsureCountLabelText()
{
	try{
		int nApptCount = 0;
		if (m_pSingleDayCtrl != NULL)
		{
			// (c.haag 2009-12-22 17:34) - PLID 28977 - Filter on appointment types and appts
			CSchedulerCountSettings s;
			for(int i = 0; i < m_pSingleDayCtrl.GetDayVisibleCount(); i++) {
				CReservation pRes(__FUNCTION__);
				int nResIndex = 0;
				while(NULL != (pRes = m_pSingleDayCtrl.GetReservation(__FUNCTION__, i, nResIndex))) {
					// (j.dinatale 2010-10-21) - PLID 36744 - Dont count the appt if not yet saved, meaning it will have a ReservationID of -1
					// (c.haag 2007-02-01 11:48) -  PLID 23666 - Don't count template blocks
					// (c.haag 2009-12-22 17:34) - PLID 28977 - Factor in type and person ID
					// (j.luckoski 2012-05-08 16:18) - PLID 50235 - Only count uncancelled appts
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
		m_nxlabelActiveWeekAptcountLabel.SetText(strCountLabelText);	

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
			GetDlgItem(IDC_WEEK_DAY_CTRL)->GetWindowRect(&rc);
			ScreenToClient(rc);
			GetClientRect(&rcParent);
			y = (rc.bottom - rcParent.top) + 5;

			// (c.haag 2010-01-04 10:12) - PLID 28977 - Filter on appointment types and appts
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
					// (c.haag 2010-01-04 10:12) - PLID 28977 - Factor in type and person ID
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
		GetDlgItem(IDC_WEEK_DAY_CTRL)->GetWindowRect(&rc);
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
void CWeekDayDlg::InvalidateDayCountLabels()
{
	try {
		for (int i = 0; i < m_arDayCountLabels.GetSize(); i++) {
			::DestroyWindow(m_arDayCountLabels[i]->GetSafeHwnd());
			delete m_arDayCountLabels[i];
		}
	} NxCatchAll("Error clearing day count labels");

	m_arDayCountLabels.RemoveAll();
}

// (a.walling 2010-06-23 09:41) - PLID 39263 - Update all columns' available location info
void CWeekDayDlg::UpdateColumnAvailLocationsDisplay()
{
	EnsureButtonLabels();
}

void CWeekDayDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	// (a.walling 2008-10-01 11:04) - PLID 31445 - This used to paint the day/resource count labels. Now
	// we use dynamically created CNxStatics for that.
}

void CWeekDayDlg::PrePrint()
{
}

void CWeekDayDlg::PostPrint()
{}

// (z.manning, 02/13/2008) - PLID 28909 - Function to return the open recordset to load attendance appointments.
ADODB::_RecordsetPtr CWeekDayDlg::GetAttendanceResRecordsetFromBaseQuery(CString strBaseQuery)
{
	COleDateTime dtStart, dtEnd;
	m_pParent->GetActiveDateRange(dtStart, dtEnd);
	return CreateParamRecordset(strBaseQuery + 
		" AND ResourceT.ID = {INT} AND AttendanceAppointmentsT.Date >= {OLEDATETIME} AND AttendanceAppointmentsT.Date <= {OLEDATETIME} "
		, m_pParent->GetActiveResourceID(), dtStart, dtEnd);
}

// (a.walling 2008-10-01 11:02) - PLID 31445 - Cleanup any dynamically created labels
void CWeekDayDlg::OnDestroy() 
{
	InvalidateDayCountLabels();

	CNxSchedulerDlg::OnDestroy();
}

// (c.haag 2009-12-22 14:02) - PLID 28977 - The user can now click on the count label to
// configure how counts are calculated
void CWeekDayDlg::OnLButtonDown(UINT nFlags, CPoint point)
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
void CWeekDayDlg::DoClickHyperlink(UINT nFlags, CPoint point)
{
	CRect rc;
	GetDlgItem(IDC_ACTIVE_WEEK_APTCOUNT_LABEL)->GetWindowRect(rc);
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
BOOL CWeekDayDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
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
		case IDC_ACTIVE_WEEK_APTCOUNT_LABEL:
			SetCursor(GetLinkCursor());
			return TRUE;
		DAY_LABEL_ARRAY(CASE_CWeekDayDlg__OnSetCursor) // (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
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
LRESULT CWeekDayDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UINT nIdc = (UINT)wParam;
		switch (nIdc) {
		DAY_LABEL_ARRAY(CASE_CWeekDayDlg__OnDayLabel) // (b.cardillo 2016-06-07 02:45) - NX-100775 - CNxLabels now
			break; // This break is not needed; it's only here to make this code appear less confusing
		default:
			break;
		}
	} NxCatchAll(__FUNCTION__);

	return __super::OnLabelClick(wParam, lParam);
}

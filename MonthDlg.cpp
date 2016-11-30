// MonthDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NxSchedulerDlg.h"
#include "CommonSchedUtils.h"
#include "MonthDlg.h"
#include "ResEntryDlg.h"
#include "GlobalUtils.h"
#include "MainFrm.h"
#include "FirstAvailableAppt.h"
#include "PracProps.h"
#include "NxStandard.h"
#include "SchedulerRc.h"
#include "GlobalSchedUtils.h"
#include "AuditTrail.h"
#include "SchedCountSetupDlg.h"
#include "SharedScheduleUtils.h"

using namespace ADODB;
#include "GlobalDataUtils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "TemplateHitSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



extern CPracticeApp theApp;

CString GetSubString(const CString &strFullString, int nSubIndex, TCHAR chDelim = '|');
void SetSubString(CString& strFullString, CString strNewSubString, int nSubIndex, TCHAR chDelim = '|');


/////////////////////////////////////////////////////////////////////////////
// CMonthDlg dialog


CMonthDlg::CMonthDlg(CWnd* pParent /*=NULL*/)
	: CNxSchedulerDlg(CMonthDlg::IDD, pParent), m_ptPriorMousePos(-1,-1)
{
	m_strDragInfo.Empty();		// Appointment drag information
	m_nVisibleNamesOffset = 0;	// Spin button offset

	//(s.dhole 6/1/2015 4:18 PM ) - PLID 65645  Remove 
	//m_IsShowingFirstAvail = FALSE; // Not showing first avail appts.

	INIT_SEMAPHORE(m_bStoring);

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Scheduler_Module/schedule_an_appointment.htm";

	m_startx = 0;
	m_starty = 0;
	m_daywidth = 0;
	m_dayheight = 0;
	m_nVisibleNamesPerDay = 0;
	m_nVisibleNamesOffset = 0;
	m_nWeekCnt = 0;
	m_bAllowScrollDown = FALSE;
}

void CMonthDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxSchedulerDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMonthDlg)
	DDX_Control(pDX, IDC_MOVE_MONTH_BACK, m_monthBackButton);
	DDX_Control(pDX, IDC_MOVE_MONTH_FORWARD, m_monthForwardButton);
	DDX_Control(pDX, IDC_WEEK_LABEL_5, m_wk5Button);
	DDX_Control(pDX, IDC_WEEK_LABEL_4, m_wk4Button);
	DDX_Control(pDX, IDC_WEEK_LABEL_3, m_wk3Button);
	DDX_Control(pDX, IDC_WEEK_LABEL_2, m_wk2Button);
	DDX_Control(pDX, IDC_WEEK_LABEL_1, m_wk1Button);
	DDX_Control(pDX, IDC_WEEK_LABEL_0, m_wk0Button);
	DDX_Control(pDX, IDC_ACTIVE_DATE_LABEL, m_nxstaticActiveDateLabel);
	DDX_Control(pDX, IDC_STATIC_MON, m_nxstaticMon);
	DDX_Control(pDX, IDC_STATIC_TUE, m_nxstaticTue);
	DDX_Control(pDX, IDC_STATIC_WED, m_nxstaticWed);
	DDX_Control(pDX, IDC_STATIC_THU, m_nxstaticThu);
	DDX_Control(pDX, IDC_STATIC_FRI, m_nxstaticFri);
	DDX_Control(pDX, IDC_STATIC_SAT, m_nxstaticSat);
	DDX_Control(pDX, IDC_EDIT_ACTIVE_SINGLEDAY_DAY, m_nxstaticEditActiveSingledayDay);
	DDX_Control(pDX, IDC_ACTIVE_DATE_APTCOUNT_LABEL, m_nxlabelActiveDateAptcountLabel);
	DDX_Control(pDX, IDC_STATIC_SUN, m_nxstaticSun);
	DDX_Control(pDX, IDC_FIRST_DAY_FRAME, m_btnFirstDayFrame);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMonthDlg, CNxSchedulerDlg)
	//{{AFX_MSG_MAP(CMonthDlg)	
	ON_BN_CLICKED(IDC_MOVE_MONTH_BACK, OnMoveMonthBack)
	ON_BN_CLICKED(IDC_MOVE_MONTH_FORWARD, OnMoveMonthForward)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CREATE()
	ON_NOTIFY(UDN_DELTAPOS, 2000, OnSpinDay)
	ON_WM_MOUSEMOVE()
	ON_WM_KILLFOCUS()
	ON_WM_TIMER()
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_NAME_OFFSETS, OnSpinAllDays)
	ON_WM_SETCURSOR()
	ON_NOTIFY(UDN_DELTAPOS, 2001, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2002, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2003, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2004, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2005, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2006, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2007, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2008, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2009, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2010, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2011, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2012, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2013, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2014, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2015, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2016, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2017, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2018, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2019, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2020, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2021, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2022, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2023, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2024, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2025, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2026, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2027, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2028, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2029, OnSpinDay)
	ON_NOTIFY(UDN_DELTAPOS, 2030, OnSpinDay)
	ON_BN_CLICKED(IDC_WEEK_LABEL_0, OnWeekLabel0)
	ON_BN_CLICKED(IDC_WEEK_LABEL_1, OnWeekLabel1)
	ON_BN_CLICKED(IDC_WEEK_LABEL_2, OnWeekLabel2)
	ON_BN_CLICKED(IDC_WEEK_LABEL_3, OnWeekLabel3)
	ON_BN_CLICKED(IDC_WEEK_LABEL_4, OnWeekLabel4)
	ON_BN_CLICKED(IDC_WEEK_LABEL_5, OnWeekLabel5)
	// (b.cardillo 2016-06-06 16:20) - NX-100773 - Moved event click handling to base class
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMonthDlg message handlers


BEGIN_EVENTSINK_MAP(CMonthDlg, CNxSchedulerDlg)

//{{AFX_EVENTSINK_MAP(CMonthDlg)
	ON_EVENT(CMonthDlg, IDC_SINGLE_DAY_CTRL, -607 /* MouseUp */, OnMouseUpSingleday, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CMonthDlg, IDC_MONTH_EVENT_CTRL, -607 /* MouseUp */, OnMouseUpEvent, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CMonthDlg, IDC_SINGLE_DAY_CTRL, 4 /* ReservationRightClick */, OnReservationRightClick, VTS_DISPATCH VTS_I4 VTS_I4)
	ON_EVENT(CMonthDlg, IDC_MONTH_EVENT_CTRL, 4 /* ReservationRightClick */, OnReservationRightClick, VTS_DISPATCH VTS_I4 VTS_I4)
	ON_EVENT(CMonthDlg, IDC_SINGLE_DAY_CTRL, 1 /* ReservationClick */, OnReservationClick, VTS_DISPATCH)
	ON_EVENT(CMonthDlg, IDC_MONTH_EVENT_CTRL, 1 /* ReservationClick */, OnReservationClick, VTS_DISPATCH)
	ON_EVENT(CMonthDlg, IDC_SINGLE_DAY_CTRL, 10 /* ReservationEndDrag */, OnReservationEndDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMonthDlg, IDC_SINGLE_DAY_CTRL, 2 /* ReservationAdded */, OnReservationAdded, VTS_DISPATCH)
	ON_EVENT(CMonthDlg, IDC_MONTH_EVENT_CTRL, 2 /* ReservationAdded */, OnReservationAdded, VTS_DISPATCH)
	ON_EVENT(CMonthDlg, IDC_SINGLE_DAY_CTRL, 12 /* ReservationEndResize */, OnReservationEndResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMonthDlg, IDC_SINGLE_DAY_CTRL, 6 /* ReservationMouseDown */, OnReservationMouseDown, VTS_DISPATCH VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CMonthDlg, IDC_MONTH_EVENT_CTRL, 6 /* ReservationMouseDown */, OnReservationMouseDown, VTS_DISPATCH VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CMonthDlg, IDC_SINGLE_DAY_CTRL, 9 /* ReservationDrag */, CNxSchedulerDlg::OnReservationDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMonthDlg, IDC_MONTH_EVENT_CTRL, 9 /* ReservationDrag */, CNxSchedulerDlg::OnReservationDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMonthDlg, IDC_SINGLE_DAY_CTRL, 11 /* ReservationResize */, CNxSchedulerDlg::OnReservationResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMonthDlg, IDC_MONTH_EVENT_CTRL, 11 /* ReservationResize */, CNxSchedulerDlg::OnReservationResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CMonthDlg, IDC_SINGLE_DAY_CTRL, 14 /* ReservationRecalculateColor */, CNxSchedulerDlg::OnReservationRecalculateColor, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()		

int CMonthDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxSchedulerDlg::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

int CMonthDlg::SetControlPositions()
{
	CNxSchedulerDlg::SetControlPositions();
	// Use the static frame position to determine the size of a day
	CRect rc, rcParent;
	GetDlgItem(IDC_FIRST_DAY_FRAME)->GetWindowRect(rc);
	GetParent()->GetWindowRect(rcParent);
	//GetDlgItem(IDC_FIRST_DAY_FRAME)->ClientToScreen(rc);

	m_startx = rc.left - rcParent.left;
	m_starty = rc.top - rcParent.top;
	m_daywidth = rc.Width();
	m_dayheight = rc.Height();

	// Determine the number of visible names
	m_nVisibleNamesPerDay = m_dayheight / m_nHeightAppointmentFont - 1;

	// Put day names directly above and aligned with calendar
	CWnd* pWnd = GetDlgItem(IDC_STATIC_SUN);
	CRect tmpRect;
	GetWindowRect(tmpRect);
	int y = (7 * m_dayheight)>>4;// 7/16 * dayheight
	pWnd->SetWindowPos(NULL, m_startx, y, m_daywidth, 17, SWP_NOZORDER);
	pWnd = GetDlgItem(IDC_STATIC_MON);
	pWnd->SetWindowPos(NULL, m_startx + m_daywidth, y, m_daywidth, 17, SWP_NOZORDER);
	pWnd = GetDlgItem(IDC_STATIC_TUE);
	pWnd->SetWindowPos(NULL, m_startx + m_daywidth * 2, y, m_daywidth, 17, SWP_NOZORDER);

	int wedwidth = 85;
	if (wedwidth < m_daywidth)
		wedwidth = m_daywidth;
	int wedoffset = (wedwidth - m_daywidth)>>1;
	pWnd = GetDlgItem(IDC_STATIC_WED);
	pWnd->SetWindowPos(NULL, m_startx + m_daywidth * 3 - wedoffset, y, wedwidth, 17, SWP_NOZORDER);
	pWnd = GetDlgItem(IDC_STATIC_THU);
	pWnd->SetWindowPos(NULL, m_startx + m_daywidth * 4, y, m_daywidth, 17, SWP_NOZORDER);
	pWnd = GetDlgItem(IDC_STATIC_FRI);
	pWnd->SetWindowPos(NULL, m_startx + m_daywidth * 5, y, m_daywidth, 17, SWP_NOZORDER);
	pWnd = GetDlgItem(IDC_STATIC_SAT);
	pWnd->SetWindowPos(NULL, m_startx + m_daywidth * 6, y, m_daywidth, 17, SWP_NOZORDER);

	// Put buttons to the left of weeks
	CButton* pButton = (CButton*)GetDlgItem(IDC_WEEK_LABEL_0);
	pButton->SetWindowPos(NULL, m_startx - 30, m_starty, 31, m_dayheight, SWP_NOZORDER);
	pButton = (CButton*)GetDlgItem(IDC_WEEK_LABEL_1);
	pButton->SetWindowPos(NULL, m_startx - 30, m_starty + m_dayheight, 31, m_dayheight, SWP_NOZORDER);
	pButton = (CButton*)GetDlgItem(IDC_WEEK_LABEL_2);
	pButton->SetWindowPos(NULL, m_startx - 30, m_starty + m_dayheight * 2, 31, m_dayheight, SWP_NOZORDER);
	pButton = (CButton*)GetDlgItem(IDC_WEEK_LABEL_3);
	pButton->SetWindowPos(NULL, m_startx - 30, m_starty + m_dayheight * 3, 31, m_dayheight, SWP_NOZORDER);
	pButton = (CButton*)GetDlgItem(IDC_WEEK_LABEL_4);
	pButton->SetWindowPos(NULL, m_startx - 30, m_starty + m_dayheight * 4, 31, m_dayheight, SWP_NOZORDER);
	pButton = (CButton*)GetDlgItem(IDC_WEEK_LABEL_5);
	pButton->SetWindowPos(NULL, m_startx - 30, m_starty + m_dayheight * 5, 31, m_dayheight, SWP_NOZORDER);

	
	CRect rect, rcEvent, rcSingleDay;

	//GetDlgItem(IDC_ACTIVE_DATE_APTCOUNT_LABEL)->SetWindowPos(NULL, m_startx + 7 * m_daywidth + (m_daywidth/10) + (m_daywidth*15)/8,
	//	y, (m_daywidth*10)/8, 17, SWP_NOZORDER);

	// CAH 10/16/01
	// TS 4/29/03: Top-align with calendar.

	//TES 12/17/2008 - PLID 32497 - Don't reposition all the singleday-related controls if they don't have access to the
	// Enterprise Edition, since we've hidden them.
	if(g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
		GetDlgItem(IDC_BTN_NEW_EVENT)->GetWindowRect(rect);
		ScreenToClient(rect);
		GetDlgItem(IDC_BTN_NEW_EVENT)->SetWindowPos(NULL, rect.left, m_starty, m_pSingleDayCtrl.GetTimeButtonWidth(), rect.Height(), SWP_NOZORDER);
		

		GetDlgItem(IDC_BTN_NEW_EVENT)->GetWindowRect(rect);
		ScreenToClient(rect);
		
		CWnd::FromHandle((HWND)m_pEventCtrl.GethWnd())->GetWindowRect(rcEvent);
		ScreenToClient(rcEvent);
		// Position read-only edit box just above
		GetDlgItem(IDC_EDIT_ACTIVE_SINGLEDAY_DAY)->SetWindowPos(NULL, m_startx + 7 * m_daywidth + (m_daywidth/10),
			y, rect.Width() + rcEvent.Width(), 17, SWP_NOZORDER);
		
		CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd())->GetWindowRect(rcSingleDay);
		ScreenToClient(rcSingleDay);
		
		CWnd::FromHandle((HWND)m_pEventCtrl.GethWnd())->SetWindowPos(NULL, rect.right, rect.top, rcSingleDay.Width() - m_pEventCtrl.GetTimeButtonWidth(), rect.Height(), SWP_NOZORDER);
		//

		// TES 2/19/2004: Let's make sure the count label doesn't overlap the singleday (can happen in 800x600).
		CRect rCount;
		GetDlgItem(IDC_ACTIVE_DATE_APTCOUNT_LABEL)->GetWindowRect(rCount);
		ScreenToClient(rCount);
		CRect rSelf;
		GetClientRect(rSelf);
		// TS 4/29/04: 
		if((rect.bottom+7) + (m_dayheight * 6 - rect.Height() - 1) + rCount.Height() > rSelf.bottom) {
			CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd())->SetWindowPos(NULL, rcSingleDay.left, rect.bottom + 7, rcSingleDay.Width(), m_dayheight * 6 - rect.Height() - 1 - rCount.Height(), SWP_NOZORDER);
		}
		else {
			CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd())->SetWindowPos(NULL, rcSingleDay.left, rect.bottom + 7, rcSingleDay.Width(), m_dayheight * 6 - rect.Height() - 1, SWP_NOZORDER);
		}
	}
	else {
		//TES 12/17/2008 - PLID 32497 - Since the singleday is invisible, align the appointment count with the left 
		// edge of the month grid.
		CRect rCount;
		GetDlgItem(IDC_ACTIVE_DATE_APTCOUNT_LABEL)->GetWindowRect(rCount);
		ScreenToClient(rCount);
		rCount.SetRect(m_startx, rCount.top, m_startx+rCount.Width(), rCount.bottom);
		GetDlgItem(IDC_ACTIVE_DATE_APTCOUNT_LABEL)->MoveWindow(rCount);
	}
	
	// (a.walling 2009-02-04 10:14) - PLID 31956 - No longer used
	//SetRgnBg();//should fix background drawing and flickering issues
	Invalidate(FALSE);

	return 1;
}

void CMonthDlg::EnsureButtonLabels()
{
}

COleDateTime CMonthDlg::GetWorkingDate(int nDay /* = 0 */)
{
	return GetActiveDate() + (COleDateTimeSpan)nDay;
}

void CMonthDlg::OnMoveMonthBack() 
{
	COleDateTime newDate = GetActiveDate();;
//	COleDateTimeSpan spanMonth;
//	spanMonth.SetDateTimeSpan(GetDaysInMonth(newDate.GetMonth() - 1, newDate.GetYear()), 0, 0, 0);
//	newDate -= spanMonth;
//	m_dtPriorDate -= spanMonth;
	int year = newDate.GetYear();
	int month = newDate.GetMonth() - 1;
	int day = newDate.GetDay();
	if (month == 0)
	{	month = 12;
		year--;
	}
	if (day > GetDaysInMonth(month, year))
		day = GetDaysInMonth(month, year);
	newDate.SetDateTime(year, month, day, newDate.GetHour(), 
		newDate.GetMinute(), newDate.GetSecond());

	SetActiveDate(newDate);
	OnSetDateSelCombo();
}

void CMonthDlg::OnMoveMonthForward() 
{
	COleDateTime newDate = GetActiveDate();;
//	COleDateTimeSpan spanMonth;
//	spanMonth.SetDateTimeSpan(GetDaysInMonth(newDate.GetMonth(), newDate.GetYear()), 0, 0, 0);
//	newDate += spanMonth;
//	m_dtPriorDate += spanMonth;
	int year = newDate.GetYear();
	int month = newDate.GetMonth() + 1;
	int day = newDate.GetDay();
	if (month == 13)
	{	month = 1;
		year++;
	}
	if (day > GetDaysInMonth(month, year))
		day = GetDaysInMonth(month, year);
	newDate.SetDateTime(year, month, day, newDate.GetHour(), 
		newDate.GetMinute(), newDate.GetSecond());

	SetActiveDate(newDate);
	OnSetDateSelCombo();
}

void CMonthDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// (j.luckoski 2012-05-07 11:44) - PLID 11597 - Grab preferences for showing cancelled appts
	m_nCancelledAppt = GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true);
	// (j.luckoski 2012-06-12 15:58) - PLID 11597 - Did not default to 24
	m_nDateRange = GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true);
	m_nCancelColor = GetRemotePropertyInt("SchedCancelledApptColor", RGB(192,192,192), 0, GetCurrentUserName(), true);
	// Don't allow user to edit single day contents
	//m_pSingleDayCtrl.SetReadOnly(TRUE);

	m_dtSingleDayDate = GetActiveDate();
	if (!m_pParent) return;	
	if (!m_bAllowUpdate && !m_bNeedUpdate) return;
	// (c.haag 2010-06-23 10:10) - PLID 39210 - Don't update if a reservation is locked by
	// a right-click action or the reservation window is open.
	// (c.haag 2010-08-27 11:38) - PLID 40108 - Moved the logic into one function
	if (!IsSafeToUpdateView()) return;

	// Fill the single day control with stuff. If a field is
	// dirty, then this will also redraw the calendar names
	//bool bTempNeed = m_bNeedUpdate;
	UpdateSingleDayCtrl();

	// CAH 11/2/01: This is legacy code from when UpdateView would
	// be called much more frequently than it needed to be. After
	// all, when you think of UpdateView, it really should mean
	// the entire view. This was also necessary to comment out
	// because the calendar would not update if you cancelled or
	// changed appointments in the singleday.

//	if (m_dtLastUpdate.GetMonth() != GetActiveDate().GetMonth() ||
//		m_dtLastUpdate.GetYear() != GetActiveDate().GetYear() ||
//		bTempNeed) {
		UpdateCalendarNames();
//	}

	m_dtLastUpdate = GetActiveDate();
}

void CMonthDlg::OnWeekLabel0() 
{
 	COleDateTime date = GetActiveDate();
	int activemonth = date.GetMonth();
	date -= COleDateTimeSpan(date.GetDay()-1,0,0,0);
	int weekday = date.GetDayOfWeek();
	date -= COleDateTimeSpan(weekday-1, 0,0,0);
	m_dtPriorDate = date;
	SetActiveDate(date);
	if (m_pParent) m_pParent->SwitchToTab(CSchedulerView::sttWeekView);
}

void CMonthDlg::OnWeekLabel1() 
{
 	COleDateTime date = GetActiveDate();
	int activemonth = date.GetMonth();
	date -= COleDateTimeSpan(date.GetDay()-1,0,0,0);
	int weekday = date.GetDayOfWeek();
	date -= COleDateTimeSpan(weekday-1 - 7, 0,0,0);
	m_dtPriorDate = date;
	SetActiveDate(date);
	if (m_pParent) m_pParent->SwitchToTab(CSchedulerView::sttWeekView);
}

void CMonthDlg::OnWeekLabel2() 
{
 	COleDateTime date = GetActiveDate();
	int activemonth = date.GetMonth();
	date -= COleDateTimeSpan(date.GetDay()-1,0,0,0);
	int weekday = date.GetDayOfWeek();
	date -= COleDateTimeSpan(weekday-1 - 14, 0,0,0);
	m_dtPriorDate = date;
	SetActiveDate(date);
	if (m_pParent) m_pParent->SwitchToTab(CSchedulerView::sttWeekView);
}

void CMonthDlg::OnWeekLabel3() 
{
 	COleDateTime date = GetActiveDate();
	int activemonth = date.GetMonth();
	date -= COleDateTimeSpan(date.GetDay()-1,0,0,0);
	int weekday = date.GetDayOfWeek();
	date -= COleDateTimeSpan(weekday-1 - 21, 0,0,0);
	m_dtPriorDate = date;
	SetActiveDate(date);
	if (m_pParent) m_pParent->SwitchToTab(CSchedulerView::sttWeekView);
}

void CMonthDlg::OnWeekLabel4() 
{
 	COleDateTime date = GetActiveDate();
	int activemonth = date.GetMonth();
	date -= COleDateTimeSpan(date.GetDay()-1,0,0,0);
	int weekday = date.GetDayOfWeek();
	date -= COleDateTimeSpan(weekday-1 - 28, 0,0,0);
	m_dtPriorDate = date;
	SetActiveDate(date);
	if (m_pParent) m_pParent->SwitchToTab(CSchedulerView::sttWeekView);
}

void CMonthDlg::OnWeekLabel5() 
{
 	COleDateTime date = GetActiveDate();
	int activemonth = date.GetMonth();
	date -= COleDateTimeSpan(date.GetDay()-1,0,0,0);
	int weekday = date.GetDayOfWeek();
	date -= COleDateTimeSpan(weekday-1 - 35, 0,0,0);
	m_dtPriorDate = date;
	SetActiveDate(date);
	if (m_pParent) m_pParent->SwitchToTab(CSchedulerView::sttWeekView);
}

// Do not call CNxSchedulerDlg::OnPaint() for painting messages
void CMonthDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CBrush redbrush(RGB(255, 64, 64));
	CPen pen(PS_SOLID, 1, RGB(0,0,0));
	CPen penThick(PS_SOLID, 3, RGB(0,0,0));
	CPen penThickRed(PS_SOLID, 3, RGB(255,64,64));
	CPen* pOldPen = dc.SelectObject(&pen);
	dc.SetTextColor(RGB(0,0,0));
	CFont* pOldFont;
	pOldFont = dc.SelectObject(&m_fntAppointment);
	CRect rcSelectedDay, rcToday;
	BOOL bDrawToday = FALSE, bDrawSelectedDay = FALSE;

 	COleDateTime date = GetActiveDate();
	int activemonth = date.GetMonth();
	date -= COleDateTimeSpan(date.GetDay()-1,0,0,0);
	int weekday = date.GetDayOfWeek();
	date -= COleDateTimeSpan(weekday-1, 0,0,0);

	COleDateTime dtToday = COleDateTime::GetCurrentTime();
	dtToday.SetDate(dtToday.GetYear(), dtToday.GetMonth(), dtToday.GetDay());

	// Set this to FALSE, and then if any visible day doesn't 
	// paint all its appointments, this will be set back to TRUE
	m_bAllowScrollDown = FALSE;

	if (GetDlgItem(IDC_SPIN_NAME_OFFSETS)->IsWindowEnabled()) {
		GetDlgItem(IDC_SPIN_NAME_OFFSETS)->EnableWindow(FALSE);
	}

	//
	// (c.haag 2006-04-12 16:05) - PLID 20081 - Clear all drag and drop regions
	//
	for (int i=0; i < 31; i++) {
		m_aDragDropRgn[i].RemoveAll();
	}


	// Loop through each day
	for (int y=m_starty; y < m_starty + m_dayheight*6; y += m_dayheight) {
		for (int x=m_startx; x < m_startx + m_daywidth*7; x += m_daywidth) {
			CRect rc(x, y, x + m_daywidth + 1, y + m_dayheight + 1);
			//
			// If this is today, or the activate date, assign the rectangles
			// accordingly
			//
			if (date == GetActiveDate())
			{
				rcSelectedDay = rc;
				bDrawSelectedDay = TRUE;
			}
			if (date == dtToday)
			{
				rcToday = rc;
				bDrawToday = TRUE;
			}
			//
			// (c.haag 2006-04-11 13:37) - PLID 20081 - All the screen painting now takes
			// place in PaintDay
			//
			PaintDay(dc, rc, date);
			date += COleDateTimeSpan(1, 0,0,0);
		}
	}

	// Draw a thin red frame around today
	if (bDrawToday) {
		dc.FrameRect(&rcToday, &redbrush);
	}

	// Draw a thick frame around the active day
	if (bDrawSelectedDay)
	{
		CRect& r = rcSelectedDay;
		r.DeflateRect(1, 1, 2, 2);//We don't want this rect to extend beyond today's rectangle.
		POINT pt[5] = 
		{
			r.left, r.top,
			r.right, r.top,
			r.right, r.bottom,
			r.left, r.bottom,
			r.left, r.top
		};

		if (GetActiveDate() == dtToday)
			dc.SelectObject(&penThickRed);
		else
			dc.SelectObject(&penThick);

		dc.Polyline(pt, 5);
	}
	dc.SelectObject(pOldPen);
	dc.SelectObject(pOldFont);
}

void CMonthDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	try {
		CNxSchedulerDlg::OnLButtonDown(nFlags, point);
		SetFocus();

		// (c.haag 2009-12-22 14:02) - PLID 28977 - The user can now click on the count label to
		// configure how counts are calculated
		DoClickHyperlink(nFlags, point);

		//
		// Get date and position of that day square
		//
		int x,y;
		COleDateTime date = GetCalendarDateUnderMouse(x, y);
		if (x == -1) return;
		if (GetActiveDate().GetMonth() != date.GetMonth()) return;
		CArray<_DRAGDROPRGN, _DRAGDROPRGN>* pddrgn = &m_aDragDropRgn[date.GetDay()-1];
		if (NULL == pddrgn) return;

		//
		// (c.haag 2006-04-12 15:48) - PLID 20081 - Now determine which appt was pressed
		// on given the mouse coordinates.
		//
		CPoint pt;
		CRect rcWnd;
		GetCursorPos(&pt);
		GetWindowRect(rcWnd);
		pt.x -= rcWnd.left;
		pt.y -= rcWnd.top;
		int nIndex = 0;
		int nVisibleNames = pddrgn->GetSize();
		BOOL bFound = FALSE;
		for (nIndex = 0; nIndex < nVisibleNames && !bFound;) {
			int nRectTop = pddrgn->GetAt(nIndex).top;
			int nRectBottom = pddrgn->GetAt(nIndex).bottom;
			if (pt.y >= nRectTop && pt.y <= nRectBottom) {
				bFound = TRUE;
			} else {
				nIndex++;
			}
		}

		if (!bFound)
			return;

		CString strVisibleName = m_astrVisibleNames[ date.GetDay()-1 ][ pddrgn->GetAt(nIndex).nNameIndex ];
		m_strDragInfo = strVisibleName + FormatDateTimeForSql(date, dtoDate) + "|";
		m_strDragName = strVisibleName;
		m_clrDragColor = (COLORREF)m_adwVisibleColors[ date.GetDay()-1 ][ pddrgn->GetAt(nIndex).nNameIndex ];
		m_iDragMonth = date.GetMonth();

		// Bind cursor to calendar
		CRect rcbound;
		GetClientRect(rcWnd);
		rcbound.left = m_startx;
		rcbound.top = rcWnd.top;
		rcbound.right = m_startx + m_daywidth * 7;
		rcbound.bottom = rcWnd.bottom;
		ClientToScreen(rcbound);
		ClipCursor(rcbound);
	}
	NxCatchAll(__FUNCTION__);
}

void CMonthDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	try {
		CNxSchedulerDlg::OnLButtonUp(nFlags, point);

		long nApptIDToUpdate = -1;

		// Free mouse cursor
		ClipCursor(NULL);

		// Stop the single day refresh timers
		KillTimer(ID_MONTH_VIEW_TIMER);
		KillTimer(ID_MONTH_VIEW_SCROLL_TIMER);

		// Get date and position of that day square
		int x,y;
		COleDateTime dtNewDate = GetCalendarDateUnderMouse(x, y);

		if (x == -1) {
			m_strDragInfo.Empty();
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			return;
		}

		CString strSrcDate = GetSubString(m_strDragInfo, 8);
		COleDateTime dtTmp;
		COleDateTime dtSrcDate;
		dtTmp = dtNewDate;
		dtTmp.SetDateTime(dtTmp.GetYear(), dtTmp.GetMonth(), dtTmp.GetDay(), 0, 0, 0);
		dtSrcDate.ParseDateTime(strSrcDate);
		
		// Go to single day view if a name wasn't dragged
		if (m_strDragInfo.GetLength() == 0 || dtTmp == dtSrcDate) {
			m_strDragInfo.Empty();
			if (m_pParent) {
				m_dtPriorDate = dtNewDate;
				SetActiveDate(dtNewDate);
				m_pParent->SwitchToTab(CSchedulerView::sttOneDayView);
			}
			return;
		}
		// Abort if mouse was released outside of calendar
		else if (x == -1 || GetActiveDate().GetMonth() != dtNewDate.GetMonth()) {
			m_strDragInfo.Empty();
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			return;
		}
		
		/* Move the patient by going in the reservations set and
		changing the day of the appointment. Be sure UpdateView()
		accompanies this.*/
		BeginWaitCursor();
		CString strOldDate;
		CString strReservationID;
		// Build search string
		// String like this: "B|Spam|Purpose|234|8:00:00 AM|9:30:00 AM|4758|"
		x = m_strDragInfo.Find('|');
		m_strDragInfo.SetAt(x, ' ');
		y = m_strDragInfo.Find('|');
		m_strDragInfo.SetAt(y, ' ');
		x = m_strDragInfo.Find('|');
		m_strDragInfo.SetAt(x, ' ');
		y = m_strDragInfo.Find('|');
		m_strDragInfo.SetAt(y, ' ');
		x = m_strDragInfo.Find('|');
		m_strDragInfo.SetAt(x, ' ');
		y = m_strDragInfo.Find('|');
		m_strDragInfo.SetAt(y, ' ');
		x = m_strDragInfo.Find('|');
		m_strDragInfo.SetAt(x, ' ');
		strReservationID = m_strDragInfo.Mid(y+1, (x-y-1));
		y = m_strDragInfo.Find('|');
		m_strDragInfo.SetAt(y, ' ');
		strOldDate = m_strDragInfo.Mid(x+1, (y-x-1));

		// See if the mouse was released on the same day
		if (strOldDate == FormatDateTimeForSql(dtNewDate, dtoDate)) {
			m_strDragInfo.Empty();
			if (m_pParent) {
				m_dtPriorDate = dtNewDate;
				SetActiveDate(dtNewDate);
				m_pParent->SwitchToTab(CSchedulerView::sttOneDayView);
			}
			return;
		}

		if(!(GetAsyncKeyState(VK_SHIFT) & 0x80000000)) {
			// Prompt user for move confirm
			if (IDYES == MessageBox("Are you sure you wish to move this appointment?",
				"Move Appointments", MB_YESNO)) {
				try {			

					// CAH 4/16/01
					long nApptID = atol(strReservationID);
					_RecordsetPtr pAppt = AppointmentGrab(nApptID);
					if (pAppt) {
						// Get the appointment's current times
						COleDateTime dtFromStart = pAppt->Fields->Item["StartTime"]->Value;
						COleDateTime dtFromEnd = pAppt->Fields->Item["EndTime"]->Value;
						COleDateTime dtFromArrival = pAppt->Fields->Item["ArrivalTime"]->Value; // (z.manning, 03/20/2007) - PLID 25280

						// Calculate the new start date, start time, and end time
						COleDateTime dtToDate, dtToStart, dtToEnd, dtToArrival;
						dtToDate.SetDate(dtNewDate.GetYear(), dtNewDate.GetMonth(), dtNewDate.GetDay());
						dtToStart.SetDateTime(dtNewDate.GetYear(), dtNewDate.GetMonth(), dtNewDate.GetDay(), dtFromStart.GetHour(), dtFromStart.GetMinute(), dtFromStart.GetSecond());
						dtToEnd.SetDateTime(dtNewDate.GetYear(), dtNewDate.GetMonth(), dtNewDate.GetDay(), dtFromEnd.GetHour(), dtFromEnd.GetMinute(), dtFromEnd.GetSecond());
						dtToArrival.SetDateTime(dtNewDate.GetYear(), dtNewDate.GetMonth(), dtNewDate.GetDay(), dtFromArrival.GetHour(), dtFromArrival.GetMinute(), dtFromArrival.GetSecond()); // (z.manning, 03/20/2007) - PLID 25280

						//DRT 7/3/03 - Removed a comment here that said "hold these temp. in our recordset (it's not used for updating, only validation). 
						//		That comment was wrong, afaik, as long as the AppointmentUpdate function has existed, it's always updated
						//		the recordset (not just validated it).
						pAppt->Fields->Item["Date"]->Value = _variant_t(dtToDate, VT_DATE);
						pAppt->Fields->Item["StartTime"]->Value = _variant_t(dtToStart, VT_DATE);
						pAppt->Fields->Item["EndTime"]->Value = _variant_t(dtToEnd, VT_DATE);
						// (z.manning, 03/20/2007) - PLID 25280 - Make sure we update arrival time as well to avoid bad data.
						pAppt->Fields->Item["ArrivalTime"]->Value = _variant_t(dtToArrival, VT_DATE);

						//DRT 4/14/03 - Appointment linking
						if(dtToStart != dtFromStart)
						{
							long nMinutes = GetSafeTotalMinuteDiff(dtToStart, dtFromStart);

							//something changed, do the appt linking prompt thing
							AttemptAppointmentLinkAction(atoi(strReservationID), nMinutes);
						}

						long nPatID = AdoFldLong(pAppt, "PatientID", 0);
						_RecordsetPtr rsPat = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS Name FROM PersonT WHERE PersonT.ID = %li", nPatID);
						CString strPatName = rsPat->eof ? "" : AdoFldString(rsPat, "Name");
						rsPat->Close();

						// (j.jones 2014-12-02 10:00) - PLID 64182 - added pParentWnd
						// (j.jones 2014-12-19 12:50) - PLID 64182 - this can now auto-change & audit the dates
						bool bAuditDates = true;
						if (AppointmentUpdate(this, pAppt, bAuditDates)) {

							nApptIDToUpdate = nApptID;

							if (bAuditDates) {
								// (c.haag 2004-01-30 17:37) - Only audit if it worked!
								//DRT 2/27/03 - Auditing!
								// (z.manning, 03/20/2007) - 25238 - Audit start time, end time, and arrival time since they 
								// all changed to be consistent with the rest of the program.
								long nAuditID = BeginNewAuditEvent();
								CString strOldStart = FormatDateTimeForInterface(dtFromStart);
								CString strOldEnd = FormatDateTimeForInterface(dtFromEnd);
								CString strOldArrival = FormatDateTimeForInterface(dtFromArrival);
								CString strNewStart = FormatDateTimeForInterface(dtToStart);
								CString strNewEnd = FormatDateTimeForInterface(dtToEnd);
								CString strNewArrival = FormatDateTimeForInterface(dtToArrival);
								long nResID = atol(strReservationID);
								if (strNewStart != strOldStart) { //These are formatted in the same way above, so they should match if the same
									AuditEvent(nPatID, strPatName, nAuditID, aeiApptStartTime, nResID, strOldStart, strNewStart, aepHigh);
								}
								if (strNewEnd != strOldEnd) {
									AuditEvent(nPatID, strPatName, nAuditID, aeiApptEndTime, nResID, strOldEnd, strNewEnd, aepHigh);
								}
								if (strNewArrival != strOldArrival) {
									AuditEvent(nPatID, strPatName, nAuditID, aeiApptArrivalTime, nResID, strOldArrival, strNewArrival, aepMedium);
								}
								//end auditing
							}

							m_bNeedUpdate = true;
						} else {
							EndWaitCursor();
							m_strDragInfo.Empty();
							return;
						}
					} else {
						AfxThrowNxException("Could not drag and drop appointment");
					}

				} NxCatchAll("CMonthDlg::OnLButtonUp");
				//UpdateView();
				//UpdateCalendarNames();

				// Change the names only for those two days
				if (m_iDragMonth == GetActiveDate().GetMonth()) {
					CString strOldData = m_strDragInfo;
					for (int i=0; i < 31; i++) {
						for (int j=0; j < m_astrVisibleNames[i].GetSize(); j++) {
							CString strVisibleName = m_astrVisibleNames[i][j];
							if (strVisibleName == m_strDragName) {
								m_astrVisibleNames[i].RemoveAt(j);
								m_adwVisibleColors[i].RemoveAt(j);//bvb
								i=31;
								break;
							}
						}
					}
				}
				if (dtNewDate.GetMonth() == GetActiveDate().GetMonth()) {
					m_astrVisibleNames[ dtNewDate.GetDay()-1 ].Add( m_strDragName );
					m_adwVisibleColors[ dtNewDate.GetDay()-1 ].Add( (DWORD)m_clrDragColor );
				}

				// Redraw calendar
				CRect rc(m_startx, m_starty, m_startx + m_daywidth*7, m_starty + m_dayheight*6);
				InvalidateRect(&rc, FALSE);

				// Change the edit text above the single day control so
				// it won't match with the date format in the control update.
				// That way, they won't match in the comparison and the
				// control will update.
				CString tmpStr;
				CWnd* pWnd = GetDlgItem(IDC_EDIT_ACTIVE_SINGLEDAY_DAY);
				pWnd->SetWindowText("");
				UpdateSingleDayCtrl();

			}
		}

		EndWaitCursor();
		m_strDragInfo.Empty();

		if (nApptIDToUpdate != -1) {
			UpdateViewBySingleAppt(nApptIDToUpdate);
		}
	}
	NxCatchAll(__FUNCTION__);
}

CString CMonthDlg::GetWhereClause()
{
	// Build SQL to span current month
	// (j.luckoski 2012-05-07 11:45) - PLID 11597 - Alter query  to show cancelled appts if they were cancelled within a certain specified range
	CString strSqlExtra = "";
	if(m_nCancelledAppt == 1) {
		strSqlExtra.Format(" AND (AppointmentsT.CancelledDate IS NULL OR (AppointmentsT.CancelledDate >= DATEADD(hh, -%li, AppointmentsT.StartTime))) ", m_nDateRange);
	} else { 
		strSqlExtra.Format(" AND AppointmentsT.Status <> 4 "); 	
	}
	CString strSqlWhere;
	COleDateTime date = GetActiveDate();
	if (date.GetMonth() < 12) {
		// CAH 4/16/01
		strSqlWhere.Format("((AppointmentsT.Date >= '%d/1/%d') AND (AppointmentsT.Date < '%d/1/%d') AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID = %d) %s) ",
			date.GetMonth(), date.GetYear(), date.GetMonth()+1,
			date.GetYear(), m_pParent->GetActiveResourceID(), strSqlExtra);
	}
	else {
		// CAH 4/16/01
		strSqlWhere.Format("((AppointmentsT.Date >= '%d/1/%d') AND (AppointmentsT.Date < '%d/1/%d') AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE ResourceID = %d) %s) ",
			date.GetMonth(), date.GetYear(), 1,
			date.GetYear()+1, m_pParent->GetActiveResourceID(), strSqlExtra);
	}

	long nSetId = GetPurposeSetId();
	if (nSetId != -1) {
		CString strAddWhere;
		if (nSetId == -2)
		{
			CString strIds = m_strMultiTypeIds;
			CString strList = "";
			long nComma = strIds.Find(",");
			while(nComma > -1) {
				long nID = atoi(strIds.Left(nComma));
				CString str;
				str.Format("OR AptTypeID = %d ", nID);
				strList += str;
				strIds = strIds.Right(strIds.GetLength() - nComma - 1);
				nComma = strIds.Find(",");
			}

			if(!strList.IsEmpty()) {
				strList = strList.Right(strList.GetLength() - 3);	//take off the opening or
				strAddWhere += strList;
			}
			else {
				//there are no items in the list, but it's selected... this shouldn't be possible
				ASSERT(FALSE);
			}
		}
		else
		{
			// Generate the additional filter criterion based on the purpose set filter
			strAddWhere.Format("AptTypeID = %li", nSetId);
		}
		// Append the additional criterion to the current filter
		if (strSqlWhere.IsEmpty()) {
			strSqlWhere = strAddWhere;
		} else {
			strSqlWhere = "(" + strSqlWhere + ") AND (" + strAddWhere + ")";
		}
	}
	long nPurpId = GetPurposeId();
	if (nPurpId != -1)
	{
		CString strAddWhere = "AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE ";
		if(nPurpId == -2) {
			//TES 10/9/03 - multiple purposes filter ("borrowed" from the multiresourcedlg)
			CString strIds = m_strMultiPurposeIds;
			CString strList = "";
			long nComma = strIds.Find(",");
			while(nComma > -1) {
				long nID = atoi(strIds.Left(nComma));
				CString str;
				str.Format("OR PurposeID = %li ", nID);
				strList += str;
				strIds = strIds.Right(strIds.GetLength() - nComma - 1);
				nComma = strIds.Find(",");
			}

			if(!strList.IsEmpty()) {
				strList = strList.Right(strList.GetLength() - 3);	//take off the opening or
				strAddWhere += strList + ")";
			}
			else {
				//there are no items in the list, but it's selected... this shouldn't be possible
				ASSERT(FALSE);
			}
		}
		else {
			// Generate the additional filter criterion based on the purpose set filter
			strAddWhere.Format("AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE PurposeID = %li)", nPurpId);
		}
		// Append the additional criterion to the current filter
		if (strSqlWhere.IsEmpty()) {
			strSqlWhere = strAddWhere;
		} else {
			strSqlWhere = "(" + strSqlWhere + ") AND (" + strAddWhere + ")";
		}
	}
	//TES 6/21/2010 - PLID 21341 - Add in the location filter.
	long nLocationId = GetLocationId();
	if (nLocationId != -1) {
		CString strAddWhere;
		// (d.singleton 2012-07-02 16:05) - PLID 47473 be able to choose multiple locations
		if(nLocationId == -2) {			
			CString strIds = m_strMultiLocationIds;
			CString strList = "";
			long nComma = strIds.Find(",");
			while(nComma > -1) {
				long nID = atoi(strIds.Left(nComma));
				CString str;
				str.Format("OR LocationID = %li ", nID);
				strList += str;
				strIds = strIds.Right(strIds.GetLength() - nComma - 1);
				nComma = strIds.Find(",");
			}

			if(!strList.IsEmpty()) {
				strList = strList.Right(strList.GetLength() - 3);	//take off the opening or
				strAddWhere += strList;
			}
			else {
				//there are no items in the list, but it's selected... this shouldn't be possible
				ASSERT(FALSE);
			}
		}
		else {
			// Generate the additional filter criterion based on the location filter
			strAddWhere.Format("LocationID = %li", nLocationId);
		}
		// Append the additional criterion to the current filter
		if (strSqlWhere.IsEmpty()) {
			strSqlWhere = strAddWhere;
		} else {
			strSqlWhere = "(" + strSqlWhere + ") AND (" + strAddWhere + ")";
		}
	}
	return strSqlWhere;
}

void CMonthDlg::UpdateCalendarNames()
{
	BeginWaitCursor();

	_RecordsetPtr prsMonthView, prsEvents;
	CString strFilter;

	BOOL bReadPurposes = GetRemotePropertyInt("SchedulerShowMonthViewPurposes", 0, 0, GetCurrentUserName(), false);

	// Clear all visible names from string arrays
	for (int i=0; i < 31; i++) {
		m_astrVisibleNames[i].RemoveAll();
		m_adwVisibleColors[i].RemoveAll();
	}

	// Clear the events array
	m_aEvents.RemoveAll();

	// Build SQL to span current month
	CString strSqlWhere = GetWhereClause();

	// Put the WHERE clause into the query

	// Open databases
	FieldsPtr fldsMonth;
	FieldPtr fldResID, fldDate, fldStartTime, fldEndTime, fldPatientID, 
		fldFirstName, fldLastName, fldAptTypeID, fldAptTypeName, fldAptTypeColor,
		fldEventName, fldAptPurposeName;
	try {
		// (z.manning, 02/13/2008) - PLID 28909 - Need to handle attendance appointments
		CString strAttendanceUnion;
		if(IsNexTechInternal())
		{
			COleDateTime dtDate = GetActiveDate();
				
			// (z.manning 2008-10-09 17:33) - PLID 31645 - Also use ResourceUserLinkT as a way to link
			// attendance appointments to resources.
			// (z.manning 2008-11-13 12:56) - PLID 31831 - Added paid/unpaid time off
			// (z.manning 2010-07-30 12:03) - PLID 39916 - Don't show attendance notes in the scheduler
			strAttendanceUnion = FormatString(
				"UNION "
				"SELECT -1 AS ResID, Date "
				"	, Username + ' - ' + COALESCE(AptTypeT.Name, '<Time Off>') "
				"	, Date, Date, -25, First, Last, AptTypeT.ID, AptTypeT.Color, AptTypeT.Name, '' "
				"FROM AttendanceAppointmentsT "
				"LEFT JOIN UsersT ON AttendanceAppointmentsT.UserID = UsersT.PersonID "
				"LEFT JOIN PersonT ON UsersT.PersonID = PersonT.ID "
				"LEFT JOIN AptTypeT ON dbo.GetAttendanceAppointmentTypeID(AttendanceAppointmentsT.ID) = AptTypeT.ID "
				"WHERE (Vacation > 0 OR Sick > 0 OR Other > 0 OR PaidTimeOff > 0 OR UnpaidTimeOff > 0) "
				"	AND (DATEPART(year, AttendanceAppointmentsT.Date) = %li AND DATEPART(month, AttendanceAppointmentsT.Date) = %li) "
				"	AND (AttendanceAppointmentsT.UserID IN (SELECT UserID FROM UserDepartmentLinkT WHERE DepartmentID IN (SELECT DepartmentID FROM DepartmentResourceLinkT WHERE ResourceID = %li)) "
				"		OR AttendanceAppointmentsT.UserID IN (SELECT UserID FROM ResourceUserLinkT WHERE ResourceID = %li)) "
				"	%s "
				, dtDate.GetYear(), dtDate.GetMonth(), m_pParent->GetActiveResourceID(), m_pParent->GetActiveResourceID(), GetExtraFilterAttendance()
				);
		}
		//TES 5/24/2004: MAKE SURE that this query always orders exactly the same as the one in UpdatePurposes().
		// (j.luckoski 2012-05-07 11:47) - PLID 11597 - If appt is cancelled than make the font the color specified in preferences.
		prsMonthView = CreateRecordset(
			"SELECT AppointmentsT.ID AS ResID, AppointmentsT.Date, AppointmentsT.Notes, AppointmentsT.StartTime, "
			"	AppointmentsT.EndTime, AppointmentsT.PatientID, PersonT.First, PersonT.Last, "
			"	AppointmentsT.AptTypeID, CASE WHEN AppointmentsT.Status = 4 THEN %li ELSE AptTypeT.Color END AS AptTypeColor, AptTypeT.Name AS AptTypeName, "
			"	%s AS AptPurposeName "
			"FROM PersonT "
			"INNER JOIN AppointmentsT ON PersonT.ID = AppointmentsT.PatientID "
			"LEFT OUTER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE %s " 
			"%s "
			"ORDER BY 4, 1 ",
			m_nCancelColor, (bReadPurposes) ? "dbo.GetPurposeString(AppointmentsT.ID)" : "''",
			strSqlWhere, strAttendanceUnion);

		// Get the fields collection
		fldsMonth = prsMonthView->Fields;
		// Load all the field references
		fldResID = fldsMonth->Item["ResID"];
		fldDate = fldsMonth->Item["Date"];
		fldStartTime = fldsMonth->Item["StartTime"];
		fldEndTime = fldsMonth->Item["EndTime"];
		fldPatientID = fldsMonth->Item["PatientID"];
		fldFirstName = fldsMonth->Item["First"];
		fldLastName = fldsMonth->Item["Last"];
		fldAptTypeID = fldsMonth->Item["AptTypeID"];
		fldAptTypeName = fldsMonth->Item["AptTypeName"];
		fldAptTypeColor = fldsMonth->Item["AptTypeColor"];
		fldEventName = fldsMonth->Item["Notes"];
		fldAptPurposeName = (bReadPurposes) ? fldsMonth->Item["AptPurposeName"] : NULL;
	} NxCatchAllCall("CMonthDlg::UpdateCalendarNames 1", return);

	long count = 0;
	while (!prsMonthView->eof) {
		CString strFirstName;
		CString strLastName;
		CString strStartTime;
		CString strEndTime;
		int iDay;
		BOOL bIsEvent;
		COleDateTime dtFirst, dtLast;

		try {
			// Get the appointment ID
			int iCurResID = AdoFldLong(fldResID);

			// Get the appointment start and end time
			dtFirst = AdoFldDateTime(fldStartTime);
			dtLast = AdoFldDateTime(fldEndTime);

			// Calculate if it's an event
			if (!dtFirst.GetHour() && !dtFirst.GetMinute() && !dtFirst.GetSecond() &&
				!dtLast.GetHour() && !dtLast.GetMinute() && !dtLast.GetSecond())
			{
				bIsEvent = TRUE;
			}
			else
			{
				bIsEvent = FALSE;
			}

			// (c.haag 2003-10-03 15:32) - Handle events here
			if (bIsEvent)
			{
				_MONTHEVENT event;
				BuildCalendarEvent(event, fldResID, fldStartTime, fldEndTime, fldEventName, fldAptTypeColor);
				m_aEvents.Add(event);
			}
			else
			{
				// Get day of appointment
				COleDateTime dt = AdoFldDateTime(fldDate);
				iDay = dt.GetDay();

				CString strTmp = BuildCalendarElement(fldResID, fldStartTime, fldEndTime,
					fldPatientID, fldFirstName, fldLastName, fldAptTypeID, fldAptTypeName,
					fldAptTypeColor, fldAptPurposeName);

				// Add to our name and color arrays
				m_astrVisibleNames[iDay-1].Add(strTmp);
				m_adwVisibleColors[iDay-1].Add(AdoFldLong(fldAptTypeColor, 0));

				count++;
			}
			prsMonthView->MoveNext();
		} NxCatchAllCall("CMonthDlg::UpdateCalendarNames 2", break);
	}	

	//TES 1/5/2012 - PLID 47310 - Now we need to load all the location templates for the month, so that we can highlight the days accordingly.
	try {
		m_ResourceAvailLocations_Month.ClearAll();
		long nLocationID = GetLocationId();
		//TES 1/5/2012 - PLID 47310 - Only bother with this if we're filtering on a location, and the preference is turned on.
		// (d.singleton 2012-07-05 14:33) - PLID 47473 need to support multi location selection ( -2 )
		if((nLocationID > 0 || nLocationID == -2) && GetRemotePropertyInt("Scheduler_Month_HighlightAvailableDays", 1, 0, GetCurrentUserName())) {
			//TES 1/5/2012 - PLID 47310 - Loop through each day in the current month.
			long nResourceID;
			COleDateTime dtActive;
			m_pParent->GetWorkingResourceAndDate(this, 0, nResourceID, dtActive);
			int nMonth = dtActive.GetMonth();
			int nDay = 1;
			dtActive.SetDate(dtActive.GetYear(), dtActive.GetMonth(), nDay);
			if(nLocationID != -2) {
				while(dtActive.GetMonth() == nMonth) {
					//TES 1/5/2012 - PLID 47310 - Get all the location templates for this day 
					CTemplateHitSet rsTemplateHits;
					rsTemplateHits.Requery(dtActive, nResourceID, FALSE, nLocationID, false, true);
					while (!rsTemplateHits.IsEOF()) {
						if(rsTemplateHits.GetResourceID() == nResourceID || rsTemplateHits.GetIsAllResources()) {
							//TES 1/5/2012 - PLID 47310 - Now add them to our class, we use the day of the month as nDayIndex.
							m_ResourceAvailLocations_Month.HandleTemplateItem(nDay, rsTemplateHits);
						}
						rsTemplateHits.MoveNext();
					}
					nDay++;
					dtActive += COleDateTimeSpan(1, 0, 0, 0);
				}
			}
			// (d.singleton 2012-07-05 14:58) - PLID 47473 support muli location selection
			else if(nLocationID == -2) {
				CDWordArray dwaryLocationIDs;
				GetLocationFilterIDs(&dwaryLocationIDs);
				while(dtActive.GetMonth() == nMonth) {
					//TES 1/5/2012 - PLID 47310 - Get all the location templates for this day 
					// (d.singleton 2012-07-05 14:58) - PLID 47473 need to do below for each location selected
					for(int i = 0; i < dwaryLocationIDs.GetCount(); i++) {
						CTemplateHitSet rsTemplateHits;
						rsTemplateHits.Requery(dtActive, nResourceID, FALSE, dwaryLocationIDs.GetAt(i), false, true);
						while (!rsTemplateHits.IsEOF()) {
							if(rsTemplateHits.GetResourceID() == nResourceID || rsTemplateHits.GetIsAllResources()) {
								//TES 1/5/2012 - PLID 47310 - Now add them to our class, we use the day of the month as nDayIndex.
								m_ResourceAvailLocations_Month.HandleTemplateItem(nDay, rsTemplateHits);
							}
							rsTemplateHits.MoveNext();
						}
					}
					nDay++;
					dtActive += COleDateTimeSpan(1, 0, 0, 0);
				}
			}
		}
	}NxCatchAll("CMonthDlg::UpdateCalendarNames 3");

	// Ok, now the joyous fun begins: Figure out the vertical placement
	// of each event.
	int iMaxHeight = 0;
	for (i=1; i < m_aEvents.GetSize(); i++)
	{
		int iFirstDay = m_aEvents[i].iFirstDay;
		int iLastDay = m_aEvents[i].iLastDay;

		for (int j=0; j < i; j++)
		{
			int a = m_aEvents[j].iFirstDay;
			int b = m_aEvents[j].iLastDay;

			if (!((iFirstDay > b && iLastDay > b) ||
				(iFirstDay < a && iLastDay < a)))
			{
				m_aEvents[i].height++;
			}
		}
	}

	//MsgBox("Count = %d", count);

	// Redraw calendar
	CRect rc(m_startx, m_starty, m_startx + m_daywidth*7, m_starty + m_dayheight*6);
	InvalidateRect(&rc, FALSE);
	
	EndWaitCursor();
}

void CMonthDlg::OnSpinDay(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	int iSpinIndex = pNMHDR->idFrom - 2000;
	int iPos = pNMUpDown->iPos;
	int iDelta = pNMUpDown->iDelta;
	
	*pResult = 0;
}

void CMonthDlg::OnSpinAllDays(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	int iDelta = pNMUpDown->iDelta;

	if (m_nVisibleNamesOffset + iDelta >= 0) {
		// If we're scrolling up OR we're allowed to scroll down
		if (iDelta < 0 || m_bAllowScrollDown) {
			m_nVisibleNamesOffset += iDelta;
			CRect rc(m_startx, m_starty, m_startx + m_daywidth*7, m_starty + m_dayheight*6);
			InvalidateRect(&rc, FALSE);
		}
	}
	*pResult = 0;
}

void CMonthDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// Stop timer to catch idle mouse movements
	KillTimer(ID_MONTH_VIEW_TIMER);

	if (m_strDragInfo.GetLength() != 0) {
		// See if cursor is below lowest part of month
/*		if (point.y > m_starty + m_dayheight*6) {
			OnMoveMonthForward();
			GetCursorPos(&point);
			SetCursorPos(point.x, rc.top + m_starty + m_dayheight*6 - 8);
		}
		// See if cursor is less than highest part of month
		else if (point.y < m_starty) {
			OnMoveMonthBack();
			GetCursorPos(&point);
			SetCursorPos(point.x, rc.top + m_starty + 8);
		}*/
	}

	// Begin the timer to catch idle mouse movements
	m_ptMousePos = point;
	if (point != m_ptPriorMousePos) {
		// Only set the timer if the mouse actually moved since the last time it was set
		m_ptPriorMousePos = point;
		SetTimer(ID_MONTH_VIEW_TIMER, WAIT_TIME, NULL);

		if (m_strDragInfo.GetLength() != 0) {
			CRect rc;
			GetWindowRect(rc);

			if (point.y > m_starty + m_dayheight*6 ||
				point.y < m_starty)
			{
				SetTimer(ID_MONTH_VIEW_SCROLL_TIMER, SCROLL_WAIT_TIME, NULL);
				TRACE("SETTIMER\n");
			}
			else
			{
				KillTimer(ID_MONTH_VIEW_SCROLL_TIMER);
			}
		}
	}
	CNxSchedulerDlg::OnMouseMove(nFlags, point);
}

// Intercept the kill focus call to destroy the timer
void CMonthDlg::OnKillFocus(CWnd* pNewWnd) 
{
	CNxSchedulerDlg::OnKillFocus(pNewWnd);
	KillTimer(ID_MONTH_VIEW_TIMER);
}

// Intercept timer for refreshing single day control
void CMonthDlg::OnTimer(UINT nIDEvent) 
{
	if (m_ResEntry) {
		if (m_ResEntry->m_Res != NULL) return;
	}

	switch(nIDEvent) {
	case ID_MONTH_VIEW_SCROLL_TIMER:
		KillTimer(ID_MONTH_VIEW_SCROLL_TIMER);
		if (m_ptMousePos.y > m_starty + m_dayheight*6) {
			OnMoveMonthForward();
			SetTimer(ID_MONTH_VIEW_SCROLL_TIMER, SCROLL_WAIT_TIME, NULL);
		}
		// See if cursor is less than highest part of month
		else if (m_ptMousePos.y < m_starty) {
			OnMoveMonthBack();
			SetTimer(ID_MONTH_VIEW_SCROLL_TIMER, SCROLL_WAIT_TIME, NULL);
		}		
		break;

	case ID_MONTH_VIEW_TIMER:
		{
			//OK, we're processing this, so let's kill the timer now, since we don't want it
			//to keep firing forever.
			KillTimer(ID_MONTH_VIEW_TIMER);

			//JJ - 7/25/01 - this timer can still fire when we switch to day view!
			//stop it if we aren't in the month view anymore
			//TS - 8/3/01 - also, make sure we're in the scheduler, this was causing letter writing to
			//crash.  Once again, thank heaven for short-circuited ||.
			//RAC - 8/10/02 - ALWAYS MAKE SURE YOUR POINTERS AREN'T NULL!
			CMainFrame *pMainFrame = GetMainFrame();
			if (pMainFrame && pMainFrame->GetSafeHwnd() == ::GetActiveWindow()) {
				CView *pView = pMainFrame->GetActiveView();
				if (pView && pView->IsKindOf(RUNTIME_CLASS(CSchedulerView))) {
					CSchedulerView *pSchedView = (CSchedulerView *)pView; // A scheduler view IS an NxView
					if (pSchedView->GetActiveTab() == 3) {
						if(IsWindowEnabled() && IsWindowVisible()) {
							/* The single day control is supposed to display all the
							appointments on the calendar day the mouse is on. If the mouse
							isn't on one, it is simply not updated. */
							// First, find out what day the mouse is on (if any)
							//BeginWaitCursor();

							CPoint pt;
							GetCursorPos(&pt);
							CRect rcWnd;
							GetWindowRect(rcWnd);
							pt.x -= rcWnd.left;
							pt.y -= rcWnd.top;

 							COleDateTime date = GetActiveDate();
							int activemonth = date.GetMonth();
							date -= COleDateTimeSpan(date.GetDay() - 1,0,0,0);
							int weekday = date.GetDayOfWeek();
							date -= COleDateTimeSpan(weekday - 1, 0,0,0);

							for (int y=m_starty; y < m_starty + m_dayheight * 6; y += m_dayheight) {
								for (int x=m_startx; x < m_startx + m_daywidth * 7; x += m_daywidth) {
									if (pt.x >= x && pt.x <= x + m_daywidth &&
										pt.y >= y && pt.y <= y + m_dayheight) {
										if (date.GetMonth() == activemonth &&
											m_dtSingleDayDate != date) {
											m_dtSingleDayDate = date;
											SetActiveDate(date);
											if(g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
												//TES 12/17/2008 - PLID 32497 - If they're on Scheduler Standard, then
												// the single day isn't visible, so don't waste time updating it.
												UpdateSingleDayCtrl();
											}

											// Redraw calendar
											CRect rc(m_startx, m_starty, m_startx + m_daywidth*7, m_starty + m_dayheight*6);
											InvalidateRect(&rc, FALSE);

											//EndWaitCursor();
											CNxSchedulerDlg::OnTimer(nIDEvent);
											return;
										}
									}
									date += COleDateTimeSpan(1, 0,0,0);
								}
							}
						}
					}
				}
			}
			break;
		}
	}

	CNxSchedulerDlg::OnTimer(nIDEvent);
}

void CMonthDlg::UpdateSingleDayCtrl()
{
	CHECK_SAMAPHORE(m_bStoring) return;

	// Stop the timer from refreshing this again
	KillTimer(ID_MONTH_VIEW_TIMER);

	// (a.walling 2010-06-23 09:41) - PLID 39263 - Also update the description
	UpdateActiveDateLabel();

	CNxSchedulerDlg::UpdateView();
}

COleDateTime CMonthDlg::GetCalendarDateUnderMouse(int &sx, int &sy)
{
	CPoint pt;
	GetCursorPos(&pt);
	CRect rcWnd;
	GetWindowRect(rcWnd);
	pt.x -= rcWnd.left;
	pt.y -= rcWnd.top;

 	COleDateTime date = GetActiveDate();
	int activemonth = date.GetMonth();
	date -= COleDateTimeSpan(date.GetDay() - 1,0,0,0);
	int weekday = date.GetDayOfWeek();
	date -= COleDateTimeSpan(weekday-1, 0,0,0);

	for (int y=m_starty; y < m_starty + m_dayheight * 6; y += m_dayheight) {
		for (int x=m_startx; x < m_startx + m_daywidth * 7; x += m_daywidth) {
			if (pt.x >= x && pt.x <= x + m_daywidth &&
				pt.y >= y && pt.y <= y + m_dayheight) {
				//if (date.GetMonth() == activemonth) {
					sx = x;
					sy = y;
					return date;
				//}
			}
			date += COleDateTimeSpan(1, 0,0,0);
		}
	}
	sx = sy = -1;
	date.SetStatus( COleDateTime::invalid );
	return date;
}

BOOL CMonthDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		if (m_strDragInfo.GetLength() != 0 && !(GetAsyncKeyState(VK_SHIFT) & 0x80000000)) {
			SetCursor( AfxGetApp()->LoadCursor( IDC_POINTER_COPY ) );
			return TRUE;
		}

		// (c.haag 2009-12-22 14:02) - PLID 28977 - Change the mouse cursor appearance if it's 
		// hovering over a hyperlink (drag-and-drops takes priority though)
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		GetDlgItem(IDC_ACTIVE_DATE_APTCOUNT_LABEL)->GetWindowRect(rc);
		ScreenToClient(&rc);

		if (rc.PtInRect(pt)) {
			SetCursor(GetLinkCursor());
			return TRUE;
		}

		return CNxSchedulerDlg::OnSetCursor(pWnd, nHitTest, message);
	}
	NxCatchAll(__FUNCTION__);
	return FALSE;
}

// Returns the bottom of the drawn header (i.e. where anything else 
// could start drawing safely without overlapping the header area)
long CMonthDlg::DrawPageHeader(CDC *pDC, LPRECT rcPage, long nSetId)
{
	CRect rectFooter;
	CFont ftrFont;
	rectFooter.SetRect(0, (int)((0.984) * (double)(rcPage->bottom - rcPage->top)), rcPage->right, rcPage->bottom);
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&ftrFont, 90, "Arial", pDC);

	// Calculate the header text
	CString strHdr, strFooter;
	CString strApptType;
	// See if we are filtering on appointment type
	if (nSetId != -1) {
		// We are filtering on appointment type so generate the type name in quotes
		long lRow = m_dlTypeFilter->FindByColumn(0, _variant_t(nSetId), 0, TRUE);
		if (lRow >= 0)
		{
			strApptType = VarString(m_dlTypeFilter->GetValue(lRow, 1));
		}		
		strApptType.Insert(0, "'");
		strApptType += "' ";
	} else {
		// We are not filtering on appointment type so use the word "All"
		strApptType = "All ";
	}
	// Create the header text
	COleDateTime dtPrintDate = COleDateTime::GetCurrentTime();
	dtPrintDate.SetDateTime(dtPrintDate.GetYear(), dtPrintDate.GetMonth(), dtPrintDate.GetDay(), 0, 0, 0);

	// (c.haag 2008-06-18 17:24) - PLID 26135 - Use a different header when printing template text
	if (m_pParent->m_bPrintTemplates) {
		strHdr.Format("All Templates for %s\n%s",
			VarString(m_dlResources->GetValue( m_dlResources->GetCurSel(), 2)), 
			FormatDateTimeForInterface(GetActiveDate(), "%B %Y"));

	} else {
		strHdr.Format("%sAppointments for %s\n%s", strApptType, /*m_ResourceCombo.GetText()*/
			VarString(m_dlResources->GetValue( m_dlResources->GetCurSel(), 2)), 
			FormatDateTimeForInterface(GetActiveDate(), "%B %Y"));
	}


	// Create and selectan appropriate font
	CFont fontHdr, *pOldFont;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&fontHdr, 240, "Arial", pDC);
	pOldFont = pDC->SelectObject(&fontHdr);
	// Set header text color
	pDC->SetTextColor( RGB(0,0,0) );
	// Determine the header area
	const int nHeaderTop = 80;
	CRect rcHeader(rcPage);
	rcHeader.DeflateRect(0, nHeaderTop, 0, 0);
	// Draw the header, storing its height
	long nHeight = nHeaderTop+pDC->DrawText(strHdr, rcHeader, DT_TOP|/*DT_SINGLELINE|*/DT_NOPREFIX|DT_CENTER);
	// Draw the footer
	strFooter.Format("Printed On: %s", FormatDateTimeForInterface(dtPrintDate, NULL, dtoDate));
	pDC->SelectObject(&ftrFont);
	pDC->DrawText(strFooter, rectFooter, DT_RIGHT);
	// Unselect the header font
	pDC->SelectObject(pOldFont);

	// Return the bottom of the drawn header (i.e. where anything else 
	// could start drawing safely without overlapping the header area)
	return rcHeader.top + nHeight;
}

// Returns the bottom of the drawn header (i.e. where anything else 
// could start drawing safely without overlapping the header area)
long CMonthDlg::DrawDayHeader(CDC *pDC, long x, long y, long nBoxWidth, long nBoxHeight, LPCTSTR strDayHdr, BOOL bDrawBorder)
{
	// Set up the colors appropriately
	pDC->SetTextColor(RGB(0,0,0) );
	pDC->SetBkColor(RGB(192,192,192));
	CBrush greyBrush(RGB(192,192,192)), *pOldBrush;
	pOldBrush = pDC->SelectObject(&greyBrush);

	// Margin around the header
	const long mx = 10, my = 4;

	// Set up the text rect for the day header and calc the size of the text
	CRect rcDayHeaderText(x+mx, y+my, x+nBoxWidth-mx, y+nBoxHeight-my);
	pDC->DrawText(strDayHdr, rcDayHeaderText, DT_CALCRECT|DT_TOP|DT_SINGLELINE|DT_NOPREFIX|DT_LEFT);

	// Set up the rect for the entire day header area and color it grey
	CRect rcDayHeaderBox(x, y, rcDayHeaderText.right+mx, rcDayHeaderText.bottom+my);
	if (bDrawBorder) pDC->Rectangle(rcDayHeaderBox);
	
	// Actually draw the text in the day header area
	pDC->DrawText(strDayHdr, rcDayHeaderText, DT_TOP|DT_SINGLELINE|DT_NOPREFIX|DT_LEFT);

	// We're done with the brush
	pDC->SelectObject(pOldBrush);

	// Return the bottom of the drawn header (i.e. where anything else 
	// could start drawing safely without overlapping the header area)
	return rcDayHeaderBox.bottom;
}

CString GetSubString(const CString &strFullString, int nSubIndex, TCHAR chDelim /*= '|'*/)
{
	// Init
	int nSubPos = 0, nNextPos = strFullString.Find(chDelim, 0);
	for (int i=0; i<nSubIndex; i++) {
		if (nNextPos >= 0) {
			// Move nPos one character forward so that 
			// the next step finds the next delimiter
			nSubPos = nNextPos + 1;
			nNextPos = strFullString.Find(chDelim, nSubPos);
		} else {
			// No more elements
			break;
		}
	}

	if (i == nSubIndex) {
		// We found the element
		if (nNextPos == -1) {
			// nSubIndex must refer to the last element in the full 
			// string because there was no terminating delimiter
			return strFullString.Mid(nSubPos);
		} else {
			// Return the substring
			return strFullString.Mid(nSubPos, nNextPos-nSubPos);
		}
	} else {
		// Failed to find the element
		return "";
	}
}

void SetSubString(CString& strFullString, CString strNewSubString, int nSubIndex, TCHAR chDelim /*= '|'*/)
{
	// Init
	int nSubPos = 0, nNextPos = strFullString.Find(chDelim, 0);
	for (int i=0; i<nSubIndex; i++) {
		if (nNextPos >= 0) {
			// Move nPos one character forward so that 
			// the next step finds the next delimiter
			nSubPos = nNextPos + 1;
			nNextPos = strFullString.Find(chDelim, nSubPos);
		} else {
			// No more elements
			break;
		}
	}

	if (i == nSubIndex) {
		// We found the element. Remove the existing substring
		if (nNextPos == -1) {
			// nSubIndex must refer to the last element in the full 
			// string because there was no terminating delimiter
			strFullString = strFullString.Left(nSubPos);
		} else {
			CString strTemp = strFullString.Left(nSubPos);
			strTemp += strFullString.Right(strFullString.GetLength() - nNextPos);
			strFullString = strTemp;
		}

		// Insert the new text
		strFullString.Insert(nSubPos, strNewSubString);
	} else {
		// Failed to find the element. Do nothing.
	}
}

CString CMonthDlg::GetPrintText(const CString &strApptText)
{
	CString strStartTime = GetSubString(strApptText, 4);
	COleDateTime dt;
	dt.ParseDateTime(strStartTime);
	if (dt.GetStatus() == COleDateTime::valid) {
		//We're going to format the time, then replace the long version of the marker with the short version.
		strStartTime = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoTime);
		strStartTime.Replace("am", "a");
		strStartTime.Replace("pm", "p");
		strStartTime.Replace("AM", "a");
		strStartTime.Replace("PM", "p");
	}
	CString str = strStartTime + "\t" + GetSubString(strApptText, 0) + " " + GetSubString(strApptText, 1);

	// Append the purpose if there is a patient ID, a purpose, and we want to show purposes
	if (m_pParent && m_pParent->m_bShowMonthProcedures && !GetSubString(strApptText, 1).IsEmpty() && !GetSubString(strApptText, 7).IsEmpty())
	{
		str += CString("\n\t") + GetSubString(strApptText, 7);
	}
	
	// CAH: Legacy code, but will keep for now
	// Print purpose in place of blank name
	//if (str == " ") {
	//	str = strStartTime + "\t" + "[" + GetSubString(strApptText, 2) + "]";
	//}

	return str;
}

BOOL CMonthDlg::DoesDayPrintOutputFit(CDC *pDC, CStringArray *pastrNames, long nBoxWidth, long nBoxHeight)
{
	if (pastrNames) {
		long y = 0;
		// Loop until we've extended past box height
		for (int i=0; i < pastrNames->GetSize() && y <= nBoxHeight; i++) {
			// Don't actually draw the text, just calculate
			y += PrintAppt(pDC, pastrNames->GetAt(i), 
				CRect(0,0,nBoxWidth, nBoxHeight),  
				TRUE);
		}
		if (y > nBoxHeight) {
			// We extended past box height so "No, the day output doesn't fit"
			return FALSE;
		} else {
			// We extended past box height so "Yes, the day output does fit"
			return TRUE;
		}
	} else {
		// We got a NULL pastrNames.  That shouldn't happen
		ASSERT(FALSE);
		return FALSE;
	}
}

void CMonthDlg::Print(CDC * pDC, CPrintInfo *pInfo)
{
	// Some useful drawing tools
	CBrush greybrush(RGB(192, 192, 192));
	CBrush whitebrush(RGB(255, 255, 255));
	CBrush* pOldBrush = pDC->SelectObject(&whitebrush);
	CPen pen(PS_SOLID, 1, RGB(0,0,0));
	CPen* pOldPen = pDC->SelectObject(&pen);
	CFont* pOldFont;

	// Decide if we're printing in color or not
	bool bBlackAndWhite;
	
	// (a.walling 2010-04-13 17:19) - PLID 37911
	// Also added a final property to force printing in color. Some printers just refuse to give us the correct information.
	BOOL bForceColorPrinting = GetRemotePropertyInt("ForceColorPrinting", FALSE, 0, "<None>", true) ? TRUE : FALSE;
	if (bForceColorPrinting) {
		bBlackAndWhite = false;
	} else {
		// (a.walling 2010-04-13 17:19) - PLID 37911
		// Here is the situation. The nBpp and nPlanes may be 1 even though this is color. This happens to me, too, and is easily reproducable
		// with the Adobe PDF printer. HP printers also seem to have a tendency to lie about this; There does not seem to be an agreement on 
		// how this is done. However I tested PowerPoint (which I know modifies its gradients and etc based on how it is printing) and it 
		// detects the Adobe PDF printer's capability properly. I fired up IDA and windbg and managed to see what it was doing. It appears
		// to be checking the dmColor member of the actual DEVMODE structure, and also calls NUMCOLORS. I found some place in MSDN that said
		// in general NUMCOLORS will be > 2 for a color printer. Both the dmColor and NUMCOLORS method work correctly for my two PDF software
		// printers, the XPS software printer, and the HP LaserJet 2200, 3330, and 3390 series, and the Lexmark X342n. So I think we are good.
		// I put in a remote property to revert to classic behavior in case this causes trouble.

		int nBpp = pDC->GetDeviceCaps(BITSPIXEL);
		int nPlanes = pDC->GetDeviceCaps(PLANES);

		int nDevCapsColors = nBpp * nPlanes; // 2 ^ nDevCapsColors
		int nDevCapsNumColors = pDC->GetDeviceCaps(NUMCOLORS); // 

		CString strDeviceName = pInfo->m_pPD->GetDeviceName(); 
		CString strDriverName = pInfo->m_pPD->GetDriverName();
		CString strPortName = pInfo->m_pPD->GetPortName();
		int nDevModeColor = -1;
		{
			DEVMODE* pDevMode = pInfo->m_pPD->GetDevMode();
			if (pDevMode && pDevMode->dmFields & DM_COLOR) {
				nDevModeColor = pDevMode->dmColor;
			}
		}

		BOOL bUseClassicCheck = GetRemotePropertyInt("UseClassicPrinterColorCheck", FALSE, 0, "<None>", true) ? TRUE : FALSE;

		if (bUseClassicCheck) {
			bBlackAndWhite = nBpp > 1 ? false : true;
		} else {
			if (nBpp > 1) {
				bBlackAndWhite = false;
			} else {
				if (nDevModeColor != -1) {
					bBlackAndWhite = nDevModeColor == DMCOLOR_COLOR ? false : true;
				} else {
					bBlackAndWhite = nDevCapsNumColors > 2 ? false : true;
				}
			}
		}
		
		LogDetail("Printer color props for '%s' using driver '%s' on '%s': DevCaps Bpp(%li), Planes(%li); DevMode dmColor(%li); DevCaps NumColors(%li); Classic check(%li); Result: %li", strDeviceName, strDriverName, strPortName, nBpp, nPlanes, nDevModeColor, nDevCapsNumColors, bUseClassicCheck, long(bBlackAndWhite ? 1 : 0));
	}

 	// Calculate important information based on the active date
	// Calculate active date info
	COleDateTime date = GetActiveDate();
	const int nActiveMonth = date.GetMonth();
	const int nActiveYear = date.GetYear();
	const int nActiveDay = date.GetDay();
	// Move to the beginning of the month and get the day of the week
	date -= COleDateTimeSpan(nActiveDay-1,0,0,0);
	const int nWeekDay = date.GetDayOfWeek();
	// Move to the next previous sunday
	date -= COleDateTimeSpan(nWeekDay-1, 0,0,0);
	// Calculate the number of rows we'll need
	const long nDaysVisible = (GetDaysInMonth(nActiveMonth, nActiveYear) + nWeekDay - 1);
	const long nRowCount = nDaysVisible / 7 + ((nDaysVisible % 7) ? 1 : 0);

	// Set the draw info
	pDC->SetTextColor(RGB(0,0,0));
	pDC->SetBkMode(TRANSPARENT);

	// Output the page header
	long nSetId = GetPurposeSetId();
	int starty = DrawPageHeader(pDC, pInfo->m_rectDraw, nSetId);

	// Calculate other useful values
	int boxwidth = pInfo->m_rectDraw.Width() / 8;  // Width of calendar day
	int boxheight = (pInfo->m_rectDraw.Height()-starty-60) / nRowCount; // Height of calendar day
	int startx = (pInfo->m_rectDraw.Width() - boxwidth*7) / 2;

	// Select the font we will be using for the rest of the print
	CFont font;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&font, 70, "Arial", pDC);
	pOldFont = pDC->SelectObject(&font);

	// Caclculate the height of the text based on the font we will be using to draw it
	LOGFONT lf;
	font.GetLogFont(&lf);
	int textheight = lf.lfHeight;
	if (textheight < 0) {
		textheight = -textheight;
	} else if (textheight == 0) {
		textheight = 30;
	}

	// Some variables used repeatedly in the upcoming loop
	CString strDayHdr;
	

	// Show all the days  (nRowCount rows, seven columns)
	for (int y=starty; y < starty + boxheight*nRowCount; y += boxheight) {
		for (int x=startx; x < startx + boxwidth*7; x += boxwidth) {
			bool bIsDayActive = false;

			// Generate this day's rect
			CRect rc(x, y, x + boxwidth + 1, y + boxheight + 1);
			
			// Is this month in the currently printing month?
			if(date.GetMonth() == nActiveMonth) bIsDayActive = true;

			
			bool bPrintMonthName = false;
			if(date.GetDay() == 1 || (x == startx && y == starty)) bPrintMonthName = true;
			PrintDay(date, rc, pDC, bBlackAndWhite, bPrintMonthName, bIsDayActive);
			/*// Draw day header
			{
				// Generate the day header text
				if (date.GetDay() == 1 || (x == startx && y == starty)) {
					// This is the first VISIBLE day of the given month so
					// prepend the month's common name to the day value
					strDayHdr.Format("%s %d", month_names[date.GetMonth()-1], date.GetDay());
				} else {
					// This is a regular day so just use the day number
					strDayHdr.Format("%d", date.GetDay());
				}
				// Draw the day header
				nDayHeaderBottom = DrawDayHeader(pDC, x, y, boxwidth, boxheight, strDayHdr, !bIsDayActive);
			}

			// Print day events
			long nEvents = 0;
			
			if (date.GetMonth() == nActiveMonth) {
				for (int i=0; i < m_aEvents.GetSize(); i++)
				{
					if (date.GetDay() >= m_aEvents[i].iFirstDay &&
						date.GetDay() <= m_aEvents[i].iLastDay)
					{
						// Set color of text
						COLORREF clr;

						if (bBlackAndWhite) {
							// Use black
							clr = RGB(0,0,0);
						} else {
							// Use purposeset color
							clr = m_aEvents[i].clr;
							// Tone down if too bright
							BYTE r = ((int)GetRValue(clr)*2)/3;
							BYTE g = ((int)GetGValue(clr)*2)/3;
							BYTE b = ((int)GetBValue(clr)*2)/3;
							clr = RGB( r, g, b );
						}

						CBrush brush(clr), *pOldBrush;
						CPen pen(PS_SOLID, 1, clr), *pOldPen;
						pOldBrush = pDC->SelectObject(&brush);
						pOldPen = pDC->SelectObject(&pen);
						pDC->Rectangle(x, y + boxheight - (textheight+3)*(nEvents+1),
							x + boxwidth, y + boxheight - (textheight+3)*(nEvents));
						pDC->SelectObject(pOldPen);
						pDC->SelectObject(pOldBrush);

						pDC->SetTextColor(RGB(255,255,255));
						pDC->DrawText(m_aEvents[i].strName,
								CRect(x + 20, y + boxheight - (textheight+3)*(nEvents+1) - 2, rc.right, rc.bottom), 
								DT_WORDBREAK|DT_TOP|DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS);
						nEvents++;
					}
				}
			}
			long lEventTopY = y + boxheight - (textheight+3)*(nEvents);
			
			// Print day appointments
			{
				if (pastrNames != NULL) {
					// Decide where the first appointment will display
					int text_y = nDayHeaderBottom;

					// Decide which appointment to display first
					int nStart = m_nVisibleNamesOffset;
					int nMaxStart = pastrNames->GetSize() - m_nVisibleNamesPerDay;
					if (nMaxStart < 0) nMaxStart = 0;
					if (nStart > nMaxStart) {
						nStart = nMaxStart;
					}
					
					if (nStart > 0) {
						// If we're starting anywhere but the first appointment, 
						// calculate the total height of all appointments and if 
						// they'll fit then start at 0
						if (DoesDayPrintOutputFit(pDC, pastrNames, boxwidth-20, rc.Height()-textheight)) {
							// The output will fit
							nStart = 0;
						}
					}

					// If we are starting anywhere but the beginning 
					// of the list, put three dots at the right
					if (nStart > 0) {
						pDC->SetTextColor(RGB(0,0,0));
						pDC->TextOut(x + boxwidth - 110, y + 50, "***");
					}
					for (int i=nStart; i < pastrNames->GetSize() && text_y <= min(rc.bottom - textheight, lEventTopY - textheight); i++) {
						// Set color of text
						COLORREF clr;
						if (bBlackAndWhite) {
							// Use black
							clr = RGB(0,0,0);
						} else {
							// Use purposeset color
							clr = (COLORREF)m_adwVisibleColors[date.GetDay()-1].GetAt(i);
							// Tone down if too bright
							//if (GetRValue(clr) + GetGValue(clr) + GetBValue(clr) > 512) {
								BYTE r = ((int)GetRValue(clr)*2)/3;
								BYTE g = ((int)GetGValue(clr)*2)/3;
								BYTE b = ((int)GetBValue(clr)*2)/3;
								clr = RGB( r, g, b );
							//}
						}
						pDC->SetTextColor( clr );
						text_y += pDC->DrawText(GetPrintText(pastrNames->GetAt(i)), 
							CRect(x + 20, text_y, rc.right, rc.bottom), 
							DT_SINGLELINE|DT_TOP|DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS);
					}
					if (i < pastrNames->GetSize()) {
						pDC->SetTextColor(RGB(0,0,0));
						pDC->TextOut(x + boxwidth - 110, y + boxheight - textheight + 50, "***");
					}
				}
			}*/

			// Default back to white if greyed
			/*if (date.GetMonth() != nActiveMonth) {
				pDC->SelectObject(pOldBrush);
				pDC->SetBkColor(RGB(255, 255, 255));
			}*/
			
			// Move to next day
			date += COleDateTimeSpan(1, 0,0,0);
		}
	}
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldFont);
}

//(s.dhole 6/1/2015 4:18 PM ) - PLID 65645  Remove 
//void CMonthDlg::FindFirstAvailableAppt()
//{
//	//(j.jones 2003-07-03 11:56) do nothing if the dialog is disabled
//	//TES 2/2/2009 - PLID 32497 - Why are we checking whether the singleday is disabled (which it can be, now, if they
//	// have the Scheduler Standard license)?  Let's just check whether they, in fact, have an active resource.
//	long nActiveResource = m_pParent->GetActiveResourceID();
//	if(nActiveResource == -1) {
//		//they don't have any resources!!
//		MsgBox("Please select a resource before using Find First Available");
//		return;
//	}
//
//	// If first available appts. are not visible, display them
//	if (!m_IsShowingFirstAvail) {		
//		GetMainFrame()->m_FirstAvailAppt.m_lDefaultResource = nActiveResource;
//		GetMainFrame()->m_FirstAvailAppt.m_strLastResource = VarString(m_dlResources->GetValue( m_dlResources->GetCurSel(), 2));//m_ResourceCombo.GetText();
//
//		if (IDCANCEL == GetMainFrame()->m_FirstAvailAppt.DoModal()) return;
//		CString strFirstResource = GetMainFrame()->m_FirstAvailAppt.m_strFirstSelectedResource;
//
//		// CAH 10/5/01: Quit if the user just wanted text results. TODO:
//		// Go to the day view for the chosen day and time.
//		if (GetMainFrame()->m_FirstAvailAppt.m_boTextResults) 
//		{
//			// Change the visible resource
//			// to the first one selected in the find
//			// first dialog.
//			for (int i=0; i < m_dlResources->GetRowCount()/*m_ResourceCombo.GetListCount()*/; i++)
//			{
//				CString str = VarString(m_dlResources->GetValue(i, 2));
//				if (str == strFirstResource)
//				{
//					//m_ResourceCombo.SetListIndex((long)i);
//					m_dlResources->PutCurSel(i);
//					OnSelChosenResources(i);
//					break;
//				}
//			}
//
//			// If this is true, it means the resource is not in the dropdown, and we should
//			// add it to the resource combo.
//			// (j.gruber 2011-02-17 13:06) - PLID 42427 - don't give a message, just switch the view to one with the top resource in it
//			if (i == m_dlResources->GetRowCount())
//			{
//				//find a view that has the first resource in it
//				_RecordsetPtr rsViews = CreateParamRecordset(""
//					" DECLARE @ItemName nVarChar(50); \r\n "
//					" SET @ItemName = {STRING}; \r\n "
//					" DECLARE @UserID INT; \r\n "
//					" SET @UserID = {INT};  \r\n "
//					" SELECT  \r\n" 
//					" CASE WHEN \r\n "
//					" (SELECT Top 1 ResourceViewID FROM ResourceViewDetailsT INNER JOIN ResourceT ON ResourceViewDetailsT.ResourceID = ResourceT.ID WHERE ResourceT.Inactive = 0 AND Relevence >= 0 AND ResourceT.Item = @ItemName) IS NULL THEN   \r\n "
//					" (SELECT -2 FROM UserResourcesT INNER JOIN ResourceT ON UserResourcesT.ResourceID = ResourceT.ID WHERE ResourceT.Inactive = 0 AND UserID = @UserID AND ResourceT.Item = @ItemName AND Relevence >=0) ELSE \r\n "
//					" (SELECT Top 1 ResourceViewID FROM ResourceViewDetailsT INNER JOIN ResourceT ON ResourceViewDetailsT.ResourceID = ResourceT.ID WHERE ResourceT.Inactive = 0 AND Relevence >= 0 AND ResourceT.Item = @ItemName ORDER BY Relevence ASC) END AS ResourceViewID \r\n "
//					, strFirstResource, GetCurrentUserID());
//				if (rsViews->eof) {
//				//	MsgBox("The appointment was made, but you must edit the visible resources to be able to see it.");
//				}
//				else {
//					//switch the view
//					long nViewID = AdoFldLong(rsViews->Fields, "ResourceViewID", -1);
//					if (nViewID != -1) {
//						m_pParent->SetCurrentResourceViewID(nViewID, -2);
//					}
//					else {
//						//this means the resource they picked is inactive or not in any view						
//					}
//				}
//			}
//			GetMainFrame()->m_FirstAvailAppt.RemoveAvailableDays();
//
//			UpdateCalendarNames();
//			return;
//		}
//
//		m_IsShowingFirstAvail = TRUE;		
//
//		// Change the visible resource
//		// to the first one selected in the find
//		// first dialog.
//		for (int i=0; i < m_dlResources->GetRowCount()/*m_ResourceCombo.GetListCount()*/; i++)
//		{
//			CString str = VarString(m_dlResources->GetValue(i, 2));
//			if (str == strFirstResource)
//			{
//				//m_ResourceCombo.SetListIndex((long)i);
//				m_dlResources->PutCurSel(i);
//				OnSelChosenResources(i);
//				break;
//			}
//		}
//
//		// Now change the date to the start time
//		SetActiveDate(GetMainFrame()->m_FirstAvailAppt.m_dtStartDate);
//		UpdateView();
//	}
//	// Otherwise, make them disappear
//	else {
//		// Remove all available (green) days
//		GetMainFrame()->m_FirstAvailAppt.RemoveAvailableDays();
//		m_IsShowingFirstAvail = FALSE;
//	}
//
//	// Redraw calendar
//	CRect rc(m_startx, m_starty, m_startx + m_daywidth*7, m_starty + m_dayheight*6);
//	InvalidateRect(&rc, FALSE);
//}

BOOL CMonthDlg::OnInitDialog() 
{
	try {
		
		// (a.walling 2010-04-13 17:24) - PLID 37911	
		// (j.luckoski 2012-05-08 17:10) - PLID 50242 - Add all properties to bulk cache
		g_propManager.CachePropertiesInBulk("MonthDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name IN ("
				"'SchedulerShowMonthViewPurposes', 'ForceColorPrinting', 'UseClassicPrinterColorCheck', "
				"'Scheduler_Month_HighlightAvailableDays', 'Scheduler_Month_HighlightAvailableDaysColor', "
				"'ShowCancelledAppointment', 'CancelledDateRange', 'SchedCancelledApptColor' "
			"))", _Q(GetCurrentUserName()));
		
		// (j.luckoski 2012-05-07 11:47) - PLID 11597 - Grab preferences for cancelled appts.
		m_nCancelledAppt = GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true);
		// (j.luckoski 2012-06-12 15:58) - PLID 11597 - Did not default to 24
		m_nDateRange = GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true);
		m_nCancelColor = GetRemotePropertyInt("SchedCancelledApptColor", RGB(192,192,192), 0, GetCurrentUserName(), true);

		// Must set m_pSingleDayCtrl before CNxSchedulerDlg::OnInitDialog is called
		// (c.haag 2010-03-26 15:01) - PLID 37332 - Use SetDispatch
		m_pSingleDayCtrl.SetDispatch( (LPDISPATCH)GetDlgItem(IDC_SINGLE_DAY_CTRL)->GetControlUnknown() );
		m_pEventCtrl.SetDispatch( (LPDISPATCH)GetDlgItem(IDC_MONTH_EVENT_CTRL)->GetControlUnknown() );

		// (c.haag 2009-12-22 13:44) - PLID 28977 - The Appt count label is now a hyperlink label
		m_nxlabelActiveDateAptcountLabel.SetType(dtsHyperlink);

		// Get a variable so we can work with the resource combo for this view
		m_dlResources = BindNxDataListCtrl(IDC_RESOURCE_LIST, false);

		// Prepare the font we'll be using for the appointments being listed in each day box
		{
			// Create the font
			//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
			CreateCompatiblePointFont(&m_fntAppointment, 80, "MS Serif");

			// Caclculate the height of the text in the font we just created
			LOGFONT lf;
			m_fntAppointment.GetLogFont(&lf);
			m_nHeightAppointmentFont = lf.lfHeight;
			if (m_nHeightAppointmentFont < 0) {
				m_nHeightAppointmentFont = -m_nHeightAppointmentFont;
			} else if (m_nHeightAppointmentFont == 0) {
				m_nHeightAppointmentFont = 11;
			}
		}


		GetDlgItem(IDC_EDIT_ACTIVE_SINGLEDAY_DAY)->SetFont(&theApp.m_boldFont);
		GetDlgItem(IDC_ACTIVE_DATE_APTCOUNT_LABEL)->SetFont(&theApp.m_multiresFont);

		m_wk0Button.AutoSet(NXB_RIGHT);
		m_wk1Button.AutoSet(NXB_RIGHT);
		m_wk2Button.AutoSet(NXB_RIGHT);
		m_wk3Button.AutoSet(NXB_RIGHT);
		m_wk4Button.AutoSet(NXB_RIGHT);
		m_wk5Button.AutoSet(NXB_RIGHT);

		m_monthBackButton.AutoSet(NXB_LEFT);
		m_monthForwardButton.AutoSet(NXB_RIGHT);

		//Set the labels correctly!
		CString strDayName;
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDAYNAME1, strDayName.GetBuffer(80), 80, FALSE);
		strDayName.ReleaseBuffer();
		SetDlgItemText(IDC_STATIC_MON, strDayName);
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDAYNAME2, strDayName.GetBuffer(80), 80, FALSE);
		strDayName.ReleaseBuffer();
		SetDlgItemText(IDC_STATIC_TUE, strDayName);
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDAYNAME3, strDayName.GetBuffer(80), 80, FALSE);
		strDayName.ReleaseBuffer();
		SetDlgItemText(IDC_STATIC_WED, strDayName);
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDAYNAME4, strDayName.GetBuffer(80), 80, FALSE);
		strDayName.ReleaseBuffer();
		SetDlgItemText(IDC_STATIC_THU, strDayName);
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDAYNAME5, strDayName.GetBuffer(80), 80, FALSE);
		strDayName.ReleaseBuffer();
		SetDlgItemText(IDC_STATIC_FRI, strDayName);
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDAYNAME6, strDayName.GetBuffer(80), 80, FALSE);
		strDayName.ReleaseBuffer();
		SetDlgItemText(IDC_STATIC_SAT, strDayName);
		NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDAYNAME7, strDayName.GetBuffer(80), 80, FALSE);
		strDayName.ReleaseBuffer();
		SetDlgItemText(IDC_STATIC_SUN, strDayName);

		//TES 12/17/2008 - PLID 32497 - Hide the single day control (and related controls) if we don't have enterprise edition.
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
			GetDlgItem(IDC_SINGLE_DAY_CTRL)->EnableWindow(FALSE);
			GetDlgItem(IDC_SINGLE_DAY_CTRL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_MONTH_EVENT_CTRL)->EnableWindow(FALSE);
			GetDlgItem(IDC_MONTH_EVENT_CTRL)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_EDIT_ACTIVE_SINGLEDAY_DAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_ACTIVE_SINGLEDAY_DAY)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_BTN_NEW_EVENT)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_NEW_EVENT)->ShowWindow(SW_HIDE);

			CRect rSingleDay;
			GetDlgItem(IDC_SINGLE_DAY_CTRL)->GetWindowRect(&rSingleDay);
			ScreenToClient(&rSingleDay);

			CRect rcFirstDayFrame;
			GetDlgItem(IDC_FIRST_DAY_FRAME)->GetWindowRect(rcFirstDayFrame);
			ScreenToClient(&rcFirstDayFrame);
			rcFirstDayFrame.right += (rSingleDay.Width()/7);
			GetDlgItem(IDC_FIRST_DAY_FRAME)->MoveWindow(&rcFirstDayFrame);

			//TES 12/17/2008 - PLID 32497 - Also make sure that neither control's "floating arrows" show up.
			m_pSingleDayCtrl.PutEnabled(VARIANT_FALSE);
			m_pEventCtrl.PutEnabled(VARIANT_FALSE);
		}

		BOOL bRet = CNxSchedulerDlg::OnInitDialog();
		
		// (a.walling 2008-05-13 10:31) - PLID 27591 - Custom date format
		m_DateSelCombo.SetCustomFormat("MMMM', 'yyyy");
		return bRet;
	}
	NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CMonthDlg::StoreDetails()
{
	BEGIN_SEMAPHORE(m_bStoring);
	
	CNxSchedulerDlg::StoreDetails();
	SetActiveDate(m_dtPriorDate);

	END_SEMAPHORE(m_bStoring);
}

void CMonthDlg::RecallDetails()
{
	// Save prior date
	m_dtPriorDate = GetActiveDate();

//	// Set new current date to be the first of the month
//	m_DateSelCombo.SetDateValue(COleVariant(GetActiveDate() - COleDateTimeSpan((newDate.GetDay() - 1), 0, 0, 0)));
	
	CNxSchedulerDlg::RecallDetails();
}

int CMonthDlg::GetEventCountInDay(int nDay)
{
	int nEvents = 0;

	for (int i=0; i < m_aEvents.GetSize(); i++)
	{
		if (m_aEvents[i].iFirstDay <= nDay && m_aEvents[i].iLastDay >= nDay)
			nEvents++;
	}
	return nEvents;
}

void CMonthDlg::GetEventsInDay(int nDay, CArray<_MONTHEVENT, _MONTHEVENT>& aEvents)
{
	for (int i=0; i < m_aEvents.GetSize(); i++)
	{
		if (m_aEvents[i].iFirstDay <= nDay && m_aEvents[i].iLastDay >= nDay)
			aEvents.Add(m_aEvents[i]);
	}
}

void CMonthDlg::UpdateViewBySingleAppt(long nResID)
{
	// (c.haag 2003-10-20 12:25) - Update the single day control
	CNxSchedulerDlg::UpdateViewBySingleAppt(nResID);

	// (c.haag 2010-06-23 10:10) - PLID 39210 - Don't update if a reservation is locked by
	// a right-click action or the reservation window is open.
	// (c.haag 2010-08-27 11:38) - PLID 40108 - Moved the logic into one function
	if (!IsSafeToUpdateView()) return;

	BOOL bReadPurposes = GetRemotePropertyInt("SchedulerShowMonthViewPurposes", 0, 0, GetCurrentUserName(), false);

	// (c.haag 2003-10-20 12:25) - Update the appointment on the calendar.
	// Start by finding the appointment in our current list. nDay will be
	// the appointment day, and nPos will be the index in that day.
	BOOL bFoundAppt = FALSE;
	BOOL bFoundEvent = FALSE;
	BOOL bInvalidate = FALSE;
	long nDay, nPos;
	for (nDay=0; nDay < 31 && !bFoundAppt; nDay++)
	{
		for (nPos=0; nPos < m_astrVisibleNames[nDay].GetSize() && !bFoundAppt; nPos++)
		{
			CString strID = GetSubString(m_astrVisibleNames[nDay][nPos], 6);
			if (atol(strID) == nResID)
				bFoundAppt = TRUE;
		}
	}

	if (bFoundAppt)
	{
		nPos--;
		nDay--;
	}
	else
	{
		// Ok, we didn't find an appointment. Look in our event list.
		// nPos will be the index of the element we find the ID at.
		for (nPos=0; nPos < m_aEvents.GetSize() && !bFoundEvent; nPos++)
		{
			if (m_aEvents[nPos].nResID == nResID)
				bFoundEvent = TRUE;
		}
		if (bFoundEvent)
			nPos--;
	}

	// Generate the extra filter
	// (a.walling 2013-05-31 14:06) - PLID 56961 - GetExtraResSimpleFilter is parameterized and simplified
	CSqlFragment filter = GetExtraResSimpleFilter();

	// Build the query
	COleDateTime dtMinDate, dtMaxDate;
	dtMinDate.SetDate(GetActiveDate().GetYear(), GetActiveDate().GetMonth(), 1);
	if (dtMinDate.GetMonth() < 12)
		dtMaxDate.SetDate(dtMinDate.GetYear(), dtMinDate.GetMonth() + 1, 1);
	else
		dtMaxDate.SetDate(dtMinDate.GetYear() + 1, 1, 1);


	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized	
	// (j.jones 2011-02-11 09:31) - PLID 35180 - ResExtendedQ is now a function that takes in:
	//@ExcludeCanceledAppointments BIT, @FromDate DATETIME, @ToDate DATETIME, @ResourceIDs NTEXT
	// (j.luckoski 2012-05-07 11:48) - PLID 11597 - Alter table so that if cancelled it may still show under certain conditions.
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
	_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), 
		"SELECT ResExQ.ID, StartTime, Convert(DateTime,ResExQ.Date) AS Date, EndTime, Notes, PatientID, LocationID, First, Last, AptTypeID, AptType, "
		"CASE WHEN ResExQ.Status = 4 THEN {INT} ELSE ResExQ.Color END AS Color, ShowStateColor, "
		"ResExQ.Status, ResExQ.CancelledDate, " // (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
		"PrimaryAppointmentInsuredPartyQ.InsuredPartyID AS PrimaryInsuredPartyID "
		"{CONST_STR} "
		"FROM ({SQL}) AS ResExQ "
		"LEFT JOIN ("
		"	SELECT AppointmentID, InsuredPartyID FROM AppointmentInsuredPartyT "
		"	WHERE Placement = 1 "
		") AS PrimaryAppointmentInsuredPartyQ ON ResExQ.ID = PrimaryAppointmentInsuredPartyQ.AppointmentID "
		"WHERE ResExQ.ID = {INT} "
		"AND ResExQ.ResourceID = {INT} "
		"AND ResExQ.StartTime >= {OLEDATETIME} AND ResExQ.StartTime < DATEADD(day, 1, {OLEDATETIME}) "
		"{SQL} "
		, m_nCancelColor
		, (bReadPurposes) ? ", dbo.GetPurposeString(ResExQ.ID) AS AptPurposeName" : ""
		, Nx::Scheduler::GetResSimpleSql()
		, nResID
		, m_pParent->GetActiveResourceID()
		, AsDateNoTime(dtMinDate), AsDateNoTime(dtMaxDate)
		, filter
	);

	if (prs->eof)
	{
		// eof means it should not appear in this view. So, we should remove it
		// from the visible names array.
		if (bFoundAppt)
		{
			m_astrVisibleNames[nDay].RemoveAt(nPos);
			m_adwVisibleColors[nDay].RemoveAt(nPos);
			bInvalidate = TRUE;
		}
		else if (bFoundEvent)
		{
			m_aEvents.RemoveAt(nPos);
			bInvalidate = TRUE;
		}
	}
	else
	{
		// The appointment should appear in our calendar. If we already have it,
		// just remove it since the cost between changing it and re-adding it
		// is negligible to the user, and this way it's easier to maintain in
		// code.
		if (bFoundAppt)
		{
			m_astrVisibleNames[nDay].RemoveAt(nPos);
			m_adwVisibleColors[nDay].RemoveAt(nPos);
		}
		else if (bFoundEvent)
		{
			m_aEvents.RemoveAt(nPos);
		}

		// Now we build the string and add it to the calendar list.
		FieldsPtr fldsMonth;
		FieldPtr fldResID, fldDate, fldStartTime, fldEndTime, fldPatientID, 
			fldFirstName, fldLastName, fldAptTypeID, fldAptTypeName, fldAptTypeColor,
			fldEventName, fldAptPurposeName;
		fldsMonth = prs->Fields;
		fldResID = fldsMonth->Item["ID"];
		fldStartTime = fldsMonth->Item["StartTime"];
		fldEndTime = fldsMonth->Item["EndTime"];
		fldPatientID = fldsMonth->Item["PatientID"];
		fldFirstName = fldsMonth->Item["First"];
		fldLastName = fldsMonth->Item["Last"];
		fldAptTypeID = fldsMonth->Item["AptTypeID"];
		fldAptTypeName = fldsMonth->Item["AptType"];
		fldAptTypeColor = fldsMonth->Item["Color"];
		fldEventName = fldsMonth->Item["Notes"];
		fldAptPurposeName = (bReadPurposes) ? fldsMonth->Item["AptPurposeName"] : NULL;

		COleDateTime dtStart = AdoFldDateTime(fldStartTime);
		if (dtStart == AdoFldDateTime(fldEndTime) &&
			dtStart.GetHour() == 0 && dtStart.GetMinute() == 0 && dtStart.GetSecond() == 0)
		{
			//////////////////////////////////////////////
			// Add a new visible event.
			_MONTHEVENT event;
			BuildCalendarEvent(event, fldResID, fldStartTime, fldEndTime, fldEventName, fldAptTypeColor);
			m_aEvents.Add(event);
		}
		else
		{
			//////////////////////////////////////////////
			// Add a new visible appointment
			CString str = BuildCalendarElement(fldResID, fldStartTime, fldEndTime, fldPatientID, fldFirstName,
				fldLastName, fldAptTypeID, fldAptTypeName, fldAptTypeColor, fldAptPurposeName);

			// Now figure out where in the day this entry belongs
			COleDateTime dtNewStart = AdoFldDateTime(fldStartTime);
			nDay = dtNewStart.GetDay() - 1;
			for (long nPlacement = 0; nPlacement < m_astrVisibleNames[nDay].GetSize(); nPlacement++)
			{
				CString strElement = m_astrVisibleNames[nDay][nPlacement];
				CString strStartTime = GetSubString(strElement, 4);
				strStartTime.Replace(":000", "");
				COleDateTime dtOldStart;
				dtOldStart.ParseDateTime(strStartTime);
				if (dtOldStart > dtNewStart)
					break;
			}

			m_astrVisibleNames[nDay].InsertAt(nPlacement, str);
			m_adwVisibleColors[nDay].InsertAt(nPlacement, AdoFldLong(fldAptTypeColor, 0));
		}

		// Figure out the vertical placement of each event.
		for (long i=0; i < m_aEvents.GetSize(); i++)
			m_aEvents[i].height = 0;
		for (i=1; i < m_aEvents.GetSize(); i++)
		{
			int iFirstDay = m_aEvents[i].iFirstDay;
			int iLastDay = m_aEvents[i].iLastDay;

			for (int j=0; j < i; j++)
			{
				int a = m_aEvents[j].iFirstDay;
				int b = m_aEvents[j].iLastDay;

				if (!((iFirstDay > b && iLastDay > b) ||
					(iFirstDay < a && iLastDay < a)))
				{
					m_aEvents[i].height++;
				}
			}
		}

		// Now flag the fact we need to redraw
		bInvalidate = TRUE;
	}

	// (c.haag 2003-10-20 12:25) - Redraw the calendar to make the changes take effect
	if (bInvalidate)
	{
		CRect rc(m_startx, m_starty, m_startx + m_daywidth*7, m_starty + m_dayheight*6);
		InvalidateRect(&rc, FALSE);
	}
}


// (a.walling 2010-06-23 09:41) - PLID 39263 - Update all columns' available location info
void CMonthDlg::UpdateColumnAvailLocationsDisplay()
{
	UpdateActiveDateLabel();
}

// (a.walling 2010-06-23 18:00) - PLID 39263 - Now a virtual function
void CMonthDlg::UpdateActiveDateLabel()
{
	CNxSchedulerDlg::UpdateActiveDateLabel();

	// Update the edit box just above it
	CWnd* pWnd = GetDlgItem(IDC_EDIT_ACTIVE_SINGLEDAY_DAY);
	// (c.haag 2010-11-16 13:49) - PLID 39444 - Also include any resource location abbreviations
	CString strNew = m_ResourceAvailLocations.GetDayLabel(0,FormatDateTimeForInterface(m_dtSingleDayDate, "%#x"));
	if (pWnd) {
		pWnd->SetWindowText(strNew);
	}
	
	// (a.walling 2010-06-23 09:41) - PLID 39263 - Also update the description
	CString strInfo = m_ResourceAvailLocations.GetDayDescription(0);

	if (pWnd && pWnd->IsKindOf(RUNTIME_CLASS(CNxStatic))) {
		CNxStatic* pStatic = (CNxStatic*)pWnd;

		pStatic->SetToolTip(strInfo);

		if (strInfo.IsEmpty()) {
			pStatic->SetColor(RGB(0x00, 0x00, 0x00));
		} else {
			pStatic->SetColor(RGB(0xC0, 0x00, 0x00));
		}
	} else {
		ASSERT(FALSE);
	}
}

void CMonthDlg::PrintDay(COleDateTime date, CRect rDayRect, CDC * pDC, bool bBlackAndWhite, bool bPrintMonthName, bool bIsActiveDay)
{
	CBrush *pOldBrush;
	CBrush greybrush(RGB(192, 192, 192));
	CBrush whitebrush(RGB(255, 255, 255));
	
	//First, we need to draw this rectangle.
	CStringArray *pastrNames;
	CDWordArray *padwColors; // (c.haag 2008-06-18 15:40) - PLID 26135

	if (!bIsActiveDay) {
		// Nope, so color it grey and make sure no appointments are printed
		pOldBrush = pDC->SelectObject(&greybrush);
		pDC->SetBkColor(RGB(192, 192, 192));
		pastrNames = NULL;
	} else {
		pOldBrush = pDC->SelectObject(&whitebrush);
		pDC->SetBkColor(RGB(255,255,255));
		// Yep, so get a pointer to the beginning of this day's array
		// (c.haag 2008-06-18 16:17) - PLID 26135 - Use the array based on the user's printing
		// preference
		if (m_pParent->m_bPrintTemplates) {
			pastrNames = &m_astrVisibleTemplateNames[date.GetDay()-1];
			padwColors = &m_adwVisibleTemplateColors[date.GetDay()-1];
		} else {
			pastrNames = &m_astrVisibleNames[date.GetDay()-1];
			padwColors = &m_adwVisibleColors[date.GetDay()-1];
		}
	}

	// Plot rectangle that represents this day
	pDC->Rectangle(&rDayRect);

	pDC->SelectObject(pOldBrush);

	long nDayHeaderBottom;

	LOGFONT lf;
	CFont* pFont = pDC->GetCurrentFont();
	pFont->GetLogFont(&lf);
	int textheight = lf.lfHeight;
	if (textheight < 0) {
		textheight = -textheight;
	} else if (textheight == 0) {
		textheight = 30;
	}

	//First, draw the header
	// Draw day header
	{
		CString strDayHdr;
		// Generate the day header text
		if (bPrintMonthName) {
			// This is the first VISIBLE day of the given month so
			// prepend the month's common name to the day value
			strDayHdr = FormatDateTimeForInterface(date, "%B %#d");
		} else {
			// This is a regular day so just use the day number
			strDayHdr = FormatDateTimeForInterface(date, "%#d");
		}
		// Draw the day header
		nDayHeaderBottom = DrawDayHeader(pDC, rDayRect.left, rDayRect.top, rDayRect.Width(), rDayRect.Height(), strDayHdr, bIsActiveDay);
	}

	// Now, Print day events
	if(bIsActiveDay) {
		long nEvents = 0;
				
		// (c.haag 2008-06-18 17:12) - PLID 26135 - If we're printing templates, don't print events
		if (!m_pParent->m_bPrintTemplates) {
			for (int i=0; i < m_aEvents.GetSize(); i++)
			{
				if (date.GetDay() >= m_aEvents[i].iFirstDay &&
					date.GetDay() <= m_aEvents[i].iLastDay)
				{
					// Set color of text
					COLORREF clr;

					if (bBlackAndWhite) {
						// Use black
						clr = RGB(0,0,0);
					} else {
						// Use purposeset color
						clr = m_aEvents[i].clr;
						// Tone down if too bright
						// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
						clr = GetDarkerColorForApptText(clr);
					}
					CBrush brush(clr), *pOldBrush;
					CPen pen(PS_SOLID, 1, clr), *pOldPen;
					pOldBrush = pDC->SelectObject(&brush);
					pOldPen = pDC->SelectObject(&pen);
					pDC->Rectangle(rDayRect.left, rDayRect.bottom - (textheight+3)*(nEvents+1),
						rDayRect.right, rDayRect.bottom- (textheight+3)*(nEvents));
					pDC->SelectObject(pOldPen);
					pDC->SelectObject(pOldBrush);
						pDC->SetTextColor(RGB(255,255,255));
					pDC->DrawText(m_aEvents[i].strName,
							CRect(rDayRect.left + 20, rDayRect.bottom - (textheight+3)*(nEvents+1) - 2, rDayRect.right, rDayRect.bottom), 
							DT_WORDBREAK|DT_TOP|DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS);
					nEvents++;
				}
			} // for (int i=0; i < m_aEvents.GetSize(); i++)
		} // if (m_pParent->m_bPrintTemplates) {
		long lEventTopY = rDayRect.bottom - (textheight+3)*(nEvents);

		// Print day appointments / templates
		{
			if (pastrNames != NULL && padwColors != NULL) {
				// Decide where the first appointment will display
				int text_y = nDayHeaderBottom;
				//Logic:  Are we scrolled to the bottom?  If we are, start at the bottom, otherwise,
				//start at the top appointment we're scrolled to.
				
				// Are we at the bottom?
				bool bScrollFromBottom = false;
				int nStart = m_nVisibleNamesOffset;
						
				if (nStart > 0) {
					// If we're starting anywhere but the first appointment, 
					// calculate the total height of all appointments and if 
					// they'll fit then start at 0
					if (DoesDayPrintOutputFit(pDC, pastrNames, rDayRect.Width()-20, rDayRect.Height()-textheight)) {
						// The output will fit
						nStart = 0;
					}

					if(nStart > 0) {
						int nMaxStart = pastrNames->GetSize() - m_nVisibleNamesPerDay + nEvents;
						if(nMaxStart < 0) nMaxStart = 0;
						if(nStart > nMaxStart) {
							nStart = nMaxStart;
							if(nStart > 0) {
								bScrollFromBottom = true;
							}
						}
					}

				}

				if(!bScrollFromBottom) {
					// If we are starting anywhere but the beginning 
					// of the list, put three dots at the right
					if (nStart > 0) {
						pDC->SetTextColor(RGB(0,0,0));
						pDC->TextOut(rDayRect.right - 110, rDayRect.top + 50, "***");
					}
					for (int i=nStart; i < pastrNames->GetSize() && text_y <= min(rDayRect.bottom - textheight, lEventTopY - textheight); i++) {
						// Set color of text
						COLORREF clr;
						if (bBlackAndWhite) {
							// Use black
							clr = RGB(0,0,0);
						} else {
							// Use purposeset color
							clr = (COLORREF)padwColors->GetAt(i);
							// Tone down if too bright
							// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
							clr = GetDarkerColorForApptText(clr);
						}
						pDC->SetTextColor( clr );
						
						//First, see if this is going to be too large.
						int nApptHeight = PrintAppt(pDC, pastrNames->GetAt(i), 
							CRect(rDayRect.left + 20, text_y, rDayRect.right, rDayRect.bottom), 
							TRUE);
						if(text_y + nApptHeight <= min(rDayRect.bottom - textheight, lEventTopY - textheight)) {
							//We're safe, we can print this appointment.
							text_y += PrintAppt(pDC, pastrNames->GetAt(i), 
								CRect(rDayRect.left + 20, text_y, rDayRect.right, rDayRect.bottom), 
								FALSE);
						}
						else {
							text_y += nApptHeight; //This is necessary to break out of the loop.
							i--;//The next iteration of the for loop will increase this, even though we didn't actually print an appt.
						}
					}
					int nTempDay = date.GetDay();
					if (i < pastrNames->GetSize()) {
						pDC->SetBkColor(RGB(255,255,255));
						pDC->SetTextColor(RGB(0,0,0));
						pDC->ExtTextOut(rDayRect.right - 110, lEventTopY- textheight, ETO_OPAQUE, CRect(rDayRect.right-110, lEventTopY-textheight, rDayRect.right-1, lEventTopY-1), "***", NULL);
					}
				}
				else {
					//OK, here's what we need to do.  Start with the last appt, and do CalcRects
					//until we get to an appointment that will run off the top of the screen.  Then, starting
					//with the last appointment to fit, go through and print them all out.
					long nTotalHeight = nDayHeaderBottom;
					for(int i = pastrNames->GetSize() - 1; i >= 0 && nTotalHeight <= min(rDayRect.bottom - textheight, lEventTopY - textheight); i--) {
						nTotalHeight += PrintAppt(pDC, pastrNames->GetAt(i), rDayRect, TRUE);
					}
					if(nTotalHeight <= min(rDayRect.bottom - textheight, lEventTopY - textheight)) {
						//It all fit!
						i = 0;
					}
					else {
						//The last iteration was an appointment that didn't fit, plus the for loop subtracted again, so...
						i = i+2;
					}

					if(i != 0) {
						//This should always happen, if we're scrolling from the bottom.
						pDC->SetTextColor(RGB(0,0,0));
						pDC->TextOut(rDayRect.right - 110, rDayRect.top + 50, "***");
					}

					
					//Now, just go through and output.
					text_y = nDayHeaderBottom;
					while(i < pastrNames->GetSize() && text_y <= min(rDayRect.bottom - textheight, lEventTopY - textheight)) {

						COLORREF clr;
						if (bBlackAndWhite) {
							// Use black
							clr = RGB(0,0,0);
						} else {
							// Use purposeset color
							clr = (COLORREF)padwColors->GetAt(i);
							// Tone down if too bright
							// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
							clr = GetDarkerColorForApptText(clr);
						}
						pDC->SetTextColor( clr );
						
						//First, see if this is going to be too large.
						int nApptHeight = PrintAppt(pDC, pastrNames->GetAt(i), 
							CRect(rDayRect.left + 20, text_y, rDayRect.right, rDayRect.bottom), 
							TRUE);
						if(text_y + nApptHeight <= min(rDayRect.bottom - textheight, lEventTopY - textheight)) {
							//We're safe, we can print this appointment.
							text_y += PrintAppt(pDC, pastrNames->GetAt(i), 
								CRect(rDayRect.left + 20, text_y, rDayRect.right, rDayRect.bottom), 
								FALSE);
							i++;
						}
						else {
							//This should never happen since we're scrolling from the bottom, but it's here for completeness.
							text_y += nApptHeight; //This is necessary to break out of the loop.
						}
					}
					
					if (i < pastrNames->GetSize()) {
						//This should never happen since we're scrolling from the bottom, but it's here for completeness.
						pDC->SetBkColor(RGB(255,255,255));
						pDC->SetTextColor(RGB(0,0,0));
						pDC->ExtTextOut(rDayRect.right - 110, lEventTopY - textheight, ETO_OPAQUE, CRect(rDayRect.right-110, lEventTopY-textheight, rDayRect.right-1, lEventTopY-1), "***", NULL);
					}
	
				}

				//Finally, because sometimes with the zooming and what-not things get out of whack, let's redraw our rectangle.
				CBrush bTransparent, *pOldBrush;
				bTransparent.CreateStockObject(NULL_BRUSH);
				pOldBrush = pDC->SelectObject(&bTransparent);
				pDC->Rectangle(rDayRect);
				pDC->SelectObject(pOldBrush);


			}
		}
	}
}

void CMonthDlg::PaintDay(CDC& dc, const CRect& rcBounds, const COleDateTime& dt)
{
	// (c.haag 2006-04-11 13:38) - PLID 20081 - We now paint the day in three sections
	//
	PaintDayBackground(dc, rcBounds, dt);
	PaintDayHeader(dc, rcBounds, dt);
	PaintDayAppointments(dc, rcBounds, dt);
}

void CMonthDlg::PaintDayBackground(CDC& dc, const CRect& rcBounds, const COleDateTime& dt)
{
	CBrush greenbrush(RGB(192, 255, 192));
	CBrush greybrush(RGB(192, 192, 192));
	CBrush whitebrush(RGB(255, 255, 255));
	//TES 1/5/2012 - PLID 47310 - Added a preference to highlight days based on Location Templates
	long nHighlightColor = GetRemotePropertyInt("Scheduler_Month_HighlightAvailableDaysColor", RGB(255,255,128), 0, GetCurrentUserName());
	CBrush highlightbrush(nHighlightColor);
	CBrush* pOldBrush = NULL;
	//
	// Grey out unrelated months
	//
	if (dt.GetMonth() != GetActiveDate().GetMonth()) {
		pOldBrush = dc.SelectObject(&greybrush);
		dc.SetBkColor(RGB(192, 192, 192));
	}
	else {
		// (r.gonet 2014-12-17) - PLID 64428 - Get the resource ID as well.
		long nResourceID = -1;
		CString strRes = "";
		if(m_dlResources->GetCurSel() != -1) {
			nResourceID = VarLong(m_dlResources->GetValue(m_dlResources->GetCurSel(), 0));
			strRes = VarString(m_dlResources->GetValue( m_dlResources->GetCurSel(), 2));//m_ResourceCombo.GetText();
		}
		//
		// Default related month to white
		//
		pOldBrush = dc.SelectObject(&whitebrush);
		dc.SetBkColor(RGB(255, 255, 255));
		//(s.dhole 6/1/2015 4:18 PM ) - PLID 65645  Remove 
		//if(m_IsShowingFirstAvail) {
		//	//
		//	// Green out non-reserved days
		//	//
		//	// (r.gonet 2014-12-17) - PLID 64428 - IsDayAvailable now needs the resource ID as well in
		//	// order to check if a day is open for a resource.
		//	if (GetMainFrame()->m_FirstAvailAppt.IsDayAvailable(dt, nResourceID, strRes)) {
		//		dc.SelectObject(&greenbrush);
		//		dc.SetBkColor(RGB(192, 255, 192));							
		//	}
		//}
		//TES 1/5/2012 - PLID 47310 - If we're not higlighting available days based on FFA, and if the preference to highlight them is turned
		// on...
		//else if(GetRemotePropertyInt("Scheduler_Month_HighlightAvailableDays", 1, 0, GetCurrentUserName())) {
		if (GetRemotePropertyInt("Scheduler_Month_HighlightAvailableDays", 1, 0, GetCurrentUserName())) {
			//TES 1/5/2012 - PLID 47310 - And if we're filtering on a location, and this resource is available for that location on this day...
			long nLocationID = GetLocationId();
			if(nLocationID > 0) {
				if(m_ResourceAvailLocations_Month.IsAvailableForDay(dt.GetDay(), nLocationID)) {
					//TES 1/5/2012 - PLID 47310 - Highlight the background.
					dc.SelectObject(&highlightbrush);
					dc.SetBkColor(nHighlightColor);
				}
			}
			// (d.singleton 2012-07-05 15:12) - PLID 47473 support multi location selection
			else if(nLocationID == -2) {
				CDWordArray dwaryLocationIDs;
				GetLocationFilterIDs(&dwaryLocationIDs);
				for(int i = 0; i < dwaryLocationIDs.GetCount(); i++) {
					if(m_ResourceAvailLocations_Month.IsAvailableForDay(dt.GetDay(), dwaryLocationIDs.GetAt(i))) {
						dc.SelectObject(&highlightbrush);
						dc.SetBkColor(nHighlightColor);
						break;
					}
				}
			}
		}
	}
	//
	// Plot the rectangle
	//
	dc.Rectangle(&rcBounds);
	dc.SelectObject(pOldBrush);
}

void CMonthDlg::PaintDayHeader(CDC& dc, const CRect& rcBounds, const COleDateTime& dt)
{
	//
	// Print the day or month name
	//
	CString strDay;
	strDay.Format("%d", dt.GetDay());
	if (dt.GetDay() == 1 || (rcBounds.left == m_startx && rcBounds.top == m_starty)) {
		strDay = FormatDateTimeForInterface(dt, "%B ") + strDay;
	}
	dc.SetTextColor( RGB(0,0,0) );
	dc.TextOut(rcBounds.left + 2, rcBounds.top + 1, strDay);
}

void CMonthDlg::PaintDayAppointments(CDC& dc, const CRect& rcBounds, const COleDateTime& dt)
{
	//
	// Don't paint appointments if we're not in the same month
	//
	if (dt.GetMonth() != GetActiveDate().GetMonth()) {
		return;
	}

	//
	// First, we need to run some calculations. Here are the important ones:
	//
	// nNonEventSlots - This is the number of slots we are allocating for painting non-event appointments.
	// A slot corresponds a single line of text. While a non-event appointment could take up multiple slots
	// due to word wrapping, the important thing to know is how many slots we have total for clipping reasons.
	//
	// nEventSlots - The number of slots we are allocating for painting event appointments. A slot corresponds
	// to a single line of text. Events do not word wrap, so they will always take one slot each.
	//
	// nFirstVisibleNonEvent - The index to the first non-event appointment we will be painting
	//
	// nFirstVisibleEvent - The index to the first event appointment we will be painting
	//
	CStringArray* pastrNames = &m_astrVisibleNames[dt.GetDay()-1];
	CArray<_DRAGDROPRGN, _DRAGDROPRGN>* pddrgn = &m_aDragDropRgn[dt.GetDay()-1];
	int nEventsInDay = GetEventCountInDay(dt.GetDay());
	int nNonEventSlots = max(1, m_nVisibleNamesPerDay - nEventsInDay); // Force at least one name to be visible
	int nEventSlots = min(m_nVisibleNamesPerDay - nNonEventSlots, nEventsInDay);
	int nFirstVisibleNonEvent = min(pastrNames->GetSize(), m_nVisibleNamesOffset);
	int nFirstVisibleEvent = max(0, nEventsInDay - (m_nVisibleNamesPerDay - nNonEventSlots));
	BOOL bShowPurpose = GetRemotePropertyInt("SchedulerShowMonthViewPurposes", 0, 0, GetCurrentUserName(), false);
	nFirstVisibleEvent = max(nFirstVisibleEvent - m_nVisibleNamesOffset, 0);
	BOOL bEnableSpinner = FALSE;

	//
	// If we are starting anywhere but the beginning of the list, put three dots at the right
	//
	if (nFirstVisibleNonEvent > 0) {
		dc.SetTextColor(RGB(0,0,0));
		dc.TextOut(rcBounds.left + m_daywidth - 11, rcBounds.top + 6, "***");
		bEnableSpinner = TRUE;
	}

	//
	// Calculate the bounding rectangle of text for non-events and events
	//
	CRect rcNonEventClip(rcBounds.left, rcBounds.top,
		rcBounds.right, rcBounds.bottom - (nEventSlots * m_nHeightAppointmentFont));
	CRect rcEventClip(rcBounds.left, rcBounds.bottom - (nEventSlots * m_nHeightAppointmentFont),
		rcBounds.right, rcBounds.bottom);

	//
	// Now paint non-event appointments
	//
	int i = nFirstVisibleNonEvent;
	int y = rcNonEventClip.top + 1 + m_nHeightAppointmentFont;
	int prevy = y;
	for (; i < pastrNames->GetSize() && y < rcNonEventClip.bottom; i++) {
		_DRAGDROPRGN ddrgn;

		// Format appointment string
		//
		CString str = pastrNames->GetAt(i);
		CString strPurpose = GetSubString(str, 7);
		ddrgn.nNameIndex = i;
		int index = str.Find('|');
		str.SetAt(index, ' ');
		index = str.Find('|');
		str = str.Left( index );

		// Print purpose in place of blank name
		//
		if (str == " ") {
			str = pastrNames->GetAt(i);
			str = str.Right( str.GetLength()-2 );
			str = str.Left( str.Find('|') );
			str = "[" + str + "]";
		}

		// Get the text color
		//
		COLORREF clr = (COLORREF)m_adwVisibleColors[dt.GetDay() - 1].GetAt(i);
		// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
		clr = GetDarkerColorForApptText(clr);
		dc.SetTextColor( clr );

		// Calculate the text region
		//
		CRect rcText(rcBounds.left + 2, y, rcBounds.left + m_daywidth, y + 1);
		dc.DrawText(str, &rcText, DT_LEFT | DT_SINGLELINE | DT_CALCRECT);
		
		// Paint the non-event appointment text if it fully fits in our clipping area
		//
		if (rcText.bottom <= rcNonEventClip.bottom) {
			prevy = y;
			dc.DrawText(str, rcText, DT_LEFT | DT_SINGLELINE);

			// Assign our drag and drop region
			ddrgn.top = y;
			ddrgn.bottom = rcText.bottom;
		} else {
			break;
		}

		// (c.haag 2006-04-11 14:57) - PLID 20081 - Print the purpose
		//
		if (bShowPurpose && strPurpose.GetLength()) {
//			CFont fntText;
//			LOGFONT lf;
//			fntText.CreatePointFont(80, "MS Serif", &dc);
//			fntText.GetLogFont(&lf);
//			lf.lfItalic = TRUE;
//			fntText.DeleteObject();
//			fntText.CreateFontIndirect(&lf);
//			CFont* pOldFont = (CFont*)dc.SelectObject(&fntText);

			strPurpose = CString("(") + strPurpose + ")";

			rcText = CRect(rcBounds.left + 2, rcText.bottom, rcBounds.left + m_daywidth, rcText.bottom + 1);
			dc.DrawText(strPurpose, &rcText, DT_LEFT | DT_WORDBREAK | DT_CALCRECT);
			if (rcText.bottom <= rcNonEventClip.bottom) {
				prevy = rcText.bottom - m_nHeightAppointmentFont;
				dc.DrawText(strPurpose, rcText, DT_LEFT | DT_WORDBREAK);

				// Update our drag and drop region
				ddrgn.top = y;
				ddrgn.bottom = rcText.bottom;
			} else {
				break;
			}

//			dc.SelectObject(pOldFont);
//			fntText.DeleteObject();
		}

		// Add our drag and drop region to the sheet so that the user can
		// drag and drop from this location
		// 
		pddrgn->Add(ddrgn);

		// Update our y variable so we know exactly where to start next time
		//
		y = rcText.bottom;
	}

	// Enable the spinner if there were more non-event appointments to draw
	//
	if (i < pastrNames->GetSize()) {
		dc.SetTextColor(RGB(0,0,0));
		dc.TextOut(rcBounds.left + m_daywidth - 11, prevy + 3, "***");
		m_bAllowScrollDown = TRUE;
		bEnableSpinner = TRUE;
	}

	//
	// Now paint events
	//
	if (nEventsInDay > 0) {
		CArray<_MONTHEVENT, _MONTHEVENT> aEvents;
		GetEventsInDay(dt.GetDay(), aEvents);
		if (nFirstVisibleEvent > 0)	{
			m_bAllowScrollDown = TRUE;
			bEnableSpinner = TRUE;
		}

		for (i = nFirstVisibleEvent, y = rcEventClip.top; i < min(nFirstVisibleEvent + nEventSlots, aEvents.GetSize()); i++)
		{
			_MONTHEVENT& ev = aEvents[i];
			COLORREF clr = ev.clr;
			CBrush brush(ev.clr);
			CPen pen(PS_SOLID, 1, ev.clr);
			CBrush* pOldBrush = (CBrush*)dc.SelectObject(&brush);
			CBrush* pOldPen = (CBrush*)dc.SelectObject(&pen);

			ev.x = rcBounds.left + 1;
			ev.y = rcEventClip.bottom - m_nHeightAppointmentFont * (1 + (ev.height - nFirstVisibleEvent));

			dc.Rectangle(rcEventClip.left, rcEventClip.bottom - m_nHeightAppointmentFont * (1 + (ev.height - nFirstVisibleEvent)),
				rcEventClip.right, rcEventClip.bottom - m_nHeightAppointmentFont * (ev.height - nFirstVisibleEvent));
			dc.SelectObject(pOldBrush);
			dc.SelectObject(pOldPen);

			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(RGB(255,255,255));
			dc.TextOut(ev.x, ev.y, ev.strName);
		}
	}

	//
	// Enable the spinner if necessary
	//
	if (bEnableSpinner && !GetDlgItem(IDC_SPIN_NAME_OFFSETS)->IsWindowEnabled()) {
		GetDlgItem(IDC_SPIN_NAME_OFFSETS)->EnableWindow(TRUE);
	}
}

// (a.walling 2007-11-06 17:22) - PLID 27998 - VS2008 - strAppt should be const
long CMonthDlg::PrintAppt(CDC *pDC, const CString &strAppt, CRect rAppt, BOOL bCalcRect /*= FALSE*/)
{
	//First, get the time, bold it, textout it.
	LOGFONT lfBold;
	pDC->GetCurrentFont()->GetLogFont(&lfBold);
	strcpy(lfBold.lfFaceName, "Arial Black");
	CFont fNew, *pfOld;
	fNew.CreateFontIndirect(&lfBold);
	pfOld = pDC->SelectObject(&fNew);

	//We have a bold font, let's get the time.
	CString strStartTime = GetSubString(strAppt, 4);
	//Astoundingly, if we include the :000 for milliseconds, COleDateTime can't parse it!!.
	strStartTime = strStartTime.Left(strStartTime.GetLength()-4);
	COleDateTime dt;
	dt.ParseDateTime(strStartTime);
	if (dt.GetStatus() == COleDateTime::valid) {
		//Format it, then replace the long marker (pm) with the short marker (p)
		strStartTime = FormatDateTimeForInterface(dt, DTF_STRIP_SECONDS, dtoTime);
		strStartTime.Replace("am", "a");
		strStartTime.Replace("pm", "p");
		strStartTime.Replace("AM", "a");
		strStartTime.Replace("PM", "p");
	}

	long nHeight = 0;
	long nTimeWidth = 0;
	CRect rLine = rAppt;
	if(bCalcRect) {
		//We're making this part only one line, and the height (which is all we care about atm) is determined by the larger font.
		nHeight = pDC->DrawText(strStartTime, rLine, DT_SINGLELINE|DT_TOP|DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS|DT_CALCRECT);
		pDC->SelectObject(pfOld);
	}
	else {
		CRect rTime;
		pDC->DrawText(strStartTime, rTime, DT_CALCRECT|DT_SINGLELINE|DT_TOP|DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS);
		nHeight = pDC->DrawText(strStartTime, rLine, DT_SINGLELINE|DT_TOP|DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS);
		pDC->SelectObject(pfOld);
		rLine.left = rLine.left + rTime.Width() + 10; //10 is an arbitrary space between time and name.
		rLine.bottom = rLine.top + nHeight;
		pDC->DrawText(GetSubString(strAppt, 0) + " " + GetSubString(strAppt, 1), rLine, DT_SINGLELINE|DT_BOTTOM|DT_END_ELLIPSIS|DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS);
	}

	// Append the purpose if there is a patient ID, a purpose, and we want to show purposes
	if (m_pParent && m_pParent->m_bShowMonthProcedures && !GetSubString(strAppt, 1).IsEmpty() && !GetSubString(strAppt, 7).IsEmpty())
	{
		//Shift our drawing area down.
		rAppt.top += nHeight;
		if(bCalcRect) {
			nHeight += pDC->DrawText(GetSubString(strAppt, 7), rAppt, DT_CALCRECT|DT_WORDBREAK|DT_END_ELLIPSIS|DT_TOP|DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS);
		}
		else {
			nHeight += pDC->DrawText(GetSubString(strAppt, 7), rAppt, DT_WORDBREAK|DT_END_ELLIPSIS|DT_TOP|DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS);
		}
		
	}

	return nHeight;
}

void CMonthDlg::Internal_GetWorkingResourceAndDateFromGUI(IN const long nColumnIndex, OUT long &nWorkingResourceID, OUT COleDateTime &dtWorkingDate) const
{
	// The month view is basically a one-day view with a massive sidebar containing a month 
	// calendar.  So we treat this exactly like the one-day view since it only supports one 
	// column in the singleday controls.

	// Since we only support one column in the singleday controls, we must fail if we're 
	// given a column index other than the 0th column
	ASSERT(nColumnIndex == 0);
	if (nColumnIndex == 0) {

		if(m_dlResources->GetCurSel() == -1) {
			m_dlResources->CurSel = 0;

			if(m_dlResources->GetCurSel() == -1) {
				ThrowNxException(
				"CMonthDlg::Internal_GetWorkingResourceAndDateFromGUI: There are no resources in the current view.");
			}
		}

		// The resource is simply the one active resource
		nWorkingResourceID = VarLong(m_dlResources->GetValue(m_dlResources->GetCurSel(), ResourceComboColumns::ID));

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
	} else {
		// We were asked for a column other than the 0th column
		ThrowNxException(
			"CMonthDlg::Internal_GetWorkingResourceAndDateFromGUI: The month view only has one "
			"column.  No information can be given for column index %li", nColumnIndex);
	}
}

BOOL CMonthDlg::Internal_IsMultiResourceSheet() const
{
	return FALSE;
}

BOOL CMonthDlg::Internal_IsMultiDateSheet() const
{
	return FALSE;
}

BOOL CMonthDlg::PreTranslateMessage(MSG *pMsg) {
	if(pMsg->message == WM_KEYDOWN) {
		switch(pMsg->wParam) {
		case VK_ESCAPE:
			TRACE("VK_ESCAPE\n");
			m_strDragInfo.Empty();
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			break;
		case VK_SHIFT:
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			break;
		}
	}
	else if(pMsg->message == WM_KEYUP) {
		switch(pMsg->wParam) {
		case VK_SHIFT:
			if(!m_strDragInfo.IsEmpty()) {
				SetCursor(AfxGetApp()->LoadCursor(IDC_POINTER_COPY));
			}
			break;
		}
	}
	return CNxSchedulerDlg::PreTranslateMessage(pMsg);
}

void CMonthDlg::UpdatePurposes()
{
	try
	{
		if (GetRemotePropertyInt("SchedulerShowMonthViewPurposes", 0, 0, GetCurrentUserName(), false)) {
			// (c.haag 2006-04-11 15:44) - PLID 19849 - If this preference is set, there's no need to
			// update the purposes because they already exist in the schedule.
			return;
		}

		// Build SQL to span current month
		CWaitCursor wc;
		CString strSqlWhere = GetWhereClause();

		//DRT 1/2/2004 - PLID 10574 - Fixed a bug which was causing this query to eliminate records which had the same start
		//	and end time, but were not events.  The delimited names list that is compared to below did not eliminate these
		//	records, which caused an exception.
		//TES 5/24/2004 - MAKE SURE that this query will always order in the exact same way as the one in UpdateCalendarNames.
		_RecordsetPtr prsMonthView = CreateRecordset(
			"SELECT AppointmentsT.ID AS ResID, AppointmentsT.Date, dbo.GetPurposeString(AppointmentsT.ID) AS AptPurposeName "
			"FROM PersonT "
			"INNER JOIN AppointmentsT ON PersonT.ID = AppointmentsT.PatientID "
			"LEFT OUTER JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
			"WHERE %s AND (CONVERT (datetime, CONVERT (varchar, StartTime, 8)) <> convert(datetime, '12:00:00 AM') OR CONVERT (datetime, CONVERT (varchar, StartTime, 8)) <> CONVERT (datetime, CONVERT (varchar, EndTime, 8))) ORDER BY AppointmentsT.StartTime, AppointmentsT.ID", strSqlWhere);
		FieldsPtr fldsMonth;
		FieldPtr fldResID, fldDate, fldAptPurposeName;
		// Get the fields collection
		fldsMonth = prsMonthView->Fields;
		// Load all the field references
		fldResID = fldsMonth->Item["ResID"];
		fldDate = fldsMonth->Item["Date"];
		fldAptPurposeName = fldsMonth->Item["AptPurposeName"];

		CString str;
		// (c.haag 2003-10-16 17:07) - Ordinals for counting what name
		// we are at in event and appointment arrays
		long nEvent = 0;
		long anAppt[32]; // 31 CStringArrays, days are 1 based.

		memset(anAppt, 0, sizeof(anAppt));
		while (!prsMonthView->eof)
		{
			// Get day of appointment
			COleDateTime dt = AdoFldDateTime(fldDate);
			long iDay = dt.GetDay();
			CString strDelimitedName = m_astrVisibleNames[iDay-1][ anAppt[iDay] ];

			// Make sure the appointment ID matches
			str = GetSubString(strDelimitedName, 6);
			if (atoi(str) != AdoFldLong(fldResID))
				ThrowNxException("Failed to match the calendar names with appointment purposes");

			// Change the purpose string from the place holder to
			// what it should actually be
			SetSubString(strDelimitedName, AdoFldString(fldAptPurposeName, ""), 7);
			m_astrVisibleNames[iDay-1][ anAppt[iDay] ] = strDelimitedName;

			// Go to the next day
			anAppt[iDay]++;

			prsMonthView->MoveNext();
		}
	}
	NxCatchAll("Error preparing printing for the calendar");
}

void CMonthDlg::PrePrint()
{
	UpdatePurposes();
}

void CMonthDlg::PostPrint()
{
}

void CMonthDlg::BuildCalendarEvent(_MONTHEVENT& event, FieldPtr& fldResID,
										FieldPtr& fldStartTime, FieldPtr& fldEndTime,
										FieldPtr& fldEventName, FieldPtr& fldAptTypeColor)
{
	COleDateTime dtFirst = AdoFldDateTime(fldStartTime);
	COleDateTime dtLast = AdoFldDateTime(fldEndTime);

	// Set the first and last days of the event
	event.iFirstDay = dtFirst.GetDay();
	if (dtLast.GetYear() == dtFirst.GetYear() && dtLast.GetMonth() == dtFirst.GetMonth())
	{
		event.iLastDay = dtLast.GetDay();
	}
	else event.iLastDay = 28; // TODO!!!

	// Set the name and color. It is possible for
	// an event, which relies on the purpose type,
	// to have no purpose name.
	if (fldEventName->Value.vt != VT_NULL)
		event.strName = AdoFldString(fldEventName);
	else
		event.strName = "";

	if (fldAptTypeColor->Value.vt == VT_NULL)
		event.clr = 0;
	else
		event.clr = AdoFldLong(fldAptTypeColor, 0);

	// Assign the appointment ID
	event.nResID = AdoFldLong(fldResID);

	// Reset height
	event.height = 0;
}

CString CMonthDlg::BuildCalendarElement(FieldPtr& fldResID,
										FieldPtr& fldStartTime, FieldPtr& fldEndTime,
										FieldPtr& fldPatientID, FieldPtr& fldFirstName,
										FieldPtr& fldLastName, FieldPtr& fldAptTypeID,
										FieldPtr& fldAptTypeName, FieldPtr& fldAptTypeColor,
										FieldPtr& fldAptPurposeName)
{
	CString strFirstName, strLastName;
	CString strStartTime, strEndTime;

	// Get patient name of appointment
	int iPatientID = AdoFldLong(fldPatientID);
	int iAptTypeID = AdoFldLong(fldAptTypeID, 0);

	if(!m_pParent->m_bShowPatientInformation) {
		//Not showing names, use type information instead.
		strLastName.Empty();
		strFirstName = AdoFldString(fldAptTypeName, "{No Type}");
	}
	else if (iPatientID != -25) {
		// Get patient first and last name
		strFirstName = "";	//Fixed crash on no first name 4/19 BVB
		strFirstName = AdoFldString(fldFirstName, "");
		if (strFirstName != "")
			strFirstName = strFirstName.Left(1);
		strLastName = AdoFldString(fldLastName);
	} else {
		// This isn't a patient record so use the type information instead
		strLastName.Empty();
		strFirstName = AdoFldString(fldAptTypeName, "[Block]");
	}

	strStartTime = FormatDateTimeForSql(AdoFldDateTime(fldStartTime));
	strEndTime = FormatDateTimeForSql(AdoFldDateTime(fldEndTime));

	// Build string like this: "B|Spam|AptTypeID|234|8:00:00 AM|9:30:00 AM|4758|AptPurpose"
	// Attach ID and names to string
	CString str;
	str.Format("%s|%s|%d|%d|%s|%s|%d|%s|",
		strFirstName, strLastName, iAptTypeID, 
		iPatientID, strStartTime, strEndTime, AdoFldLong(fldResID),
		(NULL == fldAptPurposeName) ? "" : AdoFldString(fldAptPurposeName, ""));
	return str;
}

// (c.haag 2008-06-18 17:14) - PLID 26135 - Creates text for a template record from
// PrepareTemplatePrint()
CString CMonthDlg::BuildTemplateElement(FieldsPtr& f)
{
	const COleDateTime dtStart = AdoFldDateTime(f, "StartTime");
	const COleDateTime dtEnd = AdoFldDateTime(f, "EndTime");
	const CString strStartTime = FormatDateTimeForSql(dtStart);
	const CString strEndTime = FormatDateTimeForSql(dtEnd);

	// Build string like this: "B|Spam|AptTypeID|234|8:00:00 AM|9:30:00 AM|4758|AptPurpose"
	// Attach ID and names to string
	CString str;
	str.Format("%s|%s|%d|%d|%s|%s|%d|%s|",
		"", AdoFldString(f, "Name"), -1, 
		-25, strStartTime, strEndTime, -1,
		"");
	return str;
}

// (c.haag 2008-06-18 16:22) - PLID 26135 - This function is called before a print
// or print preview where the user wants to print template names in the month view.
// It will run a query to fill in m_astrVisibleTemplateNames and 
// m_adwVisibleTemplateColors.
void CMonthDlg::PrepareTemplatePrint()
{
	try {
		CWaitCursor wc;

		int i;
		for (i=0; i < 31; i++) {
			m_astrVisibleTemplateNames[i].RemoveAll();
			m_adwVisibleTemplateColors[i].RemoveAll();
		}

		// Set dt to the first day of the active month
		COleDateTime dt = GetActiveDate();
		dt.SetDate(dt.GetYear(), dt.GetMonth(), 1);

		// Unique temp table name within this connection (other connections have their own names)
		CString strTempT1, strTempT2;
		strTempT1.Format("#tmpMonthTemplatePrint1%lu", GetTickCount());
		strTempT2.Format("#tmpMonthTemplatePrint2%lu", GetTickCount());

		// Get the resource ID
		long nWorkingResourceID = VarLong(m_dlResources->GetValue(m_dlResources->GetCurSel(), ResourceComboColumns::ID));

		// Now for the magic: Run the query to get the templates for each day
		// (c.haag 2010-04-08 17:42) - PLID 37565 - Added "TemplateID" to the first temp table
		// (d.lange 2010-12-19 12:14) - PLID 41900 - Added "OverrideLocationTemplating" and "LocationID" to the first temp table
		// (a.walling 2014-04-21 14:47) - PLID 60474 - TemplateHitAllP - open in snapshot isolation, at least for consistency, even if this is a rare path
		// (r.gonet 2015-03-13 13:52) - PLID 65273 - Added missing template collection columns to the temp table
		// that holds the output of TemplateHitAllP.
		_RecordsetPtr prs = CreateRecordset(GetRemoteDataSnapshot(), 
			"SET NOCOUNT ON \r\n"
			" \r\n"
			"DECLARE @Date DATETIME; \r\n"
			"DECLARE @LastDate DATETIME; \r\n"
			"SET @Date = '%s'; \r\n"
			"SET @LastDate = DATEADD(D, -1, DATEADD(M, 1, @Date));  \r\n"
			" \r\n"
			"CREATE TABLE %s (TemplateID INT NOT NULL, Name nvarchar(255), TemplateLineItemID INT, StartTime DATETIME, EndTime DATETIME, \r\n"
			"Color INT, IsBlock INT, Priority INT, RuleID INT, AndDetails INT, AllAppts INT, OverrideLocationTemplating BIT, ObjectType INT, \r\n"
			"ObjectID INT, ResourceID INT, AllResources INT, LocationID INT, CollectionID INT, CollectionName NVARCHAR(50)) \r\n"
			"CREATE TABLE %s (Date DATETIME, Name nvarchar(255), StartTime DATETIME, EndTime DATETIME, \r\n"
			"Color INT, ResourceID INT, AllResources INT) \r\n"
			" \r\n"
			"WHILE @Date <= @LastDate \r\n"
			"BEGIN \r\n"
			"INSERT INTO %s exec TemplateHitAllP @Date,N'%d',1 \r\n"
			"INSERT INTO %s ([Date], Name, StartTime, EndTime, Color, ResourceID, AllResources) \r\n"
			"SELECT @Date, Name, StartTime, EndTime, Color, ResourceID, AllResources FROM %s \r\n"
			"GROUP BY Name, StartTime, EndTime, Color, ResourceID, AllResources \r\n"
			"ORDER BY StartTime \r\n"
			"DELETE FROM %s \r\n"
			"SET @Date = DATEADD(d, 1, @Date); \r\n"
			"END \r\n"
			"SET NOCOUNT OFF \r\n"
			" \r\n"
			"SELECT * FROM %s \r\n"
			" \r\n"
			"SET NOCOUNT ON \r\n"
			"DROP TABLE %s \r\n"
			"DROP TABLE %s \r\n"
			"SET NOCOUNT OFF \r\n"
			,FormatDateTimeForSql(dt) // SET @Date =
			,strTempT1 // Create table
			,strTempT2 // Create table
			,strTempT1 // Insert into ... exec
			,nWorkingResourceID // N' 
			,strTempT2 // Insert into ... ([Date
			,strTempT1 // Select ... from
			,strTempT1 // Delete from
			,strTempT2 // Select * from
			,strTempT1 // Drop table
			,strTempT2 // Drop table
			);
		FieldsPtr f = prs->Fields;
		while (!prs->eof) {
			const COleDateTime dtDate = AdoFldDateTime(f, "Date");
			const int nDay = dtDate.GetDay() - 1;
			const CString strText = BuildTemplateElement(f);

			m_astrVisibleTemplateNames[nDay].Add(strText);
			m_adwVisibleTemplateColors[nDay].Add(AdoFldLong(f, "Color", 0));

			prs->MoveNext();
		}
	}
	NxCatchAll("Error in CMonthDlg::PrepareTemplatePrint");
}

void CMonthDlg::UpdateBlocks(bool bUpdateTemplateBlocks, bool bForceUpdate, bool bLoadAllTemplateInfoAtOnce)
{
	//TES 12/17/2008 - PLID 32497 - Don't bother with this in Scheduler Standard, as the singleday is hidden.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
		return;
	}

	CNxSchedulerDlg::UpdateBlocks(bUpdateTemplateBlocks, bForceUpdate, bLoadAllTemplateInfoAtOnce);
}

void CMonthDlg::UpdateBlocks(bool bUpdateTemplateBlocks, bool bForceUpdate)
{
	//TES 12/17/2008 - PLID 32497 - Don't bother with this in Scheduler Standard, as the singleday is hidden.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
		return;
	}

	CNxSchedulerDlg::UpdateBlocks(bUpdateTemplateBlocks, bForceUpdate);
}

void CMonthDlg::EnableSingleDayControls()
{
	//TES 12/18/2008 - PLID 32497 - If we're in Scheduler Standard mode, then we always want our controls to be disabled.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
		return;
	}

	CNxSchedulerDlg::EnableSingleDayControls();
}

// (c.haag 2009-12-22 14:02) - PLID 28977 - The user can now click on the count label to
// configure how counts are calculated
void CMonthDlg::DoClickHyperlink(UINT nFlags, CPoint point)
{
	// (v.maida 2016-04-22 16:27) - PLID 51696 - Don't continue if we're not on the correct tab.
	if (m_pParent && this != m_pParent->GetActiveSheet()) {
		return;
	}

	CRect rc;
	GetDlgItem(IDC_ACTIVE_DATE_APTCOUNT_LABEL)->GetWindowRect(rc);
	ScreenToClient(&rc);

	if (rc.PtInRect(point)) {
		CSchedCountSetupDlg dlg(this);
		if (IDOK == dlg.DoModal()) {
			UpdateView();			
		}
	}
}
// OneDayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NxSchedulerDlg.h"
#include "OneDayDlg.h"
#include "GlobalUtils.h"
#include "MainFrm.h"
#include "SchedulerRc.h"
#include "ResEntryDlg.h"
#include "globaldatautils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "SchedCountSetupDlg.h"

#include "ReschedulingQueue.h"
#include "WindowUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CPracticeApp theApp;
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// COneDayDlg dialog


COneDayDlg::COneDayDlg(CWnd* pParent /*=NULL*/)
	: CNxSchedulerDlg(COneDayDlg::IDD, pParent)
{
	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Scheduler_Module/schedule_an_appointment.htm";
}


void COneDayDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxSchedulerDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COneDayDlg)
	DDX_Control(pDX, IDC_ACTIVE_DATE_LABEL, m_nxstaticActiveDateLabel);
	DDX_Control(pDX, IDC_ACTIVE_DATE_APTCOUNT_LABEL, m_nxlabelActiveDateAptcountLabel);
	//}}AFX_DATA_MAP
	//TES 10/22/2004 - PLID 14497 - I don't see any point to this line, and it was causing problems at Kao's office.
	//UpdateView();
}


BEGIN_MESSAGE_MAP(COneDayDlg, CNxSchedulerDlg)
	//{{AFX_MSG_MAP(COneDayDlg)
	// (b.cardillo 2016-06-06 16:20) - NX-100773 - Moved event click handling to base class
	ON_WM_MOVE()
	ON_WM_MOVING()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COneDayDlg message handlers

BEGIN_EVENTSINK_MAP(COneDayDlg, CNxSchedulerDlg)
    //{{AFX_EVENTSINK_MAP(COneDayDlg)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_CTRL, 4 /* ReservationRightClick */, CNxSchedulerDlg::OnReservationRightClick, VTS_DISPATCH VTS_I4 VTS_I4)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_EVENT_CTRL, 4 /* ReservationRightClick */, CNxSchedulerDlg::OnReservationRightClick, VTS_DISPATCH VTS_I4 VTS_I4)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_CTRL, 2 /* ReservationAdded */, CNxSchedulerDlg::OnReservationAdded, VTS_DISPATCH)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_EVENT_CTRL, 2 /* ReservationAdded */, CNxSchedulerDlg::OnReservationAdded, VTS_DISPATCH)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_CTRL, 10 /* ReservationEndDrag */, CNxSchedulerDlg::OnReservationEndDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_CTRL, 12 /* ReservationEndResize */, CNxSchedulerDlg::OnReservationEndResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_CTRL, 1 /* ReservationClick */, CNxSchedulerDlg::OnReservationClick, VTS_DISPATCH)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_EVENT_CTRL, 1 /* ReservationClick */, CNxSchedulerDlg::OnReservationClick, VTS_DISPATCH)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_CTRL, -607 /* MouseUp */, CNxSchedulerDlg::OnMouseUpSingleday, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_EVENT_CTRL, -607 /* MouseUp */, CNxSchedulerDlg::OnMouseUpEvent, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_CTRL, 6 /* ReservationMouseDown */, OnReservationMouseDown, VTS_DISPATCH VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_EVENT_CTRL, 6 /* ReservationMouseDown */, OnReservationMouseDown, VTS_DISPATCH VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_CTRL, 9 /* ReservationDrag */, CNxSchedulerDlg::OnReservationDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_EVENT_CTRL, 9 /* ReservationDrag */, CNxSchedulerDlg::OnReservationDrag, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_CTRL, 11 /* ReservationResize */, CNxSchedulerDlg::OnReservationResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_EVENT_CTRL, 11 /* ReservationResize */, CNxSchedulerDlg::OnReservationResize, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(COneDayDlg, IDC_ONE_DAY_CTRL, 14 /* ReservationRecalculateColor */, CNxSchedulerDlg::OnReservationRecalculateColor, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


// (a.walling 2010-06-23 09:41) - PLID 39263 - Update all columns' available location info
void COneDayDlg::UpdateColumnAvailLocationsDisplay()
{
	UpdateActiveDateLabel();
}

// (a.walling 2010-06-23 18:00) - PLID 39263 - Now a virtual function
void COneDayDlg::UpdateActiveDateLabel()
{
	// (c.haag 2010-11-16 13:49) - PLID 39444 - Also include any resource location abbreviations
	SetDlgItemText(IDC_ACTIVE_DATE_LABEL, m_ResourceAvailLocations.GetDayLabel(0, FormatDateTimeForInterface(GetActiveDate(), "%#x")));
	
	// (a.walling 2010-06-23 09:41) - PLID 39263 - Also update the description
	CString strInfo = m_ResourceAvailLocations.GetDayDescription(0);

	CWnd* pWnd = GetDlgItem(IDC_ACTIVE_DATE_LABEL);
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

void COneDayDlg::Print(CDC *pDC, CPrintInfo *pInfo)
{
	CString strHdr, strFooter;
	CFont hdrFont, ftrFont;
	CFont* pOldFont;
	CRect rectHeader, rectFooter;

	// Set up header								Header is 8.4 percent of page height
	rectHeader.SetRect(0, 0, pInfo->m_rectDraw.Width(), (int)((0.084) * (double)pInfo->m_rectDraw.Height()));
	rectFooter.SetRect(0, (int)((0.984) * (double)pInfo->m_rectDraw.Height()), pInfo->m_rectDraw.Width(), pInfo->m_rectDraw.Height());

	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&hdrFont, 180, "Arial", pDC);
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&ftrFont, 90, "Arial", pDC);
	pOldFont = pDC->SelectObject(&hdrFont);
	pDC->SetTextColor( RGB(0, 0, 0) );

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
	COleDateTime dtPrintDate = COleDateTime::GetCurrentTime();
	dtPrintDate.SetDateTime(dtPrintDate.GetYear(), dtPrintDate.GetMonth(), dtPrintDate.GetDay(), 0, 0, 0);
	strHdr.Format("%sAppointments for %s\n%s", strApptType, /*m_ResourceCombo.GetText()*/
		VarString(m_dlResources->GetValue( m_dlResources->CurSel, 2 )), 
		FormatDateTimeForInterface(GetActiveDate(), "%A ") + FormatDateTimeForInterface(GetActiveDate(), NULL, dtoDate));
	pDC->DrawText(strHdr, rectHeader, DT_CENTER);
	pDC->SelectObject(pOldFont);

	//Now we're done with the header, so resize the printinfo rectdraw.
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
	//This font is the same as the singleday uses, EXCEPT BOLD.  I know because I looked at the code.
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

LRESULT COneDayDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return CNxSchedulerDlg::WindowProc(message, wParam, lParam);
}

int COneDayDlg::SetControlPositions()
{
	int nResult = CNxSchedulerDlg::SetControlPositions();
	CRect rect, rcEvent, rcSingleDay;

	// CAH 10/16/01
	GetDlgItem(IDC_BTN_NEW_EVENT)->GetClientRect(rect);
	GetDlgItem(IDC_BTN_NEW_EVENT)->SetWindowPos(NULL, 0,0, m_pSingleDayCtrl.GetTimeButtonWidth(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE);
	
	GetDlgItem(IDC_BTN_NEW_EVENT)->GetWindowRect(rect);
	ScreenToClient(rect);
	CWnd::FromHandle((HWND)m_pEventCtrl.GethWnd())->GetWindowRect(rcEvent);
	ScreenToClient(rcEvent);
	CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd())->GetWindowRect(rcSingleDay);
	ScreenToClient(rcSingleDay);
	CWnd::FromHandle((HWND)m_pEventCtrl.GethWnd())->SetWindowPos(NULL, rect.right, rcEvent.top, rcSingleDay.Width() - m_pSingleDayCtrl.GetTimeButtonWidth(), rcEvent.Height(), SWP_NOZORDER);
	
	// (a.walling 2015-01-12 08:47) - PLID 64558 - Rescheduling Queue - Dock and embed
	if (m_pReschedulingQ && m_pReschedulingQ->GetSafeHwnd()) {
		
		long leftOffset = rcSingleDay.Width() / 2;

		CRect rcClient;
		GetClientRect(&rcClient);
		CRect rcQ = rcSingleDay;		
		rcQ.right = rcQ.left + leftOffset;
		rcQ.top = rcEvent.top;
		rcQ.bottom = rcClient.bottom;

		m_pReschedulingQ->SetWindowPos(nullptr, rcQ.left, rcQ.top, rcQ.Width(), rcQ.Height(), SWP_NOZORDER);

		auto OffsetChild = [&](CWnd* pWnd) {
			CRect rc;
			pWnd->GetWindowRect(&rc);
			ScreenToClient(&rc);
			rc.left += leftOffset;
			rc.right = min(rc.right + leftOffset, rcSingleDay.right);
			pWnd->SetWindowPos(nullptr, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
		};

		OffsetChild(GetDlgItem(IDC_BTN_NEW_EVENT));
		OffsetChild(CWnd::FromHandle((HWND)m_pEventCtrl.GethWnd()));
		OffsetChild(CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd()));
		OffsetChild(GetDlgItem(IDC_ACTIVE_DATE_LABEL));
		OffsetChild(GetDlgItem(IDC_ACTIVE_DATE_APTCOUNT_LABEL));
	}
	
	return nResult;
}

BOOL COneDayDlg::OnInitDialog()
{
	try {
		// (j.luckoski 2012-04-26 10:06) - PLID 11597 - Load properties for showign cancelled variables.
		// (j.luckoski 2012-05-08 17:10) - PLID 50242 - Add all properties to bulk cache
		g_propManager.CachePropertiesInBulk("NxSchedulerDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name IN ("
					"'ShowCancelledAppointment', 'CancelledDateRange', 'SchedCancelledApptColor' "
			"))", _Q(GetCurrentUserName()));

		
		m_nCancelledAppt = GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true);
		// (j.luckoski 2012-06-12 15:58) - PLID 11597 - Did not default to 24
		m_nDateRange = GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true);
		m_nCancelColor = GetRemotePropertyInt("SchedCancelledApptColor", RGB(192,192,192), 0, GetCurrentUserName(), true);

		// Must set m_pSingleDayCtrl before CNxSchedulerDlg::OnInitDialog is called
		// (c.haag 2010-03-26 15:05) - PLID 37332 - Use SetDispatch
		m_pSingleDayCtrl.SetDispatch((LPDISPATCH)GetDlgItem(IDC_ONE_DAY_CTRL)->GetControlUnknown());
		m_pEventCtrl.SetDispatch((LPDISPATCH)GetDlgItem(IDC_ONE_DAY_EVENT_CTRL)->GetControlUnknown());

		// Get a variable so we can work with the resource combo for this view
		m_dlResources = BindNxDataListCtrl(IDC_RESOURCE_LIST, false);

		// (c.haag 2009-12-22 13:44) - PLID 28977 - The Appt count label is now a hyperlink label
		m_nxlabelActiveDateAptcountLabel.SetType(dtsHyperlink);

		GetDlgItem(IDC_ACTIVE_DATE_LABEL)->SetFont(&theApp.m_titleFont);
		GetDlgItem(IDC_ACTIVE_DATE_APTCOUNT_LABEL)->SetFont(&theApp.m_multiresFont);
		return CNxSchedulerDlg::OnInitDialog();
	}
	NxCatchAll(__FUNCTION__);
	return TRUE;
}

void COneDayDlg::Internal_GetWorkingResourceAndDateFromGUI(IN const long nColumnIndex, OUT long &nWorkingResourceID, OUT COleDateTime &dtWorkingDate) const
{
	// Since we only support one column in the singleday controls, we must fail if we're 
	// given a column index other than the 0th column
	ASSERT(nColumnIndex == 0);
	if (nColumnIndex == 0) {

		if(m_dlResources->GetCurSel() == -1) {
			m_dlResources->CurSel = 0;

			if(m_dlResources->GetCurSel() == -1) {
				ThrowNxException(
				"COneDayDlg::Internal_GetWorkingResourceAndDateFromGUI: There are no resources in the current view.");
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
			"COneDayDlg::Internal_GetWorkingResourceAndDateFromGUI: The one-day view only has one "
			"column.  No information can be given for column index %li", nColumnIndex);
	}
}

BOOL COneDayDlg::Internal_IsMultiResourceSheet() const
{
	return FALSE;
}

BOOL COneDayDlg::Internal_IsMultiDateSheet() const
{
	return FALSE;
}

void COneDayDlg::PrePrint()
{
}

void COneDayDlg::PostPrint()
{
}

void COneDayDlg::OnMove(int x, int y)
{
	CNxSchedulerDlg::OnMove(x,y);
}

void COneDayDlg::OnMoving(UINT nSide, LPRECT lpRect)
{
	CNxSchedulerDlg::OnMoving(nSide, lpRect);
}

// (c.haag 2009-12-22 14:02) - PLID 28977 - The user can now click on the count label to
// configure how counts are calculated
void COneDayDlg::OnLButtonDown(UINT nFlags, CPoint point)
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
void COneDayDlg::DoClickHyperlink(UINT nFlags, CPoint point)
{
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

// (c.haag 2009-12-22 14:02) - PLID 28977 - Change the mouse cursor appearance if it's 
// hovering over a hyperlink 
BOOL COneDayDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
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
	}
	NxCatchAll(__FUNCTION__);

	// (b.cardillo 2016-06-04 19:56) - NX-100776 - Discovered while doing this item that this was incorrectly 
	// calling the grand-base class instead of the direct base class.
	return __super::OnSetCursor(pWnd, nHitTest, message);
}

void COneDayDlg::ShowReschedulingQueueEmbedded(bool bShow, long nApptID)
{
	if (bShow) {
		if (m_pReschedulingQ && m_pReschedulingQ->GetSafeHwnd() && m_pReschedulingQ->IsWindowVisible()) {
			// (a.walling 2015-01-22 16:08) - PLID 64662 - Rescheduling Queue - selecting specific appointments
			if (nApptID != -1) {
				m_pReschedulingQ->Refresh(nApptID);
			}
			return;
		}

		if (!m_pReschedulingQ) {
			m_pReschedulingQ = std::make_unique<CReschedulingQueue>(this);
		}
		if (!m_pReschedulingQ->GetSafeHwnd()) {
			m_pReschedulingQ->Create(CReschedulingQueue::IDD, this);
		}
		// (a.walling 2015-01-22 16:08) - PLID 64662 - Rescheduling Queue - selecting specific appointments
		m_pReschedulingQ->Refresh(nApptID);
		if (!m_pReschedulingQ->IsWindowVisible()) {
			m_pReschedulingQ->ShowWindow(SW_SHOWNA);
		}
	}
	else {
		if (!m_pReschedulingQ) {
			return;
		}
		if (m_pReschedulingQ->GetSafeHwnd()) {
			m_pReschedulingQ->DestroyWindow();
		}
		m_pReschedulingQ.reset();
	}

	SetControlPositions();
}



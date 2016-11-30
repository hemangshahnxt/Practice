
// NxSchedulerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NxSchedulerDlg.h"
#include "CommonSchedUtils.h"
#include "ResourceOrderDlg.h"
#include "ReservationReadSet.h"
#include "GlobalSchedUtils.h"
#include "ResEntryDlg.h"
#include "schedulerrc.h"
#include "AuditTrail.h"
#include "appointmentsdlg.h" // For pending/in/out/noshow popup menu options
#include "nxsecurity.h"
#include "ResLinkActionDlg.h"
#include "PPCLink.h"
#include "GlobalDataUtils.h"
#include "ScheduledPatientsDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "MultiSelectDlg.h"
#include "AttendanceUtils.h"
#include "HL7Utils.h"
#include "InvUtils.h"
#include "SchedCountSetupDlg.h"
#include "RemoteDataCache.h"
#include "SharedInsuranceUtils.h"
#include "GlobalFinancialUtils.h"
#include "ApptPopupMenuManager.h"
#include <vector>
#include <set>
#include <foreach.h>
#include "DeviceLaunchUtils.h" // (j.gruber 2013-04-03 15:17) - PLID 56012
#include "TemplateHitSet.h"
#include "ViewSchedulerMixRulesDlg.h"
#include "CommonSchedUtils.h"
#include "FirstAvailableAppt.h"
using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



//DRT 7/16/2008 - PLID 30755 - CHECK_FOR_MESSAGE cannot do anything, remove it.
//#define CHECK_FOR_MESSAGE(nMsg, fAction)			if (ProcessMessages(nMsg)) fAction;

// (j.jones 2010-10-06 15:50) - PLID 40729 - upped the arbitrary maximum for views
// from 32 to 125, roughly limiting to 5 columns of 25 entries each
#define MAX_VIEW_COUNT  125

#define IDM_RV_EDIT_RESOURCES	(WM_APP+1)
#define IDM_RV_CHANGE_VIEWS		(IDM_RV_EDIT_RESOURCES+MAX_VIEW_COUNT)

extern CPracticeApp theApp;

CDWordArray m_aryViewsByMenuPosition;
//TES 7/26/2010 - PLID 39445 - We also remember the default locations for each view.
CDWordArray m_aryViewLocationsByMenuPosition;

// (a.walling 2008-05-13 10:31) - PLID 27591 - Several COleVariant conversions were removed for the CDateTimePicker

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace SINGLEDAYLib;
using namespace NXDATALISTLib;

namespace Nx {
namespace Scheduler {

// (a.walling 2013-05-31 13:31) - PLID 56961 - Simplified version of ResExtendedQ for use by GetSingleApptQuery etc
// note this does not include all fields of ResExtendedQ, just the ones needed by the nxscheduler, week, 
// multiresource, and month views for UpdateViewBySingleAppt
CSqlFragment GetResSimpleSql()
{
	return CSqlFragment(
"SELECT "
	  "AppointmentsT.ID "
	", AppointmentsT.AptTypeID "
	", AppointmentsT.CancelledDate "
	", AppointmentsT.Date "
	", AppointmentsT.EndTime "
	", AppointmentsT.LastLM "
	", AppointmentsT.LocationID "
	", AppointmentsT.ModifiedDate "
	", AppointmentsT.ModifiedLogin "
	", AppointmentsT.Notes "
	", AppointmentsT.PatientID "
	", AppointmentsT.ShowState "
	", AppointmentsT.StartTime "
	", AppointmentsT.Status "
	", AptTypeT.Color "
	", AptTypeT.Name AS AptType "
	", PersonT.First "
	", PersonT.Last "
	", PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatientName "
	", AppointmentPurposeT.PurposeID AS AptPurposeID "
	", AppointmentResourceT.ResourceID "
	", AptPurposeT.Name AS AptPurpose "
	", AptShowStateT.Color AS ShowStateColor "
"FROM AppointmentsT "
"INNER JOIN AptShowStateT "
	"ON AppointmentsT.ShowState = AptShowStateT.ID "
"LEFT JOIN PersonT "
	"ON AppointmentsT.PatientID = PersonT.ID "
"LEFT JOIN AptTypeT "
	"ON AppointmentsT.AptTypeID = AptTypeT.ID "
"LEFT JOIN AppointmentPurposeT "
	"ON AppointmentsT.ID = AppointmentPurposeT.AppointmentID "
"LEFT JOIN AppointmentResourceT "
	"ON AppointmentsT.ID = AppointmentResourceT.AppointmentID "
"LEFT JOIN AptPurposeT "
	"ON AppointmentPurposeT.PurposeID = AptPurposeT.ID"
	);
}

}
}

// (a.walling 2015-01-05 14:28) - PLID 64518 - Drop target support for CNxSchedulerDlg
class CNxSchedulerDropTarget
	: public COleDropTarget
{
public:
	CNxSchedulerDropTarget(CNxSchedulerDlg* pThat)
		: pThat(pThat)
	{}

	CNxSchedulerDlg* pThat = nullptr;

	virtual void Revoke() override
	{
		pThat = nullptr;
		__super::Revoke();
	}

	static bool IsSupported(COleDataObject* pDataObject)
	{
		if (!pDataObject) {
			return false;
		}
		if (pDataObject->IsDataAvailable(Nx::Scheduler::cfRes)) {
			return true;
		}
		return false;
	}
	
	static DROPEFFECT DefaultDropEffect(DWORD dwKeyState)
	{
		return DROPEFFECT_MOVE;
		// basic default handling for posterity; future developers, take heed!
		/*
		if (dwKeyState & MK_CONTROL) {
			if (dwKeyState & MK_SHIFT) {
				return DROPEFFECT_LINK;
			}
			else {
				return DROPEFFECT_COPY;
			}
		}
		else if (dwKeyState & MK_SHIFT) {
			return DROPEFFECT_MOVE;
		} else {
			return DROPEFFECT_MOVE;
		}
		*/
	} 

	bool CanDrop(CWnd* pWnd, COleDataObject* pDataObject, CPoint pt) const
	{		
		// MFC's COleDropTarget::XDropTarget stuff 'helpfully' converts screen into client coords
		// at the expense of meaning something different from the IDropTarget specifications O.o
		if (pWnd) {
			pWnd->ClientToScreen(&pt);
		}

		CRect rcSingleDay;
		CWnd::FromHandle((HWND)pThat->m_pSingleDayCtrl.GethWnd())->GetWindowRect(rcSingleDay);
		if (!rcSingleDay.PtInRect(pt)) {
			return false;
		}
		return true;
	}

	DROPEFFECT CalcDropEffect(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint pt) const
	{
		if (!CanDrop(pWnd, pDataObject, pt)) {
			return DROPEFFECT_NONE;
		}
		return DefaultDropEffect(dwKeyState);
	}
		
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) override
	{
		if (!IsSupported(pDataObject)) {
			return DROPEFFECT_NONE;
		}
		return CalcDropEffect(pWnd, pDataObject, dwKeyState, point);
	}

	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) override
	{
		if (!IsSupported(pDataObject)) {
			return DROPEFFECT_NONE;
		}
		return CalcDropEffect(pWnd, pDataObject, dwKeyState, point);
	}

	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) override
	{
		if (!IsSupported(pDataObject)) {
			return FALSE;
		}
		if (!CanDrop(pWnd, pDataObject, point)) {
			return FALSE;
		}
		
		CWaitCursor omgWaitForMeBbbaka;

		if (!CheckCurrentUserPermissions(bioAppointment, sptCreate|sptWrite)) {
			return FALSE;
		}

		HGLOBAL hData = pDataObject->GetGlobalData(Nx::Scheduler::cfRes);
		if (!hData) {
			ASSERT(FALSE);
			return FALSE;
		}
		
		auto targetInfo = pThat->CalcTargetInfo();
		pThat->UpdateTargetInfo(targetInfo);

		CSharedFile data;
		data.SetHandle(hData, FALSE);

		long apptID = 0;
		data.Read(&apptID, sizeof(apptID));

		data.Close();

		// (a.walling 2015-01-27 13:04) - PLID 64412 - Paste the appointment (drag/drop should behave like cut/paste!)
		// resource id must be -1 otherwise for a cut it assumes moving from one resource to another
		pThat->m_pParent->PasteAppointment(apptID, -1, CLIP_METHOD_CUT, true);
		
		// (a.walling 2015-01-29 10:38) - PLID 64705 - Reset any precision template that might be kept track of
		pThat->m_pParent->m_pResDispatch = (LPDISPATCH)-1;
		pThat->m_pParent->m_pResDispatchPtr.SetDispatch(NULL);
		
		return TRUE;
	}

	/*
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject,
		DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState,
		CPoint point);
	*/
};

/////////////////////////////////////////////////////////////////////////////
// CNxSchedulerDlg dialog

CNxSchedulerDlg::CNxSchedulerDlg(int IDD, CWnd* pParent /*=NULL*/)
	: CNxDialog(IDD, (CNxView *)pParent), m_vciColInfoOfLastMouseDownRes(*(new CSchedViewColumnInfo))
{
	m_pEventCtrl.SetDispatch(NULL); // (c.haag 2010-03-26 15:06) - PLID 37332 - Use SetDispatch
	m_bNeedUpdate = false;
	m_bAllowUpdate = false;
	m_bForceBlocksUpdate = false;
	m_bSettingInterval = false;
	m_bIsUpdatingNow = false;
	m_nDayCnt = 0;
	m_OldDate = 0.0;
	m_pParent = 0;

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Scheduler/schedule_an_appointment.htm";
	m_nPatientListStamp = 0;
	m_nActivePurposeSetId = -1;
	m_nActivePurposeId = -1;
	m_nActiveLocationId = -1;
	m_bIsDateSelComboDroppedDown = FALSE;

	// (c.haag 2007-03-15 17:39) - PLID 24514 - TRUE if the scheduler is on a timer to be
	// updated if the user is rapid-firing through days
	m_bIsWaitingForTimedUpdate = FALSE;
	// (d.thompson 2009-10-21) - PLID 35598
	m_bResLockedByRightClick = false;

	//TES 3/31/2011 - PLID 41519
	m_nPendingHighlightID = -1;
}

void CNxSchedulerDlg::InitSchedViewParent(CSchedulerView *pParent)
{
	m_pParent = pParent;
}

void CNxSchedulerDlg::OnDestroy()
{
	try{
		// (a.walling 2015-01-05 14:23) - PLID 64518 - Drop target support for CNxSchedulerDlg
		if (m_pDropTarget) {
			m_pDropTarget->Revoke();
		}
		m_pDropTarget = nullptr;

		//(e.lally 2007-08-24) PLID 27189 - The On-Destroy method gets called before the destructor and will handle
		//the cleanup of our singleDay controls. Just like the Update-Reservations function, we need to delete all
		//of our new SingleDay TextBoxSets.
		//We are doing it here because it is trying to clear things off the window when those objects don't exist
		//anymore by the time we get to the destructor code, causing exceptions elsewhere in the mainfrm.
		if(m_pSingleDayCtrl){
			m_pSingleDayCtrl.Clear(__FUNCTION__);
		}
		if (m_pEventCtrl){
			m_pEventCtrl.Clear(__FUNCTION__);
		}

		CNxDialog::OnDestroy();
	}NxCatchAll("Error in CNxSchedulerDlg::OnDestroy")	
}

CNxSchedulerDlg::~CNxSchedulerDlg()
{
	// (c.haag 2006-12-12 16:06) - PLID 23845 - Clear the m_aVisibleAppts array
	// and delete its contents
	int i, j;
	for (i=0; i < m_aVisibleTemplates.GetSize(); i++) { 
		VisibleTemplate* pTemplate = m_aVisibleTemplates[i];
		for (j=0; j < pTemplate->aRules.GetSize(); j++) {
			 delete pTemplate->aRules[j];
		}
		delete m_aVisibleTemplates[i];
	}
	for (i=0; i < m_aVisibleAppts.GetSize(); i++) { delete m_aVisibleAppts[i]; }
	m_aVisibleTemplates.RemoveAll();
	m_aVisibleAppts.RemoveAll();
	delete &m_vciColInfoOfLastMouseDownRes;
}

void CNxSchedulerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNxSchedulerDlg)
	DDX_Control(pDX, IDC_DATE_SEL_COMBO, m_DateSelCombo);
	DDX_Control(pDX, IDC_MOVE_DAY_BACK, m_btnBack);
	DDX_Control(pDX, IDC_MOVE_DAY_FORWARD, m_btnForward);
	DDX_Control(pDX, IDC_RESOURCE_ORDER_BTN, m_btnResourceOrder);

	// (j.dinatale 2010-09-09) - PLID 23009 - need to subclass our button to an NxIconButton
	DDX_Control(pDX, IDC_BTN_ROOMMANAGER, m_btnRoomManager);
	DDX_Control(pDX, IDC_BTN_RECALL, m_btnRecall);	// (j.armen 2012-02-29 17:44) - PLID 48488
	DDX_Control(pDX, IDC_VIEW_MIX_RULES, m_btnViewMixRules);
	// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
	DDX_Control(pDX, IDC_AM_PM_BTN, m_nxbtnAmPmButton);
	DDX_Control(pDX, IDC_TODAY_BTN, m_nxbtnTodayButton);
	// (b.cardillo 2016-06-06 16:20) - NX-100773 - Make event button a clickable label
	DDX_Control(pDX, IDC_BTN_NEW_EVENT, m_nxlabelEventBtnLabel);
	//}}AFX_DATA_MAP
}

//	ON_EVENT(CNxSchedulerDlg, IDC_DATE_SEL_COMBO, 2 /* Change */, OnChangeDateSelCombo, VTS_NONE)
//	ON_EVENT(CNxSchedulerDlg, IDC_DATE_SEL_COMBO, 3 /* CloseUp */, OnCloseUpDateSelCombo, VTS_NONE)
//	ON_EVENT(CNxSchedulerDlg, IDC_DATE_SEL_COMBO, 4 /* DropDown */, OnDropDownDateSelCombo, VTS_NONE)

// (a.walling 2008-05-13 10:31) - PLID 27591 - Use notify events for CDateTimePicker
BEGIN_MESSAGE_MAP(CNxSchedulerDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNxSchedulerDlg)
	ON_WM_CREATE()
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE_SEL_COMBO, OnChangeDateSelCombo)
	ON_NOTIFY(DTN_CLOSEUP, IDC_DATE_SEL_COMBO, OnCloseUpDateSelCombo)
	ON_NOTIFY(DTN_DROPDOWN, IDC_DATE_SEL_COMBO, OnDropDownDateSelCombo)
	ON_BN_CLICKED(IDC_MOVE_DAY_BACK, OnMoveDayBack)
	ON_BN_CLICKED(IDC_MOVE_DAY_FORWARD, OnMoveDayForward)
	ON_BN_CLICKED(IDC_RESOURCE_ORDER_BTN, OnResourceOrderBtn)
	ON_BN_CLICKED(IDC_BTN_SHOWPATIENTINFORMATION, OnShowPatientInfoBtn)

	// (j.dinatale 2010-09-08) - PLID 23009 - Event handle for the Room Manager button
	ON_BN_CLICKED(IDC_BTN_ROOMMANAGER, OnRoomManagerBtn)
	ON_BN_CLICKED(IDC_BTN_RECALL, OnRecallBtn)	// (j.armen 2012-02-29 17:44) - PLID 48488
	ON_BN_CLICKED(IDC_VIEW_MIX_RULES, OnViewMixRules)
	ON_BN_CLICKED(IDC_BTN_RESCHEDULING_QUEUE, OnReschedulingQueueBtn)

	ON_WM_SHOWWINDOW()
	ON_WM_DRAWITEM()
	ON_WM_CONTEXTMENU()
	ON_WM_TIMER()
	
	// (j.dinatale 2010-10-14) - PLID 23009 - No longer need this event handle, the ChangeViews or Resources button should really be using the NxButton's tooltip support
	//ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
	

	ON_COMMAND_FORWARD_BACK(ONE_DAY, Days1)
	ON_COMMAND_FORWARD_BACK(DAYS_2, Days2)
	ON_COMMAND_FORWARD_BACK(DAYS_3, Days3)
	ON_COMMAND_FORWARD_BACK(DAYS_4, Days4)
	ON_COMMAND_FORWARD_BACK(DAYS_5, Days5)
	ON_COMMAND_FORWARD_BACK(DAYS_6, Days6)
	ON_COMMAND_FORWARD_BACK(ONE_WEEK, Weeks1)
	ON_COMMAND_FORWARD_BACK(WEEKS_2, Weeks2)
	ON_COMMAND_FORWARD_BACK(WEEKS_3, Weeks3)
	ON_COMMAND_FORWARD_BACK(WEEKS_4, Weeks4)
	ON_COMMAND_FORWARD_BACK(WEEKS_5, Weeks5)
	ON_COMMAND_FORWARD_BACK(WEEKS_6, Weeks6)
	ON_COMMAND_FORWARD_BACK(WEEKS_7, Weeks7)
	ON_COMMAND_FORWARD_BACK(ONE_MONTH, Months1)
	ON_COMMAND_FORWARD_BACK(MONTHS_2, Months2)
	ON_COMMAND_FORWARD_BACK(MONTHS_3, Months3)
	ON_COMMAND_FORWARD_BACK(MONTHS_4, Months4)
	ON_COMMAND_FORWARD_BACK(MONTHS_5, Months5)
	ON_COMMAND_FORWARD_BACK(MONTHS_6, Months6)
	ON_COMMAND_FORWARD_BACK(ONE_YEAR, Years1)
	ON_COMMAND_FORWARD_BACK(YEARS_2, Years2)
	ON_COMMAND_FORWARD_BACK(YEARS_3, Years3)
	ON_COMMAND_FORWARD_BACK(YEARS_4, Years4)
	ON_COMMAND_FORWARD_BACK(YEARS_5, Years5)
	ON_WM_DESTROY()
	// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_WM_SETCURSOR()

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Days1, nsfbuDays, 1)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Days2, nsfbuDays, 2)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Days3, nsfbuDays, 3)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Days4, nsfbuDays, 4)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Days5, nsfbuDays, 5)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Days6, nsfbuDays, 6)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Weeks1, nsfbuWeeks, 1)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Weeks2, nsfbuWeeks, 2)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Weeks3, nsfbuWeeks, 3)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Weeks4, nsfbuWeeks, 4)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Weeks5, nsfbuWeeks, 5)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Weeks6, nsfbuWeeks, 6)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Weeks7, nsfbuWeeks, 7)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Months1, nsfbuMonths, 1)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Months2, nsfbuMonths, 2)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Months3, nsfbuMonths, 3)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Months4, nsfbuMonths, 4)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Months5, nsfbuMonths, 5)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Months6, nsfbuMonths, 6)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Years1, nsfbuYears, 1)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Years2, nsfbuYears, 2)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Years3, nsfbuYears, 3)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Years4, nsfbuYears, 4)
IMPLEMENT_COMMAND_FORWARD_BACK_NXSCHEDULER(Years5, nsfbuYears, 5)

/////////////////////////////////////////////////////////////////////////////
// CNxSchedulerDlg message handlers

BEGIN_EVENTSINK_MAP(CNxSchedulerDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNxSchedulerDlg)
	ON_EVENT(CNxSchedulerDlg, IDC_COMBO_APTTYPE_FILTER, 16 /* SelChosen */, OnSelChosenPurposeSetFilter, VTS_I4)
	ON_EVENT(CNxSchedulerDlg, IDC_COMBO_APTPURPOSE_FILTER, 16 /* SelChosen */, OnSelChosenPurposeFilter, VTS_I4)
	ON_EVENT(CNxSchedulerDlg, IDC_INTERVAL, 16 /* SelChosen */, OnSelChosenIntervalCombo, VTS_I4)
	ON_EVENT(CNxSchedulerDlg, IDC_RESOURCE_LIST, 16 /* SelChosen */, OnSelChosenResources, VTS_I4)
	ON_EVENT(CNxSchedulerDlg, IDC_RESOURCE_LIST, 1 /* SelChanging */, OnSelChangingResources, VTS_PI4)
	ON_EVENT(CNxSchedulerDlg, IDC_INTERVAL, 16 /* SelChosen */, OnSelChosenIntervalCombo, VTS_I4)	
	ON_EVENT(CNxSchedulerDlg, IDC_COMBO_APTTYPE_FILTER, 1 /* SelChanging */, OnSelChangingPurposeSetFilter, VTS_PI4)
	ON_EVENT(CNxSchedulerDlg, IDC_COMBO_APTPURPOSE_FILTER, 1 /* SelChanging */, OnSelChangingPurposeFilter, VTS_PI4)
	ON_EVENT(CNxSchedulerDlg, IDC_COMBO_LOCATION_FILTER, 16 /* SelChosen */, OnSelChosenLocationFilter, VTS_I4)
	ON_EVENT(CNxSchedulerDlg, IDC_COMBO_LOCATION_FILTER, 1 /* SelChanging */, OnSelChangingLocationFilter, VTS_PI4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// (j.dinatale 2010-10-14) - PLID 23009 - No longer need this event handle, the ChangeViews or Resources button should really be using the NxButton's tooltip support
//BOOL CNxSchedulerDlg::OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
//{
//    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
//    UINT nID = pNMHDR->idFrom;
//    if (pTTT->uFlags & TTF_IDISHWND)
//    {
//        // idFrom is actually the HWND of the tool
//        nID = ::GetDlgCtrlID((HWND)nID);
//        if(nID == IDC_RESOURCE_ORDER_BTN)
//        {
//            pTTT->lpszText = "Change Views or Resources...";
//            pTTT->hinst = AfxGetResourceHandle();
//            return(TRUE);
//        }
//    }
//	return FALSE;
//}

const COleDateTime &CNxSchedulerDlg::GetActiveDate()
{
	// (b.cardillo 2003-06-30 15:44) TODO: Since this is implemented as a call to the parent anyway, 
	// we're leaving it as is.  Eventually we need to get rid of this function because whoever calls 
	// it needs to know that there is an OFFICIAL date that must be shared by all tabs, and therefore 
	// no tab is allowed to implement its own "GetActiveDate"
	return m_pParent->GetPivotDate();
}

BOOL CNxSchedulerDlg::SetActiveDate(const COleDateTime &newDate)
{
	if (m_pParent->SetPivotDate(newDate)) {
		m_DateSelCombo.SetValue(newDate);
		EnsureAmPmText();
		return TRUE;
	} else {
		m_DateSelCombo.SetValue(GetActiveDate());
		return FALSE;
	}
}

int CNxSchedulerDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	ASSERT(m_pParent);	// Did you set the CNxSchedulerDlg::m_pParent = this 
								// in the CSchedulerView constructor?

	
	// (a.walling 2015-01-05 14:23) - PLID 64518 - Drop target support for CNxSchedulerDlg
	m_pDropTarget.reset(new CNxSchedulerDropTarget(this), false);
	m_pDropTarget->Register(this);

	if (m_pParent->GetSafeHwnd()) {
		m_ResEntry = m_pParent->GetResEntry();
	}

	//m_Menu.LoadMenu(IDR_RES_POP_UP_MENU);

	return 0;
}

void CNxSchedulerDlg::OnSelChosenResources(long lNewSel) 
{
	// (v.maida 2016-04-22 16:27) - PLID 51696 - Don't continue if we're not on the correct tab.
	if (m_pParent && this != m_pParent->GetActiveSheet()) {
		return;
	}
	ASSERT(lNewSel != sriNoRow);
	if (lNewSel != sriNoRow) {
		long nOldID = m_pParent->GetActiveResourceID();
		long nNewID = AllowChangeView() ? m_dlResources->GetValue(lNewSel, ResourceComboColumns::ID) : nOldID;
		m_pParent->SetActiveResourceID(nNewID, nNewID != nOldID);
	} else {
		m_pParent->SetActiveResourceID(CSchedulerView::sdrUnknownResource, TRUE);
	}
}

bool CNxSchedulerDlg::ReFilter(CReservationReadSet &rsReadSet, bool bForceOpen /* = false */)
{//NOTE:  If you make changes here, WeekDayDlg and MultiResourceDlg both override this function and
//	do their own thing.

	// Build the query
	// (c.haag 2006-12-12 16:32) - PLID 23845 - I wrote an easier-to-maintain implementation
	// that supports more fields so that we can properly suppress template blocks in the face
	// of appointments
	// (z.manning, 05/24/2007) - PLID 26122 - Removed references to EnablePrecisionTemplating preference.
	// (c.haag 2008-11-20 16:33) - PLID 31128 - Added InsuranceReferralSummary
	// (z.manning 2009-06-30 16:43) - PLID 34744 - Only load ins ref summary if necessary
	//TES 1/14/2010 - PLID 36762 - We now need to load an additional recordset for the security groups that patients in this list are 
	// members of; that way CReservationReadSet can hide their demographic information.
	// (j.jones 2011-02-11 09:31) - PLID 35180 - ResExtendedQ is now a function that takes in:
	//@ExcludeCanceledAppointments BIT, @FromDate DATETIME, @ToDate DATETIME, @ResourceIDs NTEXT
	//ResourceIDs must be comma-delimited.

	// (a.walling 2013-06-18 11:31) - PLID 57204 - Use new ResExtendedQ alternative in scheduler views for significantly better scheduler performance
	// (j.jones 2014-12-03 13:54) - PLID 64275 - if the appointment has overridden a mix rule, show it first
	CSqlFragment query(
		"SET NOCOUNT ON; \r\n"
		"DECLARE @dateFrom DATETIME; DECLARE @dateTo DATETIME; \r\n"
		"DECLARE @resourceID INT; \r\n"
		"SET @dateFrom = {OLEDATETIME}; SET @dateTo = DATEADD(d, 1, @dateFrom); \r\n"
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
		"ORDER BY SecurityGroupDetailsT.PatientID, SecurityGroupDetailsT.SecurityGroupID; "
		"SET NOCOUNT OFF; \r\n"
		, AsDateNoTime(GetActiveDate())
		, m_pParent->GetActiveResourceID()
		, Nx::Scheduler::GetResExtendedBase(m_pParent, GetActiveDate(), GetActiveDate())
		, GetExtraFilter()
	);

	CNxPerform nxp("ReFilter " __FUNCTION__);
	return rsReadSet.ReFilter(query);
}

short CNxSchedulerDlg::GetOffsetDay(CReservationReadSet &rsReadSet) 
{
	return (short)((rsReadSet.GetDate() - GetActiveDate()).GetDays());
}

void CNxSchedulerDlg::OnSetDateSelCombo() 
{
	// Get the newly selected date
	COleDateTime dtNew = m_DateSelCombo.GetValue();
	// Change the scheduler's date
	if (AllowChangeView() && m_pParent->SetPivotDate(dtNew)) {
		// The change was successful, so update the screen reflecting the appointments on the new date
		// (c.haag 2006-05-01 12:44) - PLID 20349 - Instead of updating the view immediately, we wait
		// 50 milliseconds in case the user is rapid-fire-clicking to different days
		//UpdateView();
		// (c.haag 2007-03-15 17:28) - PLID 24514 - Set the single day controls to read-only so that nobody
		// can try to open or schedule an appointment on the wrong day while waiting for the timer
		if (NULL != m_pSingleDayCtrl) { 
			m_pSingleDayCtrl.PutReadOnly(VARIANT_TRUE); 
			// (c.haag 2007-03-16 08:17) - PLID 24514 - Also make it so you can't drag or resize any appointments
			const long nDays = m_pSingleDayCtrl.GetDayTotalCount();
			for (short nDay=0; nDay < nDays; nDay++) {
				const long nReservations = m_pSingleDayCtrl.GetReservationCount(nDay);
				for (long i=0; i < nReservations; i++) {
					CReservation pRes = m_pSingleDayCtrl.GetReservation(__FUNCTION__, nDay, i);
					if (NULL != pRes) {
						pRes.PutAllowResize(VARIANT_FALSE);
						pRes.PutAllowDrag(VARIANT_FALSE);
					}
				}
			}
		}
		if (NULL != m_pEventCtrl) {
			m_pEventCtrl.PutReadOnly(VARIANT_TRUE);
			// (c.haag 2007-03-16 08:22) - PLID 24514 - Also make it so you can't drag or resize any appointments
			const long nDays = m_pEventCtrl.GetDayTotalCount();
			for (short nDay=0; nDay < nDays; nDay++) {
				const long nReservations = m_pEventCtrl.GetReservationCount(nDay);
				for (long i=0; i < nReservations; i++) {
					CReservation pRes = m_pEventCtrl.GetReservation(__FUNCTION__, nDay, i);
					if (NULL != pRes) {
						pRes.PutAllowResize(VARIANT_FALSE);
						pRes.PutAllowDrag(VARIANT_FALSE);
					}
				}
			}		
		}
		m_bIsWaitingForTimedUpdate = TRUE;
		SetTimer(IDT_UPDATE_SCHEDULER_VIEW, 200, NULL);		
	} else {
		// Setting the date failed, so set the date combo to show the date that the scheduler is actually looking at
		m_DateSelCombo.SetValue(GetActiveDate());
	}
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CNxSchedulerDlg::OnChangeDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	BOOL bQuickScheduleNavigation = TRUE;
	if (m_pParent && !m_pParent->m_bQuickScheduleNavigation) {
		bQuickScheduleNavigation = FALSE;
	}

	if (m_bIsDateSelComboDroppedDown && !bQuickScheduleNavigation) {
		// The combo is dropped down and we've been asked not to refresh 
		// until the dropdown closes up, so do nothing
	} else {
		// We're good to go, do the refresh now
		OnSetDateSelCombo();
	}

	*pResult = 0;
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CNxSchedulerDlg::OnCloseUpDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Let everyone be able to check and discover the date combo is not dropped down anymore
	m_bIsDateSelComboDroppedDown = FALSE;

	// Get the date the scheduler thinks it's on right now
	COleDateTime dtCur = GetActiveDate();
	// Get the newly selected date
	COleDateTime dtNew = m_DateSelCombo.GetValue();
	
	// See if the dates are different
	if (dtNew.GetYear() != dtCur.GetYear() || dtNew.GetMonth() != dtCur.GetMonth() || dtNew.GetDay() != dtCur.GetDay()) {
		OnSetDateSelCombo();
	}

	*pResult = 0;
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CNxSchedulerDlg::OnDropDownDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Let everyone be able to check and discover the date combo is dropped down now
	m_bIsDateSelComboDroppedDown = TRUE;

	*pResult = 0;
}

void CNxSchedulerDlg::SelChangeIntervalCombo(long nNewSel) {

	if (nNewSel == -1)
	{
		// If the selection is invalid, force it to be valid
		m_dlInterval->CurSel = nNewSel = 0;
	}
	long lCurInterval = VarLong(m_dlInterval->GetValue(nNewSel, 0));

	if (m_pParent && m_pParent->m_nInterval == lCurInterval) 
	{
		if (m_pSingleDayCtrl.GetInterval() != lCurInterval)
			m_pSingleDayCtrl.PutInterval(lCurInterval);
		return;
	}
	try {
		m_pSingleDayCtrl.PutInterval(lCurInterval);
		if (m_pParent) {
			m_pParent->m_nInterval = lCurInterval;
		}
	} catch (CException *e) {
		// If this fails, we must press forward anyway; however, it SHOULDN'T FAIL
		e->Delete();
	}
	if (!m_bSettingInterval) {
		SetRemotePropertyInt("ResInterval", 
			lCurInterval, 0, GetCurrentUserName());
		m_bNeedUpdate = true;
		UpdateView();
	}


}

void CNxSchedulerDlg::OnSelChosenIntervalCombo(long nNewSel)
{
	// (v.maida 2016-04-22 16:27) - PLID 51696 - Don't continue if we're not on the correct tab.
	if (m_pParent && this != m_pParent->GetActiveSheet()) {
		if (nNewSel == -1)
		{
			// If the selection is invalid, force it to be valid
			m_dlInterval->CurSel = nNewSel = 0;
		}
		return;
	}

	if (nNewSel == -1)
	{
		// If the selection is invalid, force it to be valid
		m_dlInterval->CurSel = nNewSel = 0;
	}

	if (CheckCurrentUserPermissions(bioSchedulerInterval, sptWrite) && AllowChangeView()) {
		SelChangeIntervalCombo(nNewSel);
	}
	else {
		m_dlInterval->SetSelByColumn(0, m_pSingleDayCtrl.GetInterval());
	}


	if (m_pSingleDayCtrl) {
		CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd())->SetFocus();
	}
}

void CNxSchedulerDlg::Print(CDC * pDC, CPrintInfo *pInfo)
{
	m_pSingleDayCtrl.Print((IUnknown *)pDC, (IUnknown *)((CPrintInfo *)pInfo));
}

BOOL CNxSchedulerDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try
	{
		// (j.luckoski 2012-05-08 17:10) - PLID 50242 - Add all properties to bulk cache
		// (j.luckoski 2012-05-09 17:12) - PLID 50242 - Deleted duplicates
		// (j.luckoski 2012-05-14 16:41) - PLID 48556 - Add SchedResourceViewTimeRange to bulk cache
		// (a.wilson 2012-06-14 15:41) - PLID 47966 - caching changed preference.
		// (z.manning 2016-04-19 11:52) - NX-100244 - Added LastSelectedAdminUserID
		// (b.cardillo 2016-06-01 14:11) - NX-100771 - Obliterate the old scheduler custom border width preference
		g_propManager.CachePropertiesInBulk("NxSchedulerDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name IN ("
					"'ShowCancelledAppointment', 'CancelledDateRange', 'SchedCancelledApptColor', "
					"'ResInterval', "
					"'SchedulerTemplateCompactText', 'SchedulerUseSimpleForeColors', "
					"'SnapResToGrid', 'ShowAppointmentTooltip', 'ShowAppointmentArrows', "
					"'RepositionAppointmentText', 'SchedHighlightLastApptColor', "
					"'SchedHighlightLastAppt', 'SchedResourceViewTimeRange', "
					"'CalculateAmPmTime', 'DefaultAmTime', 'DefaultAmAnchorTime', "
					"'DefaultPmTime', 'DefaultPmAnchorTime', 'PromptOnModifyLinkedAppts', "
					"'ApptValidateByDuration', 'ShowVoidSuperbillPrompt', 'ApptCheckOpenSlotsForMoveUps', "
					"'SchedShowResLocationAbbrev', 'SchedShowLocationTemplateTimesInDescription', 'ResShowResourceName', "
					"'ColorApptTextWithStatus', 'LastSelectedAdminUserID' "
			"))", _Q(GetCurrentUserName()));

		// (j.luckoski 2012-04-26 10:11) - PLID 11597 - Properties to show cancelled items
		m_nCancelledAppt = GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true);
		// (j.luckoski 2012-06-12 15:58) - PLID 11597 - Did not default to 24
		m_nDateRange = GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true);
		m_nCancelColor = GetRemotePropertyInt("SchedCancelledApptColor", RGB(192,192,192), 0, GetCurrentUserName(), true);
		m_btnBack.AutoSet(NXB_LEFT);
		m_btnForward.AutoSet(NXB_RIGHT);

		// (j.dinatale 2010-09-09) - PLID 23009 - Need to provide a tool tip for the new button and need to set the icon of the button
		m_btnRoomManager.SetToolTip("Room Manager");
		m_btnRoomManager.AutoSet(NXB_ROOMMANAGER);

		// (j.armen 2012-03-28 09:44) - PLID 48480 - Only display the button if we have the license
		if(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) 
		{
			// (j.armen 2012-02-29 17:45) - PLID 48488
			m_btnRecall.SetToolTip("Recalls Needing Attention");
			m_btnRecall.AutoSet(NXB_RECALL);
			m_btnRecall.ShowWindow(SW_SHOWNA);

			//(a.wilson 2012-3-23) PLID 48472 - check whether current patient has permission to access recall system.
			if ((GetCurrentUserPermissions(bioRecallSystem) & (sptRead | sptReadWithPass))) {
				m_btnRecall.EnableWindow(TRUE);
			} else {
				m_btnRecall.EnableWindow(FALSE);
			}
		}
		else
		{
			m_btnRecall.EnableWindow(FALSE);
			m_btnRecall.ShowWindow(SW_HIDE);
		}

		auto& m_btnReschedulingQ = *((CNxIconButton*)SafeGetDlgItem<CNexTechIconButton>(IDC_BTN_RESCHEDULING_QUEUE));
		m_btnReschedulingQ.SetToolTip("Rescheduling Queue");
		m_btnReschedulingQ.SetIcon(IDI_RESCHEDULINGQ, 0, TRUE, FALSE);

		// (j.dinatale 2010-10-14) - PLID 23009 - Also need a tool tip for the Resource Order button
		m_btnResourceOrder.SetToolTip("Change Views or Resources...");

		// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
		m_nxbtnTodayButton.SetType(dtsHyperlink);
		m_nxbtnTodayButton.SetHzAlign(DT_CENTER);
		m_nxbtnTodayButton.SetText("Today");
		m_nxbtnTodayButton.SetSingleLine(true);

		// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
		m_nxbtnAmPmButton.SetType(dtsHyperlink);
		m_nxbtnAmPmButton.SetHzAlign(DT_CENTER);
		m_nxbtnAmPmButton.SetText("12:00 PM");
		m_nxbtnAmPmButton.SetSingleLine(true);

		// (b.cardillo 2016-06-06 16:20) - NX-100773 - Make event button a clickable label
		m_nxlabelEventBtnLabel.SetType(dtsClickableText);
		m_nxlabelEventBtnLabel.SetHzAlign(DT_RIGHT);
		m_nxlabelEventBtnLabel.SetText("Event "); // Space at the end for a tiny margin because when the NxLabel right-aligns, it goes RIGHT up to the edge!
		m_nxlabelEventBtnLabel.SetSingleLine(true);
		m_nxlabelEventBtnLabel.SetFont(&theApp.m_notboldFont);

		//TES 2004-01-30: The interval combo must be initialized before the resource is set, now that resources have default
		//intervals.
		m_dlInterval = BindNxDataListCtrl(IDC_INTERVAL, false);
		// Fill the combo that lists the possible time intervals
		m_bSettingInterval = true;
		m_dlInterval->Clear();
		IRowSettingsPtr pRow = m_dlInterval->Row[-1];
		pRow->Value[0] = (long)5;
		m_dlInterval->InsertRow(pRow, 0);
		pRow = m_dlInterval->Row[-1];
		pRow->Value[0] = (long)10;
		m_dlInterval->InsertRow(pRow, 1);
		pRow = m_dlInterval->Row[-1];
		pRow->Value[0] = (long)15;
		m_dlInterval->InsertRow(pRow, 2);
		pRow = m_dlInterval->Row[-1];
		pRow->Value[0] = (long)20;
		m_dlInterval->InsertRow(pRow, 3);
		pRow = m_dlInterval->Row[-1];
		pRow->Value[0] = (long)30;
		m_dlInterval->InsertRow(pRow, 4);
		// (b.cardillo 2016-05-13 14:22) - NX-100239 - Eliminate the 40-minute interval option
		pRow = m_dlInterval->Row[-1];
		pRow->Value[0] = (long)60;
		m_dlInterval->InsertRow(pRow, 5);

		try {

			// Fill the resource combo and set the appropriate selection, this is entirely based on the CSchedulerView's 
			// official list of resources and single official "active resource" within that list
			Internal_ReflectCurrentResourceListOnGUI();
			Internal_ReflectActiveResourceOnGUI();
			// We'll be calling updateview as appropriate later on

		} NxCatchAllCall("CNxSchedulerDlg::OnInitDialog:init_resource_list", {
			DestroyWindow();
			if (m_pParent) m_pParent->Invalidate();
			return FALSE;
		});
		
		// (a.walling 2008-12-04 16:20) - PLID 32345 - This was being bound twice, leading to an assertion in the datalist
		//m_dlInterval = BindNxDataListCtrl(IDC_INTERVAL, false);

		m_dlTypeFilter = BindNxDataListCtrl(IDC_COMBO_APTTYPE_FILTER);	
		pRow = m_dlTypeFilter->Row[-1];
		pRow->Value[0] = (long)-1;
		pRow->Value[1] = _bstr_t(SCHEDULER_TEXT_FILTER__ALL_TYPES);
		m_dlTypeFilter->AddRow(pRow);	
		m_dlTypeFilter->TrySetSelByColumn(0, (long)-1);

		//DRT 8/5/03 - added filter for multiple types, id -2
		pRow = m_dlTypeFilter->GetRow(-1);
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _bstr_t(SCHEDULER_TEXT_FILTER__MULTI_TYPES));
		m_dlTypeFilter->AddRow(pRow);


		m_dlPurposeFilter = BindNxDataListCtrl(IDC_COMBO_APTPURPOSE_FILTER, false);
		RequeryPurposeFilters();

		//TES 6/21/2010 - PLID 21341 - Added location filter.
		m_dlLocationFilter = BindNxDataListCtrl(IDC_COMBO_LOCATION_FILTER);
		pRow = m_dlLocationFilter->Row[-1];
		pRow->Value[0] = (long)-1;
		pRow->Value[1] = _bstr_t(SCHEDULER_TEXT_FILTER__ALL_LOCATIONS);
		m_dlLocationFilter->AddRow(pRow);
		m_dlLocationFilter->TrySetSelByColumn(0, (long)-1);

		// (d.singleton 2012-06-07 09:56) - PLID 47473 added filter for multiple types, id -2
		pRow = m_dlLocationFilter->GetRow(-1);
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _bstr_t(SCHEDULER_TEXT_FILTER__MULTI_LOCATIONS));
		m_dlLocationFilter->AddRow(pRow);

		// Find out if this user has permission to change the interval.
		// If not, make it read-only.

		// TODO: Uncomment this out when we figure out how to repaint the window
		// though the control is not visible.
		//if (!(GetCurrentUserPermissions(bioSchedulerInterval) & SPT_V_________))
			//GetDlgItem(IDC_INTERVAL)->ShowWindow(SW_HIDE);
		if (!(GetCurrentUserPermissions(bioSchedulerInterval) & (SPT___W________ANDPASS))) {
			GetDlgItem(IDC_INTERVAL)->EnableWindow(FALSE);
		}

		GetControlPositions();

		long nInterval = GetRemotePropertyInt("ResInterval", 15, 0, GetCurrentUserName());
		long nIntervalPos = m_dlInterval->FindByColumn(0, nInterval, 0, TRUE);
		// (b.cardillo 2016-05-13 14:22) - NX-100239 - Eliminate the 40-minute interval option
		// If we were unable to set the dropdown to the preferred interval, probably because whatever they had it 
		// set to in data doesn't exist as an option, then revert to the 15-minute default, which certainly exists.
		if (nIntervalPos == NXDATALISTLib::sriNoRow) {
			nInterval = 15;
			nIntervalPos = m_dlInterval->SetSelByColumn(0, nInterval);
		}

		// Search for the obtained interval
		if (nIntervalPos != -1)
		{
			m_dlInterval->PutCurSel(nIntervalPos);
			SelChangeIntervalCombo(nIntervalPos);
		}

		// (b.cardillo 2016-06-01 14:11) - NX-100771 - Obliterate the old scheduler custom border width preference

		// (b.cardillo 2005-03-07 16:26) - PLID 15712 - Check the preference for 
		// whether to show "compact block text" or not
		// (b.cardillo 2005-04-04 10:06) - PLID 16141 - Made it default to true, and also made it
		// not auto-create the property.  I have it return 2 by default now, so that we can 
		// distinguish among the three possibilities: "set to true" (1), "set to false" (0), and 
		// "not set yet" (2) if we need to.
		long nUseCompactText = GetRemotePropertyInt("SchedulerTemplateCompactText", 2, 0, GetCurrentUserName(), false);
		if (nUseCompactText != 0) {
			m_pSingleDayCtrl.PutCompactBlockText(VARIANT_TRUE);
			m_pEventCtrl.PutCompactBlockText(VARIANT_TRUE);
		} else {
			m_pSingleDayCtrl.PutCompactBlockText(VARIANT_FALSE);
			m_pEventCtrl.PutCompactBlockText(VARIANT_FALSE);
		}

		// (b.cardillo 2005-07-22 13:25) - PLID 17088 - Check the preference for whether to use 
		// simple foreground color selection or to use the more advanced algorithm.
		{
			long nUseSimpleForeColors = GetRemotePropertyInt("SchedulerUseSimpleForeColors", -1, 0, GetCurrentUserName(), false);
			if (nUseSimpleForeColors != -1) {
				m_pSingleDayCtrl.PutUseSimpleForeColors(nUseSimpleForeColors ? VARIANT_TRUE : VARIANT_FALSE);
				m_pEventCtrl.PutUseSimpleForeColors(nUseSimpleForeColors ? VARIANT_TRUE : VARIANT_FALSE);
			}
		}

		m_bSettingInterval = false;
		EnableToolTips(TRUE);


	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNxSchedulerDlg::MoveCurrentDate(COleDateTimeSpan span)
{
	if(!AllowChangeView()) return;
	BeginWaitCursor();
	COleDateTime newDate;
//	GetActiveDate(newDate); BVB
	newDate = m_DateSelCombo.GetValue();
	newDate += span;
//	m_DateSelCombo.SetValue(COleVariant(newDate));
	SetActiveDate(newDate);
	OnSetDateSelCombo();
	EndWaitCursor();
}

void AddMonthsToDate(IN OUT COleDateTime &dt, int nMonths)
{
	// Get the date's current year, month and day
	int nYear = dt.GetYear();
	int nMonth = dt.GetMonth();
	int nDay = dt.GetDay();

	// Calculate the new month and year
	{
		nMonth += nMonths;
		nYear += nMonth / 12;
		nMonth = nMonth % 12;

		if (nMonth < 1) {
			nMonth += 12;
			nYear--;
		}
	}

	// Make sure there are enough days in the new month; if not, get what our new day will be
	long nDaysInNewMonth = GetDaysInMonth(nMonth, nYear);
	if (nDaysInNewMonth < nDay) {
		nDay = nDaysInNewMonth;
	}

	// Set the given date variable to contain the new date
	dt.SetDateTime(nYear, nMonth, nDay, dt.GetHour(), dt.GetMinute(), dt.GetSecond());
}

void CNxSchedulerDlg::MoveCurrentDateByMonths(int nMonths)
{
	BeginWaitCursor();
	
	{
		// Get the current date
		COleDateTime dtOldDate = m_DateSelCombo.GetValue();
		COleDateTime dtNewDate = dtOldDate;
		// Add the given number of months
		AddMonthsToDate(dtNewDate, nMonths);
		// Set the new active date
		MoveCurrentDate(dtNewDate-dtOldDate);
	}
	
	EndWaitCursor();
}

void AddYearsToDate(IN OUT COleDateTime &dt, int nYears)
{
	// Get the date's current year and day
	int nYear = dt.GetYear();
	int nMonth = dt.GetMonth();
	int nDay = dt.GetDay();

	// Calculate the new year
	nYear += nYears;

	// Make sure there are enough days in the current month of the new year; if not, get what our new day will be
	long nDaysInNewMonth = GetDaysInMonth(nMonth, nYear);
	if (nDaysInNewMonth < nDay) {
		nDay = nDaysInNewMonth;
	}

	// Set the given date variable to contain the new date
	dt.SetDateTime(nYear, nMonth, nDay, dt.GetHour(), dt.GetMinute(), dt.GetSecond());
}

void CNxSchedulerDlg::MoveCurrentDateByYears(int nYears)
{
	BeginWaitCursor();
	
	{
		// Get the current date
		COleDateTime dtOldDate = m_DateSelCombo.GetValue();
		COleDateTime dtNewDate = dtOldDate;
		// Add the given number of years
		AddYearsToDate(dtNewDate, nYears);
		// Set the new active date
		MoveCurrentDate(dtNewDate-dtOldDate);
	}
	
	EndWaitCursor();
}

void CNxSchedulerDlg::OnMoveDayBack() 
{
	MoveCurrentDate(COleDateTimeSpan(-1, 0, 0, 0));
}

void CNxSchedulerDlg::OnMoveDayForward() 
{
	MoveCurrentDate(COleDateTimeSpan(1, 0, 0, 0));
}

void CNxSchedulerDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// (j.luckoski 2012-05-02 17:09) - PLID 11597 - Load properties for cancelled appts.
	m_nCancelledAppt = GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true);
	// (j.luckoski 2012-06-12 15:58) - PLID 11597 - Did not default to 24
	m_nDateRange = GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true);
	m_nCancelColor = GetRemotePropertyInt("SchedCancelledApptColor", RGB(192,192,192), 0, GetCurrentUserName(), true);
	// (b.cardillo 2003-06-30 17:47) - We used to detect the NetUtils::Resources table-checker message here and if 
	// was changed we'd reload the current resource view.  That was just insane because this function is often called 
	// as a result of some non-UI event.  In other words the resource order could change in the middle of the user 
	// working on an appointment or doing something else in the scheduler.  That is just confusing.  So now it is 
	// only reloaded if the user takes some action on this computer (like opening the resource order dialog).

	if (!(GetCurrentUserPermissions(bioSchedulerInterval) & (SPT___W________ANDPASS))) {
		GetDlgItem(IDC_INTERVAL)->EnableWindow(FALSE);
	}
	else
		GetDlgItem(IDC_INTERVAL)->EnableWindow(TRUE);

	{
		// Make sure the entry screen is not showing
		// (d.thompson 2009-10-22) - PLID 35598 - I don't think this can happen, but just in case... if they try
		//	to UpdateView while the right click handler is active, we have to refuse.
		// (c.haag 2010-09-07 11:36) - PLID 40108 - Consolidated all commented actions into one function
		if (!IsSafeToUpdateView()) {
			return;
		}

		//(j.jones 2003-07-03 12:10) there are a few valid ways to get a view with no resources, such as deleting
		//resources from a view or removing all permissions from resources in a view. In these cases,
		//we must allow access to the scheduler, but you won't be able to do anything except edit resource views.
		if(m_pParent->GetCurrentResourceList().GetSize() == 0) {
			if(m_dlResources)
				m_dlResources->PutEnabled(VARIANT_FALSE);
			SetDlgItemText(IDC_ACTIVE_DATE_LABEL,"");
			SetDlgItemText(IDC_ACTIVE_DATE_APTCOUNT_LABEL, "");
			return;
		}
		else {
			if(m_dlResources)
				m_dlResources->PutEnabled(VARIANT_TRUE);
		}
		
		try {		

			// If we're good to go, then refresh the screen
			CWaitCursor wc;
			
			m_pSingleDayCtrl.PutSnapReservationsToGrid(GetRemotePropertyInt("SnapResToGrid", 1, 0, "<None>", true) == 0 ? false : true);
			m_pSingleDayCtrl.PutShowTooltips(GetRemotePropertyInt("ShowAppointmentTooltip", 1, 0, "<None>", true) == 0 ? false : true);
			m_pSingleDayCtrl.PutShowArrows(GetRemotePropertyInt("ShowAppointmentArrows", 1, 0, "<None>", true) == 0 ? false : true);
			m_pSingleDayCtrl.PutRepositionText(GetRemotePropertyInt("RepositionAppointmentText", 1, 0, GetCurrentUserName(), true) == 0 ? false : true);
			//TES 3/8/2011 - PLID 41519 - Load the color for "highlighted" appointments.  The default is a light blue (not similar to any of the default status colors)
			m_pSingleDayCtrl.PutHighlightColor(GetRemotePropertyInt("SchedHighlightLastApptColor", RGB(128,255,255), 0, GetCurrentUserName(), true));

			// (b.cardillo 2016-06-01 14:11) - NX-100771 - Obliterate the old scheduler custom border width preference

			// (b.cardillo 2005-03-07 16:26) - PLID 15712 - Check the preference for whether to show 
			// "compact block text" or not
			// (b.cardillo 2005-04-04 10:06) - PLID 16141 - Made it default to true, and also made it
			// not auto-create the property.  I have it return 2 by default now, so that we can 
			// distinguish among the three possibilities: "set to true" (1), "set to false" (0), and 
			// "not set yet" (2) if we need to.
			long nUseCompactText = GetRemotePropertyInt("SchedulerTemplateCompactText", 2, 0, GetCurrentUserName(), false);
			if (nUseCompactText != 0) {
				m_pSingleDayCtrl.PutCompactBlockText(VARIANT_TRUE);
				m_pEventCtrl.PutCompactBlockText(VARIANT_TRUE);
			} else {
				m_pSingleDayCtrl.PutCompactBlockText(VARIANT_FALSE);
				m_pEventCtrl.PutCompactBlockText(VARIANT_FALSE);
			}
			// (b.cardillo 2005-07-22 13:25) - PLID 17088 - Check the preference for whether to use 
			// simple foreground color selection or to use the more advanced algorithm.
			{
				long nUseSimpleForeColors = GetRemotePropertyInt("SchedulerUseSimpleForeColors", -1, 0, GetCurrentUserName(), false);
				if (nUseSimpleForeColors != -1) {
					m_pSingleDayCtrl.PutUseSimpleForeColors(nUseSimpleForeColors ? VARIANT_TRUE : VARIANT_FALSE);
					m_pEventCtrl.PutUseSimpleForeColors(nUseSimpleForeColors ? VARIANT_TRUE : VARIANT_FALSE);
				}
			}
			// (c.haag 2003-07-30 12:34) - Do this here because PutEnabled and UpdateReservations effects
			// reservation colors, and UpdateReservations takes precedence.
			// Now we've successfully updated the controls so we just have to enable them now
			//TES 12/18/2008 - PLID 32497 - Moved to separate function.
			EnableSingleDayControls();

			// (z.manning, 10/11/2006) - PLID 5812 - Make sure this is called before UpdateReservations()
			// because if a reservation is outside the visible range, the single day control will update
			// itself to make sure all appointments are included.
			SetScheduleVisibleTimeRange();

			bool bTempNeed = m_bNeedUpdate;
			UpdateReservations(false, false);
			UpdateBlocks(true, bTempNeed);	

			EnsureAmPmText();

//			// Now we've successfully updated the controls so we just have to enable them now
//			m_pEventCtrl.PutEnabled(VARIANT_TRUE);
//			m_pSingleDayCtrl.PutEnabled(VARIANT_TRUE);
			
		} NxCatchAllCall("CNxSchedulerDlg::UpdateView", ClearAndDisable());
	}
}

bool CNxSchedulerDlg::NeedUpdate()
{
/*	bool bAns = m_bNeedUpdate;
	if (!bAns) {
		unsigned long nPatientStamp = GetTimeStamp(PATIENT_LIST_TIME_STAMP_NAME);
		if ((nPatientStamp != STAMP_FAILURE) && (nPatientStamp != m_nPatientListStamp)) {
			m_nPatientListStamp = nPatientStamp;
			bAns = true;
		}
	}
	return bAns;*/

	return m_bNeedUpdate;
}

long CNxSchedulerDlg::GetActiveInterval()
{
	return VarLong(m_dlInterval->GetValue(m_dlInterval->GetCurSel(), 0));
}

bool CNxSchedulerDlg::SetActiveInterval(long nInterval)
{
	// (v.maida 2016-04-22 16:27) - PLID 51696 - Don't continue if we're not on the correct tab.
	if (m_pParent && this != m_pParent->GetActiveSheet()) {
		return false;
	}
	long listCount = m_dlInterval->GetRowCount();
	if (GetActiveInterval() == nInterval)
	{
		m_pSingleDayCtrl.PutInterval(nInterval);		
		return true;
	}
	for (long i=0; i < listCount; i++)
	{
		if (VarLong(m_dlInterval->GetValue(i, 0)) == nInterval)
		{
			m_dlInterval->PutCurSel(i);
			SelChangeIntervalCombo(i);
			return true;
		}
	}
	return false;
}

void CNxSchedulerDlg::EnsureButtonLabels()
{

}

// (c.haag 2010-07-13 12:03) - PLID 39615 - We now take in a dispatch, not a CReservation
void CNxSchedulerDlg::DoPopupContextMenu(IN const CPoint &ptPopUpAt, IN LPDISPATCH theRes, OPTIONAL IN const CSchedTemplateInfo *psti)
{
	bool bIsResCancelled = false;
	long nPatientID = -25;
	// (c.haag 2006-12-05 10:46) - PLID 23666 - We have different pop-up options for template block reservations
	BOOL bUseTemplatePopup;
	{
		CReservation pRes(__FUNCTION__, theRes); // (c.haag 2010-07-13 12:03) - PLID 39615
		if (theRes != NULL) {
				// (j.luckoski 2012-05-09 16:07) - PLID 50264 - User ReturnsRecordsParam
				if(ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT * FROM AppointmentsT WHERE Status = 4 AND ID = {INT}", pRes.GetReservationID())) {
					bIsResCancelled = true;
				}
			
		}
		bUseTemplatePopup = (NULL != pRes && pRes.GetTemplateItemID() != -1) ? TRUE : FALSE;
	}

	if (theRes != NULL) {
		m_pParent->m_pResDispatch = theRes;
		// (a.walling 2007-08-31 12:08) - PLID 27265 - If we are storing a reference to a COM pointer, we should add a ref.
		// we can do this easily by just storing a smart pointer.
		m_pParent->m_pResDispatchPtr = CReservation("CSchedulerView::m_pResDispatchPtr (CNxSchedulerDlg::DoPopupContextMenu)", theRes);

		//TES 1/14/2010 - PLID 36762 - If this isn't for a template, then it's a patient appointment.  If that patient is blocked to us,
		// then we don't want to pop up a menu, because they're not allowed to do anything to this apopintment.
		if(!bUseTemplatePopup && !GetMainFrame()->CanAccessPatient(m_pParent->m_pResDispatchPtr.GetPersonID(), true)) {
			// (c.haag 2010-08-27 13:17) - PLID 39655 - Reset the parent res dispatch objects
			m_pParent->m_pResDispatch = (LPDISPATCH)-1;
			m_pParent->m_pResDispatchPtr.SetDispatch(NULL);
			return;
		}

		//TES 3/8/2011 - PLID 41519 - If this isn't for a template or an event, then this is now the "last edited" appointment, so if
		// we're highlighting appointments, then do so.
		{
			CReservation pRes(__FUNCTION__, theRes);
			if(!bUseTemplatePopup && CWnd::FromHandle((HWND)pRes.GethWnd())->GetOwner()->m_hWnd == (HWND)m_pSingleDayCtrl.GethWnd()) {
				// (d.thompson 2012-08-01) - PLID 51898 - Changed default to Off
				if(GetRemotePropertyInt("SchedHighlightLastAppt", 0, 0, GetCurrentUserName(), true)) {
					CSingleDay pSingleDay;
					CWnd* pWndNxTextbox = CWnd::FromHandle((HWND)pRes.GethWnd());
					pSingleDay.SetDispatch((LPDISPATCH)pWndNxTextbox->GetParent()->GetControlUnknown()); // (c.haag 2010-03-26 15:11) - PLID 37332 - Use SetDispatch
					pSingleDay.SetHighlightRes(theRes);
					//TES 3/31/2011 - PLID 41519 - Clear out our "pending" highlight
					m_nPendingHighlightID = -1;
				}
			}
		}
	} else {
		m_pParent->m_pResDispatch = (LPDISPATCH)-1;
		// (a.walling 2007-08-31 12:15) - PLID 27265 - Ensure whatever reference may have previously existed is released.
		m_pParent->m_pResDispatchPtr.SetDispatch(NULL);
	}

	//TES 8/7/2006 - The paste code may need this information even if we are right-clicking on an appointment.
	long nWorkingResourceID;
	COleDateTime dtWorkingDate;
	long nClickedDay = GetMsDay();
	m_pParent->GetWorkingResourceAndDate(this, nClickedDay, nWorkingResourceID, dtWorkingDate);
	m_pParent->m_clickTargetInfo.resourceID = nWorkingResourceID;
	m_pParent->m_clickTargetInfo.date = dtWorkingDate;
	m_pParent->m_clickTargetInfo.day = nClickedDay;

	m_Menu.DestroyMenu();
	// (c.haag 2006-12-05 10:46) - PLID 23666 - We have different pop-up options for template block reservations
	m_Menu.CreatePopupMenu( bUseTemplatePopup ? CNxSchedulerMenu::eTemplateBlock : CNxSchedulerMenu::eReservation );

	CNxSchedulerMenu *pPopup = &m_Menu;
	
	ASSERT(pPopup != NULL);

	// (c.haag 2011-06-17) - PLID 36477 - We now handle some of the menu options in here (code refactoring)
	long nApptID = -1;
	{
		CReservation& pRes = m_pParent->m_pResDispatchPtr;
		if (NULL != m_pParent->m_pResDispatchPtr) {
			nApptID = pRes.GetReservationID();
			nPatientID = pRes.GetPersonID();
		}
	}
	// (j.jones 2010-09-24 11:45) - PLID 34518 - added ability to create a copayment
	CArray<long, long> aryCopayInsuredPartyIDs;
	long nPrimaryInsuredPartyID = -1;
	const int MIN_ID_COPAYMENT_RESPS = 20000;
	CApptPopupMenuManager apmm(m_Menu, nPrimaryInsuredPartyID, aryCopayInsuredPartyIDs, m_pParent, nApptID, nPatientID);

	// (z.manning, 05/03/2007) - PLID 25896 - Paste is now an option for the precision template menu, so took
	// this code block out of the if(!bUseTemplatePopup) block.
	int nMethod;
	// Show or hide the 'paste' menu item as necessary
	CClip *pClip;
	GetMainFrame()->GetClip(CLIP_RESERVATION, pClip, nMethod);
	if(pClip && pClip->IsKindOf(RUNTIME_CLASS(CClipRes))) {
		if (((CClipRes*)pClip)->m_nResID != -1) {
			pPopup->EnableMenuItem(ID_RES_PASTE, 	MF_ENABLED|MF_BYCOMMAND);
		} else {
			pPopup->EnableMenuItem(ID_RES_PASTE, 	MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
		}
	}
	else {
		pPopup->EnableMenuItem(ID_RES_PASTE, 	MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
	}

	//DRT 6/12/2008 - PLID 9679 - array of superbill templates to hold the available templates to merge.
	const int MIN_ID_SUPERBILL_TEMPLATES = 10000;
	bool bPromptIfSuperbillEmpty = false;
	long nAptTypeCategory = -1;

	// (c.haag 2006-12-05 10:46) - PLID 23666 - Don't do any of this if we are using template block reservations
	if (!bUseTemplatePopup) {
		// See if we're on an appointment
		if (theRes != NULL) {
			CReservation& pRes = m_pParent->m_pResDispatchPtr; // (c.haag 2010-07-13 12:03) - PLID 39615

			// (j.luckoski 2012-05-07 11:12) - PLID 11597 - If not cancelled activate these context menu items
			if(!bIsResCancelled) {
			// Enable the usable menu items ('edit' and 'delete')
			pPopup->EnableMenuItem(ID_RES_EDIT, 	MF_ENABLED|MF_BYCOMMAND);
			if (GetCurrentUserPermissions(bioAppointment) & (SPT_______1____ANDPASS))
				pPopup->EnableMenuItem(ID_RES_CUT, 	MF_ENABLED|MF_BYCOMMAND);
			pPopup->EnableMenuItem(ID_RES_COPY, 	MF_ENABLED|MF_BYCOMMAND);
			if (GetCurrentUserPermissions(bioAppointment) & (SPT________2___ANDPASS))
				pPopup->EnableMenuItem(ID_RES_DELETE, 	MF_ENABLED|MF_BYCOMMAND); 
			// Add extended options for appointments
			pPopup->AppendMenu(MF_SEPARATOR);
			} else { // (j.luckoski 2012-05-07 11:13) - PLID 11597 - if cancelled deactivate these.
				pPopup->EnableMenuItem(ID_RES_DELETE, 	MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
				pPopup->EnableMenuItem(ID_RES_CUT, 	MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
				pPopup->EnableMenuItem(ID_RES_COPY, 	MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
				pPopup->EnableMenuItem(ID_RESTORE_APPT, MF_ENABLED|MF_BYCOMMAND);
			}

			/*pPopup->AppendMenu(MF_ENABLED, ID_APPT_PENDING, "Mark as Pe&nding");
			pPopup->AppendMenu(MF_ENABLED, ID_APPT_IN, "Mark as &In");
			pPopup->AppendMenu(MF_ENABLED, ID_APPT_OUT, "Mark as &Out");
			pPopup->AppendMenu(MF_ENABLED, ID_APPT_NOSHOW, "Mark as &No Show");
			pPopup->AppendMenu(MF_ENABLED, ID_APPT_RECEIVED, "Mark as &Received");*/
	/*		for (long i=0; i < ((CSchedulerView*)GetParent())->m_adwShowState.GetSize(); i++)
			{
				CString str;
				str.Format("Mark as %s", ((CSchedulerView*)GetParent())->m_astrShowState[i]);
				pPopup->AppendMenu(MF_ENABLED, ID_APPT_PENDING + ((CSchedulerView*)GetParent())->m_adwShowState[i], str);
			}
	*/
			// MSC - 4/6/2004 - Check to see the state of the appt, if it's marked as no show, then just give them
			// the option to mark it as "show", similar to the way the appt tab works.
			long nResID = pRes.GetReservationID();

			// (d.moore 2007-05-22 10:42) - PLID 4013 - Changed the MoveUp bit to query the waiting list table.
			// (j.jones 2009-08-28 13:07) - PLID 35381 - added AptTypeCategory
			_RecordsetPtr rs = CreateParamRecordset(
				"SELECT PatientID, ShowState, Confirmed, AptTypeT.Category AS AptTypeCategory, "
				"CONVERT(bit, CASE WHEN EXISTS (SELECT ID FROM WaitingListT WHERE AppointmentID = {INT}) THEN 1 ELSE 0 END) AS MoveUp "
				"FROM AppointmentsT "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"WHERE AppointmentsT.ID = {INT}", nResID, nResID);

			if(!rs->eof) {
				_variant_t varShow = rs->Fields->GetItem("ShowState")->Value;
				// (c.haag 2006-05-10 08:29) - PLID 20505 - We get the patient ID here so that we don't have
				// to call IsPatientAppointment and open a second recordset
				nPatientID = VarLong(rs->Fields->GetItem("PatientID")->Value, -25);

				nAptTypeCategory = AdoFldByte(rs, "AptTypeCategory", -1);
				
				// (j.luckoski 2012-05-07 11:14) - PLID 11597 - if cancelled don't show 'no show' context menu items
				if(!bIsResCancelled) {
					if(VarLong(varShow) == 3) {
						pPopup->AppendMenu(MF_ENABLED, ID_APPT_SHOW, GetStringOfResource(IDS_UNMARK_NO_SHOW));
					}
					else {
						pPopup->AppendMenu(MF_ENABLED, ID_APPT_NOSHOWSTATUS, GetStringOfResource(IDS_MARK_NO_SHOW));

						for (long i=0; i < ((CSchedulerView*)GetParent())->m_adwShowState.GetSize(); i++)
						{
							if (((CSchedulerView*)GetParent())->m_adwShowState[i] == 3)	// Exclude NoShow from the populated list
								continue;
							CString str;
							str.Format("Mark as %s", ((CSchedulerView*)GetParent())->m_astrShowState[i]);
							// if the appt is already set to this status, then don't show this option
							if (VarLong(varShow) != (long)((CSchedulerView*)GetParent())->m_adwShowState[i])
								pPopup->AppendMenu(MF_ENABLED, ID_APPT_PENDING + ((CSchedulerView*)GetParent())->m_adwShowState[i], str);
						}
					}
				}
		
			
				long nConfirmed = 0;
				BOOL bMoveUp = FALSE;

				nConfirmed = AdoFldLong(rs, "Confirmed",0);
				bMoveUp = AdoFldBool(rs, "MoveUp",FALSE);
				
				// (j.luckoski 2012-05-07 11:19) - PLID 11597 - If not cancelled show the move up status context item
				if(!bIsResCancelled) {
				pPopup->AppendMenu(MF_SEPARATOR);
				// (c.haag 2011-06-17) - PLID 36477 - This is now handled in a utility function
				apmm.FillConfirmedOptions(nConfirmed);

				if(bMoveUp)
					pPopup->AppendMenu(MF_ENABLED, ID_APPT_REMOVE_MOVE_UP, "Remove &Move-Up Status");
				else
					pPopup->AppendMenu(MF_ENABLED, ID_APPT_MOVE_UP, "Mark as &Move-Up");
				}
			}
			rs->Close();

		} else {
			// Disable the unused menu items ('edit' and 'delete')
			pPopup->EnableMenuItem(ID_RES_EDIT, 	MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
			if (GetCurrentUserPermissions(bioAppointment) & (SPT_______1____ANDPASS))
				pPopup->EnableMenuItem(ID_RES_CUT, 		MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
			pPopup->EnableMenuItem(ID_RES_COPY, 	MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
			if (GetCurrentUserPermissions(bioAppointment) & (SPT________2___ANDPASS))
				pPopup->EnableMenuItem(ID_RES_DELETE, 	MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
		}

		// If it's on an appointment and the appointment is a patient appointment, enable 
		// the patient menu items, otherwise disable them
		if (theRes != NULL && nPatientID > 0) {
			pPopup->EnableMenuItem(ID_GOTOPATIENT, MF_ENABLED|MF_BYCOMMAND);

			//DRT 6/12/2008 - PLID 9679 - We now have some configuration options for the superbill
			//	template that will be used.  We need to modify the menu system accordingly.
			{
				//I cannot find a way to query the list for the position given an ID.  We need to insert
				//	the Print Superbill after the above go to patient, but since most of the list is
				//	created ahead of time... I can't do that easily.  So we'll just search for that 
				//	last position, and place ourselves 1 later.  If not found and nNextPos remains -1, 
				//	it gets appended.
				//(e.lally 2010-09-08) PLID 37244 - To make the print superbill option harder to click accidentally when trying to use the go to pat option,
				//I am moving this down to the end and creating a separator for it to stand alone.

				/*
				int nNextPos = -1;
				for(UINT i = 0; i < pPopup->GetMenuItemCount() && nNextPos == -1; i++) {
					if(pPopup->GetMenuItemID(i) == ID_GOTOPATIENT)
						nNextPos = i + 1;
				}
				*/

				// (c.haag 2011-06-17) - PLID 44286 - Populating superbill menu items is done in a utility function now
				if(!bIsResCancelled) {
				apmm.FillSuperbillOptions();
				}
			}

			// (j.luckoski 2012-05-07 11:21) - PLID 11597 - Hide these options if cancelled
			if(!bIsResCancelled) {
				// (j.jones 2007-11-21 14:45) - PLID 28147 - add the ability to create an allocation,
				// provided the have the license and potential permission to do so
				// (c.haag 2011-06-23) - PLID 44287 - Populating inventory options is done in a utility function now
				BOOL bAddedSeparator = apmm.FillInventoryOptions();

				// (c.haag 2011-06-24) - PLID 44317 - Populating billing options is done in a utility function now. Pass
				// in FALSE so that a separator is wedged between these and any previous items.
				bAddedSeparator = apmm.FillBillingOptions(FALSE);

				// (c.haag 2011-06-24) - PLID 44319 - Populating e-eligibility options is done in a utility function.
				bAddedSeparator = apmm.FillEEligibilityOptions(bAddedSeparator);

				// (c.haag 2011-06-24) - PLID 44319 - Populating case history options is done in a utility function.
				bAddedSeparator = apmm.FillCaseHistoryOptions(bAddedSeparator, nAptTypeCategory);
			}
		} else {
			pPopup->EnableMenuItem(ID_GOTOPATIENT, MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
			pPopup->EnableMenuItem(ID_PRINTSUPERBILL, MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
		}
	}

	// Only attempt the template menu items if we weren't given an appointment (i.e. pRes == NULL)
	// (c.haag 2006-12-05 10:48) - PLID 23666 - We should allow users to edit templates when right-clicking
	// on template block reservations
	if (theRes == NULL || bUseTemplatePopup) {
		if (psti) {
			static struct _popup_data {
				CString strName;
				COLORREF clr;
			} popup_data;

			popup_data.strName = psti->m_strName;
			popup_data.clr = psti->m_clrColor;

			// Add the template menu node
			pPopup->InsertMenu(0, MF_ENABLED | MF_BYPOSITION | MF_OWNERDRAW, 0, (char*)&popup_data);
			pPopup->InsertMenu(1, MF_BYPOSITION|MF_SEPARATOR);
		}

		pPopup->InsertMenu(99, MF_BYPOSITION|MF_SEPARATOR);
		//(e.lally 2010-07-15) PLID 39626 - Renamed menu option to Edit Scheduler Templates
		pPopup->InsertMenu(100, MF_ENABLED | MF_BYPOSITION, ID_RES_EDIT_TEMPLATES, "Edit Scheduler Templates...");
		// (z.manning 2014-12-01 17:31) - PLID 64205 - Added option to template collection setup
		pPopup->InsertMenu(101, MF_ENABLED | MF_BYPOSITION, ID_RES_EDIT_TEMPLATE_COLLECTIONS, "Manage Template Collections...");
		//(e.lally 2010-07-15) PLID 39626 - Add menu option for Location Templating
		DWORD dwEnableLocTemp =  MF_DISABLED|MF_GRAYED;
		if(g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
			dwEnableLocTemp = MF_ENABLED;
		}
		// (j.jones 2011-07-15 14:47) - PLID 38938 - changed to use ID_RES_EDIT_LOCATION_TEMPLATES
		pPopup->InsertMenu(102, dwEnableLocTemp | MF_BYPOSITION, ID_RES_EDIT_LOCATION_TEMPLATES, "Edit Location Templates...");
	}

	// (d.lange 2010-11-08 10:21) - PLID 41192 - add menu items for every device plugin thats enabled and has the ability to send to devic
	CArray<DeviceLaunchUtils::DevicePlugin*, DeviceLaunchUtils::DevicePlugin*> aryLoadedPlugins;
	// (d.lange 2011-01-13 09:52) - PLID 41192 - Make sure the menu item only appears when the user right clicks on a patient appointment
	if(theRes != NULL && nPatientID > 0) {
		// (j.gruber 2013-04-03 14:53) - PLID 56012 - consolidate
		long nBlank = -1;
		DeviceLaunchUtils::GenerateDeviceMenu(aryLoadedPlugins, pPopup, nBlank, FALSE, -1);
	}

	// Show the pop-up menu
	// (a.walling 2007-08-31 12:16) - PLID 27265 - We will track it here so we can tell if the dialog was dismissed
	// without clicking on anything. Then the m_pResDispatchPtr can be released. Otherwise it will be released in m_pParent's
	// Command handler.
	long nSelection = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, ptPopUpAt.x, ptPopUpAt.y, m_pParent);
	// (c.haag 2011-06-23) - PLID 36477 - Do NOT use CApptPopupMenuManager::HandlePopupMenuResult on nSelection 
	// because command messages like ID_APPT_CONFIRMED need to be routed through the parent (refer to the later call
	// to SendMessage(WM_COMMAND...).

	// (d.lange 2010-11-08 10:25) - PLID 41192 - return TRUE if the user selected a send to device menu item
	// (j.gruber 2013-04-03 14:57) - PLID 56012 - consolidate
	BOOL bLaunchDevice = DeviceLaunchUtils::LaunchDevice(aryLoadedPlugins, nSelection, nPatientID);

	if (nSelection == 0 || bLaunchDevice) {
		// dismissed
		m_pParent->m_pResDispatchPtr.SetDispatch(NULL);
		m_pParent->m_pResDispatch = (LPDISPATCH)-1;

		//TES 3/30/2011 - PLID 41519 - If this isn't for a template or an event, then up above we set this to be the "last edited" appointment,
		// but since they've now dismissed the context menu, we'll clear that flag.
		if (theRes != NULL) {
			CReservation pRes(__FUNCTION__, theRes);
			if(!bUseTemplatePopup && CWnd::FromHandle((HWND)pRes.GethWnd())->GetOwner()->m_hWnd == (HWND)m_pSingleDayCtrl.GethWnd()) {
				// (d.thompson 2012-08-01) - PLID 51898 - Changed default to Off
				if(GetRemotePropertyInt("SchedHighlightLastAppt", 0, 0, GetCurrentUserName(), true)) {
					CSingleDay pSingleDay;
					CWnd* pWndNxTextbox = CWnd::FromHandle((HWND)pRes.GethWnd());
					pSingleDay.SetDispatch((LPDISPATCH)pWndNxTextbox->GetParent()->GetControlUnknown()); // (c.haag 2010-03-26 15:11) - PLID 37332 - Use SetDispatch
					pSingleDay.SetHighlightRes(NULL);
					//TES 3/31/2011 - PLID 41519 - Clear out our "pending" highlight
					m_nPendingHighlightID = -1;
				}
			}
		}
	} else {
		//DRT 6/12/2008 - PLID 9679 - If the ID selected is one of our custom superbill templates, or the default 'print superbill', hijack the 
		//	message and re-form it slightly to pass in the template as well.
		// (c.haag 2011-08-22) - PLID 44286 - Get the superbill array from the menu manager
		const CStringArray& arySuperbillTemplates = apmm.GetSuperbillTemplates();
		//TES 1/17/2012 - PLID 47471 - This flag was not actually getting set, pull it from the menu manager
		bPromptIfSuperbillEmpty = apmm.GetPromptIfSuperbillEmpty();
		if(nSelection >= MIN_ID_SUPERBILL_TEMPLATES && nSelection < MIN_ID_SUPERBILL_TEMPLATES + arySuperbillTemplates.GetSize() || nSelection == ID_PRINTSUPERBILL) {
			//It is!
			BOOL bForcePrompt = 0;

			//Get the path.  If our selection is "print superbill", then there is only 1 option in the list, grab the first.
			CString strPath;
			if(nSelection == ID_PRINTSUPERBILL) {
				//First, we have to check for validity of the array.  It is possible that nothing is in the array and we want to 
				//	prompt the user.
				if(arySuperbillTemplates.GetSize() >= 1) {
					strPath = arySuperbillTemplates.GetAt(0);
				}
				else if(bPromptIfSuperbillEmpty) {
					bForcePrompt = 1;
					strPath = "";
				}
				else {
					//We have no templates but have not been asked to prompt.  This probably means they're on "default", but don't 
					//	actually have a default set.  Nothing we can do, leave it empty.
					strPath = "";
				}
			}
			else {
				//We have many, grab the right one by subtracting the selected ID minus the base ID.  That will
				//	give us the array element.
				strPath = arySuperbillTemplates.GetAt(nSelection - MIN_ID_SUPERBILL_TEMPLATES);
			}
			//Remember that we decided for display purposes to show only the "superbill path", so prepend the shared path and templates\forms 
			//	so we get all the way there.
			if(!strPath.IsEmpty()) {
				strPath = GetSharedPath() ^ "Templates\\Forms" ^ strPath;
			}
			BSTR bstr = strPath.AllocSysString();
			//DRT 6/12/2008 - PLID 9679 - I converted this to a normal message send rather than a WM_COMMAND.  I need to send
			//	the path through along with the message... and since we're sort of hijacking it anyways, it's questionable 
			//	whether this should be a command or a message either way.
			m_pParent->SendMessage(ID_PRINTSUPERBILL, (WPARAM)bstr, (LPARAM)bForcePrompt);
			//Cleanup after ourselves
			SysFreeString(bstr);
		}
		// (j.jones 2010-09-24 13:24) - PLID 34518 - check for dynamic copayment selections
		else if((nSelection >= MIN_ID_COPAYMENT_RESPS && nSelection < MIN_ID_COPAYMENT_RESPS + aryCopayInsuredPartyIDs.GetSize())
			|| (nSelection == ID_APPT_NEW_PRIMARY_COPAY && nPrimaryInsuredPartyID != -1)) {

			//dismiss the open appt. if we have one
			m_pParent->m_pResDispatchPtr.SetDispatch(NULL);
			m_pParent->m_pResDispatch = (LPDISPATCH)-1;

			long nInsuredPartyID = -1;
			if(nSelection == ID_APPT_NEW_PRIMARY_COPAY && nPrimaryInsuredPartyID != -1) {
				nInsuredPartyID = nPrimaryInsuredPartyID;
			}
			else if(nSelection >= MIN_ID_COPAYMENT_RESPS && nSelection < MIN_ID_COPAYMENT_RESPS + aryCopayInsuredPartyIDs.GetSize()) {
				nInsuredPartyID = aryCopayInsuredPartyIDs.GetAt(nSelection - MIN_ID_COPAYMENT_RESPS);
			}

			if(nInsuredPartyID == -1) {
				//should be impossible
				ThrowNxException("Copay creation failed - no insured party selected!");
			}

			PromptForCopay(nInsuredPartyID);
		}
		//(e.lally 2010-07-15) PLID 39626 - Open the Location Template editor via the mainframe
		else if(nSelection == ID_RESOURCEAVAIL_TEMPLATING){
			//(e.lally 2010-07-28) PLID 39626 - We aren't using the res, so we have to release our reference 
			m_pParent->m_pResDispatchPtr.SetDispatch(NULL);
			m_pParent->m_pResDispatch = (LPDISPATCH)-1;
			GetMainFrame()->EditLocationTemplating();
		}
		else {
			// create our WM_COMMAND message and dispatch to m_pParent.
			m_pParent->SendMessage(WM_COMMAND, MAKEWPARAM(nSelection, 0), 0);
		}
	}
	// (d.lange 2010-11-08 10:26) - PLID 41192 - Destory all loaded device plugins, so we don't leak memory
	// (j.gruber 2013-04-03 14:57) - PLID 56012 - consolidate
	DeviceLaunchUtils::DestroyLoadedDevicePlugins(aryLoadedPlugins);
}

void CNxSchedulerDlg::DoPopupContextMenu(IN const CPoint &ptPopUpAt, IN const DWORD dwClickedResourceID, IN const COleDateTime &dtClickedDate, IN const COleDateTime &dtClickedTime, long nClickedDay, IN const BOOL bClickedEvent, OPTIONAL IN const CSchedTemplateInfo *psti)
{
	// Since we have no reservation, we'll need to remember all the properties of the area we clicked on
	m_pParent->m_clickTargetInfo.time = dtClickedTime;
	m_pParent->m_clickTargetInfo.isEvent = !!bClickedEvent;
	m_pParent->m_clickTargetInfo.date = dtClickedDate;
	m_pParent->m_clickTargetInfo.day = nClickedDay;
	m_pParent->m_clickTargetInfo.resourceID = dwClickedResourceID;

	// And pop up the context menu 
	// (c.haag 2010-07-13 12:03) - PLID 39615 - Just pass in NULL instead of an empty reservation
	DoPopupContextMenu(ptPopUpAt, NULL, psti);
}


CSchedTemplateInfo *CalcSchedTemplateInfo(IN const COleDateTime &dtApplyDate, IN const COleDateTime &dtApplyTime, IN const long nResourceID);


		
void CNxSchedulerDlg::OnReservationRightClick(LPDISPATCH theRes, long x, long y) 
{
	try 
	{
		CReservation pRes(__FUNCTION__, theRes);
		// (d.thompson 2009-10-21) - PLID 35598 - The act of right clicking on a reservation gets a 
		//	pointer to the reservation, then passes it to functions that allow updates in the background 
		//	(the context menu).  Thus we have to signify that the reservation is locked, and if
		//	someone tries to refresh, don't let them.
		m_bResLockedByRightClick = true;

		// Kill the timer for the month view (I think this actually only has to be done for the month view, 
		// but it can't hurt to do it extra times, and it's been here forever so I'm leaving it JUST IN CASE 
		// it's really needed)
		KillTimer(ID_MONTH_VIEW_TIMER);

		// (c.haag 2007-03-15 17:41) - PLID 24514 - Don't show a pop-up if the scheduler is about to refresh
		if (m_bIsWaitingForTimedUpdate) {
			// (d.thompson 2009-10-21) - PLID 35598 - Release our lock before quitting.
			m_bResLockedByRightClick = false;
			return;
		}
		
		// (z.manning, 02/13/2008) - PLID 28909 - If this is an attendance reserveration, don't do anything (at least not yet)
		if(IsResAttendanceAppointment(CReservation(__FUNCTION__, theRes))) {
			// (d.thompson 2009-10-21) - PLID 35598 - Release our lock before quitting.
			m_bResLockedByRightClick = false;
			return;
		}

		// (v.maida 2016-04-22 16:27) - PLID 51696 - Make sure that we're on the correct tab before doing any menu selections
		if (this != m_pParent->GetActiveSheet()) {
			m_bResLockedByRightClick = false;
			return;
		}

		// Show the edit/cut/copy/paste pop-up menu at location x,y
		CPoint pt;
		GetCursorPos(&pt);

		// We can pop up the context menu easily because we have a reservation, no need to calculate all the info about where the mouse is
		// (c.haag 2010-07-13 12:03) - PLID 39615 - Just pass in theRes instead of a CReservation
		// (j.luckoski 2012-05-09 11:25) - PLID 11597 - Reservations for templates were being held here and needed to be cleared before
		// another function attempts a delete.
		pRes.SetDispatch(NULL);
		DoPopupContextMenu(pt, theRes, NULL);

	}NxCatchAll("CNxSchedulerDlg::OnReservationRightClick");

	// (d.thompson 2009-10-21) - PLID 35598 - Release our lock before quitting.
	m_bResLockedByRightClick = false;
}

void CNxSchedulerDlg::OnMouseUpGeneric(short Button, short Shift, long x, long y, BOOL bIsOnEventControl)
{
	// Kill the timer for the month view (I think this actually only has to be done for the month view, 
	// but it can't hurt to do it extra times, and it's been here forever so I'm leaving it JUST IN CASE 
	// it's really needed)
	KillTimer(ID_MONTH_VIEW_TIMER);

	// (c.haag 2007-03-15 17:41) - PLID 24514 - Don't show a pop-up if the scheduler is about to refresh
	if (m_bIsWaitingForTimedUpdate)
		return;

	// If it was the right button, let's pop up the context menu
	// (v.maida 2016-04-08 13:18) - PLID 51696 - Make sure that the current sheet is equal to the active sheet
	if (Button == MK_RBUTTON && (m_pParent && this == m_pParent->GetActiveSheet())) {

		// Find out where the mouse is, that's where we want to pop up the context menu
		CPoint pt;
		GetCursorPos(&pt);

		// Get the column the mouse is over
		long nClickInDay = GetMsDay();

		// Get the time the mouse is over
		COleDateTime dtWorkingTime = GetMsTime();

		// Get the resource and date associated with the column the mouse is over
		long nWorkingResourceID;
		COleDateTime dtWorkingDate;
		m_pParent->GetWorkingResourceAndDate(this, nClickInDay, nWorkingResourceID, dtWorkingDate);

		// See if there's a template on that spot
		// (c.haag 2006-05-22 11:32) - PLID 20614 - We now use data in memory rather than a query to
		// calculate this
		//CSchedTemplateInfo *psti = CalcSchedTemplateInfo(
		//	dtWorkingDate, 
		//	dtWorkingTime + COleDateTimeSpan(0, 0, GetActiveInterval()-1, 0), 
		//	nWorkingResourceID);

		CSchedTemplateInfo* psti = NULL;
		long nPriority = -1;
		for (int i=0; i < m_aVisibleTemplates.GetSize(); i++) {
			VisibleTemplate* pvis = m_aVisibleTemplates[i];
			COleDateTime dtWorking;
			dtWorking.SetDateTime(dtWorkingDate.GetYear(), dtWorkingDate.GetMonth(), dtWorkingDate.GetDay(),
				dtWorkingTime.GetHour(), dtWorkingTime.GetMinute(), dtWorkingTime.GetSecond());

			// (z.manning, 03/07/2007) - PLID 23431 - Make sure we only count templates for the column that
			// the user clicked in.
			//TES 9/3/2010 - PLID 39630 - Since we're just checking the day, we don't need to worry about ctrRange being split up.
			if (dtWorking >= pvis->ctrRange.GetStart() && dtWorking < pvis->ctrRange.GetEnd() && pvis->nPriority > nPriority
				&& nClickInDay == pvis->nSingleDayColumn) 
			{
				if (NULL == psti) {
					psti = new CSchedTemplateInfo;
				}
				nPriority = pvis->nPriority;
				psti->m_clrColor = pvis->clr;
				psti->m_strName = pvis->strText;
			}
		}
		

		// Finally we can pop up the context menu
		DoPopupContextMenu(pt, nWorkingResourceID, dtWorkingDate, dtWorkingTime, nClickInDay, bIsOnEventControl, psti);


		
		// Cleanup: If we got an sti object we need to deallocate it
		if (psti) {
			delete psti;
		}
	}
}

void CNxSchedulerDlg::OnMouseUpSingleday(short Button, short Shift, long x, long y) 
{
	OnMouseUpGeneric(Button, Shift, x, y, FALSE);
}

void CNxSchedulerDlg::OnMouseUpEvent(short Button, short Shift, long x, long y)
{
	OnMouseUpGeneric(Button, Shift, x, y, TRUE);
}

long CNxSchedulerDlg::GetMsSlot()
{
	long Ans = 0;
	long nSlotHeight;
	long nTopSlot;
	CPoint pt;
	CRect tmpRect;
	nSlotHeight = m_pSingleDayCtrl.GetSlotHeight();
	nTopSlot = m_pSingleDayCtrl.GetTopSlot();
	GetCursorPos(&pt);
	CWnd *pSingleDayCtrlWnd = CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd());
	pSingleDayCtrlWnd->GetWindowRect(tmpRect);
	pSingleDayCtrlWnd->ClientToScreen(tmpRect);

	pt.y += tmpRect.top / 2;
	//if (tmpRect.PtInRect(pt)) {
		long tmpLong = pt.y - tmpRect.top;
		if(tmpLong < 0) Ans = -1; //Higher than the top; must be the event slot.
		else Ans = tmpLong / nSlotHeight + nTopSlot;
	//}
	return Ans;
}

long CNxSchedulerDlg::GetMsDay(CSingleDay pCtrl)
{
	long Ans = 0;
	long nDayWidth;
	long nLeftDay;
	long nTimeButtonWidth = 0;
	CPoint pt;
	CRect tmpRect;
	nDayWidth = pCtrl.GetDayWidth();
	nLeftDay = pCtrl.GetLeftDay();
	if (pCtrl.GetTimeButtonVisible()) 
		nTimeButtonWidth = pCtrl.GetTimeButtonWidth();
	GetCursorPos(&pt);
	CWnd *pCtrlWnd = CWnd::FromHandle((HWND)pCtrl.GethWnd());
	pCtrlWnd->GetWindowRect(tmpRect);
	pCtrlWnd->ClientToScreen(tmpRect);

	pt.x += tmpRect.left / 2;
	pt.y += tmpRect.top / 2;

	if (tmpRect.PtInRect(pt)) {
		long tmpLong = pt.x - tmpRect.left - nTimeButtonWidth;
		Ans = tmpLong / nDayWidth + nLeftDay;
	}
	else
		Ans = -1;

	return Ans;
}

long CNxSchedulerDlg::GetMsDay()
{
	long Ans;

	if ((Ans = GetMsDay(m_pSingleDayCtrl)) == -1)
	{
		if ((Ans = GetMsDay(m_pEventCtrl)) == -1)		
			Ans = 0;
	}
	return Ans;
}

COleDateTime CNxSchedulerDlg::GetMsTime()
{
	int nSlot = GetMsSlot();
	COleDateTime dtBeginTime;
	COleDateTimeSpan dtInterval;
	dtBeginTime.ParseDateTime(m_pSingleDayCtrl.GetBeginTime());
	dtInterval.SetDateTimeSpan(0, 0, m_pSingleDayCtrl.GetInterval(), 0);

	return dtBeginTime.m_dt + dtInterval * (double)nSlot;
}

COleDateTime CNxSchedulerDlg::GetWorkingDate(int nDay /* = 0 */)
{
	return GetActiveDate();
}

void CNxSchedulerDlg::OnAmPmBtn() 
{
	if (m_pSingleDayCtrl != NULL && m_pParent) {
		COleDateTime dtNewTime, dtAmTime, dtPmTime;
		//Are we calculating?
		if(GetRemotePropertyInt("CalculateAmPmTime", 0, 0, "<None>", true)) {
			//OK, the A.M. time will either be the opening time, or, if the office is closed, 8:00 AM
			if(!m_pParent->m_lohOfficeHours.GetOfficeHours(GetActiveDate().GetDayOfWeek() - 1, dtAmTime, dtPmTime)) {
				dtAmTime.SetTime(8,0,0);
				dtPmTime.SetTime(13,0,0);
			}
			else {
				//The P.M. time will be (closing time - opening time) / 2
				COleDateTimeSpan dtsDayLength = dtPmTime - dtAmTime;
				dtsDayLength = dtsDayLength / 2;
				dtPmTime = dtAmTime + dtsDayLength;
			}
		}
		else {
			//TES 1/8/2009 - PLID 32661 - These were being stored as strings, which was wrong
			// and messing up the new preferences.  I made new preferences for these values,
			// which are actually stored as datetimes.
			// (c.haag 2009-02-17 10:55) - PLID 33124 - Set the date to 1899-12-30; not 1899-12-31,
			// or else the time value will contain a day as well; and break the anchor button.
			COleDateTime dtDefault;
			CString strDefault = GetRemotePropertyText("DefaultAmTime", "1899-12-30 08:00:00", 0, "<None>", false);
			dtDefault.ParseDateTime(strDefault);						
			dtAmTime = GetRemotePropertyDateTime("DefaultAmAnchorTime", &dtDefault, 0, "<None>", true);
			strDefault = GetRemotePropertyText("DefaultPmTime", "1899-12-30 13:00:00", 0, "<None>", false);
			dtDefault.ParseDateTime(strDefault);
			dtPmTime = GetRemotePropertyDateTime("DefaultPmAnchorTime", &dtDefault, 0, "<None>", true);
			// (c.haag 2009-02-17 11:11) - PLID 33124 - Remove any date value from these times in case
			// the value in data contained a day
			dtAmTime.SetTime(dtAmTime.GetHour(), dtAmTime.GetMinute(), dtAmTime.GetSecond());
			dtPmTime.SetTime(dtPmTime.GetHour(), dtPmTime.GetMinute(), dtPmTime.GetSecond());
		}

		if (m_pParent->m_bGoToAM) {
			// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
			m_nxbtnAmPmButton.SetText(FormatDateTimeForInterface(dtPmTime, DTF_STRIP_SECONDS, dtoTime));
			dtNewTime = dtAmTime;
			m_pParent->m_bGoToAM = false;
		} else {
			// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
			m_nxbtnAmPmButton.SetText(FormatDateTimeForInterface(dtAmTime, DTF_STRIP_SECONDS, dtoTime));
			//dtNewTime.SetTime(12, 0, 0);
			dtNewTime = dtPmTime;
			m_pParent->m_bGoToAM = true;
		}
		int nNewSlot;
		nNewSlot = m_pSingleDayCtrl.GetTimeSlot(dtNewTime);
		m_pSingleDayCtrl.PutTopSlot(nNewSlot);
	}
}

// Ignores the date portion of the given datetimes
void CNxSchedulerDlg::ScrollToMakeTimeRangeVisible(const COleDateTime &dtStartTime, const COleDateTime &dtEndTime)
{
	// First make the end slot visible
	m_pSingleDayCtrl.MakeSlotVisible(m_pSingleDayCtrl.GetTimeSlot(dtEndTime));
	
	// Then make the start slot visible; we do this last because it's more important that it be visible in the end
	m_pSingleDayCtrl.MakeSlotVisible(m_pSingleDayCtrl.GetTimeSlot(dtStartTime));
}

void CNxSchedulerDlg::OnTodayBtn() 
{
	BeginWaitCursor();
	if(!AllowChangeView()) return;
	// (v.maida 2016-04-22 16:27) - PLID 51696 - Don't continue if we're not on the correct tab.
	if (m_pParent && this != m_pParent->GetActiveSheet()) {
		return;
	}
	SetActiveDate(COleDateTime::GetCurrentTime());
	m_bNeedUpdate = true;
	UpdateView();
	EndWaitCursor();
}

void CNxSchedulerDlg::OnResourceOrderBtn()
{
	//DRT 4/30/03 - Requires sptRead permission to change the current view
	if (!CheckCurrentUserPermissions(bioSchedResourceOrder, sptRead)) 
		return;

	try {
		// Create the popup menu
		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		
		//TES 7/26/2010 - PLID 39445 - Added LocationID.
		_RecordsetPtr prsViews = CreateRecordset("SELECT ID, Name, LocationID FROM ResourceViewsT");
		if (!prsViews->eof) {
			// Fill the menu with items relating the individual views
			m_aryViewsByMenuPosition.RemoveAll();
			m_aryViewLocationsByMenuPosition.RemoveAll();
			FieldsPtr flds = prsViews->Fields;
			
			long nLastLoadedResourceViewID = m_pParent->GetLastLoadedResourceViewID(this);

			m_aryViewsByMenuPosition.Add(CSchedulerView::srvCurrentUserDefaultView);
			//TES 7/26/2010 - PLID 39445 - The default for the standard view is <No Default>
			m_aryViewLocationsByMenuPosition.Add(-2);
			mnu.InsertMenu(0, MF_BYPOSITION, IDM_RV_CHANGE_VIEWS, "Standard View");
			if (nLastLoadedResourceViewID == CSchedulerView::srvCurrentUserDefaultView) {
				mnu.SetDefaultItem(0, TRUE);
			}

			for (int i=1; i<MAX_VIEW_COUNT && !prsViews->eof; i++) {

				// (j.jones 2010-10-06 15:45) - PLID 40729 - for every 25 view entries,
				// add a new column
				int iNewColFlag = 0;
				if(i % 25 == 0) {
					//create a new column with this entry
					iNewColFlag = MF_MENUBARBREAK;
				}

				long nID = AdoFldLong(flds, "ID");
				m_aryViewsByMenuPosition.Add(nID);
				//TES 7/26/2010 - PLID 39445 - Get the LocationID
				m_aryViewLocationsByMenuPosition.Add(AdoFldLong(flds, "LocationID", -2));
				mnu.InsertMenu(i, MF_BYPOSITION|iNewColFlag, IDM_RV_CHANGE_VIEWS+i, AdoFldString(flds, "Name"));
				if (nLastLoadedResourceViewID == nID) {
					mnu.SetDefaultItem(i, TRUE);
				}
				prsViews->MoveNext();
			}

			// (j.jones 2010-10-06 15:45) - PLID 40729 - if the last entry filled up a column,
			// don't bother adding a separator
			int iNewColFlag = 0;
			if(i % 25 == 0) {
				//create a new column with this entry
				iNewColFlag = MF_MENUBARBREAK;
			}
			else {
				// Add a separator and then the "Edit Resources" menu item
				mnu.InsertMenu(i++, MF_BYPOSITION|MF_SEPARATOR, 0, "");
			}

			//DRT 4/30/03 - If they don't have at least write permission, they can't do anything
			//		in this dialog anyways, so gray it out
			if (!(GetCurrentUserPermissions(bioSchedResourceOrder) & (sptWrite|sptWriteWithPass)))
				mnu.InsertMenu(i++, MF_BYPOSITION|MF_GRAYED|iNewColFlag, IDM_RV_EDIT_RESOURCES, "&Edit Resources...");
			else
				mnu.InsertMenu(i++, MF_BYPOSITION|iNewColFlag, IDM_RV_EDIT_RESOURCES, "&Edit Resources...");

			// Pop up the window
			CRect rc;
			CWnd *pWnd = GetDlgItem(IDC_RESOURCE_ORDER_BTN);
			if (pWnd) {
				pWnd->GetWindowRect(&rc);
				mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
			} else {
				CPoint pt;
				GetCursorPos(&pt);
				mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
			}
		} else {
			// There are no custom views so just assume the user wants to edit resources
			PostMessage(WM_COMMAND, IDM_RV_EDIT_RESOURCES, 0);
		}

	} NxCatchAll("CNxSchedulerDlg::OnResourceOrderBtn");
}

void CNxSchedulerDlg::OnShowPatientInfoBtn()
{
	if (GetMainFrame())	GetMainFrame()->PostMessage(WM_COMMAND, ID_VIEW_SHOWPATIENTINFORMATION);
	SetFocus();
}

void CNxSchedulerDlg::OnReservationAdded(LPDISPATCH theRes) 
{
	try {
		// (v.maida 2016-04-08 13:18) - PLID 51696 - Make sure that the current sheet is equal to the active sheet
		if (m_pParent && m_ResEntry && this == m_pParent->GetActiveSheet()) {
			CWaitCursor wc;
			// Do some stuff to the resentry dialog based on the reservation object, make sure the 
			// CReservation goes out of scope before we call ZoomRes
			long nWorkingResourceID;
			COleDateTime dtWorkingDate;
			// (c.haag 2010-08-27 11:38) - PLID 40108 - "Reserve" the resentrydlg so that UpdateView
			// messages get ignored while the resentry object is being initialized.
			CReserveResEntry rre(m_ResEntry, __FUNCTION__);
			{
				CReservation pRes(__FUNCTION__, theRes);
				// Then get the info out of the current view (we could be in the week or multi-resource tab)
				m_pParent->GetWorkingResourceAndDate(this, pRes.GetDay(), nWorkingResourceID, dtWorkingDate);

				//Increment the count on screen.
				EnsureCountLabelText();
			}
			// Show the appointment (after the CReservation smart-pointer has gone out of scope)
			m_ResEntry->ZoomResNewAppointment(theRes, nWorkingResourceID, dtWorkingDate);
		}
	} NxCatchAllCall("CNxSchedulerDlg::OnReservationAdded", try { CReservation(__FUNCTION__, theRes).DeleteRes(__FUNCTION__); } NxCatchAllIgnore(); );
}

// (c.haag 2005-12-09 16:18) - PLID 16849 - This function was written to make
// CommitResDrag more maintainable. We're breaking the function into two sections:
// validation and execution. All of the validation is done in CanSaveReservation.
// All of the execution will take place in a transaction.
//

//
// (c.haag 2005-12-12 16:38) - PLID 16849 - AppointmentGrabWithValidation
// is more than 255 debug characters long
//
#pragma warning(push)
#pragma warning (disable:4786)
// (j.jones 2014-12-02 15:39) - PLID 64182 - if a mix rule is overridden, it is passed up to the caller
_RecordsetPtr CNxSchedulerDlg::AppointmentGrabWithValidation(CReservation& pRes, IN const COleDateTime &dtDraggedFromDate, IN const long nDraggedFromResourceID, OUT std::vector<SchedulerMixRule> &overriddenMixRules, OUT SelectedFFASlotPtr &pSelectedFFASlot)
{
	try {
		//
		// Make sure we have a valid reservation object
		//
		if (NULL == pRes) {
			ASSERT(FALSE); // (c.haag 2005-12-14 09:48) - We should never get here I think
			return NULL;
		}

		//
		//BVB - this is a little funky if they password this function, but no one in there right mind will.
		//Allow and deny work fine
		//
		if (!UserPermission(DragAppointment))
			return NULL;

		long nResID = pRes.GetReservationID();

		_RecordsetPtr pAppt = AppointmentGrab(nResID, TRUE, TRUE);
		FieldsPtr f = pAppt->Fields;

		
		// (j.luckoski 2012-05-07 11:22) - PLID 11597 - If cancelled don't allow grab and drag
		// (a.walling 2013-01-21 16:48) - PLID 54744 - Available in recordset
		if (4 == AdoFldByte(pAppt, "Status", -1)) {
			return NULL;
		}

		//
		// If the appointment was modified since you dragged it, raise
		// an action prompt.

		//DRT 10/7/2008 - PLID 31350 - If you get an ASSERT here because m_dtActiveApptLastModified is invalid, see 
		//	this PLID, and the corresponding comment in CSchedulerView::TrackLastModifiedDate().  I do not believe
		//	it's possible to get an assert here without first getting one there.  No resolution or cause is yet found.
		if (m_pParent->m_dtActiveApptLastModified < AdoFldDateTime(f, "ModifiedDate")) {
			if (IDNO == MsgBox(MB_YESNO, "This appointment was changed by %s since you started to modify it. Do you wish to overwrite that user's changes?", AdoFldString(f, "ModifiedLogin")))
			{
				return NULL;
			}
		}

		//
		// (c.haag 2006-05-23 11:51) - PLID 20665 - Calculate the resources
		//
		CDWordArray adwResourceIDs;
		//GetAppointmentResourceIDs(nResID, adwResourceIDs);
		LoadResourceIDStringIntoArray(AdoFldString(f, "ResourceIDs", ""), adwResourceIDs);

		//
		// Find out the viewinfo of the res before the dragging started
		//
		long nDroppingOnResourceID;
		COleDateTime dtDroppingOnDate;
		m_pParent->GetWorkingResourceAndDate(this, pRes.GetDay(), nDroppingOnResourceID, dtDroppingOnDate);

		//
		// If we're dragging the appointment to a different resource we have to do an extra check to 
		// see if the appointment already has a record for that resource.  If it does, then cancel the 
		// drag-and-drop and tell the user why, because the user obviously didn't intend to do this.
		//
		if (nDraggedFromResourceID != nDroppingOnResourceID)
		{
			// If we're dragging the appointment to a resource that doesn't exist, check for it
			// (j.luckoski 2012-05-09 16:07) - PLID 50264 - User ReturnsRecordsParam
			if (!ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT ID FROM ResourceT WHERE ID = {INT}", nDroppingOnResourceID))	{
				MsgBox("You cannot drag the appointment to this resource because it has been deleted by another user.");
				return NULL;
			}

			// (c.haag 2006-05-23 10:55) - PLID 20665 - We already have this information in memory
			//
			//rs = CreateRecordset("SELECT TOP 1 AppointmentID FROM AppointmentResourceT WHERE AppointmentResourceT.AppointmentID = %d AND AppointmentResourceT.ResourceID = %d",
			//	pRes.GetReservationID(), nDroppingOnResourceID);
			//if (!rs->eof)
			//{
			//	rs->Close();
			//	MsgBox("The appointment you have dragged is already assigned to this resource, and cannot be moved there.");
			//	return NULL;
			//}
			//rs->Close();
			for (int i=0; i < adwResourceIDs.GetSize(); i++) {
				if (nDroppingOnResourceID == (long)adwResourceIDs[i]) {
					MsgBox("The appointment you have dragged is already assigned to this resource, and cannot be moved there.");
					return NULL;
				}
			}
		}

		//
		// The following code substitutes the call to AppointmentUpdate in the scope of
		// appointment validation
		//
		long nPatientID = AdoFldLong(f, "PatientID");
		long nLocationID = AdoFldLong(f, "LocationID");
		long nAptTypeID = AdoFldLong(f, "AptTypeID", -1);
		// (j.jones 2014-12-02 09:31) - PLID 64182 - get the primary insured ID
		long nPrimaryInsuredPartyID = AdoFldLong(f, "PrimaryInsuredPartyID", -1);
		COleDateTime dtDate = dtDroppingOnDate;
		COleDateTime dtStart = pRes.GetStartTime();
		COleDateTime dtEnd = pRes.GetEndTime();
		CDWordArray adwPurposeIDs;
		LoadPurposeIDStringIntoArray(AdoFldString(f, "PurposeIDs", ""), adwPurposeIDs);

		if (nDraggedFromResourceID != nDroppingOnResourceID) {
			for (long i=0; i < adwResourceIDs.GetSize(); i++) {
				if (nDraggedFromResourceID == (long)adwResourceIDs[i]) {
					adwResourceIDs[i] = nDroppingOnResourceID;
					break;
				}
			}
						
			// (a.walling 2010-06-17 12:27) - PLID 23560 - Check resource sets when dragging
			CString strMessage;
			bool bNoConflicts = CheckExistingApptResourceSetConflicts(nAptTypeID, nResID, adwResourceIDs, strMessage);
			
			if (!bNoConflicts) {
				int nRet = MessageBox(strMessage, NULL, MB_YESNO|MB_ICONEXCLAMATION);

				if (nRet == IDYES) {
					if (!CheckAdministratorPassword()) {
						return NULL;
					}
				} else {
					return NULL;
				}
			}
		}
		dtDate.SetDate(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay());
		dtStart.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtStart.GetHour(), dtStart.GetMinute(), dtStart.GetSecond());
		dtEnd.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), dtEnd.GetHour(), dtEnd.GetMinute(), dtEnd.GetSecond());
		if(CWnd::FromHandle((HWND)pRes.GethWnd())->GetOwner()->m_hWnd == (HWND)m_pSingleDayCtrl.GethWnd() &&
			dtStart.GetHour() == 0 && dtStart.GetMinute() == 0 && dtEnd.GetHour() == 0 && dtEnd.GetMinute() == 0)
		{
			//We're on the singleday, and yet the start and end time are both midnight.  That will be confusing.
			//Let's set the end time to 11:59, to keep things clear.
			dtEnd.SetDateTime(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay(), 23, 59, 0);
		}

		//
		// If someone made an appointment in such a way that the end time is
		// 12:00am and the start time is not 12:00am, we assume the appt spilled
		// into the next day, and therefore, we have to change the end time to
		// 11:59pm
		//
		if (dtEnd.GetHour() == 0 && dtEnd.GetMinute() == 0 &&
			!(dtStart.GetHour() == 0 && dtStart.GetMinute() == 0))
		{
			dtEnd.SetDateTime(dtEnd.GetYear(), dtEnd.GetMonth(), dtEnd.GetDay(),
				23,59,0);
		}

		//
		//DRT 5/7/03 - Warn them for insurance authorizations.  If they said "no" to the warning, we don't want to save the appointment
		//		This really has nothing to do with permissions, but this warning should be popping up anywhere you'd be checking for
		//		double booking, so it's a logical place to put it
		//		The recordset (pAppt) is already open, so we're not using much resources to read the fields out of it
		//
		if(!AttemptWarnForInsAuth(nPatientID, dtDate))
		{
			return NULL;
		}

		//
		// Validate the appointment as is by permissions
		//
		if (!AppointmentValidateByPermissions(nPatientID, adwResourceIDs, nLocationID, dtDate,
			dtStart, dtEnd, nAptTypeID, adwPurposeIDs, nResID))
		{
			return NULL;
		}

		//
		// If the user will be prompted to decide how to manually deal with
		// linked appointments, we don't have a guarantee the user will
		// move them. Therefore, they should be ignored when we check templates.
		//
		BOOL bIgnoreLinkedAppts;
		if (GetRemotePropertyInt("PromptOnModifyLinkedAppts", 0, 0, "<None>", true))
			bIgnoreLinkedAppts = TRUE;
		else
			bIgnoreLinkedAppts = FALSE;

		//TES 10/27/2010 - PLID 40868 - We need to construct a VisibleAppointment struct representing the new appointment, we
		// can then pass it to CalculateTemplateItemID() to determine which precision template (if any) to use in the validation.
		VisibleAppointment appt;
		appt.nReservationID = nResID;
		appt.nAptTypeID = nAptTypeID;
		appt.anResourceIDs.Copy(adwResourceIDs);
		appt.anPurposeIDs.Copy(adwPurposeIDs);
		appt.dtStart = dtStart;
		appt.dtEnd = dtEnd;
		appt.nOffsetDay = (short)pRes.GetDay();
		//
		// Validate the appointment as is by rules
		//
		if (!AppointmentValidateByRules(nPatientID, adwResourceIDs, nLocationID, dtDate,
			dtStart, dtEnd, nAptTypeID, adwPurposeIDs, nResID, bIgnoreLinkedAppts, FALSE, CalculateTemplateItemID(&appt)))
		{
			return NULL;
		}

		//
		// Validate the appointment as is by alarms
		//
		if (!AppointmentValidateByAlarms(nPatientID, dtDate, dtStart, nAptTypeID, adwPurposeIDs,
			adwResourceIDs, nResID))
		{
			return NULL;
		}

		//
		// Validate the appointment as is by duration
		//
		if(GetRemotePropertyInt("ApptValidateByDuration", 0, 0, "<None>", true)) {
			if (!AppointmentValidateByDuration(dtStart, dtEnd, nAptTypeID, adwResourceIDs,
				adwPurposeIDs))
			{
				return NULL;
			}
		}

		// (j.jones 2014-12-02 09:27) - PLID 64182 - Validate against Scheduler Mix Rules last, and ignore the current appointment.
		// Also we do not need to validate if they haven't dragged it to a different date or a different resource,
		// as those are the only things dragging can change that affects mix rules (appt. duration has no effect).
		if (dtDraggedFromDate != dtDroppingOnDate || nDraggedFromResourceID != nDroppingOnResourceID) {
			if (!AppointmentValidateByMixRules(this, nPatientID, dtDate, nLocationID, nPrimaryInsuredPartyID, nAptTypeID, adwResourceIDs, adwPurposeIDs, overriddenMixRules, pSelectedFFASlot, nResID)) {
				return NULL;
			}
		}

		//
		// If we get here, all validation has passed, so return the recordset
		//
		return pAppt;
	}
	NxCatchAll("Error validating the appointment being dragged");
	return NULL;
}
#pragma warning(pop)

// (c.haag 2005-12-14 09:53) - PLID 16849 - This is the new way we use the pseudo semaphore.
// This will guarantee m_bAllowUpdate is false through the CommitResDrag function, and that
// it's set back to true when we exit the function no matter how we exit.
//
class CPreventUpdate
{
protected:
	bool& m_bAllowUpdate;
	bool m_bOldValue;

public:
	void Release() {
		m_bAllowUpdate = m_bOldValue;
	}

public:
	CPreventUpdate(bool& bAllowUpdate) : m_bAllowUpdate(bAllowUpdate) {
		m_bOldValue = bAllowUpdate;
		bAllowUpdate = false;
	}
	CPreventUpdate::~CPreventUpdate() {
		Release();
	}
};

// (a.wilson 2014-09-22 09:58) - PLID 63170 - update to use the full parameter extablechecker.
void CNxSchedulerDlg::CommitResDrag(LPDISPATCH theRes, IN const COleDateTime &dtDraggedFromDate, IN const long nDraggedFromResourceID)
{
	//
	// (c.haag 2005-12-19 10:50) - PLID 16849 - I basically rewrote this function.
	// All comments of mine without punch list ID's made in the month of December 2005
	// should be considered part of the item.
	//
	try {
		_RecordsetPtr pAppt;
		CReservation pRes(__FUNCTION__, theRes);
		CWaitCursor wc;
		CPreventUpdate pu(m_bAllowUpdate);

		// (c.haag 2010-08-11 12:58) - PLID 40077 - This is much too soon to reset the dragging reservation.
		// If later code causes a pop-up to appear, then the reservation being dragged can potentially be deleted
		// by a table checker, causing problems.
		//if (m_pParent) {
		//	// We're not dragging or resizing a reservation anymore
		//	m_pParent->m_pResDragging = NULL;
		//}

		//TES 3/8/2011 - PLID 41519 - This is now the "last edited" appointment, so highlight it if this is the singleday, and if
		// that's what we're doing.
		// (d.thompson 2012-08-01) - PLID 51898 - Changed default to Off
		if(GetRemotePropertyInt("SchedHighlightLastAppt", 0, 0, GetCurrentUserName(), true) 
			&& CWnd::FromHandle((HWND)pRes.GethWnd())->GetOwner()->m_hWnd == (HWND)m_pSingleDayCtrl.GethWnd()) {
			CSingleDay pSingleDay;
			CWnd* pWndNxTextbox = CWnd::FromHandle((HWND)pRes.GethWnd());
			pSingleDay.SetDispatch((LPDISPATCH)pWndNxTextbox->GetParent()->GetControlUnknown()); // (c.haag 2010-03-26 15:11) - PLID 37332 - Use SetDispatch
			pSingleDay.SetHighlightRes(theRes);
			//TES 3/31/2011 - PLID 41519 - Clear out our "pending" highlight
			m_nPendingHighlightID = -1;
		}

		//
		// (c.haag 2005-12-09 17:20) - PLID 16849 - Validate the appointment here. If it's valid,
		// then we get a recordset to it. If not, it returns NULL.
		//
		// (j.jones 2014-12-02 15:39) - PLID 64182 - if a mix rule is overridden, it is passed up to the caller
		std::vector<SchedulerMixRule> overriddenMixRules;
		SelectedFFASlotPtr pSelectedFFASlot;
		pSelectedFFASlot.reset();
		if (NULL == (pAppt = AppointmentGrabWithValidation(pRes, dtDraggedFromDate, nDraggedFromResourceID, overriddenMixRules, pSelectedFFASlot)))
		{
			pu.Release();
			pRes.ReleaseAndClear(); // (c.haag 2010-05-06 11:38) - PLID 38655 - Release our reference to the reservation
								// because UpdateView will disconnect it from the SingleDay.
			if (m_pParent) {
				// (c.haag 2010-08-11 12:58) - PLID 40077 - We're not dragging or resizing a reservation anymore
				m_pParent->m_pResDragging = NULL;
			}
			m_bNeedUpdate = true;
			UpdateView();
			return;
		}

		//
		// (c.haag 2005-12-12 16:43) - By this point in execution, all validation has been completed
		// and we are definitely changing the appointment. Now we gather all the NEW appointment information
		//		
		long nDroppingOnResourceID;
		COleDateTime dtDroppingOnDate;
		m_pParent->GetWorkingResourceAndDate(this, pRes.GetDay(), nDroppingOnResourceID, dtDroppingOnDate);
		COleDateTime dtNewDate = dtDroppingOnDate;
		long nResID = pRes.GetReservationID();
		FieldsPtr f = pAppt->Fields;		
		COleDateTime dtNewStart = pRes.GetStartTime();
		COleDateTime dtNewEnd = pRes.GetEndTime();
		long nPatientID = AdoFldLong(pAppt, "PatientID");
		long nShowState = AdoFldLong(pAppt, "ShowState");
		long nLocationID = AdoFldLong(pAppt, "LocationID");
		long nStatus = (long)AdoFldByte(pAppt, "Status");
		dtNewDate.SetDate(dtNewDate.GetYear(), dtNewDate.GetMonth(), dtNewDate.GetDay());
		dtNewStart.SetDateTime(dtNewDate.GetYear(), dtNewDate.GetMonth(), dtNewDate.GetDay(), dtNewStart.GetHour(), dtNewStart.GetMinute(), dtNewStart.GetSecond());
		dtNewEnd.SetDateTime(dtNewDate.GetYear(), dtNewDate.GetMonth(), dtNewDate.GetDay(), dtNewEnd.GetHour(), dtNewEnd.GetMinute(), dtNewEnd.GetSecond());
		if(CWnd::FromHandle((HWND)pRes.GethWnd())->GetOwner()->m_hWnd == (HWND)m_pSingleDayCtrl.GethWnd() &&
			dtNewStart.GetHour() == 0 && dtNewStart.GetMinute() == 0 && dtNewEnd.GetHour() == 0 && dtNewEnd.GetMinute() == 0)
		{
			//We're on the singleday, and yet the start and end time are both midnight.  That will be confusing.
			//Let's set the end time to 11:59, to keep things clear.
			dtNewEnd.SetDateTime(dtNewDate.GetYear(), dtNewDate.GetMonth(), dtNewDate.GetDay(), 23, 59, 0);
		}

		// If someone made an appointment in such a way that the end time is
		// 12:00am and the start time is not 12:00am, we assume the appt spilled
		// into the next day, and therefore, we have to change the end time to
		// 11:59pm
		if (dtNewEnd.GetHour() == 0 && dtNewEnd.GetMinute() == 0 &&
			!(dtNewStart.GetHour() == 0 && dtNewStart.GetMinute() == 0))
		{
			dtNewEnd.SetDateTime(dtNewEnd.GetYear(), dtNewEnd.GetMonth(), dtNewEnd.GetDay(),
				23,59,0);
		}

		COleDateTime dtOldDate = AdoFldDateTime(f, "Date");
		COleDateTime dtOldStart = AdoFldDateTime(f, "StartTime");
		COleDateTime dtOldEnd = AdoFldDateTime(f, "EndTime");

		// (z.manning, 4/19/2006, PLID 19088) - Calculate new arrival time.
		COleDateTime dtOldArrival = AdoFldDateTime(f, "ArrivalTime");
		COleDateTime dtNewArrival = dtOldArrival - (dtOldStart -  dtNewStart);
		// Make sure we didn't go to a previous day
		if(dtNewArrival.GetDay() != dtNewStart.GetDay()) {
			dtNewArrival.SetDateTime(dtNewStart.GetYear(), dtNewStart.GetMonth(), dtNewStart.GetDay(), 0, 0, 0);
		}

		// (j.jones 2014-12-19 10:52) - PLID 64182 - if a new appointment slot was provided, move the appt.
		// to that slot
		long nOldLocationID = nLocationID;
		if (pSelectedFFASlot != NULL && pSelectedFFASlot->IsValid()) {			
			dtNewStart = pSelectedFFASlot->dtStart;
			dtNewEnd = pSelectedFFASlot->dtEnd;
			dtNewArrival = pSelectedFFASlot->dtArrival;
			dtNewDate.SetDate(dtNewStart.GetYear(), dtNewStart.GetMonth(), dtNewStart.GetDay());
			nLocationID = pSelectedFFASlot->nLocationID; // (r.farnworth 2016-02-02 12:08) - PLID 68116 - FFA results transmit location to new appointment.
		}

		//
		// (c.haag 2005-12-12 17:00) - Now gather all the OLD appointment information
		//
		CDWordArray adwOldResources;

		// (d.moore 2007-10-15) - PLID 26546 - Get the moveup value for the old appointment.
		BOOL bMoveUp = AdoFldBool(f, "MoveUp", FALSE);

		// (c.haag 2006-05-23 12:04) - PLID 20665 - New, faster method
		//try {
		//	GetAppointmentResourceIDs(nResID, adwOldResources);
		//} NxCatchAllCall("Error gathering old appointment information", {UpdateView();return;})
		// (a.wilson 2014-09-22 09:58) - PLID 63170 - have to replace the old resource with the newly dragged one.
		CString strResourceIDs = AdoFldString(f, "ResourceIDs", "");
		LoadResourceIDStringIntoArray(strResourceIDs, adwOldResources);

		// (j.jones 2014-12-19 10:52) - PLID 64182 - if a new appointment slot was provided, check
		// to see if the resource changed
		if (pSelectedFFASlot != NULL && pSelectedFFASlot->IsValid()) {
			strResourceIDs = GenerateDelimitedListFromLongArray(pSelectedFFASlot->dwaResourceIDs, " ");
		}
		//otherwise just replace the resource the user changed
		else if (nDraggedFromResourceID != nDroppingOnResourceID) {
			strResourceIDs = (" " + strResourceIDs + " ");
			strResourceIDs.Replace(FormatString(" %li ", nDraggedFromResourceID), FormatString(" %li ", nDroppingOnResourceID));
			strResourceIDs.Trim();
		}

		// Make sure the new appointment location is valid
		long nNewLocationID = nLocationID;
		if (GetValidAppointmentLocation(nLocationID, nNewLocationID, nResID, dtNewDate, dtNewStart))
		{
			// If the appointment was dropped into a new location template, make sure the LocationID is updated
			if (nLocationID != nNewLocationID)
			{
				nLocationID = nNewLocationID;
			}
		}

		//
		// (c.haag 2005-12-12 17:21) - Do some pre-save decision making
		//
		//DRT 7/8/2005 - PLID 16664 - If there are superbills tied to this appointment, give the user the opportunity to mark them VOID.
		//	Only if the date changed (not time)
		bool bVoidSuperbills = false;
		CDWordArray arySuperbill;
		try {
			if((GetRemotePropertyInt("ShowVoidSuperbillPrompt", 1, 0, "<None>", false) == 1) && (GetCurrentUserPermissions(bioVoidSuperbills) & sptWrite))
			{
				CString strOldDate = FormatDateTimeForSql(dtOldDate, dtoDate);
				CString strNewDate = FormatDateTimeForSql(dtNewDate, dtoDate);
				if(strOldDate != strNewDate) {
					_RecordsetPtr prsSuperbill = CreateRecordset("SELECT SavedID FROM PrintedSuperbillsT WHERE ReservationID = %li AND Void = 0", pRes.GetReservationID());
					CString strSuperbillIDs = "";
					long nSuperbillIDCnt = 0;
					while(!prsSuperbill->eof) {
						long nID = AdoFldLong(prsSuperbill, "SavedID", -1);
						if(nID > -1) {
							CString str;	str.Format("%li, ", nID);
							strSuperbillIDs += str;
							arySuperbill.Add(nID);
							nSuperbillIDCnt++;
						}

						prsSuperbill->MoveNext();
					}
					strSuperbillIDs.TrimRight(", ");
					prsSuperbill->Close();

					if(nSuperbillIDCnt > 0) {
						//They are tied to superbills, we will warn the user and give them an opportunity to give up
						CString strFmt;
						strFmt.Format("This appointment is tied to %li superbill%s (ID%s:  %s).  "
							"Do you wish to mark these superbills as VOID?\r\n\r\n"
							" - If you choose YES, all related superbill IDs will be marked VOID.\r\n"
							" - If you choose NO, the superbills will remain tied to this appointment.", 
							nSuperbillIDCnt, nSuperbillIDCnt == 1 ? "" : "s", nSuperbillIDCnt == 1 ? "" : "s", strSuperbillIDs);

						if(AfxMessageBox(strFmt, MB_YESNO) == IDYES) {
							//void these superbills
							bVoidSuperbills = true;
						}
					}
				}
			}
		// (c.haag 2010-05-06 11:38) - PLID 38655 - Release our reference to the reservation
		// because UpdateView will disconnect it from the SingleDay.

			// (c.haag 2010-08-11 12:58) - PLID 40077 - We're not dragging or resizing a reservation anymore
		} NxCatchAllCall("Error checking for potentially voided superbills", {pRes.ReleaseAndClear();if (m_pParent) {m_pParent->m_pResDragging = NULL;}UpdateView();return;})

		//
		// (c.haag 2005-12-12 17:03) - Generate the SQL statement that will change the appointment
		//
		//TES 5/5/2008 - PLID 29319 - Update the ModifiedDate and ModifiedLogin fields.
		// (d.thompson 2009-12-31) - PLID 36740 - Parameterized for use in UpdateProcInfoSurgeon
		CString strApptBatch = BeginSqlBatch();
		CNxParamSqlArray args;
		AddParamStatementToSqlBatch(strApptBatch, args, 
			"UPDATE AppointmentsT SET Date = {STRING}, StartTime = {STRING}, EndTime = {STRING}, ArrivalTime = {STRING}, "
			"ModifiedDate = getdate(), ModifiedLogin = {STRING}, LocationID = {INT} WHERE ID = {INT};",
			FormatDateTimeForSql(dtNewDate), FormatDateTimeForSql(dtNewStart), FormatDateTimeForSql(dtNewEnd), FormatDateTimeForSql(dtNewArrival), 
			GetCurrentUserName(), nLocationID, nResID);

		// (j.jones 2014-12-19 10:52) - PLID 64182 - if a new appointment slot was provided, check
		// to see if the resource changed
		if (pSelectedFFASlot != NULL && pSelectedFFASlot->IsValid()) {
			AddParamStatementToSqlBatch(strApptBatch, args, "DELETE FROM AppointmentResourceT WHERE AppointmentID = {INT};", nResID);
			for (int i = 0; i < pSelectedFFASlot->dwaResourceIDs.GetSize(); i++) {
				AddParamStatementToSqlBatch(strApptBatch, args, "INSERT INTO AppointmentResourceT (AppointmentID, ResourceID) VALUES ({INT}, {INT})", nResID, pSelectedFFASlot->dwaResourceIDs[i]);
			}
		}
		//otherwise just save the resource the user changed
		else if (nDraggedFromResourceID != nDroppingOnResourceID) {
			AddParamStatementToSqlBatch(strApptBatch, args,
				"UPDATE AppointmentResourceT SET ResourceID = {INT} WHERE ResourceID = {INT} AND AppointmentID = {INT};",
				nDroppingOnResourceID, nDraggedFromResourceID, nResID);
		}

		// (c.haag 2010-04-01 15:10) - PLID 38005 - Delete from the appointment reminder table
		if (dtOldDate != dtNewDate || dtOldStart != dtNewStart) {
			AddParamStatementToSqlBatch(strApptBatch, args, 
				"DELETE FROM AppointmentRemindersT WHERE AppointmentID = {INT};\r\n"
				, nResID);
		}

		// (j.jones 2014-12-02 15:32) - PLID 64182 - if the user overrode a mix rule,
		// track that they did so
		if (overriddenMixRules.size() > 0) {
			TrackAppointmentMixRuleOverride(nResID, overriddenMixRules);
		}

		//TES 11/25/2008 - PLID 32066 - Update any ProcInfo records tied to this appointment, they
		// should maybe have a new surgeon.
		// (d.thompson 2009-12-31) - PLID 36740 - UpdateProcInfoSurgeon now requires a parameterized batch
		// (c.haag 2013-11-27) - PLID 59831 - UpdateProcInfoSurgeon no longer returns SQL. This must be run
		// outside the query batch. We can't put it in a BEGIN_TRANS ... or else the API could freeze waiting for
		// Practice to release locks.

		//
		// (c.haag 2005-12-12 17:03) - Generate the audit information
		//
		long nAuditID = BeginAuditTransaction();
		CString strNewStart = FormatDateTimeForInterface(dtNewStart);
		CString strNewEnd = FormatDateTimeForInterface(dtNewEnd);
		CString strNewArrival = FormatDateTimeForInterface(dtNewArrival);
		CString strOldStart = FormatDateTimeForInterface(dtOldStart);
		CString strOldEnd = FormatDateTimeForInterface(dtOldEnd);
		CString strOldArrival = FormatDateTimeForInterface(dtOldArrival);
		CString strPatientName = GetExistingPatientName(nPatientID);
		CString strOldLocationName = GetLocationName(nOldLocationID);
		CString strNewLocationName = GetLocationName(nLocationID);
		try {
			if(strOldStart != strNewStart) {	//these are formatted in the same way above, so they should match if the same
				AuditEvent(nPatientID, strPatientName, nAuditID, aeiApptStartTime, nResID, strOldStart, strNewStart, aepHigh);
			}
			if(strOldEnd != strNewEnd) {	//these are formatted in the same way above, so they should match if the same
				AuditEvent(nPatientID, strPatientName, nAuditID, aeiApptEndTime, nResID, strOldEnd, strNewEnd, aepHigh);
			}
			if(strOldArrival != strNewArrival) {
				AuditEvent(nPatientID, strPatientName, nAuditID, aeiApptArrivalTime, nResID, strOldArrival, strNewArrival, aepMedium);
			}
			// (r.farnworth 2016-02-03 08:22) - PLID 68116 - We need to audit locations now
			if (strOldLocationName != strNewLocationName) {
				AuditEvent(nPatientID, strPatientName, nAuditID, aeiApptLocation, nResID, strOldLocationName, strNewLocationName, aepMedium);
			}
			//audit the resources!
			// (j.jones 2014-12-19 10:52) - PLID 64182 - if a new appointment slot was provided, check
			// to see if the resource changed
			if (pSelectedFFASlot != NULL && pSelectedFFASlot->IsValid()) {
				//did the resources change?
				bool bChanged = false;
				if (adwOldResources.GetSize() != pSelectedFFASlot->dwaResourceIDs.GetSize()) {
					bChanged = true;
				}
				else {
					//the lists are the same size - are they different?
					for (int i = 0; i < adwOldResources.GetSize() && !bChanged; i++) {
						bool bFound = false;
						for (int j = 0; j < pSelectedFFASlot->dwaResourceIDs.GetSize() && !bFound; j++) {
							if (adwOldResources[i] == pSelectedFFASlot->dwaResourceIDs[j]) {
								bFound = true;
							}
						}
						//a resource in the old list isn't in the new list
						if (!bFound) {
							bChanged = true;
						}
					}
				}

				if (bChanged) {
					CString strOldRes, strNewRes;
					_RecordsetPtr prsResource = CreateParamRecordset("SELECT Item FROM ResourceT WHERE ID IN ({INTARRAY});\r\n"
						"SELECT Item FROM ResourceT WHERE ID IN ({INTARRAY})",
						adwOldResources, pSelectedFFASlot->dwaResourceIDs);

					while(!prsResource->eof) {
						if (!strOldRes.IsEmpty()) {
							strOldRes += ", ";
						}
						strOldRes += VarString(prsResource->Fields->Item["Item"]->Value);
						prsResource->MoveNext();
					}

					prsResource = prsResource->NextRecordset(NULL);

					while (!prsResource->eof) {
						if (!strNewRes.IsEmpty()) {
							strNewRes += ", ";
						}
						strNewRes += VarString(prsResource->Fields->Item["Item"]->Value);
						prsResource->MoveNext();
					}

					AuditEvent(nPatientID, strPatientName, nAuditID, aeiApptResource, nResID, strOldRes, strNewRes, aepMedium, aetChanged);
				}
			}
			//otherwise just audit the resource the user changed
			else if(nDraggedFromResourceID != nDroppingOnResourceID) {
				CString strOldRes, strNewRes;
				// (j.jones 2010-11-17 13:50) - PLID 41521 - fixed so these are separate recordsets, not a unioned dataset
				_RecordsetPtr prsResource = CreateParamRecordset("SELECT Item FROM ResourceT WHERE ID = {INT};\r\n"
					"SELECT Item FROM ResourceT WHERE ID = {INT}",
					nDraggedFromResourceID, nDroppingOnResourceID);

				strOldRes = VarString(prsResource->Fields->Item["Item"]->Value);

				prsResource = prsResource->NextRecordset(NULL);

				strNewRes = VarString(prsResource->Fields->Item["Item"]->Value);

				AuditEvent(nPatientID, strPatientName, nAuditID, aeiApptResource, nResID, strOldRes, strNewRes, aepMedium, aetChanged);
			}
		// (c.haag 2010-05-06 11:38) - PLID 38655 - Release our reference to the reservation
		// because UpdateView will disconnect it from the SingleDay.
		// (c.haag 2010-08-11 12:58) - PLID 40077 - We're not dragging or resizing a reservation anymore
		} NxCatchAllCall("Error generating audit details", {pRes.ReleaseAndClear();if (m_pParent) {m_pParent->m_pResDragging = NULL;}UpdateView();return;})

		//
		// (c.haag 2005-12-13 10:10) - Close the appointment recordset now that we're
		// done with it
		//
		pAppt->Close();

		//
		// (c.haag 2005-12-12 17:10) - Now save the appointment and auditing information. We've never
		// used transactions before; I would like to avoid introducing them here.
		//
		//(e.lally 2008-11-26) PLID 32156 - Preformatted queries must use std Sql execution.
		try {
			// (d.thompson 2009-12-31) - PLID 36740 - Parameterized the batch
			//ExecuteSqlStd(strSqlAppt);
			ExecuteParamSqlBatch(GetRemoteData(), strApptBatch, args);
			CommitAuditTransaction(nAuditID);		
			// (c.haag 2013-11-27) - PLID 59831 - Run UpdateProcInfoSurgeon here.
			PhaseTracking::UpdateProcInfoSurgeon(-2, nResID);
		// (c.haag 2010-05-06 11:38) - PLID 38655 - Release our reference to the reservation
		// because UpdateView will disconnect it from the SingleDay.
			// (c.haag 2010-08-11 12:58) - PLID 40077 - We're not dragging or resizing a reservation anymore
		} NxCatchAllCall("Error moving the appointment", {pRes.ReleaseAndClear();if (m_pParent) {m_pParent->m_pResDragging = NULL;}UpdateView();return;})

		//
		// (c.haag 2005-12-12 17:12) - Now that the data has been saved, lets do post-saving stuff like
		// move-ups and voiding superbills.
		//

		try {
			// (c.haag 2004-06-23 09:47) - Flag the appointment as having been
			// modified in the Palm. If we change resources, and only call this
			// after the fact, then it won't actually be synced because it may
			// not get through the filter.
			UpdatePalmSyncT(nResID);
			
			// Look for a move-up appointment to fill the opened time
			// (d.thompson 2012-08-01) - PLID 51898 - Changed default to Prompt (1)
			if(GetRemotePropertyInt("ApptCheckOpenSlotsForMoveUps", 1, 0, GetCurrentUserName(), true) == 1) {
				AttemptFindAvailableMoveUp(dtOldStart, dtOldEnd, dtNewStart, dtNewEnd, nResID, (nDroppingOnResourceID != nDraggedFromResourceID), adwOldResources);
			}

			//update case history dates, if needed
			if(dtOldDate != dtNewDate && IsSurgeryCenter(false)) {
				COleDateTime dtDateOnly;
				dtDateOnly.SetDate(dtNewDate.GetYear(),dtNewDate.GetMonth(),dtNewDate.GetDay());
				_RecordsetPtr rs = CreateRecordset("SELECT Count(ID) AS CountOfID FROM CaseHistoryT WHERE AppointmentID = %li AND SurgeryDate <> '%s'", pRes.GetReservationID(), FormatDateTimeForSql(dtDateOnly,dtoDate));
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
						//TES 1/9/2007 - PLID 23575 - Go through each record and audit.
						// (a.walling 2008-06-04 15:27) - PLID 29900 - Get the personid here
						_RecordsetPtr rsCaseHistories = CreateRecordset("SELECT ID, SurgeryDate, PersonID FROM CaseHistoryT WHERE AppointmentID = %li AND SurgeryDate <> '%s'", pRes.GetReservationID(), FormatDateTimeForSql(dtDateOnly,dtoDate));
						while(!rsCaseHistories->eof) {
							long nCaseHistoryID = AdoFldLong(rsCaseHistories, "ID");
							_variant_t varOldDate = rsCaseHistories->Fields->GetItem("SurgeryDate")->Value;
							long nCaseHistoryPatientID = AdoFldLong(rsCaseHistories, "PersonID");
							CString strOldDate;
							if(varOldDate.vt == VT_DATE) {
								strOldDate = FormatDateTimeForInterface(VarDateTime(varOldDate), NULL, dtoDate);
							}
							else {
								strOldDate = "<None>";
							}

							// (a.walling 2008-06-04 15:28) - PLID 29900 - Use the correct patient name
							ExecuteSql("UPDATE CaseHistoryT SET SurgeryDate = '%s' WHERE ID = %li",FormatDateTimeForSql(dtDateOnly,dtoDate), nCaseHistoryID);
							AuditEvent(nCaseHistoryPatientID, GetExistingPatientName(nCaseHistoryPatientID), BeginNewAuditEvent(), aeiCaseHistorySurgeryDate, nCaseHistoryID, strOldDate, FormatDateTimeForInterface(dtDateOnly,NULL,dtoDate), aepMedium, aetChanged);
							rsCaseHistories->MoveNext();
						}
					}
				}
			}

			//DRT 7/8/2005 - PLID 16664 - Void any superbill IDs which were in the list.
			if(bVoidSuperbills) {
				ExecuteSql("UPDATE PrintedSuperbillsT SET Void = 1, VoidDate = GetDate(), VoidUser = '%s' WHERE PrintedSuperbillsT.ReservationID = %li AND Void = 0", _Q(GetCurrentUserName()), pRes.GetReservationID());
			}

			// TODO: Is IncrementTimeStamp still used at all?
			//IncrementTimeStamp(RES_TIME_STAMP_NAME);

			//CClient::RefreshTable(NetUtils::AppointmentsT, pRes.GetReservationID());
			// (j.jones 2007-09-06 15:19) - PLID 27312 - required the EndTime as a parameter
			// (j.jones 2014-08-05 10:35) - PLID 63167 - this now calls the version that runs a recordset so it can get the new resource list
			CClient::RefreshAppointmentTable(nResID, nPatientID, dtNewStart, dtNewEnd, nStatus, nShowState, nLocationID, strResourceIDs);

			// Update Microsoft Outlook
			PPCAddAppt(nResID);
			// Update PalmSyncT
			UpdatePalmSyncT(nResID);

			// (z.manning 2008-07-16 14:34) - PLID 30490 - Handle any HL7 messages relating to this appointment
			// (r.gonet 12/03/2012) - PLID 54107 - Changed to use refactored send function.
			SendUpdateAppointmentHL7Message(nResID);

			AppointmentCheckPatientProvider(nPatientID, nResID);

			// If the appointment changed days, query the user about
			// other linked appointments
			if (dtOldStart != dtNewStart)
			{
				long nMinutes = GetSafeTotalMinuteDiff(dtNewStart, dtOldStart);

				//DRT 4/15/03 - See comments in CSchedulerview::CopyRes that does the same thing for
				//		some issues possible if we try the function-calling approach.
				PostMessage(NXM_RESERVATION_INVOKE_RESLINK, nResID, nMinutes);
				//AttemptAppointmentLinkAction(pRes.GetReservationID(), nMinutes);
			}


			// (d.moore 2007-10-15) - PLID 26546 - Check to see if moving the appointment may have made
			//  the new dates match other appointments or requests in the waiting list. Also, the 
			//  m_bAllowUpdate value must be released before this check can be made.
			pu.Release();
			if (!bMoveUp && !(dtNewDate == dtOldDate && dtNewStart == dtOldStart)) {		
				CheckAppointmentAgainstWaitingList (nResID, nPatientID, adwOldResources, dtNewDate, dtNewStart);
			}

			// (c.haag 2010-05-06 11:38) - PLID 38655 - Release our reference to the reservation
			// because UpdateView will disconnect it from the SingleDay.
			pRes.ReleaseAndClear();

			if (m_pParent) {
				// (c.haag 2010-08-11 12:58) - PLID 40077 - We're not dragging or resizing a reservation anymore.
				// While we do this again outside the try/catch, we want to 1. Set it to null as soon as possible
				// 2. Let UpdateView not be stopped by m_pResDragging not being null.
				m_pParent->m_pResDragging = NULL;
			}

			// (c.haag 2006-12-06 09:33) - PLID 23666 - We need to reposition all of the template blocks on
			// the screen. I will miss not having to do this as it spared us an albeit tiny speed hit
			UpdateView();

		// (c.haag 2010-05-06 11:38) - PLID 38655 - Release our reference to the reservation
		// because UpdateView will disconnect it from the SingleDay.
		// (c.haag 2010-08-11 12:58) - PLID 40077 - We're not dragging or resizing a reservation anymore
		} NxCatchAllCall("The appointment was moved, but an error occured with post-move operations", {pRes.ReleaseAndClear();if (m_pParent) {m_pParent->m_pResDragging = NULL;}UpdateView();return;})
	} NxCatchAll("An unexpected error occured while moving the appointment");

	if (m_pParent) {
		// (c.haag 2010-08-11 12:58) - PLID 40077 - We're not dragging or resizing a reservation anymore
		m_pParent->m_pResDragging = NULL;
	}
}

void CNxSchedulerDlg::OnReservationDrag(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY)
{
	if (m_pParent)
	{
		// Store what reservation we're dragging so we can tell at any given
		// time if we are dragging something.
		m_pParent->m_pResDragging = theRes;
	}
}

void CNxSchedulerDlg::OnReservationEndDrag(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY) 
{
	// (j.jones 2007-05-16 16:41) - PLID 26030 - This call to TrackLastModifiedDate
	// was formerly in OnReservationDrag, but that meant a recordset call nearly constantly
	// while you dragged an appt., when we weren't modifying data and the tracked value
	// wasn't checked until CommitResDrag. Thus, do not even try to track the modified
	// date until now at the end, just prior to CommitResDrag.

	// (v.maida 2016-04-08 13:18) - PLID 51696 - It's possible that the expected current sheet does not match the actual active sheet, in some circumstances.
	if (m_pParent && this != m_pParent->GetActiveSheet()) {
		m_pParent->m_pResDragging = NULL;
		return;
	}

	// Track the reservation so we will know if it was modified when it's
	// released
	// (c.haag 2010-05-06 11:48) - PLID 38655 - We don't need pRes beyond the call to Track...
	{
		CReservation pRes(__FUNCTION__, theRes);
		m_pParent->TrackLastModifiedDate(pRes.GetReservationID());		
	}

	CommitResDrag(theRes, m_vciColInfoOfLastMouseDownRes.GetDate(), m_vciColInfoOfLastMouseDownRes.GetResourceID());
}

void CNxSchedulerDlg::OnReservationResize(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY)
{
	if (m_pParent)
	{
		// Store what reservation we're dragging so we can tell at any given
		// time if we are dragging something.
		m_pParent->m_pResDragging = theRes;

		// Track the reservation so we will know if it was modified when it's
		// released
		CReservation pRes(__FUNCTION__, theRes);
		m_pParent->TrackLastModifiedDate(pRes.GetReservationID());
	}
}

void CNxSchedulerDlg::OnReservationEndResize(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY) 
{
	// (v.maida 2016-04-08 13:18) - PLID 51696 - It's possible that the expected current sheet does not match the actual active sheet, in some circumstances.
	if (m_pParent && this != m_pParent->GetActiveSheet()) {
		m_pParent->m_pResDragging = NULL;
		return;
	}
	CommitResDrag(theRes, m_vciColInfoOfLastMouseDownRes.GetDate(), m_vciColInfoOfLastMouseDownRes.GetResourceID());
}

void CNxSchedulerDlg::OnReservationClick(LPDISPATCH theRes) 
{
	try {
		// (v.maida 2016-04-08 13:18) - PLID 51696 - Make sure that the current sheet is equal to the active sheet
		if (m_pParent && m_ResEntry && this == m_pParent->GetActiveSheet()) {
			// (c.haag 2007-03-15 17:41) - PLID 24514 - Don't show a pop-up if the scheduler is about to refresh
			if (m_bIsWaitingForTimedUpdate)
				return;

			BeginWaitCursor();
			// (c.haag 2010-08-27 11:38) - PLID 40108 - "Reserve" the resentrydlg so that UpdateView
			// messages get ignored while the resentry object is being initialized. I also moved IsResAttendanceAppointment
			// below this call for completeness.
			CReserveResEntry rre(m_ResEntry, __FUNCTION__);
			CReservation pRes(__FUNCTION__, theRes);

			// (z.manning, 02/13/2008) - PLID 28909 - If this is an attendance reserveration, don't do anything (at least not yet)
			if(IsResAttendanceAppointment(pRes)) {
				return;
			}

			long nDay = pRes.GetDay();
			//tmpRes.ReleaseDispatch();

//#ifdef MAY_SHOW_2002
//			// We will load in existing information
//#else
//			m_ResEntry->m_ItemID = GetWorkingItemID(nDay);
//#endif

			// (c.haag 2006-12-04 17:32) - PLID 23666 - Check if we are on a template line item block.
			// If so, we need to do a special kind of zoom.
			if (-1 != pRes.GetTemplateItemID()) {
				// Get the date and resource id that correspond to the column that we put the reservation in
				COleDateTime dtWorkingDate;
				long nWorkingResourceID;
				m_pParent->GetWorkingResourceAndDate(this, pRes.GetDay(), nWorkingResourceID, dtWorkingDate);
				m_ResEntry->ZoomResNewAppointment(theRes, nWorkingResourceID, dtWorkingDate);
			}
			// If this is not a template item, it must be an actual appointment
			else {
				m_ResEntry->ZoomResExistingAppointment(theRes);
			}
			EndWaitCursor();
		}
	// (d.thompson 2009-07-07) - PLID 34803 - Extra logging to be removed when we figure out PLID 37221
	}NxCatchAllCall("Error in OnReservationClick()", Log("PLID 34803:  %s", CClient::PLID34803_GetPrintableCString()););
}

void CNxSchedulerDlg::UpdateBlocks(bool bUpdateTemplateBlocks, bool bForceUpdate)
{
	// (z.manning, 05/24/2007) - PLID 26062 - Added a 2nd overload of UpdateBlocks to determine how we should load template info.
	UpdateBlocks(bUpdateTemplateBlocks, bForceUpdate, false);
}

void CNxSchedulerDlg::UpdateBlocks(bool bUpdateTemplateBlocks, bool bForceUpdate, bool bLoadAllTemplateInfoAtOnce)
{
	if (!m_pParent) return;	
	if (!m_bAllowUpdate) return;

	// CH 3/20
	if (bForceUpdate) m_bForceBlocksUpdate = true;

	if (!IsWindowVisible() && !m_bNeedUpdate && !m_bForceBlocksUpdate) return;

	// CH 3/20
//	if (bForceUpdate) m_bForceBlocksUpdate = true;

	//DRT 7/16/2008 - PLID 30755 - CHECK_FOR_MESSAGE cannot do anything, remove it.
	//CHECK_FOR_MESSAGE(SCM_BEGIN_UPDATE, return);

	// Redraw the whole thing only if necessary
	// (d.singleton 2012-07-10 11:58) - PLID 47473 need to update if the location is -2 ( multiple locations ) as they could have changed the locations selected.
	if (m_bNeedUpdate || m_bForceBlocksUpdate || m_dtOldBlockDate != GetActiveDate() || GetLocationId() == -2) {
		int leftDay, rightDay;
		leftDay = 0;
		rightDay = m_pSingleDayCtrl.GetDayTotalCount() - 1;
		
		CWnd *pSingleDayWnd = CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd());
		CWnd *pEventWnd = (m_pEventCtrl != NULL) ? CWnd::FromHandle((HWND)m_pEventCtrl.GethWnd()) : NULL;
		
		pSingleDayWnd->SetRedraw(FALSE);
		if(pEventWnd)
			pEventWnd->SetRedraw(FALSE); // BAND-AID CODE FOR EVENT CTRL XP FIX
		try {
			// Because of the way the singleday handles blocks, 
			// Removing all the blocks and re-adding each one is just
			// as effecient as trying to re-use blocks.

			//DRT 7/16/2008 - PLID 30755 - CHECK_FOR_MESSAGE cannot do anything, remove it.
			//CHECK_FOR_MESSAGE(SCM_BEGIN_UPDATE, return);

			int i,j;
			m_pSingleDayCtrl.RemoveBlock(-1);
			// (b.cardillo 2016-06-04 19:47) - NX-100772 - Make event background same as the default office hours background color (white)
			if (pEventWnd) {
				m_pEventCtrl.RemoveBlock(-1);
			}
			CTemplateHitSet rsTemplateHits;
			for (i=0; i < m_aVisibleTemplates.GetSize(); i++) {
				VisibleTemplate* pTemplate = m_aVisibleTemplates[i];
				for (j=0; j < pTemplate->aRules.GetSize(); j++) {
					 delete pTemplate->aRules[j];
				}
				delete m_aVisibleTemplates[i];
			}
			m_aVisibleTemplates.RemoveAll();

			// (z.manning, 05/21/2007) - PLID 26062 - Instead of looking up template info once per resource,
			// let's get all the resource IDs right away and just get all the info at once if we're asked to.
			if(bLoadAllTemplateInfoAtOnce) {
				// (a.walling 2010-06-23 09:41) - PLID 39263 - Clear out the all the stored info
				m_ResourceAvailLocations.ClearAll();

				CArray<long,long> arynResourceIDs;
				COleDateTime dtWorkingDate;
				for (i = leftDay; i <= rightDay; i++) {
					long nResourceID;
					m_pParent->GetWorkingResourceAndDate(this, i, nResourceID, dtWorkingDate);
					arynResourceIDs.Add(nResourceID);
				}
				if(arynResourceIDs.GetSize() > 0) {
					//TES 6/22/2010 - PLID 39278 - Pass in the location ID to determine whether Resource Availability templates should
					// be included.
					// (a.walling 2010-06-25 12:36) - PLID 39278 - Specify that we want both
					rsTemplateHits.Requery(dtWorkingDate, arynResourceIDs, TRUE, GetLocationId(), true, true);
				}
			}

			for (i = leftDay; i <= rightDay; i++) {
				//DRT 7/16/2008 - PLID 30755 - CHECK_FOR_MESSAGE cannot do anything, remove it.
				//CHECK_FOR_MESSAGE(SCM_BEGIN_UPDATE, return);

				// Get the date corresponding to this column
				long nWorkingResourceID;
				COleDateTime dtWorkingDate;
				m_pParent->GetWorkingResourceAndDate(this, i, nWorkingResourceID, dtWorkingDate);
				// (b.cardillo 2016-06-04 19:47) - NX-100772 - Make event background same as the default office hours background color (white)
				if (pEventWnd) {
					// Color the background of the events control white.
					m_pEventCtrl.AddBlock(i, g_cdtZero, g_cdtZero + COleDateTimeSpan(0, 24, 0, 0), DEFAULT_OFFICE_HOURS_TEMPLATE_COLOR, "", VARIANT_TRUE);
				}

				// First add the office hours "template" (we used to store this in a 
				// special template, now we just read directly from the office hours 
				// for this location)
				{
					COleDateTime dtOpen, dtClose;
					if (m_pParent->m_lohOfficeHours.GetOfficeHours(dtWorkingDate.GetDayOfWeek() - 1, dtOpen, dtClose)) {
						m_pSingleDayCtrl.AddBlock(i, dtOpen, dtClose, DEFAULT_OFFICE_HOURS_TEMPLATE_COLOR, "", TRUE);
					}
				}
				
				// Then read the real templates
				// (z.manning, 05/24/2007) - PLID 26062 - Only load these if we didn't load everything already.
				if(!bLoadAllTemplateInfoAtOnce) {
					//TES 6/22/2010 - PLID 39278 - Pass in the location ID to determine whether Resource Availability templates should
					// be included.
					// (a.walling 2010-06-25 12:36) - PLID 39278 - Specify that we want both
					rsTemplateHits.Requery(dtWorkingDate, nWorkingResourceID, TRUE, GetLocationId(), true, true);
					// (a.walling 2010-06-23 09:41) - PLID 39263 - Clear out the info for this resource ID
					m_ResourceAvailLocations.ClearForDay(i);
				}

				if (!rsTemplateHits.IsBOF()) 
				{
					// (z.manning, 05/21/2007) - PLID 26062 - Since we may have loaded template info for all resources,
					// we need to go though every template hit for each resource, so move back to the beginning
					// when we start a new resource.
					rsTemplateHits.MoveFirst();

					// (c.haag 2006-12-13 14:15) - PLID 23845 - In this loop, it is possible for line items
					// to appear more than once now that we do a left join on template rules when the preference
					// for enabling block scheduling is turned on. The query sorts by priority by line item ID,
					// so we should compare the current line item ID with the previous line item ID to make sure
					// we don't repeat any blocks
					long nPrevTemplateItemID = -1;
					long nPrevTemplateRuleID = -1;
					//TES 9/3/2010 - PLID 39630 - Track the times in which templates with "Exclude Location Templates" flagged on one
					// of their rules are uppermost.
					CComplexTimeRange rangeExcludeLocTemplates;
					while (!rsTemplateHits.IsEOF()) 
					{
						//DRT 7/16/2008 - PLID 30755 - CHECK_FOR_MESSAGE cannot do anything, remove it.
						//CHECK_FOR_MESSAGE(SCM_BEGIN_UPDATE, return);

						// (z.manning, 05/21/2007) - PLID 26062 - We may have loaded info for every resource, so unless this
						// template's resource ID is the same as the one we're working with or it's all resources, then just skip it.
						if(rsTemplateHits.GetResourceID() != nWorkingResourceID && !rsTemplateHits.GetIsAllResources()) {
							rsTemplateHits.MoveNext();
							continue;
						}
						
						// (a.walling 2010-06-23 09:41) - PLID 39263 - Regardless of anything other than that, keep track of the location IDs assigned for each resource for this day.
						m_ResourceAvailLocations.HandleTemplateItem(i, rsTemplateHits);

						//TES 6/22/2010 - PLID 39278 - If this is a Resource Availability template for the location we're currently
						// filtered on, then we don't want to show it, we only show those templates in times where the resource
						// is at a different location.
						// (a.walling 2010-06-23 09:41) - PLID 39263 - Hmm....
						long nResourceAvailLocationID = rsTemplateHits.GetLocationID();
						if ( (GetLocationId() == -1 && nResourceAvailLocationID != -1) || 
							 (GetLocationId() != -1 && nResourceAvailLocationID == GetLocationId() ) ) {
							rsTemplateHits.MoveNext();
							continue;
						}
						// (d.singleton 2012-07-05 12:05) - PLID 47473 support multi location selection in location filter
						else if(GetLocationId() == -2) {
							BOOL bMatch = FALSE;
							CDWordArray dwaryLocationIDs;
							GetLocationFilterIDs(&dwaryLocationIDs);
							for(int i = 0; i < dwaryLocationIDs.GetCount(); i++) {
								if(nResourceAvailLocationID == (long)dwaryLocationIDs.GetAt(i)) {
									bMatch = TRUE;
									break;
								}
							}
							if(bMatch) {
								rsTemplateHits.MoveNext();
								continue;
							}
						}
									
						//TES 9/7/2010 - PLID 39630 - We may have a non-contiguous range for this item, so set up an array of contiguous
						// segments.
						CArray<TimeRange,TimeRange&> arTimes;
						TimeRange tr;
						tr.dtStart = rsTemplateHits.GetStartTime();
						tr.dtEnd = rsTemplateHits.GetEndTime();
						arTimes.Add(tr);

						//TES 9/7/2010 - PLID 39630 - Now, if this is a location template, take out the range that we've tracked
						// as excluding location templates (this is the part that may result in discontinuity).
						//NOTE: This code relies upon the fact that location templates are always returned last in rsTemplateHits, meaning
						// that if we hit a location template, we must necessarily have already processed all regular templates, and
						// have therefore calculated the full range that excludes location templates.
						if(nResourceAvailLocationID != -1) {
							rangeExcludeLocTemplates.RemoveFromRanges(arTimes);
						}
						
						// CAH 7/18/02: The following conditional really needs to be:
						//
						// if (!rsTemplateHits.IsOfficeHoursTemplate())
						//
						// But I didn't implement it because Bob is still in Daycago, and I.
						// don't know a better solution for the conditional.
						if (rsTemplateHits.GetPriority() != 0 || rsTemplateHits.GetText() != CString(""))
						{
							// (c.haag 2006-12-13 14:21) - PLID 23845 - If the current and previous line
							// items ID match, then we are repeating the template item. This should only
							// happen if the preference to show template blocks in the scheduler is turned
							// on. Even though the template item repeats, the template rules are unique, so
							// we need to read those in.
							if (rsTemplateHits.GetLineItemID() == nPrevTemplateItemID) {

								// Now read the rules into the visible template object
								VisibleTemplate* pvis = m_aVisibleTemplates[ m_aVisibleTemplates.GetSize() - 1 ];
								VisibleTemplateRule* pRule;
								if (nPrevTemplateRuleID != rsTemplateHits.GetRuleID()) {
									pRule = new VisibleTemplateRule;
									pRule->nRuleID = rsTemplateHits.GetRuleID();
									pRule->bAndDetails = rsTemplateHits.GetRuleAndDetails();
									pRule->bAllAppts = rsTemplateHits.GetRuleAllAppts();
									//TES 8/31/2010 - PLID 39630 - Added OverrideLocationTemplating
									pRule->bOverrideLocationTemplating = rsTemplateHits.GetRuleOverrideLocationTemplating();
									pvis->aRules.Add(pRule);
								} else {
									pRule = pvis->aRules[ pvis->aRules.GetSize() - 1 ]; 
									ASSERT(pRule);
								}
								pRule->anObjectType.Add( rsTemplateHits.GetRuleObjectType() );
								pRule->anObjectID.Add( rsTemplateHits.GetRuleObjectID() );
								nPrevTemplateRuleID = rsTemplateHits.GetRuleID();								

							} else {
								//TES 9/7/2010 - PLID 39630 - Go through each contiguous segment, and add it.
								for(int nRange = 0; nRange < arTimes.GetSize(); nRange++) {
									//TES 9/3/2010 - PLID 39630 - The previous template (if any) now has all of its rules loaded, so go 
									// through each of them and check whether any of them have Override Location Templating checked.
									if(m_aVisibleTemplates.GetSize()) {
										bool bOverrideLocTemplating = false;
										VisibleTemplate* pvisPrev = m_aVisibleTemplates[ m_aVisibleTemplates.GetSize() - 1 ];
										if(pvisPrev->nLocationID == -1) {
											for(int nPrevRule = 0; nPrevRule < pvisPrev->aRules.GetSize() && !bOverrideLocTemplating; nPrevRule++) {
											// (r.gonet 06/25/2012) - 51162 - If the previous template belongs to another column (which could be a different day or resource) then
											//  don't extend this template's override property to the current column.
												if(pvisPrev->aRules[nPrevRule]->bOverrideLocationTemplating && pvisPrev->nSingleDayColumn == i) {
													bOverrideLocTemplating = true;
												}
											}
											//TES 9/3/2010 - PLID 39630 - Now, if this template overrides location templating, then
											// add its time to our range.  If it does not, then we have to remove the time from our range,
											// because we know this template is a higher priority than any previous ones that did override.
											if(bOverrideLocTemplating) {
												rangeExcludeLocTemplates.AddRange(pvisPrev->ctrRange.GetStart(), pvisPrev->ctrRange.GetEnd());
												//TES 9/7/2010 - PLID 39630 - Tell m_ResourceAvailLocations to exclude this range as well.
												TimeRange tr;
												tr.dtStart = pvisPrev->ctrRange.GetStart();
												tr.dtEnd = pvisPrev->ctrRange.GetEnd();
												m_ResourceAvailLocations.AddExcludedRange(i, tr);
											}
											else {
												rangeExcludeLocTemplates.RemoveRange(pvisPrev->ctrRange.GetStart(), pvisPrev->ctrRange.GetEnd());
												//TES 9/7/2010 - PLID 39630 - Tell m_ResourceAvailLocations to include this range as well.
												TimeRange tr;
												tr.dtStart = pvisPrev->ctrRange.GetStart();
												tr.dtEnd = pvisPrev->ctrRange.GetEnd();
												m_ResourceAvailLocations.RemoveExcludedRange(i, tr);
											}
										}
									}

									nPrevTemplateRuleID = -1;

									// (c.haag 2007-01-09 09:15) - PLID 23666 - Only add regular templates as
									// single day blocks. Do not add the precision templating "template blocks".
									if (!rsTemplateHits.GetIsBlock()) {
										//TES 9/7/2010 - PLID 39630 - Use the times from this segment, not rsTemplateHits.
										m_pSingleDayCtrl.AddBlock(i, arTimes[nRange].dtStart, 
											arTimes[nRange].dtEnd, rsTemplateHits.GetColor(), m_pParent->m_bShowTemplateText ? _bstr_t(rsTemplateHits.GetText()) : "", TRUE);
									}

									// (c.haag 2006-05-22 11:22) - PLID 20614 - Add the times to our template list in memory
									VisibleTemplate* pvis = new VisibleTemplate;
									//TES 9/7/2010 - PLID 39630 - Use the times from this segment, not rsTemplateHits.
									COleDateTime dtStart = arTimes[nRange].dtStart;
									COleDateTime dtEnd = arTimes[nRange].dtEnd;

									//TES 9/3/2010 - PLID 39630 - VisibleTemplate now has a CComplexTimeRange, instead of dtStart and dtEnd
									dtStart.SetDateTime(dtWorkingDate.GetYear(), dtWorkingDate.GetMonth(),
										dtWorkingDate.GetDay(), dtStart.GetHour(), dtStart.GetMinute(), dtStart.GetSecond());
									dtEnd.SetDateTime(dtWorkingDate.GetYear(), dtWorkingDate.GetMonth(),
										dtWorkingDate.GetDay(), dtEnd.GetHour(), dtEnd.GetMinute(), dtEnd.GetSecond());
									pvis->ctrRange.AddRange(dtStart, dtEnd);
									pvis->nLocationID = rsTemplateHits.GetLocationID();
									pvis->nPriority = rsTemplateHits.GetPriority();
									pvis->strText = rsTemplateHits.GetText();
									pvis->clr = rsTemplateHits.GetColor();
									// (c.haag 2006-12-04 10:17) - PLID 23666 - Add template block information
									pvis->bIsBlock = rsTemplateHits.GetIsBlock();
									pvis->nTemplateItemID = rsTemplateHits.GetLineItemID();
									pvis->nSingleDayColumn = i;
									//TES 6/15/2011 - PLID 43973 - Need to set the Resource ID
									pvis->nResourceID = rsTemplateHits.GetResourceID();
									// (d.singleton 2012-01-10 16:15) - PLID 46522 - need to check for All Resources or else resource id just gets set to -1
									pvis->bAllResources = rsTemplateHits.GetIsAllResources();									
									// (c.haag 2006-12-13 15:46) - PLID 23845 - Now read in the template rules.
									// Since we only check rules for block templates, there is no need to read
									// them for non-block templates.
									//TES 9/3/2010 - PLID 39630 - We do check rules for non-block templates now, to see if they 
									// Override Location Templating
									//if (pvis->bIsBlock) {
										VisibleTemplateRule* pRule = new VisibleTemplateRule;
										pRule->nRuleID = rsTemplateHits.GetRuleID();
										pRule->bAndDetails = rsTemplateHits.GetRuleAndDetails();
										pRule->bAllAppts = rsTemplateHits.GetRuleAllAppts();
										//TES 8/31/2010 - PLID 39630 - Added OverrideLocationTemplating
										pRule->bOverrideLocationTemplating = rsTemplateHits.GetRuleOverrideLocationTemplating();
										pRule->anObjectType.Add( rsTemplateHits.GetRuleObjectType() );
										pRule->anObjectID.Add( rsTemplateHits.GetRuleObjectID() );
										nPrevTemplateRuleID = rsTemplateHits.GetRuleID();
										pvis->aRules.Add(pRule);
									//}
									nPrevTemplateItemID = rsTemplateHits.GetLineItemID();
									///////////////////////////////////////////////////////////////////////////
									m_aVisibleTemplates.Add(pvis);
								}

							}
						}
						rsTemplateHits.MoveNext();
					} // while (!rsTemplateHits.IsEOF()) {
				} // if (!rsTemplateHits.IsBOF()) {
			} // for (int i = leftDay; i <= rightDay; i++) {
		} NxCatchAll("CNxSchedulerDlg::UpdateBlocks");

		// (c.haag 2006-12-04 09:29) - PLID 23666 - Update template blocks that show as reservations
		if (bUpdateTemplateBlocks) {
			UpdateTemplateBlocks(true); 
		}

		// (a.walling 2010-06-23 09:41) - PLID 39263 - Update the display
		UpdateColumnAvailLocationsDisplay();

		pSingleDayWnd->SetRedraw(TRUE);
		if(pEventWnd)
			pEventWnd->SetRedraw(TRUE); // BAND-AID CODE FOR EVENT CTRL XP FIX
		m_pSingleDayCtrl.ShowBlock(-1);
		pSingleDayWnd->RedrawWindow();
		if(pEventWnd)
		{
			m_pEventCtrl.ShowBlock(-1);
			pEventWnd->RedrawWindow(); // BAND-AID CODE FOR EVENT CTRL XP FIX
		}
		m_dtOldBlockDate = GetActiveDate();
		m_bForceBlocksUpdate = false;
	} // if (m_bNeedUpdate || m_bForceBlocksUpdate || m_dtOldBlockDate != GetActiveDate()) {
	else {
		// (c.haag 2006-12-04 09:29) - PLID 23666 - Update template blocks that show as reservations
		if (bUpdateTemplateBlocks) {
			UpdateTemplateBlocks(true); 
		}
	}
}

void CNxSchedulerDlg::UpdateTemplateBlocks(bool bResolveRefresh)
{
	//
	// (c.haag 2006-12-04 09:34) - PLID 23666 - This function looks for all the block templates
	// in our collection and puts them on the screen one column at a time All logic in this function
	// must follow these rules:
	//
	// A - Higher level non-block template line items will always hide a block, either partially or fully, 
	//     depending on when they're scheduled
	// B - Multiple blocks are ordered in decreasing priority from left to right
	// C - If block A for a given time range is the highest priority, then any other line item in the entire 
	//     database that is a block in that time range must be loaded
	// D - Have a preference to make it so an appointment can either partially obstruct a block, or just make the
	//	   block invisible altogether if it overlaps. This does not apply to higher priority templates.
	// E - Set the proper template block appointment text color
	// F - Use CompareTimes() to do time comparisons
	//
	// To accomplish this, we do a multi-pass technique that does the following:
	//
	// 1. Build an array of unobstructed template block item times
	// 2. Build an array of template block item times obstructed only by appointments. Other blocks and
	//    priorities are ignored. This must consider rule D and F
	// 3. Build an array of template block item times obstructed by higher level templates. This must consider
	//    rule A and C.
	// 4. Sort the array by priority (rule B)
	// 5. Add the array elements to the schedule
	//
	//

	// (c.haag 2006-12-07 17:18) - PLID 23666 - If we are not using precision templating, then
	// blocks will be treated like normal templates. Therefore, we should do nothing here.
	// (z.manning, 05/24/2007) - PLID 26122 - Precision templating is not optional.

	try {
		const int nVisibleTemplates = m_aVisibleTemplates.GetSize();
		short nLeft = 0;
		short nRight = (short)m_pSingleDayCtrl.GetDayTotalCount() - 1;

		// Do for all visible days
		for (short iColumn=nLeft; iColumn <= nRight; iColumn++) {
			CArray<BlockReservation, BlockReservation> aBlockRes;
			int j;

			//
			// 1. Build an array of unobstructed template block item times
			//
			COleDateTime dtWorking = GetWorkingDate(iColumn);
			for (j=0; j < nVisibleTemplates; j++) {
				VisibleTemplate* pvis = m_aVisibleTemplates[j];
				if (pvis->bIsBlock && pvis->nSingleDayColumn == iColumn) {
					BlockReservation br;
					//TES 9/3/2010 - PLID 39630 - A VisibleTemplate may now have multiple non-contiguous ranges, so go through each one.
					CArray<TimeRange,TimeRange&> arTimes;
					pvis->ctrRange.GetRanges(arTimes);
					for(int nTime = 0; nTime < arTimes.GetSize(); nTime++) {
						br.pTemplate = pvis;
						br.dtStart.SetDateTime(
							dtWorking.GetYear(), dtWorking.GetMonth(), dtWorking.GetDay(),
							arTimes[nTime].dtStart.GetHour(), arTimes[nTime].dtStart.GetMinute(), arTimes[nTime].dtStart.GetSecond()
							);
						br.dtEnd.SetDateTime(
							dtWorking.GetYear(), dtWorking.GetMonth(), dtWorking.GetDay(),
							arTimes[nTime].dtEnd.GetHour(), arTimes[nTime].dtEnd.GetMinute(), arTimes[nTime].dtEnd.GetSecond()
							);
						aBlockRes.Add(br);
					}
				}
			}

			// (z.manning 2008-07-25 16:16) - PLID 30804 - Both of the 2 functions below were moved
			// to GlobalSchedUtils, so I added parameters to each of them to allow that.
			//
			// 2. Build an array of template block item times obstructed only by appointments. Other blocks and
			//    priorities are ignored. This must consider rule D and F
			//
			RepopulateBlockResArray_ByAppointments(aBlockRes, m_aVisibleAppts, iColumn, GetWorkingDate(iColumn), GetActiveInterval(), m_pSingleDayCtrl);

			// 3. Build an array of template block item times obstructed by higher level templates. This must consider
			//    rule A and C.
			RepopulateBlockResArray_ByTemplates(aBlockRes, m_aVisibleTemplates, iColumn, GetWorkingDate(iColumn), FALSE);

			//
			// 4. Sort the array by priority (rule B) - We don't actually need to do manual sorting. The reason
			// is that we already query for the templates in order of priority, and every array traversal and
			// population up to this point has been linear. All we have to do is follow step 5 and traverse
			// aBlockRes backwards
			//
			// 5. Add the array elements to the schedule
			//

			// (c.haag 2006-12-04 09:41) - PLID 23666 - Remove all appointments related to templates that show
			// up like appointments
			// (j.gruber 2011-02-17 14:57) - PLID 42425 - Check if we'll need to resolve and ensure we do
			BOOL bNeedsResolve = FALSE;
			ClearBlockReservations(iColumn, bNeedsResolve);
			if (bNeedsResolve) {
				bResolveRefresh = TRUE;
			}

			for (j=aBlockRes.GetSize() - 1; j >= 0; j--) {
				const BlockReservation& br = aBlockRes[j];
				CReservation pRes(m_pSingleDayCtrl.AddReservation(__FUNCTION__, iColumn, 
					br.dtStart, br.dtEnd, br.pTemplate->clr, TRUE));
				if (pRes) {
					pRes.PutReservationID(-1);
					pRes.PutTemplateItemID(br.pTemplate->nTemplateItemID);
					pRes.PutAllowResize(FALSE);
					pRes.PutAllowDrag(FALSE);
					MakeResLookLikeTemplateItem(pRes, br.pTemplate->strText, br.pTemplate->clr);
				} else {
					ThrowNxException("Failed to create reservation!");
				}
			}
		} // for (short iColumn=nLeft; iColumn <= nRight; iColumn++) {

		if (bResolveRefresh) {
			if (NULL != m_pSingleDayCtrl) {
				m_pSingleDayCtrl.Resolve();
				m_pSingleDayCtrl.Refresh();
			}
			if (NULL != m_pEventCtrl) {
				m_pEventCtrl.Resolve();
				m_pEventCtrl.Refresh();
			}
		}
	}
	NxCatchAll("Error in CNxSchedulerDlg::UpdateTemplateBlocks()");
}

// (j.gruber 2011-02-17 14:59) - PLID 42425 - added BOOL& to ensure we know when we need to resolve
void CNxSchedulerDlg::ClearBlockReservations(int iColumn, OUT BOOL &bNeedsResolve)
{
	//
	// (c.haag 2006-12-04 09:34) - PLID 23666 - This function removes all templates that appear
	// as reservations.
	//
	CReservation pRes(__FUNCTION__);
	int i = 0;

	bNeedsResolve = FALSE;
	do {
		try {
			if (NULL != (pRes = m_pSingleDayCtrl.GetReservation(__FUNCTION__, iColumn, i++))) {
				if (-1 != pRes.GetTemplateItemID()) {
					// (j.gruber 2011-02-17 15:00) - PLID 42425 - don't refresh each time
					pRes.DeleteRes(__FUNCTION__, FALSE);
					i--;
					bNeedsResolve = TRUE;
				}
			}
		} catch (...) {	// GetReservation will throw an exception if there are no reservations in the singleday.
			pRes.SetDispatch(NULL);
		}
	} while (pRes != NULL);
}

void CNxSchedulerDlg::UpdateReservations(bool bUpdateTemplateBlocks, bool bResolveAndRefresh)
{
	try {
		if (!IsWindowVisible() && !NeedUpdate()) return;

		if (!m_pParent) return;	
		if (!m_bAllowUpdate) return;
		if (m_bIsUpdatingNow) return;
		m_bIsUpdatingNow = true;

		// Get the data
		bool bGotData;
		CReservationReadSet rs;
		try {
			rs.m_pSchedView = m_pParent;
			CNxPerform nxp("ReFilter " __FUNCTION__);
			bGotData = ReFilter(rs, NeedUpdate());
		} NxCatchAllThrow("CNxSchedulerDlg::UpdateReservations:ReFilter");

		// (j.jones 2014-12-03 15:55) - PLID 64276 - clear our colored times
		m_aryColoredTimes.clear();
		ResetTimeColors(false);

		if (bGotData) {
			long i;
			
			// Init
			//TES 3/8/2011 - PLID 41519 - We need to track which appointment is currently "highlighted" before we remove them all, that way
			// we can restore it later
			long nHighlightResID = -1;
			try {
				// (d.thompson 2012-08-01) - PLID 51898 - Changed default to Off
				if(GetRemotePropertyInt("SchedHighlightLastAppt", 0, 0, GetCurrentUserName(), true)) {
					//TES 3/31/2011 - PLID 41519 - If we were given an ID to use, use it.
					if(m_nPendingHighlightID != -1) {
						nHighlightResID = m_nPendingHighlightID;
					}
					else {LPDISPATCH resHighlight = m_pSingleDayCtrl.GetHighlightRes();
						if(resHighlight) {
							//TES 3/8/2011 - PLID 41519 - This is our highlighted appointment, grab the ID.
							CReservation cresHighlight(__FUNCTION__, resHighlight);
							nHighlightResID = cresHighlight.GetReservationID();
						}
					}
				}
				//TES 3/31/2011 - PLID 41519 - Make sure we don't re-use this "pending" ID in the future.
				m_nPendingHighlightID = -1;

				m_pParent->ClearDispatchClip(this);
				m_pSingleDayCtrl.Clear(__FUNCTION__);
				// (c.haag 2006-12-12 15:31) - PLID 23845 - Clear the visible appointments array
				for (i=0; i < m_aVisibleAppts.GetSize(); i++) { delete m_aVisibleAppts[i]; }
				m_aVisibleAppts.RemoveAll(); 
				if (m_pEventCtrl) m_pEventCtrl.Clear(__FUNCTION__);
				if (m_pParent && m_pParent->m_pdlgScheduledPatients->GetSafeHwnd()) m_pParent->m_pdlgScheduledPatients->Clear();
				/////////////
				// (b.cardillo 2003-07-08 16:19) TODO: It seems this stuff should be in the UpdateView instead of the 
				// UpdateReservations, that way UpdateBlocks doesn't need its own.
				// If anyone had set this to true, it's no longer necessary (because we're updating now)
				m_bNeedUpdate = false;
				// If anyone had posted a message to update, it's no longer necessary (because we're updating now)
				MSG msg;
				// (a.walling 2010-12-17 14:49) - PLID 40444 - The modules popup makes it much easier to introduce a problem here
				// since this will also dispatch any sent messages. So I'll fix it to only deal with posted messages.
				PeekMessage(&msg, m_hWnd, NXM_UPDATEVIEW, NXM_UPDATEVIEW, PM_REMOVE | PM_QS_POSTMESSAGE | PM_NOYIELD);
				////////////////
			} NxCatchAllThrow("CNxSchedulerDlg::UpdateReservations:Init");

			// Some variables
			COleDateTime sTime, eTime;
			CString strAppts;			
			CString strMultiPurpose;
			i = 0;

			//DRT 1/5/2005 - PLID 15200 - This property was being checked (that's a data access, folks) once
			//	per every appointment on the screen!
			// (a.wilson 2012-06-14 16:18) - PLID 47966 - change for new preference type.
			// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
			long nProp_ColorApptBgWithStatus = GetRemotePropertyInt("ColorApptBgWithStatus", GetPropertyInt("ColorApptBgWithStatus", 1, 0, false), 0, GetCurrentUserName(), true);

			// (c.haag 2006-04-28 15:21) - PLID 20351 - This one, too!
			long nProp_ColorApptTextWithStatus = GetRemotePropertyInt("ColorApptTextWithStatus", GetPropertyInt("ColorApptTextWithStatus", 0, 0, false), 0, GetCurrentUserName(), true);

			// Add all appointments to the appropriate days
			for (; !rs.IsEOF(); rs.MoveNext()) {

				strMultiPurpose = GetMultiPurposeStringFromCurrentReservationReadSetPosition(rs);

				// Increment i, which gives us a true reservation count, but it appears that the only point of 
				// getting that count is so that we know if we added ANY reservations or not
				i++;
				
				// (j.luckoski 2012-07-02 09:42) - PLID 11597 - Set bool here for handling cancelled appt cases below
				bool bIsResCancelled = false;
				// (a.walling 2013-01-21 16:48) - PLID 54744 - Available in ReadSet
				if (rs.GetCancelledDate().GetStatus() == COleDateTime::valid) {
					bIsResCancelled = true;
				}

				// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
				bool bIsMixRuleOverridden = rs.GetIsMixRuleOverridden();

				// b.cardillo (2002-06-05 17:41)
				// If the user is viewing the scheduled patient list, make sure there's an entry for this patient
				try {
					if (m_pParent && m_pParent->m_pdlgScheduledPatients->GetSafeHwnd() && rs.GetPersonID() != -25) {
						switch (rs.GetNoShow()) {
						case 0: // Pending appointment
							// Add patient
							m_pParent->m_pdlgScheduledPatients->AddAppointment(rs.GetID(), rs.GetStartTime(), rs.GetPersonID(), rs.GetPatientName(), FALSE);
							break;
						case 1: // In
							// Add patient as "in"
							m_pParent->m_pdlgScheduledPatients->AddAppointment(rs.GetID(), rs.GetStartTime(), rs.GetPersonID(), rs.GetPatientName(), TRUE);
							break;
						case 2: // Out
						case 3: // No Show
							// Do nothing
							break;
						}
					}
				} NxCatchAllThrow("CNxSchedulerDlg::UpdateReservations:AddToScheduledPatientsList");

				// CAH 4/17
				// If this is an event, add it to the event control
				if (rs.GetStartTime() == rs.GetEndTime() && rs.GetStartTime().GetHour() == 0 && rs.GetStartTime().GetMinute() == 0)
				{
					if (m_pEventCtrl) {
						try {
							// Add the appointment at the right time while retrieving a pointer to it
							CReservation pRes(m_pEventCtrl.AddReservation(__FUNCTION__, GetOffsetDay(rs), 
								rs.GetStartTime(), rs.GetEndTime(), rs.GetColor(), TRUE));
							
							if (pRes) {
								// Set the appropriate text for the appointment
								// (j.luckoski 2012-05-07 11:23) - PLID 11597 - If cancelled display CANCELLED
								// (j.luckoski 2012-06-20 11:10) - PLID 11597 - Utilize two different GetBoxText functions that take
								// a value for cancelled and uncancelled appts.
								// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
								pRes.PutText((_bstr_t)rs.GetBoxText(strMultiPurpose, nProp_ColorApptTextWithStatus, bIsResCancelled, bIsMixRuleOverridden));
								
								if(!bIsResCancelled) {
									// (j.luckoski 2012-06-20 14:45) - PLID 11597 - Return putallowdrag back to true
									pRes.PutAllowDrag(TRUE);
									pRes.PutAllowResize(TRUE);
								} else {
									//j.luckoski 2012-06-20 11:09) - PLID 11597 - Make all cancelled appts immune from drag and resize
									pRes.PutAllowDrag(VARIANT_FALSE);
									pRes.PutAllowResize(VARIANT_FALSE);
								}

								// (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
								if (rs.GetCancelledDate().GetStatus() == COleDateTime::valid) {
									pRes.Data["CancelledDate"] = _variant_t(rs.GetCancelledDate(), VT_DATE);
								} else {									
									pRes.Data["CancelledDate"] = g_cvarNull;
								}
					
								// Set the appropriate ID for the appointment
								pRes.PutReservationID(rs.GetID());
								// (c.haag 2009-12-21 17:21) - PLID 36691 - Assign the appointment type and patient ID
								pRes.PutAptTypeID(rs.GetAptTypeID());
								pRes.PutPersonID(rs.GetPersonID());
								// (c.haag 2003-07-30 10:51) - Color the reservation background
								// (j.luckoski 2012-06-20 11:12) - PLID 11597 - If cancelled, use cancelled color
								COLORREF clrBkg = DEFAULT_APPT_BACKGROUND_COLOR;
								if (bIsResCancelled) {
									clrBkg = m_nCancelColor;
								}
								else if (nProp_ColorApptBgWithStatus) {
									// (j.jones 2014-12-04 13:17) - PLID 64119 - added GetStatusOrMixRuleColor, which will return the mix rule color
									// if one exists and the appt. is pending, otherwise it returns the status color
									clrBkg = rs.GetStatusOrMixRuleColor();
								}
								else {
									clrBkg = rs.GetColor();
								}

								ColorReservationBackground(pRes, clrBkg);
							}
						} NxCatchAllThrow("CNxSchedulerDlg::UpdateReservations:AddEvent");
					}
				}
				else {
					try {
						// Add the appointment at the right time while retrieving a pointer to it
						CReservation pRes(m_pSingleDayCtrl.AddReservation(__FUNCTION__, GetOffsetDay(rs), 
							rs.GetStartTime(), rs.GetEndTime(), rs.GetColor(), TRUE));
						if (pRes) {
							// (j.luckoski 2012-05-07 11:23) - PLID 11597 - If cancelled display CANCELLED
							// Set the appropriate text for the appointment
							// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
							pRes.PutText((_bstr_t)rs.GetBoxText(strMultiPurpose, nProp_ColorApptTextWithStatus, bIsResCancelled, bIsMixRuleOverridden));
							if(!bIsResCancelled) {
								// (j.luckoski 2012-06-20 14:45) - PLID 11597 - Return putallowdrag back to true
								pRes.PutAllowDrag(TRUE);
								pRes.PutAllowResize(TRUE);
							} else {
								// (j.luckoski 2012-06-20 11:09) - PLID 11597 - Make all cancelled appts immune from drag and resize
								pRes.PutAllowDrag(VARIANT_FALSE);
								pRes.PutAllowResize(VARIANT_FALSE);
							}

							// (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
							if (rs.GetCancelledDate().GetStatus() == COleDateTime::valid) {
								pRes.Data["CancelledDate"] = _variant_t(rs.GetCancelledDate(), VT_DATE);
							} else {									
								pRes.Data["CancelledDate"] = g_cvarNull;
							}

							// Set the appropriate ID for the appointment
							pRes.PutReservationID(rs.GetID());
							// (c.haag 2009-12-21 17:21) - PLID 36691 - Assign the appointment type and patient ID
							pRes.PutAptTypeID(rs.GetAptTypeID());
							pRes.PutPersonID(rs.GetPersonID());
							// (c.haag 2003-07-30 10:51) - Color the reservation background
							// (j.luckoski 2012-06-20 11:12) - PLID 11597 - If cancelled, use cancelled color
							COLORREF clrBkg = DEFAULT_APPT_BACKGROUND_COLOR;
							if (bIsResCancelled) {
								clrBkg = m_nCancelColor;
							}
							else if (nProp_ColorApptBgWithStatus) {
								// (j.jones 2014-12-04 13:17) - PLID 64119 - added GetStatusOrMixRuleColor, which will return the mix rule color
								// if one exists and the appt. is pending, otherwise it returns the status color
								clrBkg = rs.GetStatusOrMixRuleColor();
							}
							else {
								clrBkg = rs.GetColor();
							}
							ColorReservationBackground(pRes, clrBkg);

							// (c.haag 2006-12-12 15:32) - PLID 23845 - Update the visible appointments array
							// if we are using precision templating
							// (z.manning, 05/24/2007) - PLID 26122 - Precision templating is not optional.
							{
								// (j.luckoski 2012-05-07 11:24) - PLID 11597 - If not cancelled, add to visible appts for percision templates.
								// (j.luckoski 2012-05-09 14:42) - PLID 50264 - Use ReturnsRecordsParam
								if(!bIsResCancelled) {
									VisibleAppointment* va = new VisibleAppointment;
									va->nReservationID = rs.GetID();
									va->nAptTypeID = rs.GetAptTypeID();
									LoadResourceIDStringIntoArray(rs.GetResourceIDString(), va->anResourceIDs);
									LoadPurposeIDStringIntoArray(rs.GetPurposeIDString(), va->anPurposeIDs);
									va->dtStart = rs.GetStartTime();
									va->dtEnd = rs.GetEndTime();
									va->nOffsetDay = GetOffsetDay(rs);
									m_aVisibleAppts.Add(va);
								}
							}

							//TES 3/8/2011 - PLID 41519 - If this is the same appointment that was previously "highlighted", highlight it.
							if(rs.GetID() == nHighlightResID) {
								m_pSingleDayCtrl.SetHighlightRes(pRes.GetDispatch());
							}

							// (j.jones 2014-12-03 15:55) - PLID 64276 - if this appt. overrode a mix rule,
							// color the time yellow
							if (bIsMixRuleOverridden) {						
								if (m_pSingleDayCtrl) {
									//cache that this time needs colored
									ColoredTime ct(rs.GetStartTime(), rs.GetEndTime(), RGB(255, 253, 170));
									m_aryColoredTimes.push_back(ct);

									//now color that time slot
									m_pSingleDayCtrl.SetTimeButtonColors(ct.dtStart, ct.dtEnd, ct.color);
								}
							}
						}	
					} NxCatchAllThrow("CNxSchedulerDlg::UpdateReservations:AddAppointment");
				}
			}

			rs.Close();
						
			// Update labels
			try {
				m_OldDate = GetActiveDate();
				// (a.walling 2010-06-23 18:00) - PLID 39263 - Now a virtual function
				UpdateActiveDateLabel();
				//(z.manning, PLID 15892, 06/30/05)
				//The resource view in the scheduler now shows the name of the current view.
				// (c.haag 2006-04-12 13:01) - PLID 20104 - I made it faster because I'm that cool.
				/*long nLastLoadedResourceViewID = m_pParent->GetLastLoadedResourceViewID(this);
				_RecordsetPtr prsViews = CreateRecordset("SELECT Name FROM ResourceViewsT WHERE ID=%li", nLastLoadedResourceViewID);
				FieldsPtr flds = prsViews->Fields;
				CString strViewName;
				if(prsViews->eof) {
					strViewName = "Standard View";
				}
				else {
					strViewName = "View: " + AdoFldString(flds, "Name");
				}
				SetDlgItemText(IDC_CURRENT_VIEW_NAME, strViewName);
				prsViews->Close();*/
				SetDlgItemText(IDC_CURRENT_VIEW_NAME, m_pParent->GetLastLoadedResourceViewName(this));
				EnsureCountLabelText();
				EnsureButtonLabels();
			} NxCatchAllThrow("CNxSchedulerDlg::UpdateReservations:UpdatingLabels");
		}

		// (z.manning, 02/13/2008) - PLID 28909 - Handle attendance reserverations.
		// This needs to be called after we update the buttons' label text or else we may
		// get exceptions when calling GetWorkingResourceAndDate.
		if(IsNexTechInternal()) {
			UpdateAttendanceReservations();
		}

		// (c.haag 2006-12-04 09:29) - PLID 23666 - Update template blocks that show as reservations.
		// Pass in false as the parameter because we already do a resolve and refresh afterwards
		if (bUpdateTemplateBlocks) {
			UpdateTemplateBlocks(false);
		}

		// (c.haag 2007-01-04 10:13) - PLID 23666 - Only resolve and refresh when necessary because
		// we might be calling it again later on in UpdateBlocks when block templates are updated
		if (bResolveAndRefresh) {
			try {
				m_pSingleDayCtrl.Resolve();
				if (m_pEventCtrl) m_pEventCtrl.Resolve();
			} NxCatchAllThrow("CNxSchedulerDlg::UpdateReservations:Resolve");

			// Refresh always
			try {
				m_pSingleDayCtrl.Refresh();
				if (m_pEventCtrl) m_pEventCtrl.Refresh();
			} NxCatchAllThrow("CNxSchedulerDlg::UpdateReservations:Refresh");
		}

		// (j.jones 2015-01-08 09:54) - PLID 64276 - recolor the times on the new schedule view,
		// it is possible the blocks have been shifted due to new hours added outside of
		// the initial office hours
		ResetTimeColors(true);

		m_bIsUpdatingNow = false;
	} NxCatchAllCallThrow("CNxSchedulerDlg::UpdateReservations:Outside", {
		m_bIsUpdatingNow = false; 
	});
}

void CNxSchedulerDlg::StoreDetails()
{
	// Remember certain properties to set after tab change
	ASSERT(m_pParent); // m_pParent needs to be set to a pointer to the containing tab view
	if (m_pParent) {
		m_pParent->m_nTopSlot = m_pSingleDayCtrl.GetTopSlot();
		m_pParent->m_nInterval = GetActiveInterval();
		m_pParent->m_nPurposeSetId = GetPurposeSetId();
		//TES 6/21/2010 - PLID 21341 - Remember the location filter (not sure why the purpose filter isn't remembered here).
		m_pParent->m_nLocationId = GetLocationId();
		// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
		m_pParent->m_strAMPMBtn = m_nxbtnAmPmButton.GetText();
	}

	//TES 3/8/2011 - PLID 41519 - When switching tabs, clear out the "highlighted" appointment
	ClearHighlightedReservation();
}

void CNxSchedulerDlg::RecallDetails()
{
	m_bAllowUpdate = false;
	m_DateSelCombo.SetValue(m_pParent->GetPivotDate());
	SetActiveInterval(m_pParent->GetInterval());
	m_pSingleDayCtrl.PutTopSlot(m_pParent->GetTopSlot());
	// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
	m_nxbtnAmPmButton.SetText(m_pParent->m_strAMPMBtn);
	SetActivePurposeSetId(m_pParent->m_nPurposeSetId);
	m_pParent->RequeryAllPurposeFilters();	//DRT 8/5/03 - If we just set the purpose above, we need to requery the purposes appropriately
	SetActivePurposeId(m_pParent->m_nPurposeId);
	//TES 6/21/2010 - PLID 21341 - Set the Location ID.
	SetActiveLocationId(m_pParent->m_nLocationId);

	//TES 12/12/2014 - PLID 64120 - Only show the View Rules button if there are any rules in the database
	//TES 2/11/2015 - PLID 64120 - Moved this here from OnInitDialog(), we now need to check every time we bring up the dialog
	if (DoesDatabaseHaveMixRules())
	{
		m_btnViewMixRules.ShowWindow(SW_SHOWNA);
	}
	else {
		m_btnViewMixRules.ShowWindow(SW_HIDE);
	}

	m_bAllowUpdate = true;
	m_bNeedUpdate = true;
	// (c.haag 2003-10-15 12:36) - This should be called by another function
	// after we leave this one.
	//UpdateView();
}

// This is a utility function, it simply asks the given tab to fill in its resource list (however it 
// wants to present that to the user).  Anyone who calls this function has to guarantee that the data
// parts of the given tab are refreshed after this function is called.  For that reason, this function
// should always be sure that the data parts are cleared out and disabled, to guarantee the said 
// responsibility is met.
void CNxSchedulerDlg::Internal_ReflectCurrentResourceListOnGUI()
{
	try {
		ASSERT(m_dlResources != NULL);

		m_dlResources->Clear();
		
		const CResourceEntryPtrArray &arypResourceList = m_pParent->GetCurrentResourceList();
		long nCount = arypResourceList.GetSize();
		for (long i=0; i<nCount; i++) {
			// Get the resource entry
			const CResourceEntry *pre = arypResourceList.GetAt(i);
			// Generate the row
			IRowSettingsPtr pRow = m_dlResources->GetRow(sriGetNewRow);
			pRow->PutValue(ResourceComboColumns::ID, pre->m_nResourceID);
			pRow->PutValue(ResourceComboColumns::Relevence, nCount - i);
			pRow->PutValue(ResourceComboColumns::Name, (LPCTSTR)pre->m_strResourceName);
			_variant_t varNull;
			varNull.vt = VT_NULL;
			pRow->PutValue(ResourceComboColumns::DefaultInterval, pre->m_nDefaultInterval == -1 ? varNull : pre->m_nDefaultInterval);
			// Add the newly generated row
			m_dlResources->AddRow(pRow);
		}

		// We expect that whoever called this function will take care of telling us to reflect the 
		// active resource id, and only when we know the right resource id will we have enough 
		// information to update the view.  In the mean time we need to clear and disable the view.
		m_dlResources->PutCurSel(sriNoRow);
		ClearAndDisable();
		InvalidateData();

	} catch (...) {
		// If there were any exceptions whatsoever, we need to clear and disable this view
		ClearAndDisable();
		// And re-throw the exception
		throw;
	}
}

void CNxSchedulerDlg::Internal_ReflectActiveResourceOnGUI()
{
	try {
		ASSERT(m_dlResources != NULL);
		ASSERT(m_dlInterval != NULL);

		// Find the resource we're going to set the selection to
		long nNewResourceID = m_pParent->GetActiveResourceID();
		if (nNewResourceID != CSchedulerView::sdrUnknownResource) {
			// Select that row
			m_dlResources->SetSelByColumn(ResourceComboColumns::ID, nNewResourceID);

			// Now there better be something is selected; if there isn't then the CSchedulerView 
			// data is screwed up so throw an exception
			if (m_dlResources->GetCurSel() != sriNoRow) {
				//TES 2004-01-30: If this resource has a default interval, set it.
				if(m_dlResources->GetValue(m_dlResources->CurSel, ResourceComboColumns::DefaultInterval).vt == VT_I4) {
					// (b.cardillo 2016-05-13 14:22) - NX-100239 - Eliminate the 40-minute interval option
					int nSel = m_dlInterval->SetSelByColumn(0, VarLong(m_dlResources->GetValue(m_dlResources->CurSel, ResourceComboColumns::DefaultInterval)));
					// If we were unable to set the dropdown to the preferred interval, probably because whatever they had it 
					// set to in data doesn't exist as an option, then revert to the 15-minute default, which certainly exists.
					if (nSel == NXDATALISTLib::sriNoRow) {
						m_dlInterval->SetSelByColumn(0, 15);
					}
					if(this == m_pParent->GetActiveSheet())
						SelChangeIntervalCombo(m_dlInterval->CurSel);
				}
				// Sweet, we're done so update the view now that we know the active resource id
				InvalidateData();
			} else {
				// Nothing selected, which means we must have failed to set it in the above loop
				ThrowNxException(
					"CNxSchedulerDlg::Internal_ReflectActiveResourceOnGUI: The currently selected "
					"resource id %li is not in the list of available resources!", 
					nNewResourceID);
			}
		} else {
			// The current resource is unknown, that's not acceptable; the CSchedulerView data is 
			// somehow messed up.
			m_dlResources->PutCurSel(sriNoRow);
			ClearAndDisable();
			//ThrowNxException("CNxSchedulerDlg::Internal_ReflectActiveResourceOnGUI: There is no known active resource id!");
		}
	} catch (...) {
		// If there were any exceptions whatsoever, we need to clear and disable this view
		ClearAndDisable();
		// And re-throw the exception
		throw;
	}
}

void CNxSchedulerDlg::ClearAndDisable()
{
	if (GetSafeHwnd()) {
		try {
			m_pEventCtrl.PutEnabled(VARIANT_FALSE);
			m_pEventCtrl.Clear(__FUNCTION__);
			m_pEventCtrl.RemoveBlock(-1);
			m_pSingleDayCtrl.PutEnabled(VARIANT_FALSE);
			m_pSingleDayCtrl.Clear(__FUNCTION__);
			m_pSingleDayCtrl.RemoveBlock(-1);
		} NxCatchAllCall("CNxSchedulerDlg::ClearAndDisable", {
			// Well this is insanity because what I'd like to do on failure is clear 
			// everything, but the act of clearing is what failed!  Looks like we'll 
			// have to take drastic measures: we're going to destroy the current 
			// sheet.  If the derived classes want to handle this a little more 
			// gracefully since they know the IDCs of the singledays, they should 
			// feel free.
			DestroyWindow();
			if (m_pParent) m_pParent->Invalidate();
		});
	}
}

void CNxSchedulerDlg::OnChangeResourceViewMenuCommand(WPARAM wParam)
{
	try {
		// We'll use this variable to put the id the user wants to change to
		long nNewResourceViewID = CSchedulerView::srvUnknownResourceView;
		long nNewResourceViewLocationID = -2;
		if (wParam == IDM_RV_EDIT_RESOURCES) {
			// Special menu item to edit the resources
			CResourceOrderDlg dlgResourceOrder(this);
			dlgResourceOrder.m_nCurResourceViewID = m_pParent->GetLastLoadedResourceViewID(this);
			if (dlgResourceOrder.DoModal() == IDOK) {
				// See what was selected after dismissing the edit window
				nNewResourceViewID = dlgResourceOrder.m_nCurResourceViewID;
				//TES 7/27/2010 - PLID 39445 - Also pull the default location from the dialog.
				nNewResourceViewLocationID = dlgResourceOrder.m_nCurLocationID;
			}
		} else /* if (wParam >= IDM_RV_CHANGE_VIEWS && wParam <= (IDM_RV_CHANGE_VIEWS + MAX_VIEW_COUNT-1)) */ { 
			ASSERT(wParam >= IDM_RV_CHANGE_VIEWS && wParam <= (IDM_RV_CHANGE_VIEWS + MAX_VIEW_COUNT-1));
			// The user clicked one of the menu items to simply switch to a certain view, just figure out which one
			int nMenuPosition = wParam-IDM_RV_CHANGE_VIEWS;
			ASSERT(m_aryViewsByMenuPosition.GetSize() > nMenuPosition);
			if (m_aryViewsByMenuPosition.GetSize() > nMenuPosition) {
				// We know the view based on its menu position
				nNewResourceViewID = m_aryViewsByMenuPosition[nMenuPosition];
				ASSERT(m_aryViewLocationsByMenuPosition.GetSize() > nMenuPosition);
				//TES 7/26/2010 - PLID 39445 - Also get the default location for this view.
				nNewResourceViewLocationID = m_aryViewLocationsByMenuPosition[nMenuPosition];
			}
		}
		// Now if we've correctly determined the view the user wants to switch to, then switch to it
		if (nNewResourceViewID != CSchedulerView::srvUnknownResourceView && AllowChangeView()) {
			//TES 7/26/2010 - PLID 39445 - Pass in the default location
			m_pParent->SetCurrentResourceViewID(nNewResourceViewID, nNewResourceViewLocationID);
		}
	} NxCatchAllCall("CNxSchedulerDlg::OnChangeResourceViewMenuCommand", ClearAndDisable());
}

LRESULT CNxSchedulerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		// (a.walling 2008-09-18 17:09) - PLID 28040 - This is no longer used
		/*
	case SCM_BEGIN_UPDATE:
		FastOptionUpdateView();
		return 0;
		break;
		*/
	case WM_COMMAND:
		// See if the command was one of the "resource view" menus
		if (wParam == IDM_RV_EDIT_RESOURCES || (wParam >= IDM_RV_CHANGE_VIEWS && wParam <= (IDM_RV_CHANGE_VIEWS + MAX_VIEW_COUNT-1))) {
			// Call our handler for this
			OnChangeResourceViewMenuCommand(wParam);
			return 0;
		}
		break;
	case NXM_RESERVATION_DELETE:
		try {
			// No one should send this message unless lParam is non-null
			if (lParam != NULL)
			{
				// Get the object (this increments the reference count
				CReservation pRes("NXM_RESERVATION_DELETE", (LPDISPATCH)lParam);

				// (a.walling 2007-08-31 12:57) - PLID 27265 - If there is a popup menu for this Reservation,
				// close the popup menu to prevent the user from doing anything to a Reservation that is no
				// longer valid. I think closing the popup menu is good enough, since a messagebox would interrupt.
				if (m_pParent && AreCOMObjectsEqual(pRes.GetDispatch(), m_pParent->m_pResDispatchPtr.GetDispatch())) {
					// cancel the popup menu!
					m_pParent->SendMessage(WM_CANCELMODE, 0, 0);
				}
				
				// Since one extra reference was added to guarantee the object would survive 
				// between the PostMessage and this WindowProc, we need to decrement the reference 
				// count by 1 extra (beyond the normal decrement that will happen when our smart-
				// pointer goes out of scope)
				pRes.Release();

				// Now do the Delete (which was the whole point of this message)
				pRes.DeleteRes("NXM_RESERVATION_DELETE");

				//We now have one fewer appointments on screen than before.
				EnsureCountLabelText();
			}
			return 0;
		} NxCatchAll("CNxSchedulerDlg::WindowProc:NXM_RESERVATION_DELETE");
		break;
	case NXM_RESERVATION_INVOKE_RESLINK:
		{
			AttemptAppointmentLinkAction(wParam, lParam);
		}
		break;
	case NXM_UPDATEVIEW:
		{
			// (v.maida 2016-04-08 13:18) - PLID 51696 - if the current sheet is not equal to the active one, then an exception will eventually be thrown 
			// in GetWorkingResourceAndDate(), so don't proceed if that particular scenario occurs. 
			// This can occur if this message is belatedly processed for a tab that is no longer the active tab.
			if (m_pParent && this == m_pParent->GetActiveSheet()) {
				UpdateView();
			}
		}
		break;

	default:
		break;
	}
	return CNxDialog::WindowProc(message, wParam, lParam);
}

// (c.haag 2010-02-04 12:17) - PLID 37221 - Now uses CReservation instead of IReservation*
bool IsPatientAppointment(CReservation pRes)
{
	try {
		if (NULL != pRes) {
			// Get the PatientID of the ReservationID
			_RecordsetPtr prs = CreateRecordset(
				"SELECT PatientID FROM AppointmentsT WHERE ID = %li", pRes.GetReservationID());
			// Check that the reservation is valid
			if (! prs->eof) {
				// Check to see that it is a valid patient
				if (AdoFldLong(prs, "PatientID") > 0) {
					return true;
				}
			}
		}
		
		return false;
	} NxCatchAllCall("IsPatientAppointment", return false);
}

long CNxSchedulerDlg::GetPurposeSetId()
{
	if (m_dlTypeFilter == NULL) return -1;
	return m_dlTypeFilter->GetValue(m_dlTypeFilter->GetCurSel(), 0).lVal;
}

long CNxSchedulerDlg::GetPurposeId()
{
	if (m_dlPurposeFilter == NULL) return -1;
	return m_dlPurposeFilter->GetValue(m_dlPurposeFilter->GetCurSel(), 0).lVal;
}

//TES 6/21/2010 - PLID 21341
long CNxSchedulerDlg::GetLocationId()
{
	if (m_dlLocationFilter == NULL) return -1;
	return VarLong(m_dlLocationFilter->GetValue(m_dlLocationFilter->GetCurSel(), 0),-1);
}

void CNxSchedulerDlg::OnSelChosenPurposeSetFilter(long nNewSel)
{
	try {
		if (m_pParent) {
			if(!AllowChangeView()) {
				m_dlTypeFilter->SetSelByColumn(0,m_nActivePurposeSetId);
				return;
			}
			// (v.maida 2016-04-22 16:27) - PLID 51696 - Don't continue if we're not on the correct tab.
			if (this != m_pParent->GetActiveSheet()) {
				return;
			}
			// Get the ID of the type to be filtered on
			long nNewId;
			if (nNewSel != sriNoRow) {
				// Get the ID out of the datalist
				nNewId = VarLong(m_dlTypeFilter->GetValue(nNewSel, 0));
			} else {
				// Nothing selected, try to put it back to the "All" selection
				m_dlTypeFilter->SetSelByColumn(0, (long)-1);
				// The "all" selection is -1 id
				nNewId = -1;
			}
			if(nNewId == m_nActivePurposeSetId) {
				// (z.manning, 11/25/05, PLID 18454)
				// If the selection is {Multiple Types} then we don't want to return, but rather bring up
				// the multiple selection dialog.
				if(nNewId != -2) {
					//TES 3/16/2004: Let's not refresh if we don't need to, both because of performance and also because the resentry
					//may be pinned on screen.
					return;
				}
			}
			// Now we are guaranteed to have the correct id, so set it and refresh
			if(nNewId == -2) {
				//multiple types
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(this, "AptTypeT");

				//preselect existing stuff
				CString str = m_strMultiTypeIds;
				long nComma = str.Find(",");
				while(nComma > 0) {
					dlg.PreSelect(atoi(str.Left(nComma)));
					str = str.Right(str.GetLength() - (nComma + 1));

					nComma = str.Find(",");
				}

				//DRT 10/6/03 - PLID 9415 - Really we should be looking at what is currently selected.  Then we pop up the box, and 
				//		if they cancel it, we should reset to what was previously there.  If they hit 'OK', then we proceed on.
				long nOldTypeRow = m_dlTypeFilter->GetCurSel();
				if(dlg.Open("AptTypeT", "", "ID", "Name", "Select types") == IDCANCEL) {
					//if they cancelled, we want to leave the setting as it was, so don't do anything and just quit
					m_dlTypeFilter->SetSelByColumn(0, (long)m_pParent->m_nPurposeSetId);
					return;
				}
				else {
					CString strOut = dlg.GetMultiSelectIDString();
					if(strOut.IsEmpty()) {
						MsgBox("You cannot filter on no types.  The filter will be reset to { Show All Types }");
						m_dlTypeFilter->SetSelByColumn(0, (long)-1);
						nNewId = -1;	//just change it here, and let us continue on
					}

					//we like commas better than spaces
					m_strMultiTypeIds = strOut + ",";
					m_strMultiTypeIds.Replace(" ", ",");
				}	m_pParent->SetAllMultiTypeStrings(m_strMultiTypeIds);
			}

			SetActivePurposeSetId(nNewId);
			m_pParent->m_nPurposeSetId = nNewId;
			SetActivePurposeId(-1);
			m_pParent->RequeryAllPurposeFilters();
			UpdateView();
		}
	} NxCatchAll("CNxSchedulerDlg::OnSelChosenPurposeSetFilter");
}

void CNxSchedulerDlg::OnSelChosenPurposeFilter(long nNewSel)
{
	try {
		if (m_pParent) {
			if(!AllowChangeView()) {
				m_dlPurposeFilter->SetSelByColumn(0,m_nActivePurposeId);
				return;
			}
			// (v.maida 2016-04-22 16:27) - PLID 51696 - Don't continue if we're not on the correct tab.
			if (this != m_pParent->GetActiveSheet()) {
				return;
			}
			// Get the ID of the purpose to be filtered on
			long nNewId;
			if (nNewSel != sriNoRow) {
				// Get the id out of the datalist
				nNewId = VarLong(m_dlPurposeFilter->GetValue(nNewSel, 0));
			} else {
				// Nothing selected, try to put it back to the "All" selection
				m_dlPurposeFilter->SetSelByColumn(0, (long)-1);
				// The "all" selection is -1 id
				nNewId = -1;
			}
			// Now we are guaranteed to have the correct id, so set it and refresh
			if(nNewId == -2) {
				//multiple purposes
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(this, "AptPurposeT");

				//preselect existing stuff
				CString str = m_strMultiPurposeIds;
				long nComma = str.Find(",");
				while(nComma > 0) {
					dlg.PreSelect(atoi(str.Left(nComma)));
					str = str.Right(str.GetLength() - (nComma + 1));

					nComma = str.Find(",");
				}

				//DRT 10/6/03 - PLID 9415 - Really we should be looking at what is currently selected.  Then we pop up the box, and 
				//		if they cancel it, we should reset to what was previously there.  If they hit 'OK', then we proceed on.
				// (c.haag 2008-12-18 11:10) - PLID 32376 - Filter out inactive procedures
				if(dlg.Open("AptPurposeT", "AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1)", "ID", "Name", "Select purposes") == IDCANCEL) {
					//if they cancelled, we want to leave the setting as it was, so don't do anything and just quit
					m_dlPurposeFilter->SetSelByColumn(0, (long)m_pParent->m_nPurposeId);
					return;
				}
				else {
					CString strOut = dlg.GetMultiSelectIDString();
					if(strOut.IsEmpty()) {
						MsgBox("You cannot filter on no purposes.  The filter will be reset to { Show All Purposes }");
						m_dlPurposeFilter->SetSelByColumn(0, (long)-1);
						nNewId = -1;	//just change it here, and let us continue on
					}

					m_strMultiPurposeIds = strOut + ",";
					m_strMultiPurposeIds.Replace(" ", ",");
					m_pParent->SetAllMultiPurposeStrings(m_strMultiPurposeIds);
				}
			}

			SetActivePurposeId(nNewId);
			m_pParent->m_nPurposeId = nNewId;
			m_pParent->RequeryAllPurposeFilters();
			UpdateView();
		}
	} NxCatchAll("CNxSchedulerDlg::OnSelChosenPurposeFilter");
}

void CNxSchedulerDlg::OnSelChosenLocationFilter(long nNewSel)
{
	try {
		//TES 6/21/2010 - PLID 21341 - Added, copied from OnSelChosenTypeFilter() (though this one doesn't allow multiple).
		if (m_pParent) {
			if(!AllowChangeView()) {
				m_dlLocationFilter->SetSelByColumn(0,m_nActiveLocationId);
				return;
			}
			// (v.maida 2016-04-22 16:27) - PLID 51696 - Don't continue if we're not on the correct tab.
			if (this != m_pParent->GetActiveSheet()) {
				return;
			}
			// Get the ID of the type to be filtered on
			long nNewId;
			if (nNewSel != sriNoRow) {
				// Get the ID out of the datalist
				nNewId = VarLong(m_dlLocationFilter->GetValue(nNewSel, 0));
			} else {
				// Nothing selected, try to put it back to the "All" selection
				m_dlLocationFilter->SetSelByColumn(0, (long)-1);
				// The "all" selection is -1 id
				nNewId = -1;
			}
			if(nNewId == m_nActiveLocationId && nNewId != -2) {
				//TES 3/16/2004: Let's not refresh if we don't need to, both because of performance and also because the resentry
				// (d.singleton 2012-06-07 12:20) - PLID 47473 we do need to pop up the multiselect list if it is already selected
				//may be pinned on screen.
				return;
			}
			// (d.singleton 2012-06-07 11:29) - PLID 47473 they selected mutliple
			if(nNewId == -2) {
				CMultiSelectDlg dlg(this, "LocationsT");

				//get pre select list
				CString str = m_strMultiLocationIds;
				long nComma = str.Find(",");
				while(nComma > 0) {
					dlg.PreSelect(atoi(str.Left(nComma)));
					str = str.Right(str.GetLength() - (nComma + 1));

					nComma = str.Find(",");
				}

				if(dlg.Open("LocationsT", "TypeID = 1 AND Active = 1", "ID", "Name", "Select Locations") == IDCANCEL) {
					//if they cancelled, we want to leave the setting as it was, so don't do anything and just quit
					m_dlLocationFilter->SetSelByColumn(0, (long)m_pParent->m_nLocationId);
					return;
				}
				else {
					CString strOut = dlg.GetMultiSelectIDString();
					if(strOut.IsEmpty()) {
						MsgBox("You cannot filter on no locations.  The filter will be reset to { Show All Locations }");
						m_dlLocationFilter->SetSelByColumn(0, (long)-1);
						nNewId = -1;	//just change it here, and let us continue on
					}

					m_strMultiLocationIds = strOut + ",";
					m_strMultiLocationIds.Replace(" ", ",");
					m_pParent->SetAllMultiLocationStrings(m_strMultiLocationIds);
				}

			}
			// Now we are guaranteed to have the correct id, so set it and refresh
			SetActiveLocationId(nNewId);
			m_pParent->m_nLocationId = nNewId;
			UpdateView();
		}
	} NxCatchAll("CNxSchedulerDlg::OnSelChosenLocationFilter");
}

void CNxSchedulerDlg::GetTypeFilterIDs(CDWordArray* dwaryTypes)
{
	GetIDsFromCommaDelimitedString(dwaryTypes, m_strMultiTypeIds);
}

void CNxSchedulerDlg::GetPurposeFilterIDs(CDWordArray* dwaryPurposes)
{
	GetIDsFromCommaDelimitedString(dwaryPurposes, m_strMultiPurposeIds);
}
// (d.singleton 2012-07-02 15:14) - PLID 47473 - be able to choose multiple locations.
void CNxSchedulerDlg::GetLocationFilterIDs(CDWordArray* dwaryLocations)
{
	GetIDsFromCommaDelimitedString(dwaryLocations, m_strMultiLocationIds);
}

void CNxSchedulerDlg::RequeryPurposeFilters()
{
	CString strFilter;

	long nTypeSel = m_dlTypeFilter->GetCurSel();
	if(nTypeSel == -1) {
		ASSERT(FALSE);
		return;	//this shouldnt ever happen
	}

	if(VarLong(m_dlTypeFilter->GetValue(nTypeSel, 0)) == -2) {
		//multiple types
		CString strIDs = m_strMultiTypeIds;
		CString strTypes;

		long nComma = strIDs.Find(",");
		while(nComma > 0) {
			long nID = atoi(strIDs.Left(nComma));
			strIDs = strIDs.Right(strIDs.GetLength() - (nComma + 1));
			nComma = strIDs.Find(",");

			CString str;
			str.Format("OR AptPurposeTypeT.AptTypeID = %li ", nID);

			strTypes += str;
		}

		if(!strTypes.IsEmpty()) {
			strTypes = strTypes.Right(strTypes.GetLength() - 3);	//remove the opening 'OR '
			// (c.haag 2008-12-18 11:10) - PLID 32376 - Filter out inactive procedures
			strFilter.Format("AptPurposeT.ID IN (SELECT AptPurposeID FROM AptPurposeTypeT WHERE %s) AND AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1)", strTypes);
		}
	}
	else {
		// (c.haag 2008-12-18 11:10) - PLID 32376 - Filter out inactive procedures
		strFilter.Format("AptPurposeT.ID IN (SELECT AptPurposeTypeT.AptPurposeID FROM AptPurposeTypeT WHERE AptPurposeTypeT.AptTypeID = %li) AND AptPurposeT.ID NOT IN (SELECT ID FROM ProcedureT WHERE Inactive = 1)", 
			m_pParent->m_nPurposeSetId);
	}

	if (m_dlPurposeFilter->WhereClause != _bstr_t(strFilter)) {
		m_dlPurposeFilter->WhereClause = _bstr_t(strFilter);
		m_dlPurposeFilter->Requery();

		IRowSettingsPtr pRow = m_dlPurposeFilter->Row[-1];
		pRow->Value[0] = (long)-1;
		pRow->Value[1] = _bstr_t(SCHEDULER_TEXT_FILTER__ALL_PURPOSES);
		m_dlPurposeFilter->AddRow(pRow);	
		m_dlPurposeFilter->TrySetSelByColumn(0, (long)-1);

		//DRT 8/5/03 - Added a row for multiple purposes, id -2
		pRow = m_dlPurposeFilter->GetRow(-1);
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _bstr_t(SCHEDULER_TEXT_FILTER__MULTI_PURPOSES));
		m_dlPurposeFilter->AddRow(pRow);
	}
}

BOOL CNxSchedulerDlg::SetActivePurposeSetId(long nNewId)
{
	if (nNewId != m_nActivePurposeSetId && m_dlTypeFilter != NULL) {
		if (m_dlTypeFilter->FindByColumn(0, _variant_t(nNewId), 0, TRUE) >= 0)
		{
			m_bNeedUpdate = true;
			m_nActivePurposeSetId = nNewId;
			return TRUE;
		}
		// If we got here, the ID wasn't found
		return FALSE;
	}
	// If it's already set, don't search for it again
	return TRUE;
}

BOOL CNxSchedulerDlg::SetActivePurposeId(long nNewId)
{
	if (nNewId != m_nActivePurposeId && m_dlPurposeFilter != NULL) {
		if (m_dlPurposeFilter->FindByColumn(0, _variant_t(nNewId), 0, TRUE) >= 0)
		{
			m_bNeedUpdate = true;
			m_nActivePurposeId = nNewId;
			return TRUE;
		}
		// If we got here, the ID wasn't found
		return FALSE;
	}
	// If it's already set, don't search for it again
	return TRUE;
}

//TES 6/21/2010 - PLID 21341
BOOL CNxSchedulerDlg::SetActiveLocationId(long nNewId)
{
	if (nNewId != m_nActiveLocationId && m_dlLocationFilter != NULL) {
		if (m_dlLocationFilter->FindByColumn(0, _variant_t(nNewId), 0, TRUE) >= 0)
		{
			m_bNeedUpdate = true;
			m_nActiveLocationId = nNewId;
			return TRUE;
		}
		// If we got here, the ID wasn't found
		return FALSE;
	}
	// If it's already set, don't search for it again
	return TRUE;
}

void CNxSchedulerDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your message handler code here and/or call default
	if (nIDCtl == IDR_RES_POP_UP_MENU)
	{
		TextOut(lpDrawItemStruct->hDC, lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top, "alal", sizeof("alal"));
	}
	else
		CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CNxSchedulerDlg::OnNewEvent()
{
	try {

		if(!m_pEventCtrl.GetEnabled())
			return;

		// (c.haag 2010-08-27 11:38) - PLID 40108 - "Reserve" the resentrydlg so that UpdateView
		// messages get ignored while the resentry object is being initialized.
		CReserveResEntry rre(m_ResEntry, __FUNCTION__);
		// Create the res in the 0th column because the user just clicked the button; we're just simulating 
		// singleday behavior, to the user the "events" button looks just like the singleday's time buttons.
		CReservation pRes(m_pEventCtrl.AddReservation(__FUNCTION__, 0, 
			GetActiveDate(), GetActiveDate(), RGB(0,0,0), TRUE));
		// (v.maida 2016-04-22 16:27) - PLID 51696 - Make sure that we're on the correct tab before doing any menu selections
		if (pRes && (m_pParent && this == m_pParent->GetActiveSheet())) {
			// Get the date and resource id that correspond to the column that we put the reservation in
			COleDateTime dtWorkingDate;
			long nWorkingResourceID;
			m_pParent->GetWorkingResourceAndDate(this, pRes.GetDay(), nWorkingResourceID, dtWorkingDate);
			
			// Initialize the resentry and show it
			m_pEventCtrl.Resolve(); // Show the reservation on the scheduler
			m_ResEntry->ZoomResNewAppointment(pRes, nWorkingResourceID, dtWorkingDate);
		}
	} NxCatchAll("CNxSchedulerDlg::OnNewEvent");
}

void CNxSchedulerDlg::OnReservationMouseDown(LPDISPATCH theRes, short Button, short Shift, long x, long y)
{
	try {
		// (b.cardillo 2003-06-23 14:04) I believe this function is meant as a way of handling three events 
		// with a single implementation:
		//   - starting to drag an appointment
		//   - starting to resize an appointment
		//   - right-clicking on an appointment
		//  It still handles other cases (like a simple left click) when they are not necessary, but it 
		// doesn't hurt anything to do it in those cases too.
		// (v.maida 2016-04-08 13:18) - PLID 51696 - Make sure that the current sheet is equal to the active sheet
		CReservation pRes(__FUNCTION__, theRes);
		if (pRes && (m_pParent && this == m_pParent->GetActiveSheet())) {
			m_pParent->GetWorkingResourceAndDate(this, pRes.GetDay(), m_vciColInfoOfLastMouseDownRes);
		}
	} NxCatchAllCall("CNxSchedulerDlg::OnReservationMouseDown", ClearAndDisable());
}

void CNxSchedulerDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	try {
		long nCtrlID = pWnd->GetDlgCtrlID();
		switch (nCtrlID) {
		case IDC_MOVE_DAY_FORWARD:
		case IDC_MOVE_WEEK_FORWARD:
		case IDC_MOVE_MONTH_FORWARD:
		case IDC_MOVE_DAY_BACK:
		case IDC_MOVE_WEEK_BACK:
		case IDC_MOVE_MONTH_BACK:
			{
				// Load the scheduler's context popup menus
				CMenu mnu;
				if (mnu.LoadMenu(IDR_SCHEDULER_CONTEXT_MENUS)) {
					// We got the menu that has all the context popup menus in it, now get the specific submenu for the contexted button
					CMenu *pMenu = NULL;
					BOOL bIsForward = FALSE;
					UINT nDefaultMenuID = 0;
					{
						switch (nCtrlID) {
						case IDC_MOVE_DAY_FORWARD:
							pMenu = mnu.GetSubMenu(1);
							bIsForward = TRUE;
							nDefaultMenuID = ID_FORWARD_ONE_DAY;
							break;
						case IDC_MOVE_WEEK_FORWARD:
							pMenu = mnu.GetSubMenu(1);
							bIsForward = TRUE;
							nDefaultMenuID = ID_FORWARD_ONE_WEEK;
							break;
						case IDC_MOVE_MONTH_FORWARD:
							pMenu = mnu.GetSubMenu(1);
							bIsForward = TRUE;
							nDefaultMenuID = ID_FORWARD_ONE_MONTH;
							break;
						case IDC_MOVE_DAY_BACK:
							pMenu = mnu.GetSubMenu(0);
							bIsForward = FALSE;
							nDefaultMenuID = ID_BACK_ONE_DAY;
							break;
						case IDC_MOVE_WEEK_BACK:
							pMenu = mnu.GetSubMenu(0);
							bIsForward = FALSE;
							nDefaultMenuID = ID_BACK_ONE_WEEK;
							break;
						case IDC_MOVE_MONTH_BACK:
							pMenu = mnu.GetSubMenu(0);
							bIsForward = FALSE;
							nDefaultMenuID = ID_BACK_ONE_MONTH;
							break;
						default:
							pMenu = NULL;
							break;
						}
					}

					// Pop the menu in the right place with the right properties
					if (pMenu) {
						// Set the correct default menu item
						pMenu->SetDefaultItem(nDefaultMenuID);
						
						// Get the window rect for use in placing the popup menu
						CRect rc;
						pWnd->GetWindowRect(&rc);

						// Pop up the menu
						pMenu->TrackPopupMenu(
							(bIsForward ? TPM_LEFTALIGN : TPM_RIGHTALIGN), 
							(bIsForward ? rc.right : rc.left), 
							rc.top, 
							this);	

					} else {
						AfxThrowNxException("Could not load forward button submenu from scheduler context menus");
					}
				} else {
					AfxThrowNxException("Could not load scheduler context menus for forward button");
				}
			}
			break;
		default:
			break;
		}
	} NxCatchAll("CNxSchedulerDlg::OnContextMenu");
}

void CNxSchedulerDlg::MoveCurrentDateByUnits(EnumNxSchedulerForwardBackUnits nsfbuUnits, int nMoveByCount)
{
	switch (nsfbuUnits)
	{
	case nsfbuDays:
		MoveCurrentDate(COleDateTimeSpan(nMoveByCount, 0, 0, 0));
		break;
	case nsfbuWeeks:
		MoveCurrentDate(COleDateTimeSpan(nMoveByCount * 7, 0, 0, 0));
		break;
	case nsfbuMonths:
		MoveCurrentDateByMonths(nMoveByCount);
		break;
	case nsfbuYears:
		MoveCurrentDateByYears(nMoveByCount);
		break;
	default:
		// Unknown units
		ASSERT(FALSE);
		break;
	}
}

void CNxSchedulerDlg::UpdateViewBySingleAppt(CTableCheckerDetails* pDetails)
{
	// (c.haag 2010-06-23 10:10) - PLID 39210 - Don't update if a reservation is locked by
	// a right-click action or the reservation window is open.
	// (c.haag 2010-08-27 11:38) - PLID 40108 - Moved the logic into one function
	if (!IsSafeToUpdateView()) return;

	/////////////////////////////////////////////////////////////////////////
	// (c.haag 2005-01-12 13:11) - Get the reservation object
	// that corresponds to the details object. It may not exist,
	// but we need to know if it does or not.
	/////////////////////////////////////////////////////////////////////////
	// (j.jones 2014-08-05 10:34) - PLID 63167 - this now uses an enumeration
	long nResID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiAppointmentID), -1);
	COleDateTime dtStart = VarDateTime(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiStartTime), g_cdtInvalid);
	// (j.jones 2007-09-06 15:20) - PLID 27312 - added EndTime to the tablechecker (unused in this function though)
	COleDateTime dtEnd = VarDateTime(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiEndTime), g_cdtInvalid);
	long nStatus = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiStatus), -1); // 4 = cancelled
	long nShowState = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiShowState), -1);
	// (j.jones 2014-08-05 10:50) - PLID 63184 - get the patientID, LocationID, ResourceIDs
	long nPatientID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiPatientID), -1);
	long nLocationID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiLocationID), -1);
	CString strResourceIDs = VarString(pDetails->GetDetailData(TableCheckerDetailIndex::Appointments_DetailIndex::adiResourceIDs), "");

	// (j.jones 2014-08-14 12:11) - PLID 63184 - I refactored this code to be easier to read.
	// The logic is as follows:
	// - Search our view for the appointment ID. If we're displaying it, we have to run the recordset to update it.
	//	Otherwise, find out if the appointment should be shown in our view.
	// - The appointment should NOT be shown if:
	//		- The appointment is cancelled, and we're not showing cancelled appointments.
	//		- The appointment date is not in our current date range.
	//		- The appointment resource is not in our current resource filter or view.
	//		- The appointment location is not a location we're filtering on (if a location filter is used).

	// (c.haag 2010-08-03 10:41) - PLID 39962 - Refactored to keep pRes in scope for only as long
	// as it needs to be.
	bool bAppointmentExistsInView = false;
	{
		CReservation pRes(__FUNCTION__, NULL);
		for (long iDay=0; iDay < m_pSingleDayCtrl.GetDayTotalCount() && pRes == NULL; iDay++)
		{
			long i = 0;
			do {
				try {
					pRes = m_pSingleDayCtrl.GetReservation(__FUNCTION__, iDay, i++);
				}
				catch (...)
				{
					pRes.SetDispatch(NULL);
				}
			} while (pRes != NULL && pRes.GetReservationID() != nResID);
		}

		//did we find the reservation object?
		if (pRes) {
			//the changed appointment is currently displayed on the screen
			bAppointmentExistsInView = true;
		}
	}
		
	/////////////////////////////////////////////////////////////////////////
	// (c.haag 2005-01-12 13:11) - Figure out if the day should be visible.	
	/////////////////////////////////////////////////////////////////////////
	
	bool bAppointmentCanBeIgnored = false;

	// (j.jones 2014-08-14 12:16) - PLID 63184 - if the appointment is already in view,
	// we always have to update it, so skip all these checks if it exists
	if (!bAppointmentExistsInView) {

		//is the appt. date in our current view?
		if (!bAppointmentCanBeIgnored) {
			COleDateTime dtActive = GetActiveDate();
			if (!((CSchedulerView*)GetParent())->IsDateInCurView(dtStart))
			{
				//this appt. is not for a date in our view, we can ignore it
				bAppointmentCanBeIgnored = true;
			}
		}

		// (j.jones 2014-08-14 12:25) - PLID 63184 - added location filtering
		// is the appt. location in our current location filter?
		if (!bAppointmentCanBeIgnored) {
			if (!IsLocationInView(nLocationID))
			{
				//this appt. is not for a location in our filter, we can ignore it
				bAppointmentCanBeIgnored = true;
			}
		}

		// (j.jones 2014-08-14 12:25) - PLID 63184 - added resource filtering
		// are any of the appt. resources in our current resource filter?
		if (!bAppointmentCanBeIgnored) {
			CDWordArray aryResourceIDs;
			ParseDelimitedStringToDWordArray(strResourceIDs, " ", aryResourceIDs);
			if (!((CSchedulerView*)GetParent())->AreAnyResourcesInCurrentFilter(aryResourceIDs))
			{
				//this appt. is not for a resource in our filter, we can ignore it
				bAppointmentCanBeIgnored = true;
			}
		}

		//is the appointment cancelled, and are we hiding cancelled appointments?
		if (!bAppointmentCanBeIgnored) {
			if (nStatus == 4 && m_nCancelledAppt != 1) {
				//this appt. is cancelled, and we are not viewing cancelled appts., so we can ignore it
				bAppointmentCanBeIgnored = true;
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////
	// (c.haag 2005-01-12 13:11) - Now make our command decision based on
	// all the information we have. The logic is: If the appointment should
	// be on the screen (again we are ignoring type, purpose and resource
	// filters), or, the appointment should NOT be on the screen but it
	// currently IS on the screen, we need to deal with it.
	/////////////////////////////////////////////////////////////////////////
	// (j.jones 2014-08-14 12:19) - PLID 63184 - this now just uses our booleans
	// we calculated earlier in the function
	if (bAppointmentExistsInView || !bAppointmentCanBeIgnored)
	{
		UpdateViewBySingleAppt(nResID);
	}
}

// (j.jones 2014-08-13 16:57) - PLID 63184 - this now takes in the ResourceIDs and PurposeIDs, rather than a recordset
void CNxSchedulerDlg::EnsureApptInVisibleApptArray(long nResID, ADODB::_RecordsetPtr& prsAppt, CString strResourceIDs, CString strPurposeIDs, short nOffsetDay)
{
	// (c.haag 2007-01-05 09:16) - PLID 23845 - This function ensures that an appointment
	// exists in m_aVisibleAppts for proper appointment rendering when precision templating
	// is being used
	const int nAppts = m_aVisibleAppts.GetSize();
	int nIndex = -1;
	// (j.luckoski 2012-05-07 11:27) - PLID 11597 - If cancelled, the appt is not in visible appt so don't run this area
	// (j.luckoski 2012-05-09 14:42) - PLID 50264 - Use ReturnsRecordsParam
	// (a.walling 2013-01-21 16:48) - PLID 54744 - Available in Recordset
	if(4 != AdoFldByte(prsAppt, "Status", -1)) {
		for (int i=0; i < nAppts && -1 == nIndex; i++) {
			if (nResID == m_aVisibleAppts[i]->nReservationID &&
				nOffsetDay == (int)m_aVisibleAppts[i]->nOffsetDay) {
					nIndex = i;
			}
		}
		if (-1 == nIndex) {
			nIndex = nAppts;
			m_aVisibleAppts.Add(new VisibleAppointment);
			m_aVisibleAppts[nIndex]->nReservationID = nResID;
			m_aVisibleAppts[nIndex]->nOffsetDay = nOffsetDay;	
		}

		VisibleAppointment* pAppt = m_aVisibleAppts[nIndex];
		pAppt->nAptTypeID = AdoFldLong(prsAppt, "AptTypeID", -1);
		pAppt->dtStart = AdoFldDateTime(prsAppt, "StartTime");
		pAppt->dtEnd = AdoFldDateTime(prsAppt, "EndTime");

		// Update the resources and purposes. This is a speed hit, but so is pulling it from ::GetSingleApptQuery. There's
		// no way around this.
		// (j.jones 2014-08-13 16:58) - PLID 63184 - these are now provided to us
		LoadResourceIDStringIntoArray(strResourceIDs, pAppt->anResourceIDs);
		LoadPurposeIDStringIntoArray(strPurposeIDs, pAppt->anPurposeIDs);
	}
}

void CNxSchedulerDlg::EnsureApptNotInVisibleApptArray(long nResID, short nOffsetDay)
{
	// (c.haag 2007-01-05 09:16) - PLID 23845 - This function ensures that an appointment
	// does not exist in m_aVisibleAppts for proper appointment rendering when precision
	// scheduling is being used
	const int nAppts = m_aVisibleAppts.GetSize();
	for (int i=0; i < nAppts; i++) {
		if (nResID == m_aVisibleAppts[i]->nReservationID &&
			nOffsetDay == (int)m_aVisibleAppts[i]->nOffsetDay) {
			delete m_aVisibleAppts[i];
			m_aVisibleAppts.RemoveAt(i);
			return;
		}
	}
}

void CNxSchedulerDlg::UpdateViewBySingleAppt(long nResID)
{
	CSingleDay pCtrl = NULL;
	BOOL bNeedUpdateView = FALSE;
	if (m_pSingleDayCtrl == NULL)
		return;

	// (c.haag 2010-06-23 10:10) - PLID 39210 - Don't update if a reservation is locked by
	// a right-click action or the reservation window is open.
	// (c.haag 2010-08-27 11:38) - PLID 40108 - Moved the logic into one function
	if (!IsSafeToUpdateView()) return;

	// If the user is in the act of dragging or resizing the appointment,
	// and the appointment ID matches nResID, we do not want to change
	// anything on the screen. We will let CommitResDrag deal with cases
	// where the appointment is deleted.
	if (m_pParent && m_pParent->m_pResDragging != NULL)
	{
		CReservation pRes(__FUNCTION__, m_pParent->m_pResDragging);
		if (pRes.GetReservationID() == nResID)
			return;
	}

	// (v.maida 2016-04-22 16:27) - PLID 51696 - Don't continue if we're not on the correct tab.
	if (m_pParent && this != m_pParent->GetActiveSheet()) {
		return;
	}

	COleDateTime dtStart, dtEnd;
	BOOL bInvisible = FALSE;
	BOOL bSingleDayChanged = FALSE; // (c.haag 2013-08-05) - PLID 57868
	try {
		BOOL bNeedResolve = FALSE;
		_RecordsetPtr prs = GetSingleApptQuery(nResID);
		
		// (j.luckoski 2012-07-02 09:49) - PLID 11597 - Handle the bool for cancelled appts here to be accessed below
		bool bIsResCancelled = false;

		/* If this appointment is deleted, we want to wipe
		it off the singleday */
		if (prs->eof)
		{
			bInvisible = TRUE;
			prs->Close();
		}
		else
		{
			// (a.walling 2013-01-21 16:48) - PLID 54744 - Available in Recordset
			if (4 == AdoFldByte(prs, "Status", -1)) {
				bIsResCancelled = true;
			}

			/* If this is an event, change our target control to
			the event control. */
			dtStart = prs->Fields->Item["StartTime"]->Value.date;
			dtEnd = prs->Fields->Item["EndTime"]->Value.date;

			if (dtStart == dtEnd &&	dtStart.GetHour() == 0 && dtStart.GetMinute() == 0)
			{
				pCtrl = m_pEventCtrl;
			}
			else
			{
				pCtrl = m_pSingleDayCtrl;
			}
		}
		if (pCtrl == NULL && !bInvisible)
			return;

		/* If this appointment is cancelled, we want to wipe
		it off the singleday */
		if (!bInvisible && prs->Fields->Item["Status"]->Value.lVal == 4 && m_nCancelledAppt == 0)
		{
			bInvisible = TRUE;
			prs->Close();
		}

		// (j.jones 2014-08-13 16:55) - PLID 63184 - we now load this on demand, just once
		bool bLoadedResourcePurposeInfo = false;
		CString strResourceIDs = "";
		CString strPurposeIDs = "";
		CDWordArray adwResourceIDs;
		CDWordArray adwPurposeIDs;
		
		/* Now traverse every singleday column */
		for (long iDay=0; iDay < m_pSingleDayCtrl.GetDayTotalCount(); iDay++)
		{
			/* For this column we need to find the appointment. We
			assume a column will either have 1, or 0 occurences of
			this appointment. */
			CReservation pRes(__FUNCTION__), pResOld(__FUNCTION__);
			BOOL bAllowDelete = TRUE;
			long i=0;
			do {
				pResOld = pRes;
				try {
					pRes = m_pSingleDayCtrl.GetReservation(__FUNCTION__, iDay, i++);
				}
				catch (...)	/* GetReservation will throw an exception if there are no reservations
							in the singleday. */
				{
					pRes.SetDispatch(NULL);
				}
			} while (pRes != NULL && pRes.GetReservationID() != nResID && pResOld != pRes);
			if (pResOld == pRes)
				pRes.SetDispatch(NULL);
			/* If we found the reservation but haven't figured out which singleday to use
			beforehand, we now know which one it is. */
			if (pRes != NULL && pCtrl == NULL)
				pCtrl = m_pSingleDayCtrl;


			/* If we didn't find it, repeat the search in the single day control.
			This can happen if prs is eof but the appointment that changed is
			actually an event */
			if (pRes == NULL && m_pEventCtrl != NULL)
			{
				i=0;
				do {
					pResOld = pRes;
					try {
						pRes = m_pEventCtrl.GetReservation(__FUNCTION__, iDay, i++);
					}
					catch (...)	/* GetReservation will throw an exception if there are no reservations
								in the singleday. */
					{
						pRes.SetDispatch(NULL);
					}
				} while (pRes != NULL && pRes.GetReservationID() != nResID && pResOld != pRes);

				/* If we found the reservation but haven't figured out which singleday to use
				beforehand, we now know which one it is. */
				if (pRes != NULL && pCtrl == NULL)
					pCtrl = m_pEventCtrl;
			}
			if (pResOld == pRes)
				pRes.SetDispatch(NULL);

			// (c.haag 2007-01-05 08:35) - PLID 23845 - Ensure the appointment does not exist
			// in the VisibleAppointment array. We know this should be called here because if
			// the appointment should indeed be visible, we will satisfy the conditional that
			// results in the visible appointment being added and/or updated. This is for
			// precision templating only; if precision templating is turned off, then this
			// has no effect
			EnsureApptNotInVisibleApptArray(nResID, (short)iDay);

			/* If we still don't have a singleday by now, proceed to the next
			column */
			if (pCtrl == NULL)
				continue;

			/* It's found; we need to either delete it or update it if
			it's in the wrong spot */
			if (!bInvisible && 
				/* Before we know for a fact that this appointment should be
				added to this slot, find out that this slot actually supports
				the appointment */
				SingleApptFitsSlot(iDay, prs))
			{
				COleDateTime dtDate = AdoFldDateTime(prs->Fields, "Date");
				dtStart = AdoFldDateTime(prs->Fields, "StartTime");
				dtEnd = AdoFldDateTime(prs->Fields, "EndTime");
				long nLocationID = AdoFldLong(prs->Fields, "LocationID");
				long nAptTypeID = AdoFldLong(prs->Fields, "AptTypeID", -1);
				long nPrimaryInsuredPartyID = AdoFldLong(prs->Fields, "PrimaryInsuredPartyID", -1);

				/***** DON'T FORGET THAT WHEN YOU CUT AND PASTE AN APPT, DO
				NOT SEND A TABLECHECKER MESSAGE REGARDING THE NEW APPOINTMENT
				UNTIL ITS END TIME HAS BEEN DECIDED! *****/

				// Add the appointment at the right time while retrieving a pointer to it
				CReservation pResNew(__FUNCTION__);

				if (pRes != NULL)
				{
					pResNew = pRes;
					bAllowDelete = FALSE;
				}
				else
				{
					pResNew = pCtrl.AddReservation(__FUNCTION__, (short)iDay, 
						dtStart, dtEnd, 
						// (c.haag 2006-05-11 10:48) - PLID 20580 - This information is in ResExtendedQ
						//AppointmentGetBorderColor(nAptTypeID),
						AdoFldLong(prs, "ShowStateColor", DEFAULT_APPT_BACKGROUND_COLOR),
						TRUE);
					bSingleDayChanged = TRUE; // (c.haag 2013-08-05) - PLID 57868
				}

				if (pResNew == NULL)
				{
					/* The act of making a new reservation failed, so lets
					fall back on the old way of doing things. */
					bNeedUpdateView = TRUE;
				}
				else
				{
					/* Properly color the background */

					/* Build our multi-purpose string */
					CString strMultiPurpose;
					CDWordArray adwPurposes;
					prs->MoveFirst();
					while (!prs->eof)
					{
						if(prs->Fields->Item["AptPurposeID"]->Value.vt != VT_NULL) {
							// See if this purpose was already added
							//TES 11/7/2007 - PLID 27979 - VS2008 - for() loops
							int i = 0;
							for (i=0; i < adwPurposes.GetSize(); i++)
							{
								if (adwPurposes.GetAt(i) == (DWORD)AdoFldLong(prs, "AptPurposeID"))
									break;
							}
							if (i == adwPurposes.GetSize())
							{
								adwPurposes.Add((DWORD)AdoFldLong(prs, "AptPurposeID"));
								strMultiPurpose += AdoFldString(prs->Fields->Item["AptPurpose"], "") + ", ";
							}
						}
						prs->MoveNext();
					}
					if (strMultiPurpose.GetLength() >= 2)
					{
						// Cut the last ", "
						strMultiPurpose = strMultiPurpose.Left( strMultiPurpose.GetLength() - 2);
					}
					prs->MoveFirst();

					// (j.jones 2014-08-13 16:55) - PLID 63184 - we now load this on demand, just once
					if (!bLoadedResourcePurposeInfo) {
						// (c.haag 2007-01-05 09:54) - PLID 23845 - Pull the resources and purposes. This is a speed hit, but so is pulling
						// it from ::GetSingleApptQuery (which returns multiple records for one appt). There's no way around this.
						_RecordsetPtr prsResourcePurpose = CreateParamRecordset("SELECT dbo.GetResourceIDString({INT}) AS ResourceIDs, dbo.GetPurposeIDString({INT}) AS PurposeIDs",
							nResID, nResID);

						// (j.jones 2011-02-11 12:24) - PLID 35180 - load the resource IDs
						if (!prsResourcePurpose->eof) {
							strResourceIDs = VarString(prsResourcePurpose->Fields->Item["ResourceIDs"]->Value, "");
							strPurposeIDs = VarString(prsResourcePurpose->Fields->Item["PurposeIDs"]->Value, "");
						}

						prsResourcePurpose->Close();

						LoadResourceIDStringIntoArray(strResourceIDs, adwResourceIDs);
						LoadPurposeIDStringIntoArray(strPurposeIDs, adwPurposeIDs);

						bLoadedResourcePurposeInfo = true;
					}

					/* Update the information */
					COleDateTime dtStartHMS, dtEndHMS;
					dtStartHMS.SetTime(dtStart.GetHour(), dtStart.GetMinute(), 0);
					dtEndHMS.SetTime(dtEnd.GetHour(), dtEnd.GetMinute(), 0);
					pResNew.PutReservationID(nResID);
					// (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
					pResNew.Data["CancelledDate"] = prs->Fields->Item["CancelledDate"]->Value;
					// (j.jones 2011-02-11 12:01) - PLID 35180 - required dates and a string of resource IDs
					// (j.luckoski 2012-05-07 11:30) - PLID 11597 - If cancelled, display cancelled on appt.
					// (j.luckoski 2012-05-09 14:42) - PLID 50264 - Use ReturnsRecordsParam
					// (j.luckoski 2012-06-11 16:06) - PLID 11597 - pRes might have been causing a invalid pointer error.
					pResNew.PutText(_bstr_t(AppointmentGetText(nResID, dtStart, dtEnd, strResourceIDs, (CSchedulerView*)GetParent()/*m_pSchedulerView*/, strMultiPurpose)));
					if(bIsResCancelled) {
						// (j.luckoski 2012-06-20 11:14) - PLID 11597 - Prohibit drag and resizing on cancelled appts.
						pResNew.PutAllowDrag(VARIANT_FALSE);
						pResNew.PutAllowResize(VARIANT_FALSE);
					} else {
						// (j.luckoski 2012-06-20 14:45) - PLID 11597 - Return putallowdrag back to true
						// (j.luckoski 2012-06-25 10:35) - PLID 11597 - Accidentally put pRes instead of pResNew
						pResNew.PutAllowDrag(TRUE);
						pResNew.PutAllowResize(TRUE);
					}
					pResNew.PutStartTime(dtStartHMS);
					pResNew.PutEndTime(dtEndHMS);
					// (c.haag 2005-10-14 13:37) - PLID 17956 - Now the border and back colors are updated
					// (c.haag 2006-05-11 10:48) - PLID 20580 - This information is in ResExtendedQ, so there's
					// no need to call global utility functions to get it
					COLORREF clrBorder = AdoFldLong(prs, "Color", 0);
					pResNew.PutBorderColor(clrBorder);
					//TES 3/8/2011 - PLID 41519 - If we are highlighting appointments, and this was the highlighted appointment, then tell
					// the control to highlight the new appointment.
					// (d.thompson 2012-08-01) - PLID 51898 - Changed default to Off
					if (GetRemotePropertyInt("SchedHighlightLastAppt", 0, 0, GetCurrentUserName(), true)
						&& pCtrl.GetHighlightRes() == pRes.GetDispatch()) {
							pCtrl.SetHighlightRes(pResNew.GetDispatch());
					}
					else {
						// (a.wilson 2012-06-14 16:18) - PLID 47966 - change for new preference type.
						// (j.luckoski 2012-06-20 11:14) - PLID 11597 - Edit the color different for cancelled or uncancelled appts.
						COLORREF clrBkg = DEFAULT_APPT_BACKGROUND_COLOR;
						if (bIsResCancelled) {
							clrBkg = m_nCancelColor;
						}
						// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
						else if (GetRemotePropertyInt("ColorApptBgWithStatus", GetPropertyInt("ColorApptBgWithStatus", 1, 0, false), 0, GetCurrentUserName(), true)) {
							// (j.jones 2014-12-04 14:46) - PLID 64119 - use the mix rule color if one exists and
							// the appt. is pending, otherwise use the status color
							long nMixRuleColor = -1;
							if (AdoFldLong(prs, "ShowState", 0) == 0) {	//pending
								nMixRuleColor = GetAppointmentMixRuleColor(AsDateNoTime(dtDate), nLocationID, nPrimaryInsuredPartyID, nAptTypeID, adwResourceIDs, adwPurposeIDs);
							}

							if (nMixRuleColor != -1) {
								clrBkg = nMixRuleColor;
							}
							else {
								clrBkg = AdoFldLong(prs, "ShowStateColor", DEFAULT_APPT_BACKGROUND_COLOR);
							}
						}
						else {
							clrBkg = clrBorder;
						}
						ColorReservationBackground(pResNew, clrBkg);
					}

					// (c.haag 2007-01-05 08:35) - PLID 23845 - Ensure the appointment exists
					// in the VisibleAppointment array. We know this should be called here because
					// this code is not missed if the appointment exists on the screen. This is for
					// precision templating only; if precision templating is turned off, then this
					// has no effect
					// (j.jones 2014-08-13 16:57) - PLID 63184 - pass in the ResourceIDs and PurposeIDs, rather than a recordset
					EnsureApptInVisibleApptArray(nResID, prs, strResourceIDs, strPurposeIDs, (short)iDay);

					bSingleDayChanged = TRUE; // (c.haag 2013-08-05) - PLID 57868
				}
			}

			/* Delete the old reservation in the singleday if it exists */
			if (pRes != NULL && bAllowDelete)
			{
				/* Do not delete the reservation object if the user has exclusive access to it.
				This reservation will be properly refreshed on the screen after the user either saves or
				cancels the changes. We will leave the specifics of that to CResEntryDlg. */
				if (m_ResEntry && pRes != m_ResEntry->m_Res)
				{
					// (a.walling 2007-08-31 12:57) - PLID 27265 - If there is a popup menu for this Reservation,
					// close the popup menu to prevent the user from doing anything to a Reservation that is no
					// longer valid. I think closing the popup menu is good enough, since a messagebox would interrupt.
					if (m_pParent && AreCOMObjectsEqual(pRes.GetDispatch(), m_pParent->m_pResDispatchPtr.GetDispatch())) {
						// cancel the popup menu!
						m_pParent->SendMessage(WM_CANCELMODE, 0, 0);
					}
					pRes.DeleteRes(__FUNCTION__);

					bSingleDayChanged = TRUE; // (c.haag 2013-08-05) - PLID 57868
				}
				bNeedResolve = FALSE; /* We don't need a resolve anymore */
			}

			/* Flag a "Resolve" if necessary */
			else if (!bInvisible)
			{
				bNeedResolve = TRUE;
			}
		}
		if (bNeedResolve)
		{
			pCtrl.Resolve();
			pCtrl.Refresh();
		}
		/* Reflect the fact that the count may have changed */
		EnsureCountLabelText();

		/* Reflect the change in the scheduled patients window */
		if (m_pParent && m_pParent->m_pdlgScheduledPatients->GetSafeHwnd())
		{
			if (bInvisible || AdoFldLong(prs, "ShowState") > 1)
			{
				m_pParent->m_pdlgScheduledPatients->DeleteAppointment(nResID);				
			}	
			else
			{
				switch (AdoFldLong(prs, "ShowState"))
				{
				case 0:	m_pParent->m_pdlgScheduledPatients->AddAppointment(nResID, AdoFldDateTime(prs, "StartTime"), AdoFldLong(prs, "PatientID"), AdoFldString(prs, "PatientName"), FALSE); break;
				case 1: m_pParent->m_pdlgScheduledPatients->AddAppointment(nResID, AdoFldDateTime(prs, "StartTime"), AdoFldLong(prs, "PatientID"), AdoFldString(prs, "PatientName"), TRUE); break;
				default:
					break;
				}
			}
		}
	}
	NxCatchAll("Error updating view by single appt");
	//(e.lally 2007-08-27) PLID 27190 - This call to .detach() has been here since the creation of the UpdateViewBySingleAppt
		//function on 11/25/2002, but it is causing memory leaks to occur since it does not have a corresponding attach or release.
		//I did a historic review of the code and could not find any remnants of special attaches or releases to counteract this.
		//I checked other scheduler files as well as of 1/1/2003 and those turned up empty as well. We believe it is a safe enough
		//assumption to get rid of this pCtrl.Detach() without creating instability (an actual crash) elsewhere in code. 
	//pCtrl.Detach();

	// (z.manning, 10/12/2006) - PLID 5812 - There is now a legitimate reason to update the view from here.
	// If the times of this appointment fall outside the current visible range of the schedule, we need to
	// update the view so it will make the necessary adjustments to increase the visible range of the schedule.
	// (c.haag 2013-08-05) - PLID 57868 - If bInvisible is true, that means the appointment should not appear in the view at all;
	// and there would therefore be no reason to update the visible time range. One could argue that the range should collapse
	// if an appoinment on the edge was moved, but I don't think that's not much of a benefit when the CPU cycles on both the
	// client and the server can be better spent elsewhere.
	if(!bInvisible && NeedToUpdateVisibleTimeRange(dtStart,dtEnd))
	{
		m_bNeedUpdate = true;
		UpdateView();
	}
	else if (bNeedUpdateView)	// This should never happen, but it helps make this air-tight
	{
		ASSERT(FALSE);
		LogDetail("WARNING: UpdateView called from UpdateViewBySingleAppt()");
		UpdateView();

		// (c.haag 2013-08-05) - PLID 57868 - If the singleday didn't change in any way, there's no reason to repopulate
		// the precision or non-precision template blocks
	} else if (bSingleDayChanged) {
		// (c.haag 2007-01-04 17:30) - PLID 23666 - Ensure the blocks appear at the correct times. By
		// this appointment in time, the VisibleAppointments array should be up to date if we use precision
		// templating
		UpdateTemplateBlocks(true);
	}
}

//DRT 7/29/2004 - PLID 9960 - This function loops through all reservations currently on the screen 
//	and returns whether it is visible or not.  This does NOT take scrolling into account, it just
//	looks at all appointments on the current single day & event control.
//Copied & modified a bit of this code out of UpdateViewBySingleAppt()
//TODO:  It would be nice if UpdateViewBySingleAppt() used this function (it would need to return
//	a pointer to the reservation instead of a bool or some such), but I don't want to cause
//	more instability right now.
BOOL CNxSchedulerDlg::IsAppointmentVisible(long nResID)
{
	if (m_pSingleDayCtrl == NULL)
		return FALSE;

	try {
		/* Now traverse every singleday column */
		for (long iDay=0; iDay < m_pSingleDayCtrl.GetDayTotalCount(); iDay++)
		{
			/* For this column we need to find the appointment. We
			assume a column will either have 1, or 0 occurences of
			this appointment. */
			CReservation pRes(__FUNCTION__, NULL), pResOld(__FUNCTION__, NULL);
			long i=0;
			do {
				pResOld = pRes;
				try {
					pRes = m_pSingleDayCtrl.GetReservation(__FUNCTION__, iDay, i++);

					if(pRes != NULL && pRes.GetReservationID() == nResID)
						return TRUE;
				}
				catch (...)	/* GetReservation will throw an exception if there are no reservations
							in the singleday. */
				{
					pRes.SetDispatch(NULL);
				}
			} while (pRes != NULL && pResOld != pRes);

			if (pResOld == pRes)
				pRes.SetDispatch(NULL);

			/* If we didn't find it, repeat the search in the event control.
			This can happen if prs is eof but the appointment that changed is
			actually an event */
			if (pRes == NULL && m_pEventCtrl != NULL)
			{
				i=0;
				do {
					pResOld = pRes;
					try {
						pRes = m_pEventCtrl.GetReservation(__FUNCTION__, iDay, i++);

						if(pRes != NULL && pRes.GetReservationID() == nResID)
							return TRUE;
					}
					catch (...)	/* GetReservation will throw an exception if there are no reservations
								in the singleday. */
					{
						pRes.SetDispatch(NULL);
					}
				} while (pRes != NULL && pResOld != pRes);

			}
		}

	} NxCatchAll("Error in IsAppointmentVisible()");

	//if we have not found it at this point, it doesn't exist
	return FALSE;
}

_RecordsetPtr CNxSchedulerDlg::GetSingleApptQuery(long nResID)
{
	// Generate the extra filter
	// (a.walling 2013-05-31 14:06) - PLID 56961 - GetExtraResSimpleFilter is parameterized and simplified
	CSqlFragment filter = GetExtraResSimpleFilter();
	
	// Build the query

	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	// (j.jones 2011-02-11 09:31) - PLID 35180 - ResExtendedQ is now a function that takes in:
	//@ExcludeCanceledAppointments BIT, @FromDate DATETIME, @ToDate DATETIME, @ResourceIDs NTEXT
	// (j.luckoski 2012-05-07 11:31) - PLID 11597-  Query will grab cancelled appts if they are cancelled within so many hours

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



	// (j.luckoski 2012-06-20 11:15) - PLID 11597 - Cancelled appts return the cancelled color for showstate and color
	// (a.walling 2013-05-31 14:06) - PLID 56961 - Select from GetResSimpleSql aliased to ResExQ, and restrict the date range and resource(s)
	// (a.walling 2013-06-18 11:12) - PLID 57196 - Use snapshot isolation for the main scheduler CReservationReadSet queries
	// (j.jones 2014-12-04 15:31) - PLID 64119 - added PrimaryInsuredPartyID
	return CreateParamRecordset(GetRemoteDataSnapshot(), 
		"DECLARE @QueryDate DATETIME; \r\n"
		"SET @QueryDate = {OLEDATETIME}; \r\n"
		"SELECT Status, StartTime, Convert(DateTime,ResExQ.Date) AS Date, EndTime, LocationID, AptTypeID, AptPurposeID, AptPurpose, PatientID, PatientName, ShowState, "
		"LastLM, ModifiedDate, ModifiedLogin, '' AS ResourceString, CASE WHEN ResExQ.Status = 4 THEN {INT} ELSE Color END AS Color, "
		"CASE WHEN ResExQ.Status = 4 THEN {INT} ELSE ShowStateColor END AS ShowStateColor, "
		"ResExQ.Status, ResExQ.CancelledDate, " // (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
		"PrimaryAppointmentInsuredPartyQ.InsuredPartyID AS PrimaryInsuredPartyID "
		"FROM ({SQL}) AS ResExQ "
		"LEFT JOIN ("
		"	SELECT AppointmentID, InsuredPartyID FROM AppointmentInsuredPartyT "
		"	WHERE Placement = 1 "
		") AS PrimaryAppointmentInsuredPartyQ ON ResExQ.ID = PrimaryAppointmentInsuredPartyQ.AppointmentID "
		"WHERE ResExQ.ID = {INT} "
		"AND ResExQ.ResourceID = {INT} "
		"AND ResExQ.StartTime >= @QueryDate AND ResExQ.StartTime < DATEADD(day, 1, @QueryDate) "
		"{SQL} "
		"ORDER BY AptPurpose "
		, AsDateNoTime(GetActiveDate())
		, m_nCancelColor, m_nCancelColor
		, Nx::Scheduler::GetResSimpleSql()
		, nResID
		, m_pParent->GetActiveResourceID()
		, filter
	);
}

BOOL CNxSchedulerDlg::SingleApptFitsSlot(long iDay, _RecordsetPtr& prsAppt)
{
	return TRUE;
}

void CNxSchedulerDlg::EnsureAmPmText()
{
	COleDateTime dtNewTime, dtAmTime, dtPmTime;
	//Are we calculating?
	if(GetRemotePropertyInt("CalculateAmPmTime", 0, 0, "<None>", true)) {
		//OK, the A.M. time will either be the opening time, or, if the office is closed, 8:00 AM
		if(!m_pParent->m_lohOfficeHours.GetOfficeHours(GetActiveDate().GetDayOfWeek() - 1, dtAmTime, dtPmTime)) {
			dtAmTime.SetTime(8,0,0);
			dtPmTime.SetTime(13,0,0);
		}
		else {
			//The P.M. time will be (closing time - opening time) / 2
			COleDateTimeSpan dtsDayLength = dtPmTime - dtAmTime;
			dtsDayLength = dtsDayLength / 2;
			dtPmTime = dtAmTime + dtsDayLength;
		}
	}
	else {
		//TES 1/8/2009 - PLID 32661 - These were being stored as strings, which was wrong
		// and messing up the new preferences.  I made new preferences for these values,
		// which are actually stored as datetimes.
		// (c.haag 2009-02-17 10:55) - PLID 33124 - Set the date to 1899-12-30; not 1899-12-31,
		// or else the time value will contain a day as well; and break the anchor button.
		COleDateTime dtDefault;
		CString strDefault = GetRemotePropertyText("DefaultAmTime", "1899-12-30 08:00:00", 0, "<None>", false);
		dtDefault.ParseDateTime(strDefault);						
		dtAmTime = GetRemotePropertyDateTime("DefaultAmAnchorTime", &dtDefault, 0, "<None>", true);
		strDefault = GetRemotePropertyText("DefaultPmTime", "1899-12-30 13:00:00", 0, "<None>", false);
		dtDefault.ParseDateTime(strDefault);
		dtPmTime = GetRemotePropertyDateTime("DefaultPmAnchorTime", &dtDefault, 0, "<None>", true);
		// (c.haag 2009-02-17 11:11) - PLID 33124 - Remove any date value from these times in case
		// the value in data contained a day
		dtAmTime.SetTime(dtAmTime.GetHour(), dtAmTime.GetMinute(), dtAmTime.GetSecond());
		dtPmTime.SetTime(dtPmTime.GetHour(), dtPmTime.GetMinute(), dtPmTime.GetSecond());
	}
	if(m_pParent->m_bGoToAM) {
		// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
		m_nxbtnAmPmButton.SetText(FormatDateTimeForInterface(dtAmTime, DTF_STRIP_SECONDS, dtoTime));
	}
	else {
		// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
		m_nxbtnAmPmButton.SetText(FormatDateTimeForInterface(dtPmTime, DTF_STRIP_SECONDS, dtoTime));
	}
	
	// (z.manning, 10/16/2006) - PLID 5812 - This code was moved here from CSchedulerView::OnCreate()
	// because we need to know the schedule's time range before we can accurately specify the top slot.
	if(m_pParent->GetTopSlot() < 0) {
		// Make sure we set the top slot relative to the current time range.  Also, we need to
		// get the start time again because it may have changed when it was being set if an appt
		// did not fit on the range we tried to set it to.
		long nInterval = GetRemotePropertyInt("ResInterval", 15, 0, GetCurrentUserName());
		COleDateTime dtStart;
		dtStart.ParseDateTime(m_pSingleDayCtrl.GetBeginTime(), VAR_TIMEVALUEONLY);

		// (z.manning, 03/29/2007) - PLID 25412 - If the anchor time is less than the visible start time, ensure
		// we don't try to set the top slot to a negative value.
		int nAmTimeHour = dtAmTime.GetHour();
		if(nAmTimeHour < dtStart.GetHour()) {
			nAmTimeHour = dtStart.GetHour();
		}

		m_pParent->m_nTopSlot = ( (nAmTimeHour - dtStart.GetHour()) * 60 + dtAmTime.GetMinute() ) / nInterval; // 8:00a slot = 8 * slot/hour
		m_pSingleDayCtrl.PutTopSlot(m_pParent->GetTopSlot());
	}
}


void CNxSchedulerDlg::GetPrintSlotRange(long &nFirstPrintSlot, long &nLastPrintSlot, CPrintInfo *pInfo)
{
	//TES 03-21-2003: This function is an essential duplicate of an identically named function in the singleday.
	//In theory, this should just call that function, but that function is not exposed, and apparently exposing
	//it would be, shall we say, problematic.  So, if either one is changed, they both need to be changed.
	COleDateTime dtBegin, dtEnd;
	nFirstPrintSlot = -1;
	nLastPrintSlot = -1;
	long slotMin, slotMax;
	for(int i = 0; i < m_pSingleDayCtrl.GetDayVisibleCount(); i++) { //For each day, get the count.  We'll take the highest one and return it.
		
		slotMin = -1;
		slotMax  = -1;
		//First, make sure we've got at least one node.
		CReservation resFirst = m_pSingleDayCtrl.GetReservation(__FUNCTION__, i, 0);
		if(resFirst) {
			slotMin = resFirst.GetTopSlot();
			slotMax = resFirst.GetBottomSlot();
		}

		if(slotMin != -1 || slotMax != -1) {
			//OK, we've definitely got one.  So, find the biggest and smallest.
			long nIndex = 0;
			CReservation pPrevious = CReservation(__FUNCTION__, NULL), pCurrent = m_pSingleDayCtrl.GetReservation(__FUNCTION__, i, nIndex);
			if(pCurrent) {
				if(pCurrent.GetTopSlot() < slotMin) slotMin = pCurrent.GetTopSlot();
				if(pCurrent.GetBottomSlot() > slotMax) slotMax = pCurrent.GetBottomSlot();
				pPrevious = pCurrent;
				nIndex++;
				pCurrent = m_pSingleDayCtrl.GetReservation(__FUNCTION__, i, nIndex);
				while(pCurrent != NULL && pCurrent != pPrevious) {
					if(pCurrent.GetTopSlot() < slotMin) slotMin = pCurrent.GetTopSlot();
					if(pCurrent.GetBottomSlot() > slotMax) slotMax = pCurrent.GetBottomSlot();
					pPrevious = pCurrent;
					nIndex++;
					pCurrent = m_pSingleDayCtrl.GetReservation(__FUNCTION__, i, nIndex);
				}
			}
		}
		else {
			//They don't have any appts.
			slotMin = -1;
			slotMax = -1;
		}

		NXSINGLEDAYPRINT *nxsp = reinterpret_cast<NXSINGLEDAYPRINT *>(pInfo->m_lpUserData);
		if(nxsp->bPrintBlockColor || nxsp->bPrintBlockText) { //If we're showing templates, make sure we show all of them.
			long nWorkingResourceID;
			COleDateTime dtWorkingDate;
			m_pParent->GetWorkingResourceAndDate(this, i, nWorkingResourceID, dtWorkingDate);
			CTemplateHitSet rsTemplateHits;
			// (a.walling 2010-06-25 12:36) - PLID 39278 - Exclude resource availability templates
			rsTemplateHits.Requery(dtWorkingDate, nWorkingResourceID, FALSE, -1, true, false);
			while (!rsTemplateHits.IsEOF()) {
				if(slotMin == -1 || slotMin > m_pSingleDayCtrl.GetTimeSlot(rsTemplateHits.GetStartTime())) slotMin = m_pSingleDayCtrl.GetTimeSlot(rsTemplateHits.GetStartTime());
				if(slotMax == -1 || slotMax < m_pSingleDayCtrl.GetTimeSlot(rsTemplateHits.GetEndTime())) slotMax = m_pSingleDayCtrl.GetTimeSlot(rsTemplateHits.GetEndTime());
				rsTemplateHits.MoveNext();
			}
		}
		long nMaxFirstSlotToPrint;
		{
			COleDateTime dtMinRequired;
			dtMinRequired.SetTime(8, 0, 0);
			dtBegin.ParseDateTime(m_pSingleDayCtrl.GetBeginTime());
			if (dtBegin > dtMinRequired) {
				// Our first slot is already greater than the official min, so start with the first slot
				nMaxFirstSlotToPrint = 0;
			} else {
				// Our first slot is less than the min, so start with the min, which is the number of slots between the begin time and the min required time
				nMaxFirstSlotToPrint = (long)((dtMinRequired - dtBegin).GetTotalMinutes()) / (long)GetActiveInterval();
			}
			
		}
		long nMinLastSlotToPrint;
		{
			COleDateTime dtMaxRequired;
			dtMaxRequired.SetTime(17, 0, 0);
			dtEnd.ParseDateTime(m_pSingleDayCtrl.GetEndTime());
			COleDateTime dtFakeEndTime = dtEnd;
			while (dtFakeEndTime <= dtBegin) {
				dtFakeEndTime += COleDateTimeSpan(1, 0, 0, 0);
			}
			if (dtFakeEndTime < dtMaxRequired) {
				// Our last slot is already less than the official max, so end with the last slot
				nMinLastSlotToPrint = (long)((dtFakeEndTime - dtBegin).GetTotalMinutes()) / (long)GetActiveInterval();
			} else {
				// Our last slot is greater than the max, so start with the max, which is the number of slots between the begin time and the max allowed time
				nMinLastSlotToPrint = (long)((dtMaxRequired - dtBegin).GetTotalMinutes()) / (long)GetActiveInterval();
			}
			
		}
		if (slotMin == -1 || slotMin > nMaxFirstSlotToPrint) slotMin = nMaxFirstSlotToPrint;
		if (slotMax == -1 || slotMax < nMinLastSlotToPrint) slotMax = nMinLastSlotToPrint;
		long nCount = slotMax - slotMin + 1;

		if(nCount > m_pSingleDayCtrl.GetSlotTotalCount()) {
			//We don't want to print more slots than we actually have, so...
			slotMax = slotMin + m_pSingleDayCtrl.GetSlotTotalCount() - 1;
		}
		if(nFirstPrintSlot == -1 || nFirstPrintSlot > slotMin) nFirstPrintSlot = slotMin;
		if(nLastPrintSlot == -1 || nLastPrintSlot < slotMax) nLastPrintSlot = slotMax;
	}
}

// (c.haag 2010-03-26 15:07) - PLID 37332 - We now take in a singleday wrapper
// (c.haag 2010-04-30 12:19) - PLID 38379 - We now pass in the owner that wants it
CReservation SearchSingleDayForReservation(const CString& strOwner, IN CSingleDay sd, IN const long nReservationID)
{
	// Outer loop: Loop through all days in this control
	long nDayTotalCount = sd.GetDayTotalCount();
	for (long nDay=0; nDay<nDayTotalCount; nDay++) {
		// Inner loop: Loop through all reservations on this day of this control
		for (long i=0; ; i++) {
			// Get the reservation for this index
			CReservation pRes = sd.GetReservation(strOwner, nDay, i);
			// See if we have a reservation
			if (pRes != NULL) {
				// We have a reservation, so see if it's reservation id is the one being requested
				if (pRes.GetReservationID() == nReservationID) {
					// It's the one we've been searching for so return it
					return pRes;
				} else {
					// It's not the one, so let the searching continue
				}
			} else {
				// There are no more reservations on this day...this is how we know to break out 
				// of this loop and move on to the next day
				break;
			}
		}
	}

	// If we made it here then we know that all of the reservations on all of the days of this 
	// singleday control had reservation ids other than the one we were looking for.  In other 
	// words, our search is complete and the reservation was not found so return NULL
	return CReservation(strOwner, NULL);
}

// If pbIsInEventControl is given, it is always written to (unless an exception is thrown)
// (c.haag 2010-04-30 12:20) - PLID 38379 - We now pass in an owner
CReservation CNxSchedulerDlg::GetAppointmentReservation(const CString& strOwner, long nAppointmentID, OPTIONAL OUT BOOL *pbIsInEventControl)
{
	// First see if it's in the events control
	{
		// Search the events control
		CReservation pEventRes = SearchSingleDayForReservation(strOwner, m_pEventCtrl, nAppointmentID);
		if (pEventRes) {
			// Found the reservation in the event control, so let the caller know that's where it was found
			if (pbIsInEventControl) {
				*pbIsInEventControl = TRUE;
			}
			// And return the reservation
			return pEventRes;
		}
	}

	// Now see if it's in the normal singleday
	{
		// Search the normal singleday
		CReservation pRes = SearchSingleDayForReservation(strOwner, m_pSingleDayCtrl, nAppointmentID);
		if (pRes) {
			// Found the reservation in the normal singleday, so let the caller know it wasn't found in the events
			if (pbIsInEventControl) {
				*pbIsInEventControl = FALSE;
			}
			// And return the reservation
			return pRes;
		}
	}

	// If we made it here, the reservation wasn't found
	// So obviously it's not in the event control
	if (pbIsInEventControl) {
		*pbIsInEventControl = FALSE;
	}
	// And return failure
	return CReservation(strOwner, NULL);
}

void CNxSchedulerDlg::ScrollToMakeDayVisible(long nDayToMakeVisible)
{
	long nLeftDay = m_pSingleDayCtrl.GetLeftDay();
	if (nDayToMakeVisible < nLeftDay) {
		// Scroll the singledays to make the given day the left day
		m_pEventCtrl.PutLeftDay(nDayToMakeVisible);
		m_pSingleDayCtrl.PutLeftDay(nDayToMakeVisible);
		// Refresh the button labels
		EnsureButtonLabels();
	} else {
		long nRightDay = nLeftDay + m_pSingleDayCtrl.GetDayVisibleCount() - 1;
		if (nDayToMakeVisible > nRightDay) {
			// Scroll the singledays to make the given day just visible on the right
			long nNewLeftDay = nLeftDay + nDayToMakeVisible - nRightDay;
			m_pEventCtrl.PutLeftDay(nNewLeftDay);
			m_pSingleDayCtrl.PutLeftDay(nNewLeftDay);
			// Refresh the button labels
			EnsureButtonLabels();
		} else {
			// Nothing needs to be done here because it was already visible
		}
	}
}

// throws exceptions on any kind of failure
void CNxSchedulerDlg::ShowAppointment(long nAppointmentID, BOOL bShowResEntry, const IN OPTIONAL COleDateTime *pdtChangeDateTo, const IN OPTIONAL long *pnChangeResourceTo)
{
	// See if we need to change our view at all first
	if (pdtChangeDateTo || pnChangeResourceTo) {
		if(!AllowChangeView()) return;
		// (v.maida 2016-04-22 16:27) - PLID 51696 - Don't continue if we're not on the correct tab.
		if (m_pParent && this != m_pParent->GetActiveSheet()) {
			return;
		}
		// See if we need to change to another date
		if (pdtChangeDateTo) {
			// We need to change to another date
			SetActiveDate(*pdtChangeDateTo);
			m_bNeedUpdate = true; // I don't know why SetActiveDate() doesn't do this for us.
		}
		if (pnChangeResourceTo) {
			// We need to change to another resource (but don't update yet, because we're going to do it below)
			m_pParent->SetActiveResourceID(*pnChangeResourceTo, FALSE);
		}
		// We know we changed at least one of them so update
		m_pParent->UpdateView();
	}
	
	BOOL bApptIsInEventControl = FALSE;
	// (c.haag 2010-08-27 11:38) - PLID 40108 - "Reserve" the resentrydlg so that UpdateView
	// messages get ignored while the resentry object is being initialized.
	CReserveResEntry rre(m_ResEntry, __FUNCTION__);
	CReservation pRes = GetAppointmentReservation(__FUNCTION__, nAppointmentID, &bApptIsInEventControl);
	if (pRes) {
		// See whether the res is on the event control or the normal control
		if (!bApptIsInEventControl) {
			// It's in the normal control, so make sure it's scrolled to a point where the 
			// actual reservation can be seen by the user
			ScrollToMakeTimeRangeVisible(pRes.GetStartTime(), pRes.GetEndTime());
		}

		// Scroll horizontally if necessary (this applies whether the appointment is in the 
		// normal singleday control or event singleday control)
		ScrollToMakeDayVisible(pRes.GetDay());
		
		// If requested, call zoomres on the appointment to show the resentry screen
		if (bShowResEntry) {
			ASSERT(m_pParent && m_ResEntry);
			CWaitCursor wc;
			m_ResEntry->ZoomResExistingAppointment(pRes);
		}

		// We've done what we've been asked
	} else {
		// We couldn't show the appointment
		ThrowNxException("CNxSchedulerDlg::ShowAppointment(%li, %s, %li, %li) Could not find appointment in current view!", 
			nAppointmentID, bShowResEntry ? "true" : "false", pdtChangeDateTo, pnChangeResourceTo);
	}
}

void CNxSchedulerDlg::InvalidateData()
{
	// Invalidate
	m_bNeedUpdate = true;
	
	// Post the update message
	MSG msg;
	// (a.walling 2010-12-17 14:49) - PLID 40444 - The modules popup makes it much easier to introduce a problem here
	// since this will also dispatch any sent messages. So I'll fix it to only deal with posted messages.
	if (m_hWnd != NULL && this == m_pParent->GetActiveSheet() && !PeekMessage(&msg, m_hWnd, NXM_UPDATEVIEW, NXM_UPDATEVIEW, PM_NOREMOVE | PM_QS_POSTMESSAGE | PM_NOYIELD)) {
		// Need to post a message to refresh (note: In the case of NxSchedulerDlg, if someone explicitly 
		// calls UpdateView on us before the message is pumped, then the UpdateView will remove the 
		// message from the queue so it won't be called again; also m_bNeedUpdate probably won't be true 
		// anymore so it wouldn't matter anyway)
		PostMessage(NXM_UPDATEVIEW);
	}
}

void CNxSchedulerDlg::OnSelChangingResources(long FAR* nNewSel) 
{
	if (*nNewSel == sriNoRow) {
		*nNewSel = m_dlResources->GetCurSel();
	}
}

//TES 7/1/2003: TODO: At some point, this should use a method of the singleday, and that method should 
//not loop.
void CNxSchedulerDlg::EnsureCountLabelText()
{
	try {
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
					// (c.haag 2006-11-29 17:45) - PLID 23666 - Don't count template blocks
					// (c.haag 2009-12-22 17:34) - PLID 28977 - Factor in type and person ID
					// (j.luckoski 2012-05-08 16:18) - PLID 50235 - Only count uncancelled appts
					// (j.luckoski 2012-05-09 16:07) - PLID 50264 - User ReturnsRecordsParam
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
		strCountLabelText.Format("%i Appt(s).", nApptCount);
		// (c.haag 2009-12-22 13:49) - PLID 28977 - Now use NxLabels
		((CNxLabel*)GetDlgItem(IDC_ACTIVE_DATE_APTCOUNT_LABEL))->SetText(strCountLabelText);
		// (a.walling 2008-09-22 13:26) - PLID 31445 - This is erasing the label, everything we need
		// is already handled within CNxStatic's SetText handler.
		//InvalidateDlgItem(IDC_ACTIVE_DATE_APTCOUNT_LABEL);
	}	NxCatchAll(__FUNCTION__);
}

bool CNxSchedulerDlg::AllowChangeView()
{
	if(!m_ResEntry || !m_ResEntry->IsWindowVisible()) return true;
	else return m_ResEntry->AllowChangeView();
}

CString CNxSchedulerDlg::GetExtraFilter()
{
	// (j.jones 2011-02-11 09:37) - PLID 35180 - replaced references to ResExtendedQ with ResExQ
	// (a.walling 2013-06-18 11:31) - PLID 57204 - replaced references to ResExQ with underlying table names

	CString strExtraFilter;
	long nSetId = GetPurposeSetId();
	if (nSetId != -1) {

		if(nSetId == -2) {
			//multiple types are selected
			CString strIds = m_strMultiTypeIds;
			CString strList = "";
			long nComma = strIds.Find(",");
			while(nComma > -1) {
				long nID = atoi(strIds.Left(nComma));
				CString str;
				str.Format("OR AppointmentsT.AptTypeID = %li ", nID);
				strList += str;
				strIds = strIds.Right(strIds.GetLength() - nComma - 1);
				nComma = strIds.Find(",");
			}

			if(!strList.IsEmpty()) {
				strList = strList.Right(strList.GetLength() - 3);	//take off the opening or

				CString str;
				str.Format(" AND AppointmentsT.ID IN (SELECT ID FROM AppointmentsT WHERE %s) ", strList);
				strExtraFilter += str;
			}
			else {
				//there are no items in the list, but it's selected... this shouldn't be possible
				ASSERT(FALSE);
			}
		}
		else {
			// An actual set is selected so choose only those appointments that are in that set
			strExtraFilter.Format("AND (AppointmentsT.AptTypeID = %li)", nSetId);
		}
	}

	long nPurpId = GetPurposeId();
	if (nPurpId != -1)
	{
		if(nPurpId == -2) {
			//DRT 8/5/03 - multiple purposes filter
			CString strIds = m_strMultiPurposeIds;
			CString strList = "";
			long nComma = strIds.Find(",");
			while(nComma > -1) {
				long nID = atoi(strIds.Left(nComma));
				CString str;
				str.Format("OR AppointmentPurposeT.PurposeID = %li ", nID);
				strList += str;
				strIds = strIds.Right(strIds.GetLength() - nComma - 1);
				nComma = strIds.Find(",");
			}

			if(!strList.IsEmpty()) {
				strList = strList.Right(strList.GetLength() - 3);	//take off the opening or

				CString str;
				str.Format(" AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT WHERE %s) ", strList);
				strExtraFilter += str;
			}
			else {
				//there are no items in the list, but it's selected... this shouldn't be possible
				ASSERT(FALSE);
			}
		}
		else {
			CString str;
			str.Format(" AND EXISTS (SELECT * FROM AppointmentPurposeT WHERE AppointmentPurposeT.AppointmentID = AppointmentsT.ID AND AppointmentPurposeT.PurposeID = %li)", nPurpId);
			strExtraFilter += str;
		}
	}
	//TES 6/21/2010 - PLID 21341 - Add the Location filter.
	long nLocationId = GetLocationId();
	if (nLocationId != -1) {
		// (d.singleton 2012-06-07 11:07) - PLID 47473
		if(nLocationId == -2) {
			CString strIds = m_strMultiLocationIds;
			CString strList = "";
			long nComma = strIds.Find(",");
			while(nComma > -1) {
				long nID = atoi(strIds.Left(nComma));
				CString str;
				str.Format("OR AppointmentsT.LocationID = %li ", nID);
				strList += str;
				strIds = strIds.Right(strIds.GetLength() - nComma - 1);
				nComma = strIds.Find(",");
			}

			if(!strList.IsEmpty()) {
				strList = strList.Right(strList.GetLength() - 3);	//take off the opening or

				CString str;
				str.Format(" AND AppointmentsT.ID IN (SELECT ID FROM AppointmentsT WHERE %s) ", strList);
				strExtraFilter += str;
			}
			else {
				//there are no items in the list, but it's selected... this shouldn't be possible
	 			//ASSERT(FALSE);
			}
		}
		else {
			// An actual location is selected so choose only those appointments that are at that location
			CString str;
			str.Format(" AND (AppointmentsT.LocationID = %li) ", nLocationId);
			strExtraFilter += str;
		}
	}

	// (a.walling 2013-06-18 11:31) - PLID 57204 - Moved cancelled appointment filtering options into this common area
	// (j.luckoski 2012-04-26 10:10) - PLID 11597 - Set up date range for showing cancelled appts.
	long nCancelledAppt = GetRemotePropertyInt("ShowCancelledAppointment", 0, 0, GetCurrentUserName(), true);
	// (j.luckoski 2012-06-12 15:58) - PLID 11597 - Did not default to 24
	long nDateRange = GetRemotePropertyInt("CancelledDateRange", 24, 0, GetCurrentUserName(), true);

	if(m_nCancelledAppt == 1) {
		strExtraFilter.AppendFormat("  AND (AppointmentsT.CancelledDate IS NULL OR (AppointmentsT.CancelledDate >= DATEADD(hh, -%li, AppointmentsT.StartTime))) ", nDateRange);
	} else {
		strExtraFilter.Append(" AND (AppointmentsT.Status <> 4)");
	}

	return strExtraFilter;
}

// (a.walling 2013-05-31 13:31) - PLID 56961 - Parameterized version of GetExtraFilter; all fields available in GetResSimpleSql
CSqlFragment CNxSchedulerDlg::GetExtraResSimpleFilter()
{
	CSqlFragment filter;

	// types
	{
		long nSetId = GetPurposeSetId();

		if (nSetId != -1) {
			if (nSetId != -2) {
				filter += CSqlFragment(" AND ResExQ.AptTypeID = {INT} ", nSetId);
			} else {
				// multiple
				filter += CSqlFragment(" AND ResExQ.AptTypeID IN ({INTSTRING}) ", m_strMultiTypeIds);
			}
		}
	}

	// purposes
	{
		long nPurpId = GetPurposeId();
		if (nPurpId != -1) {
			if (nPurpId != -2) {
				filter += CSqlFragment(" AND ResExQ.AptPurposeID = {INT} ", nPurpId);
			} else {
				// multiple
				filter += CSqlFragment(" AND ResExQ.AptPurposeID IN ({INTSTRING}) ", m_strMultiPurposeIds);
			}
		}
	}

	// locations
	{
		long nLocationId = GetLocationId();
		if (nLocationId != -1) {
			if (nLocationId != -2) {
				filter += CSqlFragment(" AND ResExQ.LocationID = {INT} ", nLocationId);
			} else {
				// multiple
				filter += CSqlFragment(" AND ResExQ.LocationID IN ({INTSTRING}) ", m_strMultiLocationIds);
			}
		}
	}

	return filter;
}

void CNxSchedulerDlg::OnTimer(UINT nIDEvent) 
{
	if (IDT_UPDATE_SCHEDULER_VIEW == nIDEvent) {
		KillTimer(nIDEvent);	

		// (c.haag 2007-03-15 17:30) - PLID 24514 - Restore the read/write states of the single day controls
		// and reset the waiting flag
		m_bIsWaitingForTimedUpdate = FALSE;
		if (NULL != m_pSingleDayCtrl) { m_pSingleDayCtrl.PutReadOnly(VARIANT_FALSE); }
		if (NULL != m_pEventCtrl) { m_pEventCtrl.PutReadOnly(VARIANT_FALSE); }		

		// (j.gruber 2009-04-14 12:33) - PLID 27061 - check to see if we are the active tab before updating
		if ((const CNxDialog *)this == m_pParent->GetActiveSheet()) {
			UpdateView();
		}		
	}
}

void CNxSchedulerDlg::SetScheduleVisibleTimeRange()
{
	// (z.manning, 03/13/2007) - PLID 23635 - Moved the "base" code for this to GlobalSchedUtils so the
	// template editor can use it as well;
	COleDateTime dtStart, dtEnd;
	GetScheduleVisibleTimeRange(m_pParent->m_lohOfficeHours, GetActiveDate().GetDayOfWeek(), dtStart, dtEnd);

	// (z.manning, 03/13/2007) - PLID 5812 - If we're changing the time range, we'll need to trigger a full update.
	COleDateTime dtOldStart, dtOldEnd;
	dtOldStart.ParseDateTime(m_pSingleDayCtrl.GetBeginTime(), VAR_TIMEVALUEONLY);
	dtOldEnd.ParseDateTime(m_pSingleDayCtrl.GetEndTime(), VAR_TIMEVALUEONLY);
	if( !(CompareTimes(dtStart,dtOldStart) & CT_EQUAL) || !(CompareTimes(dtEnd,dtOldEnd) & CT_EQUAL) ) {
		m_bNeedUpdate = true;
		m_pSingleDayCtrl.PutBeginTime(_bstr_t(dtStart.Format(VAR_TIMEVALUEONLY)));
		m_pSingleDayCtrl.PutEndTime(_bstr_t(dtEnd.Format(VAR_TIMEVALUEONLY)));
		// (j.jones 2014-12-03 15:55) - PLID 64276 - clear our colored times
		m_aryColoredTimes.clear();
		// (j.jones 2015-01-08 09:54) - PLID 64276 - recolor the times on the new schedule view
		// so that all the colors are wiped
		ResetTimeColors(false);
	}
}

BOOL CNxSchedulerDlg::NeedToUpdateVisibleTimeRange(COleDateTime dtApptStart, COleDateTime dtApptEnd)
{
	// (z.manning, 10/12/2006) - PLID 5812 - If the given times fall within the visible time range of the
	// scheduler, return false, otherwise true.

	COleDateTime dtMidnight, dtTimeRangeStart, dtTimeRangeEnd, dtStartTime, dtEndTime;

	dtMidnight.SetTime(0,0,0);

	dtTimeRangeStart.ParseDateTime(m_pSingleDayCtrl.GetBeginTime(), VAR_TIMEVALUEONLY);
	dtTimeRangeEnd.ParseDateTime(m_pSingleDayCtrl.GetEndTime(), VAR_TIMEVALUEONLY);

	// We only want to deal with the times.
	dtApptStart.SetTime(dtApptStart.GetHour(), dtApptStart.GetMinute(), dtApptStart.GetSecond());
	dtApptEnd.SetTime(dtApptEnd.GetHour(), dtApptEnd.GetMinute(), dtApptEnd.GetSecond());

	if( (CompareTimes(dtApptStart,dtTimeRangeStart) & CT_LESS_THAN)
		|| ((CompareTimes(dtApptEnd,dtTimeRangeEnd) & CT_GREATER_THAN) && !(CompareTimes(dtTimeRangeEnd,dtMidnight) & CT_EQUAL)) ) 
	{
		return TRUE;
	}

	return FALSE;
}

// (z.manning, 05/10/2007) - PLID 11593 - Update the reservation text only.
void CNxSchedulerDlg::UpdateReservationText()
{
	try
	{
		// (z.manning, 05/10/2007) - PLID 11593 - Load the entire reservation set for the current view so we
		// only have to access data once.
		CReservationReadSet rs;
		rs.m_pSchedView = m_pParent;
		// (a.wilson 2012-06-14 16:18) - PLID 47966 - change for new preference type.
		long nPropColorApptTextWithStatus = GetRemotePropertyInt("ColorApptTextWithStatus", GetPropertyInt("ColorApptTextWithStatus", 0, 0, false), 0, GetCurrentUserName(), true);
		if( ReFilter(rs, NeedUpdate()) )
		{
			// (z.manning, 05/10/2007) - PLID 11593 - Now go through each appointment in the set and try
			// to match it with an appointment on one of the single day controls, and then update the text
			// when we find it.
			for (; !rs.IsEOF(); rs.MoveNext()) 
			{
				CString strMultiPurpose = GetMultiPurposeStringFromCurrentReservationReadSetPosition(rs);
				long nResID = rs.GetID();
				
				// (j.luckoski 2012-07-02 10:03) - PLID 11597 - Set bool for cancelled appts for use below
				bool bIsResCancelled = false;
				// (a.walling 2013-01-21 16:48) - PLID 54744 - Available in ReadSet
				if(rs.GetCancelledDate().GetStatus() == COleDateTime::valid) {
					bIsResCancelled = true;
				}

				// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
				bool bIsMixRuleOverridden = rs.GetIsMixRuleOverridden();

				BOOL bFound = FALSE;
				if(m_pSingleDayCtrl != NULL) {
					const long nDays = m_pSingleDayCtrl.GetDayTotalCount();
					for(short nDay = 0; nDay < nDays && !bFound; nDay++) {
						const long nReservations = m_pSingleDayCtrl.GetReservationCount(nDay);
						for(long i = 0; i < nReservations && !bFound; i++) {
							CReservation pRes = m_pSingleDayCtrl.GetReservation(__FUNCTION__, nDay, i);
							// (j.luckoski 2012-05-07 11:32) - PLID 11597 - If cancelled show it
							if(pRes != NULL && nResID == pRes.GetReservationID()) {
								// (j.luckoski 2012-05-09 14:42) - PLID 50264 - Use ReturnsRecordsParam
								// Set the appropriate text for the appointment
								// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
								pRes.PutText((_bstr_t)rs.GetBoxText(strMultiPurpose, nPropColorApptTextWithStatus, bIsResCancelled, bIsMixRuleOverridden));
								if(!bIsResCancelled) {
									// (j.luckoski 2012-06-20 14:45) - PLID 11597 - Return putallowdrag back to true
									pRes.PutAllowDrag(TRUE);
									pRes.PutAllowResize(TRUE);
								} else {
									// (j.luckoski 2012-06-20 11:16) - PLID 11597 - Prohibit dragging and resizing on cancelled apts.
									pRes.PutAllowDrag(VARIANT_FALSE);
									pRes.PutAllowResize(VARIANT_FALSE);
								}

								// (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
								if (rs.GetCancelledDate().GetStatus() == COleDateTime::valid) {
									pRes.Data["CancelledDate"] = _variant_t(rs.GetCancelledDate(), VT_DATE);
								} else {									
									pRes.Data["CancelledDate"] = g_cvarNull;
								}

								bFound = TRUE;
							}
						}
					}
				}
				// (z.manning, 05/10/2007) - PLID 11593 - Only search through events if we've not yet found
				// the current reservation ID.
				if(!bFound && m_pEventCtrl != NULL) {
					const long nDays = m_pEventCtrl.GetDayTotalCount();
					for(short nDay=0; nDay < nDays && !bFound; nDay++) {
						const long nReservations = m_pEventCtrl.GetReservationCount(nDay);
						for(long i = 0; i < nReservations && !bFound; i++) {
							CReservation pRes = m_pEventCtrl.GetReservation(__FUNCTION__, nDay, i);
							// (j.luckoski 2012-05-07 11:32) - PLID 11597 - If cancelled, show it on appt text.
							if(pRes != NULL && nResID == pRes.GetReservationID()) {
								// (j.luckoski 2012-05-09 14:42) - PLID 50264 - Use ReturnsRecordsParam
								// Set the appropriate text for the appointment
								// (j.jones 2014-12-03 10:58) - PLID 64274 - added bIsMixRuleOverridden
								pRes.PutText((_bstr_t)rs.GetBoxText(strMultiPurpose, nPropColorApptTextWithStatus, bIsResCancelled, bIsMixRuleOverridden));
								if(!bIsResCancelled) {
									// (j.luckoski 2012-06-20 14:45) - PLID 11597 - Return putallowdrag back to true
									pRes.PutAllowDrag(TRUE);
									pRes.PutAllowResize(TRUE);
								} else {
									// (j.luckoski 2012-06-20 11:17) - PLID 11597 - Prohibit dragging and resizing of cancelled apts
									pRes.PutAllowDrag(VARIANT_FALSE);
									pRes.PutAllowResize(VARIANT_FALSE);
								}
								bFound = TRUE;

								// (a.walling 2013-01-21 16:48) - PLID 54747 - Populate CancelledDate property for scheduler reservations
								if (rs.GetCancelledDate().GetStatus() == COleDateTime::valid) {
									pRes.Data["CancelledDate"] = _variant_t(rs.GetCancelledDate(), VT_DATE);
								} else {									
									pRes.Data["CancelledDate"] = g_cvarNull;
								}
							}
						}
					}		
				}
			}
		}
		else
		{
			ASSERT(FALSE);
		}

	}NxCatchAll("CNxSchedulerDlg::UpdateReservationText");
}

// (z.manning, 02/13/2008) - PLID 28909 - Function to update template reservations.
void CNxSchedulerDlg::UpdateAttendanceReservations()
{
	// (z.manning, 02/14/2008) - PLID 28909 - Here is the base query to load all attendance appointments.
	// (z.manning 2008-10-09 17:33) - PLID 31645 - Also use ResourceUserLinkT as a way to link
	// attendance appointments to resources.
	// (z.manning 2008-11-13 12:50) - PLID 31831 - Added paid, unpaid
	// (z.manning 2010-07-30 12:05) - PLID 39916 - Removed notes from this query as we no longer show them in the scheduler
	CString strBaseSql = FormatString(
		"SELECT DISTINCT AttendanceAppointmentsT.ID AS AttendanceID, AttendanceAppointmentsT.Date \r\n"
		"	, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS UserName \r\n"
		"	, ResourceT.ID AS ResourceID, Vacation, Sick, Other \r\n"
		"	, AptTypeT.Color AS AptTypeColor, PaidTimeOff, UnpaidTimeOff \r\n"
		"FROM AttendanceAppointmentsT \r\n"
		"LEFT JOIN PersonT ON AttendanceAppointmentsT.UserID = PersonT.ID \r\n"
		"LEFT JOIN UserDepartmentLinkT ON AttendanceAppointmentsT.UserID = UserDepartmentLinkT.UserID \r\n"
		"LEFT JOIN DepartmentResourceLinkT ON UserDepartmentLinkT.DepartmentID = DepartmentResourceLinkT.DepartmentID \r\n"
		"LEFT JOIN ResourceUserLinkT ON AttendanceAppointmentsT.UserID = ResourceUserLinkT.UserID \r\n"
		"INNER JOIN ResourceT ON DepartmentResourceLinkT.ResourceID = ResourceT.ID OR ResourceUserLinkT.ResourceID = ResourceT.ID \r\n"
		"LEFT JOIN AptTypeT ON dbo.GetAttendanceAppointmentTypeID(AttendanceAppointmentsT.ID) = AptTypeT.ID \r\n"
		"WHERE (Vacation > 0 OR Sick > 0 OR Other > 0 OR PaidTimeOff > 0 OR UnpaidTimeOff > 0) %s \r\n"
		, GetExtraFilterAttendance()
		);

	// (z.manning, 02/14/2008) - PLID 28909 - Now call this virtual function to filter on only the appointments
	// that are applicable to the current tab the user is on.
	_RecordsetPtr prs = GetAttendanceResRecordsetFromBaseQuery(strBaseSql);

	for(; !prs->eof; prs->MoveNext())
	{
		FieldsPtr pflds = prs->Fields;
		BOOL bResAdded = FALSE;
		for(int nCol = 0; nCol < m_pEventCtrl.GetDayTotalCount() && !bResAdded; nCol++)
		{
			long nWorkingResourceID;
			COleDateTime dtWorkingDate;
			m_pParent->GetWorkingResourceAndDate(this, nCol, nWorkingResourceID, dtWorkingDate);

			long nResourceID = AdoFldLong(pflds, "ResourceID");
			COleDateTime dtDate = AdoFldDateTime(pflds, "Date");

			// (z.manning, 9/2/2008) - PLID 31162 - Make sure we working with dates only here.
			dtWorkingDate.SetDate(dtWorkingDate.GetYear(), dtWorkingDate.GetMonth(), dtWorkingDate.GetDay());
			dtDate.SetDate(dtDate.GetYear(), dtDate.GetMonth(), dtDate.GetDay());

			// (z.manning, 02/14/2008) - Check and see if this appointment belongs on this column and if not,
			// simply go on to the next column.
			if(nResourceID != nWorkingResourceID || dtDate != dtWorkingDate) {
				continue;
			}
			bResAdded = TRUE;

			// (z.manning, 02/14/2008) - Ok, we found the right column for this appt, so add it as an event.
			COleDateTime dtMidnight;
			dtMidnight.SetTime(0,0,0);
			CReservation pRes(m_pEventCtrl.AddReservation(__FUNCTION__, nCol, dtMidnight, dtMidnight, AdoFldLong(pflds, "AptTypeColor", RGB(255,0,0)), TRUE));

			// (z.manning, 02/14/2008) - Now prepare the text to go into the res box. Start with the name bolded.
			CString strResText = "%1b" + AdoFldString(pflds, "UserName", "") + "%0b - ";
			// (z.manning, 02/14/2008) - Now add any vacation, sick, or other hours.
			COleCurrency cyVacation = ConvertDecimalToCurrency(AdoFldDecimal(pflds, "Vacation"));
			COleCurrency cySick = ConvertDecimalToCurrency(AdoFldDecimal(pflds, "Sick"));
			// (z.manning 2008-11-13 12:51) - PLID 31831 - Paid/Unpaid
			COleCurrency cyPaid = ConvertDecimalToCurrency(AdoFldDecimal(pflds, "PaidTimeOff"));
			COleCurrency cyUnpaid = ConvertDecimalToCurrency(AdoFldDecimal(pflds, "UnpaidTimeOff"));
			COleCurrency cyOther = ConvertDecimalToCurrency(AdoFldDecimal(pflds, "Other"));
			COleCurrency cyZero(0, 0);
			if(cyVacation > cyZero) {
				// (d.thompson 2014-02-27) - PLID 61016 - Renamed vacation to PTO
				strResText += FormatString("PTO (%s hours), ", FormatAttendanceValue(cyVacation));
			}
			if(cySick > cyZero) {
				strResText += FormatString("Sick (%s hours), ", FormatAttendanceValue(cySick));
			}
			// (z.manning 2008-11-13 12:52) - PLID 31831 - Paid/Unpaid
			if(cyPaid > cyZero) {
				strResText += FormatString("Paid time off (%s hours), ", FormatAttendanceValue(cyPaid));
			}
			if(cyUnpaid > cyZero) {
				strResText += FormatString("Unpaid time off (%s hours), ", FormatAttendanceValue(cyUnpaid));
			}
			if(cyOther > cyZero) {
				strResText += FormatString("Other (%s hours), ", FormatAttendanceValue(cyOther));
			}
			strResText.TrimRight(", ");

			pRes.PutText(_bstr_t(strResText));
			pRes.PutAllowDrag(VARIANT_FALSE);
			pRes.PutAllowResize(VARIANT_FALSE);
		}

		// (z.manning, 02/14/2008) - PLID 28909 - If this assertion fires it means we loaded a reservation
		// from data but never actually added it to the single day. Most likely, we are loading an appt
		// that falls outside the filter of the current schedule tab in which case the query should be fixed
		// to not load unnecessary appointments.
		ASSERT(bResAdded);
	}
}

// (z.manning, 02/13/2008) - PLID 28909 - Function to return the open recordset to load attendance appointments.
_RecordsetPtr CNxSchedulerDlg::GetAttendanceResRecordsetFromBaseQuery(CString strBaseQuery)
{
	return CreateParamRecordset(strBaseQuery + 
		" AND ResourceT.ID = {INT} AND AttendanceAppointmentsT.Date = {OLEDATETIME} "
		, m_pParent->GetActiveResourceID(), GetActiveDate());
}

CString CNxSchedulerDlg::GetExtraFilterAttendance()
{	
	CString strTypePurposeFilter;
	if(GetPurposeSetId() == -2) {
		// (z.manning, 02/14/2008) - Multiple types.
		if(!m_strMultiTypeIds.IsEmpty()) {
			CString strTypeIDs = m_strMultiTypeIds;
			strTypeIDs.TrimRight(',');
			strTypePurposeFilter.Format(" AND (AptTypeT.ID IN (%s)) ", strTypeIDs);
		}
	}
	else if(GetPurposeSetId() > 0) {
		strTypePurposeFilter.Format(" AND (AptTypeT.ID = %li) ", GetPurposeSetId());
	}

	// (z.manning, 02/14/2008) - PLID 28909 - For the time being, all attendance appointments are considered
	// to not have a purpose, so if we have a purpose filter set then nothing to do here.
	if(GetPurposeId() != -1) {
		strTypePurposeFilter += " AND (1 = 0) ";
	}

	return strTypePurposeFilter;
}

void CNxSchedulerDlg::EnableSingleDayControls()
{
	//TES 12/18/2008 - PLID 32497 - Default behavior, we always want to enable them (derived classes may feel differently).
	m_pEventCtrl.PutEnabled(VARIANT_TRUE);
	m_pSingleDayCtrl.PutEnabled(VARIANT_TRUE);
}

void CNxSchedulerDlg::OnSelChangingPurposeSetFilter(long FAR* nNewSel)
{
	try {
		if(*nNewSel != m_dlTypeFilter->CurSel && !g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse,"Filtering the Schedule by appointment type and purpose", "The_Scheduler_Module/Filter_the_Schedule_by_Appointment_Type.htm")) {
			//TES 12/18/2008 - PLID 32508 - This is not available for Scheduler Standard users.
			*nNewSel = m_dlTypeFilter->CurSel;
			m_dlTypeFilter->DropDownState = VARIANT_FALSE;
		}

	}NxCatchAll("Error in CNxSchedulerDlg::OnSelChangingPurposeSetFilter()");
}

void CNxSchedulerDlg::OnSelChangingPurposeFilter(long FAR* nNewSel)
{
	try {
		if(*nNewSel != m_dlPurposeFilter->CurSel && !g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse,"Filtering the Schedule by appointment type and purpose", "The_Scheduler_Module/Filter_the_Schedule_by_Appointment_Type.htm")) {
			//TES 12/18/2008 - PLID 32508 - This is not available for Scheduler Standard users.
			*nNewSel = m_dlPurposeFilter->CurSel;
			m_dlPurposeFilter->DropDownState = VARIANT_FALSE;
		}

	}NxCatchAll("Error in CNxSchedulerDlg::OnSelChangingPurposeFilter()");
}

void CNxSchedulerDlg::OnSelChangingLocationFilter(long FAR* nNewSel)
{
	try {
		//TES 6/21/2010 - PLID 21341 - Location filtering isn't allowed on scheduler standard.
		if(*nNewSel != m_dlLocationFilter->CurSel && !g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse,"Filtering the Schedule by appointment location")) {
			*nNewSel = m_dlLocationFilter->CurSel;
			m_dlLocationFilter->DropDownState = VARIANT_FALSE;
		}

	}NxCatchAll("Error in CNxSchedulerDlg::OnSelChangingLocationFilter()");
}

// (a.walling 2010-06-23 09:41) - PLID 39263 - Update available location info
void CNxSchedulerDlg::UpdateColumnAvailLocationsDisplay()
{
	// nothing
}

// (a.walling 2010-06-23 09:41) - PLID 39263
CNxSchedulerDlg::CResourceAvailLocations::~CResourceAvailLocations()
{
	ClearAll();
}

// (a.walling 2010-06-23 09:41) - PLID 39263
void CNxSchedulerDlg::CResourceAvailLocations::ClearAll()
{
	//TES 8/3/2010 - PLID 39264 - Renamed
	POSITION pos = m_mapLocationTemplateItems.GetStartPosition();

	while (pos) {
		long nResourceID = 0;
		CList<LocationTemplateItem>* plistLocations = NULL;

		m_mapLocationTemplateItems.GetNextAssoc(pos, nResourceID, plistLocations);

		delete plistLocations;
	}

	m_mapLocationTemplateItems.RemoveAll();

	//TES 9/7/2010 - PLID 39630 - Also clean up m_mapExcludedTimes;
	pos = m_mapExcludedTimes.GetStartPosition();
	while (pos) {
		long nResourceID = 0;
		CComplexTimeRange *pRange = NULL;
		m_mapExcludedTimes.GetNextAssoc(pos, nResourceID, pRange);
		delete pRange;
	}
	m_mapExcludedTimes.RemoveAll();
}

// (a.walling 2010-06-23 09:41) - PLID 39263
void CNxSchedulerDlg::CResourceAvailLocations::ClearForDay(long nDayIndex)
{
	//TES 8/3/2010 - PLID 39264 - We track LocationTemplateItems instead of LocationIDs now.
	CList<LocationTemplateItem>* plistLocations = NULL;
	if (m_mapLocationTemplateItems.Lookup(nDayIndex, plistLocations)) {	
		delete plistLocations;
		m_mapLocationTemplateItems.RemoveKey(nDayIndex);

		TRACE("\tDay %li:\tCleared locations list!\n", nDayIndex);
	}

	//TES 9/7/2010 - PLID 39630 - Also clean up m_mapExcludedTimes;
	CComplexTimeRange *pRange = NULL;
	if(m_mapExcludedTimes.Lookup(nDayIndex, pRange)) {
		delete pRange;
		m_mapExcludedTimes.RemoveKey(nDayIndex);
	}
}

// (a.walling 2010-06-23 09:41) - PLID 39263 - Keep track of the location IDs assigned for each resource. This will handle duplicates.
void CNxSchedulerDlg::CResourceAvailLocations::HandleTemplateItem(long nDayIndex, CTemplateHitSet& templateHitSet)
{
	long nLocationID = templateHitSet.GetLocationID();

	if (nLocationID == -1) {
		return;
	}

	//TES 9/7/2010 - PLID 39630 - We may have a non-contiguous range for this item, so set up an array of contiguous
	// segments.
	CArray<TimeRange,TimeRange&> arTimes;
	TimeRange tr;
	tr.dtStart = templateHitSet.GetStartTime();
	tr.dtEnd = templateHitSet.GetEndTime();
	arTimes.Add(tr);

	//TES 9/7/2010 - PLID 39630 - Now, take out the range (if any) that we've been told to exclude (this is the part that 
	// may result in discontinuity).
	//NOTE: This code relies upon the fact that location templates are always returned last in rsTemplateHits, meaning
	// that if we hit a location template, we must necessarily have already processed all regular templates, and
	// have therefore calculated the full range that excludes location templates.
	CComplexTimeRange * prangeExcluded = m_mapExcludedTimes[nDayIndex];
	if(prangeExcluded) {
		prangeExcluded->RemoveFromRanges(arTimes);
	}
	for(int i = 0; i < arTimes.GetSize(); i++) {
		//TES 8/3/2010 - PLID 39264 - We now track more information than just the LocationID.
		LocationTemplateItem lti;
		lti.nLocationID = nLocationID;
		//TES 9/7/2010 - PLID 39630 - Use the time from this segment, not templateHitSet.
		lti.dtStart = arTimes[i].dtStart;
		lti.dtEnd = arTimes[i].dtEnd;
		lti.nPriority = templateHitSet.GetPriority();
		// (r.gonet 05/25/2012) - PLID 49059  - Save the color
		lti.nColor = templateHitSet.GetColor();

		CList<LocationTemplateItem>*& plistLocations = m_mapLocationTemplateItems[nDayIndex];

		if (!plistLocations) {
			plistLocations = new CList<LocationTemplateItem>();

			CList<LocationTemplateItem>*& plistLocations2 = m_mapLocationTemplateItems[nDayIndex];
			ASSERT(plistLocations2 == plistLocations);
		}

		plistLocations->AddTail(lti);

		//TES 8/3/2010 - PLID 39264 - Added all the information we're now tracking.
		TRACE("\tDay %li:\tAdded location template item %li (%s - %s, Priority %li)\n", nDayIndex, 
			lti.nLocationID, lti.dtStart.Format(), lti.dtEnd.Format(), lti.nPriority);
	}
}

CString FormatDateTimeRangeForInterface(const COleDateTime& dtStart, const COleDateTime& dtEnd)
{
	const COleDateTime* pdtStart = &dtStart;
	const COleDateTime* pdtEnd = &dtEnd;

	if (dtEnd < dtStart) {
		pdtStart = &dtEnd;
		pdtEnd = &dtStart;
	}

	if ((pdtEnd->m_dt - pdtStart->m_dt) >= 1.0) {
		return FormatString("%s - %s", FormatDateTimeForInterface(*pdtStart, DTF_STRIP_SECONDS|DTF_STRIP_YEARS), FormatDateTimeForInterface(*pdtEnd, DTF_STRIP_SECONDS|DTF_STRIP_YEARS));
	}

	return FormatString("%s - %s", FormatDateTimeForInterface(*pdtStart, DTF_STRIP_SECONDS, dtoTime), FormatDateTimeForInterface(*pdtEnd, DTF_STRIP_SECONDS, dtoTime));
}

// (j.luckoski 2012-06-29 08:41) - PLID 48556 - Moduled this into its own function. It formats it for the system by converting it to a string using
// heavy abbreviations in order make it fit on the scheduler. 
CString FormatDateTimeForDayLabel(const COleDateTime& dtDate) 
{
	// (j.luckoski 2012-06-29 09:36) - PLID 48556 -Adjust for interface and locale settings then process heavy abbreviation.
	CString strFormattedTime = FormatDateTimeForInterface(dtDate, DTF_STRIP_SECONDS, dtoTime);
	CString strDate = strFormattedTime.TrimLeft(_T("0"));
	strDate.Replace(":00","");
	strDate.Replace("AM","");
	strDate.Replace("PM","");
	strDate.Replace(" ","");
	return strDate;
}

// (j.luckoski 2012-05-14 14:50) - PLID 48556 - Based off of FormatDateTimeRangeForInterface, but extremely abbreviated for
// easier readibility on day label.
CString FormatDateTimeRangeForDayLabel(const COleDateTime& dtStart, const COleDateTime& dtEnd)
{
	const COleDateTime* pdtStart = &dtStart;
	const COleDateTime* pdtEnd = &dtEnd;
	
	if (dtEnd < dtStart) {
		pdtStart = &dtEnd;
		pdtEnd = &dtStart;
		
	}

	// (j.luckoski 2012-05-14 14:50) - PLID 48556 - Remove leading zeros and if an even hour, remove trailing zeros
	// (j.luckoski 2012-06-28 12:10) - PLID 48556 - Removed casting of LPCTSTR as it was wrong to do so.
	COleDateTime dtStartTime = *pdtStart;
	COleDateTime dtEndTime = *pdtEnd;
	
	// (j.luckoski 2012-05-14 14:51) - PLID 48556 - Also removes spaces between the time and dash for space saving design
	// (j.luckoski 2012-06-29 08:45) - PLID 48556 - Call a seperate function to reduce code redundancy.
	return FormatString(_T("%s-%s"), FormatDateTimeForDayLabel(dtStartTime), FormatDateTimeForDayLabel(dtEndTime));
}

CString CNxSchedulerDlg::CResourceAvailLocations::GetDayDescription(long nDayIndex)
{
	CString strInfo;

	CList<LocationTemplateItem>* plistLocations = NULL;
	CList<long> listLocationIDs;
	if (m_mapLocationTemplateItems.Lookup(nDayIndex, plistLocations) && plistLocations) {	

		CString strLocations;
		
		//TES 8/3/2010 - PLID 39264 - Iterate through the list, and create a new list with just unique Location IDs.

		// (a.walling 2011-08-09 08:44) - PLID 45023 - Put the locations in order of first template occurrence

		// Gather all into sorted list
		std::set<LocationTemplateItem> templateItems(boost::begin(*plistLocations), boost::end(*plistLocations));
		
		// Gather the first templates for each location ID in order of earliest template
		std::set<LocationTemplateItem> locationTemplateItems;
		std::set<long> locationIDs;
		foreach(const LocationTemplateItem& item, templateItems) {
			if (!locationIDs.count(item.nLocationID)) {
				locationIDs.insert(item.nLocationID);
				locationTemplateItems.insert(item);
			}
		}

		//TES 8/3/2010 - PLID 39264 - Now output the list as text.
		
		foreach(const LocationTemplateItem& item, locationTemplateItems) {
			// (a.walling 2010-06-23 11:19) - PLID 39321 - Use the remote data cache to minimize sql server accesses
			// (c.haag 2010-11-16 13:49) - PLID 39444 - Prefix each location with its abbreviation if the user wants to
			CString strLocation = GetRemoteDataCache().GetLocationName(item.nLocationID);
			CString strAbbrev;

			if (GetRemotePropertyInt("SchedShowResLocationAbbrev", 1, 0, GetCurrentUserName(), false))
			{
				// (b.spivey, March 28, 2012) - PLID 47521 - If we have a location abbreviation in cache, use it. 
				strAbbrev = GetRemoteDataCache().GetLocationAbbreviation(item.nLocationID);
				if (strAbbrev.IsEmpty()) {
					//If empty, we didn't have one in the database. Lets proceed as normal. 
					strAbbrev = GetLocationAbbreviation(strLocation);
				}
				strAbbrev += " - ";
			}
			
			// (a.walling 2011-08-09 08:44) - PLID 45023 - Display the time ranges if desired

			CString strTimes;
			if (GetRemotePropertyInt("SchedShowLocationTemplateTimesInDescription", 1, 0, GetCurrentUserName(), true)) {
				typedef std::pair<COleDateTime, COleDateTime> COleDateTimeRange;
				std::vector<COleDateTimeRange> dtRanges;

				foreach(const LocationTemplateItem& locItem, templateItems) {
					if (locItem.nLocationID != item.nLocationID) continue;

					COleDateTime dtStart = AsTimeNoDate(locItem.dtStart);
					COleDateTime dtEnd = AsTimeNoDate(locItem.dtEnd);

					if (dtRanges.empty()) {
						dtRanges.push_back(std::make_pair(dtStart, dtEnd));
						continue;
					}

					if (dtStart < dtRanges.back().second) {
						// item start still within range, extend the end if necessary

						if (dtEnd > dtRanges.back().second) {
							dtRanges.back().second = dtEnd;
						}
					} else {
						// item does not fit within current range, so start a new one
						dtRanges.push_back(std::make_pair(dtStart, dtEnd));
					}
				}

				// now we have coalesced all contiguous and overlapping ranges; display them!
				foreach(const COleDateTimeRange& dtRange, dtRanges) {
					strTimes.AppendFormat(" (%s),", FormatDateTimeRangeForInterface(dtRange.first, dtRange.second));
				}

				strTimes.TrimRight(",");
			}

			strLocations.AppendFormat("%s%s%s\r\n", strAbbrev, strLocation, strTimes);
		}

		strLocations.TrimRight("\r\n");

		if (!strLocations.IsEmpty()) {
			strInfo.Format("Available today at these locations:   \r\n\r\n%s", strLocations);
		} else {
			strInfo = strLocations;
		}
	}

	return strInfo;
}

// (j.luckoski 2012-05-14 14:47) - PLID 48556 - Keep GetDayLabel but call the default values for full loaded GetDayLabel
CString CNxSchedulerDlg::CResourceAvailLocations::GetDayLabel(long nDayIndex, const CString& strExistingText) 
{
	CString strLabel = CResourceAvailLocations::GetDayLabel(nDayIndex, strExistingText, false);
	return strLabel;
}

// (c.haag 2010-11-16 13:49) - PLID 39444 - Returns a further formatted day label that can include
// resource location abbreviations
// (j.luckoski 2012-05-14 14:52) - PLID 48556 - Added bShowTimes for future proofing this function to show
// date ranges in day label.
CString CNxSchedulerDlg::CResourceAvailLocations::GetDayLabel(long nDayIndex, const CString& strExistingText, bool bShowTimes /* = false */)
{
	// Quit immediately if the user doesn't want to do special formatting
	if (!GetRemotePropertyInt("SchedShowResLocationAbbrev", 1, 0, GetCurrentUserName(), false)) {
		return strExistingText;
	}

	// (j.luckoski 2012-05-14 16:34) - PLID 48556 - If preference is false, then don't display time displays.
	if(!GetRemotePropertyInt("SchedResourceViewTimeRange", 0, 0, GetCurrentUserName(), true)) {
		bShowTimes = false;
	}

	// (j.luckoski 2012-06-22 15:11) - PLID 48556 - Only show 23 characters of the name if longer than 23.
	CString strTempLabel;
	CString strLabel = strExistingText;
	// (j.luckoski 2012-06-28 17:16) - PLID 48556 - Switched to 12 characters to satisfy 1024x768 resolution requirement.
	// toDO: we need to make this dynamic to look good on monitors larger than 1024x768
	if(bShowTimes && strLabel.GetLength() > 12) {
			CString strTempName = strLabel.Left(12);
			strLabel = strTempName + "...";
	}
	CList<LocationTemplateItem>* plistLocations = NULL;
	//CList<long> listLocationIDs;
	if (m_mapLocationTemplateItems.Lookup(nDayIndex, plistLocations) && plistLocations) 
	{	
		// (j.luckoski 2012-05-14 14:54) - PLID 48556 - If resource view, don't show dash but skip to next line of button
		if(!bShowTimes) {
			strLabel += " - ";
		} else {
			// (j.luckoski 2012-06-22 15:13) - PLID 48556 - Added \r\n for precaution.
			strLabel += "\r\n";
		}

		// (a.walling 2011-08-09 08:44) - PLID 45023 - Put the locations in order of first template occurrence

		// Gather all into sorted list
		std::set<LocationTemplateItem> templateItems;
		foreach(const LocationTemplateItem& item, *plistLocations) {
			templateItems.insert(item);
		}
		
		// Gather the first templates for each location ID in order of earliest template
		std::set<LocationTemplateItem> locationTemplateItems;
		std::set<long> locationIDs;
		foreach(const LocationTemplateItem& item, templateItems) {
			if (!locationIDs.count(item.nLocationID)) {
				locationIDs.insert(item.nLocationID);
				locationTemplateItems.insert(item);
			}
		}

		// Not iterate through the ID list getting location names and apply them to the label.
		// If we get overburdened, just show the first few.

		int nAddedLocations = 0;

		foreach(const LocationTemplateItem& item, locationTemplateItems) {
			if (nAddedLocations++ == 3) {
				// (j.luckoski 2012-06-28 10:15) - PLID 48556 - Needs to add the ... to the temp label and not hte label as doing that
				// will cause it to cause alignment issues
				strTempLabel.TrimRight("/");
				strTempLabel += "...";
				break;
			}
			// (b.spivey, March 26, 2012) - PLID 47521 - Get abbreviation. If empty we use the previous algorithm. 
			CString strLocation = GetRemoteDataCache().GetLocationName(item.nLocationID);
			CString strAbbrev = GetRemoteDataCache().GetLocationAbbreviation(item.nLocationID); 
			if(strAbbrev.IsEmpty()) {
				strAbbrev = GetLocationAbbreviation(strLocation);
			}

			// (j.luckoski 2012-05-14 14:54) - PLID 48556 - Grab and format each date range for each resource.
			CString strTimes = "";
			if(bShowTimes) {
				if (GetRemotePropertyInt("SchedShowLocationTemplateTimesInDescription", 1, 0, GetCurrentUserName(), true)) {
					typedef std::pair<COleDateTime, COleDateTime> COleDateTimeRange;
					std::vector<COleDateTimeRange> dtRanges;

					foreach(const LocationTemplateItem& locItem, templateItems) {
						if (locItem.nLocationID != item.nLocationID) continue;

						COleDateTime dtStart = AsTimeNoDate(locItem.dtStart);
						COleDateTime dtEnd = AsTimeNoDate(locItem.dtEnd);

						if (dtRanges.empty()) {
							dtRanges.push_back(std::make_pair(dtStart, dtEnd));
							continue;
						}

						if (dtStart < dtRanges.back().second) {
							// item start still within range, extend the end if necessary

							if (dtEnd > dtRanges.back().second) {
								dtRanges.back().second = dtEnd;
							}
						} else {
							// item does not fit within current range, so start a new one
							dtRanges.push_back(std::make_pair(dtStart, dtEnd));
						}
					}

					// now we have coalesced all contiguous and overlapping ranges; display them!
					foreach(const COleDateTimeRange& dtRange, dtRanges) {
						strTimes.AppendFormat("(%s),", FormatDateTimeRangeForDayLabel(dtRange.first, dtRange.second));
					}

					strTimes.TrimRight(",");
				}
			}

			// (j.luckoski 2012-05-14 14:55) - PLID 48556 - Append times
			// (j.luckoski 2012-06-22 15:14) - PLID Append times into label to truncate if needed
			strTempLabel += FormatString("%s%s/", strAbbrev, strTimes);	
		}
			
		// (j.luckoski 2012-06-22 15:14) - PLID 48556 - Truncate location abbrevs and times to 12 characters with ...
		// toDO: we need to make this dynamic to look good on monitors larger than 1024x768
		if(bShowTimes && strTempLabel.GetLength() > 12) {
			CString strTemp = strTempLabel.Left(12);
			strTemp.TrimRight("...");
			strLabel += strTemp + "...";
		} else {
			strLabel += strTempLabel;
		}

			strLabel.TrimRight("/");
		
	}
	return strLabel;
}

// (r.gonet 05/25/2012) - PLID 49059 - Returns the highest priority location template found for a certain day and time
//  Returns NULL if there is no location template at that slot.
bool CNxSchedulerDlg::CResourceAvailLocations::GetLocationTemplate(long nDayIndex, const COleDateTime &dtTime, OUT LocationTemplateItem &locationTemplateItem)
{
	//TES 8/3/2010 - PLID 39264 - Sometimes the times are off by a second, force it to just be hour+minute.
	COleDateTime dtCompare;
	dtCompare.SetTime(dtTime.GetHour(), dtTime.GetMinute(), 0);
	//TES 8/3/2010 - PLID 39264 - Get the list for the day.
	CList<LocationTemplateItem> *plistLTIs = NULL;
	if(m_mapLocationTemplateItems.Lookup(nDayIndex, plistLTIs) && plistLTIs) {
		//TES 8/3/2010 - PLID 39264 - Iterate through all the Location Template Items, track the highest-priority location ID.
		long nCurrentPriority = -1;
		POSITION pos = plistLTIs->GetHeadPosition();
		while (pos) {
			LocationTemplateItem lti = plistLTIs->GetNext(pos);
			//TES 8/3/2010 - PLID 39264 - Sometimes the times are off by a second, force them to just be hour+minute.
			COleDateTime dtStart;
			dtStart.SetTime(lti.dtStart.GetHour(), lti.dtStart.GetMinute(), 0);
			COleDateTime dtEnd;
			dtEnd.SetTime(lti.dtEnd.GetHour(), lti.dtEnd.GetMinute(), 0);
			//TES 8/3/2010 - PLID 39264 - Does the time match?  (start times match if they're equal, end times do not).
			if(dtStart <= dtCompare && dtEnd > dtCompare) {
				///TES 8/3/2010 - PLID 39264 - Times match, is this a higher priority than whatever we've got already?
				if(lti.nPriority > nCurrentPriority) {
					// This is highest priority template that we've found so far
					locationTemplateItem = lti;
					nCurrentPriority = lti.nPriority;
				}
			}
		}
		//Return whatever we found.
		if(nCurrentPriority != -1) {
			// We found something
			return true;
		} else {
			// No templates apply!
			return false;
		}
	}
	//No list for this day, so there's no location template
	return false;
}

// (r.gonet 05/25/2012) - PLID 49059 - Get the color of the highest priority location template. Returns true if there is a location tempate at this slot.
//  Returns false if there is no location template at this slot.
bool CNxSchedulerDlg::CResourceAvailLocations::GetLocationTemplateColor(long nDayIndex, const COleDateTime &dtTime, OUT long &nLocationTemplateColor)
{
	LocationTemplateItem item;
	if(GetLocationTemplate(nDayIndex, dtTime, item)) {
		nLocationTemplateColor = item.nColor;
		return true;
	} else {
		return false;
	}
}

//TES 8/3/2010 - PLID 39264 - For the given day and time, returns the location ID corresponding to the highest-priority Location
// Template for that slot (will return -1 if there is no Location Template at that slot).
long CNxSchedulerDlg::CResourceAvailLocations::GetLocationID(long nDayIndex, const COleDateTime &dtTime)
{
	//TES 8/3/2010 - PLID 39264 - Sometimes the times are off by a second, force it to just be hour+minute.
	COleDateTime dtCompare;
	dtCompare.SetTime(dtTime.GetHour(), dtTime.GetMinute(), 0);
	//TES 8/3/2010 - PLID 39264 - Get the list for the day.
	CList<LocationTemplateItem> *plistLTIs = NULL;
	if(m_mapLocationTemplateItems.Lookup(nDayIndex, plistLTIs) && plistLTIs) {
		//TES 8/3/2010 - PLID 39264 - Iterate through all the Location Template Items, track the highest-priority location ID.
		long nCurrentLocationID = -1;
		long nCurrentPriority = -1;
		POSITION pos = plistLTIs->GetHeadPosition();
		while (pos) {
			LocationTemplateItem lti = plistLTIs->GetNext(pos);
			//TES 8/3/2010 - PLID 39264 - Sometimes the times are off by a second, force them to just be hour+minute.
			COleDateTime dtStart;
			dtStart.SetTime(lti.dtStart.GetHour(), lti.dtStart.GetMinute(), 0);
			COleDateTime dtEnd;
			dtEnd.SetTime(lti.dtEnd.GetHour(), lti.dtEnd.GetMinute(), 0);
			//TES 8/3/2010 - PLID 39264 - Does the time match?  (start times match if they're equal, end times do not).
			if(dtStart <= dtCompare && dtEnd > dtCompare) {
				///TES 8/3/2010 - PLID 39264 - Times match, is this a higher priority than whatever we've got already?
				if(lti.nPriority > nCurrentPriority) {
					//TES 8/3/2010 - PLID 39264 - It is, so this is now our official item.
					nCurrentLocationID = lti.nLocationID;
					nCurrentPriority = lti.nPriority;
				}
			}
		}
		//TES 8/3/2010 - PLID 39264 - Return whatever we found.
		return nCurrentLocationID;
	}
	//TES 8/3/2010 - PLID 39264 - No list for this day, so there's no location template
	return -1;
}

//TES 9/7/2010 - PLID 39630 - Adds a time range that should be excluded from location templates.
void CNxSchedulerDlg::CResourceAvailLocations::AddExcludedRange(long nDayIndex, const TimeRange &trRange)
{
	CComplexTimeRange *& prangeExcluded = m_mapExcludedTimes[nDayIndex];
	if(!prangeExcluded) {
		prangeExcluded = new CComplexTimeRange;
	}
	prangeExcluded->AddRange(trRange.dtStart, trRange.dtEnd);
}

//TES 9/7/2010 - PLID 39630 - Removes a time range from the times that should be excluded from location templates
void CNxSchedulerDlg::CResourceAvailLocations::RemoveExcludedRange(long nDayIndex, const TimeRange &trRange)
{
	CComplexTimeRange *& prangeExcluded = m_mapExcludedTimes[nDayIndex];
	if(!prangeExcluded) {
		prangeExcluded = new CComplexTimeRange;
	}
	prangeExcluded->RemoveRange(trRange.dtStart, trRange.dtEnd);

}

//TES 1/5/2012 - PLID 47310 - Is there any time on the given day on which the resource is available for the given location?
bool CNxSchedulerDlg::CResourceAvailLocations::IsAvailableForDay(long nDayIndex, long nLocationID)
{
	//TES 1/5/2012 - PLID 47310 - Logic copied out of GetDayLabel()
	CList<LocationTemplateItem>* plistLocations = NULL;
	//CList<long> listLocationIDs;
	if (m_mapLocationTemplateItems.Lookup(nDayIndex, plistLocations) && plistLocations) {
		// Gather all into sorted list
		std::set<LocationTemplateItem> templateItems;
		foreach(const LocationTemplateItem& item, *plistLocations) {
			templateItems.insert(item);
		}
		
		// Gather the first templates for each location ID in order of earliest template
		std::set<LocationTemplateItem> locationTemplateItems;
		std::set<long> locationIDs;
		foreach(const LocationTemplateItem& item, templateItems) {
			if(item.nLocationID == nLocationID) {
				return true;
			}
		}
	}
	return false;

}

// (a.walling 2010-06-23 18:00) - PLID 39263 - Now a virtual function
void CNxSchedulerDlg::UpdateActiveDateLabel()
{
	SetDlgItemText(IDC_ACTIVE_DATE_LABEL, FormatDateTimeForInterface(GetActiveDate(), "%#x"));
}

// (c.haag 2010-08-27 11:38) - PLID 40108 - Returns TRUE if it's ok to process an UpdateView-type command
BOOL CNxSchedulerDlg::IsSafeToUpdateView() const
{
	if (NULL == m_ResEntry) {
		// This should never ever happen as the schedulerview creates the one and only resentry object and every
		// child sheet gets a pointer to it, but we really need to check for nulls for completeness
		return TRUE;
	}
	if ((m_ResEntry->GetSafeHwnd() && m_ResEntry->IsWindowVisible()) // Not ok if the resentry window is open
		|| m_bResLockedByRightClick // Not ok if the user is in a right-click action
		|| m_ResEntry->IsReserved(TRUE) // Not ok if the resentry is reserved (meaning it -could- open or is opening)
		)
		return FALSE;
	else
		return TRUE;
}

// (j.dinatale 2010-09-08) - PLID 23009 - Need to show the room manager when the room manager button is pressed
void CNxSchedulerDlg::OnRoomManagerBtn()
{
	try{
		// have the main frame show the room manager for us
		GetMainFrame()->ShowRoomManager();
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-02-29 17:46) - PLID 48488
void CNxSchedulerDlg::OnRecallBtn()
{
	try {
		GetMainFrame()->ShowRecallsNeedingAttention(false);
	} NxCatchAll(__FUNCTION__);
}

void CNxSchedulerDlg::OnReschedulingQueueBtn()
{
	try {
		GetMainFrame()->ShowReschedulingQueue();
	} NxCatchAll(__FUNCTION__);
}

//TES 10/27/2010 - PLID 40868 - A new function which will determine which precision template, if any, an appointment represented
// by the given VisibleAppointment struct would fall.  If there is currently an appointment on this dialog with the same ID as that
// in pAppt, it will be ignored, under the assumption that pAppt is a newly modified version intended to replace that appointment.
long CNxSchedulerDlg::CalculateTemplateItemID(VisibleAppointment *pAppt)
{
	//TES 10/27/2010 - PLID 40868 - Copied this stretch of code out of UpdateTemplateBlocks().
	CArray<BlockReservation, BlockReservation> aBlockRes;
	int nVisibleTemplates = m_aVisibleTemplates.GetCount();
	for (int i=0; i < nVisibleTemplates; i++) {
		VisibleTemplate* pvis = m_aVisibleTemplates[i];
		if (pvis->bIsBlock && pvis->nSingleDayColumn == pAppt->nOffsetDay) {
			COleDateTime dtTemplateStart = pvis->ctrRange.GetStart();
			COleDateTime dtTemplateEnd = pvis->ctrRange.GetEnd();
			BlockReservation br;
			br.pTemplate = pvis;
			br.dtStart.SetDateTime(
				pAppt->dtStart.GetYear(), pAppt->dtStart.GetMonth(), pAppt->dtStart.GetDay(),
				dtTemplateStart.GetHour(), dtTemplateStart.GetMinute(), dtTemplateStart.GetSecond()
				);
			br.dtEnd.SetDateTime(
				pAppt->dtStart.GetYear(), pAppt->dtStart.GetMonth(), pAppt->dtStart.GetDay(),
				dtTemplateEnd.GetHour(), dtTemplateEnd.GetMinute(), dtTemplateEnd.GetSecond()
				);
			aBlockRes.Add(br);
		}
	}
	//TES 10/27/2010 - PLID 40868 - Now generate an array of all the visible appointments on the sheet EXCEPT the one
	// that we were given (if any)
	CArray<VisibleAppointment*,VisibleAppointment*> arAppt;
	int nApptCount = m_aVisibleAppts.GetSize();
	for(int i = 0; i < nApptCount; i++) {
		if(m_aVisibleAppts[i]->nReservationID != pAppt->nReservationID) {
			arAppt.Add(m_aVisibleAppts[i]);
		}
	}
	//TES 10/27/2010 - PLID 40868 - Now add the one we were given, and pass it into RepopulateBlockResArray_ByAppointments(), thus
	// telling it to output the corresponding TemplateItemID (if any).
	arAppt.Add(pAppt);
	long nTemplateItemID = -1;
	RepopulateBlockResArray_ByAppointments(aBlockRes, arAppt, pAppt->nOffsetDay, pAppt->dtStart, GetActiveInterval(), m_pSingleDayCtrl, pAppt, &nTemplateItemID);
	return nTemplateItemID;
}

// (d.lange 2010-11-08 10:22) - PLID 41192 - We return TRUE if we call LaunchDevice for the loaded device plugin
// (j.gruber 2013-04-03 14:59) - PLID 56012 - not needed anymore

void CNxSchedulerDlg::OnReservationRecalculateColor(LPDISPATCH theRes)
{
	try {
		//TES 3/8/2011 - PLID 41519 - This appointment was previously highlighted, and now it isn't.  Since its color may have changed since 
		// it was created (if the status changed), we need to calculate what the current color should be, and set it.
		// (a.wilson 2012-06-14 16:18) - PLID 47966 - change for new preference type.
		CReservation pRes(__FUNCTION__, theRes);
		
		// (j.luckoski 2012-06-20 11:18) - PLID 11597 - recalculate color based upon appointments status as open
		// or cancelled.

		bool bIsResCancelled = false;
		// (a.walling 2013-01-21 16:48) - PLID 54744 - Use CancelledDate in Reserveration::Data property map
		if(pRes.Data["CancelledDate"].vt == VT_DATE) {
			bIsResCancelled = true;
		}

		COLORREF clrBkg = DEFAULT_APPT_BACKGROUND_COLOR;
		if (bIsResCancelled) {
			clrBkg = m_nCancelColor;
		}
		// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
		else if (GetRemotePropertyInt("ColorApptBgWithStatus", GetPropertyInt("ColorApptBgWithStatus", 1, 0, false), 0, GetCurrentUserName(), true)) {
			// (j.jones 2014-12-04 14:46) - PLID 64119 - use the mix rule color if one exists and
			// the appt. is pending, otherwise use the status color
			long nMixRuleColor = -1;
			long nStatusID = -1;
			OLE_COLOR clrStatus = AppointmentGetStatusColor(pRes.GetReservationID(), &nStatusID);

			if (nStatusID == 0) {	//pending
				nMixRuleColor = GetAppointmentMixRuleColor(pRes.GetReservationID());
			}

			if (nMixRuleColor != -1) {
				clrBkg = nMixRuleColor;
			}
			else {
				clrBkg = clrStatus;
			}
		}
		else if (GetRemotePropertyInt("ColorApptBackground", GetPropertyInt("ColorApptBackground", 0, 0, false), 0, GetCurrentUserName(), true)) {
			clrBkg = pRes.GetBorderColor();
		}
		else {
			clrBkg = DEFAULT_APPT_BACKGROUND_COLOR;
		}
		ColorReservationBackground(theRes, clrBkg);
		
	} NxCatchAllCall(__FUNCTION__, try { CReservation(__FUNCTION__, theRes).DeleteRes(__FUNCTION__); } NxCatchAllIgnore(); );
}

//TES 3/8/2011 - PLID 41519 - Clear out the currently "highlighted" appointment (if any)
void CNxSchedulerDlg::ClearHighlightedReservation()
{
	if(m_pSingleDayCtrl) {
		m_pSingleDayCtrl.SetHighlightRes(NULL);
	}
	//TES 3/31/2011 - PLID 41519 - Clear out our "pending" highlight
	m_nPendingHighlightID = -1;
}

// (j.jones 2014-08-14 12:47) - PLID 63184 - Added function that lets us know if
// a given location ID is in our current location filter. Always returns true
// if we're viewing all locations.
bool CNxSchedulerDlg::IsLocationInView(long nLocationID)
{
	if (nLocationID < 0) {
		//this tells us nothing
		return true;
	}

	long nFilterLocationID = GetLocationId();

	if (nFilterLocationID == -1) {
		//we aren't filtering by any location
		return true;
	}

	if (nFilterLocationID > -1) {
		//we are filtering by one location
		return nFilterLocationID == nLocationID;
	}

	if (nFilterLocationID == -2) {
		//we are filtering by multiple locations,
		//find out if our requested location is one of them
		CDWordArray dwaryLocationIDs;
		GetLocationFilterIDs(&dwaryLocationIDs);
		if (dwaryLocationIDs.GetSize() == 0) {
			//this should not be possible
			ASSERT(FALSE);
			return true;
		}

		//now return true if our location is in the filter
		for (int i = 0; i < dwaryLocationIDs.GetCount(); i++) {
			if ((long)(dwaryLocationIDs.GetAt(i)) == nLocationID) {
				//this location is in our filter
				return true;
			}
		}

		//our location is not in the filter
		return false;
	}

	//there are no known values for m_nLocationId
	//besides -1, -2, and a positive value
	ASSERT(FALSE);
	return true;
}

void CNxSchedulerDlg::OnViewMixRules()
{
	try {
		//TES 11/18/2014 - PLID 64120 - Launch the Scheduler Mix Rules dialog
		CViewSchedulerMixRulesDlg dlg;
		//TES 11/18/2014 - PLID 64123 - Initialize to the active resource and date
		dlg.m_dtInitialDate = GetActiveDate();
		dlg.m_nInitialResourceID = m_pParent->GetActiveResourceID();
		if (IDOK == dlg.DoModal()) {
			//TES 11/18/2014 - PLID 64124 - If they selected a new resource and date, flip to it.
			//TES 12/30/2014 - PLID 64124 - Switch to the Day view, then get the active sheet, since this may no longer be it
			g_Modules[Modules::Scheduler]->ActivateTab(SchedulerModule::DayTab);
			CNxSchedulerDlg *pActiveSheet = (CNxSchedulerDlg*)m_pParent->GetActiveSheet();
			if (dlg.m_nInitialResourceID != dlg.m_nNewResourceID) {
				if (m_pParent->IsResourceInCurView(dlg.m_nNewResourceID)) {
					m_pParent->SetActiveResourceID(dlg.m_nNewResourceID, TRUE);
				}
				else {
					MsgBox("The resource you selected is not available in your current view");
				}
			}
			if (dlg.m_dtInitialDate != dlg.m_dtNewDate) {
				pActiveSheet->SetActiveDate(dlg.m_dtNewDate);
				//TES 12/16/2014 - PLID 64124 - Update the screen
				pActiveSheet->OnSetDateSelCombo();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-12-03 16:08) - PLID 64276 - Resets the colors on the time slots on the left,
// mainly for after a ResEntry window is closed and its gray color is removed.
// Special colors such as time slots with override appts. will be reloaded unless
// bReColorTimes is set to false, in which case all slots will lose their special colors.
// The current time is always colored.
void CNxSchedulerDlg::ResetTimeColors(bool bReColorTimes /*= true*/)
{
	if (m_pSingleDayCtrl) {

		//this will set all the time slot colors to their basic gray,
		//except for the current time
		m_pSingleDayCtrl.SetTimeButtonColors(0, 0, 0);

		if (bReColorTimes) {
			//recalculate any time slots that need coloring
			for each(ColoredTime ct in m_aryColoredTimes) {
				m_pSingleDayCtrl.SetTimeButtonColors(ct.dtStart, ct.dtEnd, ct.color);
			}
		}
	}
}

// (j.jones 2014-12-11 15:07) - PLID 64178 - moved CComplexTimeRange from CommonSchedUtils
//TES 9/2/2010 - PLID 39630 - Add the given range of times to the set of ranges encapsulated by this object, combining entries as appropriate.
void CComplexTimeRange::AddRange(const COleDateTime &dtStart, const COleDateTime &dtEnd)
{
	POSITION pos = m_Ranges.GetHeadPosition();
	while (pos) {
		POSITION posRange = pos;
		TimeRange range = m_Ranges.GetNext(pos);
		//The range we were given is either:
		// completely before this one, in which case we can just insert it,
		// starts before this one and ends during it, in which case we need to update this one's start time,
		// starts before/during and ends after this one, in which case we need to find where it ends and extend/combine ranges,
		// completely inside this one, in which case we don't need to do anything,
		// completely after this one, in which case we move to the next position.
		//First, does it start before this one ends?
		if (CompareTimes(dtStart, range.dtEnd) & CT_LESS_THAN_OR_EQUAL) {
			//OK, does it end before this one starts?
			if (CompareTimes(dtEnd, range.dtStart) & CT_LESS_THAN) {
				//Yes, so insert it
				TimeRange rangeNew;
				rangeNew.dtStart = dtStart;
				rangeNew.dtEnd = dtEnd;
				m_Ranges.InsertBefore(posRange, rangeNew);
				return;
			}
			else {
				//No, these two overlap.  Does it end before this one ends?
				if (CompareTimes(dtEnd, range.dtEnd) & CT_LESS_THAN_OR_EQUAL) {
					//It does, does it start before this one starts?
					if (CompareTimes(dtStart, range.dtStart) & CT_LESS_THAN) {
						//It does, so update the start time.
						range.dtStart = dtStart;
						m_Ranges.SetAt(posRange, range);
						return;
					}
					else {
						//It's completely within this one, so do nothing.
						return;
					}
				}
				else {
					//No, it continues past the end of this one.
					//Does it extend into the next one (if any)?
					POSITION posNext = pos;
					while (posNext) {
						POSITION posCurrent = posNext;
						TimeRange rangeNext = m_Ranges.GetNext(posNext);
						CString strNextStart = rangeNext.dtStart.Format();
						CString strNextEnd = rangeNext.dtEnd.Format();
						if (CompareTimes(dtEnd, rangeNext.dtStart) & CT_GREATER_THAN_OR_EQUAL) {
							//It does extend into this one.  Does it go past it?
							if (CompareTimes(dtEnd, rangeNext.dtEnd) & CT_LESS_THAN_OR_EQUAL) {
								//No, it ends inside this one, so update the first range's end time.
								range.dtEnd = rangeNext.dtEnd;
								//Check if we need to update the first range's start time.
								if (CompareTimes(dtStart, range.dtStart) & CT_LESS_THAN) {
									range.dtStart = dtStart;
								}
								m_Ranges.SetAt(posRange, range);
								//Remove this range.
								m_Ranges.RemoveAt(posCurrent);
								return;
							}
							else {
								//Remove this range, and keep going
								m_Ranges.RemoveAt(posCurrent);
							}
						}
						else {
							//It ends before this one starts.  So, update the first range's end time.
							range.dtEnd = dtEnd;
							//Check if we need to update the first range's start time
							if (CompareTimes(dtStart, range.dtStart) & CT_LESS_THAN) {
								range.dtStart = dtStart;
							}
							m_Ranges.SetAt(posRange, range);
							return;
						}
					}
					//If we get here, this range goes past the end of our list.  Update the first range's end time
					range.dtEnd = dtEnd;
					//Check if we need to update the first range's start time
					if (CompareTimes(dtStart, range.dtStart) & CT_LESS_THAN) {
						range.dtStart = dtStart;
					}
					m_Ranges.SetAt(posRange, range);
					return;
				}
			}
		}
	}

	//If we get here, this one starts after the last one ends, so add it to the end.
	TimeRange rangeNew;
	rangeNew.dtStart = dtStart;
	rangeNew.dtEnd = dtEnd;
	m_Ranges.AddTail(rangeNew);
}

//TES 9/2/2010 - PLID 39630 - Remove the given range of times to the set of ranges encapsulated by this object, splitting up entries as appropriate.
void CComplexTimeRange::RemoveRange(const COleDateTime &dtStart, const COleDateTime &dtEnd)
{
	POSITION pos = m_Ranges.GetHeadPosition();
	while (pos) {
		POSITION posCurrent = pos;
		TimeRange range = m_Ranges.GetNext(pos);
		//Does this end after the given start?
		if (CompareTimes(range.dtEnd, dtStart) & CT_GREATER_THAN) {
			//Yes, does it start before the given start?
			if (CompareTimes(range.dtStart, dtStart) & CT_LESS_THAN) {
				//Yes, does it end after the given end?
				if (CompareTimes(range.dtEnd, dtEnd) & CT_GREATER_THAN) {
					//It does, so we need to split this into two parts.
					TimeRange rangeNew;
					rangeNew.dtStart = dtEnd;
					rangeNew.dtEnd = range.dtEnd;
					m_Ranges.InsertAfter(posCurrent, rangeNew);
					range.dtEnd = dtStart;
					m_Ranges.SetAt(posCurrent, range);
					return;
				}
				else {
					//No, so just trim this one and keep going.
					range.dtEnd = dtStart;
					m_Ranges.SetAt(posCurrent, range);
				}
			}
			else {
				//It starts after the given start, does it start after the given end?
				if (CompareTimes(range.dtStart, dtEnd) & CT_GREATER_THAN_OR_EQUAL) {
					//We're done
					return;
				}
				else {
					//It starts before the given end, does it end after it?
					if (CompareTimes(range.dtEnd, dtEnd) & CT_GREATER_THAN) {
						//It does, so trim and keep going.
						range.dtStart = dtEnd;
						m_Ranges.SetAt(posCurrent, range);
					}
					else {
						//It doesn't, so this one's completely within the given range, so remove it.
						m_Ranges.RemoveAt(posCurrent);
					}
				}
			}
		}
	}
}

COleDateTime CComplexTimeRange::GetStart()
{
	COleDateTime dtStart;
	if (!m_Ranges.IsEmpty()) {
		dtStart = m_Ranges.GetHead().dtStart;
	}
	return dtStart;
}

COleDateTime CComplexTimeRange::GetEnd()
{
	COleDateTime dtEnd;
	if (!m_Ranges.IsEmpty()) {
		dtEnd = m_Ranges.GetTail().dtEnd;
	}
	return dtEnd;
}

//TES 9/3/2010 - PLID 39630 - Get the range, as an array of contiguous ranges.
void CComplexTimeRange::GetRanges(CArray<TimeRange, TimeRange&> &arOut)
{
	arOut.RemoveAll();
	POSITION pos = m_Ranges.GetHeadPosition();
	while (pos) {
		arOut.Add(m_Ranges.GetNext(pos));
	}
}

//TES 9/3/2010 - PLID 39630 - Get the number of contiguous ranges in this object.
int CComplexTimeRange::GetCount()
{
	return m_Ranges.GetCount();
}

//TES 9/7/2010 - PLID 39630 - Remove the range that is specified by this object from the range specified by arTimes.
// This is basically saying arTimes = arTimes & ~(this object's range).
void CComplexTimeRange::RemoveFromRanges(CArray<TimeRange, TimeRange&> &arTimes)
{
	//TES 9/7/2010 - PLID 39630 - Just make a range equivalent to what we were passed in, then go through all of our
	// ranges and remove them from that range, then put that range back into arTimes;
	CComplexTimeRange rangeOut;
	for (int i = 0; i < arTimes.GetSize(); i++) {
		rangeOut.AddRange(arTimes[i].dtStart, arTimes[i].dtEnd);
	}
	POSITION pos = m_Ranges.GetHeadPosition();
	while (pos) {
		TimeRange tr = m_Ranges.GetNext(pos);
		rangeOut.RemoveRange(tr.dtStart, tr.dtEnd);
	}
	rangeOut.GetRanges(arTimes);
}

//TES 12/3/2014 - PLID 64180 - An option to launch the FFA with pre-filled settings, immediately displaying the results.
//TES 12/18/2014 - PLID 64466 - If successful, this will fill pSelectedSlot with information about the slot the user chose through FFA
void CNxSchedulerDlg::FindFirstAvailableApptWithPresets(long nPatientID, long nAptTypeID, const CDWordArray& adwPurpose, const CDWordArray& adwResource, long nInsuredPartyID, OUT SelectedFFASlotPtr &pSelectedSlot, long nLocationID)
{
	pSelectedSlot.reset();

	GetMainFrame()->m_FirstAvailAppt.m_bRunImmediately = true;
	//TES 12/18/2014 - PLID 64466 - Tell FFA to just return the slot, rather than trying to create the appointment
	GetMainFrame()->m_FirstAvailAppt.m_bReturnSlot = true;
	//TES 12/3/2014 - PLID 64180 - Set the overrides for the values we were given.
	GetMainFrame()->m_FirstAvailAppt.m_bDefaultOverrides.bPatient = true;
	GetMainFrame()->m_FirstAvailAppt.m_nPreselectPatientID = nPatientID;
	GetMainFrame()->m_FirstAvailAppt.m_bDefaultOverrides.bAppt = true;
	GetMainFrame()->m_FirstAvailAppt.m_nPreselectApptTypeID = nAptTypeID;
	//TES 12/3/2014 - PLID 64180 - If we were given multiple purposes, we can't run immediately, our caller should have notified the user.
	if (adwPurpose.GetSize() > 1) {
		GetMainFrame()->m_FirstAvailAppt.m_bRunImmediately = false;
	}
	else if (adwPurpose.GetSize() == 1) {
		GetMainFrame()->m_FirstAvailAppt.m_nPreselectApptPurposeID = adwPurpose[0];
	}
	GetMainFrame()->m_FirstAvailAppt.m_bDefaultOverrides.bResources = true;
	GetMainFrame()->m_FirstAvailAppt.m_aryPreselectResources.RemoveAll();
	foreach(DWORD dwResID, adwResource) {
		GetMainFrame()->m_FirstAvailAppt.m_aryPreselectResources.Add(dwResID);
	}
	//(s.dhole 6/4/2015 4:26 PM ) - PLID 65638
	if (adwResource.GetSize() > 1) {
		GetMainFrame()->m_FirstAvailAppt.m_nPreselectResourceTypeSelection = -202; //FFA_MULTIPLE_SCHEDULER_RESOURCE
	}
	else if (adwResource.GetSize() == 1) {

		GetMainFrame()->m_FirstAvailAppt.m_nPreselectResourceTypeSelection = adwResource[0];
	}
	GetMainFrame()->m_FirstAvailAppt.m_bDefaultOverrides.bInsurance = true;
	GetMainFrame()->m_FirstAvailAppt.m_nPreselectInsuredPartyID = nInsuredPartyID;

	GetMainFrame()->m_FirstAvailAppt.m_bDefaultOverrides.bLocation = true;
	GetMainFrame()->m_FirstAvailAppt.m_aryPreselectLocations.RemoveAll();
	GetMainFrame()->m_FirstAvailAppt.m_aryPreselectLocations.Add(nLocationID);

	//TES 12/3/2014 - PLID 64180 - Launch the FFA
	if (IDCANCEL == GetMainFrame()->m_FirstAvailAppt.DoModal()) {
		//TES 12/18/2014 - PLID 64466 - Make sure we don't return a slot
		pSelectedSlot.reset();
		return;
	}
	//TES 12/18/2014 - PLID 64466 - This is where we'll fill in the slot information once I finish implementing that
	pSelectedSlot = make_shared<SelectedFFASlot>();
	//TES 1/7/2015 - PLID 64466 - This is implemented now, just copy the information to pSelectedSlot and return
	pSelectedSlot = GetMainFrame()->m_FirstAvailAppt.m_UserSelectedSlot;

}

// Updates the m_pParent's m_pResDispatch stuff and m_*Click* props
Nx::Scheduler::TargetInfo CNxSchedulerDlg::CalcTargetInfo()
{
	// Get the column the mouse is over
	long nClickInDay = GetMsDay();

	// Get the time the mouse is over
	COleDateTime dtWorkingTime = GetMsTime();

	// Get the resource and date associated with the column the mouse is over
	long nWorkingResourceID;
	COleDateTime dtWorkingDate;
	m_pParent->GetWorkingResourceAndDate(this, nClickInDay, nWorkingResourceID, dtWorkingDate);
	
	Nx::Scheduler::TargetInfo info;
	GetCursorPos(&info.ptScreen);
	info.time = dtWorkingTime;
	info.isEvent = FALSE;
	info.date = dtWorkingDate;
	info.day = nClickInDay;
	info.resourceID = nWorkingResourceID;

	return info;
}

void CNxSchedulerDlg::UpdateTargetInfo(Nx::Scheduler::TargetInfo targetInfo)
{
	m_pParent->m_clickTargetInfo = targetInfo;
	
	m_pParent->m_pResDispatch = (LPDISPATCH)-1;
	m_pParent->m_pResDispatchPtr.SetDispatch(NULL);

	CString owner = __FUNCTION__;

	// (a.walling 2015-01-29 10:38) - PLID 64705 - Figure out precision template from point in singleday for drag/drop to specific precision template
	long count = m_pSingleDayCtrl.GetReservationCount((short)targetInfo.day);
	for (long i = 0; i < count; ++i) {
		auto res = m_pSingleDayCtrl.GetReservation(owner, targetInfo.day, i);
		
		if (!res || res.GetTemplateItemID() == -1) {
			continue;
		}

		HWND hwnd = (HWND)res.GethWnd();
		if (!hwnd || !::IsWindow(hwnd)) {
			continue;
		}

		CRect rc;
		if (!::GetWindowRect(hwnd, &rc)) {
			continue;
		}

		if (rc.PtInRect(targetInfo.ptScreen)) {
			m_pParent->m_pResDispatchPtr = res;
			m_pParent->m_pResDispatch = res.GetDispatch();
		}
	}
}

//TES 2/11/2015 - PLID 64120 - Used when first creating a mix rule, to update whatever scheduler tab might already be open
void CNxSchedulerDlg::ShowMixRulesButton()
{
	m_btnViewMixRules.ShowWindow(SW_SHOWNA);
}

// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
BOOL CNxSchedulerDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try {
		CPoint point;
		GetMessagePos(&point);
		ScreenToClient(&point);
		int nID;
		{
			CWnd *pChild = ChildWindowFromPoint(point, CWP_SKIPINVISIBLE);
			if (pChild != NULL && pChild->IsWindowEnabled()) {
				nID = pChild->GetDlgCtrlID();
			} else {
				nID = 0;
			}
		}
		switch (nID) {
		case IDC_TODAY_BTN:
		case IDC_AM_PM_BTN:
			SetCursor(GetLinkCursor());
			return TRUE;
		default:
			// (b.cardillo 2016-06-06 16:20) - NX-100773 (supplemental) - Don't show the hand cursor, it's not a hyperlink
			break;
		}
	} NxCatchAll(__FUNCTION__);

	return __super::OnSetCursor(pWnd, nHitTest, message);
}

// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
LRESULT CNxSchedulerDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UINT nIdc = (UINT)wParam;
		switch (nIdc) {
		case IDC_TODAY_BTN:
			// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
			OnTodayBtn();
			break;
		case IDC_AM_PM_BTN:
			// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
			OnAmPmBtn();
			break;
		case IDC_BTN_NEW_EVENT:
			// (b.cardillo 2016-06-06 16:20) - NX-100773 - Make event button a clickable label
			OnNewEvent();
			break;
		default:
			break;
		}
	} NxCatchAll(__FUNCTION__);
	return 0;
}

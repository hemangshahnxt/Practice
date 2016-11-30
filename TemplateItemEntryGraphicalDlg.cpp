// TemplateItemEntryGraphicalDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SchedulerRc.h"
#include "TemplateItemEntryGraphicalDlg.h"
#include "TemplateHitSet.h"
#include "GlobalSchedUtils.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "TemplateLineItemDlg.h"
#include "GlobalSchedUtils.h"
#include "TemplateSelectDlg.h"
#include "TemplatesDlg.h"
#include "TemplateItemChangeTimePromptDlg.h"
#include "TemplateInterfaceOptionsDlg.h"
#include "DontShowDlg.h"
#include "ChildFrm.h"
#include "Client.h"
#include "SchedulerView.h"
#include "NxSchedulerDlg.h"
#include "NameAndColorEntryDlg.h"
#include "ChooseDateRangeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2008-05-13 10:37) - PLID 27591 - Removed some COleVariant and VarDateTime usage that is no longer necessary

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace SINGLEDAYLib;
using namespace ADODB;
using namespace NexTech_Accessor;


// (z.manning 2015-01-15 09:45) - PLID 64210 - Background color when editing template collections
CBrush g_brTemplateCollectionBackground;

enum EResorceListColumns
{
	rlcID = 0,
	rlcName = 1,
};

// (a.vengrofski 2010-03-18 15:08) - PLID <28394> - Moved this so it could be used more.
enum
{
	miEdit = 1,
	miDelete,
	miCut,
	miCopy,
	miPaste,
	miCopyAll, // (a.vengrofski 2009-12-30 12:27) - PLID <28394>
	miPasteAll, // (a.vengrofski 2009-12-30 12:27) - PLID <28394>
};

// (c.haag 2014-12-16) - PLID 64255 - Used for collection sorting
static int __cdecl CompareUsedCollection(const void* p1, const void * p2)
{
	return (((const CTemplateItemEntryGraphicalDlg::UsedCollection*)p1)->Name.Compare(
		((const CTemplateItemEntryGraphicalDlg::UsedCollection*)p2)->Name));
}

/////////////////////////////////////////////////////////////////////////////
// CTemplateItemEntryGraphicalDlg dialog


// (z.manning 2014-12-01 17:42) - PLID 64205 - Added dialog type param to constructor
CTemplateItemEntryGraphicalDlg::CTemplateItemEntryGraphicalDlg(ESchedulerTemplateEditorType type, CWnd* pParent /*=NULL*/)
	: CNxDialog(CTemplateItemEntryGraphicalDlg::IDD, (CNxView*)pParent),
	m_lohOfficeHours(*(new CLocationOfficeHours()))
{
	EnableDragHandle(false);	// (j.armen 2012-05-30 11:46) - PLID 49854 - Drag handle doesn't look good here.  Disable it.
	m_bIsDateSelComboDroppedDown = FALSE;
	m_bGoToAM = FALSE;
	m_pdlgTemplateList = NULL;
	m_nDefaultResourceID = -1;
	m_dtDefaultDate = g_cdtInvalid;
	m_bResMoving = FALSE;
	m_pClippedLineItem = NULL;
	m_pCopyAllLineItem = NULL;// (a.vengrofski 2009-12-31 18:20) - PLID <28394>
	m_eLastClipType = crtNone;
	m_bResourceListLoaded = FALSE;
	m_bVisible = TRUE;
	m_bLoadAllResources = FALSE;
	m_pClippedCollectionTemplate = NULL;

	m_eEditorType = type;
	// (z.manning 2014-12-03 10:42) - PLID 64210 - This variable is redundant now that we have one for the type
	// but let's keep it around to avoid changing a bunch of places in code.
	m_bUseResourceAvailTemplates = (type == stetLocation);
}

CTemplateItemEntryGraphicalDlg::~CTemplateItemEntryGraphicalDlg()
{
	if (m_pClippedLineItem) {
		delete m_pClippedLineItem;
	}
	if (m_pCopyAllLineItem) {
		delete m_pCopyAllLineItem;
	}
	delete &m_lohOfficeHours;


	// (z.manning 2014-12-10 08:43) - PLID 64210 - Reset the shared brush to the default brackground color
	if (g_brTemplateCollectionBackground.m_hObject != NULL) {
		g_brTemplateCollectionBackground.DeleteObject();
	}
}


void CTemplateItemEntryGraphicalDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTemplateItemEntryGraphicalDlg)
	DDX_Control(pDX, IDC_MOVE_DAY_FORWARD, m_btnMoveDayForward);
	DDX_Control(pDX, IDC_MOVE_DAY_BACK, m_btnMoveDayBack);
	DDX_Control(pDX, IDC_DATE_SEL_COMBO, m_dtpActiveDate);
	DDX_Control(pDX, IDC_TODAY_BTN, m_btnToday);
	DDX_Control(pDX, IDC_AM_PM_BTN, m_btnAmPm);
	DDX_Control(pDX, IDC_CLOSE_BTN, m_btnClose);
	DDX_Control(pDX, IDC_OPTIONS_BTN, m_btnOptions);
	DDX_Control(pDX, IDC_CREATE_COLLECTION_FROM_VIEW_BTN, m_btnCreateCollectionFromView);
	DDX_Control(pDX, IDC_SCHEDULE_TEMPLATING_HELP, m_btnHelp);
	DDX_Control(pDX, IDC_STATIC_DAY_OF_WEEK, m_nxstaticDayOfWeek);
	DDX_Control(pDX, IDC_TEMPLATE_LIST_PLACEHOLDER, m_btnTemplateListPlaceholder);
	DDX_Control(pDX, IDC_STATIC_COLLECTIONS_USED, m_nxlCollectionsUsed);
	DDX_Control(pDX, IDC_REMOVE_TEMPLATES_BTN, m_btnRemoveTemplates);
	//}}AFX_DATA_MAP
}


//	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_DATE_SEL_COMBO, 2 /* Change */, OnChangeDateSelCombo, VTS_NONE)
//	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_DATE_SEL_COMBO, 3 /* CloseUp */, OnCloseUpDateSelCombo, VTS_NONE)
//	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_DATE_SEL_COMBO, 4 /* DropDown */, OnDropDownDateSelCombo, VTS_NONE)

// (a.walling 2008-05-13 10:31) - PLID 27591 - Use notify events for CDateTimePicker
// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CTemplateItemEntryGraphicalDlg, CNxDialog)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATE_SEL_COMBO, OnChangeDateSelCombo)
	ON_NOTIFY(DTN_CLOSEUP, IDC_DATE_SEL_COMBO, OnCloseUpDateSelCombo)
	ON_NOTIFY(DTN_DROPDOWN, IDC_DATE_SEL_COMBO, OnDropDownDateSelCombo)
	ON_BN_CLICKED(IDC_CLOSE_BTN, OnCloseBtn)
	ON_BN_CLICKED(IDC_MOVE_DAY_FORWARD, OnMoveDayForward)
	ON_BN_CLICKED(IDC_MOVE_DAY_BACK, OnMoveDayBack)
	ON_WM_CREATE()
	ON_MESSAGE(NXM_UPDATEVIEW, OnUpdateView)
	ON_BN_CLICKED(IDC_OPTIONS_BTN, OnOptionsBtn)
	ON_BN_CLICKED(IDC_CREATE_COLLECTION_FROM_VIEW_BTN, OnCreateCollectionFromViewBtn)
	ON_BN_CLICKED(IDC_REMOVE_TEMPLATES_BTN, OnRemoveTemplatesBtn)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_SCHEDULE_TEMPLATING_HELP, OnHelp)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
END_MESSAGE_MAP()

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CTemplateItemEntryGraphicalDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CTemplateItemEntryGraphicalDlg)
	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_INTERVAL, 16 /* SelChosen */, OnSelChosenInterval, VTS_DISPATCH)
	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_TEMPLATE_ITEM_ENTRY_CTRL, 4 /* ReservationRightClick */, OnReservationRightClickTemplateItemEntryCtrl, VTS_DISPATCH VTS_I4 VTS_I4)
	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_TEMPLATE_ITEM_ENTRY_CTRL, 1 /* ReservationClick */, OnReservationClickTemplateItemEntryCtrl, VTS_DISPATCH)
	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_TEMPLATE_ITEM_ENTRY_CTRL, 2 /* ReservationAdded */, OnReservationAddedTemplateItemEntryCtrl, VTS_DISPATCH)
	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_RESOURCE_LIST, 16 /* SelChosen */, OnSelChosenResourceList, VTS_DISPATCH)
	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_TEMPLATE_ITEM_ENTRY_CTRL, 10 /* ReservationEndDrag */, OnReservationEndDragTemplateItemEntryCtrl, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_TEMPLATE_ITEM_ENTRY_CTRL, 12 /* ReservationEndResize */, OnReservationEndResizeTemplateItemEntryCtrl, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_TEMPLATE_ITEM_ENTRY_CTRL, 9 /* ReservationDrag */, OnReservationDragTemplateItemEntryCtrl, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_TEMPLATE_ITEM_ENTRY_CTRL, 11 /* ReservationResize */, OnReservationResizeTemplateItemEntryCtrl, VTS_DISPATCH VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_TEMPLATE_ITEM_ENTRY_CTRL, -607 /* MouseUp */, OnMouseUpTemplateItemEntryCtrl, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_RESOURCE_LIST, 20 /* TrySetSelFinished */, OnTrySetSelFinishedResourceList, VTS_I4 VTS_I4)
	ON_EVENT(CTemplateItemEntryGraphicalDlg, IDC_TEMPLATE_ITEM_ENTRY_CTRL, -606 /* MouseMove */, OnMouseMoveTemplateItemEntryCtrl, VTS_I2 VTS_I2 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTemplateItemEntryGraphicalDlg message handlers

BOOL CTemplateItemEntryGraphicalDlg::OnInitDialog() 
{
	CWaitCursor wc;
	CNxDialog::OnInitDialog();

	try
	{		
		m_btnCreateCollectionFromView.AutoSet(NXB_NEW); // (c.haag 2014-12-17) - PLID 64246 - Set the button icon		
		m_btnRemoveTemplates.AutoSet(NXB_DELETE); // (c.haag 2014-12-17) - PLID 64253 - Set the button icon

		if (m_eEditorType == stetCollection)
		{
			// (z.manning 2014-12-10 08:43) - PLID 64210 - Use a different background color when editing collections to
			// differentiate it.
			g_brTemplateCollectionBackground.CreateSolidBrush(RGB(255, 242, 255));
		}

		//TES 6/19/2010 - PLID 5888 - We're going to be checking different permissions, depending on whether we're editing Resource
		// Availability templates.
		m_bio = m_bUseResourceAvailTemplates?bioResourceAvailTemplating:bioSchedTemplating;

		//TES 6/21/2010 - PLID 5888 - Update the window title to indicate whether these are Resource Availability templates
		if(m_bUseResourceAvailTemplates) {
			//(e.lally 2010-07-15) PLID 39626 - Renamed to Location Templates
			SetWindowText("Location Template Editor");
		}
		else if (m_eEditorType == stetCollection) {
			SetWindowText("Schedule Template Collection Manager");
		}

		if(m_pdlgTemplateList == NULL) {
			//TES 6/19/2010 - PLID 5888 - Pass in m_bUseResourceAvailTemplates
			// (z.manning 2014-12-03 14:36) - PLID 64332 - Pass in the type
			m_pdlgTemplateList = new CTemplatesDlg(m_eEditorType, this);
			m_pdlgTemplateList->SetDefaultCollectionID(m_bstrDefaultCollectionID); // (c.haag 2014-12-15) - PLID 64246
			m_pdlgTemplateList->Create(IDD_TEMPLATES_DLG, this);

			CRect rc;
			CWnd* pwndTemplateList = GetDlgItem(IDC_TEMPLATE_LIST_PLACEHOLDER);
			if(pwndTemplateList) {
				pwndTemplateList->GetWindowRect(&rc);
				ScreenToClient(&rc);
				m_pdlgTemplateList->MoveWindow(rc);
				m_pdlgTemplateList->BringWindowToTop();
			}
		}

		m_btnMoveDayForward.AutoSet(NXB_RIGHT);
		m_btnMoveDayBack.AutoSet(NXB_LEFT);
		// (z.manning, 04/29/2008) - PLID 29814 - Style for close button
		m_btnClose.AutoSet(NXB_CLOSE);

		// (c.haag 2015-02-05) - PLID 64255 - Show ellipses instead of overflowing large text to subsequent lines 
		m_nxlCollectionsUsed.SetSingleLine();

		UpdateVisibleControls();

		// (z.manning 2014-12-09 15:57) - PLID 64210 - Resources aren't applicable when editing collections
		if (m_eEditorType != stetCollection)
		{
			// (z.manning, 12/04/2006) - Load the resource list and set the selection to whatever was said by
			// the person 
			m_pdlResourceList = BindNxDataList2Ctrl(IDC_RESOURCE_LIST, false);
			LoadResourceList();
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new temporary trysetsel method
			long nTrySetSelResult = m_pdlResourceList->TrySetSelByColumn_Deprecated(rlcID, m_nDefaultResourceID);
			if (nTrySetSelResult >= 0) {
				// We found the row we were looking for.
				m_bResourceListLoaded = TRUE;
			}
			else if (nTrySetSelResult == NXDATALIST2Lib::sriNoRowYet_WillFireEvent) {
				// We're requerying, so the list is still loading.
				m_bResourceListLoaded = FALSE;
			}
			else {
				ASSERT(nTrySetSelResult == NXDATALIST2Lib::sriNoRow);
				// This list is done loading, but we couldn't find our resource. Let's use the top row.
				m_bResourceListLoaded = TRUE;
				SetResourceListSelectionAnyRow();
			}
		}

		m_lohOfficeHours.Init(GetCurrentLocationID());

		// (c.haag 2010-03-26 15:13) - PLID 37332 - Use SetDispatch
		m_pSingleDayCtrl.SetDispatch( (LPDISPATCH)GetDlgItem(IDC_TEMPLATE_ITEM_ENTRY_CTRL)->GetControlUnknown() );
		m_pSingleDayCtrl.PutDayTotalCount(__FUNCTION__, 1);

		// (a.walling 2007-11-14 14:19) - PLID 28059 - VS2008 - Bad binds; should be BindNxDataList2Ctrl.
		m_pdlInterval = BindNxDataList2Ctrl(IDC_INTERVAL, false);
		// Fill the combo that lists the possible time intervals
		m_pdlInterval->Clear();
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlInterval->GetNewRow();
		pRow->Value[0] = (long)5;
		m_pdlInterval->AddRowAtEnd(pRow, NULL);
		pRow = m_pdlInterval->GetNewRow();
		pRow->Value[0] = (long)10;
		m_pdlInterval->AddRowAtEnd(pRow, NULL);
		pRow = m_pdlInterval->GetNewRow();
		pRow->Value[0] = (long)15;
		m_pdlInterval->AddRowAtEnd(pRow, NULL);
		pRow = m_pdlInterval->GetNewRow();
		pRow->Value[0] = (long)20;
		m_pdlInterval->AddRowAtEnd(pRow, NULL);
		pRow = m_pdlInterval->GetNewRow();
		pRow->Value[0] = (long)30;
		m_pdlInterval->AddRowAtEnd(pRow, NULL);
		pRow = m_pdlInterval->GetNewRow();
		pRow->Value[0] = (long)60;
		m_pdlInterval->AddRowAtEnd(pRow, NULL);

		// (b.cardillo 2016-05-13 14:22) - NX-100239 - Eliminate the 40-minute interval option
		// Recall the selection from the last time, defaulting to 15 minute interval
		{
			long nInterval = GetRemotePropertyInt("GraphicalTemplateInterfaceInterval", 15, 0, GetCurrentUserName(), true);
			NXDATALIST2Lib::IRowSettingsPtr pSel = m_pdlInterval->SetSelByColumn(0, nInterval);
			// If we were unable to set the dropdown to the preferred interval, probably because whatever they had it 
			// set to in data doesn't exist as an option, then revert to the 15-minute default, which certainly exists.
			if (pSel == nullptr) {
				pSel = m_pdlInterval->SetSelByColumn(0, 15);
			}
			if (pSel != nullptr) {
				nInterval = VarLong(pSel->GetValue(0));
			} else {
				// This should never be hit because the 15-minute option has to exist; this isn't that important though, 
				// so rather than failing utterly, just be kind and pretend we're using the 15-minute option.
				nInterval = 15;
			}
			m_pSingleDayCtrl.PutInterval(nInterval);
		}

		COleDateTime dtDate = COleDateTime::GetCurrentTime();

		// (j.jones 2011-07-15 14:38) - PLID 39838 - set a default date, if we have one
		if(m_dtDefaultDate.GetStatus() == COleDateTime::valid) {
			dtDate = m_dtDefaultDate;
		}
		else {
			// (c.haag 2007-01-08 14:37) - PLID 24163 - Try to open to the active day
			// in the scheduler if possible
			CChildFrame *frmSched = GetMainFrame()->GetOpenViewFrame(SCHEDULER_MODULE_NAME);
			if(frmSched) {
				CSchedulerView *viewSched = (CSchedulerView*)frmSched->GetActiveView();
				if (viewSched) {
					COleDateTime dtTo;
					viewSched->GetActiveDateRange(dtDate, dtTo);
				}
			}
		}

		m_dtpActiveDate.SetValue(dtDate);

		// (z.manning, 01/18/2007) - If the active resource ID is -1 then either...
		//  1. We weren't passed a default resource, so the TrySetSel process will eventually handle this
		//     by setting the selection to the top row and then calling update view.
		//  2. We have no resource in which case the single day will be disabled anyway.
		// Ergo, we don't call UpdateView here if the active resource ID is -1.
		if(GetActiveResourceID() != -1 || m_eEditorType == stetCollection) {
			UpdateView();
		}

		// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
		m_btnToday.SetType(dtsHyperlink);
		m_btnToday.SetHzAlign(DT_CENTER);
		m_btnToday.SetText("Today");
		m_btnToday.SetSingleLine(true);

		// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
		m_btnAmPm.SetType(dtsHyperlink);
		m_btnAmPm.SetHzAlign(DT_CENTER);
		m_btnAmPm.SetText("12:00 PM");
		m_btnAmPm.SetSingleLine(true);


		m_bGoToAM = TRUE;
		OnAmPmBtn();

		// (z.manning, 11/14/2006) - Call this again since we added the template list dialog to this dialog.
		GetControlPositions();

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnInitDialog");
	// (v.maida 2016-06-06 15:55) - NX-100631 - Set a larger default size so that the dialog is easier to read if it's restored down to a smaller size.
	SetWindowPos(NULL, 0, 0, 994, 738, 0);
	ShowWindow(SW_SHOWMAXIMIZED);

	if (m_pSingleDayCtrl.GetEnabled()) {
		PutFocusOnSingleDay();
		return FALSE;
	}
	else {
		return TRUE;
	}
}

int CTemplateItemEntryGraphicalDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// (a.walling 2008-05-23 12:54) - PLID 30099
	if (CNxDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	try {
		GetControlPositions();
	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnCreate");
	
	return 0;
}

BOOL CTemplateItemEntryGraphicalDlg::DestroyWindow() 
{
	if(m_pdlgTemplateList != NULL) {
		m_pdlgTemplateList->DestroyWindow();
		delete m_pdlgTemplateList;
		m_pdlgTemplateList = NULL;
	}
	
	return CNxDialog::DestroyWindow();
}

void CTemplateItemEntryGraphicalDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {

		// (z.manning 2009-07-10 15:13) - PLID 22054 - No point to doing this if we're not visible.
		if(!m_bVisible) {
			return;
		}

		// (z.manning, 03/13/2007) - PLID 23635 - Set the visible range of the single day.
		SetVisibleTimeRange();

		m_pSingleDayCtrl.PutSnapReservationsToGrid(VARIANT_FALSE);
		m_pSingleDayCtrl.PutShowTooltips(VARIANT_FALSE);
		m_pSingleDayCtrl.PutShowArrows(GetRemotePropertyInt("ShowAppointmentArrows", 1, 0, "<None>", true) == 0 ? VARIANT_FALSE : VARIANT_TRUE);
		m_pSingleDayCtrl.PutRepositionText(VARIANT_TRUE);
		m_pSingleDayCtrl.PutCompactBlockText(VARIANT_TRUE);
		// (b.cardillo 2016-06-01 14:11) - NX-100771 - Obliterate the old scheduler custom border width preference

		UpdateTemplateReservations();
		
		if(m_pSingleDayCtrl.GetSlotTotalCount() >= 20) {
			m_pSingleDayCtrl.PutSlotVisibleCount(20);  // Chosen arbirtarily
		}
		else {
			m_pSingleDayCtrl.PutSlotVisibleCount(m_pSingleDayCtrl.GetSlotTotalCount());
		}
		
		UpdateDayOfWeekText();

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::UpdateView");
}

// (z.manning, 12/07/2006) - PLID 23805 -  Returns the the new reservation object if pRes is split in 2, returns NULL otherwise.
CReservation CTemplateItemEntryGraphicalDlg::SplitNonBlockReservation(CReservation pRes, COleDateTime dtSplitStart, COleDateTime dtSplitEnd)
{
	CReservation pNewRes = CReservation(__FUNCTION__, NULL);
	COleDateTime dtExistingResStart = pRes.GetStartTime();
	COleDateTime dtExistingResEnd = pRes.GetEndTime();
	// The new reservation has the exact same times as an existing reservation
	if( (CompareTimes(dtSplitStart,dtExistingResStart) & CT_EQUAL)
		&& (CompareTimes(dtSplitEnd,dtExistingResEnd) & CT_EQUAL) )
	{
		// Just collapse the existing reservation.  It will be removed later.
		pRes.PutEndTime(dtSplitStart);
	}
	// The new reservation falls in the middle of this reservation.
	else if( (CompareTimes(dtSplitStart,dtExistingResStart) & CT_GREATER_THAN)\
		&& (CompareTimes(dtSplitEnd,dtExistingResEnd) & CT_LESS_THAN) )
	{
		// We need to split the existing reservation into 2.
		BOOL bExistingResIsBlock;
		m_mapLineItemIDToIsBlock.Lookup(pRes.GetReservationID(), bExistingResIsBlock);
		pNewRes = AddReservation(__FUNCTION__, dtSplitEnd, dtExistingResEnd, pRes.GetReservationID(), (LPCTSTR)pRes.GetText(), pRes.GetBackColor(), bExistingResIsBlock);
		pRes.PutEndTime(dtSplitStart);
	}
	// The new reservation overlaps the top of this existing reservation.
	else if( (CompareTimes(dtSplitStart,dtExistingResEnd) & CT_LESS_THAN)
		&& (CompareTimes(dtSplitEnd,dtExistingResEnd) & CT_GREATER_THAN_OR_EQUAL) )
	{
		pRes.PutEndTime(dtSplitStart);
	}
	// The new reservation overlaps the bottom of this existing reservation.
	else if( (CompareTimes(dtSplitEnd,dtExistingResStart) & CT_GREATER_THAN)
		&& (CompareTimes(dtSplitStart,dtExistingResStart) & CT_LESS_THAN_OR_EQUAL) )
	{
		pRes.PutStartTime(dtSplitEnd);
	}

	return pNewRes;
}

void CTemplateItemEntryGraphicalDlg::UpdateTemplateReservations()
{
	CWnd *pSingleDayWnd = CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd());
	
	// (z.manning, 11/10/2006) - Wipe out all reservation and blocks on the single day.
	pSingleDayWnd->SetRedraw(FALSE);
	m_pSingleDayCtrl.Clear(__FUNCTION__);
	m_pSingleDayCtrl.RemoveBlock(-1);

	m_mapLineItemIDToPriority.RemoveAll();
	m_mapLineItemIDToIsBlock.RemoveAll();
	m_mapLineItemIDToFromCollection.RemoveAll(); // (c.haag 2014-12-17) - PLID 64294

	long nWorkingResourceID = GetActiveResourceID();

	// Add the template for office hours.
	COleDateTime dtOpen, dtClose;
	if (m_lohOfficeHours.GetOfficeHours(GetActiveDate().GetDayOfWeek() - 1, dtOpen, dtClose)) {
		m_pSingleDayCtrl.AddBlock(0, dtOpen, dtClose, DEFAULT_OFFICE_HOURS_TEMPLATE_COLOR, "", TRUE);
	}

	CArray<long, long> arynResourceIDs;
	CMap<long, long, long, long> mapTemplateItemIDToTemplateID;
	CMap<long, long, CString, LPCTSTR> mapLineItemIDToText;
	CMap<long, long, OLE_COLOR, OLE_COLOR> mapLineItemIDToColor;
	CArray<CTemplateLineItemInfo*, CTemplateLineItemInfo*> arypLineItems;
	
	if (m_eEditorType == stetCollection)
	{
		_SchedulerTemplateCollectionPtr pCollection = m_pdlgTemplateList->GetSelectedCollection();
		if (pCollection == NULL)
		{
			// (z.manning 2014-12-04 11:40) - PLID 64215 - Disable the single day if no collection is selected
			m_pSingleDayCtrl.PutEnabled(VARIANT_FALSE);
		}
		else
		{
			m_pSingleDayCtrl.PutEnabled(VARIANT_TRUE);

			// (z.manning 2014-12-09 12:40) - PLID 64423 - Although resource ID is meaningless when editing collections,
			// add the dummy value anyway just to ensure the later loop executes. We don't actually use the resource
			// ID for anything when editing collections.
			arynResourceIDs.Add(nWorkingResourceID);

			// (z.manning 2014-12-09 12:40) - PLID 64423 - Load all the templates for this collection
			Nx::SafeArray<IUnknown*> saCollectionTemplates(pCollection->Templates);
			for each (_SchedulerTemplateCollectionTemplatePtr pCollectionTemplate in saCollectionTemplates)
			{
				CTemplateLineItemInfo *pLineItem = new CTemplateLineItemInfo;
				pLineItem->m_nTemplateID = AsLong(pCollectionTemplate->Template->ID);
				pLineItem->m_nID = AsLong(pCollectionTemplate->ID);
				mapTemplateItemIDToTemplateID.SetAt(pLineItem->m_nID, pLineItem->m_nTemplateID);
				pLineItem->m_dtStartTime = pCollectionTemplate->StartTime;
				pLineItem->m_dtEndTime = pCollectionTemplate->EndTime;
				pLineItem->m_bIsBlock = pCollectionTemplate->IsBlock ? TRUE : FALSE;
				arypLineItems.Add(pLineItem);

				mapLineItemIDToText.SetAt(pLineItem->m_nID, pCollectionTemplate->Template->Name);
				mapLineItemIDToColor.SetAt(pLineItem->m_nID, ConvertHexStringToCOLORREF(pCollectionTemplate->Template->Color));
				m_mapLineItemIDToPriority.SetAt(pLineItem->m_nID, pCollectionTemplate->Template->Priority);
				m_mapLineItemIDToIsBlock.SetAt(pLineItem->m_nID, pLineItem->m_bIsBlock);
			}
		}
	}
	else
	{
		// Then read the real templates
		CTemplateHitSet rsTemplateHits;

		// (c.haag 2014-12-16) - PLID 64255 - Clear out the Used Collections array so we can populate it with
		// template hit results.
		m_UsedCollections.RemoveAll();

		if (m_bLoadAllResources) {
			// (z.manning 2009-07-20 19:00) - PLID 22054 - Even though we only show one resource at a time on
			// this dialog, I added the ability to load all resources at once as an optimization for the Time
			// Scheduled Productivity by Template report since it uses this dialog and the template splitting
			// logic to get the available time for each day.
			GetAllLoadedResourceIDs(arynResourceIDs);
			// (a.walling 2010-06-25 12:36) - PLID 39278 - Exclude resource availability templates if necessary
			rsTemplateHits.Requery(GetActiveDate(), arynResourceIDs, FALSE, -1, m_bUseResourceAvailTemplates ? false : true, m_bUseResourceAvailTemplates);
		}
		else {
			arynResourceIDs.Add(nWorkingResourceID);
			// (z.manning, 05/24/2007) - PLID 26062 - Added parameter to requery call as it's no longer optional.
			// (a.walling 2010-06-25 12:36) - PLID 39278 - Exclude resource availability templates if necessary
			rsTemplateHits.Requery(GetActiveDate(), nWorkingResourceID, FALSE, -1, m_bUseResourceAvailTemplates ? false : true, m_bUseResourceAvailTemplates);
		}		

		// (z.manning 2014-12-09 12:12) - PLID 64423 - Moved the template loading logic here and now keep track of the
		// line items in an array and we'll process the array later.
		if (m_eEditorType != stetCollection && !rsTemplateHits.IsBOF())
		{
				rsTemplateHits.MoveFirst();
				CArray<TemplateItemReservationInfo, TemplateItemReservationInfo> aryBlocks;
				while (!rsTemplateHits.IsEOF())
				{
					
					// (s.tullis 2015-06-17 11:30) - PLID 66411- We don't want to add duplicate Lineitems
					//  If the Template lineitem has not been added.. add it
					//  Else just add the this resource assigned to the Lineitem
					CTemplateLineItemInfo *pLineItem = FindLineItemInArray(arypLineItems, rsTemplateHits.GetTemplateID(), rsTemplateHits.GetLineItemID());
					if (pLineItem == NULL)
					{	
						CTemplateLineItemInfo *pLineItem = new CTemplateLineItemInfo;
						long nCollectionID = rsTemplateHits.GetCollectionID();
						pLineItem->m_nTemplateID = rsTemplateHits.GetTemplateID();
						pLineItem->m_nID = rsTemplateHits.GetLineItemID();
						mapTemplateItemIDToTemplateID.SetAt(pLineItem->m_nID, pLineItem->m_nTemplateID);
						pLineItem->m_dtStartTime = rsTemplateHits.GetStartTime();
						pLineItem->m_dtEndTime = rsTemplateHits.GetEndTime();
						pLineItem->m_bIsBlock = rsTemplateHits.GetIsBlock();
						// (z.manning 2014-12-18 16:15) - PLID 64256 - Set the collection ID
						pLineItem->m_nCollectionID = nCollectionID;
						pLineItem->AddResource(rsTemplateHits.GetResourceID(), "");
						arypLineItems.Add(pLineItem);
						// (c.haag 2014-12-16) - PLID 64255 - Read in collection fields and add them to m_UsedCollections.
						// It's important that we include the -1 collection ID too -- the label generation needs to know whether
						// non-collection template items are present.					
						{
							BOOL bFound = FALSE;
							for (int i = 0; i < m_UsedCollections.GetSize() && !bFound; i++)
							{
								if (m_UsedCollections[i].ID == nCollectionID)
								{
									bFound = TRUE;
								}
							}
							if (!bFound)
							{
								UsedCollection c = { nCollectionID, rsTemplateHits.GetCollectionName() };
								m_UsedCollections.Add(c);
							}
						}

						mapLineItemIDToText.SetAt(pLineItem->m_nID, rsTemplateHits.GetText());
						mapLineItemIDToColor.SetAt(pLineItem->m_nID, rsTemplateHits.GetColor());
						m_mapLineItemIDToPriority.SetAt(pLineItem->m_nID, rsTemplateHits.GetPriority());
						m_mapLineItemIDToIsBlock.SetAt(pLineItem->m_nID, pLineItem->m_bIsBlock);
						m_mapLineItemIDToFromCollection.SetAt(pLineItem->m_nID, rsTemplateHits.GetCollectionID() > 0 ); // (c.haag 2014-12-17) - PLID 64294
					}
					else{
						// (s.tullis 2015-06-17 11:30) - PLID 66411 - Add the Resource ID to our Line Item
						pLineItem->AddResource(rsTemplateHits.GetResourceID(), "");

					}

					rsTemplateHits.MoveNext();
				}
			

			// (c.haag 2014-12-16) - PLID 64255 - Sort the contents of the used collection array
			if (m_UsedCollections.GetCount() > 0)
			{
				qsort(m_UsedCollections.GetData(), m_UsedCollections.GetSize(), sizeof(UsedCollection), CompareUsedCollection);
			}
		}
	}
	// (s.tullis 2015-08-24 14:37) - PLID 66411 - Now we need to load the templates in the editor per resource
	const int narrResourceCount = arynResourceIDs.GetSize();
	for (int nResourceIndex = 0; nResourceIndex < narrResourceCount; nResourceIndex++)
	{
		CArray<TemplateItemReservationInfo, TemplateItemReservationInfo> aryBlocks;
		for (int nLineItemIndex = 0; nLineItemIndex < arypLineItems.GetCount(); nLineItemIndex++)
		{
			CTemplateLineItemInfo *pLineItem = arypLineItems.GetAt(nLineItemIndex);
			// (s.tullis 2015-08-24 14:37) - PLID 66411 
			// We don't want Template lineitems that don't apply to the current resource
			// We only want to filter resources if this being loaded for all resources
			// m_bLoadAllResources is only TRUE when this dialog is loaded for the Time Scheduled Productivity By Scheduler Templates
			if (m_bLoadAllResources && pLineItem->GetResourceIndexByID(arynResourceIDs.GetAt(nResourceIndex)) == -1)
			{
				continue;
			}
			long nPriority = 0;
			m_mapLineItemIDToPriority.Lookup(pLineItem->m_nID, nPriority);
			OLE_COLOR dwColor = 0;
			mapLineItemIDToColor.Lookup(pLineItem->m_nID, dwColor);
			CString strText;
			mapLineItemIDToText.Lookup(pLineItem->m_nID, strText);

			// (z.manning, 11/20/2006) - PLID 23611 - Load all non-blocks first and save the rest for later.
			if (pLineItem->m_bIsBlock) {
				aryBlocks.Add(TemplateItemReservationInfo(pLineItem->m_nID, pLineItem->m_dtStartTime, pLineItem->m_dtEndTime, nPriority, strText, dwColor));
			}
			else {
				AddReservation(__FUNCTION__, pLineItem->m_dtStartTime, pLineItem->m_dtEndTime, pLineItem->m_nID, strText, dwColor, pLineItem->m_bIsBlock);
				// (z.manning, 11/02/2006) - PLID 23805 - We only show the top priority for a given time, so we may
				// need to remove or split lower priority template reservations.  Note: we subtract 1 from
				// the reservation count because we don't want to modify the one we just added.
				long nCount = m_pSingleDayCtrl.GetReservationCount(0);
				for (int nResIndex = 0; nResIndex < nCount; nResIndex++) {
					CReservation pExistingRes = m_pSingleDayCtrl.GetReservation(__FUNCTION__, 0, nResIndex);
					if (pExistingRes != NULL && pExistingRes.GetReservationID() != pLineItem->m_nID) {

						// We need to split this existing reservation in case there's any overlap with
						// the higer priority reservation we just added.
						CReservation pSplitRes = SplitNonBlockReservation(pExistingRes, pLineItem->m_dtStartTime, pLineItem->m_dtEndTime);

						if (CompareTimes(pExistingRes.GetStartTime(), pExistingRes.GetEndTime()) & CT_GREATER_THAN_OR_EQUAL) {
							// If this is true then an existing reservation was completely overwritten, so get rid of it
							// and then make sure we adjust the count accordingly.
							pExistingRes.DeleteRes(__FUNCTION__);
							pExistingRes.SetDispatch(NULL);
							nCount--;
							nResIndex--;
						}
					}
				}
			}
		}
		// (z.manning, 11/20/2006) - PLID 23611 - Ok, we've loaded all the non-blocks, now it's time to load the
		// blocks. Here is how we will do so:
		//  -- If a block has a higher priority than the other template(s) at the same time, we load the block
		//     and remove the lower priority template for the blocks's time range.
		//  -- As long as a block is the highest priority at a give time, there is no limit to the number of 
		//     blocks that can be loaded.
		//  -- If a only part of a block is the highest priority (e.g. half of it falls on a template with a higher
		//     priority and the other half falls on untemplated space), we truncate the block.
		//  (Much of this algorithm is in SplitBlockReservation().)
		// We start at the end of the array because that's where the higest priority blocks are and we want
		// to load them in order of priority from left to right.
		for (int nBlockIndex = aryBlocks.GetSize() - 1; nBlockIndex >= 0; nBlockIndex--)
		{
			// Get info for the next block.
			long nBlockPriority = aryBlocks.GetAt(nBlockIndex).m_nPriority;
			COleDateTime dtNewResStart = aryBlocks.GetAt(nBlockIndex).m_dtStartTime;
			COleDateTime dtNewResEnd = aryBlocks.GetAt(nBlockIndex).m_dtEndTime;

			CReservation pBlock = AddReservation(__FUNCTION__, dtNewResStart, dtNewResEnd, aryBlocks.GetAt(nBlockIndex).m_nID,
				aryBlocks.GetAt(nBlockIndex).m_strText, aryBlocks.GetAt(nBlockIndex).m_dwColor, TRUE);

			// (c.haag 2010-07-14 16:31) - PLID 39615 - Completely relinquish our ownership of the dispatch
			// in pBlock before calling SplitBlockReservation.
			LPDISPATCH theBlock = pBlock.GetDispatch();
			pBlock.ReleaseAndClear();
			SplitBlockReservation(theBlock, dtNewResStart, dtNewResEnd, nBlockPriority);
		}
		// (z.manning 2009-07-10 11:38) - PLID 22054 - If we loaded all resources then let's also get
		// the total templated minutes for each resource. (Used by the Time Sched Productivity by
		// Template report).
		if (m_bLoadAllResources)
		{
			long nTotalMinutes = 0;
			const long nResCount = m_pSingleDayCtrl.GetReservationCount(0);
			// (z.manning 2009-07-20 19:06) - PLID 22054 - Loop through all loaded reservations and
			// add the total time for all visible templates including adding precision templates that
			// may overlap as that is considered double available time per Meikin.
			for (int nResIndex = 0; nResIndex < nResCount; nResIndex++) {
				CReservation pRes = m_pSingleDayCtrl.GetReservation(__FUNCTION__, 0, nResIndex);
				long nCurrentTemplateItemID = pRes.GetReservationID();
				long nCurrentTemplateID = -1;
				if (!mapTemplateItemIDToTemplateID.Lookup(nCurrentTemplateItemID, nCurrentTemplateID)) {
					ASSERT(FALSE);
				}
				// (s.tullis 2015-08-24 14:37) - PLID 66411 
				// Add the Template start and end times of our current Resource to the Array
				// Check the template filter disregard Templates not in the filter array
				if (m_arynTemplateIDFilter.GetSize() == 0 || IsIDInArray(nCurrentTemplateID, m_arynTemplateIDFilter)){
					ResourceStartEndTime rsetNew;
					rsetNew.nResourceID = arynResourceIDs.GetAt(nResourceIndex);
					rsetNew.dtStartTime = pRes.GetStartTime();
					rsetNew.dtEndTime = pRes.GetEndTime();
					m_aryResourceStartEndTimes.Add(rsetNew);
				}
				
			
			}
			// (z.manning 2009-07-20 19:07) - PLID 22054 - Need to clear out the single day when going to the
			// next resource.
			m_pSingleDayCtrl.Clear(__FUNCTION__);
			m_pSingleDayCtrl.RemoveBlock(-1);
		}

	}
	// (z.manning 2014-12-09 12:19) - PLID 64423 - Clean up memory
	for (int nLineItemIndex = 0; nLineItemIndex < arypLineItems.GetCount(); nLineItemIndex++) {
		CTemplateLineItemInfo *pLineItem = arypLineItems.GetAt(nLineItemIndex);
		delete pLineItem;
	}
	arypLineItems.RemoveAll();

	m_pSingleDayCtrl.Resolve();
	m_pSingleDayCtrl.Refresh();
	pSingleDayWnd->SetRedraw(TRUE);
	m_pSingleDayCtrl.ShowBlock(-1);
	pSingleDayWnd->RedrawWindow();

	// (c.haag 2014-12-16) - PLID 64255 - Update the Used Collections label now that we've populated
	// the used collections array.
	UpdateUsedCollectionsLabel();
}

// (c.haag 2015-01-02) - PLID 64257 - Returns true if a collection is used in the current view
bool CTemplateItemEntryGraphicalDlg::IsCollectionUsed(long nID)
{
	for (int i = 0; i < m_UsedCollections.GetCount(); i++)
	{
		if (m_UsedCollections[i].ID == nID)
		{
			return true;
		}
	}
	return false;
}

// (c.haag 2014-12-16) - PLID 64255 - Update the Used Collections label based on the contents of m_UsedCollections
// (c.haag 2015-01-02) - PLID 64257 - We now hide the Collections label if it shouldn't be visible
void CTemplateItemEntryGraphicalDlg::UpdateUsedCollectionsLabel()
{
	if (m_eEditorType != stetNormal)
	{
		m_nxlCollectionsUsed.ShowWindow(SW_HIDE);
	}
	else
	{
		m_nxlCollectionsUsed.ShowWindow(SW_SHOW);
		if (0 == m_UsedCollections.GetSize() || (1 == m_UsedCollections.GetSize() && m_UsedCollections[0].ID == -1))
		{
			// No line items or no collection line items
			m_nxlCollectionsUsed.SetText("Collections Used: < None >");
			m_nxlCollectionsUsed.SetToolTip("");
			m_nxlCollectionsUsed.SetType(dtsHyperlink);
			m_nxlCollectionsUsed.SetTextColorOverride(::GetSysColor(COLOR_HOTLIGHT));
		}
		else
		{
			CString str = "Collections Used: ";
			BOOL bHasNonCollectionItems = FALSE;
			for (int i = 0; i < m_UsedCollections.GetSize(); i++)
			{
				if (m_UsedCollections[i].ID == -1)
				{
					bHasNonCollectionItems = TRUE;
				}
				else
				{
					str.Append(m_UsedCollections[i].Name);
					if (i < m_UsedCollections.GetSize() - 1)
					{
						str.Append(", ");
					}
				}
			}

			m_nxlCollectionsUsed.SetText(str);
			m_nxlCollectionsUsed.SetToolTip(str);
			m_nxlCollectionsUsed.SetType(dtsHyperlink);
			m_nxlCollectionsUsed.SetTextColorOverride(bHasNonCollectionItems ? RGB(255, 0, 0) : ::GetSysColor(COLOR_HOTLIGHT));
		}
		m_nxlCollectionsUsed.Invalidate();
	}
}

void CTemplateItemEntryGraphicalDlg::LoadResourceList()
{
	CString strWhere = "Inactive = 0 ";

	// (z.manning, 12/05/2006) - Well, I had this implemented originally to exclude resources that
	// the user did not have permissions for even though they could still technically edit templates
	// for these resources using the non-graphical editor. But c.haag and I agreed this was inconsistent
	// so I'm leaving it out for the time being.
	/*
	// Generate a list of the resource IDs this user is not allowed to see
	CString strHiddenResourceIDs = CalcExcludedResourceIDs();
	// Now add to the where clause if there are any resources that need to explicitly be excluded
	if (!strHiddenResourceIDs.IsEmpty()) {
		strWhere += FormatString(" AND ResourceT.ID NOT IN (%s) ", strHiddenResourceIDs);
	}
	*/

	m_pdlResourceList->WhereClause = _bstr_t(strWhere);
	m_pdlResourceList->Requery();
}

void CTemplateItemEntryGraphicalDlg::OnCloseBtn() 
{
	CDialog::OnCancel();	
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CTemplateItemEntryGraphicalDlg::OnChangeDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CWaitCursor wc;
	try {

		if(m_bIsDateSelComboDroppedDown) {
			// The combo is dropped down and we've been asked not to refresh 
			// until the dropdown closes up, so do nothing
		} 
		else {
			// We're good to go, do the refresh now
			UpdateView();
		}

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnChangeDateSelCombo");

	*pResult = 0;
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CTemplateItemEntryGraphicalDlg::OnCloseUpDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CWaitCursor wc;
	try {

		// Let everyone be able to check and discover the date combo is not dropped down anymore
		m_bIsDateSelComboDroppedDown = FALSE;

		UpdateView();

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnCloseUpDateSelCombo");
	PutFocusOnSingleDay();

	*pResult = 0;
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CTemplateItemEntryGraphicalDlg::OnDropDownDateSelCombo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Let everyone be able to check and discover the date combo is dropped down now
	m_bIsDateSelComboDroppedDown = TRUE;

	*pResult = 0;
}

void CTemplateItemEntryGraphicalDlg::OnMoveDayForward() 
{
	CWaitCursor wc;
	try {

		COleDateTime dtNextDay = GetActiveDate() + COleDateTimeSpan(1,0,0,0);
		m_dtpActiveDate.SetValue(dtNextDay);
		UpdateView();

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnMoveDayForward");
	PutFocusOnSingleDay();
}

void CTemplateItemEntryGraphicalDlg::OnMoveDayBack() 
{
	CWaitCursor wc;
	try {

		COleDateTime dtPreviousDay = GetActiveDate() - COleDateTimeSpan(1,0,0,0);
		m_dtpActiveDate.SetValue(dtPreviousDay);
		UpdateView();

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnMoveDayBack");
	PutFocusOnSingleDay();
}

// (c.haag 2010-04-30 12:30) - PLID 38379 - We now take in an owner
CReservation CTemplateItemEntryGraphicalDlg::AddReservation(const CString& strOwner, COleDateTime dtStart, COleDateTime dtEnd, long nID, CString strText, OLE_COLOR dwColor, BOOL bIsBlock)
{
	CReservation pRes = m_pSingleDayCtrl.AddReservation(strOwner, 0, dtStart, dtEnd, dwColor, TRUE);

	if(pRes) {
		pRes.PutReservationID(nID);

		// (c.haag 2014-12-17) - PLID 64294 - Show whether this is a collection template
		BOOL bFromCollection = FALSE;
		m_mapLineItemIDToFromCollection.Lookup(nID, bFromCollection);
		if (bFromCollection && strText.Find(" (Collection Template)") == -1) {
			// Ensure this text is not bold.
			strText += "%0b (Collection Template)";
		}

		if(bIsBlock && strText.Find(" (Precision Template)") == -1) {
			// Ensure this text is not bold.
			strText += "%0b (Precision Template)";
		}

		MakeResLookLikeTemplateItem(pRes, strText, dwColor);

		return pRes;
	}
	else {
		AfxThrowNxException("CTemplateItemEntryGraphicalDlg::UpdateTemplateReservations - Failed to create ReservationPtr");
	}

	return CReservation(strOwner, NULL);
}

void CTemplateItemEntryGraphicalDlg::OnTodayBtn() 
{
	CWaitCursor wc;
	try {

		// (c.haag 2014-01-12) - PLID 64257 - Only set the date
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dt;
		dt.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());
		m_dtpActiveDate.SetValue(dt);
		UpdateView();

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnTodayBtn");
	PutFocusOnSingleDay();
}

void CTemplateItemEntryGraphicalDlg::OnAmPmBtn() 
{
	CWaitCursor wc;
	try {

		if (m_pSingleDayCtrl != NULL) {
			COleDateTime dtNewTime, dtAmTime, dtPmTime;
			//Are we calculating?
			if(GetRemotePropertyInt("CalculateAmPmTime", 0, 0, "<None>", true)) {
				//OK, the A.M. time will either be the opening time, or, if the office is closed, 8:00 AM
				if(!m_lohOfficeHours.GetOfficeHours(GetActiveDate().GetDayOfWeek() - 1, dtAmTime, dtPmTime)) {
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

			if (m_bGoToAM) {
				// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
				m_btnAmPm.SetText(FormatDateTimeForInterface(dtPmTime, DTF_STRIP_SECONDS, dtoTime));
				dtNewTime = dtAmTime;
				m_bGoToAM = FALSE;
			} 
			else {
				// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
				m_btnAmPm.SetText(FormatDateTimeForInterface(dtAmTime, DTF_STRIP_SECONDS, dtoTime));
				dtNewTime = dtPmTime;
				m_bGoToAM = TRUE;
			}
			int nNewSlot;
			nNewSlot = m_pSingleDayCtrl.GetTimeSlot(dtNewTime);
			m_pSingleDayCtrl.PutTopSlot(nNewSlot);
		}

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnAmPmBtn");
	PutFocusOnSingleDay();
}

void CTemplateItemEntryGraphicalDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CTemplateItemEntryGraphicalDlg::OnSelChosenInterval(LPDISPATCH lpRow) 
{
	CWaitCursor wc;
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			// If the selection is invalid, force it to be valid
			m_pdlInterval->SetSelByColumn(0, (long)15);
		}

		long nCurInterval = VarLong(pRow->GetValue(0));

		if (m_pSingleDayCtrl.GetInterval() == nCurInterval) {
			return;
		}

		m_pSingleDayCtrl.PutInterval(nCurInterval);
			
		// (z.manning, 11/07/2006) - This is very different from the scheduler, so we'll use one global settting to remember this.
		SetRemotePropertyInt("GraphicalTemplateInterfaceInterval", nCurInterval, 0, GetCurrentUserName());
		UpdateView();

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnSelChosenInterval");

	if (m_pSingleDayCtrl.GetEnabled()) {
		PutFocusOnSingleDay();
	}
	else {
		::SetFocus(NULL);
	}
}

void CTemplateItemEntryGraphicalDlg::OnReservationClickTemplateItemEntryCtrl(LPDISPATCH theRes) 
{
	CWaitCursor wc;
	CTemplateLineItemInfo *pLineItem = NULL;
	try
	{
		// (z.manning, 12/11/2006) - PLID 23768 - There is currently no way to make the template item
		// dialog read-only, so just restrict access to it altogether. (I entered PLID 23831 to give
		// the line item dialog a read only ablility so this can be improved in the future.)
		if(!CheckCurrentUserPermissions(m_bio,sptWrite)) {
			return;
		}

		// (c.haag 2010-06-29 13:48) - PLID 39393 - Only keep the reservation for as long as we need it
		CReservation pRes(__FUNCTION__, theRes);
		long nLineItemID = pRes.GetReservationID();
		pRes.ReleaseAndClear();

		if (m_eEditorType == stetCollection)
		{
			// (z.manning 2014-12-17 10:09) - PLID 64427 - Handle editing collection templates
			_SchedulerTemplateCollectionTemplatePtr pCollectionTemplate = m_pdlgTemplateList->GetCollectionTemplateByID(nLineItemID);
			if (pCollectionTemplate != NULL)
			{
				CTemplateSelectDlg dlgTemplateSelect(m_eEditorType, this);
				dlgTemplateSelect.m_nSelectedTemplateID = AsLong(pCollectionTemplate->Template->ID);
				dlgTemplateSelect.m_dtStartTime = pCollectionTemplate->StartTime;
				dlgTemplateSelect.m_dtEndTime = pCollectionTemplate->EndTime;
				dlgTemplateSelect.m_bIsBlock = pCollectionTemplate->IsBlock ? TRUE : FALSE;
				if (dlgTemplateSelect.DoModal() == IDOK)
				{
					CommitCollectionTemplate(nLineItemID, &dlgTemplateSelect);
				}
			}
		}
		else
		{
			// (c.haag 2006-12-11 11:49) - PLID 23808 - Gracefully fail if the line item no longer exists
			if (!ReturnsRecordsParam(FormatString("SELECT TOP 1 ID FROM %sTemplateItemT WHERE ID = {INT}", m_bUseResourceAvailTemplates ? "ResourceAvail" : ""), nLineItemID)) {
				MessageBox("This line item has been deleted by another user.");
				UpdateView();
				return;
			}

			pLineItem = new CTemplateLineItemInfo;
			//TES 6/19/2010 - PLID 5888 - Tell it whether to use Resource Availability templates
			pLineItem->m_bUseResourceAvailTemplates = m_bUseResourceAvailTemplates;
			pLineItem->LoadFromID(nLineItemID);
			CTemplateLineItemDlg dlgLineItem(m_eEditorType, this);
			if (dlgLineItem.ZoomLineItem(pLineItem) == IDOK) {

				// (z.manning, 11/10/2006) - Save the line item.
				try {
					ExecuteSql(pLineItem->GenerateSaveString());
					// (c.haag 2006-12-12 10:58) - PLID 23808 - Notify other computers about the change
					//TES 6/21/2010 - PLID 5888 - Send the appropriate tablechecker
					CClient::RefreshTable(m_bUseResourceAvailTemplates ? NetUtils::ResourceAvailTemplateItemT : NetUtils::TemplateItemT, pLineItem->m_nID);
				}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnReservationClickTemplateItemEntryCtrl - Failed to save line item.");

				RefreshEverything();
			}
		}
	}
	NxCatchAll("CTemplateItemEntryGraphicalDlg::OnReservationClickTemplateItemEntryCtrl");

	if(pLineItem) {
		delete pLineItem;
	}
	PutFocusOnSingleDay();
}

void CTemplateItemEntryGraphicalDlg::OnReservationRightClickTemplateItemEntryCtrl(LPDISPATCH theRes, long x, long y) 
{
	try {
		// (c.haag 2010-06-29 09:23) - PLID 39393 - Pass in the dispatch directly		
		PopupContextMenu(theRes);

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnReservationRightClickTemplateItemEntryCtrl");
}

// (c.haag 2010-06-29 09:23) - PLID 39393 - We now take in a dispatch instead of a CReservation
void CTemplateItemEntryGraphicalDlg::PopupContextMenu(LPDISPATCH theRes)
{	
	// (z.manning, 11/28/2006) - PLID 23665 - We need a reservation pointer in order to enable certain menu options.
	UINT nHaveResEnableMenu;
	if(theRes != NULL) {
		nHaveResEnableMenu = MF_ENABLED;
	}
	else {
		nHaveResEnableMenu = MF_GRAYED;
	}

	// (z.manning, 11/28/2006) - Only enable the paste option if they've already copied something.
	// (z.manning 2014-12-18 08:46) - PLID 64225 - Handle collection template copying too
	UINT nEnablePaste;
	if (m_pClippedLineItem != NULL || m_pClippedCollectionTemplate != NULL) {
		// We have a copied line item!
		nEnablePaste = MF_ENABLED;
	}
	else {
		// Gotta copy before you can paste.
		nEnablePaste = MF_GRAYED;
	}

	UINT nEnableCopyAll;
	if (m_pSingleDayCtrl.GetReservationCount(0) > 0){
		nEnableCopyAll = MF_ENABLED;
	} else {
		nEnableCopyAll = MF_GRAYED;
	}
	UINT nEnablePasteAll;
	if (m_aryToBeCopied.GetSize() > 0){
		nEnablePasteAll = MF_ENABLED;
	} else {
		nEnablePasteAll = MF_GRAYED;
	}

	CMenu mnuPopup;
	mnuPopup.CreatePopupMenu();
	mnuPopup.AppendMenu(nHaveResEnableMenu|MF_STRING|MF_BYPOSITION, miEdit, "Edit...");
	mnuPopup.AppendMenu(MF_SEPARATOR|MF_BYPOSITION);
	mnuPopup.AppendMenu(nHaveResEnableMenu|MF_STRING|MF_BYPOSITION, miCut, "Cut...");
	mnuPopup.AppendMenu(nHaveResEnableMenu|MF_STRING|MF_BYPOSITION, miCopy, "Copy...");
	mnuPopup.AppendMenu(nEnablePaste|MF_STRING|MF_BYPOSITION, miPaste, "Paste...");
	// (z.manning 2014-12-17 17:17) - PLID 64225 - Copy day is not relevant when editing collections
	if (m_eEditorType != stetCollection)
	{
		mnuPopup.AppendMenu(MF_SEPARATOR | MF_BYPOSITION);// (a.vengrofski 2009-12-30 12:28) - PLID <28394> - Give them some room.
		mnuPopup.AppendMenu(nEnableCopyAll | MF_STRING | MF_BYPOSITION, miCopyAll, "Copy Day");// (a.vengrofski 2009-12-30 12:28) - PLID <28394> - Only show if there is at least 1 template
		mnuPopup.AppendMenu(nEnablePasteAll | MF_STRING | MF_BYPOSITION, miPasteAll, "Paste Day");// (a.vengrofski 2009-12-30 12:28) - PLID <28394> - Only Show if there is something to be pasted
	}
	mnuPopup.AppendMenu(MF_SEPARATOR|MF_BYPOSITION);
	mnuPopup.AppendMenu(nHaveResEnableMenu|MF_STRING|MF_BYPOSITION, miDelete, "Delete...");
	mnuPopup.SetDefaultItem(miEdit);

	CPoint pt;
	GetCursorPos(&pt);

	// (z.manning, 11/28/2006) - Make sure this is done before TrackPopupMenu because it's dependent
	// on the current mouse position.
	COleDateTime dtCurrentSlotTime = GetCurrentSlotTime();

	// (c.haag 2010-06-29 09:23) - PLID 39393 - We now pass in the dispatch instead of the CReservation.
	// Now every called function will deal with its usage in its own way.
	int nResult = mnuPopup.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
	switch(nResult)
	{
		case miEdit:
			OnReservationClickTemplateItemEntryCtrl(theRes);
			break;

		case miDelete:
			DeleteLineItem(theRes);
			break;

		case miCut:
			// (z.manning 2008-06-12 11:50) - PLID 29273 - Do not allow any cutting
			// without this permission as it will delete a template item when pasted.
			if(!CheckCurrentUserPermissions(m_bio,sptDelete)) {
				return;
			}
			SetClippedReservation(theRes, crtCut);
			break;

		case miCopy:
			SetClippedReservation(theRes, crtCopy);
			break;
			
		case miPaste:
			PasteLineItem(dtCurrentSlotTime);
			break;

		case miCopyAll:// (a.vengrofski 2009-12-30 12:28) - PLID <28394> - Copy the day
			CopyEntireDay();
			break;

		case miPasteAll:// (a.vengrofski 2009-12-30 12:28) - PLID <28394> - Paste the day
			PasteEntireDay();
			break;

		default:
			break;
	}
}

void CTemplateItemEntryGraphicalDlg::OnReservationAddedTemplateItemEntryCtrl(LPDISPATCH theRes) 
{
	CTemplateLineItemInfo* pLineItem = NULL;
	CReservation pRes(__FUNCTION__, theRes);
	try {

		if(pRes)
		{
			// (z.manning, 12/11/2006) - PLID 23768 - Make sure we have create permission.
			if(CheckCurrentUserPermissions(m_bio, sptCreate))
			{
				// (z.manning, 11/14/2006) - PLID 23443 - Populate the line item info with defaults based on the
				// current state and selected time range.
				pLineItem = new CTemplateLineItemInfo;
				//TES 6/19/2010 - PLID 5888 - Tell it whether to use Resource Availability templates
				pLineItem->m_bUseResourceAvailTemplates = m_bUseResourceAvailTemplates;
				COleDateTime dtStartDate = GetActiveDate();
				// (c.haag 2006-11-29 10:55) - PLID 23443 - We should only have a date value
				dtStartDate.SetDate(dtStartDate.GetYear(), dtStartDate.GetMonth(), dtStartDate.GetDay());
				pLineItem->m_esScale = sWeekly;
				pLineItem->m_dtStartTime = pRes.GetStartTime();
				pLineItem->m_dtEndTime = pRes.GetEndTime();
				pLineItem->m_dtStartDate = dtStartDate;
				pLineItem->m_dtEndDate = g_cdtInvalid;
				pLineItem->m_nInclude = DAYVAL(dtStartDate.GetDayOfWeek() - 1);
				pLineItem->m_nDayNumber = GetActiveDate().GetDay();

				//TES 6/19/2010 - PLID 5888 - Resource Availability templates don't have precision templates
				if(!m_bUseResourceAvailTemplates) {
					//TES 12/19/2008 - PLID 32537 - Scheduler Standard users aren't allowed to create precision templates.
					if(g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
						// (z.manning, 12/07/2006) - PLID 23809 - Check preference to see if we should default it to being a block.
						if(GetRemotePropertyInt("DefaultTemplateLineItemsAsblocks", 0, 0, GetCurrentUserName(), true) == 1) {
							pLineItem->m_bIsBlock = TRUE;
						}
						else {
							pLineItem->m_bIsBlock = FALSE;
						}
					}
					else {
						pLineItem->m_bIsBlock = FALSE;
					}
				}

				// (z.manning, 12/07/2006) - PLID 23809 - Check preference for default resource.
				//TES 6/21/2010 - PLID 5888 - Split into separate properties.
				CString strProperty = m_bUseResourceAvailTemplates?"DefaultResourceAvailTemplateLineItemsAllResources":"DefaultTemplateLineItemsAllResources";
				if(GetRemotePropertyInt(strProperty, 0, 0, GetCurrentUserName(), true) == 1) {
					pLineItem->m_bAllResources = TRUE;
				}
				else {
					pLineItem->m_bAllResources = FALSE;
					// (z.manning 2011-12-07 11:40) - PLID 46910 - Pass in the resource name too
					pLineItem->AddResource(GetActiveResourceID(), GetActiveResourceName());
				}

				if (m_eEditorType == stetCollection)
				{
					_SchedulerTemplateCollectionPtr pCollection = m_pdlgTemplateList->GetSelectedCollection();
					if (pCollection != NULL)
					{
						// (z.manning 2014-12-05 15:59) - PLID 64219 - Special handling for collections
						CTemplateSelectDlg dlgTemplateSelect(m_eEditorType, this);
						dlgTemplateSelect.m_dtStartTime = pLineItem->m_dtStartTime;
						dlgTemplateSelect.m_dtEndTime = pLineItem->m_dtEndTime;
						if (dlgTemplateSelect.DoModal() == IDOK)
						{
							pRes.ReleaseAndClear();

							// (z.manning 2014-12-09 09:50) - PLID 64219 - Add the collection template
							CommitCollectionTemplate(g_cvarNull, &dlgTemplateSelect);
						}
						else {
							pRes.DeleteRes(__FUNCTION__);
						}
					}
					else {
						pRes.DeleteRes(__FUNCTION__);
					}
				}
				else
				{
					// (z.manning, 11/14/2006) - Now open the line item editor.
					// (z.manning 2014-12-11 09:28) - PLID 64230 - Pass in the editor type
					CTemplateLineItemDlg dlgLineItem(m_eEditorType, this);
					//TES 6/19/2010 - PLID 5888 - Pass in m_bUseResourceAvailTemplates
					if (dlgLineItem.ZoomLineItem(pLineItem) == IDOK) {
						// Ok, they want to create this line item, so they must choose or create a template for it.
						// (z.manning 2014-12-05 14:07) - PLID 64219 - Pass in the type
						CTemplateSelectDlg dlgTemplateSelect(m_eEditorType, this);
						if (dlgTemplateSelect.DoModal() == IDOK) {
							// (c.haag 2010-06-29 09:23) - PLID 39393 - Discard the reservation before we refresh everything, or else it will become
							// a dangling reservation.
							pRes.ReleaseAndClear();
							// All right, we have a template, so let's save the new line item.
							pLineItem->m_nTemplateID = dlgTemplateSelect.m_nSelectedTemplateID;
							ExecuteSql(pLineItem->GenerateSaveString());
							// (c.haag 2006-12-12 10:58) - PLID 23808 - Notify other computers about the change.
							//TES 6/21/2010 - PLID 5888 - Send the appropriate tablechecker
							CClient::RefreshTable(m_bUseResourceAvailTemplates ? NetUtils::ResourceAvailTemplateItemT : NetUtils::TemplateItemT, -1);
							RefreshEverything();
							m_pdlgTemplateList->RequeryTemplateList();
						}
						else {
							pRes.DeleteRes(__FUNCTION__);
						}
					}
					else {
						pRes.DeleteRes(__FUNCTION__);
					}
				}
			}
			else {
				// (z.manning, 12/11/2006) - PLID 23768 - No permission, delete the reservation.
				pRes.DeleteRes(__FUNCTION__);
			}
		}

	}NxCatchAllCall("CTemplateItemEntryGraphicalDlg::OnReservationAddedTemplateItemEntryCtrl", if(pRes) { pRes.DeleteRes(__FUNCTION__); });

	if(pLineItem) {
		delete pLineItem;
	}
	PutFocusOnSingleDay();
}

// (z.manning 2014-12-17 13:15) - PLID 64427
void CTemplateItemEntryGraphicalDlg::CommitCollectionTemplate(const _variant_t &varCollectionTemplateID, CTemplateSelectDlg *pdlgTemplateSelect)
{
	CommitCollectionTemplate(varCollectionTemplateID, pdlgTemplateSelect->m_nSelectedTemplateID
		, pdlgTemplateSelect->m_dtStartTime, pdlgTemplateSelect->m_dtEndTime, pdlgTemplateSelect->m_bIsBlock);
}

// (z.manning 2014-12-17 13:41) - PLID 64427
void CTemplateItemEntryGraphicalDlg::CommitCollectionTemplate(const _variant_t &varCollectionTemplateID, const long nTemplateID
	, const COleDateTime &dtStartTime, const COleDateTime &dtEndTime, BOOL bIsBlock)
{
	_SchedulerTemplateCollectionPtr pCollection = m_pdlgTemplateList->GetSelectedCollection();
	if (pCollection == NULL) {
		ASSERT(FALSE);
		AfxThrowNxException("Tried to commit collection template without a selected collection");
	}

	_SchedulerTemplateCollectionTemplateCommitPtr pCommit(__uuidof(SchedulerTemplateCollectionTemplateCommit));
	pCommit->CollectionID = pCollection->ID;
	pCommit->TemplateID = _bstr_t(nTemplateID);
	pCommit->StartTime = dtStartTime;
	pCommit->EndTime = dtEndTime;
	pCommit->IsBlock = bIsBlock ? VARIANT_TRUE : VARIANT_FALSE;

	if (varCollectionTemplateID.vt == VT_NULL) {
		GetAPI()->CreateSchedulerTemplateCollectionTemplate(GetAPISubkey(), GetAPILoginToken(), pCommit);
	}
	else {
		pCommit->ID = _bstr_t(varCollectionTemplateID);
		GetAPI()->EditSchedulerTemplateCollectionTemplate(GetAPISubkey(), GetAPILoginToken(), pCommit);
	}

	RefreshEverything();
}

void CTemplateItemEntryGraphicalDlg::SetDefaultResourceID(long nResourceID)
{
	m_nDefaultResourceID = nResourceID;
}

// (j.jones 2011-07-15 14:38) - PLID 39838 - set a default date
void CTemplateItemEntryGraphicalDlg::SetDefaultDate(const COleDateTime dt)
{
	m_dtDefaultDate = dt;
}

// (c.haag 2014-12-15) - PLID 64246 - Set a default collection ID
void CTemplateItemEntryGraphicalDlg::SetDefaultCollectionID(_bstr_t bstrCollectionID)
{
	m_bstrDefaultCollectionID = bstrCollectionID;
}

void CTemplateItemEntryGraphicalDlg::PutFocusOnSingleDay()
{
	try {
		if(m_pSingleDayCtrl) {
			CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd())->SetFocus();
		}
	}NxCatchAllIgnore();
}

void CTemplateItemEntryGraphicalDlg::OnSelChosenResourceList(LPDISPATCH lpRow) 
{
	CWaitCursor wc;
	try {

		UpdateView();

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnSelChosenResourceList");
	PutFocusOnSingleDay();	
}

LRESULT CTemplateItemEntryGraphicalDlg::OnUpdateView(WPARAM wParam, LPARAM lParam)
{
	try {

		UpdateView();

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnUpdateView");

	return S_OK;
}

void CTemplateItemEntryGraphicalDlg::OnReservationEndDragTemplateItemEntryCtrl(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY) 
{
	// (z.manning, 11/15/2006) - PLID 23555 - Prompt them if they want to update the times for this line item always
	// or just this once and then update.
	CTemplateLineItemInfo* pLineItem = NULL;
	try {

		CReservation pResPostDrag = CReservation(__FUNCTION__, theRes);
		if(pResPostDrag != NULL) {

			long nLineItemID = pResPostDrag.GetReservationID();
			COleDateTime dtNewStart = pResPostDrag.GetStartTime();
			COleDateTime dtNewEnd = pResPostDrag.GetEndTime();
			// (c.haag 2010-06-29 13:55) - PLID 39393 - Discard the reservation; we never use it from here.
			pResPostDrag.ReleaseAndClear();

			// If the time didn't change, we don't need to to any of this.
			if (!((CompareTimes(dtNewStart, m_dtResPreMoveStartTime) & CT_EQUAL)
				&& (CompareTimes(dtNewEnd, m_dtResPreMoveEndTime) & CT_EQUAL)))
			{
				// (z.manning, 12/11/2006) - PLID 23768 - Do we have permission to change line items?
				if (!CheckCurrentUserPermissions(m_bio, sptWrite)) {
					UpdateView();
					return;
				}

				if (m_eEditorType == stetCollection)
				{
					// (z.manning 2014-12-17 15:07) - PLID 64427 - Handle template collections
					_SchedulerTemplateCollectionTemplatePtr pCollectionTemplate = m_pdlgTemplateList->GetCollectionTemplateByID(nLineItemID);
					if (pCollectionTemplate != NULL)
					{
						CommitCollectionTemplate(pCollectionTemplate->ID, AsLong(pCollectionTemplate->Template->ID), dtNewStart
							, dtNewEnd, pCollectionTemplate->IsBlock ? TRUE : FALSE);
					}
				}
				else
				{
					pLineItem = new CTemplateLineItemInfo;
					//TES 6/19/2010 - PLID 5888 - Pass in m_bUseResourceAvailTemplates
					pLineItem->m_bUseResourceAvailTemplates = m_bUseResourceAvailTemplates;
					pLineItem->LoadFromID(nLineItemID);

					CString strSql = BeginSqlBatch();

					CTemplateItemChangeTimePromptDlg dlgPrompt(pLineItem, m_eEditorType, this);

					// If this line item only actually spans one day, no need to prompt.
					int nResult;
					if (pLineItem->m_dtStartDate == pLineItem->m_dtEndDate) {
						nResult = ID_CHANGE_LINE_ITEM;
					}
					else {
						nResult = dlgPrompt.DoModal();
					}
					EnsureValidEndTime(dtNewEnd);
					switch (nResult)
					{
					case ID_CHANGE_LINE_ITEM:
						// They want to change all instances of this line item, so simply update it in data.
						pLineItem->m_dtStartTime = dtNewStart;
						pLineItem->m_dtEndTime = dtNewEnd;

					// (z.manning 2015-01-05 17:17) - PLID 64521 - Since we changed this line item make sure it's no longer tied
					// to a collection apply.
					pLineItem->m_nCollectionID = -1;

						pLineItem->AddSaveStringToBatch(strSql);
						break;

					case ID_CHANGE_ONCE:
						// Only want to change this one specific instance. The way we handle this is making
						// an exception for the line on the active date, and then inserting a new line item
						// with the same info with that starts and ends on the new times.
						// Make an exception on the active date.
						AddStatementToSqlBatch(strSql, pLineItem->GenerateExceptionString(GetActiveDate()));
						// Now create a new line item for just the active date at the new times.
						pLineItem->SetNew();
						pLineItem->m_dtStartTime = dtNewStart;
						pLineItem->m_dtEndTime = dtNewEnd;
						pLineItem->m_dtStartDate = GetActiveDate();
						pLineItem->m_dtEndDate = GetActiveDate();
						pLineItem->m_esScale = sOnce; // This doesn't affect functionality at all, but makes the description clearer.
						pLineItem->m_arydtExceptionDates.RemoveAll();
						pLineItem->AddSaveStringToBatch(strSql);
						break;
					}

					if (nResult != IDCANCEL) {
						ExecuteSqlBatch(strSql);
						// (c.haag 2006-12-12 10:58) - PLID 23808 - Notify other computers about the change
						//TES 6/21/2010 - PLID 5888 - Send the appropriate tablechecker
						if (pLineItem->IsNew()) {
							CClient::RefreshTable(m_bUseResourceAvailTemplates ? NetUtils::ResourceAvailTemplateItemT : NetUtils::TemplateItemT, -1);
						}
						else {
							CClient::RefreshTable(m_bUseResourceAvailTemplates ? NetUtils::ResourceAvailTemplateItemT : NetUtils::TemplateItemT, pLineItem->m_nID);
						}
						RefreshEverything();
					}
					else {
						// (z.manning, 12/11/2006) - PLID 23818 - Even if they cancelled, make sure to 
						// update the view to make sure the reservation goes back to its original location.
						UpdateView();
					}
				}
			}
		}
		else {
			ASSERT(FALSE);
		}

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnReservationEndDragTemplateItemEntryCtrl");

	if(pLineItem) {
		delete pLineItem;
	}
	m_bResMoving = FALSE;
}

void CTemplateItemEntryGraphicalDlg::OnReservationEndResizeTemplateItemEntryCtrl(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY) 
{
	try {

		// (z.manning, 11/15/2006) - PLID 23555 - Functionality is the exact same as dragging.
		OnReservationEndDragTemplateItemEntryCtrl(theRes, x, y, OrgX, OrgY);

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnReservationEndResizeTemplateItemEntryCtrl");
}


void CTemplateItemEntryGraphicalDlg::OnReservationDragTemplateItemEntryCtrl(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY) 
{
	try {

		// (z.manning, 11/16/2006) - PLID 23555 - As soon as a drag or resize begins, store the current
		// reservation times so we can tell if something changed after the drag/resize is done.
		if(!m_bResMoving) {
			CReservation pRes(__FUNCTION__, theRes);
			m_dtResPreMoveStartTime = pRes.GetStartTime();
			m_dtResPreMoveEndTime = pRes.GetEndTime();
			m_bResMoving = TRUE;
		}

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnReservationDragTemplateItemEntryCtrl");
}

void CTemplateItemEntryGraphicalDlg::OnReservationResizeTemplateItemEntryCtrl(LPDISPATCH theRes, long x, long y, long OrgX, long OrgY) 
{
	try {
		
		// (z.manning, 11/15/2006) - PLID 23555 - Functionality is the exact same as dragging.
		OnReservationDragTemplateItemEntryCtrl(theRes, x, y, OrgX, OrgY);

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnReservationResizeTemplateItemEntryCtrl");
}

// (c.haag 2010-06-29 09:23) - PLID 39393 - We now take in a dispatch instead of a CReservation
void CTemplateItemEntryGraphicalDlg::DeleteLineItem(LPDISPATCH theRes)
{
	// (z.manning, 11/16/2006) - PLID 23574 - Ask them if they want to delete the entire line item
	// or just the active date's instance of it.
	try {

		// (z.manning, 12/11/2006) - PLID 23768 - Make sure we have delete permissions.
		if(!CheckCurrentUserPermissions(m_bio,sptDelete)) {
			return;
		}

		// (c.haag 2014-12-12) - PLID 64221- Act based on our editor type
		switch (m_eEditorType)
		{
		case stetCollection:
			DeleteCollectionTemplate(theRes);
			break;

		default:
			DeleteTemplateLineItem(theRes);
			break;
		}

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::DeleteReservation");
}

// (c.haag 2014-12-12) - PLID 64221 - Collections editor version of DeleteLineItem
void CTemplateItemEntryGraphicalDlg::DeleteCollectionTemplate(LPDISPATCH theRes)
{
	_SchedulerTemplateCollectionPtr pCollection = m_pdlgTemplateList->GetSelectedCollection();
	if (pCollection == NULL)
	{
		// We should never get here
	}
	else
	{
		CReservation pRes(__FUNCTION__, theRes);
		long nResID = pRes.GetReservationID();
		pRes.ReleaseAndClear();

		CString strMessage;
		strMessage.Format(R"(Are you sure you wish to delete this template item from the %s collection?

This will not affect any previous applies.)"
	, (LPCTSTR)pCollection->Name);

		if (IDYES == MessageBox(strMessage, NULL, MB_ICONQUESTION | MB_YESNO))
		{
			GetAPI()->DeleteSchedulerTemplateCollectionTemplate(GetAPISubkey(), GetAPILoginToken(), _bstr_t(nResID));
			// This does not affect previous applies so don't send a table checker
			RefreshEverything();
		}
	}
}

// (c.haag 2014-12-12) - PLID 64221 - Non-Collections editor version of DeleteLineItem. Moved from the original DeleteLineItem
void CTemplateItemEntryGraphicalDlg::DeleteTemplateLineItem(LPDISPATCH theRes)
{
	// (c.haag 2010-06-29 09:23) - PLID 39393 - We only need the reservation ID; so grab it and release our
	// reference to the reservation.
	CReservation pRes(__FUNCTION__, theRes);
	long nResID = pRes.GetReservationID();
	pRes.ReleaseAndClear();

	// (c.haag 2006-12-11 11:49) - PLID 23808 - Gracefully fail if the line item no longer exists
	if (!ReturnsRecordsParam(FormatString("SELECT TOP 1 ID FROM %sTemplateItemT WHERE ID = {INT}", m_bUseResourceAvailTemplates ? "ResourceAvail" : ""), nResID)) {
		MessageBox("This line item has been deleted by another user.");
		UpdateView();
		return;
	}

	CTemplateLineItemInfo* pLineItem = new CTemplateLineItemInfo;
	try
	{
		pLineItem->m_bUseResourceAvailTemplates = m_bUseResourceAvailTemplates;
		pLineItem->LoadFromID(nResID);

		CString strSql = BeginSqlBatch();

		CTemplateItemChangeTimePromptDlg dlgPrompt(pLineItem, m_eEditorType, this);
		dlgPrompt.SetChangeType("Delete");

		// If this line item only actually spans one day, then changing the line item is no different
		// than changing it once.
		int nResult;
		if (pLineItem->m_dtStartDate == pLineItem->m_dtEndDate) {
			if (IDYES == MessageBox("Are you sure you want to delete this template item?", NULL, MB_YESNO)) {
				nResult = ID_CHANGE_LINE_ITEM;
			}
			else {
				nResult = IDCANCEL;
			}
		}
		else {
			nResult = dlgPrompt.DoModal();
		}
		switch (nResult)
		{
		case ID_CHANGE_LINE_ITEM:
			// They want to delete the whole thing, so just delete it from data.
			AddStatementToSqlBatch(strSql, pLineItem->GenerateDeleteString());
			break;

		case ID_CHANGE_ONCE:
			// They only want to delete the active date's instance, so make an exception for the line item
			// on the active date.
			AddStatementToSqlBatch(strSql, pLineItem->GenerateExceptionString(GetActiveDate()));
			break;
		}

		if (nResult != IDCANCEL) {
			ExecuteSqlBatch(strSql);
			// (c.haag 2006-12-12 10:58) - PLID 23808 - Notify other computers about the change. Pass in
			// the ID even though it is gone from data; handlers should be able to deal with cases where
			// the passed in ID does not exist in data.
			//TES 6/21/2010 - PLID 5888 - Send the appropriate tablechecker
			CClient::RefreshTable(m_bUseResourceAvailTemplates ? NetUtils::ResourceAvailTemplateItemT : NetUtils::TemplateItemT, pLineItem->m_nID);
			RefreshEverything();
		}
		delete pLineItem;
		pLineItem = NULL;
	}
	catch (...)
	{
		if (pLineItem) {
			delete pLineItem;
		}
		throw;
	}
}

void CTemplateItemEntryGraphicalDlg::OnOptionsBtn() 
{
	try {

		CTemplateInterfaceOptionsDlg dlgOptions(this);

		//TES 6/19/2010 - PLID 5888 - Pass in m_bUseResourceAvailTemplates
		dlgOptions.m_bUseResourceAvailTemplates = m_bUseResourceAvailTemplates;

		// (z.manning, 11/21/2006) - PLID 23809 - The only options thus far only affect the creation
		// of new line items, so we don't need to update the view or anything.
		dlgOptions.DoModal();

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnOptionsBtn");
	PutFocusOnSingleDay();
}

// (c.haag 2014-12-15) - PLID 64246 - Lets the user create a template collection from the current view
void CTemplateItemEntryGraphicalDlg::OnCreateCollectionFromViewBtn()
{
	try
	{
		if (m_eEditorType == stetCollection)
		{
			// Should never get here
			ASSERT(FALSE);
			return;
		}

		// Check permissions
		if (!CheckCurrentUserPermissions(m_bio, sptCreate)) {
			return;
		}
		
		// Make sure there's at least one item in the single day. There should never be a case where the single day has nothing in it
		// but there are actually templates in data (unless maybe someone on another computer changed them and we don't know it yet)
		if (0 == m_pSingleDayCtrl.GetReservationCount(0))
		{
			MessageBox("No templates were found in the current view. The collection was not created.", NULL, MB_OK | MB_ICONERROR);
			return;
		}

		// Let the user know what's going on
		DontShowMeAgain(this, 
			R"(You have chosen to create a template collection from the currently selected resource and date. Doing this will create a collection of your scheduler templates, keep their time slots intact, and whether the template is a precision template.

You will be prompted to select a date range, period, and resource(s) upon applying this collection to your schedule.)"
			, "CTemplateItemEntryGraphicalDlg_OnCreateCollectionFromViewBtn");

		// Have the user choose a name and color for the collection
		CNameAndColorEntryDlg dlgEntry(this);
		dlgEntry.m_strWindowTitle = "New Template Collection";
		dlgEntry.m_nTextLimit = 50;
		dlgEntry.m_strSqlTable = "TemplateCollectionT";
		dlgEntry.m_strSqlColumn = "Name";

		if (dlgEntry.DoModal() == IDOK)
		{
			// When creating a template collection from the current view, create a collection of all of the scheduler 
			// templates that are visible on that day. The templates in the collection need to maintain the time range
			// selected on the editor, and whether or not the template is a precision template. Do not carry over the 
			// date range, period, or resources into the template collection as those items will be set when adding the 
			// collection to the schedule. When creating a collection from a template day, it will copy over all templates,
			// not just visible templates.
			CArray<long, long> arynResourceIDs;
			CTemplateHitSet rsTemplateHits;
			long nWorkingResourceID = GetActiveResourceID();

			// Query for all the hits
			if (m_bLoadAllResources) {
				GetAllLoadedResourceIDs(arynResourceIDs);
				rsTemplateHits.Requery(GetActiveDate(), arynResourceIDs, FALSE, -1, m_bUseResourceAvailTemplates ? false : true, m_bUseResourceAvailTemplates);
			}
			else {
				arynResourceIDs.Add(nWorkingResourceID);
				rsTemplateHits.Requery(GetActiveDate(), nWorkingResourceID, FALSE, -1, m_bUseResourceAvailTemplates ? false : true, m_bUseResourceAvailTemplates);
			}

			// Now iterate through all the hits
			Nx::SafeArray<IUnknown *> saryTemplateCommits; // Collection of _SchedulerTemplateCollectionTemplateCommitPtr			
			if (!rsTemplateHits.IsBOF())
			{
				for (int nResourceIndex = 0; nResourceIndex < arynResourceIDs.GetSize(); nResourceIndex++)
				{
					rsTemplateHits.MoveFirst();
					long nCurrentResourceID = arynResourceIDs.GetAt(nResourceIndex);
					CArray<TemplateItemReservationInfo, TemplateItemReservationInfo> aryBlocks;
					while (!rsTemplateHits.IsEOF())
					{
						if (rsTemplateHits.GetResourceID() != nCurrentResourceID && !rsTemplateHits.GetIsAllResources()) {
							rsTemplateHits.MoveNext();
							continue;
						}

						_SchedulerTemplateCollectionTemplateCommitPtr p(__uuidof(SchedulerTemplateCollectionTemplateCommit));
						p->TemplateID = _bstr_t(rsTemplateHits.GetTemplateID());
						p->StartTime = rsTemplateHits.GetStartTime();
						p->EndTime = rsTemplateHits.GetEndTime();
						p->IsBlock = rsTemplateHits.GetIsBlock();
						saryTemplateCommits.Add(p);

						rsTemplateHits.MoveNext();
					}
				}
			}

			if (0 == saryTemplateCommits.GetSize())
			{
				MessageBox("No templates were found in the current view. The collection was not created.", NULL, MB_OK | MB_ICONERROR);
				return;
			}
			else
			{
				// Now put everything together and save to data
				_SchedulerTemplateCollectionCommitPtr pCommit(__uuidof(SchedulerTemplateCollectionCommit));
				pCommit->Name = _bstr_t(dlgEntry.m_strName);
				pCommit->Color = _bstr_t(ConvertCOLORREFToHexString(dlgEntry.m_nColor));
				pCommit->TemplateCommits = saryTemplateCommits;

				_SchedulerTemplateCollectionPtr pNewCollection = GetAPI()->CreateSchedulerTemplateCollection(GetAPISubkey(), GetAPILoginToken(), pCommit);

				if (IDYES == MessageBox(R"(Your template collection was successfully created!

Would you like to open the Template Collection Manager now?)", NULL, MB_YESNO | MB_ICONQUESTION))
				{
					// Open the template collection manager on top of the scheduler template editor with the new collection selected
					CTemplateItemEntryGraphicalDlg dlgTemplateEditor(stetCollection, this);
					dlgTemplateEditor.SetDefaultResourceID(GetActiveResourceID());					
					dlgTemplateEditor.SetDefaultDate(GetActiveDate());
					dlgTemplateEditor.SetDefaultCollectionID(pNewCollection->ID);
					dlgTemplateEditor.DoModal();
				}
				else
				{
					// If No, return to the Schedule Template Editor
				}
			}
		}
	}
	NxCatchAll(__FUNCTION__);
	PutFocusOnSingleDay();
}

// (c.haag 2015-02-06) - PLID 64353 - Calculates which resources are affected by a call to CreateTemplateExceptionsForResourceTimeRange
_RecordsetPtr CTemplateItemEntryGraphicalDlg::GetAffectedResourcesForTemplateExceptionCreation(const COleDateTime& dtFromTime, const COleDateTime& dtToTime)
{
	return CreateParamRecordset(R"(
SET NOCOUNT ON
DECLARE @affectedTemplateItems TABLE (ID INT NOT NULL)
INSERT INTO @affectedTemplateItems
	SELECT DISTINCT
		TemplateItemT.ID
	FROM
		TemplateItemT
	LEFT JOIN TemplateItemResourceT
		ON TemplateItemResourceT.TemplateItemID = TemplateItemT.ID
		AND TemplateItemResourceT.ResourceID = {INT}
	LEFT JOIN TemplateItemExceptionT
		ON TemplateItemExceptionT.TemplateItemID = TemplateItemT.ID
		AND TemplateItemExceptionT.Date = {OLEDATETIME}
	WHERE
		-- Date filter
		TemplateItemT.StartDate <= dbo.AsDateNoTime({OLEDATETIME})
		AND (TemplateItemT.EndDate IS NULL OR TemplateItemT.EndDate >= dbo.AsDateNoTime({OLEDATETIME}))
		-- Time filter
		AND TemplateItemT.StartTime >= dbo.AsTimeNoDate({OLEDATETIME})
		AND TemplateItemT.StartTime < dbo.AsTimeNoDate({OLEDATETIME})
		-- Resource filter
		AND (TemplateItemResourceT.ResourceID IS NOT NULL OR TemplateItemT.AllResources = CONVERT(BIT,1))
		-- Duplicates filter
		AND TemplateItemExceptionT.ID IS NULL
	ORDER BY
		TemplateItemT.ID

SET NOCOUNT OFF

-- Select true if at least one affected line item is for all resources, or false if not
SELECT
	TOP 1 1
FROM 
	@affectedTemplateItems A
	INNER JOIN TemplateItemT ON TemplateItemT.ID = A.ID
	WHERE TemplateItemT.AllResources <> CONVERT(BIT,0)

-- If false, select the individual resource names
IF (@@ROWCOUNT = 0)
BEGIN
	SELECT DISTINCT
		ResourceT.ID, ResourceT.Item
	FROM
		@affectedTemplateItems A
	INNER JOIN TemplateItemResourceT
		ON TemplateItemResourceT.TemplateItemID = A.ID
	INNER JOIN ResourceT
		ON ResourceT.ID = TemplateItemResourceT.ResourceID
	ORDER BY
		ResourceT.Item
END

)"
, GetActiveResourceID()
, GetActiveDate()
, GetActiveDate()
, GetActiveDate()
, dtFromTime
, dtToTime
);
}

// (c.haag 2014-12-17) - PLID 64253 - Gets the warning message for removing templates in a given time range
CString CTemplateItemEntryGraphicalDlg::GetRemoveTemplatesWarningTextByTimeRange(const COleDateTime& dtFromTime, const COleDateTime& dtToTime, OUT BOOL& bQuestion)
{
	_RecordsetPtr prsAffectedResources = GetAffectedResourcesForTemplateExceptionCreation(dtFromTime, dtToTime);
	if (!prsAffectedResources->eof)
	{
		bQuestion = TRUE;
		return FormatString(R"(You have selected to clear all of the templates starting in between %s and %s on %s containing the %s resource.

This change will affect ALL RESOURCES in the scheduler.

Are you sure you wish to do this?)"
, FormatDateTimeForInterface(dtFromTime, DTF_STRIP_SECONDS, dtoTime)
, FormatDateTimeForInterface(dtToTime, DTF_STRIP_SECONDS, dtoTime)
, FormatDateTimeForInterface(GetActiveDate(), NULL, dtoDate)
, GetActiveResourceName()
);
	}
	else
	{
		CString strResources;
		prsAffectedResources = prsAffectedResources->NextRecordset(NULL);
		int nResources = 0;
		const int nActiveResourceID = GetActiveResourceID();
		BOOL bWillModifyData = FALSE;
		while (!prsAffectedResources->eof)
		{
			bWillModifyData = TRUE;
			if (nActiveResourceID != AdoFldLong(prsAffectedResources, "ID"))
			{
				strResources += AdoFldString(prsAffectedResources, "Item") + "\r\n";
				prsAffectedResources->MoveNext();
				if (++nResources == 10 && !prsAffectedResources->eof)
				{
					strResources += "<more>\r\n";
					break;
				}
			}
			else
			{
				prsAffectedResources->MoveNext();
			}
		}

		if (!bWillModifyData)
		{
			// This means no templates will be affected
			bQuestion = FALSE;
			return "There are no templates in the given time range that would be affected.";
		}
		else if (0 == nResources)
		{
			// This means only the current resource will be affected
			bQuestion = TRUE;
			return FormatString(R"(You have selected to clear all of the templates starting in between %s and %s on %s containing the %s resource. This will not affect any other resources.

Are you sure you wish to do this?)"
, FormatDateTimeForInterface(dtFromTime, DTF_STRIP_SECONDS, dtoTime)
, FormatDateTimeForInterface(dtToTime, DTF_STRIP_SECONDS, dtoTime)
, FormatDateTimeForInterface(GetActiveDate(), NULL, dtoDate)
, GetActiveResourceName());
		}
		else
		{
			bQuestion = TRUE;
			return FormatString(R"(You have selected to clear all of the templates starting in between %s and %s on %s containing the %s resource. The following resources will also be affected:

%s
Are you sure you wish to do this?)"
				, FormatDateTimeForInterface(dtFromTime, DTF_STRIP_SECONDS, dtoTime)
				, FormatDateTimeForInterface(dtToTime, DTF_STRIP_SECONDS, dtoTime)
				, FormatDateTimeForInterface(GetActiveDate(), NULL, dtoDate)
				, GetActiveResourceName()
				, strResources);
		}
	}
}

// (c.haag 2015-02-06) - PLID 64353 - Gets the warning message for removing templates in the day
CString CTemplateItemEntryGraphicalDlg::GetRemoveTemplatesWarningText()
{
	COleDateTime dtStartTime, dtEndTime;
	dtStartTime.SetTime(0, 0, 0);
	dtEndTime.SetTime(23, 59, 59);
	_RecordsetPtr prsAffectedResources = GetAffectedResourcesForTemplateExceptionCreation(dtStartTime, dtEndTime);
	if (!prsAffectedResources->eof)
	{
		return R"(You have selected to clear the existing templates and replace them with templates from the collection, but there are existing templates tied to other resources than the resource selected. Clearing the existing templates will also clear the templates from the other resources.

ALL RESOURCES will be affected by this change!

Are you sure you wish to do this?)";
	}
	else
	{
		CString strResources;
		prsAffectedResources = prsAffectedResources->NextRecordset(NULL);
		int nResources = 0;
		const int nActiveResourceID = GetActiveResourceID();
		BOOL bWillModifyData = FALSE;
		while (!prsAffectedResources->eof)
		{
			bWillModifyData = TRUE;
			if (nActiveResourceID != AdoFldLong(prsAffectedResources, "ID"))
			{
				strResources += AdoFldString(prsAffectedResources, "Item") + "\r\n";
				prsAffectedResources->MoveNext();
				if (++nResources == 10 && !prsAffectedResources->eof)
				{
					strResources += "<more>\r\n";
					break;
				}
			}
			else
			{
				prsAffectedResources->MoveNext();
			}
		}

		if (!bWillModifyData)
		{
			// This means no templates will be affected
			return "";
		}
		else if (0 == nResources)
		{
			// This means only the current resource will be affected
			return R"(You have selected to clear the existing templates and replace them with templates from collections. This will not affect any other scheduler resources.

Are you sure you wish to do this?)";
		}
		else
		{
			return FormatString(R"(You have selected to clear the existing templates and replace them with templates from the collection, but there are existing templates tied to other resources than the resource selected. Clearing the existing templates will also clear the templates used by the following resources:

%s
Are you sure you wish to do this?)"
, strResources);
		}
	}
}

// (c.haag 2014-12-17) - PLID 64253 - "Removes" templates over a time range by adding exceptions to them for
// each day in the time range
void CTemplateItemEntryGraphicalDlg::OnRemoveTemplatesBtn()
{
	try
	{
		CChooseDateRangeDlg dlgChooseDates(this);
		dlgChooseDates.SetChooseTimeRange(TRUE);
		dlgChooseDates.SetCancelButtonVisible(TRUE);
		if (IDOK == dlgChooseDates.DoModal())
		{
			// Whether or not data will actually change, we always present the user with a prompt. It will either be a statement that
			// no data will be modified, or a confirmation prompt to modify the data.
			BOOL bQuestion = FALSE;
			CString strWarning = GetRemoveTemplatesWarningTextByTimeRange(dlgChooseDates.GetFromDate(), dlgChooseDates.GetToDate(), bQuestion);
			if (!bQuestion)
			{
				// Must be a statement that no data will be modified
				MessageBox(strWarning, NULL, MB_OK | MB_ICONINFORMATION);
				return;
			}
			else
			{
				// Data will surely be modified
				if (IDYES == MessageBox(strWarning, NULL, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
				{
					_CreateSchedulerTemplateExceptionsForResourceTimeRangeRequestPtr pRequest(__uuidof(CreateSchedulerTemplateExceptionsForResourceTimeRangeRequest));
					pRequest->ResourceID = _bstr_t(GetActiveResourceID());
					pRequest->date = GetActiveDate();
					pRequest->StartTime = dlgChooseDates.GetFromDate();
					pRequest->EndTime = dlgChooseDates.GetToDate();
					GetAPI()->CreateSchedulerTemplateExceptionsForResourceTimeRange(GetAPISubkey(), GetAPILoginToken(), pRequest);

					RefreshEverything();
				}
			}
		}
	}
	NxCatchAll(__FUNCTION__);
	PutFocusOnSingleDay();
}

void CTemplateItemEntryGraphicalDlg::UpdateDayOfWeekText()
{
	// (z.manning 2014-12-03 11:06) - PLID 64210 - This is hidden when editing collections
	if (m_eEditorType == stetCollection) {
		return;
	}

	CString strDay = FormatDateTimeForInterface(GetActiveDate(), "%A");
	GetDlgItem(IDC_STATIC_DAY_OF_WEEK)->ShowWindow(SW_HIDE);
	SetDlgItemText(IDC_STATIC_DAY_OF_WEEK, strDay);
	GetDlgItem(IDC_STATIC_DAY_OF_WEEK)->ShowWindow(SW_SHOW);
}

HBRUSH CTemplateItemEntryGraphicalDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// (a.walling 2008-05-22 12:44) - PLID 29497 - Use NxDialog base class
	HBRUSH hbr = CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch (nCtlColor)
	{	
		case CTLCOLOR_STATIC:
			if (pWnd->GetExStyle() & WS_EX_TRANSPARENT) {
				pDC->SetBkMode(TRANSPARENT);
				return (HBRUSH)GetStockObject(NULL_BRUSH);
			}
			break;

		case CTLCOLOR_DLG:
		case CTLCOLOR_BTN:
		case CTLCOLOR_MSGBOX:
		case CTLCOLOR_EDIT:
		default:
			break;
	}
	return hbr;
}

BOOL CTemplateItemEntryGraphicalDlg::OnEraseBkgnd(CDC* pDC)
{
	if (g_brTemplateCollectionBackground.m_hObject != NULL)
	{
		// (z.manning 2015-01-15 09:42) - PLID 64210 - We use a custom background color here
		// (Note: no exception handling here because we don't want to hinder drawing performance)
		CRect rcClient;
		GetClientRect(rcClient);
		pDC->FillRect(rcClient, &g_brTemplateCollectionBackground);

		//TRUE = No further erasing is required
		return TRUE;
	}
	else {
		return CNxDialog::OnEraseBkgnd(pDC);
	}
}

// (c.haag 2010-06-29 09:23) - PLID 39393 - We now take in a dispatch instead of a CReservation
void CTemplateItemEntryGraphicalDlg::SetClippedReservation(LPDISPATCH theRes, EClipReservationType eType)
{
	// (c.haag 2010-06-29 09:23) - PLID 39393 - All we need from theRes is its reservation ID. Get it and then never look back.
	CReservation pRes(__FUNCTION__, theRes);
	long nResID = pRes.GetReservationID();
	pRes.ReleaseAndClear();

	if (m_eEditorType == stetCollection)
	{
		// (z.manning 2014-12-18 08:47) - PLID 64225 - Handle template collection copying
		m_pClippedCollectionTemplate = NULL;

		_SchedulerTemplateCollectionTemplatePtr pClippedCollectionTemplate = m_pdlgTemplateList->GetCollectionTemplateByID(nResID);
		if (pClippedCollectionTemplate == NULL) {
			return;
		}

		m_pClippedCollectionTemplate.CreateInstance(__uuidof(SchedulerTemplateCollectionTemplate));
		m_pClippedCollectionTemplate->ID = pClippedCollectionTemplate->ID;
		m_pClippedCollectionTemplate->Template = pClippedCollectionTemplate->Template;
		m_pClippedCollectionTemplate->StartTime = pClippedCollectionTemplate->StartTime;
		m_pClippedCollectionTemplate->EndTime = pClippedCollectionTemplate->EndTime;
		m_pClippedCollectionTemplate->IsBlock = pClippedCollectionTemplate->IsBlock;
	}
	else
	{
		// (c.haag 2006-12-11 11:49) - PLID 23808 - Gracefully fail if the line item no longer exists
		//TES 6/19/2010 - PLID 5888 - Check the appropriate table.
		if (!ReturnsRecordsParam(FormatString("SELECT TOP 1 ID FROM %sTemplateItemT WHERE ID = {INT}", m_bUseResourceAvailTemplates ? "ResourceAvail" : ""), nResID)) {
			MessageBox("This line item has been deleted by another user.");
			UpdateView();
			return;
		}

		if (m_pClippedLineItem != NULL) {
			delete m_pClippedLineItem;
			m_pClippedLineItem = NULL;
		}

		m_pClippedLineItem = new CTemplateLineItemInfo;
		//TES 6/19/2010 - PLID 5888 - Pass in m_bUseResourceAvailTemplates
		m_pClippedLineItem->m_bUseResourceAvailTemplates = m_bUseResourceAvailTemplates;
		m_pClippedLineItem->LoadFromID(nResID);
	}

	m_eLastClipType = eType;
}

void CTemplateItemEntryGraphicalDlg::OnMouseUpTemplateItemEntryCtrl(short Button, short Shift, long x, long y) 
{
	try {

		if(Button == MK_RBUTTON) {
			// (c.haag 2010-06-29 09:23) - PLID 39393 - Pass in the dispatch directly instead of a CReservation
			// (we have none to begin with, so pass in NULL)	
			PopupContextMenu(NULL);
		}

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnMouseUpTemplateItemEntryCtrl");
}

// (z.manning, 11/27/2006) - PLID 23665 - Essentially a copy of CNxSchedulerDlg::GetMsSlot, but
// without handling the event single day.
int CTemplateItemEntryGraphicalDlg::GetCurrentSlot()
{
	int nSlot = 0;
	int nSlotHeight;
	int nTopSlot;
	CPoint pt;
	CRect rc;

	nSlotHeight = m_pSingleDayCtrl.GetSlotHeight();
	nTopSlot = m_pSingleDayCtrl.GetTopSlot();
	GetCursorPos(&pt);
	CWnd *pSingleDayCtrlWnd = CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd());
	pSingleDayCtrlWnd->GetWindowRect(rc);
	pSingleDayCtrlWnd->ClientToScreen(rc);

	pt.y += rc.top / 2;
	nSlot = (pt.y - rc.top) / nSlotHeight + nTopSlot;

	return nSlot;
}

// (z.manning, 11/27/2006) - PLID 23665 - Essentially a copy of CNxSchedulerDlg::GetMsTime, but
// without handling the event single day.
COleDateTime CTemplateItemEntryGraphicalDlg::GetCurrentSlotTime()
{
	int nSlot = GetCurrentSlot();
	COleDateTime dtBeginTime;
	dtBeginTime.ParseDateTime(m_pSingleDayCtrl.GetBeginTime());

	COleDateTimeSpan dtInterval(0, 0, m_pSingleDayCtrl.GetInterval(), 0);

	return ( dtBeginTime.m_dt + dtInterval * (double)nSlot );
}

// (z.manning, 11/27/2006) - PLID 23665 - We should have a line item in memory.  We need to update its times
// and any other applicable info and then save the new line item.
void CTemplateItemEntryGraphicalDlg::PasteLineItem(COleDateTime dtNewStart)
{
	// (z.manning, 12/11/2006) - PLID 23768 - Make sure we have the necessary permissions.
	ESecurityPermissionType eRequiredPermission;
	if(m_eLastClipType == crtCut) {
		// (z.manning, 02/14/2007) - PLID 23768 - Cut/paste should require delete permission as well.
		eRequiredPermission = (ESecurityPermissionType)(sptCreate|sptDelete);
	}
	else {
		ASSERT(m_eLastClipType == crtCopy);
		eRequiredPermission = sptCreate;
	}
	if(!CheckCurrentUserPermissions(m_bio,eRequiredPermission)) {
		return;
	}

	if (m_eEditorType == stetCollection)
	{
		// (z.manning 2014-12-18 08:50) - PLID 64225 - Handling pasting collection templates

		if (m_pClippedCollectionTemplate == NULL) {
			return;
		}

		_SchedulerTemplateCollectionPtr pCollection = m_pdlgTemplateList->GetSelectedCollection();
		if (pCollection == NULL) {
			return;
		}

		_SchedulerTemplateCollectionTemplateCommitPtr pCommit(__uuidof(SchedulerTemplateCollectionTemplateCommit));
		pCommit->CollectionID = pCollection->ID;
		pCommit->TemplateID = m_pClippedCollectionTemplate->Template->ID;
		pCommit->StartTime = dtNewStart;
		COleDateTime dtEndTime = COleDateTime(m_pClippedCollectionTemplate->EndTime) + (dtNewStart - COleDateTime(m_pClippedCollectionTemplate->StartTime));
		EnsureValidEndTime(dtEndTime);
		pCommit->EndTime = dtEndTime;
		pCommit->IsBlock = m_pClippedCollectionTemplate->IsBlock;
		GetAPI()->CreateSchedulerTemplateCollectionTemplate(GetAPISubkey(), GetAPILoginToken(), pCommit);

		if (m_eLastClipType == crtCut && m_pClippedCollectionTemplate->ID != g_cbstrNull) {
			GetAPI()->DeleteSchedulerTemplateCollectionTemplate(GetAPISubkey(), GetAPILoginToken(), m_pClippedCollectionTemplate->ID);
			m_pClippedCollectionTemplate->ID = g_cbstrNull;
		}

		RefreshEverything();
	}
	else
	{
		// (c.haag 2006-12-11 11:49) - PLID 23808 - Gracefully fail if the line item no longer exists
		//TES 6/19/2010 - PLID 5888 - Check the appropriate table.
		// (z.manning 2014-12-18 11:17) - PLID 64225 - This check doesn't make any sense. We are not copying
		// the line item from data, we're copying it from memory. And if they cut the line item it will be
		// deleted after the first place and this check would prevent them from pasting it more than once,
		// which they should be able to do.
		/*if (!ReturnsRecordsParam(FormatString("SELECT TOP 1 ID FROM %sTemplateItemT WHERE ID = {INT}", m_bUseResourceAvailTemplates ? "ResourceAvail" : ""), m_pClippedLineItem->m_nID)) {
			MessageBox("This line item has been deleted by another user.");
			UpdateView();
			return;
		}*/

		if (m_pClippedLineItem == NULL) {
			AfxThrowNxException("Tried to paste a NULL line item.");
		}
		ASSERT(m_eLastClipType != crtNone);

		// Handle monthly scales that are pattern based.
		if (m_pClippedLineItem->m_esScale == sMonthly && m_pClippedLineItem->m_embBy == mbPattern) {
			// For now, we do not support this as it is quite complex.
			MessageBox("Practice does not support copying monthly scaled template items that are by pattern. You must create the new line item manually.");
			if (m_pClippedLineItem) {
				// Get rid of the clipped item since they can't paste it anyway.
				delete m_pClippedLineItem;
			}
			m_pClippedLineItem = NULL;
			return;
		}

		// (a.vengrofski 2010-03-18 11:39) - PLID <28394> - New function which is the commented out code below.
		PasteSingleItem(m_pClippedLineItem, dtNewStart);
	}
}

COleDateTime CTemplateItemEntryGraphicalDlg::GetActiveDate()
{
	// (c.haag 2014-01-12) - PLID 64257 - Try to only return a date
	COleDateTime dt = m_dtpActiveDate.GetValue();
	if (dt.GetStatus() != COleDateTime::valid)
	{
		return dt;
	}
	else
	{
		COleDateTime dtDate;
		dtDate.SetDate(dt.GetYear(), dt.GetMonth(), dt.GetDay());
		return dtDate;
	}
}

// (z.manning 2009-07-10 10:54) - PLID 22054
void CTemplateItemEntryGraphicalDlg::SetActiveDate(const COleDateTime dt)
{
	m_dtpActiveDate.SetValue(dt);
}

long CTemplateItemEntryGraphicalDlg::GetActiveResourceID()
{
	// (z.manning 2014-12-09 15:59) - PLID 64210 - Don't have resources when editing collections
	if (m_eEditorType == stetCollection) {
		return -1;
	}

	if(m_bResourceListLoaded) {
		// (z.manning, 12/05/2006) - The resource list is loaded, so find the current selection.
		if(m_pdlResourceList->GetRowCount() > 0) {
			NXDATALIST2Lib::IRowSettingsPtr pCurResourceRow = m_pdlResourceList->GetCurSel();
			if(pCurResourceRow == NULL) {
				AfxThrowNxException("CTemplateItemEntryGraphicalDlg::GetActiveResourceID - There is not a selected resource.");
			}
			return VarLong(pCurResourceRow->GetValue(rlcID));
		}
		else {
			// Apparently we don't have any resources. This situation should have been handled elsewhere.
			return -1;
		}
	}
	else {
		// (z.manning, 12/05/2006) - The resouce list is still loading. This is only done when initializing
		// the dialog, so use the default resource ID. If this turns out to be an invalid ID, the 
		// TrySetSelFinished event will handle it.
		return m_nDefaultResourceID;
	}
}

CString CTemplateItemEntryGraphicalDlg::GetActiveResourceName()
{
	// (z.manning 2014-12-09 15:59) - PLID 64210 - Don't have resources when editing collections
	if (m_eEditorType == stetCollection) {
		return "";
	}

	if(!m_bResourceListLoaded) {
		m_pdlResourceList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	}

	NXDATALIST2Lib::IRowSettingsPtr pCurResourceRow = m_pdlResourceList->GetCurSel();
	if(pCurResourceRow != NULL) {
		CString strResourceName = VarString(pCurResourceRow->GetValue(rlcName));
		return strResourceName;
	}
	
	return "";
}

void CTemplateItemEntryGraphicalDlg::EnsureValidEndTime(COleDateTime &dtEndTime)
{
	COleDateTime dtMidnight;
	dtMidnight.SetTime(0,0,0);
	
	// Make sure we don't get past midnight of the next day or any midnight times at all.
	if( (dtEndTime >= COleDateTimeSpan(1,0,0,0)) || (CompareTimes(dtEndTime,dtMidnight) & CT_EQUAL) ) {
		dtEndTime.SetTime(23, 59, 0);
	}
}

// (c.haag 2010-07-14 16:31) - PLID 39615 - We now take in an LPDISPATCH instead of a CReservation
void CTemplateItemEntryGraphicalDlg::SplitBlockReservation(LPDISPATCH theBlock, COleDateTime dtSplitStart, COleDateTime dtSplitEnd, int nBlockPriority)
{
	// (z.manning, 12/05/2006) - PLID 23611 - Go through all existing non-blocks and either 
	// overwrite/split them or overwrite/split the block, depending on which has the higer priority.
	int nCount = m_pSingleDayCtrl.GetReservationCount(0);
	CReservation pBlock(__FUNCTION__, theBlock);
	for(int nResIndex = 0; nResIndex < nCount && pBlock != NULL; nResIndex++) 
	{				
		CReservation pExistingRes = m_pSingleDayCtrl.GetReservation(__FUNCTION__, 0, nResIndex);
		long nExistingResID = pExistingRes.GetReservationID();

		// We don't want to overwrite a block, so look that up.
		BOOL bCurResIsBlock = FALSE;
		if(!m_mapLineItemIDToIsBlock.Lookup(nExistingResID, bCurResIsBlock)) {
			ASSERT(FALSE);
		}
		if(!bCurResIsBlock)
		{
			// Lookup the priority of the non-block.
			long nExistingResPriority;
			if(!m_mapLineItemIDToPriority.Lookup(nExistingResID, nExistingResPriority)) {
				AfxThrowNxException(FormatString("Failed to find priority for line item ID: %li",nExistingResID));
			}

			// Note that it is intentional that we don't do less than or equal here. That's basically
			// saying that a block takes precedent over a non-block of the same template.
			if(nBlockPriority < nExistingResPriority) {
				// If the block has a lower priority than the existing template, we need to split/remove it
				// as though it were a non-block if they overlap at all.
				CReservation pSplitRes = SplitNonBlockReservation(pBlock, pExistingRes.GetStartTime(), pExistingRes.GetEndTime());
				if(pSplitRes) {
					// The block reservation was split in 2.  We need to make sure to try and split this
					// new block as well.
					// (c.haag 2010-07-14 16:31) - PLID 39615 - Make sure that this function call is completely done with
					// pSplitRes before passing the dispatch on to another function call.
					COleDateTime dtStart = pSplitRes.GetStartTime();
					COleDateTime dtEnd = pSplitRes.GetEndTime();
					LPDISPATCH theSplitRes = pSplitRes.GetDispatch();
					pSplitRes.ReleaseAndClear();
					SplitBlockReservation(theSplitRes, dtStart, dtEnd, nBlockPriority);
				}

				if(CompareTimes(pBlock.GetStartTime(),pBlock.GetEndTime()) & CT_GREATER_THAN_OR_EQUAL) {
					// If this is true then the block was completely overwritten, so get rid of it.
					pBlock.DeleteRes(__FUNCTION__);
					pBlock.SetDispatch(NULL);
				}
			}
			else  {
				// Ok, the current reservation is not a block and has a lower priority. So, if our block
				// overlaps it at all, we need to shrink/remove the non-block.
				SplitNonBlockReservation(pExistingRes, dtSplitStart, dtSplitEnd);

				if(CompareTimes(pExistingRes.GetStartTime(),pExistingRes.GetEndTime()) & CT_GREATER_THAN_OR_EQUAL) {
					// If this is true then an existing reservation was completely overwritten, so get rid of it
					// and then make sure we adjust the count accordingly.
					pExistingRes.DeleteRes(__FUNCTION__);
					pExistingRes.SetDispatch(NULL);
					nCount--;
					nResIndex--;
				}
			}
		}
	}			
}

void CTemplateItemEntryGraphicalDlg::OnTrySetSelFinishedResourceList(long nRowEnum, long nFlags) 
{
	// (z.manning, 01/18/2007) - Make sure this is done right away, so that anything we do within here
	// knows that the resource list is done loading.
	m_bResourceListLoaded = TRUE;

	try {

		if(nFlags == NXDATALIST2Lib::dlTrySetSelFinishedFailure) {
			// (z.manning, 12/04/2006) - We may have had an invalid default resource, just set
			// the selection to the top row.
			SetResourceListSelectionAnyRow();
		}

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnTrySetSelFinishedResourceList");
}

void CTemplateItemEntryGraphicalDlg::SetResourceListSelectionAnyRow()
{
	ASSERT(!m_pdlResourceList->IsRequerying());

	// (z.manning, 12/05/2006) - Let's just set it to the top row (assuming we have at least 1 row).
	if(m_pdlResourceList->GetRowCount() > 0) {
		m_pdlResourceList->PutCurSel(m_pdlResourceList->GetTopRow());
		UpdateView();
	}
	else {
		// The user must not have any available resources. Warn them but let them use the non-graphical
		// portion of the dialog.
		MessageBox("You do not permissions for any resources (or there are none in your system). You will not be able to use the graphical schedule template editor.");
		m_nDefaultResourceID = -1;
		CWnd *pSingleDayWnd = CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd());
		if(pSingleDayWnd) {
			pSingleDayWnd->EnableWindow(FALSE);
		}
	}
}

void CTemplateItemEntryGraphicalDlg::RefreshEverything()
{
	if (m_eEditorType == stetCollection) {
		// (z.manning 2014-12-18 11:32) - PLID 64210 - Collection templates are loaded when loading collections so if we
		// editing collections then we need to reload that list every time we do a full refresh.
		m_pdlgTemplateList->RequeryTemplateList();
	}

	// (z.manning, 12/06/2006) - PLID 23818 - Refresh our dialog and the scheduler.
	UpdateView();	

	CNxSchedulerDlg *pdlgScheduler = NULL;
	CChildFrame *frmSched = GetMainFrame()->GetOpenViewFrame(SCHEDULER_MODULE_NAME);
	if(frmSched) {
		CNxTabView *viewSched = (CNxTabView*)frmSched->GetActiveView();
		// (j.jones 2012-08-08 10:11) - PLID 51063 - added check that the active view is non-null
		if(viewSched) {
			pdlgScheduler = (CNxSchedulerDlg*)viewSched->GetActiveSheet();
			if(pdlgScheduler) {
				pdlgScheduler->m_bNeedUpdate = true;
				pdlgScheduler->PostMessage(NXM_UPDATEVIEW);
			}
		}
	}
}

void CTemplateItemEntryGraphicalDlg::OnHelp() 
{
	try {
		
		OpenManual("NexTech_Practice_Manual.chm", "System_Setup/Scheduler_Setup/Template_the_Schedule.htm");

	}NxCatchAll("CTemplateItemEntryGraphicalDlg::OnHelp");
}

void CTemplateItemEntryGraphicalDlg::OnMouseMoveTemplateItemEntryCtrl(short Button, short Shift, long x, long y) 
{
	try {

		// (z.manning, 01/15/2007) - PLID 7555 - The mouse if moving over the single day.  If the single day
		// does not have focus, make sure that it does to avoid annoying mouse wheel scrolling issues.
		if(GetFocus() != CWnd::FromHandle((HWND)m_pSingleDayCtrl.GethWnd())) {
			PutFocusOnSingleDay();
		}

	}NxCatchAllIgnore();
}

// (z.manning, 03/13/2007) - PLID 23635 - Sets the time range of the single day control based on the schedule preference.
void CTemplateItemEntryGraphicalDlg::SetVisibleTimeRange()
{
	COleDateTime dtStart, dtEnd, dtOldStart, dtOldEnd;
	GetScheduleVisibleTimeRange(m_lohOfficeHours, GetActiveDate().GetDayOfWeek(), dtStart, dtEnd);

	// (z.manning 2015-01-06 14:39) - PLID 64210 - Changed this to only update when it needs to
	dtOldStart.ParseDateTime(m_pSingleDayCtrl.GetBeginTime(), VAR_TIMEVALUEONLY);
	dtOldEnd.ParseDateTime(m_pSingleDayCtrl.GetEndTime(), VAR_TIMEVALUEONLY);
	if (!(CompareTimes(dtStart, dtOldStart) & CT_EQUAL) || !(CompareTimes(dtEnd, dtOldEnd) & CT_EQUAL)) {
		m_pSingleDayCtrl.PutBeginTime(_bstr_t(dtStart.Format(VAR_TIMEVALUEONLY)));
		m_pSingleDayCtrl.PutEndTime(_bstr_t(dtEnd.Format(VAR_TIMEVALUEONLY)));
	}
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CTemplateItemEntryGraphicalDlg::OnOK()
{
	//Eat the message
}

// (z.manning 2009-07-10 10:51) - PLID 22054 - Added an option to have this dialog be invisible
void CTemplateItemEntryGraphicalDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	try
	{
		if(!m_bVisible) {
			lpwndpos->flags &= ~SWP_SHOWWINDOW;
		}
		CNxDialog::OnWindowPosChanging(lpwndpos);

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-07-10 11:06) - PLID 22054 - Populates the array with all loaded resource IDs
void CTemplateItemEntryGraphicalDlg::GetAllLoadedResourceIDs(OUT CArray<long,long> &arynResourceIDs)
{
	m_pdlResourceList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	for(pRow = m_pdlResourceList->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
		const long nResourceID = VarLong(pRow->GetValue(rlcID));
		arynResourceIDs.Add(nResourceID);
	}
}

// (z.manning 2009-07-10 12:13) - PLID 22054 - Gets the total minutes for the given resource for
// the active date.
long CTemplateItemEntryGraphicalDlg::GetTotalMinutesByResourceID(const long nResourceID)
{
	long nMinutes = 0;
	if(!m_mapResourceIDToTotalMinutes.Lookup(nResourceID, nMinutes)) {
		return 0;
	}
	return nMinutes;
}

// (z.manning 2009-07-18 13:50) - PLID 22054
void CTemplateItemEntryGraphicalDlg::ClearTotalMinuteMap()
{
	m_mapResourceIDToTotalMinutes.RemoveAll();
}

// (j.jones 2010-12-09 12:27) - PLID 41782 - added ability to get an array of all start/end times
// for templates for a given resource ID
void CTemplateItemEntryGraphicalDlg::GetStartEndTimeArrayForResourceID(IN const long nResourceID, OUT CArray<ResourceStartEndTime, ResourceStartEndTime> &aryStartEndTimes)
{
	for(int i=0; i<m_aryResourceStartEndTimes.GetSize(); i++) {		
		ResourceStartEndTime rsetExist = (ResourceStartEndTime)m_aryResourceStartEndTimes.GetAt(i);
		//just send out the requested resource info
		if(rsetExist.nResourceID == nResourceID) {
			ResourceStartEndTime rsetNew;
			rsetNew.nResourceID = nResourceID;
			rsetNew.dtStartTime = rsetExist.dtStartTime;
			rsetNew.dtEndTime = rsetExist.dtEndTime;
			aryStartEndTimes.Add(rsetNew);
		}
	}
}

void CTemplateItemEntryGraphicalDlg::ClearStartEndTimeArray()
{
	m_aryResourceStartEndTimes.RemoveAll();
}
// (s.tullis 2015-08-24 14:37) - PLID 66411 - Renamed
// (z.manning 2009-07-18 13:36) - PLID 22054
void CTemplateItemEntryGraphicalDlg::SetTemplateIDFilter(CArray<long,long> &arynTemplateIDs)
{
	m_arynTemplateIDFilter.RemoveAll();
	m_arynTemplateIDFilter.Append(arynTemplateIDs);
}
// (a.vengrofski 2009-12-28 12:48) - PLID <28394> - Let's copy the entire day's worth of templates to another Resource and/or day.
void CTemplateItemEntryGraphicalDlg::CopyEntireDay()
{
	try {
		long nResToCopy = m_pSingleDayCtrl.GetReservationCount(0);
		COleDateTime dt = COleDateTime::GetCurrentTime();

		m_lResID = VarLong(m_pdlResourceList->GetCurSel()->GetValue(rlcID));
		if (nResToCopy == 0){
			MessageBox("There are no templates to copy.");
			return;
		}
		m_aryToBeCopied.RemoveAll();
		for (int i = 0; i < nResToCopy; i++){
			CReservation sdc = m_pSingleDayCtrl.GetReservation(__FUNCTION__, 0,i);
			if (sdc.GetReservationID() != -1){
				m_aryToBeCopied.Add(sdc.GetReservationID());
			}
		}
	}NxCatchAll("CTemplateItemEntryGraphicalDlg::CopyEntireDay");
}
// (a.vengrofski 2010-03-19 10:45) - PLID <28394> - Seperated the copy from the paste functions.
void CTemplateItemEntryGraphicalDlg::PasteEntireDay()
{
	if(!CheckCurrentUserPermissions(m_bio,sptCreate)) {
		return;
	}
	BOOL bMonthlyWarn = FALSE;
	for (int i = 0; i < m_aryToBeCopied.GetSize(); i++){
		if(m_pCopyAllLineItem != NULL) {
			delete m_pCopyAllLineItem;
			m_pCopyAllLineItem = NULL;
		}
		m_pCopyAllLineItem = new CTemplateLineItemInfo;
		//TES 7/2/2010 - PLID 39262 - Pass in the resource avail templates flag
		m_pCopyAllLineItem->m_bUseResourceAvailTemplates = m_bUseResourceAvailTemplates;
		// (z.manning 2010-08-31 13:35) - PLID 39596 - Tell this function not to throw an exception since any of the
		// template items on the copied day could have been deleted by now. Instead check to see if we successfully
		// loaded a line item before attempting to copy it.
		if(m_pCopyAllLineItem->LoadFromID(m_aryToBeCopied.ElementAt(i), TRUE)) {
			if(m_pCopyAllLineItem->m_esScale == sMonthly && m_pCopyAllLineItem->m_embBy == mbPattern) {
				if (!bMonthlyWarn){
					MessageBox("Monthly scaled template items that are by pattern cannot be copied. They must be created manually.");
					bMonthlyWarn = TRUE;
				}
				continue;
			}
			PasteSingleItem(m_pCopyAllLineItem, m_pCopyAllLineItem->m_dtStartTime, TRUE);
		}
	}
	//TES 6/21/2010 - PLID 5888 - Send the appropriate tablechecker
	CClient::RefreshTable(m_bUseResourceAvailTemplates?NetUtils::ResourceAvailTemplateItemT:NetUtils::TemplateItemT, -1);
	RefreshEverything();
}


// (a.vengrofski 2010-03-18 10:36) - PLID <28394> - Split common paste functionality into this function.
//If bNoRefresh is true it is the responsibility of the caller to notify and refresh.
void CTemplateItemEntryGraphicalDlg::PasteSingleItem(CTemplateLineItemInfo* pSourceLineItem, COleDateTime dtNewStart, BOOL bNoRefresh /*= FALSE*/)
{

	BOOL bValid = TRUE;

	CTemplateLineItemInfo* pNewLineItem = new CTemplateLineItemInfo(pSourceLineItem);
	pNewLineItem->SetNew();

	// Find the new time based on what slot the mouse is in.
	pNewLineItem->m_dtEndTime = pNewLineItem->m_dtEndTime + (dtNewStart - pNewLineItem->m_dtStartTime);
	pNewLineItem->m_dtStartTime = dtNewStart;
	EnsureValidEndTime(pNewLineItem->m_dtEndTime);

	// Here is how we handle copying line item resources:
	// -- If it's all resources, then we make the new one all resources.
	// -- If it's only 1 resource, then we replace it with the active resource (if it's not already).
	// -- If it's multiple resources, we ensure the active resource is there and then copy it as is.
	if(!pNewLineItem->m_bAllResources) {
		BOOL bActiveResourceFound = FALSE;
		for(int i = 0; i < pNewLineItem->GetResourceCount(); i++) {
			if(pNewLineItem->GetResourceByIndex(i).m_nID == GetActiveResourceID()) {
				bActiveResourceFound = TRUE;
				break;
			}
		}

		if(!bActiveResourceFound) {
			if(pNewLineItem->GetResourceCount() == 1) {
				pNewLineItem->RemoveAllResources();
			}
			// (z.manning 2011-12-07 11:40) - PLID 46910 - Pass in the resource name too
			pNewLineItem->AddResource(GetActiveResourceID(), GetActiveResourceName());
		}
	}

	// Make the start date of the new line item the active date no matter what. This will ensure that
	// all scales (e.g. bi-weekly) will behave properly.
	pNewLineItem->m_dtStartDate = GetActiveDate();
	// For the end date of the line item, we will just leave it the same UNLESS they are pasting to
	// a date outside the line item's range.
	// Also, for the once scale we only want a one-day range no matter what.
	if( (!pNewLineItem->NeverEnds() && GetActiveDate() > pNewLineItem->m_dtEndDate)	|| pNewLineItem->m_esScale == sOnce )
	{
		pNewLineItem->m_dtEndDate = GetActiveDate();
	}

	// Regardless of what days are selected in the clipped line item, only select the day of the week
	// of the active date in order to keep things simpler and now having users inadvertantly creating
	// line items where they did not intend to. (Note: we do this regarless of scale even though not 
	// all scales use this.)
	pNewLineItem->m_nInclude = DAYVAL(GetActiveDate().GetDayOfWeek() - 1);

	// Set the day number to the active date (again, regardless of scale).
	pNewLineItem->m_nDayNumber = GetActiveDate().GetDay();

	// (z.manning, 12/13/2006) - PLID 23665 - Do NOT copy line item exceptions as this is a new line
	// item so they do not apply.
	pNewLineItem->m_arydtExceptionDates.RemoveAll();

	// All right, line item should be updated, so let's prompt them and then save assuming we can.
	CString strSql = "";
	if(bValid) {
		try {
			//CString strConfirmation = FormatString("Do you want to create the following template line item?\r\n\r\n%s", pNewLineItem->GetApparentDescription());
			//if(IDYES == MessageBox(strConfirmation, "Confirm New Template Item", MB_YESNO)) {
			pNewLineItem->AddSaveStringToBatch(strSql);

			// If this was a cut, we need to delete the existing line item.
			if(m_eLastClipType == crtCut) {
				AddStatementToSqlBatch(strSql, pSourceLineItem->GenerateDeleteString());
			}
			ExecuteSqlBatch(strSql);
			if (!bNoRefresh){// (a.vengrofski 2010-03-18 16:00) - PLID <28394> - If this is part of a larger batch execute the query and it is the responsibility of the caller to notify and refresh.
			// (c.haag 2006-12-12 10:58) - PLID 23808 - Notify other computers about the change.
				//TES 6/21/2010 - PLID 5888 - Send the appropriate tablechecker
				CClient::RefreshTable(m_bUseResourceAvailTemplates?NetUtils::ResourceAvailTemplateItemT:NetUtils::TemplateItemT, -1);
				RefreshEverything();
			}
		}NxCatchAll("CTemplateItemEntryGraphicalDlg::PasteLineItem - Failed to save new line item.");
	}

	if(pNewLineItem) {
		delete pNewLineItem;
	}
}

// (z.manning 2014-12-03 13:02) - PLID 64210
void CTemplateItemEntryGraphicalDlg::UpdateVisibleControls()
{
	if (m_eEditorType == stetLocation || m_eEditorType == stetCollection) {
		//TES 6/19/2010 - PLID 5888 - We haven't written help for the Resource Availability templates yet.
		GetDlgItem(IDC_SCHEDULE_TEMPLATING_HELP)->EnableWindow(FALSE);
		GetDlgItem(IDC_SCHEDULE_TEMPLATING_HELP)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CREATE_COLLECTION_FROM_VIEW_BTN)->ShowWindow(SW_HIDE); // (c.haag 2014-12-15) - PLID 64246
		GetDlgItem(IDC_REMOVE_TEMPLATES_BTN)->ShowWindow(SW_HIDE); // (c.haag 2014-12-17) - PLID 64253
	}

	if (m_eEditorType == stetCollection)
	{
		GetDlgItem(IDC_DATE_SEL_COMBO)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MOVE_DAY_FORWARD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_MOVE_DAY_BACK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TODAY_BTN)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_OPTIONS_BTN)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STATIC_DAY_OF_WEEK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_RESOURCE_LIST)->ShowWindow(SW_HIDE);
	}
}

// (c.haag 2015-01-02) - PLID 64257 - Show the hyperlink cursor when hovering over a hyperlink
BOOL CTemplateItemEntryGraphicalDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	try 
	{
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
		case IDC_STATIC_COLLECTIONS_USED:
			if (m_nxlCollectionsUsed.IsWindowVisible() && dtsHyperlink == m_nxlCollectionsUsed.GetType())
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
			break;
		case IDC_TODAY_BTN:
		case IDC_AM_PM_BTN:
			// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
			SetCursor(GetLinkCursor());
			return TRUE;
		default:
			break;
		}

	} NxCatchAll(__FUNCTION__);

	return __super::OnSetCursor(pWnd, nHitTest, message);
}

// (c.haag 2015-01-02) - PLID 64257 - Handle label clicks
LRESULT CTemplateItemEntryGraphicalDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try 
	{
		UINT nIdc = (UINT)wParam;
		switch (nIdc) {
		case IDC_STATIC_COLLECTIONS_USED:
			// Make sure the label was visible and in hyperlink form first
			if (m_nxlCollectionsUsed.IsWindowVisible() && dtsHyperlink == m_nxlCollectionsUsed.GetType())
			{
				// (c.haag 2015-01-02) - PLID 64257 - Bring up a multi-select dialog containing a data list of 
				// sortable collection names where the user can select the collections they wish to add to that day.
				// To bring this up appropriately, we need to know which collections are applied to this day as well.
				// We can use m_UsedCollections to find this out.
				CMultiSelectDlg dlg(this, "TemplateCollectionT");
				std::set<long> aOldSections;
				CArray<long, long> anIDs;
				CStringArray astrNames;

				// (c.haag 2014-01-06) - PLID 64520 - Use SchedulerTemplateCollectionFilter
				_SchedulerTemplateCollectionFilterPtr pCollectionFilter(__uuidof(SchedulerTemplateCollectionFilter));
				_SchedulerTemplateCollectionsPtr pCollections = GetAPI()->GetSchedulerTemplateCollections(GetAPISubkey(), GetAPILoginToken(), Nx::SafeArray<IUnknown*>::FromValue(pCollectionFilter));
				Nx::SafeArray<IUnknown *> saCollections(pCollections->Collections);
				for each (_SchedulerTemplateCollectionPtr pCollection in saCollections)
				{
					Nx::SafeArray<IUnknown *> saTemplates(pCollection->Templates);
					if (saTemplates.GetCount() == 0 && !IsCollectionUsed(atol(pCollection->ID)))
					{
						// If there are no templates then we can't apply it to this day
					}
					else
					{
						long nID = atol(pCollection->ID);
						anIDs.Add(nID);						
						astrNames.Add(pCollection->Name);
						if (IsCollectionUsed(nID))
						{
							// The ID has to be cast into a string for pre-selections to work. I entered PLID 64514 to fix this
							// in the multi-select list.
							dlg.PreSelect(AsString(nID), TRUE);
							aOldSections.insert(nID);
						}
					}
				}

				if (0 == anIDs.GetSize())
				{
					// If we get here, it means there are no collections we can put in the list.
					MessageBox("There are no collections that can be applied to this day.", NULL, MB_ICONINFORMATION | MB_OK);
				}
				else if (IDOK == dlg.Open(anIDs, astrNames, "Please select collections to apply to this day. If you select a collection, a new apply will be created for it. If you unselect a collection, exceptions will be made for its applies in the current view."))
				{
					std::set<long> aNewSelections;
					dlg.FillArrayWithIDs(aNewSelections);
					
					// Find the ones to remove and the ones to add
					std::set<long> setRemovedIDs;
					std::set<long> setAddedIDs;
					boost::set_difference(aOldSections, aNewSelections, inserter(setRemovedIDs, setRemovedIDs.end()));
					boost::set_difference(aNewSelections, aOldSections, inserter(setAddedIDs, setAddedIDs.end()));

					// (c.haag 2015-01-07) - PLID 64353 - Warn the user if they intend to do applies in the presence of existing resources
					if (setAddedIDs.size() > 0 && m_pSingleDayCtrl.GetReservationCount(0) > 0)
					{
						enum Commands {
							eClearExistingTemplates = 0x100,
							eAddToExistingTemplates,
						};

						CString strWarning = FormatString(R"(The resource '%s' already has templates on the scheduler for %s.

Please make a selection below to proceed, or you may cancel this action:)"
, GetActiveResourceName(), FormatDateTimeForInterface(GetActiveDate(), 0, dtoDate));
						NxTaskDialog dlg;
						dlg.Config()
							.WarningIcon()
							.CancelOnly()
							.MainInstructionText(strWarning)
							.AddCommand(eClearExistingTemplates, "Clear existing templates and replace them with templates from the previously selected collections.")
							.AddCommand(eAddToExistingTemplates, "Add templates from the previously selected collections to the existing templates.")
							.DefaultButton(IDCANCEL);
						auto result = dlg.DoModal();
						switch (result)
						{
						case eClearExistingTemplates:
							// (c.haag 2014-01-12) - PLID 64257 - All templates should be cleared off today's resource schedule, and templates of collections selected from the list
							// should be applied.

							// First, clear today's resource schedule.
							{
								// Generate a warning message. The warning can be one of the following values:
								// - Blank - That means no data would be modified by CreateSchedulerTemplateExceptionsForResourceTimeRange
								// - Anything else - The user needs to confirm before proceeding
								CString strWarning = GetRemoveTemplatesWarningText();
								if (strWarning.IsEmpty())
								{
									// No data would be modified by CreateSchedulerTemplateExceptionsForResourceTimeRange
								}
								else
								{
									if (IDNO == MessageBox(strWarning, NULL, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
									{
										// Declined to continue
										return 0;
									}
								}

								COleDateTime dtStartTime, dtEndTime;
								dtStartTime.SetTime(0, 0, 0);
								dtEndTime.SetTime(23, 59, 59);

								_CreateSchedulerTemplateExceptionsForResourceTimeRangeRequestPtr pRequest(__uuidof(CreateSchedulerTemplateExceptionsForResourceTimeRangeRequest));
								pRequest->ResourceID = _bstr_t(GetActiveResourceID());
								pRequest->date = GetActiveDate();
								pRequest->StartTime = dtStartTime;
								pRequest->EndTime = dtEndTime;
								GetAPI()->CreateSchedulerTemplateExceptionsForResourceTimeRange(GetAPISubkey(), GetAPILoginToken(), pRequest);
							}

							// Now that the day is clear in data, we need to clear out setRemovedIDs since there's nothing left to remove,
							// and assign everything in aNewSelections to setAddedIDs
							setRemovedIDs.clear();
							setAddedIDs = aNewSelections;
							break;
						case eAddToExistingTemplates:
							// The user selected No. That means all templates should stay the same but for ones tied to collections
							// that the user selected or unselected.
							break;
						case IDCANCEL:
							return 0;
						}
					}

					// (c.haag 2014-01-12) - PLID 64257 - If we need to change anything, then do it through the API
					if (setAddedIDs.size() > 0 || setRemovedIDs.size() > 0)
					{
						Nx::SafeArray<BSTR> addedIDs;
						for each (long id in setAddedIDs)
						{
							addedIDs.Add(_bstr_t(id));
						}
						Nx::SafeArray<BSTR> removedIDs;
						for each (long id in setRemovedIDs)
						{
							removedIDs.Add(_bstr_t(id));
						}
						GetAPI()->ChangeCollectionPresencesForResourceDate(GetAPISubkey(), GetAPILoginToken(),
							_bstr_t(GetActiveResourceID()), GetActiveDate(), addedIDs, removedIDs);
					}

					// Reflect the data changes on the screen and notify other workstations
					RefreshEverything();
				}
			}
			break;
		case IDC_TODAY_BTN:
			// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
			OnTodayBtn();
			break;
		case IDC_AM_PM_BTN:
			// (b.cardillo 2016-06-04 19:56) - NX-100776 - Change the Today and AM/PM buttons into links
			OnAmPmBtn();
			break;
		default:
			break;
		}
	} NxCatchAll(__FUNCTION__);
	return 0;
}

// (s.tullis 2015-06-17 11:30) - PLID 66401 - Utility fuction to find a Template Lineitem in the array
CTemplateLineItemInfo* CTemplateItemEntryGraphicalDlg::FindLineItemInArray(CArray<CTemplateLineItemInfo*, CTemplateLineItemInfo*> &arypLineItems, long nTemplateID, long nLineitemID){
	CTemplateLineItemInfo* pLineItem = NULL;
	const int nLineItemArraySize = arypLineItems.GetSize();
	try{
		for (int i = 0; i < nLineItemArraySize; i++)
		{
			if (arypLineItems.GetAt(i)->m_nTemplateID == nTemplateID && arypLineItems.GetAt(i)->m_nID == nLineitemID)
			{
				pLineItem = arypLineItems.GetAt(i);
				return pLineItem;
			}
		}

	}NxCatchAll(__FUNCTION__)
	return pLineItem;
}


// NxTabView.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NxTabView.h"
#include "GlobalUtils.h"
#include "General1Dlg.h"
#include "FirstAvailableAppt.h"
#include "ChildFrm.h"
#include "Barcode.h"
#include "ContactView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_TABS 512 //arbitrary

/////////////////////////////////////////////////////////////////////////////
// CNxTabView

IMPLEMENT_DYNCREATE(CNxTabView, CNxView)

CNxTabView::CNxTabView()
{
	m_pActiveSheet = NULL;
	m_defaultTab = -1;
	m_pSheet.SetSize(4, 4);
	m_pSheet.RemoveAll();

	m_hSetSheetFocus = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_bIsPrintPreviewRunning = false;

}

CNxTabView::~CNxTabView()
{
	m_pActiveSheet = 0;
	m_pSheet.RemoveAll();

	if (m_hSetSheetFocus)
	{
		CloseHandle(m_hSetSheetFocus);
		m_hSetSheetFocus = NULL;
	}
}


BEGIN_MESSAGE_MAP(CNxTabView, CNxView)
	//{{AFX_MSG_MAP(CNxTabView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CNxTabView, CNxView)
    //{{AFX_EVENTSINK_MAP(CNxTabView)
		ON_EVENT(CNxTabView, IDC_TABS, 1 /* SelectTab */, OnSelectTab, VTS_I2 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()
/////////////////////////////////////////////////////////////////////////////
// CNxTabView drawing

void CNxTabView::SetDefaultControlFocus()
{
	if (m_pActiveSheet)
	{
		CWnd* pWnd = m_pActiveSheet->GetNextDlgTabItem(NULL);
		if (pWnd) pWnd->SetFocus();
	}
}

void CNxTabView::OnDraw(CDC* pDC)
{
	// If the event to set focus to the first tabbing sheet control
	// is signaled, reset the event and set the focus.
	if (m_hSetSheetFocus && WaitForSingleObject(m_hSetSheetFocus, 0) == WAIT_OBJECT_0)
	{
		ResetEvent(m_hSetSheetFocus);
		SetDefaultControlFocus();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CNxTabView diagnostics

#ifdef _DEBUG
void CNxTabView::AssertValid() const
{
	CView::AssertValid();
}

void CNxTabView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CNxTabView message handlers

#define TABHEIGHT 24
#define TOPMARGIN 0	//JJ - this number will put space between the top of the tabs and the toolbars, used to be 4

BOOL CNxTabView::OnEraseBkgnd(CDC* pDC) 
{
	CRect rect;
	GetClientRect(rect);
	if (m_pActiveSheet->GetSafeHwnd()) {
		// If there's a sheet, then all we want to erase is the area behind the tabs
		// (a.walling 2008-04-01 17:30) - PLID 29497 - This is what it shoudl be; was using
		// TOPMARGIN which is effectively a rect with no area.
		rect.bottom = TABHEIGHT;
	}

	pDC->FillRect(rect, &m_brush);
	
	//	Background will NOT be filled by CWnd, it will be filled by the sheet
	return TRUE;
}

void CNxTabView::OnSize(UINT nType, int cx, int cy) 
{
	CNxView::OnSize(nType, cx, cy);

	// Resize the tab control to fix view
	if (m_wndTab.GetSafeHwnd())
	{
		// (b.cardillo 2004-03-18 15:10) - We used to SetRedraw(FALSE) here and put it back to true 
		// after all the sizing.  But doing this causes the CNxDialog::SetRgnBg() function to think 
		// the dialog is not visible, and therefore that all its controls are not visible.  Since it 
		// skips invisible children in it's generation of the background region it was skipping ALL 
		// children, and therefore the background region was just the whole dialog, which defeats the 
		// purpose of having a background region in the first place.  Also, the SetRedraw had no 
		// discernable effect on Windows XP in either classsic mode or XP mode.  My guess is this 
		// might have been our workaround for other operating systems so we need to test my change on 
		// Win98, WinNT and Win2K.  I don't expect to find any problems though because we're now 
		// moving all the windows with the bRedraw flag set to FALSE, which means nothing should 
		// happen on screen until the app's message queue starts pumping again, and then all the 
		// sizing will appear to happen simultaneously which was the point of the SetRedraw stuff in 
		// the first place.


		m_wndTab.MoveWindow(0, TOPMARGIN, cx, TABHEIGHT, FALSE);

		// Recalc the sheet rectangle
		GetClientRect(&m_rectSheet);
		m_rectSheet.top += TABHEIGHT + TOPMARGIN;

		// Reposition and resize whatever the current sheet is
		if (m_pActiveSheet && m_pActiveSheet->GetSafeHwnd()) {
			m_pActiveSheet->MoveWindow(m_rectSheet, FALSE);
			m_pActiveSheet->SetControlPositions();
		}
	}
}

// Returns the new tab's 0-based index on success, INVALID_SHEET on error
// (a.walling 2008-07-02 17:42) - PLID 27648 - Support the alternate Short Title
short CNxTabView::CreateSheet(CNxDialog * pNewSheet, LPCTSTR strTitle /* = NULL */, LPCTSTR strShortTitle /* = NULL */)
{
	if (!m_wndTab.GetSafeHwnd())
		return INVALID_SHEET;

	short nNewTabIndex = m_tab->Size;
	m_tab->Size = nNewTabIndex + 1;
	m_tab->Label[nNewTabIndex] = strTitle;
	m_tab->ShortLabel[nNewTabIndex] = strShortTitle; // (a.walling 2008-07-03 09:27) - PLID 27648

	// Add the new sheet to the list of sheets
	m_pSheet.Add(pNewSheet);
	
	// Return the index of the new tab
	return nNewTabIndex;
}

// (a.walling 2010-11-26 13:08) - PLID 40444 - Create a sheet by calling into the modules code to get the tab info
short CNxTabView::CreateSheet(CNxDialog *pNewSheet, Modules::Tabs& tabs, int ordinal)
{
	Modules::ElementPtr pElement = tabs[ordinal];
	if (!pElement) {
		ThrowNxException("Invalid module tab!");
	}
	return CreateSheet(pNewSheet, pElement->Name(), pElement->ShortName());
}

int CNxTabView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndTab.CreateControl(_T("NXTAB.NxTabCtrl.1"), "NxTabView", WS_CHILD, CRect(0,0,0,0), this, IDC_TABS))
		return -1;

	m_tab = m_wndTab.GetControlUnknown();
	m_tab->TabWidth = 12;
	m_tab->Size = 0;
	//TES 10/29/03: The theory here is that OnSelectTab will get -1 as oldTab and realize that
	//the tab is being selected as part of the initialization of the module, and will therefore
	//no to handle it slightly differently than normal tab selection.
	m_tab->CurSel = -1;
	m_wndTab.ShowWindow(SW_SHOW);

	// (j.jones 2016-04-15 11:26) - NX-100214 - changed this to the standard NxDialog brush
	m_brush.CreateSolidBrush(CNxDialog::GetSolidBackgroundColor());

	return 0;
}

void CNxTabView::OnSelectTab(short newTab, short oldTab)
{
	CWaitCursor wait;
	CNxDialog *oldSheet;

	if (newTab == -1) {	// This can happen if you click in the dark grey area in the scheduler
		m_tab->CurSel = oldTab;
		return;
	}
	
	oldSheet = m_pActiveSheet;
	m_pActiveSheet = (CNxDialog *)m_pSheet[newTab];

	if (!m_pActiveSheet)//dialog sheet must exist
	{	HandleException(NULL, "Dialog sheet not found");
		m_pActiveSheet = oldSheet;
		m_tab->CurSel = oldTab;
		return;
	}

	//DRT 8/27/03 - There is a (big) problem with the saving here.  We are creating the new tab (if need
	//	be), before calling save on the old one.  However, if we create it, OnInitDialog gets called, and
	//	the focus is set to a new control.  Then we call StoreDetails, which in the case of General1 at 
	//	least, calls GetFocus() to see what control needs to be saved.  I can't find any harm in saving 
	//	here, before the active sheet is setup.  
	if(oldSheet != m_pActiveSheet) {
		if (oldSheet->GetSafeHwnd())
			oldSheet->StoreDetails();
	}

	// If dialog hasn't been created yet, do it
	if (!m_pActiveSheet->GetSafeHwnd()) 
	{
		//once this line executes, it's very likely the focus has moved to a control on our
		//new sheet
		if (!m_pActiveSheet->Create(m_pActiveSheet->IDD, this)) 
		{	HandleException(NULL, "Dialog creation failed");
			m_pActiveSheet = oldSheet;
			m_tab->CurSel = oldTab;
			return;
		}
	}

	//If we changed tabs
	if (oldSheet != m_pActiveSheet)
	{	//See above comments for why this is saved up there
//		if (oldSheet->GetSafeHwnd())
//			oldSheet->StoreDetails();

		//Load the new tab
		m_pActiveSheet->MoveWindow(m_rectSheet);
		m_pActiveSheet->SetControlPositions();
		m_pActiveSheet->RecallDetails();

		if (oldSheet->GetSafeHwnd())
			oldSheet->EnableWindow(FALSE);
		
		// CAH 11/2: This is called in RecallDetails
		// (a.walling 2010-10-12 17:00) - PLID 40906 - bForceRefresh should be false when simply selecting the tab
		// (a.walling 2011-01-17 15:03) - PLID 40906 - Since table checker updates could not fit into scope, reverting to always 'force' the refresh.
		m_pActiveSheet->UpdateView(true);
		m_pActiveSheet->ShowWindow(SW_SHOW);

		if (oldSheet->GetSafeHwnd())
		{
			oldSheet->ShowWindow(SW_HIDE);
			oldSheet->EnableWindow(TRUE);
		}

		// Signal the event indicating that we want to set
		// the focus of the current sheet to the first control
		// in the tab order list.
		SetEvent(m_hSetSheetFocus);
	}
}

void CNxTabView::RecallDetails(CNxDialog * newSheet)
{

}

void CNxTabView::StoreDetails(CNxDialog * oldSheet)
{

}

// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
void CNxTabView::UpdateView(bool bForceRefresh)
{
	if (m_pActiveSheet->GetSafeHwnd()) {
		m_pActiveSheet->UpdateView(bForceRefresh);
	}
}

void CNxTabView::OnFilePrint()
{
	// (b.cardillo 2004-08-24 12:00) - PLID 13969 - We used to call PrePrint here but see c.haag's 
	// comment below, it had to be moved to a more correct spot, so the derived class implmenetations 
	// of PrePrint would work properly.  But if anyone is relying on the screenshot mechanism built 
	// into NxDialog (even if derived classes make use of it) the screenshot has to be taken here 
	// BEFORE the print dialog is put on the screen...otherwise the print dialog will be included in 
	// the screenshot, and that would make no sense.
	if (m_pActiveSheet->GetSafeHwnd()) {
		// Copy a picture of the sheet to memory
		m_pActiveSheet->StorePrintImage();
	}

	// (c.haag 2003-10-20 12:16) - I moved PrePrint to OnBeginPrinting
	// because PrePrint should be called after the print settings are
	// determined.

	CView::OnFilePrint();

	// (c.haag 2003-10-20 12:07) - FYI: OnPrint() may actually be fired
	// before this function actually exits.
}

void CNxTabView::OnFilePrintPreview()
{	
	// (b.cardillo 2004-08-24 12:00) - PLID 13969 - We used to call PrePrint here but see c.haag's 
	// comment below, it had to be moved to a more correct spot, so the derived class implmenetations 
	// of PrePrint would work properly.  But if anyone is relying on the screenshot mechanism built 
	// into NxDialog (even if derived classes make use of it) the screenshot has to be taken here 
	// BEFORE the print dialog is put on the screen...otherwise the print dialog will be included in 
	// the screenshot, and that would make no sense.
	if (m_pActiveSheet->GetSafeHwnd()) {
		// Copy a picture of the sheet to memory
		m_pActiveSheet->StorePrintImage();
	}

	// (c.haag 2003-10-20 12:16) - I moved PrePrint to OnBeginPrinting
	// because PrePrint should be called after the print settings are
	// determined.

	CView::OnFilePrintPreview();

	// (c.haag 2003-10-20 12:07) - FYI: OnPrint() may actually be fired
	// before this function actually exits.
}

void CNxTabView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	if (m_pActiveSheet->GetSafeHwnd()) {
		m_pActiveSheet->Print(pDC, pInfo);
	}
}

BOOL CNxTabView::OnPreparePrinting(CPrintInfo* pInfo) 
{
   return DoPreparePrinting(pInfo);
}

void CNxTabView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	// Call default function
	CView::OnEndPrinting(pDC, pInfo);

	//Allow them to close Practice again.
	m_bIsPrintPreviewRunning = false;
	
	// Unselect the bitmap and delete it
	if (m_pActiveSheet) {
		m_pActiveSheet->PostPrint();
	}
}

void CNxTabView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	// Standard is to print exactly one per page
	//pInfo->SetMinPage(1);
	//pInfo->SetMaxPage(1);
	//Prevent Practice from closing while the preview is on screen.
	m_bIsPrintPreviewRunning = true;

	if (m_pActiveSheet->GetSafeHwnd()) {
		// Copy a picture of the sheet to memory
		m_pActiveSheet->PrePrint();
	}

	CView::OnBeginPrinting(pDC, pInfo);
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CNxTabView::Hotkey (int key)
{
	if (m_pActiveSheet)
		return m_pActiveSheet->Hotkey(key);

	//unhandled
	return 1;
}

BOOL CNxTabView::PreTranslateMessage(MSG *pMsg) 
{	//keeps left and right from swapping tabs
	if (pMsg->message == WM_KEYDOWN) 
		switch (pMsg->wParam)
		{	case VK_LEFT:
			case VK_RIGHT:
			case VK_UP:
			case VK_DOWN:
				return TRUE;
			break;
		}
	return CNxView::PreTranslateMessage(pMsg);
}

static unsigned int Bitcount(const unsigned int source)
{
	int count = 0;
	for (int i = 0; i < 32; i++)
		if (source & (0x1<<i))
			count++;
	return count;
}

short CNxTabView::GetActiveTab()
{
	return m_tab->CurSel;
}

BOOL CNxTabView::SetActiveTab(short tab)
{
	BOOL tabOK = TRUE;
	int oldTab = m_tab->CurSel;
	
	if (tab >= m_tab->Size) {
		tab = m_tab->Size - 1;
		tabOK = FALSE;
	} else if (tab < 0) {
		tab = 0;
		tabOK = FALSE;
	}

	if(oldTab == -1) oldTab = tab;

	m_tab->CurSel = tab;
	OnSelectTab(tab, oldTab);

	return tabOK;
}

void CNxTabView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	// (a.walling 2010-12-09 09:25) - PLID 40444 - Handle an override default tab to open a module immediately to the desired tab
	// rather than opening to the default tab and then switching
	if (!m_pActiveSheet && bActivate) {
		short nOverrideDefaultTab = -1;

		// Determine the default tab
		if (m_defaultTab == -1) {
			CWnd *pParent = GetParent();
			if (pParent && pParent->IsKindOf(RUNTIME_CLASS(CChildFrame))) {
				CChildFrame *pChildFrame = (CChildFrame *)pParent;
				CString strModuleName = pChildFrame->GetType();
				short nDefaultTabProperty = (short)GetRemotePropertyInt("MyDefaultTab_" + strModuleName, 0, 0, GetCurrentUserName(), false);
				m_defaultTab = g_Modules[strModuleName]->ResolveDefaultTab(nDefaultTabProperty);

				// (a.walling 2010-12-09 09:25) - PLID 40444 - Get the override tab, but still set m_defaultTab as appropriate.
				nOverrideDefaultTab = g_Modules[strModuleName]->OverrideDefaultTab();
				
				if(strModuleName == "Contacts") {
					// (k.messina 2010-08-25 12:57) - PLID 37957 lock internal contact notes
					if(IsNexTechInternal()) {
						if(( (CContactView*)this )->CheckViewNotesPermissions()) {
							m_defaultTab = 0; //switch to general tab if permissions are not allowed
							nOverrideDefaultTab = -1;
						}
					}
				}
			}
		}

		// (a.walling 2010-12-09 09:25) - PLID 40444 - Activate the override tab if available
		if (nOverrideDefaultTab != -1) {
			SetActiveTab(nOverrideDefaultTab);
		} else {		
			if (m_defaultTab != -1) {
				SetActiveTab(m_defaultTab);
			}
		}
	}

	CNxView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

LRESULT CNxTabView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	try {
		switch (message) {
		case NXM_ALLOW_CLOSE:
			if (m_bIsPrintPreviewRunning == true) {
				MsgBox("To close the preview please use the 'Close' button on the taskbar");
				return AC_CANNOT_CLOSE;
			}
			else {
				return AC_CAN_CLOSE;
			}
			break;
		default:
			// return CNxView::WindowProc(message, wParam, lParam);
			break;
		}
	} NxCatchAll("Error in CNxTabView::WindowProc");
	// (a.walling 2007-08-31 12:00) - PLID 27265 - SchedulerView was sending the message
	// here during an exception, which could throw the same one twice, and in some case
	// actually managed to get a huge loop of exceptions.
	
	return CNxView::WindowProc(message, wParam, lParam);
}

CString CNxTabView::GetTabLabel(short tab)
{
	ASSERT(m_tab != NULL);
	if (m_tab != NULL) {
		return (LPCTSTR)(m_tab->GetLabel(tab));
	} else {
		return _T("");
	}
}

//DRT 11/30/2007 - PLID 28252 - There is no reason we shouldn't just send it to 
//	the active sheet.  This makes it easier for others down the road.  If the sheet
//	doesn't want it, it can ignore it.  Thus, I've moved the barcode handling code
//	from each individual view, and moved it into the parent class.
LRESULT CNxTabView::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	if(m_pActiveSheet) {
		return m_pActiveSheet->SendMessage(WM_BARCODE_SCAN, wParam, lParam);
	}

	return 0;
}

// (z.manning 2010-07-19 16:00) - PLID 39222
void CNxTabView::OnUpdateViewUI(CCmdUI *pCmdUI)
{
	try
	{
		pCmdUI->Enable(TRUE);
		
	}NxCatchAll(__FUNCTION__);
}
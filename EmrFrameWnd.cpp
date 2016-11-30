// EmrFrameWnd.cpp : implementation file
//

#include "stdafx.h"
#include "EmrUtils.h"
#include "Practice.h"
#include "EmrIcons.h"
#include "EMRTopic.h"
#include "EmrFrameWnd.h"
#include "EmrChildWnd.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "foreach.h"
#include "EmrRc2.h"
// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI
#include <NxAdvancedUILib/NxRibbonControls.h>
#include <NxAdvancedUILib/NxRibbonCheckList.h>
#include <NxAdvancedUILib/NxRibbonListButton.h>
#include <NxSystemUtilitiesLib/dynamic_ptr.h>
#include <afxpriv.h>
#include <afxribbonbar.h>
#include <set>
#include "WindowUtils.h"
#include <NxPracticeSharedLib/PracDataEmr.h>
#include <NxPracticeSharedLib/PracData.h>
#include "EMR.h"
#include "EMN.h"

// (a.walling 2011-10-20 21:22) - PLID 46076 - Facelift - EMR frame window

// (a.walling 2012-02-28 14:53) - PLID 48451 - moved patient-related logic here to CEmrPatientFrameWnd

// (a.walling 2012-02-29 06:42) - PLID 46644 - TreePane sets m_bIsTemplate in constructor; moved some construction logic to derived classes

// (a.walling 2012-04-17 16:56) - PLID 49750 - Use CNxMDIFrameWndEx base class

// (a.walling 2012-05-22 09:59) - PLID 50556 - Prevent unnecessary redraws of ribbon combo boxes by always using CNxRibbonAutoComboBox instead of CMFCRibbonComboBox

// (a.walling 2014-04-24 12:00) - VS2013 - update cached multi_index structs to use ->get<>(), fixed deprecated mfc win32 wrappers

// CEmrFrameWnd

IMPLEMENT_DYNAMIC(CEmrFrameWnd, CNxMDIFrameWndEx)

// (j.dinatale 2012-09-18 15:13) - PLID 52702 - cache the EMR Save preference
// (j.jones 2012-09-27 09:23) - PLID 36220 - cache the drug interaction preference
CEmrFrameWnd::CEmrFrameWnd(bool bIsTemplate)
	: m_bIsTemplate(bIsTemplate)
	, m_bEditMode(bIsTemplate)
	, m_contextCategories(EmrContextCategory::None)
	, m_emrTreePane(bIsTemplate)
	, m_pLastActiveTopic(NULL)
	, m_pLastActiveEMN(NULL)
	, m_nEMRSavePref(1)
{
	m_bCanConvertControlBarToMDIChild = TRUE;

	// (a.walling 2012-11-09 09:05) - PLID 53672 - bPreventChildFrameActivation will clear all posted WM_MDIACTIVATE messages for the m_wndClientArea upon LoadMDIState completion
	// (a.walling 2013-02-13 09:41) - PLID 54772 - more advanced handling of loading in CNxMDIFrameWndEx makes that unnecessary now
	EnableMDIState(true);

	// (a.walling 2012-06-28 17:22) - PLID 48990 - Invalidate cached procedures and emrstatuslist records since we have no table checkers for them
	using namespace PracData;
	InvalidateCached<ProcedureT>();
	InvalidateCached<EmrStatusListT>();
}

CEmrFrameWnd::~CEmrFrameWnd()
{
}

void CEmrFrameWnd::PostNcDestroy()
{
	CNxMDIFrameWndEx::PostNcDestroy();
}

void CEmrFrameWnd::Activate(CWnd* pWnd)
{	
	// (a.walling 2012-07-03 10:56) - PLID 51284 - Fixing activation ambiguities
	//if (!bForceSwitchToTopicView && IsPreviewPaneActive()) {
	//	if (CEmrTopicView* pTopicView = dynamic_cast<CEmrTopicView*>(pWnd)) {
	//		return;
	//	}
	//}

	CFrameWnd* pFrameWnd = pWnd->GetParentFrame();
	CMDIFrameWnd* pMDIFrameWnd = dynamic_cast<CMDIFrameWnd*>(pFrameWnd);

	if (pMDIFrameWnd) {
		MDIActivate(pMDIFrameWnd);
		m_wndClientArea.SetActiveTab(pWnd->GetSafeHwnd());
	} else if (pFrameWnd) {
		MDIActivate(pFrameWnd);
	} else {
		ASSERT(FALSE);
		pWnd->BringWindowToTop();
	}
}

LRESULT CEmrFrameWnd::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	// Note: wParam == bDisableIfNoHandler

	try {

		LRESULT lr = CNxMDIFrameWndEx::OnIdleUpdateCmdUI(wParam, lParam);

		UpdateContextCategories();
		RefreshContextCategories();
		
		if (m_pEmrTopicViewDocTemplate) {
			m_pEmrTopicViewDocTemplate->OnIdle();
		}

		if (IsTemplate()) {
			OnUpdateFrameTitle(FALSE);
		}
	} NxCatchAllIgnore();

	return CNxMDIFrameWndEx::OnIdleUpdateCmdUI(wParam, lParam);
}

void CEmrFrameWnd::RefreshContextCategories()
{
	UINT visibleCategories = 0;

	for (int i = 0; i < m_wndRibbonBar.GetCategoryCount(); ++i) {
		CMFCRibbonCategory* pCategory = m_wndRibbonBar.GetCategory(i);
		UINT category = pCategory->GetContextID();
		if (category && pCategory->IsVisible()) {
			//TRACE(__FUNCTION__ " - Visible category %li\n", category);
			visibleCategories |= category;
		}
	}

	if (m_contextCategories == visibleCategories) {
		return;
	}
		
	CMFCRibbonCategory* pActiveCategory;
	if (m_wndRibbonBar.GetHideFlags() & (AFX_RIBBONBAR_HIDE_ELEMENTS | AFX_RIBBONBAR_HIDE_ALL)) {
		pActiveCategory = NULL;
	} else {
		pActiveCategory = m_wndRibbonBar.GetActiveCategory();
	}

	m_wndRibbonBar.HideAllContextCategories();

	UINT category = EmrContextCategory::Begin;

	do {
		if (m_contextCategories & category) {
			//TRACE(__FUNCTION__ " - Showing category %li\n", category);
			m_wndRibbonBar.ShowContextCategories(category, TRUE);
		}

		category <<= 1;

	} while (category < EmrContextCategory::End);

	if (pActiveCategory && pActiveCategory->IsVisible()) {
		m_wndRibbonBar.SetActiveCategory(pActiveCategory);
	}

	// (a.walling 2014-09-19 08:20) - PLID 63485 - Just RecalcLayout, not ForceRecalcLayout. The latter also reloads all fonts and etc.
	m_wndRibbonBar.RecalcLayout();
	m_wndRibbonBar.RedrawWindow();
	SendMessage(WM_NCPAINT, 0, 0);
}

BEGIN_MESSAGE_MAP(CEmrFrameWnd, CNxMDIFrameWndEx)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, &CEmrFrameWnd::OnIdleUpdateCmdUI)
	ON_REGISTERED_MESSAGE(AFX_WM_ON_GET_TAB_TOOLTIP, &CEmrFrameWnd::OnGetTabToolTip)
	// (a.walling 2012-03-20 09:24) - PLID 48990 - Called before a drop down or menu is displayed
	ON_REGISTERED_MESSAGE(AFX_WM_ON_BEFORE_SHOW_RIBBON_ITEM_MENU, &CEmrFrameWnd::OnBeforeShowRibbonItemMenu)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_ACTIVATE()
	ON_WM_ACTIVATEAPP()
	ON_WM_NCACTIVATE()
	ON_WM_GETMINMAXINFO()
	//ON_WM_APPCOMMAND()  ON_WM_APPCOMMAND() is broken; CWnd::OnAppCommand uses a BOOL return value, despite AfxSig_APPCOMMAND using void; regardless we want to be able to return a value anyway
	ON_MESSAGE(WM_APPCOMMAND, &CEmrFrameWnd::OnAppCommand)

	////
	/// UI state

	// (a.walling 2012-07-16 08:56) - PLID 51547 - Reset layout command
	ON_UPDATE_COMMAND_UI(ID_EMR_RESET_LAYOUT, &CEmrFrameWnd::OnUpdate_AlwaysEnable)

	ON_UPDATE_COMMAND_UI(ID_EMR_SAVE, &CEmrFrameWnd::OnUpdateSave)
	ON_UPDATE_COMMAND_UI(ID_EMR_SAVE_ALL, &CEmrFrameWnd::OnUpdateSaveAll)
	ON_UPDATE_COMMAND_UI(ID_EMR_SAVE_AND_CLOSE, &CEmrFrameWnd::OnUpdateCloseEMR) // Should always be enabled, even if nothing can save, as long as we can close
	ON_UPDATE_COMMAND_UI(ID_EMR_CLOSE, &CEmrFrameWnd::OnUpdateCloseEMR)

	ON_UPDATE_COMMAND_UI(ID_MDI_TABBED_DOCUMENT, &CEmrFrameWnd::OnUpdateMdiTabbedDocument)
	ON_UPDATE_COMMAND_UI(ID_EMR_EDIT_MODE, &CEmrFrameWnd::OnUpdateEditMode)
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_FULLSCREEN, &CEmrFrameWnd::OnUpdateViewFullScreen)
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_MESSAGE_BAR, &CEmrFrameWnd::OnUpdateViewMessageBar) // (a.walling 2011-12-12 09:15) - PLID 46645 - Message bar

	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_TREE_PANE, &CEmrFrameWnd::OnUpdateViewTreePane)
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_PREVIEW_PANE, &CEmrFrameWnd::OnUpdateViewPreviewPane)
	ON_UPDATE_COMMAND_UI(ID_EMR_VIEW_CLASSIC_BUTTON_PANE, &CEmrFrameWnd::OnUpdateViewClassicButtonPane) //(e.lally 2012-02-16) PLID 48065

	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_WRITE_ACCESS, &CEmrFrameWnd::OnUpdateWriteAccess)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_CHART, &CEmrFrameWnd::OnUpdateChart)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_CATEGORY, &CEmrFrameWnd::OnUpdateCategory)

	// (a.walling 2012-06-28 17:13) - PLID 51277 - More Info button
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_MORE_INFO, &CEmrFrameWnd::OnUpdateMoreInfo)
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_DESCRIPTION, &CEmrFrameWnd::OnUpdateDescription)

	//TES 2/13/2014 - PLID 60749 - Codes button
	ON_UPDATE_COMMAND_UI(ID_EMR_EMN_CODES, &CEmrFrameWnd::OnUpdateCodes)
	
	// (r.gonet 08/03/2012) - PLID 51027 - Wound Care Auto Coding button
	ON_UPDATE_COMMAND_UI(ID_EMR_WOUND_CARE_AUTO_CODING, &CEmrFrameWnd::OnUpdateWoundCareAutoCoding)

	//(e.lally 2012-02-29) PLID 48065 - This is common to both patients and templates so I moved it back to its home
	ON_UPDATE_COMMAND_UI(ID_EMR_DEBUG, &CEmrFrameWnd::OnUpdateEmrDebug)

	// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
	ON_UPDATE_COMMAND_UI(ID_EMR_PREVIEW_PANE_PRINT, &CEmrFrameWnd::OnUpdatePreviewPanePrint)
	ON_UPDATE_COMMAND_UI(ID_EMR_PREVIEW_PANE_PRINT_PREVIEW, &CEmrFrameWnd::OnUpdatePreviewPanePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_EMR_PREVIEW_PANE_PRINT_MULTIPLE, &CEmrFrameWnd::OnUpdatePreviewPanePrintMultiple)
	ON_UPDATE_COMMAND_UI(ID_EMR_PREVIEW_PANE_CONFIGURE, &CEmrFrameWnd::OnUpdatePreviewPaneConfigure)

	ON_UPDATE_COMMAND_UI(ID_EMR_PREVIEW_PANE_AUTO_SCROLL, &CEmrFrameWnd::OnUpdatePreviewPaneAutoScroll)

	//TES 1/17/2014 - PLID 60397 - Option to hide the EMN title on the preview pane, per template
	ON_UPDATE_COMMAND_UI(ID_EMR_TEMPLATE_HIDE_TITLE, &CEmrFrameWnd::OnUpdateTemplateHideTitle)

	////
	/// UI Commands

	// (a.walling 2012-07-16 08:56) - PLID 51547 - Reset layout command
	ON_COMMAND(ID_EMR_RESET_LAYOUT, &CEmrFrameWnd::OnResetLayout)

	ON_COMMAND(ID_EMR_SAVE, &CEmrFrameWnd::OnSave)
	ON_COMMAND(ID_EMR_SAVE_ALL, &CEmrFrameWnd::OnSaveAll)
	ON_COMMAND(ID_EMR_SAVE_AND_CLOSE, &CEmrFrameWnd::OnSaveAndClose)
	ON_COMMAND(ID_EMR_CLOSE, &CEmrFrameWnd::OnCloseEMR)

	ON_COMMAND(ID_EMR_EDIT_MODE, &CEmrFrameWnd::OnEditMode)
	
	ON_COMMAND(ID_MDI_TABBED_DOCUMENT, &CEmrFrameWnd::OnMdiTabbedDocument)
	ON_COMMAND(ID_MDI_MOVE_TO_NEXT_GROUP, &CEmrFrameWnd::OnMdiMoveToNextGroup)
	ON_COMMAND(ID_MDI_MOVE_TO_PREV_GROUP, &CEmrFrameWnd::OnMdiMoveToPrevGroup)
	ON_COMMAND(ID_MDI_NEW_HORZ_TAB_GROUP, &CEmrFrameWnd::OnMdiNewHorzTabGroup)
	ON_COMMAND(ID_MDI_NEW_VERT_GROUP, &CEmrFrameWnd::OnMdiNewVertGroup)

	ON_COMMAND(ID_EMR_VIEW_FULLSCREEN, &CEmrFrameWnd::OnViewFullScreen)
	ON_COMMAND(ID_EMR_VIEW_MESSAGE_BAR, &CEmrFrameWnd::OnViewMessageBar) // (a.walling 2011-12-12 09:15) - PLID 46645 - Message bar

	ON_COMMAND(ID_EMR_VIEW_TREE_PANE, &CEmrFrameWnd::OnViewTreePane)
	ON_COMMAND(ID_EMR_VIEW_PREVIEW_PANE, &CEmrFrameWnd::OnViewPreviewPane)
	ON_COMMAND(ID_EMR_VIEW_CLASSIC_BUTTON_PANE, &CEmrFrameWnd::OnViewClassicButtonPane) //(e.lally 2012-02-16) PLID 48065

	ON_COMMAND(ID_EMR_EMN_WRITE_ACCESS, &CEmrFrameWnd::OnWriteAccess)
	
	ON_COMMAND(ID_EMR_EMN_CHART, &CEmrFrameWnd::OnChart)
	ON_COMMAND(ID_EMR_EMN_CATEGORY, &CEmrFrameWnd::OnCategory)
	// (a.walling 2012-06-28 17:13) - PLID 51277 - More Info button
	ON_COMMAND(ID_EMR_EMN_MORE_INFO, &CEmrFrameWnd::OnMoreInfo)
	ON_COMMAND(ID_EMR_EMN_DESCRIPTION, &CEmrFrameWnd::OnDescription)

	//TES 2/13/2014 - PLID 60749 - Codes button
	ON_COMMAND(ID_EMR_EMN_CODES, &CEmrFrameWnd::OnCodes)

	// (r.gonet 08/03/2012) - PLID 51027 - Wound Care Auto Coding button
	ON_COMMAND(ID_EMR_WOUND_CARE_AUTO_CODING, &CEmrFrameWnd::OnWoundCareAutoCoding)

	//(e.lally 2012-02-29) PLID 48065 - This is common to both patients and templates so I moved it back to its home
	ON_COMMAND(ID_EMR_DEBUG, &CEmrFrameWnd::OnEmrDebug)

	// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
	ON_COMMAND(ID_EMR_PREVIEW_PANE_PRINT, &CEmrFrameWnd::OnPreviewPanePrint)
	ON_COMMAND(ID_EMR_PREVIEW_PANE_PRINT_PREVIEW, &CEmrFrameWnd::OnPreviewPanePrintPreview)
	ON_COMMAND(ID_EMR_PREVIEW_PANE_PRINT_MULTIPLE, &CEmrFrameWnd::OnPreviewPanePrintMultiple)
	ON_COMMAND(ID_EMR_PREVIEW_PANE_CONFIGURE, &CEmrFrameWnd::OnPreviewPaneConfigure)

	ON_COMMAND(ID_EMR_PREVIEW_PANE_AUTO_SCROLL, &CEmrFrameWnd::OnPreviewPaneAutoScroll)

	//TES 1/17/2014 - PLID 60397 - Option to hide the EMN title on the preview pane, per template
	ON_COMMAND(ID_EMR_TEMPLATE_HIDE_TITLE, &CEmrFrameWnd::OnTemplateHideTitle)

END_MESSAGE_MAP()

// CEmrFrameWnd Command UI handlers

void CEmrFrameWnd::OnUpdateMdiTabbedDocument(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck();
}

void CEmrFrameWnd::OnUpdateEditMode(CCmdUI* pCmdUI)
{
	//(e.lally 2012-04-04) PLID 48065 - Make sure we have an active and writable EMN
	//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);
	pCmdUI->SetCheck(m_bEditMode ? BST_CHECKED : BST_UNCHECKED);
}

// CEmrFrameWnd Command handlers

void CEmrFrameWnd::OnEditMode()
{
	try {
		SetEditMode(!m_bEditMode);
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::SetEditMode(bool bEditMode)
{
	m_bEditMode = bEditMode;

	CEmrTopicView* pTopicView = GetActiveEmrTopicView();
	if (pTopicView) {
		pTopicView->SetAllowEdit(m_bEditMode);
	}

	// (j.jones 2007-07-06 10:20) - PLID 25457 - added SetEditMode tothe preview control
	if (GetEmrPreviewCtrl()) {
		GetEmrPreviewCtrl()->SetAllowEdit(m_bEditMode);
	}
}

// CEmrFrameWnd message handlers

int CEmrFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	try {
		if (CNxMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
			return -1;

		ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	} NxCatchAll(__FUNCTION__);

	return 0;
}

void CEmrFrameWnd::OnClose()
{	
	try {
		if (::GetFocus()) {
			::SetFocus(NULL); // force kill focus
		}
		__super::OnClose();
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnDestroy()
{
	__super::OnDestroy();
}

void CEmrFrameWnd::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	try {
		switch (nState)
		{
			case WA_ACTIVE:
				ASSERT(TRUE);
				break;
			case WA_CLICKACTIVE:
				ASSERT(TRUE);
				break;
			case WA_INACTIVE:
				ASSERT(TRUE);
				break;
			default:
				ASSERT(TRUE);
				break;
		}

		__super::OnActivate(nState, pWndOther, bMinimized);
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	try {
		if (!bActive) {
			ASSERT(TRUE);
		} else {
			ASSERT(TRUE);
		}

		__super::OnActivateApp(bActive, dwThreadID);
	} NxCatchAll(__FUNCTION__);
}

BOOL CEmrFrameWnd::OnNcActivate(BOOL bActive)
{
	return __super::OnNcActivate(bActive);
}

void CEmrFrameWnd::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	try {
		__super::OnGetMinMaxInfo(lpMMI);

		if (!lpMMI) return;

		lpMMI->ptMinTrackSize = CPoint(320, 320);
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::InitializeEmrFrame()
{
	CMDITabInfo mdiTabParams;
	
	mdiTabParams.m_tabLocation = CMFCTabCtrl::LOCATION_TOP;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_SCROLLED; // other styles available...
	mdiTabParams.m_bTabIcons = FALSE;    // set to TRUE to enable document icons on MDI tabs
	mdiTabParams.m_bTabCloseButton = FALSE;
	mdiTabParams.m_bTabCustomTooltips = TRUE; // sends registered message AFX_WM_ON_GET_TAB_TOOLTIP is true
	mdiTabParams.m_bAutoColor = FALSE;    // set to FALSE to disable auto-coloring of MDI tabs
	mdiTabParams.m_bDocumentMenu = TRUE; // enable the document menu at the right edge of the tab area
	mdiTabParams.m_bEnableTabSwap = TRUE;
	mdiTabParams.m_bFlatFrame = TRUE;
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // set to FALSE to place close button at right of tab area

	EnableMDITabbedGroups(TRUE, mdiTabParams);

	{

		m_wndRibbonBar.Create(this);

		// (a.walling 2011-12-09 10:16) - PLID 46642 - Artwork
		m_ribbonToolImages.Load(IDB_EMRFRAME_TOOLIMAGES_SMALL);
		m_ribbonToolImages.SetImageSize(CSize(16, 16));

		m_mainButton.SetImage(IDR_EMRFRAME);
		// (a.walling 2014-04-04 16:36) - PLID 61670 - Set the main button's sprite size
		m_mainButton.SetImageSize(CSize(38,38));
		m_mainButton.SetText(_T("\nf"));
		m_mainButton.SetToolTipText("NexEMR");

		m_wndRibbonBar.SetApplicationButton(&m_mainButton, CSize (45, 45));

		InitializeRibbon();
	}
	{
		m_wndStatusBar.Create(this);

		InitializeStatusBar();

		// (a.walling 2014-09-19 08:20) - PLID 63485 - Just RecalcLayout, not ForceRecalcLayout. The latter also reloads all fonts and etc.
		m_wndStatusBar.RecalcLayout();
		m_wndStatusBar.Invalidate();
	}
	InitializeCaptionBar(); // (a.walling 2011-12-12 09:15) - PLID 46645 - Message bar
	InitializePanes();

	// (a.walling 2012-03-12 13:31) - PLID 48824 - NxRibbonBar now handles all this in LoadState automatically
	/*
	if (m_wndRibbonBar.GetHideFlags() & AFX_RIBBONBAR_HIDE_ELEMENTS) {
		// already minimized
	} else {
		m_wndRibbonBar.ToggleMimimizeState();
	}
	*/

	// (j.dinatale 2012-09-18 14:21) - PLID 52702 - store the EMRSavePref so we can refer to it later
	m_nEMRSavePref = GetRemotePropertyInt("EMRSavePref", 1, 0, GetCurrentUserName(), true);

	EnableFullScreenMode(ID_EMR_VIEW_FULLSCREEN);
}

LRESULT CEmrFrameWnd::OnGetTabToolTip(WPARAM wParam, LPARAM lParam)
{
	try {
		CMFCTabToolTipInfo* pInfo = reinterpret_cast<CMFCTabToolTipInfo*>(lParam);
		ASSERT(pInfo);

		if (pInfo) {
			if (!pInfo->m_pTabWnd->IsMDITab())
			{
				return 0;
			}

			CFrameWnd* pTabWndFrame = dynamic_cast<CFrameWnd*>(pInfo->m_pTabWnd->GetTabWndNoWrapper(pInfo->m_nTabIndex));
			if (!pTabWndFrame) return 0;
				
			CEmrTopicView* pTopicView = dynamic_cast<CEmrTopicView*>(pTabWndFrame->GetActiveView());
			if (!pTopicView) return 0;

			CEMN* pEMN = pTopicView->GetAttachedEMN();
			if (!pEMN) return 0;
					
			CString strEMNDescription = pEMN->GetDescription();
			if (strEMNDescription.IsEmpty()) {
				strEMNDescription = "EMN";
			}
			strEMNDescription.Replace("\r", "");
			strEMNDescription.Replace("\n", " ");

			COleDateTimeSpan dtsDate = pEMN->GetEMNDate() - COleDateTime::GetCurrentTime();

			CMFCToolTipCtrl* pToolTipCtrl = dynamic_cast<CMFCToolTipCtrl*>(&pInfo->m_pTabWnd->GetToolTipCtrl());

			if (pToolTipCtrl) {
				pToolTipCtrl->SetDescription(strEMNDescription);
				pInfo->m_strText.Format("%s - %s", FormatDateTimeForInterface(pEMN->GetEMNDate()), DescribeRelativeDateTimeSpan(dtsDate));
			} else {
				pInfo->m_strText.Format("%s\n%s (%s)", strEMNDescription, FormatDateTimeForInterface(pEMN->GetEMNDate()), DescribeRelativeDateTimeSpan(dtsDate));
			}
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (a.walling 2012-03-20 09:24) - PLID 48990 - Called before a drop down or menu is displayed; ensure it has all necessary data available via SynchronizeDelayedRibbonSubitems
LRESULT CEmrFrameWnd::OnBeforeShowRibbonItemMenu(WPARAM wParam, LPARAM lParam)
{
	try {
		CMFCRibbonBaseElement* pElement = reinterpret_cast<CMFCRibbonBaseElement*>(lParam);

		SynchronizeDelayedRibbonSubitems(pElement);
		
		if (pElement->GetID()) {
			if (CMFCRibbonBar* pRibbonBar = pElement->GetTopLevelRibbonBar()) {
				CArray<CMFCRibbonBaseElement*, CMFCRibbonBaseElement*> arItems;
				pRibbonBar->GetElementsByID(pElement->GetID(), arItems);

				foreach(CMFCRibbonBaseElement* pOtherElement, arItems) {
					if (pOtherElement == pElement) {
						continue;
					}

					SynchronizeDelayedRibbonSubitems(pOtherElement);
				}
			}
		}
	} NxCatchAll(__FUNCTION__);

	return Default();
}

// (a.walling 2012-03-20 09:24) - PLID 48990 - Ensure the ribbon item has its menu fully populated with all data
void CEmrFrameWnd::SynchronizeDelayedRibbonSubitems(CMFCRibbonBaseElement* pElement)
{	
	if (!pElement) return;
	
	typedef void (CEmrFrameWnd::*SyncFn)(CMFCRibbonBaseElement*);

	struct SyncEntry {
		UINT nID;
		SyncFn fn;
	};

	// (a.walling 2012-04-06 13:19) - PLID 48990 - Changed this a bit to have a list of entries rather than a switch statement which can quickly get out of control
	static SyncEntry syncEntries[] = {
		{ID_EMR_EMN_CHART,					&CEmrFrameWnd::SynchronizeDelayedChartSubitems},		// (a.walling 2012-03-26 12:49) - PLID 49205 - Chart, category
		{ID_EMR_EMN_CATEGORY,				&CEmrFrameWnd::SynchronizeDelayedCategorySubitems},		// (a.walling 2012-03-26 12:49) - PLID 49205 - Chart, category
	};

	try {

		UINT nID = pElement->GetID();

		foreach(SyncEntry& syncEntry, syncEntries) {
			if (nID == syncEntry.nID) {
				EnsureCachedData();
				(this->*(syncEntry.fn))(pElement);
				return;
			}
		}

		// (a.walling 2012-05-25 17:39) - PLID 50664 - Forward to view
		if (CEmrTopicView* pView = GetActiveEmrTopicView()) {
			pView->SynchronizeDelayedRibbonSubitems(pElement);
		}
	} NxCatchAll(__FUNCTION__);
}

BOOL CEmrFrameWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (nCode == CN_UPDATE_COMMAND_UI)
	{
		CCmdUI *pCmdUI = (CCmdUI *)pExtra;
	}

	return __super::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CEmrFrameWnd::OnShowMDITabContextMenu(CPoint point, DWORD dwAllowedItems, BOOL bDrop)
{
	try {
		if (!dwAllowedItems) {
			return FALSE;
		}

		CMenu menu;
		VERIFY(menu.LoadMenu(bDrop ? IDR_POPUP_DROP_MDITABS : IDR_POPUP_MDITABS));

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT(pPopup != NULL);

		if (pPopup)
		{
			if ((dwAllowedItems & AFX_MDI_CREATE_HORZ_GROUP) == 0)
			{
				pPopup->DeleteMenu(ID_MDI_NEW_HORZ_TAB_GROUP, MF_BYCOMMAND);
			}

			if ((dwAllowedItems & AFX_MDI_CREATE_VERT_GROUP) == 0)
			{
				pPopup->DeleteMenu(ID_MDI_NEW_VERT_GROUP, MF_BYCOMMAND);
			}

			if ((dwAllowedItems & AFX_MDI_CAN_MOVE_NEXT) == 0)
			{
				pPopup->DeleteMenu(ID_MDI_MOVE_TO_NEXT_GROUP, MF_BYCOMMAND);
			}

			if ((dwAllowedItems & AFX_MDI_CAN_MOVE_PREV) == 0)
			{
				pPopup->DeleteMenu(ID_MDI_MOVE_TO_PREV_GROUP, MF_BYCOMMAND);
			}

			if ((dwAllowedItems & AFX_MDI_CAN_BE_DOCKED) == 0)
			{
				pPopup->DeleteMenu(ID_MDI_TABBED_DOCUMENT, MF_BYCOMMAND);
			}

			CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;
			if (pPopupMenu)
			{
				pPopupMenu->SetAutoDestroy(FALSE);
				pPopupMenu->Create(this, point.x, point.y, pPopup->GetSafeHmenu());
			}
		}
	} NxCatchAll(__FUNCTION__);

	return TRUE;
}


void CEmrFrameWnd::Create()
{
	m_pEmrTopicViewDocTemplate.reset(new CEmrDocTemplate(IDR_EMRFRAME,
		RUNTIME_CLASS(CEmrDocument),
		RUNTIME_CLASS(CEmrChildWnd),
		this)
	);

	if (!LoadFrame(IDR_EMRFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL)) {
		ThrowNxException("Failed to create EMR frame");
	}

	// (a.walling 2012-04-18 15:09) - PLID 49772 - Ensure the frame is shown and activated
	// (a.walling 2012-04-18 18:14) - PLID 49772 - Now handled by CNxMDIFrameWndEx
	//ActivateFrame(SW_SHOW);
}

CEmrTopicView* CEmrFrameWnd::GetEmrTopicView(CEMN* pEMN, bool bAutoCreate)
{	
	// (a.walling 2013-02-13 09:30) - PLID 56125 - Topic and more info view creation handling now within the doctemplate
	return m_pEmrTopicViewDocTemplate->GetEmrTopicView(pEMN, bAutoCreate);
}

CEmrTopicView* CEmrFrameWnd::GetActiveEmrTopicView()
{
	if (!this) return NULL;

	CView* pActiveView = GetActiveView();

	CEmrTopicView* pActiveEmrTopicView = dynamic_cast<CEmrTopicView*>(pActiveView);

	if (!pActiveEmrTopicView) { // normal when tabbed
		CMDIChildWnd* pChild = dynamic_cast<CMDIChildWnd*>(GetActiveFrame());
		if (pChild) {
			pActiveEmrTopicView = dynamic_cast<CEmrTopicView*>(pChild->GetActiveView());
		}
	}

	return pActiveEmrTopicView;
}

bool CEmrFrameWnd::HasEmrTopicView()
{
	POSITION pos = m_pEmrTopicViewDocTemplate->GetFirstDocPosition();
	while (pos) {
		CEmrDocument* pDoc = polymorphic_downcast<CEmrDocument*>(m_pEmrTopicViewDocTemplate->GetNextDoc(pos));
		CEmrTopicView* pView = pDoc->FindView<CEmrTopicView>();

		if (pView && pView->GetAttachedEMN() && pView->IsWindowVisible()) {
			return true;
		}
	}

	return false;
}


// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
CEmrMoreInfoView* CEmrFrameWnd::GetEmrMoreInfoView(CEMN* pEMN, bool bAutoCreate)
{
	// (a.walling 2013-02-13 09:30) - PLID 56125 - Topic and more info view creation handling now within the doctemplate
	return m_pEmrTopicViewDocTemplate->GetEmrMoreInfoView(pEMN, bAutoCreate);
}

// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
CEMNMoreInfoDlg* CEmrFrameWnd::GetMoreInfoDlg(CEMN* pEMN, bool bAutoCreate)
{
	CEmrMoreInfoView* pMoreInfoView = GetEmrMoreInfoView(pEMN, bAutoCreate);

	if (!pMoreInfoView) return NULL;

	return pMoreInfoView->GetMoreInfoDlg();
}

// (a.walling 2011-11-11 11:11) - PLID 46638 - Handle EMN More Info in a separate tab as a view
void CEmrFrameWnd::ShowMoreInfo(CEMN* pEMN)
{
	ASSERT(pEMN);
	if (!pEMN) return;

	CEmrMoreInfoView* pEmrMoreInfoView = EnsureEmrMoreInfoView(pEMN);

	Activate(pEmrMoreInfoView);
}

//TES 2/12/2014 - PLID 60740 - Added for the new <Codes> topic
CEmrCodesView* CEmrFrameWnd::GetEmrCodesView(CEMN* pEMN, bool bAutoCreate)
{
	return m_pEmrTopicViewDocTemplate->GetEmrCodesView(pEMN, bAutoCreate);
}

//TES 2/12/2014 - PLID 60740 - Added for the new <Codes> topic
CEmrCodesDlg* CEmrFrameWnd::GetEmrCodesDlg(CEMN* pEMN, bool bAutoCreate)
{
	CEmrCodesView* pCodesView  = GetEmrCodesView(pEMN, bAutoCreate);

	if (!pCodesView ) return NULL;

	return pCodesView ->GetEmrCodesDlg();
}

//TES 2/12/2014 - PLID 60740 - Added for the new <Codes> topic
void CEmrFrameWnd::ShowCodesView(CEMN* pEMN)
{
	ASSERT(pEMN);
	if (!pEMN) return;

	CEmrCodesView* pEmrCodesView = EnsureEmrCodesView(pEMN);

	Activate(pEmrCodesView);
}

void CEmrFrameWnd::ActivateTopic(CEMRTopic* pTopic, bool bActivateTopicView)
{	
	ASSERT(pTopic);
	if (!pTopic) return;

	CEMN* pEMN = pTopic->GetParentEMN();
	ASSERT(pEMN);
	if (!pEMN) return;

	// (a.walling 2012-07-03 10:56) - PLID 51284 - Activate EMN
	ActivateEMN(pEMN, pTopic, bActivateTopicView);
}

void CEmrFrameWnd::ActivateEMNOnly(CEMN* pEMN)
{
	DoActivateEMN(pEMN, NULL, true);
}

void CEmrFrameWnd::ActivateEMN(CEMN* pEMN, CEMRTopic* pTopic, bool bActivateTopicView)
{
	ASSERT(pEMN);
	if (!pEMN) return;

	CEmrTopicView* pTopicView = GetEmrTopicView(pEMN, false);

	if (!pTopic && pTopicView) {
		pTopic = pTopicView->GetActiveTopic();
	}

	DoActivateEMN(pEMN, pTopic, bActivateTopicView); // mm yes, quite
}

void CEmrFrameWnd::DoActivateEMN(CEMN* pEMN, CEMRTopic* pTopic, bool bActivateTopicView)
{
	ASSERT(pEMN);
	if (!pEMN) return;

	CEMR* pEMR = pEMN->GetParentEMR();

	ASSERT(pEMR);
	if (!pEMR) return;

	CEmrTopicView* pTopicView = GetEmrTopicView(pEMN, false);

	// (a.walling 2012-10-04 12:41) - PLID 52878 - This is a hack at the moment, but for now just ensure the last active EMN and Topic pointers are valid

	if (pTopic) {
		pTopic = pEMR->VerifyPointer(pTopic);
		_ASSERTE(pTopic);
	}
	
	CEMN* pPreviouslyActiveEMN = pEMR->VerifyPointer(m_pLastActiveEMN);
	m_pLastActiveEMN = pEMN;

	CEMRTopic* pPreviouslyActiveTopic = pEMR->VerifyPointer(m_pLastActiveTopic);
	m_pLastActiveTopic = pTopic;

	if (GetEmrPreviewCtrl()) {
		if (!pEMN->IsPreviewCurrent() && !pEMN->IsLoading()) {
			pEMN->GenerateHTMLFile(TRUE, TRUE, FALSE);
		}		
		
		GetEmrPreviewCtrl()->SetEMN(pEMN);
	}

	pEMN->TryStartTrackingTime();

	if (bActivateTopicView) {
		pTopicView = EnsureEmrTopicView(pEMN);
		if (!IsPreviewPaneActive()) {
			if (!pTopic) {
				pTopicView->ScrollToPosition(CPoint(0,0));
				pTopicView->SetActiveTopicWnd(CEmrTopicWndPtr());
			}
			Activate(pTopicView);
		}
	}

	if (pTopic) {
		if (pTopicView) {
			pTopicView->ShowTopicOnActivate(pTopic);
		}
		GetEmrTreeWnd()->HighlightActiveTopic(pTopic);
	} else {
		if (pTopicView) {
			pTopicView->ScrollToPosition(CPoint(0,0));
			pTopicView->SetActiveTopicWnd(CEmrTopicWndPtr());
		}
		// (a.walling 2012-07-03 10:56) - PLID 51284 - Highlight active EMN
		GetEmrTreeWnd()->HighlightActiveEMN(pEMN);
	}
	GetEmrTreeWnd()->TryAutoWriteAccess(pEMN, NULL);

	// (a.walling 2012-07-03 10:56) - PLID 51284 - Call virtual event
	// (j.dinatale 2012-09-18 16:56) - PLID 52702 - pass in to and from pointers
	if (pPreviouslyActiveEMN != pEMN) {
		OnActiveEMNChanged(pPreviouslyActiveEMN, pEMN);
	}

	// (a.walling 2012-07-03 10:56) - PLID 51284 - Call virtual event
	// (j.dinatale 2012-09-18 16:56) - PLID 52702 - pass in to and from pointers
	if (pPreviouslyActiveTopic != pTopic) {
		OnActiveTopicChanged(pPreviouslyActiveTopic, pTopic);
	}
}

// (a.walling 2013-01-17 12:36) - PLID 54666 - ShowEMN will behave like EmrTreeWnd's ShowEMN, such as when first creating an EMN
void CEmrFrameWnd::ShowEMN(CEMN* pEMN)
{
	if (CEmrTreeWnd* pTreeWnd = GetEmrTreeWnd()) {
		pTreeWnd->ShowEMN(pEMN);
	}
}

bool CEmrFrameWnd::IsPreviewPaneActive()
{
	if (!m_emrPreviewPane.GetSafeHwnd()) {
		return false;
	}

	if (CMDIChildWndEx* pChildWndEx = dynamic_cast<CMDIChildWndEx*>(MDIGetActive())) {
		if (CEmrPreviewPane* pDockedPreviewPane = dynamic_cast<CEmrPreviewPane*>(pChildWndEx->GetTabbedPane())) {
			return true;
		}
	}

	// (a.walling 2012-07-03 10:56) - PLID 51284 - Check for active focus
	CWnd* pFocus = GetFocus();
	if (pFocus && (&m_emrPreviewPane == pFocus || m_emrPreviewPane.IsChild(pFocus)) ) {
		return true;
	}

	return false;
}

CEMRPreviewCtrlDlg* CEmrFrameWnd::GetEmrPreviewCtrl()
{
	return m_emrPreviewPane.SafeGetEmrPreviewCtrl();
}

CEmrTreeWnd* CEmrFrameWnd::GetEmrTreeWnd()
{
	return m_emrTreePane.GetEmrTreeWnd();
}

BOOL CEmrFrameWnd::PreTranslateMessage(MSG* pMsg) 
{	
	return CNxMDIFrameWndEx::PreTranslateMessage(pMsg);
}

// (a.walling 2012-03-12 17:21) - PLID 46076 - Apply our icon within PreCreateWindow since IDR_EMRFRAME does not have an icon
BOOL CEmrFrameWnd::PreCreateWindow( CREATESTRUCT& cs )
{
	if (!__super::PreCreateWindow(cs)) {
		return FALSE;
	}

	HICON hUseThisIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	WNDCLASS wndcls;
	if (cs.lpszClass != NULL &&
		GetClassInfo(AfxGetInstanceHandle(), cs.lpszClass, &wndcls) &&
		wndcls.hIcon != hUseThisIcon)
	{
		// register a very similar WNDCLASS
		cs.lpszClass = AfxRegisterWndClass(wndcls.style,
			wndcls.hCursor, wndcls.hbrBackground, hUseThisIcon);
	}

	return TRUE;
}

void CEmrFrameWnd::OnMdiTabbedDocument()
{
	try {
		CMDIChildWndEx* pMDIChild = DYNAMIC_DOWNCAST(CMDIChildWndEx, MDIGetActive());
		if (pMDIChild == NULL)
		{
			ASSERT(FALSE);
			return;
		}

		TabbedDocumentToControlBar(pMDIChild);
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnMdiMoveToNextGroup()
{
	try {
		MDITabMoveToNextGroup();
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnMdiMoveToPrevGroup()
{
	try {
		MDITabMoveToNextGroup(FALSE);
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnMdiNewHorzTabGroup()
{
	try {
		MDITabNewGroup(FALSE);
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnMdiNewVertGroup()
{
	try {
		MDITabNewGroup();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
std::auto_ptr<CMFCRibbonButtonsGroup> CEmrFrameWnd::CreateNavigationButtonsGroup()
{
	std::auto_ptr<CMFCRibbonButtonsGroup> pTopicNavGroup(new CMFCRibbonButtonsGroup);

	CMFCToolBarImages statusBarImages;
	statusBarImages.SetImageSize(CSize(16, 16));

	if (statusBarImages.Load(IDB_EMRFRAME_STATUSBARIMAGES_SMALL))
	{
		pTopicNavGroup->SetImages(&statusBarImages, NULL, NULL);
	}

	// (a.walling 2011-12-18 14:15) - PLID 46646 - Topic navigation arrows
	// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
	pTopicNavGroup->AddButton(new CNxRibbonButton(ID_EMR_FIRST_TOPIC, "", EmrIcons::StatusBar::First));
	pTopicNavGroup->AddButton(new CNxRibbonButton(ID_EMR_PREVIOUS_TOPIC, "", EmrIcons::StatusBar::Previous));
	// (a.walling 2012-05-25 17:39) - PLID 50664 - Topic list
	pTopicNavGroup->AddButton(new CNxRibbonListButton(ID_EMR_TOPIC_LIST, "Topics", EmrIcons::StatusBar::Topics, EmrIcons::StatusBar::None));
	pTopicNavGroup->AddButton(new CNxRibbonButton(ID_EMR_NEXT_TOPIC, "", EmrIcons::StatusBar::Next));
	pTopicNavGroup->AddButton(new CNxRibbonButton(ID_EMR_LAST_TOPIC, "", EmrIcons::StatusBar::Last));

	return pTopicNavGroup;
}

void CEmrFrameWnd::OnUpdateViewFullScreen(CCmdUI* pCmdUI)
{
	//pCmdUI->Enable(TRUE);
}

void CEmrFrameWnd::OnViewFullScreen()
{
	ASSERT(FALSE);
	return;

	////(e.lally 2012-03-15) PLID 48918 - If we are about to go into Full Screen mode, make sure we have an active topic view
	//if(!IsFullScreen()){
	//	CEmrTopicView* pTopicView = GetActiveEmrTopicView();
	//	if(pTopicView == NULL && GetActiveEMN() != NULL){
	//		GetEmrTopicView(GetActiveEMN(), true);
	//	}
	//}

	////Toggles between full screen and regular modes
	//ShowFullScreen();

	//if (IsFullScreen()) {
	//	// custom layout here
	//} else {
	//	CRect rectWindow;
	//	GetWindowRect(&rectWindow);

	//	MoveWindow(rectWindow.left, rectWindow.top, rectWindow.Width() + 1, rectWindow.Height(), FALSE);
	//	MoveWindow(rectWindow.left, rectWindow.top, rectWindow.Width(), rectWindow.Height(), FALSE);
	//	Invalidate();
	//}
}

// (a.walling 2011-12-12 09:15) - PLID 46645 - Message bar
void CEmrFrameWnd::OnUpdateViewMessageBar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndMessageBar.IsVisible());
}

// (a.walling 2011-12-12 09:15) - PLID 46645 - Message bar
void CEmrFrameWnd::OnViewMessageBar()
{
	try {
		ShowPane(&m_wndMessageBar, !(m_wndMessageBar.IsVisible()), FALSE, FALSE);
		RecalcLayout();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-12-12 09:15) - PLID 46645 - Message bar
void CEmrFrameWnd::InitializeCaptionBar()
{
	m_wndMessageBar.Create(WS_CHILD | WS_CLIPSIBLINGS, this, ID_EMR_VIEW_MESSAGE_BAR, -1, TRUE);
}

// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history
bool CEmrFrameWnd::CanAccess_EMRAuditHistory(bool bSilent)
{
	// (d.thompson 2009-06-03) - PLID 34468 - This used to require access to the Audit trail tab
	//	in administrator.  Now it requires permission to the sub-permissions for "NexEMR" AND 
	//	"Miscellaneous" (templates and other go here).  It does NOT require that you have the audit
	//	tab above those available.  This is a method to let users see EMR histories without seeing 
	//	other histories.
	//if(CheckCurrentUserPermissions(bioAdminAuditTrail, sptRead))
	return !!CheckCurrentUserPermissions(bioAuditTrailEMR, sptRead, FALSE, 0, TRUE, TRUE) && CheckCurrentUserPermissions(bioAuditTrailMisc, sptRead, FALSE, 0, bSilent ? TRUE : FALSE, bSilent ? TRUE : FALSE);
}

// (a.walling 2012-03-23 16:04) - PLID 49190 - Confidential info
// (j.armen 2012-07-02 11:25) - PLID 49831 - Check Patient EMR Permission
bool CEmrFrameWnd::CanAccess_ConfidentialInfo(bool bSilent)
{
	return !!CheckCurrentUserPermissions(bioPatientEMR, sptDynamic2, FALSE, 0, bSilent ? TRUE : FALSE, bSilent ? TRUE : FALSE);
}

void CEmrFrameWnd::OnUpdate_AlwaysEnable(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// (j.armen 2012-07-02 11:25) - PLID 49831 - Only enable save button when we have an active editable emn
void CEmrFrameWnd::OnUpdateSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);
}

// (j.armen 2012-07-02 11:25) - PLID 49831 - Only enable save all button when we have at least one editable emn
void CEmrFrameWnd::OnUpdateSaveAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(HasEditableEMN() ? TRUE : FALSE);
}

void CEmrFrameWnd::OnUpdateCloseEMR(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

// (a.walling 2012-07-16 08:56) - PLID 51547 - Reset layout command
void CEmrFrameWnd::OnResetLayout()
{
	try {
		ClearLayout();
		MessageBox("The saved layout has been cleared, and the current layout will not be saved when closed. The next time you open this window, default layout will be used.");
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnSave()
{
	try {
		if (GetActiveEmrTopicView() && GetActiveEmrTopicView()->GetAttachedEMN()) {
			GetEmrTreeWnd()->SaveEMR(esotEMN, (long)GetActiveEmrTopicView()->GetAttachedEMN(), TRUE);
		} else {
			OnSaveAll();
		}
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnSaveAll()
{
	try {
		GetEmrEditorBase()->Save(TRUE);
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnSaveAndClose()
{	
	try {
		if (g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
			PostMessage(WM_CLOSE);
			return;
		}

		// (j.jones 2012-10-11 18:01) - PLID 52820 - let the Save function know we're closing
		if (essSuccess == GetEmrEditorBase()->Save(TRUE, TRUE)) {
			PostMessage(WM_CLOSE);
		}
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnCloseEMR()
{
	try {
		PostMessage(WM_CLOSE);
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnUpdateViewPreviewPane(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_emrPreviewPane.GetSafeHwnd() && m_emrPreviewPane.IsVisible() ? TRUE : FALSE);
}

void CEmrFrameWnd::OnUpdateViewTreePane(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_emrTreePane.IsVisible() ? TRUE : FALSE);
}

//(e.lally 2012-02-16) PLID 48065 - Handles updating the UI (ribbon checkbox) based on our state.
void CEmrFrameWnd::OnUpdateViewClassicButtonPane(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_emrClassicButtonPane.IsVisible() ? TRUE : FALSE);
}

void CEmrFrameWnd::OnViewPreviewPane()
{
	try {
		if (!m_emrPreviewPane.GetSafeHwnd()) {
			return;
		}

		CDockablePane& pane(m_emrPreviewPane);

		if (pane.IsVisible()) {
			pane.ShowPane(FALSE, FALSE, FALSE);
		} else {
			pane.ShowPane(TRUE, FALSE, TRUE);
		}
		RecalcLayout();
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnViewTreePane()
{
	try {
		CDockablePane& pane(m_emrTreePane);

		if (pane.IsVisible()) {
			pane.ShowPane(FALSE, FALSE, FALSE);
		} else {
			pane.ShowPane(TRUE, FALSE, TRUE);
		}
		RecalcLayout();
	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-02-16) PLID 48065 - Shows or hides the EMR Classic Button pane
void CEmrFrameWnd::OnViewClassicButtonPane()
{
	try {
		CDockablePane& pane(m_emrClassicButtonPane);

		if (pane.IsVisible()) {
			pane.ShowPane(FALSE, FALSE, FALSE);
		} else {
			pane.ShowPane(TRUE, FALSE, TRUE);
		}
		RecalcLayout();
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// (a.walling 2012-07-03 10:56) - PLID 51335 - Use NxSetWindowText
	SetTitle(GenerateTitleBarText());
	bool bMainFrameChanged = WindowUtils::NxSetWindowText(this, m_strTitle);
	bool bRibbonChanged = WindowUtils::NxSetWindowText(&m_wndRibbonBar, m_strTitle);

	if (bMainFrameChanged || bRibbonChanged) {
		m_wndRibbonBar.RecalcLayout();
	}
}

// (j.armen 2012-07-02 11:26) - PLID 49831 - Only allow switching write access when we have permission to do so
void CEmrFrameWnd::OnUpdateWriteAccess(CCmdUI* pCmdUI)
{
	CEMN* pEMN = GetActiveEMN();
	
	if (pEMN) {
		if(!pEMN->IsLockedAndSaved()
			&& CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE)
			&& (g_pLicense->HasEMR(CLicense::cflrSilent) == 2)) {
			pCmdUI->Enable(TRUE);
			pCmdUI->SetText(pEMN->IsWritable() ? "Writable" : "Read-only");
		} else {
			pCmdUI->Enable(FALSE);
			pCmdUI->SetText("Read-only");
		}
	} else {
		pCmdUI->Enable(FALSE);
		pCmdUI->SetText("Read-only");
	}
}

// (a.walling 2012-03-26 12:49) - PLID 49205 - Chart, category
void CEmrFrameWnd::OnUpdateChart(CCmdUI* pCmdUI)
{
	//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	CMFCRibbonCmdUI* pRibbonCmdUI = dynamic_cast<CMFCRibbonCmdUI*>(pCmdUI);
	if (!pRibbonCmdUI) return;

	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pRibbonCmdUI->m_pUpdated);
	if (!pList) return;	

	EMNChart curChart = pEMN->GetChart();

	if (-1 == curChart.nID) {
		pList->SelectItem(-1);
	} else if (!pList->SelectItem((DWORD_PTR)curChart.nID)) {
		ASSERT(curChart.nID >= 0);
		pList->SelectItem(pList->AddItem(curChart.strName, (DWORD_PTR)curChart.nID));
	}
}

// (a.walling 2012-03-26 12:49) - PLID 49205 - Chart, category
void CEmrFrameWnd::OnUpdateCategory(CCmdUI* pCmdUI)
{
	//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	CMFCRibbonCmdUI* pRibbonCmdUI = dynamic_cast<CMFCRibbonCmdUI*>(pCmdUI);
	if (!pRibbonCmdUI) return;

	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pRibbonCmdUI->m_pUpdated);
	if (!pList) return;	

	EMNCategory curCategory = pEMN->GetCategory();

	if (-1 == curCategory.nID) {
		pList->SelectItem(-1);
	} else if (!pList->SelectItem((DWORD_PTR)curCategory.nID)) {
		ASSERT(curCategory.nID >= 0);
		pList->SelectItem(pList->AddItem(curCategory.strName, (DWORD_PTR)curCategory.nID));
	}
}

// (a.walling 2012-06-28 17:13) - PLID 51277 - More Info button
void CEmrFrameWnd::OnUpdateMoreInfo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEMN() ? TRUE : FALSE);
}

//TES 2/13/2014 - PLID 60749 - Codes button
void CEmrFrameWnd::OnUpdateCodes(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEMN() ? TRUE : FALSE);
}

void CEmrFrameWnd::OnUpdateDescription(CCmdUI* pCmdUI)
{
	//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);

	CString strDesc;

	if (CEMN* pEMN = GetActiveEMN()) {
		strDesc = pEMN->GetDescription();
	}

	// (a.walling 2012-03-23 18:01) - PLID 49196 - Description - ensure we set the EditText rather than the label if this is a ribbon item
	if (dynamic_ptr<CMFCRibbonCmdUI> pRibbonCmdUI = pCmdUI) {
		// (a.walling 2012-05-22 14:46) - PLID 50571 - Always use the best type to handle non-virtual overrides
		if (dynamic_ptr<CNxRibbonEditEx> pEdit = pRibbonCmdUI->m_pUpdated) {
			pEdit->SetEditText(strDesc);
		}
	}
}

// (r.gonet 08/03/2012) - PLID 51027 - If the EMN is editable, then activate the button. Else disable it.
void CEmrFrameWnd::OnUpdateWoundCareAutoCoding(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetActiveEditableEMN() ? TRUE : FALSE);
}

//(e.lally 2012-02-29) PLID 48065 - This is common to both patients and templates so I moved it back to its home
void CEmrFrameWnd::OnUpdateEmrDebug(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CEmrFrameWnd::OnWriteAccess()
{
	try {
		GetEmrTreeWnd()->PostMessage(WM_COMMAND, MAKEWPARAM(IDM_TOGGLEREADONLY, 0), 0);
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-03-26 12:49) - PLID 49205 - Chart, category
void CEmrFrameWnd::OnChart()
{
	try {
		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		CEMN* pEMN = GetActiveEditableEMN();
		if (!pEMN) return;

		if (dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_EMN_CHART)) {
			EMNChart newChart;
			long nCurSel = pList->GetCurSel();
			if (-1 != nCurSel) {
				newChart.nID = pList->GetItemData(nCurSel);
				newChart.strName = pList->GetItem(nCurSel);
			}
			
			EMNCategory curCategory = pEMN->GetCategory();

			pEMN->SetChart(newChart.nID, newChart.strName, FALSE);
			
			// (a.walling 2012-04-06 13:36) - PLID 49205 - Ensure the category is OK
			if (-1 != curCategory.nID) {
				using namespace PracData;
				if (!Cached<EmnTabChartCategoryLinkT>()->get<0>().count(EmnTabChartCategoryLink(newChart.nID, curCategory.nID))) {
					//MessageBox("This chart/category combination is not valid. There is no longer a category selected on this EMN.");
					MessageBox(FormatString("This chart %s is not valid with the category %s. There is no longer a category selected on this EMN.", newChart.strName, curCategory.strName));
					// (a.walling 2012-06-08 14:41) - PLID 49205 - Was not actually resetting the category
					pEMN->SetCategory(-1, "");
					return;
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-03-26 12:49) - PLID 49205 - Chart, category
void CEmrFrameWnd::OnCategory()
{
	try {
		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		CEMN* pEMN = GetActiveEditableEMN();
		if (!pEMN) return;

		if (dynamic_ptr<CNxRibbonAutoComboBox> pList = m_wndRibbonBar.FindByID(ID_EMR_EMN_CATEGORY)) {		
			long nCurSel = pList->GetCurSel();
			if (-1 != nCurSel) {
				EMNCategory newCategory;
				newCategory.nID = pList->GetItemData(nCurSel);
				newCategory.strName = pList->GetItem(nCurSel);
				pEMN->SetCategory(newCategory.nID, newCategory.strName);
			} else {
				pEMN->SetCategory(-1, "");
			}
		}	
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-06-28 17:13) - PLID 51277 - More Info button
void CEmrFrameWnd::OnMoreInfo()
{
	try {
		if (CEMN* pEMN = GetActiveEMN()) {
			ShowMoreInfo(pEMN);
		}
	} NxCatchAll(__FUNCTION__);
}

//TES 2/13/2014 - PLID 60749 - Codes button
void CEmrFrameWnd::OnCodes()
{
	try {
		if (CEMN* pEMN = GetActiveEMN()) {
			ShowCodesView(pEMN);
		}
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnDescription()
{
	try {
		// (a.walling 2012-03-23 18:01) - PLID 49196 - Description
		//(e.lally 2012-05-07) PLID 48951 - Use new name that shows active EMN can be edited
		if (CEMN* pEMN = GetActiveEditableEMN()) {
			// (a.walling 2012-05-22 14:46) - PLID 50571 - Always use the best type to handle non-virtual overrides
			if (dynamic_ptr<CNxRibbonEditEx> pEdit = m_wndRibbonBar.FindByID(ID_EMR_EMN_DESCRIPTION)) {
				// (a.walling 2012-03-22 16:50) - PLID 49141 - This will notify the interface for us now
				pEMN->SetDescription(pEdit->GetEditText());
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 08/03/2012) - PLID 51027 - Wound Care Auto Coding button
void CEmrFrameWnd::OnWoundCareAutoCoding()
{
	try {
		if (CEMN* pEMN = GetActiveEditableEMN()) {
			pEMN->PerformWoundCareAutoCoding();
		} else {
			// We shouldn't be able to get to this code, since the button should be disabled,
			//  so assert to see why we have. No harm, since this is handled, but still shouldn't happen.
			ASSERT(FALSE);
		}
	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-02-29) PLID 48065 - This is common to both patients and templates so I moved it back to its home
void CEmrFrameWnd::OnEmrDebug()
{
	try {
		GetEmrTreeWnd()->OnEMRDebug();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
void CEmrFrameWnd::OnUpdatePreviewPanePrint(CCmdUI* pCmdUI)
{
	if (GetActiveEMN() && GetEmrPreviewCtrl()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
void CEmrFrameWnd::OnUpdatePreviewPanePrintPreview(CCmdUI* pCmdUI)
{
	if (GetActiveEMN() && GetEmrPreviewCtrl()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
void CEmrFrameWnd::OnUpdatePreviewPanePrintMultiple(CCmdUI* pCmdUI)
{
	if (CEMN* pActiveEMN = GetActiveEMN()) {
		if (GetEmrPreviewCtrl() && pActiveEMN->GetParentEMR() && pActiveEMN->GetParentEMR()->GetEMNCount() > 1) {
			pCmdUI->Enable(TRUE);
			return;
		}
	}

	pCmdUI->Enable(FALSE);
}

void CEmrFrameWnd::OnUpdatePreviewPaneConfigure(CCmdUI* pCmdUI)
{
	if (GetActiveEMN() && GetEmrPreviewCtrl()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
void CEmrFrameWnd::OnUpdatePreviewPaneAutoScroll(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	if (CEmrTreeWnd* pTreeWnd = GetEmrTreeWnd()) {
		pCmdUI->SetCheck(pTreeWnd->IsAutoScroll() ? 1 : 0);

		if (GetActiveEMN() && GetEmrPreviewCtrl()) {
			bEnable = TRUE;
		}

	} else {
		pCmdUI->SetCheck(0);
	}	

	pCmdUI->Enable(bEnable);
}

void CEmrFrameWnd::OnUpdateTemplateHideTitle(CCmdUI* pCmdUI)
{
	//TES 1/17/2014 - PLID 60397 - Option to hide the EMN title on the preview pane, per template
	if (CEMN* pEMN = GetActiveEMN()) {
		pCmdUI->Enable(TRUE);
		pCmdUI->SetCheck(pEMN->GetHideTitleOnPreview()?1:0);
	}
	else {
		pCmdUI->SetCheck(0);
		pCmdUI->Enable(FALSE);
	}
}

// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
void CEmrFrameWnd::OnPreviewPanePrint()
{
	try {
		if (CEMRPreviewCtrlDlg* pEmrPreviewCtrl = GetEmrPreviewCtrl()) {
			pEmrPreviewCtrl->Print();
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
void CEmrFrameWnd::OnPreviewPanePrintPreview()
{
	try {
		if (CEMRPreviewCtrlDlg* pEmrPreviewCtrl = GetEmrPreviewCtrl()) {
			pEmrPreviewCtrl->PrintPreview();
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
void CEmrFrameWnd::OnPreviewPanePrintMultiple()
{
	try {
		if (CEMRPreviewCtrlDlg* pEmrPreviewCtrl = GetEmrPreviewCtrl()) {
			pEmrPreviewCtrl->PrintMultipleEMNs();
		}
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnPreviewPaneConfigure()
{
	try {
		// (a.walling 2008-10-14 10:22) - PLID 31678 - Configure the preview via the preview control
		if (GetEmrPreviewCtrl()) {
			GetEmrPreviewCtrl()->ConfigurePreview();
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-03-07 08:36) - PLID 48680 - Preview pane commands and options
void CEmrFrameWnd::OnPreviewPaneAutoScroll()
{
	try {
		GetEmrTreeWnd()->m_bAutoScroll = GetEmrTreeWnd()->m_bAutoScroll ? FALSE : TRUE;

		SetRemotePropertyInt("EMNPreviewAutoScroll", GetEmrTreeWnd()->m_bAutoScroll, 0, GetCurrentUserName());

		if (GetEmrTreeWnd()->m_bAutoScroll) {
			GetEmrTreeWnd()->OnGotoPreview();
		}
	} NxCatchAll(__FUNCTION__);
}

void CEmrFrameWnd::OnTemplateHideTitle()
{
	try {
		//TES 1/17/2014 - PLID 60397 - Option to hide the EMN title on the preview pane, per template
		if (CEMN* pEMN = GetActiveEMN()) {
			pEMN->SetHideTitleOnPreview(!pEMN->GetHideTitleOnPreview());
		}
	}NxCatchAll(__FUNCTION__);
}

///////////

CEMN* CEmrFrameWnd::GetActiveEMN()
{
	CEmrTreeWnd* pTreeWnd = GetEmrTreeWnd();

	if (!pTreeWnd) {
		return NULL;
	}

	return pTreeWnd->GetActiveEMN();
}

// (j.armen 2012-07-02 11:29) - PLID 49831 - Determine if we have at least one editable emn in this emr
BOOL CEmrFrameWnd::HasEditableEMN()
{
	CEmrTreeWnd* pTreeWnd = GetEmrTreeWnd();

	if (!pTreeWnd) {
		return NULL;
	}

	return pTreeWnd->HasEditableEMN();
}

//(e.lally 2012-05-07) PLID 48951 - Renamed GetActiveWritableEMN to GetActiveEditableEMN to show we are checking locked status, permissions etc. too
CEMN* CEmrFrameWnd::GetActiveEditableEMN()
{
	CEMN* pEMN = GetActiveEMN();

	//(e.lally 2012-05-07) PLID 48951 - Check CanBeEdited
	if (!pEMN || !pEMN->CanBeEdited()) {
		return NULL;
	}

	return pEMN;
}

CEMR* CEmrFrameWnd::GetActiveEMR()
{
	if (CEmrTreeWnd* pTreeWnd = GetEmrTreeWnd()) {
		return pTreeWnd->GetEMR();
	}

	return NULL;
}

// (j.dinatale 2012-09-18 15:13) - PLID 52702 - be able to retrieve the cached value for the EMR save preference
long CEmrFrameWnd::GetEMRSavePref()
{
	return m_nEMRSavePref;
}

#if(_WIN32_WINNT < 0x0501)
#define APPCOMMAND_NEW                    29
#define APPCOMMAND_CLOSE                  31
#define APPCOMMAND_SAVE                   32
#define APPCOMMAND_PRINT                  33
#endif /* _WIN32_WINNT >= 0x0501 */

// (a.walling 2012-03-05 13:14) - PLID 48615 - calls TranslateAppCommand, checks via UpdateCmdUI that the command is enabled, posts WM_COMMAND if so
LRESULT CEmrFrameWnd::OnAppCommand(WPARAM wParam, LPARAM lParam)
{
	try {
		UINT nTranslated = TranslateAppCommand((HWND)wParam, GET_APPCOMMAND_LPARAM(lParam), GET_DEVICE_LPARAM(lParam), GET_KEYSTATE_LPARAM(lParam));

		// make sure command has not become disabled before routing
		if (WindowUtils::IsCommandEnabled(nTranslated, this)) {
			PostMessage(WM_COMMAND, nTranslated);
			return TRUE;
		}
	} NxCatchAll(__FUNCTION__);

	return Default();
}

// (a.walling 2012-03-05 13:14) - PLID 48615 - translate appcommand into command id; return 0 for no handler.
UINT CEmrFrameWnd::TranslateAppCommand(HWND hwndFrom, UINT nCmd, UINT nDevice, UINT nKey)
{
	switch (nCmd) {
		case APPCOMMAND_BROWSER_FORWARD:
			return (nKey & MK_CONTROL) ? ID_EMR_LAST_TOPIC : ID_EMR_NEXT_TOPIC;
		case APPCOMMAND_BROWSER_BACKWARD:
			return (nKey & MK_CONTROL) ? ID_EMR_FIRST_TOPIC : ID_EMR_PREVIOUS_TOPIC;
		case APPCOMMAND_SAVE:
			return ID_EMR_SAVE_ALL;
		case APPCOMMAND_PRINT:
			return ID_EMR_PREVIEW_PANE_PRINT_PREVIEW;
		case APPCOMMAND_NEW:
			return IsTemplate() ? 0 : ID_EMR_NEW_EMN;
		case APPCOMMAND_CLOSE:
			return ID_EMR_CLOSE;
		default:
			TRACE(__FUNCTION__" - unsupported WM_APPCOMMAND (hwnd 0x%08x; cmd 0x%04x; dev %04x; key %04x;\n", hwndFrom, nCmd, nDevice, nKey);
			return 0;
	}
}

// (a.walling 2012-03-26 12:49) - PLID 49205 - Chart, category
// (a.walling 2012-04-06 12:00) - PLID 49205 - Use NxModel's PracData cache
void CEmrFrameWnd::SynchronizeDelayedChartSubitems(CMFCRibbonBaseElement* pElement)
{
	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pElement);

	if (!pList) return;

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;
	
	long nCurChartID = pEMN->GetChart();

	pList->RemoveAllItems();

	using namespace PracData;
	for (const EmnTabChart& chart : Cached<EmnTabChartsT>()->get<Priority>()) {
		pList->AddItem(chart.description, (DWORD_PTR)chart.id);
	}

	if (-1 != nCurChartID) {
		pList->SelectItem((DWORD_PTR)nCurChartID);
	}
}

// (a.walling 2012-03-26 12:49) - PLID 49205 - Chart, category
// (a.walling 2012-04-06 12:00) - PLID 49205 - Use NxModel's PracData cache
void CEmrFrameWnd::SynchronizeDelayedCategorySubitems(CMFCRibbonBaseElement* pElement)
{
	CNxRibbonAutoComboBox* pList = dynamic_cast<CNxRibbonAutoComboBox*>(pElement);

	if (!pList) return;

	CEMN* pEMN = GetActiveEMN();
	if (!pEMN) return;

	long nCurrentChartID = pEMN->GetChart();
	long nCurCategoryID = pEMN->GetCategory();

	pList->RemoveAllItems();

	using namespace PracData;
	for (const EmnTabCategory& category : Cached<EmnTabCategoriesT>()->get<Priority>()) {
		if (-1 != nCurrentChartID) {
			if (!Cached<EmnTabChartCategoryLinkT>()->get<0>().count(EmnTabChartCategoryLink(nCurrentChartID, category.id))) {
				continue;
			}
		}

		pList->AddItem(category.description, (DWORD_PTR)category.id);
	}

	if (-1 != nCurCategoryID) {
		pList->SelectItem((DWORD_PTR)nCurCategoryID);
	}
}

// (a.walling 2012-07-03 10:56) - PLID 51284 - virtual events can be overridden by derived classes
// (j.dinatale 2012-09-18 16:56) - PLID 52702 - pass in to and from pointers
void CEmrFrameWnd::OnActiveEMNChanged(CEMN *pFrom, CEMN *pTo)
{
}

void CEmrFrameWnd::OnActiveTopicChanged(CEMRTopic *pFrom, CEMRTopic *pTo)
{
}

// (a.walling 2013-02-13 09:30) - PLID 52629 - Dynamic view layout - create views from path
CMDIChildWndEx* CEmrFrameWnd::CreateDocumentWindow(LPCTSTR lpcszDocName, CObject* pObj)
{
	CView* pView = NULL;

	if (!strcmp(lpcszDocName, "Topics.View.Emr.Nx")) {
		pView = m_pEmrTopicViewDocTemplate->GetEmrTopicView(NULL, false);
		if (!pView) {
			pView = m_pEmrTopicViewDocTemplate->CreateEmrTopicView();
			// (a.walling 2013-05-02 11:39) - PLID 52629 - Ensure the frame has an active view
			if (pView && pView->GetParentFrame()) {
				pView->GetParentFrame()->SetActiveView(pView, FALSE);
			}
		}
	} /*else if (!strcmp(lpcszDocName, "MoreInfo.View.Emr.Nx")) {
		pView = m_pEmrTopicViewDocTemplate->CreateEmrMoreInfoView();
	}*/

	//if (!pView) {
	//	pView = m_pEmrTopicViewDocTemplate->CreatePlaceholderView(lpcszDocName);
	//}

	if (!pView) {
		return NULL;
	}

	return polymorphic_downcast<CMDIChildWndEx*>(pView->GetParentFrame());
}

// (a.walling 2013-02-13 09:30) - PLID 52629 - Dynamic view layout - create views from path
CMDIChildWndEx* CEmrFrameWnd::CreateNewWindow(LPCTSTR lpcszDocName, CObject* pObj)
{
	return CreateDocumentWindow(lpcszDocName, pObj);
}


namespace {

// (a.walling 2013-04-17 17:25) - PLID 52629 - Finds the last tab group that has a topic tab and no static tabs
std::vector<Nx::MDI::TabGroup>::iterator FindTargetTopicTabGroup(std::vector<Nx::MDI::TabGroup>& tabGroups)
{
	std::vector<Nx::MDI::TabGroup>::iterator itLastTarget = tabGroups.end();
	for (std::vector<Nx::MDI::TabGroup>::iterator it = tabGroups.begin(); it != tabGroups.end(); ++it) {
		struct {
			int staticTabs;
			int topicTabs;
			int dynamicTabs;
		} stats = {0, 0, 0};

		for each (const Nx::MDI::Tab& tab in it->tabs) {
			if (tab.nBarID != -1) {
				++stats.staticTabs;
			} else if (tab.strDocumentName == "Topics.View.Emr.Nx") {
				++stats.topicTabs;
			} else {
				++stats.dynamicTabs;
			}
		}

		if (stats.topicTabs && !stats.staticTabs) {
			itLastTarget = it;
		}
	}

	return itLastTarget;
}

void RemoveAllButLastTopicTab(Nx::MDI::TabGroup& tabGroup)
{
	// remove all but the last topic tab in this group
	std::vector<Nx::MDI::Tab>::iterator prevTopicTab = tabGroup.tabs.end();
	for (std::vector<Nx::MDI::Tab>::iterator itTab = tabGroup.tabs.begin(); itTab != tabGroup.tabs.end(); ++itTab) {
		if (itTab->strDocumentName == "Topics.View.Emr.Nx") {
			if (prevTopicTab != tabGroup.tabs.end()) {
				prevTopicTab->strDocumentName.Empty();
			}
			prevTopicTab = itTab;
		}
	}
}

void RemoveAllTopicTab(Nx::MDI::TabGroup& tabGroup)
{
	for (std::vector<Nx::MDI::Tab>::iterator itTab = tabGroup.tabs.begin(); itTab != tabGroup.tabs.end(); ++itTab) {
		if (itTab->strDocumentName == "Topics.View.Emr.Nx") {
			itTab->strDocumentName.Empty();
		}
	}
}

}

// (a.walling 2013-04-17 17:25) - PLID 52629 - Modify the tab groups before opening
void CEmrFrameWnd::PreOpenTabGroups(std::vector<Nx::MDI::TabGroup>& tabGroups)
{
	// (a.walling 2013-04-17 17:25) - PLID 52629 - basically, we will try to keep the tabgroups open in case of multiple topic views saved in the layout
	// (a.walling 2013-05-02 14:54) - PLID 52629 - Always need to filter out the any duplicate topic tabs, even if only one group

	std::vector<Nx::MDI::TabGroup>::iterator itTopicGroupTarget = FindTargetTopicTabGroup(tabGroups);
	if (itTopicGroupTarget == tabGroups.end()) {
		itTopicGroupTarget = tabGroups.begin();
	}

	for (std::vector<Nx::MDI::TabGroup>::iterator it = tabGroups.begin(); it != tabGroups.end(); ++it) {
		if (it != itTopicGroupTarget) {
			// remove all topic tabs in this group
			RemoveAllTopicTab(*it);
		} else {
			// remove all but the last topic tab in this group
			RemoveAllButLastTopicTab(*it);
		}
	}
}

//////////////////////////////////////


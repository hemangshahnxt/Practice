// EMRCustomPreviewLayoutFrameWnd.cpp : implementation file
// (c.haag 2013-01-16) - PLID 54612 - Initial implementation. This is the
// frame window that contains one or more views.

#include "stdafx.h"
#include <NxAdvancedUILib/NxHtmlMDIDoc.h>
#include "EMRCustomPreviewLayoutsMDIFrame.h"
#include "nxexception.h"
#include "NxStandard.h"
#include "AdministratorRc.h"
#include "NxAPI.h"

// CEMRCustomPreviewLayoutsMDIFrame

IMPLEMENT_DYNAMIC(CEMRCustomPreviewLayoutsMDIFrame, CMDIFrameWndEx)

std::vector<CEMRCustomPreviewLayoutsMDIFrame*> g_allCustomPreviewLayoutFrameWnds;

/* static */ CEMRCustomPreviewLayoutsMDIFrame* CEMRCustomPreviewLayoutsMDIFrame::FindOpenTemplate(long nEmrTemplateID)
{
	foreach (CEMRCustomPreviewLayoutsMDIFrame* pFrameWnd, g_allCustomPreviewLayoutFrameWnds) {
		if (!pFrameWnd || !::IsWindow(pFrameWnd->GetSafeHwnd())) {
			continue;
		}

		if (pFrameWnd->GetTemplateID() == nEmrTemplateID) {
			return pFrameWnd;
		}
	}

	return NULL;
}

/* static */ CEMRCustomPreviewLayoutsMDIFrame* CEMRCustomPreviewLayoutsMDIFrame::LaunchWithTemplate(long nEmrTemplateID, const CString& strEmrTemplateName)
{
	// (c.haag 2013-01-23) - PLID 54739 - Initial implementation
	if (CEMRCustomPreviewLayoutsMDIFrame* pOpenTemplateWnd = FindOpenTemplate(nEmrTemplateID)) {
		pOpenTemplateWnd->ActivateFrame(SW_SHOW);
		pOpenTemplateWnd->SetActiveWindow();
		return pOpenTemplateWnd;
	} else {
		CWaitCursor wc;

		// Load the custom preview layouts
		NexTech_Accessor::_EMRCustomPreviewLayoutFilterPtr pFilter(__uuidof(NexTech_Accessor::EMRCustomPreviewLayoutFilter)); 
		NexTech_Accessor::_EMRCustomPreviewLayoutOptionsPtr pOptions(__uuidof(NexTech_Accessor::EMRCustomPreviewLayoutOptions));
		pFilter->EMRTemplateIDs = Nx::SafeArray<BSTR>::FromValue(_bstr_t(nEmrTemplateID));
		pOptions->IncludeData = VARIANT_TRUE;

		//	Create our SAFEARRAY to be passed to the DiagCodeImport function in the API
		Nx::SafeArray<IUnknown *> saFilters = Nx::SafeArray<IUnknown *>::FromValue(pFilter);
		NexTech_Accessor::_EMRCustomPreviewLayoutsPtr pLayouts = GetAPI()->GetEMRCustomPreviewLayouts(
			GetAPISubkey(), GetAPILoginToken(), saFilters, pOptions);

		// Create a new frame that contains the custom preview layout editor. It uses reference
		// counting so it will stay in memory even after we leave this function. This works consistently
		// as the EMR template editor does.
		std::auto_ptr<CEMRCustomPreviewLayoutsMDIFrame> pCustomPreviewLayoutFrame;
		pCustomPreviewLayoutFrame.reset(new CEMRCustomPreviewLayoutsMDIFrame(nEmrTemplateID, strEmrTemplateName));

		// Now that we've constructed the CNxHtmlMDIFrameWnd object, lets create the frame
		// and effectively create the editor window.
		pCustomPreviewLayoutFrame->LoadFrame(IDR_CUSTOMPREVIEWLAYOUTS);

		// Proceed if it worked
		if (pCustomPreviewLayoutFrame->GetSafeHwnd()) 
		{
			// (v.maida 2016-06-06 15:55) - NX-100631 - Set a larger default size so that the dialog is easier to read if it's restored down to a smaller size.
			pCustomPreviewLayoutFrame->SetWindowPos(NULL, 0, 0, 994, 738, 0);
			pCustomPreviewLayoutFrame->ShowWindow(SW_SHOWMAXIMIZED);
			pCustomPreviewLayoutFrame->UpdateWindow();

			CNxHtmlMDITemplate *pDocTemplate = pCustomPreviewLayoutFrame->GetDocTemplate();
			int nLayouts = 0;
			if (NULL != pLayouts->Layouts) {
				Nx::SafeArray<IUnknown *> saryLayouts(pLayouts->Layouts);
				if (saryLayouts.GetCount() > 0)
				{
					foreach(NexTech_Accessor::_EMRCustomPreviewLayoutPtr pLayout, saryLayouts)
					{
						CNxHtmlMDIDoc* pDoc = dynamic_cast<CNxHtmlMDIDoc*>(pDocTemplate->OpenDocumentFile(NULL, TRUE));
						pDoc->SetDocName((LPCTSTR)pLayout->Name);
						pDoc->SetHtml((LPCTSTR)pLayout->data);
						pDoc->SetID(atol((LPCTSTR)pLayout->ID));
						pDoc->UpdateAllViews(NULL);
						nLayouts++;
					}
				}
			}

			// If there are no layouts for this template, generate the default one based on the template content
			if (0 == nLayouts) {
				pCustomPreviewLayoutFrame->CreateNewLayout(TRUE);
			}

			// Reference counting; release our own hold on it
			pCustomPreviewLayoutFrame.release();
			return pCustomPreviewLayoutFrame.get();
		}
		else
		{
			return NULL;
		}
	}
}

CEMRCustomPreviewLayoutsMDIFrame::CEMRCustomPreviewLayoutsMDIFrame(long nEmrTemplateID, const CString& strEmrTemplateName) :
m_TreePane(nEmrTemplateID, strEmrTemplateName)
{
	m_nEmrTemplateID = nEmrTemplateID;
	m_strEmrTemplateName = strEmrTemplateName;
	g_allCustomPreviewLayoutFrameWnds.push_back(this);
}

CEMRCustomPreviewLayoutsMDIFrame::~CEMRCustomPreviewLayoutsMDIFrame()
{
	boost::remove_erase(g_allCustomPreviewLayoutFrameWnds, this);
}

// Create the panes
void CEMRCustomPreviewLayoutsMDIFrame::CreatePanes()
{
	// Enable docking
	EnableDocking(CBRS_ALIGN_ANY);
	GetDockingManager()->	EnableDockSiteMenu(FALSE);

	// Create the EMR template tree pane
	m_TreePane.CreateEx(WS_EX_CONTROLPARENT, m_strEmrTemplateName, this, CRect(0, 0, 250, 100), TRUE, 
		ID_EMRCUSTOMPREVIEWLAYOUTSFRAME_TREEPANE,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT, 
		AFX_CBRS_REGULAR_TABS, AFX_CBRS_RESIZE);
	m_TreePane.EnableDocking(CBRS_ALIGN_LEFT);
	m_TreePane.SetMinSize(CSize(40, 65));
	DockPane(&m_TreePane);
}

// Returns TRUE if there are any modified documents in the frame
BOOL CEMRCustomPreviewLayoutsMDIFrame::CheckForModifiedDocuments(CArray<CNxHtmlMDIDoc*,CNxHtmlMDIDoc*>& apModifiedDocuments, CString& strWarning)
{
	// Check for unsaved documents!
	POSITION pos = m_pDocTemplate->GetFirstDocPosition();
	apModifiedDocuments.RemoveAll();
	
	// Build a warning message, too
	strWarning = "The following layouts have been modified:\r\n\r\n";
	while (pos) {
		if (CNxHtmlMDIDoc* pDoc = dynamic_cast<CNxHtmlMDIDoc*>(m_pDocTemplate->GetNextDoc(pos))) {
			if (pDoc->IsModified())	{
				apModifiedDocuments.Add(pDoc);
				if (apModifiedDocuments.GetCount() >= 21) {
					strWarning += "...\r\n";
					break;
				} else {
					strWarning += pDoc->GetDocName() + "\r\n";
				}
			}
		}			
	}

	if (apModifiedDocuments.GetCount() > 0)
	{
		strWarning += "\r\nDo you wish to save your changes?";
	}

	return (apModifiedDocuments.GetCount() > 0);
}

// Saves the contents of a view to data
void CEMRCustomPreviewLayoutsMDIFrame::SaveToDatabase(CEMRCustomPreviewLayoutView *pView)
{
	// (c.haag 2013-01-23) - PLID 54739 - Initial implementation
	// Get the document from the view and use that for the operation
	CNxHtmlMDIDoc *pDoc = dynamic_cast<CNxHtmlMDIDoc*>(pView->GetDocument());
	if (pDoc->IsModified())	{
		// Before we save the document, we have to ensure the document has the latest HTML from the view.
		// It would be inefficient to update the document with every keystroke; but we do ensure the document
		// was flagged as modified with those keystrokes.
		pDoc->SetHtml(pView->GetHtml());
		SaveToDatabase(pDoc);
	}
}

void CEMRCustomPreviewLayoutsMDIFrame::SaveToDatabase(CNxHtmlMDIDoc *pDoc)
{
	// (c.haag 2013-01-23) - PLID 54739 - Save a modified document to data through the API
	if (pDoc->IsModified())
	{
		// See if the document has a name assigned to it.
		if (!pDoc->HasDocName())
		{
			// If not, that must mean that it was the default document generated when the user opened the frame.
			// Force the user to choose a name for it.
			CString strLayoutName = pDoc->GetDocName(); // Default to the document's default layout name (should resemble something like 'unnamed')
			int nResult = PromptForLayoutName(this, strLayoutName);
			if(nResult == IDOK)
			{			
				pDoc->SetDocName(strLayoutName);
			}
			else
			{
				return; // Don't let the user save it without naming it. (nope.jpg)
			}
		}

		// Create the commit object
		NexTech_Accessor::_EMRCustomPreviewLayoutCommitPtr pCommit(__uuidof(NexTech_Accessor::EMRCustomPreviewLayoutCommit)); 
		if (!pDoc->IsNew()) {
			pCommit->ID = _bstr_t(pDoc->GetID());
		}
		pCommit->EMRTemplateID = _bstr_t(GetTemplateID());
		pCommit->Name = _bstr_t(pDoc->GetDocName());
		pCommit->data = _bstr_t(pDoc->GetHtml());

		// Now send it to the API
		Nx::SafeArray<IUnknown *> saCommits = Nx::SafeArray<IUnknown *>::FromValue(pCommit);
		NexTech_Accessor::_EMRCustomPreviewLayoutCommitResultsPtr pResults = GetAPI()->EditEMRCustomPreviewLayouts(
			GetAPISubkey(), GetAPILoginToken(), saCommits);

		// Now update our ID
		if (pDoc->IsNew())
		{
			Nx::SafeArray<IUnknown *> saryResults(pResults->Results);
			NexTech_Accessor::_EMRCustomPreviewLayoutCommitResultPtr pResult = saryResults[0];
			pDoc->SetID( atol((LPCTSTR)pResult->ID) );
		}

		// Reset the modified flag
		pDoc->SetModifiedFlag(FALSE);
	}
	else {
		// Document was not modified
	}
}

void CEMRCustomPreviewLayoutsMDIFrame::DeleteFromDatabase(CEMRCustomPreviewLayoutView *pView)
{
	// (c.haag 2013-01-23) - PLID 54739 - Initial implementation
	// Get the document from the view and use that for the operation
	CNxHtmlMDIDoc *pDoc = dynamic_cast<CNxHtmlMDIDoc*>(pView->GetDocument());
	DeleteFromDatabase(pDoc);
}

void CEMRCustomPreviewLayoutsMDIFrame::DeleteFromDatabase(CNxHtmlMDIDoc *pDoc)
{
	// (c.haag 2013-01-23) - PLID 54739 - Initial implementation
	// Delete the document data from the database through the API
	Nx::SafeArray<BSTR> saDoomedIDs = Nx::SafeArray<BSTR>::FromValue(_bstr_t(pDoc->GetID()));
	GetAPI()->DeleteEMRCustomPreviewLayouts(
		GetAPISubkey(), GetAPILoginToken(), saDoomedIDs);
}

/*
const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;
*/

BEGIN_MESSAGE_MAP(CEMRCustomPreviewLayoutsMDIFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_FILE_NEWLAYOUT, OnNewLayout)
	ON_COMMAND(ID_FILE_SAVEALLLAYOUTS, OnFileSaveAllLayouts)
	ON_COMMAND(ID_VIEW_SHOWHIDDENTEMPLATEDETAILS, OnViewShowHiddenTemplateDetails)
	ON_COMMAND(ID_FILE_SAVEALLLAYOUTSANDEXIT, OnFileSaveAllLayoutsAndExit)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_COMMAND(ID_VIEW_EXPANDALLTREEITEMS, OnViewExpandAllTreeItems)
	ON_COMMAND(ID_VIEW_COLLAPSEALLTREEITEMS, OnViewCollapseAllTreeItems)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVEALLLAYOUTS, OnUpdateFileSaveAllLayouts)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVEALLLAYOUTSANDEXIT, OnUpdateFileSaveAllLayouts)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWHIDDENTEMPLATEDETAILS, OnUpdateViewShowHiddenTemplateDetails)

	//ON_COMMAND(ID_WINDOW_MANAGER, &CEMRCustomPreviewLayoutsMDIFrame::OnWindowManager)
	//ON_COMMAND(ID_VIEW_CUSTOMIZE, &CEMRCustomPreviewLayoutsMDIFrame::OnViewCustomize)
	//ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CEMRCustomPreviewLayoutsMDIFrame::OnToolbarCreateNew)
END_MESSAGE_MAP()

/*
static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};*/

int CEMRCustomPreviewLayoutsMDIFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	try
	{
		if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
			return -1;

		//BOOL bNameValid;

		// set the visual manager used to draw all user interface elements
		//CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));

		CMDITabInfo mdiTabParams;
		mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // other styles available...
		mdiTabParams.m_bTabCloseButton = FALSE; // No close buttons allowed!
		mdiTabParams.m_bActiveTabCloseButton = FALSE;      // set to FALSE to place close button at right of tab area
		mdiTabParams.m_bTabIcons = FALSE;    // set to TRUE to enable document icons on MDI taba
		mdiTabParams.m_bAutoColor = TRUE;    // set to FALSE to disable auto-coloring of MDI tabs
		mdiTabParams.m_bDocumentMenu = FALSE; // enable the document menu at the right edge of the tab area
		EnableMDITabbedGroups(TRUE, mdiTabParams);

		if (!m_wndMenuBar.Create(this))
		{
			TRACE0("Failed to create menubar\n");
			return -1;      // fail to create
		}

		m_wndMenuBar.SetPaneStyle((m_wndMenuBar.GetPaneStyle() & ~(CBRS_GRIPPER)) | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

		// prevent the menu bar from taking the focus on activation
		CMFCPopupMenu::SetForceMenuFocus(FALSE);

		/*
		if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
			!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME))
		{
			TRACE0("Failed to create toolbar\n");
			return -1;      // fail to create
		}

		CString strToolBarName;
		bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
		ASSERT(bNameValid);
		m_wndToolBar.SetWindowText(strToolBarName);

		CString strCustomize;
		bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
		ASSERT(bNameValid);
		m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

		// Allow user-defined toolbars operations:
		InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

		if (!m_wndStatusBar.Create(this))
		{
			TRACE0("Failed to create status bar\n");
			return -1;      // fail to create
		}
		m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));*/

		// TODO: Delete these five lines if you don't want the toolbar and menubar to be dockable
		m_wndMenuBar.EnableDocking(CBRS_ALIGN_TOP);
		//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
		EnableDocking(CBRS_ALIGN_ANY);
		DockPane(&m_wndMenuBar);
		//DockPane(&m_wndToolBar);


		// enable Visual Studio 2005 style docking window behavior
		CDockingManager::SetDockingMode(DT_SMART);
		// enable Visual Studio 2005 style docking window auto-hide behavior
		EnableAutoHidePanes(CBRS_ALIGN_ANY);

		// Enable enhanced windows management dialog
		//EnableWindowsDialog(ID_WINDOW_MANAGER, IDS_WINDOWS_MANAGER, TRUE);

		// Enable toolbar and docking window menu replacement
		//EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

		// enable quick (Alt+drag) toolbar customization
		//CMFCToolBar::EnableQuickCustomization();

		/*if (CMFCToolBar::GetUserImages() == NULL)
		{
			// load user-defined toolbar images
			if (m_UserImages.Load(_T(".\\UserImages.bmp")))
			{
				m_UserImages.SetImageSize(CSize(16, 16), FALSE);
				CMFCToolBar::SetUserImages(&m_UserImages);
			}
		}*/

		/*
		// enable menu personalization (most-recently used commands)
		// TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.
		CList<UINT, UINT> lstBasicCommands;

		lstBasicCommands.AddTail(ID_FILE_NEW);
		lstBasicCommands.AddTail(ID_FILE_OPEN);
		lstBasicCommands.AddTail(ID_FILE_SAVE);
		lstBasicCommands.AddTail(ID_FILE_PRINT);
		lstBasicCommands.AddTail(ID_APP_EXIT);
		lstBasicCommands.AddTail(ID_EDIT_CUT);
		lstBasicCommands.AddTail(ID_EDIT_PASTE);
		lstBasicCommands.AddTail(ID_EDIT_UNDO);
		lstBasicCommands.AddTail(ID_APP_ABOUT);
		lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
		lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);

		CMFCToolBar::SetBasicCommands(lstBasicCommands);*/
	}
	NxCatchAll(__FUNCTION__);

	return 0;
}

void CEMRCustomPreviewLayoutsMDIFrame::OnClose()
{
	try {
		// Check for unsaved documents!
		CArray<CNxHtmlMDIDoc*,CNxHtmlMDIDoc*> apModifiedDocuments;
		CString strWarning;
		if (CheckForModifiedDocuments(apModifiedDocuments, strWarning))
		{
			switch (AfxMessageBox(strWarning, MB_YESNOCANCEL))
			{
			case IDYES:
				OnFileSaveAllLayouts();
				// Return if any documents are still unsaved because it must mean an error
				// occurred or the user canceled the operation.
				if (CheckForModifiedDocuments(apModifiedDocuments, strWarning))
					return;
				break;
			case IDNO:
				break;
			case IDCANCEL:
				return;
			}
		}

		__super::OnClose();
	}
	NxCatchAll(__FUNCTION__);
}

// Prompts for a layout name. Empty strings are rejected.
/* static */ int CEMRCustomPreviewLayoutsMDIFrame::PromptForLayoutName(CWnd* pParent, CString &strResult)
{
	// (c.haag 2013-02-08) - PLID 54739 - Initial implementation
	CString strOriginalName = strResult;
	BOOL bRetry;
	int nResult;
	do
	{
		bRetry = FALSE;
		nResult = InputBoxLimitedWithParent(pParent, "Please enter a name for the layout", strResult, "", 255, false, false, NULL);
		if (IDOK == nResult) {
			strResult.TrimRight();
			if (strResult.IsEmpty())
			{
				AfxMessageBox("Please enter a non-empty name.", MB_ICONERROR);
				bRetry = TRUE;
			}
		}
	} while (bRetry);
	return nResult;
}

// Creates and retunrs a new layout document. Set bSilent to FALSE to prompt the user to choose
// a name for the layout, or TRUE to generate it with the default name for layouts.
CNxHtmlMDIDoc* CEMRCustomPreviewLayoutsMDIFrame::CreateNewLayout(BOOL bSilent)
{
	// (c.haag 2013-01-23) - PLID 54739 - Initial implementation
	CWaitCursor wc;
	CString strLayoutName;

	if (!bSilent)
	{
		// If we're not silent, prompt the user for a layout name
		int nResult = PromptForLayoutName(this, strLayoutName);
		if(nResult != IDOK) return NULL;
	}
	else
	{
		// Just defer the layout's name to whatever its default is as prescribed by the document class
	}

	NexTech_Accessor::_EMRCustomPreviewLayoutPtr pLayout = GetAPI()->CreateEMRCustomPreviewLayoutFromEMRTemplate(
		GetAPISubkey(), GetAPILoginToken(), _bstr_t(AsString(m_nEmrTemplateID)));

	CNxHtmlMDIDoc* pDoc = dynamic_cast<CNxHtmlMDIDoc*>(m_pDocTemplate->OpenDocumentFile(NULL, TRUE));
	pDoc->SetHtml((LPCTSTR)pLayout->data);
	if (!strLayoutName.IsEmpty()) pDoc->SetDocName(strLayoutName);
	pDoc->UpdateAllViews(NULL);

	pDoc->SetModifiedFlag(); // Flag it modified because it's new
	return pDoc;
}

void CEMRCustomPreviewLayoutsMDIFrame::OnNewLayout()
{
	// (c.haag 2013-01-23) - PLID 54739 - Creates a new layout
	try {
		CreateNewLayout(FALSE);
	} NxCatchAll(__FUNCTION__);
}

void CEMRCustomPreviewLayoutsMDIFrame::OnFileSaveAllLayouts()
{
	// (c.haag 2013-01-23) - PLID 54739 - Initial implementation
	try {
		// Now go through and save all the unsaved layouts
		POSITION pos = m_pDocTemplate->GetFirstDocPosition();
		while (pos) {
			if (CNxHtmlMDIDoc* pDoc = dynamic_cast<CNxHtmlMDIDoc*>(m_pDocTemplate->GetNextDoc(pos))) 
			{
				if (pDoc->IsModified())	{
					// We need to pull the view content into the document because they are not auto-synced with
					// each keystroke modification
					POSITION posView = pDoc->GetFirstViewPosition();
					if (posView)
					{
						if (CEMRCustomPreviewLayoutView* pView = dynamic_cast<CEMRCustomPreviewLayoutView*>(pDoc->GetNextView(posView))) {
							SaveToDatabase(pView);
						}
					}					
				}
			}			
		}

	} NxCatchAll(__FUNCTION__);
}

void CEMRCustomPreviewLayoutsMDIFrame::OnViewShowHiddenTemplateDetails()
{
	// This is a checked toggle that shows or hides hidden template details in the
	// EMR template tree.
	try {
		// Update the tree
		m_TreePane.SetShowHiddenItems( m_TreePane.GetShowHiddenItems() ? FALSE : TRUE );
	} NxCatchAll(__FUNCTION__);
}

void CEMRCustomPreviewLayoutsMDIFrame::OnFileSaveAllLayoutsAndExit()
{
	try {
		// Save all the layouts
		OnFileSaveAllLayouts();
		// Now exit
		OnFileExit();
	} NxCatchAll(__FUNCTION__);
}

void CEMRCustomPreviewLayoutsMDIFrame::OnFileExit()
{
	try {
		PostMessage(WM_CLOSE);
	} NxCatchAll(__FUNCTION__);
}

void CEMRCustomPreviewLayoutsMDIFrame::OnUpdateFileSaveAllLayouts(CCmdUI* pCmdUI)
{
	try {
		pCmdUI->Enable( (NULL == m_pDocTemplate->GetFirstDocPosition()) ? FALSE : TRUE );
	} NxCatchAll(__FUNCTION__);
}

void CEMRCustomPreviewLayoutsMDIFrame::OnUpdateViewShowHiddenTemplateDetails(CCmdUI* pCmdUI)
{
	try {
		pCmdUI->SetCheck(m_TreePane.GetShowHiddenItems());
	} NxCatchAll(__FUNCTION__);
}

BOOL CEMRCustomPreviewLayoutsMDIFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	try {
		if( !CMDIFrameWndEx::PreCreateWindow(cs) )
			return FALSE;
		cs.style |= WS_MAXIMIZE;
	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

// CEMRCustomPreviewLayoutsMDIFrame diagnostics

#ifdef _DEBUG
void CEMRCustomPreviewLayoutsMDIFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CEMRCustomPreviewLayoutsMDIFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CEMRCustomPreviewLayoutsMDIFrame message handlers

/*void CEMRCustomPreviewLayoutsMDIFrame::OnWindowManager()
{
	ShowWindowsDialog();
}*/

//void CEMRCustomPreviewLayoutsMDIFrame::OnViewCustomize()
//{
//	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
//	pDlgCust->EnableUserDefinedToolbars();
//	pDlgCust->Create();
//}

/*
LRESULT CEMRCustomPreviewLayoutsMDIFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}
*/
BOOL CEMRCustomPreviewLayoutsMDIFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	m_pDocTemplate.reset(new CNxHtmlMDITemplate(nIDResource,
		RUNTIME_CLASS(CNxHtmlMDIDoc),
		RUNTIME_CLASS(CMDIChildWndEx),
		RUNTIME_CLASS(CEMRCustomPreviewLayoutView),
		this));


	// base class does the real work

	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}

	SetTitle("EMR Custom Preview Layouts for " + m_strEmrTemplateName);

	CreatePanes();

	// enable customization button for all user toolbars
	/*
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}*/

	return TRUE;
}

// (z.manning 2013-03-12 15:34) - PLID 55595
void CEMRCustomPreviewLayoutsMDIFrame::OnViewExpandAllTreeItems()
{
	try
	{
		m_TreePane.ExpandOrCollapseAllTopics(TRUE);
	}
	NxCatchAll(__FUNCTION__);
}

// (z.manning 2013-03-12 15:34) - PLID 55595
void CEMRCustomPreviewLayoutsMDIFrame::OnViewCollapseAllTreeItems()
{
	try
	{
		m_TreePane.ExpandOrCollapseAllTopics(FALSE);
	}
	NxCatchAll(__FUNCTION__);
}
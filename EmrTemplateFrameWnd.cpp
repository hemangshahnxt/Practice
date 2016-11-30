// EmrTemplateFrameWnd.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "EmrIcons.h"
#include "EmrTemplateFrameWnd.h"
#include "EmrAuditHistoryDlg.h"
// (a.walling 2012-04-09 14:34) - PLID 49527 - NxAdvancedUI
#include <NxAdvancedUILib/NxRibbonControls.h>
#include <NxAdvancedUILib/NxRibbonListButton.h>
#include <NxAdvancedUILib/NxRibbonPanel.h>
#include <NxPracticeSharedLib/PracDataEmr.h>
#include <NxPracticeSharedLib/PracData.h>

#include <foreach.h>
#include <NxAlgorithm.h>
#include "EMR.h"
#include "EMN.h"

// (a.walling 2011-12-11 13:25) - PLID 46644 - Template editor
// (a.walling 2012-02-29 06:42) - PLID 46644 - Moved some CEmrTemplateEditorDlg logic in here

// (a.walling 2012-05-22 09:59) - PLID 50556 - Prevent unnecessary redraws of ribbon combo boxes by always using CNxRibbonAutoComboBox instead of CMFCRibbonComboBox

// (a.walling 2012-05-22 15:15) - PLID 50582 - Use CNxRibbonButton, CNxRibbonCheckbox everywhere to ensure all bugs are consistently worked around

// (a.walling 2012-06-26 13:36) - PLID 51204 - Use CNxRibbonCategory, CNxRibbonPanel

// (a.walling 2013-04-08 08:57) - PLID 56131 - Added more icons, more!

// CEmrTemplateFrameWnd



// (a.walling 2012-03-02 11:52) - PLID 48469 - Made these static members which return a pointer to the new instance
CEmrTemplateFrameWnd* CEmrTemplateFrameWnd::CreateNewTemplate(long nCollectionID)
{
	std::auto_ptr<CEmrTemplateFrameWnd> pTemplateFrame;

	try {
		pTemplateFrame.reset(new CEmrTemplateFrameWnd(-1, nCollectionID));

		pTemplateFrame->Create();

		if (pTemplateFrame->GetSafeHwnd()) {
			return pTemplateFrame.release();
		}
	} NxCatchAll_NoParent(__FUNCTION__); // (a.walling 2014-05-05 13:32) - PLID 61945

	return NULL;
}

// (a.walling 2012-03-02 11:52) - PLID 48469 - Made these static members which return a pointer to the new instance
CEmrTemplateFrameWnd* CEmrTemplateFrameWnd::LaunchWithTemplate(long nEmrTemplateID)
{	
	// (a.walling 2012-04-10 15:59) - PLID 48469 - If already being edited, bring it to the top
	if (CEmrTemplateFrameWnd* pOpenTemplateWnd = CEmrTemplateFrameWnd::FindOpenTemplate(nEmrTemplateID)) {
		pOpenTemplateWnd->ActivateFrame(SW_SHOW);
		pOpenTemplateWnd->SetActiveWindow();
		return pOpenTemplateWnd;
	}
	
	//TES 10/30/2009 - PLID 35808 - Check whether this is their NexWeb template, and warn them accordingly.
	//(e.lally 2011-05-04) PLID 43537 - Use new NexWebDisplayT structure
	ADODB::_RecordsetPtr rsNexWebTemplate = CreateParamRecordset("SELECT EmrTemplateID FROM NexWebDisplayT "
		"WHERE EmrTemplateID = {INT} AND Visible = 1 ", nEmrTemplateID);
	if(!rsNexWebTemplate->eof) {
		// (b.cardillo 2010-09-22 09:18) - PLID 39568 - Corrected wording now that we allow more than one NexWeb EMR template
		if(IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "Warning: The template you are editing is a NexWeb template.  Any changes "
			"you make to it will be published to your website, and will be visible to patients.  Are you sure you "
			"wish to continue editing this template?")) {
				return NULL;
		}
	}

	// (j.gruber 2012-08-31 14:39) - PLID 52285 - warn if they are editing an OMR template	
	if(ReturnsRecordsParam("SELECT EmrTemplateID FROM OMRFormT WHERE EmrTemplateID = {INT} ", nEmrTemplateID)) {	
		if(IDYES != MsgBox(MB_YESNO|MB_ICONEXCLAMATION, "Warning: The template you are editing is associated with an OMR Form.  Are you sure you "
			"wish to continue editing this template?")) {
				return NULL;
		}
	}

	std::auto_ptr<CEmrTemplateFrameWnd> pTemplateFrame;

	try {
		pTemplateFrame.reset(new CEmrTemplateFrameWnd(nEmrTemplateID));

		pTemplateFrame->Create();

		if (pTemplateFrame->GetSafeHwnd()) {
			return pTemplateFrame.release();
		}
	} NxCatchAll_NoParent(__FUNCTION__);

	return NULL;
}

namespace {
	std::vector<CEmrTemplateFrameWnd*> g_allTemplateFrameWnds;
}

// (a.walling 2012-04-10 15:59) - PLID 48469 - Find any open windows for the same template ID
CEmrTemplateFrameWnd* CEmrTemplateFrameWnd::FindOpenTemplate(long nEmrTemplateID)
{
	foreach (CEmrTemplateFrameWnd* pTemplateFrameWnd, g_allTemplateFrameWnds) {
		if (!pTemplateFrameWnd || !::IsWindow(pTemplateFrameWnd->GetSafeHwnd())) {
			continue;
		}

		if (pTemplateFrameWnd->GetTemplateID() == nEmrTemplateID) {
			return pTemplateFrameWnd;
		}
	}

	return NULL;
}

IMPLEMENT_DYNAMIC(CEmrTemplateFrameWnd, CEmrFrameWnd)

// (a.walling 2012-03-02 11:52) - PLID 48469 - Constructor takes template and collection ids
CEmrTemplateFrameWnd::CEmrTemplateFrameWnd(long nEmrTemplateID, long nCollectionID)
	: CEmrFrameWnd(true)
	, m_nEmrTemplateID(nEmrTemplateID)
	, m_nNewTemplateCollectionID(nCollectionID)
{
	// (a.walling 2012-04-06 16:40) - PLID 49505 - Set a background color hint for the ribbon
	m_wndRibbonBar.SetRibbonBackgroundBaseColor(lerpColor(GetNxColor(GNC_ADMIN, 0), RGB(0xFF, 0xFF, 0xFF), 0.50));

	g_allTemplateFrameWnds.push_back(this);
}

CEmrTemplateFrameWnd::~CEmrTemplateFrameWnd()
{
	boost::remove_erase(g_allTemplateFrameWnds, this);
}


BEGIN_MESSAGE_MAP(CEmrTemplateFrameWnd, CEmrFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()

	////
	/// UI update
	//(e.lally 2012-02-29) PLID 48065 - These are specific to templates so I moved them here to their new home
	ON_UPDATE_COMMAND_UI(ID_EMR_EM_CHECKLIST, &CEmrTemplateFrameWnd::OnUpdateEmChecklist)
	ON_UPDATE_COMMAND_UI(ID_EMR_POSITION_TOPICS, &CEmrTemplateFrameWnd::OnUpdatePositionTopics)

	// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history
	ON_UPDATE_COMMAND_UI(ID_EMR_SHOW_EMN_AUDIT_HISTORY, &CEmrTemplateFrameWnd::OnUpdateShowTemplateAuditHistory)

	////
	/// Commands
	//(e.lally 2012-02-29) PLID 48065 - These are specific to templates so I moved them here to their new home
	ON_COMMAND(ID_EMR_EM_CHECKLIST, &CEmrTemplateFrameWnd::OnEmChecklist)
	ON_COMMAND(ID_EMR_POSITION_TOPICS, &CEmrTemplateFrameWnd::OnPositionTopics)

	// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history
	ON_COMMAND(ID_EMR_SHOW_EMN_AUDIT_HISTORY, &CEmrTemplateFrameWnd::OnShowTemplateAuditHistory)

END_MESSAGE_MAP()

long CEmrTemplateFrameWnd::GetTemplateID()
{
	return GetActiveEMN()->GetSafeTemplateID();
}

void CEmrTemplateFrameWnd::OnUpdateEmChecklist(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CEmrTemplateFrameWnd::OnUpdatePositionTopics(CCmdUI* pCmdUI)
{
	//(e.lally 2012-04-04) PLID 48065 - Make sure we have an active and writable EMN
	//(e.lally 2012-05-07) PLID 48951 - Renamed GetActiveWritableEMN so here we should manually check that state.
	CEMN* pEMN = GetActiveEMN();
	if (!pEMN || !pEMN->IsWritable()) {
		pCmdUI->Enable(FALSE);
	}
	else {
		pCmdUI->Enable(TRUE);
	}
}

// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history
void CEmrTemplateFrameWnd::OnUpdateShowTemplateAuditHistory(CCmdUI* pCmdUI)
{
	if (GetActiveEMN() && -1 != GetActiveEMN()->GetSafeTemplateID() && CanAccess_EMRAuditHistory()) {
		pCmdUI->Enable(TRUE);
	} else {
		pCmdUI->Enable(FALSE);
	}
}

void CEmrTemplateFrameWnd::OnEmChecklist()
{
	try {
		GetEmrTreeWnd()->OnEMChecklistSetup();
	} NxCatchAll(__FUNCTION__);
}

void CEmrTemplateFrameWnd::OnPositionTopics()
{
	try {
		GetEmrTreeWnd()->OnPositionTopics();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-03-23 15:27) - PLID 49187 - Audit history
void CEmrTemplateFrameWnd::OnShowTemplateAuditHistory()
{
	try {
		if (!CheckAccess_EMRAuditHistory()) {
			return;
		}

		CEMRAuditHistoryDlg dlg(this);
		dlg.m_nEMRTemplateID = GetActiveEMN()->GetSafeTemplateID();
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

int CEmrTemplateFrameWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEmrFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	try {
		InitializeEmrFrame();

		OnUpdateFrameTitle(FALSE);

	} NxCatchAll("CEmrTemplateFrameWnd::OnCreate");
	
	return 0;
}

// (a.walling 2012-02-28 14:53) - PLID 48451 - implement CEmrFrameWnd abstract members

void CEmrTemplateFrameWnd::InitializeRibbon()
{
	// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
	InitializeRibbon_MainPanel();
	InitializeRibbon_Edit();
	InitializeRibbon_Template();
	InitializeRibbon_MoreInfo();
	InitializeRibbon_View();
	InitializeRibbon_QAT();

	InitializeRibbonTabButtons();
}

void CEmrTemplateFrameWnd::InitializeRibbon_MainPanel() 
{
	CMFCRibbonMainPanel* pMainPanel = m_wndRibbonBar.AddMainCategory("Main", IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);

	// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
	pMainPanel->Add(new CNxRibbonButton(ID_EMR_SAVE, "Save", EmrIcons::Small::Save, EmrIcons::Large::Save));
	pMainPanel->Add(new CNxRibbonButton(ID_EMR_SAVE_AND_CLOSE, "Save and Close", EmrIcons::Small::SaveAndClose, EmrIcons::Large::SaveAndClose));
	pMainPanel->Add(new CNxRibbonButton(ID_EMR_CLOSE, "Close", EmrIcons::Small::Close, EmrIcons::Large::Close));
}

// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrTemplateFrameWnd::InitializeRibbon_View()
{
	CNxRibbonCategory* pCategory = m_wndRibbonBar.AddCategory("View", IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Panes");

		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_VIEW_TREE_PANE, "Topic List"));
		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_VIEW_CLASSIC_BUTTON_PANE, "Classic Buttons")); //(e.lally 2012-02-16) PLID 48065
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Layout");

		pPanel->Add(new CNxRibbonButton(ID_EMR_RESET_LAYOUT, "Reset Layout", EmrIcons::Small::Refresh, EmrIcons::Large::Refresh));
	}
}

// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrTemplateFrameWnd::InitializeRibbon_Edit()
{
	CNxRibbonCategory* pCategory = m_wndRibbonBar.AddCategory("Edit", IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Data");

		CNxRibbonButton* pSave = new CNxRibbonButton(ID_EMR_SAVE, "Save", EmrIcons::Small::Save, EmrIcons::Large::Save);
		pSave->SetAlwaysLargeImage(TRUE);
		pPanel->Add(pSave);
		pPanel->Add(new CNxRibbonButton(ID_EMR_SAVE_AND_CLOSE, "Save and Close", EmrIcons::Small::SaveAndClose, EmrIcons::Large::SaveAndClose));
		pPanel->Add(new CNxRibbonButton(ID_EMR_CLOSE, "Close", EmrIcons::Small::Close, EmrIcons::Large::Close));
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Topics");

		// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		CNxRibbonButton* pAddSubtopic = (new CNxRibbonButton(ID_EMR_NEW_SUBTOPIC, "Add Subtopic", EmrIcons::Small::AddSubtopic, EmrIcons::Large::AddSubtopic));
		pAddSubtopic->SetAlwaysLargeImage(TRUE);
		pPanel->Add(pAddSubtopic);
		pPanel->Add(new CNxRibbonButton(ID_EMR_NEW_TOPIC, "Add Topic", EmrIcons::Small::AddTopic, EmrIcons::Large::AddTopic));
		pPanel->Add(new CNxRibbonButton(ID_EMR_IMPORT_SUBTOPICS, "Import Subtopics", EmrIcons::Small::ImportSubtopic, EmrIcons::Large::ImportSubtopic));
		pPanel->Add(new CNxRibbonButton(ID_EMR_IMPORT_TOPICS, "Import Topics", EmrIcons::Small::AddTopic, EmrIcons::Large::ImportTopic));
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Items");

		// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		CNxRibbonButton* pAddItem = new CNxRibbonButton(ID_EMR_ADD_ITEM, "Add Item", EmrIcons::Small::Item, EmrIcons::Large::AddItem);
		pAddItem->SetAlwaysLargeImage(TRUE);
		pPanel->Add(pAddItem);
		pPanel->Add(new CNxRibbonButton(ID_EMR_ADD_IMAGE, "Add Image", EmrIcons::Small::ImageItem, EmrIcons::Large::None));
		
		// (j.jones 2013-08-07 14:37) - PLID 42958 - added ability to let another user sign an EMN
		// (though for templates this will end up being disabled)
		{
			std::auto_ptr<CNxRibbonButton> pNew(new CNxRibbonButton(ID_EMR_ADD_YOUR_SIGNATURE, "Add Signature", EmrIcons::Small::SignatureItem, EmrIcons::Large::None));

			pNew->AddSubItem(new CNxRibbonButton(ID_EMR_ADD_YOUR_SIGNATURE, "Add Your Signature", EmrIcons::Small::SignatureItem, EmrIcons::Large::None));
			pNew->AddSubItem(new CNxRibbonButton(ID_EMR_ADD_OTHER_USERS_SIGNATURE, "Add Another User's Signature", EmrIcons::Small::SignatureItem, EmrIcons::Large::None));

			pPanel->Add(pNew.release());
		}

		pPanel->Add(new CNxRibbonButton(ID_EMR_ADD_TEXT_MACRO, "Add Text Macro", EmrIcons::Small::TextItem, EmrIcons::Large::None));
	}
}

// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrTemplateFrameWnd::InitializeRibbon_Template()
{
	CNxRibbonCategory* pCategory = m_wndRibbonBar.AddCategory("Template", IDB_EMRFRAME_SMALL, IDB_EMRFRAME_LARGE);

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Access");

		// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		CNxRibbonButton* pWriteAccessButton = new CNxRibbonButton(ID_EMR_EMN_WRITE_ACCESS, "Writable", EmrIcons::Small::WriteAccess, EmrIcons::Large::WriteAccess);
		pWriteAccessButton->SetAlwaysLargeImage(TRUE);
		pPanel->Add(pWriteAccessButton);
	}

	// (a.walling 2012-06-28 17:13) - PLID 51277 - More Info button
	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("View");
		pPanel->Add(new CNxRibbonButton(ID_EMR_EMN_MORE_INFO, "More Info Tab", EmrIcons::Small::MoreInfo, EmrIcons::Large::MoreInfo));
		//TES 2/13/2014 - PLID 60749 - Codes button (uses same icon)
		pPanel->Add(new CNxRibbonButton(ID_EMR_EMN_CODES, "Codes Tab", EmrIcons::Small::MoreInfo, EmrIcons::Large::MoreInfo));
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Status");

		pPanel->SetJustifyColumns(TRUE);
		pPanel->SetCenterColumnVert(TRUE);
		
		pPanel->Add(new CNxRibbonAutoComboBox(ID_EMR_EMN_CHART, FALSE, 75, "Chart"));
		pPanel->Add(new CNxRibbonAutoComboBox(ID_EMR_EMN_CATEGORY, FALSE, 75, "Category"));
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Info");

		pPanel->SetCenterColumnVert(TRUE);

		// (a.walling 2012-06-11 12:55) - PLID 50937 - Support line breaks
		pPanel->Add(new CMFCRibbonLabel("Template Description"));
		CNxRibbonEditEx* pEmnDescription = new CNxRibbonEditEx(ID_EMR_EMN_DESCRIPTION, 200, "");
		pEmnDescription->SetAllowLineBreaks();
		pEmnDescription->SetLimitText(200);
		pPanel->Add(pEmnDescription);
	}

	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Auditing");

			// (a.walling 2012-05-01 12:05) - PLID 50094 - CNxRibbonButton works around issues in CMFCRibbonButton wrt showing a popup menu
		pPanel->Add(new CNxRibbonButton(ID_EMR_SHOW_EMN_AUDIT_HISTORY, "Template history", EmrIcons::Small::Command, EmrIcons::Large::Command));
	}

	//TES 1/17/2014 - PLID 60397 - Option to hide the EMN title on the preview pane, per template
	{
		CNxRibbonPanel* pPanel = pCategory->AddPanel("Preview Pane Overrides");

		pPanel->Add(new CNxRibbonCheckBox(ID_EMR_TEMPLATE_HIDE_TITLE, "Hide EMN Title"));
	}
}

// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrTemplateFrameWnd::InitializeRibbon_MoreInfo()
{
	// we are nihilists
}

// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
void CEmrTemplateFrameWnd::InitializeRibbon_QAT()
{
	// Add quick access toolbar commands:
	CList<UINT, UINT> lstQATCmds;

	lstQATCmds.AddTail(ID_EMR_SAVE);

	m_wndRibbonBar.SetQuickAccessCommands(lstQATCmds);
}

void CEmrTemplateFrameWnd::InitializeStatusBar()
{
	// (a.walling 2012-06-06 08:46) - PLID 50913 - Organizing the ribbon
	m_wndStatusBar.AddExtendedElement(CreateNavigationButtonsGroup().release(), "Topic Navigation");
}

void CEmrTemplateFrameWnd::InitializePanes()
{	
	m_emrTreePane.CreateEx(		WS_EX_CONTROLPARENT, "Topic List",			this, CRect(0, 0, 250, 100), TRUE, ID_EMR_VIEW_TREE_PANE,				WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT  |  CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE); 
	//(e.lally 2012-02-16) PLID 48065 - Add button pane from our "classic" interface
	m_emrClassicButtonPane.CreateEx(WS_EX_CONTROLPARENT, "Classic Buttons",	this, CRect(0, 0, 800, 70),  TRUE, ID_EMR_VIEW_CLASSIC_BUTTON_PANE,		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI | CBRS_SIZE_FIXED, AFX_CBRS_REGULAR_TABS, AFX_DEFAULT_DOCKING_PANE_STYLE);

	m_emrTreePane.EnableDocking(CBRS_ALIGN_ANY);
	m_emrClassicButtonPane.EnableDocking(CBRS_ALIGN_ANY); //(e.lally 2012-02-16) PLID 48065

	EnableDocking(CBRS_ALIGN_ANY);
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	DockPane(&m_emrTreePane);
	DockPane(&m_emrClassicButtonPane); //(e.lally 2012-02-16) PLID 48065

	// (a.walling 2012-07-16 09:30) - PLID 51548 - Default layout, tree should be pinned
	//m_emrTreePane.SetAutoHideMode(TRUE, CBRS_LEFT, NULL, FALSE);

	//(e.lally 2012-03-15) PLID 48915 - Set a minimum size so the panes can always be found. 
	//	The main goal is to avoid allowing the user to shrink them down to a 0x0 pane, 
	//	because it is hard to find the sizable border to make it visible again. 
	//	But for some, I made it large enough to see at least a partial icon.
	m_emrTreePane.SetMinSize(CSize(40, 65));
	m_emrClassicButtonPane.SetMinSize(CSize(75, 55));
}

CString CEmrTemplateFrameWnd::GenerateTitleBarText()
{
	CString str = "Template - ";
	if (CEMN* pEMN = GetActiveEMN()) {
		str.Append(pEMN->GetDescription());
	}
	str.Append(" - NexEMR");
	str.Replace("  ", " ");

	return str;
}

// (a.walling 2012-03-02 16:30) - PLID 48598 - return our layout section name
CString CEmrTemplateFrameWnd::GetLayoutSectionName()
{
	return "EMRTemplate";
}

// (a.walling 2012-03-05 12:33) - PLID 46644 - Handle closing and saving / prompting re the template
void CEmrTemplateFrameWnd::OnClose()
{
	try {
		if(GetEmrTreeWnd()->IsEMRUnsaved()) {
			int nResult = MessageBox("You have made changes to the template, do you wish to save these changes?\n\n"
				"'Yes' will save the changes and close the template.\n"
				"'No' will discard the changes and close the template.\n"
				"'Cancel' will cancel closing the template.", "Practice", MB_ICONEXCLAMATION|MB_YESNOCANCEL);
			if(nResult == IDCANCEL) {
				return;
			} else if(nResult == IDNO) {
				if(IDYES == MessageBox("All of your template changes will be unrecoverable! Are you sure you wish to close without saving?",
					"Practice", MB_ICONEXCLAMATION|MB_YESNO)) {

					//they really want to close without saving

					CEmrFrameWnd::OnClose();
				}
				
				return;
			}
		}

		if (GetEmrTreeWnd()->IsEMREMChecklistDlgVisible()) {
			MessageBox("Please close the E/M Checklist Setup dialog before dismissing this template.", "Practice", MB_OK | MB_ICONSTOP);
			GetEmrTreeWnd()->BringEMREMChecklistDlgToTop();
			return;
		}

		// (j.jones 2012-10-12 09:09) - PLID 52820 - pass in TRUE for closing
		if(GetEmrEditorBase()->Save(TRUE, TRUE) != essSuccess) {
			return;
		}
		
		CEmrFrameWnd::OnClose();
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2012-04-06 13:19) - PLID 48990 - Ensure the most common lists are cached
void CEmrTemplateFrameWnd::EnsureCachedData()
{
	// note this will not bother loading a tables if it is already loaded
	using namespace PracData;
	CacheLoader()
		.Load<EmnTabChartsT>()
		.Load<EmnTabCategoriesT>()
		.Load<EmnTabChartCategoryLinkT>()
		.Run();
}

// (j.armen 2012-06-11 17:09) - PLID 48808 - Handle the database reconnecting 
//	by releasing and attempting to obtain a new write token
void CEmrTemplateFrameWnd::OnDatabaseReconnect()
{
	foreach(CEMN* pEMN, GetEmrTreeWnd()->GetEMR()->GetAllEMNs()) {
		// (a.walling 2013-10-01 10:11) - PLID 58827 - Skip new templates
		if (!pEMN || (pEMN->GetID() == -1) || !pEMN->IsWritable()) {
			continue;
		}

		pEMN->ReleaseWriteToken();
		GetEmrTreeWnd()->PostMessage(NXM_TRY_ACQUIRE_WRITE_ACCESS, (WPARAM)pEMN->GetID());
	}
}

// (j.armen 2012-06-11 17:10) - PLID 48808 - Iterate through all frames
void CEmrTemplateFrameWnd::HandleDatabaseReconnect()
{
	foreach(CEmrTemplateFrameWnd* pFrame, g_allTemplateFrameWnds)
		pFrame->OnDatabaseReconnect();
}
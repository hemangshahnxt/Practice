#include "stdafx.h"
#include "EmrClassicButtonPane.h"
#include "EmrFrameWnd.h"
#include "EmrRc.h"
#include "EmrRc2.h"	// (j.dinatale 2012-06-25 16:36) - PLID 48065 - need this for some of the standard menu IDs
#include "EMR.h"

//(e.lally 2012-02-16) PLID 48065 - Created. Moved some of the old EmrTreeWnd code here.

IMPLEMENT_DYNCREATE(CEMRClassicButtonPane, CEmrPane)

BEGIN_MESSAGE_MAP(CEMRClassicButtonPane, CEmrPane)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()

	////
	/// UI State overrides
	ON_UPDATE_COMMAND_UI(ID_EMR_EDIT_MODE, &CEMRClassicButtonPane::OnUpdateClassicBtnEditMode)

	////
	/// UI Command overrides
	ON_COMMAND(ID_EMR_ADD_TOPICS_BTN_MENU, &CEMRClassicButtonPane::OnAddTopicsBtnMenu)
	ON_COMMAND(ID_EMR_PROBLEM_LIST_BTN_MENU, &CEMRClassicButtonPane::OnProblemListBtnMenu)

END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEMRClassicButtonPane, CEmrPane)
END_EVENTSINK_MAP()

//(e.lally 2012-02-16) PLID 48065 - We only want to change the text on our specific button, so handle the UI state here instead
void CEMRClassicButtonPane::OnUpdateClassicBtnEditMode(CCmdUI* pCmdUI)
{

	//(e.lally 2012-04-04) PLID 48065 - Make sure we have an active and writable EMN	
	if (!GetEmrFrameWnd()) {
		pCmdUI->Enable(FALSE);
	}
	else {
		//(e.lally 2012-05-07) PLID 48951 - ensure active EMN can be edited
		pCmdUI->Enable(GetEmrFrameWnd()->GetActiveEditableEMN() ? TRUE : FALSE);
	}
	if(GetEmrFrameWnd()->IsEditMode()){
		pCmdUI->SetText("Disable Edit Mode");
	}
	else {
		pCmdUI->SetText("Enable Edit Mode");
	}
}


//(e.lally 2012-02-16) PLID 48065 - We want to specifically do a popup menu for the Add Topics button in just this pane, so declare it here.
void CEMRClassicButtonPane::OnAddTopicsBtnMenu()
{
	try {
		//pop-out a menu of options
		//-1 - Add new topic
		//-2 - Add new subtopic
		//-3 - Import as topics
		//-4 - Import as subtopics

		if (!GetEmrFrameWnd() || !GetEmrFrameWnd()->GetEmrTreeWnd()) {
			return;
		}

#pragma TODO("e.lally - How do we determine if a topic is selected?")
		//Is a topic selected?
		bool bIsTopicSelected = false;
		CEmrTopicView* pTopicView = GetEmrFrameWnd()->GetActiveEmrTopicView();
		if (pTopicView) {
			if(pTopicView->GetActiveTopic() != NULL){
				bIsTopicSelected = true;
			}
		}
		

		// (a.walling 2012-06-15 08:34) - PLID 46625 - Use CNxMenu
		// (j.dinatale 2012-06-25 16:37) - PLID 48065 - changed the IDs to be those found in EmrRc2.h
		CNxMenu mnu;
		mnu.CreatePopupMenu();
		mnu.InsertMenu(0, MF_ENABLED|MF_BYPOSITION, ID_EMR_NEW_TOPIC, "Add New Topic");
		mnu.InsertMenu(1, (bIsTopicSelected?MF_ENABLED:MF_DISABLED|MF_GRAYED)|MF_BYPOSITION, ID_EMR_NEW_SUBTOPIC, "Add New Subtopic");
		mnu.InsertMenu(2, MF_ENABLED|MF_BYPOSITION, ID_EMR_IMPORT_TOPICS, "Import Topics");
		mnu.InsertMenu(3, (bIsTopicSelected?MF_ENABLED:MF_DISABLED|MF_GRAYED)|MF_BYPOSITION, ID_EMR_IMPORT_SUBTOPICS, "Import Subtopics");

		CRect rc = GetControlRect(ID_EMR_ADD_TOPICS_BTN_MENU);
		ClientToScreen(&rc);

		// Pop up the menu to the right of the button
		int nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, rc.right, rc.top, this, NULL);
		switch(nCmdId) {
			case ID_EMR_NEW_TOPIC:	//Add new topic
				GetEmrTreeWnd()->AddNewTopic(FALSE);
				break;
			case ID_EMR_NEW_SUBTOPIC:	//Add new subtopic
				GetEmrTreeWnd()->AddNewTopic(TRUE);
				break;
			case ID_EMR_IMPORT_TOPICS:	//Import as topics
				GetEmrTreeWnd()->ImportTopics(FALSE);
				break;
			case ID_EMR_IMPORT_SUBTOPICS:	//Import as subtopics
				GetEmrTreeWnd()->ImportTopics(TRUE);
				break;
		}
		mnu.DestroyMenu();

	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-02-16) PLID 48065 - We want to specifically do a popup menu for the Problem list button in just this pane, so declare it here.
void CEMRClassicButtonPane::OnProblemListBtnMenu()
{
	try {
		//pop-out a menu of options
		//-1 - Add new topic
		//-2 - Add new subtopic
		//-3 - Import as topics
		//-4 - Import as subtopics

		if (!GetTopLevelFrame()) {
			return;
		}

		// (a.walling 2011-11-30 08:39) - PLID 46625 - Use CNxMenu
		CNxMenu mnu;
		mnu.CreatePopupMenu();
		// (a.walling 2012-06-11 08:53) - PLID 50894 - Problems
		mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, ID_EMR_SHOW_PROBLEMS, "View EMR Problems");
		mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, ID_EMR_ADD_NEW_PROBLEM_TO_EMR, "Link with New Problem");
		// (c.haag 2009-05-26 17:42) - PLID 34249 - Added an option for adding an existing EMR problems
		mnu.AppendMenu(MF_ENABLED|MF_BYPOSITION, ID_EMR_LINK_PROBLEM_TO_EMR, "Link with Existing Problems");

		CRect rc = GetControlRect(ID_EMR_PROBLEM_LIST_BTN_MENU);
		ClientToScreen(&rc);

		// (a.walling 2012-02-28 14:53) - PLID 48451 - Post the command to the top level frame

		// Pop up the menu to the right of the button
		mnu.DoPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN, rc.right, rc.top, GetTopLevelFrame(), NULL);

	} NxCatchAll(__FUNCTION__);
}


int CEMRClassicButtonPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEmrPane::OnCreate(lpCreateStruct) == -1)
		return -1;

	try {
		CRect rcClient;
		GetClientRect(&rcClient);

		CreateControls();

		return 0;
	} NxCatchAll(__FUNCTION__);

	return -1;
}


CEMRClassicButtonPane::CEMRClassicButtonPane()
	: m_pButtonFont(NULL) // (a.walling 2014-04-29 16:55) - PLID 61966 - Init m_pButtonFont to NULL in constructor
{
}

//(e.lally 2012-02-16) PLID 48065
CEMRClassicButtonPane::~CEMRClassicButtonPane()
{
	if(m_pButtonFont) {
		delete m_pButtonFont;
		m_pButtonFont = NULL;
	}
}


//(e.lally 2012-02-16) PLID 48065 - Moved from EmrTreeWnd and updated to handle new structure.
void CEMRClassicButtonPane::CreateControls()
{
	try {
		CEmrFrameWnd* pEmrFrameWnd = GetEmrFrameWnd();
		if(pEmrFrameWnd == NULL){
			ThrowNxException("Could not get EMR Frame Window");
		}
		m_bIsTemplate = pEmrFrameWnd->IsTemplate();

		if(!m_pButtonFont) {
			LOGFONT lf;
			if(SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, 0)) {
				m_pButtonFont = new CFont;
				m_pButtonFont->CreateFontIndirect(&lf);
			}
		}

		// (c.haag 2007-08-30 15:57) - PLID 27256 - Don't create the "Add New EMN" button on a template
		// (c.haag 2008-04-25 15:36) - PLID 29796 - NxIconified applicable buttons
		// (j.jones 2009-03-03 11:54) - PLID 33308 - renamed to just "New EMN"
		if(!m_bIsTemplate && !m_btnNewEmn.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_NEW_EMN);
			m_btnNewEmn.Create("New EMN", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_NEW_EMN);
			m_btnNewEmn.SetFont(m_pButtonFont);
			m_btnNewEmn.SetTextColor(RGB(0,0,255));
			m_btnNewEmn.AutoSet(NXB_NEW);
		}
		if(!m_btnNextNode.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_NEXT_TOPIC);
			m_btnNextNode.Create("", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_NEXT_TOPIC);
			m_btnNextNode.AutoSet(NXB_RIGHT);
		}
		if(!m_btnPreviousNode.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_PREVIOUS_TOPIC);
			m_btnPreviousNode.Create("", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_PREVIOUS_TOPIC);
			m_btnPreviousNode.AutoSet(NXB_LEFT);
		}
		if(!m_btnEditMode.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_EDIT_MODE);
			m_btnEditMode.Create("Edit Mode", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_AUTOCHECKBOX|BS_PUSHLIKE, rBtn, this, ID_EMR_EDIT_MODE);
			m_btnEditMode.SetFont(m_pButtonFont);
		}
		if(!m_btnAddTopics.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_ADD_TOPICS_BTN_MENU);
			// (c.haag 2008-04-28 09:34) - PLID 29796 - Renamed from "Add Topics/Subtopics..." to "Add New Topics..." to make room for the icon
			// (j.jones 2008-07-09 09:52) - PLID 24624 - removed "..." to give us more space, no other buttons here had it anyways
			// (j.jones 2008-07-17 13:23) - PLID 30729 - renamed to "Add Topics"
			m_btnAddTopics.Create("Add Topics", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_ADD_TOPICS_BTN_MENU);
			m_btnAddTopics.SetFont(m_pButtonFont);
			m_btnAddTopics.SetTextColor(RGB(0,0,255));
			m_btnAddTopics.AutoSet(NXB_NEW);
		}
		if(!m_btnAddItemToTopic.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_ADD_ITEM);
			m_btnAddItemToTopic.Create("Add Item", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_ADD_ITEM);
			m_btnAddItemToTopic.SetFont(m_pButtonFont);
			m_btnAddItemToTopic.SetTextColor(RGB(0,0,255));
			m_btnAddItemToTopic.AutoSet(NXB_NEW);
		}
		// (c.haag 2007-08-30 15:58) - PLID 27256 - Do not create the "Create ToDo Task" button on a template
		// (c.haag 2008-04-28 09:32) - PLID 29796 - Renamed from "Create ToDo Task" to "New ToDo Task" to make room for the button icon
		// (j.jones 2009-03-03 11:54) - PLID 33308 - renamed to just "New ToDo"
		if(!m_bIsTemplate && !m_btnCreateToDo.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_NEW_TODO);
			m_btnCreateToDo.Create("New ToDo", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_NEW_TODO);
			m_btnCreateToDo.SetFont(m_pButtonFont);
			m_btnCreateToDo.SetTextColor(RGB(0,0,255));
			m_btnCreateToDo.AutoSet(NXB_NEW);
		}
		// (c.haag 2007-08-30 15:58) - PLID 27256 - Do not create the "Record Audio" button on a template
		if (!m_bIsTemplate && !m_btnRecordAudio.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_NEW_RECORDING);
			// (z.manning 2009-08-12 12:23) - PLID 27694 - Renamed to just "Audio"
			m_btnRecordAudio.Create("Audio", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_NEW_RECORDING);
			m_btnRecordAudio.SetFont(m_pButtonFont);
			m_btnRecordAudio.SetTextColor(RGB(0,0,255));
			// (c.haag 2008-04-29 16:30) - PLID 29796 - Established an icon for invoking the audio recorder dialog
			m_btnRecordAudio.SetIcon(IDI_GO_RECORD_DLG);
		}
		// (j.jones 2009-03-03 12:05) - PLID 33308 - added an "e-Rx" button, not on templates, for E-Prescribing
		// (j.jones 2009-03-24 14:44) - PLID 33652 - supported the NewCrop license
		// (j.jones 2013-03-01 09:15) - PLID 52818 - allowed this for our E-Rx too, but not when they have no E-Rx license,
		// they would have to use the ribbon or More Info tab for the queue at that point
		// (j.jones 2013-04-03 13:54) - PLID 56069 - We no longer hide this based on the E-Rx license, it shows anytime the
		// pill bottle is shown in the ribbon, which means it is only hidden if they don't have EMR.
		if (!m_bIsTemplate && !m_btnEPrescribing.GetSafeHwnd() && g_pLicense->HasEMROrExpired(CLicense::cflrSilent) == 2) {
			CRect rBtn = GetControlRect(ID_EMR_VIEW_EPRESCRIBING);

			// (j.jones 2013-04-03 13:54) - PLID 56069 - make the name say "Meds" if they don't have
			// either NewCrop or SureScripts e-Rx license
			CLicense::EPrescribingType eRxType = g_pLicense->HasEPrescribing(CLicense::cflrSilent);		
			CString strERxName = "e-Rx";
			if(eRxType == CLicense::eptNone) {
				//call this "Meds" if no E-Rx license of any sort
				strERxName = "Meds";
			}

			m_btnEPrescribing.Create(strERxName, WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_VIEW_EPRESCRIBING);
			m_btnEPrescribing.SetFont(m_pButtonFont);
			m_btnEPrescribing.AutoSet(NXB_PILLBOTTLE);
		}
		// (j.jones 2008-07-09 08:56) - PLID 24624 - add a "Patient Summary" button, not on templates,
		// called it "Pt. Summary" at the EMR team's request
		if (!m_bIsTemplate && !m_btnPatientSummary.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_PATIENT_SUMMARY);
			m_btnPatientSummary.Create("Pt. Summary", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_PATIENT_SUMMARY);
			m_btnPatientSummary.SetFont(m_pButtonFont);
			m_btnPatientSummary.SetTextColor(RGB(0,0,255));
			// (z.manning 2009-08-12 12:35) - PLID 27694 - Removed this icon (which was debatably related in the first place)
			// to crate more room for a Save and Close button.
			//m_btnPatientSummary.AutoSet(NXB_INSPECT);
		}
		// (j.jones 2008-07-17 13:07) - PLID 30729 - add an EMR Problem List button, but just show the flag icon, no text
		if (!m_bIsTemplate && !m_btnEMRProblemList.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_PROBLEM_LIST_BTN_MENU);
			m_btnEMRProblemList.Create("", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_PROBLEM_LIST_BTN_MENU);
			m_btnEMRProblemList.SetFont(m_pButtonFont);
			m_btnEMRProblemList.SetTextColor(RGB(0,0,255));
			m_btnEMRProblemList.SetIcon(IDI_EMR_PROBLEM_EMPTY);

		}
		// (j.jones 2009-03-03 11:54) - PLID 33308 - renamed to just "Save"
#pragma TODO("Figure out which 'save' to associate this with")
		if(!m_btnSaveChanges.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_SAVE);
			m_btnSaveChanges.Create("Save", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_SAVE);
			m_btnSaveChanges.SetFont(m_pButtonFont);
			m_btnSaveChanges.SetTextColor(RGB(0,0,255));
			m_btnSaveChanges.SetIcon(IDI_SAVE);
		}
		// (z.manning 2009-08-12 14:26) - PLID 27694 - Save and Close button
		// Don't put this on templates since we already have an OK button for this.
		//(e.lally 2012-04-03) PLID 48065 - With the pane creation, we can use the save and close button again in the template editor
		if(!m_btnSaveAndClose.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_SAVE_AND_CLOSE);
			m_btnSaveAndClose.Create("Save && Close", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_SAVE_AND_CLOSE);
			m_btnSaveAndClose.SetFont(m_pButtonFont);
			m_btnSaveAndClose.SetTextColor(RGB(0,0,255));
			m_btnSaveAndClose.AutoSet(NXB_OK);
		}

		//(e.lally 2012-04-03) PLID 48065 - Added a dynamically created cancel button to the template editor mode
		if(m_bIsTemplate && !m_btnCancel.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_CLOSE);
			m_btnCancel.Create("Cancel", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_CLOSE);
			m_btnCancel.SetFont(m_pButtonFont);
			m_btnCancel.SetTextColor(RGB(255,0,0));
			m_btnCancel.AutoSet(NXB_CANCEL);
		}
#ifdef _DEBUG
		// (c.haag 2007-08-04 10:03) - PLID 26946 - Used for EMR debugging
		if(!m_btnDebug.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_DEBUG);
			m_btnDebug.Create("Debug Info", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_DEBUG);
			m_btnDebug.SetFont(m_pButtonFont);
		}
#endif
		// (c.haag 2007-08-30 16:21) - PLID 27058 - E/M Checklist
		if (m_bIsTemplate && !m_btnEMChecklist.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_EM_CHECKLIST);
			// (j.jones 2013-01-04 14:48) - PLID 28135 - changed to say E/M, and not use an ampersand
			m_btnEMChecklist.Create("E/M Checklist Setup", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_EM_CHECKLIST);
			m_btnEMChecklist.SetFont(m_pButtonFont);
			m_btnEMChecklist.SetTextColor(RGB(0,0,255));
		}
		//TES 9/9/2009 - PLID 35495 - Button to position template topics
		if(m_bIsTemplate && !m_btnPositionTopics.GetSafeHwnd()) {
			CRect rBtn = GetControlRect(ID_EMR_POSITION_TOPICS);
			m_btnPositionTopics.Create("Position Topics", WS_CHILD|WS_VISIBLE|BS_CENTER|BS_PUSHBUTTON|BS_OWNERDRAW, rBtn, this, ID_EMR_POSITION_TOPICS);
			m_btnPositionTopics.SetFont(m_pButtonFont);
			m_btnPositionTopics.AutoSet(NXB_MODIFY);
		}
	}NxCatchAll(__FUNCTION__);
}


#define HORIZ_BUFFER	5
#define VERT_BUFFER		5
#define MAX_TREE_WIDTH	200
#define ARROW_SIZE	35
//(e.lally 2012-02-16) PLID 48065 - Moved from EmrTreeWnd, mostly unchanged.
CRect CEMRClassicButtonPane::GetControlRect(UINT nID)
{
	CRect rThis;
	GetClientRect(rThis);
	
	//HORIZ_BUFFER and VERT_BUFFER on all sides.  The right side of the tree is 20% of the width, or MAX_TREE_WIDTH, whichever is
	//smaller. HORIZ_BUFFER between that and the topic area.  The top of the buttons are at 6% of the height, VERT_BUFFER between 
	//them and the tree/topic. The buttons are each 8% of the width, but at least 130 pixels, The Edit Mode button is left-aligned 
	//with the tree, the New EMN button is HORIZ_BUFFER to the right of that, the Add Topics button is flush to the right of that, 
	//and the Add Item To Topic is flush to the right of that.  The Next and Previous buttons are 20x20, right-aligned with the
	//topic area, and flush against the bottom.  The Save Changes button is the same width as the others, top-aligned with them,
	//and HORIZ_BUFFER to the left of the Previous button.

	//(e.lally 2012-02-16) PLID 48065 - We now top align
	int nButtonTop = 0 +VERT_BUFFER;//(int)(rThis.Height()*0.94);
	int nButtonBottom = rThis.Height()-VERT_BUFFER;
	CRect rControl;

	if(rThis.Height() <= rThis.Width()){
		// (c.haag 2006-08-08 10:56) - PLID 19805 - We now scale the buttons according to the window size.
		// The old way of doing things is commented out for posterity.
		// (j.jones 2008-07-09 09:45) - PLID 24624 - I shrank btn_space and topic_btn_width,
		// added debug_btn_width, add_item_btn_width, and save_button_incr,
		// and slightly shrunk btn_width, in order to fit an additional button for the patient summary
		// (j.jones 2008-07-17 13:09) - PLID 30729 - added problem_btn_width, which forced me to
		// define most remaining button widths in order to properly fit the text on the screen
		// (j.jones 2009-03-03 11:57) - PLID 33308 - shrunk more buttons to add the e-Rx button
		// (z.manning 2009-08-12 16:43) - PLID 27694 - shrunk more buttons to add the Save and Close button
		const float btn_space = 0.0012f;
		//(e.lally 2012-04-04) PLID 48065 - In the template editor, the buttons can and should be increased in size to fill the empty space
		//This float give the percent to multiply by. The 1.0 meaning keep it at 100% of the original size and 1.1 being 110% of the original size.
		float template_expansion_factor = 1.0f;
	#ifdef _DEBUG // (c.haag 2007-08-04 10:06) - PLID 26946 - If we're in debug mode, we will have an additional button
		const float btn_width = 0.097f;
		const float btn_pivot = 0.0057f + btn_width + btn_space;	
		const float new_emn_btn_width = 0.086f;
		const float add_item_btn_width = 0.083f;
		const float topic_btn_width = 0.093f;
		const float todo_btn_width = 0.088f;
		const float audio_btn_width = 0.034f;
		const float checklist_btn_width = 0.12f;
		const float erx_btn_width = 0.060f;
		const float summary_btn_width = 0.074f;	
		const float problem_btn_width = 0.037f;
		const float save_btn_width = 0.068f;
		const float save_and_close_button_incr = 1.09f;
		//TES 9/9/2009 - PLID 35495 - Button to position spawned topics on templates
		const float position_topics_btn_width = 0.12f;
		//(e.lally 2012-04-03) PLID 48065 - Dynamic cancel and template save & close buttons
		const float cancel_btn_width = 0.11f;
		const float template_save_and_close_btn_width = 0.13f; //Only to be used in the template editor
	#else
		const float btn_width = 0.1f;
		const float btn_pivot = 0.0057f;	
		const float new_emn_btn_width = 0.0928f;
		const float add_item_btn_width = 0.092f;
		const float topic_btn_width = 0.100f;
		const float todo_btn_width = 0.097f;
		const float audio_btn_width = 0.070f;
		const float checklist_btn_width = 0.12f;
		const float erx_btn_width = 0.070f;
		const float summary_btn_width = 0.082f;
		const float problem_btn_width = 0.04f;
		const float save_btn_width = 0.070f;
		const float save_and_close_button_incr = 1.1f;
		//TES 9/9/2009 - PLID 35495 - Button to position spawned topics on templates
		const float position_topics_btn_width = 0.12f;
		//(e.lally 2012-04-03) PLID 48065 - Dynamic cancel and template save & close buttons
		const float cancel_btn_width = 0.098f;
		const float template_save_and_close_btn_width = 0.11f; //Only to be used in the template editor

		if(m_bIsTemplate){
			//(e.lally 2012-04-04) PLID 48065 - In debug mode we have an extra button, but in release mode we
			//need to expand the button sizes to fill the empty space.
			template_expansion_factor = 1.224f;
		}
	#endif

		const float btn_lastbuttonpivot = 0.993f;
		const float btn_nextprevwidth = 0.03f;



		switch(nID) {
	#ifdef _DEBUG // (c.haag 2007-08-04 10:06) - PLID 26946 - For debugging
		case ID_EMR_DEBUG:
			rControl.SetRect((int)(rThis.Width() * ( btn_pivot - (btn_width + btn_space) )),
				nButtonTop, 
				(int)(rThis.Width() * (btn_pivot - (btn_width + btn_space) + btn_width)),
				nButtonBottom);
			break;
	#endif
		case ID_EMR_EDIT_MODE:
			//nButtonWidth, nButtonTop to nButtonBottom, left aligned with the tree.
			{
				rControl.SetRect((int)(rThis.Width() * btn_pivot), nButtonTop, (int)(rThis.Width() * (btn_pivot + btn_width)), nButtonBottom);
				//rControl.SetRect(HORIZ_BUFFER, nButtonTop, HORIZ_BUFFER+nButtonWidth, nButtonBottom);
			}
			break;

		case ID_EMR_NEW_EMN:
		case ID_EMR_ADD_TOPICS_BTN_MENU:
		case ID_EMR_ADD_ITEM:
		case ID_EMR_NEW_TODO:
		case ID_EMR_NEW_RECORDING:
		case ID_EMR_EM_CHECKLIST:
		case ID_EMR_SAVE:
		case ID_EMR_PATIENT_SUMMARY:	// (j.jones 2008-07-09 09:00) - PLID 24624
		case ID_EMR_PROBLEM_LIST_BTN_MENU:	// (j.jones 2008-07-17 13:09) - PLID 30729
		case ID_EMR_VIEW_EPRESCRIBING:	// (j.jones 2009-03-03 12:04) - PLID 33308
		case ID_EMR_POSITION_TOPICS:	////TES 9/9/2009 - PLID 35495
		case ID_EMR_SAVE_AND_CLOSE: // (z.manning 2009-08-12 14:27) - PLID 27694 - Added Save & Close button
		case ID_EMR_CLOSE:			//(e.lally 2012-04-03) PLID 48065 - Dynamic cancel button
			{
				//(e.lally 2012-04-03) PLID 48065 - Special handling to align the save and close button from the right in a patient EMN
				if(nID == ID_EMR_SAVE_AND_CLOSE && !m_bIsTemplate){
					//nButtonWidth, nButtonTop to nButtonBottom, HORIZ_BEFFER left of ID_EMR_PREVIOUS_TOPIC
					int x1 = (int)(rThis.Width() * (btn_lastbuttonpivot - 2 * (btn_nextprevwidth - btn_space) - btn_width * save_and_close_button_incr ));
					int x2 = (int)(rThis.Width() * (btn_lastbuttonpivot - 2 * (btn_nextprevwidth + btn_space)));
					rControl.SetRect(x1, nButtonTop, x2, nButtonBottom);
					break;
				}
				// (c.haag 2007-08-30 16:00) - PLID 27256 - If m_bIsTemplate is TRUE, then we will not be using
				// all the available buttons. As such, we need to reorganize how we calculate the rectangle of the
				// button being asked for. We now maintain a map of control ID's to button sizes, as well as an array
				// that tells us the appearance order of the buttons. 
				// (c.haag 2007-08-30 16:24) - PLID 27058 - Added support for the E/M Checklist button for templates
				//
				// Populate the size map. All button sizes should be the same regardless of whether this is a template
				//
				CMap<long,long,CSize,CSize&> mapBtnSizes;
				const int h = nButtonBottom - nButtonTop; // h is always the button height
				mapBtnSizes.SetAt(ID_EMR_NEW_EMN, CSize((int)(rThis.Width() * new_emn_btn_width * template_expansion_factor), h));
				mapBtnSizes.SetAt(ID_EMR_ADD_TOPICS_BTN_MENU, CSize((int)(rThis.Width() * topic_btn_width * template_expansion_factor), h));
				mapBtnSizes.SetAt(ID_EMR_ADD_ITEM, CSize((int)(rThis.Width() * add_item_btn_width * template_expansion_factor), h));
				mapBtnSizes.SetAt(ID_EMR_NEW_TODO, CSize((int)(rThis.Width() * todo_btn_width), h));
				mapBtnSizes.SetAt(ID_EMR_NEW_RECORDING, CSize((int)(rThis.Width() * audio_btn_width), h));
				mapBtnSizes.SetAt(ID_EMR_EM_CHECKLIST, CSize((int)(rThis.Width() * checklist_btn_width * template_expansion_factor), h));
				mapBtnSizes.SetAt(ID_EMR_VIEW_EPRESCRIBING, CSize((int)(rThis.Width() * erx_btn_width), h));
				mapBtnSizes.SetAt(ID_EMR_PATIENT_SUMMARY, CSize((int)(rThis.Width() * summary_btn_width), h));
				mapBtnSizes.SetAt(ID_EMR_PROBLEM_LIST_BTN_MENU, CSize((int)(rThis.Width() * problem_btn_width), h));
				mapBtnSizes.SetAt(ID_EMR_SAVE, CSize((int)(rThis.Width() * save_btn_width * template_expansion_factor), h));
				mapBtnSizes.SetAt(ID_EMR_POSITION_TOPICS, CSize((int)(rThis.Width() * position_topics_btn_width), h));
				//(e.lally 2012-04-03) PLID 48065 - Position cancel button the same way we do all the others
				mapBtnSizes.SetAt(ID_EMR_CLOSE, CSize((int)(rThis.Width() * cancel_btn_width), h));
				//(e.lally 2012-04-03) PLID 48065 - We know we are in the template editor and can use this calculation here
				mapBtnSizes.SetAt(ID_EMR_SAVE_AND_CLOSE, CSize((int)(rThis.Width() * template_save_and_close_btn_width * template_expansion_factor), h));
				
				//
				// Populate the index map. This will be different for templates versus non-templates.
				//
				CArray<unsigned int,long> aBtnIndices;
				if (m_bIsTemplate) {
					aBtnIndices.Add(ID_EMR_ADD_TOPICS_BTN_MENU);
					aBtnIndices.Add(ID_EMR_ADD_ITEM);
					aBtnIndices.Add(ID_EMR_EM_CHECKLIST);
					aBtnIndices.Add(ID_EMR_POSITION_TOPICS);
					//(e.lally 2012-04-03) PLID 48065 - Added Save, Save & Close, and Cancel buttons to the template editor
					aBtnIndices.Add(ID_EMR_SAVE);
					aBtnIndices.Add(ID_EMR_SAVE_AND_CLOSE);
					aBtnIndices.Add(ID_EMR_CLOSE);
				} else {
					aBtnIndices.Add(ID_EMR_NEW_EMN);
					aBtnIndices.Add(ID_EMR_ADD_TOPICS_BTN_MENU);
					aBtnIndices.Add(ID_EMR_ADD_ITEM);
					aBtnIndices.Add(ID_EMR_NEW_TODO);
					aBtnIndices.Add(ID_EMR_NEW_RECORDING);
					aBtnIndices.Add(ID_EMR_VIEW_EPRESCRIBING);
					aBtnIndices.Add(ID_EMR_PATIENT_SUMMARY);
					aBtnIndices.Add(ID_EMR_PROBLEM_LIST_BTN_MENU);
					aBtnIndices.Add(ID_EMR_SAVE);
				}
				//
				// Figure out the index of the requested button
				//
				long nBtnIndex = -1;
				int i;
				for (i=0; i < aBtnIndices.GetSize() && -1 == nBtnIndex; i++) {
					if (aBtnIndices[i] == nID) {
						nBtnIndex = i;
					}
				}
				if (-1 == nBtnIndex) {
					ASSERT(FALSE); // We should never get here
					return CRect(0,0,0,0);
				} else {
					// We found the index. Now, given the index and button sizes, we can calculate
					// the position of the button
					int x = (int)((float)rThis.Width() * (btn_pivot + btn_width + btn_space));
					CSize s;
					for (i=0; i < nBtnIndex; i++) {
						mapBtnSizes.Lookup(aBtnIndices[i], s);
						x += s.cx + (int)((float)rThis.Width() * btn_space);
					}
					// Now we have the position of the button. We're ready to set the control rectangle
					mapBtnSizes.Lookup(aBtnIndices[i], s);
					rControl.SetRect(x, nButtonTop, x + s.cx, nButtonTop + s.cy);
				}
			}
			break;

		case ID_EMR_NEXT_TOPIC:
			//ARROW_SIZE wide, nButtonTop to nButtonBottom height, 
			//horizontally flush with ID_EMR_SAVE, vertically flush with right side of tree
			{
				rControl.SetRect((int)(rThis.Width() * (btn_lastbuttonpivot - btn_nextprevwidth)), nButtonTop, (int)(rThis.Width() * btn_lastbuttonpivot), nButtonBottom);
			}
			break;
		case ID_EMR_PREVIOUS_TOPIC:
			//ARROW_SIZE wide, nButtonTop to nButtonBottom height, 
			//horizontally flush with ID_EMR_SAVE, vertically flush with ID_EMR_NEXT_TOPIC
			{
				rControl.SetRect((int)(rThis.Width() * (btn_lastbuttonpivot - btn_nextprevwidth * 2 - btn_space)), nButtonTop, (int)(rThis.Width() * (btn_lastbuttonpivot - btn_nextprevwidth - btn_space)), nButtonBottom);
			}
			break;
			
		}
	}
	else {
		//******************We are taller than we are wide**************************
		//(e.lally 2012-02-28) PLID 48065 - Added support for left/right docking where the pane is taller than it is wide.

		CMap<long,long,int,int> mapBtnPosition;

		int nPos =0;
		//(e.lally 2012-02-29) PLID 48065 - Took into account which buttons are visible in the template editor vs patient chart
		mapBtnPosition.SetAt(ID_EMR_EDIT_MODE, nPos++);
		if (!m_bIsTemplate) {
			mapBtnPosition.SetAt(ID_EMR_NEW_EMN, nPos++);
		}
		mapBtnPosition.SetAt(ID_EMR_ADD_TOPICS_BTN_MENU, nPos++);
		mapBtnPosition.SetAt(ID_EMR_ADD_ITEM, nPos++);
		if (!m_bIsTemplate) {
			mapBtnPosition.SetAt(ID_EMR_NEW_TODO, nPos++);
			mapBtnPosition.SetAt(ID_EMR_NEW_RECORDING, nPos++);
			mapBtnPosition.SetAt(ID_EMR_VIEW_EPRESCRIBING, nPos++);
			mapBtnPosition.SetAt(ID_EMR_PATIENT_SUMMARY, nPos++);
			mapBtnPosition.SetAt(ID_EMR_PROBLEM_LIST_BTN_MENU, nPos++);
		}
		else {
			mapBtnPosition.SetAt(ID_EMR_EM_CHECKLIST, nPos++);
			mapBtnPosition.SetAt(ID_EMR_POSITION_TOPICS, nPos++);
		}
		mapBtnPosition.SetAt(ID_EMR_SAVE, nPos++);
		//(e.lally 2012-04-03) PLID 48065 - Show the Save & Close button always
		mapBtnPosition.SetAt(ID_EMR_SAVE_AND_CLOSE, nPos++);
		//(e.lally 2012-04-03) PLID 48065 - Show a cancel button on the template editor
		if(m_bIsTemplate) {
			mapBtnPosition.SetAt(ID_EMR_CLOSE, nPos++);
		}

		mapBtnPosition.SetAt(ID_EMR_PREVIOUS_TOPIC, nPos);//Notice we don't increment here, we're sharing a v-slot
		mapBtnPosition.SetAt(ID_EMR_NEXT_TOPIC, nPos++);

#ifdef _DEBUG
		mapBtnPosition.SetAt(ID_EMR_DEBUG, nPos++);//This should always be last
#endif

		int nButtonLeft = rThis.left + HORIZ_BUFFER;
		int nButtonRight = rThis.right - HORIZ_BUFFER;
		CRect rPane = CRect(nButtonLeft, rThis.top, nButtonRight, rThis.bottom);
		//Set the max size we'll draw in to 200w x 800h to avoid buttons that are so large they look weird.
		if(rPane.Width() > 200){
			rPane.right = rPane.left + 200;
		}
		if(rPane.Height() > 800){
			//This equates to an approx max height of 57px per button
			rPane.bottom = rPane.top + 800;
		}
		//(e.lally 2012-02-29) PLID 48065 - Since templates have fewer buttons, make sure the height is at a max of 60px per button
		else if(m_bIsTemplate && rPane.Height() > (nPos * 60)){
			rPane.bottom = rPane.top + (nPos * 60);
		}

		const float btn_height = 1/(float)nPos;

		int nBtnPos = -1;
		mapBtnPosition.Lookup(nID, nBtnPos);
		ASSERT(nBtnPos != -1);

		//Give a little space in between the left and right topic nav buttons
		const int cnNavBtnBuffer = 2;
		switch(nID) {
		default:
			//Lookup the position
			rControl.SetRect(rPane.left, (int)(rPane.top + (rPane.Height() * nBtnPos * btn_height)), rPane.right, (int)(rPane.top + (rPane.Height() * (nBtnPos + 1) * btn_height)));
		break;
		case ID_EMR_PREVIOUS_TOPIC:
			//Set to half the width, left justified
			rControl.SetRect(rPane.left, (int)(rPane.top + (rPane.Height() * nBtnPos * btn_height)), (int)(rPane.right/(float) 2) - cnNavBtnBuffer, (int)(rPane.top + (rPane.Height() * (nBtnPos + 1) * btn_height)));
			break;
		case ID_EMR_NEXT_TOPIC:
			//Set to half the width, right justified
			rControl.SetRect((int)(rPane.right/(float) 2) - cnNavBtnBuffer, (int)(rPane.top + (rPane.Height() * nBtnPos * btn_height)), rPane.right, (int)(rPane.top + (rPane.Height() * (nBtnPos + 1) * btn_height)));
			break;
		}
	}

	return rControl;
}

// (j.jones 2008-07-24 08:35) - PLID 30729 - change the button icon based on whether we have problems
//(e.lally 2012-02-16) PLID 48065 - Moved from EmrTreeWnd
void CEMRClassicButtonPane::UpdateEMRProblemButtonIcon()
{
	try {
		//(e.lally 2012-04-05) PLID 49446 - Re-instituted icon updating

		if(!m_btnEMRProblemList.GetSafeHwnd()) {
			//do not try to access the button if it doesn't exist
			return;
		}

		if (!GetEmrFrameWnd() || !GetEmrFrameWnd()->GetEmrTreeWnd()) {
			return;
		}

		//get all the problems on this EMR
		CArray<CEmrProblem*, CEmrProblem*> aryProblems;
		GetEmrFrameWnd()->GetEmrTreeWnd()->GetEMR()->GetAllProblems(aryProblems);

		BOOL bHasOpen = FALSE;
		BOOL bHasClosed = FALSE;

		//ignore deleted problems, check for open and closed ones
		for(int i=0; i<aryProblems.GetSize() && !(bHasOpen && bHasClosed); i++) {

			CEmrProblem *pProblem = (CEmrProblem*)aryProblems.GetAt(i);
			if(pProblem != NULL && !pProblem->m_bIsDeleted) {

				if(pProblem->m_nStatusID == 2) {
					//we have a closed problem
					bHasClosed = TRUE;
				}
				else {
					//we have an open problem
					bHasOpen = TRUE;
				}
			}
		}

		// (a.walling 2012-07-06 13:33) - PLID 51404 - This was constantly invalidating itself on every idle update
		WORD iconID = IDI_EMR_PROBLEM_FLAG;

		if(!bHasOpen && !bHasClosed) {
			//we have no problems
			iconID = IDI_EMR_PROBLEM_EMPTY;
		}
		else if(!bHasOpen && bHasClosed) {
			//we have only closed problems
			iconID = IDI_EMR_PROBLEM_CLOSED;
		}
		else {
			//we have some open problems
		}

		// (a.walling 2012-07-06 13:33) - PLID 51404 - Only update if changed
		if (iconID != m_btnEMRProblemList.GetIconID()) {
			m_btnEMRProblemList.SetIcon(iconID);
			m_btnEMRProblemList.Invalidate(FALSE);
			m_btnEMRProblemList.RedrawWindow();
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2012-02-16) PLID 48065 - Moved from EmrTreeWnd, mostly unchanged. Removed EmrTree specific code
void CEMRClassicButtonPane::RepositionControls()
{
	try {

		ASSERT(IsWindow(GetSafeHwnd()));

		CRect rc;

		// (c.haag 2007-08-30 16:00) - PLID 27256 - Verify that the button
		// exists first
		if (IsWindow(m_btnNewEmn.GetSafeHwnd())) {
			rc = GetControlRect(ID_EMR_NEW_EMN);
			m_btnNewEmn.MoveWindow(&rc);
		}
		rc = GetControlRect(ID_EMR_EDIT_MODE);
		m_btnEditMode.MoveWindow(&rc);
		rc = GetControlRect(ID_EMR_ADD_TOPICS_BTN_MENU);
		m_btnAddTopics.MoveWindow(&rc);
		rc = GetControlRect(ID_EMR_ADD_ITEM);
		m_btnAddItemToTopic.MoveWindow(&rc);
		// (c.haag 2007-08-30 16:00) - PLID 27256 - Verify that the button
		// exists first
		if (IsWindow(m_btnCreateToDo.GetSafeHwnd())) {
			rc = GetControlRect(ID_EMR_NEW_TODO);
			m_btnCreateToDo.MoveWindow(&rc);
		}
		// (c.haag 2007-08-30 16:00) - PLID 27256 - Verify that the button
		// exists first
		if (IsWindow(m_btnRecordAudio.GetSafeHwnd())) {
			rc = GetControlRect(ID_EMR_NEW_RECORDING);
			m_btnRecordAudio.MoveWindow(&rc);
		}
		// (j.jones 2009-03-03 12:06) - PLID 33308 - added e-Rx button
		if (IsWindow(m_btnEPrescribing.GetSafeHwnd())) {
			rc = GetControlRect(ID_EMR_VIEW_EPRESCRIBING);
			m_btnEPrescribing.MoveWindow(&rc);
		}
		// (j.jones 2008-07-09 08:59) - PLID 24624 - added Patient Summary button
		if (IsWindow(m_btnPatientSummary.GetSafeHwnd())) {
			rc = GetControlRect(ID_EMR_PATIENT_SUMMARY);
			m_btnPatientSummary.MoveWindow(&rc);
		}
		// (j.jones 2008-07-17 13:08) - PLID 30729 - added Problem List button
		if (IsWindow(m_btnEMRProblemList.GetSafeHwnd())) {
			rc = GetControlRect(ID_EMR_PROBLEM_LIST_BTN_MENU);
			m_btnEMRProblemList.MoveWindow(&rc);
		}
		rc = GetControlRect(ID_EMR_SAVE);
		m_btnSaveChanges.MoveWindow(&rc);
		// (z.manning 2009-08-12 14:02) - PLID 27694 - Added save and close button
		if(IsWindow(m_btnSaveAndClose.GetSafeHwnd())) {
			rc = GetControlRect(ID_EMR_SAVE_AND_CLOSE);
			m_btnSaveAndClose.MoveWindow(&rc);
		}
#ifdef _DEBUG
		// (c.haag 2007-08-04 10:03) - PLID 26946 - Used for EMR debugging
		rc = GetControlRect(ID_EMR_DEBUG);
		m_btnDebug.MoveWindow(&rc);
#endif
		// (c.haag 2007-08-30 16:23) - PLID 27058 - E/M Checklist
		if (m_bIsTemplate) {
			rc = GetControlRect(ID_EMR_EM_CHECKLIST);
			m_btnEMChecklist.MoveWindow(&rc);

			//TES 9/9/2009 - PLID 35495 - Button to position spawned topics
			rc = GetControlRect(ID_EMR_POSITION_TOPICS);
			m_btnPositionTopics.MoveWindow(&rc);

			//(e.lally 2012-04-03) PLID 48065 - Dynamic Cancel button
			if (IsWindow(m_btnCancel.GetSafeHwnd())) {
				rc = GetControlRect(ID_EMR_CLOSE);
				m_btnCancel.MoveWindow(&rc);
			}
		}
		rc = GetControlRect(ID_EMR_NEXT_TOPIC);
		m_btnNextNode.MoveWindow(&rc);
		rc = GetControlRect(ID_EMR_PREVIOUS_TOPIC);
		m_btnPreviousNode.MoveWindow(&rc);
	} NxCatchAllCall("Error 100 in CEMRClassicButtonPane::RepositionControls()",return;);
}


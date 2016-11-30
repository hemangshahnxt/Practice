// NotesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NotesDlg.h"
#include "GlobalUtils.h"
#include "MainFrm.h"
#include "NxStandard.h"
#include "NoteCategories.h"
#include "PracProps.h"
#include "PatientView.h"
#include "GlobalDataUtils.h"
#include "AuditTrail.h"
#include "NxSecurity.h"
#include "MacroEditDlg.h"
#include "MacroSelectDlg.h"
#include "SearchNotesDlg.h"
#include "AdvPrintOptions.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "GlobalReportUtils.h"
#include "ReferralTreeDlg.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "PatientsRc.h"
#include "LabFollowupDlg.h" // (j.dinatale 2011-01-24) - PLID 41975
#include "MultiSelectDlg.h" // (j.armen 2011-07-21 09:42) - PLID 13283
#include "AlbertaHLINKUtils.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.
// (c.haag 2010-08-26 11:26) - PLID 39473 - All instances of GetActivePatient... have been changed to GetActivePerson...
// (a.walling 2010-09-09 12:25) - PLID 40267 - Protected PatientNote audits to ensure this is actually a patient and not a contact. Some of these
// should not be reachable from code anyway, but exist to prevent any future confusion.


//PLID 18881 6/4/2008 r.galicki - Added constant for priority and color columns
#define COLUMN_ID			0
#define COLUMN_USER_ID		1
// (a.walling 2010-07-13 17:59) - PLID 39182
#define COLUMN_NOTE_INPUT_DATE	2
#define COLUMN_COLOR		3
#define COLUMN_DATE			4
#define COLUMN_CATEGORY		5
#define COLUMN_PRIORITY		6
#define COLUMN_SHOW_ON_STATEMENT	7 // (j.gruber 2010-06-14 13:56) - PLID 39153 - added
#define COLUMN_SEND_ON_CLAIM		8 // (j.jones 2011-09-19 14:21) - PLID 42135
#define COLUMN_NOTE			9
#define COLUMN_BILLSTATUSNOTE_BILLID	10 // (r.gonet 07/22/2014) - PLID 62525 - Need to track the linked bill ID if this note is a bill status note.

//PLID 18881 6/4/2008 R.G.	-	Added constants for priorities
#define PRIORITY_LOW		3
#define PRIORITY_MEDIUM		2
#define PRIORITY_HIGH		1

// (r.galicki 2008-06-27 11:15) - PLID 18881 - Default colors
//DRT 8/25/2008 - PLID 31154 - Changed defaults to red / blue / black
#define DEFAULT_COLOR_HIGH			RGB(255, 0, 0)
#define DEFAULT_COLOR_MEDIUM		RGB(0, 0, 200)
#define DEFAULT_COLOR_LOW			RGB(0, 0, 0)

// (a.walling 2012-11-05 11:58) - PLID 53588 - Resolve conflict with mshtmcid.h

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define IDM_ADD     42703
#define IDM_DELETE_NOTE  42704
#define IDM_PRINT_NOTE   42705
#define IDM_ADD_MACRO   42706
#define IDM_ADD_NEW_THREAD 42707
#define IDM_ADD_TO_SELECTED_THREAD 42708
#define IDM_ADD_NEW_THREAD_MACRO 42709
#define IDM_ADD_TO_SELECTED_THREAD_MACRO 42710
//#define IDM_DELETE_NOTE 42711
#define IDM_DELETE_THREAD 42712
// (z.manning 2010-05-04 13:08) - PLID 36511 - Menu options to print/preview single note
#define IDM_PREVIEW_SINGLE_NOTE 42713
#define IDM_PRINT_SINGLE_NOTE 42714

#define ID_MULTIPLE_CATS -3 // (j.armen 2011-07-21 11:46) - PLID 13283 - Addded category selection for having multiple categories
#define ID_ALL_CATS		-2
#define ID_NO_CAT		-1

// (j.gruber 2009-11-02 13:06) - PLID 35815 - new datalist
enum MessageListColumns {
	mlcParentID = 0,
	mlcChildID, 
	mlcThreadID,
	mlcNoteID,
	mlcDateCol,
	mlcSubject,
	mlcStatus,
	mlcNote,
};

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CNotesDlg dialog


CNotesDlg::CNotesDlg(CWnd* pParent, BOOL bUnsavedCharge)
	: CPatientDialog(CNotesDlg::IDD, pParent), 
	m_tcNoteCategories(NetUtils::NoteCatsF)
{
	//{{AFX_DATA_INIT(CNotesDlg)
		m_IsEditing = FALSE;
		m_nBillID = -1;
		m_nLineItemID = -1;
		m_nMailID = -1;
		m_bUnsavedCharge = bUnsavedCharge;
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Patient_Information/Notes/add_a_note.htm";
	m_bRequeryToSetSel = false;
	
	m_bIsBillingNote = false;
	// (j.jones 2011-09-19 14:46) - PLID 42135 - added BillingNoteType, used if m_bIsBillingNote is true
	m_bntBillingNoteType = bntNone;

	m_bIsHistoryNote = false; // (c.haag 2010-07-01 13:45) - PLID 39473
	m_bIsForPatient = true; // (a.walling 2010-09-09 12:21) - PLID 40267
	m_bIsCreatingNewNote = FALSE;
	// (j.gruber 2009-11-02 11:22) - PLID 35815 - set variable
	m_bThemeOpen = FALSE;
	// (a.walling 2010-09-20 11:41) - PLID 40589 - Just use the overridden id now
	//m_nHistoryNotePersonID = -1; // (c.haag 2010-08-26 11:26) - PLID 39473
	m_clrHistoryNote = 0; // (c.haag 2010-08-26 14:52) - PLID 39473

	// (a.walling 2010-09-20 11:41) - PLID 40589 - Initialize the person ID
	m_id = -1;
	// (a.walling 2010-09-20 11:41) - PLID 40589 - if m_id set manually, GetActivePersonID will always return m_id
	m_bIsValidID = false;

	// (j.dinatale 2010-12-22) - PLID 41885 - needed to keep track of lab information
	m_nLabResultID = -1;
	m_nLabID = -1;
	m_bIsLabNotes = false;
	m_clrLabNote = 0;

	// (j.gruber 2014-12-12 15:00) - PLID 64391 - Rescheduling Queue - attached appointments to notes
	m_nApptID = -1;
	m_dtApptStartTime = g_cdtNull;
	
		
	// (j.dinatale 2010-12-22) - PLID 41915 - initialize prepended text to empty string
	m_strPrependText = "";

	// (j.dinatale 2010-12-23) - PLID 41932 - default the category override value to -1
	m_nCategoryIDOverride = -1;

	// (j.dinatale 2010-12-23) - PLID 41930 - default to note auto add a note on show
	m_bAutoAddNoteOnShow = false;

	m_vtRecallID = g_cvarNull;	// (j.armen 2012-03-19 09:11) - PLID 48780 - Default to VT_NULL
	//(s.dhole 8/29/2014 10:22 AM ) - PLID 63516 set defult value
	m_nPatientRemindersSentID =-1;
	m_bIsPatientRemindersSent = false;
	m_sPatientRemindersPrefix = "";
}


void CNotesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNotesDlg)
	DDX_Control(pDX, IDC_NOTES_SHOW_GRID, m_btnShowGridlines);
	DDX_Control(pDX, IDC_CATEGORY_FILTER, m_btnFilterCategory);
	DDX_Control(pDX, IDC_SEARCH_NOTES, m_btnSearchNotes);
	DDX_Control(pDX, IDC_EDIT_MACROS, m_editMacros);
	DDX_Control(pDX, IDC_ADD_MACRO, m_AddMacroBtn);
	DDX_Control(pDX, IDC_EDIT_CATEGORY, m_editCategories);
	DDX_Control(pDX, IDC_ADD, m_addButton);
	DDX_Control(pDX, IDC_NOTES_BKG, m_bkg);
	DDX_Control(pDX, IDC_CHECK_SPELLING_BTN, m_btnCheckSpelling); // (b.cardillo 2006-12-13 12:40) - PLID 23713 - Spell-checking of all notes.
	DDX_Control(pDX, IDC_MULTIPLE_NOTE_CAT_FILTER, m_nxlMultipleCatSelection); // (j.armen 2011-08-04 15:59) - PLID 13283 - Added label for when multiple categories are selected in the filter
	DDX_Control(pDX, IDC_CHECK_HIDE_BILL_NOTES, m_checkHideBills);
	DDX_Control(pDX, IDC_CHECK_HIDE_CLAIM_NOTES, m_checkHideClaims);
	DDX_Control(pDX, IDC_CHECK_HIDE_STATEMENT_NOTES, m_checkHideStatements); 
	//}}AFX_DATA_MAP
}

// (j.dinatale 2010-12-23) - PLID 41930 - added event handle for requery finished
// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.
//(c.copits 2011-07-05) PLID 22709 - Remember column settings
BEGIN_EVENTSINK_MAP(CNotesDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNotesDlg)
	ON_EVENT(CNotesDlg, IDC_NXDLNOTES, 7 /* RButtonUp */, OnRButtonUpNxdlnotes, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CNotesDlg, IDC_NXDLNOTES, 10 /* EditingFinished */, OnEditingFinishedNxdlnotes, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNotesDlg, IDC_NXDLNOTES, 17 /* ColumnClicking */, OnColumnClickingNxdlnotes, VTS_I2 VTS_PBOOL)
	ON_EVENT(CNotesDlg, IDC_NXDLNOTES, 21 /* EditingStarted */, OnEditingStartedNxdlnotes, VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CNotesDlg, IDC_NXDLNOTES, 9 /* EditingFinishing */, OnEditingFinishingNxdlnotes, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CNotesDlg, IDC_NXDLNOTES, 2 /* SelChanged */, OnSelChangedNxdlnotes, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CNotesDlg, IDC_NXDLNOTES, 24 /* FocusLost */, OnFocusLostNxdlnotes, VTS_NONE)
	ON_EVENT(CNotesDlg, IDC_NXDLNOTES, 18 /* RequeryFinished */, OnRequeryFinishedNoteList, VTS_I2)
	ON_EVENT(CNotesDlg, IDC_NOTE_CATEGORY_LIST, 1 /* SelChanging */, OnSelChangingNoteCategoryList, VTS_PI4)
	ON_EVENT(CNotesDlg, IDC_NOTE_CATEGORY_LIST, 18 /* RequeryFinished */, OnRequeryFinishedNoteCategoryList, VTS_I2)
	ON_EVENT(CNotesDlg, IDC_NOTE_CATEGORY_LIST, 16 /* SelChosen */, OnSelChosenNoteCategoryList, VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CNotesDlg, IDC_TAB_PATIENT_NOTES, 1, CNotesDlg::SelectTabTabPatientNotes, VTS_I2 VTS_I2)
	ON_EVENT(CNotesDlg, IDC_PATIENT_MESSAGING_THREAD_LIST, 8, CNotesDlg::EditingStartingPatientMessagingThreadList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CNotesDlg, IDC_PATIENT_MESSAGING_THREAD_LIST, 10, CNotesDlg::EditingFinishedPatientMessagingThreadList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNotesDlg, IDC_PATIENT_MESSAGING_THREAD_LIST, 7, CNotesDlg::RButtonUpPatientMessagingThreadList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)	
	ON_EVENT(CNotesDlg, IDC_PATIENT_MESSAGING_THREAD_LIST, 9, CNotesDlg::EditingFinishingPatientMessagingThreadList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CNotesDlg, IDC_NXDLNOTES, 22, CNotesDlg::ColumnSizingFinishedNotes, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CNotesDlg, IDC_NXDLNOTES, 23 /* ChangeColumnSortFinished */, CNotesDlg::ColumnSizingFinishedNotes, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CNotesDlg, IDC_PATIENT_MESSAGING_THREAD_LIST, 22, CNotesDlg::ColumnSizingFinishedNotesMessageThreads, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
	ON_EVENT(CNotesDlg, IDC_PATIENT_MESSAGING_THREAD_LIST, 23 /* ChangeColumnSortFinished */, CNotesDlg::ColumnSizingFinishedNotesMessageThreads, VTS_I2 VTS_BOOL VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

BEGIN_MESSAGE_MAP(CNotesDlg, CNxDialog)
	//{{AFX_MSG_MAP(CNotesDlg)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_ADD, OnAddButtonClicked)
	ON_BN_CLICKED(IDC_EDIT_CATEGORY, OnEditCategory)
	ON_BN_CLICKED(IDC_ADD_MACRO, OnAddMacroButtonClicked)
	ON_BN_CLICKED(IDC_EDIT_MACROS, OnEditMacros)
	ON_BN_CLICKED(IDC_SEARCH_NOTES, OnSearchNotes)
	ON_BN_CLICKED(IDC_CATEGORY_FILTER, OnCategoryFilter)
	ON_BN_CLICKED(IDC_NOTES_SHOW_GRID, OnNotesShowGrid)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_CHECK_SPELLING_BTN, OnCheckSpellingBtn) // (b.cardillo 2006-12-13 12:40) - PLID 23713 - Spell-checking of all notes.
	ON_BN_CLICKED(IDC_NOTES_REMEMBER_COL_SETTINGS, OnBnClickedRememberColSettings)
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick) // (j.armen 2011-08-04 15:59) - PLID 13283
	ON_WM_SETCURSOR() // (j.armen 2011-08-04 16:00) - PLID 13283
	ON_BN_CLICKED(IDC_CHECK_HIDE_BILL_NOTES, OnHideBillNotes)
	ON_BN_CLICKED(IDC_CHECK_HIDE_CLAIM_NOTES, OnHideClaimNotes)
	ON_BN_CLICKED(IDC_CHECK_HIDE_STATEMENT_NOTES, OnHideStatementNotes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNotesDlg message handlers

void CNotesDlg::SetColor(OLE_COLOR nNewColor)
{
	m_bkg.SetColor(nNewColor);

	CPatientDialog::SetColor(nNewColor);
}

void CNotesDlg::RefreshColors()
{
	// (r.galicki 2008-06-26 16:59) - PLID 18881 - Update color column
	try {
		bool bColorize = (GetRemotePropertyInt("ColorizeNotes", 1, 0, GetCurrentUserName(), true) == 1);
		CString strField;
		if(bColorize) {
			strField.Format("CASE WHEN Priority = 1 THEN %li WHEN Priority = 2 THEN %li WHEN Priority = 3 THEN %li END",
				GetRemotePropertyInt("NotesPriorityHigh", DEFAULT_COLOR_HIGH, 0, GetCurrentUserName(), TRUE),
				GetRemotePropertyInt("NotesPriorityMedium", DEFAULT_COLOR_MEDIUM, 0, GetCurrentUserName(), TRUE),
				GetRemotePropertyInt("NotesPriorityLow", DEFAULT_COLOR_LOW, 0, GetCurrentUserName(), TRUE));
		}
		else {
			//default to black
			strField.Format("0");
		}

		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pNxDlNotes->GetColumn(COLUMN_COLOR);
		if (pCol) {
			pCol->PutFieldName((LPCTSTR)strField);
		}
	}NxCatchAll("Error in CNotesDlg::RefreshColors \n Error updating color field");
}

BOOL CNotesDlg::OnInitDialog() 
{
	try {
		m_editCategories.AutoSet(NXB_MODIFY);
		m_addButton.AutoSet(NXB_NEW);
		//DRT 4/11/2008 - PLID 29636 - NxIconify
		m_btnCheckSpelling.AutoSet(NXB_SPELLCHECK);
		m_AddMacroBtn.AutoSet(NXB_NEW);
		m_editMacros.AutoSet(NXB_MODIFY);

		// (r.galicki 2008-06-26 16:58) - PLID 18881 - Initialize note colors
		g_propManager.CachePropertiesInBulk("CNotesDlgNumberProp", propNumber,
				"(Username = '%s') AND ("
				"Name = 'ColorizeNotes' OR "
				"Name = 'NotesPriorityHigh' OR "
				"Name = 'NotesPriorityMedium' OR "
				"Name = 'NotesPriorityLow' OR "
				// (j.jones 2011-09-20 09:43) - PLID 44934 - added Alberta option
				"Name = 'UseAlbertaHLINK' OR "
				// (b.spivey, October 04, 2012) - PLID 30398 - Cache
				// (b.spivey, October 17, 2012) - PLID 30398 - Moved cache. 
				"Name = 'PatientNotesHideOtherBillNotes' OR "
				"Name = 'PatientNotesHideClaimNotes' OR "
				"Name = 'PatientNotesHideStatementNotes' OR "
				"Name = 'NotesShowGridBill' OR " // (r.gonet 2015-01-20 15:40) - PLID 63490 - Bulk cached the other gridline properties while I was at it.
				"Name = 'NotesShowGridHistory' OR "
				"Name = 'NotesShowGridLab' OR "
				"Name = 'NotesShowGridRecall' OR " // (r.gonet 2015-01-20 15:40) - PLID 63490 - Bulk cached recall grid lines property, which didn't exist before.
				"Name = 'NotesShowGridReminderHistory' OR " //(s.dhole 8/29/2014 10:35 AM ) - PLID 63516
				"Name = 'NotesShowGridAppointments' OR "
				"Name = 'NotesShowGrid' OR "
				"Name = 'ApptNotesDefaultCategory' " // (j.gruber 2015-02-04 10:28) - PLID 64392 - Rescheduling Queue - In Tools > Preferences > Scheduling > Display 2, add a Global preference that reads “Default Appointment History Notes Category”. Inside the preference, there should be a drop down menu with a list of Categories. The default selection should be < No Category >.
				")",
				_Q(GetCurrentUserName()));

		//(c.copits 2011-07-05) PLID 22709 - Remember column settings
		g_propManager.CachePropertiesInBulk("CNotesDlg", propText,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'DefaultPatientsNotesColumnSizes' OR "
				"Name = 'DefaultPatientsNotesColumnSort' OR "
				"Name = 'DefaultPatientsNotesMessageThreadsColumnSizes' OR "
				"Name = 'DefaultPatientsNotesMessageThreadsColumnSort' OR "
				"Name = 'DefaultPatientsNotesRememberColumnSettings' OR "
				"Name = 'DefaultPatientsNotesRememberMessageThreadsColumnSettings' "
				")"
				, _Q(GetCurrentUserName()));

		// (b.spivey, October 04, 2012) - PLID 30398 - Cache
		g_propManager.CachePropertiesInBulk("CNotesDlgFilters", propNumber, 
			"(Username = '<None>' OR Username = '%s') AND ( "
			"Name = 'PatientNotesHideOtherBillNotes' OR "
			"Name = 'PatientNotesHideClaimNotes' OR "
			"Name = 'PatientNotesHideStatementNotes' "
			") ", _Q(GetCurrentUserName()));

		// (j.gruber 2008-07-28 14:50) - PLID 30860 - hide the odd cancel button
		GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);

		m_bSplit = false;
		
		//Set up the datalist
		try {
			// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.
			m_pNxDlNotes = BindNxDataList2Ctrl(IDC_NXDLNOTES, false);
			
			if (m_pNxDlNotes == NULL) {
				HandleException(NULL, "Error in CNotesDlg::OnInitDialog \n DataList is Null, Cannot Continue");
				return FALSE;
			}

			m_pMessagesList = BindNxDataList2Ctrl(IDC_PATIENT_MESSAGING_THREAD_LIST, false);

			m_pMessagesList->FromClause = "(SELECT NULL as ParentID, PatientMessagingThreadT.ID as ChildID, PatientMessagingThreadT.ID as ThreadID, NULL as NoteID, Subject, CreatedDate as Date, CONVERT(int, Status) as Status, Notes.PersonID, NULL as Note  "
				" FROM PatientMessagingThreadT INNER JOIN Notes ON PatientMessagingThreadT.ID = Notes.PatientMessagingThreadID "
				" UNION  "
				" SELECT PatientMessagingThreadID as ParentID, NULL as ChildID, PatientMessagingThreadID as ThreadID, Notes.ID as NoteID, NULL as Subject, Date, NULL as Status, PersonID, Note FROM Notes "
				" WHERE PatientMessagingThreadID IS NOT NULL) Q";

		}NxCatchAll("Error in CNotesDlg::OnInitDialog \n Could Not Bind DataList");

		m_pCategories = BindNxDataListCtrl(IDC_NOTE_CATEGORY_LIST, true);

		m_IsComboVisible = false;

		// (a.walling 2010-10-12 14:54) - PLID 40977 - Don't load the id until UpdateView is called
		/*
		m_id = GetActivePersonID();
		*/
		
		CNxDialog::OnInitDialog();
		SecureControls();
		m_bAllowUpdate = true;

		// (j.gruber 2009-11-02 11:12) - PLID 35815 - added sub tab for patient messaging	
		InitializeTabs();

		// (j.dinatale 2010-12-22) - PLID 41885 - added m_nLabID and m_nLabResultID
		// (c.haag 2010-07-01 13:45) - PLID 39473 - Added m_nMailID
		// (j.armen 2012-03-19 09:11) - PLID 48780 - added m_vtRecallID
		//(s.dhole 8/29/2014 10:32 AM ) - PLID 63516 Added m_nPatientRemindersSentID
		// (j.gruber 2014-12-12 15:00) - PLID 64391 - Rescheduling Queue - attached appointments to notes		
		if (m_nBillID != -1 || m_nLineItemID != -1 || m_nMailID != -1 || m_nLabResultID != -1 || m_nLabID != -1 || m_vtRecallID != g_cvarNull || m_nPatientRemindersSentID != -1 || m_nApptID != -1)  {
			//if either is not -1, it means this is being called from the financial dialog,
			//not the patient view, so update the color and call UpdateView()
			if (m_bIsHistoryNote) {
				// (c.haag 2010-08-26 14:51) - PLID 39473 - Set the color using a local override
				SetColor(m_clrHistoryNote);
			} else {
				// (j.dinatale 2010-12-22) - PLID 41885 - if this is some form of a lab note, need to override the color to the lab dialog
				if(m_bIsLabNotes){
					SetColor(m_clrLabNote);
				} else {
					if(m_vtRecallID != g_cvarNull) {
						SetColor(m_clrRecallNote);
					} else{
						SetColor(GetNxColor(GNC_PATIENT_STATUS, GetMainFrame()->m_patToolBar.GetActivePatientStatus()));
					}
				}
			}
			UpdateView();
		}
		
		//It checks to see where the notes dialog is called from and then gets the correct properties for it
		if (m_bIsBillingNote) {
			if(GetRemotePropertyInt("NotesShowGridBill", 0, 0, GetCurrentUserName(), true)) {
				CheckDlgButton(IDC_NOTES_SHOW_GRID, BST_CHECKED);
				m_pNxDlNotes->GridVisible = true;
			}
		}
		else if (m_bIsHistoryNote)
		{
			// (c.haag 2010-07-01 13:45) - PLID 39473
			if(GetRemotePropertyInt("NotesShowGridHistory", 0, 0, GetCurrentUserName(), true)) {
				CheckDlgButton(IDC_NOTES_SHOW_GRID, BST_CHECKED);
				m_pNxDlNotes->GridVisible = true;
			}			
		}
		else if(m_bIsLabNotes)
		{
			// (j.dinatale 2010-12-22) - PLID 41885 - if lab notes, determine if the grid should be shown
			if(GetRemotePropertyInt("NotesShowGridLab", 0, 0, GetCurrentUserName(), true)){
				CheckDlgButton(IDC_NOTES_SHOW_GRID, BST_CHECKED);
				m_pNxDlNotes->GridVisible = true;
			}
		}
		// (r.gonet 2015-01-20 15:40) - PLID 63490 - Load the grid lines if the user has set it for recalls
		else if (m_vtRecallID != g_cvarNull) {
			if (GetRemotePropertyInt("NotesShowGridRecall", 0, 0, GetCurrentUserName(), true)) {
				CheckDlgButton(IDC_NOTES_SHOW_GRID, BST_CHECKED);
				m_pNxDlNotes->GridVisible = true;
			}
		}
		//(s.dhole 8/29/2014 10:34 AM ) - PLID 63516
		else if (m_bIsPatientRemindersSent)
		{
			if (GetRemotePropertyInt("NotesShowGridReminderHistory", 0, 0, GetCurrentUserName(), true)) {
				CheckDlgButton(IDC_NOTES_SHOW_GRID, BST_CHECKED);
				m_pNxDlNotes->GridVisible = true;
			}
		}
		else if (m_nApptID != -1) // (j.gruber 2014-12-15 09:56) - PLID 64391 - Rescheduling Queue - appointment notes
		{
			if (GetRemotePropertyInt("NotesShowGridAppointments", 0, 0, GetCurrentUserName(), true))
			{
				CheckDlgButton(IDC_NOTES_SHOW_GRID, BST_CHECKED);
				m_pNxDlNotes->GridVisible = true;
			}
		}
		else
		{
			if(GetRemotePropertyInt("NotesShowGrid", 0, 0, GetCurrentUserName(), true)) {
				CheckDlgButton(IDC_NOTES_SHOW_GRID, BST_CHECKED);
				m_pNxDlNotes->GridVisible = true;
			}
		}

		// (a.walling 2010-10-13 16:12) - PLID 40977 - Moved to LoadList
		//RefreshColors();

		// (b.spivey, October 05, 2012) - PLID 30398 - Init the checkboxes to the proper state. 
		if(GetRemotePropertyInt("PatientNotesHideOtherBillNotes", TRUE, 0, GetCurrentUserName(), true)) {
			m_checkHideBills.SetCheck(TRUE);
		}

		if(GetRemotePropertyInt("PatientNotesHideClaimNotes", TRUE, 0, GetCurrentUserName(), true)) {
			m_checkHideClaims.SetCheck(TRUE);
		}

		if(GetRemotePropertyInt("PatientNotesHideStatementNotes", TRUE, 0, GetCurrentUserName(), true)) {
			m_checkHideStatements.SetCheck(TRUE);
		}


	}NxCatchAll(__FUNCTION__);
	return TRUE;
}


void CNotesDlg::InitializeTabs() {

	try {
		//theme copied from historydlg
		{
			UXTheme* pTheme = new UXTheme();
			pTheme->OpenThemeData(GetDlgItem(IDC_ADD)->GetSafeHwnd(), "Button");	//this doesn't matter

			if(pTheme->IsOpen())
				m_bThemeOpen = TRUE;
			else
				m_bThemeOpen = FALSE;

			//cleanup
			pTheme->CloseThemeData();
			delete pTheme;
		}

		//initialize the tabs
		m_SubTab = GetDlgItemUnknown(IDC_TAB_PATIENT_NOTES);
		if (m_SubTab == NULL)
		{
			HandleException(NULL, "Failed to bind NxTab control", __LINE__, __FILE__);
			PostMessage(WM_COMMAND, IDCANCEL);
			return;
		}

		//now lets setup our tabs
		m_SubTab->BottomTabs = true;

		// (j.jones 2016-04-20 11:01) - NX-100214 - Set HeaderMode to false, which will
		// use a slightly different theme than the module tabs use.
		// A HeaderMode of false looks nicer when the tab is next to a datalist.
		m_SubTab->HeaderMode = false;

		m_SubTab->PutTabWidth(2);
		m_SubTab->PutSize(2);
		m_SubTab->PutLabel(0, "Notes");//The first tab is always the usual notes.
		m_SubTab->ResetTabBackColor(0);
		m_SubTab->PutLabel(1, "Message Threads") ;		
		m_SubTab->ResetTabBackColor(1);

		//default to 0
		m_SubTab->CurSel = 0;

	}NxCatchAll(__FUNCTION__);
}

void CNotesDlg::ResetControls()
{
	try {
		long nCurSel = m_SubTab->CurSel;

		switch (nCurSel) {

			case 0: //on the notes tab

				GetDlgItem(IDC_NXDLNOTES)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_PATIENT_MESSAGING_THREAD_LIST)->ShowWindow(SW_HIDE);
				// (j.armen 2011-08-04 16:00) - PLID 13283 - Ensure the correct control is shown based on current selection
				if(m_pCategories->GetCurSel() == ID_MULTIPLE_CATS)
				{
					GetDlgItem(IDC_CATEGORY_FILTER)->ShowWindow(SW_HIDE);
					m_nxlMultipleCatSelection.ShowWindow(SW_SHOW);
				}
				else
				{
					m_nxlMultipleCatSelection.ShowWindow(SW_HIDE);
					GetDlgItem(IDC_CATEGORY_FILTER)->ShowWindow(SW_SHOW);
				}
				GetDlgItem(IDC_NOTE_CATEGORY_LIST)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_NOTES_SHOW_GRID)->ShowWindow(SW_SHOW);	

				//(c.copits 2011-07-05) PLID 22709 - Remember column settings
				if(GetRemotePropertyInt("DefaultPatientsNotesRememberColumnSettings", 0, 0, GetCurrentUserName(), true) == 1) {
					CheckDlgButton(IDC_NOTES_REMEMBER_COL_SETTINGS, TRUE);
					SetColumnSizes();
					SetColumnSorts();
				}
				else {
					CheckDlgButton(IDC_NOTES_REMEMBER_COL_SETTINGS, FALSE);
				}
			break;

			case 1: //on patient messaging tab
				GetDlgItem(IDC_NXDLNOTES)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_PATIENT_MESSAGING_THREAD_LIST)->ShowWindow(SW_SHOW);
				GetDlgItem(IDC_CATEGORY_FILTER)->ShowWindow(SW_HIDE);
				// (j.armen 2011-08-04 16:10) - PLID 13283 - Hide the label
				m_nxlMultipleCatSelection.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NOTE_CATEGORY_LIST)->ShowWindow(SW_HIDE);
				GetDlgItem(IDC_NOTES_SHOW_GRID)->ShowWindow(SW_HIDE);

				//(c.copits 2011-07-05) PLID 22709 - Remember column settings
				if(GetRemotePropertyInt("DefaultPatientsNotesMessageThreadsRememberColumnSettings", 0, 0, GetCurrentUserName(), true) == 1) {
					CheckDlgButton(IDC_NOTES_REMEMBER_COL_SETTINGS, TRUE);
					SetColumnSizes();
					SetColumnSorts();
				}
				else {
					CheckDlgButton(IDC_NOTES_REMEMBER_COL_SETTINGS, FALSE);
				}
			break;
		}


		//see if we have any data in the table
		//this has to go here so that it doesn't get reset
		//check the license here also
		// (j.dinatale 2010-12-22) - PLID 41885 - if its not a lab note
		// (a.walling 2010-10-13 16:10) - PLID 40977 - Move database access at the end of everything else
		// (j.jones 2013-11-22 09:54) - PLID 59696 - removed the check on PatientMessagingThreadT, you should
		// be able to see this tab even if you've never used the feature before
		if (m_bIsForPatient && (!m_bIsBillingNote) && (!m_bIsLabNotes)  && (m_vtRecallID == g_cvarNull)
			&& (!m_bIsHistoryNote) // (c.haag 2010-07-01 13:45) - PLID 39473
			&& (!m_bIsPatientRemindersSent) //(s.dhole 8/29/2014 10:47 AM ) - PLID 63516
			&& (m_nApptID == -1) // (j.gruber 2014-12-15 09:56) - PLID 64391 - Rescheduling Queue - appointment notes
			&& (g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrSilent))
			) {
			//they have NexWeb, and this is not a popup dialog, so show the tabs
			GetDlgItem(IDC_TAB_PATIENT_NOTES)->ShowWindow(SW_SHOW);
		}
		else {
			//set the datalist to go to the bottom tab
			CRect rctTab;
			GetDlgItem(IDC_TAB_PATIENT_NOTES)->GetWindowRect(rctTab);
			//ScreenToClient(&rctTab);

			CRect rctList;
			GetDlgItem(IDC_NXDLNOTES)->GetWindowRect(rctList);
			//ScreenToClient(&rctList);
		
			CRect rctNew;
			rctNew.SetRect(rctList.left, rctList.top, rctList.right, rctTab.bottom);
			ScreenToClient(&rctNew);

			GetDlgItem(IDC_NXDLNOTES)->MoveWindow(rctNew, TRUE);
			GetDlgItem(IDC_TAB_PATIENT_NOTES)->ShowWindow(SW_HIDE);	

			// (b.spivey, October 02, 2012) - PLID 30398 - Hide them.
			m_checkHideBills.ShowWindow(FALSE);
			m_checkHideBills.EnableWindow(FALSE); 
			m_checkHideClaims.ShowWindow(FALSE);
			m_checkHideClaims.EnableWindow(FALSE); 
			m_checkHideStatements.ShowWindow(FALSE);
			m_checkHideStatements.EnableWindow(FALSE); 
		}	

		//Unless this is the patients module.
		//(s.dhole 8/29/2014 10:46 AM ) - PLID  63516 Added m_bIsPatientRemindersSent
		// (j.gruber 2014-12-15 09:56) - PLID 64391 - Rescheduling Queue - appointment notes
		if(m_bIsForPatient && (!m_bIsBillingNote) && (!m_bIsLabNotes)  && (m_vtRecallID == g_cvarNull) 
			&& (!m_bIsHistoryNote) && (!m_bIsPatientRemindersSent) && (m_nApptID == -1)) {

				m_checkHideBills.ShowWindow(TRUE);
				m_checkHideBills.EnableWindow(TRUE); 
				m_checkHideClaims.ShowWindow(TRUE);
				m_checkHideClaims.EnableWindow(TRUE); 
				m_checkHideStatements.ShowWindow(TRUE);
				m_checkHideStatements.EnableWindow(TRUE); 
		}

		
	}NxCatchAll(__FUNCTION__);
}


void CNotesDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {
		if(!m_bAllowUpdate) 
			return;

		// (a.wilson 2014-08-11 10:07) - PLID 63244 - check and update note categories.
		EnsureUpdatedCategoryFilter();

		// (j.gruber 2009-11-02 11:27) - PLID 35815 - reset the controls appropratiely
		ResetControls();

		// (r.galicki 2008-06-26 17:00) - PLID 18881 - Refresh colors
		// (a.walling 2010-10-13 16:12) - PLID 40977 - Moved to LoadList
		//RefreshColors();

		//set the where clause to be just for this patient
		long nCurrentlyLoadedID = m_id;
		m_id = GetActivePersonID();

		// (j.gruber 2010-06-14 13:59) - PLID 39153 - if we are not on billing, hide the statement checkbox
		if (!m_bIsBillingNote) {
			m_pNxDlNotes->GetColumn(COLUMN_SHOW_ON_STATEMENT)->PutColumnStyle(csVisible | csFixedWidth);				
			m_pNxDlNotes->GetColumn(COLUMN_SHOW_ON_STATEMENT)->PutStoredWidth(0);
		}
		else {
			m_pNxDlNotes->GetColumn(COLUMN_SHOW_ON_STATEMENT)->PutColumnStyle(csVisible | csEditable | csFixedWidth);				
			m_pNxDlNotes->GetColumn(COLUMN_SHOW_ON_STATEMENT)->PutStoredWidth(70);
		}

		// (j.jones 2011-09-19 14:21) - PLID 42135 - only show the Claim checkbox if this is a charge note
		if(m_bIsBillingNote && m_bntBillingNoteType == bntCharge) {
			m_pNxDlNotes->GetColumn(COLUMN_SEND_ON_CLAIM)->PutColumnStyle(csVisible | csEditable | csFixedWidth);				
			m_pNxDlNotes->GetColumn(COLUMN_SEND_ON_CLAIM)->PutStoredWidth(45);
		}
		else {
			m_pNxDlNotes->GetColumn(COLUMN_SEND_ON_CLAIM)->PutColumnStyle(csVisible | csFixedWidth);				
			m_pNxDlNotes->GetColumn(COLUMN_SEND_ON_CLAIM)->PutStoredWidth(0);
		}

		// (a.walling 2010-10-13 16:12) - PLID 40977
		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;
		}
		
		if (bForceRefresh || m_ForceRefresh) {
			LoadList();	
		}
		m_ForceRefresh = false;

		// (a.walling 2010-10-13 16:12) - PLID 40977 - Moved to LoadList		
		//DRT 11/2/2005 - In Internal only, prompt for a referral if this prospect hasn't been contacted in past 3 months
		//AttemptPromptForReferral();

		//TRACE("CNotesDlg::UpdateView\n");
	}NxCatchAll(__FUNCTION__);
}

void CNotesDlg::LoadList()
{
	try {
		// (a.walling 2010-10-13 16:12) - PLID 40977 - Moved to LoadList
		RefreshColors();

		BOOL bIsMessaging;

		bIsMessaging = (m_SubTab->CurSel == 1);

		// (j.armen 2012-03-19 09:12) - PLID 48780 - switched to using a CSqlFragment for readability
		CSqlFragment sqlWhere;

		if (bIsMessaging) {

			//we wouldn't be here if we were on the billing tab, so just filter on patient
			sqlWhere.Create("PersonID = {INT}", m_id);
			// (b.cardillo 2012-09-20 13:19) - PLID 52757 - Fixed to apply to the messages list instead of the notes list
			m_pMessagesList->PutWhereClause(_bstr_t(sqlWhere.Flatten()));
			m_pMessagesList->Requery();
		}
		else {
			
			if(IsDlgButtonChecked(IDC_CATEGORY_FILTER)) {
				// (j.armen 2011-07-21 11:48) - PLID 13283 - Directly call function for filtering notes to bypass the multiselect dlg
				FilterNotesByRow(m_pCategories->GetCurSel());
			}
			else {
				//we're not filtering on category, just do the usual
				// (j.armen 2012-03-19 09:12) - PLID 48780 - Added m_vtRecallID
				if(!m_bUnsavedCharge) {
					if(m_nBillID != -1)
						sqlWhere.Create("PersonID = {INT} AND BillID = {INT}", m_id, m_nBillID);
					else if(m_nLineItemID != -1)
						sqlWhere.Create("PersonID = {INT} AND LineItemID = {INT}", m_id, m_nLineItemID);
					else if(m_nMailID != -1) // (c.haag 2010-07-01 13:45) - PLID 39473 - Added m_nMailID
						sqlWhere.Create("PersonID = {INT} AND MailID = {INT}", m_id, m_nMailID);
					else if(m_nLabResultID != -1)	// (j.dinatale 2010-12-22) - PLID 41885 - added m_nLabResultID
						sqlWhere.Create("PersonID = {INT} AND LabResultID = {INT}", m_id, m_nLabResultID);
					else if(m_nLabID != -1)	// (j.dinatale 2010-12-22) - PLID 41885 - added m_nLabID
						sqlWhere.Create("PersonID = {INT} AND LabID = {INT}", m_id, m_nLabID);
					else if(m_vtRecallID != g_cvarNull)
						sqlWhere.Create("PersonID = {INT} AND RecallID = {VT_I4}", m_id, m_vtRecallID);
					else if (m_nPatientRemindersSentID != -1)	//(s.dhole 8/29/2014 10:52 AM ) - PLID  63516
						sqlWhere.Create("PersonID = {INT} AND PatientRemindersSentID = {INT}", m_id, m_nPatientRemindersSentID);
					else if (m_nApptID != -1)  // (j.gruber 2014-12-12 15:00) - PLID 64391 - Rescheduling Queue - attached appointments to notes
						sqlWhere.Create("PersonID = {INT} AND AppointmentID = {INT}", m_id, m_nApptID);					
					else {
						sqlWhere.Create("PersonID = {INT}", m_id);

						// (b.spivey, October 05, 2012) - PLID 30398 - build a where clause to hide billing notes. 
						CString str = "";
						CString strWhere = "";

						//other billing notes
						if(m_checkHideBills.IsWindowEnabled() && !m_checkHideBills.GetCheck()) {
							str =	" AND CASE "
									"	WHEN ((BillID Is Not Null OR LineItemID Is Not Null) "
									"	AND Coalesce(ShowOnStatement,0) = 0 AND Coalesce(SendOnClaim,0) = 0) "
									"	THEN 1 ELSE 0 "
									"END = 0 ";
							strWhere += str;
							str = "";
						}

						// (b.spivey, October 17, 2012) - PLID 30398 - Filter ONLY claims or ONLY statements. 
						//both claim and statement
						if ((m_checkHideClaims.IsWindowEnabled() == TRUE && !m_checkHideClaims.GetCheck()) 
							&& (m_checkHideStatements.IsWindowEnabled() == TRUE && !m_checkHideStatements.GetCheck())) {

							str =	" AND "
									" CASE WHEN ((ShowOnStatement = 1 OR SendOnClaim = 1)) "
									" THEN 1 ELSE 0 "
									" END = 0 ";
							strWhere += str;
							str = "";
							
						}
						//claim notes
						else if(m_checkHideClaims.IsWindowEnabled() == TRUE && !m_checkHideClaims.GetCheck()) {
							// (b.spivey, October 18, 2012) - PLID 30398 - ShowOnStatement can be null
							str =	" AND 	"
									" CASE WHEN ((SendOnClaim = 1) AND (SendOnClaim <> ShowOnStatement OR ShowOnStatement IS NULL)) "
									" THEN 1 ELSE 0 "
									" END = 0 ";
							strWhere += str;
							str = "";
						}
						//statement notes
						else if(m_checkHideStatements.IsWindowEnabled() == TRUE && !m_checkHideStatements.GetCheck()) {
							// (b.spivey, October 18, 2012) - PLID 30398 - SendOnClaim can be null
							str =	" AND "
									" CASE WHEN ((ShowOnStatement = 1) AND (SendOnClaim <> ShowOnStatement OR SendOnClaim IS NULL)) "
									" THEN 1 ELSE 0 "
									" END = 0";
							strWhere += str;
							str = "";
						}
						sqlWhere += CSqlFragment(strWhere);
					}
					m_pNxDlNotes->PutWhereClause(_bstr_t(sqlWhere.Flatten()));
					m_pNxDlNotes->Requery();
				}
				else {
					if(!m_arUnsavedChargeNotes.IsEmpty()) {
						for(int i = 0; i < m_arUnsavedChargeNotes.GetCount(); i++) {
							NXDATALIST2Lib::IRowSettingsPtr pRow = m_pNxDlNotes->GetNewRow();
							pRow->PutValue(COLUMN_ID, m_arUnsavedChargeNotes.GetAt(i).varNoteID);
							pRow->PutValue(COLUMN_USER_ID, m_arUnsavedChargeNotes.GetAt(i).varUserID);
							// (a.walling 2010-07-13 17:59) - PLID 39182
							pRow->PutValue(COLUMN_NOTE_INPUT_DATE, m_arUnsavedChargeNotes.GetAt(i).varDate);

							//PLID 18881 r.galicki 6/12/08 - default color setting
							COLORREF rowColor;
							// (r.galicki 2008-06-27 12:26) - PLID 18881 - Color new row, if preference set
							if(1 == GetRemotePropertyInt("ColorizeNotes", 1, 0, GetCurrentUserName(), TRUE)) {
								CString strPriority;
								COLORREF defaultColor;
								switch ((long)m_arUnsavedChargeNotes.GetAt(i).varPriority) {
								case 1:
									strPriority = "NotesPriorityHigh";
									defaultColor = DEFAULT_COLOR_HIGH;
									break;
								case 2:
									strPriority = "NotesPriorityMedium";
									defaultColor = DEFAULT_COLOR_MEDIUM;
									break;
								case 3: default:
									strPriority = "NotesPriorityLow";
									defaultColor = DEFAULT_COLOR_LOW;
									break;
								}
								rowColor = (COLORREF)GetRemotePropertyInt(strPriority, defaultColor, 0, GetCurrentUserName(), TRUE);
							}
							else {
								rowColor = 0;
							}

							pRow->PutValue(COLUMN_COLOR, (long)rowColor);
							pRow->ForeColor = rowColor;
							pRow->PutValue(COLUMN_DATE, m_arUnsavedChargeNotes.GetAt(i).varDate);
							// if we have a category, set it
							if(VarLong(m_arUnsavedChargeNotes.GetAt(i).varCategory, -1) > 0) {
								pRow->PutValue(COLUMN_CATEGORY, m_arUnsavedChargeNotes.GetAt(i).varCategory);
							}
							pRow->PutValue(COLUMN_PRIORITY, m_arUnsavedChargeNotes.GetAt(i).varPriority); //PLID 18881 r.galicki 6/12/08 - default priority setting
							// (a.walling 2011-05-10 09:16) - PLID 41789 - Whether to show on statement
							pRow->PutValue(COLUMN_SHOW_ON_STATEMENT, m_arUnsavedChargeNotes.GetAt(i).varStatement); // (j.gruber 2010-06-14 14:16) - PLID 39153 - added show on statement
							// (j.jones 2011-09-19 14:25) - PLID 42135 - added Claim checkbox, NULL unless a charge
							pRow->PutValue(COLUMN_SEND_ON_CLAIM, m_arUnsavedChargeNotes.GetAt(i).varClaim);
							pRow->PutValue(COLUMN_NOTE, m_arUnsavedChargeNotes.GetAt(i).varNote);
							// (r.gonet 07/30/2014) - PLID 62525
							pRow->PutValue(COLUMN_BILLSTATUSNOTE_BILLID, g_cvarNull);
							pRow = m_pNxDlNotes->AddRowSorted(pRow, NULL);

							// (d.singleton 2012-03-30 15:59) - PLID 49257 since we are manually adding the data the rows never requery and never fill the category
							//  and priority drop down menus,  so we must do that manually as well.
							NXDATALIST2Lib::IFormatSettingsPtr pfsCategory(__uuidof(NXDATALIST2Lib::FormatSettings));
							NXDATALIST2Lib::IFormatSettingsPtr pfsPriority(__uuidof(NXDATALIST2Lib::FormatSettings));
							//Category First
							pfsCategory->PutDataType(VT_I4);
							pfsCategory->PutFieldType(NXDATALIST2Lib::cftComboSimple);
							pfsCategory->PutEditable(VARIANT_TRUE);
							pfsCategory->PutConnection(_variant_t((LPDISPATCH)GetRemoteData())); //we're going to let this combo use Practice's connection
							pfsCategory->PutComboSource(_bstr_t("SELECT -25 AS ID, '' AS Description UNION ALL SELECT ID, Description FROM NoteCatsF ORDER BY Description ASC"));
							pfsCategory->EmbeddedComboDropDownMaxHeight = 200;
							pfsCategory->EmbeddedComboDropDownWidth = 150;
							//now priority
							pfsPriority->PutDataType(VT_I2);
							pfsPriority->PutFieldType(NXDATALIST2Lib::cftComboSimple);
							pfsPriority->PutEditable(VARIANT_TRUE);
							pfsPriority->PutConnection(_variant_t((LPDISPATCH)GetRemoteData())); //we're going to let this combo use Practice's connection
							pfsPriority->PutComboSource(_bstr_t("SELECT 1, 'High' UNION SELECT 2,'Medium' UNION SELECT 3,'Low'"));
							pfsPriority->EmbeddedComboDropDownMaxHeight = 200;
							pfsPriority->EmbeddedComboDropDownWidth = 75;

							//set the format overrides
							pRow->PutRefCellFormatOverride(COLUMN_CATEGORY, pfsCategory);
							pRow->PutRefCellFormatOverride(COLUMN_PRIORITY, pfsPriority);
						}
					}
				}
			}
		}

		// (a.walling 2010-10-13 16:12) - PLID 40977 - Moved to LoadList
		//DRT 11/2/2005 - In Internal only, prompt for a referral if this prospect hasn't been contacted in past 3 months
		AttemptPromptForReferral();
		m_ForceRefresh = false;
	}NxCatchAll(__FUNCTION__);
}



CString CNotesDlg::GenerateSQL()
{
/*
	CString	tmpStartDate, 
			tmpEndDate, 
			strWhere;

	m_pNxDlNotes->Requery();


	else m_NotesList.RefreshContents();
	return tmpSQL;
	*/
	return "";
}

void CNotesDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CNotesDlg::Hotkey(int key)
{
//	switch(key) removed this hotkey
//	{	case VK_RETURN:
//			OnClickBtnAdd();
//	}

	//unhandled
	return 1;
}

// (a.walling 2010-09-20 11:41) - PLID 40589 - Set the patient ID
void CNotesDlg::SetPersonID(long nID)
{
	ASSERT(!GetSafeHwnd()); // must not have already been created!

	m_id = nID;
	// (a.walling 2010-09-20 11:41) - PLID 40589 - if m_id set manually, GetActivePersonID will always return m_id
	m_bIsValidID = true;
}

void CNotesDlg::OnRButtonUpNxdlnotes(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	//if(m_IsComboVisible){
	//	m_NotesList.ComboHide();
	//	m_IsComboVisible = FALSE;}

	//MSC 5/23/03 - Gray out the Add Note Option if they don't have permission
	bool bAllowAddNote = true;
	if(!(GetCurrentUserPermissions(bioPatientNotes) & (sptCreate|sptCreateWithPass)))
	{
		bAllowAddNote = false;
	}
	else
	{
		bAllowAddNote = true;
	}

	CMenu menPopup;
	menPopup.m_hMenu = CreatePopupMenu();
	// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.
	if(lpRow == NULL){
		menPopup.InsertMenu(1, MF_BYPOSITION|(bAllowAddNote ? 0:MF_GRAYED), IDM_ADD, "Add Note");
		menPopup.InsertMenu(2, MF_BYPOSITION, IDM_PRINT_NOTE, "Print Notes");
		menPopup.InsertMenu(3, MF_BYPOSITION|(bAllowAddNote ? 0:MF_GRAYED), IDM_ADD_MACRO, "Add Macro");
		m_bWhiteAdd =true;
	}else{
		// Build a menu popup with the ability to delete the current row
		// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.
		m_pNxDlNotes->PutCurSel(NXDATALIST2Lib::IRowSettingsPtr(lpRow));
		
		//DRT 4/28/03 - Gray this out if they don't have permission
		if (!(GetCurrentUserPermissions(bioPatientNotes) & (sptDelete|sptDeleteWithPass)))
			menPopup.InsertMenu(0, MF_BYPOSITION|MF_GRAYED, IDM_DELETE_NOTE, "Delete Note");
		else
			menPopup.InsertMenu(0, MF_BYPOSITION, IDM_DELETE_NOTE, "Delete Note");		
				
		menPopup.InsertMenu(1, MF_BYPOSITION|(bAllowAddNote ? 0:MF_GRAYED), IDM_ADD, "Add Note");
		menPopup.InsertMenu(3, MF_BYPOSITION|(bAllowAddNote ? 0:MF_GRAYED), IDM_ADD_MACRO, "Add Macro");
				
		menPopup.InsertMenu(2, MF_BYPOSITION, IDM_PRINT_NOTE, "Print Notes");

		// (z.manning 2010-05-04 13:09) - PLID 36511 - Added options to print/preview a single note
		menPopup.AppendMenu(MF_SEPARATOR);
		menPopup.AppendMenu(MF_ENABLED, IDM_PREVIEW_SINGLE_NOTE, "Preview Single Note");
		menPopup.AppendMenu(MF_ENABLED, IDM_PRINT_SINGLE_NOTE, "Print Single Note");

		m_bWhiteAdd = false;
	}
	

	CPoint pt;
	pt.x = x;
	pt.y = y;
	CWnd* dlNotes = GetDlgItem(IDC_NXDLNOTES);
	if (dlNotes != NULL) {
		dlNotes->ClientToScreen(&pt);
		menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
	}
	else {
		HandleException(NULL, "An error ocurred while creating menu");
	}




}


BOOL CNotesDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	try {

		BOOL bPreview = FALSE;
		switch (wParam) {

			case IDM_ADD:
				AddNewNote();
			break;

			case IDM_PRINT_NOTE:
				//if not called from the billing tab
				// (c.haag 2010-07-01 13:45) - PLID 39473 - Added m_nMailID
				// (j.dinatale 2011-01-03) - PLID 41975 - added LabID and LabResultID
				//(s.dhole 8/29/2014 12:24 PM ) - PLID 63516 m_nPatientRemindersSentID
				// (j.gruber 2014-12-12 15:00) - PLID 64391 - Rescheduling Queue - attached appointments to notes
				// (r.gonet 2015-01-20 15:40) - PLID 63490 - Check if we have a recall.
				if (m_nBillID == -1 && m_nLineItemID == -1 && m_nMailID == -1 && m_nLabResultID == -1 && m_nLabID == -1 && m_vtRecallID == g_cvarNull && m_nPatientRemindersSentID == -1 && m_nApptID == -1) {
					((CPatientView *)GetParent())->OnFilePrint();
				}
				else {
					//called from the billing tab, so just print those notes

					CAdvPrintOptions PrintOption(this);

					CString strWhere;
					if(m_nBillID != -1)
						strWhere.Format("Notes.BillID = %li",m_nBillID);
					else if(m_nMailID != -1) // (c.haag 2010-07-01 13:45) - PLID 39473 - Added m_nMailID
						strWhere.Format("Notes.MailID = %li",m_nMailID);
					else if(m_nLineItemID != -1)
						strWhere.Format("Notes.LineItemID = %li",m_nLineItemID);
					else if (m_nLabResultID != -1)
						strWhere.Format("Notes.LabResultID = %li", m_nLabResultID);
					else if (m_nLabID != -1)
						strWhere.Format("Notes.LabID = %li", m_nLabID);
					else if (m_vtRecallID != g_cvarNull) // (r.gonet 2015-01-20 15:40) - PLID 63490 - If we have a recall, then filter the notes on the recall ID.
						strWhere.Format("Notes.RecallID = %li", VarLong(m_vtRecallID, -1));
					else if (m_nPatientRemindersSentID != -1)//(s.dhole 8/29/2014 4:02 PM ) - PLID 63516
						strWhere.Format("Notes.PatientRemindersSentID = %li", m_nPatientRemindersSentID);
					else if (m_nApptID != -1) // (j.gruber 2014-12-12 15:00) - PLID 64391 - Rescheduling Queue - attached appointments to notes
						strWhere.Format("Notes.AppointmentID = %li", m_nApptID);
					
					try{					
						PrintOption.m_btnCaption = "Print";
						PrintOption.m_bDateRange = true;
						PrintOption.m_bAllOptions = true;
						PrintOption.m_bDetailed = false;
						PrintOption.m_bOptionCombo = true;
						PrintOption.m_cBoundCol		 = 1;
						PrintOption.m_cAllCaption	 = "All Categories";
						PrintOption.m_cSingleCaption = "One Category";

						// set up from date
						_RecordsetPtr rs = CreateRecordset("SELECT Min(Notes.Date) AS FromDate FROM Notes WHERE %s", strWhere);
						//when you select the Min of a value, if there are no records, it will return a VT_NULL variant as the only record
						//so you can't use the rs->eof flag to filter on this
						if (rs->Fields->Item["FromDate"]->Value.vt == VT_NULL) {
							MessageBox("No Notes for this Patient");
							rs->Close();
							return FALSE;
						}
						rs->MoveFirst();
						COleDateTime dateTmp = AdoFldDateTime(rs, "FromDate");
						PrintOption.m_dtInitFrom = dateTmp;
						rs->Close();

						// set up to date
						rs = CreateRecordset("SELECT Max(Notes.Date) AS ToDate FROM Notes WHERE %s", strWhere);
						//when you select the Min of a value, if there are no records, it will return a VT_NULL variant as the only record
						//so you can't use the rs->eof flag to filter on this
						if (rs->Fields->Item["ToDate"]->Value.vt == VT_NULL) {
							MessageBox("No Notes for this Patient");
							rs->Close();
							return FALSE;
						}
						rs->MoveFirst();
						dateTmp = AdoFldDateTime(rs, "ToDate");
						PrintOption.m_dtInitTo = dateTmp;
						rs->Close();

						//set up the datalist
						PrintOption.m_cSQL = "(SELECT ID, Description AS Text FROM NoteCatsF) as CatQ";

					}NxCatchAll("Error printing notes.");
				
					long nResult = PrintOption.DoModal();

					if (nResult == IDOK) {	//run the report with the date / category filter
				
						COleDateTime  dateTo, dateFrom;
						CString ToDate, FromDate;

						CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(240)]);
						//add the category filter via the Extra filter
						infReport.nExtraID = PrintOption.m_nResult;
						//filter on the patient
						infReport.nPatient = m_id;
						//filter on the dates chosen
						if(PrintOption.m_bDateRange)
						{
							infReport.strDateOptions = "1;Note Date;Date;;";
							infReport.nDateFilter = 1;
							infReport.nDateRange = 2;
							infReport.DateFrom = PrintOption.m_dtFromDate;
							infReport.DateTo   = PrintOption.m_dtToDate;
						}

						infReport.SetExtraValue(strWhere);

						//send directly to the printer
						CPrintDialog* dlg;
						dlg = new CPrintDialog(FALSE);
						CPrintInfo prInfo;
						prInfo.m_bPreview = false;
						prInfo.m_bDirect = false;
						prInfo.m_bDocObject = false;
						if (prInfo.m_pPD) delete prInfo.m_pPD;
						prInfo.m_pPD = dlg;

						//Made new function for running reports - JMM 5-28-04
						// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
						RunReport(&infReport, false, (CWnd *)this, "Patient Notes", &prInfo, TRUE);
					}
				}
			break;

			case IDM_ADD_MACRO:
				AddMacro();
			break;

			case IDM_ADD_NEW_THREAD:
				
				AddNewNote(-1, -1);
			break;

			case IDM_ADD_TO_SELECTED_THREAD:
				{
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMessagesList->CurSel;
					if (pRow) {
						//get the thread ID
						long nThreadID = VarLong(pRow->GetValue(mlcThreadID), -1);
						AddNewNote(-1, nThreadID);
					}
					else {
						MsgBox("Please choose a thread to add a note to");
					}
				}
			break;

			case IDM_ADD_NEW_THREAD_MACRO:
				
				AddMacro(-1);
			break;

			case IDM_ADD_TO_SELECTED_THREAD_MACRO:
				{
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMessagesList->CurSel;
					if (pRow) {
						//get the thread ID
						long nThreadID = VarLong(pRow->GetValue(mlcThreadID), -1);
						ASSERT(nThreadID != -1);
						AddMacro(nThreadID);
					}
					else {
						MsgBox("Please choose a thread to add a macro to");
					}
				}
			break;

			case IDM_DELETE_NOTE:
				OnDeleteNote();
			break;

			case IDM_DELETE_THREAD:
				{
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMessagesList->CurSel;
					if (pRow) {
						//get the thread ID
						long nThreadID = VarLong(pRow->GetValue(mlcThreadID), -1);
						ASSERT(nThreadID != -1);
						OnDeleteNote(nThreadID);
					}
					else {
						MsgBox("Please choose a thread to delete.");
					}
				}
			break;		

			case IDM_PREVIEW_SINGLE_NOTE:
				bPreview = TRUE;
				// (z.manning 2010-05-04 13:18) - PLID 36511 - No break here, we want to continue.
			case IDM_PRINT_SINGLE_NOTE:

				// (z.manning 2010-05-04 13:26) - PLID 36511 - Print the patient notes report for the currently
				// selected single note.
				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(240)]);
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pNxDlNotes->GetCurSel();
				if(pRow != NULL) {
					long nNoteID = VarLong(pRow->GetValue(0));
					infReport.SetExtraValue(FormatString("Notes.ID = %li", nNoteID));

					//send directly to the printer
					CPrintDialog* dlg;
					dlg = new CPrintDialog(FALSE);
					CPrintInfo prInfo;
					prInfo.m_bPreview = bPreview;
					prInfo.m_bDirect = FALSE;
					prInfo.m_bDocObject = FALSE;
					if (prInfo.m_pPD) { delete prInfo.m_pPD; }
					prInfo.m_pPD = dlg;

					//Made new function for running reports - JMM 5-28-04
					RunReport(&infReport, bPreview, this, "Patient Notes", &prInfo);

					// (z.manning 2010-07-06 17:22) - PLID 36511 - If we're previewing and the dialog is modal, close it
					if(bPreview && m_bIsModal) {
						// (j.dinatale 2011-01-21) - PLID 41975 - need to always minimize the labs follow up dialog
						if(GetMainFrame()->GetSafeHwnd() && GetMainFrame()->m_pLabFollowupDlg->GetSafeHwnd()){
							GetMainFrame()->m_pLabFollowupDlg->ShowWindow(SW_MINIMIZE);
						}

						EndDialogAndModalParent(IDOK);						
					}
				}
			break;

			return true;
		}
	}NxCatchAll(__FUNCTION__);

	return CNxDialog::OnCommand(wParam, lParam);
}





void CNotesDlg::OnEditingFinishedNxdlnotes(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.

	

	try {

		if (!bCommit) {
			return;
		}

		// (a.walling 2010-07-13 18:11) - PLID 39182 - Check permissions and etc again to see if we can edit this note
		bool bRequireSameDayCheck = false;
		if (!CanEditNote(lpRow, bRequireSameDayCheck, true, nCol)) { // silent this time		
			ThrowNxException("The current user does not have permission to modify this note!");
		}
		// if bRequireSameDayCheck, caller must check same day in a query since the client machine can't be trusted. However it is still possible
		// that the query text could be edited before it goes to the server. But if someone is that capable, they could probably get into the 
		// database anyway.

		// (a.walling 2010-07-16 09:53) - PLID 39182 - Batch everything!
		CParamSqlBatch batch;
		CAuditTransaction auditTransaction;
		bool bUpdateView = false;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		// (d.singleton 2012-03-27 12:18) - PLID 49257
		int nChargeNoteIndex;
		for(int i = 0; i < m_arUnsavedChargeNotes.GetCount(); i++) {
			if(m_arUnsavedChargeNotes.GetAt(i).varNoteID == pRow->GetValue(COLUMN_ID)) {
				nChargeNoteIndex = i;
				break;
			}
		}
		switch (nCol) {
			//category
			case COLUMN_CATEGORY:
				// (a.walling 2010-07-16 09:41) - PLID 39182 - Check bCommit
				if (bCommit) {
					if(varNewValue.vt!=VT_NULL && varNewValue.vt != VT_EMPTY) {
						// (d.singleton 2012-03-27 12:55) - PLID 49257
						if(m_bUnsavedCharge) {
							m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varCategory = varNewValue;
							break;
						}
						if (VarLong(varNewValue) == -25) {
							//no selection
							batch.Add("UPDATE Notes SET Category = NULL WHERE ID = {INT}",VarLong(pRow->GetValue(0)));
						}
						else {
							batch.Add("UPDATE Notes SET Category = {INT} WHERE ID = {INT}",VarLong(varNewValue),VarLong(pRow->GetValue(0)));
						}
					}

					//update the category filter if it's active
					if(IsDlgButtonChecked(IDC_CATEGORY_FILTER)) {
						bUpdateView = true;
					}
				}

			break;
			//Note
			case COLUMN_NOTE:
				if (bCommit) {
					if (varOldValue.vt == VT_EMPTY || VarString(varOldValue) != VarString(varNewValue)) {
						if (m_bSplit)  {// find the new row
							// (d.singleton 2012-03-30 09:25) - PLID 49257 already handle split notes for unsaved charges in the OnFinishing handler
							if(!m_bUnsavedCharge) {
								NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pNxDlNotes->FindByColumn(COLUMN_NOTE, _variant_t(varNewValue), 0, false);
								if (pNewRow == NULL) { ThrowNxException("The index of the unsplit item could not be found!"); }
								// (a.walling 2006-07-17 16:58) - PLID 20206 Set the date so we can have them be in order in the data
								batch.Add("UPDATE Notes SET Note = {STRING}, Date = {OLEDATETIME} WHERE ID = {INT}",
									VarString(varNewValue), VarDateTime(pNewRow->GetValue(COLUMN_DATE)), VarLong(pNewRow->GetValue(0)));
								m_bSplit = false;
								m_pNxDlNotes->Sort();
							}
						}
						else {
							// (d.singleton 2012-03-27 14:58) - PLID 49257
							if(m_bUnsavedCharge) {
								m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varNote = varNewValue;
							}
							else {
								batch.Add("UPDATE Notes SET Note = {STRING} WHERE ID = {INT}",
									VarString(varNewValue), VarLong(pRow->GetValue(0)));
							}
						}

						CString strNew = CString(varNewValue.bstrVal);
						CString strOld = CString(varOldValue.bstrVal);
						long nNoteID = VarLong(pRow->GetValue(0));

						// (a.walling 2010-09-09 12:22) - PLID 40267
						if(m_bIsForPatient && varNewValue.bstrVal != varOldValue.bstrVal) {
							// (j.gruber 2009-12-31 13:08) - PLID 36310 - change audit ID to be the note ID
							AuditEvent(GetActivePersonID(), GetActivePersonName(), auditTransaction, aeiPatientNote, nNoteID, strOld, strNew, aepMedium, aetChanged);
						}
					}

				}
			break;
			case COLUMN_DATE:
				if(bCommit){
					// (a.walling 2010-07-16 09:49) - PLID 39182 - This is all handled in CanEditNote now.
					/*
					if (!CheckCurrentUserPermissions(bioPatientNotes, sptDynamic0, FALSE, 0, TRUE, TRUE)) {
						// (b.cardillo 2005-04-29 12:52) - PLID 11489 - Just before we write to the data we 
						// silently check to ensure this user has permission to change the date.  If not we 
						// throw an exception.  We don't give a message box here, because it's the caller's 
						// responsibility to make sure the user gets the appropriate UI that cleanly lets the 
						// user know he can't change the date.  This is only a last line of defense against 
						// illegal changing of the date.
						ThrowNxException("The current user does not have permission to change the date on this note!");
					}
					*/
					long nID = VarLong(pRow->GetValue(0));
					CString strOld = FormatDateTimeForInterface(VarDateTime(GetTableField("Notes","[Date]","ID",nID)), NULL, dtoNaturalDatetime, false);
					CString strNew = FormatDateTimeForInterface(VarDateTime(varNewValue), NULL, dtoNaturalDatetime, false);
					// (c.haag 2007-03-05 15:06) - PLID 25067 - Do nothing if the date did not change
					if (strOld != strNew) {
						// (d.singleton 2012-03-27 15:10) - PLID 49257
						if(m_bUnsavedCharge) {
							m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varDate = VarDateTime(varNewValue);
						}
						else {
							batch.Add("UPDATE Notes SET Date = {OLEDATETIME} WHERE ID = {INT}", VarDateTime(varNewValue), VarLong(pRow->GetValue(0)));
							// (c.haag 2007-03-05 15:07) - PLID 25067 - Audit the change
							// (a.walling 2010-09-09 12:22) - PLID 40267
							if (m_bIsForPatient) {
								AuditEvent(GetActivePersonID(), GetActivePersonName(), auditTransaction, aeiPatientNoteDate, nID, strOld, strNew, aepMedium, aetChanged);
							}
						}
					}
				}
			break;
			// (r.galicki 2008-06-25 14:53) - PLID 18881 - Added check for editing priority
			case COLUMN_PRIORITY:
				if (bCommit) {
					if(varNewValue.vt!=VT_NULL && varNewValue.vt != VT_EMPTY) {
						// (d.singleton 2012-03-27 15:12) - PLID 49257
						if(m_bUnsavedCharge) {
							m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varPriority = VarShort(varNewValue);
						}
						else {
							batch.Add("UPDATE Notes SET Priority = {INT} WHERE ID = {INT}",VarShort(varNewValue),VarLong(pRow->GetValue(COLUMN_ID)));
						}
						//R.G. - Manually update colors of the rows.  The reasoning behind this is an apparent bug in the implementation of the 
						//color column type.  Whenever the color column itself is updated, the change in text color will not be reflected until
						//the list is refreshed.  As a result, the color must be manually update to reflect the change in priority.
						// (a.walling 2010-07-16 09:57) - PLID 39182 - This bug has bitten me several times and always annoys me.
						if(1 == GetRemotePropertyInt("ColorizeNotes", 1, 0, GetCurrentUserName(), TRUE))
						{
							//TES 2/27/2009 - PLID 32915 - We also need to set the color column, otherwise splitting won't work properly.
							long nColor = 0;
							switch(VarShort(varNewValue)) {
								case PRIORITY_HIGH:
									nColor = GetRemotePropertyInt("NotesPriorityHigh", DEFAULT_COLOR_HIGH, 0, GetCurrentUserName(), TRUE);
									break;
								case PRIORITY_MEDIUM:
									nColor = GetRemotePropertyInt("NotesPriorityMedium", DEFAULT_COLOR_MEDIUM, 0, GetCurrentUserName(), TRUE);
									break;
								case PRIORITY_LOW: default:
									nColor = GetRemotePropertyInt("NotesPriorityLow", DEFAULT_COLOR_LOW, 0, GetCurrentUserName(), TRUE);
									break;
							}
							pRow->PutForeColor(nColor);
							pRow->PutValue(COLUMN_COLOR, nColor);
						}
					}
				}
				break;
			case COLUMN_SHOW_ON_STATEMENT:
				// (j.gruber 2010-06-15 10:48) - PLID 39153 - added statement to billing notes 
				if (bCommit) {
					if (m_bIsBillingNote) {
						if(m_bUnsavedCharge) {
							// (d.singleton 2012-03-27 15:23) - PLID 49257 set the row statement = true,  change all other rows to false
							m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varStatement = VarBool(pRow->GetValue(COLUMN_SHOW_ON_STATEMENT));
							for(int i = 0; i < m_arUnsavedChargeNotes.GetCount(); i++) {
								if(i != nChargeNoteIndex) {
									m_arUnsavedChargeNotes.GetAt(i).varStatement = g_cvarFalse;
								}
							}
						}
						else {
							//first we need to set all the notes for this object to 0
							// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
							if (m_nBillID != -1) {
								batch.Add("UPDATE NoteInfoT SET ShowOnStatement = 0 WHERE BillID = {INT}; \r\n ", m_nBillID);
								batch.Add("UPDATE NoteInfoT SET ShowOnStatement = {INT} WHERE NoteID = {INT}", VarBool(pRow->GetValue(COLUMN_SHOW_ON_STATEMENT), FALSE) ? 1 : 0, VarLong(pRow->GetValue(COLUMN_ID)));
							}
							else if (m_nLineItemID != -1) {
								batch.Add("UPDATE NoteInfoT SET ShowOnStatement = 0 WHERE LineItemID = {INT}; \r\n ", m_nLineItemID);
								batch.Add("UPDATE NoteInfoT SET ShowOnStatement = {INT} WHERE NoteID = {INT}", VarBool(pRow->GetValue(COLUMN_SHOW_ON_STATEMENT), FALSE) ? 1 : 0, VarLong(pRow->GetValue(COLUMN_ID)));
							}
						}
						//set all the other rows to have a false check
						NXDATALIST2Lib::IRowSettingsPtr pLoopRow = m_pNxDlNotes->GetFirstRow();
						while (pLoopRow) {
							if (pLoopRow != pRow) {
								pLoopRow->PutValue(COLUMN_SHOW_ON_STATEMENT, g_cvarFalse);
							}
							pLoopRow = pLoopRow->GetNextRow();
						}
					}
					else {
						//wha?
						ASSERT(FALSE);
					}
				}
			break;

			// (j.jones 2011-09-19 14:21) - PLID 42135 - added the Claim checkbox
			case COLUMN_SEND_ON_CLAIM:
				if (bCommit) {
					if(m_bIsBillingNote && m_bntBillingNoteType == bntCharge) {
						// (d.singleton 2012-03-27 15:33) - PLID 49257 set this row claim to true,  all others to false
						if(m_bUnsavedCharge) {
							m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varClaim = VarBool(pRow->GetValue(COLUMN_SEND_ON_CLAIM));
							for(int i = 0; i < m_arUnsavedChargeNotes.GetCount(); i++) {
								if(i != nChargeNoteIndex) {
									m_arUnsavedChargeNotes.GetAt(i).varClaim = g_cvarFalse;
								}
							}
						}
						else {
							//first we need to set all the notes for this object to 0, as you can only
							//check one note per charge
							if(m_nLineItemID != -1) {
								batch.Add("UPDATE NoteInfoT SET SendOnClaim = 0 WHERE LineItemID = {INT}; \r\n ", m_nLineItemID);
								batch.Add("UPDATE NoteInfoT SET SendOnClaim = {INT} WHERE NoteID = {INT}", VarBool(pRow->GetValue(COLUMN_SEND_ON_CLAIM), FALSE) ? 1 : 0, VarLong(pRow->GetValue(COLUMN_ID)));
							}
						}
						//set all the other rows to have a false check
						NXDATALIST2Lib::IRowSettingsPtr pLoopRow = m_pNxDlNotes->GetFirstRow();
						while (pLoopRow) {
							if (pLoopRow != pRow) {
								pLoopRow->PutValue(COLUMN_SEND_ON_CLAIM, g_cvarFalse);
							}
							pLoopRow = pLoopRow->GetNextRow();
						}
					}
					else {
						//should be impossible
						ASSERT(FALSE);
					}
				}
			break;
		}

		if (!batch.IsEmpty()) {
			// commit sql and audit transactions
			if (bRequireSameDayCheck) {
				long nNoteID = VarLong(pRow->GetValue(COLUMN_ID));

				batch.Prepend(
					"IF ( "
					"dbo.AsDateNoTime(CONVERT(DATETIME, (SELECT NoteInputDate FROM Notes WHERE ID = {INT}))) "
						"<> "
						"dbo.AsDateNoTime(CONVERT(DATETIME, GetDate())) "
					") "
					"BEGIN "
						"RAISERROR('This note may not be edited outside of the same numeric date that it was entered.', 16, 31) "
						"ROLLBACK TRAN RETURN \r\n" // (a.walling 2011-01-27 13:34) - PLID 34813 - Rollback
					"END", nNoteID);
			}

			batch.Execute(GetRemoteData());
			auditTransaction.Commit();
		}

		if (bUpdateView) {
			UpdateView();
		}
	}NxCatchAllCall("Error in editing note.", { UpdateView(); });	
}


void CNotesDlg::OnColumnClickingNxdlnotes(short nCol, BOOL FAR* bAllowSort) 
{
	
}

void CNotesDlg::OnEditingStartedNxdlnotes(LPDISPATCH lpRow, short nCol, long nEditType) 
{
	// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.

	try {
		long nBegin, nEnd;
		m_pNxDlNotes->GetEditingHighlight(&nBegin, &nEnd);
		m_pNxDlNotes->SetEditingHighlight(nEnd, nEnd, TRUE);
	} NxCatchAll("CNotesDlg::OnEditingStartedNxdlnotes");
}

// (a.walling 2010-07-13 18:11) - PLID 39182 - Check permissions and etc to see if we can edit this note
bool CNotesDlg::CanEditNote(NXDATALIST2Lib::IRowSettingsPtr pCheckRow, bool &bRequireSameDayCheck, bool bSilent, short nCol)
{
	bRequireSameDayCheck = false;

	// (a.walling 2010-07-13 18:11) - PLID 39182 - Check for same-day 
	bool bHasSameDay = (GetCurrentUserPermissions(bioPatientNotes) & (sptDynamic1|sptDynamic1WithPass)) ? true : false;

	bool bCanWrite = (GetCurrentUserPermissions(bioPatientNotes) & (sptWrite|sptWriteWithPass)) ? true : false;

	// (r.gonet 07/22/2014) - PLID 62525 - They can't edit bill status notes (aside from non-essentials) except from the bill dialog.
	long nBillStatusNote_BillID = VarLong(pCheckRow->GetValue(COLUMN_BILLSTATUSNOTE_BILLID), -1);
	if (nBillStatusNote_BillID != -1 && (nCol == COLUMN_DATE || nCol == COLUMN_NOTE)) {
		return false;
	}

	// there is a permission for modifying the date; must also have write permission.
	if (nCol == COLUMN_DATE) {
		if (CheckCurrentUserPermissions(bioPatientNotes, sptDynamic0, FALSE, 0, bSilent ? TRUE : FALSE, bSilent ? TRUE : FALSE) && 
			CheckCurrentUserPermissions(bioPatientNotes, sptWrite, FALSE, 0, bSilent ? TRUE : FALSE, bSilent ? TRUE : FALSE)) {
			return true;
		} else {
			return false;
		}
	}
	//(e.lally 2011-06-17) PLID 43963 - Added permission specific to priority so that some users can set the category but not change notes,
	//	therefore it is independent of the write permission
	else if(nCol == COLUMN_CATEGORY) {
		bool bCanChangeCategory = (GetCurrentUserPermissions(bioPatientNotes) & (sptDynamic2|sptDynamic2WithPass)) ? true : false;
		if (bCanChangeCategory && CheckCurrentUserPermissions(bioPatientNotes, sptDynamic2, FALSE, 0, bSilent ? TRUE : FALSE, bSilent ? TRUE : FALSE)) {
			return true;
		} else {
			if(bCanChangeCategory){
				//They can change the category (with pass), but the check failed, so we have to fail
				return false;
			}
			//They couldn't change the category, so we'll see if the same day edit allows them to further down
		}
	}
	//(e.lally 2011-06-17) PLID 43963 - Added permission specific to priority so that some users can set the priority but not change notes,
	//	therefore it is independent of the write permission
	else if(nCol == COLUMN_PRIORITY) {
		bool bCanChangePriority = (GetCurrentUserPermissions(bioPatientNotes) & (sptDynamic3|sptDynamic3WithPass)) ? true : false;
		if (bCanChangePriority && CheckCurrentUserPermissions(bioPatientNotes, sptDynamic3, FALSE, 0, bSilent ? TRUE : FALSE, bSilent ? TRUE : FALSE)) {
			return true;
		} else {
			if(bCanChangePriority){
				//They can change the priority (with pass), but the check failed, so we have to fail
				return false;
			}
			//They couldn't change the priority, so we'll see if the same day edit allows them to further down
		}
	}

	// (j.dinatale 2011-03-17 09:33) - PLID 42863 - We want the check for a new note to be AFTER the check for the date,
	//		because if they have the permission to create notes, we still want them to be able to fill out the note but we still want to
	//		deny the ability to edit the date column if they arent allowed to.
	if (m_bIsCreatingNewNote) {
		return true;
	}
	
	if (!bCanWrite && bHasSameDay) {
		if (pCheckRow == NULL) {
			return false;
		}

		// at this point, they don't have write permission. However, they have the same-day, same-user edit permission

		// check same user
		long nNoteOwnerID = VarLong(pCheckRow->GetValue(COLUMN_USER_ID), -1);
		COleDateTime dtInputDate = VarDateTime(pCheckRow->GetValue(COLUMN_NOTE_INPUT_DATE), g_cdtNull);

		// cutting off the DATE value to an int (no rounding) will give us the numeric day.
		if ((nNoteOwnerID != GetCurrentUserID()) || (dtInputDate.GetStatus() != COleDateTime::valid) || (int(dtInputDate.m_dt) != int(GetRemoteServerTime().m_dt))) {
			if (!bSilent) {
				PermissionsFailedMessageBox(this, "You may only edit notes created by you on the same date that they were entered.\n");
			}
			return false;
		}

		// alright, same day, same user, so we can check for the same day permission in a fashion that may prompt for the password if necessary
		if (CheckCurrentUserPermissions(bioPatientNotes, sptDynamic1, FALSE, 0, bSilent ? TRUE : FALSE, bSilent ? TRUE : FALSE)) {
			bRequireSameDayCheck = true; // caller must check same day in a query since the client machine can't be trusted
			return true;
		}
	//(e.lally 2011-07-07) PLID 43963 - Check the write permission for the note column (and any not the priority, category, date)
	} else if (nCol != COLUMN_PRIORITY && nCol != COLUMN_CATEGORY && nCol != COLUMN_DATE) {
		if(CheckCurrentUserPermissions(bioPatientNotes, sptWrite, FALSE, 0, bSilent ? TRUE : FALSE, bSilent ? TRUE : FALSE)) {
			return true;
		}
	}

	//(e.lally 2011-07-07) PLID 43963 - Make sure we tell the user they don't have permission if we get this far
	PermissionsFailedMessageBox(this, "", "change this field");

	return false;
}

void CNotesDlg::OnEditingFinishingNxdlnotes(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	if(*pbCommit == FALSE)
		return;

	NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return;
	}

	// (a.walling 2010-07-13 18:11) - PLID 39182 - Break out of here if no change
	{
		// (j.dinatale 2011-03-17 11:34) - PLID 42863 - We have to determine if we are on the date column, there is a weird issue with the milliseconds of
		//		of the date value where the old and new values have different milliseconds.
		if(nCol == COLUMN_DATE){
			COleDateTime dtOldDate = VarDateTime(varOldValue, g_cdtInvalid);
			COleDateTime dtNewDate = VarDateTime(*pvarNewValue, g_cdtInvalid);

			if(dtOldDate.GetStatus() == COleDateTime::valid && dtNewDate.GetStatus() == COleDateTime::valid){
				if(dtOldDate.GetYear() == dtNewDate.GetYear() && dtOldDate.GetMonth() == dtNewDate.GetMonth() && dtOldDate.GetDay() == dtNewDate.GetDay()
					&& dtOldDate.GetHour() == dtNewDate.GetHour() && dtOldDate.GetMinute() == dtNewDate.GetMinute() && dtOldDate.GetSecond() == dtNewDate.GetSecond()){
					*pbCommit = FALSE;
					return;
				}
			}
		}else{
			_variant_t varOldValueCopy = varOldValue;
			if (varOldValueCopy == pvarNewValue) {			
				*pbCommit = FALSE;
				return;
			}
		}
	}

	// (r.gonet 07/22/2014) - PLID 62525 - They can't edit bill status notes (aside from non-essentials) except from the bill dialog.
	long nBillStatusNote_BillID = VarLong(pRow->GetValue(COLUMN_BILLSTATUSNOTE_BILLID), -1);
	if (nBillStatusNote_BillID != -1 && (nCol == COLUMN_DATE || nCol == COLUMN_NOTE)) {
		MessageBox("This note is a bill status note linked to a bill and cannot be edited or deleted outside of the bill.", "Error", MB_ICONERROR | MB_OK);
		*pbCommit = FALSE;
		return;
	}

	// (a.walling 2010-07-13 18:11) - PLID 39182 - Check permissions and etc to see if we can edit this note
	bool bRequireSameDayCheck = false;
	if (!CanEditNote(lpRow, bRequireSameDayCheck, false, nCol)) {
		*pbCommit = FALSE;
		return;
	}
	// if bRequireSameDayCheck, caller must check same day in a query since the client machine can't be trusted. However it is still possible
	// that the query text could be edited before it goes to the server. But if someone is that capable, they could probably get into the 
	// database anyway.

	// (d.singleton 2012-03-27 12:18) - PLID 49257
	int nChargeNoteIndex;
	for(int i = 0; i < m_arUnsavedChargeNotes.GetCount(); i++) {
		if(m_arUnsavedChargeNotes.GetAt(i).varNoteID == pRow->GetValue(COLUMN_ID)) {
			nChargeNoteIndex = i;
			break;
		}
	}

	switch (nCol) {

		case COLUMN_SEND_ON_CLAIM:
			// (j.jones 2011-09-19 15:39) - PLID 42135 - if the note is set to send on a claim
			// and is > 80 characters, warn about the length
			// (j.jones 2011-09-20 09:42) - PLID 44934 - do not warn if Alberta is enabled
			if(VarBool(pvarNewValue, FALSE)
				&& VarString(pRow->GetValue(COLUMN_NOTE), "").GetLength() > 80
				&& !UseAlbertaHLINK()) {

				if(IDNO == MsgBox(MB_YESNO, "This note is configured to send on an electronic claim, "
					"but the note you have entered is over the maximum size of 80 characters allowed on a claim.\n\n"
					"If you continue, only the first 80 characters will be sent on the claim. Are you sure you wish to send this note on the claim?")) {
					*pbCommit = FALSE;
					return;
				}
			}
			break;

		//Note
		case COLUMN_NOTE:
			try {
				//_variant_t var = AsBool(pRow->GetValue(COLUMN_SEND_ON_CLAIM));
				
				if (pvarNewValue->vt != VT_BSTR) {
					MsgBox("The text you entered is not valid. \n Your changes will not be saved");
					*pbCommit = FALSE;
				}
/*			DRT 10/9/02 - Removed restriction on blank note, it is valid.  See PL #5954
				else if(CString(pvarNewValue->bstrVal) == "") {
					MsgBox("You cannot enter an empty note.");
					*pbCommit = FALSE;
					*pbContinue = FALSE;
				}
*/				// (j.jones 2011-09-19 15:39) - PLID 42135 - if the note is set to send on a claim
				// and is > 80 characters, warn about the length
				// (j.jones 2011-09-20 09:42) - PLID 44934 - do not warn if Alberta is enabled
				
				else if(VarBool(pRow->GetValue(COLUMN_SEND_ON_CLAIM), FALSE)
					&& VarString(pvarNewValue, "").GetLength() > 80
					&& !UseAlbertaHLINK()) {

					if(IDNO == MsgBox(MB_YESNO, "This note is configured to send on an electronic claim, "
						"but the note you have entered is over the maximum size of 80 characters allowed on a claim.\n\n"
						"If you continue, only the first 80 characters will be sent on the claim. Are you sure you wish to save this note?")) {
						*pbCommit = FALSE;
						*pbContinue = FALSE;
						return;
					}
				}
				else if(CString(pvarNewValue->bstrVal).GetLength() > 4000) {
				//	MsgBox("The text you entered is longer than the maximum amount (4000) and has been shortened.\n"
				//		"Please double-check your note and make changes as needed.");
				//	pvarNewValue->bstrVal = CString(pvarNewValue->bstrVal).Left(4000).AllocSysString();
					//DRT 7/16/03 - If it's over 4000, we want to try to split the note into multiple notes, and insert them all
					int result = MsgBox(MB_YESNOCANCEL, "The note you have entered is over the maximum size of 4000 characters.  Would you like to split it into "
						"multiple notes?\n"
						"Pressing 'Yes' will split the note into multiple notes.\n"
						"Pressing 'No' will truncate the existing note.\n"
						"Pressing 'Cancel' will let you edit the note again.");

					if(result == IDCANCEL) {
						*pbCommit = FALSE;
						*pbContinue = FALSE;
						return;
					}

					if(result == IDNO) {
						// (a.walling 2012-05-17 17:07) - PLID 50481 - Fix BSTR leaks
						CString str(pvarNewValue->bstrVal);
						::VariantClear(pvarNewValue);
						*pvarNewValue = _variant_t(str.Left(4000)).Detach();
						// (d.singleton 2012-03-27 12:33) - PLID 49257
						if(m_bUnsavedCharge) {
							m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varNote = pvarNewValue->bstrVal;
						}
						return;
					}

					//handle the splitting!

					//TODO:  Try to find the period closest, etc and split there.  But for now,
					//		we just split them at the 4000 marks.

					m_bSplit = true;
					COleDateTime dtOldest;

					// (PLID 20206) a.walling 5/17/06 Fixed the splitting code, was pulling variables the wrong way.

					CString strNoteRem = CString(pvarNewValue->bstrVal);
					//TODO pvarNewValue->bstrVal = strNoteRem.Left(4000).AllocSysString();

					// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.
					_variant_t vardtNoteDate = pRow->GetValue(COLUMN_DATE);
					_variant_t varstrNoteOwnerID = pRow->GetValue(COLUMN_USER_ID);
					_variant_t varnNoteCategory = pRow->GetValue(COLUMN_CATEGORY);
					// (r.galicki 2008-06-26 15:45) - PLID 18881 - Carry over priority/color info
					_variant_t varnNotePriority = pRow->GetValue(COLUMN_PRIORITY);
					_variant_t varnNoteColor = pRow ->GetValue(COLUMN_COLOR);

					long nUserID = GetCurrentUserID();

					long nExistingNoteID = VarLong(pRow->GetValue(COLUMN_ID), -1);

					COleDateTime dt = VarDateTime(vardtNoteDate);

					// (a.walling 2010-11-01 11:07) - PLID 40965 - Parameterized
					_variant_t varCategory = g_cvarNull;
					if(varnNoteCategory.vt == VT_I4) //make sure there is a category
						varCategory = varnNoteCategory;

					_variant_t varBillID = m_nBillID != -1 ? _variant_t(m_nBillID, VT_I4) : g_cvarNull;
					_variant_t varLineItemID = m_nLineItemID != -1 ? _variant_t(m_nLineItemID, VT_I4) : g_cvarNull;
					// (c.haag 2010-07-01 13:45) - PLID 39473 - Added strMailID
					// (c.haag 2010-07-01 13:45) - PLID 39473 - Added m_nMailID
					_variant_t varMailID = m_nMailID != -1 ? _variant_t(m_nMailID, VT_I4) : g_cvarNull;

					// (j.dinatale 2010-12-22) - PLID 41885 - have new fields, added m_nLabResultID and m_nLabID
					_variant_t varLabResultID = m_nLabResultID != -1 ? _variant_t(m_nLabResultID, VT_I4) : g_cvarNull;
					_variant_t varLabID = m_nLabID != -1 ? _variant_t(m_nLabID, VT_I4) : g_cvarNull;

					// (j.gruber 2014-12-12 15:00) - PLID 64391 - Rescheduling Queue - attached appointments to notes
					_variant_t varApptID = m_nApptID != -1 ? _variant_t(m_nApptID, VT_I4) : g_cvarNull;
					
					CArray<CString, CString&> arNotes;
					while(strNoteRem.GetLength() > 4000) {
						CString strNew;

						//grab the left 4000
						strNew = strNoteRem.Left(4000);
						strNoteRem = strNoteRem.Right(strNoteRem.GetLength() - 4000);

						//add to array
						arNotes.Add(strNew);
					}
					//arNotes.Add(strNoteRem);

					// the first one goes here...
					::VariantClear(pvarNewValue);
					*pvarNewValue = _variant_t(strNoteRem).Detach();

					COleDateTimeSpan dtsSecond(0,0,0,1);
					COleDateTimeSpan dtsBackNotes(0,0,0, arNotes.GetSize() + 1);
					COleDateTime dtSplit = dt - dtsBackNotes;
					dtOldest = dtSplit;
					
					//insert it into the data with the same time + category
					// and add it to the datalist
					// we loop in reverse to give it a readable order
					// (a.walling 2010-07-26 16:48) - PLID 39182 - Check dates if necessary
					//if(!m_bUnsavedCharge) {
					for (int i = arNotes.GetSize() - 1; i >= 0; i--) {
						long nNewID;
						CString strInsertProcSql;

						dtSplit += dtsSecond;

						// (r.galicki 2009-01-15 16:07) - PLID 32750 - Priority is stored as a byte, not short, use VarByte(...)
						// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
						// (a.walling 2010-11-01 11:07) - PLID 40965 - Parameterized
						CParamSqlBatch batch;
						batch.Declare(
							"SET NOCOUNT ON "
							"DECLARE @nNewID int "
							"DECLARE @dtCurrent datetime ");
						if (bRequireSameDayCheck && nExistingNoteID != -1) {
							batch.Add(
								"IF ( "
								"dbo.AsDateNoTime(CONVERT(DATETIME, (SELECT NoteInputDate FROM Notes WHERE ID = {INT}))) "
								"<> "
								"dbo.AsDateNoTime(CONVERT(DATETIME, GetDate())) "
								") "
								"BEGIN "
								"RAISERROR('This note may not be edited outside of the same numeric date that it was entered.', 16, 31) "
								"ROLLBACK TRAN RETURN \r\n" // (a.walling 2011-01-27 13:34) - PLID 34813 - Rollback
								"END", nExistingNoteID);
						}
						batch.Add("SET @dtCurrent = GetDate() ");
						// (d.singleton 2012-03-27 12:42) - PLID 49257
						if(!m_bUnsavedCharge) {
							// (j.armen 2014-01-31 09:31) - PLID 60568 - Idenitate NoteDataT
							batch.Add("INSERT INTO Notes (PersonID, Date, UserID, Category, Note, Priority) "
								"VALUES ({INT}, {OLEDATETIME}, {INT}, {VT_I4}, {STRING}, {INT}) ",
								m_id, dtSplit, nUserID, varCategory, arNotes[i], (long)VarByte(varnNotePriority));
							batch.Add("SET @nNewID = SCOPE_IDENTITY() ");
							// (j.dinatale 2010-12-22) - PLID 41885 - added LabResultID and LabID to the query
							// (j.gruber 2014-12-12 15:00) - PLID 64391 - Rescheduling Queue - attached appointments to notes
							// (r.gonet 2015-01-20 15:40) - PLID 63490 - Save the recall ID too.
							batch.Add("INSERT INTO NoteInfoT (NoteID, BillID, LineItemID, MailID, LabResultID, LabID, RecallID, AppointmentID) "
								"VALUES (@nNewID, {VT_I4}, {VT_I4}, {VT_I4}, {VT_I4}, {VT_I4}, {VT_I4}, {VT_I4})",
								varBillID, varLineItemID, varMailID, varLabResultID, varLabID, m_vtRecallID, varApptID);
							// (c.haag 2010-08-26 16:13) - PLID 39473 - Safety check
							if (-1 != m_nMailID) {
								batch.Declare(
									"IF ("
									"	(SELECT PersonID FROM Notes WHERE ID = @nNewID) "
									"	<> "
									"	(SELECT PersonID FROM MailSent WHERE MailID = (SELECT MailID FROM NoteInfoT WHERE NoteID = @nNewID)) "
									") BEGIN "
									"	RAISERROR ('The history record was moved to another patient! This transaction has been rolled back.', 16, 1) ROLLBACK TRAN RETURN "
									"END "
									);
							}
						}
						// (d.singleton 2012-03-27 12:42) - PLID 49257
						else {
							_RecordsetPtr rsID = batch.CreateRecordset(GetRemoteData());

							if(i == 0) {
								m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varDate = dtSplit;
								m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varCategory = (long)varCategory;
								m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varLineItemID = varLineItemID;								
								m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varNoteID = VarLong(rsID->Fields->GetItem((long)0)->Value);
								m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varPersonID = m_id;
								m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varPriority = VarByte(varnNotePriority);
								m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varUserID = nUserID;
								m_arUnsavedChargeNotes.GetAt(nChargeNoteIndex).varNote = arNotes[i];
							}
							else if(i > 0) {
								UnsavedChargeNote note;
								note.varDate = dtSplit;
								note.varCategory = (long)varCategory;
								note.varLineItemID = varLineItemID;
								note.varNoteID = VarLong(rsID->Fields->GetItem((long)0)->Value);
								note.varPersonID = m_id;
								note.varPriority = VarByte(varnNotePriority);
								note.varUserID = nUserID;
								note.varNote = arNotes[i];
								m_arUnsavedChargeNotes.Add(note);
							}
						}
						batch.Declare(					
							"SET NOCOUNT OFF "
							"SELECT @nNewID, @dtCurrent AS CurrentDate  ");

						_RecordsetPtr rsID = batch.CreateRecordset(GetRemoteData());

						nNewID = VarLong(rsID->Fields->GetItem((long)0)->Value);
						_variant_t vtCurrentDate = rsID->Fields->GetItem("CurrentDate")->Value;


						// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Renamed this variable to indicate that it's 
						// the "new" row so as to distinguish from the row for which we've received this event.
						NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pNxDlNotes->GetNewRow();
						//copy the existing columns except note + id
						pNewRow->PutValue(COLUMN_USER_ID, varstrNoteOwnerID);
						// (a.walling 2010-07-13 17:59) - PLID 39182
						pNewRow->PutValue(COLUMN_NOTE_INPUT_DATE, vtCurrentDate);
						pNewRow->PutValue(COLUMN_DATE, COleVariant(dtSplit));
						pNewRow->PutValue(COLUMN_CATEGORY, varnNoteCategory);

						// (r.galicki 2008-06-26 15:43) - PLID 18881 - added code for new priority and color columns
						pNewRow->PutValue(COLUMN_COLOR, varnNoteColor);
						//TES 2/16/2009 - PLID 32915 - We also need to set the actual color, the datalist won't do
						// it on its own until the next requery.
						if(varnNoteColor.vt == VT_I4) {
							pNewRow->PutForeColor(VarLong(varnNoteColor));
						}
						pNewRow->PutValue(COLUMN_PRIORITY, varnNotePriority);

						pNewRow->PutValue(COLUMN_ID, (long)nNewID);
						pNewRow->PutValue(COLUMN_NOTE, _bstr_t(arNotes[i]));
						// (r.gonet 07/30/2014) - PLID 62525
						pNewRow->PutValue(COLUMN_BILLSTATUSNOTE_BILLID, g_cvarNull);

						m_pNxDlNotes->AddRowSorted(pNewRow, NULL);
					}

					pRow->PutValue(COLUMN_DATE, COleVariant(dtOldest));

					//now update our current note with whatever is left
					//a.walling plid/20206 5/17/06 this is first done before the loop, then saved in EditingFinished.
					//pvarNewValue->bstrVal = strNoteRem.AllocSysString();
				}
			} NxCatchAll("Error in OnEditingFinishingNxdlnotes()");
		break;
		case COLUMN_DATE:
			// (b.cardillo 2005-04-29 12:51) - PLID 11489 - Make sure the user is allowed to change note dates.
			// (a.walling 2010-07-16 09:49) - PLID 39182 - This is all handled in CanEditNote now.
			/*if (!CheckCurrentUserPermissions(bioPatientNotes, sptDynamic0)) {
				*pbCommit = FALSE;
				return;
			}*/
			//If this isn't a date, is an invalid date, or has been converted to 12/30/1899 and therefore is some crazy thing.
			if(pvarNewValue->vt != VT_DATE || VarDateTime(*pvarNewValue).GetStatus() != COleDateTime::valid || VarDateTime(*pvarNewValue).GetYear() <= 1899 || VarDateTime(*pvarNewValue).GetYear() > 2222) {
				MsgBox("The text you entered does not correspond to a valid date. \n Your changes will not be saved");
				*pbCommit = FALSE;
			}
		break;

	}
	
}

void CNotesDlg::OnSelChangedNxdlnotes(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	// (b.cardillo 2005-05-02 12:25) - PLID 11600 - If the selection just changed 
	// then we know the user has finished creating the new note (if he was 
	// creating a note in the first place, which may not necessarily be the case, 
	// but either way we KNOW he's not creating one now).
	m_bIsCreatingNewNote = FALSE;
}

void CNotesDlg::OnFocusLostNxdlnotes()
{
	// (b.cardillo 2005-05-02 12:24) - PLID 11600 - If the focus is being lost 
	// to a window that's not part of the datalist itself (like the in-place 
	// text editor or the embedded combo) then the focus is truly LOST and so 
	// we can't still be editing the row we just added, so we reset the boolean.
	if (!::IsOrHasAncestor(::GetFocus(), GetDlgItem(IDC_NXDLNOTES)->GetSafeHwnd())) {
		m_bIsCreatingNewNote = FALSE;
	}
}

void CNotesDlg::OnAddButtonClicked() 
{
	try {
		BOOL bIsMessaging = (m_SubTab->CurSel == 1);

		if (!bIsMessaging) {
			AddNewNote();
		}
		else {

			//pop up a menu for whether to start a new thread or add to the selected thread
			PopupMenu(natAddNote);
		}
	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2009-11-02 12:42) - PLID 35815 - pops up a menu for whether to do the associated action for a new thread or the selected one
void CNotesDlg::PopupMenu(noteActionType naType) 
{
	try {
		CMenu mnu;
		mnu.CreatePopupMenu();
		
		CWnd *pWnd;
		
		switch (naType) {

			case natAddNote:
				mnu.InsertMenu(1, MF_BYPOSITION, IDM_ADD_NEW_THREAD, "Add New Thread");
				mnu.InsertMenu(2, MF_BYPOSITION, IDM_ADD_TO_SELECTED_THREAD, "Add To Selected Thread");
				pWnd = GetDlgItem(IDC_ADD);
			break;
			case natAddMacro:
				mnu.InsertMenu(1, MF_BYPOSITION, IDM_ADD_NEW_THREAD_MACRO, "Add New Thread");
				mnu.InsertMenu(2, MF_BYPOSITION, IDM_ADD_TO_SELECTED_THREAD_MACRO, "Add To Selected Thread");
				pWnd = GetDlgItem(IDC_ADD_MACRO);
			break;
		}		
		
		
		CRect rc;
			
		if (pWnd) {
			pWnd->GetWindowRect(&rc);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, rc.right, rc.top, this, NULL);
		} else {
			CPoint pt;
			GetCursorPos(&pt);
			mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}	

	}NxCatchAll(__FUNCTION__);
}



void CNotesDlg::OnDeleteNote(long nThreadID /*=-1*/) 
{
	try {

		// (j.gruber 2009-11-16 10:10) - PLID 35815 - added messaging list
		BOOL bIsMessaging = (m_SubTab->CurSel == 1);

		if (!bIsMessaging) {
			// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.

			//DRT 4/28/03 - Use bioPatientNotes, and the new sptDelete
			if (!CheckCurrentUserPermissions(bioPatientNotes, sptDelete))
				return;

			long nNoteID;
			NXDATALIST2Lib::IRowSettingsPtr pCurSel;
			try {

				m_bAllowUpdate = false;
				pCurSel = m_pNxDlNotes->GetCurSel();
				if (pCurSel == NULL) {
					MsgBox("Please Select a note to delete.");
					m_bAllowUpdate = true;
					return;
				}
				try {
					if(pCurSel->GetValue(0).vt==VT_EMPTY) {
						m_bAllowUpdate = true;
						return;
					}
					nNoteID = VarLong(pCurSel->GetValue(0));
				}NxCatchAll("Error in CNotesDlg::OnDelete \n Cannot get ID value");

				// (r.gonet 07/21/2014) - PLID 62525 - Check if this is a bill status note
				long nBillStatusNote_BillID = VarLong(pCurSel->GetValue(COLUMN_BILLSTATUSNOTE_BILLID), -1);
				if (nBillStatusNote_BillID != -1) {
					// They can't edit bill status notes at all except from the bill dialog.
					MessageBox("This note is a bill status note linked to a bill and cannot be edited or deleted outside of the bill.", "Error", MB_ICONERROR | MB_OK);
					return;
				}

				if(MessageBox("Are you sure you wish to delete this note?", "Delete?", MB_YESNO)==IDNO) {
					m_bAllowUpdate = true;
					return;
				}

				//PLID 18881 R.G. 6/4/08 -	hardcoded '4' did not work when new column was added, replaced with COLUMN_NOTE
				CString strOld = CString(pCurSel->GetValue(COLUMN_NOTE).bstrVal);

				// (d.singleton 2012-03-29 17:32) - PLID 49257
				if(m_bUnsavedCharge) {
					for(int i = 0; i < m_arUnsavedChargeNotes.GetCount(); i++) {
						if(m_arUnsavedChargeNotes.GetAt(i).varNoteID == pCurSel->GetValue(COLUMN_ID)) {
							m_arUnsavedChargeNotes.RemoveAt(i);
						}
					}
					m_pNxDlNotes->RemoveRow(pCurSel);
				}
				else {
					try{
						// (r.gonet 07/21/2014) - PLID 62525 - Clear the reference a bill can have to a note for the Bill Status Note
						ExecuteParamSql("UPDATE BillsT SET StatusNoteID = NULL WHERE BillsT.StatusNoteID = {INT}", nNoteID);
						// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
						ExecuteSql("DELETE FROM NoteDataT WHERE ID = %li", nNoteID);
						m_pNxDlNotes->RemoveRow(pCurSel);

					}NxCatchAll("Error in CNotesDlg::OnDelete \n Cannot Delete");

					//auditing
					long nAuditID = -1;
					nAuditID = BeginNewAuditEvent();
					if(nAuditID != -1) {
						// (j.gruber 2009-12-31 13:09) - PLID 36310 - changed from patientID to NoteID
						// (a.walling 2010-09-09 12:27) - PLID 40267 - Audit using the contact event when appropriate
						AuditEvent(m_bIsForPatient ? GetActivePersonID() : -1, GetActivePersonName(), nAuditID, m_bIsForPatient ? aeiPatientNoteDelete : aeiContactNoteDelete, nNoteID, strOld, "", aepMedium, aetDeleted);
					}
				}
			
			}NxCatchAll("Error in CNotesDlg::OnDelete");

			m_bAllowUpdate = true;	
		}
		else {

			// (j.gruber 2009-11-16 10:30) - PLID 35815 - patient messaging

			//check the thread id
			if (nThreadID == -1) {

				//we are deleting just one note
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMessagesList->CurSel;
				if (pRow) {

					long nNewThreadID = VarLong(pRow->GetValue(mlcThreadID));
					NXDATALIST2Lib::IRowSettingsPtr pParentRow = pRow->GetParentRow();

					if (pParentRow) {

						//check to see if this is the last note left in the thread
						NXDATALIST2Lib::IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
						long nCount = 0;
						while (pChildRow) {
							nCount++;
							pChildRow = pChildRow->GetNextRow();
						}

						if (nCount == 1) {
							//there is only one
							
							if (IDNO == MsgBox(MB_YESNO, "This is the last note in the thread, by deleting this note, you will be deleting the entire thread.\nDo you want to continue?")) {
								//they don't want to continue
								return;
							}
							else {
								//they want to delete the whole thread
								OnDeleteNote(nNewThreadID);
								return;
							}
						}

						long nNoteID = VarLong(pRow->GetValue(mlcNoteID));

						//make sure they want to delete it
						if (IDNO == MsgBox(MB_YESNO, "Are you sure you wish to delete this note?")) {
							return;
						}

						CString strOld = VarString(pRow->GetValue(mlcNote), "");

						// (r.gonet 07/21/2014) - PLID 62525 - Clear the reference a bill can have to a note for the Bill Status Note
						ExecuteParamSql("UPDATE BillsT SET StatusNoteID = NULL WHERE BillsT.StatusNoteID = {INT}", nNoteID);

						// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
						ExecuteParamSql("DELETE FROM NoteDataT WHERE ID = {INT}", nNoteID);

						long nAuditID = -1;
						nAuditID = BeginNewAuditEvent();
						if(nAuditID != -1) {
							// (a.walling 2010-09-09 12:27) - PLID 40267 - Audit using the contact event when appropriate
							AuditEvent(m_bIsForPatient ? GetActivePersonID() : -1, GetActivePersonName(), nAuditID, m_bIsForPatient ? aeiPatientNoteDelete : aeiContactNoteDelete, nNoteID, strOld, "", aepMedium, aetDeleted);
						}

						m_pMessagesList->RemoveRow(pRow);
					}
				}
			}
			else {

				//they want to delete a whole thread
				//find the row
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMessagesList->FindByColumn(mlcThreadID, (long)nThreadID, NULL, TRUE);
				if (pRow) {

					if (IDNO == MsgBox(MB_YESNO, "Are you sure you wish to delete this thread?")) {
						return;
					}

					//make sure we have the parent, even though we should
					if (pRow->GetParentRow() != NULL) {
						pRow = pRow->GetParentRow();
					}

					//set up the audit
					NXDATALIST2Lib::IRowSettingsPtr pChild;
					pChild = pRow->GetFirstChildRow();
					long nAuditTransID = -1;
					
					while (pChild) {
						if (nAuditTransID == -1) {
							nAuditTransID = BeginAuditTransaction();
						}


						CString strOld = VarString(pChild->GetValue(mlcNote), "");
						long nNoteID = VarLong(pChild->GetValue(mlcNoteID));
						
						// (a.walling 2010-09-09 12:27) - PLID 40267 - Audit using the contact event when appropriate
						AuditEvent(m_bIsForPatient ? GetActivePersonID() : -1, GetActivePersonName(), nAuditTransID, m_bIsForPatient ? aeiPatientNoteDelete : aeiContactNoteDelete, nNoteID, strOld, "", aepMedium, aetDeleted);
						//m_pMessagesList->RemoveRow(pChild);
						pChild = pChild->GetNextRow();
					}

					//now do the deletion
					try {

						// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
						ExecuteParamSql("BEGIN TRAN; \r\n"
							" DELETE NoteDataT FROM NoteDataT INNER JOIN NoteInfoT ON NoteDataT.ID = NoteInfoT.NoteID WHERE NoteInfoT.PatientMessagingThreadID = {INT}; "
							" \r\nIF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
							" DELETE FROM PatientMessagingThreadT WHERE ID = {INT}; "
							" \r\nIF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
							" COMMIT TRAN; ", nThreadID, nThreadID
						);

						//commit the audit
						if (nAuditTransID != -1) {
							CommitAuditTransaction(nAuditTransID);
						}

						//remove the rows
						m_pMessagesList->RemoveRow(pRow);
					}NxCatchAllCall("Error deleting thread.",
						//rollback the audit
						if (nAuditTransID != -1) {
							RollbackAuditTransaction(nAuditTransID);
						}
					);
					
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CNotesDlg::OnEditCategory() 
{
	CNoteCategories	dlg(this);
	dlg.m_bEditingNoteCat = true;
	dlg.DoModal();
	//TODO: replace this requery with the RefreshComboSource datalist feature, when it exists
	// (d.singleton 2012-03-29 10:52) - PLID 49257
	if(!m_bUnsavedCharge) {
		m_pNxDlNotes->Requery();
	}
	
	//save the current selection, requery, then try to reset
	if(IsDlgButtonChecked(IDC_CATEGORY_FILTER)) {
		m_pCategories->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		long nCat = VarLong(m_pCategories->GetValue(m_pCategories->GetCurSel(), 0));
		m_pCategories->Requery();
		m_pCategories->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		if(m_pCategories->SetSelByColumn(0, (long)nCat) == -1) {
			//OK this failed, so we probably just deleted this item!
			//This code is executed before the RequeryFinished() function, so we can
			//set a flag and let it take care of it
			m_bRequeryToSetSel = true;
		}
	}
	else
		m_pCategories->Requery();


}

void CNotesDlg::SecureControls()
{
	//MSC 5/23/03 - The Add button should check the Create permission
	if (!(GetCurrentUserPermissions(bioPatientNotes) & (SPT____C_______ANDPASS)))
	{
		GetDlgItem(IDC_ADD)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADD_MACRO)->EnableWindow(FALSE);
	}

	//DRT 7/24/03 - Hide the category dropdown if the box is unchecked
	if(!IsDlgButtonChecked(IDC_CATEGORY_FILTER)) {
		((CWnd*)GetDlgItem(IDC_NOTE_CATEGORY_LIST))->EnableWindow(FALSE);
	}
}

void CNotesDlg::OnAddMacroButtonClicked() 
{
	try {
		BOOL bIsMessaging = (m_SubTab->CurSel == 1);

		if (!bIsMessaging) {
			AddMacro();
		}
		else {

			//pop up a menu for whether to start a new thread or add to the selected thread
			PopupMenu(natAddMacro);
		}	
	}NxCatchAll(__FUNCTION__);
}

void CNotesDlg::AddMacro(long nThreadID /*=-1*/) 
{
	try {
		CMacroSelectDlg dlg(this);
		// (a.walling 2011-05-10 10:05) - PLID 41789
		dlg.m_bForBillingNotes = (m_nLineItemID != -1 || m_nBillID != -1);
		if(IDOK == dlg.DoModal()) {
			long nSelectedMacro = dlg.m_nSelectedID;
			if(nSelectedMacro != -1){
				// there was a macro selected, create a note and enter the appropriate macro
				AddNewNote(nSelectedMacro, nThreadID);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CNotesDlg::OnEditMacros() 
{
	CMacroEditDlg dlg(this);
	dlg.DoModal();
}

void CNotesDlg::AddNewNote(long nMacroID /* = -1 */, long nThreadID /*=-1*/)
{
	//DRT 4/28/03 - use bioPatientNotes, and the new permission for create
	if (!CheckCurrentUserPermissions(bioPatientNotes, sptCreate))
		return;

	try	{
		// Scroll to the top of the notes list
		//if (m_NotesList.GetCount() > 0)
		//	m_NotesList.MakeVisible( m_NotesList.GetRecordItem( 0, m_NotesList.GetBoundColumn() ) );


		//
		//DRT 11/14/2007 - Internal only (no PLID) - We want to require the 'Specialty' field to be filled
		//	before allowing notes to be entered.  This is in custom list 4 (ID is 24).
		if(IsNexTechInternal()) {
			//Will have to query for it
			//	BONUS Optimization:  I took the query text from the Custom1Dlg.cpp where list box loading is done.  Right now those aren't
			//	cached, but if one day we cache them, I've made this query text identical, so there won't be 2 procedure cache entries.
			// (j.armen 2011-11-01 16:26) - PLID 11490 - The data for CustomListData has been moved, so we'll need to look for it here
			_RecordsetPtr prs = CreateParamRecordset("SELECT CustomListItemsID AS Val FROM CustomListDataT WHERE PersonID = {INT} AND FieldID = {INT}", m_id, 24);
			if(prs->eof) {
				//There is no value selected, this will now prevent notes until you fill it in.
				AfxMessageBox("This record does not have a specialty selected.  You must fill in the specialty field before you may enter notes.");
				return;
			}
		}

		m_bAllowUpdate = false;

		// (j.gruber 2009-11-02 14:09) - PLID 35815 - added subtab
		BOOL bIsMessaging = (m_SubTab->CurSel == 1);
		
		// Add the note and set the owner ID
		_variant_t varPatID;
		varPatID = (_variant_t)m_id;
		
		long nCatID = -1;

		CString strMacro;
		
		// if we have a macro, get the macro and the category ID
		// (a.walling 2011-05-10 09:16) - PLID 41789 - Gather more macro info
		bool bMacroSuppressUserName = false;
		bool bMacroShowOnStatement = false;
		if(nMacroID != -1){
			GetMacro(nMacroID, strMacro, nCatID, bMacroSuppressUserName, bMacroShowOnStatement);
		}
		else{
			// (j.dinatale 2010-12-23) - PLID 41591 - check the default category if its a lab note, there is a preference for it now
			if (m_nApptID != -1) {
				// (j.gruber 2014-12-16 09:10) - PLID 64392 - Rescheduling Queue - use our note preference
				nCatID = GetRemotePropertyInt("ApptNotesDefaultCategory", -1, 0, "<None>", TRUE);
			}
			else if(m_nCategoryIDOverride != -1){
				nCatID = m_nCategoryIDOverride;
			}else{
				// if we aren't using a macro, check and see if there is a default category
				nCatID = GetRemotePropertyInt("DefaultNoteCatID", NULL, 0, "<None>", TRUE);
			}
		}

		CString strPrefix;
		_variant_t  varNote;

		// (a.walling 2011-05-10 09:16) - PLID 41789 - Suppress the username prefix if necessary
		if (bIsMessaging || bMacroSuppressUserName) {
			//don't insert the username
		}
		else if (m_bIsPatientRemindersSent) //(s.dhole 8/29/2014 12:43 PM ) - PLID 63516 When adding a new note, it should display with “<user name> - (Reminder: <Reminder Sent Date> - <Method>) – “
		{
			strPrefix = FormatString("%s - (%s) - ", CString(GetCurrentUserName()) , m_sPatientRemindersPrefix);
		}
		else if (m_nApptID != -1) // (j.gruber 2014-12-15 10:03) - PLID 64391 - Rescheduling Queue - appointment notes		
		{
			strPrefix = FormatString("%s - RE: Appointment on %s-%s %s for %s\r\n\r\n",
				CString(GetCurrentUserName()),
				FormatDateTimeForInterface(m_dtApptStartTime, NULL, dtoDate),
				FormatDateTimeForInterface(m_dtApptStartTime, NULL, dtoTime),				
				m_strApptTypePurpose.IsEmpty() ? "" : "for " + m_strApptTypePurpose, m_strApptResources);
		}
		else {
			strPrefix = CString(GetCurrentUserName()) + " - ";
		}		
		// (j.dinatale 2010-12-22) - PLID 41915 - if there is prepended text, go ahead and prepend the text to the note
		if(!m_strPrependText.IsEmpty()){
			strPrefix += (m_strPrependText + " - ");
		}
		

		varNote = (strPrefix + strMacro);

		//TES 10/21/2003: Before doing this make sure the notes aren't still requerying (see above comment).
		// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.
		m_pNxDlNotes->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		//TES 1/15/2004: Get the NewNumber at the last possible moment.

		
		if (!bIsMessaging) {
			bool bNeedsCategoryRefilter = false;
			//Are we filtering on a category?
			if(IsDlgButtonChecked(IDC_CATEGORY_FILTER)) {
				long nFilteredCat = VarLong(m_pCategories->GetValue(m_pCategories->CurSel, 0));
				if(nFilteredCat != ID_ALL_CATS) {
					// (j.armen 2011-07-21 12:29) - PLID 13283 - Check to see if we have multiple categories 
					//	selected, and if so are we selected on the category type we are editing
					BOOL bMultipleCat = FALSE;
					if(nFilteredCat == ID_MULTIPLE_CATS) {
						for(int i = 0; i < m_arynCatSelection.GetSize(); i++) {
							if(m_arynCatSelection[i] == nCatID || (m_arynCatSelection[i] == ID_NO_CAT && nCatID == 0)) {
								bMultipleCat = TRUE;
								break;
							}
						}
					}
					if(nFilteredCat != nCatID && !(nFilteredCat == ID_NO_CAT && nCatID <= 0) && !bMultipleCat) {
						// (b.savon 2012-01-25 16:08) - PLID 47777 - Default Note Cateogry to the selected
						BOOL bResetFilter = FALSE;
						//OK, we are filtering on a different category than we want to add.
						//Let's ask the user what to do.
						// (j.armen 2011-07-21 16:40) - PLID 13283 - create message based on current filter selection
						CString strMessage;
						if(nFilteredCat == ID_MULTIPLE_CATS)
						{
							strMessage.Format("You are currently filtering on notes with multiple categories, but the note you are adding will default to a different category. "
								"This note may not be visible in your current filter.  Would you like to expand the filter to include this category?   If you say No, the category filter will be removed.");
							
							// (j.armen 2011-07-21 16:41) - PLID 13283 - show the msg box
							if(IDYES == MsgBox(MB_YESNO, strMessage))
							{
								// (j.armen 2011-07-21 16:41) - PLID 13283 - if were were not previously in multi selection, then clear the list and add the filter for the current view
								if(nFilteredCat != ID_MULTIPLE_CATS)
								{
									m_arynCatSelection.RemoveAll();
									m_arynCatSelection.Add(nFilteredCat);
								}
								// (j.armen 2011-07-21 16:44) - PLID 13283 - if the cat id = 0, then it it is a note without a category and should be set as -1
								if(nCatID == 0)
								{
									m_arynCatSelection.Add(-1);
								}
								else // (j.armen 2011-07-21 16:45) - PLID 13283 - else add the new note category to the list
								{
									m_arynCatSelection.Add(nCatID);
								}
								m_pCategories->FindByColumn(0, (long) ID_MULTIPLE_CATS, 0, VARIANT_TRUE);
								bNeedsCategoryRefilter = true;
							} else {
								bResetFilter = TRUE;
							}
						}
						else
						{
							CString strCategory = VarString(m_pCategories->GetValue(m_pCategories->CurSel, 1));
							strMessage.Format("You are currently filtering on notes with a category of %s, but the note you are adding will default to a different category.\n"
							"Would you like to change this note to category %s?  If you say No, the category filter will be removed.", strCategory, strCategory);
							// (b.savon 2012-01-25 13:08) - PLID 47777 - Default Note Cateogry to the selected
							if(IDYES == MsgBox(MB_YESNO, strMessage))
							{
								nCatID = nFilteredCat;
								if(nCatID == ID_NO_CAT) nCatID = NULL;
							}else {
								bResetFilter = TRUE;
							}
						}

						//We've displayed the message and they have made their decision
						// bResetFilter will be SET when the user chooses NO in the message prompt
						if( bResetFilter ){
							CheckDlgButton(IDC_CATEGORY_FILTER, BST_UNCHECKED);
							//TES 10/21/2003: Note that I am calling this BEFORE adding the note to data.  This is important, because
							//this function will requery the datalist.
							m_bAllowUpdate = true;
							OnCategoryFilter();
							m_bAllowUpdate = false;
							// (b.savon 2012-01-26 09:23) - PLID 47800 - Wait until we are done requerying
							// or we will have a race condition below.  The requery takes some time, whereas
							// the code below executes very quickly.  Sometimes too quickly in that it executes
							// before the datalist is done requerying.  This causes a 'phantom' double row
							// to be added to the datalist that is removed the next time the view is refreshed.
							// So, let's wait until were done, and then add the row to the list to avoid this 
							// race condition.
							m_pNxDlNotes->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
						}
					}
				}
			}

			// (r.galicki 2008-06-26 16:12) - PLID 18881 - Save 'Priority' information
			long nDefaultPriority = GetRemotePropertyInt("MyDefaultNotesPriority", PRIORITY_LOW, 0, GetCurrentUserName(), TRUE);

			// Save the change
			//DRT 10/14/03 - PLID 9408 - A note category of "nothing" MUST be NULL, not -1!  It has always been this way, and 
			//		must continue to be this way.
			// (a.walling 2010-11-01 11:07) - PLID 40965 - Parameterized
			_variant_t varCatID = nCatID > 0 ? _variant_t(nCatID, VT_I4) : g_cvarNull;
			_variant_t varBillID = m_nBillID != -1 ? _variant_t(m_nBillID, VT_I4) : g_cvarNull;
			_variant_t varLineItemID = m_nLineItemID != -1 ? _variant_t(m_nLineItemID, VT_I4) : g_cvarNull;
			// (c.haag 2010-07-01 13:45) - PLID 39473 - Added strMailID
			// (c.haag 2010-07-01 13:45) - PLID 39473 - Added m_nMailID
			_variant_t varMailID = m_nMailID != -1 ? _variant_t(m_nMailID, VT_I4) : g_cvarNull;

			// (j.dinatale 2010-12-22) - PLID 41885 - have new fields, added m_nLabResultID and m_nLabID
			_variant_t varLabResultID = m_nLabResultID != -1 ? _variant_t(m_nLabResultID, VT_I4) : g_cvarNull;
			_variant_t varLabID = m_nLabID != -1 ? _variant_t(m_nLabID, VT_I4) : g_cvarNull;

			// (j.gruber 2014-12-12 15:00) - PLID 64391 - Rescheduling Queue - attached appointments to notes
			_variant_t varApptID = m_nApptID != -1 ? _variant_t(m_nApptID, VT_I4) : g_cvarNull;

			//(s.dhole 8/29/2014 12:04 PM ) - PLID 63516

			_variant_t varPatientRemindersSentID = m_nPatientRemindersSentID != -1 ? _variant_t(m_nPatientRemindersSentID, VT_I4) : g_cvarNull;

			// (a.walling 2011-05-10 09:16) - PLID 41789 - Whether to show on statement
			_variant_t varShowOnStatement = g_cvarFalse;
			if (bMacroShowOnStatement && (m_nLineItemID != -1 || m_nBillID != -1)) {
				varShowOnStatement = g_cvarTrue;
			}

			// (j.jones 2011-09-19 14:53) - PLID 42135 - determine whether to show on a claim
			_variant_t varSendOnClaim = g_cvarNull;
			if(m_bntBillingNoteType == bntCharge && m_nLineItemID != -1) {
				varSendOnClaim = g_cvarFalse;
			}
			
			// (z.manning, 03/05/2007) - PLID 25065 - Pull the note's time from the server.			
			// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
			// (a.walling 2010-11-01 11:07) - PLID 40965 - Parameterized
			CParamSqlBatch batch;
			batch.Declare(
				"SET NOCOUNT ON "
				"DECLARE @nNewID int "
				"DECLARE @dtCurrent datetime ");
			batch.Add("SET @dtCurrent = GetDate() ");
			// (d.singleton 2012-04-02 13:58) - PLID 49257 do not insert data if the charge isnt saved yet,  store in array
			if(!m_bUnsavedCharge) {
				// (j.armen 2014-01-31 09:31) - PLID 60568 - Idenitate NoteDataT
				batch.Add("INSERT INTO Notes (PersonID, Date, UserID, Category, Note, Priority) "
					"VALUES ({INT}, @dtCurrent, {INT}, {VT_I4}, {STRING}, {INT})",
					m_id, GetCurrentUserID(), varCatID, VarString(varNote), nDefaultPriority);
				batch.Add("SET @nNewID = SCOPE_IDENTITY() ");
				// (j.dinatale 2010-12-22) - PLID 41885 - added LabResultID and LabID to the query

				// (a.walling 2011-05-18 09:03) - PLID 41789 - Ensure only one item is set to show on statement as appropriate
				if (varShowOnStatement) {
					if (m_nBillID != -1) {
						batch.Add("UPDATE NoteInfoT SET ShowOnStatement = 0 WHERE BillID = {INT}; \r\n ", m_nBillID);
					}
					else if (m_nLineItemID != -1) {
						batch.Add("UPDATE NoteInfoT SET ShowOnStatement = 0 WHERE LineItemID = {INT}; \r\n ", m_nLineItemID);
					}
				}
				// (a.walling 2011-05-10 09:16) - PLID 41789 - Whether to show on statement
				// (j.armen 2012-03-19 09:13) - PLID 48780 - Added m_vtRecallID
				//(s.dhole 8/29/2014 12:04 PM ) - PLID 63516 added varPatientRemindersSentID
				// (j.gruber 2014-12-12 15:00) - PLID 64391 - Rescheduling Queue - attached appointments to notes				
				batch.Add("INSERT INTO NoteInfoT (NoteID, BillID, LineItemID, MailID, LabResultID, LabID, ShowOnStatement, RecallID,PatientRemindersSentID, AppointmentID) "
					"VALUES (@nNewID, {VT_I4}, {VT_I4}, {VT_I4}, {VT_I4}, {VT_I4}, {VT_BOOL}, {VT_I4}, {VT_I4}, {VT_I4})",
					varBillID, varLineItemID, varMailID, varLabResultID, varLabID, varShowOnStatement, m_vtRecallID, varPatientRemindersSentID, varApptID);
				// (c.haag 2010-08-26 16:13) - PLID 39473 - Safety check
				if (-1 != m_nMailID) {
					batch.Declare(
						"IF ("
						"	(SELECT PersonID FROM Notes WHERE ID = @nNewID) "
						"	<> "
						"	(SELECT PersonID FROM MailSent WHERE MailID = (SELECT MailID FROM NoteInfoT WHERE NoteID = @nNewID)) "
						") BEGIN "
						"	RAISERROR ('The history record was moved to another patient! This transaction has been rolled back.', 16, 1) ROLLBACK TRAN RETURN "
						"END "
						);
				}
			}
			//(c.copits 2011-07-19) PLID 37177 - Audit creating a note
			batch.Declare(
				"SET NOCOUNT OFF "
				"SELECT @dtCurrent AS CurrentDate, @nNewID as NoteID ");

			_RecordsetPtr rsID = batch.CreateRecordset(GetRemoteData());

			//Our procedure will either return exactly one row, or throw an exception.
			_variant_t varNewID = rsID->Fields->GetItem("NoteID")->Value;
			_variant_t vtCurrentDate = rsID->Fields->GetItem("CurrentDate")->Value;

			// (d.singleton 2012-04-02 14:05) - PLID 49257 do not audit adding note if we havent actually saved them yet
			if(!m_bUnsavedCharge) { 
				//(c.copits 2011-07-19) PLID 37177 - Audit creating a note
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if (nAuditID != -1) {
					CString strOld, strNew;
					long nNoteID = VarLong(rsID->Fields->GetItem("NoteID")->Value);
					strOld = "";
					strNew = VarString(varNote);
					AuditEvent(GetActivePersonID(), GetActivePersonName(), nAuditID, aeiPatientNote, nNoteID, strOld, strNew, aepMedium, aetCreated);
				}
			}

			// (a.walling 2011-05-18 15:34) - PLID 41789 - Update the UI as well, huzzah
			if (varShowOnStatement) {
				for (NXDATALIST2Lib::IRowSettingsPtr pRow = m_pNxDlNotes->GetFirstRow(); pRow; pRow = pRow->GetNextRow()) {					
					if (VarBool(pRow->GetValue(COLUMN_SHOW_ON_STATEMENT), FALSE)) {
						pRow->PutValue(COLUMN_SHOW_ON_STATEMENT, g_cvarFalse);
					}
				}
			}

			// (d.singleton 2012-03-29 15:43) - PLID 49257 support creating notes for unsaved charges, add note to array
			// (j.armen 2014-02-07 11:07) - PLID 60568 - When a note is unsaved, we give it an ID of 1 as a temp placeholder
			_variant_t varID = (VarLong(varNewID, 1) + m_arUnsavedChargeNotes.GetCount());
			if(m_bUnsavedCharge) {
				UnsavedChargeNote note;
				note.varDate = vtCurrentDate;
				note.varCategory = varCatID;
				note.varLineItemID = varLineItemID;	
				note.varNoteID = varID;
				note.varPersonID = m_id;
				note.varPriority = nDefaultPriority;
				note.varUserID = GetCurrentUserID();
				note.varNote = varNote;
				note.varStatement = varShowOnStatement;
				note.varClaim = varSendOnClaim;
				m_arUnsavedChargeNotes.Add(note);
			}
			// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pNxDlNotes->GetNewRow();
			pRow->PutValue(COLUMN_ID, varID);
			pRow->PutValue(COLUMN_USER_ID, (_variant_t)GetCurrentUserID());
			// (a.walling 2010-07-13 17:59) - PLID 39182
			pRow->PutValue(COLUMN_NOTE_INPUT_DATE, vtCurrentDate);
			//PLID 18881 r.galicki 6/12/08 - default color setting
			COLORREF rowColor;
			// (r.galicki 2008-06-27 12:26) - PLID 18881 - Color new row, if preference set
			if(1 == GetRemotePropertyInt("ColorizeNotes", 1, 0, GetCurrentUserName(), TRUE)) {
				CString strPriority;
				COLORREF defaultColor;
				switch (nDefaultPriority) {
					case 1:
						strPriority = "NotesPriorityHigh";
						defaultColor = DEFAULT_COLOR_HIGH;
						break;
					case 2:
						strPriority = "NotesPriorityMedium";
						defaultColor = DEFAULT_COLOR_MEDIUM;
						break;
					case 3: default:
						strPriority = "NotesPriorityLow";
						defaultColor = DEFAULT_COLOR_LOW;
						break;
				}
				rowColor = (COLORREF)GetRemotePropertyInt(strPriority, defaultColor, 0, GetCurrentUserName(), TRUE);
			}
			else {
				rowColor = 0;
			}
			
			pRow->PutValue(COLUMN_COLOR, (long)rowColor);
			pRow->ForeColor = rowColor;
			pRow->PutValue(COLUMN_DATE, vtCurrentDate);
			// if we have a category, set it
			if(nCatID > 0) pRow->PutValue(COLUMN_CATEGORY, nCatID);
			pRow->PutValue(COLUMN_PRIORITY, (BYTE)nDefaultPriority); //PLID 18881 r.galicki 6/12/08 - default priority setting
			// (a.walling 2011-05-10 09:16) - PLID 41789 - Whether to show on statement
			pRow->PutValue(COLUMN_SHOW_ON_STATEMENT, varShowOnStatement); // (j.gruber 2010-06-14 14:16) - PLID 39153 - added show on statement
			// (j.jones 2011-09-19 14:25) - PLID 42135 - added Claim checkbox, NULL unless a charge
			pRow->PutValue(COLUMN_SEND_ON_CLAIM, varSendOnClaim);
			pRow->PutValue(COLUMN_NOTE, varNote);
			// (r.gonet 07/30/2014) - PLID 62525 - Initialize the billstatusnote billid column.
			pRow->PutValue(COLUMN_BILLSTATUSNOTE_BILLID, g_cvarNull);
			pRow = m_pNxDlNotes->AddRowSorted(pRow, NULL);

			// (d.singleton 2012-03-30 15:59) - PLID 49257 since we are manually adding the data the rows never requery and never fill the category
			//  and priority drop down menus,  so we must do that manually as well.
			if(m_bUnsavedCharge) {
				NXDATALIST2Lib::IFormatSettingsPtr pfsCategory(__uuidof(NXDATALIST2Lib::FormatSettings));
				NXDATALIST2Lib::IFormatSettingsPtr pfsPriority(__uuidof(NXDATALIST2Lib::FormatSettings));
				//Category First
				pfsCategory->PutDataType(VT_I4);
				pfsCategory->PutFieldType(NXDATALIST2Lib::cftComboSimple);
				pfsCategory->PutEditable(VARIANT_TRUE);
				pfsCategory->PutConnection(_variant_t((LPDISPATCH)GetRemoteData())); //we're going to let this combo use Practice's connection
				pfsCategory->PutComboSource(_bstr_t("SELECT -25 AS ID, '' AS Description UNION ALL SELECT ID, Description FROM NoteCatsF ORDER BY Description ASC"));
				pfsCategory->EmbeddedComboDropDownMaxHeight = 200;
				pfsCategory->EmbeddedComboDropDownWidth = 150;
				//now priority
				pfsPriority->PutDataType(VT_I2);
				pfsPriority->PutFieldType(NXDATALIST2Lib::cftComboSimple);
				pfsPriority->PutEditable(VARIANT_TRUE);
				pfsPriority->PutConnection(_variant_t((LPDISPATCH)GetRemoteData())); //we're going to let this combo use Practice's connection
				pfsPriority->PutComboSource(_bstr_t("SELECT 1, 'High' UNION SELECT 2,'Medium' UNION SELECT 3,'Low'"));
				pfsPriority->EmbeddedComboDropDownMaxHeight = 200;
				pfsPriority->EmbeddedComboDropDownWidth = 75;

				//set the format overrides
				pRow->PutRefCellFormatOverride(COLUMN_CATEGORY, pfsCategory);
				pRow->PutRefCellFormatOverride(COLUMN_PRIORITY, pfsPriority);
			}

			if (pRow != NULL) {
				m_pNxDlNotes->PutCurSel(pRow);
				m_bIsCreatingNewNote = TRUE;
				m_pNxDlNotes->StartEditing(pRow, COLUMN_NOTE);

				// (j.armen 2011-10-10 16:36) - PLID 13283 - Refilting causes a requery, so if we need to do so, we can finally do it here.
				if(bNeedsCategoryRefilter) {
					FilterNotesByRow(m_pCategories->GetCurSel());
				}
			}
		}
		else {

			CString strInsertProcSql;

			//we aren't colorizing these
			
			if (nThreadID == -1) {

				//new thread

				// (j.gruber 2009-12-01 08:43) - PLID 35814 - added subject to the insert	
				// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
				// (a.walling 2010-11-01 11:07) - PLID 40965 - Parameterized
				// (j.armen 2014-01-31 09:31) - PLID 60568 - Idenitate NoteDataT
				_RecordsetPtr rsID = CreateParamRecordset(
				"SET NOCOUNT ON "
				"BEGIN TRAN "
				"DECLARE @nNewNoteID int "
				"DECLARE @nNewThreadID int "
				"DECLARE @dtCurrent datetime "
				"SET @dtCurrent = GetDate() "
				"INSERT INTO PatientMessagingThreadT (CreatedDate, Status, Subject) VALUES (@dtCurrent, 0, '') "
				"SET @nNewThreadID = convert(int, @@identity) "
				"INSERT INTO Notes (PersonID, Date, UserID, Note) "
				" VALUES ({INT}, @dtCurrent, {INT}, {STRING}) "
				"SET @nNewNoteID = SCOPE_IDENTITY() "
				"INSERT INTO NoteInfoT (NoteID, PatientMessagingThreadID, IsPatientCreated) "
				" VALUES (@nNewNoteID, @nNewThreadID, 0) "
				"COMMIT TRAN "
				"SET NOCOUNT OFF "
				"SELECT @nNewNoteID as NoteID, @nNewThreadID as ThreadID, @dtCurrent AS CurrentDate ",
				m_id, GetCurrentUserID(), VarString(varNote));

				//Our procedure will either return exactly one row, or throw an exception.
				_variant_t varNewNoteID = rsID->Fields->Item["NoteID"]->Value;
				_variant_t varCurrentDate = rsID->Fields->Item["CurrentDate"]->Value;
				_variant_t varNewThreadID = rsID->Fields->Item["ThreadID"]->Value;

				//we created a new thread, so we need 2 new rows
				NXDATALIST2Lib::IRowSettingsPtr pParentRow = m_pMessagesList->GetNewRow();
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMessagesList->GetNewRow();

				pParentRow->PutValue(mlcParentID, g_cvarNull);
				pParentRow->PutValue(mlcChildID, varNewThreadID);
				pParentRow->PutValue(mlcThreadID, varNewThreadID);
				pParentRow->PutValue(mlcNoteID, g_cvarNull);
				pParentRow->PutValue(mlcSubject, _variant_t(""));
				pParentRow->PutValue(mlcStatus, (long)0);
				pParentRow->PutValue(mlcDateCol, varCurrentDate);
				pParentRow->PutValue(mlcNote, g_cvarNull);				
				
				pRow->PutValue(mlcParentID, varNewThreadID);
				pRow->PutValue(mlcChildID, g_cvarNull);
				pRow->PutValue(mlcThreadID, varNewThreadID);
				pRow->PutValue(mlcNoteID, varNewNoteID);
				pRow->PutValue(mlcSubject, g_cvarNull);
				pRow->PutValue(mlcStatus, g_cvarNull);
				pRow->PutValue(mlcDateCol, varCurrentDate);
				pRow->PutValue(mlcNote, varNote);

				//add the row
				m_pMessagesList->AddRowSorted(pParentRow, NULL);
				m_pMessagesList->AddRowSorted(pRow, pParentRow);				

				m_pMessagesList->CurSel = pRow;
				m_pMessagesList->StartEditing(pRow, mlcNote);

				//(c.copits 2011-11-08) PLID 37177 - Audit creating a note
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if (nAuditID != -1) {
					CString strOld, strNew;
					long nNoteID = VarLong(rsID->Fields->GetItem("NoteID")->Value);
					strOld = "";
					strNew = VarString(varNote);
					AuditEvent(GetActivePersonID(), GetActivePersonName(), nAuditID, aeiPatientNote, nNoteID, strOld, strNew, aepMedium, aetCreated);
				}

			}
			else {

				//add to existing thread

				// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
				// (a.walling 2010-11-01 11:07) - PLID 40965 - Parameterized
				// (j.armen 2014-01-31 09:31) - PLID 60568 - Idenitate NoteDataT
				_RecordsetPtr rsID = CreateParamRecordset(
				"SET NOCOUNT ON "
				"BEGIN TRAN "
				"DECLARE @nNewNoteID int "
				"DECLARE @nThreadID int "
				"DECLARE @dtCurrent datetime "
				"SET @dtCurrent = GetDate() "
				//(e.lally 2011-01-14) PLID 40965 - Fixed Adam's spacing issue because of critical demo today
				"SET @nThreadID = {INT} "
				"INSERT INTO Notes (PersonID, Date, UserID, Note) "
				" VALUES ({INT}, @dtCurrent, {INT}, {STRING}) "
				"SET @nNewNoteID = SCOPE_IDENTITY() "
				"INSERT INTO NoteInfoT (NoteID, PatientMessagingThreadID, IsPatientCreated) "
				//(e.lally 2010-09-23) PLID 40657 - This is not patient created!
				" VALUES (@nNewNoteID, @nThreadID, 0) "
				"COMMIT TRAN "
				"SET NOCOUNT OFF "
				"SELECT @nNewNoteID as NoteID, @nThreadID as ThreadID, @dtCurrent AS CurrentDate ",
				nThreadID, m_id, GetCurrentUserID(), VarString(varNote));

				//Our procedure will either return exactly one row, or throw an exception.
				_variant_t varNewNoteID = rsID->Fields->Item["NoteID"]->Value;
				_variant_t varCurrentDate = rsID->Fields->Item["CurrentDate"]->Value;				
				
				NXDATALIST2Lib::IRowSettingsPtr pParentRow;

				//find the parent row
				pParentRow = m_pMessagesList->FindByColumn(mlcChildID, (long)nThreadID, NULL, FALSE);
				if (pParentRow) {
					//make sure this is the parent row
					if (pParentRow->GetParentRow() != NULL) {
						pParentRow = pParentRow->GetParentRow();
					}
						
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMessagesList->GetNewRow();
					pRow->PutValue(mlcParentID, (long)nThreadID);					
					pRow->PutValue(mlcChildID, g_cvarNull);
					pRow->PutValue(mlcThreadID, (long)nThreadID);					
					pRow->PutValue(mlcNoteID, varNewNoteID);
					pRow->PutValue(mlcStatus, g_cvarNull);
					pRow->PutValue(mlcSubject, g_cvarNull);
					pRow->PutValue(mlcDateCol, varCurrentDate);
					pRow->PutValue(mlcNote, varNote);

					//add the row					
					m_pMessagesList->AddRowSorted(pRow, pParentRow);	
					
					m_pMessagesList->CurSel = pRow;
					m_pMessagesList->StartEditing(pRow, mlcNote);

					//(c.copits 2011-11-08) PLID 37177 - Audit creating a note
					long nAuditID = -1;
					nAuditID = BeginNewAuditEvent();
					if (nAuditID != -1) {
						CString strOld, strNew;
						long nNoteID = VarLong(rsID->Fields->GetItem("NoteID")->Value);
						strOld = "";
						strNew = VarString(varNote);
						AuditEvent(GetActivePersonID(), GetActivePersonName(), nAuditID, aeiPatientNote, nNoteID, strOld, strNew, aepMedium, aetCreated);
					}

				}
			}
		}

	}NxCatchAll("Error in CNotesDlg::OnClickBtnAdd");
	m_bAllowUpdate = true;
}

// (a.walling 2011-05-10 09:16) - PLID 41789 - Gather more macro info
void CNotesDlg::GetMacro(const IN long nMacroID, CString &strMacro, long& nCatID, bool& bMacroSuppressUserName, bool& bMacroShowOnStatement)
{
	if(nMacroID == -1){
		return;
	}
	try{
		//get the macro
		// (a.walling 2011-05-10 09:16) - PLID 41789 - Parameterized
		_RecordsetPtr rsMacro = CreateParamRecordset("SELECT * FROM NoteMacroT WHERE ID = {INT}", nMacroID);
		if(!rsMacro->eof) {
			strMacro = AdoFldString(rsMacro, "MacroNotes", "");
			bMacroSuppressUserName = FALSE != AdoFldBool(rsMacro, "SuppressUserName", FALSE);
			bMacroShowOnStatement = FALSE != AdoFldBool(rsMacro, "ShowOnStatement", FALSE);
			nCatID = AdoFldLong(rsMacro, "CategoryID", -1);
		}
	}NxCatchAll("Error in CNotesDlg::GetMacro");
}

// (a.walling 2011-05-10 09:16) - PLID 41789 - Use just one function and recordset to get the macro info

void CNotesDlg::OnSearchNotes() 
{
	if(m_nBillID != -1 || m_nLineItemID != -1) {
		//we're filtered right now, but the search will not be, so warn them
		AfxMessageBox("You are currently filtering the notes on financial information. However, the search will look through all the notes for this patient.");
	}
	// (c.haag 2010-07-01 13:45) - PLID 39473
	if(m_nMailID != -1) {
		//we're filtered right now, but the search will not be, so warn them
		AfxMessageBox("You are currently filtering the notes on patient history information. However, the search will look through all the notes for this patient.");
	}

	// (r.gonet 2015-01-20 15:40) - PLID 63490 - Warn if the user is viewing recall notes.
	if (m_vtRecallID != g_cvarNull) {
		AfxMessageBox("You are currently viewing patient recall notes. However, the search will look through all the notes for the patient associated with this recall.");
	}

	// (j.dinatale 2011-01-03) - PLID 41885
	if(m_nLabResultID != -1 || m_nLabID != -1) {
		//we're filtered right now, but the search will not be, so warn them
		AfxMessageBox("You are currently viewing lab notes. However, the search will look through all the notes for the patient associated with this lab.");
	}

	// (j.gruber 2014-12-15 09:50) - PLID 64391 - Rescheduling Queue - notes for appts
	if (m_nApptID != -1) {
		//we're filtered right now, but the search will not be, so warn them
		AfxMessageBox("You are currently filtering the notes on appointment information. However, the search will look through all the notes for this patient.");
	}

	CSearchNotesDlg dlg(this);
	dlg.nPersonID = m_id;
	dlg.DoModal();
}

void CNotesDlg::OnSelChangingNoteCategoryList(long FAR* nNewSel) 
{
	//reset to the first row if they try to select nothing
	if(*nNewSel == -1) {
		*nNewSel = 0;
	}
}

void CNotesDlg::OnRequeryFinishedNoteCategoryList(short nFlags) 
{
	try {
		//need to add 2 rows - 1 for {All Categories}, 1 for {No Category}
		IRowSettingsPtr pRow = m_pCategories->GetRow(-1);
		pRow->PutValue(0, (long)ID_NO_CAT);
		pRow->PutValue(1, _bstr_t(" { No Category } "));
		m_pCategories->AddRow(pRow);

		// (j.armen 2011-07-21 09:44) - PLID 13283 - Add entry for Multiple Categories
		pRow = m_pCategories->GetRow(-1);
		pRow->PutValue(0, (long)ID_MULTIPLE_CATS);
		pRow->PutValue(1, _bstr_t(" { Multiple Categories } "));
		m_pCategories->AddRow(pRow);

		pRow = m_pCategories->GetRow(-1);
		pRow->PutValue(0, (long)ID_ALL_CATS);
		pRow->PutValue(1, _bstr_t(" { All Categories } "));
		m_pCategories->AddRow(pRow);

		if(m_bRequeryToSetSel) {
			//If this is set, then we need to set the selection - since we don't
			//know what to set, we'll go with the all categories idea
			long nRow = m_pCategories->SetSelByColumn(0, (long)ID_ALL_CATS);
			if(nRow > -1)
				// (j.armen 2011-07-21 11:49) - PLID 13283 - Directly call function for filtering notes to bypass the multi select dlg
				FilterNotesByRow(nRow);
			else
				AfxThrowNxException("Could not set the category list");

			m_bRequeryToSetSel = false;
		}

	} NxCatchAll("Error in CNotesDlg::OnRequeryFinishedNoteCategoryList()");
}

// (j.dinatale 2010-12-23) - PLID 41930 - event handle for when the note list finishes requery
void CNotesDlg::OnRequeryFinishedNoteList(short nFlags) 
{
	try{
		// (j.dinatale 2010-12-23) - PLID 41930 - auto add a new note if the dialog is told to do so.
		if(m_bAutoAddNoteOnShow){

			// (j.dinatale 2011-02-16) - PLID 41930 - for whatever reason, the datalist does not decide
			//		to take focus when you tell it to edit a row
			GetDlgItem(IDC_NXDLNOTES)->SetFocus();

			// add the new note
			AddNewNote();

			// only auto add a note once
			m_bAutoAddNoteOnShow = false;
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2011-07-21 11:51) - PLID 13283 - Handle getting the category selection from the row.
// Default showing the multiselect box to false unless we explicity want to show it (such as OnSelChosenNoteCategorylist)
void CNotesDlg::FilterNotesByRow(long nRow, BOOL bShowMultiSelect /*= FALSE*/)
{
	if(nRow == -1) {
		//shouldnt be able to happen, but just in case....
		ASSERT(FALSE);
		m_pCategories->PutCurSel(0);
		nRow = 0;
	}

	long nCatID = VarLong(m_pCategories->GetValue(nRow, 0));
	
	if(nCatID != ID_MULTIPLE_CATS)
	{
		m_arynCatSelection.RemoveAll();
		if(nCatID != ID_ALL_CATS)
		{
			m_arynCatSelection.Add(nCatID);
		}
	}
	
	//now filter the rows of the notes
	if(FilterNotesByCat(nCatID, bShowMultiSelect))
	{
		// (j.armen 2011-07-21 12:08) - PLID 13283 - if filter was successful, update the last note row with the current row
		m_nLastNoteFilterRow = nRow;
	}

	// (j.armen 2011-08-04 10:54) - PLID 13283 - Update the interface to show either the combo box or the label
	switch(VarLong(m_pCategories->GetValue(m_pCategories->GetCurSel(), 0)))
	{
		case ID_MULTIPLE_CATS:	
			m_nxlMultipleCatSelection.SetText("");
			for(int i = 0; i < m_arynCatSelection.GetSize(); i++)
			{
				long nCatID = m_pCategories->SearchByColumn(0, _bstr_t(m_arynCatSelection.GetAt(i)), 0, VARIANT_FALSE);
				if(m_nxlMultipleCatSelection.GetText().IsEmpty())
				{
					m_nxlMultipleCatSelection.SetText((VarString(m_pCategories->GetValue(nCatID, 1))));
					m_nxlMultipleCatSelection.SetToolTip(VarString(m_pCategories->GetValue(nCatID, 1)));
				}
				else
				{
					m_nxlMultipleCatSelection.SetText(m_nxlMultipleCatSelection.GetText() + ", " + VarString(m_pCategories->GetValue(nCatID, 1)));
					m_nxlMultipleCatSelection.SetToolTip(m_nxlMultipleCatSelection.GetToolTip() + "\r\n" + VarString(m_pCategories->GetValue(nCatID, 1)));
				}
			}
			m_nxlMultipleCatSelection.SetSingleLine();
			m_nxlMultipleCatSelection.SetType(dtsHyperlink);
			ShowDlgItem(IDC_NOTE_CATEGORY_LIST, SW_HIDE);
			m_nxlMultipleCatSelection.ShowWindow(SW_SHOWNA);
			break;
		default:
			m_nxlMultipleCatSelection.ShowWindow(SW_HIDE);
			ShowDlgItem(IDC_NOTE_CATEGORY_LIST, SW_SHOWNA);
			break;
			//IDC_MULTIPLE_NOTE_CAT_FILTER
	}
}

// (j.armen 2011-07-21 11:53) - PLID 13283 - When selection is chosen, filter notes by the result.
// if the filter requests multiselect then we must show the dlg
void CNotesDlg::OnSelChosenNoteCategoryList(long nRow) 
{
	FilterNotesByRow(nRow, TRUE);	
}

//filters the notes list by the category id given
//use ID_NO_CAT for the items with no category selected, 
//and ID_ALL_CATS for all items
// (j.armen 2011-07-21 12:15) - PLID 13283 - ID_MULTIPLE_CATS for multiple categories
// (j.armen 2011-07-21 12:15) - PLID 13283 - returns true if the last selection should be updated
BOOL CNotesDlg::FilterNotesByCat(long nCatID, BOOL bShowMultiSelect)
{
	try {
		if(!IsDlgButtonChecked(IDC_CATEGORY_FILTER))
			//shouldnt be able to get here with this unchecked
			return TRUE;

		CString strWhere;

		// (j.armen 2011-08-04 16:12) - PLID 13283 - First check to see if it's multi select.
		//	Multi select can trigger single select actions if one or no options are chosen.
		if(nCatID == ID_MULTIPLE_CATS) {
			if(bShowMultiSelect) {
				// (j.armen 2011-07-21 11:56) - PLID 13283 - show our multiselect dlg
				// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
				CMultiSelectDlg dlg(this, "NoteCatsF");
				dlg.PreSelect(m_arynCatSelection);
				CString strFrom;
				strFrom.Format("(SELECT ID, Description FROM NoteCatsF UNION SELECT %li, ' { No Category } ') NoteCatsF", ID_NO_CAT);
				if(IDOK == dlg.Open(strFrom, "", "ID", "Description", "Select Multiple Categories"))	{
					dlg.FillArrayWithIDs(m_arynCatSelection);
				}
				else {
					// (j.armen 2011-07-21 12:10) - PLID 13283 - cancel and set our selection back to previous
					if(VarLong(m_pCategories->GetValue(m_nLastNoteFilterRow, 0)) != ID_MULTIPLE_CATS)
					{
						m_pCategories->PutCurSel(m_nLastNoteFilterRow);
						return FALSE;
					}
				}
			}

			// (j.armen 2011-08-04 16:13) - PLID 13283 - Get number of elements that were returned.
			//	if 0 or 1 then either select all categories or the specific category that was chosen.
			//	else, continue processing as multi select and generate where clause
			switch(m_arynCatSelection.GetSize())
			{
				case 0:
					nCatID = ID_ALL_CATS;
					m_pCategories->PutCurSel(m_pCategories->SearchByColumn(0, _bstr_t(ID_ALL_CATS), 0, VARIANT_FALSE));
					break;
				case 1:
					nCatID = m_arynCatSelection.GetAt(0);
					m_pCategories->PutCurSel(m_pCategories->SearchByColumn(0, _bstr_t(nCatID), 0, VARIANT_FALSE));					
					break;
				default:
					// (j.armen 2011-07-21 11:57) - PLID 13283 - Check to see if the No Category was selected.  We need to handle that as IS NULL later on
					BOOL bShowNoCat = FALSE;
					for(int i = 0; i < m_arynCatSelection.GetSize(); i++) {
						if(m_arynCatSelection[i] == ID_NO_CAT) {
							m_arynCatSelection.RemoveAt(i);
							bShowNoCat = TRUE;
							break;
						}
					}

					// (j.armen 2011-07-21 11:57) - PLID 13283 - Generate the where clause for multiple selection
					if(bShowNoCat) {
						strWhere.Format("(Notes.Category IN (%s) OR Notes.Category IS NULL) AND Notes.PersonID = %li", ArrayAsString(m_arynCatSelection, false), m_id);
						m_arynCatSelection.Add(ID_NO_CAT);
					}
					else {
						strWhere.Format("Notes.Category IN (%s) AND Notes.PersonID = %li", ArrayAsString(m_arynCatSelection, false), m_id);
					}
					break;
			}
		}

		// (j.armen 2011-08-04 16:15) - PLID 13283 - Now check to see if we need to run no category, all category, or a specific category
		if(nCatID == ID_NO_CAT) {
			strWhere.Format("Notes.Category IS NULL AND Notes.PersonID = %li", m_id);
		}
		else if(nCatID == ID_ALL_CATS) {
			strWhere.Format("Notes.PersonID = %li", m_id);	//nothing, show all items
		}
		else if(nCatID != ID_MULTIPLE_CATS){
			//valid category
			strWhere.Format("Notes.Category = %li AND Notes.PersonID = %li", nCatID, m_id);
		} //Else we have already handled for multiple categories

		CString str = "";
		if(m_nBillID != -1)
			str.Format(" AND BillID = %li", m_nBillID);
		else if(m_nLineItemID != -1)
			str.Format(" AND LineItemID = %li", m_nLineItemID);
		else if(m_nMailID != -1) // (c.haag 2010-07-01 13:45) - PLID 39473
			str.Format(" AND MailID = %li", m_nMailID);
		else if(m_nLabResultID != -1)	// (j.dinatale 2010-12-22) - PLID 41885 - added m_nLabResultID
			str.Format(" AND LabResultID = %li", m_nLabResultID);
		else if (m_nLabID != -1)	// (j.dinatale 2010-12-22) - PLID 41885 - added m_nLabID
			str.Format(" AND LabID = %li", m_nLabID);
		else if (m_vtRecallID != g_cvarNull) // (r.gonet 2015-01-20 15:40) - PLID 63490 - If the user is viewing recall notes, then filter on that recall.
			str.Format(" AND RecallID = %li", VarLong(m_vtRecallID, -1));
		else if (m_nPatientRemindersSentID != -1)	//(s.dhole 8/29/2014 12:04 PM ) - PLID 63516 m_nPatientRemindersSentID
			str.Format(" AND PatientRemindersSentID = %li", m_nPatientRemindersSentID);
		else if (m_nApptID != -1)
			str.Format(" AND AppointmentID = %li", m_nApptID); // (j.gruber 2014-12-15 09:50) - PLID 64391 - Rescheduling Queue - notes for appts

		strWhere += str;

		// (b.spivey, October 05, 2012) - PLID 30398 - Build a where clause to hide billing notes. 
		if(m_checkHideBills.IsWindowEnabled() == TRUE && !m_checkHideBills.GetCheck()) {
			str =	" AND CASE "
					"	WHEN ((BillID Is Not Null OR LineItemID Is Not Null) "
					"	AND Coalesce(ShowOnStatement,0) = 0 AND Coalesce(SendOnClaim,0) = 0) "
					"	THEN 1 ELSE 0 "
					"END = 0 ";
			strWhere += str;
			str = "";
		}

		// (b.spivey, October 17, 2012) - PLID 30398 - Filter ONLY claims or ONLY statements. 
		//both claim and statement
		if ((m_checkHideClaims.IsWindowEnabled() == TRUE && !m_checkHideClaims.GetCheck()) 
			&& (m_checkHideStatements.IsWindowEnabled() == TRUE && !m_checkHideStatements.GetCheck())) {

			str =	" AND "
					" CASE WHEN ((ShowOnStatement = 1 OR SendOnClaim = 1)) "
					" THEN 1 ELSE 0 "
					" END = 0 ";
			strWhere += str;
			str = "";
			
		}
		//claim notes
		else if(m_checkHideClaims.IsWindowEnabled() == TRUE && !m_checkHideClaims.GetCheck()) {
			// (b.spivey, October 18, 2012) - PLID 30398 - ShowOnStatement can be null
			str =	" AND 	"
					" CASE WHEN ((SendOnClaim = 1) AND (SendOnClaim <> ShowOnStatement OR ShowOnStatement IS NULL)) "
					" THEN 1 ELSE 0 "
					" END = 0 ";
			strWhere += str;
			str = "";
		}
		//statement notes
		else if(m_checkHideStatements.IsWindowEnabled() == TRUE && !m_checkHideStatements.GetCheck()) {
			// (b.spivey, October 18, 2012) - PLID 30398 - SendOnClaim can be null
			str =	" AND "
					" CASE WHEN ((ShowOnStatement = 1) AND (SendOnClaim <> ShowOnStatement OR SendOnClaim IS NULL)) "
					" THEN 1 ELSE 0 "
					" END = 0";
			strWhere += str;
			str = "";
		}


		m_pNxDlNotes->PutWhereClause(_bstr_t(strWhere));

		m_pNxDlNotes->Requery();

		return TRUE;
	} NxCatchAll("Error in CNotesDlg::FilterNotesByCat()");
	return FALSE;
}

void CNotesDlg::OnCategoryFilter() 
{
	try {
		if(IsDlgButtonChecked(IDC_CATEGORY_FILTER)) {
			GetDlgItem(IDC_NOTE_CATEGORY_LIST)->EnableWindow(TRUE);
			long nRow = m_pCategories->SetSelByColumn(0, (long)ID_ALL_CATS);
			if(nRow > -1)
				// (j.armen 2011-07-21 11:59) - PLID 2385 - directly call function to filter the notes based on row
				FilterNotesByRow(nRow);
			else {
				//shouldn't be possible, but if we somehow get no rows, we're gonna be confused
				ASSERT(FALSE);
			}
		}
		else {
			GetDlgItem(IDC_NOTE_CATEGORY_LIST)->EnableWindow(FALSE);
			// (j.armen 2011-08-04 16:18) - PLID 13283 - Reset the filter controls the filters are disabled
			m_nxlMultipleCatSelection.ShowWindow(SW_HIDE);
			GetDlgItem(IDC_CATEGORY_FILTER)->ShowWindow(SW_SHOW);
			m_pCategories->SetSelByColumn(0, (long)ID_ALL_CATS);
			UpdateView();
		}
	} NxCatchAll("Error in CNotesDlg::OnCategoryFilter()");
}

void CNotesDlg::OnNotesShowGrid() 
{
	try {
		// (r.gonet 2015-01-20 15:40) - PLID 63490 - Removed code duplication by turning an if/else into one branch, 
		// which fixed a bug with the ReminderHistory gridline property not saving. Also added a recall gridline property.
		long nNewValue = IsDlgButtonChecked(IDC_NOTES_SHOW_GRID) ? 1 : 0;
		if (m_bIsBillingNote)
			SetRemotePropertyInt("NotesShowGridBill", nNewValue, 0, GetCurrentUserName());
		else if (m_bIsHistoryNote) // (c.haag 2010-07-01 13:45) - PLID 39473
			SetRemotePropertyInt("NotesShowGridHistory", nNewValue, 0, GetCurrentUserName());
		else if (m_bIsLabNotes) // (j.dinatale 2010-12-22) - PLID 41885
			SetRemotePropertyInt("NotesShowGridLab", nNewValue, 0, GetCurrentUserName());
		else if (m_vtRecallID != g_cvarNull)
			SetRemotePropertyInt("NotesShowGridRecall", nNewValue, 0, GetCurrentUserName());
		else if (m_bIsPatientRemindersSent) //(s.dhole 8/29/2014 12:04 PM ) - PLID 63516
			SetRemotePropertyInt("NotesShowGridReminderHistory", nNewValue, 0, GetCurrentUserName());
		else if (m_nApptID != -1) // (j.gruber 2014-12-15 09:50) - PLID 64391 - Rescheduling Queue - notes for appts
			SetRemotePropertyInt("NotesShowGridAppointments", nNewValue, 0, GetCurrentUserName());
		else
			SetRemotePropertyInt("NotesShowGrid", nNewValue, 0, GetCurrentUserName());

		m_pNxDlNotes->GridVisible = IsDlgButtonChecked(IDC_NOTES_SHOW_GRID) ? VARIANT_TRUE : VARIANT_FALSE;
	}NxCatchAll("Error in CNotesDlg::OnNotesShowGrid()");	
}


LRESULT CNotesDlg::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		if (wParam == NetUtils::NoteCatsF) {
			// (a.wilson 2014-08-11 10:06) - PLID 63244 - check and update note categories.
			EnsureUpdatedCategoryFilter((long)lParam);
		}
	} NxCatchAll("Error in CNotesDlg::OnTableChanged");

	return 0;
}

//DRT 11/2/2005 - PLID 18177 - If a patient has not had contact in the past 3 months, we will prompt and ask the user if they wish to 
void CNotesDlg::AttemptPromptForReferral()
{
	if(IsNexTechInternal()) {
		//We warn if 1)  There are no notes in the past 3 months, and 2)  If this is a prospect
		if(ReturnsRecords("SELECT TOP 1 * FROM PatientsT LEFT JOIN Notes ON PatientsT.PersonID = Notes.PersonID "
			"WHERE PatientsT.PersonID = %li AND (PatientsT.CurrentStatus <> 2 OR Date >= DATEADD(mm, -3, GetDate()) "
			"OR (SELECT MAX(Date) FROM MultiReferralsT WHERE MultiReferralsT.PatientID = PatientsT.PersonID) >= DATEADD(mm, -3, GetDate()))", GetActivePersonID())) {

			//There are notes in the past 3 months, no warnings to be given, we don't need to do anything.
			return;
		}

		if(AfxMessageBox("This prospect has had no contact in the past 3 months.  Would you like to enter a new referral source?", MB_YESNO) == IDNO)
			return;

		//pop up the referral dialog and do all that work
		//DRT 11/2/2005 - This code was copied from CGeneral2Dlg::AddReferral()
		CReferralTreeDlg dlg(this);
		long result = dlg.DoModal();	//returns the id selected

		try{
			if(result > 0){	//need to put the new one in MultiReferralsT
				//first thing to check, we don't want the same source multiple times
				_RecordsetPtr rs = CreateRecordset("SELECT ReferralID FROM MultiReferralsT WHERE ReferralID = %li AND PatientID = %li", result, m_id);
				if(rs->eof){
					COleDateTime dt = COleDateTime::GetCurrentTime();
					CString str;
					str = FormatDateTimeForSql(dt, dtoDate);

					BOOL bNewReferral = IsRecordsetEmpty("SELECT ReferralID FROM MultiReferralsT WHERE PatientID = %li",m_id);

					ExecuteSql("INSERT INTO MultiReferralsT (ReferralID, PatientID, Date) (SELECT %li, %li, '%s')", result, m_id, str);

					//this preference lets them leave existing referrals as primary
					//1 - make new referral primary (default), 2 - leave existing as primary
					long nPrimaryReferralPref = GetRemotePropertyInt("PrimaryReferralPref",1,0,"<None>",TRUE);

					//now "select" it as the primary referral source
					if(bNewReferral || nPrimaryReferralPref == 1)
						ExecuteSql("UPDATE PatientsT SET ReferralID = %li WHERE PersonID = %li", result, m_id);
				}
				else {
					MsgBox("You cannot add a referral source more than once per patient.");
				}
			}

			// Only audit if we the user selected a referral to add
			if (result > 0) {
				//auditing
				CString strNew;
				_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ReferralSourceT WHERE PersonID = %li",result);
				if(!rs->eof) {
					strNew = AdoFldString(rs, "Name","");
				}
				rs->Close();
				long nAuditID = -1;
				nAuditID = BeginNewAuditEvent();
				if(nAuditID != -1) {
					AuditEvent(GetActivePersonID(), GetActivePersonName(), nAuditID, aeiPatientReferralAdd, m_id, "", strNew, aepMedium, aetChanged);
					{
						//DRT 11/2/2005 - This is a copy of the CGeneral2Dlg::UpdateChangedDate function
						_RecordsetPtr rs = CreateRecordset("SELECT ChangedDate FROM AuditT "
							"INNER JOIN AuditDetailsT ON AuditT.ID = AuditDetailsT.AuditID WHERE AuditT.ID = %li AND ItemID <= 1000",nAuditID);

						if(!rs->eof) {
							SetDlgItemText(IDC_LAST_MODIFIED, FormatDateTimeForInterface(AdoFldDateTime(rs, "ChangedDate")));
						}
						rs->Close();
					}
				}
			}
		} NxCatchAll("Error adding referral source");

	}
}

// (b.cardillo 2006-12-13 12:40) - PLID 23713 - Spell-checking of all notes.
void CNotesDlg::OnCheckSpellingBtn()
{
	try {
		// (b.cardillo 2006-12-14 16:09) - PLID 23713 - Made it call the nxdl2 spell check method.

		// Check permissions
		if(!(GetCurrentUserPermissions(bioPatientNotes) & (sptWrite|sptWriteWithPass))) {
			// The user doesn't have permissions to write, so if there ARE mistakes they won't be 
			// correctable.  Just let the user know this but still give him or her the option of 
			// spell checking.
			int nResult;

			// (a.walling 2010-07-13 18:11) - PLID 39182 - We will prompt them if they have the same-day permission. We'll just change the wording a bit.
			if (GetCurrentUserPermissions(bioPatientNotes) & (sptDynamic1|sptDynamic1WithPass)) {
				nResult = MessageBox(
					"You do not have permissions to modify notes other than those created today, so if "
					"there are any spelling mistakes on any older notes you will not be allowed to correct "
					"them.  Do you still want to check the spelling of the notes?", 
					NULL, MB_ICONQUESTION|MB_OKCANCEL);

			} else {
				nResult = MessageBox(
					"You do not have permissions to modify notes, so if there are any spelling mistakes "
					"you will not be allowed to correct them.  Do you still want to check the spelling "
					"of the notes?", NULL, MB_ICONQUESTION|MB_OKCANCEL);
			}
			if (nResult != IDOK) {
				return;
			}
		}

		// (r.gonet 07/22/2014) - PLID 62525 - They can't edit bill status notes (aside from non-essentials) except from the bill dialog.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pNxDlNotes->GetFirstRow();
		while (pRow) {
			long nBillStatusNote_BillID = VarLong(pRow->GetValue(COLUMN_BILLSTATUSNOTE_BILLID), -1);
			if (nBillStatusNote_BillID != -1) {
				MessageBox("At least one note is a bill status note linked to a bill and cannot be edited or deleted outside of the bill. If it has any spelling mistakes, these will not be corrected.", "Bill Status Notes", MB_ICONWARNING | MB_OK);
				break;
			}
			pRow = pRow->GetNextRow();
		}

		// (j.gruber 2009-11-16 16:41) - PLID 35815 - patient messaging
		BOOL bIsMessaging = (m_SubTab->CurSel == 1);

		if (!bIsMessaging) {

			// No go ahead with the spell check
			NXDATALIST2Lib::ESpellCheckResult escr = m_pNxDlNotes->SpellCheckAllEditableCells();
			if (escr == NXDATALIST2Lib::scrCompletedNoMistakes || escr == NXDATALIST2Lib::scrCompletedFoundMistakes) {
				// The spelling check completed without being canceled, so report its completion
				MessageBox("The spelling check is complete.", NULL, MB_OK|MB_ICONINFORMATION);
			}
		}
		else {

			NXDATALIST2Lib::ESpellCheckResult escr = m_pMessagesList->SpellCheckAllEditableCells();
			if (escr == NXDATALIST2Lib::scrCompletedNoMistakes || escr == NXDATALIST2Lib::scrCompletedFoundMistakes) {
				// The spelling check completed without being canceled, so report its completion
				MessageBox("The spelling check is complete.", NULL, MB_OK|MB_ICONINFORMATION);
			}
		}
			
	} NxCatchAll("CNotesDlg::OnCheckSpellingBtn");
}

void CNotesDlg::SelectTabTabPatientNotes(short newTab, short oldTab)
{
	try {
		if (newTab != oldTab) {
			UpdateView();
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2009-11-17 09:05) - PLID 35815 - patient messaging
void CNotesDlg::EditingStartingPatientMessagingThreadList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		
		if (pRow) {

			//is this a child row?
			if (pRow->GetParentRow()) {

				//it is, we can edit the date and note
				if (nCol != mlcDateCol && nCol != mlcNote) {
					*pbContinue = false;
				}
			}
			else {
				//it's a parent, we can edit Subject, and status
				if (nCol != mlcSubject && nCol != mlcStatus) {
					*pbContinue = false;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CNotesDlg::EditingFinishedPatientMessagingThreadList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		if (bCommit) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			
			if (pRow) {

				//is this a child row?
				if (pRow->GetParentRow() == NULL) {

					//it's a parent
					long nThreadID = VarLong(pRow->GetValue(mlcThreadID));

					//parents can only edit subject and status
					switch (nCol) {
						case mlcSubject:
							{
								ExecuteParamSql("UPDATE PatientMessagingThreadT SET Subject = {STRING} WHERE ID = {INT}", VarString(varNewValue, ""), nThreadID);

								// (a.walling 2010-09-09 12:24) - PLID 40267
								if (m_bIsForPatient) {
									CString strOld = VarString(varOldValue, "");
									CString strNew = VarString(varNewValue, "");

									long nAuditID = BeginNewAuditEvent();
									if (nAuditID != -1) {
										AuditEvent(GetActivePersonID(), GetActivePersonName(), nAuditID, aeiPatientMessagingSubject, nThreadID, strOld, strNew, aepMedium, aetChanged);
									}
								}
							}
						break;

						case mlcStatus:
							{
								ExecuteParamSql("UPDATE PatientMessagingThreadT SET Status = {INT} WHERE ID = {INT}", VarLong(varNewValue), nThreadID);
								
								long nOldValue = VarLong(varOldValue, -1);
								long nNewValue = VarLong(varNewValue, -1);
								
								long nAuditID = BeginNewAuditEvent();
								if (nAuditID != -1) {
									AuditEvent(GetActivePersonID(), GetActivePersonName(), nAuditID, aeiPatientMessagingStatus, nThreadID, 
										nOldValue == 0 ? "<Open>" : "<Closed>", 
										nNewValue == 0 ? "<Open>" : "<Closed>", aepMedium, aetChanged);
								}
							}
						break;
					}			

				}
				else {

					//should never be null, so don't send a default
					long nNoteID = VarLong(pRow->GetValue(mlcNoteID));

					//can edit date and note
					switch (nCol) {

						case mlcDateCol:
							{
								if (varNewValue.vt == VT_DATE) {
									CString strOld = FormatDateTimeForInterface(VarDateTime(varOldValue), NULL, dtoNaturalDatetime, false);
									CString strNew = FormatDateTimeForInterface(VarDateTime(varNewValue), NULL, dtoNaturalDatetime, false);
						
									if (strOld != strNew) {
										ExecuteParamSql("UPDATE Notes SET Date = {VT_DATE} WHERE ID = {INT} ", varNewValue, nNoteID);
										
										// (a.walling 2010-09-09 12:25) - PLID 40267
										if (m_bIsForPatient) {
											long nAuditID = BeginNewAuditEvent();
											if (nAuditID != -1) {
												AuditEvent(GetActivePersonID(), GetActivePersonName(), nAuditID, aeiPatientNoteDate, nNoteID, strOld, strNew, aepMedium, aetChanged);
											}
										}
									}
								}
								
							}
						break;

						case mlcNote:
							
							if (varOldValue.vt == VT_EMPTY || VarString(varOldValue) != VarString(varNewValue)) {
								long nNoteID =  VarLong(pRow->GetValue(mlcNoteID));

								if (m_bSplit)  {// find the new row
									NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pMessagesList->FindByColumn(mlcNote, _variant_t(varNewValue), 0, false);
									if (pNewRow == NULL) { 
										ThrowNxException("The index of the unsplit item could not be found!"); 
									}
									//Set the date so we can have them be in order in the data
									ExecuteSql("UPDATE Notes SET Note = '%s', Date = '%s' WHERE ID = %li",
										_Q(VarString(varNewValue)), FormatDateTimeForSql(VarDateTime(pNewRow->GetValue(mlcDateCol))), nNoteID);
									m_bSplit = false;									
									
								}
								else {
									ExecuteSql("UPDATE Notes SET Note = '%s' WHERE ID = %li",
										_Q(VarString(varNewValue)), nNoteID);
								}

								CString strNew = VarString(varNewValue, "");
								CString strOld = VarString(varOldValue, "");
								
					
								// (a.walling 2010-09-09 12:25) - PLID 40267
								if(strNew != strOld && m_bIsForPatient) {
									long nAuditID = BeginNewAuditEvent();
									if (nAuditID != -1) {
										AuditEvent(GetActivePersonID(), GetActivePersonName(), nAuditID, aeiPatientNote, nNoteID, strOld, strNew, aepMedium, aetChanged);
									}
								}
							}
						break;
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CNotesDlg::RButtonUpPatientMessagingThreadList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		bool bAllowAddNote = true;
		if(!(GetCurrentUserPermissions(bioPatientNotes) & (sptCreate|sptCreateWithPass)))
		{
			bAllowAddNote = false;
		}
		else
		{
			bAllowAddNote = true;
		}

		bool bIsParent = false;
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			long nParentID = VarLong(pRow->GetValue(mlcParentID), -1);
			if (nParentID == -1) {
				bIsParent = true;
			}
		}

		CMenu menPopup;
		menPopup.m_hMenu = CreatePopupMenu();

		if(lpRow == NULL){
			menPopup.InsertMenu(1, MF_BYPOSITION|(bAllowAddNote ? 0:MF_GRAYED), IDM_ADD_NEW_THREAD, "Add New Thread");			
			menPopup.InsertMenu(2, MF_BYPOSITION|(bAllowAddNote ? 0:MF_GRAYED), IDM_ADD_NEW_THREAD_MACRO, "Add New Thread From Macro");
		}else{
			// Build a menu popup with the ability to delete the current row			
			m_pMessagesList->PutCurSel(NXDATALIST2Lib::IRowSettingsPtr(lpRow));
			
			//DRT 4/28/03 - Gray this out if they don't have permission
			if (!(GetCurrentUserPermissions(bioPatientNotes) & (sptDelete|sptDeleteWithPass))) {
				if (bIsParent) {
					menPopup.InsertMenu(0, MF_BYPOSITION|MF_GRAYED, IDM_DELETE_THREAD, "Delete Thread");
				}
				else {
					menPopup.InsertMenu(0, MF_BYPOSITION|MF_GRAYED, IDM_DELETE_NOTE, "Delete Note");
				}
			}
			else {
				if (bIsParent) {
					menPopup.InsertMenu(0, MF_BYPOSITION, IDM_DELETE_THREAD, "Delete Thread");		
				}
				else {
					menPopup.InsertMenu(0, MF_BYPOSITION, IDM_DELETE_NOTE, "Delete Note");		
				}
			}
					
			menPopup.InsertMenu(1, MF_BYPOSITION|(bAllowAddNote ? 0:MF_GRAYED), IDM_ADD_NEW_THREAD, "Add New Thread");
			menPopup.InsertMenu(2, MF_BYPOSITION|(bAllowAddNote ? 0:MF_GRAYED), IDM_ADD_TO_SELECTED_THREAD, "Add Note To Selected Thread");
			menPopup.InsertMenu(3, MF_BYPOSITION|(bAllowAddNote ? 0:MF_GRAYED), IDM_ADD_NEW_THREAD_MACRO, "Add New Thread From Macro");				
			menPopup.InsertMenu(4, MF_BYPOSITION|(bAllowAddNote ? 0:MF_GRAYED), IDM_ADD_TO_SELECTED_THREAD_MACRO, "Add Note To Selected Thread From Macro");				
			
			
		}
		

		CPoint pt;
		pt.x = x;
		pt.y = y;
		CWnd* dlNotes = GetDlgItem(IDC_PATIENT_MESSAGING_THREAD_LIST);
		if (dlNotes != NULL) {
			dlNotes->ClientToScreen(&pt);
			menPopup.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this, NULL);
		}
		else {
			HandleException(NULL, "An error ocurred while creating menu");
		}

	}NxCatchAll(__FUNCTION__);
}

void CNotesDlg::EditingFinishingPatientMessagingThreadList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{

	try {

		if(*pbCommit == FALSE)
		return;

		if (!CheckCurrentUserPermissions(bioPatientNotes, sptWrite))
		{
			*pbCommit = FALSE;
			return;
		}


		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			
		if (pRow) {

			//is this a child row?
			if (pRow->GetParentRow()) {

				NXDATALIST2Lib::IRowSettingsPtr pParentRow;
				pParentRow = pRow->GetParentRow();

				//it is!
				//check the values that they are adding
				switch (nCol) {
					case mlcDateCol:
						if (!CheckCurrentUserPermissions(bioPatientNotes, sptDynamic0)) {
							*pbCommit = FALSE;
							return;
						}
						//If this isn't a date, is an invalid date, or has been converted to 12/30/1899 and therefore is some crazy thing.
						if(pvarNewValue->vt != VT_DATE || VarDateTime(*pvarNewValue).GetStatus() != COleDateTime::valid || VarDateTime(*pvarNewValue).GetYear() <= 1899) {
							MsgBox("The text you entered does not correspond to a valid date. \n Your changes will not be saved");
							*pbCommit = FALSE;
						}

					break;

					case mlcNote:
						{
							if (pvarNewValue->vt != VT_BSTR) {
								MsgBox("The text you entered is not valid. \n Your changes will not be saved");
								*pbCommit = FALSE;
							}
							else if(CString(pvarNewValue->bstrVal).GetLength() > 4000) {
					
								int result = MsgBox(MB_YESNOCANCEL, "The note you have entered is over the maximum size of 4000 characters.  Would you like to split it into "
									"multiple notes?\n"
									"Pressing 'Yes' will split the note into multiple notes.\n"
									"Pressing 'No' will truncate the existing note.\n"
									"Pressing 'Cancel' will let you edit the note again.");

								if(result == IDCANCEL) {
									*pbCommit = FALSE;
									*pbContinue = FALSE;
									return;
								}

								if(result == IDNO) {
									CString strNewValue(pvarNewValue->bstrVal);
									::VariantClear(pvarNewValue);
									*pvarNewValue = _variant_t(strNewValue.Left(4000)).Detach();
									return;
								}

								//handle the splitting!

								//we just split them at the 4000 marks.

								m_bSplit = true;
								COleDateTime dtOldest;

								CString strNoteRem = CString(pvarNewValue->bstrVal);
								NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
								_variant_t vardtNoteDate = pRow->GetValue(mlcDateCol);
								_variant_t varParentID = pRow->GetValue(mlcParentID);
								_variant_t varChildID = pRow->GetValue(mlcChildID);
								_variant_t varThreadID = pRow->GetValue(mlcThreadID);
					
								long nUserID = GetCurrentUserID();
								COleDateTime dt = VarDateTime(vardtNoteDate);

								CString strCategory = "NULL";
								CString strBillID = "NULL", strLineItemID = "NULL";
					
								CArray<CString, CString&> arNotes;
								while(strNoteRem.GetLength() > 4000) {
									CString strNew;

									//grab the left 4000
									strNew = strNoteRem.Left(4000);
									strNoteRem = strNoteRem.Right(strNoteRem.GetLength() - 4000);

									//add to array
									arNotes.Add(strNew);
								}
								//arNotes.Add(strNoteRem);

								// the first one goes here...
								::VariantClear(pvarNewValue);
								*pvarNewValue = _variant_t(strNoteRem).Detach();

								COleDateTimeSpan dtsSecond(0,0,0,1);
								COleDateTimeSpan dtsBackNotes(0,0,0, arNotes.GetSize() + 1);
								COleDateTime dtSplit = dt - dtsBackNotes;
								dtOldest = dtSplit;
					
								//insert it into the data with the same time + category
								// and add it to the datalist
								// we loop in reverse to give it a readable order
								for (int i = arNotes.GetSize() - 1; i >= 0; i--) {
									long nNewID;
									CString strInsertProcSql;

									dtSplit += dtsSecond;
	
									// (a.walling 2010-08-02 12:11) - PLID 39867 - Moved Notes metadata to NoteInfoT
									// (j.armen 2014-01-31 09:31) - PLID 60568 - Idenitate NoteDataT
									strInsertProcSql.Format("SET NOCOUNT ON "
										"BEGIN TRAN "
										"DECLARE @nNewID int "
										" \r\nIF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
										"INSERT INTO Notes (PersonID, Date, UserID, Category, Note) VALUES (%li, '%s', %li, %s, '%s')"
										" \r\nIF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
										"SET @nNewID = SCOPE_IDENTITY() "
										" \r\nIF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
										"INSERT INTO NoteInfoT (NoteID, BillID, LineItemID, PatientMessagingThreadID, IsPatientCreated) VALUES (@nNewID, %s, %s, %li, 0)"
										" \r\nIF @@ERROR <> 0 BEGIN ROLLBACK TRAN RETURN END\r\n" 
										" COMMIT TRAN "
										"SET NOCOUNT OFF "
										"SELECT @nNewID as NewID ",
										m_id, FormatDateTimeForSql(dtSplit), nUserID, strCategory, _Q(arNotes[i]), strBillID, strLineItemID, VarLong(varThreadID));

									_RecordsetPtr rsID = CreateRecordsetStd(strInsertProcSql);
						
									nNewID = AdoFldLong(rsID, "NewID");

									NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pMessagesList->GetNewRow();
									//copy the existing columns except note + id
									pNewRow->PutValue(mlcParentID, varParentID);
									pNewRow->PutValue(mlcChildID, varChildID);
									pNewRow->PutValue(mlcThreadID, varThreadID);
									pNewRow->PutValue(mlcNoteID, (long)nNewID);
									pNewRow->PutValue(mlcDateCol, COleVariant(dtSplit));
									pNewRow->PutValue(mlcSubject, g_cvarNull);
									pNewRow->PutValue(mlcStatus, g_cvarNull);									
									pNewRow->PutValue(mlcNote, _bstr_t(arNotes[i]));

									m_pMessagesList->AddRowSorted(pNewRow, pParentRow);
								}

								pRow->PutValue(mlcDateCol, COleVariant(dtOldest));					
							}
						}						
					break;
				}

			}
			else {

				//parents can edit date, subject and status
				switch (nCol) {

					case mlcSubject:
						{
						//check to make sure its not too long
						CString strSubject = VarString(pvarNewValue, "");

						if (strSubject.GetLength() > 255) {

							MsgBox("The text you entered is longer than the maximum amount (255) and has been truncated.");
							::VariantClear(pvarNewValue);
							*pvarNewValue = _variant_t(strSubject.Left(255)).Detach();
						}
					}
					break;

					case mlcDateCol:
						if (!CheckCurrentUserPermissions(bioPatientNotes, sptDynamic0)) {
							*pbCommit = FALSE;
							return;
						}
						//If this isn't a date, is an invalid date, or has been converted to 12/30/1899 and therefore is some crazy thing.
						if(pvarNewValue->vt != VT_DATE || VarDateTime(*pvarNewValue).GetStatus() != COleDateTime::valid || VarDateTime(*pvarNewValue).GetYear() <= 1899) {
							MsgBox("The text you entered does not correspond to a valid date. \n Your changes will not be saved");
							*pbCommit = FALSE;
						}
					break;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
	
}

// (c.haag 2010-08-26 11:26) - PLID 39473 - Returns the correct "active" person ID.
long CNotesDlg::GetActivePersonID()
{	
	// (a.walling 2010-09-20 11:41) - PLID 40589 - Get the appropriate person ID
	// (a.walling 2010-09-20 11:41) - PLID 40589 - if m_id set manually, GetActivePersonID will always return m_id
	if (m_bIsValidID) {
		ASSERT(m_id != -1);
		return m_id;
		// (a.walling 2010-09-20 11:41) - PLID 40589 - this just uses the override ID now
		/* else if (m_bIsHistoryNote) {
			return m_nHistoryNotePersonID;
		} */
	} else {
		return GetActivePatientID();
	}
}

// (c.haag 2010-08-26 11:26) - PLID 39473 - Returns the correct "active" person name.
CString CNotesDlg::GetActivePersonName()
{
	// (a.walling 2010-09-20 11:41) - PLID 40589 - this just uses the override ID now
	/*
	if(m_bIsHistoryNote)
		return GetExistingPatientName(m_nHistoryNotePersonID);
	else
		return GetActivePatientName();
	*/
	return GetExistingPatientName(GetActivePersonID());
}

//(c.copits 2011-07-05) PLID 22709 - Remember column settings
void CNotesDlg::ColumnSizingFinishedNotes(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {

		SaveColumnSettings();

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-11-08) PLID 22709 - Remember column settings
void CNotesDlg::ColumnSizingFinishedNotesMessageThreads(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try {

		SaveColumnSettings();

	} NxCatchAll(__FUNCTION__);
}

void CNotesDlg::SaveColumnSettings()
{
	try {
		//(c.copits 2011-07-05) PLID 22709 - Taken from RoomManagerDlg.cpp and LabFollowUpDlg.cpp

		if (!IsDlgButtonChecked(IDC_NOTES_REMEMBER_COL_SETTINGS)) {
			return;
		}

		// Write settings to ConfigRT
		long nCurSel = m_SubTab->CurSel;
		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		CString strWidthTemp, strWidth, strSort, strSortTemp;

		switch (nCurSel) {
			case 0:	// Notes tab
				for(int i = 0; i < m_pNxDlNotes->GetColumnCount(); i++) {
					pCol = m_pNxDlNotes->GetColumn(i);
					if(pCol) {
						// Save width and sort of each column
						BOOL bAscending = (VARIANT_TRUE == pCol->GetSortAscending()) ? TRUE : FALSE;
						short nSortPriority = pCol->GetSortPriority();
						DWORD dwValue = MAKELONG(nSortPriority, bAscending);

						strWidthTemp.Format("%li,", pCol->GetStoredWidth());
						strSortTemp.Format("%lu,", dwValue);
					}

					strWidth += strWidthTemp;
					strSort += strSortTemp;
				}
				SetRemotePropertyText("DefaultPatientsNotesColumnSizes", strWidth, 0, GetCurrentUserName());
				SetRemotePropertyText("DefaultPatientsNotesColumnSort", strSort, 0, GetCurrentUserName());
				break;
			case 1:	// Message threads
				for(int i = 0; i < m_pMessagesList->GetColumnCount(); i++) {
					pCol = m_pMessagesList->GetColumn(i);
					if(pCol) {
						// Save width and sort of each column
						BOOL bAscending = (VARIANT_TRUE == pCol->GetSortAscending()) ? TRUE : FALSE;
						short nSortPriority = pCol->GetSortPriority();
						DWORD dwValue = MAKELONG(nSortPriority, bAscending);

						strWidthTemp.Format("%li,", pCol->GetStoredWidth());
						strSortTemp.Format("%lu,", dwValue);
					}

					strWidth += strWidthTemp;
					strSort += strSortTemp;
				}
				SetRemotePropertyText("DefaultPatientsNotesMessageThreadsColumnSizes", strWidth, 0, GetCurrentUserName());
				SetRemotePropertyText("DefaultPatientsNotesMessageThreadsColumnSort", strSort, 0, GetCurrentUserName());
				break;
		}

		SetColumnSizes();
	} NxCatchAll(__FUNCTION__);
}

void CNotesDlg::SetColumnSizes()
{
	try {
		//(c.copits 2011-07-05) PLID 22709 - Taken from RoomManagerDlg.cpp and LabFollowUpDlg.cpp
		
		if (!IsDlgButtonChecked(IDC_NOTES_REMEMBER_COL_SETTINGS)) {
			return;
		}

		CString strCols;
		long nCurSel = m_SubTab->CurSel;
		switch (nCurSel) {
			case 0: // Notes tab		
				strCols = GetRemotePropertyText("DefaultPatientsNotesColumnSizes", "", 0, GetCurrentUserName(), true);

				if(strCols.IsEmpty()) {
					return;
				}

				if(!strCols.IsEmpty()) {
					NXDATALIST2Lib::IColumnSettingsPtr pCol;
					int nWidth = 0, i=0;

					//parse the columns out and set them
					int nComma = strCols.Find(",");
					while(nComma > 0) {
						nWidth = atoi(strCols.Left(nComma));
						strCols = strCols.Right(strCols.GetLength() - (nComma+1));

						pCol = m_pNxDlNotes->GetColumn(i);
						if(pCol != NULL) {
							pCol->PutStoredWidth(nWidth);
						}
						i++;
						nComma = strCols.Find(",");
					}
				}
				break;
			case 1: // Message Threads
				strCols = GetRemotePropertyText("DefaultPatientsNotesMessageThreadsColumnSizes", "", 0, GetCurrentUserName(), true);
				if(strCols.IsEmpty()) {
					return;
				}

				if(!strCols.IsEmpty()) {
					NXDATALIST2Lib::IColumnSettingsPtr pCol;
					int nWidth = 0, i=0;

					//parse the columns out and set them
					int nComma = strCols.Find(",");
					while(nComma > 0) {
						nWidth = atoi(strCols.Left(nComma));
						strCols = strCols.Right(strCols.GetLength() - (nComma+1));

						pCol = m_pMessagesList->GetColumn(i);
						if(pCol != NULL) {
							pCol->PutStoredWidth(nWidth);
						}
						i++;
						nComma = strCols.Find(",");
					}
				}
				break;

		}

	} NxCatchAll(__FUNCTION__);
}

void CNotesDlg::SetColumnSorts()
{

	try {

		if (!IsDlgButtonChecked(IDC_NOTES_REMEMBER_COL_SETTINGS)) {
			return;
		}

		//(c.copits 2011-07-05) PLID 22709 - Taken from RoomManagerDlg.cpp and LabFollowUpDlg.cpp
		CString strSorts;
		long nCurSel = m_SubTab->CurSel;
		switch (nCurSel) {
			case 0: // Notes tab
				strSorts = GetRemotePropertyText("DefaultPatientsNotesColumnSort", "", 0, GetCurrentUserName(), true);

				if(strSorts.IsEmpty()) {
					return;
				}

				if(!strSorts.IsEmpty()) {
					NXDATALIST2Lib::IColumnSettingsPtr pCol;
					int i = 0;
					DWORD dwSort;

					//parse the columns out and set them
					int nComma = strSorts.Find(",");
					while(nComma > 0) {
						dwSort = atoi(strSorts.Left(nComma));
						strSorts = strSorts.Right(strSorts.GetLength() - (nComma+1));

						pCol = m_pNxDlNotes->GetColumn(i);
						if(pCol != NULL) {

							short nSortPriority = LOWORD(dwSort);
							BOOL bAscending = HIWORD(dwSort);

							pCol->PutSortPriority(nSortPriority);
							pCol->PutSortAscending(bAscending ? VARIANT_TRUE : VARIANT_FALSE);
						}

						i++;
						nComma = strSorts.Find(",");
					}
				}
				break;
			case 1: // Message threads
				strSorts = GetRemotePropertyText("DefaultPatientsNotesMessageThreadsColumnSort", "", 0, GetCurrentUserName(), true);
				
				if(strSorts.IsEmpty()) {
					return;
				}

				if(!strSorts.IsEmpty()) {
					NXDATALIST2Lib::IColumnSettingsPtr pCol;
					int i = 0;
					DWORD dwSort;

					//parse the columns out and set them
					int nComma = strSorts.Find(",");
					while(nComma > 0) {
						dwSort = atoi(strSorts.Left(nComma));
						strSorts = strSorts.Right(strSorts.GetLength() - (nComma+1));

						pCol = m_pMessagesList->GetColumn(i);
						if(pCol != NULL) {

							short nSortPriority = LOWORD(dwSort);
							BOOL bAscending = HIWORD(dwSort);

							pCol->PutSortPriority(nSortPriority);
							pCol->PutSortAscending(bAscending ? VARIANT_TRUE : VARIANT_FALSE);
						}

						i++;
						nComma = strSorts.Find(",");
					}
				}
				break;
		}

	} NxCatchAll(__FUNCTION__);
}

//(c.copits 2011-07-05) PLID 22709 - Remember column settings
void CNotesDlg::OnBnClickedRememberColSettings()
{
	try {
		long nCurSel = m_SubTab->CurSel;
		switch (nCurSel) {
			case 0: // Notes tab
				if (IsDlgButtonChecked(IDC_NOTES_REMEMBER_COL_SETTINGS)) {
					SetRemotePropertyInt("DefaultPatientsNotesRememberColumnSettings", 1, 0, GetCurrentUserName());
					SaveColumnSettings();
				}
				else {
					SetRemotePropertyInt("DefaultPatientsNotesRememberColumnSettings", 0, 0, GetCurrentUserName());
				}
				break;
			case 1: // Message threads
				if (IsDlgButtonChecked(IDC_NOTES_REMEMBER_COL_SETTINGS)) {
					SetRemotePropertyInt("DefaultPatientsNotesMessageThreadsRememberColumnSettings", 1, 0, GetCurrentUserName());
					SaveColumnSettings();
				}
				else {
					SetRemotePropertyInt("DefaultPatientsNotesMessageThreadsRememberColumnSettings", 0, 0, GetCurrentUserName());
				}
				break;
		}
	} NxCatchAll(__FUNCTION__);
}

// (j.armen 2011-08-04 16:18) - PLID 13283 - Handle label clicks
LRESULT CNotesDlg::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UINT nIdc = (UINT)wParam;
		switch(nIdc) 
		{
			case IDC_MULTIPLE_NOTE_CAT_FILTER:
				FilterNotesByRow(m_pCategories->SearchByColumn(0, _bstr_t(ID_MULTIPLE_CATS), 0, FALSE), TRUE);	
				break;
			default:
				ASSERT(FALSE);
				break;
		}
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.armen 2011-08-04 16:19) - PLID 13283 - Handle setting cursor on mouse over
BOOL CNotesDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	try {
		CPoint pt;
		CRect rc;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		if(m_nxlMultipleCatSelection.IsWindowVisible() && m_nxlMultipleCatSelection.IsWindowEnabled())
		{
			m_nxlMultipleCatSelection.GetWindowRect(rc);
			ScreenToClient(&rc);
			if(rc.PtInRect(pt))
			{
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}NxCatchAll(__FUNCTION__);
	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

// (b.spivey, October 05, 2012) - PLID 30398 - Event handlers for hiding bill/claim/statement notes
void CNotesDlg::OnHideBillNotes()
{
	try {
		SetRemotePropertyInt("PatientNotesHideOtherBillNotes", m_checkHideBills.GetCheck(), 0, GetCurrentUserName());
		LoadList();
	}NxCatchAll(__FUNCTION__);	
}

void CNotesDlg::OnHideClaimNotes()
{
	try {
		SetRemotePropertyInt("PatientNotesHideClaimNotes", m_checkHideClaims.GetCheck(), 0, GetCurrentUserName());
		LoadList();
	}NxCatchAll(__FUNCTION__);
}

void CNotesDlg::OnHideStatementNotes()
{
	try {
		SetRemotePropertyInt("PatientNotesHideStatementNotes", m_checkHideStatements.GetCheck(), 0, GetCurrentUserName());
		LoadList();
	}NxCatchAll(__FUNCTION__);
}

// (a.wilson 2014-08-11 08:20) - PLID 63244 - ensure the category filter is uptodate.
void CNotesDlg::EnsureUpdatedCategoryFilter(const long & nID /* = -1 */)
{
	if (m_tcNoteCategories.Changed()) {
		try {
			//requery the combo source
			// (b.cardillo 2006-12-13 13:02) - PLID 6808 - Changed the notes list to a dl2 for pixel-wise vertical scrolling.
			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pNxDlNotes->GetColumn(COLUMN_CATEGORY);
			pCol->PutComboSource(_bstr_t("SELECT ID, Description FROM NoteCatsF UNION SELECT -1, '{No Category}' ORDER BY Description"));

			//From OnTableChanged()
			if (nID > -1) {
				long nRow = m_pCategories->CurSel;
				long nCurrentID = -4;
				if (nRow != sriNoRow) {
					nCurrentID = VarLong(m_pCategories->GetValue(nRow, 0), -4); // -4 is not used as an id within the categories.
				}

				_RecordsetPtr rs = CreateParamRecordset("SELECT Description FROM NoteCatsF WHERE ID = {INT}", nID);

				//Remove the row so that we can either keep it removed or reinsert its updated state.
				m_pCategories->RemoveRow(m_pCategories->FindByColumn(0, nID, 0, VARIANT_FALSE));
				if (!rs->eof) {
					IRowSettingsPtr pRow = m_pCategories->GetRow(sriGetNewRow);
					pRow->PutValue(0, nID);
					pRow->PutValue(1, _bstr_t(AdoFldString(rs, "Description")));
					m_pCategories->AddRow(pRow);
				}
				//attempt to reselect the previous option if it was the row we updated.
				if (nRow != sriNoRow && nCurrentID > -4 && nCurrentID == nID) {
					m_pCategories->SetSelByColumn(0, nCurrentID);
					if (m_pCategories->CurSel == sriNoRow) {
						//if the currentid was wiped out then set to {all}
						m_pCategories->SetSelByColumn(0, -2);
					}
				}
			}
			//From UpdateView()
			else {
				long nCurrentID = -4; // -4 is not used as an id within the categories.
				if (m_pCategories->CurSel != sriNoRow) {
					nCurrentID = VarLong(m_pCategories->GetValue(m_pCategories->CurSel, 0), -4);
				}
				m_pCategories->Requery();

				if (nCurrentID > -4) {
					m_pCategories->SetSelByColumn(0, nCurrentID);
					if (m_pCategories->CurSel == sriNoRow) {
						//if the currentid was wiped out then set to {all}
						m_pCategories->SetSelByColumn(0, -2);
					}
				}
			}
		} NxCatchAll(__FUNCTION__);
	}
}

// (j.gruber 2014-12-12 15:00) - PLID 64391 - Rescheduling Queue - attached appointments to notes
void CNotesDlg::SetApptInformation(long nApptID, COleDateTime dtStartTime, CString strApptTypePurpose, CString strApptResources)
{
	m_nApptID = nApptID;
	m_dtApptStartTime = dtStartTime;
	m_strApptTypePurpose = strApptTypePurpose;
	m_strApptResources = strApptResources;
}
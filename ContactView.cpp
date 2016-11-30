// ContactView.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ContactView.h"
#include "GlobalReportUtils.h"
#include "GlobalUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "AdvPrintOptions.h"
#include "GlobalDataUtils.h"
#include "boost/bind.hpp"
#include "ContactsGeneral.h"
#include "ContactsNotes.h"
#include "ContactTodo.h"
#include "HistoryDlg.h"
#include "AttendanceDlg.h"
#include "ListMergeDlg.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CContactView

IMPLEMENT_DYNCREATE(CContactView, CNxTabView)

// (j.jones 2013-05-08 08:45) - PLID 56591 - changed the child tabs to be declared by reference,
// which means their initialization is slightly different now
CContactView::CContactView()
	: m_generalSheet(*(new CContactsGeneral(this)))
	, m_notesSheet(*(new CContactsNotes(this)))
	, m_todoSheet(*(new CContactTodo(this)))
	, m_HistorySheet(*(new CHistoryDlg(this)))
	, m_dlgAttendanceSheet(*(new CAttendanceDlg(this)))
{
	m_bPatientsToolBar = false;
	m_bContactsToolBar = true;
}

CContactView::~CContactView()
{
	try {

		// (j.jones 2013-05-08 08:45) - PLID 56591 - changed the child tabs to be declared by reference,
		// which means we have to clean up their pointers now
		// They are never null. They have to have been filled in the constructor.
		delete &m_generalSheet;
		delete &m_notesSheet;
		delete &m_todoSheet;
		delete &m_HistorySheet;
		delete &m_dlgAttendanceSheet;
	
	}NxCatchAll(__FUNCTION__);
}


BEGIN_MESSAGE_MAP(CContactView, CNxTabView)
	//{{AFX_MSG_MAP(CContactView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
	ON_COMMAND(ID_FILE_PRINT, CContactView::OnPrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CContactView::OnPrintPreview)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx) // (s.tullis 2014-09-05 13:15) - PLID 63226
	ON_UPDATE_COMMAND_UI(IDM_UPDATE_VIEW, CNxTabView::OnUpdateViewUI) // (z.manning 2010-07-19 16:06) - PLID 39222
	// (a.walling 2016-02-15 08:11) - PLID 67826 - Combine Providers
	ON_UPDATE_COMMAND_UI(ID_CONTACT_COMBINE, OnUpdateContactCombine)
	ON_COMMAND(ID_CONTACT_COMBINE, CContactView::OnContactCombine)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContactView diagnostics

#ifdef _DEBUG
void CContactView::AssertValid() const
{
	CNxTabView::AssertValid();
}

void CContactView::Dump(CDumpContext& dc) const
{
	CNxTabView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CContactView message handlers

BOOL CContactView::CheckPermissions()
{
	if (!UserPermission(ContactModuleItem)) {
		return FALSE;
	}
	return TRUE;
}

int CContactView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxTabView::OnCreate(lpCreateStruct) == -1)
		return -1;

	Modules::Tabs& tabs = g_Modules[Modules::Contacts]->Reset().GetTabs();

	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to create the tabs

	CreateSheet(&m_generalSheet, tabs, ContactsModule::GeneralTab);
	CreateSheet(&m_notesSheet, tabs, ContactsModule::NotesTab);
	CreateSheet(&m_todoSheet, tabs, ContactsModule::FollowUpTab);
	//DRT 11/11/2003 - This should have been done all along!  Changed this to just use the History dialog here instead of
	//	a whole new dialog.  I had to make minimal changes in the history dialog to handle this (about 10 minutes work).
//	CreateSheet(&m_HistorySheet, "&History");
	CreateSheet(&m_HistorySheet, tabs, ContactsModule::HistoryTab);

	// (z.manning, 11/28/2007) - PLID 28218 - Remove this check if we ever give this feature to clients.
	if(IsNexTechInternal()) {
		// (z.manning, 11/28/2007) - PLID 28216 - Added a sheet for attendance tracking for users.
		CreateSheet(&m_dlgAttendanceSheet, tabs, ContactsModule::AttendanceTab);
		CheckViewNotesPermissions();
	}

	//DRT 4/18/2008 - PLID 29711 - Set the text color of the tabs.
	// (j.jones 2016-04-18 17:46) - NX-100214 - the NxTab control now defaults
	// to the value of CNexTechDialog::GetDefaultNxTabTextColor()

	//tell the history dialog we're not in kansas anymore!
	m_HistorySheet.m_bInPatientsModule = false;
	return 0;
}

// Menu Updates
void CContactView::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)		{ pCmdUI->Enable(); }
void CContactView::OnUpdateFilePrint(CCmdUI* pCmdUI)			{ pCmdUI->Enable(); }



void CContactView::OnPrintPreview()
{
	PrintTab(true);
}

BOOL CContactView::PrintTab(bool Preview, CPrintInfo *pInfo /* =0 */)
{
	CString strFilter;

	switch(GetActiveTab()){
	case ContactsModule::GeneralTab:
		{	//General
			try {
				long nContactID = GetActiveContactID();
				int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();
				CString strRepName;
				long nRepID;

				switch(nStatus) {
				case 0x0:
					//other contact
					nRepID = 451;
					strRepName = "ContactsPreviewOther";
					break;
				case 0x1:
					//referring physician
					nRepID = 452;
					strRepName = "ContactsPreviewRefPhys";
					break;
				case 0x2:
					//provider
					nRepID = 448;
					strRepName = "ContactsPreviewProvider";
					break;
				case 0x4:
					//user
					nRepID = 449;
					strRepName = "ContactsPreviewUser";
					break;
				case 0x8:
					//supplier
					nRepID = 450;
					strRepName = "ContactsPreviewSupplier";
					break;
				default:
					//unknown
					ASSERT(FALSE);
				}

				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nRepID)]);

				//add the personid we want via the extra filter
				infReport.nExtraID = nContactID;

				//Made new function for running reports - JMM 5-28-04
				RunReport(&infReport, Preview, (CWnd *)this, "Contacts General", pInfo);
				
			} NxCatchAll("Error in CContactView::PrintTab:General");
		}
		break;

	case ContactsModule::FollowUpTab:
		{
			CAdvPrintOptions PrintOption(this);
			int nContactID = GetActiveContactID();

			_RecordsetPtr rsTmp;
			CString sql;
			COleDateTime dateTmp;
			
			// set up Printing Options
			if (Preview) PrintOption.m_btnCaption = "Preview";
			else PrintOption.m_btnCaption = "Print";
			PrintOption.m_bDateRange = true;
			PrintOption.m_bAllOptions = false;
			PrintOption.m_bDetailed = false;
			PrintOption.m_bOptionCombo = false;

			// set up from date
			sql.Format ("SELECT Min(ToDoList.Deadline) AS FromDate FROM ToDoList WHERE (((ToDoList.PersonID)=%d));", nContactID);
			rsTmp = CreateRecordsetStd(sql);
			//when you select the Min of a value, if there are no records, it will return a VT_NULL variant as the only record
			//so you can't use the rsTmp->eof flag to filter on this
			if (rsTmp->eof || rsTmp->Fields->Item["FromDate"]->Value.vt == VT_NULL) {
				MessageBox("No todo's for this Contact");
				rsTmp->Close();
				return FALSE;
			}
			dateTmp = AdoFldDateTime(rsTmp, "FromDate");
			PrintOption.m_dtInitFrom = dateTmp;
			rsTmp->Close();

			// set up to date
			sql.Format ("SELECT Max(ToDoList.Deadline) AS ToDate FROM ToDoList WHERE (((ToDoList.PersonID)=%d));", nContactID);
			rsTmp = CreateRecordsetStd(sql);
			//when you select the Min of a value, if there are no records, it will return a VT_NULL variant as the only record
			//so you can't use the rsTmp->eof flag to filter on this
			if (rsTmp->Fields->Item["ToDate"]->Value.vt == VT_NULL) {
				MessageBox("No to do's for this Contact");
				rsTmp->Close();
				return FALSE;
			}
			dateTmp = AdoFldDateTime(rsTmp, "ToDate");
			PrintOption.m_dtInitTo = dateTmp;
			rsTmp->Close();

			int nResult = PrintOption.DoModal();
			
			if (nResult == IDOK) {
				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(15)]);
				infReport.nPatient = nContactID;

				if(m_todoSheet.FinishedChecked())	//we're filtering completed items
					infReport.nExtraID = 2;
				else if (m_todoSheet.UnfinishedChecked())	//filtering incomplete items
					infReport.nExtraID = 1;
				else
					infReport.nExtraID = -1;

							
				if(PrintOption.m_bDateRange)
				{
					infReport.strDateOptions = "1;Deadline;Date;;";
					infReport.nDateFilter = 1;
					infReport.nDateRange = 2;
					infReport.DateFrom = PrintOption.m_dtFromDate;
					infReport.DateTo   = PrintOption.m_dtToDate;
				}
				
				CRParameterInfo *paramInfo = new CRParameterInfo;
				CPtrArray paParams;
				CString strDateFrom, strDateTo;
				strDateFrom = PrintOption.m_dtFromDate.Format(VAR_DATEVALUEONLY);
				strDateTo = PrintOption.m_dtToDate.Format(VAR_DATEVALUEONLY);
				paramInfo->m_Data = strDateFrom;
				paramInfo->m_Name = "DateFrom";
				paParams.Add((void *)paramInfo);
				paramInfo = new CRParameterInfo;
				paramInfo->m_Data = strDateTo;
				paramInfo->m_Name = "DateTo";
				paParams.Add((void *)paramInfo);

				// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
				RunReport(&infReport, &paParams, Preview, (CWnd *)this, "Contact ToDo List", pInfo, TRUE);
				ClearRPIParameterList(&paParams);	//Cleanup after ourselves
			}		
			
		}
		break;
	case ContactsModule::NotesTab:
		{
		CAdvPrintOptions		PrintOption(this);
		int nContactID = GetActiveContactID();
		COleDateTime dateTmp;
/*		CReportInfo  infReport(CReports::gcs_aryKnownReports[131]);
		infReport.ViewReport("Contact Notes", "ContactNotes", TRUE, this);*/
		try{
			// set up Printing Options
			if (Preview) PrintOption.m_btnCaption = "Preview";
			else PrintOption.m_btnCaption = "Print";
			PrintOption.m_bDateRange = true;
			PrintOption.m_bDetailed = false;
			PrintOption.m_bOptionCombo = true;
			PrintOption.m_cBoundCol		 = 1;
			PrintOption.m_cAllCaption	 = "All Categories";
			PrintOption.m_cSingleCaption = "One Category";

			//Does this contact even have any notes?
			if (!ExistsInTable("Notes", "PersonID = %li", nContactID)){
				MessageBox("No Notes for this Contact");
				return false;
			}
			
			// set up from date
			_RecordsetPtr rs = CreateRecordset("SELECT Min(Notes.Date) AS FromDate FROM Notes WHERE (((Notes.PersonID)=%li))", nContactID);
			rs->MoveFirst();
			dateTmp = AdoFldDateTime(rs, "FromDate");
			PrintOption.m_dtInitFrom = dateTmp;
			rs->Close();

			// set up to date
			rs = CreateRecordset("SELECT Max(Notes.Date) AS ToDate FROM Notes WHERE (((Notes.PersonID)=%li));", nContactID);
			rs->MoveFirst();
			dateTmp = AdoFldDateTime(rs, "ToDate");
			PrintOption.m_dtInitTo = dateTmp;
			rs->Close();

			//set up the datalist
			PrintOption.m_cSQL = "(SELECT ID, Description AS Text FROM NoteCatsF) as CatQ";

		}NxCatchAll("Error in CContactView::OnPreparePrinting");
	
		int nResult = PrintOption.DoModal();

		if (nResult == IDOK) {	//run the report with the date / category filter
	
			COleDateTime  dateTo, dateFrom;
			CString ToDate, FromDate;

			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(221)]);

			//add the category filter via the Provider
			infReport.nProvider = PrintOption.m_nResult;
			//add 
			infReport.nExtraID = PrintOption.m_nResult;

			//filter on the dates chosen
			if(PrintOption.m_bDateRange)
			{
				infReport.strDateOptions = "1;Note Date;Date;;";
				infReport.nDateFilter = 1;
				infReport.nDateRange = 2;
				infReport.DateFrom = PrintOption.m_dtFromDate;
				infReport.DateTo   = (PrintOption.m_dtToDate);
			}

			//Made new function for running reports - JMM 5-28-04
			// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
			RunReport(&infReport, Preview, (CWnd *)this, "Contact Notes", pInfo, TRUE);
			
			
		}
		}
		break;
		
	case ContactsModule::HistoryTab: //History
		{

			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(407)]);
			infReport.nPatient = GetActiveContactID();

			//Tell the report we're in the contacts module.
			CRParameterInfo *paramInfo = new CRParameterInfo;
			CPtrArray paParams;
			paramInfo->m_Data = "Contact History";
			paramInfo->m_Name = "ReportTitle";
			paParams.Add((void *)paramInfo);
			// (j.jones 2010-04-28 14:20) - PLID 35591 - this report now has a date range, not used in the preview
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "01/01/1000";
			paramInfo->m_Name = "DateFrom";
			paParams.Add(paramInfo);
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "12/31/5000";
			paramInfo->m_Name = "DateTo";
			paParams.Add(paramInfo);

			//Made new function for running reports - JMM 5-28-04
			RunReport(&infReport, &paParams, Preview, (CWnd *)this, "History Tab", pInfo);
			ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves
		}
		break;


	}
	return true;
}

void CContactView::OnPrint()
{
	if(GetActiveTab() == ContactsModule::NotesTab){
		CView::OnFilePrint(); //This will call OnPreparePrinting, which will call PrintTab with pInfo,
							  //which will enable NxReportJob::OutputToPrinter to pop up the printer
							  //options dialog.  It's all very complicated, and the other two tabs do it
							  //by calling CNxTabView::OnFilePrint()
	}
	else{
		PrintTab(false);
	}
}

BOOL CContactView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	if(pInfo->m_bPreview)
		return CNxTabView::OnPreparePrinting(pInfo);
	else{
		if(GetActiveTab() == ContactsModule::NotesTab){
		PrintTab(pInfo->m_bPreview ? 1 : 0, pInfo);
		return false;
		}
		else{
			return CNxTabView::OnPreparePrinting(pInfo);
		}
	}
}
// (s.tullis 2014-09-05 13:15) - PLID 63226
LRESULT CContactView::OnTableChangedEx(WPARAM wParam, LPARAM lParam) {
	try {

		if (GetMainFrame() && !GetMainFrame()->IsActiveView(CONTACT_MODULE_NAME)) {
			//the contacts module is not active, so don't bother updating the active tab
			return 0;
		}

		if (m_pActiveSheet == &m_todoSheet) {
			m_todoSheet.OnTableChangedEx(wParam, lParam);
		}

	}NxCatchAll(__FUNCTION__)

		return 0;
	}

LRESULT CContactView::OnTableChanged(WPARAM wParam, LPARAM lParam) {
	try {


		// (j.jones 2014-08-08 09:55) - PLID 63232 - if the Contacts module is not active,
		// do nothing, because each tab properly updates itself when it becomes active again
		if (GetMainFrame() && !GetMainFrame()->IsActiveView(CONTACT_MODULE_NAME)) {
			//the contacts module is not active, so don't bother updating the active tab
			return 0;
		}

		// (a.walling 2006-08-15 10:33) - PLID 21991 - We used to send table checkers to the sheets
		// depending on the message. Now we'll send them indiscriminately
		// and let the handler handle it (as per their namesake)
		// Eventually this entire section could be replaced by m_pActiveSheet.SendMessage(WM_TABLE_CHANGED, wParam, lParam)
		// so we wouldn't even really need to know which class the active sheet is, and it would ignore if it didnt have TCs,
		// but I will keep it like this so it is in sync with the patientview.

		if (m_pActiveSheet == &m_HistorySheet) {
			m_HistorySheet.OnTableChanged(wParam, lParam);
		}
		else if (m_pActiveSheet == &m_notesSheet) {
			m_notesSheet.OnTableChanged(wParam, lParam);
		}
		else if (m_pActiveSheet == &m_todoSheet) {
			m_todoSheet.OnTableChanged(wParam, lParam);
		}
		else if (m_pActiveSheet == &m_generalSheet) {
			m_generalSheet.OnTableChanged(wParam, lParam);
		}

	} NxCatchAll(__FUNCTION__);

	return 0;
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CContactView::Hotkey(int key)
{	
	if(IsWindowEnabled()) {
		int nKey = IgnoreRightAlt() ? VK_LMENU : VK_MENU;
		if (GetAsyncKeyState(nKey) & 0xE000)
		{	short tab = -1;
			switch (key)
			{	case 'G':
					tab = ContactsModule::GeneralTab;
					break;
				case 'N':
					//admins do not apply, so if not admin and it is locked, then lock it.
					if(!IsCurrentUserAdministrator() && m_bIsLocked)
						break;
					tab = ContactsModule::NotesTab;
					break;
				case 'U':
					tab = ContactsModule::FollowUpTab;
					break;
				// (z.manning 2011-06-17 15:29) - PLID 41656 - Changed this from H to R to match patients module and not conflict with help menu
				case 'R':
					tab = ContactsModule::HistoryTab;
					break;
				// (v.maida - 2014-02-07 10:52) - PLID 38198 - Added the ALT+D hotkey for switching to the Attendance tab.
				case 'D':
					if (IsNexTechInternal()) {
						tab = ContactsModule::AttendanceTab;
					}
					break;
				default:
					tab = -1;
					break;
			}

			if(tab != -1) {
				SetActiveTab(tab);
				return 0;
			}
		}
	}

	return CNxTabView::Hotkey(key);
}

void CContactView::OnSelectTab(short newTab, short oldTab)//used for the new NxTab
{
	//TES 10/29/03: If this tab is being selected by the initialization of the module, 
	//which will be the case if newTab == oldTab, then silently see if they have read permissions,
	//and if they don't, send them back to the general tab.
	if (newTab == ContactsModule::NotesTab) {
		if(oldTab == ContactsModule::NotesTab) {
			if(!(GetCurrentUserPermissions(bioContactsNotes) & SPT__R________)) {
				OnSelectTab(ContactsModule::GeneralTab,ContactsModule::GeneralTab);
				m_tab->CurSel = ContactsModule::GeneralTab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioContactsNotes,sptRead)) {
			m_tab->CurSel = oldTab;
			return;
		}
	}
	else if (newTab == ContactsModule::FollowUpTab) {
		if(oldTab == ContactsModule::FollowUpTab) {
			if(!(GetCurrentUserPermissions(bioContactsFollowUp) & SPT__R________)) {
				OnSelectTab(ContactsModule::GeneralTab,ContactsModule::GeneralTab);
				m_tab->CurSel = ContactsModule::GeneralTab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioContactsFollowUp,sptRead)) {
			m_tab->CurSel = oldTab;
			return;
		}
	}
	// (d.thompson 2009-06-05) - PLID 34500 - This permission has existed for over 6 years, and never done anything.
	else if (newTab == ContactsModule::HistoryTab) {
		if(oldTab == ContactsModule::HistoryTab) {
			if(!(GetCurrentUserPermissions(bioContactHistory) & SPT__R________)) {
				OnSelectTab(ContactsModule::GeneralTab,ContactsModule::GeneralTab);
				m_tab->CurSel = ContactsModule::GeneralTab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioContactHistory, sptRead)) {
			m_tab->CurSel = oldTab;
			return;
		}
	}

	CNxTabView::OnSelectTab(newTab, oldTab);

	// (z.manning, 02/08/2008) - PLID 28216 - In Internal only, make sure we hide the the contact toolbar
	// when we enter the attendance tab.
	if(IsNexTechInternal()) {
		if(newTab == ContactsModule::AttendanceTab) {
			m_bContactsToolBar = false;
		}
		else {
			m_bContactsToolBar = true;
		}
		ShowToolBars();
	}
}

// (k.messina 2010-04-09 17:09) - PLID 37957 for internal use: check for permissions to view notes
bool CContactView::CheckViewNotesPermissions()
{
	//assume we have access (change this to locked if needed later on)
	m_bIsLocked = false;

	Modules::Tabs& tabs = g_Modules[Modules::Contacts]->GetTabs();

	bool bIsBlocked = true;
	if (!IsCurrentUserAdministrator()) {
		//check if this account is blocked from this contact's notes
		_RecordsetPtr rs = CreateParamRecordset("SELECT CAST(1 AS BIT) AS LOCKED  "
			"WHERE EXISTS(SELECT PersonID FROM ContactsLockedT WHERE PersonID = {INT}) "
			"AND NOT EXISTS(SELECT UserID FROM ContactAccessT WHERE UserID = {INT}) ",
			GetActiveContactID(), GetCurrentUserID());		
		bIsBlocked = rs->eof ? false : true;
	} else {
		bIsBlocked = false; // (a.walling 2011-02-16 14:17) - PLID 40444 - Admins should not be blocked, of course
	}

	//admins do not apply, so if not admin and it is locked, then lock it.
	if(bIsBlocked)
	{
		//set to locked now that we see you are not an admin AND have been blocked
		m_bIsLocked = true;

		//if we are on the notes tab, move to general
		 if(m_tab->CurSel == ContactsModule::NotesTab)
			 SetActiveTab(ContactsModule::GeneralTab);

		 //hide notes tab
		tabs[ContactsModule::NotesTab]->Enable(false).Show(false);
	}
	else if(!m_tab->ShowTab[ContactsModule::NotesTab])
	{
		//make sure the notes tab is shown
		tabs[ContactsModule::NotesTab]->Enable(true).Show(true);
	}

	std::for_each(tabs.begin(), tabs.end(), boost::bind(&Modules::NxTabUpdater, m_tab, _1));

	return m_bIsLocked;
}


// (a.walling 2016-02-15 08:11) - PLID 67826 - Combine Providers
void CContactView::OnUpdateContactCombine(CCmdUI * pCmdUI)
{
	try {
		// (a.walling 2016-02-15 13:03) - PLID 68261 - New permissions for Location merge and Provider merge
		// apparently we just always enable this and show a permission warning when actually clicked.
		pCmdUI->Enable();
	} NxCatchAllIgnore()
}

// (a.walling 2016-02-15 08:11) - PLID 67826 - Combine Providers
void CContactView::OnContactCombine()
{
	try {
		if (!CheckCurrentUserPermissions(bioMergeProviders, sptDynamic0)) {
			return;
		}

		CListMergeDlg dlg(this);
		dlg.m_eListType = mltProviders;
		dlg.DoModal();
		UpdateView();
	} NxCatchAll(__FUNCTION__);
}


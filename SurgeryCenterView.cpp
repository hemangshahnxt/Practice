// AdminView.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "SurgeryCenterView.h"
#include "GlobalReportUtils.h"
#include "InternationalUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "PreferenceCardsDlg.h"
#include "CaseHistoriesDlg.h"
#include "CredentialsDlg.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CPracticeApp theApp;

using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CContactView

IMPLEMENT_DYNCREATE(CSurgeryCenterView, CNxTabView)


// (j.jones 2013-05-08 09:10) - PLID 56591 - changed the child tabs to be declared by reference,
// which means their initialization is slightly different now
CSurgeryCenterView::CSurgeryCenterView()
	: m_sheetPreferenceCardSetup(*(new CPreferenceCardsDlg(this)))
	, m_sheetCaseHistories(*(new CCaseHistoriesDlg(this)))
	, m_sheetCredentials(*(new CCredentialsDlg(this)))
{
	m_bMarketToolBar = false;
	m_bPatientsToolBar = false;
	m_bContactsToolBar = false;
	m_bDateToolBar = false;
}

CSurgeryCenterView::~CSurgeryCenterView()
{
	try {

		// (j.jones 2013-05-08 09:10) - PLID 56591 - changed the child tabs to be declared by reference,
		// which means we have to clean up their pointers now
		// They are never null. They have to have been filled in the constructor.
		delete &m_sheetPreferenceCardSetup;
		delete &m_sheetCaseHistories;
		delete &m_sheetCredentials;
	
	}NxCatchAll(__FUNCTION__);
}


BEGIN_MESSAGE_MAP(CSurgeryCenterView, CNxTabView)
	//{{AFX_MSG_MAP(CContactView)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(IDM_UPDATE_VIEW, CNxTabView::OnUpdateViewUI) // (z.manning 2010-07-19 16:06) - PLID 39222
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSurgeryCenterView diagnostics

#ifdef _DEBUG
void CSurgeryCenterView::AssertValid() const
{
	CNxTabView::AssertValid();
}

void CSurgeryCenterView::Dump(CDumpContext& dc) const
{
	CNxTabView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CContactView message handlers

BOOL CSurgeryCenterView::CheckPermissions()
{
	if(!g_pLicense->CheckForLicense(CLicense::lcSurgeryCenter, CLicense::cflrUse))
		return FALSE;
	if (!CheckCurrentUserPermissions(bioASCModule,sptView))
		return FALSE;

	return TRUE;
}


int CSurgeryCenterView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxTabView::OnCreate(lpCreateStruct) == -1)
		return -1;

	Modules::Tabs& tabs = g_Modules[Modules::SurgeryCenter]->Reset().GetTabs();

	CreateSheet(&m_sheetCaseHistories, tabs, SurgeryCenterModule::CaseHistoryTab);
	// (j.jones 2009-08-25 17:40) - PLID 35338 - this is now the preference cards dialog
	CreateSheet(&m_sheetPreferenceCardSetup, tabs, SurgeryCenterModule::DoctorPrefsTab);
	CreateSheet(&m_sheetCredentials, tabs, SurgeryCenterModule::CredentialsTab);

	//DRT 4/18/2008 - PLID 29711 - Set the text color of the tabs.
	// (j.jones 2016-04-18 17:46) - NX-100214 - the NxTab control now defaults
	// to the value of CNexTechDialog::GetDefaultNxTabTextColor()

	// (a.walling 2008-07-03 09:19) - PLID 27648 - Make these tabs a bit bigger
	m_tab->TabWidth = 8;

	return 0;
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CSurgeryCenterView::Hotkey(int key)
{	
	if(IsWindowEnabled()) {
		int nKey = IgnoreRightAlt() ? VK_LMENU : VK_MENU;
		if (GetAsyncKeyState(nKey) & 0xE000)
		{	short tab = -1;
			switch (key)
			{	case 'C':
					tab = SurgeryCenterModule::CaseHistoryTab;
					break;
				case 'P':
					tab = SurgeryCenterModule::DoctorPrefsTab;
					break;
				case 'D':
					tab = SurgeryCenterModule::CredentialsTab;
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

void CSurgeryCenterView::OnSelectTab(short newTab, short oldTab)//used for the new NxTab
{
	if (oldTab == SurgeryCenterModule::CaseHistoryTab && newTab != SurgeryCenterModule::CaseHistoryTab)
	{
		//if they are on the case history tab, don't let them leave if they have open case histories
		if(theApp.m_arypCaseHistories.GetSize() > 0) {
			long nSize = theApp.m_arypCaseHistories.GetSize();
			for (long i=0; i<nSize; i++) {
				CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
				if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
					MessageBox("Please close all open Case Histories before continuing.");
					m_tab->CurSel = oldTab;
					return;
				}
			}
		}
	}

	if (newTab == SurgeryCenterModule::CredentialsTab) {
		if(oldTab == SurgeryCenterModule::CredentialsTab) {
			if(!(GetCurrentUserPermissions(bioCredentialsTab) & SPT__R________)) {
				OnSelectTab(SurgeryCenterModule::CaseHistoryTab,SurgeryCenterModule::CaseHistoryTab);
				m_tab->CurSel = SurgeryCenterModule::CaseHistoryTab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioCredentialsTab,sptRead)) {
			m_tab->CurSel = oldTab;
			return;
		}
	}

	CNxTabView::OnSelectTab(newTab, oldTab);
}

LRESULT CSurgeryCenterView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
	case NXM_ALLOW_CLOSE:		
		return WarnIfStillOpen();
		break;
	case NXM_PRE_CLOSE:
		PreClose();
		break;
	}
	return CNxView::WindowProc(message, wParam, lParam);
}

LRESULT CSurgeryCenterView::WarnIfStillOpen()
{
	BOOL bWarningGiven = FALSE;

	//check to see if any case histories are open, and if so warn the user
	if (theApp.m_arypCaseHistories.GetSize() > 0) {		
		try {
			long nSize = theApp.m_arypCaseHistories.GetSize();
			for (long i=0; i<nSize; i++) {
				CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
				if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
					if (!bWarningGiven) {
						bWarningGiven = TRUE;
						MessageBox("Please close all open Case Histories before continuing.");
					}
					pWnd->BringWindowToTop();
					// Restore it (or maximize it) if it's currently minimized
					{
						WINDOWPLACEMENT wp;
						pWnd->GetWindowPlacement(&wp);
						if (wp.showCmd == SW_MINIMIZE || wp.showCmd == SW_SHOWMINIMIZED) {
							if ((wp.flags & WPF_RESTORETOMAXIMIZED) != 0) {
								wp.showCmd = SW_SHOWMAXIMIZED;
							} else {
								wp.showCmd = SW_SHOWNORMAL;
							}
							pWnd->SetWindowPlacement(&wp);
						}
					}
				}
			}
		} NxCatchAllCall("CSurgeryCenterView::WarnIfStillOpen",
			bWarningGiven = TRUE;
		);		
	}

	if(bWarningGiven)
		return AC_CANNOT_CLOSE;
	else
		return AC_CAN_CLOSE;
}

void CSurgeryCenterView::PreClose()
{
	if(theApp.m_arypCaseHistories.GetSize() > 0) {
		try {
			long nSize = theApp.m_arypCaseHistories.GetSize();
			for(long i=0; i<nSize; i++) {
				CWnd *pWnd = (CWnd *)theApp.m_arypCaseHistories[i];
				if (pWnd->GetSafeHwnd() && pWnd->IsWindowVisible()) {
					pWnd->SendMessage(WM_COMMAND, IDCANCEL);
				}
			}
		}NxCatchAll("CSurgeryCenterView::PreClose()");
	}
}

void CSurgeryCenterView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{	
	CNxTabView::OnPrint(pDC, pInfo);
}

void CSurgeryCenterView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	pInfo->SetMinPage(1);
	pInfo->SetMaxPage(1);
	
	CNxTabView::OnBeginPrinting(pDC, pInfo);
}

void CSurgeryCenterView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	CNxTabView::OnEndPrinting(pDC, pInfo);
}

BOOL CSurgeryCenterView::OnPreparePrinting(CPrintInfo* pInfo) 
{

	try {
		CString str, 
				sql;
		
		CPtrArray			paParam;		

		CNxDialog* ptrCurrentTab = GetActiveSheet();

		if (ptrCurrentTab == &m_sheetCaseHistories) {

			// Show immediate feedback by way of the wait cursor
			CWaitCursor wc;

			// Create a copy of the report object
			CReportInfo rep(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(387)]);

			if(m_sheetCaseHistories.m_radioDateRange.GetCheck()) {
				rep.nDateFilter = 1;
				rep.nDateRange = 2;
				rep.DateFrom = m_sheetCaseHistories.m_dtFrom.GetValue();
				rep.DateTo = m_sheetCaseHistories.m_dtTo.GetValue();
			}

			if(m_sheetCaseHistories.m_radioIncomplete.GetCheck())
				rep.SetExtraValue("= 0");
			else if(m_sheetCaseHistories.m_radioCompleted.GetCheck())
				rep.SetExtraValue("= 1");

			//check to see if there is a default report
			_RecordsetPtr rsDefault = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = 387");
			CString strFileName;

			if (rsDefault->eof) {

				strFileName = "CaseHistoryIndiv";
			}
			else {
				
				long nDefaultCustomReport = AdoFldLong(rsDefault, "CustomReportID", -1);

				if (nDefaultCustomReport > 0) {

					_RecordsetPtr rsFileName = CreateRecordset("SELECT FileName FROM CustomReportsT WHERE ID = 387 AND Number = %li", nDefaultCustomReport);

					if (rsFileName->eof) {

						//this should never happen
						MessageBox("Practice could not find the custom report.  Please contact NexTech for assistance");
					}
					else {
						
						//set the default
						rep.nDefaultCustomReport = nDefaultCustomReport;
						strFileName =  AdoFldString(rsFileName, "FileName");
					}
				}
				else {
					//if this occurs it just means they want the default, which in this case, there is only one
					strFileName = "CaseHistoryIndiv";
					
				}
			}

			//Made new function for running reports - JMM 5-28-04
			// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
			RunReport(&rep, pInfo->m_bPreview, (CWnd *)this, "Case History", pInfo, m_sheetCaseHistories.m_radioDateRange.GetCheck());

			return FALSE;

		}

		// (j.jones 2009-08-25 17:40) - PLID 35338 - this is now the preference cards dialog
		if (ptrCurrentTab == &m_sheetPreferenceCardSetup) {

			// Show immediate feedback by way of the wait cursor
			CWaitCursor wc;

			// Create a copy of the report object
			CReportInfo rep(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(556)]);

			long nPreferenceCardID = m_sheetPreferenceCardSetup.GetCurrentPreferenceCardID();
			if(nPreferenceCardID != -1) {
				rep.nExtraID = nPreferenceCardID;
			}

			//check to see if there is a default report
			_RecordsetPtr rsDefault = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = 556");
			CString strFileName;

			if (rsDefault->eof) {

				strFileName = "SurgeryASCList";
			}
			else {
				
				long nDefaultCustomReport = AdoFldLong(rsDefault, "CustomReportID", -1);

				if (nDefaultCustomReport > 0) {

					_RecordsetPtr rsFileName = CreateRecordset("SELECT FileName FROM CustomReportsT WHERE ID = 556 AND Number = %li", nDefaultCustomReport);

					if (rsFileName->eof) {

						//this should never happen
						MessageBox("Practice could not find the custom report.  Please contact NexTech for assistance");
					}
					else {
						
						//set the default
						rep.nDefaultCustomReport = nDefaultCustomReport;
						strFileName =  AdoFldString(rsFileName, "FileName");
					}
				}
				else {
					//if this occurs it just means they want the default, which in this case, there is only one
					strFileName = "SurgeryASCList";
					
				}
			}

			//Made new function for running reports - JMM 5-28-04
			RunReport(&rep, pInfo->m_bPreview, (CWnd *)this, "Preference Cards", pInfo);

			return FALSE;

		}
		else {
			return CNxTabView::OnPreparePrinting(pInfo);
		}

	}NxCatchAll("Error in CSurgeryCenterView::OnPreparePrinting()");
	return FALSE;
}

void CSurgeryCenterView::OnUpdateFilePrint(CCmdUI* pCmdUI) 
{
	CNxDialog* ptrCurrentTab = GetActiveSheet();
	
	//there's nothing to print for Credentials, yet
	if (ptrCurrentTab == &m_sheetCredentials) {
		pCmdUI->Enable(FALSE);
	}
	else {
		pCmdUI->Enable();
	}
}

void CSurgeryCenterView::OnUpdateFilePrintPreview(CCmdUI* pCmdUI) 
{
	CNxDialog* ptrCurrentTab = GetActiveSheet();
	
	//there's nothing to print for Credentials, yet
	if (ptrCurrentTab == &m_sheetCredentials) {
		pCmdUI->Enable(FALSE);
	}
	else {
		pCmdUI->Enable();
	}
}

int CSurgeryCenterView::ShowPrefsDlg()
{
	if (m_pActiveSheet == &m_sheetCredentials)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piCredentialing);
	// (j.jones 2009-08-25 17:39) - PLID 35338 - changed to use the preference card prefs
	else if (m_pActiveSheet == &m_sheetPreferenceCardSetup)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(),GetRegistryBase(), piPreferenceCards);
	else
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piAscModule);
}
// LinksView.cpp : implementation file
// (d.thompson 2009-11-16) - PLID 36134 - Created
//

#include "stdafx.h"
#include "Practice.h"
#include "LinksView.h"
#include "boost/bind.hpp"
#include "ExportDlg.h"
#include "HL7BatchDlg.h"
#include "TopsSearchDlg.h"
#include "BOLDLinkDlg.h"
#include "DeviceConfigTabDlg.h"		// (d.lange 2010-05-07 15:48) - PLID 38536 - Added Devices tab dialog
#include "SendLabsDlg.h"		// (a.vengrofski 2010-05-28 09:37) - PLID <38919> - Added the dialog
#include "ReceiveLabsDlg.h"		// (a.vengrofski 2010-07-23 08:56) - PLID <38919> - Added the dialog
#include "DirectMessageReceivedDlg.h" // (j.camacho 2013-10-17 17:54) - PLID 59064
#include "CancerCasesDlg.h" //TES 4/23/2014 - PLID 61854
#include "OnlineVisitsDlg.h"  // (r.farnworth 2016-02-25 09:59) - PLID 68396

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums


// CLinksView

IMPLEMENT_DYNCREATE(CLinksView, CNxTabView)

// (j.jones 2013-05-08 08:37) - PLID 56591 - changed the child tabs to be declared by reference,
// which means their initialization is slightly different now
CLinksView::CLinksView()
	: m_dlgExport(*(new CExportDlg(this)))
	, m_HL7BatchDlg(*(new CHL7BatchDlg(this)))
	, m_TopsSearchDlg(*(new CTopsSearchDlg(this)))
	, m_dlgBoldLink(*(new CBoldLinkDlg(this)))
	, m_dlgDeviceConfigTab(*(new CDeviceConfigTabDlg(this)))
	, m_dlgSendLabsTab(*(new CSendLabsDlg(this)))
	, m_dlgReceiveLabsTab(*(new CReceiveLabsDlg(this)))
	, m_dlgDirectMessageTab(*(new CDirectMessageReceivedDlg(this)))
	, m_dlgCancerCasesTab(*(new CCancerCasesDlg(this)))
	, m_dlgOnlineVisitsTab(*(new COnlineVisitsDlg(this)))
{

}

CLinksView::~CLinksView()
{
	try {

		// (j.jones 2013-05-08 08:37) - PLID 56591 - changed the child tabs to be declared by reference,
		// which means we have to clean up their pointers now
		// They are never null. They have to have been filled in the constructor.
		delete &m_dlgExport;
		delete &m_HL7BatchDlg;
		delete &m_TopsSearchDlg;
		delete &m_dlgBoldLink;
		delete &m_dlgDeviceConfigTab;
		delete &m_dlgSendLabsTab;
		delete &m_dlgReceiveLabsTab;
		delete &m_dlgDirectMessageTab;// (j.camacho 2013-10-17 18:00) - PLID 59064
		delete &m_dlgCancerCasesTab;//TES 4/23/2014 - PLID 61854
		delete &m_dlgOnlineVisitsTab; // (r.farnworth 2016-02-25 09:54) - PLID 68396
	
	}NxCatchAll(__FUNCTION__);
}

BEGIN_MESSAGE_MAP(CLinksView, CNxTabView)
	ON_WM_CREATE()
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrintPreview)
	ON_UPDATE_COMMAND_UI(IDM_UPDATE_VIEW, CNxTabView::OnUpdateViewUI) // (z.manning 2010-07-19 16:06) - PLID 39222
END_MESSAGE_MAP()


// CLinksView diagnostics
#ifdef _DEBUG
void CLinksView::AssertValid() const
{
	CNxTabView::AssertValid();
}

#ifndef _WIN32_WCE
void CLinksView::Dump(CDumpContext& dc) const
{
	CNxTabView::Dump(dc);
}
#endif
#endif //_DEBUG


// CLinksView message handlers
int CLinksView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxTabView::OnCreate(lpCreateStruct) == -1)
		return -1;

	Modules::Tabs& tabs = g_Modules[Modules::Links]->Reset().GetTabs();

	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to create the tabs


	// (d.thompson 2009-11-16) - PLID 36301 - Moved from CFinView
	CreateSheet(&m_dlgExport, tabs, LinksModule::ExportTab);
	// (j.jones 2008-04-08 15:01) - PLID 29587 - added HL7 tab
	CreateSheet(&m_HL7BatchDlg, tabs, LinksModule::HL7Tab);
	//(e.lally 2009-10-07) PLID 35803 - Added TOPS tab
	CreateSheet(&m_TopsSearchDlg, tabs, LinksModule::TOPSTab);
	// (j.gruber 2010-04-29 10:37) - PLID 38337 - BOLD
	CreateSheet(&m_dlgBoldLink, tabs, LinksModule::BOLDTab);
	//TES 4/23/2014 - PLID 61854
	CreateSheet(&m_dlgCancerCasesTab, tabs, LinksModule::CancerCasesTab);
	// (d.lange 2010-05-07 15:37) - PLID 38536 - Added Devices tab
	CreateSheet(&m_dlgDeviceConfigTab, tabs, LinksModule::DeviceConfigTab);
	// (a.vengrofski 2010-05-28 09:38) - PLID <38919> - Added Sending Labs tab
	CreateSheet(&m_dlgSendLabsTab, tabs, LinksModule::SendLabsTab);
	// (a.vengrofski 2010-07-22 17:08) - PLID <38919> - Added Receive Labs tab
	CreateSheet(&m_dlgReceiveLabsTab, tabs, LinksModule::ReceiveLabsTab);
	// (j.camacho 2013-10-17 17:53) - PLID 59064
	CreateSheet(&m_dlgDirectMessageTab, tabs, LinksModule::DirectMessage);
	// (r.farnworth 2016-02-25 09:54) - PLID 68396 - Create a new tab in the Links module that is for Online Visits
	CreateSheet(&m_dlgOnlineVisitsTab, tabs, LinksModule::OnlineVisits);

	//DRT 4/18/2008 - PLID 29711 - Set the text color of the tabs.
	// (j.jones 2016-04-18 17:46) - NX-100214 - the NxTab control now defaults
	// to the value of CNexTechDialog::GetDefaultNxTabTextColor()

	ShowTabs();

	return 0;
}

void CLinksView::OnDraw(CDC* pDC) 
{
	CDocument* pDoc = GetDocument();	
	
}

BOOL CLinksView::CheckPermissions()
{
	if(!CheckCurrentUserPermissions(bio3rdPartyLinks, sptView)) {
		return FALSE;
	}
	return TRUE;
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
// (d.thompson 2009-11-16) - PLID 36134 - Copied from financial to keep the hotkeys consistent.
int CLinksView::Hotkey (int key)
{
	if(IsWindowEnabled()) {
		int nKey = IgnoreRightAlt() ? VK_LMENU : VK_MENU;
		if (GetAsyncKeyState(nKey) & 0xE000)
		{	short CurrentTab = -1;
			switch (key)
			{
				// (d.thompson 2009-11-16) - PLID 36301 - Moved from CFinView
				case 'X':
					CurrentTab = LinksModule::ExportTab;
					break;
				// (j.jones 2008-04-08 15:01) - PLID 29587 - added HL7 tab
				case '7':
					CurrentTab = LinksModule::HL7Tab;
					break;
				//(e.lally 2009-10-07) PLID 35803 - Added TOPS tab
				case 'T':
					CurrentTab = LinksModule::TOPSTab;
					break;
				// (j.gruber 2010-04-28 12:22) - PLID 38337 - BOLD tab
				case 'B':
					CurrentTab = LinksModule::BOLDTab;
				break;
				// (d.lange 2010-05-07 16:01) - PLID 38536 - Devices tab
				case 'D':
					CurrentTab = LinksModule::DeviceConfigTab;
					break;
				// (a.vengrofski 2010-05-28 09:39) - PLID <38919> - Send Labs tab
				case 'S':
					CurrentTab = LinksModule::SendLabsTab;
					break;
				// (a.vengrofski 2010-07-22 17:09) - PLID <38919> - Receive Labs Tab
				case 'R':
					CurrentTab = LinksModule::ReceiveLabsTab;
					break;
				// (j.camacho 2013-11-20 15:21) - PLID 59064 - Direct Message Tab
				case 'I':
					CurrentTab = LinksModule::DirectMessage;
					break;
				//TES 4/23/2014 - PLID 61854 - Cancer Cases tab
				case 'C':
					CurrentTab = LinksModule::CancerCasesTab;
					break;
				// (r.farnworth 2016-02-25 10:06) - PLID 68396 - Create a new tab in the Links module that is for Online Visits
				case 'O':
					CurrentTab = LinksModule::OnlineVisits;
					break;
				default:
					break;
			}
			if (CurrentTab != -1) {
				SetActiveTab(CurrentTab);
				return 0;
			}
		}
	}

	return CNxTabView::Hotkey(key);
}

void CLinksView::OnSelectTab(short newTab, short oldTab)
{
	// (d.thompson 2009-11-16) - PLID 36301 - Moved from CFinView
	if (newTab == LinksModule::ExportTab) {
		//There are no permissions or requirements on the export tab, always fall back here
	}
	// (j.jones 2008-04-08 15:01) - PLID 29587 - added HL7 tab (go back one tab to Export if no license)
	else if (newTab == LinksModule::HL7Tab) {
		if(oldTab == LinksModule::HL7Tab) {
			//TES 5/18/2009 - PLID 34282 - Added HL7 permissions
			if(!(GetCurrentUserPermissions(bioHL7BatchDlg) & SPT__R________) || !g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent)) {
				OnSelectTab(LinksModule::ExportTab, LinksModule::ExportTab);
				return;
			}
			else {
				m_tab->CurSel = LinksModule::HL7Tab;
			}
		}
		//TES 5/18/2009 - PLID 34282 - Added HL7 permissions
		else if (!CheckCurrentUserPermissions(bioHL7BatchDlg,sptRead) || !g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent)) {
			m_tab->CurSel = oldTab;
			return;
		}
	}
	//(e.lally 2009-10-07) PLID 35803 - added TOPS tab, no permissions or licensing in place yet
	// (j.jones 2009-11-04 09:45) - PLID 36135 - there is now a license, we will always display the tab
	// but not allow entry without the license
	else if (newTab == LinksModule::TOPSTab) {
		if(oldTab == LinksModule::TOPSTab) {
			if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcTOPSLink, CLicense::cflrSilent)) {				
				OnSelectTab(LinksModule::ExportTab, LinksModule::ExportTab);
				AfxMessageBox("Please contact NexTech Technical Support to access the TOPS feature.");
				return;
			}
			else {
				m_tab->CurSel = LinksModule::TOPSTab;
			}
		}
		else if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcTOPSLink, CLicense::cflrSilent)) {			
			m_tab->CurSel = oldTab;
			AfxMessageBox("Please contact NexTech Technical Support to access the TOPS feature.");
			return;
		}
	}
	// (j.gruber 2010-04-28 12:25) - PLID 38337
	else if (newTab == LinksModule::BOLDTab) {
		if(oldTab == LinksModule::BOLDTab) {
			// (j.gruber 2010-06-02 08:33) - PLID 38935 - licensing
			if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBold, CLicense::cflrUse)) {				
				OnSelectTab(LinksModule::ExportTab, LinksModule::ExportTab);
				AfxMessageBox("Please contact NexTech Technical Support to access the BOLD link.");
				return;
			}
			else {
				m_tab->CurSel = LinksModule::BOLDTab;
			}
		}
		// (j.gruber 2010-06-02 08:33) - PLID 38935 - licensing
		else if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBold, CLicense::cflrUse)) {			
			m_tab->CurSel = oldTab;
			AfxMessageBox("Please contact NexTech Technical Support to access the BOLD link.");
			return;
		}
	}
	// (d.lange 2010-05-07 16:02) - PLID 38536
	else if (newTab == LinksModule::DeviceConfigTab) {
		if(oldTab == LinksModule::DeviceConfigTab) {
			// (c.haag 2010-06-30 11:25) - PLID 39424 - License checking.
			if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcDeviceImport, CLicense::cflrSilent)) {				
				OnSelectTab(LinksModule::ExportTab, LinksModule::ExportTab);
				AfxMessageBox("Please contact NexTech Technical Support to access the Device Import feature.");
				return;
			}
			else {
				m_tab->CurSel = LinksModule::DeviceConfigTab;
			}
		}
		else if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcDeviceImport, CLicense::cflrSilent)) {
			m_tab->CurSel = oldTab;
			AfxMessageBox("Please contact NexTech Technical Support to access the Device Import feature.");
			return;
		}
	}
	// (a.vengrofski 2010-05-28 09:41) - PLID <38919> - New tab 'Send Labs'
	else if (newTab == LinksModule::SendLabsTab) {
		if(oldTab == LinksModule::SendLabsTab) {
			if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent)) {				
				OnSelectTab(LinksModule::ExportTab, LinksModule::ExportTab);
				AfxMessageBox("Please contact NexTech Technical Support to access the Sending Labs feature.");
				return;
			}
			else {
				m_tab->CurSel = LinksModule::SendLabsTab;
			}
		}
		else if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent)) {
			m_tab->CurSel = oldTab;
			AfxMessageBox("Please contact NexTech Technical Support to access the Sending Labs feature.");
			return;
		}
	}
	// (a.vengrofski 2010-07-22 17:10) - PLID <38919> - New tab 'Receive Labs'
	else if (newTab == LinksModule::ReceiveLabsTab) {
		if(oldTab == LinksModule::ReceiveLabsTab) {
			if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent)) {				
				OnSelectTab(LinksModule::ExportTab, LinksModule::ExportTab);
				AfxMessageBox("Please contact NexTech Technical Support to access the Sending Labs feature.");
				return;
			}
			else {
				m_tab->CurSel = LinksModule::ReceiveLabsTab;
			}
		}
		else if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent)) {
			m_tab->CurSel = oldTab;
			AfxMessageBox("Please contact NexTech Technical Support to access the Sending Labs feature.");
			return;
		}
	}
	// (j.camacho 2013-10-17 18:00) - PLID 59064
	else if (newTab == LinksModule::DirectMessage) {
		// (b.spivey - November 27th, 2013) - PLID 59590 - License changes for Direct Messaging
		if (oldTab == LinksModule::DirectMessage) {
			if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcDirectMessage, CLicense::cflrUse)) {
				
				OnSelectTab(LinksModule::ExportTab, LinksModule::ExportTab);
				AfxMessageBox("Please contact NexTech Technical Support to access the Direct Message Tab.");
				return;
			} else {
				m_tab->CurSel = LinksModule::DirectMessage;
			}
		} else if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcDirectMessage, CLicense::cflrUse)) {
			m_tab->CurSel = oldTab;
			AfxMessageBox("Please contact NexTech Technical Support to access the Direct Message Tab.");
			return;
		}
	}
	//TES 4/23/2014 - PLID 61854
	else if (newTab == LinksModule::CancerCasesTab) {
		//TES 5/5/2014 - PLID 61854 - Show the tab if they have the NexEMR license
		if (oldTab == LinksModule::CancerCasesTab) {
			if (!g_pLicense || !g_pLicense->HasEMR(CLicense::cflrSilent)) {
				OnSelectTab(LinksModule::ExportTab, LinksModule::ExportTab);
				AfxMessageBox("Please contact NexTech Technical Support to access the Cancer Cases Tab.");
				return;
			}
			else {
				if (!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
					OnSelectTab(LinksModule::ExportTab, LinksModule::ExportTab);
					return;
				}
			}
		}
		else {
			if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcNexEMR, CLicense::cflrUse)) {
				m_tab->CurSel = oldTab;
				AfxMessageBox("Please contact NexTech Technical Support to access the Cancer Cases Tab.");
				return;
			}
			else {
				if (!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
					m_tab->CurSel = oldTab;
					return;
				}
			}
		}
	}
	// (r.farnworth 2016-02-25 10:05) - PLID 68396 - Create a new tab in the Links module that is for Online Visits
	else if (newTab == LinksModule::OnlineVisits) {
		// (b.cardillo 2016-03-13 16:13) - PLID 68409 - Control the visibility of the online visit tab
		if (!g_pLicense || g_pLicense->GetIagnosisProviderCount() == 0) {
			if (oldTab == LinksModule::OnlineVisits) {
				m_tab->CurSel = LinksModule::ExportTab;
				OnSelectTab(LinksModule::ExportTab, LinksModule::ExportTab);
			} else {
				m_tab->CurSel = oldTab;
			}
			AfxMessageBox("Please contact NexTech Technical Support to access the Online Visits Tab.");
			return;
		}

		// (r.farnworth 2016-03-04 08:56) - PLID 68451 - Create a permission to get into the Iagnosis Online Visits tab.
		if (!CheckCurrentUserPermissions(bioOnlineVisitTab, sptView)) {
			if (oldTab == LinksModule::OnlineVisits) {
				m_tab->CurSel = LinksModule::ExportTab;
				OnSelectTab(LinksModule::ExportTab, LinksModule::ExportTab);
			}
			else {
				m_tab->CurSel = oldTab;
			}
			return;
		}
	}
	else {
		//require the developer implement this function
		AfxThrowNxException("Unhandled tab selection %li.", newTab);
	}

	CNxTabView::OnSelectTab(newTab, oldTab);
}

void CLinksView::ShowTabs()
{
	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to update the tabs
	Modules::Tabs& tabs = g_Modules[Modules::Links]->GetTabs();
	std::for_each(tabs.begin(), tabs.end(), boost::bind(&Modules::NxTabUpdater, m_tab, _1));
}

LRESULT CLinksView::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {

		// (j.jones 2014-08-08 10:39) - PLID 63232 - if the Links module is not active,
		// do nothing, because each tab properly updates itself when it becomes active again
		if (GetMainFrame() && !GetMainFrame()->IsActiveView(LINKS_MODULE_NAME)) {
			//the links module is not active, so don't bother updating the active tab
			return 0;
		}

		// (j.jones 2014-08-07 14:19) - PLID 63179 - send tablecheckers only to
		// the active tab, not to all tabs
		if (m_pActiveSheet) {
			return m_pActiveSheet->SendMessage(WM_TABLE_CHANGED, wParam, lParam);
		}

	} NxCatchAll(__FUNCTION__);

	return 0;
}

// (j.jones 2014-08-07 14:22) - PLID 63179 - added an Ex handler
LRESULT CLinksView::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {

		// (j.jones 2014-08-08 10:39) - PLID 63232 - if the Links module is not active,
		// do nothing, because each tab properly updates itself when it becomes active again
		if (GetMainFrame() && !GetMainFrame()->IsActiveView(LINKS_MODULE_NAME)) {
			//the links module is not active, so don't bother updating the active tab
			return 0;
		}

		if (m_pActiveSheet) {
			return m_pActiveSheet->SendMessage(WM_TABLE_CHANGED_EX, wParam, lParam);
		}

	} NxCatchAll(__FUNCTION__);

	return 0;
}

void CLinksView::OnUpdateFilePrint(CCmdUI* pCmdUI) 
{
	CNxDialog* ptrCurrentTab = GetActiveSheet();

	//Currently there is no printing in the links tab.  If you wish to enable it for a specific tab, just check here
	/*if (ptrCurrentTab == &MEMBER_TAB_HERE) {
		pCmdUI->Enable(TRUE);
	}
	else {*/
	pCmdUI->Enable(FALSE);
}

void CLinksView::OnUpdateFilePrintPreview(CCmdUI* pCmdUI) 
{
	CNxDialog* ptrCurrentTab = GetActiveSheet();
	
	//Currently there is no printing in the links tab.  If you wish to enable it for a specific tab, just check here
	/*if (ptrCurrentTab == &MEMBER_TAB_HERE) {
		pCmdUI->Enable(TRUE);
	}
	else {*/
	pCmdUI->Enable(FALSE);
}

int CLinksView::ShowPrefsDlg()
{
	// (j.jones 2008-04-23 14:44) - PLID 29597 - we have HL7 preferences now
	if (m_pActiveSheet == &m_HL7BatchDlg) {
		return ShowPreferencesDlg(GetRemoteData(), GetCurrentUserName(), GetRegistryBase(), piHL7);
	}
	// (j.jones 2010-10-26 09:27) - PLID 41068 - we have device preferences now
	else if (m_pActiveSheet == &m_dlgDeviceConfigTab) {
		return ShowPreferencesDlg(GetRemoteData(), GetCurrentUserName(), GetRegistryBase(), piDevices);
	}
	else {
		return ShowPreferencesDlg(GetRemoteData(), GetCurrentUserName(), GetRegistryBase(), piLinksModule);
	}
}

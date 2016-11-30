// AdminView.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "AdminView.h"
#include "pracprops.h"
#include "Barcode.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "PreferenceUtils.h"
#include "GlobalReportUtils.h"
#include "DontShowDlg.h"
#include "boost/bind.hpp"
#include "CPTCodes.h"
#include "Zipcodes.h"
#include "MainSurgeries.h"
#include "CptCodes.h"
#include "MainSurgeries.h"
#include "Zipcodes.h"
#include "HcfaSetup.h"
#include "UB92Setup.h"
#include "MultiFees.h"
#include "practiceinfo.h"
#include "Auditing.h"
#include "PhaseDlg.h"
#include "ProcedureDlg.h"
#include "EMRSetupDlg.h"
#include "SchedulerSetupDlg.h"
#include "CustomRecordSetupDlg.h"
#include "LabsSetupDlg.h"
#include "Retail.h"
#include "LinksSetupDlg.h"
#include "ListMergeDlg.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CAdminView

IMPLEMENT_DYNCREATE(CAdminView, CNxTabView)

// (j.jones 2013-05-07 12:21) - PLID 53969 - changed the child tabs to be declared by reference,
// which means their initialization is slightly different now
CAdminView::CAdminView()
	: m_NexSpaSheet(*(new CRetail(this)))
	, m_HCFASheet(*(new CHcfaSetup(this)))
	, m_UB92Sheet(*(new CUB92Setup(this)))
	, m_MultiFeeSheet(*(new CMultiFees(this)))
	, m_SurgerySheet(*(new CMainSurgeries(this)))
	, m_SchedulerSheet(*(new CSchedulerSetupDlg(this)))
	, m_ProcedureSheet(*(new CProcedureDlg(this)))
	, m_CustomRecordSheet(*(new CCustomRecordSetupDlg(this)))
	, m_EMRSheet(*(new CEMRSetupDlg(this)))
	, m_PhaseSheet(*(new CPhaseDlg(this)))
	, m_ZipCodeSheet(*(new CZipcodes(this)))
	, m_AuditingSheet(*(new CAuditing(this)))
	, m_LabsSheet(*(new CLabsSetupDlg(this)))
	, m_LinksSheet(*(new CLinksSetupDlg(this)))
	, m_BillingSheet(*(new CCPTCodes(this)))
	, m_LocationSheet(*(new CPracticeInfo(this)))
{
	m_bTriedFirstTab = false;
}
CAdminView::~CAdminView()
{
	try {

		// (j.jones 2013-05-07 12:21) - PLID 53969 - changed the child tabs to be declared by reference,
		// which means we have to clean up their pointers now
		// They are never null. They have to have been filled in the constructor.
		delete &m_NexSpaSheet;
		delete &m_HCFASheet;
		delete &m_UB92Sheet;
		delete &m_MultiFeeSheet;
		delete &m_SurgerySheet;
		delete &m_SchedulerSheet;
		delete &m_ProcedureSheet;
		delete &m_CustomRecordSheet;
		delete &m_EMRSheet;
		delete &m_PhaseSheet;
		delete &m_ZipCodeSheet;
		delete &m_AuditingSheet;
		delete &m_LabsSheet;
		delete &m_LinksSheet;
		delete &m_BillingSheet;
		delete &m_LocationSheet;

	}NxCatchAll(__FUNCTION__);
}

BEGIN_MESSAGE_MAP(CAdminView, CNxTabView)
	//{{AFX_MSG_MAP(CAdminView)
	ON_WM_CREATE()
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CAdminView::OnPrintPreview)
	ON_COMMAND(ID_FILE_PRINT, CAdminView::OnPrint)
	ON_UPDATE_COMMAND_UI(IDM_UPDATE_VIEW, CNxTabView::OnUpdateViewUI) // (z.manning 2010-07-19 16:06) - PLID 39222
	// (a.walling 2016-02-15 08:13) - PLID 67827 - Combine Locations
	ON_UPDATE_COMMAND_UI(ID_LOCATION_COMBINE, OnUpdateLocationCombine)
	ON_COMMAND(ID_LOCATION_COMBINE, CAdminView::OnLocationCombine)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//DRT 2/24/2004 - PLID 10854 - For now, only enable the print/preview buttons
//	if we are on the multi fee tab.
// (a.walling 2006-11-21 14:41) - PLID 3897 - Enable for any tab that may have a relevant report.
// (j.gruber 2007-02-20 10:06) - PLID 24440 - make the procedure tab print the cheat sheet information
void CAdminView::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
{
	if(GetActiveTab() == AdminModule::MultiFeeTab ||
		GetActiveTab() == AdminModule::SurgeryTab || 
		GetActiveTab() == AdminModule::BillingTab ||		// service codes
		GetActiveTab() == AdminModule::HCFATab ||		// insurance companies
		GetActiveTab() == AdminModule::UB92Tab ||		// insurance companies
		GetActiveTab() == AdminModule::Scheduler2Tab ||	// appointment types by purpose
		GetActiveTab() == AdminModule::PhaseTab ||
		GetActiveTab() == AdminModule::ProcedureTab)			// tracking ladders

		pCmdUI->Enable();
	else
		pCmdUI->Enable(FALSE);
}

// (j.gruber 2007-02-20 10:06) - PLID 24440 - make the procedure tab print the cheat sheet information
void CAdminView::OnUpdateFilePrint(CCmdUI* pCmdUI)
{
	if(GetActiveTab() == AdminModule::MultiFeeTab || 
		GetActiveTab() == AdminModule::SurgeryTab || 
		GetActiveTab() == AdminModule::BillingTab ||
		GetActiveTab() == AdminModule::HCFATab ||
		GetActiveTab() == AdminModule::UB92Tab ||
		GetActiveTab() == AdminModule::Scheduler2Tab ||
		GetActiveTab() == AdminModule::PhaseTab ||
		GetActiveTab() == AdminModule::ProcedureTab)

		pCmdUI->Enable();
	else
		pCmdUI->Enable(FALSE);
}

BOOL CAdminView::CheckPermissions()
{
	if (!UserPermission(AdministratorModuleItem)) {
		return FALSE;
	}
	return TRUE;
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CAdminView::Hotkey (int key)
{
	short newTab = -1;

	if(IsWindowEnabled()) {
		int nKey = IgnoreRightAlt() ? VK_LMENU : VK_MENU;
		if (GetAsyncKeyState(nKey) & 0xE000)
		{	switch (key) 
			{	case 'L':
					newTab = AdminModule::LocationsTab;
					break;
				case 'B':
					newTab = AdminModule::BillingTab;
					break;
				// (j.gruber 2007-03-19 11:19) - PLID 25165 - adding a discounts tab
				// (a.wetta 2007-03-29 10:48) - PLID 25407 - Changed to retail tab
				// (a.wetta 2007-05-16 08:56) - PLID 25960 - This is now the NexSpa tab
				case 'P':
					if(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)) {
						newTab = AdminModule::NexSpaTab;
					}
					else{
						newTab = m_tab->CurSel;
					}
					break;
				case 'C':
					newTab = AdminModule::HCFATab;
					break;	
				//TES 3/26/2007 - PLID 24993 - Changed accelerators, UB tab is now 'U', Surgery tab is now 'G'
				case 'U':
					newTab = AdminModule::UB92Tab;
					break;	
				case 'G':
					newTab = AdminModule::SurgeryTab;
					break;
				case 'D':
					newTab = AdminModule::MultiFeeTab;
					break;
				case 'S':
					//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
					if(g_pLicense && g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent)) {
						newTab = AdminModule::Scheduler2Tab;
					}
					else{
						newTab = m_tab->CurSel;
					}
					break;
				case 'Z':
					newTab = AdminModule::ZipCodeTab;
					break;
				case 'A':
					newTab = AdminModule::AuditingTab;
					break;
				case 'K':
					newTab = AdminModule::PhaseTab;
					break;
				case 'R':
					newTab = AdminModule::ProcedureTab;
					break;
				case 'E':
					if(g_pLicense && g_pLicense->HasEMR(CLicense::cflrUse) == 1) {
						newTab = AdminModule::CustomRecordsTab;
					}
					else if(g_pLicense && g_pLicense->HasEMR(CLicense::cflrUse) == 2) {
						newTab = AdminModule::EMRTab;
					}
					else {
						newTab = m_tab->CurSel;
					}
					break;
				//(e.lally 2010-10-25) PLID 40994 - Add Links tab
				case 'I':
					newTab = AdminModule::LinksTab;
					break;
			}

			if (newTab != -1 && m_tab->ShowTab[newTab]) {
					SetActiveTab(newTab);
					return 0;
			}
		}
		else {
			if(key == VK_F5) {
				if (GetActiveTab() == AdminModule::AuditingTab) {
					m_AuditingSheet.PostMessage(NXM_AUDIT_REFRESH,NULL,NULL);
					return 0;
				}
			}
		}
	}

	return CNxTabView::Hotkey(key);
}

void CAdminView::ShowTabs()
{
	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to update the tabs
	Modules::Tabs& tabs = g_Modules[Modules::Admin]->GetTabs();
	std::for_each(tabs.begin(), tabs.end(), boost::bind(&Modules::NxTabUpdater, m_tab, _1));
}

int CAdminView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxTabView::OnCreate(lpCreateStruct) == -1)
		return -1;

	Modules::Tabs& tabs = g_Modules[Modules::Admin]->Reset().GetTabs();

	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to create the tabs

	CreateSheet(&m_LocationSheet,		tabs, AdminModule::LocationsTab);

	CreateSheet(&m_BillingSheet,		tabs, AdminModule::BillingTab);
	// (j.gruber 2007-03-19 11:20) - PLID 25165 - adding discounts setup
	// (a.wetta 2007-03-29 10:50) - PLID 25407 - Changed to retail tab
	// (a.wetta 2007-05-16 08:59) - PLID 25960 - Change to NexSpa tab
	CreateSheet(&m_NexSpaSheet,			tabs, AdminModule::NexSpaTab);
	CreateSheet(&m_HCFASheet,			tabs, AdminModule::HCFATab);
	//TES 3/12/2007 - PLID 24993 - Renamed from "UB92" to "UB".  This also involved changing the accelerator key from 9 to U,
	// which in turn meant changing the Surgery tab's accelerator from u to g.
	CreateSheet(&m_UB92Sheet,			tabs, AdminModule::UB92Tab);
	CreateSheet(&m_MultiFeeSheet,		tabs, AdminModule::MultiFeeTab);
	CreateSheet(&m_SurgerySheet,		tabs, AdminModule::SurgeryTab);
	CreateSheet(&m_SchedulerSheet,		tabs, AdminModule::Scheduler2Tab);
	CreateSheet(&m_ProcedureSheet,		tabs, AdminModule::ProcedureTab);
	CreateSheet(&m_CustomRecordSheet,	tabs, AdminModule::CustomRecordsTab);
	CreateSheet(&m_EMRSheet,			tabs, AdminModule::EMRTab);
	CreateSheet(&m_PhaseSheet,			tabs, AdminModule::PhaseTab);
	CreateSheet(&m_LabsSheet,			tabs, AdminModule::LabsTab);
	//(e.lally 2010-10-25) PLID 40994 - Add Links tab
	CreateSheet(&m_LinksSheet,			tabs, AdminModule::LinksTab);
	CreateSheet(&m_ZipCodeSheet,		tabs, AdminModule::ZipCodeTab);
	CreateSheet(&m_AuditingSheet,		tabs, AdminModule::AuditingTab);

	//DRT 4/18/2008 - PLID 29711 - Set the text color of the tabs.
	// (j.jones 2016-04-18 17:46) - NX-100214 - the NxTab control now defaults
	// to the value of CNexTechDialog::GetDefaultNxTabTextColor()

	ShowTabs();

	// (j.gruber 2007-03-19 11:20) - PLID 25165 - adding discounts setup
	m_tab->TabWidth = 15;

	return 0;
}

void CAdminView::OnSelectTab(short newTab, short oldTab)
{

	//TES 10/29/2003: The algorithm represented below is: if this tab is selected because we're
	//initializing the view (oldTab == newTab), then silently check whether we have full permissions
	//to this tab.  If we do not, then, starting with the first tab, loop through until we find one 
	//that we have full access to.  This code has changed because the module can now start with 
	//any tab, not just the Locations tab, based on preferences.
	switch (newTab)
	{
		case AdminModule::LocationsTab:
			if(oldTab == AdminModule::LocationsTab){
				//We are now trying the first tab, so make sure nobody loops back to us.
				m_bTriedFirstTab = true;
				if(!(GetCurrentUserPermissions(bioAdminLocations) & SPT__R________)) {
					OnSelectTab(AdminModule::BillingTab, AdminModule::BillingTab);
					return;
				}
				else{
					m_tab->CurSel = AdminModule::LocationsTab;
				}
			}
			if (!CheckCurrentUserPermissions(bioAdminLocations, sptRead))
			{	SetActiveTab(oldTab);
				return;
			}
			break;
		case AdminModule::BillingTab:
			if(oldTab == AdminModule::BillingTab){
				if(!(GetCurrentUserPermissions(bioAdminBilling) & SPT__R________)){
					if(!m_bTriedFirstTab) {//Let's try it.
						OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					}
					else {
						OnSelectTab(AdminModule::NexSpaTab, AdminModule::NexSpaTab);					
					}
					return;
				}
				else{
					m_tab->CurSel = AdminModule::BillingTab;
				}
			}
			if (!CheckCurrentUserPermissions(bioAdminBilling, sptRead))
			{	SetActiveTab(oldTab);
				return;
			}
			break;
		// (a.wetta 2007-05-17 13:45) - PLID 25394 - Handle the NexSpa tab
		case AdminModule::NexSpaTab:
			// (a.wetta 2007-05-24 11:28) - PLID 25960 - Check the license and use the usage counts
			if (!IsSpa(TRUE)) {
				// They don't have a license or don't want to use a usage count
				if(oldTab == AdminModule::NexSpaTab) {
					// (z.manning, 12/05/2007) - PLID 28281 - Dont't try to select the locations tab if we've
					// done so already.
					if(!m_bTriedFirstTab) {
						oldTab = AdminModule::LocationsTab;
					}
					else {
						oldTab = AdminModule::HCFATab;
					}
				}
				OnSelectTab(oldTab, oldTab);
				m_tab->CurSel = oldTab;
				return;
			}

			if(oldTab == AdminModule::NexSpaTab){
				if(!(GetCurrentUserPermissions(bioAdminNexSpa) & SPT__R________)){
					if(!m_bTriedFirstTab) {//Let's try it.
						OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					}
					else {
						OnSelectTab(AdminModule::HCFATab, AdminModule::HCFATab);					
					}
					return;
				}
				else{
					m_tab->CurSel = AdminModule::NexSpaTab;
				}
			}
			
			if (!CheckCurrentUserPermissions(bioAdminNexSpa, sptRead))
			{	
				SetActiveTab(oldTab);
				return;
			}
			break;
		case AdminModule::HCFATab:
			if(oldTab == AdminModule::HCFATab){
				if(!(GetCurrentUserPermissions(bioAdminHCFA) & SPT__R________)){
					if(!m_bTriedFirstTab) {//Let's try it.
						OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					}
					else {
						OnSelectTab(AdminModule::SurgeryTab, AdminModule::SurgeryTab);  //Skip UB92 tab, it uses same permissions
					}
					return;
				}
				else{
					m_tab->CurSel = AdminModule::HCFATab;
				}
			}
			if (!CheckCurrentUserPermissions(bioAdminHCFA, sptRead))
			{	SetActiveTab(oldTab);
				return;
			}
			break;
		case AdminModule::UB92Tab: //Uses same permissions as the HCFA (for the moment).
			if(oldTab == AdminModule::UB92Tab){
				if(!(GetCurrentUserPermissions(bioAdminHCFA) & SPT__R________)){
					if(!m_bTriedFirstTab) {//Let's try it.
						OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					}
					else {
						OnSelectTab(AdminModule::SurgeryTab, AdminModule::SurgeryTab);
					}
					return;
				}
			}
			if(!CheckCurrentUserPermissions(bioAdminHCFA, sptRead))
			{
				SetActiveTab(oldTab);
				return;
			}
			break;

		case AdminModule::SurgeryTab:
			if(oldTab == AdminModule::SurgeryTab){
				if(!(GetCurrentUserPermissions(bioAdminSurgery) & SPT__R________)){
					if(!m_bTriedFirstTab) {//Let's try it.
						OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					}
					else {
						OnSelectTab(AdminModule::MultiFeeTab, AdminModule::MultiFeeTab);
					}
					return;
				}
				else{
					m_tab->CurSel = AdminModule::SurgeryTab;
				}
			}
			if (!CheckCurrentUserPermissions(bioAdminSurgery, sptRead))
			{	SetActiveTab(oldTab);
				return;
			}
			break;
		case AdminModule::MultiFeeTab:
			if(oldTab == AdminModule::MultiFeeTab){
				if(!(GetCurrentUserPermissions(bioAdminMultiFee) & SPT__R________)){
					if(!m_bTriedFirstTab) {//Let's try it.
						OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					}
					else {
						OnSelectTab(AdminModule::Scheduler2Tab, AdminModule::Scheduler2Tab);
					}
					return;
				}
				else{
					m_tab->CurSel = AdminModule::MultiFeeTab;
				}
			}
			if (!CheckCurrentUserPermissions(bioAdminMultiFee, sptRead))
			{	SetActiveTab(oldTab);
				return;
			}
			break;
		case AdminModule::Scheduler2Tab:
			if(oldTab == AdminModule::Scheduler2Tab){
				if(!(GetCurrentUserPermissions(bioAdminScheduler) & SPT__R________)){
					if(!m_bTriedFirstTab) {//Let's try it.
						OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					}
					else {
						OnSelectTab(AdminModule::ProcedureTab, AdminModule::ProcedureTab);
						//this will definitely work, so...
						m_tab->CurSel = AdminModule::ProcedureTab;
					}
					return;
				}
				else{
					m_tab->CurSel = AdminModule::Scheduler2Tab;
				}
			}
			if (!CheckCurrentUserPermissions(bioAdminScheduler, sptRead))
			{	SetActiveTab(oldTab);
				return;
			}
			break;
		
		case AdminModule::CustomRecordsTab:
			if(oldTab == AdminModule::CustomRecordsTab) {
				if(!(GetCurrentUserPermissions(bioAdminEMR) & SPT__R________)){
					if(!m_bTriedFirstTab) {//Let's try it.
						OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					}
					else {
						OnSelectTab(AdminModule::PhaseTab, AdminModule::PhaseTab);
					}
					return;
				}
				else {
					//PLID 21449: see if the tab is visible
					if (m_tab->ShowTab[AdminModule::CustomRecordsTab] == FALSE) {
						//take 'em back
						if(!m_bTriedFirstTab) {//Let's try it.
							OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
						}
						else {
							OnSelectTab(AdminModule::PhaseTab, AdminModule::PhaseTab);
						}
						return;
					}
					else {
						m_tab->CurSel = AdminModule::CustomRecordsTab;
					}
				}
			}
			if(!CheckCurrentUserPermissions(bioAdminEMR, sptRead))
			{	SetActiveTab(oldTab);
				return;
			}
			break;

		case AdminModule::EMRTab:
			{
				//DRT 1/20/2005 - PLID 15340 - EMR usage counts must be checked!
				long nEMRUsage = g_pLicense->HasEMR(CLicense::cflrUse);

				if(nEMRUsage == 2) {
					//EMR L2 (NexEMR) is active and we can use it (said Yes to trial or has full functionality)
				}
				else {
					//If the old tab is also the emr tab (this can happen if you have NexEMR set as your default tab and first load the module)
					//	we'll have to revert them to General1
					if(oldTab == AdminModule::EMRTab)
						oldTab = AdminModule::LocationsTab;

					//0 = Not allowed or they said "No" when asked if they wanted to use a trial.  Since we already made it into this
					//	tab, it's most likely the latter.  We need to move them back out of this tab.
					//1 = Has EMR L1 (Custom Records) License.  They shouldn't be here, they should be in the case above for
					//	'AdminModule::CustomRecordsTab'.
					//In either instance, we refuse access and send 'em packing.
					OnSelectTab(oldTab, oldTab);
					m_tab->CurSel = oldTab;
					return;
				}

				if(oldTab == AdminModule::EMRTab) {
					if(!(GetCurrentUserPermissions(bioAdminEMR) & SPT__R________)){
						if(!m_bTriedFirstTab) {//Let's try it.
							OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
						}
						else {
							OnSelectTab(AdminModule::PhaseTab, AdminModule::PhaseTab);
						}
						return;
					}
					else {
						m_tab->CurSel = AdminModule::EMRTab;
					}
				}
				if (!CheckCurrentUserPermissions(bioAdminEMR, sptRead))
				{	SetActiveTab(oldTab);
					return;
				}
			}
			break;

		case AdminModule::PhaseTab:
			if(oldTab == AdminModule::PhaseTab){
				if(!(GetCurrentUserPermissions(bioAdminTracking) & SPT__R________)){
					//(z.manning, PLID 16666, 07/05/05)
					//As of 07/05/05, the procedures tab did not have permissions
					OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					return;
				}
				
				//PLID 21449: see if the tab is visible
				if (m_tab->ShowTab[AdminModule::PhaseTab] == FALSE) {
					//esnd 'em packing
					OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					return;
				}
			}
			if(!CheckCurrentUserPermissions(bioAdminTracking, sptRead))
			{
				SetActiveTab(oldTab);
				return;
			}
			break;

		case AdminModule::ZipCodeTab:
			if(oldTab == AdminModule::ZipCodeTab){
				if(!(GetCurrentUserPermissions(bioAdminZipCode) & SPT__R________)){
					//We can't have gotten here unless this was the default tab, so...
					OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					return;
				}
			}
			if(!CheckCurrentUserPermissions(bioAdminZipCode, sptRead))
			{
				SetActiveTab(oldTab);
				return;
			}
			break;

		case AdminModule::AuditingTab:
			if(oldTab == AdminModule::AuditingTab){
				if(!(GetCurrentUserPermissions(bioAdminAuditTrail) & SPT__R________)){
					//We can't have gotten here unless this was the default tab, so...
					OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					return;
				}
			}
			if(!CheckCurrentUserPermissions(bioAdminAuditTrail, sptRead))
			{
				SetActiveTab(oldTab);
				return;
			}
			break;

		// (m.hancock 2006-07-05 11:16) - PLID 21187 - Administrator section for labs
		//JMM - licensing is already done by showing the tab or not, so that shouldn't matter here
		case AdminModule::LabsTab:
			if(oldTab == AdminModule::LabsTab){
				//PLID 21449: see if the labs tab is visible
				if (m_tab->ShowTab[AdminModule::LabsTab] == FALSE) {
					//go to the locations tab
					OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					return;
				}
				else {
					if(!(GetCurrentUserPermissions(bioAdminLabs) & SPT__R________)){
						//We can't have gotten here unless this was the default tab, so...
						OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
						return;
					}
				}
			}
			if(!CheckCurrentUserPermissions(bioAdminLabs, sptRead))
			{
				SetActiveTab(oldTab);
				return;
			}
			break;

		//(e.lally 2010-10-25) PLID 40994 - Add Links tab
		case AdminModule::LinksTab:
			if(oldTab == AdminModule::LinksTab){
				//see if the Links Tab is visible
				if (m_tab->ShowTab[AdminModule::LinksTab] == FALSE) {
					//go to the locations tab
					OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
					return;
				}
				else {
					if(!(GetCurrentUserPermissions(bioAdminLinks) & sptRead)){
						//We can't have gotten here unless this was the default tab, so...
						OnSelectTab(AdminModule::LocationsTab, AdminModule::LocationsTab);
						return;
					}
				}
			}
			if(!CheckCurrentUserPermissions(bioAdminLinks, sptRead))
			{
				SetActiveTab(oldTab);
				return;
			}
			break;

	}
	CNxTabView::OnSelectTab(newTab, oldTab);
}

void CAdminView::GoToBilling()
{
//	if(!CheckAccess("Administrator", "Enter Module"))
//DRT 10/18/02 - We don't need to check this permission here, because it's always checked again in the next line (setting the tab to billing)
//	if (!CheckCurrentUserPermissions(bioAdminBilling, sptRead))
//		return;

	SetActiveTab(AdminModule::BillingTab);
}

//DRT 11/30/2007 - PLID 28252 - Reworked barcode architecture.  This is handled by the base class.

BOOL CAdminView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	return CNxTabView::OnPreparePrinting(pInfo);
}

void CAdminView::OnPrintPreview()
{
	PrintTab(true);
}

void CAdminView::OnPrint()
{
	PrintTab(false);
}

BOOL CAdminView::PrintTab(bool bPreview, CPrintInfo *pInfo/* = 0*/)
{
	switch(GetActiveTab()) {
	case AdminModule::MultiFeeTab:
		{
			try {
				CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(481)]);
				NXDATALISTLib::_DNxDataListPtr pList = m_MultiFeeSheet.m_pFeeGroups;
				if(pList == NULL)
					return FALSE;

				long nCurSel = pList->GetCurSel();
				if(nCurSel == -1) {
					// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
					MsgBox("You must be viewing a fee schedule before you can preview this report.");
					return FALSE;
				}

				infReport.nExtraID = VarLong(pList->GetValue(nCurSel, 0));
				//Made new function for running reports - JMM 5-28-04
				// (d.lange 2015-10-14 11:03) - PLID 67117 - Changed from Multi Fee Preview to New Fee Preview
				RunReport(&infReport, bPreview, (CWnd *)this, "New Fee Preview", pInfo);
				
				return FALSE;

			} NxCatchAll("Error in CAdminView::PrintTab() - Fee Sched.");

		}
		break;

	case AdminModule::SurgeryTab:
		{
			try {
				//run a different report depending on if they have ASC or not
				// (j.jones 2009-08-26 17:31) - PLID 35271 - obsolete, Surgeries are now ONLY surgeries
				/*
				if(IsSurgeryCenter(FALSE)) {

					CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(556)]);
					
					if(m_SurgerySheet.m_pSurgeryNames->GetCurSel() != -1) {
						infReport.nExtraID = VarLong(m_SurgerySheet.m_pSurgeryNames->GetValue(m_SurgerySheet.m_pSurgeryNames->GetCurSel(),0));
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
								infReport.nDefaultCustomReport = nDefaultCustomReport;
								strFileName =  AdoFldString(rsFileName, "FileName");
							}
						}
						else {
							//if this occurs it just means they want the default, which in this case, there is only one
							strFileName = "SurgeryASCList";
							
						}
					}

					//Made new function for running reports - JMM 5-28-04
					RunReport(&infReport, bPreview, (CWnd *)this, "Preference Cards", pInfo);

					return FALSE;
				}
				else {*/
					//they do NOT have the surgery center module so run the regular surgery report

					CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(1)]);
					
					if(m_SurgerySheet.m_pSurgeryNames->GetCurSel() != -1) {
						infReport.nExtraID = VarLong(m_SurgerySheet.m_pSurgeryNames->GetValue(m_SurgerySheet.m_pSurgeryNames->GetCurSel(),0));
					}

					//check to see if there is a default report
					_RecordsetPtr rsDefault = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = 1");
					CString strFileName;

					if (rsDefault->eof) {

						strFileName = "SurgPriceListR";
					}
					else {
						
						long nDefaultCustomReport = AdoFldLong(rsDefault, "CustomReportID", -1);

						if (nDefaultCustomReport > 0) {

							_RecordsetPtr rsFileName = CreateRecordset("SELECT FileName FROM CustomReportsT WHERE ID = 1 AND Number = %li", nDefaultCustomReport);

							if (rsFileName->eof) {

								//this should never happen
								MessageBox("Practice could not find the custom report.  Please contact NexTech for assistance");
							}
							else {
								
								//set the default
								infReport.nDefaultCustomReport = nDefaultCustomReport;
								strFileName =  AdoFldString(rsFileName, "FileName");
							}
						}
						else {
							//if this occurs it just means they want the default, which in this case, there is only one
							strFileName = "SurgPriceListR";
							
						}
					}

					//Made new function for running reports - JMM 5-28-04
					RunReport(&infReport, bPreview, (CWnd *)this, "Surgery List and Prices", pInfo);

					return FALSE;
				//}

			} NxCatchAll("Error in CAdminView::PrintTab() - Surgery");

		}
		break;

	case AdminModule::BillingTab:
		{
			// (a.walling 2006-11-21 15:21) - PLID 3897 - Run the service code report
			try {
				CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(11)]);

				RunReport(&infReport, bPreview, (CWnd *)this, "Service Codes Preview", pInfo);
				
				return FALSE;

			} NxCatchAll("Error in CAdminView::PrintTab() - Fee Sched.");

		}
		break;

	case AdminModule::HCFATab:
	case AdminModule::UB92Tab:
		{
			// (a.walling 2006-11-21 15:21) - PLID 3897 - Run the insurance company report
			try {
				CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(57)]);

				RunReport(&infReport, bPreview, (CWnd *)this, "Insurance Companies Preview", pInfo);
				
				return FALSE;

			} NxCatchAll("Error in CAdminView::PrintTab() - Fee Sched.");

		}
		break;

	case AdminModule::Scheduler2Tab:
		{
			// (a.walling 2006-11-21 15:21) - PLID 3897 - Run the Appointment Types by Purpose report
			try {
				CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(279)]);

				RunReport(&infReport, bPreview, (CWnd *)this, "Appointment Types by Purpose Preview", pInfo);
				
				return FALSE;

			} NxCatchAll("Error in CAdminView::PrintTab() - Fee Sched.");

		}
		break;


	case AdminModule::PhaseTab:
		{
			// (a.walling 2006-11-21 15:21) - PLID 3897 - Run the ladders report
			try {
				CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(392)]);

				NXDATALISTLib::_DNxDataListPtr pList = m_PhaseSheet.m_ladder;

				if (pList) {
					long LadderID;
					if (m_PhaseSheet.GetLadder(LadderID)) // returns value in LadderID
					{
						infReport.AddExtraValue(AsString(LadderID));
						RunReport(&infReport, bPreview, (CWnd *)this, "Tracking Ladders Preview", pInfo);
					}					
				}
				
				return FALSE;

			} NxCatchAll("Error in CAdminView::PrintTab() - Fee Sched.");

		}
		break;
	// (j.gruber 2007-02-20 10:06) - PLID 24440 - make the procedure tab print the cheat sheet information
	case AdminModule::ProcedureTab: 
		try {

			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(569)]);

				if (m_ProcedureSheet) {
					long nProcID;
					if (m_ProcedureSheet.GetProcedureID(nProcID)) // returns value in ProcedureID
					{
						CString strFilter;
						strFilter.Format(" ID IN (%li) ", nProcID);
						infReport.strExternalFilter = strFilter;					

						DontShowMeAgain(this, "To see a list of all Procedure Cheat Sheet Information, please run the report from the Administration Tab in the Reports Module.", GetCurrentUserName(), "Practice", TRUE, FALSE);
						RunReport(&infReport, bPreview, (CWnd *)this, "Procedure Cheat Sheet Information", pInfo);
					}					
				}
				
				return FALSE;

			} NxCatchAll("Error in CAdminView::PrintTab() - Procedure Tab.");




	default:
		break;
	}

	return true;
}

int CAdminView::ShowPrefsDlg()
{
	if (m_pActiveSheet == &m_BillingSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piCptCodes);
	else if (m_pActiveSheet == &m_SurgerySheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(),GetRegistryBase(), piSurgeries);
	else if (m_pActiveSheet == &m_PhaseSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(),GetRegistryBase(), piTracking);
	else if (m_pActiveSheet == &m_AuditingSheet){
		// (e.lally 2009-06-02) PLID 34396 - Go to the auditing section
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(),GetRegistryBase(), piAuditing);
	} else if (m_pActiveSheet == &m_ZipCodeSheet) {
		// (d.lange 2010-08-02 15:28) - PLID 38249 - Go to the zip code preferences
		return ShowPreferencesDlg(GetRemoteData(), GetCurrentUserName(), GetRegistryBase(), piZipCodes);
	}
	else if (m_pActiveSheet == &m_LinksSheet) {
		//(e.lally 2010-11-29) PLID 40994 - Go to Links preferences
		return ShowPreferencesDlg(GetRemoteData(), GetCurrentUserName(), GetRegistryBase(), piLinksModule);
	}
	else
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(),GetRegistryBase(), piAdminModule);
}

void CAdminView::OnInitialUpdate()
{
	CNxTabView::OnInitialUpdate();

	//Don't show the Import EMR Content item if they're not licensed for the EMR L2
	if(g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
		RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Tools", ID_IMPORT_EMR_CONTENT, FALSE);
	}

	// (z.manning, 07/17/2007) - PLID 18359 - Remove the import NexForms menu option if they don't have a
	// NexForms license.
	if(!g_pLicense->CheckForLicense(CLicense::lcNexForms, CLicense::cflrSilent)) {
		RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Tools", ID_TOOLS_IMPORT_NEXFORMS_CONTENT, FALSE);
	}
}


LRESULT CAdminView::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {

		// (j.jones 2014-08-08 09:50) - PLID 63232 - if the Admin module is not active,
		// do nothing, because each tab properly updates itself when it becomes active again
		if (GetMainFrame() && !GetMainFrame()->IsActiveView(ADMIN_MODULE_NAME)) {
			//the administrator module is not active, so don't bother updating the active tab
			return 0;
		}

		if (m_pActiveSheet == &m_LocationSheet)
			m_LocationSheet.OnTableChanged(wParam, lParam);
		if (m_pActiveSheet == &m_BillingSheet)
			m_BillingSheet.OnTableChanged(wParam, lParam);
		if(m_pActiveSheet == &m_HCFASheet)
			m_HCFASheet.OnTableChanged(wParam, lParam);
		if (m_pActiveSheet == &m_UB92Sheet) 
			m_UB92Sheet.OnTableChanged(wParam, lParam);
		if (m_pActiveSheet == &m_MultiFeeSheet) 
			m_MultiFeeSheet.OnTableChanged(wParam, lParam);
		if (m_pActiveSheet == &m_SurgerySheet)
			m_SurgerySheet.OnTableChanged(wParam, lParam);
		if (m_pActiveSheet == &m_SchedulerSheet)
			m_SchedulerSheet.OnTableChanged(wParam, lParam);
		if(m_pActiveSheet == &m_ProcedureSheet)
			m_ProcedureSheet.OnTableChanged(wParam, lParam);
		if (m_pActiveSheet == &m_CustomRecordSheet) 
			m_CustomRecordSheet.OnTableChanged(wParam, lParam);
		if (m_pActiveSheet == &m_EMRSheet) 
			m_EMRSheet.OnTableChanged(wParam, lParam);
		if (m_pActiveSheet == &m_PhaseSheet) 
			m_PhaseSheet.OnTableChanged(wParam, lParam);
		//(e.lally 2010-10-25) PLID 40994 - Add Links tab
		if(m_pActiveSheet == &m_LinksSheet)
			m_LinksSheet.OnTableChanged(wParam, lParam);
		if (m_pActiveSheet == &m_ZipCodeSheet) 
			m_ZipCodeSheet.OnTableChanged(wParam, lParam);
		if (m_pActiveSheet == &m_AuditingSheet) 
			m_AuditingSheet.OnTableChanged(wParam, lParam);

	} NxCatchAll(__FUNCTION__);

	return 0;
}


// (a.walling 2016-02-15 08:13) - PLID 67827 - Combine Locations
void CAdminView::OnUpdateLocationCombine(CCmdUI * pCmdUI)
{
	try {
		// (a.walling 2016-02-15 13:03) - PLID 68261 - New permissions for Location merge and Provider merge
		// apparently we just always enable this and show a permission warning when actually clicked.
		pCmdUI->Enable();
	} NxCatchAllIgnore()
}

void CAdminView::OnLocationCombine()
{
	try {
		if (!CheckCurrentUserPermissions(bioMergeLocations, sptDynamic0)) {
			return;
		}

		CListMergeDlg dlg(this);
		dlg.m_eListType = mltLocations;
		dlg.DoModal();
		UpdateView();
	} NxCatchAll(__FUNCTION__);
}

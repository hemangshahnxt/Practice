#include "stdafx.h"
#include "Modules.h"
#include <algorithm>
#include "boost/bind.hpp"
#include "boost/function.hpp"
#include "GlobalFinancialUtils.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - The Modules namespace and other code is used to keep track of the various modules and their tabs and optional common actions
// This keeps things in a single, unified place, since tabs are not part of the menu system. This also allows us the flexibility to create and handle menus dynamically.

using std::for_each;
using boost::bind;

Modules::ModuleInfo g_Modules;

namespace Modules
{

LPCTSTR GetModuleName(Type module)
{
	switch (module) {
		case Scheduler:
			return "Scheduler";
			break;
		case Patients:
			return "Patients";
			break;
		case Admin:
			return "Administrator";
			break;
		case Inventory:
			return "Inventory";
			break;
		case Financial:
			return "Financial";
			break;
		case Marketing:
			return "Marketing";
			break;
		case Reports:
			return "Reports";
			break;
		case Contacts:
			return "Contacts";
			break;
		case LetterWriting:
			return "Letter Writing";
			break;
		case SurgeryCenter:
			return "Surgery Center";
			break;
		case Links:
			return "Links";
			break;
		default:
			ASSERT(FALSE);
			return "";
			break;
	}
}



void ModuleInfo::Initialize()
{
	modules[Modules::Scheduler]		= PracticeModulePtr(new SchedulerModule());
	modules[Modules::Patients]		= PracticeModulePtr(new PatientsModule());
	modules[Modules::Admin]			= PracticeModulePtr(new AdminModule());
	modules[Modules::Inventory]		= PracticeModulePtr(new InventoryModule());
	modules[Modules::Financial]		= PracticeModulePtr(new FinancialModule());
	modules[Modules::Marketing]		= PracticeModulePtr(new MarketingModule());
	modules[Modules::Reports]		= PracticeModulePtr(new ReportsModule());
	modules[Modules::Contacts]		= PracticeModulePtr(new ContactsModule());
	modules[Modules::LetterWriting]	= PracticeModulePtr(new LetterWritingModule());
	modules[Modules::SurgeryCenter]	= PracticeModulePtr(new SurgeryCenterModule());
	modules[Modules::Links]			= PracticeModulePtr(new LinksModule());
}

void ModuleInfo::ResetAll()
{
	for_each(modules.begin(), modules.end(), 
		bind(&PracticeModule::Reset,
			bind(&PracticeModuleMap::value_type::second, _1)
		)
	);
}

void ModuleInfo::LoadAll()
{
	for_each(modules.begin(), modules.end(), 
		bind(&PracticeModule::Load,
			bind(&PracticeModuleMap::value_type::second, _1)
		)
	);
}

PracticeModulePtr ModuleInfo::GetModulePtr(Modules::Type moduleType)
{
	PracticeModulePtr pModule(modules[moduleType]);
	_ASSERTE(pModule);
	if (!pModule) {
		ThrowNxException("Invalid module!");
	}

	pModule->Load();

	return pModule;
}

PracticeModulePtr ModuleInfo::GetModulePtrByName(LPCTSTR szModuleName)
{
	// (a.walling 2010-12-06 12:27) - PLID 40444 - This was incorrectly using find_if agains selecting the result of the Name function,
	// hence returning the first non-NULL LPCTSTR returned from PracticeModule::Name. We should have been comparing the string!
	PracticeModuleMap::iterator it = std::find_if(modules.begin(), modules.end(), 
		bind(strcmp, 
			bind(&PracticeModule::Name,
				bind(&PracticeModuleMap::value_type::second, _1)
			),
			szModuleName
		) == 0
	);

	if (it == modules.end()) {
		ThrowNxException("Invalid module name %s", szModuleName);
	}

	return it->second;
}


class PopupMenuHelperImpl : public PopupMenuHelper
{
public:
	typedef boost::function<void()> ActivateElementFn;
	typedef std::map<long,ActivateElementFn> ActivateElementActionMap;

	PopupMenuHelperImpl(CMenu& menu, long& nMenuID, long& nCmdID)
		: PopupMenuHelper(menu, nMenuID, nCmdID)
	{
	};

	static void ActivateMenuFn(WORD nMenuID) {
		CMainFrame* pMainFrame = GetMainFrame();
		ASSERT(pMainFrame && pMainFrame->GetSafeHwnd());
		::PostMessage(pMainFrame->GetSafeHwnd(), WM_COMMAND, MAKEWPARAM(nMenuID, 0), 0);
	};

	virtual void AppendAllPopupMenus() {
		ASSERT(m_menu.GetSafeHmenu());

		PracticeModuleMap modules(g_Modules.GetModules());

		for_each(modules.begin(), modules.end(), 
			bind(&PopupMenuHelper::AppendModulePopupMenu,
				this, 
				bind(&PracticeModuleMap::value_type::second, _1)
			)
		);
	};

	virtual void AppendModulePopupMenu(PracticeModulePtr module) {
		CMenu submenu;

		CreateModulePopupMenu(submenu, module);

		if (!submenu.GetSafeHmenu()) {
			return;
		}

		bool bEnabled = module->Enabled() && (GetMainFrame() ? GetMainFrame()->IsWindowEnabled() ? true : false : false);
		DWORD dwFlags = MF_BYPOSITION | MF_POPUP;

		if (!bEnabled) {
			dwFlags |= MF_DISABLED | MF_GRAYED;
		}

		m_nNextMenuID++;
		m_menu.InsertMenu(m_nNextMenuID, dwFlags, (UINT_PTR)submenu.Detach(), module->Name());
	};

	virtual void CreateModulePopupMenu(CMenu& submenu, PracticeModulePtr module) {
		module->CreatePopupMenu(submenu, this);
	};

	virtual bool HandleCommand(long nCmdID) {
		ActivateElementFn action = activateElementActionMap[nCmdID];
		if (action) {
			CMainFrame* pMainFrame = GetMainFrame();
			if (pMainFrame) {
				WINDOWPLACEMENT windowPlacement;
				windowPlacement.length = sizeof(windowPlacement);
				if (pMainFrame->GetWindowPlacement(&windowPlacement) && windowPlacement.showCmd == SW_SHOWMINIMIZED) {
					pMainFrame->ShowWindow(SW_RESTORE);
				}

				pMainFrame->BringWindowToTop();
				pMainFrame->SetForegroundWindow();
			}
			action();
			return true;
		} else {
			return false;
		}
	};

	ActivateElementActionMap activateElementActionMap;
};

PopupMenuHelperPtr PopupMenuHelper::Create(CMenu& menu, long& nMenuID, long& nCmdID)
{
	return PopupMenuHelperPtr(new PopupMenuHelperImpl(menu, nMenuID, nCmdID));
}

void NxTabUpdater(NxTab::_DNxTabPtr pTabs, Tabs::value_type value)
{
	pTabs->ShowTab[value.first] = value.second->Visible() ? VARIANT_TRUE : VARIANT_FALSE;
}

}; // end namespace Modules



//////////////////////////////////////////




PracticeModule::PracticeModule(Modules::Type _type)
	: Modules::Element(Modules::GetModuleName(_type), "", 0)
	, loaded(false)
	, type(_type)
	, overrideDefaultTab(-1)
{
}

void PracticeModule::Activate()
{
	CMainFrame* pMainFrame = GetMainFrame();
	if (!pMainFrame) {
		return;
	}

	if (!pMainFrame->FlipToModule(Name())) {
		return;
	}
}

void PracticeModule::ActivateTab(int tab)
{
	CMainFrame* pMainFrame = GetMainFrame();
	if (!pMainFrame) {
		return;
	}

	try {
		// (a.walling 2010-12-09 09:25) - PLID 40444 - Set the override tab so the module opens directly to the desired tab,
		// ensuring it is reset even if an exception is thrown.
		overrideDefaultTab = (short)tab;

		if (pMainFrame->FlipToModule(Name())) {
			CNxView* pView = pMainFrame->GetOpenView(Name());
			if (pView->GetActiveTab() != tab) {
				pView->SetActiveTab(tab);
			}
		}
	} NxCatchAllCallThrow("Error activating module tab", {overrideDefaultTab = -1;});

	overrideDefaultTab = -1;
}

short PracticeModule::ResolveDefaultTab(short tab)
{
	return tab;
}

PracticeModule& PracticeModule::Reset()
{
	loaded = false;
	tabs.clear();
	return *this;
}

PracticeModule& PracticeModule::Load()
{
	if (loaded) return *this;

	tabs.clear();
	LoadElements();
	SecureElements();

	loaded = true;
	return *this;
}

bool PracticeModule::CreatePopupMenu(CMenu& submenu, Modules::PopupMenuHelper* pPopupMenuHelperInterface)
{
	if (!Visible()) {
		return false;
	}

	Load();

	int nVisibleTabs = std::count_if(tabs.begin(), tabs.end(), 
		bind(&Modules::Element::Visible,
			bind(&Modules::Tabs::value_type::second, _1)
		)
	);

	int nVisibleElements = std::count_if(elements.begin(), elements.end(), 
		bind(&Modules::Element::Visible,
			bind(&Modules::Elements::value_type::second, _1)
		)
	);

	if (nVisibleTabs + nVisibleElements <= 0) {
		return false;
	}

	bool bEnabled = Enabled() && (GetMainFrame() ? GetMainFrame()->IsWindowEnabled() ? true : false : false);

	Modules::PopupMenuHelperImpl* pPopupMenuHelper = dynamic_cast<Modules::PopupMenuHelperImpl*>(pPopupMenuHelperInterface);

	if (!pPopupMenuHelper) {
		ASSERT(FALSE);
		return false;
	}
	
	submenu.CreatePopupMenu();

	if (nVisibleTabs) {
		CString strDefaultTabProperty = FormatString("MyDefaultTab_%s", Name());
		int nDefaultTab = ResolveDefaultTab((short)GetRemotePropertyInt(strDefaultTabProperty, 0, 0, GetCurrentUserName(), false));
		int nActiveTab = -1;
		if (GetMainFrame()) {
			CNxView* pView = GetMainFrame()->GetOpenView(Name());
			if (pView && pView->IsKindOf(RUNTIME_CLASS(CNxTabView))) {
				CNxTabView* pTabView = static_cast<CNxTabView*>(pView);

				nActiveTab = pTabView->GetActiveTab();
			}
		}
		for (Modules::Tabs::iterator tab_it = tabs.begin(); tab_it != tabs.end(); tab_it++) {

			Modules::Element& element(*tab_it->second);

			if (element.Visible()) {
				DWORD dwFlags = MF_BYPOSITION;
				if (!bEnabled || !element.Enabled()) {
					dwFlags |= MF_DISABLED | MF_GRAYED;
				} else if (nActiveTab == tab_it->first) {
					dwFlags |= MF_CHECKED;
				}

				pPopupMenuHelper->m_nNextCmdID++;
				pPopupMenuHelper->m_nNextMenuID++;

				submenu.InsertMenu(pPopupMenuHelper->m_nNextMenuID, dwFlags, pPopupMenuHelper->m_nNextCmdID, element.Name());

				if (tab_it->first == nDefaultTab) {
					submenu.SetDefaultItem(pPopupMenuHelper->m_nNextCmdID);
				}

				// (a.walling 2010-11-26 13:08) - PLID 40444 - Bind a function object to the action map; this will lead to g_Modules.ActivateTab(GetType()) when called.
				pPopupMenuHelper->activateElementActionMap[pPopupMenuHelper->m_nNextCmdID] = bind(
					&PracticeModule::ActivateTab, 
					bind(&Modules::ModuleInfo::GetModule, &g_Modules, GetType()),
					tab_it->first
				);
			}
		}
	}

	if (nVisibleTabs && nVisibleElements) {
		pPopupMenuHelper->m_nNextMenuID++;
		submenu.InsertMenu(pPopupMenuHelper->m_nNextMenuID, MF_BYPOSITION|MF_SEPARATOR, 0, "");
	}

	if (nVisibleElements) {
		for (Modules::Elements::iterator elem_it = elements.begin(); elem_it != elements.end(); elem_it++) {

			Modules::Element& element(*elem_it->second);

			ASSERT(element.MenuID() != 0);

			if (element.Visible()) {
				DWORD dwFlags = MF_BYPOSITION;
				if (!element.Enabled()) {
					dwFlags |= MF_DISABLED | MF_GRAYED;
				}

				pPopupMenuHelper->m_nNextCmdID++;
				pPopupMenuHelper->m_nNextMenuID++;

				submenu.InsertMenu(pPopupMenuHelper->m_nNextMenuID, dwFlags, pPopupMenuHelper->m_nNextCmdID, element.Name());

				// (a.walling 2010-11-26 13:08) - PLID 40444 - Bind a function object to the action map; this will lead to PopupMenuHelperImpl::ActivateMenuFn(element.MenuID()) when called.
				pPopupMenuHelper->activateElementActionMap[pPopupMenuHelper->m_nNextCmdID] = bind(&Modules::PopupMenuHelperImpl::ActivateMenuFn, element.MenuID());
			}
		}
	}

	return true;
}

void PracticeModule::AddTab(int ordinal, LPCTSTR szName, LPCTSTR szShortName)
{
	tabs[ordinal] = Modules::ElementPtr(new Element(szName, szShortName, 0));
}

void PracticeModule::AddElement(int ordinal, LPCTSTR szName, WORD menuID)
{
	elements[ordinal] = Modules::ElementPtr(new Element(szName, "", menuID));
}

// Specific module tab loading

void SchedulerModule::LoadElements()
{
	AddTab(DayTab, "&Day");
	AddTab(WeekTab, "Wee&k");
	AddTab(MultiResourceTab, "&Resources");
	AddTab(MonthTab, "M&onth");

	AddElement(RoomManager, "Room &Manager", ID_ROOM_MANAGER);
	AddElement(FindFirstAvailable, "&Find First Available...", ID_FIRST_AVAILABLE_APPT);		
	AddElement(WaitingList, "&Waiting List", ID_SHOW_MOVEUP_LIST);
	// (a.walling 2014-12-22 12:07) - PLID 64369 - Rescheduling Queue command in menus
	AddElement(ReschedulingQueue, "Rescheduling &Queue", ID_RESCHEDULING_QUEUE);
}

void SchedulerModule::SecureElements()
{
	tabs[DayTab]->Enable(true).Show(true);
	tabs[WeekTab]->Enable(true).Show(true);
	tabs[MultiResourceTab]->Enable(true).Show(true);
	tabs[MonthTab]->Enable(true).Show(true);

	elements[RoomManager]->Enable(true).Show(true);
	elements[FindFirstAvailable]->Enable(true).Show(true);
	elements[WaitingList]->Enable(true).Show(true);
	elements[ReschedulingQueue]->Enable(true).Show(true);
}

void PatientsModule::LoadElements()
{
	AddTab(General1Tab, "General &1", "Gen. &1");
	AddTab(General2Tab, "General &2", "Gen. &2");
	AddTab(CustomTab, "&Custom", "");
	AddTab(ProcedureTab, "Track&ing", "Track.");
	AddTab(FollowUpTab, "Follow &Up", "F.&Up");
	AddTab(NotesTab, "&Notes", "");
	AddTab(AppointmentTab, "A&ppts.", "");
	AddTab(QuoteTab, "&Quotes", "");
	AddTab(InsuranceTab, "In&surance", "In&sr.");
	AddTab(BillingTab, "&Billing", "");
	AddTab(HistoryTab, "Histo&ry", "");
	// (z.manning 2011-06-17 15:26) - PLID 41656 - Fixed the accelerator to show on X as that's always been the hotkey
	AddTab(NexPhotoTab, "Ne&xPhoto", "");
	AddTab(CustomRecordsTab, 
		g_pLicense->CheckForLicense(CLicense::lcEMRStandard, CLicense::cflrSilent) 
		? "&EMR (Std)" : "Custom R&ecords");
	AddTab(NexEMRTab, "Nex&EMR", "");
	AddTab(MedicationTab, "Me&dications", "Me&ds.");
	AddTab(LabTab, "Labs", ""); // (a.walling 2011-01-12 15:02) - PLID 40444 - Well every letter in Labs is used by something else; so I'll revert back to having no accelerator for now.
	AddTab(RefractiveTab, "&Outcomes", "&Outcms.");
	AddTab(SupportTab, "Supp&ort", "");
	AddTab(SalesTab, "Sa&les", "");
	AddTab(ImplementationTab, "I&mplementation", "");
	// (j.gruber 2012-03-07 15:42) - PLID 48702 patient dashboard
	AddTab(PatientDashboard, "Dashb&oard", "Dshbrd");

	AddElement(NewPatient, "Create &New Patient...", ID_NEW_PATIENT_BTN);
	AddElement(ToDoList, "&To-Do List", ID_OPEN_TODO_LIST);
}

void PatientsModule::SecureElements()
{
	//DRT 11/23/2004 - PLID 14779 - Sales is now allowed to sell the 'Billing' package without
	//	selling the 'Scheduler' package - when they buy, the 'Patients' module in the license is
	//	only selected if they purchase the 'Scheduler' on the sales proposal.  So therefore, 
	//	if they have 'Billing' in the license, but not 'Patients', 
	//
	//If they purchase the module called "patients" and not the module called "billing", they get:
	//  G1, G2, Custom, Followup, Notes, Insurance, History, Medications.  [Tracking, EMR, Billing, Quotes, Appts are separate license]
 	//
	//If they purchase "Billing", but not "Patients", they get:
	//  G1, G2, Custom, Follow Up, Notes, Insurance
	//
	//So if they've bought billing, but no patients module, they will not be able to view:
	//  History No
	//  Medications No
	//[ignoring the other licensed tabs]
	//
	//Should notes/followup be available for all or just patients module?
	//Yes

	if(IsRefractive()) {
		tabs[RefractiveTab]->Enable(true).Show(true);
	}
	else {
		tabs[RefractiveTab]->Enable(false).Show(false);
	}

	//if (!(GetCurrentUserPermissions(bioPatientNotes) & SPT_V_________))
	//	tabs[NotesTab]->Enable(false).Show(false);
	//else 
	tabs[NotesTab]->Enable(true).Show(true);

	//PLID 14779 - See above.  History tab is only available if they have the patients module license, not
	//	the billing.
	if(g_pLicense->CheckForLicense(CLicense::lcPatient, CLicense::cflrJustCheckingLicenseNoUI)) {
		//if (!(GetCurrentUserPermissions(bioPatientHistory) & SPT_V_________))
		//	tabs[HistoryTab]->Enable(false).Show(false);
		//else 
		tabs[HistoryTab]->Enable(true).Show(true);
	}
	else {
		tabs[HistoryTab]->Enable(false).Show(false);
	}

	// (c.haag 2009-08-20 09:48) - PLID 35231 - Licensing
	// (c.haag 2009-11-13 10:45) - PLID 36180 - Permissions
	if(g_pLicense->CheckForLicense(CLicense::lcNexPhoto, CLicense::cflrJustCheckingLicenseNoUI)) {
		if (!(GetCurrentUserPermissions(bioPatientNexPhoto) & SPT_V_________))
			tabs[NexPhotoTab]->Enable(false).Show(false);
		else tabs[NexPhotoTab]->Enable(true).Show(true);
	}
	else {
		tabs[NexPhotoTab]->Enable(false).Show(false);
	}

	//if (!(GetCurrentUserPermissions(bioPatientEMR) & SPT_V_________))
	//	tabs[EMRTab]->Enable(false).Show(false);
//	else 

	if (!g_pLicense || g_pLicense->HasEMR(CLicense::cflrSilent) != 1)
		tabs[CustomRecordsTab]->Enable(false).Show(false);
	else tabs[CustomRecordsTab]->Enable(true).Show(true);

	// (a.walling 2007-11-28 09:55) - PLID 28044
	if (!g_pLicense || g_pLicense->HasEMROrExpired(CLicense::cflrSilent) != 2)
		tabs[NexEMRTab]->Enable(false).Show(false);
	else tabs[NexEMRTab]->Enable(true).Show(true);

	//if (!(GetCurrentUserPermissions(bioPatientTracking) & SPT_V_________))
	//	tabs[ProcedureTab]->Enable(false).Show(false);
	//else 
	if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent))
		tabs[ProcedureTab]->Enable(false).Show(false);
	else tabs[ProcedureTab]->Enable(true).Show(true);



	if (!(GetCurrentUserPermissions(bioPatientInsurance) & SPT_V_________))
		tabs[InsuranceTab]->Enable(false).Show(false);
	else tabs[InsuranceTab]->Enable(true).Show(true);

	if (!(GetCurrentUserPermissions(bioPatientBilling) & SPT_V_________) || !g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent))
		tabs[BillingTab]->Enable(false).Show(false);
	else tabs[BillingTab]->Enable(true).Show(true);

	if (!(GetCurrentUserPermissions(bioPatientQuotes) & SPT_V_________) || !g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcQuotes, CLicense::cflrSilent))
		tabs[QuoteTab]->Enable(false).Show(false);
	else tabs[QuoteTab]->Enable(true).Show(true);

	//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
	if (!g_pLicense || !g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent))
		tabs[AppointmentTab]->Enable(false).Show(false);
	else tabs[AppointmentTab]->Enable(true).Show(true);

	if (!(GetCurrentUserPermissions(bioPatientLabs) & sptView) || !g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent))
		tabs[LabTab]->Enable(false).Show(false);
	else tabs[LabTab]->Enable(true).Show(true);

	//PLID 14779 - See above.  Medications tab is only available if they have the patients module license, not
	//	the billing.
	if(g_pLicense->CheckForLicense(CLicense::lcPatient, CLicense::cflrJustCheckingLicenseNoUI)) {
		if (!(GetCurrentUserPermissions(bioPatientMedication) & SPT_V_________))
			tabs[MedicationTab]->Enable(false).Show(false);
		else tabs[MedicationTab]->Enable(true).Show(true);
	}
	else {
		tabs[MedicationTab]->Enable(false).Show(false);
	}

	//m.hancock - 4-27-2006 - Added tab for Patient Labs / Pathology.
	//TODO: display tab according to licensing, which has not been determined yet
	//tabs[LabTab]->Enable(true).Show(true);

	if(IsNexTechInternal()) {
		tabs[SupportTab]->Enable(true).Show(true);
		tabs[SalesTab]->Enable(true).Show(true);
		// (j.gruber 2007-11-07 11:45) - PLID 28023  - added Implementation Tab
		tabs[ImplementationTab]->Enable(true).Show(true);
	}
	else {
		tabs[SupportTab]->Enable(false).Show(false);
		tabs[SalesTab]->Enable(false).Show(false);
		// (j.gruber 2007-11-07 11:45) - PLID 28023  - added Implementation Tab
		tabs[ImplementationTab]->Enable(false).Show(false);
	}
	
	if (CheckCurrentUserPermissions(bioPatient, sptCreate, FALSE, 0, TRUE, TRUE)) {
		elements[NewPatient]->Enable(true).Show(true);
	} else {
		elements[NewPatient]->Enable(false).Show(false);
	}

	// (j.gruber 2012-03-07 15:42) - PLID 48702 patient dashboard
	// (a.wilson 2012-07-05 11:09) - PLID 51332 - replace to check for EMR License, just like EMR
	if (!g_pLicense || g_pLicense->HasEMR(CLicense::cflrSilent) != 2) {
		tabs[PatientDashboard]->Enable(false).Show(false);
	} else {
		tabs[PatientDashboard]->Enable(true).Show(true);
	}

	elements[ToDoList]->Enable(true).Show(true);
}


short PatientsModule::ResolveDefaultTab(short tab)
{	
	// (j.jones 2004-12-22 08:37) - takes the IDs from Preferences,
	// and returns the proper Patient tab
	
	switch(tab) {
		case 0:
			return General1Tab;
		case 1:
			return General2Tab;
		case 2:
			return CustomTab;
		case 3:
			return ProcedureTab;
		case 4:
			return FollowUpTab;
		case 5:
			return NotesTab;
		case 6:
			if(g_pLicense->HasEMR(CLicense::cflrSilent) == 2)
				return NexEMRTab;
			else
				return CustomRecordsTab;
		case 7:
			return InsuranceTab;
		case 8:
			return BillingTab;
		case 9:
			return QuoteTab;
		case 10:
			return AppointmentTab;
		case 11:
			return HistoryTab;
		case 12:
			return MedicationTab;
		case 15:
			return RefractiveTab;
		case 16: 
			return LabTab; 
		case 17: // (c.haag 2009-11-09 17:00) - PLID 35231
			return NexPhotoTab;
		case 18: // (a.wilson 2012-07-03 14:17) - PLID 51332 - adding dashboard tab.
			return PatientDashboard;
		default:
			return General1Tab;
	}
}

void AdminModule::LoadElements()
{
	AddTab(LocationsTab, "&Locations");
	AddTab(BillingTab, "&Billing");
	AddTab(NexSpaTab, "NexS&pa");
	AddTab(HCFATab, "H&CFA");
	AddTab(UB92Tab, "&UB");
	AddTab(MultiFeeTab, "Fee Sched.");
	AddTab(SurgeryTab, "Sur&gery");
	AddTab(Scheduler2Tab, "&Scheduler");
	AddTab(ProcedureTab, "P&rocedure");
	AddTab(CustomRecordsTab,
		g_pLicense->CheckForLicense(CLicense::lcEMRStandard, CLicense::cflrSilent) 
		? "&EMR (Std)" : "Custom R&ecords");
	AddTab(EMRTab, "Nex&EMR");
	AddTab(PhaseTab, "Trac&king");
	AddTab(LabsTab, "Labs");
	AddTab(LinksTab, "L&inks");
	AddTab(ZipCodeTab, "&Zip Code");
	AddTab(AuditingTab, "&Auditing");
}

void AdminModule::SecureElements()
{
	//release for show
	if (!(GetCurrentUserPermissions(bioAdminLocations) & SPT_V_________))
		tabs[LocationsTab]->Enable(false).Show(false);
	else tabs[LocationsTab]->Enable(true).Show(true);

	if(!(GetCurrentUserPermissions(bioAdminBilling) & SPT_V_________))
		tabs[BillingTab]->Enable(false).Show(false);
	else tabs[BillingTab]->Enable(true).Show(true);

	if(!(GetCurrentUserPermissions(bioAdminHCFA) & SPT_V_________)) {
		tabs[HCFATab]->Enable(false).Show(false);
		tabs[UB92Tab]->Enable(false).Show(false);
	}
	else {
		tabs[HCFATab]->Enable(true).Show(true);
		tabs[UB92Tab]->Enable(true).Show(true);
	}

	if(!(GetCurrentUserPermissions(bioAdminSurgery) & SPT_V_________))
		tabs[SurgeryTab]->Enable(false).Show(false);
	else tabs[SurgeryTab]->Enable(true).Show(true);

	if(!(GetCurrentUserPermissions(bioAdminMultiFee) & SPT_V_________))
		tabs[MultiFeeTab]->Enable(false).Show(false);
	else tabs[MultiFeeTab]->Enable(true).Show(true);

	//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
	if(!(GetCurrentUserPermissions(bioAdminScheduler) & SPT_V_________) || !g_pLicense || !g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent))
		tabs[Scheduler2Tab]->Enable(false).Show(false);
	else tabs[Scheduler2Tab]->Enable(true).Show(true);
	
	if (!g_pLicense || (g_pLicense->HasEMR(CLicense::cflrSilent) != 1) 
		|| !(GetCurrentUserPermissions(bioAdminEMR) & SPT_V_________)) 
		tabs[CustomRecordsTab]->Enable(false).Show(false);
	else tabs[CustomRecordsTab]->Enable(true).Show(true);

	if (!g_pLicense || (g_pLicense->HasEMR(CLicense::cflrSilent) != 2) 
		|| !(GetCurrentUserPermissions(bioAdminEMR) & SPT_V_________)) 
		tabs[EMRTab]->Enable(false).Show(false);
	else tabs[EMRTab]->Enable(true).Show(true);

	if(!(GetCurrentUserPermissions(bioAdminTracking) & SPT_V_________))
		tabs[PhaseTab]->Enable(false).Show(false);
	else tabs[PhaseTab]->Enable(true).Show(true);

	if(!(GetCurrentUserPermissions(bioAdminZipCode) & SPT_V_________))
		tabs[ZipCodeTab]->Enable(false).Show(false);
	else tabs[ZipCodeTab]->Enable(true).Show(true);

	if(!(GetCurrentUserPermissions(bioAdminAuditTrail) & SPT_V_________))
		tabs[AuditingTab]->Enable(false).Show(false);
	else tabs[AuditingTab]->Enable(true).Show(true);

	if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent))
		tabs[PhaseTab]->Enable(false).Show(false);
	else tabs[PhaseTab]->Enable(true).Show(true);;

	// (m.hancock 2006-07-05 11:16) - PLID 21187 - Administrator section for labs
	//DRT 7/6/2006 - PLID 21088 - Added permission check.  Also added the license check, which
	//	should have already been done.
	if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) {
		tabs[LabsTab]->Enable(false).Show(false);
	}
	else {
		//Permission
		if(GetCurrentUserPermissions(bioAdminLabs) & sptView)
			tabs[LabsTab]->Enable(true).Show(true);
		else
			tabs[LabsTab]->Enable(false).Show(false);
	}

	// (a.wetta 2007-05-16 09:24) - PLID 25960 - Check license before showing the NexSpa tab
	if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent) ||
		!(GetCurrentUserPermissions(bioAdminNexSpa) & SPT_V_________))
		tabs[NexSpaTab]->Enable(false).Show(false);
	else tabs[NexSpaTab]->Enable(true).Show(true);

	//(e.lally 2010-10-25) PLID 40994 - Always show the links
	tabs[LinksTab]->Enable(true).Show(true);
}

short AdminModule::ResolveDefaultTab(short tab)
{	
	// (j.jones 2004-12-22 08:37) - takes the IDs from Preferences,
	// and returns the proper Admin tab
	
	switch(tab) {
		case 0:
			return LocationsTab;
		case 1:
			return BillingTab;
		case 2:
			return HCFATab;
		case 3:
			return UB92Tab;
		case 4:
			return MultiFeeTab;
		case 5:
			return SurgeryTab;
		case 6:
			return Scheduler2Tab;
		case 7:
			return ProcedureTab;
		case 8:
			if(g_pLicense->HasEMR(CLicense::cflrSilent) == 2)
				return EMRTab;
			else
				return CustomRecordsTab;
		
		case 9:
			return PhaseTab;
		case 10:
			return ZipCodeTab;
		case 11:
			return AuditingTab;
		case 12:
			return LabsTab;
		// (j.gruber 2007-03-19 11:19) - PLID 25165 - adding a discounts tab
		// (a.wetta 2007-03-29 10:48) - PLID 25407 - Changed to retail tab
		// (a.wetta 2007-05-16 08:57) - PLID 25960 - Now the NexSpa tab
		case 13:
			// (a.wetta 2007-05-16 17:43) - PLID 25960 - Make sure that they have the license for this tab
			// before this is set as the default start tab
			if(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent)) 
				return NexSpaTab;
			else
				// Return the default tab
				return LocationsTab;
		break;
		//(e.lally 2010-10-25) PLID 40994 - Add Links tab
		case 14:
			return LinksTab;
		default:
			return LocationsTab;
	}
}

void InventoryModule::LoadElements()
{	
	AddTab(ItemTab, "&Product");
	AddTab(OrderTab, "&Order");
	AddTab(AllocationTab, "A&llocation");
	AddTab(OverviewTab, "Ov&erview");
	AddTab(ReportsTab, "&Reports");
	AddTab(FramesTab, "Frame&s");
	// (j.dinatale 2012-04-17 10:30) - PLID 49078 - Change name from Glasses to optical
	// (j.jones 2016-04-26 15:30) - NX-100214 - added a ShortLabel
	AddTab(GlassesOrderTab, "Opti&cal Orders", "Opti&cal");
	//d.singleton - made sure it only shows in internal
	if(IsNexTechInternal())
	{
		AddTab(InventoryManagementTab,"Ma&nagement", "Mgm&nt"); // r.wilson (2011-4-8) - PLID 43188  
	}
}

void InventoryModule::SecureElements()
{
	if((GetCurrentUserPermissions(bioInvItem) & (SPT_V_________))) {
		tabs[ItemTab]->Enable(true).Show(true);
	}
	else {
		tabs[ItemTab]->Enable(false).Show(false);
	}

	tabs[OrderTab]->Enable(true).Show(true);

	// (j.jones 2007-11-06 11:24) - PLID 28003 - added Allocation tab
	// (j.jones 2007-11-12 15:08) - PLID 28074 - added allocation tab permissions
	// (a.walling 2008-02-15 16:37) - PLID 28946 - hide if not licensed
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if(g_pLicense->HasCandAModule(CLicense::cflrSilent) && (GetCurrentUserPermissions(bioInventoryAllocation) & (SPT_V_________))) {
		tabs[AllocationTab]->Enable(true).Show(true);
	}
	else {
		tabs[AllocationTab]->Enable(false).Show(false);
	}

	// (j.jones 2007-11-06 12:11) - PLID 27989 - added Overview tab
	// (a.walling 2008-02-15 16:37) - PLID 28946 - hide if not licensed
	//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if(g_pLicense->HasCandAModule(CLicense::cflrSilent) && (GetCurrentUserPermissions(bioInventoryOverview) & (SPT_V_________))) {
		tabs[OverviewTab]->Enable(true).Show(true);
	}
	else {
		tabs[OverviewTab]->Enable(false).Show(false);
	}

	// (c.haag 2009-01-13 12:07) - PLID 32683 - Added Report tab	
	// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
	if(g_pLicense->HasCandAModule(CLicense::cflrSilent) && (GetCurrentUserPermissions(bioInventoryModuleReportsTab) & (SPT_V_________))) {
		tabs[ReportsTab]->Enable(true).Show(true);
	}
	else {
		tabs[ReportsTab]->Enable(false).Show(false);
	}

	// (z.manning 2010-06-17 16:04) - PLID 39222 - Frames tab
	// (c.haag 2010-06-30 11:25) - PLID 39424 - License checking
	if(g_pLicense->CheckForLicense(CLicense::lcFrames, CLicense::cflrSilent)) {
		tabs[FramesTab]->Enable(true).Show(true);
	}
	else {
		tabs[FramesTab]->Enable(false).Show(false);
	}

	// (s.dhole 2010-11-15 11:30) - PLID 41470
	//TES 12/9/2010 - PLID 41701 - License checking
	if(g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent) && (GetCurrentUserPermissions(bioInventoryModule) & (SPT_V_________))) {
		tabs[GlassesOrderTab]->Enable(true).Show(true);
	}
	else {
		tabs[GlassesOrderTab]->Enable(false).Show(false);
	}
}

void FinancialModule::LoadElements()
{	
	AddTab(BankingTab, "&Banking");
	AddTab(QBMSTab, "&Credit Cards");
	AddTab(PaperBatchTab, "&Paper Batch");
	AddTab(EBillingTab, "&EBilling Batch");
	AddTab(EEligibilityTab, "E-Eli&gibility");
	AddTab(BatchPayTab, "B&atch Payments", "B&atch Pay.");
	AddTab(BillingFUTab, "Billing &Followup", "Billing &F/U");
}

void FinancialModule::SecureElements()
{
	//these commented out permissions never existed before, so we will need to make a mod for all people that
	//have access to the financial module should have access to these sections

	//#pragma message (TODO "Don't forget to handle (in mainframe.cpp) the case when all these tabs are unavailable or inaccessible.")

	if (GetCurrentUserPermissions(bioBankingTab) & SPT__R_________ANDPASS && g_pLicense && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
		tabs[BankingTab]->Enable(true).Show(true);
	}
	else {
		tabs[BankingTab]->Enable(false).Show(false);
	}

	if ((GetCurrentUserPermissions(bioClaimForms) & SPT_V_________) && g_pLicense && g_pLicense->CheckForLicense(CLicense::lcHCFA, CLicense::cflrSilent)) {
		tabs[PaperBatchTab]->Enable(true).Show(true);
	}
	else {
		tabs[PaperBatchTab]->Enable(false).Show(false);
	}

	if ((GetCurrentUserPermissions(bioEBilling) & SPT_V_________) && g_pLicense && g_pLicense->CheckForLicense(CLicense::lcEbill, CLicense::cflrSilent))
		tabs[EBillingTab]->Enable(true).Show(true);
	else
		tabs[EBillingTab]->Enable(false).Show(false);

	// (j.jones 2007-05-01 15:13) - PLID 8993 - added E-Eligibility Tab
	// (j.jones 2007-06-29 09:11) - PLID 23950 - added E-Eligibility licensing
	// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
	if ((GetCurrentUserPermissions(bioEEligibility) & SPT_V_________) && g_pLicense && g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrSilent))
		tabs[EEligibilityTab]->Enable(true).Show(true);
	else
		tabs[EEligibilityTab]->Enable(false).Show(false);
	// (s.tullis 2014-06-23 08:57) - PLID 49455 - Permission: Batch payment 
	if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent) && ((GetCurrentUserPermissions(bioBatchPayment) & SPT__R________))) {
		tabs[BatchPayTab]->Enable(true).Show(true);
	}
	else {
		tabs[BatchPayTab]->Enable(false).Show(false);
	}
	
	// (s.tullis 2014-06-23 11:23) - PLID 62506 - Permission: Billing Followup 
	if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent) && ((GetCurrentUserPermissions(bioBillingfollowup) & SPT__R________))) {
		tabs[BillingFUTab]->Enable(true).Show(true);
	}
	else {
		tabs[BillingFUTab]->Enable(false).Show(false);
	}

	// (d.thompson 2009-06-30) - PLID 34745
	// (d.thompson 2010-09-02) - PLID 40371 - Check for any cc processing license
	// (j.jones 2015-09-30 10:46) - PLID 67177 - this tab is not available if ICCP is licensed and enabled, IsICCPEnabled checks both
	if ((GetCurrentUserPermissions(bioCCProcessingTab) & SPT_V__________ANDPASS) && g_pLicense && g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent)
		&& !IsICCPEnabled()) {
		tabs[QBMSTab]->Enable(true).Show(true);
	}
	else {
		tabs[QBMSTab]->Enable(false).Show(false);
	}
	


}

// (j.jones 2007-05-01 15:33) - PLID 8993 - created due to E-Eligibility being a new tab in the middle
// (d.thompson 2009-11-16) - PLID 36301 - I removed the export, HL7, and TOPS tabs.  If someone had
//	these as their default tab previously, they will resolve back to 
short FinancialModule::ResolveDefaultTab(short tab)
{	
	//takes the IDs from Preferences, and returns the proper Financial tab
	
	switch(tab) {
		case 0:
			return BankingTab;
		case 1:
			return PaperBatchTab;
		case 2:
			return EBillingTab;
		case 3:
			return BatchPayTab;
		case 4:
			return BillingFUTab;
		case 6:
			return EEligibilityTab;
		// (d.thompson 2009-06-30) - PLID 34745 - Added QBMS tab
		case 8:
			return QBMSTab;

		// (d.thompson 2009-11-16) - PLID 36301 - Moved Export, HL7, TOPS to the 'Links' module.  If these
		//	are in data, just send them back to the banking tab.
		case 5:	//Export
		// (j.jones 2008-04-08 15:01) - PLID 29587 - added HL7 tab
		case 7:	//HL7
		//(e.lally 2009-10-07) PLID 35803 - Added TOPS tab
		case 9:	//TOPS
		default:
			return BankingTab;
	}
}

void MarketingModule::LoadElements()
{
	AddTab(ReferralTab, "&Referrals");
	AddTab(ProcedureTab, "Proced&ure");
	AddTab(CoordinatorTab, "C&oordinator");
	AddTab(DateTab, "&Date");
	AddTab(ReasonTab, "Rea&son");
	AddTab(PerfIndTab, "&Performance Indices", "&Performance Ix.");
	AddTab(RegionalTab, "R&egional");
	AddTab(RetentionTab, "Rete&ntion");
	AddTab(EffectivenessTab, "E&ffect&iveness");
	AddTab(CostTab, "&Cost");
	AddTab(InternalTab, "Interna&l");
}

void MarketingModule::SecureElements()
{
	long nUseMarketing = g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrSilent);
	long nUseRetention = g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrSilent);

	if (nUseMarketing == 1) {
		tabs[ReferralTab]->Enable(true).Show(true);
		tabs[ProcedureTab]->Enable(true).Show(true);
		tabs[CoordinatorTab]->Enable(true).Show(true);
		tabs[ReasonTab]->Enable(true).Show(true);
		tabs[DateTab]->Enable(true).Show(true);
		tabs[PerfIndTab]->Enable(true).Show(true);
		tabs[RegionalTab]->Enable(true).Show(true);
		tabs[EffectivenessTab]->Enable(true).Show(true);
		tabs[CostTab]->Enable(true).Show(true);
	
	}
	else {

		tabs[ReferralTab]->Enable(false).Show(false);
		tabs[ProcedureTab]->Enable(false).Show(false);
		tabs[CoordinatorTab]->Enable(false).Show(false);
		tabs[ReasonTab]->Enable(false).Show(false);
		tabs[DateTab]->Enable(false).Show(false);
		tabs[PerfIndTab]->Enable(false).Show(false);
		tabs[RegionalTab]->Enable(false).Show(false);
		tabs[EffectivenessTab]->Enable(false).Show(false);
		tabs[CostTab]->Enable(false).Show(false);
	}

	if (nUseRetention == 1) {
		tabs[RetentionTab]->Enable(true).Show(true);
	}
	else {
		tabs[RetentionTab]->Enable(false).Show(false);
	}

	if (IsNexTechInternal() ) {
		tabs[InternalTab]->Enable(true).Show(true);
	}
	else {
		tabs[InternalTab]->Enable(false).Show(false);
	}
}

void ReportsModule::LoadElements()
{
	AddTab(PatientsTab, "Patients");
	AddTab(ContactsTab, "Contacts");
	AddTab(MarketingTab, "Marketing");
	AddTab(InventoryTab, "Inventory");
	AddTab(SchedulerTab, "Scheduler");
	AddTab(ChargesTab, "Charges");
	AddTab(PaymentsTab, "Payments");
	AddTab(FinancialTab, "Financial");
	AddTab(ASCTab, "ASC");
	// (j.jones 2016-04-26 15:30) - NX-100214 - added a ShortLabel
	AddTab(AdministrationTab, "Administration", "Admin");
	AddTab(OtherTab, "Other");
	// (j.jones 2016-04-26 15:30) - NX-100214 - added a ShortLabel
	AddTab(PracAnalTab, "Prac. Analysis", "Analysis");
	AddTab(OpticalTab, "Optical");
}

void ReportsModule::SecureElements()
{
	if((!g_pLicense->CheckForLicense(CLicense::lcPatient, CLicense::cflrSilent)) /*||
		(! CheckReportTabAvailability(Patients))*/) {
		tabs[PatientsTab]->Enable(false).Show(false);
	}

	//TES 5/14/2004: This tab has tracking reports in it, so we'll just show those.
	/*if(!g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrSilent)) {
		tabs[Marketing]->Enable(false).Show(false);
		nSize--;
	}*/
	
	if((!g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrSilent) && !g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent))
		/*|| (!CheckReportTabAvailability(Marketing))*/) { 
		tabs[MarketingTab]->Enable(false).Show(false);
	}

	if((!g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent)) /*||
		(! CheckReportTabAvailability(Inventory))*/) {
		tabs[InventoryTab]->Enable(false).Show(false);
	}

	//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
	if((!g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent)) /*||
		( !CheckReportTabAvailability(Scheduler)) */) {
		tabs[SchedulerTab]->Enable(false).Show(false);
	}

	if((!g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) /*||
		(! CheckReportTabAvailability(Charges)) */) {
		tabs[ChargesTab]->Enable(false).Show(false);
	}

	if((!g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) /*||
		(! CheckReportTabAvailability(Payments)) */) {
		tabs[PaymentsTab]->Enable(false).Show(false);
	}

	if((!g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) /*||
		(! CheckReportTabAvailability(Financial)) */) {
		tabs[FinancialTab]->Enable(false).Show(false);
	}

	if(!IsSurgeryCenter(false) /*|| 
		(! CheckReportTabAvailability(ASC))*/ ) {
		tabs[ASCTab]->Enable(false).Show(false);
	}
	// (s.dhole 2012-04-02 09:56) - PLID 49341
	if(!g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)) {
		tabs[OpticalTab]->Enable(false).Show(false);
	}
	
}

void ContactsModule::LoadElements()
{
	AddTab(GeneralTab, "&General");
	AddTab(NotesTab, "&Notes");
	AddTab(FollowUpTab, "Follow &Up");
	// (z.manning 2011-06-17 15:27) - PLID 41656 - Changed the accelerator to R to match patients module and not conflict with help menu
	AddTab(HistoryTab, "Histo&ry");
	AddTab(AttendanceTab, "Atten&dance");
	
	AddElement(NewContact, "New &Contact", ID_NEW_CONTACT);
}

void ContactsModule::SecureElements()
{
	tabs[GeneralTab]->Enable(true).Show(true);
	tabs[NotesTab]->Enable(true).Show(true);
	tabs[FollowUpTab]->Enable(true).Show(true);
	tabs[HistoryTab]->Enable(true).Show(true);
	if(IsNexTechInternal()) {
		tabs[AttendanceTab]->Enable(true).Show(true);
	} else {
		tabs[AttendanceTab]->Enable(false).Show(false);
	}
}

void SurgeryCenterModule::LoadElements()
{
	AddTab(CaseHistoryTab, "&Case Histories");
	AddTab(DoctorPrefsTab, "Doctor &Prefs.");
	AddTab(CredentialsTab, "Cre&dentials");
}

void SurgeryCenterModule::SecureElements()
{
	tabs[CaseHistoryTab]->Enable(true).Show(true);
	tabs[DoctorPrefsTab]->Enable(true).Show(true);
	tabs[CredentialsTab]->Enable(true).Show(true);
}

void LinksModule::LoadElements()
{
	AddTab(ExportTab, "E&xport");
	AddTab(HL7Tab, "HL&7");
	AddTab(TOPSTab, "&TOPS");
	AddTab(BOLDTab, "&BOLD");
	AddTab(DeviceConfigTab, "&Devices");
	AddTab(SendLabsTab, "&Send Labs");
	AddTab(ReceiveLabsTab, "&Receive Labs");
	// (j.jones 2016-04-26 15:30) - NX-100214 - added a ShortLabel
	AddTab(DirectMessage, "D&irect Message", "D&irect Msg"); // (j.camacho 2013-10-16 17:47) - PLID 59064
	AddTab(CancerCasesTab, "&Cancer Cases"); //TES 4/23/2014 - PLID 61854
	AddTab(OnlineVisits, "&Online Visits"); // (r.farnworth 2016-02-25 10:02) - PLID 68396
	
}

void LinksModule::SecureElements()
{
	// (d.thompson 2009-11-16) - PLID 36301 - Moved from CFinView
	//Always available
	tabs[ExportTab]->Enable(true).Show(true);

	// (j.jones 2008-04-08 15:01) - PLID 29587 - added HL7 tab
	//TES 5/18/2009 - PLID 34282 - Added HL7 permissions
	if ((GetCurrentUserPermissions(bioHL7BatchDlg) & SPT_V_________) && g_pLicense && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent)) {
		tabs[HL7Tab]->Enable(true).Show(true);
		tabs[SendLabsTab]->Enable(true).Show(true);// (a.vengrofski 2010-05-28 09:45) - PLID <38919> - Wrapped the sending labs into HL7 for the time being
		tabs[ReceiveLabsTab]->Enable(true).Show(true);// (a.vengrofski 2010-07-22 17:13) - PLID <38919> - Wrapped the sending labs into HL7 for the time being
	}
	else {
		tabs[HL7Tab]->Enable(false).Show(false);
		tabs[SendLabsTab]->Enable(false).Show(false);// (a.vengrofski 2010-05-28 09:45) - PLID <38919> - Wrapped the sending labs into HL7 for the time being
		tabs[ReceiveLabsTab]->Enable(false).Show(false);// (a.vengrofski 2010-07-22 17:13) - PLID <38919> - Wrapped the sending labs into HL7 for the time being
	}

	//(e.lally 2009-10-07) PLID 35803 - Added TOPS tab
	tabs[TOPSTab]->Enable(true).Show(true);

	// (j.gruber 2010-04-28 12:26) - PLID 38337 - BOLD Tab
	tabs[BOLDTab]->Enable(true).Show(true);

	// (d.lange 2010-05-07 16:04) - PLID 38536 - Devices Tab
	// (c.haag 2010-06-30 11:25) - PLID 39424 - License checking.
	if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcDeviceImport, CLicense::cflrSilent)) {
		tabs[DeviceConfigTab]->Enable(true).Show(true);
	}
	else {
		tabs[DeviceConfigTab]->Enable(false).Show(false);
	}
	// (j.camacho 2013-10-17 16:20) - PLID 59064
	if (g_pLicense && g_pLicense->CheckForLicense(CLicense::lcDirectMessage, CLicense::cflrSilent)) {
		tabs[DirectMessage]->Enable(true).Show(true);
	}
	else {
		tabs[DirectMessage]->Enable(false).Show(false);
	}
	//TES 4/23/2014 - PLID 61854 - Cancer Cases Tab
	//TES 5/5/2014 - PLID 61854 - Added license/permission checks
	if ((GetCurrentUserPermissions(bioPatientEMR) & SPT__R_________ANDPASS) && g_pLicense && g_pLicense->HasEMR(CLicense::cflrSilent)) {
		tabs[CancerCasesTab]->Enable(true).Show(true);
	}
	else {
		tabs[CancerCasesTab]->Enable(false).Show(false);
	}

	// (r.farnworth 2016-03-04 09:05) - PLID 68396 - Create a new tab in the Links module that is for Online Visits
	// (r.farnworth 2016-03-04 09:05) - PLID 68451 - Create a permission to get into the Iagnosis Online Visits tab.
	if (GetCurrentUserPermissions(bioOnlineVisitTab) & (sptView | sptViewWithPass)) {
		tabs[OnlineVisits]->Enable(true).Show(true);
	}
	else {
		tabs[OnlineVisits]->Enable(false).Show(false);
	}
}

//TES 4/23/2014 - PLID 61854 - Added Cancer Cases tab in the middle, so we need this function now
short LinksModule::ResolveDefaultTab(short tab)
{
	switch(tab) {
	case 0:
		return ExportTab;
		break;
	case 1:
		return HL7Tab;
		break;
	case 2:
		return TOPSTab;
		break;
	case 3:
		return BOLDTab;
		break;
	case 5:
		return DeviceConfigTab;
		break;
	case 6:
		return SendLabsTab;
		break;
	case 7:
		return ReceiveLabsTab;
		break;
	case 8:
		return DirectMessage;
		break;
	case 9:
		return CancerCasesTab;
		break;
	case 10:
		// (r.farnworth 2016-03-14 12:36) - PLID 68396
		return OnlineVisits;
		break;
	default:
		return ExportTab;
		break;
	}
}
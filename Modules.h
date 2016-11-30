#pragma once

#include <map>
#include "boost/shared_ptr.hpp"

// (a.walling 2010-11-26 13:08) - PLID 40444 - The Modules namespace and other code is used to keep track of the various modules and their tabs and optional common actions
// This keeps things in a single, unified place, since tabs are not part of the menu system. This also allows us the flexibility to create and handle menus dynamically.

// forward def
class PracticeModule;
typedef boost::shared_ptr<PracticeModule> PracticeModulePtr;
typedef std::map<int, PracticeModulePtr> PracticeModuleMap;

namespace Modules 
{

// Module enumerations (in toolbar order!)
enum Type {
	Patients = 0,
	Scheduler,
	LetterWriting,
	Contacts,
	Marketing,
	Inventory,
	Financial,
	SurgeryCenter,
	Reports,
	Admin,
	Links,
	Invalid = -1
};

LPCTSTR GetModuleName(Type module);

class Element
{
public:
	Element(LPCTSTR szName, LPCTSTR szShortName, WORD menuID_)
		: name(szName)
		, shortName(szShortName)
		, menuID(menuID_)
		, enabled(true)
		, visible(true)

	{
	};

	virtual ~Element() {};

	LPCTSTR Name() const {
		return name;
	};

	LPCTSTR ShortName() const {
		return shortName;
	};

	WORD MenuID() const {
		return menuID;
	};

	bool Enabled() const {
		return enabled;
	};

	bool Visible() const {
		return visible;
	};

	//

	Element& Enable(bool _enable) {
		enabled = _enable;
		return *this;
	};

	Element& Show(bool _show) {
		visible = _show;
		return *this;
	};

protected:
	LPCTSTR name;
	LPCTSTR shortName;
	WORD menuID;
	bool enabled;
	bool visible;
};
typedef boost::shared_ptr<Element> ElementPtr;

typedef std::map<int, ElementPtr> Tabs;
typedef std::map<int, ElementPtr> Elements;

class ModuleInfo
{
public:	
	ModuleInfo() {
		Initialize();
	};

	PracticeModuleMap& GetModules() {
		return modules;
	};

	PracticeModulePtr operator[](Modules::Type moduleType) {
		return GetModulePtr(moduleType);
	};

	PracticeModulePtr operator[](LPCTSTR szModuleName) {
		return GetModulePtrByName(szModuleName);
	};

	PracticeModulePtr GetModulePtr(Modules::Type moduleType);
	PracticeModulePtr GetModulePtrByName(LPCTSTR szModuleName);

	PracticeModule& GetModule(Modules::Type moduleType) {
		return *GetModulePtr(moduleType);
	};
	PracticeModule& GetModuleByName(LPCTSTR szModuleName) {
		return *GetModulePtrByName(szModuleName);
	};

	void Initialize();
	void ResetAll();
	void LoadAll();

	void AppendAllPopupMenus(CMenu& menu, long& nMenuID, long& nCmdID);

protected:
	PracticeModuleMap modules;
};

class PopupMenuHelper;
typedef boost::shared_ptr<PopupMenuHelper> PopupMenuHelperPtr;

class PopupMenuHelper
{
public:
	PopupMenuHelper(CMenu& menu, long& nMenuID, long& nCmdID)
		: m_menu(menu)
		, m_nNextMenuID(nMenuID)
		, m_nNextCmdID(nCmdID)
	{
	};

	virtual ~PopupMenuHelper() {};

	static PopupMenuHelperPtr Create(CMenu& menu, long& nMenuID, long& nCmdID);

	virtual void AppendAllPopupMenus() = 0;

	virtual void AppendModulePopupMenu(PracticeModulePtr module) = 0;

	virtual void CreateModulePopupMenu(CMenu& submenu, PracticeModulePtr module) = 0;

	virtual bool HandleCommand(long nCmdID) = 0;

	long& m_nNextMenuID;
	long& m_nNextCmdID;
	CMenu& m_menu;
};


void NxTabUpdater(NxTab::_DNxTabPtr pTabs, Tabs::value_type value);

}; // end namespace Modules



//////////////////////////////////////////




class PracticeModule : public Modules::Element
{
public:
	PracticeModule(Modules::Type _type);

	void Activate();
	void ActivateTab(int tab);

	virtual short ResolveDefaultTab(short tab);
	// (a.walling 2010-12-09 09:25) - PLID 40444 - Handle an override default tab to open a module immediately to the desired tab
	// rather than opening to the default tab and then switching
	virtual short OverrideDefaultTab() {
		return overrideDefaultTab;
	};

	PracticeModule& Reset();
	PracticeModule& Load();

	Modules::Tabs& GetTabs() {
		Load();
		return tabs;
	};

	Modules::Elements& GetElements() {
		Load();
		return elements;
	};

	Modules::Type GetType() {
		return type;
	};

	virtual bool CreatePopupMenu(CMenu& submenu, Modules::PopupMenuHelper* pPopupMenuHelperInterface);

protected:
	Modules::Type type;
	bool loaded;

	Modules::Tabs tabs;
	Modules::Elements elements;

	short overrideDefaultTab;

	void AddTab(int ordinal, LPCTSTR szName, LPCTSTR szShortName = "");
	void AddElement(int ordinal, LPCTSTR szName, WORD menuID = 0);

	virtual void LoadElements() = 0;
	virtual void SecureElements() = 0;
};

// Actual modules and tabs

class SchedulerModule : public PracticeModule
{
public:
	SchedulerModule() 
		: PracticeModule(Modules::Scheduler)
	{
	};

	enum Tab
	{
		DayTab = 0,
		WeekTab,
		MultiResourceTab,
		MonthTab,
		Last_Tab
	};

	enum Element
	{
		RoomManager = Last_Tab,
		FindFirstAvailable,
		WaitingList,
		ReschedulingQueue,
		Last_Element,
	};

protected:
	virtual void LoadElements();
	virtual void SecureElements();
};

class PatientsModule : public PracticeModule
{
public:
	PatientsModule()
		: PracticeModule(Modules::Patients)
	{
	};

	enum Tab
	{
		General1Tab = 0,
		General2Tab,
		CustomTab,
		ProcedureTab,
		FollowUpTab,
		NotesTab,
		AppointmentTab,
		QuoteTab,
		InsuranceTab,
		BillingTab,
		HistoryTab,
		NexPhotoTab,	// (c.haag 2009-08-19 15:05) - PLID 35231
		CustomRecordsTab,
		NexEMRTab,	
		MedicationTab,
		LabTab,
		RefractiveTab,
		SupportTab,
		SalesTab,
		ImplementationTab,
		PatientDashboard, // (j.gruber 2012-06-15 20:47) - PLID 48702
		Last_Tab
	};

	enum Element
	{
		NewPatient = Last_Tab,
		ToDoList,
		Last_Element,
	};
	
	virtual short ResolveDefaultTab(short tab);

protected:
	virtual void LoadElements();
	virtual void SecureElements();
};

class AdminModule : public PracticeModule
{
public:
	AdminModule() 
		: PracticeModule(Modules::Admin)
	{
	};

	enum Tab
	{
		LocationsTab = 0,
		BillingTab,
		// (j.gruber 2007-03-19 11:19) - PLID 25165 - adding a discounts tab
		// (a.wetta 2007-03-29 10:38) - PLID 25407 - This tab is being turned into the Retail tab and discounts 
		// are on this tab
		// (a.wetta 2007-05-16 08:58) - PLID 25960 - Now the NexSpa tab
		NexSpaTab,
		HCFATab,
		UB92Tab,
		MultiFeeTab,
		SurgeryTab,
		Scheduler2Tab,
		ProcedureTab,
		CustomRecordsTab,
		EMRTab,
		PhaseTab,
		LabsTab,
		//(e.lally 2010-10-25) PLID 40994 - Added Links tab
		LinksTab,
		ZipCodeTab,
		AuditingTab,
		Last_Tab
	};
	
	virtual short ResolveDefaultTab(short tab);

protected:
	virtual void LoadElements();
	virtual void SecureElements();
};

class InventoryModule : public PracticeModule
{
public:
	InventoryModule() 
		: PracticeModule(Modules::Inventory)
	{
	};

	enum Tab
	{
		ItemTab = 0,
		OrderTab,
		AllocationTab,	// (j.jones 2007-11-06 11:22) - PLID 28003 - added Allocation tab
		OverviewTab,	// (j.jones 2007-11-06 12:11) - PLID 27989 - addded Overview tab
		ReportsTab,		// (c.haag 2009-01-12 15:19) - PLID 32683 - Added Reports tab
		FramesTab,		// (z.manning 2010-06-17 15:34) - PLID 3922
		GlassesOrderTab,// (s.dhole 2010-09-15 16:18) - PLID 40538
		InventoryManagementTab,  // r.wilson (2011-4-8) - PLID 43188 
		Last_Tab    
	};

protected:
	virtual void LoadElements();
	virtual void SecureElements();
};

class FinancialModule : public PracticeModule
{
public:
	FinancialModule() 
		: PracticeModule(Modules::Financial)
	{
	};

	// (d.thompson 2009-11-16) - PLID 36301 - Removed the Export, HL7, and TOPS tabs and put them
	//	in the 'Links' module.
	enum Tab
	{
		BankingTab = 0,
		// (d.thompson 2009-06-30) - PLID 34745 - Added cc processing as second tab
		QBMSTab,
		PaperBatchTab,
		EBillingTab,
		EEligibilityTab,
		BatchPayTab,
		BillingFUTab,
		Last_Tab
	};
	
	virtual short ResolveDefaultTab(short tab);

protected:
	virtual void LoadElements();
	virtual void SecureElements();
};

class MarketingModule : public PracticeModule
{
public:
	MarketingModule() 
		: PracticeModule(Modules::Marketing)
	{
	};

	enum Tab
	{
		ReferralTab = 0,
		ProcedureTab,
		CoordinatorTab,
		DateTab,
		ReasonTab,
		PerfIndTab,
		RegionalTab,
		RetentionTab,
		EffectivenessTab,
		CostTab,
		InternalTab,
		Last_Tab
	};

protected:
	virtual void LoadElements();
	virtual void SecureElements();
};

class ReportsModule : public PracticeModule
{
public:
	ReportsModule() 
		: PracticeModule(Modules::Reports)
	{
	};

	// (j.gruber 2008-07-11 11:36) - PLID 28976 - add Practice Analysis tab
	// (s.dhole 2012-03-30 17:14) - PLID 49341 Added OpticalTab 
	enum Tab 
	{
		PatientsTab = 0,
		ContactsTab,
		MarketingTab,
		InventoryTab,
		SchedulerTab,
		ChargesTab,
		PaymentsTab,
		FinancialTab,
		ASCTab,
		AdministrationTab,
		OtherTab,
		PracAnalTab,
		OpticalTab,
		Last_Tab
	};

protected:
	virtual void LoadElements();
	virtual void SecureElements();
};

class ContactsModule : public PracticeModule
{
public:
	ContactsModule() 
		: PracticeModule(Modules::Contacts)
	{
	};

	// (k.messina 2010-04-14 11:36)
	enum Tab
	{
		GeneralTab = 0,
		NotesTab,
		FollowUpTab,
		HistoryTab,
		AttendanceTab,
		Last_Tab
	};

	enum Element
	{
		NewContact = Last_Tab,
		Last_Element,
	};

protected:
	virtual void LoadElements();
	virtual void SecureElements();
};

class LetterWritingModule : public PracticeModule
{
public:
	LetterWritingModule() 
		: PracticeModule(Modules::LetterWriting)
	{
	};

protected:
	virtual void LoadElements() {};
	virtual void SecureElements() {};
};

class SurgeryCenterModule : public PracticeModule
{
public:
	SurgeryCenterModule() 
		: PracticeModule(Modules::SurgeryCenter)
	{
	};

	enum Tab
	{
		CaseHistoryTab = 0,
		DoctorPrefsTab,
		CredentialsTab,
		Last_Tab
	};

protected:
	virtual void LoadElements();
	virtual void SecureElements();
};

class LinksModule : public PracticeModule
{
public:
	LinksModule() 
		: PracticeModule(Modules::Links)
	{
	};

	// (j.gruber 2010-04-28 12:26) - PLID 38337 - added BOLD tab
	// (d.lange 2010-05-07 16:00) - PLID 38536 - Added Devices tab
	// (a.vengrofski 2010-05-28 09:34) - PLID <38919> - Added Send Labs Tab
	// (a.vengrofski 2010-07-22 17:10) - PLID <38919> - Added Receive Labs Tab
	// (j.camacho 2013-10-17 16:03) - PLID  - Added DirectMessage Tab
	// (r.farnworth 2016-02-25 09:56) - PLID 68396 - Added Online Vists
	enum Tab 
	{
		ExportTab = 0,
		HL7Tab,
		TOPSTab,
		BOLDTab,
		CancerCasesTab, //TES 4/23/2014 - PLID 61854
		DeviceConfigTab,
		SendLabsTab,
		ReceiveLabsTab,
		DirectMessage,
		OnlineVisits,
		Last_Tab
		
	};

protected:
	virtual void LoadElements();
	virtual void SecureElements();
	//TES 4/23/2014 - PLID 61854 - Added Cancer Cases tab in the middle, so we need this function now
	virtual short ResolveDefaultTab(short tab);
};

// singleton
extern Modules::ModuleInfo g_Modules;

//
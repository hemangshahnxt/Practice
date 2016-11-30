// Reports.cpp : implementation file
//

#include "stdafx.h"
#include "pracprops.h"
#include "Reports.h"
#include "ReportsRc.h"
//#include "PrintOptions.h"
#include "ExternalForm.h"
//TES 1/2/2008 - PLID 28000 - VS2008 - Attempting to test 28000, I discovered that this is dead code.
//#include "EditReportBatch.h"
#include "NxStandard.h"
//#include "ReportDocView.h"
#include "ReportView.h"
#include "GlobalUtils.h"
#include "GlobalReportUtils.h"
#include "GetNewIDName.h"
#include "MsgBox.h"
#include "ProcessRptDlg.h"
#include "GlobalDataUtils.h"
#include "FilterEditDlg.h"
#include "Filter.h"
#include "Groups.h"
#include "ReportInfo.h"
#include "PracProps.h"
#include "EditReportPickerDlg.h"
#include "MainFrm.h"
#include "NxSecurity.h"
#include "DontShowDlg.h"
#include "RegUtils.h"
#include "FileUtils.h"
#include "ChildFrm.h"

#include "InternationalUtils.h"

#include "MultiSelectDlg.h"
#include "GlobalDrawingUtils.h"
#include "NxMessageDef.h"
#include "boost/bind.hpp"
#include "ChooseDateRangeDlg.h" // (j.dinatale 2011-11-14 15:52) - PLID 45658

#include "ReportAdo.h"

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace NxTab;
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define USER_BATCH_NAME		"< Default Batch >"
#define IDT_FREEBIGDATALISTS 1042

//////////////////////////////////////////////////////////////////////////////
// CReports dialog

/* Reports Module -- code documentation
	updated: 7/28/99 

  Here's how it works:  all reports settings and options are stored in a const array of CReportInfo objects
  The Available Reports DataList displays these. Each time the user selects a report to display or print,
  the CReportInfo object is copied and added to the current batch.  This way, any number of reports can be
  batched, and all the settings for the report are in memory instead of only in the const array.  LoadFilters()
  retrieves the necessary data from CReportInfo to set the filters.  When a filter is changed, the CReportInfo
  object is instantly updated.  CReportInfo is responsible for printing/previewing the report and usually does 
  so by simply opening an ADO recordset based on its own sql statement combined with the given filter after #'s, 
  {'s and }'s are stripped.  Subreports' filters are handled by the unstripped version of the given filter.>

  -- Have fun.
*/

CReports::CReports(CWnd* pParent)
	: CNxDialog(CReports::IDD, pParent),
	m_patientChecker(NetUtils::PatCombo),
	m_doctorChecker(NetUtils::Providers),	
	m_locationChecker(NetUtils::LocationsT),
	m_groupChecker(NetUtils::Groups),
	m_filterChecker(NetUtils::FiltersT),
	m_nSelectedPatientID(-1)
{
	//{{AFX_DATA_INIT(CReports)
	//}}AFX_DATA_INIT
	m_lstReports.RemoveAll();
	m_bToolTipsActive = false;

	m_nCurrentGroup = -1;
	m_bModified = FALSE;
	m_strCurrentBatchName = USER_BATCH_NAME;

	m_bFilterShowInactivePatients = FALSE;
	m_bFilterShowInactiveProviders = FALSE;
	m_bFilterShowInactiveLocations = FALSE;

	CurrReport = NULL;
}

CReports::~CReports()
{
	// Deallocate the memory for everything in the batch
	CReportInfo* pRep;
	for (int i = m_RBatch.GetUpperBound(); i >= 0; i--) {
		// Get the pointer
		pRep = (CReportInfo *)m_RBatch.GetAt(i);
		// Delete the object
		if (pRep) delete pRep;
		// Remove the pointer from the list
		m_RBatch.RemoveAt(i);
	}

	// Deallocate the memory that each map element 
	// refers to and then remove all the map elements
	POSITION p = m_lstReports.GetStartPosition();
	void *pKey;
	while (p) {
		m_lstReports.GetNextAssoc(p, pKey, (void *&)pRep);
		if (pRep) delete pRep;
	}
	m_lstReports.RemoveAll();
}

void CReports::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	// (a.walling 2008-04-22 13:47) - PLID 29642 - m_CurrName had a member var but never initialized here
	//{{AFX_DATA_MAP(CReports)
	DDX_Control(pDX, IDC_REPORTS_ALL_YEAR, m_btnAllYears);
	DDX_Control(pDX, IDC_MULTI_PROV_LIST, m_nxlProviderLabel);
	DDX_Control(pDX, IDC_MULTI_EXT_LIST, m_nxlExtLabel);
	DDX_Control(pDX, IDC_SELECTPROV, m_rSelectProv);
	DDX_Control(pDX, IDC_SELECTLOCATION, m_rSelectLoc);
	DDX_Control(pDX, IDC_ALLLOCATIONS, m_rAllLocations);
	DDX_Control(pDX, IDC_ALLPROVS, m_rAllProviders);
	DDX_Control(pDX, IDC_SUMMARY, m_SummaryCheck);
	DDX_Control(pDX, IDC_DETAILED, m_DetailCheck);
	DDX_Control(pDX, IDC_ALLPATS, m_rAllPatients);
	DDX_Control(pDX, IDC_SELECTPAT, m_rSelectPat);
	DDX_Control(pDX, IDC_ALLDATES, m_rAllDates);
	DDX_Control(pDX, IDC_DATERANGE, m_rDateRange);
	DDX_Control(pDX, IDC_DATE_RANGE_OPTIONS, m_rDateRangeOptions);
	DDX_Control(pDX, IDC_USE_EXTENDED, m_UseExtended);
	DDX_Control(pDX, IDC_CURRNAME, m_CurrName);
	DDX_Control(pDX, IDC_NOFILGROUP, m_No_Group_Filter);
	DDX_Control(pDX, IDC_USEFILTER, m_UseFilter);
	DDX_Control(pDX, IDC_USEGROUP, m_UseGroup);
	DDX_Control(pDX, IDC_CLEARSELECT, m_clearSelectBtn);
	DDX_Control(pDX, IDC_KILLREPORT, m_killReportBtn);
	DDX_Control(pDX, IDC_ADDREPORT, m_addReportBtn);
	DDX_Control(pDX, IDC_TO, m_to);
	DDX_Control(pDX, IDC_FROM, m_from);
	DDX_Control(pDX, IDC_DESCFRAME2, m_NameBox);
	DDX_Control(pDX, IDC_INGROUP, m_nxstaticIngroup);
	DDX_Control(pDX, IDC_LABEL_FROMDATE, m_nxstaticLabelFromdate);
	DDX_Control(pDX, IDC_LABEL_TO_DATE, m_nxstaticLabelToDate);
	DDX_Control(pDX, IDC_NEW_BATCH_BTN, m_btnNewBatch);
	DDX_Control(pDX, IDC_DELETEBATCH, m_btnDeleteBatch);
	DDX_Control(pDX, IDC_CREATEMERGE, m_btnCreateMergeGroup);
	DDX_Control(pDX, IDC_EDITREPORT, m_btnEditReports);
	DDX_Control(pDX, IDC_SAVEBATCH, m_btnSaveBatch);
	DDX_Control(pDX, IDC_SAVEBATCH_AS, m_btnSaveBatchAs);
	DDX_Control(pDX, IDC_MULTI_LOC_LIST, m_nxlLocationLabel);
	DDX_Control(pDX, IDC_REPORTS_APPLY_FILTERS, m_btnApplyFilters);
	DDX_Control(pDX, IDC_BTN_LAUNCH_SSRS_REPORTING, m_btnLaunchSSRS);
	//}}AFX_DATA_MAP
}


//	ON_EVENT(CReports, IDC_FROM, 2 /* Change */, OnChangeFrom, VTS_NONE)
//	ON_EVENT(CReports, IDC_TO, 2 /* Change */, OnChangeTo, VTS_NONE)

// (a.walling 2008-05-13 14:58) - PLID 27591 - Use notify handlers for datetimepicker events
BEGIN_MESSAGE_MAP(CReports, CNxDialog)
	//{{AFX_MSG_MAP(CReports)
	ON_BN_CLICKED(IDC_CLEARSELECT, OnClearSelect)
	ON_BN_CLICKED(IDC_ADDREPORT, OnAddReport)
	ON_BN_CLICKED(IDC_KILLREPORT, OnRemoveReport)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_CREATEMERGE, OnCreateMergeGroup)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_FROM, OnChangeFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_TO, OnChangeTo)
	ON_BN_CLICKED(IDC_USEFILTER, OnUsefilter)
	ON_BN_CLICKED(IDC_USEGROUP, OnUsegroup)
	ON_BN_CLICKED(IDC_NOFILGROUP, OnNofilgroup)
	ON_BN_CLICKED(IDC_USE_EXTENDED, OnUseExtended)
	ON_BN_CLICKED(IDC_ALLDATES, OnAllDates)
	ON_BN_CLICKED(IDC_DATERANGE, OnDateRange)
	ON_BN_CLICKED(IDC_ALLPATS, OnAllpats)
	ON_BN_CLICKED(IDC_SELECTPAT, OnSelectpat)
	ON_BN_CLICKED(IDC_DETAILED, OnDetailed)
	ON_BN_CLICKED(IDC_SUMMARY, OnSummary)
	ON_BN_CLICKED(IDC_ALLLOCATIONS, OnAlllocations)
	ON_BN_CLICKED(IDC_SELECTLOCATION, OnSelectlocation)
	ON_BN_CLICKED(IDC_ALLPROVS, OnAllprovs)
	ON_BN_CLICKED(IDC_SELECTPROV, OnSelectprov)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_EDITREPORT, OnEditreport)
	ON_BN_CLICKED(IDC_DATE_RANGE_OPTIONS, OnDateRangeOptions)
	ON_BN_CLICKED(IDC_EDIT_FILTER, OnEditFilter)
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_MESSAGE(NXM_NXLABEL_LBUTTONDOWN, OnLabelClick)
	ON_BN_CLICKED(IDC_NEW_BATCH_BTN, OnNewBatch)
	ON_BN_CLICKED(IDC_SAVEBATCH, SaveBatchChanges)
	ON_BN_CLICKED(IDC_DELETEBATCH, OnDeleteBatch)
	ON_BN_CLICKED(IDC_SAVEBATCH_AS, OnSavebatchAs)
	ON_BN_CLICKED(IDC_REPORT_HELP, OnReportHelp)
	ON_BN_CLICKED(IDC_SEARCH_REPORTS, OnSearchReports)
	ON_BN_CLICKED(IDC_REPORTS_ALL_YEAR, OnReportsAllYear)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_VERIFY_ALL_REPORTS, &CReports::OnBnClickedVerifyAllReports)
	ON_BN_CLICKED(IDC_REPORTS_APPLY_FILTERS, &CReports::OnBnClickedReportsApplyFilters)
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BTN_LAUNCH_SSRS_REPORTING, &CReports::OnBnClickedBtnLaunchSsrsReporting)
END_MESSAGE_MAP()


/*
This isn't necessary anymore
BOOL GetReportTabViewPerm(IN EBuiltInObjectIDs ebio, OUT BOOL &bWithPass)
{
	CPermissions perms = GetCurrentUserPermissions(ebio);
	if (perms.bView) {
		bWithPass = FALSE;
		return TRUE;
	} else if (perms.bViewWithPass) {
		bWithPass = TRUE;
		return TRUE;
	} else {
		bWithPass = FALSE;
		return FALSE;
	}
}
*/

//THIS FUNCTION WAS DRASTICALLY CHANGED BY THE NEW PERMISSIONS
//PERMISSIONS ARE NOW BASED ON REPORTS, NOT TABS, SO MOST OF THIS FUNCTION IS
// NO LONGER NECESSARY - JMM
short CReports::GetDefaultTab()
{
	// See which tab will be our default tab, and whether we need to prompt the user for the password or not
	short nAns = -1;
	//BOOL bWithPass;
	//First, see if we can get into our default tab from the rpeferences, otherwise start at the beginning.
	nAns = (short)GetRemotePropertyInt("MyDefaultTab_Reports", 0, 0, GetCurrentUserName(), true);
	/*BOOL bDefaultAccessed = false;
	switch(nAns) {
	case ReportsModule::PatientsTab:
		bDefaultAccessed = GetReportTabViewPerm(bioReportsPatientTab, bWithPass);
		break;
	case ReportsModule::ContactsTab:
		bDefaultAccessed = GetReportTabViewPerm(bioReportsContactTab, bWithPass);
		break;
	case ReportsModule::MarketingTab:
		bDefaultAccessed = GetReportTabViewPerm(bioReportsMarketingTab, bWithPass);
		break;
	case ReportsModule::InventoryTab:
		bDefaultAccessed = GetReportTabViewPerm(bioReportsInventoryTab, bWithPass);
		break;
	case ReportsModule::SchedulerTab:
	case ReportsModule::OtherTab:
		bDefaultAccessed = GetReportTabViewPerm(bioReportsModule, bWithPass);
		break;
	case ReportsModule::ChargesTab:
	case ReportsModule::PaymentsTab:
	case ReportsModule::FinancialTab:
		bDefaultAccessed = GetReportTabViewPerm(bioReportsFinancialLow, bWithPass);
		break;
	case ReportsModule::ASCTab:
		bDefaultAccessed = GetReportTabViewPerm(bioReportsASCTab, bWithPass);
		break;
	case ReportsModule::AdministrationTab:
		bDefaultAccessed = GetReportTabViewPerm(bioReportsAdminTab, bWithPass);
		break;
	default:
		//What?  Just assert, and do it the old way.
		ASSERT(FALSE);
		break;
	}
	
	if(!bDefaultAccessed) {
		//We couldn't get to our default tab, let's go back to the old way of doing things.

			 if (GetReportTabViewPerm(bioReportsPatientTab, bWithPass))		nAns = ReportsModule::PatientsTab;
		else if (GetReportTabViewPerm(bioReportsContactTab, bWithPass))		nAns = ReportsModule::ContactsTab;
		else if (GetReportTabViewPerm(bioReportsMarketingTab, bWithPass))	nAns = ReportsModule::MarketingTab;
		else if (GetReportTabViewPerm(bioReportsInventoryTab, bWithPass))	nAns = ReportsModule::InventoryTab;
		//else if (GetReportTabViewPerm(bioReportsModule/*bioReportsSchedulerTab*//*, bWithPass))	nAns = ReportsModule::SchedulerTab;*/
		//else if (GetReportTabViewPerm(bioReportsFinancialLow/*bioReportsChargesTab*/, bWithPass))		nAns = ReportsModule::ChargesTab;
		//else if (GetReportTabViewPerm(bioReportsFinancialLow/*bioReportsPaymentsTab*/, bWithPass))	nAns = ReportsModule::PaymentsTab;
		/*else if (GetReportTabViewPerm(bioReportsFinancialLow, bWithPass))	nAns = ReportsModule::FinancialTab;
		else if (GetReportTabViewPerm(bioReportsASCTab,	bWithPass))			nAns = ReportsModule::ASCTab;
		else if (GetReportTabViewPerm(bioReportsAdminTab, bWithPass))		nAns = ReportsModule::AdministrationTab;
		//else if (GetReportTabViewPerm(bioReportsModule*//*bioReportsOtherTab*//*, bWithPass))		nAns = ReportsModule::OtherTab;*/
		//else {
			// No tab  can be the default
		//	nAns = -1;
		//}
	//}

	// If we're supposed to prompt for a password
	/*if (nAns != -1 && bWithPass) {
		// See if we already prompted
		// This is a little sketchy, but it's the only way without slightly changing the reports module architecture
		BOOL bAlreadyPromptedForPassword;
		{
			// The assumption here is that the only way to get to the reports module is by 
			// checking the permissions for it, and if the permissions were checked properly 
			// and the password was required, then the password would have been demanded.  
			// This means that if we made it to the point where we're trying to determine 
			// the default tab, then we shouldn't prompt them again.
			if (GetCurrentUserPermissions(bioReportsModule).bViewWithPass) {
				bAlreadyPromptedForPassword = TRUE;
			} else {
				bAlreadyPromptedForPassword = FALSE;
			}
		}

		// If we didn't already prompt for the password, prompt for it now
		if (!bAlreadyPromptedForPassword) {
			if (!CheckCurrentUserPassword()) {
				// They failed the password prompt, so we'll have to return failure
				nAns = -1;
			}
		}
	}*/

	return nAns;
}

/////////////////////////////////////////////////////////////////////////////
// CReports message handlers

BOOL CReports::OnInitDialog() 
{
	extern CPracticeApp theApp;

	CNxDialog::OnInitDialog();

	try {

		CurrReport = NULL;

		m_nxlProviderLabel.SetColor(0x00DEB05C);
		m_nxlProviderLabel.SetText("");
		m_nxlProviderLabel.SetType(dtsHyperlink);
		m_nxlExtLabel.SetColor(0x00DEB05C);
		m_nxlExtLabel.SetText("");
		m_nxlExtLabel.SetType(dtsHyperlink);
		//(e.lally 2008-09-05) PLID 6780 - Initialize the hyperlink label
		m_nxlLocationLabel.SetColor(0x00DEB05C);
		m_nxlLocationLabel.SetText("");
		m_nxlLocationLabel.SetType(dtsHyperlink);

		m_brush.CreateSolidBrush(PaletteColor(0x00DEB05C));
		// (j.gruber 2009-12-28 16:54) - PLID 19189 - apply filters
		m_btnApplyFilters.AutoSet(NXB_MODIFY);

		GetDlgItem(IDC_CURRNAME)->SetFont(&theApp.m_subtitleFont);

		//Attach to the NxTab - probably should have a global function
		CWnd *pWnd = GetDlgItem(IDC_TAB);
		if (pWnd)
			m_tab = pWnd->GetControlUnknown();
		else m_tab = NULL;

		if (m_tab == NULL)
		{
			HandleException(NULL, "Could not create tab control", __LINE__, __FILE__);
			return FALSE;
		}
		// (s.dhole 2012-04-02 09:47) - PLID 49341 Change count
		m_tab->Size = 13;
		m_tab->TabWidth = 13;

		Modules::Tabs& tabs = g_Modules[Modules::Reports]->Reset().GetTabs();

		ASSERT (ReportsModule::Last_Tab == m_tab->Size);
		for (int i = 0; i < ReportsModule::Last_Tab; i++) {
			m_tab->Label[i] = tabs[i]->Name();
			// (j.jones 2016-04-26 15:30) - NX-100214 - added ShortLabel support
			m_tab->ShortLabel[i] = tabs[i]->ShortName();
		}

		//DRT 4/18/2008 - PLID 29711 - Set the text color of the tabs.
		// (j.jones 2016-04-18 17:46) - NX-100214 - the NxTab control now defaults
		// to the value of CNexTechDialog::GetDefaultNxTabTextColor()

		long nTabsShown = ShowTabs();

		//check to see if we are at zero
		if (nTabsShown == 0) {
			//give a messagebox and return
			MsgBox("You do not have access to view any reports, please see your office manager");
		}

		//Let's enable some tooltip action
		//We need to do this right away, because some of the other code may call functions that will try to do
		//tool tip stuff, which can cause problems.
		if(m_ToolCtrl.Create(this)){
			m_ToolCtrl.SetDelayTime(TTDT_AUTOPOP, 32767);
			if(m_ToolCtrl.AddTool(&m_NameBox, "None Selected")){
				m_strCurrentToolText = "None Selected";
				m_ToolCtrl.Activate(TRUE);
				m_bToolTipsActive = true;
			}
			else{
				m_bToolTipsActive = false;
			}
		}
		else{
			m_bToolTipsActive = false;
		}

		// (z.manning 2009-12-07 10:26) - Show the verify all reports button if we should.
#ifdef ENABLE_VERIFY_ALL_REPORTS
		GetDlgItem(IDC_VERIFY_ALL_REPORTS)->ShowWindow(SW_SHOW);
#endif

		//when we load a report, it will change these to the current date. But it should never
		//show 01/30/2000, so change it here too
		m_from.SetValue((COleVariant)COleDateTime::GetCurrentTime());
		m_to.SetValue((COleVariant)COleDateTime::GetCurrentTime());

		// (j.jones 2010-06-14 17:01) - PLID 39117 - added bulk caching
		g_propManager.CachePropertiesInBulk("Reports-Number", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'ReportFilters_IncludeInactivePatients' OR "
			"Name = 'ReportFilters_IncludeInactiveProviders' OR "
			"Name = 'ReportFilters_IncludeInactiveLocations' OR "
			"Name = 'SttmntUse70Version' OR "
			"Name = 'SttmntEnvelope' OR "
			"Name = 'SttmntUseGuarantor' OR "
			"Name = 'SttmntUseDocName' OR "
			"Name = 'SttmntUseComma' OR "
			"Name = 'SttmntShowDiag' OR "
			"Name = 'SttmntAge' OR "
			"Name = 'SttmntShowFooter' OR "
			"Name = 'SttmntUseDocAddress' OR "
			"Name = 'SttmntHidePrePayments' OR "
			"Name = 'SttmntCombineBillsAfterXDaysOld' OR "
			"Name = 'SttmntDaysOld' OR "
			"Name = 'SttmntShowLastPayInfo' OR "
			"Name = 'SttmntUseRemit' OR "
			"Name = 'SttmntRemitLocation' OR "
			"Name = 'SttmntAcceptVisa' OR "
			"Name = 'SttmntAcceptMstrCard' OR "
			"Name = 'SttmntAcceptDiscover' OR "
			"Name = 'SttmntAcceptAmex' OR "
			"Name = 'ResShowLocName' OR "
			"Name = 'ReceiptShowChargeInfo' OR "
			"Name = 'ReceiptShowTax' OR "
			"Name = 'Reports_SuppressAR' OR "
			"Name = 'Rpt_ReturnUseTaxTotals' OR "
			"Name = 'Rpt_UseReturnProduct' OR "
			"Name = 'InvItem_OrderByOnHandAmount' OR "
			"Name = 'AuditTrail_ARIssues_OnlyPastItems' OR "
			"Name = 'FinancialSummaryQueryAR' OR "
			"Name = 'DailyBatchDefaultDate' OR "
			"Name = 'MyDefaultTab_Reports' OR "
			// (j.jones 2011-04-12 10:46) - PLID 31219 - added ability to show all charges
			// on any bills with balances, when on the summary statement
			"Name = 'SttmntIncludePaidCharges' OR "
			// (j.luckoski 2012-10-03 17:03) - PLID 24684 - Batch prefs for saving provider filter for batch, either current or saved
			// (j.luckoski 2012-10-03 17:04) - PLID 52949 - Pref to save prov filter for current batch
			"Name = 'ReportFilters_RememberProviderFilter' OR "
			// (j.luckoski 2012-10-03 17:04) - PLID 53003 - Pref to save prov filter for saved batch
			"Name = 'ReportFilters_SaveProviderFilter' "
			"OR Name = 'Reports_ProjectedSurgeryIncome_ExcludeFinishedLadders' "	// (j.jones 2013-03-12 09:31) - PLID 55156
			"OR Name = 'Rpt_Show_Adjs_In_Appl_Proc_Comm' "	// (j.jones 2013-06-05 11:22) - PLID 56786 - added this missing cached pref
			"OR Name = 'ServerSideCursorReport' " //(r.wilson 9/17/2013) PLID 58651 - Cache this value
			"OR Name = 'SttmntHideChargebacks' " //TES 7/17/2014 - PLID 62563
			"OR Name = 'BlockLabReportsFromLoadingSignatureImages' "	// (r.goldschmidt 2015-01-20 14:05) - PLID 63982
			"OR Name = 'BatchPayments_EnableCapitation' " // (r.gonet 2015-11-12 03:04) - PLID 67466
			"OR Name = 'SupportTempAllFinQReport' " // (c.haag 2016-03-01) - PLID 68460
			"OR Name = 'TempAllFinQTimeout' " // (c.haag 2016-03-01) - PLID 68565
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("Reports-Text", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'SttmntPaymentDesc' OR "
			"Name = 'SttmntAdjustmentDesc' OR "
			"Name = 'SttmntRefundDesc' OR "
			"Name = 'ReceiptCustomInfo' OR "
			"Name = 'SttmntExtendedFilter' "
			")",
			_Q(GetCurrentUserName()));

		g_propManager.CachePropertiesInBulk("Reports-Memo", propMemo,
			"(Username = '<None>' OR Username = '%s') AND ("
			// (j.luckoski 2012-10-03 17:05) - PLID 24684 - We are storing more in the current batch, so lets make it a memo
			// to store more information.
			"Name = 'Reports.Batches.CurrentBatch' OR "
			"Name = 'SttmntName' OR "
			"Name = 'SttmntCallMe' OR "
			"Name = 'SttmntText' OR "
			"Name = 'Sttmnt30DayNote' OR "
			"Name = 'Sttmnt60DayNote' OR "
			"Name = 'Sttmnt90DayNote' OR "
			"Name = 'Sttmnt90+DayNote' "
			")",
			_Q(GetCurrentUserName()));

		// (j.jones 2010-06-14 16:53) - PLID 39117 - cache the values for whether we include inactive
		// patients, providers, or locations in the dropdown filters
		m_bFilterShowInactivePatients = GetRemotePropertyInt("ReportFilters_IncludeInactivePatients", 0, 0, "<None>", true) == 1;
		m_bFilterShowInactiveProviders = GetRemotePropertyInt("ReportFilters_IncludeInactiveProviders", 0, 0, "<None>", true) == 1;
		m_bFilterShowInactiveLocations = GetRemotePropertyInt("ReportFilters_IncludeInactiveLocations", 0, 0, "<None>", true) == 1;

		// Attach to the nxdatalist controls and bind to our data but don't requery
		m_ReportList = BindNxDataListCtrl(IDC_REPORTSAVAIL, false);
		m_SelectList = BindNxDataListCtrl(IDC_SELECTLIST, false);
		m_ExtFilterList = BindNxDataListCtrl(IDC_EXT_FILTER, false);
		m_pDateOptions = BindNxDataListCtrl(IDC_DATE_OPTIONS, false);
		m_BatchList = BindNxDataListCtrl(IDC_COMBO_REPORTBATCHES, true);
		IRowSettingsPtr pRow = m_BatchList->GetRow(-1);
		pRow->PutValue(0,USER_BATCH_NAME);
		//TES 12/18/2008 - PLID 32514 - They're not allowed to use report batches if they 
		// have the Scheduler Standard version
		CString strCurrentBatch;
		if(g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
			// (j.luckoski 2012-10-03 17:10) - PLID 24684 - Made the current batch a memo
			strCurrentBatch = GetRemotePropertyMemo("Reports.Batches.CurrentBatch", "", 0, GetCurrentUserName(), false);
		}
		pRow->PutValue(1,(LPCTSTR)strCurrentBatch);
		m_BatchList->CurSel = m_BatchList->AddRow(pRow);
		GetDlgItem(IDC_DELETEBATCH)->EnableWindow(FALSE);
		m_pReportTypeList = BindNxDataListCtrl(IDC_REPORT_TYPE_LIST, false);

		// (j.gruber 2008-07-11 13:08) - PLID 30692 - date quick filter
		m_pDateOptionList = BindNxDataList2Ctrl(IDC_REPORTS_QUICK_DATE_FILTER, false);
		NXDATALIST2Lib::IRowSettingsPtr pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvAll);
		pRow2->PutValue(1, _variant_t("All"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvOneYear);
		pRow2->PutValue(1, _variant_t("One Year"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvCustom);
		pRow2->PutValue(1, _variant_t("Custom"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);
		
		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvSeparator);
		pRow2->PutValue(1, _variant_t("--------------------------------"));
		pRow2->PutBackColorSel(RGB(255,255,255));
		pRow2->PutForeColorSel(RGB(0,0,0));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvToday);
		pRow2->PutValue(1, _variant_t("Today"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);		

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvThisWeek);
		pRow2->PutValue(1, _variant_t("This Week"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvThisMonth);
		pRow2->PutValue(1, _variant_t("This Month"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvThisQuarter);
		pRow2->PutValue(1, _variant_t("This Quarter"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvThisYear);
		pRow2->PutValue(1, _variant_t("This Year"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);
		
		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvSeparator);
		pRow2->PutValue(1, _variant_t("--------------------------------"));
		pRow2->PutBackColorSel(RGB(255,255,255));
		pRow2->PutForeColorSel(RGB(0,0,0));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvThisMonthToDate);
		pRow2->PutValue(1, _variant_t("This Month To Date"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvThisQuarterToDate);
		pRow2->PutValue(1, _variant_t("This Quarter To Date"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvThisYearToDate);
		pRow2->PutValue(1, _variant_t("This Year To Date"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvSeparator);
		pRow2->PutValue(1, _variant_t("--------------------------------"));
		pRow2->PutBackColorSel(RGB(255,255,255));
		pRow2->PutForeColorSel(RGB(0,0,0));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		// (j.gruber 2010-09-08 09:50) - PLID 37425 - added Yesterday
		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvYesterday);
		pRow2->PutValue(1, _variant_t("Yesterday"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvLastWeek);
		pRow2->PutValue(1, _variant_t("Last Week"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvLastMonth);
		pRow2->PutValue(1, _variant_t("Last Month"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvLastQuarter);
		pRow2->PutValue(1, _variant_t("Last Quarter"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		pRow2 = m_pDateOptionList->GetNewRow();
		pRow2->PutValue(0, (long)erdvLastYear);
		pRow2->PutValue(1, _variant_t("Last Year"));
		m_pDateOptionList->AddRowAtEnd(pRow2, NULL);

		// Set button icons
		m_addReportBtn.AutoSet(NXB_RIGHT);
		m_killReportBtn.AutoSet(NXB_LEFT);
		m_clearSelectBtn.AutoSet(NXB_LLEFT);
		// (z.manning, 04/28/2008) - PLID 29807 - Set more button styles
		m_btnNewBatch.AutoSet(NXB_NEW);
		m_btnDeleteBatch.AutoSet(NXB_DELETE);
		m_btnCreateMergeGroup.AutoSet(NXB_NEW);
		m_btnEditReports.AutoSet(NXB_MODIFY);
		m_btnSaveBatch.SetIcon(IDI_SAVE);
		m_btnSaveBatchAs.SetIcon(IDI_SAVE);

		m_btnLaunchSSRS.AutoSet(NXB_SSRS);
		
		// Flip to the default tab
		short nTabIndex = GetDefaultTab();
		if (nTabIndex != -1) {
			m_tab->CurSel = nTabIndex;
	//		m_RepCategory.SetActiveTab(nTabIndex);
			DoInitLists(-1);
		}

		//Hide the Report list for the statement
		GetDlgItem(IDC_REPORT_TYPE_LIST)->ShowWindow(SW_HIDE);

		// Attach our variables to the NxDataList controls

		// (j.jones 2010-06-14 17:23) - PLID 39117 - no longer requeries by default
		m_PatSelect = BindNxDataListCtrl(IDC_PATSELECT, false);
		m_PatSelect->PutSnapToVertically(VARIANT_FALSE);
		m_nSelectedPatientID = GetActivePatientID();

		// (j.jones 2010-06-14 17:23) - PLID 39117 - show/hide inactive patients
		CString strPatWhere = "PatientsT.PersonID <> -25 AND PatientsT.CurrentStatus <> 4 ";
		if(!m_bFilterShowInactivePatients) {
			strPatWhere += "AND PersonT.Archived = 0";
		}
		m_PatSelect->PutWhereClause((LPCTSTR)strPatWhere);
		//m_PatSelect->Requery();
		
		// (a.walling 2011-08-04 14:36) - PLID 44788 - Ensure the initial patient fits in the filters
		if (!ReturnsRecordsParam("SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE {CONST_STR} AND PersonT.ID = {INT}", strPatWhere, m_nSelectedPatientID)) {
			ADODB::_RecordsetPtr prs = CreateRecordset("SELECT TOP 1 ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE %s ORDER BY Last, First, Middle", strPatWhere);
			if (prs->eof) {
				m_nSelectedPatientID = -1;
			} else {
				m_nSelectedPatientID = AdoFldLong(prs, "ID");
			}
		}
		
		if (m_nSelectedPatientID == -1) {
			m_PatSelect->CurSel = -1;
			m_PatSelect->PutComboBoxText("");
		} else {
			m_PatSelect->PutComboBoxText((LPCTSTR)GetExistingPatientName(m_nSelectedPatientID));
		}

		m_ProvSelect = BindNxDataListCtrl(IDC_PROVSELECT, false);

		//DRT 5/25/2004 - PLID 12561 - Requery properly in 1 place
		RequeryProviderList();

		//(e.lally 2008-09-05) PLID 6780 - Do not auto-requery, we want to set extra options first
		m_LocationSelect = BindNxDataListCtrl(IDC_LOCATIONSELECT, false);
		RequeryLocationList("Managed = 1");

		m_GroupSelect = BindNxDataListCtrl(IDC_GROUPDL, false);

		//DRT 7/3/2007 - PLID 26540 - This needs to happen after the datalists have been bound, LoadFilters() is called
		//	inside here, and it can make attempts to reference several of these lists.
		//TES 12/18/2008 - PLID 32514 - They're not allowed to use batches if they're on the
		// Scheduler Standard version
		if(g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
			LoadBatch("CurrentBatch", GetCurrentUserName());
		}

		//DRT 7/8/2005 - PLID 6308 - Added permissions for the report batch functionality.  Disable
		//	the various buttons depending on their permissions.
		CPermissions perms = GetCurrentUserPermissions(bioReportBatch);
		int nAccess = (perms & (sptRead|sptReadWithPass));
		if(!(perms & (sptRead|sptReadWithPass))) {
			//Read access controls whether they can use the dropdown of batches or not.
			GetDlgItem(IDC_COMBO_REPORTBATCHES)->EnableWindow(FALSE);
		}
		if(!(perms & (sptWrite|sptWriteWithPass))) {
			//Write access controls the "Save" button only
			GetDlgItem(IDC_SAVEBATCH)->EnableWindow(FALSE);
		}
		if(!(perms & (sptCreate|sptCreateWithPass))) {
			//Create access controls the "New" and "Save As" buttons"
			GetDlgItem(IDC_SAVEBATCH_AS)->EnableWindow(FALSE);
			GetDlgItem(IDC_NEW_BATCH_BTN)->EnableWindow(FALSE);
		}
		if(!(perms & (sptDelete|sptDeleteWithPass))) {
			GetDlgItem(IDC_DELETEBATCH)->EnableWindow(FALSE);
		}

		if (!(GetCurrentUserPermissions(bioEnterpriseReporting) & sptView)) {
			GetDlgItem(IDC_BTN_LAUNCH_SSRS_REPORTING)->EnableWindow(FALSE);
		}

		/********************************************************************************************************
		//TES 9/11/03: If they don't have a license for letter writing, don't allow them to edit filters from
		//here.  Note: We are intentionally allowing them to use any filters they already happen to have, because
		//a.) if they have any that they've been using, it's sort of our fault that we let them create them, so 
		//we won't take them away, and b.) hopefully this will tempt people into buying letter writing.
		/********************************************************************************************************/
		if(!g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent)) {
			GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(FALSE);
		}
		
		ResetFilters();

		


	} NxCatchAll("Error initializing reports.");

	return TRUE;
}



BOOL CReports::CheckReportTabAvailability(const short nTabIndex) {

	CString strCategory = "";

	switch (nTabIndex) {
		// (a.walling 2010-12-09 09:14) - PLID 40444 - This was using the incorrect category name, even before my changes. Ack!
		case ReportsModule::PatientsTab:
			strCategory = "PatientP";
		break;

		// (a.walling 2010-12-09 09:14) - PLID 40444 - This was using the incorrect category name, even before my changes. Ack!
		case ReportsModule::ContactsTab:	
			strCategory = "OthrContactP";
		break;

		case ReportsModule::MarketingTab:		
			strCategory = "MarketP";
		break;

		case ReportsModule::InventoryTab:		
			strCategory = "InventoryP";
		break;

		case ReportsModule::SchedulerTab:		
			strCategory = "ScheduleP";
		break;

		case ReportsModule::ChargesTab:		
			strCategory = "ChargesP";
		break;

		case ReportsModule::PaymentsTab:		
			strCategory = "PaymentsP";
		break;

		case ReportsModule::FinancialTab:		
			strCategory = "FinancialP";
		break;

		case ReportsModule::ASCTab:			
			strCategory = "ASCP";
		break;

		case ReportsModule::AdministrationTab:	
			strCategory = "AdminP";
		break;

		case ReportsModule::OtherTab:			
			strCategory = "OtherP";
		break;

		// (a.walling 2010-12-09 09:14) - PLID 40444 - This was using the incorrect category name. I did not change this as part of this PL 
		// item, but before this PL item this tab enum would never be passed into this funciton.
		case ReportsModule::PracAnalTab:			
			strCategory = "PracAnalP";
		break;
		// (s.dhole 2012-03-30 17:19) - PLID 49341
		case ReportsModule::OpticalTab:			
			strCategory = "OpticalP";
		break;

	}		
		//loop through all the reports and check whether any are available, if one
		// is available then show the tab, otherwise, don't
	extern CMap<long, long, long, long> g_mapAvailReports;
	
	long nTest = g_mapAvailReports.GetCount();
	for (long i=0; i<gcs_nKnownReportCount; i++) {
		if (gcs_aryKnownReports[i].strCategory.CompareNoCase(strCategory) == 0) {
			long nReportID = gcs_aryKnownReports[i].nID;
			if (g_mapAvailReports.Lookup(nReportID, nReportID)) {
				return TRUE;
			}
		}
	}

	//if we got here, no reports are usable
	return FALSE;
}

long CReports::ShowTabs()
{
	short nSize = m_tab->GetTabWidth();

	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to update the tabs
	Modules::Tabs& tabs = g_Modules[Modules::Reports]->GetTabs();
	std::for_each(tabs.begin(), tabs.end(), boost::bind(&Modules::NxTabUpdater, m_tab, _1));

	int nHiddenTabs = 
		std::count_if(tabs.begin(), tabs.end(), 
			!bind(&Modules::Element::Visible,
				bind(&Modules::Tabs::value_type::second, _1)
			)
		);

	nSize -= nHiddenTabs;

	m_tab->TabWidth = nSize;
	return nSize;
}

void CReports::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);
	
	SetControlPositions();
}

void CReports::OnCancel() 
{
//	CNxDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CReports, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CReports)
	ON_EVENT(CReports, IDC_REPORTSAVAIL, 2 /* SelChanged */, OnSelChangedReports, VTS_I4)
	ON_EVENT(CReports, IDC_SELECTLIST, 2 /* SelChanged */, OnSelChangedSelected, VTS_I4)
	ON_EVENT(CReports, IDC_REPORTSAVAIL, 3 /* DblClick */, OnDblClickReportsavail, VTS_I4 VTS_I4)
	ON_EVENT(CReports, IDC_SELECTLIST, 3 /* DblClick */, OnDblClickSelectlist, VTS_I4 VTS_I4)
	ON_EVENT(CReports, IDC_PROVSELECT, 2 /* SelChanged */, OnSelChangedProvselect, VTS_I4)
	ON_EVENT(CReports, IDC_PATSELECT, 2 /* SelChanged */, OnSelChangedPatselect, VTS_I4)
	ON_EVENT(CReports, IDC_PATSELECT, 26 /* DroppingDown */, OnDroppingDownPatselect, VTS_NONE)
	ON_EVENT(CReports, IDC_GROUP, 2 /* Change */, OnChangeGroup, VTS_NONE)
	ON_EVENT(CReports, IDC_GROUPDL, 2 /* SelChanged */, OnSelChangedGroup, VTS_I4)
	ON_EVENT(CReports, IDC_TAB, 1 /* SelectTab */, OnSelectTab, VTS_I2 VTS_I2)
	ON_EVENT(CReports, IDC_EXT_FILTER, 16 /* SelChosen */, OnSelChosenExtFilter, VTS_I4)
	ON_EVENT(CReports, IDC_COMBO_REPORTBATCHES, 16 /* SelChosen */, OnSelChosenBatchList, VTS_I4)
	ON_EVENT(CReports, IDC_GROUPDL, 16 /* SelChosen */, OnSelChosenGroupdl, VTS_I4)
	ON_EVENT(CReports, IDC_GROUPDL, 18 /* RequeryFinished */, OnRequeryFinishedGroupdl, VTS_I2)
	ON_EVENT(CReports, IDC_DATE_OPTIONS, 16 /* SelChosen */, OnSelChosenDateOptions, VTS_I4)
	ON_EVENT(CReports, IDC_PROVSELECT, 16 /* SelChosen */, OnSelChosenProvselect, VTS_I4)
	ON_EVENT(CReports, IDC_PROVSELECT, 1 /* SelChanging */, OnSelChangingProvselect, VTS_PI4)
	ON_EVENT(CReports, IDC_REPORT_TYPE_LIST, 16 /* SelChosen */, OnSelChosenReportTypeList, VTS_I4)
	ON_EVENT(CReports, IDC_REPORTS_QUICK_DATE_FILTER, 16 /* SelChosen */, OnSelChosenReportsQuickDateFilter, VTS_DISPATCH)
	ON_EVENT(CReports, IDC_REPORTS_QUICK_DATE_FILTER, 1 /* SelChanging */, OnSelChangingReportsQuickDateFilter, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CReports, IDC_LOCATIONSELECT, 16 /* SelChosen */, OnSelChosenLocationSelect, VTS_I4)
	ON_EVENT(CReports, IDC_LOCATIONSELECT, 1 /* SelChanging */, OnSelChangingLocationSelect, VTS_PI4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CReports, IDC_COMBO_REPORTBATCHES, 1, CReports::OnSelChangingComboReportbatches, VTS_PI4)
	ON_EVENT(CReports, IDC_REPORT_TYPE_LIST, 1 /* SelChanging */, OnSelChangingReportType, VTS_PI4)
END_EVENTSINK_MAP()

void CReports::OnAddReport() 
{
	// We need to have access to both NxDataLists
	ASSERT(m_SelectList != NULL);
	ASSERT(m_ReportList != NULL);

	try {
		long nCurSel = m_ReportList->CurSel;
		if (nCurSel >= 0) {
			// Only try to add the report if there's one selected
			_variant_t varReportId = m_ReportList->GetValue(nCurSel, 0);
			if (varReportId.vt == VT_I4) {
				// Get the report info

				long nRepID = VarLong(varReportId);

				//check to see if it is a statement report
				if (nRepID == 169 || nRepID == 337 || nRepID == 434 || nRepID == 435 || nRepID == 483 || nRepID == 484) {
				//check to see if they are using the 7.0 reports
				if (GetRemotePropertyInt("SttmntUse70Version")) {
					switch(nRepID) {
						case 169:
							nRepID = 353;
						break;
						case 337:
							nRepID = 355;
						break;
						case 434:
							nRepID = 436;
							break;
						case 435:
							nRepID = 437;
							break;
						case 483:
							nRepID = 485;
							break;
						case 484:
							nRepID = 486;
							break;
						default:
						MessageBox("Error determining statement Type");
						break;
					}
					
				}
			}
				CReportInfo *pRep = FindReport(nRepID);
				ASSERT(pRep);
				if (pRep) {

					//PLID 14613 - check the permissions here also
					if (! CheckCurrentUserReportAccess(pRep->nID, FALSE, TRUE)) {
						return;
					}

					// Show that this is the selected report
					SetDlgItemText(IDC_CURRNAME, pRep->strPrintName);
					
					//Set the new tooltip.
					if(m_bToolTipsActive){
						m_ToolCtrl.DelTool(&m_NameBox, IDC_DESCFRAME2);
						m_ToolCtrl.AddTool(&m_NameBox, pRep->GetDescription());
						m_strCurrentToolText = pRep->GetDescription();
					}
					
					// Add the selected report to the list
					AddReport(pRep->nID, TRUE);
					m_bModified = TRUE;
				} else {
					AfxThrowNxException("Could not find report %li with ID %li.", nCurSel, pRep->nID);
				}
			} else {
				AfxThrowNxException("Invalid data type in Report list.");
			}
		}
	} NxCatchAll("CReports::OnAddReports Error 3.  Could not select report.");
}

CReportInfo *CReports::LoadReport(const CReportInfo &repCopyFrom, bool bAutoReload /*= false*/)
{
	// Store the id because we use it a lot
	long nRepId = repCopyFrom.nID;

	// We have to check to make sure the given report has a valid ID
	ASSERT(nRepId > 0);
	if (nRepId <= 0 /*|| nRepId > TODO: determine a max ID */) {
		return NULL;
	}
	
	// See if we've gotten this report yet
	CReportInfo *pRep = (CReportInfo *)m_lstReports[(void *)nRepId];
	if (pRep && bAutoReload) {
		// A report with this ID has already been loaded but 
		// the caller wants to reload it from the given report
		delete pRep;
		pRep = NULL;
		m_lstReports[(void *)nRepId] = NULL;
	}
	// At this point we either have a valid report or not.  if not, load it from repCopyFrom
	if (!pRep) {
		pRep = new CReportInfo(repCopyFrom);
		m_lstReports[(void *)nRepId] = (void *)pRep;
	}
	
	// Either we already loaded the report or we found it in the list
	// If pRep is still NULL at this point, then nReportId refers to an unknown report
	ASSERT(pRep);
	return pRep;
}

CReportInfo * CReports::FindReport(long nReportId)
{
	// See if we've gotten this report yet
	CReportInfo *pRep = (CReportInfo *)m_lstReports[(void *)nReportId];
	if (!pRep) {
		// Get the report from the master list
		for (long i=0; i<gcs_nKnownReportCount; i++) {
			if (gcs_aryKnownReports[i].nID == nReportId) {
				pRep = new CReportInfo(gcs_aryKnownReports[i]);
				m_lstReports[(void *)nReportId] = (void *)pRep;
			}
		}
	}
	
	// Either we already loaded the report or we found it in the list
	// If pRep is still NULL at this point, then nReportId refers to an unknown report
	ASSERT(pRep);
	return pRep;
}

// (c.haag 2009-01-13 11:46) - PLID 32683 - This is similar to AddReport, but OnAddReport and all
// its internal checks are done beforehand. This function assumes the active tab has the report.
void CReports::AddReportFromExternalModule(long nReportID)
{
	if (NULL != m_ReportList) {
		if (sriNoRow != m_ReportList->SetSelByColumn(0, nReportID)) {
			OnAddReport();
		} else {
			// Could not find the report
		}
	} else {
		// The report list hasn't been initialized yet
	}
}

void CReports::AddReport(long nReportID /* = -1 */, BOOL bAutoSelect /*= TRUE*/) 
{
	long nTrueId;

	if (nReportID == -1) {
		if (m_ReportList) {
			// Get the id from the selected row of the report list
			_variant_t varReportId = m_ReportList->GetValue(m_ReportList->GetCurSel(), 0);
			if (varReportId.vt != VT_I4) return;

			nTrueId = varReportId.lVal;
		} else {
			ASSERT(FALSE);
			return;
		}
	} else {
		// Use the given id
		nTrueId = nReportID;
	}

	ASSERT(m_SelectList != NULL);

	try {
		// Grab the requested CReportInfo object
		CReportInfo* pReport = FindReport(nReportID);
		if (pReport) {
			// Make sure this report is not already in the select list
			if (m_SelectList->FindByColumn(0, nReportID, 0, bAutoSelect) >= 0) {
				// Found and automatically selected so do nothing
			} else {
				// Not found so add it to the "selected" list and highlight it
				IRowSettingsPtr pRow = m_SelectList->Row[-1];
				pRow->Value[0] = pReport->nID;
				pRow->Value[1] = _bstr_t(pReport->strPrintName);
				long nIndex = m_SelectList->AddRow(pRow);
				if (bAutoSelect) {
					m_SelectList->CurSel = nIndex;
				}

				//check the database and see if there is a default for this report

				//check to see if it is a statement report
				/*_RecordsetPtr rsDefault;
				if (pReport->nID == 169 || pReport->nID == 337) {
					if (GetRemotePropertyInt("SttmntUse70Version")) {
						long nID;
						switch(pReport->nID) {
							case 169:
								nID = 353;
							break;
							case 337:
								nID = 355;
							break;
							default:
							MessageBox("Error determining statement Type");
						break;
						}
						rsDefault = CreateRecordset("SELECT CustomReportID, DetailOption, DateOption FROM DefaultReportsT WHERE ID = %li", nID);
					}
				}
				else {
*/
					
				//}
				
				_RecordsetPtr rsDefault = CreateRecordset("SELECT CustomReportID, DetailOption, DateOption FROM DefaultReportsT WHERE ID = %li", pReport->nID);
				if (! rsDefault->eof) {
					pReport->nDefaultCustomReport = AdoFldLong(rsDefault, "CustomReportID", -1);
					pReport->nDetail = (short)AdoFldLong(rsDefault, "DetailOption", pReport->nDetail);
					pReport->nDateFilter = (short)AdoFldLong(rsDefault, "DateOption", pReport->nDateFilter);
				}
			}
			if (bAutoSelect) {
				// Load the filters for the given report
				LoadFilters(pReport);
			}
		} else {
			HandleException(NULL, "CReports::AddReport 5", __LINE__, __FILE__);
		}
	} NxCatchAll("CReports::AddReport 3");
}

void CReports::RemoveReport(long nReportID /* = -1 */)
{
	ASSERT(m_SelectList != NULL);
	if (m_SelectList) {
		try {
			// Are we removing the currently selected item (-1) 
			// or are we removing the item with the specified ID
			long nIndex = -1;
			if (nReportID == -1) {
				// The currently selected item
				nIndex = m_SelectList->CurSel;
			} else {
				// The given ID
				nIndex = m_SelectList->FindByColumn(0, nReportID, 0, FALSE);
			}

			// If we found the index for the item we're removing, go ahead and remove it
			if (nIndex >= 0) {

				// (j.jones 2010-10-25 16:14) - PLID 41001 - calculate the next report to select
				long nNextIndex = -1;
				//try to get the next report in the list
				if(nIndex < m_SelectList->GetRowCount()-1) {
					//the current index is not the last row,
					//and when we remove this row, the current
					//'next' row will have the same index
					nNextIndex = nIndex;
				}
				//try to get the previous report in the list
				else if(nIndex > 0) {
					//the current index is not the first row,
					//so use the previous index
					nNextIndex = nIndex - 1;
				}
				else {
					//shouldn't be possible unless this is the only selected row,
					//in which case the new selection will be -1
					nNextIndex = -1;
				}

				m_SelectList->RemoveRow(nIndex);

				//load the filters for the newly selected report, if there is one.
				m_SelectList->PutCurSel(nNextIndex);
				if(nNextIndex != -1) {
					//make sure this report is visible
					m_SelectList->EnsureRowVisible(nNextIndex);
				}
				OnSelChangedSelected(m_SelectList->GetCurSel());
			}
		} NxCatchAll("CReports::RemoveReport 2");
	} else {
		HandleException(NULL, "CReports::RemoveReport 3:  Could not attach to the "
			"select list.  The selected report has not been removed.", __LINE__, __FILE__);
	}
	m_bModified = TRUE;
}

void CReports::OnRemoveReport() 
{
	try {
		// Remove the currently selected report
		RemoveReport();
		m_bModified = TRUE;

		// Clear out the filters because nothing's selected
		//TS:  Au contraire!  There may well be something selected.  Let this be a lesson unto thee.
		if(m_SelectList->CurSel == -1)
			ResetFilters();
	}NxCatchAll("Error in CReports::OnRemoveReport()");
}


void CReports::OnDblClickReportsavail(long nRow, long nCol) 
{
	OnAddReport();
}

void CReports::OnDblClickSelectlist(long nRow, long nCol) 
{
	OnRemoveReport();
}


void CReports::OnSelChangedReports(long nRow)
{ 
	ASSERT(m_ReportList != NULL);

	// Empty the filter stuff because they have if something is selected in the Reports 
	// list, then it must be the case that NOTHING is selected in the Selected list
	
	//TS:  Once again, that assumption is just not true!  Please, people, don't go making wild statements
	//that can't be proved.
	if(m_SelectList->CurSel == -1)
		ResetFilters();

	try {
		// Get the ID from the listbox itself
		_variant_t varId;
		varId.vt = VT_EMPTY;

		//DRT 3/2/2004 - PLID 11186 - We can't check the row if it's -1!
		if(nRow != -1)
			varId = m_ReportList->Value[nRow][0];

		if (varId.vt == VT_I4) {
			// Get the report based on that ID
			CReportInfo *pRep = FindReport(varId.iVal);
			ASSERT(pRep);
			if (pRep) {
				// (z.manning, 10/28/05, PLID 18120)
				// If there's an item selected in both availabe reports and current batch, we want to display
				// the one selected in current batch.
				if(m_SelectList->GetCurSel() == -1) {
					// Use the name from the report
					SetDlgItemText(IDC_CURRNAME, pRep->strPrintName);

					//Set the new tooltip.
					if(m_bToolTipsActive){
						m_ToolCtrl.DelTool(&m_NameBox, IDC_DESCFRAME2);
						m_ToolCtrl.AddTool(&m_NameBox, pRep->GetDescription());
						m_strCurrentToolText = pRep->GetDescription();
					}
				}
			}
		} else {
			// There might be a report selected in current batch.
			CString strDisplayText;
			if(m_SelectList->GetCurSel() == -1) {
				strDisplayText = "None Selected";
			}
			else {
				strDisplayText = FindReport(m_SelectList->Value[m_SelectList->GetCurSel()][0])->strPrintName;
			}
			SetDlgItemText(IDC_CURRNAME, strDisplayText);
			//Set the new tooltip.
			if(m_bToolTipsActive){
				m_ToolCtrl.DelTool(&m_NameBox, IDC_DESCFRAME2);
				m_ToolCtrl.AddTool(&m_NameBox, strDisplayText);
				m_strCurrentToolText = strDisplayText;
			}
		}
	} NxCatchAll("CReports::OnSelChangedReports Error 1");
}

void CReports::OnSelChangedSelected(long nRow)
{
	ASSERT(m_SelectList != NULL);

	try {
		// Get the ID from the listbox itself
		_variant_t varId;
		varId.vt = VT_EMPTY;
		//DRT 3/2/2004 - PLID 11186 - We can't check the row if it's -1!
		if(nRow != -1)
			varId = m_SelectList->Value[nRow][0];

		if (varId.vt == VT_I4) {
			// Get the report based on that ID
			CReportInfo *pRep = FindReport(varId.iVal);
			ASSERT(pRep);
			if (pRep) {
				// Use the name from the report
				SetDlgItemText(IDC_CURRNAME, pRep->strPrintName);
					
				//Set the new tooltip.
				if(m_bToolTipsActive){
					m_ToolCtrl.DelTool(&m_NameBox, IDC_DESCFRAME2);
					m_ToolCtrl.AddTool(&m_NameBox, pRep->GetDescription());
					m_strCurrentToolText = pRep->GetDescription();
				}

				LoadFilters(pRep);
			}
		} else {
			// There might be something selected in available reports.
			CString strDisplayText;
			if(m_ReportList->GetCurSel() == -1) {
				strDisplayText = "None Selected";
			}
			else {
				strDisplayText = FindReport(m_ReportList->Value[m_ReportList->GetCurSel()][0])->strPrintName;
			}			
			SetDlgItemText(IDC_CURRNAME, strDisplayText);
			//Set the new tooltip.
			if(m_bToolTipsActive){
				m_ToolCtrl.DelTool(&m_NameBox, IDC_DESCFRAME2);
				m_ToolCtrl.AddTool(&m_NameBox, strDisplayText);
				m_strCurrentToolText = strDisplayText;
			}
			
			// TODO: not sure this is the right thing to call in this case
			ResetFilters();
		}
	} NxCatchAll("CReports::OnSelChangedSelected Error 1");
}

void CReports::OnClearSelect()
{
	CReportInfo* pCurrRep;

	// Removing elements in reverse order in an effort to be efficient and robust
	for (int i = m_RBatch.GetUpperBound(); i >= 0; i--) {
		pCurrRep = (CReportInfo*) m_RBatch.GetAt(i);
		m_RBatch.RemoveAt(i);
		if (pCurrRep){
			delete pCurrRep;
		}
	}

	HR(m_SelectList->Clear());

	// (z.manning, 10/28/05, PLID 18120)
	// There might be something selecting in the available report list.
	CString strDisplayText;
	if(m_ReportList->GetCurSel() == -1) {
		strDisplayText = "None Selected";
	}
	else {
		strDisplayText = FindReport(m_ReportList->Value[m_ReportList->GetCurSel()][0])->strPrintName;
	}
	SetDlgItemText(IDC_CURRNAME, strDisplayText);	
	//Set the new tooltip.
	if(m_bToolTipsActive){
		m_ToolCtrl.DelTool(&m_NameBox, IDC_DESCFRAME2);
		m_ToolCtrl.AddTool(&m_NameBox, strDisplayText);
		m_strCurrentToolText = strDisplayText;
	}

	ResetFilters();

}

BOOL CheckReportTabAccess(IN const BOOL bHasLicense, const CString &strTextName)
{
	// Check license
	if (!bHasLicense) { 
		// No license, warn user and return failure
		MsgBox("You do not have the proper license to use %s. Please see your office manager.", strTextName); 
		return FALSE; 
	}

	//with the new permission structure, we don't have to check tab access anymore
	// Check permissiosn (this function automatically prompts/warns the user
	/*if (!CheckCurrentUserPermissions(ebioReportObject, sptView)) { 
		// No permissions, return failure
		return FALSE; 
	}*/

	// They have access
	return TRUE;
}


// This macro is specific to the GetReportTabCategory function (hence the GRTC_ on the name).  It is not a portable macro, as it makes use of parameters and local variables.
#define GRTC_CHECK_ACCESS(bHasLicense, strCategory, strTextName)  if (bNeedCheckAccess && !CheckReportTabAccess(bHasLicense, strTextName)) return ""; else return strCategory;

CString GetReportTabCategory(IN const short nTrueTabIndex, IN const BOOL bNeedCheckAccess)
{
	switch (nTrueTabIndex) {
	case ReportsModule::PatientsTab:		GRTC_CHECK_ACCESS(g_pLicense->CheckForLicense(CLicense::lcPatient, CLicense::cflrSilent),	"PatientP",		"patient reports"); break;
	case ReportsModule::ContactsTab:		GRTC_CHECK_ACCESS(true,						"OthrContactP",	"contact reports"); break;
	case ReportsModule::MarketingTab:		GRTC_CHECK_ACCESS(true,	"MarketP",		"marketing reports"); break;
	case ReportsModule::InventoryTab:		GRTC_CHECK_ACCESS(g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrSilent),		"InventoryP",	"inventory reports"); break;
		//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
	case ReportsModule::SchedulerTab:		GRTC_CHECK_ACCESS(g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent),	"ScheduleP",	"scheduling reports"); break;
	case ReportsModule::ChargesTab:		GRTC_CHECK_ACCESS(g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent),		"ChargesP",		"billing-related reports"); break;
	case ReportsModule::PaymentsTab:		GRTC_CHECK_ACCESS(g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent),		"PaymentsP",	"billing-related reports"); break;
	case ReportsModule::FinancialTab:		GRTC_CHECK_ACCESS(g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent),		"FinancialP",	"billing-related reports"); break;
	case ReportsModule::ASCTab:			GRTC_CHECK_ACCESS(IsSurgeryCenter(FALSE),	"ASCP",		"ASC reports"); break;
	case ReportsModule::AdministrationTab:	GRTC_CHECK_ACCESS(true,						"AdminP",		"administrative reports"); break;
	case ReportsModule::OtherTab:			GRTC_CHECK_ACCESS(true,						"OtherP",		"diagnostic reports"); break;
	case ReportsModule::PracAnalTab:		GRTC_CHECK_ACCESS(true,						"PracAnalP",		"Practice Analysis Reports"); break;
	// (s.dhole 2012-04-02 09:47) - PLID 49341
	case ReportsModule::OpticalTab:		GRTC_CHECK_ACCESS(g_pLicense->CheckForLicense(CLicense::lcGlassesOrders , CLicense::cflrSilent),						"OpticalP",		"Optical Reports"); break;
	default:
		ASSERT(FALSE);
		return "";
		break;
	}
}
	
//just requeries the listboxes based on the given tab
//-1 indicates a requery the active tab
//returns true if it changed anything
bool CReports::DoInitLists(short nTabIndex/*=-1*/)
{
	// Determine the category string to use for deciding which reports to fill the list with
	CString strCategory; 
	{
		if (nTabIndex == -1) {
			strCategory = GetReportTabCategory(m_tab->CurSel, false);

			//check to make sure that the currently selected tab is visible
			if (m_tab->ShowTab[m_tab->CurSel] == FALSE) {
				//loop through the tabs until we get one that isn't
				for (int i = 0; i < m_tab->GetSize(); i++) {
					if (m_tab->ShowTab[i]) {
						m_tab->CurSel = i;
						strCategory = GetReportTabCategory(i, false);
						break;
					}
				}
			}
		} else {
			strCategory = GetReportTabCategory(nTabIndex, true);

			// If the category string is empty because of a user permission
			// not being set, we need to post this message so the tab
			// control gets the mouse-up message that is taken away from
			// it by the modal dialog invoked to tell the user that person
			// does not have permission.
			if (strCategory.IsEmpty()) {
				GetDlgItem(IDC_TAB)->PostMessage(WM_LBUTTONUP);
			}
		}
	}

	// We're going to have nothing selected, so set the appropriate strings
	{
		SetDlgItemText(IDC_CURRNAME,"None Selected");
		
		//Set the new tooltip.
		if(m_bToolTipsActive){
			m_ToolCtrl.DelTool(&m_NameBox, IDC_DESCFRAME2);
			m_ToolCtrl.AddTool(&m_NameBox, "None Selected");
			m_strCurrentToolText = "None Selected";
		}
	}

	// Clear the list box, and fill it with the reports
	if (m_ReportList != NULL) {
		try {
			if (strCategory != "") {
				// Empty the report list and refill it with the appropriate reports
				HR(m_ReportList->Clear());

				// Loop through the list of known reports
				const CReportInfo *pRep;
				for (long i=0; i<gcs_nKnownReportCount; i++) {
					if (gcs_aryKnownReports[i].strCategory.CompareNoCase(strCategory) == 0) {
//						pRep = LoadReport(gcs_aryKnownReports[i]);
						pRep = &gcs_aryKnownReports[i];
						ASSERT(pRep);
						if (pRep) {
							//if (!pRep->bPhaseOut || GetState_Version4()) {
								// Either this report is not being phased 
								// out or we are displaying phased-out reports

								if(CheckLicenseForReport(&(gcs_aryKnownReports[i]), true)) {

									if (!CheckCurrentUserReportAccess(gcs_aryKnownReports[i].nID, TRUE)) {

										//PLID 14613 - still show it, but grey the row out
										IRowSettingsPtr pRow = m_ReportList->Row[-1];
										pRow->Value[0] = pRep->nID;
										pRow->Value[1] = _bstr_t(pRep->strPrintName);
										pRow->ForeColor =  RGB(196,191,189);
										pRow->ForeColorSel =  RGB(196,191,189);
										m_ReportList->AddRow(pRow);
									}
									else {
										//if we are attempting to add a refractive report, and we're inside a refractive license OR
										//we are attempting to add an ASC report, and we're inside an ASC license OR
										//OR we are attempting to add a nextech report, and we're inside a nextech license
										//the report does not exist in either list

										IRowSettingsPtr pRow = m_ReportList->Row[-1];
										pRow->Value[0] = pRep->nID;
										pRow->Value[1] = _bstr_t(pRep->strPrintName);
										m_ReportList->AddRow(pRow);

									}
								}
							//}
						} else {
							HandleException(NULL, 
								"CReports::DoInitLists Error 2.  Could not load unknown report: " + 
								gcs_aryKnownReports[i].strCategory + "\\" + 
								gcs_aryKnownReports[i].strReportFile, __LINE__, __FILE__);
							return false;
						}
					}
				}
				// Success
				return true;
			}
		} NxCatchAllCall("CReports::DoInitLists Error 1", return false);
	}
	
	// If we made it here we failed
	return false;
}

void CReports::OnSelChangedProvselect(long nRow) 
{
	//TES 8/5/03: This is all in OnSelChosen now.
}

void CReports::OnSelChangedPatselect(long nRow)
{
	try {
		// We must have a current report or else what the heck are we doing?
		ASSERT(CurrReport);
		if (CurrReport) {
			if (nRow >= 0) {
				// (a.walling 2011-08-04 14:36) - PLID 44788
				// Get the currently selected patient's ID
				m_nSelectedPatientID = VarLong(m_PatSelect->Value[nRow][0]);
				CurrReport->nPatient = m_nSelectedPatientID;
			} else {
				// TODO: I don't know what this case really means
				ASSERT(FALSE);
			}
		} else {
			// CurrReport was NULL
			AfxThrowNxException("Current Report cannot be NULL");
		}
	} NxCatchAll("CReports::OnSelChangedPatselect Error 1");
}

// (a.walling 2011-08-04 14:36) - PLID 44788 - Load the datalist if necessary when dropping down
void CReports::OnDroppingDownPatselect()
{
	try {
		KillTimer(IDT_FREEBIGDATALISTS);
		if (m_PatSelect->GetRowCount() == 0 && !m_PatSelect->IsRequerying()) {
			m_PatSelect->Requery();

			if (m_PatSelect->TrySetSelByColumn(0, m_nSelectedPatientID) < 0) {
				m_PatSelect->ComboBoxText = (LPCTSTR)GetExistingPatientName(m_nSelectedPatientID);
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2011-08-04 14:36) - PLID 44788 - Delay freeing the datalist when hiding
void CReports::OnShowWindow(BOOL bShow, UINT nStatus)
{
	try {
		if (!bShow) {
			SetTimer(IDT_FREEBIGDATALISTS, GetRemotePropertyInt("FreeBigDataListsDelay", 1200000, 0, "<None>", true), NULL);
		} else {
			KillTimer(IDT_FREEBIGDATALISTS);
		}
	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnShowWindow(bShow, nStatus);
}

// (a.walling 2011-08-04 14:36) - PLID 44788 - Delay freeing the datalist when hiding / deactivating
void CReports::OnParentViewActivate(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	try {
		if (!bActivate) {
			SetTimer(IDT_FREEBIGDATALISTS, GetRemotePropertyInt("FreeBigDataListsDelay", 1200000, 0, "<None>", true), NULL);
		} else {
			KillTimer(IDT_FREEBIGDATALISTS);
		}
	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnParentViewActivate(bActivate, pActivateView, pDeactiveView);
}

// (a.walling 2011-08-04 14:36) - PLID 44788 - Free the datalist memory after hidden long enough
void CReports::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == IDT_FREEBIGDATALISTS) {
		try {
			KillTimer(IDT_FREEBIGDATALISTS);

			if (m_PatSelect) {
				m_PatSelect->Clear();
				m_PatSelect->ComboBoxText = (LPCTSTR)GetExistingPatientName(m_nSelectedPatientID);
			}
		} NxCatchAll(__FUNCTION__);
	}

	CNxDialog::OnTimer(nIDEvent);
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CReports::OnChangeFrom(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (CurrReport) 
		CurrReport->DateFrom = m_from.GetValue();

	*pResult = 0;
}

// (a.walling 2008-05-13 10:34) - PLID 27591 - Use the new notify events
void CReports::OnChangeTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (CurrReport) 
		CurrReport->DateTo = m_to.GetValue();
	
	*pResult = 0;
}

void CReports::EnableFilterDateOptions(bool enable/*=true*/)
{
	if (enable)
	{
		//m_rServDate.EnableWindow(TRUE);
	//	m_rInputDate.EnableWindow(TRUE);
		//GetDlgItem(IDC_LABEL_FILTER)->EnableWindow(TRUE);
	}
	else 
	{//	m_rServDate.EnableWindow(FALSE);
		//m_rInputDate.EnableWindow(FALSE);
		//GetDlgItem(IDC_LABEL_FILTER)->EnableWindow(FALSE);
	}

//	InvalidateDlgItem(IDC_LABEL_FILTER);
}

void CReports::AddExtItem(LPCSTR bound, LPCSTR display)
{
	IRowSettingsPtr pRow;

	pRow = m_ExtFilterList->GetRow(-1);
	pRow->Value[0] = bound;
	pRow->Value[1] = display;
	m_ExtFilterList->AddRow(pRow);
}

CString CReports::GetExtItemName(LPCTSTR strID)
{
	int nRow = m_ExtFilterList->FindByColumn(0, _bstr_t(strID), 0, FALSE);
	if(nRow != -1) {
		return VarString(m_ExtFilterList->GetValue(nRow,1));
	}
	else {
		ASSERT(FALSE);
		return "";
	}
}

void CReports::LoadExtendedFilter (CReportInfo *pReport)
{
	if (!pReport->bExtended) // not enabled
	{ 
		ShowDlgItem(IDC_MULTI_EXT_LIST, SW_HIDE);
		ShowDlgItem(IDC_EXT_FILTER, SW_SHOW);
		m_UseExtended.EnableWindow(FALSE);
		EnableDlgItem(IDC_EXT_FILTER, FALSE);
		return;
	}

	// (j.gruber 2007-03-12 16:05) - PLID 22121 - size the drop down back to normal
	m_ExtFilterList->DropDownWidth = -1;
	
	// is enabled
	m_UseExtended.EnableWindow(TRUE);
	SetDlgItemText(IDC_EXTENDED_TEXT,pReport->strExtraText);
	m_ExtFilterList->Clear();// take out whats in there now
	// set up the combo according to a format:
	// 1:  > 0, < 0, or = 0
	// 2:  Cash, Check, Charge, etc..
	// 3:  SQL
	switch (pReport->nExtendedStyle)
	{
		case 1:
			AddExtItem("> 0", "> 0");
			AddExtItem("= 0", "= 0");
			AddExtItem("< 0", "< 0");
			AddExtItem("<> 0", "<> 0");
			break;
		case 2:
			//TES 8/22/2005 - Add a {Multiple} row.
			if(pReport->GetAllowMultipleExtended()) AddExtItem("-3", "{Multiple}");

			AddExtItem("1", "Cash");
			AddExtItem("2", "Check");
			AddExtItem("3", "Credit Card");
			AddExtItem("0", "Adjustment");		//DRT 7/24/02 - Apparently this has been 0, not 6, since we started SQL, and we've ALWAYS been wrong in reports
			AddExtItem("7", "Cash Refund");
			AddExtItem("8", "Check Refund");
			AddExtItem("9", "Charge Refund");
			break;
		case 3:	// Create a query based on the extended sql of this report
			try 
			{
				//TES 8/22/2005 - Add a {Multiple} row.
				if(pReport->GetAllowMultipleExtended()) AddExtItem("-3", "{Multiple}");

				if(pReport->nID == 274 && IsSurgeryCenter(false)) {
					//DRT 7/28/03 - This report can filter on case history, but we only want to show it
					//	if they are on an ASC license.
					pReport->strExtendedSql += " UNION SELECT 4, 'Case History' ";
				}
				if(pReport->nID == 669) {
					// (d.thompson 2010-11-17) - PLID 41514 - I think the above is sufficient precedent.  The Credit Card Reconciliation
					//	extended filter needs to vary depending on their license.  QBMS licenses get QBMS accounts, Chase licenses get
					//	Chase accounts.  This seems to be the best/only place to make this distinction.
					if(g_pLicense && g_pLicense->HasCreditCardProc_QBMS(CLicense::cflrSilent)) {
						pReport->strExtendedSql = "SELECT ID, Description FROM QBMS_SetupData";
					}
					else if(g_pLicense && g_pLicense->HasCreditCardProc_Chase(CLicense::cflrSilent)) {
						pReport->strExtendedSql = "SELECT ID, Description FROM Chase_SetupDataT";
					}
					else {
						//It should be impossible to reach this state if you don't have a QBMS or Chase credit card license, 
						//	so just let the query fail if we happened to reach it.
					}
				}
				_RecordsetPtr prs = CreateRecordset(pReport->strExtendedSql);
				FieldsPtr flds = prs->Fields;
				FieldPtr fld0 = flds->Item[(long)0];
				FieldPtr fld1 = flds->Item[(long)1];

				// Loop through each record putting fields 0 and 1 into the extended listbox
				while (!prs->eof) 
				{
					try 
					{
						CString strBound;
						_variant_t varBound;

						varBound = fld0->Value;
						switch (varBound.vt)//BVB we can add other possible values here, the old way only handled these 2 anyhow
						{
							case VT_I4:
								strBound.Format("%i", VarLong(varBound));
								break;
							case VT_BSTR:
								strBound = VarString(varBound);
								break;
							default:
								ASSERT(FALSE);
								strBound = "";
								break;
						}
						AddExtItem(strBound, VarString(fld1->Value));
					} catch (CException*) { // (a.walling 2011-04-25 11:15) - PLID 42914 - Use NxCatch
						NXCATCH_DISPATCH("CReports::LoadFilters Error 5", NxCatch::eDefault);
					}
					HR(prs->MoveNext());
				}
			}
			NxCatchAll("CReports::LoadFilters Error 10");
			break;
		case 4:

			AddExtItem("StatementSubQ.PatTotResp > 0", "Patient Balance > 0");
			AddExtItem("StatementSubQ.PatTotResp = 0", "Patient Balance = 0");
			AddExtItem("StatementSubQ.PatTotResp < 0", "Patient Balance < 0");
			AddExtItem("StatementSubQ.PatTotResp <> 0", "Patient Balance <> 0");
			
			AddExtItem("StatementSubQ.InsTotResp > 0", "Insurance Balance > 0");
			AddExtItem("StatementSubQ.InsTotResp = 0", "Insurance Balance = 0");
			AddExtItem("StatementSubQ.InsTotResp < 0", "Insurance Balance < 0");
			AddExtItem("StatementSubQ.InsTotResp <> 0", "Insurance Balance <> 0");

			AddExtItem("((StatementSubQ.PattotResp > 0) OR (StatementSubQ.InsTotResp > 0))", "Patient or Ins. Balance > 0");
			AddExtItem("((StatementSubQ.PattotResp = 0) OR (StatementSubQ.InsTotResp = 0))", "Patient or Ins. Balance = 0");
			AddExtItem("((StatementSubQ.PattotResp < 0) OR (StatementSubQ.InsTotResp < 0))", "Patient or Ins. Balance < 0");
			AddExtItem("((StatementSubQ.PattotResp <> 0) OR (StatementSubQ.InsTotResp <> 0))", "Patient or Ins. Balance <> 0");

			AddExtItem("((StatementSubQ.PattotResp > 0) AND (StatementSubQ.InsTotResp > 0))", "Patient and Ins. Balance > 0");
			AddExtItem("((StatementSubQ.PattotResp = 0) AND (StatementSubQ.InsTotResp = 0))", "Patient and Ins. Balance = 0");
			AddExtItem("((StatementSubQ.PattotResp < 0) AND (StatementSubQ.InsTotResp < 0))", "Patient and Ins. Balance < 0");
			AddExtItem("((StatementSubQ.PattotResp <> 0) AND (StatementSubQ.InsTotResp <> 0))", "Patient and Ins. Balance <> 0");

			break;
		case 5:
			AddExtItem("1", "0-30");
			AddExtItem("2", "31-60");
			AddExtItem("3", "61-90");
			AddExtItem("4", "91-120");
			AddExtItem("5", "> 120");
			AddExtItem("6", "> 90");
			AddExtItem("7", "> 60");
			AddExtItem("8", "> 30");
			break;

		// (j.gruber 2007-03-12 16:05) - PLID 22121 - made the statement by location filter by locations also
		case 6:
			{


			//first add the items we always had
			AddExtItem("-1:StatementSubQ.PatTotResp > 0", "Patient Balance > 0");
			AddExtItem("-1:StatementSubQ.PatTotResp = 0", "Patient Balance = 0");
			AddExtItem("-1:StatementSubQ.PatTotResp < 0", "Patient Balance < 0");
			AddExtItem("-1:StatementSubQ.PatTotResp <> 0", "Patient Balance <> 0");
			
			AddExtItem("-1:StatementSubQ.InsTotResp > 0", "Insurance Balance > 0");
			AddExtItem("-1:StatementSubQ.InsTotResp = 0", "Insurance Balance = 0");
			AddExtItem("-1:StatementSubQ.InsTotResp < 0", "Insurance Balance < 0");
			AddExtItem("-1:StatementSubQ.InsTotResp <> 0", "Insurance Balance <> 0");

			AddExtItem("-1:((StatementSubQ.PattotResp > 0) OR (StatementSubQ.InsTotResp > 0))", "Patient or Ins. Balance > 0");
			AddExtItem("-1:((StatementSubQ.PattotResp = 0) OR (StatementSubQ.InsTotResp = 0))", "Patient or Ins. Balance = 0");
			AddExtItem("-1:((StatementSubQ.PattotResp < 0) OR (StatementSubQ.InsTotResp < 0))", "Patient or Ins. Balance < 0");
			AddExtItem("-1:((StatementSubQ.PattotResp <> 0) OR (StatementSubQ.InsTotResp <> 0))", "Patient or Ins. Balance <> 0");

			AddExtItem("-1:((StatementSubQ.PattotResp > 0) AND (StatementSubQ.InsTotResp > 0))", "Patient and Ins. Balance > 0");
			AddExtItem("-1:((StatementSubQ.PattotResp = 0) AND (StatementSubQ.InsTotResp = 0))", "Patient and Ins. Balance = 0");
			AddExtItem("-1:((StatementSubQ.PattotResp < 0) AND (StatementSubQ.InsTotResp < 0))", "Patient and Ins. Balance < 0");
			AddExtItem("-1:((StatementSubQ.PattotResp <> 0) AND (StatementSubQ.InsTotResp <> 0))", "Patient and Ins. Balance <> 0");


			_RecordsetPtr rsLocations = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND TypeID = 1 ");
			while (! rsLocations->eof) {

				//increase the drop down width
				m_ExtFilterList->DropDownWidth = 400;


				CString strName = AdoFldString(rsLocations, "Name", "");
				CString strID;

				strID.Format("%li", AdoFldLong(rsLocations, "ID"));

				AddExtItem(strID + ":StatementSubQ.PatTotResp > 0", "Patient Balance at " + strName + " > 0");
				AddExtItem(strID + ":StatementSubQ.PatTotResp = 0", "Patient Balance at " + strName + "  = 0");
				AddExtItem(strID + ":StatementSubQ.PatTotResp < 0", "Patient Balance at " + strName + "  < 0");
				AddExtItem(strID + ":StatementSubQ.PatTotResp <> 0", "Patient Balance at " + strName + "  <> 0");
			
				AddExtItem(strID + ":StatementSubQ.InsTotResp > 0", "Insurance Balance at " + strName + "  > 0");
				AddExtItem(strID + ":StatementSubQ.InsTotResp = 0", "Insurance Balance at " + strName + "  = 0");
				AddExtItem(strID + ":StatementSubQ.InsTotResp < 0", "Insurance Balance at " + strName + "  < 0");
				AddExtItem(strID + ":StatementSubQ.InsTotResp <> 0", "Insurance Balance at " + strName + "  <> 0");

				AddExtItem(strID + ":((StatementSubQ.PattotResp > 0) OR (StatementSubQ.InsTotResp > 0))", "Patient or Ins. Balance at " + strName + "  > 0");
				AddExtItem(strID + ":((StatementSubQ.PattotResp = 0) OR (StatementSubQ.InsTotResp = 0))", "Patient or Ins. Balance at " + strName + "  = 0");
				AddExtItem(strID + ":((StatementSubQ.PattotResp < 0) OR (StatementSubQ.InsTotResp < 0))", "Patient or Ins. Balance at " + strName + "  < 0");
				AddExtItem(strID + ":((StatementSubQ.PattotResp <> 0) OR (StatementSubQ.InsTotResp <> 0))", "Patient or Ins. Balance at " + strName + "  <> 0");

				AddExtItem(strID + ":((StatementSubQ.PattotResp > 0) AND (StatementSubQ.InsTotResp > 0))", "Patient and Ins. Balance at " + strName + "  > 0");
				AddExtItem(strID + ":((StatementSubQ.PattotResp = 0) AND (StatementSubQ.InsTotResp = 0))", "Patient and Ins. Balance at " + strName + "  = 0");
				AddExtItem(strID + ":((StatementSubQ.PattotResp < 0) AND (StatementSubQ.InsTotResp < 0))", "Patient and Ins. Balance at " + strName + "  < 0");
				AddExtItem(strID + ":((StatementSubQ.PattotResp <> 0) AND (StatementSubQ.InsTotResp <> 0))", "Patient and Ins. Balance at " + strName + "  <> 0");
			
				rsLocations->MoveNext();
			}
			rsLocations->Close();
			}

			break;
	}

	CStringArray saExtraValues;
	pReport->GetExtraValues(saExtraValues);
	//TES 12/18/2008 - PLID 32514 - They're not allowed to use the Extended filter if they
	// have the Scheduler Standard version
	// (j.gruber 2009-02-03 16:43) - PLID 32939 - except if its the statement
	if((!IsStatement(pReport->nID)) && !g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent) ) {
		saExtraValues.RemoveAll();
		pReport->ClearExtraValues();
	}
	if (saExtraValues.GetSize() == 1) 
	{
		ShowDlgItem(IDC_MULTI_EXT_LIST, SW_HIDE);
		ShowDlgItem(IDC_EXT_FILTER, SW_SHOW);
		m_UseExtended.SetCheck(1);
		EnableDlgItem(IDC_EXT_FILTER, TRUE);
		m_ExtFilterList->SetSelByColumn(0, _bstr_t(saExtraValues[0]));
	}
	else if(saExtraValues.GetSize() > 1) {
		m_UseExtended.SetCheck(1);
		ShowDlgItem(IDC_EXT_FILTER, SW_HIDE);
		CString strDisplayList;
		for(int i=0; i < saExtraValues.GetSize(); i++) {
			strDisplayList += GetExtItemName(saExtraValues[i]) + ",";
		}
		strDisplayList.TrimRight(",");
		m_nxlExtLabel.SetText(strDisplayList);
		m_nxlExtLabel.SetType(dtsHyperlink);
		ShowDlgItem(IDC_MULTI_EXT_LIST, SW_SHOW);
		InvalidateDlgItem(IDC_MULTI_EXT_LIST);
	}
	else 
	{
		switch(pReport->nID) {
			case 169:  //Patient Statement
			//case 234:
			case 337:			
			case 338:
			case 353:
			//case 354:
			case 355:
			//case 356:
			case 434:
			case 435:
			case 436:
			case 437:
			case 483:
			case 484:
			case 485:
			case 486: {
				//TES 12/18/2008 - PLID 32514 - If they're on Scheduler Standard, they're not
				// allowed to use the Extended filter, so don't default it to anything.
				// (j.gruber 2009-02-03 16:51) - PLID 32939 - except if its a statement
				if((IsStatement(pReport->nID))|| g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
					// (j.jones 2006-10-19 09:23) - PLID 23148 - auto-select "Patient Balance > 0"
					bool bWasLoading = m_bIsLoading;
					m_bIsLoading = FALSE;
					ShowDlgItem(IDC_MULTI_EXT_LIST, SW_HIDE);
					ShowDlgItem(IDC_EXT_FILTER, SW_SHOW);
					m_UseExtended.SetCheck(1);
					OnUseExtended();
					EnableDlgItem(IDC_EXT_FILTER, TRUE);
					//we may have remembered their previous selection
					if(m_ExtFilterList->CurSel == -1) {
						m_ExtFilterList->CurSel = 0;
						OnSelChosenExtFilter(m_ExtFilterList->CurSel);
					}
					m_bIsLoading = bWasLoading;
					return;
				}
				break;
				}
			default:
				break;
		}

		//otherwise, select nothing
		ShowDlgItem(IDC_MULTI_EXT_LIST, SW_HIDE);
		ShowDlgItem(IDC_EXT_FILTER, SW_SHOW);
		m_ExtFilterList->CurSel = -1;
		if(m_UseExtended.GetCheck()) {
			OnSelChosenExtFilter(-1);
		}
	}
}

void CReports::LoadFilters(CReportInfo *pReport)
{
	CString StrTemp;

	BeginWaitCursor ();
	m_bIsLoading = true; // use this bool for events that get fired as a result of
		// changing the data .. 

	// Make the given report current (even if it is NULL)
	CurrReport = pReport;

	// If NULL was given then we call ResetFilters instead
	if (pReport == NULL) {
		// TODO: I commented this line out because it wasn't there 
		// before and I want to be consistent but it seems to me 
		// that we might want it in here
//		ResetFilters();
		return;
	}

	//check to see if there is a default report
	//if the default is a custom report, grey out the detail/summary buttons, but leave the rest alone

	//PLID 15293 - if this is a statement, use the drop down
	if (IsStatement(pReport->nID)) {
		//hide the detail and summary options
		m_DetailCheck.ShowWindow(SW_HIDE);
		m_SummaryCheck.ShowWindow(SW_HIDE);

		//load the list
		// (j.gruber 2010-03-11 12:29) - PLID 29120 -don't reset
		SetReportList(m_pReportTypeList, pReport->nID, pReport, FALSE);

		//show the list
		GetDlgItem(IDC_REPORT_TYPE_LIST)->ShowWindow(SW_SHOW);
		if (m_pReportTypeList->CurSel != -1) {
			pReport->strReportFile = VarString(m_pReportTypeList->GetValue(m_pReportTypeList->CurSel, 3));
			pReport->nDefaultCustomReport = VarLong(m_pReportTypeList->GetValue(m_pReportTypeList->CurSel, 1));
		}
		else {
			pReport->strReportFile = "";
		}

	

		LoadDateFilters(pReport);
	}
	else {
		//show the detail and summary buttons and hide the list
		m_DetailCheck.ShowWindow(SW_SHOW);
		m_SummaryCheck.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_REPORT_TYPE_LIST)->ShowWindow(SW_HIDE);



		if (pReport->nDefaultCustomReport > 0) {

			m_DetailCheck.EnableWindow(FALSE);
			m_SummaryCheck.EnableWindow(FALSE);

			LoadDateFilters(pReport);
		}
	//TODO: I am 95% sure that all this is unnecessary, because the info contained in DefaultReportsT
	//has now passed into the ReportInfo class itself.  But I want to double-check that before I'm done.
	/*else if (pReport->nDefaultNumber < NODEFAULT) {
		//they have one of our reports selected as the default
		//so we have to check the detail and date things, but the rest, we
		// we leave as it was
		if (pReport->nDefaultNumber == DETAILED) {

			m_DetailCheck.EnableWindow(TRUE);
			m_SummaryCheck.EnableWindow(TRUE);
			m_DetailCheck.SetCheck(1);
			m_SummaryCheck.SetCheck(0);

			LoadDateFilters(pReport);
		}
		else if (pReport->nDefaultNumber == SUMMARY) {
			
			m_DetailCheck.EnableWindow(TRUE);
			m_SummaryCheck.EnableWindow(TRUE);
			m_DetailCheck.SetCheck(0);
			m_SummaryCheck.SetCheck(1);

			LoadDateFilters(pReport);
		}
		else if (pReport->nDefaultNumber == DETAILEDSERVICE) {

			m_DetailCheck.EnableWindow(TRUE);
			m_SummaryCheck.EnableWindow(TRUE);
			m_DetailCheck.SetCheck(1);
			m_SummaryCheck.SetCheck(0);

			EnableFilterDateOptions(true);
			LoadDateFilters(pReport);
			m_rAllDates.SetCheck(0);
			m_rDateRange.SetCheck(1);
			m_rServDate.SetCheck(1);
			m_rInputDate.SetCheck(0);
		}
		else if (pReport->nDefaultNumber == DETAILEDINPUT) {

			m_DetailCheck.EnableWindow(TRUE);
			m_SummaryCheck.EnableWindow(TRUE);
			m_DetailCheck.SetCheck(1);
			m_SummaryCheck.SetCheck(0);

			EnableFilterDateOptions(true);
			LoadDateFilters(pReport);
			m_rAllDates.SetCheck(0);
			m_rDateRange.SetCheck(1);
			m_rServDate.SetCheck(0);
			m_rInputDate.SetCheck(1);

		}
		else if (pReport->nDefaultNumber == SUMMARYSERVICE) {

			m_DetailCheck.EnableWindow(TRUE);
			m_SummaryCheck.EnableWindow(TRUE);
			m_DetailCheck.SetCheck(0);
			m_SummaryCheck.SetCheck(1);

			EnableFilterDateOptions(true);
			LoadDateFilters(pReport);
			m_rAllDates.SetCheck(0);
			m_rDateRange.SetCheck(1);
			m_rServDate.SetCheck(1);
			m_rInputDate.SetCheck(0);

		}
		else if (pReport->nDefaultNumber == SUMMARYINPUT) {

			m_DetailCheck.EnableWindow(TRUE);
			m_SummaryCheck.EnableWindow(TRUE);
			m_DetailCheck.SetCheck(0);
			m_SummaryCheck.SetCheck(1);

			EnableFilterDateOptions(true);
			LoadDateFilters(pReport);
			m_rAllDates.SetCheck(0);
			m_rDateRange.SetCheck(1);
			m_rServDate.SetCheck(0);
			m_rInputDate.SetCheck(1);

		}
		else if (pReport->nDefaultNumber == INPUT) {

			LoadFormatFilters(pReport);

			EnableFilterDateOptions(true);
			LoadDateFilters(pReport);
			m_rAllDates.SetCheck(0);
			m_rDateRange.SetCheck(1);
			m_rServDate.SetCheck(0);
			m_rInputDate.SetCheck(1);

		}
		else if (pReport->nDefaultNumber == SERVICE) {

			LoadFormatFilters(pReport);
			
			EnableFilterDateOptions(true);
			LoadDateFilters(pReport);
			m_rAllDates.SetCheck(0);
			m_rDateRange.SetCheck(1);
			m_rServDate.SetCheck(1);
			m_rInputDate.SetCheck(0);

		}
	}*/
		else {

				// detail or summary
				LoadFormatFilters(pReport);
				LoadDateFilters(pReport);
		}
	}

		//load the rest of the filters, they are the same whether there is a default report or not

		// (a.walling 2011-08-04 14:36) - PLID 44788
		if (pReport->nPatient > 0) {
			m_nSelectedPatientID = pReport->nPatient;
		}
		
		if (m_nSelectedPatientID == -1) {
			m_PatSelect->CurSel = -1;
			m_PatSelect->PutComboBoxText("");
		} else if (m_PatSelect->TrySetSelByColumn(0, m_nSelectedPatientID) < 0) {
			m_PatSelect->PutComboBoxText((LPCTSTR)GetExistingPatientName(m_nSelectedPatientID));
		}

		// patient filter
		if (!pReport->nPatient) {
			m_rAllPatients.EnableWindow(FALSE);
			m_rSelectPat.EnableWindow(FALSE);
			EnableDlgItem(IDC_PATSELECT, FALSE);
		}
		else {
			m_rAllPatients.EnableWindow();
			m_rSelectPat.EnableWindow();
			if (pReport->nPatient > 0) { // someone is selected
				//TES 12/18/2008 - PLID 32514 - They're not allowed to use the Patient filter
				// if they have the Scheduler Standard license.
				if(!g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent)) {
					pReport->nPatient = -1;
					m_rSelectPat.SetCheck(0);
					m_rAllPatients.SetCheck(1);
					EnableDlgItem(IDC_PATSELECT, FALSE);
				}
				else {
					m_rSelectPat.SetCheck(1);
					m_rAllPatients.SetCheck(0);
					EnableDlgItem(IDC_PATSELECT, TRUE);
				}
			}
			else {
				m_rSelectPat.SetCheck(0);
				m_rAllPatients.SetCheck(1);
				EnableDlgItem(IDC_PATSELECT, FALSE);
			}
		} // end patient filter
		
		// provider filter
		if (!pReport->nProvider) {
			m_rAllProviders.EnableWindow(FALSE);
			m_rSelectProv.EnableWindow(FALSE);
			ShowDlgItem(IDC_MULTI_PROV_LIST, SW_HIDE);
			ShowDlgItem(IDC_PROVSELECT, SW_SHOW);
			EnableDlgItem(IDC_PROVSELECT, FALSE);

		}
		else {
			m_rAllProviders.EnableWindow();
			m_rSelectProv.EnableWindow();

			if (pReport->nProvider != -1) { // somebody is selected
				m_rSelectProv.SetCheck(1);
				m_rAllProviders.SetCheck(0);
				m_ProvSelect->SetSelByColumn(0, pReport->nProvider);

				EnableDlgItem(IDC_PROVSELECT, TRUE);
				//Check if they have "all" selected
				if(CurrReport->m_dwProviders.GetSize() > 1) {
					ShowDlgItem(IDC_PROVSELECT, SW_HIDE);
					CString strIDList, strID, strProvList;
					for(int i=0; i < CurrReport->m_dwProviders.GetSize(); i++) {
						strID.Format("%li, ", (long)CurrReport->m_dwProviders.GetAt(i));
						strIDList += strID;
					}
					strIDList = strIDList.Left(strIDList.GetLength()-2);
					_RecordsetPtr rsProvs = CreateRecordset("SELECT ID, Last + ', ' + First + ' ' + Middle AS Name FROM PersonT INNER JOIN ProvidersT ON PersonT.Id = ProvidersT.PersonID WHERE PersonT.ID IN (%s) ORDER BY Last, First, Middle ASC", strIDList);
					while(!rsProvs->eof) {
						strProvList += AdoFldString(rsProvs, "Name") + ", ";
						rsProvs->MoveNext();
					}
					rsProvs->Close();
					strProvList = strProvList.Left(strProvList.GetLength()-2);
					m_nxlProviderLabel.SetText(strProvList);
					m_nxlProviderLabel.SetType(dtsHyperlink);
					ShowDlgItem(IDC_MULTI_PROV_LIST, SW_SHOW);
					InvalidateDlgItem(IDC_MULTI_PROV_LIST);
				}
				else {
					//They selected exactly one. (even if that one was "<No Provider>"
					ShowDlgItem(IDC_MULTI_PROV_LIST, SW_HIDE);
					ShowDlgItem(IDC_PROVSELECT, SW_SHOW);
				}
				
			}
			else { // all providers selected
				m_rAllProviders.SetCheck(1);
				m_rSelectProv.SetCheck(0);
				ShowDlgItem(IDC_MULTI_PROV_LIST, SW_HIDE);
				ShowDlgItem(IDC_PROVSELECT, SW_SHOW);
				EnableDlgItem(IDC_PROVSELECT, FALSE);
			}

		}  // end provider filter

		//location filter
		//if the report should not be filtered by location then 
		//diable the location boxes
		if (!pReport->nLocation) {
			m_rAllLocations.EnableWindow(FALSE);
			m_rSelectLoc.EnableWindow(FALSE);
			EnableDlgItem(IDC_LOCATIONSELECT, FALSE);
			//(e.lally 2008-09-05) PLID 6780 - Toggle dropdown and multiselect
			ShowDlgItem(IDC_MULTI_LOC_LIST, SW_HIDE);
			ShowDlgItem(IDC_LOCATIONSELECT, SW_SHOW);
		}
		else { //the report should be filtered
			m_rAllLocations.EnableWindow();
			m_rSelectLoc.EnableWindow();

			//DRT 7/3/2007 - PLID 11920 - I added a function to be able to specify the WHERE clause of 
			//	this combo.  Some reports want managed locations, some reports want managed & unmanaged...
			//	some report down the road may want to just show locations that contain the letter Q.  So
			//	each report designer can put in an override for the where clause.
			CString strNewWhere = pReport->GetLocationWhereClause();

			//See what's already in the list.  At present time, this rarely changes, so we can dodge a requery most of the time.
			CString strCurrentWhere = VarString(m_LocationSelect->WhereClause);

			if(strCurrentWhere != strNewWhere) {
				//Our WHERE clause has changed.  We need to requery the list
				//The code below will handle choosing a selection
				//(e.lally 2008-09-05) PLID 6780 - Call our requery function so we still have No and Multiple location options
				// (j.jones 2010-07-19 09:54) - PLID 39117 - pass in the WhereClause, we will set it inside the function
				RequeryLocationList(strNewWhere);
			}

			//(e.lally 2008-09-05) PLID 6780 - Check against the sentinel -1 value, other negatives are still used
			if (pReport->nLocation != -1) {  //a location is already selected
				//TES 12/18/2008 - PLID 32514 - They're not allowed to use the Location filter
				// if they have the Scheduler Standard license.
				if(!g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent)) {
					pReport->nLocation = -1;
					m_rSelectLoc.SetCheck(0);
					m_rAllLocations.SetCheck(1);
					EnableDlgItem(IDC_LOCATIONSELECT, FALSE);
				}
				else {
					m_rSelectLoc.SetCheck(1);
					m_rAllLocations.SetCheck(0);
					EnableDlgItem(IDC_LOCATIONSELECT, TRUE);
					m_LocationSelect->SetSelByColumn(0, pReport->nLocation);

					//(e.lally 2008-09-05) PLID 6780 - Check if they have "all" selected
					if(CurrReport->m_dwLocations.GetSize() > 1) {
						ShowDlgItem(IDC_LOCATIONSELECT, SW_HIDE);
						CString strIDList, strID, strLocationList;
						for(int i=0; i < CurrReport->m_dwLocations.GetSize(); i++) {
							strID.Format("%li, ", (long)CurrReport->m_dwLocations.GetAt(i));
							strIDList += strID;
						}
						strIDList = strIDList.Left(strIDList.GetLength()-2);
						_RecordsetPtr rsLocations = CreateRecordset("SELECT Name FROM LocationsT WHERE LocationsT.ID IN (%s) ORDER BY Name ASC", strIDList);
						while(!rsLocations->eof) {
							strLocationList += AdoFldString(rsLocations, "Name") + ", ";
							rsLocations->MoveNext();
						}
						rsLocations->Close();
						strLocationList = strLocationList.Left(strLocationList.GetLength()-2);
						m_nxlLocationLabel.SetText(strLocationList);
						m_nxlLocationLabel.SetType(dtsHyperlink);
						//(e.lally 2008-10-20) PLID 6780 - Toggle multi-select and dropdown
						ShowDlgItem(IDC_MULTI_LOC_LIST, SW_SHOW);
						ShowDlgItem(IDC_LOCATIONSELECT, SW_HIDE);
						InvalidateDlgItem(IDC_MULTI_LOC_LIST);
					}
					else{
						//(e.lally 2008-10-20) PLID 6780 - Toggle multi-select and dropdown
						ShowDlgItem(IDC_MULTI_LOC_LIST, SW_HIDE);
						ShowDlgItem(IDC_LOCATIONSELECT, SW_SHOW);
					}
				}
			}
			else { //all locations is selected
				m_rAllLocations.SetCheck(1);
				m_rSelectLoc.SetCheck(0);
				EnableDlgItem(IDC_LOCATIONSELECT, FALSE);
				//(e.lally 2008-09-05) PLID 6780 - Toggle multi-select and dropdown
				ShowDlgItem(IDC_MULTI_LOC_LIST, SW_HIDE);
				ShowDlgItem(IDC_LOCATIONSELECT, SW_SHOW);
			}
		}

		// extended selection filter
		m_UseExtended.SetCheck(0);
		ShowDlgItem(IDC_MULTI_EXT_LIST, SW_HIDE);
		ShowDlgItem(IDC_EXT_FILTER, SW_SHOW);
		EnableDlgItem(IDC_EXT_FILTER, FALSE);
		LoadExtendedFilter(pReport);

		// CreateMergeGroup
		if (pReport->bCreateGroup && !pReport->strReportFile.IsEmpty()) {
			GetDlgItem (IDC_CREATEMERGE)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem (IDC_CREATEMERGE)->EnableWindow(FALSE);
		}

		if (!pReport->bUseGroupOption || pReport->strReportFile.IsEmpty())
		{	//Use Merge Group disabled
			m_No_Group_Filter.EnableWindow(FALSE);
			m_UseFilter.EnableWindow(FALSE);
			m_UseGroup.EnableWindow(FALSE);
			GetDlgItem(IDC_GROUPDL)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(FALSE);
			
		}
		else
		{	//Use Merge Group enabled
			
			//DRT 8/6/03 - Due to a bug in SQL Server sp3, filters will not
			//		work with this report.  This can be removed when we are 
			//		sure this bug is fixed (due to be in sp4), and that anyone
			//		using this version will be on sp4.
			//TES 4/19/04 - Sched Prod by Resource has the same problem.
			if(pReport->nID != 370 && pReport->nID != 500)
				m_UseFilter.EnableWindow(TRUE);
			else
				m_UseFilter.EnableWindow(FALSE);

			m_No_Group_Filter.EnableWindow(TRUE);
			m_UseGroup.EnableWindow(TRUE);
				
			//TES 5/30/03: Adding filter type filtering.
			if (pReport->bUseFilter) {
				//TES 12/18/2008 - PLID 32514 - They're not allowed to use the Filter filter
				// if they have the Scheduler Standard license.
				if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
					m_No_Group_Filter.SetCheck(1);
					m_UseFilter.SetCheck(0);
					m_UseGroup.SetCheck(0);
					GetDlgItem(IDC_GROUPDL)->EnableWindow(FALSE);
					GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(FALSE);
					pReport->bUseFilter = FALSE;
				}
				else {
					if (m_GroupSelect->FromClause == (bstr_t)"GroupsT") {
						m_GroupSelect->FromClause = "FiltersT";
						CString strWhere;
						if(pReport->nSupportedSubfilter != -1) {
							strWhere.Format("Type = 1 OR Type = %li", pReport->nSupportedSubfilter);
						}
						else {
							strWhere = "Type = 1";
						}
						m_GroupSelect->WhereClause = _bstr_t(strWhere);
						m_GroupSelect->GetColumn(2)->PutFieldName(_bstr_t("Type"));

						//TES 3/26/2007 - PLID 20528 - We also need to show the CreatedDate and ModifiedDate columns, as well as
						// the column headers, and expand the dropdown width.
						m_GroupSelect->HeadersVisible = g_cvarTrue;
						m_GroupSelect->DropDownWidth = 400;
						m_GroupSelect->GetColumn(3)->ColumnStyle = m_GroupSelect->GetColumn(3)->ColumnStyle|csVisible;
						m_GroupSelect->GetColumn(4)->ColumnStyle = m_GroupSelect->GetColumn(4)->ColumnStyle|csVisible;

						m_GroupSelect->Requery();
					}
					//TES 5/30/03: Took this out, it should have just been taken care of.
					//m_GroupSelect->FromClause = "FiltersT";

					GetDlgItem(IDC_GROUPDL)->EnableWindow(TRUE);
					GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent));
					m_UseFilter.SetCheck(1);
					m_No_Group_Filter.SetCheck(0);
					m_UseGroup.SetCheck(0);
					//DRT 1/28/03 - This looks stupid (and it is), but here's the reason:  If a filter is chosen, but has no selection
					//			the pReport->nGroup is -1 (meaning no selection).  But for some great idea, we decided {New Filter} should
					//			have an ID of -1 (it really should be -2, or 0, or something that can't be an ID).  It's not *that* big of
					//			a deal to change it now, I have no idea how many other things I would have to change to do the same thing,
					//			so I'm just doing it this way.
					m_GroupSelect->SetSelByColumn(0, (long)(pReport->nFilterID == -1 ? -2 : pReport->nFilterID));
				}
			}
			else if (pReport->bUseGroup) {
				//TES 12/18/2008 - PLID 32514 - They're not allowed to use the Group filter
				// if they have the Scheduler Standard license.
				if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
					m_No_Group_Filter.SetCheck(1);
					m_UseFilter.SetCheck(0);
					m_UseGroup.SetCheck(0);
					GetDlgItem(IDC_GROUPDL)->EnableWindow(FALSE);
					GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(FALSE);
					pReport->bUseGroup = FALSE;
				}
				else {
					if (m_GroupSelect->FromClause == (bstr_t)"FiltersT") {
						m_GroupSelect->FromClause = "GroupsT";
						m_GroupSelect->WhereClause = "";
						m_GroupSelect->GetColumn(2)->PutFieldName("-1 AS Type");

						//TES 3/26/2007 - PLID 20528 - We also need to hide the CreatedDate and ModifiedDate columns, as well as
						// the column headers, and reduce the dropdown width.
						m_GroupSelect->HeadersVisible = g_cvarFalse;
						m_GroupSelect->DropDownWidth = -1;
						m_GroupSelect->GetColumn(3)->ColumnStyle = m_GroupSelect->GetColumn(3)->ColumnStyle & (~csVisible);
						m_GroupSelect->GetColumn(4)->ColumnStyle = m_GroupSelect->GetColumn(4)->ColumnStyle & (~csVisible);

						m_GroupSelect->Requery();
					}
					GetDlgItem(IDC_GROUPDL)->EnableWindow(TRUE);
					GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(FALSE);
					m_UseFilter.SetCheck(0);
					m_No_Group_Filter.SetCheck(0);
					m_UseGroup.SetCheck(1);
					//DRT 1/28/03 - This looks stupid (and it is), but here's the reason:  If a filter is chosen, but has no selection
					//			the pReport->nGroup is -1 (meaning no selection).  But for some great idea, we decided {New Filter} should
					//			have an ID of -1 (it really should be -2, or 0, or something that can't be an ID).  It's not *that* big of
					//			a deal to change it now, I have no idea how many other things I would have to change to do the same thing,
					//			so I'm just doing it this way.
					m_GroupSelect->SetSelByColumn(0, (long)(pReport->nGroup == -1 ? -2 : pReport->nGroup));
				}

			}
			else {
				m_No_Group_Filter.SetCheck(1);
				m_UseFilter.SetCheck(0);
				m_UseGroup.SetCheck(0);
				GetDlgItem(IDC_GROUPDL)->EnableWindow(FALSE);
				GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(FALSE);
				
			}
		}

		//set the edit button to be active so that they can edit the report
		//JMM 5/11/04 All reports are editable		
		//if (pReport->bEditable) {
			GetDlgItem(IDC_EDITREPORT)->EnableWindow(TRUE);
		//}
		//else {
		//	GetDlgItem(IDC_EDITREPORT)->EnableWindow(FALSE);
		//}

		// (j.gruber 2008-07-16 12:32) - PLID 28976 - Configure the all Years 
		if (pReport->strCategory == "PracAnalP") {
			GetDlgItem(IDC_REPORTS_ALL_YEAR)->ShowWindow(SW_SHOW);
			if (pReport->bUseAllYears) {
				CheckDlgButton(IDC_REPORTS_ALL_YEAR, 1);
			}
			else {
				CheckDlgButton(IDC_REPORTS_ALL_YEAR, 0);
			}	
		}
		else {
			GetDlgItem(IDC_REPORTS_ALL_YEAR)->ShowWindow(SW_HIDE);
		}

		// (j.gruber 2009-12-28 14:34) - PLID 19189 - apply filters button
		if (CurrReport->nLocation == 0 && CurrReport->nProvider == 0 && CurrReport->nDateRange == 0 && CurrReport->nPatient == 0) {
			GetDlgItem(IDC_REPORTS_APPLY_FILTERS)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_REPORTS_APPLY_FILTERS)->EnableWindow(TRUE);
		}



		EndWaitCursor();
		m_bIsLoading = false;

} // end LoadFilters

void CReports::LoadFormatFilters(CReportInfo *pReport) {

	if (pReport->nDetail == 0) {
		m_DetailCheck.EnableWindow(FALSE);
		m_SummaryCheck.EnableWindow(FALSE);
	}
	else {
		m_DetailCheck.EnableWindow(TRUE);
		m_SummaryCheck.EnableWindow(TRUE);
		if (pReport->nDetail == 1) {
			m_DetailCheck.SetCheck(1);
			m_SummaryCheck.SetCheck(0);
		}
		else if (pReport->nDetail == 2) {
			m_SummaryCheck.SetCheck(1);
			m_DetailCheck.SetCheck(0);
		}
	}
}


void CReports::LoadDateFilters(CReportInfo *pReport) {

	COleDateTime dateTmp, zerodate;
	zerodate.SetDate(1000,01,01);

	//OK, first off, let's check if there are date options to show.  If there are, they are ALWAYS
	//shown, even if date filtering is disabled.  Financial Activity - Monthly is a good example.
	m_pDateOptions->Clear();
	if(pReport->strDateOptions != "") {
		long nIndex = 0;
		while(nIndex < pReport->strDateOptions.GetLength() ) {
			//Load a record into the datalist.
			//First, the id.
			IRowSettingsPtr pRow = m_pDateOptions->GetRow(-1);
			long nSemicolon = pReport->strDateOptions.Find(";", nIndex);
			pRow->PutValue(0, (long)atoi(pReport->strDateOptions.Mid(nIndex, nSemicolon-nIndex)));
			nIndex = nSemicolon+1;

			//Now, the display name
			nSemicolon = pReport->strDateOptions.Find(";", nIndex);
			pRow->PutValue(1, _bstr_t(pReport->strDateOptions.Mid(nIndex, nSemicolon-nIndex)));
			nIndex = nSemicolon+1;

			//Now, the field name
			nSemicolon = pReport->strDateOptions.Find(";", nIndex);
			pRow->PutValue(2, _bstr_t(pReport->strDateOptions.Mid(nIndex, nSemicolon-nIndex)));
			nIndex = nSemicolon+1;

			//Finally the report suffix
			nSemicolon = pReport->strDateOptions.Find(";", nIndex);
			pRow->PutValue(3, _bstr_t(pReport->strDateOptions.Mid(nIndex, nSemicolon-nIndex)));
			nIndex = nSemicolon+1;

			m_pDateOptions->AddRow(pRow);
		}

		m_pDateOptions->SetSelByColumn(0, (long)pReport->nDateFilter);
		OnSelChosenDateOptions(m_pDateOptions->CurSel);
	}
	//Now show the datalist (if appropriate)
	if(m_pDateOptions->GetRowCount() > 1) {
		//Show the datalist.
		m_rDateRange.SetCheck(0);
		m_rDateRange.ShowWindow(SW_HIDE);

		m_rDateRangeOptions.ShowWindow(SW_SHOW);
		//We'll disable the radio button (not the datalist), for now, let the code below enable it as appropriate.
		m_rDateRangeOptions.EnableWindow(FALSE);
		//We'll check it if we're only doing one date, or if they've selected "Date Range"
		m_rDateRangeOptions.SetCheck(pReport->bOneDate || pReport->nDateRange > 0);
		ShowDlgItem(IDC_DATE_OPTIONS, SW_SHOW);
		GetDlgItem(IDC_DATE_OPTIONS)->EnableWindow();
		m_pDateOptions->SetSelByColumn(0, (long)pReport->nDateFilter);
		OnSelChosenDateOptions(m_pDateOptions->CurSel);
	}

	//Now, go on with the rest of the filter stuff
	if (!pReport->nDateRange) { // dates not enabled
		m_rAllDates.EnableWindow(FALSE);
		m_rDateRange.EnableWindow(FALSE);
		// (j.gruber 2008-07-11 13:08) - PLID 30692 - date quick filter
		GetDlgItem(IDC_REPORTS_QUICK_DATE_FILTER)->EnableWindow(FALSE);
		// (j.gruber 2008-07-22 09:42) - - PLID 28976 - All Years Filter
		GetDlgItem(IDC_REPORTS_ALL_YEAR)->ShowWindow(SW_HIDE);

		if(m_pDateOptions->GetRowCount() <= 1) {
			m_rDateRange.ShowWindow(SW_SHOW);
			m_rDateRangeOptions.ShowWindow(SW_HIDE);
			ShowDlgItem(IDC_DATE_OPTIONS, SW_HIDE);
		}
		else {
			m_rDateRange.ShowWindow(SW_HIDE);
			//The datalist is already shown; from above.
		}

		m_from.EnableWindow(FALSE);
		m_to.EnableWindow(FALSE);
	}
	else { // there is a date filter to load

		//Now that we know the datalist is correct, we shall proceed.
		if (pReport->bOneDate) { // in the event of a One Date box 
			m_to.EnableWindow(TRUE);
			m_rAllDates.EnableWindow(FALSE);
			// (j.gruber 2008-07-11 13:08) - PLID 30692 - date quick filter
			GetDlgItem(IDC_REPORTS_QUICK_DATE_FILTER)->EnableWindow(FALSE);
			// (j.gruber 2008-07-22 09:42) - - PLID 28976 - All Years Filter
			GetDlgItem(IDC_REPORTS_ALL_YEAR)->ShowWindow(SW_HIDE);
			
			//Which date stuff are we showing?
			if(m_pDateOptions->GetRowCount() <= 1) {
				//We didn't show the datalist, so show the single option.
				m_rDateRangeOptions.SetCheck(0);
				m_rDateRangeOptions.ShowWindow(SW_HIDE);
				ShowDlgItem(IDC_DATE_OPTIONS, SW_HIDE);

				SetDlgItemText(IDC_DATERANGE, VarString(m_pDateOptions->GetValue(0, 1)));

				m_rDateRange.ShowWindow(SW_SHOW);
				m_rDateRange.EnableWindow(TRUE);
				m_rDateRange.SetCheck(1);
			}
			else {
				m_rDateRange.SetCheck(0);
				m_rDateRange.ShowWindow(SW_HIDE);
				//We need to show and check the radio button
				m_rDateRangeOptions.ShowWindow(SW_SHOW);
				m_rDateRangeOptions.EnableWindow();
				m_rDateRangeOptions.SetCheck(1);
			}


				
//			pReport->nDateRange = 1;
			SetDlgItemText(IDC_LABEL_FROMDATE, "");
			SetDlgItemText(IDC_LABEL_TO_DATE, "As of: ");

			m_to.SetValue(COleVariant(pReport->DateTo));

			//Don't set the m_from date, because we don't use it anyway.
			m_rAllDates.SetCheck(0);
			m_from.EnableWindow (FALSE);
			
		}

		else { // all dates checked
			m_rAllDates.EnableWindow();
			m_rAllDates.SetCheck(pReport->nDateRange <= 0);
			// (j.gruber 2008-07-11 13:08) - PLID 30692 - date quick filter
			GetDlgItem(IDC_REPORTS_QUICK_DATE_FILTER)->EnableWindow(TRUE);
			if (pReport->nDateRange <= 0) {
				//set to all
				// (j.gruber 2008-07-11 13:08) - PLID 30692 - date quick filter
				m_pDateOptionList->SetSelByColumn(0, (long)-1);
				// (j.gruber 2008-07-22 09:42) - - PLID 28976 - All Years Filter
				GetDlgItem(IDC_REPORTS_ALL_YEAR)->EnableWindow(FALSE);
			}
			else {
				//set to custom
				// (j.gruber 2008-07-11 13:08) - PLID 30692 - date quick filter
				m_pDateOptionList->SetSelByColumn(0, (long)2);
				// (j.gruber 2008-07-22 09:42) - - PLID 28976 - All Years Filter
				GetDlgItem(IDC_REPORTS_ALL_YEAR)->EnableWindow(TRUE);
			}

			//Which date stuff are we showing?
			if(m_pDateOptions->GetRowCount() <= 1) {
				//We didn't show the datalist, so show the single option.
				m_rDateRangeOptions.SetCheck(0);
				m_rDateRangeOptions.ShowWindow(SW_HIDE);
				ShowDlgItem(IDC_DATE_OPTIONS, SW_HIDE);

				if(m_pDateOptions->GetRowCount() == 0)
					SetDlgItemText(IDC_DATERANGE, "**No Date Specified!**");
				else
					SetDlgItemText(IDC_DATERANGE, VarString(m_pDateOptions->GetValue(0, 1)));

				m_rDateRange.ShowWindow(SW_SHOW);
				m_rDateRange.EnableWindow(TRUE);
				m_rDateRange.SetCheck(pReport->nDateRange > 0);
			}
			else {
				m_rDateRange.SetCheck(0);
				m_rDateRange.EnableWindow(FALSE);
				m_rDateRange.ShowWindow(SW_HIDE);
				
				m_rDateRangeOptions.ShowWindow(SW_SHOW);
				m_rDateRangeOptions.EnableWindow();
				m_rDateRangeOptions.SetCheck(pReport->nDateRange > 0);
			}

			SetDlgItemText(IDC_LABEL_FROMDATE,"From: ");
			SetDlgItemText(IDC_LABEL_TO_DATE,"To: ");

			//DRT 1/28/03 - When Tom introduced his big change to the way date filters work, a problem appeared where the date fields were always
			//			being reset to todays date when you selected a report (even if you had previously set a date on them and switched off and back).
			//			I comment these lines out because the date boxes are already set elsewhere to a default date, and all this code 
			//			accomplishes is to screw up dates when you're moving around between selections.
			//			Then I added code to set the date to whatever is saved in that report object, since it is given a default value on 
			//			creation.  It now works much better.
//			COleVariant varNow(COleDateTime::GetCurrentTime());
//			m_from.SetValue(varNow);
//			m_to.SetValue(varNow);
			m_from.SetValue(_variant_t(pReport->DateFrom));
			m_to.SetValue(_variant_t(pReport->DateTo));
			EnableDateRange(pReport->nDateRange > 0);
		}

		/*else if (pReport->nDateRange > 0) { // date range is checked
			m_rAllDates.EnableWindow(TRUE);
			m_rDateRange.EnableWindow(TRUE);

			EnableDateRange(true);
			m_from.SetValue((COleVariant)pReport->DateFrom);
			m_to.SetValue((COleVariant)pReport->DateTo);

			m_rDateRange.SetCheck(1);
			m_rAllDates.SetCheck(0);
			SetDlgItemText(IDC_DATERANGE,pReport->strDateCaption);

			SetDlgItemText(IDC_LABEL_FROMDATE,"From: ");
			SetDlgItemText(IDC_LABEL_TO_DATE,"To: ");

		}*/

	}

	// if the extra date filter is enabled, e.g. Input/Service Dates
	/*if (pReport->nDateFilter) {
		EnableFilterDateOptions(true);
		if (pReport->nDateFilter == 2)
		{
			m_rServDate.SetCheck(0);
			m_rInputDate.SetCheck(1);
		}

		else {
			m_rServDate.SetCheck(1);
			m_rInputDate.SetCheck(0);
			pReport->nDateFilter = 1;
		}
	}
	else {
		EnableFilterDateOptions(false);
	}*/
}

void CReports::ResetFilters()
{
	CurrReport = NULL;

	m_DetailCheck.EnableWindow(FALSE);
	m_SummaryCheck.EnableWindow(FALSE);
	m_rAllDates.SetCheck(1);
	m_rDateRange.SetCheck(0);
	m_rDateRange.ShowWindow(SW_SHOW);
	m_rDateRangeOptions.SetCheck(0);
	m_rDateRangeOptions.ShowWindow(SW_HIDE);
	ShowDlgItem(IDC_DATE_OPTIONS, SW_HIDE);
	EnableDateRange(false);
	m_rAllDates.EnableWindow(FALSE);
	m_rDateRange.EnableWindow(FALSE);
	//EnableFilterDateOptions(false);
	// (j.gruber 2008-07-11 13:08) - PLID 30692 - date quick filter
	GetDlgItem(IDC_REPORTS_QUICK_DATE_FILTER)->EnableWindow(FALSE);
	// (j.gruber 2008-07-22 09:42) - - PLID 28976 - All Years Filter
	GetDlgItem(IDC_REPORTS_ALL_YEAR)->ShowWindow(SW_HIDE);
	// (j.gruber 2009-12-28 14:30) - PLID 19189 - Apply filters button
	GetDlgItem(IDC_REPORTS_APPLY_FILTERS)->EnableWindow(FALSE);
	

	m_UseExtended.EnableWindow(FALSE);
	ShowDlgItem(IDC_MULTI_EXT_LIST, SW_HIDE);
	ShowDlgItem(IDC_EXT_FILTER, SW_SHOW);
	EnableDlgItem(IDC_EXT_FILTER, FALSE);
	m_ExtFilterList->Clear();

	m_rAllPatients.EnableWindow(FALSE);
	m_rAllProviders.EnableWindow(FALSE);
	m_rAllLocations.EnableWindow(FALSE);
	m_rSelectPat.EnableWindow(FALSE);
	m_rSelectProv.EnableWindow(FALSE);
	m_rSelectLoc.EnableWindow(FALSE);
	ShowDlgItem(IDC_MULTI_PROV_LIST, SW_HIDE);
	ShowDlgItem(IDC_PROVSELECT, SW_SHOW);
	EnableDlgItem(IDC_PROVSELECT, FALSE);
	EnableDlgItem(IDC_PATSELECT, FALSE);
	EnableDlgItem(IDC_LOCATIONSELECT, FALSE);
	//(e.lally 2008-09-05) PLID 6780 - Follow the lead of the multi-provider
	ShowDlgItem(IDC_MULTI_LOC_LIST, SW_HIDE);
	ShowDlgItem(IDC_LOCATIONSELECT, SW_SHOW);
	GetDlgItem (IDC_CREATEMERGE)->EnableWindow(FALSE);

	m_No_Group_Filter.EnableWindow(FALSE);
	m_No_Group_Filter.SetCheck(1);
	m_UseFilter.EnableWindow(FALSE);
	m_UseFilter.SetCheck(0);
	m_UseGroup.EnableWindow(FALSE);
	m_UseGroup.SetCheck(0);
	GetDlgItem(IDC_GROUPDL)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(FALSE);

	SetDlgItemText(IDC_INGROUP, "");

	//set the edit button to be Inactive so that they can edit the report
	GetDlgItem(IDC_EDITREPORT)->EnableWindow(FALSE);

	//get rid of the report type list and put back the summary and detailed buttons
	GetDlgItem(IDC_REPORT_TYPE_LIST)->ShowWindow(SW_HIDE);
	m_DetailCheck.ShowWindow(SW_SHOW);
	m_SummaryCheck.ShowWindow(SW_SHOW);


	/* (z.manning, 10/28/05, PLID 18120)
	// This has nothing to do with reseting the filter.
	SetDlgItemText(IDC_CURRNAME,"None Selected");
	
	//Set the new tooltip.
	if(m_bToolTipsActive){
		m_ToolCtrl.DelTool(&m_NameBox, IDC_DESCFRAME2);
		m_ToolCtrl.AddTool(&m_NameBox, "None Selected");
		m_strCurrentToolText = "None Selected";
	}*/

	m_bIsLoading = false;
	
}

#include "Practice.h"

#define SET_PROGRESS_DESC(desc)		Progressdlg.SetDlgItemText(IDC_DESCRIPTION_LABEL, desc); Progressdlg.RedrawWindow()

BOOL CReports::PrintReport(CReportInfo *pReport, bool bPreview, CPrintInfo *pInfo /* =0 */)
{
	if (!pReport) {
		// We can't do anything if the pReport is NULL
		ASSERT(FALSE);
		HandleException(NULL, "CReports::PrintReport Error 1.  Could not print null report.", __LINE__, __FILE__);
		return FALSE;
	}

	// TODO: Eventually make this a parameter and make this function static
	CWnd *pParentWnd = this;

	CString StrTemp, 
			FormData, 
			SortBy,
			setSQL,
			RepID,
			RepFile;


	if(m_GroupSelect->CurSel == -1) {
		if(pReport->bUseGroup) {
			AfxMessageBox("There is no group selected. Please select a group or select 'No Filter/Group'.");
			return false;
		}
		else if(pReport->bUseFilter) {
			AfxMessageBox("There is no filter selected. Please select a group or select 'No Filter/Group'.");
			return false;
		}
	}

	// external form
	if (pReport->bExternal) {
		//TES 12/18/2008 - PLID 32514 - They can't use the external filter if they're
		// on the Scheduler Standard version.
		if(g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrSilent)) {
			CExternalForm Form(this);

			Form.m_RepID.Format("%i",pReport->nID);

			if(pReport->nDateRange == 1) {
				if(pReport->nID == 670) {
					// (z.manning 2009-07-20 17:32) - PLID 22054 - When loading the list of template IDs for
					// the Time Scheduled Productivity by Scheduler Template report, filter out any template
					// IDs that are not active as of the start date of the date range for this report.
					pReport->strListBoxSQL = FormatString(
						"12SELECT ID, Name FROM TemplateT \r\n"
						"WHERE ID IN (SELECT TemplateID FROM TemplateItemT WHERE EndDate >= '%s' OR EndDate IS NULL) \r\n"
						"ORDER BY Name \r\n"
						, FormatDateTimeForSql(pReport->DateFrom));
				}
			}

			Form.m_Caption		= pReport->strPrintName;
			Form.m_ColFormat	= pReport->strListBoxFormat;
			Form.m_ColWidths	= pReport->strListBoxWidths;
			StrTemp				= pReport->strListBoxSQL;
			Form.m_BoundCol		= atoi(StrTemp.Left(1));
			SortBy				= StrTemp.Left(2);
			Form.m_SortBy		= SortBy.Right(1);
			Form.m_SQL			= StrTemp.Right(StrTemp.GetLength()-2);
			Form.m_FilterField  = pReport->strRecordSource + "." + pReport->strFilterField;
			Form.m_Filter		= &(pReport->strExternalFilter);

			Form.m_RepID.Format("%li",pReport->nID);

			if (Form.DoModal() == IDCANCEL) return FALSE;
		}
		
	} // end external form

	//lets get to it
	BeginWaitCursor();
	
	// TODO: make sure this comment is true
	// We used to check to see if this was a crystal report, but 
	// now we only have crystal reports so just use the report path
	//This is true -JMM
	RepFile = pReport->strReportFile;
	RepFile.TrimLeft(); RepFile.TrimRight();

	BOOL bAns = TRUE;

	//crystal
	if (IsStatement(pReport->nID)) {
		
		long nStyle = GetRemotePropertyInt("SttmntEnvelope", 1, 0, "<None>") ;
		// (r.gonet 12/18/2012) - PLID 53629 - Save the statement output types so if they change
		//  while the report is running, then we will have a cache of their values at the time of
		//  the report being run. 
		// (r.goldschmidt 2014-08-05 14:43) - PLID 62717 - category type also
		pReport->nStatementType = nStyle;
		pReport->nOutputType = GetRemotePropertyInt("SttmntSendToHistory", 0, 0, "<None>", true);
		pReport->nCategoryType = GetRemotePropertyInt("SttmntSendToHistoryCategory", 0, 0, "<None>", true);
		switch (nStyle) {

			case 2:  //default report
				//If we are here then they have a default report checked, but no default report set.  Give them the warning...
				if (RepFile.IsEmpty()) {
					MsgBox("You have chosen to run a custom report, but you have no custom report chosen, please select a custom report and run this statement again");
					//return FALSE;
				}
			break;
			case 3:
				// (r.goldschmidt 2016-01-08 11:03) - PLID 67839 - by location e-statements are now allowed
				//check to see that it isn't a by provider report
				if (pReport->nID == 483  || pReport->nID == 484 || pReport->nID == 485 || pReport->nID == 486) {
					MsgBox("E-Statements are not available by provider. Please either choose a format other than E-Statement or run the E-Statement by patient or by location." );
					return FALSE;
				}
				else {
					//RepFile = VarString(m_pReportTypeList->GetValue(m_pReportTypeList->CurSel, 3));
					// (j.gruber 2008-09-24 10:04) - PLID 31485 - changed bIndiv Paramter to false
					//(e.lally 2011-03-03) PLID 42666 - Save the Use Extended since we try to reload the last one selected
					SetRemotePropertyText("SttmntExtendedFilter", pReport->GetExtraValue(), 0, "<None>");
					return RunEStatements(pReport, pReport->nDefaultCustomReport == -2, FALSE);						
				}
				// (j.gruber 2008-09-24 10:06) - 
			break;
			default:
				//do nothing
			break;
		}

		// (j.gruber 2007-03-12 16:05) - PLID 25148 - added a warning about location mismatches
		if (pReport->nExtendedStyle == 6) {
			//by location report

			//check to see if their location in the location filter matches the location in the extended filter
			CStringArray aryStr;
			pReport->GetExtraValues(aryStr);
			if (aryStr.GetSize() > 0) {

				if (pReport->nLocation != -1) {

					long nTopLocationID = pReport->nLocation;

					CString strTmp = aryStr.GetAt(0);
					long nResult = strTmp.Find(":");
					strTmp = strTmp.Left(nResult);
					long nBottomLocID = atoi(strTmp);

					if (nTopLocationID != nBottomLocID && (nBottomLocID != -1)) {

						if (MsgBox(MB_YESNO, "You are running a statement by location, but your location filter value is different than you balance filter location.  \n"
							"This could cause undesired results in the statement.  Are you sure you wish to continue?") == IDNO) {
							return FALSE;
						}
					}
				}
			}
		}
	}

	// (c.haag 2016-03-10) - PLID 68565 - Financial Activity - Today's Service Date is a special case where we always filter on today's date.
	if (pReport->nID == 603)
	{
		pReport->strDateFilterField = "TDate"; // TDate
		pReport->nDateRange = 2; // Date Range
		ADODB::_RecordsetPtr rs = CreateRecordset("SELECT dbo.AsDateNoTime(GetDate()) AS Now");
		pReport->DateFrom = pReport->DateTo = AdoFldDateTime(rs, "Now");
	}

	// (j.dinatale 2011-11-14 13:04) - PLID 45658 - need to check and see if we are running the for daily financial activity for 
	//		currently logged in user
	if(pReport->nID == 719){
		// filter on the current user
		CString strUserNameFilter;
		strUserNameFilter.Format(" ({DailyFinancialQ.InputName} IN ('%s')) ", _Q(GetCurrentUserName()));
		pReport->strExternalFilter = strUserNameFilter;

		// need to grab the date from the server, since we cant trust the local computer
		COleDateTime dtMin;
		ADODB::_RecordsetPtr rs = CreateRecordset("SELECT DateAdd(day, -7, dbo.AsDateNoTime(GetDate())) AS MinDate");

		if(!rs->eof){
			dtMin = AdoFldDateTime(rs->Fields, "MinDate", g_cdtInvalid);
		}

		rs->Close();

		// if we didnt specify a date range, or if for some reason either date is invalid, or either date is below our minimum, 
		//		we need to have the user correct it
		if(pReport->nDateRange == -1 
			|| pReport->DateFrom.GetStatus() != COleDateTime::valid || pReport->DateTo.GetStatus() != COleDateTime::valid 
			|| CompareDatesNoTime(pReport->DateFrom, dtMin) < 0 || CompareDatesNoTime(pReport->DateTo, dtMin) < 0)
		{
			AfxMessageBox("The Financial Activity - Daily For Logged On User report cannot be run with a From or To date earlier than 7 days from today. Please choose a valid date range on the following screen.");

			// show the dialog, and force the minimum date to be the date we got from the sql server
			CChooseDateRangeDlg dlgChooseDates(GetMainFrame());
			dlgChooseDates.SetMinDate(dtMin);
			dlgChooseDates.DoModal();

			// at this point, we have gotten valid dates from the dialog
			pReport->DateFrom = dlgChooseDates.GetFromDate();
			pReport->DateTo = dlgChooseDates.GetToDate();

			// (j.dinatale 2012-07-24 11:26) - PLID 50661 - also set the date range radio button
			pReport->nDateRange = 1;
		}

		// (j.dinatale 2012-07-24 11:09) - PLID 50661 - should be setting the date filter! not the date range option!
		// set the date range based on the selection in the date drop down.
		if (pReport->strDateFilterField == "BDate") {
			pReport->nDateFilter = 1;
		}
		else if (pReport->strDateFilterField == "TDate") {
			pReport->nDateFilter = 2;
		}
		else if (pReport->strDateFilterField == "IDate") {
			pReport->nDateFilter = 3;
		}

		// just in case the info in the reportinfo object changed, manually set our selection
		long nCurrSel = m_SelectList->CurSel;
		if(nCurrSel != -1){
			long nRepID = VarLong(m_SelectList->Value[nCurrSel][0], -1);
			if(nRepID == 719){
				OnSelChangedSelected(nCurrSel);
			}
		}
	}

	if (!RepFile.IsEmpty()) {
		
		CProcessRptDlg Progressdlg(this);
		CRect msgRect;
		Progressdlg.Create(IDD_PROCESSINGDLG, NULL);
		Progressdlg.ShowWindow(SW_SHOW);
		// find coordinates
		CWnd *pGrandParent = pParentWnd->GetParent();
		ASSERT(pGrandParent);
		if (pGrandParent) {
			// Get the window size and placement of the report's parent's 
			// parent (TODO: Is this really necessary?  I mean come on!)
			pGrandParent->GetWindowRect(msgRect);
		} else {
			// If getting the grandparent failed (who really expected 
			// that to work anyway) let's just use the parent
			pParentWnd ->GetWindowRect(msgRect);
		}
		msgRect.left = (msgRect.Width() / 2) - 100;
		msgRect.right = msgRect.left + 200;
		msgRect.top = (msgRect.Height() / 2) - 50;
		msgRect.bottom = msgRect.top + 100;
		Progressdlg.SetWindowPos(&wndTop, msgRect.left, msgRect.top, msgRect.Width(), msgRect.Height(), SWP_NOSIZE | SWP_NOZORDER);
		

		
		//check the default stuff
		if(pReport->nDefaultCustomReport > 0  && !IsStatement(pReport->nID)) {
			//it is a custom report, so we have to get the filename
			_RecordsetPtr rsFileName = CreateRecordset("SELECT Filename FROM CustomReportsT WHERE ID = %li AND Number = %li", pReport->nID, pReport->nDefaultCustomReport);
			if (rsFileName->eof) {
				MsgBox("Practice could not find the custom report.  Please contact NexTech for assistance");
				return FALSE;
			}
			else {
				RepFile = AdoFldString(rsFileName, "Filename");


				/* Moved to the view report function so that the PP reports could take advantage of it also
				long nVersion = AdoFldLong(rsFileName, "Version");
				if (nVersion != pReport->nVersion) {

					//the versions don't match, so we have to not let them print
					MsgBox("The version of this report does not match the version of the query.  Please contact NexTech Support.");
					return FALSE;
				}
				*/
			

			}
		}else {

			if (IsStatement(pReport->nID) ) {

				//check the statement stuff here because we might have to change the base name of the report
				if (pReport->nID == 169 || pReport->nID == 234 || pReport->nID == 337 || pReport->nID == 338 || pReport->nID == 483 || pReport->nID == 484) {

					//first check to see if they are already using the default report
					if (!GetRemotePropertyInt("SttmntUse70Version")) {
						
						if (DontShowMeAgain(NULL, "There is a new, simpler layout of the statement available. \nWould you like to set Version 7.0 as your default?", "PatientStatements", "Statements", true, TRUE) == IDYES) {

							//tell them how to change it back
							MsgBox("To set the statement back to the 6.0 version, go into Statement Configuration and uncheck the \"Use Version 7.0\" check box");


							//set the option in ConfigRT
							SetRemotePropertyInt("SttmntUse70Version", 1, 0, "<None>");

							//RepFile += "70";
							//PLID 15293 - Get the appropriate 70 report file
							RepFile = StmtConvertTo70Name(RepFile);
						}
					}
					else {
						//they are already using it, so set the report filename
						//RepFile += "70";
					//	RepFile = VarString(m_pReportTypeList->GetValue(m_pReportTypeList->CurSel, 3));
					}

				}		
				else {
					//get the report file from the dropdown
					//RepFile = VarString(m_pReportTypeList->GetValue(m_pReportTypeList->CurSel, 3));
				}
					
				
				//save the extended filter they are using 
				SetRemotePropertyText("SttmntExtendedFilter", pReport->GetExtraValue(), 0, "<None>");

				
				
				

			}


			//do the regular stuff
			//We ALWAYS want to add the suffix, just sometimes it will be blank.
			//if(pReport->nDateRange != 0) {
				RepFile += pReport->GetDateSuffix(pReport->nDateFilter);
				RepFile.TrimRight();
			//}
		
		
			if (!IsStatement(pReport->nID)){ 
				if (pReport->nDetail == 2) RepFile += "Smry";
				else if (pReport->nDetail == 1) RepFile += "Dtld";
			}

		}

		
		CRPEngine *RepEngine = CRPEngine::GetEngine();
		if (!RepEngine) RepEngine = GetMainFrame()->GetReportEngine();

		CString str;
		if (!IsStatement(pReport->nID)) {
			if (pReport->nDetail == 2) str = " Summary";
			else if (pReport->nDetail == 1) str = " Detailed";
			else str = "";
		}
		else {
			//set it based on the report file name
			RepFile.MakeLower();
			if (RepFile.Find("smry") > 0) {
				str = " Summary ";
			}
			else {
				str = " Detailed ";
			}
		}
			

		StrTemp.Format ("Report %i - %s", (RepEngine->GetNPrintJobs() + 1), pReport->strPrintName + str);
		SetReportParameters(RepFile, pReport);

		CString strGroupName;
		if (pReport->bUseGroup && m_GroupSelect->CurSel != -1) {
			strGroupName = VarString(m_GroupSelect->GetValue(m_GroupSelect->CurSel, 1));
		}		
		
		//Group is selected
		if (pReport->bUseGroup && strGroupName != "") {
			// Add the selected group to the filter
			CString strGroupFilter;
			if (GenerateGroupFilter(pReport, &strGroupFilter)) {
				if (!strGroupFilter.IsEmpty()) {
					pReport->strGroupFilter = "(" + strGroupFilter + ")";
				} else {
					pReport->strGroupFilter = _T("");
				}
			} else {
				pReport->strGroupFilter = _T("");
			}
		}
		else if (pReport->bUseFilter && !(m_GroupSelect->CurSel == 0 && VarString(m_GroupSelect->GetValue(m_GroupSelect->CurSel,1)) == "{New Filter}")) {
			CString strFilter;
			if (GenerateFilter(pReport, &strFilter)) {
				if (! strFilter.IsEmpty()) {
					pReport->strFilterString = "(" + strFilter + ")";
				}
				else {
					if(AfxMessageBox("There are no patients that match this filter, would you like to run the report on all patients?", MB_YESNO) == IDNO)
						return false;
					//well they want to run you anyways, so i suppose we'll let it slide
					pReport->strFilterString = _T("");
				}
			} 
			else {
				return false;
			}
		}
			
		//long nOldTimeout;
		//nOldTimeout = g_ptrRemoteData->CommandTimeout;

		//g_ptrRemoteData->CommandTimeout = 600;
		
		// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
		CIncreaseCommandTimeout cict(CReportInfo::IsServerSide(pReport->nID) ? GetRemoteDataReportSnapshot() : GetRemoteDataSnapshot(), 600);

		// TODO: Eventually the ELSE portion of this IF statement should be an error condition
		if (pReport->ViewReport(StrTemp, RepFile, bPreview, pParentWnd, pInfo)) {
			// Success  Do nothing
		} else {
			// TODO: Make this an error condition because the CReportInfo::ViewReport should succeed for all reports
			// For now this is not an error condition, so run the report the old fashioned way
			//tada
			
			// I'm not quite clear on what the above comments are meant to indicate, 
			// but it seems to me that we should at least be remembering to return 
			// FALSE if since ViewReport failed.
			// This is usually because the user clicked cancel on the print dialog
			bAns = FALSE;
		}

		//reset the timeout
		//g_ptrRemoteData->CommandTimeout = nOldTimeout;

		EndWaitCursor();
		Progressdlg.DestroyWindow();

		//make these two string empty por favor
		pReport->strGroupFilter = "";
		pReport->strFilterString = "";
	}

	return bAns;
}

void CReports::Print(bool bPreview, CPrintInfo *pInfo /* =0 */)
{
	try {
		// Check security
		CReportInfo *pRep;
		long i, nCount = m_SelectList->GetRowCount();

		// (c.haag 2009-12-18 12:16) - PLID 29264 - Gather a list of all unlicensed reports.
		// If any exist, forbid them from running the batch.
		CArray<CReportInfo*,CReportInfo*> apUnlicReports;
		for (i=0; i<nCount; i++) {
			pRep = FindReport(VarLong(m_SelectList->Value[i][0]));
			if (!CheckLicenseForReport(pRep, true)) 
			{
				apUnlicReports.Add(pRep);
			}
		}
		if (apUnlicReports.GetSize() > 0) {
			CString strLicWarn = "Your office is not licensed to run the following reports:\r\n\r\n";
			for (i=0; i < apUnlicReports.GetSize(); i++) {
				strLicWarn += apUnlicReports[i]->strPrintName + "\r\n";
				if (i >= 10 && i != apUnlicReports.GetSize() - 1) {
					strLicWarn += "<more>\r\n";
					break;
				}
			}
			strLicWarn += "\r\nPlease contact NexTech Systems for assistance.";
			AfxMessageBox(strLicWarn, MB_OK | MB_ICONSTOP);
			return;
		}

		//loop through all the selected reports
		for (i=0; i<nCount; i++) {
			pRep = FindReport(VarLong(m_SelectList->Value[i][0]));
			ASSERT(pRep);

			// need security?  do you have it? (if not this function gives the message for us)
			//PLID 14613 - we aren't forcing the permission check because we should have already 
			// checked that when they added it to the batch
			if (!CheckCurrentUserReportAccess(pRep->nID, false)) {
				return;
			}
		}
	} NxCatchAll("CReports::Print Error 1");
	
	try {		
		// Loop through all the reports in the "selected" list
		long nCount = m_SelectList->GetRowCount();
		CReportInfo *pRep;
		for (long i=0; i<nCount; i++) {
			// Print the report
			pRep = FindReport(VarLong(m_SelectList->Value[i][0]));
			ASSERT(pRep);
			if (!PrintReport(pRep, bPreview, pInfo)) {
				// If the report didn't print, the error has already 
				// been given to the user so just stop trying to print
				return;
			}
			//For the second through nCount reports, don't pop up the print dialog'
			if(pInfo){
				pInfo->m_bDirect = TRUE;
			}
		
		}
	} NxCatchAllCall("CReports::Print Error 3", {if(GetMainFrame()) GetMainFrame()->m_bIsReportRunning = false;});
}



void CReports::OnCreateMergeGroup() 
{
	if (!CurrReport) return;
	//if (CurrReport->strRecordSource.IsEmpty()) return;

	if (!UserPermission(EditGroup))
		return;

	BeginWaitCursor();


	try {
		//TES 7/17/03: All the stuff about getting the name from the user is now in the CreateGroup() report.
		
		//Change the timeout so we don't get an error.
		//long nOldTimeout;
		//nOldTimeout = g_ptrRemoteData->CommandTimeout;

		//g_ptrRemoteData->CommandTimeout = 600;
		
		// (a.walling 2009-08-11 13:25) - PLID 35178 - Use the snapshot connection
		CIncreaseCommandTimeout cict(GetRemoteDataSnapshot(), 600);

		//Make sure that the filter filter (that's not a typo) is loaded
		GenerateFilter(CurrReport, &(CurrReport->strFilterString));
		if(CurrReport->CreateGroup()) {
			//update the group checker
			m_groupChecker.Refresh();

			// Tell the user that the group has been successfully created
			MsgBox("Group has been created.");
			if(GetDlgItem(IDC_GROUPDL)->IsWindowEnabled()) {
				if((VarString(m_GroupSelect->FromClause)) == "GroupsT") {
					//PLID 21197 - this causes errors if there is no selection
					_variant_t vSelectedGroup;
					if (m_GroupSelect->GetCurSel() == -1) {
						vSelectedGroup.lVal = -1;
					}
					else {
						//m.hancock - 3/16/2006 - PLID 19750 - Retain the currently selected group
						vSelectedGroup = m_GroupSelect->GetValue(m_GroupSelect->CurSel, 0);
					}
					m_GroupSelect->Requery();
					//m.hancock - 3/16/2006 - PLID 19750 - Put the selection back
					m_GroupSelect->FindByColumn(0, vSelectedGroup, 0, VARIANT_TRUE);
				}
			}
		}
		
		//Reset the timeout
		//g_ptrRemoteData->CommandTimeout = nOldTimeout;

	} NxCatchAll("CReports::OnCreateMergeGroup");

	EndWaitCursor();
}

// CReportInfo functions
int CReports::RepSeek (long nReportID, int nFailValue /*= 0*/)
{
	for (int i = 0; i < m_RBatch.GetSize(); i++)
		if (((CReportInfo *)m_RBatch.GetAt(i))->nID == nReportID) return i;

	return nFailValue;
}

void CReports::SetReportParameters(CString csReport, CReportInfo *rpt)
{
	CRParameterInfo *paramInfo;
	CString tmp;

	//TS:  Let's clear the report's parameter list first, because you can run it twice with different parameters
	rpt->ClearParameterList();

	//Set Basic Report Header Parameters (These should be in all reports)
	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = GetCurrentUserName();
	paramInfo->m_Name = "CurrentUserName";
	rpt->AddToParameterList(paramInfo);

	/*TES 8/6/03: OK, after going through every .rpt file ("What?  EVERY .rpt file?  Are you kidding?"  
	//   "Would that I were."), I have determined that any report that uses the provid or provname filters
	//   will henceforth use only the provname filter, which will be blank if All Providers is selected, 
	//   otherwise it will be the provider name(s), or "{No Provider}"
	/*paramInfo = new CRParameterInfo;
	tmp.Format ("%li", rpt->nProvider);
	paramInfo->m_Data = tmp;
	paramInfo->m_Name = "ProvID";
	rpt->paramList.Add(paramInfo);*/

	paramInfo = new CRParameterInfo;
	tmp.Format ("%li", rpt->nLocation);
	paramInfo->m_Data = tmp;
	paramInfo->m_Name = "LocID";
	rpt->AddToParameterList(paramInfo);


	
	//add in the name of the provider if it exists
	CString strProvName;
	if(rpt->nProvider == 0 || rpt->nProvider == -1) {
		strProvName = "";
	}
	else if(rpt->nProvider == -2) {
		strProvName = "{No Provider}";
	}
	else {
		CString strIDList, strID;
		for(int i=0; i < rpt->m_dwProviders.GetSize(); i++) {
			strID.Format("%li, ", (long)rpt->m_dwProviders.GetAt(i));
			strIDList += strID;
		}
		strIDList = strIDList.Left(strIDList.GetLength()-2);
		_RecordsetPtr rsProvs = 
			CreateRecordset("SELECT ID, Last + ', ' + First + ' ' + Middle AS Name FROM PersonT "
			"INNER JOIN ProvidersT ON PersonT.Id = ProvidersT.PersonID WHERE PersonT.ID IN (%s) ORDER BY Last, First, Middle ASC", 
			strIDList);
		while(!rsProvs->eof) {
			strProvName += AdoFldString(rsProvs, "Name") + ", ";
			rsProvs->MoveNext();
		}
		rsProvs->Close();
		strProvName = strProvName.Left(strProvName.GetLength()-2);

		// (j.dinatale 2010-12-07) - PLID 41531 - Need to truncate the string because crystal can handle string parameters that are more than 254 characters long
		//	The reason why we are checking for a length of 250 and truncating to 241 characters is because most reports use a formula to append extra text 
		//	to the ProvName parameter. Usually what is added is "For " at the start of the provider list, so for sake of having it, 
		//	"...(more)" is 9 characters and "For " is 4 characters which means we will have 254 - 9 - 4 = 241 characters to work with with a max 
		//	string length of 254 - 4 = 250 characters to account for the "For " out in front.
		if(strProvName.GetLength() > 250){
			strProvName = strProvName.Left(241);
			strProvName += "...(more)";
		}
	}
	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = strProvName;
	paramInfo->m_Name = "ProvName";
	rpt->AddToParameterList(paramInfo);

	//add in the name of the location if it exists
	_RecordsetPtr rs = CreateRecordset("SELECT Name FROM LocationsT WHERE ID = %li", rpt->nLocation);
	paramInfo = new CRParameterInfo;
	//(e.lally 2008-09-18) PLID 31423 - Update the location name param to use the multi location and no location selections.
	CString strLocationName;
	if(rpt->nLocation > 0){
		strLocationName.Format("%s", CString(rs->Fields->Item["Name"]->Value.bstrVal));
	}
	else if(rpt->nLocation == -2){
		strLocationName = "{No Location}";
	}
	else if(rpt->nLocation == -3){
		CString strIDList, strID;
		for(int i=0; i < rpt->m_dwLocations.GetSize(); i++) {
			strID.Format("%li, ", (long)rpt->m_dwLocations.GetAt(i));
			strIDList += strID;
		}
		strIDList = strIDList.Left(strIDList.GetLength()-2);
		_RecordsetPtr rsLocs = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE LocationsT.ID IN (%s) ORDER BY Name ASC", strIDList);
		while(!rsLocs->eof) {
			strLocationName += AdoFldString(rsLocs, "Name") + ", ";
			rsLocs->MoveNext();
		}
		rsLocs->Close();
		strLocationName = strLocationName.Left(strLocationName.GetLength()-2);

		// (j.dinatale 2010-12-15) - PLID 41850 - Need to truncate the string because crystal can handle string parameters that are more than 254 characters long
		//	The reason why we are checking for a length of 234 and truncating to 225 characters is because most reports use a formula to append extra text 
		//	to the LocName parameter. Usually what is added is "Filtered Location:  " at the start of the provider list, so for sake of having it, 
		//	"...(more)" is 9 characters and "Filtered Location:  " is 20 characters which means we will have 254 - 9 - 20 = 225 characters to work with with a max 
		//	string length of 254 - 20 = 234 characters to account for the "Filtered Location:  " out in front.
		if(strLocationName.GetLength() > 234){
			strLocationName = strLocationName.Left(225);
			strLocationName += "...(more)";
		}
	}
	else{
		strLocationName.Empty();
	}
	paramInfo->m_Data = strLocationName;
	paramInfo->m_Name = "LocName";
	rpt->AddToParameterList(paramInfo);

	//to have a date header, add DateTo and DateFrom as string parameters to your report
	//a lot of reports use these parameters so be careful if you change anything
	if(rpt->nDateRange == 1)
	{
		tmp = rpt->GetDateName(rpt->nDateFilter);
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = "DateFilter";
		rpt->AddToParameterList(paramInfo);

		CString day,mo,yr;
		if(rpt->bOneDate)
		{
			rpt->DateTo.SetDateTime(rpt->DateTo.GetYear(), rpt->DateTo.GetMonth(), rpt->DateTo.GetDay(), 0, 0, 0);
			tmp = FormatDateTimeForInterface(rpt->DateTo, DTF_STRIP_SECONDS);
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = tmp;
			paramInfo->m_Name = "DateTo";
			rpt->AddToParameterList(paramInfo);
		}
		else 
		{
			rpt->DateFrom.SetDateTime(rpt->DateFrom.GetYear(), rpt->DateFrom.GetMonth(), rpt->DateFrom.GetDay(), 0, 0, 0);
			if (csReport == "PatientBDay")
			{
				tmp = rpt->DateFrom.Format("%m/%d");
			}
			else {
				tmp = FormatDateTimeForInterface(rpt->DateFrom, DTF_STRIP_SECONDS);
			}

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = tmp;
			paramInfo->m_Name = "DateFrom";
			rpt->AddToParameterList(paramInfo);

			//TES 9/14/2010 - PLID 25900 - Add a region-independent version of DateFrom, for reports (such as Financial Summary)
			// that use it for actual calculations, as opposed to just a display value.
			paramInfo = new CRParameterInfo;
			paramInfo->m_Name = "DateFromValue";
			paramInfo->m_Data = FormatDateTimeForSql(rpt->DateFrom, dtoDate);
			rpt->AddToParameterList(paramInfo);

			rpt->DateTo.SetDateTime(rpt->DateTo.GetYear(), rpt->DateTo.GetMonth(), rpt->DateTo.GetDay(), 0, 0, 0);
			if (csReport == "PatientBDay")
			{
				tmp = rpt->DateTo.Format("%m/%d");
			}
			else {
				tmp = FormatDateTimeForInterface(rpt->DateTo, DTF_STRIP_SECONDS);
			}

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = tmp;
			paramInfo->m_Name = "DateTo";
			rpt->AddToParameterList(paramInfo);

			//TES 9/14/2010 - PLID 25900 - Add a region-independent version of DateTo, for reports (such as Financial Summary)
			// that use it for actual calculations, as opposed to just a display value.
			paramInfo = new CRParameterInfo;
			paramInfo->m_Name = "DateToValue";
			paramInfo->m_Data = FormatDateTimeForSql(rpt->DateTo, dtoDate);
			rpt->AddToParameterList(paramInfo);
		}

	}
	else if(rpt->nDateRange == -1)
	{
		if(rpt->bOneDate)  //A date is _always_ select for bOneDate, so...
		{
			tmp = rpt->GetDateName(rpt->nDateFilter);
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = tmp;
			paramInfo->m_Name = "DateFilter";
			rpt->AddToParameterList(paramInfo);

			rpt->DateTo.SetDateTime(rpt->DateTo.GetYear(), rpt->DateTo.GetMonth(), rpt->DateTo.GetDay(), 0, 0, 0);
			tmp = FormatDateTimeForInterface(rpt->DateTo, DTF_STRIP_SECONDS);
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = tmp;
			paramInfo->m_Name = "DateTo";
			rpt->AddToParameterList(paramInfo);
		}
		else 
		{
			//TES 4/13/2005 - PLID 16220 - Why NOT pass in the name of the date filter?
			tmp = rpt->GetDateName(rpt->nDateFilter);
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = tmp;
			paramInfo->m_Name = "DateFilter";
			rpt->AddToParameterList(paramInfo);

			tmp = "01/01/1000";
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = tmp;
			paramInfo->m_Name = "DateFrom";
			rpt->AddToParameterList(paramInfo);

			//TES 9/14/2010 - PLID 25900 - We'll keep the region-independent value the same here, it's not a real date anyway, just
			// a signifier.
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = tmp;
			paramInfo->m_Name = "DateFromValue";
			rpt->AddToParameterList(paramInfo);

			tmp = "12/31/5000";
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = tmp;
			paramInfo->m_Name = "DateTo";
			rpt->AddToParameterList(paramInfo);		

			//TES 9/14/2010 - PLID 25900 - We'll keep the region-independent value the same here, it's not a real date anyway, just
			// a signifier.
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = tmp;
			paramInfo->m_Name = "DateToValue";
			rpt->AddToParameterList(paramInfo);
		}
	}

	// (j.gruber 2008-07-21 12:55) - PLID 28976 - Support All Years so we can show it in the report
	//add the all years
	if (rpt->bUseAllYears) {
		tmp = "TRUE";
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = "DateUseAllYears";
		rpt->AddToParameterList(paramInfo);
	}
	else {
		tmp = "FALSE";
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = "DateUseAllYears";
		rpt->AddToParameterList(paramInfo);
	}
	

	//Set up the time format.
	CString strTemp;
	NxGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, strTemp.GetBuffer(81), 81, true);
	strTemp.ReleaseBuffer();
	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = strTemp;
	paramInfo->m_Name = "TimeFormat";
	rpt->AddToParameterList(paramInfo);

	//Patient Statement
	if (IsStatement(rpt->nID)) {
		paramInfo = new CRParameterInfo;
		tmp.Format("%d", GetRemotePropertyInt("SttmntUseGuarantor"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"UseGuar";
		rpt->AddToParameterList(paramInfo);

		paramInfo = new CRParameterInfo;
		tmp.Format("%s", GetRemotePropertyMemo("SttmntName"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"ContactPerson";
		rpt->AddToParameterList(paramInfo);

		paramInfo = new CRParameterInfo;
		tmp.Format("%d", GetRemotePropertyInt("SttmntUseDocName"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"DocName";
		rpt->AddToParameterList(paramInfo);
		
		paramInfo = new CRParameterInfo;
		tmp.Format("%s", GetRemotePropertyMemo( "SttmntCallMe"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"StmmtTitle";
		rpt->AddToParameterList(paramInfo); 

		paramInfo = new CRParameterInfo;
		tmp.Format("%d", GetRemotePropertyInt("SttmntUseComma"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"UseComma";
		rpt->AddToParameterList(paramInfo);
	
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = GetRemotePropertyMemo("SttmntText",(CString)"-");
		paramInfo->m_Name = (CString)"CustomText";
		rpt->AddToParameterList(paramInfo);


		paramInfo = new CRParameterInfo;
		tmp.Format("%d", GetRemotePropertyInt("SttmntShowDiag"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"ShowDiag";
		rpt->AddToParameterList(paramInfo);

		paramInfo = new CRParameterInfo;
		tmp.Format("%d", GetRemotePropertyInt("SttmntAge"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"SttmntAge";
		rpt->AddToParameterList(paramInfo);

		paramInfo = new CRParameterInfo;
		tmp.Format("%d", GetRemotePropertyInt("SttmntShowFooter"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"SttmntFooter";
		rpt->AddToParameterList(paramInfo);


		//whether to use the Location address or the doctor's address
		paramInfo = new CRParameterInfo;
		tmp.Format("%d", GetRemotePropertyInt("SttmntUseDocAddress"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"DocAddress";
		rpt->AddToParameterList(paramInfo);

		//whether to hide prepayments or not
		paramInfo = new CRParameterInfo;
		tmp.Format("%d", GetRemotePropertyInt("SttmntHidePrePayments"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"HidePrePays";
		rpt->AddToParameterList(paramInfo);

		// (j.gruber 2007-01-05 10:37) - PLID 24036 - add combine bill balance info
		paramInfo = new CRParameterInfo;
		tmp.Format("%d", GetRemotePropertyInt("SttmntCombineBillsAfterXDaysOld", 0, 0, "<None>"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"SttmntCombineBillBalance";
		rpt->AddToParameterList(paramInfo);

		paramInfo = new CRParameterInfo;
		tmp.Format("%d", GetRemotePropertyInt("SttmntDaysOld", 60, 0, "<None>"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"SttmntDaysOld";
		rpt->AddToParameterList(paramInfo);

		// (j.gruber 2007-01-09 10:18) - PLID 24168  - add last pay show info
		paramInfo = new CRParameterInfo;
		tmp.Format("%d", GetRemotePropertyInt("SttmntShowLastPayInfo", 0, 0, "<None>"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"SttmntShowLastPayInfo";
		rpt->AddToParameterList(paramInfo);

		// (j.jones 2011-04-12 10:46) - PLID 31219 - added ability to show all charges
		// on any bills with balances, when on the summary statement
		paramInfo = new CRParameterInfo;
		tmp.Format("%li", GetRemotePropertyInt("SttmntIncludePaidCharges", 0, 0, "<None>"));
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = (CString)"SttmntIncludePaidCharges";
		rpt->AddToParameterList(paramInfo);

		//check to see whether we remit to a certain address
		long nRemit = GetRemotePropertyInt("SttmntUseRemit", 0, 0, "<None>");

		if (nRemit) {

			//get the location id
			long nLocationID = GetRemotePropertyInt("SttmntRemitLocation", -1, 0, "<None>");

			//get the name and address info for the location
			_RecordsetPtr rsLocation = CreateRecordset("SELECT Name, Address1, Address2, City, State, Zip FROM LocationsT WHERE ID = %li", nLocationID);
			CString strName, strAddress1, strAddress2, strCity, strState, strZip;

			if (rsLocation->eof) {

				//output an error message because they said to use remit but have an invalid location
				MsgBox("The location selected to remit to is invalid.  \nPlease check that this is a current location in the statement configuration and then run the statement again.");
			}
			else {

				strName = AdoFldString(rsLocation, "Name");
				strAddress1 = AdoFldString(rsLocation, "Address1");
				strAddress2 = AdoFldString(rsLocation, "Address2");
				strAddress2.TrimLeft();
				strAddress2.TrimRight();
				if (strAddress2.IsEmpty() ) {
					strAddress2 = "Empty";
				}
				strCity= AdoFldString(rsLocation, "City");
				strState= AdoFldString(rsLocation, "State");
				strZip= AdoFldString(rsLocation, "Zip");
			}

			//remit name
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = strName;
			paramInfo->m_Name = (CString)"SttmntRemitLocationName";
			rpt->AddToParameterList(paramInfo);

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = strAddress1;
			paramInfo->m_Name = (CString)"SttmntRemitLocationAddress1";
			rpt->AddToParameterList(paramInfo);

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = strAddress2;
			paramInfo->m_Name = (CString)"SttmntRemitLocationAddress2";
			rpt->AddToParameterList(paramInfo);
		
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = strCity;
			paramInfo->m_Name = (CString)"SttmntRemitLocationCity";
			rpt->AddToParameterList(paramInfo);

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = strState;
			paramInfo->m_Name = (CString)"SttmntRemitLocationState1";
			rpt->AddToParameterList(paramInfo);
		
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = strZip;
			paramInfo->m_Name = (CString)"SttmntRemitLocationZip";
			rpt->AddToParameterList(paramInfo);

		}
		else {

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "None";
			paramInfo->m_Name = (CString)"SttmntRemitLocationName";
			rpt->AddToParameterList(paramInfo);

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "None";
			paramInfo->m_Name = (CString)"SttmntRemitLocationAddress1";
			rpt->AddToParameterList(paramInfo);

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "None";
			paramInfo->m_Name = (CString)"SttmntRemitLocationAddress2";
			rpt->AddToParameterList(paramInfo);
		
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "None";
			paramInfo->m_Name = (CString)"SttmntRemitLocationCity";
			rpt->AddToParameterList(paramInfo);

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "None";
			paramInfo->m_Name = (CString)"SttmntRemitLocationState1";
			rpt->AddToParameterList(paramInfo);
		
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "None";
			paramInfo->m_Name = (CString)"SttmntRemitLocationZip";
			rpt->AddToParameterList(paramInfo);
		}

		//Credit Descriptions
		CString strPaymentDesc, strAdjustmentDesc, strRefundDesc;
		strPaymentDesc = GetRemotePropertyText("SttmntPaymentDesc", "", 0, "<None>");
		strAdjustmentDesc = GetRemotePropertyText("SttmntAdjustmentDesc", "", 0, "<None>");
		strRefundDesc = GetRemotePropertyText("SttmntRefundDesc", "", 0, "<None>");


		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strPaymentDesc;
		paramInfo->m_Name = (CString)"SttmntPaymentDesc";
		rpt->AddToParameterList(paramInfo);


		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strAdjustmentDesc;
		paramInfo->m_Name = (CString)"SttmntAdjustmentDesc";
		rpt->AddToParameterList(paramInfo);

		
		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strRefundDesc;
		paramInfo->m_Name = (CString)"SttmntRefundDesc";
		rpt->AddToParameterList(paramInfo);

		//Credit Card Acceptance
		CString strtmp;
		paramInfo = new CRParameterInfo;
		strtmp.Format("%d", GetRemotePropertyInt("SttmntAcceptVisa", 1, 0, "<None>"));
		paramInfo->m_Data = strtmp;
		paramInfo->m_Name = (CString)"AcceptVisa";
		rpt->AddToParameterList(paramInfo);

		paramInfo = new CRParameterInfo;
		strtmp.Format("%d", GetRemotePropertyInt("SttmntAcceptMstrCard", 1, 0, "<None>"));
		paramInfo->m_Data = strtmp;
		paramInfo->m_Name = (CString)"AcceptMstrCard";
		rpt->AddToParameterList(paramInfo);
		
		paramInfo = new CRParameterInfo;
		strtmp.Format("%d", GetRemotePropertyInt("SttmntAcceptDiscover", 1, 0, "<None>"));
		paramInfo->m_Data = strtmp;
		paramInfo->m_Name = (CString)"AcceptDiscover";
		rpt->AddToParameterList(paramInfo);

		paramInfo = new CRParameterInfo;
		strtmp.Format("%d", GetRemotePropertyInt("SttmntAcceptAmex", 1, 0, "<None>"));
		paramInfo->m_Data = strtmp;
		paramInfo->m_Name = (CString)"AcceptAmex";
		rpt->AddToParameterList(paramInfo);

		//AR Notes
		CString strThirty, strSixty, strNinety, strNinetyPlus;
		strThirty = GetRemotePropertyMemo ("Sttmnt30DayNote", "", 0, "<None>");
		strSixty = GetRemotePropertyMemo ("Sttmnt60DayNote", "", 0, "<None>");
		strNinety = GetRemotePropertyMemo ("Sttmnt90DayNote", "", 0, "<None>");
		strNinetyPlus = GetRemotePropertyMemo ("Sttmnt90+DayNote", "", 0, "<None>");

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strThirty;
		paramInfo->m_Name = (CString)"ThirtyDayNote";
		rpt->AddToParameterList(paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strSixty;
		paramInfo->m_Name = (CString)"SixtyDayNote";
		rpt->AddToParameterList(paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strNinety;
		paramInfo->m_Name = (CString)"NinetyDayNote";
		rpt->AddToParameterList(paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strNinetyPlus;
		paramInfo->m_Name = (CString)"NinetyPlusNote";
		rpt->AddToParameterList(paramInfo);
			
			

	}//end Patient statement if

	if(rpt->strReportFile == "SchAppts") {//Scheduler reports
		paramInfo = new CRParameterInfo;
		paramInfo->m_Name = "ShowLocName";
		paramInfo->m_Data = GetRemotePropertyInt("ResShowLocName", 0, 0, GetCurrentUserName(), true) ? "true" : "false";
		rpt->AddToParameterList(paramInfo);
	}

	//DRT 6/4/03 - You'd think there was an easier way to do this - apparently you can't add parameters from inside the
	//		GetSql...() functions with the query (which would be easier to find them IMO).
	//		Add one for the title of the Patient List report - if we filter, it might need to be called Prospect List, etc.
	if (rpt->nID == 14) {
		//DRT 6/4/03 - If there is an extended filter, send it in as a parameter
		CString strParam = "";
		if (rpt->bExtended) {
			//TES 8/22/2005 - There could be multiple
			CStringArray saExtraValues;
			rpt->GetExtraValues(saExtraValues);
			for(int i = 0; i < saExtraValues.GetSize(); i++) {
				switch(atoi(saExtraValues[i])){
					case 1:	strParam += "Patient and ";	break;
					case 2:	strParam += "Prospect and ";	break;
					case 3:	strParam += "Patient/Prospect and ";	break;
					default:	strParam += "";	break;
				}
			}
			if(!strParam.IsEmpty()) {
				ASSERT(strParam.Right(5) == " and ");
				strParam = strParam.Left(strParam.GetLength()-5);
			}
		}

		paramInfo = new CRParameterInfo;
		paramInfo->m_Name = "RptTitle";
		paramInfo->m_Data = strParam;
		rpt->AddToParameterList(paramInfo);
	}

	//JMM 7/21/03 Receipt parameters
	if (rpt->nID == 138 || rpt->nID == 325) {

		paramInfo = new CRParameterInfo;
		paramInfo->m_Name = "ReceiptShowChargeInfo";
		paramInfo->m_Data = GetRemotePropertyInt("ReceiptShowChargeInfo", 0, 0, "<None>", true) ? "true" : "false";
		rpt->AddToParameterList(paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Name = "ReceiptShowTax";
		paramInfo->m_Data = GetRemotePropertyInt("ReceiptShowTax", 1, 0, "<None>", true) ? "true" : "false";
		rpt->AddToParameterList(paramInfo);

		paramInfo = new CRParameterInfo;
		paramInfo->m_Name = "ReceiptCustomInfo";
		paramInfo->m_Data = GetRemotePropertyText("ReceiptCustomInfo", "", 0, "<None>", true);
		rpt->AddToParameterList(paramInfo);
	}

	//DRT 9/10/03 - Financial history general report preview
	if(rpt->nID == 446) {
		CRParameterInfo* paramInfo;
		CString tmp;
		paramInfo = new CRParameterInfo;
		tmp.Format ("%s", GetCurrentLocationName());
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = "ShowLocation";
		rpt->AddToParameterList(paramInfo);
	}

	// (a.walling 2007-03-22 14:32) - PLID 25113 - Add parameter of whether we are including expired certs or not
	if (rpt->nID == 499)
	{
		CRParameterInfo* paramInfo = new CRParameterInfo;
		paramInfo->m_Name = "ExpiredCertificates";
		CStringArray sa;
		rpt->GetExtraValues(sa);
		if (sa.GetSize() > 0) {
			paramInfo->m_Data = "included";
		} else {
			paramInfo->m_Data = "excluded";
		}
		
		rpt->AddToParameterList(paramInfo);
	}

	//TES 2/26/2003: Monthly Financial
	if(rpt->nID == 154) {
		CRParameterInfo* paramInfo;
		CString tmp;
		paramInfo = new CRParameterInfo;
		tmp.Format("%s", rpt->GetExtraValue());//Won't be multiple, because this report is flagged to not support multiple.
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = "MonthYearFilter";
		rpt->AddToParameterList(paramInfo);
	}

	//TES 11/11/2005 - PLID 18307 - Suppress the AR on Financial reports.
	if(rpt->nID == 154 /*Monthly Financial*/ || rpt->nID == 137 /*Financial Summary*/) {
		CRParameterInfo* paramInfo;
		paramInfo = new CRParameterInfo;
		// (d.thompson 2012-08-13) - PLID 51969 - Changed default to Yes
		paramInfo->m_Data = GetRemotePropertyInt("Reports_SuppressAR", 1, 0, "<None>", true) == 0 ? "0" : "1";
		paramInfo->m_Name = "SuppressAR";
		rpt->AddToParameterList(paramInfo);
	}

	//DRT 8/31/2005 - PLID 15996 - Tax Totals
	if(rpt->nID == 148) {
		CRParameterInfo* paramInfo;
		CString tmp;
		paramInfo = new CRParameterInfo;
		short nOption = 0;
		if(GetRemotePropertyInt("Rpt_ReturnUseTaxTotals", 1, 0, "<None>", true)) {
			//If we have chosen to use return data on the tax totals report
			if(GetRemotePropertyInt("Rpt_UseReturnProduct", 0, 0, "<None>", true)) {
				//If we have chosen to use the Return Product feature to specify our data
				nOption = 2;
			}
			// (d.thompson 2009-01-05) - PLID 32612 - Removed RetAdj in favor of !RetProduct
			else {
				//If we have chosen to use applied adjustments to specify our data
				nOption = 1;
			}
		}

		//0 = Not in use, 1 = Use applied adjustments, 2 = Use returned products
		tmp.Format("%li", nOption);
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = "ReturnMethod";
		rpt->AddToParameterList(paramInfo);
	}


	// (j.gruber 2011-12-22 16:43) - PLID 36465
	if (rpt->nID == 170 //AR by Insurance
		|| rpt->nID == 385 //AR by Insurance Co Office
		|| rpt->nID == 391  //AR split by Resp
		|| rpt->nID == 659 //AR by Fin Class		
		|| rpt->nID == 388 //AR by Patient Resp
	) {
		CString strFilter;
		if (rpt->nDateFilter == 3) {
			strFilter = "Payment Input Date";
		}
		else if (rpt->nDateFilter == 2) {
			strFilter = "Payment Date";
		}
		else {
			strFilter = "";
		}

		CRParameterInfo* paramInfo = new CRParameterInfo;
		paramInfo->m_Data = strFilter;
		paramInfo->m_Name = "PayDate";
		rpt->AddToParameterList(paramInfo);
	}

	// (a.walling 2007-01-16 16:40) - PLID 22764 - Marketing report filter parameter
	if (rpt->nID == 402 /* Marketing Cost */) {
		CRParameterInfo* paramInfo = new CRParameterInfo;

		paramInfo->m_Data = "";
		paramInfo->m_Name = "FilterDescription";
		rpt->AddToParameterList(paramInfo);
	}

	// (z.manning, 09/20/2007) - PLID 27467 - The procedure content report can now be run from admin tab
	// in reports module, so we need to set it parameters here.
	if(rpt->nID == 570 /* Procedure Content */)
	{
		CRParameterInfo* pProcedureContentParam = new CRParameterInfo;
		pProcedureContentParam->m_Data = "FALSE";
		pProcedureContentParam->m_Name = "ShowOneFieldOnly";
		rpt->AddToParameterList(pProcedureContentParam);

		pProcedureContentParam = new CRParameterInfo;
		pProcedureContentParam->m_Data = "<All Fields>";
		pProcedureContentParam->m_Name = "FieldToShow";
		rpt->AddToParameterList(pProcedureContentParam);

		// (z.manning, 09/20/2007) - PLID 27467 - When running this from the reports module, include the
		// procedure "cheat sheet" fields.
		pProcedureContentParam = new CRParameterInfo;
		pProcedureContentParam->m_Data = "TRUE";
		pProcedureContentParam->m_Name = "ShowCheatSheetFields";
		rpt->AddToParameterList(pProcedureContentParam);
	}

	// (j.jones 2008-02-14 14:03) - PLID 28864 - sent the ordering preference to the report,
	// so it can have a note that tells the user how the "needs ordered" conclusion was made
	// (j.jones 2008-02-21 09:40) - PLID 28982 - the physical inventory - serialized report needs
	// this parameter as well
	if(rpt->nID == 180		//Inventory Items To Be Ordered
		|| rpt->nID == 623) { //Physical Inventory - Serialized - Tally Sheet
		CRParameterInfo* paramInfo;
		CString tmp;
		paramInfo = new CRParameterInfo;
		long nOption = GetRemotePropertyInt("InvItem_OrderByOnHandAmount", 0, 0, "<None>", true);

		//0 = we compared "Actual" stock to the Reorderpoint
		//1 = we compared the "Available" stock to the Reorderpoint
		tmp.Format("%li", nOption);
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = "OrderRule";
		rpt->AddToParameterList(paramInfo);
	}

	// (j.jones 2008-03-14 10:03) - PLID 28951 - send the adv. inventory license as a parameter
	// this parameter as well
	if(rpt->nID == 323) {	//Serial Numbered / Expirable Products By Patient
		CRParameterInfo* paramInfo;
		CString tmp;
		paramInfo = new CRParameterInfo;
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		long nOption = g_pLicense->HasCandAModule(CLicense::cflrSilent) ? 1 : 0;

		//0 = they do not have the adv. inventory license
		//1 = they do have the license
		tmp.Format("%li", nOption);
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = "HasAdvInventory";
		rpt->AddToParameterList(paramInfo);
	}

	//TES 7/14/2008 - PLID 30668 - The Audit Trail - AR Issues report needs to know if changes to items input within the
	// date range have been filtered out.
	if(rpt->nID == 632) {	//Audit Trail - AR Issues
		CRParameterInfo* paramInfo;
		CString tmp;
		paramInfo = new CRParameterInfo;
		if(GetRemotePropertyInt("AuditTrail_ARIssues_OnlyPastItems", 1, 0, "<None>", true)) {
			tmp = "1";
		}
		else {
			tmp = "0";
		}
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = "ShowChangesToPastItems";
		rpt->AddToParameterList(paramInfo);
	}

	//(e.lally 2008-07-24) PLID 30732 - Added parameter for the date filter used to be shown in the report header.
	if(rpt->nID == 651) {	//Problem List by Patient
		CRParameterInfo* paramInfo;
		CString tmp;
		paramInfo = new CRParameterInfo;
		tmp = rpt->GetDateName(rpt->nDateFilter);
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = "DateFilterType";
		rpt->AddToParameterList(paramInfo);
	}

	// (j.gruber 2013-03-06 15:53) - PLID 55203
	if (rpt->nID == 628) {
		//do the same thing as the print preview
		//TES 6/18/2008 - PLID 30395 - Filter on all dates		
		CRParameterInfo *tmpParam;
		tmpParam = new CRParameterInfo;
		tmpParam->m_Name = "DateTo";
		tmpParam->m_Data = "12/31/5000";
		rpt->AddToParameterList(tmpParam);

		tmpParam = new CRParameterInfo;
		tmpParam->m_Name = "DateFrom";
		tmpParam->m_Data = "01/01/1000";
		rpt->AddToParameterList(tmpParam);		
	}

	// (j.gruber 2010-03-11 10:09) - PLID 37713 - added parameter for calculating AR
	if (rpt->nID == 137) {
		CRParameterInfo* paramInfo;
		CString tmp;
		paramInfo = new CRParameterInfo;
		if(GetRemotePropertyInt("FinancialSummaryQueryAR", 0, 0, "<None>")) {
			tmp = "1";
		}
		else {
			tmp = "0";
		}
		paramInfo->m_Data = tmp;
		paramInfo->m_Name = "QueryAR";
		rpt->AddToParameterList(paramInfo);
	}

	// (j.jones 2010-04-28 12:28) - PLID 35591 - added support for the patient history
	// report name, this won't be called if run from a print preview, which can renamd
	// the title to be Contact History
	if(rpt->nID == 407) {	//Patient History

		CRParameterInfo *paramInfo = new CRParameterInfo;
		CPtrArray paParams;
		paramInfo->m_Data = "Patient History";
		paramInfo->m_Name = "ReportTitle";
		rpt->AddToParameterList(paramInfo);
	}
}

BOOL CReports::GenerateGroupFilter(IN CReportInfo *pReport, OUT CString *pstrOutFilter /*= NULL*/, OUT long *pnItemCount /*= NULL*/)
{
	ASSERT(pReport);
	if (pReport && pReport->bUseGroupOption && pReport->bUseGroup) {
		try {
			// Do different things depending on what the caller wants
			if (pstrOutFilter) {
				// Try to open a recordset based on either the current lookup or the selected group
				CString strFilter;
				if (pReport->nGroup == 0) { 
					// TODO: Make this pull using the current lookup query instead of all patients
					strFilter.Format("%s.PatID IN (SELECT PersonID FROM PatientsT WHERE PersonID <> -25)", pReport->strRecordSource);
					/* I think this is the right way of fixing the above issue, but I don't have time to test it right now
					CMainFrame *pMainFrame = GetMainFrame();
					if (pMainFrame) {
						CString strWhere = pMainFrame->GetPatientFilter(false);
						prs = CreateRecordset("SELECT PersonID FROM PatientsT WHERE %s", strWhere.IsEmpty()?"PersonID <> -25":strWhere);
					}
					*/
				} else if (pReport->nGroup != -9999) {
					strFilter.Format(" %s.PatID IN (SELECT PersonID FROM GroupDetailsT WHERE GroupID = %li)", pReport->strRecordSource, pReport->nGroup);
				} else {
					strFilter.Format(" %s.PatID IN (SELECT PersonID FROM PatientsT WHERE PersonID <> -25)", pReport->strRecordSource);
				}

				// Try to use the recordset to generate the patient filter
				// Loop through the list adding each patient into the filter
				/*CString strFilter, strTemp;
				long nCount;
				FieldPtr fld = prs->Fields->Item["PersonID"];
				while (!prs->eof) {
					strTemp.Format("{%s.PatID} = %i ", pReport->strRecordSource, AdoFldLong(fld));
					strFilter += strTemp + " OR ";
					HR(prs->MoveNext());
					nCount++;
				}
				// Better to add one last " OR " and then remove 
				// it here, than check eof twice on every iteration
				if (strFilter.Right(4) == " OR ") {
					strFilter.Delete(strFilter.GetLength() - 4, 4);
				}
				*/

				// Return the result
				*pstrOutFilter = strFilter;
				/*if (pnItemCount) {
					*pnItemCount = nCount;
				}
				*/
			} else if (!pstrOutFilter && pnItemCount) {
				// Try to open a recordset based on either the current lookup or the selected group
				_RecordsetPtr prs;
				if (pReport->nGroup == 0) { 
					// TODO: Make this pull using the current lookup query instead of all patients
					prs = CreateRecordset("SELECT COUNT(PersonID) AS ItemCount FROM PatientsT WHERE PersonID <> -25");
				} else if (pReport->nGroup != -9999) {
					prs = CreateRecordset("SELECT COUNT(PersonID) AS ItemCount FROM GroupDetailsT WHERE GroupID = %li", pReport->nGroup);
				} else {
					prs = CreateRecordset("SELECT COUNT(PersonID) AS ItemCount FROM PatientsT WHERE PersonID <> -25");
				}

				// Get the count from the recordset
				ASSERT(prs != NULL && !prs->eof);
				if (!prs->eof) {
					*pnItemCount = AdoFldLong(prs, _T("ItemCount"));
				} else {
					return FALSE;
				}
			} else {
				// Both output parameters are null so do nothing
			}
			// Generated a group listing
			return TRUE;
		} NxCatchAll("CReports::GenerateGroupFilter");
	}

	// Didn't generate a group listing
	return FALSE;
}





void CReports::OnChangeGroup() 
{
	if (CurrReport && m_GroupSelect->GetRowCount() > 0) {
		try {
			// Create a recordset that just returns the ID of the currently named group
			_RecordsetPtr prs = CreateRecordset(
				"SELECT ID FROM GroupsT WHERE GroupsT.Name = '%s'",
				_Q(VarString(m_GroupSelect->GetValue(m_GroupSelect->CurSel, 1))));
			if (!prs->eof) {
				CurrReport->nGroup = AdoFldLong(prs, "ID");
			} else {
				// TODO: The old code didn't do this but I think that was a bug.  Just confirm that Practice2000 prior to 
				// november 2000 failed to correctly select "Current Filter" after a user group had already been selected
				// and you can take this comment out
				CurrReport->nGroup = 0;
			}
		} NxCatchAll("CReports::OnChangeGroup");
	}
	long nItemCount = 0;
	if (GenerateGroupFilter(CurrReport, NULL, &nItemCount)) {
		CString strCount;
		strCount.Format("%li In Group", nItemCount);
		SetDlgItemText(IDC_INGROUP, strCount);
		//InvalidateDlgItem(IDC_INGROUP);
	}
}

void CReports::InitGroupCombo()
{
	if (m_GroupSelect->GetRowCount() > 0) m_GroupSelect->Clear();

	// Set up combo box
	GetDlgItem(IDC_GROUPDL)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent));
	
	// Add built-in item (current filter)
	IRowSettingsPtr pRow = m_GroupSelect->GetRow(-1);
	pRow->PutValue(0, (long)-1);
	pRow->PutValue(1, _variant_t("***Current Patient Filter***"));
	m_GroupSelect->InsertRow(pRow, -1);
	
	try {
		// Open a recordset of all the groups
//		_RecordsetPtr prs = CreateRecordset(
//			"SELECT Name, ID FROM GroupsT");
		// Set up references to the few fields in the recordset
//		FieldsPtr flds = prs->Fields;
//		FieldPtr fldID = flds->Item["ID"];
//		FieldPtr fldName = flds->Item["Name"];
//		CString strTemp;
//		// Loop through each record adding a representative row to the combo for each
//		while (!prs->eof) {
//			strTemp.Format("%i", AdoFldLong(fldID));
//			m_cbGroup.AddItem(AdoFldString(fldName), m_cbGroup.GetListCount());
//			m_cbGroup.SetColumn((long)1, m_cbGroup.GetListCount()-(long)1, strTemp);
//			HR(prs->MoveNext());
//		}
	} NxCatchAllCall("CReports::InitGroupCombo", return);
}

CString GetReportNameFromReportFilename(LPCTSTR strReportFilename)
{
	CString strReportName = strReportFilename;
	if(strReportName.Right(4).CompareNoCase(".rpt") == 0) {
		strReportName.Delete(strReportName.GetLength() - 4, 4);
	}

	CStringArray arystrReportSuffixes;
	arystrReportSuffixes.Add("Smry");
	arystrReportSuffixes.Add("Dtld");
	arystrReportSuffixes.Add("Input");
	arystrReportSuffixes.Add("Service");

	while(true)
	{
		BOOL bFound = FALSE;
		for(int i = arystrReportSuffixes.GetSize() - 1; i >= 0; i--)
		{
			CString strSuffix = arystrReportSuffixes.GetAt(i);
			if(strReportName.GetLength() >= strSuffix.GetLength()) {
				if(strReportName.Right(strSuffix.GetLength()).CompareNoCase(strSuffix) == 0) {
					strReportName.Delete(strReportName.GetLength() - strSuffix.GetLength(), strSuffix.GetLength());
					bFound = TRUE;
					arystrReportSuffixes.RemoveAt(i);
				}
			}
		}

		if(!bFound) {
			break;
		}
	}

	return strReportName;
}

// (z.manning 2009-08-03 09:23) - You can use this function to automatically check and see if
// any reports fail to verify. Just call this from whereever you want and it will run and pop
// up a message box when it's done.
#ifdef ENABLE_VERIFY_ALL_REPORTS
#import "craxdrt.tlb" exclude("IFontDisp") rename("FindText","CRAXDRT_FindText")
void CheckIfAllReportsAreVerfied()
{
	CWaitCursor wc;
	BOOL bTTX = FALSE;
	CString strLastReportFile;
	try
	{
		CString strFailedReports, strManualVerify, strSkipped;
		// (z.manning 2009-08-03 09:26) - For reasons I was never able to figure out, this would not work
		// when doing all reports at once because Crystal would give an error about too many files being open.
		// I did get it to work by doing it in increments of 50 (although I had to skip report 120 (Adjustments)
		// as it never worked and I just did that manually).
		CArray<long,long> arynManualVerfiy;
		arynManualVerfiy.Add(120);
		arynManualVerfiy.Add(125);
		arynManualVerfiy.Add(297);
		for(short nReportIndex = 1; nReportIndex < 50/*CReportInfo::REPORT_NEXT_INFO_ID*/; nReportIndex++)
		{
			long nInfoIndex = CReportInfo::GetInfoIndex(nReportIndex);
			if(nInfoIndex == -1) {
				continue;
			}
			CReportInfo rptinfo(CReports::gcs_aryKnownReports[nInfoIndex]);

			// (z.manning 2009-08-03 09:28) - Statements were weird so I just checked them manually.
			if(IsStatement(rptinfo.nID) || IsIDInArray(rptinfo.nID, arynManualVerfiy)) {
				strManualVerify += rptinfo.strReportFile + "\r\n";
				continue;
			}

			CFileFind ff;
			BOOL bContinue = ff.FindFile(GetReportsPath() ^ rptinfo.strReportFile + "*.rpt");
			while(bContinue)
			{
				bContinue = ff.FindNextFile();
				if(ff.IsDirectory() || ff.IsDots()) {
					continue;
				}

				strLastReportFile = ff.GetFileName();

				// (z.manning 2011-02-09 12:06) - When looking for all the report files for a given report we may
				// get too many results returned (e.g. DailyFinancial will also return all the DailyFinancialCPT reports, etc.)
				// This is an attempt to reconcile that.
				if(rptinfo.strReportFile.CompareNoCase(::GetReportNameFromReportFilename(ff.GetFileName())) != 0) {
					strSkipped += ff.GetFileName() + " (expected " + rptinfo.strReportFile + ") \r\n";
					continue;
				}

				// (a.walling 2011-07-28 14:22) - PLID 44787 - Newer crystal runtimes set themselves as the CurVer but provide a non-backwards compatible interface.
				// So, request the 8.5 version, and fallback to version independent if that fails.
				CRAXDRT::IApplicationPtr  pApplication;
				pApplication.CreateInstance("CrystalDesignRuntime.Application.8.5");
				if (!pApplication) {
					pApplication.CreateInstance("CrystalDesignRuntime.Application");
				}
				if(pApplication == NULL) {
					AfxThrowNxException("CheckIfAllReportsAreVerfied - Could not load instance of Crystal app");
				}

				CRAXDRT::IReportPtr pReport;
				pReport = pApplication->OpenReport(_bstr_t(ff.GetFilePath()));

				if (pReport == NULL) {
					MsgBox("Unable to load report! You may try opening the report editor and clicking the verify button.");
					return;
				}

				SetCurrentDirectory(GetReportsPath());
				if (CreateAllTtxFiles(rptinfo.nID, GetReportsPath()))
				{
					// (z.manning 2009-12-07 11:54) - A lot of reports are still hard-codes to specific
					// paths so let's also create ttx files there so there's less that need to be verified
					// manually.
					if(FileUtils::DoesFileOrDirExist("C:\\PracStation")) {
						CreateAllTtxFiles(rptinfo.nID, "C:\\PracStation");
					}
					if(FileUtils::DoesFileOrDirExist("D:\\PracStation")) {
						CreateAllTtxFiles(rptinfo.nID, "D:\\PracStation");
					}

					bTTX = TRUE;
					//check to see that the ttx file name, is what we think it is
					CString strReportServerName = (LPCTSTR)pReport->Database->Tables->Item[1]->GetLogOnServerName();
					CString strTtxFileName;

					if (IsStatement(rptinfo.nID)) {
						strTtxFileName = GetStatementFileName(rptinfo.nID) + ".ttx";
					}
					else {
						strTtxFileName = rptinfo.strReportFile + ".ttx";
					}

					//if (strReportServerName.CompareNoCase(strTtxFileName) == 0)
					{
						long nDifferences = 1;
						try {
							pReport->Database->Tables->Item[1]->CheckDifferences(&nDifferences, NULL);
						}NxCatchAllCall("Error within CheckDifferences", nDifferences = 1);

						if(nDifferences != NULL) {
							strFailedReports += ff.GetFileName() + "\r\n";
						}
						
						// we are done with these
						rptinfo.DeleteTtxFiles();
						try {
							if(FileUtils::DoesFileOrDirExist("C:\\PracStation")) {
								DeleteFile("C:\\PracStation" ^ strTtxFileName);
							}
							if(FileUtils::DoesFileOrDirExist("D:\\PracStation")) {
								DeleteFile("D:\\PracStation" ^ strTtxFileName);
							}
						}NxCatchAllIgnore();
					}
					//else {
						//strManualVerify += ff.GetFileName() + "\r\n";
					//}
				}
				else {
					ASSERT(FALSE);
				}

				pReport.Release();
				pApplication.Release();
			}
		}

		CString strMessage;
		if(strFailedReports.IsEmpty() && strManualVerify.IsEmpty()) {
			strMessage = "All reports verfied\r\n";
		}
		else {
			strMessage = FormatString("The following reports failed to verify\r\n%s\r\n\r\n"
				"The following reports must be verfied manually...\r\n%s\r\n\r\n"
				"The following reports were skipped...\r\n%s\r\n\r\n"
				, strFailedReports, strManualVerify, strSkipped);
		}
		TRACE("%s", strMessage);
		CMsgBox dlgMsg(NULL);
		dlgMsg.msg = strMessage;
		dlgMsg.DoModal();
	
	}NxCatchAll("::VerifyAllReports, LastReport = " + strLastReportFile);
}
#endif

void CReports::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {

		CString strDefaultLocationWhereClause = "Managed = 1";

		// (j.jones 2010-06-14 16:53) - PLID 39117 - reload the report filter preferences,
		// rebuild the filter contents if they changed
		BOOL bOldFilterShowInactivePatients = m_bFilterShowInactivePatients;
		BOOL bOldFilterShowInactiveProviders = m_bFilterShowInactiveProviders;
		BOOL bOldFilterShowInactiveLocations = m_bFilterShowInactiveLocations;

		m_bFilterShowInactivePatients = GetRemotePropertyInt("ReportFilters_IncludeInactivePatients", 0, 0, "<None>", true) == 1;
		m_bFilterShowInactiveProviders = GetRemotePropertyInt("ReportFilters_IncludeInactiveProviders", 0, 0, "<None>", true) == 1;
		m_bFilterShowInactiveLocations = GetRemotePropertyInt("ReportFilters_IncludeInactiveLocations", 0, 0, "<None>", true) == 1;

		if (m_patientChecker.Changed() || bOldFilterShowInactivePatients != m_bFilterShowInactivePatients) {
			// (j.jones 2010-06-14 17:23) - PLID 39117 - show/hide inactive patients
			CString strPatWhere = "PatientsT.PersonID <> -25 AND PatientsT.CurrentStatus <> 4 ";
			if(!m_bFilterShowInactivePatients) {
				strPatWhere += "AND PersonT.Archived = 0";
			}
			m_PatSelect->PutWhereClause((LPCTSTR)strPatWhere);

			// (a.walling 2011-08-04 14:36) - PLID 44788 - Ensure the current selected patient fits the filters
			if (m_PatSelect->GetRowCount() != 0) {
				m_PatSelect->Clear();
			}
			
			if (m_nSelectedPatientID != -1) {
				if (!ReturnsRecordsParam("SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE {CONST_STR} AND PersonT.ID = {INT}", strPatWhere, m_nSelectedPatientID)) {
					ADODB::_RecordsetPtr prs = CreateRecordset("SELECT TOP 1 ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE %s ORDER BY Last, First, Middle", strPatWhere);
					if (prs->eof) {
						m_nSelectedPatientID = -1;
					} else {
						m_nSelectedPatientID = AdoFldLong(prs, "ID");
					}
				}
				
				if (m_nSelectedPatientID == -1) {
					m_PatSelect->CurSel = -1;
					m_PatSelect->PutComboBoxText("");
				} else if (m_PatSelect->TrySetSelByColumn(0, m_nSelectedPatientID) < 0) {
					m_PatSelect->PutComboBoxText((LPCTSTR)GetExistingPatientName(m_nSelectedPatientID));
				}
			}
		}
		if (m_doctorChecker.Changed() || bOldFilterShowInactiveProviders != m_bFilterShowInactiveProviders) {
			//DRT 5/25/2004 - PLID 12561 - We can't just call
			//	"Requery", otherwise the multiple and none 
			//	won't get loaded into the list.
			RequeryProviderList();
		}
		// The "group select" datalist may not have a FROM clause, and if it doesn't then it should not be requeried lest we get an error
		if (m_groupChecker.Changed() && !CString((LPCTSTR)m_GroupSelect->GetFromClause()).IsEmpty()) {
			m_GroupSelect->Requery();
		}

		if (m_filterChecker.Changed() && IsDlgButtonChecked(IDC_USEFILTER)) {
			m_GroupSelect->Requery();
		}

		//check to see if we need to update the report list
		if (m_SelectList->CurSel != -1) {
			long nID = VarLong(m_SelectList->GetValue(m_SelectList->GetCurSel(), 0));
			CReportInfo *pReport = FindReport(nID);

			// (j.jones 2010-07-19 09:56) - PLID 39117 - use the default location where clause,
			// to potentially re-filter the location list later on in this function
			strDefaultLocationWhereClause = pReport->GetLocationWhereClause();
			
			if (GetDlgItem(IDC_REPORT_TYPE_LIST)->IsWindowVisible()) {
				// (j.gruber 2010-03-11 12:30) - PLID 29120 - don't reset
				SetReportList(m_pReportTypeList, nID, pReport, FALSE);
			}
		}

		// (j.jones 2010-07-19 09:56) - PLID 39117 - moved the location requery here because the
		// location where clause may change based on the currently selected report
		if (m_locationChecker.Changed() || bOldFilterShowInactiveLocations != m_bFilterShowInactiveLocations) {
			RequeryLocationList(strDefaultLocationWhereClause);
		}

		// TODO: I don't know why we don't do this anymore, but then again I don't know why we ever did
//		m_ReportList->Requery();
//		m_SelectList->Requery();
	} NxCatchAll("CReports::UpdateView Error 1.  Could not update window.");
}

//controls saving the currently selected item, requerying the list, 
//waiting until the requery is done (providers are a small list), 
//adding the "none" and "multiple" options, then reselecting the item.
void CReports::RequeryProviderList()
{
	try {
		//Select the current item (-1 if nothing selected)
		long nSaved = -1;
		long nSel = m_ProvSelect->GetCurSel();
		if(nSel != sriNoRow)
			nSaved = VarLong(m_ProvSelect->GetValue(nSel, 0));

		// (j.jones 2010-06-14 17:23) - PLID 39117 - show/hide inactive providers
		CString strProvWhere = "";
		if(!m_bFilterShowInactiveProviders) {
			strProvWhere = "PersonT.Archived = 0";
		}

		CString strOldWhereClause = (LPCTSTR)m_ProvSelect->GetWhereClause();
		//if we are toggling between inactive & active, and multiple were selected, revert to all
		if(!m_bFilterShowInactiveProviders && strOldWhereClause != strProvWhere && nSaved == -3) {
			nSaved = -1;
		}

		m_ProvSelect->PutWhereClause((LPCTSTR)strProvWhere);

		//Requery the list
		m_ProvSelect->Requery();

		//Add the appropriate "special" rows
		//-2 = "<No Provider>" selected, -3 = "<Multiple Providers>" selected, 
		IRowSettingsPtr pRow;
		pRow = m_ProvSelect->GetRow(-1);
		pRow->PutValue(0, (long)-3);
		pRow->PutValue(1, _bstr_t(" {Multiple Providers}"));
		m_ProvSelect->InsertRow(pRow, 0);

		pRow = m_ProvSelect->GetRow(-1);
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _bstr_t(" {No Provider}"));
		m_ProvSelect->InsertRow(pRow, 0);

		//Finally, reselect the correct item
		long nReselectedRow = -1;
		if(nSaved != -1) {
			nReselectedRow = m_ProvSelect->SetSelByColumn(0, (long)nSaved);
		}
		
		if(nReselectedRow == -1) {
			m_rSelectProv.SetCheck(FALSE);
			EnableDlgItem(IDC_PROVSELECT, FALSE);
			m_rAllProviders.SetCheck(TRUE);
			
			ShowDlgItem(IDC_MULTI_PROV_LIST, SW_HIDE);
			ShowDlgItem(IDC_PROVSELECT, SW_SHOW);
		}

	} NxCatchAll("Error requerying provider list.");
}


//(e.lally 2008-09-05) PLID 6780 - Copied from providers and updated for the use of locations.
//controls saving the currently selected item, requerying the list, 
//waiting until the requery is done (locations are a small list), 
//adding the "none" and "multiple" options, then reselecting the item.
// (j.jones 2010-07-19 09:54) - PLID 39117 - this now takes in a default where clause
void CReports::RequeryLocationList(CString strDefaultWhereClause)
{
	try {
		//Select the current item (-1 if nothing selected)
		long nSaved = -1;
		long nSel = m_LocationSelect->GetCurSel();
		if(nSel != sriNoRow)
			nSaved = VarLong(m_LocationSelect->GetValue(nSel, 0));

		// (j.jones 2010-06-14 17:23) - PLID 39117 - show/hide inactive locations
		CString strLocWhere = strDefaultWhereClause;
		if(!m_bFilterShowInactiveLocations) {
			strLocWhere += " AND LocationsT.Active = 1";
		}

		CString strOldWhereClause = (LPCTSTR)m_LocationSelect->GetWhereClause();
		//if we are toggling between inactive & active, and multiple were selected, revert to all
		if(!m_bFilterShowInactiveLocations &&  strOldWhereClause != strLocWhere && nSaved == -3) {
			nSaved = -1;
		}

		m_LocationSelect->PutWhereClause((LPCTSTR)strLocWhere);

		//Requery the list
		m_LocationSelect->Requery();

		//Add the appropriate "special" rows
		//-2 = "<No Location>" selected, -3 = "<Multiple Locations>" selected, 
		IRowSettingsPtr pRow;
		pRow = m_LocationSelect->GetRow(-1);
		pRow->PutValue(0, (long)-3);
		pRow->PutValue(1, _bstr_t(" {Multiple Locations}"));
		m_LocationSelect->InsertRow(pRow, 0);

		pRow = m_LocationSelect->GetRow(-1);
		pRow->PutValue(0, (long)-2);
		pRow->PutValue(1, _bstr_t(" {No Location}"));
		m_LocationSelect->InsertRow(pRow, 0);

		//Finally, reselect the correct item
		long nReselectedRow = -1;
		if(nSaved != -1) {
			nReselectedRow = m_LocationSelect->SetSelByColumn(0, (long)nSaved);
		}
		
		if(nReselectedRow == -1) {
			m_rSelectLoc.SetCheck(FALSE);
			EnableDlgItem(IDC_LOCATIONSELECT, FALSE);
			m_rAllLocations.SetCheck(TRUE);

			ShowDlgItem(IDC_MULTI_LOC_LIST, SW_HIDE);
			ShowDlgItem(IDC_LOCATIONSELECT, SW_SHOW);
		}

	} NxCatchAll("Error requerying location list.");
}

void CReports::OnClose() 
{
	//This function never gets called (how quaint!) so this code is now in CheckAllowClose
	/*try{
		CString strCurList;
		BuildIDList(m_SelectList, strCurList);
		SetRemotePropertyText(CString("Reports.Batches.CurrentBatch"), strCurList, 0, GetCurrentUserName());
	}
	NxCatchAll("CReports::OnDestroy Error 1.  Could not save report batch list.");*/
	CNxDialog::OnClose();
}

void CReports::OnDestroy() 
{
	//destroy the tooltip window if it's there
	DestroyTT();

	CNxDialog::OnDestroy();
}


void CReports::LoadBatch(LPCTSTR strBatchName, LPCTSTR strUserName)
{
	try {

		// Get the list of reports that belong in this batch
		// (j.luckoski 2012-10-03 17:15) - PLID 24684 - Get the batch from a memo and not text.
		CString strRepList = GetRemotePropertyMemo(CString("Reports.Batches.") + strBatchName, "", 0, strUserName, false);
		CStringArray aryReports;
		long nId;
		
		CString strPermissionMsg = "The following reports could not be added to the batch because you don't have permission to run them:\n";
		BOOL bShowPermMsg = FALSE;
		
		// Unload the current batch
		OnClearSelect();

		// This assumes a ccomma delimited and terminated list
		ParseDelimitedStringToStringArray(strRepList, ",", aryReports);
		
		for (int i=0;i < aryReports.GetSize();i++) {
			
			CString strTemp = aryReports.GetAt(i);
			CDWordArray aryIDs;

			// This assumes reports also have a ; delimited list of provider IDs
			ParseDelimitedStringToDWordArray(strTemp, ";", aryIDs);

			// The first however is the report ID
			nId = (long)aryIDs.GetAt(0);

			// Make sure it's a valid integer
			ASSERT(nId > 0);
			if (nId > 0) {
				//we have to get the report, then check if it has permissions to be run before we add it to the list

				//If you don't do this, all hell will break loose because it won't know what the .rpt files are named.
				CReportInfo *pRep = FindReport(nId);

				//DRT 2/24/2004 - PLID 11081 - If, somehow, ConfigRT got an invalid report ID, then this could still be null!
				if(pRep) {

					//check to see if we have permision to this report
					if (CheckCurrentUserReportAccess(pRep->nID, TRUE, TRUE)) {

						// It's valid so add it to the select list
						AddReport(nId, FALSE);
						
						// (j.luckoski 2012-10-03 17:19) - PLID 24684 - Add the vector stored provIds into the reports array
						
						//Clear the loaded reports providers as merging will confuse users
						// (f.gelderloos 2013-09-05 10:02) - PLID 58343 Bug fix, nProvider was not resetting
						// (j.kuziel 2013-11-11) - PLID 59386 - Only reset to all providers if provider-filtering is applicable.
						pRep->m_dwProviders.RemoveAll();
						if(pRep->nProvider != 0) {
							pRep->nProvider = -1;
						}
						
						// Go through starting at 1 to grab remaining IDs which are providers
						for (int r=1;r<aryIDs.GetSize();r++) {

							pRep->nProvider =aryIDs.GetAt(r);
							pRep->m_dwProviders.Add((DWORD)pRep->nProvider);
						}
					
						//DRT 3/7/03 - After some more investigating (and a bug found), we have determined that my below comment was foolish.  
						//			Turns out that if you have a OneDate (well, the daily batch, not all of them), it would not get the date 
						//			loaded, and when you run the report (without having clicked on it to LoadFilters), it would not set the
						//			nDateFilter variable, so it would return the date field as "".  We commented that back in, which slows things
						//			down a little bit, and then added a line below this loop to set the selection to the first item in the list.
						//DRT 1/28/03 - After some changes to the LoadFilters function (which do all this new fangled date listbox stuff), the
						//			function is incredibly slow.  As far as my testing has indicated, this function is also completely useless
						//			in this context.  It's called when you click on each report anyways, so why call it for every report we load?
						LoadFilters(pRep);

						//DRT 4/28/03 - If they have the preference set, and we're looking at the daily batch, we want to set the date filter to today
						//		for each item in this report batch.
						if((CString(strBatchName).CompareNoCase("Daily") == 0) && (GetRemotePropertyInt("DailyBatchDefaultDate", 1, 0, "<None>", true) == 1)) {
							//if not a OneDate and they are actually using the date range field
							if(!pRep->bOneDate && pRep->nDateRange != 0) {
								//comments say this should be 2, but the code that actually adds the parameters to the report says that it needs to be 1...crazy
								pRep->nDateRange = 1;
								pRep->nDateFilter = 1;
								pRep->DateFrom = COleDateTime::GetCurrentTime();
								pRep->DateTo = COleDateTime::GetCurrentTime();
							}
						}
					}
					else {
						//they don't have permission to run the report so we are going to give a message at the end of the function
						//first, set the bool to show the message
						bShowPermMsg = TRUE; 

						//next, format the message with the report name in it
						strPermissionMsg += pRep->strPrintName + "\n";
					}
				}
				else {
					//DRT 2/24/2004 - PLID 11081 - The report can't be found.  This is not a big deal, we just won't try to load it.
					//	The loop will continue and load any valid reports that it finds.
				}
			} else {
				// It's not valid so throw an exception
				AfxThrowNxException("Unexpected report ID value of %li.  Ensure that the report ID is a valid report ID.", nId);
			}

		}

		//now if any of the reports don't have permission, tell them about it
		if (bShowPermMsg) {

			MessageBox(strPermissionMsg, "NexTech", MB_OK);
		}

		//set the selection to the first item
		OnSelChangedSelected(-1);
	} NxCatchAll("CReports::OnInitDialog Error 1.  Could not load report batch list.");
}

void CReports::OnUsefilter() 
{
	//TES 12/18/2008 - PLID 32514 - They're not allowed to use this filter
	// if they have the Scheduler Standard license.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Filtering reports based on letter writing filters", "The_Reports_Module/run_reports.htm")) {
		m_No_Group_Filter.SetCheck(1);
		m_UseFilter.SetCheck(0);
		m_UseGroup.SetCheck(0);
		CurrReport->bUseGroup = FALSE;
		CurrReport->bUseFilter = FALSE;
		m_GroupSelect->CurSel = -1;
		GetDlgItem(IDC_GROUPDL)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(FALSE);
		//I changed this to just blank out the box because I think that looks better
		SetDlgItemText(IDC_INGROUP, "");
		m_nCurrentGroup = -1;
		return;
	}

	GetDlgItem(IDC_GROUPDL)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent));
	//I changed this to blank out the box because I think that looks better -JMM
	SetDlgItemText(IDC_INGROUP, "");
	//DRT 1/28/03 - Just blanking out the # in group doesnt do it, it is still showing the name of the last one selected
	//			and giving false illusions.
	m_GroupSelect->PutCurSel(-1);
	//GetDlgItem(IDC_INGROUP)->EnableWindow(FALSE);

	CurrReport->bUseFilter = TRUE;
	CurrReport->bUseGroup = FALSE;
	
	m_GroupSelect->FromClause = "FiltersT";
	CString strWhere;
	if(CurrReport->nSupportedSubfilter != -1) {
		strWhere.Format("Type = 1 OR Type = %li", CurrReport->nSupportedSubfilter);
	}
	else {
		strWhere = "Type = 1";
	}
	m_GroupSelect->WhereClause = _bstr_t(strWhere);
	m_GroupSelect->GetColumn(2)->PutFieldName(_bstr_t("Type"));

	//TES 3/26/2007 - PLID 20528 - We also need to show the CreatedDate and ModifiedDate columns, as well as
	// the column headers, and expand the dropdown width.
	m_GroupSelect->HeadersVisible = g_cvarTrue;
	m_GroupSelect->DropDownWidth = 400;
	m_GroupSelect->GetColumn(3)->ColumnStyle = m_GroupSelect->GetColumn(3)->ColumnStyle|csVisible;
	m_GroupSelect->GetColumn(4)->ColumnStyle = m_GroupSelect->GetColumn(4)->ColumnStyle|csVisible;

	m_nCurrentGroup = m_GroupSelect->CurSel == -1 ? -1 : VarLong(m_GroupSelect->GetValue(m_GroupSelect->CurSel, 0));
	m_GroupSelect->Requery();

/*	DRT 5/25/2004 - PLID 12563 - This code was redundant.  We call the requery, then the RequeryFinished was
	//	already handling re-adding the {New Filter} row (though poorly, I also fix that).  However, this code 
	//	was attempting to add that row itself as well, and causing some issues.  This has been removed, and
	//	the RequeryFinished() is now fully handling adding the {New Filter} row.

	//
	//TES 9/11/03: If they don't have a license for letter writing, don't allow them to add new filters from
	//here.  Note: We are intentionally allowing them to use any filters they already happen to have, because
	//a.) if they have any that they've been using, it's sort of our fault that we let them create them, so 
	//we won't take them away, and b.) hopefully this will tempt people into buying letter writing.
	//
	IRowSettingsPtr pRow;
	if(g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent)) {
		pRow = m_GroupSelect->GetRow(-1);
		pRow->PutValue(0, long(-1));
		pRow->PutValue(1, _variant_t("{New Filter}"));
		pRow->PutValue(2, (long)-1);
	}
	if(g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent)) {
		m_GroupSelect->InsertRow(pRow, 0);
	}
*/	
}

void CReports::OnUsegroup() 
{
	//TES 12/18/2008 - PLID 32514 - They're not allowed to use this filter
	// if they have the Scheduler Standard license.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Filtering reports based on letter writing groups", "The_Reports_Module/run_reports.htm")) {
		m_No_Group_Filter.SetCheck(1);
		m_UseFilter.SetCheck(0);
		m_UseGroup.SetCheck(0);
		CurrReport->bUseGroup = FALSE;
		CurrReport->bUseFilter = FALSE;
		m_GroupSelect->CurSel = -1;
		GetDlgItem(IDC_GROUPDL)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(FALSE);
		//I changed this to just blank out the box because I think that looks better
		SetDlgItemText(IDC_INGROUP, "");
		m_nCurrentGroup = -1;
		return;
	}

	GetDlgItem(IDC_GROUPDL)->EnableWindow(TRUE);
	GetDlgItem(IDC_INGROUP)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(FALSE);
	m_GroupSelect->FromClause = "GroupsT";
	CurrReport->bUseGroup = TRUE;
	CurrReport->bUseFilter = FALSE;
	CurrReport->nFilterID = -1;

	m_GroupSelect->WhereClause = _bstr_t("");
	m_GroupSelect->GetColumn(2)->PutFieldName(_bstr_t("-1 AS Type"));

	//TES 3/26/2007 - PLID 20528 - We also need to hide the CreatedDate and ModifiedDate columns, as well as
	// the column headers, and reduce the dropdown width.
	m_GroupSelect->HeadersVisible = g_cvarFalse;
	m_GroupSelect->DropDownWidth = -1;
	m_GroupSelect->GetColumn(3)->ColumnStyle = m_GroupSelect->GetColumn(3)->ColumnStyle & (~csVisible);
	m_GroupSelect->GetColumn(4)->ColumnStyle = m_GroupSelect->GetColumn(4)->ColumnStyle & (~csVisible);

	//the edit box was showing even though nothing was selected in the dropdown
	SetDlgItemText(IDC_INGROUP, "");
	//DRT 1/28/03 - Just blanking out the # in group doesnt do it, it is still showing the name of the last one selected
	//			and giving false illusions.
	m_GroupSelect->PutCurSel(-1);

	m_nCurrentGroup = m_GroupSelect->CurSel == -1 ? -1 : VarLong(m_GroupSelect->GetValue(m_GroupSelect->CurSel, 0));
	m_GroupSelect->Requery();
}

void CReports::OnSelChangedGroup(long nNewSel) 
{


	
}

void CReports::OnNofilgroup() 
{
	m_No_Group_Filter.SetCheck(1);
	CurrReport->bUseGroup = FALSE;
	CurrReport->bUseFilter = FALSE;
	m_GroupSelect->CurSel = -1;
	GetDlgItem(IDC_GROUPDL)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_FILTER)->EnableWindow(FALSE);
	//I changed this to just blank out the box because I think that looks better
	SetDlgItemText(IDC_INGROUP, "");
	m_nCurrentGroup = -1;
	//GetDlgItem(IDC_INGROUP)->EnableWindow(FALSE);
}

BOOL CReports::PreTranslateMessage(MSG* pMsg) 
{
	m_ToolCtrl.RelayEvent(pMsg);

	return CNxDialog::PreTranslateMessage(pMsg);
}

void CReports::OnSelectTab(short newTab, short oldTab) 
{
	if (!DoInitLists(newTab))
	{
		m_tab->CurSel = oldTab; // CAH 12/19: This was here before

		// CAH 12/19 //
//		DoInitLists(oldTab);
		///////////////
	}
}

void CReports::OnUseExtended() 
{
	if (!m_bIsLoading) {
		if (m_UseExtended.GetCheck())  {
			//TES 12/18/2008 - PLID 32514 - They're not allowed to use this filter
			// if they have the Scheduler Standard license.
			// (j.gruber 2009-02-03 16:46) - PLID 32939 -except the statements
			if((!IsStatement(CurrReport->nID)) && !g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Filtering reports based on extended filters", "The_Reports_Module/run_reports.htm")) {
				m_UseExtended.SetCheck(0);
				ShowDlgItem(IDC_MULTI_EXT_LIST, SW_HIDE);
				ShowDlgItem(IDC_EXT_FILTER, SW_SHOW);
				EnableDlgItem(IDC_EXT_FILTER, FALSE);
				CurrReport->ClearExtraValues();
			}
			else {
				EnableDlgItem(IDC_EXT_FILTER, TRUE);
				if (IsStatement(CurrReport->nID)) {
					ShowDlgItem(IDC_MULTI_EXT_LIST, SW_HIDE);
					ShowDlgItem(IDC_EXT_FILTER, SW_SHOW);
					//try to set the filter to be what they used last
					CString strExtraField = GetRemotePropertyText("SttmntExtendedFilter", "", 0, "<None>", TRUE);
					if (! strExtraField.IsEmpty()) {
						m_ExtFilterList->SetSelByColumn(0, _variant_t(strExtraField));
						OnSelChosenExtFilter(m_ExtFilterList->CurSel);
					}
					else {
						//(e.lally 2011-03-03) PLID 42666 - Select the first one in the list
						m_ExtFilterList->CurSel = 0;
						OnSelChosenExtFilter(m_ExtFilterList->CurSel);
					}
				}
				else {
					LoadExtendedFilter(CurrReport);
				}
			}
		} else {
			ShowDlgItem(IDC_MULTI_EXT_LIST, SW_HIDE);
			ShowDlgItem(IDC_EXT_FILTER, SW_SHOW);
			EnableDlgItem(IDC_EXT_FILTER, FALSE);
			CurrReport->ClearExtraValues();
		}
	}	
}

void CReports::OnServiceDate() 
{
	CurrReport->nDateFilter = 1;
	if (m_tab->CurSel == 6)	//payments
		CurrReport->strDateCaption = "Payment Date";
	else
		CurrReport->strDateCaption = "Service Date";
	//don't change the text of the date label - dt 3/6/01
	//	SetDlgItemText (IDC_DATERANGE, "Service Date");	
}

void CReports::OnInputDate() 
{
	CurrReport->nDateFilter = 2;
	CurrReport->strDateCaption = "Input Date";
	//don't change the text of the date label - dt 3/6/01
	//	SetDlgItemText (IDC_DATERANGE, "Input Date");
}

void CReports::EnableDateRange(bool enable /*=true*/)
{
	CWnd *pFrom, *pTo;
	CRect rcFrom, rcTo;

	pFrom = GetDlgItem(IDC_LABEL_FROMDATE);
	pTo = GetDlgItem(IDC_LABEL_TO_DATE);

	if (enable)
	{
		m_from.EnableWindow(TRUE);
		m_to.EnableWindow(TRUE);
		pFrom->EnableWindow(TRUE);
		pTo->EnableWindow(TRUE);
		GetDlgItem(IDC_DATE_OPTIONS)->EnableWindow(TRUE);

		// (j.gruber 2008-07-11 13:08) - PLID 30692 - date quick filter
		GetDlgItem(IDC_REPORTS_QUICK_DATE_FILTER)->EnableWindow(TRUE);
		// (j.gruber 2008-07-22 09:42) - - PLID 28976 - All Years Filter
		GetDlgItem(IDC_REPORTS_ALL_YEAR)->EnableWindow(TRUE);

		

		/*if(CurrReport->nDateFilter)
		{
			GetDlgItem(IDC_LABEL_FILTER)->EnableWindow(TRUE);
			GetDlgItem(IDC_SERVICE_DATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_INPUT_DATE)->EnableWindow(TRUE);
		}*/

	}
	else
	{
		m_from.EnableWindow(FALSE);
		m_to.EnableWindow(FALSE);
		pFrom->EnableWindow(FALSE);
		pTo->EnableWindow(FALSE);

		// (j.gruber 2008-07-11 13:08) - PLID 30692 - date quick filter
		GetDlgItem(IDC_REPORTS_QUICK_DATE_FILTER)->EnableWindow(TRUE);
		// (j.gruber 2008-07-22 09:42) - - PLID 28976 - All Years Filter
		GetDlgItem(IDC_REPORTS_ALL_YEAR)->EnableWindow(FALSE);
		
		
		/*if(CurrReport && CurrReport->nDateFilter)
		{
			GetDlgItem(IDC_LABEL_FILTER)->EnableWindow(TRUE);
			GetDlgItem(IDC_SERVICE_DATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_INPUT_DATE)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_LABEL_FILTER)->EnableWindow(FALSE);
			GetDlgItem(IDC_SERVICE_DATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_INPUT_DATE)->EnableWindow(FALSE);
		}*/
	}

//	InvalidateDlgItem(IDC_LABEL_FROMDATE);
//	InvalidateDlgItem(IDC_LABEL_TO_DATE);
}

void CReports::OnAllDates() 
{
	if (CurrReport) 
		CurrReport->nDateRange = -1;
	// (j.gruber 2008-07-11 13:08) - PLID 30692 - date quick filter
	m_pDateOptionList->SetSelByColumn(0, (long)-1);
	EnableDateRange(false);
}

void CReports::OnDateRange() 
{
	if(CurrReport) {
		if(!CurrReport->bOneDate) {
			EnableDateRange(true);
			if (CurrReport) 
				CurrReport->nDateRange = 1;

			if (!m_bIsLoading) 
			{
				m_from.SetValue((COleVariant)COleDateTime::GetCurrentTime());
				m_to.SetValue((COleVariant)COleDateTime::GetCurrentTime());
				// (j.gruber 2008-07-11 13:08) - PLID 30692 - date quick filter
				m_pDateOptionList->SetSelByColumn(0, (long)2);
				if (CurrReport) 
					CurrReport->DateTo = CurrReport->DateFrom = (COleDateTime::GetCurrentTime());
			}
			else 
			{
				if (!CurrReport) 
					return;
				m_from.SetValue ((COleVariant)CurrReport->DateFrom);
				m_to.SetValue ((COleVariant)CurrReport->DateTo);
				// (j.gruber 2008-07-11 13:08) - PLID 30692 - date quick filter
				m_pDateOptionList->SetSelByColumn(0, (long)2);
				
			}
		}
	}
	
}

void CReports::OnAllpats() 
{
	EnableDlgItem(IDC_PATSELECT, FALSE);	
	CurrReport->nPatient = -1;
}

void CReports::OnSelectpat() 
{
	try {
		//TES 12/18/2008 - PLID 32514 - They're not allowed to use this filter
		// if they have the Scheduler Standard license.
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Filtering reports based on specific patients", "The_Reports_Module/run_reports.htm")) {
			m_rSelectPat.SetCheck(0);
			EnableDlgItem(IDC_PATSELECT, FALSE);
			m_rAllPatients.SetCheck(1);
			return;
		}

		// (a.walling 2011-08-04 14:36) - PLID 44788 - Lots of unnecessary code removed now

		// Handle the onselchange event provider dropdown
		CurrReport->nPatient = m_nSelectedPatientID;
		EnableDlgItem(IDC_PATSELECT, TRUE);
	} NxCatchAll("CReports::OnClickSelectpat Error 1");	
}

void CReports::OnDetailed() 
{
	if (CurrReport) 
		CurrReport->nDetail = 1;	
}

void CReports::OnSummary() 
{
	if (CurrReport) 
		CurrReport->nDetail = 2;
}

void CReports::OnAlllocations() 
{
	//disable the dropdown
	if (m_rAllLocations.GetCheck()) {
		GetDlgItem(IDC_LOCATIONSELECT)->EnableWindow(FALSE);
		//(e.lally 2008-09-05) PLID 6780 - hide the hyperlink for multiple locations
		m_nxlLocationLabel.SetType(dtsDisabledHyperlink);
		m_nxlLocationLabel.Invalidate();
	}
	if (CurrReport){
		CurrReport->nLocation = -1;	
	}
}

void CReports::OnSelectlocation() 
{
	if (m_bIsLoading) return;
	////////////////////////////////////////////////////////

	try {
		//TES 12/18/2008 - PLID 32514 - They're not allowed to use this filter
		// if they have the Scheduler Standard license.
		if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Filtering reports based on specific locations", "The_Reports_Module/run_reports.htm")) {
			m_rSelectLoc.SetCheck(0);
			EnableDlgItem(IDC_LOCATIONSELECT, FALSE);
			m_rAllLocations.SetCheck(1);
			return;
		}

		// What's currently selected
		long nCurSel = m_LocationSelect->CurSel;
		// Check to see if there's anything in the provider list
		if (m_LocationSelect->GetRowCount() > 0) {
			// If nothing's selected in the dropdown, select the current location
			if (nCurSel < 0) {
				m_LocationSelect->SetSelByColumn(0, _variant_t(GetCurrentLocationID()));
				//(e.lally 2008-09-05) PLID 6780 - Be sure to reset our cursel
				nCurSel = m_LocationSelect->CurSel;
			}
			// Handle the onselchange event Location dropdown
			EnableDlgItem(IDC_LOCATIONSELECT, TRUE);
			//(e.lally 2008-09-05) PLID 6780 - Check for filtering on None or Multiple locations
			if(GetDlgItem(IDC_LOCATIONSELECT)->IsWindowVisible()) {
				//Double check that it's not showing "{Multiple Locations}"
				if(nCurSel >= 0 && VarLong(m_LocationSelect->GetValue(nCurSel, 0)) == -3) {
					nCurSel = 2;
					m_LocationSelect->CurSel = nCurSel;
				}
				OnSelChosenLocationSelect(nCurSel);
			}
			else {
				//<Multiple> is selected, and the array from when we selected it is still valid.
				CurrReport->nLocation = -3;
				m_nxlLocationLabel.SetType(dtsHyperlink);
				m_nxlLocationLabel.Invalidate();
			}

		} else {
			//(e.lally 2008-090-05 This should not be possible, but I am leaving it in)
			// There's nothing in the location list so tell the user and unselect this option
			MsgBox(MB_OK|MB_ICONINFORMATION, "There are no locations to select from");
			m_rSelectLoc.SetCheck(FALSE);
			EnableDlgItem(IDC_LOCATIONSELECT, FALSE);
			m_rAllLocations.SetCheck(TRUE);
		}
	} NxCatchAll("CReports::OnClickSelectLocation Error 1");

}

void CReports::OnAllprovs() 
{
	if (CurrReport) {
		CurrReport->nProvider = -1;
		EnableDlgItem(IDC_PROVSELECT, FALSE);
		m_nxlProviderLabel.SetType(dtsDisabledHyperlink);
		m_nxlProviderLabel.Invalidate();
	}	
}

void CReports::OnSelectprov() 
{
	try {
		// What's currently selected
		long nCurSel = m_ProvSelect->CurSel;
		// Check to see if there's anything in the provider list
		if (m_ProvSelect->GetRowCount() > 0) {
			// If nothing's selected in the dropdown, select the first "real" item
			if (nCurSel < 0) {
				nCurSel = 2;
				m_ProvSelect->CurSel = nCurSel;
			}
			// Handle the onselchange event provider dropdown
			EnableDlgItem(IDC_PROVSELECT, TRUE);
			if(GetDlgItem(IDC_PROVSELECT)->IsWindowVisible()) {
				//Double check that it's not showing "{Multiple Providers}"
				if(VarLong(m_ProvSelect->GetValue(nCurSel, 0)) == -3) {
					nCurSel = 2;
					m_ProvSelect->CurSel = nCurSel;
				}
				OnSelChosenProvselect(nCurSel);
			}
			else {
				//<Multiple> is selected, and the array from when we selected it is still valid.
				CurrReport->nProvider = -3;
				m_nxlProviderLabel.SetType(dtsHyperlink);
				m_nxlProviderLabel.Invalidate();
			}
		} else {
			// There's nothing in the provider list so tell the user and unselect this option
			MsgBox(MB_OK|MB_ICONINFORMATION, RCS(IDS_REPORTS_NO_PROVIDERS));
			m_rSelectProv.SetCheck(0);
			EnableDlgItem(IDC_PROVSELECT, FALSE);
			m_rAllProviders.SetCheck(1);
		}
	} NxCatchAll("CReports::OnClickSelectprov Error 1");	
}

void CReports::OnSelChosenExtFilter(long nRow) 
{
	try {
		if (!m_bIsLoading) {
			if(nRow != -1) {
				if(VarString(m_ExtFilterList->Value[nRow][0]) == "-3") {
					//They selected "{Multiple}"
					OnExtList();
				}
				else {
					CurrReport->SetExtraValue(VarString(m_ExtFilterList->Value[nRow][0]));
				}
			}
			else {
				//no row selected - force a selection

				switch (CurrReport->nID) {
					// (j.jones 2016-05-06 12:03) - NX-100501 - Monthly reports show all months from 1990 to 12 months from now,
					// need to default to the current month (unless one is selected)
					case 154:	//Financial Activity - Monthly
					{
						//what is the current month?
						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						//the filter option is YYYYMM numerically, like 199912 if you're partying like Prince (RIP),
						//or 201504 if April Fool's Day, but sadly in the filter itself, it's a string
						CString strMonthYear;
						strMonthYear.Format("%li%02li", (long)(dtNow.GetYear()), (long)(dtNow.GetMonth()));
						//this should never fail
						long nSel = m_ExtFilterList->SetSelByColumn(0, _bstr_t(strMonthYear));
						if (nSel == -1) {
							//this should be impossible!
							ASSERT(false);
							break;
						}
						else {
							OnSelChosenExtFilter(m_ExtFilterList->CurSel);
							return;
						}
					}
					break;
					// (j.jones 2016-05-06 12:03) - NX-100501 - Yearly reports show all years from 1990 - 2030,
					// need to default to the current year (unless one is selected)
					case 155:	//Financial Activity - Yearly
					case 262:	//ASPS Survey
					case 271:	//Inventory Sales Graph
					{
						//what is the current year?
						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						CString strYear;
						strYear.Format("%li", (long)(dtNow.GetYear()));
						//this should never fail
						long nSel = m_ExtFilterList->SetSelByColumn(0, _bstr_t(strYear));
						if (nSel == -1) {
							//this should be impossible!
							ASSERT(false);
							break;
						}
						else {
							OnSelChosenExtFilter(m_ExtFilterList->CurSel);
							return;
						}
					}
					break;
					default:
						break;
				}

				//Set it to the first non-{Multiple} row.
				if(m_ExtFilterList->GetRowCount() == 0) {
					MsgBox("There are no items in the filter list!");
					m_UseExtended.SetCheck(0);
					OnUseExtended();
					// (j.gruber 2011-06-17 13:37) - PLID 37482 - don't continue on
					return;
				}
				if(VarString(m_ExtFilterList->GetValue(0,0)) == "-3") {
					if(m_ExtFilterList->GetRowCount() == 1) {
						MsgBox("There are no items in the filter list!");
						m_UseExtended.SetCheck(0);
						OnUseExtended();
					}
					else {
						m_ExtFilterList->CurSel = 1;
						OnSelChosenExtFilter(1);
					}
				}
				else {
					m_ExtFilterList->CurSel = 0;
					OnSelChosenExtFilter(0);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

HBRUSH CReports::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	//to avoid drawing problems with transparent text and disabled ites
	//override the NxDialog way of doing text with a non-grey background
	//NxDialog relies on the NxColor to draw the background, then draws text transparently
	//instead, we actually color the background of the STATIC text
	if (nCtlColor == CTLCOLOR_STATIC && pWnd->GetDlgCtrlID() != IDC_CURRNAME && pWnd->GetDlgCtrlID() != IDC_MULTI_PROV_LIST
		&& pWnd->GetDlgCtrlID() != IDC_MULTI_EXT_LIST) {
		extern CPracticeApp theApp;
		pDC->SelectPalette(&theApp.m_palette, FALSE);
		pDC->RealizePalette();
		pDC->SetBkColor(PaletteColor(0x00DEB05C));
		return m_brush;
	} else {
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}



long CReports::CheckAllowClose()
{
	try{
		CString strCurList;
		BuildIDList(m_SelectList, strCurList);
		
		if(GetRemotePropertyInt("ReportFilters_RememberProviderFilter", 0, 0, GetCurrentUserName(), true) == 1) {
			strCurList = AddBatchProviders(strCurList);
		}

		SetRemotePropertyMemo(CString("Reports.Batches.CurrentBatch"), strCurList, 0, GetCurrentUserName());
	}
	NxCatchAll("CReports::CheckAllowClose Error 1.  Could not save report batch list.");
	return AC_CAN_CLOSE;
}

// (j.luckoski 2012-10-03 17:21) - PLID 24684 - Added function to add the ; delimited list of providers after the report ID
CString CReports::AddBatchProviders(CString strCurList) 
{
	// j.luckoski - Input the list as a comma delimited list etc reports 1, 2, 3 will be strCurList = '1,2,3'
	// In the end it will append all providers after the report ID delimited by a ';'
	// and output such as '1;123,2;12;13,3;4;5' where commas seperate reports, and semi-colons 
	// seperate provider filters so in above report 1 will have provider 123 selected.
	CString strBatchList = "";
	long nId;
	CString strTemp = "";

	CDWordArray aryStringCurList;
	ParseDelimitedStringToDWordArray(strCurList, ",", aryStringCurList);

	for (int i = 0; i < aryStringCurList.GetSize(); i++) {

		nId = long(aryStringCurList.GetAt(i));
		CReportInfo *pRep = FindReport(nId);

		if(pRep != NULL) {

			strTemp.Format("%i", nId);
			strBatchList += strTemp;

			if(pRep->m_dwProviders.GetSize() > 0 && pRep->nProvider != -1) {
				strBatchList += ";" + GenerateDelimitedListFromLongArray(pRep->m_dwProviders, ";");
			}else if(pRep->nProvider == -2) {
				strBatchList += ";-2";
			}

		}
		strBatchList += ",";
	}

	// j.luckoski - returns a list of '1;2;3,5;6;7' where the reports are 1 and 5 and 2 and 3 are providers of 1 and 6,7 of report 5. 
	return strBatchList;
}


// (a.walling 2010-11-26 13:08) - PLID 40444 - Allow interaction from the view
short CReports::GetActiveTab()
{
	if (m_tab) {
		return (short)m_tab->CurSel;
	} else {
		return -1;
	}
}

// (c.haag 2009-01-12 16:45) - PLID 32683 - Sets the active tab in the reports sheet.
// Returns TRUE on success, and FALSE on failure.
BOOL CReports::SetActiveTab(ReportsModule::Tab newtab)
{
	const ReportsModule::Tab oldtab = (ReportsModule::Tab)m_tab->CurSel;
	if (oldtab == newtab) {
		// Tab is already set; nothing to do
		return TRUE;
	}
	else if (!m_tab->ShowTab[newtab]) {
		// Tab is not visible
		return FALSE;
	}
	else if (!CheckReportTabAvailability(newtab)) {
		// Tab is not available
		return FALSE;
	}
	else {
		m_tab->CurSel = (short)newtab;
		OnSelectTab(newtab, oldtab);
		return TRUE;
	}
}

void CReports::OnSelChosenGroupdl(long nRow) 
{
	if(nRow != -1) {
		if (m_GroupSelect->FromClause == (bstr_t)"FiltersT" && nRow == 0 && VarString(m_GroupSelect->GetValue(nRow, 1)) == "{New Filter}") {
			//they chose to make a new filter

			if (!CheckCurrentUserPermissions(bioLWFilter, sptWrite))
				return;
		
			CFilterEditDlg dlg(NULL, fboPerson, CGroups::IsActionSupported, CGroups::CommitSubfilterAction);
			long nResult;
			nResult = dlg.DoModal();

			if (nResult == 1) {
				//they clicked OK
				long nID = dlg.GetFilterId();
				/*
				IRowSettingsPtr pRow = m_GroupSelect->GetRow(-1);				
				pRow->PutValue(0, nID);
				pRow->PutValue(1, _variant_t(dlg.m_strFilterName));
				pRow->PutValue(2, (long)dlg.m_nFilterType);

				m_GroupSelect->AddRow(pRow);
				*/

				// (j.jones 2004-07-12 09:35) - changed to a requery because you might have
				//created nested filters, and they need to show up
				m_GroupSelect->Requery();
				m_GroupSelect->SetSelByColumn(0, nID);
				//OK, now we'll set the new selection to the actual new selection.
				nRow = m_GroupSelect->CurSel;

				//DRT 6/4/02 - Added network call to refresh the letter writing module
				m_groupChecker.Refresh();
			}

			CurrReport->nGroup = -1;
			CurrReport->nFilterID = VarLong(m_GroupSelect->GetValue(nRow, 0));
		} else if (m_GroupSelect->FromClause == (bstr_t)"FiltersT") { 
			CurrReport->nGroup = -1;
			CurrReport->nFilterID = VarLong(m_GroupSelect->GetValue(nRow, 0));
		} else if (m_GroupSelect->FromClause == (bstr_t)"GroupsT") {
			CurrReport->nGroup = VarLong(m_GroupSelect->GetValue(nRow, 0));
			CurrReport->nFilterID = -1;
		}

		//this code was previously in OnChangeGroup(), but apparently this code is no longer being called, so I moved it here
		//this calculates the number of patients in the group, and puts it in the edit box
		long nItemCount = 0;
		if (GenerateGroupFilter(CurrReport, NULL, &nItemCount)) {
			CString strCount;
			strCount.Format("%li In Group", nItemCount);
			SetDlgItemText(IDC_INGROUP, strCount);
		}
		//Now, call the EXACT SAME FUNCTION, with entirely different parameters, to do an ENTIRELY DIFFERENT THING!  I did not write
		//this function.  I do not know why somebody decided to put two entirely different functions under the same name.  I can only
		//work with what I've got.
		CString strGroupFilter;
		if (GenerateGroupFilter(CurrReport, &strGroupFilter)) {
			if (!strGroupFilter.IsEmpty()) {
				CurrReport->strGroupFilter = "(" + strGroupFilter + ")";
			} else {
				CurrReport->strGroupFilter = _T("");
			}
		} else {
			CurrReport->strGroupFilter = _T("");
		}
	}
}

void CReports::OnRequeryFinishedGroupdl(short nFlags) 
{
	try {
		//DRT 6/1/2004 - PLID 12694 - m.carlson got an error here with a low level exception, 
		//	but I can't figure out what could possible cause it except for an absurdly wierd
		//	condition (pointer to the license or datalist was NULL, which shouldn't be possible).
		//	I put in a few checks for wierd things like that, but noone was ever able to reproduce
		//	the problem.
		if(m_GroupSelect == NULL || g_pLicense == NULL) {
			ASSERT(FALSE);
			return;
		}

		//DRT 5/25/2004 - PLID 12563 - This code should always be adding the {New Filter} line
		//	if you have the license.  We can then check the nFilterID below to see if we need to 
		//	reset that selection.
		if(m_GroupSelect->FromClause == _bstr_t("FiltersT")) {
			//Check the selction first.  There is a conflict where the {New Filter} 
			//	uses -1, but the nFilterID is set to -1 for no selection, so we must
			//	check this before we add the {New Filter} back in.
			if(CurrReport) {
				if(m_GroupSelect->SetSelByColumn(0, CurrReport->nFilterID) == -1) {
					//The filter has disappeared!
					CurrReport->nFilterID = -1;
				}
			}

			//Add the new filter row - only if they have the license for Letter writing
			if(g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrSilent)) {
				IRowSettingsPtr pRow = m_GroupSelect->GetRow(-1);
				pRow->PutValue(0, long(-1));
				pRow->PutValue(1, _variant_t("{New Filter}"));
				pRow->PutValue(2, (long)-1);
				m_GroupSelect->InsertRow(pRow, 0);
			}
		}
		else if(m_nCurrentGroup != -1) 
			m_GroupSelect->SetSelByColumn(0, m_nCurrentGroup);
	}NxCatchAll("Error in CReports::OnRequeryFinishedGroupdl()");
}

void CReports::OnEditreport() 
{


	if (CheckCurrentUserPermissions(bioReportDesigner, sptRead)) { 
			
			CEditReportPickerDlg dlg(this, CurrReport);
			long nResult = dlg.DoModal();
				
			//Reload the filters for this report, in case they changed the default or somthing
			LoadFilters(CurrReport);
	}
	
}

void CReports::OnDateRangeOptions() 
{
	OnDateRange();
}


void CReports::OnSelChosenDateOptions(long nRow) 
{
	if(nRow != -1) {
		CurrReport->nDateFilter = (short)VarLong(m_pDateOptions->GetValue(nRow, 0));
		CurrReport->strDateFilterField = VarString(m_pDateOptions->GetValue(nRow, 2));
	}
	else {
		//They HAVE to select something, if at all possible.
		if(m_pDateOptions->GetRowCount() > 0) {
			m_pDateOptions->CurSel = 0;
			OnSelChosenDateOptions(0);
		}
	}
}

void CReports::OnEditFilter() 
{
	if(m_GroupSelect->FromClause == (bstr_t)"FiltersT" && m_GroupSelect->CurSel != -1) {
		if (!CheckCurrentUserPermissions(bioLWFilter, sptWrite))
					return;
		
		if (!g_pLicense->CheckForLicense(CLicense::lcLetter, CLicense::cflrUse)) {
			return;
		}
		
		//TES 6/15/2004: This isn't necessarily person based!
		CFilterEditDlg dlg(NULL, VarLong(m_GroupSelect->GetValue(m_GroupSelect->CurSel, 2)), CGroups::IsActionSupported, CGroups::CommitSubfilterAction);
		long nResult;
		nResult = dlg.EditFilter(VarLong(m_GroupSelect->GetValue(m_GroupSelect->CurSel, 0)));

		if (nResult == 1) {
			//they clicked OK
			m_GroupSelect->Requery();

			//DRT 6/4/02 - Added network call to refresh the letter writing module
			m_groupChecker.Refresh();
		}
	}
}

//
///////////////////////////////////////////////////////////////////////
//
//DRT 7/29/03 - Tooltip functionality!  A lot of this code was borrowed
//	and modified from NxTrack.  For right now, this is all on hold, and none 
//	of this code does anything.  PLID 6426 details what is going to happen, 
//	so I'm leaving this code which may be an asset to it.  If I am not the 
//	one to complete said PL item, feel free to mangle, delete, and modify 
//	the code between here and the bottom comment as you see fit.

HWND WINAPI CreateTT(HINSTANCE hInst, HWND hwndOwner)
{
    INITCOMMONCONTROLSEX icex;
    HWND        hwndTT;
    TOOLINFO    ti;

    // Load the ToolTip class from the DLL.
    icex.dwSize = sizeof(icex);
    icex.dwICC  = ICC_BAR_CLASSES;

    if(!InitCommonControlsEx(&icex))
       return NULL;

    // Create the ToolTip control.
    hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, TEXT(""),
                          WS_POPUP,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          CW_USEDEFAULT, CW_USEDEFAULT,
                          NULL, (HMENU)NULL, hInst,
                          NULL);

    // Prepare TOOLINFO structure for use as tracking ToolTip.
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_IDISHWND | TTF_TRACK;
    ti.hwnd   = hwndOwner;
    ti.uId    = (UINT)1;//g_hwndMain;
    ti.hinst  = NULL;
    ti.lpszText  = LPSTR_TEXTCALLBACK;
    ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0; 

    // Add the tool to the control, displaying an error if needed.
    if(!SendMessage(hwndTT,TTM_ADDTOOL,0,(LPARAM)&ti)){
        MessageBox(hwndOwner,"Could not create ToolTip control.",
                   "Error",MB_OK);
        return NULL;
    }

    // Activate (display) the tracking ToolTip. Then, set a global
    // flag value to indicate that the ToolTip is active, so other
    // functions can check to see if it's visible.
    SendMessage(hwndTT,TTM_TRACKACTIVATE,(WPARAM)TRUE,(LPARAM)&ti);

    return hwndTT;
}

void EnsurePtInScreenBounds(IN OUT CPoint &pt)
{
	const int cnScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int cnScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	// We use 1,1 here as a minimum because for some reason if you put 0 as x or y the tooltip 
	// doesn't do any wordwrapping, I don't understand but it's not really a big deal.
	if (!(CRect(1, 1, cnScreenWidth, cnScreenHeight).PtInRect(pt))) {
		pt.x = cnScreenWidth;
		pt.y = cnScreenHeight;
	}
}

/*	DRT - Removed - This was the code from the ? button.  A lot of this should
/		be useful when the description is a hyperlink.
void CReports::OnReportHelpDesc() 
{
	///////////////////
	HWND hWndTT = CreateTT(AfxGetInstanceHandle(), GetSafeHwnd());

	::SendMessage(hWndTT, TTM_SETMAXTIPWIDTH, 0, 200);

	CRect rect;
	GetDlgItem(IDC_CURRNAME)->GetWindowRect(&rect);
	CPoint pt = rect.TopLeft();

	// I discovered that if you pass a pt that's WAY out of bounds into this message, it can LOCK YOUR SYSTEM so we call EnsurePtInScreenBounds to make that impossible
	EnsurePtInScreenBounds(pt);
	::SendMessage(hWndTT, TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y));

	//keep track of the window
	if (m_hTipWnd) {
		::DestroyWindow(m_hTipWnd);
	}
	m_hTipWnd = hWndTT;
	////////////////////

	//Now set a timer - It will stay on screen for 20 seconds
	//Note that this tooltip doesn't behave entirely like others... 
	//	Most go away when you click or move your mouse, but since this
	//	pops up from a button, there's a good chance they'll be moving their
	//	mouse immediately afterwards.  So this stays up for 20 seconds, 
	//	and is destroyed after that time, or if you click anywhere in the module
	SetTimer(IDT_HELP_DESC, 20000, NULL);

}
*/
BOOL CReports::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	LPNMHDR phdr = (LPNMHDR)lParam;
	if (phdr->code == TTN_GETDISPINFO) {

		//TODO:  get the current report and it's description
		char szTxt[1024];
		strcpy(szTxt, GetCurrentReportDesc());

		LPNMTTDISPINFO lpnmtdi = (LPNMTTDISPINFO)lParam;
		lpnmtdi->lpszText = (char *)(LPCTSTR)szTxt;
		lpnmtdi->hinst = NULL;
		//lpnmtdi->uFlags = TTF_IDISHWND;
	}

	return CNxDialog::OnNotify(wParam, lParam, pResult);
}

//destroys the tooltip window if it is active.  does nothing if 
void CReports::DestroyTT()
{
	if(m_hTipWnd) {
		::DestroyWindow(m_hTipWnd);
		m_hTipWnd = NULL;
	}
}

CString CReports::GetCurrentReportDesc()
{
	return m_strCurrentToolText;
}

void CReports::OnLButtonDown(UINT nFlags, CPoint point) 
{
	//they have clicked somewhere in the reports module, so close
	//the tooltip if it's up
	DestroyTT();
	
	CNxDialog::OnLButtonDown(nFlags, point);
}

//DRT 7/29/03 - End of tooltip code.  Stop mangling.
///////////////////////////////////////////////////////////////////
//

LRESULT CReports::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch(message) {
	case NXM_ADD_REPORT:
		{
			//we want to add the report with the given wParam ID
			long nID = (long)wParam;

			if(nID <= 0) 
				break;

			AddReport(nID, false);
			m_bModified = TRUE;
			return 0;
		}
		break;
	default:
		break;
	}

	return CNxDialog::WindowProc(message, wParam, lParam);
}

void CReports::OnProvList() 
{
	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "ProvidersT");
		dlg.PreSelect(CurrReport->m_dwProviders);

		// (j.jones 2010-06-15 13:59) - PLID 39117 - use the from/where clauses from the dropdown
		CString strFromClause = VarString(_variant_t(m_ProvSelect->GetFromClause()));
		CString strWhereClause = VarString(_variant_t(m_ProvSelect->GetWhereClause()));

		if(IDOK == dlg.Open(strFromClause, strWhereClause, "PersonT.ID", "Last + ', ' + First + ' ' + Middle", "Select one or more providers to filter on:", 1)) {
			dlg.FillArrayWithIDs(CurrReport->m_dwProviders);
		
			if(CurrReport->m_dwProviders.GetSize() > 1) {
				ShowDlgItem(IDC_PROVSELECT, SW_HIDE);
				m_nxlProviderLabel.SetText(dlg.GetMultiSelectString());
				m_nxlProviderLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_MULTI_PROV_LIST, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_PROV_LIST);
				CurrReport->nProvider = -3;
			}
			else if(CurrReport->m_dwProviders.GetSize() == 1) {
				//They selected exactly one.
				ShowDlgItem(IDC_MULTI_PROV_LIST, SW_HIDE);
				ShowDlgItem(IDC_PROVSELECT, SW_SHOW);
				m_ProvSelect->SetSelByColumn(0, (long)CurrReport->m_dwProviders.GetAt(0));
				CurrReport->nProvider = (long)CurrReport->m_dwProviders.GetAt(0);
			}
			else {
				//They didn't select any.  But we told multiselect dlg they had to pick at least one!
				ASSERT(FALSE);
			}
		}
		else {
			//Check if they have "multiple" selected
			if(CurrReport->m_dwProviders.GetSize() > 1) {
				ShowDlgItem(IDC_PROVSELECT, SW_HIDE);
				CString strIDList, strID, strProvList;
				for(int i=0; i < CurrReport->m_dwProviders.GetSize(); i++) {
					strID.Format("%li, ", (long)CurrReport->m_dwProviders.GetAt(i));
					strIDList += strID;
				}
				strIDList = strIDList.Left(strIDList.GetLength()-2);
				_RecordsetPtr rsProvs = CreateRecordset("SELECT ID, Last + ', ' + First + ' ' + Middle AS Name FROM PersonT INNER JOIN ProvidersT ON PersonT.Id = ProvidersT.PersonID WHERE PersonT.ID IN (%s) ORDER BY Last, First, Middle ASC", strIDList);
				while(!rsProvs->eof) {
					strProvList += AdoFldString(rsProvs, "Name") + ", ";
					rsProvs->MoveNext();
				}
				rsProvs->Close();
				strProvList = strProvList.Left(strProvList.GetLength()-2);
				m_nxlProviderLabel.SetText(strProvList);
				m_nxlProviderLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_MULTI_PROV_LIST, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_PROV_LIST);
			}
			else {
				//They selected exactly one. (even if that one was "<No Provider>"
				ShowDlgItem(IDC_MULTI_PROV_LIST, SW_HIDE);
				ShowDlgItem(IDC_PROVSELECT, SW_SHOW);
				m_ProvSelect->SetSelByColumn(0, CurrReport->nProvider);
			}
		}
	}NxCatchAll("Error in CReports::OnProvList()");

}

//(e.lally 2008-09-05) PLID 6780 - Created for the use of filtering on None or Multiple locations
void CReports::OnLocationList() 
{
	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "LocationsT");
		dlg.PreSelect(CurrReport->m_dwLocations);
		//Rather than hard-code this, let's grab the filter criteria from the regular dropdown.
		CString strLocationFromClause = VarString(_variant_t(m_LocationSelect->GetFromClause()));
		CString strLocationWhereClause = VarString(_variant_t(m_LocationSelect->GetWhereClause()));
		if(IDOK == dlg.Open(strLocationFromClause, strLocationWhereClause, "LocationsT.ID", "Name", "Select one or more locations to filter on:", 1)) {
			dlg.FillArrayWithIDs(CurrReport->m_dwLocations);
		
			if(CurrReport->m_dwLocations.GetSize() > 1) {
				ShowDlgItem(IDC_LOCATIONSELECT, SW_HIDE);
				m_nxlLocationLabel.SetText(dlg.GetMultiSelectString());
				m_nxlLocationLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_MULTI_LOC_LIST, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_LOC_LIST);
				CurrReport->nLocation = -3;
			}
			else if(CurrReport->m_dwLocations.GetSize() == 1) {
				//They selected exactly one.
				ShowDlgItem(IDC_MULTI_LOC_LIST, SW_HIDE);
				ShowDlgItem(IDC_LOCATIONSELECT, SW_SHOW);
				m_LocationSelect->SetSelByColumn(0, (long)CurrReport->m_dwLocations.GetAt(0));
				CurrReport->nLocation = (long)CurrReport->m_dwLocations.GetAt(0);
			}
			else {
				//They didn't select any.  But we told multiselect dlg they had to pick at least one!
				ASSERT(FALSE);
			}
		}
		else {
			//Check if they have "multiple" selected
			if(CurrReport->m_dwLocations.GetSize() > 1) {
				ShowDlgItem(IDC_LOCATIONSELECT, SW_HIDE);
				CString strIDList, strID, strLocationList;
				for(int i=0; i < CurrReport->m_dwLocations.GetSize(); i++) {
					strID.Format("%li, ", (long)CurrReport->m_dwLocations.GetAt(i));
					strIDList += strID;
				}
				strIDList = strIDList.Left(strIDList.GetLength()-2);
				_RecordsetPtr rsLocations = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE LocationsT.ID IN (%s) ORDER BY Name ASC", strIDList);
				while(!rsLocations->eof) {
					strLocationList += AdoFldString(rsLocations, "Name") + ", ";
					rsLocations->MoveNext();
				}
				rsLocations->Close();
				strLocationList = strLocationList.Left(strLocationList.GetLength()-2);
				m_nxlLocationLabel.SetText(strLocationList);
				m_nxlLocationLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_MULTI_LOC_LIST, SW_SHOW);
				InvalidateDlgItem(IDC_MULTI_LOC_LIST);
			}
			else {
				//They selected exactly one. (even if that one was "<No Location>"
				ShowDlgItem(IDC_MULTI_LOC_LIST, SW_HIDE);
				ShowDlgItem(IDC_LOCATIONSELECT, SW_SHOW);
				m_LocationSelect->SetSelByColumn(0, CurrReport->nLocation);
			}
		}
	}NxCatchAll("Error in CReports::OnLocationList()");

}

void CReports::OnExtList() 
{
	try {
		// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
		CMultiSelectDlg dlg(this, "CReports::OnExtList");
		CStringArray saExtraValues;
		CVariantArray vaExtraValues;
		CurrReport->GetExtraValues(saExtraValues);
		for(int i = 0; i < saExtraValues.GetSize(); i++) vaExtraValues.Add(_variant_t(saExtraValues[i]));
		dlg.PreSelect(vaExtraValues);

		//Since we don't have a good way of generating this filter with an IN or a temp table, limit the number they can select.
		_variant_t varIDToSkip = _bstr_t("-3");//Don't let them select the {Multiple} row.
		CVariantArray va;
		va.Add(varIDToSkip);
		BOOL bChanged = FALSE;
		if(IDOK == dlg.Open(m_ExtFilterList, va, "Select the items you wish to filter on", 1, 75)) {
			bChanged = TRUE;
			dlg.FillArrayWithIDs(vaExtraValues);
		}
		//Now, update the screen.
		CurrReport->ClearExtraValues();
		for(i = 0; i < vaExtraValues.GetSize(); i++) {
			CurrReport->AddExtraValue(AsString(vaExtraValues[i]));
		}
	
		if(vaExtraValues.GetSize() > 1) {
			if(bChanged) {
				ShowDlgItem(IDC_EXT_FILTER, SW_HIDE);
				m_nxlExtLabel.SetText(dlg.GetMultiSelectString());
				m_nxlExtLabel.SetType(dtsHyperlink);
				ShowDlgItem(IDC_MULTI_EXT_LIST, SW_SHOW);
			}
			InvalidateDlgItem(IDC_MULTI_EXT_LIST);
		}
		else if(vaExtraValues.GetSize() == 1) {
			//They selected exactly one.
			ShowDlgItem(IDC_MULTI_EXT_LIST, SW_HIDE);
			ShowDlgItem(IDC_EXT_FILTER, SW_SHOW);
			EnableDlgItem(IDC_EXT_FILTER, TRUE);
			m_ExtFilterList->SetSelByColumn(0, vaExtraValues[0]);
		}
		else {
			//They didn't select any.
			ShowDlgItem(IDC_MULTI_EXT_LIST, SW_HIDE);
			ShowDlgItem(IDC_EXT_FILTER, SW_SHOW);
			m_ExtFilterList->CurSel = -1;
			OnSelChosenExtFilter(-1);
		}
	}NxCatchAll("Error in CReports::OnExtList()");

}

BOOL CReports::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	CRect rc;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	//(e.lally 2008-09-05) PLID 31275 - Fixed logic to work when more than one multi-select hyperlink is active
	if (CurrReport){
		if(CurrReport->nProvider == -3){
			GetDlgItem(IDC_MULTI_PROV_LIST)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
		//(e.lally 2008-09-05) PLID 6780 - Add link cursor for location hyperlink, when it's active
		if(CurrReport->nLocation == -3){
			GetDlgItem(IDC_MULTI_LOC_LIST)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if (rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}

		CStringArray saExtraValues;
		CurrReport->GetExtraValues(saExtraValues);
		if(saExtraValues.GetSize() > 1) {
			GetDlgItem(IDC_MULTI_EXT_LIST)->GetWindowRect(rc);
			ScreenToClient(&rc);

			if(rc.PtInRect(pt)) {
				SetCursor(GetLinkCursor());
				return TRUE;
			}
		}
	}

	return CNxDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CReports::OnSelChosenProvselect(long nRow) 
{
	try {
		// We must have a current report or else what the heck are we doing?
		ASSERT(CurrReport);
		if (CurrReport) {
			if (nRow >= 0) {
				if(VarLong(m_ProvSelect->Value[nRow][0]) == -3) {
					//They selected "<Multiple Providers>"
					OnProvList();
				}
				else if(VarLong(m_ProvSelect->Value[nRow][0]) == -2) {
					//They selected "<No Providers>"
					CurrReport->m_dwProviders.RemoveAll();
					CurrReport->nProvider = -2;
				}
				else {
					//They selected a provider
					CurrReport->m_dwProviders.RemoveAll();
					CurrReport->nProvider = VarLong(m_ProvSelect->Value[nRow][0]);
					CurrReport->m_dwProviders.Add((DWORD)CurrReport->nProvider);
				}
			} else {
				//No selection is made -- this shouldn't be possible
				ASSERT(FALSE);
			}
		} else {
			// CurrReport was NULL
			AfxThrowNxException("Current Report cannot be NULL");
		}
	} NxCatchAll("CReports::OnSelChosenProvselect Error 1");
}

LRESULT CReports::OnLabelClick(WPARAM wParam, LPARAM lParam)
{
	UINT nIdc = (UINT)wParam;
	//(e.lally 2008-09-05) PLID 6780 - Add option for locations
	switch(nIdc) {
	case IDC_MULTI_PROV_LIST:
		OnProvList();
		break;
	case IDC_MULTI_LOC_LIST:
		OnLocationList();
		break;
	case IDC_MULTI_EXT_LIST:
		OnExtList();
		break;
	default:
		//What?  Some strange NxLabel is posting messages to us?
		ASSERT(FALSE);
		break;
	}
	return 0;
}

void CReports::OnSelChosenBatchList(long nRow)
{
	// (c.haag 2003-08-06 15:24) - Make the function safe if the user
	// selected a non-row so Jennie won't get my dollar.
	if (nRow == -1)
	{
		m_BatchList->CurSel = 0;
		nRow = 0;
	}

	CString strBatchName = (LPCTSTR)_bstr_t(m_BatchList->GetValue(nRow, 0));
	
	//DRT 7/8/2005 - PLID 6308 - Check permissions for batch reports
	CPermissions perms = GetCurrentUserPermissions(bioReportBatch);
	if(perms & (sptDelete|sptDeleteWithPass)) {
		// MSC 2004-03-10
		// if they have the default batch selected, disable the delete button because they can't delete it anyway
		if(strBatchName == USER_BATCH_NAME)
		{
			GetDlgItem(IDC_DELETEBATCH)->EnableWindow(FALSE);
		}
		else{
			GetDlgItem(IDC_DELETEBATCH)->EnableWindow(TRUE);
		}
	}

	// (c.haag 2003-08-06 15:27) - Save changes the user made to the selected batch
	if (m_bModified)
	{
		// (c.haag 2007-02-28 14:41) - If we reselected the same batch, ask the user if they want to revert
		// to the existing version of the batch. For consistency with the existing message box, if the current
		// batch name is < Default Batch >, we will call it that.
		// 
		if (strBatchName == m_strCurrentBatchName) {
			if (IDNO == MsgBox(MB_YESNO | MB_ICONQUESTION, "Would you like to revert to the saved %s batch?", m_strCurrentBatchName)) {
				return;
			}
			m_bModified = FALSE;
		} else {
			if (IDYES == MsgBox(MB_YESNO, "Would you like to save your changes to the %s batch?", m_strCurrentBatchName))
			{
				SaveBatchChanges();
			}
			m_bModified = FALSE;
		}
	}

	if (strBatchName == USER_BATCH_NAME)
		LoadBatch("CurrentBatch", GetCurrentUserName());
	else
		LoadBatch(strBatchName, NULL);
	m_strCurrentBatchName = strBatchName;
}

void CReports::SaveBatchChanges()
{
	//TES 12/18/2008 - PLID 32514 - They can't modify Report Batches if they're on the
	// Scheduler Standard version.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Saved batches of commonly run reports", "The_Reports_Module/create_run_report_batches.htm")) {
		return;
	}

	//DRT 7/8/2005 - PLID 6308 - Check permissions.
	if(!CheckCurrentUserPermissions(bioReportBatch, sptWrite))
		return;

	CString strCurList;
	BuildIDList(m_SelectList, strCurList);
	if (m_strCurrentBatchName == USER_BATCH_NAME) {
		// (j.luckoski 2012-11-09 15:29) - PLID 24684 - Store current batch providers for this user only
		if(GetRemotePropertyInt("ReportFilters_RememberProviderFilter", 0, 0, GetCurrentUserName(), true) == 1) {
			strCurList = AddBatchProviders(strCurList);
		}
		SetRemotePropertyMemo(CString("Reports.Batches.CurrentBatch"), strCurList, 0, GetCurrentUserName());
	} else {
		// (j.luckoski 2012-11-09 15:30) - PLID 24684 - Store providers for all reports in batch for all users.
		if(GetRemotePropertyInt("ReportFilters_SaveProviderFilter", 0, 0, "<None>", true) == 1) {
			strCurList = AddBatchProviders(strCurList);
		}
		SetRemotePropertyMemo(CString("Reports.Batches.") + m_strCurrentBatchName, strCurList, 0, NULL);
	}
	m_bModified = FALSE;
}

void CReports::OnNewBatch()
{
	//TES 12/18/2008 - PLID 32514 - They can't modify Report Batches if they're on the
	// Scheduler Standard version.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Saved batches of commonly run reports", "The_Reports_Module/create_run_report_batches.htm")) {
		return;
	}

	//DRT 7/8/2005 - PLID 6308 - Check permissions.
	if(!CheckCurrentUserPermissions(bioReportBatch, sptCreate))
		return;

	CString strNewBatch; // Here we do Gremlins 2 - The New Batch
	BOOL bValid = FALSE;

	do {
		if (m_bModified)
		{
			if (IDYES == MsgBox(MB_YESNO, "Would you like to save your changes to the %s batch?", m_strCurrentBatchName))
			{
				SaveBatchChanges();
			}
			m_bModified = FALSE;
		}

		if (IDCANCEL == InputBoxLimited(this, "Enter a name for the new batch", strNewBatch, "",111,false,false,NULL))
			return;

		strNewBatch.TrimRight();
		if (strNewBatch == USER_BATCH_NAME || strNewBatch == "CurrentBatch" || strNewBatch.IsEmpty() || m_BatchList->FindByColumn(0, (LPCTSTR)strNewBatch, 0, false) != -1)
			MsgBox("Please select a different name for the new report batch");
		else
			bValid = TRUE;
	} while (!bValid);

	// Ok we have a valid batch here
	SetRemotePropertyMemo(CString("Reports.Batches.") + strNewBatch, "", 0, NULL);
	IRowSettingsPtr pRow = m_BatchList->GetRow(-1);
	pRow->PutValue(0,(LPCTSTR)strNewBatch);
	pRow->PutValue(1,"");
	m_BatchList->AddRow(pRow);
	m_BatchList->Sort();
	OnSelChosenBatchList(m_BatchList->FindByColumn(0, (LPCTSTR)strNewBatch, 0, TRUE));
}

void CReports::OnDeleteBatch()
{
	//TES 12/18/2008 - PLID 32514 - They can't modify Report Batches if they're on the
	// Scheduler Standard version.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Saved batches of commonly run reports", "The_Reports_Module/create_run_report_batches.htm")) {
		return;
	}

	//DRT 7/8/2005 - PLID 6308 - Check permissions.
	if(!CheckCurrentUserPermissions(bioReportBatch, sptDelete))
		return;

	if (m_strCurrentBatchName == USER_BATCH_NAME)
	{
		MsgBox("You can not delete your default batch");
		return;
	}

	if (IDNO == MsgBox(MB_YESNO, "Are you sure you wish to delete the %s batch?", m_strCurrentBatchName))
		return;

	try 
	{
		ExecuteSql("DELETE FROM ConfigRT WHERE NAME = 'Reports.Batches.%s' AND Username = '<None>'",
			_Q(m_strCurrentBatchName));

		m_BatchList->RemoveRow( m_BatchList->FindByColumn(0, (LPCTSTR)m_strCurrentBatchName, 0, FALSE));
		m_BatchList->CurSel = 0;
		OnSelChosenBatchList(0);
	}
	NxCatchAll("Error deleting report batch");
}

void CReports::OnSavebatchAs() 
{
	//TES 12/18/2008 - PLID 32514 - They can't modify Report Batches if they're on the
	// Scheduler Standard version.
	if(!g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Saved batches of commonly run reports", "The_Reports_Module/create_run_report_batches.htm")) {
		return;
	}

	//DRT 7/8/2005 - PLID 6308 - Check permissions.
	if(!CheckCurrentUserPermissions(bioReportBatch, sptCreate))
		return;

	//First, create the new batch.
	CString strNewBatch; // Here we do Gremlins 2 - The New Batch
	BOOL bValid = FALSE;

	do {
		if (IDCANCEL == InputBoxLimited(this, "Enter a name for the new batch", strNewBatch, "",111,false,false,NULL))
			return;

		strNewBatch.TrimRight();
		if (strNewBatch == USER_BATCH_NAME || strNewBatch == "CurrentBatch" || strNewBatch.IsEmpty() || m_BatchList->FindByColumn(0, (LPCTSTR)strNewBatch, 0, false) != -1)
			MsgBox("Please select a different name for the new report batch");
		else
			bValid = TRUE;
	} while (!bValid);

	// Ok we have a valid batch here
	SetRemotePropertyMemo(CString("Reports.Batches.") + strNewBatch, "", 0, NULL);
	IRowSettingsPtr pRow = m_BatchList->GetRow(-1);
	pRow->PutValue(0,(LPCTSTR)strNewBatch);
	pRow->PutValue(1,"");
	m_BatchList->AddRow(pRow);
	m_BatchList->Sort();
	m_BatchList->SetSelByColumn(0, (LPCTSTR)strNewBatch);
	m_strCurrentBatchName = strNewBatch;
	
	//Now, save the current list of reports to that new batch.
	SaveBatchChanges();
}

void CReports::OnReportHelp() 
{
	//(j.anspach 06-09-2005 10:53 PLID 16662) - Updating the OpenManual call to work with the new help system.
	//  With the new help system there are not help files for each individual type of report.  So we will just
	//  send them to the basic report help.

	/*CString strPage = "Overview_of_the_Reports_Module.htm";	//default to the base overview

	if(CurrReport) {

		//attempt to open right to the current tab of the selected report.  If they don't have a report selected, 
		//just open to the default overview.
		if(CurrReport->strCategory == "PatientP")
			strPage = "Tabs/Patients.htm";
		else if(CurrReport->strCategory == "OthrContactP")
			strPage = "Tabs/Contacts.htm";
		else if(CurrReport->strCategory == "MarketP")
			strPage = "Tabs/Marketing.htm";
		else if(CurrReport->strCategory == "InventoryP")
			strPage = "Tabs/Inventory.htm";
		else if(CurrReport->strCategory == "ScheduleP")
			strPage = "Tabs/Scheduling.htm";
		else if(CurrReport->strCategory == "ChargesP")
			strPage = "Tabs/Charges.htm";
		else if(CurrReport->strCategory == "PaymentsP")
			strPage = "Tabs/Payments.htm";
		else if(CurrReport->strCategory == "FinancialP")
			strPage = "Tabs/Financial.htm";
		else if(CurrReport->strCategory == "ASCP")
			strPage = "Tabs/ASC.htm";
		else if(CurrReport->strCategory == "AdminP")
			strPage = "Tabs/Administration.htm";
		else if(CurrReport->strCategory == "OtherP")
			strPage = "Tabs/Other.htm";
	}*/

	OpenManual("NexTech_Practice_Manual.chm", "The_Reports_Module/run_reports.htm");

}

void CReports::OnSearchReports()
{
	GetMainFrame()->OnActivitiesSearchReportDescriptions();
}

void CReports::OnSelChangingProvselect(long FAR* nNewSel) 
{
	if(*nNewSel == sriNoRow)
		*nNewSel = 0;	//reset sel to first row
}

LRESULT CReports::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
	
		switch(wParam) {

		case NetUtils::FiltersT:

			try {

				// (j.jones 2014-08-07 15:58) - PLID 63232 - if the Reports module is not active,
				// do nothing, because this tablechecker is checked again in UpdateView
				if (GetMainFrame() && !GetMainFrame()->IsActiveView(REPORT_MODULE_NAME)) {
					//the reports module is not active, so don't bother updating filters
					break;
				}

				//check to see if the use filter option is checked
				if (IsDlgButtonChecked(IDC_USEFILTER)) {
					long nFilterID = lParam;
					if (nFilterID == -1) {

						//just requery because they didn't send the value
						m_GroupSelect->Requery();
					}
					else {

						_RecordsetPtr rsFilter = CreateRecordset("SELECT ID, Name, Type FROM FiltersT WHERE ID = %li AND Type = 1 ", nFilterID);

						if (rsFilter->eof) {

							//remove the row from the datalist if it is there
							m_GroupSelect->RemoveRow(m_GroupSelect->FindByColumn(0, nFilterID, 0, FALSE));
						}
						else {

							//update or insert the row
							long nRowFilter = m_GroupSelect->FindByColumn(0, nFilterID, 0, FALSE);
							
							//get the row, 
							IRowSettingsPtr pRow = m_GroupSelect->GetRow(nRowFilter);

							pRow->PutValue(0, nFilterID);
							pRow->PutValue(1, _variant_t(AdoFldString(rsFilter, "Name")));
							pRow->PutValue(2, _variant_t(AdoFldLong(rsFilter, "Type")));

							if (nRowFilter == -1) {
								//insert
								m_GroupSelect->AddRow(pRow);
							}
						}
					}
				}
			} NxCatchAll("Error In CReports::OnTableChanged:FiltersT");
			break;
		
		case NetUtils::StatementConfig:

			try {
				if (m_SelectList) {
					//we really only care about this if a statement is is in the selected list
					if (m_SelectList->CurSel != -1) {
						long nID = VarLong(m_SelectList->GetValue(m_SelectList->GetCurSel(), 0), -1);
						if (nID != -1) {
							if (GetDlgItem(IDC_REPORT_TYPE_LIST)->IsWindowVisible()) {
								CReportInfo *pReport = FindReport(nID);
								
								if (GetDlgItem(IDC_REPORT_TYPE_LIST)->IsWindowVisible()) {
									// (j.gruber 2010-03-11 12:23) - PLID 29120 - they changed something in the config, reset the selection
									SetReportList(m_pReportTypeList, nID, pReport, TRUE);
								}
							}
							
							//loop through the list and warn them that the selection changed
							long p = m_SelectList->GetFirstRowEnum();
							LPDISPATCH lpDisp = NULL;
							BOOL bWarn = FALSE;
							while (p) {
								m_SelectList->GetNextRowEnum(&p, &lpDisp);
								IRowSettingsPtr pRow(lpDisp); lpDisp->Release();
								long nReportID = VarLong(pRow->GetValue(0), -1);
								
								if (IsStatement(nReportID) && nReportID != nID) {
									bWarn = TRUE;
								}				
							}

							if (bWarn) {
								MessageBox("The Statement Configuration settings have changed, please check that the correct statement report is available before running the statement");
							}
							
						}
					}
				}
			} NxCatchAll("Error In CReports::OnTableChanged:StatementConfig");
			
			break;
		}
	} NxCatchAll("Error In CReports::OnTableChanged");

	return 0;
}

void CReports::OnSelChosenReportTypeList(long nRow) 
{

	try {
		if (IsStatement(CurrReport->nID)) {
			CurrReport->strReportFile = VarString(m_pReportTypeList->GetValue(m_pReportTypeList->CurSel, 3));
			CurrReport->nDefaultCustomReport = VarLong(m_pReportTypeList->GetValue(m_pReportTypeList->CurSel, 1));
		}
	}NxCatchAll("Error In CReports::OnSelChosenReportTypeList");
	
}


// (j.gruber 2008-07-11 16:17) - PLID 30682 add quick date filter select
void CReports::ChangeSelectionQuickDateFilter(LPDISPATCH lpRow) {

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nValue;
		if (pRow == NULL) {
			nValue = -2;
		}
		else {
			nValue  = VarLong(pRow->GetValue(0));
		}

		if (nValue == -2) {
			//a separator or null, set it to custom
			m_pDateOptionList->SetSelByColumn(0, (long)2);
			nValue = 2;
		}

		long nYear, nMonth, nDay, nDayOfWeek;

		nYear = COleDateTime::GetCurrentTime().GetYear();
		nMonth = COleDateTime::GetCurrentTime().GetMonth();
		nDay = COleDateTime::GetCurrentTime().GetDay();
		nDayOfWeek = COleDateTime::GetCurrentTime().GetDayOfWeek();

		COleDateTimeSpan dtSpan;	
		COleDateTime dtFrom = m_from.GetDateTime();
		COleDateTime dtTo = m_to.GetDateTime();
		COleDateTime dtTemp;

		COleDateTime dtSetTo, dtSetFrom;

		// (j.gruber 2010-09-08 10:16) - PLID 37425 - also changed the values to an enum
		switch(nValue) {
			case erdvAll:
				//All Dates
				
				break;		
			case erdvOneYear: 
				//Year
				dtFrom.SetDate(nYear - 1, nMonth, nDay);
				dtTo =  COleDateTime::GetCurrentTime();
				
			break;

			case erdvCustom:
				//Custom
				m_rAllDates.SetCheck(0);
				m_rDateRangeOptions.SetCheck(1);
				EnableDateRange(true);
			break;

			case erdvToday:
				//Today
				dtFrom = COleDateTime::GetCurrentTime();
				dtTo = COleDateTime::GetCurrentTime();				
			break;

			case erdvThisWeek: 
				//This week:
				dtFrom.SetDate(nYear,nMonth,nDay);			
				//if this is Sunday, start from today
				if (nDayOfWeek != 0) {
					dtSpan.SetDateTimeSpan(nDayOfWeek - 1,0,0,0);
					dtFrom -= dtSpan;
				}	

				dtTo.SetDate(nYear,nMonth,nDay);
				dtSpan.SetDateTimeSpan(6,0,0,0);
				dtTo = dtFrom + dtSpan;				
			break;

			case erdvThisMonth: 
				//This Month
				dtFrom.SetDate(nYear, nMonth, 1);
				if (nMonth == 12) {
					dtTo.SetDate(nYear, 12, 31);
				}
				else {
					dtTemp.SetDate(nYear, nMonth + 1, 1);
					COleDateTimeSpan dtSpan(1,0,0,0);
					dtTo = dtTemp - dtSpan;
				}
				m_from.SetValue(_variant_t(dtFrom));					
			break;

			case erdvThisQuarter:
				//This Quarter
				//a quarter is 1/1 - 3/31, 4/1 - 6/30, 7/1 - 9/30, 10/1 - 12/31
				if(nMonth >= 1 && nMonth <= 3) {
					//quarter 1
					dtFrom.SetDate(nYear,1,1);
					dtTo.SetDate(nYear,3,31);
				}
				else if(nMonth >= 4 && nMonth <= 6) {
					//quarter 2
					dtFrom.SetDate(nYear,4,1);
					dtTo.SetDate(nYear,6,30);
				}
				else if(nMonth >= 7 && nMonth <= 9) {
					//quarter 3
					dtFrom.SetDate(nYear,7,1);
					dtTo.SetDate(nYear,9,30);
				}
				else if(nMonth >= 10 && nMonth <= 12) {
					//quarter 4
					dtFrom.SetDate(nYear,10,1);
					dtTo.SetDate(nYear,12,31);
				}
			break;

			case erdvThisYear: 
				//This year
				dtFrom.SetDate(nYear, 1, 1);
				dtTo.SetDate(nYear, 12,31);
			break;

			case erdvThisMonthToDate: 
				//This Month to date
				dtFrom.SetDate(nYear, nMonth, 1);
				dtTo = COleDateTime::GetCurrentTime();
			break;

			case erdvThisQuarterToDate:	 
				//This quarter to date
				//a quarter is 1/1 - 3/31, 4/1 - 6/30, 7/1 - 9/30, 10/1 - 12/31
				if(nMonth >= 1 && nMonth <= 3) {
					//quarter 1
					dtFrom.SetDate(nYear,1,1);
				}
				else if(nMonth >= 4 && nMonth <= 6) {
					//quarter 2
					dtFrom.SetDate(nYear,4,1);
				}
				else if(nMonth >= 7 && nMonth <= 9) {
					//quarter 3
					dtFrom.SetDate(nYear,7,1);
				}
				else if(nMonth >= 10 && nMonth <= 12) {
					//quarter 4
					dtFrom.SetDate(nYear,10,1);
				}
				dtTo = COleDateTime::GetCurrentTime();
			break;

			case erdvThisYearToDate:
				//this year to date
				dtFrom.SetDate(nYear, 1, 1);
				dtTo = COleDateTime::GetCurrentTime();
			break;

			// (j.gruber 2010-09-08 10:03) - PLID 37425 - added Yesterday
			case erdvYesterday:
				//Yesterday
				dtFrom.SetDate(nYear,nMonth,nDay);		
				dtSpan.SetDateTimeSpan(1,0,0,0);
				dtFrom -= dtSpan;
				dtTo = dtFrom;				
			break;


			case erdvLastWeek:
				//last week
				dtFrom.SetDate(nYear,nMonth,nDay);		
				dtSpan.SetDateTimeSpan(nDayOfWeek,0,0,0);
				dtFrom -= dtSpan;
				dtTo = dtFrom;
				dtSpan.SetDateTimeSpan(6,0,0,0);
				dtFrom -= dtSpan;
			break;

			case erdvLastMonth:
				//Last month
				dtFrom.SetDate(nYear, nMonth, 1);
				dtSpan.SetDateTimeSpan(1,0,0,0);
				dtFrom -= dtSpan;
				dtTo = dtFrom;
				dtFrom.SetDate(dtFrom.GetYear(),dtFrom.GetMonth(),1);

			break;

			case erdvLastQuarter:
				//last quarter
				//a quarter is 1/1 - 3/31, 4/1 - 6/30, 7/1 - 9/30, 10/1 - 12/31
				if(nMonth >= 1 && nMonth <= 3) {
					//Q1, so last quarter was Q4 of previous year
					dtFrom.SetDate(nYear-1,10,1);
					dtTo.SetDate(nYear-1,12,31);		
				}
				else if(nMonth >= 4 && nMonth <= 6) {
					//Q2, so last quarter was quarter 1
					dtFrom.SetDate(nYear,1,1);
					dtTo.SetDate(nYear,3,31);				
				}
				else if(nMonth >= 7 && nMonth <= 9) {
					//Q3, so last quarter was quarter 2
					dtFrom.SetDate(nYear,4,1);
					dtTo.SetDate(nYear,6,30);
				}
				else if(nMonth >= 10 && nMonth <= 12) {
					//Q4, so last quarter was quarter 3
					//(e.lally 2010-10-18) PLID 40970 - Fixed to be July-Sept, the real Q3
					dtFrom.SetDate(nYear,7,1);
					dtTo.SetDate(nYear,9,30);
				}
			break;

			case erdvLastYear: 
				//last year
				dtFrom.SetDate(nYear - 1, 1, 1);
				dtTo.SetDate(nYear - 1, 12, 31);
			break;

			default :
				//Separator or -1.
				//Set it to custom, leave the dates how they are.
			break;
		}

		if (nValue == erdvAll) {
			//all 
			if (CurrReport) {
				CurrReport->nDateRange = -1;
			}
			m_rAllDates.SetCheck(1);
			if (m_pDateOptions->GetRowCount() <= 1) {
				m_rDateRange.SetCheck(0);
			}
			else {
				m_rDateRangeOptions.SetCheck(0);
			}
			EnableDateRange(false);			
		}
		else {
			//separator or selected
			//set the display
			m_from.SetValue(_variant_t(dtFrom));
			m_to.SetValue(_variant_t(dtTo));

			//now set the report values
			if (CurrReport) {
				CurrReport->DateTo = dtTo;
				CurrReport->DateFrom = dtFrom;
				CurrReport->nDateRange = 1;
			}

			m_rAllDates.SetCheck(0);
			if (m_pDateOptions->GetRowCount() <= 1) {
				m_rDateRange.SetCheck(1);
			}
			else {
				m_rDateRangeOptions.SetCheck(1);
			}

			
			EnableDateRange(true);		
		}


			

	}NxCatchAll("Error In CReports::ChangeSelectionQuickDateFilter");


}

// (j.gruber 2008-07-11 16:17) - PLID 30692 - quick date filter select
void CReports::OnSelChosenReportsQuickDateFilter(LPDISPATCH lpRow) 
{
	try {

		ChangeSelectionQuickDateFilter(lpRow);

	}NxCatchAll("Error in CReports::OnSelChosenReportsQuickDateFilter");
	
}

void CReports::OnSelChangingReportsQuickDateFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		if (*lppNewSel == NULL) {
			// Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	
	}NxCatchAll("Error in CReports::OnSelChangingReportsQuickDateFilter");	
	
}

// (j.gruber 2008-07-22 09:08) - PLID 28976 - added All year filter
void CReports::OnReportsAllYear() 
{
	try {

		if (IsDlgButtonChecked(IDC_REPORTS_ALL_YEAR)) {
			if (CurrReport) {
				CurrReport->bUseAllYears = TRUE;
			}
		}
		else {
			if (CurrReport) {
				CurrReport->bUseAllYears = FALSE;
			}
		}

	}NxCatchAll("Error in CReports::OnReportsAllYear() ");
	
	
}

//(e.lally 2008-09-05) PLID 6780 - copied from providers and updated for locations
void CReports::OnSelChosenLocationSelect(long nRow) 
{
	try {
		// We must have a current report or else what are we doing?
		ASSERT(CurrReport);
		if (CurrReport) {
			if (nRow >= 0) {
				if(VarLong(m_LocationSelect->Value[nRow][0]) == -3) {
					//They selected "<Multiple Locations>"
					OnLocationList();
				}
				else if(VarLong(m_LocationSelect->Value[nRow][0]) == -2) {
					//They selected "<No Location>"
					CurrReport->m_dwLocations.RemoveAll();
					CurrReport->nLocation = -2;
				}
				else {
					//They selected a provider
					CurrReport->m_dwLocations.RemoveAll();
					CurrReport->nLocation = VarLong(m_LocationSelect->Value[nRow][0]);
					CurrReport->m_dwLocations.Add((DWORD)CurrReport->nLocation);
				}
			} else {
				//No selection is made -- this shouldn't be possible
				ASSERT(FALSE);
			}
		} else {
			// CurrReport was NULL
			AfxThrowNxException("Current Report cannot be NULL");
		}
	} NxCatchAll("Error in CReports::OnSelChosenLocationSelect");
	
}

void CReports::OnSelChangingLocationSelect(long FAR* nNewSel) 
{
	try{
		//(e.lally 2008-09-05) PLID 6780 - Disallow no selection
		if(*nNewSel == sriNoRow){
			*nNewSel = 0;	//reset sel to first row
		}
	}NxCatchAll("Error in CReports::OnSelChangingLocationSelect");
}

void CReports::OnSelChangingComboReportbatches(long* nNewSel)
{
	try {
		//TES 12/18/2008 - PLID 32514 - They can't use Report Batches if they're on the
		// Scheduler Standard version.
		if(*nNewSel != m_BatchList->CurSel && !g_pLicense->CheckSchedulerAccess_Enterprise(CLicense::cflrUse, "Saved batches of commonly run reports", "The_Reports_Module/create_run_report_batches.htm")) {
			//TES 12/18/2008 - PLID 32514 - This is not available for Scheduler Standard users.
			*nNewSel = m_BatchList->CurSel;
			m_BatchList->DropDownState = VARIANT_FALSE;
		}
	}NxCatchAll("Error in CReports::OnSelChangingComboReportbatches()");
}

// (z.manning 2009-12-07 10:27) - Note: The button for this handler is only visible if
// ENABLE_VERIFY_ALL_REPORTS is defined.
void CReports::OnBnClickedVerifyAllReports()
{
	try
	{
#ifdef ENABLE_VERIFY_ALL_REPORTS
		CheckIfAllReportsAreVerfied();
#endif

	}NxCatchAll(__FUNCTION__);
}


// (j.gruber 2009-12-28 17:34) - PLID 19189 - used when mismatch in location filters
BOOL CReports::GetAllowedFilteredLocations(CString strLocationFilter, long nCopyFromLocation, CDWordArray *dwCopyFromLocations, CDWordArray *dwAllowedLocations) {

	if (nCopyFromLocation == -2 || nCopyFromLocation == -1) {

		//all or none
		return TRUE;
	}

	_RecordsetPtr rsLocations = CreateParamRecordset(FormatString("SELECT ID FROM LocationsT WHERE %s", strLocationFilter));
	BOOL bLocationIDInFilter = FALSE;

	while (!rsLocations->eof) {
		long nLocID = AdoFldLong(rsLocations, "ID");

		//check to see if it is our filtered location
		if (nLocID == nCopyFromLocation) {
			bLocationIDInFilter = TRUE;
		}
		
		//now see if it is in our dword array
		BOOL bContinue = TRUE;
		for (int i = 0; i < dwCopyFromLocations->GetSize() && bContinue; i++) {

			if (nLocID == dwCopyFromLocations->GetAt(i)) {
				dwAllowedLocations->Add(nLocID);
				bContinue = FALSE;
			}
		}
		rsLocations->MoveNext();
	}

	return bLocationIDInFilter;
}

// (j.gruber 2009-12-28 17:34) - PLID 19189 - apply filters to selected reports
void CReports::OnBnClickedReportsApplyFilters()
{
	try {

		//make sure they have a selected report
		if (m_SelectList->CurSel == -1) {

			//message
			MsgBox("Please select a report in the current batch to copy filter values from.");
			return;
		}


		// pop up a message box, so they know what this does
		if (IDNO == DontShowMeAgain(NULL, "This will set location, provider, patient, and date filter values from the highlighted report to all the reports in the current batch where applicable.\nAre you sure you want to continue?", "ReportsApplyAllFilters","Practice",  false, true, false)) {
			return;
		}

		CWaitCursor cWait;

		//alrighty, we are good to go!!
		//get the selected report
		long nCopyFromReportID = m_SelectList->GetValue(m_SelectList->CurSel, 0);

		CReportInfo *pCopyFromReport = FindReport(nCopyFromReportID);

		CDWordArray dwLocations, dwProviders;
		long nLocationID = -1, nProvFilter = -1, nPatFilter = -1;
		COleDateTime dtTo, dtFrom;

		BOOL bFilterProv, bFilterLocation, bFilterPatient, bFilterDate;
		bFilterProv = bFilterLocation = bFilterPatient = bFilterDate = FALSE;

		CString strLocationWhereClause = pCopyFromReport->GetLocationWhereClause();

		//see what we CAN copy
		if (pCopyFromReport->nProvider != 0) {
			bFilterProv = TRUE;
		}
		if (pCopyFromReport->nLocation != 0) {
			bFilterLocation = TRUE;
		}
		if (pCopyFromReport->nPatient != 0) {
			bFilterPatient = TRUE;
		}
		if (pCopyFromReport->nDateFilter != 0) {
			bFilterDate = TRUE;
		}		

		//loop through all the selected reports, minus this one
		long p = m_SelectList->GetFirstRowEnum();
		LPDISPATCH lpDisp = NULL;		
		while (p) {
			m_SelectList->GetNextRowEnum(&p, &lpDisp);
			NXDATALISTLib::IRowSettingsPtr pRow(lpDisp); lpDisp->Release();

			//make sure we aren't on the selected row
			long nID = pRow->GetValue(0);
			if (nID != nCopyFromReportID) {

				//ok, so we have to copy over the filters
				CReportInfo *pCopyToReport = FindReport(nID);

				if (pCopyToReport->nLocation != 0 && bFilterLocation) {

					//it supports the location filter
					if (strLocationWhereClause == pCopyToReport->GetLocationWhereClause()) {
						//we know we are set
						pCopyToReport->nLocation = pCopyFromReport->nLocation;

						pCopyToReport->m_dwLocations.RemoveAll();
						for (int i = 0; i < pCopyFromReport->m_dwLocations.GetSize(); i++) {
							pCopyToReport->m_dwLocations.Add(pCopyFromReport->m_dwLocations.GetAt(i));
						}							
					}
					else {
						//load an array with allowed values
						CDWordArray dwAllowedLocations;
						BOOL bAllowLocationID;
						bAllowLocationID = GetAllowedFilteredLocations(pCopyToReport->GetLocationWhereClause(), pCopyFromReport->nLocation, &pCopyFromReport->m_dwLocations, &dwAllowedLocations); 

						if (dwAllowedLocations.GetSize() > 0) {
							//we have multiple locations, but we might not be able to use them all
							if (dwAllowedLocations.GetSize() == 1) {
								pCopyToReport->nLocation = dwAllowedLocations.GetAt(0);
							}
							else {

								pCopyToReport->nLocation = -3;
								
								pCopyToReport->m_dwLocations.RemoveAll();
								for (int i = 0; i < dwAllowedLocations.GetSize(); i++ ) {
									pCopyToReport->m_dwLocations.Add(dwAllowedLocations.GetAt(i));
								}
							}
						}
						else {

							if (bAllowLocationID) {
								pCopyToReport->nLocation = pCopyFromReport->nLocation;
							}					
						}
					}
				}

				if (pCopyToReport->nProvider != 0 && bFilterProv) {

					//it supports the provider filter										
					pCopyToReport->nProvider = pCopyFromReport->nProvider;
					pCopyToReport->m_dwProviders.RemoveAll();
					for (int i = 0; i < pCopyFromReport->m_dwProviders.GetSize(); i++) {
						pCopyToReport->m_dwProviders.Add(pCopyFromReport->m_dwProviders.GetAt(i));
					}						
				}

				if (pCopyToReport->nPatient != 0 && bFilterPatient) {
					pCopyToReport->nPatient = pCopyFromReport->nPatient;
				}

				if (pCopyToReport->nDateFilter != 0 && bFilterDate) {
					if (pCopyToReport->bOneDate != 0) {
						pCopyToReport->DateTo = pCopyFromReport->DateTo;
					}
					else {
						if (pCopyFromReport->bOneDate) {
							//we can only copy the one date we have
							pCopyToReport->DateTo = pCopyFromReport->DateTo;
							pCopyToReport->nDateRange = 1;
						}
						else {

							// (j.gruber 2011-07-22 12:47) - PLID 42220 - this needs to check if date range is set
							if (pCopyToReport->nDateRange != 0 ) {
								pCopyToReport->nDateRange = pCopyFromReport->nDateRange;
								pCopyToReport->DateFrom = pCopyFromReport->DateFrom;
								pCopyToReport->DateTo = pCopyFromReport->DateTo;
							}
						}
					}
				}

			}				
			
		}
		
	}NxCatchAll(__FUNCTION__);
}


void CReports::OnSelChangingReportType(long FAR* nNewSel) 
{
	try{
		// (j.gruber 2011-06-17 12:34) - PLID 38835 - Disallow no selection
		if(*nNewSel == sriNoRow){
			*nNewSel = 0;	//reset sel to first row
		}
	}NxCatchAll(__FUNCTION__);
}


void CReports::OnBnClickedBtnLaunchSsrsReporting()
{
	try {

		if (!g_pLicense->CheckForLicense(CLicense::lcEnterpriseReporting, CLicense::cflrUse)) {
			MsgBox("Enterprise Reporting is not available without a license.\n"
				"Please contact NexTech Sales if you wish to add this product.", MB_ICONEXCLAMATION | MB_OK);
			return;
		}

		SHELLEXECUTEINFO sei;
		memset(&sei, 0, sizeof(SHELLEXECUTEINFO));

		
		CString strParameter;
		strParameter.Format("\"%s\" \"%s\"", AsString(GetAPISubkey()), AsString(GetAPILoginToken()));

		CString strSSRSViewer = GetPracPath(PracPath::PracticePath) ^ "Report Viewer.exe";

		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.hwnd = (HWND)GetDesktopWindow();
		sei.lpFile = strSSRSViewer;
		sei.lpParameters = strParameter;
		sei.nShow = SW_SHOW;
		sei.hInstApp = NULL;

		// Run ShellExecute
		if (!ShellExecuteEx(&sei)) {
			MessageBox("Unable to launch SSRS Report Viewer!", "Nextech", MB_ICONERROR);
		}

	}NxCatchAll(__FUNCTION__);
}

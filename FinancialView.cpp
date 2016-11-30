// FinancialView.cpp : implementation file
//

//TS:  OK, first of all, the file is named FinancialView.cpp, but the first line claims that it is InvView.cpp,
//     and the class definition is CFinView.  Let's hear it for consistency.

#include "stdafx.h"
#include "practice.h"
#include "FinancialView.h"
#include "pracprops.h"
#include "PreferenceUtils.h"
#include "GlobalReportUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "boost/bind.hpp"
#include "EbillingFormDlg.h"
#include "BatchPrintDlg.h"
#include "PracBanking.h"
#include "BatchPayments.h"
#include "BillingFollowUpDlg.h"
#include "EEligibilityTabDlg.h"
#include "ReviewCCTransactionsDlg.h"
#include "GlobalFinancialUtils.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CFinView

IMPLEMENT_DYNCREATE(CFinView, CNxTabView)

// (j.jones 2013-05-07 11:52) - PLID 53969 - changed the child tabs to be declared by reference,
// which means their initialization is slightly different now
CFinView::CFinView()
	: m_EBilling(*(new CEbillingFormDlg(this)))
	, m_PaperBatch(*(new CBatchPrintDlg(this)))
	, m_PBanking(*(new PracBanking(this)))
	, m_BatchPayments(*(new CBatchPayments(this)))
	, m_BillingFollowUpDlg(*(new CBillingFollowUpDlg(this)))
	, m_EEligibilityTabDlg(*(new CEEligibilityTabDlg(this)))
	, m_ReviewCCDlg(*(new CReviewCCTransactionsDlg(this)))
{
}

CFinView::~CFinView()
{
	try {
		// (j.jones 2013-05-07 11:52) - PLID 53969 - changed the child tabs to be declared by reference,
		// which means we have to clean up their pointers now
		// They are never null. They have to have been filled in the constructor.
		delete &m_EBilling;
		delete &m_PaperBatch;
		delete &m_PBanking;
		delete &m_BatchPayments;
		delete &m_BillingFollowUpDlg;
		delete &m_EEligibilityTabDlg;
		delete &m_ReviewCCDlg;

	}NxCatchAll(__FUNCTION__);
}


BEGIN_MESSAGE_MAP(CFinView, CNxTabView)
	//{{AFX_MSG_MAP(CFinView)
	ON_WM_CREATE()
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(IDM_UPDATE_VIEW, CNxTabView::OnUpdateViewUI) // (z.manning 2010-07-19 16:06) - PLID 39222
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CFinView::CheckPermissions()
{
	if(!UserPermission(FinancialModuleItem)) {
		return FALSE;
	}
	return TRUE;
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CFinView::Hotkey (int key)
{
	if(IsWindowEnabled()) {
		int nKey = IgnoreRightAlt() ? VK_LMENU : VK_MENU;
		if (GetAsyncKeyState(nKey) & 0xE000)
		{	short CurrentTab = -1;
			switch (key)
			{
				case 'B':
					CurrentTab = FinancialModule::BankingTab;
					break;
				case 'P':
					CurrentTab = FinancialModule::PaperBatchTab;
					break;
				case 'E':
					CurrentTab = FinancialModule::EBillingTab;
					break;
				// (j.jones 2007-05-01 15:13) - PLID 8993 - added E-Eligibility Tab
				case 'G':
					CurrentTab = FinancialModule::EEligibilityTab;
					break;
				case 'A':
					CurrentTab = FinancialModule::BatchPayTab;
					break;
				case 'F':
					CurrentTab = FinancialModule::BillingFUTab;
					break;
				// (d.thompson 2009-06-30) - PLID 34745 - Added CC processing tab
				case 'C':
					CurrentTab = FinancialModule::QBMSTab;
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


int CFinView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	try {
		if (CNxTabView::OnCreate(lpCreateStruct) == -1)
			return -1;

		//TS 9-12-02: We're creating all sheets, then hiding some, otherwise things get all confused.
		
		Modules::Tabs& tabs = g_Modules[Modules::Financial]->Reset().GetTabs();

		// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to create the tabs

		CreateSheet(&m_PBanking, tabs, FinancialModule::BankingTab);
		// (d.thompson 2009-06-30) - PLID 34745 - Added CC Processing
		// (d.lange 2010-09-02 10:27) - PLID 40311 - ReviewCCTransactionsDlg replaced QBMS_ReviewTransactionsDlg,
		CreateSheet(&m_ReviewCCDlg, tabs, FinancialModule::QBMSTab);
		CreateSheet(&m_PaperBatch, tabs, FinancialModule::PaperBatchTab);
		CreateSheet(&m_EBilling, tabs, FinancialModule::EBillingTab);
		// (j.jones 2007-05-01 15:13) - PLID 8993 - added E-Eligibility Tab
		CreateSheet(&m_EEligibilityTabDlg, tabs, FinancialModule::EEligibilityTab);
		// (a.walling 2008-07-03 09:28) - PLID 27648 - Support the alternate Short Title
		CreateSheet(&m_BatchPayments, tabs, FinancialModule::BatchPayTab);
		// (d.thompson 2009-12-11) - PLID 36563 - Renamed from 'F.Up' to 'F/U'
		CreateSheet(&m_BillingFollowUpDlg, tabs, FinancialModule::BillingFUTab);

		//DRT 4/18/2008 - PLID 29711 - Set the text color of the tabs.
		// (j.jones 2016-04-18 17:46) - NX-100214 - the NxTab control now defaults
		// to the value of CNexTechDialog::GetDefaultNxTabTextColor()

		ShowTabs();
	} NxCatchAll(__FUNCTION__);

	return 0;
}

void CFinView::OnDraw(CDC* pDC) 
{
	CDocument* pDoc = GetDocument();	
	
}

void CFinView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	CNxTabView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CFinView::OnSelectTab(short newTab, short oldTab)//used for the new NxTab
{
	// (j.jones 2010-06-23 10:31) - if you add more tabs, be sure to update MainFrm's line for
	//UTBB_ADD_TO_ENABLED_VIEWS(TB_FINANCIAL,
	//to enable/disable based on your licensing,
	//update GlobalUtils GetDefaultModuleName() so it can't be auto-selected,
	//and update CMainFrame::OnFinancialModule()

	//TES 10/29/03: If this tab is being selected by the initialization of the module, 
	//which will be the case if newTab == oldTab, then silently see if they have read permissions,
	//and if they don't, send them back to the banking tab.
	
	if (newTab == FinancialModule::BankingTab) {
		if(oldTab == FinancialModule::BankingTab) {
			if(!(GetCurrentUserPermissions(bioBankingTab) & SPT__R________) || !g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
				OnSelectTab(FinancialModule::PaperBatchTab, FinancialModule::PaperBatchTab);
				return;
			}
			else {
				m_tab->CurSel = FinancialModule::BankingTab;
			}
		}
		else if (!CheckCurrentUserPermissions(bioBankingTab,sptRead) || !g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
			m_tab->CurSel = oldTab;
			return;
		}
	}
	else if (newTab == FinancialModule::PaperBatchTab) {
		if(oldTab == FinancialModule::PaperBatchTab) {
			if(!(GetCurrentUserPermissions(bioClaimForms) & SPT__R________) || !g_pLicense->CheckForLicense(CLicense::lcHCFA, CLicense::cflrUse)) {
				OnSelectTab(FinancialModule::EBillingTab,FinancialModule::EBillingTab);				
				return;
			}
			else {
				m_tab->CurSel = FinancialModule::PaperBatchTab;
			}
		}
		else if (!CheckCurrentUserPermissions(bioClaimForms,sptRead) || !g_pLicense->CheckForLicense(CLicense::lcHCFA, CLicense::cflrUse)) {
			m_tab->CurSel = oldTab;
			return;
		}
	}
	else if (newTab == FinancialModule::EBillingTab) {
		if(oldTab == FinancialModule::EBillingTab) {
			if(!(GetCurrentUserPermissions(bioEBilling) & SPT__R________) || !g_pLicense->CheckForLicense(CLicense::lcEbill, CLicense::cflrUse)) {
				OnSelectTab(FinancialModule::EEligibilityTab,FinancialModule::EEligibilityTab);
				return;
			}
			else {
				m_tab->CurSel = FinancialModule::EBillingTab;
			}
		}
		else if (!CheckCurrentUserPermissions(bioEBilling,sptRead) || !g_pLicense->CheckForLicense(CLicense::lcEbill, CLicense::cflrUse)) {
			m_tab->CurSel = oldTab;
			return;
		}
	}
	// (j.jones 2007-05-01 15:13) - PLID 8993 - added E-Eligibility Tab
	else if (newTab == FinancialModule::EEligibilityTab) {
		if(oldTab == FinancialModule::EEligibilityTab) {
			// (j.jones 2007-06-29 09:11) - PLID 23950 - added E-Eligibility licensing
			// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
			if(!(GetCurrentUserPermissions(bioEEligibility) & SPT__R________) || !g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrSilent)) {
				OnSelectTab(FinancialModule::BatchPayTab,FinancialModule::BatchPayTab);
				return;
			}
			else {
				m_tab->CurSel = FinancialModule::EEligibilityTab;
			}
		}
		// (j.jones 2011-05-06 15:44) - PLID 40430 - added eligibility permissions
		else if (!CheckCurrentUserPermissions(bioEEligibility,sptRead) || !g_pLicense->CheckForLicense(CLicense::lcEEligibility, CLicense::cflrSilent)) {
			m_tab->CurSel = oldTab;
			return;
		}
	}// (s.tullis 2014-06-03 15:39) - PLID 49455 - Permission: Batch payment User Permission to Control Read and Wring in the batch Payment Tab
	else if (newTab == FinancialModule::BatchPayTab) {
		if(oldTab == FinancialModule::BatchPayTab) {
			if (!(GetCurrentUserPermissions(bioBatchPayment) & SPT__R________) || !g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
				OnSelectTab(FinancialModule::BillingFUTab, FinancialModule::BillingFUTab);
				return;
			}
			else {
				m_tab->CurSel = FinancialModule::BatchPayTab;
			}
		}
		else if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent) || !(GetCurrentUserPermissions(bioBatchPayment) & SPT__R________)) {
			m_tab->CurSel = oldTab;
			return;
		}
	}// (s.tullis 2014-06-23 11:23) - PLID 62506 - Permission: Billing Followup User Permission to Control Read and Writing in the Billing Followup Tab
	else if (newTab == FinancialModule::BillingFUTab) {
		if(oldTab == FinancialModule::BillingFUTab) {
			if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent) || !(GetCurrentUserPermissions(bioBillingfollowup) & SPT__R________)) {
				
				OnSelectTab(FinancialModule::BankingTab, FinancialModule::BankingTab);
				return;
			}
			else {
				m_tab->CurSel = FinancialModule::BillingFUTab;
			}
		}
		else if (!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent) || !(GetCurrentUserPermissions(bioBillingfollowup) & SPT__R________)) {
			m_tab->CurSel = oldTab;
			return;
		}
	}
	// (d.thompson 2009-06-30) - PLID 34745 - added QBMS tab (go back to Banking if no license)
	// (d.thompson 2010-09-02) - PLID 40371 - This should show up for any cc processing license
	else if (newTab == FinancialModule::QBMSTab) {
		if(oldTab == FinancialModule::QBMSTab) {
			// (j.jones 2015-09-30 10:46) - PLID 67177 - this tab is not available if ICCP is licensed and enabled, IsICCPEnabled checks both
			if(!(GetCurrentUserPermissions(bioCCProcessingTab) & SPT_V_________) || !g_pLicense || !g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent)
				|| IsICCPEnabled()) {
				OnSelectTab(FinancialModule::BankingTab, FinancialModule::BankingTab);
				return;
			}
			else {
				m_tab->CurSel = FinancialModule::QBMSTab;
			}
		}
		// (j.jones 2015-09-30 10:46) - PLID 67177 - this tab is not available if ICCP is licensed and enabled, IsICCPEnabled checks both
		else if (!CheckCurrentUserPermissions(bioCCProcessingTab, sptView) || !g_pLicense || !g_pLicense->HasCreditCardProc_Any(CLicense::cflrSilent)
			|| IsICCPEnabled()) {

			if (IsICCPEnabled()) {
				//give a warning
				AfxMessageBox("The Credit Cards tab is not available when Integrated Credit Card Processing is enabled.");
			}
			m_tab->CurSel = oldTab;
			return;
		}
	}
	
	// (j.jones 2010-06-23 10:31) - if you add more tabs, be sure to update MainFrm's line for
	//UTBB_ADD_TO_ENABLED_VIEWS(TB_FINANCIAL,
	//to enable/disable based on your licensing,
	//update GlobalUtils GetDefaultModuleName() so it can't be auto-selected,
	//and update CMainFrame::OnFinancialModule()

	else {
		m_tab->CurSel = newTab;
	}

	CNxTabView::OnSelectTab(newTab, oldTab);
}

void CFinView::ShowTabs()
{
	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to update the tabs
	Modules::Tabs& tabs = g_Modules[Modules::Financial]->GetTabs();
	std::for_each(tabs.begin(), tabs.end(), boost::bind(&Modules::NxTabUpdater, m_tab, _1));

	// (d.thompson 2009-11-17) - PLID 36301 - Workaround.  Unfortunately this module can be setup so that a user has
	//	access to the module, but all the tabs are hidden.  Our architecture just doesn't handle that kind of work, 
	//	and trouble ensues.  As a workaround, we're just going to detect that state, tell the user it's a bad config
	//	(what are they really expecting to happen?), and throw an exception.  Since we're in the OnCreate handler, this
	//	will likely cause practice to crash.  TODO:  PLID 36318 will find a better solution for this.
	bool bAtLeastOneVisible = false;
	for(int i = 0; i < m_tab->Size; i++) {
		if(m_tab->ShowTab[i] != VARIANT_FALSE) {
			bAtLeastOneVisible = true;
		}
	}

	if(!bAtLeastOneVisible) {
		AfxThrowNxException("You have permission to the financial module, but you do not have permission to any tabs.  This "
			"is an invalid state, please correct your permissions.");
	}
}

int CFinView::ShowPrefsDlg()
{
	if (m_pActiveSheet == &m_EBilling) {
		return ShowPreferencesDlg(GetRemoteData(), GetCurrentUserName(), GetRegistryBase(), piEbilling);
	}
	else if (m_pActiveSheet == &m_BatchPayments) {
		return ShowPreferencesDlg(GetRemoteData(), GetCurrentUserName(), GetRegistryBase(), piBatchPayments);
	}
	else {
		return ShowPreferencesDlg(GetRemoteData(), GetCurrentUserName(), GetRegistryBase(), piFinancialModule);
	}
}

LRESULT CFinView::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {

		// (j.jones 2014-08-08 10:16) - PLID 63232 - if the Financial module is not active,
		// do nothing, because each tab properly updates itself when it becomes active again
		if (GetMainFrame() && !GetMainFrame()->IsActiveView(FINANCIAL_MODULE_NAME)) {
			//the financial module is not active, so don't bother updating the active tab
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
LRESULT CFinView::OnTableChangedEx(WPARAM wParam, LPARAM lParam)
{
	try {

		// (j.jones 2014-08-08 10:16) - PLID 63232 - if the Financial module is not active,
		// do nothing, because each tab properly updates itself when it becomes active again
		if (GetMainFrame() && !GetMainFrame()->IsActiveView(FINANCIAL_MODULE_NAME)) {
			//the financial module is not active, so don't bother updating the active tab
			return 0;
		}

		if (m_pActiveSheet) {
			return m_pActiveSheet->SendMessage(WM_TABLE_CHANGED_EX, wParam, lParam);
		}

	} NxCatchAll(__FUNCTION__);

	return 0;
}

void CFinView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{	
	CNxTabView::OnPrint(pDC, pInfo);
}

void CFinView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	pInfo->SetMinPage(1);
	pInfo->SetMaxPage(1);
	
	CNxTabView::OnBeginPrinting(pDC, pInfo);
}

void CFinView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	CNxTabView::OnEndPrinting(pDC, pInfo);
}

BOOL CFinView::OnPreparePrinting(CPrintInfo* pInfo) 
{

	try {
		CString str, 
				sql;
		
		CPtrArray			paParam;

		CNxDialog* ptrCurrentTab = GetActiveSheet();

		if (ptrCurrentTab == &m_BatchPayments) {

			// Show immediate feedback by way of the wait cursor
			CWaitCursor wc;

			if(m_BatchPayments.m_BatchPayments->CurSel == -1) {
				AfxMessageBox("You must select a batch payment first before you can run the report.");
				return FALSE;
			}

			long nBatchPayID = m_BatchPayments.m_BatchPayments->GetValue(m_BatchPayments.m_BatchPayments->CurSel,0).lVal;

			// Create a copy of the report object
			CReportInfo rep(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(322)]);

			rep.nDateFilter = 1;
			rep.nDateRange = -1;

			rep.SetExtraValue(AsString(nBatchPayID));

			//Set up the parameters.
			CPtrArray paParams;
			CRParameterInfo *paramInfo;
			
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = GetCurrentUserName();
			paramInfo->m_Name = "CurrentUserName";
			paParams.Add(paramInfo);

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "01/01/1000";
			paramInfo->m_Name = "DateFrom";
			paParams.Add((void *)paramInfo);

			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "12/31/5000";
			paramInfo->m_Name = "DateTo";
			paParams.Add((void *)paramInfo);

			//check to see if there is a default report
			_RecordsetPtr rsDefault = CreateRecordset("SELECT ID, CustomReportID FROM DefaultReportsT WHERE ID = 322");
			CString strFileName;

			if (rsDefault->eof) {

				strFileName = "BatchPaymentsService";
			}
			else {
				
				long nDefaultCustomReport = AdoFldLong(rsDefault, "CustomReportID", -1);

				if (nDefaultCustomReport > 0) {

					_RecordsetPtr rsFileName = CreateRecordset("SELECT FileName FROM CustomReportsT WHERE ID = 322 AND Number = %li", nDefaultCustomReport);

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
					strFileName = "BatchPaymentsService";
					
				}
			}

			rep.strReportFile = strFileName;

			//Made new function for running reports - JMM 5-28-04
			RunReport(&rep, &paParams, pInfo->m_bPreview, (CWnd *)this, "Batch Payments", pInfo);
			ClearRPIParameterList(&paParams);

			return FALSE;

		}
		else {
			return CNxTabView::OnPreparePrinting(pInfo);
		}

	}NxCatchAll("Error in CFinView::OnPreparePrinting()");
	return FALSE;
}

void CFinView::OnUpdateFilePrint(CCmdUI* pCmdUI) 
{
	CNxDialog* ptrCurrentTab = GetActiveSheet();
	
	if (ptrCurrentTab == &m_BatchPayments) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
}

void CFinView::OnUpdateFilePrintPreview(CCmdUI* pCmdUI) 
{
	CNxDialog* ptrCurrentTab = GetActiveSheet();
	
	if (ptrCurrentTab == &m_BatchPayments) {
		pCmdUI->Enable(TRUE);
	}
	else {
		pCmdUI->Enable(FALSE);
	}
}
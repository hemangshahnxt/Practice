// MarketView.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "MarketView.h"
#include "MarketPrintSetupDlg.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "marketutils.h"
#include "MarketReferralsPrintSetupDlg.h"
#include "Docbar.h"
#include "MarketRetentionPrintSetupDlg.h"
#include "GlobalReportUtils.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "boost/bind.hpp"
#include "MarketCost.h"
#include "MarketEffect.h"
#include "MarketZipDlg.h"
#include "MarketRetentionGraphDlg.h"
#include "MarketPatCoordDlg.h"
#include "MarketReferralSourceDlg.h"
#include "MarketProcedureDlg.h"
#include "MarketDateDlg.h"
#include "MarketOtherDlg.h"
#include "MarketBaselineDlg.h"
#include "MarketInternalDlg.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMarketView

IMPLEMENT_DYNCREATE(CMarketView, CNxTabView)

// (j.jones 2013-05-08 09:10) - PLID 56591 - changed the child tabs to be declared by reference,
// which means their initialization is slightly different now
CMarketView::CMarketView()
	: m_costSheet(*(new CMarketCostDlg(this)))
	, m_effectSheet(*(new CEffectiveness(this)))
	, m_zipSheet(*(new CMarketZipDlg(this)))
	, m_RetentionSheet(*(new CMarketRetentionGraphDlg(this)))
	, m_PatCoordSheet(*(new CMarketPatCoordDlg(this)))
	, m_RefSourSheet(*(new CMarketReferralSourceDlg(this)))
	, m_ProcedureSheet(*(new CMarketProcedureDlg(this)))
	, m_DateSheet(*(new CMarketDateDlg(this)))
	, m_OtherSheet(*(new CMarketOtherDlg(this)))
	, m_BaselineSheet(*(new CMarketBaselineDlg(this)))
	, m_InternalSheet(*(new CMarketInternalDlg(this)))
{
	m_bMarketToolBar = true;
	m_bDefaultTabLoading = FALSE;
	m_bFocus = false;
	
}

CMarketView::~CMarketView()
{
	try {

		// (j.jones 2013-05-08 09:10) - PLID 56591 - changed the child tabs to be declared by reference,
		// which means we have to clean up their pointers now
		// They are never null. They have to have been filled in the constructor.
		delete &m_costSheet;
		delete &m_effectSheet;
		delete &m_zipSheet;
		delete &m_RetentionSheet;
		delete &m_PatCoordSheet;
		delete &m_RefSourSheet;
		delete &m_ProcedureSheet;
		delete &m_DateSheet;
		delete &m_OtherSheet;
		delete &m_BaselineSheet;
		delete &m_InternalSheet;
	
	}NxCatchAll(__FUNCTION__);
}

BEGIN_MESSAGE_MAP(CMarketView, CNxTabView)
	//{{AFX_MSG_MAP(CMarketView)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
//	ON_COMMAND(ID_FILE_PRINT_DIRECT, CNxTabView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_MESSAGE(NXM_MARKET_READY, OnMarketReady)
	ON_UPDATE_COMMAND_UI(IDM_UPDATE_VIEW, CNxTabView::OnUpdateViewUI) // (z.manning 2010-07-19 16:06) - PLID 39222
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMarketView diagnostics

#ifdef _DEBUG
void CMarketView::AssertValid() const
{
	CNxTabView::AssertValid();
}

void CMarketView::Dump(CDumpContext& dc) const
{
	CNxTabView::Dump(dc);
}

#endif //_DEBUG

//////////////////////////////////////
BOOL CMarketView::CheckPermissions()
{
	//PLID 16078 JMM - 4/28/2005 - made retention licensed separately
	if(!g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrUse) && !g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrJustCheckingLicenseNoUI))
		return FALSE;

	if (!UserPermission(MarketingModuleItem)) {
		return FALSE;
	}
	
	return TRUE;
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CMarketView::Hotkey (int key)
{
	if(IsWindowEnabled()) {
		int nKey = IgnoreRightAlt() ? VK_LMENU : VK_MENU;
		if (GetAsyncKeyState(nKey) & 0xE000)
		{	short tab = -1;
			if (m_nUseMarketing == 1) {
				switch (key)
				{	
					case 'R':
						tab = 0;
						break;
					case 'U':
						tab = 1;
						break;
					case 'O':
						tab = 2;
					break;
					case 'D':
						tab = 3;
					break;
					case 'S':
						tab = 4;
					break;
					case 'P':
						tab = 5;
					break;
					case 'E':
						tab = 6;
					break;
					case 'I':
						if (!g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrSilent)) {
							tab = 8;
						}
						else {
							tab = 7;
						}
					break;
					case 'C':
						if (!g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrSilent)) {
							tab = 9;
						}
						else {
							tab = 8;
						}
					break;
					case 'L':
						if (!g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrSilent)) {
							tab = 10;
						}
						else {
							tab = 9;
						}
				}
			}
			else if (m_nUseRetention == 1) {
				switch(key) {
					case 'N':
						tab = 7;
					break;
				}
			
			}
			if (tab != -1) {
				SetActiveTab(tab);
				return 0;
			}
		}
	}

	return CNxTabView::Hotkey(key);
}

void CMarketView::OnSelectTab(short newTab, short oldTab) {

	//if they don't have a license for it, flip them to the referrals tab, if they don't have marketing,
	//flip them to the patients module

	if (newTab == MarketingModule::RetentionTab) {

		//if retention is the default tab we have to do this or else we get stuck in an infinte loop of questions
		//so we asked before and now we don't need to check here
		if (!m_bDefaultTabLoading) {
			m_nUseRetention = g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrUse);
		}
		else {
			m_bDefaultTabLoading = FALSE;
		}
		
		if (m_nUseRetention == 0) {
			if (oldTab == MarketingModule::RetentionTab) {
				oldTab = 0;
			}
			OnSelectTab(oldTab, oldTab);
			m_tab->CurSel = oldTab;
			return;
		}
		
		if (oldTab == MarketingModule::RetentionTab) {
			m_tab->CurSel = oldTab;
		}
	}		
	else if (newTab == MarketingModule::ReferralTab) {

		//see if they have licensing
		if(m_nUseMarketing == 0) {
			//check to see if they have retention
			if (m_nUseRetention == 0) {
				//send them to the patients module
				GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME);
			}
			else {
				//send them to retention
				OnSelectTab(MarketingModule::RetentionTab, MarketingModule::RetentionTab);
				m_tab->CurSel = MarketingModule::RetentionTab;
				return;
			}
		}
	
	}
	else if (newTab == MarketingModule::ProcedureTab) {

		//see if they have licensing
		if(m_nUseMarketing == 0) {
			//check to see if they have retention
			if (m_nUseRetention == 0) {
				//send them to the patients module
				GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME);
			}
			else {
				//send them to retention
				OnSelectTab(MarketingModule::RetentionTab, MarketingModule::RetentionTab);
				m_tab->CurSel = MarketingModule::RetentionTab;
				return;
			}
		}

	}
	else if (newTab == MarketingModule::CoordinatorTab) {

		//see if they have licensing
		if(m_nUseMarketing == 0) {
			//check to see if they have retention
			if (m_nUseRetention == 0) {
				//send them to the patients module
				GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME);
			}
			else {
				//send them to retention
				OnSelectTab(MarketingModule::RetentionTab, MarketingModule::RetentionTab);
				m_tab->CurSel = MarketingModule::RetentionTab;
				return;
			}
		}

	}
	else if (newTab == MarketingModule::DateTab) {

		//see if they have licensing
		if(m_nUseMarketing == 0) {
			//check to see if they have retention
			if (m_nUseRetention == 0) {
				//send them to the patients module
				GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME);
			}
			else {
				//send them to retention
				OnSelectTab(MarketingModule::RetentionTab, MarketingModule::RetentionTab);
				m_tab->CurSel = MarketingModule::RetentionTab;
				return;
			}
		}

	}
	else if (newTab == MarketingModule::ReasonTab) {

		//see if they have licensing
		if(m_nUseMarketing == 0) {
			//check to see if they have retention
			if (m_nUseRetention == 0) {
				//send them to the patients module
				GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME);
			}
			else {
				//send them to retention
				OnSelectTab(MarketingModule::RetentionTab, MarketingModule::RetentionTab);
				m_tab->CurSel = MarketingModule::RetentionTab;
				return;
			}
		}

	}
	else if (newTab == MarketingModule::PerfIndTab) {

		//see if they have licensing
		if(m_nUseMarketing == 0) {
			//check to see if they have retention
			if (m_nUseRetention == 0) {
				//send them to the patients module
				GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME);
			}
			else {
				//send them to retention
				OnSelectTab(MarketingModule::RetentionTab, MarketingModule::RetentionTab);
				m_tab->CurSel = MarketingModule::RetentionTab;
				return;
			}
		}

	}
	else if (newTab == MarketingModule::RegionalTab) {

		//see if they have licensing
		if(m_nUseMarketing == 0) {
			//check to see if they have retention
			if (m_nUseRetention == 0) {
				//send them to the patients module
				GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME);
			}
			else {
				//send them to retention
				OnSelectTab(MarketingModule::RetentionTab, MarketingModule::RetentionTab);
				m_tab->CurSel = MarketingModule::RetentionTab;
				return;
			}
		}

	}
	else if (newTab == MarketingModule::EffectivenessTab) {

		//see if they have licensing
		if(m_nUseMarketing == 0) {
			//check to see if they have retention
			if (m_nUseRetention == 0) {
				//send them to the patients module
				GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME);
			}
			else {
				//send them to retention
				OnSelectTab(MarketingModule::RetentionTab, MarketingModule::RetentionTab);
				m_tab->CurSel = MarketingModule::RetentionTab;
				return;
			}
		}

	}
	else if (newTab == MarketingModule::CostTab) {

		//see if they have licensing
		if(m_nUseMarketing == 0) {
			//check to see if they have retention
			if (m_nUseRetention == 0) {
				//send them to the patients module
				GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME);
			}
			else {
				//send them to retention
				OnSelectTab(MarketingModule::RetentionTab, MarketingModule::RetentionTab);
				m_tab->CurSel = MarketingModule::RetentionTab;
				return;
			}
		}

	}
			
	CNxTabView::OnSelectTab(newTab, oldTab);
}


void CMarketView::ShowTabs()
{
	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to update the tabs
	Modules::Tabs& tabs = g_Modules[Modules::Marketing]->GetTabs();

	if (m_nUseRetention == 1) {
		tabs[MarketingModule::RetentionTab]->Enable(true).Show(true);
	}
	else {
		if (m_bDefaultTabLoading) {
			//if it is the default tab, we are still going to show it
			tabs[MarketingModule::RetentionTab]->Enable(true).Show(true);
		}
		else {
			tabs[MarketingModule::RetentionTab]->Enable(false).Show(false);
		}
	}

	std::for_each(tabs.begin(), tabs.end(), boost::bind(&Modules::NxTabUpdater, m_tab, _1));
}
///////////////////////////////////////
// CMarketView message handlers
int CMarketView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxTabView::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_nUseMarketing = GetMainFrame()->m_pDocToolBar->m_nUseMarketing;
	m_nUseRetention = GetMainFrame()->m_pDocToolBar->m_nUseRetention;

	// (z.manning 2009-09-09 14:18) - PLID 35051 - Cache some preferences used in multiple tabs
	g_propManager.CachePropertiesInBulk("CMarketView", propNumber,
		"(Username = '<None>' OR Username = '%s') AND Name IN ("
		"	'CRGSplitConsults' "
		//(e.lally 2009-09-28) PLID 35521
		"	, 'MarketingUseDetailedProcedureFilter' "
		")",
		_Q(GetCurrentUserName()));
	g_propManager.CachePropertiesInBulk("CMarketView", propText,
		"(Username = '<None>' OR Username = '%s') AND Name IN ("
		"	'CRGPurposeList' "
		"	, 'CRGConsultList' "
		"	, 'CRGSurgeryList' "
		"	, 'CRGConsultLabels' "
		"	, 'CRGSurgeryLabel' "
		//(e.lally 2009-09-14) PLID 35527
		"	, 'CRGSingleConsultList' "
		"	, 'CRGSingleConsultLabels' "
		")",
		_Q(GetCurrentUserName()));

	if (m_nUseMarketing == -2 || m_nUseRetention == -2) {
		//they aren't set yet, we need to set them ourselves
		m_nUseMarketing = g_pLicense->CheckForLicense(CLicense::lcMarket, CLicense::cflrUse);

		if (m_nUseMarketing == 0) {
			m_nUseRetention = g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrUse);

			//if they are both false, kick them out of the module
			if (m_nUseRetention == 0) {
				return -1; 
			}
		}
		else {
			//check retention silently
			m_nUseRetention = g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrSilent);
		}
	}

	//check to see if retention is the default tab, and if so, ask now
	if ((short)GetRemotePropertyInt("MyDefaultTab_Marketing" , 0, 0, GetCurrentUserName(), false) == 7) {

		//check for retention now
		m_nUseRetention = g_pLicense->CheckForLicense(CLicense::lcRetention, CLicense::cflrUse);
		m_bDefaultTabLoading = TRUE;
	}

	
	
	 
	//DRT 4/11/03 - Removed the CreateSheet for the m_conversionSheet, it has never
	//		been used in about 2 years, and was replaced by code in referralSheet 
	//		shortly after it's creation.

	//CreateSheet(&m_referralSheet,	"Compa&re");	
	//CreateSheet(&m_ConversionSheet,   "Con&version");
	//CreateSheet(&m_EffectivenessSheet,   "&Effectiveness");
	//CreateSheet(&m_FilteredTotalsSheet,   "&Filtered Totals");
	
	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to create the tabs
	Modules::Tabs& tabs = g_Modules[Modules::Marketing]->Reset().GetTabs();
	
	CreateSheet(&m_RefSourSheet,		tabs, MarketingModule::ReferralTab);
	CreateSheet(&m_ProcedureSheet,		tabs, MarketingModule::ProcedureTab);
	CreateSheet(&m_PatCoordSheet,		tabs, MarketingModule::CoordinatorTab);
	CreateSheet(&m_DateSheet,		tabs, MarketingModule::DateTab);
	CreateSheet(&m_OtherSheet,		tabs, MarketingModule::ReasonTab);
	// (a.walling 2008-07-03 09:34) - PLID 27648 - Use the alternate short label
	CreateSheet(&m_BaselineSheet,	tabs, MarketingModule::PerfIndTab);
	CreateSheet(&m_zipSheet,		tabs, MarketingModule::RegionalTab);
	CreateSheet(&m_RetentionSheet,	tabs, MarketingModule::RetentionTab);
	//CreateSheet(&m_trendsSheet,		"&Trends");
	CreateSheet(&m_effectSheet,		tabs, MarketingModule::EffectivenessTab);
	CreateSheet(&m_costSheet,		tabs, MarketingModule::CostTab);

	//CreateSheet(&m_trendsSheet,		"&Trends");
	
	if(IsNexTechInternal())
		CreateSheet(&m_InternalSheet,	tabs, MarketingModule::InternalTab);

	// (c.haag 2004-04-13 11:17) - The default tab width is 12. If the size is 10,
	// we are wasting two tabs worth of space. Looks very unprofessional!
	m_tab->TabWidth = m_tab->Size;

	//DRT 4/18/2008 - PLID 29711 - Set the text color of the tabs.
	// (j.jones 2016-04-18 17:46) - NX-100214 - the NxTab control now defaults
	// to the value of CNexTechDialog::GetDefaultNxTabTextColor()

	ShowTabs();
	
	return 0;
}

void CMarketView::OnUpdateFilePrint(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();	
}

void CMarketView::OnUpdateFilePrintPreview(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable();	
}

void CMarketView::OnFilePrint() 
{
	CNxTabView::OnFilePrint();
}

void CMarketView::OnFilePrintPreview() 
{
	CNxTabView::OnFilePrintPreview();
}

BOOL CMarketView::OnPreparePrinting(CPrintInfo* pInfo) 
{

	try {

		// (j.jones 2010-07-19 17:36) - PLID 39053 - Remember that reports use the snapshot connection (if one is supported).
		// Also, if printed, the temp table will not be dropped until the connection ends.
		ADODB::_ConnectionPtr pCon = GetRemoteDataSnapshot();

		// (j.gruber 2011-05-26 16:28) - PLID 43869 - Increate the timeout
		CIncreaseCommandTimeout cict(GetRemoteDataSnapshot(), 600);

		CString strPatientTempTable;

		CNxDialog* ptrCurrentTab = GetActiveSheet();
		CString strFileName, strReportName;
		long nReportID;

		if(GetMainFrame() == NULL || GetMainFrame()->m_pDocToolBar == NULL){
			ASSERT(FALSE);
			ThrowNxException("CMarketView::OnPreparePrinting : Invalid MainFrame pointer.");
		}

		// filter string
		CRParameterInfo *pFilter = new CRParameterInfo;
		CPtrArray paramList;
		pFilter->m_Data = GetMainFrame()->m_pDocToolBar->GetDescription();
		pFilter->m_Name = (CString)"FilterDescription";
		paramList.Add((void *)pFilter);

		//Date filter
		MarketFilter mfDateFilter = GetMainFrame()->m_pDocToolBar->GetFilterType(mftDate);

		if(ptrCurrentTab == &m_RefSourSheet) {

			long nReportID, nProvID = -1;
			BOOL bEnableAll = TRUE;

			MarketGraphType mktType = (MarketGraphType)GetType();

			switch (mktType) {

				case EFFMonByRefSour:
					nReportID = 502;
				break;

				case CNVConsToSurgByRefSour:
					// (j.gruber 2010-10-22 13:08) - PLID 24764 - check if they've set the consults
					if (! IsConsToProcSetupValid()) {
						MessageBox("Please set up your consult and procedure configuration settings by clicking the ... button before running this graph");			
						ClearRPIParameterList(&paramList);
						return FALSE;
					}
					if(mfDateFilter == mfReferralDate){
						//(e.lally 2009-09-25) PLID 35654 - Different report when running by referral date
						nReportID = 681;
					}
					else{
						//(e.lally 2009-09-24) PLID 35053 - Depreciated and recreated under new ID
						nReportID = 680;
					}
				break;

				case NUMPatByRefSour:
					nReportID = 505;
				break;

				case REFNoShowByReferral:
					nReportID = 506;
				break;

				case REFInqtoCons:
					nReportID = 507;
					bEnableAll = FALSE;
				break;

				case REFProsToCons:
					nReportID = 508;
				break;

				case REFSchedVClosed:
					nReportID = 509;
				break;

			}

			CMarketReferralsPrintSetupDlg  dlgPrint(this, pInfo->m_bPreview, bEnableAll);

			if (dlgPrint.DoModal() != IDOK) {
				ClearRPIParameterList(&paramList); // (a.walling 2007-04-03 17:55) - PLID 25446
				return FALSE;
			}

			if (GetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>", TRUE)) {

				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nReportID)]);

				infReport.nDetail = 1;

				infReport.nProvider = nProvID;
				CRParameterInfo *pRefType = NULL;
				CRParameterInfo *pRefPatientID = NULL;
				CRParameterInfo *pRefPhysID = NULL;

				if(mktType == CNVConsToSurgByRefSour){

					//(e.lally 2009-09-24) PLID 35053 - Generate special filters for the report
					CStringArray saryFilters;
					BuildDocbarFilterArray(saryFilters, mktType, pCon, strPatientTempTable);
					//(e.lally 2009-09-28) PLID 35594 - Added back in support for referal source filtering. Right now this is a
						//locally defined filter. We also have two options, one for using the primary referral source, and one for using
						//all referral sources on a patient's account. We need to send in both, even though one or both could be empty.
					CString strPrimaryReferralFilter = ((CMarketReferralSourceDlg*)ptrCurrentTab)->GetConsToProcPrimaryReferralFilter();
					CString strMultiReferralFilter = ((CMarketReferralSourceDlg*)ptrCurrentTab)->GetConsToProcMultiReferralFilter();
					//(e.lally 2009-12-03) PLID 35521 - Update their spot in the array.
					if(saryFilters.GetCount() >= 17){
						saryFilters[15] = strPrimaryReferralFilter;
						saryFilters[16] = strMultiReferralFilter;
					}

					for(int i=0, nCount = saryFilters.GetCount(); i<nCount; i++){
						infReport.AddExtraValue(saryFilters.GetAt(i));
					}

					pRefType = new CRParameterInfo;
					//Get the type of referral source to display (set in the print setup dlg above)
					long nRefType = GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>");
					//Send in the type of referral to show as a report param
					pRefType->m_Data = FormatString("%li", nRefType);
					pRefType->m_Name = (CString)"RefTypeDisplay";
					paramList.Add((void *)pRefType);

					//(e.lally 2010-03-12) PLID 37709 - Pass in the special linked referral IDs for referring physician and referring patient
					pRefPatientID = new CRParameterInfo;
					long nRefPatID = GetRemotePropertyInt("DefaultPatientReferral", -1, 0, "<None>", true);
					//Send in the type of referral to show as a report param
					pRefPatientID->m_Data = FormatString("%li", nRefPatID);
					pRefPatientID->m_Name = (CString)"RefPatientReferralID";
					paramList.Add((void *)pRefPatientID);

					pRefPhysID = new CRParameterInfo;
					long nRefPhysID = GetRemotePropertyInt("DefaultPhysicianReferral", -1, 0, "<None>", true);
					//Send in the type of referral to show as a report param
					pRefPhysID->m_Data = FormatString("%li", nRefPhysID);
					pRefPhysID->m_Name = (CString)"RefPhysReferralID";
					paramList.Add((void *)pRefPhysID);
				}
				else{

					CString strFilter1, strFilter2, strFilter3, strFilter4, strFilter5;

					((CMarketReferralSourceDlg*)ptrCurrentTab)->GetCurrentGraphFilters(strFilter1, strFilter2, strFilter3, strFilter4, strFilter5, pCon, strPatientTempTable);
					infReport.SetExtraValue(strFilter1);
					infReport.strExtraText = strFilter2;
					infReport.strFilterField = strFilter3;
					infReport.strExtraField = strFilter4;					
					// (j.jones 2012-06-19 10:16) - PLID 48481 - now we pass in the dates from/to, and strFilter5
					infReport.strExtendedSql = strFilter5;
					CString from, to;
					from = GetMainFrame()->m_pDocToolBar->GetFromDate();
					to = GetMainFrame()->m_pDocToolBar->GetToDate();

					COleDateTime dtFrom, dtTo;
					dtFrom.ParseDateTime(from);
					dtTo.ParseDateTime(to);

					//increment the to date
					dtTo += COleDateTimeSpan(1,0,0,0);
					to = FormatDateTimeForSql(dtTo, dtoDate);

					//if we aren't filtering by date, we *should* just make a huge date range,
					//but the graph appears to always filter costs by the date range no matter what
					//so the report needs to match the graph
					/*
					if(!GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
						dtFrom.SetDate(1800, 1, 1);
						dtTo.SetDate(5000,12,31);
					}
					*/
					infReport.DateFrom = dtFrom;
					infReport.DateTo = dtTo;
				}

				// (j.gruber 2009-06-26 16:47) - PLID 34718 - added referral filter description
				CRParameterInfo *pFilter = new CRParameterInfo;				
				if (GetRemotePropertyInt("MarketReferralPrintOption", 1, 0, "<None>") == 1) {
					pFilter->m_Data = ((CMarketReferralSourceDlg*)ptrCurrentTab)->GetReferralFilterDescription();
				}
				else {
					pFilter->m_Data = "";
				}
				pFilter->m_Name = (CString)"RefFilterDescription";
				paramList.Add((void *)pFilter);			

				//Made new function for running reports - JMM 5-28-04
				RunReport(&infReport, &paramList, pInfo->m_bPreview, (CWnd *)this, infReport.strReportName, pInfo);
				ClearRPIParameterList(&paramList);

				return FALSE;

			}
					
		}
		else if(ptrCurrentTab == &m_ProcedureSheet) {


			MarketGraphType mktType = (MarketGraphType)GetType();

			switch (mktType) {

				case EFFMonByProc:
					nReportID = 510;
				break;

				case PROCConsultToSurg:
					// (j.gruber 2010-10-22 13:08) - PLID 24764 - check if they've set the consults
					if (! IsConsToProcSetupValid()) {
						MessageBox("Please set up your consult and procedure configuration settings by clicking the ... button before running this graph");									
						ClearRPIParameterList(&paramList);
						return FALSE;
					}
					//(e.lally 2009-08-27) PLID 35330 - Recreted report under new ID.
					nReportID = 673;
				break;

				case PROCPatients:
					nReportID = 512;
				break;

				case NUMNoShowByProc:
					nReportID = 513;
				break;

				case CNVInqToConsByProc:
					nReportID = 514;
				break;

				case CNVProsToConsByProc:
					nReportID = 515;
				break;

				case PROCSchedVClosed:
					nReportID = 516;
				break;

				case EFFMonByCategory:
					nReportID = 517;
				break;

			}

			
			// (j.gruber 2010-10-22 13:20) - PLID 24764 - moved down
			CMarketPrintSetupDlg  dlgPrint(this, pInfo->m_bPreview);

			if (dlgPrint.DoModal() != IDOK) {
				ClearRPIParameterList(&paramList); // (a.walling 2007-04-03 17:55) - PLID 25446
				return FALSE;
			}

			if (GetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>", TRUE)) {

				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nReportID)]);

				infReport.nDetail = 1;

				if(mktType == PROCConsultToSurg){
					//(e.lally 2009-08-28) PLID 35330 - Generate special filters for the report
					CStringArray saryFilters;
					BuildDocbarFilterArray(saryFilters, mktType, pCon, strPatientTempTable);
					//(e.lally 2009-09-11) PLID 35521 - Added back in support for procedure filtering. Right now this is a
						//locally defined filter, but eventually should be a shared ability.
					CString strProcedureFilter = ((CMarketProcedureDlg*)ptrCurrentTab)->GetConsToProcProcedureFilter();
					//(e.lally 2009-12-03) PLID 35521 - Update its spot in the array.
					if(saryFilters.GetCount() >= 13){
						saryFilters[12] = strProcedureFilter;
					}

					for(int i=0, nCount = saryFilters.GetCount(); i<nCount; i++){
						infReport.AddExtraValue(saryFilters.GetAt(i));
					}
				}
				else{
					infReport.SetExtraValue(((CMarketProcedureDlg*)ptrCurrentTab)->GetCurrentGraphFilters(pCon, strPatientTempTable));
				}

				// (j.gruber 2009-06-26 13:03) - PLID 34719 -  get the filter description
				if (mktType != EFFMonByCategory) {
					CRParameterInfo *pFilter = new CRParameterInfo;				
					pFilter->m_Data = ((CMarketProcedureDlg*)ptrCurrentTab)->GetProcedureFilterDescription();
					pFilter->m_Name = (CString)"ProcFilterDescription";
					paramList.Add((void *)pFilter);			
				}

				//Made new function for running reports - JMM 5-28-04
				RunReport(&infReport, &paramList, pInfo->m_bPreview, (CWnd *)this, infReport.strReportName, pInfo);
				ClearRPIParameterList(&paramList);

				return FALSE;

			}
			
			
		}
		else if(ptrCurrentTab == &m_PatCoordSheet) {
			

			MarketGraphType mktType = (MarketGraphType)GetType();

			switch (mktType) {

				case EFFMonByPatCoord:
					nReportID = 518;
				break;

				case CNVConsToSurgByPatCoord:
					// (j.gruber 2010-10-22 13:08) - PLID 24764 - check if they've set the consults
					if (! IsConsToProcSetupValid()) {
						MessageBox("Please set up your consult and procedure configuration settings by clicking the ... button before running this graph");			
						ClearRPIParameterList(&paramList);
						return FALSE;
					}
					//(e.lally 2009-08-28) PLID 35331 - Recreted report under new ID.
					nReportID = 676;
				break;

				case COORDPatients:
					nReportID = 520;
				break;

				case NUMCanByPatCoord:
					nReportID = 521;
				break;

				case CNVInqToConsByStaff:
					nReportID = 522;
				break;

				case COORDProsToCons:
					nReportID = 523;
				break;

				case COORDSchedVClosed:
					nReportID = 524;
				break;


			}

			// (j.gruber 2010-10-22 13:20) - PLID 24764 - moved down
			CMarketPrintSetupDlg  dlgPrint(this, pInfo->m_bPreview);

			if (dlgPrint.DoModal() != IDOK) {
				ClearRPIParameterList(&paramList); // (a.walling 2007-04-03 17:55) - PLID 25446
				return FALSE;
			}

			if (GetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>", TRUE)) {

				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nReportID)]);

				infReport.nDetail = 1;
				
				if(mktType == CNVConsToSurgByPatCoord){
					//(e.lally 2009-08-28) PLID 35331 - Generate special filters for the report
					CStringArray saryFilters;
					BuildDocbarFilterArray(saryFilters, mktType, pCon, strPatientTempTable);
					for(int i=0, nCount = saryFilters.GetCount(); i<nCount; i++){
						infReport.AddExtraValue(saryFilters.GetAt(i));
					}
				}
				else {
					CString strFilter1, strFilter2;
					((CMarketPatCoordDlg*)ptrCurrentTab)->GetCurrentGraphFilters(strFilter1, strFilter2, pCon, strPatientTempTable);
					infReport.SetExtraValue(strFilter1);
					infReport.strExtraText = strFilter2;
				}

				//Made new function for running reports - JMM 5-28-04
				RunReport(&infReport, &paramList, pInfo->m_bPreview, (CWnd *)this, infReport.strReportName, pInfo);
				ClearRPIParameterList(&paramList);
				
				return FALSE;

			}
		}
		else if(ptrCurrentTab == &m_DateSheet) {
			

			MarketGraphType mktType = (MarketGraphType)GetType();

			CString strGroupFilter;

			switch (mktType) {

				case DATERevByDate:
					nReportID = 525;
				break;

				case CNVConstoSurgByDate: 
				{
					// (j.gruber 2010-10-22 13:08) - PLID 24764 - check if they've set the consults
					if (! IsConsToProcSetupValid()) {
						MessageBox("Please set up your consult and procedure configuration settings by clicking the ... button before running this graph");			
						ClearRPIParameterList(&paramList);
						return FALSE;
					}
					if (GetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>", TRUE)) {

						//this is a special case
						//(e.lally 2009-09-08) PLID 35332 - Recreated under a new report ID
						CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(677)]);
				
						infReport.nDetail = 1;
						//(e.lally 2009-09-08) PLID 35332 - Generate special filters for the report
						CStringArray saryFilters;
						BuildDocbarFilterArray(saryFilters, mktType, pCon, strPatientTempTable);
						//(e.lally 2009-09-11) PLID 35521 - Added back in support for procedure filtering. Right now this is a
							//locally defined filter, but eventually should be a shared ability.
						CString strProcedureFilter = ((CMarketDateDlg*)ptrCurrentTab)->GetConsToProcProcedureFilter();
						//(e.lally 2009-12-03) PLID 35521 - Update its spot in the array.
						if(saryFilters.GetCount() >= 13){
							saryFilters[12] = strProcedureFilter;
						}

						for(int i=0, nCount = saryFilters.GetCount(); i<nCount; i++){
							infReport.AddExtraValue(saryFilters.GetAt(i));
						}

						//(e.lally 2009-09-11) PLID 35521 - Added support for the procedure filtering description. 
							//Right now this is a locally defined filter, but eventually should be a shared ability.
						if (mktType != EFFMonByCategory) {
							CRParameterInfo *pFilter = new CRParameterInfo;				
							pFilter->m_Data = ((CMarketDateDlg*)ptrCurrentTab)->GetPurposeFilterString();
							pFilter->m_Name = (CString)"ProcFilterDescription";
							paramList.Add((void *)pFilter);			
						}

						//Made new function for running reports - JMM 5-28-04
						RunReport(&infReport, &paramList, pInfo->m_bPreview, (CWnd *)this, infReport.strReportName, pInfo);
						ClearRPIParameterList(&paramList);

						return FALSE;

					}
				}

				break;

				case DATEPatients:
					nReportID = 527;
				break;

				case DATENoShowCancel:
					nReportID = 528;
				break;

				case DATEInqToCons:
					nReportID = 529;
				break;

				case DATEProsToCons:
					nReportID = 530;
				break;

				case DATECloseByProc:
					nReportID = 531;
				break;
				// (j.gruber 2011-05-06 10:56) - PLID 43584 - report for appts to charges
				case DATEApptToCharge:
					{
						nReportID = 707;
						strGroupFilter = ((CMarketDateDlg*)ptrCurrentTab)->GetConvGroupFilter();

						CRParameterInfo *pFilter = new CRParameterInfo;				
						pFilter->m_Data = ((CMarketDateDlg*)ptrCurrentTab)->GetConvGroupFilterString();
						pFilter->m_Name = (CString)"ConvGroupFilterDescription";
						paramList.Add((void *)pFilter);		

					}
				break;


			}

			// (j.gruber 2010-10-22 13:21) - PLID 24764 - moved down
			CMarketPrintSetupDlg  dlgPrint(this, pInfo->m_bPreview);

			if (dlgPrint.DoModal() != IDOK) {
				ClearRPIParameterList(&paramList); // (a.walling 2007-04-03 17:55) - PLID 25446
				return FALSE;
			}

			if (GetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>", TRUE)) {

				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nReportID)]);

				infReport.nDetail = 1;

				CString strFilter1 = "", strFilter2 = "", strFilter3 = "",strFilter4 = "";

				((CMarketDateDlg*)ptrCurrentTab)->GetCurrentGraphFilters(strFilter1, strFilter2, strFilter3, strFilter4, pCon, strPatientTempTable);

				infReport.SetExtraValue(strFilter1);
				infReport.strExtraText = strFilter2;
				infReport.strExtraField = strFilter3;
				infReport.strFilterField = strFilter4;

				// (j.gruber 2011-05-06 10:56) - PLID 43584
				if (nReportID == 707) {
					//appt to charge graph, we need to add the group filter
					infReport.strDateCaption = strGroupFilter;
				}				

				//Made new function for running reports - JMM 5-28-04
				RunReport(&infReport, &paramList, pInfo->m_bPreview, (CWnd *)this, infReport.strReportName, pInfo);
				ClearRPIParameterList(&paramList);

				return FALSE;

			}
		}
		else if(ptrCurrentTab == &m_OtherSheet) {

			CMarketPrintSetupDlg  dlgPrint(this, pInfo->m_bPreview);

			if (dlgPrint.DoModal() != IDOK) {
				ClearRPIParameterList(&paramList); // (a.walling 2007-04-03 17:55) - PLID 25446
				return FALSE;
			}

			
			MarketGraphType mktType = (MarketGraphType)GetType();

			switch (mktType) {

				case NUMNoShowByReason:
					nReportID = 533;
				break;

				case NUMCanByReason:
					nReportID = 532;
				break;
			}

			if (GetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>", TRUE)) {

				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nReportID)]);

				infReport.nDetail = 1;

				infReport.SetExtraValue(((CMarketOtherDlg*)ptrCurrentTab)->GetCurrentGraphFilters(pCon, strPatientTempTable));

				//Made new function for running reports - JMM 5-28-04
				RunReport(&infReport, &paramList, pInfo->m_bPreview, (CWnd *)this, infReport.strReportName, pInfo);
				ClearRPIParameterList(&paramList);

				return FALSE;

			}
		}
		else if(ptrCurrentTab == &m_BaselineSheet) {
			CMarketPrintSetupDlg  dlgPrint(this, pInfo->m_bPreview);
			if (dlgPrint.DoModal() != IDOK) {
				ClearRPIParameterList(&paramList); // (a.walling 2007-04-03 17:54) - PLID 25446
				return FALSE;
			}

			if (GetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>", TRUE)) {

				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(534)]);

				infReport.nDetail = 1;

				CString strSql = ((CMarketBaselineDlg*)ptrCurrentTab)->GetReportSQL(pCon, strPatientTempTable);

				if (!strSql.IsEmpty()) {
					
					infReport.SetExtraValue(strSql);
					//Made new function for running reports - JMM 5-28-04
					RunReport(&infReport, &paramList, pInfo->m_bPreview, (CWnd *)this, infReport.strReportName, pInfo);
					ClearRPIParameterList(&paramList);
	
					return FALSE;
				}

				
			}
		}
		else if(ptrCurrentTab == &m_zipSheet) {
			
		}
		else if(ptrCurrentTab == &m_RetentionSheet) {

			if (!((CMarketRetentionGraphDlg*)ptrCurrentTab)->CheckBoxes()) {
				//there are no check boxes checked
				MsgBox("Please select at least one of the date range check boxes before printing this report");
				ClearRPIParameterList(&paramList); // (a.walling 2007-04-03 17:55) - PLID 25446
				return FALSE;
			}


			CMarketRetentionPrintSetupDlg  dlgPrint(this, pInfo->m_bPreview);
			if (dlgPrint.DoModal() != IDOK) {
				ClearRPIParameterList(&paramList); // (a.walling 2007-04-03 17:55) - PLID 25446
				return FALSE;
			}

			long nReportID;
			if (GetRemotePropertyInt("MarketPrintStyleOption", 1, 0, "<None>", TRUE)) {

				
				CString strPurposeIDs = "";
				BOOL bExcludeAppts = FALSE;
				CString strProcedureNames;

				if (GetRemotePropertyInt("MarketRetentionOption", 1, 0, "<None>")) {

					nReportID = 535;
					strPurposeIDs = dlgPrint.m_strMultiPurposeIds;
					bExcludeAppts = dlgPrint.m_bExcludeAppts;
					strProcedureNames = dlgPrint.m_strPurposeNames;
				}
				else {
					nReportID = 536;

					strPurposeIDs = dlgPrint.m_strMultiPurposeIds;
					bExcludeAppts = dlgPrint.m_bExcludeAppts;
					strProcedureNames = dlgPrint.m_strPurposeNames;
				}

				CString strExcludeAppts;
				COleDateTime dtDate = COleDateTime::GetCurrentTime();
				if (! bExcludeAppts) {
					strExcludeAppts.Format("Not Excluding Future Appointments");
				}
				else {					
					if (strPurposeIDs.IsEmpty() || strPurposeIDs == "-1") {
						strExcludeAppts.Format("Excluding All Appointments after %s ", FormatDateTimeForInterface(dtDate, NULL, dtoDate));
					}
					else {
						strExcludeAppts.Format("Excluding Appointments after %s with one of these Purposes: %s", 
							FormatDateTimeForInterface(dtDate, NULL, dtoDate), strProcedureNames);
					}
				}

				// (b.spivey, November 04, 2011) - PLID 46267 - 
				CString strExcludeUnretainedValue, strExcludeUnretainedFilter;
				((CMarketRetentionGraphDlg*)ptrCurrentTab)->m_nxeditExcludeUnretainedRange.GetWindowText(strExcludeUnretainedValue); 
				int nExcludeUnretainedRange = atoi(strExcludeUnretainedValue); 
				if(((CMarketRetentionGraphDlg*)ptrCurrentTab)->m_checkExcludeUnretained.GetCheck()){
					strExcludeUnretainedFilter.Format("Excluding unretained patients greater than %li months before the Pivot Date ", nExcludeUnretainedRange); 
				}
				else{
					strExcludeUnretainedFilter = ""; 
				}

				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(nReportID)]);
				
				infReport.SetExtraValue(((CMarketRetentionGraphDlg*)ptrCurrentTab)->GetPrintSql(bExcludeAppts, strPurposeIDs, pCon, strPatientTempTable));

				//PLID 15604 - added some info to the retention reports, via paramteters
				CRParameterInfo *tmpPInfo;

				//Pivot Date
				tmpPInfo = new CRParameterInfo;
				COleDateTime dtPivot = VarDateTime(((CMarketRetentionGraphDlg*)ptrCurrentTab)->m_dtPivot.GetValue());
				tmpPInfo->m_Data = "Pivot Date: " + FormatDateTimeForInterface(dtPivot, NULL, dtoDate);
				tmpPInfo->m_Name = (CString)"PivotDate";
				paramList.Add((void *)tmpPInfo);

				// (b.spivey, November 03, 2011) - PLID 46267 - Make sure to add it to the report. 
				//Exclude Unretained Patients Range 
				tmpPInfo = new CRParameterInfo; 
				tmpPInfo->m_Data = strExcludeUnretainedFilter; 
				tmpPInfo->m_Name = (CString)"ExcludeUnretained"; 
				paramList.Add((void *)tmpPInfo); 

				if (nReportID == 536) {
				
					//Unretained Title
					tmpPInfo = new CRParameterInfo;
					tmpPInfo->m_Data = ((CMarketRetentionGraphDlg*)ptrCurrentTab)->GetUnretainedTitle();
					tmpPInfo->m_Name = (CString)"UnretainedTitle";
					paramList.Add((void *)tmpPInfo);
				}

				//Location Filter
				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = GetMainFrame()->m_pDocToolBar->GetLocationFilterString();
				tmpPInfo->m_Name = (CString)"LocationFilterTitle";
				paramList.Add((void *)tmpPInfo);

				//Provider Filter
				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = GetMainFrame()->m_pDocToolBar->GetProviderFilterString();
				tmpPInfo->m_Name = (CString)"ProviderFilterTitle";
				paramList.Add((void *)tmpPInfo);

				//Patient Coordinator Filter
				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = GetMainFrame()->m_pDocToolBar->GetPatFilterFilterString();
				tmpPInfo->m_Name = (CString)"PatCoordFilterTitle";
				paramList.Add((void *)tmpPInfo);

				//Date Filter
				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = GetMainFrame()->m_pDocToolBar->GetDateFilterString();
				tmpPInfo->m_Name = (CString)"DateFilterTitle";
				paramList.Add((void *)tmpPInfo);

				//Use Tracking
				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = ((CMarketRetentionGraphDlg*)ptrCurrentTab)->GetUseTracking();
				tmpPInfo->m_Name = (CString)"UseTracking";
				paramList.Add((void *)tmpPInfo);

				//ExcludeAppts
				
				tmpPInfo = new CRParameterInfo;
				tmpPInfo->m_Data = strExcludeAppts;
				tmpPInfo->m_Name = (CString)"ExcludeAppts";
				paramList.Add((void *)tmpPInfo);



				//Made new function for running reports - JMM 5-28-04
				RunReport(&infReport, &paramList, pInfo->m_bPreview, (CWnd *)this, infReport.strReportName, pInfo);
				ClearRPIParameterList(&paramList);	//DRT - PLID 18085 - Cleanup after ourselves


				return FALSE;

			}
		}
		else if(ptrCurrentTab == &m_effectSheet) {
			// (a.walling 2006-08-08 10:51) - PLID 3897 - Replace screenshots with a report
			if (m_effectSheet.IsRequerying()) {
				AfxMessageBox("Please wait for the tab to finish loading before running this report.");
				ClearRPIParameterList(&paramList); // (a.walling 2007-04-03 17:55) - PLID 25446
				return FALSE;
			}

			CString strExpense, strReferrals, strRefCost, strPatients, strPatCost, strFeesBilled, strFeesBilledPercent, strFeesCollected, strFeesCollectedPercent;

			m_effectSheet.GetResults(strExpense, strReferrals, strRefCost, strPatients, strPatCost, strFeesBilled, strFeesBilledPercent, strFeesCollected, strFeesCollectedPercent);

			CString from, to;
			from = GetMainFrame()->m_pDocToolBar->GetFromDate();
			to = GetMainFrame()->m_pDocToolBar->GetToDate();

			COleDateTime dtFrom, dtTo;
			dtFrom.ParseDateTime(from);
			dtTo.ParseDateTime(to);

			from = FormatDateTimeForInterface(dtFrom, NULL, dtoDate);
			to = FormatDateTimeForInterface(dtTo, NULL, dtoDate);

			CString strReferralSource, strGroupBy;

			strReferralSource = m_effectSheet.GetReferralSource();
			strGroupBy = m_effectSheet.GetGroupBy();

			CString sql = m_effectSheet.GetReportSQL(pCon);
			
			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(573)]);
			infReport.SetExtraValue(sql);

			CRParameterInfo *p;
			p = new CRParameterInfo;
			p->m_Name = "DateFrom";
			if(GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
				p->m_Data = from;
			}
			else {
				p->m_Data = "01/01/1000"; // all dates
			}
			paramList.Add(p);
			p = new CRParameterInfo;
			p->m_Name = "DateTo";
			p->m_Data = to;
			paramList.Add(p);
			p = new CRParameterInfo;
			p->m_Name = "Expense";
			p->m_Data = strExpense;
			paramList.Add(p);
			p = new CRParameterInfo;
			p->m_Name = "Referrals";
			p->m_Data = strReferrals;
			paramList.Add(p);
			p = new CRParameterInfo;
			p->m_Name = "ReferralCost";
			p->m_Data = strRefCost;
			paramList.Add(p);
			p = new CRParameterInfo;
			p->m_Name = "Patients";
			p->m_Data = strPatients;
			paramList.Add(p);
			p = new CRParameterInfo;
			p->m_Name = "PatientCost";
			p->m_Data = strPatCost;
			paramList.Add(p);
			p = new CRParameterInfo;
			p->m_Name = "FeesBilled";
			p->m_Data = strFeesBilled;
			paramList.Add(p);
			p = new CRParameterInfo;
			p->m_Name = "FeesBilledPercent";
			p->m_Data = strFeesBilledPercent;
			paramList.Add(p);
			p = new CRParameterInfo;
			p->m_Name = "FeesCollected";
			p->m_Data = strFeesCollected;
			paramList.Add(p);
			p = new CRParameterInfo;
			p->m_Name = "FeesCollectedPercent";
			p->m_Data = strFeesCollectedPercent;
			paramList.Add(p);
			p = new CRParameterInfo;
			p->m_Name = "ReferralSource";
			p->m_Data = strReferralSource;
			paramList.Add(p);
			p = new CRParameterInfo;
			p->m_Name = "GroupedBy";
			p->m_Data = strGroupBy;
			paramList.Add(p);
			
			//Made new function for running reports - JMM 5-28-04
			RunReport(&infReport, &paramList, pInfo->m_bPreview, (CWnd *)this, infReport.strReportName, pInfo);
			ClearRPIParameterList(&paramList);

			return FALSE;
		}
		else if(ptrCurrentTab == &m_costSheet) {
			// (a.walling 2006-08-08 10:51) - PLID 3897 - Replace screenshots with a report
/*			CMarketPrintSetupDlg  dlgPrint(this, pInfo->m_bPreview);
			if (dlgPrint.DoModal() != IDOK) {
				return FALSE;
			}*/

			CRParameterInfo *paramFrom, *paramTo;

			paramFrom = new CRParameterInfo;
			paramFrom->m_Name = "DateFrom";
			paramTo = new CRParameterInfo;
			paramTo->m_Name = "DateTo";
			
			CString from, to, sql;
			from = GetMainFrame()->m_pDocToolBar->GetFromDate();
			to = GetMainFrame()->m_pDocToolBar->GetToDate();

			COleDateTime dtFrom, dtTo;
			dtFrom.ParseDateTime(from);
			dtTo.ParseDateTime(to);

			CString strLocation = GetMainFrame()->m_pDocToolBar->GetLocationString();
			CString strDateField = GetMainFrame()->m_pDocToolBar->GetFilterField(mftDate);
			if(GetMainFrame()->m_pDocToolBar->UseFilter(mftDate)) {
				paramFrom->m_Data = FormatDateTimeForInterface(dtFrom, NULL, dtoDate);
				paramTo->m_Data = FormatDateTimeForInterface(dtTo, NULL, dtoDate);

				if (strDateField.CompareNoCase("MarketingCostsT.DatePaid") == 0) {
					if(strLocation.IsEmpty()) {
						sql.Format("MarketingCostsT.%s >= '%s' AND MarketingCostsT.%s < DATEADD(day,1,'%s')", strDateField, from, strDateField, to);
					}
					else {
						sql.Format("MarketingCostsT.%s >= '%s' AND MarketingCostsT.%s < DATEADD(day,1,'%s') AND MarketingCostsT.LocationID IN %s", strDateField, from, strDateField, to, strLocation);
					}
				}
				else {
					//its effectiveness date so we need to set the filter differently
					if(strLocation.IsEmpty()) {
						sql.Format(" (MarketingCostsT.EffectiveFrom <= '%s' AND MarketingCostsT.EffectiveTo >= '%s')", to, from);
					}
					else {
						sql.Format(" (MarketingCostsT.EffectiveFrom <= '%s' AND MarketingCostsT.EffectiveTo >= '%s') AND MarketingCostsT.LocationID IN %s", to, from, strLocation);
					}
				}
			}
			else {
				paramFrom->m_Data = "";
				paramTo->m_Data = "12/31/5000";
				if(!strLocation.IsEmpty())
					sql.Format("MarketingCostsT.LocationID IN %s", strLocation);
			}

			paramList.Add(paramFrom);
			paramList.Add(paramTo);

			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(402)]);
			infReport.SetExtraValue(sql);
			
			//Made new function for running reports - JMM 5-28-04
			RunReport(&infReport, &paramList, pInfo->m_bPreview, (CWnd *)this, infReport.strReportName, pInfo);
			ClearRPIParameterList(&paramList);

			return FALSE;
		}

		ClearRPIParameterList(&paramList); // (a.walling 2007-04-03 17:55) - PLID 25446

		//********************set the position to be landscape
		ptrCurrentTab->OnBeginPrinting(NULL,pInfo);

		// Set the paper orientation
		ASSERT(pInfo && pInfo->m_pPD);
		if (pInfo && pInfo->m_pPD) {

			// First make sure we have the devmode and devnames objects as copies of the app's settings
			if (pInfo->m_pPD->m_pd.hDevMode == NULL || pInfo->m_pPD->m_pd.hDevNames == NULL) {
				// Get a copy of the app's device settings, we will use these to set the report's device settings after making our adjustments
				DEVMODE *pFinDevMode = NULL;
				LPTSTR strPrinter = NULL, strDriver = NULL, strPort = NULL;
				AllocCopyOfAppDeviceSettings(&pFinDevMode, &strPrinter, &strDriver, &strPort);
				
				// dev MODE
				if (pInfo->m_pPD->m_pd.hDevMode == NULL) {
					// The devmode object isn't set, so set it to a copy of the app's
					pInfo->m_pPD->m_pd.hDevMode = AllocDevModeCopy(pFinDevMode);
				}
				// dev NAMES
				if (pInfo->m_pPD->m_pd.hDevNames == NULL) {
					// The devnames object isn't set, so set it to a copy of the app's
					pInfo->m_pPD->m_pd.hDevNames = AllocDevNamesCopy(strPrinter, strDriver, strPort);
				}

				// We're done with our copies because we've made a second set of copies and stored them in the pInfo object
				FreeCopyOfDeviceSettings(&pFinDevMode, &strPrinter, &strDriver, &strPort);
			}

			// Then set the page orientation correctly
			DEVMODE *pInfoDevMode = (DEVMODE *)GlobalLock(pInfo->m_pPD->m_pd.hDevMode);
			ASSERT(pInfoDevMode);
			pInfoDevMode->dmOrientation = DMORIENT_LANDSCAPE;
			GlobalUnlock(pInfo->m_pPD->m_pd.hDevMode);
		} else {
			int nResult = AfxMessageBox(
				"Your page orientation selection could not be applied.  If you "
				"continue, the current default orientation will be used.", 
				MB_OKCANCEL|MB_ICONEXCLAMATION);
			if (nResult != IDOK) {
				// They chose not to continue
				return CNxTabView::OnPreparePrinting(pInfo);
			}
		}

		// Get the current handles
		extern CPracticeApp theApp;
		HANDLE hAppOldDevMode = theApp.GetDevModeHandle();
		HANDLE hAppOldDevNames = theApp.GetDevNamesHandle();
		
		// Set the new handles (if the handles aren't set, just leave the app's handles as is)
		if (pInfo->m_pPD->m_pd.hDevMode) theApp.SetDevModeHandle(pInfo->m_pPD->m_pd.hDevMode);
		if (pInfo->m_pPD->m_pd.hDevNames) theApp.SetDevNamesHandle(pInfo->m_pPD->m_pd.hDevNames);
		
		// Do the standard prepare printing (which uses the CWinApp's handles)
		BOOL bPrepared = CNxTabView::DoPreparePrinting(pInfo);

		// Return the handles to their original values
		theApp.SetDevModeHandle(hAppOldDevMode);
		theApp.SetDevNamesHandle(hAppOldDevNames);

		//TES 6/2/2004: Don't keep printing if they cancelled!
		if(!bPrepared) {
			return FALSE;
		}
		else {
			return TRUE;
		}

		//******************8end setting position
	}NxCatchAllCall("Error Printing", return FALSE);
	
	//return CNxTabView::OnPreparePrinting(pInfo);
}


void CMarketView::OnPrint(CDC *pDC, CPrintInfo *pInfo)  {
	
	CNxDialog* ptrCurrentTab = GetActiveSheet();
	ptrCurrentTab->Print(pDC, pInfo);


}

LRESULT CMarketView::OnMarketReady(WPARAM wParam, LPARAM lParam)
{
	// (c.haag 2007-03-15 17:00) - PLID 24253 - This message is handled when a filter
	// changes. Here we force every sheet with a "Reload" button to reset their graphs.
	//(a.wilson 2011-10-7) PLID 38789 - changed force refresh to prevent them all from clearing when switching tabs.
	//(a.wilson 2011-11-18) PLID 38789 - updated so that we know which filter has changed.
	bool bForceReset = false;

	//(a.wilson 2011-11-21) PLID 38789 - basically if it isn't the retention tab...then its a filter change. 
	//In that case force reset graphs.
	if (wParam != -1) { bForceReset = true; }

	if (IsWindow(m_PatCoordSheet.GetSafeHwnd())) m_PatCoordSheet.ResetGraph(true, "", bForceReset);
	if (IsWindow(m_DateSheet.GetSafeHwnd())) m_DateSheet.ResetGraph(true, "", bForceReset);
	if (IsWindow(m_RefSourSheet.GetSafeHwnd())) m_RefSourSheet.ResetGraph(true, "", bForceReset);
	if (IsWindow(m_ProcedureSheet.GetSafeHwnd())) m_ProcedureSheet.ResetGraph(true, "", bForceReset);
	if (IsWindow(m_BaselineSheet.GetSafeHwnd())) m_BaselineSheet.ResetGraph(true, "", bForceReset);
	if (IsWindow(m_OtherSheet.GetSafeHwnd())) m_OtherSheet.ResetGraph(true, "", bForceReset);
	if (IsWindow(m_RetentionSheet.GetSafeHwnd())) m_RetentionSheet.ResetGraph(true, "", bForceReset);
	if (IsWindow(m_zipSheet.GetSafeHwnd())) m_zipSheet.ResetGraph(true, "", bForceReset);

	// (c.haag 2007-03-15 17:03) - Now percolate the message down to the sheet
	if (GetActiveSheet()) {
		GetActiveSheet()->PostMessage(NXM_MARKET_READY, wParam, lParam);
	}

	return 0;
}
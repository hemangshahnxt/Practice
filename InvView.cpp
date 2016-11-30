// InvView.cpp : implementation file
//

#include "stdafx.h"
#include "InvView.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "Barcode.h"
#include "GlobalReportUtils.h"
#include "InternationalUtils.h"
#include "InvUtils.h"
#include "boost/bind.hpp"
#include "InvEditDlg.h"
#include "InvOrderDlg.h"
#include "InvAllocationsDlg.h"	// (j.jones 2007-11-06 11:22) - PLID 28003 - added Allocation tab
#include "InvOverviewDlg.h"		// (j.jones 2007-11-06 12:11) - PLID 27989 - addded Overview tab
#include "InvReportsDlg.h"		// (c.haag 2009-01-12 15:21) - PLID 32683 - Added Reports tab
#include "InvFramesDlg.h"
#include "InvVisionWebDlg.h" // (s.dhole 2010-09-15 16:45) - PLID 
#include "InvInternalManagementDlg.h"  // r.wilson (2011-4-8) - PLID 43188

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInvView

IMPLEMENT_DYNCREATE(CInvView, CNxTabView)

// (j.jones 2013-05-07 12:02) - PLID 53969 - changed the child tabs to be declared by reference,
// which means their initialization is slightly different now
CInvView::CInvView()
	: m_EditorSheet(*(new CInvEditDlg(this)))
	, m_OrderSheet(*(new CInvOrderDlg(this)))
	, m_AllocationSheet(*(new CInvAllocationsDlg(this)))
	, m_OverviewSheet(*(new CInvOverviewDlg(this)))
	, m_ReportsSheet(*(new CInvReportsDlg(this)))
	, m_FramesSheet(*(new CInvFramesDlg(this)))
	, m_GlassesOrderSheet(*(new CInvVisionWebDlg(this)))
	, m_InventoryManagementSheet(*(new CInvInternalManagementDlg(this)))
{
}

CInvView::~CInvView()
{
	try {

		// (j.jones 2013-05-07 12:02) - PLID 53969 - changed the child tabs to be declared by reference,
		// which means we have to clean up their pointers now
		// They are never null. They have to have been filled in the constructor.
		delete &m_EditorSheet;
		delete &m_OrderSheet;
		delete &m_AllocationSheet;
		delete &m_OverviewSheet;
		delete &m_ReportsSheet;
		delete &m_FramesSheet;
		delete &m_GlassesOrderSheet;
		delete &m_InventoryManagementSheet;

	}NxCatchAll(__FUNCTION__);
}

BEGIN_MESSAGE_MAP(CInvView, CNxTabView)
	//{{AFX_MSG_MAP(CInvView)
	ON_WM_CREATE()

	ON_COMMAND(ID_FILE_PRINT, CNxTabView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CNxTabView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CNxTabView::OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
	ON_UPDATE_COMMAND_UI(IDM_UPDATE_VIEW, OnUpdateViewUI)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvView diagnostics

#ifdef _DEBUG
void CInvView::AssertValid() const
{
	CNxTabView::AssertValid();
}

void CInvView::Dump(CDumpContext& dc) const
{
	CNxTabView::Dump(dc);
}
#endif //_DEBUG

//////////////////////////////////////

// (a.walling 2006-09-14 13:28) - PLID 3897 - Disable print preview for these tabs
void CInvView::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
{
	switch(GetActiveTab()) {
		case InventoryModule::ItemTab:
		case InventoryModule::AllocationTab:	// (j.jones 2007-11-06 12:25) - PLID 28003 - support this for Allocation, until we have an appropriate report
			// (a.walling 2006-10-16 15:56) - PLID 3897 - Re-enable screenshot print previews
			pCmdUI->Enable(TRUE);
			break;
		case InventoryModule::GlassesOrderTab:// (s.dhole 2011-01-21 18:13) - PLID 40538 - Glasses 
		case InventoryModule::FramesTab: // (j.gruber 2011-06-17 13:45) - PLID 42204
		case InventoryModule::ReportsTab: // (j.gruber 2011-06-17 13:45) - PLID 42204		
			pCmdUI->Enable(FALSE);
			break;
		default:	
			pCmdUI->Enable();
			break;
	}
}

// (a.walling 2006-09-14 13:28) - PLID 3897 - Disable print for these tabs
void CInvView::OnUpdateFilePrint(CCmdUI* pCmdUI)
{
	switch(GetActiveTab()) {
		case InventoryModule::ItemTab:
		case InventoryModule::AllocationTab:	// (j.jones 2007-11-06 12:25) - PLID 28003 - support this for Allocation, until we have an appropriate report
			// (a.walling 2006-10-16 15:56) - PLID 3897 - Re-enable screenshot print previews
			pCmdUI->Enable(TRUE);
			break;

		case InventoryModule::GlassesOrderTab:// (s.dhole 2011-01-21 18:13) - PLID 40538 - Glasses 
		case InventoryModule::FramesTab: // (j.gruber 2011-06-17 13:45) - PLID 42204
		case InventoryModule::ReportsTab: // (j.gruber 2011-06-17 13:45) - PLID 42204
			pCmdUI->Enable(FALSE);
			break;
		default:	
			pCmdUI->Enable();
			break;
	}
}

// (z.manning 2010-07-16 17:26) - PLID 39222
void CInvView::OnUpdateViewUI(CCmdUI *pCmdUI)
{
	try
	{
		switch(GetActiveTab())
		{
			// (z.manning 2010-07-16 17:26) - PLID 39222 - No refresh on the Frames tab
			case InventoryModule::FramesTab:
				pCmdUI->Enable(FALSE);
				break;

			default:
				pCmdUI->Enable(TRUE);
				break;
		}
	}NxCatchAll(__FUNCTION__);
}

BOOL CInvView::CheckPermissions()
{
	if(!g_pLicense->CheckForLicense(CLicense::lcInv, CLicense::cflrUse))
		return FALSE;

	if (!UserPermission(InventoryModuleItem)) {
		return FALSE;
	}
	return TRUE;
}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CInvView::Hotkey (int key)
{
	if(IsWindowEnabled()) {
		int nKey = IgnoreRightAlt() ? VK_LMENU : VK_MENU;
		if (GetAsyncKeyState(nKey) & 0xE000)
		{	short CurrentTab = -1;
			switch (key)
			{	case 'P':
					CurrentTab = InventoryModule::ItemTab;
					break;
				case 'O':
					CurrentTab = InventoryModule::OrderTab;
					break;
				// (j.jones 2007-11-06 11:23) - PLID 28003 - added Allocation tab
				case 'L':
					CurrentTab = InventoryModule::AllocationTab;
					break;
				// (j.jones 2007-11-06 12:11) - PLID 27989 - added Overview tab
				case 'E':
					CurrentTab = InventoryModule::OverviewTab;
					break;
				case 'R':
					CurrentTab = InventoryModule::ReportsTab;
					break;
				case 'S':
					CurrentTab = InventoryModule::FramesTab;
					break;
				case 'G': 
					// (s.dhole 2010-09-15 16:18) - PLID 40538
					CurrentTab = InventoryModule::GlassesOrderTab;
					break;
				case 'N':
					//r.wilson 2010-06-08 - PLID 43188
					if(IsNexTechInternal()) {
						CurrentTab = InventoryModule::InventoryManagementTab;
					}
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

	return CNxView::Hotkey (key);
}

///////////////////////////////////////
// CInvView message handlers


int CInvView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if(CNxTabView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// (j.jones 2007-11-13 13:11) - PLID 28004 - create all the sheets, ShowTabs will hide them later

	Modules::Tabs& tabs = g_Modules[Modules::Inventory]->Reset().GetTabs();

	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to create the tabs

	CreateSheet(&m_EditorSheet, tabs, InventoryModule::ItemTab);
	CreateSheet(&m_OrderSheet, tabs, InventoryModule::OrderTab);
	// (j.jones 2007-11-06 11:24) - PLID 28003 - added Allocation tab
	CreateSheet(&m_AllocationSheet, tabs, InventoryModule::AllocationTab);
	// (j.jones 2007-11-06 12:11) - PLID 27989 - added Overview tab
	CreateSheet(&m_OverviewSheet, tabs, InventoryModule::OverviewTab);
	// (c.haag 2009-01-12 15:20) - PLID 32683 - Added Reports tab
	CreateSheet(&m_ReportsSheet, tabs, InventoryModule::ReportsTab);
	// (z.manning 2010-06-17 16:31) - PLID 39222
	CreateSheet(&m_FramesSheet, tabs, InventoryModule::FramesTab);
	// (s.dhole 2010-09-15 16:18) - PLID 40538
	CreateSheet(&m_GlassesOrderSheet, tabs, InventoryModule::GlassesOrderTab);
	 // r.wilson (2011-4-8) - PLID 43188 - Create new tab
	 // d.singleton (2011-5-20) - PLID 43188 - Made sure it doesnt show outside of internal
	if(IsNexTechInternal())
	{
		CreateSheet(&m_InventoryManagementSheet,tabs,InventoryModule::InventoryManagementTab);
	}
	//DRT 4/18/2008 - PLID 29711 - Set the text color of the tabs.
	// (j.jones 2016-04-18 17:46) - NX-100214 - the NxTab control now defaults
	// to the value of CNexTechDialog::GetDefaultNxTabTextColor()

	//(r.wilson 5/11/2012) PLID 43741 - This clear all status values from my psuedo cache if the inventory module gets closed and then opened again .. essentially it reloads the the values
	if(GetMainFrame()->m_mapGlassesOrderStatus.IsEmpty() != TRUE)
	{
		GetMainFrame()->m_mapGlassesOrderStatus.RemoveAll();
	}

	ShowTabs();

	return 0;
}

// (d.thompson 2009-01-29) - PLID 32894 - We need a 1-time warning about the C&A module
static bool g_bConsignAllocWarningGiven = false;

void CInvView::OnSelectTab(short newTab, short oldTab)
{
	if (newTab == InventoryModule::ItemTab)
	{
		// (j.jones 2007-11-12 15:12) - PLID 28074 - removed the unreliable m_tab->Size concept
		// and made it so if we try to select the InventoryModule::ItemTab, and don't have permissions,
		// select the InventoryModule::OrderTab		
		if(oldTab == InventoryModule::ItemTab) {
			//check for the read permission
			if(!(GetCurrentUserPermissions(bioInvItem) & SPT__R________)){
				OnSelectTab(InventoryModule::OrderTab, InventoryModule::OrderTab);
				m_tab->CurSel = InventoryModule::OrderTab;
				return;
			}
		}
		else if (!CheckCurrentUserPermissions(bioInvItem, sptRead))
		{	
			SetActiveTab(oldTab);
			return;
		}
	}
	else if(newTab == InventoryModule::OrderTab)
	{
		//Do nothing
	}
	// (j.jones 2007-11-06 11:26) - PLID 28003 - added Allocation tab
	else if(newTab == InventoryModule::AllocationTab)
	{
		// (j.jones 2007-11-12 15:08) - PLID 28074 - added allocation tab permissions
		// (a.walling 2008-02-27 09:08) - PLID 28946 - Check licensing
		if(oldTab == InventoryModule::AllocationTab) {
			//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
			// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
			if(!(GetCurrentUserPermissions(bioInventoryAllocation) & SPT__R________) || !g_pLicense->HasCandAModule(CLicense::cflrUse)) {
				OnSelectTab(InventoryModule::ItemTab, InventoryModule::ItemTab);
				m_tab->CurSel = InventoryModule::ItemTab;
				return;
			}
		}
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(!CheckCurrentUserPermissions(bioInventoryAllocation, sptRead)  || !g_pLicense->HasCandAModule(CLicense::cflrUse))
		{
			SetActiveTab(oldTab);
			return;
		}		
	}
	// (j.jones 2007-11-06 12:14) - PLID 27989 - added Overview tab
	else if(newTab == InventoryModule::OverviewTab)
	{
		// (a.walling 2008-02-27 09:09) - PLID 28946 - Check licensing
		if(oldTab == InventoryModule::OverviewTab) {
			//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
			// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
			if(!(GetCurrentUserPermissions(bioInventoryOverview) & SPT__R________) || !g_pLicense->HasCandAModule(CLicense::cflrUse)) {
				OnSelectTab(InventoryModule::ItemTab, InventoryModule::ItemTab);
				m_tab->CurSel = InventoryModule::ItemTab;
				return;
			}
		}
		//DRT - PLID 30117 - Use BetaHasAdvInventory() for now.  Remove this when beta period is over.
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(!CheckCurrentUserPermissions(bioInventoryOverview, sptRead) || !g_pLicense->HasCandAModule(CLicense::cflrUse))
		{
			SetActiveTab(oldTab);
			return;
		}
	}
	// (c.haag 2009-01-13 12:06) - PLID 32683 - Added Reports tab
	else if(newTab == InventoryModule::ReportsTab) {
		if(oldTab == InventoryModule::ReportsTab) {
			// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
			if(!(GetCurrentUserPermissions(bioInventoryModuleReportsTab) & SPT__R________) || !g_pLicense->HasCandAModule(CLicense::cflrUse)) {
				OnSelectTab(InventoryModule::ItemTab, InventoryModule::ItemTab);
				m_tab->CurSel = InventoryModule::ItemTab;
				return;
			}
		}
		// (d.thompson 2009-01-27) - PLID 32859 - Replaced beta with C&A module
		if(!CheckCurrentUserPermissions(bioInventoryModuleReportsTab, sptRead) || !g_pLicense->HasCandAModule(CLicense::cflrUse))
		{
			SetActiveTab(oldTab);
			return;
		}
	}
	// (z.manning 2010-06-17 15:50) - PLID 39222
	else if(newTab == InventoryModule::FramesTab) {
		// (c.haag 2010-06-30 11:25) - PLID 39424 - License checking
		if(oldTab == InventoryModule::FramesTab) {
			if(!g_pLicense->CheckForLicense(CLicense::lcFrames, CLicense::cflrUse)) {
				OnSelectTab(InventoryModule::ItemTab, InventoryModule::ItemTab);
				m_tab->CurSel = InventoryModule::ItemTab;
				return;
			}
		}
		if(!g_pLicense->CheckForLicense(CLicense::lcFrames, CLicense::cflrUse))
		{
			SetActiveTab(oldTab);
			return;
		}
	}

	//TES 12/9/2010 - PLID 41701 - License checking
	else if(newTab == InventoryModule::GlassesOrderTab ) {
		
		if(oldTab == InventoryModule::GlassesOrderTab) {
			if(!g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrUse)) {
				OnSelectTab(InventoryModule::ItemTab, InventoryModule::ItemTab);
				m_tab->CurSel = InventoryModule::ItemTab;
				return;
			}
		}
		if(!g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrUse))
		{
			SetActiveTab(oldTab);
			return;
		}
		
	}

	// (d.thompson 2009-01-29) - PLID 32894 - When a user attempts to open the allocation or overview or report
	//	tabs, make sure they are warned once per Practice session about the module.
	//I am turning this off in debug mode, as it will drive developers completely insane.
#ifndef _DEBUG
	if((newTab == InventoryModule::OverviewTab || newTab == InventoryModule::AllocationTab || newTab == InventoryModule::ReportsTab) && !g_bConsignAllocWarningGiven) {
		g_bConsignAllocWarningGiven = true;
		AfxMessageBox("The Consignment and Allocation module is a proprietary NexTech product, US Patent Pending.");
	}
#endif

	CNxTabView::OnSelectTab(newTab, oldTab);
}

BOOL CInvView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	short tab = GetActiveTab();

	switch (tab)
	{
		// (j.jones 2007-11-06 11:26) - PLID 28003 - supported the Allocation tab, just an image preview for now
		// (j.jones 2007-11-13 13:15) - PLID 28004 - fixed so we used proper ordinals
		case InventoryModule::ItemTab:
		{
			return CNxTabView::OnPreparePrinting(pInfo);
		}
		break;

		// (c.haag 2008-01-07 15:45) - PLID 28084 - We now have an allocation list report
		case InventoryModule::AllocationTab:
		{
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(619)]);
			CString strExtraQuery;
			CPtrArray paramList;
			long nStatus = -1;

			// (c.haag 2008-01-07 16:21) - Calculate the status filter
			m_AllocationSheet.GetCurrentFilters(nStatus);
			switch (nStatus) {
			case 1: // Completed & Not Billed (equates to 2 in the query)
				strExtraQuery = " WHERE StatusType = 2 ";
				break;
			case 2: // Completed & Billed (equates to 1 in the query)
				strExtraQuery = " WHERE StatusType = 1 ";
				break;
			default: // Active (equates to 0 in the query)
				strExtraQuery = " WHERE StatusType = 0 ";
				break;
			}
			infReport.strReportName = strExtraQuery;

			//Add our date parameters as the defaults.  This is needed for the report to understand correctly
			CRParameterInfo *pFilter = new CRParameterInfo;
			pFilter->m_Name = "DateFrom";
			pFilter->m_Data = "01/01/1000";
			paramList.Add((void *)pFilter);

			pFilter = new CRParameterInfo;
			pFilter->m_Name = "DateTo";
			pFilter->m_Data = "12/31/5000";
			paramList.Add((void *)pFilter);

			RunReport(&infReport, &paramList, pInfo->m_bPreview, (CWnd *)this, "Allocation List", pInfo);
			ClearRPIParameterList(&paramList);
		}
		break;


		case InventoryModule::OrderTab:
		{
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(18)]);
			
			//Made new function for running reports - JMM 5-28-04
			RunReport(&infReport, pInfo->m_bPreview, (CWnd *)this, "On Order / Received", pInfo);	
		}
		break;

		//DRT 11/16/2007 - PLID 27990 - We now have a report for the consignment list		
		case InventoryModule::OverviewTab:

		try {
			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(618)]);
			CPtrArray paramList;

			//We need to apply all the filters
			// (j.jones 2008-03-05 17:13) - PLID 29202 - added provider filters to this report
			// (j.jones 2009-02-09 16:11) - PLID 32872 - added OrderID
			long nProductID = -1, nSupplierID = -1, nLocationID = -1, nType = -1, nProviderID = -1, nOrderID = -1;
			short nDateFilter = -1;
			CString strDateFilter;
			bool bUseDateFilter = false;
			COleDateTime dtFrom, dtTo;
			long nCategoryID = -1, nProductType = 0;

			//Pull the filters from the dialog
			//TES 9/3/2008 - PLID 31237 - Added parameters for which date they're filtering on.
			m_OverviewSheet.GetCurrentFilters(nProductID, nSupplierID, nLocationID, nType, nProviderID, bUseDateFilter, nDateFilter, strDateFilter, dtFrom, dtTo, nCategoryID, nProductType, nOrderID);

			//These are standard filters, used by the reports module
			if(nLocationID != -1) {
				infReport.nLocation = nLocationID;
			}
			if(nType != -1) {
				if(nType == (long)InvUtils::ostOnHand) {
					//"on hand" is a special filter that has both "available" and "allocated" products
					infReport.AddExtraValue( FormatString("%li", (long)InvUtils::ostAvailable) );
					infReport.AddExtraValue( FormatString("%li", (long)InvUtils::ostAllocated) );
				}
				else {
					infReport.AddExtraValue( FormatString("%li", nType) );
				}
			}
			if(nProviderID != -1) {
				infReport.nProvider = nProviderID;
			}
			//TES 9/3/2008 - PLID 31237 - Information about which date they're filtering on is pulled from the tab now.
			infReport.nDateFilter = nDateFilter;
			CRParameterInfo *pFilter = new CRParameterInfo;
			pFilter->m_Name = "DateFilter";
			pFilter->m_Data = strDateFilter;
			paramList.Add((void *)pFilter);

			if(bUseDateFilter) {
				infReport.nDateRange = 2;	//2 means use a date range
				infReport.DateFrom = dtFrom;
				infReport.DateTo = dtTo;

				//We unfortunately have to set our own parameters for the date
				pFilter = new CRParameterInfo;
				pFilter->m_Name = "DateFrom";
				pFilter->m_Data = FormatDateTimeForInterface(dtFrom, NULL, dtoDate);
				paramList.Add((void *)pFilter);

				pFilter = new CRParameterInfo;
				pFilter->m_Name = "DateTo";
				pFilter->m_Data = FormatDateTimeForInterface(dtTo, NULL, dtoDate);
				paramList.Add((void *)pFilter);
			}
			else {
				//Add our date parameters as the defaults.  This is needed for the report to understand correctly
				CRParameterInfo *pFilter = new CRParameterInfo;
				pFilter->m_Name = "DateFrom";
				pFilter->m_Data = "01/01/1000";
				paramList.Add((void *)pFilter);

				pFilter = new CRParameterInfo;
				pFilter->m_Name = "DateTo";
				pFilter->m_Data = "12/31/5000";
				paramList.Add((void *)pFilter);
			}


			//
			//Specials - I hate doing this.  We really need to fix up reports.  But the deal is -- I need many filters, and we really only provide
			//	'extended' and the mystical 'extra id'.  If you want a 3rd, you just have to dive off the deep end and find some random
			//	field that isn't being used for this report.  Unfortunately I'm using a lot of them, so that digs me clear down to 
			//	the strReportName field.  This was really abolished a while ago (Access), and has no use any longer, so it's safe to use.
			//	It just makes me ill that I have to resort to this.
			//So, we're going to use the report name as an extra bit of the WHERE clause.  We'll generate the query here, and just pass it in, 
			//	then the specialized report code will tack it on.  This is very nonstandard.
			//
			CString strExtraQuery;

			if(nProductID != -1) {
				strExtraQuery += FormatString(" AND ProductT.ID = %li", nProductID);
			}

			if(nSupplierID != -1) {
				//We'll add this with the magical "Extra ID", then the report will have to specifically look for its existence.
				strExtraQuery += FormatString(" AND ProductT.ID IN (SELECT ProductID FROM MultiSupplierT WHERE SupplierID = %li)", nSupplierID);
			}

			if(nCategoryID != -1) {
				strExtraQuery += FormatString(" AND ServiceT.Category = %li", nCategoryID);
			}

			if(nProductType != optAll) {
				// (j.jones 2009-03-18 17:53) - PLID 33579 - split consignment into three filter options
				//0 is "all"
				//1 is non-consignment
				//2 is consignment - all
				//3 is consignment - paid
				//4 is consignment - unpaid
				// (c.haag 2007-12-03 11:48) - PLID 28204 - The consignment bit is now a status flag. To
				// take this logic literally, we have to change the format a little.
				//strExtraQuery += FormatString(" AND ProductItemsT.Status = %li", nProductType == 1 ? 0 : 1);
				if(nProductType == optConsignmentAll) {
					strExtraQuery += FormatString(" AND ProductItemsT.Status = %li", InvUtils::pisConsignment);
				}
				else if(nProductType == optConsignmentPaid) {
					strExtraQuery += FormatString(" AND ProductItemsT.Status = %li AND ProductItemsT.Paid = 1", InvUtils::pisConsignment);
				}
				else if(nProductType == optConsignmentUnpaid) {
					strExtraQuery += FormatString(" AND ProductItemsT.Status = %li AND ProductItemsT.Paid = 0", InvUtils::pisConsignment);
				}
				else {
					strExtraQuery += FormatString(" AND ProductItemsT.Status <> %li", InvUtils::pisConsignment);
				}
			}

			// (j.jones 2009-02-09 16:12) - PLID 32873 - added OrderID
			if(nOrderID != -1) {
				strExtraQuery += FormatString(" AND ProductItemsT.OrderDetailID IN (SELECT ID FROM OrderDetailsT WHERE OrderID = %li)", nOrderID);
			}

			//Set it in the report
			infReport.strReportName = strExtraQuery;

			// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
			RunReport(&infReport, &paramList, pInfo->m_bPreview, (CWnd*)this, "Consignment List", pInfo, bUseDateFilter);
			ClearRPIParameterList(&paramList);
		} NxCatchAll("Error in OnPreparePrinting : InventoryModule::OverviewTab");
		break;

	}

	return FALSE;
}

//DRT 11/30/2007 - PLID 28252 - Reworked barcode architecture.  This is handled by the base class.

// (j.jones 2007-11-13 13:10) - PLID 28004 - required to keep our ordinals accurate
void CInvView::ShowTabs()
{	
	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to update the tabs
	Modules::Tabs& tabs = g_Modules[Modules::Inventory]->GetTabs();
	std::for_each(tabs.begin(), tabs.end(), boost::bind(&Modules::NxTabUpdater, m_tab, _1));
}


LRESULT CInvView::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {

		// (j.jones 2014-08-08 10:29) - PLID 63232 - if the Inventory module is not active,
		// do nothing, because each tab properly updates itself when it becomes active again
		if (GetMainFrame() && !GetMainFrame()->IsActiveView(INVENTORY_MODULE_NAME)) {
			//the inventory module is not active, so don't bother updating the active tab
			return 0;
		}

		// (c.haag 2007-11-15 12:42) - PLID 28094 - Added table checker support
		if (m_pActiveSheet) {
			m_pActiveSheet->SendMessage(WM_TABLE_CHANGED, wParam, lParam);
		}
	} NxCatchAll(__FUNCTION__);
	return 0;
}

int CInvView::ShowPrefsDlg()
{
	//TES 11/17/2010 - PLID 41528 - There's a preference node for the Glasses Order tab now.
	if(m_pActiveSheet == &m_GlassesOrderSheet) {
		return ShowPreferencesDlg(GetRemoteData(), GetCurrentUserName(), GetRegistryBase(), piGlassesOrder);
	}
	else {
		return ShowPreferencesDlg(GetRemoteData(), GetCurrentUserName(), GetRegistryBase(), piInventoryModule);
	}
}
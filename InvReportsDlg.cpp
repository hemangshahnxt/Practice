// InvReportsDlg.cpp : implementation file
// (c.haag 2009-01-12 15:21) - PLID 32683 - Initial implementation
//

#include "stdafx.h"
#include "Practice.h"
#include "InvReportsDlg.h"
#include "ReportView.h"
#include "GlobalReportUtils.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

// CInvReportsDlg dialog

IMPLEMENT_DYNAMIC(CInvReportsDlg, CNxDialog)

CInvReportsDlg::CInvReportsDlg(CWnd* pParent)
	: CNxDialog(CInvReportsDlg::IDD, pParent)
{

}

CInvReportsDlg::~CInvReportsDlg()
{
}

// (c.haag 2009-01-12 16:32) - Opens the reports module, selects the inventory tab,
// and selects the report defined by nReportID. If nReportID is not -1, the batch
// is cleared and the report is added to it. If it is -1, the batch is unchanged.
void CInvReportsDlg::JumpToReportsModule(long nReportID)
{
	// Check for permissions first
	if (!UserPermission(ReportModuleItem)) {
		return;
	}
	if (nReportID != -1 && !CheckCurrentUserReportAccess(nReportID, FALSE, TRUE)) {
		return;
	}

	CMainFrame *pMainFrame = GetMainFrame();
	if (pMainFrame != NULL) {
		// Open the reports module
		if (pMainFrame->FlipToModule(REPORT_MODULE_NAME)) {
			CNxTabView *pView = pMainFrame->GetActiveView();
			if (pView && pView->IsKindOf(RUNTIME_CLASS(CReportView))) {
				CReportView* pReportView = ((CReportView*)pView);
				// Always set the active tab to the Inventory tab, even if we're not
				// selecting a specific report.
				if (pReportView->SetActiveTab(ReportsModule::InventoryTab)) {
					if (nReportID != -1) {
						pReportView->ClearReportBatch();
						pReportView->AddReportToBatch(nReportID);
					}
				} else {
					// The tab was not available. There is no prompt from within SetActiveReportTab() or its children if it
					// fails...but we should probably never get here anyway if we cleared CheckCurrentUserReportAccess(). 
					AfxMessageBox("No Inventory reports are available from the Reports module. Please contact your administrator for assistance.", MB_OK | MB_ICONERROR);
				}
			} else {
				// This should never happen
				ASSERT(FALSE);
			}
		} else {
			// Failed to open the module
		}
	} else {
		// CMainFrm is NULL. This should never happen.
		ASSERT(FALSE);
	}
}

void CInvReportsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}
BEGIN_MESSAGE_MAP(CInvReportsDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_CONSIGNMENT_HISTORY_BY_DATE, OnBnConsignmentHistoryByDate)
	ON_BN_CLICKED(IDC_BTN_CONSIGNMENT_LIST, OnBnConsignmentList)
	ON_BN_CLICKED(IDC_BTN_CONSIGNMENT_TURN_RATE_BY_MONTH, OnBnConsignmentTurnRateByMonth)
	ON_BN_CLICKED(IDC_BTN_SERIAL_BY_PT, OnBnSerialByPt)
	ON_BN_CLICKED(IDC_BTN_SERIAL_IN_STOCK, OnBnSerialInStock)
	ON_BN_CLICKED(IDC_BTN_PHYSICAL_INVENTORY, OnBnPhysicalInventory)
	ON_BN_CLICKED(IDC_BTN_ALLOCATION_LIST, OnBnAllocationList)
	ON_BN_CLICKED(IDC_BTN_GO_REPORTS, OnBnGoReports)
END_MESSAGE_MAP()

void CInvReportsDlg::OnBnConsignmentHistoryByDate()
{
	try {
		JumpToReportsModule(621);
	}
	NxCatchAll("Error in CInvReportsDlg::OnBnConsignmentHistoryByDate");
}

void CInvReportsDlg::OnBnConsignmentList()
{
	try {
		JumpToReportsModule(620);
	}
	NxCatchAll("Error in CInvReportsDlg::OnBnConsignmentList");
}

void CInvReportsDlg::OnBnConsignmentTurnRateByMonth()
{
	try {
		JumpToReportsModule(655);
	}
	NxCatchAll("Error in CInvReportsDlg::OnBnConsignmentTurnRateByMonth");
}

void CInvReportsDlg::OnBnSerialByPt()
{
	try {
		JumpToReportsModule(323);
	}
	NxCatchAll("Error in CInvReportsDlg::OnBnSerialByPt");
}

void CInvReportsDlg::OnBnSerialInStock()
{
	try {
		JumpToReportsModule(324);
	}
	NxCatchAll("Error in CInvReportsDlg::OnBnSerialInStock");
}

void CInvReportsDlg::OnBnPhysicalInventory()
{
	try {
		JumpToReportsModule(623);
	}
	NxCatchAll("Error in CInvReportsDlg::OnBnPhysicalInventory");
}

void CInvReportsDlg::OnBnAllocationList()
{
	try {
		JumpToReportsModule(619);
	}
	NxCatchAll("Error in CInvReportsDlg::OnBnAllocationList");
}

void CInvReportsDlg::OnBnGoReports()
{
	try {
		JumpToReportsModule(-1);
	}
	NxCatchAll("Error in CInvReportsDlg::OnBnGoReports");
}
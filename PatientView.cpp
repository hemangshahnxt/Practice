// PatientView.cpp : implementation of the CPatientView class
//

#include "stdafx.h"
#include "Practice.h"
#include "PracProps.h"
#include "PatientView.h"
#include "SuperBill.h"
#include "Reports.h"
#include "UserWarningDlg.h"	// Chris 6/3/99
#include "NxStandard.h"
#include "MainFrm.h"
#include "GlobalReportUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "AdvPrintOptions.h"
#include "BillingModuleDlg.h"
#include "Barcode.h"
#include "PatientGroupsDlg.h"
#include "BillingRc.h"
#include "AuditTrail.h"
#include "GlobalDataUtils.h"
#include "HistoryFilterDlg.h"
#include "InternationalUtils.h"
#include "preferenceutils.h"
#include "nxmessagedef.h"
#include "NxPreviewView.h"
#include "EmrUtils.h"
#include "DontShowDlg.h"
#include "OPOSMSRDevice.h"
#include "Rewards.h"
#include "ReferralOrderDlg.h"
#include "OrderSetsDlg.h"
#include "PatientImmunizationsDlg.h"
#include "PatientGraphConfigDlg.h"
#include "AdvanceDirectiveDlg.h"
#include "boost/bind.hpp"
#include "OPOSBarcodeScanner.h"	//(a.wilson 2012-1-13) PLID 47485
#include "RecallsNeedingAttentionDlg.h" //(a.wilson 2012-2-27) PLID 48305
#include "RecallUtils.h"  //(a.wilson 2012-2-27) PLID 48305
#include "Rewards.h" // (j.luckoski 2013-03-04 11:51) - PLID 33548
#include "General1Dlg.h"
#include "General2Dlg.h"
#include "Custom1Dlg.h"
#include "FollowUpDlg.h"
#include "AppointmentsDlg.h"
#include "HistoryDlg.h"
#include "NexPhotoDlg.h"
#include "FollowUpDlg.h"
#include "NotesDlg.h"
#include "InsuranceDlg.h"
#include "FinancialDlg.h"
#include "QuotesDlg.h"
#include "MedicationDlg.h"
#include "PatientProcedureDlg.h"
#include "SupportDlg.h"
#include "SalesDlg.h"
#include "RefractiveDlg.h"
#include "CustomRecordsTabDlg.h"
#include "PatientNexEMRDlg.h"
#include "PatientLabsDlg.h"
#include "ImplementationDlg.h"
#include "PatientDashboardDlg.h"
#include "ProcInfoCenterDlg.h"
#include "summaryofCareExportdlg.h"
#include "CDSInterventionListDlg.h"
#include "SharedScheduleUtils.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define FINANCIAL_TAB_IN_PROGRESS

// (a.walling 2009-06-05 12:52) - PLID 34496
#define NXM_POST_ONDRAW	WM_APP+0x100

using namespace ADODB;
using namespace NXDATALISTLib;

/////////////////////////////////////////////////////////////////////////////
// CPatientView

IMPLEMENT_DYNCREATE(CPatientView, CNxTabView)

// (a.walling 2008-09-10 15:05) - PLID 31334
// (a.walling 2009-06-05 12:52) - PLID 34496
BEGIN_MESSAGE_MAP(CPatientView, CNxTabView)
	//{{AFX_MSG_MAP(CPatientView)`
	ON_WM_CREATE()
	ON_COMMAND(ID_SUPERBILL, OnSuperbill)
	ON_WM_SYSKEYDOWN()
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_COMMAND(ID_PAT_GROUP_DLG, OnPatGroupDlg)
	ON_COMMAND(IDM_REFERRAL_ORDERS, OnReferralOrders)
	ON_COMMAND(IDM_PATIENT_ORDER_SETS, OnOrderSets)
	ON_MESSAGE(NXM_G1THUMB_PROCESSING_COMPLETE, OnImageProcessingCompleted)
	ON_MESSAGE(NXM_HIDE_BILLING_TOOL_WINDOWS, OnHideBillingToolWindows)
	ON_MESSAGE(NXM_PREVIEW_CLOSED, OnPreviewClosed)
	ON_MESSAGE(NXM_PREVIEW_PRINTED, OnPreviewPrinted)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, OnUpdateFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT, OnUpdateFilePrint)
	ON_MESSAGE(WM_MSR_DATA_EVENT, OnMSRDataEvent)
	ON_MESSAGE(NXM_BARSCAN_DATA_EVENT, OnOPOSBarcodeScannerEvent)
	ON_MESSAGE(NXM_WIA_EVENT, OnWIAEvent)
	ON_COMMAND(ID_ACTIVITIES_IMMUNIZATIONS, OnImmunizations)
	ON_COMMAND(ID_ACTIVITIES_EXPORT_SUMMARY_OF_CARE, OnExportSummaryofCare) // (j.gruber 2013-11-05 12:27) - PLID 59323
	ON_COMMAND(ID_ACTIVITIES_ADVANCE_DIRECTIVES, OnAdvanceDirectives)
	ON_COMMAND(ID_EMR_GRAPHPATIENTEMRDATA, OnGraphPatientEMRData)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_PRINT, CNxTabView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CNxTabView::OnFilePrint)
	//ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_MESSAGE(NXM_POST_ONDRAW, OnPostOnDraw)
	ON_UPDATE_COMMAND_UI(IDM_UPDATE_VIEW, CNxTabView::OnUpdateViewUI) // (z.manning 2010-07-19 16:06) - PLID 39222
	// (j.fouts 2012-06-06 11:06) - PLID 49611 - Handle the changing of preferences
	ON_MESSAGE(WM_PREFERENCE_UPDATED, OnPreferenceUpdated)
	// (j.gruber 2012-06-11 16:11) - PLID 50225
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_MESSAGE(WM_SYSTEM_NAME_UPDATED, OnSystemNameUpdated)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatientView construction/destruction

// (a.walling 2006-09-14 13:28) - PLID 3897 - Disable print preview for these tabs
void CPatientView::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
{
	switch(GetActiveTab()) {
		case PatientsModule::LabTab:
		case PatientsModule::ProcedureTab:
		case PatientsModule::CustomRecordsTab:
		case PatientsModule::CustomTab:
			// (a.walling 2006-10-16 15:56) - PLID 3897 - Re-enable screenshot print previews
			pCmdUI->Enable(TRUE);
			break;
		default:	
			pCmdUI->Enable();
			break;
	}
}

// (a.walling 2006-09-14 13:28) - PLID 3897 - Disable print for these tabs
void CPatientView::OnUpdateFilePrint(CCmdUI* pCmdUI)
{
	switch(GetActiveTab()) {
		case PatientsModule::LabTab:
		case PatientsModule::ProcedureTab:
		case PatientsModule::CustomRecordsTab:
		case PatientsModule::CustomTab:
			// (a.walling 2006-10-16 15:56) - PLID 3897 - Re-enable screenshot print previews
			pCmdUI->Enable(TRUE);
			break;
		default:	
			pCmdUI->Enable();
			break;
	}
}

// (j.fouts 2012-06-06 11:07) - PLID 49611 - Handle the changing of preferences
LRESULT CPatientView::OnPreferenceUpdated(WPARAM wParam, LPARAM lParam)
{
	try
	{
		//Let NexEMR know we changed prefs
		// (j.fouts 2012-06-13 11:21) - PLID 49611 - Only send the message is the window has been created
		if(IsWindow(m_NexEMRSheet))
		{
			m_NexEMRSheet.SendMessage(WM_PREFERENCE_UPDATED);
		}
	}
	NxCatchAll(__FUNCTION__);

	return 0;
}

// (r.gonet 2016-05-24 15:54) - NX-100732 - Handle when the connected system's name changes, ie
// a remote desktop session has just ended or started. Pass it along to listeners.
LRESULT CPatientView::OnSystemNameUpdated(WPARAM wParam, LPARAM lParam)
{
	try {
		if (IsWindow(m_NexPhotoSheet)) {
			m_NexPhotoSheet.SendMessage(WM_SYSTEM_NAME_UPDATED);
		}
	} NxCatchAll(__FUNCTION__);

	return 0;
}

BOOL CPatientView::CheckPermissions()
{
	if (!CheckCurrentUserPermissions(bioPatientsModule, sptView))
		return FALSE;
	return TRUE;
}

// (j.jones 2013-05-07 16:02) - PLID 56591 - changed the child tabs to be declared by reference,
// which means their initialization is slightly different now
CPatientView::CPatientView()
	: m_General1Sheet(*(new CGeneral1Dlg(this)))
	, m_General2Sheet(*(new CGeneral2Dlg(this)))
	, m_CustomSheet(*(new CCustom1Dlg(this)))
	, m_InsuranceSheet(*(new CInsuranceDlg(this)))
	, m_NotesSheet(*(new CNotesDlg(this)))
	, m_FollowUpSheet(*(new CFollowUpDlg(this)))
	, m_AppointmentsSheet(*(new CAppointmentsDlg(this)))
	, m_HistorySheet(*(new CHistoryDlg(this)))
	, m_MedicationSheet(*(new CMedicationDlg(this)))
	, m_FinancialSheet(*(new CFinancialDlg(this)))
	, m_QuotesSheet(*(new CQuotesDlg(this)))
	, m_ProcedureSheet(*(new CPatientProcedureDlg(this)))
	, m_CustomRecordsSheet(*(new CCustomRecordsTabDlg(this)))
	, m_NexEMRSheet(*(new CPatientNexEMRDlg(this)))
	, m_SupportSheet(*(new CSupportDlg(this)))
	, m_SalesSheet(*(new CSalesDlg(this)))
	, m_RefractiveSheet(*(new CRefractiveDlg(this)))
	, m_LabSheet(*(new CPatientLabsDlg(this)))
	, m_ImplementationSheet(*(new CImplementationDlg(this)))
	, m_NexPhotoSheet(*(new CNexPhotoDlg(this)))
	, m_PatientDashboardSheet(*(new CPatientDashboardDlg(this)))
{
	m_bPatientsToolBar = true;
	EnableAutomation();
	m_CurrentTab = -1;
	m_bNeedToDropPatientList = true;
//	m_General1Sheet.m_pParent = this;

	// (c.haag 2003-10-01 12:05) - The billing code is very volatile right now and
	// there's too much stuff to cram into a smaller window. Lets revisit the need
	// for this later on.
	//if (IsScreen640x480())
	//	m_BillingDlg = new CBillingModuleDlg(NULL, IDD_BILLING_MODULE_DLG_640X480);
	//else
		m_BillingDlg = new CBillingModuleDlg(NULL, IDD_BILLING_MODULE_DLG);

	m_BillingDlg->m_pPatientView = this;
	m_FinancialSheet.m_pParent = this;
	m_CustomRecordsSheet.m_pParent = this;

	m_iWarningPatientID = -99999; // Chris 6/3/99

	m_hCheckPatientWarning = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_nCurrentStatus = -1;

	// (d.singleton 2012-04-10 11:21) - PLID 50471
	m_bIsPicPreOpCalendar = FALSE;
	bMedSchedPrintInfoLoaded = FALSE;
	m_nMedSchedDays = 0;
}

CPatientView::~CPatientView()
{
	try {

		if (m_hCheckPatientWarning)
		{
			CloseHandle(m_hCheckPatientWarning);
			m_hCheckPatientWarning = NULL;
		}

		if (m_BillingDlg) {
			// (a.walling 2009-02-23 10:38) - PLID 11034 - Destroy the dialog from here, rather than in the CWnd destructor, so we can handle
			// the DestroyWindow virtual function
			if (m_BillingDlg->GetSafeHwnd()) {
				m_BillingDlg->DestroyWindow();
			}
			delete m_BillingDlg;
			m_BillingDlg = NULL;
		}

		// (j.jones 2013-05-07 16:03) - PLID 56591 - changed the child tabs to be declared by reference,
		// which means we have to clean up their pointers now
		// They are never null. They have to have been filled in the constructor.
		delete &m_General1Sheet;
		delete &m_General2Sheet;
		delete &m_CustomSheet;
		delete &m_InsuranceSheet;
		delete &m_NotesSheet;
		delete &m_FollowUpSheet;
		delete &m_AppointmentsSheet;
		delete &m_HistorySheet;
		delete &m_MedicationSheet;
		delete &m_FinancialSheet;
		delete &m_QuotesSheet;
		delete &m_ProcedureSheet;
		delete &m_CustomRecordsSheet;
		delete &m_NexEMRSheet;
		delete &m_SupportSheet;
		delete &m_SalesSheet;
		delete &m_RefractiveSheet;
		delete &m_LabSheet;
		delete &m_ImplementationSheet;
		delete &m_NexPhotoSheet;
		delete &m_PatientDashboardSheet;

	}NxCatchAll(__FUNCTION__);
}

void CPatientView::DoDataExchange(CDataExchange* pDX)
{
	CNxTabView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatientView)
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CPatientView diagnostics

#ifdef _DEBUG
void CPatientView::AssertValid() const
{
	CNxTabView::AssertValid();
}

void CPatientView::Dump(CDumpContext& dc) const
{
	CNxTabView::Dump(dc);
}

#endif //_DEBUG

void CPatientView::ShowTabs()
{
	if (!g_pLicense)
	{	ASSERT(FALSE);
		HandleException(NULL, "Practice could not find licensing information for this machine.\n"
			"Practice will now close");
		exit(1);
	}

	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to update the tabs
	Modules::Tabs& tabs = g_Modules[Modules::Patients]->GetTabs();
	std::for_each(tabs.begin(), tabs.end(), boost::bind(&Modules::NxTabUpdater, m_tab, _1));
}

int CPatientView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CNxTabView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// (j.jones 2006-04-14 10:15) - PLID 20139 - Load all common patients module properties into the
	// NxPropManager cache
	g_propManager.CachePropertiesInBulk("PatientView", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'MyDefaultTab_Patients' OR "
		"Name = 'FormatPhoneNums' OR "
		"Name = 'DisableQuickBooks' OR "
		"Name = 'General1ImageCount' OR "
		"Name = 'ShowInsuranceReferralWarning' OR "
		"Name = 'AutoDropPatList' OR "
		"Name = 'WarnCopays' OR "	// (j.jones 2010-08-03 10:19) - PLID 39937
		//(e.lally 2012-04-13) PLID 49637
		"Name = 'PhotosLbl_Attached' OR "
		"Name = 'PhotosLbl_Category' OR "
		"Name = 'PhotosLbl_Procedures' OR "
		"Name = 'PhotosLbl_ServiceDate' OR "
		"Name = 'PhotosLbl_Staff' OR "
		"Name = 'PhotosLbl_Filename' OR "
		"Name = 'PhotosLbl_Notes' OR "
		"Name = 'PatientPastDueRecallPrompt' OR " //(a.wilson 2012-2-27) PLID 48305 - preference to show or ignore past due recalls .
		"Name = 'DisplayRewardsWarning' " // (j.luckoski 2013-03-04 13:20) - PLID 33548 - Check preference before allowing prompt
		")",
		_Q(GetCurrentUserName()));

	if (!m_BillingDlg->GetSafeHwnd())
	{
		// (c.haag 2003-10-01 12:05) - The billing code is very volatile right now and
		// there's too much stuff to cram into a smaller window. Lets revisit the need
		// for this later on.
		//if (IsScreen640x480())
		//	m_BillingDlg->Create(NULL, IDD_BILLING_MODULE_DLG_640X480);
		//else
		// (a.walling 2009-12-23 10:18) - PLID 7002 - Create with this view as the parent
		m_BillingDlg->Create(this, IDD_BILLING_MODULE_DLG);
	}

	// (a.walling 2010-11-26 13:08) - PLID 40444 - Call into the modules code to create the tabs
	Modules::Tabs& tabs = g_Modules[Modules::Patients]->Reset().GetTabs();

	// (a.walling 2008-07-02 17:44) - PLID 27648 - Support the alternate Short Title
	CreateSheet(&m_General1Sheet,		tabs, PatientsModule::General1Tab);
	CreateSheet(&m_General2Sheet,		tabs, PatientsModule::General2Tab);
	CreateSheet(&m_CustomSheet,			tabs, PatientsModule::CustomTab);
	CreateSheet(&m_ProcedureSheet,		tabs, PatientsModule::ProcedureTab);
	CreateSheet(&m_FollowUpSheet,		tabs, PatientsModule::FollowUpTab);
	CreateSheet(&m_NotesSheet,			tabs, PatientsModule::NotesTab);
	CreateSheet(&m_AppointmentsSheet,	tabs, PatientsModule::AppointmentTab);
	CreateSheet(&m_QuotesSheet,			tabs, PatientsModule::QuoteTab);
	CreateSheet(&m_InsuranceSheet,		tabs, PatientsModule::InsuranceTab);
	CreateSheet(&m_FinancialSheet,		tabs, PatientsModule::BillingTab);
	CreateSheet(&m_HistorySheet,		tabs, PatientsModule::HistoryTab);
	CreateSheet(&m_NexPhotoSheet,		tabs, PatientsModule::NexPhotoTab);
	CreateSheet(&m_CustomRecordsSheet,	tabs, PatientsModule::CustomRecordsTab);
	CreateSheet(&m_NexEMRSheet,			tabs, PatientsModule::NexEMRTab);
	CreateSheet(&m_MedicationSheet,		tabs, PatientsModule::MedicationTab);
	CreateSheet(&m_LabSheet,			tabs, PatientsModule::LabTab);
	CreateSheet(&m_RefractiveSheet,		tabs, PatientsModule::RefractiveTab);
	CreateSheet(&m_SupportSheet,		tabs, PatientsModule::SupportTab);
	CreateSheet(&m_SalesSheet,			tabs, PatientsModule::SalesTab);
	CreateSheet(&m_ImplementationSheet,	tabs, PatientsModule::ImplementationTab);
	CreateSheet(&m_PatientDashboardSheet, tabs, PatientsModule::PatientDashboard); 	// (j.gruber 2012-03-07 15:42) - PLID 48702 patient dashboard

	// (a.walling 2007-05-21 14:22) - PLID 26079 - Initialize the rewards property cache
	Rewards::Initialize();	

//this way it doesn't look like a tab is being hidden from our users
	// (c.haag 2009-08-20 17:35) - PLID 35231 - Increased the tab width to include NexPhoto

	// (a.wilson 2012-07-05 11:34) - PLID 51332 - replace with check for license and permission.
	// (j.gruber 2012-03-07 15:48) - PLID 48702 - at least for now, check to see if we are showing the dashboard so it doesn't look like we are missing a tab
	BOOL bShowDash = (g_pLicense && g_pLicense->HasEMR(CLicense::cflrSilent) == 2);
	if(IsNexTechInternal())
		// (j.gruber 2008-02-13 12:54) - PLID 28023 - incremented 2 since nothing were added for the labs tab
		m_tab->TabWidth = bShowDash ? 19 : 18;
	else {
		if(IsRefractive()) {
			m_tab->TabWidth = bShowDash ? 17 : 16; 
		}
		else
			m_tab->TabWidth = bShowDash ? 16 : 15; 
	}

	//DRT 4/18/2008 - PLID 29711 - Set the text color of the tabs.
	// (j.jones 2016-04-18 17:46) - NX-100214 - the NxTab control now defaults
	// to the value of CNexTechDialog::GetDefaultNxTabTextColor()


	ShowTabs();
	UpdateView();
	return 0;
}

CBillingModuleDlg * CPatientView::GetBillingDlg()
{
	return m_BillingDlg;
}

void CPatientView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	CNxTabView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CPatientView::SetColor()
{
	DWORD color;
	CPatientDialog *pWnd;
	CMainFrame *pMainFrame;
	CPatientToolBar *pPatToolbar;
	_DNxDataListPtr pPatList;

	try
	{
		pMainFrame = GetMainFrame();
		if (pMainFrame && m_pActiveSheet) 
		{	// Get a pointer to the main window's patient toolbar
			pPatToolbar = &(pMainFrame->m_patToolBar);
			if (pPatToolbar->GetSafeHwnd()) 
			{	
				// (b.spivey, May 24, 2012) - PLID 50432 - Deprecated. Internal and main work the same way now. 
				// Get a pointer to the main patient toolbar's patient dropdown
				/* if(IsNexTechInternal()) {
					//DRT 5/23/2006 - PLID 20770
					//	For sales purposes, we are going to do some coloring based on the patient type.  All of these
					//	apply only to clients, not prospects.
					//
					//	>> Red
					//		Client 5 Mission Critical Problems Continuous FU
					//	>> Light Red (Pink)
					//		Client 4 Significant Ebilling Problems Follow Up
					//		Client 4 Significant Hardware Problems Follow Up
					//		Client 4 Significant Problems Follow Up
					//	>> Yellow
					//		Client 3 Minor Problems
					//	>> Normal coloring (blue)
					//		Client 1 Happy
					//		Client 2 Stable
					//
					//Note that I have made these colors configurable through ConfigRT, due to the 
					//	nature of sales often changing their mind.  The above comments are the originals, 
					//	and may be changed in the future.

					//Default for most
					color = GetNxColor(GNC_PATIENT_STATUS, pPatToolbar->GetActivePatientStatus());

					//Optimization:  The current status is already stored, hidden, in the patient toolbar.  Retrieve it so
					//	we are only querying for patients.
					long nStatus = pPatToolbar->GetActivePatientStatus();
					if(nStatus == 1) {
						_RecordsetPtr prs = CreateRecordset("SELECT TypeOfPatient FROM PatientsT WHERE PersonID = %li", GetActivePatientID());
						if(!prs->eof) {
							//Hardcoded ID numbers
							switch(AdoFldLong(prs, "TypeOfPatient", -1)) {
							case 244:	//Client 5 Mission Critical Problems Continuous FU
								color = GetRemotePropertyInt("Internal_PatientColor_Status5", RGB(255, 130, 130), 0, "<None>", true);
								break;
							case 243:	//Client 4 Significant Problems Follow Up
							case 320:	//Client 4 Significant Hardware Problems Follow Up
							case 319:	//Client 4 Significant Ebilling Problems Follow Up
								color = GetRemotePropertyInt("Internal_PatientColor_Status4", RGB(255, 187, 189), 0, "<None>", true);
								break;
							case 242:	//Client 3 Minor Problems
								color = GetRemotePropertyInt("Internal_PatientColor_Status3", RGB(255, 255, 166), 0, "<None>", true);
								break;
							}
						}
						prs->Close();
					}
				}*/
				
					//TES 1/6/2010 - PLID 36761 - Remember the current status
					m_nCurrentStatus = pPatToolbar->GetActivePatientStatus();
					// (b.spivey, May 15, 2012) - PLID 20752 - Get the PatientID and use it with GetNxColor(). 
					long nPatID = pPatToolbar->GetActivePatientID(); 
					//Normal setup, not using practice internal
					color = GetNxColor(GNC_PATIENT_STATUS, m_nCurrentStatus, nPatID);
				

				// (j.gruber 2012-03-07 15:42) - PLID 48702 patient dashboard
				//the patient dashboard has no color, so don't try to set it
				if (m_tab->CurSel != PatientsModule::PatientDashboard) {
					pWnd = (CPatientDialog *)m_pActiveSheet;
					if (color != pWnd->GetCurrentColor())
						pWnd->SetColor(color);
				}
			}
		}
	}NxCatchAll("Error setting colors");
}

void CPatientView::PromptPatientWarning()
{
	// (a.walling 2009-06-05 12:56) - PLID 34496 - All dialogs from this function will now have a proper parent

	try {
		//DRT 3/31/2008 - PLID 29489 - We can greatly optimize this function by (a) parameterizing the query information, and 
		//	(b) only executing 1 trip to the server instead of 6.
		//We'll also simplify by just declaring the patient ID ahead of time, since all the queries use it.
		CString strQuery;
		strQuery.Format(
			"DECLARE @PatID int;\r\n"
			"SET @PatID = {INT};\r\n");


		//
		//Warning #1 - EMR Reconstructed Details.  Only shows if we're NOT on the EMR screen.
		if (GetActiveSheet() != &m_NexEMRSheet) {
			CString strTmp;
			strTmp.Format("SELECT COUNT(DISTINCT EMRMasterT.ID) AS CountToWarn "
				"FROM EMRMasterT INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
				"INNER JOIN ReconstructedEMRDetailsT ON EMRDetailsT.ID = ReconstructedEMRDetailsT.EMRDetailID "
				"WHERE ReconstructedEMRDetailsT.ReviewStatus IN (-1) AND EMRMasterT.PatientID = @PatID;\r\n");
			strQuery += strTmp;
		}

		//
		//Warning #2 - Patient general 2 warning field.  This always shows if data exists.
		{
			// (a.walling 2010-07-01 16:25) - PLID 18081 - Warning categories
			CString strTmp;
			strTmp.Format("SELECT PersonT.DisplayWarning, PersonT.Last + ', ' + PersonT.First AS FullName, PersonT.WarningMessage, UserName, PersonT.WarningDate, PersonT.WarningUseExpireDate, PersonT.WarningExpireDate, WarningCategoriesT.Color AS WarningColor, WarningCategoriesT.ID AS WarningID, WarningCategoriesT.Name AS WarningName, "
				"PatientsT.DefaultReferringPhyID, ReferringPhysT.ShowProcWarning, ReferringPhysT.ProcWarning, "
				"PersonRefPhys.Last + ', ' + PersonRefPhys.First + ' ' + PersonRefPhys.Middle AS RefPhysName "
				"FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"LEFT JOIN WarningCategoriesT ON PersonT.WarningCategoryID = WarningCategoriesT.ID "
				"LEFT JOIN UsersT ON UsersT.PersonID = PersonT.WarningUserID "
				"LEFT JOIN (ReferringPhysT INNER JOIN PersonT PersonRefPhys ON ReferringPhysT.PersonID = PersonRefPhys.ID) "
				"ON PatientsT.DefaultReferringPhyID = ReferringPhysT.PersonID "
				"WHERE (((PatientsT.PersonID)=@PatID));\r\n");
			strQuery += strTmp;
		}

		//
		//Warning #3 - SupportExpires and low patient type messages.  These only show up in Internal.
		if(IsNexTechInternal()) {
			CString strTmp;
			strTmp.Format("SELECT SupportExpires FROM NxClientsT WHERE PersonID = @PatID;\r\n");
			strQuery += strTmp;

			strTmp.Format("SELECT TypeOfPatient, GroupName FROM PatientsT INNER JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex "
				"WHERE PersonID = @PatID AND (TypeOfPatient = 243 OR TypeOfPatient = 244);\r\n");
			strQuery += strTmp;
		}

		//
		//Warning #4 - Insurance referral warnings.  These only show up if the preference is enabled to warn.
		//	DRT 3/31/2008 - PLID 29489 - While changing to parameterized, I changed the date filtering to use SQL getdate() instead
		//	of FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate).  This does mean a slight change in that it'll use the 
		//	server time for warning instead of the client time.
		// (c.haag 2008-11-21 10:45) - PLID 32136 - Filter out inactive insured parties
		// (j.jones 2011-09-29 15:53) - PLID 44980 - ignore referrals if in use on a voided bill
		// (j.gruber 2011-10-06 10:34) - PLID 45837 - add fields
		// (j.gruber 2011-10-06 10:35) - PLID 45838 - changed to any non-expired referral
		if(GetRemotePropertyInt("ShowInsuranceReferralWarning",0,0,"<None>",TRUE) == 1) {
			CString strTmp;
			strTmp.Format(
				"SELECT InsuranceCoT.Name AS InsCoName, "
				"AuthNum, StartDate, EndDate, NumVisits, (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) AS NumUsed, "
				" PersonProvT.ID as ProviderID, PersonProvT.Last + ', ' + PersonProvT.First + ' ' + PersonProvT.Middle AS ProviderName, "
				" LocationsT.ID as LocationID, LocationsT.Name as LocationName, InsuranceReferralsT.Comments "			
				"FROM InsuranceReferralsT "
				"INNER JOIN InsuredPartyT ON InsuranceReferralsT.InsuredPartyID = InsuredPartyT.PersonID "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"LEFT JOIN ProvidersT ON InsuranceReferralsT.ProviderID = ProvidersT.PersonID "
				"LEFT JOIN PersonT PersonProvT ON ProvidersT.PersonID = PersonProvT.ID "
				"LEFT JOIN LocationsT ON InsuranceReferralsT.LocationId = LocationsT.ID "					
				"LEFT JOIN (SELECT Count(InsuranceReferralID) AS NumUsed, InsuranceReferralID FROM BillsT WHERE Deleted = 0 AND InsuranceReferralID IS NOT NULL AND BillsT.ID NOT IN (SELECT OriginalBillID FROM BillCorrectionsT) "
				"GROUP BY InsuranceReferralID) AS NumUsedQ ON InsuranceReferralsT.ID = NumUsedQ.InsuranceReferralID "
				"WHERE NumVisits > (CASE WHEN NumUsed Is NULL THEN 0 ELSE NumUsed END) "
				"AND EndDate >= convert(datetime, convert(nvarchar, getdate(), 101)) AND InsuredPartyT.PatientID = @PatID "
				"AND InsuredPartyT.RespTypeID <> -1 "
				"ORDER BY StartDate;\r\n");
			strQuery += strTmp;
		}

		//
		//Warning #5 - CoPay warning.
		// (j.jones 2010-08-02 11:23) - PLID 39937 - WarnCoPay is now a preference, and no longer a setting per insured party
		// (j.jones 2010-09-01 15:58) - PLID 40356 - changed so we never warn about $0.00 / 0% copays
		BOOL bWarnCopays = (GetRemotePropertyInt("WarnCopays", 0, 0, GetCurrentUserName(), true) == 1);
		if(bWarnCopays) {
			// (j.jones 2009-09-17 10:33) - PLID 35572 - ensure we do not warn about inactive insurances
			CString strTmp;
			strTmp.Format(
				" SELECT [Last] + ', ' + [First] AS FullName, InsuranceCoT.Name AS InsCoName, "
				" CoPayMoney, CopayPercentage, ServicePayGroupsT.Name AS PayGroupName "
				" FROM InsuredPartyT "
				" INNER JOIN PatientsT ON InsuredPartyT.PatientID = PatientsT.PersonID "
				" INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				" INNER JOIN InsuredPartyPayGroupsT ON InsuredPartyT.PersonID = InsuredPartyPayGroupsT.InsuredPartyID "
				" INNER JOIN ServicePayGroupsT ON InsuredPartyPayGroupsT.PayGroupID = ServicePayGroupsT.ID "
				" WHERE PatientsT.PersonID = @PatID AND InsuredPartyT.RespTypeID <> -1 "
				" AND ((CoPayMoney Is Not Null AND CopayMoney <> Convert(money,0)) OR (CopayPercentage Is Not Null AND CopayPercentage <> 0)) "
				" ORDER BY RespTypeID, ServicePayGroupsT.Name;\r\n");
			strQuery += strTmp;
		}

		//
		//Warning #6 - Warn for any allergies.  This is always executed.
		{		
			CString strTmp;
			// (j.gruber 2009-06-02 15:05) - PLID 34450 - show only active allergies
			// (j.jones 2013-08-12 15:52) - PLID 57977 - Changed this query to get the allergies itself and not call the UDF function.
			// This is a left join because if warning is enabled, but they have no allergies, we say "<None>".
			strTmp.Format(
				"SELECT PersonT.FullName, PatientAllergyQ.AllergyName "
				"FROM PatientsT "
				"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				"LEFT JOIN (SELECT EmrDataT.Data AS AllergyName, PatientAllergyT.PersonID "
				"	FROM PatientAllergyT "
				"	LEFT JOIN AllergyT ON PatientAllergyT.AllergyID = AllergyT.ID "
				"	LEFT JOIN EmrDataT ON EmrDataT.ID = AllergyT.EmrDataID "
				"	WHERE PatientAllergyT.Discontinued = 0) AS PatientAllergyQ ON PatientsT.PersonID = PatientAllergyQ.PersonID "
				"WHERE PatientsT.PersonID = @PatID "
				"AND PersonT.DisplayAllergyWarning = 1 "
				"ORDER BY PatientAllergyQ.AllergyName;\r\n");
			strQuery += strTmp;
		}

		//Warning #7 - Generate or warn the user of a past due recall appointment with no linked appointment.
		//(a.wilson 2012-2-27) PLID 48305 - for the recall project.  when navigating to a new patient. warn if any past due.
		// (j.armen 2012-03-20 13:36) - PLID 49057 - avoid ambiguity - Reference the table
		// (a.wilson 2012-3-23) PLID 48472 - added recall permission to prevent popup if they dont have red permission.
		// (j.armen 2012-03-28 11:17) - PLID 48480 - Added License Check
		bool bDisplayRecalls = 
			(g_pLicense->CheckForLicense(CLicense::lcRecall, CLicense::cflrSilent)) &&
			(GetRemotePropertyInt("PatientPastDueRecallPrompt", 1, 0, GetCurrentUserName(), true) == 1 &&
			(GetCurrentUserPermissions(bioRecallSystem) & sptRead));

		// (a.walling 2013-11-25 10:58) - PLID 60006 - If this is visible we do not do anything, so we can just not query at all.
		if (GetMainFrame()->m_pRecallsNeedingAttentionDlg && GetMainFrame()->m_pRecallsNeedingAttentionDlg->IsWindowVisible()) {
			bDisplayRecalls = false;
		}

		if (bDisplayRecalls) {

			// (a.walling 2013-11-25 10:58) - PLID 60006 - Only need past due recalls
			// (a.walling 2014-01-06 11:36) - PLID 60006 - RecallT.RecallDate <= dbo.AsDateNoTime(GetDate()) intead of >=
			strQuery += "\r\n"
				"SELECT CASE WHEN EXISTS(\r\n"
					"SELECT NULL \r\n"
					"FROM RecallT \r\n"
					"LEFT JOIN AppointmentsT \r\n"
						"ON RecallT.RecallAppointmentID = AppointmentsT.ID \r\n"
					"WHERE \r\n"
						"RecallT.PatientID = @PatID \r\n"
						"AND RecallT.RecallDate <= dbo.AsDateNoTime(GetDate()) \r\n"
						"AND RecallT.Discontinued <> 1 \r\n"
						"AND (RecallT.RecallAppointmentID IS NULL OR AppointmentsT.Status = 4 OR AppointmentsT.ShowState = 3) \r\n"
				") THEN 1 ELSE 0 END;";
		}

		//
		//Warning #8 - Warn for any rewards. // (j.luckoski 2013-03-04 13:44) - PLID 33548 
		{		
			CString strTmp;
			strTmp.Format(
				" SELECT PersonT.DisplayRewardsWarning, PersonT.FullName "
				" FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
				" WHERE (((PatientsT.PersonID)= @PatID));\r\n");
			strQuery += strTmp;
		}

		//Warning #9 - Warn on any active CDS Interventions  //TES 11/4/2013 - PLID 59276
		{
			CString strTmp;
			strTmp.Format(" SELECT PersonT.FullName "
				" FROM DecisionRuleInterventionsT INNER JOIN PersonT ON DecisionRuleInterventionsT.PatientID = PersonT.ID "
				" WHERE DecisionRuleInterventionsT.AcknowledgedBy Is Null AND DecisionRuleInterventionsT.PatientID = @PatID;\r\n");
			strQuery += strTmp;
		}



		//
		//Create the recordset, parameterized with our patient ID.
		// (j.jones 2013-08-12 15:52) - PLID 57977 - Used snapshot isolation.
		_RecordsetPtr prsAllWarnings = CreateParamRecordset(GetRemoteDataSnapshot(), strQuery, m_iWarningPatientID);

		//From here we'll just go through and pull off the recordsets that were generated as they are needed.

		//
		//Warning #1
		// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Give a message if any EMNs contain reconstructed details.
		if (GetActiveSheet() != &m_NexEMRSheet) {
			_RecordsetPtr prs = prsAllWarnings;
				
			long nCountToWarn = AdoFldLong(prs->GetFields()->GetItem("CountToWarn"));
			if (nCountToWarn > 0) {
				DontShowMeAgain(this, "This patient has EMNs with reconstructed details that have not yet "
					"been verified by you.  Please review these EMNs in the NexEMR tab to verify they are correct. \r\n"
					"\r\n"
					// (a.walling 2011-08-23 14:15) - PLID 44647 - exception/error emails to allsupport@nextech.com
					"If you have ANY questions please contact NexTech Technical Support at 888-417-8464 or allsupport@nextech.com.", 
					"ReconstructedEMRDetails", "Reconstructed Data", FALSE, FALSE);
			}

			prsAllWarnings = prsAllWarnings->NextRecordset(NULL);
		}

		//
		//Warning #2
		// Open the recordset
		_RecordsetPtr rs = prsAllWarnings;
			
		CString strTitle;

		if(!rs->eof) {
			// Get pointers to useful field objects
			FieldsPtr flds = rs->Fields;
			FieldPtr fldDispWarning = flds->Item["DisplayWarning"];
			
			// If the patient is displaying a warning
			BOOL bDispWarning = AdoFldBool(fldDispWarning);
			if (bDispWarning) {

				//DRT 1/10/2006 - PLID 18784 - The expiration warning was just saying "return", and therefore it was
				//	making ALL system warnings expired (allergies, copays, etc).  It should only apply to the G2 warning, 
				//	and leave the rest as is.
				BOOL bG2WarningExpired = FALSE;

				//check to see if they are using the expiration date
				if (AdoFldBool(flds, "WarningUseExpireDate")) {

					//check to see if the warning has expired
					COleDateTime dtExpire = VarDateTime(flds->Item["WarningExpireDate"]->Value);
					COleDateTime dtToday;
					dtToday.SetDate(COleDateTime::GetCurrentTime().GetYear(), COleDateTime::GetCurrentTime().GetMonth(), COleDateTime::GetCurrentTime().GetDay());
					if (dtExpire < dtToday) {

						//don't show it
						bG2WarningExpired = TRUE;
					}
				}



//*
				// TODO: Fix the login instead of putting the band-aid here
				// RAC - I had to put this in because of a design flaw in the Login.  Brad set up the login screen to be 
				// displayed until Practice finishes loading and becomes visible.  The problem is that the login screen 
				// can't be the application's main window once the CMainFrame is loaded or else the views (like the 
				// Patients View) will try to create themselves as MDI children of the login screen instead of the 
				// CMainFrame.  But then we're left with the problem that the userwarning message comes up BEHIND the 
				// login screen and then the program seems to be locked up even though it's really not because you can 
				// always just move the login screen out of the way...so my temporary solution was to just hide the login 
				// screen if and when the userwarning is about to be displayed.  The problem is that userwarnings are not 
				// the only things that bring up modal dialog boxes...AfxMessageBox and other message boxes do too.
				extern CPracticeApp theApp;
				if ((theApp.m_pMainWnd != (CWnd *)theApp.m_dlgLogin) && ((CWnd *)theApp.m_dlgLogin)->GetSafeHwnd()) {
					((CWnd *)theApp.m_dlgLogin)->ShowWindow(SW_HIDE);
				}
//*/

				if(!bG2WarningExpired) {
					// Display it
					if (!GetPropertyInt("G2ShowWarningStats", 1, 0))
						strTitle = AdoFldString(flds, "FullName") + ":";
					else
					{
						if (flds->Item["UserName"]->Value.vt == VT_NULL ||
							flds->Item["WarningDate"]->Value.vt == VT_NULL)
						{
							strTitle = AdoFldString(flds, "FullName") + ":";
						}
						else
						{
							strTitle = CString("Warning for ") + AdoFldString(flds, "FullName") + " entered by " + AdoFldString(flds, "UserName") + " on " + FormatDateTimeForInterface(AdoFldDateTime(flds, "WarningDate"), DTF_STRIP_SECONDS, dtoDate);
						}
					}

					CUserWarningDlg dlgWarning(this);
					// (a.walling 2010-07-01 16:34) - PLID 18081 - Warning categories
					BOOL bKeepWarning = dlgWarning.DoModalWarning(AdoFldString(flds, "WarningMessage"), 
						TRUE, strTitle, "Patient Warning", AdoFldLong(flds, "WarningColor", 0), AdoFldLong(flds, "WarningID", -1), AdoFldString(flds, "WarningName", ""));

					//DRT - 11/25/02 - We cannot compare booleans like this - 0 is false, nonzero is true. 
					//				In some cases one will be 1, the other -1, both true, but not equal
					//if (bKeepWarning != bDispWarning) {

					// If the user asked to change the displaywarning status of the patient
					if((bKeepWarning == 0 && bDispWarning != 0) || (bKeepWarning != 0 && bDispWarning == 0)) {
						// Change it
						//DRT 4/8/2004 - PLID 11838 - This update is failing, saying that the row cannot be located for
						//	updating.  I'm not sure why atm when it's worked previously, but since we don't
						//	do recordset updates anywhere else, we really shouldn't be doing them here.
						//fldDispWarning->Value = bKeepWarning?true:false;
						//rs->Update();
						ExecuteSql("UPDATE PersonT SET DisplayWarning = %li WHERE ID = %li", bKeepWarning?1:0, m_iWarningPatientID);

						//auditing
						// (a.walling 2008-08-20 13:01) - PLID 29900 - Avoid GetActivePatient*
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(m_iWarningPatientID, GetExistingPatientName(m_iWarningPatientID), nAuditID, aeiPatientWarning, m_iWarningPatientID, (bKeepWarning == 0 ? "Warn" : "Don't Warn"), (bKeepWarning == 0 ? "Don't Warn" : "Warn"), aepMedium, aetChanged);

						// Update the view given the change
						CNxTabView::UpdateView();
						// Don't be silly, just update the one checkbox!
	//					if (m_pActiveSheet == m_General1Sheet)
					}

					// This is called because the patient warning may steal
					// the focus away from the proper control
					SetDefaultControlFocus();
				}
			}

			//TES 4/2/2004: Check for a referring physician's warning.
			if(AdoFldBool(flds, "ShowProcWarning", FALSE)) {
				//OK, we need to list the procedures performed by this patient.
				CString strMessage = "This patient's referring physician (" + AdoFldString(flds, "RefPhysName") + ") performs the following procedures:\r\n";
				_RecordsetPtr rsProcs = CreateRecordset("SELECT Name FROM ProcedureT INNER JOIN RefPhysProcLinkT ON ProcedureT.ID = RefPhysProcLinkT.ProcedureID "
					"WHERE RefPhysProcLinkT.RefPhysID = %li", AdoFldLong(flds, "DefaultReferringPhyID"));
				while(!rsProcs->eof) {
					strMessage += AdoFldString(rsProcs, "Name") + "; ";
					rsProcs->MoveNext();
				}
				rsProcs->Close();
				if(strMessage.Right(2) == "; ") strMessage = strMessage.Left(strMessage.GetLength()-2);
				CString strExtraText = AdoFldString(flds, "ProcWarning", "");
				if(!strExtraText.IsEmpty()) {
					strMessage += "\r\n" + strExtraText;
				}
				// (a.walling 2009-06-05 12:57) - PLID 34496 - Used to pass unsafe string into MsgBox, which would format, and could fail if contained a %
				MessageBox(strMessage, NULL, MB_ICONINFORMATION);
			}

		}

		//
		//Warning #3
		if(IsNexTechInternal()) {
			//TS 12-27-2001: I'm putting this here because, it seems to me, we want this support warning to behave
			//exactly the same as the patient warning.  Eventually, this will be integrated with the NxSupport.
			_RecordsetPtr rsSupportDate = (prsAllWarnings = prsAllWarnings->NextRecordset(NULL));
			if(!rsSupportDate->eof) {
				_variant_t varExpiration = rsSupportDate->Fields->GetItem("SupportExpires")->Value;
				if(varExpiration.vt == VT_BSTR) { //This is _not_ a date field. Thanks, Don.
					if(_bstr_t(varExpiration).length() != 0) {
						COleDateTime dtExpires;
						dtExpires.ParseDateTime(_bstr_t(varExpiration));

						// (d.thompson 2012-12-31) - PLID 54393 - Tech support expires after the day, not the time.  We can't compare minutes.
						COleDateTime dtNow = COleDateTime::GetCurrentTime();
						dtNow.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());

						if(dtExpires < dtNow) {
							MessageBox("Tech support has expired!", NULL, MB_ICONINFORMATION);
						}
					}
				}
			}

			//DT 2-25-02 - Pop up a similar message box if the client status is low (status 4 or 5)
			_RecordsetPtr rsStatus = (prsAllWarnings = prsAllWarnings->NextRecordset(NULL));
			if(!rsStatus->eof){
				CString str;
				str = "Patient has status: " + CString(rsStatus->Fields->Item["GroupName"]->Value.bstrVal);
				MessageBox(str, NULL, MB_ICONINFORMATION);
			}
		}

		//
		//Warning #4
		if(GetRemotePropertyInt("ShowInsuranceReferralWarning",0,0,"<None>",TRUE) == 1) {
			//show active insurance referrals
			_RecordsetPtr rsInsuranceReferrals = (prsAllWarnings = prsAllWarnings->NextRecordset(NULL));

			if(!rsInsuranceReferrals->eof) {

				CString strPrompt;
				strPrompt = "This patient has the following active insurance referrals:";

				while(!rsInsuranceReferrals->eof) {

					CString strInsCoName = AdoFldString(rsInsuranceReferrals, "InsCoName","");
					CString strAuthNum = AdoFldString(rsInsuranceReferrals, "AuthNum","");
					COleDateTime dtStartDate = AdoFldDateTime(rsInsuranceReferrals, "StartDate");
					COleDateTime dtEndDate = AdoFldDateTime(rsInsuranceReferrals, "EndDate");
					long NumVisits = AdoFldLong(rsInsuranceReferrals, "NumVisits",0);
					long NumUsed = AdoFldLong(rsInsuranceReferrals, "NumUsed",0);

					// (j.gruber 2011-10-05 12:22) - PLID 45837 - added providers, locations, and comments
					long nProviderID = AdoFldLong(rsInsuranceReferrals, "ProviderID", -1);
					CString strProvName;
					if (nProviderID != -1) {
						strProvName = "Provider: " + AdoFldString(rsInsuranceReferrals, "ProviderName", "") + ";";
					}
					CString strLocName;
					long nLocationID = AdoFldLong(rsInsuranceReferrals, "LocationID", -1);
					if (nLocationID != -1) {
						strLocName = "Location: " + AdoFldString(rsInsuranceReferrals, "LocationName", "") + ";";
					}

					CString strTemp;
					strTemp = AdoFldString(rsInsuranceReferrals, "Comments", "");
					CString strComments;
					if (!strTemp.IsEmpty()) {
						strComments = "Comments: " + strTemp + ";";
					}			


					CString strDates;
					if(dtStartDate == dtEndDate) {
						strDates.Format("Allowed Date: %s;",FormatDateTimeForInterface(dtStartDate,NULL,dtoDate));
					}
					else {
						strDates.Format("Allowed Dates: %s - %s;",FormatDateTimeForInterface(dtStartDate,NULL,dtoDate),FormatDateTimeForInterface(dtEndDate,NULL,dtoDate));
					}

					CString strWarning;
					strWarning.Format("\n\nInsurance: %s; Allowed Visits: %li, Remaining: %li; %s %s %s %s",strInsCoName,NumVisits,NumVisits - NumUsed,strDates, strProvName, strLocName, strComments);
					strPrompt += strWarning;

					rsInsuranceReferrals->MoveNext();
				}

				MessageBox(strPrompt, NULL, MB_ICONINFORMATION);
			}
		}

		//Warning #5
		//now check for the CoPay Warning
		// (j.jones 2010-08-02 11:23) - PLID 39937 - WarnCoPay is now a preference, and no longer a setting per insured party
		if(bWarnCopays) {
			_RecordsetPtr rsCoPay = (prsAllWarnings = prsAllWarnings->NextRecordset(NULL));

			if(!rsCoPay->eof) {
				//*
				// TODO: Fix the login instead of putting the band-aid here
				// RAC - I had to put this in because of a design flaw in the Login.  Brad set up the login screen to be 
				// displayed until Practice finishes loading and becomes visible.  The problem is that the login screen 
				// can't be the application's main window once the CMainFrame is loaded or else the views (like the 
				// Patients View) will try to create themselves as MDI children of the login screen instead of the 
				// CMainFrame.  But then we're left with the problem that the userwarning message comes up BEHIND the 
				// login screen and then the program seems to be locked up even though it's really not because you can 
				// always just move the login screen out of the way...so my temporary solution was to just hide the login 
				// screen if and when the userwarning is about to be displayed.  The problem is that userwarnings are not 
				// the only things that bring up modal dialog boxes...AfxMessageBox and other message boxes do too.
				extern CPracticeApp theApp;
				if ((theApp.m_pMainWnd != (CWnd *)theApp.m_dlgLogin) && ((CWnd *)theApp.m_dlgLogin)->GetSafeHwnd()) {
					((CWnd *)theApp.m_dlgLogin)->ShowWindow(SW_HIDE);
				}

				CString strName = AdoFldString(rsCoPay, "FullName","") + ":";
				CString strCoPayWarning = "This patient has the following copays:\n";

				while(!rsCoPay->eof) {

					CString str, strCopay;

					// (j.jones 2010-08-03 13:17) - PLID 39937 - the copay is now in two fields,
					// mutually exclusive, they cannot both be filled
					_variant_t varCoPayMoney = rsCoPay->Fields->Item["CoPayMoney"]->Value;
					_variant_t varCopayPercentage = rsCoPay->Fields->Item["CopayPercentage"]->Value;

					// (j.jones 2010-09-01 15:58) - PLID 40356 - changed so we never warn about $0.00 / 0% copays
					if(varCoPayMoney.vt == VT_CY && VarCurrency(varCoPayMoney) != COleCurrency(0,0)) {
						//flat amount
						strCopay.Format("%s",FormatCurrencyForInterface(VarCurrency(varCoPayMoney,COleCurrency(0,0))));
					}
					else if(varCopayPercentage.vt == VT_I4 && VarLong(varCopayPercentage) != 0) {
						//percentage
						strCopay.Format("%li%%",VarLong(varCopayPercentage,0));
					}
					else {
						//shady, why do we even have results?
						rsCoPay->MoveNext();
						continue;
					}

					str.Format("\n%s - %s: %s",
						AdoFldString(rsCoPay, "InsCoName",""),
						AdoFldString(rsCoPay, "PayGroupName",""),
						strCopay);
					strCoPayWarning += str;

					rsCoPay->MoveNext();
				}


				// Display it
				MessageBox(strCoPayWarning, "CoPay Warning", MB_ICONINFORMATION|MB_OK);

				// This is called because the patient warning may steal
				// the focus away from the proper control
				SetDefaultControlFocus();
			}
		}

		//
		//Warning #6
		//ok, now check for the Allergy Warning, this code is basically copied from above with changes for the recordset, etc
		// Open the recordset
		_RecordsetPtr rsAllergy = (prsAllWarnings = prsAllWarnings->NextRecordset(NULL));

		// (j.jones 2013-08-12 15:52) - PLID 57977 - Changed this query to get the allergies itself and not call the UDF function.
		// It's a left join because if warning is enabled, but they have no allergies, we say "<None>".
		if(!rsAllergy->eof) {
			FieldsPtr flds = rsAllergy->Fields;
			CString strAllergyPatientName = AdoFldString(flds, "FullName");
			CString strAllergyList = "";

			while(!rsAllergy->eof) {
				//if we have only one record, and there are no allergies, we show "<None>"
				CString strAllergyName = AdoFldString(flds, "AllergyName", "<None>");
				if(!strAllergyList.IsEmpty()) {
					strAllergyList += ", ";
				}
				strAllergyList += strAllergyName;
				rsAllergy->MoveNext();
			}

			// TODO: Fix the login instead of putting the band-aid here
			// RAC - I had to put this in because of a design flaw in the Login.  Brad set up the login screen to be 
			// displayed until Practice finishes loading and becomes visible.  The problem is that the login screen 
			// can't be the application's main window once the CMainFrame is loaded or else the views (like the 
			// Patients View) will try to create themselves as MDI children of the login screen instead of the 
			// CMainFrame.  But then we're left with the problem that the userwarning message comes up BEHIND the 
			// login screen and then the program seems to be locked up even though it's really not because you can 
			// always just move the login screen out of the way...so my temporary solution was to just hide the login 
			// screen if and when the userwarning is about to be displayed.  The problem is that userwarnings are not 
			// the only things that bring up modal dialog boxes...AfxMessageBox and other message boxes do too.
			extern CPracticeApp theApp;
			if ((theApp.m_pMainWnd != (CWnd *)theApp.m_dlgLogin) && ((CWnd *)theApp.m_dlgLogin)->GetSafeHwnd()) {
				((CWnd *)theApp.m_dlgLogin)->ShowWindow(SW_HIDE);
			}
				
			CString strAllergyWarning = "This patient has the following allergies:  " + strAllergyList;

			// Display it
			CUserWarningDlg dlgWarning(this);
			// (a.walling 2010-07-01 16:34) - PLID 18081 - Warning categories
			BOOL bKeepWarning = dlgWarning.DoModalWarning(strAllergyWarning, 
				TRUE, strAllergyPatientName + ":", "Allergy Warning", NULL, -1, "");

			//if they turned off the warning, save the change
			if(bKeepWarning == 0) {
				ExecuteParamSql("UPDATE PersonT SET DisplayAllergyWarning = 0 WHERE ID = {INT}", m_iWarningPatientID);

				// Update the view given the change
				CNxTabView::UpdateView();
			}

			// This is called because the patient warning may steal
			// the focus away from the proper control
			SetDefaultControlFocus();
		}

		//Warning #7 - get records of possible past due recalls.
		//(a.wilson 2012-2-27) PLID 48305 - check for any possible past due recalls.
		if (bDisplayRecalls) {
			// (a.walling 2013-11-25 11:00) - PLID 60006 - Always returns 1 or 0.
			_RecordsetPtr rsRecall = (prsAllWarnings = prsAllWarnings->NextRecordset(NULL));
			if (VarLong(rsRecall->Fields->Item[0L]->Value)) { //check to see if we have any records
				CMainFrame  *pMainFrame = GetMainFrame();
				//if the recall needing attention dialog is not hiding then don't show, otherwise show it.
				if (pMainFrame != NULL && (pMainFrame->m_pRecallsNeedingAttentionDlg == NULL || 
					!pMainFrame->m_pRecallsNeedingAttentionDlg->IsWindowVisible())) {
					GetMainFrame()->ShowRecallsNeedingAttention(true);
				}
			}
		}

		// (j.luckoski 2013-03-04 13:46) - PLID 33548 - Display warning	
		//Warning #8
		//rewards warning
		_RecordsetPtr rsRewards = (prsAllWarnings = prsAllWarnings->NextRecordset(NULL));
		
		// (a.walling 2014-07-23 08:39) - PLID 62998 - This was calculating reward totals regardless of the conditional
		
		COleCurrency cyRewards = g_ccyZero;
		if (!rsRewards->eof && g_pLicense->CheckForLicense(CLicense::lcNexSpa, CLicense::cflrSilent) && GetRemotePropertyInt("DisplayRewardsWarning", 0, 0, GetCurrentUserName(), true) == 1) {
			cyRewards = Rewards::GetTotalPoints(GetActivePatientID());
		}

		 // (j.luckoski 2013-03-04 13:25) - PLID 33548
		if(cyRewards > g_ccyZero) {
			// Get pointers to useful field objects
			FieldsPtr flds = rsRewards->Fields;
			FieldPtr fldDispWarning = flds->Item["DisplayRewardsWarning"];
			
			CString strRewards = FormatCurrencyForInterface(cyRewards, FALSE, TRUE);
			
			// If the patient is displaying a warning
			BOOL bDispWarning = AdoFldBool(fldDispWarning);
			if (bDispWarning) {

//*
				// TODO: Fix the login instead of putting the band-aid here
				// RAC - I had to put this in because of a design flaw in the Login.  Brad set up the login screen to be 
				// displayed until Practice finishes loading and becomes visible.  The problem is that the login screen 
				// can't be the application's main window once the CMainFrame is loaded or else the views (like the 
				// Patients View) will try to create themselves as MDI children of the login screen instead of the 
				// CMainFrame.  But then we're left with the problem that the userwarning message comes up BEHIND the 
				// login screen and then the program seems to be locked up even though it's really not because you can 
				// always just move the login screen out of the way...so my temporary solution was to just hide the login 
				// screen if and when the userwarning is about to be displayed.  The problem is that userwarnings are not 
				// the only things that bring up modal dialog boxes...AfxMessageBox and other message boxes do too.
				extern CPracticeApp theApp;
				if ((theApp.m_pMainWnd != (CWnd *)theApp.m_dlgLogin) && ((CWnd *)theApp.m_dlgLogin)->GetSafeHwnd()) {
					((CWnd *)theApp.m_dlgLogin)->ShowWindow(SW_HIDE);
				}

				CString strRewardsWarning = "This patient has available reward points:  " +  strRewards;
			


				// Display it
				CUserWarningDlg dlgWarning(this);
				// (a.walling 2010-07-01 16:34) - PLID 18081 - Warning categories
				BOOL bKeepWarning = dlgWarning.DoModalWarning(strRewardsWarning, 
					TRUE, AdoFldString(flds, "FullName") + ":", "Rewards Warning", NULL, -1, "");

				// If the user asked to change the displaywarning status of the patient
				if((bKeepWarning == 0 && bDispWarning != 0) || (bKeepWarning != 0 && bDispWarning == 0)) {
					// Change it
					//fldDispWarning->Value = bKeepWarning?true:false;
					//rsRewards->Update();
					// (a.walling 2009-03-02 11:41) - PLID 33289 - This parameterized recordset does not support updating
					ExecuteSql("UPDATE PersonT SET DisplayRewardsWarning = %li WHERE ID = %li", bKeepWarning?1:0, m_iWarningPatientID);
				
					// Update the view given the change
					CNxTabView::UpdateView();
					// Don't be silly, just update the one checkbox!
//					if (m_pActiveSheet == m_General1Sheet)
				}

				// This is called because the patient warning may steal
				// the focus away from the proper control
				SetDefaultControlFocus();

			}
		}

		//Warning #9 - CDS Interventions //TES 11/4/2013 - PLID 59276
		_RecordsetPtr rsInterventions = (prsAllWarnings = prsAllWarnings->NextRecordset(NULL));
		if(!rsInterventions->eof) {
			//TES 11/4/2013 - PLID 59276 - They have interactions, so pop up the CDS Intervention list and give it their ID
			CCDSInterventionListDlg dlg;
			dlg.OpenWithPatientInterventions(m_iWarningPatientID, AdoFldString(rsInterventions, "FullName"));
		}


		// (c.haag 2006-07-03 16:54) - PLID 19977 - Show patient EMR problem warnings
		// (a.walling 2009-06-05 12:59) - PLID 34496 - Include parent window
		PromptPatientEMRProblemWarning(m_iWarningPatientID, this);

	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
void CPatientView::UpdateView(bool bForceRefresh)
{
	CNxTabView::UpdateView(bForceRefresh);

	CMainFrame *pMainFrame = GetMainFrame();
	
	if(GetActivePatientID() != m_iWarningPatientID) {
		m_iWarningPatientID = GetActivePatientID();

		// (j.jones 2009-06-03 09:57) - PLID 34461 - audit that we're accessing this patient's record
		long nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1) {
			// (a.walling 2009-12-17 17:34) - PLID 35779 - Should be aetOpened, not aetChanged
			AuditEvent(m_iWarningPatientID, GetExistingPatientName(m_iWarningPatientID), nAuditID, aeiViewedPatientRecord, m_iWarningPatientID, "", "Opened record in Patients Module", aepLow, aetOpened);
		}
		
		if (!m_pActiveSheet->GetSafeHwnd()) {// || pPatientSet == NULL) {

			// Tell the view that when it's done painting, we
			// want to see if a patient warning message is to appear	
			SetEvent(m_hCheckPatientWarning);
			return;
		}

		SetColor();
		PromptPatientWarning();
	}
	else {
		//TES 1/6/2010 - PLID 36761 - This used to always do nothing if the patient ID hadn't changed, but it's possible that the patient's
		// status has changed, in which case we need to update the color.
		if(pMainFrame) {
			short nNewStatus = pMainFrame->m_patToolBar.GetActivePatientStatus();
			if(nNewStatus != m_nCurrentStatus) {
				SetColor();
			}
		}
	}

}

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CPatientView::Hotkey (int key)
{
	int nCurrentTab = m_tab->CurSel;
	int nNewTab = nCurrentTab;
	if(IsWindowEnabled()) {
		int nKey = IgnoreRightAlt() ? VK_LMENU : VK_MENU;
		if (GetAsyncKeyState(nKey) & 0xE000)
		{	
			switch (key)
			{	case '1':
					nNewTab = PatientsModule::General1Tab;
					break;
				case '2':
					nNewTab = PatientsModule::General2Tab;
					break;
				case 'C':
					nNewTab = PatientsModule::CustomTab;
					break;
				case 'I':
					if(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrUse)) {
						nNewTab = PatientsModule::ProcedureTab;	
					}
					else{
						nNewTab = nCurrentTab;
					}
					break;
				case 'U':
					nNewTab = PatientsModule::FollowUpTab;
					break;
				case 'N':
					nNewTab = PatientsModule::NotesTab;
					break;
				case 'E':
					if(g_pLicense && g_pLicense->HasEMR(CLicense::cflrUse) == 2) {
						nNewTab = PatientsModule::NexEMRTab;
					}
					else if(g_pLicense && g_pLicense->HasEMR(CLicense::cflrUse) == 1) {
						nNewTab = PatientsModule::CustomRecordsTab;
					}
					else {
						nNewTab = nCurrentTab;
					}
					break;
				case 'S':
					nNewTab = PatientsModule::InsuranceTab;
					break;
				case 'B':
					nNewTab = PatientsModule::BillingTab;
					break;
				case 'Q':
					nNewTab = PatientsModule::QuoteTab;
					break;
				case 'P':
					//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
					if(g_pLicense && g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent)) {
						nNewTab = PatientsModule::AppointmentTab;
					}
					else{
						nNewTab = nCurrentTab;
					}
					break;
				case 'R':
					nNewTab = PatientsModule::HistoryTab;
					break;
				case 'X': // (c.haag 2009-08-19 15:06) - PLID 35231
					if(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcNexPhoto, CLicense::cflrUse)) {
						nNewTab = PatientsModule::NexPhotoTab;
					}
					else{
						nNewTab = nCurrentTab;
					}
					break;
				case 'D':
					nNewTab = PatientsModule::MedicationTab;
					break;
				// (j.gruber 2012-06-25 12:19) - PLID 48702
				// (a.wilson 2012-07-05 11:35) - PLID 51332 - added check for license.
				case 'O':
					if (g_pLicense && g_pLicense->HasEMR(CLicense::cflrUse) == 2) {
						nNewTab = PatientsModule::PatientDashboard;
					}
					break;
				// (a.wilson 2013-10-29 10:24) - PLID 59001 - hidden hotkey for labs since all letters of labs are taken.
				case 'Z':
					if(g_pLicense && g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrUse)) {
						nNewTab = PatientsModule::LabTab;
					}
					else{
						nNewTab = nCurrentTab;
					}
					break;
				default:
					nNewTab = -1;
					break;
			}

			if (nNewTab != -1 && nCurrentTab != nNewTab) {
				SetActiveTab(nNewTab);
				return 0;
			}
		}
		else if (GetAsyncKeyState(VK_CONTROL) & 0xE000)//Ctrl is down
		{
			switch (key)
			{
				case 'D':
				//TES 1/6/2010 - PLID 36761 - New accessor for dropdown state.
				GetMainFrame()->m_patToolBar.SetDroppedDown(TRUE);
				return 0;
			}
		}
	}

	return CNxTabView::Hotkey(key);
}

void CPatientView::Delete (void)
{
}

void CPatientView::OnSuperbill() 
{
	try {
		SuperBill (GetActivePatientID());
		// (j.jones 2012-08-08 10:11) - PLID 51063 - added check that the active view is non-null
		if(GetMainFrame()->GetActiveView()) {
			GetMainFrame()->GetActiveView()->UpdateView();
		}
	}NxCatchAll(__FUNCTION__);
}

BOOL CPatientView::OnPreparePrinting(CPrintInfo* pInfo) 
{

	try {
		CString str, 
				sql, 
				sCategory; // used in Notes tab
		
		int PatientID;
		int nResult;

		CAdvPrintOptions		PrintOption(this);

		CPtrArray			paParam;
		CRParameterInfo*	ParamInfo;

		COleDateTime dateTmp, dateTmp2;
		_RecordsetPtr rs;
		
		PatientID = GetActivePatientID();

		CNxDialog* ptrCurrentTab = GetActiveSheet();

		//TODO: This is sort of a work-around for the fact that the EN_KILLFOCUS message is not sent until _after_ the
		//Crystal report is generated, meaning that if you change a field and immediately print (not print preview) the
		//report, the change will not be reflected.  However, this solution is less than ideal.
		
		//TES 2/9/2004: General 1 now does this in a more correct way, which was actually being sabotaged by this very line
		//of code.
		if(ptrCurrentTab != &m_General1Sheet)
			ptrCurrentTab->StoreDetails();	

		if (ptrCurrentTab == &m_General1Sheet || ptrCurrentTab == &m_General2Sheet) {
			
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(223)]);
			infReport.nPatient = PatientID;
			
			ParamInfo = new CRParameterInfo;
			ParamInfo->m_Name = "NextAppointment";
			COleDateTime dtNextAppt = GetNextAppt (COleDateTime::GetCurrentTime(), PatientID);
			if(dtNextAppt){
				ParamInfo->m_Data = FormatDateTimeForInterface(dtNextAppt, DTF_STRIP_SECONDS);
			}
			else{
				ParamInfo->m_Data = "No appointment scheduled.";
			}
			paParam.Add ((void *)ParamInfo);


			//Made new function for running reports - JMM 5-28-04
			RunReport(&infReport, &paParam, pInfo->m_bPreview, (CWnd *)this, "General Patient Information", pInfo);
			ClearRPIParameterList(&paParam);	//DRT - PLID 18085 - Cleanup after ourselves

			return FALSE;

		} else if (ptrCurrentTab == &m_NotesSheet) {
			try{
				// set up Printing Options
				if (pInfo->m_bPreview) PrintOption.m_btnCaption = "Preview";
				else PrintOption.m_btnCaption = "Print";
				PrintOption.m_bDateRange = true;
				PrintOption.m_bAllOptions = true;
				PrintOption.m_bDetailed = false;
				PrintOption.m_bOptionCombo = true;
				PrintOption.m_cBoundCol		 = 1;
				PrintOption.m_cAllCaption	 = "All Categories";
				PrintOption.m_cSingleCaption = "One Category";

				// set up from date
				rs = CreateRecordset("SELECT Min(Notes.Date) AS FromDate FROM Notes WHERE (((Notes.PersonID)=%li))", PatientID);
				//when you select the Min of a value, if there are no records, it will return a VT_NULL variant as the only record
				//so you can't use the rs->eof flag to filter on this
				if (rs->Fields->Item["FromDate"]->Value.vt == VT_NULL) {
					MessageBox("No Notes for this Patient");
					rs->Close();
					return FALSE;
				}
				rs->MoveFirst();
				dateTmp = AdoFldDateTime(rs, "FromDate");
				PrintOption.m_dtInitFrom = dateTmp;
				rs->Close();

				// set up to date
				rs = CreateRecordset("SELECT Max(Notes.Date) AS ToDate FROM Notes WHERE (((Notes.PersonID)=%li));", PatientID);
				//when you select the Min of a value, if there are no records, it will return a VT_NULL variant as the only record
				//so you can't use the rs->eof flag to filter on this
				if (rs->Fields->Item["ToDate"]->Value.vt == VT_NULL) {
					MessageBox("No Notes for this Patient");
					rs->Close();
					return FALSE;
				}
				rs->MoveFirst();
				dateTmp = AdoFldDateTime(rs, "ToDate");
				PrintOption.m_dtInitTo = dateTmp;
				rs->Close();

				//set up the datalist
				PrintOption.m_cSQL = "(SELECT ID, Description AS Text FROM NoteCatsF) as CatQ";

			}NxCatchAll("Error in CPatientView::OnPreparePrinting");
		
			nResult = PrintOption.DoModal();

			if (nResult == IDOK) {	//run the report with the date / category filter
		
				COleDateTime  dateTo, dateFrom;
				CString ToDate, FromDate;

				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(240)]);
				//add the category filter via the Extra filter
				infReport.nExtraID = PrintOption.m_nResult;
				//filter on the patient
				infReport.nPatient = PatientID;
				//filter on the dates chosen
				if(PrintOption.m_bDateRange)
				{
					infReport.strDateOptions = "1;Note Date;Date;;";
					infReport.nDateFilter = 1;
					infReport.nDateRange = 2;
					infReport.DateFrom = PrintOption.m_dtFromDate;
					infReport.DateTo   = PrintOption.m_dtToDate;
				}

				//Made new function for running reports - JMM 5-28-04
				// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
				RunReport(&infReport, pInfo->m_bPreview, (CWnd *)this, "Patient Notes", pInfo, TRUE);

				
			}

			return FALSE;
			

		} else if (ptrCurrentTab == &m_InsuranceSheet) { 

				
				CString strSQL;
				
				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(224)]);
				infReport.nPatient = PatientID;

				//Made new function for running reports - JMM 5-28-04
				RunReport(&infReport, pInfo->m_bPreview, (CWnd *)this, "Patient Insurance Information", pInfo);
							
			return FALSE;

		} else if (ptrCurrentTab == &m_AppointmentsSheet) {

			_RecordsetPtr rsTmp;
			
			// set up Printing Options
			if (pInfo->m_bPreview) PrintOption.m_btnCaption = "Preview";
			else PrintOption.m_btnCaption = "Print";
			PrintOption.m_bDateRange = true;
			PrintOption.m_bAllOptions = false;
			PrintOption.m_bDetailed = false;
			PrintOption.m_bOptionCombo = false;

			// set up from date
			sql.Format ("SELECT Min(AppointmentsT.Date) AS FromDate FROM AppointmentsT WHERE (((AppointmentsT.PatientID)=%d));", PatientID);
			rsTmp = CreateRecordsetStd(sql);
			//when you select the Min of a value, if there are no records, it will return a VT_NULL variant as the only record
			//so you can't use the rsTmp->eof flag to filter on this
			if (rsTmp->Fields->Item["FromDate"]->Value.vt == VT_NULL) {
				MessageBox("No Appointments for this Patient");
				rsTmp->Close();
				return FALSE;
			}
			dateTmp = AdoFldDateTime(rsTmp, "FromDate");
			PrintOption.m_dtInitFrom = dateTmp;
			rsTmp->Close();

			// set up to date
			sql.Format ("SELECT Max(AppointmentsT.Date) AS ToDate FROM AppointmentsT WHERE (((AppointmentsT.PatientID)=%d));", PatientID);
			rsTmp = CreateRecordsetStd(sql);
			//when you select the Min of a value, if there are no records, it will return a VT_NULL variant as the only record
			//so you can't use the rsTmp->eof flag to filter on this
			if (rsTmp->Fields->Item["ToDate"]->Value.vt == VT_NULL) {
				MessageBox("No Appointments for this Patient");
				rsTmp->Close();
				return FALSE;
			}
			dateTmp = AdoFldDateTime(rsTmp, "ToDate");
			PrintOption.m_dtInitTo = dateTmp;
			rsTmp->Close();

			nResult = PrintOption.DoModal();
		
			if (nResult == IDOK) {
							
				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(222)]);
		
				if(PrintOption.m_bDateRange)
				{
					infReport.nPatient = PatientID;
					infReport.strDateOptions = "1;Appt. Date;Date;;";
					infReport.nDateFilter = 1;
					infReport.nDateRange = 2;
					infReport.DateFrom = PrintOption.m_dtFromDate;
					infReport.DateTo   = PrintOption.m_dtToDate;
				}
				
				//Made new function for running reports - JMM 5-28-04
				// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
				RunReport(&infReport, pInfo->m_bPreview, (CWnd *)this, "Patient Appointments", pInfo, PrintOption.m_bDateRange);
			}		
								
			return FALSE;

		} else if (ptrCurrentTab == &m_FinancialSheet) {

			//DRT 3/13/03 - To avoid duplicating code, I moved all the code from here to main frame, so it can be called from both places the code
			//			is currently duplicated
			GetMainFrame()->PrintHistoryReport(pInfo->m_bPreview, pInfo);

			return FALSE;

		} else if (ptrCurrentTab == &m_FollowUpSheet) {

			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(110)]);
			infReport.nPatient = PatientID;

			if(m_FollowUpSheet.m_completed.GetCheck())	//we're filtering completed items
				infReport.nExtraID = 2;
			else if (m_FollowUpSheet.m_incomplete.GetCheck())	//filtering incomplete items
				infReport.nExtraID = 1;
			else
				infReport.nExtraID = -1;

			_RecordsetPtr rsTmp;
			
			// set up Printing Options
			if (pInfo->m_bPreview) PrintOption.m_btnCaption = "Preview";
			else PrintOption.m_btnCaption = "Print";
			PrintOption.m_bDateRange = true;
			PrintOption.m_bAllOptions = false;
			PrintOption.m_bDetailed = false;
			PrintOption.m_bOptionCombo = false;

			// set up from date
			sql.Format ("SELECT Min(ToDoList.Deadline) AS FromDate FROM ToDoList WHERE (((ToDoList.PersonID)=%d));", PatientID);
			rsTmp = CreateRecordsetStd(sql);
			//when you select the Min of a value, if there are no records, it will return a VT_NULL variant as the only record
			//so you can't use the rsTmp->eof flag to filter on this
			if (rsTmp->Fields->Item["FromDate"]->Value.vt == VT_NULL) {
				MessageBox("No todo's for this Patient");
				rsTmp->Close();
				return FALSE;
			}
			dateTmp = AdoFldDateTime(rsTmp, "FromDate");
			PrintOption.m_dtInitFrom = dateTmp;
			rsTmp->Close();

			// set up to date
			sql.Format ("SELECT Max(ToDoList.Deadline) AS ToDate FROM ToDoList WHERE (((ToDoList.PersonID)=%d));", PatientID);
			rsTmp = CreateRecordsetStd(sql);
			//when you select the Min of a value, if there are no records, it will return a VT_NULL variant as the only record
			//so you can't use the rsTmp->eof flag to filter on this
			if (rsTmp->Fields->Item["ToDate"]->Value.vt == VT_NULL) {
				MessageBox("No to do's for this Patient");
				rsTmp->Close();
				return FALSE;
			}
			dateTmp = AdoFldDateTime(rsTmp, "ToDate");
			PrintOption.m_dtInitTo = dateTmp;
			rsTmp->Close();

			nResult = PrintOption.DoModal();
			
			if (nResult == IDOK) {
							
				if(PrintOption.m_bDateRange)
				{
					infReport.strDateOptions = "1;Deadline;Date;;";
					infReport.nDateFilter = 1;
					infReport.nDateRange = 2;
					infReport.DateFrom = PrintOption.m_dtFromDate;
					infReport.DateTo   = PrintOption.m_dtToDate;
				}
				
				//Made new function for running reports - JMM 5-28-04
				// (j.camacho 2014-10-21 12:34) - PLID 62716 - need to specify to let us set dates
				RunReport(&infReport, pInfo->m_bPreview, (CWnd *)this, "Patient ToDo List", pInfo, TRUE);
			}		
			
			return FALSE;

		}
		else if (ptrCurrentTab == &m_QuotesSheet) {
			// Quote List (PP)
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(574)]);
			infReport.nPatient = PatientID;

			//Made new function for running reports - JMM 5-28-04
			RunReport(&infReport, pInfo->m_bPreview, (CWnd *)this, "Quote List", pInfo);
			
			return FALSE;
		}
		// (d.singleton 2012-05-17 15:52) - PLID 50471 check to see if its the pre op calender,  if so proceed
		else if (ptrCurrentTab == &m_MedicationSheet || m_bIsPicPreOpCalendar) {

			// (d.singleton 2012-05-17 15:52) - PLID 50471 check to see if its NOT the pre op calender,  if not proceed
			if(!m_MedicationSheet.m_bIsMedSchedPrinting && !m_bIsPicPreOpCalendar) {
				
				CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(444)]);
				infReport.nPatient = PatientID;

				//Made new function for running reports - JMM 5-28-04
				RunReport(&infReport, pInfo->m_bPreview, (CWnd *)this, "Medications Tab", pInfo);
				
				return FALSE;
			}

			// (d.singleton 2012-11-01 11:38) - PLID 53409 Pre-op Calendar only prints the first page,  was calling the wrong function when it was a pre op
			// calendar		
			if(m_MedicationSheet.m_bIsMedSchedPrinting) {
				ptrCurrentTab->OnBeginPrinting(NULL, pInfo);
			}
			else if(m_bIsPicPreOpCalendar) {
				OnBeginPreOpCalendarPrinting(NULL, pInfo);
			}

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
			
			return TRUE;	

		} else if(ptrCurrentTab == &m_HistorySheet) {
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(407)]);
			infReport.nPatient = PatientID;

			//Tell the report we're in the patients module.
			CRParameterInfo *paramInfo = new CRParameterInfo;
			CPtrArray paParams;
			paramInfo->m_Data = "Patient History";
			paramInfo->m_Name = "ReportTitle";
			paParams.Add((void *)paramInfo);
			// (j.jones 2010-04-28 14:20) - PLID 35591 - this report now has a date range, not used in the preview
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "01/01/1000";
			paramInfo->m_Name = "DateFrom";
			paParams.Add(paramInfo);
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "12/31/5000";
			paramInfo->m_Name = "DateTo";
			paParams.Add(paramInfo);

			//Made new function for running reports - JMM 5-28-04
			RunReport(&infReport, &paParams, pInfo->m_bPreview, (CWnd *)this, "History Tab", pInfo);
			ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves
			
			return FALSE;

		} else if(ptrCurrentTab == &m_RefractiveSheet) {
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(571)]);
			infReport.nPatient = PatientID;

			//Made new function for running reports - JMM 5-28-04
			RunReport(&infReport, NULL, pInfo->m_bPreview, (CWnd *)this, "Patient Outcomes", pInfo);
			//ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves
			
			return FALSE;
		} else if(ptrCurrentTab == &m_LabSheet) {
			CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(568)]);
			infReport.nPatient = PatientID;

			//Tell the report we're in the contacts module.
			CRParameterInfo *paramInfo = new CRParameterInfo;
			CPtrArray paParams;
			paramInfo->m_Data = "01/01/1000";
			paramInfo->m_Name = "DateFrom";
			paParams.Add((void *)paramInfo);
			paramInfo = new CRParameterInfo;
			paramInfo->m_Data = "01/01/1000";
			paramInfo->m_Name = "DateTo";
			paParams.Add((void *)paramInfo);

			//Made new function for running reports - JMM 5-28-04
			RunReport(&infReport, &paParams, pInfo->m_bPreview, (CWnd *)this, "Patient Labs", pInfo);
			ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves

			return FALSE; // everyone else is
		} else if(ptrCurrentTab == &m_NexEMRSheet) {
			PrintOption.m_btnCaption = "Print";
			PrintOption.m_bDetailed = true; // get whether we use detailed or summary
			PrintOption.m_bDateRange = false;
			PrintOption.m_bAllOptions = false;
			PrintOption.m_bOptionCombo = false;
			PrintOption.m_strSpecialCaption = "Print / Preview M&ultiple EMNs";
			int nResult = PrintOption.DoModal();
			// (a.walling 2009-11-24 14:47) - PLID 36418
			if (nResult == 15 /*ID_SPECIAL*/) {
				m_NexEMRSheet.PrintMultipleEMNs();
			} else if ( (nResult == 0 /*ID_DETAILED*/ ) || (nResult == 1 /*ID_SUMMARY*/) ) {
				CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(576)]);
				infReport.nPatient = PatientID;
				infReport.nDetail = (nResult + 1); // {ID_DETAILED, ID_SUMMARY} == {0,1}
												   // {Disabled, Detailed, Summary} == {0,1,2}
												   // Therefore, Result + 1 gives us the desired num
				if (nResult == 0) { //Detailed
					infReport.strReportFile += "Dtld";
				}
				else if (nResult == 1) { //Summary
					infReport.strReportFile += "Smry";
				}
				/*
				//Tell the report we're in the contacts module.
				CRParameterInfo *paramInfo = new CRParameterInfo;
				CPtrArray paParams;
				paramInfo->m_Data = "01/01/1000";
				paramInfo->m_Name = "DateFrom";
				paParams.Add((void *)paramInfo);
				*/

				//Made new function for running reports - JMM 5-28-04
				RunReport(&infReport, pInfo->m_bPreview, (CWnd *)this, "NexEMR", pInfo);
				//ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves
			}
		}
		else {
			return CNxTabView::OnPreparePrinting(pInfo);
		}


	}NxCatchAll("Error in CPatientView::OnPreparePrinting()");
	return FALSE;

}

void CPatientView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CView::OnUpdate(pSender, lHint, pHint);

	if (m_CurrentTab == 7) {
		UpdateView();
	}
}

void CPatientView::OnSelectTab(short newTab, short oldTab)//used for the new NxTab
{	
	// (a.walling 2008-05-07 13:56) - PLID 29951 - Do default processing
	HandleSelectTab(newTab, oldTab);

	// (a.walling 2008-05-07 13:56) - PLID 29951 - If we are switching tabs to or from the NexEMR tab,
	// ensure the toolbar is up to date.
	// (z.manning 2015-03-13 15:15) - NX-100432 - Dashboard too
	if (newTab == PatientsModule::NexEMRTab || oldTab == PatientsModule::NexEMRTab ||
		newTab == PatientsModule::PatientDashboard || oldTab == PatientsModule::PatientDashboard)
	{
		GetMainFrame()->UpdateToolBarButtons(false);
	}
}

// (a.walling 2008-05-07 13:56) - PLID 29951 - Called from OnSelectTab; handles most everything.
void CPatientView::HandleSelectTab(short newTab, short oldTab)
{
	//TES 10/29/03: If this tab is being selected by the initialization of the module, 
	//which will be the case if newTab == oldTab, then silently see if they have read permissions,
	//and if they don't, send them back to the general 1 tab.
	// (z.manning, 04/16/2008) - PLID 29680 - Removed references to obsolete 640x480 dialogs

	if (newTab == PatientsModule::InsuranceTab)
	{	
		if(oldTab == PatientsModule::InsuranceTab) {
			if(!(GetCurrentUserPermissions(bioPatientInsurance) & SPT__R________)) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioPatientInsurance,sptRead))
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	else if (newTab == PatientsModule::BillingTab)
	{	
		if(oldTab == PatientsModule::BillingTab) {
			if(!(GetCurrentUserPermissions(bioPatientBilling) & SPT__R________) || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrUse)) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioPatientBilling,sptRead) || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrUse))
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	else if (newTab == PatientsModule::QuoteTab)
	{	
		if(oldTab == PatientsModule::QuoteTab) {
			if(!(GetCurrentUserPermissions(bioPatientQuotes) & SPT__R________) || !g_pLicense->CheckForLicense(CLicense::lcQuotes, CLicense::cflrUse)) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioPatientQuotes,sptRead) || !g_pLicense->CheckForLicense(CLicense::lcQuotes, CLicense::cflrUse))
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	else if (newTab == PatientsModule::MedicationTab)
	{	
		if(oldTab == PatientsModule::MedicationTab) {
			if(!(GetCurrentUserPermissions(bioPatientMedication) & SPT__R________)) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioPatientMedication,sptRead))
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	else if (newTab == PatientsModule::CustomRecordsTab)
	{
		if(g_pLicense->HasEMR(CLicense::cflrSilent) < 1) {
			//we don't have license for this level of EMR!
			OnSelectTab(PatientsModule::General1Tab, PatientsModule::General1Tab);
			m_tab->CurSel = PatientsModule::General1Tab;
			return;
		}

		if(oldTab == PatientsModule::CustomRecordsTab) {
			if(!(GetCurrentUserPermissions(bioPatientEMR) & SPT__R________)) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioPatientEMR,sptRead))
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	else if (newTab == PatientsModule::NexEMRTab)
	{
		//DRT 1/20/2005 - PLID 15340 - EMR usage counts must be checked!
		// (a.walling 2007-11-28 09:57) - PLID 28044
		long nEMRUsage = g_pLicense->HasEMROrExpired(CLicense::cflrUse);

		if(nEMRUsage == 2) {
			//EMR L2 (NexEMR) is active and we can use it (said Yes to trial or has full functionality)
		}
		else {
			//If the old tab is also the emr tab (this can happen if you have NexEMR set as your default tab and first load the module)
			//	we'll have to revert them to General1
			if(oldTab == PatientsModule::NexEMRTab)
				oldTab = PatientsModule::General1Tab;

			//0 = Not allowed or they said "No" when asked if they wanted to use a trial.  Since we already made it into this
			//	tab, it's most likely the latter.  We need to move them back out of this tab.
			//1 = Has EMR L1 (Custom Records) License.  They shouldn't be here, they should be in the case above for
			//	'CustomRecordsTab'.
			//In either instance, we refuse access and send 'em packing.
			OnSelectTab(oldTab, oldTab);
			m_tab->CurSel = oldTab;
			return;
		}

		if(oldTab == PatientsModule::NexEMRTab) {
			if(!(GetCurrentUserPermissions(bioPatientEMR) & SPT__R________)) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioPatientEMR,sptRead))
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	else if (newTab == PatientsModule::ProcedureTab)
	{	
		if(oldTab == PatientsModule::ProcedureTab) {
			if(!(GetCurrentUserPermissions(bioPatientTracking) & SPT__R________) || !g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrUse)) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioPatientTracking,sptRead))
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	else if (newTab == PatientsModule::NotesTab)
	{	
		if(oldTab == PatientsModule::NotesTab) {
			if(!(GetCurrentUserPermissions(bioPatientNotes) & SPT__R________)) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioPatientNotes,sptRead))
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	else if (newTab == PatientsModule::HistoryTab)
	{	
		if(oldTab == PatientsModule::HistoryTab) {
			if(!(GetCurrentUserPermissions(bioPatientHistory) & SPT__R________)) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!CheckCurrentUserPermissions(bioPatientHistory,sptRead))
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	else if (newTab == PatientsModule::LabTab)
	{
		//PLID 21449 - make sure the labs tab is visible
		if(oldTab == PatientsModule::LabTab) {
			if (m_tab->ShowTab[PatientsModule::LabTab] == FALSE || !(GetCurrentUserPermissions(bioPatientLabs) & sptRead)) {
				//take 'em outta here
				OnSelectTab(PatientsModule::General1Tab, PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}

		if(!CheckCurrentUserPermissions(bioPatientLabs, sptRead)) {
			m_tab->CurSel = oldTab;
			return;
		}
	}
	else if(newTab == PatientsModule::RefractiveTab)
	{
		if(oldTab == PatientsModule::RefractiveTab) {
			if(!IsRefractive()) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!IsRefractive())
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	else if(newTab == PatientsModule::SalesTab)
	{
		if(oldTab == PatientsModule::SalesTab) {
			if(!IsNexTechInternal()) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!IsNexTechInternal())
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	else if(newTab == PatientsModule::SupportTab)
	{
		if(oldTab == PatientsModule::SupportTab) {
			if(!IsNexTechInternal()) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!IsNexTechInternal())
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	// (j.gruber 2007-11-07 11:49) - PLID 28023 - added implementation tab
	else if(newTab == PatientsModule::ImplementationTab)
	{
		if(oldTab == PatientsModule::ImplementationTab) {
			if(!IsNexTechInternal()) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		}
		if (!IsNexTechInternal())
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	// (c.haag 2009-11-13 10:47) - PLID 36180 - Permission checking
	else if(newTab == PatientsModule::NexPhotoTab)
	{
		if(oldTab == PatientsModule::NexPhotoTab) {
			// (c.haag 2009-11-19 16:38) - PLID 35231 - Also license checking
			// (a.walling 2014-04-25 16:00) - VS2013 - We definitely don't support Win2k any longer
			if(!(GetCurrentUserPermissions(bioPatientNexPhoto) & SPT__R________) || !g_pLicense->CheckForLicense(CLicense::lcNexPhoto, CLicense::cflrUse)) {
				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
			
		}
		if (!CheckCurrentUserPermissions(bioPatientNexPhoto,sptRead))
		{	m_tab->CurSel = oldTab;
			return;
		}
	}
	// (j.gruber 2012-06-19 15:46) - PLID 51067
	// (a.wilson 2012-07-03 12:13) - PLID 51332 - adding same checks as EMR Has. Also have check for IE Version >= 8.
	else if (newTab == PatientsModule::PatientDashboard)
	{
		//check licensing, IE (Silently to prevent a message from default tab preference), Permissions.
		if (oldTab == PatientsModule::PatientDashboard) {
			if (!(g_pLicense->HasEMR(CLicense::cflrUse) == 2) || 
			!CheckIEVersion(TRUE) || 
			!(GetCurrentUserPermissions(bioPatientEMR, sptRead))) {

				OnSelectTab(PatientsModule::General1Tab,PatientsModule::General1Tab);
				m_tab->CurSel = PatientsModule::General1Tab;
				return;
			}
		} else {
			if (!(g_pLicense->HasEMR(CLicense::cflrUse) == 2) || 
			!CheckIEVersion() || 
			!(CheckCurrentUserPermissions(bioPatientEMR, sptRead))) {

				m_tab->CurSel = oldTab;
				return;
			}
		}
	}
	CNxTabView::OnSelectTab(newTab, oldTab);
	SetColor();
}

void CPatientView::OnPatGroupDlg() 
{
	CPatientGroupsDlg dlg(this);
	dlg.m_bAutoWriteToData = TRUE;
	dlg.m_nPatID = GetActivePatientID();
	dlg.DoModal();
}


void CPatientView::OpenBill(int nBillID)
{	
	// (a.walling 2009-12-22 17:27) - PLID 7002 - Maintain only one instance of a bill
	if (GetMainFrame()->IsBillingModuleOpen(true)) {
		return;
	}
	SetActiveTab(PatientsModule::BillingTab);
	m_BillingDlg->m_boAskForNewPayment = FALSE;
	m_BillingDlg->OpenWithBillID(nBillID, BillEntryType::Bill, 2);
}


void CPatientView::OpenQuote(int nQuoteID)
{
	SetActiveTab(PatientsModule::QuoteTab);
	CBillingModuleDlg dlg(this);
	dlg.m_boAskForNewPayment = FALSE;
	dlg.OpenWithBillID(nQuoteID, BillEntryType::Quote, 2);
}


void CPatientView::OpenLetter(int nMailID)
{
	SetActiveTab(PatientsModule::HistoryTab);
	m_HistorySheet.OpenDocumentByID(nMailID);
}

void CPatientView::OpenPacket(int nMergedPacketID)
{
	SetActiveTab(PatientsModule::HistoryTab);
	m_HistorySheet.OpenPacket(nMergedPacketID);
}

void CPatientView::OpenPayment(int nPayID)
{
	SetActiveTab(PatientsModule::BillingTab);
	m_FinancialSheet.OpenPayment(nPayID);
}

void CPatientView::OpenEMR(int nEmrID)
{
	if (g_pLicense && g_pLicense->HasEMR(CLicense::cflrUse) == 2) {
		// (z.manning 2015-05-06 11:08) - NX-100433 - Now have a preference to determine which tab to use
		SetActiveTab(GetMainFrame()->GetPrimaryEmrTab());
		long nPicID = VarLong(GetTableField("(SELECT EmrMasterT.ID, PicT.ID AS PicID FROM (SELECT * FROM EmrMasterT WHERE Deleted = 0) AS EmrMasterT INNER JOIN PicT ON EmrMasterT.EmrGroupID = PicT.EmrGroupID) PicQ", "PicID", "ID", nEmrID));
		GetMainFrame()->EditEmrRecord(nPicID, nEmrID);
	}
}

void CPatientView::SetDefaultControlFocus()
{
	CNxEdit* pEdit;

	// If the current sheet is General 1, give the notes field
	// focus
	if (m_pActiveSheet == &m_General1Sheet)
	{
		pEdit = (CNxEdit*)m_General1Sheet.GetDlgItem(IDC_NOTES);
		pEdit->SetFocus();
		// This is a trick to make the cursor appear at the end
		// of the edit box
		pEdit->SetSel(0,-1);
		pEdit->SetSel(-1,-1);
	}
	// If the current sheet is General 2, give the occupation
	// field focus
	else if (m_pActiveSheet == &m_General2Sheet)
	{
		pEdit = (CNxEdit*)m_General2Sheet.GetDlgItem(IDC_OCCUPATION);
		pEdit->SetFocus();
		// This is a trick to make the cursor appear at the end
		// of the edit box
		pEdit->SetSel(0,-1);
		pEdit->SetSel(-1,-1);
	}
	else
	{
		CNxTabView::SetDefaultControlFocus();
	}
}

void CPatientView::OnDraw(CDC* pDC) 
{
	CNxTabView::OnDraw(pDC);

	// (a.walling 2009-06-05 12:54) - PLID 34496 - Post a message to handle this during next pump
	PostMessage(NXM_POST_ONDRAW);

	/*
	// Display the patient warning if the view was just created
	if (m_hCheckPatientWarning && WaitForSingleObject(m_hCheckPatientWarning, 0) == WAIT_OBJECT_0)
	{
		ResetEvent(m_hCheckPatientWarning);
		PromptPatientWarning();
	}
	// if they have the preference set to auto-drop the list and we haven't dropped the list yet, then 
	// drop it
	if(m_bNeedToDropPatientList && GetRemotePropertyInt("AutoDropPatList",0,0,GetCurrentUserName(),TRUE) == 1){
		GetMainFrame()->m_patToolBar.m_toolBarCombo->DropDownState = true;
	}
	m_bNeedToDropPatientList = false;
	*/
}

void CPatientView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	
	CNxDialog *pActiveSheet = GetActiveSheet();

	if(pActiveSheet == &m_MedicationSheet) {
		pActiveSheet->Print(pDC, pInfo);
	}
	else if(m_bIsPicPreOpCalendar) {
		this->PrintPreOpCalendar(pDC, pInfo);
	}
	else {
		CNxTabView::OnPrint(pDC, pInfo);
	}
}

void CPatientView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	CNxDialog *pActiveSheet = GetActiveSheet();

	if(pActiveSheet == &m_MedicationSheet) {		
		pActiveSheet->OnBeginPrinting(pDC, pInfo);
	}
	// (d.singleton 2012-11-01 15:29) - PLID 53409 Pre-op Calendar only prints the first page
	else if(m_bIsPicPreOpCalendar) {
		this->OnBeginPreOpCalendarPrinting(pDC, pInfo);
	}
	else {
		pInfo->SetMinPage(1);
		pInfo->SetMaxPage(1);
	}
	
	CNxTabView::OnBeginPrinting(pDC, pInfo);
}

void CPatientView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo) 
{
	CNxTabView::OnEndPrinting(pDC, pInfo);
}

// (j.gruber 2012-06-11 16:01) - PLID 50225 - need tablechangedex for appointments
LRESULT CPatientView::OnTableChangedEx(WPARAM wParam, LPARAM lParam) {

	try {

		//TES 8/14/2014 - PLID 63520 - PatCombo now has an EX tablechecker, handle it the same way as the regular one.
		if (wParam == NetUtils::PatCombo) {
			CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
			long nPatientID = VarLong(pDetails->GetDetailData(CClient::pcdiPersonID), -1);
			if (nPatientID == -1 || nPatientID == GetActivePatientID()) {

				m_General1Sheet.SetForceRefresh();
				m_General2Sheet.SetForceRefresh();
				m_CustomSheet.SetForceRefresh();
				m_InsuranceSheet.SetForceRefresh();
				m_NotesSheet.SetForceRefresh();
				m_FollowUpSheet.SetForceRefresh();
				m_AppointmentsSheet.SetForceRefresh();
				m_HistorySheet.SetForceRefresh();
				m_MedicationSheet.SetForceRefresh();
				m_FinancialSheet.SetForceRefresh();
				m_QuotesSheet.SetForceRefresh();
				m_ProcedureSheet.SetForceRefresh();
				m_CustomRecordsSheet.SetForceRefresh();
				m_NexEMRSheet.SetForceRefresh();
				m_SupportSheet.SetForceRefresh();
				m_SalesSheet.SetForceRefresh();
				m_RefractiveSheet.SetForceRefresh();
				m_LabSheet.SetForceRefresh();
				m_ImplementationSheet.SetForceRefresh();
				m_NexPhotoSheet.SetForceRefresh();
				// (j.gruber 2012-06-15 20:49) - PLID 48702 - dashboard
				m_PatientDashboardSheet.SetForceRefresh();
			}
		}

		// (j.jones 2014-08-08 10:39) - PLID 63232 - if the Patients module is not active,
		// do nothing, because each tab properly updates itself when it becomes active again
		if (GetMainFrame() && !GetMainFrame()->IsActiveView(PATIENT_MODULE_NAME)) {
			//the patients module is not active, so don't bother updating the active tab
			return 0;
		}

		if (m_pActiveSheet) {
			return m_pActiveSheet->SendMessage(WM_TABLE_CHANGED_EX, wParam, lParam);
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

LRESULT CPatientView::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		// (c.haag 2004-06-01 13:53) - We used to send TC messages to sheets
		// based on what the TC message was. Now we indiscriminately send
		// them to all sheets.

		
		// (a.walling 2010-12-27 09:14) - PLID 40908 - If this is for the active patient, we should ensure the view will be refreshed on the next updateview
		bool bNeedSetRefresh = false;

		switch (wParam) {
			case NetUtils::PatG1:
			case NetUtils::PatCombo:
				{
					long nPatientID = (long)lParam;
					if (nPatientID == -1 || nPatientID == GetActivePatientID()) {
						bNeedSetRefresh = true;
					}
				}
				break;
		}

		// (a.walling 2010-12-27 09:14) - PLID 40908 - Why these are not in a collection of NexTechDialog or NxDialog, I do not know.
		// so instead of for_each(tabs.begin(), tabs.end(), bind(&CNexTechDialog::SetForceRefresh, _1, true)) I'll have to do this:
		if (bNeedSetRefresh) {
			m_General1Sheet.SetForceRefresh();
			m_General2Sheet.SetForceRefresh();
			m_CustomSheet.SetForceRefresh();
			m_InsuranceSheet.SetForceRefresh();
			m_NotesSheet.SetForceRefresh();
			m_FollowUpSheet.SetForceRefresh();
			m_AppointmentsSheet.SetForceRefresh();
			m_HistorySheet.SetForceRefresh();
			m_MedicationSheet.SetForceRefresh();
			m_FinancialSheet.SetForceRefresh();
			m_QuotesSheet.SetForceRefresh();
			m_ProcedureSheet.SetForceRefresh();
			m_CustomRecordsSheet.SetForceRefresh();
			m_NexEMRSheet.SetForceRefresh();
			m_SupportSheet.SetForceRefresh();
			m_SalesSheet.SetForceRefresh();
			m_RefractiveSheet.SetForceRefresh();
			m_LabSheet.SetForceRefresh();
			m_ImplementationSheet.SetForceRefresh();
			m_NexPhotoSheet.SetForceRefresh();
			// (j.gruber 2012-06-15 20:49) - PLID 48702 - dashboard
			m_PatientDashboardSheet.SetForceRefresh();
		}

		// (j.jones 2014-08-08 10:39) - PLID 63232 - if the Patients module is not active,
		// do nothing, because each tab properly updates itself when it becomes active again
		if (GetMainFrame() && !GetMainFrame()->IsActiveView(PATIENT_MODULE_NAME)) {
			//the patients module is not active, so don't bother updating the active tab
			return 0;
		}

		// (a.walling 2010-12-27 09:14) - PLID 40908 - Now, just send the table changed message to the active sheet.
		if (m_pActiveSheet) {
			return m_pActiveSheet->SendMessage(WM_TABLE_CHANGED, wParam, lParam);
		}

		return 0;

	} NxCatchAll(__FUNCTION__);

	return 0;
}

LRESULT CPatientView::OnImageProcessingCompleted(WPARAM wParam, LPARAM lParam)
{
	if (GetActiveTab() == PatientsModule::General1Tab)
		m_General1Sheet.SendMessage(NXM_G1THUMB_PROCESSING_COMPLETE, wParam, lParam);
	return 0;
}

// (j.jones 2015-03-16 15:08) - PLID 64926 - renamed to reflect that this
// hides all billing tool windows
LRESULT CPatientView::OnHideBillingToolWindows(WPARAM wParam, LPARAM lParam)
{
	if(GetActiveTab() == PatientsModule::BillingTab)
		m_FinancialSheet.HideToolWindows();

	return 0;
}

void CPatientView::OnInitialUpdate()
{
	CNxTabView::OnInitialUpdate();
	
	//If this isn't a refractive license, don't show the Eye Graph menu.
	if(!IsRefractive()) {
		RemoveEntryFromSubMenu(GetMainFrame()->GetMenu(), "Activities", ID_ACTIVITIES_EYEGRAPH, FALSE);
	}
}

void CPatientView::NeedToDropPatientList(bool Set /* = false */)
{
	m_bNeedToDropPatientList = Set;
}

int CPatientView::ShowPrefsDlg()
{
	if (m_pActiveSheet == &m_General1Sheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piGeneral1);
	else if (m_pActiveSheet == &m_General2Sheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piGeneral2);
	else if (m_pActiveSheet == &m_CustomSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piCustom);	
	else if (m_pActiveSheet == &m_ProcedureSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piTracking);	
	else if (m_pActiveSheet == &m_FollowUpSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piToDo);
	else if (m_pActiveSheet == &m_NotesSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piNotes);	
	else if (m_pActiveSheet == &m_InsuranceSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piInsurance);
	else if (m_pActiveSheet == &m_FinancialSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piBilling);
	else if (m_pActiveSheet == &m_QuotesSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piQuotes);
	else if (m_pActiveSheet == &m_AppointmentsSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piAppointments);
	else if (m_pActiveSheet == &m_HistorySheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piHistory);
	else if (m_pActiveSheet == &m_MedicationSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piMedications);
	else if (m_pActiveSheet == &m_NexEMRSheet || m_pActiveSheet == &m_CustomRecordsSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piEMR);
	else if (m_pActiveSheet == &m_LabSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piLabs);
	// (c.haag 2009-11-05 17:35) - PLID 35920
	else if (m_pActiveSheet == &m_NexPhotoSheet)
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piNexPhoto);
	else
		return ShowPreferencesDlg(GetRemoteData(),GetCurrentUserName(), GetRegistryBase(), piPatientsModule);
}

void CPatientView::OnFilePrintPreview() {

	//taken from CNxTabView's OnFilePrintPreview
	if (m_pActiveSheet->GetSafeHwnd()) {
		// Copy a picture of the sheet to memory
		m_pActiveSheet->StorePrintImage();
	}

   // In derived classes, implement special window handling here.
  // Be sure to Unhook Frame Window close if hooked.
  // Must not create this on the frame. Must outlive this function.

  CPrintPreviewState* pState = new CPrintPreviewState;

  // DoPrintPreview's return value does not necessarily indicate that
  // Print Preview succeeded or failed, but rather what actions are
  // necessary at this point. If DoPrintPreview returns TRUE, it means
  // that OnEndPrintPreview will be (or has already been) called and
  // the pState structure will be/has been deleted.
  // If DoPrintPreview returns FALSE, it means that OnEndPrintPreview
  // WILL NOT be called and that cleanup, including deleting pState,
  // must be done here.

  if (!DoPrintPreview(AFX_IDD_PREVIEW_TOOLBAR, this,
     RUNTIME_CLASS(CNxPreviewView), pState))
  {
     // In derived classes, reverse special window handling here for
     // Preview failure case.

     TRACE0("Error: DoPrintPreview failed.\n");
     AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
     delete pState; // Preview failed to initialize, delete State now.
  }

  
}


LRESULT CPatientView::OnPreviewClosed(WPARAM wParam, LPARAM lParam) {

	CNxDialog *pActiveSheet = GetActiveSheet();


	//they closed the dialog, so we can revert back to having m_bIsMedSchedPrinting = TRUE
	//we are also done with the toggle
	if(pActiveSheet == &m_MedicationSheet) {				
		if(m_MedicationSheet.m_bIsMedSchedPrinting) {
			m_MedicationSheet.m_bIsMedSchedPrinting = FALSE;
			m_MedicationSheet.m_bToggleEndMedSchedPrinting = FALSE;
		}
	}

	// (d.singleton 2012-06-25 14:41) - PLID 50471 if the preview was the pre op calendar set m_bIsPicPreOpCalendar to false
	if(m_bIsPicPreOpCalendar) {
		m_bIsPicPreOpCalendar = FALSE;
	}

	return 0;

}

LRESULT CPatientView::OnPreviewPrinted(WPARAM wParam, LPARAM lParam) {

	//if we got here, we are printing a medication schedule, so make sure the bool is true
	CNxDialog *pActiveSheet = GetActiveSheet();
	if(pActiveSheet == &m_MedicationSheet) {				
		m_MedicationSheet.m_bIsMedSchedPrinting = FALSE;
		m_MedicationSheet.m_bToggleEndMedSchedPrinting = FALSE;
	}

	
	return 0;
}

LRESULT CPatientView::OnMSRDataEvent(WPARAM wParam, LPARAM lParam)
{
	// (a.wetta 2007-03-20 09:13) - PLID 24983 - The patient's module has been notified that a card has been swiped,
	// if the General 1 tab is the active tab, let's let the tab know about the swipe.
	if (GetActiveTab() == PatientsModule::General1Tab && m_General1Sheet.GetSafeHwnd()) {
		m_General1Sheet.SendMessage(WM_MSR_DATA_EVENT, wParam, lParam);
	}

	return 0;
}

// (a.walling 2008-09-10 15:05) - PLID 31334 - Forward to history sheet
LRESULT CPatientView::OnWIAEvent(WPARAM wParam, LPARAM lParam)
{
	try {
		//SetActiveTab(HistoryTab);

		if (GetActiveTab() != PatientsModule::HistoryTab)
			return (LRESULT)-1;
		if (!::IsWindow(m_HistorySheet.GetSafeHwnd()))
			return (LRESULT)-1;

		return m_HistorySheet.SendMessage(NXM_WIA_EVENT, wParam, lParam);
	
	}NxCatchAll("CPatientView::OnWIAEvent");

	return (LRESULT)-1;
}

// (a.walling 2009-06-05 12:53) - PLID 34496 - Handle things that used to be done in OnDraw
LRESULT CPatientView::OnPostOnDraw(WPARAM wParam, LPARAM lParam)
{
	try {
		// Display the patient warning if the view was just created
		if (m_hCheckPatientWarning && WaitForSingleObject(m_hCheckPatientWarning, 0) == WAIT_OBJECT_0)
		{
			ResetEvent(m_hCheckPatientWarning);
			PromptPatientWarning();
		}
		// if they have the preference set to auto-drop the list and we haven't dropped the list yet, then 
		// drop it
		if(m_bNeedToDropPatientList && GetRemotePropertyInt("AutoDropPatList",0,0,GetCurrentUserName(),TRUE) == 1){
			//TES 1/6/2010 - PLID 36761 - New accessor for dropdown state.
			GetMainFrame()->m_patToolBar.SetDroppedDown(TRUE);
		}
		m_bNeedToDropPatientList = false;
	} NxCatchAll("CPatientView::OnPostOnDraw");

	return 0;
}

// (z.manning 2008-09-22 17:08) - PLID 31252
CFinancialDlg* CPatientView::GetFinancialDlg()
{
	return &m_FinancialSheet;
}

// (z.manning 2009-05-05 10:08) - PLID 34172
void CPatientView::OnReferralOrders()
{
	try
	{
		// (z.manning 2009-05-08 11:57) - PLID 34172 - Permissions
		if(!CheckCurrentUserPermissions(bioReferralOrders, sptRead)) {
			return;
		}

		CReferralOrderDlg dlg(GetActivePatientID(), this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (z.manning 2009-05-05 10:08) - PLID 28554
void CPatientView::OnOrderSets()
{
	try
	{
		COrderSetsDlg dlg(GetActivePatientID(), this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (d.thompson 2009-05-14) - PLID 34232
void CPatientView::OnImmunizations()
{
	try {
		CPatientImmunizationsDlg dlg(this);
		dlg.m_nPersonID = GetActivePatientID();
		// (d.thompson 2013-07-16) - PLID 57513 - Name required for auditing
		dlg.m_strPersonName = GetActivePatientName();
		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);
}

// (j.gruber 2013-11-05 12:28) - PLID 59323 - summary of care
void CPatientView::OnExportSummaryofCare()
{
	try {
		CSummaryofCareExportDlg dlg;
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}



// (a.walling 2009-06-01 11:46) - PLID 34410
void CPatientView::OnAdvanceDirectives()
{
	try {
		CAdvanceDirectiveDlg dlg(this);
		dlg.m_nPatientID = GetActivePatientID();
		dlg.DoModal();
	} NxCatchAll(__FUNCTION__);
}

// (d.thompson 2009-05-27) - PLID 28486
void CPatientView::OnGraphPatientEMRData()
{
	try {
		CPatientGraphConfigDlg dlg(this);
		dlg.DoModal();

	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2009-12-22 17:48) - PLID 7002 - Prevent closing if a bill is active
LRESULT CPatientView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message) {
	case NXM_ALLOW_CLOSE:
		try {
			if (GetMainFrame()->IsBillingModuleOpen(true)) {
				return AC_CANNOT_CLOSE;
			}
		} NxCatchAll("CPatientView::WindowProc NXM_ALLOW_CLOSE");
		break;
	case WM_CLOSE:
		try {
			if (GetMainFrame()->IsBillingModuleOpen(true)) {
				return 0;
			}
		} NxCatchAll("CPatientView::WindowProc WM_CLOSE");
	}

	return CNxTabView::WindowProc(message, wParam, lParam);
}
//(a.wilson 2012-1-13) PLID 47485 - to handle incoming messages from mainframe for the opos barcode scanner.
LRESULT CPatientView::OnOPOSBarcodeScannerEvent(WPARAM wParam, LPARAM lParam)
{
	try {
		if (GetActiveTab() == PatientsModule::General1Tab && m_General1Sheet.GetSafeHwnd()) {
			m_General1Sheet.SendMessage(NXM_BARSCAN_DATA_EVENT, wParam, lParam);
		}
	} NxCatchAll(__FUNCTION__);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Everthing below copied
// (d.singleton 2012-04-06 12:05) - PLID 50471 copied from MedicationDlg for printing PreOp Calender
void CPatientView::OnBeginPreOpCalendarPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	if(!bMedSchedPrintInfoLoaded)
		LoadMedSchedPrintInfo();

	COleDateTime dtOriginalDate = COleDateTime::GetCurrentTime();
	BOOL bStartDate = FALSE;
	_RecordsetPtr rs = CreateRecordset("SELECT ProcedureID, StartDate FROM MedScheduleT WHERE ID = %li",m_tempMedSchedID);
	if(!rs->eof) {
		_variant_t var = rs->Fields->Item["StartDate"]->Value;
		if(var.vt == VT_DATE) {
			dtOriginalDate = VarDateTime(var);
			bStartDate = TRUE;
		}
	}

	if(bStartDate) {
		//calc pages based on how many months we need
		long nMaxPage = 1;
		long nMaxMonth = COleDateTime(dtOriginalDate + COleDateTimeSpan(m_nMedSchedDays,0,0,0)).GetMonth();
		long nStartMonth = dtOriginalDate.GetMonth();
		nMaxPage = nMaxMonth - nStartMonth + 1;
		if(nMaxPage <= 0 || (nMaxPage == 1 && m_nMedSchedDays > 31))
			nMaxPage += 12;
		pInfo->SetMaxPage(nMaxPage);
	}
	else {
		//calc how many cells we need
		long nPages = m_nMedSchedDays / 42 + (m_nMedSchedDays % 42 > 0 ? 1 : 0);
		if(nPages <= 0)
			nPages = 1;
		pInfo->SetMaxPage(nPages);
	}
}

void CPatientView::PrintPreOpCalendar(CDC *pDC, CPrintInfo *pInfo)
{
	if(!bMedSchedPrintInfoLoaded)
		LoadMedSchedPrintInfo();

	const char* month_names[12] = { "January", "February", "March",
		"April", "May", "June", "July", "August", "September",
		"October", "November", "December" };

	// Some useful drawing tools
	CBrush greybrush(RGB(192, 192, 192));
	CBrush whitebrush(RGB(255, 255, 255));
	CBrush* pOldBrush = pDC->SelectObject(&whitebrush);
	CPen pen(PS_SOLID, 2, RGB(0,0,0));
	CPen* pOldPen = pDC->SelectObject(&pen);
	CFont* pOldFont;

	// Decide if we're printing in color or not
	BOOL bBlackAndWhite;
	if (pDC->GetDeviceCaps(BITSPIXEL) > 1) {
		bBlackAndWhite = FALSE;
	} else {
		bBlackAndWhite = TRUE;
	}

	// (j.gruber 2010-06-01 16:11) - PLID 38809 - nActiveMonth needs to be initialized
	long nActiveMonth = -1;
	int nActiveYear;
	int nActiveDay;

	long nDaysVisible = 42;
	long nRowCount = 6;

	long nProcId = -1;
	COleDateTime dtStartDate = COleDateTime::GetCurrentTime();
	COleDateTime dtOriginalDate = COleDateTime::GetCurrentTime();
	BOOL bStartDate = FALSE;
	_RecordsetPtr rs = CreateRecordset("SELECT ProcedureID, StartDate FROM MedScheduleT WHERE ID = %li", m_tempMedSchedID);
	if(!rs->eof) {
		_variant_t var = rs->Fields->Item["ProcedureID"]->Value;
		if(var.vt == VT_I4) {
			nProcId = var.lVal;
		}

		var = rs->Fields->Item["StartDate"]->Value;
		if(var.vt == VT_DATE) {
			dtOriginalDate = dtStartDate = VarDateTime(var);
			bStartDate = TRUE;
		}
	}

	pInfo->SetMinPage(1);
	pInfo->SetMaxPage(1);

	long nPageID = pInfo->m_nCurPage;

	int CurrDay = 1;

	if(nPageID > 1) {
		
		for(int i=1;i<nPageID;i++) {
			if(bStartDate) {
				//if we are using dates
				dtStartDate += COleDateTimeSpan(GetDaysInMonth(dtStartDate.GetMonth(), dtStartDate.GetYear()),0,0,0);
			}
			else {
				//just increment the number
				CurrDay += 42;
			}
		}		
	}

	//////////////////////////
	COleDateTime dtActiveDate = COleDateTime::GetCurrentTime();
 	// Calculate important information based on the active date
	if(bStartDate) {

		dtActiveDate = dtStartDate;
		
		nActiveMonth = dtActiveDate.GetMonth();
		nActiveYear = dtActiveDate.GetYear();
		nActiveDay = dtActiveDate.GetDay();
		// Move to the beginning of the month and get the day of the week
		dtActiveDate -= COleDateTimeSpan(nActiveDay-1,0,0,0);
		const int nWeekDay = dtActiveDate.GetDayOfWeek();
		// Move to the next previous sunday
		dtActiveDate -= COleDateTimeSpan(nWeekDay-1, 0,0,0);	

		// Calculate the number of rows we'll need
		nDaysVisible = (GetDaysInMonth(nActiveMonth, nActiveYear) + nWeekDay - 1);
		nRowCount = nDaysVisible / 7 + ((nDaysVisible % 7) ? 1 : 0);
	}
	/////////////////////////

	if(bStartDate) {
		//calc pages based on how many months we need
		long nMaxPage = 1;
		long nMaxMonth = COleDateTime(dtOriginalDate + COleDateTimeSpan(m_nMedSchedDays,0,0,0)).GetMonth();
		long nStartMonth = dtOriginalDate.GetMonth();
		nMaxPage = nMaxMonth - nStartMonth + 1;
		if(nMaxPage <= 0 || (nMaxPage == 1 && m_nMedSchedDays > 31))
			nMaxPage += 12;
		pInfo->SetMaxPage(nMaxPage);
	}
	else {
		//calc how many cells we need
		long nPages = m_nMedSchedDays / 42 + (m_nMedSchedDays % 42 > 0 ? 1 : 0);
		if(nPages <= 0)
			nPages = 1;
		pInfo->SetMaxPage(nPages);
	}

	// Set the draw info
	pDC->SetTextColor(RGB(0,0,0));
	pDC->SetBkMode(TRANSPARENT);

	// Output the page header	
	int starty = DrawPageHeader(pDC, pInfo->m_rectDraw, nProcId);

	// Calculate other useful values
	int boxwidth = pInfo->m_rectDraw.Width() / 8;  // Width of calendar day
	int boxheight = (pInfo->m_rectDraw.Height()-starty-300) / nRowCount; // Height of calendar day
	int startx = (pInfo->m_rectDraw.Width() - boxwidth*7) / 2;

	// Select the font we will be using for the rest of the print
	CFont font;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&font, 70, "Arial", pDC);
	pOldFont = pDC->SelectObject(&font);

	// Calculate the height of the text based on the font we will be using to draw it
	LOGFONT lf;
	font.GetLogFont(&lf);
	int textheight = lf.lfHeight;
	if (textheight < 0) {
		textheight = -textheight;
	} else if (textheight == 0) {
		textheight = 30;
	}

	// Some variables used repeatedly in the upcoming loop
	CStringArray* pastrNames = NULL;
	CString strDayText;
	long nDayHeaderBottom;
	BOOL bIsDayActive;

	if(bStartDate)
		CurrDay = (long)(dtActiveDate - dtOriginalDate) + 1;

	COleDateTime dtCurrDay = COleDateTime::GetCurrentTime();
	if(bStartDate) {
		dtCurrDay = dtActiveDate;
	}

	// Draw day header
	for (int x=startx; x < startx + boxwidth*7; x += boxwidth) {
		strDayText = " ";
		//this loop will draw the 7 day headers
		//but if you move this line before the DrawDayNumber, you can have this line on every week

		if(bStartDate) {
			strDayText = FormatDateTimeForInterface(dtCurrDay,"%A");
			COleDateTimeSpan dtOneDay;
			dtOneDay.SetDateTimeSpan(1,0,0,0);
			dtCurrDay += dtOneDay;
		}

		nDayHeaderBottom = DrawDayHeader(pDC, x, starty, boxwidth, boxheight, strDayText, TRUE, FALSE);
	}

	dtCurrDay = dtActiveDate;

	starty = nDayHeaderBottom;

	// Show all the days  (nRowCount rows, seven columns)
	for (int y=starty; y < starty + boxheight*nRowCount; y += boxheight) {
		for (int x=startx; x < startx + boxwidth*7; x += boxwidth) {
			// Generate this day's rect
			CRect rc(x, y, x + boxwidth + 1, y + boxheight + 1);
			
			//IsTheDayGray?
			bIsDayActive = dtCurrDay.GetMonth() == nActiveMonth;

			// Draw day rect
			{
				if (bStartDate && !bIsDayActive) {
					// Nope, so color it grey and make sure no events are printed
					pOldBrush = pDC->SelectObject(&greybrush);
					pDC->SetBkColor(RGB(192, 192, 192));
					pastrNames = NULL;
				} else {
					// Is this day part of our schedule?
					if(CurrDay > 0 && CurrDay <= m_nMedSchedDays)
						//Yep, so get a pointer to the beginning of this day's array
						pastrNames = &m_astrMedSchedEntries[CurrDay];
					else
						pastrNames = NULL;
				}

				// Plot rectangle that represents this day
				pDC->Rectangle(&rc);
			}			

			// Draw day number
			{
				// Generate the day number text
				strDayText = " ";

				if(bStartDate) {
					if (dtActiveDate.GetDay() == 1 || (x == startx && y == starty)) {
						// This is the first VISIBLE day of the given month so
						// prepend the month's common name to the day value
						strDayText.Format("%s %d", month_names[dtActiveDate.GetMonth()-1], dtActiveDate.GetDay());
					} else {
						// This is a regular day so just use the day number
						strDayText.Format("%d", dtActiveDate.GetDay());
					}
					COleDateTimeSpan dtOneDay;
					dtOneDay.SetDateTimeSpan(1,0,0,0);
					dtActiveDate += dtOneDay;
				}

				// Draw the day header
				nDayHeaderBottom = DrawDayNumber(pDC, x, y, boxwidth, boxheight, strDayText, TRUE, FALSE);
			}
			
			
			// Print day entries
			{
				if (pastrNames != NULL) {
					// Decide where the first appointment will display
					int text_y = nDayHeaderBottom;

					// Decide which entry to display first
					int nStart = 0; //m_nVisibleNamesOffset;
					int nMaxStart = pastrNames->GetSize() - 5; //m_nVisibleNamesPerDay;
					if (nMaxStart < 0) nMaxStart = 0;
					if (nStart > nMaxStart) {
						nStart = nMaxStart;
					}
					
					if (nStart > 0) {
						// If we're starting anywhere but the first entry, 
						// calculate the total height of all entries and if 
						// they'll fit then start at 0
						if (DoesDayOutputFit(pDC, pastrNames, boxwidth-20, rc.Height()-textheight)) {
							// The output will fit
							nStart = 0;
						}
					}

					// If we are starting anywhere but the beginning 
					// of the list, put three dots at the right
					if (nStart > 0) {
						pDC->SetTextColor(RGB(0,0,0));
						pDC->TextOut(x + boxwidth - 110, y + 50, "***");
					}
					for (int i=nStart; i < pastrNames->GetSize() && text_y <= rc.bottom - textheight; i++) {
						// Set color of text
						COLORREF clr;
						if (bBlackAndWhite) {
							// Use black
							clr = RGB(0,0,0);
						} else {
							// Use purposeset color
							clr = (COLORREF)m_adwMedSchedColors[CurrDay].GetAt(i);
							// Tone down if too bright
							// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
							clr = GetDarkerColorForApptText(clr);
						}
						pDC->SetTextColor( clr );
						text_y += pDC->DrawText(pastrNames->GetAt(i), 
							CRect(x + 20, text_y, rc.right, rc.bottom), 
							DT_WORDBREAK|DT_TOP|DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS);
					}
					if (i < pastrNames->GetSize()) {
						pDC->SetTextColor(RGB(0,0,0));
						pDC->TextOut(x + boxwidth - 110, y + boxheight - textheight + 50, "***");
					}
				}
			}


			// Default back to white if greyed
			if (dtCurrDay.GetMonth() != nActiveMonth) {
				pDC->SelectObject(pOldBrush);
				pDC->SetBkColor(RGB(255, 255, 255));
			}
			
			// Move to next day
			dtCurrDay += COleDateTimeSpan(1,0,0,0);

			CurrDay++;
		}
	}
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldFont);
}

long CPatientView::DrawPageHeader(CDC *pDC, LPRECT rcPage, long nProcId)
{
	// Calculate the header text
	CString strHdr;
	CString strProcType = "";

	// See if we are displaying a particular procedure
	if (nProcId != -1) {

		// Create the header text
		_RecordsetPtr rs = CreateRecordset("SELECT Name FROM ProcedureT WHERE ID = %li",nProcId);
		if(!rs->eof) {
			CString name = CString(rs->Fields->Item["Name"]->Value.bstrVal);
			strProcType.Format("%s ",name);
		}
		rs->Close();
	}

	// Create the header text
	_RecordsetPtr rs = CreateRecordset("SELECT First + ' ' + Last AS Name FROM PersonT WHERE ID = %li",GetActivePatientID());
	if(!rs->eof) {
		CString name = CString(rs->Fields->Item["Name"]->Value.bstrVal);
		strHdr.Format("%sPre-Op Calendar for %s", strProcType, name);
	}
	else
		strHdr.Format("%sPre-Op Calendar", strProcType);
	rs->Close();

	// Create and select an appropriate font
	CFont fontHdr, *pOldFont;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(&fontHdr, 240, "Arial", pDC);
	pOldFont = pDC->SelectObject(&fontHdr);
	// Set header text color
	pDC->SetTextColor(RGB(0,0,0));
	// Determine the header area
	const int nHeaderTop = 80;
	CRect rcHeader(rcPage);
	rcHeader.DeflateRect(0, nHeaderTop, 0, 0);
	// Draw the header, storing its height
	long nHeight = nHeaderTop+pDC->DrawText(strHdr, rcHeader, DT_TOP|/*DT_SINGLELINE|*/DT_NOPREFIX|DT_CENTER);
	// Unselect the header font
	pDC->SelectObject(pOldFont);

	// Return the bottom of the drawn header (i.e. where anything else 
	// could start drawing safely without overlapping the header area)
	return rcHeader.top + nHeight;
}

long CPatientView::DrawDayNumber(CDC *pDC, long x, long y, long nBoxWidth, long nBoxHeight, LPCTSTR strDayText, BOOL bDrawBorder, BOOL bDrawGrey)
{
	// Set up the colors appropriately
	pDC->SetTextColor(RGB(0,0,0) );
	if(bDrawGrey)
		pDC->SetBkColor(RGB(192,192,192));
	CBrush greyBrush(RGB(192,192,192)), whiteBrush(RGB(255,255,255)),*pOldBrush;
	if(bDrawGrey)
		pOldBrush = pDC->SelectObject(&greyBrush);
	else
		pOldBrush = pDC->SelectObject(&whiteBrush);

	// Margin around the header
	const long mx = 10, my = 4;

	// Set up the text rect for the day header and calc the size of the text
	CRect rcDayNumberText(x+mx, y+my, x+nBoxWidth-mx, y+nBoxHeight-my);
	pDC->DrawText(strDayText, rcDayNumberText, DT_CALCRECT|DT_TOP|DT_NOPREFIX|DT_LEFT);

	rcDayNumberText.left += mx;
	rcDayNumberText.right += mx;

	const long defRight = x+(nBoxWidth/5), defBottom = y+(nBoxHeight/5);

	long boxRight, boxBottom;
	if(rcDayNumberText.right + mx < defRight)
		boxRight = defRight;
	else
		boxRight = rcDayNumberText.right + mx;
	if(rcDayNumberText.bottom + my < defBottom)
		boxBottom = defBottom;
	else
		boxBottom = rcDayNumberText.bottom + my;

	CRect rcDayNumberBox(x, y, boxRight, boxBottom);
	if (bDrawBorder)
		pDC->Rectangle(rcDayNumberBox);
	
	// Actually draw the text in the day header area
	pDC->DrawText(strDayText, rcDayNumberText, DT_TOP|DT_NOPREFIX|DT_LEFT);

	// We're done with the brush
	pDC->SelectObject(pOldBrush);

	// Return the bottom of the drawn header (i.e. where anything else 
	// could start drawing safely without overlapping the header area)
	return rcDayNumberBox.bottom;
}

BOOL CPatientView::DoesDayOutputFit(CDC *pDC, CStringArray *pastrNames, long nBoxWidth, long nBoxHeight)
{
	if (pastrNames) {
		long y = 0;
		// Loop until we've extended past box height
		for (int i=0; i < pastrNames->GetSize() && y <= nBoxHeight; i++) {
			// Don't actually draw the text, just calculate
			y += pDC->DrawText((pastrNames->GetAt(i)), 
				CRect(0, 0, nBoxWidth, nBoxHeight), 
				DT_CALCRECT|DT_WORDBREAK|DT_TOP|DT_LEFT|DT_NOPREFIX|DT_EXPANDTABS);
		}
		if (y > nBoxHeight) {
			// We extended past box height so "No, the day output doesn't fit"
			return FALSE;
		} else {
			// We extended past box height so "Yes, the day output does fit"
			return TRUE;
		}
	} else {
		// We got a NULL pastrNames.  That shouldn't happen
		ASSERT(FALSE);
		return FALSE;
	}
}

void CPatientView::LoadMedSchedPrintInfo() {

	try {

		for (int i=0; i <= 366; i++) {
			m_astrMedSchedEntries[i].RemoveAll();
			m_adwMedSchedColors[i].RemoveAll();
			m_nMedSchedDays = 0;
		}

		if(m_tempMedSchedID == -1)
			return;

		COleDateTime dtStartDate = g_cdtNull;

		// (d.singleton 2012-05-15 17:11) - PLID 50471 need start date of med schedule then load the actual appointment records
		_RecordsetPtr rs = CreateParamRecordset("SELECT StartDate FROM MedScheduleT WHERE ID = {INT}", m_tempMedSchedID);
		if(!rs->eof) {
			dtStartDate = AdoFldDateTime(rs, "StartDate", g_cdtNull);
		}

		//load the appts from the pic
		for( int i = 0; i < m_arAppointmentInfo->GetCount(); i++) {

			COleDateTimeSpan ts;
			ts = m_arAppointmentInfo->GetAt(i).dtStartDate - dtStartDate;
			long nStartDay = ts.GetDays() + 1;

			if(nStartDay < 367) {
				m_astrMedSchedEntries[nStartDay].Add(m_arAppointmentInfo->GetAt(i).strAptTime);
				m_adwMedSchedColors[nStartDay].Add(0);

				if(m_nMedSchedDays < nStartDay) {
					m_nMedSchedDays = nStartDay;
				}
			}
		}

		rs = CreateRecordset("SELECT * FROM MedSchedDetailsT WHERE MedSchedID = %li ORDER BY Priority", m_tempMedSchedID);
		_variant_t var;
		
		while(!rs->eof) {
			//load the start day, the type, start note, and color, as all items have these
			long StartDay;
			StartDay = rs->Fields->Item["StartDay"]->Value.lVal;
			int Type = rs->Fields->Item["Type"]->Value.iVal;

			CString StartNote;
			var = rs->Fields->Item["StartNote"]->Value;
			if(var.vt == VT_BSTR) {
				StartNote = CString(var.bstrVal);
			}

			DWORD color;
			var = rs->Fields->Item["Color"]->Value;
			if(var.vt == VT_I4) {
				color = var.lVal;
			}

			if(StartDay > 366) {
				rs->MoveNext();
				continue;
			}

			int DurationType = 1;
			var = rs->Fields->Item["DurationType"]->Value;
			if(var.vt == VT_UI1) {
				DurationType = var.iVal;
			}

			// (d.singleton 2012-05-15 17:06) - PLID 50471  PreOp Notes
			// (d.singleton 2013-04-05 10:57) - PLID 53301 do not add PreOp notes to consult appts
			if(Type==4) {
				for(int i = 0; i < m_arAppointmentInfo->GetCount(); i++) {
					if(m_arAppointmentInfo->GetAt(i).nCategoryID ==2) {
						//get day of appt
						COleDateTimeSpan ts;
						int nActualStartDay;
						ts = m_arAppointmentInfo->GetAt(i).dtStartDate - dtStartDate;
						long nStartDay = ts.GetDays() + 1;

						if(DurationType == 1) { //before appt
							nActualStartDay = nStartDay - StartDay;
						}
						else if(DurationType == 2) { //after appt						
							nActualStartDay = nStartDay + StartDay;
						}

						if(nActualStartDay < 367) { //upper limit of our array so skip that entry
							//add the note 
							m_astrMedSchedEntries[nActualStartDay].Add(StartNote);
							m_adwMedSchedColors[nActualStartDay].Add(color);

							if(m_nMedSchedDays < nActualStartDay) {
								m_nMedSchedDays = nActualStartDay;
							}
						}						
					}
				}
				rs->MoveNext();
				continue;
			}

			// (d.singleton 2012-05-15 17:06) - PLID 50471 Surgery Notes
			if(Type==5) {
				for(int i = 0; i < m_arAppointmentInfo->GetCount(); i++) {
					if(m_arAppointmentInfo->GetAt(i).nCategoryID ==4 || m_arAppointmentInfo->GetAt(i).nCategoryID ==3) {
						//get day of appt
						COleDateTimeSpan ts;
						int nActualStartDay;
						ts = m_arAppointmentInfo->GetAt(i).dtStartDate - dtStartDate;
						long nStartDay = ts.GetDays() + 1;

						if(DurationType == 1) { //before appt
							nActualStartDay = nStartDay - StartDay;
						}
						else if(DurationType == 2) { //after appt						
							nActualStartDay = nStartDay + StartDay;
						}

						if(nActualStartDay < 367) { //upper limit of our array so skip that entry
							//add the note 
							m_astrMedSchedEntries[nActualStartDay].Add(StartNote);
							m_adwMedSchedColors[nActualStartDay].Add(color);

							if(m_nMedSchedDays < nActualStartDay) {
								m_nMedSchedDays = nActualStartDay;
							}
						}
					}
				}
				rs->MoveNext();
				continue;
			}

			// (d.singleton 2012-05-15 17:06) - PLID 50471 PostOp Notes
			if(Type==6) {
				for(int i = 0; i < m_arAppointmentInfo->GetCount(); i++) {
					if(m_arAppointmentInfo->GetAt(i).nCategoryID ==5) {
						//get day of appt
						COleDateTimeSpan ts;
						int nActualStartDay;
						ts = m_arAppointmentInfo->GetAt(i).dtStartDate - dtStartDate;
						long nStartDay = ts.GetDays() + 1;

						if(DurationType == 1) { //before appt
							nActualStartDay = nStartDay - StartDay;
						}
						else if(DurationType == 2) { //after appt						
							nActualStartDay = nStartDay + StartDay;
						}

						if(nActualStartDay < 367) { //upper limit of our array so skip that entry
							//add the note 
							m_astrMedSchedEntries[nActualStartDay].Add(StartNote);
							m_adwMedSchedColors[nActualStartDay].Add(color);

							if(m_nMedSchedDays < nActualStartDay) {
								m_nMedSchedDays = nActualStartDay;
							}
						}
					}
				}
				rs->MoveNext();
				continue;
			}


			//add the start note and color on the start day
			m_astrMedSchedEntries[StartDay].Add(StartNote);
			m_adwMedSchedColors[StartDay].Add(color);

			if(m_nMedSchedDays < StartDay)
				m_nMedSchedDays = StartDay;

			//if a Medication, we have much more work to do
			if(Type==1) { 
				/*int DurationType = 1;
				var = rs->Fields->Item["DurationType"]->Value;
				if(var.vt == VT_UI1) {
					DurationType = var.iVal;
				}*/

				int StopMethod = 1;
				var = rs->Fields->Item["StopMethod"]->Value;
				if(var.vt == VT_UI1) {
					StopMethod = var.iVal;
				}

				CString MiddleNote;
				var = rs->Fields->Item["MiddleNote"]->Value;
				if(var.vt == VT_BSTR) {
					MiddleNote = CString(var.bstrVal);
				}

				CString StopNote;
				var = rs->Fields->Item["StopNote"]->Value;
				if(var.vt == VT_BSTR) {
					StopNote = CString(var.bstrVal);
				}

				long StopDay = StartDay;
				var = rs->Fields->Item["StopDay"]->Value;
				if(var.vt == VT_I4) {
					StopDay = var.lVal;
				}

				//if DurationType = 2, we are going to run for "StopDay" days
				if(DurationType == 2)
					StopDay = StartDay + StopDay - 1;

				//if StopMethod = 2, we will place the stop note on the day after the official "Stop Day"
				if(StopMethod == 2)
					StopDay++;

				if(StopDay > 366)
					StopDay = 366;

				if(m_nMedSchedDays < StopDay)
					m_nMedSchedDays = StopDay;


				for(int i = StartDay+1; i < StopDay; i++) {
					m_astrMedSchedEntries[i].Add(MiddleNote);
					m_adwMedSchedColors[i].Add(color);
				}

				//don't display twice if only one day
				if(StopDay > StartDay) {
					m_astrMedSchedEntries[StopDay].Add(StopNote);
					m_adwMedSchedColors[StopDay].Add(color);
				}
			}

			rs->MoveNext();
		}
		rs->Close();

		bMedSchedPrintInfoLoaded = TRUE;	

	}NxCatchAll("Error generating Medication Schedule.");
}

long CPatientView::DrawDayHeader(CDC *pDC, long x, long y, long nBoxWidth, long nBoxHeight, LPCTSTR strDayText, BOOL bDrawBorder, BOOL bDrawGrey)
{
	// Set up the colors appropriately
	pDC->SetTextColor(RGB(0,0,0));
	if(bDrawGrey)
		pDC->SetBkColor(RGB(192,192,192));
	CBrush greyBrush(RGB(192,192,192)), whiteBrush(RGB(255,255,255)),*pOldBrush;
	if(bDrawGrey)
		pOldBrush = pDC->SelectObject(&greyBrush);
	else
		pOldBrush = pDC->SelectObject(&whiteBrush);

	// Margin around the header
	const long mx = 10, my = 4;

	// Set up the text rect for the day header and calc the size of the text
	CRect rcDayHeaderText(x+mx, y+my, x+nBoxWidth-mx, y+nBoxHeight-my);
	pDC->DrawText(strDayText, rcDayHeaderText, DT_CALCRECT|DT_TOP|DT_NOPREFIX|DT_LEFT);

	rcDayHeaderText.left += mx;
	rcDayHeaderText.right += mx;

	// Set up the rect for the entire day header area and color it grey
	CRect rcDayHeaderBox(x, y, x + nBoxWidth + 1, y + (nBoxHeight/5) + 1);
	if (bDrawBorder)
		pDC->Rectangle(rcDayHeaderBox);
	
	// Actually draw the text in the day header area
	pDC->DrawText(strDayText, rcDayHeaderText, DT_TOP|DT_NOPREFIX|DT_LEFT);

	// We're done with the brush
	pDC->SelectObject(pOldBrush);

	// Return the bottom of the drawn header (i.e. where anything else 
	// could start drawing safely without overlapping the header area)
	return rcDayHeaderBox.bottom;
}

// (a.wilson 2012-07-06 15:48) - PLID 51332 - adding silent flag for default tab preference.
// (j.gruber 2012-06-19 15:50) - PLID 51067
BOOL CPatientView::CheckIEVersion(BOOL bSilent /* = FALSE */)
{
	if (NxRegUtils::DoesValueExist("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Internet Explorer\\Version")) {
		// (j.gruber 2012-06-19 14:00) - PLID 51067 - check if they have at least IE 8
		CString strVersion = NxRegUtils::ReadString("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Internet Explorer\\Version");
		//now we just need the first part
		long nResult = strVersion.Find(".");
		if (nResult != -1){
			//see what the value is
			if (atoi(strVersion.Left(nResult)) < 8) {
				//output a message box telling them they can't use the dashboard
				if (!bSilent) MsgBox("The patient dashboard requires an Internet Explorer version of 8 or higher.  Please upgrade Internet Explorer if you would like to use the dashboard.");
				return FALSE;
			}
			else {
				return TRUE;
			}
		}	
		else {
			if (!bSilent) MsgBox("Practice could not determine the current version of Internet Explorer.  Please ensure Internet Explorer 8 or higher is installed before using the dashboard.");
			return FALSE;
		}
	}
	else {
		if (!bSilent) MsgBox("Practice could not determine the current version of Internet Explorer.  Please ensure Internet Explorer 8 or higher is installed before using the dashboard.");
		return FALSE;

	}
}

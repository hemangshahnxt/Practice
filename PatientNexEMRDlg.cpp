// PatientNexEMRDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EmrRc.h"
#include "PatientNexEMRDlg.h"
#include "EmrEditorDlg.h"
#include "EMRAuditHistoryDlg.h"
#include "ConfigureNexEMRGroupsDlg.h"
#include "GlobalUtils.h"
#include "EmrUtils.h"
#include "EmrCollectionSetupDlg.h"
#include "EmrTemplateManagerDlg.h"
#include "SelectDlg.h"
#include "EmrTemplateEditorDlg.h"
#include "EMRChooseTemplateDlg.h"
#include "EMR.h"
#include "EMN.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "AuditTrail.h"
#include "EMRSummaryDlg.h"
#include "EMRWordDocumentListDlg.h"
#include "EmrMoveEmnDlg.h"
#include "EMRProblemListDlg.h"
#include "PracticeRc.h"
#include "LicenseEMRProvidersDlg.h"
#include "dynuxtheme.h"
#include "ConfigureEMRTabsDlg.h"
#include "ChooseEMNCategoryDlg.h"
#include "PatientNexEMRConfigureColumnsDlg.h"
#include "PatientSummaryDlg.h"
#include "EMRAnalysisDlg.h"
#include "TodoUtils.h"
#include "CCDUtils.h"
#include "CCDInterface.h" //(e.lally 2010-02-18) PLID 37438
#include "PatientWellnessDlg.h"
#include "WellnessDataUtils.h"
#include "DeleteEMNConfirmDlg.h"
#include "EMRPreviewMultiPopupDlg.h"
#include "NxXMLUtils.h"
#include "ViewWebinarDlg.h"
#include "EmrColors.h"
#include "DecisionRuleUtils.h"
#include "MergeEngine.h"
#include "SelectCCDAInfoDlg.h"
#include <CancerCaseDocument.h>
#include "NxAPIUtils.h"
#include "CCDAUtils.h" // (b.savon 2014-04-30 09:31) - PLID 61791 - User Story 10 - Auxiliary CCDA Items - Requirement 100150

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CPatientNexEMRDlg dialog


//////////////////////////////////
//		Utility Functions		//
//////////////////////////////////
#define	EMN_INDENT	"   "

//Enumeration for the Existing EMR Columns
enum EMRColumns {
	eecPicID = 0,
	eecID,
	eecParentID,
	eecEMRID,
	eecEMNID,
	eecMailID, // (z.manning 2008-07-03 09:49) - PLID 25574 - Added MailID as we now support history docs
	eecMaxDate,
	eecProblemBitmap,
	eecDate,
	eecInputDate,
	eecProvider,	// (d.thompson 2010-01-08) - PLID 35925 - Added provider name
	eecSecondaryProvider, // (z.manning 2010-05-11 16:44) - PLID 38354
	eecTechnicians,	// (d.lange 2011-04-29 15:31) - PLID 43382 - Added Assistant/Technician
	eecPreviewIcon, // (z.manning 2008-07-02 11:41) - PLID 30596 - Added column to easily preview EMNs
	eecWordIcon,
	eecClinicalSummaryIcon, // (r.gonet 04/22/2014) - PLID 61807 - Added column to easily view the status of the clinical summary.
	eecEMRDescription, // (z.manning, 04/05/2007) - PLID 25518 - In tab view, we have a separate column for EMR description.
	eecDescription,
	eecStatus,
	eecTabChartID,
	eecTabCategoryID,
	eecCompletionStatus, // (j.jones 2007-06-14 10:03) - PLID 26276 - column for the EMRMasterT.CompletionStatus
	eecRowColor, // (j.jones 2007-06-19 10:14) - PLID 26276 - column for the row color, based on CompletionStatus
	eecHistoryPathName, // (z.manning 2008-07-03 11:24) - PLID 25574 - Added path for history documents
	eecPatientCreatedStatus, // (z.manning 2009-12-15 13:01) - PLID 35810
	eecModifiedDate, // (z.manning 2012-09-10 14:44) - PLID 52543
	eecTemplateID, // (c.haag 2013-02-28) - PLID 55368 - Added template ID
};

//Enumeration for New EMR Columns
enum NewEMRList {
	necID = 0,
	necParent,
	necType,
	necDescription,
	necRecordID,
	necColor
};

//Enumeration of possible New EMR Columns row types
enum NewEMRListType {
	neltHeader = 0,		//This is a hard coded header row.
	neltProcedure,		//This row is a procedure
	neltTemplate,		//This row is an EMR Template
	neltProcInfo,		//This is a record in ProcInfoT
};

// (a.walling 2010-06-30 14:02) - PLID 29974
enum EmrListThreadIconTypes {
	eWordIcon	= 0x01,
	eAudioIcon	= 0x02,
	eInvalid	= 0xFF,
};

#define TAB_HIGHLIGHT_COLOR	RGB(235,235,235)

// (a.wetta 2007-01-29 09:59) - This is used in the context menu of the exiting EMN list to determine if a new EMN should be created
// or if the user just wants to change the category of an EMN.  The offset represents an EMN tab ID of -1.
#define TAB_CAT_ID_OFFSET		-9999999
#define TAB_CHART_ID_OFFSET		-999999999


//////////////////////////////////////
//		END Utility Functions		//
//////////////////////////////////////


// (j.jones 2014-08-21 11:33) - PLID 63189 - added tablechecker object for EMR templates
CPatientNexEMRDlg::CPatientNexEMRDlg(CWnd* pParent)
	: CPatientDialog(CPatientNexEMRDlg::IDD, pParent)
	, m_pEMRLockReminderDlg(this)
	, m_templateChecker(NetUtils::EMRTemplateT)
{
	//{{AFX_DATA_INIT(CPatientNexEMRDlg)
	//}}AFX_DATA_INIT
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_NexEMR_Module/Creating_an_EMR_for_a_Patient/Creating_an_EMR.htm";	
	m_pFont = NULL;
	m_pdlgTemplateManager = NULL;
	m_pFinishEMRListThread = NULL;
	
	// (a.walling 2010-10-14 17:29) - PLID 40978
	m_id = -1;

	// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Reset our last notification on construction 
	// obviously, since we always want to notify them the first time.
	m_nReconstructedDetailsLastNotifiedPatientID = -1;

	m_bNeedProviderLicenseCheck = true;
	m_bNeedDictationLicenseCheck = true;
	m_bThemeOpen = FALSE;
	m_bEMNTabView = FALSE;
	m_pEMRPreviewPopupDlg = NULL;
	m_bShowPreview = FALSE;
	m_nCurEMRSelected = -1;
	m_bCurTemplateInfoIsGeneric = FALSE;

	// (a.walling 2007-11-28 10:03) - PLID 28044
	m_bExpired = false;

	m_nGroupIDToAutoExpand = -1;
	m_nRefreshNewListCount = 0; // (a.walling 2010-08-31 12:34) - PLID 29140 - Prevent refreshing the new list more often than necessary
}

void CPatientNexEMRDlg::RecallDetails()
{
	// (b.cardillo 2006-11-20 10:37) - PLID 22565 - First reset our last notification, so that 
	// we always notify them when they open the NexEMR tab.
	m_nReconstructedDetailsLastNotifiedPatientID = -1;
	
	// Then call the base class
	CPatientDialog::RecallDetails();
}

CPatientNexEMRDlg::~CPatientNexEMRDlg()
{
	if(m_pFont) {
		delete m_pFont;
		m_pFont = NULL;
	}

	// Destroy our thread objects
	KillEMRListThread(240);
}

void CPatientNexEMRDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatientNexEMRDlg)	
	DDX_Control(pDX, IDC_BTN_EMR_ANALYSIS, m_btnEMRAnalysis);
	DDX_Control(pDX, IDC_BTN_PT_SUMMARY, m_btnPatSummary);
	DDX_Control(pDX, IDC_NEXEMR_SHOW_PREVIEW, m_btnShowPreview);
	DDX_Control(pDX, IDC_CONFIGURE_NEXEMR_COLUMNS, m_btnConfigureColumns);
	DDX_Control(pDX, IDC_BTN_CONFIGURE_TABS, m_btnConfigureTabs);
	DDX_Control(pDX, IDC_TAB_VIEW_CHECK, m_btnTabViewCheck);
	DDX_Control(pDX, IDC_BTN_EMR_SEENTODAY, m_btnEMRPatientsSeenToday);
	DDX_Control(pDX, IDC_BTN_EMR_SUMMARY, m_btnEMRSummary);
	DDX_Control(pDX, IDC_BTN_PATIENTS_TO_BE_BILLED, m_btnPatientsToBeBilled);
	DDX_Control(pDX, IDC_EDIT_EMN_TEMPLATE, m_btnEditTemplates);
	DDX_Control(pDX, IDC_NEXEMR_CONFIGURE_GROUPS, m_btnConfigureGroups);
	DDX_Control(pDX, IDC_NEXEMR_LOCK_MANAGER, m_btnLockManager);
	DDX_Control(pDX, IDC_BTN_EMR_PROBLEM_LIST, m_btnEMRProblemList);
	DDX_Control(pDX, IDC_NXC_BUTTONS, m_bkgLeft);
	DDX_Control(pDX, IDC_NXC_DETAILS, m_bkgTop);
	DDX_Control(pDX, IDC_NXC_EMR_BOTTOM, m_bkgBottom);
	DDX_Control(pDX, IDC_NXL_PATIENT_CHART , m_nxlPatientChart);
	DDX_Control(pDX, IDC_EMR_BUTTON_HEADER, m_nxstaticEmrButtonHeader);
	DDX_Control(pDX, IDC_SHOW_HISTORY, m_btnShowHistory);
	DDX_Control(pDX, IDC_BTN_PATIENT_WELLNESS, m_btnWellness);
	DDX_Control(pDX, IDC_NXL_NEW_MEDICAL_NOTE, m_nxlNewMedicalNote);		// (j.armen 2012-04-23 14:43) - PLID 49608 - Added label for New Medical Note
	// (j.armen 2012-04-24 17:07) - PLID 49863 - Universal Templates use the dynamic buttons (of which there are 6)
	DDX_Control(pDX, IDC_EMR_DYNA1, m_btnDynamic1);
	DDX_Control(pDX, IDC_EMR_DYNA2, m_btnDynamic2);
	DDX_Control(pDX, IDC_EMR_DYNA3, m_btnDynamic3);
	DDX_Control(pDX, IDC_EMR_DYNA4, m_btnDynamic4);
	DDX_Control(pDX, IDC_EMR_DYNA5, m_btnDynamic5);
	DDX_Control(pDX, IDC_EMR_DYNA6, m_btnDynamic6);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPatientNexEMRDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPatientNexEMRDlg)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_NEXEMRTABPOPUP_RENAME_EMR, OnRenameEMR)
	ON_COMMAND(ID_NEXEMRTABPOPUP_EDITEMR, OnEditEMR)
	ON_COMMAND(ID_NEXEMRTABPOPUP_DELETEEMR, OnDeleteEMR)
	ON_COMMAND(ID_NEXEMRTABPOPUP_VIEWHISTORY, OnViewHistory)
	ON_COMMAND(ID_NEXEMRTABPOPUP_CREATENEWEMR, OnCreateEMR)
	ON_BN_CLICKED(IDC_NEXEMR_CONFIGURE_GROUPS, OnNexemrConfigureGroups)
	ON_BN_CLICKED(IDC_NEXEMR_LOCK_MANAGER, OnNexemrLockManager)
	ON_BN_CLICKED(IDC_EDIT_EMN_TEMPLATE, OnEditEmnTemplate)
	ON_MESSAGE(NXM_EDIT_EMR_OR_TEMPLATE, OnMsgEditEMRTemplate)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_MESSAGE(WM_TABLE_CHANGED_EX, OnTableChangedEx)
	ON_MESSAGE(NXM_NEXEMR_EMRLIST_FINISHED, OnEMRListFinished)
	ON_MESSAGE(NXM_NEXEMR_EMRLIST_WORDICON, OnEMRListWordIcon)
	ON_MESSAGE(NXM_NEXEMR_EMRLIST_PROBLEM_ICON, OnEMRListProblemIcon)
	ON_MESSAGE(NXM_NEXEMR_EMRLIST_COLOR, OnEMRListColor)
	ON_MESSAGE(NXM_NEXEMR_EMRLIST_COMPLETION_STATUS, OnEMRListCompletionStatus)
	ON_BN_CLICKED(IDC_BTN_PATIENTS_TO_BE_BILLED, OnBtnPatientsToBeBilled)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_MORE_TEMPLATES, OnMoreTemplates)
	ON_COMMAND(ID_NEXEMRTABPOPUP_LOCKEMN, OnLockEMN)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_EMR_SUMMARY, OnBtnEmrSummary)
	ON_COMMAND(ID_NEXEMRTABPOPUP_MOVEEMN_TOEMR, OnMoveEMNToEMR)
	ON_BN_CLICKED(IDC_BTN_EMR_PROBLEM_LIST, OnBtnEmrProblemList)
	ON_BN_CLICKED(IDC_BTN_EMR_SEENTODAY, OnBtnEmrSeenToday)
	ON_BN_CLICKED(IDC_TAB_VIEW_CHECK, OnTabViewCheck)
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_TABS, OnBtnConfigureTabs)
	ON_COMMAND(ID_CONFIGURE_CATEGORIES, OnConfigureCategories)
	ON_BN_CLICKED(IDC_CONFIGURE_NEXEMR_COLUMNS, OnConfigureNexemrColumns)
	ON_BN_CLICKED(IDC_NEXEMR_SHOW_PREVIEW, OnShowPreview)
	ON_COMMAND(ID_NEXEMRTABPOPUP_SHOWPREVIEW, OnMenuShowPreview)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_SHOW_HISTORY, OnShowHistory)
	ON_COMMAND(ID_CONFIGURE_CHARTS, OnConfigureCategories)
	ON_BN_CLICKED(IDC_BTN_PT_SUMMARY, OnBtnPtSummary)
	ON_BN_CLICKED(IDC_BTN_EMR_ANALYSIS, OnBtnEMRAnalysis)
	ON_BN_CLICKED(IDC_BTN_PATIENT_WELLNESS, OnBtnWellness)	
	ON_COMMAND(ID_NEXEMRTABPOPUP_PUBLISHEMNTONEXWEB, OnPublishEMNToNexWeb)
	ON_COMMAND(ID_GENERATECLINICALSUMMARY_XMLONLY, OnGenerateClinicalSummaryXMLOnly) // (j.gruber 2013-11-11 15:39) - PLID 59415 
	/*ON_COMMAND(ID_GENERATECLINICALSUMMARY_PDFONLY, OnGenerateClinicalSummaryPDFOnly) // (j.gruber 2013-11-11 15:39) - PLID 59415  // (j.gruber 2014-05-06 14:05) - PLID 61923 - taken out
	ON_COMMAND(ID_GENERATECLINICALSUMMARY_XMLANDPDF, OnGenerateClinicalSummaryXMLAndPDF) // (j.gruber 2013-11-11 15:39) - PLID 59415 // (j.gruber 2014-05-06 14:05) - PLID 61923 - taken out*/
	ON_COMMAND(ID_GENERATECLINICALSUMMARY_CUSTOMIZED, OnGenerateClinicalSummaryCustomized) // (j.gruber 2013-11-13 15:39) - PLID 59420
	ON_COMMAND(ID_GENERATECLINICALSUMMARY_CANCERCASESUBMISSION, OnGenerateClinicalSummaryCancerCaseSubmission)
	// (j.armen 2012-04-24 17:08) - PLID 49863 - Handle clicking the dynamic buttons
	ON_BN_CLICKED(IDC_EMR_DYNA1, OnBnClickedEmrDyna1)
	ON_BN_CLICKED(IDC_EMR_DYNA2, OnBnClickedEmrDyna2)
	ON_BN_CLICKED(IDC_EMR_DYNA3, OnBnClickedEmrDyna3)
	ON_BN_CLICKED(IDC_EMR_DYNA4, OnBnClickedEmrDyna4)
	ON_BN_CLICKED(IDC_EMR_DYNA5, OnBnClickedEmrDyna5)
	ON_BN_CLICKED(IDC_EMR_DYNA6, OnBnClickedEmrDyna6)
	// (j.fouts 2012-06-06 11:02) - PLID 49611 - Handle Preferences being updated
	ON_MESSAGE(WM_PREFERENCE_UPDATED, OnPreferenceUpdated)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPatientNexEMRDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPatientNexEMRDlg)
	ON_EVENT(CPatientNexEMRDlg, IDC_NEW_LIST, 17 /* ColumnClicking */, OnColumnClickingNewList, VTS_I2 VTS_PBOOL)
	ON_EVENT(CPatientNexEMRDlg, IDC_NEW_LIST, 26 /* RowExpanded */, OnRowExpandedNewList, VTS_DISPATCH)
	ON_EVENT(CPatientNexEMRDlg, IDC_NEW_LIST, 19 /* LeftClick */, OnLeftClickNewList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientNexEMRDlg, IDC_EXISTING_LIST, 19 /* LeftClick */, OnLeftClickExistingList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientNexEMRDlg, IDC_EXISTING_LIST, 17 /* ColumnClicking */, OnColumnClickingExistingList, VTS_I2 VTS_PBOOL)
	ON_EVENT(CPatientNexEMRDlg, IDC_EXISTING_LIST, 6 /* RButtonDown */, OnRButtonDownExistingList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientNexEMRDlg, IDC_EXISTING_LIST, 2 /* SelChanged */, OnSelChangedExistingList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CPatientNexEMRDlg, IDC_EXISTING_LIST, 18 /* RequeryFinished */, OnRequeryFinishedExistingList, VTS_I2)
	ON_EVENT(CPatientNexEMRDlg, IDC_NEW_LIST, 18 /* RequeryFinished */, OnRequeryFinishedNewList, VTS_I2)
	ON_EVENT(CPatientNexEMRDlg, IDC_EMN_TABS, 1 /* SelectTab */, OnSelectTabEMNTabs, VTS_I2 VTS_I2)
	ON_EVENT(CPatientNexEMRDlg, IDC_EXISTING_LIST, 10 /* EditingFinished */, OnEditingFinishedExistingList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPatientNexEMRDlg, IDC_EXISTING_LIST, 28 /* CurSelWasSet */, OnCurSelWasSetExistingList, VTS_NONE)
	ON_EVENT(CPatientNexEMRDlg, IDC_EXISTING_LIST, 9 /* EditingFinishing */, OnEditingFinishingExistingList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPatientNexEMRDlg, IDC_NEW_LIST, 7 /* RButtonUp */, OnRButtonUpNewList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientNexEMRDlg, IDC_CHART_TABS, 1 /* SelectTab */, OnSelectTabChartTabs, VTS_I2 VTS_I2)
	ON_EVENT(CPatientNexEMRDlg, IDC_EXISTING_LIST, 8 /* EditingStarting */, OnEditingStartingExistingList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatientNexEMRDlg message handlers

BOOL CPatientNexEMRDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		//Initialize datalists
		m_pExistingList = BindNxDataList2Ctrl(IDC_EXISTING_LIST, false);
		m_pNewTree = BindNxDataList2Ctrl(IDC_NEW_LIST, false);

		// (j.armen 2012-04-23 14:04) - PLID 49614 - This datalist should be bolded!
		// (a.walling 2012-08-16 08:38) - PLID 52164 - Fixed memory leak -- GetFont() returns a raw IFontDisp* which must be released.
		IFontPtr(IFontDispPtr(m_pNewTree->GetFont(), false))->put_Bold(TRUE);

		// (a.walling 2007-11-28 10:03) - PLID 28044 - we are expired if HasEMR returns false and we are in this dialog anyway
		m_bExpired = g_pLicense->HasEMR(CLicense::cflrSilent) != 2;

		// Initialize sort priorities

		// (j.jones 2012-07-20 08:44) - PLID 41791 - This will default the sorting to be
		// service date DESC, input date DESC. For Tab View, the user can change the sort.
		// For Tree View, it is unchangeable.
		InitializeExistingListSortOrder();

		// (j.fouts 2012-06-05 12:29) - PLID 49611 - Moved the bulk cache to a seperate method
		ReloadProperties();

		//DRT 4/14/2008 - PLID 29636 - NxIconify
		m_btnConfigureGroups.AutoSet(NXB_MODIFY);
		m_btnEditTemplates.AutoSet(NXB_MODIFY);
		m_btnConfigureColumns.AutoSet(NXB_MODIFY);
		m_btnConfigureTabs.AutoSet(NXB_MODIFY);
		m_btnShowPreview.AutoSet(NXB_INSPECT); // (a.walling 2008-04-30 09:13) - PLID 29842 - Magnifying glass icon
		m_btnEMRSummary.SetIcon(IDI_EMRSUMMARY, 24, TRUE); // (a.walling 2008-04-30 11:01) - PLID 29842 - EMR Summary icon
		m_btnEMRPatientsSeenToday.SetIcon(IDI_PATIENTSSEEN, 24, TRUE); // (a.walling 2008-04-30 12:30) - PLID 29842 - Patients Seen Today
		m_btnEMRProblemList.AutoSet(NXB_PROBLEM); // (a.walling 2008-04-30 13:22) - PLID 29842 - EMR Problem flag
		m_btnPatientsToBeBilled.SetIcon(IDI_WRITECHECK, 24, FALSE); // (a.walling 2008-04-30 13:27) - PLID 29842 - Write check icon for patients to be billed
		m_btnPatSummary.AutoSet(NXB_INSPECT);	// (j.jones 2008-07-29 11:21) - PLID 30873
		m_btnEMRAnalysis.AutoSet(NXB_INSPECT);	// (j.jones 2008-10-14 16:59) - PLID 14411
		m_btnWellness.AutoSet(NXB_MODIFY); // (j.gruber 2009-05-26 15:51) - PLID 34348 - added button to access wellness dlg
		m_btnLockManager.AutoSet(NXB_MODIFY); // (z.manning 2009-08-25 09:25) - PLID 31944

		// (a.walling 2007-02-01 10:39) - PLID 24513 - Unsafe unsynchronized boolean to signal the thread to cancel.
		// m_bCancelThread = false;
		// (a.walling 2007-04-23 09:16) - PLID 24535 - Now we use sync objects

		//Bold the static text areas
		// (a.walling 2008-06-11 08:37) - PLID 30351 - Antialiased quality for the font is much easier on the eyes
		// (a.walling 2008-11-18 09:25) - PLID 31956 - We are now cleartype-safe, so use DEFAULT_QUALITY
		m_pFont = new CFont;
		// (a.walling 2016-06-01 11:12) - NX-100195 - use Segoe UI
		m_pFont->CreateFont(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));

		// (d.thompson 2009-11-03) - PLID 36112 - Removed the 'Open a...' text control
		// (j.armen 2012-04-23 14:43) - PLID 49608 - Added label for New Medical Note
		m_nxlNewMedicalNote.SetFont(m_pFont);
		m_nxlPatientChart.SetFont(m_pFont);
		m_nxstaticEmrButtonHeader.SetFont(m_pFont);

		// Create the template manager dialog
		m_pdlgTemplateManager = new CEmrTemplateManagerDlg(this);

		// (c.haag 2006-07-03 11:21) - PLID 19977 - Load the EMR problem flag icon
		m_hIconHasProblemFlag = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_FLAG), IMAGE_ICON, 16,16, 0);
		m_hIconHadProblemFlag = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_EMR_PROBLEM_CLOSED), IMAGE_ICON, 16,16, 0);
		m_hIconPreview = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_INSPECT), IMAGE_ICON, 16,16, 0);
		// (r.gonet 04/22/2014) - PLID 61807 - Load the Clinical Summary Status icon
		m_hIconClinicalSummaryMerged = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_CLIN_SUM_MERGED), IMAGE_ICON, 16, 16, 0);
		m_hIconClinicalSummaryNexWeb = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_CLIN_SUM_NEXWEB), IMAGE_ICON, 16, 16, 0);

		// (a.walling 2006-09-19 14:32) - PLID 18745 - Load the preference to remind to lock aged EMRs.
		m_bRemindToLock = GetRemotePropertyInt("EmnRemindLock", 1, 0, GetCurrentUserName(), true) == 1;

		//  We need to test to see if we are in XP mode.  Since we're
		//	not actually drawing, I just open this open, and set a member variable.
		{
			UXTheme* pTheme = new UXTheme();
			pTheme->OpenThemeData(GetDlgItem(IDC_NEW)->GetSafeHwnd(), "Button");	//this doesn't matter

			if(pTheme->IsOpen())
				m_bThemeOpen = TRUE;
			else
				m_bThemeOpen = FALSE;

			//cleanup
			pTheme->CloseThemeData();
			delete pTheme;
		}

		m_pEMNTabs = GetDlgItemUnknown(IDC_EMN_TABS);
		// (a.wetta 2007-06-28 11:14) - PLID 26094 - Initialize the chart tabs
		m_pChartTabs = GetDlgItemUnknown(IDC_CHART_TABS);
		if (m_pEMNTabs == NULL || m_pChartTabs == NULL)
		{
			HandleException(NULL, "Failed to bind NxTab control", __LINE__, __FILE__);
			PostMessage(WM_COMMAND, IDCANCEL);
			return 0;
		}
		else {
			RefreshEMNTabs();
		}
		m_pEMNTabs->CurSel = m_pEMNTabs->GetSize()-1;
		m_pChartTabs->CurSel = 0;

		// (j.jones 2016-04-20 11:01) - NX-100214 - Set HeaderMode to false, which will
		// use a slightly different theme than the module tabs use.
		// A HeaderMode of false looks nicer when the tab is next to a datalist.
		m_pEMNTabs->HeaderMode = false;
		m_pChartTabs->HeaderMode = false;

		// (z.manning, 08/17/2007) - PLID 27109 - The EMR tab view option is now a global preference.
		if (GetRemotePropertyInt("EMRTabView", 0, 0, "<None>", true)) {
			CheckDlgButton(IDC_TAB_VIEW_CHECK, 1);
			m_bEMNTabView = TRUE;
			// (z.manning 2008-07-01 10:57) - PLID 25574 - If we're in tab view then show the history
			// checkbox option. (only if they have history permission)
			if(CheckCurrentUserPermissions(bioPatientHistory, sptRead, FALSE, 0, TRUE, TRUE)) {
				m_btnShowHistory.ShowWindow(SW_SHOW);
				m_btnShowHistory.SetCheck(GetRemotePropertyInt("NexEMRTabShowHistory", BST_UNCHECKED, 0, GetCurrentUserName(), true));
			}
		}
		else {
			CheckDlgButton(IDC_TAB_VIEW_CHECK, 0);
			m_bEMNTabView = FALSE;
			m_btnShowHistory.ShowWindow(SW_HIDE);
			m_btnShowHistory.SetCheck(BST_UNCHECKED);
		}

		// (a.wetta 2007-02-21 16:44) - PLID 24405 - Clear out the EMR list before changing the columns of the datalist, 
		// this will prevent an annoying moving around of the columns
		m_pExistingList->Clear();
		EnsureTabsPosition();

		// (a.walling 2007-11-28 10:02) - PLID 28044 - Moved this to the end of InitDialog.
		// (c.haag 2006-03-28 16:11) - PLID 19890 - Secure controls so that users who don't have
		// access can't use certain functions
		SecureControls();

		// (a.walling 2007-04-13 09:49) - PLID 25648 - Load and initialize our preview popup
		// (a.walling 2007-04-16 14:04) - PLID 25648 - Moved all this to CreatePreviewWindow, which is called
			// when needed to prevent any delay when initially opening the NexEMR window.

		// (a.walling 2008-10-10 09:58) - PLID 31651 - This should only happen once when the dialog is created
		CheckUnlockedAgedEMNs();

		// (b.savon 2011-12-19 17:45) - PLID 47074 - Show EMR Facelift Webinar Message
		CViewWebinarDlg dlg;
		dlg.DoModal("NexTech has given EMR a facelift.  We have recorded a webinar to aid in the transition.  Please click the link below and navigate to the 'EMR Facelift' webinar.",
					"EMRFaceliftWebinar", "Practice - EMR Facelift Webinar", "Watch the Webinar Now",
					ewaEMR);

	} NxCatchAll("Error in OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (j.fouts 2012-06-06 11:04) - PLID 49611 - Move bulk cache so that it can be reloaded when a pref changes
void CPatientNexEMRDlg::ReloadProperties()
{
	g_propManager.BulkCache("PatientNexEMRDlg", propNumber | propbitMemo,
	"(Username = '<None>' OR Username = '%s') AND ("		
	"Name = 'EmnRemindLock' OR "
	"Name = 'EMRTabView' OR "
	"Name = 'EMRColorPartial' OR "
	"Name = 'EMRColorFull' OR "
	"Name = 'EMRColorLocked' OR "
	"Name = 'EMRColorRequired' OR "
	"Name = 'EMRIncludeInactiveLadders' OR "
	"Name = 'ShowEMRPreferredProcedures' OR "
	"Name = 'ShowNexEMRColumnCategory' OR "
	"Name = 'ShowNexEMRColumnChart' OR "
	"Name = 'ShowNexEMRColumnDate' OR "
	"Name = 'ShowNexEMRColumnEMR Description' OR "
	"Name = 'ShowNexEMRColumnEMN Description' OR "
	"Name = 'ShowNexEMRColumnHistory Documents' OR "
	"Name = 'ShowNexEMRColumnProblem' OR "
	"Name = 'ShowNexEMRColumnStatus' OR "
	"Name = 'ShowNexEMRColumnProvider' OR "		// (d.thompson 2010-01-08) - PLID 35925
	"Name = 'ShowNexEMRColumnSecondary Provider' OR " // (z.manning 2010-05-11 16:45) - PLID 38354
	"Name = 'ShowNexEMRColumnAssistant/Technician' OR "	// (d.lange 2011-04-29 15:34) - PLID 43382 - Added Assistant/Technician. //(e.lally 2011-08-26) PLID 44950 - Fixed spelling
	"Name = 'EMRPreviewPopupWidth' OR "
	"Name = 'EMRPreviewPopupHeight' OR "
	"Name = 'EMRPreviewPopupTop' OR "
	"Name = 'EMRPreviewPopupLeft' OR "
	"Name = 'NexEMRTabShowHistory' OR " // (z.manning 2008-07-01 11:08) - PLID 25574
	"Name = 'ShowNexEMRColumnPreview Icon' OR " // (z.manning 2008-07-02 14:14) - PLID 30596
	"Name = 'EMRAutoExpandGroupID' OR " // (z.manning 2009-08-11 16:49) - PLID 32989
	"Name = 'PromptAddNewEMNsToEMR' "	// (j.jones 2009-08-28 11:25) - PLID 29185
	"OR Name = 'ShowWellnessButton' OR "	// (d.thompson 2009-09-17) - PLID 35581
	"Name = 'WarnWhenCreatingNexWebEmn' OR "	//TES 11/13/2009 - PLID 35807
	"Name = 'PromptAddNewEMNsToLadder' "	// (j.jones 2010-04-16 14:27) - PLID 38236
	"OR Name = 'EmnRemindLockDays' "		//(e.lally 2011-08-26) PLID 44950
	"OR Name = 'EmnRemindField' "			//(e.lally 2011-08-26) PLID 44950
	"OR Name = 'PhotoViewerSortCriteria' " //(e.lally 2012-04-25) PLID 49634
	"OR Name = 'PhotoViewerSortCriteriaOrder' " //(e.lally 2012-04-25) PLID 49634
	"OR Name = 'EmrPhotoTabType' " //(e.lally 2012-04-30) PLID 49636
	"OR Name = 'NexEMRGridlines' "
	"OR Name = 'RequireCPTCodeEMNLocking' " 
	"OR Name = 'RequireDiagCodeEMNLocking' "
	"OR Name = 'ShowNexEMRColumnClinical Summary Icon' " // (r.gonet 04/22/2014) - PLID 61807

	//These are all memo values
	"OR Name = 'MU_ExcludedTemplates' "		//(e.lally 2012-04-05) PLID 49378 
	"OR Name = 'CancerCaseExport_LastFolder' " //TES 5/7/2015 - PLID 65969
	")",
	_Q(GetCurrentUserName()));

	// (j.fouts 2012-06-04 17:18) - PLID 49611 - Set these depending on the preferences
	long nGridStylePref = GetRemotePropertyInt("NexEMRGridlines", 2, 0,  GetCurrentUserName(), true);
	if(nGridStylePref == 1)
	{
		m_pExistingList->GridVisible = VARIANT_FALSE;
	}
	else if(nGridStylePref == 2)
	{
		m_pExistingList->GridVisible = VARIANT_TRUE;
		m_pExistingList->GridlineSize = 2;
	}
}

// (j.fouts 2012-06-06 11:04) - PLID 49611 - Handle the updating of preferences
LRESULT CPatientNexEMRDlg::OnPreferenceUpdated(WPARAM wParam, LPARAM lParam)
{
	try
	{
		ReloadProperties();
	}
	NxCatchAll(__FUNCTION__);

	return 0;
}

void CPatientNexEMRDlg::OnDestroy() 
{
	KillEMRListThread(240);

	// (a.walling 2007-04-13 10:02) - PLID 25648 - Save our positioning info
	if (m_pEMRPreviewPopupDlg) {
		// (a.walling 2010-01-11 12:39) - PLID 36837 - Position info is saved in the dialog itself now

		m_pEMRPreviewPopupDlg->DestroyWindow();
		delete m_pEMRPreviewPopupDlg;
		m_pEMRPreviewPopupDlg = NULL;
	}

	if (m_pdlgTemplateManager) {
		delete m_pdlgTemplateManager;
		m_pdlgTemplateManager = NULL;
	}

	// (a.walling 2008-06-03 09:51) - PLID 27686 - This was leaking 2 handles each time
	DestroyIcon(m_hIconHasProblemFlag);
	DestroyIcon(m_hIconHadProblemFlag);
	DestroyIcon(m_hIconPreview);
	// (r.gonet 04/22/2014) - PLID 61807 - Clean up the Clinical Summary icon
	DestroyIcon(m_hIconClinicalSummaryMerged);
	DestroyIcon(m_hIconClinicalSummaryNexWeb);

	m_hIconHasProblemFlag = NULL;
	m_hIconHadProblemFlag = NULL; 
	m_hIconPreview = NULL;
	// (r.gonet 04/22/2014) - PLID 61807 - Clean up the clinical summary icons.
	m_hIconClinicalSummaryMerged = NULL;
	m_hIconClinicalSummaryNexWeb = NULL;

	CNxDialog::OnDestroy();
}

void CPatientNexEMRDlg::SetColor(OLE_COLOR nNewColor)
{
	m_bkgLeft.SetColor(nNewColor);
	m_bkgTop.SetColor(nNewColor);
	// (j.jones 2016-04-18 16:48) - NX-100214 - added a third colored background
	m_bkgBottom.SetColor(nNewColor);
	m_pEMNTabs->BackColor = nNewColor;
	m_pChartTabs->BackColor = nNewColor;

	CPatientDialog::SetColor(nNewColor);
}

void CPatientNexEMRDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {
		// (a.walling 2010-10-14 17:30) - PLID 40978
		long nCurrentlyLoadedID = m_id;
		m_id = GetActivePatientID();

		// (d.thompson 2009-09-17) - PLID 35581 - If the preference is set to hide, do not display
		//	this button.
		if(GetRemotePropertyInt("ShowWellnessButton", 1, 0, "<None>", true) == 0) {
			m_btnWellness.EnableWindow(FALSE);
			m_btnWellness.ShowWindow(SW_HIDE);
		}
		else {
			//Show it
			m_btnWellness.EnableWindow(TRUE);
			m_btnWellness.ShowWindow(SW_SHOW);
		}
		
		// (a.walling 2010-10-14 17:34) - PLID 40978
		if (nCurrentlyLoadedID != m_id) {
			m_ForceRefresh = true;
		}

		if (bForceRefresh || m_ForceRefresh) {
			if ((bForceRefresh && !m_ForceRefresh)) {
				//(e.lally 2011-10-11) PLID 44728 - The list of templates needs updated when the Refresh button is clicked
				m_nRefreshNewListCount = 0;
			}
			Refresh();
		}
		m_ForceRefresh = false;

	} NxCatchAll("Error in UpdateView");
}

// (a.walling 2010-10-14 17:32) - PLID 40978
void CPatientNexEMRDlg::Refresh()
{
	try {
		// (j.jones 2007-06-11 11:17) - PLID 26271 - clear our current template info
		ClearCurEMRTemplateInfo();

		// (a.wetta 2007-02-21 16:44) - PLID 24405 - Clear out the EMR list before changing the columns of the datalist, 
		// this will prevent an annoying moving around of the columns
		m_pExistingList->Clear();
		EnsureTabsPosition();
		ReloadEMRList();
		// (a.walling 2010-08-31 12:34) - PLID 29140 - Prevent refreshing the new list more often than necessary
		// (j.jones 2014-08-21 11:33) - PLID 63189 - added tablechecker object for EMR templates
		if (m_nRefreshNewListCount == 0 || m_templateChecker.Changed()) {
			RefreshNewList();
		}
		//(e.lally 2011-10-11) PLID 44728 - If the procedure based section is showing,  
		//	refresh the per patient active NexTrack procedures
		else if (GetRemotePropertyInt("ShowEMRPreferredProcedures", 1)) {
			RefreshNewTreeActiveNexTrackProcedures();
		}
		
		CheckEmrProviderLicensing();
		// (z.manning 2014-02-04 15:37) - PLID 55001 - Also prompt for Nuance dictation licenses
		CheckDictationLicensing();

		// (a.walling 2008-10-10 09:58) - PLID 31651 - This should only happen once when the dialog is created
		//CheckUnlockedAgedEMNs();

		// (a.walling 2007-04-13 10:56) - PLID 25648 - Update the preview popup with the new patient id
		if (m_pEMRPreviewPopupDlg) {

			// (j.jones 2009-09-22 11:55) - PLID 31620 - Send an empty list of EMNs now,
			// OnRequeryFinishedExistingList will fill the preview pane once the EMNs
			// are loaded into the list.
			// (a.walling 2010-08-05 15:40) - PLID 40015 - This will not affect the preview popup if the same patient.
			CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
			m_pEMRPreviewPopupDlg->SetPatientID(GetActivePatientID(), aryEMNs);
		}

		// (j.gruber 2009-06-04 12:35) - PLID 34484 - update the wellness records/button appropriately
		RefreshWellnessAlerts();
		m_ForceRefresh = false;
	} NxCatchAllThrow(__FUNCTION__);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//		This section of code is all related to redrawing and interacting with the list of EMRs		//
//////////////////////////////////////////////////////////////////////////////////////////////////////

//Call to reload the entire EMR / EMN list.  This function will clear the list and reload it from data.
//	No Parameters
//	No Return Value
//This function catches its own exceptions
void CPatientNexEMRDlg::ReloadEMRList()
{
	try {
		//TODO - Save the current state of things, position, which EMRs are dropped down, etc.

		//Clear any buttons
		ClearDynaButtons();

		//If we're already processing, abort and restart
		KillEMRListThread(240);

		// (j.armen 2012-02-14 11:10) - PLID 48131 - Store these as CSqlFragments now
		CSqlFragment sqlFrom;
		CSqlFragment sqlWhere;
		CSqlFragment sqlComboSource;

		// (j.jones 2007-06-19 10:10) - PLID 26276 - grab the colors to put them in the query
		// (a.walling 2012-05-31 14:49) - PLID 50719 - EmrColors
		COLORREF cPartial = EmrColors::Topic::Partial();
		COLORREF cFull = EmrColors::Topic::Complete();
		COLORREF cNone = RGB(255,255,255);
		// (j.jones 2007-06-15 09:30) - PLID 26297 - added locked color preference
		COLORREF cLocked = EmrColors::Topic::Locked();
		// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details default color
		COLORREF cRequired = EmrColors::Topic::Required();

		// (z.manning, 03/10/2008) - PLID 29243 - Reworked this to make sure that reconstructed detail status
		// color has the highest priority.
		// (j.armen 2012-02-14 11:12) - PLID 48131 - Store as CSqlFragment
		// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details.  Also cleaned up the completion status numeric literals to use the enum values instead
		CSqlFragment sqlColorQuery(
			"CASE "
			"	WHEN EMRMasterT.CompletionStatus = {CONST_INT} THEN {CONST_INT} "
			"	WHEN EMRMasterT.Status = {CONST_INT} THEN {CONST_INT} "
			"	WHEN EMRMasterT.CompletionStatus = {CONST_INT} THEN {CONST_INT} "
			"	WHEN EMRMasterT.CompletionStatus = {CONST_INT} THEN {CONST_INT} "
			"	WHEN EMRMasterT.CompletionStatus = {CONST_INT} THEN {CONST_INT} "
			"	WHEN EMRMasterT.CompletionStatus = {CONST_INT} THEN {CONST_INT} "
			"	ELSE {INT} "
			"	END"
			, ecsReconstructed, GetReconstructedEMRDetailReviewStateColor(-1)
			, 2, (long)cLocked
			, ecsEmpty, (long)cNone
			, ecsPartiallyComplete, (long)cPartial
			, ecsComplete, (long)cFull
			, ecsRequired, (long)cRequired
			, (long)cNone);

		// (a.wetta 2007-01-04 11:22) - PLID 14635 - If they are using the tab view, then we want to view all EMNs without the EMRs
		// (z.manning, 03/27/2007) - PLID 25363 - Removed the logic to determine if an EMR/EMN has a Word document
		// from these queries. We now do that after the requery has finished in the FinishEMRListThread.
		// (z.manning, 04/04/2007) - PLID 25489 - Added support to both queries for EmnTabChartsLinkT.
		// (z.manning, 04/06/2007) - PLID 25518 - Added a separate description column for EMR description (for use in tab view).
		// (z.manning, 04/06/2007) - PLID 25527 - For tab view only, we load the template name for EMN description if
		// the actual description is blank.
		// (j.jones 2007-06-14 10:06) - PLID 26276 - added CompletionStatus
		if (m_bEMNTabView) {

			// (a.walling 2009-05-06 09:35) - PLID 34176 - Added support for SELECTION_CCD
			// (j.armen 2012-02-14 11:13) - PLID 48131 - Store as CSqlFragments
			// (j.gruber 2013-11-08 11:19) - PLID 59375 - include CCDA
			//TES 11/14/2013 - PLID 57415 - Include Cancer Case submissions
			CSqlFragment sqlEmnCategoryWhere;
			CSqlFragment sqlHistoryWhere(
				"WHERE Selection IN ({STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}, {STRING}) AND PathName <> '' AND substring(PathName, 1, 7) <> 'OBJECT:' AND substring(PathName, 1, 5) <> 'FORM:' "
				, SELECTION_WORDDOC, SELECTION_FILE, SELECTION_JPEG, SELECTION_AUDIO, SELECTION_CCD, SELECTION_CCDA, SELECTION_CANCERCASE);
			if(m_pEMNTabs->GetSize()-1 != m_pEMNTabs->CurSel) {
				long nTabCat = m_aryEmnCategoryTabs.GetAt(m_pEMNTabs->CurSel).nID;
				sqlEmnCategoryWhere += CSqlFragment(" AND EMNTabCategoriesLinkT.EMNTabCategoryID = {INT} ", nTabCat);
				sqlHistoryWhere += CSqlFragment(
					" AND MailSent.CategoryID IN (SELECT NoteCategoryID FROM EmrHistoryCategoryLinkT WHERE EmnTabCategoryID = {INT}) "
					, nTabCat);
			}
			// (z.manning, 04/04/2007) - PLID 25489 - If we have a selected chart, make sure we filter on that as well.
			// (j.armen 2012-02-14 11:13) - PLID 48131 - Store as CSqlFragment
			CSqlFragment sqlComboWhere;
			long nCurChartID = GetCurSelChartID();
			if(nCurChartID > 0) {
				sqlEmnCategoryWhere += CSqlFragment(" AND EmnTabChartsLinkT.EmnTabChartID = {INT} ", nCurChartID);

				// (z.manning, 05/23/2007) - PLID 25489 - If we have a specific chart tab selected
				// let's filter the category dropdown based on that.
				sqlComboWhere.Create(" WHERE EmnTabCategoriesT.ID IN (SELECT EmnTabCategoryID FROM EmnTabChartCategoryLinkT WHERE EmnTabChartID = {INT}) ", nCurChartID);
			}

			// (z.manning 2008-07-01 08:40) - PLID 25574 - We now also have the option of showing history
			// documents here.
			// (j.armen 2012-02-14 11:13) - PLID 48131 - Store as CSqlFragments
			CSqlFragment sqlHistoryFrom;
			if(m_btnShowHistory.GetCheck() == BST_CHECKED)
			{
				// (d.lange 2010-02-08 13:26) - PLID 36559 - The {All Categories} tab showed all the history documents
				if (m_pEMNTabs->GetSize()-1 == m_pEMNTabs->CurSel) {
					if (nCurChartID > 0) {
						sqlHistoryWhere += CSqlFragment(
							" AND MailSent.CategoryID IN (SELECT NoteCategoryID FROM EmnTabChartCategoryLinkT INNER JOIN EmrHistoryCategoryLinkT"
							" ON EmnTabChartCategoryLinkT.EmnTabCategoryID = EmrHistoryCategoryLinkT.EmnTabCategoryID WHERE EmnTabChartID = {INT})", nCurChartID);
					} else {
						sqlHistoryWhere += CSqlFragment(
							" AND MailSent.CategoryID IN (SELECT NoteCategoryID FROM EmrHistoryCategoryLinkT)");
					}
				}
				// (j.jones 2008-09-05 12:52) - PLID 30288 - supported MailSentNotesT
				// (d.thompson 2010-01-21) - PLID 35925 - Added blank as provider name
				// (z.manning 2010-05-11 16:45) - PLID 38354 - Added blank name as sec. provider
				// (d.lange 2011-04-29 15:35) - PLID 43382 - Added Assistant/Technician
				// (z.manning 2012-09-10 14:48) - PLID 52543 - Added modified date
				// (c.haag 2013-02-28) - PLID 55368 - Added template ID
				// (r.gonet 04/22/2014) - PLID 61807 - No clinical summary icon for history rows.
				sqlHistoryFrom.Create(" \r\nUNION \r\n"
					"SELECT NULL AS ID, NULL AS ParentID, NULL AS EMRID, NULL AS EMNID, MailSent.PersonID AS PatientID, "
					"	MailSent.Date AS MaxDate, MailSent.ServiceDate AS Date, MailSent.Date AS InputDate, "
					"	'<History Document>' AS EmrDescription, '' AS Status, MailSentNotesT.Note AS Description, NULL AS PicID, "
					"	'' AS HasWordDocuments, "
					"	NULL AS ClinicalSummaryStatus, "
					"	0 AS CompletionStatus, {CONST_INT} AS RowColor, "
					"	NULL AS EmnTabChartID, NULL AS EmnTabCategoryID, Selection, MailSent.MailID, PathName, NULL AS PatientCreatedStatus, "
					"	NULL AS ProviderName, NULL AS SecondaryProvider, NULL AS Technician, NULL AS ModifiedDate, NULL AS TemplateID "
					"FROM MailSent "
					"LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
					"{SQL} "
					, cNone, sqlHistoryWhere);
			}

			// (a.wetta 2007-01-04 13:28) - PLID 14636 - When we are using the tab view, do not show the parent EMRs, so the ParentIDs
			// for all of the EMN records must have blank ParentIDs.
			// (j.jones 2008-07-15 17:45) - PLID 30739 - supported EMRRegardingType and EMRRegardingID
			// for all problem types, including showing EMR-level items on each EMN
			// (c.haag 2009-05-12 12:38) - PLID 34234 - Formatted all statements to use new EMR problem link structure
			// (z.manning 2009-12-15 12:57) - PLID 35810 - Added PatientCreatedStatus
			// (d.thompson 2010-01-08) - PLID 35925 - Added provider name
			// (z.manning 2010-05-11 16:45) - PLID 38354 - Secondary provider
			// (c.haag 2010-08-04 11:23) - PLID 39980 - Changed PIC join to a LEFT join to reveal EMR's with no PIC (should be very rare)
			// (d.lange 2011-04-29 15:36) - PLID 43382 - Added Assistant/Technician
			// (z.manning 2011-05-19 17:12) - PLID 33114 - Changed this query to filter out EMNs that are tied to charts
			// that current user does not have permission to.
			// (j.jones 2011-07-05 17:24) - PLID 44432 - supported custom statuses
			// (z.manning 2012-09-10 14:48) - PLID 52543 - Added modified date
			// (c.haag 2013-02-28) - PLID 55368 - Added template ID
			// (r.gonet 04/22/2014) - PLID 61807 - Load the appropriate clinical summary icon depending on whether the 
			// (a.walling 2015-01-28 10:15) - PLID 63919 - Filter MailSent on PersonID to avoid scary joins
			// Clinical Summary was accessed by the patient on NexWeb, merged to Word, or none.
			sqlFrom.Create(
				"( "
				"SELECT NULL AS ID, NULL AS ParentID, EMRMasterT.EMRGroupID AS EMRID, EMRMasterT.ID AS EMNID, "
				"	EMRMasterT.PatientID, EMRMasterT.Date AS MaxDate, EMRMasterT.Date, EMRMasterT.InputDate, EmrGroupsT.Description AS EmrDescription, "
				"	EMRStatusListT.Name AS Status, "
				"	CASE WHEN RTRIM( REPLACE(REPLACE(EmrMasterT.Description,CHAR(10),''),CHAR(13),'') ) = '' THEN '<' + EmrTemplateT.Name + '>' ELSE EmrMasterT.Description END AS Description, "
				"	PicT.ID AS PicID, '' AS HasWordDocuments, "
				"	CASE WHEN COALESCE(ClinicalSummaryStatusQ.Accessed, 0) = 1 THEN {CONST_INT} WHEN COALESCE(ClinicalSummaryStatusQ.Merged, 0) = 1 THEN {CONST_INT} ELSE NULL END AS ClinicalSummaryStatus, "
				"	EMRMasterT.CompletionStatus, {SQL} AS RowColor, EmnTabChartsLinkT.EmnTabChartID, EMNTabCategoriesLinkT.EMNTabCategoryID, "
				"	NULL AS Selection, NULL AS MailID, NULL AS PathName, "
				"	EmrMasterT.PatientCreatedStatus, dbo.GetEmnProviderList(EMRMasterT.ID) AS ProviderName, "
				"	dbo.GetEmnSecondaryProviderList(EmrMasterT.ID) AS SecondaryProvider, "
				"	dbo.GetEmnTechniciansT(EmrMasterT.ID) AS Technician, EmrMasterT.ModifiedDate, EMRMasterT.TemplateID "
				"FROM EMRMasterT "
				"INNER JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
				"LEFT JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID "
				"LEFT JOIN EmrTemplateT ON EmrMasterT.TemplateID = EmrTemplateT.ID "
				"LEFT JOIN EMNTabCategoriesLinkT ON EmrMasterT.ID = EMNTabCategoriesLinkT.EMNID "
				"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
				"LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
				"LEFT JOIN "
				"( "
				"	SELECT MailSent.EMNID, 1 AS Merged, (CASE WHEN MAX(NexWebCcdaAccessHistoryT.AccessType) IN (-1,-2,-3) THEN 1 ELSE 0 END) AS Accessed "
					"	FROM MailSent "
					"	LEFT JOIN NexWebCcdaAccessHistoryT ON MailSent.MailID = NexWebCcdaAccessHistoryT.MailSentMailID "
					"	WHERE MailSent.Selection = 'BITMAP:CCDA' AND MailSent.CCDATypeField = 2 "
					"	AND MailSent.PersonID = {INT} "
					"	GROUP BY MailSent.EMNID "
				") ClinicalSummaryStatusQ ON EMRMasterT.ID = ClinicalSummaryStatusQ.EMNID "
				"WHERE (PicT.IsCommitted IS NULL OR PicT.IsCommitted = 1) AND EMRGroupsT.Deleted = 0 AND "
				"	EMRMasterT.Deleted = 0 {SQL} {SQL} \r\n"
				"{SQL} "
				") AS EMRRecordsQ "
				, (long)m_hIconClinicalSummaryNexWeb, (long)m_hIconClinicalSummaryMerged
				, sqlColorQuery, GetActivePatientID(), sqlEmnCategoryWhere, GetEmrChartPermissionFilter(), sqlHistoryFrom);

			sqlWhere.Create(" PatientID = {INT} AND ID IS NULL ", GetActivePatientID());

			// (z.manning, 05/23/2007) - PLID 25489 - Set the category embedded combo's query.
			sqlComboSource.Create(
				"SELECT ID AS EMNTabCategoryID, Description, Priority FROM EMNTabCategoriesT {SQL} "
				"UNION "
				"SELECT -1, '', -1 "
				"ORDER BY Priority ", sqlComboWhere);

			// (j.jones 2008-05-02 11:20) - PLID 28027 - when in tab view, the date column is formatted as date
			NXDATALIST2Lib::IColumnSettingsPtr pDateCol = m_pExistingList->GetColumn(eecDate);
			pDateCol->PutFieldType(NXDATALIST2Lib::cftDateShort);
		}
		else {
			// (a.wetta 2007-01-04 13:42) - This is the ordinary EMR view
			// (z.manning 2008-07-03 12:12) - PLID 25574 - Added necessary fields for history fields that
			// are in the datalist.
			// (j.jones 2008-07-15 17:45) - PLID 30739 - supported EMRRegardingType and EMRRegardingID
			// for all problem types
			// (c.haag 2009-05-12 12:46) - PLID 34234 - Supports new EMR problem linking structure
			// (z.manning 2009-12-15 12:58) - PLID 35810 - Added PatientCreatedStatus
			// (d.thompson 2010-01-08) - PLID 35925 - Added provider name
			// (z.manning 2010-05-11 16:46) - PLID 38354 - Secondary provider
			// (c.haag 2010-08-04 11:23) - PLID 39980 - Changed PIC join to a LEFT join to reveal EMR's with no PIC (should be very rare)
			// (d.lange 2011-04-29 15:38) - PLID 43382 - Added Assistant/Technicians
			// (z.manning 2011-05-19 17:12) - PLID 33114 - Changed this query to filter out EMNs that are tied to charts
			// that current user does not have permission to.
			// (j.jones 2011-07-05 17:24) - PLID 44432 - supported custom statuses
			// (z.manning 2012-09-10 14:49) - PLID 52543 - Added modified date
			// (c.haag 2013-02-28) - PLID 55368 - Added template ID
			// (r.gonet 04/22/2014) - PLID 61807 - Added the display of the clinical summary icon
			// which indicates whether the clinical summary has been accessed by the patient on NexWeb, merged, or neither.
			// It is only displayed on EMN rows.
			sqlFrom.Create(
				"( "
				"SELECT ID, NULL AS ParentID, NULL AS EMNTabCategoryID, NULL AS EmnTabChartID, ID AS EMRID, NULL AS EMNID, PatientID, "
				"	(SELECT Min(Date) AS Date FROM EMRMasterT WHERE Deleted = 0 AND EMRGroupID = EMRGroupsT.ID) AS Date, "
				"	(SELECT Max(Date) AS MaxDate FROM EMRMasterT WHERE Deleted = 0 AND EMRGroupID = EMRGroupsT.ID) AS MaxDate, "
				"	InputDate, '' AS EmrDescription, Description, CASE WHEN Status = 1 THEN 'Closed' ELSE 'Open' END AS Status, PicID, "
				"	'' AS HasWordDocuments, "
				"	NULL AS ClinicalSummaryStatus, "
				"	0 AS CompletionStatus, {CONST_INT} AS RowColor, "
				"	NULL AS Selection, NULL AS MailID, NULL AS PathName, NULL AS PatientCreatedStatus, NULL AS ProviderName, NULL AS SecondaryProvider, "
				"	NULL AS Technician, NULL AS ModifiedDate, NULL AS TemplateID "
				"FROM (SELECT EMRGroupsT.*, PicT.ID AS PicID FROM EMRGroupsT "
				"	LEFT JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID "
				"	WHERE PicT.IsCommitted IS NULL OR PicT.IsCommitted = 1) EMRGroupsT "
				"WHERE Deleted = 0 "
				"UNION "
				"SELECT NULL AS ID, EMRMasterT.EMRGroupID AS ParentID, NULL AS EMNTabCategoryID, NULL As EmnTabChartID, EMRMasterT.EMRGroupID AS EMRID, "
				"	EMRMasterT.ID AS EMNID, EMRMasterT.PatientID, EMRMasterT.Date AS MaxDate, EMRMasterT.Date, "
				"	EMRMasterT.InputDate, '' AS EmrDescription, EMRMasterT.Description, "
				"	EMRStatusListT.Name AS Status, "
				"	PicT.ID AS PicID, '' AS HasWordDocuments, "
				"	CASE WHEN COALESCE(ClinicalSummaryStatusQ.Accessed, 0) = 1 THEN {CONST_INT} WHEN COALESCE(ClinicalSummaryStatusQ.Merged, 0) = 1 THEN {CONST_INT} ELSE NULL END AS ClinicalSummaryStatus, "
				"	EMRMasterT.CompletionStatus, {SQL} AS RowColor, NULL AS Selection, NULL AS MailID, NULL AS PathName, "
				"	EmrMasterT.PatientCreatedStatus, dbo.GetEmnProviderList(EMRMasterT.ID) AS ProviderName, "
				"	dbo.GetEmnSecondaryProviderList(EmrMasterT.ID) AS SecondaryProvider, "
				"	dbo.GetEmnTechniciansT(EmrMasterT.ID) AS Technician, EmrMasterT.ModifiedDate, EmrMasterT.TemplateID "
				"FROM EMRMasterT "
				"INNER JOIN EMRGroupsT ON EMRMasterT.EMRGroupID = EMRGroupsT.ID "
				"LEFT JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID "
				"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
				"LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
				"LEFT JOIN "
				"( "
				"	SELECT MailSent.EMNID, 1 AS Merged, (CASE WHEN MAX(NexWebCcdaAccessHistoryT.AccessType) IN (-1,-2,-3) THEN 1 ELSE 0 END) AS Accessed "
					"	FROM MailSent "
					"	LEFT JOIN NexWebCcdaAccessHistoryT ON MailSent.MailID = NexWebCcdaAccessHistoryT.MailSentMailID "
					"	WHERE MailSent.Selection = 'BITMAP:CCDA' AND MailSent.CCDATypeField = 2 "
					"	AND MailSent.PersonID = {INT} "
					"	GROUP BY MailSent.EMNID "
				") ClinicalSummaryStatusQ ON EMRMasterT.ID = ClinicalSummaryStatusQ.EMNID "
				"WHERE (PicT.IsCommitted IS NULL OR PicT.IsCommitted = 1) AND EMRGroupsT.Deleted = 0 "
				"	AND EMRMasterT.Deleted = 0 {SQL} "
				") AS EMRRecordsQ"
				, (long)cNone
				, (long)m_hIconClinicalSummaryNexWeb, (long)m_hIconClinicalSummaryMerged
				, sqlColorQuery, GetActivePatientID(), GetEmrChartPermissionFilter());

			sqlWhere.Create("PatientID = {INT}", GetActivePatientID());

			// (j.jones 2008-05-02 11:20) - PLID 28027 - when in non-tab view, the date column is formatted as a string
			NXDATALIST2Lib::IColumnSettingsPtr pDateCol = m_pExistingList->GetColumn(eecDate);
			pDateCol->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);
		}

		// (j.armen 2012-02-14 11:15) - PLID 48131 - Place the From and Where Param clauses into the data list
		m_pExistingList->FromClause = _bstr_t(sqlFrom.Flatten());
		m_pExistingList->WhereClause = _bstr_t(sqlWhere.Flatten());
		// (z.manning, 05/23/2007) - PLID 25489 - Make sure we have no combo source when we requery because if we
		// do, then it will run a query to set it as will the PutComboSource call 2 lines later.
		m_pExistingList->GetColumn(eecTabCategoryID)->PutComboSource(_bstr_t(""));
		m_pExistingList->Requery();
		m_pExistingList->GetColumn(eecTabCategoryID)->PutComboSource(_bstr_t(sqlComboSource.Flatten()));

		GetDlgItem(IDC_EMR_BUTTON_HEADER)->ShowWindow(SW_HIDE);
		// (j.jones 2007-06-11 10:50) - PLID 26271 - clear the template info, then refilter buttons
		ClearCurEMRTemplateInfo();
		ReFilterButtons();

	} NxCatchAll("Error in ReloadEMRList");
}

void CPatientNexEMRDlg::OnLeftClickExistingList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {

		// (z.manning, 03/30/2007) - PLID 25427 - Tab view now allows multiple selection, so we need to handle
		// selection changes for it here.
		if(m_bEMNTabView) {
			HandleSelectionChange(GetCurSelExistingList());
		}

		//Nothing to do on empty rows
		if(!lpRow) 
			return;

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		switch(nCol) {
		case eecWordIcon:
			{
				//Word icons are hyperlinks to open the Word document or list of documents
				try {
					_variant_t var = pRow->GetValue(eecEMNID);
					if(var.vt == VT_NULL) {
						//check the PIC record
						_variant_t varPicID = pRow->GetValue(eecPicID);
						if(varPicID.vt == VT_I4) {

							CEMRWordDocumentListDlg dlg(this);
							dlg.m_nPatientID = GetActivePatientID();
							dlg.m_nPICID = VarLong(varPicID);

							//if just one document, don't show the list, just auto-launch
							_RecordsetPtr rs = CreateRecordset("SELECT PathName FROM MailSent "
								"WHERE Selection = 'BITMAP:MSWORD' AND PicID = %li", VarLong(varPicID));
							if(!rs->eof) {
								if(rs->GetRecordCount() == 1) {
									dlg.OpenWordDocument(AdoFldString(rs, "PathName",""));
									return;
								}

								//load the Word document(s) for the PIC							
								dlg.DoModal();
							}
							rs->Close();
						}
					}
					else {
						//This is an EMN.
						long nEMNID = VarLong(pRow->GetValue(eecEMNID));

						CEMRWordDocumentListDlg dlg(this);
						dlg.m_nPatientID = GetActivePatientID();
						dlg.m_nEMNID = nEMNID;

						//if just one document, don't show the list, just auto-launch
						_RecordsetPtr rs = CreateRecordset("SELECT PathName FROM MailSent "
							"WHERE Selection = 'BITMAP:MSWORD' AND EMNID = %li", nEMNID);
						if(!rs->eof) {
							if(rs->GetRecordCount() == 1) {
								dlg.OpenWordDocument(AdoFldString(rs, "PathName",""));
								return;
							}

							//load the Word document(s) for the EMN						
							dlg.DoModal();
						}
						rs->Close();
					}
				} NxCatchAll("Error in OnLeftClickExistingEmrList:WordIcon");

			}
			break;
			//(a.wilson 2014-5-15) PLID 61808 - if the user clicks on the icon then it should open a clinical summary attached to the emn.
		case eecClinicalSummaryIcon:
			//CCDA icons are hyperlinks to open the Clinical Summary or Summary of Care document or list of documents
			try
			{
				long nEMNID = AsLong(pRow->GetValue(eecEMNID));
				long nPatientID = GetActivePatientID();

				if (nEMNID > 0)
				{
					//check for how many documents
					_RecordsetPtr prs = CreateParamRecordset(
						"SELECT PathName "
						"FROM	MailSent "
						"WHERE	MailSent.Selection = 'BITMAP:CCDA' "
						"AND	MailSent.CCDAtypeField = 2 "
						"AND	MailSent.EMNID = {INT} ", nEMNID);

					if (!prs->eof)
					{
						if (prs->GetRecordCount() == 1)
						{
							//open the single clinical summary.
							CString strFullPath = (GetPatientDocumentPath(nPatientID, 0) ^ AdoFldString(prs, "PathName", ""));
							ViewXMLDocument(strFullPath, this);
						}
						else
						{
							//open multi selection dialog.
							CEMRWordDocumentListDlg dlg(this, edtClinicalSummary);
							dlg.m_nPatientID = nPatientID;
							dlg.m_nEMNID = nEMNID;
							dlg.DoModal();
						}			
					}
					prs->Close();
				}
				else
				{
					//do nothing.
					return;
				}
			} NxCatchAll("Error in OnLeftClickExistingEmrList:ClinicalSummaryIcon");
			break;
		case eecEMRDescription: // (z.manning, 04/10/2007) - PLID 25518 - In tab view, EMR description column should be a link as well.
		case eecDescription:
			{
				//Descriptions are hyperlinks to open the EMN
				try {
					// (a.walling 2007-06-04 15:02) - PLID 25648 - Hide the preview if opening an EMN
					HidePreviewWindow();

					_variant_t var = pRow->GetValue(eecEMNID);
					long nMailID = VarLong(pRow->GetValue(eecMailID), -1);
					if(nMailID != -1) {
						// (z.manning 2008-07-03 12:14) - PLID 25574 - Open the history document if they
						// clicked on a history row link.
						OpenHistoryDocument(pRow);
					}
					else if(var.vt == VT_NULL) {
						//This should be the EMR record.
						_variant_t varPicID = pRow->GetValue(eecPicID);

						if(varPicID.vt == VT_I4) 
							LaunchPICEditor(VarLong(varPicID));
						else {
							// (c.haag 2010-08-04 11:49) - PLID 39980 - If we get here, we have no PIC.
							// Try to get an EMN for this EMR and auto-create one.
							long nEMRID = VarLong(pRow->GetValue(eecEMRID),-1);
							if (-1 == nEMRID) {
								// No PIC, no EMR, nothing we can do. 
								ThrowNxException("Attempted to open an EMR without a valid EMR ID or Pic ID!");
							}
							else {
								// We have an EMR. Open to any EMN; it doesn't matter which one, so that
								// we can auto-create a PIC for it.
								_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM EmrMasterT WHERE EmrGroupID = {INT} AND Deleted = 0", nEMRID);
								if (prs->eof) {
									// We have no PIC, and the EMR has no EMN's.
									prs->Close();
									AfxMessageBox("The EMR could not be opened because it is not assigned to a PIC, and does not contain any EMN's.", MB_OK | MB_ICONERROR);
								}
								else {
									// We found an EMN. Launch the PIC (it will auto-create itself in data if necessary).
									long nEMNID = AdoFldLong(prs, "ID");
									prs->Close();
									LaunchPICEditorWithEMN(-1, nEMNID);
								}
							}
						}
					}
					else {
						//This is an EMN.
						long nEMNID = VarLong(pRow->GetValue(eecEMNID));

						_variant_t varPicID = pRow->GetValue(eecPicID);
						// (c.haag 2010-08-04 11:49) - PLID 39980 - It's now OK to launch the PIC editor with a
						// -1 PicID. Somehow, it's possible to create an EMR without a PIC; this will silently work
						// around the issue.
						long nPicID = VarLong(varPicID, -1);

						LaunchPICEditorWithEMN(nPicID, nEMNID);
					}
				} NxCatchAll("Error in OnLeftClickExistingEmrList:Description");

			}
			break;

		case eecPreviewIcon:
			// (z.manning 2008-07-02 14:11) - PLID 30596 - Added a datalist column to enable
			// very easy access to the EMR preview.
			long nEmnID = VarLong(pRow->GetValue(eecEMNID), -1);
			long nMailID = VarLong(pRow->GetValue(eecMailID), -1);
			if(nEmnID != -1) {
				OnMenuShowPreview();
			}
			else if(nMailID != -1) {
				OpenHistoryDocument(pRow);
			}
			break;
		}

	} NxCatchAll("Error in OnLeftClickExistingList");
}

void CPatientNexEMRDlg::OnColumnClickingExistingList(short nCol, BOOL FAR* bAllowSort) 
{
	if (m_bEMNTabView) {
		// (a.wetta 2007-01-19 14:43) - PLID 24320 - If we're in the tab view let them sort
		*bAllowSort = TRUE;
	}
	else {
		*bAllowSort = FALSE;
	}
}

void CPatientNexEMRDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	long nChartCategoryAuditID = -1;
	try {
		//DRT - I had to move Tom's submenu already, and had to update a couple places.  Just keep it
		//	here in a const (since it has no ID of its own).
		// (z.manning 2009-07-23 08:05) - PLID 34049 - Changed these values after moving a few menu
		// options around.
		// (j.jones 2010-09-30 15:03) - PLID 40730 - values changed again as the ability to change
		// the EMN description was inserted before these options
		//(e.lally 2011-12-13) PLID 46968 - Changed these values after inserting a Publish To Portal menu option after Add New
		// (j.gruber 2013-11-11 16:43) - PLID 59415 - updated positions
		const int ADD_NEW_EMN_POSITION = 2;
		const int GENERATE_CLINICAL_SUMMARY = 6;
		const int CHANGE_EMN_CHARTS = 8;
		const int CHANGE_EMN_CATEGORY = 9;

		//Check to see if the focus is on our EMR list
		CWnd* pEMRWnd = GetDlgItem(IDC_EXISTING_LIST);
		if(pWnd->GetSafeHwnd() == pEMRWnd->GetSafeHwnd()) {
			//The EMR list has focus, so create our menu
			CMenu mnu;
			mnu.LoadMenu(IDR_NEXEMR_POPUP);
			CMenu* pSubMenu = mnu.GetSubMenu(0);
			if(pSubMenu) {
				//Check our selection to determine which popup options are available.
				//	No Selection:  No menu
				//	EMR Row:  Edit, Delete, View History*, Add New EMN
				//	EMN Row:  Edit, Delete, View History*, Add New EMN, Lock EMN, Move EMN
				//		*View History is modified to clearly state "View EMR History" or "View EMN History"
				NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();
				if(pCurSel == NULL) {
					//No current selection
					pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_EDITEMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					// (j.jones 2010-09-30 13:21) - PLID 40730 - added ability to rename an EMR/EMN, disabled here
					pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_RENAME_EMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_DELETEEMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_VIEWHISTORY, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					pSubMenu->EnableMenuItem(ADD_NEW_EMN_POSITION, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);	//This is the "Add New EMN -> " popup menu, which has no ID
					//(e.lally 2011-12-09) PLID 46968 - disabled ability to publish an EMN to the patient's NexWeb Portal when none are selected
					pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_PUBLISHEMNTONEXWEB, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_LOCKEMN, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_MOVEEMN_TOEMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					pSubMenu->EnableMenuItem(CHANGE_EMN_CHARTS, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
					pSubMenu->EnableMenuItem(CHANGE_EMN_CATEGORY, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
					pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_SHOWPREVIEW, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
					// (j.gruber 2013-11-11 15:39) - PLID 59415 - disable genrate clincial summary options
					pSubMenu->EnableMenuItem(GENERATE_CLINICAL_SUMMARY, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);					
				}
				else {

					// (z.manning 2008-07-03 09:51) - PLID 25574 - No menu for history rows
					long nMailID = VarLong(pCurSel->GetValue(eecMailID), -1);
					if(nMailID != -1) {
						return;
					}
					_variant_t var = pCurSel->GetValue(eecEMNID);

					CEMN::EPatientCreatedStatus ePatientCreatedStatus = (CEMN::EPatientCreatedStatus)VarByte(pCurSel->GetValue(eecPatientCreatedStatus), CEMN::pcsInvalid);

					// (z.manning, 03/30/2007) - PLID 25427 - If we have more than one selection, let's go ahead
					// and disable all menu options except for changing categories.
					if(GetSelectionCountExistingList() != 1)
					{
						pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_EDITEMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
						// (j.jones 2010-09-30 13:21) - PLID 40730 - added ability to rename an EMR/EMN, disabled here
						pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_RENAME_EMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
						pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_DELETEEMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
						pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_VIEWHISTORY, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
						pSubMenu->EnableMenuItem(ADD_NEW_EMN_POSITION, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);	//This is the "Add New EMN -> " popup menu, which has no ID
						//(e.lally 2011-12-09) PLID 46968 - For now we'll only allow them to publish one EMN at a time.
						pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_PUBLISHEMNTONEXWEB, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
						pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_LOCKEMN, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
						pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_MOVEEMN_TOEMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
						pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_SHOWPREVIEW, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

						// (j.gruber 2013-11-11 15:40) - PLID 59415 - can only generate a clinical summary for one emn at a time
						pSubMenu->EnableMenuItem(GENERATE_CLINICAL_SUMMARY, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);						

						// (z.manning, 05/22/2007) - PLID 25489 - Different charts can be associated with different
						// categories, so if we have multiple selections with different chart IDs, just disable
						// the change category option.
						// (a.walling 2007-11-28 10:39) - PLID 28044 - Disable if expired
						if(!DoAllSelectedRowsHaveSameChartID() || m_bExpired) {
							pSubMenu->EnableMenuItem(CHANGE_EMN_CATEGORY, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
						}
					}
					else
					{
						//All menu rows are enabled.  However, we need to rename them to be EMN or EMR, depending on the row
						//	we've selected.
						// (d.moore 2007-10-03) - PLID 27632 - 'Edit EMN' should always be referred to as 'Open EMN'.
						CString strEdit = "&Open EMR";
						CString strDelete = "&Delete EMR";
						CString strHistory = "&View EMR History";
						CString strRename = "&Change EMN Description";
						if(var.vt == VT_I4) {
							//We are on an EMN, so change our strings

							// (c.haag 2006-03-28 16:19) - PLID 19890 - Use slightly different options that do the same
							// thing based on user permissions
							// (d.moore 2007-10-03) - PLID 27632 - 'Edit EMN' should always be referred to as 'Open EMN'.
							strEdit = "&Open EMN";
							strRename = "&Change EMN Description";
							/*
							if (CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE)) {
								strEdit = "&Edit EMN";
							} else {
								strEdit = "&Open EMN";
							}
							*/
							strDelete = "&Delete EMN";
							strHistory = "&View EMN History";
						} else {
							// (c.haag 2006-03-28 16:33) - PLID 19890 - If we are on an EMR, change "edit" to "open"
							strEdit = "&Open EMR";
							strRename = "&Change EMR Description";

							// (z.manning, 06/26/2006) - PLID 20354 - We can't move EMRs.
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_MOVEEMN_TOEMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

							// (a.walling 2007-04-13 13:29) - PLID 25648 - We can't really preview an EMR
							// although all the menu item does is pop up the show preview dialog
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_SHOWPREVIEW, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

							//(e.lally 2011-12-13) PLID 46968 - For now, we'll only allow publishing one EMN at a time
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_PUBLISHEMNTONEXWEB, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

							// (j.gruber 2013-11-11 15:41) - PLID 59415 - generate a clinical summary for only one emn at a time
							pSubMenu->EnableMenuItem(GENERATE_CLINICAL_SUMMARY, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);							

						}

						//Do not allow the user to lock an EMN that is already locked, that's absurd!
						_variant_t varStatus = pCurSel->GetValue(eecStatus);
						long nDeleteFlags = 0;
						long nRenameFlags = 0;
						// (z.manning 2009-12-15 13:09) - PLID 35810 - No deleting patient-created unfinalized EMNs
						if(ePatientCreatedStatus == CEMN::pcsCreatedByPatientNotFinalized) {
							nDeleteFlags = MF_DISABLED|MF_GRAYED;
							nRenameFlags = MF_DISABLED|MF_GRAYED;
							// (z.manning 2009-12-15 13:15) - PLID 35810 - No moving or locking these EMNs either
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_MOVEEMN_TOEMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_LOCKEMN, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
							//(e.lally 2011-12-13) PLID 46968 - We can't publish something that's already in use by the patient, it's redundant
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_PUBLISHEMNTONEXWEB, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);							
						}
						// (a.walling 2007-11-28 10:40) - PLID 28044
						else if(var.vt == VT_I4 && (VarString(varStatus, "").CompareNoCase("Locked") != 0) && !m_bExpired) {
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_LOCKEMN, MF_BYCOMMAND|MF_ENABLED);

							if(ePatientCreatedStatus == CEMN::pcsCreatedByPatientFinalized){
								//(e.lally 2011-12-13) PLID 46968 - It's not locked, but the patient did finalize it once already.
								//	It can be made available to them again, but reword to indicate it was available once before.
								pSubMenu->ModifyMenu(ID_NEXEMRTABPOPUP_PUBLISHEMNTONEXWEB, MF_BYCOMMAND, ID_NEXEMRTABPOPUP_PUBLISHEMNTONEXWEB, "Re-&Publish To Portal");
							}

							//DRT 3/13/2006 - PLID 19687 - If the EMN is not locked, deletion is OK
							nDeleteFlags = MF_ENABLED;
							// (j.jones 2010-09-30 15:14) - PLID 40730 - so is renaming
							nRenameFlags = MF_ENABLED;
						}
						else {
							//DRT 3/13/2006 - PLID 19672 - "Lock EMN" should be disabled if we are on an EMR, or if the EMN is already locked.
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_LOCKEMN, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

							// (j.jones 2010-09-30 13:21) - PLID 40730 - added ability to rename an EMR/EMN, disabled here
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_RENAME_EMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

							// (z.manning, 06/28/2006) - They should not have the option of moving a locked EMN.
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_MOVEEMN_TOEMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

							//(e.lally 2011-12-13) PLID 46968 - Can't publish locked EMNs or EMRs
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_PUBLISHEMNTONEXWEB, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);

							if(var.vt == VT_NULL) {
								//This is an EMR, it's OK to delete
								nDeleteFlags = MF_ENABLED;
								// (j.jones 2010-09-30 15:14) - PLID 40730 - also OK to rename
								nRenameFlags = MF_ENABLED;
							}
							else {
								//Do not allow deleting of locked EMNs
								nDeleteFlags = MF_DISABLED|MF_GRAYED;
								// (j.jones 2010-09-30 15:14) - PLID 40730 - disallow renaming as well
								nRenameFlags = MF_DISABLED|MF_GRAYED;
							}
						}

						// (a.walling 2007-11-28 10:42) - PLID 28044 - added expiration checks to all these permission checks...

						// (c.haag 2006-03-28 16:22) - PLID 19890 - Alter menu toggles based on permissions
						if (!CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) || m_bExpired) {
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_LOCKEMN, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
						}
						// (j.jones 2007-05-15 17:00) - PLID 25431 - you can't create an EMR
						// without Create and Write permissions
						if (!CheckCurrentUserPermissions(bioPatientEMR, sptCreate, FALSE, 0, TRUE, TRUE)
							|| !CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE)
							 || m_bExpired) {
							pSubMenu->EnableMenuItem(ADD_NEW_EMN_POSITION, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
						}
						// (j.jones 2006-09-13 09:24) - PLID 22493 - removed bioPatientEMR delete permission,
						// it is now administrator-only
						// (a.walling 2007-11-28 10:41) - PLID 28044 - Also removed when expired
						if(!IsCurrentUserAdministrator() || m_bExpired) {
							nDeleteFlags = MF_DISABLED|MF_GRAYED;
						}
						// (z.manning, 06/28/2006) - PLID 21248 - We now have a permission specifically for moving EMNs.
						// (a.wetta 2007-01-12 13:12) - PLID 24224 - If they are in the tabbed view, they should not have the "Move EMN" option.
						// This doesn't make sense in this view mode and can be confusing
						if (!CheckCurrentUserPermissions(bioPatientEMR, sptDynamic0, FALSE, 0, TRUE, TRUE) || m_bEMNTabView || m_bExpired) {
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_MOVEEMN_TOEMR, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
						}

						// (a.wetta 2007-01-04 14:33) - PLID 14635 - If they don't have write permission they shouldn't be able
						// to change an EMN's category
						if (!CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE)
							 || m_bExpired) {
							// (a.wetta 2007-03-27 17:21) - PLID 25384 - This was previously not working because MF_BYCOMMAND was used instead of MF_BYPOSITION
							pSubMenu->EnableMenuItem(CHANGE_EMN_CHARTS, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
							pSubMenu->EnableMenuItem(CHANGE_EMN_CATEGORY, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
						}

						//(e.lally 2011-12-13) PLID 46968 - Silently check NexWeb Patient Portal license and silently get permissions
						if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrSilent) 
						  || !(GetCurrentUserPermissions(bioPublishEMNToNexWeb) & (sptDynamic0 | sptDynamic0WithPass)) ){
							pSubMenu->EnableMenuItem(ID_NEXEMRTABPOPUP_PUBLISHEMNTONEXWEB, MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
						}

						//change the text of the various menu items
						pSubMenu->ModifyMenu(ID_NEXEMRTABPOPUP_EDITEMR, MF_BYCOMMAND, ID_NEXEMRTABPOPUP_EDITEMR, strEdit);						
						pSubMenu->ModifyMenu(ID_NEXEMRTABPOPUP_DELETEEMR, MF_BYCOMMAND|nDeleteFlags, ID_NEXEMRTABPOPUP_DELETEEMR, strDelete);
						pSubMenu->ModifyMenu(ID_NEXEMRTABPOPUP_VIEWHISTORY, MF_BYCOMMAND, ID_NEXEMRTABPOPUP_VIEWHISTORY, strHistory);

						// (j.jones 2010-09-30 14:11) - PLID 40730 - added ability to rename an EMR/EMN
						pSubMenu->ModifyMenu(ID_NEXEMRTABPOPUP_RENAME_EMR, MF_BYCOMMAND|nRenameFlags, ID_NEXEMRTABPOPUP_RENAME_EMR, strRename);
					}

					// (a.wetta 2007-01-04 14:39) - PLID 14635 - Add a submenu for the EMN tab categories

					// If they are not in the "tab view" mode or are on an EMR, disable this menu item
					if (!m_bEMNTabView || var.vt != VT_I4 || m_bExpired) {
						pSubMenu->EnableMenuItem(CHANGE_EMN_CHARTS, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
						pSubMenu->EnableMenuItem(CHANGE_EMN_CATEGORY, MF_BYPOSITION|MF_DISABLED|MF_GRAYED);
					}
					else {
						CMenu *pChangeEMNCat = pSubMenu->GetSubMenu(CHANGE_EMN_CATEGORY);
						if(pChangeEMNCat == NULL){
							//Someone forgot to update the defines above
							AfxThrowNxException("Could not find the 'Change Category' submenu!");
						}
						// Insert the "No Category" option
						pChangeEMNCat->InsertMenu(0, MF_BYPOSITION|MF_ENABLED, TAB_CAT_ID_OFFSET, "{No Category}");

						// (z.manning, 05/23/2007) - PLID 25489 - Only load categories that are associated with the current
						// selection's chart type if it has one. Note: yes, it's possible to have multiple selections,
						// however, this menu option should not be enabled if we have multiple selections with
						// differing chart types.
						CString strWhere = "";
						if(pCurSel) {
							long nChartID = VarLong(pCurSel->GetValue(eecTabChartID), -1);
							if(nChartID != -1) {
								strWhere = FormatString(" WHERE EmnTabCategoriesT.ID IN (SELECT EmnTabCategoryID FROM EmnTabChartCategoryLinkT WHERE EmnTabChartID = %li) ", nChartID);
							}
						}
						
						_RecordsetPtr prsEMNTabCat = CreateRecordset("SELECT * FROM EMNTabCategoriesT %s ORDER BY Priority DESC ", strWhere);
						while(!prsEMNTabCat->eof) {
							long nCatID = AdoFldLong(prsEMNTabCat, "ID");
							CString strDescription = AdoFldString(prsEMNTabCat, "Description");

							//add as a negative number, so we don't overlap any other in-use IDs
							// (j.jones 2007-02-26 08:44) - PLID 24926 - utilized ConvertToControlText to fix potential issues such as ampersands not displaying
							pChangeEMNCat->InsertMenu(0, MF_BYPOSITION|MF_ENABLED, (-nCatID+(TAB_CAT_ID_OFFSET-1)), ConvertToControlText(strDescription));

							prsEMNTabCat->MoveNext();
						}

						// (z.manning, 04/04/2007) - PLID 25489 - Load the charts into the chart popup menu,
						// including an option for "no chart."
						CMenu *pChangeEmnChart = pSubMenu->GetSubMenu(CHANGE_EMN_CHARTS);
						if(pChangeEmnChart == NULL){
							//Someone forgot to update the defines above
							AfxThrowNxException("Could not find the 'Change Chart' submenu!");
						}
						pChangeEmnChart->InsertMenu(0, MF_BYPOSITION|MF_ENABLED, TAB_CHART_ID_OFFSET, "{No Chart}");
						// (z.manning 2011-05-19 15:38) - PLID 33114 - Moved this logic to a utility function that also handles
						// the current user's EMR chart permissions.
						_RecordsetPtr prsEmnCharts = GetEmrChartRecordset();
						while(!prsEmnCharts->eof) {
							long nChartID = AdoFldLong(prsEmnCharts, "ID");
							CString strDescription = AdoFldString(prsEmnCharts, "Description");

							//add as a negative number, so we don't overlap any other in-use IDs
							pChangeEmnChart->InsertMenu(0, MF_BYPOSITION|MF_ENABLED, (-nChartID+(TAB_CHART_ID_OFFSET-1)), ConvertToControlText(strDescription));

							prsEmnCharts->MoveNext();
						}
					}
				}

				//TES 3/13/2006 - Add a submenu for the templates that the Dynamic Button manager has already calculated.
				
				// (j.jones 2006-10-31 11:59) - PLID 23288 - we can't use the button manager because there may be more
				// templates than the button manager is able to display

				// (j.jones 2007-06-11 10:53) - PLID 26271 - the template loading has been moved to BuildCurEMRTemplateInfo,
				// which should have been called prior to the context menu if this EMR is already selected, otherwise
				// we may need to call it ourselves

				CMenu *pAddEmn = pSubMenu->GetSubMenu(ADD_NEW_EMN_POSITION);

				if(pCurSel != NULL) {

					long nEMRID = VarLong(pCurSel->GetValue(eecEMRID),-1);

					if(nEMRID == -1) {
						//should be impossible, but handle it anyways
						ClearCurEMRTemplateInfo();
					}
					else if(nEMRID != m_nCurEMRSelected) {
						// (j.jones 2007-06-11 10:50) - PLID 26271 - build the template info anew,
						// if the m_nCurEMRSelected is out of date
						BuildCurEMRTemplateInfo(nEMRID, FALSE);
					}

					//now that we have the information in the array, we do not need to run recordsets here
					for(int i=0;i<m_arCurEMRTemplateInfo.GetSize();i++) {

						EMRTemplateInfo etInfo = (EMRTemplateInfo)(m_arCurEMRTemplateInfo.GetAt(i));

						//add as a negative number, so we don't overlap any other in-use IDs
						// (j.jones 2007-02-26 08:44) - PLID 24926 - utilized ConvertToControlText to fix potential issues such as ampersands not displaying
						pAddEmn->InsertMenu(0, MF_BYPOSITION|MF_ENABLED,-(etInfo.nID), ConvertToControlText(etInfo.strName));
					}
				}

				//Ensure the position given is a valid one (if they use the keyboard, it won't be)
				if (point.x == -1) {
					CRect rc;
					pWnd->GetWindowRect(&rc);
					GetCursorPos(&point);
					if (!rc.PtInRect(point)) {
						point.x = rc.left+5;
						point.y = rc.top+5;
					}
				}

				//pop up the menu at the given position 
				long nResult = pSubMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, point.x, point.y, this, NULL);
				if(nResult < 0) {
					//Get the currently selected EMR
					NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();
					if(pCurSel == NULL) {
						//this shouldn't happen
						ASSERT(FALSE);
						return;
					}

					if (nResult > TAB_CAT_ID_OFFSET) {
						//they chose a template

						//pull the EMR ID out of the list
						_variant_t varEMR = pCurSel->GetValue(eecPicID);

						//Now open the editor with this EMR
						LaunchPICEditorWithNewEMN(VarLong(varEMR), -nResult);
						return;
					}
					else if(nResult > TAB_CHART_ID_OFFSET){
						// (a.wetta 2007-01-04 15:41) - PLID 14635 - The want to change the EMN tab category of this EMN
						
						// (z.manning, 03/30/2007) - PLID 25427 - We now support selecting, and thus changing multiple
						// EMNs' tab categories at once.
						CArray<long> arySelectedEmnIDs;
						GetAllSelectedEmnIDs(arySelectedEmnIDs);
						
						if(arySelectedEmnIDs.GetSize() > 0)
						{
							long nTabCatID = -nResult + (TAB_CAT_ID_OFFSET-1);
							
							// (a.walling 2008-08-27 14:13) - PLID 30515 - Warn beforehand if possible of any concurrency issues
							if (WarnIfEMNConcurrencyIssuesExist(this, CSqlFragment("{INTARRAY}", arySelectedEmnIDs), "The category cannot be changed.")) {
								return;
							}

							nChartCategoryAuditID = BeginAuditTransaction();
							CString strNewValue;
							pSubMenu->GetMenuString(nResult, strNewValue, MF_BYCOMMAND);

							// (z.manning, 03/30/2007) - PLID 25427 - Put these sql statement in a batch.
							CString strSql = BeginSqlBatch();

							// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
							// (a.walling 2008-08-19 10:07) - PLID 30515 - Ensure we don't modify an EMN that is currently being edited
							// (j.armen 2013-05-14 12:31) - PLID 56680 - EMN Access restructuring
							AddStatementToSqlBatch(strSql, 
								"IF EXISTS(\r\n"
								"	SELECT 1\r\n"
								"	FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK)\r\n"
								"	WHERE EmnID IN (%s))\r\n"
								"BEGIN\r\n"
								"	RAISERROR('Category cannot be changed; one or more EMNs are currently protected for editing.', 16, 43)\r\n"
								"	ROLLBACK TRAN\r\n"
								"	RETURN\r\n"
								"END\r\n",
								ArrayAsString(arySelectedEmnIDs,false));

							// First delete the old record in EMNTabCategoriesLinkT
							AddStatementToSqlBatch(strSql, "DELETE FROM EMNTabCategoriesLinkT WHERE EMNID IN (%s)", ArrayAsString(arySelectedEmnIDs,false));

							// (z.manning, 03/30/2007) - PLID 25427 - We may have more than one.
							for(int i = 0; i < arySelectedEmnIDs.GetSize(); i++) {
								// Make sure that they aren't changing the category to "No Category."  In that case we don't have to insert anything.
								if (nTabCatID != -1) {
									// Now insert the new record to link the EMN and the tab category
									AddStatementToSqlBatch(strSql, "INSERT INTO EMNTabCategoriesLinkT (EMNID, EMNTabCategoryID) VALUES (%li, %li)",
											arySelectedEmnIDs.GetAt(i), nTabCatID);
								}

								// (z.manning, 04/19/2007) - PLID 25714 - Audit the Category change.
								NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->FindByColumn(eecEMNID, arySelectedEmnIDs.GetAt(i), NULL, VARIANT_FALSE);
								CString strOldValue;
								if(pRow) {
									_variant_t vtOldValue = pRow->GetOutputValue(eecTabCategoryID);
									if(vtOldValue.vt == VT_BSTR) {
										strOldValue = VarString(vtOldValue);
									}
									if(strOldValue.IsEmpty()) {
										strOldValue = "{No Category}";
									}
								}
								else {
									ASSERT(FALSE);
								}
								if(strOldValue != strNewValue) {
									AuditEvent(GetActivePatientID(), GetActivePatientName(), nChartCategoryAuditID, aeiEMNCategory, arySelectedEmnIDs.GetAt(i), strOldValue, strNewValue, aepMedium, aetChanged);
								}
							}

							ExecuteSqlBatch(strSql);
							CommitAuditTransaction(nChartCategoryAuditID);
							nChartCategoryAuditID = -1;

							// (z.manning, 03/30/2007) - PLID 25427 - Update all selected rows with the new category.
							// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
							NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_pExistingList->GetFirstSelRow();
							while (pCurSelRow != NULL) {
								NXDATALIST2Lib::IRowSettingsPtr pRow = pCurSelRow;
								pCurSelRow = pCurSelRow->GetNextSelRow();

								// (z.manning 2008-07-03 12:25) - PLID 25574 - Don't update history rows
								if(VarLong(pRow->GetValue(eecMailID), -1) == -1) {
									pRow->PutValue(eecTabCategoryID, nTabCatID);
								}

								// (z.manning, 04/05/2007) - PLID 25508 - Make sure we don't display any EMNs
								// that are no longer a member of the currectly selected category.
								if(nTabCatID != GetCurSelCategoryID() && GetCurSelCategoryID() != -1) {
									m_pExistingList->RemoveRow(pRow);
								}
							}
						}

						return;
					}
					else {
						// (z.manning, 04/04/2007) - PLID 25489 - They must be changing the chart.
						CArray<long> arySelectedEmnIDs;
						GetAllSelectedEmnIDs(arySelectedEmnIDs);
						BOOL bShowedIncompatibleCategoryWarning = FALSE;

						if(arySelectedEmnIDs.GetSize() > 0) {
							CString strSql = BeginSqlBatch();

							// (a.walling 2008-08-27 14:13) - PLID 30515 - Warn beforehand if possible of any concurrency issues
							if (WarnIfEMNConcurrencyIssuesExist(this, CSqlFragment("{INTARRAY}", arySelectedEmnIDs), "The chart cannot be changed.")) {
								return;
							}

							// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
							// (a.walling 2008-08-19 10:07) - PLID 30515 - Ensure we don't modify an EMN that is currently being edited
							// (j.armen 2013-05-14 12:32) - PLID 56680 - EMN Access restructuring
							AddStatementToSqlBatch(strSql, 
								"IF EXISTS(\r\n"
								"	SELECT 1\r\n"
								"	FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK)\r\n"
								"	WHERE EmnID IN (%s))\r\n"
								"BEGIN\r\n"
								"	RAISERROR('Chart cannot be changed; one or more EMNs are currently protected for editing.', 16, 43)\r\n"
								"	ROLLBACK TRAN\r\n"
								"	RETURN\r\n"
								"END\r\n",
								ArrayAsString(arySelectedEmnIDs,false));

							AddStatementToSqlBatch(strSql, "DELETE FROM EmnTabChartsLinkT WHERE EmnID IN (%s)", ArrayAsString(arySelectedEmnIDs,false));

							long nChartID = -nResult + (TAB_CHART_ID_OFFSET - 1);
							nChartCategoryAuditID = BeginAuditTransaction();
							CString strNewValue;
							pSubMenu->GetMenuString(nResult, strNewValue, MF_BYCOMMAND);

							for(int i = 0; i < arySelectedEmnIDs.GetSize(); i++) {
								if(nChartID > 0) {
									AddStatementToSqlBatch(strSql, "INSERT INTO EmnTabChartsLinkT (EmnID, EmnTabChartID) VALUES (%li, %li)"
										, arySelectedEmnIDs.GetAt(i), nChartID);
								}

								// (z.manning, 04/19/2007) - PLID 25714 - Audit the chart change.
								NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->FindByColumn(eecEMNID, arySelectedEmnIDs.GetAt(i), NULL, VARIANT_FALSE);
								CString strOldValue;
								if(pRow) {
									_variant_t vtOldValue = pRow->GetOutputValue(eecTabChartID);
									if(vtOldValue.vt == VT_BSTR) {
										strOldValue = VarString(vtOldValue);
									}
									if(strOldValue.IsEmpty()) {
										strOldValue = "{No Chart}";
									}
								}
								else {
									ASSERT(FALSE);
								}
								if(strOldValue != strNewValue) {
									AuditEvent(GetActivePatientID(), GetActivePatientName(), nChartCategoryAuditID, aeiEMNChart, arySelectedEmnIDs.GetAt(i), strOldValue, strNewValue, aepMedium, aetChanged);

									// (z.manning, 05/23/2007) - PLID 25489 - If they changed this row, check and see
									// if we have a category ID as well.  If we do
									if(pRow) {
										long nCategoryID = VarLong(pRow->GetValue(eecTabCategoryID), -1);
										if(nCategoryID != -1) {
											if(!ReturnsRecords("SELECT EmnTabCategoryID FROM EmnTabChartCategoryLinkT WHERE EmnTabCategoryID = %li AND EmnTabChartID = %li", nCategoryID, nChartID)) {
												if(!bShowedIncompatibleCategoryWarning) {
										// (j.luckoski 2012-03-14 14:10) - PLID 48887 - Fixed the grammar on the following warning statement so that it is clear.
													MessageBox("Some EMN(s) have a category that is not associated with the newly selected chart. The category has been reset to nothing.");
													bShowedIncompatibleCategoryWarning = TRUE;
												}
												_variant_t vtNull;
												vtNull.vt = VT_NULL;
												// (z.manning 2008-07-03 12:25) - PLID 25574 - Don't update history rows
												if(VarLong(pRow->GetValue(eecMailID), -1) == -1) {
													pRow->PutValue(eecTabCategoryID, vtNull);
													OnEditingFinishedExistingList(pRow, eecTabCategoryID, nCategoryID, vtNull, VARIANT_TRUE);
												}
											}
										}
									}
								}
							}

							ExecuteSqlBatch(strSql);
							CommitAuditTransaction(nChartCategoryAuditID);
							nChartCategoryAuditID = -1;

							// (z.manning, 03/30/2007) - PLID 25489 - Update all selected rows with the new category.
							// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
							NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_pExistingList->GetFirstSelRow();
							while (pCurSelRow != NULL) {
								NXDATALIST2Lib::IRowSettingsPtr pRow = pCurSelRow;
								pCurSelRow = pCurSelRow->GetNextSelRow();

								// (z.manning 2008-07-03 12:25) - PLID 25574 - Don't update history rows
								if(VarLong(pRow->GetValue(eecMailID), -1) == -1) {
									pRow->PutValue(eecTabChartID, nChartID);
								}

								// (z.manning, 04/05/2007) - PLID 25508 - Make sure we don't display any EMNs
								// that are no longer a member of the currectly selected chart.
								if(nChartID != GetCurSelChartID() && GetCurSelChartID() != -1) {
									m_pExistingList->RemoveRow(pRow);
								}
							}
						}
					}
				}
				else if(nResult != 0) {
					//fire the normal response
					PostMessage(WM_COMMAND, MAKEWPARAM(nResult, BN_CLICKED));
				}
			}
		}
		
		/*	If you want to check focus for another window, do it here in an else if(... )	*/
	} NxCatchAllCall("Error in OnContextMenu", if(nChartCategoryAuditID == -1) { RollbackAuditTransaction(nChartCategoryAuditID); });
}

void CPatientNexEMRDlg::OnRButtonDownExistingList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		//set the row to whatever they right clicked on
		//	Actual work is done in the ContextMenu function
		GetDlgItem(IDC_EXISTING_LIST)->SetFocus();
		// (z.manning, 03/30/2007) - PLID 25427 - Don't do this if we have multiple selections because we
		// want to let users have the option to right click on multiple selected EMNs.
		if(GetSelectionCountExistingList() <= 1 || !IsRowSelected(pRow)) {
			m_pExistingList->PutCurSel(pRow);
		}

		//fire the OnSelChanged like we left clicked the list, so that the buttons
		//	are filtered.
		HandleSelectionChange(lpRow);

	} NxCatchAll("Error in OnRButtonDownExistingList");
}

void CPatientNexEMRDlg::OnEditEMR()
{
	try {
		//Get the currently selected row
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();

		//"fake" a click event of the description column, which will handle the code to pop up an EMR
		OnLeftClickExistingList(pCurSel, eecDescription, 0, 0, 0);

	} NxCatchAll("Error in OnEditEMR");
}

void CPatientNexEMRDlg::OnDeleteEMR()
{
	try {
		// (a.walling 2007-11-28 10:19) - PLID 28044
		if (m_bExpired) return;

		//Get the current selection
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();

		// (j.jones 2006-09-13 09:24) - PLID 22493 - removed bioPatientEMR delete permission,
		// it is now administrator-only
		// (a.walling 2007-11-28 13:11) - PLID 28044 - Also exit if expired EMR license
		if(!IsCurrentUserAdministrator() || pCurSel == NULL || m_bExpired) {
			return;
		}

		//Is this an EMR or EMN?
		_variant_t var = pCurSel->GetValue(eecEMNID);
		if(var.vt == VT_I4) {
			//If there is an EMN ID, this record must be an EMN we are deleting

			// (j.jones 2009-10-01 11:36) - PLID 30479 - added a specific dialog
			// to prompt for deletion confirmation
			CDeleteEMNConfirmDlg dlg(ecdtEMN, VarString(pCurSel->GetValue(eecDescription), ""), this);
			if(dlg.DoModal() != DELETE_EMN_RETURN_DELETE) {
				//unless the return value specifically says to delete, leave now
				return;
			}

			//The safest way of deleting is just to load the EMR, tell it to remove the 
			//	selected EMN, then tell it to re-save itself.  This isn't terribly fast, 
			//	but it's a good tradeoff for safety in deleting.
			BeginWaitCursor();

			CEMR* pEMR = new CEMR;
			//DRT 7/27/2007 - PLID 26836 - Since we are just deleting the EMR, set the "emn to be displayed" to -1, so
			//	that all EMNs are loaded as fast as possible.
			long nEMNIDToDisplay = -1;
			pEMR->LoadFromID(VarLong(pCurSel->GetValue(eecEMRID)), FALSE, nEMNIDToDisplay);
			CEMN *pEMN = pEMR->GetEMNByID(VarLong(pCurSel->GetValue(eecEMNID)));
			if(!pEMR->RemoveEMN(pEMN)) {
				//This can fail if the EMN is locked, for instance.
				// (c.haag 2006-07-10 16:43) - PLID 19977 - This can fail if the EMR/EMN is associated with
				// problem details
				// (a.walling 2008-06-25 16:05) - PLID 30515 - Also will fail if the EMN is in use
				AfxMessageBox("Failed to delete the given EMN.  It may be locked, in use, or associated with one or more problem details.");
				delete pEMR;
				return;
			}

			//TES 7/14/2008 - PLID 30196 - Moved this block of code to before the SaveEMRObject call.
			CString strWarningMessage;
			ASSERT(pEMN);
			if(pEMN) {
				// (c.haag 2008-07-23 17:22) - PLID 30820 - Check for any problems that were not explicitly deleted but
				// are still going to be flagged as deleted because their owner is as such, and warn the user. Return
				// if the user cancelled the dialog (meaning they did not want to delete problems, which means they cannot
				// delete anything else).
				if (!CheckForDeletedEmrProblems(esotEMN, (long)pEMN, TRUE)) {
					// (j.jones 2009-05-30 18:55) - PLID 34408 - cleanup our memory
					delete pEMR;
					return;
				}

				//TES 6/5/2008 - PLID 30196 - If the deleted EMN had a system table on it, we need to warn them to check 
				// the Medications tab.		
				BOOL bHasCurrentMeds = pEMN->HasSystemTable(eistCurrentMedicationsTable);
				BOOL bHasAllergies = pEMN->HasSystemTable(eistAllergiesTable);
				if(bHasCurrentMeds || bHasAllergies) {
					//TES 6/5/2008 - PLID 30196 - Warn them to check the medications tab.
					CString strTables;
					if(bHasCurrentMeds && bHasAllergies) {
						strTables = "'Current Medications' and 'Allergies' tables";
					}
					else if(bHasCurrentMeds) {
						strTables = "'Current Medications' table";
					}
					else if(bHasAllergies) {
						strTables = "'Allergies' table";
					}

					strWarningMessage.Format("The deleted EMN contained the %s.  Please review this patient's Medications tab to make sure that "
						"its corresponding information is up to date.", strTables);
				}
			}

			// (j.jones 2012-09-28 09:00) - PLID 52820 - SaveEMRObject requires this parameter, but currently
			// we do not check drug interactions after deleting EMRs
			BOOL bDrugInteractionsChanged = FALSE;

			// (j.jones 2009-06-17 12:56) - PLID 34652 - make sure we check to see if the save failed
			CDWordArray arNewCDSInterventions;
			//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
			if(FAILED(SaveEMRObject(esotEMR, (long)pEMR, FALSE, bDrugInteractionsChanged, arNewCDSInterventions))) {
				//we assume that whatever caused a failure explained why, so we don't have to
				//Cleanup
				delete pEMR;
				EndWaitCursor();
				ReloadEMRList();
				return;
			}

			//TES 7/14/2008 - PLID 30196 - Now tell the user about any deleted systems tables.
			if(!strWarningMessage.IsEmpty()) {
				MsgBox("%s", strWarningMessage);
			}

			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);

			//Clear memory
			delete pEMR;

			EndWaitCursor();

			AfxMessageBox("Your EMN has been successfully deleted.");
		}
		else {
			//We are deleting an entire EMR
			CDWordArray arNewCDSInterventions;
			//TES 10/31/2013 - PLID 59251 - If this triggers any interventions, notify the user
			if (TRUE == AttemptRemovePICRecord(VarLong(pCurSel->GetValue(eecPicID)), arNewCDSInterventions)) {
				// (c.haag 2006-03-16 12:24) - PLID 19673 - As we do with EMN's, tell the
				// user that the EMR deleted successfully.
				AfxMessageBox("Your EMR has been successfully deleted.");

				//TES 3/24/2006 - The datalist selection has probably changed.
				HandleSelectionChange(GetCurSelExistingList());
			}
			GetMainFrame()->DisplayNewInterventions(arNewCDSInterventions);
		}

		ReloadEMRList();
	} NxCatchAll("Error in OnDeleteEMR");
}

void CPatientNexEMRDlg::OnViewHistory()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();
		if(pCurSel == NULL)
			return;
		if (!CheckCurrentUserPermissions(bioPatientEMR, sptRead))
			return;

		CEMRAuditHistoryDlg dlg(this);

		if(pCurSel->GetValue(eecEMNID).vt == VT_I4) {
			//we are on an EMN
			dlg.m_nEMNID = VarLong(pCurSel->GetValue(eecEMNID));
			dlg.m_nEMRID = VarLong(pCurSel->GetValue(eecEMRID));
		}
		else {
			//both EMNs and EMRs need the EMR ID filled
			dlg.m_nEMRID = VarLong(pCurSel->GetValue(eecEMRID));
		}
		dlg.DoModal();

	} NxCatchAll("Error in OnViewHistory");
}

void CPatientNexEMRDlg::OnCreateEMR()
{
	try {

	} NxCatchAll("Error in OnCreateEMR");
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
//		END section of code is related to redrawing and interacting with the list of EMRs			//
//////////////////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////
//  Methods for launching the EMR/EMN editor			//
//////////////////////////////////////////////////////////

void CPatientNexEMRDlg::LaunchPICEditorWithEMN(long nPicID, long nEMNID)
{
	//Open the PIC to the clinical tab and load to the given EmnID
	GetMainFrame()->EditEmrRecord(nPicID, nEMNID);
}

void CPatientNexEMRDlg::LaunchPICEditor(long nPicID)
{
	//Edit the EMR by opening the PIC to the clinical tab
	GetMainFrame()->EditEmrRecord(nPicID);
}

//Edit an existing EMR record, but add a new EMN onto it.  nPicID must be valid.
void CPatientNexEMRDlg::LaunchPICEditorWithNewEMN(long nPicID, long nTemplateID)
{
	// (a.walling 2007-11-28 10:52) - PLID 28044
	if (m_bExpired) {
		ASSERT(FALSE); // should have caught this before now
		return;
	}

	// (j.gruber 2012-05-31 10:52) - PLID 49046 - moved to global function
	GlobalLaunchPICEditorWithNewEMN(nPicID, nTemplateID);
}

//Start a brand new EMR with a new EMN... give an nPicID if you wish it to be tied to an existing
//	PIC (tracking), otherwise -1 if you want it to be new.
void CPatientNexEMRDlg::LaunchPICEditorWithNewEMR(long nPicID, long nTemplateID)
{
	// (a.walling 2007-11-28 10:52) - PLID 28044
	if (m_bExpired) {
		ASSERT(FALSE); // should have caught this before now
		return;
	}

	// (j.gruber 2012-05-31 10:52) - PLID 49046 - moved to global function
	GlobalLaunchPICEditorWithNewEMR(nPicID, nTemplateID);
	
}

//////////////////////////////////////////////////////////
//		END Methods for launching the EMR/EMN editor	//
//////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////
//		Methods for working with the New EMR List		//
//////////////////////////////////////////////////////////

//Color the headers slightly gray so they look disabled.  User clicking
//	on these rows has no effect.
// (j.armen 2012-04-23 14:05) - PLID 49614 - Updating the color
#define HEADER_FORECOLOR		RGB(0, 160, 40)

//Level 1 Header - Expandable
#define	SYMPTOM_BASED_ID		-1
#define SYMPTOM_BASED_NAME		"Symptom/Subjective Based EMR"

//Level 1 Header - Expandable
#define	PROCEDURE_BASED_ID		-2
#define PROCEDURE_BASED_NAME	"Procedure Based EMR"

//Level 2 Header - Expandable
#define PROCEDURE_ALL_ID		-3
#define PROCEDURE_ALL_NAME		"All Procedures"

//Level 2 Header - Expandable
#define PROCEDURE_TRACKED_ID	-4
#define	PROCEDURE_TRACKED_NAME	"Active NexTrack Procedures"

//Level 2 Header - Expandable
#define PROCEDURE_PREFERRED_ID	-5
#define PROCEDURE_PREFERRED_NAME	"Most Common Procedures"

/*
UINT RefreshNewListThread(LPVOID p)
{
	CPatientNexEMRDlg* pDlg = (CPatientNexEMRDlg*)p;

	try {

		pDlg->m_pNewTree->Clear();

		// Open an new connection based on the existing one
		{
			_ConnectionPtr pExistingConn = GetRemoteData();
			_ConnectionPtr pNewConn(__uuidof(Connection));
			pDlg->m_pRefreshNewListConn = pNewConn;
			// Use the same connection timeout
			pDlg->m_pRefreshNewListConn->PutConnectionTimeout(pExistingConn->GetConnectionTimeout());
			// Open the connection
			pDlg->m_pRefreshNewListConn->Open(pExistingConn->ConnectionString, "", "", 0);
			// Use the same command timeout
			pDlg->m_pRefreshNewListConn->PutCommandTimeout(pExistingConn->GetCommandTimeout());
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow;

		_variant_t varNull;
		varNull.vt = VT_NULL;

		//Root rows - These are the base rows of the tree.  All other rows added will be children below these.
		pRow = pDlg->m_pNewTree->GetNewRow();
		pRow->PutValue(necID, (long)PROCEDURE_BASED_ID);
		pRow->PutValue(necParent, varNull);
		pRow->PutValue(necType, (short)neltHeader);
		pRow->PutValue(necDescription, _bstr_t(PROCEDURE_BASED_NAME));
		pRow->PutForeColor(HEADER_FORECOLOR);
		NXDATALIST2Lib::IRowSettingsPtr pProcedureRow = pDlg->m_pNewTree->AddRowAtEnd(pRow, NULL);

		pRow = pDlg->m_pNewTree->GetNewRow();
		pRow->PutValue(necID, (long)SYMPTOM_BASED_ID);
		pRow->PutValue(necParent, varNull);
		pRow->PutValue(necType, (short)neltHeader);
		pRow->PutValue(necDescription, _bstr_t(SYMPTOM_BASED_NAME));
		pRow->PutForeColor(HEADER_FORECOLOR);
		NXDATALIST2Lib::IRowSettingsPtr pSymptomRow = pDlg->m_pNewTree->AddRowAtEnd(pRow, NULL);

		//We then need to load each of the children rows

		//First the Procedure Children

		//We have 3 hard-coded headers under the "By Procedure" option, we want to add all of them as children
		//of the given parent.

		//1)  Preferred Procedures - These are procedures that this office performs regularly.  This 
		//	will not include procedures which are in the "tracked" section.
		pRow = pDlg->m_pNewTree->GetNewRow();
		pRow->PutValue(necID, (long)PROCEDURE_PREFERRED_ID);
		pRow->PutValue(necParent, (short)PROCEDURE_BASED_ID);
		pRow->PutValue(necType, (short)neltHeader);
		pRow->PutValue(necDescription, _bstr_t(PROCEDURE_PREFERRED_NAME));
		pRow->PutForeColor(HEADER_FORECOLOR);
		NXDATALIST2Lib::IRowSettingsPtr pPreferredRow = pDlg->m_pNewTree->AddRowAtEnd(pRow, pProcedureRow);

		//2)  Tracked Procedures - These are procedures currently tracked by this patient
		pRow = pDlg->m_pNewTree->GetNewRow();
		pRow->PutValue(necID, (long)PROCEDURE_TRACKED_ID);
		pRow->PutValue(necParent, (long)PROCEDURE_BASED_ID);
		pRow->PutValue(necType, (short)neltHeader);
		pRow->PutValue(necDescription, _bstr_t(PROCEDURE_TRACKED_NAME));
		pRow->PutForeColor(HEADER_FORECOLOR);
		NXDATALIST2Lib::IRowSettingsPtr pTrackedRow = pDlg->m_pNewTree->AddRowAtEnd(pRow, pProcedureRow);

		//3)  All Other Procedures - This is collapsed by default, but will show all other procedures
		//	if expanded.
		pRow = pDlg->m_pNewTree->GetNewRow();
		pRow->PutValue(necID, (long)PROCEDURE_ALL_ID);
		pRow->PutValue(necParent, (long)PROCEDURE_BASED_ID);
		pRow->PutValue(necType, (short)neltHeader);
		pRow->PutValue(necDescription, _bstr_t(PROCEDURE_ALL_NAME));
		pRow->PutForeColor(HEADER_FORECOLOR);
		NXDATALIST2Lib::IRowSettingsPtr pOtherRow = pDlg->m_pNewTree->AddRowAtEnd(pRow, pProcedureRow);

		//Now that we have added all the hard-coded rows, we need to populate the details under each

		// (j.jones 2006-06-01 10:08) - since this is now asynchronous, we should add rows in order
		// of usage, which would be symptom, tracked, preferred, other

		//load the symptom children
		{
			CString strSql;
			strSql.Format("SELECT ID, Name "
					"FROM EMRTemplateT "
					"WHERE EMRTemplateT.EMRSymptomGroup = 1 AND EMRTemplateT.Deleted = 0 "
					"ORDER BY Name");
			_RecordsetPtr prsProc = pDlg->m_pRefreshNewListConn->Execute((LPCTSTR)strSql, NULL, 0);
			while(!prsProc->eof) {
				long nID = AdoFldLong(prsProc, "ID");
				CString strName = AdoFldString(prsProc, "Name", "");

				//Setup a new row with the information we got from the database
				NXDATALIST2Lib::IRowSettingsPtr pRow = pDlg->m_pNewTree->GetNewRow();
				pRow->PutValue(necID, (long)nID);
				pRow->PutValue(necParent, (long)SYMPTOM_BASED_ID);
				pRow->PutValue(necDescription, _bstr_t(strName));
				pRow->PutValue(necType, (short)neltTemplate);

				//Make the description cell a hyperlink, for ease of use
				pRow->PutCellLinkStyle(necDescription, NXDATALIST2Lib::dlLinkStyleTrue);

				pDlg->m_pNewTree->AddRowAtEnd(pRow, pSymptomRow);

				prsProc->MoveNext();
			}
			prsProc->Close();
		}
		
		//load the list of all procedures we are currently tracking for this patient
		{
			BOOL bExpanded = FALSE;

			CString strSql;
			strSql.Format("SELECT ProcInfoT.ID AS ProcInfoID, PicT.ID AS PicID, dbo.CalcProcInfoName(ProcInfoT.ID) AS Procedures "
					"FROM ProcInfoT  "
					"INNER JOIN PicT ON ProcInfoT.ID = PicT.ProcInfoID  "
					"WHERE ProcInfoT.PatientID = %li AND PicT.EmrGroupID Is Null AND PicT.IsCommitted = 1", GetActivePatientID());
			_RecordsetPtr prsTracked = pDlg->m_pRefreshNewListConn->Execute((LPCTSTR)strSql, NULL, 0);
			while(!prsTracked->eof) {
				long nProcInfoID = AdoFldLong(prsTracked, "PicID");
				CString strProcedures = AdoFldString(prsTracked, "Procedures", "");

				strProcedures.TrimRight();

				// (j.jones 2006-06-16 09:05) - PLID 21051 - if a ladder has no procedures,
				// it is pointless to display a blank line, so don't
				if(strProcedures.IsEmpty()) {
					prsTracked->MoveNext();
					continue;
				}

				//Setup a new row with the information we got from the database
				NXDATALIST2Lib::IRowSettingsPtr pRow = pDlg->m_pNewTree->GetNewRow();
				pRow->PutValue(necID, (long)nProcInfoID);
				pRow->PutValue(necParent, (long)PROCEDURE_TRACKED_ID);
				pRow->PutValue(necDescription, _bstr_t(strProcedures));
				pRow->PutValue(necType, (short)neltProcInfo);

				//Make the description cell a hyperlink, for ease of use
				pRow->PutCellLinkStyle(necDescription, NXDATALIST2Lib::dlLinkStyleTrue);

				pDlg->m_pNewTree->AddRowAtEnd(pRow, pTrackedRow);

				// (j.jones 2006-06-16 08:53) - PLID 21050 - if we added any row, auto-expand this section
				pProcedureRow->Expanded = TRUE;
				pTrackedRow->Expanded = TRUE;
				bExpanded = TRUE;

				prsTracked->MoveNext();
			}
			prsTracked->Close();
		}

		//preferred procedures
		{
			CString strSql;
			strSql.Format("SELECT ID, Name "
					"FROM ProcedureT "
					"WHERE ProcedureT.EMRPreferredProcedure = 1 "
					"ORDER BY ProcedureT.Name");
			_RecordsetPtr prsProc = pDlg->m_pRefreshNewListConn->Execute((LPCTSTR)strSql, NULL, 0);
			while(!prsProc->eof) {
				long nID = AdoFldLong(prsProc, "ID");
				CString strName = AdoFldString(prsProc, "Name", "");

				//Setup a new row with the information we got from the database
				NXDATALIST2Lib::IRowSettingsPtr pRow = pDlg->m_pNewTree->GetNewRow();
				pRow->PutValue(necID, (long)nID);
				pRow->PutValue(necParent, (long)PROCEDURE_PREFERRED_ID);
				pRow->PutValue(necDescription, _bstr_t(strName));
				pRow->PutValue(necType, (short)neltProcedure);

				//Make the description cell a hyperlink, for ease of use
				pRow->PutCellLinkStyle(necDescription, NXDATALIST2Lib::dlLinkStyleTrue);

				pDlg->m_pNewTree->AddRowAtEnd(pRow, pPreferredRow);

				prsProc->MoveNext();
			}
			prsProc->Close();
		}

		//other procedures
		{
			// (j.jones 2006-07-17 16:57) - PLID 21474 - this list is filtered on all procedures
			// that have an associated EMR template
			CString strSql;
			strSql.Format("SELECT ID, Name FROM ProcedureT "
				"WHERE ID IN (SELECT ProcedureID FROM EMRTemplateProceduresT "
				"INNER JOIN EMRTemplateT ON EMRTemplateProceduresT.EMRTemplateID = EMRTemplateT.ID WHERE Deleted = 0) "
				"ORDER BY ProcedureT.Name");
			_RecordsetPtr prsProc = pDlg->m_pRefreshNewListConn->Execute((LPCTSTR)strSql, NULL, 0);
			while(!prsProc->eof) {
				long nID = AdoFldLong(prsProc, "ID");
				CString strName = AdoFldString(prsProc, "Name", "");

				//Setup a new row with the information we got from the database
				NXDATALIST2Lib::IRowSettingsPtr pRow = pDlg->m_pNewTree->GetNewRow();
				pRow->PutValue(necID, (long)nID);
				pRow->PutValue(necParent, (long)PROCEDURE_ALL_ID);
				pRow->PutValue(necDescription, _bstr_t(strName));
				pRow->PutValue(necType, (short)neltProcedure);

				//Make the description cell a hyperlink, for ease of use
				pRow->PutCellLinkStyle(necDescription, NXDATALIST2Lib::dlLinkStyleTrue);

				pDlg->m_pNewTree->AddRowAtEnd(pRow, pOtherRow);

				prsProc->MoveNext();
			}
			prsProc->Close();
		}
	}
	catch (_com_error &e)
	{
		CString strError;
		strError.Format("Could not load new list. Error: %s.", e.ErrorMessage());
		MessageBox(GetActiveWindow(), strError, "NexTech Practice", MB_ICONEXCLAMATION|MB_OK);
	}

	if(pDlg->m_pRefreshNewListConn)
		pDlg->m_pRefreshNewListConn.Release();

	// Now tell the window that we are done loading the list
	if (pDlg->GetSafeHwnd())
		pDlg->PostMessage(ID_NEXEMR_NEWLIST_LOADED);
	return 0;
}*/

//(e.lally 2011-10-11) PLID 44728 - Refreshes just the entries under Active NexTrack Procedures
//	Procedure based section must be showing.
void CPatientNexEMRDlg::RefreshNewTreeActiveNexTrackProcedures()
{
	if (m_pNewTree->IsRequerying()) {
		ASSERT(FALSE);
		m_pNewTree->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
	}
	NXDATALIST2Lib::IRowSettingsPtr pParentRow = m_pNewTree->FindByColumn(necID, (long)PROCEDURE_TRACKED_ID, NULL, VARIANT_FALSE);
	if(pParentRow == NULL){
		//This should never happen. We couldn't find the "Active NexTrack Procedures" row. 
		//	Did the caller forget to check if it was visible?
		ASSERT(FALSE);
		//As a failsafe we'll refresh the whole tree.
		RefreshNewList();
		return;
	}
	m_pNewTree->SetRedraw(VARIANT_FALSE);
	
	//(e.lally 2011-10-11) PLID 44728 - Remove the child rows of procedures
	{
		NXDATALIST2Lib::IRowSettingsPtr pChildRow = pParentRow->GetFirstChildRow();
		while(pChildRow != NULL){
			m_pNewTree->RemoveRow(pChildRow);
			pChildRow = pParentRow->GetFirstChildRow();
		}
	}
	
	NXDATALIST2Lib::IRowSettingsPtr pNewRow = NULL;

	//(e.lally 2011-10-11) PLID 44728 - Query the database and manually fill in the tree.
	// (j.armen 2012-02-14 11:15) - PLID 48131 - Use the parameratized version
	_RecordsetPtr rs = CreateParamRecordset("{SQL}\r\nORDER BY Description", GetNewTreeActiveNexTrackSql());
	FieldsPtr pFields = rs->Fields;
	while(!rs->eof){
		pNewRow = m_pNewTree->GetNewRow();
		if(pNewRow != NULL){
			long nID = AdoFldLong(pFields, "ID");
			pNewRow->PutValue(necID, (long)nID);
			_variant_t varParent = pFields->Item["Parent"]->Value;
			if(varParent.vt == VT_I4){
				long nParent = AdoFldLong(pFields, "Parent");
				pNewRow->PutValue(necParent, (long)nParent);
			}
			long nType = AdoFldLong(pFields, "Type");
			pNewRow->PutValue(necType, (long)nType);
			CString strDescription = AdoFldString(pFields, "Description", "");
			pNewRow->PutValue(necDescription, _bstr_t(strDescription));
			long nRecordID = AdoFldLong(pFields, "RecordID");
			pNewRow->PutValue(necRecordID, (long)nRecordID);
			long nColor = AdoFldLong(pFields, "Color");
			pNewRow->PutValue(necColor, (long)nColor);

			m_pNewTree->AddRowSorted(pNewRow, pParentRow);
		}

		rs->MoveNext();
	}

	SetNewListStylesRecurse(pParentRow);

	m_pNewTree->SetRedraw(VARIANT_TRUE);
}

//(e.lally 2011-10-11) PLID 44728 - SQL for just the "Active NexTrack Procedures" section
// (j.armen 2012-02-14 11:15) - PLID 48131 - Parameratized this function
CSqlFragment CPatientNexEMRDlg::GetNewTreeActiveNexTrackSql()
{
	CSqlFragment sqlExcludeInactive;
	if(!GetRemotePropertyInt("EMRIncludeInactiveLadders",0,0,"<None>",true))
		sqlExcludeInactive.Create("AND (LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1) OR LaddersT.ID IS NULL)");

	//load the list of all procedures we are currently tracking for this patient
	return CSqlFragment(
		"SELECT 0 AS ID, {INT} AS Parent, {INT} AS Type, dbo.CalcProcInfoName(ProcInfoT.ID) AS Description, 0 AS Color, PicT.ID AS RecordID, 0 AS SortOrder "
		"FROM ProcInfoT  "
		"INNER JOIN PicT ON ProcInfoT.ID = PicT.ProcInfoID  "
		"LEFT JOIN LaddersT ON ProcInfoT.ID = LaddersT.ProcInfoID "
		"WHERE ProcInfoT.PatientID = {INT} AND PicT.EmrGroupID Is Null AND PicT.IsCommitted = 1 {SQL}",
		PROCEDURE_TRACKED_ID, neltProcInfo,	GetActivePatientID(), sqlExcludeInactive);	
}

void CPatientNexEMRDlg::RefreshNewList()
{
	try {
		//
		// (c.haag 2006-07-18 13:15) - PLID 20889 - This is basically RefreshNewListThread but done
		// entirely with a query. We have to intercept OnRequeryFinished to assign the hyperlinks, but
		// everything else is done right here.
		//
		// Couple notes:
		//  - The ID and Parent MUST not use record ID's such as ProcedureT.ID or PicT.ID.
		// (a.wetta 2007-06-12 14:34) - PLID 26234 - Except the custom EMN template groups need to use the EMN group IDs as the ID and Parent
		//  - The SortOrder defines the order of the nodes in the tree
		//
		//TES 8/2/2006 - PLID 21687 - Check the preference to show completed ladders in the Active NexTrack Procedures section..
		//
		// (z.manning, 02/20/2007) - PLID 24838 - Don't show templates from inactive collections
		// (for sympton/subjective part of the list).
		// (j.armen 2012-02-14 11:16) - PLID 48131 - Store this as a CSqlFragment so that we can pass a param version to the datalist
		CSqlFragment sql("(");

		// (a.wetta 2007-06-13 10:19) - PLID 26234 - Only show the preferred procedures if they have them set to show
		// (c.haag 2008-12-19 11:13) - PLID 32521 - Hide inactive procedures
		//(e.lally 2011-10-11) PLID 44728 - Put section for "Active NexTrack Procedures" in its own function
		if (GetRemotePropertyInt("ShowEMRPreferredProcedures", 1)) {

			//Root rows - These are the base rows of the tree.  All other rows added will be children below these.
			sql += CSqlFragment("SELECT {CONST_INT} AS ID, NULL AS Parent, {CONST_INT} AS Type, {STRING} AS Description, {CONST_INT} AS Color, -1 AS RecordID, {CONST_INT} AS SortOrder\r\n",	
				PROCEDURE_BASED_ID, neltHeader, PROCEDURE_BASED_NAME, HEADER_FORECOLOR, PROCEDURE_BASED_ID);

			//We then need to load each of the children rows

			//First the Procedure Children

			//We have 3 hard-coded headers under the "By Procedure" option, we want to add all of them as children
			//of the given parent.

			//1)  Preferred Procedures - These are procedures that this office performs regularly.  This 
			//	will not include procedures which are in the "tracked" section.
			sql += CSqlFragment("UNION SELECT {CONST_INT} AS ID, {CONST_INT} AS Parent, {CONST_INT} AS Type, {STRING} AS Description, {CONST_INT} AS Color, -1 AS RecordID, {CONST_INT} AS SortOrder\r\n",
				PROCEDURE_PREFERRED_ID, PROCEDURE_BASED_ID, neltHeader, PROCEDURE_PREFERRED_NAME, HEADER_FORECOLOR, PROCEDURE_PREFERRED_ID);

			//2)  Tracked Procedures - These are procedures currently tracked by this patient
			sql += CSqlFragment("UNION SELECT {CONST_INT} AS ID, {CONST_INT} AS Parent, {CONST_INT} AS Type, {STRING} AS Description, {CONST_INT} AS Color, -1 AS RecordID, {CONST_INT} AS SortOrder\r\n",
				PROCEDURE_TRACKED_ID, PROCEDURE_BASED_ID, neltHeader, PROCEDURE_TRACKED_NAME, HEADER_FORECOLOR, PROCEDURE_TRACKED_ID);

			//3)  All Other Procedures - This is collapsed by default, but will show all other procedures
			//	if expanded.
			sql += CSqlFragment("UNION SELECT {CONST_INT} AS ID, {CONST_INT} AS Parent, {CONST_INT} AS Type, {STRING} AS Description, {CONST_INT} AS Color, -1 AS RecordID, {CONST_INT} AS SortOrder\r\n",
				PROCEDURE_ALL_ID, PROCEDURE_BASED_ID, neltHeader, PROCEDURE_ALL_NAME, HEADER_FORECOLOR, PROCEDURE_ALL_ID);

			//load the list of all procedures we are currently tracking for this patient
			//(e.lally 2011-10-11) PLID 44728 - Put this SQL segment in its own function		
			sql += CSqlFragment("UNION {SQL}\r\n", GetNewTreeActiveNexTrackSql());

			//preferred procedures
			sql += CSqlFragment(
				"UNION SELECT * FROM (SELECT TOP 100 PERCENT 0 AS ID, {CONST_INT} AS Parent, {CONST_INT} AS Type, Name AS Description, 0 AS Color, ID AS RecordID, 0 AS SortOrder "
				"FROM ProcedureT "
				"WHERE ProcedureT.Inactive = 0 AND ProcedureT.EMRPreferredProcedure = 1 "
				"ORDER BY ProcedureT.Name) SubQ1\r\n",
				PROCEDURE_PREFERRED_ID, neltProcedure);

			//other procedures
			sql += CSqlFragment(
				"UNION SELECT * FROM (SELECT TOP 100 PERCENT 0 AS ID, {CONST_INT} AS Parent, {CONST_INT} AS Type, Name AS Description, 0 AS Color, ID AS RecordID, 0 AS SortOrder "
				"FROM ProcedureT "
				"WHERE ProcedureT.Inactive = 0 AND ID IN (SELECT ProcedureID FROM EMRTemplateProceduresT "
				"INNER JOIN EMRTemplateT ON EMRTemplateProceduresT.EMRTemplateID = EMRTemplateT.ID WHERE Deleted = 0) "
				"ORDER BY ProcedureT.Name) SubQ2\r\n",
				PROCEDURE_ALL_ID, neltProcedure);

			sql += CSqlFragment("UNION ");
		}		

		// (a.wetta 2007-06-12 09:36) - PLID 26234 - Add the custom group templates
		// Add the custom group headers
		sql += CSqlFragment("SELECT * FROM (SELECT TOP 100 PERCENT ID, NULL AS Parent, {CONST_INT} AS Type, Name AS Description, {CONST_INT} AS Color, -1 AS RecordID, SortOrder FROM EMNTemplateGroupsT ORDER BY SortOrder) CustomGroupsQ\r\n",
			neltHeader, HEADER_FORECOLOR);

		// Add all of the templates for the custom groups
		sql += CSqlFragment(
			"UNION SELECT * FROM (SELECT TOP 100 PERCENT 0 AS ID, EMNTemplateGroupID AS Parent, {CONST_INT} AS Type, EMRTemplateT.Name AS Description, 0 AS Color, EMRTemplateT.ID AS RecordID, EMNTemplateGroupsLinkT.SortOrder "
			"FROM EMNTemplateGroupsLinkT "
			"LEFT JOIN EMRTemplateT ON EMNTemplateGroupsLinkT.EMRTemplateID = EMRTemplateT.ID "
			"LEFT JOIN EMRCollectionT ON EMRTemplateT.CollectionID = EMRCollectionT.ID "
			"WHERE EMRTemplateT.Deleted = 0 AND EMRCollectionT.Inactive = 0 "
			"ORDER BY EMNTemplateGroupsLinkT.SortOrder) TemplateSubQ"
			") FinalQ",	
			neltTemplate);

		// (z.manning 2009-08-11 17:02) - PLID 32989 - Get the ID of the custom group to auto expand so
		// we can do so when the requery finishes.
		m_nGroupIDToAutoExpand = GetRemotePropertyInt("EMRAutoExpandGroupID", -1, 0, GetCurrentUserName());

		if (m_pNewTree->IsRequerying()) {
			m_pNewTree->CancelRequery();
		}

		// (j.armen 2012-02-14 11:20) - PLID 48131 - Use the CSqlFragment in the FromClause
		m_pNewTree->FromClause = _bstr_t(sql.Flatten());
		m_pNewTree->Requery();
		// (a.walling 2010-08-31 12:34) - PLID 29140 - Prevent refreshing the new list more often than necessary
		m_nRefreshNewListCount++;

		// (j.jones 2014-08-21 11:33) - PLID 63189 - reset our tablechecker object, as it is possible
		// we refreshed for reasons other than responding to the tablechecker
		m_templateChecker.Changed();

	} NxCatchAll("Error in PopulateNewList");
}

void CPatientNexEMRDlg::OnColumnClickingNewList(short nCol, BOOL FAR* bAllowSort) 
{
	//disallow sorting
	*bAllowSort = FALSE;
}

void CPatientNexEMRDlg::OnRowExpandedNewList(LPDISPATCH lpRow) 
{
	try {
		//When we expand a row, we want to go through and collapse every other row
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);


		//There are a few options:
		//	- We newly expanded the Subjective based.  Symptom or Procedure (plus children) must collapse
		//	- We newly expanded the Symptom based.     Subjective or Procedure (plus children) must collapse
		//	- We newly expanded the Procedure based.   Symptom or Subjective must collapse.
		//	- We newly expanded a child of the Procedure based.  Nothing changes, because we already had Procedure expanded, 
		//		so no other row could have been.

		//If the row has a parent, we are in case #4
		if(pRow->GetParentRow()) {
			//Do nothing
			//TODO - Do we want to apply the same train of thought to the sub-procedure headers?  Right now you can have all 3
			//	open at once.  I'm leaning toward leaving it this way.
		}
		else {
			//TODO - loop through all root level rows
			NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pNewTree->GetFirstRow();

			//TODO - do some loop here
			while(pCurRow) {

				if(pCurRow->IsSameRow(pRow)) {
					//this is the row that we just expanded, do nothing
				}
				else
					pCurRow->PutExpanded(VARIANT_FALSE);

				pCurRow = pCurRow->GetNextRow();
			}
		}

	} NxCatchAll("Error in OnRowExpandedNewList");
}

void CPatientNexEMRDlg::OnLeftClickNewList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	if(!lpRow)
		return;

	// (a.walling 2007-11-28 10:52) - PLID 28044
	if (m_bExpired) return;

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		//get the data from the row
		long nID = VarLong(pRow->GetValue(necRecordID));
		long nType = VarLong(pRow->GetValue(necType));

		switch(nType) {
		case neltHeader:
			//This is a hard coded header row.  We don't do a thing.
			break;
		case neltProcedure:
			{
				//This row is a new procedure (not a tracked one).  We need to ask the user what template
				//	they want to start with.
				long nTemplateID = -1;
				if(PromptUserForTemplateFromProcedureID(nID, nTemplateID)) {
					HidePreviewWindow(); // (a.walling 2007-06-04 15:13) - PLID 25648
					LaunchPICEditorWithNewEMR(-1, nTemplateID);
				}
			}
			break;
		case neltTemplate:
			{
				HidePreviewWindow(); // (a.walling 2007-06-04 15:13) - PLID 25648

				// (j.gruber 2012-05-31 12:41) - PLID 49046 - made a global function
				BOOL bEMRLaunched = CreateEMNFromTemplate(nID, this);

				if(!bEMRLaunched) {					
					GlobalLaunchPICEditorWithNewEMR(-1, nID);
				}
			}
			break;
		case neltProcInfo:
			//This is a record in ProcInfoT.

			//In the old system, it popped up and you chose a procedure.  Then you got the PIC, and chose what template to start with.  So now
			//	you're choosing the procedure from the tree, and we need to popup a list of available templates (filtered appropriately).
			long nTemplateID = -1;
			if(PromptUserForTemplateFromPic(nID, nTemplateID)) {
				HidePreviewWindow(); // (a.walling 2007-06-04 15:13) - PLID 25648
				LaunchPICEditorWithNewEMR(nID, nTemplateID);
			}
			break;
		}

	} NxCatchAll("Error in OnLeftClickNewList");
}



//////////////////////////////////////////////////////////
//		END Methods for working with the New EMR List	//
//////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////
//		Methods for working with the Filterable Buttons		//
//////////////////////////////////////////////////////////////

//Event handler for the selection changing in the existing EMR list.
//	Calls functions to update the filterable buttons appropriately.
void CPatientNexEMRDlg::OnSelChangedExistingList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {

		if(GetSelectionCountExistingList() == 0) {
			HandleSelectionChange(NULL);
		}

		// (z.manning, 03/30/2007) - PLID 25427 - Tab view now allows multiple selection, so we can't handle
		// selection changing here if we're in tab view.
		if(m_bEMNTabView) {
			// (j.gruber 2008-09-17 11:01) - PLID 31218 - this has to call HandleSelectionList in case they are using the keyboard
			HandleSelectionChange(GetCurSelExistingList());
			return;
		}

		HandleSelectionChange(lpNewSel);

	} NxCatchAll("Error in OnSelChangedExistingEmrList");
}

/*		I pulled these queries out of CEMRGroupDlg::CheckFillTemplateList()

	//This is the query for "all templates"
	CreateRecordset("SELECT EMRTemplateT.ID AS TemplateID, EMRTemplateT.Name AS TemplateName,  "
			"EMRTemplateT.AddOnce, EMRCollectionT.ID AS CollectionID, EMRCollectionT.Name AS CollectionName "
			"FROM "
			"EMRTemplateT LEFT JOIN EMRCollectionT ON EMRTemplateT.CollectionID = EMRCollectionT.ID "
			"WHERE "
			"EmrTemplateT.Deleted = 0 AND EmrCollectionT.Inactive = 0 AND EmrTemplateT.ID IN (SELECT TemplateID FROM EMRTemplateTopicsT) "
			"ORDER BY MenuOrder, EMRTemplateT.Name, EMRCollectionT.Name");

	//This is the "first collection" query
	CreateRecordset("SELECT EMRTemplateT.ID AS TemplateID, EMRTemplateT.Name AS TemplateName,  "
			"EMRTemplateT.AddOnce, EMRCollectionT.ID AS CollectionID, EMRCollectionT.Name AS CollectionName "
			"FROM "
			"EMRTemplateT LEFT JOIN EMRCollectionT ON EMRTemplateT.CollectionID = EMRCollectionT.ID "
			"WHERE "
			"EmrTemplateT.Deleted = 0 AND EmrTemplateT.ID IN (SELECT TemplateID FROM EMRTemplateTopicsT)  "
			"AND CollectionID IN  "
			"(SELECT TOP 1 EMRCollectionT.ID FROM EMRCollectionT INNER JOIN EMRTemplateT ON EMRCollectionT.ID = EMRTemplateT.CollectionID  "
			"INNER JOIN EMRTemplateTopicsT ON EMRTemplateT.ID = EMRTemplateTopicsT.TemplateID WHERE EmrCollectionT.Inactive = 0 ORDER BY MenuOrder) "
			"ORDER BY MenuOrder, EMRTemplateT.Name, EMRCollectionT.Name");

	//This is the "filtered to this EMR" query
	CreateRecordset("SELECT EMRTemplateT.ID AS TemplateID, EMRTemplateT.Name AS TemplateName,  "
			"EMRTemplateT.AddOnce, EMRCollectionT.ID AS CollectionID, EMRCollectionT.Name AS CollectionName "
			"FROM "
			"EMRTemplateT LEFT JOIN EMRCollectionT ON EMRTemplateT.CollectionID = EMRCollectionT.ID "
			"WHERE "
			"EmrTemplateT.Deleted = 0 AND EmrCollectionT.Inactive = 0 AND  "
			"EmrTemplateT.ID IN (SELECT TemplateID FROM EMRTemplateTopicsT)  "
			"AND EmrTemplateT.ID IN (SELECT EMRTemplateID FROM EmrTemplateProceduresT  "
			"WHERE ProcedureID IN (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID IN (SELECT ProcInfoID FROM PicT WHERE EmrGroupID = %li))  "
			"OR ProcedureID IN (SELECT ProcedureID FROM EMRProcedureT WHERE EMRID IN (SELECT ID FROM EMRMasterT WHERE EmrGroupID = %li))  "
			") AND (EmrTemplateT.AddOnce = 0 OR EmrTemplateT.ID NOT IN (SELECT TemplateID FROM EmrMasterT WHERE EmrGroupID = %li)) "
			"ORDER BY MenuOrder, EMRTemplateT.Name, EMRCollectionT.Name", nEMRID, nEMRID, nEMRID);
*/

//Clears & re-filters the list of buttons for creating new EMNs based on the current
//	EMR list.
// (j.jones 2007-06-11 10:49) - PLID 26271 - removed the nEMRID parameter in lieu
// of the pre-loaded array
void CPatientNexEMRDlg::ReFilterButtons()
{
	//Clear the buttons
	ClearDynaButtons();
	
	// (j.jones 2007-06-11 10:52) - PLID 26271 - if we have no templates in the list,
	// show nothing
	if(m_arCurEMRTemplateInfo.GetSize() == 0) {
		return;
	}

	// (c.haag 2006-03-28 16:28) - PLID 19890 - If the user does not have permission to create EMNs,
	// show nothing
	// (j.jones 2007-05-15 17:14) - PLID 25431 - you can't create an EMR
	// without Create and Write permissions
	if (!CheckCurrentUserPermissions(bioPatientEMR, sptCreate, FALSE, 0, TRUE, TRUE)
		|| !CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE)) {
		return;
	}

	// (j.jones 2007-06-11 10:53) - PLID 26271 - the template loading has been moved to BuildCurEMRTemplateInfo,
	// and that should be called prior to ReFilterButtons if the list has changed

	// (j.armen 2012-04-24 17:08) - PLID 49863 - Add first 5 universal templates

	for(int i = 0; i < m_arCurEMRTemplateInfo.GetSize() && i < 5; i++) {
		AddDynaButton(m_arCurEMRTemplateInfo[i].nID, m_arCurEMRTemplateInfo[i].strName);
	}

	// (j.jones 2008-10-14 16:08) - PLID 31691 - add -2 first as "more templates..."
	// (j.armen 2012-04-24 17:09) - PLID 49863 - Then add the "more templates..." last
	AddDynaButton(-2, "More Templates...");
}

void CPatientNexEMRDlg::OnMoreTemplates() 
{
	try {
		//Need to display a list of all available templates.
		CEMRChooseTemplateDlg dlg(this);
		dlg.m_bSilentReload = TRUE;
		if(dlg.DoModal() == IDCANCEL)
			return;

		//template they selected
		long nTemplateID = dlg.m_nSelectedTemplateID;

		//PIC they are currently on
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();
		if(pCurSel == NULL)
			return;

		//and now open the PIC
		LaunchPICEditorWithNewEMN(VarLong(pCurSel->GetValue(eecPicID)), nTemplateID);

	} NxCatchAll("Error in OnMoreTemplates()");
}



//////////////////////////////////////////////////////////////
//		END Methods for working with the Filterable Buttons	//
//////////////////////////////////////////////////////////////

void CPatientNexEMRDlg::OnNexemrConfigureGroups() 
{
	try {
		// (c.haag 2006-04-04 10:59) - PLID 19890 - We now check permissions
		if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
			CConfigureNexEMRGroupsDlg dlg(this);
			if(dlg.DoModal() == IDOK)
				RefreshNewList();
		}
	} NxCatchAll("Error in OnNexemrConfigureGroups");
}

BOOL CPatientNexEMRDlg::PromptUserForTemplateFromProcedureID(IN long nProcedureID, OUT long &nTemplateID)
{
	try {
		CEMRChooseTemplateDlg dlg(this);
		dlg.AddProcedureFilter(nProcedureID);
		if(dlg.DoModal() == IDCANCEL)
			return FALSE;

		nTemplateID = dlg.m_nSelectedTemplateID;

		return TRUE;
	} NxCatchAll("Error in PromptUserForTemplateFromProcedureID");

	return FALSE;
}

BOOL CPatientNexEMRDlg::PromptUserForTemplateFromPic(IN long nPicID, OUT long &nTemplateID)
{
	try {
		CEMRChooseTemplateDlg dlg(this);

		//We need to find all procedures associated with this PIC record, and add them.  Only add master procedures.  We are given a PicID, 
		//	the procedures are in the ProcInfo records.
		// (c.haag 2008-12-19 11:17) - PLID 32521 - Hide inactive procedures from the multi-select list, but tell the user there are inactive procedures
		// before showing the list.
		_RecordsetPtr prs = CreateParamRecordset(
			"SELECT ProcInfoDetailsT.ProcedureID, ProcedureT.Name FROM ProcInfoDetailsT "
			"INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID WHERE ProcInfoID = (SELECT ProcInfoID FROM PicT WHERE ID = {INT}) "
			"AND ProcedureT.Inactive = 1 AND MasterProcedureID IS NULL;\r\n"
			"SELECT ProcInfoDetailsT.ProcedureID, ProcedureT.Name FROM ProcInfoDetailsT "
			"INNER JOIN ProcedureT ON ProcInfoDetailsT.ProcedureID = ProcedureT.ID WHERE ProcInfoID = (SELECT ProcInfoID FROM PicT WHERE ID = {INT}) "
			"AND ProcedureT.Inactive = 0 AND MasterProcedureID IS NULL", nPicID, nPicID);
		if (!prs->eof) {
			CString strWarn = "The following tracked procedures are inactive, and cannot be added to a new EMR:\r\n\r\n";
			while (!prs->eof) {
				strWarn += AdoFldString(prs, "Name") + "\r\n";
				prs->MoveNext();
			}
			strWarn += "\r\nAre you SURE you wish to continue without adding those procedures?";
			if (IDNO == MsgBox(MB_ICONWARNING | MB_YESNO, strWarn)) {
				return FALSE;
			}
		}

		prs = prs->NextRecordset(NULL);
		while(!prs->eof) {
			//Get the ID
			long nID = AdoFldLong(prs, "ProcedureID");
			//Add it as a filter
			dlg.AddProcedureFilter(nID);

			prs->MoveNext();
		}
		prs->Close();

		//Done adding all procedures, we may now launch the list
		if(dlg.DoModal() == IDCANCEL)
			return FALSE;

		nTemplateID = dlg.m_nSelectedTemplateID;
		return TRUE;
	} NxCatchAll("ERror loading template list from tracking.");

	return FALSE;
}

//DRT 12/15/2005 - Copied & modified from patientemrdlg.cpp
void CPatientNexEMRDlg::OnEditEmnTemplate() 
{
	try {
		// (a.walling 2007-11-28 10:19) - PLID 28044
		if (m_bExpired) return;

		// Pop up a menu to let the user choose which collection to base this emr template on
		CRect rc;
		GetDlgItem(IDC_EDIT_EMN_TEMPLATE)->GetWindowRect(&rc);
		long nEMRCollectionID = SelectEMRCollection(rc.right, rc.top, this, FALSE);
		if (nEMRCollectionID != -2) {
			if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
				// (j.gruber 2012-05-31 15:36) - PLID 49054 - move to global function so add this
				StartEMRTemplateWithCollection(nEMRCollectionID, this);
			}
		}

	} NxCatchAll("Error in OnEditEmnTemplate");
}

//DRT 12/15/2005 - Copied & modified from patientemrdlg.cpp
long CPatientNexEMRDlg::SelectEMRCollection(long x, long y, CWnd *pParent, BOOL bIsEMR /*as opposed to an EMR template*/)
{
	CDWordArray arydwIDs;
	
	// Create a popup menu with all the collections as menu items
	_RecordsetPtr prs = CreateRecordset("SELECT ID, Name, MenuOrder FROM EMRCollectionT WHERE Inactive = 0 ORDER BY MenuOrder");
	FieldsPtr pflds = prs->GetFields();
	FieldPtr fldID = pflds->GetItem("ID");
	FieldPtr fldName = pflds->GetItem("Name");
	FieldPtr fldMenuOrder = pflds->GetItem("MenuOrder");

	//(e.lally 2012-03-28) PLID 49275 - Put the list of collections in a submenu if there are a lot of them.
	CMenu mnuCollections;
	mnuCollections.CreatePopupMenu();
	long nLastMenuOrder;
	BOOL bLastMenuOrderSet = FALSE;
	BOOL bGrouping = FALSE;
	while (!prs->eof) {
		// Get the menu order number
		long nMenuOrder = AdoFldLong(fldMenuOrder, -1);
		// If we're now hitting the 2nd item in a group, insert the separator above the first item 
		// in the group; on the other hand, if we WERE grouping and we just hit a new menu order 
		// which would mean we're no longer grouping, then add a separator at the end (before we 
		// add our new menu item below).
		if (bLastMenuOrderSet) {
			if (nMenuOrder == nLastMenuOrder) {
				if (!bGrouping) {
					long nCount = mnuCollections.GetMenuItemCount();
					if (nCount > 1 && !(mnuCollections.GetMenuState(nCount-2, MF_BYPOSITION) & MF_SEPARATOR)) {
						mnuCollections.InsertMenu(nCount-1, MF_SEPARATOR|MF_BYPOSITION);
					}
					bGrouping = TRUE;
				}
			} else if (bGrouping) {
				// Add a separator
				mnuCollections.AppendMenu(MF_SEPARATOR|MF_BYPOSITION);
				bGrouping = FALSE;
			}
		}
		// Add it to the array
		int nIndex = arydwIDs.Add(AdoFldLong(fldID));
		// The command id will be the index plus 1, which leaves 0 as a valid sentinal return 
		// value for TrackPopupMenu() below.
		int nCmdId = nIndex + 1;
		// Add the menu item, passing the command id we just calculated
		mnuCollections.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, nCmdId, AsMenuItem(AdoFldString(fldName)));
		// Remember this order number for next time
		nLastMenuOrder = nMenuOrder;
		if (nLastMenuOrder >= 0) {
			bLastMenuOrderSet = TRUE;
		} else {
			ASSERT(bLastMenuOrderSet == FALSE);
		}
		// Move to the next collection name
		prs->MoveNext();
	}

	CMenu mnu;
	bool bUseSubmenu = false;
	//(e.lally 2012-03-28) PLID 49275 - Put the list of collections in a submenu if there are a lot of them.
	if(arydwIDs.GetSize() > 30){
		bUseSubmenu = true;
		mnu.CreatePopupMenu();
		mnu.InsertMenu(0, MF_BYPOSITION|MF_POPUP, (UINT)mnuCollections.m_hMenu, "&Filter On Collection...");
	}
	else {
		mnu.m_hMenu = mnuCollections.GetSafeHmenu(); 
	}

	// Add the built-in menu items
	{
		// If any came before, add a separator
		BOOL bNeedSeparator = FALSE;
		if (arydwIDs.GetSize() > 0) {
			bNeedSeparator = TRUE;
		}
		if (bIsEMR) {
			if(bNeedSeparator) {
				mnu.AppendMenu(MF_ENABLED|MF_SEPARATOR|MF_BYPOSITION);
				bNeedSeparator = FALSE;
			}
			// One guaranteed one so there's always the option of having a collectionless emr
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, -1, "<&Generic EMR>");
		}
		if ((GetCurrentUserPermissions(bioAdminEMR) & SPT__R_________ANDPASS)) {
			if(bNeedSeparator) {
				mnu.AppendMenu(MF_ENABLED|MF_SEPARATOR|MF_BYPOSITION);
			}
			// One that lets the user add collections
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, -2, "&New Collection...");
			// Add one that lets you manage the collection list
			mnu.AppendMenu(MF_ENABLED|MF_SEPARATOR|MF_BYPOSITION);
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, -3, "Manage &Collections...");
		}
		if ((GetCurrentUserPermissions(bioAdminEMR) & SPT__R_________ANDPASS) && !bIsEMR) {
			if(bNeedSeparator) {
				mnu.AppendMenu(MF_ENABLED|MF_SEPARATOR|MF_BYPOSITION);
			}
			// Let the user manage their templates
			mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, -4, "Manage &Templates...");
		}
	}

	// Pop up the menu to the right of the button
	int nCmdId = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD|TPM_TOPALIGN, x, y, pParent, NULL);
	//This will destroy the menu and any submenus
	mnu.DestroyMenu();

	if (nCmdId == 0) {
		// The user canceled the menu
		return -2;
	} else if (nCmdId == -1) {
		// The user chose the built-in "generic emr" collection
		return -1;
	} else if (nCmdId == -2) {
		// The user wants to create a new collection
		if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
			long nNewEMRCollectionID = CreateNewEMRCollection();
			if (nNewEMRCollectionID == -1) {
				// The user cancelled
				return -2;
			} else {

				// (j.jones 2007-06-11 11:17) - PLID 26271 - our current template info 
				// may be invalid now
				NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();
				if(pCurSel != NULL) {
					//We have a selection, so refresh the dynamic buttons.
					//Find the EMR ID of the current record (it's either the EMR itself, or
					//	an EMN under that EMR).  -1 if it's a whitespace record with no EMR.
					long nEMRID = VarLong(pCurSel->GetValue(eecEMRID),-1);
					// (j.jones 2007-06-11 10:50) - PLID 26271 - build the template info, then refilter buttons
					BuildCurEMRTemplateInfo(nEMRID, TRUE);
					ReFilterButtons();
				}
				else {
					// (j.jones 2007-06-11 11:17) - PLID 26271 - our current template info 
					// may be invalid now
					ClearCurEMRTemplateInfo();
					ReFilterButtons();
				}

				return nNewEMRCollectionID;
			}
		}
		else {
			return -2;
		}
	} else if (nCmdId == -3) {
		// The user wants to manage the whole set of collections rather than select one
		if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
			CEmrCollectionSetupDlg dlg(this);
			// (z.manning, 02/20/2007) - If they saved changes, we need to refresh the new list in case
			// they re/inactivated any collections.
			if(dlg.DoModal() == IDOK) {
				RefreshNewList();

				// (j.jones 2007-06-11 11:17) - PLID 26271 - our current template info 
				// may be invalid now
				NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();
				if(pCurSel != NULL) {
					//We have a selection, so refresh the dynamic buttons.
					//Find the EMR ID of the current record (it's either the EMR itself, or
					//	an EMN under that EMR).  -1 if it's a whitespace record with no EMR.
					long nEMRID = VarLong(pCurSel->GetValue(eecEMRID),-1);
					// (j.jones 2007-06-11 10:50) - PLID 26271 - build the template info, then refilter buttons
					BuildCurEMRTemplateInfo(nEMRID, TRUE);
					ReFilterButtons();
				}
				else {
					// (j.jones 2007-06-11 11:17) - PLID 26271 - our current template info 
					// may be invalid now
					ClearCurEMRTemplateInfo();
					ReFilterButtons();
				}
			}
		}
		// Since the user didn't want to select one, return as if she canceled the menu
		return -2;
	} else if (nCmdId == -4) {
		//The user wants to manage the whole set of collections
		//CEmrTemplateManagerDlg dlg(this);
		//dlg.DoModal();
		if (m_pdlgTemplateManager) {
			// (c.haag 2006-04-04 11:01) - PLID 19890 - Check permissions
			if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
				m_pdlgTemplateManager->DoModal();

				//m.hancock - 4/14/2006 - PLID 20122 - Refresh Dynamic Buttons if you close the Manage Templates dialog
				//Check if we have selected an existing EMR
				NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();
				if(pCurSel != NULL) {
					//We have a selection, so refresh the dynamic buttons.
					//Find the EMR ID of the current record (it's either the EMR itself, or
					//	an EMN under that EMR).  -1 if it's a whitespace record with no EMR.
					long nEMRID = VarLong(pCurSel->GetValue(eecEMRID),-1);
					// (j.jones 2007-06-11 10:50) - PLID 26271 - build the template info, then refilter buttons
					BuildCurEMRTemplateInfo(nEMRID, TRUE);
					ReFilterButtons();
				}
				else {
					// (j.jones 2007-06-11 11:17) - PLID 26271 - our current template info 
					// may be invalid now
					ClearCurEMRTemplateInfo();
					ReFilterButtons();
				}

			}
		} else {
			ASSERT(FALSE);
		}
		return -2;
	} else {
		// Get the ID out of the array based on the index chosen by the user (remember to 
		// subtract 1 because we have a command id and we want to convert it back to an 
		// index, see above loop).
		return arydwIDs.GetAt(nCmdId - 1);
	}
}


LRESULT CPatientNexEMRDlg::OnMsgEditEMRTemplate(WPARAM wParam, LPARAM lParam)
{
	try {
		// (a.walling 2007-11-28 10:19) - PLID 28044
		if (m_bExpired) return 0;

		// (a.walling 2012-03-02 11:39) - PLID 48469 - Pass to mainframe
		GetMainFrame()->PostMessage(NXM_EDIT_EMR_OR_TEMPLATE, wParam, lParam);

	} NxCatchAll("Error in CPatientNexEMRDlg::OnMsgEditEMRTemplate");

	return 0;
}

LRESULT CPatientNexEMRDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		// (c.haag 2006-03-01 13:31) -  PLID 19208 - We now pass table checker
		// messages to the template list
		// (j.jones 2014-08-08 16:59) - PLID 63182 - this function had nothing in it, so I removed its existence
		/*
		if (m_pdlgTemplateManager && IsWindow(m_pdlgTemplateManager->GetSafeHwnd())) {
			m_pdlgTemplateManager->OnTableChanged(wParam, lParam);
		}
		*/

		//DRT 2/7/2006 - PLID 19203 - I copied this from the old EMR, but removed the
		//	birthdate-caused refresh, as it doesn't change the data at all.

		switch(wParam) {
		case NetUtils::EMRMasterT: {
			try {
				// (j.jones 2006-04-06 12:22) - only refresh if the tablechecker is for this patient,
				// or all patients
				// (j.jones 2014-08-08 16:57) - PLID 63182 - used m_id, and ignored -1 record IDs
				if (lParam == m_id) {
					UpdateView();
				}
			} NxCatchAll("Error in CPatientNexEMRDlg::OnTableChanged:EMRMasterT");
			break;
		}

		case NetUtils::EMRTemplateT:
			// (j.jones 2014-08-12 09:59) - PLID 63189 - we no longer immediately refresh on EMRTemplateT
			// tablecheckers, instead we only refresh immediately if our session made the change, which
			// should call RefreshNewTemplateList()
			break;

		case NetUtils::MailSent:
			// (j.jones 2014-08-04 14:56) - PLID 63182 - MailSent should only be sent in EX form,
			// if a regular version is sent we will not respond to it		
			break;

		// (j.jones 2008-07-17 09:14) - PLID 30730 - handled problems
		case NetUtils::EMRProblemsT:
			try
			{				
				// (j.jones 2008-07-23 16:22) - PLID 30823 - we now get the patient ID
				// as the lParam, making this code simple
				// (j.jones 2014-08-08 16:57) - PLID 63182 - used m_id, and ignored -1 record IDs
				if (lParam == m_id) {
					UpdateView();
				}

			}NxCatchAll("CPatientNexEMRDlg::OnTableChanged:EMRProblemsT");
			break;
		}

		//DRT 2/7/2006 - PLID 19203 - Pre-redesign, we were sending table checker
		//	messages to the dialog if and only if we had an m_pEMRDlg pointer.  Which
		//	only happened if you made a new EMR from the tab.  If you edited an existing one,
		//	the EMR tab never did a thing with table checkers.  In the redesign, we
		//	have no access to the dialog itself to send the table checker any longer.
		//This will be handled from the mainfrm from here out -- PLID 19216

	} NxCatchAll("Error in CPatientNexEMRDlg::OnTableChanged");

	return 0;
}

// (j.jones 2014-08-04 14:55) - PLID 63182 - added Ex handling
LRESULT CPatientNexEMRDlg::OnTableChangedEx(WPARAM wParam, LPARAM lParam) {

	try {
		switch (wParam) {

		case NetUtils::MailSent:

			try
			{
				// (z.manning 2008-07-01 16:07) - PLID 25574 - If we're showing history items we may need
				// to refresh here.
				// (j.jones 2014-08-04 14:56) - PLID 63182 - only refresh if the patient ID is our current patient
				CTableCheckerDetails* pDetails = (CTableCheckerDetails*)lParam;
				long nPatientID = VarLong(pDetails->GetDetailData(TableCheckerDetailIndex::MailSent_DetailIndex::msdiPatientID), -1);

				if (m_btnShowHistory.GetCheck() && nPatientID == m_id) {
					UpdateView();
				}
			}NxCatchAll("CPatientNexEMRDlg::OnTableChangedEx:MailSent");
		}
	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.jones 2014-08-12 09:58) - PLID 63189 - added ability for an outside source to
// refresh the new template list
void CPatientNexEMRDlg::RefreshNewTemplateList()
{
	try {

		// (j.jones 2014-08-12 10:00) - PLID 63189 - this code used to be called upon
		// receipt of an EMRTemplateT tablechecker, instead our session will just
		// call this function if a template changes

		RefreshNewList();

		// (j.jones 2007-06-11 11:17) - PLID 26271 - our current template info 
		// may be invalid now
		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();
		if (pCurSel != NULL) {
			//We have a selection, so refresh the dynamic buttons.
			//Find the EMR ID of the current record (it's either the EMR itself, or
			//	an EMN under that EMR).  -1 if it's a whitespace record with no EMR.
			long nEMRID = VarLong(pCurSel->GetValue(eecEMRID), -1);
			// (j.jones 2007-06-11 10:50) - PLID 26271 - build the template info, then refilter buttons
			BuildCurEMRTemplateInfo(nEMRID, TRUE);
			ReFilterButtons();
		}
		else {
			// (j.jones 2007-06-11 11:17) - PLID 26271 - our current template info 
			// may be invalid now
			ClearCurEMRTemplateInfo();
			ReFilterButtons();
		}

	} NxCatchAll(__FUNCTION__);
}

void CPatientNexEMRDlg::OnBtnPatientsToBeBilled() 
{
	// (a.walling 2007-11-28 10:19) - PLID 28044
	if (m_bExpired) return;
	GetMainFrame()->ShowPatientEMNsToBeBilled();
}

void CPatientNexEMRDlg::OnLockEMN()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = GetCurSelExistingList();

		if(pRow == NULL)
			return;

		//Ensure this row is really an EMN, not an EMR
		_variant_t var = pRow->GetValue(eecEMNID);
		if(var.vt == VT_NULL) {
			AfxMessageBox("You cannot lock an EMR, please choose an individual EMN to lock.");
			return;
		}

		// (z.manning 2009-08-11 15:22) - PLID 24277 - Use lock permission instead of write
		if (!CheckCurrentUserPermissions(bioPatientEMR, sptDynamic3)) {
			return;
		}

		//get the EMN data to lock
		long nEMNID = VarLong(pRow->GetValue(eecEMNID), -1);

		//TES 2/13/2007 - PLID 23401 - Make sure this EMN's providers are licensed.
		if(!AreEmnProvidersLicensed(nEMNID)) {
			return;
		}

		// (d.singleton 2013-07-23 16:39) - PLID 44840 - Having a pop up to warn doctor a diagnosis code and/or CPT needs to be selected before locking an EMN
		if(GetRemotePropertyInt("RequireCPTCodeEMNLocking", 0, 0, "<None>", true) && !ReturnsRecordsParam("SELECT * FROM EMRChargesT WHERE EMRID = {INT} AND Deleted = 0", nEMNID)) {
			AfxMessageBox("There are no CPT codes selected on this EMN, and therefore it will not be locked");
			return;		
		}
		if(GetRemotePropertyInt("RequireDiagCodeEMNLocking", 0, 0, "<None>", true) && !ReturnsRecordsParam("SELECT * FROM EMRDiagCodesT WHERE EMRID = {INT} AND Deleted = 0", nEMNID)) {
			AfxMessageBox("There are no Diagnosis codes selected on this EMN, and therefore it will not be locked");
			return;
		}

		// (c.haag 2008-07-07 16:02) - PLID 30632 - Warn if there are any outstanding todo alarms for this EMN
		// (c.haag 2008-07-11 11:33) - PLID 30550 - Warn of non-detail-specific alarms
		// (z.manning 2016-01-13 13:34) - PLID 67778 - Moved this logic to a shared function
		if (!PromptIfAnyOutstandingTodos(GetRemoteData(), nEMNID, GetSafeHwnd())) {
			return;
		}

		// (b.savon 2015-12-29 10:18) - PLID 58470 - Do not allow an EMN to be locked if it has an incomplete Rx associated with it.
		if (HasUnsettledPrescriptions(nEMNID)) {
			return;
		}
		else {
			// No unsettled prescriptions
		}

		// (b.cardillo 2012-03-08 11:40) - PLID 42207 - Required details
		// Make sure there are no unfilled required details
		if (CEMN::HasVisibleUnfilledRequiredDetails(nEMNID)) {
			// Can't lock an EMN that has unfilled required details
			MsgBox(MB_OK|MB_ICONEXCLAMATION, 
				_T("This EMN may not be locked because it contains at least one required detail that has not been ")
				_T("filled in.  Please complete all required details and try again."));
			// Abort
			return;
		}

		//To lock this EMN, we're going to load the EMR, and then save it.  This is similar to the functionality
		//	 of the EMRTreeWnd::ChangeStatus() function, 
		if(IDYES != MsgBox(MB_YESNO, "You have chosen to lock this EMN.  Once a locked EMN is saved, it will not be possible for any user to make any changes to it, under any circumstances.\n"
			"Are you SURE you wish to do this?")) {
			return;
		}
		
		//Note that table checker messages will be sent after we do the SaveEMRObject, so we
		//do not need to do any refreshing ourself.

		//get old status
		// (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT - Parameterized
		// (j.jones 2011-07-05 17:02) - PLID 44432 - supported custom statuses
		_RecordsetPtr rs = CreateParamRecordset("SELECT Status AS StatusID, "
			"EMRStatusListT.Name AS StatusName "
			"FROM EMRMasterT "
			"LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
			"WHERE EMRMasterT.ID = {INT}", nEMNID);
		if(!rs->eof) {
			long nOldStatusID = AdoFldLong(rs, "StatusID",-1);
			// (j.jones 2011-07-22 14:31) - PLID 44494 - if the status is already locked,
			// do not try to lock it again
			if(nOldStatusID != 2) {
				CString strOldStatusName = AdoFldString(rs, "StatusName", "Unidentified");
				// (a.walling 2008-06-25 17:43) - PLID 30515 - Ensure the EMN is not being modified
				// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
				// (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT - Parameterized
				// (j.armen 2013-05-14 12:34) - PLID 56680 - EMN Access restructuring
				long nAffected = 0;
				ExecuteParamSql(GetRemoteData(), &nAffected, 
					"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
					"UPDATE EMRMasterT\r\n"
					"	SET Status = {INT}, ModifiedDate = GetDate()\r\n"
					"WHERE ID = {INT}\r\n"
					"	AND ID NOT IN (SELECT EmnID FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK) WHERE EmnID = {INT})", 2, nEMNID, nEMNID);

				if (nAffected > 0) {
					if(nOldStatusID != 2) {
						long nAuditID = BeginNewAuditEvent();
						AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiEMNStatus, nEMNID, strOldStatusName, "Locked", aepHigh, aetChanged);
					}

					// (c.haag 2008-07-17 15:15) - PLID 30550 - We also need to move todos related to the EMR to the EMRLockedTodos table.
					ExecuteSql(GetEMRLockTodoAlarmSql(nEMNID));

					//this table checker will cause the screen to refresh, so we don't need to now
					CClient::RefreshTable(NetUtils::EMRMasterT, GetActivePatientID());
				} else {
					MessageBox("The EMN is being modified by another user and cannot be locked");
				}
			}
			else {
				// (j.jones 2011-07-22 14:31) - PLID 44494 - If the status is already locked,
				// we really should not have had the option to lock it again, so perhaps our
				// tablechecker was lost? Send a new one.
				CClient::RefreshTable(NetUtils::EMRMasterT, GetActivePatientID());
			}
		}
		rs->Close();

	} NxCatchAll("Error in OnLockEMN");
}

void CPatientNexEMRDlg::SecureControls()
{
	try {
		// (c.haag 2006-03-28 12:45) - PLID 19890 - Disable buttons if the user doesn't have
		// permission to do various things
		// (j.jones 2007-05-15 17:14) - PLID 25431 - you can't create an EMR
		// without Create and Write permissions
		// (a.walling 2007-11-28 10:04) - PLID 28044 - Or if your license is expired
		if (!CheckCurrentUserPermissions(bioPatientEMR, sptCreate, FALSE, 0, TRUE, TRUE)
			|| !CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE)
			|| m_bExpired) {
			GetDlgItem(IDC_NEW_LIST)->EnableWindow(FALSE);
		}
		// (a.walling 2007-11-28 10:05) - PLID 28044 - Check for expired here too
		if (!CheckCurrentUserPermissions(bioAdminEMR, sptRead, FALSE, 0, TRUE, TRUE) || m_bExpired) {
			GetDlgItem(IDC_NEXEMR_CONFIGURE_GROUPS)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_EMN_TEMPLATE)->EnableWindow(FALSE);
			// (z.manning 2009-08-25 09:22) - PLID 31944 - Added lock manager button
			GetDlgItem(IDC_NEXEMR_LOCK_MANAGER)->EnableWindow(FALSE);
		}
		// (a.walling 2007-11-28 10:02) - PLID 28044 - Moved this from InitDialog and added check for Expired
		// (a.wetta 2007-03-27 17:31) - PLID 25384 - Don't let them change the tab category if they don't have write permissions
		if (!CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE) || m_bExpired) {
			m_pExistingList->GetColumn(eecTabCategoryID)->PutEditable(FALSE);
		}

		// (a.walling 2007-11-28 11:09) - PLID 28044
		if (m_bExpired) {
			GetDlgItem(IDC_BTN_PATIENTS_TO_BE_BILLED)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_CONFIGURE_TABS)->EnableWindow(FALSE);
			GetDlgItem(IDC_BTN_EMR_SEENTODAY)->EnableWindow(FALSE);
		}
	}  NxCatchAll("Error in SecureControls");
}

void CPatientNexEMRDlg::OnSize(UINT nType, int cx, int cy) 
{
	CNxDialog::OnSize(nType, cx, cy);

	try {
		// (j.armen 2012-04-24 17:10) - PLID 49863 - Make sure the dynamic buttons exist.
		//  Then refilter the buttons as the text display might change based on size
		if(IsWindow(m_btnDynamic1.GetSafeHwnd())) {
			SetControlPositions();
			ReFilterButtons();
		}
	}  NxCatchAll(__FUNCTION__);
	
}

void CPatientNexEMRDlg::OnRequeryFinishedExistingList(short nFlags) 
{
	// (a.walling 2014-07-17 09:59) - PLID 62927 - Preview popup sporadically changes to 'Please select an EMN for the preview to appear.' message
	// if the requery was cancelled, that simply means another one is in progress and this event will fire again; defer any handling until then. 
	// otherwise the datalist will be empty, and we will send an empty list of EMNs to the popup preview!
	if (nFlags == NXDATALIST2Lib::dlRequeryFinishedCanceled) {
		return;
	}

	try {
		//If we're already processing, abort and restart
		KillEMRListThread(240);

		// (a.wetta 2007-01-17 16:34) - PLID 24224 - If the user is using the tabbed view, then there are no children rows
		if (m_bEMNTabView) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->GetFirstRow();
			while (NULL != pRow) {
				// (z.manning 2008-07-02 11:46) - PLID 30596 - Added a column to preview EMNs
				long nEmnID = VarLong(pRow->GetValue(eecEMNID), -1);
				if(nEmnID != -1) {
					pRow->PutValue(eecPreviewIcon, (long)m_hIconPreview);
				}

				pRow = pRow->GetNextRow();
			}
		}
		else {
			// (c.haag 2006-07-03 10:58) - PLID 19977 - Populate the problem flag field
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->GetFirstRow();
			while (NULL != pRow) {

				NXDATALIST2Lib::IRowSettingsPtr pChild = pRow->GetFirstChildRow();
				while (NULL != pChild) {
					// (z.manning 2008-07-02 11:46) - PLID 30596 - Added a column to preview EMNs
					long nEmnID = VarLong(pChild->GetValue(eecEMNID), -1);
					if(nEmnID != -1) {
						pChild->PutValue(eecPreviewIcon, (long)m_hIconPreview);
					}

					pChild = pChild->GetNextRow();
				}

				pRow = pRow->GetNextRow();
			}
		}

		// (a.walling 2007-04-23 08:58) - PLID 24535 - This was being done in the thread, but it's very quick, so I've put it here.

		// (j.jones 2008-05-02 11:18) - PLID 28027 - the date range isn't possible when in the
		// tab view, so don't bother trying to process it when in that view
		if(!m_bEMNTabView) {

			NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pExistingList->GetFirstRow();
			//loop once to expand immediately and set the dates
			while(pCurRow) {
				//first expand the row
				pCurRow->PutExpanded(VARIANT_TRUE);

				//now display the date range for the EMR
				COleDateTime dtMin = VarDateTime(pCurRow->GetValue(eecDate), VarDateTime(pCurRow->GetValue(eecInputDate)));
				COleDateTime dtMax = VarDateTime(pCurRow->GetValue(eecMaxDate), VarDateTime(pCurRow->GetValue(eecInputDate)));

				CString strDateRange, strMin, strMax;
				strMin = FormatDateTimeForInterface(dtMin, DTF_STRIP_SECONDS, dtoDate, true);
				strMax = FormatDateTimeForInterface(dtMax, DTF_STRIP_SECONDS, dtoDate, true);
				if(strMin == strMax)
					strDateRange = strMin;
				else
					strDateRange = strMin + " - " + strMax;
				pCurRow->PutValue(eecDate, _bstr_t(strDateRange));

				//move to the next row
				pCurRow = pCurRow->GetNextRow();
			}
		}

		// (a.walling 2007-04-23 09:00) - PLID 24535 - Start our colorization/word icon thread.

		
		m_pFinishEMRListThread = new CFinishEMRListThread(GetSafeHwnd(), CFinishEMRListThread::FinishEMRListThreadFunction);

		long nCount = PopulateEMRListObjects(m_pFinishEMRListThread->m_arListObjects);
		// (a.walling 2010-11-01 12:38) - PLID 40965
		m_pFinishEMRListThread->m_nPatientID = m_id;

		if (nCount > 0) {
			// only start the thread if we have more than zero objects, of course.

			// m_bAutoDelete is set in the thread constructor
			m_pFinishEMRListThread->CreateThread();
		} else {
			delete m_pFinishEMRListThread;
			m_pFinishEMRListThread = NULL;
		}

		// (b.cardillo 2006-11-20 10:37) - PLID 22565 - Give a message if any EMNs contain reconstructed details.
		if (nFlags == NXDATALIST2Lib::dlRequeryFinishedCompleted && GetActivePatientID() != m_nReconstructedDetailsLastNotifiedPatientID) {
			// Remember for the future that our active patient id is now the last one we did this check for.
			m_nReconstructedDetailsLastNotifiedPatientID = GetActivePatientID();
			// Do the check.
			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			_RecordsetPtr prs = CreateParamRecordset(
				"SELECT COUNT(DISTINCT EMRMasterT.ID) AS CountToWarn "
				"FROM EMRMasterT INNER JOIN EMRDetailsT ON EMRMasterT.ID = EMRDetailsT.EMRID "
				"INNER JOIN ReconstructedEMRDetailsT ON EMRDetailsT.ID = ReconstructedEMRDetailsT.EMRDetailID "
				"WHERE ReconstructedEMRDetailsT.ReviewStatus IN (-1) AND EMRMasterT.PatientID = {INT}", GetActivePatientID());
			long nCountToWarn = AdoFldLong(prs->GetFields()->GetItem("CountToWarn"));
			if (nCountToWarn > 0) {
				MessageBox(
					"This patient has EMNs with reconstructed details that have not yet "
					"been verified by you.  Please review these EMNs (colored light pink for your convenience) to verify they are correct. \r\n"
					"\r\n"
					// (a.walling 2011-08-23 14:15) - PLID 44647 - exception/error emails to allsupport@nextech.com
					"If you have ANY questions please contact NexTech Technical Support at 888-417-8464 or allsupport@nextech.com.", 
					"Reconstructed Data", MB_ICONINFORMATION|MB_OK);
			}
		}

		// (j.jones 2009-09-22 11:55) - PLID 31620 - Populate the open preview pane
		// with the available EMNs
		if (m_pEMRPreviewPopupDlg)
		{
			// (z.manning 2012-09-10 14:50) - PLID 52543 - Use the new EmnPreviewPopup struct
			CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
			long nIndex = -1;

			//find all EMNIDs in the list
			if (m_bEMNTabView) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->GetFirstRow();
				while(pRow) {
					long nAddEMNID = VarLong(pRow->GetValue(eecEMNID), -1);
					if(nAddEMNID != -1) {
						aryEMNs.Add(EmnPreviewPopup(nAddEMNID, VarDateTime(pRow->GetValue(eecModifiedDate))));
					}

					pRow = pRow->GetNextRow();
				}
			}
			else {
				
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->GetFirstRow();
				while(pRow) {

					NXDATALIST2Lib::IRowSettingsPtr pChild = pRow->GetFirstChildRow();
					while(pChild) {
						
						long nAddEMNID = VarLong(pChild->GetValue(eecEMNID), -1);
						if(nAddEMNID != -1) {
							aryEMNs.Add(EmnPreviewPopup(nAddEMNID, VarDateTime(pChild->GetValue(eecModifiedDate))));
						}

						pChild = pChild->GetNextRow();
					}

					pRow = pRow->GetNextRow();
				}
			}

			m_pEMRPreviewPopupDlg->PreviewEMN(aryEMNs, -1);
		}

	}NxCatchAll("Error reloading patient EMR list.");
}


// (a.walling 2007-04-23 09:11) - PLID 24535 - Add all of our list items to our object array (including their ID and Type)
long CPatientNexEMRDlg::PopulateEMRListObjects(CArray<EMRListObject, EMRListObject> &arListObjects)
{
	arListObjects.RemoveAll();

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->FindAbsoluteFirstRow(VARIANT_FALSE);

		while (pRow) {
			EMRListObject eloObject;

			eloObject.nID = VarLong(pRow->GetValue(eecEMNID), -1);
			eloObject.eotType = eotInvalid;

			// (j.jones 2007-06-19 10:17) - PLID 26276 - this is now obsolete
			/*
			long nStatus = VarLong(pRow->GetValue(eecCompletionStatus), -2);
			eloObject.ecsCompletionStatus = (EMNCompletionStatus)nStatus;
			*/

			if (eloObject.nID == -1) {
				// this must be an EMR row
				eloObject.eotType = eotEMR;
				eloObject.nID = VarLong(pRow->GetValue(eecEMRID), -1);
			} else if (eloObject.nID >= 0) {
				// this is an EMN row
				eloObject.eotType = eotEMN;
			} else {
				eloObject.eotType = eotHistory;
			}

			arListObjects.Add(eloObject);
			pRow = m_pExistingList->FindAbsoluteNextRow(pRow, VARIANT_FALSE);
		}

		return arListObjects.GetSize();
	} NxCatchAll("Error populating EMR Object list");

	return 0;
}

LRESULT CPatientNexEMRDlg::OnEMRListFinished(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2007-04-23 10:26) - PLID 24535 - Everything is cleanly handled in the Thread class destructor,
	// but this may come in handy in the future, so I'll leave the handler in here.

	// (a.walling 2008-05-06 17:22) - PLID 29941 - Set this to NULL so we do not try to access it any longer.
	m_pFinishEMRListThread = NULL;

	return 0;
}

void CPatientNexEMRDlg::KillEMRListThread(long nTimeMSToWait /*= 0*/)
{
	// Destroy our thread object
	if (m_pFinishEMRListThread)
	{
		//m_bCancelThread = true; // (a.walling 2007-02-01 10:41) - PLID 24513 - Unsynchronized, unsafe flag for the thread a la Mirror Link

		// Signal the stop object
		//SetEvent(m_pFinishEMRListThread->m_hStopThreadEvent);

		if (WAIT_TIMEOUT == WaitForSingleObject(m_pFinishEMRListThread, nTimeMSToWait)) {
			// The thread is still going
			LogDetail("EMR List Thread is unresponsive! (waited %li ms)", nTimeMSToWait);

			// we'll wait one more time
			if (WAIT_TIMEOUT == WaitForSingleObject(m_pFinishEMRListThread, nTimeMSToWait)) {
				TerminateThread(m_pFinishEMRListThread, 0);
			}
		}

		m_pFinishEMRListThread = NULL;

		/*
		// Wait for the thread to terminate
		WaitForSingleObject(m_pFinishEMRListThread->m_hThread, nTimeMSToWait);

		// Get the exit code
		DWORD dwExitCode = 0;
		::GetExitCodeThread(m_pFinishEMRListThread->m_hThread, &dwExitCode);
		// See if the thread is still active
		if (dwExitCode == STILL_ACTIVE) {
			// The thread is still going so post a quit message to it and let it delete itself

			PostThreadMessage(m_pFinishEMRListThread->m_nThreadID, WM_QUIT, 0, 0);

			if (WAIT_TIMEOUT == WaitForSingleObject(m_pFinishEMRListThread->m_hThread, nTimeMSToWait)) {
				LogDetail("Terminating EMR List thread!");
				TerminateThread(m_pFinishEMRListThread->m_hThread, 0);				
			}
		} else {


			// (a.walling 2007-01-31 12:08) - PLID 24513
			// we were unconditionally deleting the thread. let's only do it if it is not running.
			delete m_pFinishEMRListThread;

		}
		*/
		
		/*if(m_pFinishEMRListConn)
			m_pFinishEMRListConn.Release();*/
	}
}

void CPatientNexEMRDlg::OnBtnEmrSummary() 
{
	if(!CheckCurrentUserPermissions(bioPatientEMR, sptRead)) {
		return;
	}

	CEMRSummaryDlg dlg(this);
	dlg.m_nPatientID = GetActivePatientID();
	dlg.DoModal();
}

// (j.jones 2009-09-24 12:22) - PLID 31672 - renamed OnMoveEMN to OnMoveEMNToEMR
void CPatientNexEMRDlg::OnMoveEMNToEMR()
{
	// (z.manning, 06/27/2006) - PLID 20354 - Can now move EMNs to another EMR.
	try {

		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();
		if(pCurSel) {
			long nEmnID = VarLong(pCurSel->GetValue(eecEMNID), -1);
			if(nEmnID == -1) {
				AfxThrowNxException("CPatientNexEMRDlg::OnMoveEMNToEMR - Invalid EMN ID for '%s'", 
					VarString(pCurSel->GetValue(eecDescription), ""));
			}
			
			if(CheckCurrentUserPermissions(bioPatientEMR, sptDynamic0)) {
				// (j.jones 2009-09-24 15:04) - PLID 31672 - this is moving between EMRs for the same patient
				// (j.jones 2009-09-28 08:39) - PLID 31672 - removed this value
				CEmrMoveEmnDlg dlgMoveEmn(nEmnID, this);
				if(dlgMoveEmn.DoModal() == IDOK) {
					m_pExistingList->Requery();
				}
			}
		}
		else {
			ASSERT(FALSE);
		}

	}NxCatchAll("CPatientNexEMRDlg::OnMoveEMNToEMR");
}

void CPatientNexEMRDlg::OnBtnEmrProblemList() 
{
	try {
		
		// (j.jones 2008-07-17 08:51) - PLID 30730 - added a modeless EMR Problem List
		CMainFrame *pFrame = GetMainFrame();
		if(pFrame) {
			//the mainframe's ShowEMRProblemList function will handle permission checking
			GetMainFrame()->PostMessage(NXM_OPEN_EMR_PROBLEM_LIST, GetActivePatientID());
		}

	}NxCatchAll("Error in CPatientNexEMRDlg::OnBtnEmrProblemList");
}

void CPatientNexEMRDlg::SetNewListStylesRecurse(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	while (NULL != pRow)
	{
		NewEMRListType eRowType = (NewEMRListType)VarLong(pRow->GetValue(necType));

		NXDATALIST2Lib::IRowSettingsPtr pChild = pRow->GetFirstChildRow();
		switch (VarLong(pRow->GetValue(necParent), 0)) {
		case PROCEDURE_TRACKED_ID:
		case PROCEDURE_PREFERRED_ID:
		case PROCEDURE_ALL_ID:
			pRow->PutCellLinkStyle(necDescription, NXDATALIST2Lib::dlLinkStyleTrue);
			break;
		default:
			break;
		}

		// (a.wetta 2007-06-12 09:58) - PLID 26234 - Additionally, if this is a custom template group, make sure that the 
		// templates get the cell link style
		if (VarLong(pRow->GetValue(necParent), 0) > 0) {
			pRow->PutCellLinkStyle(necDescription, NXDATALIST2Lib::dlLinkStyleTrue);
		}

		if (NULL != pChild) {
			SetNewListStylesRecurse(pChild);
		}

		//
		// (c.haag 2006-07-27 10:05) - PLID 21050 - We now auto-expand the Active NexTrack
		// Procedure tree if it has items in it
		//
		if ((long)PROCEDURE_TRACKED_ID == VarLong(pRow->GetValue(necID))) {
			if (NULL != pChild) {
				pRow->Expanded = TRUE;
				//DRT 8/20/2007 - PLID 26448 - Make sure we check for a NULL parent.  This shouldn't ever happen.
				if(pRow->GetParentRow()) {
					pRow->GetParentRow()->Expanded = TRUE;
				}
			}
		}

		// (z.manning 2009-08-11 17:19) - PLID 32989 - If this is a custom template group row
		// and the ID matches the row this user wants to auto expand then expand it.
		if(eRowType == neltHeader && m_nGroupIDToAutoExpand > 0 && VarLong(pRow->GetValue(necID)) == m_nGroupIDToAutoExpand) {
			pRow->PutExpanded(VARIANT_TRUE);
		}

		pRow = pRow->GetNextRow();
	} 
}

void CPatientNexEMRDlg::OnRequeryFinishedNewList(short nFlags) 
{
	try {
		// (c.haag 2006-07-18 16:35) - PLID 20889 - We have to assign link styles
		// to the leaf nodes
		SetNewListStylesRecurse(m_pNewTree->GetFirstRow());
	}NxCatchAll("CPatientNexEMRDlg::OnRequeryFinishedNewList");
}

void CPatientNexEMRDlg::CheckUnlockedAgedEMNs()
{
	try {
		if (!m_bRemindToLock)
			return;

		// (z.manning 2008-07-30 10:07) - PLID 30883 - The reminder dialog no long shows
		// the count, so let's only check for the existence of aged unlocked EMNs rather
		// than the whole count.
		long nUnlocked = CEMRLockReminderDlg::NumUnlockedAgedEMNs(TRUE);
		m_bRemindToLock = false;

		if (nUnlocked > 0) {
			if (m_pEMRLockReminderDlg.GetSafeHwnd() == NULL)
				m_pEMRLockReminderDlg.Create(IDD_LOCK_REMINDER,GetActiveWindow());

			// (z.manning 2008-07-30 09:55) - PLID 30883 - We no longer show the count of unlocked
			// EMNs on this dialog.  If we ever show them again, you can uncomment this line and
			// take out the call to UpdateText (which is called from SetUnlockedAgedEMNs).
			//m_pEMRLockReminderDlg.SetUnlockedAgedEMNs(nUnlocked);
			m_pEMRLockReminderDlg.UpdateText();

			m_pEMRLockReminderDlg.CenterWindow();
		
			m_pEMRLockReminderDlg.ShowWindow(SW_SHOWNORMAL);
		}
	}NxCatchAll("Error in CPatientNexEMRDlg::CheckUnlockedAgedEMNs()");
}

void CPatientNexEMRDlg::CheckEmrProviderLicensing()
{
	try {
		if (!m_bNeedProviderLicenseCheck)
			return;

		m_bNeedProviderLicenseCheck = false;

		//TES 12/26/2006 - PLID 23403 - If they have > 0 allowed provider licenses, and 0 used licenses, they need to set it up.
		long nAllowed = g_pLicense->GetEMRProvidersCountAllowed();
		long nUsed = g_pLicense->GetEMRProvidersCountUsed();
		if(nAllowed > 0 && nUsed == 0) {
			if(IDYES == MsgBox(MB_YESNO, "You have %li EMR Provider licenses, but have not assigned a provider to any of them.  "
				"Would you like to setup your EMR Provider licenses now?  You will not be able to lock any EMNs until you have "
				"assigned at least one provider to a license.  You can configure your EMR Provider licenses at any time by going "
				"to \"Tools->Licensing->Manage EMR Provider Licenses...\"", nAllowed))
			{
				// (z.manning 2013-02-04 09:25) - PLID 55001 - Set the type to EMR providers
				CLicenseEMRProvidersDlg dlg(lmdtEmrProviders, this);
				dlg.DoModal();
			}
		}

	}NxCatchAll("Error in CPatientNexEMRDlg::CheckEmrProviderLicensing()");
}

// (z.manning 2014-02-04 15:23) - PLID 55001
void CPatientNexEMRDlg::CheckDictationLicensing()
{
	try
	{
		if (!m_bNeedDictationLicenseCheck) {
			return;
		}

		m_bNeedDictationLicenseCheck = false;

		// (z.manning 2014-02-04 15:23) - PLID 55001 - If they have available Nuance licenses but have not set any
		// yet then prompt to to do so.
		long nAllowed = g_pLicense->GetNuanceUserCountAllowed();
		long nUsed = g_pLicense->GetNuanceUserCountUsed();
		if(nAllowed > 0 && nUsed == 0)
		{
			if(IDYES == MsgBox(MB_YESNO, "You have %li dictation user licenses, but have not assigned a user to any of them.  "
				"Would you like to setup your dictation licenses now?\r\n\r\nYou can configure your dictation licenses at any "
				"time by going to \"Tools->Licensing->Manage Dictation User Licenses...\"", nAllowed))
			{
				CLicenseEMRProvidersDlg dlg(lmdtNuanceUsers, this);
				dlg.DoModal();
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CPatientNexEMRDlg::OnBtnEmrSeenToday() 
{
	// (b.spivey, January 23, 2013) - PLID 54765 - Launch the adv. view instead. 
	GetMainFrame()->ShowEMRSearch();	
}

void CPatientNexEMRDlg::OnTabViewCheck() 
{
	// (z.manning, 08/17/2007) - PLID 27109 - The EMR tab view option is now a global preference.
	SetRemotePropertyInt("EMRTabView", IsDlgButtonChecked(IDC_TAB_VIEW_CHECK), 0, "<None>");

	m_bEMNTabView = IsDlgButtonChecked(IDC_TAB_VIEW_CHECK) ? TRUE : FALSE;

	// (z.manning 2008-07-01 10:57) - PLID 25574 - If we're in tab view then show the history
	// checkbox option.
	if(m_bEMNTabView && CheckCurrentUserPermissions(bioPatientHistory, sptRead, FALSE, 0, TRUE, TRUE)) {
		m_btnShowHistory.ShowWindow(SW_SHOW);
		m_btnShowHistory.SetCheck(GetRemotePropertyInt("NexEMRTabShowHistory", BST_UNCHECKED, 0, GetCurrentUserName(), true));
	}
	else {
		m_btnShowHistory.ShowWindow(SW_HIDE);
		m_btnShowHistory.SetCheck(BST_UNCHECKED);
	}

	// (a.wetta 2007-02-21 16:44) - PLID 24405 - Clear out the EMR list before changing the columns of the datalist, 
	// this will prevent an annoying moving around of the columns
	m_pExistingList->Clear();
	EnsureTabsPosition();
	ReloadEMRList();
}

void CPatientNexEMRDlg::EnsureTabsPosition() 
{
	// (z.manning 2010-05-11 17:36) - PLID 38354 - Moved most of the column show/hides outside
	// of the tab view if/else block as this is now supported in non-tab view as well.
	NXDATALIST2Lib::IColumnSettingsPtr pCol;

	// (z.manning, 04/04/2007) - PLID 25518 - Show or hide the date column depending on the option.
	pCol = m_pExistingList->GetColumn(eecDate);
	long nStyle = pCol->GetColumnStyle();
	if(1 == GetRemotePropertyInt("ShowNexEMRColumnDate", 1, 0, "<None>", true)) {
		pCol->StoredWidth = 90;
		nStyle = (nStyle & ~NXDATALIST2Lib::csFixedWidth) | NXDATALIST2Lib::csWidthData; 
	}
	else {
		pCol->StoredWidth = 0;
		nStyle = (nStyle & ~NXDATALIST2Lib::csWidthData) | NXDATALIST2Lib::csFixedWidth; 
	}
	pCol->PutColumnStyle(nStyle);

	// (z.manning, 04/04/2007) - PLID 25518 - Show or hide the EMN description column depending on the option.
	pCol = m_pExistingList->GetColumn(eecDescription);
	nStyle = pCol->GetColumnStyle();
	if(1 == GetRemotePropertyInt("ShowNexEMRColumnEMN Description", 1, 0, "<None>", true)) {
		nStyle = (nStyle & ~NXDATALIST2Lib::csFixedWidth) | NXDATALIST2Lib::csWidthAuto; 
	}
	else {
		nStyle = (nStyle & ~NXDATALIST2Lib::csWidthAuto) | NXDATALIST2Lib::csFixedWidth; 
		pCol->StoredWidth = 0;
	}
	pCol->PutColumnStyle(nStyle);

	// (z.manning, 04/04/2007) - PLID 25518 - Show or hide the word icon column depending on the option.
	pCol = m_pExistingList->GetColumn(eecWordIcon);
	if(1 == GetRemotePropertyInt("ShowNexEMRColumnHistory Documents", 1, 0, "<None>", true)) {
		pCol->StoredWidth = 25;
	}
	else {
		pCol->StoredWidth = 0;
	}

	// (z.manning, 04/04/2007) - PLID 30596 - Show or hide the preview icon column depending on the option.
	pCol = m_pExistingList->GetColumn(eecPreviewIcon);
	if(1 == GetRemotePropertyInt("ShowNexEMRColumnPreview Icon", 1, 0, "<None>", true)) {
		pCol->StoredWidth = 25;
	}
	else {
		pCol->StoredWidth = 0;
	}

	// (r.gonet 04/22/2014) - PLID 61807 - Added a column to view the clinical summary status
	pCol = m_pExistingList->GetColumn(eecClinicalSummaryIcon);
	if(1 == GetRemotePropertyInt("ShowNexEMRColumnClinical Summary Icon", 1, 0, "<None>", true)) {
		pCol->StoredWidth = 25;
	}
	else {
		pCol->StoredWidth = 0;
	}

	// (z.manning, 04/04/2007) - PLID 25518 - Show or hide the problem icon column depending on the option.
	pCol = m_pExistingList->GetColumn(eecProblemBitmap);
	if(1 == GetRemotePropertyInt("ShowNexEMRColumnProblem", 1, 0, "<None>", true)) {
		pCol->StoredWidth = 25;
	}
	else {
		pCol->StoredWidth = 0;
	}

	// (z.manning, 04/04/2007) - PLID 25518 - Show or hide the status column depending on the option.
	pCol = m_pExistingList->GetColumn(eecStatus);
	if(1 == GetRemotePropertyInt("ShowNexEMRColumnStatus", 1, 0, "<None>", true)) {
		pCol->StoredWidth = 12;
	}
	else {
		pCol->StoredWidth = 0;
	}

	// (d.thompson 2010-01-08) - PLID 35925 - Show or hide the provider column
	pCol = m_pExistingList->GetColumn(eecProvider);
	if(1 == GetRemotePropertyInt("ShowNexEMRColumnProvider", 1, 0, "<None>", true)) {
		pCol->StoredWidth = 10;
	}
	else {
		pCol->StoredWidth = 0;
	}

	// (z.manning 2010-05-11 16:47) - PLID 38354 - Secondary provider column
	pCol = m_pExistingList->GetColumn(eecSecondaryProvider);
	if(1 == GetRemotePropertyInt("ShowNexEMRColumnSecondary Provider", 1, 0, "<None>", true)) {
		pCol->StoredWidth = 12;
	}
	else {
		pCol->StoredWidth = 0;
	}

	// (d.lange 2011-04-29 15:54) - PLID 43382 - Added Assistant/Technician column
	pCol = m_pExistingList->GetColumn(eecTechnicians);
	if(1 == GetRemotePropertyInt("ShowNexEMRColumnAssistant/Technician", 1, 0, "<None>", true)) {
		pCol->StoredWidth = 12;
	}else {
		pCol->StoredWidth = 0;
	}

	if (m_bEMNTabView) {
		GetDlgItem(IDC_BTN_CONFIGURE_TABS)->ShowWindow(SW_SHOW);
		// (a.walling 2007-11-28 10:50) - PLID 28044
		GetDlgItem(IDC_BTN_CONFIGURE_TABS)->EnableWindow(m_bExpired ? FALSE : TRUE);
		GetDlgItem(IDC_EMN_TABS)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EMN_TABS)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHART_TABS)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_CHART_TABS)->EnableWindow(TRUE);

		// Reposition the EMN list so that the tabs have room
		CRect rcMarker, rcChartTabs, rcOriginal;
		GetDlgItem(IDC_EMN_TABS)->GetWindowRect(&rcMarker);
		ScreenToClient(rcMarker);
		GetDlgItem(IDC_CHART_TABS)->GetWindowRect(&rcChartTabs);
		ScreenToClient(rcChartTabs);
		GetDlgItem(IDC_EXISTING_LIST)->GetWindowRect(&rcOriginal);
		ScreenToClient(rcOriginal);
		GetDlgItem(IDC_EXISTING_LIST)->SetWindowPos(NULL, rcChartTabs.right, rcMarker.bottom, rcOriginal.right-rcChartTabs.right, rcOriginal.bottom-rcMarker.bottom, SWP_NOZORDER);

		// Show the "Tab Category" column
		// (z.manning, 04/04/2007) - PLID 25518 - Show or hide the category depending on the option.
		pCol = m_pExistingList->GetColumn(eecTabCategoryID);
		if(1 == GetRemotePropertyInt("ShowNexEMRColumnCategory", 1, 0, "<None>", true)) {
			pCol->StoredWidth = 12;
		}
		else {
			pCol->StoredWidth = 0;
		}

		// (z.manning, 04/04/2007) - PLID 25489 - Show the "Tab Chart" column
		// (z.manning, 04/04/2007) - PLID 25518 - Show or hide the chart column depending on the option.
		pCol = m_pExistingList->GetColumn(eecTabChartID);
		if(1 == GetRemotePropertyInt("ShowNexEMRColumnChart", 1, 0, "<None>", true)) {
			pCol->StoredWidth = 12;
		}
		else {
			pCol->StoredWidth = 0;
		}

		// (z.manning, 04/05/2007) - PLID 25518 - Show the EMR description column and rename the regular description column.
		m_pExistingList->GetColumn(eecDescription)->PutColumnTitle("EMN Description");
		pCol = m_pExistingList->GetColumn(eecEMRDescription);
		nStyle = pCol->GetColumnStyle();
		if(1 == GetRemotePropertyInt("ShowNexEMRColumnEMR Description", 1, 0, "<None>", true)) {
			nStyle = (nStyle & ~NXDATALIST2Lib::csFixedWidth) | NXDATALIST2Lib::csWidthAuto; 
		}
		else {
			nStyle = (nStyle & ~NXDATALIST2Lib::csWidthAuto) | NXDATALIST2Lib::csFixedWidth; 
			pCol->StoredWidth = 0;
		}
		pCol->PutColumnStyle(nStyle);

		// (a.wetta 2007-01-19 16:15) - PLID 24320 - They can sort in the tab view
		m_pExistingList->AllowSort = TRUE;
		// (z.manning, 03/30/2007) - PLID 25427 - Also allow multi-selection so they can assign multiple categories at once.
		m_pExistingList->AllowMultiSelect = VARIANT_TRUE;
	}
	else {
		GetDlgItem(IDC_BTN_CONFIGURE_TABS)->ShowWindow(SW_HIDE);
		// (a.walling 2007-11-28 10:50) - PLID 28044
		GetDlgItem(IDC_BTN_CONFIGURE_TABS)->EnableWindow(FALSE);
		GetDlgItem(IDC_EMN_TABS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EMN_TABS)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHART_TABS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CHART_TABS)->EnableWindow(FALSE);

		// Hide the "Tab Category" column
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pExistingList->GetColumn(eecTabCategoryID);
		pCol->StoredWidth = 0;

		// (z.manning, 04/04/2007) - PLID 25489 - Hide the "Tab Chart" column
		pCol = m_pExistingList->GetColumn(eecTabChartID);
		pCol->StoredWidth = 0;

		// (z.manning, 04/05/2007) - PLID 25518 - Hide the EMR description column.
		pCol = m_pExistingList->GetColumn(eecEMRDescription);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible | NXDATALIST2Lib::csFixedWidth);
		// (a.walling 2008-06-11 16:58) - PLID 29524 - If the user manually sized this column, then it will remain sized
		// that much! Even though the stored with is actually 0, the internal 'actual' width is not, and it remains. By
		// resetting it to 0, we would think that it would be reset, but it does not since the new value == the stored value.
		// However we can fix it simply enough just by sending a different value. 1 is chosen arbitrarily. This all occurs
		// before a paint cycle so it causes no flickering and no noticable performance hit (< 1 ms)
		pCol->PutStoredWidth(1);
		pCol->PutStoredWidth(0);
		m_pExistingList->GetColumn(eecDescription)->PutColumnTitle("Description");

		// Reposition the EMN list now that the tabs are gone
		CRect rcMarker, rcChartTabs, rcOriginal;
		GetDlgItem(IDC_EMN_TABS)->GetWindowRect(&rcMarker);
		ScreenToClient(rcMarker);
		GetDlgItem(IDC_CHART_TABS)->GetWindowRect(&rcChartTabs);
		ScreenToClient(rcChartTabs);
		GetDlgItem(IDC_EXISTING_LIST)->GetWindowRect(&rcOriginal);
		ScreenToClient(rcOriginal);
		GetDlgItem(IDC_EXISTING_LIST)->SetWindowPos(NULL, rcChartTabs.left, rcMarker.top, rcOriginal.right-rcChartTabs.left, rcOriginal.bottom-rcMarker.top, SWP_NOZORDER);
	
		// (a.wetta 2007-01-19 16:15) - PLID 24320 - They cannot sort in the ordinary EMR view
		m_pExistingList->AllowSort = FALSE;
		// Reset all of the sort priorities
		// (j.jones 2012-07-20 08:44) - PLID 41791 - This will default the sorting to be
		// service date DESC, input date DESC. For Tab View, the user can change the sort.
		// But for Tree View, it is unchangeable, which is why we have to call it after
		// ensuring the Tree View.
		InitializeExistingListSortOrder();
		
		// (z.manning, 03/30/2007) - PLID 25427 - Don't allow multi select in the non-tab view.
		m_pExistingList->AllowMultiSelect = VARIANT_FALSE;

		// (z.manning, 04/06/2007) - PLID 25518 - Hide the configure columns button as it does not apply to non-tab view.
		// (z.manning 2010-05-11 17:30) - PLID 38354 - We now support this in non-tab view
		//GetDlgItem(IDC_CONFIGURE_NEXEMR_COLUMNS)->ShowWindow(SW_HIDE);
	}
}

void CPatientNexEMRDlg::RefreshEMNTabs()
{
	try {
		/////////////////////////////////
		// CHART TABS
		// (a.wetta 2007-06-28 12:46) - PLID 26094 - Changed the loading of the charts now that the chart tabs are NxTabs

		// (a.wetta 2007-07-11 16:40) - PLID 26094 - Be sure to set the basic tab color since we are over-ridding the tab colors
		m_pChartTabs->PutTabColor(GetSysColor(COLOR_3DFACE));

		// (z.manning, 04/04/2007) - PLID 25489 - Remember what chart was selected then clear them out.
		long nSelectedChartID = GetCurSelChartID();

		// (z.manning, 04/04/2007) - PLID 25489 - Pull the tabs from data.
		// (z.manning 2011-05-19 15:37) - PLID 33114 - Moved the code to load these to a utility function that will also
		// factor in EMR chart permissions.
		_RecordsetPtr prsCharts = GetEmrChartRecordset();
		// Add all of the charts to the chart array
		m_aryChartTabs.RemoveAll();
		while(!prsCharts->eof) 
		{
			EMNTabInfo etiNew;
			etiNew.nID = AdoFldLong(prsCharts, "ID");
			etiNew.strDescription = ConvertToControlText(AdoFldString(prsCharts, "Description"));
			etiNew.nColor = AdoFldLong(prsCharts, "Color", -1);
			m_aryChartTabs.Add(etiNew);
			prsCharts->MoveNext();
		}
		prsCharts->Close();

		// Set the chart tabs
		m_pChartTabs->PutTabWidth(m_aryChartTabs.GetSize()+1);
		m_pChartTabs->PutSize(m_aryChartTabs.GetSize()+1);

		//First tab is always "{All Charts}."
		m_pChartTabs->PutLabel(0, "{All Charts}");
		// (j.jones 2016-04-26 15:26) - NX-100214 - added a ShortLabel version
		m_pChartTabs->PutShortLabel(0, "{All}");
		
		for(int j = 0; j < m_aryChartTabs.GetSize(); j++) {
			m_pChartTabs->PutLabel(j+1, _bstr_t(m_aryChartTabs.GetAt(j).strDescription)) ;
			//  Reset the back color for all tabs, otherwise
			//	if we pull things in and then put them back, we'll end up with multiple 
			//	highlighted items.
			if (m_aryChartTabs.GetAt(j).nColor != -1) {
				m_pChartTabs->SetTabBackColor(j+1, m_aryChartTabs.GetAt(j).nColor);
				// (j.jones 2016-04-18 17:46) - NX-100214 - the NxTab control now defaults
				// to the value of CNexTechDialog::GetDefaultNxTabTextColor(), but we want
				// black text for any user-defined background colors
				m_pChartTabs->SetTabForeColor(j+1, RGB(0,0,0));
			} else {
				m_pChartTabs->ResetTabBackColor(j+1);
				m_pChartTabs->ResetTabForeColor(j+1);
			}
		}		

		//Make sure the right tab is selected.
		BOOL bFoundChartMatch = FALSE;
		for(int k = 0; !bFoundChartMatch && k < m_aryChartTabs.GetSize(); k++) {
			if(m_aryChartTabs.GetAt(k).nID == nSelectedChartID) {
				m_pChartTabs->CurSel = k+1;
				bFoundChartMatch = TRUE;
			}
		}
		if (!bFoundChartMatch) {
			// We still didn't find the one that was selected before.  So switch to the default, the all charts tab.
			m_pChartTabs->PutCurSel(0);
		}

		/////////////////////////////////
		// CATEGORY TABS

		// (a.wetta 2007-07-11 16:40) - PLID 26094 - Be sure to set the basic tab color since we are over-ridding the tab colors
		m_pEMNTabs->PutTabColor(GetSysColor(COLOR_3DFACE));

		//Store the current selected id.
		long nSelID;
		if(m_pEMNTabs->CurSel == -1 || m_pEMNTabs->CurSel >= m_aryEmnCategoryTabs.GetSize()) nSelID = -2;
		else nSelID = m_aryEmnCategoryTabs.GetAt(m_pEMNTabs->CurSel).nID;

		//Load up our tab array.
		m_aryEmnCategoryTabs.RemoveAll();
		
		// (z.manning, 04/04/2007) - PLID 25489 - If we're filtering on a certain chart, make sure we only
		// select categories that are associated with it.
		CString strWhereClause;
		if(GetCurSelChartID() > 0) {
			strWhereClause = FormatString(
				" WHERE ID IN (SELECT EmnTabCategoryID FROM EmnTabChartCategoryLinkT WHERE EmnTabChartID = %li) "
				, GetCurSelChartID());
		}
		_RecordsetPtr rsTabs = CreateRecordset("SELECT ID, Description FROM EMNTabCategoriesT %s ORDER BY Priority ASC", strWhereClause);
		while(!rsTabs->eof) {
			EMNTabInfo etiNew;
			etiNew.nID = AdoFldLong(rsTabs, "ID");
			etiNew.strDescription = ConvertToControlText(AdoFldString(rsTabs, "Description"));
			m_aryEmnCategoryTabs.Add(etiNew);
			rsTabs->MoveNext();
		}
		rsTabs->Close();

		m_pEMNTabs->PutTabWidth(m_aryEmnCategoryTabs.GetSize()+1);
		m_pEMNTabs->PutSize(m_aryEmnCategoryTabs.GetSize()+1);
		
		for(int i = 0; i < m_aryEmnCategoryTabs.GetSize(); i++) {
			m_pEMNTabs->PutLabel(i, _bstr_t(m_aryEmnCategoryTabs.GetAt(i).strDescription)) ;
			//  Reset the back color for all tabs, otherwise
			//	if we pull things in and then put them back, we'll end up with multiple 
			//	highlighted items.
			m_pEMNTabs->ResetTabBackColor(i);
		}		
		
		//Last tab is always "{All Categories}."
		m_pEMNTabs->PutLabel(m_pEMNTabs->GetSize()-1, "{All Categories}");
		// (j.jones 2016-04-26 15:26) - NX-100214 - added a ShortLabel version
		m_pEMNTabs->PutLabel(m_pEMNTabs->GetSize()-1, "{All}");

		//Make sure the right tab is selected.
		if(nSelID == -2) m_pEMNTabs->CurSel = m_pEMNTabs->GetSize()-1;
		else {
			// We used to just loop through and hope 
			// for the best.  Now we detect if we succeeded or not by way of the bFoundMatch 
			// variable.  Also, as a nice side-effect we get out of the loop early when possible.
			BOOL bFoundMatch = FALSE;
			for(int i = 0; !bFoundMatch && i < m_aryEmnCategoryTabs.GetSize(); i++) {
				if(m_aryEmnCategoryTabs.GetAt(i).nID == nSelID) {
					m_pEMNTabs->CurSel = i;
					bFoundMatch = TRUE;
				}
			}
			if (!bFoundMatch) {
				// We still didn't find the one that was selected before, which means the user must 
				// have hidden that category.  So switch to the default, the miscellaneous tab.
				m_pEMNTabs->PutCurSel(m_pEMNTabs->GetSize() - 1);
			}
		}
	}NxCatchAll("Error in RefreshTabs");
}

void CPatientNexEMRDlg::OnBtnConfigureTabs() 
{
	ConfigureEMNTabs();		
}

void CPatientNexEMRDlg::OnSelectTabEMNTabs(short newTab, short oldTab) 
{
	ReloadEMRList();
}

void CPatientNexEMRDlg::OnConfigureCategories() 
{
	ConfigureEMNTabs();	
}

void CPatientNexEMRDlg::ConfigureEMNTabs() 
{
	try {
		
		CConfigureEMRTabsDlg dlg(this);

		if(dlg.DoModal() == IDCANCEL)
			return;

		RefreshEMNTabs();
		ReloadEMRList();


	} NxCatchAll("Error in CPatientNexEMRDlg::ConfigureEMNTabs()");

}

void CPatientNexEMRDlg::OnEditingFinishingExistingList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
	
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long nNewCategoryID = pvarNewValue->vt == VT_EMPTY ? -1 : VarLong(pvarNewValue, -1);
		switch(nCol)
		{
			// (z.manning, 05/23/2007) - PLID 25489 - As of now anyway, it's not possible for embedded combos in a
			// datalist to have different dropdown contents per row, so if they are in the "All charts" tab, then
			// they will have all categories available for selection even though some of the EMNs may have charts
			// selected that are not associated with all categories. So, we need to check here when they change
			// the chart type to make sure the chart/category combination is valid.
			case eecTabCategoryID:
				if(nNewCategoryID != -1) {
					long nChartID = VarLong(pRow->GetValue(eecTabChartID), -1);
					if(nChartID != -1) {
						if(!ReturnsRecords("SELECT EmnTabCategoryID FROM EmnTabChartCategoryLinkT WHERE EmnTabCategoryID = %li AND EmnTabChartID = %li", nNewCategoryID, nChartID)) {
							//DRT 2/15/08 - PLID 28921 - Typo fixed.
							MessageBox("This category is not linked with this EMNs chart.");
							*pbCommit = FALSE;
						}
					}
				}
			break;
		}


	}NxCatchAll("CPatientNexEMRDlg::OnEditingFinishingExistingList");
}

void CPatientNexEMRDlg::OnEditingFinishedExistingList(LPDISPATCH lpRow, short nCol, const _variant_t &varOldValue, const _variant_t &varNewValue, VARIANT_BOOL bCommit)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		long lEMNID = -1;
		long lTabCatID = -1;

		switch(nCol){
		case eecTabCategoryID:
				if (_variant_t(pRow->GetValue(eecEMNID)).vt == VT_NULL) {
					// (a.wetta 2007-01-24 15:02) - Somehow they are editing an EMR and not an EMN's tab category, this is not possible
					return;
				}
				
				// (a.wetta 2007-03-27 17:21) - PLID 25384 - If the user doesn't have permission to write, then don't let them change the category
				if (!CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE)) {
					pRow->PutValue(eecTabCategoryID, varOldValue);
					return;
				}
				
				lEMNID = VarLong(pRow->GetValue(eecEMNID));
				
				// (a.wetta 2007-01-12 13:01) - PLID 24224 - Update the tab category when the user selects a new one from the combo box
				// Make sure the value was acutally changed
				if (varOldValue != varNewValue) {

					// (j.jones 2007-06-11 10:30) - PLID 26270 - put these two statements in a sql batch
					CString strSql = BeginSqlBatch();

					// (a.walling 2008-08-27 14:13) - PLID 30515 - Warn beforehand if possible of any concurrency issues
					if (WarnIfEMNConcurrencyIssuesExist(this, CSqlFragment("{INT}", lEMNID), "The category cannot be changed.")) {
						return;
					}

					// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
					// (a.walling 2008-08-19 10:07) - PLID 30515 - Ensure we don't modify an EMN that is currently being edited
					// (j.armen 2013-05-14 12:34) - PLID 56680 - EMN Access Refactoring
					AddStatementToSqlBatch(strSql, 
						"IF EXISTS(\r\n"
						"	SELECT 1\r\n"
						"	FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK)\r\n"
						"	WHERE EmnID = %li)\r\n"
						"BEGIN\r\n"
						"	RAISERROR('Category cannot be changed; the EMN is currently protected for editing.', 16, 43)\r\n"
						"	ROLLBACK TRAN\r\n"
						"	RETURN\r\n"
						"END\r\n", lEMNID);

					// First delete the old record in EMNTabCategoriesLinkT
					AddStatementToSqlBatch(strSql, "DELETE FROM EMNTabCategoriesLinkT WHERE EMNID = %li", lEMNID);

					if (_variant_t(pRow->GetValue(eecTabCategoryID)).vt != VT_NULL)
						lTabCatID = VarLong(pRow->GetValue(eecTabCategoryID));

					// Make sure that they aren't changing the category to "No Category."  In that case we don't have to insert anything.
					if (lTabCatID != -1) {
						// Now insert the new record to link the EMN and the tab category
						AddStatementToSqlBatch(strSql, "INSERT INTO EMNTabCategoriesLinkT (EMNID, EMNTabCategoryID) VALUES (%li, %li)",
								lEMNID, lTabCatID);
					}

					//now execute the batch
					ExecuteSqlBatch(strSql);

					// (z.manning, 04/19/2007) - PLID 25714 - Audit the category change.
					CString strNewValue = (lTabCatID == -1 || pRow->GetOutputValue(eecTabCategoryID).vt == VT_EMPTY) ? "{No Category}" : VarString(pRow->GetOutputValue(eecTabCategoryID), "");
					long nOldTabCatID = VarLong(varOldValue, -1);
					CString strOldValue = "{No Category}";
					if(nOldTabCatID != -1) {
						strOldValue = VarString(GetTableField("EmnTabCategoriesT", "Description", "ID", nOldTabCatID), "");
					}
					AuditEvent(GetActivePatientID(), GetActivePatientName(), BeginNewAuditEvent(), aeiEMNCategory, lEMNID, strOldValue, strNewValue, aepMedium, aetChanged);
					
					// (z.manning, 04/05/2007) - PLID 25508 - Make sure we don't display any EMNs
					// that are no longer a member of the currectly selected category.
					if(lTabCatID != GetCurSelCategoryID() && GetCurSelCategoryID() != -1) {
						m_pExistingList->RemoveRow(pRow);
					}
				}

			break;

		case eecTabChartID:
			{
				if(pRow->GetValue(eecEMNID).vt == VT_NULL || varNewValue.vt == VT_EMPTY) {
					return;
				}
				
				long nNewChartID = VarLong(varNewValue, -1);

				// (z.manning, 04/04/2007) - PLID 25489 - Make sure they have EMR write permission.
				if (!CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE)) {
					pRow->PutValue(eecTabChartID, varOldValue);
					return;
				}

				// (z.manning 2011-05-20 10:34) - PLID 33114 - Also make sure they have permission to this chart
				if(nNewChartID != -1 && !CheckCurrentUserPermissions(bioEmrCharts, sptView, TRUE, nNewChartID)) {
					pRow->PutValue(eecTabChartID, varOldValue);
					return;
				}

				lEMNID = VarLong(pRow->GetValue(eecEMNID));

				if(varOldValue != varNewValue) {
					// (a.walling 2008-08-19 10:20) - PLID 30515 - Batch these queries
					CString strSqlBatch = BeginSqlBatch();					

					// (a.walling 2008-08-27 14:13) - PLID 30515 - Warn beforehand if possible of any concurrency issues
					if (WarnIfEMNConcurrencyIssuesExist(this, CSqlFragment("{INT}", lEMNID), "The chart cannot be changed.")) {
						return;
					}
					
					// (a.walling 2008-07-28 16:14) - PLID 30515 - Use UPDLOCK, HOLDLOCK
					// (a.walling 2008-08-19 10:07) - PLID 30515 - Ensure we don't modify an EMN that is currently being edited
					// (j.armen 2013-05-14 12:34) - PLID 56680 - EMN Access Refactoring
					AddStatementToSqlBatch(strSqlBatch, 
						"IF EXISTS(\r\n"
						"	SELECT 1\r\n"
						"	FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK)\r\n"
						"	WHERE EmnID = %li)\r\n"
						"BEGIN\r\n"
						"	RAISERROR('Chart cannot be changed; the EMN is currently protected for editing.', 16, 43)\r\n"
						"	ROLLBACK TRAN\r\n"
						"	RETURN\r\n"
						"END\r\n", lEMNID);

					// (z.manning, 04/04/2007) - PLID 25489 - Update the chart type in data.
					if(varOldValue != varNewValue) {
						AddStatementToSqlBatch(strSqlBatch, "DELETE FROM EmnTabChartsLinkT WHERE EmnID = %li", lEMNID);
						if(varNewValue.vt == VT_I4 && nNewChartID > 0) {
							AddStatementToSqlBatch(strSqlBatch, "INSERT INTO EmnTabChartsLinkT (EmnID, EmnTabChartID) VALUES (%li, %li)"
								, lEMNID, nNewChartID);
						}
					}

					ExecuteSqlBatch(strSqlBatch);

					// (z.manning, 04/19/2007) - PLID 25714 - Audit the chart change.
					CString strNewValue = nNewChartID == -1 ? "{No Chart}" : VarString(pRow->GetOutputValue(eecTabChartID), "");
					long nOldTabChartID = VarLong(varOldValue, -1);
					CString strOldValue = "{No Chart}";
					if(nOldTabChartID != -1) {
						strOldValue = VarString(GetTableField("EmnTabChartsT", "Description", "ID", nOldTabChartID), "");
					}
					AuditEvent(GetActivePatientID(), GetActivePatientName(), BeginNewAuditEvent(), aeiEMNChart, lEMNID, strOldValue, strNewValue, aepMedium, aetChanged);

					// (z.manning, 04/04/2007) - PLID 25489 - If we're on a specific chart tab and they changed
					// the chart, we need to remove that EMN from the list.
					if(GetCurSelChartID() > 0 && GetCurSelChartID() != nNewChartID) {
						m_pExistingList->RemoveRow(pRow);
					}

					// (z.manning, 05/23/2007) - PLID 25489 - Check and see if we have a category ID for this EMN.
					// If we do, we need to make sure it's associated with the new chart and if not, then clear it out.
					// (z.manning 2011-09-02 12:14) - PLID 38681 - Don't do this check if they just removed the chart.
					long nCategoryID = VarLong(pRow->GetValue(eecTabCategoryID), -1);
					if(nCategoryID != -1 && nNewChartID != -1) {
						if(!ReturnsRecordsParam("SELECT EmnTabCategoryID FROM EmnTabChartCategoryLinkT WHERE EmnTabCategoryID = {INT} AND EmnTabChartID = {INT}", nCategoryID, nNewChartID)) {
							MessageBox("This EMN's category is not associated with the newly selected chart. The category has been reset to nothing.");
							_variant_t vtNull;
							vtNull.vt = VT_NULL;
							pRow->PutValue(eecTabCategoryID, vtNull);
							OnEditingFinishedExistingList(pRow, eecTabCategoryID, nCategoryID, vtNull, VARIANT_TRUE);
						}
					}
				}
			}
			break;

		default: 
			// This isn't an editable column, so we do nothing
			break;
		}
	}NxCatchAll("Error in CPatientNexEMRDlg::OnEditingFinishedExistingList");
}

int CPatientNexEMRDlg::SetControlPositions()
{
	// (a.wetta 2007-01-19 17:30) - By hiding the existing list here, it prevents it from flashing as the controls are re-positioned
	GetDlgItem(IDC_EXISTING_LIST)->ShowWindow(SW_HIDE);

	long nAns = CNxDialog::SetControlPositions();

	EnsureTabsPosition();

	GetDlgItem(IDC_EXISTING_LIST)->ShowWindow(SW_SHOW);

	return nAns;
}

BOOL CPatientNexEMRDlg::IsEMNTabView() 
{
	return m_bEMNTabView;
}

// (z.manning, 03/30/2007) - PLID 25427 - Returns the number of rows selected in the existing list.
long CPatientNexEMRDlg::GetSelectionCountExistingList()
{
	long nCount = 0;
	if(m_bEMNTabView) {
		// (z.manning, 03/30/2007) - PLID 25427 - Tab view allows multiple selection, so let's go through the rows
		// and find how many are selected.
		// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
		NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_pExistingList->GetFirstSelRow();
		while (pCurSelRow != NULL) {
			nCount++;
			pCurSelRow = pCurSelRow->GetNextSelRow();
		}
	}
	else {
		// (z.manning, 03/30/2007) - PLID 25427 - Non-tab view does not allow multi select, so we either have 0 or 1 selections.
		ASSERT(!m_pExistingList->AllowMultiSelect);
		if(GetCurSelExistingList() != NULL) {
			nCount = 1;
		}
	}

	return nCount;
}

// (z.manning, 03/30/2007) - PLID 25427 - We now handle selection differently when we're in tab view since
// it allows multiple selection, so created a general function to handle selection changes.
void CPatientNexEMRDlg::HandleSelectionChange(LPDISPATCH lpNewSel)
{
	GetDlgItem(IDC_EMR_BUTTON_HEADER)->ShowWindow(SW_HIDE);
	// (z.manning, 03/30/2007) - PLID 25427 - Don't bother with this if we have multiple selections.
	// (a.walling 2007-11-28 10:18) - PLID 28044 - Or if we are expired
	if(!lpNewSel || GetSelectionCountExistingList() != 1 || m_bExpired) {
		//No selection, we need to filter the buttons to nothing
		// (j.jones 2007-06-11 10:50) - PLID 26271 - clear the template info, then refilter buttons
		ClearCurEMRTemplateInfo();
		ReFilterButtons();
		return;
	}

	NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

	long nMailID = VarLong(pRow->GetValue(eecMailID), -1);
	if(nMailID != -1) {
		// (z.manning 2008-07-03 10:59) - PLID 25574 - If this is a history document, don't bother
		// with this.
		m_nxstaticEmrButtonHeader.SetWindowText("");

		// (j.gruber 2008-09-02 13:01) - PLID 31218 - need to clear the buttons also
		ClearCurEMRTemplateInfo();
		ReFilterButtons();
		return;
	}

	//Find the EMR ID of the current record (it's either the EMR itself, or
	//	an EMN under that EMR).  -1 if it's a whitespace record with no EMR.
	long nEMRID = VarLong(pRow->GetValue(eecEMRID),-1);
	// (j.jones 2007-06-11 10:50) - PLID 26271 - build the template info, then refilter buttons,
	// but only build if the m_nCurEMRSelected changed
	if(nEMRID != m_nCurEMRSelected)
		BuildCurEMRTemplateInfo(nEMRID, FALSE);
	ReFilterButtons();

	//Now update the header text to the appropriate name

	// (c.haag 2006-03-28 16:28) - PLID 19890 - But only if we have permission. On the subject,
	// should this even be visible to beign with if there are no buttons? I don't think it should.
	// (j.jones 2007-05-15 17:00) - PLID 25431 - you can't create an EMR
	// without Create and Write permissions
	if (CheckCurrentUserPermissions(bioPatientEMR, sptCreate, FALSE, 0, TRUE, TRUE)
		&& CheckCurrentUserPermissions(bioPatientEMR, sptWrite, FALSE, 0, TRUE, TRUE))
	{
		GetDlgItem(IDC_EMR_BUTTON_HEADER)->ShowWindow(SW_SHOW);
		CString strName;

		//We want to show the name of the EMR, not the EMN.
		NXDATALIST2Lib::IRowSettingsPtr pParent = pRow->GetParentRow();
		if(pParent) {
			//The row we have selected must be a child (EMN), so therefore we'll
			//	use the description from the parent.
			strName = VarString(pParent->GetValue(eecDescription), "");
		}
		else {
			//No parent, we must be on the EMR level.
			strName = VarString(pRow->GetValue(eecDescription), "");
		}

		SetDlgItemText(IDC_EMR_BUTTON_HEADER, "Associate a new EMN with: " + strName);
	}
}

// (z.manning, 03/30/2007) - PLID 25427 - The datalist's CurSel member can't be used when multiple selection is
// allowed, so use this function to get the existing list's current selection.
NXDATALIST2Lib::IRowSettingsPtr CPatientNexEMRDlg::GetCurSelExistingList()
{
	// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this to use Get__SelRow() set of functions instead of Get__SelEnum()
	return m_pExistingList->GetFirstSelRow();
}

// (z.manning, 03/30/2007) - PLID 25427 - Returns the EMN IDs for all currently selected rows.
void CPatientNexEMRDlg::GetAllSelectedEmnIDs(OUT CArray<long> &naryEmnIDs)
{
	// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
	NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_pExistingList->GetFirstSelRow();
	while (pCurSelRow) {
		NXDATALIST2Lib::IRowSettingsPtr pRow = pCurSelRow;
		pCurSelRow = pCurSelRow->GetNextSelRow();

		COleVariant varEmnID = pRow->GetValue(eecEMNID);
		if(varEmnID.vt == VT_I4) {
			naryEmnIDs.Add(VarLong(varEmnID));
		}
	}
}

// (z.manning, 04/06/2007) - PLID 25489 - Returns true if the given row is selected, false otherwise (including if the row is null).
BOOL CPatientNexEMRDlg::IsRowSelected(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if(pRow == NULL) {
		return FALSE;
	}

	// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
	NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_pExistingList->GetFirstSelRow();
	while (pCurSelRow != NULL) {
		NXDATALIST2Lib::IRowSettingsPtr pNextRow = pCurSelRow;
		pCurSelRow = pCurSelRow->GetNextSelRow();

		if(pRow == pNextRow) {
			return TRUE;
		}
	}

	return FALSE;
}

// (z.manning, 04/04/2007) - PLID 25489 - Updates the size of the chart tab control.
// (a.wetta 2007-06-28 12:35) - PLID 26094 - This function is no long necessary now that the chart tabs are NxTabs
/*void CPatientNexEMRDlg::UpdateChartTabsSize()
{
	CRect rcChartTabSize;
	m_tabCharts.GetClientRect(rcChartTabSize);
	// (z.manning, 04/24/2007) - PLID 25489 - There is some extra space at the borders of the tab control,
	// which is why we subtract six from the height here.
	int nTabHeight = (rcChartTabSize.Height() - 6) / m_tabCharts.GetItemCount();
	m_tabCharts.SetItemSize( CSize(nTabHeight, rcChartTabSize.Width()) );
}*/

// (z.manning, 04/04/2007) - PLID 25489 - Returns the chart ID of the chart for the currenctly selected tab,
// or -1 if there is no selection or it does not apply.
long CPatientNexEMRDlg::GetCurSelChartID()
{
	// (a.wetta 2007-06-29 11:32) - PLID 26094 - Updated function to work with NxTabs
	long nSelectedChartID = -1;
	long nChartAryPos = m_pChartTabs->CurSel-1;
	if(nChartAryPos < m_aryChartTabs.GetSize() && nChartAryPos >= 0) {
		nSelectedChartID = m_aryChartTabs.GetAt(nChartAryPos).nID;
	}

	return nSelectedChartID;
}

// (a.wetta 2007-06-28 13:12) - PLID 26094 - No longer used
/*void CPatientNexEMRDlg::OnSelchangeChartTabs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// (z.manning, 04/04/2007) - PLID 25489 - We need to refresh the tabs because different charts can
	// be associated with different categories. Then reload the list.
	RefreshEMNTabs();
	ReloadEMRList();
	
	*pResult = 0;
}*/

// (z.manning, 04/05/2007) - PLID 25508 - Returns the category ID of the chart for the currently selected tab
// or -1 if there's no selection.
long CPatientNexEMRDlg::GetCurSelCategoryID()
{
	long nCurrentTabCategoryID = -1;
	if(m_pEMNTabs->CurSel < m_aryEmnCategoryTabs.GetSize()) {
		nCurrentTabCategoryID = m_aryEmnCategoryTabs.GetAt(m_pEMNTabs->CurSel).nID;
	}

	return nCurrentTabCategoryID;
}

void CPatientNexEMRDlg::OnConfigureNexemrColumns() 
{
	// (z.manning, 04/06/2007) - PLID 25518 - Only let users with admin EMR permissions change the column
	// settings since they are global.
	if(CheckCurrentUserPermissions(bioAdminEMR, sptRead)) {
		CPatientNexEMRConfigureColumnsDlg dlg(this);
		dlg.DoModal();		
	}
}

// (a.walling 2007-04-13 12:59) - PLID 25648 - Update the preview popup with the current selection
void CPatientNexEMRDlg::OnCurSelWasSetExistingList() 
{
	try {
		if (m_pEMRPreviewPopupDlg) {
			PreviewSelectedEMN(); // returns if the preview pane is not visible
		}
	} NxCatchAll("Error in OnCurSelWasSetExistingList");
}

// (a.walling 2007-04-13 12:59) - PLID 25648 - the Show/Hide preview button was pressed.
void CPatientNexEMRDlg::OnShowPreview() 
{
	try {
		CreatePreviewWindow();
		m_bShowPreview = !m_bShowPreview;
		EnsureShowPreviewButton();
		if (m_bShowPreview) {
			if (m_pEMRPreviewPopupDlg) {
				PreviewSelectedEMN();
			}
		} else {
			m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
		}
	} NxCatchAll("Error in OnShowPreview");
}

// (a.walling 2007-04-13 15:52) - PLID 25648 - this unconditionally displays the preview window.
void CPatientNexEMRDlg::OnMenuShowPreview() 
{
	try {
		CreatePreviewWindow();
		m_bShowPreview = TRUE;
		EnsureShowPreviewButton();

		if (m_pEMRPreviewPopupDlg) {
			PreviewSelectedEMN();
		}
	} NxCatchAll("Error in OnShowPreview");
}

// (a.walling 2007-04-16 14:01) - 25648 - If the preview window is not created, then create it.
void CPatientNexEMRDlg::CreatePreviewWindow()
{
	try {
		if (m_pEMRPreviewPopupDlg == NULL) {
			// create the dialog!

			// (a.walling 2007-04-13 09:49) - PLID 25648 - Load and initialize our preview popup
			m_pEMRPreviewPopupDlg = new CEMRPreviewPopupDlg(this);
			m_pEMRPreviewPopupDlg->m_pNexEMRDlg = this;
			m_pEMRPreviewPopupDlg->Create(IDD_EMR_PREVIEW_POPUP, this);

			// (j.jones 2009-09-22 11:55) - PLID 31620 - PreviewEMN now takes in an array
			// of all available EMN IDs, but since we haven't opened the dialog yet,
			// we can pass in an empty array.
			CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNIDs;
			m_pEMRPreviewPopupDlg->SetPatientID(GetActivePatientID(), aryEMNIDs);

			// (a.walling 2010-01-11 12:37) - PLID 36837 - Support different 'subsections' for positioning preferences
			m_pEMRPreviewPopupDlg->RestoreSize(""); // default subsection for window position

			m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
		}
	} NxCatchAll("Error creating EMR Preview window!");
}

void CPatientNexEMRDlg::HidePreviewWindow()
{
	// (a.walling 2007-06-04 14:32) - PLID 25648
	try {
		if (m_bShowPreview) {
			if (m_pEMRPreviewPopupDlg) {
				m_pEMRPreviewPopupDlg->ShowWindow(SW_HIDE);
				m_bShowPreview = FALSE;
				EnsureShowPreviewButton();
			}
		}
	} NxCatchAll("Error in HidePreviewWindow");
}

// (a.walling 2007-04-13 15:53) - PLID 25648
void CPatientNexEMRDlg::EnsureShowPreviewButton()
{
	try {
		GetDlgItem(IDC_NEXEMR_SHOW_PREVIEW)->SetWindowText(m_bShowPreview ? "Hide Preview" : "Show Preview");
	} NxCatchAll("Error in CPatientNexEMRDlg::EnsureShowPreviewButton");
}

// (a.walling 2007-04-13 12:59) - PLID 25648
// throws errors to caller
void CPatientNexEMRDlg::PreviewSelectedEMN()
{
	if (m_bShowPreview) {
		if (m_pEMRPreviewPopupDlg) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->GetCurSel();

			long nEmnID = -1;
			if (pRow) {
				nEmnID = VarLong(pRow->GetValue(eecEMNID), -1);
			} else {				
				// (a.walling 2010-08-05 15:45) - PLID 40015 - No cursel? Don't mess with it. If the patient changes it will be refreshed anyway.
				return;
			}

			// (j.jones 2009-09-22 11:55) - PLID 31620 - PreviewEMN now takes in an array
			// of all available EMN IDs, and the index of the array representing the EMN
			// we wish to view
			// (z.manning 2012-09-10 14:53) - PLID 52543 - Use the new EmnPreviewPopup struct
			CArray<EmnPreviewPopup, EmnPreviewPopup&> aryEMNs;
			long nIndex = -1;

			//find all EMNIDs in the list
			if (m_bEMNTabView)
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->GetFirstRow();
				while(pRow) {
					long nAddEMNID = VarLong(pRow->GetValue(eecEMNID), -1);
					if(nAddEMNID != -1) {
						aryEMNs.Add(EmnPreviewPopup(nAddEMNID, VarDateTime(pRow->GetValue(eecModifiedDate))));
						//if this is the EMN we want to display, set that as the index
						if(nAddEMNID == nEmnID) {
							nIndex = aryEMNs.GetSize() - 1;
						}
					}

					pRow = pRow->GetNextRow();
				}
			}
			else {
				
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->GetFirstRow();
				while(pRow) {

					NXDATALIST2Lib::IRowSettingsPtr pChild = pRow->GetFirstChildRow();
					while(pChild) {
						
						long nAddEMNID = VarLong(pChild->GetValue(eecEMNID), -1);
						if(nAddEMNID != -1) {
							aryEMNs.Add(EmnPreviewPopup(nAddEMNID, VarDateTime(pChild->GetValue(eecModifiedDate))));
							//if this is the EMN we want to display, set that as the index
							if(nAddEMNID == nEmnID) {
								nIndex = aryEMNs.GetSize() - 1;
							}
						}

						pChild = pChild->GetNextRow();
					}

					pRow = pRow->GetNextRow();
				}
			}
			
			m_pEMRPreviewPopupDlg->PreviewEMN(aryEMNs, nIndex);

			// (a.walling 2010-01-11 16:20) - PLID 27733 - Only show if it is not already
			if (!m_pEMRPreviewPopupDlg->IsWindowVisible()) {
				m_pEMRPreviewPopupDlg->ShowWindow(SW_SHOWNA);
			}
		}
	}
}

// (a.walling 2007-04-13 13:12) - PLID 25648
void CPatientNexEMRDlg::SetShowPreview(BOOL bShow)
{
	// the preview popup will call this function to tell us that it is now hidden
	m_bShowPreview = bShow;
	EnsureShowPreviewButton();
}

// (a.walling 2007-04-13 13:13) - PLID 25648 - Hide popups when moving among tabs
void CPatientNexEMRDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);

	long nShowCmd = bShow ? SW_SHOWNA : SW_HIDE;
	
	// we realy need to hide/show any of our modeless popup dialogs
	// (z.manning 2009-08-19 15:19) - PLID 31751 - Actually, no we don't. EMR teams says this should
	// be visible until the user closes it.
	/*if ((m_pEMRPreviewPopupDlg != NULL) && m_bShowPreview) {
		m_pEMRPreviewPopupDlg->ShowWindow(nShowCmd);
		m_bShowPreview = nShowCmd != SW_HIDE;
	}*/

	// (a.walling 2007-09-26 12:20) - PLID 25648
	EnsureShowPreviewButton();

	// (a.walling 2008-10-10 09:55) - PLID 31651 - This used to show the reminder constantly, even though
	// it may not have been appropriate if no EMNs are unlocked. The correct thing to do is handle it in
	// CheckUnlockedAgedEMNs(). The real reason this was here was to simply hide the dialog when leaving
	// the NexEMR tab. It will come back when the view is updated, if necessary.
	if (!bShow && IsWindow(m_pEMRLockReminderDlg.GetSafeHwnd()))
		m_pEMRLockReminderDlg.ShowWindow(nShowCmd);
}

// (a.walling 2007-04-23 10:28) - PLID 24535 - Add an icon to this row
LRESULT CPatientNexEMRDlg::OnEMRListWordIcon(WPARAM wParam, LPARAM lParam)
{
	// wParam = ID of the EMR or EMN
	// lParam = eotType of the ID (EMR or EMN)

	try {
		// (a.walling 2010-06-30 14:02) - PLID 29974 - type is now in the low byte of the low word, icon types are in the high byte of the low word.
		EMRObjectType eotType = (EMRObjectType)LOBYTE(LOWORD(lParam));
		BYTE nIconTypes = HIBYTE(LOWORD(lParam));

		EMRColumns eecCol;

		if (eotType == eotEMN) {
			eecCol = eecEMNID;
		} else if (eotType == eotEMR) {
			eecCol = eecEMRID;
		} else if (eotType == eotHistory) {
			return -1;
		} else {
			// invalid
			ASSERT(FALSE);
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->FindByColumn(eecCol, _variant_t((long)wParam, VT_I4), NULL, VARIANT_FALSE);

		if (pRow) {
			// (a.walling 2010-06-30 14:02) - PLID 29974 - Layer the appropriate icons.
			CString strIcon;
			if (nIconTypes & eWordIcon) {
				strIcon += "BITMAP:MSWORD+";
			}
			if (nIconTypes & eAudioIcon) {
				strIcon += "BITMAP:AUDIO+";
			}
			strIcon.TrimRight("+");

			pRow->PutValue(eecWordIcon, _bstr_t((LPCTSTR)strIcon));
		} // if we don't find it, no problem, must be an old message.

		return 0;
	} NxCatchAll("Error in OnEMRListWordIcon");

	return -1;
}

// (a.walling 2007-04-23 10:28) - PLID 24535 - Colour the appropriate row
LRESULT CPatientNexEMRDlg::OnEMRListColor(WPARAM wParam, LPARAM lParam)
{
	// wParam = ID of the EMN
	// lParam = Color of the EMN (we don't colour EMR rows)

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->FindByColumn(eecEMNID, _variant_t((long)wParam, VT_I4), NULL, VARIANT_FALSE);

		if (pRow) {
			pRow->PutBackColor(lParam);
		} // if we don't find it, no problem, must be an old message.

		return 1;
	} NxCatchAll("Error in OnEMRListColor");

	return -1;
}

// (j.jones 2007-06-14 10:34) - PLID 26276 - added Completion Status
LRESULT CPatientNexEMRDlg::OnEMRListCompletionStatus(WPARAM wParam, LPARAM lParam)
{
	// wParam = ID of the EMN
	// lParam = EMNCompletionStatus for the given EMN

	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->FindByColumn(eecEMNID, _variant_t((long)wParam, VT_I4), NULL, VARIANT_FALSE);

		if (pRow) {
			pRow->PutValue(eecCompletionStatus, (long)lParam);
		} // if we don't find it, no problem, must be an old message.

		return 1;
	} NxCatchAll("Error in OnEMRListCompletionStatus");

	return -1;
}

// (a.walling 2007-04-23 11:11) - PLID 24535 - prepare the thread
CFinishEMRListThread::CFinishEMRListThread(HWND hwnd, AFX_THREADPROC pfnThreadProc)
	: CWinThread (pfnThreadProc, NULL) // See thrdcore.cpp. Pass in the worker function address
							// (Otherwise, the WinThread will try to create a message pump for us!)
{
	m_bAutoDelete = TRUE;  // will 'delete this' when the ThreadProc has returned

	m_hStopThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // create our sync event to signal the thread proc to stop

	m_hwndNotify = hwnd; // the window to notify with messages
	
	// See thrdcore.cpp. Set a reference to our thread object as the thread's parameter
	m_pThreadParams = this;

	// (a.walling 2010-11-01 12:38) - PLID 40965
	m_nPatientID = -1;
}

// (a.walling 2007-04-23 11:11) - PLID 24535 - destruct the thread
CFinishEMRListThread::~CFinishEMRListThread()
{
	CloseHandle(m_hStopThreadEvent); // close our sync event

	// destroy!
}

// (a.walling 2007-04-23 11:50) - PLID 24535 - Loop through all our EMR Objects, and post a message telling to add a word icon or colourize.
UINT CFinishEMRListThread::FinishEMRListThreadFunction(LPVOID p) // static
{
	CFinishEMRListThread* pThread = (CFinishEMRListThread*)p;

	if (pThread == NULL) {
		ASSERT(FALSE);
		return 0;
	}

	try {
		// first set up our data connection
		//ADODB::_ConnectionPtr pCon(__uuidof(Connection));
		ADODB::_ConnectionPtr pCon;
		int i;

		// Open an new connection based on the existing one
		{
			// (a.walling 2010-07-23 17:11) - PLID 39835 - Use GetThreadRemoteData to get a new connection using default values within a thread
			pCon = GetThreadRemoteData();
		}

		// set the word icons

		// (c.haag 2007-06-15 17:07) - PLID 26356 - Rather than querying in the loop here,
		// build lists of EMN's and EMR's to use in a unified query below to set the word
		// icons. We don't check for the signaled state here because this loop should be
		// lightning fast.
		//
		// The code has also been moved above the coloring code because the view appears 
		// to load faster than otherwise

		CArray<long, long> anEMNIDs;
		CArray<long, long> anEMRIDs;
		for (i = 0; i < pThread->m_arListObjects.GetSize(); i++) {
			EMRListObject eloObject = pThread->m_arListObjects[i];

			if (eloObject.eotType == eotEMN) {
				// object is an emn
				anEMNIDs.Add(eloObject.nID);

			}
			else if (eloObject.eotType == eotEMR) {
				// object is an emr
				anEMRIDs.Add(eloObject.nID);

			}
			else if(eloObject.eotType == eotHistory) {
			}
			else {
				// invalid
				ASSERT(FALSE);
			}
		}

		// (z.manning 2010-07-28 15:47) - PLID 39874 - We now handle the problem flag here rather than the inefficient
		// subquery we used to have in the main query to load the existing list. First off, run queries to load all
		// EMR and EMN problems for this patient. Per pre-existing behavior, we only show the problem flag for EMR rows
		// if it has problems with EMR regarding type.
		// (a.walling 2014-07-23 09:12) - PLID 63003 - Use CONST_INT for EMRProblemLinkT.EMRRegardingType enums
		_RecordsetPtr prsProblems = CreateParamRecordset(pCon, FormatString(
			"SELECT EmrID, CAST(MIN(CAST(Resolved AS int)) AS bit) AS Resolved \r\n"
			"FROM %s \r\n"
			"WHERE Deleted = 0 AND PatientID = {INT} AND EmrRegardingType = {CONST_INT} AND EmrID IS NOT NULL \r\n"
			"GROUP BY EmrID \r\n"
			"\r\n"
			"SELECT EmnID, CAST(MIN(CAST(Resolved AS int)) AS bit) AS Resolved \r\n"
			"FROM %s \r\n"
			"WHERE Deleted = 0 AND PatientID = {INT} AND EmnID IS NOT NULL \r\n"
			"GROUP BY EmnID \r\n"
			, GetEmrProblemListFromClause(), GetEmrProblemListFromClause())
			, GetActivePatientID(), eprtEmrEMR, GetActivePatientID());
		LPARAM lParam;
		// (z.manning 2010-07-28 15:49) - PLID 39874 - Go through all EMR problems and post a message to the main
		// thread for which icon to use for the problem status for that EMR.
		for(; !prsProblems->eof; prsProblems->MoveNext())
		{
			const BOOL bResolved = AdoFldBool(prsProblems->GetFields(), "Resolved");
			const long nEmrID = AdoFldLong(prsProblems->GetFields(), "EmrID");
			// (z.manning 2010-07-28 15:51) - PLID 39874 - The LOBYTE of the LOWORD of the lparam tells the main
			// thread if this is an EMN row (TRUE) or EMR row (FALSE).
			lParam = MAKELPARAM(MAKEWORD(FALSE, bResolved), 0);
			::PostMessage(pThread->m_hwndNotify, NXM_NEXEMR_EMRLIST_PROBLEM_ICON, nEmrID, lParam);
		}
		prsProblems = prsProblems->NextRecordset(NULL);
		// (z.manning 2010-07-28 15:49) - PLID 39874 - Go through all EMN problems and post a message to the main
		// thread for which icon to use for the problem status for that EMN.
		for(; !prsProblems->eof; prsProblems->MoveNext())
		{
			const BOOL bResolved = AdoFldBool(prsProblems->GetFields(), "Resolved");
			const long nEmnID = AdoFldLong(prsProblems->GetFields(), "EmnID");
			// (z.manning 2010-07-28 15:51) - PLID 39874 - The LOBYTE of the LOWORD of the lparam tells the main
			// thread if this is an EMN row (TRUE) or EMR row (FALSE).
			lParam = MAKELPARAM(MAKEWORD(TRUE, bResolved), 0);
			::PostMessage(pThread->m_hwndNotify, NXM_NEXEMR_EMRLIST_PROBLEM_ICON, nEmnID, lParam);
		}
		prsProblems->Close();

		// (c.haag 2007-06-15 17:20) - PLID 26356 - Now that we know the ID's of all the
		// EMN's and EMR's to add, build a unified query that tells us which ones are assoicated
		// with Word documents.

		// (a.walling 2010-06-30 14:02) - PLID 29974 - Audio cannot be attached to an EMN, just an EMR/PIC, but still group to prevent duplicate messages
		// (a.walling 2010-07-27 16:55) - PLID 39433 - Can be attached to an EMN as well now, too.
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		CParamSqlBatch batch;
		if (!anEMNIDs.IsEmpty()) {
			batch.Add(FormatString("SELECT %d AS Type, EmnID AS ID, "
				"SUM(CASE WHEN Selection = 'BITMAP:MSWORD' THEN 1 ELSE 0 END) AS WordAttachments, "
				"SUM(CASE WHEN Selection = 'BITMAP:AUDIO' THEN 1 ELSE 0 END) AS AudioAttachments "
				"FROM MailSent WHERE Selection IN ('BITMAP:MSWORD', 'BITMAP:AUDIO') AND EmnID IS NOT NULL AND MailSent.PersonID = {INT} "
				"GROUP BY EmnID"
				, eotEMN), pThread->m_nPatientID);
		}

		// (a.walling 2010-06-30 14:02) - PLID 29974 - Include count of word and audio attachments and group to prevent duplicate messages
		// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
		if (!anEMRIDs.IsEmpty()) {
			batch.Add(FormatString("SELECT %d AS Type, PicT.EmrGroupID AS ID, "
				"SUM(CASE WHEN Selection = 'BITMAP:MSWORD' THEN 1 ELSE 0 END) AS WordAttachments, "
				"SUM(CASE WHEN Selection = 'BITMAP:AUDIO' THEN 1 ELSE 0 END) AS AudioAttachments "
				"FROM PicT "
				"INNER JOIN MailSent ON MailSent.PicID = PicT.ID "
				"WHERE Selection IN ('BITMAP:MSWORD', 'BITMAP:AUDIO') AND PicT.EmrGroupID IS NOT NULL AND MailSent.PersonID = {INT} "
				"GROUP BY PicT.EmrGroupID"
				, eotEMR), pThread->m_nPatientID);
		}

		if (!batch.IsEmpty()) {
			// ((c.haag 2007-06-15 17:26) - PLID 26356 - If we have a valid query, then run it and assign the icons

			// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
			batch.Prepend("SET NOCOUNT ON");
			batch.Declare("SET NOCOUNT OFF");

			_RecordsetPtr prsWord = batch.CreateRecordset(pCon);

			while (prsWord) {
				FieldsPtr f = prsWord->Fields;
				while (!prsWord->eof) {
					
					// check for signalled state
					if (WAIT_TIMEOUT != WaitForSingleObject(pThread->m_hStopThreadEvent, 0)) {
						// we have been signalled to exit!
						::PostMessage(pThread->m_hwndNotify, NXM_NEXEMR_EMRLIST_FINISHED, 0, 0);

						// connection will be freed on exit

						return 0;
					}
					else {
						// send a message
						// (a.walling 2010-06-30 14:02) - PLID 29974
						long nID = AdoFldLong(f, "ID");
						long nType = AdoFldLong(f, "Type");

						bool bExpected = false;
						if (nType == eotEMN) {
							for (int i = 0; i < anEMNIDs.GetSize(); i++) {
								if (anEMNIDs[i] == nID) {
									bExpected = true;
									break;
								}
							}
						} else if (nType == eotEMR) {
							for (int i = 0; i < anEMRIDs.GetSize(); i++) {
								if (anEMRIDs[i] == nID) {
									bExpected = true;
									break;
								}
							}
						}

						if (bExpected) {
							long nWordAttachments = AdoFldLong(f, "WordAttachments", 0);
							long nAudioAttachments = AdoFldLong(f, "AudioAttachments", 0);

							BYTE nIconTypes = 0;
							if (nWordAttachments) {
								nIconTypes |= eWordIcon;
							}
							if (nAudioAttachments) {
								nIconTypes |= eAudioIcon;
							}
							LPARAM lParam = MAKELPARAM(MAKEWORD((BYTE)nType, (BYTE)nIconTypes), 0);
							// type is now in the low byte of the low word, icon types are in the high byte of the low word.
							::PostMessage(pThread->m_hwndNotify, NXM_NEXEMR_EMRLIST_WORDICON, (WPARAM)nID, (LPARAM)lParam);
						}
					}

					prsWord->MoveNext();
				}
				prsWord = prsWord->NextRecordset(NULL);
			}

		}

		// (j.jones 2007-06-19 10:16) - PLID 26276 - this is now obsolete
		/*
		// set the background colours.
		for (i = 0; i < pThread->m_arListObjects.GetSize(); i++) {
			EMRListObject eloObject = pThread->m_arListObjects[i];

			if (eloObject.eotType == eotEMN) {

				// (j.jones 2007-06-14 10:09) - PLID 26276 - We now store the CompletionStatus
				// in data, and it is currently in the datalist. However, it defaults to NULL
				// in the data structure, and so if it is NULL and not locked, we need to calculate the color
				// (the ecsCompletionStatus should be ecsLocked if the EMN is locked)
				EMNCompletionStatus ecsStatus = eloObject.ecsCompletionStatus;

				if(ecsStatus == ecsNeedsCalculated) {

					// (j.jones 2007-06-18 16:26) - PLID 26276 - This logic has been changed
					// such that the CompletionStatus should NEVER be NULL, so if we get here,
					// somehow code has failed somewhere such that we get in this state.
					// This code will correct the data, but in theory it should never be called.
					ASSERT(FALSE);

					//we have no status, we need to calculate it
					ecsStatus = CalculateEMNBackgroundStatusFromID(eloObject.nID, pCon);

					//need to update the list accordingly
					::PostMessage(pThread->m_hwndNotify, NXM_NEXEMR_EMRLIST_COMPLETION_STATUS, eloObject.nID, ecsStatus);
				}
				
				//now calculate the color
				COLORREF cColor = CalculateEMNBackgroundColorFromStatus(ecsStatus, FALSE);

				// send a message
				::PostMessage(pThread->m_hwndNotify, NXM_NEXEMR_EMRLIST_COLOR, eloObject.nID, cColor);

				// check for signalled state
				if (WAIT_TIMEOUT != WaitForSingleObject(pThread->m_hStopThreadEvent, 0)) {
					// we have been signalled to exit!
					::PostMessage(pThread->m_hwndNotify, NXM_NEXEMR_EMRLIST_FINISHED, 0, 0);

					// connection will be freed on exit

					return 0;
				}
			}
		}
		*/
		
		::PostMessage(pThread->m_hwndNotify, NXM_NEXEMR_EMRLIST_FINISHED, 0, 0);

	} NxCatchAllThread("Error in FinishEMRListThread");

	return 0;

	/*
	CPatientNexEMRDlg* pDlg = (CPatientNexEMRDlg*)p;

	try {

		//if the dialog has been destroyed, quietly return
		if(!pDlg->GetSafeHwnd() || pDlg->m_bCancelThread) {
			return 0;
		}



		//if the dialog has been destroyed, quietly return
		if(!pDlg->GetSafeHwnd() || pDlg->m_bCancelThread) {
			return 0;
		}

		//we need to show the date range for each EMR, and colorize each EMN

		//Start looping at the first row
		



		//if the dialog has been destroyed, quietly return
		if(!pDlg->GetSafeHwnd() || pDlg->m_bCancelThread) {
			return 0;
		}

		// (a.wetta 2007-02-21 14:03) - PLID 24405 - We no longer want to auto size the column to the data because it causes the columns
		// to move around when switching between tab and non-tab view, which can be annoying.  Now there is just a set width for the column.
		//pDlg->m_pExistingList->CalcColumnWidthFromData(eecDate, TRUE, TRUE);

		// (z.manning, 03/27/2007) - PLID 25363 - We now determine if an EMN has assocatiated Word documents
		// here instead of in the datalist's query to speed up the apparent loading speed of the existing list.
		// This is faster than the status coloring, so let's do it before that.
		pCurRow = pDlg->m_pExistingList->FindAbsoluteFirstRow(VARIANT_FALSE);
		while(pCurRow) 
		{
			if(pDlg->GetSafeHwnd() && !pDlg->m_bCancelThread && pDlg->m_pFinishEMRListConn != NULL) {
				long nEmnID = VarLong(pCurRow->GetValue(eecEMNID), -1);
				_RecordsetPtr prsHasWordDocuments;
				if(nEmnID >= 0) {
					prsHasWordDocuments = CreateRecordset(pDlg->m_pFinishEMRListConn,
						"SELECT CASE WHEN EXISTS  \r\n"
						"(  \r\n"
						"	SELECT MailSent.MailID  \r\n"
						"	FROM MailSent  \r\n"
						"	INNER JOIN EmrMasterT ON MailSent.EmnID = EmrMasterT.ID  \r\n"
						"	WHERE Selection = 'BITMAP:MSWORD' AND EmrMasterT.ID = %li  \r\n"
						")  \r\n"
						"THEN 'BITMAP:MSWORD' ELSE '' END AS HasWordDocuments  \r\n"
						, nEmnID );
				}
				else {
					prsHasWordDocuments = CreateRecordset(pDlg->m_pFinishEMRListConn,
						"SELECT CASE WHEN EXISTS  \r\n"
						"(  \r\n"
						"	SELECT MailSent.MailID  \r\n"
						"	FROM MailSent  \r\n"
						"	INNER JOIN PicT ON MailSent.PicID = PicT.ID  \r\n"
						"	INNER JOIN EmrGroupsT ON PicT.EmrGroupID = EmrGroupsT.ID  \r\n"
						"	WHERE Selection = 'BITMAP:MSWORD' AND EmrGroupsT.ID = %li  \r\n"
						")  \r\n"
						"THEN 'BITMAP:MSWORD' ELSE '' END AS HasWordDocuments  \r\n"
						, VarLong(pCurRow->GetValue(eecEMRID),-1) );
				}

				if(pDlg->GetSafeHwnd() && !pDlg->m_bCancelThread) {
					if(prsHasWordDocuments != NULL && !prsHasWordDocuments->eof) {
						pCurRow->PutValue(eecWordIcon, prsHasWordDocuments->Fields->GetItem("HasWordDocuments")->Value);
					}

					pCurRow = pDlg->m_pExistingList->FindAbsoluteNextRow(pCurRow, VARIANT_FALSE);
				}
				else {
					return 0;
				}
			}
			else {
				return 0;
			}
		}

		//now loop again to color the EMNs, which is slower
		pCurRow = pDlg->m_pExistingList->GetFirstRow();
		while(pCurRow) {

			//colorize each EMN
			// (a.wetta 2007-01-09 16:36) - PLID 14635 - If the user is using the tabbed view, then there are no children rows
			if (pDlg->IsEMNTabView()) {
				if(pDlg->GetSafeHwnd() && !pDlg->m_bCancelThread) {
					if(pDlg->m_pFinishEMRListConn) {
						pCurRow->PutBackColor(CalculateEMNBackgroundColor(VarLong(pCurRow->GetValue(eecEMNID), -1), pDlg->m_pFinishEMRListConn));
					}
					else {
						return 0;
					}
				}
				else {
					return 0;
				}
			}
			else {
				NXDATALIST2Lib::IRowSettingsPtr pCurChildRow = pCurRow->GetFirstChildRow();
				while(pCurChildRow) {

					// (j.jones 2006-06-01 11:02) - if we're trying to close the thread but it hasn't
					// stopped yet, the objects we are trying to access may go out of scope.
					// If so, detect that state and quietly exit.
					if(pDlg->GetSafeHwnd() && !pDlg->m_bCancelThread) {
						if(pDlg->m_pFinishEMRListConn) {
							pCurChildRow->PutBackColor(CalculateEMNBackgroundColor(VarLong(pCurChildRow->GetValue(eecEMNID), -1), pDlg->m_pFinishEMRListConn));
						}
						else {
							return 0;
						}
					}
					else {
						return 0;
					}

					pCurChildRow = pCurChildRow->GetNextRow();
				}		
			}

			//move to the next row
			pCurRow = pCurRow->GetNextRow();
		}

	}NxCatchAllThread("FinishEMRListThread (PatientNexEMRDlg)");

	if(pDlg->m_pFinishEMRListConn)
		pDlg->m_pFinishEMRListConn.Release();

	// Now tell the window that we are done manipulating the list
	if (pDlg->GetSafeHwnd())
		pDlg->PostMessage(NXM_NEXEMR_EMRLIST_FINISHED);
	return 0;
	*/
}

// (z.manning, 05/22/2007) - PLID 25489 - FALSE if there are multiple selections in the existing list and
// they do not all have the same chart ID, TRUE otherwise.
BOOL CPatientNexEMRDlg::DoAllSelectedRowsHaveSameChartID()
{
	long nChartID;
	// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
	NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_pExistingList->GetFirstSelRow();
	if (pCurSelRow) {
		NXDATALIST2Lib::IRowSettingsPtr pNextRow = pCurSelRow;
		pCurSelRow = pCurSelRow->GetNextSelRow();

		nChartID = VarLong(pNextRow->GetValue(eecTabChartID), -1);

		while (pCurSelRow) {
			NXDATALIST2Lib::IRowSettingsPtr pNextRow = pCurSelRow;
			pCurSelRow = pCurSelRow->GetNextSelRow();

			if( nChartID !=  VarLong(pNextRow->GetValue(eecTabChartID),-1) ) {
				return FALSE;
			}
		}
	}

	return TRUE;
}

// (j.jones 2007-06-11 10:32) - PLID 26271 - store the list of relevant templates
void CPatientNexEMRDlg::BuildCurEMRTemplateInfo(long nEMRID, BOOL bForceClear)
{
	try {

		//The left-click, right-click, and context menu all need the list
		//of relevant templates that may be added as a new EMN to a given EMR,
		//but when you do any of these actions, they are routinely for the same EMR.
		//So load it up once for a given EMR, and don't reload unless we select
		//a different EMR.

		BOOL bCleared = FALSE;

		//if asked, clear the current info
		if(bForceClear) {
			ClearCurEMRTemplateInfo();
			bCleared = TRUE;
		}

		//if we're called on -1, just leave
		if(nEMRID == -1) {
			return;
		}

		//now assign our current template ID
		m_nCurEMRSelected = nEMRID;

		//migrated from ReFilterButtons

		//0) TES 5/28/2008 - PLID 30169 - Show any templates which are flagged as "universal."  These take precedence over
		// anything else.
		_RecordsetPtr prsTemplates = CreateRecordset("SELECT EMRTemplateT.ID AS TemplateID, EMRTemplateT.Name AS TemplateName,  "
			"EMRTemplateT.AddOnce, EMRCollectionT.ID AS CollectionID, EMRCollectionT.Name AS CollectionName "
			"FROM "
			"EMRTemplateT LEFT JOIN EMRCollectionT ON EMRTemplateT.CollectionID = EMRCollectionT.ID "
			"WHERE "
			"EmrTemplateT.Deleted = 0 AND EmrCollectionT.Inactive = 0 AND  "
			"EmrTemplateT.IsUniversal = 1 "
			"AND (EmrTemplateT.AddOnce = 0 OR EmrTemplateT.ID NOT IN (SELECT COALESCE(TemplateID,-1) FROM EmrMasterT WHERE Deleted = 0 AND EmrGroupID = %li)) "
			"ORDER BY MenuOrder, EMRTemplateT.Name, EMRCollectionT.Name", m_nCurEMRSelected);
		while(!prsTemplates->eof) {
			
			//before we add anything, if we didn't clear the list, do it now!
			if(!bCleared) {
				ClearCurEMRTemplateInfo();
				bCleared = TRUE;
				m_nCurEMRSelected = nEMRID;
			}

			long nTemplateID = AdoFldLong(prsTemplates, "TemplateID");
			// (z.manning 2010-04-13 11:06) - PLID 38174 - EmrTemplateT.Name is nullable
			CString strTemplateName = AdoFldString(prsTemplates, "TemplateName", "");

			long nCollectionID = AdoFldLong(prsTemplates, "CollectionID");
			CString strCollectionName = AdoFldString(prsTemplates, "CollectionName");

			//add to our array
			EMRTemplateInfo etInfo;
			etInfo.nID = nTemplateID;
			etInfo.strName = strTemplateName;
			m_arCurEMRTemplateInfo.Add(etInfo);

			prsTemplates->MoveNext();
		}

		//1)  If our EMR has procedures to it, filter appropriately on that procedure.
		BOOL bAtLeastOne = FALSE;
		prsTemplates = CreateRecordset("SELECT EMRTemplateT.ID AS TemplateID, EMRTemplateT.Name AS TemplateName,  "
			"EMRTemplateT.AddOnce, EMRCollectionT.ID AS CollectionID, EMRCollectionT.Name AS CollectionName "
			"FROM "
			"EMRTemplateT LEFT JOIN EMRCollectionT ON EMRTemplateT.CollectionID = EMRCollectionT.ID "
			"WHERE "
			"EmrTemplateT.Deleted = 0 AND EmrCollectionT.Inactive = 0 AND  "
			//TES 5/28/2008 - PLID 30169 - Filter out any "universal" templates, they would have already been added to our array.
			"EmrTemplateT.IsUniversal = 0 AND "
			"EmrTemplateT.ID IN (SELECT TemplateID FROM EMRTemplateTopicsT)  "
			"AND EmrTemplateT.ID IN (SELECT EMRTemplateID FROM EmrTemplateProceduresT  "
			"WHERE ProcedureID IN (SELECT ProcedureID FROM ProcInfoDetailsT WHERE ProcInfoID IN (SELECT ProcInfoID FROM PicT WHERE EmrGroupID = %li))  "
			"OR ProcedureID IN (SELECT ProcedureID FROM EMRProcedureT WHERE Deleted = 0 AND EMRID IN (SELECT ID FROM EMRMasterT WHERE Deleted = 0 AND EmrGroupID = %li))  "
			") AND (EmrTemplateT.AddOnce = 0 OR EmrTemplateT.ID NOT IN (SELECT COALESCE(TemplateID,-1) FROM EmrMasterT WHERE Deleted = 0 AND EmrGroupID = %li)) "
			"ORDER BY MenuOrder, EMRTemplateT.Name, EMRCollectionT.Name", m_nCurEMRSelected, m_nCurEMRSelected, m_nCurEMRSelected);

		while(!prsTemplates->eof) {
			
			//before we add anything, if we didn't clear the list, do it now!
			if(!bCleared) {
				ClearCurEMRTemplateInfo();
				bCleared = TRUE;
				m_nCurEMRSelected = nEMRID;
			}

			long nTemplateID = AdoFldLong(prsTemplates, "TemplateID");
			// (z.manning 2010-04-13 11:06) - PLID 38174 - EmrTemplateT.Name is nullable
			CString strTemplateName = AdoFldString(prsTemplates, "TemplateName", "");

			long nCollectionID = AdoFldLong(prsTemplates, "CollectionID");
			CString strCollectionName = AdoFldString(prsTemplates, "CollectionName");

			//add to our array
			EMRTemplateInfo etInfo;
			etInfo.nID = nTemplateID;
			etInfo.strName = strTemplateName;
			m_arCurEMRTemplateInfo.Add(etInfo);

			bAtLeastOne = TRUE;

			//we're loading a specific list of procedures, not a general collection
			m_bCurTemplateInfoIsGeneric = FALSE;

			prsTemplates->MoveNext();
		}

		//2)  If we got nothing above, we'll just load the default list of templates
		// (j.jones 2007-06-11 11:31) - PLID 26271 - Do not load if we already have a "generic" list,
		// you will get the same results, unless the templates changed in which case other code should
		// have already cleared the list.
		if(!bAtLeastOne && !(m_bCurTemplateInfoIsGeneric && m_arCurEMRTemplateInfo.GetSize() > 0)) {
			prsTemplates = CreateRecordset("SELECT EMRTemplateT.ID AS TemplateID, EMRTemplateT.Name AS TemplateName,  "
				"EMRTemplateT.AddOnce, EMRCollectionT.ID AS CollectionID, EMRCollectionT.Name AS CollectionName "
				"FROM "
				"EMRTemplateT LEFT JOIN EMRCollectionT ON EMRTemplateT.CollectionID = EMRCollectionT.ID "
				"WHERE "
				"EmrTemplateT.Deleted = 0 AND EmrTemplateT.ID IN (SELECT TemplateID FROM EMRTemplateTopicsT)  "
				//TES 5/28/2008 - PLID 30169 - Filter out any "universal" templates, they would have already been added to our array.
				"AND EmrTemplateT.IsUniversal = 0 AND "
				"CollectionID IN  "
				"(SELECT TOP 1 EMRCollectionT.ID FROM EMRCollectionT INNER JOIN EMRTemplateT ON EMRCollectionT.ID = EMRTemplateT.CollectionID  "
				"INNER JOIN EMRTemplateTopicsT ON EMRTemplateT.ID = EMRTemplateTopicsT.TemplateID WHERE EmrCollectionT.Inactive = 0 ORDER BY MenuOrder) "
				"ORDER BY MenuOrder, EMRTemplateT.Name, EMRCollectionT.Name");

			while(!prsTemplates->eof) {

				//before we add anything, if we didn't clear the list, do it now!
				if(!bCleared) {
					ClearCurEMRTemplateInfo();
					bCleared = TRUE;
					m_nCurEMRSelected = nEMRID;
				}

				long nTemplateID = AdoFldLong(prsTemplates, "TemplateID");
				// (z.manning 2010-04-13 11:06) - PLID 38174 - EmrTemplateT.Name is nullable
				CString strTemplateName = AdoFldString(prsTemplates, "TemplateName", "");

				long nCollectionID = AdoFldLong(prsTemplates, "CollectionID");
				CString strCollectionName = AdoFldString(prsTemplates, "CollectionName");

				//add to our array
				EMRTemplateInfo etInfo;
				etInfo.nID = nTemplateID;
				etInfo.strName = strTemplateName;
				m_arCurEMRTemplateInfo.Add(etInfo);

				//we're loading a general collection, not a specific list of procedures
				m_bCurTemplateInfoIsGeneric = TRUE;

				prsTemplates->MoveNext();
			}
		}

	}NxCatchAll("Error in CPatientNexEMRDlg::BuildCurEMRTemplateInfo");
}

// (j.jones 2007-06-11 10:32) - PLID 26271 - clear the list of relevant templates
void CPatientNexEMRDlg::ClearCurEMRTemplateInfo()
{
	try {

		//first clear the ID
		m_nCurEMRSelected = -1;

		//now empty the array
		m_arCurEMRTemplateInfo.RemoveAll();

		//revert our flag
		m_bCurTemplateInfoIsGeneric = FALSE;
		
	}NxCatchAll("Error in CPatientNexEMRDlg::ClearCurEMRTemplateInfo");
}

void CPatientNexEMRDlg::OnRButtonUpNewList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if(lpRow) {
			// (a.wetta 2007-06-18 09:34) - PLID 26235 - Handle the context menu for the new EMR list. The context menu will give the user
			// the option to add the selected template to an existing active EMR or create a new EMR.
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			//m_pEMNGroupTemplateList->SetSelByColumn(egtfID, pRow->GetValue(egtfID));

			// Get the data from the row
			long nTemplateID = VarLong(pRow->GetValue(necRecordID));
			long nType = VarLong(pRow->GetValue(necType));

			// Only show the pop up menu if the new EMR row is a template row
			if (nType == neltTemplate) {
				// This offset is used when creating the command IDs for the menu items
				const long nIDOffset = -1000;

				CMenu mPopup;
				mPopup.CreatePopupMenu();

				// Get all of the active EMRs for the patient
				bool bRecordSetEmpty = true;
				_RecordsetPtr prsActiveEMRs = CreateRecordset("SELECT ID, ID AS EMRID, PatientID, (SELECT Min(Date) AS Date FROM EMRMasterT WHERE Deleted = 0 AND EMRGroupID = EMRGroupsT.ID) AS Date, "
												"(SELECT Max(Date) AS MaxDate FROM EMRMasterT WHERE Deleted = 0 AND EMRGroupID = EMRGroupsT.ID) AS MaxDate, "
												"InputDate, Description, PicID "
												"FROM (SELECT EMRGroupsT.*, PicT.ID AS PicID FROM EMRGroupsT "
												"	INNER JOIN PicT ON EmrGroupsT.ID = PicT.EmrGroupID "
												"	WHERE PicT.IsCommitted IS NULL OR PicT.IsCommitted = 1) EMRGroupsT "
												"WHERE Deleted = 0 AND Status <> 1 AND PatientID = %li "
												"ORDER BY InputDate DESC, Description ", GetActivePatientID());
				while(!prsActiveEMRs->eof) {
					bRecordSetEmpty = false;
					CString strDescription = AdoFldString(prsActiveEMRs, "Description");
					long nPicID = AdoFldLong(prsActiveEMRs, "PicID");

					// Get the date
					COleDateTime dtInputDate = AdoFldDateTime(prsActiveEMRs, "InputDate");
					COleDateTime dtMinDate = AdoFldDateTime(prsActiveEMRs, "Date", dtInputDate);
					COleDateTime dtMaxDate = AdoFldDateTime(prsActiveEMRs, "MaxDate", dtInputDate);					

					CString strDateRange, strMin, strMax;
					strMin = FormatDateTimeForInterface(dtMinDate, DTF_STRIP_SECONDS, dtoDate, true);
					strMax = FormatDateTimeForInterface(dtMaxDate, DTF_STRIP_SECONDS, dtoDate, true);
					if(strMin == strMax)
						strDateRange = strMin;
					else
						strDateRange = strMin + " - " + strMax;
				
					// Format the menu option
					CString strMenu;
					strMenu.Format("%s (%s)", strDescription, strDateRange);

					// Add as a negative number, so we don't overlap any other in-use IDs
					mPopup.AppendMenu(MF_ENABLED, -nPicID + nIDOffset, ConvertToControlText(strMenu));

					prsActiveEMRs->MoveNext();
				}
				
				if (!bRecordSetEmpty) {
					mPopup.AppendMenu(MF_SEPARATOR, 0);
				}

				mPopup.AppendMenu(MF_ENABLED, nIDOffset + 1, "{New EMR}");

				// Pop up the menu
				CPoint pt;
				GetCursorPos(&pt);
				long nResult = mPopup.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, pt.x, pt.y, this);

				// Handle the result
				if (nResult == nIDOffset + 1) {
					// Create a new EMR
					LaunchPICEditorWithNewEMR(-1, nTemplateID);
				}
				else if (nResult < nIDOffset) {
					// Add the selected template to an existing EMR
					LaunchPICEditorWithNewEMR(-nResult + nIDOffset, nTemplateID);
				}
			}
		}		
	}NxCatchAll("Error in CPatientNexEMRDlg::OnRButtonUpNewList");
}

void CPatientNexEMRDlg::OnSelectTabChartTabs(short newTab, short oldTab) 
{
	// (z.manning, 04/04/2007) - PLID 25489 - We need to refresh the tabs because different charts can
	// be associated with different categories. Then reload the list.
	RefreshEMNTabs();
	ReloadEMRList();
}

void CPatientNexEMRDlg::OnShowHistory()
{
	try
	{
		SetRemotePropertyInt("NexEMRTabShowHistory", m_btnShowHistory.GetCheck(), 0, GetCurrentUserName());
		ReloadEMRList();

	}NxCatchAll("CPatientNexEMRDlg::OnShowHistory");
}

void CPatientNexEMRDlg::OnEditingStartingExistingList(LPDISPATCH lpRow, short nCol, VARIANT FAR* pvarValue, BOOL FAR* pbContinue)
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		// (z.manning 2008-07-03 09:54) - PLID 25574 - No editing anything in history rows
		long nMailID = VarLong(pRow->GetValue(eecMailID), -1);
		if(nMailID != -1) {
			*pbContinue = FALSE;
		}

	}NxCatchAll("CPatientNexEMRDlg::OnEditingStartingExistingList");
}

// (z.manning 2008-07-03 11:12) - PLID 25574 0 Opens a history document for the given row
void CPatientNexEMRDlg::OpenHistoryDocument(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	if(pRow == NULL) {
		return;
	}

	long nMailID = VarLong(pRow->GetValue(eecMailID), -1);
	CString strHistoryFile = VarString(pRow->GetValue(eecHistoryPathName), "");

	
	if (strHistoryFile.Find('\\') == -1) {
		// The "path" doesn't have a backslash, so it's just a filename, which means it should use patient's shared documents path
		strHistoryFile = GetPatientDocumentPath(GetActivePatientID()) ^ strHistoryFile;
	}

	if (nMailID != -1) {
		// (j.jones 2010-06-30 11:12) - PLID 38031 - changed to also support generic XML
		BOOL bIsXMLFileType = NxXMLUtils::IsXMLFileExtension(strHistoryFile);
		if(bIsXMLFileType && NxXMLUtils::IsValidXMLFile(strHistoryFile)) {
			try {
				ViewXMLDocument(strHistoryFile, this);
				return;
			} NxCatchAll("Error preparing XML document for viewing");
		}
	}

	if(!OpenDocument(strHistoryFile, GetActivePatientID())) {
		MsgBox(RCS(IDS_NO_FILE_OPEN));
	}
}

// (j.jones 2008-07-29 11:19) - PLID 30873 - added OnBtnPtSummary
void CPatientNexEMRDlg::OnBtnPtSummary() 
{
	try {

		CPatientSummaryDlg dlg(this);
		dlg.m_nPatientID = GetActivePatientID();
		dlg.m_strPatientName = GetActivePatientName();
		dlg.DoModal();

	}NxCatchAll("Error in CPatientNexEMRDlg::OnPatientSummary");
}

// (j.jones 2008-10-14 16:58) - PLID 14411 - added EMR Analysis
void CPatientNexEMRDlg::OnBtnEMRAnalysis() 
{
	try {

		if(GetMainFrame()) {
			GetMainFrame()->ShowEMRAnalysisDlg();
		}

	}NxCatchAll("Error in CPatientNexEMRDlg::OnBtnEMRAnalysis");
}

// (j.gruber 2009-05-26 15:37) - PLID 34348 - added Wellness
void CPatientNexEMRDlg::OnBtnWellness() 
{
	try {

		CPatientWellnessDlg dlg(GetActivePatientID(), this);
		dlg.DoModal();

	}NxCatchAll("Error in CPatientNexEMRDlg::OnBtnWellness");
}

// (j.gruber 2009-06-04 12:36) - PLID 34484 - updating wellness alerts and button
void CPatientNexEMRDlg::RefreshWellnessAlerts() 
{
	try {

		/****NOTE: A VERY SIMILIAR QUERY IS USED IN PatientWellnessDlg.cpp::LoadAlertList
		THEY ONLY DIFFER IN WHAT THEY RETURN
		SO IF THIS CHANGES, MAKE SURE TO CHECK THAT ONE ALSO******/
		//TES 7/8/2009 - PLID 34534 - Moved the query in question to a #define in WellnessDataUtils.h, so it only
		// needs to be maintained in one place now.
		ADODB::_RecordsetPtr rsAlerts = CreateParamRecordset(GetRemoteData(), "SET NOCOUNT ON;"
			"	 DECLARE @nPatientID INT  "
			" 	 SET @nPatientID = {INT}; "
			/*" 	 SET @nPatientID = 12496; "*/
			"  "
			+ WELLNESS_INSTANTIATION_SQL + 
			" 	/*now select our records that we need to show*/ "
			" 	SET NOCOUNT OFF; "
			" 	 "			
			" 	SELECT ID FROM PatientWellnessT WHERE DELETED = 0 AND PatientID = @nPatientID AND CompletedDate IS NULL "			
			, GetActivePatientID(), GetCurrentUserID()); 

			if (!rsAlerts->eof) {

				//change the button to be red or something
				m_btnWellness.SetTextColor(RGB(255,0,0));
				m_btnWellness.Invalidate();
				
			}
			else {
				m_btnWellness.AutoSet(NXB_MODIFY);
				m_btnWellness.Invalidate();
			}

	}NxCatchAll("Error in CPatientNexEMRDlg::RefreshWellnessAlerts() ");


}

// (z.manning 2009-08-25 09:31) - PLID 31944 - Added a button to show the lock manager dialog
void CPatientNexEMRDlg::OnNexemrLockManager()
{
	try
	{
		if(GetMainFrame() != NULL) {
			GetMainFrame()->ShowLockManager();
		}

	}NxCatchAll(__FUNCTION__);
}


// (a.walling 2009-11-24 14:48) - PLID 36418
void CPatientNexEMRDlg::PrintMultipleEMNs()
{
	try {
		// (b.savon 2011-11-22 11:54) - PLID 25782 - Added PatientID
		CEMRPreviewMultiPopupDlg dlg(GetActivePatientID(), this);

		//find all EMNIDs in the list
		if (m_bEMNTabView) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->GetFirstRow();
			while(pRow) {
				COleDateTime dtInvalid;

				long nAddEMNID = VarLong(pRow->GetValue(eecEMNID), -1);
				// (c.haag 2013-02-28) - PLID 55368 - Added template ID
				long nTemplateID = VarLong(pRow->GetValue(eecTemplateID), -1); 
				CString strEMRTitle = VarString(pRow->GetValue(eecEMRDescription), "");
				CString strEMNTitle = VarString(pRow->GetValue(eecDescription), "");
				COleDateTime dtDate = VarDateTime(pRow->GetValue(eecDate), dtInvalid);
				COleDateTime dtInputDate = VarDateTime(pRow->GetValue(eecInputDate), dtInvalid);
				// (z.manning 2012-09-11 17:50) - PLID 52543 - Need modified date too
				COleDateTime dtModifiedDate = VarDateTime(pRow->GetValue(eecModifiedDate), dtInvalid);

				if(nAddEMNID != -1) {					
					dlg.AddAvailableEMN(NULL, nAddEMNID, nTemplateID, strEMRTitle, strEMNTitle, dtDate, dtInputDate, dtModifiedDate);
				}

				pRow = pRow->GetNextRow();
			}
		}
		else {			
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->GetFirstRow();
			while(pRow) {

				NXDATALIST2Lib::IRowSettingsPtr pChild = pRow->GetFirstChildRow();
				while(pChild) {
					
					COleDateTime dtInvalid;

					long nAddEMNID = VarLong(pChild->GetValue(eecEMNID), -1);
					// (c.haag 2013-02-28) - PLID 55368 - Added template ID
					long nTemplateID = VarLong(pRow->GetValue(eecTemplateID), -1); 
					CString strEMRTitle = VarString(pRow->GetValue(eecEMRDescription), "");
					CString strEMNTitle = VarString(pChild->GetValue(eecDescription), "");
					COleDateTime dtDate = VarDateTime(pChild->GetValue(eecDate), dtInvalid);
					COleDateTime dtInputDate = VarDateTime(pChild->GetValue(eecInputDate), dtInvalid);
					// (z.manning 2012-09-11 17:50) - PLID 52543 - Need modified date too
					COleDateTime dtModifiedDate = VarDateTime(pChild->GetValue(eecModifiedDate), dtInvalid);

					if(nAddEMNID != -1) {
						dlg.AddAvailableEMN(NULL, nAddEMNID, nTemplateID, strEMRTitle, strEMNTitle, dtDate, dtInputDate, dtModifiedDate);
					}

					pChild = pChild->GetNextRow();
				}

				pRow = pRow->GetNextRow();
			}
		}

		dlg.DoModal();
	} NxCatchAll("Error printing multiple EMNs");
}

// (z.manning 2010-07-28 14:52) - PLID 39874
LRESULT CPatientNexEMRDlg::OnEMRListProblemIcon(WPARAM wParam, LPARAM lParam)
{
	try
	{
		// (z.manning 2010-07-28 15:51) - PLID 39874 - The LOBYTE of the LOWORD of the lparam tells the main
		// thread if this is an EMN row (TRUE) or EMR row (FALSE).
		BOOL bIsEmnRow = (BOOL)LOBYTE(LOWORD(lParam));
		BOOL bProblemResolved = (BOOL)HIBYTE(LOWORD(lParam));

		short nCol;
		if(bIsEmnRow) {
			nCol = eecEMNID;
		}
		else {
			if(m_bEMNTabView) {
				return 0;
			}
			nCol = eecEMRID;
		}

		// (z.manning 2010-07-28 15:56) - PLID 39874 - Now find the row the caller passed in and set the
		// appropriate problem icon.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pExistingList->FindByColumn(nCol, (long)wParam, NULL, VARIANT_FALSE);
		if(pRow != NULL)
		{
			if(bProblemResolved) {
				pRow->Value[eecProblemBitmap] = (long)m_hIconHadProblemFlag;
			}
			else {
				pRow->Value[eecProblemBitmap] = (long)m_hIconHasProblemFlag;
			}
		}

	}NxCatchAll(__FUNCTION__);
	return 0;
}

// (j.jones 2010-09-30 14:11) - PLID 40730 - added ability to rename an EMR/EMN
void CPatientNexEMRDlg::OnRenameEMR()
{
	try {

		if(m_bExpired) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();

		if(pCurSel == NULL) {
			return;
		}

		long nEMRID = VarLong(pCurSel->GetValue(eecEMRID),-1);
		long nEMNID = VarLong(pCurSel->GetValue(eecEMNID),-1);

		if(nEMRID == -1 && nEMNID == -1) {
			//this record is neither an EMN nor an EMR (how is that possible?)
			return;
		}

		//check permissions
		if(!CheckCurrentUserPermissions(bioPatientEMR, sptWrite)) {
			return;
		}

		if(nEMNID != -1) {
			//rename the EMN

			//do they have access?

			// (j.armen 2013-05-14 12:35) - PLID 56680 - EMN Access refactoring
			_RecordsetPtr rs = CreateParamRecordset(
				"SELECT 1 FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK) WHERE EmnID = {INT}\r\n"
				""
				"SELECT Description FROM EMRMasterT WHERE ID = {INT}",
				nEMNID, nEMNID);

			if(!rs->eof) {
				MsgBox("This EMN is currently being modified by another user. Its description cannot be changed at this time.");
				return;
			}

			rs = rs->NextRecordset(NULL);

			//get the absolute most current description
			CString strOldDescription = "";
			if(!rs->eof) {
				strOldDescription = AdoFldString(rs, "Description", "");
			}
			rs->Close();

			CString strNewDescription = strOldDescription;
			if(InputBoxLimited(this, "Enter a new description for this EMN:",strNewDescription,"",255,false,false,NULL) != IDOK) {
				return;
			}

			//yell at them if they entered a blank name (technically it is supported
			//but we try really hard to not let them do this, so just disallow)
			strNewDescription.TrimLeft();
			strNewDescription.TrimRight();

			if(strNewDescription == strOldDescription) {
				//just return silently
				return;
			}

			if(strNewDescription.IsEmpty()) {
				MsgBox("No EMN description was entered. No change has been made.");
				return;
			}

			//now save, checking once more that nobody has taken access since we last checked
			long nAffected = 0;
			{
				// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
				// (j.armen 2013-05-14 12:35) - PLID 56680 - EMN Access Refactoring
				NxAdo::PushMaxRecordsWarningLimit pmr(1);
				ExecuteParamSql(GetRemoteData(), &nAffected,
					"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
					"	UPDATE EMRMasterT SET Description = {STRING}, ModifiedDate = GetDate()\r\n"
					"WHERE ID = {INT}\r\n"
					"	AND ID NOT IN (SELECT EmnID FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK) WHERE EmnID = {INT})",
					strNewDescription, nEMNID, nEMNID);
			}

			if (nAffected > 0) {				
				//the change succeeded, so audit and send a tablechecker					
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiEMNDescription, nEMNID, strOldDescription, strNewDescription, aepHigh, aetChanged);

				//this table checker will cause the screen to refresh, so we don't need to now
				CClient::RefreshTable(NetUtils::EMRMasterT, GetActivePatientID());

			} else {
				MsgBox("This EMN is being modified by another user and cannot be edited.");
			}
		}
		else if(nEMRID != -1) {
			//rename the EMR

			//do they have access?

			// (j.armen 2013-05-14 12:35) - PLID 56680 - EMN Access Refactoring
			_RecordsetPtr rs = CreateParamRecordset(
				"SELECT 1 FROM EMNAccessT WITH(UPDLOCK, HOLDLOCK) "
				"WHERE EmnID IN (SELECT ID FROM EMRMasterT WHERE EmrGroupID = {INT} AND Deleted = 0)\r\n"
				""
				"SELECT Description FROM EMRGroupsT WHERE ID = {INT}",
				nEMRID, nEMRID);

			if(!rs->eof) {
				MsgBox("This EMR contains EMNs which are currently being modified by another user. Its description cannot be changed at this time.");
				return;
			}

			rs = rs->NextRecordset(NULL);

			//get the absolute most current description
			CString strOldDescription = "";			
			if(!rs->eof) {
				strOldDescription = AdoFldString(rs, "Description", "");
			}
			rs->Close();

			CString strNewDescription = strOldDescription;
			if(InputBoxLimited(this, "Enter a new description for this EMR:",strNewDescription,"",255,false,false,NULL) != IDOK) {
				return;
			}

			//yell at them if they entered a blank name (technically it is supported
			//but we try really hard to not let them do this, so just disallow)
			strNewDescription.TrimLeft();
			strNewDescription.TrimRight();

			if(strNewDescription == strOldDescription) {
				//just return silently
				return;
			}

			if(strNewDescription.IsEmpty()) {
				MsgBox("No EMR description was entered. No change has been made.");
				return;
			}

			//now save, checking once more that nobody has taken access since we last checked
			long nAffected = 0;
			{
				// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
				NxAdo::PushMaxRecordsWarningLimit pmr(1);
				ExecuteParamSql(GetRemoteData(), &nAffected, 
					"SET NOCOUNT OFF\r\n" // (a.walling 2011-05-27 12:37) - PLID 43866 - Explicitly set NOCOUNT
					"UPDATE EMRGroupsT SET Description = {STRING} "
					"WHERE EMRGroupsT.ID = {INT} AND EMRGroupsT.ID NOT IN ("
					"	SELECT EMRMasterT.EMRGroupID FROM EMRMasterT WITH(UPDLOCK, HOLDLOCK) "
					"	INNER JOIN EMNAccessT WITH(UPDLOCK, HOLDLOCK) ON EMRMasterT.ID = EMNAccessT.EMNID "
					"	WHERE EMRMasterT.EMRGroupID = {INT} "
					"	AND EMRMasterT.Deleted = 0 AND EMRGroupsT.Deleted = 0)", strNewDescription, nEMRID, nEMRID);
			}

			if (nAffected > 0) {				
				//the change succeeded, so audit and send a tablechecker					
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(GetActivePatientID(), GetActivePatientName(), nAuditID, aeiEMRDescription, nEMRID, strOldDescription, strNewDescription, aepHigh, aetChanged);

				//this table checker will cause the screen to refresh, so we don't need to now
				CClient::RefreshTable(NetUtils::EMRMasterT, GetActivePatientID());

			} else {
				MsgBox("This EMR is being modified by another user and cannot be edited.");
			}
		}

	} NxCatchAll(__FUNCTION__);
}

//(e.lally 2011-12-13) PLID 46968 - Makes the existing patient EMN available for a patient to fill out online via NexWeb
void CPatientNexEMRDlg::OnPublishEMNToNexWeb()
{
	try {
		//Is the EMR license expired?
		if(m_bExpired) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();

		if(pCurSel == NULL) {
			return;
		}

		long nEMNID = VarLong(pCurSel->GetValue(eecEMNID),-1);
		if(nEMNID == -1){
			//We're on an EMR? This shouldn't be happening
			return;
		}

		//(e.lally 2011-12-13) PLID 46968 - officially validate license and check permissions
		if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcNexWebPortal, CLicense::cflrUse) 
			|| !(CheckCurrentUserPermissions(bioPublishEMNToNexWeb, sptDynamic0)) ){
				//denied
				return;
		}

		_variant_t varUserLoginTokenID = g_cvarNull;
		long nStatus = -1;
		CEMN::EPatientCreatedStatus ePatientCreatedStatus = CEMN::pcsInvalid;
		//(e.lally 2011-12-14) PLID 46968 - Loading a whole EMN is a slow process for a few pieces of information. 
		//	Querying the database specifically for just the data we need is much faster here.
		//Get the status and write access information
		// (j.armen 2013-05-14 12:36) - PLID 56680 - EMN Access Refactoring
		_RecordsetPtr rs = CreateParamRecordset(
			"SELECT EMRMasterT.Status, EMNAccessT.UserLoginTokenID, EMRMasterT.PatientCreatedStatus "
			"FROM EMRMasterT "
			"LEFT JOIN EMNAccessT WITH(UPDLOCK, HOLDLOCK) ON EMRMasterT.ID = EMNAccessT.EmnID "
			"WHERE EMRMasterT.ID = {INT}", nEMNID);
		if(!rs->eof) {
			varUserLoginTokenID = rs->Fields->Item["UserLoginTokenID"]->Value;
			nStatus = AdoFldLong(rs, "Status",-1);
			ePatientCreatedStatus = (CEMN::EPatientCreatedStatus)AdoFldByte(rs, "PatientCreatedStatus", CEMN::pcsInvalid);
		}
		rs->Close();

		//Check if it is locked now
		if(nStatus == 2){
			//Reload the EMR list and prompt the user.
			ReloadEMRList();
			MessageBox("This EMN has been locked and can no longer be published.", "NexTech Practice", MB_OK|MB_ICONERROR);
			return;
		}

		//Check if someone has the write access
		if (varUserLoginTokenID.vt != VT_NULL) {
			// someone has this EMN open for editing
			MessageBox("This EMN has read-only access available and cannot be published until all users have released their write access.", "NexTech Practice", MB_OK|MB_ICONERROR);
			return;
		}

		//Check if it is already marked as patient created not finalized
		if(ePatientCreatedStatus == CEMN::pcsCreatedByPatientNotFinalized){
			//It's already been published
			MessageBox("This EMN can no longer be published, because the patient already has access to edit it.", "NexTech Practice", MB_OK|MB_ICONERROR);
			return;
		}

		//Prompt the user to double check the answers they are making available are ok for the patient to see
		if(IDOK != MessageBox(("This patient will be able to view and edit all the content "
			"already saved on this EMN through their NexWeb Patient Portal. Make sure you have "
			"reviewed its contents before continuing.\n\n"
			"Are you sure you wish to continue?"), "NexTech Practice", MB_OKCANCEL|MB_ICONWARNING)){
				return;
		}

		CString strOldStatus = "";
		if(ePatientCreatedStatus == CEMN::pcsCreatedByPatientFinalized){
			strOldStatus = "Patient-Finalized EMN";
		}
		else if(ePatientCreatedStatus == CEMN::pcsCreatedByPractice){
			strOldStatus = "Practice-Created EMN";
		}

		//(e.lally 2011-12-13) PLID 46968 - Update it to '1' (patient created)
		ExecuteParamSql("UPDATE EMRMasterT SET PatientCreatedStatus = 1 WHERE EMRMasterT.ID = {INT};", nEMNID);
		//Reflect the change in the datalist
		pCurSel->PutValue(eecPatientCreatedStatus, _variant_t((short)CEMN::pcsCreatedByPatientNotFinalized, VT_I2));
		//(e.lally 2011-12-13) PLID 46968 - Audit
		AuditEvent(GetActivePatientID(), GetActivePatientName(), BeginNewAuditEvent(), aeiFinalizeNexWebEMN, nEMNID, strOldStatus, "Patient-Entered EMN", aepMedium, aetChanged);

	}NxCatchAll(__FUNCTION__);
}


// (j.gruber 2013-11-11 16:52) - PLID 59415 - created for
void CPatientNexEMRDlg::OnGenerateClinicalSummaryXMLOnly()
{
	try {
		//Is the EMR license expired?
		if(m_bExpired) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();

		if(pCurSel == NULL) {
			return;
		}

		// (r.gonet 04/22/2014) - PLID 61805 - Clinical Summaries now need to be associated with a PIC
		long nPICID = VarLong(pCurSel->GetValue(eecPicID),-1);
		long nEMNID = VarLong(pCurSel->GetValue(eecEMNID),-1);
		if(nPICID == -1 || nEMNID == -1){
			//We're on an EMR? This shouldn't be happening
			return;
		}		
		
		// (r.gonet 04/22/2014) - PLID 61805 - Pass along the PICID so it gets associated with the MailSent record.
		GenerateAndAttachClinicalSummary(nPICID, nEMNID, true, false);

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2013-11-11 16:52) - PLID 59415 - created for
// (j.gruber 2014-05-06 14:06) - PLID 61923 - taken out
/*void CPatientNexEMRDlg::OnGenerateClinicalSummaryPDFOnly()
{
	try {
		//Is the EMR license expired?
		if(m_bExpired) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();

		if(pCurSel == NULL) {
			return;
		}

		// (r.gonet 04/22/2014) - PLID 61805 - Clinical Summaries now need to be associated with a PIC
		long nPICID = VarLong(pCurSel->GetValue(eecPicID),-1);
		long nEMNID = VarLong(pCurSel->GetValue(eecEMNID),-1);
		if(nPICID == -1 || nEMNID == -1){
			//We're on an EMR? This shouldn't be happening
			return;
		}		
		
		// (r.gonet 04/22/2014) - PLID 61805 - Pass along the PICID so it gets associated with the MailSent record.
		GenerateAndAttachClinicalSummary(nPICID, nEMNID, false, true);

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2013-11-11 16:52) - PLID 59415 - created for
// (j.gruber 2014-05-06 14:06) - PLID 61923 - taken out
void CPatientNexEMRDlg::OnGenerateClinicalSummaryXMLAndPDF()
{
	try {
		//Is the EMR license expired?
		if(m_bExpired) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();

		if(pCurSel == NULL) {
			return;
		}

		// (r.gonet 04/22/2014) - PLID 61805 - Clinical Summaries now need to be associated with a PIC
		long nPICID = VarLong(pCurSel->GetValue(eecPicID),-1);
		long nEMNID = VarLong(pCurSel->GetValue(eecEMNID),-1);		
		if(nPICID == -1 || nEMNID == -1){
			//We're on an EMR? This shouldn't be happening
			return;
		}		
		
		// (r.gonet 04/22/2014) - PLID 61805 - Pass along the PICID so it gets associated with the MailSent record.
		GenerateAndAttachClinicalSummary(nPICID, nEMNID, true, true);
		

	}NxCatchAll(__FUNCTION__);
}*/


// (j.gruber 2013-11-11 16:52) - PLID 59415 - created for
void CPatientNexEMRDlg::OnGenerateClinicalSummaryCustomized()
{
	try {
		//Is the EMR license expired?
		if(m_bExpired) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();

		if(pCurSel == NULL) {
			return;
		}

		// (r.gonet 04/22/2014) - PLID 61805 - Clinical Summaries now need to be associated with a PIC
		long nPICID = VarLong(pCurSel->GetValue(eecPicID),-1);
		long nEMNID = VarLong(pCurSel->GetValue(eecEMNID),-1);		
		if(nPICID == -1 || nEMNID == -1){
			//We're on an EMR? This shouldn't be happening
			return;
		}		
		
		// (a.walling 2014-05-09 10:20) - PLID 61788
		// (r.gonet 04/22/2014) - PLID 61805 - Pass along the PICID so it gets associated with the MailSent record.
		CSelectCCDAInfoDlg dlg(ctClinicalSummary, nPICID, nEMNID, GetActivePatientID(), this);
		if (dlg.DoModal())
		{
			// (r.gonet 04/28/2014) - PLID 61807 - Get the EMR tab refreshed so that the clinical summary icon shows up.
			CClient::RefreshTable(NetUtils::EMRMasterT, GetActivePatientID());
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2013-11-11 16:52) - PLID 59415 - created for
// (r.gonet 04/22/2014) - PLID 61805 - Addded PICID since Clinical Summaries must be associated with PICs now.
void CPatientNexEMRDlg::GenerateAndAttachClinicalSummary(long nPICID, long nEMNID, bool bGenerateXML, bool bGeneratePDF)
{
	long nCurrentPersonID = GetActivePatientID();
	// (b.savon 2014-04-30 09:31) - PLID 61791 - User Story 10 - Auxiliary CCDA Items - Requirement 100150 - Forward to utilities
	// (r.gonet 04/22/2014) - PLID 61805 - Pass along the PICID so it gets associated with the MailSent record.
	CCDAUtils::GenerateClinicalSummary(nCurrentPersonID, nPICID, nEMNID, GetSafeHwnd(), bGenerateXML, bGeneratePDF);

	if (IsDlgButtonChecked(IDC_SHOW_HISTORY))
	{
		//reload the history items to they can see them
		ReloadEMRList();
	}
	// (r.gonet 04/28/2014) - PLID 61807 - Get the EMR tab refreshed so that the clinical summary icon shows up.
	CClient::RefreshTable(NetUtils::EMRMasterT, nCurrentPersonID);
}

//TES 11/21/2013 - PLID 57415 - Right-click option to generate a Cancer Case document
void CPatientNexEMRDlg::OnGenerateClinicalSummaryCancerCaseSubmission()
{
	try {
		//Is the EMR license expired?
		if(m_bExpired) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();

		if(pCurSel == NULL) {
			return;
		}

		long nEMRID = VarLong(pCurSel->GetValue(eecEMRID),-1);		
		if(nEMRID == -1){
			return;
		}		
		
		//TES 7/8/2013 - PLID 57415 - Determine if there are any diagnoses that should trigger generating a Cancer Case
		CEMR *pEMR = new CEMR();
		long nEMNIDTemp = -2;
		pEMR->LoadFromID(nEMRID, FALSE, nEMNIDTemp);
		//TES 11/14/2013 - PLID 57415 - If we find one, we're going to want to know which EMN and diagnosis it was. We also want to figure
		// out the most recent date any EMN was modified, we'll only generate a new document if an EMN has been modified since the last time
		// we generated one.
		long nEMNID = -1;
		long nCancerDiagCodeID = -1;
		CString strDiagCode, strDiagDesc;
		COleDateTime dtLastModified = COleDateTime(1900,1,1,0,0,0);
		//TES 5/1/2014 - PLID 61916 - Also track the EMN date, the Cancer Case document uses it in the filename
		COleDateTime dtEmnDate;
		//TES 5/2/2014 - PLID 61855 - Track the EMN description, CreateNewCancerCase uses it in pop-up messages
		CString strEmnDescription;
		for(int nEMN = 0; nEMN < pEMR->GetEMNCount(); nEMN++) {
			CEMN *pEMN = pEMR->GetEMN(nEMN);
			if(pEMN->GetEMNModifiedDate() > dtLastModified) {
				dtLastModified = pEMN->GetEMNModifiedDate();
			}
			for(int nDiag = 0; nDiag < pEMN->GetDiagCodeCount() && nCancerDiagCodeID == -1; nDiag++) {			
				EMNDiagCode *pDiag = pEMN->GetDiagCode(nDiag);
				// (j.gruber 2016-05-31 14:48) - NX-100777 - if they have both an ICD-9 and ICD-10, this is currently defaulting to the 9, we want it to default to the 10,
				//so reverse the order.  The only time it should show the 9 is if there is no 10. 
				if (IsCancerDiagnosis(pDiag->strCode_ICD10)) 
				{
					// (j.jones 2014-02-27 14:10) - PLID 61069 - now we track the diag code ID
					nCancerDiagCodeID = pDiag->nDiagCodeID_ICD10;
					strDiagCode = pDiag->strCode_ICD10;
					strDiagDesc = pDiag->strCodeDesc_ICD10;
					nEMNID = pEMN->GetID();
					dtEmnDate = pEMN->GetEMNDate();
					strEmnDescription = pEMN->GetDescription();
				}
				else if (IsCancerDiagnosis(pDiag->strCode)) 
				{
					// (j.jones 2014-02-27 14:10) - PLID 61069 - now we track the diag code ID
					nCancerDiagCodeID = pDiag->nDiagCodeID;
					strDiagCode = pDiag->strCode;
					strDiagDesc = pDiag->strCodeDesc;
					nEMNID = pEMN->GetID();
					dtEmnDate = pEMN->GetEMNDate();
					strEmnDescription = pEMN->GetDescription();
				}
				
			}
		}
		//TES 11/14/2013 - PLID 57415 - OK, if we found a cancer diagnosis, create the CDA document for it
		if(nCancerDiagCodeID != -1) {
			//TES 5/7/2015 - PLID 65969 - Prompt the user for an additional folder to save to
			CString strCancerCaseFolder;
			CString strLastExportFolder = GetRemotePropertyMemo("CancerCaseExport_LastFolder", "", 0, GetCurrentUserName());
			if (strLastExportFolder.IsEmpty() || !DoesExist(strLastExportFolder)) {
				strLastExportFolder = GetSharedPath() ^ "CancerCases";
				FileUtils::EnsureDirectory(strLastExportFolder);
			}
			if (!BrowseToFolder(&strCancerCaseFolder, "Select Cancer Case export folder (file will also be saved in the patient's history)", GetSafeHwnd(), NULL, strLastExportFolder)) {
				return;
			}
			SetRemotePropertyMemo("CancerCaseExport_LastFolder", strCancerCaseFolder, 0, GetCurrentUserName());
			// (j.jones 2014-02-27 14:10) - PLID 61069 - this now takes in a diag code ID, in addition to the code and description
			//TES 5/1/2014 - PLID 61916 - Moved this function to EmrUtils, so pass in the patient ID, and this window as a message parent
			//TES 5/1/2014 - PLID 61916 - Also pass in the EMN date
			//TES 5/2/2014 - PLID 61855 - Pass in the EMN description
			//TES 5/7/2015 - PLID 65969 - Pass in the additional folder
			CreateNewCancerCaseDocument(m_id, VarLong(pCurSel->GetValue(eecPicID),-1), nEMNID, nCancerDiagCodeID, strDiagCode, strDiagDesc, dtLastModified, dtEmnDate, this, NULL, strEmnDescription, strCancerCaseFolder);
		}
		else {
			MsgBox("This EMR does not have any cancer diagnoses attached to it. No Cancer Case submission has been created.");
		}
		

	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-04-24 17:10) - PLID 49863 - Disable and remove all of the dynamic buttons
void CPatientNexEMRDlg::ClearDynaButtons()
{
	m_aryDynaButtonIDs.RemoveAll();

	m_btnDynamic1.EnableWindow(FALSE);
	m_btnDynamic2.EnableWindow(FALSE);
	m_btnDynamic3.EnableWindow(FALSE);
	m_btnDynamic4.EnableWindow(FALSE);
	m_btnDynamic5.EnableWindow(FALSE);
	m_btnDynamic6.EnableWindow(FALSE);

	m_btnDynamic1.ShowWindow(SW_HIDE);
	m_btnDynamic2.ShowWindow(SW_HIDE);
	m_btnDynamic3.ShowWindow(SW_HIDE);
	m_btnDynamic4.ShowWindow(SW_HIDE);
	m_btnDynamic5.ShowWindow(SW_HIDE);
	m_btnDynamic6.ShowWindow(SW_HIDE);
}

// (j.armen 2012-04-24 17:11) - PLID 49863 - Add dynamic buttons
void CPatientNexEMRDlg::AddDynaButton(const long& nID, CString strName)
{
	UpdateData();
	CNxIconButton *pbtnDynamic = NULL;

	switch(m_aryDynaButtonIDs.GetSize())
	{
		case 0:	pbtnDynamic = &m_btnDynamic1;	break;
		case 1:	pbtnDynamic = &m_btnDynamic2;	break;
		case 2:	pbtnDynamic = &m_btnDynamic3;	break;
		case 3:	pbtnDynamic = &m_btnDynamic4;	break;
		case 4:	pbtnDynamic = &m_btnDynamic5;	break;
		case 5:	pbtnDynamic = &m_btnDynamic6;	break;
	}

	if(pbtnDynamic)
	{
		m_aryDynaButtonIDs.Add(nID);	// (j.armen 2012-04-24 17:14) - PLID 49863 - Store the id in our array
		strName.Replace("\r\n", " ");	//m.hancock - 2/23/2006 - PLID 19428 - Replace newline characters with a space
		strName.Replace("&", "&&");		//Handle any ampersands in text to remain text
		
		CRect rcBtn = CRect(0,0,0,0);
		CSize szText = CSize(0,0);
		pbtnDynamic->GetWindowRect(rcBtn);	// (j.armen 2012-04-24 17:14) - PLID 49863 - Get the button size
		pbtnDynamic->SetTextColor(RGB(0, 0, 255));	//Button text should be blue for "New" ness
		pbtnDynamic->SetToolTip(strName);

		for(int i = strName.GetLength(); i > 0; i--)
		{
			// (j.armen 2012-04-24 17:15) - PLID 49863 - Calculate the string that we want to attempt fitting onto the button
			CString strBtnText = strName.Left(i) + (i != strName.GetLength() ? "..." : "");

			// (j.armen 2012-04-24 17:15) - PLID 49863 - Get the size of the text on the screen
			szText = CClientDC(pbtnDynamic).GetTextExtent(strBtnText);
			
			// (j.armen 2012-04-24 17:15) - PLID 49863 - If the size of the text is less than or equal to the width of the button,
			//  then set the name and break out of the loop
			if(szText.cx <= rcBtn.Width()) {
				strName = strBtnText;
				break;
			}
		}

		// (j.armen 2012-04-24 17:17) - PLID 49863 - Set button text, enable, and show
		pbtnDynamic->SetWindowText(strName);
		pbtnDynamic->EnableWindow();
		pbtnDynamic->ShowWindow(SW_SHOW);		
	}
}

// (j.armen 2012-04-24 17:17) - PLID 49863 - Generic handler for but
void CPatientNexEMRDlg::EmrDynaBnClicked(const long& nID)
{
	if (m_bExpired) return;	// (a.walling 2007-11-28 10:19) - PLID 28044

	//Get the currently selected EMR
	NXDATALIST2Lib::IRowSettingsPtr pCurSel = GetCurSelExistingList();
	if(pCurSel == NULL) {
		//this shouldn't happen
		ASSERT(FALSE);
		return;
	}

	// (j.jones 2008-10-14 16:08) - PLID 31691 - a value of -2 is "more templates..."
	if(nID == -2) {
		OnMoreTemplates();
	} else {
		//pull the EMR ID out of the list
		_variant_t varEMR = pCurSel->GetValue(eecPicID);

		//Now open the editor with this EMR
		LaunchPICEditorWithNewEMN(VarLong(varEMR), nID);
	}
}

// (j.armen 2012-04-24 17:26) - PLID 49863 - Dynamic Button Handler
void CPatientNexEMRDlg::OnBnClickedEmrDyna1()
{
	try
	{
		if(m_aryDynaButtonIDs.GetSize() <= 0) 
			ThrowNxException("Unable to map dynamic button to emn");
		else
			EmrDynaBnClicked(m_aryDynaButtonIDs[0]);
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-04-24 17:26) - PLID 49863 - Dynamic Button Handler
void CPatientNexEMRDlg::OnBnClickedEmrDyna2()
{
	try
	{
		if(m_aryDynaButtonIDs.GetSize() <= 1) 
			ThrowNxException("Unable to map dynamic button to emn");
		else
			EmrDynaBnClicked(m_aryDynaButtonIDs[1]);
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-04-24 17:26) - PLID 49863 - Dynamic Button Handler
void CPatientNexEMRDlg::OnBnClickedEmrDyna3()
{
	try
	{
		if(m_aryDynaButtonIDs.GetSize() <= 2) 
			ThrowNxException("Unable to map dynamic button to emn");
		else
			EmrDynaBnClicked(m_aryDynaButtonIDs[2]);
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-04-24 17:26) - PLID 49863 - Dynamic Button Handler
void CPatientNexEMRDlg::OnBnClickedEmrDyna4()
{
	try
	{
		if(m_aryDynaButtonIDs.GetSize() <= 3) 
			ThrowNxException("Unable to map dynamic button to emn");
		else
			EmrDynaBnClicked(m_aryDynaButtonIDs[3]);
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-04-24 17:26) - PLID 49863 - Dynamic Button Handler
void CPatientNexEMRDlg::OnBnClickedEmrDyna5()
{
	try
	{
		if(m_aryDynaButtonIDs.GetSize() <= 4) 
			ThrowNxException("Unable to map dynamic button to emn");
		else
			EmrDynaBnClicked(m_aryDynaButtonIDs[4]);
	}NxCatchAll(__FUNCTION__);
}

// (j.armen 2012-04-24 17:26) - PLID 49863 - Dynamic Button Handler
void CPatientNexEMRDlg::OnBnClickedEmrDyna6()
{
	try
	{
		if(m_aryDynaButtonIDs.GetSize() <= 5) 
			ThrowNxException("Unable to map dynamic button to emn");
		else
			EmrDynaBnClicked(m_aryDynaButtonIDs[5]);
	}NxCatchAll(__FUNCTION__);
}


// (j.jones 2012-07-20 08:44) - PLID 41791 - This will apply our fixed sort order,
// which will only apply to Tab View upon creation, and Tree View at all times.
void CPatientNexEMRDlg::InitializeExistingListSortOrder()
{
	try
	{
		//moved to a function so we could call this in multiple places
		
		NXDATALIST2Lib::IColumnSettingsPtr pSortCol;
		for (int i = 0; i < m_pExistingList->GetColumnCount(); i++) {
			pSortCol = m_pExistingList->GetColumn(i);
			// (j.jones 2012-07-20 08:44) - PLID 41791 - This used to only sort by input date descending,
			// but it should really be service date descending. If two service dates are the same,
			// then we'll sort by input date as a secondary sort.
			if (i == eecDate) {
				pSortCol->SortPriority = 0;
				pSortCol->SortAscending = FALSE;
			}
			else if (i == eecInputDate) {
				pSortCol->SortPriority = 1;
				pSortCol->SortAscending = FALSE;
			}
			else {
				pSortCol->SortPriority = -1;
			}
		}		

	}NxCatchAll(__FUNCTION__);
}

//TES 5/1/2014 - PLID 61916 - Moved CreateNewCancerDocument() to EmrUtils
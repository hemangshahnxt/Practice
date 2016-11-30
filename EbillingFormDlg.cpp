// EbillingFormDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "EbillingFormDlg.h"
#include "HCFADlg.h"
#include "UB92.h"
#include "UB04.h"
#include "ADADlg.h"
#include "IDPADlg.h"
#include "NYWCDlg.h"
#include "MICRDlg.h"
#include "MICR2007Dlg.h"
#include "Practice.h"
#include "NxStandard.h"
#include "GlobalReportUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "Ebilling.h"
#include "ANSIProperties.h"
#include "ANSIReportParserDlg.h"
#include "ANSI277ReportParserDlg.h"
#include "EbillingValidationDlg.h"
#include "RetrieveEbillingBatchDlg.h"
#include "HCFAImageSetupDlg.h"
#include "EbillingValidationConfigDlg.h"
#include "OHIPSetupDlg.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "OHIPValidationConfigDlg.h"
#include "OHIPReportManagerDlg.h"
#include "OHIPDialerSetupDlg.h"
#include "OHIPDialerUtils.h"
#include "NYMedicaidDlg.h"
#include "OHIPUtils.h"
#include "SchedulerView.h"
#include "ResEntryDlg.h"
#include "BlankAssignmentBenefitsDlg.h"
#include "AlbertaHLINKSetupDlg.h"
#include "AlbertaClaimValidationConfigDlg.h"
#include "AlbertaHLINKUtils.h"
#include "AuditTrail.h"
#include "AlbertaAssessmentParser.h"
#include "EbillingClearinghouseIntegrationSetupDlg.h"
#include "GenericBrowserDlg.h"
#include "EEligibility.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

// (a.walling 2014-03-19 10:05) - PLID 61346 - Converted several #defines into Enums (eg HCFA, UB are now actProf, actInst respectively)

#define ID_SELECT_PROV		43021
#define ID_SELECT_CLAIM_PROV		43022	// (j.jones 2012-03-26 10:43) - PLID 48854 - added ability to select by claim provider

#define ID_PREVIEW_CLAIM_FILE_UNSELECTED	43023 	// (b.spivey, August 27th, 2014) - PLID 63492
#define ID_PREVIEW_CLAIM_FILE_SELECTED	43024 	// (b.spivey, August 27th, 2014) - PLID 63492

enum ProviderComboColumns {
	pccID = 0,
	pccName,
};

#define COLUMN_HCFA_ID					0
#define COLUMN_PATIENT_NAME				1
#define COLUMN_BILL_DATE				2
#define COLUMN_BILL_DESC				3
#define COLUMN_INSURED_PARTY_ID			4	// (j.jones 2009-09-01 08:54) - PLID 34749
#define COLUMN_INS_CO_ID				5
#define COLUMN_INS_CO_NAME				6
#define COLUMN_RESP_NAME				7
#define COLUMN_PROV_ID					8   //r.wilson (6/11/2012) PLID 48853 - Provider ID Field
#define COLUMN_PROV_NAME				9
#define COLUMN_CLAIM_PROV_ID			10   //r.wilson (6/11/2012) PLID 48853 - Claim Provider ID Field
#define COLUMN_CLAIM_PROV_NAME			11	// (j.jones 2012-03-26 08:58) - PLID 48854
#define COLUMN_BILL_ID					12
#define COLUMN_PAT_USER_DEFINED_ID		13
#define COLUMN_PATIENT_ID				14
#define COLUMN_CURRENT_SELECT			15
#define COLUMN_VALIDATION_STATE_VALUE	16
#define COLUMN_VALIDATION_STATE_ICON	17
#define COLUMN_LAST_SEND_DATE			18
#define COLUMN_COUNT_CHARGES_BATCHED	19
#define COLUMN_COUNT_CHARGES			20
#define COLUMN_CHARGES_BATCHED			21

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEbillingFormDlg dialog
CEbillingFormDlg::CEbillingFormDlg(CWnd* pParent)
	: CNxDialog(CEbillingFormDlg::IDD, pParent),
	m_providerChecker(NetUtils::Providers)
{
	//{{AFX_DATA_INIT(CEffectiveness)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Billing/Insurance_Billing/Send_an_Electronic_Batch.htm";

	m_hIconCheck = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_GREEN_CHECK), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_hIconRedX = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_X), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_hIconGrayX = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_GRAY_X), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_hIconQuestion = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_BLUE_QUESTION), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
}

CEbillingFormDlg::~CEbillingFormDlg()
{
	DestroyIcon((HICON)m_hIconCheck);
	DestroyIcon((HICON)m_hIconRedX);
	DestroyIcon((HICON)m_hIconGrayX);
	DestroyIcon((HICON)m_hIconQuestion);
}

void CEbillingFormDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEbillingFormDlg)
	DDX_Control(pDX, IDC_BTN_OHIP_REPORT_MANAGER, m_btnOHIPReportManager);
	//DDX_Control(pDX, IDC_FORMAT_OHIP_CLAIMS_ERROR_BTN, m_btnFormatOHIPClaimsError);
	//DDX_Control(pDX, IDC_FORMAT_OHIP_BATCH_EDIT_BTN, m_btnFormatOHIPBatchEdit);
	DDX_Control(pDX, IDC_BTN_CONFIGURE_CLAIM_VALIDATION, m_btnConfigureClaimValidation);
	DDX_Control(pDX, IDC_FORMAT_997_BTN, m_btnFormat997);
	DDX_Control(pDX, IDC_FORMAT_277_BTN, m_btnFormat277);
	DDX_Control(pDX, IDC_BTN_RETRIEVE_BATCHES, m_btnRetrieveBatches);
	DDX_Control(pDX, IDC_RADIO_PRODUCTION_BATCH, m_production);
	DDX_Control(pDX, IDC_RADIO_TEST_BATCH, m_test);
	DDX_Control(pDX, IDC_VALIDATE_UNSELECTED_CLAIMS, m_btnValidateUnselected);	
	DDX_Control(pDX, IDC_VALIDATE_SELECTED_CLAIMS, m_btnValidateSelected);
	DDX_Control(pDX, IDC_VALIDATE_ALL_CLAIMS, m_btnValidateAll);
	DDX_Control(pDX, IDC_EXPORT, m_btnExportBatch);
	DDX_Control(pDX, IDC_PRINTLIST, m_btnPrintAll);
	DDX_Control(pDX, IDC_RESET, m_btnResetAll);
	DDX_Control(pDX, IDC_PRINT_UNSELECTED_LIST, m_btnPrintUnselected);
	DDX_Control(pDX, IDC_PRINT_SELECTED_LIST, m_btnPrintSelected);
	DDX_Control(pDX, IDC_RESET_SELECTED, m_btnResetSelected);
	DDX_Control(pDX, IDC_RESET_UNSELECTED, m_btnResetUnselected);
	DDX_Control(pDX, IDC_UNSELECT_ALL_EBILLS, m_btnUnselectAll);
	DDX_Control(pDX, IDC_UNSELECT_ONE_EBILL, m_btnUnselectOne);
	DDX_Control(pDX, IDC_SELECT_ALL_EBILLS, m_btnSelectAll);
	DDX_Control(pDX, IDC_SELECT_ONE_EBILL, m_btnSelectOne);
	DDX_Control(pDX, IDC_CONFIG, m_Config);
	DDX_Control(pDX, IDC_CLEARINGHOUSE_INTEGRATION_SETUP_BTN, m_btnClearinghouseIntegration);
	DDX_Control(pDX, IDC_EBILLING_UNSELECTED_LABEL, m_nxstaticEbillingUnselectedLabel);
	DDX_Control(pDX, IDC_UNSELECTED_TOTAL, m_nxstaticUnselectedTotal);
	DDX_Control(pDX, IDC_FORM_TYPE_LABEL, m_nxstaticFormTypeLabel);
	DDX_Control(pDX, IDC_EBILLING_SELECTED_LABEL, m_nxstaticEbillingSelectedLabel);
	DDX_Control(pDX, IDC_SELECTED_TOTAL, m_nxstaticSelectedTotal);
	DDX_Control(pDX, IDC_BTN_OHIP_DIALER_SETUP, m_btnOHIPDialerSetup);
	DDX_Control(pDX, IDC_BTN_DOWNLOAD_REPORTS, m_btnDownloadReports);
	DDX_Control(pDX, IDC_EBILLING_PROVIDER_LABEL, m_nxstaticProviderLabel);
	DDX_Control(pDX, IDC_LABEL_WARN_ASSIGNMENT_OF_BENEFITS_EBILLING, m_nxstaticAssignmentOfBenefitsWarningLabel);
	DDX_Control(pDX, IDC_BTN_WARN_ASSIGNMENT_OF_BENEFITS_EBILLING, m_btnAssignmentOfBenefitsWarning);
	DDX_Control(pDX, IDC_BTN_ALBERTA_ASSESSMENT_FILE, m_btnAlbertaAssessmentFile);
	DDX_Control(pDX, IDC_BTN_LAUNCH_TRIZETTO_WEBSITE, m_btnLaunchTrizettoWebsite);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEbillingFormDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEbillingFormDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	ON_BN_CLICKED(IDC_REMOVEBUTTON, OnRemoveHCFA)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_PRINTLIST, OnPrintList)
	ON_BN_CLICKED(IDC_OPENBUTTON, OnOpenHCFA)
	ON_COMMAND(ID_VALIDATE_CLAIM, OnValidateOne)
	ON_BN_CLICKED(IDC_GOTOPATIENT, OnGoToPatient)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(ID_SEND_TO_HCFA, SendToPaper)
	ON_BN_CLICKED(IDC_CONFIG, OnConfig)
	ON_BN_CLICKED(IDC_SELECT_ONE_EBILL, OnSelectOne)
	ON_BN_CLICKED(IDC_SELECT_ALL_EBILLS, OnSelectAll)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_EBILL, OnUnselectOne)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_EBILLS, OnUnselectAll)
	ON_BN_CLICKED(IDC_PRINT_SELECTED_LIST, OnPrintSelectedList)
	ON_BN_CLICKED(IDC_RESET_SELECTED, OnResetSelected)
	ON_BN_CLICKED(IDC_RESET_UNSELECTED, OnResetUnselected)
	ON_BN_CLICKED(IDC_PRINT_UNSELECTED_LIST, OnPrintUnselectedList)
	ON_BN_CLICKED(IDC_VALIDATE_ALL_CLAIMS, OnValidateAllClaims)
	ON_BN_CLICKED(IDC_VALIDATE_SELECTED_CLAIMS, OnValidateSelectedClaims)
	ON_BN_CLICKED(IDC_VALIDATE_UNSELECTED_CLAIMS, OnValidateUnselectedClaims)
	ON_BN_CLICKED(IDC_RADIO_PRODUCTION_BATCH, OnProductionBatch)
	ON_BN_CLICKED(IDC_RADIO_TEST_BATCH, OnTestBatch)
	ON_BN_CLICKED(IDC_BTN_RETRIEVE_BATCHES, OnBtnRetrieveBatches)
	ON_BN_CLICKED(IDC_FORMAT_277_BTN, OnFormat277Btn)
	ON_BN_CLICKED(IDC_FORMAT_997_BTN, OnFormat997Btn)
	ON_COMMAND(ID_SELECT_INSCO, OnSelectInsCo)
	ON_COMMAND(ID_SELECT_INSGROUP, OnSelectInsGroup)
	ON_COMMAND(ID_SELECT_PROV, OnSelectProvider)
	ON_COMMAND(ID_SELECT_CLAIM_PROV, OnSelectClaimProvider)
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_CLAIM_VALIDATION, OnBtnConfigureClaimValidation)	
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_REMOVE_HCFA, OnRemoveHCFA)
	ON_BN_CLICKED(IDC_OPEN_HCFA, OnOpenHCFA)	
	//ON_BN_CLICKED(IDC_FORMAT_OHIP_BATCH_EDIT_BTN, OnFormatOhipBatchEditBtn)
	//ON_BN_CLICKED(IDC_FORMAT_OHIP_CLAIMS_ERROR_BTN, OnFormatOhipClaimsErrorBtn)
	ON_BN_CLICKED(IDC_BTN_OHIP_REPORT_MANAGER, OnBtnOhipReportManager)
	ON_BN_CLICKED(IDC_BTN_OHIP_DIALER_SETUP, OnBtnOhipDialerSetup)
	ON_BN_CLICKED(IDC_BTN_DOWNLOAD_REPORTS, OnBtnDownloadReports)
	ON_COMMAND(ID_SEND_LIST_TO_HCFA, OnSendListToHcfa)
	//}}AFX_MSG_MAP		
	ON_BN_CLICKED(IDC_BTN_WARN_ASSIGNMENT_OF_BENEFITS_EBILLING, OnBtnWarnAssignmentOfBenefitsEbilling)
	ON_BN_CLICKED(IDC_BTN_ALBERTA_ASSESSMENT_FILE, OnBtnAlbertaAssessmentFile)
	ON_COMMAND(ID_MARK_CLAIM_SENT_SELECTED, OnMarkClaimSentSelected)
	ON_COMMAND(ID_MARK_ALL_CLAIMS_SENT_SELECTED, OnMarkAllClaimsSentSelected)
	ON_COMMAND(ID_MARK_CLAIM_SENT_UNSELECTED, OnMarkClaimSentUnselected)
	ON_COMMAND(ID_MARK_ALL_CLAIMS_SENT_UNSELECTED, OnMarkAllClaimsSentUnselected)
	ON_COMMAND(ID_PREVIEW_CLAIM_FILE_UNSELECTED, OnPreviewClaimFileUnselected)
	ON_COMMAND(ID_PREVIEW_CLAIM_FILE_SELECTED, OnPreviewClaimFileSelected)
	ON_BN_CLICKED(IDC_CLEARINGHOUSE_INTEGRATION_SETUP_BTN, &CEbillingFormDlg::OnBnClickedClearinghouseIntegrationSetupBtn)
	ON_BN_CLICKED(IDC_BTN_LAUNCH_TRIZETTO_WEBSITE, &CEbillingFormDlg::OnBtnLaunchTrizettoWebsite)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEbillingFormDlg message handlers

BOOL CEbillingFormDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();
	m_UnselectedBatchList = BindNxDataListCtrl(IDC_UNSELECTED_BATCHLIST, false);
	m_SelectedBatchList = BindNxDataListCtrl(IDC_SELECTED_BATCHLIST, false);
	m_LocationCombo = BindNxDataListCtrl(IDC_LOCATION_COMBO_ELECTRONIC);
	m_ExportStyle = BindNxDataListCtrl(IDC_EXPORTSTYLE,false);
	m_FormTypeCombo = BindNxDataListCtrl(IDC_EBILLING_FORM_TYPE_COMBO,false);
	m_ExportFormat = BindNxDataListCtrl(IDC_EXPORTFORMAT, false);
	// (j.jones 2009-08-14 09:49) - PLID 35235 - added provider filter
	m_ProviderCombo = BindNxDataListCtrl(IDC_EBILLING_PROVIDER_COMBO);

	// (j.jones 2011-08-16 17:22) - PLID 44805 - finally moved this query into code
	// (j.jones 2012-03-26 08:59) - PLID 48854 - added claim provider name
	CString strListFrom = "";

	// (j.jones 2014-07-09 09:11) - PLID 62568 - bills with an "on hold" status are hidden here
	strListFrom.Format("(SELECT HCFATrackT.ID AS HCFAID, BillsQ.FormType, BillsQ.ID AS BillID, BillsQ.Date, BillsQ.PatientID, "
		"BillsQ.Description AS BillDesc, PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle AS PatName, "	
		"CASE WHEN CountOfDocsQ.CountDocs = 1 THEN ("
		"	SELECT ID FROM PersonT "
		"	WHERE ID IN ("
		"		SELECT DoctorsProviders FROM ChargesT "
		"		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"		WHERE Deleted = 0 AND Batched = 1 "
		"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"		AND BillID = BillsQ.ID"
		"		) "
		"	) END AS DocID, "		
		"CASE WHEN CountOfDocsQ.CountDocs = 1 THEN ("
		"	SELECT Last + ', ' + First + ' ' + Middle FROM PersonT "
		"	WHERE ID IN ("
		"		SELECT DoctorsProviders FROM ChargesT "
		"		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"		WHERE Deleted = 0 AND Batched = 1 "
		"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"		AND BillID = BillsQ.ID"
		"		) "
		"	) END AS DocName, "
		"CASE CountOfClaimProvsQ.CountDocs  "
		"	WHEN 1 THEN CountOfClaimProvsQ.FirstProviderID "
		"	ELSE -1  "
		"	END AS ClaimProvID, "		
		"CASE WHEN CountOfClaimProvsQ.CountDocs = 1 THEN ("
		"	SELECT Last + ', ' + First + ' ' + Middle FROM PersonT "
		"	WHERE ID = CountOfClaimProvsQ.FirstProviderID "
		"	) END AS ClaimProvider, "
		"BillsQ.InsuredPartyID, InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, "
		"RespTypeT.TypeName AS RespName, PatientsT.UserDefinedID, HCFATrackT.CurrentSelect, "
		"HCFATrackT.ValidationState, LastClaimQ.LastSendDate, "
		"(SELECT Count(ChargesT.ID) "
		"	FROM ChargesT "
		"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"	WHERE Deleted = 0 AND Batched = 1 "
		"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"	AND ChargesT.BillID = BillsQ.ID "
		") AS CountOfChargesBatched, "
		"(SELECT Count(ChargesT.ID) "
		"	FROM ChargesT "
		"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"	WHERE Deleted = 0 AND ChargesT.BillID = BillsQ.ID "
		"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		") AS CountOfCharges "
		"FROM HCFATrackT "
		"INNER JOIN (SELECT * FROM BillsT WHERE Deleted = 0) AS BillsQ ON HCFATrackT.BillID = BillsQ.ID "
		"LEFT JOIN BillStatusT ON BillsQ.StatusID = BillStatusT.ID "
		"INNER JOIN (SELECT * FROM ChargesT "
		"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"	WHERE Batched = 1 "
		"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"	) AS ChargesT ON BillsQ.ID = ChargesT.BillID "
		"INNER JOIN (SELECT * FROM LineItemT WHERE Deleted = 0) AS LineItemQ ON ChargesT.ID = LineItemQ.ID "
		"INNER JOIN PatientsT ON BillsQ.PatientID = PatientsT.PersonID "
		"INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID "
		"LEFT JOIN ("
		"	SELECT Count(DoctorsProviders) AS CountDocs, BillID "
		"	FROM ("
		"		SELECT DoctorsProviders, BillID "
		"		FROM (SELECT * FROM ChargesT "
		"			LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"			LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"			WHERE Batched = 1 "
		"			AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"			AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"		) AS ChargesT "
		"		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"		WHERE Deleted = 0 AND DoctorsProviders <> -1 "
		"		GROUP BY BillID, DoctorsProviders "
		"	) AS CountDocsQ "
		"	GROUP BY BillID "
		") AS CountOfDocsQ ON BillsQ.ID = CountOfDocsQ.BillID "
		"LEFT JOIN InsuredPartyT ON BillsQ.InsuredPartyID = InsuredPartyT.PersonID "
		"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"LEFT JOIN ("
		"	SELECT Max(Date) AS LastSendDate, BillID "
		"	FROM ClaimHistoryT "
		//(r.wilson 10/2/2012) plid 52970 - Line below was "   WHERE SendType >= 0 "
		"	WHERE SendType >= %li "
		"	GROUP BY BillID "
		") AS LastClaimQ ON BillsQ.ID = LastClaimQ.BillID "
		"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
		"LEFT JOIN ("
		"	SELECT Min(ProviderID) AS FirstProviderID, Count(ProviderID) AS CountDocs, BillID "
		"	FROM ("
		"		SELECT BillID, "
		"		CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
		"			(CASE WHEN BillsT.FormType = 2 "
		"				THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
		"			WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
		"			ELSE ChargeProvidersT.ClaimProviderID END) "
		"		WHEN BillsT.FormType = 1 "
		"			THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID Is Not Null "
		"				THEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID "
		"				ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
		"				ELSE ChargeProvidersT.ClaimProviderID END) "
		"		END) "
		"		ELSE ChargeProvidersT.ClaimProviderID END) END AS ProviderID "
		"		FROM (SELECT * FROM ChargesT "
		"			LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"			LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"			WHERE Batched = 1 "
		"			AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"			AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"		) AS ChargesT "
		"		INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"		INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
		"		INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
		"		LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
		"		LEFT JOIN ProvidersT ChargeProvidersT ON ChargesT.DoctorsProviders = ChargeProvidersT.PersonID "
		"		LEFT JOIN ProvidersT PatientProvidersT ON PatientsT.MainPhysician = PatientProvidersT.PersonID "
		"		LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
		"		LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"		LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
		"		LEFT JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "
		"		LEFT JOIN HCFAClaimProvidersT ON InsuranceCoT.PersonID = HCFAClaimProvidersT.InsuranceCoID AND (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.PersonID ELSE ChargeProvidersT.PersonID END) = HCFAClaimProvidersT.ProviderID "
		"		WHERE LineItemT.Deleted = 0 "
		"		AND "
		"			CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
		"				(CASE WHEN BillsT.FormType = 2 "
		"					THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
		"				WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
		"				ELSE ChargeProvidersT.ClaimProviderID END) "
		"			WHEN BillsT.FormType = 1 "
		"				THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID Is Not Null "
		"					THEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID "
		"					ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
		"					ELSE ChargeProvidersT.ClaimProviderID END) "
		"			END) "
		"			ELSE ChargeProvidersT.ClaimProviderID END) END <> -1 "
		"		GROUP BY BillID, "
		"			CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
		"				(CASE WHEN BillsT.FormType = 2 "
		"					THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
		"				WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
		"				ELSE ChargeProvidersT.ClaimProviderID END) "
		"			WHEN BillsT.FormType = 1 "
		"				THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID Is Not Null "
		"					THEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID "
		"					ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
		"					ELSE ChargeProvidersT.ClaimProviderID END) "
		"			END) "
		"			ELSE ChargeProvidersT.ClaimProviderID END) END "
		"	) AS CountClaimProvsQ "
		"	GROUP BY BillID "
		") AS CountOfClaimProvsQ ON BillsQ.ID = CountOfClaimProvsQ.BillID "		
		"WHERE Coalesce(BillStatusT.Type, -1) != %li "
		"GROUP BY HCFATrackT.ID, BillsQ.FormType, BillsQ.ID, BillsQ.Date, BillsQ.PatientID, BillsQ.Description, "
		"PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle, CountOfDocsQ.CountDocs, CountOfClaimProvsQ.CountDocs, CountOfClaimProvsQ.FirstProviderID, "
		"InsuranceCoT.PersonID, InsuranceCoT.Name, RespTypeT.TypeName, PatientsT.UserDefinedID, "
		"HCFATrackT.Batch, BillsQ.InsuredPartyID, HCFATrackT.CurrentSelect, HCFATrackT.ValidationState, "
		"LastClaimQ.LastSendDate "
		"HAVING HCFATrackT.Batch = 2 AND HCFATrackT.CurrentSelect = [CurrentSelectFilter] "
		") AS HCFAEbillQ", ClaimSendType::Electronic, EBillStatusType::OnHold);

	CString strUnselectedFrom = strListFrom;
	strUnselectedFrom.Replace("[CurrentSelectFilter]","0");
	m_UnselectedBatchList->PutFromClause((LPCTSTR)strUnselectedFrom);

	CString strSelectedFrom = strListFrom;
	strSelectedFrom.Replace("[CurrentSelectFilter]","1");
	m_SelectedBatchList->PutFromClause((LPCTSTR)strSelectedFrom);

	// (j.jones 2010-07-23 16:15) - PLID 34105 - hide the warning label by default,
	// will be shown again if needed in UpdateView
	m_nxstaticAssignmentOfBenefitsWarningLabel.SetColor(RGB(255,0,0));
	extern CPracticeApp theApp;
	m_nxstaticAssignmentOfBenefitsWarningLabel.SetFont(&theApp.m_boldFont);
	m_nxstaticAssignmentOfBenefitsWarningLabel.ShowWindow(SW_HIDE);
	m_btnAssignmentOfBenefitsWarning.SetIcon(IDI_BLUE_QUESTION);
	m_btnAssignmentOfBenefitsWarning.ShowWindow(SW_HIDE);

	IRowSettingsPtr pRow;
	pRow = m_FormTypeCombo->GetRow(-1);
	pRow->PutValue(0,(long)1);
	pRow->PutValue(1,"HCFA");
	m_FormTypeCombo->AddRow(pRow);
	pRow = m_FormTypeCombo->GetRow(-1);
	pRow->PutValue(0,(long)2);
	//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
	pRow->PutValue(1,"UB");
	m_FormTypeCombo->AddRow(pRow);

	// (j.jones 2008-05-09 09:35) - PLID 29986 - completely removed NSF from the program,
	// but since we store the last export type they used, we can't change the indices here
	pRow = m_FormTypeCombo->GetRow(-1);
	pRow->PutValue(0,(long)IMAGE);
	pRow->PutValue(1,"Image");
	m_ExportFormat->AddRow(pRow);
	pRow = m_FormTypeCombo->GetRow(-1);
	pRow->PutValue(0,(long)ANSI);
	pRow->PutValue(1,"ANSI");
	m_ExportFormat->AddRow(pRow);
	pRow = m_FormTypeCombo->GetRow(-1);
	// (j.jones 2006-11-09 08:29) - PLID 21570 - added support for OHIP
	pRow->PutValue(0,(long)OHIP);
	pRow->PutValue(1,"OHIP");
	m_ExportFormat->AddRow(pRow);
	// (j.jones 2010-07-09 14:37) - PLID 29971 - added support for Alberta HLINK
	pRow = m_FormTypeCombo->GetRow(-1);
	pRow->PutValue(0,(long)ALBERTA);
	pRow->PutValue(1,"Alberta HLINK");
	m_ExportFormat->AddRow(pRow);

	pRow = m_LocationCombo->GetRow(-1);
	pRow->PutValue(0,(long)-1);
	pRow->PutValue(1,_bstr_t(" <All Locations>"));
	m_LocationCombo->AddRow(pRow);
	m_LocationCombo->TrySetSelByColumn(0,(long)-1);

	// (j.jones 2008-05-07 11:34) - PLID 29854 - set some icons for modernization,
	// but intentionally kept the color scheme from before so as to maintain the
	// button grouping appearance

	m_btnSelectOne.AutoSet(NXB_DOWN);
	m_btnSelectAll.AutoSet(NXB_DDOWN);
	m_btnUnselectOne.AutoSet(NXB_UP);
	m_btnUnselectAll.AutoSet(NXB_UUP);

	m_btnResetUnselected.AutoSet(NXB_DELETE);
	m_btnResetAll.AutoSet(NXB_DELETE);
	m_btnResetSelected.AutoSet(NXB_DELETE);

	m_btnPrintUnselected.AutoSet(NXB_PRINT_PREV);
	m_btnPrintAll.AutoSet(NXB_PRINT_PREV);
	m_btnPrintSelected.AutoSet(NXB_PRINT_PREV);

	m_btnValidateUnselected.AutoSet(NXB_INSPECT);
	m_btnValidateAll.AutoSet(NXB_INSPECT);
	m_btnValidateSelected.AutoSet(NXB_INSPECT);

	m_btnExportBatch.AutoSet(NXB_EXPORT);

	m_btnConfigureClaimValidation.AutoSet(NXB_MODIFY);
	m_btnRetrieveBatches.AutoSet(NXB_MODIFY);
	m_Config.AutoSet(NXB_MODIFY);
	m_btnClearinghouseIntegration.AutoSet(NXB_MODIFY);

	// (j.jones 2009-03-09 15:07) - PLID 32405 - added m_btnOHIPDialerSetup	
	m_btnOHIPDialerSetup.AutoSet(NXB_MODIFY);
	// (j.jones 2009-03-09 15:07) - PLID 33419 - added m_btnDownloadReports
	m_btnDownloadReports.AutoSet(NXB_INSPECT);

	// (d.singleton 2011-07-05) - PLID 44422 - button to open Alberta billing error reports parser dialog
	m_btnAlbertaAssessmentFile.AutoSet(NXB_INSPECT);

	m_btnLaunchTrizettoWebsite.SetIcon(IDI_TRIZETTO);
	m_btnLaunchTrizettoWebsite.SetTextColor(0xA00000);	//this is the same color as the Export button

	//set colors to more easily separate the three types of options
	m_btnResetUnselected.SetTextColor(RGB(150,0,0));
	m_btnPrintUnselected.SetTextColor(RGB(150,0,0));
	m_btnValidateUnselected.SetTextColor(RGB(150,0,0));
	m_btnResetAll.SetTextColor(RGB(0,0,150));
	m_btnPrintAll.SetTextColor(RGB(0,0,150));
	m_btnValidateAll.SetTextColor(RGB(0,0,150));
	m_btnResetSelected.SetTextColor(RGB(0,100,0));
	m_btnPrintSelected.SetTextColor(RGB(0,100,0));
	m_btnValidateSelected.SetTextColor(RGB(0,100,0));
	
	try {

		// (j.jones 2006-05-08 14:16) - PLID 20479 - Load all common ebilling tab properties into the
		// NxPropManager cache
		g_propManager.CachePropertiesInBulk("EbillingFormDlg-Number", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'FormatStyle' OR "
			"Name = 'EbillingFormatType' OR "
			"Name = 'EbillingClaimType' OR "
			"Name = 'EnvoyProduction' OR "
			// (j.jones 2008-02-11 15:30) - PLID 28847 - included DisallowBatchingPatientClaims, and extraneous preferences
			"Name = 'DisallowBatchingPatientClaims' OR "
			"Name = 'Use2007MICR' OR "
			"Name = 'AutoValidateBatchClaims' OR "
			// (j.jones 2008-02-12 11:35) - PLID 28848 - added HidePatientChargesOnClaims
			"Name = 'HidePatientChargesOnClaims' OR "
			// (j.jones 2009-08-14 09:55) - PLID 35235 - use the OHIP per-provider setting
			"Name = 'OHIP_SeparateAccountsPerProviders' OR "
			// (j.jones 2010-02-02 09:16) - PLID 33060 - added option to send G2 ref. phy. in a claim
			"Name = 'OHIPUseG2RefPhy' OR "
			"Name = 'HCFAImageChargesPerPage' OR "
			// (j.jones 2010-04-16 11:14) - PLID 38225 - added ability to disable sending REF*XX in provider loops
			"Name = 'ANSIDisableREFXX' OR "
			// (j.jones 2010-05-18 11:09) - PLID 37788 - added ShowPatientIDForInsurance
			"Name = 'ShowPatientIDForInsurance' OR "
			// (j.jones 2010-10-12 17:33) - PLID 29971 - cache Alberta HLINK settings
			"Name = 'Alberta_PatientULICustomField' OR "
			"Name = 'Alberta_PatientRegNumberCustomField' OR "
			"Name = 'Alberta_ProviderBAIDCustomField' OR "
			// (j.jones 2010-11-08 14:54) - PLID 39620 - added Alberta option
			"Name = 'UseAlbertaHLINK' OR "
			// (j.jones 2011-03-07 14:03) - PLID 42660 - cached CLIA settings
			// (j.jones 2011-04-05 10:20) - PLID 42372 - CLIA preferences are obsolete
			// (j.jones 2011-04-20 10:45) - PLID 41490 - added ability to ignore diagnosis codes not linked to charges
			"Name = 'SkipUnlinkedDiagsOnClaims' OR "
			"Name = 'Alberta_SubmitterPrefixCustomField' OR " // (j.dinatale 2012-12-28 11:30) - PLID 54370
			"Name = 'Claim_AllowSentWithoutPrinting' OR " //(r.wilson 10/8/2012) plid 40712
			"Name = 'Alberta_LastTransactionID' OR "// (j.dinatale 2013-02-05 10:56) - PLID 54733
			"Name = 'Claim_SendPatAddressWhenPOS12'	OR " // (j.jones 2013-04-24 17:09) - PLID 55564
			"Name = 'OHIP_AutoDialerEnabled' OR " 		// (b.spivey, June 26th, 2014) - PLID 62603 - cache
			"Name = 'EbillingClearinghouseIntegration_Enabled' OR "
			"Name = 'Clearinghouse_LoginType' "
			")",
			_Q(GetCurrentUserName()));

		// (j.jones 2010-07-12 14:49) - PLID 29971 - cached text preferences
		g_propManager.CachePropertiesInBulk("EbillingFormDlg-Text", propText,
			"(Username = '<None>' OR Username = '%s') AND ("			
			"Name = 'EbillingSubmitter' OR "
			"Name = 'ResSubmitter' OR "
			"Name = 'SiteID' OR "
			"Name = 'OHIP_GroupNumber' OR "
			"Name = 'OHIP_MOHOfficeCode' OR "
			// (j.jones 2010-10-12 17:33) - PLID 29971 - cache Alberta HLINK settings
			"Name = 'Alberta_SubmitterPrefix' OR "
			"Name = 'Alberta_PayToCode' OR "
			"Name = 'GlobalClearinghouseLoginName' "
			")",
			_Q(GetCurrentUserName()));
		
		g_propManager.CachePropertiesInBulk("EbillingFormDlg-Image", propImage,
			"(Username = '<None>') AND ("
			"Name = 'GlobalClearinghousePassword' "
			"OR Name = 'GlobalClearinghousePortalPassword' "
			")");

		//FormatID is the ID of the actual format used (Envoy, EZ2000, etc.)
		// (j.jones 2016-05-19 10:22) - NX-100685 - added a universal function for getting the default E-Billing / E-Eligibility format
		FormatID = GetDefaultEbillingANSIFormatID();

		// (j.jones 2006-11-14 15:32) - PLID 23534 - changed the default format style to Image,
		// also switched to Image if the stored format style was NSF and NSF is disabled
		// (j.jones 2008-12-02 09:00) - PLID 32291 - changed the default to be ANSI

		//FormatStyle is the Image, ANSI, or OHIP option
		FormatStyle = GetRemotePropertyInt("EbillingFormatType", ANSI, 0, "<None>", true);

		// (j.jones 2008-05-09 09:35) - PLID 29986 - completely removed NSF from the program
		if(FormatStyle != IMAGE && FormatStyle != ANSI && FormatStyle != OHIP && FormatStyle != ALBERTA) {
			// (j.jones 2009-03-12 11:55) - PLID 33487 - We never saved this change to data.
			// So when we changed the default value from Image to ANSI, people who have had
			// the old NSF setting had been hitting this code. The line must be drawn here.
			
			//see if they sent electronic claims before
			//(r.wilson 10/2/2012) plid 52970 - Replace hardcoded SendType with enumerated value (old value was 0)
			if(IsRecordsetEmpty("SELECT TOP 1 ID FROM ClaimHistoryT WHERE SendType = %li", ClaimSendType::Electronic)) {
				//if not, their default is ANSI
				FormatStyle = ANSI;
			}
			else {
				//if so, their default is IMAGE
				FormatStyle = IMAGE;
			}

			//now save this setting
			SetRemotePropertyInt("EbillingFormatType", FormatStyle, 0, "<None>");
		}

		ClaimType = GetRemotePropertyInt("EbillingClaimType",actProf,0,"<None>",TRUE);
		if(ClaimType == actProf)
			m_FormTypeCombo->TrySetSelByColumn(0,(long)1);
		else if(ClaimType == actInst)
			m_FormTypeCombo->TrySetSelByColumn(0,(long)2);

		//arrange the screen accordingly

		// (j.jones 2008-05-09 09:37) - PLID 29986 - completely removed NSF from the program
		if(FormatStyle == IMAGE) {
			m_ExportFormat->SetSelByColumn(0, (long)FormatStyle);
			PrepareImage();
		}
		else if(FormatStyle == OHIP) {
			// (j.jones 2006-11-09 08:29) - PLID 21570 - added support for OHIP
			m_ExportFormat->SetSelByColumn(0, (long)FormatStyle);
			PrepareOHIP();
		}
		else if(FormatStyle == ALBERTA) {
			// (j.jones 2010-07-09 14:37) - PLID 29971 - added support for Alberta HLINK
			m_ExportFormat->SetSelByColumn(0, (long)FormatStyle);
			PrepareAlberta();
		}
		else {
			//assume ANSI
			FormatStyle = ANSI;
			m_ExportFormat->SetSelByColumn(0, (long)FormatStyle);
			PrepareANSI();
		}

		ChangeFormats();

		if(m_ExportStyle->CurSel == -1) {
			FormatID = -1;
			if(FormatStyle == ANSI) {
				m_btnExportBatch.EnableWindow(FALSE);
			}
		}
		else {
			m_btnExportBatch.EnableWindow(TRUE);
		}

		CString strWhere;
		strWhere.Format("FormType = %li", ClaimType == actProf ? 1 : 2);

		//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
		// (j.jones 2009-03-16 16:55) - PLID 32321 - supported OHIP labeling,
		// turns out this was a bug all along when loading, as switching to
		// OHIP or Image always removed the claim type
		CString strLabel = "";
		// (j.jones 2010-07-09 15:00) - PLID 29971 - corrected to just be ANSI
		if(FormatStyle == ANSI) {
			strLabel = ClaimType == actProf ? "HCFA" : "UB";
		}
		UpdateLabels(strLabel);

		m_UnselectedBatchList->PutWhereClause(_bstr_t(strWhere));
		m_SelectedBatchList->PutWhereClause(_bstr_t(strWhere));

		COleVariant var;
		m_Counter = 0;

		int tempint = GetRemotePropertyInt(_T("EnvoyProduction"),0,0,_T("<None>"));
		if(tempint == 1)
			m_production.SetCheck(TRUE);
		else m_test.SetCheck(TRUE);
	}
	NxCatchAll("Error in batch E-Billing dialog");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CEbillingFormDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CNxDialog::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEbillingFormDlg::OnPaint() 
{
	CNxDialog::OnPaint();
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEbillingFormDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CEbillingFormDlg::OnExport() 
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(m_FormTypeCombo->GetCurSel() == -1)
		return;

	if(!CheckCurrentUserPermissions(bioEBilling,sptRead) || !g_pLicense->CheckForLicense(CLicense::lcEbill, CLicense::cflrUse)) 
		return;

	try {

		if(m_SelectedBatchList->GetRowCount() == 0) {
			AfxMessageBox("There are no claims in the selected list. Please batch some claims before exporting.");
			return;
		}

		bool bIsIntegrationEnabled = GetRemotePropertyInt("EbillingClearinghouseIntegration_Enabled", FALSE, 0, "<None>", true) ? true : false;

		//warns if auto-uploads are enabled and multiple SiteIDs exist, and the current claims require
		//separate exports
		//fills the siteID & password to use upon success
		NexTech_Accessor::_ClearinghouseLoginPtr pClearinghouseLogin = nullptr;
		if (FormatStyle == ANSI && bIsIntegrationEnabled)
		{
			if (!GetAutoUploadCredentialsForClaims(pClearinghouseLogin))
			{
				return;
			}
		}

		if(ClaimType == actInst && FormatStyle == IMAGE) {
			//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
			if(IDNO == MessageBox("The Image export style will only export claims in a HCFA Image format.\n"
				"Your current Form Type selection is including UB claims.\n\n"
				"Are you sure you wish to export these UB claims as HCFA Images?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		// (j.jones 2008-02-11 16:36) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
		if(GetRemotePropertyInt("DisallowBatchingPatientClaims",0,0,"<None>",TRUE) == 1) {
			//we have to check each bill
			for(int i=0; i<m_SelectedBatchList->GetRowCount(); i++) {
				long nBillID = VarLong(m_SelectedBatchList->GetValue(i, COLUMN_BILL_ID));
				// (j.jones 2010-10-21 14:48) - PLID 41051 - this function can warn why the claim can't be created,
				// but does not do so here, we will handle it ourselves
				if(!CanCreateInsuranceClaim(nBillID, TRUE)) {
					CString str;
					str.Format("The %s claim for patient %s can not be exported either because no charges on the bill are batched "
						"or because your preference to disallow batching or printing claims with no insurance responsibility has been enabled.\n\n"
						"Please unbatch this claim before exporting the batch.",
						FormatDateTimeForInterface(VarDateTime(m_SelectedBatchList->GetValue(i, COLUMN_BILL_DATE)), NULL, dtoDate),
						VarString(m_SelectedBatchList->GetValue(i, COLUMN_PATIENT_NAME)));
					AfxMessageBox(str);
					return;
				}
			}
		}

		// (j.jones 2009-09-01 08:41) - PLID 34749 - we now disallow opening claims without a group
		if(FormatStyle != ALBERTA && FormatStyle != OHIP && (ClaimType == actProf || ClaimType == actInst)) {
			for(int i=0; i<m_SelectedBatchList->GetRowCount(); i++) {

				long nInsuredPartyID = VarLong(m_SelectedBatchList->GetValue(i, COLUMN_INSURED_PARTY_ID), -1);

				if(ClaimType == actProf) {
					long nHCFAGroupID = -1;
					if(nInsuredPartyID != -1) {
						_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoT.HCFASetupGroupID FROM InsuranceCoT "
							"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
							"WHERE InsuredPartyT.PersonID = {INT}", nInsuredPartyID);
						if(!rs->eof) {
							nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);
						}
						rs->Close();
					}

					if(nHCFAGroupID == -1) {
						CString str;
						str.Format("The %s claim for patient %s has an insurance company that is not configured in a HCFA group.\n"
							"You can set up a HCFA group in the HCFA tab of the Administrator module.\n\n"
							"This HCFA cannot be opened until the insurance company has been properly set up.",
							FormatDateTimeForInterface(VarDateTime(m_SelectedBatchList->GetValue(i, COLUMN_BILL_DATE)), NULL, dtoDate),
							VarString(m_SelectedBatchList->GetValue(i, COLUMN_PATIENT_NAME)));
						MsgBox(str);
						return;
					}
				}
				else if(ClaimType == actInst) {
					long nUBGroupID = -1;
					if(nInsuredPartyID != -1) {
						_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoT.UB92SetupGroupID FROM InsuranceCoT "
							"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
							"WHERE InsuredPartyT.PersonID = {INT}", nInsuredPartyID);
						if(!rs->eof) {
							nUBGroupID = AdoFldLong(rs, "UB92SetupGroupID", -1);
						}
						rs->Close();
					}

					if(nUBGroupID == -1) {
						CString str;
						str.Format("The %s claim for patient %s has an insurance company that is not configured in a UB group.\n"
							"You can set up a UB group in the UB tab of the Administrator module.\n\n"
							"This claim cannot be opened until the insurance company has been properly set up.",
							FormatDateTimeForInterface(VarDateTime(m_SelectedBatchList->GetValue(i, COLUMN_BILL_DATE)), NULL, dtoDate),
							VarString(m_SelectedBatchList->GetValue(i, COLUMN_PATIENT_NAME)));
						MsgBox(str);
						return;
					}
				}
			}
		}

		// (j.jones 2007-03-01 08:57) - PLID 25015 - added preference to auto-validate claims before exporting
		if(GetRemotePropertyInt("AutoValidateBatchClaims", 1, 0, "<None>", true) == 1) {

			//Validate selected claims, find the result
			EEbillingValidationResults eevrResult = Validate(-2);
			if(eevrResult == eevrFailedOther) {
				//really this shouldn't happen, it would be from a check such as permissions
				//that we already would have done. But if so, return silently.
				return;
			}
			else if(eevrResult == eevrFailedError) {
				//if this is returned, we displayed a notepad of errors/warnings,
				//and thus should ask if the user wishes to cancel
				//(Yes to cancel, so yes-click-happy users will not continue exporting without thinking.)
				if(IDYES == MessageBox("The Claim Validation check has generated a list of potential problems to be reviewed.\n"
					"Errors will likely cause claim rejections; Warnings may or may not cause a rejection.\n\n"
					"Would you like to cancel exporting claims in order to correct these issues?","Practice",MB_YESNO|MB_ICONEXCLAMATION)) {
					return;
				}
			}
		}

		CEbilling dlg(this);
		_variant_t var;
		dlg.m_FormatID = FormatID;
		dlg.m_FormatStyle = FormatStyle;
		// (j.jones 2010-10-18 15:59) - PLID 40346 - changed boolean to an enum
		if(ClaimType == actInst)
			dlg.m_actClaimType = actInst;
		else
			dlg.m_actClaimType = actProf;

		dlg.m_nFilterByLocationID = VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0), -1);

		long nProviderID = -1;
		if (m_ProviderCombo->GetCurSel() != -1) {
			nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
		}
		dlg.m_nFilterByProviderID = nProviderID;

		// (j.jones 2009-08-14 11:49) - PLID 34914 - are they sending per-provider?
		// (j.dinatale 2012-12-27 11:58) - PLID 54136 - need to pass along which provider for Alberta
		if(nProviderID == -1 && ((FormatStyle == OHIP && GetRemotePropertyInt("OHIP_SeparateAccountsPerProviders", 0, 0, "<None>", TRUE) == 1) || FormatStyle == ALBERTA))
		{
			AfxMessageBox("You must select a provider to submit claims for.");
			return;
		}

		CString str = "";
		str = GetRemotePropertyText(_T("ResSubmitter"),"",0,_T("<None>"));
		if(str!="")
			dlg.m_Contact = str;
		str = GetRemotePropertyText(_T("SiteID"),"",0,_T("<None>"));
		if(str!="")
			dlg.m_SiteID = str;
		dlg.m_pClearinghouseLogin = pClearinghouseLogin;

		int nResult = dlg.DoModal();
		// (s.tullis 2014-08-08 10:16) - PLID 62780 - 
		if (UseAlbertaHLINK())
		{
			if (FormatStyle == ALBERTA){

				UpdateAlbertaBillstatus();
			}

			
		}
		
		UpdateView();

	} NxCatchAll("Error in EBillingFormDlg::OnExport");

} // end OnExport()

void CEbillingFormDlg::OnRemoveHCFA() 
{
	if(!CheckCurrentUserPermissions(bioEBilling,sptWrite)) return;

	if(m_CurrList==0) {
		if(m_UnselectedBatchList->GetCurSel()==-1) {
			AfxMessageBox("There is no claim selected.");
			return;
		}
	}
	else {
		if(m_SelectedBatchList->GetCurSel()==-1) {
			AfxMessageBox("There is no claim selected.");
			return;
		}
	}

	// to Remove a claim from the list!
	CString sql;
	// (b.eyers 2015-10-06) - PLID 42101
	long nBillID; 
	std::vector<long> aryBillID;
	try {	
		if (m_CurrList == 0) {
			nBillID = m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(), COLUMN_BILL_ID).lVal; // (b.eyers 2015-10-06) - PLID 42101
			sql.Format("DELETE FROM HCFATrackT WHERE ID = %li", m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(), COLUMN_HCFA_ID).lVal);
		}
		else {
			nBillID = m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(), COLUMN_BILL_ID).lVal; // (b.eyers 2015-10-06) - PLID 42101
			sql.Format("DELETE FROM HCFATrackT WHERE ID = %li", m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(), COLUMN_HCFA_ID).lVal);
		}
		ExecuteSql("%s", sql);

		// (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, electronic to unbatched
		aryBillID.push_back(nBillID);
		AuditInsuranceBatch(aryBillID, 0, 2);

		UpdateView();
	} NxCatchAll("Error in OnRemoveHCFA");

} // end Remove


void CEbillingFormDlg::OnReset()
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(!CheckCurrentUserPermissions(bioEBilling,sptWrite)) return;

	if (MessageBox("Are you sure you want to Unbatch all claims?","Question",MB_YESNO|MB_ICONQUESTION) == IDNO) return;

	try { 

		long nLocationID = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
		CString strWhere = "";
		if(nLocationID != -1) {
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			strWhere.Format(" AND BillID IN (SELECT BillID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemT.LocationID = %li)",nLocationID);
		}

		if(m_FormTypeCombo->GetCurSel() == -1)
			m_FormTypeCombo->CurSel = 1;

		CString strWhere2;
		strWhere2.Format(" AND BillID IN (SELECT ID FROM BillsT WHERE FormType = %li)",m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal);

		strWhere += strWhere2;

		long nProviderID = -1;
		if (m_ProviderCombo->GetCurSel() != -1) {
			nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
		}

		if (nProviderID != -1)
		{
			//filter on any bill that contains the selected provider, even if they may
			//also contain other providers
			CString strProviderWhere;

			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			strProviderWhere.Format("BillID IN (SELECT BillID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ChargesT.DoctorsProviders = %li)", nProviderID);

			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strProviderWhere;
		}

		// (j.jones 2014-07-09 10:26) - PLID 62568 - do not remove on hold claims
		ExecuteSql("DELETE FROM HCFATrackT WHERE Batch = 2 %s AND BillID IN ("
				"SELECT BillsT.ID FROM BillsT "
				"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
				"WHERE Coalesce(BillStatusT.Type, -1) != %li "
				")", strWhere, EBillStatusType::OnHold);

		// (b.eyers 2015-10-06) - PLID 42101 - mass audit insurance claim, electronic to unbatched
		_DNxDataListPtr pList, pList2;
		pList = m_UnselectedBatchList;
		pList2 = m_SelectedBatchList;
		std::vector<long> aryBillIDs;
		for (int i = 0; i < pList->GetRowCount(); i++) {
			long nBillID = VarLong(pList->GetValue(i, COLUMN_BILL_ID));
			aryBillIDs.push_back(nBillID);
		}
		for (int i = 0; i < pList2->GetRowCount(); i++) {
			long nBillID = VarLong(pList2->GetValue(i, COLUMN_BILL_ID));
			aryBillIDs.push_back(nBillID);
		}
		if (aryBillIDs.size() > 0)
			AuditInsuranceBatch(aryBillIDs, 0, 2);

		UpdateView();
	} NxCatchAll("Error clearing batched claims.");

} // end OnReset()

void CEbillingFormDlg::OnResetSelected() 
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(!CheckCurrentUserPermissions(bioEBilling,sptWrite)) return;

	if (MessageBox("Are you sure you want to Unbatch all selected claims?","Question",MB_YESNO|MB_ICONQUESTION) == IDNO) return;

	try { 

		long nLocationID = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
		CString strWhere = "";
		if(nLocationID != -1) {
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			strWhere.Format(" AND BillID IN (SELECT BillID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemT.LocationID = %li)",nLocationID);
		}

		if(m_FormTypeCombo->GetCurSel() == -1)
			m_FormTypeCombo->CurSel = 1;

		CString strWhere2;
		strWhere2.Format(" AND BillID IN (SELECT ID FROM BillsT WHERE FormType = %li)",m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal);

		strWhere += strWhere2;

		long nProviderID = -1;
		if (m_ProviderCombo->GetCurSel() != -1) {
			nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
		}

		if (nProviderID != -1)
		{
			//filter on any bill that contains the selected provider, even if they may
			//also contain other providers
			CString strProviderWhere;
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			strProviderWhere.Format("BillID IN (SELECT BillID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ChargesT.DoctorsProviders = %li)", nProviderID);

			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strProviderWhere;
		}

		// (j.jones 2014-07-09 10:26) - PLID 62568 - do not remove on hold claims
		ExecuteSql("DELETE FROM HCFATrackT WHERE Batch = 2 AND CurrentSelect = 1 %s "
			"AND BillID IN ("
			"SELECT BillsT.ID FROM BillsT "
			"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
			"WHERE Coalesce(BillStatusT.Type, -1) != %li "
			")", strWhere, EBillStatusType::OnHold);

		// (b.eyers 2015-10-06) - PLID 42101 - mass audit insurance claim, selected electronic to unbatched
		_DNxDataListPtr pList;
		pList = m_SelectedBatchList;
		std::vector<long> aryBillIDs;
		for (int i = 0; i < pList->GetRowCount(); i++) {
			long nBillID = VarLong(pList->GetValue(i, COLUMN_BILL_ID));
			aryBillIDs.push_back(nBillID);
		}
		if (aryBillIDs.size() > 0)
			AuditInsuranceBatch(aryBillIDs, 0, 2);

		UpdateView();
	} NxCatchAll("Error clearing selected batched claims.");
}

void CEbillingFormDlg::OnResetUnselected() 
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(!CheckCurrentUserPermissions(bioEBilling,sptWrite)) return;

	if (MessageBox("Are you sure you want to Unbatch all unselected claims?","Question",MB_YESNO|MB_ICONQUESTION) == IDNO) return;

	try {

		long nLocationID = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
		CString strWhere = "";
		if(nLocationID != -1) {
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			strWhere.Format(" AND BillID IN (SELECT BillID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemT.LocationID = %li)",nLocationID);
		}

		if(m_FormTypeCombo->GetCurSel() == -1)
			m_FormTypeCombo->CurSel = 1;

		CString strWhere2;
		strWhere2.Format(" AND BillID IN (SELECT ID FROM BillsT WHERE FormType = %li)",m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal);

		strWhere += strWhere2;

		long nProviderID = -1;
		if (m_ProviderCombo->GetCurSel() != -1) {
			nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
		}

		if (nProviderID != -1)
		{
			//filter on any bill that contains the selected provider, even if they may
			//also contain other providers
			CString strProviderWhere;
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			strProviderWhere.Format("BillID IN (SELECT BillID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ChargesT.DoctorsProviders = %li)", nProviderID);

			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strProviderWhere;
		}

		// (j.jones 2014-07-09 10:26) - PLID 62568 - do not remove on hold claims
		ExecuteSql("DELETE FROM HCFATrackT WHERE Batch = 2 AND CurrentSelect = 0 %s "
			"AND BillID IN ("
			"SELECT BillsT.ID FROM BillsT "
			"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
			"WHERE Coalesce(BillStatusT.Type, -1) != %li "
			")", strWhere, EBillStatusType::OnHold);

		// (b.eyers 2015-10-06) - PLID 42101 - mass audit insurance claim, unselected electronic to unbatched
		_DNxDataListPtr pList;
		pList = m_UnselectedBatchList;
		std::vector<long> aryBillIDs;
		for (int i = 0; i < pList->GetRowCount(); i++) {
			long nBillID = VarLong(pList->GetValue(i, COLUMN_BILL_ID));
			aryBillIDs.push_back(nBillID);
		}
		if (aryBillIDs.size() > 0)
			AuditInsuranceBatch(aryBillIDs, 0, 2);

		UpdateView();
	} NxCatchAll("Error clearing unselected batched claims.");
}


BEGIN_EVENTSINK_MAP(CEbillingFormDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEbillingFormDlg)
	ON_EVENT(CEbillingFormDlg, IDC_UNSELECTED_BATCHLIST, 6 /* RButtonDown */, OnRButtonDownUnselectedBatchlist, VTS_I4 VTS_I4 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEbillingFormDlg, IDC_UNSELECTED_BATCHLIST, 18 /* RequeryFinished */, OnRequeryFinishedUnselectedBatchlist, VTS_I2)
	ON_EVENT(CEbillingFormDlg, IDC_SELECTED_BATCHLIST, 6 /* RButtonDown */, OnRButtonDownSelectedBatchlist, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEbillingFormDlg, IDC_SELECTED_BATCHLIST, 18 /* RequeryFinished */, OnRequeryFinishedSelectedBatchlist, VTS_I2)
	ON_EVENT(CEbillingFormDlg, IDC_UNSELECTED_BATCHLIST, 3 /* DblClickCell */, OnDblClickCellUnselectedBatchlist, VTS_I4 VTS_I2)
	ON_EVENT(CEbillingFormDlg, IDC_SELECTED_BATCHLIST, 3 /* DblClickCell */, OnDblClickCellSelectedBatchlist, VTS_I4 VTS_I2)
	ON_EVENT(CEbillingFormDlg, IDC_EXPORTSTYLE, 16 /* SelChosen */, OnSelChosenExportstyle, VTS_I4)
	ON_EVENT(CEbillingFormDlg, IDC_LOCATION_COMBO_ELECTRONIC, 16 /* SelChosen */, OnSelChosenLocationComboElectronic, VTS_I4)
	ON_EVENT(CEbillingFormDlg, IDC_EBILLING_FORM_TYPE_COMBO, 16 /* SelChosen */, OnSelChosenEbillingFormTypeCombo, VTS_I4)
	ON_EVENT(CEbillingFormDlg, IDC_EXPORTFORMAT, 16 /* SelChosen */, OnSelChosenExportformat, VTS_I4)
	ON_EVENT(CEbillingFormDlg, IDC_EBILLING_PROVIDER_COMBO, 16, OnSelChosenEbillingProviderCombo, VTS_I4)
	//}}AFX_EVENTSINK_MAP	
END_EVENTSINK_MAP()

void CEbillingFormDlg::OnPrintList() 
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	//this will print a list of all electronically batched claims

	if(!CheckCurrentUserPermissions(bioEBilling,sptRead)) return;
	// will print the list of batched claims..
	CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(230)]);

	CString strWhere;
	long nLocationID = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
	if(nLocationID != -1) {
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		strWhere.Format(" BillID IN (SELECT BillID FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND LineItemT.LocationID = %li)",nLocationID);
	}

	if(m_FormTypeCombo->GetCurSel() == -1)
		m_FormTypeCombo->CurSel = 1;

	CString strWhere2;
	strWhere2.Format("FormType = %li",m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal);

	if(!strWhere.IsEmpty())
		strWhere += " AND ";

	strWhere += strWhere2;

	long nProviderID = -1;
	if (m_ProviderCombo->GetCurSel() != -1) {
		nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
	}

	if (nProviderID != -1)
	{
		//filter on any bill that contains the selected provider, even if they may
		//also contain other providers
		CString strProviderWhere;
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		strProviderWhere.Format("BillID IN (SELECT BillID FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND ChargesT.DoctorsProviders = %li)", nProviderID);

		if(!strWhere.IsEmpty()) {
			strWhere += " AND ";
		}
		strWhere += strProviderWhere;
	}

	//this prompts the report to only show claims where CurrentSelect = TRUE, and the right form style
	infReport.SetExtraValue(strWhere);	

	//Made new function for running reports - JMM 5-28-04
	RunReport(&infReport, true, (CWnd *)this, "Electronic Claim Batch List");
	
}

void CEbillingFormDlg::OnPrintSelectedList() 
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	//this will print a list of only the selected claims

	if(!CheckCurrentUserPermissions(bioEBilling,sptRead)) return;

	if(m_SelectedBatchList->GetRowCount()==0) {
		AfxMessageBox("The are no claims selected.");
		return;
	}

	// will print the list of batched HCFA's..
	CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(230)]);

	CString strWhere;
	CString str;
	str.Format(" {HCFATrackT.CurrentSelect} = 1 ");
	strWhere += str;
	
	long nLocationID = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
	if(nLocationID != -1) {
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		str.Format(" AND BillID IN (SELECT BillID FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND LineItemT.LocationID = %li)",nLocationID);
		strWhere += str;
	}

	if(m_FormTypeCombo->GetCurSel() == -1)
		m_FormTypeCombo->CurSel = 1;

	CString strWhere2;
	strWhere2.Format("FormType = %li",m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal);

	if(!strWhere.IsEmpty())
		strWhere += " AND ";

	strWhere += strWhere2;

	long nProviderID = -1;
	if (m_ProviderCombo->GetCurSel() != -1) {
		nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
	}

	if (nProviderID != -1)
	{
		//filter on any bill that contains the selected provider, even if they may
		//also contain other providers
		CString strProviderWhere;
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		strProviderWhere.Format("BillID IN (SELECT BillID FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND ChargesT.DoctorsProviders = %li)", nProviderID);

		if(!strWhere.IsEmpty()) {
			strWhere += " AND ";
		}
		strWhere += strProviderWhere;
	}

	//this prompts the report to only show claims where CurrentSelect = TRUE, and the right form style
	infReport.SetExtraValue(strWhere);
	
	//Made new function for running reports - JMM 5-28-04
	RunReport(&infReport, true, (CWnd *)this, "Electronic Claim Batch List");
	
}

void CEbillingFormDlg::OnPrintUnselectedList() 
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	//this will print a list of only the unselected claims

	if(!CheckCurrentUserPermissions(bioEBilling,sptRead)) return;

	if(m_UnselectedBatchList->GetRowCount()==0) {
		AfxMessageBox("The are no claims selected.");
		return;
	}

	// will print the list of batched claims..
	CReportInfo  infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(230)]);

	CString strWhere;
	CString str;
	str.Format(" {HCFATrackT.CurrentSelect} = 0 ");
	strWhere += str;
	
	long nLocationID = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
	if(nLocationID != -1) {
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		str.Format(" AND BillID IN (SELECT BillID FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND LineItemT.LocationID = %li)",nLocationID);
		strWhere += str;
	}

	if(m_FormTypeCombo->GetCurSel() == -1)
		m_FormTypeCombo->CurSel = 1;

	CString strWhere2;
	strWhere2.Format("FormType = %li",m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal);

	if(!strWhere.IsEmpty())
		strWhere += " AND ";

	strWhere += strWhere2;

	long nProviderID = -1;
	if (m_ProviderCombo->GetCurSel() != -1) {
		nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
	}

	if (nProviderID != -1)
	{
		//filter on any bill that contains the selected provider, even if they may
		//also contain other providers
		CString strProviderWhere;
		// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
		strProviderWhere.Format("BillID IN (SELECT BillID FROM ChargesT "
			"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND ChargesT.DoctorsProviders = %li)", nProviderID);

		if(!strWhere.IsEmpty()) {
			strWhere += " AND ";
		}
		strWhere += strProviderWhere;
	}

	//this prompts the report to only show claims where CurrentSelect = TRUE, and the right form style
	infReport.SetExtraValue(strWhere);
	//Made new function for running reports - JMM 5-28-04
	RunReport(&infReport, true, (CWnd *)this, "Electronic Claim Batch List");
}

void CEbillingFormDlg::OnOpenHCFA() 
{
	COleVariant var;
	CString HCFAID, StrSQL;
//	HCFADlg dlg;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptRead)) {
		return;
	}

	//CString str;
	long BillID, PatientID;

	if(m_CurrList==0) {
		var = m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(),COLUMN_HCFA_ID);
		PatientID = m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(),COLUMN_PATIENT_ID).lVal;
		BillID = m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(),COLUMN_BILL_ID).lVal;
	}
	else {
		var = m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(),COLUMN_HCFA_ID);
		PatientID = m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(),COLUMN_PATIENT_ID).lVal;
		BillID = m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(),COLUMN_BILL_ID).lVal;
	}

	if (var.vt != 0) {

		// (j.jones 2008-02-11 16:36) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
		// (j.jones 2010-10-21 14:48) - PLID 41051 - this function now warns why the claim can't be created
		if(!CanCreateInsuranceClaim(BillID, FALSE)) {
			return;
		}

		// (j.jones 2005-10-31 15:34) - PLID 17787 - we shouldn't have non-HCFA/UB92 forms in this tab,
		// but since claims can be converted to the nonstandard forms, we need to support opening them

		long nFormType = 1;
		long nInsuredPartyID = -1;

		_RecordsetPtr rs = CreateParamRecordset("SELECT FormType, InsuredPartyID FROM BillsT WHERE ID = {INT}",BillID);
		if(!rs->eof) {
			nFormType = AdoFldLong(rs, "FormType",1);
			nInsuredPartyID = AdoFldLong(rs, "InsuredPartyID", -1);
		}	
		rs->Close();

		switch(nFormType) {

			case 1: {

				// (j.jones 2009-09-01 08:41) - PLID 34749 - we now disallow opening HCFAs without a HCFA group
				long nHCFAGroupID = -1;
				if(nInsuredPartyID != -1) {
					_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoT.HCFASetupGroupID FROM InsuranceCoT "
						"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
						"WHERE InsuredPartyT.PersonID = {INT}", nInsuredPartyID);
					if(!rs->eof) {
						nHCFAGroupID = AdoFldLong(rs, "HCFASetupGroupID", -1);
					}
					rs->Close();
				}

				if(nHCFAGroupID == -1) {
					MsgBox("The selected insurance company is not configured in a HCFA group.\n"
						"You can set up a HCFA group in the HCFA tab of the Administrator module.\n\n"
						"This HCFA cannot be opened until the insurance company has been properly set up.");
					return;
				}

				CHCFADlg hcfa(this);
				hcfa.m_PatientID = PatientID;
				hcfa.DoModal(BillID);
				break;
			}

			case 2: {

				// (j.jones 2009-09-01 08:41) - PLID 34749 - we now disallow opening UBs without a UB group
				long nUBGroupID = -1;
				if(nInsuredPartyID != -1) {
					_RecordsetPtr rs = CreateParamRecordset("SELECT InsuranceCoT.UB92SetupGroupID FROM InsuranceCoT "
						"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
						"WHERE InsuredPartyT.PersonID = {INT}", nInsuredPartyID);
					if(!rs->eof) {
						nUBGroupID = AdoFldLong(rs, "UB92SetupGroupID", -1);
					}
					rs->Close();
				}

				if(nUBGroupID == -1) {
					MsgBox("The selected insurance company is not configured in a UB group.\n"
						"You can set up a UB group in the UB tab of the Administrator module.\n\n"
						"This claim cannot be opened until the insurance company has been properly set up.");
					return;
				}

				//TES 3/13/2007 - PLID 24993 - Use the correct form.
				if(GetUBFormType() == eUB04) {//UB04
					CUB04Dlg dlg(this);
					dlg.m_PatientID = PatientID;
					dlg.DoModal(BillID);
				}
				else {
					CUB92Dlg dlg(this);
					dlg.m_PatientID = PatientID;
					dlg.DoModal(BillID);
				}
				break;
			}

			case 3: {
				// (j.armen 2014-03-05 09:17) - PLID 60784 - Fill BillID, PatientID in constructor
				CADADlg ada(this, BillID, PatientID);
				ada.DoModal();
				break;
			}

			case 4: {
				CIDPADlg idpa(this);

				idpa.m_PatientID = PatientID;
				idpa.DoModal(BillID);
				break;
			}

			case 5: {
				CNYWCDlg nywc(this);

				nywc.m_PatientID = PatientID;
				nywc.DoModal(BillID);
				break;
			}

			case 6: {

				// (j.jones 2007-05-09 10:25) - PLID 25550 - check the internal preference
				// for which MICR form to use

				if(GetRemotePropertyInt("Use2007MICR", 1, 0, "<None>", true) == 1) {

					CMICR2007Dlg micr(this);

					micr.m_PatientID = PatientID;
					micr.DoModal(BillID);
				}
				else {

					CMICRDlg micr(this);

					micr.m_PatientID = PatientID;
					micr.DoModal(BillID);
				}
				break;
			}

			case 7: {

				// (j.jones 2010-02-04 11:51) - PLID 37113 - supported the NY Medicaid form
				CNYMedicaidDlg nymcaid(this);
				nymcaid.m_PatientID = PatientID;
				nymcaid.DoModal(BillID);
				break;
			}
		}
	}

	if(m_CurrList==0) {
		m_UnselectedBatchList->Requery();
	}
	else {
		m_SelectedBatchList->Requery();
	}

} // end OnOpenHCFA


void CEbillingFormDlg::OnStyleChange(long iNewRow) 
{

	try {

		ChangeFormats();

		OnSelChosenExportstyle(m_ExportStyle->CurSel);

		OnSelChosenEbillingFormTypeCombo(m_FormTypeCombo->CurSel);
	
	} NxCatchAll("Error changing export style.");

} // end OnStyleChange

void CEbillingFormDlg::ChangeFormats()
{
	switch(FormatStyle) {
		// (j.jones 2010-07-09 14:37) - PLID 29971 - added support for Alberta HLINK
		case ALBERTA:
			GetDlgItem(IDC_EXPORTSTYLE)->ShowWindow(SW_HIDE);
			//hide the ANSI buttons
			m_btnFormat997.EnableWindow(FALSE);
			m_btnFormat277.EnableWindow(FALSE);
			m_btnFormat997.ShowWindow(SW_HIDE);
			m_btnFormat277.ShowWindow(SW_HIDE);
			//hide the OHIP Report Manager button
			m_btnOHIPReportManager.EnableWindow(FALSE);
			m_btnOHIPReportManager.ShowWindow(SW_HIDE);
			//hide the OHIP dialer button
			m_btnOHIPDialerSetup.EnableWindow(FALSE);
			m_btnOHIPDialerSetup.ShowWindow(SW_HIDE);
			//hide the download reports button
			m_btnDownloadReports.EnableWindow(FALSE);
			m_btnDownloadReports.ShowWindow(SW_HIDE);
			//hide the test/production options
			m_production.EnableWindow(FALSE);
			m_test.EnableWindow(FALSE);
			m_production.ShowWindow(SW_HIDE);
			m_test.ShowWindow(SW_HIDE);
			m_Config.SetWindowText("Alberta HLINK Properties");
			//hide the clearinghouse integration button
			EnsureClearinghouseIntegrationControls();
			// (d.singleton 2011-07-05) - PLID 44422 - added button to open file explorer to choose what error report to parse
			m_btnAlbertaAssessmentFile.EnableWindow(TRUE);
			m_btnAlbertaAssessmentFile.ShowWindow(SW_SHOW);
			UpdateLabels("");
			break;

		case OHIP:
			GetDlgItem(IDC_EXPORTSTYLE)->ShowWindow(SW_HIDE);
			// (j.jones 2006-12-11 17:18) - PLID 23811 - disabled validation config for OHIP
			// (j.jones 2008-12-03 13:59) - PLID 32258 - re-enabled validation config for OHIP,
			// which means it always shows now
			//GetDlgItem(IDC_BTN_CONFIGURE_CLAIM_VALIDATION)->ShowWindow(SW_HIDE);
			m_btnFormat997.EnableWindow(FALSE);
			m_btnFormat277.EnableWindow(FALSE);
			m_btnFormat997.ShowWindow(SW_HIDE);
			m_btnFormat277.ShowWindow(SW_HIDE);
			// (j.jones 2008-07-02 15:42) - PLID 30604 - added OHIP batch edit button
			//m_btnFormatOHIPBatchEdit.EnableWindow(TRUE);			
			//m_btnFormatOHIPBatchEdit.ShowWindow(SW_SHOW);
			// (j.jones 2008-07-02 15:42) - PLID 21968 - added OHIP claim error button
			//m_btnFormatOHIPClaimsError.EnableWindow(TRUE);
			//m_btnFormatOHIPClaimsError.ShowWindow(SW_SHOW);
			// (j.jones 2008-12-17 09:45) - PLID 31900 - replaced the batch edit and claim error buttons
			// with the OHIP Report Manager button
			m_btnOHIPReportManager.EnableWindow(TRUE);
			m_btnOHIPReportManager.ShowWindow(SW_SHOW);
			// (j.jones 2009-03-09 15:53) - PLID 32405 - show the OHIP dialer button
			m_btnOHIPDialerSetup.EnableWindow(TRUE);
			m_btnOHIPDialerSetup.ShowWindow(SW_SHOW);		
			// (j.jones 2009-03-09 15:56) - PLID 33419 - show the download reports button
			m_btnDownloadReports.EnableWindow(TRUE);
			m_btnDownloadReports.ShowWindow(SW_SHOW);
			// (j.jones 2009-03-09 15:56) - PLID 33419 - hide the test/production options
			m_production.EnableWindow(FALSE);
			m_test.EnableWindow(FALSE);
			m_production.ShowWindow(SW_HIDE);
			m_test.ShowWindow(SW_HIDE);
			// (d.singleton 2011-07-05) - PLID 44422 - added button to open file explorer to choose what error report to parse
			m_btnAlbertaAssessmentFile.EnableWindow(FALSE);
			m_btnAlbertaAssessmentFile.ShowWindow(SW_HIDE);
			m_Config.SetWindowText("OHIP Properties");
			UpdateLabels("");
			break;
		case ANSI:
			GetDlgItem(IDC_EXPORTSTYLE)->ShowWindow(SW_SHOW);
			//GetDlgItem(IDC_BTN_CONFIGURE_CLAIM_VALIDATION)->ShowWindow(SW_SHOW);
			m_btnFormat997.ShowWindow(SW_SHOW);
			m_btnFormat277.ShowWindow(SW_SHOW);
			m_btnFormat997.EnableWindow(TRUE);
			m_btnFormat277.EnableWindow(TRUE);
			// (j.jones 2008-07-02 15:42) - PLID 30604 - added OHIP batch edit button
			//m_btnFormatOHIPBatchEdit.EnableWindow(FALSE);			
			//m_btnFormatOHIPBatchEdit.ShowWindow(SW_HIDE);
			// (j.jones 2008-07-02 15:42) - PLID 21968 - added OHIP claim error button
			//m_btnFormatOHIPClaimsError.EnableWindow(FALSE);
			//m_btnFormatOHIPClaimsError.ShowWindow(SW_HIDE);
			// (j.jones 2008-12-17 09:45) - PLID 31900 - replaced the batch edit and claim error buttons
			// with the OHIP Report Manager button
			m_btnOHIPReportManager.EnableWindow(FALSE);
			m_btnOHIPReportManager.ShowWindow(SW_HIDE);
			// (j.jones 2009-03-09 15:53) - PLID 32405 - hide the OHIP dialer button
			m_btnOHIPDialerSetup.EnableWindow(FALSE);
			m_btnOHIPDialerSetup.ShowWindow(SW_HIDE);
			// (j.jones 2009-03-09 15:56) - PLID 33419 - hide the download reports button
			m_btnDownloadReports.EnableWindow(FALSE);
			m_btnDownloadReports.ShowWindow(SW_HIDE);
			// (j.jones 2009-03-09 15:56) - PLID 33419 - show the test/production options
			m_production.EnableWindow(TRUE);
			m_test.EnableWindow(TRUE);
			m_production.ShowWindow(SW_SHOW);
			m_test.ShowWindow(SW_SHOW);
			// (d.singleton 2011-07-05) - PLID 44422 - added button to open file explorer to choose what error report to parse
			m_btnAlbertaAssessmentFile.EnableWindow(FALSE);
			m_btnAlbertaAssessmentFile.ShowWindow(SW_HIDE);
			m_Config.SetWindowText("ANSI Properties");
			break;
		// (j.jones 2008-05-09 09:53) - PLID 29986 - made IMAGE be the default
		case IMAGE:
		default:
			GetDlgItem(IDC_EXPORTSTYLE)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_BTN_CONFIGURE_CLAIM_VALIDATION)->ShowWindow(SW_SHOW);
			m_btnFormat997.EnableWindow(FALSE);
			m_btnFormat277.EnableWindow(FALSE);
			m_btnFormat997.ShowWindow(SW_HIDE);
			m_btnFormat277.ShowWindow(SW_HIDE);
			// (j.jones 2008-07-02 15:42) - PLID 30604 - added OHIP batch edit button
			//m_btnFormatOHIPBatchEdit.EnableWindow(FALSE);			
			//m_btnFormatOHIPBatchEdit.ShowWindow(SW_HIDE);
			// (j.jones 2008-07-02 15:42) - PLID 21968 - added OHIP claim error button
			//m_btnFormatOHIPClaimsError.EnableWindow(FALSE);
			//m_btnFormatOHIPClaimsError.ShowWindow(SW_HIDE);
			// (j.jones 2008-12-17 09:45) - PLID 31900 - replaced the batch edit and claim error buttons
			// with the OHIP Report Manager button
			m_btnOHIPReportManager.EnableWindow(FALSE);
			m_btnOHIPReportManager.ShowWindow(SW_HIDE);
			// (j.jones 2009-03-09 15:53) - PLID 32405 - hide the OHIP dialer button
			m_btnOHIPDialerSetup.EnableWindow(FALSE);
			m_btnOHIPDialerSetup.ShowWindow(SW_HIDE);
			// (j.jones 2009-03-09 15:56) - PLID 33419 - hide the download reports button
			m_btnDownloadReports.EnableWindow(FALSE);
			m_btnDownloadReports.ShowWindow(SW_HIDE);
			// (j.jones 2009-03-09 15:56) - PLID 33419 - hide the test/production options
			m_production.EnableWindow(FALSE);
			m_test.EnableWindow(FALSE);
			m_production.ShowWindow(SW_HIDE);
			m_test.ShowWindow(SW_HIDE);
			// (d.singleton 2011-07-05) - PLID 44422 - added button to open file explorer to choose what error report to parse
			m_btnAlbertaAssessmentFile.EnableWindow(FALSE);
			m_btnAlbertaAssessmentFile.ShowWindow(SW_HIDE);
			m_Config.SetWindowText("Image Properties");
			UpdateLabels("");
			break;
	}

	EnsureClearinghouseIntegrationControls();

	if(m_ExportStyle->SetSelByColumn(0,(long)FormatID) == -1) {
		m_ExportStyle->CurSel = 0;
	}

	//some formats don't allow the "all" option for providers
	{
		bool bNeedsAllProvidersOption = true;
		if ((FormatStyle == OHIP && GetRemotePropertyInt("OHIP_SeparateAccountsPerProviders", 0, 0, "<None>", TRUE) == 1) || FormatStyle == ALBERTA)
		{
			bNeedsAllProvidersOption = false;
		}

		bool bSelectionChanged = false;

		long nAllRow = m_ProviderCombo->FindByColumn(pccID, (long)-1, 0, VARIANT_FALSE);
		if (bNeedsAllProvidersOption && nAllRow == -1) 
		{
			//create this row
			IRowSettingsPtr pAllRow = m_ProviderCombo->GetRow(-1);
			pAllRow->PutValue(pccID, (long)-1);
			pAllRow->PutValue(pccName, _bstr_t(" <All Providers>"));
			long nAllRow = m_ProviderCombo->InsertRow(pAllRow, 0);
			if (m_ProviderCombo->GetCurSel() == -1)
			{
				m_ProviderCombo->PutCurSel(nAllRow);
			}
		}
		else if (!bNeedsAllProvidersOption && nAllRow != -1)
		{
			//remove this row
			
			long nCurProviderID = -1;
			if (m_ProviderCombo->GetCurSel() != -1) {
				long nCurProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
				if (nCurProviderID == -1) {
					bSelectionChanged = true;
				}
			}
			m_ProviderCombo->RemoveRow(nAllRow);
			if (bSelectionChanged) {
				m_ProviderCombo->PutCurSel(0);
		UpdateView();
	}
		}
	}
}

void CEbillingFormDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	try
	{
		EnsureClearinghouseIntegrationControls();
	}
	NxCatchAll(__FUNCTION__);
	CNxDialog::OnShowWindow(bShow, nStatus);
}

void CEbillingFormDlg::EnsureClearinghouseIntegrationControls()
{
	if (FormatStyle == ANSI)
	{
		m_btnClearinghouseIntegration.ShowWindow(SW_SHOW);
		m_btnClearinghouseIntegration.EnableWindow(TRUE);
		TryShowTrizettoWebButton();
	}
	else
	{
		m_btnClearinghouseIntegration.ShowWindow(SW_HIDE);
		m_btnClearinghouseIntegration.EnableWindow(FALSE);
		m_btnLaunchTrizettoWebsite.ShowWindow(SW_HIDE);
		m_btnLaunchTrizettoWebsite.EnableWindow(FALSE);
	}
}

void CEbillingFormDlg::SendToPaper()
{
	try {

		CString sql;
		if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) {
			return;
		}

		if(!CheckCurrentUserPermissions(bioEBilling,sptWrite)) {
			return;
		}

		// (j.jones 2008-02-11 16:48) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
		// (but you can unbatch claims)
		long nBillID = -1;
		if(m_CurrList==0) {
			nBillID = VarLong(m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(),COLUMN_BILL_ID));
		}
		else {
			nBillID = VarLong(m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(),COLUMN_BILL_ID));
		}

		// (j.jones 2010-10-21 14:48) - PLID 41051 - this function now warns why the claim can't be created
		if(!CanCreateInsuranceClaim(nBillID, FALSE)) {
			return;
		}

		// (j.jones 2006-11-16 11:34) - PLID 23558 - ensured batching to paper uses the correct batch for all form types

		if(m_CurrList==0)
			sql.Format("UPDATE HCFATrackT SET Batch = 1, CurrentSelect = 0 WHERE ID = %li", m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(),COLUMN_HCFA_ID).lVal);
		else
			sql.Format("UPDATE HCFATrackT SET Batch = 1, CurrentSelect = 0 WHERE ID = %li", m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(),COLUMN_HCFA_ID).lVal);

		ExecuteSql("%s", sql);
			
		// (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, electronic to paper
		std::vector<long> aryBillID;
		aryBillID.push_back(nBillID);
		AuditInsuranceBatch(aryBillID, 1, 2);

		if(m_CurrList==0) {
			m_UnselectedBatchList->Requery();
		}
		else {
			m_SelectedBatchList->Requery();
		}
	} NxCatchAll("Error in SendToPaper()");

	RefreshTotals();
}

void CEbillingFormDlg::OnConfig() 
{
	if(!CheckCurrentUserPermissions(bioEBilling,sptWrite)) return;

	// (j.jones 2008-05-09 09:54) - PLID 29986 - removed NSF from the program

	// (j.jones 2010-10-12 14:47) - PLID 40901 - added support for Alberta HLINK
	if(FormatStyle == ALBERTA) {
		CAlbertaHLINKSetupDlg dlg(this);
		
		// (j.jones 2010-11-08 14:04) - PLID 39620 - handle when the health number changed
		if(dlg.DoModal() == IDOK && dlg.m_bHealthNumberCustomFieldChanged
			&& UseAlbertaHLINK()) {
				
			//if the custom field changed, tell the patient toolbar
			//that we need to show a new column for the health card number
			GetMainFrame()->m_patToolBar.PopulateColumns();
			GetMainFrame()->m_patToolBar.Requery();

			//also requery the scheduler patient list
			CSchedulerView* pView = (CSchedulerView*)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
			if(pView && pView->GetResEntry()) {
				pView->GetResEntry()->RequeryPersons();
			}
		}
	}
	else if(FormatStyle == ANSI) {
		CANSIProperties dlg(this);
		dlg.m_FormatID = FormatID;
		dlg.DoModal();

		m_ExportStyle->Requery();
		OnSelChosenExportstyle(m_ExportStyle->SetSelByColumn(0,(long)FormatID));
	}
	else if(FormatStyle == IMAGE) {
		CHCFAImageSetupDlg dlg(this);
		dlg.DoModal();
	}
	else if(FormatStyle == OHIP) {
		// (j.jones 2006-11-09 08:29) - PLID 21570 - added support for OHIP
		// (j.jones 2010-11-08 13:59) - PLID 39620 - same for Alberta
		COHIPSetupDlg dlg(this);
		if(dlg.DoModal() == IDOK && dlg.m_bHealthNumberCustomFieldChanged
			&& UseOHIP()) {
			// (j.jones 2010-05-04 11:44) - PLID 32325 - if the custom field changed,
			// tell the patient toolbar that we need to show a new column for the health card number
			GetMainFrame()->m_patToolBar.PopulateColumns();
			GetMainFrame()->m_patToolBar.Requery();

			//also requery the scheduler patient list
			CSchedulerView* pView = (CSchedulerView*)GetMainFrame()->GetOpenView(SCHEDULER_MODULE_NAME);
			if(pView && pView->GetResEntry()) {
				pView->GetResEntry()->RequeryPersons();
			}
		}
	}
}

void CEbillingFormDlg::OnRButtonDownUnselectedBatchlist(long nRow, long nCol, long x, long y, long nFlags) 
{
	// Do nothing if the user didn't actually click in a cell
	if (nRow == -1 || nCol == -1) {
		return;
	}

	CPoint pt;
	GetCursorPos(&pt);

	m_CurrList = 0;

	//first clear out the multi-selection, if one exists
	m_UnselectedBatchList->PutCurSel(-1);
	m_UnselectedBatchList->PutCurSel(nRow);

	if (m_UnselectedBatchList->GetValue(nRow,COLUMN_HCFA_ID).vt != VT_EMPTY) {

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;

		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SEND_TO_HCFA, "&Send to Paper Batch");
		if(GetRemotePropertyInt("Claim_AllowSentWithoutPrinting", 0, 0, "<None>", false) == 1){
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_MARK_CLAIM_SENT_UNSELECTED,"&Mark As Sent"); //(r.wilson 10/8/2012) plid 40712
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_MARK_ALL_CLAIMS_SENT_UNSELECTED,"Mar&k All As Sent"); //(r.wilson 10/8/2012) plid 40712
		}
		//TES 1/13/2009 - PLID 24138 - Added an option to send the entire list to the Paper Batch
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SEND_LIST_TO_HCFA, "Send &All Unselected Claims To Paper Batch");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_OPEN_HCFA, "&Open");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_VALIDATE_CLAIM, "&Validate Claim");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_REMOVE_HCFA, "&Remove from Batch");

		// (b.spivey, August 27th, 2014) - PLID 63492 - preview the claim file. 
		_RecordsetPtr rs = CreateParamRecordset("SELECT ANSIVersion FROM EbillingFormatsT WHERE ID = {INT}", FormatID);
		if (FormatStyle == ANSI && ((ANSIVersion)AdoFldLong(rs, "ANSIVersion", (long)av5010) == av5010)) {
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_PREVIEW_CLAIM_FILE_UNSELECTED, "Preview Claim File");
		}

		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_INSCO, "Select Claims with this &Insurance Co.");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_INSGROUP, "Select Claims with this Insurance Gro&up");		
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_PROV, "Select Claims with this &Provider");
		// (j.jones 2012-03-26 10:43) - PLID 48854 - added ability to select by claim provider
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_CLAIM_PROV, "Select Claims with this &Claim Provider");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_GOTOPATIENT, "&Go To Patient");

		mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}
}

void CEbillingFormDlg::OnRButtonDownSelectedBatchlist(long nRow, short nCol, long x, long y, long nFlags) 
{
	// Do nothing if the user didn't actually click in a cell
	if (nRow == -1 || nCol == -1) {
		return;
	}

	CPoint pt;
	GetCursorPos(&pt);

	m_CurrList = 1;

	//first clear out the multi-selection, if one exists
	m_SelectedBatchList->PutCurSel(-1);
	m_SelectedBatchList->PutCurSel(nRow);

	if (m_SelectedBatchList->GetValue(nRow,COLUMN_HCFA_ID).vt != VT_EMPTY) {

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;

		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SEND_TO_HCFA, "&Send to Paper Batch");
		if(GetRemotePropertyInt("Claim_AllowSentWithoutPrinting", 0, 0, "<None>", false) == 1){
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_MARK_CLAIM_SENT_SELECTED,"&Mark As Sent"); //(r.wilson 10/8/2012) plid 40712
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_MARK_ALL_CLAIMS_SENT_SELECTED,"Mar&k All As Sent"); //(r.wilson 10/8/2012) plid 40712
		}
		//TES 1/13/2009 - PLID 24138 - Added an option to send the entire list to the Paper Batch
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SEND_LIST_TO_HCFA, "Send &All Selected Claims To Paper Batch");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_OPEN_HCFA, "&Open");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_VALIDATE_CLAIM, "&Validate Claim");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_REMOVE_HCFA, "&Remove from Batch");

		// (b.spivey, August 27th, 2014) - PLID 63492 - preview the claim file. 
		_RecordsetPtr rs = CreateParamRecordset("SELECT ANSIVersion FROM EbillingFormatsT WHERE ID = {INT}", FormatID);
		if (FormatStyle == ANSI && ((ANSIVersion)AdoFldLong(rs, "ANSIVersion", (long)av5010) == av5010)) {
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_PREVIEW_CLAIM_FILE_SELECTED, "Preview Claim File");
		}

		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_INSCO, "Unselect Claims with this &Insurance Co.");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_INSGROUP, "Unselect Claims with this Insurance Gro&up");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_PROV, "Unselect Claims with this &Provider");
		// (j.jones 2012-03-26 10:43) - PLID 48854 - added ability to select by claim provider
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_CLAIM_PROV, "Unselect Claims with this &Claim Provider");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_GOTOPATIENT, "&Go To Patient");

		mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}	
}

void CEbillingFormDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {

		// (j.jones 2010-07-23 16:15) - PLID 34105 - if HCFA and not OHIP,
		// warn if any setup can have assignment of benefits not filled
		// (this does not account for individual bills with Box 13 overridden)
		BOOL bShowWarning = TRUE;

		// (j.jones 2010-07-27 09:26) - PLID 39854 - this is now supported on UBs as well
		BOOL bIsHCFA = (m_FormTypeCombo->CurSel != -1 && VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->CurSel,0),-1) == 1);
		BOOL bIsUB = (m_FormTypeCombo->CurSel != -1 && VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->CurSel,0),-1) == 2);

		// (j.jones 2016-05-19 10:22) - NX-100685 - load the format type, it may have changed
		long nFormatID = GetDefaultEbillingANSIFormatID();
		m_ExportStyle->SetSelByColumn(0, (long)nFormatID);

		// (j.jones 2010-11-08 13:58) - PLID 39620 - skip if Alberta
		if(UseOHIP() || UseAlbertaHLINK() || (!bIsHCFA && !bIsUB)) {
			bShowWarning = FALSE;
		}
		// (j.jones 2010-07-27 09:28) - PLID 39854 - now we tell the function which form type to check
		if(bShowWarning && CanAssignmentOfBenefitsBeBlank(bIsHCFA ? babtHCFA : babtUB)) {
			m_nxstaticAssignmentOfBenefitsWarningLabel.ShowWindow(SW_SHOW);
			m_btnAssignmentOfBenefitsWarning.ShowWindow(SW_SHOW);
		}
		else {
			m_nxstaticAssignmentOfBenefitsWarningLabel.ShowWindow(SW_HIDE);
			m_btnAssignmentOfBenefitsWarning.ShowWindow(SW_HIDE);
		}
		
		//requery if the tablechecker changed or if it hasn't been requeried yet
		if(m_providerChecker.Changed() || m_ProviderCombo->GetRowCount() == 0) {

			_variant_t varCurSel = g_cvarNull;
			if(m_ProviderCombo->GetCurSel() != -1) {
				varCurSel = m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID);
			}

			//always requery, the login information may have changed
			m_ProviderCombo->Requery();

			int nRow = -1;
			if(varCurSel.vt != VT_NULL) {
				nRow = m_ProviderCombo->SetSelByColumn(pccID, varCurSel);
			}

			if(nRow == -1) {
				//set the first selection
				if(m_ProviderCombo->GetRowCount() > 0) {
					m_ProviderCombo->PutCurSel(0);
				}
			}
		}

		RefilterLists();

	}NxCatchAll("Error in CEbillingFormDlg::UpdateView");
}

void CEbillingFormDlg::OnRequeryFinishedUnselectedBatchlist(short nFlags) 
{
	for(int i=0;i<m_UnselectedBatchList->GetRowCount();i++) {

		//update the validation icon
		int v = m_UnselectedBatchList->GetValue(i,COLUMN_VALIDATION_STATE_VALUE).iVal;
		switch(v) {			
			case 1: //passed
				m_UnselectedBatchList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconCheck));
				break;
			case 2: //failed
				m_UnselectedBatchList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconRedX));
				break;
			case 3: //warning
				m_UnselectedBatchList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconQuestion));
				break;
			case 0: //untested
				m_UnselectedBatchList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconGrayX));
			default:
				break;
		}

		//and update the provider name, if null
		_variant_t var = m_UnselectedBatchList->GetValue(i,COLUMN_PROV_NAME);
		if(var.vt != VT_BSTR) {
			long nBillID = VarLong(m_UnselectedBatchList->GetValue(i,COLUMN_BILL_ID));
			_RecordsetPtr rs = CreateRecordset("SELECT Last + ', ' + First + ' ' + Middle AS DocName "
				"FROM PersonT WHERE ID IN (SELECT DoctorsProviders FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE BillID = %li AND Deleted = 0) "
				"GROUP BY Last + ', ' + First + ' ' + Middle ORDER BY Last + ', ' + First + ' ' + Middle",nBillID);

			CString strDocs = "";

			while(!rs->eof) {

				strDocs += AdoFldString(rs, "DocName","");
				strDocs += ", ";

				rs->MoveNext();
			}
			rs->Close();

			strDocs.TrimRight(", ");

			m_UnselectedBatchList->PutValue(i,COLUMN_PROV_NAME,_bstr_t(strDocs));
		}

		// (j.jones 2012-03-26 09:19) - PLID 48854 - show multiple claim providers
		var = m_UnselectedBatchList->GetValue(i,COLUMN_CLAIM_PROV_NAME);
		if(var.vt != VT_BSTR) {
			long nBillID = VarLong(m_UnselectedBatchList->GetValue(i,COLUMN_BILL_ID));
			_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS DocName "
				"FROM PersonT WHERE ID IN ("
				"	SELECT "
				"	CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
				"		(CASE WHEN BillsT.FormType = 2 "
				"			THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"		WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
				"		ELSE ChargeProvidersT.ClaimProviderID END) "
				"	WHEN BillsT.FormType = 1 "
				"		THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID Is Not Null "
				"			THEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID "
				"			ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"	END) "
				"	ELSE ChargeProvidersT.ClaimProviderID END) END AS ProviderID "
				"	FROM (SELECT * FROM ChargesT "
				"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"		WHERE Batched = 1 "
				"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	) AS ChargesT "
				"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
				"	INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
				"	LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
				"	LEFT JOIN ProvidersT ChargeProvidersT ON ChargesT.DoctorsProviders = ChargeProvidersT.PersonID "
				"	LEFT JOIN ProvidersT PatientProvidersT ON PatientsT.MainPhysician = PatientProvidersT.PersonID "
				"	LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"	LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"	LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
				"	LEFT JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "
				"	LEFT JOIN HCFAClaimProvidersT ON InsuranceCoT.PersonID = HCFAClaimProvidersT.InsuranceCoID AND (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.PersonID ELSE ChargeProvidersT.PersonID END) = HCFAClaimProvidersT.ProviderID "
				"	WHERE LineItemT.Deleted = 0 AND BillsT.ID = {INT} "
				") "
				"GROUP BY Last + ', ' + First + ' ' + Middle ORDER BY Last + ', ' + First + ' ' + Middle", nBillID);

			CString strDocs = "";

			while(!rs->eof) {

				strDocs += AdoFldString(rs, "DocName","");
				strDocs += ", ";

				rs->MoveNext();
			}
			rs->Close();

			strDocs.TrimRight(", ");

			m_UnselectedBatchList->PutValue(i,COLUMN_CLAIM_PROV_NAME,_bstr_t(strDocs));

					

		}
		
		//(e.lally 2007-02-27) PLID 24773 - Format the Last Sent date to reflect the regional settings
		// (j.jones 2009-03-09 11:37) - PLID 33393 - we fixed this in the datalist now
		/*
		_variant_t varLastSent = m_UnselectedBatchList->GetValue(i,COLUMN_LAST_SEND_DATE);
		if(varLastSent.vt == VT_BSTR) {
			CString strLastSentDt = VarString(varLastSent);
			if(strLastSentDt != "Never"){
				//It is an actual date, format it to the regional settings
				COleDateTime dtLastSent;
				dtLastSent.ParseDateTime(strLastSentDt, NULL);
				strLastSentDt = FormatDateTimeForInterface(dtLastSent, NULL, dtoDate, false);
				m_UnselectedBatchList->PutValue(i,COLUMN_LAST_SEND_DATE, _bstr_t(strLastSentDt));
			}
		}
		*/

		//r.wilson (6/11/2012) PLID 48853 
		IRowSettingsPtr pRowCurrent = m_UnselectedBatchList->GetRow(i);

		//r.wilson (6/11/2012) PLID 48853 - This block will change the text color of the Claim Provider's Name if it's not the same 
		//                                   Provider in the row
		if(pRowCurrent != NULL)
		{
			long nProviderID = VarLong(pRowCurrent->GetValue(COLUMN_PROV_ID),-1);
			long nClaimProvider = VarLong(pRowCurrent->GetValue(COLUMN_CLAIM_PROV_ID),-1);			
			
			if(nProviderID != nClaimProvider && nProviderID != -1)
			{			
				// If the Provider and Claim Provider are different then make the Claim Provider's text Red
				pRowCurrent->PutCellForeColor(COLUMN_CLAIM_PROV_NAME,RGB(255,0,0));								
				pRowCurrent->PutCellForeColorSel(COLUMN_CLAIM_PROV_NAME,RGB(255,0,0));
			}	
		}
	}

	CString str;
	str.Format("%li",m_UnselectedBatchList->GetRowCount());
	SetDlgItemText(IDC_UNSELECTED_TOTAL,str);

	CRect rc;
	GetDlgItem(IDC_UNSELECTED_TOTAL)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);

	CalcShowBatchedChargeCount();
}

void CEbillingFormDlg::OnRequeryFinishedSelectedBatchlist(short nFlags) 
{
	for(int i=0;i<m_SelectedBatchList->GetRowCount();i++) {
		int v = m_SelectedBatchList->GetValue(i,COLUMN_VALIDATION_STATE_VALUE).iVal;
		switch(v) {			
			case 1: //passed
				m_SelectedBatchList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconCheck));
				break;
			case 2: //failed
				m_SelectedBatchList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconRedX));
				break;
			case 3: //warning
				m_SelectedBatchList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconQuestion));
				break;
			case 0: //untested
				m_SelectedBatchList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconGrayX));
			default:
				break;
		}

		//and update the provider name, if null
		_variant_t var = m_SelectedBatchList->GetValue(i,COLUMN_PROV_NAME);
		if(var.vt != VT_BSTR) {
			long nBillID = VarLong(m_SelectedBatchList->GetValue(i,COLUMN_BILL_ID));
			_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS DocName "
				"FROM PersonT WHERE ID IN (SELECT DoctorsProviders FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE BillID = {INT} AND Deleted = 0) "
				"GROUP BY Last + ', ' + First + ' ' + Middle ORDER BY Last + ', ' + First + ' ' + Middle", nBillID);

			CString strDocs = "";

			while(!rs->eof) {

				strDocs += AdoFldString(rs, "DocName","");
				strDocs += ", ";

				rs->MoveNext();
			}
			rs->Close();

			strDocs.TrimRight(", ");

			m_SelectedBatchList->PutValue(i,COLUMN_PROV_NAME,_bstr_t(strDocs));
		}

		// (j.jones 2012-03-26 09:19) - PLID 48854 - show multiple claim providers
		var = m_SelectedBatchList->GetValue(i,COLUMN_CLAIM_PROV_NAME);
		if(var.vt != VT_BSTR) {
			long nBillID = VarLong(m_SelectedBatchList->GetValue(i,COLUMN_BILL_ID));
			_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle AS DocName "
				"FROM PersonT WHERE ID IN ("
				"	SELECT "
				"	CASE WHEN ChargesT.ClaimProviderID Is Not Null THEN ChargesT.ClaimProviderID ELSE "
				"		(CASE WHEN BillsT.FormType = 2 "
				"			THEN (CASE WHEN UB92SetupT.Box82Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"		WHEN UB92SetupT.Box82Setup = 3 THEN ReferringPhysT.PersonID "
				"		ELSE ChargeProvidersT.ClaimProviderID END) "
				"	WHEN BillsT.FormType = 1 "
				"		THEN (CASE WHEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID Is Not Null "
				"			THEN HCFAClaimProvidersT.ANSI_2010AA_ProviderID "
				"			ELSE (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.ClaimProviderID "
				"			ELSE ChargeProvidersT.ClaimProviderID END) "
				"	END) "
				"	ELSE ChargeProvidersT.ClaimProviderID END) END AS ProviderID "
				"	FROM (SELECT * FROM ChargesT "
				"		LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"		LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"		WHERE Batched = 1 "
				"		AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"		AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"	) AS ChargesT "
				"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"	INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
				"	INNER JOIN PatientsT ON BillsT.PatientID = PatientsT.PersonID "
				"	LEFT JOIN ReferringPhysT ON BillsT.RefPhyID = ReferringPhysT.PersonID "
				"	LEFT JOIN ProvidersT ChargeProvidersT ON ChargesT.DoctorsProviders = ChargeProvidersT.PersonID "
				"	LEFT JOIN ProvidersT PatientProvidersT ON PatientsT.MainPhysician = PatientProvidersT.PersonID "
				"	LEFT JOIN InsuredPartyT ON BillsT.InsuredPartyID = InsuredPartyT.PersonID "
				"	LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"	LEFT JOIN HCFASetupT ON InsuranceCoT.HCFASetupGroupID = HCFASetupT.ID "
				"	LEFT JOIN UB92SetupT ON InsuranceCoT.UB92SetupGroupID = UB92SetupT.ID "
				"	LEFT JOIN HCFAClaimProvidersT ON InsuranceCoT.PersonID = HCFAClaimProvidersT.InsuranceCoID AND (CASE WHEN HCFASetupT.Box33Setup = 2 THEN PatientProvidersT.PersonID ELSE ChargeProvidersT.PersonID END) = HCFAClaimProvidersT.ProviderID "
				"	WHERE LineItemT.Deleted = 0 AND BillsT.ID = {INT} "
				") "
				"GROUP BY Last + ', ' + First + ' ' + Middle ORDER BY Last + ', ' + First + ' ' + Middle", nBillID);

			CString strDocs = "";

			while(!rs->eof) {

				strDocs += AdoFldString(rs, "DocName","");
				strDocs += ", ";

				rs->MoveNext();
			}
			rs->Close();

			strDocs.TrimRight(", ");

			m_SelectedBatchList->PutValue(i,COLUMN_CLAIM_PROV_NAME,_bstr_t(strDocs));
		}

		//(e.lally 2007-02-27) PLID 24773 - Format the Last Sent date to reflect the regional settings
		// (j.jones 2009-03-09 11:37) - PLID 33393 - we fixed this in the datalist now
		/*
		_variant_t varLastSent = m_SelectedBatchList->GetValue(i,COLUMN_LAST_SEND_DATE);
		if(varLastSent.vt == VT_BSTR) {
			CString strLastSentDt = VarString(varLastSent);
			if(strLastSentDt != "Never"){
				//It is an actual date, format it to the regional settings
				COleDateTime dtLastSent;
				dtLastSent.ParseDateTime(strLastSentDt, NULL);
				strLastSentDt = FormatDateTimeForInterface(dtLastSent, NULL, dtoDate, false);
				m_SelectedBatchList->PutValue(i,COLUMN_LAST_SEND_DATE, _bstr_t(strLastSentDt));
			}
		}
		*/


		//r.wilson (6/11/2012) PLID 48853 
		IRowSettingsPtr pRowCurrent = m_SelectedBatchList->GetRow(i);

		//r.wilson (6/11/2012) PLID 48853 - This block will change the text color of the Claim Provider's Name if it's not the same 
		//                                   Provider in the row
		if(pRowCurrent != NULL)
		{
			long nProviderID = VarLong(pRowCurrent->GetValue(COLUMN_PROV_ID),-1);
			long nClaimProviderID = VarLong(pRowCurrent->GetValue(COLUMN_CLAIM_PROV_ID),-1);
					
			if(nProviderID != nClaimProviderID && nProviderID != -1)
			{			
				// If the Provider and Claim Provider are different then make the Claim Provider's text Red
				pRowCurrent->PutCellForeColor(COLUMN_CLAIM_PROV_NAME,RGB(255,0,0));
				pRowCurrent->PutCellForeColorSel(COLUMN_CLAIM_PROV_NAME,RGB(255,0,0));
			}	
		}
	
	}

	CString str;
	str.Format("%li",m_SelectedBatchList->GetRowCount());
	SetDlgItemText(IDC_SELECTED_TOTAL,str);

	CRect rc;
	GetDlgItem(IDC_SELECTED_TOTAL)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);

	CalcShowBatchedChargeCount();
}

void CEbillingFormDlg::OnGoToPatient() {

	try {
		long nPatientID;

		if(m_CurrList==0)
			nPatientID = m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(),COLUMN_PATIENT_ID).lVal;
		else
			nPatientID = m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(),COLUMN_PATIENT_ID).lVal;

		if (nPatientID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {

				if(!pMainFrame->m_patToolBar.DoesPatientExistInList(nPatientID)) {
					if(IDNO == MessageBox("This patient is not in the current lookup. \n"
						"Do you wish to reset the lookup to include all patients?","Practice",MB_ICONQUESTION|MB_YESNO)) {
						return;
					}
				}
				//TES 1/7/2010 - PLID 36761 - This function may fail now
				if(pMainFrame->m_patToolBar.TrySetActivePatientID(nPatientID)) {

					//Now just flip to the patient's module and set the active Patient
					pMainFrame->FlipToModule(PATIENT_MODULE_NAME);
					CNxTabView *pView = pMainFrame->GetActiveView();
					if(pView)
						pView->UpdateView();
				}
			}//end if MainFrame
			else {
				MsgBox(MB_ICONSTOP|MB_OK, "ERROR 1236 - EbillingFormDlg.cpp: Cannot Open Mainframe");
			}//end else pMainFrame
		}//end if nPatientID
	}NxCatchAll("Error in OnGoToPatient");
}

void CEbillingFormDlg::OnDblClickCellUnselectedBatchlist(long nRowIndex, short nColIndex) 
{
	OnSelectOne();
}

void CEbillingFormDlg::OnDblClickCellSelectedBatchlist(long nRowIndex, short nColIndex) 
{
	OnUnselectOne();
}

void CEbillingFormDlg::OnSelectOne() 
{
	try {

		if (m_UnselectedBatchList->GetCurSel()!=-1)
		{			
			m_SelectedBatchList->TakeCurrentRow(m_UnselectedBatchList);			
		}

		UpdateCurrentSelect(1);

		RefreshTotals();

	}NxCatchAll("Error in OnSelectOne()");
}

void CEbillingFormDlg::OnSelectAll() 
{
	try {

		m_SelectedBatchList->TakeAllRows(m_UnselectedBatchList);

		UpdateCurrentSelect(1);

		RefreshTotals();

	}NxCatchAll("Error in OnSelectAll()");
}

void CEbillingFormDlg::OnUnselectOne() 
{
	try {

		if (m_SelectedBatchList->GetCurSel()!=-1)
			m_UnselectedBatchList->TakeCurrentRow(m_SelectedBatchList);

		UpdateCurrentSelect(0);

		RefreshTotals();

	}NxCatchAll("Error in OnUnselectOne()");
}

void CEbillingFormDlg::OnUnselectAll() 
{
	try {

		m_UnselectedBatchList->TakeAllRows(m_SelectedBatchList);

		UpdateCurrentSelect(0);

		RefreshTotals();

	}NxCatchAll("Error in OnUnselectAll()");
}

//UpdateCurrentSelect will take in the ID of one of the lists (0 - Unselected, 1 - Selected)
//and check for any items that were just moved over from the other list. If any
//are marked as being on the other list, we update the data to reflect which list the item belongs in.
void CEbillingFormDlg::UpdateCurrentSelect(int iList)
{
	try {

		_variant_t var;

		if(iList==0) {
			//loop through the list, look for CurrentSelect = TRUE, and reverse it
			for(int i=0;i<m_UnselectedBatchList->GetRowCount();i++) {
				var = m_UnselectedBatchList->GetValue(i,COLUMN_CURRENT_SELECT);
				if(var.boolVal)
					ExecuteSql("UPDATE HCFATrackT SET CurrentSelect = 0 WHERE ID = %li",m_UnselectedBatchList->GetValue(i,COLUMN_HCFA_ID).lVal);
				var.boolVal = FALSE;
				m_UnselectedBatchList->PutValue(i,COLUMN_CURRENT_SELECT,var);

				//(r.wilson 6/27/2012) plid 48853
				IRowSettingsPtr pRowCurrent = m_UnselectedBatchList->GetRow(i);
				if(pRowCurrent != NULL)
				{
					long nProviderID = VarLong(pRowCurrent->GetValue(COLUMN_PROV_ID),-1);
					long nClaimProvider = VarLong(pRowCurrent->GetValue(COLUMN_CLAIM_PROV_ID),-1);			
					
					if(nProviderID != nClaimProvider && nProviderID != -1)
					{			
						// If the Provider and Claim Provider are different then make the Claim Provider's text Red
						pRowCurrent->PutCellForeColor(COLUMN_CLAIM_PROV_NAME,RGB(255,0,0));								
						pRowCurrent->PutCellForeColorSel(COLUMN_CLAIM_PROV_NAME,RGB(255,0,0));
					}	
				}

			}
		}
		else {
			//loop through the list, look for CurrentSelect = FALSE, and reverse it
			for(int i=0;i<m_SelectedBatchList->GetRowCount();i++) {
				var = m_SelectedBatchList->GetValue(i,COLUMN_CURRENT_SELECT);
				if(!var.boolVal)
					ExecuteSql("UPDATE HCFATrackT SET CurrentSelect = 1 WHERE ID = %li",m_SelectedBatchList->GetValue(i,COLUMN_HCFA_ID).lVal);
				var.boolVal = TRUE;
				m_SelectedBatchList->PutValue(i,COLUMN_CURRENT_SELECT,var);

				//(r.wilson 6/27/2012) plid 48853
				IRowSettingsPtr pRowCurrent = m_SelectedBatchList->GetRow(i);
				if(pRowCurrent != NULL)
				{
					long nProviderID = VarLong(pRowCurrent->GetValue(COLUMN_PROV_ID),-1);
					long nClaimProvider = VarLong(pRowCurrent->GetValue(COLUMN_CLAIM_PROV_ID),-1);			
					
					if(nProviderID != nClaimProvider && nProviderID != -1)
					{			
						// If the Provider and Claim Provider are different then make the Claim Provider's text Red
						pRowCurrent->PutCellForeColor(COLUMN_CLAIM_PROV_NAME,RGB(255,0,0));								
						pRowCurrent->PutCellForeColorSel(COLUMN_CLAIM_PROV_NAME,RGB(255,0,0));
					}	
				}

			}
		}

	}NxCatchAll("Error in UpdateCurrentSelect");
}

void CEbillingFormDlg::RefreshTotals()
{
	CString str;
	str.Format("%li",m_SelectedBatchList->GetRowCount());
	SetDlgItemText(IDC_SELECTED_TOTAL,str);

	str.Format("%li",m_UnselectedBatchList->GetRowCount());
	SetDlgItemText(IDC_UNSELECTED_TOTAL,str);

	CRect rc;
	GetDlgItem(IDC_SELECTED_TOTAL)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);

	GetDlgItem(IDC_UNSELECTED_TOTAL)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);

	CalcShowBatchedChargeCount();
}

// (j.jones 2007-03-01 08:59) - PLID 25015 - changed Validate to return an enum
EEbillingValidationResults CEbillingFormDlg::Validate(long HCFAID) {

	if(m_FormTypeCombo->GetCurSel() == -1)
		return eevrFailedOther;

	if(m_LocationCombo->GetCurSel() == -1)
		return eevrFailedOther;

	if(!CheckCurrentUserPermissions(bioEBilling,sptRead))
		return eevrFailedOther;

	long nProviderID = -1;
	if (m_ProviderCombo->GetCurSel() != -1) {
		nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
	}

	CEbillingValidationDlg dlg(this);

	long nLocation = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
	long nClaimType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;

	dlg.DoModal(HCFAID,FormatID,FormatStyle,nLocation,nProviderID,nClaimType,TRUE);

	UpdateView();

	if(dlg.m_bErrorListDisplayed)
		return eevrFailedError;
	else
		return eevrSuccess;
}

void CEbillingFormDlg::OnValidateOne() {

	if(m_LocationCombo->GetCurSel() == -1)
		return;

	try {
		
		long HCFAID = 0;

		if(m_CurrList == 0)
			HCFAID = m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(),COLUMN_HCFA_ID).lVal;
		else
			HCFAID = m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(),COLUMN_HCFA_ID).lVal;

		if(HCFAID > 0)
			Validate(HCFAID);
	
	}NxCatchAll("Error validating claim.");
}

void CEbillingFormDlg::OnValidateAllClaims() 
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(m_UnselectedBatchList->GetRowCount() == 0 && m_SelectedBatchList->GetRowCount() == 0) {
		AfxMessageBox("There are no batched claims to validate!");
		return;
	}
	
	Validate(-3);
}

void CEbillingFormDlg::OnValidateSelectedClaims() 
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(m_SelectedBatchList->GetRowCount() == 0) {
		AfxMessageBox("There are no selected claims to validate!");
		return;
	}

	Validate(-2);
}

void CEbillingFormDlg::OnValidateUnselectedClaims() 
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(m_UnselectedBatchList->GetRowCount() == 0) {
		AfxMessageBox("There are no unselected claims to validate!");
		return;
	}

	Validate(-1);
}

void CEbillingFormDlg::OnProductionBatch() 
{
	int tempint;

	// envoy test / production
	if (m_production.GetCheck()) tempint = 1;
	else tempint = 0;

	SetRemotePropertyInt(_T("EnvoyProduction"),tempint,0,_T("<None>"));
}

void CEbillingFormDlg::OnTestBatch() 
{
	int tempint;

	// envoy test / production
	if (m_production.GetCheck()) tempint = 1;
	else tempint = 0;

	SetRemotePropertyInt(_T("EnvoyProduction"),tempint,0,_T("<None>"));
}

void CEbillingFormDlg::PrepareImage()
{
	m_btnExportBatch.EnableWindow(TRUE);
}

void CEbillingFormDlg::PrepareOHIP()
{
	m_btnExportBatch.EnableWindow(TRUE);
}

// (j.jones 2010-07-09 14:37) - PLID 29971 - added support for Alberta HLINK
void CEbillingFormDlg::PrepareAlberta()
{
	m_btnExportBatch.EnableWindow(TRUE);
}

void CEbillingFormDlg::PrepareANSI()
{
	m_ExportStyle->Requery();

	if(m_ExportStyle->SetSelByColumn(0,(long)FormatID) == -1)
		m_ExportStyle->CurSel = 0;
}

void CEbillingFormDlg::OnBtnRetrieveBatches() 
{
	CRetrieveEbillingBatchDlg dlg(this);
	dlg.m_pEbillingTabPtr = this;
	dlg.DoModal();

	UpdateView();	
}

void CEbillingFormDlg::OnFormat997Btn() 
{
	if(FormatStyle == ANSI) {
		//parse the 997 file
		CANSIReportParserDlg dlg(this);
		dlg.DoModal();
	}
}

void CEbillingFormDlg::OnFormat277Btn() 
{
	if(FormatStyle == ANSI) {
		//parse the 277 file
		CANSI277ReportParserDlg dlg(this);
		dlg.DoModal();
	}
}

void CEbillingFormDlg::OnSelectInsCo() {

	//select/unselect all claims with the same insurance company
	try {

		if(m_CurrList==0) {

			if(m_UnselectedBatchList->GetCurSel()==-1) {
				AfxMessageBox("There is no claim selected");
				return;
			}

			//move all claims with this insurance company to the selected list
			long nInsCoID = VarLong(m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(),COLUMN_INS_CO_ID),-1);
			for(int i=m_UnselectedBatchList->GetRowCount()-1;i>=0;i--) {
				IRowSettingsPtr pRow = m_UnselectedBatchList->GetRow(i);
				if(VarLong(pRow->GetValue(COLUMN_INS_CO_ID),-1) == nInsCoID)
					m_SelectedBatchList->TakeRow(pRow);
			}

			UpdateCurrentSelect(1);

		}
		else {

			if(m_SelectedBatchList->GetCurSel()==-1) {
				AfxMessageBox("There is no claim selected");
				return;
			}

			//move all claims with this insurance company to the unsselected list
			long nInsCoID = VarLong(m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(),COLUMN_INS_CO_ID),-1);
			for(int i=m_SelectedBatchList->GetRowCount()-1;i>=0;i--) {
				IRowSettingsPtr pRow = m_SelectedBatchList->GetRow(i);
				if(VarLong(pRow->GetValue(COLUMN_INS_CO_ID),-1) == nInsCoID)
					m_UnselectedBatchList->TakeRow(pRow);
			}

			UpdateCurrentSelect(0);
		}		

		RefreshTotals();
		
	} NxCatchAll("Error in OnSelectInsCo");

}

void CEbillingFormDlg::OnSelectInsGroup() {

	//select/unselect all claims with the same insurance group
	try {

		if(m_CurrList==0) {

			if(m_UnselectedBatchList->GetCurSel()==-1) {
				AfxMessageBox("There is no claim selected");
				return;
			}

			//move all claims with this insurance group to the selected list
			long nInsCoID = VarLong(m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(),COLUMN_INS_CO_ID),-1);
			long nInsGroup = -1;
			_RecordsetPtr rs = CreateRecordset("SELECT HCFASetupGroupID FROM InsuranceCoT WHERE PersonID = %li",nInsCoID);
			if(!rs->eof) {
				nInsGroup = AdoFldLong(rs, "HCFASetupGroupID",-1);
			}
			rs->Close();

			for(int i=m_UnselectedBatchList->GetRowCount()-1;i>=0;i--) {
				IRowSettingsPtr pRow = m_UnselectedBatchList->GetRow(i);
				long nSelInsCoID = VarLong(pRow->GetValue(COLUMN_INS_CO_ID),-1);
				if(!IsRecordsetEmpty("SELECT PersonID FROM InsuranceCoT WHERE HCFASetupGroupID = %li AND PersonID = %li",nInsGroup,nSelInsCoID)) {
					m_SelectedBatchList->TakeRow(pRow);
				}
			}

			UpdateCurrentSelect(1);

		}
		else {

			if(m_SelectedBatchList->GetCurSel()==-1) {
				AfxMessageBox("There is no claim selected");
				return;
			}

			//move all claims with this insurance group to the unsselected list
			long nInsCoID = VarLong(m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(),COLUMN_INS_CO_ID),-1);
			long nInsGroup = -1;
			_RecordsetPtr rs = CreateRecordset("SELECT HCFASetupGroupID FROM InsuranceCoT WHERE PersonID = %li",nInsCoID);
			if(!rs->eof) {
				nInsGroup = AdoFldLong(rs, "HCFASetupGroupID",-1);
			}
			rs->Close();

			for(int i=m_SelectedBatchList->GetRowCount()-1;i>=0;i--) {
				IRowSettingsPtr pRow = m_SelectedBatchList->GetRow(i);
				long nSelInsCoID = VarLong(pRow->GetValue(COLUMN_INS_CO_ID),-1);
				if(!IsRecordsetEmpty("SELECT PersonID FROM InsuranceCoT WHERE HCFASetupGroupID = %li AND PersonID = %li",nInsGroup,nSelInsCoID)) {
					m_UnselectedBatchList->TakeRow(pRow);
				}
			}

			UpdateCurrentSelect(0);
		}		

		RefreshTotals();
		
	} NxCatchAll("Error in OnSelectInsGroup");
}

void CEbillingFormDlg::OnSelectProvider() {

	//select/unselect all claims with the same provider name (explicit name comparison, not multi-doctor)
	try {

		if(m_CurrList==0) {

			if(m_UnselectedBatchList->GetCurSel()==-1) {
				AfxMessageBox("There is no claim selected");
				return;
			}

			//move all claims with this provider to the selected list
			CString strProvName = VarString(m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(),COLUMN_PROV_NAME),"");
			for(int i=m_UnselectedBatchList->GetRowCount()-1;i>=0;i--) {
				IRowSettingsPtr pRow = m_UnselectedBatchList->GetRow(i);
				if(VarString(pRow->GetValue(COLUMN_PROV_NAME),"") == strProvName)
					m_SelectedBatchList->TakeRow(pRow);
			}

			UpdateCurrentSelect(1);

		}
		else {

			if(m_SelectedBatchList->GetCurSel()==-1) {
				AfxMessageBox("There is no claim selected");
				return;
			}

			//move all claims with this provider to the unsselected list
			CString strProvName = VarString(m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(),COLUMN_PROV_NAME),"");
			for(int i=m_SelectedBatchList->GetRowCount()-1;i>=0;i--) {
				IRowSettingsPtr pRow = m_SelectedBatchList->GetRow(i);
				if(VarString(pRow->GetValue(COLUMN_PROV_NAME),"") == strProvName)
					m_UnselectedBatchList->TakeRow(pRow);
			}

			UpdateCurrentSelect(0);
		}		

		RefreshTotals();
		
	} NxCatchAll("Error in OnSelectProv");

}

// (j.jones 2012-03-26 10:43) - PLID 48854 - added ability to select by claim provider
void CEbillingFormDlg::OnSelectClaimProvider() {

	//select/unselect all claims with the same claim provider name (explicit name comparison, not multi-doctor)
	try {

		if(m_CurrList==0) {

			if(m_UnselectedBatchList->GetCurSel()==-1) {
				AfxMessageBox("There is no claim selected");
				return;
			}

			//move all claims with this claim provider to the selected list
			CString strProvName = VarString(m_UnselectedBatchList->GetValue(m_UnselectedBatchList->GetCurSel(),COLUMN_CLAIM_PROV_NAME),"");
			for(int i=m_UnselectedBatchList->GetRowCount()-1;i>=0;i--) {
				IRowSettingsPtr pRow = m_UnselectedBatchList->GetRow(i);
				if(VarString(pRow->GetValue(COLUMN_CLAIM_PROV_NAME),"") == strProvName)
					m_SelectedBatchList->TakeRow(pRow);
			}

			UpdateCurrentSelect(1);

		}
		else {

			if(m_SelectedBatchList->GetCurSel()==-1) {
				AfxMessageBox("There is no claim selected");
				return;
			}

			//move all claims with this claim provider to the unsselected list
			CString strProvName = VarString(m_SelectedBatchList->GetValue(m_SelectedBatchList->GetCurSel(),COLUMN_CLAIM_PROV_NAME),"");
			for(int i=m_SelectedBatchList->GetRowCount()-1;i>=0;i--) {
				IRowSettingsPtr pRow = m_SelectedBatchList->GetRow(i);
				if(VarString(pRow->GetValue(COLUMN_CLAIM_PROV_NAME),"") == strProvName)
					m_UnselectedBatchList->TakeRow(pRow);
			}

			UpdateCurrentSelect(0);
		}		

		RefreshTotals();
		
	} NxCatchAll(__FUNCTION__);
}

void CEbillingFormDlg::OnSelChosenExportstyle(long nRow) 
{
	try{
		if(nRow == -1) {
			FormatID = -1;
			if(FormatStyle == ANSI) {
				m_btnExportBatch.EnableWindow(FALSE);
			}
		}
		else {
			FormatID = (m_ExportStyle->GetValue(nRow,0).lVal);
			m_btnExportBatch.EnableWindow(TRUE);
		}

		SetRemotePropertyInt(_T("FormatStyle"),FormatID,0,_T("<None>"));

		// (j.dinatale 2012-12-27 11:55) - PLID 54136 - we need to update our view here now, added exception handling
		UpdateView();
	}NxCatchAll(__FUNCTION__);
}

void CEbillingFormDlg::OnSelChosenLocationComboElectronic(long nRow) 
{
	try {

		if(nRow == -1) {
			m_LocationCombo->SetSelByColumn(0,(long)-1);
		}

		// (j.jones 2009-08-14 10:01) - PLID 35235 - just call RefilterLists
		RefilterLists();

	}NxCatchAll("Error changing location.");
}

void CEbillingFormDlg::CalcShowBatchedChargeCount() {

	try {

		//determine if we need to show the count of charges sent,
		//if so, show it in both lists for a uniform display

		BOOL bShowChargesBatched = FALSE;

		//TES 11/5/2007 - PLID 27978 - VS2008 - for() loops
		int i = 0;
		for(i=0;i<m_UnselectedBatchList->GetRowCount() && !bShowChargesBatched;i++) {
			IRowSettingsPtr pRow = m_UnselectedBatchList->GetRow(i);

			//calculate the charges sent
			if(VarLong(pRow->GetValue(COLUMN_COUNT_CHARGES_BATCHED),0) < VarLong(pRow->GetValue(COLUMN_COUNT_CHARGES),0)) {
				bShowChargesBatched = TRUE;
			}
		}

		for(i=0;i<m_SelectedBatchList->GetRowCount() && !bShowChargesBatched;i++) {
			IRowSettingsPtr pRow = m_SelectedBatchList->GetRow(i);

			//calculate the charges sent
			if(VarLong(pRow->GetValue(COLUMN_COUNT_CHARGES_BATCHED),0) < VarLong(pRow->GetValue(COLUMN_COUNT_CHARGES),0)) {
				bShowChargesBatched = TRUE;
			}
		}

		if(bShowChargesBatched) {
			//if any charges batched are less than the total charges on the bill, show the charges batched column
			IColumnSettingsPtr pCol1 = m_UnselectedBatchList->GetColumn(COLUMN_CHARGES_BATCHED);
			pCol1->PutStoredWidth(92);
			IColumnSettingsPtr pCol2 = m_SelectedBatchList->GetColumn(COLUMN_CHARGES_BATCHED);
			pCol2->PutStoredWidth(92);
		}
		else {
			//otherwise we can hide it
			IColumnSettingsPtr pCol1 = m_UnselectedBatchList->GetColumn(COLUMN_CHARGES_BATCHED);
			pCol1->PutStoredWidth(0);
			IColumnSettingsPtr pCol2 = m_SelectedBatchList->GetColumn(COLUMN_CHARGES_BATCHED);
			pCol2->PutStoredWidth(0);
		}

	}NxCatchAll("Error displaying count of charges batched.");

}

void CEbillingFormDlg::OnSelChosenEbillingFormTypeCombo(long nRow) 
{
	if(nRow == -1) {
		m_FormTypeCombo->CurSel = 1;
	}

	if(VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->CurSel,0),-1) == 1) {
		ClaimType = actProf;
		SetRemotePropertyInt("EbillingClaimType",actProf,0,"<None>");		
	}
	else {
		ClaimType = actInst;
		SetRemotePropertyInt("EbillingClaimType",actInst,0,"<None>");
	}

	// (j.jones 2010-07-23 16:15) - PLID 34105 - if HCFA and not OHIP,
	// warn if any setup can have assignment of benefits not filled
	// (this does not account for individual bills with Box 13 overridden)
	BOOL bShowWarning = TRUE;

	// (j.jones 2010-07-27 09:26) - PLID 39854 - this is now supported on UBs as well
	BOOL bIsHCFA = (m_FormTypeCombo->CurSel != -1 && VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->CurSel,0),-1) == 1);
	BOOL bIsUB = (m_FormTypeCombo->CurSel != -1 && VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->CurSel,0),-1) == 2);

	// (j.jones 2010-11-08 13:58) - PLID 39620 - skip if Alberta
	if(UseOHIP() || UseAlbertaHLINK() || (!bIsHCFA && !bIsUB)) {
		bShowWarning = FALSE;
	}
	// (j.jones 2010-07-27 09:28) - PLID 39854 - now we tell the function which form type to check
	if(bShowWarning && CanAssignmentOfBenefitsBeBlank(bIsHCFA ? babtHCFA : babtUB)) {
		m_nxstaticAssignmentOfBenefitsWarningLabel.ShowWindow(SW_SHOW);
		m_btnAssignmentOfBenefitsWarning.ShowWindow(SW_SHOW);
	}
	else {
		m_nxstaticAssignmentOfBenefitsWarningLabel.ShowWindow(SW_HIDE);
		m_btnAssignmentOfBenefitsWarning.ShowWindow(SW_HIDE);
	}

	// (j.jones 2009-08-14 10:03) - PLID 35235 - call RefilterLists()
	RefilterLists();

	// (j.jones 2006-06-14 16:08) - I put up this message after the processing so
	// they can read it while the list is reloading
	if(ClaimType == actInst && FormatStyle == IMAGE) {
		AfxMessageBox("The Image export style will only export the claims in a HCFA Image format.\n"
			"Please double-check the Form Type selection and ensure it is correct before exporting your claims.");
	}
}

void CEbillingFormDlg::UpdateLabels(CString strFormType)
{
	if(!strFormType.IsEmpty())
		strFormType += " ";

	CString str;
	str.Format("Unselected Batched %sClaims:",strFormType);		
	SetDlgItemText(IDC_EBILLING_UNSELECTED_LABEL,str);
	CRect rc;		
	GetDlgItem(IDC_EBILLING_UNSELECTED_LABEL)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);
	str.Format("Selected Batched %sClaims:",strFormType);
	SetDlgItemText(IDC_EBILLING_SELECTED_LABEL,str);
	GetDlgItem(IDC_EBILLING_SELECTED_LABEL)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);

	str.Format("Unbatch\nUnselected\n%sClaims",strFormType);
	SetDlgItemText(IDC_RESET_UNSELECTED,str);
	str.Format("Preview\nUnselected\n%sClaims",strFormType);
	SetDlgItemText(IDC_PRINT_UNSELECTED_LIST,str);
	str.Format("Validate\nUnselected\n%sClaims",strFormType);
	SetDlgItemText(IDC_VALIDATE_UNSELECTED_CLAIMS,str);

	str.Format("Unbatch\nAll\n%sClaims",strFormType);
	SetDlgItemText(IDC_RESET,str);
	str.Format("Preview\nAll Batched\n%sClaims",strFormType);
	SetDlgItemText(IDC_PRINTLIST,str);
	str.Format("Validate\nAll\n%sClaims",strFormType);
	SetDlgItemText(IDC_VALIDATE_ALL_CLAIMS,str);

	str.Format("Unbatch\nSelected\n%sClaims",strFormType);
	SetDlgItemText(IDC_RESET_SELECTED,str);
	str.Format("Preview\nSelected\n%sClaims",strFormType);
	SetDlgItemText(IDC_PRINT_SELECTED_LIST,str);
	str.Format("Validate\nSelected\n%sClaims",strFormType);
	SetDlgItemText(IDC_VALIDATE_SELECTED_CLAIMS,str);
}

void CEbillingFormDlg::OnBtnConfigureClaimValidation() 
{
	try {

		// (j.jones 2008-12-03 13:56) - PLID 32258 - added support for the OHIP validation configuration
		if(FormatStyle == OHIP) {
			COHIPValidationConfigDlg dlg(this);
			dlg.DoModal();
		}
		// (j.jones 2010-11-03 10:20) - PLID 41288 - added an Alberta validation configuration
		else if(FormatStyle == ALBERTA) {
			CAlbertaClaimValidationConfigDlg dlg(this);
			dlg.DoModal();
		}
		else {
			CEbillingValidationConfigDlg dlg(this);
			dlg.m_bIsANSI = TRUE;
			
			// (j.jones 2012-10-22 13:36) - PLID 53297 - added ANSI Version, we assume 5010 unless told otherwise
			if(FormatID != -1) {
				_RecordsetPtr rs = CreateParamRecordset("SELECT ANSIVersion FROM EbillingFormatsT WHERE ID = {INT}", FormatID);
				if(!rs->eof) {
					dlg.m_avANSIVersion = (ANSIVersion)AdoFldLong(rs, "ANSIVersion", (long)av5010);
				}
				rs->Close();
			}

			dlg.DoModal();
		}

	}NxCatchAll("Error in CEbillingFormDlg::OnBtnConfigureClaimValidation");
}

void CEbillingFormDlg::OnSelChosenExportformat(long nRow) 
{
	if(nRow == -1) {
		m_ExportFormat->CurSel = 0;
		nRow = m_ExportFormat->CurSel;
	}

	long nValue = m_ExportFormat->GetValue(nRow, 0);

	// (j.jones 2008-05-09 09:37) - PLID 29986 - completely removed NSF from the program
	
	if(nValue == IMAGE) {

		//HCFA Image

		FormatStyle = 2;
		SetRemotePropertyInt(_T("EbillingFormatType"),FormatStyle,0,_T("<None>"));

		PrepareImage();
		
		OnStyleChange(-1);
	}
	else if(nValue == OHIP) {

		//OHIP

		FormatStyle = OHIP;
		SetRemotePropertyInt(_T("EbillingFormatType"),FormatStyle,0,_T("<None>"));

		PrepareOHIP();
		
		OnStyleChange(-1);
	}
	// (j.jones 2010-07-09 14:37) - PLID 29971 - added support for Alberta HLINK
	else if(nValue == ALBERTA) {

		//Alberta HLINK
		FormatStyle = ALBERTA;
		SetRemotePropertyInt(_T("EbillingFormatType"),FormatStyle,0,_T("<None>"));

		PrepareAlberta();
		
		OnStyleChange(-1);
	}
	else {

		//ANSI

		FormatStyle = ANSI;
		SetRemotePropertyInt(_T("EbillingFormatType"),FormatStyle,0,_T("<None>"));

		PrepareANSI();

		OnStyleChange(m_ExportStyle->CurSel);
		OnSelChosenExportstyle(m_ExportStyle->CurSel);
	}
}

// (j.jones 2008-12-17 09:44) - PLID 31900 - removed these buttons in lieu of the report manager
/*
// (j.jones 2008-07-02 15:40) - PLID 30604 - added ability to format OHIP Batch Edit report
void CEbillingFormDlg::OnFormatOhipBatchEditBtn() 
{
	try {

		COHIPBatchEditParser dlg;
		dlg.ParseFile();

	}NxCatchAll("Error in CEbillingFormDlg::OnFormatOhipBatchEditBtn");
}

// (j.jones 2008-07-02 15:41) - PLID 21968 - added ability to format OHIP Claims Error report
void CEbillingFormDlg::OnFormatOhipClaimsErrorBtn() 
{
	try {

		COHIPClaimsErrorParser dlg;
		dlg.ParseFile();

	}NxCatchAll("Error in CEbillingFormDlg::OnFormatOhipClaimsErrorBtn");
}
*/

// (j.jones 2008-12-17 09:44) - PLID 31900 - added the OHIP Report Manager, which
// replaces the batch edit / claims error buttons
void CEbillingFormDlg::OnBtnOhipReportManager()
{
	try {

		COHIPReportManagerDlg dlg(this);
		dlg.m_bAutoScanForReports = FALSE;	// (j.jones 2009-03-10 10:57) - PLID 33419 - added, but not used here
		dlg.DoModal();

	}NxCatchAll("Error in CEbillingFormDlg::OnBtnOhipReportManager");
}

// (j.jones 2009-03-09 15:06) - PLID 32405 - added the OHIP dialer setup
void CEbillingFormDlg::OnBtnOhipDialerSetup()
{
	try {

		COHIPDialerSetupDlg dlg(this);
		dlg.DoModal();

		// (j.jones 2009-08-14 10:14) - PLID 35235 - update our view to reflect
		// a potential provider filtering change
		UpdateView();

	}NxCatchAll("Error in CEbillingFormDlg::OnBtnOhipDialerSetup");
}

// (j.jones 2009-03-09 15:07) - PLID 33419 - added the Download Reports button
void CEbillingFormDlg::OnBtnDownloadReports()
{
	try {

		long nProviderID = -1;
		if (m_ProviderCombo->GetCurSel() != -1) {
			nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
		}

		if (nProviderID == -1 && GetRemotePropertyInt("OHIP_SeparateAccountsPerProviders", 0, 0, "<None>", TRUE) == 1
			&& FormatStyle == OHIP) {

			AfxMessageBox("You must select a provider to download reports for.");
			return;
		}

		long nReturnValue = DownloadReportsFromOHIP(nProviderID);

		//this function should have had a message no matter what the return value was

		//The return value is -1 if a problem occurred, 0 if no reports downloaded, 1 if reports were downloaded.
		if(nReturnValue == 1) {
			//auto-open the report manager, and auto-import
			COHIPReportManagerDlg dlg(this);
			dlg.m_bAutoScanForReports = TRUE;
			dlg.DoModal();
		}

	}NxCatchAll("Error in CEbillingFormDlg::OnBtnDownloadReports");
}

// (j.jones 2009-08-14 09:56) - PLID 35235 - added provider filter
void CEbillingFormDlg::OnSelChosenEbillingProviderCombo(long nRow)
{
	try {

		RefilterLists();
	
	}NxCatchAll("Error in CEbillingFormDlg::OnSelChosenEbillingProviderCombo");
}

// (j.jones 2009-08-14 09:59) - PLID 35235 - added a filter function
void CEbillingFormDlg::RefilterLists()
{
	try {

		if(m_LocationCombo->GetCurSel() == -1) {
			m_LocationCombo->PutCurSel(0);

			//just allow an exception if this were to be empty - would be impossible
		}

		long nLocationID = VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0),-1);

		CString strWhere;

		if(nLocationID != -1) {
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			strWhere.Format(" BillID IN (SELECT BillID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemT.LocationID = %li)",nLocationID);
		}

		if(m_FormTypeCombo->GetCurSel() == -1) {
			m_FormTypeCombo->CurSel = 1;
		}

		CString strWhere2;
		strWhere2.Format("FormType = %li",m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal);

		if(!strWhere.IsEmpty()) {
			strWhere += " AND ";
		}

		strWhere += strWhere2;

		//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
		// (j.jones 2009-03-16 16:55) - PLID 32321 - supported OHIP labeling,
		// turns out this was a bug all along when loading, as switching to
		// OHIP or Image always removed the claim type
		CString strLabel = "";
		if(FormatStyle == ANSI) {
			strLabel = ClaimType == actProf ? "HCFA" : "UB";
		}
		UpdateLabels(strLabel);
		
		//the provider filter is required to be used for OHIP and Alberta, and cannot be "All"
		if ((FormatStyle == OHIP && GetRemotePropertyInt("OHIP_SeparateAccountsPerProviders", 0, 0, "<None>", TRUE) == 1) || FormatStyle == ALBERTA) {
			
			//make sure the "All" row is gone
			long nAllRow = m_ProviderCombo->FindByColumn(pccID, (long)-1, 0, VARIANT_FALSE);
			if (nAllRow != -1) {
				m_ProviderCombo->RemoveRow(nAllRow);
			}
			if (m_ProviderCombo->GetCurSel() == -1) {
				//there should be an entry here, allow an exception if not
				m_ProviderCombo->PutCurSel(0);
			}
		}

		long nProviderID = -1;
		if (m_ProviderCombo->GetCurSel() != -1) {
			nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
		}

		if(nProviderID != -1)
		{
			//filter on any bill that contains the selected provider, even if they may
			//also contain other providers
			CString strProviderWhere;
			// (j.jones 2011-08-16 17:22) - PLID 44805 - we will filter out "original" and "void" charges
			strProviderWhere.Format("BillID IN (SELECT BillID FROM ChargesT "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND ChargesT.DoctorsProviders = %li)", nProviderID);

			if(!strWhere.IsEmpty()) {
				strWhere += " AND ";
			}
			strWhere += strProviderWhere;
		}

		m_UnselectedBatchList->PutWhereClause(_bstr_t(strWhere));
		m_SelectedBatchList->PutWhereClause(_bstr_t(strWhere));

		m_UnselectedBatchList->Requery();
		m_SelectedBatchList->Requery();
	
	}NxCatchAll("Error in CEbillingFormDlg::RefilterLists");
}

//TES 1/13/2009 - PLID 24138 - Added; sends all claims in the current list to Paper Batch.  Basically copied from SendToHcfa()
void CEbillingFormDlg::OnSendListToHcfa()
{
	try {

		CString sql;
		if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) {
			return;
		}

		if(!CheckCurrentUserPermissions(bioEBilling,sptWrite)) {
			return;
		}

		// (j.jones 2008-02-11 16:48) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
		// (but you can unbatch claims)
		_DNxDataListPtr pList;
		if(m_CurrList == 0) {
			pList = m_UnselectedBatchList;
		} 
		else {
			pList = m_SelectedBatchList;
		}

		CString strIDList;
		std::vector<long> aryBillIDs; // (b.eyers 2015-10-06) - PLID 42101
		bool bOneFailed = false;
		for(int i = 0; i < pList->GetRowCount(); i++) {

			long nBillID = VarLong(pList->GetValue(i,COLUMN_BILL_ID));
			
			// (j.jones 2010-10-21 14:48) - PLID 41051 - this function can warn why the claim can't be created,
			// but does not do so here, we will warn later
			if(!CanCreateInsuranceClaim(nBillID, TRUE)) {
				bOneFailed = true;
			}
			else {
				strIDList += AsString(nBillID) + ",";
				aryBillIDs.push_back(nBillID); // (b.eyers 2015-10-06) - PLID 42101
			}
		}

		if(!strIDList.IsEmpty()) {
			strIDList.TrimRight(",");
			ExecuteSql("UPDATE HCFATrackT SET Batch = 1, CurrentSelect = 0 WHERE BillID IN (%s)", strIDList);
				
			AuditInsuranceBatch(aryBillIDs, 1, 2); // (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, electronic to paper

			pList->Requery();
		}

		if(bOneFailed) {
			// (j.jones 2010-10-21 14:54) - PLID 41051 - tweaked this message to reflect multiple failure possibilities
			AfxMessageBox("At least one claim was not batched either because no charges on the bill are batched, "
				"or because your preference to disallow batching or printing claims with no insurance responsibility has been enabled.");
		}
	} NxCatchAll(__FUNCTION__);

	RefreshTotals();

}

// (j.jones 2010-07-23 16:25) - PLID 34105 - only shown if assignment of benefits can be blank,
// clicking this button should explain why the warning is displayed
void CEbillingFormDlg::OnBtnWarnAssignmentOfBenefitsEbilling()
{
	try {

		BOOL bIsHCFA = (m_FormTypeCombo->CurSel != -1 && VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->CurSel,0),-1) == 1);
		BOOL bIsUB = (m_FormTypeCombo->CurSel != -1 && VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->CurSel,0),-1) == 2);

		if(!bIsHCFA && !bIsUB) {
			//how was this button accessible?
			ASSERT(FALSE);
			return;
		}

		CBlankAssignmentBenefitsDlg dlg(this);
		// (j.jones 2010-07-27 09:47) - PLID 39854 - show info. for only the selected form type
		dlg.m_babtType = bIsHCFA ? babtHCFA : babtUB;
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (d.singleton 2011-07-05) - PLID 44422 - added button to open file explorer to choose what error report to parse
// (j.jones 2014-07-24 10:56) - PLID 62535 - renamed this function
void CEbillingFormDlg::OnBtnAlbertaAssessmentFile()
{
	try {

		// (j.jones 2014-07-24 11:18) - PLID 62535 - this now calls the parser class
		CAlbertaAssessmentParser parser;
		parser.ParseFile(this);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2014-07-24 10:48) - PLID 62579 - moved the Alberta assessment parsing to its own class	

void CEbillingFormDlg::OnMarkClaimSentSelected()
{
	OnMarkClaimSent(m_SelectedBatchList);
}

void CEbillingFormDlg::OnMarkAllClaimsSentSelected()
{
	OnMarkAllClaimsSent(m_SelectedBatchList);
}

void CEbillingFormDlg::OnMarkClaimSentUnselected()
{
	OnMarkClaimSent(m_UnselectedBatchList);
}

void CEbillingFormDlg::OnMarkAllClaimsSentUnselected()
{
	OnMarkAllClaimsSent(m_UnselectedBatchList);
}

// (b.spivey, August 27th, 2014) - PLID 63492 - Previewing the claim file here. 
void CEbillingFormDlg::OnPreviewClaimFileSelected()
{
	try {
		long nRow = m_SelectedBatchList->GetCurSel();
		if (nRow != -1) {
			long nBillID = VarLong(m_SelectedBatchList->GetValue(nRow, COLUMN_BILL_ID), -1);
			OnPreviewClaimFile(nBillID);
		}

	}NxCatchAll(__FUNCTION__);
}

void CEbillingFormDlg::OnPreviewClaimFileUnselected()
{
	try {

		long nRow = m_UnselectedBatchList->GetCurSel();
		if (nRow != -1) {
			long nBillID = VarLong(m_UnselectedBatchList->GetValue(nRow, COLUMN_BILL_ID), -1);
			OnPreviewClaimFile(nBillID);
		}

	}NxCatchAll(__FUNCTION__); 

}

void CEbillingFormDlg::OnPreviewClaimFile(long nBillID)
{
	if (m_LocationCombo->GetCurSel() == -1) {
		return;
	}

	if (m_FormTypeCombo->GetCurSel() == -1) {
		return;
	}

	CEbilling dlg(this, true, nBillID);

	dlg.m_FormatID = FormatID;
	dlg.m_FormatStyle = FormatStyle;

	// (j.jones 2010-10-18 15:59) - PLID 40346 - changed boolean to an enum
	if (ClaimType == actInst) {
		dlg.m_actClaimType = actInst;
	}
	else {
		dlg.m_actClaimType = actProf;
	}

	dlg.m_nFilterByLocationID = VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(), 0), -1);

	long nProviderID = -1;
	if (m_ProviderCombo->GetCurSel() != -1) {
		nProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
	}
	dlg.m_nFilterByProviderID = nProviderID;

	CString str = "";
	str = GetRemotePropertyText(_T("ResSubmitter"), "", 0, _T("<None>"));
	if (str != "") {
		dlg.m_Contact = str;
	}

	str = GetRemotePropertyText(_T("SiteID"), "", 0, _T("<None>"));
	if (str != "") {
		dlg.m_SiteID = str;
	}

	int nResult = dlg.DoModal();

}

void CEbillingFormDlg::OnMarkClaimSent(NXDATALISTLib::_DNxDataListPtr p_List)
{
	try
	{	
		if(IDNO == MessageBox("Are you sure you want mark this claim as sent without printing it?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) 
		{
			return;
		}
		if(p_List->GetCurSel() == -1){
			return;
		}

		IRowSettingsPtr pRow = p_List->GetRow(p_List->GetCurSel());
		long nBillID = VarLong(pRow->GetValue(COLUMN_BILL_ID));
		long nInsuredPartyID = VarLong(pRow->GetValue(COLUMN_INSURED_PARTY_ID), -1);
		long nPatientID = VarLong(pRow->GetValue(COLUMN_PATIENT_ID));
		CString strPersonName = VarString(pRow->GetValue(COLUMN_PATIENT_NAME));
		CString strUsername = GetCurrentUserName();
		CString strAuditText = FormatString("Bill: \"%s\" was marked SENT without being printed for patient %s", VarString(pRow->GetValue(COLUMN_BILL_DESC)), strPersonName );

		long nHcfaTrackID = VarLong(pRow->GetValue(COLUMN_HCFA_ID));

		if(nInsuredPartyID == -1) 
		{
			MessageBox("Could not mark sent because there is no insured party associated with this claim.");
			return;
		}

		CParamSqlBatch sqlBatch;
		AddToClaimHistory(nBillID, nInsuredPartyID, ClaimSendType::SentWithoutPrint, "", FALSE, FALSE);	
		sqlBatch.Add(" DELETE FROM HCFATrackT WHERE ID = {INT} ", nHcfaTrackID);
		sqlBatch.Execute(GetRemoteData());
		p_List->Requery();
		CEbillingFormDlg::RefreshTotals();

		// (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, electronic to paper
		std::vector<long> aryBillID;
		aryBillID.push_back(nBillID);
		AuditInsuranceBatch(aryBillID, 0, 2);

		//Audit 
		long nAuditTransactionID = BeginAuditTransaction();
		AuditEvent(nPatientID, strPersonName, nAuditTransactionID, aeiClaimSentWithoutPrinting, nBillID, "", strAuditText, aepMedium, aetChanged);
		CommitAuditTransaction(nAuditTransactionID);
	}NxCatchAll(__FUNCTION__)
}

//(r.wilson 10/8/2012) plid 40712 - Takes in a datalist pointer and marks that list's current selection as Sent without printing
void CEbillingFormDlg::OnMarkAllClaimsSent(NXDATALISTLib::_DNxDataListPtr p_List)
{
	
	if(IDNO == MessageBox("Are you sure you want mark all claims as sent without printing them?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) 
	{
		return;
	}

	long nAuditTransactionID = -1;

	if(p_List->GetRowCount() > 0){
		nAuditTransactionID = BeginAuditTransaction();
	}

	CSqlFragment sqlFrag;
	CParamSqlBatch sqlBatch;	
	long nBillID = -1;
	long nInsuredPartyID = -1;
	long nPatientID = -1;
	CString strPersonName;
	CString strUsername = GetCurrentUserName();
	CString strAuditText;
	long nHcfaTrackID = -1;
	std::vector<long> aryBillID; // (b.eyers 2015-10-06) - PLID 42101

	try
	{	
		for(int i = 0; i < p_List->GetRowCount(); i++){

			nBillID = VarLong(p_List->GetValue(i, COLUMN_BILL_ID));
			nInsuredPartyID = VarLong(p_List->GetValue(i, COLUMN_INSURED_PARTY_ID), -1);
			nPatientID = VarLong(p_List->GetValue(i,COLUMN_PATIENT_ID));
			strPersonName = VarString(p_List->GetValue(i, COLUMN_PATIENT_NAME));
			strAuditText = FormatString("Bill: \"%s\" was marked SENT without being printed for patient %s", VarString(p_List->GetValue(i,COLUMN_BILL_DESC)), strPersonName );
			nHcfaTrackID = VarLong(p_List->GetValue(i, COLUMN_HCFA_ID));

			if(nInsuredPartyID > 0)
			{
				aryBillID.push_back(nBillID); // (b.eyers 2015-10-06) - PLID 42101
				AddToClaimHistory(nBillID, nInsuredPartyID, ClaimSendType::SentWithoutPrint, "", FALSE, FALSE);				
				sqlBatch.Add("DELETE FROM HCFATrackT WHERE ID = {INT};", nHcfaTrackID);
				AuditEvent(nPatientID,strPersonName, nAuditTransactionID, aeiClaimSentWithoutPrinting, -1, "", strAuditText, aepMedium, aetChanged);
			}														
		}
		
		sqlBatch.Execute(GetRemoteData());
		CommitAuditTransaction(nAuditTransactionID);

		// (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, electronic to unbatched
		AuditInsuranceBatch(aryBillID, 0, 2);

		p_List->Requery();
		CEbillingFormDlg::RefreshTotals();
	}
	NxCatchAllSilentCallThrow(if(nAuditTransactionID != -1)
			{
				RollbackAuditTransaction(nAuditTransactionID);
				AfxMessageBox("Could not process request.");
			}
	)
}

// (s.tullis 2014-08-08 10:16) - PLID 62780 -
void CEbillingFormDlg::UpdateAlbertaBillstatus(){

	MFCArray<long> arrBillID;
	
	long nAuditTransactionID=-1;
	// no items in the select list dont update
	if (m_SelectedBatchList->GetRowCount() == 0){
		return;
	}
	IRowSettingsPtr pRow;

	// Get all the ID's in the select list
	for (int i = 0; i < m_SelectedBatchList->GetRowCount(); i++){
		pRow = m_SelectedBatchList->GetRow(i);
		if (pRow){
			arrBillID.Add(VarLong(pRow->GetValue(COLUMN_BILL_ID)));
		}
	}
	
	TrySetAlbertaStatus(arrBillID, AlbertaBillingStatus::ePending);
}

//called when we would like to show the button to launch the Trizetto website,
//will show/hide based on whether any web passwords have been entered
void CEbillingFormDlg::TryShowTrizettoWebButton()
{
	try {

		bool bShouldShow = false;

		//find out if the configuration is for one global site ID
		NexTech_Accessor::ClearinghouseType eClearinghouseType = NexTech_Accessor::ClearinghouseType_TriZetto;
		long nLoginType = GetRemotePropertyInt("Clearinghouse_LoginType", 0, (long)eClearinghouseType, "<None>", true);
		//0 - global, 1 - indiv.
		if (nLoginType == 0)
		{
			//get the global site ID and web password
			CString strSiteID = GetRemotePropertyText("GlobalClearinghouseLoginName", "", (long)eClearinghouseType, "<None>", true);
			_variant_t varPassword = GetRemotePropertyImage("GlobalClearinghousePortalPassword", (long)eClearinghouseType, "<None>", true);

			if (!strSiteID.IsEmpty() && varPassword.vt != VT_EMPTY && varPassword.vt != VT_NULL) {
				if (!DecryptStringFromVariant(varPassword).IsEmpty()) {
					//they have a password (blank is invalid)
					bShouldShow = true;
				}
			}
		}
		else {
			//they have multiple site IDs configured,
			//does anybody have a site & and web password?

			NexTech_Accessor::_ClearinghousePortalLoginListPtr pLogins =
				GetAPI()->LoadAllClearinghousePortalLogins(GetAPISubkey(), GetAPILoginToken(), eClearinghouseType);

			Nx::SafeArray<IUnknown*> saLogins = pLogins->PortalLogins;
			if (saLogins != NULL && saLogins.GetSize() > 0) {
				for (int i = 0; i < (int)saLogins.GetSize() && !bShouldShow; i++)
				{
					NexTech_Accessor::_ClearinghousePortalLoginPtr pPortalLogin = saLogins[i];
					if (!((CString)(LPCTSTR)pPortalLogin->SiteID).IsEmpty()
						&& !((CString)(LPCTSTR)pPortalLogin->PortalPassword).IsEmpty())
					{
						bShouldShow = true;
						break;
					}
				}
			}
		}

		m_btnLaunchTrizettoWebsite.EnableWindow(bShouldShow ? TRUE : FALSE);
		m_btnLaunchTrizettoWebsite.ShowWindow(bShouldShow ? SW_SHOW : SW_HIDE);
		
	}NxCatchAll(__FUNCTION__);
}

void CEbillingFormDlg::OnBnClickedClearinghouseIntegrationSetupBtn()
{
	try
	{
		if (!CheckCurrentUserPermissions(bioClearinghouseIntegrationSettings, sptWrite))
		{
			return;
		}

		CEbillingClearinghouseIntegrationSetupDlg dlg(this);
		dlg.DoModal();

		// Found this necessary to dispatch ConfigRT update packets.
		PeekAndPump();

		//ensure the Trizetto buttons are either shown or hidden
		EnsureClearinghouseIntegrationControls();
	}
	NxCatchAll(__FUNCTION__);
}

void CEbillingFormDlg::OnBtnLaunchTrizettoWebsite()
{
	try {
		
		CString strSiteID = "";
		CString strPortalPassword = "";

		NexTech_Accessor::ClearinghouseType eClearinghouseType = NexTech_Accessor::ClearinghouseType_TriZetto;
		//find out if the configuration is for one global site ID
		long nLoginType = GetRemotePropertyInt("Clearinghouse_LoginType", 0, (long)eClearinghouseType, "<None>", true);
		//0 - global, 1 - indiv.
		if (nLoginType == 0)
		{
			//get the global site ID and web password
			strSiteID = GetRemotePropertyText("GlobalClearinghouseLoginName", "", (long)eClearinghouseType, "<None>", true);
			_variant_t varPassword = GetRemotePropertyImage("GlobalClearinghousePortalPassword", (long)eClearinghouseType, "<None>", true);
			if (varPassword.vt != VT_EMPTY && varPassword.vt != VT_NULL) {
				strPortalPassword = DecryptStringFromVariant(varPassword);
			}
			else {
				MessageBox("There is no portal password currently configured for your clearinghouse.\n\n"
					"Please review your credentials in the Clearinghouse Integration setup screen.", "Practice", MB_ICONINFORMATION | MB_OK);
				return;
			}
		}
		else {
			
			//they have multiple site IDs configured,
			//get all site & and web password combinations from the API

			NexTech_Accessor::_ClearinghousePortalLoginListPtr pLogins =
				GetAPI()->LoadAllClearinghousePortalLogins(GetAPISubkey(), GetAPILoginToken(), eClearinghouseType);

			Nx::SafeArray<IUnknown*> saLogins = pLogins->PortalLogins;

			if (saLogins == NULL || saLogins.GetSize() == 0) {
				MessageBox("Your clearinghouse is currently configured to have a different login per provider and location, but no portal passwords have been entered.\n\n"
					"Please review your credentials in the Clearinghouse Integration setup screen.", "Practice", MB_ICONINFORMATION | MB_OK);
				return;
			}

			std::map<long, NexTech_Accessor::_ClearinghousePortalLoginPtr> mapIndexToLogin;
			
			long nIndexID = 1000;

			CMenu mnu;
			mnu.CreatePopupMenu();

			for (int i = 0; i < (int)saLogins.GetSize(); i++)
			{
				nIndexID++;

				NexTech_Accessor::_ClearinghousePortalLoginPtr pPortalLogin = saLogins[i];
				mapIndexToLogin[nIndexID] = pPortalLogin;
				
				CString strLabel;
				strLabel.Format("%s - %s / %s", (LPCTSTR)pPortalLogin->SiteID, (LPCTSTR)pPortalLogin->LocationNames, (LPCTSTR)pPortalLogin->ProviderNames);

				mnu.AppendMenu(MF_ENABLED | MF_STRING | MF_BYPOSITION, nIndexID, strLabel);
			}

			CRect rc;
			m_btnLaunchTrizettoWebsite.GetWindowRect(&rc);

			int nRetIndex = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, rc.right, rc.top, this);
			if (nRetIndex > 1000) {
				NexTech_Accessor::_ClearinghousePortalLoginPtr pPortalLogin = mapIndexToLogin[nRetIndex];
				strSiteID = (LPCTSTR)pPortalLogin->SiteID;
				strPortalPassword = (LPCTSTR)pPortalLogin->PortalPassword;
			}
			else {
				return;
			}
		}

		//now log into Trizetto using their POST method

		CString strHTML;
		strHTML.Format("<html><body> "
			"<form id=\"loginform\" action=\"https://mytools.gatewayedi.com/logon.aspx\" method=\"post\">"
			"<input type=\"hidden\" name=\"SwitchUserID\" value=\"%s\"/>"
			"<input type=\"hidden\" name=\"UserPass\" value=\"%s\"/>"
			"</form>"
			"<script language=\"JavaScript\"> "
			"document.forms[\"loginform\"].submit(); "
			"</script></body></html>", strSiteID, strPortalPassword);
		
		//Now write the HTML to a temporary file. This is in the user's %temp% path.
		CString strFilename;
		HANDLE hFile = FileUtils::CreateTempFile(AsString((long)GetTickCount), "html", &strFilename, TRUE);
		CFile fOut(hFile);
		fOut.Write(strHTML, strHTML.GetLength());
		fOut.Close();
			
		if ((int)ShellExecute(GetSafeHwnd(), NULL, strFilename, NULL, "", SW_MAXIMIZE) < 32) {
			//try just launching the website
			ShellExecute(NULL, NULL, "https://mytools.gatewayedi.com/logon.aspx", NULL, NULL, SW_SHOW);

			//delete our file right now
			DeleteFileWhenPossible(strFilename);

			MessageBox("Could not automatically log in to TriZetto Provider Solutions, if this problem persists please contact Nextech for assistance.");
			return;
		}

		//delete the temp file in 2 minutes
		DeleteFileWhenPossible(strFilename, 120000);

	}NxCatchAll(__FUNCTION__)
}

//Checks to see if the user is about to auto-upload ANSI claims to Trizetto,
//and if so, do they have multiple SiteIDs configured. If they do, the claim
//list is validated to confirm the export qualifies for only one Site ID.
//If this returns true, the export can continue with the provided Site ID and password.
//If this returns false, the export should be cancelled.
//This function will have told the user why the claims could not be exported.
bool CEbillingFormDlg::GetAutoUploadCredentialsForClaims(NexTech_Accessor::_ClearinghouseLoginPtr& pClearinghouseLogin)
{
	//throw exceptions to the caller

	//this function is not needed if not exporting as ANSI
	if (FormatStyle != ANSI) {
		return true;
	}

	//return true if auto-uploads are disabled
	bool bIsIntegrationEnabled = GetRemotePropertyInt("EbillingClearinghouseIntegration_Enabled", FALSE, 0, "<None>", true) ? true : false;
	if (!bIsIntegrationEnabled) {
		return true;
	}

	NexTech_Accessor::ClearinghouseType eClearinghouseType = NexTech_Accessor::ClearinghouseType_TriZetto;
	//find out if the configuration is for one global site ID, if so it is faster
	//to look up ConfigRT than call the API
	long nLoginType = GetRemotePropertyInt("Clearinghouse_LoginType", 0, (long)eClearinghouseType, "<None>", true);
	//0 - global, 1 - indiv.
	if (nLoginType == 0)
	{
		pClearinghouseLogin = NexTech_Accessor::_ClearinghouseLoginPtr(__uuidof(NexTech_Accessor::ClearinghouseLogin));
		pClearinghouseLogin->LoginType = NexTech_Accessor::ClearinghouseLoginType_Global;

		//get the global site ID and web password
		pClearinghouseLogin->SiteID = _bstr_t(GetRemotePropertyText("GlobalClearinghouseLoginName", "", (long)eClearinghouseType, "<None>", true));
		if (pClearinghouseLogin->SiteID.length() == 0) {
			MessageBox("E-Billing Clearinghouse Integration is currently enabled, but there is no Login / Site ID entered.\n\n"
				"Please review your credentials in the Clearinghouse Integration setup screen.", "Practice", MB_ICONEXCLAMATION | MB_OK);
			return false;
		}
		_variant_t varPassword = GetRemotePropertyImage("GlobalClearinghousePassword", (long)eClearinghouseType, "<None>", true);
		if (varPassword.vt != VT_EMPTY && varPassword.vt != VT_NULL) {
			pClearinghouseLogin->password = _bstr_t(DecryptStringFromVariant(varPassword));
			if (pClearinghouseLogin->password.length() == 0)
			{
				MessageBox("E-Billing Clearinghouse Integration is currently enabled, but there is no password entered.\n\n"
					"Please review your credentials in the Clearinghouse Integration setup screen.", "Practice", MB_ICONEXCLAMATION | MB_OK);
				return false;
			}
		}

		_variant_t varPortalPassword = GetRemotePropertyImage("GlobalClearinghousePortalPassword", (long)eClearinghouseType, "<None>", true);
		if (varPortalPassword.vt != VT_EMPTY && varPortalPassword.vt != VT_NULL) {
			pClearinghouseLogin->PortalPassword = _bstr_t(DecryptStringFromVariant(varPortalPassword));
		}

		//if we get here, we're good to go, no need to validate anything else
		return true;
	}

	//if we get here, they have multiple site IDs and wish to auto-upload, so we need to
	//validate that their current list of claims can all be sent via only one Site ID

	//this is all in the API

	long nFilteredLocationID = -1;
	if (m_LocationCombo->GetCurSel() != -1) {
		nFilteredLocationID = VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(), 0), -1);
	}

	long nFilteredProviderID = -1;
	if (m_ProviderCombo->GetCurSel() != -1) {
		nFilteredProviderID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->GetCurSel(), pccID), -1);
	}

	NexTech_Accessor::ClaimFormType pClaimType = NexTech_Accessor::ClaimFormType_HCFA;
	if (ClaimType == actInst)
	{
		pClaimType = NexTech_Accessor::ClaimFormType_UB;
	}

	NexTech_Accessor::_ClearinghouseEbillingLookupResultPtr pResult =
		GetAPI()->GetClearinghouseLoginForEbillingBatch(GetAPISubkey(), GetAPILoginToken(), pClaimType,
			eClearinghouseType, _bstr_t(AsString(nFilteredLocationID)), _bstr_t(AsString(nFilteredProviderID)));

	if (pResult == NULL) {
		//should be impossible
		ThrowNxException("GetClearinghouseLoginForEbillingBatch returned NULL.");
	}

	if (pResult->loginInfo == NULL)
	{
		//show the error info
		CString strError = CString((LPCSTR)pResult->errorInfo);
		if (strError.IsEmpty()) {
			//how is this possible?
			ASSERT(FALSE);
			strError = "E-Billing Clearinghouse Integration is currently enabled, but there is an unknown problem with your credentials.\n\nPlease review your credentials in the Clearinghouse Integration setup screen.";
		}
		MessageBox(strError, "Practice", MB_ICONINFORMATION | MB_OK);
		return false;
	}
	else {
		pClearinghouseLogin = pResult->loginInfo;
		return true;
	}
}
// BatchPrintDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "BatchPrintDlg.h"
#include "HCFADlg.h"
#include "UB92.h"
#include "UB04.h"
#include "ADADlg.h"
#include "IDPADlg.h"
#include "NYWCDlg.h"
#include "MICRDlg.h"
#include "MICR2007Dlg.h"
#include "NxStandard.h"
#include "PracProps.h"
#include "GlobalReportUtils.h"
#include "ReportInfo.h"
#include "Reports.h"
#include "EbillingValidationDlg.h"
#include "EbillingValidationConfigDlg.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "PrintWaitDlg.h"
#include "NYMedicaidDlg.h"
#include "OHIPUtils.h"
#include "BlankAssignmentBenefitsDlg.h"
#include "AlbertaHLINKUtils.h"
#include <PrintUtils.h>
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 10:28) - PLID 28000 - Need to specify namespace
using namespace ADODB;

#define COLUMN_HCFA_ID					0
#define COLUMN_BILL_DATE				1
#define COLUMN_PATIENT_NAME				2
#define COLUMN_BILL_DESC				3
#define COLUMN_INSURED_PARTY_ID			4	// (j.jones 2009-09-01 08:54) - PLID 34749
#define COLUMN_INS_CO_ID				5
#define COLUMN_INS_CO_NAME				6
#define COLUMN_RESP_NAME				7
#define COLUMN_PROV_ID					8  //r.wilson (6/11/2012) PLID 48853 - Provider ID Field
#define COLUMN_PROV_NAME				9
#define COLUMN_CLAIM_PROV_ID			10 //r.wilson (6/11/2012) PLID 48853 - Claim Provider ID Field
#define COLUMN_CLAIM_PROV_NAME			11
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
// CBatchPrintDlg dialog
CBatchPrintDlg::CBatchPrintDlg(CWnd* pParent)
	: CNxDialog(CBatchPrintDlg::IDD, pParent)
{
//	m_preferralsRS = new CDaoRecordset(&m_database);
	//{{AFX_DATA_INIT(CEffectiveness)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Billing/Insurance_Billing/print_a_paper_hcfa_batch.htm";

	m_hIconCheck = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_GREEN_CHECK), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_hIconRedX = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_X), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_hIconGrayX = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_GRAY_X), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	m_hIconQuestion = (HICON)LoadImage(AfxGetApp()->m_hInstance,
		MAKEINTRESOURCE(IDI_BLUE_QUESTION), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
}

CBatchPrintDlg::~CBatchPrintDlg()
{
	DestroyIcon((HICON)m_hIconCheck);
	DestroyIcon((HICON)m_hIconRedX);
	DestroyIcon((HICON)m_hIconGrayX);
	DestroyIcon((HICON)m_hIconQuestion);
}

void CBatchPrintDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBatchPrintDlg)
	DDX_Control(pDX, IDC_BTN_CONFIGURE_CLAIM_VALIDATION, m_btnConfigureClaimValidation);
	DDX_Control(pDX, IDC_VALIDATE_SELECTED_PAPER_CLAIMS, m_btnValidateSelected);
	DDX_Control(pDX, IDC_VALIDATE_ALL_PAPER_CLAIMS, m_btnValidateAll);
	DDX_Control(pDX, IDC_VALIDATE_UNSELECTED_PAPER_CLAIMS, m_btnValidateUnselected);
	DDX_Control(pDX, IDC_PRINT_UNSELECTED_PAPER, m_btnPrintUnselected);
	DDX_Control(pDX, IDC_PRINT_SELECTED_LIST, m_btnPrintSelected);
	DDX_Control(pDX, IDC_PRINTBATCH, m_btnPrintBatch);
	DDX_Control(pDX, IDC_PRINTLIST, m_btnPrintAll);
	DDX_Control(pDX, IDC_RESET_UNSELECTED_PAPER, m_btnResetUnselected);
	DDX_Control(pDX, IDC_RESET_SELECTED_PAPER, m_btnResetSelected);
	DDX_Control(pDX, IDC_RESET, m_btnResetAll);
	DDX_Control(pDX, IDC_UNSELECT_ONE_HCFA, m_btnUnselectOne);
	DDX_Control(pDX, IDC_UNSELECT_ALL_HCFAS, m_btnUnselectAll);
	DDX_Control(pDX, IDC_SELECT_ALL_HCFAS, m_btnSelectAll);
	DDX_Control(pDX, IDC_SELECT_ONE_HCFA, m_btnSelectOne);
	DDX_Control(pDX, IDC_UNSELECTED_LABEL, m_nxstaticUnselectedLabel);
	DDX_Control(pDX, IDC_PAPER_UNSELECTED_TOTAL, m_nxstaticPaperUnselectedTotal);
	DDX_Control(pDX, IDC_SELECTED_LABEL, m_nxstaticSelectedLabel);
	DDX_Control(pDX, IDC_PAPER_SELECTED_TOTAL, m_nxstaticPaperSelectedTotal);
	DDX_Control(pDX, IDC_LABEL_WARN_ASSIGNMENT_OF_BENEFITS_PAPER, m_nxstaticAssignmentOfBenefitsWarningLabel);
	DDX_Control(pDX, IDC_BTN_WARN_ASSIGNMENT_OF_BENEFITS_PAPER, m_btnAssignmentOfBenefitsWarning);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBatchPrintDlg, CNxDialog)
	//{{AFX_MSG_MAP(CBatchPrintDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_REMOVEBUTTON, OnRemoveHCFA)
	ON_BN_CLICKED(IDC_PRINTLIST, OnPrintList)
	ON_BN_CLICKED(IDC_GOTOPATIENT, OnGoToPatient)
	ON_BN_CLICKED(IDC_OPENBUTTON, OnOpenHCFA)
	ON_COMMAND(ID_VALIDATE_CLAIM, OnValidateOne)
	ON_BN_CLICKED(IDC_SEND_TO_EBILL, SendToEBill)
	ON_BN_CLICKED(IDC_PRINTBATCH, OnPrintBatch)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_SELECT_ONE_HCFA, OnSelectOneHcfa)
	ON_BN_CLICKED(IDC_SELECT_ALL_HCFAS, OnSelectAllHcfas)
	ON_BN_CLICKED(IDC_UNSELECT_ONE_HCFA, OnUnselectOneHcfa)
	ON_BN_CLICKED(IDC_UNSELECT_ALL_HCFAS, OnUnselectAllHcfas)
	ON_BN_CLICKED(IDC_PRINT_SELECTED_LIST, OnPrintSelectedList)
	ON_BN_CLICKED(IDC_RESET_SELECTED_PAPER, OnResetSelectedPaper)
	ON_BN_CLICKED(IDC_RESET_UNSELECTED_PAPER, OnResetUnselectedPaper)
	ON_BN_CLICKED(IDC_PRINT_UNSELECTED_PAPER, OnPrintUnselectedPaper)
	ON_COMMAND(ID_SELECT_INSCO, OnSelectInsCo)
	ON_COMMAND(ID_SELECT_INSGROUP, OnSelectInsGroup)
	ON_BN_CLICKED(IDC_VALIDATE_UNSELECTED_PAPER_CLAIMS, OnValidateUnselectedPaperClaims)
	ON_BN_CLICKED(IDC_VALIDATE_ALL_PAPER_CLAIMS, OnValidateAllPaperClaims)
	ON_BN_CLICKED(IDC_VALIDATE_SELECTED_PAPER_CLAIMS, OnValidateSelectedPaperClaims)
	ON_BN_CLICKED(IDC_OPEN_HCFA, OnOpenHCFA)
	ON_BN_CLICKED(IDC_REMOVE_HCFA, OnRemoveHCFA)
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()	
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_CLAIM_VALIDATION, OnBtnConfigureClaimValidation)
	ON_COMMAND(ID_SEND_LIST_TO_EBILL, OnSendListToEBill)
	ON_COMMAND(ID_MARK_CLAIM_SENT_SELECTED, OnMarkClaimSentSelected)
	ON_COMMAND(ID_MARK_ALL_CLAIMS_SENT_SELECTED, OnMarkAllClaimsSentSelected)
	ON_COMMAND(ID_MARK_CLAIM_SENT_UNSELECTED, OnMarkClaimSentUnselected)
	ON_COMMAND(ID_MARK_ALL_CLAIMS_SENT_UNSELECTED, OnMarkAllClaimsSentUnselected)
	ON_BN_CLICKED(IDC_BTN_WARN_ASSIGNMENT_OF_BENEFITS_PAPER, OnBtnWarnAssignmentOfBenefitsPaper)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBatchPrintDlg message handlers

BOOL CBatchPrintDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
//	SetIcon(m_hIcon, TRUE);			// Set big icon
//	SetIcon(m_hIcon, FALSE);		// Set small icon

	// (j.jones 2008-02-11 16:15) - PLID 28847 - added bulk caching
	g_propManager.CachePropertiesInBulk("BatchPrintDlg", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'DisallowBatchingPatientClaims' OR "
		"Name = 'Use2007MICR' OR "
		"Name = 'AutoValidateBatchClaims' OR "
		// (j.jones 2008-02-12 11:35) - PLID 28848 - added HidePatientChargesOnClaims
		"Name = 'HidePatientChargesOnClaims' OR "
		// (j.jones 2010-11-08 14:54) - PLID 39620 - added Alberta option
		"Name = 'UseAlbertaHLINK' OR "
		"Name = 'Alberta_PatientULICustomField' OR "
		// (j.jones 2011-04-20 10:45) - PLID 41490 - added ability to ignore diagnosis codes not linked to charges
		"Name = 'SkipUnlinkedDiagsOnClaims' OR "
		"Name = 'Claim_AllowSentWithoutPrinting' " //(r.wilson 10/8/2012) plid 40712
		"OR Name = 'Claim_SendPatAddressWhenPOS12'	" // (j.jones 2013-04-24 17:09) - PLID 56453
		")",
		_Q(GetCurrentUserName()));

	// (j.jones 2008-05-07 11:28) - PLID 29854 - set some icons for modernization,
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

	m_btnPrintBatch.AutoSet(NXB_PRINT);

	m_btnConfigureClaimValidation.AutoSet(NXB_MODIFY);

	// (j.jones 2010-07-23 16:59) - PLID 34105 - hide the warning label by default,
	// will be shown again if needed in UpdateView
	m_nxstaticAssignmentOfBenefitsWarningLabel.SetColor(RGB(255,0,0));
	extern CPracticeApp theApp;
	m_nxstaticAssignmentOfBenefitsWarningLabel.SetFont(&theApp.m_boldFont);
	m_nxstaticAssignmentOfBenefitsWarningLabel.ShowWindow(SW_HIDE);
	m_btnAssignmentOfBenefitsWarning.SetIcon(IDI_BLUE_QUESTION);
	m_btnAssignmentOfBenefitsWarning.ShowWindow(SW_HIDE);

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

	m_FormTypeCombo = BindNxDataListCtrl(IDC_FORM_TYPE_COMBO,false);

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

	//TODO - If we make a Dental option in the license, have this use it.
	pRow = m_FormTypeCombo->GetRow(-1);
	pRow->PutValue(0,(long)3);
	pRow->PutValue(1,"ADA");
	m_FormTypeCombo->AddRow(pRow);

	pRow = m_FormTypeCombo->GetRow(-1);
	pRow->PutValue(0,(long)4);
	pRow->PutValue(1,"IDPA");
	m_FormTypeCombo->AddRow(pRow);

	pRow = m_FormTypeCombo->GetRow(-1);
	pRow->PutValue(0,(long)5);
	pRow->PutValue(1,"NYWC");
	m_FormTypeCombo->AddRow(pRow);

	pRow = m_FormTypeCombo->GetRow(-1);
	pRow->PutValue(0,(long)6);
	pRow->PutValue(1,"MICR");
	m_FormTypeCombo->AddRow(pRow);

	// (j.jones 2010-02-04 11:51) - PLID 37113 - supported the NY Medicaid form
	pRow = m_FormTypeCombo->GetRow(-1);
	pRow->PutValue(0,(long)7);
	pRow->PutValue(1,"NY Medicaid");
	m_FormTypeCombo->AddRow(pRow);

	m_UnselectedList = BindNxDataListCtrl(IDC_PAPER_UNSELECTED_BATCHLIST, false);	
	m_SelectedList = BindNxDataListCtrl(IDC_PAPER_SELECTED_BATCHLIST, false);
	m_LocationCombo = BindNxDataListCtrl(IDC_LOCATION_COMBO_PAPER);

	pRow = m_LocationCombo->GetRow(-1);
	pRow->PutValue(0,(long)-1);
	pRow->PutValue(1,_bstr_t("<All Locations>"));
	m_LocationCombo->AddRow(pRow);
	m_LocationCombo->TrySetSelByColumn(0,(long)-1);

	// (j.jones 2011-08-16 17:15) - PLID 44782 - I moved the from clause into code for readability
	//r.wilson (6/11/2012) PLID 48853 - Loaded Provider and ClaimProvider information 
	//(r.wilson 10/2/2012) plid 52970 - Replace hardcoded SendTypes with enumerated values
	// (j.jones 2014-07-08 16:51) - PLID 62568 - bills with an "on hold" status are hidden here
	CString strFromClauseBasic = "";
	strFromClauseBasic.Format("(SELECT BillsQ.FormType, BillsQ.Date, HCFATrackT.ID AS HCFAID, "
		"BillsQ.ID AS BillID, BillsQ.PatientID, BillsQ.Description AS BillDesc, "
		"PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle AS PatName, BillsQ.InsuredPartyID, "
		"InsuranceCoT.PersonID AS InsCoID, InsuranceCoT.Name AS InsCoName, RespTypeT.TypeName AS RespName, "
		"PatientsT.UserDefinedID, HCFATrackT.CurrentSelect, HCFATrackT.ValidationState, LastClaimQ.LastSendDate, "
		"(SELECT Count(ChargesT.ID) FROM ChargesT "
		"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"	WHERE Deleted = 0 AND Batched = 1 "
		"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"	AND ChargesT.BillID = BillsQ.ID) AS CountOfChargesBatched, "
		"(SELECT Count(ChargesT.ID) FROM ChargesT "
		"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
		"	LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"	LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"	WHERE Deleted = 0 "
		"	AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"	AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"	AND ChargesT.BillID = BillsQ.ID) AS CountOfCharges, "
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
		"	) END AS DocID, " //r.wilson (6/11/2012) PLID 48853 - Provider ID Column
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
		"	) END AS DocName, " //r.wilson (6/11/2012) PLID 48853 - Provider Column		
		"CASE WHEN CountOfClaimProvsQ.CountDocs = 1 THEN CountOfClaimProvsQ.FirstProviderID "
		"	ELSE -1  "
		"	END AS ClaimProvID, "
		"CASE WHEN CountOfClaimProvsQ.CountDocs = 1 THEN ("
		"	SELECT Last + ', ' + First + ' ' + Middle FROM PersonT "
		"	WHERE ID = CountOfClaimProvsQ.FirstProviderID "
		"	) END AS ClaimProvider " //r.wilson (6/11/2012) PLID 48853 - ClaimProvider Column
		""
		"FROM HCFATrackT "
		"INNER JOIN (SELECT BillsT.* FROM BillsT WHERE Deleted = 0) AS BillsQ ON HCFATrackT.BillID = BillsQ.ID "
		"LEFT JOIN BillStatusT ON BillsQ.StatusID = BillStatusT.ID "
		"INNER JOIN ChargesT ON BillsQ.ID = ChargesT.BillID "
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
		") AS CountOfDocsQ ON BillsQ.ID = CountOfDocsQ.BillID " //r.wilson (6/11/2012) PLID 48853 - Get # of providers associated with the selected bill
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
		"		WHERE LineItemT.Deleted = 0 AND "
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
		"	) AS CountClaimProvsQ " //r.wilson (6/11/2012) PLID 48853 - Get Claims Provider in
		"	GROUP BY BillID  "
		") AS CountOfClaimProvsQ ON BillsQ.ID = CountOfClaimProvsQ.BillID  "
		"LEFT JOIN InsuredPartyT ON BillsQ.InsuredPartyID = InsuredPartyT.PersonID "
		"LEFT JOIN (SELECT Max(Date) AS LastSendDate, BillID FROM ClaimHistoryT "
		"	WHERE SendType >= %li GROUP BY BillID) AS LastClaimQ ON BillsQ.ID = LastClaimQ.BillID "
		"LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON ChargesT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
		"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON ChargesT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
		"WHERE ChargesT.Batched = 1 AND LineItemQ.Deleted = 0 "
		"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
		"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
		"AND Coalesce(BillStatusT.Type, -1) != %li "
		"GROUP BY HCFATrackT.ID, BillsQ.FormType, BillsQ.ID, BillsQ.PatientID, BillsQ.Description, "
		"PersonT.[Last] + ', ' + PersonT.[First] + ' ' + PersonT.Middle, InsuranceCoT.PersonID, InsuranceCoT.Name, "
		"RespTypeT.TypeName, PatientsT.UserDefinedID, HCFATrackT.Batch, BillsQ.InsuredPartyID, HCFATrackT.CurrentSelect, "
		"HCFATrackT.ValidationState, LastClaimQ.LastSendDate, BillsQ.Date, CountOfDocsQ.CountDocs, CountOfClaimProvsQ.CountDocs, CountOfClaimProvsQ.FirstProviderID "
		"HAVING HCFATrackT.Batch != 2 AND HCFATrackT.CurrentSelect = [SELECTVALUE]) AS HCFAPaperQ",
		ClaimSendType::Electronic, EBillStatusType::OnHold);

	CString strFromClauseUnselected = strFromClauseBasic;
	CString strFromClauseSelected = strFromClauseBasic;
	strFromClauseUnselected.Replace("[SELECTVALUE]", "0");
	strFromClauseSelected.Replace("[SELECTVALUE]", "1");

	//set to be HCFA by default
	m_FormTypeCombo->TrySetSelByColumn(0, (long)1);

	//set the list's where clause, defaulting to HCFA
	CString strWhere;
	strWhere.Format("FormType = 1");

	m_UnselectedList->PutFromClause(_bstr_t(strFromClauseUnselected));
	m_UnselectedList->PutWhereClause(_bstr_t(strWhere));
	m_SelectedList->PutFromClause(_bstr_t(strFromClauseSelected));
	m_SelectedList->PutWhereClause(_bstr_t(strWhere));

	//do not requery yet, UpdateView() will do this

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBatchPrintDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CNxDialog::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CBatchPrintDlg::OnPaint() 
{
	CNxDialog::OnPaint();
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CBatchPrintDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CBatchPrintDlg::OnRemoveHCFA() 
{
	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) return;

	if(m_pList->GetCurSel()==-1) {
		AfxMessageBox("There is no claim selected");
		return;
	}

	// to Remove a claim from the list!
	try {	
		// (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, paper to unbatched
		long nBillID = m_pList->GetValue(m_pList->GetCurSel(), COLUMN_BILL_ID).lVal;
		std::vector<long> aryBillID;
		aryBillID.push_back(nBillID);
		AuditInsuranceBatch(aryBillID, 0, 1);

		ExecuteSql("DELETE FROM HCFATrackT WHERE ID = %li", m_pList->GetValue(m_pList->GetCurSel(),COLUMN_HCFA_ID).lVal);
		UpdateView();
	} NxCatchAll("Error in OnRemoveHCFA");

} // end Remove

BEGIN_EVENTSINK_MAP(CBatchPrintDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CBatchPrintDlg)
	ON_EVENT(CBatchPrintDlg, IDC_PAPER_SELECTED_BATCHLIST, 6 /* RButtonDown */, OnRButtonDownPaperSelectedBatchlist, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CBatchPrintDlg, IDC_PAPER_SELECTED_BATCHLIST, 3 /* DblClickCell */, OnDblClickCellPaperSelectedBatchlist, VTS_I4 VTS_I2)
	ON_EVENT(CBatchPrintDlg, IDC_PAPER_UNSELECTED_BATCHLIST, 3 /* DblClickCell */, OnDblClickCellPaperUnselectedBatchlist, VTS_I4 VTS_I2)
	ON_EVENT(CBatchPrintDlg, IDC_PAPER_UNSELECTED_BATCHLIST, 6 /* RButtonDown */, OnRButtonDownPaperUnselectedBatchlist, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CBatchPrintDlg, IDC_PAPER_SELECTED_BATCHLIST, 18 /* RequeryFinished */, OnRequeryFinishedPaperSelectedBatchlist, VTS_I2)
	ON_EVENT(CBatchPrintDlg, IDC_PAPER_UNSELECTED_BATCHLIST, 18 /* RequeryFinished */, OnRequeryFinishedPaperUnselectedBatchlist, VTS_I2)
	ON_EVENT(CBatchPrintDlg, IDC_FORM_TYPE_COMBO, 16 /* SelChosen */, OnSelChosenFormTypeCombo, VTS_I4)
	ON_EVENT(CBatchPrintDlg, IDC_LOCATION_COMBO_PAPER, 16 /* SelChosen */, OnSelChosenLocationComboPaper, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CBatchPrintDlg::OnPrintList() 
{
	if(m_FormTypeCombo->GetCurSel() == -1)
		return;

	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptRead)) return;
	// will print the list of ALL batched claims..
	
	CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(228)]);
	
	CString strWhere;
	CString str;
	str.Format(" {BillsT.FormType} = %li", m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal);
	strWhere += str;
	
	long nLocationID = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
	if(nLocationID != -1) {
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		str.Format(" AND BillID IN (SELECT BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND LineItemT.LocationID = %li)",nLocationID);
		strWhere += str;
	}

	infReport.SetExtraValue(strWhere);
	//Made new function for running reports - JMM 5-28-04
	RunReport(&infReport, true, this, "Paper Claim List");
	
}

void CBatchPrintDlg::OnOpenHCFA() 
{

	if(!CheckCurrentUserPermissions(bioClaimForms,sptRead)) return;

	_variant_t var;

	if(m_pList->GetCurSel()==-1) {
		AfxMessageBox("There is no claim selected");
		return;
	}

	// to open a claim from the list
	CString sql;
	try {	
		var = m_pList->GetValue(m_pList->GetCurSel(),COLUMN_HCFA_ID);

		//check to see what form type it is, then open that form

		// (j.jones 2008-02-11 16:36) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
		long nBillID = VarLong(m_pList->GetValue(m_pList->GetCurSel(),COLUMN_BILL_ID));
		long nInsuredPartyID = VarLong(m_pList->GetValue(m_pList->GetCurSel(), COLUMN_INSURED_PARTY_ID), -1);
		// (j.jones 2010-10-21 14:48) - PLID 41051 - this function now warns why the claim can't be created
		if(!CanCreateInsuranceClaim(nBillID, FALSE)) {
			return;
		}

		if (var.vt != VT_EMPTY) {

			long FormType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;

			switch(FormType) {
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

					//Use _PersonID_, not UserDefinedID
					hcfa.m_PatientID = m_pList->GetValue(m_pList->GetCurSel(),COLUMN_PATIENT_ID).lVal;
					hcfa.DoModal(nBillID);
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

					//TES 3/13/2007 - PLID 24993 - Choose the right UB form
					if(GetUBFormType() == eUB04) {//UB04
						CUB04Dlg dlg(this);
						dlg.m_PatientID = m_pList->GetValue(m_pList->GetCurSel(),COLUMN_PATIENT_ID).lVal;
						dlg.DoModal(nBillID);
					}
					else {
						CUB92Dlg dlg(this);
						dlg.m_PatientID = m_pList->GetValue(m_pList->GetCurSel(),COLUMN_PATIENT_ID).lVal;
						dlg.DoModal(nBillID);
					}
					break;
				}

				case 3: {
					// (j.armen 2014-03-05 09:17) - PLID 60784 - Fill BillID, PatientID in constructor
					CADADlg ada(this, nBillID,
						VarLong(m_pList->GetValue(m_pList->GetCurSel(),COLUMN_PATIENT_ID)));
					ada.DoModal();
					break;
				}

				case 4: {
					CIDPADlg idpa(this);

					//Use _PersonID_, not UserDefinedID
					idpa.m_PatientID = m_pList->GetValue(m_pList->GetCurSel(),COLUMN_PATIENT_ID).lVal;
					idpa.DoModal(nBillID);
					break;
				}

				case 5: {
					CNYWCDlg nywc(this);

					//Use _PersonID_, not UserDefinedID
					nywc.m_PatientID = m_pList->GetValue(m_pList->GetCurSel(),COLUMN_PATIENT_ID).lVal;
					nywc.DoModal(nBillID);
					break;
				}

				case 6: {

					// (j.jones 2007-05-09 10:23) - PLID 25550 - check the internal preference
					// for which MICR form to use

					if(GetRemotePropertyInt("Use2007MICR", 1, 0, "<None>", true) == 1) {

						CMICR2007Dlg micr(this);

						//Use _PersonID_, not UserDefinedID
						micr.m_PatientID = m_pList->GetValue(m_pList->GetCurSel(),COLUMN_PATIENT_ID).lVal;
						micr.DoModal(nBillID);
					}
					else {

						CMICRDlg micr(this);

						//Use _PersonID_, not UserDefinedID
						micr.m_PatientID = m_pList->GetValue(m_pList->GetCurSel(),COLUMN_PATIENT_ID).lVal;
						micr.DoModal(nBillID);
					}
					break;
				}

				case 7: {

					// (j.jones 2010-02-04 11:51) - PLID 37113 - supported the NY Medicaid form
					CNYMedicaidDlg nymcaid(this);
					//Use _PersonID_, not UserDefinedID
					nymcaid.m_PatientID = VarLong(m_pList->GetValue(m_pList->GetCurSel(),COLUMN_PATIENT_ID));
					nymcaid.DoModal(nBillID);
					break;
				}
			}			
			
			m_pList->Requery();
		}
	
	} NxCatchAll("Error in OnOpenHCFA");	 

} // end OnOpenHCFA

void CBatchPrintDlg::SendToEBill()
{
	try {

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
			nBillID = VarLong(m_UnselectedList->GetValue(m_UnselectedList->GetCurSel(),COLUMN_BILL_ID));
		}
		else {
			nBillID = VarLong(m_SelectedList->GetValue(m_SelectedList->GetCurSel(),COLUMN_BILL_ID));
		}

		// (j.jones 2010-10-21 14:48) - PLID 41051 - this function now warns why the claim can't be created
		if(!CanCreateInsuranceClaim(nBillID, FALSE)) {
			return;
		}

		ExecuteSql("UPDATE HCFATrackT SET Batch = 2, CurrentSelect = 0 WHERE ID = %li", m_pList->GetValue(m_pList->GetCurSel(),COLUMN_HCFA_ID).lVal);

		// (b.eyers 2015-10-06) - PLID 42101, audit insurance claim, paper to electronic
		std::vector<long> aryBillID;
		aryBillID.push_back(nBillID);
		AuditInsuranceBatch(aryBillID, 2, 1);


		m_pList->Requery();

		RefreshTotals();
	
	} NxCatchAll(_T("Error in sending claim to Ebilling batch."));	
}

void CBatchPrintDlg::OnPrintBatch() 
{
	try {

		if(m_FormTypeCombo->GetCurSel() == -1)
			return;

		if(m_LocationCombo->GetCurSel() == -1)
			return;

		// (z.manning, 05/16/2008) - PLID 30050 - Converted to NxDialog
		CPrintWaitDlg dlgWait(this);
		CString str;
		CRect rc;
		int count = m_SelectedList->GetRowCount();

		if(!CheckCurrentUserPermissions(bioClaimForms,sptRead))
			return;

		if(m_SelectedList->GetRowCount()==0) {
			AfxMessageBox("The are no claims selected.");
			return;
		}

		// (j.jones 2008-02-11 16:45) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
		if(GetRemotePropertyInt("DisallowBatchingPatientClaims",0,0,"<None>",TRUE) == 1) {
			//we have to check each bill
			for(int i=0; i<m_SelectedList->GetRowCount(); i++) {
				long nBillID = VarLong(m_SelectedList->GetValue(i, COLUMN_BILL_ID));
				// (j.jones 2010-10-21 14:48) - PLID 41051 - this function can warn why the claim can't be created,
				// but does not do so here, we will handle it ourselves
				if(!CanCreateInsuranceClaim(nBillID, TRUE)) {
					CString str;
					str.Format("The %s claim for patient %s will not be printed either because no charges on the bill are batched "
						"or because your preference to disallow batching or printing claims with no insurance responsibility has been enabled.\n\n"
						"Please unbatch this claim before printing the batch.",
						FormatDateTimeForInterface(VarDateTime(m_SelectedList->GetValue(i, COLUMN_BILL_DATE)), NULL, dtoDate),
						VarString(m_SelectedList->GetValue(i, COLUMN_PATIENT_NAME)));
					AfxMessageBox(str);
					return;
				}
			}
		}

		//warn about alignment issues
		//(r.wilson 10/2/2012) plid 52970 - Leaving this variable alone on purpose for this pl item
		long FormType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;

		// (j.jones 2007-04-25 11:43) - PLID 4758 - Handle having a default per workstation,
		// defaulting to the FormAlignT default printer if the per-workstation default doesn't exist.
		long nDefault = GetPropertyInt("DefaultFormAlignID", -1, FormType-1, FALSE);
		CString strDefault = "[Default] = 1";
		if(nDefault != -1)
			strDefault.Format("ID = %li", nDefault);

		if(IsRecordsetEmpty("SELECT * FROM FormAlignT WHERE %s AND FormID = %li", strDefault, FormType-1) &&
			IDYES == MessageBox("You have not saved this form's alignment settings. The printout may not be aligned correctly until you do this.\n"
				"To set this up properly, click \"Yes\", open up any claim form, and then click on the \"Align Form\" button. Create a printer, and check the \"Default For My Workstation\" box.\n"
				"Then configure the alignment as needed. Would you like to cancel printing and align the form now?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
		}

		// (j.jones 2007-03-01 08:57) - PLID 25015 - added preference to auto-validate claims before exporting
		if(GetRemotePropertyInt("AutoValidateBatchClaims", 1, 0, "<None>", true) == 1) {

			//Validate selected claims, find the result
			EPaperValidationResults epvrResult = Validate(-2);
			if(epvrResult == epvrFailedOther) {
				//really this shouldn't happen, it would be from a check such as permissions
				//that we already would have done. But if so, return silently.
				return;
			}
			else if(epvrResult == epvrFailedError) {
				//if this is returned, we displayed a notepad of errors/warnings,
				//and thus should ask if the user wishes to cancel
				//(Yes to cancel, so yes-click-happy users will not continue exporting without thinking.)
				if(IDYES == MessageBox("The Claim Validation check has generated a list of potential problems to be reviewed.\n"
					"Errors will likely cause claim rejections; Warnings may or may not cause a rejection.\n\n"
					"Would you like to cancel printing claims in order to correct these issues?","Practice",MB_YESNO|MB_ICONEXCLAMATION)) {
					return;
				}
			}
		}

		//DRT 6/3/03 - Going along with changes to the individual form printings, this dialog will follow the same settings.
		//		Lookup the saved settings for each type of form and load them for the printing.
		HGLOBAL hSavedDevMode = NULL, hSavedDevNames = NULL;
		HGLOBAL hOldDevMode = NULL, hOldDevNames = NULL;	//for saving the old settings
		CString strReg;
		strReg.Format("PrintSettingsGlobal\\%s", VarString(m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(), 1)));
		// (z.manning 2010-11-22 12:43) - PLID 40486 - Renamed this function
		LoadDevSettingsFromRegistryHKLM(strReg, hSavedDevNames, hSavedDevMode);

		m_printDlg = new CPrintDialog(FALSE, PD_ALLPAGES | PD_RETURNDC, NULL);
		// Initialize some of the fields in PRINTDLG structure.
		m_printDlg->m_pd.nMinPage = m_printDlg->m_pd.nMaxPage = 1;
		m_printDlg->m_pd.nFromPage = m_printDlg->m_pd.nToPage = 1;

		//DRT
		hOldDevMode = m_printDlg->m_pd.hDevMode;
		hOldDevNames = m_printDlg->m_pd.hDevNames;
		m_printDlg->m_pd.hDevMode = hSavedDevMode;
		m_printDlg->m_pd.hDevNames = hSavedDevNames;
		//

		if(m_printDlg->DoModal() == IDCANCEL) {
			delete m_printDlg;
			m_printDlg = NULL;
			return;
		}

		dlgWait.Create(IDD_PRINT_WAIT_DLG, this);	
		dlgWait.GetWindowRect(rc);
		dlgWait.SetWindowPos(&wndTopMost,
			(GetSystemMetrics(SM_CXFULLSCREEN) - rc.Width())/2, (GetSystemMetrics(SM_CYFULLSCREEN)-rc.Height())/2,
				0,0, SWP_NOSIZE | SWP_SHOWWINDOW);

		// (j.dinatale 2010-07-23) - PLID 39692 - We need to create a device context to pass around to the various dialogs
		HDC hdcPrn = NULL;
		CDC *pPrintDC = NULL;

		// (j.dinatale 2010-07-23) - PLID 39692 - Obtain a handle to the device context.
		hdcPrn = m_printDlg->GetPrinterDC();
		if(!hdcPrn)
			hdcPrn = m_printDlg->CreatePrinterDC();

		if(hdcPrn != NULL)
		{
			// (j.dinatale 2010-07-23) - PLID 39692 - create a new device context
			pPrintDC = new CDC;

			// (j.dinatale 2010-07-23) - PLID 39692 - attach it and start a new document
			pPrintDC->Attach(hdcPrn);
			pPrintDC->StartDoc("Claim Forms");
		}


		// (j.dinatale 2010-07-28) - PLID 39803 - keeps track of whether or not the job was aborted.
		bool bJobAborted = false;

		BeginWaitCursor();
		for (int i=0; i < count; i++) {

			// (j.dinatale 2010-07-28) - PLID 39803 - we already check what form type is at the beginning of the function call.
			// The form type is needed down below as well, but since the batch printing form is inactive while printing, the combo box cant really change.
			// There is no need to do this every time the loop executes.
			//check the form type, and print that form		
			//long FormType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;

			// if the job is aborted, break from the loop so that way we hit the bottom half of the function.
			// this is important also because we free the device context after the loop
			if(bJobAborted)
				break;

			switch(FormType) {
				case 1: {

					long nInsuredPartyID = VarLong(m_SelectedList->GetValue(i, COLUMN_INSURED_PARTY_ID), -1);

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
						CString str;
						str.Format("The %s claim for patient %s has an insurance company that is not configured in a HCFA group.\n"
							"You can set up a HCFA group in the HCFA tab of the Administrator module.\n\n"
							"This HCFA cannot be opened until the insurance company has been properly set up.",
							FormatDateTimeForInterface(VarDateTime(m_SelectedList->GetValue(i, COLUMN_BILL_DATE)), NULL, dtoDate),
							VarString(m_SelectedList->GetValue(i, COLUMN_PATIENT_NAME)));

						// (j.dinatale 2010-07-23) - PLID 39692 - if we reach here, we shouldnt print the batch. the user needs to correct something...
						//		so instead of printing only some of the batch, we wont print any of it, so abort the job and detach the printer and delte the handle. 
						//		They can correct the form that needs correction and rerun the batch. Unfortunately, we are also making entries
						//		 to ClaimHistoryT and HCFATrackingT and that will need to be corrected in another PL item.
						// (j.dinatale 2010-07-29) - PLID 39803 - moved the logic for enddoc/aborting to the bottom half of the function
						/*if(hdcPrn != NULL)
						{
							pPrintDC->AbortDoc();
							pPrintDC->Detach();

							delete pPrintDC;
						}*/

						// (j.dinatale 2010-07-28) - PLID 39803 - at this point, the job was aborted. So set our flag.
						bJobAborted = true;

						MsgBox(str);

						// (j.dinatale 2010-07-29) - PLID 39803 - lets break instead of returning, so we have a chance to run other things after this loop if we did abort
						break;
					}

					CHCFADlg hcfa(this);

					str.Format("Printing HCFA %d of %d", i+1, count);
					dlgWait.GetDlgItem(IDC_LABEL)->SetWindowText(str);
					dlgWait.RedrawWindow();

					hcfa.m_ShowWindowOnInit = FALSE;
					hcfa.m_printDlg = m_printDlg;
					hcfa.m_PatientID = m_SelectedList->GetValue(i, COLUMN_PATIENT_ID).lVal;
					hcfa.m_ID = m_SelectedList->GetValue(i, COLUMN_BILL_ID).lVal;
					hcfa.m_bPrintWarnings = FALSE;
					hcfa.Create(IDD_HCFA, this);

					// (j.dinatale 2010-07-23) - PLID 39692 - assign the dialog a device context since we now have a member variable in it to pass it around
					hcfa.m_pPrintDC = pPrintDC;

					// (j.dinatale 2010-07-28) - PLID 39803 - we dont want the dialog to update the claims tables since we may end up not printing.
					hcfa.m_bUpdateClaimsTables = false;

					hcfa.OnPrint();
					hcfa.DestroyWindow();
					break;
				}

				case 2: {

					long nInsuredPartyID = VarLong(m_SelectedList->GetValue(i, COLUMN_INSURED_PARTY_ID), -1);

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
						CString str;
						str.Format("The %s claim for patient %s has an insurance company is not configured in a UB group.\n"
							"You can set up a UB group in the UB tab of the Administrator module.\n\n"
							"This claim cannot be opened until the insurance company has been properly set up.",
							FormatDateTimeForInterface(VarDateTime(m_SelectedList->GetValue(i, COLUMN_BILL_DATE)), NULL, dtoDate),
							VarString(m_SelectedList->GetValue(i, COLUMN_PATIENT_NAME)));
						
						// (j.dinatale 2010-07-23) - PLID 39692 - if we reach here, we shouldnt print the batch. the user needs to correct something...
						//		so instead of printing only some of the batch, we wont print any of it, so abort the job and detach the printer and delte the handle. 
						//		They can correct the form that needs correction and rerun the batch. Unfortunately, we are also making entries
						//		 to ClaimHistoryT and HCFATrackingT and that will need to be corrected in another PL item.
						// (j.dinatale 2010-07-29) - PLID 39803 - moved the logic for enddoc/aborting to the bottom half of the function
						/*if(hdcPrn != NULL)
						{
							pPrintDC->AbortDoc();
							pPrintDC->Detach();

							delete pPrintDC;
						}*/

						// (j.dinatale 2010-07-28) - PLID 39803 - at this point, the job was aborted. So set our flag.
						bJobAborted = true;

						MsgBox(str);

						// (j.dinatale 2010-07-29) - PLID 39803 - lets break instead of returning, so we have a chance to run other things after this loop if we did abort
						break;
					}

					//TES 3/13/2007 - PLID 24993 - Choose the right UB Form
					if(GetUBFormType() == eUB04) {//UB04
						CUB04Dlg dlg(this);

						str.Format("Printing UB %d of %d", i+1, count);
						dlgWait.GetDlgItem(IDC_LABEL)->SetWindowText(str);
						dlgWait.RedrawWindow();

						dlg.m_ShowWindowOnInit = FALSE;
						dlg.m_printDlg = m_printDlg;
						dlg.m_PatientID = m_SelectedList->GetValue(i, COLUMN_PATIENT_ID).lVal;
						dlg.m_ID = m_SelectedList->GetValue(i, COLUMN_BILL_ID).lVal;
						dlg.m_bPrintWarnings = FALSE;
						dlg.Create(IDD_HCFA, this);

						// (j.dinatale 2010-07-23) - PLID 39692 - assign the dialog a device context since we now have a member variable in it to pass it around
						dlg.m_pPrintDC = pPrintDC;

						// (j.dinatale 2010-07-28) - PLID 39803 - we dont want the dialog to update the claims tables since we may end up not printing.
						dlg.m_bUpdateClaimsTables = false;

						dlg.OnClickPrint();
						dlg.DestroyWindow();
			
					}
					else {//UB92
						CUB92Dlg dlg(this);

						str.Format("Printing UB %d of %d", i+1, count);
						dlgWait.GetDlgItem(IDC_LABEL)->SetWindowText(str);
						dlgWait.RedrawWindow();

						dlg.m_ShowWindowOnInit = FALSE;
						dlg.m_printDlg = m_printDlg;
						dlg.m_PatientID = m_SelectedList->GetValue(i, COLUMN_PATIENT_ID).lVal;
						dlg.m_ID = m_SelectedList->GetValue(i, COLUMN_BILL_ID).lVal;
						dlg.m_bPrintWarnings = FALSE;
						dlg.Create(IDD_HCFA, this);

						// (j.dinatale 2010-07-23) - PLID 39692 - assign the dialog a device context since we now have a member variable in it to pass it around
						dlg.m_pPrintDC = pPrintDC;

						// (j.dinatale 2010-07-28) - PLID 39803 - we dont want the dialog to update the claims tables since we may end up not printing.
						dlg.m_bUpdateClaimsTables = false;

						dlg.OnClickPrint();
						dlg.DestroyWindow();
					}
					break;
				}

				case 3: {
					// (j.armen 2014-03-05 09:17) - PLID 60784 - Fill BillID, PatientID in constructor
					CADADlg ada(this,
						VarLong(m_SelectedList->GetValue(i, COLUMN_BILL_ID)),
						VarLong(m_SelectedList->GetValue(i, COLUMN_PATIENT_ID)));

					str.Format("Printing ADA %d of %d", i+1, count);
					dlgWait.GetDlgItem(IDC_LABEL)->SetWindowText(str);
					dlgWait.RedrawWindow();

					ada.m_ShowWindowOnInit = FALSE;
					ada.m_printDlg = m_printDlg;
					ada.m_bPrintWarnings = FALSE;
					ada.Create(IDD_HCFA, this);

					// (j.dinatale 2010-07-23) - PLID 39692 - assign the dialog a device context since we now have a member variable in it to pass it around
					ada.m_pPrintDC = pPrintDC;

					// (j.dinatale 2010-07-28) - PLID 39803 - we dont want the dialog to update the claims tables since we may end up not printing.
					ada.m_bUpdateClaimsTables = false;

					ada.OnClickPrint();
					ada.DestroyWindow();
					break;
				}

				case 4: {
					CIDPADlg idpa(this);

					str.Format("Printing IDPA %d of %d", i+1, count);
					dlgWait.GetDlgItem(IDC_LABEL)->SetWindowText(str);
					dlgWait.RedrawWindow();

					idpa.m_ShowWindowOnInit = FALSE;
					idpa.m_printDlg = m_printDlg;
					idpa.m_PatientID = m_SelectedList->GetValue(i, COLUMN_PATIENT_ID).lVal;
					idpa.m_ID = m_SelectedList->GetValue(i, COLUMN_BILL_ID).lVal;
					idpa.m_bPrintWarnings = FALSE;
					idpa.Create(IDD_HCFA, this);

					// (j.dinatale 2010-07-23) - PLID 39692 - assign the dialog a device context since we now have a member variable in it to pass it around
					idpa.m_pPrintDC = pPrintDC;

					// (j.dinatale 2010-07-28) - PLID 39803 - we dont want the dialog to update the claims tables since we may end up not printing.
					idpa.m_bUpdateClaimsTables = false;

					idpa.OnClickPrint();
					idpa.DestroyWindow();
					break;
				}

				case 5: {
					CNYWCDlg nywc(this);

					str.Format("Printing NYWC %d of %d", i+1, count);
					dlgWait.GetDlgItem(IDC_LABEL)->SetWindowText(str);
					dlgWait.RedrawWindow();

					nywc.m_ShowWindowOnInit = FALSE;
					nywc.m_printDlg = m_printDlg;
					nywc.m_PatientID = m_SelectedList->GetValue(i, COLUMN_PATIENT_ID).lVal;
					nywc.m_ID = m_SelectedList->GetValue(i, COLUMN_BILL_ID).lVal;
					nywc.m_bPrintWarnings = FALSE;
					nywc.Create(IDD_HCFA, this);

					// (j.dinatale 2010-07-23) - PLID 39692 - assign the dialog a device context since we now have a member variable in it to pass it around
					nywc.m_pPrintDC = pPrintDC;

					// (j.dinatale 2010-07-28) - PLID 39803 - we dont want the dialog to update the claims tables since we may end up not printing.
					nywc.m_bUpdateClaimsTables = false;

					nywc.OnClickPrint();
					nywc.DestroyWindow();
					break;
				}

				case 6: {

					// (j.jones 2007-05-09 10:24) - PLID 25550 - check the internal preference
					// for which MICR form to use

					if(GetRemotePropertyInt("Use2007MICR", 1, 0, "<None>", true) == 1) {

						CMICR2007Dlg micr(this);

						str.Format("Printing MICR %d of %d", i+1, count);
						dlgWait.GetDlgItem(IDC_LABEL)->SetWindowText(str);
						dlgWait.RedrawWindow();

						micr.m_ShowWindowOnInit = FALSE;
						micr.m_printDlg = m_printDlg;
						micr.m_PatientID = m_SelectedList->GetValue(i, COLUMN_PATIENT_ID).lVal;
						micr.m_ID = m_SelectedList->GetValue(i, COLUMN_BILL_ID).lVal;
						micr.m_bPrintWarnings = FALSE;
						micr.Create(IDD_HCFA, this);

						// (j.dinatale 2010-07-23) - PLID 39692 - assign the dialog a device context since we now have a member variable in it to pass it around
						micr.m_pPrintDC = pPrintDC;

						// (j.dinatale 2010-07-28) - PLID 39803 - we dont want the dialog to update the claims tables since we may end up not printing.
						micr.m_bUpdateClaimsTables = false;

						micr.OnClickPrint();
						micr.DestroyWindow();
					}
					else {

						CMICRDlg micr(this);

						str.Format("Printing MICR %d of %d", i+1, count);
						dlgWait.GetDlgItem(IDC_LABEL)->SetWindowText(str);
						dlgWait.RedrawWindow();

						micr.m_ShowWindowOnInit = FALSE;
						micr.m_printDlg = m_printDlg;
						micr.m_PatientID = m_SelectedList->GetValue(i, COLUMN_PATIENT_ID).lVal;
						micr.m_ID = m_SelectedList->GetValue(i, COLUMN_BILL_ID).lVal;
						micr.m_bPrintWarnings = FALSE;
						micr.Create(IDD_HCFA, this);

						// (j.dinatale 2010-07-23) - PLID 39692 - assign the dialog a device context since we now have a member variable in it to pass it around
						micr.m_pPrintDC = pPrintDC;

						// (j.dinatale 2010-07-28) - PLID 39803 - we dont want the dialog to update the claims tables since we may end up not printing.
						micr.m_bUpdateClaimsTables = false;

						micr.OnClickPrint();
						micr.DestroyWindow();
					}
					break;
				}

				case 7: {

					// (j.jones 2010-02-04 11:51) - PLID 37113 - supported the NY Medicaid form
					CNYMedicaidDlg nymcaid(this);
					str.Format("Printing NY Medicaid %d of %d", i+1, count);
					dlgWait.GetDlgItem(IDC_LABEL)->SetWindowText(str);
					dlgWait.RedrawWindow();

					nymcaid.m_ShowWindowOnInit = FALSE;
					nymcaid.m_printDlg = m_printDlg;
					nymcaid.m_PatientID = m_SelectedList->GetValue(i, COLUMN_PATIENT_ID).lVal;
					nymcaid.m_ID = m_SelectedList->GetValue(i, COLUMN_BILL_ID).lVal;
					nymcaid.m_bPrintWarnings = FALSE;
					nymcaid.Create(IDD_HCFA, this);

					// (j.dinatale 2010-07-23) - PLID 39692 - assign the dialog a device context since we now have a member variable in it to pass it around
					nymcaid.m_pPrintDC = pPrintDC;

					// (j.dinatale 2010-07-28) - PLID 39803 - we dont want the dialog to update the claims tables since we may end up not printing.
					nymcaid.m_bUpdateClaimsTables = false;

					nymcaid.OnClickPrint();
					nymcaid.DestroyWindow();
					break;
				}
			}

		}

		// (j.dinatale 2010-07-28) - PLID 39803 - We need to update ClaimsHistoryT and HCFATrackT with the correct info if we didnt abort.
		// This function is now split in two, the top half handles printing and the bottom half handles the after printing logic
		if(!bJobAborted){

			// (j.dinatale 2010-07-23) - PLID 39692 - after we print, lets make sure we end our document, detach, and delete our context if one was created
			// (j.dinatale 2010-07-29) - PLID 39803 - we need to do different things based on if the job aborted or not, if we end up here, then the job was successful, so enddoc
			if(hdcPrn != NULL){
				pPrintDC->EndDoc();
				pPrintDC->Detach();

				delete pPrintDC;
			}

			// get the current location, dont put this in the loop because it only needs to be run once
			long nLocation = GetCurrentLocation();

			// only grab the current username once, again dont put this in the loop, only run it once
			CString strUsername = GetCurrentUserName();

			// get the string name associated with the form type that we are running a batch for, 
			// all forms in the batch are the same so run this once
			CString strClaimType;
			switch(FormType)
			{
			case 1:
				// HCFA
				strClaimType  = "HCFA";
				break;
			case 2:
				// UB form, we need to determine which type
				if(GetUBFormType() == eUB04)
					strClaimType = "UB04";
				else
					strClaimType = "UB92";
				break;
			case 3:
				// ADA
				strClaimType = "ADA";
				break;
			case 4:
				// IDPA
				strClaimType = "IDPA";
				break;
			case 5:
				// NYWC
				strClaimType = "NYWC";
				break;
			case 6:
				// MIRC, there is a standard and 2007 version, both appear to use the same string name
				strClaimType = "MICR";
				break;
			case 7:
				// NY Medicaid
				strClaimType = "NY Medicaid";
				break;
			default:
				// if the case is not defined, then we dont know the name of the form, leave it as empty string
				 strClaimType = "";
			}

			for(int j = 0; j < count; j++)
			{
				// start a new note
				CString strNote = "";

				// get the Bill Id, Bill Description, Insured Party ID, HCFA ID, Insurance Comp Name, 
				// and Ins Comp ID from the current row in the selected datalist
				long nBillID = VarLong(m_SelectedList->GetValue(j, COLUMN_BILL_ID), -1);
				CString strBillDesc = VarString(m_SelectedList->GetValue(j, COLUMN_BILL_DESC), "");
				long nInsuredPartyID = VarLong(m_SelectedList->GetValue(j, COLUMN_INSURED_PARTY_ID), -1);
				long nPatientID = VarLong(m_SelectedList->GetValue(j, COLUMN_PATIENT_ID), -1);
				long nHCFAID = VarLong(m_SelectedList->GetValue(j, COLUMN_HCFA_ID), -1);
				CString strInsCoName = VarString(m_SelectedList->GetValue(j, COLUMN_INS_CO_NAME), "");
				long nInsCoID = VarLong(m_SelectedList->GetValue(j, COLUMN_INS_CO_ID), -1);

				// the service date will be filled in later
				CString strDate = "";

				// (j.jones 2013-01-23 09:19) - PLID 54734 - moved the claim history addition to its own function,
				// this also updates HCFATrackT.LastSendDate
				AddToClaimHistory(nBillID, nInsuredPartyID, (ClaimSendType::ESendType)FormType);

				//now add to patient history			
				// get the top line item for the bill, and use that as the service date
				// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
				_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 LineItemT.Date FROM LineItemT "
					"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
					"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
					"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
					"WHERE ChargesT.BillID = {INT} AND Batched = 1 AND Deleted = 0 "
					"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
					"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
					"ORDER BY LineItemT.Date ASC", nBillID);

				// fill in the service date
				if(!rs->eof) {
					COleDateTime dt;
					dt = AdoFldDateTime(rs, "Date");
					strDate.Format(", Service Date: %s", FormatDateTimeForInterface(dt));
				}
				rs->Close();

				// format our note and append the date
				strNote.Format("%s printed - Bill Description: '%s', Insurance Company: %s", strClaimType, strBillDesc,strInsCoName);
				strNote += strDate;

				// call create new mail sent function
				CreateNewMailSentEntry(nPatientID, strNote, "", "", strUsername, "", nLocation);
			}
		} else {
			// if we end up here, the job aborted, so we need to abort the print job instead of ending the doc
			if(hdcPrn != NULL)
			{
				pPrintDC->AbortDoc();
				pPrintDC->Detach();

				delete pPrintDC;
			}

			// then we need to let the user know that it was aborted.
			MsgBox(MB_OK|MB_ICONEXCLAMATION, "Errors were found in your claim that will cause a rejection,\n"
			"as a result, the batch printing has been aborted. Please correct the errors before printing again.","Paper Batch");
		}

		//DRT
		//save whatever the dialog knows
		hSavedDevMode = m_printDlg->m_pd.hDevMode;
		hSavedDevNames = m_printDlg->m_pd.hDevNames;

		// (z.manning 2010-11-22 12:43) - PLID 40486 - Renamed this function
		SaveDevSettingsToRegistryHKLM(strReg, hSavedDevNames, hSavedDevMode);
		//De-allocate our saved dev modes
		if (hSavedDevMode) {
			GlobalFree(hSavedDevMode);
			hSavedDevMode = NULL;
		}
		if (hSavedDevNames) {
			GlobalFree(hSavedDevNames);
			hSavedDevNames = NULL;
		}

		//Re-load the global settings here
		m_printDlg->m_pd.hDevNames = hOldDevNames;
		m_printDlg->m_pd.hDevMode = hOldDevMode;
		//

		dlgWait.DestroyWindow();
		EndWaitCursor();

		delete m_printDlg;
		m_printDlg = NULL;

		m_SelectedList->Requery();

	}NxCatchAll("Error in CBatchPrintDlg::OnPrintBatch");
}

void CBatchPrintDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	try {

		// (j.jones 2010-07-23 16:15) - PLID 34105 - if HCFA and not OHIP,
		// warn if any setup can have assignment of benefits not filled
		// (this does not account for individual bills with Box 13 overridden)
		BOOL bShowWarning = TRUE;

		// (j.jones 2010-07-27 09:26) - PLID 39854 - this is now supported on UBs as well
		BOOL bIsHCFA = (m_FormTypeCombo->CurSel != -1 && VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->CurSel,0),-1) == 1);
		BOOL bIsUB = (m_FormTypeCombo->CurSel != -1 && VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->CurSel,0),-1) == 2);

		// (j.jones 2010-11-03 15:11) - PLID 39620 - supported Alberta
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

		m_UnselectedList->Requery();
		m_SelectedList->Requery();

	}NxCatchAll(__FUNCTION__);
}

void CBatchPrintDlg::OnGoToPatient() {

	try {
		long nPatientID;
		nPatientID = m_pList->GetValue(m_pList->GetCurSel(),COLUMN_PATIENT_ID).lVal;
		if (nPatientID != -1) {
			//Set the active patient
			CMainFrame *pMainFrame;
			pMainFrame = GetMainFrame();
			if (pMainFrame != NULL) {
				
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

void CBatchPrintDlg::OnRButtonDownPaperSelectedBatchlist(long nRow, short nCol, long x, long y, long nFlags) 
{
	// Do nothing if the user didn't actually click in a cell
	if (nRow == -1 || nCol == -1) {
		return;
	}

	//first clear out the multi-selection, if one exists
	m_SelectedList->PutCurSel(-1);
	m_SelectedList->PutCurSel(nRow);

	m_pList = m_SelectedList;
	m_CurrList = 1;

	if (m_SelectedList->GetValue(nRow,COLUMN_HCFA_ID).vt != VT_EMPTY) {

		CPoint pt;
		GetCursorPos(&pt);
		
		long FormType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		int nFlags = MF_BYPOSITION;
		if(!g_pLicense->CheckForLicense(CLicense::lcEbill, CLicense::cflrSilent) || (FormType != 1 && FormType != 2))
			nFlags |= MF_DISABLED | MF_GRAYED;
		mnu.InsertMenu(nIndex++, nFlags, IDC_SEND_TO_EBILL, "&Send To E-Billing");
		//TES 1/13/2009 - PLID 24138 - Added an option to send the entire list to the E-Billing Batch
		mnu.InsertMenu(nIndex++, nFlags, ID_SEND_LIST_TO_EBILL, "Send &All Selected Claims To E-Billing");
		if(GetRemotePropertyInt("Claim_AllowSentWithoutPrinting", 0, 0, "<None>", false) == 1){
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_MARK_CLAIM_SENT_SELECTED,"&Mark As Sent"); //(r.wilson 10/8/2012) plid 40712
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_MARK_ALL_CLAIMS_SENT_SELECTED,"Mar&k All As Sent"); //(r.wilson 10/8/2012) plid 40712
		}
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_OPEN_HCFA, "&Open");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_VALIDATE_CLAIM, "&Validate Claim");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_REMOVE_HCFA, "&Remove from Batch");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_INSCO, "Unselect Claims with this &Insurance Co.");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_INSGROUP, "Unselect Claims with this Insurance Grou&p");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_GOTOPATIENT, "&Go To Patient");

		mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}
}

void CBatchPrintDlg::OnDblClickCellPaperSelectedBatchlist(long nRowIndex, short nColIndex) 
{
	OnUnselectOneHcfa();
}

void CBatchPrintDlg::OnDblClickCellPaperUnselectedBatchlist(long nRowIndex, short nColIndex) 
{
	OnSelectOneHcfa();
}

void CBatchPrintDlg::OnRButtonDownPaperUnselectedBatchlist(long nRow, short nCol, long x, long y, long nFlags) 
{
	// Do nothing if the user didn't actually click in a cell
	if (nRow == -1 || nCol == -1) {
		return;
	}

	//first clear out the multi-selection, if one exists
	m_UnselectedList->PutCurSel(-1);
	m_UnselectedList->PutCurSel(nRow);

	m_pList = m_UnselectedList;
	m_CurrList = 0;

	if (m_UnselectedList->GetValue(nRow,COLUMN_HCFA_ID).vt != VT_EMPTY) {

		long FormType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;

		CPoint pt;
		GetCursorPos(&pt);

		CMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
		long nIndex = 0;
		int nFlags = MF_BYPOSITION;
		if(!g_pLicense->CheckForLicense(CLicense::lcEbill, CLicense::cflrSilent) || (FormType != 1 && FormType != 2))
			nFlags |= MF_DISABLED | MF_GRAYED;
		mnu.InsertMenu(nIndex++, nFlags, IDC_SEND_TO_EBILL, "&Send To E-Billing");
		//TES 1/13/2009 - PLID 24138 - Added an option to send the entire list to the E-Billing Batch
		mnu.InsertMenu(nIndex++, nFlags, ID_SEND_LIST_TO_EBILL, "Send &All Unselected Claims To E-Billing");
		if(GetRemotePropertyInt("Claim_AllowSentWithoutPrinting", 0, 0, "<None>", false) == 1){
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_MARK_CLAIM_SENT_UNSELECTED,"&Mark As Sent"); //(r.wilson 10/8/2012) plid 40712
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_MARK_ALL_CLAIMS_SENT_UNSELECTED,"Mar&k All As Sent"); //(r.wilson 10/8/2012) plid 40712
		}
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_OPEN_HCFA, "&Open");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_VALIDATE_CLAIM, "&Validate Claim");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_REMOVE_HCFA, "&Remove from Batch");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_INSCO, "Select Claims with this &Insurance Co.");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, ID_SELECT_INSGROUP, "Select Claims with this Insurance Grou&p");
		mnu.InsertMenu(nIndex++, MF_BYPOSITION | MF_SEPARATOR);
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, IDC_GOTOPATIENT, "&Go To Patient");

		mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
	}	
}

void CBatchPrintDlg::OnRequeryFinishedPaperSelectedBatchlist(short nFlags) 
{
	for(int i=0;i<m_SelectedList->GetRowCount();i++) {

		//update the validation icon
		int v = m_SelectedList->GetValue(i,COLUMN_VALIDATION_STATE_VALUE).iVal;
		switch(v) {			
			case 1: //passed
				m_SelectedList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconCheck));
				break;
			case 2: //failed
				m_SelectedList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconRedX));
				break;
			case 3: //warning
				m_SelectedList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconQuestion));
				break;
			case 0: //untested
				m_SelectedList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconGrayX));
			default:
				break;
		}

		//(e.lally 2007-02-27) PLID 24773 - Format the Last Sent date to reflect the regional settings
		// (j.jones 2009-03-09 11:37) - PLID 33393 - we fixed this in the datalist now
		/*
		_variant_t varLastSent = m_SelectedList->GetValue(i,COLUMN_LAST_SEND_DATE);
		if(varLastSent.vt == VT_BSTR) {
			CString strLastSentDt = VarString(varLastSent);
			if(strLastSentDt != "Never"){
				//It is an actual date, format it to the regional settings
				COleDateTime dtLastSent;
				dtLastSent.ParseDateTime(strLastSentDt, NULL);
				strLastSentDt = FormatDateTimeForInterface(dtLastSent, NULL, dtoDate, false);
				m_SelectedList->PutValue(i,COLUMN_LAST_SEND_DATE, _bstr_t(strLastSentDt));
			}
		}
		*/

		//r.wilson (6/11/2012) PLID 48853 
		IRowSettingsPtr pRowCurrent = m_SelectedList->GetRow(i);

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
	str.Format("%li",m_SelectedList->GetRowCount());
	SetDlgItemText(IDC_PAPER_SELECTED_TOTAL,str);

	

	CRect rc;
	GetDlgItem(IDC_PAPER_SELECTED_TOTAL)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);

	CalcShowBatchedChargeCount();
}

void CBatchPrintDlg::OnRequeryFinishedPaperUnselectedBatchlist(short nFlags) 
{
	for(int i=0;i<m_UnselectedList->GetRowCount();i++) {

		//update the validation icon
		int v = m_UnselectedList->GetValue(i,COLUMN_VALIDATION_STATE_VALUE).iVal;
		switch(v) {			
			case 1: //passed
				m_UnselectedList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconCheck));
				break;
			case 2: //failed
				m_UnselectedList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconRedX));
				break;
			case 3: //warning
				m_UnselectedList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconQuestion));
				break;
			case 0: //untested
				m_UnselectedList->PutValue(i,COLUMN_VALIDATION_STATE_ICON,_variant_t((long)m_hIconGrayX));
			default:
				break;
		}

		//(e.lally 2007-02-27) PLID 24773 - Format the Last Sent date to reflect the regional settings
		// (j.jones 2009-03-09 11:37) - PLID 33393 - we fixed this in the datalist now
		/*
		_variant_t varLastSent = m_UnselectedList->GetValue(i,COLUMN_LAST_SEND_DATE);
		if(varLastSent.vt == VT_BSTR) {
			CString strLastSentDt = VarString(varLastSent);
			if(strLastSentDt != "Never"){
				//It is an actual date, format it to the regional settings
				COleDateTime dtLastSent;
				dtLastSent.ParseDateTime(strLastSentDt, NULL);
				strLastSentDt = FormatDateTimeForInterface(dtLastSent, NULL, dtoDate, false);
				m_UnselectedList->PutValue(i,COLUMN_LAST_SEND_DATE, _bstr_t(strLastSentDt));
			}
		}
		*/

		//r.wilson (6/11/2012) PLID 48853 
		IRowSettingsPtr pRowCurrent = m_UnselectedList->GetRow(i);

		//r.wilson (6/11/2012) PLID 48853 - This block will change the text color of the Claim Provider's Name if it's not the same 
		//                                   Provider in the row
		if(pRowCurrent != NULL)
		{	

			long nProviderID = VarLong(pRowCurrent->GetValue(COLUMN_PROV_ID),-1);
			long nClaimProviderID = VarLong(pRowCurrent->GetValue(COLUMN_CLAIM_PROV_ID), -1);
					
			if(nProviderID != nClaimProviderID && nProviderID != -1)
			{			
				// If the Provider and Claim Provider are different then make the Claim Provider's text Red
				pRowCurrent->PutCellForeColor(COLUMN_CLAIM_PROV_NAME,RGB(255,0,0));
				pRowCurrent->PutCellForeColorSel(COLUMN_CLAIM_PROV_NAME,RGB(255,0,0));
			}
		
		}

	}

	

		CString str;
		str.Format("%li",m_UnselectedList->GetRowCount());
		SetDlgItemText(IDC_PAPER_UNSELECTED_TOTAL,str);

	CRect rc;
	GetDlgItem(IDC_PAPER_UNSELECTED_TOTAL)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);

	CalcShowBatchedChargeCount();
}

void CBatchPrintDlg::RefreshTotals()
{
	CString str;
	str.Format("%li",m_SelectedList->GetRowCount());
	SetDlgItemText(IDC_PAPER_SELECTED_TOTAL,str);

	str.Format("%li",m_UnselectedList->GetRowCount());
	SetDlgItemText(IDC_PAPER_UNSELECTED_TOTAL,str);

	CRect rc;
	GetDlgItem(IDC_PAPER_SELECTED_TOTAL)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);

	GetDlgItem(IDC_PAPER_UNSELECTED_TOTAL)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);

	CalcShowBatchedChargeCount();
}

void CBatchPrintDlg::OnSelectOneHcfa() 
{
	try {

		if (m_UnselectedList->GetCurSel()!=-1)
			m_SelectedList->TakeCurrentRow(m_UnselectedList);

		UpdateCurrentSelect(1);

		RefreshTotals();

	}NxCatchAll("Error in OnSelectOne()");
}

void CBatchPrintDlg::OnSelectAllHcfas() 
{
	try {

		m_SelectedList->TakeAllRows(m_UnselectedList);

		UpdateCurrentSelect(1);

		RefreshTotals();

	}NxCatchAll("Error in OnSelectAll()");
}

void CBatchPrintDlg::OnUnselectOneHcfa() 
{
	try {

		if (m_SelectedList->GetCurSel()!=-1)
			m_UnselectedList->TakeCurrentRow(m_SelectedList);

		UpdateCurrentSelect(0);

		RefreshTotals();

	}NxCatchAll("Error in OnUnselectOne()");
}

void CBatchPrintDlg::OnUnselectAllHcfas() 
{
	try {

		m_UnselectedList->TakeAllRows(m_SelectedList);

		UpdateCurrentSelect(0);

		RefreshTotals();

	}NxCatchAll("Error in OnUnselectAll()");
}

//UpdateCurrentSelect will take in the ID of one of the lists (0 - Unselected, 1 - Selected)
//and check for any items that were just moved over from the other list. If any
//are marked as being on the other list, we update the data to reflect which list the item belongs in.
void CBatchPrintDlg::UpdateCurrentSelect(int iList)
{
	try {

		_variant_t var;

		if(iList==0) {
			//loop through the list, look for CurrentSelect = TRUE, and reverse it
			for(int i=0;i<m_UnselectedList->GetRowCount();i++) {
				var = m_UnselectedList->GetValue(i,COLUMN_CURRENT_SELECT);
				if(var.boolVal)
					ExecuteSql("UPDATE HCFATrackT SET CurrentSelect = 0 WHERE ID = %li",m_UnselectedList->GetValue(i,COLUMN_HCFA_ID).lVal);
				var.boolVal = FALSE;
				m_UnselectedList->PutValue(i,COLUMN_CURRENT_SELECT,var);

				//(r.wilson 6/27/2012) plid 48853
				IRowSettingsPtr pRowCurrent = m_UnselectedList->GetRow(i);
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
			for(int i=0;i<m_SelectedList->GetRowCount();i++) {
				var = m_SelectedList->GetValue(i,COLUMN_CURRENT_SELECT);
				if(!var.boolVal)
					ExecuteSql("UPDATE HCFATrackT SET CurrentSelect = 1 WHERE ID = %li",m_SelectedList->GetValue(i,COLUMN_HCFA_ID).lVal);
				var.boolVal = TRUE;
				m_SelectedList->PutValue(i,COLUMN_CURRENT_SELECT,var);

				//(r.wilson 6/27/2012) plid 48853
				IRowSettingsPtr pRowCurrent = m_SelectedList->GetRow(i);
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

void CBatchPrintDlg::OnPrintSelectedList() 
{
	if(m_FormTypeCombo->GetCurSel() == -1)
		return;

	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptRead)) return;
	// will print the list of SELECTED batched claims..

	if(m_SelectedList->GetRowCount()==0) {
		AfxMessageBox("The are no claims selected.");
		return;
	}
	
	CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(228)]);
	
	CString strWhere;
	CString str;
	str.Format(" {HCFATrackT.CurrentSelect} = 1 AND {BillsT.FormType} = %li", m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal);
	strWhere += str;
	
	long nLocationID = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
	if(nLocationID != -1) {
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		str.Format(" AND BillID IN (SELECT BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND LineItemT.LocationID = %li)",nLocationID);
		strWhere += str;
	}

	//this prompts the report to only show claims where CurrentSelect = TRUE, and the right form style
	infReport.SetExtraValue(strWhere);

	//Made new function for running reports - JMM 5-28-04
	RunReport(&infReport, true, this, "Paper Claim List");
	
}

void CBatchPrintDlg::OnPrintUnselectedPaper() 
{
	if(m_FormTypeCombo->GetCurSel() == -1)
		return;

	if(m_LocationCombo->GetCurSel() == -1)
		return;

	//this will print a list of only the unselected claims

	if(!CheckCurrentUserPermissions(bioClaimForms,sptRead)) return;

	if(m_UnselectedList->GetRowCount()==0) {
		AfxMessageBox("The are no claims selected.");
		return;
	}

	// will print the list of batched claims..
	CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(228)]);
	
	CString strWhere;
	CString str;
	str.Format(" {HCFATrackT.CurrentSelect} = 0 AND {BillsT.FormType} = %li", m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal);
	strWhere += str;
	
	long nLocationID = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
	if(nLocationID != -1) {
		// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
		str.Format(" AND BillID IN (SELECT BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
			"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
			"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
			"AND LineItemT.LocationID = %li)",nLocationID);
		strWhere += str;
	}

	//this prompts the report to only show claims where CurrentSelect = FALSE, and the right form style
	infReport.SetExtraValue(strWhere);

	//Made new function for running reports - JMM 5-28-04
	RunReport(&infReport, true, this, "Paper Claim List");
}

void CBatchPrintDlg::OnReset() 
{
	if(m_FormTypeCombo->GetCurSel() == -1)
		return;

	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) return;

	CString str;
	str.Format("Are you sure you want to Unbatch all %s claims?",CString(m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),1).bstrVal));

	if (MessageBox(str,"Question",MB_YESNO|MB_ICONQUESTION) == IDNO) return;

	try { 

		long FormType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;

		long nLocationID = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
		CSqlFragment sqlLocation("");
		if(nLocationID != -1) {
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			sqlLocation = CSqlFragment(" AND BillID IN (SELECT BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemT.LocationID = {INT})", nLocationID);
		}

		// (j.jones 2014-07-09 10:08) - PLID 62568 - do not touch on hold claims
		ExecuteParamSql("DELETE FROM HCFATrackT WHERE Batch = 1 AND BillID IN ("
			"SELECT BillsT.ID FROM BillsT "
			"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
			"WHERE BillsT.FormType = {INT} "
			"AND Coalesce(BillStatusT.Type, -1) != {CONST_INT} "
			") {SQL}", FormType, EBillStatusType::OnHold, sqlLocation);

		// (b.eyers 2015-10-06) - PLID 42101, mass audit insurance claim, paper to unbatched
		_DNxDataListPtr pList, pList2;
		pList = m_SelectedList;
		pList2 = m_UnselectedList;
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
			AuditInsuranceBatch(aryBillIDs, 0, 1);

	} NxCatchAll("An Error occured clearing the list.");

	m_UnselectedList->Requery();
	m_SelectedList->Requery();

}

void CBatchPrintDlg::OnResetSelectedPaper() 
{
	if(m_FormTypeCombo->GetCurSel() == -1)
		return;

	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) return;

	CString str;
	str.Format("Are you sure you want to Unbatch all selected %s claims?",CString(m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),1).bstrVal));

	if (MessageBox(str,"Question",MB_YESNO|MB_ICONQUESTION) == IDNO) return;

	try { 

		long FormType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;

		long nLocationID = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
		CSqlFragment sqlLocation("");
		if(nLocationID != -1) {
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			sqlLocation = CSqlFragment(" AND BillID IN (SELECT BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemT.LocationID = {INT})",nLocationID);
		}
		
		// (j.jones 2014-07-09 10:26) - PLID 62568 - do not remove on hold claims
		ExecuteParamSql("DELETE FROM HCFATrackT WHERE Batch = 1 AND CurrentSelect = 1 "
			"AND BillID IN (SELECT BillsT.ID FROM BillsT "
			"	LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
			"	WHERE BillsT.FormType = {INT} AND Coalesce(BillStatusT.Type, -1) != {CONST_INT} "
			") {SQL}", FormType, EBillStatusType::OnHold, sqlLocation);

		// (b.eyers 2015-10-06) - PLID 42101 - mass audit insurance claim, selected paper to unbatched
		_DNxDataListPtr pList;
		pList = m_SelectedList;
		std::vector<long> aryBillIDs;
		for (int i = 0; i < pList->GetRowCount(); i++) {
			long nBillID = VarLong(pList->GetValue(i, COLUMN_BILL_ID));
			aryBillIDs.push_back(nBillID);
		}
		if (aryBillIDs.size() > 0)
			AuditInsuranceBatch(aryBillIDs, 0, 1);

		UpdateView();
	} NxCatchAll("Error in clearing batch list.");
}

void CBatchPrintDlg::OnResetUnselectedPaper() 
{
	if(m_FormTypeCombo->GetCurSel() == -1)
		return;

	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptWrite)) return;

	CString str;
	str.Format("Are you sure you want to Unbatch all unselected %s claims?",CString(m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),1).bstrVal));

	if (MessageBox(str,"Question",MB_YESNO|MB_ICONQUESTION) == IDNO) return;

	try { 
		long FormType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;

		long nLocationID = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
		CSqlFragment sqlLocation("");
		if(nLocationID != -1) {
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			sqlLocation = CSqlFragment(" AND BillID IN (SELECT BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemT.LocationID = {INT})", nLocationID);
		}

		// (j.jones 2014-07-09 10:26) - PLID 62568 - do not remove on hold claims
		ExecuteParamSql("DELETE FROM HCFATrackT WHERE Batch = 1 AND CurrentSelect = 0 AND BillID IN ("
			"SELECT BillsT.ID FROM BillsT "
			"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
			"WHERE BillsT.FormType = {INT} "
			"AND Coalesce(BillStatusT.Type, -1) != {CONST_INT} "
			") {SQL}", FormType, EBillStatusType::OnHold, sqlLocation);

		// (b.eyers 2015-10-06) - PLID 42101 - mass audit insurance claim, unselected paper to unbatched
		_DNxDataListPtr pList;
		pList = m_UnselectedList;
		std::vector<long> aryBillIDs;
		for (int i = 0; i < pList->GetRowCount(); i++) {
			long nBillID = VarLong(pList->GetValue(i, COLUMN_BILL_ID));
			aryBillIDs.push_back(nBillID);
		}
		if (aryBillIDs.size() > 0)
			AuditInsuranceBatch(aryBillIDs, 0, 1);

		UpdateView();
	} NxCatchAll("Error in clearing batch list.");
	
}

void CBatchPrintDlg::OnSelChosenFormTypeCombo(long nRow) 
{
	if(nRow == -1) {
		m_FormTypeCombo->CurSel = 0;
		nRow = 0;
	}

	try {

		// (j.jones 2010-07-23 16:15) - PLID 34105 - if HCFA and not OHIP,
		// warn if any setup can have assignment of benefits not filled
		// (this does not account for individual bills with Box 13 overridden)
		BOOL bShowWarning = TRUE;
		// (j.jones 2010-07-27 09:26) - PLID 39854 - this is now supported on UBs as well
		BOOL bIsHCFA = (m_FormTypeCombo->CurSel != -1 && VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->CurSel,0),-1) == 1);
		BOOL bIsUB = (m_FormTypeCombo->CurSel != -1 && VarLong(m_FormTypeCombo->GetValue(m_FormTypeCombo->CurSel,0),-1) == 2);

		// (j.jones 2010-11-03 15:11) - PLID 39620 - supported Alberta
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

		//let the filter be generated and requeried just once
		OnSelChosenLocationComboPaper(m_LocationCombo->GetCurSel());

		CString strFormType;
		strFormType = CString(m_FormTypeCombo->GetValue(nRow,1).bstrVal);
		
		CString str;
		str.Format("Unselected Batched %s Claims:",strFormType);		
		SetDlgItemText(IDC_UNSELECTED_LABEL,str);
		CRect rc;		
		GetDlgItem(IDC_UNSELECTED_LABEL)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);
		str.Format("Selected Batched %s Claims:",strFormType);
		SetDlgItemText(IDC_SELECTED_LABEL,str);
		GetDlgItem(IDC_SELECTED_LABEL)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		str.Format("Unbatch\nUnselected\n%s Claims",strFormType);
		SetDlgItemText(IDC_RESET_UNSELECTED_PAPER,str);
		str.Format("Preview\nUnselected\n%s Claims",strFormType);
		SetDlgItemText(IDC_PRINT_UNSELECTED_PAPER,str);
		str.Format("Validate\nUnselected\n%s Claims",strFormType);
		SetDlgItemText(IDC_VALIDATE_UNSELECTED_PAPER_CLAIMS,str);

		str.Format("Unbatch\nAll\n%s Claims",strFormType);
		SetDlgItemText(IDC_RESET,str);
		str.Format("Preview\nAll Batched\n%s Claims",strFormType);
		SetDlgItemText(IDC_PRINTLIST,str);
		str.Format("Validate\nAll\n%s Claims",strFormType);
		SetDlgItemText(IDC_VALIDATE_ALL_PAPER_CLAIMS,str);

		str.Format("Unbatch\nSelected\n%s Claims",strFormType);
		SetDlgItemText(IDC_RESET_SELECTED_PAPER,str);
		str.Format("Preview\nSelected\n%s Claims",strFormType);
		SetDlgItemText(IDC_PRINT_SELECTED_LIST,str);
		str.Format("Validate\nSelected\n%s Claims",strFormType);
		SetDlgItemText(IDC_VALIDATE_SELECTED_PAPER_CLAIMS,str);

		str.Format("Print\n%s Batch",strFormType);
		SetDlgItemText(IDC_PRINTBATCH,str);
		

	}NxCatchAll("Error changing form type.");
}

void CBatchPrintDlg::OnSelectInsCo() {

	//select/unselect all claims with the same insurance company
	try {

		if(m_CurrList==0) {

			if(m_UnselectedList->GetCurSel()==-1) {
				AfxMessageBox("There is no claim selected");
				return;
			}

			//move all claims with this insurance company to the selected list
			long nInsCoID = VarLong(m_UnselectedList->GetValue(m_UnselectedList->GetCurSel(),COLUMN_INS_CO_ID),-1);
			for(int i=m_UnselectedList->GetRowCount()-1;i>=0;i--) {
				IRowSettingsPtr pRow = m_UnselectedList->GetRow(i);
				if(VarLong(pRow->GetValue(COLUMN_INS_CO_ID),-1) == nInsCoID)
					m_SelectedList->TakeRow(pRow);
			}

			UpdateCurrentSelect(1);

		}
		else {

			if(m_SelectedList->GetCurSel()==-1) {
				AfxMessageBox("There is no claim selected");
				return;
			}

			//move all claims with this insurance company to the unsselected list
			long nInsCoID = VarLong(m_SelectedList->GetValue(m_SelectedList->GetCurSel(),COLUMN_INS_CO_ID),-1);
			for(int i=m_SelectedList->GetRowCount()-1;i>=0;i--) {
				IRowSettingsPtr pRow = m_SelectedList->GetRow(i);
				if(VarLong(pRow->GetValue(COLUMN_INS_CO_ID),-1) == nInsCoID)
					m_UnselectedList->TakeRow(pRow);
			}

			UpdateCurrentSelect(0);
		}		

		RefreshTotals();
		
	} NxCatchAll("Error in OnSelectInsCo");
}

void CBatchPrintDlg::OnSelectInsGroup() {

	//select/unselect all claims with the same insurance group
	try {

		if(m_CurrList==0) {

			if(m_UnselectedList->GetCurSel()==-1) {
				AfxMessageBox("There is no claim selected");
				return;
			}

			//move all claims with this insurance group to the selected list
			long nInsCoID = VarLong(m_UnselectedList->GetValue(m_UnselectedList->GetCurSel(),COLUMN_INS_CO_ID),-1);
			long nInsGroup = -1;
			_RecordsetPtr rs = CreateRecordset("SELECT HCFASetupGroupID FROM InsuranceCoT WHERE PersonID = %li",nInsCoID);
			if(!rs->eof) {
				nInsGroup = AdoFldLong(rs, "HCFASetupGroupID",-1);
			}
			rs->Close();

			for(int i=m_UnselectedList->GetRowCount()-1;i>=0;i--) {
				IRowSettingsPtr pRow = m_UnselectedList->GetRow(i);
				long nSelInsCoID = VarLong(pRow->GetValue(COLUMN_INS_CO_ID),-1);
				if(!IsRecordsetEmpty("SELECT PersonID FROM InsuranceCoT WHERE HCFASetupGroupID = %li AND PersonID = %li",nInsGroup,nSelInsCoID)) {
					m_SelectedList->TakeRow(pRow);
				}
			}

			UpdateCurrentSelect(1);

		}
		else {

			if(m_SelectedList->GetCurSel()==-1) {
				AfxMessageBox("There is no claim selected");
				return;
			}

			//move all claims with this insurance group to the unsselected list
			long nInsCoID = VarLong(m_SelectedList->GetValue(m_SelectedList->GetCurSel(),COLUMN_INS_CO_ID),-1);
			long nInsGroup = -1;
			_RecordsetPtr rs = CreateRecordset("SELECT HCFASetupGroupID FROM InsuranceCoT WHERE PersonID = %li",nInsCoID);
			if(!rs->eof) {
				nInsGroup = AdoFldLong(rs, "HCFASetupGroupID",-1);
			}
			rs->Close();

			for(int i=m_SelectedList->GetRowCount()-1;i>=0;i--) {
				IRowSettingsPtr pRow = m_SelectedList->GetRow(i);
				long nSelInsCoID = VarLong(pRow->GetValue(COLUMN_INS_CO_ID),-1);
				if(!IsRecordsetEmpty("SELECT PersonID FROM InsuranceCoT WHERE HCFASetupGroupID = %li AND PersonID = %li",nInsGroup,nSelInsCoID)) {
					m_UnselectedList->TakeRow(pRow);
				}
			}

			UpdateCurrentSelect(0);
		}		

		RefreshTotals();
		
	} NxCatchAll("Error in OnSelectInsGroup");
}

void CBatchPrintDlg::OnSelChosenLocationComboPaper(long nRow) 
{
	try {

		if(nRow == -1)
			m_LocationCombo->SetSelByColumn(0,(long)-1);

		CString strWhere;
		strWhere.Format("FormType = %li",m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal);

		long nLocationID = VarLong(m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0),-1);

		if(nLocationID != -1) {
			CString str;
			// (j.jones 2011-08-16 17:15) - PLID 44782 - we will filter out "original" and "void" charges
			str.Format(" AND BillID IN (SELECT BillID FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
				"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
				"WHERE Deleted = 0 AND ChargesT.Batched = 1 "
				"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
				"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
				"AND LineItemT.LocationID = %li)",nLocationID);
			strWhere += str;
		}

		m_UnselectedList->PutWhereClause(_bstr_t(strWhere));
		m_SelectedList->PutWhereClause(_bstr_t(strWhere));
		m_UnselectedList->Requery();
		m_SelectedList->Requery();

	}NxCatchAll("Error changing location.");
}

void CBatchPrintDlg::CalcShowBatchedChargeCount() {

	try {

		//determine if we need to show the count of charges sent,
		//if so, show it in both lists for a uniform display

		BOOL bShowChargesBatched = FALSE;
		
		// (a.walling 2007-11-05 13:07) - PLID 27974 - VS2008 - for() loops
		int i = 0;

		for(i=0;i<m_UnselectedList->GetRowCount() && !bShowChargesBatched;i++) {
			IRowSettingsPtr pRow = m_UnselectedList->GetRow(i);

			//calculate the charges sent
			if(VarLong(pRow->GetValue(COLUMN_COUNT_CHARGES_BATCHED),0) < VarLong(pRow->GetValue(COLUMN_COUNT_CHARGES),0)) {
				bShowChargesBatched = TRUE;
			}
		}

		for(i=0;i<m_SelectedList->GetRowCount() && !bShowChargesBatched;i++) {
			IRowSettingsPtr pRow = m_SelectedList->GetRow(i);

			//calculate the charges sent
			if(VarLong(pRow->GetValue(COLUMN_COUNT_CHARGES_BATCHED),0) < VarLong(pRow->GetValue(COLUMN_COUNT_CHARGES),0)) {
				bShowChargesBatched = TRUE;
			}
		}

		if(bShowChargesBatched) {
			//if any charges batched are less than the total charges on the bill, show the charges batched column
			IColumnSettingsPtr pCol1 = m_UnselectedList->GetColumn(COLUMN_CHARGES_BATCHED);
			pCol1->PutStoredWidth(92);
			IColumnSettingsPtr pCol2 = m_SelectedList->GetColumn(COLUMN_CHARGES_BATCHED);
			pCol2->PutStoredWidth(92);
		}
		else {
			//otherwise we can hide it
			IColumnSettingsPtr pCol1 = m_UnselectedList->GetColumn(COLUMN_CHARGES_BATCHED);
			pCol1->PutStoredWidth(0);
			IColumnSettingsPtr pCol2 = m_SelectedList->GetColumn(COLUMN_CHARGES_BATCHED);
			pCol2->PutStoredWidth(0);
		}

	}NxCatchAll("Error displaying count of charges batched.");
}

// (j.jones 2007-03-01 08:59) - PLID 25015 - changed Validate to return an enum
EPaperValidationResults CBatchPrintDlg::Validate(long HCFAID) {

	if(m_LocationCombo->GetCurSel() == -1)
		return epvrFailedOther;

	if(m_FormTypeCombo->GetCurSel() == -1)
		return epvrFailedOther;

	if(!CheckCurrentUserPermissions(bioClaimForms,sptRead))
		return epvrFailedOther;

	CEbillingValidationDlg dlg(this);

	long nLocation = m_LocationCombo->GetValue(m_LocationCombo->GetCurSel(),0).lVal;
	long nClaimType = m_FormTypeCombo->GetValue(m_FormTypeCombo->GetCurSel(),0).lVal;

	// (j.jones 2009-08-14 13:50) - PLID 35235 - added provider filter for OHIP, not used here
	dlg.DoModal(HCFAID,-1,2,nLocation,-1,nClaimType,FALSE);

	UpdateView();

	if(dlg.m_bErrorListDisplayed)
		return epvrFailedError;
	else
		return epvrSuccess;
}

void CBatchPrintDlg::OnValidateOne() {

	if(m_LocationCombo->GetCurSel() == -1)
		return;

	try {
		
		long HCFAID = 0;

		if(m_CurrList == 0)
			HCFAID = m_UnselectedList->GetValue(m_UnselectedList->GetCurSel(),COLUMN_HCFA_ID).lVal;
		else
			HCFAID = m_SelectedList->GetValue(m_SelectedList->GetCurSel(),COLUMN_HCFA_ID).lVal;

		if(HCFAID > 0)
			Validate(HCFAID);
	
	}NxCatchAll("Error validating claim.");
}


void CBatchPrintDlg::OnValidateUnselectedPaperClaims() 
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(m_UnselectedList->GetRowCount() == 0) {
		AfxMessageBox("There are no unselected claims to validate!");
		return;
	}

	Validate(-1);
}

void CBatchPrintDlg::OnValidateAllPaperClaims() 
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(m_UnselectedList->GetRowCount() == 0 && m_SelectedList->GetRowCount() == 0) {
		AfxMessageBox("There are no batched claims to validate!");
		return;
	}
	
	Validate(-3);
}

void CBatchPrintDlg::OnValidateSelectedPaperClaims() 
{
	if(m_LocationCombo->GetCurSel() == -1)
		return;

	if(m_SelectedList->GetRowCount() == 0) {
		AfxMessageBox("There are no selected claims to validate!");
		return;
	}

	Validate(-2);
}

void CBatchPrintDlg::OnBtnConfigureClaimValidation() 
{
	CEbillingValidationConfigDlg dlg(this);
	dlg.m_bIsPaper = TRUE;
	dlg.DoModal();
}

//TES 1/13/2009 - PLID 24138 - Added; sends all claims in the current list to the E-Billing Batch. Basically copied from SendToEBill()
void CBatchPrintDlg::OnSendListToEBill()
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
			pList = m_UnselectedList;
		} 
		else {
			pList = m_SelectedList;
		}

		CString strIDList;
		bool bOneFailed = false;
		std::vector<long> aryBillIDs; // (b.eyers 2015-10-06) - PLID 42101 
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
			ExecuteSql("UPDATE HCFATrackT SET Batch = 2, CurrentSelect = 0 WHERE BillID IN (%s)", strIDList);

			AuditInsuranceBatch(aryBillIDs, 2, 1); // (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, paper to electronic
				
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

// (j.jones 2010-07-23 17:00) - PLID 34105 - only shown if assignment of benefits can be blank,
// clicking this button should explain why the warning is displayed
void CBatchPrintDlg::OnBtnWarnAssignmentOfBenefitsPaper()
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

//(r.wilson 10/8/2012) plid 40712 -
/*
#define COLUMN_HCFA_ID					0
#define COLUMN_BILL_DATE				1
#define COLUMN_PATIENT_NAME				2
#define COLUMN_BILL_DESC				3
#define COLUMN_INSURED_PARTY_ID			4	// (j.jones 2009-09-01 08:54) - PLID 34749
#define COLUMN_INS_CO_ID				5
#define COLUMN_INS_CO_NAME				6
#define COLUMN_RESP_NAME				7
#define COLUMN_PROV_ID					8  //r.wilson (6/11/2012) PLID 48853 - Provider ID Field
#define COLUMN_PROV_NAME				9
#define COLUMN_CLAIM_PROV_ID			10 //r.wilson (6/11/2012) PLID 48853 - Claim Provider ID Field
#define COLUMN_CLAIM_PROV_NAME			11
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

*/
void CBatchPrintDlg::OnMarkClaimSentSelected()
{
	OnMarkClaimSent(m_SelectedList);
}

//(r.wilson 10/8/2012) plid 40712 -
void CBatchPrintDlg::OnMarkAllClaimsSentSelected()
{
	OnMarkAllClaimsSent(m_SelectedList);
}

void CBatchPrintDlg::OnMarkClaimSentUnselected()
{
	OnMarkClaimSent(m_UnselectedList);
}

//(r.wilson 10/8/2012) plid 40712 -
void CBatchPrintDlg::OnMarkAllClaimsSentUnselected()
{
	OnMarkAllClaimsSent(m_UnselectedList);
}


//(r.wilson 10/8/2012) plid 40712 - Takes in a datalist pointer and marks that list's current selection as Sent without printing
void CBatchPrintDlg::OnMarkClaimSent(NXDATALISTLib::_DNxDataListPtr p_List)
{
	try
	{		
		if(IDNO == MessageBox("Are you sure you want mark this claim as sent without printing it?","Practice",MB_ICONEXCLAMATION|MB_YESNO)) 
		{
			return;
		}

		//(r.wilson 10/8/2012) plid 40712 - If somehow there is nothing selected then leave now
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
		CParamSqlBatch sqlBatch;

		//(r.wilson 10/8/2012) plid 40712 - This should not happen in the real world but I am doing it anyway. If somehow a claim has no insured party then we will skip then request
		//									to mark it as sent
		if(nInsuredPartyID == -1) 
		{
			MessageBox("Could not mark sent because there is no insured party associated with this claim.");
			return;
		}
		
		AddToClaimHistory(nBillID, nInsuredPartyID, ClaimSendType::SentWithoutPrint,"", FALSE, FALSE);
		sqlBatch.Add(" DELETE FROM HCFATrackT WHERE ID = {INT} ", nHcfaTrackID);
		sqlBatch.Execute(GetRemoteData());

		//(r.wilson 10/8/2012) plid 40712 - Refresh the datalist and other parts of the UI
		p_List->Requery();
		RefreshTotals();

		// (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, paper to unbatched
		std::vector<long> aryBillID;
		aryBillID.push_back(nBillID);
		AuditInsuranceBatch(aryBillID, 0, 1);


		//(r.wilson 10/8/2012) plid 40712 - Audit 
		long nAuditTransactionID = BeginAuditTransaction();
		AuditEvent(nPatientID, strPersonName, nAuditTransactionID, aeiClaimSentWithoutPrinting, nBillID, "", strAuditText, aepMedium, aetChanged);
		CommitAuditTransaction(nAuditTransactionID);
	}NxCatchAll(__FUNCTION__)
}

void CBatchPrintDlg::OnMarkAllClaimsSent(NXDATALISTLib::_DNxDataListPtr p_List)
{

	CSqlFragment sqlFrag;
	CParamSqlBatch sqlBatch;
	long nAuditTransactionID = -1;
	
	if(p_List->GetRowCount() > 0){
		nAuditTransactionID = BeginAuditTransaction();
	}
	
	long nBillID;
	long nInsuredPartyID;
	long nPatientID;
	CString strPersonName;
	CString strUsername = GetCurrentUserName();
	CString strAuditText;
	long nHcfaTrackID;
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
				sqlBatch.Add("DELETE FROM HCFATrackT WHERE ID = {INT};", nHcfaTrackID);
				AddToClaimHistory(nBillID, nInsuredPartyID, ClaimSendType::SentWithoutPrint, "", FALSE, FALSE);
				AuditEvent(nPatientID,strPersonName, nAuditTransactionID, aeiClaimSentWithoutPrinting, -1, "", strAuditText, aepMedium, aetChanged);
			}								
		}

		sqlBatch.Execute(GetRemoteData());
		CommitAuditTransaction(nAuditTransactionID);

		// (b.eyers 2015-10-06) - PLID 42101 - audit insurance claim, paper to unbatched
		AuditInsuranceBatch(aryBillID, 0, 1);

		p_List->Requery();
		RefreshTotals();
	}
	NxCatchAllSilentCallThrow(if(nAuditTransactionID != -1)
			{
				RollbackAuditTransaction(nAuditTransactionID);
				AfxMessageBox("Could not process request.");
			}
	)
}
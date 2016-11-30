/************************************************************************
FinancialDlg.cpp : implementation file

Keyboard interface command quick reference

Up arrow - move brightened row up
Down arrow - move brightened row down
Right arrow - expand line
Left arrow - collapse line
Shift key - Once to select a line to begin a drag
and drop, and once again to end it.
Backspace - The same as a right click on the brightened row


This source contains code for the financial tab of the patients
module. All items that appear in the list are in an UNBOUND NxDataList
filled with a CPtrArray named after the now defunct FinancialTabInfoT
table. All the member queries add items to this table or its temporary
counterparts.

************************************************************************/

#include "stdafx.h"
#include "FinancialDlg.h"
#include "FinancialApply.h"
#include "GlobalFinancialUtils.h"
#include "GlobalReportUtils.h"
#include "GlobalDataUtils.h"
#include "PaymentDlg.h"
#include "ApplyListDlg.h"
#include "ApplyManagerDlg.h"
#include "PatientView.h"
#include "Reports.h"
#include "GoStatements.h"
#include "ReportInfo.h"
#include "Barcode.h"
#include "EditComboBox.h"
#include "MsgBox.h"
#include "Packages.h"
#include "ResponsiblePartyDlg.h"
#include "nxmessagedef.h"
#include "EditChargeRespDetailDlg.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "NotesDlg.h"
#include "NxModalParentDlg.h"
#include "FinancialLineItemPostingDlg.h"
#include "ChooseDragRespDlg.h"
#include "InvUtils.h"
#include "HCFADlg.h"
#include "UB92.h"
#include "UB04.h"
#include "ADADlg.h"
#include "IDPADlg.h"
#include "NYWCDlg.h"
#include "MICRDlg.h"
#include "MICR2007Dlg.h"
#include "AuditTrail.h"
#include "DontShowDlg.h"
#include "SalesReceiptConfigDlg.h"
#include "ApplyToRefundDlg.h"
#include "SerializedItemsDlg.h"
#include "NYMedicaidDlg.h"
#include "SingleSelectDlg.h"
#include "BillDiscountOverviewDlg.h"
#include "Rewards.h"
#include "GlassesOrderHistoryDlg.h"
#include "FinancialCorrection.h"
#include "InsuranceReversalDlg.h"
#include <NxUILib\NxMenuCommandMap.h>
#include "BillingModuleDlg.h"
#include "PracticeRc.h"
#include "EBilling.h"
#include "Packages.h"
#include "PendingQuotesDlg.h"
#include "SearchBillingTabDlg.h"

using namespace ADODB;
using namespace NXDATALIST2Lib;

// (j.dinatale 2010-10-28) - PLID 28773 - Replaced all the enums with member variables which denote the indices of the columns
// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



extern CPracticeApp theApp;

// Compiler defines ////////////////////////////////////
// CH //////////////////////////////////////////////////
#define KEYBOARD_INTERFACE_ENABLED
////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define INVALID_PATIENT_ID	-100

#define BLANK_LINE "Blank"

#define ID_RUNDETAIL	5
#define ID_RUNSUMMARY	6
#define ID_RUNPREVIEW   7

#define IDT_SHOW_WARNING	46080

void FinTabItemCopy(FinTabItem* src, FinTabItem* dest);

// (j.jones 2010-05-07 08:20) - PLID 37398 - added enums for balance filter columns & types
enum BalanceFilterComboColumns {

	bfccDesc = 0,
	bfccLocID,
	bfccLocName,
	bfccBalanceType,
};

enum BalanceFilterBalanceType {

	bfbtTotal = 0,
	bfbtPatient,
	bfbtInsurance,
};

// (j.jones 2010-06-18 10:33) - PLID 39150 - converted to a class
FinTabItem::FinTabItem() {

	LineID = g_cvarNull;
	LineType = g_cvarNull;
	Expandable = g_cvarNull;
	ExpandChar = g_cvarNull;
	ExpandSubChar = g_cvarNull;
	Date = g_cvarNull;
	NoteIcon = g_cvarNull;
	HasNotes = g_cvarNull;
	// (a.wilson 2014-07-24 11:26) - PLID 63015
	OnHold = g_cvarNull;
	// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
	IsVoided = g_cvarNull;
	IsCorrected = g_cvarNull;
	Description = g_cvarNull;
	ShowChargeAmount = g_cvarNull;
	BillID = g_cvarNull;
	ChargeID = g_cvarNull;
	PayID = g_cvarNull;
	PayToPayID = g_cvarNull;
	ApplyID = g_cvarNull;
	ShowBalanceAmount = g_cvarNull;
	ChargeAmount = g_cvarNull;
	// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field
	TotalPayAmount = g_cvarNull;
	PayAmount = g_cvarNull;
	Adjustment = g_cvarNull;
	Refund = g_cvarNull;
	BalanceAmount = g_cvarNull;
	InsResp1 = g_cvarNull;
	InsResp2 = g_cvarNull;
	InsRespOther = g_cvarNull;
	PatResp = g_cvarNull;
	IsPrePayment = g_cvarNull;
	Discount = g_cvarNull;
	HasPercentOff = g_cvarNull;
	Provider = g_cvarNull;
	Location = g_cvarNull;
	POSName = g_cvarNull;
	POSCode = g_cvarNull;
	InputDate = g_cvarNull;
	CreatedBy = g_cvarNull;
	FirstChargeDate = g_cvarNull;
	DiagCode1 = g_cvarNull;
	DiagCode1WithName = g_cvarNull;
	DiagCodeList = g_cvarNull;
	// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
	// and added ChargeAllowable
	FeeSchedAllowable = g_cvarNull;
	ChargeAllowable = g_cvarNull;
	// (j.jones 2011-12-20 11:30) - PLID 47119 - added deductible and coinsurance
	ChargeDeductible = g_cvarNull;
	ChargeCoinsurance = g_cvarNull;
	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
	InvoiceNumber = g_cvarNull;
}

// (j.jones 2010-06-18 10:23) - PLID 39150 - clear the arypDynInsResps array
FinTabItem::~FinTabItem() {

	for(int i=arypDynInsResps.GetSize()-1;i>=0;i--) {
		delete (DynInsResp*)arypDynInsResps.GetAt(i);
		arypDynInsResps.RemoveAt(i);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFinancialDlg dialog

// (j.jones 2011-07-15 12:32) - PLID 43117 - needs to be dynamic
IMPLEMENT_DYNAMIC(CFinancialDlg, CNxDialog)

// (j.jones 2015-03-16 14:46) - PLID 64926 - added the search dialog, and
// converted Packages & Quotes to be references
CFinancialDlg::CFinancialDlg(CWnd* pParent)
: CPatientDialog(CFinancialDlg::IDD, pParent),
m_dlgPackages(*(new CPackages(this))),
m_dlgQuotes(*(new CPendingQuotesDlg(this))),
m_dlgSearchBillingTab(*(new CSearchBillingTabDlg(this))),
m_LocationChecker(NetUtils::LocationsT),
// (j.dinatale 2010-11-02) - PLID 28773 - no longer need this table checker
//m_ConfigBillColumnsChecker(NetUtils::ConfigBillColumnsT),
m_RespTypeChecker(NetUtils::RespTypeT)
{
	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "Billing/Add_Charges_to_Bills.htm";
	m_iCurPatient = -25;//if you remove this, you will crash if it equals the current patient the first time they load the tab -BVB
	//{{AFX_DATA_INIT(CFinancialDlg)
	m_iLastPatID = -1;
	m_iCurrentOrder = 0;
	m_boAllowUpdate = TRUE;
	m_iInsuranceViewToggle = 0;
	// (d.thompson 2008-12-09 10:25) - PLID 32370
	m_nShowRefunds = 0;
	m_bIsScreenEnabled = TRUE;
	m_SavedRow_LineID = 0;
	m_RedrawRefCount = 0;
	m_SavedTopRow_LineID = 0;
	m_bOneClickExpansion = FALSE;
	m_bOtherInsExists = FALSE;
	m_nPostEditBill_ApplyPaymentID = -1;
	//}}AFX_DATA_INIT

	m_hDiscount = NULL;
	m_hBlank = NULL;
	m_hNotes = NULL;
	m_hOnHold = NULL;
	// (j.jones 2015-02-20 11:22) - PLID 64935 - added void & correct icons
	m_hVoided = NULL;
	m_hCorrected = NULL;

	m_bSortASC = FALSE;

	m_bStatementNoteChanged = FALSE;
	m_bNoteChanged = FALSE;
	//PLID  16640 - take out minor discrepancy message
	//m_bCheckApplies = TRUE;

	//m_pParent = 0;

	// (j.dinatale 2010-10-27) - PLID 28773 - no longer need to preassign column indices, this is all done
	//		in the new SetUpColumns function
	// (j.jones 2010-06-16 16:14) - PLID 39150 - Every column after secondary insurance is dynamic,
	// the column ID may be different at any given time. Default these now.
	// (Somehow, assigning nColumnIndex++ does not work.)
	//short nColumnIndex = btcFirstDynamicColumn;
	//m_nOtherInsuranceColID = nColumnIndex; nColumnIndex++;
	//m_nBalanceColID = nColumnIndex; nColumnIndex++;
	//m_nProviderColID = nColumnIndex; nColumnIndex++;
	//m_nLocationColID = nColumnIndex; nColumnIndex++;
	//m_nPOSNameColID = nColumnIndex; nColumnIndex++;
	//m_nPOSCodeColID = nColumnIndex; nColumnIndex++;
	//m_nInputDateColID = nColumnIndex; nColumnIndex++;
	//m_nCreatedByColID = nColumnIndex; nColumnIndex++;
	//m_nFirstChargeDateColID = nColumnIndex; nColumnIndex++;
	//m_nDiagCode1ColID = nColumnIndex; nColumnIndex++;
	//m_nDiagCode1WithNameColID = nColumnIndex; nColumnIndex++;
	//m_nDiagCodeListColID = nColumnIndex; nColumnIndex++;
	//// (j.jones 2010-09-01 15:33) - PLID 40331 - added allowable column
	//m_nAllowableColID = nColumnIndex; nColumnIndex++;

	//invalid column, must always be the last entry
	//m_nMaxColID = nColumnIndex;

	// (j.jones 2015-03-17 14:02) - PLID 64929 - caches the current highlighted cell's back & fore color
	m_pCurSearchResultsHighlightedRow = NULL;
	m_nCurSearchResultsHighlightedColumn = -1;
	m_clrCurSearchResultCellBackgroundColor = RGB(255, 255, 255);
	m_clrCurSearchResultCellForegroundColor = RGB(0, 0, 0);
	m_clrCurSearchResultCellBackgroundSelColor = RGB(255, 255, 255);
	m_clrCurSearchResultCellForegroundSelColor = RGB(0, 0, 0);
}

CFinancialDlg::~CFinancialDlg()
{
	// (j.jones 2010-06-18 10:47) - PLID 39150 - must cast as FinTabItem*
	// in order to properly delete
	for(int w=g_aryFinancialTabInfoT.GetSize()-1;w>=0;w--) {
		delete (FinTabItem*)g_aryFinancialTabInfoT.GetAt(w);
	}
	g_aryFinancialTabInfoT.RemoveAll();
	for(w=g_aryFinancialTabInfoTemp.GetSize()-1;w>=0;w--) {
		delete (FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(w);
	}
	g_aryFinancialTabInfoTemp.RemoveAll();
	for(w=g_aryFinancialTabInfoTemp2.GetSize()-1;w>=0;w--) {
		delete (FinTabItem*)g_aryFinancialTabInfoTemp2.GetAt(w);
	}
	g_aryFinancialTabInfoTemp2.RemoveAll();

	m_dlgPackages.DestroyWindow();
	delete &m_dlgPackages;
	m_dlgQuotes.DestroyWindow();
	delete &m_dlgQuotes;
	m_dlgSearchBillingTab.DestroyWindow();
	delete &m_dlgSearchBillingTab;

	DestroyIcon((HICON)m_hDiscount);
	DestroyIcon((HICON)m_hBlank);
	DestroyIcon((HICON)m_hNotes);
	DestroyIcon((HICON)m_hOnHold);
	// (j.jones 2015-02-20 11:22) - PLID 64935 - added void & correct icons
	DestroyIcon((HICON)m_hVoided);
	DestroyIcon((HICON)m_hCorrected);
}

void CFinancialDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFinancialDlg)
	DDX_Control(pDX, IDC_CHECK_HIDE_VOIDED_ITEMS, m_checkHideVoidedItems);
	DDX_Control(pDX, IDC_CHECK_HIDE_ZERO_BALANCES, m_checkHideZeroBalances);
	DDX_Control(pDX, IDC_CHECK_BATCH, m_btnSupressSt);
	DDX_Control(pDX, IDC_SHOW_REFUNDS, m_btnShowRefunds);
	DDX_Control(pDX, IDC_SHOW_QUOTES, m_btnShowQuotes);
	DDX_Control(pDX, IDC_CLAIM_HISTORY, m_claimhistorybutton);
	DDX_Control(pDX, IDC_RESPONSIBLE_PARTY, m_btnResponsibleParty);
	DDX_Control(pDX, IDC_SHOW_PACKAGES, m_btnShowPackages);
	DDX_Control(pDX, IDC_SHOW_INS_BALANCE, m_showInsBalanceButton);
	DDX_Control(pDX, IDC_APPLY_MANAGER, m_applyManagerButton);
	DDX_Control(pDX, IDC_PRINT_HISTORY, m_printHistoryButton);
	DDX_Control(pDX, IDC_PREVIEW_STATEMENT, m_previewStatementButton);
	DDX_Control(pDX, IDC_NEW_PAYMENT, m_newPaymentButton);
	DDX_Control(pDX, IDC_NEW_BILL, m_newBillButton);
	DDX_Control(pDX, IDC_FINANCIAL_BKG, m_bkg);
	DDX_Control(pDX, IDC_EDIT_NOTES, m_nxeditEditNotes);
	DDX_Control(pDX, IDC_STATEMENT_NOTES, m_nxeditStatementNotes);
	DDX_Control(pDX, IDC_LAST_PAT_PAY_DATE, m_nxeditLastPatPayDate);
	DDX_Control(pDX, IDC_LAST_INS_PAY_DATE, m_nxeditLastInsPayDate);
	DDX_Control(pDX, IDC_LABEL2, m_nxstaticLabel2);
	DDX_Control(pDX, IDC_LABEL_BALANCE_DUE, m_nxstaticLabelBalanceDue);
	DDX_Control(pDX, IDC_LABEL3, m_nxstaticLabel3);
	DDX_Control(pDX, IDC_LABEL_PREPAYMENTS, m_nxstaticLabelPrepayments);
	DDX_Control(pDX, IDC_LABEL_LINE, m_nxstaticLabelLine);
	DDX_Control(pDX, IDC_LABEL5, m_nxstaticLabel5);
	DDX_Control(pDX, IDC_LABEL_TOTAL_BALANCE, m_nxstaticLabelTotalBalance);
	DDX_Control(pDX, IDC_LABEL_CHARGES, m_nxstaticLabelCharges);
	DDX_Control(pDX, IDC_LABEL_PAYMENTS, m_nxstaticLabelPayments);
	DDX_Control(pDX, IDC_LABEL_ADJUSTMENTS, m_nxstaticLabelAdjustments);
	DDX_Control(pDX, IDC_LABEL_REFUNDS, m_nxstaticLabelRefunds);
	DDX_Control(pDX, IDC_LABEL_NET_CHARGES, m_nxstaticLabelNetCharges);
	DDX_Control(pDX, IDC_LABEL_NET_PAYMENTS, m_nxstaticLabelNetPayments);
	DDX_Control(pDX, IDC_EMNS_TO_BE_BILLED, m_btnEMNsToBeBilled);
	DDX_Control(pDX, IDC_GLASSES_ORDER_HISTORY, m_btnGlassesOrderHistory);
	DDX_Control(pDX, IDC_SEARCH_BILLING, m_btnSearchBilling);
	//}}AFX_DATA_MAP
}

static BOOL boOneTimeInit = TRUE;

BOOL CFinancialDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		// (j.jones 2010-06-14 09:28) - PLID 39138 - added Charge Diag Code list
		g_propManager.CachePropertiesInBulk("FinancialDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'GlobalPeriodSort' OR "
			"Name = 'OneClickBillExpand' OR "
			"Name = 'ShowFinancialGridLines' OR "
			"Name = 'InsViewType' OR "
			// (j.jones 2010-09-27 15:56) - PLID 40123 - InsViewType is obsolete, replaced with BillingTab_ShowInsuranceColumns,
			// but is used to determine the default value for BillingTab_ShowInsuranceColumns
			"Name = 'BillingTab_ShowInsuranceColumns' OR "
			//DRT 12/9/2008 - PLID 32370 - Split ShowBillingRefunds from InsViewType
			"Name = 'ShowBillingRefunds' OR "
			"Name = 'ShowInsuranceNamesAsColumns' OR "
			"Name = 'GrayOutInsuranceColumns' OR "
			"Name = 'PopupPackageWindow' OR "
			"Name = 'PopupQuoteWindow' OR "
			"Name = 'CheckWarnGlobalPeriod' OR "
			"Name = 'PayAdjRefMenu' OR "
			"Name = 'BillingHideBalances' OR "
			"Name = 'BillingHideBalancesRange' OR "
			"Name = 'BillingHideBalancesRangeType' OR "
			// (j.jones 2007-04-19 11:10) - PLID 25721 - included BillingAllowSameDayEdits
			"Name = 'BillingAllowSameDayEdits' OR "
			// (j.jones 2007-08-10 09:23) - PLID 23769 - included the packages/quotes options
			"Name = 'IncludeAllPrePaysInPopUps' OR "
			"Name = 'DoNotBillNonPrepaidPackages' OR "
			// (j.jones 2008-02-11 15:30) - PLID 28847 - included DisallowBatchingPatientClaims
			"Name = 'DisallowBatchingPatientClaims' OR "
			// (j.jones 2008-02-12 11:35) - PLID 28848 - added HidePatientChargesOnClaims
			"Name = 'HidePatientChargesOnClaims' OR "
			// (j.jones 2010-06-14 08:52) - PLID 39138 - added ability to show diag codes inside descriptions
			"Name = 'ShowBillDiagCodesInBillingTabBillDesc' OR "
			"Name = 'ShowChargeDiagCodesInBillingTabBillDesc' OR "
			// (j.jones 2010-06-14 09:42) - PLID 39145 - added ability to word-wrap the description column
			"Name = 'WordWrapBillingTabDescription' OR "
			// (j.jones 2010-06-14 09:57) - PLID 39120 - added ability to show the last sent date in the bill description
			"Name = 'ShowBillLastSentDateInBillingTabBillDesc' OR " 
			// (a.walling 2010-06-16 12:37) - PLID 39121 - Bulk cache FinancialDlg_IncludePaymentCheckNumber
			"Name = 'FinancialDlg_IncludePaymentCheckNumber' OR " 
			// (j.jones 2010-09-23 08:55) - PLID 39307 - added abilities to show the adjustment reason & group codes in the adjustment description
			"Name = 'ShowAdjustmentGroupCodeInBillingTabAdjDesc' OR "
			"Name = 'ShowAdjustmentReasonCodeInBillingTabAdjDesc' OR "
			// (j.dinatale 2010-10-28) - PLID 28773 - add the ability to remember column width
			"Name = 'RememberBillingTabColumnWidths' OR "
			// (j.jones 2011-04-20 10:45) - PLID 41490 - added ability to ignore diagnosis codes not linked to charges
			"Name = 'SkipUnlinkedDiagsOnClaims'  OR "
			"Name = 'VoidChargeAdj_DefaultCat' OR "// (j.gruber 2011-08-08 12:17) - PLID 45596 - default category for voided charge adjustments
			"Name = 'dontshow BillingHideBalancesDontShowWarning' OR " //(e.lally 2011-08-26) PLID 45210 - cache it
			// (a.wilson 2012-06-14 11:24) - PLID 47966 add "ShowSalesReceiptDialog" to cache.
			"Name = 'ShowSalesReceiptDialog' "
			"OR Name = 'GlobalPeriod_OnlySurgicalCodes' "	// (j.jones 2012-07-23 17:24) - PLID 51651
			"OR Name = 'GlobalPeriod_IgnoreModifier78' OR "	// (j.jones 2012-07-26 09:53) - PLID 50489
			"Name = 'DisplayModifiersInBillingTabDescription' "
			"OR Name = 'Claim_SendPatAddressWhenPOS12'	" // (j.jones 2013-04-24 17:09) - PLID 56453
			// (j.jones 2013-07-19 15:38) - PLID 57653 - used in CheckUnbatchCrossoverClaim
			"OR Name = 'ERemit_UnbatchMA18orNA89_MarkForwardToSecondary' "
			"OR Name = 'BillPrimaryIns' " // (d.singleton 2014-02-28 11:30) - PLID 61072 
			"OR Name = 'EbillingFormatType' " // (b.spivey, September 24th, 2014) - PLID 63491 - cache it. 
			"OR Name = 'FormatStyle' "
			"OR Name = 'ERemitAddClaimNumberToDescription' " // (d.singleton 2014-10-09 17:57) - PLID 62698
			// (j.jones 2015-02-23 12:48) - PLID 64934 - added checkbox to hide voided items
			"OR Name = 'BillingTabHideVoidedItems' "
			// (j.jones 2015-03-20 13:33) - PLID 65402 - added refund void & correct preferences
			// (r.goldschmidt 2016-03-02 16:09) - PLID 68447 - make preference global
			"OR Name = 'RefundingAPayment_VoidAndCorrect_Global' "
			"OR Name = 'RefundingAPayment_VoidAndCorrect_Warn_Global' "
			")",
			_Q(GetCurrentUserName()));
		
		// (j.jones 2010-06-08 15:54) - PLID 39042 - these are now member variables
		m_hDiscount = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_DISCOUNT), IMAGE_ICON, 16,16, 0);
		m_hBlank = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BLANK), IMAGE_ICON, 16,16, 0);
		m_hNotes = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_BILL_NOTES), IMAGE_ICON, 16,16, 0);
		// (a.wilson 2014-07-24 11:27) - PLID 63015 - on hold icon.
		m_hOnHold = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_HOLD), IMAGE_ICON, 16, 16, 0);
		// (j.jones 2015-02-20 11:22) - PLID 64935 - added void & correct icons
		m_hVoided = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_CANCEL2_ICON), IMAGE_ICON, 16, 16, 0);
		m_hCorrected = (HICON)LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_CHECKMARK), IMAGE_ICON, 16, 16, 0);

		//DRT 4/14/2008 - PLID 29636 - NxIconify!
		m_newBillButton.AutoSet(NXB_NEW);
		m_newPaymentButton.AutoSet(NXB_NEW);
		m_btnResponsibleParty.AutoSet(NXB_MODIFY);
		m_applyManagerButton.AutoSet(NXB_MODIFY);
		m_showInsBalanceButton.SetTextColor(0x00404080);
		// (j.jones 2015-03-16 14:20) - PLID 64926 - added ability to search the billing tab
		m_btnSearchBilling.AutoSet(NXB_INSPECT);

		CFont *bold = &theApp.m_boldFont;

		GetDlgItem(IDC_LABEL2)->SetFont(bold);
		GetDlgItem(IDC_LABEL3)->SetFont(bold);
		GetDlgItem(IDC_LABEL5)->SetFont(bold);
		GetDlgItem(IDC_LABEL_LINE)->SetFont(bold);
		GetDlgItem(IDC_LABEL_BALANCE_DUE)->SetFont(bold);
		GetDlgItem(IDC_LABEL_PREPAYMENTS)->SetFont(bold);
		GetDlgItem(IDC_LABEL_TOTAL_BALANCE)->SetFont(bold);

		m_iCurPatient = GetActivePatientID();
		
		m_BillingDlg = m_pParent->GetBillingDlg();
		m_BillingDlg->m_pFinancialDlg = this;

		// (j.jones 2010-06-07 14:43) - PLID 39042 - converted to a datalist 2
		m_List = BindNxDataList2Ctrl(IDC_BILLING_TAB_LIST,false);

		//these lists are still datalist 1
		m_StatementNoteCombo = BindNxDataListCtrl(IDC_STATEMENT_NOTE_COMBO);
		m_BalanceFilterCombo = BindNxDataListCtrl(IDC_BALANCE_FILTER_COMBO,false);
		m_GlobalPeriodList = BindNxDataListCtrl(IDC_GLOBAL_PERIOD_LIST,false);

		// (j.gruber 2011-09-09 09:43) - PLID 45408 - don't show original or voids
		m_GlobalPeriodList->FromClause = _bstr_t("(SELECT CPTCodeT.Code, ServiceT.Name, LineItemT.Date, CPTCodeT.GlobalPeriod, "
			"DATEADD(day,GlobalPeriod,LineItemT.Date) AS ExpDate, (CASE WHEN (DATEADD(day,GlobalPeriod,LineItemT.Date) > GetDate()) THEN 0 ELSE 1 END) AS Expired, "
			"LineItemT.PatientID, ServicePayGroupsT.Category AS PayGroupCategory, "
			"Coalesce(ChargesT.CPTModifier, '') AS Mod1, Coalesce(ChargesT.CPTModifier2, '') AS Mod2, Coalesce(ChargesT.CPTModifier3, '') AS Mod3, Coalesce(ChargesT.CPTModifier4, '') AS Mod4 "
			"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
			"LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON LineItemT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON LineItemT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND GlobalPeriod Is Not Null AND CPTCodeT.GlobalPeriod <> 0 "
			"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "					
			") AS Q");

		// (j.jones 2012-07-24 09:14) - PLID 51651 - added a preference to only track global periods for
		// surgical codes only, if it is disabled when we would look at all codes
		long nSurgicalCodesOnly = GetRemotePropertyInt("GlobalPeriod_OnlySurgicalCodes", 1, 0, "<None>", true);

		// (j.jones 2012-07-26 09:53) - PLID 50489 - added another preference to NOT track global periods
		// if the charge uses modifier 78
		long nIgnoreModifier78 = GetRemotePropertyInt("GlobalPeriod_IgnoreModifier78", 1, 0, "<None>", true);

		CString str;
		str.Format("PatientID = %li "
			"AND (%li <> 1 OR PayGroupCategory = %li) "
			"AND (%li <> 1 OR (Mod1 <> '78' AND Mod2 <> '78' AND Mod3 <> '78' AND Mod4 <> '78')) ",
			m_iCurPatient,
			nSurgicalCodesOnly, PayGroupCategory::SurgicalCode,
			nIgnoreModifier78);
		m_GlobalPeriodList->WhereClause = _bstr_t(str);

		int GlobalPeriodSort = GetRemotePropertyInt("GlobalPeriodSort",0,0,"<None>",TRUE);
		if(GlobalPeriodSort == 0)
			m_GlobalPeriodList->GetColumn(1)->PutSortAscending(TRUE);
		else
			m_GlobalPeriodList->GetColumn(1)->PutSortAscending(FALSE);

		// (j.jones 2006-04-20 09:07) - this will requery in CalculateTotals()
		//m_GlobalPeriodList->Requery();

		BuildBalanceFilterCombo();

		// (j.dinatale 2010-10-29) - PLID 28773 - keep track of if we are remembering widths
		m_bRememberColWidth = (GetRemotePropertyInt("RememberBillingTabColumnWidths", 0, 0, GetCurrentUserName(), true) == 1);
		((CButton *)GetDlgItem(IDC_BILLINGTAB_REMCOLWIDTH))->SetCheck(m_bRememberColWidth);

		// (j.dinatale 2010-10-27) - PLID 28773 - All Column setup is done in the SetUpColumns function now
		//		Removed: The hardcoded inserts of columns, the color coding of each column, various other datalist configuration
		//		Also, column order is dependent on data. So any additional columns to be added must be appended to the end of the enum
		//		in the .h file. Also, columns to be added must have an entry in the switch in SetUpColumnByType function and a mod must
		//		be written to add the column to data per user.
		// Set up the columns
		// if you wish to change the order of these columns, you must also change the order in the enum in the .h file
		SetUpColumns(true);

		//PLID  16640 - take out minor discrepancy message
		//m_bCheckApplies = TRUE;

		//TES 11/28/2011 - PLID 43700 - Hide the button if they don't have permission
		if(!g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrSilent)) {
			GetDlgItem(IDC_GLASSES_ORDER_HISTORY)->ShowWindow(SW_HIDE);
		}

		SetTimer(IDT_SHOW_WARNING, 25, NULL);

	} NxCatchAll("Error in CFinancialDlg::OnInitDialog");


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFinancialDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	// (z.manning, 5/19/2006, PLID 20726) - We may still have focus on a field that has been changed,
	// so let's make sure we don't lose any changes.
	StoreDetails();

	if(m_LocationChecker.Changed()) {
		m_BalanceFilterCombo->Clear();
		BuildBalanceFilterCombo();
	}

	// (j.dinatale 2010-10-29) - PLID 28773 - we no longer have columns in this table that affect this tab
	// (j.jones 2008-09-16 17:27) - PLID 19623 - reload shown/hidden column information if it changed
	//if(m_ConfigBillColumnsChecker.Changed()) {
	//ReconfigureVisibleColumns();
	//}

	// (j.dinatale 2010-10-28) - PLID 28773 - Since columns are all dynamic, it is just easier to rebuild the entire list of columns if a RespType 
	//		change occurs or a billing column change occurs
	// (j.jones 2010-06-17 14:42) - PLID 39150 - update our insurance columns if RespTypeT changed
	if(m_RespTypeChecker.Changed()) {
		if(m_bRememberColWidth){
			SaveColumnWidths();
		}
		SetUpColumns(true);
	}

	// (j.jones 2010-06-14 09:43) - PLID 39145 - added ability to word-wrap the description column
	{
		IColumnSettingsPtr pCol = m_List->GetColumn(m_nDescriptionColID);
		if(pCol) {
			// (d.thompson 2012-08-01) - PLID 51898 - Changed default to word-wrap (1)
			if(GetRemotePropertyInt("WordWrapBillingTabDescription", 1, 0, GetCurrentUserName(), true) == 1) {
				pCol->PutFieldType(cftTextWordWrap);
			}
			else {
				pCol->PutFieldType(cftTextSingleLine);
			}
		}
	}

	//if (!IsWindowVisible()) return;

	//JMJ - though we should always have m_boAllowUpdate in sync, this will forcibly correct it
	if(m_iCurPatient != GetActivePatientID())
		m_boAllowUpdate = TRUE;

	if (!m_boAllowUpdate)
		return;

	m_boAllowUpdate = FALSE;

	//set this boolean to their preference to expand bills completely with one click
	// (d.thompson 2012-06-27) - PLID 51220 - Changed default to Yes
	m_bOneClickExpansion = GetRemotePropertyInt("OneClickBillExpand",1,0,GetCurrentUserName(),TRUE) == 1 ? TRUE : FALSE;

	// (j.jones 2009-12-28 17:04) - PLID 27237 - parameterized
	_RecordsetPtr tmpRS = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT CurrentStatus, FinancialNotes, StatementNote, SuppressStatement FROM PatientsT WHERE PatientsT.PersonID = {INT}", GetActivePatientID());
	if(!tmpRS->eof) {
		//update financial notes
		m_FinancialNotes = AdoFldString(tmpRS, "FinancialNotes","");
		GetDlgItem(IDC_EDIT_NOTES)->SetWindowText(m_FinancialNotes);
		m_bNoteChanged = FALSE;

		//update suppress statement button
		if (AdoFldBool(tmpRS, "SuppressStatement",FALSE))
			m_SuppressStatement = true;
		else m_SuppressStatement = false;
		((CButton *)GetDlgItem(IDC_CHECK_BATCH))->SetCheck(m_SuppressStatement);

		//update statement note
		CString str = AdoFldString(tmpRS, "StatementNote");
		SetDlgItemText(IDC_STATEMENT_NOTES, str);
		m_bStatementNoteChanged = FALSE;

		long nStatus = AdoFldShort(tmpRS, "CurrentStatus",1);
		//TES 12/18/2009 - PLID 35055 - PatientView already handles the colors just fine, there's no need for this code, which
		// was screwing up the special status-based colors in Internal.
		/*if(m_bkg.GetColor() != GetNxColor(GNC_PATIENT_STATUS, nStatus))
		SetColor(GetNxColor(GNC_PATIENT_STATUS, nStatus));*/
	}
	tmpRS->Close();

	// (j.jones 2012-07-24 09:14) - PLID 51651 - added a preference to only track global periods for
	// surgical codes only, if it is disabled when we would look at all codes
	long nSurgicalCodesOnly = GetRemotePropertyInt("GlobalPeriod_OnlySurgicalCodes", 1, 0, "<None>", true);

	// (j.jones 2012-07-26 09:53) - PLID 50489 - added another preference to NOT track global periods
	// if the charge uses modifier 78
	long nIgnoreModifier78 = GetRemotePropertyInt("GlobalPeriod_IgnoreModifier78", 1, 0, "<None>", true);

	CString str;
	str.Format("PatientID = %li "
		"AND (%li <> 1 OR PayGroupCategory = %li) "
		"AND (%li <> 1 OR (Mod1 <> '78' AND Mod2 <> '78' AND Mod3 <> '78' AND Mod4 <> '78')) ",
		GetActivePatientID(),
		nSurgicalCodesOnly, PayGroupCategory::SurgicalCode,
		nIgnoreModifier78);
	m_GlobalPeriodList->WhereClause = _bstr_t(str);

	int GlobalPeriodSort = GetRemotePropertyInt("GlobalPeriodSort",0,0,"<None>",TRUE);
	if(GlobalPeriodSort == 0)
		m_GlobalPeriodList->GetColumn(1)->PutSortAscending(TRUE);
	else
		m_GlobalPeriodList->GetColumn(1)->PutSortAscending(FALSE);

	if (m_iCurPatient == GetActivePatientID())
		QuickRefresh();


	/////////////////////////////////////////////////////////////////
	// Only set the colors the first time around
	if (boOneTimeInit) {
		boOneTimeInit = FALSE;
	}

	/////////////////////////////////////////////////////////////////////
	// If the patient changed, collapse all the bills and refresh the
	// listbox
	if (m_iCurPatient != GetActivePatientID()) 
	{
		// (j.jones 2015-03-17 13:25) - PLID 64931 - always clear the search if the patient changed
		ClearSearchResults();

		if(m_List->GetRowCount() > 0) {
			m_List->PutTopRow(m_List->GetFirstRow());
			m_List->PutCurSel(m_List->GetFirstRow());
		}
		m_SavedRow_LineID = 0;
		m_SavedTopRow_LineID = 0;

		int oldpatient = m_iCurPatient;
		m_iCurPatient = GetActivePatientID();

		ClearExpandLists();
		FillTabInfo();

		/////////////////////////////////////////////////////

		// CAH 1/31
		// Take the focus away from the patients combo if necessary.
		// This will prevent anyone pressing the down arrow from
		// dropping it down.
		// RAC 2/16
		// TODO: This takes the focus even if it was set by some 
		// method other than the user clicking on the drop-down.
		// This happens, for example, when the user changes patients 
		// in the scheduler.  It seems like maybe we should set this 
		// focus in the event handler of the one datalist we care 
		// about, the Patient Toolbar datalist.  For now, I just 
		// made it check where the focus currently is.
		///SetFocus();
		if (GetFocus() == &(GetMainFrame()->m_patToolBar.m_wndPatientsCombo)) {
			SetFocus();
		}

		//PLID  16640 - take out minor discrepancy message
		//m_bCheckApplies = TRUE;


	}

	// (j.jones 2005-12-05 16:12) - PLID 18521 - only pop up the package/quote window if the active view is the Patient view
	CMainFrame  *pMainFrame;
	pMainFrame = GetMainFrame();
	if (pMainFrame != NULL) {
		CNxTabView *pView = pMainFrame->GetActiveView();
		if (pView && pView->IsKindOf(RUNTIME_CLASS(CPatientView))) {
			TryShowPackageWindow();
		}
	}

	m_boAllowUpdate = TRUE;

	// (j.jones 2015-03-17 13:25) - PLID 64931 - RevalidateSearchResults will restore our current search
	// if nothing changed, or clear the search if content did change
	RevalidateSearchResults();
}

// (j.jones 2010-06-07 14:49) - PLID 39042 - deprecated
/*
int CFinancialDlg::GetRow(COleVariant varBoundValue)
{
int count = m_List->GetRowCount();
for (int iRecord = 0; iRecord < count; iRecord++) {
IRowSettingsPtr pRow = m_List->
if (m_List->GetValue(iRecord, btcLineID).lVal == varBoundValue.lVal)
return iRecord;
}
return -1;
}
*/

void CFinancialDlg::SetColor(OLE_COLOR nNewColor)
{
	m_bkg.SetColor(nNewColor);
	CPatientDialog::SetColor(nNewColor);
}

// This function labels all the bills, charges and payments as collapsed
void CFinancialDlg::ClearExpandLists()
{
	m_aiExpandedBranches.RemoveAll();
	m_aiExpandedSubBranches.RemoveAll();
	m_aiExpandedPayBranches.RemoveAll();

	// (j.jones 2015-03-17 13:25) - PLID 64931 - clear the search results window
	// only if the expanded branches aren't cached or if we're in the middle of a redraw
	if (m_RedrawRefCount == 0 && aiTempExpandedBranches.GetSize() == 0
		&& aiTempExpandedPayBranches.GetSize() == 0
		&& aiTempExpandedSubBranches.GetSize() == 0) {

		ClearSearchResults();
	}
}


// only called from QuickRefresh, saves and restores expansion arrays
void CFinancialDlg::SaveExpansionInfo()
{
	aiTempExpandedBranches.Copy(m_aiExpandedBranches);
	aiTempExpandedPayBranches.Copy(m_aiExpandedPayBranches);
	aiTempExpandedSubBranches.Copy(m_aiExpandedSubBranches);
}
void CFinancialDlg::RestoreExpansionInfo()
{
	m_aiExpandedBranches.Copy(aiTempExpandedBranches);
	m_aiExpandedPayBranches.Copy(aiTempExpandedPayBranches);
	m_aiExpandedSubBranches.Copy(aiTempExpandedSubBranches);
	aiTempExpandedBranches.RemoveAll();
	aiTempExpandedPayBranches.RemoveAll();
	aiTempExpandedSubBranches.RemoveAll();
}


// This function returns true if an item is in the expanded list.
// The expanded list holds line IDs for bills.
BOOL CFinancialDlg::InExpandedList(long itemID)
{
	for (int i=0; i < m_aiExpandedBranches.GetSize(); i++) {
		if (m_aiExpandedBranches[i] == itemID)
			return TRUE;
	}
	return FALSE;
}

// This function returns true if an item is in the expanded sub list.
// The expanded sub list holds line IDs for charges.
BOOL CFinancialDlg::InExpandedSubList(long itemID)
{
	for (int i=0; i < m_aiExpandedSubBranches.GetSize(); i++) {
		int j = m_aiExpandedSubBranches[i];
		if (m_aiExpandedSubBranches[i] == itemID)
			return TRUE;
	}
	return FALSE;
}

// This function returns true if an item is in the pay expanded list.
// The expanded list holds line IDs for payments.
BOOL CFinancialDlg::InExpandedPayList(long itemID)
{
	for (int i=0; i < m_aiExpandedPayBranches.GetSize(); i++) {
		if (m_aiExpandedPayBranches[i] == itemID)
			return TRUE;
	}
	return FALSE;
}

BEGIN_MESSAGE_MAP(CFinancialDlg, CNxDialog)
	//{{AFX_MSG_MAP(CFinancialDlg)
	ON_WM_SHOWWINDOW()
	ON_EN_KILLFOCUS(IDC_EDIT_NOTES, OnKillfocusEditNotes)
	ON_EN_KILLFOCUS(IDC_STATEMENT_NOTES, OnKillfocusStatementNotes)
	ON_EN_CHANGE(IDC_EDIT_NOTES, OnChangeEditNotes)
	ON_EN_CHANGE(IDC_STATEMENT_NOTES, OnChangeStatementNotes)
	ON_BN_CLICKED(IDC_SHOW_REFUNDS, OnShowRefunds)
	ON_BN_CLICKED(IDC_CHECK_BATCH, OnCheckBatch)
	ON_BN_CLICKED(IDC_EDIT_STATEMENT_NOTES, OnEditStatementNotes)
	ON_BN_CLICKED(IDC_NEW_BILL, OnNewBill)
	ON_BN_CLICKED(IDC_NEW_PAYMENT, OnNewPayment)
	ON_BN_CLICKED(IDC_PRINT_HISTORY, OnPrintHistory)
	ON_BN_CLICKED(IDC_PREVIEW_STATEMENT, OnPreviewStatement)
	ON_BN_CLICKED(IDC_SHOW_INS_BALANCE, OnShowInsBalance)
	ON_BN_CLICKED(IDC_APPLY_MANAGER, OnApplyManager)
	ON_BN_CLICKED(IDC_SHOW_PACKAGES, OnShowPackages)
	ON_BN_CLICKED(IDC_RESPONSIBLE_PARTY, OnResponsibleParty)
	ON_BN_CLICKED(IDC_CLAIM_HISTORY, OnClaimHistory)
	ON_BN_CLICKED(IDC_SHOW_QUOTES, OnShowQuotes)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_CHECK_HIDE_ZERO_BALANCES, OnCheckHideZeroBalances)
	ON_MESSAGE(WM_BARCODE_SCAN, OnBarcodeScan)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_PTS_TO_BE_BILLED, OnBnClickedPtsToBeBilled)
	ON_BN_CLICKED(IDC_BILLINGTAB_REMCOLWIDTH, OnRememberColWidth)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_GLASSES_ORDER_HISTORY, OnGlassesOrderHistory)
	ON_BN_CLICKED(IDC_CHECK_HIDE_VOIDED_ITEMS, OnCheckHideVoidedItems)
	ON_BN_CLICKED(IDC_SEARCH_BILLING, OnBtnSearchBilling)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFinancialDlg message handlers

void CFinancialDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CNxDialog::OnShowWindow(bShow, nStatus);

	if (bShow) {
		GetDlgItem(IDC_BILLING_TAB_LIST)->SetFocus();
		m_iCurPatient = GetActivePatientID();

		GetDlgItem(IDC_NEW_BILL)->SetFocus();
	}
	else {
		HideToolWindows();		
	}
}

BEGIN_EVENTSINK_MAP(CFinancialDlg, CNxDialog)
	//{{AFX_EVENTSINK_MAP(CFinancialDlg)
	ON_EVENT(CFinancialDlg, IDC_STATEMENT_NOTE_COMBO, 16 /* SelChosen */, OnSelChosenStatementNoteCombo, VTS_I4)
	ON_EVENT(CFinancialDlg, IDC_BALANCE_FILTER_COMBO, 16 /* SelChosen */, OnSelChosenBalanceFilterCombo, VTS_I4)
	ON_EVENT(CFinancialDlg, IDC_GLOBAL_PERIOD_LIST, 18 /* RequeryFinished */, OnRequeryFinishedGlobalPeriodList, VTS_I2)	
	ON_EVENT(CFinancialDlg, IDC_BILLING_TAB_LIST, 6, OnRButtonDownBillingTabList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CFinancialDlg, IDC_BILLING_TAB_LIST, 17, OnColumnClickingBillingTabList, VTS_I2 VTS_PBOOL)
	ON_EVENT(CFinancialDlg, IDC_BILLING_TAB_LIST, 12, OnDragBeginBillingTabList, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CFinancialDlg, IDC_BILLING_TAB_LIST, 13, OnDragOverCellBillingTabList, VTS_PBOOL VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CFinancialDlg, IDC_BILLING_TAB_LIST, 14, OnDragEndBillingTabList, VTS_DISPATCH VTS_I2 VTS_DISPATCH VTS_I2 VTS_I4)
	ON_EVENT(CFinancialDlg, IDC_BILLING_TAB_LIST, 19, OnLeftClickBillingTabList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CFinancialDlg, IDC_BILLING_TAB_LIST, 22, OnColumnResize, VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// (j.dinatale 2010-10-27) - PLID 28773 - Needed to take into account remember column widths throughout this entire function, also, we are ensuring that columns are
//		visible when they should be by setting their width when appropriate
// (j.jones 2008-09-17 16:00) - PLID 19623 - renamed this function into something more accurate
void CFinancialDlg::DisplayInsuranceAndRefundColumns()
{
	try {

		CWaitCursor pWait;

		DisableFinancialScreen();

		IColumnSettingsPtr pCol;

		// (d.thompson 2008-12-09 10:26) - PLID 32370 - Split the refunds flag to its own property
		if(m_nShowRefunds) {
			pCol = m_List->GetColumn(m_nRefundColID);
			// (j.dinatale 2010-11-04) - PLID 28773 - Set up the column according to any stored width if necessary, otherwise take on the width of the data
			//		all width changes now follow this logic since remember column widths has been implemented
			if(m_bRememberColWidth){
				long nWidth = 0;
				// try and look up the stored width
				if(m_mapColIndexToStoredWidth.Lookup(m_nRefundColID, nWidth)){
					// if its for some reason not visible, make it visible so the user sees it
					if(nWidth <= 0){
						nWidth = 75;
					}
					pCol->PutStoredWidth(nWidth);
				}else{
					pCol->PutStoredWidth(75);
				}
			}else{
				//refunds do include the header text in the calculation
				pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_TRUE);
			}
			pCol->ColumnStyle = m_bRememberColWidth ? csVisible : csVisible|csWidthData;
		}
		else {
			pCol = m_List->GetColumn(m_nRefundColID);
			pCol->PutStoredWidth(0);
			pCol->ColumnStyle = csVisible|csFixedWidth;
		}

		// (d.thompson 2008-12-09 10:27) - PLID 32370 - Removed refunds from each case, removed all
		//	the extraneous refunds cases.
		// (j.jones 2010-09-27 15:56) - PLID 40123 - Changed the preference here, the new values for this
		// preference are: 0 - hide all, 1 - show all, 2 - hide unused
		switch (m_iInsuranceViewToggle) {
		case 1:	//Show All

			pCol = m_List->GetColumn(m_nPatientBalanceColID);
			if(m_bRememberColWidth){
				long nWidth = 0;
				if(m_mapColIndexToStoredWidth.Lookup(m_nPatientBalanceColID, nWidth)){
					if(nWidth <= 0){
						nWidth = 75;
					}
					pCol->PutStoredWidth(nWidth);
				}else{
					pCol->PutStoredWidth(75);
				}
			}else{
				//Patient Resp. does include the header text in the calculation
				pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_TRUE);
			}
			pCol->ColumnStyle = m_bRememberColWidth ? csVisible : csVisible|csWidthData;

			pCol = m_List->GetColumn(m_nPrimInsColID);
			if(m_bRememberColWidth){
				long nWidth = 0;
				if(m_mapColIndexToStoredWidth.Lookup(m_nPrimInsColID, nWidth)){
					if(nWidth <= 0){
						nWidth = 75;
					}
					pCol->PutStoredWidth(nWidth);
				}else{
					pCol->PutStoredWidth(75);
				}
			}else{
				pCol->CalcWidthFromData(VARIANT_FALSE, VARIANT_TRUE);
			}
			pCol->ColumnStyle = m_bRememberColWidth ? csVisible : csVisible|csWidthData;

			pCol = m_List->GetColumn(m_nSecInsColID);
			if(m_bRememberColWidth){
				long nWidth = 0;
				if(m_mapColIndexToStoredWidth.Lookup(m_nSecInsColID, nWidth)){
					if(nWidth <= 0){
						nWidth = 75;
					}
					pCol->PutStoredWidth(nWidth);
				}else{
					pCol->PutStoredWidth(75);
				}
			}else{
				pCol->CalcWidthFromData(VARIANT_FALSE, VARIANT_TRUE);
			}
			pCol->ColumnStyle = m_bRememberColWidth ? csVisible : csVisible|csWidthData;
				

			pCol = m_List->GetColumn(m_nOtherInsuranceColID);
			if(m_bRememberColWidth){
				long nWidth = 0;
				if(m_mapColIndexToStoredWidth.Lookup(m_nOtherInsuranceColID, nWidth)){
					if(nWidth <= 0){
						nWidth = 75;
					}
					pCol->PutStoredWidth(nWidth);
				}else{
					pCol->PutStoredWidth(75);
				}
			}else{
				pCol->CalcWidthFromData(VARIANT_FALSE, VARIANT_TRUE);
			}
			pCol->ColumnStyle = m_bRememberColWidth ? csVisible : csVisible|csWidthData;

			// (j.dinatale 2010-10-27) - PLID 28773 - the first "dynamic" column right after the secondary insurance column
			// (j.jones 2010-06-18 12:18) - PLID 39150 - set the dynamic columns, if any
			for(short i=m_nSecInsColID + 1; i<m_nOtherInsuranceColID; i++) {
				pCol = m_List->GetColumn(i);
				if(m_bRememberColWidth){
					long nWidth = 0;
					if(m_mapColIndexToStoredWidth.Lookup(i, nWidth)){
						if(nWidth <= 0){
							nWidth = 75;
						}
						pCol->PutStoredWidth(nWidth);
					}else{
						pCol->PutStoredWidth(75);
					}
				}else{
					pCol->CalcWidthFromData(VARIANT_FALSE, VARIANT_TRUE);
				}
				pCol->ColumnStyle = m_bRememberColWidth ? csVisible : csVisible|csWidthData;	
			}
			break;

		case 2:	//Hide Unused
			{
				//remember if we show any insurance, we have to show
				//the patient column, but if we hide all insurance,
				//we don't have to show the patient column
				BOOL bShowedAtLeastOne = FALSE;

				//primary insurance column
				pCol = m_List->GetColumn(m_nPrimInsColID);
				if(m_GuarantorID1 != -1) {
					bShowedAtLeastOne = TRUE;
					if(m_bRememberColWidth){
						long nWidth = 0;
						if(m_mapColIndexToStoredWidth.Lookup(m_nPrimInsColID, nWidth)){
							if(nWidth <= 0){
								nWidth = 75;
							}
							pCol->PutStoredWidth(nWidth);
						}else{
							pCol->PutStoredWidth(75);
						}
					}else{
						pCol->CalcWidthFromData(VARIANT_FALSE, VARIANT_TRUE);
					}
					pCol->ColumnStyle = m_bRememberColWidth ? csVisible : csVisible|csWidthData;
				}
				else {
					pCol->PutStoredWidth(0);
					pCol->ColumnStyle = csVisible|csFixedWidth;
				}

				//secondary insurance column
				pCol = m_List->GetColumn(m_nSecInsColID);
				if(m_GuarantorID2 != -1) {
					bShowedAtLeastOne = TRUE;
					if(m_bRememberColWidth){
						long nWidth = 0;
						if(m_mapColIndexToStoredWidth.Lookup(m_nSecInsColID, nWidth)){
							if(nWidth <= 0){
								nWidth = 75;
							}
							pCol->PutStoredWidth(nWidth);
						}else{
							pCol->PutStoredWidth(75);
						}
					}else{
						pCol->CalcWidthFromData(VARIANT_FALSE, VARIANT_TRUE);
					}
					pCol->ColumnStyle = m_bRememberColWidth ? csVisible : csVisible|csWidthData;
				}
				else {
					pCol->PutStoredWidth(0);
					pCol->ColumnStyle = csVisible|csFixedWidth;
				}

				//check the dynamic insurance columns, if any
				for(int d=0;d<m_aryDynamicRespTypeIDs.GetSize();d++) {
					long nRespTypeID = (long)m_aryDynamicRespTypeIDs.GetAt(d);

					short iColumnIndex = -1;
					if(m_mapRespTypeIDToColumnIndex.Lookup(nRespTypeID, iColumnIndex) && iColumnIndex != -1) {

						pCol = m_List->GetColumn(iColumnIndex);

						//it is a dynamic insurance column, does this patient have a guarantor?
						long nInsuredPartyID = -1;
						if(m_mapRespTypeIDsToGuarantorIDs.Lookup(nRespTypeID, nInsuredPartyID) && nInsuredPartyID != -1) {
							//they do have a guarantor
							bShowedAtLeastOne = TRUE;
							if(m_bRememberColWidth){
								long nWidth = 0;
								if(m_mapColIndexToStoredWidth.Lookup(iColumnIndex, nWidth)){
									if(nWidth <= 0){
										nWidth = 75;
									}
									pCol->PutStoredWidth(nWidth);
								}else{
									pCol->PutStoredWidth(75);
								}
							}else{
								pCol->CalcWidthFromData(VARIANT_FALSE, VARIANT_TRUE);
							}
							pCol->ColumnStyle = m_bRememberColWidth ? csVisible : csVisible|csWidthData;
						}
						else {
							//no guarantor, hide it
							pCol->PutStoredWidth(0);
							pCol->ColumnStyle = csVisible|csFixedWidth;
						}
					}
					else {
						//shouldn't be possible, but don't assert, just ignore it here
					}
				}

				//other insurance column
				pCol = m_List->GetColumn(m_nOtherInsuranceColID);
				if(m_bOtherInsExists) {
					bShowedAtLeastOne = TRUE;
					if(m_bRememberColWidth){
						long nWidth = 0;
						if(m_mapColIndexToStoredWidth.Lookup(m_nOtherInsuranceColID, nWidth)){
							if(nWidth <= 0){
								nWidth = 75;
							}
							pCol->PutStoredWidth(nWidth);
						}else{
							pCol->PutStoredWidth(75);
						}
					}else{
						pCol->CalcWidthFromData(VARIANT_FALSE, VARIANT_TRUE);
					}
					pCol->ColumnStyle = m_bRememberColWidth ? csVisible : csVisible|csWidthData;
				}
				else {
					pCol->PutStoredWidth(0);
					pCol->ColumnStyle = csVisible|csFixedWidth;
				}

				//now for the patient balance, hide the column if we're not
				//showing any insurance columns
				pCol = m_List->GetColumn(m_nPatientBalanceColID);
				if(bShowedAtLeastOne) {
					if(m_bRememberColWidth){
						long nWidth = 0;
						if(m_mapColIndexToStoredWidth.Lookup(m_nPatientBalanceColID, nWidth)){
							if(nWidth <= 0){
								nWidth = 75;
							}
							pCol->PutStoredWidth(nWidth);
						}else{
							pCol->PutStoredWidth(75);
						}
					}else{
						//Patient Resp. does include the header text in the calculation
						pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_TRUE);
					}
					pCol->ColumnStyle = m_bRememberColWidth ? csVisible : csVisible|csWidthData;
				}
				else {
					pCol->PutStoredWidth(0);
					pCol->ColumnStyle = csVisible|csFixedWidth;
				}
			}
			break;

		case 0:	//Hide All
		default:
			pCol = m_List->GetColumn(m_nPatientBalanceColID);
			pCol->PutStoredWidth(0);
			pCol->ColumnStyle = csVisible|csFixedWidth;

			pCol = m_List->GetColumn(m_nPrimInsColID);
			pCol->PutStoredWidth(0);
			pCol->ColumnStyle = csVisible|csFixedWidth;

			pCol = m_List->GetColumn(m_nSecInsColID);
			pCol->PutStoredWidth(0);
			pCol->ColumnStyle = csVisible|csFixedWidth;

			pCol = m_List->GetColumn(m_nOtherInsuranceColID);
			pCol->PutStoredWidth(0);
			pCol->ColumnStyle = csVisible|csFixedWidth;

			// (j.dinatale 2010-10-27) - PLID 28773 - the first "dynamic" column right after the secondary insurance column
			// (j.jones 2010-06-18 12:18) - PLID 39150 - set the dynamic columns, if any
			for(short i=m_nSecInsColID + 1; i<m_nOtherInsuranceColID; i++) {
				pCol = m_List->GetColumn(i);
				pCol->PutStoredWidth(0);
				pCol->ColumnStyle = csVisible|csFixedWidth;
			}
			break;
		}		

		// (j.jones 2008-09-18 13:00) - PLID 19623 - Strangely, the datalist will attempt to scroll
		// to the right by the width of the column we most recently made visible. Bob says that while
		// it seems bug-like, it was at one point intentional. Bob's own suggestion was to make a dummy
		// column at index 0, of zero-width, and hide & show it, which will cause the datalist to force
		// the newly displayed column to be visible. Luckily we need no dummy column, we can use
		// btcLineID. In the unlikely and unnecessary event that some user expanded column widths to
		// display btcLineID, we're going to hide it. Too bad.
		pCol = m_List->GetColumn(btcLineID);
		pCol->PutStoredWidth(0);
		pCol->ColumnStyle = csFixedWidth;
		pCol->ColumnStyle = csVisible|csFixedWidth;

		// (j.jones 2008-09-17 15:58) - PLID 31405 - resize the description column to reflect this change
		// (j.jones 2008-09-18 11:50) - not necessary because EnableFinancialScreen will handle this
		//ResizeDescriptionColumn();

	}NxCatchAll("Error in CFinancialDlg::DisplayInsuranceAndRefundColumns");

	EnableFinancialScreen();
}

/********************************************************
*	CalculateTotals() will fill the subtotals on the
*	bottom of the dialog
*
* Callers:	OnLeftClickBillingTabList
*			OnClickBtnBill
*			OnClickBtnPayment
*			OnPopupSelectionFinancialList
*			OnEndDragDropFinancialList
*			FillTabInfo
*			Pay_Adj_Ref_Click
*			
********************************************************/
void CFinancialDlg::CalculateTotals()
{
	COleCurrency cyNetPayments, cyTotalPayments, cyNetBills, cyTotalBills,
		cyTotalAdjustments, cyTotalRefunds, cyBalance, cyPrePayBalance,
		cyTotalPatBalance;

	EnsureRemoteData();
	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//LogDetail("Calculating totals");

	try {
		cyNetPayments = COleCurrency(0,0);
		cyTotalPayments = COleCurrency(0,0);
		cyNetBills = COleCurrency(0,0);
		cyTotalBills = COleCurrency(0,0);
		cyTotalAdjustments = COleCurrency(0,0);
		cyTotalRefunds = COleCurrency(0,0);
		cyBalance = COleCurrency(0,0);
		cyPrePayBalance = COleCurrency(0,0);
		cyTotalPatBalance = COleCurrency(0,0);

		//////////////////////////////////////////////////////////
		// Get the total for the bills
		//////////////////////////////////////////////////////////
		CString str;
		COleVariant var;

		_RecordsetPtr rs;

		m_GlobalPeriodList->SetRedraw(FALSE);
		m_GlobalPeriodList->Requery();

		// (j.jones 2009-12-30 14:56) - PLID 36729 - this code has been converted
		// to get both recordsets from a function that creates and opens it with one data access
		rs = CreatePatientTotalsRecordset(m_iCurPatient);
		if(!rs->eof) {
			COleCurrency cyTotal = AdoFldCurrency(rs, "GrandTotal",COleCurrency(0,0));
			cyNetBills = cyTotal;
			cyTotalBills = cyTotal;
		}

		rs = rs->NextRecordset(NULL);

		while(!rs->eof) {
			long Type = AdoFldLong(rs, "Type",1);
			COleCurrency cySumOfAmount = AdoFldCurrency(rs, "SumOfAmount",COleCurrency(0,0));

			//add the payments, even prepayments
			if(Type == 1) {
				cyTotalPayments += cySumOfAmount;
				cyNetPayments += cySumOfAmount;
			}
			// Add the adjustments
			else if (Type == 2) {
				cyTotalAdjustments += cySumOfAmount;
				cyNetBills -= cySumOfAmount;
			}
			// Add the refunds
			else if (Type == 3) {
				cyTotalRefunds += cySumOfAmount;
				cyNetPayments += cySumOfAmount;
			}

			rs->MoveNext();
		}
		rs->Close();

		//only report UNAPPLIED PrePayments			
		cyPrePayBalance = CalculatePrePayments(m_iCurPatient, CSqlFragment("{CONST_STR}", GetBalanceFilter("PaymentsT")));

		cyBalance = cyNetBills - cyNetPayments + cyPrePayBalance;
		cyTotalPatBalance = cyNetBills - cyNetPayments;

		CRect rc;

		// Fill static text members
		str = FormatCurrencyForInterface(cyBalance);
		(GetDlgItem(IDC_LABEL_BALANCE_DUE))->SetWindowText(str);

		GetDlgItem(IDC_LABEL_BALANCE_DUE)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		str = FormatCurrencyForInterface(cyPrePayBalance);
		(GetDlgItem(IDC_LABEL_PREPAYMENTS))->SetWindowText(str);

		GetDlgItem(IDC_LABEL_PREPAYMENTS)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		str = FormatCurrencyForInterface(cyTotalPatBalance);
		(GetDlgItem(IDC_LABEL_TOTAL_BALANCE))->SetWindowText(str);

		GetDlgItem(IDC_LABEL_TOTAL_BALANCE)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		str = FormatCurrencyForInterface(cyTotalBills);
		(GetDlgItem(IDC_LABEL_CHARGES))->SetWindowText(str);

		GetDlgItem(IDC_LABEL_CHARGES)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		str = FormatCurrencyForInterface(cyTotalAdjustments);
		(GetDlgItem(IDC_LABEL_ADJUSTMENTS))->SetWindowText(str);

		GetDlgItem(IDC_LABEL_ADJUSTMENTS)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		str = FormatCurrencyForInterface(cyNetBills);
		(GetDlgItem(IDC_LABEL_NET_CHARGES))->SetWindowText(str);

		GetDlgItem(IDC_LABEL_NET_CHARGES)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		str = FormatCurrencyForInterface(cyTotalPayments);
		(GetDlgItem(IDC_LABEL_PAYMENTS))->SetWindowText(str);

		GetDlgItem(IDC_LABEL_PAYMENTS)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		str = FormatCurrencyForInterface(cyTotalRefunds);
		(GetDlgItem(IDC_LABEL_REFUNDS))->SetWindowText(str);

		GetDlgItem(IDC_LABEL_REFUNDS)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		str = FormatCurrencyForInterface(cyNetPayments);
		(GetDlgItem(IDC_LABEL_NET_PAYMENTS))->SetWindowText(str);

		GetDlgItem(IDC_LABEL_NET_PAYMENTS)->GetWindowRect(rc);
		ScreenToClient(rc);
		InvalidateRect(rc);

		//calculate last payment date		
		SetDlgItemText(IDC_LAST_PAT_PAY_DATE,"");
		SetDlgItemText(IDC_LAST_INS_PAY_DATE,"");
		// (j.jones 2007-02-20 09:55) - PLID 24666 - ensured the last payment date is truly a PAYMENT, not an adj. or refund
		//DRT 10/21/2008 - PLID 31774 - Parameterized
		// (j.jones 2009-08-24 13:04) - PLID 14229 - now we break this down into patient and insurance
		// (a.wilson 2013-03-29 10:45) - PLID 55329 - ensure voided payments are not included. still include corrected.
		rs = CreateParamRecordset(GetRemoteDataSnapshot(),
		// (b.spivey, February 25, 2013) - PLID 38117 - Query for latest patient payment date and amount. 
			// (b.spivey, March 08, 2013) - PLID 38117 - changed from WITH statements because occassionally NxQuery appends exec. 
			" SELECT TOP 1	LineItemT.ID, LineItemT.Amount AS PatAmt, LineItemT.[Date] AS PatDate \r\n"
			" FROM			LineItemT \r\n"
			" INNER JOIN	PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n"
			" LEFT JOIN		LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID \r\n"
			" LEFT JOIN		LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID \r\n"
			" WHERE			LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type = 1 AND PaymentsT.InsuredPartyID = -1 \r\n"
			"				AND OriginalLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL \r\n"
			" ORDER BY		LineItemT.[Date] DESC, LineItemT.ID DESC \r\n"
			" \r\n"
			// (b.spivey, February 25, 2013) - PLID 38117 - Query for latest insurance payment date and amount.
			" SELECT TOP 1	LineItemT.ID, LineItemT.Amount AS InsAmt, LineItemT.[Date] AS InsDate \r\n"
			" FROM			LineItemT \r\n"
			" INNER JOIN	PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n"
			" LEFT JOIN		LineItemCorrectionsT OriginalLineItemsT ON LineItemT.ID = OriginalLineItemsT.OriginalLineItemID \r\n"
			" LEFT JOIN		LineItemCorrectionsT VoidingLineItemsT ON LineItemT.ID = VoidingLineItemsT.VoidingLineItemID \r\n"
			" WHERE			LineItemT.PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type = 1 AND PaymentsT.InsuredPartyID > 0 \r\n"
			"				AND OriginalLineItemsT.OriginalLineItemID IS NULL AND VoidingLineItemsT.VoidingLineItemID IS NULL \r\n"
			" ORDER BY		LineItemT.[Date] DESC, LineItemT.ID DESC \r\n",
			m_iCurPatient, m_iCurPatient);
		if(!rs->eof) {
			COleDateTime dtLastPatPayDate;
			_variant_t var = rs->Fields->Item["PatDate"]->Value;
			CString strLastPtPay = "";
			if(var.vt == VT_DATE) {
				dtLastPatPayDate = VarDateTime(var);
				strLastPtPay = FormatDateTimeForInterface(dtLastPatPayDate, NULL, dtoDate);	
			}

			// (b.spivey, February 25, 2013) - PLID 38117 - patient Payment amount. 
			COleCurrency cyPatientAmt = AdoFldCurrency(rs->Fields, "PatAmt", g_ccyNull);
			if (cyPatientAmt != g_ccyNull) {
				strLastPtPay += " for " + FormatCurrencyForInterface(cyPatientAmt);
			}

			//If we have a string value, set the interface display.
			if (strLastPtPay.GetLength() > 0) {
				SetDlgItemText(IDC_LAST_PAT_PAY_DATE,strLastPtPay);
			}
		}

		rs = rs->NextRecordset(NULL);

		if(!rs->eof) {
			COleDateTime dtLastInsPayDate;
			_variant_t var = rs->Fields->Item["InsDate"]->Value;
			CString strLastInsPay = "";
			if(var.vt == VT_DATE) {
				dtLastInsPayDate = VarDateTime(var);
				strLastInsPay = FormatDateTimeForInterface(dtLastInsPayDate, NULL, dtoDate);	
			}

			// (b.spivey, February 25, 2013) - PLID 38117 - Ins payment amount.  
			COleCurrency cyInsAmt = AdoFldCurrency(rs->Fields, "InsAmt", g_ccyNull);
			if (cyInsAmt != g_ccyNull) {
				strLastInsPay += " for " + FormatCurrencyForInterface(cyInsAmt);
			}

			//If we have a string value, set the interface display.
			if (strLastInsPay.GetLength() > 0) {
				SetDlgItemText(IDC_LAST_INS_PAY_DATE,strLastInsPay);
			}
		}
		rs->Close();

		//if the package or quote window is open, recalculate its totals
		RefreshPackagesQuotes();

		// (j.jones 2015-03-17 13:25) - PLID 64931 - RevalidateSearchResults will restore our current search
		// if nothing changed, or clear the search if content did change
		RevalidateSearchResults();
	}
	NxCatchAll("FinancialDlg::CalculateTotals");
}

/********************************************************
*	RedrawBill() will delete a bill and its charges from
*	FinancialTabInfoT (The table used by the listbox) and
*	redraw it with all its charges if expanded.
*
* Callers:	OnLeftClickBillingTabList
*			OnColumnClickFinancialList
*			OnPopupSelectionFinancialList
*			OnEndDragDropFinancialList
*			ExpandBill
*			Pay_Adj_Ref_Click
********************************************************/
void CFinancialDlg::RedrawBill(long iBillID)
{
	CString str;
	int iLineID; //iRecordID;

	try {

		for(int index=0;index<g_aryFinancialTabInfoT.GetSize();index++) {
			if(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(index))->BillID.vt == VT_I4
				&& ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(index))->BillID.lVal == iBillID)
				break;
		}
		if(index==g_aryFinancialTabInfoT.GetSize())
			return;
		iLineID = ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(index))->LineID.lVal;

		// Step 1: Delete the bill and all of it's children
		for(int q=g_aryFinancialTabInfoT.GetSize()-1;q>=0;q--) {
			if(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(q))->BillID.vt == VT_I4
				&& ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(q))->BillID.lVal == iBillID) {
					delete (FinTabItem*)g_aryFinancialTabInfoT.GetAt(q);
					g_aryFinancialTabInfoT.RemoveAt(q);
			}
		}

		// Step 2: Add all the lines below this bill into the temporary table
		for(int x=g_aryFinancialTabInfoTemp.GetSize()-1;x>=0;x--) {
			delete (FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(x);
		}
		g_aryFinancialTabInfoTemp.RemoveAll();

		FinTabItem *pNew;
		for(int w=0;w<g_aryFinancialTabInfoT.GetSize();w++){
			if(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(w))->LineID.vt == VT_I4
				&& ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(w))->LineID.lVal > iLineID) {
					pNew = new FinTabItem;
					FinTabItemCopy(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(w)),pNew);
					pNew->LineID = (long)GetNewNum("Temp");
					g_aryFinancialTabInfoTemp.Add(pNew);
			}
		}

		// Step 3: Delete all the lines below and including this bill from FinancialTabInfoT
		for(int z=g_aryFinancialTabInfoT.GetSize()-1;z>=0;z--) {
			if(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(z))->LineID.vt == VT_I4
				&& ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(z))->LineID.lVal >= iLineID) {
					delete (FinTabItem*)g_aryFinancialTabInfoT.GetAt(z);
					g_aryFinancialTabInfoT.RemoveAt(z);
			}
		}
		
		// Step 4: Add the bill in its new form

		// (j.jones 2009-12-30 09:33) - PLID 36729 - this code has been converted
		// to build a batch of recordsets for bills and charges - will be just
		// one recordset if it is a collapsed bill

		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		if(InExpandedList(iBillID)) {
			AddExpandedBillsToSqlBatch(strSqlBatch, aryParams, iBillID, m_iCurPatient);

			//load the charges now, since we know we are displaying them next
			AddChargesToSqlBatch(strSqlBatch, aryParams, iBillID, g_cvarNull, m_iCurPatient, FALSE);
		}
		else {
			AddCollapsedBillsToSqlBatch(strSqlBatch, aryParams, iBillID, m_iCurPatient);
		}

		//this may only have one recordset, it may have two
		// (j.jones 2015-03-18 09:18) - PLID 64927 - use the snapshot connection
		_RecordsetPtr rs = CreateParamRecordsetBatch(GetRemoteDataSnapshot(), strSqlBatch, aryParams);

		// (j.jones 2010-06-14 09:07) - PLID 39138 - get the preference info. for the diag codes
		BOOL bIncludeBillDiagsInDesc = GetRemotePropertyInt("ShowBillDiagCodesInBillingTabBillDesc", 0, 0, "<None>", true) == 1;
		// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
		BOOL bIncludeChargeDiagsInDesc = GetRemotePropertyInt("ShowChargeDiagCodesInBillingTabBillDesc", 1, 0, "<None>", true) == 1;

		// (j.jones 2010-06-14 09:57) - PLID 39120 - added ability to show the last sent date in the bill description
		// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
		BOOL bIncludeLastSentDate = GetRemotePropertyInt("ShowBillLastSentDateInBillingTabBillDesc", 1, 0, "<None>", true) == 1;

		int i = 0;
		while(!rs->eof) {
			pNew = new FinTabItem;
			pNew->LineID = (long)GetNewNum("T");
			pNew->LineType = rs->Fields->Item["LineType"]->Value;
			pNew->BillID = (long)rs->Fields->Item["ID"]->Value;
			pNew->ExpandChar = rs->Fields->Item["ExpandChar"]->Value;
			pNew->Date = rs->Fields->Item["Date"]->Value;
			pNew->Location = rs->Fields->Item["LocName"]->Value;
			pNew->NoteIcon = rs->Fields->Item["NoteIcon"]->Value;			
			pNew->HasNotes = (long)rs->Fields->Item["HasNotes"]->Value;
			pNew->OnHold = (long)rs->Fields->Item["OnHold"]->Value;	// (a.wilson 2014-07-24 12:52) - PLID 63015
			// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
			pNew->IsVoided = VarLong(rs->Fields->Item["IsVoided"]->Value, 0);
			pNew->IsCorrected = VarLong(rs->Fields->Item["IsCorrected"]->Value, 0);
			pNew->Description = rs->Fields->Item["Description"]->Value;
			pNew->ChargeAmount = rs->Fields->Item["ChargeAmount"]->Value;
			// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field
			pNew->TotalPayAmount = rs->Fields->Item["TotalPayAmount"]->Value;
			pNew->PayAmount = rs->Fields->Item["PayAmount"]->Value;
			pNew->Adjustment = rs->Fields->Item["Adjustment"]->Value;
			pNew->BalanceAmount = rs->Fields->Item["Balance"]->Value;
			pNew->PatResp = rs->Fields->Item["PatResp"]->Value;
			pNew->InsResp1 = rs->Fields->Item["Ins1Resp"]->Value;
			pNew->InsResp2 = rs->Fields->Item["Ins2Resp"]->Value;

			// (j.jones 2010-06-18 10:04) - PLID 39150 - fill the dynamic columns
			for(int d=0;d<m_aryDynamicRespTypeIDs.GetSize();d++) {
				long nRespTypeID = (long)m_aryDynamicRespTypeIDs.GetAt(d);

				//these columns should exist in our query
				CString strField;
				strField.Format("Ins%liResp", nRespTypeID);

				DynInsResp *dir = new DynInsResp;
				dir->nRespTypeID = nRespTypeID;
				dir->varResp = rs->Fields->Item[(LPCTSTR)strField]->Value;
				pNew->arypDynInsResps.Add(dir);
			}

			pNew->InsRespOther = rs->Fields->Item["InsRespOther"]->Value;
			pNew->Refund = rs->Fields->Item["Refund"]->Value;
			pNew->Discount = rs->Fields->Item["DiscountTotal"]->Value; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get the discount total
			pNew->HasPercentOff = (long)rs->Fields->Item["HasPercentOff"]->Value; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get has percent off discount

			// (j.jones 2008-09-17 10:00) - PLID 19623 - supported new columns
			pNew->Provider = rs->Fields->Item["Provider"]->Value;
			pNew->POSName = rs->Fields->Item["POSName"]->Value;
			pNew->POSCode = rs->Fields->Item["POSCode"]->Value;
			pNew->InputDate = rs->Fields->Item["InputDate"]->Value;
			pNew->CreatedBy = rs->Fields->Item["CreatedBy"]->Value;
			pNew->FirstChargeDate = rs->Fields->Item["FirstChargeDate"]->Value;

			// (j.jones 2010-05-26 16:50) - PLID 28184 - added diagnosis code fields
			pNew->DiagCode1 = rs->Fields->Item["DiagCode1"]->Value;
			pNew->DiagCode1WithName = rs->Fields->Item["DiagCode1WithName"]->Value;
			pNew->DiagCodeList = rs->Fields->Item["BillDiagCodeList"]->Value;

			// (j.jones 2010-09-01 15:39) - PLID 40331 - added allowable
			// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
			// and added ChargeAllowable
			pNew->FeeSchedAllowable = g_cvarNull;
			pNew->ChargeAllowable = g_cvarNull;
			// (j.jones 2011-12-20 11:30) - PLID 47119 - added deductible and coinsurance
			pNew->ChargeDeductible = g_cvarNull;
			pNew->ChargeCoinsurance = g_cvarNull;
			// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number, always filled if
			// a value exists, even if the invoice number feature is turned off
			pNew->InvoiceNumber = rs->Fields->Item["InvoiceID"]->Value;

			// (j.jones 2010-06-14 09:06) - PLID 39138 - if the preference asks to put the diag codes in the bill desc, do so now 
			if(bIncludeBillDiagsInDesc) {
				CString strDescription = VarString(pNew->Description, "");
				CString strDiags = AdoFldString(rs, "BillDiagCodeList", "");
				if(!strDescription.IsEmpty() && !strDiags.IsEmpty()) {
					strDescription += ", ";
				}
				strDescription += strDiags;

				pNew->Description = (LPCTSTR)strDescription;
			}

			// (j.jones 2010-06-14 09:57) - PLID 39120 - added ability to show the last sent date in the bill description
			if(bIncludeLastSentDate) {
				CString strDescription = VarString(pNew->Description, "");
				COleDateTime dtInvalid;
				dtInvalid.SetStatus(COleDateTime::invalid);
				COleDateTime dt = AdoFldDateTime(rs, "LastSentDate", dtInvalid);

				if(dt.GetStatus() != COleDateTime::invalid) {

					if(!strDescription.IsEmpty()) {
						strDescription += ", ";
					}
					strDescription += "Last Sent: ";
					strDescription += FormatDateTimeForInterface(dt, NULL, dtoDate);

					pNew->Description = (LPCTSTR)strDescription;
				}
			}

			g_aryFinancialTabInfoT.Add(pNew);
			rs->MoveNext();
			i++;
		}

		if (i == 0) {
			MsgBox("This bill has been removed from the screen. Practice will now requery this patient's financial information.");
			FullRefresh();
			return;
		}

		// Step 5: Add the charges
		if (InExpandedList(iBillID)) {

			// (j.jones 2009-12-30 09:33) - PLID 36729 - if in the expanded list,
			// the next recordset will have the charges
			rs = rs->NextRecordset(NULL);			
			FinTabItem *pNew;
			while(!rs->eof) {
				pNew = new FinTabItem;
				pNew->LineID = (long)GetNewNum("T");
				pNew->LineType = rs->Fields->Item["Expr1"]->Value;
				pNew->BillID = (long)rs->Fields->Item["BillID"]->Value;
				pNew->ChargeID = rs->Fields->Item["ID"]->Value;
				pNew->Date = rs->Fields->Item["Date"]->Value;
				pNew->Location = rs->Fields->Item["LocName"]->Value;
				pNew->NoteIcon = rs->Fields->Item["NoteIcon"]->Value;
				pNew->HasNotes = (long)rs->Fields->Item["HasNotes"]->Value;
				pNew->OnHold = (long)0;	// (a.wilson 2014-07-24 13:00) - PLID 63015 - charges aren't on hold.
				// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
				pNew->IsVoided = VarLong(rs->Fields->Item["IsVoided"]->Value, 0);
				pNew->IsCorrected = VarLong(rs->Fields->Item["IsCorrected"]->Value, 0);
				pNew->Description = rs->Fields->Item["Expr12"]->Value;
				pNew->ChargeAmount = rs->Fields->Item["Amount"]->Value;
				pNew->BalanceAmount = rs->Fields->Item["Expr3"]->Value;
				pNew->ExpandChar = rs->Fields->Item["Expr2"]->Value;
				// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field
				pNew->TotalPayAmount = rs->Fields->Item["TotalPayAmount"]->Value;
				pNew->PayAmount = rs->Fields->Item["Expr4"]->Value;
				pNew->Adjustment = rs->Fields->Item["Expr5"]->Value;
				pNew->ExpandSubChar = rs->Fields->Item["Expr6"]->Value;
				pNew->PatResp = rs->Fields->Item["Expr7"]->Value;
				pNew->InsResp1 = rs->Fields->Item["Expr8"]->Value;
				pNew->InsResp2 = rs->Fields->Item["Expr9"]->Value;

				// (j.jones 2010-06-18 10:04) - PLID 39150 - fill the dynamic columns
				for(int d=0;d<m_aryDynamicRespTypeIDs.GetSize();d++) {
					long nRespTypeID = (long)m_aryDynamicRespTypeIDs.GetAt(d);

					//these columns should exist in our query
					CString strField;
					strField.Format("Ins%liResp", nRespTypeID);

					DynInsResp *dir = new DynInsResp;
					dir->nRespTypeID = nRespTypeID;
					dir->varResp = rs->Fields->Item[(LPCTSTR)strField]->Value;
					pNew->arypDynInsResps.Add(dir);
				}

				pNew->InsRespOther = rs->Fields->Item["Expr10"]->Value;
				pNew->Refund = rs->Fields->Item["Expr11"]->Value;
				// (j.gruber 2009-03-17 16:41) - PLID 33360 - changed to new structure
				pNew->Discount = rs->Fields->Item["TotalDiscount"]->Value; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get the discount
				pNew->HasPercentOff = (long)rs->Fields->Item["HasPercentOff"]->Value; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get has percent off discount
				// (j.jones 2008-09-17 10:00) - PLID 19623 - supported new columns
				pNew->Provider = rs->Fields->Item["Provider"]->Value;
				pNew->POSName = rs->Fields->Item["POSName"]->Value;
				pNew->POSCode = rs->Fields->Item["POSCode"]->Value;
				pNew->InputDate = rs->Fields->Item["InputDate"]->Value;
				pNew->CreatedBy = rs->Fields->Item["CreatedBy"]->Value;
				pNew->FirstChargeDate = rs->Fields->Item["FirstChargeDate"]->Value;

				// (j.jones 2010-05-26 16:50) - PLID 28184 - added diagnosis code fields
				pNew->DiagCode1 = rs->Fields->Item["DiagCode1"]->Value;
				pNew->DiagCode1WithName = rs->Fields->Item["DiagCode1WithName"]->Value;
				pNew->DiagCodeList = rs->Fields->Item["ChargeDiagCodeList"]->Value;

				// (j.jones 2010-06-14 09:06) - PLID 39138 - if the preference asks to put the diag codes in the charge desc, do so now 
				if(bIncludeChargeDiagsInDesc) {
					CString strDescription = VarString(pNew->Description, "");
					CString strDiags = AdoFldString(rs, "ChargeDiagCodeList", "");
					if(!strDescription.IsEmpty() && !strDiags.IsEmpty()) {
						strDescription += ", ";
					}
					strDescription += strDiags;

					pNew->Description = (LPCTSTR)strDescription;
				}

				// (j.jones 2010-09-01 15:39) - PLID 40331 - added allowable
				// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
				// and added ChargeAllowable
				pNew->FeeSchedAllowable = rs->Fields->Item["FeeSchedAllowable"]->Value;
				pNew->ChargeAllowable = rs->Fields->Item["ChargeAllowable"]->Value;
				
				// (j.jones 2011-12-20 11:30) - PLID 47119 - added deductible and coinsurance
				pNew->ChargeDeductible = rs->Fields->Item["ChargeDeductible"]->Value;
				pNew->ChargeCoinsurance = rs->Fields->Item["ChargeCoinsurance"]->Value;

				// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number, always filled if
				// a value exists, even if the invoice number feature is turned off
				pNew->InvoiceNumber = rs->Fields->Item["InvoiceID"]->Value;

				g_aryFinancialTabInfoT.Add(pNew);
				rs->MoveNext();
			}

			// Draw expanded charges
			if (m_aiExpandedSubBranches.GetSize() > 0) {
				CDWordArray aiCharges, aiLineIDs;
				/*TODO: Hope this isn't used
				iRecordID = m_rsTab->AbsolutePosition;
				m_rsTab->Requery(NULL);
				m_rsTab->Move(iRecordID+1);
				*/

				// Get all the charges and lines
				//TES 11/7/2007 - PLID 27979 - VS2008 - for() loops
				int i = 0;
				for(i=0;i<g_aryFinancialTabInfoT.GetSize();i++) {
					if(VarString(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(i))->LineType) == "Charge"
						&& VarLong(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(i))->BillID,-1) == iBillID) {
							aiCharges.Add(VarLong(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(i))->ChargeID));
							aiLineIDs.Add(VarLong(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(i))->LineID));
					}
				}

				// Redraw the expanded charges
				// Redrawing charges will modify the LineIDs! We must do this backwards.
				// a.walling 04/14/2006
				for (i = aiCharges.GetSize() - 1; i >= 0; i--) {
					RedrawCharge(iBillID, aiCharges.GetAt(i), aiLineIDs.GetAt(i));
				}

			}
		}

		rs->Close();

		// Step 6: Insert a blank line if expanded
		if (InExpandedList(iBillID)) {
			pNew = new FinTabItem;
			pNew->LineID = (long)GetNewNum("T");
			pNew->LineType = _variant_t(_T("Blank"));
			pNew->BillID = _variant_t(iBillID);
			pNew->HasNotes = (long)(0);
			pNew->OnHold = (long)0;	// (a.wilson 2014-07-24 13:00) - PLID 63015
			// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
			pNew->IsVoided = (long)0;
			pNew->IsCorrected = (long)0;
			pNew->Discount = _variant_t(COleCurrency(0,0)); // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get the discount total
			pNew->HasPercentOff = (long)0; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get has percent off discount
			g_aryFinancialTabInfoT.Add(pNew);
		}

		// Step 7: Add all the lines from FinancialTabInfoTemp back to FinancialTabInfoT
		for(int t=0;t<g_aryFinancialTabInfoTemp.GetSize();t++) {
			pNew = new FinTabItem;
			FinTabItemCopy(((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(t)),pNew);
			pNew->LineID = (long)GetNewNum("T");
			g_aryFinancialTabInfoT.Add(pNew);
		}

		// Step 8: If there are expanded charges, then go through each charge and make
		// sure it's redrawn correctly	
		if (m_aiExpandedSubBranches.GetSize() > 0) {
			CDWordArray aiChargeIDs, aiLineIDs;

			// Refresh the list so we can begin to expand charges
			RefreshList();

			_variant_t var;

			// Find all the charges for this bill
			// (j.jones 2010-06-07 14:50) - PLID 39042 - this is now a datalist 2
			IRowSettingsPtr pRow = m_List->GetFirstRow();
			while(pRow) {
				var = pRow->GetValue(m_nBillIDColID);
				if(var.vt == VT_I4 && VarLong(var) == iBillID) {
					var = pRow->GetValue(btcChargeID);
					if (var.vt != VT_NULL && var.vt != VT_EMPTY) {
						aiChargeIDs.Add(VarLong(var));
						var = pRow->GetValue(btcLineID);
						aiLineIDs.Add(VarLong(var));
					}
				}

				pRow = pRow->GetNextRow();
			}

			// Redraw all the expanded charges

			//JJ - this will expand the charges, meaning lineIDs change!
			//So go through this in reverse
			for (i=aiChargeIDs.GetSize()-1; i>=0; i--) {
				if (InExpandedSubList(aiChargeIDs.GetAt(i))) {
					RedrawCharge(iBillID, aiChargeIDs.GetAt(i), aiLineIDs.GetAt(i));
				}
			}		
		}

	} NxCatchAll("Error in RedrawBill");

}

/*************************************************************
* This function will redraw a single charge that is already
* existent in FinancialTabInfoT (the table used by the listbox).
*
* Callers:	RedrawBill
*			ExpandCharge
*************************************************************/
void CFinancialDlg::RedrawCharge(long iBillID, long iChargeID, long iLineID)
{
	CString str;
	FinTabItem *pNew;

	try{
		// Step 1: Delete the charge and all of it's children
		for(int q=g_aryFinancialTabInfoT.GetSize()-1;q>=0;q--) {
			if(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(q))->ChargeID.vt == VT_I4 &&
				((FinTabItem*)g_aryFinancialTabInfoT.GetAt(q))->ChargeID.lVal == iChargeID) {
					delete (FinTabItem*)g_aryFinancialTabInfoT.GetAt(q);
					g_aryFinancialTabInfoT.RemoveAt(q);
			}
		}

		// Step 2: Add all the lines below this bill into the temporary table
		for(int x=g_aryFinancialTabInfoTemp2.GetSize()-1;x>=0;x--) {
			delete (FinTabItem*)g_aryFinancialTabInfoTemp2.GetAt(x);
		}
		g_aryFinancialTabInfoTemp2.RemoveAll();

		for(int w=0;w<g_aryFinancialTabInfoT.GetSize();w++){
			if(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(w))->LineID.vt == VT_I4 &&
				((FinTabItem*)g_aryFinancialTabInfoT.GetAt(w))->LineID.lVal > iLineID) {
					pNew = new FinTabItem;
					FinTabItemCopy(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(w)),pNew);
					pNew->LineID = (long)GetNewNum("Temp2");
					g_aryFinancialTabInfoTemp2.Add(pNew);
			}
		}

		// Step 3: Delete all the lines below and including this bill from FinancialTabInfoT
		for(int z=g_aryFinancialTabInfoT.GetSize()-1;z>=0;z--) {
			if(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(z))->LineID.vt == VT_I4 &&
				((FinTabItem*)g_aryFinancialTabInfoT.GetAt(z))->LineID.lVal >= iLineID) {
					delete (FinTabItem*)g_aryFinancialTabInfoT.GetAt(z);
					g_aryFinancialTabInfoT.RemoveAt(z);
			}
		}

		// Step 4: Add the charge in its new form
		BOOL bExpanded = InExpandedSubList(iChargeID);

		int i = 0;

		// (j.jones 2009-12-30 09:33) - PLID 36729 - this code has been converted
		// to build a batch of recordsets for charges and payments - will be just
		// one recordset if it is a collapsed charge

		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		AddChargesToSqlBatch(strSqlBatch, aryParams, iBillID, iChargeID, m_iCurPatient, bExpanded);

		if(InExpandedSubList(iChargeID)) {
			//load the applied pays, since we know we are displaying them later
			AddAppliedPaysToSqlBatch(strSqlBatch, aryParams, iBillID, iChargeID, m_iCurPatient);
		}

		// (j.jones 2010-06-14 09:07) - PLID 39138 - get the preference info. for the diag codes
		// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
		BOOL bIncludeChargeDiagsInDesc = GetRemotePropertyInt("ShowChargeDiagCodesInBillingTabBillDesc", 1, 0, "<None>", true) == 1;

		//this may only have one recordset, it may have two
		// (j.jones 2015-03-18 09:18) - PLID 64927 - use the snapshot connection
		_RecordsetPtr rs = CreateParamRecordsetBatch(GetRemoteDataSnapshot(), strSqlBatch, aryParams);
		while(!rs->eof) {
			pNew = new FinTabItem;
			pNew->LineID = (long)GetNewNum("T");
			pNew->LineType = rs->Fields->Item["Expr1"]->Value;
			pNew->BillID = rs->Fields->Item["BillID"]->Value;
			pNew->ChargeID = rs->Fields->Item["ID"]->Value;
			pNew->Date = rs->Fields->Item["Date"]->Value;
			pNew->Location = rs->Fields->Item["LocName"]->Value;
			pNew->NoteIcon = rs->Fields->Item["NoteIcon"]->Value;
			pNew->HasNotes = (long)rs->Fields->Item["HasNotes"]->Value;
			pNew->OnHold = (long)0;	// (a.wilson 2014-07-24 13:03) - PLID 63015
			// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
			pNew->IsVoided = VarLong(rs->Fields->Item["IsVoided"]->Value, 0);
			pNew->IsCorrected = VarLong(rs->Fields->Item["IsCorrected"]->Value, 0);
			pNew->Description = rs->Fields->Item["Expr12"]->Value;
			pNew->ChargeAmount = rs->Fields->Item["Amount"]->Value;
			pNew->BalanceAmount = rs->Fields->Item["Expr3"]->Value;
			pNew->ExpandChar = rs->Fields->Item["Expr2"]->Value;
			// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field
			pNew->TotalPayAmount = rs->Fields->Item["TotalPayAmount"]->Value;
			pNew->PayAmount = rs->Fields->Item["Expr4"]->Value;
			pNew->Adjustment = rs->Fields->Item["Expr5"]->Value;
			pNew->ExpandSubChar = rs->Fields->Item["Expr6"]->Value;
			pNew->PatResp = rs->Fields->Item["Expr7"]->Value;
			pNew->InsResp1 = rs->Fields->Item["Expr8"]->Value;
			pNew->InsResp2 = rs->Fields->Item["Expr9"]->Value;

			// (j.jones 2010-06-18 10:04) - PLID 39150 - fill the dynamic columns
			for(int d=0;d<m_aryDynamicRespTypeIDs.GetSize();d++) {
				long nRespTypeID = (long)m_aryDynamicRespTypeIDs.GetAt(d);

				//these columns should exist in our query
				CString strField;
				strField.Format("Ins%liResp", nRespTypeID);

				DynInsResp *dir = new DynInsResp;
				dir->nRespTypeID = nRespTypeID;
				dir->varResp = rs->Fields->Item[(LPCTSTR)strField]->Value;
				pNew->arypDynInsResps.Add(dir);
			}

			pNew->InsRespOther = rs->Fields->Item["Expr10"]->Value;
			pNew->Refund = rs->Fields->Item["Expr11"]->Value;
			// (j.gruber 2009-03-17 17:02) - PLID 33360 - changed discount structure
			pNew->Discount = rs->Fields->Item["TotalDiscount"]->Value; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get the discount
			pNew->HasPercentOff = (long)rs->Fields->Item["HasPercentOff"]->Value; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get has percent off discount
			// (j.jones 2008-09-17 10:00) - PLID 19623 - supported new columns
			pNew->Provider = rs->Fields->Item["Provider"]->Value;
			pNew->POSName = rs->Fields->Item["POSName"]->Value;
			pNew->POSCode = rs->Fields->Item["POSCode"]->Value;
			pNew->InputDate = rs->Fields->Item["InputDate"]->Value;
			pNew->CreatedBy = rs->Fields->Item["CreatedBy"]->Value;
			pNew->FirstChargeDate = rs->Fields->Item["FirstChargeDate"]->Value;

			// (j.jones 2010-05-26 16:50) - PLID 28184 - added diagnosis code fields
			pNew->DiagCode1 = rs->Fields->Item["DiagCode1"]->Value;
			pNew->DiagCode1WithName = rs->Fields->Item["DiagCode1WithName"]->Value;
			pNew->DiagCodeList = rs->Fields->Item["ChargeDiagCodeList"]->Value;

			// (j.jones 2010-09-01 15:39) - PLID 40331 - added allowable
			// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
			// and added ChargeAllowable
			pNew->FeeSchedAllowable = rs->Fields->Item["FeeSchedAllowable"]->Value;
			pNew->ChargeAllowable = rs->Fields->Item["ChargeAllowable"]->Value;

			// (j.jones 2011-12-20 11:30) - PLID 47119 - added deductible and coinsurance
			pNew->ChargeDeductible = rs->Fields->Item["ChargeDeductible"]->Value;
			pNew->ChargeCoinsurance = rs->Fields->Item["ChargeCoinsurance"]->Value;

			// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number, always filled if
			// a value exists, even if the invoice number feature is turned off
			pNew->InvoiceNumber = rs->Fields->Item["InvoiceID"]->Value;

			// (j.jones 2010-06-14 09:06) - PLID 39138 - if the preference asks to put the diag codes in the charge desc, do so now 
			if(bIncludeChargeDiagsInDesc) {
				CString strDescription = VarString(pNew->Description, "");
				CString strDiags = AdoFldString(rs, "ChargeDiagCodeList", "");
				if(!strDescription.IsEmpty() && !strDiags.IsEmpty()) {
					strDescription += ", ";
				}
				strDescription += strDiags;

				pNew->Description = (LPCTSTR)strDescription;
			}

			g_aryFinancialTabInfoT.Add(pNew);
			rs->MoveNext();
			i++;
		}

		if (i == 0) {
			MsgBox("A charge has been removed from the screen. Practice will now requery this patient's financial information.");
			FullRefresh();
			return;
		}

		// Step 5: Add the applies
		if (InExpandedSubList(iChargeID)) {
			int i = 0;

			// (j.jones 2009-12-30 12:28) - PLID 36729 - this recordset should have already been loaded above
			rs = rs->NextRecordset(NULL);
			while(!rs->eof) {
				pNew = new FinTabItem;
				pNew->LineID = (long)GetNewNum("T");
				pNew->BillID = rs->Fields->Item["Expr1"]->Value;
				pNew->LineType = rs->Fields->Item["Expr2"]->Value;
				pNew->Date = rs->Fields->Item["Date"]->Value;
				pNew->Location = rs->Fields->Item["LocName"]->Value;
				pNew->NoteIcon = rs->Fields->Item["NoteIcon"]->Value;
				pNew->HasNotes = (long)rs->Fields->Item["HasNotes"]->Value;
				pNew->OnHold = (long)0;	// (a.wilson 2014-07-24 13:03) - PLID 63015	
				// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
				pNew->IsVoided = VarLong(rs->Fields->Item["IsVoided"]->Value, 0);
				pNew->IsCorrected = VarLong(rs->Fields->Item["IsCorrected"]->Value, 0);
				pNew->Description = rs->Fields->Item["Expr3"]->Value;
				pNew->ChargeID = rs->Fields->Item["ChargeID"]->Value;
				// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field
				pNew->TotalPayAmount = rs->Fields->Item["TotalPayAmount"]->Value;
				pNew->PayAmount = rs->Fields->Item["Expr4"]->Value;
				pNew->Adjustment = rs->Fields->Item["Expr5"]->Value;
				pNew->PayID = rs->Fields->Item["SourceID"]->Value;
				pNew->ApplyID = rs->Fields->Item["ID"]->Value;
				pNew->ExpandChar = rs->Fields->Item["Expr6"]->Value;
				pNew->ExpandSubChar = rs->Fields->Item["Expr7"]->Value;
				pNew->Discount = _variant_t(COleCurrency(0,0)); // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get the discount total
				pNew->HasPercentOff = (long)0; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get has percent off discount
				// (j.jones 2008-09-17 10:00) - PLID 19623 - supported new columns
				pNew->Provider = rs->Fields->Item["Provider"]->Value;
				pNew->POSName = rs->Fields->Item["POSName"]->Value;
				pNew->POSCode = rs->Fields->Item["POSCode"]->Value;
				pNew->InputDate = rs->Fields->Item["InputDate"]->Value;
				pNew->CreatedBy = rs->Fields->Item["CreatedBy"]->Value;
				pNew->FirstChargeDate = rs->Fields->Item["FirstChargeDate"]->Value;

				// (j.jones 2010-05-26 16:50) - PLID 28184 - added diagnosis code fields
				pNew->DiagCode1 = rs->Fields->Item["DiagCode1"]->Value;
				pNew->DiagCode1WithName = rs->Fields->Item["DiagCode1WithName"]->Value;
				pNew->DiagCodeList = rs->Fields->Item["BillDiagCodeList"]->Value;

				// (j.jones 2010-09-01 15:39) - PLID 40331 - added allowable
				// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
				// and added ChargeAllowable
				pNew->FeeSchedAllowable = g_cvarNull;
				pNew->ChargeAllowable = g_cvarNull;

				// (j.jones 2011-12-20 11:30) - PLID 47119 - added deductible and coinsurance
				pNew->ChargeDeductible = g_cvarNull;
				pNew->ChargeCoinsurance = g_cvarNull;

				// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number, always filled if
				// a value exists, even if the invoice number feature is turned off
				pNew->InvoiceNumber = rs->Fields->Item["InvoiceID"]->Value;

				// (d.singleton 2014-10-09 17:57) - PLID 62698 add the claim number to the line item description
				long nPrefType = GetRemotePropertyInt("ERemitAddClaimNumberToDescription", 0, 0, "<None>", true);
				if (nPrefType > 0) {
					CString strDescription, strClaimNumber, strNewDescription, strPaymentType;
					strDescription = VarString(pNew->Description, "");
					strClaimNumber = AdoFldString(rs, "ClaimNumber", "");
					strPaymentType = VarString(pNew->LineType, "");
					if (!strClaimNumber.IsEmpty()) {
						if (nPrefType == 1 && (VarCurrency(pNew->PayAmount, g_ccyZero) > g_ccyZero)) {
							strNewDescription.Format("%s Claim Number: %s", strDescription, strClaimNumber);
							pNew->Description = _variant_t(strNewDescription);
						}
						else if (nPrefType == 2 && (VarCurrency(pNew->Adjustment, g_ccyZero) > g_ccyZero)) {
							strNewDescription.Format("%s Claim Number: %s", strDescription, strClaimNumber);
							pNew->Description = _variant_t(strNewDescription);
						}
						else if (nPrefType == 3) {
							strNewDescription.Format("%s Claim Number: %s", strDescription, strClaimNumber);
							pNew->Description = _variant_t(strNewDescription);
						}
					}
				}

				g_aryFinancialTabInfoT.Add(pNew);
				rs->MoveNext();
				i++;
			}

			if (i > 0) {
				// Insert a blank line
				pNew = new FinTabItem;
				pNew->LineID = (long)GetNewNum("T");
				pNew->LineType = _variant_t(_T("Blank"));
				pNew->BillID = (long)iBillID;
				pNew->ChargeID = (long)iChargeID;
				pNew->ExpandChar = "BITMAP:DETAILLINE";
				pNew->HasNotes = (long)(0);
				pNew->OnHold = (long)0;	// (a.wilson 2014-07-24 13:03) - PLID 63015
				// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
				pNew->IsVoided = (long)0;
				pNew->IsCorrected = (long)0;
				pNew->Discount = _variant_t(COleCurrency(0,0)); // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get the discount total
				pNew->HasPercentOff = (long)0; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get has percent off discount
				g_aryFinancialTabInfoT.Add(pNew);
			}
			else {
				// Remove the up arrow from the payment
				for(int w=0;(w+1)<g_aryFinancialTabInfoT.GetSize();w++) {
				}
				if(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(w))->LineID.vt == VT_I4
					&& ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(w))->LineID.lVal == iLineID)
					((FinTabItem*)g_aryFinancialTabInfoT.GetAt(w))->ExpandSubChar.vt = VT_NULL;
			}
		}

		rs->Close();

		// Step 6: Add all the lines from FinancialTabInfoTemp2 back to FinancialTabInfoT
		for(int t=0;t<g_aryFinancialTabInfoTemp2.GetSize();t++) {
			pNew = new FinTabItem;
			FinTabItemCopy(((FinTabItem*)g_aryFinancialTabInfoTemp2.GetAt(t)),pNew);
			pNew->LineID = (long)GetNewNum("T");
			g_aryFinancialTabInfoT.Add(pNew);
		}

	}NxCatchAll("Error in FinancialDlg::RedrawCharge");

}

/*************************************************************
* This function will delete all standalone payments from
* FinancialTabInfoT (the table used by the listbox) and
* regenerate them.
*
* Callers:	OnLeftClickBillingTabList
*			OnColumnClickFinancialList
*			OnNewBill
*			OnNewPayment
*			OnPopupSelectionFinancialList
*			OnEndDragDropFinancialList
*			ExpandPayment
*			FillTabInfo
*			Pay_Adj_Ref_Click (May be called by OnLeftClickBillingTabList --
*				by clicking on the payment adjustment or refund column)
*
*************************************************************/
void CFinancialDlg::RedrawAllPayments()
{
	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//LogDetail("Redrawing all payments");
	CWaitCursor pWait;

	try {
		CString str;

		// (j.jones 2009-12-29 11:01) - PLID 27237 - replaced IsRecordsetEmpty with a parameterized recordset
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM LineItemT "
			"WHERE PatientID = {INT} AND LineItemT.Deleted = 0 AND LineItemT.Type >=1  AND LineItemT.Type <= 3",m_iCurPatient);
		if(rs->eof) {
			RefreshList();
			return;
		}
		rs->Close();

		// Step 1: Delete all the payments adjustments and refunds from both tables. If
		// we're using the special view, just delete everything from both tables.
		for(int z=g_aryFinancialTabInfoT.GetSize()-1;z>=0;z--) {
			if(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(z))->LineType.vt == VT_BSTR) {
				if(CString(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(z))->LineType.bstrVal) == CString("Payment") ||
					CString(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(z))->LineType.bstrVal) == CString("Adjustment") ||
					CString(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(z))->LineType.bstrVal) == CString("Refund") ||
					CString(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(z))->LineType.bstrVal) == CString("AppliedPayToPay") ||
					(CString(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(z))->LineType.bstrVal) == CString("Blank") &&
					((FinTabItem*)g_aryFinancialTabInfoT.GetAt(z))->PayID.vt == VT_I4)) {
						delete (FinTabItem*)g_aryFinancialTabInfoT.GetAt(z);
						g_aryFinancialTabInfoT.RemoveAt(z);
				}
			}
		}

		for(int x=g_aryFinancialTabInfoTemp.GetSize()-1;x>=0;x--) {
			delete (FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(x);
		}
		g_aryFinancialTabInfoTemp.RemoveAll();


		// Step 2: Add all the payments adjustments and refunds to the temporary table
		// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
		//LogDetail("Executing CreateAddUnappliedPaysRecordset...");
		int i = 0;
		// (j.jones 2009-12-30 10:40) - PLID 36729 - this code has been converted
		// to get the recordset from a function that creates and opens it
		rs = CreateAddUnappliedPaysRecordset(m_iCurPatient);

		FinTabItem *pNew;
		while(!rs->eof) {
			pNew = new FinTabItem;
			pNew->LineID = (long)GetNewNum("Temp");
			pNew->PayID = rs->Fields->Item["ID"]->Value;
			pNew->LineType = rs->Fields->Item["LineType"]->Value;
			pNew->Date = rs->Fields->Item["Date"]->Value;
			pNew->Location = rs->Fields->Item["LocName"]->Value;
			pNew->NoteIcon = rs->Fields->Item["NoteIcon"]->Value;
			pNew->HasNotes = (long)rs->Fields->Item["HasNotes"]->Value;
			pNew->OnHold = (long)0; // (a.wilson 2014-07-24 14:26) - PLID 63015			
			// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
			pNew->IsVoided = VarLong(rs->Fields->Item["IsVoided"]->Value, 0);
			pNew->IsCorrected = VarLong(rs->Fields->Item["IsCorrected"]->Value, 0);
			pNew->Description = rs->Fields->Item["Description"]->Value;
			pNew->ShowChargeAmount = rs->Fields->Item["ShowChargeAmount"]->Value;
			pNew->ShowBalanceAmount = rs->Fields->Item["ShowBalanceAmount"]->Value;
			// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field
			pNew->TotalPayAmount = rs->Fields->Item["TotalPayAmount"]->Value;
			pNew->PayAmount = rs->Fields->Item["PayAmount"]->Value;
			pNew->Adjustment = rs->Fields->Item["Adjustment"]->Value;
			pNew->Refund = rs->Fields->Item["Refund"]->Value;
			pNew->BalanceAmount = rs->Fields->Item["Balance"]->Value;		
			pNew->ExpandChar = rs->Fields->Item["ExpandChar"]->Value;
			pNew->Discount = _variant_t(COleCurrency(0,0)); // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get the discount total
			pNew->HasPercentOff = (long)0; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get has percent off discount
			// (j.jones 2008-09-17 10:00) - PLID 19623 - supported new columns
			pNew->Provider = rs->Fields->Item["Provider"]->Value;
			pNew->POSName = rs->Fields->Item["POSName"]->Value;
			pNew->POSCode = rs->Fields->Item["POSCode"]->Value;
			pNew->InputDate = rs->Fields->Item["InputDate"]->Value;
			pNew->CreatedBy = rs->Fields->Item["CreatedBy"]->Value;
			pNew->FirstChargeDate = rs->Fields->Item["FirstChargeDate"]->Value;

			// (j.jones 2010-05-26 16:50) - PLID 28184 - added diagnosis code fields
			pNew->DiagCode1 = rs->Fields->Item["DiagCode1"]->Value;
			pNew->DiagCode1WithName = rs->Fields->Item["DiagCode1WithName"]->Value;
			pNew->DiagCodeList = rs->Fields->Item["BillDiagCodeList"]->Value;

			// (j.jones 2010-09-01 15:39) - PLID 40331 - added allowable
			// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
			// and added ChargeAllowable
			pNew->FeeSchedAllowable = g_cvarNull;
			pNew->ChargeAllowable = g_cvarNull;

			// (j.jones 2011-12-20 11:30) - PLID 47119 - added deductible and coinsurance
			pNew->ChargeDeductible = g_cvarNull;
			pNew->ChargeCoinsurance = g_cvarNull;

			// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number, it will be null here
			pNew->InvoiceNumber = rs->Fields->Item["InvoiceID"]->Value;

			// (d.singleton 2014-10-09 17:57) - PLID 62698 add the claim number to the line item description
			long nPrefType = GetRemotePropertyInt("ERemitAddClaimNumberToDescription", 0, 0, "<None>", true);
			if (nPrefType > 0) {
				CString strDescription, strClaimNumber, strNewDescription, strPaymentType;				
				strDescription = VarString(pNew->Description, "");
				strClaimNumber = AdoFldString(rs, "ClaimNumber", "");
				strPaymentType = VarString(pNew->LineType, "");
				if (!strClaimNumber.IsEmpty()) {
					if (nPrefType == 1 && strPaymentType.CompareNoCase("Payment") == 0) {
						strNewDescription.Format("%s Claim Number: %s", strDescription, strClaimNumber);
						pNew->Description = _variant_t(strNewDescription);
					}
					else if (nPrefType == 2 && strPaymentType.CompareNoCase("Adjustment") == 0) {
						strNewDescription.Format("%s Claim Number: %s", strDescription, strClaimNumber);
						pNew->Description = _variant_t(strNewDescription);
					}
					else if (nPrefType == 3) {
						strNewDescription.Format("%s Claim Number: %s", strDescription, strClaimNumber);
						pNew->Description = _variant_t(strNewDescription);
					}
				}				
			}

			g_aryFinancialTabInfoTemp.Add(pNew);
			rs->MoveNext();
			i++;
		}
		rs->Close();

		// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
		//LogDetail("CreateAddUnappliedPaysRecordset added %d rows", i);

		if (m_aiExpandedPayBranches.GetSize() == 0) 
		{	
			for(int t=0;t<g_aryFinancialTabInfoTemp.GetSize();t++) {
				pNew = new FinTabItem;
				FinTabItemCopy(((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(t)),pNew);
				pNew->LineID = (long)GetNewNum("T");
				g_aryFinancialTabInfoT.Add(pNew);
			}
			RefreshList();
			return;
		}

		// TODO: Suspect code!
		long iSrcLineID;
		long iDestLineID;
		long iPayID;

		if(g_aryFinancialTabInfoTemp.GetSize() > 0) {
			iSrcLineID = ((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(0))->LineID.lVal;
			iDestLineID = ((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(0))->LineID.lVal;
		}

		for(int q=0;q<g_aryFinancialTabInfoTemp.GetSize();q++) {
			if (((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(q))->PayID.vt != VT_I4) {
				iDestLineID++;
				continue;
			}
			iPayID = ((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(q))->PayID.lVal;

			for (int i=0; i < m_aiExpandedPayBranches.GetSize(); i++) {
				if (m_aiExpandedPayBranches.GetAt(i) == iPayID) {
					for(int j=0;j<g_aryFinancialTabInfoTemp.GetSize();j++) {

						// (j.jones 2015-02-23 16:38) - PLID 64934 - payments have the DOWNARROW for ExpandChar
						// if they have applies, NULL if they do not, so if ExpandChar is NULL, ignore the pay's
						// presence in m_aiExpandedPayBranches
						FinTabItem *pItem = (FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(j);
						if (VarLong(pItem->PayID) == iPayID
							&& pItem->ExpandChar.vt != VT_NULL) {

							if (VarString(pItem->LineType) == "Payment") {
								pItem->ExpandChar = _T("BITMAP:UPARROW");
								pItem->Refund = g_cvarNull;
								pItem->Adjustment = g_cvarNull;
							}
							if (VarString(pItem->LineType) == "Adjustment") {
								pItem->ExpandChar = _T("BITMAP:UPARROW");
								pItem->Refund = g_cvarNull;
								pItem->PayAmount = g_cvarNull;
							}
							if (VarString(pItem->LineType) == "Refund") {
								pItem->ExpandChar = _T("BITMAP:UPARROW");
								pItem->PayAmount = g_cvarNull;
								pItem->Adjustment = g_cvarNull;
							}
						}
					}

					for(int x=0;x<g_aryFinancialTabInfoTemp.GetSize();x++) {
						if(((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(x))->LineID.lVal >= iSrcLineID &&
							((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(x))->LineID.lVal <= iDestLineID) {
								pNew = new FinTabItem;
								FinTabItemCopy(((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(x)),pNew);
								pNew->LineID = (long)GetNewNum("T");
								g_aryFinancialTabInfoT.Add(pNew);
						}
					}

					// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
					//LogDetail("Executing CreateAddAppliedPayToPayRecordset where Payment ID = %d", iPayID);
					int i = 0;
					// (j.jones 2009-12-30 09:33) - PLID 36729 - this code has been converted
					// to get the recordset from a function that creates and opens it
					_RecordsetPtr rs = CreateAddAppliedPayToPayRecordset(iPayID, m_iCurPatient);
					while(!rs->eof) {
						pNew = new FinTabItem;
						pNew->LineID = (long)GetNewNum("T");
						pNew->ApplyID = rs->Fields->Item["ApplyID"]->Value;
						pNew->PayID = rs->Fields->Item["PayID"]->Value;
						pNew->ExpandChar = rs->Fields->Item["Expr1"]->Value;
						pNew->Date = rs->Fields->Item["Date"]->Value;
						pNew->Location = rs->Fields->Item["LocName"]->Value;
						pNew->NoteIcon = rs->Fields->Item["NoteIcon"]->Value;
						pNew->HasNotes = (long)rs->Fields->Item["HasNotes"]->Value;
						pNew->OnHold = (long)0;	// (a.wilson 2014-07-24 14:26) - PLID 63015
						// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
						pNew->IsVoided = VarLong(rs->Fields->Item["IsVoided"]->Value, 0);
						pNew->IsCorrected = VarLong(rs->Fields->Item["IsCorrected"]->Value, 0);
						pNew->Description = rs->Fields->Item["Expr2"]->Value;
						pNew->LineType = rs->Fields->Item["Expr3"]->Value;
						// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field
						pNew->TotalPayAmount = rs->Fields->Item["TotalPayAmount"]->Value;
						pNew->PayAmount = rs->Fields->Item["Expr4"]->Value;
						pNew->Adjustment = rs->Fields->Item["Expr5"]->Value;
						pNew->Refund = rs->Fields->Item["Expr6"]->Value;
						pNew->Discount = _variant_t(COleCurrency(0,0)); // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get the discount total
						pNew->HasPercentOff = (long)0; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get has percent off discount
						// (j.jones 2008-09-17 10:00) - PLID 19623 - supported new columns
						pNew->Provider = rs->Fields->Item["Provider"]->Value;
						pNew->POSName = rs->Fields->Item["POSName"]->Value;
						pNew->POSCode = rs->Fields->Item["POSCode"]->Value;
						pNew->InputDate = rs->Fields->Item["InputDate"]->Value;
						pNew->CreatedBy = rs->Fields->Item["CreatedBy"]->Value;
						pNew->FirstChargeDate = rs->Fields->Item["FirstChargeDate"]->Value;

						// (j.jones 2010-05-26 16:50) - PLID 28184 - added diagnosis code fields
						pNew->DiagCode1 = rs->Fields->Item["DiagCode1"]->Value;
						pNew->DiagCode1WithName = rs->Fields->Item["DiagCode1WithName"]->Value;
						pNew->DiagCodeList = rs->Fields->Item["BillDiagCodeList"]->Value;

						// (j.jones 2010-09-01 15:39) - PLID 40331 - added allowable
						// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
						// and added ChargeAllowable
						pNew->FeeSchedAllowable = g_cvarNull;
						pNew->ChargeAllowable = g_cvarNull;

						// (j.jones 2011-12-20 11:30) - PLID 47119 - added deductible and coinsurance
						pNew->ChargeDeductible = g_cvarNull;
						pNew->ChargeCoinsurance = g_cvarNull;

						// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number, it will be null here
						pNew->InvoiceNumber = rs->Fields->Item["InvoiceID"]->Value;

						// (d.singleton 2014-10-09 17:57) - PLID 62698 add the claim number to the line item description
						long nPrefType = GetRemotePropertyInt("ERemitAddClaimNumberToDescription", 0, 0, "<None>", true);
						if (nPrefType > 0) {
							CString strDescription, strClaimNumber, strNewDescription, strPaymentType;
							strDescription = VarString(pNew->Description, "");
							strClaimNumber = AdoFldString(rs, "ClaimNumber", "");
							if (!strClaimNumber.IsEmpty()) {
								if (nPrefType == 1 && (VarCurrency(pNew->PayAmount, g_ccyZero) > g_ccyZero)) {
									strNewDescription.Format("%s Claim Number: %s", strDescription, strClaimNumber);
									pNew->Description = _variant_t(strNewDescription);
								}
								else if (nPrefType == 2 && (VarCurrency(pNew->Adjustment, g_ccyZero) > g_ccyZero)) {
									strNewDescription.Format("%s Claim Number: %s", strDescription, strClaimNumber);
									pNew->Description = _variant_t(strNewDescription);
								}
								//need to make sure this is either a payment or adjustment in case of refunds
								else if (nPrefType == 3 && (pNew->Adjustment.vt != VT_NULL || pNew->PayAmount.vt != VT_NULL)) {
									strNewDescription.Format("%s Claim Number: %s", strDescription, strClaimNumber);
									pNew->Description = _variant_t(strNewDescription);
								}
							}
						}

						g_aryFinancialTabInfoT.Add(pNew);
						rs->MoveNext();
						i++;
					}
					rs->Close();

					pNew = new FinTabItem;
					pNew->LineID = (long)GetNewNum("T");
					pNew->LineType = _variant_t(_T("Blank"));
					pNew->PayID = _variant_t(iPayID);
					pNew->HasNotes = (long)(0);
					pNew->OnHold = (long)0; // (a.wilson 2014-07-24 14:27) - PLID 63015
					// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
					pNew->IsVoided = (long)0;
					pNew->IsCorrected = (long)0;
					pNew->Discount = _variant_t(COleCurrency(0,0)); // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get the discount total
					pNew->HasPercentOff = (long)0; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get has percent off discount
					g_aryFinancialTabInfoT.Add(pNew);

					iSrcLineID = iDestLineID + 1;
					break;
				}
			}

			iDestLineID++;
		}
		/*str.Format("INSERT INTO FinancialTabInfoT (LineType, Expandable, ExpandChar, ExpandSubChar, [Date], Description, ShowChargeAmount, BillID, ChargeID, PayID, PayToPayID, ApplyID, ShowBalanceAmount, ChargeAmount, PayAmount, Adjustment, Refund, BalanceAmount, InsResp1, InsResp2, InsRespOther, PatResp) "
		"SELECT FinancialTabInfoTemp.LineType, FinancialTabInfoTemp.Expandable, FinancialTabInfoTemp.ExpandChar, FinancialTabInfoTemp.ExpandSubChar, FinancialTabInfoTemp.Date, FinancialTabInfoTemp.Description, FinancialTabInfoTemp.ShowChargeAmount, FinancialTabInfoTemp.BillID, FinancialTabInfoTemp.ChargeID, FinancialTabInfoTemp.PayID, FinancialTabInfoTemp.PayToPayID, FinancialTabInfoTemp.ApplyID, FinancialTabInfoTemp.ShowBalanceAmount, FinancialTabInfoTemp.ChargeAmount, FinancialTabInfoTemp.PayAmount, FinancialTabInfoTemp.Adjustment, FinancialTabInfoTemp.Refund, FinancialTabInfoTemp.BalanceAmount, FinancialTabInfoTemp.InsResp1, FinancialTabInfoTemp.InsResp2, FinancialTabInfoTemp.InsRespOther, FinancialTabInfoTemp.PatResp FROM FinancialTabInfoTemp WHERE FinancialTabInfoTemp.LineID >= %d AND FinancialTabInfoTemp.LineID <= %d",
		iSrcLineID, iDestLineID);
		*/
		for(int y=0;y<g_aryFinancialTabInfoTemp.GetSize();y++) {
			if(((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(y))->LineID.vt == VT_I4
				&& ((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(y))->LineID.lVal >= iSrcLineID
				&& ((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(y))->LineID.lVal <= iDestLineID) {
					pNew = new FinTabItem;
					FinTabItemCopy(((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(y)),pNew);
					pNew->LineID = (long)GetNewNum("T");
					g_aryFinancialTabInfoT.Add(pNew);
			}
		}
	}
	NxCatchAll("FinancialDlg::RedrawAllPayments");

	RefreshList();
}

/********************************************************
*	ExpandBill() will expand or collapse the list of
*	charges directed from a bill
*
* Callers:	OnLeftClickBillingTabList
*			
********************************************************/
void CFinancialDlg::ExpandBill(long iBillID, long iLineID)
{
	BOOL boWasExpanded = FALSE;
	CString str;

	CWaitCursor pWait;
	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//LogDetail("Expanding/Collapsing Bill %d  at Line %d", iBillID, iLineID);

	/////////////////////////////////////////////////////////////
	// Look to see if this bill is already expanded. If so, collapse
	// it and exit.
	for (int i=0; i < m_aiExpandedBranches.GetSize(); i++) {
		if (m_aiExpandedBranches[i] == iBillID) {
			m_aiExpandedBranches.RemoveAt(i);
			boWasExpanded = TRUE;

			// (j.jones 2015-03-17 13:25) - PLID 64931 - clear the search results window
			ClearSearchResults();
			break;
		}
	}

	/////////////////////////////////////////////////////////////
	// Otherwise, expand the bill
	if (!boWasExpanded) {
		//it's just this easy to expand the bill
		m_aiExpandedBranches.Add(iBillID);
	}
	else {
		//////////////////////////////////////////////////////////////////////////////
		// If we're collapsing the bill, collapse all the charges as well. Note: This
		// SQL will not include the actual bill because the bill charge ID is Null.
		_RecordsetPtr rs(__uuidof(Recordset));
		_variant_t var;

		for(int j=0;j<g_aryFinancialTabInfoT.GetSize();j++) {
			if(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(j))->BillID.vt == VT_I4
				&& ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(j))->BillID.lVal == iBillID
				&& ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(j))->ChargeID.vt == VT_I4) {
					for (int i=0; i < m_aiExpandedSubBranches.GetSize(); i++) {
						if (m_aiExpandedSubBranches[i] == ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(j))->ChargeID.lVal) {
							m_aiExpandedSubBranches.RemoveAt(i);

							// (j.jones 2015-03-17 13:25) - PLID 64931 - clear the search results window
							ClearSearchResults();
							break;
						}

					}
			}
		}
	}

	DisableFinancialScreen();

	RedrawBill(iBillID);

	// (j.jones 2010-06-07 17:11) - PLID 39042 - m_SavedRow_LineID now references the actual LineID value
	IRowSettingsPtr pRow = m_List->FindByColumn(m_nBillIDColID,(long)iBillID,NULL,TRUE);
	if(pRow) {
		m_SavedRow_LineID = VarLong(pRow->GetValue(btcLineID), 0);
	}
	else {
		m_SavedRow_LineID = 0;
	}

	RefreshList();

	//if we want to expand all the charges, this is where we should do it
	if(!boWasExpanded && m_bOneClickExpansion) {

		//this is a pseudo-copy of the code from OnColumnClicking

		COleVariant var;
		long BillID, ChargeID, LineID;
		//expand all the charges for this bill
		// (j.jones 2010-06-07 14:50) - PLID 39042 - this is now a datalist 2
		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while(pRow) {

			//only do this for the currently expanded bill
			var = pRow->GetValue(m_nBillIDColID);
			if(var.vt != VT_I4 || VarLong(var) != iBillID) {
				pRow = pRow->GetNextRow();
				continue;
			}

			//first check to see if the expand char is valid, otherwise there is no need
			//to get the remaining data
			var = pRow->GetValue(m_nExpandSubCharColID);
			if(var.vt != VT_BSTR) {
				pRow = pRow->GetNextRow();
				continue;
			}
			//only expand if it is not a detail line (therefore it is an arrow)
			if (CString(var.bstrVal) != "BITMAP:DOWNARROW" && CString(var.bstrVal) != "BITMAP:UPARROW") {
				pRow = pRow->GetNextRow();
				continue;
			}
			//now get the IDs. If any are invalid, skip it.
			var = pRow->GetValue(m_nBillIDColID);
			if(var.vt != VT_I4) {
				pRow = pRow->GetNextRow();
				continue;
			}
			BillID = VarLong(var);
			var = pRow->GetValue(btcChargeID);
			if(var.vt != VT_I4) {
				pRow = pRow->GetNextRow();
				continue;
			}
			ChargeID = VarLong(var);
			var = pRow->GetValue(btcLineID);
			if(var.vt != VT_I4) {
				pRow = pRow->GetNextRow();
				continue;
			}
			LineID = VarLong(var);

			//you will not get to this point unless all the data is valid
			ExpandCharge(ChargeID,BillID,LineID);

			//we have to go back to the line ID we were on, then keep advancing
			pRow = m_List->FindByColumn(btcLineID, (long)LineID, NULL, FALSE);
			if(pRow) {
				pRow = pRow->GetNextRow();
			}
		}
	}

	EnableFinancialScreen();

	//re-select the bill, but do not change the selection if it doesn't exist
	IRowSettingsPtr pSelRow = m_List->FindByColumn(m_nBillIDColID, (long)iBillID, NULL, FALSE);
	if(pSelRow) {
		m_List->PutCurSel(pSelRow);
	}
}

/********************************************************
*	ExpandCharge() will expand or collapse the list of
*	applies directed to a single charge.
*
* Callers:	OnLeftClickBillingTabList
*			
********************************************************/
void CFinancialDlg::ExpandCharge(long iChargeID, long iBillID, long iLineID)
{
	BOOL boWasExpanded = FALSE;
	CString str;

	CWaitCursor pWait;
	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//LogDetail("Expanding/Collapsing Charge %d (Bill %d)  at Line %d", iChargeID, iBillID, iLineID);

	for (int i=0; i < m_aiExpandedSubBranches.GetSize(); i++) {
		if (m_aiExpandedSubBranches[i] == iChargeID) {
			m_aiExpandedSubBranches.RemoveAt(i);
			boWasExpanded = TRUE;

			// (j.jones 2015-03-17 13:25) - PLID 64931 - clear the search results window
			ClearSearchResults();
			break;
		}
	}

	if (!boWasExpanded) {
		m_aiExpandedSubBranches.Add(iChargeID);
	}

	RedrawCharge(iBillID, iChargeID, iLineID);

	// (j.jones 2010-06-07 17:11) - PLID 39042 - m_SavedRow_LineID now references the actual LineID value
	IRowSettingsPtr pRow = m_List->FindByColumn(btcChargeID,(long)iChargeID,NULL,TRUE);
	if(pRow) {
		m_SavedRow_LineID = VarLong(pRow->GetValue(btcLineID), 0);
	}
	else {
		m_SavedRow_LineID = 0;
	}

	RefreshList();

	//re-select the charge, but do not change the selection if it doesn't exist
	IRowSettingsPtr pSelRow = m_List->FindByColumn(btcChargeID, (long)iChargeID, NULL, FALSE);
	if(pSelRow) {
		m_List->PutCurSel(pSelRow);
	}
}

/********************************************************
*	ExpandPayment() will expand or collapse the list of
*	payments/adjustments/refunds directed from a
*	payment/adjustment/refund
*
* Callers:	OnLeftClickBillingTabList
********************************************************/
void CFinancialDlg::ExpandPayment(long iPaymentID)
{
	COleVariant var;
	CWaitCursor pWait;

	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//LogDetail("Expanding/Collapsing Payment %d", iPaymentID);

	// (j.jones 2010-06-07 17:11) - PLID 39042 - m_SavedRow_LineID now references the actual LineID value
	IRowSettingsPtr pRow = m_List->FindByColumn(btcPayID,(long)iPaymentID,NULL,TRUE);
	if(pRow) {
		m_SavedRow_LineID = VarLong(pRow->GetValue(btcLineID), 0);
	}
	else {
		m_SavedRow_LineID = 0;
	}

	/////////////////////////////////////////////////////////////
	// Look to see if this payment is already expanded. If so, collapse
	// it and exit.
	for (int i=0; i < m_aiExpandedPayBranches.GetSize(); i++) {
		if (m_aiExpandedPayBranches[i] == iPaymentID) {
			m_aiExpandedPayBranches.RemoveAt(i);

			RedrawAllPayments();

			//re-select the payment, but do not change the selection if it doesn't exist
			IRowSettingsPtr pSelRow = m_List->FindByColumn(btcPayID, (long)iPaymentID, NULL, FALSE);
			if(pSelRow) {
				m_List->PutCurSel(pSelRow);
			}

			// (j.jones 2015-03-17 13:25) - PLID 64931 - clear the search results window
			ClearSearchResults();
			return;
		}
	}

	/////////////////////////////////////////////////////////////
	// Otherwise, expand the payment
	m_aiExpandedPayBranches.Add(iPaymentID);
	RedrawAllPayments();

	//re-select the payment, but do not change the selection if it doesn't exist
	IRowSettingsPtr pSelRow = m_List->FindByColumn(btcPayID, (long)iPaymentID, NULL, FALSE);
	if(pSelRow) {
		m_List->PutCurSel(pSelRow);
	}
}

/********************************************************
*	FullRefresh will completely rebuild all queries,
*	empty the expanded charges & payments lists, and
*	fully requery FinancialTabInfoT with all financial
*	information, as opposed to QuickRefresh and FillTabInfo
*	which do not rebuild the queries.
*
* Callers:	OnLeftClickBillingTabList
*			OnPopupSelectionFinancialList
*			OnEndDragDropFinancialList
*			RedrawBill
*			RedrawCharge
*			Pay_Adj_Ref_Click
*			
********************************************************/
void CFinancialDlg::FullRefresh() //::rebuild ::requery
{
	EnsureRemoteData();

	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//LogDetail("Beginning full refresh");

	CWaitCursor pWait;
	for(int x=g_aryFinancialTabInfoTemp.GetSize()-1;x>=0;x--) {
		delete (FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(x);
	}
	g_aryFinancialTabInfoTemp.RemoveAll();
	ClearExpandLists();

	if(m_List->GetRowCount() > 0) {
		m_List->PutTopRow(m_List->GetFirstRow());
		m_List->PutCurSel(m_List->GetFirstRow());
	}
	FillTabInfo();
}

void CFinancialDlg::QuickRefresh()
{
	EnsureRemoteData();

	// a.walling 4/13/06 PL 16493
	SaveExpansionInfo();

	CWaitCursor pWait;
	for(int x=g_aryFinancialTabInfoTemp.GetSize()-1;x>=0;x--) {
		delete (FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(x);
	}
	g_aryFinancialTabInfoTemp.RemoveAll();
	ClearExpandLists();

	if(m_List->GetRowCount() > 0) {
		m_List->PutTopRow(m_List->GetFirstRow());
		m_List->PutCurSel(m_List->GetFirstRow());
	}
	m_SavedRow_LineID = 0;
	m_SavedTopRow_LineID = 0;

	if(GetRemotePropertyInt("ShowFinancialGridLines",1,0,GetCurrentUserName(),TRUE)==1) {
		m_List->GridVisible = TRUE;
	}
	else {
		m_List->GridVisible = FALSE;
	}

	// (j.jones 2015-02-23 12:48) - PLID 64934 - added checkbox to hide voided items,
	// which is also a preference, so we need to reload the status on refresh
	m_checkHideVoidedItems.SetCheck(GetRemotePropertyInt("BillingTabHideVoidedItems", 1, 0, GetCurrentUserName(), true) == 0 ? FALSE : TRUE);

	m_checkHideZeroBalances.SetCheck(GetRemotePropertyInt("BillingHideBalances", 1, 0, GetCurrentUserName(), TRUE) == 0 ? FALSE : TRUE);
	long nRange = GetRemotePropertyInt("BillingHideBalancesRange", 3, 0, GetCurrentUserName(), TRUE);
	long nRangeType = GetRemotePropertyInt("BillingHideBalancesRangeType", 2, 0, GetCurrentUserName(), TRUE);
	//nRangeType: 1 = Days, 2 = Months, 3 = Years
	CString strRangeType;
	if(nRangeType == 1)
		strRangeType = "Days";
	else if(nRangeType == 2)
		strRangeType = "Months";
	else
		strRangeType = "Years";

	// (j.jones 2006-05-03 13:01) - change the label to match the preferences
	// (j.jones 2015-02-19 09:18) - PLID 64942 - this now says "Financial Items", not "Bills", because payments can also now be hidden
	CString strZeroBalanceLabel;
	strZeroBalanceLabel.Format("Hide Zero-Balance Financial Items Over %li %s Old", nRange, strRangeType);
	SetDlgItemText(IDC_CHECK_HIDE_ZERO_BALANCES, strZeroBalanceLabel);

	CRect rc;
	GetDlgItem(IDC_CHECK_HIDE_ZERO_BALANCES)->GetWindowRect(rc);
	ScreenToClient(rc);
	InvalidateRect(rc);

	// (d.thompson 2008-12-09) - PLID 32370 - Wrote a mod that split 'InsViewType' to only include
	//	the insurance fields, and put the refunds into its own property.
	// (j.jones 2010-09-27 15:56) - PLID 40123 - InsViewType is obsolete, replaced with BillingTab_ShowInsuranceColumns,
	// but is used to determine the default value for BillingTab_ShowInsuranceColumns. The new values for this
	// preference are: 0 - hide all, 1 - show all, 2 - hide unused
	long nOldInsViewType = GetRemotePropertyInt("InsViewType", 0, 0, GetCurrentUserName(), false);
	//the old preference had more options, if any were nonzero, they are 1 now, meaning
	//that showing anything before now shows all insurances
	if(nOldInsViewType != 0) {
		nOldInsViewType = 1;
	}
	m_iInsuranceViewToggle = GetRemotePropertyInt("BillingTab_ShowInsuranceColumns", nOldInsViewType, 0, GetCurrentUserName());

	//remember that the button shows the NEXT option in the sequence, not the currently displayed view
	switch (m_iInsuranceViewToggle) {	
		case 1:  //currently showing all
			GetDlgItem(IDC_SHOW_INS_BALANCE)->SetWindowText("Hide Unused Insurance");
			break;
		case 2: //currently hiding unused
			GetDlgItem(IDC_SHOW_INS_BALANCE)->SetWindowText("Hide Insurance");
			break;
		case 0: //currently hiding all
		default:
			GetDlgItem(IDC_SHOW_INS_BALANCE)->SetWindowText("Show Insurance");
			break;
	}

	// (d.thompson 2008-12-09) - PLID 32370 - Split out from InsViewType to its own preference.  Default
	//	remains off.
	m_nShowRefunds = GetRemotePropertyInt("ShowBillingRefunds", 0, 0, GetCurrentUserName());
	if (m_nShowRefunds)
		((CButton *)GetDlgItem(IDC_SHOW_REFUNDS))->SetCheck(TRUE);
	else
		((CButton *)GetDlgItem(IDC_SHOW_REFUNDS))->SetCheck(FALSE);

	DisableFinancialScreen();

	RestoreExpansionInfo(); // a.walling PL 16493

	FillTabInfo();

	EnableFinancialScreen();
}

// Local definition solely for code readability
#define SET_Q_SQL(pq, sql) if (bNeedCreate) pq->Create(NULL, sql); else pq->SetSQL(sql);

/********************************************************
*	FillTabInfo() will make financial records become
*	visible to the user by filling in FinancialTabInfoT
*	with line items; including bills, charges, and
*	transactions. This is the most often called method of
*	refreshing the listbox with financial information
*
* Callers:	UpdateView
*			OnLeftClickBillingTabList
*			FullRefresh (last thing called)
*			QuickRefresh
*			Pay_Adj_Ref_Click
*
********************************************************/
void CFinancialDlg::FillTabInfo()
{
	CString str,temp;

	TRACE("::FillTabInfo\n");

	try {
		CWaitCursor pWait;

		DisableFinancialScreen();

		/////////////////////////////////////////////////////////////
		// Empty the temporary tables
		for(int x=g_aryFinancialTabInfoT.GetSize()-1;x>=0;x--) {
			delete (FinTabItem*)g_aryFinancialTabInfoT.GetAt(x);
		}
		g_aryFinancialTabInfoT.RemoveAll();

		for(x=g_aryFinancialTabInfoTemp.GetSize()-1;x>=0;x--) {
			delete (FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(x);
		}
		g_aryFinancialTabInfoTemp.RemoveAll();

		for(x=g_aryFinancialTabInfoTemp2.GetSize()-1;x>=0;x--) {
			delete (FinTabItem*)g_aryFinancialTabInfoTemp2.GetAt(x);
		}
		g_aryFinancialTabInfoTemp2.RemoveAll();

		//JJ - 5/10/2001 - This must happen every time the screen refreshes, incase they add insurances!
		_RecordsetPtr tmpRS;
		CString tmpSQL;

		// (j.jones 2006-04-20 09:19) - load the insco names now, since we are already opening a recordset,
		// for the possible use as column names
		CString strPrimaryInsuranceName = "Primary";
		CString strSecondaryInsuranceName = "Secondary";
		CString strOtherInsuranceName = "Other Resp";
		long nCountOtherInsurances = 0;

		// (j.jones 2010-06-18 11:37) - PLID 39150 - re-initialize the other insurance column names
		CMap<short, short, CString, CString> mapColumnIndexesToTitles;

		for(int i=0; i<m_aryDynamicRespTypeIDs.GetSize(); i++) {
			long nRespTypeID = (long)m_aryDynamicRespTypeIDs.GetAt(i);

			short iColumnIndex = -1;
			if(m_mapRespTypeIDToColumnIndex.Lookup(nRespTypeID, iColumnIndex) && iColumnIndex != -1) {
				CString strRespTypeName = "";
				if(m_mapRespTypeIDToRespName.Lookup(nRespTypeID, strRespTypeName) && !strRespTypeName.IsEmpty()) {
					mapColumnIndexesToTitles.SetAt(iColumnIndex, strRespTypeName);
				}
				else {
					//should not be possible
					ASSERT(FALSE);
					mapColumnIndexesToTitles.SetAt(iColumnIndex, "Other Resp");
				}
			}
			else {
				//should not be possible
				ASSERT(FALSE);
			}
		}

		// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Show
		BOOL bShowInsuranceNamesAsColumns = GetRemotePropertyInt("ShowInsuranceNamesAsColumns",1,0,"<None>",TRUE) == 1;

		//re-initialize the guarantor IDs
		m_GuarantorID1 = -1;
		m_GuarantorID2 = -1;
		m_bOtherInsExists = FALSE;

		// (j.jones 2010-06-18 11:27) - PLID 39150 - clear the map for other guarantor IDs
		m_mapRespTypeIDsToGuarantorIDs.RemoveAll();

		//DRT 10/21/2008 - PLID 31774 - Parameterized
		tmpRS = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuredPartyT.PersonID, InsuredPartyT.RespTypeID, InsuranceCoT.Name "
			"FROM InsuredPartyT INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
			"WHERE InsuredPartyT.PatientID = {INT}", m_iCurPatient);
		while(!tmpRS->eof) {

			long nRespTypeID = AdoFldLong(tmpRS, "RespTypeID", -1);

			switch(nRespTypeID) {
				case 1: //primary
					m_GuarantorID1 = AdoFldLong(tmpRS, "PersonID", -1);
					strPrimaryInsuranceName = AdoFldString(tmpRS, "Name", "Primary");
					break;
				case 2: //secondary
					m_GuarantorID2 = AdoFldLong(tmpRS, "PersonID", -1);
					strSecondaryInsuranceName = AdoFldString(tmpRS, "Name", "Secondary");
					break;
				default: { //other

					short iColumnIndex = -1;
					if(m_mapRespTypeIDToColumnIndex.Lookup(nRespTypeID, iColumnIndex) && iColumnIndex != -1) {
						//this is a displayed column, track the ID and the name
						m_mapRespTypeIDsToGuarantorIDs.SetAt(nRespTypeID, AdoFldLong(tmpRS, "PersonID", -1));
						//don't update the map if we don't need the company names
						if(bShowInsuranceNamesAsColumns) {
							mapColumnIndexesToTitles.SetAt(iColumnIndex, AdoFldString(tmpRS, "Name", "Other Resp"));
						}
					}
					else {
						//not a displayed column
						m_bOtherInsExists = TRUE;
						nCountOtherInsurances++;
						//will always fill the first time, will revert to "Other Resp" any other time
						if(nCountOtherInsurances == 1) {
							strOtherInsuranceName = AdoFldString(tmpRS, "Name", "Other Resp");
						}
						else {
							strOtherInsuranceName = "Other Resp";
						}						
					}
					break;
						 }
			}

			tmpRS->MoveNext();
		}
		tmpRS->Close();

		if(bShowInsuranceNamesAsColumns) {
			//put the primary and secondary company names as the column titles
			if((LPCTSTR)m_List->GetColumn(m_nPrimInsColID)->GetColumnTitle() != (LPCTSTR)strPrimaryInsuranceName)
				m_List->GetColumn(m_nPrimInsColID)->PutColumnTitle(_bstr_t(strPrimaryInsuranceName));
			if((LPCTSTR)m_List->GetColumn(m_nSecInsColID)->GetColumnTitle() != (LPCTSTR)strSecondaryInsuranceName)
				m_List->GetColumn(m_nSecInsColID)->PutColumnTitle(_bstr_t(strSecondaryInsuranceName));
			if((LPCTSTR)m_List->GetColumn(m_nOtherInsuranceColID)->GetColumnTitle() != (LPCTSTR)strOtherInsuranceName)
				m_List->GetColumn(m_nOtherInsuranceColID)->PutColumnTitle(_bstr_t(strOtherInsuranceName));
		}
		else {
			//else put the words "primary", "secondary", and "other resp" as the column titles
			if((LPCTSTR)m_List->GetColumn(m_nPrimInsColID)->GetColumnTitle() != (LPCTSTR)"Primary")
				m_List->GetColumn(m_nPrimInsColID)->PutColumnTitle("Primary");
			if((LPCTSTR)m_List->GetColumn(m_nSecInsColID)->GetColumnTitle() != (LPCTSTR)"Secondary")
				m_List->GetColumn(m_nSecInsColID)->PutColumnTitle("Secondary");
			if((LPCTSTR)m_List->GetColumn(m_nOtherInsuranceColID)->GetColumnTitle() != (LPCTSTR)"Other Resp")
				m_List->GetColumn(m_nOtherInsuranceColID)->PutColumnTitle("Other Resp");
		}

		// (j.jones 2010-06-18 11:50) - PLID 39150 - set the dynamic column names, it will be the
		// company name or the resp name already, don't need to check the preference
		for(int i=0; i<m_aryDynamicRespTypeIDs.GetSize(); i++) {
			long nRespTypeID = (long)m_aryDynamicRespTypeIDs.GetAt(i);

			short iColumnIndex = -1;
			if(m_mapRespTypeIDToColumnIndex.Lookup(nRespTypeID, iColumnIndex) && iColumnIndex != -1) {
				CString strColumnName = "";
				if(mapColumnIndexesToTitles.Lookup(iColumnIndex, strColumnName) && !strColumnName.IsEmpty()) {
					m_List->GetColumn(iColumnIndex)->PutColumnTitle((LPCTSTR)strColumnName);
				}
				else {
					//should not be possible
					ASSERT(FALSE);
					m_List->GetColumn(iColumnIndex)->PutColumnTitle("Other Resp");
				}
			}
			else {
				//should not be possible
				ASSERT(FALSE);
			}
		}

		// (j.jones 2010-09-27 16:32) - PLID 40123 - now this function must be called during every reload
		DisplayInsuranceAndRefundColumns();

		BOOL bGrayOutColumns = GetRemotePropertyInt("GrayOutInsuranceColumns",0,0,"<None>",TRUE);

		// (j.jones 2010-06-07 15:29) - PLID 39042 - dropped the "brighten row"
		// concept since datalist 2s allow setting the selection color
		if(m_GuarantorID1 == -1 && bGrayOutColumns) {
			COLORREF clr = 0x00999999;
			IColumnSettingsPtr pCol = m_List->GetColumn(m_nPrimInsColID);
			pCol->PutBackColor(clr);			
			pCol->PutBackColorSel(LightenSelColor(clr));
			pCol->PutForeColorSel(RGB(0,0,0));
		}
		else {
			COLORREF clr = 0x00B0B0EE;
			IColumnSettingsPtr pCol = m_List->GetColumn(m_nPrimInsColID);
			pCol->PutBackColor(clr);			
			pCol->PutBackColorSel(LightenSelColor(clr));
			pCol->PutForeColorSel(RGB(0,0,0));
		}

		if(m_GuarantorID2 == -1 && bGrayOutColumns) {
			COLORREF clr = 0x00999999;
			IColumnSettingsPtr pCol = m_List->GetColumn(m_nSecInsColID);
			pCol->PutBackColor(clr);			
			pCol->PutBackColorSel(LightenSelColor(clr));
			pCol->PutForeColorSel(RGB(0,0,0));
		}
		else {
			COLORREF clr = 0x00B0B0EE;
			IColumnSettingsPtr pCol = m_List->GetColumn(m_nSecInsColID);
			pCol->PutBackColor(clr);			
			pCol->PutBackColorSel(LightenSelColor(clr));
			pCol->PutForeColorSel(RGB(0,0,0));
		}

		if(!m_bOtherInsExists && bGrayOutColumns) {
			COLORREF clr = 0x00999999;
			IColumnSettingsPtr pCol = m_List->GetColumn(m_nOtherInsuranceColID);
			pCol->PutBackColor(clr);			
			pCol->PutBackColorSel(LightenSelColor(clr));
			pCol->PutForeColorSel(RGB(0,0,0));
		}
		else {
			COLORREF clr = 0x00B0B0EE;
			IColumnSettingsPtr pCol = m_List->GetColumn(m_nOtherInsuranceColID);
			pCol->PutBackColor(clr);			
			pCol->PutBackColorSel(LightenSelColor(clr));
			pCol->PutForeColorSel(RGB(0,0,0));
		}

		// (j.jones 2010-06-18 12:09) - PLID 39150 - colorize the dynamic insurance columns
		for(int d=0;d<m_aryDynamicRespTypeIDs.GetSize();d++) {
			long nRespTypeID = (long)m_aryDynamicRespTypeIDs.GetAt(d);

			short iColumnIndex = -1;
			if(m_mapRespTypeIDToColumnIndex.Lookup(nRespTypeID, iColumnIndex) && iColumnIndex != -1) {

				//it is a dynamic insurance column, does this patient have a guarantor?
				long nInsuredPartyID = -1;

				COLORREF clr = 0x00B0B0EE;

				//if we aren't graying out columns, always use the insurance color
				if(!bGrayOutColumns || (m_mapRespTypeIDsToGuarantorIDs.Lookup(nRespTypeID, nInsuredPartyID) && nInsuredPartyID != -1)) {
					//insurance color
					clr = (0x00B0B0EE);
				}
				else {
					//gray
					clr = (0x00999999);
				}

				IColumnSettingsPtr pCol = m_List->GetColumn(iColumnIndex);
				pCol->PutBackColor(clr);
				pCol->PutBackColorSel(LightenSelColor(clr));
				pCol->PutForeColorSel(RGB(0,0,0));
			}
			else {
				//shouldn't be possible, but don't assert, just ignore it here
			}
		}

		CArray<long, long> aiBillsToExpand; // a.walling

		// (j.jones 2010-06-14 09:07) - PLID 39138 - get the preference info. for the diag codes
		BOOL bIncludeBillDiagsInDesc = GetRemotePropertyInt("ShowBillDiagCodesInBillingTabBillDesc", 0, 0, "<None>", true) == 1;

		// (j.jones 2010-06-14 09:57) - PLID 39120 - added ability to show the last sent date in the bill description
		// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
		BOOL bIncludeLastSentDate = GetRemotePropertyInt("ShowBillLastSentDateInBillingTabBillDesc", 1, 0, "<None>", true) == 1;

		////////////////////////////////////////////////////////////
		// If showing bills as normal, do this stuff
		// (j.jones 2009-12-29 11:01) - PLID 27237 - replaced IsRecordsetEmpty with a parameterized recordset
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM BillsT WHERE PatientID = {INT} AND BillsT.Deleted = 0", m_iCurPatient);
		if(!rs->eof) {

			// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
			//LogDetail("Executing AddCollapsedBillsToSqlBatch where PatientID = %d", m_iCurPatient);

			// (j.jones 2009-12-30 09:33) - PLID 36729 - this code has been converted
			// to build a batch of recordsets, but for this case it will only return one

			CString strSqlBatch;
			CNxParamSqlArray aryParams;

			AddCollapsedBillsToSqlBatch(strSqlBatch, aryParams, g_cvarNull, m_iCurPatient);

			// (j.jones 2015-03-18 09:18) - PLID 64927 - use the snapshot connection
			_RecordsetPtr rs = CreateParamRecordsetBatch(GetRemoteDataSnapshot(), strSqlBatch, aryParams);
			int i = 0;
			FinTabItem *pNew;
			while(!rs->eof) {
				pNew = new FinTabItem;
				pNew->LineID = (long)GetNewNum("T");
				pNew->LineType = rs->Fields->Item["LineType"]->Value;
				pNew->BillID = (long)rs->Fields->Item["ID"]->Value;
				pNew->ExpandChar = rs->Fields->Item["ExpandChar"]->Value;
				pNew->Date = rs->Fields->Item["Date"]->Value;
				pNew->Location = rs->Fields->Item["LocName"]->Value;
				pNew->NoteIcon = rs->Fields->Item["NoteIcon"]->Value;
				pNew->HasNotes = (long)rs->Fields->Item["HasNotes"]->Value;
				pNew->OnHold = (long)rs->Fields->Item["OnHold"]->Value; // (a.wilson 2014-07-24 12:52) - PLID 63015
				// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
				pNew->IsVoided = VarLong(rs->Fields->Item["IsVoided"]->Value, 0);
				pNew->IsCorrected = VarLong(rs->Fields->Item["IsCorrected"]->Value, 0);
				pNew->Description = rs->Fields->Item["Description"]->Value;
				pNew->ChargeAmount = rs->Fields->Item["ChargeAmount"]->Value;
				// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field
				pNew->TotalPayAmount = rs->Fields->Item["TotalPayAmount"]->Value;
				pNew->PayAmount = rs->Fields->Item["PayAmount"]->Value;
				pNew->Adjustment = rs->Fields->Item["Adjustment"]->Value;
				pNew->BalanceAmount = rs->Fields->Item["Balance"]->Value;
				pNew->PatResp = rs->Fields->Item["PatResp"]->Value;
				pNew->InsResp1 = rs->Fields->Item["Ins1Resp"]->Value;
				pNew->InsResp2 = rs->Fields->Item["Ins2Resp"]->Value;

				// (j.jones 2010-06-18 10:04) - PLID 39150 - fill the dynamic columns
				for(int d=0;d<m_aryDynamicRespTypeIDs.GetSize();d++) {
					long nRespTypeID = (long)m_aryDynamicRespTypeIDs.GetAt(d);

					//these columns should exist in our query
					CString strField;
					strField.Format("Ins%liResp", nRespTypeID);

					DynInsResp *dir = new DynInsResp;
					dir->nRespTypeID = nRespTypeID;
					dir->varResp = rs->Fields->Item[(LPCTSTR)strField]->Value;
					pNew->arypDynInsResps.Add(dir);
				}

				pNew->InsRespOther = rs->Fields->Item["InsRespOther"]->Value;
				pNew->Refund = rs->Fields->Item["Refund"]->Value;
				pNew->Discount = rs->Fields->Item["DiscountTotal"]->Value; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get the discount total
				pNew->HasPercentOff = (long)rs->Fields->Item["HasPercentOff"]->Value; // (a.wetta 2007-04-24 10:55) - PLID 25170 - Get has percent off discount
				// (j.jones 2008-09-17 10:00) - PLID 19623 - supported new columns
				pNew->Provider = rs->Fields->Item["Provider"]->Value;
				pNew->POSName = rs->Fields->Item["POSName"]->Value;
				pNew->POSCode = rs->Fields->Item["POSCode"]->Value;
				pNew->InputDate = rs->Fields->Item["InputDate"]->Value;
				pNew->CreatedBy = rs->Fields->Item["CreatedBy"]->Value;
				pNew->FirstChargeDate = rs->Fields->Item["FirstChargeDate"]->Value;

				// (j.jones 2010-05-26 16:50) - PLID 28184 - added diagnosis code fields
				pNew->DiagCode1 = rs->Fields->Item["DiagCode1"]->Value;
				pNew->DiagCode1WithName = rs->Fields->Item["DiagCode1WithName"]->Value;
				pNew->DiagCodeList = rs->Fields->Item["BillDiagCodeList"]->Value;

				// (j.jones 2010-09-01 15:39) - PLID 40331 - added allowable
				// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
				// and added ChargeAllowable
				pNew->FeeSchedAllowable = g_cvarNull;
				pNew->ChargeAllowable = g_cvarNull;

				// (j.jones 2011-12-20 11:30) - PLID 47119 - added deductible and coinsurance
				pNew->ChargeDeductible = g_cvarNull;
				pNew->ChargeCoinsurance = g_cvarNull;

				// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number, always filled if
				// a value exists, even if the invoice number feature is turned off
				pNew->InvoiceNumber = rs->Fields->Item["InvoiceID"]->Value;

				// (j.jones 2010-06-14 09:06) - PLID 39138 - if the preference asks to put the diag codes in the bill desc, do so now 
				if(bIncludeBillDiagsInDesc) {
					CString strDescription = VarString(pNew->Description, "");
					CString strDiags = AdoFldString(rs, "BillDiagCodeList", "");
					if(!strDescription.IsEmpty() && !strDiags.IsEmpty()) {
						strDescription += ", ";
					}
					strDescription += strDiags;

					pNew->Description = (LPCTSTR)strDescription;
				}

				// (j.jones 2010-06-14 09:57) - PLID 39120 - added ability to show the last sent date in the bill description
				if(bIncludeLastSentDate) {
					CString strDescription = VarString(pNew->Description, "");
					COleDateTime dtInvalid;
					dtInvalid.SetStatus(COleDateTime::invalid);
					COleDateTime dt = AdoFldDateTime(rs, "LastSentDate", dtInvalid);
					if(dt.GetStatus() != COleDateTime::invalid) {

						if(!strDescription.IsEmpty()) {
							strDescription += ", ";
						}
						strDescription += "Last Sent: ";
						strDescription += FormatDateTimeForInterface(dt, NULL, dtoDate);

						pNew->Description = (LPCTSTR)strDescription;
					}
				}

				// a.walling PL 16493 redraw these bills later

				if (InExpandedList(pNew->BillID)) {
					aiBillsToExpand.Add(pNew->BillID);
				}

				g_aryFinancialTabInfoT.Add(pNew);
				rs->MoveNext();
				i++;
			}
			rs->Close();



			for (int iter = 0; iter < aiBillsToExpand.GetSize(); iter++) {
				RedrawBill(aiBillsToExpand[iter]);
			}

			// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
			//LogDetail("%d bills added", i);

		}
		else {
			// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
			//LogDetail("Not Executing AddBillsQ for PatientID = %d. No bills for this patient.", m_iCurPatient);
		}
		rs->Close();			

		RedrawAllPayments();

		RefreshList();

		CalculateTotals();

		EnableFinancialScreen();
		return;
	}
	NxCatchAll("Error in FinancialDlg::FillTabInfo");
}

// (j.jones 2010-06-07 15:32) - PLID 39042 - deprecated
//void CFinancialDlg::ColorList()

/********************************************************
*	ShowBillApplies() will create a dialog showing all
*	the applies directed to a single bill
********************************************************/
// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
void CFinancialDlg::ShowBillApplies(NXDATALIST2Lib::IRowSettingsPtr pRow, int iComboMode)
{
	if(pRow == NULL) {
		return;
	}

	GetMainFrame()->DisableHotKeys();
	CApplyListDlg dlgApply(this);
	dlgApply.m_iDestID = VarLong(pRow->GetValue(m_nBillIDColID));
	dlgApply.m_strClickedType = "Bill";
	dlgApply.DoModal();

	// Redraw everything without doing a mass refresh
	// a.walling 4/14/06
	// ClearExpandLists(); this is done in QuickRefresh with SaveExpansionInfo
	QuickRefresh();
	GetMainFrame()->EnableHotKeys();
}

/********************************************************
*	ShowChargeApplies() will create a dialog showing all
*	the applies directed to a single charge
********************************************************/
// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
void CFinancialDlg::ShowChargeApplies(NXDATALIST2Lib::IRowSettingsPtr pRow, int iComboMode)
{
	if(pRow == NULL) {
		return;
	}

	GetMainFrame()->DisableHotKeys();
	CApplyListDlg dlgApply(this);
	dlgApply.m_iDestID = VarLong(pRow->GetValue(btcChargeID));
	dlgApply.m_strClickedType = "Charge";
	dlgApply.DoModal();

	// Redraw everything without doing a mass refresh
	// a.walling 4/14/06
	// ClearExpandLists(); this is done in QuickRefresh with SaveExpansionInfo
	QuickRefresh();
	GetMainFrame()->EnableHotKeys();
}

/********************************************************
*	Pay_Adj_Ref_Click() is called when the user clicks on
*	a payment, adjustment, or refund line item
********************************************************/
// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
void CFinancialDlg::Pay_Adj_Ref_Click(NXDATALIST2Lib::IRowSettingsPtr pRow, long iColumn)
{
	if(pRow == NULL) {
		return;
	}

	// (j.jones 2010-06-08 09:15) - PLID 39042 - I discovered that these initial permission checks are flawed,
	// previously they would check Create permissions if the column's lVal was zero. Well, it's a currency,
	// not an lVal! But this also means that clicking on a legitimate $0.00 payment fires the wrong permission.
	// On charge & bill lines, we don't know which is true, as clicking always tries to add new. For now
	// we are maintaining the old logic, just with better code, as it hasn't really caused any problems yet.

	BOOL Is_New = FALSE;
	if(iColumn == m_nAdjustmentColID) {
		if(pRow->GetValue(m_nAdjustmentColID).vt != VT_CY || VarCurrency(pRow->GetValue(m_nAdjustmentColID)) == COleCurrency(0,0)) {
			Is_New = TRUE;
			if(!CheckCurrentUserPermissions(bioAdjustment,sptCreate)) return;
		}
		else {
			if (!CheckCurrentUserPermissions(bioAdjustment,sptRead))
				return;
		}
	}
	if(iColumn == m_nPaymentColID) {
		if(pRow->GetValue(m_nPaymentColID).vt != VT_CY || VarCurrency(pRow->GetValue(m_nPaymentColID)) == COleCurrency(0,0)) {
			Is_New = TRUE;
			if(!CheckCurrentUserPermissions(bioPayment,sptCreate)) return;
		}
		else {
			if (!CheckCurrentUserPermissions(bioPayment,sptRead))
				return;
		}
	}
	if(iColumn == m_nRefundColID) {
		if(pRow->GetValue(m_nRefundColID).vt != VT_CY || VarCurrency(pRow->GetValue(m_nRefundColID)) == COleCurrency(0,0)
			|| VarString(pRow->GetValue(btcLineType)) == "Applied"
			|| VarString(pRow->GetValue(btcLineType)) == "InsuranceApplied") {
				Is_New = TRUE;
				if(!CheckCurrentUserPermissions(bioRefund,sptCreate)) return;
		}
		else {
			if (!CheckCurrentUserPermissions(bioRefund,sptRead))
				return;
		}
	}
	// (j.jones 2007-04-19 11:10) - PLID 25721 - In most cases CanEdit is not needed, so first
	// silently see if they have permission, and if they do, then move ahead normally. But if
	// they don't, or they need a password, then check CanEdit prior to the permission check that
	// would stop or password-prompt the user.
	// (a.walling 2008-06-23 14:21) - PLID 30472 - This can cause an exception since there are situations where the PayID is EMPTY (but the state is valid)
	// (c.haag 2009-03-10 10:36) - PLID 32433 - We now use CanChangeHistoricFinancial to validate editing
	// (c.haag 2009-06-02 17:33) - PLID 34393 - We were using CanChangeHistoricFinancial the wrong way. Now we
	// use it the correct way, and we cleaned up the conditional syntax so it's easier to understand. The old code
	// read:
	//
	//if(Is_New || (
	//	(iColumn == m_nPaymentColID && ((GetCurrentUserPermissions(bioPayment) & sptWrite) || (m_List->GetValue(iLineID,btcPayID).vt == VT_I4 && CanEdit("Payment", VarLong(m_List->GetValue(iLineID,btcPayID),-1))) || CheckCurrentUserPermissions(bioPayment,sptWrite))) ||
	//	(iColumn == m_nAdjustmentColID && ((GetCurrentUserPermissions(bioAdjustment) & sptWrite) || (m_List->GetValue(iLineID,btcPayID).vt == VT_I4 && CanEdit("Adjustment", VarLong(m_List->GetValue(iLineID,btcPayID),-1))) || CheckCurrentUserPermissions(bioAdjustment,sptWrite))) ||
	//	(iColumn == m_nRefundColID && ((GetCurrentUserPermissions(bioRefund) & sptWrite) || (m_List->GetValue(iLineID,btcPayID).vt == VT_I4 && CanEdit("Refund", VarLong(m_List->GetValue(iLineID,btcPayID),-1))) || CheckCurrentUserPermissions(bioRefund,sptWrite)))
	//	)) {
	//
	// When you dissect that statement, it comes out to:
	//
	// if (Is_New) { success }													// If the pay/adj/ref dollar amount for the line item is zero { success }
	// else if (iColumn == m_nPaymentColID) {										// Else if user clicked on payment column
	//	if (GetCurrentUserPermissions(bioPayment) & sptWrite) { success }		//		If user has payment write permissions (silent check) { success }
	//	else if (btcPayID is valid and (CanEdit())) { success }					//		Else if the line item is a payment, and the user can edit that payment { success }
	//	else if (CheckCurrentUserPermissions()) { success }						//		Else if user has payment write permissions (non-silent check) { success }
	// }
	// else if (iColumn == m_nAdjustmentColID) {										// Else if user clicked on adjustment column
	//	.. repeat conditionals above for adjustments							// ...
	// }
	// else if (iColumn == m_nRefundColID) {											// Else if user clicked on refund column
	//	.. repeat conditionals above for refunds								// ...
	// }
	//
	// Or, in a more simplified form:
	//		"If the user has payment write permissions, or they can otherwise edit the specific payment; then success."
	//
	// This may seem weird at first, but it was all by intent. GetCurrentUserPermissions is called first because it's silent.
	// CanEdit is called next because a user who does not have payment write permissions can still edit a payment if they are
	// the one who entered it on the same day that they want to edit it. If both fail, CheckCurrentUserPermissions is called because
	// it's not silent; it will give the user a chance to enter a password if they need one. If they fail, the user gets a "You do
	// not have permissions to do this" message.
	//	
	// In the new implementation, all we really needed to do was replace both CanEdit() and CheckCurrentUserPermissions() with 
	// CanChangeHistoricFinancial() because it's designed to do both (and CanChangeHistoricFinancial() is not silent). However, 
	// to avoid further confusion, the logic has been broken out.
	//
	BOOL bShowPaymentDlg = FALSE;
	long nPayID;
	if (Is_New) {
		nPayID = -1;
	} else if (pRow->GetValue(btcPayID).vt != VT_I4) {
		nPayID = -1;
	} else {
		nPayID = VarLong(pRow->GetValue(btcPayID));
	}

	if (Is_New) {
		// The user clicked on a pay/adj/ref column that has a zero dollar amount. Let the user
		// open the payment window because they're about to enter a new payment.
		bShowPaymentDlg = TRUE;
	}
	else {
		// We need to check user permissions. The permissions we check depend on what kind of item the user
		// clicked on.
		EBuiltInObjectIDs bio;
		CString strName;
		BOOL bTest = TRUE;
		if (iColumn == m_nPaymentColID) { bio = bioPayment; strName = "Payment"; }
		else if (iColumn == m_nAdjustmentColID) { bio = bioAdjustment; strName = "Adjustment"; }
		else if (iColumn == m_nRefundColID) { bio = bioRefund; strName = "Refund"; }
		else { bTest = FALSE; } // Unsupported type. Don't show the payment window.

		if (bTest) {
			// The user clicked on a pay/adj/ref column. Let them invoke the payment dialog if one of the following is true:
			// - The user has pay/adj/ref write permissions
			// - There is a valid pay/adj/ref ID, and the user has permission to edit the pay/adj/ref (it could be because
			// they don't have write permissions but they can edit items they created on the same day, for instance)
			if (GetCurrentUserPermissions(bio) & sptWrite) {
				bShowPaymentDlg = TRUE; // The user has pay/adj/ref write permissions
			}
			else if (-1 != nPayID) {
				// If we get here, the user doesn't have write-w/out-password permissions, but the line item is payment-like.
				// This function is how we officially check to see if a user can edit a payment-like line item with a specific
				// ID. It is not silent, either. If it fails, just don't show the dialog.
				if (CanChangeHistoricFinancial(strName, nPayID, bio, sptWrite)) {
					bShowPaymentDlg = TRUE; // There is a valid payment ID, and the user has permission to edit the payment
				}
			} else {
				// If we get here, it means the user doesn't have write-w/out-password permissions, and the line item is
				// not payment-like. So, repeat the permission test; but this time, give them the option to enter a password,
				// and tell them if they failed.
				if (CheckCurrentUserPermissions(bio,sptWrite)) {
					bShowPaymentDlg = TRUE;
				}
			}
		} // if (bTest) {
		else {
			// Unsupported type. Don't show the payment window.
		}
	}

	// (c.haag 2009-06-02 17:40) - PLID 34393 - Use bShowPaymentDlg now
	if (bShowPaymentDlg) {
		CPaymentDlg dlg(this);
		dlg.m_PatientID = m_iCurPatient;
		COleVariant var = pRow->GetValue(btcLineType);
		CString str, strLineType = CString(var.bstrVal);

		try {

			BOOL bApplyToPayment = TRUE;

			////////////////////////////////////////////////////////////////
			// Find out if it's a standalone payment (one that is not
			// represented as an apply to another line item
			if (strLineType == "Payment" || strLineType == "Adjustment" || strLineType == "PrePayment" || strLineType == "Refund") {

				////////////////////////////////////////////////////////////////
				// If this is a standalone payment and the user clicked on the
				// adjustment column, allow the user to add an adjustment to
				// this payment
				if ((strLineType == "Payment" && iColumn != m_nPaymentColID) ||
					(strLineType == "Adjustment" && iColumn == m_nRefundColID) ||
					(strLineType == "Refund" && iColumn == m_nAdjustmentColID)) {

						////////////////////////////////////////////////////////////////
						// Default the adjustment so that it will cancel out
						// the payment when applied
						var = pRow->GetValue(m_nBalanceColID); // var=BalanceAmount
						dlg.m_cyFinalAmount = var.cyVal;
						dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;
						dlg.m_cyFinalAmount *= -1;					

						var = pRow->GetValue(btcPayID); // var=PaymentID

						// (j.jones 2011-09-13 15:36) - PLID 44887 - disallow on original/void items
						LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(VarLong(var));
						if(licsStatus == licsOriginal) {
							AfxMessageBox("This line item has been corrected, and can no longer be modified.");
							return;
						}
						else if(licsStatus == licsVoid) {
							AfxMessageBox("This line item is a void line item from an existing correction, and cannot be modified.");
							return;
						}

						// (j.jones 2009-12-28 17:04) - PLID 27237 - parameterized & combined recordsets
						// (a.walling 2010-03-15 12:23) - PLID 37751 - Include KeyIndex
						_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PaymentsT.InsuredPartyID, LineItemT.LocationID, "
							"PaymentsT.PayMethod, PaymentPlansT.CCNumber, PaymentPlansT.SecurePAN, PaymentPlansT.KeyIndex, "
							"PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.CreditCardID "
							"FROM PaymentsT "
							"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
							"LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
							"WHERE PaymentsT.ID = {INT}", VarLong(var));
						if(!rs->eof) {
							dlg.m_iDefaultInsuranceCo = AdoFldLong(rs, "InsuredPartyID",-1);
							dlg.m_DefLocationID = AdoFldLong(rs, "LocationID",-1);

							if (iColumn == m_nAdjustmentColID) {
								dlg.m_iDefaultPaymentType = 1;	// 1 = Adjustment
								dlg.m_bAdjustmentFollowMaxAmount = TRUE;
							}
							else {
								// (j.gruber 2007-07-30 12:05) - PLID 26704 - we don't need to pop up the payment dialog because we know what they want to apply it to
								// (c.haag 2010-10-12 10:38) - PLID 35723 - We now call SetPayToApplyToID
								// (j.jones 2015-09-30 10:37) - PLID 67172 - renamed this function
								dlg.SetPaymentToApplyTo(VarLong(var));
								dlg.m_ApplyOnOK = TRUE;
								//we are making this false because the paymetn dialg already does it
								bApplyToPayment = FALSE;
								dlg.m_iDefaultPaymentType = 2;	// 2 = Refund

								// (j.gruber 2007-08-02 12:16) - PLID 26916 - fill in the cc information from the payment
								// (a.walling 2007-10-31 09:40) - PLID 27891 - Get the encrypted PAN
								if (AdoFldLong(rs, "PayMethod", -1) == 3) {
									dlg.m_bSetCreditCardInfo = TRUE;
									dlg.m_strNameOnCard = AdoFldString(rs, "CCHoldersName", "");
									//(e.lally 2007-10-30) PLID 27892 - Use the new public function to set the cc number
									// (a.walling 2007-10-31 09:37) - PLID 27891 - This is just the last 4 digits now. Send the unencrypted CCNumber, and 
									// the payment dlg will take care of privatizing it.
									// dlg.SetCreditCardNumber(AdoFldString(rsPayPlan, "CCNumber", ""));
									// (a.walling 2010-03-15 12:23) - PLID 37751 - Use NxCrypto
									//dlg.SetCreditCardNumber(DecryptStringFromVariant(rs->Fields->Item["SecurePAN"]->Value));
									CString strCCNumber;
									NxCryptosaur.DecryptStringFromVariant(rs->Fields->Item["SecurePAN"]->Value, AdoFldLong(rs, "KeyIndex", -1), strCCNumber);
									dlg.SetCreditCardNumber(strCCNumber);
									COleDateTime dtNULL;
									dtNULL.SetDate(1899,12,31);
									COleDateTime dtExpire = AdoFldDateTime(rs, "CCExpDate", dtNULL);
									if (dtExpire != dtNULL) {
										CString strMonth = AsString((long)dtExpire.GetMonth());
										if (strMonth.GetLength() == 1) {
											strMonth = '0' + strMonth;
										}
										dlg.m_strExpDate = strMonth + "/" + AsString((long)dtExpire.GetYear()).Right(2);
									}
									dlg.m_nCreditCardID = AdoFldLong(rs, "CreditCardID", -1);
								}
								else {
									dlg.m_bSetCreditCardInfo = FALSE;
								}
							}
						}
						rs->Close();

						////////////////////////////////////////////////////////////////
						// Open the payment dialog to create the adjustment or refund,
						// then apply it to the payment
						if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {

							// (j.gruber 2007-07-30 15:49) - PLID 26704 - took this out for refunds since I added it to the payment dialog
							if (bApplyToPayment) {
								AutoApplyPayToPay(dlg.m_varPaymentID.lVal, m_iCurPatient, strLineType, VarLong(var));
							}
							FillTabInfo();
							//ResetBrightenedRow();
						}
				}
				else if (!((strLineType == "Adjustment" || strLineType == "Refund") && iColumn == m_nPaymentColID))  {
					////////////////////////////////////////////////////////////////
					// Set up the payment dialog to show this payment
					dlg.m_varPaymentID = pRow->GetValue(btcPayID); // var=PaymentID

					////////////////////////////////////////////////////////////////
					// Bring up the payments dialog to edit an existing payment
					dlg.DoModal(__FUNCTION__, __LINE__);

					// a.walling 4/14/06
					// ClearExpandLists(); this is unnecessary
					FillTabInfo();
					//ResetBrightenedRow();
				}
			}
			else if (strLineType == "Bill") {
				COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;
				m_boAllowUpdate = FALSE;

				////////////////////////////////////////////////////////////////
				// Take the bill total minus the applied payments.
				// The result should be brought up in an auto-apply dialog
				var = pRow->GetValue(m_nBillIDColID);

				// (j.jones 2011-09-13 15:36) - PLID 44887 - disallow on original/void bills
				long nBillID = VarLong(var);

				if(IsVoidedBill(nBillID)) {
					AfxMessageBox("This bill has been corrected, and can no longer be modified.");
					m_boAllowUpdate = TRUE;
					return;
				}
				else if(!DoesBillHaveUncorrectedCharges(nBillID)) {
					AfxMessageBox("All charges in this bill have been corrected, and can no longer be modified.");
					m_boAllowUpdate = TRUE;
					return;
				}

				if (!GetBillTotals(VarLong(var), m_iCurPatient, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
					MsgBox("This bill has been removed from the screen. Practice will now requery this patient's financial information.");
					FullRefresh();
					m_boAllowUpdate = TRUE;
					return;
				}
				dlg.m_cyFinalAmount = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;//BVB - cyInsurance;
				dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;
				dlg.m_varBillID = var;
				dlg.m_ApplyOnOK = TRUE;
				if (iColumn == m_nAdjustmentColID) {
					dlg.m_iDefaultPaymentType = 1;
					dlg.m_cyFinalAmount *= -1;
				}

				if ((iColumn == m_nPaymentColID && cyPayments != COleCurrency(0,0)) ||
					(iColumn == m_nAdjustmentColID && cyAdjustments != COleCurrency(0,0)) ||
					(iColumn == m_nRefundColID && cyRefunds != COleCurrency(0,0))) {

					////////////////////////////////////////////////////////
					// Bring up the apply manager dialog to show applies
					// for this bill
						
					// (j.jones 2015-02-25 09:45) - PLID 64939 - made one modular function to open Apply Manager
					long nBillID = VarLong(pRow->GetValue(m_nBillIDColID));
					OpenApplyManager(nBillID, -1, -1, -1);
				}
				else {
					/////////////////////////////////////////////////////////////
					// Bring up the payments dialog to create the payment and
					// apply it to the bill
					if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
						// (r.gonet 2015-07-06 18:02) - PLID 65327 - If a bill is created, we need to do more than
						// just redraw the payments.
						if (dlg.m_bBillCreated) {
							QuickRefresh();
						} else {
							RedrawBill(VarLong(pRow->GetValue(m_nBillIDColID)));
							RedrawAllPayments();
							CalculateTotals();
						}

						// (j.dinatale 2010-10-05) - PLID 35061 - even though we clicked on a bill line, the user can make an
						//		adjustment in the payment dialog, so we need to check if we need to unbatch the insurance payment
						CheckUnbatchClaim(VarLong(pRow->GetValue(m_nBillIDColID)));
					}
				}
				RefreshList();
				m_boAllowUpdate = TRUE;
			}
			else if (strLineType == "Charge") {
				COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;

				//////////////////////////////////////////////////////////////////////
				// Take the charge total minus the applied payments.
				// The result should be brought up in an auto-apply	dialog		
				m_boAllowUpdate = FALSE;
				var = pRow->GetValue(btcChargeID);

				// (j.jones 2011-09-13 15:36) - PLID 44887 - disallow on original/void charges
				long nChargeID = VarLong(var);

				LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nChargeID);
				if(licsStatus == licsOriginal) {
					AfxMessageBox("This charge has been corrected, and can no longer be modified.");
					m_boAllowUpdate = TRUE;
					return;
				}
				else if(licsStatus == licsVoid) {
					AfxMessageBox("This charge is a void charge from an existing correction, and cannot be modified.");
					m_boAllowUpdate = TRUE;
					return;
				}

				GetChargeTotals(VarLong(var), m_iCurPatient, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance);
				dlg.m_cyFinalAmount = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;//BVB - cyInsurance;
				dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;
				dlg.m_varChargeID = var;
				dlg.m_ApplyOnOK = TRUE;
				if (iColumn == m_nAdjustmentColID) {
					dlg.m_iDefaultPaymentType = 1;
					dlg.m_cyFinalAmount *= -1;
				}

				if ((iColumn == m_nPaymentColID && cyPayments != COleCurrency(0,0)) ||
					(iColumn == m_nAdjustmentColID && cyAdjustments != COleCurrency(0,0)) ||
					(iColumn == m_nRefundColID && cyRefunds != COleCurrency(0,0))) {

					////////////////////////////////////////////////////////
					// Bring up the apply manager dialog to show applies
					// for this charge

					// (j.jones 2015-02-25 09:45) - PLID 64939 - made one modular function to open Apply Manager
					long nBillID = VarLong(pRow->GetValue(m_nBillIDColID));
					long nChargeID = VarLong(pRow->GetValue(btcChargeID));
					OpenApplyManager(nBillID, nChargeID, -1, -1);

					m_boAllowUpdate = TRUE;
				}
				else {
					/////////////////////////////////////////////////////////////
					// Bring up the payments dialog to create the payment and
					// apply it to the charge
					if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
						m_boAllowUpdate = TRUE;
						// (r.gonet 2015-07-06 18:02) - PLID 65327 - If a bill is created, we need to do more than
						// just redraw the payments.
						if (dlg.m_bBillCreated) {
							QuickRefresh();
						} else {
							RedrawBill(VarLong(pRow->GetValue(m_nBillIDColID)));
							RedrawAllPayments();
							CalculateTotals();
						}

						// (j.dinatale 2010-10-05) - PLID 35061 - even though we clicked on a bill line, the user can make an
						//		adjustment in the payment dialog, so we need to check if we need to unbatch the insurance payment
						CheckUnbatchClaim(VarLong(pRow->GetValue(m_nBillIDColID)));
					}
					else
						m_boAllowUpdate = TRUE;
				}
				m_boAllowUpdate = TRUE;
			}
			else if((strLineType == CString("Applied") || strLineType == CString("InsuranceApplied"))
				&& iColumn == m_nRefundColID) {
					long PaymentID = VarLong(pRow->GetValue(btcPayID),-1);
					if(PaymentID != -1) {

						// (j.jones 2011-09-13 15:37) - PLID 44887 - disallow if original/void
						LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(PaymentID);
						if (licsStatus == licsOriginal) {
							AfxMessageBox("This line item has been corrected, and can no longer be modified.");
							m_boAllowUpdate = TRUE;
							return;
						}
						else if (licsStatus == licsVoid) {
							AfxMessageBox("This line item is a void line item from an existing correction, and cannot be modified.");
							m_boAllowUpdate = TRUE;
							return;
						}
						//TES 7/25/2014 - PLID 63049 - Don't allow them to delete chargeback charges
						if (DoesPayHaveChargeback(CSqlFragment("PaymentsT.ID = {INT}", PaymentID))) {
							MessageBox("This payment is associated with at least one Chargeback, and cannot be unapplied in order to refund it. "
								"In order to remove this payment, right-click on it and select 'Undo Chargeback.'");
							m_boAllowUpdate = TRUE;
							return;
						}

						bool bForceRefresh = false;

						// (j.dinatale 2012-05-30 12:23) - PLID 49315 - check if they can modify applies behind a financial close.
						//		If they cant, then yell at them.
						long nApplyID = VarLong(pRow->GetValue(btcApplyID), -1);

						// (j.jones 2015-03-20 13:56) - PLID 65402 - supported voiding and correcting automatically
						if (nApplyID > 0) {
							long nOldApplyID = nApplyID;
							//This will check the RefundingAPayment_VoidAndCorrect preference to determine
							//if the applied payment should be voided first.
							//This function also calls CanChangeHistoricFinancial on the apply in order
							//to check permissions and optionally suppress the close warning.
							if (!RefundingAppliedPayment_CheckVoidAndCorrect(PaymentID, nApplyID, true)) {
								//if this returns false, the refund cannot continue either
								//due to permissions, or a decision to cancel voiding
								m_boAllowUpdate = TRUE;
								return;
							}

							//if the apply ID changed, a correction occurred, we need to know to refresh later
							if (nOldApplyID != nApplyID) {
								bForceRefresh = true;
							}
						}
						else {
							//there should always be an ApplyID
							ASSERT(FALSE);
						}

						// (j.jones 2009-12-28 17:04) - PLID 27237 - parameterized & combined recordsets
						// (j.gruber 2007-08-02 12:16) - PLID 26916 - fill in the cc information from the payment
						// (a.walling 2007-10-31 09:39) - PLID 27891 - Get the encrypted CC number
						// (a.walling 2010-03-15 12:24) - PLID 37751 - Include KeyIndex
						_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PaymentsT.InsuredPartyID, LineItemT.LocationID, "
							"PaymentsT.PayMethod, PaymentPlansT.CCNumber, PaymentPlansT.SecurePAN, PaymentPlansT.KeyIndex, "
							"PaymentPlansT.CCHoldersName, PaymentPlansT.CCExpDate, PaymentPlansT.CCAuthNo, PaymentPlansT.CreditCardID "
							"FROM PaymentsT "
							"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
							"LEFT JOIN PaymentPlansT ON PaymentsT.ID = PaymentPlansT.ID "
							"WHERE PaymentsT.ID = {INT}",PaymentID);
						if(!rs->eof) {

							dlg.m_iDefaultPaymentType = 2; //refund
							var = pRow->GetValue(m_nPaymentColID);
							dlg.m_cyFinalAmount = var.cyVal;
							dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;
							dlg.m_cyMaxAmount *= -1;

							// (j.gruber 2007-07-30 12:05) - PLID 26704 - we don't need to pop up the payment dialog because we know what they want to apply it to
							// (c.haag 2010-10-12 10:38) - PLID 35723 - We now call SetPayToApplyToID
							// (j.jones 2015-09-30 10:37) - PLID 67172 - renamed this function
							dlg.SetPaymentToApplyTo(PaymentID);

							dlg.m_iDefaultInsuranceCo = AdoFldLong(rs, "InsuredPartyID",-1);
							dlg.m_DefLocationID = AdoFldLong(rs, "LocationID",-1);

							if (AdoFldLong(rs, "PayMethod", -1) == 3) {
								dlg.m_bSetCreditCardInfo = TRUE;
								dlg.m_strNameOnCard = AdoFldString(rs, "CCHoldersName", "");
								//(e.lally 2007-10-30) PLID 27892 - Use the new public function to set the cc number
								// (a.walling 2007-10-31 09:37) - PLID 27891 - This is just the last 4 digits now. Send the unencrypted CCNumber, and 
								// the payment dlg will take care of privatizing it.
								// dlg.SetCreditCardNumber(AdoFldString(rsPayPlan, "CCNumber", ""));
								// (a.walling 2010-03-15 12:24) - PLID 37751 - Use NxCrypto
								//dlg.SetCreditCardNumber(DecryptStringFromVariant(rs->Fields->Item["SecurePAN"]->Value));
								CString strCCNumber;
								NxCryptosaur.DecryptStringFromVariant(rs->Fields->Item["SecurePAN"]->Value, AdoFldLong(rs, "KeyIndex", -1), strCCNumber);
								dlg.SetCreditCardNumber(strCCNumber);
								COleDateTime dtNULL;
								dtNULL.SetDate(1899,12,31);
								COleDateTime dtExpire = AdoFldDateTime(rs, "CCExpDate", dtNULL);
								if (dtExpire != dtNULL) {
									CString strMonth = AsString((long)dtExpire.GetMonth());
									if (strMonth.GetLength() == 1) {
										strMonth = '0' + strMonth;
									}
									dlg.m_strExpDate = strMonth + "/" + AsString((long)dtExpire.GetYear()).Right(2);
								}
								dlg.m_nCreditCardID = AdoFldLong(rs, "CreditCardID", -1);
							}
							else {
								dlg.m_bSetCreditCardInfo = FALSE;
							}

							if (dlg.DoModal(__FUNCTION__, __LINE__) != IDCANCEL) {
								long RefundID = dlg.m_varPaymentID.lVal;			
								ApplyRefundToAppliedPayment(PaymentID, nApplyID, RefundID, TRUE);
							}
							// (j.jones 2015-08-25 15:51) - PLID 65402 - if a correction occurred
							// we need to force a refresh despite not making a refund
							else if (bForceRefresh) {
								QuickRefresh();
								CalculateTotals();
							}
						}
						rs->Close();
					}
			}
		}
		NxCatchAll("FinancialDlg::Pay_Adj_Ref_Click");
	}
}

void CFinancialDlg::OnCheckBatch() 
{
	try {

		CString sql;
		CString strAuditOld, strAuditNew;
		EnsureRemoteData();

		//see if they have permissions
		if (!CheckCurrentUserPermissions(bioSuppressStatement, sptWrite)) {
			//uncheck the box 
			((CButton *)GetDlgItem(IDC_CHECK_BATCH))->SetCheck(!(((CButton *)GetDlgItem(IDC_CHECK_BATCH))->GetCheck()));
			return;
		}

		if (((CButton *)GetDlgItem(IDC_CHECK_BATCH))->GetCheck()) {			
			strAuditOld = "Unsuppressed";
			strAuditNew = "Suppressed";
		}
		else {
			strAuditOld = "Suppressed";
			strAuditNew = "Unsuppressed";
		}

		// (j.jones 2009-12-29 14:18) - PLID 27237 - parameterized
		ExecuteParamSql("UPDATE PatientsT SET SuppressStatement = {INT} WHERE PersonID = {INT}", ((CButton *)GetDlgItem(IDC_CHECK_BATCH))->GetCheck() ? 1 : 0, m_iCurPatient);

		long nAuditID = BeginNewAuditEvent();
		AuditEvent(GetActivePatientID(), GetActivePatientName(),nAuditID,aeiSuppressStatement,GetActivePatientID(),strAuditOld,strAuditNew,aepHigh,aetChanged);
	}
	NxCatchAll("OnClickCheckBatch");
}

// (j.jones 2010-06-07 14:54) - PLID 39042 - deprecated
//void CFinancialDlg::BrightenRow(NXDATALIST2Lib::IRowSettingsPtr pRow);
//void CFinancialDlg::DarkenRow(NXDATALIST2Lib::IRowSettingsPtr pRow)
//void CFinancialDlg::ResetBrightenedRow()

////////////////////////////////////////////////////////////////
// Keyboard interface functionality CH
////////////////////////////////////////////////////////////////

//DRT 5/27/2004 - PLID 12610 - Return a value of 0 for handled, 1 for unhandled
int CFinancialDlg::Hotkey(int key)
{
#ifndef KEYBOARD_INTERFACE_ENABLED
	return;
#endif

	//TES 1/6/2010 - PLID 36761 - New accessor for the dropdown state
	if (GetMainFrame()->m_patToolBar.IsDroppedDown())
		return 0;
	if (GetFocus()==GetDlgItem(IDC_EDIT_NOTES))
		return 0;
	if (GetFocus()==GetDlgItem(IDC_STATEMENT_NOTES))
		return 0;

	if (HotKeysEnabled() && IsWindowEnabled())
	{	
		if (GetAsyncKeyState(VK_CONTROL) & 0xE000)//Ctrl is down
		{
			switch (key) {
			case 'B':
				OnNewBill();
				return 0;
			case 'P':
				OnNewPayment();
				return 0;
			}
		}
	}

	// (a.walling 2010-01-27 10:44) - PLID 37083 - Do not process these 'hotkeys' unless our main frame window is the active window
	if (GetActiveWindow()->GetSafeHwnd() == GetMainFrame()->GetSafeHwnd()) {			
		switch (key) {
		case VK_UP:
			MoveHighlight(-1, 0);
			return 0;
		case VK_DOWN:
			MoveHighlight(1, 0);
			return 0;
		case VK_LEFT:
			CollapseOnHighlight();
			return 0;
		case VK_RIGHT:
			ExpandOnHighlight();
			return 0;
		}
	}

	return 1;
}

void CFinancialDlg::MoveHighlight(int nRows, int nPages)
{
	if (m_List->GetRowCount() == 0)
		return;

	// (j.jones 2010-06-07 16:31) - PLID 39042 - supported datalist 2
	IRowSettingsPtr pRow = m_List->GetCurSel();
	if(pRow == NULL) {
		return;
	}

	if(nRows < 0) {
		//attempt to move to previous X rows
		while(nRows < 0 && pRow != NULL) {
			pRow = pRow->GetPreviousRow();
			nRows++;
		}
	}
	else if(nRows > 0) {
		//attempt to move to next X rows
		while(nRows > 0 && pRow != NULL) {
			pRow = pRow->GetNextRow();
			nRows--;
		}
	}

	if(pRow) {
		m_List->PutCurSel(pRow);
	}
}

void CFinancialDlg::ExpandOnHighlight()
{
	// (j.jones 2010-06-07 16:31) - PLID 39042 - supported datalist 2
	IRowSettingsPtr pRow = m_List->GetCurSel();
	if(pRow == NULL) {
		return;
	}

	COleVariant varExpandChar = pRow->GetValue(m_nExpandCharColID);
	COleVariant varExpandSubChar = pRow->GetValue(m_nExpandSubCharColID);

	if (varExpandChar.vt == VT_NULL || varExpandChar.vt == VT_EMPTY)
		return;

	if (m_List->GetRowCount() == 0)
		return;

	if (CString("BITMAP:DOWNARROW") == CString(varExpandChar.bstrVal)) {
		LeftClickList(pRow, m_nExpandCharColID);
		return;
	}
	else if (varExpandSubChar.vt != VT_NULL && varExpandSubChar.vt != VT_EMPTY) {
		if (CString("BITMAP:DOWNARROW") == CString(varExpandSubChar.bstrVal)) {
			LeftClickList(pRow, m_nExpandSubCharColID);
			return;
		}
		else
			return;
	}
	else
		return;

	LeftClickList(pRow, m_nExpandCharColID);
}

void CFinancialDlg::CollapseOnHighlight()
{
	// (j.jones 2010-06-07 16:31) - PLID 39042 - supported datalist 2
	IRowSettingsPtr pRow = m_List->GetCurSel();
	if(pRow == NULL) {
		return;
	}

	COleVariant varExpandChar = pRow->GetValue(m_nExpandCharColID);
	COleVariant varExpandSubChar = pRow->GetValue(m_nExpandSubCharColID);

	if (varExpandChar.vt == VT_NULL || varExpandChar.vt == VT_EMPTY)
		return;

	if (m_List->GetRowCount() == 0)
		return;

	if (varExpandSubChar.vt != VT_NULL && varExpandSubChar.vt != VT_EMPTY && CString("BITMAP:DOWNARROW") != CString(varExpandSubChar.bstrVal)) {
		LeftClickList(pRow, m_nExpandSubCharColID);
	}
	else if (CString("BITMAP:DOWNARROW") != CString(varExpandChar.bstrVal)) {
		LeftClickList(pRow, m_nExpandCharColID);
	}
	else
		return;
}

BOOL CFinancialDlg::PreTranslateMessage(MSG* pMsg) 
{
	// For keyboard interface -- disables arrow keys from manipulating
	// button focus
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam != VK_TAB && pMsg->wParam != VK_RETURN) &&
		GetFocus() != GetDlgItem(IDC_STATEMENT_NOTES) &&
		GetFocus() != GetDlgItem(IDC_EDIT_NOTES))
		return 0;
	if(pMsg->message == NXM_POST_EDIT_BILL) {
		PostEditBill(pMsg->wParam,pMsg->lParam);
		return TRUE;
	}
	if(pMsg->message == NXM_POST_PACKAGE_PAYMENT) {
		PostPackagePayment();
		return TRUE;
	}
	if(pMsg->message == NXM_BILL_PACKAGE) {
		BillPackage(pMsg->wParam,pMsg->lParam);
		return TRUE;
	}
	return CNxDialog::PreTranslateMessage(pMsg);
}

///////////////////////////////////////////////////////////////

void CFinancialDlg::OnShowRefunds() 
{
	if (!IsWindowVisible())
		return;

	// (d.thompson 2008-12-09) - PLID 32370 - Split refunds to its own preference
	m_nShowRefunds = IsDlgButtonChecked(IDC_SHOW_REFUNDS) ? 1 : 0;
	SetRemotePropertyInt("ShowBillingRefunds", m_nShowRefunds, 0, GetCurrentUserName());

	// (j.jones 2008-09-17 16:00) - PLID 19623 - renamed this function into something more accurate
	DisplayInsuranceAndRefundColumns();
}

//TES 3/5/04: An attempt to avoid having 2 notes dlgs pop up on top of each other.
static bool l_bProcessingClick = false;

// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
void CFinancialDlg::LeftClickList(NXDATALIST2Lib::IRowSettingsPtr pRow, long iColumn)
{
	try {

		if(pRow == NULL) {
			return;
		}

		m_List->PutCurSel(pRow);

		CString strLineType = VarString(pRow->GetValue(btcLineType));

		VARIANT v, vLineID, vBillID;

		// (j.jones 2010-06-18 12:25) - PLID 39150 - calculate if this is a dynamic insurance column
		long nDynamicRespTypeID = -1;
		m_mapColumnIndexToRespTypeID.Lookup((short)iColumn, nDynamicRespTypeID);

		/////////////////////////////////////////////////////
		// User clicked in the leftmost column (Expand bill column)
		/////////////////////////////////////////////////////
		if(iColumn == m_nExpandCharColID) {
			// If the cell is empty, do nothing
			v = pRow->GetValue((int)iColumn);
			if (v.vt == VT_NULL || v.vt == VT_EMPTY) return;

			// Make sure an arrow exists
			v = pRow->GetValue(m_nExpandCharColID);
			if (v.vt == VT_EMPTY || v.vt == VT_NULL)
				return;

			// See if we're expanding a bill or a payment/adjustment/refund
			v = pRow->GetValue(btcLineType);
			if (CString(v.bstrVal) == CString("Bill") || CString(v.bstrVal) == CString("Charge")) {
				// Expand the bill
				v = pRow->GetValue(m_nBillIDColID);
				vLineID = pRow->GetValue(btcLineID);
				ExpandBill(v.lVal, vLineID.lVal);
			}
			else {
				// Expand the payment
				v = pRow->GetValue(btcPayID);
				ExpandPayment(v.lVal);
			}

			return;
		}
		/////////////////////////////////////////////////////
		// User clicked in the column
		// next to the leftmost column (Expand charge column)
		/////////////////////////////////////////////////////
		else if(iColumn == m_nExpandSubCharColID) {
			// If the cell is empty, do nothing 
			v = pRow->GetValue((int)iColumn);
			if (v.vt == VT_NULL || v.vt == VT_EMPTY)
				return;

			// Make sure an arrow exists
			v = pRow->GetValue(m_nExpandSubCharColID);
			if (v.vt == VT_EMPTY || v.vt == VT_NULL)
				return;

			v = pRow->GetValue(btcLineType);
			if (CString(v.bstrVal) == CString("AppliedBill") || CString(v.bstrVal) == CString("AppliedCharge")) {
				/////////////////////////////////////////////////////////////////
				// Expand the applied bill (I think this functionality is
				// obselete)
				v = pRow->GetValue(m_nBillIDColID);
				for (int i=0; i < m_aiExpandedSubBranches.GetSize(); i++) {
					if (m_aiExpandedSubBranches[i] == v.lVal) {
						m_aiExpandedSubBranches.RemoveAt(i);
						FillTabInfo();

						// (j.jones 2015-03-17 13:25) - PLID 64931 - clear the search results window
						ClearSearchResults();
						return;
					}
				}
				m_aiExpandedSubBranches.Add(v.lVal);
				FillTabInfo();
				return;
			}	
			else {
				/////////////////////////////////////////////////////////////////
				// Expand the charge
				v = pRow->GetValue(btcChargeID);
				vBillID = pRow->GetValue(m_nBillIDColID);
				vLineID = pRow->GetValue(btcLineID);
				ExpandCharge(v.lVal, vBillID.lVal, vLineID.lVal);
			}
			return;
		}
		/////////////////////////////////////////////////////
		// User clicked in the notes column (the file icon)
		/////////////////////////////////////////////////////
		else if(iColumn == m_nNoteIconColID) {

			BOOL bRefreshIcon = FALSE;
			CString strLineType = "";
			long nItemID = -1;

			// If the cell is empty, do nothing
			v = pRow->GetValue((int)iColumn);
			if (v.vt == VT_NULL || v.vt == VT_EMPTY) return;

			// Make sure a note icon exists
			v = pRow->GetValue(m_nNoteIconColID);
			if (v.vt == VT_EMPTY || v.vt == VT_NULL)
				return;

			// See if we are on a bill or a line item
			v = pRow->GetValue(btcLineType);
			if (CString(v.bstrVal) == CString("Bill")) {					
				v = pRow->GetValue(m_nBillIDColID);

				CNotesDlg dlgNotes(this);
				dlgNotes.m_bIsBillingNote = true;
				// (j.jones 2011-09-19 14:46) - PLID 42135 - added m_bntBillingNoteType
				dlgNotes.m_bntBillingNoteType = bntBill;
				dlgNotes.m_nBillID = VarLong(v,-1);
				CNxModalParentDlg dlg(this, &dlgNotes, CString("Bill Notes"));
				dlg.DoModal();

				// (j.jones 2004-07-29 10:07) - store this data to refresh the note icon
				BOOL bRefreshIcon = TRUE;
				strLineType = "Bill";
				nItemID = VarLong(v,-1);									
			}
			else if(CString(v.bstrVal) == CString("Charge")) {
				v = pRow->GetValue(btcChargeID);

				CNotesDlg dlgNotes(this);
				dlgNotes.m_bIsBillingNote = true;
				// (j.jones 2011-09-19 14:46) - PLID 42135 - added m_bntBillingNoteType
				dlgNotes.m_bntBillingNoteType = bntCharge;
				dlgNotes.m_nLineItemID = VarLong(v,-1);
				CNxModalParentDlg dlg(this, &dlgNotes, CString("Charge Notes"));
				dlg.DoModal();

				// (j.jones 2004-07-29 10:07) - store this data to refresh the note icon
				BOOL bRefreshIcon = TRUE;
				strLineType = "Charge";
				nItemID = VarLong(v,-1);
			}
			else if(CString(v.bstrVal) == CString("Applied") || CString(v.bstrVal) == CString("AppliedPayToPay")) {
				v = pRow->GetValue(btcApplyID);

				long nLineItemID = -1;

				if(v.vt == VT_I4) {
					//for applies to payments
					// (j.jones 2009-12-28 17:04) - PLID 27237 - parameterized
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT SourceID FROM AppliesT WHERE ID = {INT}", VarLong(v, -1));
					if(!rs->eof) {
						nLineItemID = rs->Fields->Item["SourceID"]->Value.lVal;
					}
					rs->Close();
				}
				else {
					v = pRow->GetValue(btcPayID);
					nLineItemID = VarLong(v, -1);
				}

				CNotesDlg dlgNotes(this);
				dlgNotes.m_bIsBillingNote = true;
				// (j.jones 2011-09-19 14:46) - PLID 42135 - added m_bntBillingNoteType
				dlgNotes.m_bntBillingNoteType = bntPayment;
				dlgNotes.m_nLineItemID = nLineItemID;
				CNxModalParentDlg dlg(this, &dlgNotes, CString("Pay/Adj/Ref Notes"));
				dlg.DoModal();

				// (j.jones 2004-07-29 10:07) - store this data to refresh the note icon
				BOOL bRefreshIcon = TRUE;
				strLineType = VarString(pRow->GetValue(btcLineType), "");
				nItemID = nLineItemID;
			}
			else {
				//a pay/adj/ref
				v = pRow->GetValue(btcPayID);

				CNotesDlg dlgNotes(this);
				dlgNotes.m_bIsBillingNote = true;
				// (j.jones 2011-09-19 14:46) - PLID 42135 - added m_bntBillingNoteType
				dlgNotes.m_bntBillingNoteType = bntPayment;
				dlgNotes.m_nLineItemID = VarLong(v,-1);
				CNxModalParentDlg dlg(this, &dlgNotes, CString("Pay/Adj/Ref Notes"));
				dlg.DoModal();

				// (j.jones 2004-07-29 10:07) - store this data to refresh the note icon
				BOOL bRefreshIcon = TRUE;
				strLineType = "Payment";
				nItemID = VarLong(v,-1);
			}

			// (j.jones 2004-07-29 10:04) - now update the note icon status
			for(int i=0;i<g_aryFinancialTabInfoT.GetSize();i++){
				if(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(i))->LineID.vt == VT_I4
					&& ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(i))->LineID.lVal == VarLong(pRow->GetValue(btcLineID))) {

						((FinTabItem*)g_aryFinancialTabInfoT.GetAt(i))->HasNotes = (long)(DoesLineHaveNotes(strLineType,nItemID) ? 1 : 0);

						if(VarLong((((FinTabItem*)g_aryFinancialTabInfoT.GetAt(i))->HasNotes),0) == 1) {							
							pRow->PutValue(m_nNoteIconColID,(long)m_hNotes);
						}
						else {
							pRow->PutValue(m_nNoteIconColID,((FinTabItem*)g_aryFinancialTabInfoT.GetAt(i))->NoteIcon);
						}
						break;					
				}
			}

			return;
		}
		// (j.jones 2010-06-11 09:49) - PLID 39109 - supported clicking the discount icon
		else if(iColumn == m_nDiscountIconColID) {

			// If the cell is empty, do nothing
			v = pRow->GetValue((int)iColumn);
			if (v.vt == VT_NULL || v.vt == VT_EMPTY) {
				return;
			}

			//do we have the discount icon?
			if(VarLong(v, -1) != (long)m_hDiscount) {
				return;
			}

			long nBillID = -1;
			long nChargeID = -1;

			v = pRow->GetValue(btcLineType);
			if(VarString(v, "") == CString("Bill")) {					
				nBillID = VarLong(pRow->GetValue(m_nBillIDColID), -1);							
			}
			else if(VarString(v, "") == CString("Charge")) {
				nBillID = VarLong(pRow->GetValue(m_nBillIDColID), -1);
				nChargeID = VarLong(pRow->GetValue(btcChargeID), -1);
			}

			//we must have a bill ID
			if(nBillID == -1) {
				return;
			}

			//open the dialog for this bill
			CBillDiscountOverviewDlg dlg(this);
			dlg.m_nBillID = nBillID;
			dlg.m_nDefChargeID = nChargeID;
			dlg.DoModal();

			//this dialog is read-only, we don't need to refresh anything when it closes

			return;
		}

		/////////////////////////////////////////////////////
		// User clicked in the charge column
		/////////////////////////////////////////////////////
		else if(iColumn == m_nChargeColID) {
			// If a payment line, open up a bill and auto-apply the payment to the bill
			if (strLineType == "Payment" || strLineType == "PrePayment" ||
				strLineType == "Adjustment" || strLineType == "Refund") {

					_variant_t v = pRow->GetValue(btcPayID);
					long nPaymentID = -1;
					if(v.vt == VT_I4) {
						nPaymentID = VarLong(v);

						// (j.jones 2011-09-13 15:37) - PLID 44887 - disallow if original/void
						if(nPaymentID != -1) {
							LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nPaymentID);
							if(licsStatus == licsOriginal) {
								AfxMessageBox("This line item has been corrected, and can no longer be modified.");
								return;
							}
							else if(licsStatus == licsVoid) {
								AfxMessageBox("This line item is a void line item from an existing correction, and cannot be modified.");
								return;
							}
						}
					}
					else {
						// (j.jones 2008-06-13 17:42) - let me know if you ever hit this assertion
						ASSERT(FALSE);
					}

					/////////////////////////////////////////////////////////////////
					// Open a new bill
					// (a.walling 2009-12-22 17:54) - PLID 7002 - Maintain only one instance of a bill
					if(nPaymentID != -1 && CheckCurrentUserPermissions(bioBill,sptCreate) && !GetMainFrame()->IsBillingModuleOpen(true)) {

						// (j.jones 2008-06-13 17:49) - PLID 29872 - cache this value so we know what payment to apply later
						m_nPostEditBill_ApplyPaymentID = nPaymentID;

						//GetMainFrame()->DisableHotKeys();
						m_BillingDlg->m_pFinancialDlg = this;		//DRT 12/27/2004 - PLID 15083 - See comments in OnNewBill
						m_BillingDlg->m_boAskForNewPayment = FALSE;
						m_boAllowUpdate = FALSE;
						m_BillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 3);
						m_boAllowUpdate = TRUE;
						//GetMainFrame()->EnableHotKeys();
					}

					//the payment is applied in PostEditBill(3)
			}
			else if (strLineType != "Bill" && strLineType != "Charge") return;
			else {
				/////////////////////////////////////////////////////////////////
				// Open an existing bill

				//GetMainFrame()->DisableHotKeys();
				// (j.jones 2007-04-19 11:18) - PLID 25721 - In most cases CanEdit is not needed, so first
				// silently see if they have permission, and if they do, then move ahead normally. But if
				// they don't, or they need a password, then check CanEdit prior to the permission check that
				// would stop or password-prompt the user.
				// (c.haag 2009-03-10 11:06) - PLID 32433 - We now use CanChangeHistoricFinancial
				if (pRow->GetValue(m_nBillIDColID).vt != VT_EMPTY) {

					// (a.walling 2009-12-22 17:54) - PLID 7002 - Maintain only one instance of a bill
					if (CanChangeHistoricFinancial("Bill", VarLong(pRow->GetValue(m_nBillIDColID),-1), bioBill, sptRead) && !GetMainFrame()->IsBillingModuleOpen(true)) {
						//if((GetCurrentUserPermissions(bioBill) & sptRead) || CanEdit("Bill", VarLong(m_List->GetValue(iLineID, m_nBillIDColID),-1)) || CheckCurrentUserPermissions(bioBill,sptRead)) {
						v = pRow->GetValue(m_nBillIDColID);
						m_BillingDlg->m_iBillID = v.lVal;
						m_BillingDlg->m_pFinancialDlg = this;		//DRT 12/27/2004 - PLID 15083 - See comments in OnNewBill
						m_boAllowUpdate = FALSE;
						m_BillingDlg->OpenWithBillID(v.lVal, BillEntryType::Bill, 2);
						m_boAllowUpdate = TRUE;
					}
				}
				//GetMainFrame()->EnableHotKeys();

				//the screen is refreshed in PostEditBill(2)
			}
		}

		/////////////////////////////////////////////////////
		// User clicked in an insurance column
		/////////////////////////////////////////////////////		
		// (j.jones 2010-06-18 12:27) - PLID 39150 - we would have already loaded nDynamicRespTypeID,
		// so if it is not -1, this is a dynamic insurance column
		else if(iColumn == m_nPrimInsColID || iColumn == m_nSecInsColID
			|| iColumn == m_nOtherInsuranceColID || nDynamicRespTypeID != -1) {

				m_boAllowUpdate = FALSE;

				COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;
				COleCurrency cyRemainder;

				int InsType = 1;
				long nOtherInsID = -1;

				if(iColumn == m_nPrimInsColID) {
					InsType = 1;
				}
				else if(iColumn == m_nSecInsColID) {
					InsType = 2;
				}
				else if(nDynamicRespTypeID != -1) {
					// (j.jones 2010-06-18 12:28) - PLID 39150 - support nDynamicRespTypeID
					InsType = nDynamicRespTypeID;
				}
				else if(iColumn == m_nOtherInsuranceColID) {

					long ID;

					if(m_bOtherInsExists) {
						if (strLineType == "Bill")
							ID = VarLong(pRow->GetValue(m_nBillIDColID));
						else if (strLineType == "Charge")
							ID = VarLong(pRow->GetValue(btcChargeID));
						else {
							m_boAllowUpdate = TRUE;
							return;
						}

						CChooseDragRespDlg dlg(this);
						dlg.m_nTargetType = 3;	//we want a responsibility to apply a payment to
						dlg.m_strLineType = strLineType;
						dlg.m_nLineID = ID;

						// (j.jones 2010-06-18 14:32) - PLID 39150 - pass in a comma delimited list of RespTypeIDs to ignore,
						// to be appended to the existing ignored list of IDs 1 and 2
						CString strIDsToIgnore;
						for(int r=0;r<m_aryDynamicRespTypeIDs.GetSize();r++) {
							long nRespTypeID = (long)m_aryDynamicRespTypeIDs.GetAt(r);
							if(!strIDsToIgnore.IsEmpty()) {
								strIDsToIgnore += ",";
							}
							strIDsToIgnore += AsString(nRespTypeID);						
						}
						dlg.m_strRespTypeIDsToIgnore = strIDsToIgnore;

						if(dlg.DoModal() == IDCANCEL) {
							m_boAllowUpdate = TRUE;
							return;
						}

						//we have our resp!
						InsType = dlg.m_nRespTypeID;
						nOtherInsID = dlg.m_nInsPartyID;
					}
					else {
						InsType = -1;
						nOtherInsID = -1;
					}
				}

				long nInsuredPartyID = -1;

				switch (InsType) {
				case 1:
					nInsuredPartyID = m_GuarantorID1;
					break;
				case 2:
					nInsuredPartyID = m_GuarantorID2;
					break;
				default:
					// (j.jones 2010-06-18 12:30) - PLID 39150 - if this is a dynamic column, get the
					// cached guarantor ID from the map
					if(nDynamicRespTypeID != -1) {
						m_mapRespTypeIDsToGuarantorIDs.Lookup(nDynamicRespTypeID, nInsuredPartyID);
					}
					else {
						nInsuredPartyID = nOtherInsID;
					}
					break;
				}

				// (j.jones 2012-10-30 16:43) - PLID 53444 - moved the dialog definition to be where it is actually used
				CPaymentDlg dlg(this);
				dlg.m_PatientID = m_iCurPatient;
				dlg.m_iDefaultInsuranceCo = nInsuredPartyID;

				/////////////////////////////////////////////////////////////////
				// If the user clicked on a bill line, try to auto-apply a new
				// insurance payment to the bill
				if (strLineType == "Bill") {
					v = pRow->GetValue(m_nBillIDColID);

					long nBillID = VarLong(v);

					// (j.jones 2011-09-13 15:37) - PLID 44887 - disallow changing non-editable bills
					if(IsVoidedBill(nBillID)) {
						AfxMessageBox("This bill has been corrected, and can no longer be modified.");
						m_boAllowUpdate = TRUE;
						return;
					}
					else if(!DoesBillHaveUncorrectedCharges(nBillID)) {
						AfxMessageBox("All charges in this bill have been corrected, and can no longer be modified.");
						m_boAllowUpdate = TRUE;
						return;
					}

					// (j.jones 2011-07-22 12:19) - PLID 42231 - GetBillInsuranceTotals now takes in an insured party ID, not a RespTypeID
					if((InsType != -1 && !GetBillInsuranceTotals(v.lVal, m_iCurPatient, nInsuredPartyID, &cyInsurance, &cyPayments, &cyAdjustments, &cyRefunds))
						|| (InsType == -1 && !GetInactiveInsTotals(nInsuredPartyID, v.lVal, -1, m_iCurPatient, cyInsurance, cyPayments))) {
							MsgBox("This bill has been removed from this screen. Practice will now requery this patient's financial information.");
							FullRefresh();
							return;
					}

					// dlg = Payment dialog

					////////////////////////////////////////////////////////
					// Open the payment dialog and create the payment
					dlg.m_cyFinalAmount = cyRemainder = cyInsurance - cyPayments - cyAdjustments - cyRefunds;

					dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;  // Don't force a maximum amount because we may change insurance resp.

					dlg.m_varBillID = v;
					if(CheckCurrentUserPermissions(bioPayment,sptCreate)) {
						if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {

							AutoApplyPayToBill(dlg.m_varPaymentID.lVal,	m_iCurPatient, "Bill", v.lVal); // v = BillID

							//If there is any balance left for the given responsibility, ask to adjust
							//(By this point they will have already paid and shifted)
							// (j.jones 2011-07-22 12:19) - PLID 42231 - GetBillInsuranceTotals now takes in an insured party ID, not a RespTypeID
							if((InsType != -1 && !GetBillInsuranceTotals(v.lVal, m_iCurPatient, nInsuredPartyID, &cyInsurance, &cyPayments, &cyAdjustments, &cyRefunds))
								|| (InsType == -1 && !GetInactiveInsTotals(nInsuredPartyID, v.lVal, -1, m_iCurPatient, cyInsurance, cyPayments))) {
									MsgBox("This bill has been removed from this screen. Practice will now requery this patient's financial information.");
									FullRefresh();
									return;
							}
							cyRemainder = cyInsurance - cyPayments - cyAdjustments - cyRefunds;
							if(cyRemainder > COleCurrency(0,0)) {
								if(IDYES == MessageBox("Would you like to adjust the remaining balance for this insurance responsibility?","Practice",MB_ICONQUESTION|MB_YESNO)) {
									if(CheckCurrentUserPermissions(bioAdjustment,sptCreate)) {

										//Make an adjustment
										CPaymentDlg dlg(this);
										dlg.m_PatientID = m_iCurPatient;

										switch (InsType) {
										case 1: dlg.m_iDefaultInsuranceCo = m_GuarantorID1; break;
										case 2: dlg.m_iDefaultInsuranceCo = m_GuarantorID2; break;
										default:
											dlg.m_iDefaultInsuranceCo = nOtherInsID;
											break;
										}

										dlg.m_cyFinalAmount = cyRemainder;
										dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;
										dlg.m_varBillID = v;
										dlg.m_ApplyOnOK = TRUE;
										dlg.m_iDefaultPaymentType = 1;
										dlg.m_cyFinalAmount *= -1;

										dlg.DoModal(__FUNCTION__, __LINE__);
									}
								}
							}

							CheckUnbatchClaim(v.lVal);

							// (r.gonet 2015-07-06 18:02) - PLID 65327 - If a bill is created, we need to do more than
							// just redraw the payments.
							if (dlg.m_bBillCreated) {
								QuickRefresh();
							} else {
								RedrawBill(v.lVal);
								RedrawAllPayments();
								CalculateTotals();
							}
						}				
					}
				}
				/////////////////////////////////////////////////////////////////
				// If the user clicked on a charge line, try to auto-apply a new
				// insurance payment to the charge
				else if (strLineType == "Charge") {
					v = pRow->GetValue(btcChargeID);

					long nChargeID = VarLong(v);

					// (j.jones 2011-09-13 15:36) - PLID 44887 - disallow on original/void charges
					LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nChargeID);
					if(licsStatus == licsOriginal) {
						AfxMessageBox("This charge has been corrected, and can no longer be modified.");
						m_boAllowUpdate = TRUE;
						return;
					}
					else if(licsStatus == licsVoid) {
						AfxMessageBox("This charge is a void charge from an existing correction, and cannot be modified.");
						m_boAllowUpdate = TRUE;
						return;
					}

					// (j.jones 2009-12-29 12:00) - PLID 27237 - get this from the list, not from data
					long nBillID = VarLong(pRow->GetValue(m_nBillIDColID), -1);
					if (!GetChargeInsuranceTotals(v.lVal, m_iCurPatient, nInsuredPartyID, &cyInsurance, &cyPayments, &cyAdjustments, &cyRefunds)) {
						MsgBox("This bill has been removed from this screen. Practice will now requery this patient's financial information.");
						m_boAllowUpdate = TRUE;
						FullRefresh();
						return;
					}

					// dlg = Payment dialog
					dlg.m_cyFinalAmount = cyInsurance - cyPayments - cyAdjustments - cyRefunds;
					dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;
					dlg.m_varChargeID = v.lVal;
					dlg.m_ApplyOnOK = TRUE;
					if(CheckCurrentUserPermissions(bioPayment,sptCreate)) {
						if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
							//ApplyInsurancePayToBill(dlg.m_varPaymentID.lVal, dlg.m_cyFinalAmount, "Charge", VarLong(var), GetInsuranceIDFromType(InsType));

							//If there is any balance left for the given responsibility, ask to adjust
							//(By this point they will have already paid and shifted)
							if (!GetChargeInsuranceTotals(v.lVal, m_iCurPatient, nInsuredPartyID, &cyInsurance, &cyPayments, &cyAdjustments, &cyRefunds)) {
								MsgBox("This bill has been removed from this screen. Practice will now requery this patient's financial information.");
								m_boAllowUpdate = TRUE;
								FullRefresh();
								return;
							}
							cyRemainder = cyInsurance - cyPayments - cyAdjustments - cyRefunds;
							if(cyRemainder > COleCurrency(0,0)) {
								if(IDYES == MessageBox("Would you like to adjust the remaining balance for this insurance responsibility?","Practice",MB_ICONQUESTION|MB_YESNO)) {
									if(CheckCurrentUserPermissions(bioAdjustment,sptCreate)) {

										//Make an adjustment
										CPaymentDlg dlg(this);
										dlg.m_PatientID = m_iCurPatient;

										switch (InsType) {
										case 1: dlg.m_iDefaultInsuranceCo = m_GuarantorID1; break;
										case 2: dlg.m_iDefaultInsuranceCo = m_GuarantorID2; break;
										default:
											dlg.m_iDefaultInsuranceCo = nOtherInsID;
											break;
										}

										dlg.m_cyFinalAmount = cyRemainder;
										dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;
										dlg.m_varChargeID = v.lVal;
										dlg.m_ApplyOnOK = TRUE;
										dlg.m_iDefaultPaymentType = 1;
										dlg.m_cyFinalAmount *= -1;

										dlg.DoModal(__FUNCTION__, __LINE__);
									}
								}
							}

							CheckUnbatchClaim(nBillID);

							// (r.gonet 2015-07-06 18:02) - PLID 65327 - If a bill is created, we need to do more than
							// just redraw the payments.
							if (dlg.m_bBillCreated) {
								QuickRefresh();
							} else {
								RedrawBill(VarLong(pRow->GetValue(m_nBillIDColID)));
								RedrawAllPayments();
								CalculateTotals();
							}
						}
					}
				}

				m_boAllowUpdate = TRUE;
		}

		/////////////////////////////////////////////////////
		// User clicked in the patient balance column
		/////////////////////////////////////////////////////		
		else if(iColumn == m_nPatientBalanceColID) {
			// Make it have the same effect as the user clicking on
			// the payment column
			iColumn = m_nPaymentColID;
		}

		/////////////////////////////////////////////////////
		// User clicked in a payment or adjustment column
		/////////////////////////////////////////////////////		
		// UserPerm is set in Pay_Adj_Ref_Click
		if (iColumn == m_nPaymentColID || iColumn == m_nAdjustmentColID) {
			Pay_Adj_Ref_Click(pRow, iColumn);
		}
		else if(iColumn == m_nRefundColID) {
			if (strLineType == "Payment" || strLineType == "Adjustment" ||
				strLineType == "Refund" || strLineType == "Applied" || strLineType == "InsuranceApplied") {
					Pay_Adj_Ref_Click(pRow, iColumn);
			}
		}

		// (j.jones 2015-02-27 09:51) - PLID 64943 - if they click the Total Pay Amount column,
		// open ApplyManager
		if (iColumn == m_nTotalPaymentAmountColID) {
			if (pRow) {
				long nPayAdjRefID = VarLong(pRow->GetValue(btcPayID), -1);
				long nApplyID = VarLong(pRow->GetValue(btcApplyID), -1);
				
				if (nPayAdjRefID == -1) {
					//this isn't a payment row, but are we displaying anything?
					if (pRow->GetValue(m_nTotalPaymentAmountColID).vt == VT_CY) {
						//if you hit this, find out why we're showing any data in this row
						//if it is not a pay/adj/ref row
						ASSERT(FALSE);

						//continue on, Apply Manager will just open normally and not default
						//to any particular line item
					}
					else {
						//this isn't a payment row, and the field has no value
						return;
					}
				}

				OpenApplyManager(-1, -1, nPayAdjRefID, nApplyID);
			}
		}

	}NxCatchAll("Error in LeftClickList");
}

// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
void CFinancialDlg::RightClickList(NXDATALIST2Lib::IRowSettingsPtr pRow, long iCol) {

	m_List->CurSel = pRow;

	CPoint pt;
	GetCursorPos(&pt);

	OnContextMenu(GetDlgItem(IDC_BILLING_TAB_LIST), pt);
}

void CFinancialDlg::RefreshList() {

	CWaitCursor wait;

	DisableFinancialScreen();

	try {

		// (j.jones 2010-06-08 09:18) - PLID 39042 - track the selected line ID,
		// and try to re-select it later
		IRowSettingsPtr pSelRow = m_List->GetCurSel();
		_variant_t varLineID = g_cvarNull;
		if(pSelRow) {
			varLineID = pSelRow->GetValue(btcLineID);
		}

		// (j.jones 2015-03-17 14:08) - PLID 64929 - ensure our cached colored cell, if any, is cleared
		m_pCurSearchResultsHighlightedRow = NULL;
		m_nCurSearchResultsHighlightedColumn = -1;
		m_clrCurSearchResultCellBackgroundColor = RGB(255, 255, 255);
		m_clrCurSearchResultCellForegroundColor = RGB(0, 0, 0);
		m_clrCurSearchResultCellBackgroundSelColor = RGB(255, 255, 255);
		m_clrCurSearchResultCellForegroundSelColor = RGB(0, 0, 0);

		m_List->Clear();

		for(int i=0; i<g_aryFinancialTabInfoT.GetSize();i++) {
			IRowSettingsPtr pRow = m_List->GetNewRow();
			FinTabItem *pItem = ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(i));

			pRow->PutValue(btcLineID,pItem->LineID);
			pRow->PutValue(m_nBillIDColID,pItem->BillID);
			pRow->PutValue(btcPayToPayID,pItem->PayToPayID);
			pRow->PutValue(btcPayID,pItem->PayID);
			pRow->PutValue(btcApplyID,pItem->ApplyID);
			pRow->PutValue(btcLineType,pItem->LineType);
			pRow->PutValue(btcChargeID,pItem->ChargeID);
			pRow->PutValue(m_nExpandCharColID,pItem->ExpandChar);
			pRow->PutValue(m_nExpandSubCharColID,pItem->ExpandSubChar);
			pRow->PutValue(m_nDateColID,pItem->Date);
			// (j.jones 2008-09-17 09:56) - PLID 19623 - supported new columns
			pRow->PutValue(m_nProviderColID,pItem->Provider);
			pRow->PutValue(m_nLocationColID,pItem->Location);
			pRow->PutValue(m_nPOSNameColID,pItem->POSName);
			pRow->PutValue(m_nPOSCodeColID,pItem->POSCode);
			pRow->PutValue(m_nInputDateColID,pItem->InputDate);
			pRow->PutValue(m_nCreatedByColID,pItem->CreatedBy);
			pRow->PutValue(m_nFirstChargeDateColID,pItem->FirstChargeDate);
			// (j.jones 2010-05-26 16:50) - PLID 28184 - added diagnosis code columns
			pRow->PutValue(m_nDiagCode1ColID,pItem->DiagCode1);
			pRow->PutValue(m_nDiagCode1WithNameColID,pItem->DiagCode1WithName);
			pRow->PutValue(m_nDiagCodeListColID,pItem->DiagCodeList);
			// (j.jones 2010-09-01 15:33) - PLID 40331 - added allowable column
			// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
			// and added ChargeAllowable
			pRow->PutValue(m_nFeeSchedAllowableColID, pItem->FeeSchedAllowable);
			pRow->PutValue(m_nChargeAllowableColID, pItem->ChargeAllowable);
			// (j.jones 2011-12-20 11:30) - PLID 47119 - added deductible and coinsurance
			pRow->PutValue(m_nChargeDeductibleColID, pItem->ChargeDeductible);
			pRow->PutValue(m_nChargeCoinsuranceColID, pItem->ChargeCoinsurance);
			// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
			pRow->PutValue(m_nInvoiceNumberColID, pItem->InvoiceNumber);

			if(VarLong((pItem->HasNotes),0) == 1) {
				extern CPracticeApp theApp;
				pRow->PutValue(m_nNoteIconColID,(long)m_hNotes);
			}
			else {
				pRow->PutValue(m_nNoteIconColID,pItem->NoteIcon);
			}

			{
				// (a.wilson 2014-07-24 11:46) - PLID 63015
				// (j.jones 2015-02-20 11:28) - PLID 64935 - the On Hold column can also show a void & correct icon,
				// using the following logic:
				//	- original line items that have been voided:						Void icon
				//	- voiding line items that cancel out an original:					Void icon
				//	- corrected line items that are not re-voided, and not on hold:		Corrected Icon
				//	- any editable line item (including corrections) that are on hold:	On Hold Icon
				// In other words, On Hold supercedes Corrected, but does not supercede Void.
				_variant_t varIcon = g_cvarNull;
				if (VarLong(pItem->IsVoided, 0) == 1) {
					//this is a voiding line item, or a voided original line item
					varIcon = (long)m_hVoided;
				}
				else if (VarLong(pItem->OnHold, 0) == 1) {
					//this is not a void item, and it is on hold
					varIcon = (long)m_hOnHold;
				}
				else if (VarLong(pItem->IsCorrected, 0) == 1) {
					//this is an editable corrected line item that
					//is not on hold nor has it been voided again
					varIcon = (long)m_hCorrected;
				}
				pRow->PutValue(m_nOhHoldIconColID, varIcon);
			}

			// (j.jones 2015-02-23 15:13) - PLID 64934 - added an assert to ensure
			// we're never showing voided/original items if the user wished to hide them
			if (m_checkHideVoidedItems.GetCheck()) {
				//if you hit this, one of the queries to load bills/charges/pays
				//is returning an original or void item that is supposed to be hidden
				ASSERT(VarLong(pItem->IsVoided, 0) == 0);

				//don't actually suppress the row, it will just show with the Void icon
			}

			// (a.wetta 2007-04-24 10:59) - PLID 25170 - Update the discount icon
			/*if((pItem->Discount).vt != VT_EMPTY &&
			VarCurrency((pItem->Discount), COleCurrency(0,0)) > COleCurrency(0,0)) {
			if (((pItem->HasPercentOff).vt != VT_EMPTY &&
			VarLong((pItem->HasPercentOff), 0) == 1)) {*/
			if(VarCurrency((pItem->Discount), COleCurrency(0,0)) > COleCurrency(0,0) ||
				VarLong((pItem->HasPercentOff),0) == 1) {
					//put the icon that denotes them having a discount				
					pRow->PutValue(m_nDiscountIconColID,(long)m_hDiscount);
			} 
			else {				
				pRow->PutValue(m_nDiscountIconColID, (long)m_hBlank);
			}

			pRow->PutValue(m_nDescriptionColID,pItem->Description);
			pRow->PutValue(m_nChargeColID,pItem->ChargeAmount);
			// (j.jones 2015-02-27 09:51) - PLID 64943 - fill the Total Pay Amount column
			pRow->PutValue(m_nTotalPaymentAmountColID, pItem->TotalPayAmount);
			pRow->PutValue(m_nPaymentColID,pItem->PayAmount);
			pRow->PutValue(m_nAdjustmentColID,pItem->Adjustment);
			pRow->PutValue(m_nRefundColID,pItem->Refund);
			pRow->PutValue(m_nPatientBalanceColID,pItem->PatResp);
			pRow->PutValue(m_nPrimInsColID,pItem->InsResp1);
			pRow->PutValue(m_nSecInsColID,pItem->InsResp2);

			// (j.jones 2010-06-18 10:51) - PLID 39150 - fill the dynamic columns
			for(int r=0;r<m_aryDynamicRespTypeIDs.GetSize();r++) {
				long nRespTypeID = (long)m_aryDynamicRespTypeIDs.GetAt(r);

				//find the column
				short iColumnIndex = -1;
				if(m_mapRespTypeIDToColumnIndex.Lookup(nRespTypeID, iColumnIndex) && iColumnIndex != -1) {

					//these fields should exist in the item
					BOOL bFound = FALSE;
					for(int d=0; d<pItem->arypDynInsResps.GetSize() && !bFound; d++) {
						DynInsResp *dir = (DynInsResp*)pItem->arypDynInsResps.GetAt(d);
						if(dir->nRespTypeID == nRespTypeID) {
							bFound = TRUE;

							//fill the column
							pRow->PutValue(iColumnIndex, dir->varResp);
						}
					}

					if(!bFound) {
						//possible if a standalone payment line that has no resps.
						pRow->PutValue(iColumnIndex, g_cvarNull);
					}
				}
				else {
					//should be impossible, this would mean that we
					//loaded a resp type for a column that does not exist,
					//and these should never be out of sync
					ASSERT(FALSE);
				}
			}

			pRow->PutValue(m_nOtherInsuranceColID,pItem->InsRespOther);
			pRow->PutValue(m_nBalanceColID,pItem->BalanceAmount);
			m_List->AddRowAtEnd(pRow, NULL);
		}

		m_List->SetSelByColumn(btcLineID, varLineID);

		// (j.jones 2015-03-17 13:25) - PLID 64931 - RevalidateSearchResults will restore our current search
		// if nothing changed, or clear the search if content did change
		RevalidateSearchResults();

	}NxCatchAll("Error in RefreshList()");

	// (j.jones 2008-09-17 15:01) - PLID 31405 - after refreshing the list, we must resize the description column
	// (j.jones 2008-09-18 11:50) - not necessary because EnableFinancialScreen will handle this
	//ResizeDescriptionColumn();

	EnableFinancialScreen();
}

//Sends a new LineID, based on FinancialTabInfo T, Temp, or Temp2.
int CFinancialDlg::GetNewNum(CString arrayname) {

	long ID;

	if(arrayname == "T") {
		if(g_aryFinancialTabInfoT.GetSize()>0) {
			ID = ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(g_aryFinancialTabInfoT.GetSize()-1))->LineID.lVal;
			if(ID>0)
				return ID + 1;
		}
	}
	else if (arrayname == "Temp") {
		if(g_aryFinancialTabInfoTemp.GetSize()>0) {
			ID = ((FinTabItem*)g_aryFinancialTabInfoTemp.GetAt(g_aryFinancialTabInfoTemp.GetSize()-1))->LineID.lVal;
			if(ID>0)
				return ID + 1;
		}
	}
	else if (arrayname == "Temp2") {
		if(g_aryFinancialTabInfoTemp2.GetSize()>0) {
			ID = ((FinTabItem*)g_aryFinancialTabInfoTemp2.GetAt(g_aryFinancialTabInfoTemp2.GetSize()-1))->LineID.lVal;
			if(ID>0)
				return ID + 1;
		}
	}

	return 1;
}

void FinTabItemCopy(FinTabItem* src, FinTabItem* dest) {
	dest->LineID = src->LineID;
	dest->LineType = src->LineType;
	dest->Expandable = src->Expandable;
	dest->ExpandChar = src->ExpandChar;
	dest->ExpandSubChar = src->ExpandSubChar;
	dest->Date = src->Date;		
	dest->HasNotes = src->HasNotes;
	dest->NoteIcon = src->NoteIcon;
	// (a.wilson 2014-07-24 11:43) - PLID 63015
	dest->OnHold = src->OnHold;
	// (j.jones 2015-02-20 11:35) - PLID 64935 - added IsVoided and IsCorrected
	dest->IsVoided = src->IsVoided;
	dest->IsCorrected = src->IsCorrected;
	dest->Description = src->Description;
	dest->ShowChargeAmount = src->ShowChargeAmount;
	dest->BillID = src->BillID;
	dest->ChargeID = src->ChargeID;
	dest->PayID = src->PayID;
	dest->PayToPayID = src->PayToPayID;
	dest->ApplyID = src->ApplyID;
	dest->ShowBalanceAmount = src->ShowBalanceAmount;
	dest->ChargeAmount = src->ChargeAmount;
	// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field
	dest->TotalPayAmount = src->TotalPayAmount;
	dest->PayAmount = src->PayAmount;
	dest->Adjustment = src->Adjustment;
	dest->Refund = src->Refund;
	dest->BalanceAmount = src->BalanceAmount;
	dest->InsResp1 = src->InsResp1;
	dest->InsResp2 = src->InsResp2;

	// (j.jones 2010-06-18 10:04) - PLID 39150 - clear the old dynamic columns,
	// then add the new ones
	for(int i=dest->arypDynInsResps.GetSize()-1;i>=0;i--) {
		delete dest->arypDynInsResps.GetAt(i);
		dest->arypDynInsResps.RemoveAt(i);
	}
	for(int i=0;i<src->arypDynInsResps.GetSize();i++) {
		DynInsResp *oldDir = (DynInsResp*)src->arypDynInsResps.GetAt(i);
		DynInsResp *newDir = new DynInsResp;
		newDir->nRespTypeID = oldDir->nRespTypeID;
		newDir->varResp = oldDir->varResp;
		dest->arypDynInsResps.Add(newDir);
	}

	dest->InsRespOther = src->InsRespOther;
	dest->PatResp = src->PatResp;
	dest->IsPrePayment = src->IsPrePayment;
	dest->Discount = src->Discount;
	dest->HasPercentOff = src->HasPercentOff;
	// (j.jones 2008-09-17 10:02) - PLID 19623 - supported new columns
	dest->Provider = src->Provider;
	dest->Location = src->Location;
	dest->POSName = src->POSName;
	dest->POSCode = src->POSCode;
	dest->InputDate = src->InputDate;
	dest->CreatedBy = src->CreatedBy;
	dest->FirstChargeDate = src->FirstChargeDate;
	// (j.jones 2010-05-26 16:50) - PLID 28184 - added diagnosis code fields
	dest->DiagCode1 = src->DiagCode1;
	dest->DiagCode1WithName = src->DiagCode1WithName;
	dest->DiagCodeList = src->DiagCodeList;
	// (j.jones 2010-09-01 15:39) - PLID 40331 - added allowable
	// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable,
	// and added ChargeAllowable
	dest->FeeSchedAllowable = src->FeeSchedAllowable;
	dest->ChargeAllowable = src->ChargeAllowable;
	// (j.jones 2011-12-20 11:30) - PLID 47119 - added deductible and coinsurance
	dest->ChargeDeductible = src->ChargeDeductible;
	dest->ChargeCoinsurance = src->ChargeCoinsurance;
	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
	dest->InvoiceNumber = src->InvoiceNumber;
}

void CFinancialDlg::StoreDetails()
{
	try {

		BOOL bAddedStatement = FALSE;
		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		CString strNoteText;
		GetDlgItemText(IDC_STATEMENT_NOTES, strNoteText);

		if(m_bStatementNoteChanged) {			

			// (j.jones 2009-12-29 14:18) - PLID 27237 - parameterized & batched
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "Update PatientsT SET StatementNote = {STRING} WHERE PersonID = {INT}", strNoteText, m_iCurPatient);
			////////////////////////////////////
			//prompt to save note
			if(strNoteText!="" && (!(m_StatementNoteCombo->GetCurSel() != -1 &&
				(CString(m_StatementNoteCombo->GetValue(m_StatementNoteCombo->GetCurSel(),0).bstrVal)==strNoteText)))) {
					if(IDYES == MessageBox("Would you like to add this statement note for future use?","Practice",MB_YESNO|MB_ICONINFORMATION)) {
						// (j.jones 2009-12-29 14:18) - PLID 27237 - parameterized & batched
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "INSERT INTO StatementNotesT (Note) VALUES ({STRING})", strNoteText);
						bAddedStatement = TRUE;
					}
			}
		}

		GetDlgItemText (IDC_EDIT_NOTES, m_FinancialNotes);
		if(m_bNoteChanged) {
			// (j.jones 2009-12-29 14:18) - PLID 27237 - parameterized & batched
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE PatientsT SET FinancialNotes = {STRING} WHERE PersonID = {INT}", m_FinancialNotes, m_iCurPatient);
		}

		if(!strSqlBatch.IsEmpty()) {
			// (a.walling 2012-02-09 17:13) - PLID 48115 - Use NxAdo::PushMaxRecordsWarningLimit and NxAdo::PushPerformanceWarningLimit
			NxAdo::PushMaxRecordsWarningLimit pmr(3);
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
		}

		if(bAddedStatement) {
			m_StatementNoteCombo->Requery();
		}

		m_bStatementNoteChanged = FALSE;
		m_bNoteChanged = FALSE;

	} NxCatchAll("Error in OnChangeEditNotes");

}

void CFinancialDlg::OnKillfocusEditNotes() 
{
	if(m_bNoteChanged)
		StoreDetails();	
}

void CFinancialDlg::OnKillfocusStatementNotes() 
{
	if(m_bStatementNoteChanged)
		StoreDetails();
}

void CFinancialDlg::OnChangeEditNotes() 
{
	m_bNoteChanged = TRUE;	
}

void CFinancialDlg::OnChangeStatementNotes() 
{
	m_bStatementNoteChanged = TRUE;
}

void CFinancialDlg::PostEditBill(long iBillID, int iSaveType)
{
	CWaitCursor pWait;

	try {
		switch(iSaveType) {
	case 1: //New bill
		if (iBillID != -1) {

			//JJ - 12-19-2001 - We used to just add the line manually, but it does not add it in the proper order,
			//so just refresh cleanly.
			QuickRefresh();	
		}
		break;
	case 2: //Existing bill
		RedrawBill(iBillID);
		RedrawAllPayments();
		CalculateTotals();

		//ResetBrightenedRow();
		break;
	case 3: {
		/////////////////////////////////////////////////////////////////
		// Apply the payment to the bill

		// (j.jones 2008-06-13 17:48) - PLID 29872 - use the cached payment ID that started this bill
		long nPaymentID = m_nPostEditBill_ApplyPaymentID;
		m_nPostEditBill_ApplyPaymentID = -1;

		//-1 shouldn't be possible, but safely handle it if it is found
		if(nPaymentID == -1) {
			ASSERT(FALSE);
		}

		// (a.walling 2010-05-03 14:24) - PLID 38461 - Set to true if they cannot autoapply due to lack of permissions or other reason; a message box
		// will appear to inform them of this.
		bool bCannotAutoApply = false;

		// (j.jones 2007-04-19 11:18) - PLID 25721 - In most cases CanEdit is not needed, so first
		// silently see if they have permission, and if they do, then move ahead normally. But if
		// they don't, or they need a password, then check CanEdit prior to the permission check that
		// would stop or password-prompt the user.
		// (a.walling 2010-05-03 14:23) - PLID 38461 - Don't bother checking if iBillID is -1 or the nPaymentID is -1
		if (iBillID != -1 && nPaymentID != -1) {
			// (a.walling 2010-05-03 14:33) - PLID 38461 - No where seems to call CanEdit("Payment") with regards to creation of applies, just deletes.
			// This is the only place that still calls this after fixing the other two that led to exceptions. Since we are not actually editing
			// the payment (only applies), I am getting rid of this here too for consistency.
			// (j.jones 2011-08-24 17:40) - PLID 45176 - CanApplyLineItem will check the normal apply create permission,
			// but only after it checks to see if the source payment is closed
			// (j.jones 2013-07-01 10:45) - PLID 55517 - this function can now potentially correct the payment, if so
			// the payment ID will be changed to reflect the new, corrected line item
			if(CanApplyLineItem(nPaymentID) != ecalirCannotApply) {

				//before allowing them to apply, see if the source is a prepayment and the destination
				//is not a bill or charge created from the quote that the prepayment is linked with
				if(!AllowPaymentApply(nPaymentID, iBillID,"Bill")) {

					FullRefresh();
					return;
				}

				AutoApplyPayToBill(nPaymentID, m_iCurPatient, "Bill", iBillID);
			} else {
				bCannotAutoApply = true;
			}
		}

		if (bCannotAutoApply) {
			// (a.walling 2010-05-03 14:25) - PLID 38461 - Run on sentence! NO SOUP FOR YOU!
			AfxMessageBox("The payment could not be automatically applied; you must apply it manually.");
		}

		/////////////////////////////////////////////////////////////////
		// Redraw everything
		// a.walling 4/14/06 Instead of a FullRefresh, a QuickRefresh will suffice
		// FullRefresh();
		QuickRefresh();
		break;
			}
	default:
		break;
		}
	}NxCatchAll("Error in PostEditBill()");
}

LRESULT CFinancialDlg::OnBarcodeScan(WPARAM wParam, LPARAM lParam)
{
	// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
	// (j.jones 2010-03-15 15:19) - PLID 37719 - removed unnecessary log
	//Log("Barcode scanned lParam = %s", (LPCTSTR)_bstr_t((BSTR)lParam));

	if (m_BillingDlg->IsWindowVisible())
		return m_BillingDlg->SendMessage(WM_BARCODE_SCAN, wParam, lParam);

	// CH 4/5 Temporary idea: Open the billing
	// dialog
	// (a.walling 2009-12-22 17:55) - PLID 7002 - Maintain only one instance of a bill - silent
	if(CheckCurrentUserPermissions(bioBill,sptCreate) && !GetMainFrame()->IsBillingModuleOpen(false))
	{
		try {
			// (a.walling 2007-11-08 16:28) - PLID 27476 - Need to convert this correctly from a bstr
			// (c.haag 2003-07-31 17:49) - Don't open it if it's not a barcode anywhere
			// (j.jones 2004-02-24 10:19) - it won't compare to see if the product is billable against the current location, just any location
			// (j.jones 2009-12-28 17:13) - PLID 27237 - parameterized
			//(c.copits 2010-09-24) PLID 40317 - Allow duplicate UPC codes for FramesData certification
			_RecordsetPtr prs = GetBestUPCProduct(lParam);
			if (!prs->eof)
			{
				prs->Close();
				m_BillingDlg->m_pFinancialDlg = this;		//DRT 12/27/2004 - PLID 15083 - See comments in OnNewBill
				m_BillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1);
				return m_BillingDlg->SendMessage(WM_BARCODE_SCAN, wParam, lParam);
			}
		}
		NxCatchAll("Error validating barcode");
	}
	//TES 2003-12-22: Don't send this message if the bill didn't even get opened.
	//return m_BillingDlg->SendMessage(WM_BARCODE_SCAN, wParam, lParam);
	return 0;
}

void CFinancialDlg::OnSelChosenStatementNoteCombo(long nRow) 
{
	// (f.dinatale 2010-09-01) - PLID 37549 - Added the try/catch and the use of VarString to default to empty string on VT_NULL
	try{
		if(nRow != -1){
			CString str;
			GetDlgItem(IDC_STATEMENT_NOTES)->GetWindowText(str);
			if(str!="" &&
				IDNO == MessageBox("This will overwrite the current text. Are you sure?","Practice",MB_YESNO|MB_ICONINFORMATION)) {
					return;
			}

			str = VarString(m_StatementNoteCombo->GetValue(nRow,0), "");
			SetDlgItemText(IDC_STATEMENT_NOTES,str);
			m_StatementNoteCombo->PutComboBoxText("");
			m_bStatementNoteChanged = TRUE;
			StoreDetails();
		}
	} NxCatchAll(__FUNCTION__);
}

void CFinancialDlg::OnEditStatementNotes() 
{
	try{
		GetMainFrame()->DisableHotKeys();
		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 6, m_StatementNoteCombo, "Edit Combo Box").DoModal();
		GetMainFrame()->EnableHotKeys();
	} NxCatchAll(__FUNCTION__);
}

//JJ - Disable/EnableFinancialScreen and AdjustVisibleRows are useful ways to
//manage the layout of the datalist while it is expanded, collapsed, redrawn, etc.
//By using these tools, we can requery the list all we want and still be viewing
//the same rows, even if it means scrolling the list.

//JJ- DisableFinancialScreen should be called before any expand/collapse
//or any other function that will redraw the screen. This uses
//reference counting, which plays a bigger role in this function's
//partner, EnableFinancialScreen.
//This function will save the current selected row, and disable the redrawing
//of the screen so that any datalist operations can occur stealthily.
void CFinancialDlg::DisableFinancialScreen() {

	try {

		//Increment the reference count for these function pairs
		m_RedrawRefCount++;

		//if the screen is already hidden, leave
		if(!m_bIsScreenEnabled)
			return;
		else {

			// (j.jones 2010-06-07 17:07) - PLID 39042 - these fields now track the LineID
			// of the respective rows

			//save the current row's line ID
			IRowSettingsPtr pRow = m_List->GetCurSel();
			if(pRow) {
				m_SavedRow_LineID = VarLong(pRow->GetValue(btcLineID), 0);
			}
			else {
				m_SavedRow_LineID = 0;
			}

			pRow = m_List->GetTopRow();
			if(pRow) {
				m_SavedTopRow_LineID = VarLong(pRow->GetValue(btcLineID), 0);
			}
			else {
				m_SavedTopRow_LineID = 0;
			}

			//turn off the redraw
			m_List->SetRedraw(FALSE);

			//set the boolean to let future instances of this function
			//know that this function has already been called
			m_bIsScreenEnabled = FALSE;
		}

	}NxCatchAll("Error in CFinancialDlg::DisableFinancialScreen");
}


//JJ- EnableFinancialScreen MUST be called following DisableFinancialScreen.
//If they don't get called an equal number of times, the screen will never
//re-enable. This function will set the selection to the previously saved row,
//brighten it, adjust the screen to show the appropriate information (AdjustVisibleRows),
//and then redraw the screen.
//This function uses reference counting, so if a Disable/Enable pair gets called within
//another Disable/Enable pair, only the outer calls get executed.
//The reasoning behind this is, these functions hide the screen while various
//operations get carried out. If one of those enables the screen, it defeats the whole purpose.
void CFinancialDlg::EnableFinancialScreen() {

	try {

		//Decrement the reference count for these function pairs
		m_RedrawRefCount--;

		//if the screen is already enabled (should not be possible)
		//or the reference count is not zero (much more likely),
		//leave this function
		if(m_bIsScreenEnabled || m_RedrawRefCount > 0)
			return;
		else {
			//if the reference count is zero, we are at the top level
			//of the Disable/Enable pairs, so let's go to work!

			// (j.dinatale 2010-10-27) - PLID 28773 - Only resize the description column if the user isnt opting to remember col width
			// (j.jones 2008-09-17 15:58) - PLID 31405 - resize the description column, to make
			// sure that whatever change we just made doesn't recalculate the widths
			if(!m_bRememberColWidth){
				ResizeDescriptionColumn();
			}

			//first set the top row, so we aren't disoriented

			// (j.jones 2010-06-07 17:13) - PLID 39042 - now try to find the row by line ID
			if(m_SavedTopRow_LineID > 0) {
				IRowSettingsPtr pRow = m_List->FindByColumn(btcLineID, (long)m_SavedTopRow_LineID, NULL, FALSE);
				if(pRow) {
					m_List->PutTopRow(pRow);
				}
			}

			//then set the selection to where we previously were
			if(m_SavedRow_LineID > 0) {
				m_List->FindByColumn(btcLineID, (long)m_SavedRow_LineID, NULL, TRUE);
			}

			//m_List->TopRowIndex = m_SavedRow;

			//all done! Let's redraw the window now
			m_List->SetRedraw(TRUE);
			UpdateWindow();

			// (j.jones 2010-06-08 12:58) - PLID 39042 - AdjustVisibleRows has to be
			// called after SetRedraw is called, otherwise the EnsureRowInView() logic
			// just doesn't do anything in the Datalist 2

			//now call AdjustVisibleRows, to slickly set our viewing area up
			AdjustVisibleRows();

			//let future calls of these functions know the screen is ready to go
			m_bIsScreenEnabled = TRUE;

			m_RedrawRefCount = 0;

			// (j.jones 2015-03-17 13:25) - PLID 64931 - RevalidateSearchResults will restore our current search
			// if nothing changed, or clear the search if content did change
			RevalidateSearchResults();
		}

	}NxCatchAll("Error in CFinancialDlg::EnableFinancialScreen");
}

//JJ- AdjustVisibleRows will make expanded items behave like Windows Explorer.
//If we expand a bill, the datalist gets reloaded behind the scenes. As
//a result, we would be at the top of the list, and if the expanded bill is scrolled
//down off the bottom of the list, it is hidden. If we merely used PutCurSel,
//we would only see the Bill line, at the bottom of the screen. The expanded
//information would still be hidden. This is where AdjustVisibleRows comes into play.
//Basic rules -
//If the expanded item, let's say a bill, has several sub-items, like charges,
//the list should advance to show the last item in the expanded list. If that
//takes up the whole screen and pushes the original expanded item off the screen,
//we need to at least show the expanded item.
//Use EnsureRowInView to accomplish this by first showing the bottom row. That
//will advance the list down to show that row as the last row. Then use
//EnsureRowInView to show the original expanded item. If it is off the screen,
//the list will advance upwards to make that the top item. If ExpandRowVisible
//is called on a row that is already visible, nothing happens. So at the end of
//the function, call it again just to make sure.
void CFinancialDlg::AdjustVisibleRows() {

	try {

		//DRT 3/2/2004 - PLID 11187 - if the saved row is not > 0, don't do anything, it will put us at the top
		if(m_SavedRow_LineID <= 0) 
			return;

		//DRT 3/2/2004 - PLID 11187 - also the saved row might be not a valid row, if this is a different patient
		if(m_SavedRow_LineID > m_List->GetRowCount() - 1)
			return;

		_variant_t var;
		//start by getting the line type

		IRowSettingsPtr pSavedRow = m_List->FindByColumn(btcLineID, (long)m_SavedRow_LineID, NULL, FALSE);
		if(pSavedRow == NULL) {
			return;
		}

		var = pSavedRow->GetValue(btcLineType);

		if(var.vt == VT_BSTR) {
			CString strLineType = CString(var.bstrVal);
			IRowSettingsPtr pTopRow = NULL;
			IRowSettingsPtr pBottomRow = NULL;

			//Bills and charges will use the same code.
			//If a bill is expanded, we want to show the bill line and all
			//of its charges that can possibly fit in the window.
			//If a charge is expanded, we want the same behavior because
			//we still want to show the bill line.
			//In the event an expanded charge is off the screen that the bill
			//is on, we will at least be advanced to that line.
			//(I doubt this will happen much, if at all. If so, we need to
			//make separate logic for the Charge line)
			if(strLineType == "Bill" || strLineType == "Charge") {
				pTopRow = pSavedRow;
				long BillID, tmpID;
				var = pSavedRow->GetValue(m_nBillIDColID);
				if(var.vt == VT_I4) {
					BillID = VarLong(var);
					tmpID = BillID;
					IRowSettingsPtr pTempRow = pSavedRow;
					pBottomRow = pTempRow;
					while(BillID == tmpID && pTempRow != NULL) {
						pBottomRow = pTempRow;
						pTempRow = pTempRow->GetNextRow();
						//DRT 3/2/2004 - PLID 11187 - You can't get values from rows that don't exist.
						tmpID = -1;
						if(pTempRow) {
							var = pTempRow->GetValue(m_nBillIDColID);
							if(var.vt == VT_I4) {
								tmpID = VarLong(var);
							}
						}
					}
					m_List->EnsureRowInView(pBottomRow);
					m_List->EnsureRowInView(pTopRow);
				}
			}
			//Pay/Adj/Ref will all be handled the same way, with a method just like Bills
			//and Charges. The only difference is that the outer expansion is the PayID,
			//rather than BillID, and will be as such regardless of the Payment type.
			else if(strLineType == "Payment" || strLineType == "Adjustment" || strLineType == "Refund") {
				pTopRow = pSavedRow;
				long PayID, tmpID;
				var = pSavedRow->GetValue(btcPayID);
				if(var.vt == VT_I4) {
					PayID = VarLong(var);
					tmpID = PayID;
					IRowSettingsPtr pTempRow = pSavedRow;
					pBottomRow = pTempRow;
					while(PayID == tmpID && pTempRow != NULL) {
						pBottomRow = pTempRow;
						pTempRow = pTempRow->GetNextRow();
						//DRT 3/2/2004 - PLID 11187 - You can't get values from rows that don't exist.
						tmpID = -1;
						if(pTempRow) {
							var = pTempRow->GetValue(btcPayID);
							if(var.vt == VT_I4) {
								tmpID = VarLong(var);
							}
						}
					}
					m_List->EnsureRowInView(pBottomRow);
					m_List->EnsureRowInView(pTopRow);
				}
			}

		}

	}NxCatchAll("Error in AdjustVisibleRows");

	//still do this if any of the if statements failed, if we caught an error, etc.
	//regardless of whatever slick things we do in this function, if the m_SavedRow isn't visible,
	//it's all for nothing...
	//then set the selection to where we previously were
	if(m_SavedRow_LineID > 0) {
		IRowSettingsPtr pSavedRow = m_List->FindByColumn(btcLineID, (long)m_SavedRow_LineID, NULL, FALSE);
		if(pSavedRow) {
			m_List->EnsureRowInView(pSavedRow);
		}
	}

	//NOTE: m_SavedTopRow does NOT need to be adjusted. It is a great starting point,
	//and makes the expanded item stay put if the screen doesn't need to be scrolled,
	//but showing all the expanded data is much more important than saving the top row.
}

void CFinancialDlg::OnNewBill() 
{
	// (j.jones 2014-07-28 13:26) - PLID 62947 - added exception handling
	try {

		// (d.singleton 2014-02-27 14:46) - PLID 61072 - first we need to get our insured parties
		long nPatientID = GetActivePatientID();
		long nInsuredPartyID = -1;
		//check pref to see if we need pop up menu for bill to resp ( value of 2 )
		long nDefaultRespType = GetRemotePropertyInt("BillPrimaryIns", GetRemotePropertyInt("BillPrimaryIns", 0, 0, "<None>", false), 0, GetCurrentUserName(), TRUE);
		if (nDefaultRespType == 2) {
			_RecordsetPtr prsInsuredParties = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuredPartyT.PersonID, InsuranceCoT.Name, RespTypeT.TypeName, RespTypeT.ID AS RespTypeID "
				"FROM InsuredPartyT "
				"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
				"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
				"WHERE PatientID = {INT} AND RespTypeT.Priority <> -1 "
				"ORDER BY Coalesce(RespTypeT.CategoryPlacement,1000), RespTypeT.CategoryType, RespTypeT.Priority", nPatientID);

			if (!prsInsuredParties->eof) {
				CNxMenu menu;
				menu.m_hMenu = CreatePopupMenu();

				Nx::MenuCommandMap<long> menuCmds;

				long nMenuIndex = 0;
				//always add patient resp
				menu.InsertMenu(nMenuIndex++, MF_BYPOSITION, menuCmds(-1), "For Patient Responsibility");
				//add our other resp
				while (!prsInsuredParties->eof) {
					long nPersonID;
					CString strResp, strName, strMenuText;
					nPersonID = AdoFldLong(prsInsuredParties, "PersonID", -1);
					strResp = AdoFldString(prsInsuredParties, "TypeName", "");
					strName = AdoFldString(prsInsuredParties, "Name", "");
					strMenuText.Format("For %s (%s)", strName, strResp);
					menu.InsertMenu(nMenuIndex++, MF_BYPOSITION, menuCmds(nPersonID), strMenuText);

					prsInsuredParties->MoveNext();
				}

				CWnd *pWnd = GetDlgItem(IDC_NEW_BILL);
				if (pWnd) {
					CRect rect;
					pWnd->GetWindowRect(rect);
					nInsuredPartyID = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, rect.right, rect.top, this);
				}
				else {
					CPoint pt;
					GetCursorPos(&pt);
					nInsuredPartyID = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this);
				}

				boost::optional<long> choice = menuCmds.Translate(nInsuredPartyID);
				if (!choice) {
					return;
				}
				else {
					nInsuredPartyID = *choice;
				}
			}
		}

		if (!CheckCurrentUserPermissions(bioBill, sptCreate)) return;

		//checks for any active global period, and warns accordingly
		// (a.walling 2008-07-07 18:05) - PLID 29900 - Pass in the patient
		if (!CheckWarnGlobalPeriod(nPatientID))
			return;

		if (GetMainFrame()->IsBillingModuleOpen(true)) {
			return;
		}

		//////////////////////////////////////////////
		// The dialog will provide the interface for
		// creating a new bill
		m_boAllowUpdate = FALSE;

		if (m_pParent && m_BillingDlg)
			m_BillingDlg->m_pPatientView = m_pParent;

		//DRT 12/27/2004 - PLID 15083 - The pFinancialDlg ptr (poorly named) is only set in the OnInitDialog
		//	of this CFinancialDlg.  That was fine in the past, but now other things (like the EMR / Custom Records)
		//	need notification of when the bill is saved.  Therefore, they steal that pointer to be themselves, 
		//	and the financial dialog never retrieves it.  We'll just set it here every time they open a new bill, 
		//	and all should be well no matter who else opened it previously.
		m_BillingDlg->m_pFinancialDlg = this;
		//

		m_BillingDlg->m_boAskForNewPayment = TRUE;
		// (d.singleton 2014-02-27 17:16) - PLID 61072 - when clicking "New Bill" allowing you to choose the responsibility of the new bill before opening
		// (j.gruber 2016-03-22 10:12) - PLID 68006 - Change bFromEMR to an Enum
		m_BillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1, BillFromType::Other, nInsuredPartyID);

		//the bill is added to the screen in PostEditBill(1)

		m_boAllowUpdate = TRUE;

	}NxCatchAll(__FUNCTION__);
}

void CFinancialDlg::OnNewPayment() 
{
	try
	{
		long nPayMenu = GetRemotePropertyInt("PayAdjRefMenu", 1, 0, GetCurrentUserName(), true);

		if (nPayMenu == 0) {

			//they might just want the old interface, so just make a payment
			CreateNewPayment();
		}
		else {

			//otherwise pop up a menu of options

			// (j.jones 2007-04-05 15:58) - PLID 25514 - converted to not use defines anymore
			// (j.jones 2009-08-18 08:44) - PLID 24569 - added pre-payment option
			enum {
				miNewPay = -11,
				miNewPrePay = -12,
				miNewAdj = -13,
				miNewRef = -14,
				miPaymentProfile = -15,
			};

			CMenu mnu;
			mnu.m_hMenu = CreatePopupMenu();
			long nIndex = 0;
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, miNewPay, "Create a New &Payment");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, miNewPrePay, "Create a New Pr&e-Payment");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, miNewAdj, "Create a New &Adjustment");
			mnu.InsertMenu(nIndex++, MF_BYPOSITION, miNewRef, "Create a New &Refund");
			// (z.manning 2015-07-22 10:18) - PLID 67241 - Payment profile menu option
			if (IsICCPEnabled()) {
				mnu.InsertMenu(nIndex++, MF_BYPOSITION, miPaymentProfile, "New Pa&yment Profile");
			}

			CRect rc;
			CWnd *pWnd = GetDlgItem(IDC_NEW_PAYMENT);
			int nResult = 0;
			if (pWnd) {
				pWnd->GetWindowRect(&rc);
				nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, rc.right, rc.top, this, NULL);
			}
			else {
				CPoint pt;
				GetCursorPos(&pt);
				nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this, NULL);
			}

			switch (nResult) {
			case miNewPay:
			{
				try {
					CreateNewPayment();
				}NxCatchAll("Error creating new payment.");
			}
			break;
			// (j.jones 2009-08-18 08:44) - PLID 24569 - added pre-payment option
			case miNewPrePay:
			{
				try {
					CreateNewPayment(TRUE);
				}NxCatchAll("Error creating new pre-payment.");
			}
			break;
			case miNewAdj:
			{
				try {
					CreateNewAdjustment();
				}NxCatchAll("Error creating new adjustment.");
			}
			break;
			case miNewRef:
			{
				try {
					CreateNewRefund();
				}NxCatchAll("Error creating new refund.");
			}
			break;

			case miPaymentProfile:
				// (z.manning 2015-07-22 10:34) - PLID 67241
				OpenPaymentProfileDlg(GetActivePatientID(), this);
				break;
			}
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CFinancialDlg::OnPrintHistory() 
{
	//DRT 3/13/03 - No reason to duplicate this code, so I moved the print capabilities to the main frame
	//		for ease of access
	GetMainFrame()->PrintHistoryReport(true, 0);
}

void CFinancialDlg::OnPreviewStatement() 
{
	GetMainFrame()->DisableHotKeys();
	CGoStatements dlg(this);
	// (a.walling 2008-07-07 17:30) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
	dlg.m_nPatientID = GetActivePatientID();
	dlg.DoModal();
	GetMainFrame()->EnableHotKeys();
}

void CFinancialDlg::OnApplyManager() 
{
	try {

		// (j.jones 2015-02-25 09:45) - PLID 64939 - made one modular function to open Apply Manager
		OpenApplyManager(-1, -1, -1, -1);

	}NxCatchAll(__FUNCTION__);
}

void CFinancialDlg::OnShowInsBalance() 
{
	m_iInsuranceViewToggle++;
	if((m_iInsuranceViewToggle) == 3) {
		m_iInsuranceViewToggle -= 3;
	}

	// (j.jones 2010-09-27 15:56) - PLID 40123 - InsViewType is obsolete, replaced with BillingTab_ShowInsuranceColumns,
	// but is used to determine the default value for BillingTab_ShowInsuranceColumns. The new values for this
	// preference are: 0 - hide all, 1 - show all, 2 - hide unused

	//remember that the button shows the NEXT option in the sequence, not the currently displayed view
	switch (m_iInsuranceViewToggle) {	
		case 1:  //currently showing all
			GetDlgItem(IDC_SHOW_INS_BALANCE)->SetWindowText("Hide Unused Insurance");
			break;
		case 2: //currently hiding unused
			GetDlgItem(IDC_SHOW_INS_BALANCE)->SetWindowText("Hide Insurance");
			break;
		case 0: //currently hiding all
		default:
			GetDlgItem(IDC_SHOW_INS_BALANCE)->SetWindowText("Show Insurance");
			break;
	}

	// (j.jones 2010-09-27 16:33) - PLID 40123 - this preference name has changed
	SetRemotePropertyInt("BillingTab_ShowInsuranceColumns", m_iInsuranceViewToggle, 0, GetCurrentUserName());

	// (j.jones 2008-09-17 16:00) - PLID 19623 - renamed this function into something more accurate
	DisplayInsuranceAndRefundColumns();
}

void CFinancialDlg::OnShowPackages() 
{
	// (j.jones 2015-03-16 14:46) - PLID 64926 - this dialog is now a reference, not a pointer
	if (m_dlgPackages.GetSafeHwnd() == NULL)
	{
		m_dlgPackages.Create();
		m_dlgPackages.ShowWindow(SW_SHOW);
	}
	else {
		if (!m_dlgPackages.IsWindowVisible()) {
			m_dlgPackages.ShowWindow(SW_SHOW);
		}
	}
}

void CFinancialDlg::PostPackagePayment() {

	RedrawAllPayments();
	CalculateTotals();
}

LRESULT CFinancialDlg::BillPackage(WPARAM wParam, LPARAM lParam) {

	if (m_BillingDlg->IsWindowVisible()) {
		//m_BillingDlg->m_boAskForNewPayment = FALSE;
		m_BillingDlg->m_boShowAvailablePayments = TRUE;
		return m_BillingDlg->PostMessage(NXM_ADD_PACKAGE, wParam, lParam);
	}

	// (a.walling 2009-12-22 17:56) - PLID 7002 - Maintain only one instance of a bill
	if(CheckCurrentUserPermissions(bioBill,sptCreate) && !GetMainFrame()->IsBillingModuleOpen(true)) {
		m_BillingDlg->m_pFinancialDlg = this;		//DRT 12/27/2004 - PLID 15083 - See comments in OnNewBill
		//m_BillingDlg->m_boAskForNewPayment = FALSE;
		m_BillingDlg->m_boShowAvailablePayments = TRUE;
		m_BillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1);
	}
	return m_BillingDlg->PostMessage(NXM_ADD_PACKAGE, wParam, lParam);
}

void CFinancialDlg::OnResponsibleParty() 
{
	// (b.spivey, May 22, 2012) - PLID 50558 - Added try catch, force the color here to "patient blue"
	try {
		GetMainFrame()->DisableHotKeys();

		CResponsiblePartyDlg dlg(this);
		dlg.m_color = GetNxColor(GNC_PATIENT_STATUS, 1); 
		dlg.DoModal();

		GetMainFrame()->EnableHotKeys();
	} NxCatchAll(__FUNCTION__); 
}

void CFinancialDlg::OnSelChosenBalanceFilterCombo(long nRow) 
{
	CalculateTotals();
}

void CFinancialDlg::BuildBalanceFilterCombo()
{
	CString str;
	long ID;
	_RecordsetPtr rs;

	try {

		//add all the various balance filters.
		//For insures parties and locations, we also add their ID

		//Total Balance
		NXDATALISTLib::IRowSettingsPtr pRow = m_BalanceFilterCombo->GetRow(-1);
		str = "Total Balance";
		pRow->PutValue(bfccDesc,_bstr_t(str));
		pRow->PutValue(bfccLocID,(long)-1);
		pRow->PutValue(bfccLocName, _bstr_t(""));
		pRow->PutValue(bfccBalanceType,(long)bfbtTotal);
		m_BalanceFilterCombo->AddRow(pRow);

		//Patient Balance
		pRow = m_BalanceFilterCombo->GetRow(-1);
		str = "Patient Balance";
		pRow->PutValue(bfccDesc,_bstr_t(str));
		pRow->PutValue(bfccLocID,(long)-1);
		pRow->PutValue(bfccLocName, _bstr_t(""));
		pRow->PutValue(bfccBalanceType,(long)bfbtPatient);
		m_BalanceFilterCombo->AddRow(pRow);

		//Insurance Balance
		pRow = m_BalanceFilterCombo->GetRow(-1);
		str = "Insurance Balance";
		pRow->PutValue(bfccDesc,_bstr_t(str));
		pRow->PutValue(bfccLocID,(long)-1);
		pRow->PutValue(bfccLocName, _bstr_t(""));
		pRow->PutValue(bfccBalanceType,(long)bfbtInsurance);
		m_BalanceFilterCombo->AddRow(pRow);

		/*
		//Primary Insurance Balance
		rs = CreateRecordset("SELECT InsuredPartyT.PersonID AS InsPartyID, Name FROM InsuredPartyT INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID WHERE PatientID = %li AND RespTypeID = 2",m_iCurPatient);
		if(!rs->eof) {
		pRow = m_BalanceFilterCombo->GetRow(-1);
		pRow->PutValue(0,(long)BALANCE_INSURANCE);
		str.Format("%s (Primary) Balance",AdoFldString(rs, "Name",""));
		pRow->PutValue(1,_bstr_t(str));
		ID = AdoFldLong(rs, "InsPartyID",-1);
		pRow->PutValue(2,(long)ID);
		m_BalanceFilterCombo->AddRow(pRow);
		}
		rs->Close();

		//Secondary Insurance Balance
		rs = CreateRecordset("SELECT InsuredPartyT.PersonID AS InsPartyID, Name FROM InsuredPartyT INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID WHERE PatientID = %li AND RespTypeID = 2",m_iCurPatient);
		if(!rs->eof) {
		pRow = m_BalanceFilterCombo->GetRow(-1);
		pRow->PutValue(0,(long)BALANCE_INSURANCE);
		str.Format("%s (Secondary) Balance",AdoFldString(rs, "Name",""));
		pRow->PutValue(1,_bstr_t(str));
		ID = AdoFldLong(rs, "InsPartyID",-1);
		pRow->PutValue(2,(long)ID);
		m_BalanceFilterCombo->AddRow(pRow);
		}
		rs->Close();
		*/

		//Location Balances
		rs = CreateRecordset("SELECT ID, Name FROM LocationsT WHERE Managed = 1 AND TypeID = 1");
		while(!rs->eof) {
			// (j.jones 2010-05-07 08:23) - PLID 37398 - we now have location entries for
			// total, patient, and insurance balances
			CString strLocName = AdoFldString(rs, "Name","");
			str.Format("Total Balance for Location '%s'",strLocName);
			pRow = m_BalanceFilterCombo->GetRow(-1);
			pRow->PutValue(bfccDesc,_bstr_t(str));
			ID = AdoFldLong(rs, "ID",-1);
			pRow->PutValue(bfccLocID,(long)ID);
			pRow->PutValue(bfccLocName, _bstr_t(strLocName));
			pRow->PutValue(bfccBalanceType,(long)bfbtTotal);
			m_BalanceFilterCombo->AddRow(pRow);

			str.Format("Patient Balance for Location '%s'",strLocName);
			pRow = m_BalanceFilterCombo->GetRow(-1);
			pRow->PutValue(bfccDesc,_bstr_t(str));
			ID = AdoFldLong(rs, "ID",-1);
			pRow->PutValue(bfccLocID,(long)ID);
			pRow->PutValue(bfccLocName, _bstr_t(strLocName));
			pRow->PutValue(bfccBalanceType,(long)bfbtPatient);
			m_BalanceFilterCombo->AddRow(pRow);

			str.Format("Insurance Balance for Location '%s'",strLocName);
			pRow = m_BalanceFilterCombo->GetRow(-1);
			pRow->PutValue(bfccDesc,_bstr_t(str));
			ID = AdoFldLong(rs, "ID",-1);
			pRow->PutValue(bfccLocID,(long)ID);
			pRow->PutValue(bfccLocName, _bstr_t(strLocName));
			pRow->PutValue(bfccBalanceType,(long)bfbtInsurance);
			m_BalanceFilterCombo->AddRow(pRow);

			rs->MoveNext();
		}
		rs->Close();

		m_BalanceFilterCombo->CurSel = 0;

	}NxCatchAll("Error building balance filter list.");
}

CString CFinancialDlg::GetBalanceFilter(CString strFilterTable)
{
	try {

		CString strWhere = "";
		CString str;

		if(m_BalanceFilterCombo->CurSel == -1)
			return "";

		long nLocationID = VarLong(m_BalanceFilterCombo->GetValue(m_BalanceFilterCombo->GetCurSel(),bfccLocID), -1);
		BalanceFilterBalanceType bfbtType = (BalanceFilterBalanceType)VarLong(m_BalanceFilterCombo->GetValue(m_BalanceFilterCombo->GetCurSel(),bfccBalanceType), (long)bfbtTotal);

		// (j.jones 2010-05-07 08:31) - PLID 37398 - reworked this combo to also include
		// patient & insurance balances per location
		if(bfbtType == bfbtPatient) {
			str.Format(" AND (%s.InsuredPartyID IS NULL OR %s.InsuredPartyID = -1) ",strFilterTable,strFilterTable);
			strWhere += str;
		}
		else if(bfbtType == bfbtInsurance) {
			str.Format(" AND (%s.InsuredPartyID IS NOT NULL AND %s.InsuredPartyID <> -1) ",strFilterTable,strFilterTable);
			strWhere += str;
		}

		if(nLocationID != -1) {
			str.Format(" AND LineItemT.LocationID = %li ",nLocationID);
			strWhere += str;
		}

		return strWhere;

	}NxCatchAll("Error retrieving filter.");

	return "";
}

void CFinancialDlg::OnClaimHistory() 
{
	CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(342)]);
	infReport.nPatient = m_iCurPatient;

	CRParameterInfo *paramInfo;
	CPtrArray paParams;
	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = "";
	paramInfo->m_Name = "LocName";
	paParams.Add(paramInfo);
	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = "01/01/1000";
	paramInfo->m_Name = "DateFrom";
	paParams.Add(paramInfo);
	paramInfo = new CRParameterInfo;
	paramInfo->m_Data = "12/31/5000";
	paramInfo->m_Name = "DateTo";
	paParams.Add(paramInfo);


	infReport.strReportFile += "DTLD";

	//Made new function for running reports - JMM 5-28-04
	RunReport(&infReport, &paParams, true, (CWnd *)this, "Patient Claim History");
	ClearRPIParameterList(&paParams);	//DRT - PLID 18085 - Cleanup after ourselves
}

void CFinancialDlg::OnRequeryFinishedGlobalPeriodList(short nFlags) 
{
	for(int i=0; i<m_GlobalPeriodList->GetRowCount(); i++) {
		if(VarLong(m_GlobalPeriodList->GetValue(i,3)) == 1) {
			//expired
			NXDATALISTLib::IRowSettingsPtr(m_GlobalPeriodList->GetRow(i))->PutForeColor(RGB(255,0,0));
		}
	}

	m_GlobalPeriodList->SetRedraw(TRUE);
}

void CFinancialDlg::ApplyRefundToAppliedPayment(long nPaymentID, long nApplyID, long nRefundID, BOOL bAutoApply /*= FALSE*/)
{
	try {

		//we are going to try to apply the refund to an applied payment, first unapplying enough of the payment as we need to

		COleCurrency cyPayAmount, cyRefundAmount, cyApplyAmount;

		cyPayAmount = cyRefundAmount = cyApplyAmount = COleCurrency(0,0);

		long PayInsuredPartyID = -1, RefInsuredPartyID = -1;
		long PayLocationID = -1;
		// (j.jones 2009-12-28 17:13) - PLID 27237 - parameterized & combined into one recordset
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuredPartyID, LocationID FROM PaymentsT "
			"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID WHERE PaymentsT.ID = {INT} "
			""
			"SELECT InsuredPartyID FROM PaymentsT WHERE ID = {INT} "
			""
			"SELECT AppliesT.Amount, AppliesT.DestID, AppliesT.SourceID, SourceItem.Type AS SourceType, "
			"DestItem.Type AS DestType, PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
			"PersonT.ID AS PatID " // (a.walling 2010-01-21 15:55) - PLID 37023
			"FROM AppliesT INNER JOIN LineItemT SourceItem ON AppliesT.SourceID = SourceItem.ID INNER JOIN LineItemT DestItem "
			"ON AppliesT.DestID = DestItem.ID INNER JOIN PersonT ON DestItem.PatientID = PersonT.ID "
			"WHERE AppliesT.ID = {INT} "
			""
			"SELECT ID, Amount FROM ApplyDetailsT WHERE ApplyID = {INT}",
			nPaymentID, nRefundID, nApplyID, nApplyID);
		if(!rs->eof) {
			PayInsuredPartyID = AdoFldLong(rs, "InsuredPartyID",-1);
			PayLocationID = AdoFldLong(rs, "LocationID",-1);
		}

		rs = rs->NextRecordset(NULL);

		if(!rs->eof) {
			RefInsuredPartyID = AdoFldLong(rs, "InsuredPartyID",-1);
		}

		if(RefInsuredPartyID != PayInsuredPartyID) {
			MsgBox("The payment you are applying to has a different insurance responsibility than your refund.\n"
				"The apply will not be made.");
			// a.walling 4/14/06 Instead of a FullRefresh, a QuickRefresh will suffice
			// FullRefresh();
			QuickRefresh();
			CalculateTotals();
			return;
		}

		//load the dollar amounts
		long nDestID = -1;
		//TES 7/10/2008 - PLID 30673 - For auditing, we also need the source ID, source and dest types, and patient name
		long nSourceID = -1, nSourceType = -1, nDestType = -1, nPatID = -1;
		CString strPatName;

		rs = rs->NextRecordset(NULL);

		if(!rs->eof) {
			cyApplyAmount = AdoFldCurrency(rs, "Amount",COleCurrency(0,0));
			nDestID = AdoFldLong(rs, "DestID",-1);
			nSourceID = AdoFldLong(rs, "SourceID");
			nSourceType = AdoFldLong(rs, "SourceType");
			nDestType = AdoFldLong(rs, "DestType");
			strPatName = AdoFldString(rs, "PatName");
			nPatID = AdoFldLong(rs, "PatID");
		}

		COleCurrency cyAmount, cyOutgoing, cyIncoming;
		if (!GetPayAdjRefTotals(nRefundID, m_iCurPatient, cyAmount, cyOutgoing, cyIncoming)) {
			MsgBox("The payment you have chosen to apply from has been removed from this screen. Practice will now requery this patient's financial information.");
			FullRefresh();
			CalculateTotals();
			return;
		}
		cyRefundAmount = cyAmount - cyOutgoing + cyIncoming;

		COleCurrency cyRefundApplyAmount = -cyRefundAmount;

		//if we are not auto-applying, bring up the apply window
		if(!bAutoApply) {
			CFinancialApply dlg(this);
			dlg.m_PatientID = m_iCurPatient;
			dlg.m_nResponsibility = 0;
			dlg.m_cyNetCharges = cyApplyAmount;
			dlg.m_cyNetPayment = cyRefundAmount;
			dlg.m_boShowIncreaseCheck = FALSE;
			dlg.m_boShowAdjCheck = FALSE;
			dlg.m_boApplyToPay = TRUE;

			GetMainFrame()->DisableHotKeys();

			if (IDCANCEL == dlg.DoModal() || (!dlg.m_boZeroAmountAllowed && dlg.m_cyApplyAmount == COleCurrency(0,0))) {
				GetMainFrame()->EnableHotKeys();
				// a.walling 4/14/06 Instead of a FullRefresh, a QuickRefresh will suffice
				// FullRefresh();
				QuickRefresh();
				CalculateTotals();
				return;
			}
			GetMainFrame()->EnableHotKeys();

			cyRefundApplyAmount = -dlg.m_cyApplyAmount;
		}

		CString strMsg;

		//if the payment is going to be completely unapplied, say so
		if(cyRefundApplyAmount == cyApplyAmount) {
			strMsg.Format("You are about to refund %s of an applied payment.\n"
				"The payment will be completely unapplied from this charge and the refund will then be applied to the payment.\n"
				"Are you sure you wish to do this?",FormatCurrencyForInterface(cyApplyAmount));
		}
		else if(cyRefundApplyAmount > cyApplyAmount) {
			cyRefundApplyAmount = cyApplyAmount;
			strMsg.Format("You are about to apply %s of a %s refund to an applied payment.\n"
				"The payment will be completely unapplied from this charge and the refund will then be applied to the payment.\n"
				"Are you sure you wish to do this?",FormatCurrencyForInterface(cyRefundApplyAmount),FormatCurrencyForInterface(-cyRefundAmount));
		}
		else {			
			//otherwise explain that it will be partially unapplied
			strMsg.Format("You are attempting to apply a %s refund to %s of an applied payment.\n"
				"%s of the payment will be unapplied from this charge and the refund will then be applied to this portion of the payment.\n"
				"Are you sure you wish to do this?",FormatCurrencyForInterface(-cyRefundAmount),FormatCurrencyForInterface(cyApplyAmount),FormatCurrencyForInterface(cyRefundApplyAmount));
		}

		if(IDNO == MessageBox(strMsg,"Practice",MB_ICONQUESTION|MB_YESNO)) {

			//ask if they want to delete it
			if(IDYES == MessageBox("You have chosen not to apply the refund. Would you like to delete the refund you have just saved?","Practice",MB_ICONQUESTION|MB_YESNO)) {
				DeletePayment(nRefundID);
			}

			// a.walling 4/14/06 Instead of a FullRefresh, a QuickRefresh will suffice
			// FullRefresh();
			QuickRefresh();
			CalculateTotals();
			return;
		}

		//unapply
		if(cyRefundApplyAmount >= cyApplyAmount) {
			//delete the apply completely
			DeleteApply(nApplyID);
		}
		else {
			//unapply the amount of the refund we are about to apply
			COleCurrency cyLeftToUnapply = cyRefundApplyAmount;			

			rs = rs->NextRecordset(NULL);

			CString strSqlBatch;
			CNxParamSqlArray aryParams;

			while(!rs->eof && cyLeftToUnapply > COleCurrency(0,0)) {
				COleCurrency cyApplyDetailAmt = AdoFldCurrency(rs, "Amount",COleCurrency(0,0));
				long nApplyDetailID = AdoFldLong(rs, "ID");
				if(cyApplyDetailAmt > COleCurrency(0,0)) {
					if(cyApplyDetailAmt > cyLeftToUnapply) {
						// (j.jones 2009-12-29 14:25) - PLID 27237 - parameterized and batched
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE ApplyDetailsT SET Amount = Amount - Convert(money,{STRING}) WHERE ID = {INT}",FormatCurrencyForSql(cyLeftToUnapply),nApplyDetailID);
						cyLeftToUnapply = COleCurrency(0,0);
					}
					else if(cyApplyDetailAmt <= cyLeftToUnapply) {
						// (j.jones 2009-12-29 14:25) - PLID 27237 - parameterized and batched
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM ApplyDetailsT WHERE ID = {INT}",nApplyDetailID);
						cyLeftToUnapply -= cyApplyDetailAmt;
					}
				}
				rs->MoveNext();
			}
			rs->Close();

			// (j.jones 2009-12-29 14:25) - PLID 27237 - parameterized and batched
			AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE AppliesT SET Amount = Amount - Convert(money,{STRING}) WHERE ID = {INT}",FormatCurrencyForSql(cyRefundApplyAmount),nApplyID);

			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);

			//TES 7/10/2008 - PLID 30673 - Audit the unapply, for both the source and destination.
			long nAuditTransactionID = -1;
			if(nSourceID != -1) {//TES 7/10/2008 - PLID 30673 - This would mean the apply never existed.
				try {
					CString strAmount = FormatCurrencyForInterface(cyRefundApplyAmount, TRUE, TRUE);
					strAmount.Replace("(","");
					strAmount.Replace(")","");
					AuditEventItems aeiSourceItem;
					switch(nSourceType) {
					case 1:
						aeiSourceItem = aeiPaymentUnapplied;
						break;
					case 2:
						aeiSourceItem = aeiAdjustmentUnapplied;
						break;
					case 3:
						aeiSourceItem = aeiRefundUnapplied;
						break;
					default:
						AfxThrowNxException("Bad Source Type %li found while auditing unapply!", nSourceType);
						break;
					}
					nAuditTransactionID = BeginAuditTransaction();
					AuditEvent(nPatID, strPatName, nAuditTransactionID, aeiSourceItem, nSourceID, "", strAmount + " Unapplied", aepHigh, aetDeleted);

					//TES 7/10/2008 - PLID 30673 - Now the destination
					AuditEventItems aeiDestItem;
					switch(nDestType) {
					case 1:
						aeiDestItem = aeiItemUnappliedFromPayment;
						break;
					case 2:
						aeiDestItem = aeiItemUnappliedFromAdjustment;
						break;
					case 3:
						aeiDestItem = aeiItemUnappliedFromRefund;
						break;
					case 10:
						aeiDestItem = aeiItemUnappliedFromCharge;
						break;
					default:
						AfxThrowNxException("Bad Dest Type %li found while auditing unapply!", nDestType);
						break;
					}
					AuditEvent(nPatID, strPatName, nAuditTransactionID, aeiDestItem, nDestID, "", "Applies reduced by " + strAmount, aepHigh, aetDeleted);
					CommitAuditTransaction(nAuditTransactionID);
				}NxCatchAllSilentCallThrow(if(nAuditTransactionID != -1) {RollbackAuditTransaction(nAuditTransactionID);});
			}
		}

		//now apply
		ApplyPayToBill(nRefundID, m_iCurPatient, -cyRefundApplyAmount, "Payment", nPaymentID, -1 /*Patient Resp*/, TRUE);

		//lastly, make a adjustment for the amount refunded
		if(cyRefundApplyAmount > COleCurrency(0,0) &&
			IDYES == MessageBox("Would you like to adjust off the remaining balance of this charge?","Practice",MB_YESNO|MB_ICONQUESTION))
		{
			if(CheckCurrentUserPermissions(bioAdjustment,sptCreate)) {

				COleCurrency cyAmountToAdjust = cyRefundApplyAmount;

				CPaymentDlg dlg(this);
				dlg.m_PatientID = m_iCurPatient;
				dlg.m_iDefaultInsuranceCo = PayInsuredPartyID;
				dlg.m_iDefaultPaymentType = 1;
				dlg.m_cyFinalAmount = -cyAmountToAdjust;
				dlg.m_cyMaxAmount = -cyAmountToAdjust;
				dlg.m_DefLocationID = PayLocationID;
				if (dlg.DoModal(__FUNCTION__, __LINE__) != IDCANCEL) {

					if(PayInsuredPartyID > 0){
						//(e.lally 2007-03-30) PLID 25263 - Switched to new general Apply Pay To Bill function
						ApplyPayToBill(dlg.m_varPaymentID.lVal,m_iCurPatient,dlg.m_cyFinalAmount,"Charge",nDestID,PayInsuredPartyID,FALSE, FALSE,FALSE,FALSE,FALSE);
					}
					else
						ApplyPayToBill(dlg.m_varPaymentID.lVal,m_iCurPatient,dlg.m_cyFinalAmount,"Charge",nDestID, -1 /*Patient Resp*/, FALSE);					
				}
			}
		}

		// a.walling 4/14/06 Instead of a FullRefresh, a QuickRefresh will suffice
		// FullRefresh();
		QuickRefresh();
		CalculateTotals();

	}NxCatchAll("Error applying refund to applied payment.");
}

void CFinancialDlg::OnShowQuotes() 
{
	// (j.jones 2015-03-16 14:46) - PLID 64926 - this dialog is now a reference, not a pointer
	if (m_dlgQuotes.GetSafeHwnd() == NULL)
	{
		m_dlgQuotes.Create();
		m_dlgQuotes.ShowWindow(SW_SHOW);
	}
	else {
		if (!m_dlgQuotes.IsWindowVisible()) {
			m_dlgQuotes.ShowWindow(SW_SHOW);
		}
	}
}

void CFinancialDlg::TryShowPackageWindow()
{
	try {

		BOOL bShowPackages = GetRemotePropertyInt("PopupPackageWindow",0,0,GetCurrentUserName(),TRUE);
		BOOL bShowQuotes = GetRemotePropertyInt("PopupQuoteWindow",0,0,GetCurrentUserName(),TRUE);

		if(!bShowPackages && !bShowQuotes) {

			// (j.jones 2006-05-03 11:44) - PLID 20145 - they might have had the windows manually opened,
			// in which case we allow them to keep it open

			//refresh totals
			if (m_dlgPackages.GetSafeHwnd()) {
				if (m_dlgPackages.IsWindowVisible()) {
					m_dlgPackages.Requery();
				}
			}

			if (m_dlgQuotes.GetSafeHwnd()) {
				if (m_dlgQuotes.IsWindowVisible()) {
					m_dlgQuotes.Requery();
				}
			}

			return;
		}

		//If they don't have this preference, we want to keep the window up until they leave the tab
		HideToolWindows();

		BOOL bDisplayPackageWindow = FALSE;
		BOOL bDisplayQuoteWindow = FALSE;

		// (j.jones 2009-12-29 11:01) - PLID 27237 - replaced ReturnsRecords with a combined, parameterized recordset
		if(bShowPackages || bShowQuotes) {			
			_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), ""
				// (j.jones 2008-04-28 16:01) - PLID 29615 - filter out inactive packages, since that's what the popup does
				// (j.jones 2008-05-30 12:39) - PLID 28898 - ensured we ignore charges that have an outside fee with no practice fee
				"SELECT TOP 1 PackagesT.QuoteID FROM "
				"	(SELECT PackagesT.QuoteID, TotalAmount, CurrentAmount, Type, "
				"	(CASE WHEN Type = 1 THEN Convert(float,PackagesT.TotalCount) WHEN Type = 2 THEN PackageChargesQ.MultiUseTotalCount ELSE 0 END) AS TotalCount, "
				"	(CASE WHEN Type = 1 THEN Convert(float,PackagesT.CurrentCount) WHEN Type = 2 THEN PackageChargesQ.MultiUseCurrentCount ELSE 0 END) AS CurrentCount "
				"	FROM PackagesT "
				"	LEFT JOIN "
				"		(SELECT ChargesT.BillID, Sum(Quantity) AS MultiUseTotalCount, Sum(PackageQtyRemaining) AS MultiUseCurrentCount "
				"		FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE Deleted = 0 "
				"		AND (LineItemT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0)) "
				"		GROUP BY BillID "
				"		) AS PackageChargesQ ON PackagesT.QuoteID = PackageChargesQ.BillID "
				"	) AS PackagesT "
				"INNER JOIN BillsT ON PackagesT.QuoteID = BillsT.ID WHERE "
				"BillsT.Deleted = 0 AND BillsT.PatientID = {INT} AND BillsT.Active = 1 AND CurrentCount > 0 "
				""
				// (j.jones 2008-04-28 16:01) - PLID 29615 - filter out inactive quotes and package quotes,since that's what the popup does
				"SELECT TOP 1 BillsT.ID FROM BillsT "
				"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
				"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
				"WHERE EntryType = 2 AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 AND BillsT.Active = 1 "
				"AND BillsT.ID NOT IN (SELECT QuoteID FROM PackagesT) "
				"AND BillsT.ID NOT IN (SELECT QuoteID FROM BilledQuotesT WHERE BillID IN (SELECT ID FROM BillsT WHERE EntryType = 1 AND Deleted = 0)) "
				"AND BillsT.PatientID = {INT}", m_iCurPatient, m_iCurPatient);
			if(!rs->eof && bShowPackages) {
				bDisplayPackageWindow = TRUE;
			}

			rs = rs->NextRecordset(NULL);

			if(!rs->eof && bShowQuotes) {
				bDisplayQuoteWindow = TRUE;
			}
			rs->Close();
		}

		//check packages first. If a package exists, we will show that window.
		if(bDisplayPackageWindow) {
			OnShowPackages();
			return;
		}

		//otherwise, show quotes
		if(bDisplayQuoteWindow) {
			OnShowQuotes();
			return;
		}

	}NxCatchAll("Error displaying package/quote window.");	
}

// (j.jones 2015-03-16 15:08) - PLID 64926 - renamed to reflect that this
// hides all billing tool windows
void CFinancialDlg::HideToolWindows()
{
	try {

		if (m_dlgPackages.GetSafeHwnd())
			m_dlgPackages.ShowWindow(SW_HIDE);

		if (m_dlgQuotes.GetSafeHwnd())
			m_dlgQuotes.ShowWindow(SW_HIDE);

		if (m_dlgSearchBillingTab.GetSafeHwnd())
			m_dlgSearchBillingTab.ShowWindow(SW_HIDE);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
void CFinancialDlg::InvokePostingDlg(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bPostToWholeBill)
{
	try {

		if(pRow == NULL) {
			return;
		}

		CString strLineType = VarString(pRow->GetValue(btcLineType),"");

		long nBillID = -1;
		long nOnlyShowChargeID = -1;

		if(strLineType == "Charge") {
			// (j.jones 2009-12-28 17:13) - PLID 27237 - pulled from the screen, not data
			nBillID = VarLong(pRow->GetValue(m_nBillIDColID),-1);
			if(bPostToWholeBill) {

				// (j.jones 2011-09-13 15:37) - PLID 44887 - you can't apply to this bill
				// if it only has original/void charges
				if(IsVoidedBill(nBillID)) {
					AfxMessageBox("This bill has been corrected, and can not be modified.");
					return;
				}
				else if(!DoesBillHaveUncorrectedCharges(nBillID)) {
					AfxMessageBox("All charges in this bill have been corrected, and can not be modified.");
					return;
				}
			}
			else {

				// (j.jones 2012-08-16 10:25) - PLID 52162 - now we tell the posting dialog to filter
				// on one charge by sending in nOnlyShowChargeID
				nOnlyShowChargeID = VarLong(pRow->GetValue(btcChargeID),-1);

				// (j.jones 2011-09-13 15:37) - PLID 44887 - you can't apply to this charge
				// if any charge in it is original or void
				LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nOnlyShowChargeID);
				if(licsStatus == licsOriginal) {
					AfxMessageBox("This charge has been corrected, and can not be modified.");
					return;
				}
				else if(licsStatus == licsVoid) {
					AfxMessageBox("This charge is a void charge from an existing correction, and can not be modified.");
					return;
				}
			}
		}
		else if(strLineType == "Bill") {
			nBillID = VarLong(pRow->GetValue(m_nBillIDColID),-1);

			// (j.jones 2011-09-13 15:37) - PLID 44887 - you can't apply to this bill
			// if it only has original/void charges
			if(IsVoidedBill(nBillID)) {
				AfxMessageBox("This bill has been corrected, and can not be modified.");
				return;
			}
			else if(!DoesBillHaveUncorrectedCharges(nBillID)) {
				AfxMessageBox("All charges in this bill have been corrected, and can not be modified.");
				return;
			}

			if(!bPostToWholeBill) {
				//shouldn't be possible
				bPostToWholeBill = TRUE;
			}
		}
		else {
			//not supported
			return;
		}	

		//the user must have access to make and edit payments to enter
		if (!CheckCurrentUserPermissions(bioPayment,sptCreate))
			return;

		long nPatientID = m_iCurPatient;

		CFinancialLineItemPostingDlg dlg(this);

		// (j.jones 2012-08-16 10:20) - PLID 52162 - changed the way we launch line item posting to always
		// require the bill ID, and only send the chargeID if filtering on just one charge

		// (j.jones 2013-03-25 17:35) - PLID 55686 - Line item posting will default to the highest insurance
		// resp. on this charge or bill, whoever owes the most becomes the default insured party.
		// This calculation has been moved to be inside the dialog itself, and we tell the dialog to do it
		// by passing in -2.
		// (j.jones 2014-06-27 10:43) - PLID 62548 - added PayType, always medical here
		dlg.DoModal(eMedicalPayment, nBillID, nOnlyShowChargeID, -1, -2);

		BeginWaitCursor();
		RedrawBill(nBillID);
		RedrawAllPayments();
		RefreshList();
		CalculateTotals();
		EndWaitCursor();		

	}NxCatchAll("Error creating posting screen.");
}

BOOL CFinancialDlg::DoesLineHaveNotes(CString strLineType, long nItemID)
{
	// (j.jones 2009-12-29 11:01) - PLID 27237 - replaced ReturnsRecords calls with parameterized recordsets

	// See if we are on a bill or a line item
	if (strLineType == CString("Bill")) {
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM Notes WHERE BillID = {INT}", nItemID);
		return !rs->eof;
	}
	else if(strLineType == CString("Charge")) {
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM Notes WHERE LineItemID = {INT}", nItemID);
		return !rs->eof;
	}
	else if(strLineType == CString("Applied") || strLineType == CString("InsuranceApplied") || strLineType == CString("AppliedPayToPay")) {
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM Notes WHERE LineItemID = {INT}", nItemID);
		return !rs->eof;
	}
	else {
		//a pay/adj/ref
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM Notes WHERE LineItemID = {INT}", nItemID);
		return !rs->eof;
	}

	return FALSE;
}

void CFinancialDlg::OpenPayment(int nPayID)
{
	if (!CheckCurrentUserPermissions(bioPayment,sptRead))
		return;

	CPaymentDlg dlg(this);
	dlg.m_PatientID = m_iCurPatient;
	dlg.m_varPaymentID = _variant_t((long)nPayID);

	if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
		FillTabInfo();
	}
}

//TES 7/1/2008 - PLID 26143 - Added a parameter to exchange the product, rather than returning it.
// (j.jones 2010-06-07 14:54) - PLID 39042 - converted to take in a row pointer
void CFinancialDlg::ReturnProduct(NXDATALIST2Lib::IRowSettingsPtr pRow, BOOL bExchangingProduct)
{
	try {

		if(pRow == NULL) {
			return;
		}

		//get the ID of this charge
		long ChargeID = VarLong(pRow->GetValue(btcChargeID),-1);

		// (j.jones 2011-09-13 15:37) - PLID 44887 - you can't delete this charge
		// if any charge in it is original or void
		LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(ChargeID);
		if(licsStatus == licsOriginal) {
			AfxMessageBox("This charge has been corrected, and can not be modified.");
			return;
		}
		else if(licsStatus == licsVoid) {
			AfxMessageBox("This charge is a void charge from an existing correction, and can not be modified.");
			return;
		}

		if (!CheckCurrentUserPermissions(bioRefund,sptCreate))
			return;

		//TES 7/25/2014 - PLID 63049 - Don't allow them to delete chargeback charges
		if (DoesChargeHaveChargeback(CSqlFragment("ChargesT.ID = {INT}", ChargeID))) {
			MessageBox("This charge is associated with at least one Chargeback, and cannot be returned or exchanged. "
				"In order to make changes to this charge, you must first right-click on any associated Chargebacks, and select 'Undo Chargeback.'");
			return;
		}

		// (a.walling 2009-12-22 17:58) - PLID 7002 - Maintain only one instance of a bill
		if (bExchangingProduct && GetMainFrame()->IsBillingModuleOpen(true)) {
			return;
		}

		//get the quantity billed
		double dblQtyBilled = 1.0;
		// (j.jones 2008-06-05 15:52) - PLID 27110 - also find out how many product items are linked to this charge,
		// OR part of a linked case history
		// (j.jones 2008-09-16 12:06) - PLID 31382 - fixed the case history lookup to match by product ID as well		
		// (j.jones 2009-12-28 17:13) - PLID 27237 - combined recordsets
		// (j.jones 2013-07-23 09:25) - PLID 57676 - fixed the usage of BilledCaseHistoriesT
		long nQtySerialized = 0;		
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Coalesce(ReturnedProductsQ.TotalReturned, 0) AS TotalReturned, "
			"ChargesT.Quantity, "
			"(SELECT Count(ID) AS CountID FROM ProductItemsT WHERE "
			"	(ProductItemsT.SerialNum Is Not Null OR ProductItemsT.ExpDate Is Not Null) "
			"	AND ProductItemsT.Deleted = 0 AND (ProductItemsT.ID IN "
			"	(SELECT ProductItemID FROM ChargedProductItemsT WHERE ChargeID = {INT} "
			"	OR (ChargedProductItemsT.CaseHistoryDetailID Is Not Null "
			"		AND ChargedProductItemsT.CaseHistoryDetailID IN ( "
			"			SELECT CaseHistoryDetailsT.ID FROM CaseHistoryDetailsT "
			"			INNER JOIN BilledCaseHistoriesT ON CaseHistoryDetailsT.CaseHistoryID = BilledCaseHistoriesT.CaseHistoryID "
			"			INNER JOIN ChargesT ON BilledCaseHistoriesT.BillID = ChargesT.BillID "
			"			WHERE ChargesT.ID = {INT}) "
			"		AND ProductItemsT.ProductID = (SELECT ServiceID FROM ChargesT WHERE ID = {INT}) "
			"	)) "
			")) AS QtySerialized "
			"FROM ChargesT "
			"LEFT JOIN (SELECT Sum(QtyReturned) AS TotalReturned, ChargeID "
			"	FROM ReturnedProductsT "
			"	GROUP BY ChargeID) AS ReturnedProductsQ ON ChargesT.ID = ReturnedProductsQ.ChargeID "
			"WHERE ChargesT.ID = {INT} "
			"GROUP BY ReturnedProductsQ.TotalReturned, ChargesT.ID, ChargesT.Quantity", ChargeID, ChargeID, ChargeID, ChargeID);
		if(!rs->eof) {
			dblQtyBilled = AdoFldDouble(rs, "Quantity",1.0);
			nQtySerialized = AdoFldLong(rs, "QtySerialized", 0);

			// (j.jones 2005-05-06 12:11) - check to see how much has previously been returned
			double dblQtyAlreadyReturned = AdoFldDouble(rs, "TotalReturned",1.0);
			if(dblQtyAlreadyReturned > 0) {
				if(dblQtyAlreadyReturned >= dblQtyBilled) {
					//all the products billed have already been returned, so warn and quit
					AfxMessageBox("This patient has already returned all the items on this charge.");
					return;
				}
				else {
					//some of the products billed have already been returned, so let them know this
					//so they aren't confused as to why they can't return the entire amount billed
					CString str;
					str.Format("This patient has already returned %g of the %g items billed on this charge.\n"
						"You will only be able to return up to %g remaining item(s).",dblQtyAlreadyReturned,dblQtyBilled,
						dblQtyBilled - dblQtyAlreadyReturned);
					AfxMessageBox(str);

					//and now reduce the dblQtyBilled so the remainder of this function uses the new number as an upper limit
					dblQtyBilled -= dblQtyAlreadyReturned;
				}
			}
		}
		rs->Close();

		// (j.jones 2005-04-15 09:03) - first ask for the amount returned
		double dblQtyToRemove = dblQtyBilled;		
		if(dblQtyBilled > 1.0) {
			CString strCount;
			strCount.Format("%g", dblQtyBilled);
			// (j.jones 2009-03-10 16:19) - PLID 33441 - disable non-numerics (decimals) when serialized products exist
			int strRet = InputBox(this, "Enter the quantity to be returned:",strCount, "", false, false, 0, (nQtySerialized > 0));
			while(strRet != IDOK || atof(strCount) > dblQtyBilled || atof(strCount) == 0) {
				if(strRet == IDCANCEL) {
					return;
				}
				if(atof(strCount) == 0) {
					AfxMessageBox("Please enter a quantity greater than zero.");
				}
				else if(atof(strCount) > dblQtyBilled) {
					CString str;
					str.Format("Please enter a quantity no greater than %g",dblQtyBilled);
					AfxMessageBox(str);
				}

				strCount.Format("%g", dblQtyBilled);
				// (j.jones 2009-03-10 16:19) - PLID 33441 - disable non-numerics (decimals) when serialized products exist
				strRet = InputBox(this, "Enter the quantity to be returned:",strCount, "", false, false, 0, (nQtySerialized > 0));
			}
			dblQtyToRemove = atof(strCount);
		}

		// (j.jones 2005-04-15 09:03) - now ask for the amount to be put back into stock
		// (can be different from amount returned because some might be opened or partially used)
		double dblQtyToReturn = dblQtyToRemove;		
		if(dblQtyToRemove > 1.0) {
			CString strCount;
			strCount.Format("%g", dblQtyToRemove);
			// (j.jones 2009-03-10 16:19) - PLID 33441 - disable non-numerics (decimals) when serialized products exist
			int strRet = InputBox(this, "Enter the quantity to be placed back into stock:", strCount, "", false, false, 0, (nQtySerialized > 0));
			while(strRet != IDOK || atof(strCount) > dblQtyBilled || (atof(strCount) == 0 && strCount != "0" && strCount != "0.0")) {

				if(strRet == IDCANCEL) {
					return;
				}
				// (j.jones 2009-03-02 16:16) - PLID 33052 - zero is allowed, so I properly
				// converted this check into a validation check
				if(atof(strCount) == 0 && strCount != "0" && strCount != "0.0") {
					AfxMessageBox("Please enter a valid quantity.");
				}
				else if(atof(strCount) > dblQtyToRemove) {
					CString str;
					str.Format("Please enter a quantity no greater than %g",dblQtyToRemove);
					AfxMessageBox(str);
				}

				strCount.Format("%g", dblQtyToRemove);
				// (j.jones 2009-03-10 16:19) - PLID 33441 - disable non-numerics (decimals) when serialized products exist
				strRet = InputBox(this, "Enter the quantity to be placed back into stock:",strCount, "", false, false, 0, (nQtySerialized > 0));
			}
			dblQtyToReturn = atof(strCount);
		}
		else if(dblQtyToRemove == 1.0) {
			if(IDYES == MessageBox("Would you like to place this item back into stock?","Practice",MB_YESNO|MB_ICONQUESTION)) {
				dblQtyToReturn = 1.0;
			}
			else {
				dblQtyToReturn = 0.0;
			}
		}

		CWaitCursor pWait1;

		//now calculate the dollar amount this represents
		COleCurrency cyTotalCharge = COleCurrency(0,0);
		COleCurrency cyNewTotal = COleCurrency(0,0);
		long LocationID = -1;
		long ServiceID = -1;
		CString strLocationName;
		// (j.gruber 2009-03-17 15:50) - PLID 33360 - changed discount structure
		// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
		rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT dbo.GetChargeTotal(ChargesT.ID) AS OldTotal, Round(Convert(money,((([Amount]*([Quantity]-{DOUBLE})*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))) + "
			"(([Amount]*([Quantity]-{DOUBLE})*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))*(ChargesT.TaxRate-1)) + "
			"(([Amount]*([Quantity]-{DOUBLE})*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN CPTMultiplier3 Is Null THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN [TotalDiscount] Is Null THEN 0 ELSE [TotalDiscount] END))*(ChargesT.TaxRate2-1)) "
			")),2) AS NewTotal, ServiceID, LocationID, LocationsT.Name AS LocationName "
			"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID "
			"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID  "
			"LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID  "
			"WHERE LineItemT.Deleted = 0 "
			"AND ChargesT.ID = {INT}",dblQtyToRemove,dblQtyToRemove,dblQtyToRemove,ChargeID);
		if(!rs->eof) {
			cyTotalCharge = AdoFldCurrency(rs, "OldTotal",COleCurrency(0,0));
			cyNewTotal = AdoFldCurrency(rs, "NewTotal",COleCurrency(0,0));
			ServiceID = AdoFldLong(rs, "ServiceID",-1);
			LocationID = AdoFldLong(rs, "LocationID",-1);
			strLocationName = AdoFldString(rs, "LocationName","");
		}
		rs->Close();

		// (j.jones 2008-06-05 15:50) - PLID 27110 - prompt for which serial numbers are being put back into stock
		CArray<long,long> aryProductItemsToAdjust;
		if(dblQtyToReturn > 0.0 && nQtySerialized > 0) {

			//shouldn't be common, maybe not even possible, for a charge to
			//not have enough serialized items to match its quantity,
			//but if not, ensure we don't require they return more than they have
			long nAmtNeeded = 0;
			if(dblQtyToReturn > nQtySerialized) {
				nAmtNeeded = nQtySerialized;
			}
			else {
				nAmtNeeded = (long)dblQtyToReturn;
			}

			BOOL bContinue = TRUE;
			while(bContinue) {

				bContinue = FALSE;

				CProductItemsDlg dlg(this);
				dlg.m_EntryType = PI_RETURN_DATA;
				dlg.m_strOverrideSelectQtyText = "Quantity to return";
				dlg.m_bDisallowQtyChange = TRUE;
				dlg.m_bAllowQtyGrow = FALSE;
				dlg.m_ProductID = ServiceID;
				dlg.m_nLocationID = LocationID;
				dlg.m_CountOfItemsNeeded = nAmtNeeded;
				// (j.jones 2013-07-23 09:25) - PLID 57676 - fixed the usage of BilledCaseHistoriesT
				dlg.m_strWhere.Format("(ProductItemsT.SerialNum Is Not Null OR ProductItemsT.ExpDate Is Not Null) "
					"AND ProductItemsT.Deleted = 0 "
					"AND ProductItemsT.ID IN ("
					"	SELECT ProductItemID FROM ChargedProductItemsT "
					"	WHERE ChargeID = %li "
					"	OR (ChargedProductItemsT.CaseHistoryDetailID Is Not Null "
					"		AND ChargedProductItemsT.CaseHistoryDetailID IN ("
					"			SELECT CaseHistoryDetailsT.ID FROM CaseHistoryDetailsT "
					"			INNER JOIN BilledCaseHistoriesT ON CaseHistoryDetailsT.CaseHistoryID = BilledCaseHistoriesT.CaseHistoryID "
					"			INNER JOIN ChargesT ON BilledCaseHistoriesT.BillID = ChargesT.BillID "
					"			WHERE ChargesT.ID = %li) "
					"		AND ProductItemsT.ProductID = (SELECT ServiceID FROM ChargesT WHERE ID = %li) "
					"	)"
					")", ChargeID, ChargeID, ChargeID);
				if(dlg.DoModal() == IDCANCEL) {
					if(IDYES == MessageBox("You cannot return this product without selecting serial numbers / expiration dates to return.\n\n"
						"Do you wish to cancel returning this product?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
							return;
					}
					else {
						bContinue = TRUE;
						continue;
					}
				}
				else {

					//grab the product item IDs to return
					for(int i=0;i<dlg.m_adwProductItemIDs.GetSize();i++) {								
						aryProductItemsToAdjust.Add((long)dlg.m_adwProductItemIDs.GetAt(i));
					}
				}
			}
		}

		//calculate the amount that needs refunded (could be anywhere from nothing to the whole amount)
		COleCurrency cyAmountApplied = COleCurrency(0,0);
		// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
		rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Sum(Amount) AS ApplyAmount FROM AppliesT WHERE DestID = {INT}", ChargeID);
		if(!rs->eof) {
			cyAmountApplied = AdoFldCurrency(rs, "ApplyAmount",COleCurrency(0,0));
		}
		rs->Close();

		COleCurrency cyAmountToUnapply = COleCurrency(0,0);
		if(cyAmountApplied > cyNewTotal) {
			cyAmountToUnapply = cyAmountApplied - cyNewTotal;
		}

		//warn them about what's going to happen
		//TES 7/1/2008 - PLID 26143 - The process will be slightly different depending on whether they're returning or exchanging
		// the product, so modify the warning accordingly.
		CString strWarning;
		CString strRefund;
		CString strAdjustAndBill;
		if(!bExchangingProduct) {
			strRefund = ", prompt for a refund to be entered";
		}
		if(bExchangingProduct) {
			strAdjustAndBill = "prompt for an adjustment to be entered, then prompt you to enter a bill for the new product the patient is receiving";
		}
		else {
			strAdjustAndBill = "and then prompt for an adjustment to be entered";
		}

		if(cyAmountToUnapply > COleCurrency(0,0)) {
			strWarning.Format("You are about to return %g of these items, which will unapply %s from this charge.\n"
				"Practice will then add %g back into inventory" + strRefund + ", "
				+ strAdjustAndBill + ".\n\n"
				"Are you sure you wish to do this?",dblQtyToRemove,FormatCurrencyForInterface(cyAmountToUnapply,TRUE,TRUE),dblQtyToReturn);
		}
		else {
			strWarning.Format("You are about to return %g of these items. Practice will add %g back into inventory, "
				+ strAdjustAndBill + ".\n\n"
				"Are you sure you wish to do this?",dblQtyToRemove,dblQtyToReturn);
		}

		if(IDNO == MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNO)) {
			return;
		}

		// (j.jones 2007-03-27 08:57) - PLID 25120 - give another warning if the location the products
		// were removed from is different from the location we're returning to
		if(LocationID != GetCurrentLocationID() && dblQtyToReturn > 0.0) {

			//first ensure the item is trackable in some way AND billable for the current location,
			//if not then don't even give them the option to return to it
			// (j.jones 2009-12-29 11:01) - PLID 27237 - replaced ReturnsRecords with a parameterized recordset
			_RecordsetPtr rsProduct = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 ProductID FROM ProductLocationInfoT "
				"WHERE Billable = 1 AND TrackableStatus <> 0 AND LocationID = {INT} AND ProductID = {INT}", LocationID, ServiceID);
			if(!rsProduct->eof) {
				//it is billable and trackable (either orders or quantity) so we can return it locally

				CString strWarning;
				strWarning.Format("You are about to place inventory from another location back into stock.\n"
					"Do you want to add the stock back at the current location instead?\n"
					"(If so, any refunds or adjustments will also default to use the current location.)\n\n"
					"'Yes' will add the inventory into stock at the current location, '%s'.\n"
					"'No' will add the inventory into stock at the bill location, '%s'.", GetCurrentLocationName(), strLocationName);
				int nResult = MessageBox(strWarning, "Practice", MB_ICONQUESTION|MB_YESNOCANCEL);
				if(nResult == IDCANCEL)
					return;
				else if(nResult == IDYES)
					LocationID = GetCurrentLocationID();
				//otherwise keep the same LocationID
			}
			rsProduct->Close();
		}

		// (j.jones 2008-06-05 16:16) - PLID 27110 - I converted this into a SQL batch
		CString strSqlBatch;
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewAdjustmentID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewReturnedProductID INT");
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNewProductItemID INT");

		//make an adjustment to compensate for the return
		//remember to only adjust off dblQtyToReturn, which is the amount to put back in stock,
		//as opposed to dblQtyToRemove, which is the amount the patient is being credited for
		//TES 6/26/2008 - PLID 30522 - Assign this to the system "Returned by patient" category
		AddStatementToSqlBatch(strSqlBatch, "SET @nNewAdjustmentID = (SELECT COALESCE(Max(ID),0)+1 FROM ProductAdjustmentsT)");
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ProductAdjustmentsT "
			"(ID, ProductID, Date, Login, Quantity, Amount, LocationID, Notes, ProductAdjustmentCategoryID) "
			"SELECT @nNewAdjustmentID, %li, '%s', %li, %g, Convert(money,'%s'), %li, '%s', %li",
			ServiceID, FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), 
			GetCurrentUserID(), dblQtyToReturn, _Q(FormatCurrencyForSql(-(cyTotalCharge - cyNewTotal))), LocationID, _Q("Returned by patient " + GetExistingPatientName(m_iCurPatient)),
			InvUtils::g_nReturnedByPatientID);

		// (j.jones 2005-05-06 11:44) - PLID 16250 - at this point, store the fact that we have returned products,
		// store it with the quantity returned, which is not necessarily the quantity adjusted
		AddStatementToSqlBatch(strSqlBatch, "SET @nNewReturnedProductID = (SELECT COALESCE(Max(ID),0)+1 FROM ReturnedProductsT)");
		AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ReturnedProductsT "
			"(ID, ChargeID, DateReturned, QtyReturned, InvAdjID, UserID) "
			"VALUES (@nNewReturnedProductID, %li, '%s', %g, @nNewAdjustmentID, %li)",
			ChargeID, FormatDateTimeForSql(COleDateTime::GetCurrentTime(), dtoDate), dblQtyToRemove, GetCurrentUserID());

		// (j.jones 2008-06-05 16:22) - PLID 27110 - for each returned serialized item, insert a dummy record
		// in with the bill, and unlink the original serialized item
		for(int i=0;i<aryProductItemsToAdjust.GetSize();i++) 
		{	
			long nProductItemID = (long)(aryProductItemsToAdjust.GetAt(i));

			//the "ReturnedFrom" field will reference the item that we unlinked from the bill and put back into stock,
			//that way we will be able to know which item is the "dummy item" and what item it replaced
			//TES 6/18/2008 - PLID 29578 - Changed OrderID to OrderDetailID.
			// (j.jones 2009-03-09 13:05) - PLID 33096 - supported CreatingAdjustmentID 
			// (b.spivey, February 17, 2012) - PLID 48080 - Removed the ID in this insert statement, as it is now an identity. 
			// (c.haag 2015-10-26) - PLID 67401 - The dummy product item must have the same location as the charge. You can't
			// charge a patient at location A for a serialized product from location B's stock.
			AddStatementToSqlBatch(strSqlBatch, "INSERT INTO ProductItemsT (ProductID, SerialNum, ExpDate, OrderDetailID, LocationID, ReturnedFrom, CreatingAdjustmentID) "
				"SELECT %li, NULL, NULL, NULL, LocationID, %li, @nNewAdjustmentID FROM LineItemT WHERE ID = %d", ServiceID, nProductItemID, ChargeID);

			// (b.spivey, February 14, 2012) - PLID 48080 - To make the code changes minimal, I figured we could reuse the 
			//	@nNewproductItemID variable to grab the scope identity and then use it as before. 
			//create a new one
			AddStatementToSqlBatch(strSqlBatch, "SET @nNewProductItemID = Convert(int, SCOPE_IDENTITY())");

			//change all charged product items referencing this product to now reference the dummy one
			AddStatementToSqlBatch(strSqlBatch, "UPDATE ChargedProductItemsT SET ProductItemID = @nNewProductItemID "
				"WHERE ProductItemID = %li", nProductItemID);

			//change all allocations referencing this product to now reference the dummy one
			AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientInvAllocationDetailsT SET ProductItemID = @nNewProductItemID "
				"WHERE ProductItemID = %li", nProductItemID);

			//(j.camacho 9/19/2014) - plid 63364 - Update serialized items locations when returned to properly reflect on hand numbers
			//If its reached here and the location is different then we need to update the location of the original serialized products
			//this is not a transfer, its a return so lets update the locations accordingly and let the returns be recorded as they usually are.
			// (c.haag 2015-10-26) - PLID 67401 - Don't change the location until after we disconnect it from ChargedProductItemsT
			AddStatementToSqlBatch(strSqlBatch, "UPDATE ProductItemsT SET locationID =%li WHERE ID=%li", LocationID, nProductItemID);
		}

		//execute these changes, but get the @nNewReturnedProductID
		long nReturnedProductID = -1;
		rs = CreateRecordset(
			"SET NOCOUNT ON "
			"%s"
			"SELECT @nNewReturnedProductID AS ID "
			"SET NOCOUNT OFF ", strSqlBatch);
		if(!rs->eof) {
			nReturnedProductID = AdoFldLong(rs, "ID");
		}
		rs->Close();

		// (c.haag 2008-02-07 13:17) - PLID 28853 - Renamed from ChargeInventoryQuantity to EnsureInventoryTodoAlarms
		// because that's a closer description to what it actually does. Also removed unused quantity parameter.
		// (j.jones 2008-09-16 09:25) - PLID 31380 - EnsureInventoryTodoAlarms now supports multiple products,
		// though in this particular case, it really is only one product
		{
			CArray<long, long> aryProductIDs;
			aryProductIDs.Add(ServiceID);
			//TES 11/15/2011 - PLID 44716 - This function needs to know if we're in a transaction
			InvUtils::EnsureInventoryTodoAlarms(aryProductIDs, LocationID, false);
		}

		CDWordArray aryPaymentIDs;
		COleCurrency cyTotalRefund = COleCurrency(0,0);
		//make a refund for the necessary amount
		if(cyAmountToUnapply > COleCurrency(0,0)) {

			CWaitCursor pWait2;

			BOOL bInsurancePays = FALSE;

			//unapply payments if needed, and apply the refund
			COleCurrency cyRemainingAmountToUnapply = cyAmountToUnapply;

			long nAuditTransactionID = -1;
			try {

				//TES 7/10/2008 - PLID 30673 - For auditing purposes, we also need to track the source type and the patient name
				// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
				rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT AppliesT.ID, AppliesT.SourceID, PaymentsT.InsuredPartyID, AppliesT.Amount, "
					"AppliesT.InputDate, LineSource.Type AS SourceType, "
					"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS PatName, "
					"PersonT.ID AS PatID "  // (a.walling 2010-01-21 15:55) - PLID 37023
					"FROM AppliesT INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
					"INNER JOIN LineItemT LineSource ON PaymentsT.ID = LineSource.ID "
					"INNER JOIN PersonT ON LineSource.PatientID = PersonT.ID "
					"WHERE AppliesT.DestID = {INT} "
					"ORDER BY PaymentsT.InsuredPartyID ASC, AppliesT.InputDate DESC",ChargeID);

				CString strSqlBatch;
				CNxParamSqlArray aryParams;

				while(!rs->eof && cyRemainingAmountToUnapply > COleCurrency(0,0)) {
					COleCurrency cyApplyAmount = AdoFldCurrency(rs, "Amount",COleCurrency(0,0));
					COleCurrency cyPaymentAmountToUnapply = COleCurrency(0,0);
					//find how much to unapply, reduce our total
					if(cyApplyAmount > cyRemainingAmountToUnapply)
						cyPaymentAmountToUnapply = cyRemainingAmountToUnapply;
					else
						cyPaymentAmountToUnapply = cyApplyAmount;
					cyRemainingAmountToUnapply -= cyPaymentAmountToUnapply;

					long InsuredPartyID = AdoFldLong(rs, "InsuredPartyID",-1);
					if(InsuredPartyID > -1) {
						bInsurancePays = TRUE;
					}
					else {
						cyTotalRefund += cyPaymentAmountToUnapply;
						long nPayID = AdoFldLong(rs, "SourceID",-1);
						aryPaymentIDs.Add(nPayID);
					}

					long nApplyID = AdoFldLong(rs, "ID",-1);

					////////////////////////////////////////////////////
					// If this apply is less or equal to the amount we
					// want desired, unapply it in full
					if (cyApplyAmount <= cyPaymentAmountToUnapply) {
						DeleteApply(nApplyID, FALSE);
					}

					////////////////////////////////////////////////////
					// Otherwise, modify the apply amount
					else {
						//update the main apply
						// (j.jones 2009-12-29 14:31) - PLID 27237 - parameterized and batched
						AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE AppliesT SET Amount = Convert(money,{STRING}) WHERE ID = {INT}",
							FormatCurrencyForSql(cyApplyAmount - cyPaymentAmountToUnapply),nApplyID);

						//TES 7/10/2008 - PLID 30673 - Audit the unapply, for both the source and destination.

						CString strAmount = FormatCurrencyForInterface(cyPaymentAmountToUnapply, TRUE, TRUE);
						strAmount.Replace("(","");
						strAmount.Replace(")","");
						CString strPatName = AdoFldString(rs, "PatName");
						long nPatID = AdoFldLong(rs, "PatID");
						long nSourceType = AdoFldLong(rs, "SourceType");
						AuditEventItems aeiSourceItem;
						switch(nSourceType) {
						case 1:
							aeiSourceItem = aeiPaymentUnapplied;
							break;
						case 2:
							aeiSourceItem = aeiAdjustmentUnapplied;
							break;
						case 3:
							aeiSourceItem = aeiRefundUnapplied;
							break;
						default:
							AfxThrowNxException("Bad Source Type %li found while auditing unapply!", nSourceType);
							break;
						}
						if(nAuditTransactionID == -1) {
							nAuditTransactionID = BeginAuditTransaction();
						}
						AuditEvent(nPatID, strPatName, nAuditTransactionID, aeiSourceItem, AdoFldLong(rs, "SourceID"), "", strAmount + " Unapplied", aepHigh, aetDeleted);

						//TES 7/10/2008 - PLID 30673 - Now the destination, which we know is a charge.
						AuditEvent(nPatID, strPatName, nAuditTransactionID, aeiItemUnappliedFromCharge, ChargeID, "", "Applies reduced by " + strAmount, aepHigh, aetDeleted);

						//now update the apply details the same way
						// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
						_RecordsetPtr rsApply = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ApplyDetailsT.ID, ApplyDetailsT.Amount FROM ApplyDetailsT WHERE ApplyID = {INT} ORDER BY DetailID DESC", nApplyID);
						while(!rsApply->eof && cyPaymentAmountToUnapply != COleCurrency(0,0)) {

							COleCurrency cyDetail = AdoFldCurrency(rsApply, "Amount");
							long nApplyDetailID = AdoFldLong(rsApply, "ID");

							if (cyDetail <= cyPaymentAmountToUnapply) {
								// (j.jones 2009-12-29 14:31) - PLID 27237 - parameterized and batched
								AddParamStatementToSqlBatch(strSqlBatch, aryParams, "DELETE FROM ApplyDetailsT WHERE ID = {INT}",nApplyDetailID);
								cyPaymentAmountToUnapply -= cyDetail;
							}
							else {
								// (j.jones 2009-12-29 14:31) - PLID 27237 - parameterized and batched
								AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE ApplyDetailsT SET Amount = Convert(money,{STRING}) WHERE ID = {INT}",
									FormatCurrencyForSql(cyDetail - cyPaymentAmountToUnapply),nApplyDetailID);
								cyPaymentAmountToUnapply = COleCurrency(0,0);
							}

							rsApply->MoveNext();
						}
						rsApply->Close();

						cyPaymentAmountToUnapply = COleCurrency(0,0);
					}

					rs->MoveNext();
				}
				rs->Close();

				// (j.jones 2009-12-29 14:34) - PLID 27237 - execute all the apply changes in one batch before continuing
				if(!strSqlBatch.IsEmpty()) {
					ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
				}

				if(nAuditTransactionID != -1) {
					CommitAuditTransaction(nAuditTransactionID);
					nAuditTransactionID = -1;
				}

			}NxCatchAllSilentCallThrow(
				if(nAuditTransactionID != -1) {
					RollbackAuditTransaction(nAuditTransactionID);
				}
				);

				if(bInsurancePays) {
					AfxMessageBox("Some of the unapplied credits were insurance responsibility. You will need to adjust or refund these amounts.");
				}

				//TES 7/1/2008 - PLID 26143 - This is unnecessary if we're exchanging the product
				if(cyTotalRefund > COleCurrency(0,0) && aryPaymentIDs.GetSize() > 0 && !bExchangingProduct) {

					// (j.jones 2010-05-07 08:01) - PLID 37343 - converted to a yes/no prompt
					if(IDYES == MessageBox("Do you wish to refund the payments applied to this product?", "Practice", MB_YESNO|MB_ICONQUESTION)
						&& CheckCurrentUserPermissions(bioRefund,sptCreate)) {

							CPaymentDlg dlg(this);
							dlg.m_PatientID = m_iCurPatient;
							dlg.m_iDefaultPaymentType = 2;
							// (j.jones 2007-02-26 10:58) - PLID 24927 - disallow changing the payment type
							dlg.m_bForceDefaultPaymentType = TRUE;
							// (j.jones 2007-04-19 14:33) - PLID 25711 - converted m_bIsReturnedProductRecord into
							// m_bIsReturnedProductAdj and m_bIsReturnedProductRefund
							dlg.m_bIsReturnedProductRefund = TRUE;
							dlg.m_cyFinalAmount = cyTotalRefund;
							dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;
							dlg.m_cyMaxAmount *= -1;
							dlg.m_DefLocationID = LocationID;

							if (dlg.DoModal(__FUNCTION__, __LINE__) != IDCANCEL) {

								if(dlg.m_varPaymentID.lVal != -1) {
									// (j.jones 2005-05-06 12:02) - PLID 16250 - store the Refund ID
									// (j.jones 2009-12-29 14:34) - PLID 27237 - parameterized
									ExecuteParamSql("UPDATE ReturnedProductsT SET FinRefundID = {INT} WHERE ID = {INT}",VarLong(dlg.m_varPaymentID),nReturnedProductID);
								}

								COleCurrency cyActualRefund = -dlg.m_cyFinalAmount;
								for(int i=0; i<aryPaymentIDs.GetSize(); i++) {
									if(cyActualRefund > COleCurrency(0,0)) {
										COleCurrency cyAmountToApply = COleCurrency(0,0);
										// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
										rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Amount FROM LineItemT WHERE ID = {INT}", (long)aryPaymentIDs.GetAt(i));
										if(!rs->eof) {
											COleCurrency cyAmount = AdoFldCurrency(rs, "Amount",COleCurrency(0,0));
											if(cyActualRefund <= cyAmount) {
												cyAmountToApply = cyActualRefund;
											}
											else {
												cyAmountToApply = cyAmount;
											}
											cyActualRefund -= cyAmountToApply;

											ApplyPayToBill(dlg.m_varPaymentID.lVal,m_iCurPatient,-cyAmountToApply,"Payment",(long)aryPaymentIDs.GetAt(i), -1 /*Patient Resp*/, TRUE);
										}
										rs->Close();
									}
								}
							}
					}
				}			
		}

		//lastly, make a adjustment for the products returned
		//if the adjustment cannot be all patient resp, make as much as we can and warn them
		COleCurrency cyPatientBalance = GetChargePatientBalance(ChargeID,m_iCurPatient);
		COleCurrency cyAmountToAdjust = cyTotalCharge - cyNewTotal;
		if(cyAmountToAdjust > cyPatientBalance) {
			cyAmountToAdjust = cyPatientBalance;
			if(cyAmountToAdjust == COleCurrency(0,0)) {
				AfxMessageBox("This charge's remaining responsibility is for insurance.\n"
					"Please manually shift or adjust the balance of this charge.");
			}
			else {
				AfxMessageBox("Part of this charge's remaining responsibility is for insurance.\n"
					"An adjustment will be made for patient responsibility only.\n"
					"Please manually shift or adjust the remaining insurance balance of this charge.");								
			}			
		}

		// (j.jones 2010-05-07 08:01) - PLID 37343 - converted to a yes/no prompt
		if(IDYES == MessageBox("Do you wish to adjust the charge for this product?", "Practice", MB_YESNO|MB_ICONQUESTION)
			&& CheckCurrentUserPermissions(bioAdjustment,sptCreate)) {

				CPaymentDlg dlg(this);
				dlg.m_PatientID = m_iCurPatient;
				dlg.m_iDefaultPaymentType = 1;
				// (j.jones 2007-02-26 10:58) - PLID 24927 - disallow changing the payment type
				dlg.m_bForceDefaultPaymentType = TRUE;
				// (j.jones 2007-04-19 14:33) - PLID 25711 - converted m_bIsReturnedProductRecord into
				// m_bIsReturnedProductAdj and m_bIsReturnedProductRefund
				dlg.m_bIsReturnedProductAdj = TRUE;
				dlg.m_cyFinalAmount = -cyAmountToAdjust;
				dlg.m_cyMaxAmount = -cyAmountToAdjust;
				dlg.m_DefLocationID = LocationID;
				if (dlg.DoModal(__FUNCTION__, __LINE__) != IDCANCEL) {

					if(dlg.m_varPaymentID.lVal != -1) {
						// (j.jones 2005-05-06 12:02) - PLID 16250 - store the Adjustment ID
						// (j.jones 2009-12-29 14:34) - PLID 27237 - parameterized
						ExecuteParamSql("UPDATE ReturnedProductsT SET FinAdjID = {INT} WHERE ID = {INT}",VarLong(dlg.m_varPaymentID),nReturnedProductID);
					}

					ApplyPayToBill(dlg.m_varPaymentID.lVal,m_iCurPatient,dlg.m_cyFinalAmount,"Charge",ChargeID, -1 /*Patient Resp*/, FALSE);
				}
		}

		//TES 7/1/2008 - PLID 26143 - Now, if they're exchanging the product, make a new bill for the product they're getting
		// in exchange.
		if(bExchangingProduct) {
			if(CheckCurrentUserPermissions(bioBill, sptCreate)) {
				if(cyTotalRefund != COleCurrency(0,0)) {
					DontShowMeAgain(this, FormatString("You must now enter a bill for the product which the patient is getting in exchange.  "
						"The balance of %s which was unapplied from the original bill will be automatically applied to the new bill.",
						FormatCurrencyForInterface(cyTotalRefund)), "BillExchangedProduct");
				}
				else {
					DontShowMeAgain(this, "You must now enter a bill for the product which the patient is getting in exchange.", 
						"BillExchangedProduct");
				}
			}

			//TES 7/1/2008 - PLID 26143 - Copied out of OnNewBill.
			if (m_pParent && m_BillingDlg)
				m_BillingDlg->m_pPatientView = m_pParent;

			//DRT 12/27/2004 - PLID 15083 - The pFinancialDlg ptr (poorly named) is only set in the OnInitDialog
			//	of this CFinancialDlg.  That was fine in the past, but now other things (like the EMR / Custom Records)
			//	need notification of when the bill is saved.  Therefore, they steal that pointer to be themselves, 
			//	and the financial dialog never retrieves it.  We'll just set it here every time they open a new bill, 
			//	and all should be well no matter who else opened it previously.
			m_BillingDlg->m_pFinancialDlg = this;
			//

			//TES 7/1/2008 - PLID 26143 - If we unapplied payments, have the bill automatically apply them when it saves.
			if(!aryPaymentIDs.GetSize()) {
				m_BillingDlg->m_boAskForNewPayment = TRUE;
			}
			else {
				m_BillingDlg->m_boAskForNewPayment = FALSE;
				m_BillingDlg->ApplyPaymentIDs(aryPaymentIDs);
			}
			//TES 7/3/2008 - PLID 26143 - Now that we're not automatically adding the product, let's just default the "Bill A"
			// combo to be for Inventory Items, since we presume that's what they'll be billing.
			m_BillingDlg->SetDefaultBillA(2/*ROW_BILL_A_PRODUCT*/);

			m_BillingDlg->OpenWithBillID(-1, BillEntryType::Bill, 1);

			//TES 7/1/2008 - PLID 26143 - Add the product that they're getting in exchange to the bill, for the quantity
			// that was exchanged.
			//TES 7/3/2008 - PLID 26143 - After seeing this in action, Meikin felt it would cause more harm than good, as in
			// most cases, they will be exchanging for a different product, not the same one.  So this function is now never
			// called, but I'm leaving in the code, as we may choose to revisit this, and possibly have a preference to restore
			// this functionality.
			//m_BillingDlg->AddNewProductToBillByServiceID(ServiceID, dblQtyToRemove);

			//the bill is added to the screen in PostEditBill(1)

			m_boAllowUpdate = TRUE;
		}

		// a.walling 4/14/06 A quick refresh should work fine
		QuickRefresh();
		CalculateTotals();

	}NxCatchAll("Error returning product.");
}

void CFinancialDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	try {

		IRowSettingsPtr pRow = m_List->GetCurSel();

		GetCursorPos(&point);

		CMenu tmpMenu;

		// (j.jones 2007-04-05 15:58) - PLID 25514 - converted to not use defines anymore
		enum {
			miNewBill = -10,
			miNewPay = -11,
			miNewAdj = -12,
			miNewRef = -13,
			miNewCharge = -14,
			miDeleteBill = -15,
			miDeleteCharge = -16,
			miRelApply = -17,
			miUnapply = -18,
			miApplyNewPayToBill = -19,
			miApplyExistingPayToBill = -20,
			miApplyExistingPayToCharge = -21,
			miApplyNewPayToCharge = -22,
			miApplyMgr = -23,
			miEditPay = -24,
			miEditAdj = -25,
			miEditRef = -26,
			miViewPay = -27,
			miDeletePay = -28,
			miDeleteAdj = -29,
			miDeleteRef = -30,
			miPrintReceipt = -31,
			miEditChargeResp = -32,
			miChargePosting = -33,
			miBillPosting = -34,
			miReturnProduct = -35,
			miRefundPayment = -36,
			miSendToPaperBatch = -37,
			miSendToElectronicBatch = -38,
			miUnbatchBill = -39,	
			// (j.gruber 2007-03-15 14:35) - PLID 25223 - print the sales receipt
			miPrintSalesReceipt = -40,
			// (j.gruber 2007-03-29 15:17) - PLID 9802 - sales receipt receipt printer format
			miPrintSalesReceiptRPFormat = -41,
			// r.galicki 6/10/2008 - PLID 28380	- "View Serialized Items" menu options
			miViewBillSerializedItems = -42,
			miViewChargeSerializedItems = -43,
			//TES 7/1/2008 - PLID 26143 - Added an "Exchange Product" menu option.
			miExchangeProduct = -44,
			// (j.jones 2009-08-21 17:49) - PLID 24569 - added pre-payment option
			miNewPrePay = -45,
			// (z.manning 2010-06-17 09:07) - Transferring bills was deemed too dangerous
			//miTransferBill = -46,
			miTransferPay = -47,
			miTransferPrePay = -48,
			miTransferAdj = -49,
			miTransferRef = -50,
			// (j.gruber 2011-05-27 13:17) - PLID 44832 - correct and void a charge
			miCorrectCharge = -51,
			// (s.dhole 2011-06-02 09:59) - PLID 44832 Bill Correction menu
			miCorrectBill = -52,
			// (j.gruber 2011-05-27 13:17) - PLID 44832 - void a charge
			miVoidCharge = -53,
			// (j.dinatale 2011-06-02 17:33) - PLID 44809 - Correct a Payment
			miCorrectPayment = -54,
			// (j.dinatale 2011-06-03 09:30) - PLID 44809 - Void a payment
			miVoidPayment = -55,
			// (j.gruber 2011-06-14 17:37) - PLID 44832
			miVoidBill = -56,
			// (j.jones 2011-09-20 14:05) - PLID 45462 - undo a bill correction
			miUndoBillCorrection = -57,
			// (j.jones 2011-09-20 14:05) - PLID 45562 - undo a charge correction
			miUndoChargeCorrection = -58,
			// (j.jones 2011-09-20 14:05) - PLID 45563 - undo a payment correction
			miUndoPaymentCorrection = -59,
			// (j.armen 2012-04-23 11:11) - PLID 14102 - insurance takeback / reversal
			miInsuranceReversal = -60,
			// (r.gonet 07/07/2014) - PLID 62571 - Mark Bill On Hold
			miMarkBillOnHold = -61,
			// (r.gonet 07/07/2014) - PLID 62571 - Remove Bill Hold
			miRemoveBillHold = -62,
			// (r.gonet 07/09/2014) - PLID 62556 - Undo a chargeback
			miUndoChargeback = -63,		
			// (b.spivey, August 27th, 2014) - PLID 63491 - claim file preview
			miPrintClaimFile = -64,
			// (b.spivey, October 10th, 2014) - PLID 62699 - View EOB file. 
			miViewEOBAssociatedFile = -65,
			// (z.manning 2015-07-22 10:19) - PLID 67241
			miPaymentProfile = -66,
			// (z.manning 2015-09-04 10:51) - PLID 67226
			miPrintCreditCardReceipt = -67,
		};

		// (j.jones 2011-09-20 14:40) - PLID 45462 - cache line item correction information
		// to undo corrections later
		long nCorrectionID_ToUndo = -1;
		COleDateTime dtCorrectionInputDate;
		long nCorrectionOriginalID = -1;
		long nCorrectionVoidingID = -1;
		long nCorrectionNewID = -1;
		// (r.gonet 07/09/2014) - PLID 62556 - If this line item is a chargeback, this will be not null and will contain all the
		// information about the chargeback.
		ChargebackPtr pChargeback;

		/////////////////////////////////////////////////////////////////////
		// Build a menu popup with the ability to delete the current row
		tmpMenu.CreatePopupMenu();

		tmpMenu.InsertMenu(0, MF_BYPOSITION, miNewBill, "New &Bill");

		//create a sub-menu for pay/adj/ref
		CMenu pSubMenu1;
		pSubMenu1.CreatePopupMenu();	
		pSubMenu1.InsertMenu(0, MF_BYPOSITION, miNewPay, "Create a New &Payment");
		pSubMenu1.InsertMenu(1, MF_BYPOSITION, miNewPrePay, "Create a New Pr&e-Payment");
		pSubMenu1.InsertMenu(2, MF_BYPOSITION, miNewAdj, "Create a New &Adjustment");
		pSubMenu1.InsertMenu(3, MF_BYPOSITION, miNewRef, "Create a New &Refund");
		// (z.manning 2015-07-22 10:18) - PLID 67241 - Payment profile menu option
		if (IsICCPEnabled()) {
			pSubMenu1.InsertMenu(4, MF_BYPOSITION, miPaymentProfile, "New Pa&yment Profile");
		}

		tmpMenu.InsertMenu(-1, MF_BYPOSITION|MF_POPUP, (UINT)pSubMenu1.m_hMenu, "New &Pay/Adj/Ref...");

		tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
		tmpMenu.InsertMenu(-1, MF_BYPOSITION, miApplyMgr, "&Apply Manager");

		// (j.dinatale 2011-06-28 14:48) - PLID 44808 - moved these variables out here since they are going to be referenced when payment line items get voided/corrected
		CString strPayType = "Item";
		CString strLineType = "";

		///////////////////////////////////////////////////////////////////
		// (j.jones 2007-04-18 11:24) - PLID 25514 - if they right clicked
		// outside of the visible list, don't add more items
		if(pRow) {

			_variant_t var = pRow->GetValue(btcLineType);

			// (j.dinatale 2011-06-28 15:06) - PLID 44808 - moved to a higher scope so they can be referenced later on
			strLineType = VarString(var,"");

			// A "blank" indicates a blank line added by an expanded bill
			if (strLineType == CString(BLANK_LINE)) {
				return;
			}

			//////////////////////////////////////////////
			// Access check for applying
			else if (strLineType == CString("Applied") || strLineType == CString("InsuranceApplied") || strLineType == CString("AppliedPayToPay")) {

				// (j.jones 2008-11-12 09:38) - PLID 28818 - multiple menu items in this submenu need to know the applied
				// item type, so only query it once, in a parameter query
				// (j.armen 2012-04-23 11:11) - PLID 14102 - Also find out if this is an insurance payment
				long nApplyID = VarLong(pRow->GetValue(btcApplyID), -1);
				long nLineItemType = -1;
				long nSourceLineItemID = -1;
				long nPayMethod = 0;
				BOOL bIsPrePayment = FALSE;
				BOOL bIsInsurancePayment = FALSE;
				if(nApplyID != -1) {
					// (j.jones 2011-09-20 13:56) - PLID 45563 - get the ID of the source payment
					// (r.gonet 07/09/2014) - PLID 62556 - Get chargebacks as well.
					// (z.manning 2015-09-08 09:37) - PLID 67226 - Added pay method
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(),
						"SELECT "
						"	LineItemT.ID, LineItemT.Type, "
						"	PaymentsT.PrePayment, PaymentsT.InsuredPartyID, PaymentsT.PayMethod, "
						"	ChargebacksT.ID AS ChargebackID, ChargebacksT.ChargeID AS ChargebackChargeID, ChargebacksT.PaymentID AS ChargebackPaymentID, ChargebacksT.AdjustmentID AS ChargebackAdjustmentID "
						"FROM LineItemT "
						"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
						"INNER JOIN AppliesT ON LineItemT.ID = AppliesT.SourceID "
						"LEFT JOIN ChargebacksT ON (LineItemT.Type = {CONST_INT} AND LineItemT.ID = ChargebacksT.PaymentID) OR (LineItemT.Type = {CONST_INT} AND LineItemT.ID = ChargebacksT.AdjustmentID) "
						"WHERE AppliesT.ID = {INT}"
						, (long)LineItem::Payment, (long)LineItem::Adjustment
						, nApplyID);
					if(!rs->eof) {
						nSourceLineItemID = AdoFldLong(rs, "ID");
						nLineItemType = AdoFldLong(rs, "Type", -1);
						bIsPrePayment = AdoFldBool(rs, "PrePayment", FALSE);
						bIsInsurancePayment = (AdoFldLong(rs, "InsuredPartyID", -1) != -1);
						long nChargebackID = AdoFldLong(rs, "ChargebackID", -1);
						if (nChargebackID != -1) {
							pChargeback = make_shared<Chargeback>(nChargebackID, AdoFldLong(rs, "ChargebackChargeID"), AdoFldLong(rs, "ChargebackPaymentID"), AdoFldLong(rs, "ChargebackAdjustmentID"));
						}
						nPayMethod = AdoFldLong(rs, "PayMethod");
					}
					rs->Close();
				}

				// (j.dinatale 2011-06-28 14:47) - PLID 44808 - moved the variable declaration to the higher scope, going to need it later
				// (j.jones 2008-11-12 09:46) - PLID 28818 - find out what type of payment it is
				//CString strPayType = "Item";
				if(nLineItemType == 1 && bIsPrePayment) {
					strPayType = "PrePayment";
				}
				else if(nLineItemType == 1 && !bIsPrePayment) {
					strPayType = "Payment";
				}
				else if(nLineItemType == 2) {
					strPayType = "Adjustment";
				}
				else if(nLineItemType == 3) {
					strPayType = "Refund";
				}

				// (j.jones 2011-03-25 16:28) - PLID 41143 - added Show Related Applies as an option for
				// already applied items
				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miRelApply, "&Show Related Applies");

				//now determine our View Applied and Unapply labels
				CString strViewApplied;
				strViewApplied.Format("&View Applied %s", strPayType);
				CString strUnapply;
				strUnapply.Format("&Unapply %s", strPayType);

				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miViewPay, strViewApplied);

				// (r.gonet 07/09/2014) - PLID 62557 - Chargebacks can't be unapplied
				if (pChargeback == NULL) {
				//if it is a payment, allow a refund				
				if(strLineType != CString("AppliedPayToPay") && nApplyID != -1 && nLineItemType == 1) {
					CString strRefundApplied = "&Refund Applied Payment";
					if(bIsPrePayment) {
						strRefundApplied = "&Refund Applied PrePayment";
					}
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miRefundPayment, strRefundApplied);
				}
				}

				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");

				// (j.dinatale 2011-06-23 11:02) - PLID 44809
				// (j.jones 2011-08-29 16:55) - PLID 44880 - reversed the order such that Void & Correct shows up first, Void second
				// (j.dinatale 2012-02-01 15:24) - PLID 45511 - line item corrections is no longer in beta, so dont check our flag
				// (j.jones 2014-07-14 16:03) - PLID 62876 - you cannot void chargebacks
				if (pChargeback == NULL) {

					// (j.jones 2011-09-20 13:56) - PLID 45563 - find out if the payment is already corrected,
					// but in the event that this same ID is in multiple corrections, we want to undo the most
					// recent correction first
					BOOL bIsNewLineItemID = FALSE;
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 LineItemCorrectionsT.ID, "
						"InputDate, OriginalLineItemID, VoidingLineItemID, NewLineItemID "
						"FROM LineItemCorrectionsT "
						"WHERE OriginalLineItemID = {INT} OR VoidingLineItemID = {INT} OR NewLineItemID = {INT} "
						"ORDER BY LineItemCorrectionsT.ID DESC",
						nSourceLineItemID, nSourceLineItemID, nSourceLineItemID);
					if(!rs->eof) {
						nCorrectionID_ToUndo = VarLong(rs->Fields->Item["ID"]->Value);
						dtCorrectionInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
						nCorrectionOriginalID = VarLong(rs->Fields->Item["OriginalLineItemID"]->Value);
						nCorrectionVoidingID = VarLong(rs->Fields->Item["VoidingLineItemID"]->Value);
						nCorrectionNewID = VarLong(rs->Fields->Item["NewLineItemID"]->Value, -1);	//could be null

						//is our source payment this correction's new payment?
						bIsNewLineItemID = (nSourceLineItemID == nCorrectionNewID);
					}
					rs->Close();

					// (j.jones 2011-09-20 13:56) - PLID 45563 - if this is part of a correction, give
					// the option to undo that correction
					if(nCorrectionID_ToUndo != -1) {
						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miUndoPaymentCorrection, "Undo Correction");
					}
					
					// (j.jones 2011-09-20 13:56) - PLID 45563 - we give the correction options
					// only if the line item is not part of a correction OR it is the new line
					// item in that correction
					// (j.armen 2012-04-23 11:12) - PLID 14102 - Add in the Insurance Reversal 
					// if this is a PrePayment or Payment and an insurance payment
					if(nCorrectionID_ToUndo == -1 || bIsNewLineItemID) {

						if(nLineItemType == 1 && bIsPrePayment) {
							// (j.dinatale 2011-08-03 16:29) - PLID 44809 - we decided to allow correction of prepayments
							tmpMenu.InsertMenu(-1, MF_BYPOSITION, miCorrectPayment, "Void and Correct PrePayment");
							tmpMenu.InsertMenu(-1, MF_BYPOSITION, miVoidPayment, "Void PrePayment");
							if(bIsInsurancePayment) {
								tmpMenu.InsertMenu(-1, MF_BYPOSITION, miInsuranceReversal, "Insurance Reversal");
							}
						}
						else if(nLineItemType == 1 && !bIsPrePayment) {						
							tmpMenu.InsertMenu(-1, MF_BYPOSITION, miCorrectPayment, "Void and Correct Payment");
							tmpMenu.InsertMenu(-1, MF_BYPOSITION, miVoidPayment, "Void Payment");
							if(bIsInsurancePayment)		{
								tmpMenu.InsertMenu(-1, MF_BYPOSITION, miInsuranceReversal, "Insurance Reversal");
							}
						}
						else if(nLineItemType == 2) {						
							tmpMenu.InsertMenu(-1, MF_BYPOSITION, miCorrectPayment, "Void and Correct Adjustment");
							tmpMenu.InsertMenu(-1, MF_BYPOSITION, miVoidPayment, "Void Adjustment");
						}
						else if(nLineItemType == 3) {						
							tmpMenu.InsertMenu(-1, MF_BYPOSITION, miCorrectPayment, "Void and Correct Refund");
							tmpMenu.InsertMenu(-1, MF_BYPOSITION, miVoidPayment, "Void Refund");
						}
					}

					tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				}

				// (r.gonet 07/09/2014) - PLID 62556 - Chargebacks can be undone
				if (pChargeback != NULL) {
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miUndoChargeback, "Undo Chargeback");
				} else {
					// Row is not a chargeback
				}

				// (r.gonet 07/09/2014) - PLID 62557 - Chargebacks can't be unapplied
				if (pChargeback == NULL) {
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miUnapply, strUnapply);
				} else {
					// Row is not a chargeback
				}

				// (j.gruber 2007-03-15 14:36) - PLID 25223 - changed receipt to payment receipt and added sales receipt
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintReceipt, "Prin&t Payment Receipt");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintSalesReceipt, "Print Sales Rec&eipt");

				// (b.spivey, October 10th, 2014) - PLID 62699 - Is this a line item we can even view an EOB for?
				long nPayAdjID = VarLong(pRow->GetValue(btcPayID), -1);
				if (nLineItemType == 1 || nLineItemType == 2) {
					_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), R"( 
							SELECT BatchPaymentsT.ERemittance, BatchPaymentsT.EOBFilePath
							FROM PaymentsT 
							LEFT JOIN BatchPaymentsT ON PaymentsT.BatchPaymentID = BatchPaymentsT.ID 
							WHERE PaymentsT.ID = {INT} 
							)", nPayAdjID);

					if (!prs->eof) {
						BOOL bIsFromEOB = AdoFldBool(prs, "ERemittance", FALSE);
						bool bFilePathNotEmpty = AdoFldString(prs, "EOBFilepath", "").GetLength() > 0 ? true : false;
						if (bIsFromEOB) {
							// (b.spivey, October 14, 2014) - PLID 62699 - New Accelerator
							if (bFilePathNotEmpty) {
								tmpMenu.InsertMenuA(-1, MF_BYPOSITION, miViewEOBAssociatedFile, "Vie&w Associated EOB");
							}
							else {
								tmpMenu.InsertMenuA(-1, MF_BYPOSITION | MF_DISABLED, miViewEOBAssociatedFile, "Vie&w Associated EOB");
							}
						}
					}
				}


				// (j.gruber 2007-03-29 15:17) - PLID 9802 - sales receipt receipt printer format
				//TES 12/6/2007 - PLID 28192 - Make sure we have a POS Printer.
				if (GetMainFrame()->CheckPOSPrinter()) {
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintSalesReceiptRPFormat, "Print Sales Receipt - Receipt Printer &Format");
				}
				// (z.manning 2015-09-08 09:18) - PLID 67226 - Add an option to re-print credit card receipts
				if (IsICCPEnabled() && nPayMethod == 3) {
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintCreditCardReceipt, "Re-print &Credit Card Receipts");
				}
			}
			else if (strLineType == CString("Bill")) {
				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miNewCharge, "Edit &Charges");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miApplyNewPayToBill, "Apply a &New Payment");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miApplyExistingPayToBill, "Apply an &Existing Payment");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miBillPosting, "Apply To Bill Using &Line Item Posting");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miEditChargeResp, "Edit Charge &Responsibility Dates");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miViewBillSerializedItems, "View &Serialized Bill Items"); //r.galicki 6/10/2008 PLID 28380 - serialized item menu option
				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");

				long BillID = VarLong(pRow->GetValue(m_nBillIDColID),-1);
				CString strClaimType = "HCFA";

				_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT FormType FROM BillsT WHERE ID = {INT}", BillID);
				bool bAllowFileExport = false; 
				if(!rs->eof) {
					long nFormType = AdoFldLong(rs, "FormType",1);
					switch(nFormType) {
						case 1:
							strClaimType = "HCFA";
							bAllowFileExport = true; 
							break;
						case 2:
							//TES 3/13/2007 - PLID 24993 - Changed from UB92 to UB
							strClaimType = "UB";
							bAllowFileExport = true;
							break;
						case 3:
							strClaimType = "ADA";
							break;
						case 4:
							strClaimType = "IDPA";
							break;
						case 5:
							strClaimType = "NYWC";
							break;
						case 6:
							strClaimType = "MICR";
							break;
						case 7:
							// (j.jones 2010-02-04 11:51) - PLID 37113 - supported the NY Medicaid form
							strClaimType = "NY Medicaid";
							break;
					}
				}
				rs->Close();

				// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized & combined recordsets				
				// (r.gonet 07/07/2014) - PLID 62571 - Reduced the number of duplicated parameters being passed in and added
				// a select statement for the bill's current status. 
				rs = CreateParamRecordset(GetRemoteDataSnapshot(),
					"DECLARE @BillID INT SET @BillID = {INT}; "
					"DECLARE @PatientID INT SET @PatientID = {INT}; "
					//PLID 28380 r.galicki 6/11/2008	- if no serialized items attached to bill, gray out option
					"SELECT TOP 1 ProductID FROM ProductItemsT INNER JOIN ServiceT ON ProductItemsT.ProductID = ServiceT.ID"
					" INNER JOIN ChargedProductItemsT ON ProductItemsT.ID = ChargedProductItemsT.ProductItemID"
					" INNER JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID"
					" INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE ChargesT.BillID = @BillID "
					""
					// (j.jones 2007-04-05 14:59) - PLID 25514 - changed to support printing to any resp type
					"SELECT Name, InsuredPartyT.PersonID, TypeName "
					"FROM InsuredPartyT "
					"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
					"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
					"WHERE PatientID = @PatientID "
					"ORDER BY (CASE WHEN RespTypeID = -1 THEN 1 ELSE 0 END) ASC, "
					"RespTypeID ASC "
					""
					"SELECT DefBatch FROM HCFASetupT "
					"INNER JOIN InsuranceCoT ON HCFASetupT.ID = InsuranceCoT.HCFASetupGroupID "
					"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
					"INNER JOIN BillsT ON InsuredPartyT.PersonID = BillsT.InsuredPartyID "
					"WHERE BillsT.ID = @BillID "
					""
					"SELECT CONVERT(BIT, (CASE WHEN BillStatusT.ID IS NOT NULL AND BillStatusT.Type = {CONST_INT} THEN 1 ELSE 0 END)) AS OnHold "
					"FROM BillsT "
					"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
					"WHERE BillsT.ID = @BillID "
					, BillID, m_iCurPatient, (long)EBillStatusType::OnHold);
				if(rs->eof)
				{
					tmpMenu.EnableMenuItem(miViewBillSerializedItems, MF_GRAYED);
				}

				CMenu pSubMenuPrint;
				pSubMenuPrint.CreatePopupMenu();

				rs = rs->NextRecordset(NULL);

				if(!rs->eof) {

					int iPos = 0;

					while(!rs->eof) {

						CString strLabel;

						CString strInsCoName = AdoFldString(rs, "Name","");
						long nInsuredPartyID = AdoFldLong(rs, "PersonID",-1);
						CString strRespTypeName = AdoFldString(rs, "TypeName","");

						strLabel.Format("For %s (%s)",strInsCoName,strRespTypeName);

						pSubMenuPrint.InsertMenu(iPos, MF_BYPOSITION, nInsuredPartyID, strLabel);
						iPos++;

						rs->MoveNext();
					}

					CString strLabel;
					strLabel.Format("Pre&view %s...",strClaimType);
					tmpMenu.InsertMenu(-1, MF_BYPOSITION|MF_POPUP, (UINT)pSubMenuPrint.m_hMenu, strLabel);
				}


				// Build a menu popup with the ability to choose which batch it goes to		
				int nCurrentBatch = FindHCFABatch(BillID);
				int nDefaultBatch = 0;

				rs = rs->NextRecordset(NULL);

				if(!rs->eof) {
					nDefaultBatch = AdoFldLong(rs, "DefBatch",0);
				}

				CMenu pSubMenuBatch;
				pSubMenuBatch.CreatePopupMenu();

				pSubMenuBatch.InsertMenu(0, MF_BYPOSITION|(nCurrentBatch == 1 ? MF_CHECKED : MF_UNCHECKED), miSendToPaperBatch, "Paper Batch");
				pSubMenuBatch.InsertMenu(1, MF_BYPOSITION|(nCurrentBatch == 2 ? MF_CHECKED : MF_UNCHECKED), miSendToElectronicBatch, "Electronic Batch");
				pSubMenuBatch.InsertMenu(2, MF_BYPOSITION|(nCurrentBatch == 0 ? MF_CHECKED : MF_UNCHECKED), miUnbatchBill, "Unbatched");

				if(nDefaultBatch == 0)
					pSubMenuBatch.SetDefaultItem(3,TRUE);
				else pSubMenuBatch.SetDefaultItem(nDefaultBatch-1,TRUE);

				CString strLabel;
				strLabel.Format("&Batch %s...",strClaimType);
				tmpMenu.InsertMenu(-1, MF_BYPOSITION|MF_POPUP, (UINT)pSubMenuBatch.m_hMenu, strLabel);

				// (r.gonet 07/07/2014) - PLID 62571 - Read the On Hold status of the bill.
				BOOL bOnHold = FALSE;
				rs = rs->NextRecordset(NULL);
				if (!rs->eof) {
					bOnHold = AdoFldBool(rs, "OnHold", FALSE);
				}
				rs->Close();
				
				// (r.gonet 07/07/2014) - PLID 62571 - Give the user the ability to change the On Hold status
				// of the bill on the billing tab. 
				if (!bOnHold) {
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miMarkBillOnHold, "Mark Bill On Hold");
				} else {
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miRemoveBillHold, "Remove Bill Hold");
				}


				// (b.spivey, August 27th, 2014) - PLID 63491 - same bracket as the other ebilling stuff. 
				// (j.jones 2016-05-19 10:22) - NX-100685 - added a universal function for getting the default E-Billing / E-Eligibility format
				long nFormatID = GetDefaultEbillingANSIFormatID();
				long nEbillingFormatType = GetRemotePropertyInt("EbillingFormatType", ANSI, 0, "<None>", true);
				if (nEbillingFormatType == ANSI) {

					_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ANSIVersion FROM EbillingFormatsT WHERE ID = {INT}", nFormatID);
					if (!prs->eof) {

						ANSIVersion avANSIVersion = (ANSIVersion)AdoFldLong(prs->Fields, "ANSIVersion", (long)av5010);

						if (avANSIVersion == av5010 && bAllowFileExport)
						{
							tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintClaimFile, "Preview Claim File");
						}
					}
				}

				// (s.dhole 2011-06-02 09:59) - PLID 44832 Bill Correction menu
				
				// (j.dinatale 2012-02-01 15:24) - PLID 45511 - line item corrections is no longer in beta, so dont check our flag
				//if (IsLineItemCorrectionsEnabled_Beta()) 
				{
					tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");

					// (j.jones 2011-09-22 08:49) - PLID 45462 - find out if the bill is already corrected,
					// but in the event that this same ID is in multiple corrections, we want to undo the most
					// recent correction first
					BOOL bIsNewBillID = FALSE;
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 BillCorrectionsT.ID, "
						"InputDate, OriginalBillID, NewBillID "
						"FROM BillCorrectionsT "
						"WHERE OriginalBillID = {INT} OR NewBillID = {INT} "
						"ORDER BY BillCorrectionsT.ID DESC",
						BillID, BillID);
					if(!rs->eof) {
						nCorrectionID_ToUndo = VarLong(rs->Fields->Item["ID"]->Value);
						dtCorrectionInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
						nCorrectionOriginalID = VarLong(rs->Fields->Item["OriginalBillID"]->Value);
						nCorrectionVoidingID = -1;
						nCorrectionNewID = VarLong(rs->Fields->Item["NewBillID"]->Value, -1);	//could be null

						//is our selected bill this correction's new bill?
						bIsNewBillID = (BillID == nCorrectionNewID);
					}
					rs->Close();

					// (j.jones 2011-09-22 08:49) - PLID 45462 - if this is part of a correction, give
					// the option to undo that correction
					if(nCorrectionID_ToUndo != -1) {
						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miUndoBillCorrection, "Undo Correction");
					}
					
					// (j.jones 2011-09-22 08:49) - PLID 45462 - we give the correction options
					// only if the bill is not part of a correction OR it is the new bill
					// in that correction
					if(nCorrectionID_ToUndo == -1 || bIsNewBillID) {
						// (j.jones 2011-09-13 15:37) - PLID 44887 - if the bill has original or void charges
						// in it, do not gray this out, we will simply explain why they can't correct the bill
						// only if they choose the menu item		
						// (j.jones 2011-08-29 16:55) - PLID 44880 - reversed the order such that Void & Correct shows up first, Void second
						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miCorrectBill, "Void and Correct Bill");				
						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miVoidBill, "Void Bill");
					}

				}
				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				// (z.manning 2010-06-11 12:30) - PLID 29214 - Option to transfer bill
				// (z.manning 2010-06-17 09:10) - Too dangerous, removed
				//tmpMenu.InsertMenu(-1, MF_BYPOSITION, miTransferBill, "Transfer Bill to Another Patient");
			
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miDeleteBill, "&Delete Bill");
				
			}
			else if (strLineType == CString("Charge")) {
				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miApplyNewPayToCharge, "Apply a &New Payment");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miApplyExistingPayToCharge, "Apply an &Existing Payment");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miChargePosting, "Apply To &Charge Using Line Item Posting");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miBillPosting, "Apply To &Bill Using Line Item Posting");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miEditChargeResp, "Edit Charge &Responsibility Dates");

				//see if it is a product
				long nChargeID = VarLong(pRow->GetValue(btcChargeID),-1);		

				//PLID 28380 r.galicki 6/11/2008	- if no serialized items attached to charge, gray out option
				// (j.jones 2009-12-29 11:01) - PLID 27237 - combined recordsets
				_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ChargesT.ID, "
					"Sum(CASE WHEN ProductItemsT.ID Is Null THEN 0 ELSE 1 END) AS ChargedProductItems "
					"FROM ChargesT "
					"INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID "
					"LEFT JOIN ChargedProductItemsT ON ChargesT.ID = ChargedProductItemsT.ChargeID "
					"LEFT JOIN ProductItemsT ON ChargedProductItemsT.ProductItemID = ProductItemsT.ID "
					"WHERE ChargesT.ID = {INT} "
					"GROUP BY ChargesT.ID", nChargeID);
				if(!rs->eof) {
					//it is a product
					//r.galicki 6/10/2008 PLID 28380 - serialized item menu option
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miViewChargeSerializedItems, "View &Serialized Charge Items");
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miReturnProduct, "&Return Product");
					//TES 7/1/2008 - PLID 26143 - Added an "Exchange Product" menu option.
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miExchangeProduct, "&Exchange Product");

					//now, does it have serialized items?
					if(AdoFldLong(rs, "ChargedProductItems", 0) == 0) {
						//no, so disable the menu item
						tmpMenu.EnableMenuItem(miViewChargeSerializedItems, MF_GRAYED);
					}
				}
				rs->Close();

				// (j.gruber 2011-05-27 13:19) - PLID 44832
				// (j.jones 2011-08-29 16:55) - PLID 44880 - reversed the order such that Void & Correct shows up first, Void second
				// (j.dinatale 2012-02-01 15:24) - PLID 45511 - line item corrections is no longer in beta, so dont check our flag
				//if (IsLineItemCorrectionsEnabled_Beta()) 
				{
					tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");

					// (j.jones 2011-09-22 08:52) - PLID 45562 - find out if the charge is already corrected,
					// but in the event that this same ID is in multiple corrections, we want to undo the most
					// recent correction first
					BOOL bIsNewChargeID = FALSE;
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 LineItemCorrectionsT.ID, "
						"InputDate, OriginalLineItemID, VoidingLineItemID, NewLineItemID "
						"FROM LineItemCorrectionsT "
						"WHERE OriginalLineItemID = {INT} OR VoidingLineItemID = {INT} OR NewLineItemID = {INT} "
						"ORDER BY LineItemCorrectionsT.ID DESC",
						nChargeID, nChargeID, nChargeID);
					if(!rs->eof) {
						nCorrectionID_ToUndo = VarLong(rs->Fields->Item["ID"]->Value);
						dtCorrectionInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
						nCorrectionOriginalID = VarLong(rs->Fields->Item["OriginalLineItemID"]->Value);
						nCorrectionVoidingID = VarLong(rs->Fields->Item["VoidingLineItemID"]->Value);
						nCorrectionNewID = VarLong(rs->Fields->Item["NewLineItemID"]->Value, -1);	//could be null

						//is our selected charge this correction's new charge?
						bIsNewChargeID = (nChargeID == nCorrectionNewID);
					}
					rs->Close();

					// (j.jones 2011-09-22 08:52) - PLID 45562 - if this is part of a correction, give
					// the option to undo that correction
					if(nCorrectionID_ToUndo != -1) {
						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miUndoChargeCorrection, "Undo Correction");
					}
					
					// (j.jones 2011-09-22 08:52) - PLID 45562 - we give the correction options
					// only if the charge is not part of a correction OR it is the new charge
					// in that correction
					if(nCorrectionID_ToUndo == -1 || bIsNewChargeID) {

						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miCorrectCharge, "Void and Correct Charge");
						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miVoidCharge, "Void Charge");
					}
				}


				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miDeleteCharge, "&Delete Charge");
			}
			else if (strLineType == CString("Payment")) {
				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miRelApply, "&Show Related Applies");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miEditPay, "&Edit Payment");
				// (r.gonet 07/09/2014) - PLID 62557 - Chargeback payments can't be refunded
				if (pChargeback == NULL) {
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miRefundPayment, "&Refund Payment");
				}
				else {
					// Row is not a chargeback
				}

				long nPayMethod = 0;
				{
					BOOL bIsPrePayment = FALSE;
					BOOL bIsInsurancePayment = FALSE;
					// (j.dinatale 2011-06-02 17:32) - PLID 44809 - check for prepayments
					// (j.armen 2012-04-23 11:11) - PLID 14102 - Also find out if this is an insurance payment
					// (z.manning 2015-09-08 09:12) - PLID 67226 - Get the pay method too
					long nPayID = VarLong(pRow->GetValue(btcPayID));
					_RecordsetPtr rs = CreateParamRecordset(
						"SELECT PaymentsT.PrePayment, PaymentsT.InsuredPartyID, PaymentsT.PayMethod \r\n"
						"FROM LineItemT \r\n"
						"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n"
						"WHERE LineItemT.ID = {INT} \r\n"
						, nPayID);
					if (!rs->eof) {
						bIsPrePayment = AdoFldBool(rs, "PrePayment", FALSE);
						bIsInsurancePayment = (AdoFldLong(rs, "InsuredPartyID", -1) != -1);
						nPayMethod = AdoFldLong(rs, "PayMethod");
					}
					rs->Close();

					// (j.dinatale 2011-06-02 17:32) - PLID 44809
					// (j.jones 2011-08-29 16:55) - PLID 44880 - reversed the order such that Void & Correct shows up first, Void second
					// (j.dinatale 2012-02-01 15:24) - PLID 45511 - line item corrections is no longer in beta, so dont check our flag
					// (j.jones 2014-07-14 16:03) - PLID 62876 - you cannot void chargebacks
					if (pChargeback == NULL) {

						tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");

						// (j.jones 2011-09-20 15:46) - PLID 45563 - find out if the payment is already corrected,
						// but in the event that this same ID is in multiple corrections, we want to undo the most
						// recent correction first
						BOOL bIsNewLineItemID = FALSE;
						_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 LineItemCorrectionsT.ID, "
							"InputDate, OriginalLineItemID, VoidingLineItemID, NewLineItemID "
							"FROM LineItemCorrectionsT "
							"WHERE OriginalLineItemID = {INT} OR VoidingLineItemID = {INT} OR NewLineItemID = {INT} "
							"ORDER BY LineItemCorrectionsT.ID DESC",
							nPayID, nPayID, nPayID);
						if (!rs->eof) {
							nCorrectionID_ToUndo = VarLong(rs->Fields->Item["ID"]->Value);
							dtCorrectionInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
							nCorrectionOriginalID = VarLong(rs->Fields->Item["OriginalLineItemID"]->Value);
							nCorrectionVoidingID = VarLong(rs->Fields->Item["VoidingLineItemID"]->Value);
							nCorrectionNewID = VarLong(rs->Fields->Item["NewLineItemID"]->Value, -1);	//could be null

							//is our source payment this correction's new payment?
							bIsNewLineItemID = (nPayID == nCorrectionNewID);
						}
						rs->Close();

						// (j.jones 2011-09-20 15:46) - PLID 45563 - if this is part of a correction, give
						// the option to undo that correction
						if (nCorrectionID_ToUndo != -1) {
							tmpMenu.InsertMenu(-1, MF_BYPOSITION, miUndoPaymentCorrection, "Undo Correction");
						}

						// (j.jones 2011-09-20 15:45) - PLID 45563 - we give the correction options
						// only if the line item is not part of a correction OR it is the new line
						// item in that correction
						if (nCorrectionID_ToUndo == -1 || bIsNewLineItemID) {

							if (!bIsPrePayment){
								tmpMenu.InsertMenu(-1, MF_BYPOSITION, miCorrectPayment, "Void and Correct Payment");
								tmpMenu.InsertMenu(-1, MF_BYPOSITION, miVoidPayment, "Void Payment");
							}
							else{
								// (j.dinatale 2011-06-02 17:32) - PLID 44809 - apparently we decided to allow correction of prepayments		
								tmpMenu.InsertMenu(-1, MF_BYPOSITION, miCorrectPayment, "Void and Correct PrePayment");
								tmpMenu.InsertMenu(-1, MF_BYPOSITION, miVoidPayment, "Void PrePayment");
							}

							// (j.armen 2012-04-23 11:12) - PLID 14102 - Add in the Insurance Reversal 
							// if this is an insurance payment
							if (bIsInsurancePayment) {
								tmpMenu.InsertMenu(-1, MF_BYPOSITION, miInsuranceReversal, "Insurance Reversal");
							}
						}
					}
				}

				//TES 7/25/2014 - PLID 63049 - Don't allow deleting or transferring Chargebacks
				if (pChargeback == NULL) {
					tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
					// (z.manning 2010-06-10 14:49) - PLID 29214 - Transfer option
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miTransferPay, "Transfer Payment to Another Patient");
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miDeletePay, "&Delete Payment");
					tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				}

				// (j.gruber 2007-03-15 14:36) - PLID 25223 - changed receipt to payment receipt and added sales receipt
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintReceipt, "Prin&t Payment Receipt");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintSalesReceipt, "Print Sales Rec&eipt");
				// (j.gruber 2007-03-29 15:17) - PLID 9802 - sales receipt receipt printer format
				//TES 12/6/2007 - PLID 28192 - Make sure we have a POS Printer.
				if (GetMainFrame()->CheckPOSPrinter()) {
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintSalesReceiptRPFormat, "Print Sales Receipt - Receipt Printer &Format");
				}
				// (z.manning 2015-09-08 09:18) - PLID 67226 - Add an option to re-print credit card receipts
				if (IsICCPEnabled() && nPayMethod == 3) {
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintCreditCardReceipt, "Re-print &Credit Card Receipts");
				}

				long nPayID = VarLong(pRow->GetValue(btcPayID));

				_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), R"( 
							SELECT BatchPaymentsT.ERemittance, BatchPaymentsT.EOBFilePath
							FROM PaymentsT 
							LEFT JOIN BatchPaymentsT ON PaymentsT.BatchPaymentID = BatchPaymentsT.ID 
							WHERE PaymentsT.ID = {INT} 
							)", nPayID);

				// (b.spivey, October 10th, 2014) - PLID 62699 - Is this a line item we can even view an EOB for?
				if (!prs->eof) {
					BOOL bIsFromEOB = AdoFldBool(prs, "ERemittance", FALSE);
					bool bFilePathNotEmpty = AdoFldString(prs, "EOBFilepath", "").GetLength() > 0 ? true : false;
					if (bIsFromEOB) {
						// (b.spivey, October 14, 2014) - PLID 62699 - New Accelerator
						if (bFilePathNotEmpty) {
							tmpMenu.InsertMenuA(-1, MF_BYPOSITION, miViewEOBAssociatedFile, "Vie&w Associated EOB");
						}
						else {
							tmpMenu.InsertMenuA(-1, MF_BYPOSITION | MF_DISABLED, miViewEOBAssociatedFile, "Vie&w Associated EOB");
						}
					}
				}

			}
			else if (strLineType == CString("PrePayment"))
			{
				long nPayID = VarLong(pRow->GetValue(btcPayID));

				// (z.manning 2015-09-08 09:44) - PLID 67226 - Added pay method
				_RecordsetPtr prs = CreateParamRecordset(R"(
					SELECT BatchPaymentsT.ERemittance, BatchPaymentsT.EOBFilePath, PaymentsT.PayMethod
					FROM PaymentsT 
					LEFT JOIN BatchPaymentsT ON PaymentsT.BatchPaymentID = BatchPaymentsT.ID 
					WHERE PaymentsT.ID = {INT} 
					)"
					, nPayID);

				// (b.spivey, October 10th, 2014) - PLID 62699 - Is this a line item we can even view an EOB for?
				BOOL bIsFromEOB = FALSE;
				bool bFilePathNotEmpty = FALSE;
				long nPayMethod = 0;
				if (!prs->eof) {
					bIsFromEOB = AdoFldBool(prs, "ERemittance", FALSE);
					bFilePathNotEmpty = AdoFldString(prs, "EOBFilepath", "").GetLength() > 0 ? true : false;
					nPayMethod = AdoFldLong(prs, "PayMethod");
				}

				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miRelApply, "&Show Related Applies");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miEditPay, "&Edit PrePayment");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miRefundPayment, "&Refund PrePayment");

				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				// (z.manning 2010-06-10 14:49) - PLID 29214 - Transfer option
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miTransferPrePay, "Transfer PrePayment to Another Patient");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miDeletePay, "&Delete PrePayment");
				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");

				// (j.gruber 2007-03-15 14:36) - PLID 25223 - changed receipt to payment receipt and added sales receipt
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintReceipt, "Prin&t Payment Receipt");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintSalesReceipt, "Print Sales Rec&eipt");
				// (j.gruber 2007-03-29 15:17) - PLID 9802 - sales receipt receipt printer format
				//TES 12/6/2007 - PLID 28192 - Make sure we have a POS Printer.
				if (GetMainFrame()->CheckPOSPrinter()) {
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintSalesReceiptRPFormat, "Print Sales Receipt - Receipt Printer &Format");
				}
				// (z.manning 2015-09-08 09:18) - PLID 67226 - Add an option to re-print credit card receipts
				if (IsICCPEnabled() && nPayMethod == 3) {
					tmpMenu.InsertMenu(-1, MF_BYPOSITION, miPrintCreditCardReceipt, "Re-print &Credit Card Receipts");
				}

				if (bIsFromEOB) {
					// (b.spivey, October 14, 2014) - PLID 62699 - New Accelerator
					if (bFilePathNotEmpty) {
						tmpMenu.InsertMenuA(-1, MF_BYPOSITION, miViewEOBAssociatedFile, "Vie&w Associated EOB");
					}
					else {
						tmpMenu.InsertMenuA(-1, MF_BYPOSITION | MF_DISABLED, miViewEOBAssociatedFile, "Vie&w Associated EOB");
					}
				}

			}
			else if (strLineType == CString("Adjustment")) {
				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miRelApply, "&Show Related Applies");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miEditAdj, "&Edit Adjustment");

				// (j.dinatale 2011-06-02 17:32) - PLID 44809
				// (j.jones 2011-08-29 16:55) - PLID 44880 - reversed the order such that Void & Correct shows up first, Void second
				// (j.dinatale 2012-02-01 15:24) - PLID 45511 - line item corrections is no longer in beta, so dont check our flag
				// (j.jones 2014-07-14 16:03) - PLID 62876 - you cannot void chargebacks
				if (pChargeback == NULL) {

					tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");

					long nPayID = VarLong(pRow->GetValue(btcPayID));

					// (j.jones 2011-09-20 15:46) - PLID 45563 - find out if the payment is already corrected,
					// but in the event that this same ID is in multiple corrections, we want to undo the most
					// recent correction first
					BOOL bIsNewLineItemID = FALSE;
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 LineItemCorrectionsT.ID, "
						"InputDate, OriginalLineItemID, VoidingLineItemID, NewLineItemID "
						"FROM LineItemCorrectionsT "
						"WHERE OriginalLineItemID = {INT} OR VoidingLineItemID = {INT} OR NewLineItemID = {INT} "
						"ORDER BY LineItemCorrectionsT.ID DESC",
						nPayID, nPayID, nPayID);
					if(!rs->eof) {
						nCorrectionID_ToUndo = VarLong(rs->Fields->Item["ID"]->Value);
						dtCorrectionInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
						nCorrectionOriginalID = VarLong(rs->Fields->Item["OriginalLineItemID"]->Value);
						nCorrectionVoidingID = VarLong(rs->Fields->Item["VoidingLineItemID"]->Value);
						nCorrectionNewID = VarLong(rs->Fields->Item["NewLineItemID"]->Value, -1);	//could be null

						//is our source payment this correction's new payment?
						bIsNewLineItemID = (nPayID == nCorrectionNewID);
					}
					rs->Close();

					// (j.jones 2011-09-20 15:46) - PLID 45563 - if this is part of a correction, give
					// the option to undo that correction
					if(nCorrectionID_ToUndo != -1) {
						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miUndoPaymentCorrection, "Undo Correction");
					}
					
					// (j.jones 2011-09-20 15:45) - PLID 45563 - we give the correction options
					// only if the line item is not part of a correction OR it is the new line
					// item in that correction
					if(nCorrectionID_ToUndo == -1 || bIsNewLineItemID) {
						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miCorrectPayment, "Void and Correct Adjustment");
						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miVoidPayment, "Void Adjustment");
					}
				}

				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				// (z.manning 2010-06-10 14:49) - PLID 29214 - Transfer option
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miTransferAdj, "Transfer Adjustment to Another Patient");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miDeleteAdj, "&Delete Adjustment");

				long nAdjustmentID = VarLong(pRow->GetValue(btcPayID));

				_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), R"( 
							SELECT BatchPaymentsT.ERemittance, BatchPaymentsT.EOBFilePath
							FROM PaymentsT 
							LEFT JOIN BatchPaymentsT ON PaymentsT.BatchPaymentID = BatchPaymentsT.ID 
							WHERE PaymentsT.ID = {INT} 
							)", nAdjustmentID);
				// (b.spivey, October 10th, 2014) - PLID 62699 - Is this a line item we can even view an EOB for?
				if (!prs->eof) {
					BOOL bIsFromEOB = AdoFldBool(prs, "ERemittance", FALSE);
					bool bFilePathNotEmpty = AdoFldString(prs, "EOBFilepath", "").GetLength() > 0 ? true : false;
					if (bIsFromEOB) {
						// (b.spivey, October 14, 2014) - PLID 62699 - New Accelerator
						if (bFilePathNotEmpty) {
							tmpMenu.InsertMenuA(-1, MF_BYPOSITION, miViewEOBAssociatedFile, "Vie&w Associated EOB");
						}
						else {
							tmpMenu.InsertMenuA(-1, MF_BYPOSITION | MF_DISABLED, miViewEOBAssociatedFile, "Vie&w Associated EOB");
						}
					}
				}

			}
			else if (strLineType == CString("Refund")) {
				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miRelApply, "&Show Related Applies");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miEditRef, "&Edit Refund");

				// (j.dinatale 2011-06-02 17:32) - PLID 44809
				// (j.jones 2011-08-29 16:55) - PLID 44880 - reversed the order such that Void & Correct shows up first, Void second
				// (j.dinatale 2012-02-01 15:24) - PLID 45511 - line item corrections is no longer in beta, so dont check our flag
				// (j.jones 2014-07-14 16:03) - PLID 62876 - you cannot void chargebacks
				if (pChargeback == NULL) {
					tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
					
					long nPayID = VarLong(pRow->GetValue(btcPayID));

					// (j.jones 2011-09-20 15:46) - PLID 45563 - find out if the payment is already corrected,
					// but in the event that this same ID is in multiple corrections, we want to undo the most
					// recent correction first
					BOOL bIsNewLineItemID = FALSE;
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 LineItemCorrectionsT.ID, "
						"InputDate, OriginalLineItemID, VoidingLineItemID, NewLineItemID "
						"FROM LineItemCorrectionsT "
						"WHERE OriginalLineItemID = {INT} OR VoidingLineItemID = {INT} OR NewLineItemID = {INT} "
						"ORDER BY LineItemCorrectionsT.ID DESC",
						nPayID, nPayID, nPayID);
					if(!rs->eof) {
						nCorrectionID_ToUndo = VarLong(rs->Fields->Item["ID"]->Value);
						dtCorrectionInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
						nCorrectionOriginalID = VarLong(rs->Fields->Item["OriginalLineItemID"]->Value);
						nCorrectionVoidingID = VarLong(rs->Fields->Item["VoidingLineItemID"]->Value);
						nCorrectionNewID = VarLong(rs->Fields->Item["NewLineItemID"]->Value, -1);	//could be null

						//is our source payment this correction's new payment?
						bIsNewLineItemID = (nPayID == nCorrectionNewID);
					}
					rs->Close();

					// (j.jones 2011-09-20 15:46) - PLID 45563 - if this is part of a correction, give
					// the option to undo that correction
					if(nCorrectionID_ToUndo != -1) {
						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miUndoPaymentCorrection, "Undo Correction");
					}
					
					// (j.jones 2011-09-20 15:45) - PLID 45563 - we give the correction options
					// only if the line item is not part of a correction OR it is the new line
					// item in that correction
					if(nCorrectionID_ToUndo == -1 || bIsNewLineItemID) {
						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miCorrectPayment, "Void and Correct Refund");
						tmpMenu.InsertMenu(-1, MF_BYPOSITION, miVoidPayment, "Void Refund");
					}
				}

				tmpMenu.InsertMenu(-1, MF_SEPARATOR, 2, "");
				// (z.manning 2010-06-10 14:49) - PLID 29214 - Transfer option
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miTransferRef, "Transfer Refund to Another Patient");
				tmpMenu.InsertMenu(-1, MF_BYPOSITION, miDeleteRef, "&Delete Refund");
			}
		}

		//pMenu = tmpMenu;
		//if (pMenu != NULL)
		int nResult = tmpMenu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD,point.x, point.y,this);
		if(nResult > 0) {

			//if positive, it must be an insured party to print to
			long BillID = VarLong(pRow->GetValue(m_nBillIDColID),-1);
			PrintClaimForInsurance(BillID, nResult);
		}
		else {

			switch(nResult) {

			case miNewBill:
				{
					try {
						OnNewBill();
					}NxCatchAll("Error creating new bill.");
				}
				break;

			case miNewPay:
				{
					try {
						CreateNewPayment();
					}NxCatchAll("Error creating new payment.");
				}
				break;
				// (j.jones 2009-08-21 17:49) - PLID 24569 - added pre-payment option
			case miNewPrePay:
				{
					try {
						CreateNewPayment(TRUE);
					}NxCatchAll("Error creating new pre-payment.");
				}
				break;
			case miNewAdj:
				{
					try {
						CreateNewAdjustment();
					}NxCatchAll("Error creating new adjustment.");
				}
				break;
			case miNewRef:
				{
					try {
						CreateNewRefund();
					}NxCatchAll("Error creating new refund.");
				}
				break;

			case miPaymentProfile:
				// (z.manning 2015-07-22 10:34) - PLID 67241
				OpenPaymentProfileDlg(GetActivePatientID(), this);
				break;

			case miNewCharge:	// Edit charges
				{
					try {
						if(pRow) {
							// (j.jones 2007-04-19 11:20) - PLID 25721 - In most cases CanEdit is not needed, so first
							// silently see if they have permission, and if they do, then move ahead normally. But if
							// they don't, or they need a password, then check CanEdit prior to the permission check that
							// would stop or password-prompt the user.
							// (c.haag 2009-03-10 11:06) - PLID 32433 - We now use CanChangeHistoricFinancial
							if (pRow->GetValue(m_nBillIDColID).vt != VT_EMPTY) {
								// (a.walling 2009-12-22 17:59) - PLID 7002 - Maintain only one instance of a bill
								if (CanChangeHistoricFinancial("Bill", VarLong(pRow->GetValue(m_nBillIDColID),-1), bioBill, sptRead) && !GetMainFrame()->IsBillingModuleOpen(true)) {
									//if((GetCurrentUserPermissions(bioBill) & sptRead) || CanEdit("Bill", VarLong(pRow->GetValue(m_nBillIDColID),-1)) || CheckCurrentUserPermissions(bioBill,sptRead)) {
									COleVariant var;
									//CBillingModuleDlg BillingDlg;
									var = pRow->GetValue(m_nBillIDColID);
									m_BillingDlg->m_pFinancialDlg = this;		//DRT 12/27/2004 - PLID 15083 - See comments in OnNewBill
									m_boAllowUpdate = FALSE;
									m_BillingDlg->OpenWithBillID(VarLong(var), BillEntryType::Bill, 2);
									m_boAllowUpdate = TRUE;

									//the bill is redrawn in PostEditBill(2)
								}
							}
						}
					}NxCatchAll("Error editing charges.");
				}
				break;

			case miDeleteBill:	// Delete bill
				{
					try {
						if(pRow) {
							long nBillID = VarLong(pRow->GetValue(m_nBillIDColID));

							// (j.jones 2011-09-13 15:37) - PLID 44887 - you can't delete this bill
							// if any charge in it is original or void
							if(IsVoidedBill(nBillID)) {
								AfxMessageBox("This bill has been corrected, and can not be deleted.");
								return;
							}
							else if(DoesBillHaveOriginalOrVoidCharges(nBillID)) {
								AfxMessageBox("At least one charge in this bill has been corrected, and can not be deleted.");
								return;
							}
							else if(DoesBillHaveOriginalOrVoidApplies(nBillID)) {
								AfxMessageBox("At least one apply to this bill has been corrected, and can not be deleted.");
								return;
							}

							//TES 7/25/2014 - PLID 63049 - Don't allow them to delete chargeback charges
							if (DoesChargeHaveChargeback(CSqlFragment("ChargesT.BillID = {INT}", nBillID))) {
								MessageBox("This bill is associated with at least one Chargeback, and cannot be deleted. In order to delete this bill, you must first expand the bill "
									"on the Billing tab, right-click on any associated Chargebacks, and select 'Undo Chargeback.'");
								return;
							}

							// (j.jones 2007-04-19 11:21) - PLID 25721 - In most cases CanEdit is not needed, so first
							// silently see if they have permission, and if they do, then move ahead normally. But if
							// they don't, or they need a password, then check CanEdit prior to the permission check that
							// would stop or password-prompt the user.
							// (c.haag 2009-03-10 10:45) - PLID 32433 - Use CanChangeHistoricFinancial
							if (CanChangeHistoricFinancial("Bill", nBillID, bioBill, sptDelete)) {
								//if((GetCurrentUserPermissions(bioBill) & sptDelete) || CanEdit("Bill", nBillID) || CheckCurrentUserPermissions(bioBill,sptDelete)) {

								CString str = "Are you sure you wish to permanently delete this bill?";

								// (j.jones 2009-12-29 11:01) - PLID 27237 - replaced ReturnsRecords with a parameterized recordset
								// (j.dinatale 2012-07-11 09:26) - PLID 51468 - can now have multiple bills on PICs
								_RecordsetPtr rsProcInfo = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 1 FROM ProcInfoBillsT WHERE BillID = {INT}", nBillID);
								if(!rsProcInfo->eof) {
									//attached to a procedure
									str = "This bill is attached to a tracked procedure. Are you sure you wish to permanently delete this bill?";
								}
								rsProcInfo->Close();

								if (IDNO == MessageBox(str, "NexTech", MB_YESNO))
									break;

								// (j.jones 2013-07-23 10:03) - PLID 46493 - these functions have a void flag now, always false here

								// (j.jones 2015-11-04 09:36) - PLID 67459 - flipped the return product warning and product item warning
								
								// (j.jones 2008-06-09 17:20) - PLID 28392 - CheckWarnAlteringReturnedProducts will warn yet again
								// if there are products returned from this bill
								if(!CheckWarnAlteringReturnedProducts(true, false, nBillID)) {
									break;
								}

								// (j.jones 2008-05-27 17:06) - PLID 27982 - CheckWarnDeletingChargedProductItems will potentially
								// warn a second time, if there are existing saved product items linked to the bill
								if (!CheckWarnDeletingChargedProductItems(true, false, nBillID)) {
									break;
								}

								CWaitCursor pWait;

								//DRT 4/6/2004 - PLID 11799 - We have to check the return value of
								//	this function before removing things from the view.
								if(!DeleteBill(nBillID))
									break;

								for (int i=0; i < m_aiExpandedBranches.GetSize(); i++) {
									if (m_aiExpandedBranches.GetAt(i) == nBillID) {
										m_aiExpandedBranches.RemoveAt(i);

										// (j.jones 2015-03-17 13:25) - PLID 64931 - clear the search results window
										ClearSearchResults();
										break;
									}
								}

								// Delete bill and its children from list
								for(int q=g_aryFinancialTabInfoT.GetSize()-1;q>=0;q--) {
									if(((FinTabItem*)g_aryFinancialTabInfoT.GetAt(q))->BillID.vt == VT_I4
										&& ((FinTabItem*)g_aryFinancialTabInfoT.GetAt(q))->BillID.lVal == nBillID) {
											delete (FinTabItem*)g_aryFinancialTabInfoT.GetAt(q);
											g_aryFinancialTabInfoT.RemoveAt(q);
									}
								}
								// Since this may have resulted in unapplied payments, redraw them
								RedrawAllPayments();
								CalculateTotals();
								//ResetBrightenedRow();
							}
						}
					}NxCatchAll("Error deleting bill.");
				}
				break;

			case miDeleteCharge:	// Delete charge
				{
					try {
						if(pRow) {

							// (j.jones 2007-04-19 11:21) - PLID 25721 - In most cases CanEdit is not needed, so first
							// silently see if they have permission, and if they do, then move ahead normally. But if
							// they don't, or they need a password, then check CanEdit prior to the permission check that
							// would stop or password-prompt the user.
							// (c.haag 2009-03-10 10:45) - PLID 32433 - Use CanChangeHistoricFinancial
							if (pRow->GetValue(m_nBillIDColID).vt != VT_EMPTY) {

								COleVariant var = pRow->GetValue(btcChargeID); // var=ChargeID
								long nChargeID = VarLong(var);

								// (j.jones 2011-09-13 15:37) - PLID 44887 - you can't delete this charge
								// if any charge in it is original or void
								LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nChargeID);
								if(licsStatus == licsOriginal) {
									AfxMessageBox("This charge has been corrected, and can not be deleted.");
									return;
								}
								else if(licsStatus == licsVoid) {
									AfxMessageBox("This charge is a void charge from an existing correction, and can not be deleted.");
									return;
								}
								else if(DoesLineItemHaveOriginalOrVoidApplies(nChargeID)) {
									AfxMessageBox("At least one apply to this charge has been corrected, and can not be deleted.");
									return;
								}
								//TES 8/22/2014 - PLID 63049 - Don't allow deleting chargebacks
								else if (DoesChargeHaveChargeback(CSqlFragment("ChargesT.ID = {INT}", nChargeID))) {
									MessageBox("This charge is associated with at least one Chargeback, and cannot be deleted. "
										"In order to make changes to this charge, you must first right-click on any associated Chargebacks, and select 'Undo Chargeback.'");
									return;
								}

								if (CanChangeHistoricFinancial("Bill", VarLong(pRow->GetValue(m_nBillIDColID),-1), bioBill, sptDelete)) {
									//if((GetCurrentUserPermissions(bioBill) & sptDelete) || CanEdit("Bill", VarLong(pRow->GetValue(m_nBillIDColID),-1)) || CheckCurrentUserPermissions(bioBill,sptDelete)) {
									if (IDNO == MessageBox("Are you sure you wish to delete this charge?", "NexTech", MB_YESNO))
										break;

									CWaitCursor pWait;

									long iBillID = pRow->GetValue(m_nBillIDColID).lVal;

									// (j.jones 2005-04-05 17:47) - PLID 11966 - ensure they cannot make a bill into a state
									// where all of its charges are not batched (if there are no others, allow the deletion)
									// (j.jones 2009-03-03 09:44) - PLID 33174 - allow this now, but only if they are hiding
									// patient charges on claims
									BOOL bHidePatientChargesOnClaims = (GetRemotePropertyInt("DisallowBatchingPatientClaims",0,0,"<None>",TRUE) == 1 &&
										GetRemotePropertyInt("HidePatientChargesOnClaims",0,0,"<None>",TRUE) == 1);

									if(!bHidePatientChargesOnClaims) {

										// (j.jones 2009-12-29 11:01) - PLID 27237 - replaced with a parameterized recordset
										// (j.jones 2011-05-18 12:09) - PLID 44952 - we will filter out "original" and "void" charges
										_RecordsetPtr rsOnlyUnbatched = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT "
											"Sum(CASE WHEN Batched = 0 THEN 1 ELSE 0 END) AS CountUnbatched, "
											"Sum(CASE WHEN Batched = 1 THEN 1 ELSE 0 END) AS CountBatched "
											"FROM BillsT "
											"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
											"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
											"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalChargesQ ON LineItemT.ID = LineItemCorrections_OriginalChargesQ.OriginalLineItemID "
											"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingChargesQ ON LineItemT.ID = LineItemCorrections_VoidingChargesQ.VoidingLineItemID "
											"WHERE LineItemT.Deleted = 0 "
											"AND BillsT.ID = {INT} "
											"AND ChargesT.ID <> {INT} "
											"AND LineItemCorrections_OriginalChargesQ.OriginalLineItemID Is Null "
											"AND LineItemCorrections_VoidingChargesQ.VoidingLineItemID Is Null "
											"GROUP BY BillsT.ID ", iBillID, VarLong(var));
										if(!rsOnlyUnbatched->eof) {

											long nCountUnbatched = AdoFldLong(rsOnlyUnbatched, "CountUnbatched", 0);
											long nCountBatched = AdoFldLong(rsOnlyUnbatched, "CountBatched", 0);
											if(nCountBatched == 0 && nCountUnbatched > 0) {
												AfxMessageBox("You cannot delete this charge, because the remaining charges on this bill are not marked as Batched.\n"
													"A bill must always have at least one charge that is marked as being Batched.");
												break;
											}
										}
										rsOnlyUnbatched->Close();
									}

									// (j.jones 2013-07-23 10:03) - PLID 46493 - these functions have a void flag now, always false here

									// (j.jones 2015-11-04 09:36) - PLID 67459 - flipped the return product warning and product item warning
									
									// (j.jones 2008-06-09 17:20) - PLID 28392 - CheckWarnAlteringReturnedProducts will warn yet again
									// if there are products returned from this charge
									if(!CheckWarnAlteringReturnedProducts(false, false, VarLong(var))) {
										break;
									}

									// (j.jones 2008-05-27 17:06) - PLID 27982 - CheckWarnDeletingChargedProductItems will potentially
									// warn a second time, if there are existing saved product items linked to the charge
									if (!CheckWarnDeletingChargedProductItems(false, false, VarLong(var))) {
										break;
									}

									DeleteCharge(VarLong(var), iBillID, TRUE);
									RedrawBill(iBillID);
									// Since this may have resulted in unapplied payments, redraw them
									RedrawAllPayments();
									CalculateTotals();
									//ResetBrightenedRow();
								}
							}
						}
					}NxCatchAll("Error deleting charge.");
				}
				break;

			// (j.gruber 2011-05-27 13:20) - PLID 44832
			case miCorrectCharge:
				{
					try {
						// (j.dinatale 2011-06-27 10:20) - PLID 44812
						if(!CheckCurrentUserPermissions(bioFinancialCorrections, sptWrite)){
							return;
						}

						if (pRow) {
							
							long nChargeID = VarLong(pRow->GetValue(btcChargeID)); 
							// (z.manning 2012-10-16 17:27) - PLID 53215 - Use the member for bill ID column index, not the enum
							long nBillID = VarLong(pRow->GetValue(m_nBillIDColID));

							// (j.jones 2011-09-13 15:37) - PLID 44887 - do not allow re-correcting
							// an already corrected charge
							LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nChargeID);
							if(licsStatus == licsOriginal) {
								AfxMessageBox("This charge has already been corrected, and can no longer be modified.");
								return;
							}
							else if(licsStatus == licsVoid) {
								AfxMessageBox("This charge is a void charge from an existing correction, and cannot be modified.");
								return;
							}

							// (j.dinatale 2011-06-28 10:53) - PLID 44808 - need prompts to let the user know what is going on and when they should be using this
							//		etc, etc, etc.
							if(IDCANCEL == AfxMessageBox("This feature should be used to correct the charge provider when the charge is no longer editable. Use the Correct Bill option to correct the Location.\r\n"
								"\r\n"
								"NOTE: If any payments or adjustments are incorrect or are applied incorrectly, DO NOT correct this charge. Instead "
								"correct the incorrect line item.", MB_OKCANCEL)){
									return;
							}

							// (b.spivey, September 26, 2014) - PLID 63652 - Prompt with a warning about lockbox payments.
							CString strLockboxError = "";

							if (g_pLicense->CheckForLicense(CLicense::lcLockboxPayments, CLicense::cflrSilent)) {
								strLockboxError = "\r\n- If any payments are linked to a lockbox deposit then those lockbox payments will be marked as manually posted. This is not reversible. \r\n";
							}
							CString strErrorMessage = "";
							strErrorMessage.Format(
								"The following actions will occur:\r\n"
								"\r\n"
								"- A new adjustment will be created to offset the original charge.\r\n"
								"- Any payments or adjustments applied to the charge will also be voided and corrected.\r\n"
								"- A new negative charge will be created to cancel the original charge.\r\n"
								"- Another new charge will be created which will replace the original charge." 
								"%s"
								, strLockboxError);

							if (IDCANCEL == AfxMessageBox(strErrorMessage, MB_OKCANCEL)){
									return;
							}

							// (j.jones 2011-10-19 08:53) - PLID 45462 - changed the warning because an Undo now makes this reversible
							if(IDYES != AfxMessageBox("This correction is only reversible using the Undo Correction feature.\n"
								"Are you sure you want to perform this correction?", MB_YESNO)){
								return;
							}

							CFinancialCorrection finCor;

							CString strCurrentUserName = GetCurrentUserName();
							long nCurrentUserID = GetCurrentUserID();

							finCor.AddCorrection(ctCharge, nChargeID, strCurrentUserName, nCurrentUserID);						

							finCor.ExecuteCorrections();
							QuickRefresh();
						}	
					}NxCatchAll("Error Correcting Charge");
				}
			break;
			// (j.gruber 2011-07-05 12:18) - PLID 44832
			case miVoidCharge:
				{
					try {
						// (j.dinatale 2011-06-27 10:20) - PLID 44812
						if(!CheckCurrentUserPermissions(bioFinancialCorrections, sptWrite)){
							return;
						}

						if (pRow) {
							
							long nChargeID = VarLong(pRow->GetValue(btcChargeID)); 
							// (z.manning 2012-10-16 17:27) - PLID 53215 - Use the member for bill ID column index, not the enum
							long nBillID = VarLong(pRow->GetValue(m_nBillIDColID));

							// (j.jones 2011-09-13 15:37) - PLID 44887 - do not allow re-correcting
							// an already corrected charge
							LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nChargeID);
							if(licsStatus == licsOriginal) {
								AfxMessageBox("This charge has already been corrected, and can no longer be modified.");
								return;
							}
							else if(licsStatus == licsVoid) {
								AfxMessageBox("This charge is a void charge from an existing correction, and cannot be modified.");
								return;
							}

							// (j.dinatale 2011-09-30 09:00) - PLID 44380 - need to make sure the charge we are attempting to void isn't a gift certificate
							//	with payments against it
							// (r.gonet 2015-05-05 09:53) - PLID 65326 - Refunds can now have gift IDs, so exclude those. 
							if(ReturnsRecordsParam(
								"SELECT TOP 1 1 FROM LineItemT "
								"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
								"INNER JOIN ( "
								"	SELECT GiftID FROM LineItemT "
								"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
								"	WHERE LineItemT.Deleted = 0 AND LineItemT.GiftID IS NOT NULL AND LineItemT.Type = 1 "
								") GCWithPayments ON LineItemT.GiftID = GCWithPayments.GiftID "
								"WHERE LineItemT.Deleted = 0 AND LineItemT.GiftID IS NOT NULL AND LineItemT.ID = {INT}", nChargeID)){
									AfxMessageBox("The charge you are attempting to void is a gift certificate with payments against it. You are unable to void this charge, but you may void & correct it.");
									return;
							}

							// (j.jones 2013-07-23 10:19) - PLID 46493 - check if they have returned products
							// or any serialized items, and warn accordingly

							// (j.jones 2015-11-04 09:36) - PLID 67459 - flipped the return product warning and product item warning

							if (!CheckWarnAlteringReturnedProducts(false, true, nChargeID)) {
								return;
							}

							if (!CheckWarnDeletingChargedProductItems(false, true, nChargeID)) {
								return;
							}

							// (j.dinatale 2011-06-28 10:53) - PLID 44808 - need prompts to let the user know what is going on and when they should be using this
							//		etc, etc, etc.
							if(IDCANCEL == AfxMessageBox("This feature should be used when the charge is entirely incorrect and cannot be deleted or removed.\r\n"
								"\r\n"
								"NOTE: If any payments or adjustments are incorrect or are applied incorrectly, DO NOT void this charge. Instead void the incorrect line items.", MB_OKCANCEL)){
									return;
							}


							// (b.spivey, September 26, 2014) - PLID 63652 - Prompt with a warning about lockbox payments.
							CString strLockboxError = "";

							if (g_pLicense->CheckForLicense(CLicense::lcLockboxPayments, CLicense::cflrSilent)) {
								strLockboxError = "\r\n- If any payments are linked to a lockbox deposit then those lockbox payments will be marked as manually posted. This is not reversible. \r\n";
							}
							CString strErrorMessage = "";
							strErrorMessage.Format(
								"The following actions will occur:\r\n"
								"\r\n"
								"- A new adjustment will be created to offset the original charge.\r\n"
								"- Any payments or adjustments applied to the original charge will also be voided. If you do not wish to void a payment or adjustment, use the Void and Correct Payment/Adjustment option before voiding the charge.\r\n"
								"- A new negative charge will be created to cancel the original charge." 
								"%s"
								, strLockboxError);

							if (IDCANCEL == AfxMessageBox(strErrorMessage, MB_OKCANCEL)){
									return;
							}

							// (j.jones 2011-10-19 08:53) - PLID 45462 - changed the warning because an Undo now makes this reversible
							// (j.dinatale 2011-11-01 12:23) - PLID 44923 - if there are any linked items to this charge, warn that the items are going to be unlinked
							// (j.jones 2013-06-26 11:33) - PLID 57297 - charges with serialized products cannot be undone if you are
							// only voiding (void & correct is still undoable), so tell them this
							if(ReturnsRecordsParam("SELECT TOP 1 1 FROM ChargedProductItemsT WHERE ChargeID = {INT}", nChargeID)) {
								if(IDYES != MessageBox("This charge is for a product associated with serial numbers or expiration dates. "
									"Voiding this charge will release these products back into Inventory.\n\n"
									"You will not be able to use the Undo Correction feature on this charge. "
									"A new charge would need to be added, and the products would need to be re-selected.\n\n"
									"Are you sure you want to perform this irreversible correction?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)){
									return;
								}
							}
							else if(IDYES != AfxMessageBox("This correction is only reversible using the Undo Correction feature.\n"
								"Are you sure you want to perform this correction?", MB_YESNO)){
								return;
							}

							CFinancialCorrection finCor;

							CString strCurrentUserName = GetCurrentUserName();
							long nCurrentUserID = GetCurrentUserID();

							finCor.AddCorrection(ctCharge, nChargeID, strCurrentUserName, nCurrentUserID, FALSE);

							finCor.ExecuteCorrections();
							QuickRefresh();
						}	
					}NxCatchAll("Error Voiding Charge");
				}
			break;
			// (s.dhole 2011-06-02 09:59) - PLID 44832 Bill Correction menu
			case miCorrectBill:
				{
					try {
						// (j.dinatale 2011-06-27 10:20) - PLID 44812
						if(!CheckCurrentUserPermissions(bioFinancialCorrections, sptWrite)){
							return;
						}

						if (pRow) {
												
							// (z.manning 2012-10-16 17:27) - PLID 53215 - Use the member for bill ID column index, not the enum
							long nBillID = VarLong(pRow->GetValue(m_nBillIDColID));

							// (j.jones 2011-09-13 15:37) - PLID 44887 - do not allow re-correcting
							// an already corrected bill
							if(IsVoidedBill(nBillID)) {
								AfxMessageBox("This bill has already been corrected, and can no longer be modified.");
								return;
							}
							//You can correct a bill that has corrected charges, provided that there is at least one
							//uncorrected charge in the bill. So if there is no uncorrected charge, you can't correct the bill.
							else if(!DoesBillHaveUncorrectedCharges(nBillID)) {
								AfxMessageBox("All charges in this bill have already been corrected, and can no longer be modified.\n"
									"This bill cannot be corrected.");
								return;
							}

							// (j.dinatale 2011-06-28 10:53) - PLID 44808 - need prompts to let the user know what is going on and when they should be using this
							//		etc, etc, etc.
							if(IDCANCEL == AfxMessageBox("This feature should be used to correct the bill location when the bill is no longer editable. Use the Correct Charge option to correct the Provider.\r\n"
								"\r\n"
								"NOTE: If any payments or adjustments are incorrect or are incorrectly applied, DO NOT correct this bill. If the charge provider "
								"is incorrect on one or more charges, again DO NOT correct this bill. Correct the incorrect line items instead.", MB_OKCANCEL)){
									return;
							}

							// (b.spivey, September 26, 2014) - PLID 63652 - Prompt with a warning about lockbox payments.
							CString strLockboxError = "";

							if (g_pLicense->CheckForLicense(CLicense::lcLockboxPayments, CLicense::cflrSilent)) {
								strLockboxError = "\r\n- If any payments are linked to a lockbox deposit then those lockbox payments will be marked as manually posted. This is not reversible. \r\n";
							}
							CString strErrorMessage = "";
							strErrorMessage.Format(
								"The following actions will occur:\r\n"
								"\r\n"
								"- A new adjustment will be created to offset the amount of each original charge.\r\n"
								"- Any payments or adjustments attached to the original charges will also be voided and corrected.\r\n"
								"- New negative charges will be created to cancel each original charge.\r\n"
								"- A new bill will be created with new charges representing the original charges." 
								"%s"
								, strLockboxError);

							if(IDCANCEL == AfxMessageBox(strErrorMessage, MB_OKCANCEL)){
									return;
							}

							// (j.jones 2011-10-19 08:53) - PLID 45462 - changed the warning because an Undo now makes this reversible
							if(IDYES != AfxMessageBox("This correction is only reversible using the Undo Correction feature.\n"
								"Are you sure you want to perform this correction?", MB_YESNO)){
								return;
							}

							CFinancialCorrection finCor;

							CString strCurrentUserName = GetCurrentUserName();
							long nCurrentUserID = GetCurrentUserID();

							finCor.AddCorrection(ctBill, nBillID, strCurrentUserName, nCurrentUserID);
							
							finCor.ExecuteCorrections();
							QuickRefresh();
						}	
					}NxCatchAll("Error Correcting Bill");
				}
				break;
				case miVoidBill:
				{
					try {
						// (j.dinatale 2011-06-27 10:20) - PLID 44812
						if(!CheckCurrentUserPermissions(bioFinancialCorrections, sptWrite)){
							return;
						}

						if (pRow) {
							
							// (z.manning 2012-10-16 17:27) - PLID 53215 - Use the member for bill ID column index, not the enum
							long nBillID = VarLong(pRow->GetValue(m_nBillIDColID));

							// (j.jones 2011-09-13 15:37) - PLID 44887 - do not allow re-correcting
							// an already corrected bill
							if(IsVoidedBill(nBillID)) {
								AfxMessageBox("This bill has already been corrected, and can no longer be modified.");
								return;
							}
							//You can correct a bill that has corrected charges, provided that there is at least one
							//uncorrected charge in the bill. So if there is no uncorrected charge, you can't correct the bill.
							else if(!DoesBillHaveUncorrectedCharges(nBillID)) {
								AfxMessageBox("All charges in this bill have already been corrected, and can no longer be modified.\n"
									"This bill cannot be voided.");
								return;
							}

							// (j.dinatale 2011-09-30 09:00) - PLID 44380 - need to make sure the bill we are attempting to void doesnt contain a gift certificate
							//	with payments against it
							// (r.gonet 2015-05-05 09:53) - PLID 65326 - Refunds can now have gift IDs, so exclude those.
							if(ReturnsRecordsParam(
								"SELECT TOP 1 1 FROM LineItemT "
								"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
								"INNER JOIN ( "
								"	SELECT GiftID FROM LineItemT "
								"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
								"	WHERE LineItemT.Deleted = 0 AND LineItemT.GiftID IS NOT NULL AND LineItemT.Type = 1 "
								") GCWithPayments ON LineItemT.GiftID = GCWithPayments.GiftID "
								"WHERE LineItemT.Deleted = 0 AND LineItemT.GiftID IS NOT NULL AND ChargesT.BillID = {INT}", nBillID)){
									AfxMessageBox("The bill you are attempting to void has a charge for a gift certificate with payments against it. You are unable to void this bill, but you may void & correct it. Otherwise, you must void all other charges on the bill other than the GC charge.");
									return;
							}

							// (j.jones 2013-07-23 10:19) - PLID 46493 - check if they have returned products
							// or any serialized items, and warn accordingly

							// (j.jones 2015-11-04 09:36) - PLID 67459 - flipped the return product warning and product item warning

							if (!CheckWarnAlteringReturnedProducts(true, true, nBillID)) {
								return;
							}

							if (!CheckWarnDeletingChargedProductItems(true, true, nBillID)) {
								return;
							}

							// (j.dinatale 2011-06-28 10:53) - PLID 44808 - need prompts to let the user know what is going on and when they should be using this
							//		etc, etc, etc.
							if(IDCANCEL == AfxMessageBox("This feature should be used when the bill is entirely incorrect and cannot be deleted or removed.\r\n"
								"\r\n"
								"NOTE: If any payments, adjustments, or charges are incorrect, DO NOT void this bill. Void the incorrect line items instead.", MB_OKCANCEL)){
									return;
							}

							// (b.spivey, September 26, 2014) - PLID 63652 - Prompt with a warning about lockbox payments.
							CString strLockboxError = "";

							if (g_pLicense->CheckForLicense(CLicense::lcLockboxPayments, CLicense::cflrSilent)) {
								strLockboxError = "\r\n- If any payments are linked to a lockbox deposit then those lockbox payments will be marked as manually posted. This is not reversible. \r\n";
							}
							CString strErrorMessage = "";
							strErrorMessage.Format(
								"The following actions will occur:\r\n"
								"\r\n"
								"- A new adjustment will be created to offset the amount of each original charge.\r\n"
								"- Any payments or adjustments applied to the original charges will also be voided. If you do not wish to void a payment or adjustment, use the Correct Payment/Adjustment option before voiding the bill.\r\n"
								"- New negative charges will be created to cancel each original charge."
								"%s"
								, strLockboxError);

							if (IDCANCEL == AfxMessageBox(strErrorMessage, MB_OKCANCEL)){
									return;
							}

							// (j.jones 2011-10-19 08:53) - PLID 45462 - changed the warning because an Undo now makes this reversible
							// (j.dinatale 2011-11-01 12:23) - PLID 44923 - if there are any linked items to this bill, warn that the items are going to be unlinked
							// (j.jones 2013-06-26 11:33) - PLID 57297 - bills that have charges with serialized products cannot be undone if you are
							// only voiding (void & correct is still undoable), so tell them this
							if(ReturnsRecordsParam("SELECT TOP 1 1 FROM ChargedProductItemsT LEFT JOIN ChargesT ON ChargedProductItemsT.ChargeID = ChargesT.ID WHERE BillID = {INT}", nBillID)){
								if(IDYES != MessageBox("This bill has charges for products associated with serial numbers or expiration dates. "
									"Voiding this bill will release these products back into Inventory.\n\n"
									"You will not be able to use the Undo Correction feature on this bill. "
									"A new bill would need to be added, and the products would need to be re-selected.\n\n"
									"Are you sure you want to perform this irreversible correction?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)){
									return;
								}
							}
							else if(IDYES != AfxMessageBox("This correction is only reversible using the Undo Correction feature.\n"
								"Are you sure you want to perform this correction?", MB_YESNO)){
								return;
							}

							// (j.gruber 2011-07-05 12:18) - PLID 44832
							CFinancialCorrection finCor;

							CString strCurrentUserName = GetCurrentUserName();
							long nCurrentUserID = GetCurrentUserID();

							finCor.AddCorrection(ctBill, nBillID, strCurrentUserName, nCurrentUserID, FALSE);
							
							finCor.ExecuteCorrections();
							QuickRefresh();
						}	
					}NxCatchAll("Error Voiding Bill");
				}
				break;
			// (j.dinatale 2011-06-02 17:34) - PLID 44809 - Correct a payment
			case miCorrectPayment:
				{
					try {

						if (pRow) {

							long nPayID = -1;
							if (strLineType == CString("Applied") || strLineType == CString("InsuranceApplied") || strLineType == CString("AppliedPayToPay")) {								
								_variant_t varApplyID = pRow->GetValue(btcApplyID);
								if(varApplyID.vt == VT_I4 && VarLong(varApplyID,-1) != -1) {
									_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT SourceID FROM AppliesT WHERE ID = {INT}", VarLong(varApplyID));
									if(!rs->eof) {
										nPayID = rs->Fields->Item["SourceID"]->Value.lVal;
									}
									rs->Close();
								}
								else {
									nPayID = VarLong(pRow->GetValue(btcPayID));
								}
							}
							else {
								nPayID = VarLong(pRow->GetValue(btcPayID));
							}

							CString strLineItemTypeName;
							if (strLineType == CString("Applied") || strLineType == CString("InsuranceApplied") || strLineType == CString("AppliedPayToPay")){
								ASSERT(strPayType.CompareNoCase("Item") != 0);
								strLineItemTypeName = strPayType.MakeLower();

								// (j.dinatale 2011-09-13 17:30) - PLID 44808 - the reason why this is being changed to payment in the prompt is to avoid
								//		having to query yet again to get whether the payment is a prepayment or not, so to be consistent when prompting for an
								//		unapplied prepayment (since all the prompts call it a payment) we change it back.
								if(strLineItemTypeName == "prepayment"){
									strLineItemTypeName = "payment";
								}
							}else{
								strLineItemTypeName = strLineType.MakeLower();
							}
							
							// (b.spivey, September 11, 2014) - PLID 63652 - If the payment is linked, warn them that it'll permanently unlink
							if (IsLinkedToLockbox(nPayID)) {
								CString strTempMessage;
								strTempMessage.Format("This payment is connected to a lockbox deposit. If you void and correct this payment the lockbox deposit line item will be marked "
									"as manually posted and unassociated with this payment. \r\n\r\n"
									"Are you sure you want to void and correct this payment?");

								// (b.spivey, October 3, 2014) - PLID 63652 - Yes/No, not OK/Cancel
								if (IDNO == AfxMessageBox(strTempMessage, MB_YESNO)) {
									return;
								}
							}

							// (j.jones 2013-07-01 13:34) - PLID 55517 - this is now all a modular function,
							// including checking permissions
							if(VoidAndCorrectPayAdjRef(nPayID, strLineItemTypeName)) {
								QuickRefresh();
								CalculateTotals();
							}
						}
					}NxCatchAll("Error Correcting a Payment");
				}
				break;
			// (j.dinatale 2011-06-03 16:18) - PLID 44809 - Void a payment
			case miVoidPayment:
				{
					try{
						// (j.dinatale 2011-06-27 10:20) - PLID 44812
						if(!CheckCurrentUserPermissions(bioFinancialCorrections, sptWrite)){
							return;
						}

						if (pRow) {

							long nPayID = -1;
							if (strLineType == CString("Applied") || strLineType == CString("InsuranceApplied") || strLineType == CString("AppliedPayToPay")){								
								_variant_t varApplyID = pRow->GetValue(btcApplyID);
								if(varApplyID.vt == VT_I4 && VarLong(varApplyID,-1) != -1) {
									_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT SourceID FROM AppliesT WHERE ID = {INT}", VarLong(varApplyID));
									if(!rs->eof) {
										nPayID = rs->Fields->Item["SourceID"]->Value.lVal;
									}
									rs->Close();
								}
								else {
									nPayID = VarLong(pRow->GetValue(btcPayID));
								}
							}
							else {
								nPayID = VarLong(pRow->GetValue(btcPayID));
							}

							// (j.jones 2011-09-13 15:37) - PLID 44887 - do not allow re-correcting
							// an already corrected payment
							LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nPayID);
							if(licsStatus == licsOriginal) {
								AfxMessageBox("This line item has already been corrected, and can no longer be modified.");
								return;
							}
							else if(licsStatus == licsVoid) {
								AfxMessageBox("This line item is a void line item from an existing correction, and cannot be modified.");
								return;
							}

							// (j.dinatale 2011-06-28 10:53) - PLID 44808 - need prompts to let the user know what is going on and when they should be using this
							//		etc, etc, etc.
							CString strMessage1, strMessage2;
							CString strLineItemTypeName;

							if (strLineType == CString("Applied") || strLineType == CString("InsuranceApplied") || strLineType == CString("AppliedPayToPay")){
								ASSERT(strPayType.CompareNoCase("Item") != 0);
								strLineItemTypeName = strPayType.MakeLower();

								// (j.dinatale 2011-08-03 16:50) - PLID 44808 - the reason why this is being changed to payment in the prompt is to avoid
								//		having to query yet again to get whether the payment is a prepayment or not, so to be consistent when prompting for an
								//		unapplied prepayment (since all the prompts call it a payment) we change it back.
								if(strLineItemTypeName == "prepayment"){
									strLineItemTypeName = "payment";
								}
							}else{
								strLineItemTypeName = strLineType.MakeLower();
							}

							// (j.dinatale 2011-12-09 09:51) - PLID 46898 - tweak our message box to be a little more generic, also in this case we can add
							//		a little bit of additional info if we know its not a refund
							CString strAdditionalInfo;
							if(strLineItemTypeName.CompareNoCase("refund") != 0){
								strAdditionalInfo.Format("If you only need to remove a Bill or Charge the %s is applied to, use the Correct option on the %s first, "
									"then Void Bill or Void Charge.", strLineItemTypeName, strLineItemTypeName);
							}

							strMessage1.Format("This feature should be used when the %s has been incorrectly created and it cannot be deleted.\r\n"
								"\r\n"
								"NOTE: A %s's provider and location are reported based on the line item it is applied to. DO NOT void an applied %s if these are the fields "
								"that should be changed. Consider correcting the line item it is applied to instead. %s", 
								strLineItemTypeName, strLineItemTypeName, strLineItemTypeName, strAdditionalInfo);

							if(IDCANCEL == AfxMessageBox(strMessage1, MB_OKCANCEL)){
									return;
							}

							// (j.dinatale 2011-09-13 17:30) - PLID 44808 - we are no longer creating adjustments, need to reflect as such in the message boxes
							strMessage2.Format(
									"The following action will occur:\r\n"
									"\r\n"
									"- A reverse %s will be created and applied to the original %s to offset its amount.", strLineItemTypeName, strLineItemTypeName);

							// (j.dinatale 2011-09-13 17:30) - PLID 44808 - need to warn if we are unlinking from a quote
							{
								_RecordsetPtr rsPaymentInfo = CreateParamRecordset(GetRemoteDataSnapshot(),
									"SELECT BillsT.Description AS QuoteName FROM PaymentsT "
									"LEFT JOIN BillsT ON PaymentsT.QuoteID = BillsT.ID "
									"WHERE PaymentsT.QuoteID IS NOT NULL AND PaymentsT.ID = {INT}", nPayID);

								if(!rsPaymentInfo->eof){
									CString strTempMessage;
									strTempMessage.Format("%s\r\n- The payment will be unlinked from the quote named \"%s\".", strMessage2, AdoFldString(rsPaymentInfo->Fields, "QuoteName", ""));
									strMessage2 = strTempMessage;
								}
							}

							if(IDCANCEL == AfxMessageBox(strMessage2, MB_OKCANCEL)){
									return;
							}

							// (b.spivey, September 11, 2014) - PLID 63652 - If the payment is linked, warn them that it'll permanently unlink
							if (IsLinkedToLockbox(nPayID)) {
								CString strTempMessage;
								strTempMessage.Format("This payment is connected to a lockbox deposit. If you void this payment the lockbox deposit line item will be marked "
									"as manually posted and unassociated with this payment. \r\n\r\n"
									"Are you sure you want to void this payment?");
								
								// (b.spivey, October 3, 2014) - PLID 63652 - Yes/No, not OK/Cancel
								if (IDNO == AfxMessageBox(strTempMessage, MB_YESNO)) {
									return; 
								}
							}
							


							// (j.jones 2011-10-19 08:53) - PLID 45462 - changed the warning because an Undo now makes this reversible
							if(IDYES != AfxMessageBox("This correction is only reversible using the Undo Correction feature.\n"
								"Are you sure you want to perform this correction?", MB_YESNO)){
								return;
							}

							CFinancialCorrection finCor;

							CString strUsername = GetCurrentUserName();
							long nCurrUserID = GetCurrentUserID();

							finCor.AddCorrection(ctPayment, nPayID, strUsername, nCurrUserID, FALSE);

							finCor.ExecuteCorrections();
							QuickRefresh();
							CalculateTotals();
						}
					}NxCatchAll("Error Voiding a Payment");
				}
				break;

			// (j.jones 2011-09-20 14:05) - PLID 45462 - undo a bill correction
			case miUndoBillCorrection: {

				if(nCorrectionID_ToUndo == -1) {
					//should be impossible to have chosen this option without
					//having cached the ID of the correction to undo
					ASSERT(FALSE);
					break;
				}

				long nBillID = VarLong(pRow->GetValue(m_nBillIDColID));

				//this function will check the bioFinancialCorrections sptDelete permission,
				//but will also check to see if the correction is closed, then check the sptDynamic0
				//permission and warn at all times if closed
				if(!CanUndoCorrection("bill", dtCorrectionInputDate)) {
					return;
				}

				if(nCorrectionNewID != -1) {

					//check and see if the new bill has itself been corrected, and if so, the undo is not permitted
					if(ReturnsRecordsParam("SELECT TOP 1 ID FROM BillCorrectionsT WHERE OriginalBillID = {INT}", nCorrectionNewID)) {
						AfxMessageBox("The new bill from this correction has already been corrected itself. "
							"You cannot undo this correction until the new bill's correction is removed first.");
						return;
					}

					//if the bill has charges that have been corrected, disallow the undo
					if(DoesBillHaveOriginalOrVoidCharges(nCorrectionNewID)) {
						AfxMessageBox("The new bill from this correction has charges that have also been corrected. "
							"You will need to undo the correction for these charges in order to undo this bill correction.");
						return;
					}

					//cannot undo if the new bill has something applied to it that is corrected
					if(DoesBillHaveOriginalOrVoidApplies(nCorrectionNewID)) {
						AfxMessageBox("The new bill from this correction has items applied to it that have also been corrected. "
							"You cannot undo this correction until the new bill's corrected applies are removed first.");
						return;
					}

					// (j.jones 2014-07-15 10:46) - PLID 62876 - if the corrected bill has any chargebacks applied to it,
					// the chargebacks have to be undone before the correction can be undone
					if (ReturnsRecordsParam("SELECT BillsT.ID FROM BillsT "
						"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"INNER JOIN ChargebacksT ON ChargesT.ID = ChargebacksT.ChargeID "
						"WHERE LineItemT.Deleted = 0 AND BillsT.ID = {INT}", nCorrectionNewID)) {

						AfxMessageBox("The new bill from this correction has a chargeback payment applied to it. "
							"You must select 'Undo Chargeback' on this payment before you can undo this correction.");
						return;
				}
				}
				else {
					//the bill was only voided, and not corrected

					// (j.jones 2013-06-26 13:15) - PLID 57297 - Bills that have charges for products that need a serial number or exp. dates
					// cannot be undone if there was no corrected bill. A corrected bill would have had the product items moved to the new bill,
					// and undoing would move them back. If they only voided, the products were released back for general use.
					// Therefore, you cannot undo a voided-only bill. We would have told them this when they voided it.
					if(ReturnsRecordsParam("SELECT TOP 1 1 FROM ChargesT INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID "
						"WHERE ChargesT.BillID = {INT} AND (ProductT.HasSerialNum = 1 OR ProductT.HasExpDate = 1) ", nBillID)) {

						//cannot undo this void because the bill has products that need serialized items
						AfxMessageBox("This bill has charges for products that require serial numbers or expiration dates. "
							"When the bill was voided, these products were released back into available inventory.\n\n"
							"This void cannot be undone. A new bill will need to be created, and new products will need to be selected.");
						return;
					}
				}

				//warn twice
				CString strMessage;
				
				if(nCorrectionNewID != -1) {
					strMessage = "Undoing this correction will delete the Voided charges from the Voided bill, and delete the Corrected bill. "
						"The Original bill will be editable once more, if it has not been closed.\n\n"
						"Are you sure you want to undo this correction?";
				}
				else {
					strMessage = "Undoing this correction will delete the Voided charges from the Voided bill. "
						"The Original bill will be editable once more, if it has not been closed.\n\n"
						"Are you sure you want to undo this correction?";
				}

				if(IDNO == MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_YESNO)){
					return;
				}

				if(IDNO == MessageBox("Undoing this correction is irreversible. You would need to correct the original bill again.\n\n"
					"Are you absolutely sure you want to undo this correction?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)){
					return;
				}

				CFinancialCorrection finCor;

				//Are there another corrections with an Original line item that is applied to
				//charges in our Original bill? If so, we need to undo those as well, because it is impossible
				//for them to exist unless they were auto-corrected by this correction.
				//(Example: If this bill was corrected (A) it also corrected each charge inside that bill (B),
				//and if those charges had payments applied to them, those payments would also have been
				//auto-corrected (C). By undoing bill correction A, we will shortly undo charge corrections B,
				//but before we can do that, we have to also undo payment corrections C.)
				//But we need to ignore any charges that were corrected *prior* to this bill correction.
				_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT LineItemCorrectionsT.ID, LineItemCorrectionsT.InputDate, OriginalLineItemID, VoidingLineItemID, NewLineItemID, "
					"LineItemT.Type "
					"FROM LineItemCorrectionsT "
					"INNER JOIN LineItemT ON LineItemCorrectionsT.OriginalLineItemID = LineItemT.ID "
					"WHERE OriginalLineItemID IN "
					"	(SELECT SourceID FROM AppliesT WHERE DestID IN "
					"		(SELECT ID FROM ChargesT WHERE BillID = {INT}) "
					"	) "
					"AND LineItemCorrectionsT.InputDate >= (SELECT InputDate FROM BillCorrectionsT WHERE ID = {INT})",
					nCorrectionOriginalID, nCorrectionID_ToUndo);
				while(!rs->eof) {
					long nOtherCorrectionID_ToUndo = VarLong(rs->Fields->Item["ID"]->Value);
					COleDateTime dtOtherCorrectionInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
					long nOtherCorrectionOriginalID = VarLong(rs->Fields->Item["OriginalLineItemID"]->Value);
					long nOtherCorrectionVoidingID = VarLong(rs->Fields->Item["VoidingLineItemID"]->Value);
					long nOtherCorrectionNewID = VarLong(rs->Fields->Item["NewLineItemID"]->Value, -1);	//could be null
					long nOtherCorrectionOriginalType = VarLong(rs->Fields->Item["Type"]->Value);

					CString strOtherType = "payment";
					if(nOtherCorrectionOriginalType == LineItem::Adjustment) {
						strOtherType = "adjustment";
					}
					else if(nOtherCorrectionOriginalType == LineItem::Refund) {
						strOtherType = "refund";
					}

					//we do not need to check permissions or the close status, but we do need to see if the
					//correction's new item is itself corrected, and if so, abort
					if(nOtherCorrectionNewID != -1) {
						LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nOtherCorrectionNewID);
						if(licsStatus == licsOriginal) {
							CString strWarning;
							strWarning.Format("When this bill was corrected, an applied %s was also corrected. "
								"The new %s from that correction has already been corrected itself. "
								"You cannot undo this bill correction until the applied %s's correction is removed first.", strOtherType, strOtherType, strOtherType);
							AfxMessageBox(strWarning);
							return;
						}

						//cannot undo if the applied item has something applied to it that is correctedy
						if(DoesLineItemHaveOriginalOrVoidApplies(nOtherCorrectionNewID)) {
							CString strWarning;
							strWarning.Format("When this bill was corrected, an applied %s was also corrected. "
								"The new %s from that correction has items applied to it that have also been corrected. "
								"You cannot undo this bill correction until the applied %s's corrected applies are removed first.", strOtherType, strOtherType, strOtherType);
							AfxMessageBox(strWarning);
							return;
						}
					}

					//is our original payment also applied to a charge that has since been corrected separately?
					CString strAppliedToType = "";
					if(IsPaymentAppliedToNewCorrection(nOtherCorrectionOriginalID, "BillCorrectionsT", nCorrectionID_ToUndo, strAppliedToType)) {
						CString strWarning;
						strWarning.Format("When this bill was corrected, an applied %s was also corrected. "
							"The %s from that correction is also partially applied to a %s that was corrected later. "
							"You cannot undo this bill correction until the %s that the %s is applied to has also had its corrections undone.",
							strOtherType, strOtherType, strAppliedToType, strAppliedToType, strOtherType);
						AfxMessageBox(strWarning);
						return;
					}

					// (j.jones 2011-10-28 14:53) - PLID 45462 - first recursively undo any payments applied to this one
					if(!UndoAppliedCorrections(finCor, nOtherCorrectionOriginalID, "BillCorrectionsT", nCorrectionID_ToUndo, "bill")) {
						return;
					}

					//undo this applied correction (C)
					finCor.AddCorrectionToUndo(cutUndoPayment, nOtherCorrectionID_ToUndo, dtOtherCorrectionInputDate, nOtherCorrectionOriginalID, nOtherCorrectionVoidingID, nOtherCorrectionNewID, m_iCurPatient);

					rs->MoveNext();
				}
				rs->Close();

				//Are there charges in our bill that have corrections? There should be, because correcting a bill
				//auto-corrects all charges in that bill. We need to undo those charge corrections (B) prior to undoing
				//our bill correction (A).
				//But we need to ignore any charges that were corrected *prior* to this bill correction.
				rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT LineItemCorrectionsT.ID, LineItemCorrectionsT.InputDate, OriginalLineItemID, VoidingLineItemID, NewLineItemID, "
					"LineItemT.Type "
					"FROM LineItemCorrectionsT "
					"INNER JOIN LineItemT ON LineItemCorrectionsT.OriginalLineItemID = LineItemT.ID "
					"WHERE OriginalLineItemID IN (SELECT ID FROM ChargesT WHERE BillID = {INT}) "
					"AND LineItemCorrectionsT.InputDate >= (SELECT InputDate FROM BillCorrectionsT WHERE ID = {INT})",
					nCorrectionOriginalID, nCorrectionID_ToUndo);
				while(!rs->eof) {
					long nOtherCorrectionID_ToUndo = VarLong(rs->Fields->Item["ID"]->Value);
					COleDateTime dtOtherCorrectionInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
					long nOtherCorrectionOriginalID = VarLong(rs->Fields->Item["OriginalLineItemID"]->Value);
					long nOtherCorrectionVoidingID = VarLong(rs->Fields->Item["VoidingLineItemID"]->Value);
					long nOtherCorrectionNewID = VarLong(rs->Fields->Item["NewLineItemID"]->Value, -1);	//could be null
					long nOtherCorrectionOriginalType = VarLong(rs->Fields->Item["Type"]->Value);

					//we do not need to check permissions or the close status, but we do need to see if the
					//correction's new item is itself corrected, and if so, abort
					if(nOtherCorrectionNewID != -1) {
						// (j.jones 2011-10-19 09:06) - you will not often get to this code, but I proved
						// that you CAN get here in certain circumstances (most cases are already caught by this point)
						LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nOtherCorrectionNewID);
						if(licsStatus == licsOriginal) {
							AfxMessageBox("When this bill was corrected, the charges in that bill were also corrected. "
								"A new charge from that correction has already been corrected itself. "
								"You cannot undo this bill correction until the new charge's correction is removed first.");
							return;
						}

						//cannot undo if the new charge has something applied to it that is corrected
						if(DoesLineItemHaveOriginalOrVoidApplies(nOtherCorrectionNewID)) {
							AfxMessageBox("When this bill was corrected, the charges in that bill were also corrected. "
								"A new charge from that correction has items applied to it that have also been corrected. "
								"You cannot undo this bill correction until the new charge's corrected applies are removed first.");
							return;
						}

						// (j.jones 2013-06-26 14:29) - PLID 57297 - Charges for products that have serial numbers or exp. dates 
						// cannot be undone if the charge quantities are not identical. The product items get shifted back to the old charge,
						// but that would be inaccurate if the new charge is qty. 3 and the old charge is qty 2.
						if(ReturnsRecordsParam("SELECT TOP 1 1 FROM LineItemCorrectionsT "
								"INNER JOIN ChargesT OriginalChargesT ON LineItemCorrectionsT.OriginalLineItemID = OriginalChargesT.ID "
								"INNER JOIN ChargesT NewChargesT ON LineItemCorrectionsT.NewLineItemID = NewChargesT.ID "
								"INNER JOIN (SELECT ChargeID FROM ChargedProductItemsT GROUP BY ChargeID) AS ChargedProductItemsQ ON NewChargesT.ID = ChargedProductItemsQ.ChargeID "
								"WHERE LineItemCorrectionsT.ID = {INT} "
								"AND OriginalChargesT.Quantity <> NewChargesT.Quantity", nOtherCorrectionID_ToUndo)) {

							//cannot undo this void because a quantity differs
							AfxMessageBox("This bill has charges for products associated with serial numbers or expiration dates. "
								"These new charges have different quantities from the original bill, and the selected products have changed. "
								"These products cannot be moved back to the original bill.\n\n"
								"This correction cannot be undone. The new bill will need to be edited or corrected in order to make changes.");
							return;
						}
					}

					//undo this charge correction (B)
					finCor.AddCorrectionToUndo(cutUndoCharge, nOtherCorrectionID_ToUndo, dtOtherCorrectionInputDate, nOtherCorrectionOriginalID, nOtherCorrectionVoidingID, nOtherCorrectionNewID, m_iCurPatient);

					rs->MoveNext();
				}
				rs->Close();

				//now we can add the bill correction (A) that the user had chosen to undo
				finCor.AddCorrectionToUndo(cutUndoBill, nCorrectionID_ToUndo, dtCorrectionInputDate, nCorrectionOriginalID, -1, nCorrectionNewID, m_iCurPatient);

				finCor.ExecuteCorrectionsToUndo(cutUndoAll);

				FullRefresh();
				CalculateTotals();

				}
				break;

			// (j.jones 2011-09-20 14:06) - PLID 45562 - undo a charge correction
			case miUndoChargeCorrection: {

				if(nCorrectionID_ToUndo == -1) {
					//should be impossible to have chosen this option without
					//having cached the ID of the correction to undo
					ASSERT(FALSE);
					break;
				}

				long nChargeID = VarLong(pRow->GetValue(btcChargeID)); 
				long nBillID = VarLong(pRow->GetValue(m_nBillIDColID));

				//this function will check the bioFinancialCorrections sptDelete permission,
				//but will also check to see if the correction is closed, then check the sptDynamic0
				//permission and warn at all times if closed
				if(!CanUndoCorrection("charge", dtCorrectionInputDate)) {
					return;
				}

				//If the original charge is from a bill that has been voided, then we cannot undo it,
				//because it is part of the bill correction. They would have to undo the bill correction instead.
				if(ReturnsRecordsParam("SELECT TOP 1 ChargesT.ID FROM ChargesT "
					"INNER JOIN BillCorrectionsT ON ChargesT.BillID = BillCorrectionsT.OriginalBillID "
					"WHERE ChargesT.ID = {INT}", nCorrectionOriginalID)) {

					AfxMessageBox("This charge correction is from a voided bill. "
						"You will need to undo the correction for the original bill that was voided in order to undo this correction.");
					return;
				}

				//check and see if the new charge has itself been corrected, and if so, the undo is not permitted
				if(nCorrectionNewID != -1) {

					//If the new charge is on a voided bill, disallow undoing until the new bill correction is undone.
					/*This is actually impossible without the original charge being on a voided bill, which we already checked.
					if(ReturnsRecordsParam("SELECT TOP 1 ChargesT.ID FROM ChargesT "
						"INNER JOIN BillCorrectionsT ON ChargesT.BillID = BillCorrectionsT.OriginalBillID "
						"WHERE ChargesT.ID = {INT}", nCorrectionNewID)) {

						AfxMessageBox("The new charge from this correction is on a bill that has also been corrected. "
							"You will need to undo the correction for the new bill in order to undo this charge correction.");
						return;
					}
					*/

					LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nCorrectionNewID);
					if(licsStatus == licsOriginal) {
						AfxMessageBox("The new charge from this correction has already been corrected itself. "
							"You cannot undo this correction until the new charge's correction is removed first.");
						return;
					}

					//cannot undo if the new charge has something applied to it that is corrected
					if(DoesLineItemHaveOriginalOrVoidApplies(nCorrectionNewID)) {
						AfxMessageBox("The new charge from this correction has items applied to it that have also been corrected. "
							"You cannot undo this correction until the new charge's corrected applies are removed first.");
						return;
					}

					// (j.jones 2013-06-26 14:29) - PLID 57297 - Charges for products that have serial numbers or exp. dates 
					// cannot be undone if the charge quantities are not identical. The product items get shifted back to the old charge,
					// but that would be inaccurate if the new charge is qty. 3 and the old charge is qty 2.
					if(ReturnsRecordsParam("SELECT TOP 1 1 FROM LineItemCorrectionsT "
							"INNER JOIN ChargesT OriginalChargesT ON LineItemCorrectionsT.OriginalLineItemID = OriginalChargesT.ID "
							"INNER JOIN ChargesT NewChargesT ON LineItemCorrectionsT.NewLineItemID = NewChargesT.ID "
							"INNER JOIN (SELECT ChargeID FROM ChargedProductItemsT GROUP BY ChargeID) AS ChargedProductItemsQ ON NewChargesT.ID = ChargedProductItemsQ.ChargeID "
							"WHERE LineItemCorrectionsT.ID = {INT} "
							"AND OriginalChargesT.Quantity <> NewChargesT.Quantity", nCorrectionID_ToUndo)) {

						//cannot undo this void because the quantity differs
						AfxMessageBox("This charge is for a product associated with serial numbers or expiration dates. "
							"The new charge has a different quantity from the original charge, and the selected products have changed. "
							"These products cannot be moved back to the original charge.\n\n"
							"This correction cannot be undone. The new charge will need to be edited or corrected in order to make changes.");
						return;
					}

					// (j.jones 2014-07-15 10:46) - PLID 62876 - if the corrected charge has any chargebacks applied to it,
					// the chargebacks have to be undone before the correction can be undone
					if (ReturnsRecordsParam("SELECT ChargesT.ID FROM ChargesT "
						"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
						"INNER JOIN ChargebacksT ON ChargesT.ID = ChargebacksT.ChargeID "
						"WHERE LineItemT.Deleted = 0 AND ChargesT.ID = {INT}", nCorrectionNewID)) {

						AfxMessageBox("The new charge from this correction has a chargeback payment applied to it. "
							"You must select 'Undo Chargeback' on this payment before you can undo this correction.");
						return;
				}
				}
				else {
					//the charge was only voided, and not corrected

					// (j.jones 2013-06-26 13:15) - PLID 57297 - Charges for products that need a serial number or exp. dates
					// cannot be undone if there was no corrected charge. A corrected charge would have had the product items moved to the new charge,
					// and undoing would move them back. If they only voided, the products were released back for general use.
					// Therefore, you cannot undo a voided-only charge. We would have told them this when they voided it.
					if(ReturnsRecordsParam("SELECT TOP 1 1 FROM ChargesT INNER JOIN ProductT ON ChargesT.ServiceID = ProductT.ID "
						"WHERE ChargesT.ID = {INT} AND (ProductT.HasSerialNum = 1 OR ProductT.HasExpDate = 1) ", nChargeID)) {

						//cannot undo this void because the charge has products that need serialized items
						AfxMessageBox("This charge is for a product that requires serial numbers or expiration dates. "
							"When the charge was voided, these products were released back into available inventory.\n\n"
							"This void cannot be undone. A new charge will need to be created, and new products will need to be selected.");
						return;
					}
				}

				//warn twice
				CString strMessage;
				
				if(nCorrectionNewID != -1) {
					strMessage = "Undoing this correction will delete the Void charge and the Corrected charge. "
						"The Original charge will be editable once more, if it has not been closed.\n\n"
						"Are you sure you want to undo this correction?";
				}
				else {
					strMessage = "Undoing this correction will delete the Void charge. "
						"The Original charge will be editable once more, if it has not been closed.\n\n"
						"Are you sure you want to undo this correction?";
				}

				if(IDNO == MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_YESNO)){
					return;
				}

				if(IDNO == MessageBox("Undoing this correction is irreversible. You would need to correct the original charge again.\n\n"
					"Are you absolutely sure you want to undo this correction?", "Practice", MB_ICONEXCLAMATION|MB_YESNO)){
					return;
				}

				CFinancialCorrection finCor;

				//Are there another corrections with an Original line item that is applied to
				//our Original line item? If so, we need to undo those as well, because it is impossible
				//for them to exist unless they were auto-corrected by this correction.
				//(Example: If this charge had been corrected (A) but it had payments applied to it, those payments
				//would have been auto-corrected (B). By undoing correction A, we have to also undo correction B.)
				//***This does not need to worry about LineItemCorrectionsBalancingAdjT.***
				//But we need to ignore any payments that were corrected *prior* to this bill correction.
				_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT LineItemCorrectionsT.ID, LineItemCorrectionsT.InputDate, OriginalLineItemID, VoidingLineItemID, NewLineItemID, "
					"LineItemT.Type "
					"FROM LineItemCorrectionsT "
					"INNER JOIN LineItemT ON LineItemCorrectionsT.OriginalLineItemID = LineItemT.ID "
					"WHERE OriginalLineItemID IN (SELECT SourceID FROM AppliesT WHERE DestID = {INT}) "
					"AND LineItemCorrectionsT.InputDate >= (SELECT InputDate FROM LineItemCorrectionsT WHERE ID = {INT})",
					nCorrectionOriginalID, nCorrectionID_ToUndo);
				while(!rs->eof) {
					long nOtherCorrectionID_ToUndo = VarLong(rs->Fields->Item["ID"]->Value);
					COleDateTime dtOtherCorrectionInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
					long nOtherCorrectionOriginalID = VarLong(rs->Fields->Item["OriginalLineItemID"]->Value);
					long nOtherCorrectionVoidingID = VarLong(rs->Fields->Item["VoidingLineItemID"]->Value);
					long nOtherCorrectionNewID = VarLong(rs->Fields->Item["NewLineItemID"]->Value, -1);	//could be null
					long nOtherCorrectionOriginalType = VarLong(rs->Fields->Item["Type"]->Value);

					CString strOtherType = "payment";
					if(nOtherCorrectionOriginalType == LineItem::Adjustment) {
						strOtherType = "adjustment";
					}
					else if(nOtherCorrectionOriginalType == LineItem::Refund) {
						strOtherType = "refund";
					}

					//we do not need to check permissions or the close status, but we do need to see if the
					//correction's new item is itself corrected, and if so, abort
					if(nOtherCorrectionNewID != -1) {
						LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nOtherCorrectionNewID);
						if(licsStatus == licsOriginal) {
							CString strWarning;
							strWarning.Format("When this charge was corrected, an applied %s was also corrected. "
								"The new %s from that correction has already been corrected itself. "
								"You cannot undo this charge correction until the applied %s's correction is removed first.", strOtherType, strOtherType, strOtherType);
							AfxMessageBox(strWarning);
							return;
						}

						//cannot undo if the applied item has something applied to it that is corrected
						if(DoesLineItemHaveOriginalOrVoidApplies(nOtherCorrectionNewID)) {
							CString strWarning;
							strWarning.Format("When this charge was corrected, an applied %s was also corrected. "
								"The new %s from that correction has items applied to it that have also been corrected. "
								"You cannot undo this charge correction until the applied %s's corrected applies are removed first.", strOtherType, strOtherType, strOtherType);
							AfxMessageBox(strWarning);
							return;
						}
					}

					//is our original payment also applied to a charge that has since been corrected separately?
					CString strAppliedToType = "";
					if(IsPaymentAppliedToNewCorrection(nOtherCorrectionOriginalID, "LineItemCorrectionsT", nCorrectionID_ToUndo, strAppliedToType)) {
						CString strWarning;
						strWarning.Format("When this charge was corrected, an applied %s was also corrected. "
							"The %s from that correction is also partially applied to a %s that was corrected later. "
							"You cannot undo this charge correction until the %s that the %s is applied to has also had its corrections undone.",
							strOtherType, strOtherType, strAppliedToType, strAppliedToType, strOtherType);
						AfxMessageBox(strWarning);
						return;
					}

					// (j.jones 2011-10-28 14:53) - PLID 45462 - first recursively undo any payments applied to this one
					if(!UndoAppliedCorrections(finCor, nOtherCorrectionOriginalID, "LineItemCorrectionsT", nCorrectionID_ToUndo, "charge")) {
						return;
					}

					//undo this applied correction (B)
					finCor.AddCorrectionToUndo(cutUndoPayment, nOtherCorrectionID_ToUndo, dtOtherCorrectionInputDate, nOtherCorrectionOriginalID, nOtherCorrectionVoidingID, nOtherCorrectionNewID, m_iCurPatient);

					rs->MoveNext();
				}
				rs->Close();

				//now we can add the initial correction (A) that the user had chosen to undo,
				//this will also handle balancing adjustments from LineItemCorrectionsBalancingAdjT
				finCor.AddCorrectionToUndo(cutUndoCharge, nCorrectionID_ToUndo, dtCorrectionInputDate, nCorrectionOriginalID, nCorrectionVoidingID, nCorrectionNewID, m_iCurPatient);

				finCor.ExecuteCorrectionsToUndo(cutUndoAll);

				FullRefresh();
				CalculateTotals();

				}
				break;

			// (j.jones 2011-09-20 14:06) - PLID 45563 - undo a pay/adj/ref correction
			case miUndoPaymentCorrection: {

				if(nCorrectionID_ToUndo == -1) {
					//should be impossible to have chosen this option without
					//having cached the ID of the correction to undo
					ASSERT(FALSE);
					break;
				}

				// (j.jones 2011-09-22 16:32) - Joe coded some madness here where strLineType is accurate
				// most of the time, but not always, and it's almost impossible to detect when one is right
				// and one is wrong, hence this rather ghetto solution.
				CString strLineItemTypeName = strPayType;
				if(strPayType.CompareNoCase("Item") == 0) {
					strLineItemTypeName = strLineType;
				}
				strLineItemTypeName.MakeLower();

				//this function will check the bioFinancialCorrections sptDelete permission,
				//but will also check to see if the correction is closed, then check the sptDynamic0
				//permission and warn at all times if closed
				if(!CanUndoCorrection(strLineItemTypeName, dtCorrectionInputDate)) {
					return;
				}

				//If this payment is applied to an Original item in a correction, we cannot undo it, 
				//because it means that the payment correction is part of another master correction.
				//Technically this could be applied to two applies, but we only need to warn about one.
				//(Example: If a payment had been corrected (A) but it had a refund applied to it, that refund
				//would have been auto-corrected (B). They cannot manually undo correction B, they have to
				//undo correction A, which then auto-undoes B.)
				_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 LineItemT.Type "
					"FROM AppliesT "
					"INNER JOIN LineItemT ON AppliesT.DestID = LineItemT.ID "
					"WHERE AppliesT.SourceID = {INT} "
					"AND AppliesT.DestID IN (SELECT OriginalLineItemID FROM LineItemCorrectionsT)", nCorrectionOriginalID);
				if(!rs->eof) {

					long nAppliedType = VarLong(rs->Fields->Item["Type"]->Value);
					CString strAppliedType = "charge";
					if(nAppliedType == LineItem::Payment) {
						strAppliedType = "payment";
					}
					else if(nAppliedType == LineItem::Adjustment) {
						strAppliedType = "adjustment";
					}
					else if(nAppliedType == LineItem::Refund) {
						strAppliedType = "refund";
					}

					CString strWarning;
					strWarning.Format("The original %s from this correction is applied to an original %s that was also corrected. "
						"You will need to undo the correction for that %s that the original %s is applied to in order to undo this correction.",
						strLineItemTypeName, strAppliedType, strAppliedType, strLineItemTypeName);
					AfxMessageBox(strWarning);
					return;
				}
				rs->Close();

				//check and see if the new payment has itself been corrected, and if so, the undo is not permitted
				if(nCorrectionNewID != -1) {
					LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nCorrectionNewID);
					if(licsStatus == licsOriginal) {
						CString strWarning;
						strWarning.Format("The new %s from this correction has already been corrected itself. "
							"You cannot undo this correction until the new %s's correction is removed first.", strLineItemTypeName, strLineItemTypeName);
						AfxMessageBox(strWarning);
						return;
					}

					//cannot undo if the new payment has something applied to it that is corrected
					if(DoesLineItemHaveOriginalOrVoidApplies(nCorrectionNewID)) {
						CString strWarning;
						strWarning.Format("The new %s from this correction has items applied to it that have also been corrected. "
							"You cannot undo this correction until the new %s's corrected applies are removed first.", strLineItemTypeName, strLineItemTypeName);
						AfxMessageBox(strWarning);
						return;
					}
				}

				// (j.jones 2015-06-16 09:36) - PLID 50008 - add a check to make sure that undoing the correction
				// will not result in invalid applies
				std::vector<long> aryAppliesToUndo;
				if (!UndoCorrection_CheckInvalidApplies(strLineItemTypeName, nCorrectionOriginalID, nCorrectionVoidingID, nCorrectionNewID, false, aryAppliesToUndo)) {
					//if this returned false, cancel the undo process
					return;
				}				

				//warn twice
				CString strMessage;
				
				if(nCorrectionNewID != -1) {
					strMessage.Format("Undoing this correction will delete the Void %s and the Corrected %s. "
						"The Original %s will be editable once more, if it has not been closed.\n\n"
						"Are you sure you want to undo this correction?", 
						strLineItemTypeName, strLineItemTypeName, strLineItemTypeName);
				}
				else {
					strMessage.Format("Undoing this correction will delete the Void %s. "
						"The Original %s will be editable once more, if it has not been closed.\n\n"
						"Are you sure you want to undo this correction?", 
						strLineItemTypeName, strLineItemTypeName);
				}

				if(IDNO == MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_YESNO)){
					return;
				}

				strMessage.Format("Undoing this correction is irreversible. You would need to correct the original %s again.\n\n"
					"Are you absolutely sure you want to undo this correction?", strLineItemTypeName);
				if(IDNO == MessageBox(strMessage, "Practice", MB_ICONEXCLAMATION|MB_YESNO)){
					return;
				}

				CFinancialCorrection finCor;

				//Are there another corrections with an Original line item that is applied to
				//our Original line item? If so, we need to undo those as well, because it is impossible
				//for them to exist unless they were auto-corrected by this correction.
				//(Example: If this payment had been corrected (A) but it had a refund applied to it, that refund
				//would have been auto-corrected (B). By undoing correction A, we have to also undo correction B.)
				//But we need to ignore any payments that were corrected *prior* to this bill correction.
				rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT LineItemCorrectionsT.ID, LineItemCorrectionsT.InputDate, OriginalLineItemID, VoidingLineItemID, NewLineItemID, "
					"LineItemT.Type "
					"FROM LineItemCorrectionsT "
					"INNER JOIN LineItemT ON LineItemCorrectionsT.OriginalLineItemID = LineItemT.ID "
					"WHERE OriginalLineItemID IN (SELECT SourceID FROM AppliesT WHERE DestID = {INT}) "
					"AND LineItemCorrectionsT.InputDate >= (SELECT InputDate FROM LineItemCorrectionsT WHERE ID = {INT})",
					nCorrectionOriginalID, nCorrectionID_ToUndo);
				while(!rs->eof) {
					long nOtherCorrectionID_ToUndo = VarLong(rs->Fields->Item["ID"]->Value);
					COleDateTime dtOtherCorrectionInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
					long nOtherCorrectionOriginalID = VarLong(rs->Fields->Item["OriginalLineItemID"]->Value);
					long nOtherCorrectionVoidingID = VarLong(rs->Fields->Item["VoidingLineItemID"]->Value);
					long nOtherCorrectionNewID = VarLong(rs->Fields->Item["NewLineItemID"]->Value, -1);	//could be null
					long nOtherCorrectionOriginalType = VarLong(rs->Fields->Item["Type"]->Value);

					CString strOtherType = "payment";
					if(nOtherCorrectionOriginalType == LineItem::Adjustment) {
						strOtherType = "adjustment";
					}
					else if(nOtherCorrectionOriginalType == LineItem::Refund) {
						strOtherType = "refund";
					}

					//we do not need to check permissions or the close status, but we do need to see if the
					//correction's new item is itself corrected, and if so, abort
					if(nOtherCorrectionNewID != -1) {
						LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nOtherCorrectionNewID);
						if(licsStatus == licsOriginal) {
							CString strWarning;
							strWarning.Format("When this %s was corrected, an applied %s was also corrected. "
								"The new %s from that correction has already been corrected itself. "
								"You cannot undo this correction until the applied %s's correction is removed first.", strLineItemTypeName, strOtherType, strOtherType, strOtherType);
							AfxMessageBox(strWarning);
							return;
						}

						//cannot undo if the applied item has something applied to it that is corrected
						if(DoesLineItemHaveOriginalOrVoidApplies(nOtherCorrectionNewID)) {
							CString strWarning;
							strWarning.Format("When this %s was corrected, an applied %s was also corrected. "
								"The new %s from that correction has items applied to it that have also been corrected. "
								"You cannot undo this correction until the applied %s's corrected applies are removed first.", strLineItemTypeName, strOtherType, strOtherType, strOtherType);
							AfxMessageBox(strWarning);
							return;
						}
					}

					//is our original payment also applied to a charge that has since been corrected separately?
					CString strAppliedToType = "";
					if(IsPaymentAppliedToNewCorrection(nOtherCorrectionOriginalID, "LineItemCorrectionsT", nCorrectionID_ToUndo, strAppliedToType)) {
						CString strWarning;
						strWarning.Format("When this %s was corrected, an applied %s was also corrected. "
							"The %s from that correction is also partially applied to a %s that was corrected later. "
							"You cannot undo this %s correction until everything else the %s is applied to has also had its corrections undone.",
							strLineItemTypeName, strOtherType, strOtherType, strAppliedToType, strLineItemTypeName, strAppliedToType, strOtherType);
						AfxMessageBox(strWarning);
						return;
					}

					// (j.jones 2011-10-28 14:53) - PLID 45462 - first recursively undo any payments applied to this one
					if(!UndoAppliedCorrections(finCor, nOtherCorrectionOriginalID, "LineItemCorrectionsT", nCorrectionID_ToUndo, strLineItemTypeName)){
						return;
					}

					//undo this applied correction (B)
					finCor.AddCorrectionToUndo(cutUndoPayment, nOtherCorrectionID_ToUndo, dtOtherCorrectionInputDate, nOtherCorrectionOriginalID, nOtherCorrectionVoidingID, nOtherCorrectionNewID, m_iCurPatient);

					rs->MoveNext();
				}
				rs->Close();

				//now we can add the initial correction (A) that the user had chosen to undo
				finCor.AddCorrectionToUndo(cutUndoPayment, nCorrectionID_ToUndo, dtCorrectionInputDate, nCorrectionOriginalID, nCorrectionVoidingID, nCorrectionNewID, m_iCurPatient);

				finCor.ExecuteCorrectionsToUndo(cutUndoAll);

				// (j.jones 2015-06-16 09:56) - PLID 50008 - if there were invalid applies, unapply them now
				for each(long nApplyID in aryAppliesToUndo) {
					//this will audit each unapply
					DeleteApply(nApplyID, FALSE);
				}

				FullRefresh();
				CalculateTotals();

				}
				break;

			// (r.gonet 07/10/2014) - PLID 62556 - Undo the chargeback
			case miUndoChargeback:
			{
				if (!pRow) {
					// Don't know how they got here without a row.
					break;
				} else {
					// We have a row
				}
				if (!pChargeback) {
					// We shouldn't even be displaying this option without a chargeback
					ASSERT(FALSE);
					break;
				} else {
					// And it is a chargeback
				}
				
				// Undo the chargeback, warning if necessary.
				if (DeleteChargeback(*(pChargeback.get()), false)) {
					// And update the billing tab.
					long nBillID = VarLong(pRow->GetValue(m_nBillIDColID));
					RedrawBill(nBillID);
					RefreshList();
				} else {
					// Deleting the chargeback was cancelled or failed. No need to refresh.
				}

				break;
			}

			case miRelApply:
				{ // Show all the related applies from a single payment
					try {
						if(pRow) {
							// (j.jones 2015-02-25 10:09) - PLID 64939 - this now uses apply manager
							long nPayAdjRefID = VarLong(pRow->GetValue(btcPayID), -1);
							long nApplyID = VarLong(pRow->GetValue(btcApplyID), -1);

							OpenApplyManager(-1, -1, nPayAdjRefID, nApplyID);
						}
					}NxCatchAll("Error showing related applies.");
				}
				break;

			case miDeletePay:	// Delete a standalone payment/adjustment/refund, and unapply all related applies
				{
					try {
						if(pRow) {
							COleVariant var = pRow->GetValue(btcPayID);	//var=PaymentID

							if(var.vt == VT_I4) {
								long nPayID = VarLong(var);

								// (j.jones 2011-09-13 15:37) - PLID 44887 - you can't delete this line item
								// if any charge in it is original or void
								LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nPayID);
								if(licsStatus == licsOriginal) {
									AfxMessageBox("This payment has been corrected, and can not be deleted.");
									return;
								}
								else if(licsStatus == licsVoid) {
									AfxMessageBox("This payment is a void line item from an existing correction, and can not be deleted.");
									return;
								}
								else if(DoesLineItemHaveOriginalOrVoidApplies(nPayID)) {
									AfxMessageBox("At least one apply to this payment has been corrected, and can not be deleted.");
									return;
								}
								//TES 7/25/2014 - PLID 63049 - Don't allow them to delete chargeback charges
								if (DoesPayHaveChargeback(CSqlFragment("PaymentsT.ID = {INT}", nPayID))) {
									MessageBox("This payment is associated with at least one Chargeback, and cannot be deleted. "
										"In order to delete this payment, you must right-click on it, and select 'Undo Chargeback.'");
									return;
							}
							}

							// (j.jones 2007-04-19 11:22) - PLID 25721 - In most cases CanEdit is not needed, so first
							// silently see if they have permission, and if they do, then move ahead normally. But if
							// they don't, or they need a password, then check CanEdit prior to the permission check that
							// would stop or password-prompt the user.
							// (c.haag 2009-03-10 10:45) - PLID 32433 - Use CanChangeHistoricFinancial
							if (var.vt != VT_EMPTY && CanChangeHistoricFinancial("Payment", VarLong(var,-1), bioPayment, sptDelete)) {
								//if((GetCurrentUserPermissions(bioPayment) & sptDelete) || CanEdit("Payment", VarLong(var,-1)) || CheckCurrentUserPermissions(bioPayment,sptDelete)) {
								CString strPrompt;

								// (j.jones 2009-12-29 11:01) - PLID 27237 - replaced with a combined, parameterized recordset
								_RecordsetPtr rsCheck = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Deposited, "
									"Convert(bit, CASE WHEN ID IN (SELECT PayID FROM ProcInfoPaymentsT WHERE PayID Is Not Null) THEN 1 ELSE 0 END) AS IsInProcInfo "
									"FROM PaymentsT WHERE ID = {INT}", VarLong(var));
								if(!rsCheck->eof) {
									if(AdoFldBool(rsCheck, "IsInProcInfo", FALSE)) {
										strPrompt += "This payment is attached to a tracked procedure.\n\n";
									}
									if(AdoFldBool(rsCheck, "Deposited", FALSE)) {
										strPrompt += "This payment has already been deposited. Deleting this payment could make your "
											"past deposits and accounting reports look unusual.\n\n";
									}
								}
								rsCheck->Close();

								strPrompt += "Are you sure you want to delete this payment? This action is unrecoverable!";
								if(IDYES==MessageBox(strPrompt,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {				
									DeletePayment(VarLong(var));
									// Redraw everything
									QuickRefresh();
								}
							}
						}
					}NxCatchAll("Error deleting payment.");
				}
				break;

			case miDeleteAdj:
				{
					try {
						if(pRow) {
							COleVariant var = pRow->GetValue(btcPayID);	//var=PaymentID

							if(var.vt == VT_I4) {
								long nPayID = VarLong(var);

								// (j.jones 2011-09-13 15:37) - PLID 44887 - you can't delete this line item
								// if any charge in it is original or void
								LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nPayID);
								if(licsStatus == licsOriginal) {
									AfxMessageBox("This adjustment has been corrected, and can not be deleted.");
									return;
								}
								else if(licsStatus == licsVoid) {
									AfxMessageBox("This adjustment is a void line item from an existing correction, and can not be deleted.");
									return;
								}
								else if(DoesLineItemHaveOriginalOrVoidApplies(nPayID)) {
									AfxMessageBox("At least one apply to this adjustment has been corrected, and can not be deleted.");
									return;
								}
							}

							// (j.jones 2007-04-19 11:24) - PLID 25721 - In most cases CanEdit is not needed, so first
							// silently see if they have permission, and if they do, then move ahead normally. But if
							// they don't, or they need a password, then check CanEdit prior to the permission check that
							// would stop or password-prompt the user.
							// (c.haag 2009-03-10 10:45) - PLID 32433 - Use CanChangeHistoricFinancial
							if (var.vt != VT_EMPTY && CanChangeHistoricFinancial("Adjustment", VarLong(var,-1), bioAdjustment, sptDelete)) {
								//if((GetCurrentUserPermissions(bioAdjustment) & sptDelete) || CanEdit("Adjustment", VarLong(var,-1)) || CheckCurrentUserPermissions(bioAdjustment,sptDelete)) {
								if(IDYES==MessageBox("Are you sure you want to delete this adjustment? This action is unrecoverable!","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
									DeletePayment(VarLong(var));
									// Redraw everything
									QuickRefresh();
								}
							}
						}
					}NxCatchAll("Error deleting adjustment.");
				}
				break;

			case miDeleteRef:
				{
					try {
						if(pRow) {
							COleVariant var = pRow->GetValue(btcPayID);	//var=PaymentID

							if(var.vt == VT_I4) {
								long nPayID = VarLong(var);

								// (j.jones 2011-09-13 15:37) - PLID 44887 - you can't delete this line item
								// if any charge in it is original or void
								LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nPayID);
								if(licsStatus == licsOriginal) {
									AfxMessageBox("This refund has been corrected, and can not be deleted.");
									return;
								}
								else if(licsStatus == licsVoid) {
									AfxMessageBox("This refund is a void line item from an existing correction, and can not be deleted.");
									return;
								}
								else if(DoesLineItemHaveOriginalOrVoidApplies(nPayID)) {
									AfxMessageBox("At least one apply to this adjustment has been corrected, and can not be deleted.");
									return;
								}
							}

							// (j.jones 2007-04-19 11:25) - PLID 25721 - In most cases CanEdit is not needed, so first
							// silently see if they have permission, and if they do, then move ahead normally. But if
							// they don't, or they need a password, then check CanEdit prior to the permission check that
							// would stop or password-prompt the user.
							// (c.haag 2009-03-10 10:45) - PLID 32433 - Use CanChangeHistoricFinancial
							if (var.vt != VT_EMPTY && CanChangeHistoricFinancial("Refund", VarLong(var,-1), bioRefund, sptDelete)) {
								//if((GetCurrentUserPermissions(bioRefund) & sptDelete) || CanEdit("Refund", VarLong(var,-1)) || CheckCurrentUserPermissions(bioRefund,sptDelete)) {
								if(IDYES==MessageBox("Are you sure you want to delete this refund? This action is unrecoverable!","Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
									DeletePayment(VarLong(var));
									// Redraw everything
									QuickRefresh();
								}
							}
						}
					}NxCatchAll("Error deleting refund.");
				}
				break;

			case miUnapply: // Unapply single payment
				{
					try {

						if(pRow) {

							CWaitCursor pWait;

							long nApplyID = VarLong(pRow->GetValue(btcApplyID),-1);

							// (j.jones 2011-09-13 15:37) - PLID 44887 - If the source line item
							// is part of a correction (ie. is it original or void), you cannot
							// unapply it from where it is currently applied. 
							if(IsOriginalOrVoidApply(nApplyID)) {
								AfxMessageBox("This apply is part of a line item correction and cannot be unapplied.");
								return;
							}
							// (r.gonet 07/10/2014) - PLID 62557 - Chargebacks cannot be unapplied
							if (pChargeback != NULL) {
								AfxMessageBox("This apply is part of a chargeback and cannot be unapplied.");
								return;
							}

							// (j.jones 2007-04-19 11:25) - PLID 25721 - In most cases CanEdit is not needed, so first
							// silently see if they have permission, and if they do, then move ahead normally. But if
							// they don't, or they need a password, then check CanEdit prior to the permission check that
							// would stop or password-prompt the user.
							// (j.gruber 2013-05-15 15:10) - PLID 47098 - we don't need this here anymore since its checked below
							// with CanChangeHistoricFinancial.  That function checks both these functions
							//if((GetCurrentUserPermissions(bioApplies) & sptDelete) || CanEdit("Payment", VarLong(pRow->GetValue(btcPayID),-1))) {
							//we can just take this out without moving the other up because miUnapply only shows up when the lineType is the ones it checks for below
							//keep this in its own scope though
							{

								CWaitCursor pWait;

								CString str;
								long nPayID = VarLong(pRow->GetValue(btcPayID));				

								DisableFinancialScreen();

								_variant_t var = pRow->GetValue(btcLineType);
								CString strLineType = VarString(var,"");
								if(strLineType == "Applied" || strLineType == "AppliedPayToPay" || strLineType == "InsuranceApplied") {
									
									//CanUnapplyLineItem will check permissions, check against a financial close,
									//and potentially offer to correct a closed payment first.
									//If so, nApplyID will be changed.
									if (!CanUnapplyLineItem(nApplyID)) {
										EnableFinancialScreen();
										break;
									}

									// (j.jones 2007-02-26 15:24) - PLID 24927 - warn before unapplying if linked to the ReturnedProductsT table
									// (j.jones 2008-06-03 14:28) - PLID 29928 - only look at ReturnedProductsT records that are not for deleted charges
									// (j.jones 2009-12-28 17:21) - PLID 27237 - ensured we only ever run one recordset here
									_RecordsetPtr rsReturns;
									if(strLineType == "AppliedPayToPay") {
										//need the source ID								
										rsReturns = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM ReturnedProductsT "
											"WHERE ChargeID NOT IN (SELECT ID FROM LineItemT WHERE Deleted = 1) "
											"AND (FinRefundID IN (SELECT SourceID FROM AppliesT WHERE ID = {INT}) OR FinAdjID IN (SELECT SourceID FROM AppliesT WHERE ID = {INT}))", nApplyID, nApplyID);
									}
									else {
										rsReturns = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM ReturnedProductsT "
											"WHERE ChargeID NOT IN (SELECT ID FROM LineItemT WHERE Deleted = 1) "
											"AND (FinRefundID = {INT} OR FinAdjID = {INT})", nPayID, nPayID);
									}

									if(!rsReturns->eof) {
										if(IDNO == MessageBox("This item is linked to a Returned Product record. It is recommended that you do not unapply this item.\n"
											"Are you sure you wish to continue with the unapply?","Practice",MB_YESNO|MB_ICONEXCLAMATION)) {
												CheckDlgButton(IDC_RADIO_PAYMENT,FALSE);
												CheckDlgButton(IDC_RADIO_REFUND,TRUE);
												EnableFinancialScreen();
												break;
										}
									}
									rsReturns->Close();

									DeleteApply(nApplyID);
								}
								else {
									EnableFinancialScreen();
									break;
								}

								_variant_t varBill = pRow->GetValue(m_nBillIDColID);
								_variant_t varCharge = pRow->GetValue(btcChargeID);
								if (varBill.vt != VT_NULL && varBill.vt != VT_EMPTY) {

									//clear expand sub branches if nothing else is applied
									if (varCharge.vt != VT_NULL && varCharge.vt != VT_EMPTY) {
										long nChargeID = VarLong(varCharge);
										// (j.jones 2009-12-29 11:01) - PLID 27237 - replaced IsRecordsetEmpty with a parameterized recordset
										_RecordsetPtr rsApply = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 DestID FROM AppliesT WHERE DestID = {INT}", nChargeID);
										if(rsApply->eof) {
											for (int i=0; i < m_aiExpandedSubBranches.GetSize(); i++) {
												if (m_aiExpandedSubBranches[i] == nChargeID) {
													m_aiExpandedSubBranches.RemoveAt(i);

													// (j.jones 2015-03-17 13:25) - PLID 64931 - clear the search results window
													ClearSearchResults();
													break;
												}
											}
										}
										rsApply->Close();
									}

									RedrawBill(VarLong(varBill));
								}
								else
								{
									// Remove the payment from the expanded list
									for (int i=0; i < m_aiExpandedPayBranches.GetSize(); i++) {
										if (m_aiExpandedPayBranches[i] == nPayID) {
											m_aiExpandedPayBranches.RemoveAt(i);

											// (j.jones 2015-03-17 13:25) - PLID 64931 - clear the search results window
											ClearSearchResults();
											break;
										}
									}
								}

								RedrawAllPayments();

								EnableFinancialScreen();

								CalculateTotals();
								//ResetBrightenedRow();
							} 
						}
					}NxCatchAll("Error unapplying item.");
				}
				break;

			case miApplyNewPayToBill: // Create a new payment and auto-apply it to a bill
				{
					try {

						if(pRow) {
							COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;
							COleVariant var;
							
							CPaymentDlg dlg(this);
							dlg.m_PatientID = m_iCurPatient;

							//JMJ - 7/24/2003 - set the payment to equal the balance of the highest responsibility

							COleCurrency cyMaxBalance;
							long nInsuredPartyID = -1;

							var = pRow->GetValue(m_nBillIDColID);

							long nBillID = VarLong(var);

							// (j.jones 2011-09-13 15:37) - PLID 44887 - do not allow applying to a fully corrected bill
							if(IsVoidedBill(nBillID)) {
								AfxMessageBox("This bill has been corrected, and can not be modified.");
								return;
							}
							else if(!DoesBillHaveUncorrectedCharges(nBillID)) {
								AfxMessageBox("All charges in this bill have been corrected, and can not be modified.");
								return;
							}

							if(!CheckCurrentUserPermissions(bioPayment,sptCreate)) break;

							if (!GetBillTotals(nBillID, m_iCurPatient, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
								MsgBox("This bill has been removed from this screen. Practice will now requery this patient's financial information.");
								FullRefresh();
								break;
							}

							//set the MaxBalance to the patient balance
							cyMaxBalance = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;

							//now see if there are any responsibilities with a higher balance
							// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
							_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PersonID, RespTypeID FROM InsuredPartyT WHERE PatientID = {INT}", m_iCurPatient);
							while(!rs->eof) {
								long nRespID = AdoFldLong(rs, "RespTypeID");
								long nTempInsuredPartyID = AdoFldLong(rs, "PersonID");
								if(nRespID != -1)
									// (j.jones 2011-07-22 12:19) - PLID 42231 - GetBillInsuranceTotals now takes in an insured party ID, not a RespTypeID
									GetBillInsuranceTotals(VarLong(var), m_iCurPatient, nTempInsuredPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds);
								else {
									cyAdjustments = COleCurrency(0,0);
									cyRefunds = COleCurrency(0,0);
									GetInactiveInsTotals(nTempInsuredPartyID, VarLong(var), -1, m_iCurPatient, cyCharges, cyPayments);
								}
								COleCurrency cyRespBalance = cyCharges - cyPayments - cyAdjustments - cyRefunds;
								if(cyRespBalance > cyMaxBalance) {
									cyMaxBalance = cyRespBalance;
									nInsuredPartyID = nTempInsuredPartyID;
								}
								rs->MoveNext();
							}
							rs->Close();

							dlg.m_cyFinalAmount = cyMaxBalance;
							dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;
							if(nInsuredPartyID != -1)
								dlg.m_iDefaultInsuranceCo = nInsuredPartyID;
							dlg.m_varBillID = var;
							dlg.m_ApplyOnOK = TRUE;

							if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
								// CH - 1/8
								//AutoApplyPayToBill(dlg.m_varPaymentID.lVal,	"Bill", VarLong(var));

								//TES 10/6/2010 - PLID 40566 - This may have a 0 balance now, so see if we should unbatch it.
								CheckUnbatchClaim(VarLong(var));
								// (r.gonet 2015-07-06 18:02) - PLID 65327 - If a bill is created, we need to do more than
								// just redraw the payments.
								if (dlg.m_bBillCreated) {
									QuickRefresh();
								} else {
									RedrawBill(pRow->GetValue(m_nBillIDColID).lVal);
									RedrawAllPayments();
									CalculateTotals();
								}
							}
							RefreshList();
							///////////////////////////
						}
					}NxCatchAll("Error applying new payment to bill.");
				}
				break;

			case miApplyExistingPayToBill: // Apply an existing payment to a bill
				{
					try {
						// (j.jones 2007-04-19 11:26) - PLID 25721 - In most cases CanEdit is not needed, so first
						// silently see if they have permission, and if they do, then move ahead normally. But if
						// they don't, or they need a password, then check CanEdit prior to the permission check that
						// would stop or password-prompt the user.
						// (a.walling 2010-05-03 14:26) - PLID 38461 - This was calling CanEdit("Payment") with the btcPayID, which is VT_EMPTY, since this is 
						// a bill line and not a payment line! This seems to be inconsistent anyway; get rid of it. I checked with Josh and he agrees.

						// (j.jones 2011-09-13 15:37) - PLID 44887 - do not allow applying to a fully corrected bill
						long nBillID = VarLong(pRow->GetValue(m_nBillIDColID));
						if(IsVoidedBill(nBillID)) {
							AfxMessageBox("This bill has been corrected, and can not be modified.");
							return;
						}
						else if(!DoesBillHaveUncorrectedCharges(nBillID)) {
							AfxMessageBox("All charges in this bill have been corrected, and can not be modified.");
							return;
						}

						// (j.jones 2011-08-24 17:48) - PLID 45176 - changed to a silent apply permission check,
						// later we will call CanApplyLineItem() when a source item is chosen
						if((GetCurrentUserPermissions(bioApplies) & (sptCreate|sptCreateWithPass))) {
							ShowBillApplies(pRow, APPLY_LIST_COMBO_SEL_AVAILABLE);
						}
						else {
							//tell them they do not have permission
							PermissionsFailedMessageBox();
						}

					}NxCatchAll("Error applying payment to bill.");
				}
				break;

			case miApplyExistingPayToCharge: // Apply an existing payment to a charge
				{
					try {

						long nChargeID = VarLong(pRow->GetValue(btcChargeID));

						// (j.jones 2011-09-13 15:37) - PLID 44887 - you can't delete this charge
						// if any charge in it is original or void
						LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nChargeID);
						if(licsStatus == licsOriginal) {
							AfxMessageBox("This charge has been corrected, and can not be modified.");
							return;
						}
						else if(licsStatus == licsVoid) {
							AfxMessageBox("This charge is a void charge from an existing correction, and can not be modified.");
							return;
						}

						// (j.jones 2007-04-19 11:26) - PLID 25721 - In most cases CanEdit is not needed, so first
						// silently see if they have permission, and if they do, then move ahead normally. But if
						// they don't, or they need a password, then check CanEdit prior to the permission check that
						// would stop or password-prompt the user.
						// (a.walling 2010-05-03 14:26) - PLID 38461 - This was calling CanEdit("Payment") with the btcPayID, which is VT_EMPTY, since this is 
						// a charge line and not a payment line! This seems to be inconsistent anyway; get rid of it. I checked with Josh and he agrees.
						// (j.jones 2011-08-24 17:48) - PLID 45176 - changed to a silent apply permission check,
						// later we will call CanApplyLineItem() when a source item is chosen
						if((GetCurrentUserPermissions(bioApplies) & (sptCreate|sptCreateWithPass))) {
							ShowChargeApplies(pRow, APPLY_LIST_COMBO_SEL_AVAILABLE);
						}
						else {
							//tell them they do not have permission
							PermissionsFailedMessageBox();
						}

					}NxCatchAll("Error applying payment to charge.");
				}
				break;

			case miApplyNewPayToCharge: // Apply a new payment to a charge
				{
					try {
						if(pRow) {

							long nChargeID = VarLong(pRow->GetValue(btcChargeID));

							// (j.jones 2011-09-13 15:37) - PLID 44887 - you can't delete this charge
							// if any charge in it is original or void
							LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nChargeID);
							if(licsStatus == licsOriginal) {
								AfxMessageBox("This charge has been corrected, and can not be modified.");
								return;
							}
							else if(licsStatus == licsVoid) {
								AfxMessageBox("This charge is a void charge from an existing correction, and can not be modified.");
								return;
							}

							if(!CheckCurrentUserPermissions(bioPayment,sptCreate))
								break;
							COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;
							CPaymentDlg dlg(this);
							dlg.m_PatientID = m_iCurPatient;

							//JMJ - 7/24/2003 - set the payment to equal the balance of the highest responsibility

							COleCurrency cyMaxBalance;
							long nInsuredPartyID = -1;

							if (!GetChargeTotals(nChargeID, m_iCurPatient, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
								MsgBox("This charge has been removed from this screen. Practice will now requery this patient's financial information.");
								FullRefresh();
								break;
							}

							//set the MaxBalance to the patient balance
							cyMaxBalance = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;

							//now see if there are any responsibilities with a higher balance
							// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
							_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PersonID, RespTypeID FROM InsuredPartyT WHERE PatientID = {INT}", m_iCurPatient);
							while(!rs->eof) {
								long nRespID = AdoFldLong(rs, "RespTypeID");
								long nTempInsuredPartyID = AdoFldLong(rs, "PersonID");
								if(nRespID != -1) {
									if (!GetChargeInsuranceTotals(nChargeID, m_iCurPatient, nTempInsuredPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds)) {
										MsgBox("This charge has been removed from this screen. Practice will now requery this patient's financial information.");
										FullRefresh();
										break;
									}
								}
								else {
									cyAdjustments = COleCurrency(0,0);
									cyRefunds = COleCurrency(0,0);
									GetInactiveInsTotals(nTempInsuredPartyID, -1, nChargeID, m_iCurPatient, cyCharges, cyPayments);
								}
								COleCurrency cyRespBalance = cyCharges - cyPayments - cyAdjustments - cyRefunds;
								if(cyRespBalance > cyMaxBalance) {
									cyMaxBalance = cyRespBalance;
									nInsuredPartyID = nTempInsuredPartyID;
								}
								rs->MoveNext();
							}
							rs->Close();

							dlg.m_cyFinalAmount = cyMaxBalance;
							dlg.m_cyMaxAmount = dlg.m_cyFinalAmount;
							if(nInsuredPartyID != -1)
								dlg.m_iDefaultInsuranceCo = nInsuredPartyID;
							dlg.m_varChargeID = (long)nChargeID;
							dlg.m_ApplyOnOK = TRUE;

							if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
								//TES 10/6/2010 - PLID 40566 - This may have a 0 balance now, so see if we should unbatch it.
								CheckUnbatchClaim(VarLong(pRow->GetValue(m_nBillIDColID)));
								// (r.gonet 2015-07-06 18:02) - PLID 65327 - If a bill is created, we need to do more than
								// just redraw the payments.
								if (dlg.m_bBillCreated) {
									QuickRefresh();
								} else {
									RedrawBill(pRow->GetValue(m_nBillIDColID).lVal);
									RedrawAllPayments();
									CalculateTotals();
								}
							}
						}
					}NxCatchAll("Error applying new payment to charge.");
				}
				break;

			case miApplyMgr: // Bring up the apply manager
				{
					try {

						if(pRow) {

							// (j.jones 2011-03-25 15:56) - PLID 41143 - if this is a payment/adjustment/refund,
							// filter on that value
							long nBillID = -1;
							long nChargeID = -1;
							long nPayAdjRefID = -1;
							long nApplyID = -1;
							CString strLineType = VarString(pRow->GetValue(btcLineType), "");
							if (strLineType == "Payment" || strLineType.Find("Applied") != -1) {
								_variant_t var = pRow->GetValue(btcPayID);
								if (var.vt == VT_I4) {
									nPayAdjRefID = VarLong(var);
									nApplyID = VarLong(pRow->GetValue(btcApplyID), -1);
								}
							}
							else {
								COleVariant var = pRow->GetValue(m_nBillIDColID);
								if (var.vt == VT_I4) {
									nBillID = VarLong(var);
									var = pRow->GetValue(btcChargeID);
									if (var.vt == VT_I4) {
										nChargeID = VarLong(var);
									}
								}
							}

							// (j.jones 2015-02-25 09:45) - PLID 64939 - made one modular function to open Apply Manager
							OpenApplyManager(nBillID, nChargeID, nPayAdjRefID, nApplyID);
						}
						// (b.eyers 7/22/2014) - PLID 56685 - Apply Manager option already shows in the right click menu 
						// and wasn't working when right clicking in blank space, open up apply manager when that option is selected
						else {
							OnApplyManager();
						}
					}NxCatchAll("Error loading apply manager.");
				}
				break;

			case miEditPay: // Edit a payment
				{
					try {
						// (j.jones 2007-04-19 11:27) - PLID 25721 - In most cases CanEdit is not needed, so first
						// silently see if they have permission, and if they do, then move ahead normally. But if
						// they don't, or they need a password, then check CanEdit prior to the permission check that
						// would stop or password-prompt the user.
						// (j.jones 2007-04-19 12:22) - PLID 25271 - turns out this is redundant,
						// as Pay_Adj_Ref_Click calls this came logic
						//if((GetCurrentUserPermissions(bioPayment) & sptWrite) || CanEdit("Payment", VarLong(pRow->GetValue(btcPayID),-1)) || CheckCurrentUserPermissions(bioPayment,sptWrite))
						Pay_Adj_Ref_Click(pRow, m_nPaymentColID);
					}NxCatchAll("Error editing payment.");
				}
				break;

			case miEditAdj: // Edit an adjustment
				{
					try {
						// (j.jones 2007-04-19 11:27) - PLID 25721 - In most cases CanEdit is not needed, so first
						// silently see if they have permission, and if they do, then move ahead normally. But if
						// they don't, or they need a password, then check CanEdit prior to the permission check that
						// would stop or password-prompt the user.
						// (j.jones 2007-04-19 12:22) - PLID 25271 - turns out this is redundant,
						// as Pay_Adj_Ref_Click calls this came logic
						//if((GetCurrentUserPermissions(bioAdjustment) & sptWrite) || CanEdit("Adjustment", VarLong(pRow->GetValue(btcPayID),-1)) || CheckCurrentUserPermissions(bioAdjustment,sptWrite))
						Pay_Adj_Ref_Click(pRow, m_nAdjustmentColID);
					}NxCatchAll("Error editing adjustment.");
				}
				break;

			case miEditRef: // Edit a refund
				{
					try {
						// (j.jones 2007-04-19 11:28) - PLID 25721 - In most cases CanEdit is not needed, so first
						// silently see if they have permission, and if they do, then move ahead normally. But if
						// they don't, or they need a password, then check CanEdit prior to the permission check that
						// would stop or password-prompt the user.
						// (j.jones 2007-04-19 12:22) - PLID 25271 - turns out this is redundant,
						// as Pay_Adj_Ref_Click calls this came logic
						//if((GetCurrentUserPermissions(bioRefund) & sptWrite) || CanEdit("Refund", VarLong(pRow->GetValue(btcPayID),-1)) || CheckCurrentUserPermissions(bioRefund,sptWrite))
						Pay_Adj_Ref_Click(pRow, m_nRefundColID);
					}NxCatchAll("Error editing refund.");
				}
				break;

			case miViewPay: //View applied payment
				{
					try {
						if(pRow) {
							CPaymentDlg dlg(this);
							dlg.m_PatientID = m_iCurPatient;
							_variant_t var = pRow->GetValue(btcApplyID);
							if(var.vt == VT_I4) {
								//for applies to payments
								// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
								_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT SourceID FROM AppliesT WHERE ID = {INT}", VarLong(var));
								if(!rs->eof) {
									dlg.m_varPaymentID = rs->Fields->Item["SourceID"]->Value.lVal;
								}
								else dlg.m_varPaymentID = VarLong(var);
								rs->Close();
							}
							else {
								var = pRow->GetValue(btcPayID);
								dlg.m_varPaymentID = VarLong(var);
							}

							//figure out what kind of item it is					
							if(dlg.m_varPaymentID.vt == VT_I4 && VarLong(dlg.m_varPaymentID) != -1) {
								// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
								_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT Type FROM LineItemT WHERE ID = {INT}", VarLong(dlg.m_varPaymentID));
								if(!rs->eof) {
									long nType = AdoFldLong(rs, "Type",-1);
									if(nType == 1 && !CheckCurrentUserPermissions(bioPayment,sptRead))
										break;
									else if(nType == 3 && !CheckCurrentUserPermissions(bioRefund,sptRead))
										break;
									else if((nType == 2 || nType == 0) && !CheckCurrentUserPermissions(bioAdjustment,sptRead))
										break;
								}
								rs->Close();
							}

							if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
								// a.walling 16493 Prevent tabs from collapsing
								QuickRefresh();
							}
						}
					}NxCatchAll("Error viewing applied item.");
				}
				break;

			case miPrintReceipt:
				{
					try {
						if(pRow) {
							CPtrArray paramList;
							CString filter, param;

							CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(235)];
							_variant_t var = pRow->GetValue(btcPayID);
							if(var.vt == VT_I4){
								infReport.nExtraID = VarLong(var);

								//setup the params
								//crystal report
								CPtrArray params;
								CRParameterInfo *tmpParam;

								tmpParam = new CRParameterInfo;
								tmpParam->m_Name = "ReceiptShowChargeInfo";
								tmpParam->m_Data = GetRemotePropertyInt("ReceiptShowChargeInfo", 0, 0, "<None>", true) ? "true" : "false";
								params.Add((void *)tmpParam);

								tmpParam = new CRParameterInfo;
								tmpParam->m_Name = "ReceiptShowTax";
								tmpParam->m_Data = GetRemotePropertyInt("ReceiptShowTax", 1, 0, "<None>", true) ? "true" : "false";
								params.Add((void *)tmpParam);

								tmpParam = new CRParameterInfo;
								tmpParam->m_Name = "ReceiptCustomInfo";
								tmpParam->m_Data = GetRemotePropertyText("ReceiptCustomInfo", "", 0, "<None>", true);
								params.Add((void *)tmpParam);				

								CPrintDialog* dlg;
								dlg = new CPrintDialog(FALSE);
								CPrintInfo prInfo;
								prInfo.m_bPreview = false;
								prInfo.m_bDirect = false;
								prInfo.m_bDocObject = false;
								if (prInfo.m_pPD) delete prInfo.m_pPD;
								prInfo.m_pPD = dlg;

								//Made new function for running reports - JMM 5-28-04
								RunReport(&infReport, &params, true, (CWnd *)this, "Payment Dialog", &prInfo);
								ClearRPIParameterList(&params);	//DRT - PLID 18085 - Cleanup after ourselves

								//The CPrintInfo destructor will delete dlg.
							}
						}
					}NxCatchAll("Error printing receipt.");
				}
				break;

				// (j.gruber 2007-03-15 14:39) - PLID 25223 - added sales receipt
			case miPrintSalesReceipt:
				{
					try {
						if(pRow) {
							// (j.gruber 2007-03-30 09:59) - PLID 9802 - took out some unneeeded variables

							CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(585)];
							_variant_t var = pRow->GetValue(btcPayID);
							_variant_t varDate = pRow->GetValue(m_nDateColID);
							if(var.vt == VT_I4 && varDate.vt == VT_DATE){

								//pop up the dialog
								long nPayID = VarLong(var);
								COleDateTime dt = VarDateTime(varDate);			

								// (j.gruber 2007-03-29 14:56) - PLID 9802 - adding support for receipt printer format
								// (j.gruber 2007-07-19 11:01) - PLID 26686 - made this a global function
								RunSalesReceipt(dt, nPayID, 585, GetActivePatientID(), (CWnd *)this);
								/*CSalesReceiptConfigDlg dlg(dt, nPayID, TRUE, 585);
								dlg.DoModal();*/
							}
						}
					}NxCatchAll("Error printing sales receipt.");
				}
				break;

				// (j.gruber 2007-03-15 14:39) - PLID 9820 - added sales receipt - receipt printer format
			case miPrintSalesReceiptRPFormat:
				{
					try {
						if(pRow) {
							//we are only doing POS format for now
							//CReportInfo infReport = CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(587)];
							//see if they have a default set
							long nDefault = GetRemotePropertyInt("POSReceiptDefaultSettings", -1, 0, GetCurrentLocationName(), FALSE);
							if (nDefault < 0) {
								MsgBox("Please select a default setting from the Receipt Settings dialog under Tools->POS Tools->Receipt Printer->Configure Receipts...");
								return;
							}
							_variant_t var = pRow->GetValue(btcPayID);
							_variant_t varDate = pRow->GetValue(m_nDateColID);
							if(var.vt == VT_I4 && varDate.vt == VT_DATE){

								//pop up the dialog
								long nPayID = VarLong(var);
								COleDateTime dt = VarDateTime(varDate);			


								// (j.gruber 2007-07-19 11:01) - PLID 26686 - made this a global function
								RunSalesReceipt(dt, nDefault, nPayID, TRUE, -3, GetActivePatientID(), (CWnd *)this);
								//CSalesReceiptConfigDlg dlg(dt, nDefault, nPayID, TRUE, -3);
								//dlg.DoModal();
							}
						}
					}NxCatchAll("Error printing sales receipt - receipt printer format.");
				}
				break;

			case miPrintCreditCardReceipt: // (z.manning 2015-09-08 09:34) - PLID 67226
			{
				try
				{
					_variant_t varPaymentID = pRow->GetValue(btcPayID);
					if (varPaymentID.vt != VT_NULL) {
						PromptToPrintICCPReceipts(this, VarLong(varPaymentID));
					}
				}
				NxCatchAll("Exception printing credit card receipt");
			}
			break;


			case miEditChargeResp:   //pull up the edit charge resp dialog
				{
					try {
						if(pRow) {
							//get the bill ID and charge ID, the latter is optional
							long nBillID = VarLong(pRow->GetValue(m_nBillIDColID), -1);
							long nChargeID = -1;
							_variant_t var = pRow->GetValue(btcChargeID);
							if(var.vt == VT_I4)
								nChargeID = VarLong(var);

							if (nBillID != -1) {
								GetMainFrame()->DisableHotKeys();
								CEditChargeRespDetailDlg dlg(nBillID, nChargeID, this);
								dlg.DoModal();
								GetMainFrame()->EnableHotKeys();
							}
						}
					}NxCatchAll("Error editing charge responsibilities.");
				}
				break;

			case miChargePosting: {
				InvokePostingDlg(pRow, FALSE);
				break;
								  }

			case miBillPosting: {
				InvokePostingDlg(pRow, TRUE);
				break;
								}

			case miReturnProduct: {
				//TES 7/1/2008 - PLID 26143 - We're returning, not exchanging.
				ReturnProduct(pRow, FALSE);
				break;
								  }

			case miExchangeProduct: {
				//TES 7/1/2008 - PLID 26143 - We're exchanging, not returning.
				ReturnProduct(pRow, TRUE);
				break;
									}

			case miRefundPayment: {
				Pay_Adj_Ref_Click(pRow, m_nRefundColID);
				break;
								  }

			case miSendToPaperBatch: {

				if(pRow) {
					long BillID = VarLong(pRow->GetValue(m_nBillIDColID),-1);

					// (j.jones 2009-12-29 11:01) - PLID 27237 - replaced IsRecordsetEmpty with a parameterized recordset
					_RecordsetPtr rsBill = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM BillsT WHERE ID = {INT} "
						"AND InsuredPartyID IN (SELECT PersonID FROM InsuredPartyT)", BillID);
					if(rsBill->eof) {
						long nPrimaryInsCo = GetInsuranceIDFromType(GetActivePatientID(), 1);
						if(nPrimaryInsCo != -1) {
							if(IDNO == MessageBox("You must have an insurance company selected before batching a claim.\n"
								"Would you like to auto-select the patient's primary insurance?\n\n"
								"(If 'No', then you will need to select an insurance company on the Insurance Tab of the bill\n"
								"before you can batch this bill.)", "Practice", MB_YESNO|MB_ICONINFORMATION)) {
									return;
							}
							else {
								// (j.jones 2009-12-29 14:34) - PLID 27237 - parameterized
								ExecuteParamSql("UPDATE BillsT SET InsuredPartyID = {INT} WHERE ID = {INT}", nPrimaryInsCo, BillID);
							}
						}
						else {
							MsgBox("You must have an insurance company selected on the Insurance Tab of the bill before batching a claim.");
							return;
						}
					}
					rsBill->Close();

					//don't automatically batch a $0.00 claim
					COleCurrency cyCharges = COleCurrency(0,0);
					// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT dbo.GetClaimTotal(ID) AS Total FROM BillsT WHERE ID = {INT}", BillID);
					if(!rs->eof) {
						cyCharges = AdoFldCurrency(rs, "Total",COleCurrency(0,0)); 
					}
					rs->Close();

					CString strWarn;
					strWarn.Format("This claim is currently %s. Are you sure you wish to send the claim to the paper batch?",
						FormatCurrencyForInterface(cyCharges,TRUE,TRUE));

					if(cyCharges > COleCurrency(0,0) || 
						IDYES == MessageBox(strWarn,"Practice",MB_YESNO|MB_ICONQUESTION)) {

							BatchBill(BillID,1);
					}
				}				
				break;
									 }

			case miSendToElectronicBatch: {

				if(pRow) {							
					long BillID = VarLong(pRow->GetValue(m_nBillIDColID),-1);

					// (j.jones 2009-12-29 11:01) - PLID 27237 - replaced IsRecordsetEmpty with a parameterized recordset
					_RecordsetPtr rsBill = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM BillsT WHERE ID = {INT} "
						"AND InsuredPartyID IN (SELECT PersonID FROM InsuredPartyT)", BillID);
					if(rsBill->eof) {
						long nPrimaryInsCo = GetInsuranceIDFromType(GetActivePatientID(), 1);
						if(nPrimaryInsCo != -1) {
							if(IDNO == MessageBox("You must have an insurance company selected before batching a claim.\n"
								"Would you like to auto-select the patient's primary insurance?\n\n"
								"(If 'No', then you will need to select an insurance company on the Insurance Tab of the bill\n"
								"before you can batch this bill.)", "Practice", MB_YESNO|MB_ICONINFORMATION)) {
									return;
							}
							else {
								// (j.jones 2009-12-29 14:34) - PLID 27237 - parameterized
								ExecuteParamSql("UPDATE BillsT SET InsuredPartyID = {INT} WHERE ID = {INT}", nPrimaryInsCo, BillID);
							}
						}
						else {
							MsgBox("You must have an insurance company selected on the Insurance Tab of the bill before batching a claim.");
							return;
						}
					}
					rsBill->Close();

					//don't automatically batch a $0.00 claim
					COleCurrency cyCharges = COleCurrency(0,0);
					// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT dbo.GetClaimTotal(ID) AS Total FROM BillsT WHERE ID = {INT}", BillID);
					if(!rs->eof) {
						cyCharges = AdoFldCurrency(rs, "Total",COleCurrency(0,0)); 
					}
					rs->Close();

					CString strWarn;
					strWarn.Format("This claim is currently %s. Are you sure you wish to send the claim to the electronic batch?",
						FormatCurrencyForInterface(cyCharges,TRUE,TRUE));

					if(cyCharges > COleCurrency(0,0) || 
						IDYES == MessageBox(strWarn,"Practice",MB_YESNO|MB_ICONQUESTION)) {

							BatchBill(BillID,2);
					}
				}
				break;
										  }

			case miUnbatchBill: {

				if(pRow) {
					long BillID = VarLong(pRow->GetValue(m_nBillIDColID),-1);
					BatchBill(BillID,0);					
				}
				break;
								}

			case miViewBillSerializedItems: {
				//PLID 28380 - Initialize dialog based to display all serialized item for a bill	
				try {
					if(pRow) {
						CSerializedItemsDlg dlg(this, VarLong(pRow->GetValue(m_nBillIDColID), -1), TRUE);
						dlg.DoModal();
					}

				} NxCatchAll("Error in FinancialDlg::OnContextMenu:miViewBillSerializedItems");

				break;
											}
			case miViewChargeSerializedItems: {
				//PLID 28380 - Initialize dialog based to display all serialized item for a charge	
				try {
					if(pRow) {
						CSerializedItemsDlg dlg(this, VarLong(pRow->GetValue(btcChargeID), -1), FALSE);
						dlg.DoModal();
					}
				} NxCatchAll("Error in FinancialDlg::OnContextMenu:miViewChargeSerializedItems");

				break;
											  }

											  // (z.manning 2010-06-11 11:49) - PLID 29214 - Transferring payments, etc.
			case miTransferPay:
			case miTransferPrePay:
			case miTransferAdj:
			case miTransferRef:
			{
				long nPayID = VarLong(pRow->GetValue(btcPayID), -1);

				// (j.jones 2011-09-13 15:37) - PLID 44887 - disallow this on corrected/original line items
				if(nPayID != -1) {
					LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nPayID);
					if(licsStatus == licsOriginal) {
						AfxMessageBox("This line item has been corrected, and can not be modified.");
						return;
					}
					else if(licsStatus == licsVoid) {
						AfxMessageBox("This line item is a void line item from an existing correction, and cannot be modified.");
						return;
					}
				}

				TransferPaymentToAnotherPatient(pRow);
				break;
			}
			// (z.manning 2010-06-17 09:07) - Too dangerous, removed
			//case miTransferBill:
			//	TransferBillToAnotherPatient(pRow);
			//	break;

			// (j.armen 2012-04-23 11:15) - PLID 14102 - Handle InsuranceReversals
			case miInsuranceReversal:
				ProcessInsuranceReversal(VarLong(pRow->GetValue(btcPayID)));
				break;

			// (b.spivey, August 27th, 2014) - PLID 63491 - if they asked to print a preview claim file. 
			case miPrintClaimFile:
			{
				// (b.spivey, August 27th, 2014) - PLID 63491 - grab the bill ID and make a claim preview file. 
				long BillID = VarLong(pRow->GetValue(m_nBillIDColID), -1);

				// (j.jones 2009-12-29 11:01) - PLID 27237 - replaced IsRecordsetEmpty with a parameterized recordset
				// (s.tullis 2015-10-20 17:00) - PLID 67406 - Need to get the FormType to open the correct form
				_RecordsetPtr rsBill = CreateParamRecordset(GetRemoteDataSnapshot(),
					R"(
					SELECT case when  Count(ID) = 0 THEN 1  ELSE 0 END as NoINS FROM BillsT WHERE ID = {INT}
					AND InsuredPartyID IN (SELECT PersonID FROM InsuredPartyT);
					Select FormType FROM BillsT WHERE ID = {INT}
					)", BillID,BillID);
				if (AdoFldLong(rsBill,"NoINS") == 1 ? TRUE :FALSE) {
					MsgBox("You must have an insurance company selected on the Insurance Tab of the bill before previewing a claim.");
					break;
				}
				// (s.tullis 2015-10-20 17:00) - PLID 67406 - Get the form type
				rsBill = rsBill->NextRecordset(NULL);
				long nFormType = AdoFldLong(rsBill, "FormType");
				
				CEbilling dlg(this, true, BillID);

				_variant_t var;
				// (b.spivey, August 27th, 2014) - PLID 63491 - This is the format style selected on the ebilling dialog. 
				// (j.jones 2016-05-19 10:22) - NX-100685 - added a universal function for getting the default E-Billing / E-Eligibility format
				dlg.m_FormatID = GetDefaultEbillingANSIFormatID();
				dlg.m_FormatStyle = ANSI;
				// (s.tullis 2015-10-20 17:00) - PLID 67406 - Assign form type
				// not possible to get here without a ANSIClaimType formtype therefore its either HCFA or UB 
				dlg.m_actClaimType = nFormType == 2 ? actInst : actProf;
				dlg.m_nFilterByLocationID = -1;
				dlg.m_nFilterByProviderID = -1;

				CString str = "";
				str = GetRemotePropertyText(_T("ResSubmitter"), "", 0, _T("<None>"));
				if (str != "")
					dlg.m_Contact = str;
				str = GetRemotePropertyText(_T("SiteID"), "", 0, _T("<None>"));
				if (str != "")
					dlg.m_SiteID = str;

			

				int nResult = dlg.DoModal();
				break;
			}
			// (r.gonet 07/07/2014) - PLID 62571 - Handle the menu option to mark a bill on hold
			case miMarkBillOnHold:
				// Fall through
			// (r.gonet 07/07/2014) - PLID 62571 - Handle the menu option to remove a bill's hold.
			case miRemoveBillHold:
				try {
					if (pRow) {
						long nBillID = VarLong(pRow->GetValue(m_nBillIDColID), -1);

						// (r.gonet 07/07/2014) - PLID 62571 - Should be allowed even under financial closes.
						if (!CanChangeHistoricFinancial("Bill", VarLong(pRow->GetValue(m_nBillIDColID), -1), bioBill, sptWrite, FALSE, NULL, TRUE)) {
							break;
						}

						if (IsVoidedBill(nBillID)) {
							AfxMessageBox("This bill has been corrected, and can no longer be modified.");
							break;
						}
						SetBillOnHold(nBillID, (nResult == miMarkBillOnHold ? TRUE : FALSE));
					}
				} NxCatchAll("Error changing the bill's on hold status.");
				break;

			case miViewEOBAssociatedFile:
				long nPayID = pRow->GetValue(btcPayID);
				_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), R"( 
					SELECT BatchPaymentsT.ERemittance, BatchPaymentsT.EOBFilePath
					FROM PaymentsT 
					LEFT JOIN BatchPaymentsT ON PaymentsT.BatchPaymentID = BatchPaymentsT.ID 
					WHERE PaymentsT.ID = {INT} 
					)", nPayID);

				if (!prs->eof) {
					CString strEOBFile = AdoFldString(prs, "EOBFilePath", "");
					// (b.spivey, October 10th, 2014) - PLID 62699 - Does this path exist? 
					if (PathFileExists(strEOBFile)) {
						//Open it if so. 
						int nResult = (int)ShellExecute((HWND)this, NULL, "notepad.exe", (CString("'") + strEOBFile + CString("'")), NULL, SW_SHOW);
					}
					else {
						//Warn them if not. 
						CString str;
						// (b.spivey, October 14, 2014) - PLID 62699 - Spelling mistake
						str.Format("The following EOB file associated with this line item cannot be found. Please call Nextech Product Support for assistance. \r\n\r\n%s", strEOBFile);
						AfxMessageBox(str);
					}
				}

				break;
			}
			
		}

	}NxCatchAll("Error in CFinancialDlg::OnContextMenu");
}

// (j.armen 2012-04-23 11:15) - PLID 14102 - Handle Insurance Reversals
void CFinancialDlg::ProcessInsuranceReversal(const long& nChargeID)
{
	if(!CheckCurrentUserPermissions(bioFinancialCorrections, sptWrite)){
		return;
	}

	//do not allow re-correcting an already corrected payment
	switch(GetLineItemCorrectionStatus(nChargeID))
	{
		case licsOriginal:
			AfxMessageBox("This line item has already been corrected, and can no longer be modified.");
			return;
		case licsVoid:
			AfxMessageBox("This line item is a void line item from an existing correction, and cannot be modified.");
			return;
		default:
			break;
	}

	CString strMessage = 
		"This feature should be used when the payment has been reversed by the insurance company.\r\n\r\n"
		"The following actions will occur:\r\n"
		"\r\n"
		"- A reverse payment will be created and applied to the original payment to offset its amount.\r\n"
		"- The payment amount will be credited towards the selected batch payment.";

	// need to warn if we are unlinking from a quote
	_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(),
		"SELECT BillsT.Description AS QuoteName FROM PaymentsT "
		"LEFT JOIN BillsT ON PaymentsT.QuoteID = BillsT.ID "
		"WHERE PaymentsT.QuoteID IS NOT NULL AND PaymentsT.ID = {INT}", nChargeID);

	if(!prs->eof){
		strMessage += FormatString("\r\n- The payment will be unlinked from the quote named \"%s\".", AdoFldString(prs, "QuoteName", ""));
	}

	if(IDCANCEL == AfxMessageBox(strMessage, MB_OKCANCEL)){
		return;
	}

	if(IDOK == CInsuranceReversalDlg(this, nChargeID).DoModal())
	{
		QuickRefresh();
		CalculateTotals();
	}
}

// (r.gonet 07/07/2014) - PLID 62571 - Changes a bill's on hold state and reflects it on the billing tab.
// - nBillID: ID of the BillsT record to change.
// - bOnHold: TRUE to change the bill to On Hold. FALSE to change it to not On Hold.
void CFinancialDlg::SetBillOnHold(long nBillID, BOOL bOnHold)
{
	CWaitCursor waitCursor;
	if (!::SetBillOnHold(nBillID, bOnHold)) {
		return;
	}
	// Update the bill description in the UI
	RedrawBill(nBillID);
	RefreshList();
}

// (j.jones 2009-08-18 08:44) - PLID 24569 - added pre-payment option
void CFinancialDlg::CreateNewPayment(BOOL bIsPrePayment /*= FALSE*/)
{
	if(!CheckCurrentUserPermissions(bioPayment,sptCreate)) return;

	m_boAllowUpdate = FALSE;

	//////////////////////////////////////////////
	// The dialog will provide the interface for
	// creating a new payment
	CPaymentDlg dlg(this);
	dlg.m_PatientID = m_iCurPatient;
	dlg.m_iDefaultPaymentType = 0;
	dlg.m_bIsPrePayment = bIsPrePayment;
	if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
		// (r.gonet 2015-07-06 18:02) - PLID 65327 - If a bill is created, we need to do more than
		// just redraw the payments.
		if (dlg.m_bBillCreated) {
			QuickRefresh();
		} else {
			RedrawAllPayments();
			CalculateTotals();
		}
	}
	m_boAllowUpdate = TRUE;
}

void CFinancialDlg::CreateNewAdjustment()
{
	if(!CheckCurrentUserPermissions(bioAdjustment,sptCreate)) return;

	m_boAllowUpdate = FALSE;

	//////////////////////////////////////////////
	// The dialog will provide the interface for
	// creating a new adjustment
	CPaymentDlg dlg(this);
	dlg.m_PatientID = m_iCurPatient;
	dlg.m_iDefaultPaymentType = 1;
	if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
		// (r.gonet 2015-07-06 18:02) - PLID 65327 - If a bill is created, we need to do more than
		// just redraw the payments.
		if (dlg.m_bBillCreated) {
			QuickRefresh();
		} else {
			RedrawAllPayments();
			CalculateTotals();
		}
	}
	m_boAllowUpdate = TRUE;
}

void CFinancialDlg::CreateNewRefund()
{
	if(!CheckCurrentUserPermissions(bioRefund,sptCreate)) return;

	m_boAllowUpdate = FALSE;

	// (j.gruber 2007-07-30 12:06) - PLID 26704 - pop up the list of items to find what they want to apply this to
	CApplyToRefundDlg dlgApplyToRefund(this);
	dlgApplyToRefund.m_nPatientID = m_iCurPatient; // (a.walling 2008-07-07 17:13) - PLID 29900 - Do not use GetActive[Patient,Contact][ID,Name]
	dlgApplyToRefund.DoModal();
	long nIDToApplyRefundTo = dlgApplyToRefund.m_nPayID;
	COleCurrency cyPayAmount = dlgApplyToRefund.m_cyPayAmount;

	//TES 7/25/2014 - PLID 63049 - Don't allow them to delete chargeback charges
	if (DoesPayHaveChargeback(CSqlFragment("PaymentsT.ID = {INT}", nIDToApplyRefundTo))) {
		MessageBox("This payment is associated with at least one Chargeback, and cannot be unapplied in order to refund it. In order to delete this bill, you must first expand the bill "
			"on the Billing tab, right-click on any associated Chargebacks, and select 'Undo Chargeback.'");
		return;
	}

	//////////////////////////////////////////////
	// The dialog will provide the interface for
	// creating a new refund
	CPaymentDlg dlg(this);
	dlg.m_PatientID = m_iCurPatient;
	dlg.m_iDefaultPaymentType = 2;
	if (nIDToApplyRefundTo != -1) {
		// (c.haag 2010-10-12 10:38) - PLID 35723 - We now call SetPayToApplyToID
		// (j.jones 2015-09-30 10:37) - PLID 67172 - renamed this function
		// (j.jones 2015-09-30 10:34) - PLID 67171 - ApplyToRefund would already have warned about
		// mismatched credit cards, so we do not need to
		dlg.SetPaymentToApplyTo(nIDToApplyRefundTo, false);
		dlg.m_cyFinalAmount = cyPayAmount;
		dlg.m_ApplyOnOK = TRUE;

		// (j.gruber 2007-08-02 12:16) - PLID 26916 - fill in the cc information from the payment
		// (a.walling 2007-10-31 09:37) - PLID 27891 - Get the encrypted CC number
		// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
		// (a.walling 2010-03-15 12:25) - PLID 37751 - Include KeyIndex
		_RecordsetPtr rsPayPlan = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PayMethod, CCNumber, SecurePAN, KeyIndex, CCHoldersName, CCExpDate, CCAuthNo, CreditCardID FROM PaymentPlansT LEFT JOIN PaymentsT ON PaymentPlansT.ID = PaymentsT.ID WHERE PaymentPlansT.ID = {INT}", nIDToApplyRefundTo);
		if (!rsPayPlan->eof) {
			if (AdoFldLong(rsPayPlan, "PayMethod", -1) == 3) {
				dlg.m_bSetCreditCardInfo = TRUE;
				dlg.m_strNameOnCard = AdoFldString(rsPayPlan, "CCHoldersName", "");
				//(e.lally 2007-10-30) PLID 27892 - Use the new public function to set the cc number
				// (a.walling 2007-10-31 09:37) - PLID 27891 - This is just the last 4 digits now. Send the unencrypted CCNumber, and 
				// the payment dlg will take care of privatizing it.
				// dlg.SetCreditCardNumber(AdoFldString(rsPayPlan, "CCNumber", ""));
				// (a.walling 2010-03-15 12:25) - PLID 37751 - Use NxCrypto
				//dlg.SetCreditCardNumber(DecryptStringFromVariant(rsPayPlan->Fields->Item["SecurePAN"]->Value));
				CString strCCNumber;
				NxCryptosaur.DecryptStringFromVariant(rsPayPlan->Fields->Item["SecurePAN"]->Value, AdoFldLong(rsPayPlan, "KeyIndex", -1), strCCNumber);
				dlg.SetCreditCardNumber(strCCNumber);
				COleDateTime dtNULL;
				dtNULL.SetDate(1899,12,31);
				COleDateTime dtExpire = AdoFldDateTime(rsPayPlan, "CCExpDate", dtNULL);
				if (dtExpire != dtNULL) {
					CString strMonth = AsString((long)dtExpire.GetMonth());
					if (strMonth.GetLength() == 1) {
						strMonth = '0' + strMonth;
					}
					dlg.m_strExpDate = strMonth + "/" + AsString((long)dtExpire.GetYear()).Right(2);
				}
				dlg.m_nCreditCardID = AdoFldLong(rsPayPlan, "CreditCardID", -1);
			}
			else {
				dlg.m_bSetCreditCardInfo = FALSE;
			}
		}
	}
	if (IDCANCEL != dlg.DoModal(__FUNCTION__, __LINE__)) {
		// (r.gonet 2015-07-06 18:02) - PLID 65327 - If a bill is created, we need to do more than
		// just redraw the payments.
		if (dlg.m_bBillCreated) {
			QuickRefresh();
		} else {
			RedrawAllPayments();
			CalculateTotals();
		}
		
	}
	m_boAllowUpdate = TRUE;
}

// (j.jones 2007-04-05 14:57) - PLID 25514 - added ability to print a claim for any resp type
void CFinancialDlg::PrintClaimForInsurance(long nBillID, long nInsuredPartyID)
{
	try {

		if (!g_pLicense->CheckForLicense(CLicense::lcHCFA, CLicense::cflrUse))
		{	MsgBox("Your license indicates you didn't purchase insurance billing,\n"
		"please call Nextech if you wish to use this feature.");
		return;
		}

		if (!CheckCurrentUserPermissions(bioClaimForms,sptRead))
			return;

		// (j.jones 2008-02-11 15:25) - PLID 28847 - added option to disable batching/printing claims with 100% patient resp.
		// (j.jones 2010-10-21 14:48) - PLID 41051 - this function now warns why the claim can't be created
		if(!CanCreateInsuranceClaim(nBillID, FALSE)) {
			return;
		}

		// (j.jones 2005-05-23 10:28) - PLID 16512 - what we need to do is the following:
		// - store the current insurance/other insurance on the bill
		// - make the chosen insurance be the bill's dominant insurance
		// - make the previous dominant insurance (if one exists) the "other" insurance
		// - print
		// - revert to the previously stored insurances on the bill


		// - store the current insurance/other insurance on the bill

		long nFormType = 1;
		long nSavedInsuredPartyID = -1;
		long nSavedOthrInsuredPartyID = -1;

		// (j.jones 2009-12-28 17:21) - PLID 27237 - parameterized
		_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT FormType, InsuredPartyID, OthrInsuredPartyID FROM BillsT WHERE ID = {INT}", nBillID);
		if(!rs->eof) {
			nFormType = AdoFldLong(rs, "FormType",1);
			nSavedInsuredPartyID = AdoFldLong(rs, "InsuredPartyID",-1);
			nSavedOthrInsuredPartyID = AdoFldLong(rs, "OthrInsuredPartyID",-1);
		}
		else {
			//this should be impossible
			ASSERT(FALSE);
			return;
		}
		rs->Close();

		// - make the chosen insurance be the bill's dominant insurance
		// - make the previous dominant insurance (if one exists) the "other" insurance

		long nNewInsuredPartyID = -1;

		if(nInsuredPartyID != -1) {
			nNewInsuredPartyID = nInsuredPartyID;

			_variant_t varOthrInsID = g_cvarNull;
			if(nSavedInsuredPartyID != -1 && nSavedInsuredPartyID != nNewInsuredPartyID) {
				varOthrInsID = nSavedInsuredPartyID;
			}
			else if(nSavedOthrInsuredPartyID != -1) {
				varOthrInsID = nSavedOthrInsuredPartyID;
			}

			// (j.jones 2009-03-11 08:46) - PLID 32864 - we do not audit this because this is a temporary change
			// (j.jones 2009-12-29 14:34) - PLID 27237 - parameterized
			ExecuteParamSql("UPDATE BillsT SET InsuredPartyID = {INT}, OthrInsuredPartyID = {VT_I4} WHERE ID = {INT}",nNewInsuredPartyID,varOthrInsID,nBillID);
		}
		else {
			//this should be impossible
			ASSERT(FALSE);
			return;
		}

		// - print

		switch(nFormType) {
		case 1: {	// Open HCFA

			CHCFADlg hcfa(this);

			// (j.jones 2009-09-01 08:41) - PLID 34749 - we now disallow opening HCFAs without a HCFA group
			long nHCFAGroupID = -1;
			if(nNewInsuredPartyID != -1) {
				_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuranceCoT.HCFASetupGroupID FROM InsuranceCoT "
					"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
					"WHERE InsuredPartyT.PersonID = {INT}", nNewInsuredPartyID);
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

			hcfa.m_PatientID = m_iCurPatient;
			hcfa.DoModal(nBillID);
				}
				break;
		case 2:
			{
				// (j.jones 2009-09-01 08:41) - PLID 34749 - we now disallow opening UBs without a UB group
				long nUBGroupID = -1;
				if(nNewInsuredPartyID != -1) {
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuranceCoT.UB92SetupGroupID FROM InsuranceCoT "
						"INNER JOIN InsuredPartyT ON InsuranceCoT.PersonID = InsuredPartyT.InsuranceCoID "
						"WHERE InsuredPartyT.PersonID = {INT}", nNewInsuredPartyID);
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

				//TES 3/13/2007 - PLID 24993 - Reflect the UB type
				if(GetUBFormType() == eUB04) {
					CUB04Dlg ub04(this);
					ub04.m_PatientID = m_iCurPatient;
					ub04.DoModal(nBillID);
				}
				else {
					CUB92Dlg ub92(this);
					ub92.m_PatientID = m_iCurPatient;
					ub92.DoModal(nBillID);
				}
			}
			break;
		case 3:
			{
				// (j.armen 2014-03-05 09:17) - PLID 60784 - Fill BillID, PatientID in constructor
				CADADlg ada(this, nBillID, m_iCurPatient);
				ada.DoModal();
			}
			break;
		case 4:
			{
				CIDPADlg idpa(this);
				idpa.m_PatientID = m_iCurPatient;
				idpa.DoModal(nBillID);
			}
			break;
		case 5:
			{
				CNYWCDlg nywc(this);
				nywc.m_PatientID = m_iCurPatient;
				nywc.DoModal(nBillID);
			}
			break;
		case 6:
			{
				// (j.jones 2007-05-09 13:56) - PLID 25550 - check the internal preference
				// for which MICR form to use

				if(GetRemotePropertyInt("Use2007MICR", 1, 0, "<None>", true) == 1) {

					CMICR2007Dlg micr(this);
					micr.m_PatientID = m_iCurPatient;
					micr.DoModal(nBillID);
				}
				else {

					CMICRDlg micr(this);
					micr.m_PatientID = m_iCurPatient;
					micr.DoModal(nBillID);
				}
			}
			break;

		case 7: {

			// (j.jones 2010-02-04 11:51) - PLID 37113 - supported the NY Medicaid form
			CNYMedicaidDlg nymcaid(this);
			nymcaid.m_PatientID = m_iCurPatient;
			nymcaid.DoModal(nBillID);
			break;
				}
		}

		// - revert to the previously stored insurances on the bill

		_variant_t varInsID = g_cvarNull;
		if(nSavedInsuredPartyID != -1) {
			varInsID = nSavedInsuredPartyID;
		}

		_variant_t varOthrInsID = g_cvarNull;
		if(nSavedOthrInsuredPartyID != -1) {
			varOthrInsID = nSavedOthrInsuredPartyID;
		}

		// (j.jones 2009-03-11 08:46) - PLID 32864 - we do not audit this because this is a temporary change
		// (j.jones 2009-12-29 14:34) - PLID 27237 - parameterized
		ExecuteParamSql("UPDATE BillsT SET InsuredPartyID = {VT_I4}, OthrInsuredPartyID = {VT_I4} WHERE ID = {INT}",varInsID,varOthrInsID,nBillID);

		// (j.jones 2010-06-14 10:34) - PLID 39120 - update the bill line to reflect the last sent date, but only
		// if we are displaying the last sent date
		// (d.thompson 2012-08-07) - PLID 51969 - Changed default to Yes
		if(GetRemotePropertyInt("ShowBillLastSentDateInBillingTabBillDesc", 1, 0, "<None>", true) == 1) {
			RedrawBill(nBillID);
			RefreshList();
		}

	}NxCatchAll("Error printing claim.");
}

void CFinancialDlg::OnCheckHideZeroBalances() 
{
	if(m_checkHideZeroBalances.GetCheck()) {
		SetRemotePropertyInt("BillingHideBalances", 1, 0, GetCurrentUserName());
	}
	else {
		SetRemotePropertyInt("BillingHideBalances", 0, 0, GetCurrentUserName());
	}

	//redraw the screen
	QuickRefresh();
}

// (j.jones 2015-02-19 09:35) - PLID 64942 - This is used for the Collapsed bills recordset
// and the Unapplied pays recordset, the return value is different for each.
// Set the parameter to true for bills, false for pays.
CString CFinancialDlg::GetZeroBalanceFilter(bool bBillsRecordset)
{
	CString strWhere = "";

	if(m_checkHideZeroBalances.GetCheck()) {
		long nRange = GetRemotePropertyInt("BillingHideBalancesRange", 3, 0, GetCurrentUserName(), TRUE);
		long nRangeType = GetRemotePropertyInt("BillingHideBalancesRangeType", 2, 0, GetCurrentUserName(), TRUE);
		//nRangeType: 1 = Days, 2 = Months, 3 = Years

		CString strRangeType;
		if(nRangeType == 1)
			strRangeType = "day";
		else if(nRangeType == 2)
			strRangeType = "month";
		else
			strRangeType = "year";

		if (bBillsRecordset) {
			//for use in AddCollapsedBillsToSqlBatch()
			strWhere.Format("AND (SumOfChargeTotal-SumOfPayTotal-SumOfAdjTotal <> 0 OR FinTabBillsQ.Date > DATEADD(%s,-%li,GetDate()))",
				strRangeType, nRange);
		}
		else {
			//for use in CreateAddUnappliedPaysRecordset()

			// (j.jones 2015-02-19 09:38) - PLID 64942 - supported filtering payments
			strWhere.Format("AND ((MIN(-1*FinTabPaySubQ.Amount) + MIN((CASE WHEN [Outgoing] IS NULL THEN 0 ELSE [Outgoing] END)) - SUM(CASE WHEN PatientAppliesQ.Amount IS NULL THEN 0 ELSE PatientAppliesQ.Amount END)) <> 0 "
				"OR FinTabPaySubQ.Date > DATEADD(%s,-%li,GetDate()))",
				strRangeType, nRange);
		}
	}

	return strWhere;
}

// (j.jones 2015-02-23 15:22) - PLID 64934 - This is used in all recordsets, the return value
// is different for bills vs. charges/payments.
// Set the parameter to true for bills, false for any LineItemT records.
CSqlFragment CFinancialDlg::GetHideVoidedItemsFilter(bool bBillsRecordset)
{
	CSqlFragment sqlHideVoidedItems("");

	if (m_checkHideVoidedItems.GetCheck()) {
		if (bBillsRecordset) {
			//bills only have original & new, there are no void bills
			sqlHideVoidedItems = CSqlFragment(" AND OriginalBillsQ.ID Is Null");
		}
		else {
			//charges and payments will hide original & void line items
			sqlHideVoidedItems = CSqlFragment(" AND OriginalCorrectionsQ.ID Is Null AND VoidingCorrectionsQ.ID Is Null");
		}
	}

	return sqlHideVoidedItems;
}

void CFinancialDlg::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent) {
	case IDT_SHOW_WARNING: {

		KillTimer(IDT_SHOW_WARNING);

		DontShowMeAgain(this, "There is now an option in the lower left of the Billing tab that hides zero balance\n"
			"bills over a given date range. This feature defaults to on, and the date range can\n"
			"be configured in the Billing section of Preferences.",
			"BillingHideBalancesDontShowWarning", "Practice", FALSE, FALSE);

		break;
						   }
	}

	CNxDialog::OnTimer(nIDEvent);
}

// (a.walling 2006-06-23 10:15) - PLID 20153 Refresh the packages window if it exists and is visible
// (j.jones 2006-08-07 15:13) - PLID 21813 - changed to also refresh the quotes list
void CFinancialDlg::RefreshPackagesQuotes()
{
	// sometime in the future we can run a check to see if this bill is a package or not.
	// currently the overhead of checking outweighs the benefits.
	if (m_dlgPackages.GetSafeHwnd()) {
		if (m_dlgPackages.IsWindowVisible()) {
			m_dlgPackages.Requery();
		}
	}

	if (m_dlgQuotes.GetSafeHwnd()) {
		if (m_dlgQuotes.IsWindowVisible()) {
			m_dlgQuotes.Requery();
		}
	}
}

// (j.jones 2008-09-17 15:01) - PLID 31405 - after refreshing the list, we must resize the description column
void CFinancialDlg::ResizeDescriptionColumn()
{
	try {

		//Warning: This function is called very frequently and should
		//NEVER have a database access inside it.

		//We want to disable the screen when we do this, but we
		//can't call the Disable/EnableFinancialScreen functions
		//blindly because EnableFinancialScreen calls this function
		//itself. So check if the screen is currently enabled,
		//and if so, we will disable the screen now, and re-enable
		//it later.
		BOOL bIsResponsibleForScreenRedraw = FALSE;
		if(m_bIsScreenEnabled) {
			m_List->SetRedraw(FALSE);
			bIsResponsibleForScreenRedraw = TRUE;
		}

		long nTotalWidth = 0;
		long nListWidth = 0;

		//loop through the columns up to the balance column, skipping
		//the description column, and find the total width
		{
			for(int i=btcLineID; i <= m_nBalanceColID; i++) {
				if(i != m_nDescriptionColID) {
					IColumnSettingsPtr pCol = m_List->GetColumn(i);
					nTotalWidth += pCol->GetStoredWidth();
				}
			}
		}

		//now, find out how wide the datalist is
		{
			CRect rc;
			GetDlgItem(IDC_BILLING_TAB_LIST)->GetWindowRect(&rc);
			ScreenToClient(&rc);
			nListWidth = rc.Width();
		}

		//We can't reliably determine whether a scrollbar exists, so
		//just take back enough space to account for it at all times.
		//Get the system scrollbar width, and pad with 8 pixels to
		//make the edge of the Balance column come close to, but not
		//quite hit the edge of the list.
		{
			long nScrollbarWidth = GetSystemMetrics(SM_CXVSCROLL);		
			nListWidth -= (nScrollbarWidth + 8);
		}

		//Calculate the size of our description column to be the gap between
		//the balance column and the right side of the datalist width.
		{
			long nDescriptionSize = nListWidth - nTotalWidth;

			//Force a minimum size of the description of 50 pixels. If we do this,
			//the balance will be off the right side of the screen, but this should
			//not occur unless they are running in an unsupported resolution, or
			//are using an absurd font size.

			// (j.jones 2010-06-18 14:59) - PLID 39150 - A user can now show
			// a dynamic number of insurance columns. If they show too many of them,
			// depending on their resolution this code may be hit more often, and
			// intentionally shove the Balance column (or more!) off the right side.

			if(nDescriptionSize < 50) {
				nDescriptionSize = 50;
			}

			//Now apply this size to the description column
			IColumnSettingsPtr pCol = m_List->GetColumn(m_nDescriptionColID);
			pCol->PutStoredWidth(nDescriptionSize);
		}

		//we won't redraw the screen unless we were responsible
		//for disabling it in the first place
		if(bIsResponsibleForScreenRedraw) {
			m_List->SetRedraw(TRUE);
		}

	}NxCatchAll("Error in CFinancialDlg::ResizeDescriptionColumn");
}

// (j.jones 2008-09-17 15:56) - PLID 31405 - required to detect when the screen resized
int CFinancialDlg::SetControlPositions()
{
	long nAns = 0;

	try {

		nAns = CNxDialog::SetControlPositions();

		//resize the description column to match the new screen resize
		// (j.dinatale 2010-10-27) - PLID 28773 - Only resize the description column if the user isnt opting to remember col width
		if(!m_bRememberColWidth){
			ResizeDescriptionColumn();
		}

	}NxCatchAll("Error in CFinancialDlg::SetControlPositions");

	return nAns;
}

// (j.jones 2008-09-18 09:07) - PLID 19623 - put column coloring in one function
// (j.jones 2010-06-16 16:59) - PLID 39102 - this now takes in a short, not an enum
COLORREF CFinancialDlg::GetColorForColumn(short iColumn, BOOL bGrayOutColumns)
{
	COLORREF clr = (0x0080EEEE);

	try {

		if(iColumn == m_nBillIDColID) {
			clr = (0x0087DCE8);
		}
		else if(iColumn == m_nExpandCharColID || iColumn == m_nExpandSubCharColID) {
			clr = (0x0097DCE8);
		}
		else if(iColumn == m_nDateColID) {
			clr = (0x0080DDDD);
		}
		else if(iColumn == m_nNoteIconColID || iColumn == m_nDiscountIconColID || iColumn == m_nDescriptionColID) {
			clr = (0x0080EEEE);
		}
		else if(iColumn == m_nChargeColID) {
			clr = (0x00FFC0C0);
		}
		// (j.jones 2015-02-27 09:41) - PLID 64944 - added Total Pay Amount
		else if (iColumn == m_nTotalPaymentAmountColID) {
			clr = (RGB(242,242,162));
		}
		else if(iColumn == m_nPaymentColID) {
			clr = (RGB(192,255,192));
		}
		else if(iColumn == m_nAdjustmentColID) {
			clr = (RGB(0,230,230));
		}
		else if(iColumn == m_nRefundColID) {
			clr = (0x0000CCCC);
		}
		else if(iColumn == m_nPatientBalanceColID) {
			clr = (0x00B0B0EE);
		}
		else if(iColumn == m_nPrimInsColID) {
			if(m_GuarantorID1 == -1 && bGrayOutColumns) {
				clr = (0x00999999);
			}
			else {
				clr = (0x00B0B0EE);
			}
		}
		else if(iColumn == m_nSecInsColID) {
			if(m_GuarantorID2 == -1 && bGrayOutColumns) {
				clr = (0x00999999);
			}
			else {
				clr = (0x00B0B0EE);
			}
		}
		else if(iColumn == m_nOtherInsuranceColID) {
			if(!m_bOtherInsExists && bGrayOutColumns) {
				clr = (0x00999999);
			}
			else {
				clr = (0x00B0B0EE);
			}
		}
		else if(iColumn == m_nBalanceColID) {
			clr = (0x00C0C0FF);
		}
		else {

			// (j.jones 2010-06-17 17:01) - PLID 39150 - is it a dynamic insurance column?
			long nRespTypeID = -2;
			if(m_mapColumnIndexToRespTypeID.Lookup(iColumn, nRespTypeID) && nRespTypeID != -2) {

				//it is a dynamic insurance column, does this patient have a guarantor?
				long nInsuredPartyID = -1;

				//if we aren't graying out columns, always use the insurance color
				if(!bGrayOutColumns || (m_mapRespTypeIDsToGuarantorIDs.Lookup(nRespTypeID, nInsuredPartyID) && nInsuredPartyID != -1)) {
					//insurance color
					clr = (0x00B0B0EE);
				}
				else {
					//gray
					clr = (0x00999999);
				}
			}
			else {
				//all remaining columns should be the same yellow color
				clr = (0x0080EEEE);
			}
		}

	}NxCatchAll("Error in CFinancialDlg::SetControlPositions");

	return clr;

}

// Adds bills to FinancialTabInfoT
// (may add 1 or all depending on [Bill ID])
// (j.jones 2009-12-30 09:14) - PLID 36729 - replaced with functions that append select statements to a batch
// (j.jones 2015-02-19 09:43) - PLID 64942 - removed the where clause param
void CFinancialDlg::AddCollapsedBillsToSqlBatch(CString &strSqlBatch, CNxParamSqlArray &aryParams, _variant_t varBillID, long nPatientID)
{
	// (j.jones 2008-09-18 09:48) - PLID 19623 - significantly reformatted these queries to be more readable,
	// and added Provider, POSName, POSCode, InputDate, CreatedBy, and FirstChargeDate columns
	// (j.gruber 2009-03-17 13:09) - PLID 33360 - changed for discount structure

	// (j.jones 2009-12-30 09:43) - PLID 36729 - the additional where clause is often the zero balance filter,
	// which cannot be parameterized, but fortunately it is not frequently changed, and will just be cached
	// as part of the regular query

	// (j.jones 2010-06-18 09:39) - PLID 39150 - We can have a dynamic amount of "extra" insurance resps
	// beyond 1, 2, and Other. Build the clauses now.
	CString strDynInsRespClause1;
	CString strDynInsRespClause2;
	CString strDynInsRespClause3;
	CString strDynInsRespClause4;
	CString strDynInsRespClause5;
	CString strDynInsRespClause6;
	for(int i=0; i<m_aryDynamicRespTypeIDs.GetSize(); i++) {

		long nID = (long)m_aryDynamicRespTypeIDs.GetAt(i);

		CString str;
		str.Format("-SumOfIns%liResp", nID);
		strDynInsRespClause1 += str;

		str.Format("SumOfIns%liResp AS Ins%liResp, ", nID, nID);
		strDynInsRespClause2 += str;

		str.Format("SUM(FinTabBillSubQ.Ins%liTotal) AS SumOfIns%liResp, ", nID, nID);
		strDynInsRespClause3 += str;

		str.Format("SUM(Ins%liTotal) AS Ins%liTotal, ", nID, nID);
		strDynInsRespClause4 += str;

		str.Format("(CASE WHEN InsuredPartyT.RespTypeID = %li THEN ChargeRespT.Amount ELSE 0 END) - (CASE WHEN InsuredPartyT.RespTypeID = %li THEN COALESCE(PatientAppliesQ.TotalPayApplies, 0) + COALESCE(PatientAppliesQ.TotalAdjApplies,0) ELSE 0 END) AS Ins%liTotal, ", nID, nID, nID);
		strDynInsRespClause5 += str;

		str.Format(" AND InsuredPartyT.RespTypeID <> %li ", nID);
		strDynInsRespClause6 += str;
	}
	
	// (j.jones 2015-02-19 09:39) - PLID 64942 - Moved this call from outside the function
	// to be inside this function. Also this now takes in true to indicate we're filtering on bills.
	CString strZeroBalanceFilter = GetZeroBalanceFilter(true);

	// (j.jones 2015-02-23 12:59) - PLID 64934 - Added ability to hide voided/original items.
	// We need both the bills filter and charges filter here because we could have a non-void
	// bill with only void charges, which will also be suppressed.
	CSqlFragment sqlHideVoidedBills = GetHideVoidedItemsFilter(true);
	CSqlFragment sqlHideVoidedCharges = GetHideVoidedItemsFilter(false);

	// (j.jones 2010-05-26 16:56) - PLID 28184 - added Diag Code fields	
	// (j.jones 2010-06-14 09:29) - PLID 39120 - added LastSentDate
	// (j.jones 2010-06-18 09:21) - PLID 39150 - renamed Ins3Resp to InsRespOther
	// (j.gruber 2011-06-30 17:28) - PLID 44848 - Show corrected Bills
	//(r.wilson 10/2/2012) plid 52970 - Replace hardcoded SendType with enumerated value
	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
	// (d.singleton 2014-03-04 16:01) - PLID 61172 - update to use the new diag code structure,  also include related icd10 codes
	// (j.jones 2014-03-07 10:17) - PLID 60860 - updated BillDiagCodeList for ICD-9/10 pairings
	// (a.wilson 2014-07-24 12:43) - PLID 63015 - added on hold status.
	// (d.singleton 2014-10-14 15:52) - PLID 62698 - added paymentst.claimnumber
	// (j.jones 2015-02-20 13:22) - PLID 64935 - added IsVoided and IsCorrected, it is possible for both to be true
	// if the corrected item was re-voided
	// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field, always blank on bills
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, FormatString("SELECT " 
		"'Bill' AS LineType, " 
		"FinTabBillsQ.ID, " 
		"NULL AS Expr2, " 
		"'BITMAP:DOWNARROW' AS ExpandChar, " 
		"FinTabBillsQ.Date, " 
		"FinTabBillsQ.LocName, " 
		"FinTabBillsQ.POSName, " 
		"FinTabBillsQ.POSCode, " 
		"FinTabBillsQ.InputDate, " 
		"'BITMAP:FILE' AS NoteIcon, " 
		"CONVERT(INT,(CASE WHEN FinTabBillsQ.ID IN (SELECT BillID FROM Notes WHERE Notes.BillID IS NOT NULL) THEN 1 ELSE 0 END)) AS HasNotes, " 
		"FinTabBillsQ.CorrectDesc + 'BILL - ' + IsNull(FinTabBillsQ.Description, '(No description)') AS Description, " 
		"SumOfChargeTotal AS ChargeAmount, " 
		"SumOfPayTotal AS PayAmount, " 
		"SumOfAdjTotal AS Adjustment, " 
		"SumOfChargeTotal-SumOfPayTotal-SumOfAdjTotal AS Balance, " 
		"SumOfChargeTotal-SumOfIns1Resp-SumOfIns2Resp%s-SumOfInsRespOther-SumOfPayTotal-SumOfAdjTotal AS PatResp, " 
		"SumOfIns1Resp AS Ins1Resp, " 
		"SumOfIns2Resp AS Ins2Resp, " 
		"%s "
		"SumOfInsRespOther AS InsRespOther, " 
		"CONVERT(money,0) AS Refund, " 
		"CONVERT(money,SumOfDiscounts,2) AS DiscountTotal, " 
		"HasPercentOff, " 
		"Provider, " 
		"FinTabBillsQ.CreatedBy, " 
		"FinTabBillsQ.FirstChargeDate, " 
		"FinTabBillsQ.DiagCode1, "
		"FinTabBillsQ.DiagCode1WithName, "
		"FinTabBillsQ.BillDiagCodeList, "
		"FinTabBillsQ.ChargeDiagCodeList, "
		"FinTabBillsQ.LastSentDate, "		
		"FinTabBillsQ.InvoiceID, "
		"FinTabBillsQ.OnHold, "
		"FinTabBillsQ.IsVoided, "
		"FinTabBillsQ.IsCorrected, "
		"NULL AS TotalPayAmount "

		"\r\n\r\n"

		"FROM " 
		"(SELECT PatientBillsQ.ID, PatientBillsQ.Date, PatientBillsQ.LocName, " 
		"PatientBillsQ.POSName, PatientBillsQ.POSCode, PatientBillsQ.Description, " 
		"ROUND(CONVERT(money,PatientBillsQ.BillTotal),2) AS SumOfChargeTotal, " 
		"SUM(FinTabBillSubQ.PayTotal) AS SumOfPayTotal, " 
		"SUM(FinTabBillSubQ.AdjTotal) AS SumOfAdjTotal, " 
		"SUM(FinTabBillSubQ.Ins1Total) AS SumOfIns1Resp, " 
		"SUM(FinTabBillSubQ.Ins2Total) AS SumOfIns2Resp, " 
		"%s "
		"SUM(FinTabBillSubQ.InsOtherTotal) AS SumOfInsRespOther, " 
		"SUM(FinTabBillSubQ.Discounts) AS SumOfDiscounts, " 
		"CASE WHEN SUM(HasPercentOff) > 0 THEN 1 ELSE 0 END AS HasPercentOff, " 
		"dbo.GetBillProviderList(PatientBillsQ.ID) AS Provider, " 
		"PatientBillsQ.InputDate, " 
		"PatientBillsQ.CreatedBy, " 
		"PatientBillsQ.FirstChargeDate, " 
		"PatientBillsQ.DiagCode1, "
		"PatientBillsQ.DiagCode1WithName, "
		"PatientBillsQ.BillDiagCodeList, "
		"PatientBillsQ.ChargeDiagCodeList, "
		"PatientBillsQ.LastSentDate, "
		"PatientBillsQ.CorrectDesc, "
		"PatientBillsQ.InvoiceID, "
		"PatientBillsQ.OnHold, "
		"PatientBillsQ.IsVoided, "
		"PatientBillsQ.IsCorrected "

		"\r\n\r\n"

		"FROM " 
		"(SELECT BillsT.ID, BillsT.Date, "
		"LocationsT.Name AS LocName, "
		"CASE WHEN BillStatusT.Type = 1 THEN 1 ELSE 0 END AS OnHold, "
		"PlaceOfServiceLocT.Name AS POSName, "
		"Min(PlaceOfServiceCodesT.PlaceCodes) AS POSCode, "
		"BillsT.Description, BillsT.InputDate, " 
		"BillsT.EntryType, BillsT.PatientID, " 
		"SUM(COALESCE(ChargeRespT.Amount,0)) AS BillTotal, " 
		"UsersT.UserName AS CreatedBy, " 
		"Min(LineItemT.Date) AS FirstChargeDate, "
		"CASE WHEN Diag9Code1.ID Is Not Null AND Diag10Code1.ID Is Not Null THEN Diag10Code1.CodeNumber + ' (' + Diag9Code1.CodeNumber + ')' "
		"ELSE Coalesce(Diag10Code1.CodeNumber, Diag9Code1.CodeNumber) END AS DiagCode1, "
		""
		"STUFF((SELECT ', ' + "
		"	CASE WHEN DiagCodes9.ID Is Not Null AND DiagCodes10.ID Is Not Null THEN DiagCodes10.CodeNumber + ' (' + DiagCodes9.CodeNumber + ')' "
		"	ELSE Coalesce(DiagCodes10.CodeNumber, DiagCodes9.CodeNumber) END "
		"	FROM BillDiagCodeT "
		"	LEFT JOIN DiagCodes DiagCodes9 ON DiagCodes9.ID = BillDiagCodeT.ICD9DiagID "
		"	LEFT JOIN DiagCodes DiagCodes10 ON DiagCodes10.ID = BillDiagCodeT.ICD10DiagID "
		"	WHERE BillDiagCodeT.BillID = BillsT.ID "
		"	ORDER BY BillDiagCodeT.OrderIndex "
		"	FOR XML PATH('') "
		"), 1, 2, '') AS BillDiagCodeList, "
		""
		"NULL AS ChargeDiagCodeList, "
		"CASE WHEN Diag9Code1.ID Is Not Null AND Diag10Code1.ID IS Not Null THEN Diag10Code1.CodeNumber + ' - ' + Diag10Code1.CodeDesc + ' (' + Diag9Code1.CodeNumber + ' - ' + Diag9Code1.CodeDesc + ')' "
		"ELSE Coalesce(Diag10Code1.CodeNumber + ' - ' + Diag10Code1.CodeDesc, Diag9Code1.CodeNumber + ' - ' + Diag9Code1.CodeDesc) END AS DiagCode1WithName, "
		"ClaimHistoryQ.LastDate AS LastSentDate, "
		"BillInvoiceNumbersT.InvoiceID, "
		"CASE WHEN CorrectedBillsQ.ID IS NOT NULL THEN 'Corrected ' ELSE '' END AS CorrectDesc, "
		"(CASE WHEN OriginalBillsQ.ID Is Not Null THEN 1 ELSE 0 END) AS IsVoided, "
		"(CASE WHEN CorrectedBillsQ.ID Is Not Null THEN 1 ELSE 0 END) AS IsCorrected "
		"FROM BillsT " 
		"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID " 
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID " 
		"LEFT JOIN LineItemCorrectionsT AS OriginalCorrectionsQ ON ChargesT.ID = OriginalCorrectionsQ.OriginalLineItemID "
		"LEFT JOIN LineItemCorrectionsT AS VoidingCorrectionsQ ON ChargesT.ID = VoidingCorrectionsQ.VoidingLineItemID "
		"LEFT JOIN BillInvoiceNumbersT ON BillsT.ID = BillInvoiceNumbersT.BillID "
		"LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID " 
		"LEFT JOIN ("
		"	SELECT Max(Date) AS LastDate, ClaimHistoryT.BillID "
		"	FROM ClaimHistoryT "
		//(r.wilson 10/2/2012) plid 52970 - Line below use to be "WHERE SendType >= 0 "
		"	WHERE SendType >= {INT} "
		"	GROUP BY ClaimHistoryT.BillID "
		") AS ClaimHistoryQ ON BillsT.ID = ClaimHistoryQ.BillID "
		"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID " 
		"LEFT JOIN LocationsT PlaceOfServiceLocT ON BillsT.Location = PlaceOfServiceLocT.ID " 
		"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID " 
		"LEFT JOIN UsersT ON BillsT.InputName = UsersT.PersonID "
		"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID "
		"LEFT JOIN DiagCodes Diag9Code1 ON BillDiagCodeFlat4V.ICD9Diag1ID = Diag9Code1.ID "
		"LEFT JOIN DiagCodes Diag10Code1 ON BillDiagCodeFlat4V.ICD10Diag1ID = Diag10Code1.ID "
		"LEFT JOIN BillCorrectionsT AS OriginalBillsQ ON BillsT.ID = OriginalBillsQ.OriginalBillID "
		"LEFT JOIN BillCorrectionsT AS CorrectedBillsQ ON BillsT.ID = CorrectedBillsQ.NewBillID "
		"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
		"\r\n"
		"WHERE BillsT.PatientID = {INT} " 
		"	AND BillsT.Deleted = 0 AND LineItemT.Deleted = 0 " 
		"	{SQL} {SQL}"
		"\r\n"
		"GROUP BY BillsT.ID, BillsT.Date, LocationsT.Name, BillsT.Description, " 
		"PlaceOfServiceLocT.Name, BillsT.EntryType, BillsT.PatientID, " 
		"BillsT.InputDate, UsersT.UserName, Diag9Code1.ID, Diag9Code1.CodeNumber, Diag9Code1.CodeDesc, " 
		"Diag10Code1.ID, Diag10Code1.CodeNumber, Diag10Code1.CodeDesc, "
		"ClaimHistoryQ.LastDate, OriginalBillsQ.ID, CorrectedBillsQ.ID, BillInvoiceNumbersT.InvoiceID, BillStatusT.Type "
		"\r\n"
		") AS PatientBillsQ " 

		"\r\n"

		"LEFT JOIN " 
		"(SELECT BillID, SUM(PayTotal) AS PayTotal, SUM(AdjTotal) AS AdjTotal, " 
		"SUM(Ins1Total) AS Ins1Total, SUM(Ins2Total) AS Ins2Total, %s " 
		"SUM(InsOtherTotal) AS InsOtherTotal, SUM(TotalDiscount) AS Discounts, " 
		"CASE WHEN SUM(TotalPercentOff) > 0 THEN 1 ELSE 0 END AS HasPercentOff " 
		"FROM " 
		"(SELECT ChargesT.BillID, " 
		"dbo.GetChargeTotal(ChargesT.ID) AS ChargeTotal, " 
		"PatientChargesQ.TotalDiscount, PatientChargesQ.TotalPercentOff, " 
		"CASE WHEN PatientAppliesQ.TotalPayApplies IS NULL THEN 0 ELSE PatientAppliesQ.TotalPayApplies END AS PayTotal, " 
		"CASE WHEN PatientAppliesQ.TotalAdjApplies IS NULL THEN 0 ELSE PatientAppliesQ.TotalAdjApplies END AS AdjTotal, " 
		"(CASE WHEN InsuredPartyT.RespTypeID = 1 THEN ChargeRespT.Amount ELSE 0 END) - (CASE WHEN InsuredPartyT.RespTypeID = 1 THEN COALESCE(PatientAppliesQ.TotalPayApplies, 0) + COALESCE(PatientAppliesQ.TotalAdjApplies,0) ELSE 0 END) AS Ins1Total, " 
		"(CASE WHEN InsuredPartyT.RespTypeID = 2 THEN ChargeRespT.Amount ELSE 0 END) - (CASE WHEN InsuredPartyT.RespTypeID = 2 THEN COALESCE(PatientAppliesQ.TotalPayApplies, 0) + COALESCE(PatientAppliesQ.TotalAdjApplies,0) ELSE 0 END) AS Ins2Total, " 
		"%s "
		"(CASE WHEN (InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 %s) THEN ChargeRespT.Amount ELSE 0 END) - (CASE WHEN (InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 %s) THEN COALESCE(PatientAppliesQ.TotalPayApplies, 0) + COALESCE(PatientAppliesQ.TotalAdjApplies,0) ELSE 0 END) AS InsOtherTotal " 
		"FROM ((" 
		"(SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, TotalDiscount, TotalPercentOff " 
		"FROM LineItemT " 
		"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID " 
		"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID " 
		"LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID " 
		"WHERE LineItemT.PatientID = {INT} " 
		"	AND LineItemT.Deleted = 0 " 
		"	AND LineItemT.Type = 10 " 
		"	AND billid = BillID " 
		") AS PatientChargesQ " 
		"INNER JOIN ((ChargesT " 
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID) " 
		"LEFT JOIN (ChargeRespT " 
		"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID) ON ChargesT.ID = ChargeRespT.ChargeID) ON PatientChargesQ.ID = ChargesT.ID) " 
		"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number " 
		"LEFT JOIN CPTModifierT AS CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) " 
		"LEFT JOIN " 
		"(SELECT AppliesT.RespID, " 
		"SUM(CASE WHEN PatientPaymentsQ.Type = 1 THEN AppliesT.Amount ELSE 0 END) AS TotalPayApplies, " 
		"SUM(CASE WHEN PatientPaymentsQ.Type = 2 THEN AppliesT.Amount ELSE 0 END) AS TotalAdjApplies, " 
		"PatientPaymentsQ.InsuredPartyID FROM AppliesT " 
		"LEFT JOIN ("
		"	SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID, PaymentsT.ClaimNumber FROM LineItemT " 
		"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID " 
		"	WHERE LineItemT.PatientID = {INT} " 
		"		AND LineItemT.Deleted = 0 " 
		"		AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 " 
		") AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID " 
		"WHERE PatientPaymentsQ.ID IS NOT NULL " 
		"GROUP BY AppliesT.RespID, PatientPaymentsQ.InsuredPartyID " 
		") AS PatientAppliesQ ON ChargeRespT.ID = PatientAppliesQ.RespID " 
		") AS ChargeRespSubQ "
		"GROUP BY BillID " 
		") AS FinTabBillSubQ ON PatientBillsQ.ID = FinTabBillSubQ.BillID " 
		"GROUP BY PatientBillsQ.ID, PatientBillsQ.Date, PatientBillsQ.LocName, " 
		"PatientBillsQ.POSName, PatientBillsQ.POSCode, " 
		"PatientBillsQ.Description, PatientBillsQ.EntryType, PatientBillsQ.PatientID, " 
		"PatientBillsQ.BillTotal, PatientBillsQ.InputDate, " 
		"PatientBillsQ.CreatedBy, PatientBillsQ.FirstChargeDate, " 
		"PatientBillsQ.DiagCode1, PatientBillsQ.DiagCode1WithName, PatientBillsQ.BillDiagCodeList, "
		"PatientBillsQ.ChargeDiagCodeList, PatientBillsQ.LastSentDate, PatientBillsQ.CorrectDesc, "
		"PatientBillsQ.InvoiceID, PatientBillsQ.OnHold, PatientBillsQ.IsVoided, PatientBillsQ.IsCorrected "
		"HAVING PatientBillsQ.EntryType = 1 " 
		"	AND PatientBillsQ.PatientID = {INT} " 
		") AS FinTabBillsQ " 
		"WHERE (FinTabBillsQ.ID = {VT_I4} OR {VT_I4} IS NULL) %s " 
		"ORDER BY "
		"CASE WHEN {BIT} = 1 THEN Date END ASC, "
		"CASE WHEN {BIT} = 0 THEN Date END DESC",
		strDynInsRespClause1, strDynInsRespClause2, strDynInsRespClause3, strDynInsRespClause4, strDynInsRespClause5, strDynInsRespClause6, strDynInsRespClause6,
		strZeroBalanceFilter),
		ClaimSendType::Electronic,
		nPatientID, sqlHideVoidedBills, sqlHideVoidedCharges,
		nPatientID, nPatientID, nPatientID, varBillID, varBillID,
		m_bSortASC, m_bSortASC);
}

// Adds bills to FinancialTabInfoT
// (may add 1 or all depending on [Bill ID])
// (j.jones 2009-12-30 09:14) - PLID 36729 - replaced with functions that append select statements to a batch
void CFinancialDlg::AddExpandedBillsToSqlBatch(CString &strSqlBatch, CNxParamSqlArray &aryParams,_variant_t varBillID, long nPatientID)
{
	// (j.jones 2010-06-18 09:39) - PLID 39150 - We can have a dynamic amount of "extra" insurance resps
	// beyond 1, 2, and Other. Build the clauses now.
	CString strDynInsRespClause1;
	for(int i=0; i<m_aryDynamicRespTypeIDs.GetSize(); i++) {

		long nID = (long)m_aryDynamicRespTypeIDs.GetAt(i);

		CString str;
		str.Format("NULL AS Ins%liResp, ", nID);
		strDynInsRespClause1 += str;
	}

	// (j.jones 2015-02-23 12:59) - PLID 64934 - added ability to hide voided/original items
	CSqlFragment sqlHideVoidedItems = GetHideVoidedItemsFilter(true);

	// (j.gruber 2009-03-17 13:09) - PLID 33360 - changed for discount structure
	// (j.jones 2010-05-26 16:56) - PLID 28184 - added Diag Code fields
	// (j.jones 2010-06-18 09:21) - PLID 39150 - renamed Ins3Resp to InsRespOther
	// (j.gruber 2011-07-01 09:50) - PLID 44848 - show 'corrected' in desc
	//(r.wilson 10/2/2012) plid 52970 - Replace hardcoded SendType with enumerated value
	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
	// (d.singleton 2014-03-04 16:01) - PLID 61172 - update to use the new diag code structure,  also include related icd10 codes
	// (j.jones 2014-03-07 10:17) - PLID 60860 - updated BillDiagCodeList for ICD-9/10 pairings
	// (a.wilson 2014-07-24 12:55) - PLID 63015 - added onhold status
	// (j.jones 2015-02-20 13:22) - PLID 64935 - added IsVoided and IsCorrected, it is possible for both to be true
	// if the corrected item was re-voided
	// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field, always blank on bills
	AddParamStatementToSqlBatch(strSqlBatch, aryParams,
		FormatString("SELECT " 
		"'Bill' AS LineType, " 
		"BillsT.ID, " 
		"NULL AS Expr2, " 
		"'BITMAP:UPARROW' AS ExpandChar, " 
		"BillsT.Date, " 
		"LocationsT.Name AS LocName, " 
		"PlaceOfServiceLocT.Name AS POSName, "
		"Min(PlaceOfServiceCodesT.PlaceCodes) AS POSCode, "
		"'BITMAP:FILE' AS NoteIcon, " 
		"CONVERT(INT,(CASE WHEN BillsT.ID IN (SELECT BillID FROM Notes WHERE Notes.BillID IS NOT NULL) THEN 1 ELSE 0 END)) AS HasNotes, " 
		"(CASE WHEN CorrectedBillsQ.ID IS NOT NULL THEN 'Corrected ' ELSE '' END) + 'BILL - ' + IsNull(BillsT.Description, '(No description)') AS Description, " 
		"NULL AS ChargeAmount, " 
		"NULL AS PayAmount, " 
		"NULL AS Adjustment, " 
		"NULL AS Balance, " 
		"NULL AS PatResp, " 
		"NULL AS Ins1Resp, " 
		"NULL AS Ins2Resp, " 
		"%s "
		"NULL AS InsRespOther, " 
		"NULL AS Refund, " 
		"NULL AS DiscountTotal, " 
		"0 AS HasPercentOff, " 
		"dbo.GetBillProviderList(BillsT.ID) AS Provider, " 
		"BillsT.InputDate, " 
		"UsersT.UserName AS CreatedBy, " 
		"Min(LineItemT.Date) AS FirstChargeDate, "
		"CASE WHEN Diag9Code1.ID Is Not Null AND Diag10Code1.ID Is Not Null THEN Diag10Code1.CodeNumber + ' (' + Diag9Code1.CodeNumber + ')' "
		"ELSE Coalesce(Diag10Code1.CodeNumber, Diag9Code1.CodeNumber) END AS DiagCode1, "
		"CASE WHEN Diag9Code1.ID Is Not Null AND Diag10Code1.ID IS Not Null THEN Diag10Code1.CodeNumber + ' - ' + Diag10Code1.CodeDesc + ' (' + Diag9Code1.CodeNumber + ' - ' + Diag9Code1.CodeDesc + ')' "
		"ELSE Coalesce(Diag10Code1.CodeNumber + ' - ' + Diag10Code1.CodeDesc, Diag9Code1.CodeNumber + ' - ' + Diag9Code1.CodeDesc) END AS DiagCode1WithName, "
		""
		"STUFF((SELECT ', ' + "
		"	CASE WHEN DiagCodes9.ID Is Not Null AND DiagCodes10.ID Is Not Null THEN DiagCodes10.CodeNumber + ' (' + DiagCodes9.CodeNumber + ')' "
		"	ELSE Coalesce(DiagCodes10.CodeNumber, DiagCodes9.CodeNumber) END "
		"	FROM BillDiagCodeT "
		"	LEFT JOIN DiagCodes DiagCodes9 ON DiagCodes9.ID = BillDiagCodeT.ICD9DiagID "
		"	LEFT JOIN DiagCodes DiagCodes10 ON DiagCodes10.ID = BillDiagCodeT.ICD10DiagID "
		"	WHERE BillDiagCodeT.BillID = BillsT.ID "
		"	ORDER BY BillDiagCodeT.OrderIndex "
		"	FOR XML PATH('') "
		"), 1, 2, '') AS BillDiagCodeList, "
		""
		"NULL AS ChargeDiagCodeList, "
		"ClaimHistoryQ.LastDate AS LastSentDate, "		
		"BillInvoiceNumbersT.InvoiceID, "
		"CASE WHEN BillStatusT.Type = 1 THEN 1 ELSE 0 END AS OnHold, "
		"(CASE WHEN OriginalBillsQ.ID Is Not Null THEN 1 ELSE 0 END) AS IsVoided, "
		"(CASE WHEN CorrectedBillsQ.ID Is Not Null THEN 1 ELSE 0 END) AS IsCorrected, "
		"NULL AS TotalPayAmount "

		"\r\n\r\n"

		"FROM BillsT " 
		"INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID " 
		"INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID " 
		"LEFT JOIN BillInvoiceNumbersT ON BillsT.ID = BillInvoiceNumbersT.BillID "
		"LEFT JOIN ("
		"	SELECT Max(Date) AS LastDate, ClaimHistoryT.BillID "
		"	FROM ClaimHistoryT "
		//(r.wilson 10/2/2012) plid 52970 - Line below used to be "WHERE SendType >= 0 "
		"	WHERE SendType >= {INT} "
		"	GROUP BY ClaimHistoryT.BillID "
		") AS ClaimHistoryQ ON BillsT.ID = ClaimHistoryQ.BillID "
		"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID " 
		"LEFT JOIN LocationsT PlaceOfServiceLocT ON BillsT.Location = PlaceOfServiceLocT.ID " 
		"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID " 
		"LEFT JOIN UsersT ON BillsT.InputName = UsersT.PersonID " 
		"LEFT JOIN BillDiagCodeFlat4V ON BillsT.ID = BillDiagCodeFlat4V.BillID "
		"LEFT JOIN DiagCodes Diag9Code1 ON BillDiagCodeFlat4V.ICD9Diag1ID = Diag9Code1.ID "
		"LEFT JOIN DiagCodes Diag10Code1 ON BillDiagCodeFlat4V.ICD10Diag1ID = Diag10Code1.ID "
		"LEFT JOIN BillCorrectionsT AS OriginalBillsQ ON BillsT.ID = OriginalBillsQ.OriginalBillID "
		"LEFT JOIN BillCorrectionsT AS CorrectedBillsQ ON BillsT.ID = CorrectedBillsQ.NewBillID "
		"LEFT JOIN BillStatusT ON BillsT.StatusID = BillStatusT.ID "
		"WHERE BillsT.PatientID = {INT} " 
		"AND BillsT.Deleted = 0 " 
		"AND (BillsT.ID = {VT_I4} OR {VT_I4} IS NULL) " 
		"{SQL} "
		"GROUP BY BillsT.ID, BillsT.Description, BillsT.Date, LocationsT.Name, " 
		"PlaceOfServiceLocT.Name, BillsT.InputDate, " 
		"UsersT.UserName, Diag9Code1.ID, Diag9Code1.CodeNumber, Diag9Code1.CodeDesc, " 
		"Diag10Code1.ID, Diag10Code1.CodeNumber, Diag10Code1.CodeDesc, " 
		"ClaimHistoryQ.LastDate, OriginalBillsQ.ID, CorrectedBillsQ.ID, "
		"BillInvoiceNumbersT.InvoiceID, BillStatusT.Type "
		"ORDER BY "
		"CASE WHEN {BIT} = 1 THEN BillsT.Date END ASC, "
		"CASE WHEN {BIT} = 0 THEN BillsT.Date END DESC",
		strDynInsRespClause1),
		ClaimSendType::Electronic,nPatientID,
		varBillID, varBillID,
		sqlHideVoidedItems,
		m_bSortASC, m_bSortASC);
}

// Adds charges to FinancialTabInfoT
// (j.jones 2009-12-30 09:14) - PLID 36729 - replaced with a function that appends a select statement to a batch
void CFinancialDlg::AddChargesToSqlBatch(CString &strSqlBatch, CNxParamSqlArray &aryParams, _variant_t varBillID, _variant_t varChargeID, long nPatientID, BOOL bExpanded)
{
	// (j.jones 2010-06-18 09:39) - PLID 39150 - We can have a dynamic amount of "extra" insurance resps
	// beyond 1, 2, and Other. Build the clauses now.
	CString strDynInsRespClause1;
	CString strDynInsRespClause2;
	for(int i=0; i<m_aryDynamicRespTypeIDs.GetSize(); i++) {

		long nID = (long)m_aryDynamicRespTypeIDs.GetAt(i);

		CString str;
		str.Format("Sum(CASE WHEN InsuredPartyT.RespTypeID = %li THEN ChargeRespT.Amount ELSE 0 END) - SUM(CASE WHEN InsuredPartyT.RespTypeID = %li THEN COALESCE(PatientChargeAppliesQ.Amount,0) ELSE 0 END) AS Ins%liResp, ", nID, nID, nID);
		strDynInsRespClause1 += str;

		str.Format(" AND InsuredPartyT.RespTypeID <> %li ", nID);
		strDynInsRespClause2 += str;
	}

	// (b.spivey, February 22, 2013) - PLID 32768 - Get the preference. 
	bool bShowMods = (GetRemotePropertyInt("DisplayModifiersInBillingTabDescription", TRUE, 0, GetCurrentUserName(), true) ? true : false);

	// (j.jones 2015-02-23 12:59) - PLID 64934 - added ability to hide voided/original items
	CSqlFragment sqlHideVoidedItems = GetHideVoidedItemsFilter(false);

	// (j.jones 2010-06-14 08:53) - PLID 39138 - added per-charge diag codes	
	// (j.jones 2011-08-30 10:53) - PLID 44904 - Some of the insurance resp. calculations here used Max(),
	// when they should have used Sum(), which was never a problem until we allowed negative resps. on
	// Voided charges. Similarly, PatientAppliesQ should have been summarizing apply totals by resp. ID.
	// (b.spivey, February 22, 2013) - PLID 32768 - Keep this in a variable since it's used repeatedly. 
	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
	// (j.jones 2014-03-07 10:17) - PLID 60860 - updated ChargeDiagCodeList for ICD-9/10 pairings
	// (d.singleton 2014-10-14 15:52) - PLID 62698 - added paymentst.claimnumber
	// (j.jones 2015-02-20 13:22) - PLID 64935 - added IsVoided and IsCorrected, it is possible for both to be true
	// if the corrected item was re-voided
	// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field, always blank on charges
	// (d.lange 2015-11-30 10:26) - PLID 67128 - Loads the Fee Sched. allowable based on ChargesT.AllowableInsuredPartyID
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, FormatString(" "
		"DECLARE @ShowModifiers BIT "
		"SET @ShowModifiers = {BIT} " 
		"	" 
		"SELECT \r\n " 
		"'Charge' AS Expr1, \r\n " 
		"ChargesT.BillID, \r\n " 
		"PatientChargesQ.ID, \r\n " 
		"PatientChargesQ.Date, \r\n " 
		"FinTabAddChargesSubQ.LocName, \r\n " 
		"FinTabAddChargesSubQ.POSName, \r\n "
		"FinTabAddChargesSubQ.POSCode, \r\n "
		"'BITMAP:FILE' AS NoteIcon, \r\n " 
		"CONVERT(INT,(CASE WHEN PatientChargesQ.ID IN (SELECT LineItemID FROM Notes WHERE Notes.LineItemID IS NOT NULL) THEN 1 ELSE 0 END)) AS HasNotes, \r\n " 
		// (j.gruber 2011-06-02 09:50) - PLID 44848
		// (b.spivey, February 21, 2013) - PLID 32768 - Uses new subquery
		// (j.jones 2015-03-06 13:16) - PLID 65041 - we no longer add correction prefixes to charges
		" '       Charge - ('  + ItemModCodeQ.ComboCode + ') ' + Description AS Expr12,  \r\n "
		"FinTabAddChargesSubQ.Amount AS Amount, \r\n " 
		"FinTabAddChargesSubQ.Amount - SUM(COALESCE(PatientChargeAppliesQ.Amount,0)) AS Expr3, \r\n " 
		"'BITMAP:DETAILLINE' AS Expr2, \r\n " 
		"CASE WHEN {BIT} = 0 THEN SUM(COALESCE(PatientChargeAppliesQ.TotalPayApplies,0)) ELSE NULL END AS Expr4, \r\n " 
		"CASE WHEN {BIT} = 0 THEN SUM(COALESCE(PatientChargeAppliesQ.TotalAdjApplies,0)) ELSE NULL END AS Expr5, \r\n " 
		"CASE WHEN COUNT(PatientChargeAppliesQ.RespID) > 0 THEN CASE WHEN {BIT} = 1 THEN 'BITMAP:UPARROW' ELSE 'BITMAP:DOWNARROW' END ELSE NULL END AS Expr6, \r\n " 
		"COALESCE(FinTabAddChargesSubQ.Expr7,0) AS Expr7, \r\n " 
		"Sum(CASE WHEN InsuredPartyT.RespTypeID = 1 THEN ChargeRespT.Amount ELSE 0 END) - SUM(CASE WHEN InsuredPartyT.RespTypeID = 1 THEN COALESCE(PatientChargeAppliesQ.Amount,0) ELSE 0 END) AS Expr8, \r\n " 
		"Sum(CASE WHEN InsuredPartyT.RespTypeID = 2 THEN ChargeRespT.Amount ELSE 0 END) - SUM(CASE WHEN InsuredPartyT.RespTypeID = 2 THEN COALESCE(PatientChargeAppliesQ.Amount,0) ELSE 0 END) AS Expr9, \r\n " 
		"%s \r\n "
		"SUM(CASE WHEN (InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 %s) THEN ChargeRespT.Amount ELSE 0 END) - SUM(CASE WHEN (InsuredPartyT.RespTypeID <> 1 AND InsuredPartyT.RespTypeID <> 2 %s) THEN COALESCE(PatientChargeAppliesQ.Amount,0) ELSE 0 END) AS Expr10, \r\n " 
		"CONVERT(money,0) AS Expr11, \r\n " 
		"TotalDiscount, \r\n " 
		"CASE WHEN TotalPercentOff > 0 THEN 1 ELSE 0 END AS HasPercentOff, \r\n " 
		"FinTabAddChargesSubQ.Provider, \r\n " 
		"FinTabAddChargesSubQ.InputDate, \r\n " 
		"FinTabAddChargesSubQ.CreatedBy, \r\n " 
		"NULL AS FirstChargeDate, \r\n " 
		"NULL AS DiagCode1, \r\n "
		"NULL AS DiagCode1WithName, \r\n "
		"NULL AS BillDiagCodeList, \r\n "
		""
		"STUFF((SELECT ', ' + "
		"	CASE WHEN DiagCodes9.ID Is Not Null AND DiagCodes10.ID Is Not Null THEN DiagCodes10.CodeNumber + ' (' + DiagCodes9.CodeNumber + ')' "
		"	ELSE Coalesce(DiagCodes10.CodeNumber, DiagCodes9.CodeNumber) END "
		"	FROM BillDiagCodeT "
		"	INNER JOIN ChargeWhichCodesT ON BillDiagCodeT.ID = ChargeWhichCodesT.BillDiagCodeID "
		"	LEFT JOIN DiagCodes DiagCodes9 ON DiagCodes9.ID = BillDiagCodeT.ICD9DiagID "
		"	LEFT JOIN DiagCodes DiagCodes10 ON DiagCodes10.ID = BillDiagCodeT.ICD10DiagID "
		"	WHERE ChargeWhichCodesT.ChargeID = PatientChargesQ.ID "
		"	ORDER BY BillDiagCodeT.OrderIndex "
		"	FOR XML PATH('') "
		"), 1, 2, '') AS ChargeDiagCodeList, \r\n "
		// (j.jones 2010-09-01 16:27) - PLID 40331 - Added charge allowable, it shows the primary insurance allowable for service codes.
		// Blank if not a service code, blank if no primary company exists, and blank if no allowable is configured for that company.
		// Like other places in the system, it is multiplied by the quantity and modifiers.
		// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable
		// (j.jones 2013-04-12 15:28) - PLID 56250 - allowables are allowed on products too, so I just removed the IsCPTCode filter
		"CASE WHEN AllowableInsuredQ.InsuranceCoID Is Null THEN NULL \r\n "
		"	ELSE Round(Convert(money, \r\n "
		"dbo.GetChargeAllowableForInsuranceCo(PatientChargesQ.ID, AllowableInsuredQ.InsuranceCoID) \r\n "
		"* ChargesT.Quantity * \r\n "
		"(CASE WHEN(ChargesT.CPTMultiplier1 Is Null) THEN 1 ELSE ChargesT.CPTMultiplier1 END) * \r\n "
		"(CASE WHEN ChargesT.CPTMultiplier2 Is Null THEN 1 ELSE ChargesT.CPTMultiplier2 END) * \r\n "
		"(CASE WHEN(ChargesT.CPTMultiplier3 Is Null) THEN 1 ELSE ChargesT.CPTMultiplier3 END) * \r\n "
		"(CASE WHEN ChargesT.CPTMultiplier4 Is Null THEN 1 ELSE ChargesT.CPTMultiplier4 END) \r\n "
		"), 2) \r\n "
		"END AS FeeSchedAllowable, \r\n "
		// (j.jones 2011-04-27 11:19) - PLID 43449 - added ChargeAllowable, which will load the saved
		// charge allowable for the primary insurance company if exists, including if it happens to exist on a product
		// (j.luckoski 2013-05-08 15:20) - PLID 56471 - Removed the multiplication of allowable by quantity as the chargeallowable is already * quantity.
		"CASE WHEN PrimaryInsuredQ.InsuranceCoID Is Null OR ChargeAllowablesT.Allowable Is Null THEN NULL \r\n "
		"	ELSE Round(Convert(money, ChargeAllowablesT.Allowable *  \r\n "
		"		(CASE WHEN(ChargesT.CPTMultiplier1 Is Null) THEN 1 ELSE ChargesT.CPTMultiplier1 END) * \r\n "
		"		(CASE WHEN ChargesT.CPTMultiplier2 Is Null THEN 1 ELSE ChargesT.CPTMultiplier2 END) * \r\n "
		"		(CASE WHEN(ChargesT.CPTMultiplier3 Is Null) THEN 1 ELSE ChargesT.CPTMultiplier3 END) * \r\n "
		"		(CASE WHEN ChargesT.CPTMultiplier4 Is Null THEN 1 ELSE ChargesT.CPTMultiplier4 END) \r\n "
		"		), 2) \r\n "
		"END AS ChargeAllowable, \r\n "
		// (j.jones 2011-12-20 11:24) - PLID 47119 - added ChargeDeductible and ChargeCoinsurance,
		// both loaded for the primary insurance company, if it exists
		"CASE WHEN PrimaryInsuredQ.InsuranceCoID Is Null OR ChargeCoinsuranceT.Deductible Is Null THEN NULL \r\n "
		"	ELSE ChargeCoinsuranceT.Deductible END AS ChargeDeductible, \r\n "
		"CASE WHEN PrimaryInsuredQ.InsuranceCoID Is Null OR ChargeCoinsuranceT.Coinsurance Is Null THEN NULL \r\n "
		"	ELSE ChargeCoinsuranceT.Coinsurance END AS ChargeCoinsurance, \r\n "
		"BillInvoiceNumbersT.InvoiceID, \r\n"
		"(CASE WHEN (IsOriginalCharge = 1 OR IsVoidingCharge = 1) THEN 1 ELSE 0 END) AS IsVoided, \r\n"
		"IsCorrectingCharge AS IsCorrected, \r\n"
		"NULL AS TotalPayAmount \r\n"

		"\r\n\r\n"

		"FROM \r\n "
		"(SELECT ChargesT.BillID, PatientChargesQ.ID, ChargesT.Quantity, PatientChargesQ.Date, \r\n " 
		"ChargesT.CPTMultiplier1, ChargesT.CPTMultiplier2, ChargesT.CPTMultiplier3, ChargesT.CPTMultiplier4, \r\n "
		"COALESCE(dbo.GetChargeTotal(PatientChargesQ.ID),0) AS Amount, \r\n " 
		"Sum(CASE WHEN ChargeRespT.InsuredPartyID IS NULL THEN ChargeRespT.Amount ELSE 0 END) - SUM(CASE WHEN PatientChargeAppliesQ.Amount IS NULL OR PatientChargeAppliesQ.InsuredPartyID > 0 THEN 0 ELSE PatientChargeAppliesQ.Amount END) AS Expr7, \r\n " 
		"CASE WHEN PersonT.ID IS NULL THEN '' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS Provider, \r\n " 
		"PatientChargesQ.LocName, \r\n " 
		"PlaceOfServiceLocT.Name AS POSName, \r\n " 
		"PlaceOfServiceCodesT.PlaceCodes AS POSCode, \r\n " 
		"PatientChargesQ.InputDate, \r\n " 
		"PatientChargesQ.InputName AS CreatedBy, PatientChargesQ.TotalDiscount, PatientChargesQ.TotalPercentOff,  \r\n " 
		// (j.gruber 2011-06-02 09:51) - PLID 44848
		"PatientChargesQ.IsOriginalCharge, PatientChargesQ.IsVoidingCharge, PatientChargesQ.IsCorrectingCharge \r\n "
	
		"\r\n\r\n"

		"FROM ((\r\n " 
		"(SELECT LineItemT.*, ChargesT.Quantity, ChargesT.BillID, ChargesT.DoctorsProviders, \r\n " 
		"CPTMultiplier1, CPTMultiplier2, CPTMultiplier3, CPTMultiplier4, \r\n "
		"LocationsT.Name AS LocName, TotalDiscountQ.TotalDiscount, TotalPercentageQ.TotalPercentOff, \r\n " 
		// (j.gruber 2011-06-02 09:51) - PLID 44848
		"(CASE WHEN OriginalCorrectionsQ.ID IS NULL THEN 0 ELSE 1 END) AS IsOriginalCharge, \r\n "
		"(CASE WHEN VoidingCorrectionsQ.ID IS NULL THEN 0 ELSE 1 END) AS IsVoidingCharge, \r\n "
		"(CASE WHEN NewCorrectionsQ.ID IS NULL THEN 0 ELSE 1 END) AS IsCorrectingCharge \r\n "
		"FROM LineItemT \r\n " 
		"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID \r\n " 
		"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n " 
		"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID \r\n " 
		// (j.gruber 2011-06-02 09:52) - PLID 44848
		" LEFT JOIN LineItemCorrectionsT AS OriginalCorrectionsQ ON ChargesT.ID = OriginalCorrectionsQ.OriginalLineItemID \r\n \r\n "
		" LEFT JOIN LineItemCorrectionsT AS VoidingCorrectionsQ ON ChargesT.ID = VoidingCorrectionsQ.VoidingLineItemID \r\n \r\n "
		" LEFT JOIN LineItemCorrectionsT AS NewCorrectionsQ ON ChargesT.ID = NewCorrectionsQ.NewLineItemID \r\n \r\n "
		"WHERE LineItemT.PatientID = {INT} \r\n " 
		"	AND LineItemT.Deleted = 0 \r\n " 
		"	AND LineItemT.Type = 10 \r\n " 
		"	{SQL} \r\n"
		") AS PatientChargesQ \r\n " 
		"LEFT JOIN PersonT ON PatientChargesQ.DoctorsProviders = PersonT.ID \r\n " 
		"INNER JOIN (\r\n "
		"	ChargesT \r\n " 
		"	LEFT JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID) ON PatientChargesQ.ID = ChargesT.ID) \r\n " 
		"	LEFT JOIN \r\n " 
		"		(SELECT PatientAppliesQ.Amount, PatientAppliesQ.RespID, InsuredPartyID \r\n "
		"		FROM (SELECT Sum(AppliesT.Amount) AS Amount, AppliesT.RespID, \r\n "
		"			PatientPaymentsQ.InsuredPartyID \r\n "
		"			FROM AppliesT \r\n " 
		"			INNER JOIN (SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID, \r\n " 
		"				PaymentsT.ClaimNumber \r\n"
		"				FROM LineItemT \r\n " 
		"				INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n " 
		"				LEFT JOIN LineItemCorrectionsT AS OriginalCorrectionsQ ON LineItemT.ID = OriginalCorrectionsQ.OriginalLineItemID \r\n"
		"				LEFT JOIN LineItemCorrectionsT AS VoidingCorrectionsQ ON LineItemT.ID = VoidingCorrectionsQ.VoidingLineItemID \r\n"
		"				WHERE LineItemT.PatientID = {INT} \r\n " 
		"				AND LineItemT.Deleted = 0 \r\n " 
		"				AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 \r\n " 
		"				{SQL} \r\n"
		"			) AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID \r\n "
		"			AND AppliesT.PointsToPayments = 0 \r\n "
		"			GROUP BY AppliesT.PointsToPayments, AppliesT.RespID, \r\n "
		"			PatientPaymentsQ.InsuredPartyID \r\n "
		"		) AS PatientAppliesQ \r\n "
		"	) AS PatientChargeAppliesQ ON ChargeRespT.ID = PatientChargeAppliesQ.RespID \r\n "
		") \r\n "
		"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number \r\n "
		"LEFT JOIN CPTModifierT AS CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number \r\n " 
		"LEFT JOIN BillsT ON ChargesT.BillID = BillsT.ID \r\n " 
		"LEFT JOIN LocationsT PlaceOfServiceLocT ON BillsT.Location = PlaceOfServiceLocT.ID \r\n " 
		"LEFT JOIN PlaceOfServiceCodesT ON ChargesT.ServiceLocationID = PlaceOfServiceCodesT.ID \r\n "
		"GROUP BY ChargesT.BillID, PatientChargesQ.ID, PatientChargesQ.Date, \r\n " 
		"ChargesT.Quantity, ChargesT.TaxRate, ChargesT.TaxRate2, ChargesT.CPTMultiplier1, ChargesT.CPTMultiplier2, ChargesT.CPTMultiplier3, ChargesT.CPTMultiplier4, \r\n "
		"PatientChargesQ.Amount, \r\n " 
		"CASE WHEN PersonT.ID IS NULL THEN '' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END, \r\n " 
		"PatientChargesQ.LocName, PlaceOfServiceLocT.Name, \r\n " 
		"PlaceOfServiceCodesT.PlaceCodes, \r\n " 
		"PatientChargesQ.InputDate, PatientChargesQ.TotalDiscount, PatientChargesQ.TotalPercentOff, \r\n " 
		"PatientChargesQ.InputName, \r\n " 
		// (j.gruber 2011-06-02 09:53) - PLID 44848
		"PatientChargesQ.IsOriginalCharge, PatientChargesQ.IsVoidingCharge, PatientChargesQ.IsCorrectingCharge \r\n "
		"HAVING (((ChargesT.BillID) = {VT_I4})) \r\n " 
		"AND ((PatientChargesQ.ID) = {VT_I4} OR ((ChargesT.BillID={VT_I4}) \r\n " 
		"AND ({VT_I4} IS NULL))) \r\n " 
		") AS FinTabAddChargesSubQ \r\n " 
		"INNER JOIN (((\r\n " 
		"(SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, ChargesT.AllowableInsuredPartyID FROM LineItemT \r\n " 
		"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID \r\n " 
		"WHERE LineItemT.PatientID = {INT} \r\n "
		"	AND LineItemT.Deleted = 0 \r\n "
		"	AND LineItemT.Type = 10 \r\n "
		") AS PatientChargesQ \r\n " 
		"INNER JOIN (ChargesT \r\n " 
		"LEFT JOIN (ChargeRespT \r\n " 
		"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID) ON ChargesT.ID = ChargeRespT.ChargeID) ON PatientChargesQ.ID = ChargesT.ID) \r\n " 
		"LEFT JOIN \r\n " 
		"(SELECT DestID, RespID, SUM(Amount) AS Amount, \r\n " 
		"SUM(TotalPayApplies) AS TotalPayApplies, \r\n " 
		"SUM(TotalAdjApplies) AS TotalAdjApplies, \r\n " 
		"InsuredPartyID \r\n " 
		"FROM \r\n " 
		"(SELECT PatientAppliesQ.* \r\n " 
		"FROM \r\n " 
		"(SELECT AppliesT.ID, AppliesT.DestID, AppliesT.RespID, \r\n " 
		"AppliesT.PointsToPayments, AppliesT.Amount, PatientPaymentsQ.Description AS ApplyDesc, \r\n " 
		"PatientPaymentsQ.InsuredPartyID, \r\n " 
		"SUM(CASE WHEN PatientPaymentsQ.Type = 1 THEN AppliesT.Amount ELSE 0 END) AS TotalPayApplies, \r\n " 
		"SUM(CASE WHEN PatientPaymentsQ.Type = 2 THEN AppliesT.Amount ELSE 0 END) AS TotalAdjApplies \r\n " 
		"FROM AppliesT \r\n " 
		"LEFT JOIN \r\n " 
		"(SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID, \r\n " 
		" PaymentsT.ClaimNumber \r\n"
		"FROM LineItemT \r\n " 
		"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n " 
		"WHERE LineItemT.PatientID = {INT} \r\n " 
		"	AND LineItemT.Deleted = 0 \r\n " 
		"	AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 \r\n "
		") AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID \r\n " 
		"WHERE (((PatientPaymentsQ.ID) IS NOT NULL)) \r\n " 
		"GROUP BY AppliesT.ID, DestID, RespID, PointsToPayments, \r\n " 
		"AppliesT.Amount, PatientPaymentsQ.Type, PatientPaymentsQ.Description, \r\n " 
		"PatientPaymentsQ.InsuredPartyID, PatientPaymentsQ.ClaimNumber \r\n " 
		") AS PatientAppliesQ \r\n " 
		"WHERE (((PatientAppliesQ.PointsToPayments) = 0)) \r\n " 
		") AS AppliesSubQ \r\n " 
		"GROUP BY DestID, RespID, InsuredPartyID \r\n " 
		") AS PatientChargeAppliesQ ON ChargeRespT.ID = PatientChargeAppliesQ.RespID) \r\n " 
		"LEFT JOIN CPTModifierT ON ChargesT.CPTModifier = CPTModifierT.Number \r\n " 
		"LEFT JOIN CPTModifierT AS CPTModifierT2 ON ChargesT.CPTModifier2 = CPTModifierT2.Number) ON FinTabAddChargesSubQ.ID = PatientChargesQ.ID \r\n " 
		"LEFT JOIN (SELECT InsuredPartyT.InsuranceCoID, InsuredPartyT.PersonID AS InsuredPartyID, InsuredPartyT.PatientID \r\n "
		"	FROM InsuredPartyT \r\n "
		"	INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID \r\n "
		"	WHERE RespTypeT.Priority = 1) AS PrimaryInsuredQ ON PatientChargesQ.PatientID = PrimaryInsuredQ.PatientID \r\n "
		"LEFT JOIN ChargeAllowablesT ON PatientChargesQ.ID = ChargeAllowablesT.ChargeID AND PatientChargesQ.AllowableInsuredPartyID = ChargeAllowablesT.InsuredPartyID\r\n "
		"LEFT JOIN ChargeCoinsuranceT ON PatientChargesQ.ID = ChargeCoinsuranceT.ChargeID AND PrimaryInsuredQ.InsuredPartyID = ChargeCoinsuranceT.InsuredPartyID \r\n "
		"LEFT JOIN ( \r\n"
		"	SELECT InsuredPartyT.PersonID, InsuranceCoT.PersonID AS InsuranceCoID \r\n"
		"	FROM InsuredPartyT \r\n"
		"	INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID \r\n"
		") AS AllowableInsuredQ ON ChargesT.AllowableInsuredPartyID = AllowableInsuredQ.PersonID \r\n"
		// (b.spivey, February 21, 2013) - PLID 32768 - Outer subquery returns final value.
		"LEFT JOIN "
		"( "
		//Manipulation based on preference. 
		"	SELECT "
		"		ItemModCodeSourceQ.ChargeID, CASE WHEN @ShowModifiers = 0 THEN ItemModCodeSourceQ.ItemCode "
		"							WHEN @ShowModifiers = 1 AND ItemModCodeSourceQ.Mods IS NOT NULL "
		"								THEN ItemModCodeSourceQ.ItemCode + ' - ' + ItemModCodeSourceQ.Mods "
		"							ELSE ItemModCodeSourceQ.ItemCode END AS ComboCode "
		"	FROM "
		"	( "
		//Get values to manipulate here. 
		"		SELECT ID AS ChargeID, ItemCode, "
		"			STUFF(CASE WHEN CPTModifier IS NULL THEN '' WHEN CPTModifier = '' THEN '' ELSE ', ' + CPTModifier  END "
		"			+ CASE WHEN CPTModifier2 IS NULL THEN '' WHEN CPTModifier2 = '' THEN '' ELSE ', ' + CPTModifier2 END "
		"			+ CASE WHEN CPTModifier3 IS NULL THEN '' WHEN CPTModifier3 = '' THEN '' ELSE ', ' + CPTModifier3 END "
		"			+ CASE WHEN CPTModifier4 IS NULL THEN '' WHEN CPTModifier4 = '' THEN '' ELSE ', ' + CPTModifier4 END, 1, 2, '') AS Mods "
		"		FROM ChargesT "
		"	) ItemModCodeSourceQ "
		") ItemModCodeQ ON ChargesT.ID = ItemModCodeQ.ChargeID "
		"LEFT JOIN BillInvoiceNumbersT ON ChargesT.BillID = BillInvoiceNumbersT.BillID "
		"\r\n "
		"GROUP BY ChargesT.BillID, PatientChargesQ.ID, PatientChargesQ.Date, \r\n " 
		"ChargesT.Quantity, ChargesT.CPTMultiplier1, ChargesT.CPTMultiplier2, ChargesT.CPTMultiplier3, ChargesT.CPTMultiplier4, \r\n "
		// (j.gruber 2011-06-02 09:53) - PLID 44848
		// (j.jones 2015-03-06 13:16) - PLID 65041 - we no longer add correction prefixes to charges
		" '       Charge - ('  + ItemModCodeQ.ComboCode + ') ' + Description,  \r\n "
		"(FinTabAddChargesSubQ.Amount), (FinTabAddChargesSubQ.Expr7), \r\n " 
		"dbo.GetChargeTotal(PatientChargesQ.ID), PatientChargesQ.Amount, \r\n " 
		"FinTabAddChargesSubQ.LocName, \r\n " 
		"FinTabAddChargesSubQ.Provider, \r\n " 
		"FinTabAddChargesSubQ.POSName, \r\n " 
		"FinTabAddChargesSubQ.POSCode, \r\n " 
		"FinTabAddChargesSubQ.InputDate, \r\n " 
		"FinTabAddChargesSubQ.CreatedBy, \r\n " 
		"FinTabAddChargesSubQ.TotalPercentOff, \r\n " 
		"FinTabAddChargesSubQ.TotalDiscount, \r\n " 
		"PrimaryInsuredQ.InsuranceCoID, \r\n "
		"ChargeAllowablesT.Allowable, ChargeCoinsuranceT.Deductible, ChargeCoinsuranceT.Coinsurance, "
		// (j.gruber 2011-06-02 09:54) - PLID 44848
		"FinTabAddChargesSubQ.IsOriginalCharge, FinTabAddChargesSubQ.IsVoidingCharge, FinTabAddChargesSubQ.IsCorrectingCharge, \r\n "
		"BillInvoiceNumbersT.InvoiceID, AllowableInsuredQ.InsuranceCoID "
		""
		"HAVING (((ChargesT.BillID) = {VT_I4}) \r\n " 
		"AND ((PatientChargesQ.ID)={VT_I4})) OR (((ChargesT.BillID)={VT_I4}) \r\n " 
		"AND (({VT_I4}) IS NULL)) \r\n "
		"\r\n "
		"ORDER BY \r\n "
		"CASE WHEN {BIT} = 1 THEN PatientChargesQ.Date END ASC, \r\n "
		"CASE WHEN {BIT} = 0 THEN PatientChargesQ.Date END DESC\r\n ",
		strDynInsRespClause1, strDynInsRespClause2, strDynInsRespClause2),
		bShowMods, 
		bExpanded, bExpanded, bExpanded,
		nPatientID, sqlHideVoidedItems,
		nPatientID, sqlHideVoidedItems,
		varBillID, varChargeID, varBillID, varChargeID,
		nPatientID, nPatientID,
		varBillID, varChargeID, varBillID, varChargeID,
		m_bSortASC, m_bSortASC);
}

// Adds the payments below all the bills
// to FinancialTabInfoT
// (j.jones 2009-12-30 10:34) - PLID 36729 - replaced with a function that returns an opened recordset
ADODB::_RecordsetPtr CFinancialDlg::CreateAddUnappliedPaysRecordset(long nPatientID)
{
	// (j.jones 2010-09-23 08:55) - PLID 39307 - added abilities to show the adjustment reason & group codes in the adjustment description
	bool bIncludePaymentCheckNumber = GetRemotePropertyInt("FinancialDlg_IncludePaymentCheckNumber", TRUE, 0, "<None>", true) ? true : false;

	// (j.jones 2010-09-23 08:55) - PLID 39307 - added abilities to show the adjustment reason & group codes in the adjustment description
	// 0 - do not show, 1 - show code only, 2 - show desc. only, 3 - show both code and desc
	long nShowAdjustmentGroupCodeInBillingTabAdjDesc = GetRemotePropertyInt("ShowAdjustmentGroupCodeInBillingTabAdjDesc", 0, 0, "<None>", true);
	long nShowAdjustmentReasonCodeInBillingTabAdjDesc = GetRemotePropertyInt("ShowAdjustmentReasonCodeInBillingTabAdjDesc", 0, 0, "<None>", true);

	CString strGroupCodeFormat = "";
	if(nShowAdjustmentGroupCodeInBillingTabAdjDesc == 1) {
		//code only
		strGroupCodeFormat = " + CASE WHEN AdjustmentGroupCodesT.ID Is Not Null THEN ', Group Code: ' + AdjustmentGroupCodesT.Code ELSE '' END ";
	}
	else if(nShowAdjustmentGroupCodeInBillingTabAdjDesc == 2) {
		//desc. only
		strGroupCodeFormat = " + CASE WHEN AdjustmentGroupCodesT.ID Is Not Null THEN ', Group Code: ' + Convert(nvarchar, AdjustmentGroupCodesT.Description) ELSE '' END ";
	}
	else if(nShowAdjustmentGroupCodeInBillingTabAdjDesc == 3) {
		//both code and desc.
		strGroupCodeFormat = " + CASE WHEN AdjustmentGroupCodesT.ID Is Not Null THEN ', Group Code: ' + AdjustmentGroupCodesT.Code + ' - ' + Convert(nvarchar, AdjustmentGroupCodesT.Description) ELSE '' END ";
	}

	CString strReasonCodeFormat = "";
	if(nShowAdjustmentReasonCodeInBillingTabAdjDesc == 1) {
		//code only
		strReasonCodeFormat = " + CASE WHEN AdjustmentReasonCodesT.ID Is Not Null THEN ', Reason Code: ' + AdjustmentReasonCodesT.Code ELSE '' END ";
	}
	else if(nShowAdjustmentReasonCodeInBillingTabAdjDesc == 2) {
		//desc. only
		strReasonCodeFormat = " + CASE WHEN AdjustmentReasonCodesT.ID Is Not Null THEN ', Reason Code: ' + Convert(nvarchar, AdjustmentReasonCodesT.Description) ELSE '' END ";
	}
	else if(nShowAdjustmentReasonCodeInBillingTabAdjDesc == 3) {
		//both code and desc.
		strReasonCodeFormat = " + CASE WHEN AdjustmentReasonCodesT.ID Is Not Null THEN ', Reason Code: ' + AdjustmentReasonCodesT.Code + ' - ' + Convert(nvarchar, AdjustmentReasonCodesT.Description) ELSE '' END ";
	}

	// (j.jones 2015-02-19 09:33) - PLID 64942 - these payments can now be hidden if they have a zero balance
	CString strZeroBalanceFilter = GetZeroBalanceFilter(false);

	// (j.jones 2015-02-23 12:59) - PLID 64934 - added ability to hide voided/original items
	CSqlFragment sqlHideVoidedItems = GetHideVoidedItemsFilter(false);

	// (j.jones 2011-08-29 16:51) - PLID 44793 - now unapplied line items say their ins. resp. name
	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number, it's null here
	// (d.singleton 2014-10-14 08:13) - PLID 62698 - New preference that will add the e-remit "claim number" to the description you see for the line items in the billing tab.
	// (j.jones 2015-02-20 13:22) - PLID 64935 - added IsVoided and IsCorrected, it is possible for both to be true
	// if the corrected item was re-voided
	// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field, for standalone pay/adj/refs it's
	// no different from the pay/adj/ref column
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), FormatString("SELECT \r\n "
		"FinTabPaySubQ.ID, \r\n " 
		"FinTabPaySubQ.LineType, \r\n " 
		"FinTabPaySubQ.Date, \r\n " 
		"FinTabPaySubQ.LocName, \r\n " 
		"'' AS POSName, \r\n " 
		"'' AS POSCode, \r\n " 
		"'BITMAP:FILE' AS NoteIcon, \r\n " 
		"CONVERT(INT,(CASE WHEN FinTabPaySubQ.ID IN (SELECT LineItemID FROM Notes WHERE Notes.LineItemID IS NOT NULL) THEN 1 ELSE 0 END)) AS HasNotes, \r\n " 
		// (j.gruber 2011-06-02 11:32) - PLID 44848
		// (j.jones 2015-03-06 13:16) - PLID 65041 - we no longer add correction prefixes to pay/adj/refs
		"(CASE WHEN FinTabPaySubQ.InsuredPartyID > 0 THEN (CASE WHEN FinTabPaySubQ.RespTypeName IS NULL THEN '' ELSE '[' + FinTabPaySubQ.RespTypeName + '] ' END) ELSE '' END) + \r\n "
		"CASE WHEN (FinTabPaySubQ.Type = 1 AND PrePayment = 1) THEN 'Pre' ELSE ''END + CASE WHEN FinTabPaySubQ.Type = 1 THEN 'Payment - ' ELSE CASE WHEN FinTabPaySubQ.Type = 2 THEN 'Adjustment - ' ELSE 'Refund - 'END END + Description AS Description, \r\n " 
		"-1 AS ShowChargeAmount, \r\n " 
		"-1 AS ShowBalanceAmount, \r\n " 
		"CASE WHEN FinTabPaySubQ.Type = 1 THEN FinTabPaySubQ.Amount ELSE 0 END AS PayAmount, \r\n " 
		"CASE WHEN FinTabPaySubQ.Type = 2 THEN FinTabPaySubQ.Amount ELSE SUM(CASE WHEN PatientAppliesQ.Type = 2 THEN PatientAppliesQ.Amount ELSE 0 END) END AS Adjustment, \r\n " 
		"CASE WHEN FinTabPaySubQ.Type = 3 THEN FinTabPaySubQ.Amount ELSE SUM(CASE WHEN PatientAppliesQ.Type = 3 THEN PatientAppliesQ.Amount ELSE 0 END) END AS Refund, \r\n " 
		"MIN(-1*FinTabPaySubQ.Amount) + MIN((CASE WHEN [Outgoing] IS NULL THEN 0 ELSE [Outgoing] END)) - SUM(CASE WHEN PatientAppliesQ.Amount IS NULL THEN 0 ELSE PatientAppliesQ.Amount END) AS Balance, \r\n " 
		"CASE WHEN COUNT(PatientAppliesQ.ID) > 0 THEN 'BITMAP:DOWNARROW' ELSE NULL END AS ExpandChar, \r\n " 
		"FinTabPaySubQ.Provider, \r\n " 
		"FinTabPaySubQ.ClaimNumber, \r\n"
		"FinTabPaySubQ.InputDate, \r\n " 
		"FinTabPaySubQ.CreatedBy, \r\n " 
		"NULL AS FirstChargeDate, \r\n " 
		"NULL AS DiagCode1, \r\n "
		"NULL AS DiagCode1WithName, \r\n "
		"NULL AS BillDiagCodeList, \r\n "
		"NULL AS ChargeDiagCodeList, \r\n "
		"NULL AS InvoiceID, \r\n "
		"(CASE WHEN (IsOriginalItem = 1 OR IsVoidingItem = 1 OR IsBalancingPayment = 1) THEN 1 ELSE 0 END) AS IsVoided, \r\n"
		"IsCorrectedItem AS IsCorrected, \r\n"
		"FinTabPaySubQ.Amount AS TotalPayAmount \r\n"

		"\r\n\r\n"

		"FROM (\r\n " 
		"(SELECT PatientPaymentsQ.ID, PatientPaymentsQ.LocName, PatientPaymentsQ.InsuredPartyID, RespTypeT.TypeName AS RespTypeName, \r\n " 
		"CASE WHEN PatientPaymentsQ.Type = 1 THEN 'Payment' ELSE CASE WHEN PatientPaymentsQ.Type = 2 THEN 'Adjustment' ELSE 'Refund' END END AS LineType, \r\n " 
		"PatientPaymentsQ.Date, SUM(PatientAppliesQ.Amount) AS Outgoing, \r\n " 
		"PatientPaymentsQ.Type, PatientPaymentsQ.Description, \r\n " 
		"PatientPaymentsQ.Amount, PatientPaymentsQ.Provider, \r\n " 
		"PatientPaymentsQ.ClaimNumber, \r\n"
		"PatientPaymentsQ.InputDate, \r\n " 
		"PatientPaymentsQ.InputName AS CreatedBy, \r\n " 
		// (j.gruber 2011-06-02 11:33) - PLID 44848 - 
		// (j.jones 2015-03-06 13:16) - PLID 65041 - we no longer add correction prefixes to pay/adj/refs
		"IsOriginalItem, IsVoidingItem, IsCorrectedItem, IsBalancingPayment \r\n"
		
		"\r\n\r\n"

		"FROM \r\n " 
		// (a.walling 2010-06-10 17:53) - PLID 39121
		"(SELECT LineItemT.Date, LineItemT.Amount, LineItemT.ID, LineItemT.Type, LineItemT.InputDate, LineItemT.InputName, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID, \r\n " 
		"PaymentsT.ClaimNumber, LocationsT.Name AS LocName, \r\n " 
		"CASE WHEN PersonT.ID IS NULL THEN '' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS Provider, \r\n " 
		"%s AS Description, \r\n " // (a.walling 2010-06-10 17:53) - PLID 39121
		// (j.gruber 2011-06-02 11:33) - PLID 44848
		" CASE WHEN OriginalCorrectionsQ.ID IS NULL THEN 0 ELSE 1 END as IsOriginalItem,  \r\n "
		" CASE WHEN VoidingCorrectionsQ.ID IS NULL THEN 0 ELSE 1 END as IsVoidingItem,  \r\n "
		" CASE WHEN NewCorrectionsQ.ID IS NULL THEN 0 ELSE 1 END as IsCorrectedItem,  \r\n "
		" CASE WHEN NewCorrectionsQ.ID IS NOT NULL AND OriginalLineItemQ.Type = 1 THEN 1 ELSE 0 END as IsCorrectingPayment,  \r\n "
		" CASE WHEN NewCorrectionsQ.ID IS NOT NULL AND OriginalLineItemQ.Type = 2 THEN 1 ELSE 0 END as IsCorrectingAdjustment,  \r\n "
		" CASE WHEN NewCorrectionsQ.ID IS NOT NULL AND OriginalLineItemQ.Type = 3 THEN 1 ELSE 0 END as IsCorrectingRefund,  \r\n "
		" CASE WHEN LineItemCorrectionsBalancingAdjT.ID IS NULL THEN 0 ELSE 1 END as IsBalancingPayment \r\n "
		"FROM LineItemT \r\n " 
		"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n " 
		"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n " 
		"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID \r\n "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID \r\n "
		"LEFT JOIN PaymentPlansT ON LineItemT.Type = 1 AND PaymentsT.PayMethod = 2 AND PaymentsT.ID = PaymentPlansT.ID \r\n " // (a.walling 2010-06-10 17:53) - PLID 39121
		// (j.gruber 2011-06-02 11:34) - PLID 44848
		" LEFT JOIN LineItemCorrectionsT AS OriginalCorrectionsQ ON PaymentsT.ID = OriginalCorrectionsQ.OriginalLineItemID \r\n \r\n "
		" LEFT JOIN LineItemCorrectionsT AS VoidingCorrectionsQ ON PaymentsT.ID = VoidingCorrectionsQ.VoidingLineItemID \r\n \r\n "
		" LEFT JOIN LineItemCorrectionsT AS NewCorrectionsQ ON PaymentsT.ID = NewCorrectionsQ.NewLineItemID \r\n \r\n "
		" LEFT JOIN LineItemT AS OriginalLineItemQ ON NewCorrectionsQ.OriginalLineItemID = OriginalLineItemQ.ID \r\n "
		" LEFT JOIN LineItemCorrectionsBalancingAdjT ON PaymentsT.ID = LineItemCorrectionsBalancingAdjT.BalancingAdjID \r\n \r\n "
		"LEFT JOIN PersonT ON PaymentsT.ProviderID = PersonT.ID \r\n " 
		"WHERE LineItemT.PatientID = {INT} \r\n " 
		"	AND LineItemT.Deleted = 0 \r\n " 
		"	AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 \r\n " 
		"	{SQL} \r\n"
		") AS PatientPaymentsQ \r\n " 

		"LEFT JOIN (\r\n " 
		"	SELECT AppliesT.*, LineItemT.Type, \r\n " 
		"	LineItemT.Description AS ApplyDesc, PaymentsT.InsuredPartyID \r\n " 
		"	FROM AppliesT \r\n " 
		"	INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID \r\n"
		"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n " 
		"	LEFT JOIN LineItemCorrectionsT AS OriginalCorrectionsQ ON LineItemT.ID = OriginalCorrectionsQ.OriginalLineItemID \r\n"
		"	LEFT JOIN LineItemCorrectionsT AS VoidingCorrectionsQ ON LineItemT.ID = VoidingCorrectionsQ.VoidingLineItemID \r\n"
		"	WHERE LineItemT.PatientID = {INT} \r\n "
		"	AND LineItemT.Deleted = 0 \r\n "
		"	AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 \r\n "
		"	{SQL} \r\n"
		") AS PatientAppliesQ ON PatientPaymentsQ.ID = PatientAppliesQ.SourceID \r\n "
		"LEFT JOIN InsuredPartyT ON PatientPaymentsQ.InsuredPartyID = InsuredPartyT.PersonID \r\n " 
		"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID \r\n "
		"GROUP BY PatientPaymentsQ.ID, PatientPaymentsQ.LocName, PatientPaymentsQ.InsuredPartyID, RespTypeT.TypeName, \r\n " 
		"CASE WHEN PatientPaymentsQ.Type = 1 THEN 'Payment' ELSE CASE WHEN PatientPaymentsQ.Type = 2 THEN 'Adjustment' ELSE 'Refund' END END, \r\n " 
		"PatientPaymentsQ.Date, PatientPaymentsQ.Type, PatientPaymentsQ.Description, \r\n " 
		"PatientPaymentsQ.Amount, PatientPaymentsQ.Provider, \r\n " 
		"PatientPaymentsQ.InputDate, \r\n " 
		"PatientPaymentsQ.InputName, \r\n " 
		// (j.gruber 2011-06-02 11:34) - PLID 44848
		" PatientPaymentsQ.IsVoidingItem, PatientPaymentsQ.IsCorrectingPayment, PatientPaymentsQ.IsCorrectingAdjustment, PatientPaymentsQ.IsCorrectingRefund, PatientPaymentsQ.IsBalancingPayment, \r\n"
		" PatientPaymentsQ.ClaimNumber, PatientPaymentsQ.IsOriginalItem, PatientPaymentsQ.IsCorrectedItem "
		") AS FinTabPaySubQ \r\n " 
		"INNER JOIN PaymentsT ON FinTabPaySubQ.ID = PaymentsT.ID) \r\n " 
		"LEFT JOIN (\r\n "
		"	SELECT AppliesT.*, LineItemT.Type, \r\n "
		"	LineItemT.Description AS ApplyDesc, PaymentsT.InsuredPartyID \r\n "
		"	FROM AppliesT \r\n "
		"	INNER JOIN LineItemT ON AppliesT.SourceID = LineItemT.ID \r\n"
		"	INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n "
		"	LEFT JOIN LineItemCorrectionsT AS OriginalCorrectionsQ ON LineItemT.ID = OriginalCorrectionsQ.OriginalLineItemID \r\n"
		"	LEFT JOIN LineItemCorrectionsT AS VoidingCorrectionsQ ON LineItemT.ID = VoidingCorrectionsQ.VoidingLineItemID \r\n"
		"	WHERE LineItemT.PatientID = {INT} \r\n "
		"	AND LineItemT.Deleted = 0 \r\n "
		"	AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 \r\n "
		"	{SQL} \r\n"
		") AS PatientAppliesQ ON PaymentsT.ID = PatientAppliesQ.DestID \r\n "
		"GROUP BY FinTabPaySubQ.ID, FinTabPaySubQ.LineType, FinTabPaySubQ.Date, \r\n " 
		"FinTabPaySubQ.LocName, \r\n " 
		// (j.gruber 2011-06-02 11:35) - PLID 44848 - add CorrectionDesc
		// (j.jones 2015-03-06 13:16) - PLID 65041 - we no longer add correction prefixes to pay/adj/refs
		"(CASE WHEN FinTabPaySubQ.InsuredPartyID > 0 THEN (CASE WHEN FinTabPaySubQ.RespTypeName IS NULL THEN '' ELSE '[' + FinTabPaySubQ.RespTypeName + '] ' END) ELSE '' END) + \r\n "
		"CASE WHEN (FinTabPaySubQ.Type = 1 AND PrePayment = 1) THEN 'Pre' ELSE ''END + CASE WHEN FinTabPaySubQ.Type = 1 THEN 'Payment - ' ELSE CASE WHEN FinTabPaySubQ.Type = 2 THEN 'Adjustment - ' ELSE 'Refund - 'END END + Description, \r\n "
		"FinTabPaySubQ.Amount, FinTabPaySubQ.Outgoing, \r\n " 
		"FinTabPaySubQ.Type, FinTabPaySubQ.Provider, \r\n " 
		"FinTabPaySubQ.InputDate, FinTabPaySubQ.CreatedBy, \r\n " 
		"FinTabPaySubQ.ClaimNumber, IsOriginalItem, IsVoidingItem, IsCorrectedItem, IsBalancingPayment \r\n"
		"HAVING (Outgoing IS NULL OR COUNT(PatientAppliesQ.Amount) > 0 OR Outgoing != FinTabPaySubQ.Amount) \r\n " 
		"%s \r\n"
		"ORDER BY \r\n "
		"CASE WHEN {BIT} = 1 THEN FinTabPaySubQ.Date END ASC, \r\n "
		"CASE WHEN {BIT} = 0 THEN FinTabPaySubQ.Date END DESC\r\n ", 
		(bIncludePaymentCheckNumber ? "CASE WHEN COALESCE(PaymentPlansT.CheckNo, '') <> '' THEN LineItemT.Description + ' (Check #' + PaymentPlansT.CheckNo + ')' ELSE LineItemT.Description END" : "LineItemT.Description")
		+ strGroupCodeFormat + strReasonCodeFormat,
		strZeroBalanceFilter),
		nPatientID, sqlHideVoidedItems,
		nPatientID, sqlHideVoidedItems, 
		nPatientID, sqlHideVoidedItems,
		m_bSortASC, m_bSortASC);

	return rs;
}

// Adds the payment applies in expanded charges
// to FinancialTabInfoT
// (j.jones 2009-12-30 12:24) - PLID 36729 - replaced with a function that appends a select statement to a batch
void CFinancialDlg::AddAppliedPaysToSqlBatch(CString &strSqlBatch, CNxParamSqlArray &aryParams, long nBillID, long nChargeID, long nPatientID)
{
	bool bIncludePaymentCheckNumber = GetRemotePropertyInt("FinancialDlg_IncludePaymentCheckNumber", TRUE, 0, "<None>", true) ? true : false;

	// (j.jones 2010-09-23 08:55) - PLID 39307 - added abilities to show the adjustment reason & group codes in the adjustment description
	// 0 - do not show, 1 - show code only, 2 - show desc. only, 3 - show both code and desc
	long nShowAdjustmentGroupCodeInBillingTabAdjDesc = GetRemotePropertyInt("ShowAdjustmentGroupCodeInBillingTabAdjDesc", 0, 0, "<None>", true);
	long nShowAdjustmentReasonCodeInBillingTabAdjDesc = GetRemotePropertyInt("ShowAdjustmentReasonCodeInBillingTabAdjDesc", 0, 0, "<None>", true);

	CString strGroupCodeFormat = "";
	if(nShowAdjustmentGroupCodeInBillingTabAdjDesc == 1) {
		//code only
		strGroupCodeFormat = " + CASE WHEN AdjGroupCodeID Is Not Null THEN ', Group Code: ' + AdjGroupCode ELSE '' END ";
	}
	else if(nShowAdjustmentGroupCodeInBillingTabAdjDesc == 2) {
		//desc. only
		strGroupCodeFormat = " + CASE WHEN AdjGroupCodeID Is Not Null THEN ', Group Code: ' + Convert(nvarchar, AdjGroupCodeDesc) ELSE '' END ";
	}
	else if(nShowAdjustmentGroupCodeInBillingTabAdjDesc == 3) {
		//both code and desc.
		strGroupCodeFormat = " + CASE WHEN AdjGroupCodeID Is Not Null THEN ', Group Code: ' + AdjGroupCode + ' - ' + Convert(nvarchar, AdjGroupCodeDesc) ELSE '' END ";
	}

	CString strReasonCodeFormat = "";
	if(nShowAdjustmentReasonCodeInBillingTabAdjDesc == 1) {
		//code only
		strReasonCodeFormat = " + CASE WHEN AdjReasonCodeID Is Not Null THEN ', Reason Code: ' + AdjReasonCode ELSE '' END ";
	}
	else if(nShowAdjustmentReasonCodeInBillingTabAdjDesc == 2) {
		//desc. only
		strReasonCodeFormat = " + CASE WHEN AdjReasonCodeID Is Not Null THEN ', Reason Code: ' + Convert(nvarchar, AdjReasonCodeDesc) ELSE '' END ";
	}
	else if(nShowAdjustmentReasonCodeInBillingTabAdjDesc == 3) {
		//both code and desc.
		strReasonCodeFormat = " + CASE WHEN AdjReasonCodeID Is Not Null THEN ', Reason Code: ' + AdjReasonCode + ' - ' + Convert(nvarchar, AdjReasonCodeDesc) ELSE '' END ";
	}

	// (j.jones 2015-02-23 12:59) - PLID 64934 - added ability to hide voided/original items
	CSqlFragment sqlHideVoidedItems = GetHideVoidedItemsFilter(false);

	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
	// (j.jones 2015-02-20 13:22) - PLID 64935 - added IsVoided and IsCorrected, it is possible for both to be true
	// if the corrected item was re-voided
	// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field, this is the full LineItemT.Amount
	// of the applied item, not the applied amount
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, FormatString("SELECT \r\n " 
		"{INT} AS Expr1, \r\n " 
		"PatientPaymentsQ.LocName, \r\n " 
		"'' AS POSName, \r\n " 
		"'' AS POSCode, \r\n " 
		"'BITMAP:FILE' AS NoteIcon, \r\n " 
		"CONVERT(INT,(CASE WHEN PatientAppliesQ.SourceID IN (SELECT LineItemID FROM Notes WHERE Notes.LineItemID IS NOT NULL) THEN 1 ELSE 0 END)) AS HasNotes, \r\n " 
		"CASE WHEN PatientPaymentsQ.InsuredPartyID IS NULL OR PatientPaymentsQ.InsuredPartyID < 1 THEN 'Applied' ELSE 'InsuranceApplied' END AS Expr2, \r\n " 
		"PatientPaymentsQ.Date, \r\n " 
		// (j.gruber 2011-06-02 11:05) - PLID 44848 - added CorrectionDesc
		// (j.jones 2015-03-06 13:16) - PLID 65041 - we no longer add correction prefixes to pay/adj/refs
		"'               ' + (CASE WHEN PatientAppliesQ.InsuredPartyID > 0 THEN (CASE WHEN RespTypeT.TypeName IS NULL THEN '' ELSE '[' + RespTypeT.TypeName + '] ' END) ELSE '' END) + ((CASE WHEN PatientAppliesQ.Type=1 THEN 'Payment - ' ELSE (CASE WHEN PatientAppliesQ.Type=2 THEN 'Adjustment - ' ELSE 'Refund - ' END) END) + ApplyDesc) AS Expr3, \r\n " 
		"PatientChargesQ.ID AS ChargeID, \r\n " 
		"CASE WHEN PatientAppliesQ.Type <> 2 THEN PatientAppliesQ.Amount ELSE 0 END AS Expr4, \r\n " 
		"CASE WHEN PatientAppliesQ.Type = 2 THEN PatientAppliesQ.Amount ELSE 0 END AS Expr5, \r\n " 
		"PatientAppliesQ.SourceID, \r\n " 
		"PatientAppliesQ.ID, \r\n " 
		"'BITMAP:DETAILLINE' AS Expr6, \r\n " 
		"'BITMAP:DETAILLINE' AS Expr7, \r\n " 
		"PatientPaymentsQ.Provider, \r\n " 
		"PatientPaymentsQ.InputDate, \r\n " 
		"PatientPaymentsQ.InputName AS CreatedBy, \r\n " 
		"PatientPaymentsQ.ClaimNumber, \r\n "
		"NULL AS FirstChargeDate, \r\n " 
		"NULL AS DiagCode1, \r\n "
		"NULL AS DiagCode1WithName, \r\n "
		"NULL AS BillDiagCodeList, \r\n "
		"NULL AS ChargeDiagCodeList, \r\n "
		"PatientChargesQ.InvoiceID, "
		"(CASE WHEN (IsOriginalItem = 1 OR IsVoidingItem = 1 OR IsBalancingCharge = 1 OR IsBalancingPayment = 1) THEN 1 ELSE 0 END) AS IsVoided, \r\n"
		"IsCorrectingItem AS IsCorrected, \r\n"
		"PatientPaymentsQ.Amount AS TotalPayAmount \r\n"

		"\r\n"

		"FROM ((\r\n " 
		"(SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders, BillInvoiceNumbersT.InvoiceID \r\n " 
		"FROM LineItemT \r\n " 
		"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID \r\n " 
		"LEFT JOIN BillInvoiceNumbersT ON ChargesT.BillID = BillInvoiceNumbersT.BillID \r\n"
		"WHERE LineItemT.PatientID = {INT} \r\n "
		"	AND LineItemT.Deleted = 0 \r\n "
		"	AND LineItemT.Type = 10 \r\n "
		") AS PatientChargesQ \r\n " 
		"INNER JOIN (ChargesT \r\n " 
		"LEFT JOIN (ChargeRespT \r\n " 
		"LEFT JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID \r\n " 
		"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID) ON ChargesT.ID = ChargeRespT.ChargeID) ON PatientChargesQ.ID = ChargesT.ID) \r\n " 
		"LEFT JOIN \r\n " 
		// (a.walling 2010-06-10 17:53) - PLID 39121
		"(SELECT AppliesT.*, PatientPaymentsQ.Type, %s AS ApplyDesc, \r\n " 
		"PatientPaymentsQ.InsuredPartyID, \r\n " 
		// (j.gruber 2011-06-02 11:07) - PLID 44848
		// (j.jones 2015-03-06 13:16) - PLID 65041 - we no longer add correction prefixes to pay/adj/refs
		" IsOriginalItem, IsVoidingItem, IsCorrectingItem, IsBalancingCharge, IsBalancingPayment \r\n"

		"\r\n\r\n"

		"FROM AppliesT \r\n " 
		"LEFT JOIN \r\n "  // (a.walling 2010-06-10 17:53) - PLID 39121
		"(SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID, PaymentsT.ClaimNumber, LocationsT.Name AS LocName, \r\n "
		"COALESCE(PaymentPlansT.CheckNo, '') AS CheckNo, \r\n "
		"AdjustmentGroupCodesT.ID AS AdjGroupCodeID, AdjustmentGroupCodesT.Code AS AdjGroupCode, AdjustmentGroupCodesT.Description AS AdjGroupCodeDesc, \r\n "
		"AdjustmentReasonCodesT.ID AS AdjReasonCodeID, AdjustmentReasonCodesT.Code AS AdjReasonCode, AdjustmentReasonCodesT.Description AS AdjReasonCodeDesc, \r\n "
		// (j.gruber 2011-06-02 11:08) - PLID 44848
		" CASE WHEN OriginalCorrectionsQ.ID IS NULL THEN 0 ELSE 1 END as IsOriginalItem, \r\n \r\n "
		" CASE WHEN VoidingCorrectionsQ.ID IS NULL THEN 0 ELSE 1 END as IsVoidingItem, \r\n \r\n "
		" CASE WHEN VoidingPaymentCorrectionsT.ID IS NULL THEN 0 ELSE 1 END as IsVoidingItemAPayment, \r\n \r\n "
		" CASE WHEN BalancingChargeCorrectionsT.BalancingAdjID IS NULL THEN 0 ELSE 1 END as IsBalancingCharge, \r\n \r\n "
		" CASE WHEN NewCorrectionsQ.ID IS NULL THEN 0 ELSE 1 END as IsCorrectingItem, \r\n \r\n "
		" CASE WHEN BalancingPaymentCorrectionsT.BalancingAdjID IS NULL THEN 0 ELSE 1 END as IsBalancingPayment, \r\n \r\n "
		" OriginalLineItemQ.Type as CorrectedType \r\n \r\n "
		"FROM LineItemT \r\n " 
		"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n " 
		"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n " 
		"LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID \r\n "
		"LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID \r\n "
		"LEFT JOIN PaymentPlansT ON LineItemT.Type = 1 AND PaymentsT.PayMethod = 2 AND PaymentsT.ID = PaymentPlansT.ID \r\n " // (a.walling 2010-06-10 17:53) - PLID 39121
		// (j.gruber 2011-06-02 11:08) - PLID 44848
		" LEFT JOIN (SELECT LineItemCorrectionsT.* FROM LineItemCorrectionsT INNER JOIN (SELECT ID FROM LineItemT WHERE DELETED = 0 AND Type = 1) PaymentsT ON LineItemCorrectionsT.OriginalLineItemID = PaymentsT.ID) VoidingPaymentCorrectionsT ON PaymentsT.ID = VoidingPaymentCorrectionsT.VoidingLineItemID \r\n \r\n "
		" LEFT JOIN LineItemCorrectionsT AS OriginalCorrectionsQ ON PaymentsT.ID = OriginalCorrectionsQ.OriginalLineItemID \r\n \r\n "
		" LEFT JOIN LineItemCorrectionsT AS VoidingCorrectionsQ ON PaymentsT.ID = VoidingCorrectionsQ.VoidingLineItemID \r\n \r\n "
		" LEFT JOIN LineItemCorrectionsT AS NewCorrectionsQ ON PaymentsT.ID = NewCorrectionsQ.NewLineItemID \r\n \r\n "
		" LEFT JOIN LineItemT OriginalLineItemQ ON NewCorrectionsQ.OriginalLineItemID = OriginalLineItemQ.ID \r\n \r\n "
		" LEFT JOIN (SELECT LineItemCorrectionsBalancingAdjT.BalancingAdjID FROM LineItemCorrectionsBalancingAdjT INNER JOIN LineItemCorrectionsT ON LineItemCorrectionsBalancingAdjT.LineItemCorrectionID = LineItemCorrectionsT.ID INNER JOIN (SELECT ID FROM LineItemT WHERE DELETED = 0 AND Type = 10) ChargesT ON LineItemCorrectionsT.OriginalLineItemID = ChargesT.ID) BalancingChargeCorrectionsT ON PaymentsT.ID = BalancingChargeCorrectionsT.BalancingAdjID \r\n \r\n "
		" LEFT JOIN (SELECT LineItemCorrectionsBalancingAdjT.BalancingAdjID FROM LineItemCorrectionsBalancingAdjT INNER JOIN LineItemCorrectionsT ON LineItemCorrectionsBalancingAdjT.LineItemCorrectionID = LineItemCorrectionsT.ID INNER JOIN (SELECT ID FROM LineItemT WHERE DELETED = 0 AND Type = 1) PaymentsT ON LineItemCorrectionsT.OriginalLineItemID = PaymentsT.ID) BalancingPaymentCorrectionsT ON PaymentsT.ID = BalancingPaymentCorrectionsT.BalancingAdjID \r\n \r\n "
		"WHERE LineItemT.PatientID = {INT} \r\n " 
		"	AND LineItemT.Deleted = 0 \r\n " 
		"	AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 \r\n " 
		"	{SQL} \r\n"
		") AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID \r\n " 
		"WHERE (((PatientPaymentsQ.ID) IS NOT NULL)) \r\n " 
		") AS PatientAppliesQ ON ChargeRespT.ID = PatientAppliesQ.RespID) \r\n " 
		"LEFT JOIN \r\n " 
		"(SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ClaimNumber, PaymentsT.ID AS PaymentPlansID, \r\n " 
		"LocationsT.Name AS LocName, \r\n " 
		"CASE WHEN PersonT.ID IS NULL THEN '' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS Provider \r\n " 
		"FROM LineItemT \r\n " 
		"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n " 
		"INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n " 
		"LEFT JOIN PersonT ON PaymentsT.ProviderID = PersonT.ID \r\n " 
		"WHERE LineItemT.PatientID = {INT} \r\n "
		"	AND LineItemT.Deleted = 0 \r\n "
		"	AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 \r\n "
		") AS PatientPaymentsQ ON PatientAppliesQ.SourceID = PatientPaymentsQ.ID \r\n " 
		"GROUP BY PatientPaymentsQ.InsuredPartyID, PatientAppliesQ.Amount, PatientPaymentsQ.LocName, PatientPaymentsQ.ClaimNumber, \r\n " 
		"PatientAppliesQ.ApplyDesc, \r\n " 
		"CASE WHEN PatientPaymentsQ.InsuredPartyID IS NULL OR PatientPaymentsQ.InsuredPartyID < 1 THEN 'Applied' ELSE 'InsuranceApplied' END, \r\n " 
		"PatientPaymentsQ.Date, \r\n " 
		// (j.gruber 2011-06-02 11:09) - PLID 44848 - added Correction Desc
		// (j.jones 2015-03-06 13:16) - PLID 65041 - we no longer add correction prefixes to pay/adj/refs
		"'               ' + (CASE WHEN PatientAppliesQ.InsuredPartyID > 0 THEN (CASE WHEN RespTypeT.TypeName IS NULL THEN '' ELSE '[' + RespTypeT.TypeName + '] ' END) ELSE '' END) + ((CASE WHEN PatientAppliesQ.Type=1 THEN 'Payment - ' ELSE (CASE WHEN PatientAppliesQ.Type=2 THEN 'Adjustment - ' ELSE 'Refund - ' END) END) + ApplyDesc), \r\n " 
		"PatientChargesQ.ID, \r\n " 
		"CASE WHEN PatientAppliesQ.Type <> 2 THEN PatientAppliesQ.Amount ELSE 0 END, \r\n " 
		"CASE WHEN PatientAppliesQ.Type = 2 THEN PatientAppliesQ.Amount ELSE 0 END, \r\n " 
		"PatientAppliesQ.SourceID, PatientAppliesQ.ID, InsuredPartyT.RespTypeID, \r\n " 
		"RespTypeT.TypeName, PatientAppliesQ.InsuredPartyID, PatientAppliesQ.Type, \r\n " 
		"PatientPaymentsQ.Provider, PatientPaymentsQ.InputDate, \r\n " 
		"PatientPaymentsQ.InputName, PatientChargesQ.InvoiceID, \r\n"
		"IsOriginalItem, IsVoidingItem, IsCorrectingItem, IsBalancingCharge, IsBalancingPayment, PatientPaymentsQ.Amount \r\n " 
		"HAVING (((PatientChargesQ.ID) = {INT}) AND ((COUNT(PatientAppliesQ.ID)) > 0)) \r\n " 
		"ORDER BY \r\n "
		"CASE WHEN {BIT} = 1 THEN PatientPaymentsQ.Date END ASC, \r\n "
		"CASE WHEN {BIT} = 0 THEN PatientPaymentsQ.Date END DESC\r\n ",
		(bIncludePaymentCheckNumber ? "CASE WHEN PatientPaymentsQ.CheckNo <> '' THEN PatientPaymentsQ.Description + ' (Check #' + PatientPaymentsQ.CheckNo + ')' ELSE PatientPaymentsQ.Description END" : "PatientPaymentsQ.Description")
		+ strGroupCodeFormat + strReasonCodeFormat),
		nBillID, nPatientID,
		nPatientID, sqlHideVoidedItems,
		nPatientID, nChargeID,
		m_bSortASC, m_bSortASC);
}

// Adds payments applied to other payments
// in FinancialTabInfoT (includes insurance)
// (j.jones 2009-12-30 12:33) - PLID 36729 - replaced with a function that returns an opened recordset
ADODB::_RecordsetPtr CFinancialDlg::CreateAddAppliedPayToPayRecordset(long nPaymentID, long nPatientID)
{
	bool bIncludePaymentCheckNumber = GetRemotePropertyInt("FinancialDlg_IncludePaymentCheckNumber", TRUE, 0, "<None>", true) ? true : false;

	// (j.jones 2010-09-23 08:55) - PLID 39307 - added abilities to show the adjustment reason & group codes in the adjustment description
	// 0 - do not show, 1 - show code only, 2 - show desc. only, 3 - show both code and desc
	long nShowAdjustmentGroupCodeInBillingTabAdjDesc = GetRemotePropertyInt("ShowAdjustmentGroupCodeInBillingTabAdjDesc", 0, 0, "<None>", true);
	long nShowAdjustmentReasonCodeInBillingTabAdjDesc = GetRemotePropertyInt("ShowAdjustmentReasonCodeInBillingTabAdjDesc", 0, 0, "<None>", true);

	CString strGroupCodeFormat = "";
	if(nShowAdjustmentGroupCodeInBillingTabAdjDesc == 1) {
		//code only
		strGroupCodeFormat = " + CASE WHEN AdjGroupCodeID Is Not Null THEN ', Group Code: ' + AdjGroupCode ELSE '' END ";
	}
	else if(nShowAdjustmentGroupCodeInBillingTabAdjDesc == 2) {
		//desc. only
		strGroupCodeFormat = " + CASE WHEN AdjGroupCodeID Is Not Null THEN ', Group Code: ' + Convert(nvarchar, AdjGroupCodeDesc) ELSE '' END ";
	}
	else if(nShowAdjustmentGroupCodeInBillingTabAdjDesc == 3) {
		//both code and desc.
		strGroupCodeFormat = " + CASE WHEN AdjGroupCodeID Is Not Null THEN ', Group Code: ' + AdjGroupCode + ' - ' + Convert(nvarchar, AdjGroupCodeDesc) ELSE '' END ";
	}

	CString strReasonCodeFormat = "";
	if(nShowAdjustmentReasonCodeInBillingTabAdjDesc == 1) {
		//code only
		strReasonCodeFormat = " + CASE WHEN AdjReasonCodeID Is Not Null THEN ', Reason Code: ' + AdjReasonCode ELSE '' END ";
	}
	else if(nShowAdjustmentReasonCodeInBillingTabAdjDesc == 2) {
		//desc. only
		strReasonCodeFormat = " + CASE WHEN AdjReasonCodeID Is Not Null THEN ', Reason Code: ' + Convert(nvarchar, AdjReasonCodeDesc) ELSE '' END ";
	}
	else if(nShowAdjustmentReasonCodeInBillingTabAdjDesc == 3) {
		//both code and desc.
		strReasonCodeFormat = " + CASE WHEN AdjReasonCodeID Is Not Null THEN ', Reason Code: ' + AdjReasonCode + ' - ' + Convert(nvarchar, AdjReasonCodeDesc) ELSE '' END ";
	}

	// (j.jones 2015-02-23 12:59) - PLID 64934 - added ability to hide voided/original items
	CSqlFragment sqlHideVoidedItems = GetHideVoidedItemsFilter(false);

	// (j.gruber 2010-08-31 15:09) - PLID 37596 - fixed the note icon for applied payments
	// (j.jones 2011-08-29 16:51) - PLID 44793 - now applied pay-to-pay line items say their ins. resp. name
	// (j.gruber 2011-07-01 15:57) - PLID 44848 - 
	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number, it's null here
	// (d.singleton 2014-10-14 08:13) - PLID 62698 - New preference that will add the e-remit "claim number" to the description you see for the line items in the billing tab.
	// (j.jones 2015-02-20 13:22) - PLID 64935 - added IsVoided and IsCorrected, it is possible for both to be true
	// if the corrected item was re-voided
	// (j.jones 2015-02-27 09:51) - PLID 64943 - added the Total Pay Amount field, this is the full LineItemT.Amount
	// of the applied item, not the applied amount
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), FormatString("SELECT \r\n "
		"PatientAppliesQ.ID AS ApplyID, \r\n "
		"PatientPaymentsQ.ID AS PayID, \r\n " 
		"'BITMAP:DETAILLINE' AS Expr1, \r\n " 
		"PatientAppliesQ.Date, \r\n " 
		"PatientAppliesQ.LocName, \r\n " 
		"'' AS POSName, \r\n " 
		"'' AS POSCode, \r\n " 
		"'BITMAP:FILE' AS NoteIcon, \r\n " 
		// (j.jones 2015-03-06 13:16) - PLID 65041 - we no longer add correction prefixes to pay/adj/refs
		"CONVERT(INT,(CASE WHEN PatientAppliesQ.AppliedPayID IN (SELECT LineItemID FROM Notes WHERE Notes.LineItemID IS NOT NULL) THEN 1 ELSE 0 END)) AS HasNotes, \r\n " 
		"'       ' + (CASE WHEN PatientAppliesQ.InsuredPartyID > 0 THEN (CASE WHEN RespTypeT.TypeName IS NULL THEN '' ELSE '[' + RespTypeT.TypeName + '] ' END) ELSE '' END) +\r\n  "
		"	CASE WHEN PatientAppliesQ.Type = 1 THEN 'Payment - ' ELSE CASE WHEN PatientAppliesQ.Type = 2 THEN 'Adjustment - ' ELSE 'Refund - ' END END + [ApplyDesc] AS Expr2, \r\n " 
		"'AppliedPayToPay' AS Expr3, \r\n " 
		"CASE WHEN PatientAppliesQ.Type = 1 THEN PatientAppliesQ.Amount ELSE NULL END AS Expr4, \r\n " 
		"CASE WHEN PatientAppliesQ.Type = 2 THEN PatientAppliesQ.Amount ELSE NULL END AS Expr5, \r\n " 
		"CASE WHEN PatientAppliesQ.Type = 3 THEN PatientAppliesQ.Amount ELSE NULL END AS Expr6, \r\n " 
		"PatientAppliesQ.Provider, \r\n " 
		"PatientAppliesQ.InputDate, \r\n " 
		"PatientAppliesQ.InputName AS CreatedBy, \r\n " 
		"PatientPaymentsQ.ClaimNumber, \r\n"
		"NULL AS FirstChargeDate, \r\n " 
		"NULL AS DiagCode1,\r\n  "
		"NULL AS DiagCode1WithName,\r\n  "
		"NULL AS BillDiagCodeList,\r\n  "
		"NULL AS ChargeDiagCodeList, \r\n  "
		"NULL AS InvoiceID, "
		"(CASE WHEN (IsOriginalItem = 1 OR IsVoidingItem = 1) THEN 1 ELSE 0 END) AS IsVoided, \r\n"
		"IsCorrectingItem AS IsCorrected, \r\n"
		"PatientAppliesQ.TotalPayAmount \r\n"

		"\r\n\r\n"

		"FROM \r\n " 
		"(SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID, \r\n " 
		" PaymentsT.ClaimNumber \r\n"
		"FROM LineItemT \r\n " 
		"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID\r\n  "
		"WHERE LineItemT.PatientID = {INT} \r\n "
		"	AND LineItemT.Deleted = 0 \r\n "
		"	AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 \r\n "
		") AS PatientPaymentsQ \r\n " 
		"LEFT JOIN \r\n " 
		"	(SELECT AppliesT.*, PatientPaymentsQ.Amount AS TotalPayAmount, PatientPaymentsQ.Date, PatientPaymentsQ.Type, \r\n " 
		"		%s AS ApplyDesc, PatientPaymentsQ.InsuredPartyID, " // (a.walling 2010-06-10 17:53) - PLID 39121
		"		PatientPaymentsQ.LocName, PatientPaymentsQ.Provider, PatientPaymentsQ.ID as AppliedPayID, \r\n " 
		"		PatientPaymentsQ.IsOriginalItem, PatientPaymentsQ.IsVoidingItem, PatientPaymentsQ.IsCorrectingItem \r\n"
		"	FROM AppliesT \r\n " 
		"	LEFT JOIN \r\n " 
		"		(SELECT LineItemT.*, PaymentsT.InsuredPartyID, PaymentsT.ID AS PaymentPlansID, PaymentPlansT.CheckNo, PaymentsT.ClaimNumber," // (a.walling 2010-06-10 17:53) - PLID 391\r\n 21
		"			LocationsT.Name AS LocName, \r\n " 
		"			CASE WHEN PersonT.ID IS NULL THEN '' ELSE PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle END AS Provider, \r\n " 
		"		AdjustmentGroupCodesT.ID AS AdjGroupCodeID, AdjustmentGroupCodesT.Code AS AdjGroupCode, AdjustmentGroupCodesT.Description AS AdjGroupCodeDesc,\r\n  "
		"		AdjustmentReasonCodesT.ID AS AdjReasonCodeID, AdjustmentReasonCodesT.Code AS AdjReasonCode, AdjustmentReasonCodesT.Description AS AdjReasonCodeDesc, \r\n  "
				// (j.jones 2015-03-06 13:16) - PLID 65041 - we no longer add correction prefixes to pay/adj/refs
		"		CASE WHEN OriginalCorrectionsQ.ID IS NULL THEN 0 ELSE 1 END as IsOriginalItem, \r\n \r\n "
		"		CASE WHEN VoidingCorrectionsQ.ID IS NULL THEN 0 ELSE 1 END as IsVoidingItem, \r\n \r\n "
		"		CASE WHEN NewCorrectionsQ.ID IS NULL THEN 0 ELSE 1 END as IsCorrectingItem \r\n \r\n "
		
		"\r\n\r\n"

		"		FROM LineItemT \r\n " 
		"		INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID \r\n " 
		"		INNER JOIN LocationsT ON LineItemT.LocationID = LocationsT.ID \r\n " 
		"		LEFT JOIN AdjustmentCodesT AS AdjustmentGroupCodesT ON PaymentsT.GroupCodeID = AdjustmentGroupCodesT.ID\r\n  "
		"		LEFT JOIN AdjustmentCodesT AS AdjustmentReasonCodesT ON PaymentsT.ReasonCodeID = AdjustmentReasonCodesT.ID\r\n  "
		"		LEFT JOIN PaymentPlansT ON LineItemT.Type = 1 AND PaymentsT.PayMethod = 2 AND PaymentsT.ID = PaymentPlansT.ID " // (a.walling 2010-06-10 17:53) - PLID 391\r\n 21
		"		LEFT JOIN PersonT ON PaymentsT.ProviderID = PersonT.ID \r\n " 
		"		LEFT JOIN (SELECT LineItemCorrectionsT.* FROM LineItemCorrectionsT INNER JOIN (SELECT ID FROM LineItemT WHERE DELETED = 0 AND Type = 1) LinePaymentsT ON LineItemCorrectionsT.OriginalLineItemID = LinePaymentsT.ID) VoidingPaymentCorrectionsT ON PaymentsT.ID = VoidingPaymentCorrectionsT.VoidingLineItemID  \r\n "
		"		LEFT JOIN (SELECT LineItemCorrectionsT.* FROM LineItemCorrectionsT INNER JOIN (SELECT ID FROM LineItemT WHERE DELETED = 0 AND Type = 2) LineAdjsT ON LineItemCorrectionsT.OriginalLineItemID = LineAdjsT.ID) VoidingAdjustmentCorrectionsT ON PaymentsT.ID = VoidingAdjustmentCorrectionsT.VoidingLineItemID  \r\n "
		"		LEFT JOIN (SELECT LineItemCorrectionsT.* FROM LineItemCorrectionsT INNER JOIN (SELECT ID FROM LineItemT WHERE DELETED = 0 AND Type = 3) LineRefsT ON LineItemCorrectionsT.OriginalLineItemID = LineRefsT.ID) VoidingRefundCorrectionsT ON PaymentsT.ID = VoidingRefundCorrectionsT.VoidingLineItemID  \r\n "
		"		LEFT JOIN LineItemCorrectionsT AS OriginalCorrectionsQ ON PaymentsT.ID = OriginalCorrectionsQ.OriginalLineItemID \r\n \r\n "
		"		LEFT JOIN LineItemCorrectionsT AS VoidingCorrectionsQ ON PaymentsT.ID = VoidingCorrectionsQ.VoidingLineItemID \r\n \r\n "
		"		LEFT JOIN LineItemCorrectionsT AS NewCorrectionsQ ON PaymentsT.ID = NewCorrectionsQ.NewLineItemID \r\n \r\n "
		"		LEFT JOIN LineItemT OriginalLineItemQ ON NewCorrectionsQ.OriginalLineItemID = OriginalLineItemQ.ID \r\n "
		//"		LEFT JOIN (SELECT LineItemCorrectionsBalancingAdjT.BalancingAdjID FROM LineItemCorrectionsBalancingAdjT INNER JOIN LineItemCorrectionsT ON LineItemCorrectionsBalancingAdjT.LineItemCorrectionID = LineItemCorrectionsT.ID INNER JOIN (SELECT ID FROM LineItemT WHERE DELETED = 0 AND Type = 2) PaymentsT ON LineItemCorrectionsT.OriginalLineItemID = PaymentsT.ID) BalancingAdjustmentCorrectionsT ON PaymentsT.ID = BalancingAdjustmentCorrectionsT.BalancingAdjID  \r\n "
		"		WHERE LineItemT.PatientID = {INT} \r\n " 
		"			AND LineItemT.Deleted = 0 \r\n " 
		"			AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 \r\n "
		"			{SQL} \r\n"
		"	) AS PatientPaymentsQ ON AppliesT.SourceID = PatientPaymentsQ.ID \r\n " 
		"	WHERE (((PatientPaymentsQ.ID) IS NOT NULL)) \r\n " 
		") AS PatientAppliesQ ON PatientPaymentsQ.ID = PatientAppliesQ.DestID \r\n " 
		"LEFT JOIN InsuredPartyT ON PatientAppliesQ.InsuredPartyID = InsuredPartyT.PersonID \r\n " 
		"LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID\r\n  "
		"WHERE (((PatientPaymentsQ.ID) = {INT}) \r\n " 
		"	AND ((PatientAppliesQ.PointsToPayments) = 1)) \r\n " 
		"ORDER BY\r\n  "
		"CASE WHEN {BIT} = 1 THEN PatientAppliesQ.Date END ASC,\r\n  "
		"CASE WHEN {BIT} = 0 THEN PatientAppliesQ.Date END DESC\r\n ",
		(bIncludePaymentCheckNumber ? "CASE WHEN COALESCE(PatientPaymentsQ.CheckNo, '') <> '' THEN PatientPaymentsQ.Description + ' (Check #' + PatientPaymentsQ.CheckNo + ')' ELSE PatientPaymentsQ.Description END" : "PatientPaymentsQ.Description ")
		+ strGroupCodeFormat + strReasonCodeFormat),
		nPatientID,
		nPatientID, sqlHideVoidedItems,
		nPaymentID,
		m_bSortASC, m_bSortASC);

	return rs;
}

// Calculates the total of all bills and all payments, in two recordsets, one data access
// (j.jones 2009-12-30 13:14) - PLID 36729 - replaced with a function that returns an opened recordset
ADODB::_RecordsetPtr CFinancialDlg::CreatePatientTotalsRecordset(long nPatientID)
{
	CString strBillBalanceFilter = GetBalanceFilter("ChargeRespT");
	CString strPaymentBalanceFilter = GetBalanceFilter("PaymentsT");

	CString strSqlBatch;
	CNxParamSqlArray aryParams;

	if(strBillBalanceFilter == "") {
		//this is much faster but doesn't accomodate our filters,
		//which is good since the filters are rarely used
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "SELECT Convert(money,(CASE WHEN([TotalBills] Is Null) THEN 0 ELSE [TotalBills] END)) AS GrandTotal "
			"FROM (SELECT Sum(dbo.GetBillTotal(BillsT.ID)) AS TotalBills "
			"FROM BillsT WHERE BillsT.PatientID = {INT} AND BillsT.EntryType = 1 AND BillsT.Deleted=0) AS PatientBillTotalQ", nPatientID);
	}
	else {
		//this query is slower but pulls the necessary tables to be able to filter on
		AddParamStatementToSqlBatch(strSqlBatch, aryParams, FormatString("SELECT Convert(money,(CASE WHEN([TotalBills] Is Null) THEN 0 ELSE [TotalBills] END)) AS GrandTotal "
			"FROM (SELECT Sum(CASE WHEN([Type]=10) THEN Round(Convert(money,ChargeRespT.Amount),2) ELSE 0 END) AS TotalBills "
			"FROM BillsT INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"WHERE BillsT.PatientID = {INT} AND BillsT.Deleted=0 AND LineItemT.Deleted=0 %s) AS PatientBillTotalQ", strBillBalanceFilter), nPatientID);
	}

	//payments
	AddParamStatementToSqlBatch(strSqlBatch, aryParams, FormatString("SELECT Type, Sum(Amount) AS SumOfAmount "
		"FROM (SELECT Type, Amount FROM LineItemT INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
		"WHERE LineItemT.PatientID = {INT} AND LineItemT.Type>=1 And LineItemT.Type<=3 AND LineItemT.Deleted=0 %s) AS PatientPaymentsQ "
		"GROUP BY Type", strPaymentBalanceFilter), nPatientID);

	// (j.jones 2015-03-18 09:18) - PLID 64927 - use the snapshot connection
	_RecordsetPtr rs = CreateParamRecordsetBatch(GetRemoteDataSnapshot(), strSqlBatch, aryParams);
	return rs;
}

// (j.jones 2010-06-07 15:18) - PLID 39042 - converted into a datalist 2 version
void CFinancialDlg::OnRButtonDownBillingTabList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);

		RightClickList(pRow, nCol);

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-06-07 15:18) - PLID 39042 - converted into a datalist 2 version
void CFinancialDlg::OnColumnClickingBillingTabList(short nCol, BOOL* bAllowSort)
{
	try {

		*bAllowSort = FALSE;

		if (m_List->GetRowCount() == 0)
			return;

		/////////////////////////////////////////////////////////////////
		// Collapse all trees
		if (nCol == m_nExpandCharColID) {
			CWaitCursor pWait;

			DisableFinancialScreen();

			//first determine if anything is expanded, and collapse if it is
			if (m_aiExpandedBranches.GetSize() != 0 || m_aiExpandedPayBranches.GetSize() != 0) {
				m_aiExpandedSubBranches.RemoveAll();
				m_aiExpandedPayBranches.RemoveAll();
				RedrawAllPayments();
				for (int i=m_aiExpandedBranches.GetSize()-1; i>=0; i--) {
					int BillID = m_aiExpandedBranches.GetAt(i);
					m_aiExpandedBranches.RemoveAt(i);
					RedrawBill(BillID);
				}

				// (j.jones 2015-03-17 13:25) - PLID 64931 - clear the search results window
				ClearSearchResults();
			}else {

				IRowSettingsPtr pRow = m_List->GetFirstRow();
				while(pRow) {

					// Make sure an arrow exists
					_variant_t v = pRow->GetValue(m_nExpandCharColID);
					if (v.vt == VT_EMPTY || v.vt == VT_NULL) {
						pRow = pRow->GetNextRow();
						continue;
					}

					//only expand if it is not a detail line (therefore it is an arrow)
					if (CString(v.bstrVal) != "BITMAP:DOWNARROW" && CString(v.bstrVal) != "BITMAP:UPARROW") {
						pRow = pRow->GetNextRow();
						continue;
					}

					_variant_t vLineID = pRow->GetValue(btcLineID);

					// See if we're expanding a bill or a payment/adjustment/refund
					v = pRow->GetValue(btcLineType);
					if (CString(v.bstrVal) == CString("Bill") || CString(v.bstrVal) == CString("Charge")) {
						// Expand the bill
						v = pRow->GetValue(m_nBillIDColID);						
						ExpandBill(v.lVal, vLineID.lVal);
					}
					else {
						// Expand the payment
						v = pRow->GetValue(btcPayID);
						ExpandPayment(v.lVal);
					}

					//we have to go back to the line ID we were on, then keep advancing
					pRow = m_List->FindByColumn(btcLineID, vLineID, NULL, FALSE);
					if(pRow) {
						pRow = pRow->GetNextRow();
					}
				}
			}

			//JJ - removing this RefreshList call might make things faster, but then things don't display correctly
			RefreshList();

			//set our top row & saved row as the first row
			IRowSettingsPtr pTopRow = m_List->GetFirstRow();
			if(pTopRow) {
				m_SavedRow_LineID = VarLong(pTopRow->GetValue(btcLineID), 0);
				m_SavedTopRow_LineID = VarLong(pTopRow->GetValue(btcLineID), 0);
			}

			EnableFinancialScreen();

			if(m_List->GetRowCount() > 0) {
				m_List->PutTopRow(m_List->GetFirstRow());
				m_List->PutCurSel(m_List->GetFirstRow());
			}
			return;
		}
		else if (nCol == m_nExpandSubCharColID) {

			CWaitCursor pWait;

			//expand/collapse all charges - only for expanded bills

			//TODO - right now this just reverses the expansion of the charges, not
			//expand-all, collapse-all. This could be made more efficient,
			//but this is the cleanest and easiest way of doing it currently.

			DisableFinancialScreen();

			COleVariant var;
			long BillID, ChargeID, LineID;

			IRowSettingsPtr pRow = m_List->GetFirstRow();
			while(pRow) {

				//first check to see if the expand char is valid, otherwise there is no need
				//to get the remaining data
				var = pRow->GetValue(m_nExpandSubCharColID);
				if(var.vt != VT_BSTR) {
					pRow = pRow->GetNextRow();
					continue;
				}
				//only expand if it is not a detail line (therefore it is an arrow)
				if (CString(var.bstrVal) != "BITMAP:DOWNARROW" && CString(var.bstrVal) != "BITMAP:UPARROW") {
					pRow = pRow->GetNextRow();
					continue;
				}

				//now get the IDs. If any are invalid, skip it.
				var = pRow->GetValue(m_nBillIDColID);
				if(var.vt != VT_I4) {
					pRow = pRow->GetNextRow();
					continue;
				}
				BillID = VarLong(var);
				var = pRow->GetValue(btcChargeID);
				if(var.vt != VT_I4) {
					pRow = pRow->GetNextRow();
					continue;
				}
				ChargeID = VarLong(var);
				var = pRow->GetValue(btcLineID);
				if(var.vt != VT_I4) {
					pRow = pRow->GetNextRow();
					continue;
				}
				LineID = VarLong(var);

				//you will not get to this point unless all the data is valid
				ExpandCharge(ChargeID,BillID,LineID);

				//we have to go back to the line ID we were on, then keep advancing
				pRow = m_List->FindByColumn(btcLineID, (long)LineID, NULL, FALSE);
				if(pRow) {
					pRow = pRow->GetNextRow();
				}
			}

			//set our top row & saved row as the first row
			IRowSettingsPtr pTopRow = m_List->GetFirstRow();
			if(pTopRow) {
				m_SavedRow_LineID = VarLong(pTopRow->GetValue(btcLineID), 0);
				m_SavedTopRow_LineID = VarLong(pTopRow->GetValue(btcLineID), 0);
			}

			EnableFinancialScreen();

			if(m_List->GetRowCount() > 0) {
				m_List->PutTopRow(m_List->GetFirstRow());
				m_List->PutCurSel(m_List->GetFirstRow());
			}
			return;
		}
		else if (nCol == m_nDateColID) {
			// (j.jones 2009-12-30 09:52) - PLID 36729 - flip our sort direction
			m_bSortASC = !m_bSortASC;

			CWaitCursor pWait;
		}
		else
			return;

		/////////////////////////////////////////////////////////////////////////
		// Make sure the list won't fall off the face of the earth by scrolling
		// it back to the top
		if(m_List->GetRowCount() > 0) {
			m_List->PutTopRow(m_List->GetFirstRow());
			m_List->PutCurSel(m_List->GetFirstRow());
		}
		FillTabInfo();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-06-07 15:18) - PLID 39042 - converted into a datalist 2 version
void CFinancialDlg::OnDragBeginBillingTabList(BOOL* pbShowDrag, LPDISPATCH lpRow, short nCol, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			*pbShowDrag = FALSE;
			return;
		}

		CString str;
		// Get line type
		_variant_t var = pRow->GetValue(btcLineType);
		if (var.vt == VT_EMPTY || var.vt == VT_NULL) {
			*pbShowDrag = FALSE;
			return;
		}
		//you can only drag pay/adj/ref or responsibilities
		//DRT 2/16/2004 - Added 'Other' column back in
		str = CString(var.bstrVal);

		// (j.jones 2010-06-18 15:24) - PLID 39150 - calculate if this is a dynamic insurance column
		long nDynamicRespTypeID = -1;
		m_mapColumnIndexToRespTypeID.Lookup(nCol, nDynamicRespTypeID);

		// (j.jones 2010-07-07 17:12) - PLID 39441 - We previously never showed the source highlight if
		// we started in the "Other" column. Now we do.

		if(!(str == CString("Payment") || str == CString("PrePayment") ||
			str == CString("Adjustment") || str == CString("Refund") ||
			nCol == m_nPatientBalanceColID || nCol == m_nPrimInsColID || 
			nCol == m_nSecInsColID || nDynamicRespTypeID != -1
			|| nCol == m_nOtherInsuranceColID)) {

				*pbShowDrag = FALSE;
				return;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-06-07 15:18) - PLID 39042 - converted into a datalist 2 version
void CFinancialDlg::OnDragOverCellBillingTabList(BOOL* pbShowDrop, LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		IRowSettingsPtr pFromRow(lpFromRow);

		//if you are switching insurance responsibilities, only highlight valid drops,
		//which would be any responsibility on the same row.
		//DRT 2/16/2004 - Addeed 'Other' back into the mix (changed < to <=)
		if((nFromCol >= m_nPatientBalanceColID && nFromCol <= m_nOtherInsuranceColID) && 
			(!(nCol >= m_nPatientBalanceColID && nCol <= m_nOtherInsuranceColID) || !(pRow == pFromRow))) {
				*pbShowDrop = FALSE;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-06-07 15:18) - PLID 39042 - converted into a datalist 2 version
void CFinancialDlg::OnDragEndBillingTabList(LPDISPATCH lpRow, short nCol, LPDISPATCH lpFromRow, short nFromCol, long nFlags)
{
	try {

		IRowSettingsPtr pToRow(lpRow);
		if(pToRow == NULL) {
			return;
		}

		IRowSettingsPtr pFromRow(lpFromRow);
		if(pFromRow== NULL) {
			return;
		}

		m_List->PutCurSel(pToRow);

		//a lot of this code has been copied to CProcInfoCenterDlg::ApplyPrePaysToBill(),
		//and therefore any changes need to be applied to both places

		_variant_t varFromLineType = pFromRow->GetValue(btcLineType);
		CString strPrevLineType = VarString(varFromLineType,"");
		CString strDestLineType = VarString(pToRow->GetValue(btcLineType));

		if(strPrevLineType == "Blank" || strDestLineType == "Blank") {
			return;
		}

		// (j.jones 2011-09-13 15:37) - PLID 44887 - disallow modifying/applying *from* original/void line items
		if(strPrevLineType == "Bill") {

			if(strDestLineType != "Bill") {
				//can't drag anything on a bill but a responsibility on the same line
				return;
			}

			long nBillID = VarLong(pFromRow->GetValue(m_nBillIDColID));

			//you can't drag a bill to something else, this would only be hit if you tried
			//to shift responsibilities, so "this bill" is accurate
			if(IsVoidedBill(nBillID)) {
				AfxMessageBox("This bill has been corrected, and can no longer be modified.");
				return;
			}
			else if(!DoesBillHaveUncorrectedCharges(nBillID)) {
				AfxMessageBox("All charges in this bill have been corrected, and can no longer be modified.");				
				return;
			}
		}
		else if(strPrevLineType == "Charge") {

			if(strDestLineType != "Charge") {
				//can't drag anything on a charge but a responsibility on the same line
				return;
			}

			long nChargeID = VarLong(pFromRow->GetValue(btcChargeID));

			//you can't drag a charge to something else, this would only be hit if you tried
			//to shift responsibilities, so "this charge" is accurate
			LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nChargeID);
			if(licsStatus == licsOriginal) {
				AfxMessageBox("This charge has been corrected, and can no longer be modified.");
				return;
			}
			else if(licsStatus == licsVoid) {
				AfxMessageBox("This charge is a void charge from an existing correction, and cannot be modified.");				
				return;
			}
		}
		else {
			//must be a payment

			// Make sure previous line was an acceptable payment
			if(strPrevLineType != CString("Payment") && strPrevLineType != CString("Adjustment") &&
				strPrevLineType != CString("Refund") && strPrevLineType != CString("PrePayment")) {
				return;
			}

			_variant_t varFromPayID = pFromRow->GetValue(btcPayID);
			if(varFromPayID.vt == VT_I4) {

				long nFromPayID = VarLong(varFromPayID);

				if(nFromPayID != -1) {
					LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nFromPayID);
					if(licsStatus == licsOriginal) {
						AfxMessageBox("The line item you are dragging has been corrected, and can no longer be modified.");
						return;
					}
					else if(licsStatus == licsVoid) {
						AfxMessageBox("The line item you are dragging is a void line item from an existing correction, and cannot be modified.");
						return;
					}
				}
			}
		}

		// (j.jones 2011-09-13 15:37) - PLID 44887 - disallow applying *to* original/void line items		
		if(strDestLineType == "Bill") {
			long nBillID = VarLong(pToRow->GetValue(m_nBillIDColID));

			if(IsVoidedBill(nBillID)) {
				AfxMessageBox("This bill has been corrected, and can no longer be modified.");
				return;
			}
			else if(!DoesBillHaveUncorrectedCharges(nBillID)) {
				AfxMessageBox("All charges in this bill have been corrected, and can no longer be modified.");
				return;
			}
		}
		else if(strDestLineType == "Charge") {
			long nChargeID = VarLong(pToRow->GetValue(btcChargeID));

			LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nChargeID);
			if(licsStatus == licsOriginal) {
				AfxMessageBox("This charge has been corrected, and can no longer be modified.");
				return;
			}
			else if(licsStatus == licsVoid) {
				AfxMessageBox("This charge is a void charge from an existing correction, and cannot be modified.");
				return;
			}
		}
		else {
			//must be a payment
			long nPayID = -1;
			if(strDestLineType == CString("Applied") || strDestLineType == CString("InsuranceApplied") || strDestLineType == CString("AppliedPayToPay")) {								
				_variant_t varApplyID = pToRow->GetValue(btcApplyID);
				if(varApplyID.vt == VT_I4 && VarLong(varApplyID,-1) != -1) {
					_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT SourceID FROM AppliesT WHERE ID = {INT}", VarLong(varApplyID));
					if(!rs->eof) {
						nPayID = rs->Fields->Item["SourceID"]->Value.lVal;
					}
					rs->Close();
				}
				else {
					_variant_t var = pToRow->GetValue(btcPayID);
					if(var.vt == VT_I4) {
						nPayID = VarLong(pToRow->GetValue(btcPayID));
					}
				}
			}
			else {
				_variant_t var = pToRow->GetValue(btcPayID);
				if(var.vt == VT_I4) {
					nPayID = VarLong(pToRow->GetValue(btcPayID));
				}
			}

			if(nPayID != -1) {
				LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nPayID);
				if(licsStatus == licsOriginal) {
					AfxMessageBox("This line item has been corrected, and can no longer be modified.");
					return;
				}
				else if(licsStatus == licsVoid) {
					AfxMessageBox("This line item is a void line item from an existing correction, and cannot be modified.");
					return;
				}
			}
		}

		_RecordsetPtr rs(__uuidof(Recordset));
		_RecordsetPtr rs1(__uuidof(Recordset));
		CFinancialApply dlg(this);
		COleVariant var, tempVar;
		long InsuredPartyID;

		// Make sure end-of-drag line exists
		var = pToRow->GetValue(btcLineID);
		tempVar = pFromRow->GetValue(btcLineID);
		// (j.jones 2011-08-24 17:40) - PLID 45176 - removed an incorrect permission check for applies,
		// we aren't applying yet
		if(var.vt != VT_I4 || tempVar.vt != VT_I4) {				
			var = tempVar;
			return;
		}

		if (VarLong(var) == VarLong(tempVar)) {

			// (j.jones 2010-06-18 15:30) - PLID 39150 - calculate if either column is dynamic insurance column
			long nDynamicRespTypeID = -1;
			m_mapColumnIndexToRespTypeID.Lookup(nCol, nDynamicRespTypeID);
			long nFromDynamicRespTypeID = -1;
			m_mapColumnIndexToRespTypeID.Lookup(nFromCol, nFromDynamicRespTypeID);

			/////////////////////////////////////////////////////////////////////////
			// Allow user to transfer responsibility by dragging and dropping from the
			// primary or secondary column to the patient responsibility column
			/////////////////////////////////////////////////////////////////////////
			if (nCol == m_nPatientBalanceColID || nCol == m_nPrimInsColID ||
				nCol == m_nSecInsColID || nCol == m_nOtherInsuranceColID
				|| nDynamicRespTypeID != -1) {

					//TES 5/16/03: Don't pop up a dialog if we're dragging from and to the same cell!
					if (nCol == nFromCol)
						return;

					CString strSrc, strDst;
					// (j.jones 2010-06-18 16:05) - PLID 39150 - ensure these are initialized
					long iSrc = -1, iDst = -1;
					long nSrcInsuredPartyID = -1, nDstInsuredPartyID = -1;

					long ID = -1;
					long nBillID = -1;

					//DRT 2/16/2004 - This was down below near the call to ShiftInsBalance()
					// Get the line type
					if (strDestLineType == "Bill") {
						ID = VarLong(pToRow->GetValue(m_nBillIDColID));
						nBillID = ID;
					}
					else if (strDestLineType == "Charge") {
						ID = VarLong(pToRow->GetValue(btcChargeID));
						// (j.jones 2009-12-28 17:55) - PLID 27237 - pulled from the screen, not data
						nBillID = VarLong(pToRow->GetValue(m_nBillIDColID));
					}
					else 
						return;

					if(nFromCol == m_nPatientBalanceColID) {
						strSrc = "Patient balance";
						iSrc = 0;
					}
					else if(nFromCol == m_nPrimInsColID) {
						strSrc = "Primary insurance balance";
						iSrc = 1;
					}
					else if(nFromCol == m_nSecInsColID) {
						strSrc = "Secondary insurance balance";
						iSrc = 2;
					}
					// (j.jones 2010-06-18 15:33) - PLID 39150 - support dynamic insurance columns
					else if(nFromDynamicRespTypeID != -1) {
						//dynamic insurance column
						iSrc = nFromDynamicRespTypeID;
						CString strRespTypeName = "";
						if(!m_mapRespTypeIDToRespName.Lookup(iSrc, strRespTypeName) || strRespTypeName.IsEmpty()) {
							//this really should not be possible
							strRespTypeName = "Other Resp";
						}

						strSrc.Format("%s insurance balance", strRespTypeName);
					}
					else if(nFromCol == m_nOtherInsuranceColID) {
						if(m_bOtherInsExists) {
							//DRT 2/16/2004 - PLID 8814 - If they are dragging from an "Other" column, pop up the 
							//	dialog to choose which resp they are indeed talking about.
							CChooseDragRespDlg dlg(this);
							dlg.m_nTargetType = 1;	//we are looking for a destination
							dlg.m_strLineType = strDestLineType;
							dlg.m_nLineID = ID;

							// (j.jones 2010-06-18 14:32) - PLID 39150 - pass in a comma delimited list of RespTypeIDs to ignore,
							// to be appended to the existing ignored list of IDs 1 and 2
							CString strIDsToIgnore;
							for(int r=0;r<m_aryDynamicRespTypeIDs.GetSize();r++) {
								long nRespTypeID = (long)m_aryDynamicRespTypeIDs.GetAt(r);
								if(!strIDsToIgnore.IsEmpty()) {
									strIDsToIgnore += ",";
								}
								strIDsToIgnore += AsString(nRespTypeID);						
							}
							dlg.m_strRespTypeIDsToIgnore = strIDsToIgnore;

							if(dlg.DoModal() == IDCANCEL)
								return;

							//we have our resp!
							iSrc = dlg.m_nRespTypeID;
							nSrcInsuredPartyID = dlg.m_nInsPartyID;
							strSrc = "other balance";
						}
						else {
							MsgBox("You cannot perform this operation because no Other insurance has been assigned to this patient");
							return;
						}
					}
					else {
						return;
					}

					if(nCol == m_nPatientBalanceColID) {
						strDst = "Patient's responsibility";
						iDst = 0;
					}
					else if(nCol == m_nPrimInsColID) {
						strDst = "Primary insurance's responsibility";
						iDst = 1;
					}
					else if(nCol == m_nSecInsColID) {
						strDst = "Secondary insurance's responsibility";
						iDst = 2;
					}
					// (j.jones 2010-06-18 15:33) - PLID 39150 - support dynamic insurance columns
					else if(nDynamicRespTypeID != -1) {
						//dynamic insurance column
						iDst = nDynamicRespTypeID;
						CString strRespTypeName = "";
						if(!m_mapRespTypeIDToRespName.Lookup(iDst, strRespTypeName) || strRespTypeName.IsEmpty()) {
							//this really should not be possible
							strRespTypeName = "Other Resp";
						}

						strDst.Format("%s insurance balance", strRespTypeName);
					}
					else if(nCol == m_nOtherInsuranceColID) {
						if(m_bOtherInsExists) {
							//DRT 2/16/2004 - PLID 8814 - If they are dragging to an "Other" column, pop up the 
							//	dialog to choose which resp they are indeed talking about.
							CChooseDragRespDlg dlg(this);
							dlg.m_strLineType = strDestLineType;
							dlg.m_nLineID = ID;
							dlg.m_nTargetType = 2;	//we are looking for a source

							// (j.jones 2010-06-18 14:32) - PLID 39150 - pass in a comma delimited list of RespTypeIDs to ignore,
							// to be appended to the existing ignored list of IDs 1 and 2
							CString strIDsToIgnore;
							for(int r=0;r<m_aryDynamicRespTypeIDs.GetSize();r++) {
								long nRespTypeID = (long)m_aryDynamicRespTypeIDs.GetAt(r);
								if(!strIDsToIgnore.IsEmpty()) {
									strIDsToIgnore += ",";
								}
								strIDsToIgnore += AsString(nRespTypeID);				
							}
							dlg.m_strRespTypeIDsToIgnore = strIDsToIgnore;

							if(dlg.DoModal() == IDCANCEL)
								return;

							//we have our resp!
							iDst = dlg.m_nRespTypeID;
							nDstInsuredPartyID = dlg.m_nInsPartyID;
							strSrc = "other balance";
						}
						else {
							MsgBox("You cannot perform this operation because no Other insurance has been assigned to this patient");
							return;
						}
					}
					else {
						return;
					}

					if ((iSrc == 1 || iDst == 1) && m_GuarantorID1 < 1) {
						MsgBox("You cannot perform this operation because no Primary insurance has been assigned to this patient");
						return;
					}
					else if ((iSrc == 2 || iDst == 2) && m_GuarantorID2 < 1) {
						MsgBox("You cannot perform this operation because no Secondary insurance has been assigned to this patient");
						return;
					}
					// (j.jones 2010-06-18 15:34) - PLID 39150 - support dynamic insurance columns
					//only check if it hasn't already been calculated
					else if(iSrc > 2 && nSrcInsuredPartyID == -1) {

						CString strRespTypeName = "";
						if(!m_mapRespTypeIDToRespName.Lookup(iSrc, strRespTypeName) || strRespTypeName.IsEmpty()) {
							//this really should not be possible
							strRespTypeName = "Other";
						}

						long nInsuredPartyID = -1;
						if(!m_mapRespTypeIDsToGuarantorIDs.Lookup(iSrc, nInsuredPartyID) || nInsuredPartyID == -1) {
							MsgBox("You cannot perform this operation because no %s insurance has been assigned to this patient", strRespTypeName);
							return;
						}
					}
					//only check if it hasn't already been calculated
					else if(iDst > 2 && nDstInsuredPartyID == -1) {

						CString strRespTypeName = "";
						if(!m_mapRespTypeIDToRespName.Lookup(iDst, strRespTypeName) || strRespTypeName.IsEmpty()) {
							//this really should not be possible
							strRespTypeName = "Other";
						}

						long nInsuredPartyID = -1;
						if(!m_mapRespTypeIDsToGuarantorIDs.Lookup(iDst, nInsuredPartyID) || nInsuredPartyID == -1) {
							MsgBox("You cannot perform this operation because no %s insurance has been assigned to this patient", strRespTypeName);
							return;
						}
					}

					//if either are -1, we will have already filled in the right nInsuredPartyID from the ChooseDragRespDlg
					if(iSrc != -1) {
						nSrcInsuredPartyID = GetInsuranceIDFromType(GetActivePatientID(), iSrc);
					}
					if(iDst != -1) {
						nDstInsuredPartyID = GetInsuranceIDFromType(GetActivePatientID(), iDst);
					}

					/* JJ - 5/8/2003 - The ShiftInsBalance function calls a dialog now, which they can cancel,
					//so prompting for a message is now redundant. But I'm leaving the code here in case we ever
					//decide to bring it back.

					str.Format("Are you sure you wish to transfer the %s to the %s?", strSrc, strDst);
					if (IDYES == MessageBox(str, NULL, MB_YESNO)) {
					*/

					//DRT 2/16/2004 - I moved the code that gets the ID and line type up above, because I need it for the new
					//	choose drag resp dialog.

					try {
						// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
						ShiftInsBalance(ID, m_iCurPatient, nSrcInsuredPartyID, nDstInsuredPartyID, strDestLineType,
							"by manually dragging responsibility in the billing tab");
					}NxCatchAll("Error in ShiftInsBalance");

					CheckUnbatchClaim(nBillID);

					// Redraw the bill
					BeginWaitCursor();
					RedrawBill(VarLong(pToRow->GetValue(m_nBillIDColID)));
					RefreshList();
					EndWaitCursor();
			}

			var = tempVar;
			//goto end_jump;
			return;
		}

		// Make sure we can apply to a supported line item. The following types are
		// not supported.
		var = pToRow->GetValue(btcLineType);

		// Make sure previous line was an acceptable payment
		if(strPrevLineType != CString("Payment") && strPrevLineType != CString("Adjustment") &&
			strPrevLineType != CString("Refund") && strPrevLineType != CString("PrePayment")) {
			return;
		}

		_variant_t varFromPayID = pFromRow->GetValue(btcPayID);
		long nFromPayID = -1;
		ECanApplyLineItemResult eCanApplyResult = ecalirCannotApply;
		if(varFromPayID.vt == VT_I4) {
			nFromPayID = VarLong(varFromPayID);
			// (j.jones 2011-08-24 17:40) - PLID 45176 - CanApplyLineItem will check the normal apply create permission,
			// but only after it checks to see if the source item is closed
			// (j.jones 2013-07-01 10:45) - PLID 55517 - this function can now potentially correct the payment, if so
			// the payment ID will be changed to reflect the new, corrected line item
			eCanApplyResult = CanApplyLineItem(nFromPayID);
			if(eCanApplyResult == ecalirCannotApply) {
				var = tempVar;
				return;
			}
		}

		//JMJ 8/1/2003 in the case that a refund is being dragged to an applied payment, we DO allow it,
		//but with special functionality handled in this function
		// (j.jones 2011-03-29 15:13) - PLID 43037 - this should NOT support AppliedPayToPay
		if((CString(var.bstrVal) == CString("Applied") || CString(var.bstrVal) == CString("InsuranceApplied")) && strPrevLineType == "Refund") {
			//TES 7/25/2014 - PLID 63049 - Don't allow them to delete chargeback charges
			long nPaymentID = VarLong(pToRow->GetValue(btcPayID));
			if (DoesPayHaveChargeback(CSqlFragment("PaymentsT.ID = {INT}", nPaymentID))) {
				MessageBox("This payment is associated with at least one Chargeback, and cannot be unapplied in order to refund it. "
					"If you wish to remove this payment, right-click on it and select 'Undo Chargeback.'");
				return;
			}

			long nApplyID = VarLong(pToRow->GetValue(btcApplyID), -1);

			bool bForceRefresh = false;

			// (j.jones 2015-03-20 16:30) - PLID 65402 - supported voiding and correcting automatically
			if (nApplyID > 0) {
				long nOldApplyID = nApplyID;
				//This will check the RefundingAPayment_VoidAndCorrect preference to determine
				//if the applied payment should be voided first.
				//This function also calls CanChangeHistoricFinancial on the apply in order
				//to check permissions and optionally suppress the close warning.
				if (!RefundingAppliedPayment_CheckVoidAndCorrect(nPaymentID, nApplyID, true)) {
					//if this returns false, the refund cannot continue either
					//due to permissions, or a decision to cancel voiding
					return;
				}

				//if the apply ID changed, a correction occurred, we need to know to refresh later
				if (nOldApplyID != nApplyID) {
					bForceRefresh = true;
				}
			}
			else {
				//there should always be an ApplyID
				ASSERT(FALSE);
			}

			ApplyRefundToAppliedPayment(nPaymentID, nApplyID, nFromPayID);
			var = tempVar;

			// (j.jones 2013-07-01 16:17) - PLID 55517 - if we corrected the payment, we still
			// need to refresh the list
			// (j.jones 2015-08-25 15:51) - PLID 65402 - if a correction occurred
			// we need to force a refresh despite not making a refund
			if (eCanApplyResult == ecalirCanApply_IDHasChanged || bForceRefresh) {
				QuickRefresh();
				CalculateTotals();
			}
			return;
		}

		// (j.jones 2011-03-29 15:08) - PLID 43037 - added AppliedPayToPay to the ignore list, 
		// where it should have been all along
		// (z.manning 2015-12-11 10:33) - PLID 66476 - Added InsuranceApplied which should have always been here too
		CString strLineType = CString(var.bstrVal);
		if (
			strLineType == "Blank" || strLineType == "Applied" || strLineType == "InsuranceApplied" || strLineType == "AppliedPayToPay" ||
			(strLineType == "Adjustment" && strPrevLineType == "Payment") || 
			(strLineType == "Payment" && strPrevLineType == "Payment") ||
			(strLineType == "Refund" && strPrevLineType == "Payment") ||
			((strLineType == "Bill" || strLineType == "Charge") && strPrevLineType == "Refund")
			)
		{
				MsgBox("Practice does not support applying to this type of line item");

				// Remove green hilight
				var = tempVar;

				// (j.jones 2013-07-01 16:17) - PLID 55517 - if we corrected the payment, we still
				// need to refresh the list
				if(eCanApplyResult == ecalirCanApply_IDHasChanged) {
					QuickRefresh();
					CalculateTotals();
				}
				return;
		}

		else {
			// Apply the payment to the bill, charge, or some type of payment		
			COleCurrency cyCharges, cyPayments, cyAdjustments, cyRefunds, cyInsurance;
			COleCurrency cyAmount, cyOutgoing, cyIncoming;
			CString strClickedType = CString(var.bstrVal);
			// (r.gonet 2015-07-06 18:02) - PLID 65327 - Some operations, if performed, require
			// a quick refresh rather than just a couple redraws.
			bool bQuickRefresh = false;

			BeginWaitCursor();

			dlg.m_PatientID = m_iCurPatient;

			/////////////////////////////////////////////////////////////////////////
			// Get the insurance company of the payment. That will determine whether it
			// goes to primary, secondary, inactive, or non-insurance
			// (j.jones 2009-12-28 17:04) - PLID 27237 - parameterized
			// (j.jones 2013-07-22 11:06) - PLID 57653 - get the payment date
			rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT InsuredPartyT.PersonID AS InsuredPartyID, InsuredPartyT.RespTypeID, LineItemT.Date "
				"FROM LineItemT "
				"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
				"LEFT JOIN InsuredPartyT ON PaymentsT.InsuredPartyID = InsuredPartyT.PersonID "
				"WHERE LineItemT.ID = {INT} "
				"AND LineItemT.Deleted = 0 AND LineItemT.Type >= 1 AND LineItemT.Type <= 3", nFromPayID);
			_variant_t varRespType;
			COleDateTime dtPayment = g_cdtInvalid;
			if(!rs->eof) {
				var = rs->Fields->Item["InsuredPartyID"]->Value;
				varRespType = rs->Fields->Item["RespTypeID"]->Value;
				dtPayment = VarDateTime(rs->Fields->Item["Date"]->Value);
			}
			else {
				var.vt = VT_NULL;
				varRespType.vt = VT_NULL;
			}
			rs->Close();

			dlg.m_nResponsibility = 0;

			// (b.spivey, September 09, 2011) - PLID 45392 - Assume -1 (that's a sentinel value for patient) in case var is null. 
			InsuredPartyID = -1; 

			if (var.vt != VT_NULL && VarLong(var) > 0){ 
				InsuredPartyID = VarLong(var);
				dlg.m_nResponsibility = VarLong(varRespType);
			}

			if (strClickedType == "Bill") {
				// Get the totals for a bill
				var = pToRow->GetValue(m_nBillIDColID);

				if(dlg.m_nResponsibility == 0) {
					//Patient resp
					if (!GetBillTotals(VarLong(var), m_iCurPatient, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
						MsgBox("This bill has been removed from this screen. Practice will now requery this patient's financial information.");
						FullRefresh();
						//goto end_jump;
						return;
					}
					dlg.m_cyNetCharges = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;				
				}
				else if(dlg.m_nResponsibility == -1) {
					//inactive resp
					GetInactiveInsTotals(InsuredPartyID, VarLong(var), -1, m_iCurPatient, cyCharges, cyPayments);
					dlg.m_cyNetCharges = cyCharges - cyPayments;
				}
				else {
					//all other resp
					// (j.jones 2011-07-22 12:19) - PLID 42231 - GetBillInsuranceTotals now takes in an insured party ID, not a RespTypeID
					if (!GetBillInsuranceTotals(VarLong(var), m_iCurPatient, InsuredPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds)) {
						MsgBox("This bill has been removed from this screen. Practice will now requery this patient's financial information.");
						FullRefresh();
						//goto end_jump;
						return;
					}
					// cyCharges is the total insurance responsbility, less applies
					dlg.m_cyNetCharges = cyCharges - cyPayments - cyAdjustments - cyRefunds;
				}

				dlg.m_boShowIncreaseCheck = FALSE;
				dlg.m_boShowAdjCheck = TRUE;
			}
			else if (strClickedType == "Charge") {
				// Get the totals for a charge
				var = pToRow->GetValue(btcChargeID);

				if(dlg.m_nResponsibility == 0) {
					//Patient resp
					if (!GetChargeTotals(VarLong(var), m_iCurPatient, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds, &cyInsurance)) {
						MsgBox("This charge has been removed from this screen. Practice will now requery this patient's financial information.");
						FullRefresh();
						//goto end_jump;
						return;
					}
					dlg.m_cyNetCharges = cyCharges - cyPayments - cyAdjustments - cyRefunds - cyInsurance;
				}
				else if(dlg.m_nResponsibility == -1) {
					//inactive resp
					GetInactiveInsTotals(InsuredPartyID, -1, VarLong(var), m_iCurPatient, cyCharges, cyPayments);
					dlg.m_cyNetCharges = cyCharges - cyPayments;
				}
				else {
					//any other resp
					if (!GetChargeInsuranceTotals(VarLong(var), m_iCurPatient, InsuredPartyID, &cyCharges, &cyPayments, &cyAdjustments, &cyRefunds)) {
						MsgBox("This charge has been removed from this screen. Practice will now requery this patient's financial information.");
						FullRefresh();
						//goto end_jump;
						return;
					}
					// cyCharges is the total insurance responsbility, less applies
					dlg.m_cyNetCharges = cyCharges - cyPayments - cyAdjustments - cyRefunds;
				}

				dlg.m_boShowIncreaseCheck = TRUE;
				dlg.m_boShowAdjCheck = TRUE;
			}
			else {
				// Get the payment, adjustment, or refund total. This
				// is defined as: The original value, less all outgoing
				// applies, plus all applies directed to this payment/
				// adjustment/refund.			

				var = pToRow->GetValue(btcPayID);
				if (!GetPayAdjRefTotals(VarLong(var), m_iCurPatient, cyAmount, cyOutgoing, cyIncoming)) {
					MsgBox("This payment has been removed from this screen. Practice will now requery this patient's financial information.");
					FullRefresh();
					//goto end_jump;
					return;
				}
				cyAmount = cyAmount - cyOutgoing + cyIncoming;

				if (cyAmount < COleCurrency(0,0))
					cyAmount *= -1;

				dlg.m_cyNetCharges = cyAmount;
				dlg.m_nResponsibility = 0;
				dlg.m_boShowIncreaseCheck = FALSE;
				dlg.m_boShowAdjCheck = FALSE;
			}

			/////////////////////////////////////////////////////////////
			// Get totals for the pay/adj/ref we will apply from
			/////////////////////////////////////////////////////////////
			if (!GetPayAdjRefTotals(nFromPayID, m_iCurPatient, cyAmount, cyOutgoing, cyIncoming)) {
				MsgBox("The payment you have chosen to apply from has been removed from this screen. Practice will now requery this patient's financial information.");
				FullRefresh();
				//goto end_jump;
				return;
			}
			dlg.m_cyNetPayment = cyAmount - cyOutgoing + cyIncoming;

			CString payType;//BVB fix this condition
			payType = VarString(pFromRow->GetValue(btcLineType));
			//		if (dlg.m_cyNetPayment < COleCurrency(0,0)) {
			if (payType == "Payment" && dlg.m_cyNetPayment < COleCurrency(0,0)) 
			{	var = pToRow->GetValue(btcLineType);
			if (CString(var.bstrVal) == CString("Bill") || CString(var.bstrVal) == CString("Charge")) 
			{	MsgBox("You may not apply a negative dollar amount to a bill or charge. Please note that the payment total in the 'Balance' column is positive.");
			var = tempVar;

			// (j.jones 2013-07-01 16:17) - PLID 55517 - if we corrected the payment, we still
			// need to refresh the list
			if(eCanApplyResult == ecalirCanApply_IDHasChanged) {
				QuickRefresh();
				CalculateTotals();
			}
			return;
			//goto end_jump;
			}
			}

			// Get ID of payment to apply from
			if(IsZeroDollarPayment(nFromPayID))
				dlg.m_boZeroAmountAllowed = TRUE;

			//stop people from applying to zero balances, or applying payments of wrong responsibility
			if (payType == "Payment" && (dlg.m_cyNetCharges == COleCurrency(0,0) && !dlg.m_boZeroAmountAllowed)) 
			{	var = pToRow->GetValue(btcLineType);
			if (CString(var.bstrVal) == CString("Bill") || CString(var.bstrVal) == CString("Charge")) {
				if(dlg.m_nResponsibility == 0) {
					MsgBox("You may not apply a patient payment to a zero balance. Please note that the 'Patient Resp.' column is zero.");
					var = tempVar;

					// (j.jones 2013-07-01 16:17) - PLID 55517 - if we corrected the payment, we still
					// need to refresh the list
					if(eCanApplyResult == ecalirCanApply_IDHasChanged) {
						QuickRefresh();
						CalculateTotals();
					}
					return;
					//goto end_jump;
				}

				if(strClickedType=="Bill") {
					if(dlg.m_nResponsibility == 1)
						MsgBox("You may not apply a primary insurance payment to a bill with a zero 'Primary Resp.' balance.\n"
						"However, you can increase the primary responsibility if you apply directly to a charge.");
					else if(dlg.m_nResponsibility == 2)
						MsgBox("You may not apply a secondary insurance payment to a bill with a zero 'Secondary Resp.' balance.\n"
						"However, you can increase the secondary responsibility if you apply directly to a charge.");
					//TES 3/22/2004
					else if(dlg.m_nResponsibility == -1)
						MsgBox("You may not apply an inactive insurance payment to a bill with a zero 'Inactive Resp.' balance.");
					else {	//anything else
						CString str = GetNameFromRespTypeID(dlg.m_nResponsibility);
						MsgBox("You may not apply an insurance payment to a bill with a zero '%s Resp.' balance.\n"
							"However, you can increase the %s responsibility if you apply directly to a charge.", str, str);
					}
					var = tempVar;

					// (j.jones 2013-07-01 16:17) - PLID 55517 - if we corrected the payment, we still
					// need to refresh the list
					if(eCanApplyResult == ecalirCanApply_IDHasChanged) {
						QuickRefresh();
						CalculateTotals();
					}
					return;
				}
			}
			}

			//adjustments have different properties, so the FinancialApplyDlg must know if it is an adjustment
			if(payType == "Adjustment")
				dlg.m_boIsAdjustment = TRUE;

			if(strClickedType == "Payment")
				dlg.m_boApplyToPay = TRUE;

			if (strClickedType == "Bill")
				var = pToRow->GetValue(m_nBillIDColID); // var=BillID
			else if (strClickedType == "Charge")
				var = pToRow->GetValue(btcChargeID); // var=ChargeID

			//before allowing them to apply, see if the source is a prepayment and the destination
			//is not a bill or charge created from the quote that the prepayment is linked with
			if(!AllowPaymentApply(nFromPayID,VarLong(var),strClickedType)) {

				// (j.jones 2013-07-01 16:17) - PLID 55517 - if we corrected the payment, we still
				// need to refresh the list
				if(eCanApplyResult == ecalirCanApply_IDHasChanged) {
					QuickRefresh();
					CalculateTotals();
				}
				return;
			}

			/////////////////////////////////////////////////////////////
			// Make the financial apply dialog appear so the user may
			// specify how much to apply, and also, if this person wants
			// to write off the remaining balance to the patient if it's
			// an insurance related apply.
			/////////////////////////////////////////////////////////////
			GetMainFrame()->DisableHotKeys();		

			if (IDCANCEL == dlg.DoModal() || (!dlg.m_boZeroAmountAllowed && dlg.m_cyApplyAmount == COleCurrency(0,0))) {
				//goto end_jump;
				GetMainFrame()->EnableHotKeys();

				// (j.jones 2013-07-01 16:17) - PLID 55517 - if we corrected the payment, we still
				// need to refresh the list
				if(eCanApplyResult == ecalirCanApply_IDHasChanged) {
					QuickRefresh();
					CalculateTotals();
				}
				return;
			}
			GetMainFrame()->EnableHotKeys();

			long nBillID = -1;

			/////////////////////////////////////////////////////////////
			// Get ID of item that will be applied to and make the apply
			/////////////////////////////////////////////////////////////
			if (strClickedType == "Bill") {
				var = pToRow->GetValue(m_nBillIDColID); // var=BillID
				nBillID = VarLong(var);
				if (dlg.m_nResponsibility == 0)
					ApplyPayToBill(nFromPayID, m_iCurPatient, dlg.m_cyApplyAmount, strClickedType, VarLong(var));
				else {
					//(e.lally 2007-03-30) PLID 25263 - Switched to new general Apply Pay To Bill function
					ApplyPayToBill(nFromPayID, m_iCurPatient, dlg.m_cyApplyAmount, strClickedType, VarLong(var), InsuredPartyID, FALSE, dlg.m_boShiftBalance, dlg.m_boAdjustBalance, FALSE, TRUE, FALSE, TRUE);
				}

				if (dlg.m_boAdjustBalance) {

					if(CheckCurrentUserPermissions(bioAdjustment,sptCreate)) {

						CPaymentDlg paydlg(this);
						paydlg.m_iDefaultPaymentType = 1;
						paydlg.m_cyFinalAmount = AdjustBalance(VarLong(var), VarLong(var),m_iCurPatient,1,dlg.m_nResponsibility,InsuredPartyID);
						if(paydlg.m_cyFinalAmount.GetStatus()==COleCurrency::invalid) {
							MsgBox("This bill has been removed from this screen. Practice will now requery this patient's financial information.");
							FullRefresh();
							//goto end_jump;
							return;
						}
						paydlg.m_varBillID = var;
						paydlg.m_ApplyOnOK = TRUE;
						paydlg.m_PromptToShift = FALSE;
						// (j.jones 2008-04-29 10:29) - PLID 29744 - this didn't support inactive insurance					
						if(dlg.m_nResponsibility != 0 && InsuredPartyID > 0) {
							paydlg.m_iDefaultInsuranceCo = InsuredPartyID;
						}
						GetMainFrame()->DisableHotKeys();
						paydlg.DoModal(__FUNCTION__, __LINE__);
						GetMainFrame()->EnableHotKeys();
						if (paydlg.m_bBillCreated) {
							// (r.gonet 2015-07-06 18:02) - PLID 65327 - If a bill is created, we need to do more than
							// just redraw the payments.
							bQuickRefresh = true;
						}
					}
				}

				if (dlg.m_nResponsibility != 0) {
					// See if we need to shift the remaining balance to the 
					// patient in the case of an insurance apply.
					//JMJ - 7/24/2003 - only do this if there is still a balance
					if (dlg.m_boShiftBalance && -AdjustBalance(VarLong(var), VarLong(var),m_iCurPatient,1,dlg.m_nResponsibility,InsuredPartyID) > COleCurrency(0,0)) {
						try {
							// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
							ShiftInsBalance(VarLong(var), m_iCurPatient, GetInsuranceIDFromType(m_iCurPatient, dlg.m_nResponsibility), GetInsuranceIDFromType(m_iCurPatient, dlg.m_nShiftToResp), "Bill",
								"after applying in the billing tab through drag and drop");
						}NxCatchAll("Error in ShiftInsBalance");
					}

					// (j.jones 2011-03-23 17:28) - PLID 42936 - now we have to check the allowable for what we applied
					_RecordsetPtr rsAppliedTo = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
						"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location AS POSID, "
						"InsuredPartyT.PersonID, ChargesT.ID, Sum(AppliesT.Amount) AS Amount "
						"FROM LineItemT "
						"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
						"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
						"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
						"INNER JOIN ChargeRespT ON AppliesT.RespID = ChargeRespT.ID "
						"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
						"WHERE LineItemT.Deleted = 0 AND AppliesT.SourceID = {INT} AND BillsT.ID = {INT} "
						"GROUP BY ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
						"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location, "
						"InsuredPartyT.PersonID, ChargesT.ID", nFromPayID, VarLong(var));
					while(!rsAppliedTo->eof) {
						//WarnAllowedAmount takes in the ServiceID, InsCoID, ProviderID, ChargeID, and the dollar amount we applied
						WarnAllowedAmount(AdoFldLong(rsAppliedTo, "ServiceID", -1), AdoFldLong(rsAppliedTo, "InsuranceCoID", -1),
							AdoFldLong(rsAppliedTo, "DoctorsProviders", -1), AdoFldLong(rsAppliedTo, "LocationID", -1),
							AdoFldLong(rsAppliedTo, "POSID", -1), AdoFldLong(rsAppliedTo, "PersonID", -1),
							AdoFldLong(rsAppliedTo, "ID"), AdoFldCurrency(rsAppliedTo, "Amount"));

						//if we underpaid multiple charges, this will cause multiple prompts (this is not typical)
						rsAppliedTo->MoveNext();
					}
					rsAppliedTo->Close();
				}

				CheckUnbatchClaim(VarLong(var));

				RedrawBill(VarLong(var));
			}
			else if (strClickedType == "Charge") {
				var = pToRow->GetValue(btcChargeID); // var=ChargeID
				// (j.jones 2009-12-29 12:00) - PLID 27237 - get this from the list, not from data
				nBillID = VarLong(pToRow->GetValue(m_nBillIDColID), -1);

				if (dlg.m_nResponsibility == 0)
					ApplyPayToBill(nFromPayID, m_iCurPatient, dlg.m_cyApplyAmount, strClickedType, VarLong(var));
				else {
					if(dlg.m_boIncreaseInsBalance) {
						if(!IncreaseInsBalance(VarLong(var),m_iCurPatient,InsuredPartyID,dlg.m_cyApplyAmount)) {

							// (j.jones 2013-07-01 16:17) - PLID 55517 - if we corrected the payment, we still
							// need to refresh the list
							if(eCanApplyResult == ecalirCanApply_IDHasChanged) {
								QuickRefresh();
								CalculateTotals();
							}
							return;
						}
					}

					//(e.lally 2007-03-30) PLID 25263 - Switched to new general Apply Pay To Bill function
					ApplyPayToBill(nFromPayID, m_iCurPatient, dlg.m_cyApplyAmount, strClickedType, VarLong(var), InsuredPartyID, FALSE, dlg.m_boShiftBalance, FALSE, FALSE, TRUE, FALSE, TRUE);

					if(dlg.m_boIncreaseInsBalance) {
						//set the resp back to patient incase they adjust the balance later
						dlg.m_nResponsibility = 0;
					}
				}

				if (dlg.m_boAdjustBalance) {

					if(CheckCurrentUserPermissions(bioAdjustment,sptCreate)) {

						CPaymentDlg paydlg(this);
						paydlg.m_iDefaultPaymentType = 1;
						paydlg.m_cyFinalAmount = AdjustBalance(VarLong(var), nBillID, m_iCurPatient,2,dlg.m_nResponsibility,InsuredPartyID);
						if(paydlg.m_cyFinalAmount.GetStatus()==COleCurrency::invalid) {
							MsgBox("This charge has been removed from this screen. Practice will now requery this patient's financial information.");
							FullRefresh();
							//goto end_jump;
							return;
						}
						paydlg.m_varChargeID = var;
						paydlg.m_ApplyOnOK = TRUE;
						paydlg.m_PromptToShift = FALSE;
						// (j.jones 2008-04-29 10:29) - PLID 29744 - this didn't support inactive insurance
						if(dlg.m_nResponsibility != 0 && InsuredPartyID > 0) {
							paydlg.m_iDefaultInsuranceCo = InsuredPartyID;
						}
						GetMainFrame()->DisableHotKeys();
						paydlg.DoModal(__FUNCTION__, __LINE__);
						GetMainFrame()->EnableHotKeys();
						if (paydlg.m_bBillCreated) {
							// (r.gonet 2015-07-06 18:02) - PLID 65327 - If a bill is created, we need to do more than
							// just redraw the payments.
							bQuickRefresh = true;
						}
					}
				}

				if(dlg.m_nResponsibility != 0) {
					// See if we need to shift the remaining balance to the 
					// patient in the case of an insurance apply.
					//JMJ - 7/24/2003 - only do this if there is still a balance
					if (dlg.m_boShiftBalance && -AdjustBalance(VarLong(var), nBillID, m_iCurPatient,2,dlg.m_nResponsibility,InsuredPartyID) > COleCurrency(0,0)) {
						try {
							// (j.jones 2013-08-21 09:00) - PLID 58194 - added a descriptive audit string
							ShiftInsBalance(VarLong(var), m_iCurPatient, GetInsuranceIDFromType(m_iCurPatient, dlg.m_nResponsibility), GetInsuranceIDFromType(m_iCurPatient, dlg.m_nShiftToResp), "Charge",
								"after applying in the billing tab through drag and drop");
						}NxCatchAll("Error in ShiftInsBalance");
					}

					// (j.jones 2011-03-23 17:28) - PLID 42936 - now we have to check the allowable for what we applied
					_RecordsetPtr rsAppliedTo = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
						"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location AS POSID, "
						"InsuredPartyT.PersonID, ChargesT.ID, Sum(AppliesT.Amount) AS Amount "
						"FROM LineItemT "
						"INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
						"INNER JOIN BillsT ON ChargesT.BillID = BillsT.ID "
						"INNER JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
						"INNER JOIN ChargeRespT ON AppliesT.RespID = ChargeRespT.ID "
						"INNER JOIN InsuredPartyT ON ChargeRespT.InsuredPartyID = InsuredPartyT.PersonID "
						"WHERE LineItemT.Deleted = 0 AND AppliesT.SourceID = {INT} AND ChargesT.ID = {INT} "
						"GROUP BY ChargesT.ServiceID, InsuredPartyT.InsuranceCoID, "
						"ChargesT.DoctorsProviders, LineItemT.LocationID, BillsT.Location, "
						"InsuredPartyT.PersonID, ChargesT.ID", nFromPayID, VarLong(var));
					while(!rsAppliedTo->eof) {
						//WarnAllowedAmount takes in the ServiceID, InsCoID, ProviderID, ChargeID, and the dollar amount we applied
						WarnAllowedAmount(AdoFldLong(rsAppliedTo, "ServiceID", -1), AdoFldLong(rsAppliedTo, "InsuranceCoID", -1),
							AdoFldLong(rsAppliedTo, "DoctorsProviders", -1), AdoFldLong(rsAppliedTo, "LocationID", -1),
							AdoFldLong(rsAppliedTo, "POSID", -1), AdoFldLong(rsAppliedTo, "PersonID", -1),
							AdoFldLong(rsAppliedTo, "ID"), AdoFldCurrency(rsAppliedTo, "Amount"));

						//if we underpaid multiple charges, this will cause multiple prompts (this is not typical)
						rsAppliedTo->MoveNext();
					}
					rsAppliedTo->Close();
				}

				CheckUnbatchClaim(nBillID);

				// Find the line ID of the bill and redraw it
				var = pToRow->GetValue(m_nBillIDColID); // var=BillID
				if(var.vt == VT_I4) {
					RedrawBill(VarLong(var));
				}
			}
			else {
				var = pToRow->GetValue(btcPayID); // var=PaymentID
				ApplyPayToBill(nFromPayID, m_iCurPatient, dlg.m_cyApplyAmount, strClickedType, VarLong(var), -1 /*Patient Resp*/, TRUE);
			}

			// (j.jones 2013-07-22 10:14) - PLID 57653 - If they have configured insurance companies
			// to force unbatching due to primary crossover to secondary, force unbatching now.
			// This needs to be after shifting/batching has occurred in the normal posting flow.
			if((strClickedType == "Bill" || strClickedType == "Charge")
				&& nBillID != -1 && InsuredPartyID != -1) {
				//This ConfigRT name is misleading, it actually just means that if we do unbatch a crossed over claim,
				//claim history will only include batched charges. If false, then claim history includes all charges.
				bool bBatchedChargesOnlyInClaimHistory = (GetRemotePropertyInt("ERemit_UnbatchMA18orNA89_MarkForwardToSecondary", 1, 0, "<None>", true) == 1);

				//This function assumes that the bill's current insured party ID is now the "secondary" insured party
				//we crossed over to, and the insured party who paid was primary.
				//If the payer really was the patient's Primary, and crossing over is enabled, the bill will be unbatched.
				CheckUnbatchCrossoverClaim(m_iCurPatient, nBillID, InsuredPartyID, dtPayment,
					bBatchedChargesOnlyInClaimHistory, aeiClaimBatchStatusChangedByManualCrossover, "Batched", "Unbatched due to manual Primary/Secondary crossover");
			}

			// (r.gonet 2015-07-06 18:02) - PLID 65327 - Refresh if required but otherwise, just redraw the payments.
			if (bQuickRefresh) {
				QuickRefresh();
			} else {
				RedrawAllPayments();
				CalculateTotals();
			}
			EndWaitCursor();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-06-07 15:18) - PLID 39042 - converted into a datalist 2 version
void CFinancialDlg::OnLeftClickBillingTabList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		IRowSettingsPtr pRow(lpRow);

		if(!l_bProcessingClick) {
			l_bProcessingClick = true;
			LeftClickList(pRow, nCol);	
			l_bProcessingClick = false;
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2010-06-07 15:33) - PLID 39042 - added the calculation
// to lighten a color into its own function
COLORREF CFinancialDlg::LightenSelColor(COLORREF clrToChange)
{
	WORD r,g,b;
	COLORREF clr = clrToChange;

	if(clr != 0xFFFFFFFF) {
		r = GetRValue(clr); g = GetGValue(clr); b = GetBValue(clr);
		clr = RGB((r+255)/2,(g+255)/2,(b+255)/2);
	}

	return clr;
}

// (z.manning 2010-06-11 14:04) - PLID 29214
long GetEquivalentInsuredPartyIDForTransfer(CWnd *pwnd, const long nSourceInsPartyID, const long nNewPatientID, const CString strTransferObject, BOOL bSilent = FALSE)
{
	long nNewInsPartyID = -1;
	_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(),
		"SELECT RespTypeT.TypeName, InsuranceCoT.Name AS InsCoName, InsuredPartyT.PersonID \r\n"
		"FROM InsuredPartyT \r\n"
		"INNER JOIN InsuredPartyT SourceInsParty ON InsuredPartyT.RespTypeID = SourceInsParty.RespTypeID \r\n"
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID \r\n"
		"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID \r\n"
		"WHERE InsuredPartyT.PatientID = {INT} AND SourceInsParty.PersonID = {INT} \r\n"
		, nNewPatientID, nSourceInsPartyID);

	CString strMessage = FormatString("This %s is for an insurance responsibility. ", strTransferObject);
	if(prs->eof)
	{
		// (z.manning 2010-06-11 14:05) - PLID 29214 - The new patient does not have an equivalent
		// insured party so let's warn and the caller can set the resp to patient.
		// (j.luckoski 2012-07-16 11:23) - PLID 51348 - Corrected spelling mistake.
		nNewInsPartyID = -1;
		strMessage += FormatString("%s does not have an equivalent insurance responsibility so it will be switched to patient responsibility."
			, GetExistingPatientName(nNewPatientID));
	}
	else
	{
		nNewInsPartyID = AdoFldLong(prs->GetFields(), "PersonID");
		CString strResp = AdoFldString(prs->GetFields(), "TypeName", "");
		strResp.MakeLower();
		CString strInsCo = AdoFldString(prs->GetFields(), "InsCoName", "");
		strMessage += FormatString("The %s insurance company for %s is %s so that is where the responsibility will be assigned."
			, strResp, GetExistingPatientName(nNewPatientID), strInsCo);
	}

	strMessage += "\r\n\r\nAre you sure you want to continue?";
	if(!bSilent) {
		if(IDYES != pwnd->MessageBox(strMessage, NULL, MB_YESNO|MB_ICONQUESTION)) {
			nNewInsPartyID = -2;
		}
	}

	return nNewInsPartyID;
}

// (z.manning 2010-06-10 16:45) - PLID 29214 - Function to transfer payments to another patient
void CFinancialDlg::TransferPaymentToAnotherPatient(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return;
	}

	CString strType = VarString(pRow->GetValue(btcLineType), "");
	CString strTypeLower = strType;
	strTypeLower.MakeLower();

	const long nLineItemID = VarLong(pRow->GetValue(btcPayID), -1);
	if(nLineItemID == -1) {
		// (z.manning 2010-06-11 10:52) - PLID 29214 - This should only be called on payment rows
		ASSERT(FALSE);
		return;
	}

	// (j.jones 2011-09-13 15:37) - PLID 44887 - disallow this on corrected/original line items
	LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nLineItemID);
	if(licsStatus == licsOriginal) {
		AfxMessageBox("This line item has been corrected, and can not be modified.");
		return;
	}
	else if(licsStatus == licsVoid) {
		AfxMessageBox("This line item is a void line item from an existing correction, and cannot be modified.");
		return;
	}

	// (z.manning 2010-06-15 09:19) - PLID 39157 - Make sure they have permission
	EBuiltInObjectIDs ePerm;
	if(strType == "Adjustment") {
		ePerm = bioAdjustment;
	}
	else if(strType == "Refund") {
		ePerm = bioRefund;
	}
	else {
		ePerm = bioPayment;
	}
	if(!CheckCurrentUserPermissions(ePerm, sptDynamic0)) {
		return;
	}

	// (z.manning 2010-06-16 10:42) - PLID 29214 - If the payment is tied to anything where transferring it would mess up data
	// for the current patient, then prevent the transfer.
	_RecordsetPtr prsLinks = CreateParamRecordset(GetRemoteDataSnapshot(),
		"SET NOCOUNT ON \r\n"
		"DECLARE @nPaymentID int \r\n"
		"SET @nPaymentID = {INT} \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT PaymentsT.BatchPaymentID, PaymentsT.QuoteID, \r\n"
		"	CASE WHEN EXISTS (SELECT ProcInfoPaymentsT.PayID FROM ProcInfoPaymentsT WHERE ProcInfoPaymentsT.PayID = @nPaymentID) THEN CONVERT(bit, 1) ELSE CONVERT(bit, 0) END AS HasProcInfo \r\n"
		// (z.manning 2010-06-30 08:56) - PLID 39398 - We now allow transferring of processed credit card payments
		//"	CASE WHEN EXISTS (SELECT CreditTransactionsT.ID FROM CreditTransactionsT WHERE CreditTransactionsT.ID = @nPaymentID) THEN CONVERT(bit, 1) ELSE CONVERT(bit, 0) END AS HasCreditTran, \r\n"
		//"	CASE WHEN EXISTS (SELECT QBMS_CreditTransactionsT.ID FROM QBMS_CreditTransactionsT WHERE QBMS_CreditTransactionsT.ID = @nPaymentID) THEN CONVERT(bit, 1) ELSE CONVERT(bit, 0) END AS HasQBMSCreditTran \r\n"
		"FROM PaymentsT \r\n"
		"INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID \r\n"
		"WHERE PaymentsT.ID = @nPaymentID \r\n"
		, nLineItemID);
	if(!prsLinks->eof) {
		FieldsPtr flds = prsLinks->GetFields();
		if(AdoFldBool(flds, "HasProcInfo")) {
			MessageBox(FormatString("This %s is tied to a tracking ladder and cannot be transferred.",strTypeLower), NULL, MB_OK|MB_ICONWARNING);
			return;
		}
		if(AdoFldLong(flds, "BatchPaymentID", -1) != -1) {
			MessageBox(FormatString("This %s is part of a batch payment and cannot be transferred.",strTypeLower), NULL, MB_OK|MB_ICONWARNING);
			return;
		}
		// (z.manning 2010-06-30 08:56) - PLID 39398 - We now allow transferring of processed credit card payments
		//if(AdoFldBool(flds, "HasCreditTran") || AdoFldBool(flds, "HasQBMSCreditTran")) {
		//	MessageBox(FormatString("This %s has already been processed and cannot be transferred.",strTypeLower), NULL, MB_OK|MB_ICONWARNING);
		//	return;
		//}
		if(AdoFldLong(flds, "QuoteID", -1) != -1) {
			int nResult = MessageBox(FormatString("This %s is applied to a quote. Are you sure you want to transfer it?",strTypeLower), NULL, MB_YESNO|MB_ICONQUESTION);
			if(nResult != IDYES) {
				return;
			}
		}
	}
	prsLinks->Close();

	// (z.manning 2010-06-11 10:49) - PLID 29214 - No transferring if this is applied to anything or if 
	// anything is applied to it.
	_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(),
		"SET NOCOUNT ON \r\n"
		"DECLARE @nLineItemID int \r\n"
		"SET @nLineItemID = {INT} \r\n"
		"SET NOCOUNT OFF \r\n"
		"SELECT TOP 1 ID FROM AppliesT WHERE SourceID = @nLineItemID \r\n"
		"SELECT TOP 1 ID FROM AppliesT WHERE DestID = @nLineItemID \r\n"
		"SELECT InsuredPartyID \r\n"
		"FROM PaymentsT \r\n"
		"WHERE PaymentsT.ID = @nLineItemID \r\n"
		, nLineItemID);
	if(!prs->eof) {
		MessageBox(FormatString("This %s has been applied to at least one line item. You must unapply it completely before transferring it.", strTypeLower), NULL, MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	prs = prs->NextRecordset(NULL);
	if(!prs->eof) {
		MessageBox(FormatString("This %s has at least one line item applied to it. You must unapply everything from it before transferring.", strTypeLower), NULL, MB_OK|MB_ICONEXCLAMATION);
		return;
	}

	prs = prs->NextRecordset(NULL);
	long nInsPartyID = -1;
	if(!prs->eof) {
		nInsPartyID = AdoFldLong(prs->GetFields(), "InsuredPartyID", -1);
	}

	// (z.manning 2010-06-11 11:21) - PLID 38612 - Prompt for the patient to transfer to
	CSingleSelectDlg dlgSelectPatient(this);
	int nResult = dlgSelectPatient.Open(
		"PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID",
		FormatString("PersonT.ID > 0 AND CurrentStatus <> 4 AND Archived = 0 AND PersonT.ID <> %li", m_iCurPatient),
		"PersonT.ID", "Last + ', ' + First + ' ' + Middle + ' (' + convert(nvarchar(20),UserDefinedID) + ')'",
		FormatString("Select the patient to transfer the %s to...", strTypeLower), true);

	if(nResult != IDOK) {
		return;
	}
	const long nNewPatientID = dlgSelectPatient.GetSelectedID();

	// (z.manning 2010-06-11 12:07) - PLID 29214 - Get the amount from the appropriate column
	AuditEventItems aei;
	_variant_t varAmount = g_cvarNull;
	if(strType == "Payment" || strType == "PrePayment") {
		varAmount = pRow->GetValue(m_nPaymentColID);
		aei = aeiTransferPaymentToAnotherPatient;
	}
	else if(strType == "Adjustment") {
		varAmount = pRow->GetValue(m_nAdjustmentColID);
		aei = aeiTransferAdjustmentToAnotherPatient;
	}
	else if(strType == "Refund") {
		varAmount = pRow->GetValue(m_nRefundColID);
		aei = aeiTransferRefundToAnotherPatient;
	}
	CString strAmount;
	if(varAmount.vt == VT_CY) {
		strAmount = FormatCurrencyForInterface(VarCurrency(varAmount));
	}

	// (z.manning 2010-06-11 12:08) - PLID 29214 - One last prompt
	CString strDescription = FormatString("%s - %s", strAmount, VarString(pRow->GetValue(m_nDescriptionColID),""));
	nResult = MessageBox(FormatString(
		"Are you sure you want to transfer the following %s for %s...\r\n\r\n%s\r\n\r\n"
		"...to %s?", strTypeLower, GetExistingPatientName(m_iCurPatient), strDescription, GetExistingPatientName(nNewPatientID))
		, "Transfer Payment", MB_YESNO|MB_ICONQUESTION);
	if(nResult != IDYES) {
		return;
	}

	CParamSqlBatch sqlBatch;

	if(nInsPartyID != -1) {
		// (z.manning 2010-06-11 14:50) - PLID 29214 - This payment was insurance resp so handle that 
		// change for the new patient.
		long nNewInsPartyID = GetEquivalentInsuredPartyIDForTransfer(this, nInsPartyID, nNewPatientID, strTypeLower);
		if(nNewInsPartyID == -2) {
			return;
		}
		else {
			sqlBatch.Add("UPDATE PaymentsT SET InsuredPartyID = {INT} WHERE ID = {INT}", nNewInsPartyID, nLineItemID);
		}
	}

	// (z.manning 2010-06-15 17:55) - PLID 39107 - Also update any notes tied to this payment
	sqlBatch.Add("UPDATE Notes SET PersonID = {INT} WHERE LineItemID = {INT} ", nNewPatientID, nLineItemID);
	sqlBatch.Add("UPDATE PaymentsT SET QuoteID = NULL WHERE PaymentsT.ID = {INT}", nLineItemID);

	// (z.manning 2010-06-11 12:08) - PLID 29214 - Go time! Transfer this payment to the select patient
	sqlBatch.Add("UPDATE LineItemT SET PatientID = {INT} WHERE LineItemT.ID = {INT}", nNewPatientID, nLineItemID);

	// (z.manning 2010-06-16 10:06) - PLID 29214 - Unapply potential tracking events from the old patient, commit
	// the transfer, then apply the action to the new patient.
	PhaseTracking::UnapplyEvent(m_iCurPatient, PhaseTracking::ET_PaymentApplied, nLineItemID);
	sqlBatch.Execute(GetRemoteData());
	PhaseTracking::CreateAndApplyEvent(PhaseTracking::ET_PaymentApplied, nNewPatientID, COleDateTime::GetCurrentTime(), nLineItemID);

	// (z.manning 2010-06-15 09:45) - PLID 39157 - Audit!
	CString strOld = FormatString("%s: %s", strType, strDescription);
	CString strNew = FormatString("Transferred from '%s' to '%s'", GetExistingPatientName(m_iCurPatient), GetExistingPatientName(nNewPatientID));
	AuditEvent(nNewPatientID, GetExistingPatientName(nNewPatientID), BeginNewAuditEvent(), aei, nLineItemID, strOld, strNew, aepHigh, aetChanged);

	CClient::RefreshTable(NetUtils::PatBal, m_iCurPatient);
	CClient::RefreshTable(NetUtils::PatBal, nNewPatientID);
	FullRefresh();
}

// (j.gruber 2010-09-07 11:09) - PLID 39571 - added function to get the EMns to be billed screen
void CFinancialDlg::OnBnClickedPtsToBeBilled()
{
	try {
		//(j.camacho 2016-02-01) plid 68010
		// Don't bother adding an option to view HL7 bills, if they're not licensed for HL7
		if (IsIntellechartToBeBilledEnabled() && g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent))
		{
			//(j.camacho 2016-02-02) plid 68009  - create menu options for button
			CNxMenu menu;
			menu.m_hMenu = CreatePopupMenu();

			Nx::MenuCommandMap<long> menuCmds;

			long nMenuIndex = 0;
			menu.InsertMenu(nMenuIndex++, MF_BYPOSITION, menuCmds(1), "EMN");
			menu.InsertMenu(nMenuIndex++, MF_BYPOSITION, menuCmds(2), "Intellechart Encounter");

			long selected;
			POINT p;
			GetCursorPos(&p);
			selected = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, p.x, p.y, this);
			boost::optional<long> choice = menuCmds.Translate(selected);
			if (choice)
			{
				if (*choice == 1)
				{
					if ((g_pLicense->HasEMR(CLicense::cflrSilent) == 2) && CheckCurrentUserPermissions(bioPatientEMR, sptRead))
						GetMainFrame()->ShowPatientEMNsToBeBilled();
					else {
						MsgBox("You must have be licensed for NexEMR and have EMR read permission to access this screen.");
						return;
					}
				}
				else if (*choice == 2)
				{
					GetMainFrame()->ShowHL7ToBeBilledDlg();
				}
			}
		}
		else {
			if ((g_pLicense->HasEMR(CLicense::cflrSilent) == 2) && CheckCurrentUserPermissions(bioPatientEMR, sptRead))
				GetMainFrame()->ShowPatientEMNsToBeBilled();
			else {
				MsgBox("You must have be licensed for NexEMR and have EMR read permission to access this screen.");
				return;
			}
		}


		
	}NxCatchAll(__FUNCTION__);

	//try {
	//	// (j.armen 2012-05-31 14:39) - PLID 50718 - Check if we are licensed using the helper function
	//	if((g_pLicense->HasEMR(CLicense::cflrSilent) == 2) && CheckCurrentUserPermissions(bioPatientEMR, sptRead)) 
	//		GetMainFrame()->ShowPatientEMNsToBeBilled();
	//	else {
	//		MsgBox("You must have be licensed for NexEMR and have EMR read permission to access this screen.");
	//		return;
	//	}	
	//}NxCatchAll(__FUNCTION__);
}

//(c.copits 2010-09-24) PLID 40317 - Allow duplicate UPC codes for FramesData certification
// This function will likely be updated to pick the most suitable
// UPC code in response to a barcode scan. Practice now allows multiple
// products to have the same UPC codes. Further, products can share UPC codes
// with service codes (however, service codes cannot share UPC codes).

// Current behavior: Will return a recordset of ServiceT.IDs with all matching UPCs;
//	these ServiceT.IDs will correspond to both service codes and inventory items.

_RecordsetPtr CFinancialDlg::GetBestUPCProduct(LPARAM lParam)
{

	_RecordsetPtr prs;

	try {
		prs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ServiceT.ID FROM ServiceT LEFT JOIN ProductT ON ServiceT.ID = ProductT.ID LEFT JOIN ProductLocationInfoT ON ProductT.ID = ProductLocationInfoT.ProductID "
			"WHERE BARCODE = {STRING} AND Active <> 0 AND (ProductT.ID IS NULL OR ProductLocationInfoT.Billable <> 0)", (LPCTSTR)_bstr_t((BSTR)lParam));
	} NxCatchAll(__FUNCTION__);

	return prs;
}

// (j.dinatale 2010-10-28) - PLID 28773 - This function will set up a new column based on its type and assign its width depending if
//		column width is remembered or not
void CFinancialDlg::SetUpColumnByType(long nColumnType, short &nIndex, long nWidth, BOOL bVisible)
{
	try{
		long nVisflags = -1;	// the vis flags for the new column
		long nWidthToUse = -1;	// the width for the new column

		if(nColumnType != btcInsurance)
			m_mapColIndexToColType.SetAt(nIndex, nColumnType);	// map the column index to the column type if its not an insurance column

		// Each case is set up to set the specific columns index, start with the specific columns default width and visiblity,
		// then if remember column width is turned on, change those flags accordingly and then create our column and increment the index
		switch(nColumnType)
		{
		case btcExpandChar:	// Expand Main item
			m_nExpandCharColID = nIndex;
			nVisflags = csVisible|csWidthPercent;
			nWidthToUse = 3;

			if(m_bRememberColWidth){
				nVisflags = csVisible;
				nWidthToUse = nWidth < 0 ? 30 : nWidth;
			}
			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nExpandCharColID, _T("ExpandChar"), _T(""), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftBitmapBuiltIn;
			nIndex++;
			break;
		case btcExpandSubChar:	// Subexpand for child items
			m_nExpandSubCharColID = nIndex;
			nVisflags = csVisible|csWidthPercent;
			nWidthToUse = 3;

			if(m_bRememberColWidth){
				nVisflags = csVisible;
				nWidthToUse = nWidth < 0 ? 30 : nWidth;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nExpandSubCharColID, _T("ExpandSubChar"), _T(""), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftBitmapBuiltIn;
			nIndex++;
			break;
		case btcBillID:	// the bill ID column
			m_nBillIDColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 40 : nWidth;
				nVisflags = csVisible;
			}else{
				nVisflags = csVisible|csWidthData;
				nWidthToUse = 40;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nBillIDColID, _T("BillID"), _T("Bill ID"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcDate:	// the date column
			m_nDateColID = nIndex;
			nVisflags = csVisible|csWidthData;
			nWidthToUse = 50;

			if(m_bRememberColWidth){
				nVisflags = csVisible;
				nWidthToUse = nWidth < 0 ? 50 : nWidth;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nDateColID, _T("Date"), _T("Date"), nWidthToUse, nVisflags)))->FieldType = cftDateShort;
			nIndex++;
			break;
		case btcNoteIcon:	// the note icon column
			m_nNoteIconColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 50 : nWidth;
				nVisflags = csVisible;
			}else{
				nWidthToUse = 3;
				nVisflags = csVisible|csWidthPercent;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nNoteIconColID, _T("NoteIcon"), _T(""), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftBitmapBuiltIn;
			nIndex++;
			break;
		case btcDiscountIcon:	// the discount icon column
			m_nDiscountIconColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 35 : nWidth; 
				nVisflags = bVisible ? csVisible : csFixedWidth;
			}else{
				nWidthToUse = 35; 
				nVisflags = bVisible ? csVisible|csFixedWidth : csFixedWidth;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nDiscountIconColID, _T("DiscountIcon"), _T("Disc."), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftBitmapBuiltIn;
			nIndex++;
			break;
		case btcOnHoldIcon:	// (a.wilson 2014-07-24 11:20) - PLID 63015 - the onhold icon status column
			m_nOhHoldIconColID = nIndex;

			if (m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 50 : nWidth;
				nVisflags = csVisible;
			}
			else{
				nWidthToUse = 3;
				nVisflags = csVisible | csWidthPercent;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nOhHoldIconColID, _T("OnHoldIcon"), _T(""), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible | csFixedWidth)))->FieldType = cftBitmapBuiltIn;
			nIndex++;
			break;
		case btcDescription:	// the description column
			m_nDescriptionColID = nIndex;
			nWidthToUse = 300;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 300 : nWidth;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nDescriptionColID, _T("Description"), _T("Description"), nWidthToUse, csVisible)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcCharge: // the charge amount column
			m_nChargeColID = nIndex;
			nVisflags = csVisible|csWidthData;
			nWidthToUse = 50;

			if(m_bRememberColWidth){
				nVisflags = csVisible;
				nWidthToUse = nWidth < 0 ? 50 : nWidth;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nChargeColID, _T("Charge"), _T("Charge"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		// (j.jones 2015-02-27 08:44) - PLID 64944 - added TotalPaymentAmount, which by default
		// is not shown, but is positioned between charge and payment
		case btcTotalPaymentAmount:
			m_nTotalPaymentAmountColID = nIndex;

			if (m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 70 : nWidth;
				nVisflags = csVisible;
			}
			else{
				nWidthToUse = 70;
				nVisflags = csVisible | csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nTotalPaymentAmountColID, _T("TotalPaymentAmount"), _T("Total Amt."), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible | csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcPayment:	// the payment amount column
			m_nPaymentColID = nIndex;
			nVisflags = csVisible|csWidthData;
			nWidthToUse = 58;

			if(m_bRememberColWidth){
				nVisflags = csVisible;
				nWidthToUse = nWidth < 0 ? 58 : nWidth;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nPaymentColID, _T("Payment"), _T("Payment"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcAdjustment:	// the adjustment amount column
			m_nAdjustmentColID = nIndex;
			nVisflags = csVisible|csWidthData;
			nWidthToUse = 73;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 73 : nWidth;
				nVisflags = csVisible;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nAdjustmentColID, _T("Adjustment"), _T("Adjustment"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcRefund:	// the refund amount column
			m_nRefundColID = nIndex;
			nVisflags = csVisible|csWidthData;
			nWidthToUse = 50;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 50 : nWidth;
				nVisflags = csVisible;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nRefundColID, _T("Refund"), _T("Refund"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcInsurance:	// the insurance columns are special cases, they all must be set up at the same time, the user will have the option to move them as a single block
			SetUpInsuranceColumns(nIndex);
			break;
		case btcBalance:	// the total balance column
			m_nBalanceColID = nIndex;
			nVisflags = csVisible|csWidthData;
			nWidthToUse = 50;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 50 : nWidth;
				nVisflags = csVisible;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nBalanceColID, _T("Balance"), _T("Balance"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcProvider:	// the provider column
			m_nProviderColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 50 : nWidth;
				nVisflags = csVisible;
			}else{
				nWidthToUse = 50;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nProviderColID, _T("Provider"), _T("Provider"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcLocation:	// the location column
			m_nLocationColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 50 : nWidth; 
				nVisflags = bVisible ? csVisible : csFixedWidth;
			}else{
				nWidthToUse = 50;
				nVisflags = bVisible ? csVisible|csWidthData : csFixedWidth;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nLocationColID, _T("Location"), _T("Location"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcPOSName:	// the place of service name
			m_nPOSNameColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 50 : nWidth; 
				nVisflags = bVisible ? csVisible : csFixedWidth;
			}else{
				nWidthToUse = 50;
				nVisflags = bVisible ? csVisible|csWidthData : csFixedWidth;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nPOSNameColID, _T("POSName"), _T("Place Of Service"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcPOSCode:	// the place of service code
			m_nPOSCodeColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 60 : nWidth; 
				nVisflags = csVisible;
			}else{
				nWidthToUse = 60;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nPOSCodeColID, _T("POSName"), _T("POS Code"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcInputDate:	// the input date
			m_nInputDateColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 50 : nWidth; 
				nVisflags = csVisible;
			}else{
				nWidthToUse = 50;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nInputDateColID, _T("InputDate"), _T("Input Date"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftDateShort;
			nIndex++;
			break;
		case btcCreatedBy:	// who created the billy
			m_nCreatedByColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 50 : nWidth; 
				nVisflags = csVisible;
			}else{
				nWidthToUse = 50;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nCreatedByColID, _T("CreatedBy"), _T("Created By"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcFirstChargeDate:	// the date of first charge
			m_nFirstChargeDateColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 100 : nWidth; 
				nVisflags = csVisible;
			}else{
				nWidthToUse = 100;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nFirstChargeDateColID, _T("FirstChargeDate"), _T("First Charge Date"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftDateShort;
			nIndex++;
			break;
		case btcDiagCode1:	// diag code 1
			m_nDiagCode1ColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 80 : nWidth; 
				nVisflags = csVisible;
			}else{
				nWidthToUse = 80;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nDiagCode1ColID, _T("DiagCode1"), _T("Diag. Code 1"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcDiagCode1WithName:	//diag code 1 with the name
			m_nDiagCode1WithNameColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 80 : nWidth; 
				nVisflags = csVisible;
			}else{
				nWidthToUse = 80;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nDiagCode1WithNameColID, _T("DiagCode1WithName"), _T("Diag. Code 1"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcDiagCodeList:	// the daig code list
			m_nDiagCodeListColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 80 : nWidth; 
				nVisflags = csVisible ;
			}else{
				nWidthToUse = 80;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nDiagCodeListColID, _T("DiagCodeList"), _T("Diag. Codes"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable
		case btcFeeSchedAllowable:	// the fee schedule allowable column
			m_nFeeSchedAllowableColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 127 : nWidth; 
				nVisflags = csVisible;
			}else{
				nWidthToUse = 127;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nFeeSchedAllowableColID, _T("FeeSchedAllowable"), _T("Fee Sched. Allowable"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		// (j.jones 2011-04-27 11:19) - PLID 43449 - added ChargeAllowable
		case btcChargeAllowable:	// the charge allowable column
			m_nChargeAllowableColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 92 : nWidth; 
				nVisflags = csVisible;
			}else{
				nWidthToUse = 92;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nChargeAllowableColID, _T("ChargeAllowable"), _T("EOB Allowable"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		// (j.jones 2011-12-20 11:37) - PLID 47119 - added deductible and coinsurance
		case btcChargeDeductible:	// the charge deductible column
			m_nChargeDeductibleColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 93 : nWidth; 
				nVisflags = csVisible;
			}else{
				nWidthToUse = 93;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nChargeDeductibleColID, _T("ChargeDeductible"), _T("EOB Deductible"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		case btcChargeCoinsurance:	// the charge coinsurance column
			m_nChargeCoinsuranceColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 103 : nWidth; 
				nVisflags = csVisible;
			}else{
				nWidthToUse = 103;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nChargeCoinsuranceColID, _T("ChargeCoinsurance"), _T("EOB Coinsurance"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
		case btcInvoiceNumber:
			m_nInvoiceNumberColID = nIndex;

			if(m_bRememberColWidth){
				nWidthToUse = nWidth < 0 ? 60 : nWidth; 
				nVisflags = csVisible;
			}else{
				nWidthToUse = 60;
				nVisflags = csVisible|csWidthData;
			}

			IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nInvoiceNumberColID, _T("InvoiceID"), _T("Invoice #"), bVisible ? nWidthToUse : 0, bVisible ? nVisflags : csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			nIndex++;
			break;
		default:
			// this should never happen, we should always know what type of column we are creating
			//ASSERT(FALSE);
			break;
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2010-10-28) - PLID 28773 - This function will take care of setting all our columns up
void CFinancialDlg::SetUpColumns(bool bRemoveColumns)
{
	try{
		// Disable the screen
		DisableFinancialScreen();

		// in some instances, we want to remove the existing columns from our list, so if thats the case,
		// remove them all
		if(bRemoveColumns){
			int nColCount = m_List->GetColumnCount();

			for(int i= 0; i < nColCount; i++){
				m_List->RemoveColumn(0);
			}
		}

		// clear all our member variable values
		ClearColumnIndices();

		// LineID, PayToPayID, PayID, ApplyID, LineType, and ChargeID must ALL be added first, they are all columns that will never change and are always necessary.
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(btcLineID, _T("LineID"), _T("LineID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(btcPayToPayID, _T("PayToPayID"), _T("PayToPayID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(btcPayID, _T("PayID"), _T("PayID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(btcApplyID, _T("ApplyID"), _T("ApplyID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(btcLineType, _T("LineType"), _T("LineType"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(btcChargeID, _T("ChargeID"), _T("ChargeID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;

		// starting at the index after the first necessary column and clear out our mapping of Column Index to ColumnType
		short nCurrColIndex = btcChargeID + 1;
		m_mapColIndexToColType.RemoveAll();

		m_mapColIndexToStoredWidth.RemoveAll();

		// (j.dinatale 2010-11-02) - PLID 39226 - need to handle the order of columns
		// poll the structure for the column info
		_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ColumnType, StoredWidth, Visible FROM BillingColumnsT WHERE UserID = {INT} ORDER BY OrderIndex", GetCurrentUserID());

		// (j.dinatale 2010-11-02) - PLID 39226 - need to handle the order of columns
		// if nothing was returned, then we need to ensure the structure is in place, so add the structure and requery for the info
		if(prs->eof){
			prs->Close();
			EnsureBillColTSQLStructure();
			prs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT ColumnType, StoredWidth, Visible FROM BillingColumnsT WHERE UserID = {INT} ORDER BY OrderIndex", GetCurrentUserID());
		}
		FieldsPtr flds = prs->Fields;

		while(!prs->eof)
		{
			// grab the necessary info
			long nColType = AdoFldLong(flds, "ColumnType");
			long nStoredWidth = AdoFldLong(flds, "StoredWidth", -1);
			BOOL bVisible = AdoFldBool(flds, "Visible", TRUE);

			m_mapColIndexToStoredWidth.SetAt(nCurrColIndex, nStoredWidth);

			// set up the column
			SetUpColumnByType(nColType, nCurrColIndex, nStoredWidth, bVisible);

			prs->MoveNext();
		}
		prs->Close();

		// check all member variables for proper values (> -1)
		VerifyNonInsuranceColumns(nCurrColIndex);

		// if we are not remembering column width, then resize the description column
		if(!m_bRememberColWidth){
			ResizeDescriptionColumn();
		}

		// we need to keep track of the first invalid index, thats simply the next col index once we are done setting up
		m_nMaxColID = nCurrColIndex;

		// show drag and drop of cells
		m_List->DragVisible = TRUE;

		// not a combo box, and sort by lineID
		m_List->IsComboBox = FALSE;
		m_List->GetColumn(btcLineID)->PutSortPriority(0);
		m_List->GetColumn(btcLineID)->PutSortAscending(TRUE);

		// gray out columns preference and set up the columns to have their designated colors
		BOOL bGrayOutColumns = GetRemotePropertyInt("GrayOutInsuranceColumns",0,0,"<None>",TRUE);

		for (int i=0; i < m_nMaxColID; i++) {
			IColumnSettingsPtr pCol = m_List->GetColumn(i);
			COLORREF clr = GetColorForColumn(i, bGrayOutColumns);
			pCol->PutBackColor(clr);
			pCol->PutBackColorSel(LightenSelColor(clr));
			pCol->PutForeColorSel(RGB(0,0,0));
		}

		// enable the screen
		EnableFinancialScreen();

	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2010-10-28) - PLID 28773 - This function will take care of setting the insurance columns because they are in their own tables
void CFinancialDlg::SetUpInsuranceColumns(short &nIndex)
{
	try{
		//clear our array and maps
		m_aryDynamicRespTypeIDs.RemoveAll();
		m_mapColumnIndexToRespTypeID.RemoveAll();
		m_mapRespTypeIDToColumnIndex.RemoveAll();
		m_mapRespTypeIDToRespName.RemoveAll();

		// going to need the stored width of the Other Insurance column to be stored, it needs to be the last column
		long nOtherInsActualWidth = -1;
		long nUserID = GetCurrentUserID();

		// (j.dinatale 2010-11-02) - PLID 39226 - priority handles the column order
		// ensures that all insurances show up on the tab and 
		// creates a recordset containing our RespTypeColumnWidthsT and RespTypeT table information about which insurances are visible and their stored widths
		_RecordsetPtr prs = CreateParamRecordset(GetRemoteDataSnapshot(),
			"SELECT RespTypeID, Priority, StoredWidth, TypeName, HasBillingColumn AS Visible FROM RespTypeColumnWidthsT \r\n"
			"LEFT JOIN RespTypeT ON RespTypeColumnWidthsT.RespTypeID = RespTypeT.ID \r\n"
			"WHERE RespTypeColumnWidthsT.UserID = {INT} AND RespTypeT.HasBillingColumn = 1 AND RespTypeT.ID <> -1 \r\n"
			"UNION \r\n"
			"SELECT RespTypeID, RespTypeID AS Priority, StoredWidth, '', 1  FROM RespTypeColumnWidthsT WHERE UserID = {INT} AND RespTypeID IN (-2, -3) \r\n"
			"ORDER BY Priority \r\n", 
			nUserID, nUserID);

		if(prs->eof){
			prs->Close();
			EnsureRespTypeColWidthTSQLStruct();
			prs = CreateParamRecordset(GetRemoteDataSnapshot(),
			"SELECT RespTypeID, Priority, StoredWidth, TypeName, HasBillingColumn AS Visible FROM RespTypeColumnWidthsT \r\n"
			"LEFT JOIN RespTypeT ON RespTypeColumnWidthsT.RespTypeID = RespTypeT.ID \r\n"
			"WHERE RespTypeColumnWidthsT.UserID = {INT} AND RespTypeT.HasBillingColumn = 1 AND RespTypeT.ID <> -1 \r\n"
			"UNION \r\n"
			"SELECT RespTypeID, RespTypeID AS Priority, StoredWidth, '', 1  FROM RespTypeColumnWidthsT WHERE UserID = {INT} AND RespTypeID IN (-2, -3) \r\n"
			"ORDER BY Priority \r\n", 
			nUserID, nUserID);
		}

		FieldsPtr flds = prs->Fields;

		while(!prs->eof){
			// pull the info for each "column" pulled from our recordset
			long nID = AdoFldLong(flds, "RespTypeID");
			long nPriority = AdoFldLong(flds, "Priority", -1);
			long nStoredWidth = AdoFldLong(flds, "StoredWidth", -1);

			// some calculation will be done to determine what the actual width  is, but for the time being, start at 50
			long nActualWidth = 0;
			long nVisFlags = csVisible|csWidthData;

			// if we are remembering column width, go ahead and use the stored width pulled, if for some reason its less than 0, default to 50 pixels
			if(m_bRememberColWidth){
				nActualWidth = nStoredWidth < 0 ? 75 : nStoredWidth;
				nVisFlags = csVisible;
			}

			if(nPriority == -2){
				// priority of -2 indicates our patient balance column, so create the column accordingly, and increment our index
				m_nPatientBalanceColID = nIndex;
				IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nPatientBalanceColID, _T("PatResp"), _T("Pat. Resp"), nActualWidth, nVisFlags)))->FieldType = cftTextSingleLine;
				m_mapColIndexToColType.SetAt(nIndex, btcInsurance);
				m_mapColIndexToStoredWidth.SetAt(nIndex, nActualWidth);
				nIndex++;
			}else{
				if(nPriority == 1){
					// priority of 1 indicates our primary insurance column, so create the column accordingly, and increment our index
					m_nPrimInsColID = nIndex;
					IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nPrimInsColID, _T("InsResp1"), _T("Primary"), nActualWidth, nVisFlags)))->FieldType = cftTextSingleLine;
					m_mapColIndexToColType.SetAt(nIndex, btcInsurance);
					m_mapColIndexToStoredWidth.SetAt(nIndex, nActualWidth);
					nIndex++;
				}
				else{
					// priority of 2 indicates our secondary insurance column, so create the column accordingly, and increment our index
					if(nPriority == 2){
						m_nSecInsColID = nIndex;
						IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nSecInsColID, _T("InsResp2"), _T("Secondary"), nActualWidth, nVisFlags)))->FieldType = cftTextSingleLine;
						m_mapColIndexToColType.SetAt(nIndex, btcInsurance);
						m_mapColIndexToStoredWidth.SetAt(nIndex, nActualWidth);
						nIndex++;
					}
					else{
						if(nPriority == -3){
							// priority of -3 indicates our other insurance column, now this is a special case. This column must show up at the end of the list.
							// So, in order for us to do that, we need to store the information we pulled for this column
							nOtherInsActualWidth = nActualWidth;
						}
						else{
							CString strTypeName = AdoFldString(flds, "TypeName", "");
							CString strFieldName;
							strFieldName.Format("InsResp%li", nPriority);

							IColumnSettingsPtr pCol = IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(nIndex, _T((LPCSTR)strFieldName), _T((LPCSTR)strTypeName), nActualWidth, nVisFlags)));
							pCol->FieldType = cftTextSingleLine;

							m_mapColIndexToColType.SetAt(nIndex, btcInsurance);

							//update our array
							m_aryDynamicRespTypeIDs.Add(nID);

							//update our maps
							m_mapColumnIndexToRespTypeID.SetAt(nIndex, nID);
							m_mapRespTypeIDToColumnIndex.SetAt(nID, nIndex);
							m_mapRespTypeIDToRespName.SetAt(nID, strTypeName);

							//colorize the column, but do not call GetColorForColumn, because
							//the dynamic columns aren't configured properly yet
							//we are completely reloading the columns, we don't know if it should
							//be gray yet, FillTabInfo will correct it later
							COLORREF clr = (0x00B0B0EE);
							pCol->PutBackColor(clr);
							pCol->PutBackColorSel(LightenSelColor(clr));
							pCol->PutForeColorSel(RGB(0,0,0));

							m_mapColIndexToStoredWidth.SetAt(nIndex, nActualWidth);
							nIndex++;
						}
					}
				}
			}

			prs->MoveNext();
		}

		// now set up our other insurance column with the stored width we pulled above
		m_mapColIndexToColType.SetAt(nIndex, btcInsurance);
		m_nOtherInsuranceColID = nIndex;
		m_mapColIndexToStoredWidth.SetAt(nIndex, nOtherInsActualWidth);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nOtherInsuranceColID, _T("OtherInsResp"), _T("Other Resp"), nOtherInsActualWidth, csVisible)))->FieldType = cftTextSingleLine;
		nIndex++;

		VerifyInsuranceColumns(nIndex);

	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2010-10-28) - PLID 28773 - Saves the new column size after the resize is completed, this function will also reinsert values to the tables if they are missing
void CFinancialDlg::SaveColumnWidths()
{
	try{
		long nColCount = m_List->GetColumnCount();
		long nUserID = GetCurrentUserID();

		CString strSqlBatch = BeginSqlBatch();
		CNxParamSqlArray args;

		// (j.dinatale 2010-11-17) - PLID 28773 - need this variable for parts of the query
		AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nNextOrderIndex INT");

		// (j.jones 2011-04-27 13:17) - PLID 43449 - ChargeAllowable defaults to not visible,
		// in the process of handling it I fixed how we handle all visible columns by default,
		// since most columns default to invisible
		CArray<long, long> aryVisibleColumnDefaults;
		aryVisibleColumnDefaults.Add((long)btcProvider);
		aryVisibleColumnDefaults.Add((long)btcNoteIcon);
		aryVisibleColumnDefaults.Add((long)btcDiscountIcon);
		aryVisibleColumnDefaults.Add((long)btcOnHoldIcon);	// (a.wilson 2014-07-24 11:22) - PLID 63015

		// if we are remembering our column widths, we can go ahead and try and save the new width for each column
		// if for some reason, we didnt update a record, well then we should likely insert it at the next possible order index so that way the width gets saved
		// and so it fixes any issues we have in data
		for(int nCol = 0; nCol < nColCount; nCol++){
			long nColumnType = -1;

			// attempt to check our mapping
			if(m_mapColIndexToColType.Lookup(nCol, nColumnType)){
				IColumnSettingsPtr pCol = m_List->GetColumn(nCol);
				long nWidth = -1;

				if(!m_mapColIndexToStoredWidth.Lookup(nCol, nWidth)){
					ASSERT(FALSE);
					nWidth = pCol->GetStoredWidth();
				}

				// if its an insurance column, we need to handle its saving slightly differently
				if(nColumnType == btcInsurance){
					if(nCol == m_nPrimInsColID){
						AddParamStatementToSqlBatch(strSqlBatch, args, 
							"UPDATE RespTypeColumnWidthsT SET StoredWidth = {INT} WHERE RespTypeID = {INT} AND UserID = {INT} \r\n"
							"IF @@ROWCOUNT = 0 \r\n"
							"	INSERT INTO RespTypeColumnWidthsT(RespTypeID, StoredWidth, UserID) VALUES ({INT}, {INT}, {INT})\r\n",
							nWidth, 1, nUserID, 1, nWidth, nUserID);
					}else{
						if(nCol == m_nSecInsColID){
							AddParamStatementToSqlBatch(strSqlBatch, args, 
								"UPDATE RespTypeColumnWidthsT SET StoredWidth = {INT} WHERE RespTypeID = {INT} AND UserID = {INT} \r\n"
								"IF @@ROWCOUNT = 0 \r\n"
								"	INSERT INTO RespTypeColumnWidthsT(RespTypeID, StoredWidth, UserID) VALUES ({INT}, {INT}, {INT})\r\n",
								nWidth, 2, nUserID, 2, nWidth, nUserID);
						}else{
							if(nCol == m_nOtherInsuranceColID){
								AddParamStatementToSqlBatch(strSqlBatch, args, 
									"UPDATE RespTypeColumnWidthsT SET StoredWidth = {INT} WHERE RespTypeID = {INT} AND UserID = {INT} \r\n"
									"IF @@ROWCOUNT = 0 \r\n"
									"	INSERT INTO RespTypeColumnWidthsT(RespTypeID, StoredWidth, UserID) VALUES ({INT}, {INT}, {INT})\r\n",
									nWidth, -3, nUserID, -3, nWidth, nUserID);
							}else{
								if(nCol ==  m_nPatientBalanceColID){
									AddParamStatementToSqlBatch(strSqlBatch, args, 
										"UPDATE RespTypeColumnWidthsT SET StoredWidth = {INT} WHERE RespTypeID = {INT} AND UserID = {INT} \r\n"
										"IF @@ROWCOUNT = 0 \r\n"
										"	INSERT INTO RespTypeColumnWidthsT(RespTypeID, StoredWidth, UserID) VALUES ({INT}, {INT}, {INT})\r\n",
										nWidth, -2, nUserID, -2, nWidth, nUserID);
								}else{
									long nRespID = -1;
									if(m_mapColumnIndexToRespTypeID.Lookup(nCol, nRespID)){
										AddParamStatementToSqlBatch(strSqlBatch, args, 
											"UPDATE RespTypeColumnWidthsT SET StoredWidth = {INT} WHERE RespTypeID = {INT} AND UserID = {INT} \r\n"
											"IF @@ROWCOUNT = 0 \r\n"
											"	INSERT INTO RespTypeColumnWidthsT(RespTypeID, StoredWidth, UserID) VALUES ({INT}, {INT}, {INT})\r\n",
											nWidth, nRespID, nUserID, nRespID, nWidth, nUserID);
									}
								}
							}
						}
					}
				}
				else{
					// (j.dinatale 2010-11-17) - PLID 28773 - tweaked this so the selection of the Max orderindex was not a subquery
					// otherwise, its a regular column, save its width accordingly
					AddParamStatementToSqlBatch(strSqlBatch, args, 
						"UPDATE BillingColumnsT SET StoredWidth = {INT} WHERE ColumnType = {INT} AND UserID = {INT} \r\n"
						"IF @@ROWCOUNT = 0 BEGIN \r\n"
						"	SET @nNextOrderIndex = (SELECT COALESCE(MAX(OrderIndex), 0) + 1 FROM BillingColumnsT WHERE UserID = {INT}) \r\n"
						"	INSERT INTO BillingColumnsT(StoredWidth, ColumnType, UserID, OrderIndex, Visible) VALUES ({INT}, {INT}, {INT}, @nNextOrderIndex, "
						"	CASE WHEN {INT} IN ({INTARRAY}) THEN 1 ELSE 0 END) END\r\n",
						nWidth, nColumnType, nUserID, nUserID, nWidth, nColumnType, nUserID, nColumnType, aryVisibleColumnDefaults);
				}
			}
		}

		// (j.dinatale 2010-11-17) - PLID 28773 - tweaked this so the selection of the Max orderindex was not a subquery
		// (j.jones 2015-04-06 12:08) - PLID 64944 - replaced the hardcoded type with btcInsurance
		AddParamStatementToSqlBatch(strSqlBatch, args,
			"IF NOT EXISTS(SELECT 1 FROM BillingColumnsT WHERE ColumnType = {CONST_INT} AND UserID = {INT}) BEGIN \r\n"
			"	SET @nNextOrderIndex = (SELECT COALESCE(MAX(OrderIndex), 0) + 1 FROM BillingColumnsT WHERE UserID = {INT}) \r\n"
			"	INSERT INTO BillingColumnsT(OrderIndex, UserID, StoredWidth, Visible, ColumnType) VALUES \r\n"
			"	(@nNextOrderIndex, {INT}, -1, CASE WHEN {CONST_INT} IN ({INTARRAY}) THEN 1 ELSE 0 END, {CONST_INT}) END \r\n",
			btcInsurance, nUserID,
			nUserID,
			nUserID, btcInsurance, aryVisibleColumnDefaults, btcInsurance);

		ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2010-10-28) - PLID 28773 - Saves the new column size to a mapping after the column resize is completed
void CFinancialDlg::OnColumnResize(short nCol, BOOL bCommitted, long nOldWidth, long nNewWidth)
{
	try{
		if(m_bRememberColWidth){
			m_mapColIndexToStoredWidth.SetAt(nCol, nNewWidth);
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2010-10-29) - PLID 28773 - When the remember column width checkbox is clicked
void CFinancialDlg::OnRememberColWidth()
{
	// check the current state of the remember column width check box 
	bool bIsChecked = (((CButton *)GetDlgItem(IDC_BILLINGTAB_REMCOLWIDTH))->GetCheck() == BST_CHECKED);

	// if its different than the currently known state, go ahead and do our work
	if(bIsChecked != m_bRememberColWidth)
	{
		// store the new value, set the remote property for the user, and re-setup the dialog
		m_bRememberColWidth = bIsChecked;
		SetRemotePropertyInt("RememberBillingTabColumnWidths", bIsChecked ? 1 : 0, 0, GetCurrentUserName());

		// disable the view
		DisableFinancialScreen();

		// if we are going from remembering to not remembering, we need to save the current set up so it can be recalled later
		if(!m_bRememberColWidth){
			SaveColumnWidths();
		}

		// reconstruct our view
		SetUpColumns(true);
		UpdateView();

		// enable the view
		EnableFinancialScreen();
	}
}

void CFinancialDlg::EnsureRespTypeColWidthTSQLStruct()
{
	try{
		long nUserID = GetCurrentUserID();

		BEGIN_TRANS("BillingRespTypeColWidths"){
			ExecuteParamSql(
				// Update the RespTypeColumnWidthsT with the default info for this user
				"INSERT INTO RespTypeColumnWidthsT(RespTypeID, UserID, StoredWidth) VALUES (-2, {INT}, -1);  \r\n"
				"INSERT INTO RespTypeColumnWidthsT(RespTypeID, UserID, StoredWidth) VALUES (-3, {INT}, -1);  \r\n"
				"INSERT INTO RespTypeColumnWidthsT(RespTypeID, UserID, StoredWidth) SELECT ID, {INT}, -1 FROM RespTypeT WHERE ID <> -1;  \r\n",
				nUserID, nUserID, nUserID);
		}END_TRANS_CATCH_ALL("BillingRespTypeColWidths");
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2010-10-29) - PLID 28773 - This will set up the sql structure if for some reason it is necessary
void CFinancialDlg::EnsureBillColTSQLStructure()
{
	try{

		// (j.jones 2011-04-27 13:17) - PLID 43449 - ChargeAllowable defaults to not visible,
		// in the process of handling it I fixed how we handle all visible columns by default,
		// since most columns default to invisible
		CArray<long, long> aryVisibleColumnDefaults;
		aryVisibleColumnDefaults.Add((long)btcProvider);
		aryVisibleColumnDefaults.Add((long)btcNoteIcon);
		aryVisibleColumnDefaults.Add((long)btcDiscountIcon);
		aryVisibleColumnDefaults.Add((long)btcOnHoldIcon);	// (a.wilson 2014-07-24 11:22) - PLID 63015

		long nUserID = GetCurrentUserID();
		BEGIN_TRANS("BillingTabColumnSetUp"){
			// (j.jones 2011-04-27 13:05) - PLID 43449 - used btcMaxValue for building new columns
			ExecuteParamSql(
				"DECLARE @CurrCol int; \r\n"
				"SET @CurrCol = 8; \r\n\r\n"

				"INSERT INTO BillingColumnsT(UserID, ColumnType, OrderIndex, StoredWidth, Visible) VALUES ({INT}, 6, -1, -1, NULL); \r\n"
				"INSERT INTO BillingColumnsT(UserID, ColumnType, OrderIndex, StoredWidth, Visible) VALUES ({INT}, 7, -1, -1, NULL); \r\n"

				//Create the right number of columns in the table
				"WHILE @CurrCol < {INT} BEGIN \r\n"
				"INSERT INTO BillingColumnsT(UserID, ColumnType, OrderIndex, StoredWidth, Visible) "
				"	VALUES ({INT}, @CurrCol, @CurrCol - 8, -1, "
				"	CASE WHEN @CurrCol IN ({INTARRAY}) THEN 1 ELSE 0 END); \r\n"
				"SET @CurrCol = @CurrCol + 1; \r\n"
				"END \r\n\r\n",
				nUserID, nUserID,
				btcMaxValue, nUserID,
				aryVisibleColumnDefaults);
		}END_TRANS_CATCH_ALL("BillingTabColumnSetUp");
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2010-10-29) - PLID 28773 - need to handle column saving on destroy since close does not appear to get fired
void CFinancialDlg::OnDestroy()
{
	// if we are currently remembering widths, save them
	if(m_bRememberColWidth){
		SaveColumnWidths();
	}

	// (a.walling 2012-01-26 13:24) - PLID 47814 - Need to call base class when handling OnDestroy!
	__super::OnDestroy();
}

// (j.dinatale 2010-11-05) - PLID 39226 - Needed so that way the view can be reconstructed from outside the dialog, also have the option to force a save if necessary
// (j.jones 2013-07-12 16:50) - PLID 57550 - removed bForceSave
void CFinancialDlg::ReconstructView()
{
	// (j.jones 2015-08-19 08:52) - PLID 64931 - always clear the search before rebuilding our view
	ClearSearchResults();

	// (j.jones 2013-07-12 16:51) - PLID 57550 - removed saving column widths, this is a reload function, not a save function
	SetUpColumns(true);
	UpdateView();
}

// (j.dinatale 2010-11-01) - PLID 28773 - Verifies that patient resp. and the primary, secondary, and other insurance column indices are set so that way issues do not arise, 
//		more of a fail-safe than anything so that way the billing tab will run even if some of our data is corrupt
void CFinancialDlg::VerifyInsuranceColumns(short &nIndex)
{
	// ensure the patient balance column is in our list
	if(m_nPatientBalanceColID == -1){
		m_nPatientBalanceColID = nIndex;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nPatientBalanceColID, _T("PatResp"), _T("Pat. Resp"), 50, m_bRememberColWidth ? csVisible : csVisible|csWidthData)))->FieldType = cftTextSingleLine;
		m_mapColIndexToColType.SetAt(nIndex, btcInsurance);
		m_mapColIndexToStoredWidth.SetAt(nIndex, 50);
		nIndex++;
	}

	// ensure the primary insurance balance column is in our list
	if(m_nPrimInsColID == -1){
		m_nPrimInsColID = nIndex;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nPrimInsColID, _T("InsResp1"), _T("Primary"), 50, m_bRememberColWidth ? csVisible : csVisible|csWidthData)))->FieldType = cftTextSingleLine;
		m_mapColIndexToColType.SetAt(nIndex, btcInsurance);
		m_mapColIndexToStoredWidth.SetAt(nIndex, 50);
		nIndex++;
	}

	// ensure the secondary insurance balance column is in our list
	if(m_nSecInsColID == -1){
		m_nSecInsColID = nIndex;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nSecInsColID, _T("InsResp2"), _T("Secondary"), 50, m_bRememberColWidth ? csVisible : csVisible|csWidthData)))->FieldType = cftTextSingleLine;
		m_mapColIndexToColType.SetAt(nIndex, btcInsurance);
		m_mapColIndexToStoredWidth.SetAt(nIndex, 50);
		nIndex++;
	}

	// ensure the other insurance balance column is in our list
	if(m_nOtherInsuranceColID == -1){
		m_mapColIndexToColType.SetAt(nIndex, btcInsurance);
		m_nOtherInsuranceColID = nIndex;
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nOtherInsuranceColID, _T("OtherInsResp"), _T("Other Resp"), 50, m_bRememberColWidth ? csVisible : csVisible|csWidthData)))->FieldType = cftTextSingleLine;
		m_mapColIndexToStoredWidth.SetAt(nIndex, 50);
		nIndex++;
	}
}

// (j.dinatale 2010-11-01) - PLID 28773 - Verifies that all non insurance column indices are set so that way issues do not arise, more of a fail-safe than anything so that way
//		the billing tab will run even if some of our data is corrupt
void CFinancialDlg::VerifyNonInsuranceColumns(short &nIndex)
{
	long nVisflags = -1;
	long nWidthToUse = -1;

	if(m_nExpandCharColID == -1){
		m_nExpandCharColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcExpandChar);
		nVisflags = csVisible|csWidthPercent;
		nWidthToUse = 3;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
			nWidthToUse = 30;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nExpandCharColID, _T("ExpandChar"), _T(""), nWidthToUse, nVisflags)))->FieldType = cftBitmapBuiltIn;
		nIndex++;
	}

	if(m_nExpandSubCharColID == -1){
		m_nExpandSubCharColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcExpandSubChar);
		nVisflags = csVisible|csWidthPercent;
		nWidthToUse = 3;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
			nWidthToUse = 30;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nExpandSubCharColID, _T("ExpandSubChar"), _T(""), nWidthToUse, nVisflags)))->FieldType = cftBitmapBuiltIn;
		nIndex++;
	}

	if(m_nDateColID == -1){
		m_nDateColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcDate);
		nVisflags = csVisible|csWidthData;
		nWidthToUse = 50;

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nDateColID, _T("Date"), _T("Date"), nWidthToUse, nVisflags)))->FieldType = cftDateShort;
		nIndex++;
	}

	if(m_nNoteIconColID == -1){
		m_nNoteIconColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcNoteIcon);
		nWidthToUse = 3;
		nVisflags = csVisible|csWidthPercent;

		if(m_bRememberColWidth){
			nWidthToUse = 50;
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nNoteIconColID, _T("NoteIcon"), _T(""), nWidthToUse, nVisflags)))->FieldType = cftBitmapBuiltIn;
		nIndex++;
	}

	if(m_nDiscountIconColID == -1){
		m_nDiscountIconColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcDiscountIcon);
		nWidthToUse = 35; 
		nVisflags = csVisible|csFixedWidth;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nDiscountIconColID, _T("DiscountIcon"), _T("Disc."), nWidthToUse, nVisflags)))->FieldType = cftBitmapBuiltIn;
		nIndex++;
	}

	if (m_nOhHoldIconColID == -1) {
		m_nOhHoldIconColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcOnHoldIcon);
		nWidthToUse = 3;
		nVisflags = csVisible | csWidthPercent;

		if (m_bRememberColWidth){
			nWidthToUse = 50;
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nOhHoldIconColID, _T("OnHoldIcon"), _T(""), nWidthToUse, nVisflags)))->FieldType = cftBitmapBuiltIn;
		nIndex++;
	}

	if(m_nDescriptionColID == -1){
		m_nDescriptionColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcDescription);
		nWidthToUse = 300;

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nDescriptionColID, _T("Description"), _T("Description"), nWidthToUse, csVisible)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nChargeColID == -1){
		m_nChargeColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcCharge);
		nVisflags = csVisible|csWidthData;
		nWidthToUse = 50;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nChargeColID, _T("Charge"), _T("Charge"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	// (j.jones 2015-04-06 12:04) - PLID 64944 - added TotalPaymentAmount, which by default
	// is not shown, but is positioned between charge and payment
	if (m_nTotalPaymentAmountColID == -1){
		m_nTotalPaymentAmountColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcTotalPaymentAmount);
		//this is hidden by default, SetUpColumnByType will potentially show it later
		nWidthToUse = 0;
		nVisflags = csVisible; //|csWidthData;

		if (m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nTotalPaymentAmountColID, _T("TotalPaymentAmount"), _T("Total Amt."), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nPaymentColID == -1){
		m_nPaymentColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcPayment);
		nVisflags = csVisible|csWidthData;
		nWidthToUse = 58;

		if(m_bRememberColWidth) {
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nPaymentColID, _T("Payment"), _T("Payment"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nAdjustmentColID == -1){
		m_nAdjustmentColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcAdjustment);
		nVisflags = csVisible|csWidthData;
		nWidthToUse = 73;

		if(m_bRememberColWidth) {
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nAdjustmentColID, _T("Adjustment"), _T("Adjustment"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nRefundColID == -1){
		m_nRefundColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcRefund);
		nVisflags = csVisible|csWidthData;
		nWidthToUse = 50;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nRefundColID, _T("Refund"), _T("Refund"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nBalanceColID == -1){
		m_nBalanceColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcBalance);
		nVisflags = csVisible|csWidthData;
		nWidthToUse = 50;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nBalanceColID, _T("Balance"), _T("Balance"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nProviderColID == -1){
		m_nProviderColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcProvider);
		nWidthToUse = 50;
		nVisflags = csVisible|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nProviderColID, _T("Provider"), _T("Provider"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nLocationColID == -1){
		m_nLocationColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcLocation);
		nWidthToUse = 50;
		nVisflags = csVisible|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nLocationColID, _T("Location"), _T("Location"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nPOSNameColID == -1){
		m_nPOSNameColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcPOSName);
		nVisflags = csVisible|csWidthData;
		nWidthToUse = 50;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nPOSNameColID, _T("POSName"), _T("Place Of Service"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nPOSCodeColID == -1){
		m_nPOSCodeColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcPOSCode);
		nVisflags = csVisible|csWidthData;
		nWidthToUse = 50;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nPOSCodeColID, _T("POSName"), _T("POS Code"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nInputDateColID == -1){
		m_nInputDateColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcInputDate);
		nWidthToUse = 50;
		nVisflags = csVisible|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nInputDateColID, _T("InputDate"), _T("Input Date"), nWidthToUse, nVisflags)))->FieldType = cftDateShort;
		nIndex++;
	}

	if(m_nCreatedByColID == -1){
		m_nCreatedByColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcCreatedBy);
		nWidthToUse = 50;
		nVisflags = csVisible|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nCreatedByColID, _T("CreatedBy"), _T("Created By"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nFirstChargeDateColID == -1){
		m_nFirstChargeDateColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcFirstChargeDate);
		nWidthToUse = 100;
		nVisflags = csVisible|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nFirstChargeDateColID, _T("FirstChargeDate"), _T("First Charge Date"), nWidthToUse, nVisflags)))->FieldType = cftDateShort;
		nIndex++;
	}

	if(m_nDiagCode1ColID == -1){
		m_nDiagCode1ColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcDiagCode1);
		nWidthToUse = 80;
		nVisflags = csVisible|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nDiagCode1ColID, _T("DiagCode1"), _T("Diag. Code 1"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nDiagCode1WithNameColID == -1){
		m_nDiagCode1WithNameColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcDiagCode1WithName);
		nWidthToUse = 80;
		nVisflags = csVisible|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nDiagCode1WithNameColID, _T("DiagCode1WithName"), _T("Diag. Code 1"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nDiagCodeListColID == -1){
		m_nDiagCodeListColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcDiagCodeList);
		nWidthToUse = 80;
		nVisflags = csVisible|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nDiagCodeListColID, _T("DiagCodeList"), _T("Diag. Codes"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	// (j.jones 2011-04-27 11:19) - PLID 43449 - renamed Allowable to FeeSchedAllowable
	if(m_nFeeSchedAllowableColID == -1){
		m_nFeeSchedAllowableColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcFeeSchedAllowable);
		//this is hidden by default, SetUpColumnByType will potentially show it later
		nWidthToUse = 0;
		nVisflags = csVisible; //|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nFeeSchedAllowableColID, _T("FeeSchedAllowable"), _T("Fee Sched. Allowable"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	// (j.jones 2011-04-27 11:19) - PLID 43449 - added ChargeAllowable
	if(m_nChargeAllowableColID == -1){
		m_nChargeAllowableColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcChargeAllowable);
		//this is hidden by default, SetUpColumnByType will potentially show it later
		nWidthToUse = 0;
		nVisflags = csVisible; //|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nChargeAllowableColID, _T("ChargeAllowable"), _T("EOB Allowable"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	// (j.jones 2011-12-20 11:37) - PLID 47119 - added deductible and coinsurance
	if(m_nChargeDeductibleColID == -1){
		m_nChargeDeductibleColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcChargeDeductible);
		//this is hidden by default, SetUpColumnByType will potentially show it later
		nWidthToUse = 0;
		nVisflags = csVisible; //|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nChargeDeductibleColID, _T("ChargeDeductible"), _T("EOB Deductible"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}
	if(m_nChargeCoinsuranceColID == -1){
		m_nChargeCoinsuranceColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcChargeCoinsurance);
		//this is hidden by default, SetUpColumnByType will potentially show it later
		nWidthToUse = 0;
		nVisflags = csVisible; //|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nChargeCoinsuranceColID, _T("ChargeCoinsurance"), _T("EOB Coinsurance"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	if(m_nBillIDColID == -1){
		m_nBillIDColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcBillID);
		nVisflags = csVisible|csWidthData;
		nWidthToUse = 40;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nBillIDColID, _T("BillID"), _T("Bill ID"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}

	// (j.jones 2013-07-11 12:20) - PLID 57505 - added invoice number
	if(m_nInvoiceNumberColID == -1){
		m_nInvoiceNumberColID = nIndex;
		m_mapColIndexToColType.SetAt(nIndex, btcInvoiceNumber);
		//this is hidden by default, SetUpColumnByType will potentially show it later
		nWidthToUse = 0;
		nVisflags = csVisible; //|csWidthData;

		if(m_bRememberColWidth){
			nVisflags = csVisible;
		}

		m_mapColIndexToStoredWidth.SetAt(nIndex, nWidthToUse);
		IColumnSettingsPtr(m_List->GetColumn(m_List->InsertColumn(m_nInvoiceNumberColID, _T("InvoiceID"), _T("Invoice #"), nWidthToUse, nVisflags)))->FieldType = cftTextSingleLine;
		nIndex++;
	}
}

// (j.dinatale 2010-11-01) - PLID 28773 - clears all member variables reflecting the index of a column and sets their values to -1
void CFinancialDlg::ClearColumnIndices()
{
	m_nOtherInsuranceColID = -1;
	m_nBalanceColID = -1;
	m_nProviderColID = -1;
	m_nLocationColID = -1;
	m_nPOSNameColID = -1;
	m_nPOSCodeColID = -1;
	m_nInputDateColID = -1;
	m_nCreatedByColID = -1;
	m_nFirstChargeDateColID = -1;
	m_nDiagCode1ColID = -1;
	m_nDiagCode1WithNameColID = -1;
	m_nDiagCodeListColID = -1;
	m_nFeeSchedAllowableColID = -1;
	m_nChargeAllowableColID = -1;
	m_nChargeDeductibleColID = -1;
	m_nChargeCoinsuranceColID = -1;
	m_nInvoiceNumberColID = -1;
	m_nTotalPaymentAmountColID = -1;
	m_nMaxColID = -1;
	m_nBillIDColID = -1;
	m_nExpandCharColID = -1;
	m_nExpandSubCharColID = -1;
	m_nDateColID = -1;
	m_nNoteIconColID = -1;
	m_nDiscountIconColID = -1;
	m_nDescriptionColID = -1;
	m_nChargeColID = -1;
	m_nPaymentColID = -1;
	m_nAdjustmentColID = -1;
	m_nRefundColID = -1;
	m_nPatientBalanceColID = -1;
	m_nPrimInsColID = -1;
	m_nSecInsColID = -1;
}

void CFinancialDlg::OnGlassesOrderHistory()
{
	try {
		//TES 6/20/2011 - PLID 43700 - Just pop up the dialog (assuming they have the Glasses Order license).
		if(g_pLicense->CheckForLicense(CLicense::lcGlassesOrders, CLicense::cflrUse)) {
			CGlassesOrderHistoryDlg dlg(this);
			dlg.m_nPatientID = m_iCurPatient;
			dlg.DoModal();
		}	
	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-10-28 14:04) - PLID 45462 - added function to recursively undo applied payments,
// anytime we undo a correction that undoes a payment correction, we have to check for cases where
// that source payment has a partial apply, recursively
BOOL CFinancialDlg::UndoAppliedCorrections(CFinancialCorrection &finCor, long nOriginalPaymentIDToCheck,
										   CString strOriginalCorrectionsTableToUndo, long nOriginalCorrectionID_ToUndo,
										   CString strOriginalCorrectionUndoType)
{
	//exceptions are handled by the caller

	//this function is called whenever we are undoing a payment correction,
	//though that payment undo may come from undoing a bill or charge correction
	//that had applied payments that were also corrected

	//Example: Charge (A) had Payment (B) partially applied. The rest of the payment was Refunded (C).
	//Correcting Charge (A) auto-corrected Payment (B) which also auto-corrected Refund (C).
	//Therefore, undoing the correction for Charge (A) undoes the correction for the Payment (B).
	//The normal undo code handles this. But this function needs to take in Payment (B) and undo 
	//corrections for all applies (C) applied to the *original* payment ID. They wouldn't exist
	//unless we auto-corrected them.
	//Because of partial payments, this could involve more levels of depth than this Payment (B) and
	//Refund (C) example. Therefore, this is a recursive function.

	//Parameters:
	//CFinancialCorrection &finCor - the correction batch that will undo everything at once,
	//								the parent payment, and possibly parent charge and bill, will be added last
	//long nOriginalPaymentIDToCheck - the immediate parent payment, this function is looking for things applied to it,
	//								this ID is the *Original* payment ID
	//CString strOriginalCorrectionsTableToUndo - the table for the master correction being undone, either BillCorrectionsT or LineItemCorrectionsT
	//long nOriginalCorrectionID_ToUndo - the ID of the master correction being undone, either a BillCorrectionsT.ID or a LineItemCorrectionsT.ID
	//CString strOriginalCorrectionUndoType - the friendly name of the master correction that is being undone,
	//								used for warning messages if we cannot succeed

	//now, find applies to our given payment ID that are corrected
	//(theoretically there should not be any applies to this ID that are not corrected)
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), FormatString("SELECT LineItemCorrectionsT.ID, LineItemCorrectionsT.InputDate, OriginalLineItemID, VoidingLineItemID, NewLineItemID, "
		"LineItemT.Type "
		"FROM LineItemCorrectionsT "
		"INNER JOIN LineItemT ON LineItemCorrectionsT.OriginalLineItemID = LineItemT.ID "
		"WHERE OriginalLineItemID IN (SELECT SourceID FROM AppliesT WHERE DestID = {INT}) "
		"AND LineItemCorrectionsT.InputDate >= (SELECT InputDate FROM %s WHERE ID = {INT})", strOriginalCorrectionsTableToUndo),
		nOriginalPaymentIDToCheck, nOriginalCorrectionID_ToUndo);
	while(!rs->eof) {
		long nOtherCorrectionID_ToUndo = VarLong(rs->Fields->Item["ID"]->Value);
		COleDateTime dtOtherCorrectionInputDate = VarDateTime(rs->Fields->Item["InputDate"]->Value);
		long nOtherCorrectionOriginalID = VarLong(rs->Fields->Item["OriginalLineItemID"]->Value);
		long nOtherCorrectionVoidingID = VarLong(rs->Fields->Item["VoidingLineItemID"]->Value);
		long nOtherCorrectionNewID = VarLong(rs->Fields->Item["NewLineItemID"]->Value, -1);	//could be null
		long nOtherCorrectionOriginalType = VarLong(rs->Fields->Item["Type"]->Value);

		CString strOtherType = "payment";
		if(nOtherCorrectionOriginalType == LineItem::Adjustment) {
			strOtherType = "adjustment";
		}
		else if(nOtherCorrectionOriginalType == LineItem::Refund) {
			strOtherType = "refund";
		}

		//we do not need to check permissions or the close status, but we do need to see if the
		//correction's new item is itself corrected, and if so, abort
		if(nOtherCorrectionNewID != -1) {
			LineItemCorrectionStatus licsStatus = GetLineItemCorrectionStatus(nOtherCorrectionNewID);
			if(licsStatus == licsOriginal) {
				CString strWarning;
				//these messages aren't pretty, we don't know how deep our recursion went, so all we can say
				//is the name of what correction that are intentionally undoing, and what correction cannot
				//be undone
				strWarning.Format("When this %s was corrected, everything applied to it was also corrected. "
					"A new %s from that correction has already been corrected itself. "
					"You cannot undo this %s correction until the related %s's correction is removed first.",
					strOriginalCorrectionUndoType, strOtherType, strOriginalCorrectionUndoType, strOtherType);
				AfxMessageBox(strWarning);
				return FALSE;
			}

			//cannot undo if the applied item has something applied to it that is correctedy
			if(DoesLineItemHaveOriginalOrVoidApplies(nOtherCorrectionNewID)) {
				CString strWarning;
				//these messages aren't pretty, we don't know how deep our recursion went, so all we can say
				//is the name of what correction that are intentionally undoing, and what correction cannot
				//be undone
				strWarning.Format("When this %s was corrected, everything applied to it was also corrected. "
					"A new %s from that correction has items applied to it that have also been corrected. "
					"You cannot undo this %s correction until the related %s's corrected applies are removed first.",
					strOriginalCorrectionUndoType, strOtherType, strOriginalCorrectionUndoType, strOtherType);
				AfxMessageBox(strWarning);
				return FALSE;
			}
		}

		//is our original payment also applied to a charge that has since been corrected separately?
		CString strAppliedToType = "";
		if(IsPaymentAppliedToNewCorrection(nOtherCorrectionOriginalID, strOriginalCorrectionsTableToUndo, nOriginalCorrectionID_ToUndo, strOtherType)) {
			CString strWarning;
			strWarning.Format("When this %s was corrected, an applied %s was also corrected. "
				"The %s from that correction is also partially applied to a %s that was corrected later. "
				"You cannot undo this %s correction until the %s that this %s is applied to has also had its corrections undone.",
				strOriginalCorrectionUndoType, strOtherType, strOtherType, strAppliedToType, strOriginalCorrectionUndoType, strAppliedToType, strOtherType);
			AfxMessageBox(strWarning);
			return FALSE;
		}

		//recursively undo any payments applied to this one
		if(!UndoAppliedCorrections(finCor, nOtherCorrectionOriginalID, strOriginalCorrectionsTableToUndo, nOriginalCorrectionID_ToUndo, strOriginalCorrectionsTableToUndo)){
			return FALSE;
		}

		//undo this applied correction
		finCor.AddCorrectionToUndo(cutUndoPayment, nOtherCorrectionID_ToUndo, dtOtherCorrectionInputDate, nOtherCorrectionOriginalID, nOtherCorrectionVoidingID, nOtherCorrectionNewID, m_iCurPatient);

		rs->MoveNext();
	}
	rs->Close();

	return TRUE;
}

// (b.spivey, September 11, 2014) - PLID 63652 - If the payment is linked return true
bool CFinancialDlg::IsLinkedToLockbox(long nPaymentID)
{

	if (ReturnsRecordsParam(
		R"(SELECT TOP 1 LBT.ID 
		FROM LockboxBatchT LBT
		LEFT JOIN LockboxPaymentT LBPT ON LBT.ID = LBPT.LockboxBatchID
		LEFT JOIN LockboxPaymentMapT LBPMT ON LBPT.ID = LBPMT.LockboxPaymentID
		LEFT JOIN PaymentsT PT ON LBPMT.PaymentID = PT.ID
		LEFT JOIN LineItemT LIT ON PT.ID = LIT.ID
		WHERE LIT.Deleted = 0 AND LIT.ID = { INT }
		)", nPaymentID)) {
		return true;
	}

	return false;
}

// (j.jones 2015-02-23 12:48) - PLID 64934 - added checkbox to hide voided items
void CFinancialDlg::OnCheckHideVoidedItems()
{
	try {

		long nValue = m_checkHideVoidedItems.GetCheck() ? 1 : 0;
		SetRemotePropertyInt("BillingTabHideVoidedItems", nValue, 0, GetCurrentUserName());

		//redraw the screen
		QuickRefresh();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-02-25 09:45) - PLID 64939 - made one modular function to open Apply Manager
void CFinancialDlg::OpenApplyManager(long nBillID, long nChargeID, long nPayAdjRefID, long nApplyID)
{
	try {

		//if this is an apply to a payment, make sure we use the applied item's payment ID
		if (nPayAdjRefID != -1 && nApplyID != -1) {
			_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT SourceID FROM AppliesT WHERE ID = {INT}", nApplyID);
			if (!rs->eof) {
				//show this applied item in apply manager instead
				nPayAdjRefID = VarLong(rs->Fields->Item["SourceID"]->Value);
			}
		}

		//the default values for "none selected" are actually -2,
		//so if the caller passed in -1 for any record, change it to -2
		if (nBillID == -1) {
			nBillID = -2;
		}
		if (nChargeID == -1) {
			nChargeID = -2;
		}
		if (nPayAdjRefID == -1) {
			nPayAdjRefID = -2;
		}

		CApplyManagerDlg dlg(this);
		dlg.m_iBillID = nBillID;
		dlg.m_iChargeID = nChargeID;
		dlg.m_iPayID = nPayAdjRefID;
		dlg.m_GuarantorID1 = m_GuarantorID1;
		dlg.m_GuarantorID2 = m_GuarantorID2;
		dlg.DoModal();

		//always call QuickRefresh to reflect any unapplies that may have occurred
		QuickRefresh();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-16 14:20) - PLID 64926 - added ability to search the billing tab
void CFinancialDlg::OnBtnSearchBilling()
{
	try {

		if (m_dlgSearchBillingTab.GetSafeHwnd() == NULL)
		{
			m_dlgSearchBillingTab.Create(CSearchBillingTabDlg::IDD, this);
			
			//move the window to be just to the left of this button
			CRect rcSearchBtn;
			GetDlgItem(IDC_SEARCH_BILLING)->GetWindowRect(rcSearchBtn);
			CRect rcDialog;
			m_dlgSearchBillingTab.GetWindowRect(rcDialog);
			long nDialogWidth = rcDialog.Width();
			long nDialogHeight = rcDialog.Height();
			//move the dialog such that it is bottom-aligned and just to the left of the search button
			rcDialog.SetRect(rcSearchBtn.left - 5 - nDialogWidth, rcSearchBtn.bottom - nDialogHeight, rcSearchBtn.left - 5, rcSearchBtn.bottom);
			ASSERT(nDialogWidth == rcDialog.Width());
			ASSERT(nDialogHeight = rcDialog.Height());
			m_dlgSearchBillingTab.MoveWindow(rcDialog);
			
			m_dlgSearchBillingTab.ShowWindow(SW_SHOW);
		}
		else {
			//if the dialog exists, let this button toggle it as shown or hidden
			if (!m_dlgSearchBillingTab.IsWindowVisible()) {
				m_dlgSearchBillingTab.ShowWindow(SW_SHOW);
			}
			else {
				m_dlgSearchBillingTab.ShowWindow(SW_HIDE);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-16 17:06) - PLID 64927 - one function to expand them all
void CFinancialDlg::ExpandAllRows()
{
	try {

		if (m_List->GetRowCount() == 0) {
			//nothing to expand
			return;
		}

		CWaitCursor pWait;

		DisableFinancialScreen();

		//cache the line id that should be selected
		long nSelectedLineID = -1;
		long nTopRowLineID = -1;
		{
			IRowSettingsPtr pRow = m_List->GetCurSel();
			if (pRow) {
				nSelectedLineID = VarLong(pRow->GetValue(btcLineID), -1);
			}
			IRowSettingsPtr pTopRow = m_List->GetTopRow();
			if (pTopRow) {
				nTopRowLineID = VarLong(pTopRow->GetValue(btcLineID), -1);
				if (nSelectedLineID == -1) {
					nSelectedLineID = nTopRowLineID;
				}
			}
		}

		//expand every row if it is not already expanded
		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while (pRow) {

			// Make sure an arrow exists
			_variant_t v = pRow->GetValue(m_nExpandCharColID);
			if (VarString(v, "") != "BITMAP:DOWNARROW") {
				pRow = pRow->GetNextRow();
				continue;
			}

			long nLineID = VarLong(pRow->GetValue(btcLineID), -1);

			// See if we're expanding a bill or a payment/adjustment/refund
			CString strLineType = VarString(pRow->GetValue(btcLineType));
			if (strLineType == "Bill" || strLineType == "Charge") {
				// Expand the bill
				long nBillID = VarLong(pRow->GetValue(m_nBillIDColID), -1);
				ExpandBill(nBillID, nLineID);
			}
			else {
				// Expand the payment
				long nPaymentID = VarLong(pRow->GetValue(btcPayID), -1);
				ExpandPayment(nPaymentID);
			}

			//we have to go back to the line ID we were on, then keep advancing
			pRow = m_List->FindByColumn(btcLineID, (long)nLineID, NULL, FALSE);
			if (pRow) {
				pRow = pRow->GetNextRow();
			}
		}

		//now expand all charges
		pRow = m_List->GetFirstRow();
		while (pRow) {

			// Make sure an arrow exists
			_variant_t v = pRow->GetValue(m_nExpandSubCharColID);
			if (VarString(v, "") != "BITMAP:DOWNARROW") {
				pRow = pRow->GetNextRow();
				continue;
			}

			long nLineID = VarLong(pRow->GetValue(btcLineID), -1);
			long nBillID = VarLong(pRow->GetValue(m_nBillIDColID), -1);
			long nChargeID = VarLong(pRow->GetValue(btcChargeID), -1);

			//skip this row if we don't have these three IDs
			if (nBillID == -1 || nChargeID == -1 || nLineID == -1) {
				pRow = pRow->GetNextRow();
				continue;
			}

			//you will not get to this point unless all the data is valid
			ExpandCharge(nChargeID, nBillID, nLineID);

			//we have to go back to the line ID we were on, then keep advancing
			pRow = m_List->FindByColumn(btcLineID, (long)nLineID, NULL, FALSE);
			if (pRow) {
				pRow = pRow->GetNextRow();
			}
		}

		//EnableFinancialScreen will re-select the original line ID / top row
		m_SavedRow_LineID = nSelectedLineID;
		m_SavedTopRow_LineID = nTopRowLineID;

		EnableFinancialScreen();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-16 16:14) - PLID 64927 - Called by the search dialog to search
// visible cells by the provided search term.
// Returns the information for all matching cells.
BillingTabSearchResults CFinancialDlg::SearchBillingTab(CiString strSearchTerm)
{
	//this function uses CiStrings to force case insensitivity

	BillingTabSearchResults aryResults;

	try {

		//if the search term is empty, do nothing
		strSearchTerm.Trim();
		if (strSearchTerm.IsEmpty()) {
			//the calling code should not have allowed this
			ASSERT(FALSE);
			return aryResults;
		}

		//if we have nothing in our list, don't bother searching
		if (m_List->GetRowCount() == 0) {
			return aryResults;
		}

		//first, expand every row if it is not already expanded
		ExpandAllRows();

		CWaitCursor pWait;

		//Now search all visible rows and columns for our text,
		//case-insensitively. We do not break up words by spaces.
		//The search term could exist inside a field.
		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while (pRow) {
			for (int i = 0; i < m_nMaxColID; i++) {
				IColumnSettingsPtr pCol = m_List->GetColumn(i);
				//ignore hidden columns
				if (pCol->GetStoredWidth() <= 0) {
					continue;
				}
				//ignore icon columns
				if (i == m_nExpandCharColID || i == m_nExpandSubCharColID
					|| i == m_nNoteIconColID || i == m_nDiscountIconColID || i == m_nOhHoldIconColID) {
					continue;
				}

				//now compare this cell's value
				_variant_t var = pRow->GetValue(i);
				if (var.vt == VT_NULL || var.vt == VT_EMPTY) {
					continue;
				}

				CiString strCellValue = "";

				//string
				if (var.vt == VT_BSTR) {
					strCellValue = VarString(var);
				}
				//money
				else if (var.vt == VT_CY) {
					//did the user type in a potentially valid currency?
					COleCurrency cySearch;
					if (!cySearch.ParseCurrency(strSearchTerm)) {
						//nope - they didn't intend to search for money (or typed in garbage)
						continue;
					}

					//if they typed in money, we assume they typed in the same format
					//their regional settings are displaying currency
					strCellValue = FormatCurrencyForInterface(VarCurrency(var));
				}
				//date
				else if (var.vt == VT_DATE) {
					//did the user type in a potentially valid date?
					COleDateTime dtDate;
					if (!dtDate.ParseDateTime(strSearchTerm)) {
						//nope - but did they type in a number?
						if (atoi(strSearchTerm) <= 0) {
							//not a positive number (and 0 is pointless)
							//so they didn't type in a year, like 2015
							continue;
						}
					}

					//if they typed in a date, we assume they typed in the same format
					//their regional settings are displaying dates
					//also we will only compare on a MM/DD/YYYY search, times are not searched
					strCellValue = FormatDateTimeForInterface(VarDateTime(var), NULL, dtoDate);
				}
				//integer or double
				else if (var.vt == VT_I4 || var.vt == VT_R8) {
					strCellValue = AsString(var);
				}
				else {
					//unsupported type: use AsString to get the data,
					//but we should add special handling for this unknown type
					ASSERT(FALSE);
					strCellValue = AsString(var);
				}

				if (strCellValue.Find(strSearchTerm) != -1) {
					//the cell matches
					BillingTabSearchResultPtr pResult = make_shared<BillingTabSearchResult>();
					pResult->nLineID = VarLong(pRow->GetValue(btcLineID));	//can't be null
					pResult->nBillID = VarLong(pRow->GetValue(m_nBillIDColID), -1);
					pResult->nChargeID = VarLong(pRow->GetValue(btcChargeID), -1);
					pResult->nPaymentID = VarLong(pRow->GetValue(btcPayID), -1);
					pResult->nPayToPayID = VarLong(pRow->GetValue(btcPayToPayID), -1);
					pResult->varValue = pRow->GetValue(i);
					pResult->nColumnIndex = i;
					aryResults.push_back(pResult);
				}
			}
			pRow = pRow->GetNextRow();
		}

		return aryResults;

	}NxCatchAll(__FUNCTION__);

	//clear our results if we got an error
	aryResults.clear();
	return aryResults;
}

// (j.jones 2015-03-20 08:37) - PLID 64931 - Given a search result,
// find the cell the result is pointing to. Returns the found row if successful.
// This function does not select or highlight the cell.
IRowSettingsPtr CFinancialDlg::FindSearchResult(BillingTabSearchResultPtr pResult)
{
	try {

		// (j.jones 2015-03-20 08:41) - PLID 64929 - find the search result
		//find our line ID
		IRowSettingsPtr pRow = m_List->FindByColumn(btcLineID, pResult->nLineID, m_List->GetFirstRow(), VARIANT_FALSE);
		bool bMatched = false;
		if (pRow) {

			//tentative match
			bMatched = true;

			//make sure the line id matches our result info
			if (pResult->nBillID != -1) {
				long nBillID = VarLong(pRow->GetValue(m_nBillIDColID), -1);
				if (nBillID != pResult->nBillID) {
					//the line ID is no longer accurate
					bMatched = false;
				}
			}
			if (bMatched && pResult->nChargeID != -1) {
				long nChargeID = VarLong(pRow->GetValue(btcChargeID), -1);
				if (nChargeID != pResult->nChargeID) {
					//the line ID is no longer accurate
					bMatched = false;
				}
			}
			if (bMatched && pResult->nPaymentID != -1) {
				long nPaymentID = VarLong(pRow->GetValue(btcPayID), -1);
				if (nPaymentID != pResult->nPaymentID) {
					//the line ID is no longer accurate
					bMatched = false;
				}
			}
			if (bMatched && pResult->nPayToPayID != -1) {
				long nPayToPayID = VarLong(pRow->GetValue(btcPayToPayID), -1);
				if (nPayToPayID != pResult->nPayToPayID) {
					//the line ID is no longer accurate
					bMatched = false;
				}
			}

			if (pResult->nColumnIndex < 0 || pResult->nColumnIndex >= m_List->GetColumnCount()) {
				//how did this happen?
				ASSERT(FALSE);
				bMatched = false;
			}

			if (bMatched && pResult->varValue != pRow->GetValue(pResult->nColumnIndex)) {
				//the cell still exists, but the value changed
				bMatched = false;
			}
		}

		if (!bMatched) {
			//we haven't found this cell by LineID, which is bad if
			//called from HighlightSearchResult(), but fine if called
			//from RevalidateSearchResults()
			return NULL;
		}

		//if we're still here, return this row
		if (pRow == NULL) {
			//how is this possible?
			ASSERT(FALSE);
		}
		return pRow;

	}NxCatchAll(__FUNCTION__);

	return NULL;
}

// (j.jones 2015-03-17 12:24) - PLID 64929 - Given a search result, highlight the cell
// the result is pointing to. Returns true if successful.
bool CFinancialDlg::HighlightSearchResult(BillingTabSearchResultPtr pResult)
{
	try {

		//revert the color of the last result cell, if any
		RevertSearchResultCellColor();

		// (j.jones 2015-03-20 08:38) - PLID 64931 - moved the finding code to FindSearchResult
		IRowSettingsPtr pRow = FindSearchResult(pResult);
		bool bMatched = false;
		if (pRow) {
			bMatched = true;
		}

		if (!bMatched) {
			//we haven't found this cell, presumably something changed
			//and the search wasn't cleared - why not?
			ASSERT(FALSE);
			return false;
		}

		if (bMatched && pRow != NULL) {
			//highlight this row
			m_List->PutCurSel(pRow);

			//ensure the column exists
			if (pResult->nColumnIndex < 0 || pResult->nColumnIndex >= m_List->GetColumnCount()) {
				//how did this happen?
				ASSERT(FALSE);
				return true;
			}

			//is the column visible?
			if (pResult->nColumnIndex < m_nBalanceColID) {
				//if the column is to the left of the balance column, first
				//try to show the very first column
				//such that we are fully scrolled to the left
				m_List->EnsureColumnInView(m_nExpandCharColID);
			}
			m_List->EnsureColumnInView(pResult->nColumnIndex);

			
			//cache the cell's back & fore color: both sel and unsel
			m_clrCurSearchResultCellBackgroundColor = pRow->GetCellBackColor(pResult->nColumnIndex);
			m_clrCurSearchResultCellForegroundColor = pRow->GetCellForeColor(pResult->nColumnIndex);
			m_clrCurSearchResultCellBackgroundSelColor = pRow->GetCellBackColorSel(pResult->nColumnIndex);
			m_clrCurSearchResultCellForegroundSelColor = pRow->GetCellForeColorSel(pResult->nColumnIndex);
			m_pCurSearchResultsHighlightedRow = pRow;
			m_nCurSearchResultsHighlightedColumn = pResult->nColumnIndex;

			//now highlight the cell both when selected an unselected, such that if they
			//click off the row the cell is still colored
			pRow->PutCellBackColor(pResult->nColumnIndex, RGB(0, 0, 255)); //blue
			pRow->PutCellForeColor(pResult->nColumnIndex, RGB(255, 255, 255)); //white
			pRow->PutCellBackColorSel(pResult->nColumnIndex, RGB(0, 0, 255)); //blue
			pRow->PutCellForeColorSel(pResult->nColumnIndex, RGB(255, 255, 255)); //white
		}

		return true;

	}NxCatchAll(__FUNCTION__);

	return false;
}

// (j.jones 2015-03-17 13:25) - PLID 64931 - if the search results window exists,
// clear the results
void CFinancialDlg::ClearSearchResults()
{
	try {

		if (m_dlgSearchBillingTab.GetSafeHwnd()) {
			//when called from the billing tab, never clear the search term
			m_dlgSearchBillingTab.ClearSearchInfo(false);
		}

		// (j.jones 2015-03-17 14:08) - PLID 64929 - if we have a highlighted cell, revert its color
		RevertSearchResultCellColor();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-19 17:23) - PLID 64931 - checks to see if our search results are still
// potentially valid, clears the results if we know they are not
void CFinancialDlg::RevalidateSearchResults()
{
	try {

		if (m_RedrawRefCount > 0) {
			//the screen is still redrawing, which means EnableFinancialScreen
			//will call this function when the ref count is 0
			return;
		}

		if (m_dlgSearchBillingTab.GetSafeHwnd()) {
			m_dlgSearchBillingTab.RevalidateSearchResults();
		}
		else {
			// (j.jones 2015-03-17 14:08) - PLID 64929 - if we have a highlighted cell, revert its color
			RevertSearchResultCellColor();
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2015-03-17 14:09) - PLID 64929 - if we have a highlighted cell, revert its color
void CFinancialDlg::RevertSearchResultCellColor()
{
	if (m_pCurSearchResultsHighlightedRow != NULL && m_nCurSearchResultsHighlightedColumn != -1) {
		m_pCurSearchResultsHighlightedRow->PutCellBackColor(m_nCurSearchResultsHighlightedColumn, m_clrCurSearchResultCellBackgroundColor);
		m_pCurSearchResultsHighlightedRow->PutCellForeColor(m_nCurSearchResultsHighlightedColumn, m_clrCurSearchResultCellForegroundColor);
		m_pCurSearchResultsHighlightedRow->PutCellBackColorSel(m_nCurSearchResultsHighlightedColumn, m_clrCurSearchResultCellBackgroundSelColor);
		m_pCurSearchResultsHighlightedRow->PutCellForeColorSel(m_nCurSearchResultsHighlightedColumn, m_clrCurSearchResultCellForegroundSelColor);
	}

	m_pCurSearchResultsHighlightedRow = NULL;
	m_nCurSearchResultsHighlightedColumn = -1;
	m_clrCurSearchResultCellBackgroundColor = RGB(255, 255, 255);
	m_clrCurSearchResultCellForegroundColor = RGB(0, 0, 0);
	m_clrCurSearchResultCellBackgroundSelColor = RGB(255, 255, 255);
	m_clrCurSearchResultCellForegroundSelColor = RGB(0, 0, 0);
}